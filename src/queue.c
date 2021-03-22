/**
 * Message queue sample kernel module.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>

#define PROCFS_FILE_MODE 0666
#define MSG_MAX_SIZE 1024
#define PROCFS_NAME "msg_queue"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Chebotaryov S.");

/**
 * Pointer to procfs entry which will be used to write to & read from the queue
 */
static struct proc_dir_entry *procfs_entry;

/**
 * Strucutre to hold user messages
 */
struct user_message {
  char* buf; // message buffer
  size_t size; // message buffer size in bytes
  struct list_head list;
};


/**
 * List pointer to implement queue of messages
 */
static LIST_HEAD(msg_list);

/**
 * Spinlock for the msg_list
 */
static DEFINE_SPINLOCK(msg_list_lock);

/**
 * Constructor for user message objects
 */
struct user_message* user_message_construct(size_t msg_size) {
  struct user_message* new_msg;

  new_msg = kmalloc(sizeof(*new_msg), GFP_KERNEL);
  if (new_msg == NULL) {
    return NULL;
  }
  new_msg->buf = kmalloc(msg_size, GFP_KERNEL);
  if (new_msg->buf == NULL) {
    kfree(new_msg);
    return NULL;
  }

  new_msg->size = msg_size;
  INIT_LIST_HEAD(&new_msg->list);

  return new_msg;
}

/**
 * Destructor for user message objects
 */
void user_message_free(struct user_message* user_msg) {
  kfree(user_msg->buf);
  kfree(user_msg);
}

/**
 * Adds new user message to the tail of the list
 */
void user_message_list_add_tail(struct user_message* msg, struct list_head* head) {
  spin_lock(&msg_list_lock);
  list_add_tail(&msg->list, head);
  spin_unlock(&msg_list_lock);
  printk(KERN_INFO "kmsg_queue: added new user message to linked list: %s \n", msg->buf);
}

/**
 * Removes user_message from the top of the list and returns it
 */
struct user_message* user_message_list_pop(struct list_head* head) {
  struct user_message* user_msg;

  // take a lock before inspecting the linked list
  spin_lock(&msg_list_lock);

  if (head->next == head) {
    // release lock and return null if the list is empty
    spin_unlock(&msg_list_lock);
    return NULL;
  }

  user_msg = container_of(head->next, struct user_message, list);
  list_del(&user_msg->list);

  // release lock and return result
  spin_unlock(&msg_list_lock);
  return user_msg;
}

static ssize_t write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
  size_t buffer_size, buffer_size_bytes;
  struct user_message* new_msg;

  printk(KERN_INFO "kmsg_queue: write() (/proc/%s) called\n", PROCFS_NAME);

  if (*ppos > 0) {
    printk(
           KERN_INFO
           "kmsg_queue: write(): this is a left-over call to transer user data. "
           "Currently this module does not support arbitrarily large messages.\n");
    *ppos = 0;
    // return count bytes to avoid more calls on this message
    printk(KERN_INFO "kmsg_queue: write() (/proc/%s) finished\n", PROCFS_NAME);
    return count;
  }

  printk(KERN_INFO "kmsg_queue: got message with data buffer of size %zu\n", count);

  buffer_size = count;
  if (buffer_size > MSG_MAX_SIZE ) {
    // limit message max size to avoid excessively long messages & high memory consumption
    buffer_size = MSG_MAX_SIZE;
    printk
      (KERN_INFO
       "kmsg_queue: size of the input user message is %zu "
       "which is larger than the allowed max size: %u. "
       "User message will be truncated.\n",
       count, MSG_MAX_SIZE);
  }
  buffer_size_bytes = sizeof(*(new_msg->buf)) * buffer_size;

  // allocate memory buffer to hold the user message
  new_msg = user_message_construct(buffer_size_bytes);
  if (new_msg == NULL) {
    printk(KERN_WARNING
           "kmsg_queue: Cannot allocate memory for new user message. "
           "This message cannot be added to the queue!");
    return -EFAULT;
  }

  /* copy over message data from user space to our buffer in kernel space */
  if ( copy_from_user(new_msg->buf, ubuf, buffer_size_bytes) ) {
    printk(KERN_INFO "kmsg_queue: write() (/proc/%s) copy_from_user failed\n", PROCFS_NAME);
    // free up allocated resources and return
    user_message_free(new_msg);
    *ppos = 0;
    return -EFAULT;
  }

  // if all went well, then just insert new message to our list
  user_message_list_add_tail(new_msg, &msg_list);

  printk(KERN_INFO "kmsg_queue: write() (/proc/%s) finished\n", PROCFS_NAME);
  *ppos = buffer_size_bytes;
  return buffer_size_bytes;
}


static ssize_t read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
  struct user_message* user_msg;
  size_t buffer_size;

  printk(KERN_INFO "kmsg_queue: read() (/proc/%s) called\n", PROCFS_NAME);
  printk
    (KERN_INFO
     "kmsg_queue: max number of bytes to return (count arg value): %zu\n",
     count);

  if (*ppos > 0) {
    printk(
           KERN_INFO
           "kmsg_queue: read(): this is a left-over call to transer user data. "
           "Currently this module does not support arbitrarily large messages.");
    printk(KERN_INFO "kmsg_queue: ppos = %llu. read() finished.\n", *ppos);
    return 0;
  }

  // extract list entry from list_head structure
  user_msg = user_message_list_pop(&msg_list);
  if (user_msg == NULL) {
    printk(KERN_INFO "kmsg_queue: messages list is empty nothing to read");
    printk(KERN_INFO "kmsg_queue: read() (/proc/%s) finished\n", PROCFS_NAME);
    return 0;
  }

  printk(KERN_INFO
         "kmsg_queue: returning user message of size %zu; content: %s",
         user_msg->size, user_msg->buf);

  buffer_size = user_msg->size;
  if (buffer_size > count) {
    buffer_size = count;
    printk(KERN_INFO
           "kmsg_queue: Stored message (%zu bytes) is larger than the "
           "user buffer (%zu bytes). Message will be truncated.\n",
           user_msg->size, count);
  }

  /* fill the user buffer */
  memcpy(ubuf, user_msg->buf, buffer_size);
  *ppos = buffer_size;
  /* deallocate the user_message object */
  user_message_free(user_msg);

  /*  return the number of bytes written to user buffer */
  printk(KERN_INFO "kmsg_queue: read() (/proc/%s) finished\n", PROCFS_NAME);
  return buffer_size;
}

int	open(struct inode *inode, struct file *file) {
  try_module_get(THIS_MODULE);
  return 0;
}

int	release(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  return 0;
}

static struct proc_ops proc_file_ops =
  {
   .proc_read = read,
   .proc_write = write,
   .proc_open = open,
   .proc_release = release
  };

static int init(void)
{
  printk(KERN_INFO "kmsg_queue: entering init()\n");

  procfs_entry = proc_create(PROCFS_NAME, PROCFS_FILE_MODE, NULL, &proc_file_ops);

  printk(KERN_INFO "kmsg_queue: init() finished\n");
  return 0;
}

/*
 * Function cleans-up all left-over resources and deregisters procfs file.
 */
static void cleanup(void)
{
  struct user_message* user_msg;

  printk(KERN_INFO "kmsg_queue: entering cleanup()\n");

  while((user_msg = user_message_list_pop(&msg_list)) != NULL) {
    printk(KERN_INFO "kmsg_queue: removed message of size %zu\n", user_msg->size);
    user_message_free(user_msg);
  }

  printk(KERN_INFO "kmsg_queue: cleanup() finished\n");
  proc_remove(procfs_entry);
}

module_init(init);
module_exit(cleanup);
