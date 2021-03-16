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

#define BUFSIZE 100
#define PROCFS_FILE_MODE 0666
#define MSG_MAX_SIZE	1024
#define PROCFS_NAME "msg_queue"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Chebotaryov S.");

/**
 * Strucutre to hold user messages
 */
struct user_message {
  char* buf; // message buffer
  size_t size; // message buffer size in bytes
  struct list_head list;
};

/**
 * Constructor for user message objects
 */
struct user_message* user_message_construct(size_t msg_size) {
  struct user_message* new_msg;

  new_msg = kmalloc(sizeof(*new_msg), GFP_KERNEL);
  new_msg->buf = kmalloc(msg_size, GFP_KERNEL);
  new_msg->size = msg_size;
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
 * List pointer to implement queue of messages
 */
static LIST_HEAD(msg_list);

/**
 * Pointer to procfs entry which will be used to write to & read from the queue
 */
static struct proc_dir_entry *procfs_entry;


static ssize_t procfile_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
  unsigned int buffer_size, buffer_size_bytes;
  struct user_message* new_msg;

  printk(KERN_INFO "kmsg_queue: procfile_write (/proc/%s) called\n", PROCFS_NAME);
  printk(KERN_INFO "kmsg_queue: got message with data buffer of size %zu ", count);

  buffer_size = count;
  if (buffer_size > MSG_MAX_SIZE ) {
    // limit message max size to avoid excessively long messages & high memory consumption
    buffer_size = MSG_MAX_SIZE;
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
  return buffer_size_bytes;
}


static ssize_t procfile_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos)
{
  struct user_message* user_msg;

  printk(KERN_INFO "kmsg_queue: procfile_read (/proc/%s) called\n", PROCFS_NAME);

  if (msg_list.next == msg_list.prev) {
    printk(KERN_INFO "kmsg_queue: messages list is empty nothing to read");
    return 0;
  }

  if (*ppos > 0) {
    /* we have finished to read, return 0 */
    printk(KERN_INFO "kmsg_queue: ppos > 0. read finished: %lld\n", *ppos);
    return 0;
  }

  // extract list entry from list_head structure
  user_msg = container_of(msg_list.next, struct user_message, list);
  printk(KERN_INFO
         "kmsg_queue: returning user message of size %zu; content: %s",
         user_msg->size, user_msg->buf);

  /* fill the user buffer, return the buffer size */
  memcpy(ubuf, user_msg->buf, user_msg->size);
  *ppos = user_msg->size;

  printk(KERN_INFO "kmsg_queue: procfile_read (/proc/%s) finished\n", PROCFS_NAME);
  return user_msg->size;
}

int	procfile_open(struct inode *inode, struct file *file) {
  try_module_get(THIS_MODULE);
  return 0;
}

int	procfile_release(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  return 0;		/* success */
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
