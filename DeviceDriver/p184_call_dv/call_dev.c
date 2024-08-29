#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>

#define CALL_DEV_NAME	"calldev"
#define CALL_DEV_MAJOR	230
#define DEBUG

static int call_open (struct inode *inode, struct file *filp)
{
	int num_minor = MINOR(inode->i_rdev);
	int num_major = MAJOR(inode->i_rdev);
#ifdef DEBUG
	printk("call open -> minor : %d\n",num_minor);
#endif
#ifdef DEBUG
	printk("call open -> major : %d\n",num_major);
#endif
	return 0;
}


static loff_t call_llseek (struct file *filp, loff_t off, int whence)
{
#ifdef DEBUG
		printk("call llseek -> off : %08X, whence : %08X\n",(unsigned int)off,whence);
#endif
		return 0x23;
}

static ssize_t call_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
#ifdef DEBUG
		printk("call read -> buf : %08X, count: %08X \n",(unsigned int)buf,count);
#endif
		return 0x33;
}

ssize_t call_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos)
{
#ifdef DEBUG
		printk("call write -> buf : %08X, count: %08X \n",(unsigned int)buf,count);
#endif
		return 0x43;
}

static long call_ioctl ( struct file *filp, unsigned int cmd, unsigned long arg)
{
#ifdef DEBUG
	printk("call ioctl -> cmd : %08X, arg: %08X \n",cmd,(unsigned int)arg);
#endif
	return 0x53;
}

static int call_release (struct inode *inode, struct file *filp)
{
#ifdef DEBUG
	printk("call released \n");
#endif
	return 0;
}

struct file_operations call_fops = 
{
	.owner = THIS_MODULE,
	.llseek = call_llseek,
	.read = call_read,
	.write = call_write,
	.unlocked_ioctl = call_ioctl,
	.open = call_open,
	.release = call_release,
};

static int call_init(void)
{
	int result;

#ifdef DEBUG
	printk("call_call_init\n");
#endif
	result = register_chrdev(CALL_DEV_MAJOR, CALL_DEV_NAME, &call_fops);
	if (result < 0) return result;
	return 0;
}

static void call_exit(void)
{
#ifdef DEBUG
	printk("call call_exit\n");
#endif
	unregister_chrdev( CALL_DEV_MAJOR, CALL_DEV_NAME);
}

module_init(call_init);
module_exit(call_exit);

MODULE_AUTHOR("KCCI_AIOT");
//MODULE_
MODULE_LICENSE("Dual BSD/GPL");
