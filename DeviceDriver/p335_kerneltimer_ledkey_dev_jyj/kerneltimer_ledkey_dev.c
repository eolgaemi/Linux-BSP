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

#include <linux/interrupt.h>
#include <linux/irq.h>

#include <linux/wait.h>
#include <linux/poll.h>

#include "kerneltimer_ioctl.h"

#define KERNELTIMER_DEV_NAME		"keyled_dev"
#define KERNELTIMER_DEV_MAJOR	230

#define DEBUG 1
#define OFF 0
#define ON 1
#define GPIOCNT 8

static long gpioLedInit(void);
static void gpioLedSet(long);
static void gpioLedFree(void);

static long gpioKeyInit(void);
//static long gpioKeyGet(void);
static void gpioKeyFree(void);

typedef struct {
    int keyNumber;
    int key_irq[GPIOCNT];
} keyDataStruct;

static int gpioKeyIrqInit(keyDataStruct * pKeyData);
static void gpioKeyIrqFree(keyDataStruct * pKeyData);
irqreturn_t key_isr(int irq, void * data);
DECLARE_WAIT_QUEUE_HEAD(WaitQueue_Read);


static int gpioLed[] = {6,7,8,9,10,11,12,13};
static int gpioKey[] = {16,17,18,19,20,21,22,23};

static int timerVal = 100; // f=100HZ, T=1/100 = 10ms, 100*10ms = 1sec
static int ledVal = 0;
//static long key_data;
//static long key_data_old = 0;

//static char ledflag = 0;

module_param(timerVal,int,0);
module_param(ledVal,int,0);

struct timer_list timerLed;

void kerneltimer_func(struct timer_list *t);
void kerneltimer_registertimer(unsigned long timeover)
{
	timerLed.expires = get_jiffies_64() + timeover; //10ms * 100 = 1sec
	timer_setup(&timerLed,kerneltimer_func,0);
	add_timer(&timerLed);
	printk("kernel timer started\n");
}

static int kerneltimer_open (struct inode *inode, struct file *filp)
{
	    int i;
	    int result;
	    char * irqName[GPIOCNT] = {"irqKey0","irqKey1","irqKey2","irqKey3","irqKey4","irqKey5","irqKey6","irqKey7"};
    	keyDataStruct * pKeyData;

        int num_minor = MINOR(inode->i_rdev);
        int num_major = MAJOR(inode->i_rdev);
#ifdef DEBUG
        printk("call open -> minor : %d\n",num_minor);
#endif
#ifdef DEBUG
        printk("call open -> major : %d\n",num_major);
#endif
	    try_module_get(THIS_MODULE);

        pKeyData = (keyDataStruct *)kmalloc(sizeof(keyDataStruct),GFP_KERNEL);
        if(!pKeyData)
        	return -ENOMEM;
    	pKeyData->keyNumber = 0;

    	result = gpioKeyIrqInit(pKeyData);
    	if(result < 0)
        	return result;

    	for(i=0;i<GPIOCNT;i++)
    	{
        	result = request_irq(pKeyData->key_irq[i],key_isr,IRQF_TRIGGER_RISING,irqName[i],pKeyData);
        	if(result < 0)
        	{
            	printk("request_irq() failed irq %d\n",pKeyData->key_irq[i]);
            	return result;
        	}
    	}
#if DEBUG
		printk("timerVal : %d , sec: %d \n",  timerVal,timerVal/HZ);
#endif

    	filp->private_data = pKeyData;
		kerneltimer_registertimer(timerVal);

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
    // char kbuf;
	//int ret;
   	keyDataStruct * pKeyData = (keyDataStruct *)filp->private_data;

    if(pKeyData->keyNumber == 0)
   	{
       	if(!(filp->f_flags & O_NONBLOCK))
       	{
           	wait_event_interruptible(WaitQueue_Read,pKeyData->keyNumber);
       	}
   	}
	printk("read val = %d\n",pKeyData->keyNumber);
	put_user(pKeyData->keyNumber,buf);

    //if(ret<0)
       // return -EFAULT;
    if(pKeyData->keyNumber != 0)
        pKeyData->keyNumber = 0;

     return count;
}

ssize_t kerneltimer_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos)
{
    char kbuf;
    //int ret;
    //ret = copy_from_user(&kbuf,buf,count);
	get_user(kbuf,buf);
    ledVal = kbuf;
    //gpioLedSet(*buf);
    return count;
}

static long kerneltimer_ioctl ( struct file *filp, unsigned int cmd, unsigned long arg)
{
#ifdef DEBUG
        printk("call ioctl -> cmd : %08X, arg: %08X \n",cmd,(unsigned int)arg);
#endif
		keyled_data info = {0};

	    int err=0, size;
   		if( _IOC_TYPE( cmd ) != TIMER_MAGIC ) return -EINVAL;
    	if( _IOC_NR( cmd ) >= TIMER_MAXNR ) return -EINVAL;

		size = _IOC_SIZE( cmd );
	    if( size )
   		 {
        	if( _IOC_DIR( cmd ) & _IOC_WRITE )
            	err = access_ok( (void *) arg, size );
        	if( !err ) return err;
   		 }
		switch( cmd )
		{
				case TIMER_START :
					if(!timer_pending(&timerLed))
						kerneltimer_registertimer(timerVal);	
					printk("TIMER_START %d\n",timerVal);
            		break;
        		case TIMER_STOP :
					if(timer_pending(&timerLed))
									del_timer(&timerLed);
					printk("TIMER_STOP\n");
            		break;
        		case TIMER_VALUE :
					err = copy_from_user((void *)&info,(void *)arg,size);
					if(err !=0)
					{
						return -EFAULT;
					}
					if(info.timer_val>0)
							timerVal=info.timer_val;
					printk("TIMER_VALUE %d\n",timerVal);
            		break;
				default:
					err =-E2BIG;
					break;
		}
        return err;
}
static __poll_t kerneltimer_poll(struct file * filp, struct poll_table_struct * wait)
{

    unsigned int mask=0;
    keyDataStruct * pKeyData = (keyDataStruct *)filp->private_data;
#ifdef DEBUG
    printk("_key : %u\n",(wait->_key & POLLIN));
#endif
    if(wait->_key & POLLIN)
        poll_wait(filp, &WaitQueue_Read, wait);
    if(pKeyData->keyNumber > 0)
        mask = POLLIN;

    return mask;
}

static int kerneltimer_release (struct inode *inode, struct file *filp)
{
#ifdef DEBUG
        printk("call released \n");
#endif
		gpioKeyIrqFree(filp->private_data);
		if(timer_pending(&timerLed))
			del_timer(&timerLed);
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
	.poll	  = kerneltimer_poll,
};

irqreturn_t key_isr(int irq, void * data)
{
    int i;
    keyDataStruct * pKeyData = (keyDataStruct *)data;
    for(i=0;i<GPIOCNT;i++)
    {   
        if(irq == pKeyData->key_irq[i])
        {
            pKeyData->keyNumber = i+1;
            break;
        }
    }   

    printk("key_isr() irq : %d, keyNumber : %d\n",irq, pKeyData->keyNumber);
    wake_up_interruptible(&WaitQueue_Read);
    return IRQ_HANDLED;
}

void kerneltimer_func(struct timer_list *t)
{
#if DEBUG
		printk("ledVal : %#04x\n",(unsigned int)(ledVal));
#endif

		gpioLedSet(ledVal);
		ledVal = ~ledVal & 0xff;
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
		return 0;
}
void kerneltimer_exit(void)
{
	gpioLedSet(0);
	gpioLedFree();
	gpioKeyFree();
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
/*
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
}*/

static void gpioKeyFree(void)
{
        int i;

        for(i=0;i<GPIOCNT;i++)
        {
                gpio_free(gpioKey[i]);
        }
}

static int gpioKeyIrqInit(keyDataStruct * pKeyData)
{
    int i;
    for(i=0;i<GPIOCNT;i++)
    {
        pKeyData->key_irq[i] = gpio_to_irq(gpioKey[i]);
        if(pKeyData->key_irq[i] < 0) {
            printk("Failed gpio_to_irq() gpio%d error\n", gpioKey[i]);
            return pKeyData->key_irq[i];
        }
    }
    return 0;
}


static void gpioKeyIrqFree(keyDataStruct * pKeyData)
{
    int i;
    for(i=0;i<GPIOCNT;i++)
    {
        free_irq(pKeyData->key_irq[i],pKeyData);
    }
}

module_init(kerneltimer_init);
module_exit(kerneltimer_exit);
MODULE_AUTHOR("KCCI-AIOT-JYJ");
MODULE_DESCRIPTION("LED KEY TEST MODULE");
MODULE_LICENSE("Dual BSD/GPL");
