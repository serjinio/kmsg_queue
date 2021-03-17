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
#include <asm/uaccess.h>

#define PROCFS_FILE_MODE 0666
#define MSG_MAX_SIZE	10
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
 * Constructor for user message objects
 */
struct user_message* user_message_construct(size_t msg_size) {
  struct user_message* new_msg;

  new_msg = kmalloc(sizeof(*new_msg), GFP_KERNEL);
  new_msg->buf = kmalloc(msg_size, GFP_KERNEL);
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


static ssize_t procfile_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
  size_t buffer_size, buffer_size_bytes;
  struct user_message* new_msg;

  printk(KERN_INFO "kmsg_queue: procfile_write (/proc/%s) called\n", PROCFS_NAME);

  printk(KERN_INFO "ppos: %llu", *ppos);
  if (*ppos > 0) {
    printk(
           KERN_INFO
           "kmsg_queue: procfile_write(): this is a left-over call to transer user data. "
           "Currently this module does not support arbitrarily large messages.");
    *ppos = 0;
    // return count bytes to avoid more calls on this message
    printk(KERN_INFO "kmsg_queue: procfile_write (/proc/%s) finished\n", PROCFS_NAME);
    return count;
  }

  printk(KERN_INFO "kmsg_queue: got message with data buffer of size %zu ", count);

  buffer_size = count;
  if (buffer_size > MSG_MAX_SIZE ) {
    // limit message max size to avoid excessively long messages & high memory consumption
    buffer_size = MSG_MAX_SIZE;
    printk
      (KERN_INFO
       "kmsg_queue: size of the input user message is %zu "
       "which is larger than the allowed max size: %u. "
       "User message will be truncated.",
       count, MSG_MAX_SIZE);
  }
  buffer_size_bytes = sizeof(*(new_msg->buf)) * buffer_size;

  // allocate memory buffer to hold the user message
  new_msg = user_message_construct(buffer_size_bytes);

  /* copy over message data from user space to our buffer in kernel space */
  if ( copy_from_user(new_msg->buf, ubuf, buffer_size_bytes) ) {
    printk(KERN_INFO "kmsg_queue: procfile_write (/proc/%s) copy_from_user failed\n", PROCFS_NAME);
    // free up allocated resources and return
    user_message_free(new_msg);
    *ppos = 0;
    return -EFAULT;
  }

  // if all went well, then just insert new message to our list
  list_add_tail(&new_msg->list, &msg_list);
  printk(KERN_INFO "kmsg_queue: added new user message to linked list: %s", new_msg->buf);

  printk(KERN_INFO "kmsg_queue: procfile_write (/proc/%s) finished\n", PROCFS_NAME);
  *ppos = buffer_size_bytes;
  return buffer_size_bytes;
}


static ssize_t procfile_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
  struct user_message* user_msg;
  size_t buffer_size;

  printk(KERN_INFO "kmsg_queue: procfile_read (/proc/%s) called\n", PROCFS_NAME);
  printk
    (KERN_INFO
     "kmsg_queue: max number of bytes to return (count arg value): %zu",
     count);

  if (msg_list.next == &msg_list) {
    printk(KERN_INFO "kmsg_queue: messages list is empty nothing to read");
    printk(KERN_INFO "kmsg_queue: procfile_read (/proc/%s) finished\n", PROCFS_NAME);
    return 0;
  }

  if (*ppos > 0) {
    printk(
           KERN_INFO
           "kmsg_queue: procfile_read(): this is a left-over call to transer user data. "
           "Currently this module does not support arbitrarily large messages.");
    printk(KERN_INFO "kmsg_queue: ppos = %llu. procfile_read finished.\n", *ppos);
    return 0;
  }

  // extract list entry from list_head structure
  user_msg = container_of(msg_list.next, struct user_message, list);
  printk(KERN_INFO
         "kmsg_queue: returning user message of size %zu; content: %s",
         user_msg->size, user_msg->buf);

  buffer_size = user_msg->size;
  if (buffer_size > count) {
    buffer_size = count;
    printk(KERN_INFO
           "kmsg_queue: Stored message (%zu bytes) is larger than the "
           "user buffer (%zu bytes). Message will be truncated.",
           user_msg->size, count);
  }

  /* fill the user buffer */
  memcpy(ubuf, user_msg->buf, buffer_size);
  *ppos = buffer_size;
  /*  remove list entry and deallocate the user_message object */
  list_del(&user_msg->list);
  user_message_free(user_msg);

  /*  return the number of bytes written to user buffer */
  printk(KERN_INFO "kmsg_queue: procfile_read (/proc/%s) finished\n", PROCFS_NAME);
  return buffer_size;
}

int	procfile_open(struct inode *inode, struct file *file) {
  try_module_get(THIS_MODULE);
  return 0;
}

int	procfile_release(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  return 0;
}

static struct proc_ops proc_file_ops =
  {
   .proc_read = procfile_read,
   .proc_write = procfile_write,
   .proc_open = procfile_open,
   .proc_release = procfile_release
  };

static int init(void)
{
  procfs_entry = proc_create(PROCFS_NAME, PROCFS_FILE_MODE, NULL, &proc_file_ops);
  return 0;
}

static void cleanup(void)
{
  proc_remove(procfs_entry);
}

module_init(init);
module_exit(cleanup);
