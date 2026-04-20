/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/platform_device.h>
#include "jill.h"

static struct resource jill_resources0[] = {
	[0] = {
		.start = JILL_GPFCON,
		.end = JILL_GPFCON + 12 - 1,
		.flags = IORESOURCE_MEM,
	},
#ifdef JILL_IRQ
	[1] = {
		.start = xxx,
		.end = xxx,
		.name = JILL_DEVICE_NAME "_irq",
		.flags = IORESOURCE_IRQ,
	},
#endif
};

static struct jill_platform_data jill_platdata0 = {
	.seconds = 10,
	.nano_seconds = 100,
	.gpf_con_and = 0xc03c,
	.gpf_con_or = 0x1542,
	.gpf_up_or = 0x79,
	/* high level: quench		low level: lighten */
	.default_value = 0x0e,
};

static void jill_release_device0(struct device *pdev)
{
	/* if platform_device is dynamic alloc, we can free platform_device here. */
	JILL_PAINT_DEBUG("%s\n\n", __FUNCTION__);
}

static struct platform_device jill_platform_device0 = {
	.name = JILL_DEVICE_NAME,
	.id = 0,
	.dev= {
		.platform_data = (void *)(&jill_platdata0),
		.release = jill_release_device0,
	},
	.num_resources= ARRAY_SIZE(jill_resources0),
	.resource = jill_resources0,
};

static int __init jill_dev0_module_init(void)
{
	int ret;

	JILL_PAINT_DEBUG("%s\n\n", __FUNCTION__);

	ret = platform_device_register(&jill_platform_device0);
	return ret;
}

static void __exit jill_dev0_module_exit(void)
{
	JILL_PAINT_DEBUG("%s\n\n", __FUNCTION__);

	platform_device_unregister(&jill_platform_device0);
}

module_init(jill_dev0_module_init);
module_exit(jill_dev0_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jill");
MODULE_DESCRIPTION("S3C24XX Jill device");
MODULE_ALIAS("platform:" JILL_DEVICE_NAME"-dev0");
