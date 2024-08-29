#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int hello_init(void)

{
		printk("Hello World \n");

		return 0;
}

static void hello_exit(void)
{
		printk("Good bye World!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR ("KCCI _INTEL_EDGE");
MODULE_DESCRIPTION("Test_Module");
MODULE_LICENSE("Dual BSD/GPL");
