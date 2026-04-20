/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Montage LZ Co., Ltd.
 */
#if defined(__UBOOT__)
#include <common.h>
#include <command.h>

#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_analog.h>
#include <asm/arch-symphony6/mt_analog_parameter.h>

#include "mt_crm_clock_reg.h"

#elif defined(__KERNEL__)
#include <linux/types.h>
#include <linux/string.h>

#include "../crm/mt_log.h"
#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"
#include "../crm/mt_crm_clock_reg.h"

#include "mt_drv_analog.h"
#include "analog/mt_analog_parameter.h"
#else
/* RTOS */
#include <sys_types.h>
#include <sys_define.h>

#include "../crm/mt_log.h"
#include "../crm/mt_io.h"
#include "../crm/mt_reg_base.h"
#include "../crm/mt_crm_clock_reg.h"

#include "mt_drv_analog.h"
#include "analog/mt_analog_parameter.h"
#endif

#include "mt_analog_reg.h"
#include "mt_log_internal.h"

#define NO_DUMP_HDMI_STATE
#define NO_DUMP_USBACLK_STATE
#define NO_DUMP_VXDSSC_STATE
#define NO_DUMP_PLL_STATE
#if defined(__UBOOT__)
#include <asm/arch-symphony6/mt_analog_hdmi.h>
#include <asm/arch-symphony6/mt_analog_usbaclk.h>
#include <asm/arch-symphony6/mt_analog_vxdssc.h>
#include <asm/arch-symphony6/mt_analog_pll.h>
#elif defined(__KERNEL__)
#include "analog/mt_analog_hdmi.h"
#include "analog/mt_analog_usbaclk.h"
#include "analog/mt_analog_vxdssc.h"
#include "analog/mt_analog_pll.h"
#else
/* RTOS */
#include "analog/mt_analog_hdmi.h"
#include "analog/mt_analog_usbaclk.h"
#include "analog/mt_analog_vxdssc.h"
#include "analog/mt_analog_pll.h"
#endif

#if defined(__UBOOT__)
#define MUTEX_LOCK()			do{}while(0)
#define MUTEX_UNLOCK()			do{}while(0)
#elif defined(__KERNEL__)
static DEFINE_SPINLOCK(ana_para_lock);
static unsigned long flags;
#define MUTEX_LOCK()			spin_lock_irqsave(&ana_para_lock, flags)
#define MUTEX_UNLOCK()			spin_unlock_irqrestore(&ana_para_lock, flags)
#else
/* RTOS */
/* TODO: */
#define MUTEX_LOCK()			do{}while(0)
#define MUTEX_UNLOCK()			do{}while(0)
#endif

/* Debug */
#if 0
/* enum MT_ANALOG_PARAM_INDEX_E */
static const char *szIndex[] =
{
	"",
	"ANALOG_PARAM_INDEX_HDMIPHY",
	"ANALOG_PARAM_INDEX_VHDCLK",
	"ANALOG_PARAM_INDEX_VSDCLK",
	"ANALOG_PARAM_INDEX_VHDSSC",
	"ANALOG_PARAM_INDEX_VSDSSC",

	"ANALOG_PARAM_INDEX_RAW_VHDPLL",
	"ANALOG_PARAM_INDEX_RAW_VSDPLL",

	"ANALOG_PARAM_INDEX_ADC",

	"ANALOG_PARAM_INDEX_ADCPLL",
	"ANALOG_PARAM_INDEX_USBPLL",
	"ANALOG_PARAM_INDEX_VHDPLL",
	"ANALOG_PARAM_INDEX_VSDPLL",
	"ANALOG_PARAM_INDEX_REFPLL",
	"ANALOG_PARAM_INDEX_USB3P0PLL",
	"ANALOG_PARAM_INDEX_EPHYPLL",
	"ANALOG_PARAM_INDEX_DDRPLL",
	"ANALOG_PARAM_INDEX_ARMPLL",

	"ANALOG_PARAM_INDEX_HDMICLK",
	"ANALOG_PARAM_INDEX_HDMIDCC",

	"ANALOG_PARAM_INDEX_HDMIPHY_TST",
	"ANALOG_PARAM_INDEX_HDMITX_CH0",
	"ANALOG_PARAM_INDEX_HDMITX_CH1",
	"ANALOG_PARAM_INDEX_HDMITX_CH2",

	"ANALOG_PARAM_INDEX_VCLK_SRC",

};

static const char *get_index_str(enum MT_ANALOG_PARAM_INDEX_E index)
{
	if (index >= ANALOG_PARAM_INDEX_HDMIPHY && index < ANALOG_PARAM_INDEX_MAX)
	{
		return szIndex[index];
	}
	else
	{
		return "UNKNOWN";
	}
}
#endif

static void get_hdmiphy_parameter(struct hdmiphy_param *p_para)
{
	struct mt_analog_hdmitx_r2_attr hdmitx_r2_attr;

	dump_reg(HDMI_TX_REG2);

	mt_analog_get_attr(MT_ANA_INDEX_HDMITX_R2, (void*)&hdmitx_r2_attr);

	p_para->mute    = hdmitx_r2_attr.mute0;

	//FIXME:
	if (hdmitx_r2_attr.mute0 != hdmitx_r2_attr.mute1
		|| hdmitx_r2_attr.mute2 != hdmitx_r2_attr.mute0
		|| hdmitx_r2_attr.mute_clk != hdmitx_r2_attr.mute0
		/*|| hdmitst_r0_attr.mute != hdmitx_r2_attr.mute0*/
		)
	{
		MT_LOGW("warning: hdmi mute not consistent!\r\n");
	}

	TRACE_LOG("HDMI PHY - mute: %u\n", p_para->mute);
}

static void get_hdmiphy_tst_parameter(struct hdmiphy_tst_param *p_para)
{
	dump_reg(REG_HDMI_TEST);

	p_para->all = MT_IO_READ32(REG_HDMI_TEST);
}

static void get_hdmitx_ch0_parameter(struct hdmitx_ch_param *p_para)
{
	dump_reg(HDMI_TX_REG0);

	p_para->all = MT_IO_READ32(HDMI_TX_REG0);
}

static void get_hdmitx_ch1_parameter(struct hdmitx_ch_param *p_para)
{
	dump_reg(HDMI_TX_REG1);

	p_para->all = MT_IO_READ32(HDMI_TX_REG1);
}

static void get_hdmitx_ch2_parameter(struct hdmitx_ch_param *p_para)
{
	dump_reg(HDMI_TX_REG2);

	p_para->all = MT_IO_READ32(HDMI_TX_REG2);
}

static void get_hdmiclk_parameter(struct hdmiclk_param *p_para)
{
	struct mt_analog_usbaclk_r0_attr usbaclk_r0_attr;

	dump_reg(CLKGEN_USBACLK_REG0);

	mt_analog_get_attr(MT_ANA_INDEX_USBACLK_R0, (void*)&usbaclk_r0_attr);

	p_para->hdmi_clk_tmds_sel = usbaclk_r0_attr.hdmi_clk_tmds_sel;
	p_para->sel540M           = usbaclk_r0_attr.sel540M;
	p_para->sel2970M          = usbaclk_r0_attr.sel2970M;

	TRACE_LOG("HDMI Clk Sel - hdmi_clk_tmds_sel: %u, sel540M: %u, sel2970M: %u\n",
		p_para->hdmi_clk_tmds_sel,
		p_para->sel540M,
		p_para->sel2970M);
}

static void get_hdmidcc_parameter(struct hdmidcc_param *p_para)
{
	struct mt_analog_usbaclk_r0_attr usbaclk_r0_attr;

	dump_reg(CLKGEN_USBACLK_REG0);

	mt_analog_get_attr(MT_ANA_INDEX_USBACLK_R0, (void*)&usbaclk_r0_attr);

	p_para->dcc_xpd = usbaclk_r0_attr.dcc_xpd;
	p_para->dcc_sel = usbaclk_r0_attr.dcc_sel;

	TRACE_LOG("HDMI PHY clk DCC - dcc_xpd: %u, dcc_sel: %u\n",
		p_para->dcc_xpd,
		p_para->dcc_sel);
}

static void get_vhdclk_parameter(struct vhdclk_param *p_para)
{
	struct mt_analog_pdsys_attr pdsys_attr;

	dump_reg(REG_CLKGEN_PDSYS);

	mt_analog_get_attr(MT_ANA_INDEX_PDSYS, (void*)&pdsys_attr);

	p_para->div_sel   = pdsys_attr.vhd_div_sel;
	p_para->clk_sel_1 = pdsys_attr.vhd_clk_sel_1;
	p_para->drv0_sel  = pdsys_attr.reserved1;
	p_para->clk_sel_2 = pdsys_attr.vhd_clk_sel_2;

	TRACE_LOG("VHD CLK - div_sel: %u, clk_sel_1: %u, drv0_sel: %u, clk_sel_2: %u\n",
			p_para->div_sel,
			p_para->clk_sel_1,
			p_para->drv0_sel,
			p_para->clk_sel_2);
}

static void get_vsdclk_parameter(struct vsdclk_param *p_para)
{
	struct mt_analog_pdsys_attr pdsys_attr;

	dump_reg(REG_CLKGEN_PDSYS);

	mt_analog_get_attr(MT_ANA_INDEX_PDSYS, (void*)&pdsys_attr);

	p_para->div_sel   = pdsys_attr.vsd_div_sel;
	p_para->clk_sel_1 = pdsys_attr.vsd_clk_sel_1;
	p_para->drv0_sel  = pdsys_attr.vsd_drv0_sel;
	p_para->clk_sel_2 = pdsys_attr.vsd_clk_sel_2;

	TRACE_LOG("VSD CLK - div_sel: %u, clk_sel_1: %u, drv0_sel: %u, clk_sel_2: %u\n",
			p_para->div_sel,
			p_para->clk_sel_1,
			p_para->drv0_sel,
			p_para->clk_sel_2);
}

static void get_vhdssc_parameter(struct vhdssc_param *p_para)
{
	dump_reg(REG_CLKGEN_VHDSSC);

	p_para->all = MT_IO_READ32(REG_CLKGEN_VHDSSC);

	TRACE_LOG("VHD SSC - 0x%08X\n", p_para->all);
}

static void get_vsdssc_parameter(struct vsdssc_param *p_para)
{
	struct mt_analog_vsdssc_attr vsdssc_attr;

	dump_reg(REG_CLKGEN_VSDSSC);

	mt_analog_get_attr(MT_ANA_INDEX_VSDSSC, (void*)&vsdssc_attr);

	p_para->clk_sel = vsdssc_attr.clk_sel;
	p_para->en_ssc  = vsdssc_attr.en_ssc;
	p_para->pd_ssc  = vsdssc_attr.pd_ssc;

	TRACE_LOG("VSD SSC - clk_sel: %u, en_ssc: %u, pd_ssc: %u\n",
			p_para->clk_sel,
			p_para->en_ssc,
			p_para->pd_ssc);
}

static void get_vhdpll_parameter(struct vhdpll_param *p_para)
{
	dump_reg(REG_CLKGEN_VHDPLL);

	p_para->all = MT_IO_READ32(REG_CLKGEN_VHDPLL);

	TRACE_LOG("VHD PLL - 0x%08X\n", p_para->all);
}

static void get_vsdpll_parameter(struct vsdpll_param *p_para)
{
	dump_reg(REG_CLKGEN_VSDPLL);

	p_para->all = MT_IO_READ32(REG_CLKGEN_VSDPLL);

	TRACE_LOG("VSD PLL - 0x%08X\n", p_para->all);
}

static void get_vclk_src_parameter(struct vclk_source_param *p_para)
{
	struct mt_analog_vhdintp_attr vhd_attr;

	dump_reg(REG_CLKGEN_VHDINTP);

	mt_analog_get_attr(MT_ANA_INDEX_VHDINTP, (void*)&vhd_attr);

	p_para->vsd_os_108M_sel = vhd_attr.vsd_os108M_sel;
	p_para->tmds_sel = vhd_attr.tmds_sel;

	TRACE_LOG("vsd os108 from: %u, tmds sel: %u\n",
		p_para->vsd_os_108M_sel,
		p_para->tmds_sel);
}

static void get_adc_parameter(struct adc_param *p_para)
{
	struct mt_analog_pdsys_attr pdsys_attr;
	struct mt_analog_ethintp_attr ethintp_attr;
	struct mt_analog_adcpll_attr adcpll_attr;

	dump_reg(REG_CLKGEN_PDSYS);
	dump_reg(REG_CLKGEN_ETHINTP);
	dump_reg(REG_CLKGEN_ADCPLL);

	mt_analog_get_attr(MT_ANA_INDEX_PDSYS, (void*)&pdsys_attr);
	mt_analog_get_attr(MT_ANA_INDEX_ETHINTP, (void*)&ethintp_attr);
	mt_analog_get_attr(MT_ANA_INDEX_ADCPLL, (void*)&adcpll_attr);

	p_para->drv0.pd_drv0_clk  = pdsys_attr.pd_drv0_clk;
	p_para->drv1.pd_drv1_clk  = pdsys_attr.pd_drv1_clk;
	p_para->drv0.drv0_clk_sel = pdsys_attr.drv0_clk_sel;
	p_para->drv1.drv1_clk_sel = pdsys_attr.drv1_clk_sel;

	p_para->drv0.drv0_clk_mux     = ethintp_attr.drv0_clk_sel;
	p_para->sadc.sadc_clk_sel     = ethintp_attr.sadc_clk_sel;
	p_para->drv0.drv0_clk_81M_mux = ethintp_attr.drv0_clk_81M_sel;

	p_para->cadc.pd_cadc_270M   = adcpll_attr.pd_cadc_270M;
	p_para->cadc.pd_cdemod_81M  = adcpll_attr.pd_cdemod_81M;
	p_para->sadc.pd_sadc_1100M  = adcpll_attr.pd_sadc_1100M;
	p_para->sadc.pd_sdemod_270M = adcpll_attr.pd_sdemod_270M;
	p_para->cadc.cadc_clk_sel   = adcpll_attr.cadc_clk_sel;

	TRACE_LOG("SADC - pd_sadc_1100M: %u, pd_sdemod_270M: %u, sadc_clk_sel: %u\n",
			p_para->sadc.pd_sadc_1100M,
			p_para->sadc.pd_sdemod_270M,
			p_para->sadc.sadc_clk_sel);
	TRACE_LOG("CADC - pd_cadc_270M: %u, pd_cdemod_81M: %u, cadc_clk_sel: %u\n",
			p_para->cadc.pd_cadc_270M,
			p_para->cadc.pd_cdemod_81M,
			p_para->cadc.cadc_clk_sel);
	TRACE_LOG("DRV0 - pd_drv0_clk: %u, drv0_clk_sel: %u, drv0_clk_mux: %u, drv0_clk_81M_mux: %u\n",
			p_para->drv0.pd_drv0_clk,
			p_para->drv0.drv0_clk_sel,
			p_para->drv0.drv0_clk_mux,
			p_para->drv0.drv0_clk_81M_mux);
	TRACE_LOG("DRV1 - pd_drv1_clk: %u, drv1_clk_sel: %u\n",
			p_para->drv1.pd_drv1_clk,
			p_para->drv1.drv1_clk_sel);
}

/* get general pll parameter */
static void get_pll_parameter(enum MT_ANALOG_PARAM_INDEX_E index, struct pll_param *p_para)
{
	enum MT_ANALOG_INDEX_E attr_index;
	struct mt_analog_gen_pll_attr attr;

	if (index == ANALOG_PARAM_INDEX_ARMPLL) {
		attr_index = MT_ANA_INDEX_ARMPLL;
	} else if (index == ANALOG_PARAM_INDEX_ADCPLL) {
		attr_index = MT_ANA_INDEX_ADCPLL;
	} else if (index == ANALOG_PARAM_INDEX_USBPLL) {
		attr_index = MT_ANA_INDEX_USBPLL;
	} else if (index == ANALOG_PARAM_INDEX_VHDPLL) {
		attr_index = MT_ANA_INDEX_VHDPLL;
	} else if (index == ANALOG_PARAM_INDEX_VSDPLL) {
		attr_index = MT_ANA_INDEX_VSDPLL;
	} else if (index == ANALOG_PARAM_INDEX_EPHYPLL) {
		attr_index = MT_ANA_INDEX_EPHYPLL;
	} else if (index == ANALOG_PARAM_INDEX_USB3P0PLL) {
		attr_index = MT_ANA_INDEX_USB3P0PLL;
	} else if (index == ANALOG_PARAM_INDEX_REFPLL) {
		attr_index = MT_ANA_INDEX_REFPLL;
	} else if (index == ANALOG_PARAM_INDEX_DDRPLL) {
		attr_index = MT_ANA_INDEX_DDRPLL;
	} else {
		//error!
		return;
	}

	mt_analog_get_attr(attr_index, (void*)&attr);

	//FIXME
	//p_para->div_cal_valid = ?;
	p_para->div_cal   = attr.div_cal;
	p_para->div_front = attr.div_front;
	//FIXME
	//p_para->pre_div_valid = ?;
	p_para->pre_div   = attr.pre_div;
	p_para->div_fb    = attr.div_fb;
	p_para->vco_sel   = attr.vco_sel;
	p_para->vco_ext   = attr.vco_ext;

	TRACE_LOG("PLL[%d] - div_cal: %X, div_front: %X, pre_div: %u, div_fb: %X, vco_ext: %u, vco_sel: %u\n",
			index,
			p_para->div_cal,
			p_para->div_front,
			p_para->pre_div,
			p_para->div_fb,
			p_para->vco_ext,
			p_para->vco_sel);
}

//---------------------------------------------------------------------------//

int mt_analog_get_parameter(enum MT_ANALOG_PARAM_INDEX_E index, void *para)
{
	if (index < ANALOG_PARAM_INDEX_HDMIPHY || index >= ANALOG_PARAM_INDEX_MAX)
	{
		MT_LOGE("error: %s - invalid parameter(%d)!\n", __FUNCTION__, index);
		return (-22);
	}

	if (para == NULL)
	{
		MT_LOGE("error: %s - para is null!\n", __FUNCTION__);
		return (-22);
	}

	MT_LOGI("%s: %d(%s)\n", __FUNCTION__, index, get_index_str(index));

	MUTEX_LOCK();

	switch (index)
	{
		case ANALOG_PARAM_INDEX_HDMIPHY:
		{
			get_hdmiphy_parameter((struct hdmiphy_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMIPHY_TST:
		{
			get_hdmiphy_tst_parameter((struct hdmiphy_tst_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMITX_CH0:
		{
			get_hdmitx_ch0_parameter((struct hdmitx_ch_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMITX_CH1:
		{
			get_hdmitx_ch1_parameter((struct hdmitx_ch_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMITX_CH2:
		{
			get_hdmitx_ch2_parameter((struct hdmitx_ch_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMICLK:
		{
			get_hdmiclk_parameter((struct hdmiclk_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMIDCC:
		{
			get_hdmidcc_parameter((struct hdmidcc_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_VHDCLK:
		{
			get_vhdclk_parameter((struct vhdclk_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_VSDCLK:
		{
			get_vsdclk_parameter((struct vsdclk_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_VHDSSC:
		{
			get_vhdssc_parameter((struct vhdssc_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_VSDSSC:
		{
			get_vsdssc_parameter((struct vsdssc_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_RAW_VHDPLL:
		{
			get_vhdpll_parameter((struct vhdpll_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_RAW_VSDPLL:
		{
			get_vsdpll_parameter((struct vsdpll_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_VCLK_SRC:
		{
			get_vclk_src_parameter((struct vclk_source_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_ADC:
		{
			get_adc_parameter((struct adc_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_ADCPLL:
		case ANALOG_PARAM_INDEX_USBPLL:
		case ANALOG_PARAM_INDEX_VHDPLL:
		case ANALOG_PARAM_INDEX_VSDPLL:
		case ANALOG_PARAM_INDEX_EPHYPLL:
		case ANALOG_PARAM_INDEX_USB3P0PLL:
		case ANALOG_PARAM_INDEX_REFPLL:
		case ANALOG_PARAM_INDEX_DDRPLL:
		case ANALOG_PARAM_INDEX_ARMPLL:
		{
			get_pll_parameter(index, (struct pll_param *)para);
			break;
		}
		default:
			MUTEX_UNLOCK();
			//should not happen!
			return (-22);
			break;
	}

	MUTEX_UNLOCK();
	return 0;
}

//---------------------------------------------------------------------------//

static void set_hdmiphy_parameter(struct hdmiphy_param *p_para)
{
	struct mt_analog_hdmitx_r2_attr hdmitx_r2_attr;

	dump_reg(HDMI_TX_REG2);

	TRACE_LOG("HDMI PHY - mute: %u\n", p_para->mute);

	(void)mt_analog_acquire_lock();

	mt_analog_get_attr_no_lock(MT_ANA_INDEX_HDMITX_R2, (void*)&hdmitx_r2_attr);

	hdmitx_r2_attr.mute0 = p_para->mute;
	hdmitx_r2_attr.mute1 = p_para->mute;
	hdmitx_r2_attr.mute2 = p_para->mute;
	hdmitx_r2_attr.mute_clk = p_para->mute;

//TODO:
//	hdmi_tst_attr.mute = p_para->mute;

	mt_analog_set_attr_no_lock(MT_ANA_INDEX_HDMITX_R2, (void*)&hdmitx_r2_attr);

	(void)mt_analog_release_lock();

	dump_reg(HDMI_TX_REG2);
}

static void set_hdmiphy_tst_parameter(struct hdmiphy_tst_param *p_para)
{
	(void)mt_analog_acquire_lock();

	MT_IO_WRITE32(REG_HDMI_TEST, p_para->all);

	(void)mt_analog_release_lock();

	dump_reg(REG_HDMI_TEST);
}

static void set_hdmitx_ch0_parameter(struct hdmitx_ch_param *p_para)
{
	(void)mt_analog_acquire_lock();

	MT_IO_WRITE32(HDMI_TX_REG0, p_para->all);

	(void)mt_analog_release_lock();

	dump_reg(HDMI_TX_REG0);
}

static void set_hdmitx_ch1_parameter(struct hdmitx_ch_param *p_para)
{
	(void)mt_analog_acquire_lock();

	MT_IO_WRITE32(HDMI_TX_REG1, p_para->all);

	(void)mt_analog_release_lock();

	dump_reg(HDMI_TX_REG1);
}

static void set_hdmitx_ch2_parameter(struct hdmitx_ch_param *p_para)
{
	(void)mt_analog_acquire_lock();

	MT_IO_WRITE32(HDMI_TX_REG2, p_para->all);

	(void)mt_analog_release_lock();

	dump_reg(HDMI_TX_REG2);
}

static void set_hdmiclk_parameter(struct hdmiclk_param *p_para)
{
	struct mt_analog_usbaclk_r0_attr usbaclk_r0_attr;

	dump_reg(CLKGEN_USBACLK_REG0);

	TRACE_LOG("HDMI Clk Sel - hdmi_clk_tmds_sel: %u, sel540M: %u, sel2970M: %u\n",
		p_para->hdmi_clk_tmds_sel,
		p_para->sel540M,
		p_para->sel2970M);

	mt_analog_get_attr_lock(MT_ANA_INDEX_USBACLK_R0, (void*)&usbaclk_r0_attr);

	usbaclk_r0_attr.hdmi_clk_tmds_sel = p_para->hdmi_clk_tmds_sel;
	usbaclk_r0_attr.sel540M           = p_para->sel540M;
	usbaclk_r0_attr.sel2970M          = p_para->sel2970M;

	mt_analog_set_attr_unlock(MT_ANA_INDEX_USBACLK_R0, (void*)&usbaclk_r0_attr);

	dump_reg(CLKGEN_USBACLK_REG0);
}

static void set_hdmidcc_parameter(struct hdmidcc_param *p_para)
{
	struct mt_analog_usbaclk_r0_attr usbaclk_r0_attr;

	dump_reg(CLKGEN_USBACLK_REG0);

	TRACE_LOG("HDMI PHY clk DCC - dcc_xpd: %u, dcc_sel: %u\n",
		p_para->dcc_xpd,
		p_para->dcc_sel);

	mt_analog_get_attr_lock(MT_ANA_INDEX_USBACLK_R0, (void*)&usbaclk_r0_attr);

	usbaclk_r0_attr.dcc_xpd = p_para->dcc_xpd;
	usbaclk_r0_attr.dcc_sel = p_para->dcc_sel;

	mt_analog_set_attr_unlock(MT_ANA_INDEX_USBACLK_R0, (void*)&usbaclk_r0_attr);

	dump_reg(CLKGEN_USBACLK_REG0);
}

static void set_vhdclk_parameter(struct vhdclk_param *p_para)
{
	struct mt_analog_pdsys_attr pdsys_attr;

	dump_reg(REG_CLKGEN_PDSYS);

	TRACE_LOG("VHD CLK - div_sel: %u, clk_sel_1: %u, drv0_sel: %u, clk_sel_2: %u\n",
			p_para->div_sel,
			p_para->clk_sel_1,
			p_para->drv0_sel,
			p_para->clk_sel_2);

	mt_analog_get_attr_lock(MT_ANA_INDEX_PDSYS, (void*)&pdsys_attr);

	pdsys_attr.vhd_div_sel	 = p_para->div_sel;
	pdsys_attr.vhd_clk_sel_1 = p_para->clk_sel_1;
	pdsys_attr.reserved1     = p_para->drv0_sel;
	pdsys_attr.vhd_clk_sel_2 = p_para->clk_sel_2;

	mt_analog_set_attr_unlock(MT_ANA_INDEX_PDSYS, (void*)&pdsys_attr);

	dump_reg(REG_CLKGEN_PDSYS);
}

static void set_vsdclk_parameter(struct vsdclk_param *p_para)
{
	struct mt_analog_pdsys_attr pdsys_attr;

	dump_reg(REG_CLKGEN_PDSYS);

	TRACE_LOG("VSD CLK - div_sel: %u, clk_sel_1: %u, drv0_sel: %u, clk_sel_2: %u\n",
			p_para->div_sel,
			p_para->clk_sel_1,
			p_para->drv0_sel,
			p_para->clk_sel_2);

	mt_analog_get_attr_lock(MT_ANA_INDEX_PDSYS, (void*)&pdsys_attr);

	pdsys_attr.vsd_div_sel	 = p_para->div_sel;
	pdsys_attr.vsd_clk_sel_1 = p_para->clk_sel_1;
	pdsys_attr.vsd_drv0_sel  = p_para->drv0_sel;
	pdsys_attr.vsd_clk_sel_2 = p_para->clk_sel_2;

	mt_analog_set_attr_unlock(MT_ANA_INDEX_PDSYS, (void*)&pdsys_attr);

	dump_reg(REG_CLKGEN_PDSYS);
}

static void set_vhdssc_parameter(struct vhdssc_param *p_para)
{
	dump_reg(REG_CLKGEN_VHDSSC);

	TRACE_LOG("VHD SSC - 0x%08X\n", p_para->all);

	(void)mt_analog_acquire_lock();

	MT_IO_WRITE32(REG_CLKGEN_VHDSSC, p_para->all);

	(void)mt_analog_release_lock();

	dump_reg(REG_CLKGEN_VHDSSC);

	CHECK_REGISTER(REG_CLKGEN_VHDSSC, 0, 32, p_para->all);
}

static void set_vsdssc_parameter(struct vsdssc_param *p_para)
{
	struct mt_analog_vsdssc_attr vsdssc_attr;

	dump_reg(REG_CLKGEN_VSDSSC);

	TRACE_LOG("VSD SSC - clk_sel: %u, en_ssc: %u, pd_ssc: %u\n",
			p_para->clk_sel,
			p_para->en_ssc,
			p_para->pd_ssc);

	mt_analog_get_attr_lock(MT_ANA_INDEX_VSDSSC, (void*)&vsdssc_attr);

	vsdssc_attr.clk_sel = p_para->clk_sel;
	vsdssc_attr.en_ssc	= p_para->en_ssc;
	vsdssc_attr.pd_ssc	= p_para->pd_ssc;

	mt_analog_set_attr_unlock(MT_ANA_INDEX_VSDSSC, (void*)&vsdssc_attr);

	dump_reg(REG_CLKGEN_VSDSSC);
}

static void set_vhdpll_parameter(struct vhdpll_param *p_para)
{
	dump_reg(REG_CLKGEN_VHDPLL);

	TRACE_LOG("VHD PLL - 0x%08X\n", p_para->all);

	(void)mt_analog_acquire_lock();

	MT_IO_WRITE32(REG_CLKGEN_VHDPLL, p_para->all);

	(void)mt_analog_release_lock();

	dump_reg(REG_CLKGEN_VHDPLL);

	CHECK_REGISTER(REG_CLKGEN_VHDPLL, 0, 32, p_para->all);
}

static void set_vsdpll_parameter(struct vsdpll_param *p_para)
{
	dump_reg(REG_CLKGEN_VSDPLL);

	TRACE_LOG("VSD PLL - 0x%08X\n", p_para->all);

	(void)mt_analog_acquire_lock();

	MT_IO_WRITE32(REG_CLKGEN_VSDPLL, p_para->all);

	(void)mt_analog_release_lock();

	dump_reg(REG_CLKGEN_VSDPLL);

	CHECK_REGISTER(REG_CLKGEN_VSDPLL, 0, 32, p_para->all);
}

static void set_vclk_src_parameter(struct vclk_source_param *p_para)
{
	struct mt_analog_vhdintp_attr vhd_attr;

	TRACE_LOG("vsd os108 from: %u, tmds sel: %u\n",
		p_para->vsd_os_108M_sel,
		p_para->tmds_sel);

	mt_analog_get_attr_lock(MT_ANA_INDEX_VHDINTP, (void*)&vhd_attr);

	vhd_attr.vsd_os108M_sel = p_para->vsd_os_108M_sel;
	vhd_attr.tmds_sel = p_para->tmds_sel;

	mt_analog_set_attr_unlock(MT_ANA_INDEX_VHDINTP, (void*)&vhd_attr);

	dump_reg(REG_CLKGEN_VHDINTP);
}

static void set_adc_parameter(struct adc_param *p_para)
{
	struct mt_analog_pdsys_attr pdsys_attr;
	struct mt_analog_ethintp_attr ethintp_attr;
	struct mt_analog_adcpll_attr adcpll_attr;

	dump_reg(REG_CLKGEN_PDSYS);
	dump_reg(REG_CLKGEN_ETHINTP);
	dump_reg(REG_CLKGEN_ADCPLL);

	TRACE_LOG("SADC - pd_sadc_1100M: %u, pd_sdemod_270M: %u, sadc_clk_sel: %u\n",
			p_para->sadc.pd_sadc_1100M,
			p_para->sadc.pd_sdemod_270M,
			p_para->sadc.sadc_clk_sel);
	TRACE_LOG("CADC - pd_cadc_270M: %u, pd_cdemod_81M: %u, cadc_clk_sel: %u\n",
			p_para->cadc.pd_cadc_270M,
			p_para->cadc.pd_cdemod_81M,
			p_para->cadc.cadc_clk_sel);
	TRACE_LOG("DRV0 - pd_drv0_clk: %u, drv0_clk_sel: %u, drv0_clk_mux: %u, drv0_clk_81M_mux: %u\n",
			p_para->drv0.pd_drv0_clk,
			p_para->drv0.drv0_clk_sel,
			p_para->drv0.drv0_clk_mux,
			p_para->drv0.drv0_clk_81M_mux);
	TRACE_LOG("DRV1 - pd_drv1_clk: %u, drv1_clk_sel: %u\n",
			p_para->drv1.pd_drv1_clk,
			p_para->drv1.drv1_clk_sel);

	(void)mt_analog_acquire_lock();

	mt_analog_get_attr_no_lock(MT_ANA_INDEX_PDSYS, (void*)&pdsys_attr);
	mt_analog_get_attr_no_lock(MT_ANA_INDEX_ETHINTP, (void*)&ethintp_attr);
	mt_analog_get_attr_no_lock(MT_ANA_INDEX_ADCPLL, (void*)&adcpll_attr);

	pdsys_attr.pd_drv0_clk  = p_para->drv0.pd_drv0_clk;
	pdsys_attr.pd_drv1_clk  = p_para->drv1.pd_drv1_clk;
	pdsys_attr.drv0_clk_sel = p_para->drv0.drv0_clk_sel;
	pdsys_attr.drv1_clk_sel = p_para->drv1.drv1_clk_sel;

	ethintp_attr.drv0_clk_sel     = p_para->drv0.drv0_clk_mux;
	ethintp_attr.sadc_clk_sel     = p_para->sadc.sadc_clk_sel;
	ethintp_attr.drv0_clk_81M_sel = p_para->drv0.drv0_clk_81M_mux;

	adcpll_attr.pd_cadc_270M   = p_para->cadc.pd_cadc_270M;
	adcpll_attr.pd_cdemod_81M  = p_para->cadc.pd_cdemod_81M;
	adcpll_attr.pd_sadc_1100M  = p_para->sadc.pd_sadc_1100M;
	adcpll_attr.pd_sdemod_270M = p_para->sadc.pd_sdemod_270M;
	adcpll_attr.cadc_clk_sel   = p_para->cadc.cadc_clk_sel;

	//1:
	mt_analog_set_attr_no_lock(MT_ANA_INDEX_PDSYS, (void*)&pdsys_attr);
	//2:
	mt_analog_set_attr_no_lock(MT_ANA_INDEX_ETHINTP, (void*)&ethintp_attr);
	//3:
	mt_analog_set_attr_no_lock(MT_ANA_INDEX_ADCPLL, (void*)&adcpll_attr);

	(void)mt_analog_release_lock();

	dump_reg(REG_CLKGEN_PDSYS);
	dump_reg(REG_CLKGEN_ETHINTP);
	dump_reg(REG_CLKGEN_ADCPLL);
}

/* set pll parameter */
static void set_pll_parameter(enum MT_ANALOG_PARAM_INDEX_E index, struct pll_param *p_para)
{
	enum MT_ANALOG_INDEX_E attr_index;
	struct mt_analog_gen_pll_attr attr;

	if (index == ANALOG_PARAM_INDEX_ARMPLL) {
		attr_index = MT_ANA_INDEX_ARMPLL;
	} else if (index == ANALOG_PARAM_INDEX_ADCPLL) {
		attr_index = MT_ANA_INDEX_ADCPLL;
	} else if (index == ANALOG_PARAM_INDEX_USBPLL) {
		attr_index = MT_ANA_INDEX_USBPLL;
	} else if (index == ANALOG_PARAM_INDEX_VHDPLL) {
		attr_index = MT_ANA_INDEX_VHDPLL;
	} else if (index == ANALOG_PARAM_INDEX_VSDPLL) {
		attr_index = MT_ANA_INDEX_VSDPLL;
	} else if (index == ANALOG_PARAM_INDEX_EPHYPLL) {
		attr_index = MT_ANA_INDEX_EPHYPLL;
	} else if (index == ANALOG_PARAM_INDEX_USB3P0PLL) {
		attr_index = MT_ANA_INDEX_USB3P0PLL;
	} else if (index == ANALOG_PARAM_INDEX_REFPLL) {
		attr_index = MT_ANA_INDEX_REFPLL;
	} else if (index == ANALOG_PARAM_INDEX_DDRPLL) {
		attr_index = MT_ANA_INDEX_DDRPLL;
	} else {
		//error!
		return;
	}

	TRACE_LOG("PLL[%d] - div_cal_valid: %u, div_cal: %X, div_front: %X, pre_div_valid: %u, pre_div: %u, div_fb: %X, vco_ext: %u, vco_sel: %u\n",
			index,
			p_para->div_cal_valid,
			p_para->div_cal,
			p_para->div_front,
			p_para->pre_div_valid,
			p_para->pre_div,
			p_para->div_fb,
			p_para->vco_ext,
			p_para->vco_sel);

	mt_analog_get_attr_lock(attr_index, (void*)&attr);

	if (p_para->div_cal_valid)
		attr.div_cal = p_para->div_cal;

	if (p_para->pre_div_valid)
		attr.pre_div = p_para->pre_div;

	attr.div_front = p_para->div_front;
	attr.div_fb    = p_para->div_fb;
	attr.vco_sel   = p_para->vco_sel;
	attr.vco_ext   = p_para->vco_ext;

	mt_analog_set_attr_unlock(attr_index, (void*)&attr);
}

//---------------------------------------------------------------------------//

int mt_analog_set_parameter(enum MT_ANALOG_PARAM_INDEX_E index, void *para)
{
	if (index < ANALOG_PARAM_INDEX_HDMIPHY || index >= ANALOG_PARAM_INDEX_MAX)
	{
		MT_LOGE("error: %s - invalid parameter(%d)!\n", __FUNCTION__, index);
		return (-22);
	}

	if (para == NULL)
	{
		MT_LOGE("error: %s - para is null!\n", __FUNCTION__);
		return (-22);
	}

	MT_LOGI("%s: %d(%s)\n", __FUNCTION__, index, get_index_str(index));

	MUTEX_LOCK();

	switch (index)
	{
		case ANALOG_PARAM_INDEX_HDMIPHY:
		{
			set_hdmiphy_parameter((struct hdmiphy_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMIPHY_TST:
		{
			set_hdmiphy_tst_parameter((struct hdmiphy_tst_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMITX_CH0:
		{
			set_hdmitx_ch0_parameter((struct hdmitx_ch_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMITX_CH1:
		{
			set_hdmitx_ch1_parameter((struct hdmitx_ch_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMITX_CH2:
		{
			set_hdmitx_ch2_parameter((struct hdmitx_ch_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMICLK:
		{
			set_hdmiclk_parameter((struct hdmiclk_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_HDMIDCC:
		{
			set_hdmidcc_parameter((struct hdmidcc_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_VHDCLK:
		{
			set_vhdclk_parameter((struct vhdclk_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_VSDCLK:
		{
			set_vsdclk_parameter((struct vsdclk_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_VHDSSC:
		{
			set_vhdssc_parameter((struct vhdssc_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_VSDSSC:
		{
			set_vsdssc_parameter((struct vsdssc_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_RAW_VHDPLL:
		{
			set_vhdpll_parameter((struct vhdpll_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_RAW_VSDPLL:
		{
			set_vsdpll_parameter((struct vsdpll_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_VCLK_SRC:
		{
			set_vclk_src_parameter((struct vclk_source_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_ADC:
		{
			set_adc_parameter((struct adc_param *)para);
			break;
		}
		case ANALOG_PARAM_INDEX_ADCPLL:
		case ANALOG_PARAM_INDEX_USBPLL:
		case ANALOG_PARAM_INDEX_VHDPLL:
		case ANALOG_PARAM_INDEX_VSDPLL:
		case ANALOG_PARAM_INDEX_EPHYPLL:
		case ANALOG_PARAM_INDEX_USB3P0PLL:
		case ANALOG_PARAM_INDEX_REFPLL:
		case ANALOG_PARAM_INDEX_DDRPLL:
		case ANALOG_PARAM_INDEX_ARMPLL:
		{
			set_pll_parameter(index, (struct pll_param *)para);
			break;
		}
		default:
			MUTEX_UNLOCK();
			//should not happen!
			return (-22);
			break;
	}

	MUTEX_UNLOCK();
	return 0;
}

