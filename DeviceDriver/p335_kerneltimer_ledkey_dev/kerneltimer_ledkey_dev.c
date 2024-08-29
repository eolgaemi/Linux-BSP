#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>

#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/moduleparam.h>

#define KERNELTIMER_DEV_NAME		"kerneltimer"
#define KERNELTIMER_DEV_MAJOR	230

#define DEBUG 1
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

static int timerVal = 100; // f=100HZ, T=1/100 = 10ms, 100*10ms = 1sec
static int ledVal = 0;
static long key_data;
static long key_data_old = 0;

module_param(timerVal,int,0);
module_param(ledVal,int,0);

struct timer_list timerLed;

void kerneltimer_func(struct timer_list *t);
void kerneltimer_registertimer(unsigned long timeover)
{
	timerLed.expires = get_jiffies_64() + timeover; //10ms * 100 = 1sec
	timer_setup(&timerLed,kerneltimer_func,0);
	add_timer(&timerLed);
}

static int kerneltimer_open (struct inode *inode, struct file *filp)
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


static loff_t kerneltimer_llseek (struct file *filp, loff_t off, int whence)
{
#ifdef DEBUG
                printk("call llseek -> off : %08X, whence : %08X\n",(unsigned int)off,whence);
#endif
                return 0x23;
}

static ssize_t kerneltimer_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
                char kbuf;
                kbuf = (char)gpioKeyGet();// i know
                copy_to_user(buf,&kbuf,1);
                put_user(kbuf,buf);
#ifdef DEBUG
                printk("call read -> buf : %08X, count: %08X \n",(unsigned int)buf,count);
#endif
                return count;
}

ssize_t kerneltimer_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos)
{
                char kbuf;
                int ret;
                ret = copy_from_user(&kbuf,buf,count);
                gpioLedSet(kbuf);
                //gpioLedSet(*buf);
#ifdef DEBUG
                printk("call write -> buf : %08X, count: %08X \n",(unsigned int)buf,count);
#endif
                return count;
}

static long kerneltimer_ioctl ( struct file *filp, unsigned int cmd, unsigned long arg)
{
#ifdef DEBUG
        printk("call ioctl -> cmd : %08X, arg: %08X \n",cmd,(unsigned int)arg);
#endif
        return 0x53;
}

static int kerneltimer_release (struct inode *inode, struct file *filp)
{
#ifdef DEBUG
        printk("call released \n");
#endif
        return 0;
}

struct file_operations kerneltimer_fops =
{
//    .owner    = THIS_MODULE,
    .open     = kerneltimer_open,
    .read     = kerneltimer_read,
    .write    = kerneltimer_write,
    .unlocked_ioctl = kerneltimer_ioctl,
    .llseek   = kerneltimer_llseek,
    .release  = kerneltimer_release,
};

void kerneltimer_func(struct timer_list *t)
{
#if DEBUG
		printk("ledVal : %#04x\n",(unsigned int)(key_data));
#endif
		key_data = gpioKeyGet();

		if((key_data!=key_data_old)&(key_data>0))
		{
			gpioLedSet(key_data);
			key_data_old = key_data;
		}

		//gpioLedSet(ledVal);
		//ledVal = ~ledVal & 0xff;
		mod_timer(t,get_jiffies_64() + timerVal);
}

int kerneltimer_init(void)
{	
		int result;
		gpioLedFree();
		gpioKeyFree();
		gpioLedInit();
		gpioKeyInit();
		result = register_chrdev(KERNELTIMER_DEV_MAJOR,KERNELTIMER_DEV_NAME,&kerneltimer_fops);
#if DEBUG
		printk("timerVal : %d , sec: %d \n",  timerVal,timerVal/HZ);
#endif
		kerneltimer_registertimer(timerVal);
		return 0;
}
void kerneltimer_exit(void)
{
	gpioLedSet(0);
	gpioLedFree();
	gpioKeyFree();
	if(timer_pending(&timerLed))
			del_timer(&timerLed);
	unregister_chrdev(KERNELTIMER_DEV_MAJOR,KERNELTIMER_DEV_NAME);
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


module_init(kerneltimer_init);
module_exit(kerneltimer_exit);
MODULE_AUTHOR("KCCI-AIOT-JYJ");
MODULE_DESCRIPTION("LED KEY TEST MODULE");
MODULE_LICENSE("Dual BSD/GPL");

