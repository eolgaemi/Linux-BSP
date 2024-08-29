#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/moduleparam.h>

#define OFF 0
#define ON 1
#define GPIOCNT 8
//#define DEBUG

static long gpioLedInit(void);
static void gpioLedSet(long);
static void gpioLedFree(void);

static long gpioKeyInit(void);
static long gpioKeyGet(void);
static void gpioKeyFree(void);

static long gpioIrqRequest(void);
static void gpioIrqFree(void);

static long ledValOn = 0xff;
static long ledValOff = 0x00;


module_param(ledValOn,long,0);
module_param(ledValOff,long,0);


static int gpioLed[] = {6,7,8,9,10,11,12,13};
static int gpioKey[] = {16,17,18,19,20,21,22,23};

static int irq_numbers[8];

static long key_status_new;
static long key_status_old=0;

static irqreturn_t gpio_irq_handler(int irq, void *dev_id){

	key_status_new = gpioKeyGet();

	if((key_status_new!=key_status_old)){
		gpioLedSet(0x00);
		gpioLedSet(key_status_new);
		key_status_old=key_status_new;
	}

	return IRQ_HANDLED;
}

static int hello_init(void)

{
#ifdef DEBUG
		printk("GPIO_LEDKEY On=%ld ,Off=%ld \n",ledValOn,ledValOff);
#endif
		long ret = 0;

		ret = gpioLedInit();
		if(ret < 0)
			return ret;

		gpioLedSet(ledValOn);

		ret = gpioKeyInit();
		if(ret < 0)
			return ret;

		ret = gpioKeyGet();

		gpioIrqRequest();

		return 0;
}

static void hello_exit(void)
{		
		gpioIrqFree();
		gpioLedSet(ledValOff);
		gpioLedFree();
		gpioKeyFree();
#ifdef DEBUG
		printk("Good bye World!\n");
#endif
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
//		gpio_set_value(gpioLed[i],(val>>i) & 0x1);
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

static long gpioIrqRequest(void)
{
	int i;
	long ret;

	for(i=0;i<GPIOCNT;i++)
	{
		irq_numbers[i] = gpio_to_irq(gpioKey[i]);
		ret = request_irq(irq_numbers[i],gpio_irq_handler,IRQF_TRIGGER_RISING,"gpio_irq_handler",(void *)&gpioKey[i]);
		if(ret){
				printk(KERN_ALERT "Failed to request IRQ for GPIO %d\n",gpioKey[i]);
		}
	}

	return ret;
}

static void gpioIrqFree(void){

	int i;

	for(i=0;i<GPIOCNT;i++){
		free_irq(irq_numbers[i],(void *)&gpioKey[i]);
	}
}



module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR ("KCCI_INTEL_EDGE");
MODULE_DESCRIPTION("Test_Module");
MODULE_LICENSE("Dual BSD/GPL");
