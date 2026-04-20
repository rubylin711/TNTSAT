/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Montage LZ Co., Ltd.
 */
#if defined(__UBOOT__)
#include <common.h>
#include <command.h>

#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_analog.h>

#include "mt_crm_clock_reg.h"
#include "mt_mod_reset.h"

#elif defined(__KERNEL__)

#include <linux/types.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/delay.h>

/* FIXME */
#include "../crm/mt_log.h"
#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"
#include "../crm/mt_crm_clock_reg.h"
#include "../crm/mt_mod_reset.h"

#include "mt_drv_analog.h"
#include "mt_drv_clock.h"

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
#include "../crm/mt_crm_clock_reg.h"
#include "../crm/mt_mod_reset.h"

#include "mt_drv_analog.h"
#include "mt_drv_clock.h"
#endif

#include "mt_analog_reg.h"
#include "mt_analog_dev.h"

#if defined(__UBOOT__)
#include <asm/arch-symphony6/mt_analog_adc.h>
#include <asm/arch-symphony6/mt_analog_adac.h>
#include <asm/arch-symphony6/mt_analog_vdac.h>
#include <asm/arch-symphony6/mt_analog_vxdssc.h>
#include <asm/arch-symphony6/mt_analog_ephy.h>
#include <asm/arch-symphony6/mt_analog_hdmi.h>
#include <asm/arch-symphony6/mt_analog_usbaclk.h>
#include <asm/arch-symphony6/mt_analog_usb.h>
#include <asm/arch-symphony6/mt_analog_usb3p0.h>
#include <asm/arch-symphony6/mt_analog_bias.h>
#include <asm/arch-symphony6/mt_analog_otpreg25.h>
#include <asm/arch-symphony6/mt_analog_osc.h>
#include <asm/arch-symphony6/mt_analog_tsensor.h>
#include <asm/arch-symphony6/mt_analog_pmon.h>
#include <asm/arch-symphony6/mt_analog_pll.h>
#elif defined(__KERNEL__)
#include "analog/mt_analog_adc.h"
#include "analog/mt_analog_adac.h"
#include "analog/mt_analog_vdac.h"
#include "analog/mt_analog_vxdssc.h"
#include "analog/mt_analog_ephy.h"
#include "analog/mt_analog_hdmi.h"
#include "analog/mt_analog_usbaclk.h"
#include "analog/mt_analog_usb.h"
#include "analog/mt_analog_usb3p0.h"
#include "analog/mt_analog_bias.h"
#include "analog/mt_analog_otpreg25.h"
#include "analog/mt_analog_osc.h"
#include "analog/mt_analog_tsensor.h"
#include "analog/mt_analog_pmon.h"
#include "analog/mt_analog_pll.h"
#else
/* RTOS */
#include "analog/mt_analog_adc.h"
#include "analog/mt_analog_adac.h"
#include "analog/mt_analog_vdac.h"
#include "analog/mt_analog_vxdssc.h"
#include "analog/mt_analog_ephy.h"
#include "analog/mt_analog_hdmi.h"
#include "analog/mt_analog_usbaclk.h"
#include "analog/mt_analog_usb.h"
#include "analog/mt_analog_usb3p0.h"
#include "analog/mt_analog_bias.h"
#include "analog/mt_analog_otpreg25.h"
#include "analog/mt_analog_osc.h"
#include "analog/mt_analog_tsensor.h"
#include "analog/mt_analog_pmon.h"
#include "analog/mt_analog_pll.h"
#endif

#if defined(__UBOOT__)
#define MUTEX_LOCK()			do{}while(0)
#define MUTEX_UNLOCK()			do{}while(0)
#elif defined(__KERNEL__)
/* fix multi thread issue of switch on/off temp sensor! */
//static DEFINE_SPINLOCK(analog_lock);
extern spinlock_t mt_analog_lock_k;
static unsigned long flags;
#define MUTEX_LOCK()			spin_lock_irqsave(&mt_analog_lock_k, flags)
#define MUTEX_UNLOCK()			spin_unlock_irqrestore(&mt_analog_lock_k, flags)

static atomic_t analog_atomic = ATOMIC_INIT(0);

#else
/* RTOS */
void *analog_mutex = NULL;
#define MUTEX_LOCK()			do{ \
									if(analog_mutex == NULL) { \
										analog_mutex = mtos_mutex_create(1); \
									} \
									if(analog_mutex) \
										mtos_mutex_take(analog_mutex); \
								}while(0)
#define MUTEX_UNLOCK()			do{ if(analog_mutex){mtos_mutex_give(analog_mutex);} }while(0)
#endif

static struct mt_ana_dev *get_ana_by_name(const char *name)
{
	u32 i;

	if (name == NULL || strlen(name) == 0)
	{
		MT_LOGE("[E]%s: invalid name!\n", __FUNCTION__);
		return NULL;
	}

	for (i=0; i<mt_ana_table_size; i++)
	{
		if (strcmp(name, mt_ana_table[i].name) == 0)
		{
			MT_LOGD("%s: found %s -> %u\n", __FUNCTION__, name, i);
			return &mt_ana_table[i];
		}
	}

	MT_LOGW("%s: not found %s!\n", __FUNCTION__, name);
	return NULL;
}

#define DEF_GET_CHK_ANA(name)	struct mt_ana_dev *pAna; \
								do { pAna = get_ana_by_name(name); \
									if (pAna == NULL) \
										return (-1); \
								} while(0)

#define IDX2REG(idx)			((IO_REG_BASE) + (idx))

int mt_analog_acquire_lock(void)
{
	MUTEX_LOCK();

#if defined(__KERNEL__) && !defined(__UBOOT__)
	atomic_inc(&analog_atomic);
#endif

	return 0;
}

int mt_analog_release_lock(void)
{
	/* check lock */
#if defined(__KERNEL__) && !defined(__UBOOT__)
	if (atomic_read(&analog_atomic) == 0) {
		BUG();
		return (-1);
	}
	atomic_dec(&analog_atomic);
#endif

	MUTEX_UNLOCK();

	return 0;
}

int mt_analog_get_attr(enum MT_ANALOG_INDEX_E index, void *attr)
{
	//FIXME
	//CHECK_ID(index);
	CHECK_NULL_PTR(attr);

	MUTEX_LOCK();
	*(u32*)attr = MT_IO_READ32(IDX2REG(index));
	MUTEX_UNLOCK();

	return 0;
}

int mt_analog_get_attr_no_lock(enum MT_ANALOG_INDEX_E index, void *attr)
{
	//FIXME
	//CHECK_ID(index);
	CHECK_NULL_PTR(attr);

	/* check lock */
#if defined(__KERNEL__) && !defined(__UBOOT__)
	if (atomic_read(&analog_atomic) == 0) {
		BUG();
		return (-1);
	}
#endif

	*(u32*)attr = MT_IO_READ32(IDX2REG(index));

	return 0;
}

int mt_analog_get_attr_lock(enum MT_ANALOG_INDEX_E index, void *attr)
{
	(void)mt_analog_acquire_lock();

	return mt_analog_get_attr_no_lock(index, attr);
}

int mt_analog_set_attr_no_lock(enum MT_ANALOG_INDEX_E index, void *attr)
{
	int ret;

	//FIXME
	//CHECK_ID(index);
	CHECK_NULL_PTR(attr);

	/* check lock */
#if defined(__KERNEL__) && !defined(__UBOOT__)
	if (atomic_read(&analog_atomic) == 0) {
		BUG();
		return (-1);
	}
#endif

	ret = MT_IO_WRITE32(IDX2REG(index), *(u32*)attr);

	return ret;
}

int mt_analog_set_attr_unlock(enum MT_ANALOG_INDEX_E index, void *attr)
{
	int ret;

	//FIXME
	//CHECK_ID(index);
	CHECK_NULL_PTR(attr);

	/* check lock */
#if defined(__KERNEL__) && !defined(__UBOOT__)
	if (atomic_read(&analog_atomic) == 0) {
		BUG();
		return (-1);
	}
#endif

	ret = MT_IO_WRITE32(IDX2REG(index), *(u32*)attr);

	(void)mt_analog_release_lock();

	return ret;
}

int _analog_get_status(struct mt_ana_dev *pAna, u32 *status)
{
	int ret = -1;

	if (VALID_IO_OBJ((mt_io_obj_t*)&pAna->stat))
	{
		*status = mt_read_obj((mt_io_obj_t*)&pAna->stat);

		if (pAna->power_down)
		{
			/*
			 * e.g. ADAC
			 *       all '11111111' - Off
			 *   not all '11111111' - On
			 */
			//*status = (!*status) & MASK32[pAna->stat.width-1];
			*status = (~(*status)) & MASK32[pAna->stat.width-1];
		}

		ret = 0;
	}
	else
	{
		MT_LOGE("%s: %s has no pd/en bits!\n", __FUNCTION__, pAna->name);
	}

	return ret;
}

int mt_analog_get_status(mt_ana_t *ana, u32 *status)
{
	int ret;

	DEF_GET_CHK_ANA(ana);

	CHECK_NULL_PTR(ana);
	CHECK_NULL_PTR(status);

	MT_LOGD("%s: %s\n", __FUNCTION__, ana);

	MUTEX_LOCK();
	ret = _analog_get_status(pAna, status);
	MUTEX_UNLOCK();

	return ret;
}

int _analog_enable(struct mt_ana_dev *pAna)
{
	int ret = -1;

	if (VALID_IO_OBJ((mt_io_obj_t*)&pAna->stat))
	{
		if (pAna->power_down)
		{
			ret = mt_write_objlk(&pAna->stat, 0);
		}
		else
		{
			ret = mt_write_objlk(&pAna->stat, MASK32[pAna->stat.width-1]);
		}
	}
	else
	{
		MT_LOGE("%s: %s has no pd/en bits!\n", __FUNCTION__, pAna->name);
	}

	return ret;
}

int mt_analog_enable(mt_ana_t *ana)
{
	int ret;

	DEF_GET_CHK_ANA(ana);

	CHECK_NULL_PTR(ana);

	MT_LOGD("%s: %s\n", __FUNCTION__, ana);

	MUTEX_LOCK();
	ret = _analog_enable(pAna);
	MUTEX_UNLOCK();

	return ret;
}

int _analog_disable(struct mt_ana_dev *pAna)
{
	int ret = -1;

	if (VALID_IO_OBJ((mt_io_obj_t*)&pAna->stat))
	{
		if (pAna->power_down)
		{
			ret = mt_write_objlk(&pAna->stat, MASK32[pAna->stat.width-1]);
		}
		else
		{
			ret = mt_write_objlk(&pAna->stat, 0);
		}
	}
	else
	{
		MT_LOGE("%s: %s has no pd/en bits!\n", __FUNCTION__, pAna->name);
	}

	return ret;
}

int mt_analog_disable(mt_ana_t *ana)
{
	int ret;

	DEF_GET_CHK_ANA(ana);

	CHECK_NULL_PTR(ana);

	MT_LOGD("%s: %s\n", __FUNCTION__, ana);

	MUTEX_LOCK();
	ret = _analog_disable(pAna);
	MUTEX_UNLOCK();

	return ret;
}

int mt_analog_reset(mt_ana_t *ana)
{
	int ret;

	CHECK_NULL_PTR(ana);

	MUTEX_LOCK();
	ret = mt_mod_rst_reset((mt_mod_t*)ana);
	MUTEX_UNLOCK();

	return ret;
}

int mt_analog_release(mt_ana_t *ana)
{
	int ret;

	CHECK_NULL_PTR(ana);

	MUTEX_LOCK();
	ret = mt_mod_rst_release((mt_mod_t*)ana);
	MUTEX_UNLOCK();

	return ret;
}

int mt_analog_get_rate(mt_ana_t *ana, unsigned long *rate)
{
	int ret;

	CHECK_NULL_PTR(ana);
	CHECK_NULL_PTR(rate);

	MUTEX_LOCK();
	ret = mt_clk_get_rate((mt_clk_t*)ana, rate);
	MUTEX_UNLOCK();

	return ret;
}

int mt_analog_set_rate(mt_ana_t *ana, unsigned long rate)
{
	int ret;

	CHECK_NULL_PTR(ana);

	MUTEX_LOCK();
	ret = mt_clk_set_rate((mt_clk_t*)ana, rate);
	MUTEX_UNLOCK();

	return ret;
}

#if defined(__KERNEL__) && !defined(__UBOOT__)
void mt_analog_suspend(void)
{
	int i;
	int count = (int)suspend_analog_regs_size;

	MT_LOGD("%s: Enter\n", __FUNCTION__);

	MUTEX_LOCK();

	for (i=0; i<count; i++)
	{
		suspend_analog_regs[i].value = MT_IO_READ32(suspend_analog_regs[i].reg);
		MT_LOGD("%lX: %08X\n", suspend_analog_regs[i].reg, suspend_analog_regs[i].value);
	}

	MUTEX_UNLOCK();
}

void mt_analog_resume(void)
{
	int i;
	int count = (int)suspend_analog_regs_size;

	MT_LOGD("%s: Enter\n", __FUNCTION__);

	MUTEX_LOCK();

	for (i=0; i<count; i++)
	{
		MT_LOGD("%lX: %08X\n", suspend_analog_regs[i].reg, suspend_analog_regs[i].value);
		MT_IO_WRITE32(suspend_analog_regs[i].reg, suspend_analog_regs[i].value);
	}

	MUTEX_UNLOCK();
}

EXPORT_SYMBOL(mt_analog_get_status);
EXPORT_SYMBOL(mt_analog_enable);
EXPORT_SYMBOL(mt_analog_disable);
#endif

#if !defined(CONFIG_TARGET_SYMPHONY6_MINI) && !defined(CONFIG_TARGET_SYMPHONY6_LITE)

#if defined(__UBOOT__)
void mt_analog_dump_status(void)
#elif defined(__KERNEL__)
void mt_analog_dump_status(struct seq_file *s)
#else
/* RTOS */
void mt_analog_dump_status(int (*DP_LOG)(const char *fmt, ...))
#endif
{
	u32 i;
	u32 status = 0;
	int rst_stat = -1;

	DP_LOG("----------------------ANALOG---------------------\n");
	DP_LOG("%18s %4s %s\n", "NAME", "STAT", "RESET");
	for (i=0; i<mt_ana_table_size; i++)
	{
		mt_analog_get_status((mt_ana_t*)mt_ana_table[i].name, &status);
		mt_analog_get_reset_status((mt_ana_t*)mt_ana_table[i].name, &rst_stat);
		DP_LOG("%17s: %4s %s\n", mt_ana_table[i].name,
			status==0?"OFF":"ON",
			(rst_stat<0)?"NA":(rst_stat==0)?"Release":"Reset");
	}
	//DP_LOG("-------------------------------------------------\n");
}

#if defined(__UBOOT__)
void mt_analog_dump_state(void)
#elif defined(__KERNEL__)
void mt_analog_dump_state(struct seq_file *s)
#else
/* RTOS */
void mt_analog_dump_state(void)
#endif
{
	struct mt_analog_ao_r0_attr ao_r0_attr;
	struct mt_analog_ao_r1_attr ao_r1_attr;
	struct mt_analog_top_r0_attr top_r0_attr;
	struct mt_analog_top_r1_attr top_r1_attr;
	struct mt_analog_top_r2_attr top_r2_attr;
	struct mt_analog_top_r3_attr top_r3_attr;

	struct mt_analog_pdsys_attr pdsys_attr;
	struct mt_analog_test_attr test_attr;
	struct mt_analog_ethintp_attr ethintp_attr;

#if defined(__UBOOT__)
	void *s = NULL;
#elif defined(__KERNEL__)
#else
/* RTOS */
	void *s = NULL;
#endif

	mt_analog_get_attr(MT_ANA_INDEX_AO_R0, &ao_r0_attr);
	mt_analog_get_attr(MT_ANA_INDEX_AO_R1, &ao_r1_attr);
	mt_analog_get_attr(MT_ANA_INDEX_TOP_R0, &top_r0_attr);
	mt_analog_get_attr(MT_ANA_INDEX_TOP_R1, &top_r1_attr);
	mt_analog_get_attr(MT_ANA_INDEX_TOP_R2, &top_r2_attr);
	mt_analog_get_attr(MT_ANA_INDEX_TOP_R3, &top_r3_attr);
	mt_analog_get_attr(MT_ANA_INDEX_PDSYS, &pdsys_attr);
	mt_analog_get_attr(MT_ANA_INDEX_TEST, &test_attr);
	mt_analog_get_attr(MT_ANA_INDEX_ETHINTP, &ethintp_attr);

	DP_LOG("--------------------[ANALOG]---------------------\n");
	DP_LOG("            ana_ao_reg0[%8lX]: %08X\n", ANA_AO_REG0, MT_IO_READ32(ANA_AO_REG0));
	DP_LOG("            ana_ao_reg1[%8lX]: %08X\n", ANA_AO_REG1, MT_IO_READ32(ANA_AO_REG1));
	DP_LOG("           ana_top_reg0[%8lX]: %08X\n", ANA_TOP_REG0, MT_IO_READ32(ANA_TOP_REG0));
	DP_LOG("           ana_top_reg1[%8lX]: %08X\n", ANA_TOP_REG1, MT_IO_READ32(ANA_TOP_REG1));
	DP_LOG("           ana_top_reg2[%8lX]: %08X\n", ANA_TOP_REG2, MT_IO_READ32(ANA_TOP_REG2));
	DP_LOG("           ana_top_reg3[%8lX]: %08X\n", ANA_TOP_REG3, MT_IO_READ32(ANA_TOP_REG3));

	DP_LOG("      clkgen_cpupll_reg[%8lX]: %08X\n", REG_CLKGEN_ADCPLL, MT_IO_READ32(REG_CLKGEN_ADCPLL));
	DP_LOG("      clkgen_usbpll_reg[%8lX]: %08X\n", REG_CLKGEN_USBPLL, MT_IO_READ32(REG_CLKGEN_USBPLL));

	DP_LOG("     clkgen_vhdintp_reg[%8lX]: %08X\n", REG_CLKGEN_VHDINTP, MT_IO_READ32(REG_CLKGEN_VHDINTP));
	DP_LOG("      clkgen_vhdpll_reg[%8lX]: %08X\n", REG_CLKGEN_VHDPLL, MT_IO_READ32(REG_CLKGEN_VHDPLL));

	DP_LOG("     clkgen_vsdintp_reg[%8lX]: %08X\n", REG_CLKGEN_VSDINTP, MT_IO_READ32(REG_CLKGEN_VSDINTP));
	DP_LOG("      clkgen_vsdpll_reg[%8lX]: %08X\n", REG_CLKGEN_VSDPLL, MT_IO_READ32(REG_CLKGEN_VSDPLL));

	DP_LOG("       clkgen_pdsys_reg[%8lX]: %08X\n", REG_CLKGEN_PDSYS, MT_IO_READ32(REG_CLKGEN_PDSYS));
	DP_LOG("        clkgen_test_reg[%8lX]: %08X\n", REG_CLKGEN_TEST, MT_IO_READ32(REG_CLKGEN_TEST));
	DP_LOG("     clkgen_ethintp_reg[%8lX]: %08X\n", REG_CLKGEN_ETHINTP, MT_IO_READ32(REG_CLKGEN_ETHINTP));

	DP_LOG("      clkgen_vsdssc_reg[%8lX]: %08X\n", REG_CLKGEN_VSDSSC, MT_IO_READ32(REG_CLKGEN_VSDSSC));
	DP_LOG("      clkgen_vhdssc_reg[%8lX]: %08X\n", REG_CLKGEN_VHDSSC, MT_IO_READ32(REG_CLKGEN_VHDSSC));

	DP_LOG("    clkgen_usbaclk_reg0[%8lX]: %08X\n", CLKGEN_USBACLK_REG0, MT_IO_READ32(CLKGEN_USBACLK_REG0));
	DP_LOG("    clkgen_usbaclk_reg1[%8lX]: %08X\n", CLKGEN_USBACLK_REG1, MT_IO_READ32(CLKGEN_USBACLK_REG1));
	DP_LOG("    clkgen_usbaclk_reg2[%8lX]: %08X\n", CLKGEN_USBACLK_REG2, MT_IO_READ32(CLKGEN_USBACLK_REG2));

	/* USB */
	DP_LOG("           usb0_sw_reg0[%8lX]: %08X\n", USB0_REG0, MT_IO_READ32(USB0_REG0));
	DP_LOG("           usb0_sw_reg1[%8lX]: %08X\n", USB0_REG1, MT_IO_READ32(USB0_REG1));
	DP_LOG("           usb1_sw_reg0[%8lX]: %08X\n", USB1_REG0, MT_IO_READ32(USB1_REG0));
	DP_LOG("           usb1_sw_reg1[%8lX]: %08X\n", USB1_REG1, MT_IO_READ32(USB1_REG1));
	DP_LOG("        usb0phy_sw_reg0[%8lX]: %08X\n", USB0PHY_SW_REG0, MT_IO_READ32(USB0PHY_SW_REG0));
	DP_LOG("        usb0phy_sw_reg1[%8lX]: %08X\n", USB0PHY_SW_REG1, MT_IO_READ32(USB0PHY_SW_REG1));
	DP_LOG("        usb1phy_sw_reg0[%8lX]: %08X\n", USB1PHY_SW_REG0, MT_IO_READ32(USB1PHY_SW_REG0));
	DP_LOG("        usb1phy_sw_reg1[%8lX]: %08X\n", USB1PHY_SW_REG1, MT_IO_READ32(USB1PHY_SW_REG1));
	DP_LOG("            usb_imp_reg[%8lX]: %08X\n", REG_USB_IMP, MT_IO_READ32(REG_USB_IMP));
	DP_LOG("              usb_logic[%8lX]: %08X\n", REG_USB_LOGIC, MT_IO_READ32(REG_USB_LOGIC));

	/* USB3 */
	DP_LOG("            usb3p0_reg0[%8lX]: %08X\n", USB3P0_REG0, MT_IO_READ32(USB3P0_REG0));
	DP_LOG("            usb3p0_reg1[%8lX]: %08X\n", USB3P0_REG1, MT_IO_READ32(USB3P0_REG1));
	DP_LOG("            usb3p0_reg2[%8lX]: %08X\n", USB3P0_REG2, MT_IO_READ32(USB3P0_REG2));
	DP_LOG("            usb3p0_reg3[%8lX]: %08X\n", USB3P0_REG3, MT_IO_READ32(USB3P0_REG3));
	DP_LOG("            usb3p0_reg4[%8lX]: %08X\n", USB3P0_REG4, MT_IO_READ32(USB3P0_REG4));
	DP_LOG("     usb3p0_rxcal_reg_l[%8lX]: %08X\n", REG_USB3P0_RXCAL_L, MT_IO_READ32(REG_USB3P0_RXCAL_L));
	DP_LOG("     usb3p0_rxcal_reg_h[%8lX]: %08X\n", REG_USB3P0_RXCAL_H, MT_IO_READ32(REG_USB3P0_RXCAL_H));
	DP_LOG("        usb3p0_ssc_reg1[%8lX]: %08X\n", USB3P0_SSC_REG1, MT_IO_READ32(USB3P0_SSC_REG1));
	DP_LOG("        usb3p0_ssc_reg2[%8lX]: %08X\n", USB3P0_SSC_REG2, MT_IO_READ32(USB3P0_SSC_REG2));
	DP_LOG("         usb3p0_pll_reg[%8lX]: %08X\n", REG_USB3P0_PLL, MT_IO_READ32(REG_USB3P0_PLL));
	DP_LOG("       usb3p0_power_reg[%8lX]: %08X\n", REG_USB3P0_POWER, MT_IO_READ32(REG_USB3P0_POWER));

	//???
	DP_LOG("            ldo_psw_reg[%8lX]: %08X\n", REG_LDO_PSW, MT_IO_READ32(REG_LDO_PSW));

	DP_LOG("clkgen_(pd)ephy(rx)_reg[%8lX]: %08X\n", REG_CLKGEN_EPHY, MT_IO_READ32(REG_CLKGEN_EPHY));
	DP_LOG("     clkgen_ephypll_reg[%8lX]: %08X\n", REG_CLKGEN_EPHYPLL, MT_IO_READ32(REG_CLKGEN_EPHYPLL));

	/* HDMI */
	DP_LOG("           hdmi_tx_reg0[%8lX]: %08X\n", HDMI_TX_REG0, MT_IO_READ32(HDMI_TX_REG0));
	DP_LOG("           hdmi_tx_reg1[%8lX]: %08X\n", HDMI_TX_REG1, MT_IO_READ32(HDMI_TX_REG1));
	DP_LOG("           hdmi_tx_reg2[%8lX]: %08X\n", HDMI_TX_REG2, MT_IO_READ32(HDMI_TX_REG2));
	DP_LOG("         hdmi_test_reg0[%8lX]: %08X\n", REG_HDMI_TEST, MT_IO_READ32(REG_HDMI_TEST));
	DP_LOG("             hdmi_logic[%8lX]: %08X\n", REG_HDMI_LOGIC, MT_IO_READ32(REG_HDMI_LOGIC));

	/* ADC */
	DP_LOG("              cadc_reg0[%8lX]: %08X\n", CADC_REG0, MT_IO_READ32(CADC_REG0));
	DP_LOG("              sadc_reg0[%8lX]: %08X\n", SADC_REG0, MT_IO_READ32(SADC_REG0));
	DP_LOG("              sadc_reg1[%8lX]: %08X\n", SADC_REG1, MT_IO_READ32(SADC_REG1));
	DP_LOG("                adc_log[%8lX]: %08X\n", REG_ADC_LOG, MT_IO_READ32(REG_ADC_LOG));

	DP_LOG("              vdac_reg0[%8lX]: %08X\n", VDAC_REG0, MT_IO_READ32(VDAC_REG0));
	DP_LOG("            adac_sw_reg[%8lX]: %08X\n", REG_ADAC_SW, MT_IO_READ32(REG_ADAC_SW));
	DP_LOG("        adacphy_sw_reg0[%8lX]: %08X\n", ADACPHY_SW_REG0, MT_IO_READ32(ADACPHY_SW_REG0));
	DP_LOG("        adacphy_sw_reg1[%8lX]: %08X\n", ADACPHY_SW_REG1, MT_IO_READ32(ADACPHY_SW_REG1));
	DP_LOG("        adacphy_sw_reg2[%8lX]: %08X\n", ADACPHY_SW_REG2, MT_IO_READ32(ADACPHY_SW_REG2));
	DP_LOG("        adacphy_sw_reg3[%8lX]: %08X\n", ADACPHY_SW_REG3, MT_IO_READ32(ADACPHY_SW_REG3));

	DP_LOG("               bias_reg[%8lX]: %08X\n", REG_BIAS, MT_IO_READ32(REG_BIAS));
	DP_LOG("           otpreg25_reg[%8lX]: %08X\n", REG_OTPREG25, MT_IO_READ32(REG_OTPREG25));
	DP_LOG("                osc_reg[%8lX]: %08X\n", REG_SEC_OSC, MT_IO_READ32(REG_SEC_OSC));

	/* TSENSOR */
	DP_LOG("         tsensor_th_reg[%8lX]: %08X\n", REG_TSENSOR_TH, MT_IO_READ32(REG_TSENSOR_TH));
	DP_LOG("            tsensor_int[%8lX]: %08X\n", REG_TSENSOR_INT, MT_IO_READ32(REG_TSENSOR_INT));
	DP_LOG("            tsensor_reg[%8lX]: %08X\n", REG_TSENSOR, MT_IO_READ32(REG_TSENSOR));
	DP_LOG("           tsensor_data[%8lX]: %08X\n", REG_TSENSOR_DOUT, MT_IO_READ32(REG_TSENSOR_DOUT));
	DP_LOG("     tsensor_rst_req_th[%8lX]: %08X\n", REG_TSENSOR_RST_REQ_TH, MT_IO_READ32(REG_TSENSOR_RST_REQ_TH));

	DP_LOG("             pm_sw_reg0[%8lX]: %08X\n", PM_SW_REG0, MT_IO_READ32(PM_SW_REG0));

	/* mADC */
	DP_LOG("    vddmonitor_dout_rst[%8lX]: %08X\n", REG_VDDMNT_DOUT_RST, MT_IO_READ32(REG_VDDMNT_DOUT_RST));
	DP_LOG("         vddmonitor_set[%8lX]: %08X\n", REG_VDDMONITOR_SET, MT_IO_READ32(REG_VDDMONITOR_SET));

	DP_LOG("         intp_ctrl_reg0[%8lX]: %08X\n", INTP_CTRL_REG0, MT_IO_READ32(INTP_CTRL_REG0));
	DP_LOG("         intp_ctrl_reg1[%8lX]: %08X\n", INTP_CTRL_REG1, MT_IO_READ32(INTP_CTRL_REG1));
	DP_LOG("clkgen_video_sw_hd_reg0[%8lX]: %08X\n", CLKGEN_VIDEO_SW_HD_REG0, MT_IO_READ32(CLKGEN_VIDEO_SW_HD_REG0));
	DP_LOG("clkgen_video_sw_hd_reg1[%8lX]: %08X\n", CLKGEN_VIDEO_SW_HD_REG1, MT_IO_READ32(CLKGEN_VIDEO_SW_HD_REG1));

	/* add from "symphony6_analog_pd_reg.xlsx" */
	DP_LOG("          Keyboard ADC1[%8lX]: %08X\n", REG_AO_KEY_ADC_BASE, MT_IO_READ32(REG_AO_KEY_ADC_BASE));
	DP_LOG("          Keyboard ADC2[%8lX]: %08X\n", REG_KADC_BASE, MT_IO_READ32(REG_KADC_BASE));
	DP_LOG("            AVS_DAC_CPU[%8lX]: %08X\n", REG_AO_ANALOG_BASE+0xC0, MT_IO_READ32(REG_AO_ANALOG_BASE+0xC0));
	DP_LOG("            AVS_DAC_SYS[%8lX]: %08X\n", REG_AO_ANALOG_BASE+0xC4, MT_IO_READ32(REG_AO_ANALOG_BASE+0xC4));
	DP_LOG("            SMC0_CPCTRL[%8lX]: %08X\n", REG_SMC_BASE+0x20, MT_IO_READ32(REG_SMC_BASE+0x20));
	DP_LOG("        SMC0_5VIO_CTRL0[%8lX]: %08X\n", REG_SMC_BASE+0xAC, MT_IO_READ32(REG_SMC_BASE+0xAC));
	DP_LOG("        APCPU_ARMPLL_PD[%8lX]: %08X\n", REG_ARMCPU_BASE+0x1C, MT_IO_READ32(REG_ARMCPU_BASE+0x1C));
	DP_LOG("          DDR_PHY_CTRL0[%8lX]: %08X\n", REG_DDR_PHY_BASE+0x4, MT_IO_READ32(REG_DDR_PHY_BASE+0x4));
	DP_LOG("        DDR_PLLSSC_REG2[%8lX]: %08X\n", REG_DDR_PHY_BASE+0xB4, MT_IO_READ32(REG_DDR_PHY_BASE+0xB4));

	DP_LOG("\nana_ao_reg0[%8lX]: %08X\n", ANA_AO_REG0, *(u32*)&ao_r0_attr);
	DP_LOG("              bias ao en: %u\n", ao_r0_attr.bias_ao_en);
	DP_LOG("                 usb0 pd: %u\n", ao_r0_attr.pd_usb0);
	DP_LOG("                 usb1 pd: %u\n", ao_r0_attr.pd_usb1);
	DP_LOG("                  icx pd: %u\n", ao_r0_attr.pd_icx);
	DP_LOG("           pd vdac(cvbs): %u\n", ao_r0_attr.pd_vdac);
	DP_LOG("    pd hdmi tx channel 0: %u\n", ao_r0_attr.pd_hdmi_tx_ch0);
	DP_LOG("    pd hdmi tx channel 1: %u\n", ao_r0_attr.pd_hdmi_tx_ch1);
	DP_LOG("    pd hdmi tx channel 2: %u\n", ao_r0_attr.pd_hdmi_tx_ch2);
	DP_LOG("  pd hdmi tx clk channel: %u\n", ao_r0_attr.pd_hdmi_tx_clk_ch);
	//DP_LOG("      adac buf power ctr: 0x%02X\n", ao_r0_attr.adac_buf_power_ctr);
	DP_LOG("      adac buf power ctr: {\n");
	DP_LOG("              pd_buf_rch: %u\n", ao_r0_attr.pd_buf_rch);
	DP_LOG("             pd_adac_rch: %u\n", ao_r0_attr.pd_adac_rch);
	DP_LOG("             pd_1bit_int: %u\n", ao_r0_attr.pd_1bit_int);				/* A1+ */
	DP_LOG("              pd_vddb1p2: %u\n", ao_r0_attr.pd_vddb1p2);
	DP_LOG("              pd_buf_lch: %u\n", ao_r0_attr.pd_buf_lch);
	DP_LOG("             pd_adac_lch: %u\n", ao_r0_attr.pd_adac_lch);
	DP_LOG("      pd_internal_vdd2p7: %u }\n", ao_r0_attr.pd_internal_vdd2p7);		/* A1+ */

	DP_LOG("                 pd cadc: %u\n", ao_r0_attr.pd_cadc);
	DP_LOG("                 pd sadc: %u\n", ao_r0_attr.pd_sadc);
	DP_LOG("      pd process monitor: %u\n", ao_r0_attr.pd_proc_mon);
	DP_LOG("                 pd rng1: %u\n", ao_r0_attr.pd_rng1);
	DP_LOG("          pd usb imp cal: %u\n", ao_r0_attr.pd_usb_imp_cal);
	DP_LOG("         dreg dc test en: %u\n", ao_r0_attr.dreg_dc_tst_en);
	DP_LOG("          temp sensor pd: %u\n", ao_r0_attr.pd_tsensor);
	DP_LOG("   adac output pull down: %u\n", ao_r0_attr.adac_output_pull_down);
	DP_LOG("                 pd rng2: %u\n", ao_r0_attr.pd_rng2);
	//DP_LOG("           pd ephy pll: %u\n", ao_r0_attr.pd_ephy_pll);
	DP_LOG("             pd ephy pll: {\n");
	DP_LOG("             pd pll ephy: %u\n", ao_r0_attr.pd_pll_ephy);
	DP_LOG("            pd intp ephy: %u }\n", ao_r0_attr.pd_intp_ephy);

	DP_LOG("\nana_ao_reg1[%8lX]: %08X\n", ANA_AO_REG1, *(u32*)&ao_r1_attr);
	DP_LOG("                      pd vdac det: %u\n", ao_r1_attr.pd_vdac_det);
	DP_LOG("                        adac mute: %u\n", ao_r1_attr.adac_mute);
	DP_LOG("              pd master regulator: %u\n", ao_r1_attr.pd_master_regulator);
	DP_LOG("                      USB3 phy pd: %u\n", ao_r1_attr.pd_usb3_phy);
	DP_LOG("                      USB3 pll pd: %u\n", ao_r1_attr.pd_usb3_pll);
	//DP_LOG("digital always on regulator ctr: 0x%X\n", ao_r1_attr.dig_ao_regulator_ctr);
	DP_LOG("  digital always on regulator ctr: {\n");
	DP_LOG("                     dreg voltage: %u\n", ao_r1_attr.dreg_volt);
	DP_LOG("                      dreg ext en: %u }\n", ao_r1_attr.dreg_ext_en);

	DP_LOG("                      xtal ext en: %u\n", ao_r1_attr.xtal_ext_en);
	DP_LOG("                       pd ref pll: %u\n", ao_r1_attr.pd_ref_pll);
	DP_LOG("                    xtal gain sel: %u\n", ao_r1_attr.xtal_gain_sel);
	DP_LOG("                       pd vsd pll: %u\n", ao_r1_attr.pd_vsd_pll);
	DP_LOG("                       pd vhd pll: %u\n", ao_r1_attr.pd_vhd_pll);
	DP_LOG("                       pd usb pll: %u\n", ao_r1_attr.pd_usb_pll);
	DP_LOG("                       pd adc pll: %u\n", ao_r1_attr.pd_adc_pll);

	DP_LOG("\nana_top_reg0[%8lX]: %08X\n", ANA_TOP_REG0, *(u32*)&top_r0_attr);
	DP_LOG("         bias dc test en: %u\n", top_r0_attr.bias_dc_tst_en);
	DP_LOG("         usb0 dc test en: %u\n", top_r0_attr.usb0_dc_tst_en);
	DP_LOG("         usb1 dc test en: %u\n", top_r0_attr.usb1_dc_tst_en);
	DP_LOG("         adac dc test en: %u\n", top_r0_attr.adac_dc_tst_en);
	DP_LOG("         vdac dc test en: %u\n", top_r0_attr.vdac_dc_tst_en);
	DP_LOG("      arm pll dc test en: %u\n", top_r0_attr.armpll_dc_tst_en);
	DP_LOG("        s-adc dc test en: %u\n", top_r0_attr.sadc_dc_tst_en);
	DP_LOG("        bias dc test sel: %u\n", top_r0_attr.bias_dc_tst_sel);
	DP_LOG("         hdmi dc test en: %u\n", top_r0_attr.hdmi_dc_tst_en);
	DP_LOG("        c-adc dc test en: %u\n", top_r0_attr.cadc_dc_tst_en);
	DP_LOG("   vddmonitor dc test en: %u\n", top_r0_attr.vddmon_dc_tst_en);
	DP_LOG("      DDR PHY dc test en: %u\n", top_r0_attr.ddrphy_dc_tst_en);
	DP_LOG("  adc domain dc test sel: 0x%X\n", top_r0_attr.adc_domain_dc_tst_sel);
	DP_LOG(" hdmi domain dc test sel: %u\n", top_r0_attr.hdmi_domain_dc_tst_sel);
	DP_LOG("  usb domain dc test sel: %u\n", top_r0_attr.usb_domain_dc_tst_sel);
	DP_LOG(" adac domain dc test sel: %u\n", top_r0_attr.adac_domain_dc_tst_sel);
	DP_LOG(" vdac domain dc test sel: %u\n", top_r0_attr.vdac_domain_dc_tst_sel);

	DP_LOG("\nana_top_reg1[%8lX]: %08X\n", ANA_TOP_REG1, *(u32*)&top_r1_attr);
	DP_LOG("     S-adc input mode sel: %u\n", top_r1_attr.sadc_input_mode);
	DP_LOG("     C-adc input mode sel: %u\n", top_r1_attr.cadc_input_mode);
	DP_LOG("   clockgen domain dc sel: 0x%03X\n", top_r1_attr.clockgen_domain_dc_sel);
	DP_LOG("        ana tst io switch: %u\n", top_r1_attr.ana_tst_io_switch);
	DP_LOG(" vddcore sw en to ana tst: %u\n", top_r1_attr.vddcore_sw_en_2_ana_tst);
	//DP_LOG("   s-adc register reserve: 0x%02X\n", top_r1_attr.sadc_reg_rsv);
	DP_LOG("   s-adc register reserve: {\n");
	DP_LOG("              cal_lpf_enb: %u\n", top_r1_attr.sadc_cal_lpf_enb);
	DP_LOG("              cal_clk_reg: %u\n", top_r1_attr.sadc_cal_clk_reg);
	DP_LOG("              cal_clk_sel: %u\n", top_r1_attr.sadc_cal_clk_sel);
	DP_LOG("              cal_mux_sel: %u\n", top_r1_attr.sadc_cal_mux_sel);
	DP_LOG("             clk_cal_ctrl: %u\n", top_r1_attr.sadc_clk_cal_ctrl);
	DP_LOG("              clk_cal_sel: %u }\n", top_r1_attr.sadc_clk_cal_sel);
	DP_LOG("          temp sensor tst: 0x%X\n", top_r1_attr.tsensor_tst);
	DP_LOG("     sec osc cal freq sel: %u\n", top_r1_attr.sec_osc_cal_freq_sel);
	DP_LOG("          smc ip dc sw en: %u\n", top_r1_attr.smc_ip_dc_sw_en);
	DP_LOG("         vdd pll dc sw en: %u\n", top_r1_attr.vdd_pll_dc_sw_en);
	DP_LOG("           int ID read en: %u\n", top_r1_attr.int_ID_read_en);

	DP_LOG("\nana_top_reg2[%8lX]: %08X\n", ANA_TOP_REG2, *(u32*)&top_r2_attr);
	//DP_LOG("s-adc register reserve: 0x%02X\n", top_r2_attr.sadc_reg_rsv);
	DP_LOG("s-adc register reserve: {\n");
	DP_LOG("            ctr_buffer: %u }\n", top_r2_attr.sadc_ctr_buffer);
	//DP_LOG("              RNG1 reg: 0x%03X\n", top_r2_attr.rng1_reg);
	DP_LOG("                 RNG-1: {\n");
	DP_LOG("          clkedge_swqp: %u\n", top_r2_attr.rng1_clkedge_swqp);
	DP_LOG("               div_sel: %u\n", top_r2_attr.rng1_div_sel);
	DP_LOG("              clk_gate: %u\n", top_r2_attr.rng1_clk_gate);
	DP_LOG("                 hfast: %u\n", top_r2_attr.rng1_hfast);
	DP_LOG("                 hslow: %u\n", top_r2_attr.rng1_hslow);
	DP_LOG("               clk_sel: %u\n", top_r2_attr.rng1_clk_sel);
	//DP_LOG("          reset: %u\n", top_r2_attr.rng1_reset);
	DP_LOG("         rng_out_selbb: %u\n", top_r2_attr.rng1_rng_out_selbb);
	DP_LOG("             cp_sr_sel: %u\n", top_r2_attr.rng1_cp_sr_sel);
	DP_LOG("            pd_doubler: %u }\n", top_r2_attr.rng1_pd_doubler);
	//DP_LOG("              RNG2 reg: 0x%03X\n", top_r2_attr.rng2_reg);
	DP_LOG("                 RNG-2: {\n");
	DP_LOG("          clkedge_swqp: %u\n", top_r2_attr.rng2_clkedge_swqp);
	DP_LOG("               div_sel: %u\n", top_r2_attr.rng2_div_sel);
	DP_LOG("              clk_gate: %u\n", top_r2_attr.rng2_clk_gate);
	DP_LOG("                 hfast: %u\n", top_r2_attr.rng2_hfast);
	DP_LOG("                 hslow: %u\n", top_r2_attr.rng2_hslow);
	DP_LOG("               clk_sel: %u\n", top_r2_attr.rng2_clk_sel);
	//DP_LOG("          reset: %u\n", top_r2_attr.rng2_reset);
	DP_LOG("         rng_out_selbb: %u\n", top_r2_attr.rng2_rng_out_selbb);
	DP_LOG("             cp_sr_sel: %u\n", top_r2_attr.rng2_cp_sr_sel);
	DP_LOG("            pd_doubler: %u }\n", top_r2_attr.rng2_pd_doubler);
	DP_LOG("  ephy ref clk div sel: %u\n", top_r2_attr.ephy_ref_clk_div_sel);

	DP_LOG("\nana_top_reg3[%8lX]: %08X\n", ANA_TOP_REG3, *(u32*)&top_r3_attr);
	DP_LOG("          adac R/L data swap: %u\n", top_r3_attr.adac_rl_data_swap);
	DP_LOG("      adac data buf edge sel: %u\n", top_r3_attr.adac_data_buf_edge_sel);
	DP_LOG("        ephy pll dc test sel: %u\n", top_r3_attr.ephy_pll_dc_tst_sel);
	DP_LOG("         ephy pll dc test en: %u\n", top_r3_attr.ephy_pll_dc_tst_en);
	DP_LOG("         clockgen dc test en: 0x%02X\n", top_r3_attr.clockgen_dc_tst_en);
	DP_LOG("adac pu 1bit pcm to external: %u\n", top_r3_attr.adac_pu_1bit_pcm_to_extern);	/* A1+ */
	DP_LOG("adac pu 1bit pcm to internal: %u\n", top_r3_attr.adac_pu_1bit_pcm_to_inter);	/* A1+ */
	DP_LOG("         adac digital pad on: %u\n", top_r3_attr.adac_dig_pad_oe);				/* A1+ */
	DP_LOG("    adac digital pad driving: %u\n", top_r3_attr.adac_dig_pad_driving);			/* A1+ */
	DP_LOG("             USB3 dc test en: %u\n", top_r3_attr.usb3_dc_tst_en);
	DP_LOG("            USB3 dc test sel: 0x%X\n", top_r3_attr.usb3_dc_tst_sel);
	DP_LOG("     c-adc internal reg ctrl: %u\n", top_r3_attr.cadc_internal_reg_ctrl);		/* A1+ */

	DP_LOG("\nclkgen_pdsys_reg[%8lX]: %08X\n", REG_CLKGEN_PDSYS, *(u32*)&pdsys_attr);
	DP_LOG("          pd clk usbpll d2: %u\n", pdsys_attr.pd_clk_usbpll_d2);
	DP_LOG("          pd clk usbpll d3: %u\n", pdsys_attr.pd_clk_usbpll_d3);
	DP_LOG("          pd clk usbpll d4: %u\n", pdsys_attr.pd_clk_usbpll_d4);
	DP_LOG("          pd clk usbpll d5: %u\n", pdsys_attr.pd_clk_usbpll_d5);
	DP_LOG("          pd clk usbpll d6: %u\n", pdsys_attr.pd_clk_usbpll_d7);
	DP_LOG("         pd clk usbpll d11: %u\n", pdsys_attr.pd_clk_usbpll_d11);
	DP_LOG("                       mux: %u\n", pdsys_attr.mux);
	DP_LOG("                 div7 duty: %u\n", pdsys_attr.div7_duty);

	//DP_LOG("                pd clk vhd: 0x%02X\n", pdsys_attr.pd_clk_vhd);
	DP_LOG("                pd clk vhd: {\n");
	DP_LOG("                   div sel: %u\n", pdsys_attr.vhd_div_sel);
	DP_LOG("            hdmi clk sel-1: %u\n", pdsys_attr.vhd_clk_sel_1);
	DP_LOG("                pd vhd clk: %u\n", pdsys_attr.pd_vhd_clk);
	DP_LOG("            hdmi clk sel-2: %u }\n", pdsys_attr.vhd_clk_sel_2);

	//DP_LOG("                pd clk vsd: 0x%02X\n", pdsys_attr.pd_clk_vsd);
	DP_LOG("                pd clk vsd: {\n");
	DP_LOG("                   div sel: %u\n", pdsys_attr.vsd_div_sel);
	DP_LOG("            hdmi clk sel-1: %u\n", pdsys_attr.vsd_clk_sel_1);
	DP_LOG("              clk drv0 sel: %u\n", pdsys_attr.vsd_drv0_sel);
	DP_LOG("                pd vsd clk: %u\n", pdsys_attr.pd_vsd_clk);
	DP_LOG("            hdmi clk sel-2: %u }\n", pdsys_attr.vsd_clk_sel_2);

	DP_LOG("               pd drv0 clk: %u\n", pdsys_attr.pd_drv0_clk);
	DP_LOG("               pd drv1 clk: %u\n", pdsys_attr.pd_drv1_clk);
	DP_LOG("              drv0 clk sel: %u\n", pdsys_attr.drv0_clk_sel);
	DP_LOG("              drv1 clk sel: %u\n", pdsys_attr.drv1_clk_sel);
	DP_LOG("                pd usb div: %u\n", pdsys_attr.pd_usb_div);
	DP_LOG("            pd clk mon int: %u\n", pdsys_attr.pd_clk_mon_int);
	DP_LOG("    pd clk cal in xtal top: %u\n", pdsys_attr.pd_clk_cal);

	DP_LOG("\nclkgen_test_reg[%8lX]: %08X\n", REG_CLKGEN_TEST, *(u32*)&test_attr);
	DP_LOG("              clk test div: 0x%X\n", test_attr.clk_test_div);
	DP_LOG("              clk test sel: 0x%X\n", test_attr.clk_test_sel);
	DP_LOG("             clk divxx sel: %u\n", test_attr.clk_divxx_sel);
	DP_LOG("               clk test en: %u\n", test_attr.clk_test_en);
	DP_LOG("                    pd 81M: %u\n", test_attr.pd_81M);
	DP_LOG("              pd cadc 270M: %u\n", test_attr.pd_cadc_270M);
	DP_LOG("                  pd 1350M: %u\n", test_attr.pd_1350M);
	DP_LOG("                   pd 270M: %u\n", test_attr.pd_270M);
	DP_LOG("               pd vhd inth: %u\n", test_attr.pd_vhd_inth);
	DP_LOG("                pd vhd pll: %u\n", test_attr.pd_vhd_pll);
	DP_LOG("                pd vhd clk: %u\n", test_attr.pd_vhd_clk);
	DP_LOG("               pd vsd inth: %u\n", test_attr.pd_vsd_inth);
	DP_LOG("                pd vsd pll: %u\n", test_attr.pd_vsd_pll);
	DP_LOG("                pd vsd clk: 0x%X\n", test_attr.pd_vsd_clk);
	DP_LOG("        sel clkcal in xtal: %u\n", test_attr.clkcal_sel_xtal_div);
	DP_LOG("  sel clkcal in clkgen usb: %u\n", test_attr.clkcal_sel_usb_div);

	DP_LOG("\nclkgen_ethintp_reg[%8lX]: %08X\n", REG_CLKGEN_ETHINTP, MT_IO_READ32(REG_CLKGEN_ETHINTP));
	DP_LOG("              clk sel: %u\n", ethintp_attr.clk_sel);
	DP_LOG("         drv0 clk sel: %u\n", ethintp_attr.drv0_clk_sel);
	DP_LOG(" pllcpu clkref 30M en: %u\n", ethintp_attr.clkref_30M_en);
	DP_LOG("    pllcpu clkref sel: %u\n", ethintp_attr.clkref_sel);
	DP_LOG("        s-adc clk sel: %u\n", ethintp_attr.sadc_clk_sel);
	DP_LOG("     drv0_clk 81M sel: %u\n", ethintp_attr.drv0_clk_81M_sel);
	DP_LOG("    cadc clk 270M sel: %u\n", ethintp_attr.cadc_clk_270M_sel);
	DP_LOG("              div_sel: %u\n", ethintp_attr.div_sel);
	DP_LOG("              sw_edge: %u\n", ethintp_attr.sw_edge);
	DP_LOG("         clk_edge_sel: %u\n", ethintp_attr.clk_edge_sel);
	DP_LOG("                i_sel: %u\n", ethintp_attr.i_sel);
	DP_LOG("       dco_ctrl_valid: %u\n", ethintp_attr.dco_ctrl_valid);
	DP_LOG("        dco_ctrl_data: 0x%04X\n", ethintp_attr.dco_ctrl_data);

	mt_adc_dump_state(s);
	mt_adac_dump_state(s);
	mt_vdac_dump_state(s);
	mt_vxdssc_dump_state(s);
	mt_ephy_dump_state(s);
	mt_hdmi_dump_state(s);

	mt_usbaclk_dump_state(s);
	mt_usb_dump_state(s);
	mt_usb3p0_dump_state(s);

	mt_bias_dump_state(s);
	mt_otpreg_dump_state(s);
	mt_osc_dump_state(s);
	mt_tsensor_dump_state(s);
	mt_pmon_dump_state(s);

	mt_ana_pll_dump_state(s);

	DP_LOG("-------------------------------------------------\n");
}

#if defined(__UBOOT__)
void print_temperature(void)
#elif defined(__KERNEL__)
void print_temperature(struct seq_file *s)
#else
/* RTOS */
void print_temperature(void)
#endif
{
	int temp_int = 0;
	int temp_dec = 0;

#if defined(__UBOOT__)
	mt_analog_get_temperature(&temp_int, &temp_dec);
#elif defined(__KERNEL__)
	mt_analog_get_temperature_async(&temp_int, &temp_dec);
#else
	mt_analog_get_temperature(&temp_int, &temp_dec);
#endif

	if (temp_dec < 0)
		temp_dec = -temp_dec;

	DP_LOG("Temperature[%08lX]: %X, %d.%02d(C)\n",
		REG_TSENSOR_DOUT,
		MT_IO_READ32(REG_TSENSOR_DOUT) & TSENSOR_DOUT_MASK,
		temp_int,
		temp_dec);
}

//---------------------------------------------------------------------------//

#if defined(__UBOOT__)

static int do_cmd_analog(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "enable") == 0)
	{
		if (argc < 3)
			return CMD_RET_USAGE;

		ret = mt_analog_enable(argv[2]);

		if (ret == 0)
		{
			printf("analog enable %s - success.\n", argv[2]);
		}
		else
		{
			printf("analog enable %s - fail(%d)!\n", argv[2], ret);
		}
	}
	else if (strcmp(argv[1], "disable") == 0)
	{
		if (argc < 3)
			return CMD_RET_USAGE;

		ret = mt_analog_disable(argv[2]);

		if (ret == 0)
		{
			printf("analog disable %s - success.\n", argv[2]);
		}
		else
		{
			printf("analog disable %s - fail(%d)!\n", argv[2], ret);
		}
	}
	else if (strcmp(argv[1], "reset") == 0)
	{
		if (argc < 3)
			return CMD_RET_USAGE;

		ret = mt_analog_reset(argv[2]);

		if (strcmp(argv[2], MT_ANA_TSENSOR) == 0)
			udelay(CFG_MT_ANA_RESET_UDELAY_MAX);
		else
			udelay(CFG_MT_ANA_RESET_UDELAY_MIN);

		ret |= mt_analog_release(argv[2]);

		if (ret == 0)
		{
			printf("analog reset %s - success.\n", argv[2]);
		}
		else
		{
			printf("analog reset %s - fail(%d)!\n", argv[2], ret);
		}
	}
	else if (strcmp(argv[1], "dump") == 0)
	{
		mt_analog_dump_status();
		mt_analog_dump_state();
	}
	else if (strcmp(argv[1], "get_temp") == 0)
	{
		print_temperature();
	}
	else
	{
		return CMD_RET_USAGE;
	}

	if (ret == 0)
		return CMD_RET_SUCCESS;
	else
		return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	mt_analog, 3, 0, do_cmd_analog,
	"Analog sub-system",
	"\nmt_analog dump - dump analog information\n"
	"mt_analog enable mod - enable analog module\n"
	"mt_analog disable mod - disable analog module\n"
	"mt_analog reset mod - reset analog module\n"
	"mt_analog get_temp - get temperature\n"
);
#endif

#endif

