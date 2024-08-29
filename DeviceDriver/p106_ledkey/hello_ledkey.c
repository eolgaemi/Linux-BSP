#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define OFF 0
#define ON 1
#define GPIOCNT 8

long gpioLedInit(void);
void gpioLedSet(long);
void gpioLedFree(void);
long gpioKeyInit(void);
long gpioKeyGet(void);
void gpioKeyFree(void);
long gpioIrqRequest(void);
void gpioIrqFree(void);

int gpioLed[] = {6,7,8,9,10,11,12,13};
int gpioKey[] = {16,17,18,19,20,21,22,23};

int irq_numbers[8];

long key_status_new;
long key_status_old=0;

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
		printk("Hello GPIO_LEDKEY \n");

		long ret = 0;

		ret = gpioLedInit();
		if(ret < 0)
			return ret;

		gpioLedSet(0xff);

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
		gpioLedSet(0x00);
		gpioLedFree();
		gpioKeyFree();
		printk("Good bye World!\n");
}

long gpioLedInit(void)
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

void gpioLedSet(long val)
{
	int i;

	for(i=0;i<GPIOCNT;i++)
	{
		gpio_set_value(gpioLed[i],val & (0x1 << i));
//		gpio_set_value(gpioLed[i],(val>>i) & 0x1);
	}

}
void gpioLedFree(void)
{
	int i;

	for(i=0;i<GPIOCNT;i++)
	{
  		gpio_free(gpioLed[i]);
	}
}
long gpioKeyInit(void)
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

long gpioKeyGet(void)
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
void gpioKeyFree(void)
{
	int i;

	for(i=0;i<GPIOCNT;i++)
	{
  		gpio_free(gpioKey[i]);
	}
}

long gpioIrqRequest(void)
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

void gpioIrqFree(void){

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
