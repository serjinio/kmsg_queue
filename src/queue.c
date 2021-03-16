#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#define BUFSIZE  100


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Chebotaryov S.");

#define PROCFS_FILE_MODE 0666
#define PROCFS_MAX_SIZE		1024
#define PROCFS_NAME     "msg_queue"

/**
 * The buffer used to store character for this module
 *
 */
static char procfs_buffer[PROCFS_MAX_SIZE];

/**
 * The size of the buffer
 *
 */
static unsigned long procfs_buffer_size = 0;

static struct proc_dir_entry *procfs_entry;


static ssize_t procfile_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
  printk(KERN_INFO "procfile_write (/proc/%s) called\n", PROCFS_NAME);
  /* get buffer size */
  procfs_buffer_size = count;
  if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
    procfs_buffer_size = PROCFS_MAX_SIZE;
  }

  /* write data to the buffer */
  if ( copy_from_user(procfs_buffer, ubuf, procfs_buffer_size) ) {
    printk(KERN_INFO "procfile_write (/proc/%s) copy_from_user failed\n", PROCFS_NAME);
    *ppos = 0;
    return -EFAULT;
  }

  printk(KERN_INFO "procfs_buffer contents: /%s\n", procfs_buffer);
  return procfs_buffer_size;
}

static ssize_t procfile_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos)
{
  ssize_t ret;

  printk(KERN_INFO "procfile_read (/proc/%s) called\n", PROCFS_NAME);
  printk(KERN_INFO "procfs_buffer contents: /%s\n", procfs_buffer);

  if (*ppos > 0) {
    /* we have finished to read, return 0 */
    printk(KERN_INFO "ppos > 0 - read finished: %lld\n", *ppos);
    ret  = 0;
  } else {
    /* fill the buffer, return the buffer size */
    memcpy(ubuf, procfs_buffer, procfs_buffer_size);
    ret = procfs_buffer_size;
    *ppos = procfs_buffer_size;
  }

  return ret;
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
