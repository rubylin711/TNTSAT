/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/platform_device.h>
#include "jill.h"

static struct resource jill_resources1[] = {
	/*
	* jill_resources1 must not be equal with jill_resources0,
	* because insert_resource will check conflict.
	* if jill_resources1 be equal with jill_resources0,
	* modprobe jill-dev0
	* modprobe jill-dev1
	* rmmod jill-dev1
	* rmmod jill-dev0
	* these order will crash, the log : Modules linked in: jill_dev0(-) jill [last unloaded: jill_dev1]
	* we can only modprobe jill-dev0; modprobe jill-dev1; rmmod jill-dev0; rmmod jill-dev1,
	* if jill_resources1 not be equal with jill_resources0, we no need follow the order.
	*/
};

static struct jill_platform_data jill_platdata1 = {
	.seconds = 10,
	.nano_seconds = 100,
	.gpf_con_and = 0xc03c,
	.gpf_con_or = 0x1542,
	.gpf_up_or = 0x79,
	/* high level: quench		low level: lighten */
	.default_value = 0x0e,
};

static void jill_release_device1(struct device *pdev)
{
	/* if platform_device is dynamic alloc, we can free platform_device here. */
	JILL_PAINT_DEBUG("%s\n\n", __FUNCTION__);
}

static struct platform_device jill_platform_device1 = {
	.name = JILL_DEVICE_NAME,
	.id = 1,
	.dev= {
		.platform_data = (void *)(&jill_platdata1),
		.release = jill_release_device1,
	},
	.num_resources= ARRAY_SIZE(jill_resources1),
	.resource = jill_resources1,
};

static int __init jill_dev1_module_init(void)
{
	int ret;

	JILL_PAINT_DEBUG("%s\n\n", __FUNCTION__);

	ret = platform_device_register(&jill_platform_device1);
	return ret;
}

static void __exit jill_dev1_module_exit(void)
{
	JILL_PAINT_DEBUG("%s\n\n", __FUNCTION__);

	platform_device_unregister(&jill_platform_device1);
}

module_init(jill_dev1_module_init);
module_exit(jill_dev1_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jill");
MODULE_DESCRIPTION("S3C24XX Jill device");
MODULE_ALIAS("platform:" JILL_DEVICE_NAME"-dev1");

