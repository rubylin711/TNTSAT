/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* drv_snd_ao.c <2022-10-24>																*/
/********************************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <uapi/linux/sched/types.h>
#include "mt_drv_mmz.h"
#include "mt_common.h"
#include "mt_drv_dev.h"
#include "mt_drv_struct.h"
#include "mt_drv_module.h"
#include "mt_drv_dma.h"
#include "drv_snd_reg.h"
#include "drv_snd_ao.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_drv_dump.h"
#include "snd_fpga_test.h"
 

mt_s32 fpga_test_hdmi_config(aud_param_t *param)
{
	mt_s32 ret = MT_SUCCESS;

#ifdef AOUT_FPGA_TEST
	HDMI_AUDIO_ATTR_S stHDMIAttr;
	static HDMI_EXPORT_FUNC_S *pstHdmiFunc = NULL;

	//get hdmi function pointer
	if (NULL == pstHdmiFunc)
	{
		if (MT_SUCCESS != mt_drv_module_getfunction(MT_ID_HDMI, (mt_void**)&pstHdmiFunc))
		{
			pr_err("get hdmi func pointer FAIL\n");
			return MT_FAILURE;
		}
	}

	//audio to hdmi config
	if (pstHdmiFunc && pstHdmiFunc->pfnHdmiAudioChange)
	{
		memset(&stHDMIAttr, 0, sizeof(HDMI_AUDIO_ATTR_S));
		if (pstHdmiFunc && pstHdmiFunc->pfnHdmiGetAoAttr)
		{
			(pstHdmiFunc->pfnHdmiGetAoAttr)(MT_UNF_HDMI_ID_0, &stHDMIAttr);
		}

		//data format
		switch (param->fmt) //TODO...check index depend on hdmi path selected
		{
		case AUD_SND_DATA_FMT_PCM:
			stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM;
			break;
		case AUD_SND_DATA_FMT_AC3:
			stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3;
			break;
		case AUD_SND_DATA_FMT_EAC3:
			stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP;
			break;
		case AUD_SND_DATA_FMT_DTS:
			stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS;
			break;
		default:
			stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM;
			break;
		}

		//source interface
		if (MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM == stHDMIAttr.enAudioCode)
		{
			stHDMIAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_I2S;
		}
		else
		{
			stHDMIAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_SPDIF;
		}//TODO...case for HDMI_AUDIO_INTERFACE_HBR

		//sample rate
		if (MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP == stHDMIAttr.enAudioCode)
		{
			stHDMIAttr.enSampleRate = 4 * param->sample_rate;
		}
		else
		{
			stHDMIAttr.enSampleRate = param->sample_rate;
		}

		//others
		stHDMIAttr.enBitDepth = param->bitdepth;
		stHDMIAttr.u32Channels = param->ch_num;
		if (2 < stHDMIAttr.u32Channels)
		{
			stHDMIAttr.bIsMultiChannel = MT_TRUE;
		}
		else
		{
			stHDMIAttr.bIsMultiChannel = MT_FALSE;
		}
		stHDMIAttr.u8DownSampleParm = 0;
		stHDMIAttr.u8I2SCtlVbit = 0;

		//config implement
		if (MT_SUCCESS == (pstHdmiFunc->pfnHdmiAudioChange)(MT_UNF_HDMI_ID_0, &stHDMIAttr))
		{
			ret = MT_SUCCESS;
			pr_err("CFG: fmt<%x>, intf<%d>, samplerate<%d>, channel<%d>, bitdepth<%d>\n",
			       stHDMIAttr.enAudioCode, stHDMIAttr.enSoundIntf, stHDMIAttr.enSampleRate, stHDMIAttr.u32Channels, stHDMIAttr.enBitDepth);
		}
		else
		{
			ret = MT_FAILURE;
			//TODO...max reconfig check
		}
	}

	pr_err("audio config hdmi %s\n", MT_SUCCESS == ret ? "SUCCESS" : "FAIL");
#endif

	return ret;
}

#define REG_AOUT_INTR_ADDR 0xbf4900a0
irqreturn_t func_aout_irq_ap(mt_s32 irq, mt_void *dev_id)
{
	mt_u32 state = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(REG_AOUT_INTR_ADDR));
	printk(KERN_ERR "<TN>[AUD]apcpu aout irq reg(a0H)=[0x%08x] \n",  state);
	HAL_PUT_U32((volatile mt_u32 *)SYMPHONY_IO_VA(REG_AOUT_INTR_ADDR), 0x3f00);
	return (IRQ_HANDLED);
}

irqreturn_t aout_isr_op_ree(mt_s32 irq, mt_void *dev_id)
{
	mt_u32 op_irq_masks_clrs_ree = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf4901C0));
	mt_u32 op_irq_flags_ree = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf4901C4));
	pr_err("<TN>AOUT-REE-INT:irq_masks_clrs[%08x]irq_flags[%08x]\n", op_irq_masks_clrs_ree, op_irq_flags_ree);

	//set mask and clear irq
	HAL_PUT_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf4901C0), 0xffff0000);

	pr_err("<TN>AOUT-REE-INT: 1C0H[%08x], 1C4H[%08x]\n",
	       HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf4901C0)),
	       HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf4901C4)));
	return (IRQ_HANDLED);
}

int fpga_test_request_irq(void)
{
#ifdef AOUT_FPGA_TEST
	if (0 != request_irq(IRQ_AOUT_ID, func_aout_irq_ap, IRQF_TRIGGER_HIGH, "aout_irq", MT_NULL))
	{
		pr_err("<TN>[AUD]avcpu aout irq(#%d) register FAIL!\n",  IRQ_AOUT_ID);
	}
	else
	{
		pr_err("<TN>[AUD]avcpu aout irq(#%d) register SUCCESS\n",	IRQ_AOUT_ID);
	}

	//aout irq register for fpga test
	if (0 != request_irq(IRQ_AOUT_OP_REE_ID, aout_isr_op_ree, IRQF_TRIGGER_HIGH, "aout_op_ree", MT_NULL))
	{
		pr_err("<TN>aout irq register FAIL!\n");
	}
	else
	{
		pr_err("<TN>aout irq register SUCCESS\n");
	}
#endif

	return 0;
}


