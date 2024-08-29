#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0x92997ed8, "_printk" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x3d87ed5b, "gpio_to_desc" },
	{ 0xb4ca5a15, "gpiod_set_raw_value" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x5f754e5a, "memset" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x97255bdf, "strlen" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0x43838dc4, "__register_chrdev" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x39cf4498, "gpiod_direction_output_raw" },
	{ 0x943b4d0f, "gpiod_direction_input" },
	{ 0x9cc32aaf, "gpiod_to_irq" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0xd929107a, "proc_mkdir" },
	{ 0x95126635, "proc_create_data" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xce969d55, "remove_proc_entry" },
	{ 0xfe990052, "gpio_free" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x637493f3, "__wake_up" },
	{ 0x800473f, "__cond_resched" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x647af474, "prepare_to_wait_event" },
	{ 0x1000e51, "schedule" },
	{ 0x49970de8, "finish_wait" },
	{ 0x20000329, "simple_strtoul" },
	{ 0xdf49bbc7, "param_ops_charp" },
	{ 0xf265530, "param_ops_ulong" },
	{ 0x65a4ef4c, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "0FEC398555E86C563665B0E");
