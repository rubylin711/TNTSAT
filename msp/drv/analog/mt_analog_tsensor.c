/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2025 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#if defined(__UBOOT__)
#include <common.h>
#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_irq.h>
#include <asm/arch-symphony6/mt_drv_analog.h>
#elif defined(__KERNEL__)
#include <linux/types.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/module.h>

#include "../crm/mt_log.h"
#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"

#include "mt_drv_analog.h"
#else
/* RTOS */
#include <string.h>

#include <sys_types.h>
#include <sys_define.h>
#include <mtos_task.h>
#include <mtos_misc.h>
#include <mtos_mutex.h>

#include "../crm/mt_log.h"
#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"

#include "mt_drv_analog.h"
#endif

#include "mt_analog_reg.h"

#if defined(__UBOOT__)
#define MUTEX_LOCK()			do{}while(0)
#define MUTEX_UNLOCK()			do{}while(0)
#define TEMP_MUTEX_LOCK()		do{}while(0)
#define TEMP_MUTEX_UNLOCK()		do{}while(0)
#elif defined(__KERNEL__)
/* fix multi thread issue of switch on/off temp sensor! */
//static DEFINE_SPINLOCK(analog_lock);
extern spinlock_t mt_analog_lock_k;
static unsigned long flags;
#define MUTEX_LOCK()			spin_lock_irqsave(&mt_analog_lock_k, flags)
#define MUTEX_UNLOCK()			spin_unlock_irqrestore(&mt_analog_lock_k, flags)

/* temp sensor mutex */
/* fix multi thread issue of switch on/off temp sensor! */
//DEFINE_MUTEX(temp_lock);
extern struct mutex mt_temp_lock_k;
#define TEMP_MUTEX_LOCK()		mutex_lock(&mt_temp_lock_k)
#define TEMP_MUTEX_UNLOCK()		mutex_unlock(&mt_temp_lock_k)
#else
/* RTOS */
extern void *analog_mutex;
#define MUTEX_LOCK()			do{ \
									if(analog_mutex) \
										mtos_mutex_take(analog_mutex); \
								}while(0)
#define MUTEX_UNLOCK()			do{ if(analog_mutex){mtos_mutex_give(analog_mutex);} }while(0)
static void *temp_mutex = NULL;
#define TEMP_MUTEX_LOCK()			do{ \
									if(temp_mutex == NULL) { \
										temp_mutex = mtos_mutex_create(1); \
									} \
									if(temp_mutex) \
										mtos_mutex_take(temp_mutex); \
								}while(0)
#define TEMP_MUTEX_UNLOCK()			do{ if(temp_mutex){mtos_mutex_give(temp_mutex);} }while(0)
#endif

#if defined(__UBOOT__)
#define TSENS_LOGW				printf
//#define TSENS_LOG				printf
#define TSENS_LOG(...)			do{}while(0)

#define msleep					mdelay
#elif defined(__KERNEL__)
#define TSENS_LOGW				printk
//#define TSENS_LOG				printk
#define TSENS_LOG(...)			do{}while(0)
#else
/* RTOS */
#define TSENS_LOGW				OS_PRINTF
//#define TSENS_LOG				OS_PRINTF
#define TSENS_LOG(...)			do{}while(0)

#define msleep					mtos_task_sleep
#define udelay					mtos_task_delay_us

#define CONFIG_MT_CHIP_SYMPHONY6
#endif

/*****************************************************************************/
//Debug
//#define DEBUG_TSENSOR

/* interrupt trig once? */
#define CFG_TSENSOR_INTR_ONCE

#ifndef __UBOOT__
#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
/* Trip Reset? */
#define CFG_TSENSOR_TRIP_RESET
#endif
#endif

/*****************************************************************************/

#define IRQ_NO_TSENSOR			(32 + 46)

#define	OFFSET_TH				0x5D00E4		/* threshold */
#define TH_LO_SHIFT				0
#define TH_HI_SHIFT				16
#define TH_WIDTH				13

#define	OFFSET_INT				0x5D00E8		/* interrupt */
#define INT_HI_EN_SHIFT			20
#define INT_LO_EN_SHIFT			16
#define INT_HI_MASK_SHIFT		13
#define INT_LO_MASK_SHIFT		12
#define INT_HI_CLR_SHIFT		9
#define INT_HI_CLR_MASK			(0x1 << INT_HI_CLR_SHIFT)
#define INT_LO_CLR_SHIFT		8
#define INT_LO_CLR_MASK			(0x1 << INT_LO_CLR_SHIFT)
#define INT_HI_MODE_SHIFT		5
#define INT_LO_MODE_SHIFT		4
#define INT_HI_STATE_SHIFT		1
#define INT_HI_STATE_MASK		(0x1 << INT_HI_STATE_SHIFT)
#define INT_LO_STATE_SHIFT		0
#define INT_LO_STATE_MASK		(0x1 << INT_LO_STATE_SHIFT)

#define	OFFSET_CFG				0x5D00EC

#define	OFFSET_DATA				0x5D00F0		/* data */

#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
#define	OFFSET_RST_TH			0x5D014C		/* reset request threshold */
#define TH_RST_SHIFT			0

#define OFFSET_RST_MASK			0x50FA04		/* reset request mask */
#define RST_MASK_SHIFT			16

#define OFFSET_RST_STATE		0x50FA0C		/* reset state(read only) */
#define RST_STATE_SHIFT			16
#define RST_STATE_MASK			(0x1 << RST_STATE_SHIFT)

#define OFFSET_RST_CLR			0x50FA08		/* reset state clear */
#define RST_STATE_CLR_SHIFT		16
#endif

/* Temp sensor factors */
struct mt_tsensor_factors
{
	int TEMP_MIN;		/* min temperature, e.g. sym6: -29802 */
	int TEMP_MAX;		/* max temperature, e.g. sym6: 49733 */

	/* temp_val = para_A * code / 8192 + para_B */
	unsigned int para_A;
	int para_B;
};

#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
/* Symphony6 Tsensor factors */
static struct mt_tsensor_factors sym6_tsensor_factor =
{
	-29802,
	49733,

	79545,
	-29802,
};

#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
/* Symphony4 Tsensor factors */
static struct mt_tsensor_factors sym4_tsensor_factor =
{
	-29000,
	49336,

	78346,
	-29000,
};

#elif defined(CONFIG_MT_CHIP_SYMPHONY2)
/* Symphony2 Tsensor factors */
static struct mt_tsensor_factors sym2_tsensor_factor =
{
	-28068,
	48095,

	76173,
	-28068,
};
#else
#error "Only Sym2/4/6 support Temp Sensor!"
#endif

/* low/high/reset trips */
struct mt_tensor_trip
{
	/* HW bit to enable the trip */
	unsigned int enable_offs;
	int enable_shift;

	/* HW bit to mask the trip interrupt */
	unsigned int mask_offs;
	int mask_shift;
	int mask_active_low;		/* 0: mask, 1: unmask ? */

	/* HW field to read the trip temperature */
	unsigned int reg_offs;
	int reg_shift;
	int reg_width;
};

/* trip index */
enum
{
	TRIP_INDEX_LOW,
	TRIP_INDEX_HIGH,
#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
	TRIP_INDEX_RESET,
#endif
	TRIP_INDEX_MAX,
};

/* low/high/reset trips */
static struct mt_tensor_trip tsensor_trips[] =
{
	/* 0: LOW */
	{
		.enable_offs 	= OFFSET_INT,
		.enable_shift 	= INT_LO_EN_SHIFT,
		.mask_offs 		= OFFSET_INT,
		.mask_shift 	= INT_LO_MASK_SHIFT,
		.mask_active_low = 1,
		.reg_offs 		= OFFSET_TH,
		.reg_shift 		= TH_LO_SHIFT,
		.reg_width 		= TH_WIDTH,
	},
	/* 1: HIGH */
	{
		.enable_offs	= OFFSET_INT,
		.enable_shift	= INT_HI_EN_SHIFT,
		.mask_offs		= OFFSET_INT,
		.mask_shift 	= INT_HI_MASK_SHIFT,
		.mask_active_low = 1,
		.reg_offs		= OFFSET_TH,
		.reg_shift		= TH_HI_SHIFT,
		.reg_width		= TH_WIDTH,
	},
#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
	/* 2: RESET */
	{
		.enable_offs	= 0,
		.enable_shift	= -1,
		.mask_offs		= OFFSET_RST_MASK,
		.mask_shift 	= RST_MASK_SHIFT,
		.mask_active_low = 0,
		.reg_offs		= OFFSET_RST_TH,
		.reg_shift		= TH_RST_SHIFT,
		.reg_width		= TH_WIDTH,
	},
#endif
};

/* Temp sensor device */
struct mt_tsensor_dev
{
	unsigned long reg_base;

	struct mt_tsensor_factors *pfactor;
};

#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
/* Symphony6 Tsensor device */
static struct mt_tsensor_dev sym6_tsensor_dev =
{
	.reg_base = IO_REG_BASE,
	.pfactor = &sym6_tsensor_factor,
};

#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
/* Symphony4 Tsensor device */
static struct mt_tsensor_dev sym4_tsensor_dev =
{
	.reg_base = IO_REG_BASE,
	.pfactor = &sym4_tsensor_factor,
};

#elif defined(CONFIG_MT_CHIP_SYMPHONY2)
/* Symphony2 Tsensor device */
static struct mt_tsensor_dev sym2_tsensor_dev =
{
	.reg_base = IO_REG_BASE,
	.pfactor = &sym2_tsensor_factor,
};
#endif

/* current active tsensor device */
#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
#define CURRENT_TS_DEV		(&sym6_tsensor_dev)
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
#define CURRENT_TS_DEV		(&sym4_tsensor_dev)
#elif defined(CONFIG_MT_CHIP_SYMPHONY2)
#define CURRENT_TS_DEV		(&sym2_tsensor_dev)
#endif

/* temperature log on/off */
int opt_temp_log = 0;
/* hack temperature return degree */
int opt_temp_hack_degree = 0;

/* record last temperature for async api */
static int last_temp_int = 0;
static int last_temp_dec = 0;

/* init flag */
static int init_flag = 0;
#if defined(__KERNEL__) && !defined(__UBOOT__)
static struct task_struct *tsensor_thread_id = NULL;
#endif

#ifdef DEBUG_TSENSOR
static int tsensor_trip_low = 50;
static int tsensor_trip_high = 80;
#ifdef CFG_TSENSOR_TRIP_RESET
#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
static int tsensor_trip_reset = 90;
#endif
#endif
#else
static int tsensor_trip_low = 0;
static int tsensor_trip_high = 118;
#ifdef CFG_TSENSOR_TRIP_RESET
#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
static int tsensor_trip_reset = 125;
#endif
#endif
#endif

#if defined(__KERNEL__) && !defined(__UBOOT__)
module_param(tsensor_trip_low, int, 0660);
module_param(tsensor_trip_high, int, 0660);
#ifdef CFG_TSENSOR_TRIP_RESET
#ifdef CONFIG_MT_CHIP_SYMPHONY6
module_param(tsensor_trip_reset, int, 0660);
#endif
#endif
#endif

static void dump_regs(void)
{
#ifdef DEBUG_TSENSOR
	void *reg_base = (void*)(CURRENT_TS_DEV->reg_base);

	reg_base = reg_base;	/* avoid compile warning */

	TSENS_LOGW("%X: %08X\n", OFFSET_TH, readl(reg_base + OFFSET_TH));
	TSENS_LOGW("%X: %08X\n", OFFSET_INT, readl(reg_base + OFFSET_INT));
	TSENS_LOGW("%X: %08X\n", OFFSET_CFG, readl(reg_base + OFFSET_CFG));
	TSENS_LOGW("%X: %08X\n", OFFSET_DATA, readl(reg_base + OFFSET_DATA));
#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
	TSENS_LOGW("%X: %08X\n", OFFSET_RST_TH, readl(reg_base + OFFSET_RST_TH));
	TSENS_LOGW("%X: %08X\n", OFFSET_RST_MASK, readl(reg_base + OFFSET_RST_MASK));
	TSENS_LOGW("%X: %08X\n", OFFSET_RST_STATE, readl(reg_base + OFFSET_RST_STATE));
#endif
#endif
}

/* Convert a HW code to a temperature reading (celsius * 100) */
static int code_to_temp(struct mt_tsensor_factors *pfactor, unsigned int code)
{
	code &= TSENSOR_DOUT_MASK;

	return (int)((pfactor->para_A * code) / 8192) + pfactor->para_B;
}

/* Convert a temperature value (celsius * 100) to a HW code */
static unsigned int temp_to_code(struct mt_tsensor_factors *pfactor, int temp)
{
	unsigned int code;

	if (temp < pfactor->TEMP_MIN)
	{
		temp = pfactor->TEMP_MIN;
	}

	if (temp > pfactor->TEMP_MAX)
	{
		temp = pfactor->TEMP_MAX;
	}

	code = (unsigned int)(temp - pfactor->para_B) * 8192 / pfactor->para_A;

	return (code & TSENSOR_DOUT_MASK);
}

/*
 * type: TRIP_INDEX_LOW, TRIP_INDEX_HIGH, TRIP_INDEX_RESET
 */
static void set_trip_enable(struct mt_tsensor_dev *pdev, int type, int enable)
{
	TSENS_LOG("%s: type %d, enable %d\n", __FUNCTION__, type, enable);

	if (tsensor_trips[type].enable_shift != -1)
	{
		if (enable)
			MT_SET_BIT(pdev->reg_base + tsensor_trips[type].enable_offs, tsensor_trips[type].enable_shift, 0x1);
		else
			MT_SET_BIT(pdev->reg_base + tsensor_trips[type].enable_offs, tsensor_trips[type].enable_shift, 0);
	}
}

/*
 * type: TRIP_INDEX_LOW, TRIP_INDEX_HIGH, TRIP_INDEX_RESET
 */
static void set_trip_mask(struct mt_tsensor_dev *pdev, int type, int mask)
{
	TSENS_LOG("%s: type %d, mask %d\n", __FUNCTION__, type, mask);

	if (tsensor_trips[type].mask_shift != -1)
	{
		if (tsensor_trips[type].mask_active_low)
		{
			/* Caution: 0 - mask, 1 - unmask */
			if (mask)
				MT_SET_BIT(pdev->reg_base + tsensor_trips[type].mask_offs, tsensor_trips[type].mask_shift, 0);
			else
				MT_SET_BIT(pdev->reg_base + tsensor_trips[type].mask_offs, tsensor_trips[type].mask_shift, 0x1);
		}
		else
		{
			if (mask)
				MT_SET_BIT(pdev->reg_base + tsensor_trips[type].mask_offs, tsensor_trips[type].mask_shift, 0x1);
			else
				MT_SET_BIT(pdev->reg_base + tsensor_trips[type].mask_offs, tsensor_trips[type].mask_shift, 0);
		}
	}
}

/* set high/low interrupt and reset threshold temperature */
static void set_trip_temp(struct mt_tsensor_dev *pdev, int type, int temp)
{
	unsigned int code;

	code = temp_to_code(pdev->pfactor, temp * 100/* e.g. 50 -> 5000 */);

	TSENS_LOG("%s: type %d, temp %d, code 0x%X\n", __FUNCTION__, type, temp, code);

	MT_SET_BITS(pdev->reg_base + tsensor_trips[type].reg_offs,
		tsensor_trips[type].reg_shift,
		tsensor_trips[type].reg_width,
		code);
}

#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
/* check and clear reset state */
static void check_clear_reset_state(void)
{
	unsigned val;
	struct mt_tsensor_dev *pdev = CURRENT_TS_DEV;

	val = readl((void*)(pdev->reg_base + OFFSET_RST_STATE));

	if ((val & RST_STATE_MASK) != 0)
	{
		TSENS_LOGW("TSensor Rststate: 0x%X\n", val);

		dump_regs();

		//mask
		MT_SET_BIT(pdev->reg_base + OFFSET_RST_MASK, RST_MASK_SHIFT, 1);

		//clear
		MT_SET_BIT(pdev->reg_base + OFFSET_RST_CLR, RST_STATE_CLR_SHIFT, 1);

/* not work */
#if 0
		mt_analog_reset(MT_ANA_TSENSOR);
		udelay(CFG_MT_ANA_RESET_UDELAY_MAX);
		mt_analog_release(MT_ANA_TSENSOR);
		udelay(5);
#endif

		TSENS_LOGW("After Clear:\n");
		dump_regs();
	}
}
#endif

#if defined(__KERNEL__) || defined(__UBOOT__)
static irqreturn_t tsensor_irq_handler(int irq, void *data)
{
	unsigned long reg_addr;
	unsigned int_state;
	unsigned int_clr;
	unsigned tdata;

	struct mt_tsensor_dev *pdev = (struct mt_tsensor_dev*)data;

	reg_addr = pdev->reg_base + OFFSET_DATA;
	tdata = readl((void*)reg_addr);

	reg_addr = pdev->reg_base + OFFSET_INT;
	int_state = readl((void*)reg_addr);

	TSENS_LOGW("%s: int state 0x%x, data 0x%x\n", __FUNCTION__, int_state, tdata);

	if ((int_state & INT_LO_STATE_MASK) != 0)
	{
		/* clear int state */
		int_clr = int_state | INT_LO_CLR_MASK;
		writel(int_clr, (void*)reg_addr);

		int_clr &= ~INT_LO_CLR_MASK;
		writel(int_clr, (void*)reg_addr);

		set_trip_enable(pdev, TRIP_INDEX_LOW, 0);
		//set_trip_mask(pdev, TRIP_INDEX_LOW, 0x1);

		TSENS_LOGW("%s: temp(%d) vs low temp th(%d)\n", __FUNCTION__,
			code_to_temp(pdev->pfactor, tdata) / 100,
			tsensor_trip_low);

#ifndef CFG_TSENSOR_INTR_ONCE
		//set_trip_mask(pdev, TRIP_INDEX_LOW, 0);
		set_trip_enable(pdev, TRIP_INDEX_LOW, 1);
#endif
	}
	else if ((int_state & INT_HI_STATE_MASK) != 0)
	{
		/* clear int state */
		int_clr = int_state | INT_HI_CLR_MASK;
		writel(int_clr, (void*)reg_addr);

		int_clr &= ~INT_HI_CLR_MASK;
		writel(int_clr, (void*)reg_addr);

		set_trip_enable(pdev, TRIP_INDEX_HIGH, 0);
		//set_trip_mask(pdev, TRIP_INDEX_HIGH, 0x1);

		TSENS_LOGW("%s: temp(%d) vs high temp th(%d)\n", __FUNCTION__,
			code_to_temp(pdev->pfactor, tdata) / 100,
			tsensor_trip_high);

#ifndef CFG_TSENSOR_INTR_ONCE
		//set_trip_mask(pdev, TRIP_INDEX_HIGH, 0);
		set_trip_enable(pdev, TRIP_INDEX_HIGH, 1);
#endif
	}
	else
	{
		MT_LOGW("unknown interrupt: irq %d, state 0x%x!\n", irq, int_state);
	}

	return IRQ_HANDLED;
}
#endif

/*****************************************************************************/

int mt_tsensor_set_trip_temp(int which, int temp)
{
	if (which < 0 || which >= TRIP_INDEX_MAX)
	{
		MT_LOGE("invalid tsensor trip type %d!\n", which);
		return (-1);
	}

	MUTEX_LOCK();
	set_trip_temp(CURRENT_TS_DEV, which, temp);
	MUTEX_UNLOCK();

	return 0;
}

int mt_tsensor_set_trip_enable(int which, int enable)
{
	if (which < 0 || which >= TRIP_INDEX_MAX)
	{
		MT_LOGE("invalid tsensor trip type %d!\n", which);
		return (-1);
	}

	MUTEX_LOCK();

	set_trip_mask(CURRENT_TS_DEV, which, enable==0?1:0);
	set_trip_enable(CURRENT_TS_DEV, which, enable);

	MUTEX_UNLOCK();

	return 0;
}

#if defined(__KERNEL__) && !defined(__UBOOT__)
/* sync read temperature 1s interval */
static int tsensor_kthread(void *data)
{
	int temp_int = 0, temp_dec = 0;
	int trip_en = 0;
#ifdef CFG_TSENSOR_TRIP_RESET
#ifdef CONFIG_MT_CHIP_SYMPHONY6
	int trip_rst_en = 0;
#endif
#endif
	unsigned long n = 0;

	msleep(200);
	mt_analog_get_temperature(&temp_int, &temp_dec);

	while (!kthread_should_stop())
	{
		msleep(1000);

		n ++;

		mt_analog_get_temperature(&temp_int, &temp_dec);

		if ((temp_int < tsensor_trip_high)
			&& (temp_int > tsensor_trip_low)
			&& (trip_en == 0)) {
			TSENS_LOGW("%s: (%lu)temp %d.%02d\n", __FUNCTION__,
				n,
				temp_int,
				temp_dec<0?-temp_dec:temp_dec);

			TSENS_LOG("%s: (%lu)trip late enable\n", __FUNCTION__, n);

			mt_tsensor_set_trip_enable(TRIP_INDEX_LOW, 1);
			mt_tsensor_set_trip_enable(TRIP_INDEX_HIGH, 1);

			trip_en = 1;
			continue;
		} else {
			TSENS_LOG("%s: (%lu)temp %d.%02d\n", __FUNCTION__,
				n,
				temp_int,
				temp_dec<0?-temp_dec:temp_dec);
		}

		/* FIXME: need wait 3-5s to enable RESET trip??? */
#ifdef CFG_TSENSOR_TRIP_RESET
#ifdef CONFIG_MT_CHIP_SYMPHONY6
		/* check if "tsensor_trip_reset" legal */
		if ((tsensor_trip_reset < tsensor_trip_high)
			|| (tsensor_trip_reset == 500))
		{
			continue;
		}

		if (((temp_int + 10) < tsensor_trip_reset)
			&& (trip_rst_en == 0)) {
			TSENS_LOG("%s: (%lu)trip reset enable\n", __FUNCTION__, n);

			mt_tsensor_set_trip_temp(TRIP_INDEX_RESET, tsensor_trip_reset);
			mt_tsensor_set_trip_enable(TRIP_INDEX_RESET, 1);

			trip_rst_en = 1;
		}
#endif
#endif
	}

	return 0;
}
#endif

int mt_tsensor_init(void)
{
	int inited = 1;
	int status = 0;

	MUTEX_LOCK();
	inited = init_flag;
	MUTEX_UNLOCK();

	if (inited == 0)
	{
		TSENS_LOG("TSensor init...\n");

#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
		check_clear_reset_state();
#endif

		mt_analog_get_status(MT_ANA_TSENSOR, (u32*)&status);
		if (status == 0)
		{
			TSENS_LOGW("TSensor reset and enable...\n");

			mt_analog_reset(MT_ANA_TSENSOR);
			mt_analog_enable(MT_ANA_TSENSOR);
			udelay(CFG_MT_ANA_RESET_UDELAY_MAX);
			mt_analog_release(MT_ANA_TSENSOR);
			udelay(5);
		}

#if defined(__UBOOT__) || defined(__KERNEL__)
		mt_tsensor_set_trip_temp(TRIP_INDEX_LOW, tsensor_trip_low);
		mt_tsensor_set_trip_temp(TRIP_INDEX_HIGH, tsensor_trip_high);

		request_irq(IRQ_NO_TSENSOR, tsensor_irq_handler, IRQF_TRIGGER_HIGH, "tsensor", CURRENT_TS_DEV);
#else
/* RTOS */
		//TODO:
#endif

#if defined(__KERNEL__) && !defined(__UBOOT__)
		/* kernel */
		tsensor_thread_id = kthread_run(tsensor_kthread, NULL, "tsensor_kthread");
#else
		/* RTOS or Uboot */
		if (1) {

			/* FIXME: after enable MT_ANA_TSENSOR, should wait some time! */
			if (status == 0)
				msleep(200);

#if defined(__UBOOT__)
			mt_tsensor_set_trip_enable(TRIP_INDEX_LOW, 1);
			mt_tsensor_set_trip_enable(TRIP_INDEX_HIGH, 1);
#endif

#ifdef CFG_TSENSOR_TRIP_RESET
#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
			/* Wait Loop: temp < 120 */
			do
			{
				int temp_int = 0, temp_dec = 0;

				mt_analog_get_temperature(&temp_int, &temp_dec);

				if (temp_int + 10 >= tsensor_trip_reset)
				{
					TSENS_LOGW("%s: temp(%d) is too high >= %d\n", __FUNCTION__, temp_int, tsensor_trip_reset-10);
					msleep(1000);
					continue;
				}
				else
				{
					/* check if "tsensor_trip_reset" legal */
					if ((tsensor_trip_reset >= tsensor_trip_high)
						&& (tsensor_trip_reset != 500))
					{
						/* FIXME: need wait 3-5s to enable RESET trip??? */
						//msleep(3000);

						mt_tsensor_set_trip_temp(TRIP_INDEX_RESET, tsensor_trip_reset);
						mt_tsensor_set_trip_enable(TRIP_INDEX_RESET, 1);
					}

					break;
				}

			} while (1);
#endif
#endif
		}
#endif

		MUTEX_LOCK();
		init_flag = 1;
		MUTEX_UNLOCK();

		TSENS_LOG("TSensor inited.\n");
	}

	return 0;
}

int mt_tsensor_deinit(void)
{
	int inited = 0;

	MUTEX_LOCK();
	inited = init_flag;
	MUTEX_UNLOCK();

	if (inited)
	{
		TSENS_LOG("TSensor deinit...\n");

#if defined(__KERNEL__) && !defined(__UBOOT__)
		if (!IS_ERR(tsensor_thread_id))
		{
			kthread_stop(tsensor_thread_id);
			msleep(10);
			tsensor_thread_id = NULL;
		}
#endif

		mt_tsensor_set_trip_enable(TRIP_INDEX_LOW, 0);
		mt_tsensor_set_trip_enable(TRIP_INDEX_HIGH, 0);

#ifdef CFG_TSENSOR_TRIP_RESET
#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_ARCH_SYMPHONY6)
		mt_tsensor_set_trip_enable(TRIP_INDEX_RESET, 0);
#endif
#endif

#if defined(__UBOOT__) || defined(__KERNEL__)
		free_irq(IRQ_NO_TSENSOR, CURRENT_TS_DEV);
#else
/* RTOS */
		//TODO:
#endif

		MUTEX_LOCK();
		init_flag = 0;
		MUTEX_UNLOCK();

		TSENS_LOG("TSensor deinited.\n");
	}

	return 0;
}

static void get_temp(struct mt_tsensor_dev *pdev, int *temp_int, int *temp_dec)
{
	u32 value, reg_val;
	int temp_val;

	int retry = 5;
	int retry_interval = 10;	//10ms

Retry:
	//check if Tsensor is ON?
	value = MT_GET_BIT(ANA_AO_REG0, TSENSOR_PD_SHIFT);
	if (value != 0)
	{
		//BUG!
		MT_BUG("[BUG]Tsensor is power down while call get temperature!!!\n");
	}

	reg_val= MT_IO_READ32(pdev->reg_base + OFFSET_DATA);
	value = reg_val & TSENSOR_DOUT_MASK;

	if (value == 0)
	{
		MT_LOGW("Tsensor Reg = 0!\n");

		if (retry--)
		{
			msleep(retry_interval);
			goto Retry;
		}
	}

	temp_val = code_to_temp(pdev->pfactor, value);

	if (temp_int)
		*temp_int = temp_val / 100;

	if (temp_dec)
		*temp_dec = temp_val % 100;

	//debug
	if (temp_int != NULL && temp_dec != NULL)
	{
		if (opt_temp_log)
		{
			MT_LOGE("%X: 0x%X -> %d.%02d\r\n", OFFSET_DATA, reg_val,
				*temp_int,
				*temp_dec < 0? -(*temp_dec) : (*temp_dec)
				);
		}
		else
		{
			MT_LOGD("%X: 0x%X -> %d.%02d\r\n", OFFSET_DATA, reg_val,
				*temp_int,
				*temp_dec < 0? -(*temp_dec) : (*temp_dec)
				);
		}

		if (opt_temp_hack_degree)
		{
			*temp_int = opt_temp_hack_degree;
			*temp_dec = 0;
		}
	}
}

void mt_analog_get_temperature(int *temp_int, int *temp_dec)
{
	int ret;
	u32 status = 0;
	int sleep_time = 100;

	TEMP_MUTEX_LOCK();

	ret = mt_analog_get_status(MT_ANA_TSENSOR, &status);

	if (ret != 0 || status == 0) {
		//debug
		MT_LOGD("Enable Tsensor\r\n");

		mt_analog_enable(MT_ANA_TSENSOR);

		//debug
		MT_LOGD("Sleep %dms\r\n", sleep_time);

		msleep(sleep_time);
	}

	get_temp(CURRENT_TS_DEV, temp_int, temp_dec);

/* tsensor: always on */
#if 0
	//debug
	MT_LOGD("Disable Tsensor\r\n");

	(void)mt_analog_disable(MT_ANA_TSENSOR);
#endif

	TEMP_MUTEX_UNLOCK();

	/* record it for async api */
	MUTEX_LOCK();
	if (temp_int)
		last_temp_int = *temp_int;
	if (temp_dec)
		last_temp_dec = *temp_dec;
	MUTEX_UNLOCK();
}

void mt_analog_get_temperature_async(int *temp_int, int *temp_dec)
{
	MUTEX_LOCK();
	if (temp_int)
		*temp_int = last_temp_int;
	if (temp_dec)
		*temp_dec = last_temp_dec;
	MUTEX_UNLOCK();
}

#if defined(__KERNEL__) && !defined(__UBOOT__)
EXPORT_SYMBOL(mt_analog_get_temperature);
EXPORT_SYMBOL(mt_analog_get_temperature_async);
#endif

