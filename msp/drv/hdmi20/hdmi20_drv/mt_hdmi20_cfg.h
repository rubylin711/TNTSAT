/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _MT_HDMI20_CFG_
#define _MT_HDMI20_CFG_

#define __HDMI_OS_LINUX__	(0)    //Linux user space
#define __HDMI_OS_KERNEL__	(1)      //Linux Kernel space
#define __HDMI_OS_RTOS__	(0)		//RTOS
#define __HDMI_UBOOT__	(0)			//UBOOT

#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__)
#define	SI_FLOATPOINT_SUPPORT (1)
//#define	SI_COM_ALL_PASS (1)
#define SI_HDMI20_PRINT	printf
#define SI_LOG_LEVEL_STRING
#if 1
#define MT_FLOAT_DIV(type,a,b)					({ \
		type temp; \
		temp = (type)a; \
		temp = temp/b; \
		temp;\
	})
#define MT_FLOAT_MULTI(type,a,b,factor)		({ \
		type temp; \
		temp = (type)a * (type)b; \
		temp = temp/(type)factor; \
		temp;\
	})
#endif
#else
#define	SI_FLOATPOINT_SUPPORT (0)
//#define	SI_COM_ALL_PASS (0)
//#define SI_HDMI20_PRINT	printk
#define SI_HDMI20_PRINT(...) do{}while(0)
#define SI_LOG_LEVEL_STRING	KERN_CONT
#define MT_FLOAT_DIV(type,a,b)					({ \
		type temp; \
		temp = (type)a; \
		do_div(temp, (type)b); \
		temp;\
	})
#define MT_FLOAT_MULTI(type,a,b,factor)		({ \
		type temp; \
		temp = (type)a * (type)b; \
		do_div(temp, (type)factor); \
		temp;\
	})
#endif
#define SII_ENV_BUILD_UFD		(0)
#define	SI_DRV_DINO_CFG			(0)
#define TPG_RECOVERY_UNUSED_FUNC		(0)
#define DVI_SUPPORT
#define MT_FLOAT_FACTOR_1000		(1000)
#define MT_FLOAT_FACTOR_1000000		(1000000)
#endif
