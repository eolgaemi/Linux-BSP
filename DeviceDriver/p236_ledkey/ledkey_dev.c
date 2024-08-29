#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>

#include <linux/gpio.h>
#include <linux/moduleparam.h>

#define LEDKEY_DEV_NAME	"ledkey"
#define LEDKEY_DEV_MAJOR	230
#define DEBUG

#define OFF 0
#define ON 1
#define GPIOCNT 8

static long gpioLedInit(void);
static void gpioLedSet(long);
static void gpioLedFree(void);

static long gpioKeyInit(void);
static long gpioKeyGet(void);
static void gpioKeyFree(void);

static int gpioLed[] = {6,7,8,9,10,11,12,13};
static int gpioKey[] = {16,17,18,19,20,21,22,23};

static int ledkey_open (struct inode *inode, struct file *filp)
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


static loff_t ledkey_llseek (struct file *filp, loff_t off, int whence)
{
#ifdef DEBUG
		printk("call llseek -> off : %08X, whence : %08X\n",(unsigned int)off,whence);
#endif
		return 0x23;
}

static ssize_t ledkey_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
		char kbuf[128];
		int i;
		kbuf = (char)gpioKeyGet();// i know
		for(i=0;i<count;i++)
			get_user(kbuf[i],buf++);
		put_user(kbuf,buf);
#ifdef DEBUG
		printk("call read -> buf : %08X, count: %08X \n",(unsigned int)buf,count);
#endif
		return count;
}

ssize_t ledkey_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos)
{
		char kbuf;
		int ret;
		ret = copy_from_user(kbuf,buf);
		gpioLedSet(kbuf);
		//gpioLedSet(*buf);
#ifdef DEBUG
		printk("call write -> buf : %08X, count: %08X \n",(unsigned int)buf,count);
#endif
		return count;
}

static long ledkey_ioctl ( struct file *filp, unsigned int cmd, unsigned long arg)
{
#ifdef DEBUG
	printk("call ioctl -> cmd : %08X, arg: %08X \n",cmd,(unsigned int)arg);
#endif
	return 0x53;
}

static int ledkey_release (struct inode *inode, struct file *filp)
{
#ifdef DEBUG
	printk("call released \n");
#endif
	return 0;
}

struct file_operations ledkey_fops = 
{
	.owner = THIS_MODULE,
	.llseek = ledkey_llseek,
	.read = ledkey_read,
	.write = ledkey_write,
	.unlocked_ioctl = ledkey_ioctl,
	.open = ledkey_open,
	.release = ledkey_release,
};

static int ledkey_init(void)
{
	int result;

#ifdef DEBUG
	printk("ledkey_ledkey_init\n");
#endif

	result = register_chrdev(LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME, &ledkey_fops);
	if (result < 0) return result;

	gpioLedFree();
	gpioKeyFree();

	result = gpioLedInit();
	if(result<0)
			return result;
	
	result = gpioKeyInit();
	if(result<0)
			return result;

	return 0;
}

static void ledkey_exit(void)
{
#ifdef DEBUG
	printk("call ledkey_exit\n");
#endif
	gpioLedFree();
	gpioKeyFree();
	unregister_chrdev( LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME);
}

static long gpioLedInit(void)
{
        long ret = 0;
        int i;
        char gpioName[10];

        for(i=0;i<GPIOCNT;i++)
        {
                sprintf(gpioName,"gpio%d",gpioLed[i]);
                ret = gpio_request(gpioLed[i],gpioName);
                if(ret < 0) {
                        printk("Failed request gpio%d error\n",gpioLed[i]);
                        return ret;
                }
        }

        for(i=0;i<GPIOCNT;i++)
        {
                ret = gpio_direction_output(gpioLed[i],OFF);
                if(ret < 0) {
                        printk("Failed direction_output gpio%d error\n",gpioLed[i]);
                        return ret;
                }
        }

        return ret;
}

static void gpioLedSet(long val)
{
        int i;

        for(i=0;i<GPIOCNT;i++)
        {
                gpio_set_value(gpioLed[i],val & (0x1 << i));
//              gpio_set_value(gpioLed[i],(val>>i) & 0x1);
        }

}
static void gpioLedFree(void)
{
        int i;

        for(i=0;i<GPIOCNT;i++)
        {
                gpio_free(gpioLed[i]);
        }
}

static long gpioKeyInit(void)
{
        long ret = 0;
        int i;
        char gpioName[10];

        for(i=0;i<GPIOCNT;i++)
        {
                sprintf(gpioName,"gpio%d",gpioKey[i]);
                ret = gpio_request(gpioKey[i],gpioName);
                if(ret < 0) {
                        printk("Failed request gpio%d error\n",6);
                        return ret;
                }
        }

        for(i=0;i<GPIOCNT;i++)
        {
                ret = gpio_direction_input(gpioKey[i]);
                if(ret < 0) {
                        printk("Failed direction_output gpio%d error\n",6);
                        return ret;
                }
        }

        return ret;
}

static long gpioKeyGet(void)
{
        long keyData=0;
        long temp;
        int i;

        for(i=0;i<GPIOCNT;i++)
        {
                temp = gpio_get_value(gpioKey[i]) << i;
                keyData |= temp;
        }

        return keyData;
}

static void gpioKeyFree(void)
{
        int i;

        for(i=0;i<GPIOCNT;i++)
        {
                gpio_free(gpioKey[i]);
        }
}


module_init(ledkey_init);
module_exit(ledkey_exit);

MODULE_AUTHOR("KCCI_AIOT");
//MODULE_
MODULE_LICENSE("Dual BSD/GPL");
