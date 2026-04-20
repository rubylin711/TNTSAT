/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <mach/hardware.h>

/* Unf headers */
#include "mt_error_mpi.h"

/* Drv headers */
#include "mt_kernel_adapt.h"
#include "mt_drv_ao.h"
#include "mt_drv_ai.h"
#include "drv_ao_ioctl.h"
#include "drv_ao_ext.h"
#include "mt_module_debug.h"
//#include "drv_adsp_ext.h"
#include "drv_ao_private.h"

//#include "mt_audsp_aoe.h"
#include "hal_aoe.h"
#include "hal_cast.h"
#include "hal_aiao.h"

#include "drv_ao_op.h"
#include "drv_ao_track.h"
#include "drv_ao_aef.h"
#include "audio_util.h"
#if defined (MT_AUDIO_AI_SUPPORT)
#include "drv_ai_private.h"
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "drv_gpio_ioctl.h"
#endif

#include "drv_hdmi_ext.h"
#include "drv_hdmi_ioctl.h"

//#ifdef MT_SND_CAST_SUPPORT
#include "drv_ao_cast.h"
//drv#endif
#include "mt_drv_file.h"

#include "hal_aoe_func.h"

#ifdef CONFIG_MT_CHIP_ARIA
#include <mach/aria_io.h>
#else
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

#ifdef MT_ALSA_AO_SUPPORT
#include <linux/platform_device.h>

extern int snd_soc_suspend(struct device *dev);	//kernel inteface
extern int snd_soc_resume(struct device *dev);
extern struct platform_device *hisi_snd_device;

#define PM_LOW_SUSPEND_FLAG   0x5FFFFFFF   		//TODO
static  MT_BOOL bu32shallowSuspendActive = MT_FALSE;
#endif
#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

struct file  g_filp;
#if defined(MT_AIAO_VERIFICATION_SUPPORT)
#include "drv_aiao_ioctl_veri.h"
extern mt_void AIAO_VERI_Open(mt_void);
extern mt_void AIAO_VERI_Release(mt_void);
extern mt_s32  AIAO_VERI_ProcRead(struct seq_file *p, mt_void *v);
extern mt_s32 AIAO_VERI_ProcessCmd( struct inode *inode, struct file *file, mt_u32 cmd, mt_void *arg );
#endif

static mt_s32 AO_RegProc(mt_u32 u32Snd);
static mt_void AO_UnRegProc(mt_u32 u32Snd);
mt_s32 AO_DRV_Resume(basedev_s * pdev);
mt_s32 AO_DRV_Suspend(basedev_s * pdev,pm_message_t   state);
mt_s32 AO_Track_GetDefAttr(MT_UNF_AUDIOTRACK_ATTR_S * pstDefAttr);
mt_s32 AO_Track_AllocHandle(mt_handle *phHandle, struct file *pstFile, AO_Track_Create_Param_S_PTR p_param);
mt_void AO_Track_FreeHandle(mt_handle hHandle);
mt_s32 AO_Track_PreCreate(MT_UNF_SND_E enSound, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr,
                       MT_BOOL bAlsaTrack, AO_BUF_ATTR_S *pstBuf, mt_handle hTrack);
mt_s32 AO_Track_Destory(mt_u32 u32TrackID);
mt_s32 AO_Track_Start(mt_u32 u32TrackID);
mt_s32 AO_Track_Stop(mt_u32 u32TrackID);
mt_s32 AO_Track_SendData(mt_u32 u32TrackID, MT_UNF_AO_FRAMEINFO_S * pstAOFrame);


mt_s32 AO_DRV_Kopen(struct file  *file);
mt_s32 AO_DRV_Krelease(struct file  *file);
mt_s32 AO_Snd_Kclose(MT_UNF_SND_E  arg, struct file *file);
mt_s32 AO_Snd_Kclose(MT_UNF_SND_E  arg, struct file *file);
static mt_s32 AO_SND_SetTrackMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E enMode);

MT_DECLARE_MUTEX(g_AoMutex);


/*!
  volume max
  */
#define AUDIO_VOLUME_MAX 150

/*!
  volume max safe
  */
#define AUDIO_VOLUME_MAX_SAFE 100

#define AUD_VOLUME_STEP_COEF_1_100_99_ARIA  1.065 // (0x8000 / 64 ) ^  1/99

#define AUD_VOLUME_STEP_COEF_100_150_50_ARIA  1.086734

#define AUD_VOLUME_STEP_COEF_100_150_50_ARIA_INT  1086   //(2 ^ 6) ^ (1/50) * 1000

#define AUDIO_VOLUME_MAX_VALUE ((1 << 21) - 1)

#define AUDIO_VOLUME_MAX_VALUE_SAFE 0x7fff // 1 << 15

//#define AUD_VOLUME_STEP_COEF_1_100_99_ARIA_INT  1065 // (0x8000 / 64 ) ^  1/99     * 1000
#define AUD_VOLUME_STEP_COEF_1_100_99_ARIA_INT  137999 // (0x8000 / base is 200  ) ^  1/99     * AUD_VOLUME_EF

#define AUD_VOLUME_EF 0x20000  //( 0x20000 * 0x7fff  < 0xffffffff)

static mt_u8 g_audio_volume = 0;

static MT_BOOL  g_audio_mute = MT_FALSE;
static MT_BOOL  g_audio_mute_hdmi   = MT_FALSE;
static MT_BOOL  g_audio_mute_spdif  = MT_FALSE;

static AO_GLOBAL_PARAM_S s_stAoDrv =
{
    .u32TrackNum         =  0,
    .u32SndNum           =  0,
    .atmOpenCnt          = ATOMIC_INIT(0),
    .bReady              = MT_FALSE,

    .pstProcParam        = MT_NULL,
    //minnan remove
    //.pAdspFunc           = MT_NULL,
    .pstPDMFunc			 = MT_NULL,
    .stExtFunc           =
    {
		.pfnAO_DrvResume = AO_DRV_Resume,
		.pfnAO_DrvSuspend = AO_DRV_Suspend,

		.pfnAO_TrackGetDefAttr = AO_Track_GetDefAttr,
		.pfnAO_TrackAllocHandle = AO_Track_AllocHandle,
		.pfnAO_TrackFreeHandle = AO_Track_FreeHandle,

		.pfnAO_TrackCreate = AO_Track_PreCreate,
		.pfnAO_TrackDestory= AO_Track_Destory,
		.pfnAO_TrackStart = AO_Track_Start,
		.pfnAO_TrackStop = AO_Track_Stop,
		.pfnAO_TrackSendData = AO_Track_SendData,
    }
};


/***************************** Original Static Definition *****************************/

static void audio_get_volume_gainq(mt_u8 *p_gainq, mt_u16 *p_volume, mt_u8 percent_l)
{
  mt_u8 gainQ = 0;
  mt_u32 volume = 0;
  u8 step;

  // percent_l = 0; vol is 0,do nothing; else do something
    if(percent_l)
    {
      //100~150
      if(percent_l > AUDIO_VOLUME_MAX_SAFE)
      {
        volume = AUDIO_VOLUME_MAX_VALUE;
        step = AUDIO_VOLUME_MAX - percent_l;

        while(step--)
        {
          volume *= 1000;
          volume /= AUD_VOLUME_STEP_COEF_100_150_50_ARIA_INT;
          //MT_INFO_AIAO("\n level %d  volume 0x%x \n", (step + percent_l), volume);
        }

      }
      else //1~100
      {
        volume = AUDIO_VOLUME_MAX_VALUE_SAFE;
        step = AUDIO_VOLUME_MAX_SAFE - percent_l;

        while(step--)
        {
          volume *= AUD_VOLUME_EF;
          volume /= AUD_VOLUME_STEP_COEF_1_100_99_ARIA_INT;
          //MT_INFO_AIAO("\n level %d  volume 0x%x \n", (step + percent_l), volume);
        }
      }

      #if 0
      //dolby policy
      if((AUDIO_AC3_VSB == p_audio_priv->aud_format) ||
        (AUDIO_EAC3 == p_audio_priv->aud_format))
      {
        //OS_PRINTF("\n dolby vol  policy\n");
        volume *= AUD_VOLUME_DOLBY_COEF;
      }
      else
      {
        //OS_PRINTF("\n vol policy nooooooooooooooooo dolby\n");
      }
      #endif

      //get gainq and volume
      while(volume > 0xffff)
      {
        gainQ++;
        volume >>= 1;
      }
    }

    *p_gainq = gainQ;
    *p_volume = (u16) volume;



    return;
}



static mt_s32 audio_set_volume(mt_u8 volume)
{
  mt_u8 gainq = 0;
  mt_u16 scale = 0;

  if( (volume > AUDIO_VOLUME_MAX_SAFE))
      return MT_FAILURE;

	//printk("%s: Enter, vol=%d\n",__FUNCTION__,volume);

#if 0
    audio_get_volume_gainq(&gainq, &scale, volume);

    reg_sym_linux_volume_reg_gainq_bit( gainq);
    //must delay
    mdelay(10);
    reg_sym_linux_volume_reg_scale_bit( scale);

#else

	if (volume == 0)
	{
	    reg_sym_linux_volume_reg(0, 0, 0);
	}
	else
	{
	    audio_get_volume_gainq(&gainq, &scale, volume);

		//printk("%s: Enter, gainq=0x%x, scale=0x%x\n",__FUNCTION__,gainq,scale);

		reg_sym_linux_volume_reg(gainq, scale, 0);
    }

	//udelay(1000); //-OK
	//udelay(500);	//-OK
	//udelay(200);	//-OK
	//udelay(100);	//-OK
	udelay(50);		//-OK

#endif

    return MT_SUCCESS;
}


static SND_CARD_STATE_S * SND_CARD_GetCard(MT_UNF_SND_E enSound)
{
    return s_stAoDrv.astSndEntity[enSound].pCard;
}
static MT_UNF_SND_E SND_CARD_GetSnd(SND_CARD_STATE_S *pCard)
{
    mt_u32 i;

    for(i = 0; i < AO_MAX_TOTAL_SND_NUM; i++)
    {
        if(s_stAoDrv.astSndEntity[i].pCard  ==  pCard)
        {
            return i;
        }
    }
    return AO_MAX_TOTAL_SND_NUM;
}
#if 0  //unuse code
static mt_void AO_Snd_FreeHandle2(MT_UNF_SND_E enSound, struct file *pstFile)
{
   mt_u32 u32AefId;
   SND_CARD_STATE_S * pCard = s_stAoDrv.astSndEntity[enSound].pCard;
   mt_u32 u32FileId = ((DRV_AO_STATE_S *)(pstFile->private_data))->u32FileId[enSound];


    if(0 == atomic_read(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt))
    {
        for(u32AefId = 0; u32AefId < AFLT_MAX_CHAN_NUM; u32AefId++)
        {
            if(pCard->u32AttAef & ((mt_u32)1L << u32AefId))
            {
                AEF_DetachSnd(pCard, u32AefId);
            }
        }

        AUTIL_AO_FREE(MT_ID_AO, pCard);
        s_stAoDrv.astSndEntity[enSound].pCard = MT_NULL;
    	s_stAoDrv.u32SndNum--;
    }

    if(u32FileId < SND_MAX_OPEN_NUM)
    {
        s_stAoDrv.astSndEntity[enSound].u32File[u32FileId] = MT_NULL;
    }

    ((DRV_AO_STATE_S *)(pstFile->private_data))->u32FileId[enSound] = AO_SND_FILE_NOUSE_FLAG;

    return;
}
#endif
static mt_void AO_Snd_FreeHandle(MT_UNF_SND_E enSound, struct file *pstFile)
{
   //mt_u32 u32AefId;
  // SND_CARD_STATE_S * pCard = s_stAoDrv.astSndEntity[enSound].pCard;
   //mt_u32 u32FileId = ((DRV_AO_STATE_S *)(pstFile->private_data))->u32FileId[enSound];

    atomic_dec(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt);

    if(0 == atomic_read(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt))
   {
	//   AUTIL_AO_FREE(MT_ID_AO, pCard);  //for bug 125801 don't free it.
        s_stAoDrv.astSndEntity[enSound].pCard = MT_NULL;
    	//s_stAoDrv.u32SndNum--;

      s_stAoDrv.astSndEntity[enSound].u32File[0] = MT_NULL;
      ((DRV_AO_STATE_S *)(pstFile->private_data))->u32FileId[enSound] = AO_SND_FILE_NOUSE_FLAG;
    }
    else
    {
      MT_INFO_AO("\n\n\n\n     in  AO_Snd_FreeHandle %d \n\n\n\n", atomic_read(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt));
    }


    return;
}
#if 0 //unuse code
static mt_s32 SNDGetFreeFileId(MT_UNF_SND_E enSound)
{
    mt_u32 i;

    for(i = 0; i < SND_MAX_OPEN_NUM; i++)
    {
        if(s_stAoDrv.astSndEntity[enSound].u32File[i]  == MT_NULL)
        {
            return i;
        }
    }

    return SND_MAX_OPEN_NUM;
}

static mt_s32 AO_Snd_AllocHandle_old(MT_UNF_SND_E enSound, struct file *pstFile)
{
    SND_CARD_STATE_S *pCard;
    mt_u32 u32FreeId;

    if (enSound >= MT_UNF_SND_BUTT)
    {
        MT_ERR_AO("Bad param!\n");
        goto err0;
    }

    /* Check ready flag */
    if (s_stAoDrv.bReady != MT_TRUE)
    {
        MT_ERR_AO("Need open first!\n");
        goto err0;
    }

    u32FreeId = SNDGetFreeFileId(enSound);
    if(u32FreeId >= SND_MAX_OPEN_NUM)
	{
	    MT_ERR_AO("Get free file id faied!\n");
    	goto err0;
	}
    if(AO_SND_FILE_NOUSE_FLAG == ((DRV_AO_STATE_S *)(pstFile->private_data))->u32FileId[enSound])
    {
        ((DRV_AO_STATE_S *)(pstFile->private_data))->u32FileId[enSound] = u32FreeId;
        s_stAoDrv.astSndEntity[enSound].u32File[u32FreeId] = (ulong)pstFile;
    }
    /* Allocate new snd resource */
    if (0 == atomic_read(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt))
    {
        pCard = (SND_CARD_STATE_S *)AUTIL_AO_MALLOC(MT_ID_AO, sizeof(SND_CARD_STATE_S), GFP_KERNEL);
        if(MT_NULL == pCard)
        {
            s_stAoDrv.astSndEntity[enSound].u32File[u32FreeId] = MT_NULL;
            MT_ERR_AO("Kmalloc card failed!\n");
            goto err0;
        }
        else
        {
            s_stAoDrv.astSndEntity[enSound].pCard = pCard;
        }

        s_stAoDrv.u32SndNum++;
    }

    return MT_SUCCESS;

err0:
    return MT_FAILURE;
}
#endif 

static mt_s32 AO_Snd_AllocHandle(MT_UNF_SND_E enSound, struct file *pstFile)
{
    SND_CARD_STATE_S *pCard;
    mt_u32 u32FreeId;

    if (enSound >= MT_UNF_SND_BUTT)
    {
        MT_ERR_AO("Bad param!\n");
        goto err0;
    }

    /* Check ready flag */
    if (s_stAoDrv.bReady != MT_TRUE)
    {
        MT_ERR_AO("Need open first!\n");
        goto err0;
    }


    u32FreeId = 0;   //SNDGetFreeFileId(enSound);


    /* Allocate new snd resource */

    if (0 == atomic_read(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt))
    {
        ((DRV_AO_STATE_S *)(pstFile->private_data))->u32FileId[enSound] = u32FreeId;
        s_stAoDrv.astSndEntity[enSound].u32File[u32FreeId] = (ulong)pstFile;
		
		//for bug 125801 don't free it.
		if(0 == atomic_read(&s_stAoDrv.astSndEntity[enSound].first_open))
		{
			atomic_inc(&s_stAoDrv.astSndEntity[enSound].first_open);
	        pCard = (SND_CARD_STATE_S *)AUTIL_AO_MALLOC(MT_ID_AO, sizeof(SND_CARD_STATE_S), GFP_KERNEL);
			s_stAoDrv.astSndEntity[enSound].static_pCard = pCard;
		}
		else
		{
			pCard = s_stAoDrv.astSndEntity[enSound].static_pCard;
		}
		
        if(MT_NULL == pCard)
        {
            s_stAoDrv.astSndEntity[enSound].u32File[u32FreeId] = MT_NULL;
            MT_ERR_AO("Kmalloc card failed!\n");
            goto err0;
        }
        else
        {
            s_stAoDrv.astSndEntity[enSound].pCard = pCard;
        }

        //s_stAoDrv.u32SndNum++;
    }
    else
      MT_INFO_AO("\n atmUseTotalCnt  %d %d \n", s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt, atomic_read(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt));

    atomic_inc(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt);


    return MT_SUCCESS;

err0:
    return MT_FAILURE;
}



/******************************Snd process FUNC*************************************/
MT_BOOL AOCheckOutPortIsAttached(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort)
{
    mt_u32 u32Snd;
    mt_u32 u32Port;
    SND_CARD_STATE_S * pCard;
    for(u32Snd = 0; u32Snd < (mt_u32)MT_UNF_SND_BUTT; u32Snd++)
    {
        if(u32Snd != (mt_u32)enSound)
        {
            pCard = SND_CARD_GetCard((MT_UNF_SND_E)u32Snd);
            if (MT_NULL != pCard)
            {
                for(u32Port = 0; u32Port < pCard->stUserOpenParam.u32PortNum; u32Port++)
                {
                    if(enOutPort == pCard->stUserOpenParam.stOutport[u32Port].enOutPort)
                    {
                        return MT_TRUE;
                    }
                }
            }
        }
    }

    return MT_FALSE;
}

mt_s32 AOGetSndDefOpenAttr(AO_SND_OpenDefault_Param_S *pstSndDefaultAttr)
{
#if defined(CMTP_TYPE_hi3751v100)
	pstSndDefaultAttr->stAttr.u32PortNum = 5;

    pstSndDefaultAttr->stAttr.stOutport[0].enOutPort = MT_UNF_SND_OUTPUTPORT_DAC0;
    pstSndDefaultAttr->stAttr.stOutport[0].unAttr.stDacAttr.pPara = MT_NULL;

    pstSndDefaultAttr->stAttr.stOutport[1].enOutPort = MT_UNF_SND_OUTPUTPORT_EXT_DAC1;
    pstSndDefaultAttr->stAttr.stOutport[1].unAttr.stSpdifAttr.pPara = MT_NULL;
    pstSndDefaultAttr->stAttr.stOutport[2].enOutPort = MT_UNF_SND_OUTPUTPORT_EXT_DAC2;
    pstSndDefaultAttr->stAttr.stOutport[2].unAttr.stHDMIAttr.pPara = MT_NULL;
    pstSndDefaultAttr->stAttr.stOutport[3].enOutPort = MT_UNF_SND_OUTPUTPORT_EXT_DAC3;/*Analog Amp*/
    pstSndDefaultAttr->stAttr.stOutport[3].unAttr.stDacAttr.pPara = MT_NULL;
    pstSndDefaultAttr->stAttr.stOutport[4].enOutPort = MT_UNF_SND_OUTPUTPORT_SPDIF0;
    pstSndDefaultAttr->stAttr.stOutport[4].unAttr.stSpdifAttr.pPara = MT_NULL;
#else
    mt_s32 s32Ret = MT_SUCCESS;
    MT_UNF_PDM_SOUND_PARAM_S stSoundPdmParam;
    if (s_stAoDrv.pstPDMFunc && s_stAoDrv.pstPDMFunc->pfnGetSoundParam)
    {
        mt_u32 i;
        s32Ret = (s_stAoDrv.pstPDMFunc->pfnGetSoundParam)(pstSndDefaultAttr->enSound, &stSoundPdmParam);
        if (MT_SUCCESS == s32Ret)
        {
            if (stSoundPdmParam.u32PortNum <= MT_UNF_SND_OUTPUTPORT_MAX)
            {
                pstSndDefaultAttr->stAttr.u32PortNum = stSoundPdmParam.u32PortNum;
                for (i = 0; i < stSoundPdmParam.u32PortNum; i++)
                {
                    pstSndDefaultAttr->stAttr.stOutport[i].enOutPort = stSoundPdmParam.stOutport[i].enOutPort;
                    switch (pstSndDefaultAttr->stAttr.stOutport[i].enOutPort)
                    {
                        case MT_UNF_SND_OUTPUTPORT_DAC0:
                            MT_INFO_AO(">> MT_UNF_SND_OUTPUTPORT_DAC0\n");
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stDacAttr.pPara = MT_NULL;
                            pstSndDefaultAttr->stAttr.enSampleRate = MT_UNF_SAMPLE_RATE_48K;
                            break;
                        case MT_UNF_SND_OUTPUTPORT_SPDIF0:
                            MT_INFO_AO(">> MT_UNF_SND_OUTPUTPORT_SPDIF0\n");
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stSpdifAttr.pPara = MT_NULL;
                            pstSndDefaultAttr->stAttr.enSampleRate = MT_UNF_SAMPLE_RATE_48K;
                            break;
                        case MT_UNF_SND_OUTPUTPORT_HDMI0:
                            MT_INFO_AO(">> MT_UNF_SND_OUTPUTPORT_HDMI0\n");
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stHDMIAttr.pPara = MT_NULL;
                            pstSndDefaultAttr->stAttr.enSampleRate = MT_UNF_SAMPLE_RATE_48K;
                            break;
                        case MT_UNF_SND_OUTPUTPORT_I2S0:	//default support
                            MT_INFO_AO(">> MT_UNF_SND_OUTPUTPORT_I2S0\n");
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.bMaster = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.bMaster;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enI2sMode = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enI2sMode;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enMclkSel = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enMclkSel;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enBclkSel = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enBclkSel;
                            /*Compatible with old base param,  force to 2ch output */
							//pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enChannel = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enChannel;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enChannel = MT_UNF_I2S_CHNUM_2;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enBitDepth = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enBitDepth;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.bPcmSampleRiseEdge = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.bPcmSampleRiseEdge;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enPcmDelayCycle = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enPcmDelayCycle;
                            MT_INFO_AO("bMaster:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.bMaster);
                            MT_INFO_AO("enI2sMode:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enI2sMode);
                            MT_INFO_AO("enMclkSel:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enMclkSel);
                            MT_INFO_AO("enBclkSel:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enBclkSel);
                            MT_INFO_AO("enChannel:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enChannel);
                            MT_INFO_AO("enBitDepth:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enBitDepth);
                            MT_INFO_AO("bPcmSampleRiseEdge:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.bPcmSampleRiseEdge);
                            MT_INFO_AO("enPcmDelayCycle:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enPcmDelayCycle);
                            break;
                        case MT_UNF_SND_OUTPUTPORT_I2S1:	//default support
                            MT_INFO_AO(">> MT_UNF_SND_OUTPUTPORT_I2S1\n");
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.bMaster = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.bMaster;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enI2sMode = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enI2sMode;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enMclkSel = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enMclkSel;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enBclkSel = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enBclkSel;
                            /* I2S do not support 1&8 channel output, so force to 2ch output */
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enChannel = MT_UNF_I2S_CHNUM_2;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enBitDepth = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enBitDepth;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.bPcmSampleRiseEdge = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.bPcmSampleRiseEdge;
                            pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enPcmDelayCycle = stSoundPdmParam.stOutport[i].unAttr.stI2sAttr.stAttr.enPcmDelayCycle;
                            MT_INFO_AO("bMaster:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.bMaster);
                            MT_INFO_AO("enI2sMode:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enI2sMode);
                            MT_INFO_AO("enMclkSel:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enMclkSel);
                            MT_INFO_AO("enBclkSel:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enBclkSel);
                            MT_INFO_AO("enChannel:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enChannel);
                            MT_INFO_AO("enBitDepth:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enBitDepth);
                            MT_INFO_AO("bPcmSampleRiseEdge:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.bPcmSampleRiseEdge);
                            MT_INFO_AO("enPcmDelayCycle:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].unAttr.stI2sAttr.stAttr.enPcmDelayCycle);
                            break;
                        default:
                            MT_WARN_AO("SND Not support OUTPUTPORT:0x%x\n", pstSndDefaultAttr->stAttr.stOutport[i].enOutPort);
                            break;
                    }
                }

                return MT_SUCCESS;
            }
            MT_ERR_AO("Get PDM param invalid u32PortNum=0x%x\n", stSoundPdmParam.u32PortNum);
        }
        MT_WARN_AO("Get PDM param failed, use default param!\n");
    }
    MT_WARN_AO("Get PDMFunc Symbol failed, use default param!\n");

#if 0
    pstSndDefaultAttr->stAttr.u32PortNum = 2;

    pstSndDefaultAttr->stAttr.stOutport[0].enOutPort = MT_UNF_SND_OUTPUTPORT_DAC0;
    pstSndDefaultAttr->stAttr.stOutport[0].unAttr.stDacAttr.pPara = MT_NULL;
    pstSndDefaultAttr->stAttr.stOutport[1].enOutPort = MT_UNF_SND_OUTPUTPORT_SPDIF0;
    pstSndDefaultAttr->stAttr.stOutport[1].unAttr.stSpdifAttr.pPara = MT_NULL;

#else
    pstSndDefaultAttr->stAttr.u32PortNum = 3;
    pstSndDefaultAttr->stAttr.stOutport[0].enOutPort = MT_UNF_SND_OUTPUTPORT_DAC0;
    pstSndDefaultAttr->stAttr.stOutport[0].unAttr.stDacAttr.pPara = MT_NULL;

    /*pstSndDefaultAttr->stAttr.stOutport[1].enOutPort = MT_UNF_SND_OUTPUTPORT_EXT_DAC1;
    pstSndDefaultAttr->stAttr.stOutport[1].unAttr.stSpdifAttr.pPara = MT_NULL;
    pstSndDefaultAttr->stAttr.stOutport[2].enOutPort = MT_UNF_SND_OUTPUTPORT_EXT_DAC2;
    pstSndDefaultAttr->stAttr.stOutport[2].unAttr.stHDMIAttr.pPara = MT_NULL;
    pstSndDefaultAttr->stAttr.stOutport[3].enOutPort = MT_UNF_SND_OUTPUTPORT_EXT_DAC3;//Analog Amp
    pstSndDefaultAttr->stAttr.stOutport[3].unAttr.stDacAttr.pPara = MT_NULL; */
    pstSndDefaultAttr->stAttr.stOutport[1].enOutPort = MT_UNF_SND_OUTPUTPORT_SPDIF0;
    pstSndDefaultAttr->stAttr.stOutport[1].unAttr.stSpdifAttr.pPara = MT_NULL;
    pstSndDefaultAttr->stAttr.stOutport[2].enOutPort = MT_UNF_SND_OUTPUTPORT_HDMI0;
    pstSndDefaultAttr->stAttr.stOutport[2].unAttr.stHDMIAttr.pPara = MT_NULL;
#endif
#endif

    pstSndDefaultAttr->stAttr.enSampleRate = MT_UNF_SAMPLE_RATE_48K;

#if 0
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].enOutPort = MT_UNF_SND_OUTPUTPORT_I2S0;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.bMaster = MT_TRUE;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enI2sMode = MT_UNF_I2S_STD_MODE;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enMclkSel = MT_UNF_I2S_MCLK_256_FS;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enBclkSel = MT_UNF_I2S_BCLK_4_DIV;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enChannel = MT_UNF_I2S_CHNUM_2;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enBitDepth = MT_UNF_I2S_BIT_DEPTH_16;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.bPcmSampleRiseEdge = MT_TRUE;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enPcmDelayCycle = MT_UNF_I2S_PCM_1_DELAY;
    pstSndDefaultAttr->stAttr.u32PortNum++;
#endif
#if defined(CMTP_TYPE_hi3751v100)
#if defined(MT_I2S1_SUPPORT)
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].enOutPort = MT_UNF_SND_OUTPUTPORT_I2S1;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.bMaster = MT_TRUE;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enI2sMode = MT_UNF_I2S_STD_MODE;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enMclkSel = MT_UNF_I2S_MCLK_256_FS;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enBclkSel = MT_UNF_I2S_BCLK_4_DIV;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enChannel = MT_UNF_I2S_CHNUM_2;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enBitDepth = MT_UNF_I2S_BIT_DEPTH_16;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.bPcmSampleRiseEdge = MT_TRUE;
    pstSndDefaultAttr->stAttr.stOutport[pstSndDefaultAttr->stAttr.u32PortNum].unAttr.stI2sAttr.stAttr.enPcmDelayCycle = MT_UNF_I2S_PCM_1_DELAY;
    pstSndDefaultAttr->stAttr.u32PortNum++;
#endif
#else

#endif
    return MT_SUCCESS;
}
#if 0  //unuse code
static MT_BOOL AOCheckOutPortIsValid(MT_UNF_SND_OUTPUTPORT_E enOutPort)
{

    ///TODO: cooper should fill
    if((MT_UNF_SND_OUTPUTPORT_DAC0 != enOutPort) && (MT_UNF_SND_OUTPUTPORT_EXT_DAC1 != enOutPort)
        && (MT_UNF_SND_OUTPUTPORT_EXT_DAC2 != enOutPort) && (MT_UNF_SND_OUTPUTPORT_EXT_DAC3 != enOutPort)
        && (MT_UNF_SND_OUTPUTPORT_SPDIF0 != enOutPort) && (MT_UNF_SND_OUTPUTPORT_I2S0 != enOutPort)
        && (MT_UNF_SND_OUTPUTPORT_HDMI0 != enOutPort))
    {
        MT_ERR_AI("output port is 0x%x not support!\n", enOutPort);
        return MT_FALSE;
    }

    return MT_TRUE;
}
#endif
mt_s32 audio_hdmi_notify(u32 event, u32 param, ulong context)
{
    //disp_priv_t *p_dp = (disp_priv_t *)context;
	  SND_CARD_STATE_S *pCard =  NULL;
    
	  if((void *)context == NULL)
		  return MT_FAILURE;
	
    pCard = (SND_CARD_STATE_S *)context;  //SND_CARD_GetCard(context);
    
    //disp_priv_t *p_dp = &s_stDisplayPriv;
    //HDMI_EXPORT_FUNC_S *pstHDMIFunc = MT_NULL;
    //HDMI_AVMUTE_S avmute_t;
    //disp_priv_t *p_dp = &s_stDisplayPriv;
  	//mt_u32 hdmi_hdcp_support;
  	//mt_u32 encrypt_enable;
  	//HDMI_HDCP_CAPACITY_S hdcpcapacity_t;
  	//HDMI_FORCE_DISPLAY_ENCRYPTION_S forceDisEncryption_t;

  	//printk("----------------\n");

    //printk("%s, %d, [0x%x],[0x%x],[0x%x],[0x%x]\n", __FUNCTION__, __LINE__, event, param, pCard->hdmi_acfg_flag, context);
    
#if 0
    if ((HDMI_EVENT_INNER_CONNECTION_STATUS == (event & HDMI_EVENT_INNER_CONNECTION_STATUS)))
    {
        if (param)
        {
  			//printk(" disp_hdmi_notify line %d\n", __LINE__);
            pCard->hdmi_acfg_flag = 1;
            MT_INFO_AO("HDMI audio connected\n");
        }
        else
        {
            //pCard->hdmi_vcfg_flag = MT_TRUE;
            MT_INFO_AO("HDMI audio disconnected\n");
        }
    }
    
    if ((HDMI_EVENT_HDCP_AUTH_STATUS == (event & HDMI_EVENT_HDCP_AUTH_STATUS)))
    {
        if (param){//HDCP authentication fail
            pCard->hdmi_acfg_flag = HDMI_AUDIO_CONFIG_TIMES;
            MT_INFO_AO("HDMI HDCP authentication fail\n");
        }else{//HDCP authentication success
            MT_INFO_AO("HDMI HDCP authentication success\n");
        }
    }
#endif

    if ((HMDI_AUDIO_CONFIG_EVENT == (event & HMDI_AUDIO_CONFIG_EVENT)))
    {
        pCard->hdmi_acfg_flag = 1;
        MT_INFO_AO("AUD HMDI_AUDIO_CONFIG_EVENT: \n");
    }
        
#if 0
    if(HDMI_EVENT_HDCP_CFG_CHG == (event & HDMI_EVENT_HDCP_CFG_CHG))
    {
        if(param)
        {
            p_dp->hdmi_vcfg.hdcp_on_off = 1;
        }
        else
        {
            p_dp->hdmi_vcfg.hdcp_on_off = 0;
        }
        printk("disp drv HDMI hdcp onoff %d\n", p_dp->hdmi_vcfg.hdcp_on_off);
    }

    if (HDMI_EVENT_VIDEO_FMT_CHG == (event & HDMI_EVENT_VIDEO_FMT_CHG))
    {
        printk("HDMI input video format change\n");
    }

    if (HDMI_EVENT_VIDEO_CLK_CHG == (event & HDMI_EVENT_VIDEO_CLK_CHG))
    {
        printk("HDMI input video clock change\n");
    }

    if (HDMI_EVENT_HDCP_AUTH_STATUS == (event & HDMI_EVENT_HDCP_AUTH_STATUS))
    {
        if (param)
        {
            printk("HDCP authentication fail\n");
        }
        else
        {
            printk("HDCP authentication pass\n");
        }
    }
#endif

    return MT_SUCCESS;
}

const int srcflt_coef[256] = {
//table_2A
0xffffff0a, 0xfffff548, 0xffffffb4, 0x0000029e, 0xfffffade, 0x000008ba,
0xfffff26a, 0x000013e7, 0xffffe43c, 0x00002566, 0xffffcf0e,
0x00003ea9, 0xffffb12d, 0x000061df, 0xffff878b, 0x000093ae, 0xffff4a8a,
0x0000e17e, 0xfffee073, 0x00018315, 0xfffdb39f, 0x000513f2,
0x000e18a1, 0xfffd74f2, 0x00013cf0, 0xffff4647, 0x0000731b, 0xffffb8cc,
0x000029d2, 0xffffea92, 0x00000726, 0x000002bd, 0xfffff6ab,
0x00000d6a, 0xfffff071, 0x00001040, 0xfffff02b, 0x00000eae, 0xfffff2fb,
0x00000b30, 0xfffff67f, 0x000008aa, 0xfffff588, 0xfffffafa,
//table_2B
0x00000562, 0x0000550b, 0xfffe58eb, 0x00058622, 0x000c9794, 0xffff5694,
0xffffb935, 0x00002af1,
//table_3A
0xffffff78, 0xfffff6cc, 0xfffffede, 0x000005fb, 0xfffff9d5, 0x00000c53,
0xfffff0cf, 0x0000179c, 0xffffe2a8, 0x0000285d, 0xffffcec4,
0x00003f42, 0xffffb44f, 0x00005d63, 0xffff91a8, 0x00008567, 0xffff6230,
0x0000bfc8, 0xffff1548, 0x000130c3, 0xfffe4b1e, 0x0003680a,
0x000ee8da, 0xfffe3617, 0x0000c598, 0xffff9d44, 0x0000317f, 0xffffee1c,
0x00000025, 0x00000da8, 0xffffec21, 0x000019db, 0xffffe5bf,
0x00001c27, 0xffffe6ad, 0x00001924, 0xffffeb60, 0x000013b2, 0xfffff171,
0x00000de8, 0xfffff6ec, 0x00000a2a, 0xfffff7c9, 0xfffffa7b,
0xfffffdb7, 0xfffff51a, 0x000006cb, 0xfffffd10, 0x00000489, 0xffffff32,
0x0000007b, 0x0000055b, 0xfffff727, 0x00001219, 0xffffe5e7,
0x0000286a, 0xffffc924, 0x00004cd2, 0xffff9ac6, 0x00008824, 0xffff4d00,
0x0000f1f6, 0xfffeb2dd, 0x0001ee02, 0xfffcac9b, 0x000a2a83,
0x000a2a83, 0xfffcac9b, 0x0001ee02, 0xfffeb2dd, 0x0000f1f6, 0xffff4d00,
0x00008824, 0xffff9ac6, 0x00004cd2, 0xffffc924, 0x0000286a,
0xffffe5e7, 0x00001219, 0xfffff727, 0x0000055b, 0x0000007b, 0xffffff32,
0x00000489, 0xfffffd10, 0x000006cb, 0xfffff51a, 0xfffffdb7,
//table_3B
0xffffff12, 0xffffec99, 0x00002a60, 0xffffcbb0, 0x000019a0, 0x00006ba9,
0xfffe07d3, 0x000c3e47, 0x000721e5, 0xfffd5c3f,
0x00013893, 0xffff8531, 0x00001fc2, 0x000006a1, 0xfffff3c7, 0xfffffb4a,
0xffffefe7, 0x00003e56, 0xffff7a0b, 0x0000e902,
0xfffeb6ec, 0x00019578, 0x000e5137, 0x00019578, 0xfffeb6ec, 0x0000e902,
0xffff7a0b, 0x00003e56, 0xffffefe7, 0xfffffb4a,
0x0000017e, 0x000050ce, 0xfffeae4e, 0x00037fef, 0x000d900b, 0x000052b8,
0xffff6fce, 0x000035d6,//table_4A
0x00000855, 0x00004d4c, 0xfffe3116, 0x000785da, 0x000b48b4, 0xfffe92aa,
0x00000759, 0x00001994,
0xfffffb77, 0xffffb68c, 0x000a3582, 0x00069ff0, 0xffff8a6e, //table_4B
0xffffe685, 0x0000c89d, 0x000c82d3, 0x0003277b, 0xffffb86b,
0x00000310, 0x00036ad3, 0x000abbe4, 0x0001d7a4, //table_5A
0x00001473, 0x0005792d, 0x00099e4f, 0x0000d590,
0x00004d6d, 0x0007b34d, 0x0007b34d, 0x00004d6d,
0xfffff68f, 0x00000000, 0x0000071b, 0x00000000, 0xfffff64a, 0x00000000,
//table_2D
0x00000cea, 0x00000000, 0xffffef35, 0x00000000, 0x0000157a, 0x00000000,
0xffffe4e5, 0x00000000, 0x000021e2, 0x00000000, 0xffffd5ef, 0x00000000,
0x00003413, 0x00000000, 0xffffbf83, 0x00000000, 0x0000504d, 0x00000000,
0xffff9ace, 0x00000000, 0x00008267, 0x00000000, 0xffff512e, 0x00000000,
0x0000fc7e, 0x00000000, 0xfffe5267, 0x00000000, 0x00051620, 0x00080000,
0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000,
};



// malloc out buffer, init the value of pcard
mt_s32 AO_SND_Open( MT_UNF_SND_E enSound, MT_UNF_SND_ATTR_S *pstAttr,
                            AO_ALSA_I2S_Param_S *pstAoI2sParam, MT_BOOL bResume)//pstAoI2sParam is i2s only param
{
    mt_s32 Ret = MT_SUCCESS;
    mt_u32 i,data;
    //HDMI_AUDIO_ATTR_S stHDMIAttr;
    hdmi_notify_info_t notify_info = { 0 };
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    // check attr
    CHECK_AO_SNDCARD( enSound );
    CHECK_AO_PORTNUM( pstAttr->u32PortNum );
    CHECK_AO_SAMPLERATE( pstAttr->enSampleRate );
    memset(pCard, 0, sizeof(SND_CARD_STATE_S));
    pCard->pstHdmiFunc = MT_NULL;
#ifdef MT_SND_MUTECTL_SUPPORT
    pCard->pstGpioFunc = MT_NULL;
#endif
    pCard->enPcmOutput  = SND_PCM_OUTPUT_VIR_SPDIFORHDMI;
    pCard->enHdmiPassthrough = SND_HDMI_MODE_NONE;
    pCard->enSpdifPassthrough = SND_SPDIF_MODE_NONE;
    pCard->bHdmiDebug = MT_FALSE;

    pCard->enSaveState = SND_DEBUG_CMD_CTRL_STOP;
    pCard->u32SaveCnt = 0;
	pCard->fileHandle = MT_NULL;
	pCard->bAllTrackMute = MT_FALSE;

    memcpy(&pCard->stUserOpenParam , pstAttr , sizeof(MT_UNF_SND_ATTR_S));      //record openparam!
    MT_INFO_AO("\n\n\n AO_SND_Open \n\n\n");


  //init hdmi
  {
        /* Get hdmi functions */
        Ret = mt_drv_module_getfunction(MT_ID_HDMI, (mt_void**)&pCard->pstHdmiFunc);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("Get hdmi function err:%#x!\n", Ret);
            ///TODO: cooper should fill
            //return Ret;
        }

		    if (pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiNotifyRegister)
		    {
		        notify_info.events = HDMI_EVENT_INNER_CONNECTION_STATUS | HDMI_EVENT_HDCP_AUTH_STATUS;

            notify_info.events = HMDI_AUDIO_CONFIG_EVENT;   // use the new event form 2021-5-19
            
		        notify_info.notify = audio_hdmi_notify;
		        notify_info.context = (ulong)pCard;
            notify_info.id_type = HDMI_NOTIFY_ID_TYPE_AUDIO; // add the notify id type for indicate the module identity
		        Ret = pCard->pstHdmiFunc->pfnHdmiNotifyRegister(0, (mt_u32*)&notify_info);
		        //printk("audio notify func %p.\n", notify_info.notify);
		        //printk("\n\n\n@@@@@@@@@@@@ hdmi notify@@@@@@@@@@@@@@ ret 0x%x \n\n\n", Ret);
		    }

#if 0
        if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiGetAoAttr)
        {
            (pCard->pstHdmiFunc->pfnHdmiGetAoAttr)(MT_UNF_HDMI_ID_0, &stHDMIAttr);
        }

        stHDMIAttr.enSoundIntf  = HDMI_AUDIO_INTERFACE_I2S;
        stHDMIAttr.enSampleRate = MT_UNF_SAMPLE_RATE_48K;
        stHDMIAttr.u32Channels  = AO_TRACK_NORMAL_CHANNELNUM;
        /*get the capability of the max pcm channels of the output device*/
        if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiAudioChange)
        {
            (pCard->pstHdmiFunc->pfnHdmiAudioChange)(MT_UNF_HDMI_ID_0, &stHDMIAttr);
            MT_INFO_AO("pfnHdmiAudioChange be called\n");

        }
#endif

        pCard->enHdmiPassthrough = SND_HDMI_MODE_PCM;


        MT_INFO_AO("\n\n\n audio hdmi setting  PCM MODE \n\n\n");
   }


#ifndef MT_SND_AOE_HW_SUPPORT

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)

    //run here
    Ret = mt_drv_mmz_alloc("AO_MAipPcm", MMZ_ZONE_AV, AO_TRACK_PCM_BUFSIZE_BYTE_DEF, AIAO_BUFFER_ADDR_ALIGN,
                                 &pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);

    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("MMZ_AllocAndMap failed\n");
        goto CREATE_OP_ERR_EXIT;
    }
    Ret = mt_drv_mmz_alloc("AO_MAipSPDIF", MMZ_ZONE_AV, AO_TRACK_PCM_BUFSIZE_BYTE_DEF, AIAO_BUFFER_ADDR_ALIGN,
                                 &pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW]);
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)

    //run here
    Ret = mt_drv_mmz_alloc("AO_MAipPcm", MMZ_ZONE_PCM, AO_TRACK_PCM_BUFSIZE_BYTE_DEF, AIAO_BUFFER_ADDR_ALIGN,
                                 &pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);

    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("MMZ_AllocAndMap failed\n");
        goto CREATE_OP_ERR_EXIT;
    }
    Ret = mt_drv_mmz_alloc("AO_MAipSPDIF", MMZ_ZONE_PCM, AO_TRACK_PCM_BUFSIZE_BYTE_DEF, AIAO_BUFFER_ADDR_ALIGN,
                                 &pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW]);
							 
    Ret = mt_drv_mmz_alloc("AO_MAipMix", MMZ_ZONE_PCM, AO_TRACK_PCM_BUFSIZE_BYTE_DEF, AIAO_BUFFER_ADDR_ALIGN,
                                 &pCard->stTrackRbfMmz[SND_ENGINE_TYPE_MIXBUF_DD]);
#else
    Ret = mt_drv_mmz_alloc("AO_MAipPcm", MMZ_OTHERS, AO_TRACK_PCM_BUFSIZE_BYTE_MAX, AIAO_BUFFER_ADDR_ALIGN,
                                 &pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);
#endif
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("MMZ_AllocAndMap failed\n");
        goto CREATE_OP_ERR_EXIT;
    }

#ifdef CONFIG_MT_CHIP_SYMPHONY4
    Ret = mt_drv_mmz_map(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);
#else
    Ret = mt_drv_mmz_map_cache(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);
#endif    
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("mt_drv_mmz_map failed\n");
        goto ALLOC_PCM_ERR_EXIT;
    }

#ifdef CONFIG_MT_CHIP_SYMPHONY4
    Ret = mt_drv_mmz_map(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW]);
#else
    Ret = mt_drv_mmz_map_cache(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW]);
#endif
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("mt_drv_mmz_map failed\n");
        goto ALLOC_PCM_ERR_EXIT;
    }

#ifdef CONFIG_MT_CHIP_SYMPHONY4
    Ret = mt_drv_mmz_map(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_MIXBUF_DD]);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("mt_drv_mmz_map failed\n");
        goto ALLOC_PCM_ERR_EXIT;
    }
#endif
#else

    Ret = mt_drv_mmz_alloc_and_map("AO_MAipPcm", MMZ_OTHERS, AO_TRACK_PCM_BUFSIZE_BYTE_MAX, AIAO_BUFFER_ADDR_ALIGN,
                                 &pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("MMZ_AllocAndMap failed\n");
        goto ALLOC_PCM_ERR_EXIT;
    }
#endif

	AO_RegProc((mt_u32)enSound);

    pCard->stUserOpenParam.u32MasterOutputBufSize=pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM].size;
    pCard->stUserOpenParam.u32SlaveOutputBufSize=0;


    // cooper   reg init
    //AoeIOAddressMap();
    if(!g_audio_reg_base)
    {
       //g_audio_reg_base = ioremap(0xffd90000, 0x300);
        g_audio_reg_base = mt_get_audio_base();


        g_audio_volume = AUDIO_VOLUME_MAX_SAFE;
        g_audio_mute = MT_FALSE;

        //audio  reg init
        {
          reg_sym_linux_00_reg_32_bit(1 ); //p_regg->bitc.pcm_32b_flag= 1;
          reg_sym_linux_04_reg_justified_bit( 2); // p_reg->bitc.justified_mode = 2;

#ifndef CONFIG_MT_CHIP_SYMPHONY4
          //ADAC relate
          /*
          if(0)
          {
            ulong sys_reg_base = (ulong)mt_get_sys_ctrl_base();

            reg_sys_ctrl_reg_128_t *p_reg_128 = (reg_sys_ctrl_reg_128_t *)(sys_reg_base + 0x128);
            reg_sys_ctrl_reg_144_t *p_reg_144 = (reg_sys_ctrl_reg_144_t *)(sys_reg_base + 0x144);
            reg_sys_ctrl_reg_2c_t *p_reg_2c = (reg_sys_ctrl_reg_2c_t *)(sys_reg_base + 0x2c);

            MT_INFO_AO("\n\n sound init sys_reg_base remap 0x%x    reg128 0x%x  reg 144 0x%x \n\n", sys_reg_base, *p_reg_128, *p_reg_144);

            p_reg_128->bitc.mute_mux = 1;  //enable mute function
            p_reg_144->bitc.mute = 1; //unmute
            p_reg_2c->bitc.spdif_mux = 1; //enable spdif
          }*/
#endif

          //set src
          for(i = 0;i < 256;i++)
          {
            data = srcflt_coef[i] & 0x001fffff; //low 21bits
            data |= 0x80000000;  //wr_coef_en
            data |= i << 21;       //wr_coef_addr
            *((volatile u32 *)(g_audio_reg_base + REG_AUD_SNT_SRC)) = data;
          }
        }
        MT_INFO_AO("\n\n sound init g_audio_reg_base remap 0x%x \n\n", g_audio_reg_base);

    }
    pCard->ao_record.volume= g_audio_volume;
    pCard->ao_record.is_mute = g_audio_mute;
    pCard->ao_record.trackmode = MT_UNF_TRACK_MODE_STEREO;

    pCard->enUserHdmiMode = MT_UNF_SND_HDMI_MODE_LPCM;
    return MT_SUCCESS;

ALLOC_PCM_ERR_EXIT:
    mt_drv_mmz_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);
    mt_drv_mmz_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW]);
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    mt_drv_mmz_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_MIXBUF_DD]);
#endif	
CREATE_OP_ERR_EXIT:
    return MT_FAILURE;
}


mt_s32 AO_SND_Close(MT_UNF_SND_E enSound, MT_BOOL bSuspend)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    AO_UnRegProc((mt_u32)enSound);
#ifndef MT_SND_AOE_HW_SUPPORT
    mt_drv_mmz_unmap(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);
    mt_drv_mmz_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);

    mt_drv_mmz_unmap(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW]);
    mt_drv_mmz_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW]);

#ifdef CONFIG_MT_CHIP_SYMPHONY4
    mt_drv_mmz_unmap(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_MIXBUF_DD]);
    mt_drv_mmz_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_MIXBUF_DD]);
#endif	
#else
    mt_drv_mmz_unmap_and_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);
#endif

    // delete codes below ,as  once mapped,use it forever
    //IOaddressUnmap();
    //if(g_audio_reg_base)
      //iounmap(g_audio_reg_base);
    return MT_SUCCESS;
}

#if 0
mt_s32 AO_SND_Close2(MT_UNF_SND_E enSound, MT_BOOL bSuspend)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    AO_UnRegProc((mt_u32)enSound);
#ifndef MT_SND_AOE_HW_SUPPORT
    mt_drv_mmz_unmap(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);
    mt_drv_mmz_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);
    if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
    {
        mt_drv_mmz_unmap(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW]);
        mt_drv_mmz_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW]);
    }

    if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
    {
        mt_drv_mmz_unmap(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_HDMI_RAW]);
        mt_drv_mmz_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_HDMI_RAW]);
    }
#else
    mt_drv_mmz_unmap_and_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM]);
    if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
    {
        mt_drv_mmz_unmap_and_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW]);
    }

    if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
    {
        mt_drv_mmz_unmap_and_release(&pCard->stTrackRbfMmz[SND_ENGINE_TYPE_HDMI_RAW]);
    }
#endif
    SND_DestroyOp(pCard, bSuspend);
    TRACK_DestroyEngine(pCard);

    //IOaddressUnmap();
    if(g_audio_reg_base)
      iounmap(g_audio_reg_base);
    return MT_SUCCESS;
}
#endif

// FadeIn, FadeOut, Smooth Mute //
#if 0
//fade out for mute/stop
static mt_s32 AO_SND_FadeOutEnable(mt_u32 wait_time)
{
#define MREAD(A)      (*(volatile unsigned int *)(A))
//#define MWRITE(A, V)   (*(volatile unsigned int *)(A)) = ((unsigned int)V)

	mt_u32 volume;
	//mt_u32 value;
	mt_u32 target_gain;
	mt_u32 fade_timestep;

	if (g_audio_reg_base == 0)
		return MT_FAILURE;

	volume = MREAD(g_audio_reg_base + 0x08); //current volume

	volume &= 0xFFFF;

	target_gain = volume >> 4;
	printk("%s: Enter, target_gain=0x%x\n",__FUNCTION__,target_gain);

	//Bug: current volume=0, call AO_SND_FadeDisable has pupu issue!
	if (target_gain == 0)
		return MT_FAILURE;

	fade_timestep = 0x40;			//default: 0x100
									//0x40 is faster than 0x100

#if 0
	value = MREAD(g_audio_reg_base + 0x5c); //audio fading control register
	//value &= 0xffff000d;			//bit 1: 0 - fade out
	value &= 0x000d;				//bit 1: 0 - fade out
	value |= (fade_timestep << 16);	//bit 16~31: fade timestep
	value |= (target_gain << 4);	//bit 4~15: target_gain
	value |= 0x01;					//bit 0: 1 - enable fading

	MWRITE((g_audio_reg_base + 0x5c), value);
#else
	reg_sym_linux_fade_reg(1, 0, target_gain, fade_timestep);
#endif

	//wait fading complete
	if (wait_time > 0)
		msleep(wait_time);

	//disable fading
	//value &= 0xfffffffe;
	//MWRITE((g_audio_reg_base + 0x5c), value);

	//printk("\n\n  value 0x%x \n\n\n", value);

	return MT_SUCCESS;
}

//fade in for unmute/start
static mt_s32 AO_SND_FadeInEnable(mt_u8 volume, mt_u32 wait_time)
{
//#define MREAD(A)      (*(volatile unsigned int *)(A))
//#define MWRITE(A, V)   (*(volatile unsigned int *)(A)) = ((unsigned int)V)

	mt_u8 gainq;
	mt_u16 volume_scale;

	//mt_u32 value;
	mt_u32 target_gain;
	mt_u32 fade_timestep;

	if (g_audio_reg_base == 0)
		return MT_FAILURE;

	audio_get_volume_gainq(&gainq, &volume_scale, volume);
	target_gain = volume_scale >> 4;
	//target_gain = 0x800;
	printk("%s: Enter, target_gain=0x%x\n",__FUNCTION__,target_gain);

	if (target_gain == 0)
		return MT_FAILURE;

	fade_timestep = 0x40;			//default: 0x100
									//0x40 is faster than 0x100

#if 0
	value = MREAD(g_audio_reg_base + 0x5c); //audio fading control register
	//value &= 0xffff000f;
	value &= 0x000f;
	value |= (fade_timestep << 16);	//bit 16~31: fade timestep
	value |= (target_gain << 4);	//bit 4~15: target_gain
	value |= (0x01 << 1);			//bit 1: 1 - fade in
	value |= 0x01;					//bit 0: 1 - enable fading

	MWRITE((g_audio_reg_base + 0x5c), value);
#else
	reg_sym_linux_fade_reg(1, 1, target_gain, fade_timestep);
#endif

	//wait fading complete
	if (wait_time > 0)
		msleep(wait_time);

	//disable fading
	//value &= 0xfffffffe;
	//MWRITE((g_audio_reg_base + 0x5c), value);

	//printk("\n\n  value 0x%x \n\n\n", value);

	return MT_SUCCESS;
}

//disable fade in/fade out
static mt_s32 AO_SND_FadeDisable(void)
{
//#define MREAD(A)      (*(volatile unsigned int *)(A))
//#define MWRITE(A, V)   (*(volatile unsigned int *)(A)) = ((unsigned int)V)

	//mt_u32 value;

	if (g_audio_reg_base == 0)
		return MT_FAILURE;

#if 0
	value = MREAD(g_audio_reg_base + 0x5c);	//audio fading control register

	value &= 0xfffffffe;
	MWRITE((g_audio_reg_base + 0x5c), value);
#else
	reg_sym_linux_fade_reg(0, 0, 0, 0);
#endif

	return MT_SUCCESS;
}

static mt_s32 AO_SND_SetMute_work(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bMute)
{
    #if 1
    mt_s32 Ret;
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_SNDCARD(enSound);

    MT_INFO_AO("AO_SND_SetMute old mute %d new mute %d \n", g_audio_mute,bMute);
    if(bMute != g_audio_mute)
    {
       pCard->ao_record.is_mute = bMute;
       g_audio_mute = bMute;
     if(bMute)
      {
        //must wait enough time till fade out complete, or has pupu issue!
		//AO_SND_FadeOutEnable(60);			//-NG
		Ret = AO_SND_FadeOutEnable(80);		//-OK

        //reg_sym_linux_volume_reg_spdif_mute_hdmi_bit(1); // mute spdif hdmi
        //reg_sym_linux_volume_reg_spdif_mute_coax_bit(1); // mute spdif coax

        audio_set_volume(0);

		//Bug: current volume=0, call AO_SND_FadeDisable has pupu issue!
      	if (Ret == MT_SUCCESS)
      	{
			AO_SND_FadeDisable();
		}
      }
      else
      {
		//must wait enough time till fade in complete, or has pupu issue!
		//AO_SND_FadeInEnable(g_audio_volume, 60);		//-NG
		Ret = AO_SND_FadeInEnable(g_audio_volume, 80);	//-OK

        //reg_sym_linux_volume_reg_spdif_mute_hdmi_bit(0); // umute spdif hdmi
        //reg_sym_linux_volume_reg_spdif_mute_coax_bit(0); // umute spdif coax

        audio_set_volume(g_audio_volume);

      	if (Ret == MT_SUCCESS)
      	{
			AO_SND_FadeDisable();
		}
      }
    }
    return MT_SUCCESS;

    #else
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    return SND_SetOpMute(pCard, enOutPort, bMute);
    #endif
}

struct ao_work_struct {
	struct work_struct ao_work;

	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
	unsigned int param4;
};

static struct ao_work_struct ao_mute_work;

static void ao_mute_work_function(struct work_struct *work_arg)
{
	struct ao_work_struct *a_ptr = container_of(work_arg, struct ao_work_struct, ao_work);

	MT_UNF_SND_E enSound = a_ptr->param1;
	MT_UNF_SND_OUTPUTPORT_E enOutPort = a_ptr->param2;
	MT_BOOL bMute = a_ptr->param3;

	AO_SND_SetMute_work(enSound, enOutPort, bMute);
}
#endif

#define MREAD(A) (*((volatile unsigned int *)(A)))
#define MWRITE(A, V) *((volatile unsigned int *)(A)) = (V)
extern int sys_otp_read(u32 bit_addr, u8 len, u32 *p_result);
static mt_s32 AO_SND_SetAdacOnOff(MT_UNF_SND_E enSound, MT_BOOL bOnOff)
{
    int ret = MT_FAILURE;
    mt_u32 val = 0;
    
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    void __iomem * reg_AdacPower = ioremap(0xbf157000, 4);
    void __iomem * reg_bf5dH = ioremap(0xbf5d0000, 4);

    ret = sys_otp_read(0x3FD4*8, 32, &val);
    if(MT_SUCCESS == ret){
        if(bOnOff){//power on adac
            if((val >> 7) & 0x1){//ADAC type1: 0dbu
                MWRITE(reg_AdacPower, (0x66<<12) | ((~(0xff<<12)) & MREAD(reg_AdacPower)));
            }else{//ADAC type2: 2Vrms
                MWRITE(reg_AdacPower, (0x55<<12) | ((~(0xff<<12)) & MREAD(reg_AdacPower)));
            }
            MWRITE(reg_bf5dH, (~(0x1<<9)) & MREAD(reg_bf5dH));
        }else{//power down adac
            MWRITE(reg_AdacPower, (0xff<<12) | ((~(0xff<<12)) & MREAD(reg_AdacPower)));
        }
    }
#endif    

    return ret;
}

//zgjiere; MT_UNF_SND_OUTPUTPORT_ALL
//for ao snd mute/unmute use too much time(>80ms), so change to call work queue
static mt_s32 AO_SND_SetMute(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bMute)
{
    //revert schedule work for muting written by qinghua
    //do not use fader
        #if 0
	INIT_WORK(&ao_mute_work.ao_work, ao_mute_work_function);

	ao_mute_work.param1 = enSound;
	ao_mute_work.param2 = enOutPort;
	ao_mute_work.param3 = bMute;
	ao_mute_work.param4 = 0;

	schedule_work(&ao_mute_work.ao_work);
        #else
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_SNDCARD(enSound);

    MT_INFO_AO("AO_SND_SetMute old mute %d new mute %d \n", pCard->ao_record.is_mute,bMute);

    	if(MT_UNF_SND_OUTPUTPORT_ALL==enOutPort){
            if(bMute){
                audio_set_volume(0);
                reg_sym_linux_volume_reg_spdif_mute_hdmi_bit(1);
                reg_sym_linux_volume_reg_spdif_mute_coax_bit(1);
            }else{
                reg_sym_linux_volume_reg_spdif_mute_hdmi_bit(0);
                reg_sym_linux_volume_reg_spdif_mute_coax_bit(0);
                audio_set_volume(g_audio_volume);
            }
            g_audio_mute_hdmi = bMute;
            g_audio_mute_spdif = bMute;
            g_audio_mute = bMute;
            pCard->ao_record.is_mute = bMute;
    	}else if(MT_UNF_SND_OUTPUTPORT_HDMI0==enOutPort){
            if(bMute != g_audio_mute_hdmi){
                g_audio_mute_hdmi = bMute;
            }
            if(bMute){
                reg_sym_linux_volume_reg_spdif_mute_hdmi_bit(1);
            }else{
                reg_sym_linux_volume_reg_spdif_mute_hdmi_bit(0);
            }
        }else if(MT_UNF_SND_OUTPUTPORT_SPDIF0==enOutPort){
            if(bMute != g_audio_mute_spdif){
                g_audio_mute_spdif = bMute;
            }
            if(bMute){
                reg_sym_linux_volume_reg_spdif_mute_coax_bit(1);
            }else{
                reg_sym_linux_volume_reg_spdif_mute_coax_bit(0);
            }
        }else{
            if(bMute != g_audio_mute)
            {
                pCard->ao_record.is_mute = bMute;
                g_audio_mute = bMute;

                if(bMute){
                    printk("\n  mute \n");
#ifdef CONFIG_MT_CHIP_SYMPHONY4
                    drv_gpio_set_value(GPIO_7, GPIO_VALUE_LOW_LEVEL);
#endif
                    audio_set_volume(0);
                }else{
                    printk("\n  umute \n");
                    audio_set_volume(g_audio_volume);
#ifdef CONFIG_MT_CHIP_SYMPHONY4
                    drv_gpio_set_value(GPIO_7, GPIO_VALUE_HIGH_LEVEL);
#endif
                }
            }
        }
        #endif
	return MT_SUCCESS;
}

static mt_s32 AO_SND_GetMute(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbMute)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(pbMute);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);
#if 1
    if(MT_UNF_SND_OUTPUTPORT_HDMI0==enOutPort){
        *pbMute = g_audio_mute_hdmi;
    }else if(MT_UNF_SND_OUTPUTPORT_SPDIF0==enOutPort){
        *pbMute = g_audio_mute_spdif;
    }else{
        *pbMute = g_audio_mute;
    }
    return MT_SUCCESS;
#else
    return SND_GetOpMute(pCard, enOutPort, pbMute);
#endif
}

static mt_s32 AO_SND_SetHdmiMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_HDMI_MODE_E enMode)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_HDMIMODE(enMode);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    return SND_SetOpHdmiMode(pCard, enOutPort, enMode);
}

static mt_s32 AO_SND_GetHdmiMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_HDMI_MODE_E *penMode)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(penMode);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    return SND_GetOpHdmiMode(pCard, enOutPort, penMode);
}

static mt_s32 AO_SND_SetSpdifMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_MODE_E enMode)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_SPDIFMODE(enMode);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    return SND_SetOpSpdifMode(pCard, enOutPort, enMode);
}

static mt_s32 AO_SND_GetSpdifMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_MODE_E *penMode)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(penMode);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    return SND_GetOpSpdifMode(pCard, enOutPort, penMode);
}

mt_s32 AO_SND_SetVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_GAIN_ATTR_S stGain)
{
    #if 1
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_SNDCARD(enSound);

    MT_INFO_AO("AO_SND_SetVolume old volume %x,%d new %d  \n",enOutPort,g_audio_volume, stGain.s32Gain);
    #ifdef CONFIG_MT_AUDIO_AD
    if(MT_UNF_SND_OUTPUTPORT_AD==enOutPort){
        if(pCard->ao_record.volume_ad != stGain.s32Gain){
            pCard->ao_record.volume_ad=stGain.s32Gain;
            //printk("++set trackmod=%x\n",pCard->ao_record.volume_ad);
            (void)AO_SND_SetTrackMode(MT_UNF_SND_0,MT_UNF_SND_OUTPUTPORT_DAC0,pCard->ao_record.trackmode);
        }
        return MT_SUCCESS;
    }
    #endif
    //printk("++set.normal=%x\n",stGain.s32Gain);
       if(stGain.s32Gain != g_audio_volume)
       {
          pCard->ao_record.volume = (mt_u8)stGain.s32Gain;
          g_audio_volume = (mt_u8)stGain.s32Gain;
          if(!g_audio_mute)
          {
            return audio_set_volume(g_audio_volume);
          }
       }

      return MT_SUCCESS;
    #else
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    if (MT_TRUE == stGain.bLinearMode)
    {
        CHECK_AO_LINEARVOLUME(stGain.s32Gain);
    }
    else
    {
        CHECK_AO_ABSLUTEVOLUME(stGain.s32Gain);
    }
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    return SND_SetOpVolume(pCard, enOutPort, stGain);
    #endif
}

static mt_s32 AO_SND_GetVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_GAIN_ATTR_S *pstGain)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(pstGain);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    //TODO Check volumn Attr
#if 1
    pstGain->s32Gain = g_audio_volume;
    return MT_SUCCESS;
#else
    return SND_GetOpVolume(pCard, enOutPort, pstGain);
#endif
}

static mt_s32 AO_SND_SetSpdifCategoryCode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                      MT_UNF_SND_SPDIF_CATEGORYCODE_E enCategoryCode)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);
    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_CATEGORYCODE(enCategoryCode);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);
    return SND_SetOpSpdifCategoryCode(pCard, enOutPort, enCategoryCode);
}
static mt_s32 AO_SND_GetSpdifCategoryCode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                      MT_UNF_SND_SPDIF_CATEGORYCODE_E *penCategoryCode)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);
    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(penCategoryCode);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);
    return SND_GetOpSpdifCategoryCode(pCard, enOutPort, penCategoryCode);
}
static mt_s32 AO_SND_SetSpdifSCMSMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                      MT_UNF_SND_SPDIF_SCMSMODE_E enSCMSMode)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_SPDIFSCMSMODE(enSCMSMode);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    return SND_SetOpSpdifSCMSMode(pCard, enOutPort, enSCMSMode);
}

static mt_s32 AO_SND_GetSpdifSCMSMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                      MT_UNF_SND_SPDIF_SCMSMODE_E *penSCMSMode)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(penSCMSMode);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    return SND_GetOpSpdifSCMSMode(pCard, enOutPort, penSCMSMode);
}


//zgjiere; HAL_AIAO_P_SetSampleRate 不能动态修改，建议MT_UNF_SND_ATTR_S确定
static mt_s32 AO_SND_SetSampleRate(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                   MT_UNF_SAMPLE_RATE_E enSampleRate)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_SAMPLERATE(enSampleRate);
    CHECK_AO_NULL_PTR(pCard);

    return SND_SetOpSampleRate(pCard, enOutPort, enSampleRate);
}

static mt_s32 AO_SND_GetSampleRate(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                   MT_UNF_SAMPLE_RATE_E *penSampleRate)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(penSampleRate);
    CHECK_AO_NULL_PTR(pCard);

    return SND_GetOpSampleRate(pCard, enOutPort, penSampleRate);
}
#if 0  //unuse code
static u32 symphony_get_ad_channel_coef(mt_u32 ad_vol)
{
    if(90<=ad_vol){
        return 0x80008000;
    }else if(80<=ad_vol){
        return 0x70007000;
    }else if(60<=ad_vol){
        return 0x60006000;
    }else if(50<=ad_vol){   //default
        return 0x50005000;
    }else if(40<=ad_vol){
        return 0x40004000;
    }else if(30<=ad_vol){
        return 0x30003000;
    }else if(10<=ad_vol){
        return 0x20002000;
    }else{
        return 0x10001000;
    }

    /* ucos
    switch(ad_vol)
    {
        case AD_VOL_LEVEL_NEG_3:
            return 0x20002000;
        case AD_VOL_LEVEL_NEG_2:
            return 0x30003000;
        case AD_VOL_LEVEL_NEG_1:
            return 0x40004000;
        case AD_VOL_DEFAULT:
        case AD_VOL_LEVEL_0:
            return 0x50005000;
        case AD_VOL_LEVEL_1:
            return 0x60006000;
        case AD_VOL_LEVEL_2:
            return 0x70007000;
        case AD_VOL_LEVEL_3:
            return 0x80008000;

        default:
            return 0x80008000;
    }*/
}
#endif
static mt_s32 AO_SND_SetTrackMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E enMode)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    //printk("\n\n\n\n\n\n   1405AO_SND_SetTrackMode1111  mode %d    \n\n\n\n\n\n", enMode);
    pCard->ao_record.trackmode=enMode;

    return MT_SUCCESS;
}

static mt_s32 AO_SND_GetTrackMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E *penMode)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(penMode);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    *penMode=pCard->ao_record.trackmode;
    return MT_SUCCESS;//SND_GetOpTrackMode(pCard, enOutPort, penMode);
}


static mt_s32 AO_SND_SetAllTrackMute(MT_UNF_SND_E enSound, MT_BOOL bMute)
{
    mt_s32 s32Ret;
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);
    CHECK_AO_NULL_PTR(pCard);

    s32Ret = TRACK_SetAllMute(pCard, bMute);
    if(MT_SUCCESS != s32Ret)
    {
    	//TODO
    }

    pCard->bAllTrackMute = bMute;
	MT_INFO_AO("Track ALL get Mute Staues %d\n",pCard->bAllTrackMute);

    return s32Ret;
}

static mt_s32 AO_SND_GetAllTrackMute(MT_UNF_SND_E enSound, MT_BOOL *pbMute)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_NULL_PTR(pbMute);
    CHECK_AO_NULL_PTR(pCard);

    *pbMute = pCard->bAllTrackMute;
    return MT_SUCCESS;
}

static mt_s32  AO_SND_SetSmartVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bSmartVolume)
{
    //TO DO
    //verify
    return MT_SUCCESS;
}

static mt_s32 AO_SND_GetSmartVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbSmartVolume)
{
    //TO DO
    //verify
    return MT_SUCCESS;
}

static mt_s32 AO_SND_AttachAef(MT_UNF_SND_E enSound, mt_u32 u32AefId, mt_u32 *pu32AefProcAddr)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_NULL_PTR(pu32AefProcAddr);
    CHECK_AO_NULL_PTR(pCard);

    return AEF_AttachSnd(pCard, u32AefId, (ulong *)pu32AefProcAddr);
}

static mt_s32 AO_SND_DetachAef(MT_UNF_SND_E enSound, mt_u32 u32AefId)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_NULL_PTR(pCard);

    return AEF_DetachSnd(pCard, u32AefId);
}

static mt_s32 AO_SND_SetAefBypass(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bBypass)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    return SND_SetOpAefBypass(pCard, enOutPort, bBypass);
}

static mt_s32 AO_SND_GetAefBypass(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbBypass)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_SNDCARD(enSound);
    CHECK_AO_OUTPORT(enOutPort);
    CHECK_AO_NULL_PTR(pbBypass);
    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_PORTEXIST(pCard->stUserOpenParam.u32PortNum);

    return SND_GetOpAefBypass(pCard, enOutPort, pbBypass);
}


static mt_s32 AO_Snd_GetXRunCount(MT_UNF_SND_E enSound, mt_u32 *pu32Count)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_NULL_PTR(pCard);
    SND_GetXRunCount(pCard,pu32Count);

    return MT_SUCCESS;

}

/* snd open kernel intf */
mt_s32 AO_Snd_Kopen(AO_SND_Open_Param_S_PTR arg, struct file *file)
{
    mt_s32 s32Ret;
    MT_UNF_SND_E enSound = MT_UNF_SND_BUTT;
    AO_SND_Open_Param_S_PTR pstSndParam = ( AO_SND_Open_Param_S_PTR )arg;
    DRV_AO_STATE_S *pAOState = file->private_data;

    AO_ALSA_I2S_Param_S* pstAoI2sParam = (AO_ALSA_I2S_Param_S *)pstSndParam->pAlsaPara;//MT_ALSA_I2S_ONLY_SUPPORT
    enSound = pstSndParam->enSound;
    CHECK_AO_SNDCARD( enSound );

    s32Ret = down_interruptible(&g_AoMutex);
    if (MT_SUCCESS == AO_Snd_AllocHandle(enSound, file))
    {
        if (0 == atomic_read(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt))
        {
                s32Ret = AO_SND_Open( enSound,&pstSndParam->stAttr,pstAoI2sParam, MT_FALSE);
                if (MT_SUCCESS != s32Ret)
                {
                    AO_Snd_FreeHandle(enSound, file);
                    up(&g_AoMutex);
                    return MT_FAILURE;
                }
        }
    }

    atomic_inc(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt);
    atomic_inc(&pAOState->atmUserOpenCnt[enSound]);

    up(&g_AoMutex);

    return MT_SUCCESS;
}

/* snd close kernel intf */
mt_s32 AO_Snd_Kclose(MT_UNF_SND_E  arg, struct file *file)
{
    mt_s32 s32Ret;
    MT_UNF_SND_E enSound = MT_UNF_SND_BUTT;
    DRV_AO_STATE_S *pAOState = file->private_data;
    enSound = arg;
    CHECK_AO_SNDCARD_OPEN( enSound );

    s32Ret = down_interruptible(&g_AoMutex);
    if(atomic_dec_and_test(&pAOState->atmUserOpenCnt[enSound]))
    {
            if (atomic_dec_and_test(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt))
            {
                s32Ret = AO_SND_Close( enSound, MT_FALSE );
                if (MT_SUCCESS != s32Ret)
                {
                    atomic_inc(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt);
                    up(&g_AoMutex);
                    return MT_FAILURE;
                }

                AO_Snd_FreeHandle(enSound, file);
            }
    }
    else
	{
        atomic_dec(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt);
    }

    up(&g_AoMutex);

    return MT_SUCCESS;
}

/******************************Snd Track FUNC*************************************/
#if 0 //unuse code
static SND_CARD_STATE_S * TRACK_CARD_GetCard2(mt_u32 Id)
{
    MT_UNF_SND_E sndx;
    SND_CARD_STATE_S *pCard = MT_NULL;

    if (Id >= AO_MAX_TOTAL_TRACK_NUM)
    {
        return MT_NULL;
    }

    for (sndx = MT_UNF_SND_0; sndx < MT_UNF_SND_BUTT; sndx++)
    {
        pCard = SND_CARD_GetCard(sndx);
        if(pCard)
        {
            if (pCard->uSndTrackInitFlag & (1L << Id))
            {
                return pCard;
            }
        }
    }

    return MT_NULL;
}
#endif

static SND_CARD_STATE_S * TRACK_CARD_GetCard(mt_u32 Id)
{
    return SND_CARD_GetCard(MT_UNF_SND_0);
}

//only output handle like 0x110000 or 0x110001
mt_s32 AO_Track_AllocHandle(mt_handle *phHandle, struct file *pstFile, AO_Track_Create_Param_S_PTR p_param)
{
    mt_u32 i;

    if (MT_NULL == phHandle)
    {
        MT_ERR_AO("Bad param!\n");
        return MT_FAILURE;
    }

    /* Check ready flag */
    if (s_stAoDrv.bReady != MT_TRUE)
    {
        MT_ERR_AO("Need open first!\n");
        return MT_FAILURE;
    }

   if(MT_UNF_SND_TRACK_TYPE_MASTER == p_param->stAttr.enTrackType)
   {
      i = 0;
      MT_INFO_AO("\n creat track   master !!\n");
   }
   else if(MT_UNF_SND_TRACK_TYPE_SLAVE == p_param->stAttr.enTrackType)
   {
      i = 1;
      MT_INFO_AO("\n creat track   slave !!\n");
   }
   else
   {
      MT_INFO_AO("\n creat track   error , only  master and slave can be created !\n");
      return MT_FAILURE;
   }

    if (atomic_read(&s_stAoDrv.astTrackEntity[i].atmUseCnt)) //track can not be re create
        {
        MT_ERR_AO("AO track can not be re-create!\n");
            return MT_FAILURE;
        }


    /* Allocate resource */
    s_stAoDrv.astTrackEntity[i].u32File = (long)pstFile;
    //s_stAoDrv.u32TrackNum++;
    atomic_inc(&s_stAoDrv.astTrackEntity[i].atmUseCnt);
    /*
      define of Track Handle :
      bit31                                                           bit0
        |<----   16bit --------->|<---   8bit    --->|<---  8bit   --->|
        |--------------------------------------------------------------|
        |      MT_MOD_ID_E            |  sub_mod defined  |     chnID       |
        |--------------------------------------------------------------|
      */
    *phHandle = (MT_ID_AO << 16) | (MT_ID_TRACK << 8) | i;


      MT_INFO_AO("\n\n\n\n    AO_Track_AllocHandle  0x%x    \n\n\n\n\n", *phHandle);


    return MT_SUCCESS;

//err0:
//    return MT_FAILURE;
}

mt_s32 AO_Track_AllocHandle_old(mt_handle *phHandle, struct file *pstFile)
{
    mt_u32 i;

    if (MT_NULL == phHandle)
    {
        MT_ERR_AO("Bad param!\n");
        return MT_FAILURE;
    }

    /* Check ready flag */
    if (s_stAoDrv.bReady != MT_TRUE)
    {
        MT_ERR_AO("Need open first!\n");
        return MT_FAILURE;
    }

    /* Check channel number */
    if (s_stAoDrv.u32TrackNum >= AO_MAX_TOTAL_TRACK_NUM)
    {
        MT_ERR_AO("Too many track:%d!\n", s_stAoDrv.u32TrackNum);
        goto err0;
    }

    /* Allocate new channel */
    for (i = 0; i < AO_MAX_TOTAL_TRACK_NUM; i++)
    {
        if (0 == atomic_read(&s_stAoDrv.astTrackEntity[i].atmUseCnt))
        {
            s_stAoDrv.astTrackEntity[i].u32File = (mt_u32)MT_NULL;
            break;
        }
    }

    if (i >= AO_MAX_TOTAL_TRACK_NUM)
    {
        MT_ERR_AO("Too many track!\n");
        goto err0;
    }

    /* Allocate resource */
    s_stAoDrv.astTrackEntity[i].u32File = (long)pstFile;
    s_stAoDrv.u32TrackNum++;
    atomic_inc(&s_stAoDrv.astTrackEntity[i].atmUseCnt);
    /*
      define of Track Handle :
      bit31                                                           bit0
        |<----   16bit --------->|<---   8bit    --->|<---  8bit   --->|
        |--------------------------------------------------------------|
        |      MT_MOD_ID_E            |  sub_mod defined  |     chnID       |
        |--------------------------------------------------------------|
      */
    *phHandle = (MT_ID_AO << 16) | (MT_ID_TRACK << 8) | i;
    return MT_SUCCESS;

err0:
    return MT_FAILURE;
}

mt_void AO_Track_FreeHandle(mt_handle hHandle)
{

    hHandle &= AO_TRACK_CHNID_MASK;

    if (hHandle >= (mt_u32)AO_MAX_TOTAL_TRACK_NUM)
    	return;

    if (0 == atomic_read(&s_stAoDrv.astTrackEntity[hHandle].atmUseCnt))
	return;
    s_stAoDrv.astTrackEntity[hHandle].u32File = (long)MT_NULL;
    //s_stAoDrv.u32TrackNum--;
    atomic_set(&s_stAoDrv.astTrackEntity[hHandle].atmUseCnt, 0);
}

static mt_void AO_TRACK_SaveSuspendAttr(mt_handle hHandle, AO_Track_Create_Param_S_PTR pstTrack)
{
    hHandle &= AO_TRACK_CHNID_MASK;
    s_stAoDrv.astTrackEntity[hHandle].stSuspendAttr.enSound = pstTrack->enSound;
    s_stAoDrv.astTrackEntity[hHandle].stSuspendAttr.bAlsaTrack = pstTrack->bAlsaTrack;
    memcpy(&s_stAoDrv.astTrackEntity[hHandle].stSuspendAttr.stBufAttr, &pstTrack->stBuf, sizeof(AO_BUF_ATTR_S));
}

mt_s32 AO_Track_GetDefAttr(MT_UNF_AUDIOTRACK_ATTR_S * pstDefAttr)
{
    return TRACK_GetDefAttr(pstDefAttr);
}

static mt_s32 AO_Track_GetAttr(mt_u32 u32TrackID, MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_GetAttr(pCard, u32TrackID, pstTrackAttr);
    }
    else
    {
        return MT_FAILURE;
    }
}

static mt_s32 AO_Track_SetAttr(mt_u32 u32TrackID, MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr)
{
    mt_s32 s32Ret;
    SND_CARD_STATE_S *pCard;

    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if(pCard)
    {

        s32Ret = TRACK_SetAttr(pCard, u32TrackID, pstTrackAttr);
        if (MT_SUCCESS != s32Ret)
        {
            MT_FATAL_AO("TRACK_SetAttr fail\n");
            return MT_FAILURE;
        }
        return s32Ret;
    }
    return MT_FAILURE;
}

mt_s32 AO_Track_Create(MT_UNF_SND_E enSound, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr,
                       MT_BOOL bAlsaTrack, AO_BUF_ATTR_S *pstBuf, mt_handle hTrack)
{
    mt_s32 s32Ret;
    SND_CARD_STATE_S *pCard;
    hTrack &= AO_TRACK_CHNID_MASK;
    pCard = SND_CARD_GetCard(enSound);

    if(pCard)
    {
        s32Ret = TRACK_CreateNew(pCard, pstAttr, bAlsaTrack, pstBuf, hTrack);

        return s32Ret;
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

mt_s32 AO_Track_Destory(mt_u32 u32TrackID)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_Destroy(pCard, u32TrackID);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

mt_s32 AO_Track_Start(mt_u32 u32TrackID)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_Start(pCard, u32TrackID);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

mt_s32 AO_Track_Stop(mt_u32 u32TrackID)
{
	mt_s32 Ret = MT_SUCCESS;
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        //return TRACK_Stop(pCard, u32TrackID);
        Ret = TRACK_Stop(pCard, u32TrackID);

        //FIX Bug: 噗噗杂音, but still has tiny pupu noise

        //do not reset while track is not closed ,otherwise hardware would be wrong
		//iHAL_AOE_AIP_reset_aout_buffer();

		return Ret;
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

mt_s32 AO_Track_Pause(mt_u32 u32TrackID)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_Pause(pCard,u32TrackID);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

mt_s32 AO_Track_Flush(mt_u32 u32TrackID)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_Flush(pCard, u32TrackID);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

mt_s32 AO_Track_OpenCheck(mt_u32 u32TrackID)
{
    mt_s32 ret = MT_SUCCESS;
    CHECK_AO_TRACK_OPEN(u32TrackID);
    return ret;
}
mt_s32 AO_Track_SendData(mt_u32 u32TrackID, MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_SendData(pCard, u32TrackID, pstAOFrame);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}
EXPORT_SYMBOL(AO_Track_SendData);
EXPORT_SYMBOL(AO_Track_OpenCheck);

static mt_s32 AO_Track_SetWeight(mt_u32 u32TrackID, MT_UNF_SND_GAIN_ATTR_S stTrackGain)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_SetWeight(pCard, u32TrackID, &stTrackGain);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Track_GetWeight(mt_u32 u32TrackID, MT_UNF_SND_GAIN_ATTR_S *pstTrackGain)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_GetWeight(pCard, u32TrackID, pstTrackGain);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Track_SetAbsGain(mt_u32 u32TrackID, MT_UNF_SND_ABSGAIN_ATTR_S stTrackAbsGain)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_SetAbsGain(pCard, u32TrackID, &stTrackAbsGain);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Track_GetAbsGain(mt_u32 u32TrackID, MT_UNF_SND_ABSGAIN_ATTR_S *pstTrackAbsGain)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_GetAbsGain(pCard, u32TrackID, pstTrackAbsGain);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Track_SetMute(mt_u32 u32TrackID, MT_BOOL bMute)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_SetMute(pCard, u32TrackID, bMute);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Track_GetMute(mt_u32 u32TrackID, MT_BOOL *pbMute)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_GetMute(pCard, u32TrackID, pbMute);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Track_SetChannelMode(mt_u32 u32TrackID, MT_UNF_TRACK_MODE_E enMode)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_SetChannelMode(pCard, u32TrackID, &enMode);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Track_GetChannelMode(mt_u32 u32TrackID, MT_UNF_TRACK_MODE_E *penMode)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_GetChannelMode(pCard, u32TrackID, penMode);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Track_SetSpeedAdjust(mt_u32 u32TrackID, AO_SND_SPEEDADJUST_TYPE_E enType, mt_s32 s32Speed)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_SetSpeedAdjust(pCard, u32TrackID, enType, s32Speed);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

mt_s32 AO_Track_GetDelayMs(mt_u32 u32TrackID, mt_u32 *pu32DelayMs)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_GetDelayMs(pCard, u32TrackID, pu32DelayMs);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Track_IsBufEmpty(mt_u32 u32TrackID, MT_BOOL *pbBufEmpty)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_IsBufEmpty(pCard, u32TrackID, pbBufEmpty);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Track_SetEosFlag(mt_u32 u32TrackID, MT_BOOL bEosFlag)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_SetEosFlag(pCard, u32TrackID, bEosFlag);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}


mt_s32 AO_Track_AttachAi(mt_u32 u32TrackID, mt_handle hAi)
{
#if defined (MT_AUDIO_AI_SUPPORT)
    mt_s32 Ret;
    SND_CARD_STATE_S *pCard;
    mt_handle   hTrack;
    hTrack = u32TrackID & AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(hTrack);

    if (pCard)
    {
        Ret = TRACK_SetPcmAttr(pCard, hTrack, hAi);
        if(MT_SUCCESS != Ret)
        {
            MT_ERR_AO("call TRACK_SetPcmAttr failed!\n");
            return Ret;
        }

        Ret = AI_SetAttachFlag(hAi, hTrack, MT_TRUE);
        if(MT_SUCCESS != Ret)
        {
            MT_ERR_AO("call AI_SetAttachFlag failed!\n");
            return Ret;
        }

        Ret = TRACK_AttachAi(pCard, hTrack, hAi);
        if(MT_SUCCESS != Ret)
        {
            MT_ERR_AO("call TRACK_AttachAi failed!\n");
            return Ret;
        }
        return Ret;
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }

#else
    return MT_FAILURE;
#endif
}

mt_s32 AO_Track_DetachAi(mt_u32 u32TrackID, mt_handle hAi)
{
#if defined (MT_AUDIO_AI_SUPPORT)
    mt_s32 Ret;
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        Ret = TRACK_DetachAi(pCard, u32TrackID);
        if(MT_SUCCESS != Ret)
        {
            MT_ERR_AO("call TRACK_DetachAi failed!\n");
            return Ret;
        }

        Ret = AI_SetAttachFlag(hAi, u32TrackID, MT_FALSE);
        if(MT_SUCCESS != Ret)
        {
            MT_ERR_AO("call AI_SetAttachFlag failed!\n");
            return Ret;
        }
        return Ret;
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }

#else
    return MT_FAILURE;
#endif
}

#if 0
static mt_s32 AO_Track_GetStatus(mt_u32 u32TrackID, mt_void *pstParam)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_GetStatus(pCard, u32TrackID, pstParam);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

#endif

static mt_s32 AO_Track_MasterSlaveExchange(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID)
{
    mt_u32 u32MTrackID;
    MT_UNF_AUDIOTRACK_ATTR_S *pstTrackAttr;
    MT_BOOL bAlsaTrack;
    AO_BUF_ATTR_S *pstBuf;
    MT_UNF_SND_E enSound;

    SND_TRACK_SETTINGS_S stTrackSettings;
    mt_s32 s32Ret;

    enSound = SND_CARD_GetSnd(pCard);
	if(AO_MAX_TOTAL_SND_NUM == enSound)
		return MT_FAILURE;

    u32MTrackID = TRACK_GetMasterId(pCard);
    if (AO_MAX_TOTAL_TRACK_NUM != u32MTrackID)  //judge if master track exist
    {
        //Master -> slave
        TRACK_GetSetting(pCard,u32MTrackID, &stTrackSettings); //save track setting
        //Master Track is NOT STOP,Not support Exchange
        if(SND_TRACK_STATUS_STOP != stTrackSettings.enCurnStatus)
        {
            MT_FATAL_AO("Exist Master Track(%d) is Not Stop!\n",u32MTrackID);
            return MT_FAILURE;
        }

	    /* Destory track */
        s32Ret = AO_Track_Destory(u32MTrackID);
        if (MT_SUCCESS != s32Ret)
        {
            MT_FATAL_AO("AO_Track_Destory fail\n");
            return MT_FAILURE;
        }

        pstTrackAttr = &stTrackSettings.stTrackAttr;
        //bAlsaTrack = stTrackSettings.bAlsaTrack;
        bAlsaTrack = MT_FALSE;        //ALSA TRACK NEVER EXCHANGE
        pstBuf = &stTrackSettings.stBufAttr;
        pstTrackAttr->enTrackType = MT_UNF_SND_TRACK_TYPE_SLAVE;
        /* Recreate slave track  */
        s32Ret = AO_Track_Create(enSound, pstTrackAttr, bAlsaTrack, pstBuf, u32MTrackID);
        if (MT_SUCCESS != s32Ret)
        {
            MT_FATAL_AO("AO_Track_Create fail\n");
            return MT_FAILURE;
        }
        TRACK_RestoreSetting(pCard, u32MTrackID, &stTrackSettings); //restore track setting
    }
    if(u32MTrackID == u32TrackID)	//if input track id is master ,just return here
    {
        return MT_SUCCESS;
    }
    else
    {

	    //slave -> Master
       TRACK_GetSetting(pCard,u32TrackID, &stTrackSettings);
         /* Destory track */
        s32Ret = AO_Track_Destory(u32TrackID);
        if (MT_SUCCESS != s32Ret)
        {
            MT_FATAL_AO("AO_Track_Destory fail\n");
            return MT_FAILURE;
        }
        pstTrackAttr = &stTrackSettings.stTrackAttr;
        //bAlsaTrack = stTrackSettings.bAlsaTrack;
        bAlsaTrack = MT_FALSE;        //ALSA TRACK NEVER EXCHANGE
        pstBuf = &stTrackSettings.stBufAttr;
        pstTrackAttr->enTrackType = MT_UNF_SND_TRACK_TYPE_MASTER;
        /* Recreate Master track  */
        s32Ret = AO_Track_Create(enSound, pstTrackAttr, bAlsaTrack, pstBuf, u32TrackID);
        if (MT_SUCCESS != s32Ret)
        {
            MT_FATAL_AO("AO_Track_Create fail\n");
            return MT_FAILURE;
        }
        TRACK_RestoreSetting(pCard, u32TrackID, &stTrackSettings);
        return MT_SUCCESS;
    }

}

mt_s32 AO_Track_PreCreate_old(MT_UNF_SND_E enSound, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr,
                       MT_BOOL bAlsaTrack, AO_BUF_ATTR_S *pstBuf, mt_handle hTrack)
{
    mt_u32 u32TrackID;
    mt_s32 s32Ret;

    SND_CARD_STATE_S *pCard;
    hTrack &= AO_TRACK_CHNID_MASK;
    pCard = SND_CARD_GetCard(enSound);

    if(pCard)
    {
		s32Ret = TRACK_CheckAttr(pstAttr);
		if(MT_SUCCESS != s32Ret)
		{
			return MT_FAILURE;
		}
        if(MT_UNF_SND_TRACK_TYPE_MASTER == pstAttr->enTrackType)
        {
            u32TrackID = TRACK_GetMasterId(pCard);
            if (AO_MAX_TOTAL_TRACK_NUM != u32TrackID)  //judge if master track exist
            {
                s32Ret = AO_Track_MasterSlaveExchange(pCard, u32TrackID);    //force master to slave
                if(MT_SUCCESS != s32Ret)
                {
                    MT_ERR_AO("Failed to Force Master track(%d) To Slave!\n", u32TrackID);
                    return MT_FAILURE;
                }
            }
        }
        return AO_Track_Create(enSound, pstAttr, bAlsaTrack, pstBuf, hTrack);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

mt_s32 AO_Track_PreCreate(MT_UNF_SND_E enSound, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr,
                       MT_BOOL bAlsaTrack, AO_BUF_ATTR_S *pstBuf, mt_handle hTrack)
{
    //mt_u32 u32TrackID;
    //mt_s32 s32Ret;

    SND_CARD_STATE_S *pCard;
    hTrack &= AO_TRACK_CHNID_MASK;
    pCard = SND_CARD_GetCard(enSound);


    if(pCard)
    {
        return AO_Track_Create(enSound, pstAttr, bAlsaTrack, pstBuf, hTrack);
    }
    else
    {
        MT_INFO_AO("\n\n\n  in AO_Track_PreCreate2  card is not opened!     \n\n");
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}


static mt_s32 AO_Track_PreSetAttr(mt_u32 u32TrackID, MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr)
{
    mt_s32 s32Ret;
    SND_CARD_STATE_S *pCard;
    SND_TRACK_ATTR_SETTING_E enAttrSetting;
    MT_UNF_AUDIOTRACK_ATTR_S stTrackTmpAttr;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if(pCard)
    {
        s32Ret = TRACK_DetectAttr(pCard, u32TrackID, pstTrackAttr, &enAttrSetting);
        if(MT_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        switch(enAttrSetting)
        {
            case SND_TRACK_ATTR_RETAIN:
                s32Ret = MT_SUCCESS;
                break;
            case SND_TRACK_ATTR_MODIFY:
                s32Ret = AO_Track_SetAttr(u32TrackID, pstTrackAttr);
                break;
            case SND_TRACK_ATTR_MASTER2SLAVE:
            case SND_TRACK_ATTR_SLAVE2MASTER:
                AO_Track_GetAttr(u32TrackID, &stTrackTmpAttr);          //save track attr
                s32Ret = AO_Track_SetAttr(u32TrackID, pstTrackAttr);
                s32Ret = AO_Track_MasterSlaveExchange(pCard, u32TrackID);
                if(MT_SUCCESS != s32Ret)
                 {
                    AO_Track_SetAttr(u32TrackID, &stTrackTmpAttr);      //exchange failed ,restore track attr
                 }
                break;

            default:
                s32Ret = MT_SUCCESS;        //TODO
                break;
        }

        return s32Ret;
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}
//zgjiere; alsaí33?????

/*
//zgjiere, ?ó?úê?·??ú×?±ê×?alsa, ?D??·t??3ìDò
 */

/*
//zgjiere, alsa ? start/stop/pause/flush
 */

/*
//zgjiere, alsa êy?Yè?o?D′è?? ???°AIPè?è??ù?YAIP?áD′??????DDêy?Y′|àí? ALSA±ê×??y?ˉDD?a?¨ê±?á￡?o???D′????
 */

mt_s32 AO_Track_UpdateBufWptr(mt_u32 u32TrackID, mt_u32 *pu32WritePos)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_UpdateWptrPos(pCard, u32TrackID, pu32WritePos);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

mt_s32 AO_Track_UpdateBufRptr(mt_u32 u32TrackID, mt_u32 *pu32ReadPos)
{
	SND_CARD_STATE_S *pCard;
	u32TrackID &= AO_TRACK_CHNID_MASK;
	pCard = TRACK_CARD_GetCard(u32TrackID);
	if (pCard)
	{
		return TRACK_UpdateRptrPos(pCard, u32TrackID, pu32ReadPos);
	}
	else
	{
		return MT_ERR_AO_SOUND_NOT_OPEN;
	}
}

mt_s32 AO_Track_GetAipReadPos(mt_u32 u32TrackID, mt_u32 *pu32ReadPos)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_GetReadPos(pCard, u32TrackID, pu32ReadPos);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

mt_s32 AO_Track_FlushBuf(mt_u32 u32TrackID)
{
    SND_CARD_STATE_S *pCard;
    u32TrackID &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(u32TrackID);

    if (pCard)
    {
        return TRACK_FlushBuf(pCard, u32TrackID);
    }
    else
    {
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

#ifdef MT_ALSA_AO_SUPPORT

/* track create kernel intf */
mt_s32 AO_Track_Kcreate(AO_Track_Create_Param_S_PTR  arg, struct file *file)
{
    mt_s32 s32Ret;
    mt_handle hHandle = MT_INVALID_HANDLE;
    AO_Track_Create_Param_S_PTR pstTrack = (AO_Track_Create_Param_S_PTR)arg;

    s32Ret = down_interruptible(&g_AoMutex);

    //if (MT_SUCCESS == AO_Track_AllocHandle(&hHandle, file))
    {
        s32Ret = AO_Track_Create(pstTrack->enSound, &pstTrack->stAttr, pstTrack->bAlsaTrack,
                              &pstTrack->stBuf,
                              hHandle);
        if (MT_SUCCESS != s32Ret)
        {
            AO_Track_FreeHandle(hHandle);
            up(&g_AoMutex);
            return MT_FAILURE;
        }

        AO_TRACK_SaveSuspendAttr(hHandle, pstTrack);
        pstTrack->hTrack = hHandle;
    }
    up(&g_AoMutex);

    return MT_SUCCESS;
}
/* track destroy kernel intf */
mt_s32 AO_Track_Kdestory(mt_handle  *arg)
{
    mt_s32 s32Ret;
    mt_handle hTrack = *(mt_handle *)arg;

    s32Ret = down_interruptible(&g_AoMutex);

    CHECK_AO_TRACK_OPEN(hTrack);
    s32Ret = AO_Track_Destory( hTrack );
    if (MT_SUCCESS != s32Ret)
    {
        up(&g_AoMutex);
        return MT_FAILURE;
    }
    AO_Track_FreeHandle(hTrack);

    up(&g_AoMutex);
    return MT_SUCCESS;
}

/* track start kernel intf */
mt_s32 AO_Track_Kstart(mt_handle  *arg)
{
    mt_s32 s32Ret;
    mt_handle hTrack = *(mt_handle *)arg;

    //s32Ret = down_interruptible(&g_AoMutex);	//alsa atomic area

    CHECK_AO_TRACK_OPEN(hTrack);
    s32Ret = AO_Track_Start(hTrack);

    //up(&g_AoMutex);
    return s32Ret;
}

/* track stop kernel intf */
mt_s32 AO_Track_Kstop(mt_handle  *arg)
{
    mt_s32 s32Ret;
    mt_handle hTrack = *(mt_handle *)arg;

    //s32Ret = down_interruptible(&g_AoMutex);

    CHECK_AO_TRACK_OPEN(hTrack);
    s32Ret = AO_Track_Stop(hTrack);

    //up(&g_AoMutex);
    return s32Ret;
}
/* track stop kernel intf */
mt_s32 AO_Track_Kflush(mt_handle  *arg)
{
    mt_s32 s32Ret;
    mt_handle hTrack = *(mt_handle *)arg;

    //s32Ret = down_interruptible(&g_AoMutex);

    CHECK_AO_TRACK_OPEN(hTrack);
    s32Ret = AO_Track_Flush(hTrack);

    //up(&g_AoMutex);
    return s32Ret;
}
#endif

/******************************Snd Cast FUNC*************************************/
//#ifdef MT_SND_CAST_SUPPORT//minnan remove
static mt_s32 AO_Cast_AllocHandle(mt_handle *phHandle, struct file *pstFile, MT_UNF_SND_CAST_ATTR_S *pstUserCastAttr)
{
    mt_u32 i;
    mt_s32 Ret;
    mt_u32 uFrameSize, uBufSize;
    mmz_buffer_s stRbfMmz;

    if (MT_NULL == phHandle)
    {
        MT_ERR_AO("Bad param!\n");
        return MT_FAILURE;
    }

    /* Check ready flag */
    if (s_stAoDrv.bReady != MT_TRUE)
    {
        MT_ERR_AO("Need open first!\n");
        return MT_FAILURE;
    }

    /* Check channel number */
    if (s_stAoDrv.u32CastNum >= AO_MAX_CAST_NUM)
    {
        MT_ERR_AO("Too many Cast:%d!\n", s_stAoDrv.u32CastNum);
        goto err0;
    }

    /* Allocate new channel */
    for (i = 0; i < AO_MAX_CAST_NUM; i++)
    {
        if (0 == atomic_read(&s_stAoDrv.astCastEntity[i].atmUseCnt))
        {
            s_stAoDrv.astCastEntity[i].u32File = (mt_u32)MT_NULL;
            break;
        }
    }

    if (i >= AO_MAX_CAST_NUM)
    {
        MT_ERR_AO("Too many Cast chans!\n");
        goto err0;
    }

    /* Allocate cast mmz resource */
    uFrameSize = AUTIL_CalcFrameSize(2, 16); /* fource 2ch 16bit */
    uBufSize = pstUserCastAttr->u32PcmFrameMaxNum * pstUserCastAttr->u32PcmSamplesPerFrame * uFrameSize;
    if (uBufSize > AO_CAST_MMZSIZE_MAX)
    {
        MT_ERR_AO("Invalid Cast FrameMaxNum(%d), PcmSamplesPerFrame(%d)!\n", pstUserCastAttr->u32PcmFrameMaxNum,
                  pstUserCastAttr->u32PcmSamplesPerFrame);
        goto err0;
    }

    Ret = mt_drv_mmz_alloc_and_map("AO_Cast", MMZ_OTHERS, AO_CAST_MMZSIZE_MAX, AIAO_BUFFER_ADDR_ALIGN, &stRbfMmz);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AIAO("MMZ_AllocAndMap failed\n");
        goto err0;
    }

    s_stAoDrv.astCastEntity[i].stRbfMmz   = stRbfMmz;
    s_stAoDrv.astCastEntity[i].u32ReqSize = uBufSize;

    /* Allocate resource */
    s_stAoDrv.astCastEntity[i].u32File = (long)pstFile;
    s_stAoDrv.u32CastNum++;
    atomic_inc(&s_stAoDrv.astCastEntity[ i].atmUseCnt);
    *phHandle = (MT_ID_AO << 16) |(MT_ID_CAST << 8) | i;
    return MT_SUCCESS;

err0:
    return MT_FAILURE;
}

static mt_void AO_Cast_FreeHandle(mt_handle hHandle)
{
    hHandle &= AO_CAST_CHNID_MASK;

    /* Freee cast mmz resource */
    mt_drv_mmz_unmap_and_release(&s_stAoDrv.astCastEntity[hHandle].stRbfMmz);

    s_stAoDrv.astCastEntity[hHandle].u32File = (long)MT_NULL;
    s_stAoDrv.u32CastNum--;
    atomic_set(&s_stAoDrv.astCastEntity[hHandle].atmUseCnt, 0);
}

#define CHECK_AO_CAST_OPEN(Cast) \
    do                                                         \
    {                                                          \
        CHECK_AO_CAST(Cast);                             \
        if (0 == atomic_read(&s_stAoDrv.astCastEntity[Cast & AO_CAST_CHNID_MASK].atmUseCnt))   \
        {                                                       \
            MT_WARN_AO(" Invalid Cast id 0x%x\n", Cast);        \
            return MT_ERR_AO_INVALID_PARA;                       \
        }                                                       \
    } while (0)


static SND_CARD_STATE_S * CAST_CARD_GetCard(mt_u32 Id)
{
    MT_UNF_SND_E sndx;
    SND_CARD_STATE_S *pCard = MT_NULL;

    if (Id >= AO_MAX_CAST_NUM)
    {
        MT_WARN_AO(" Invalid Cast id 0x%x\n", Id);
        return MT_NULL;
    }

    for (sndx = MT_UNF_SND_0; sndx < MT_UNF_SND_BUTT; sndx++)
    {
        pCard = SND_CARD_GetCard(sndx);
        if(pCard)
        {
            if (pCard->uSndCastInitFlag & (1L << Id))
            {
                return pCard;
            }
        }
    }

    return MT_NULL;
}

static mt_s32 AO_Cast_SetMute(mt_u32 u32CastID, MT_BOOL bMute)
{
    SND_CARD_STATE_S *pCard;

    u32CastID &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(u32CastID);
    if (pCard)
    {
        return CAST_SetMute(pCard, u32CastID, bMute);
    }
    else
    {
        MT_FATAL_AO("Ao Sound Not Open!\n");
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}

static mt_s32 AO_Cast_GetMute(mt_u32 u32CastID, MT_BOOL *pbMute)
{
    SND_CARD_STATE_S *pCard;

    u32CastID &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(u32CastID);

    if (pCard)
    {
        return CAST_GetMute(pCard, u32CastID, pbMute);
    }
    else
    {
        MT_FATAL_AO("Ao Sound Not Open!\n");
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}


static mt_s32 AO_Cast_SetAbsGain(mt_u32 u32CastID, MT_UNF_SND_ABSGAIN_ATTR_S stCastAbsGain)
{
    SND_CARD_STATE_S *pCard;
    u32CastID &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(u32CastID);

    if (pCard)
    {
        return CAST_SetAbsGain(pCard, u32CastID, &stCastAbsGain);
    }
    else
    {
        MT_FATAL_AO("Ao Sound Not Open!\n");
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}
static mt_s32 AO_Cast_GetAbsGain(mt_u32 u32CastID, MT_UNF_SND_ABSGAIN_ATTR_S *pstCastAbsGain)
{

    SND_CARD_STATE_S *pCard;
    u32CastID &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(u32CastID);

    if (pCard)
    {
        return CAST_GetAbsGain(pCard, u32CastID, pstCastAbsGain);
    }
    else
    {
        MT_FATAL_AO("Ao Sound Not Open!\n");
        return MT_ERR_AO_SOUND_NOT_OPEN;
    }
}


static mt_s32 AO_Cast_GetDefAttr(MT_UNF_SND_CAST_ATTR_S * pstDefAttr)
{
    return CAST_GetDefAttr(pstDefAttr);
}

static mt_void AO_Cast_SaveSuspendAttr(MT_UNF_SND_E enSound, mt_handle hHandle, MT_UNF_SND_CAST_ATTR_S *pstCastAttr)
{
    hHandle &= AO_TRACK_CHNID_MASK;
    s_stAoDrv.astCastEntity[hHandle].stSuspendAttr.enSound = enSound;
    s_stAoDrv.astCastEntity[hHandle].stSuspendAttr.stCastAttr = *pstCastAttr;
}

mt_s32 AO_Cast_Create(MT_UNF_SND_E enSound, MT_UNF_SND_CAST_ATTR_S *pstCastAttr, mmz_buffer_s *pstMMz, mt_handle hCast)
{
    SND_CARD_STATE_S *pCard;
    hCast &= AO_CAST_CHNID_MASK;
    pCard = SND_CARD_GetCard(enSound);
    CHECK_AO_NULL_PTR(pCard);

    return CAST_CreateNew(pCard, pstCastAttr, pstMMz, hCast);
}


mt_s32 AO_Cast_Destory(mt_handle hCast)
{
    SND_CARD_STATE_S *pCard;
    hCast &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(hCast);
    CHECK_AO_NULL_PTR(pCard);

    return CAST_DestroyCast(pCard, hCast);
}

mt_s32 AO_Cast_SetInfo(mt_handle hCast, mt_u32 u32UserVirtAddr)
{
    SND_CARD_STATE_S *pCard;
    hCast &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(hCast);
    CHECK_AO_NULL_PTR(pCard);

    return CAST_SetInfo(pCard, hCast, u32UserVirtAddr);
}


mt_s32 AO_Cast_GetInfo(mt_handle hCast, AO_Cast_Info_Param_S *pstInfo)
{
    SND_CARD_STATE_S *pCard;
    hCast &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(hCast);
    CHECK_AO_NULL_PTR(pCard);

    return CAST_GetInfo(pCard, hCast, pstInfo);
}


mt_s32 AO_Cast_SetEnable(mt_handle hCast, MT_BOOL bEnable)
{
    SND_CARD_STATE_S *pCard;
    hCast &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(hCast);
    CHECK_AO_NULL_PTR(pCard);

    return CAST_SetEnable(pCard, hCast, bEnable);
}

mt_s32 AO_Cast_GetEnable(mt_handle hCast, MT_BOOL *pbEnable)
{
    SND_CARD_STATE_S *pCard;
    hCast &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(hCast);
    CHECK_AO_NULL_PTR(pCard);

    return CAST_GetEnable(pCard, hCast, pbEnable);
}


static mt_s32 AO_Cast_ReadData(mt_handle hCast, AO_Cast_Data_Param_S *pstCastData)
{
    SND_CARD_STATE_S *pCard;
    hCast &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(hCast);
    CHECK_AO_NULL_PTR(pCard);

    return CAST_ReadData(pCard, hCast, pstCastData);
}

static mt_s32 AO_Cast_ReleseData(mt_handle hCast, AO_Cast_Data_Param_S *pstCastData)
{
    SND_CARD_STATE_S *pCard;
    hCast &= AO_CAST_CHNID_MASK;
    pCard = CAST_CARD_GetCard(hCast);
    CHECK_AO_NULL_PTR(pCard);

    return CAST_ReleaseData(pCard, hCast, pstCastData);
}
//#endif

/********************************Driver inteface FUNC****************************************/

/*********************************** Code ************************************/
//zgjiere; proc??èYDè?óê?·??ú×?? ?e??·???àà??óúV1R1
static MT_BOOL bSaveThreadRunFlag = MT_FALSE;
static MT_BOOL bSuspend2SaveThreadFlag = MT_FALSE;	//MT_TRUE meas suspend start, thread should exit
static volatile MT_BOOL bSaveThread2SuspendFlag = MT_FALSE;	//MT_TRUE means  suspend wait until hi_false

static mt_s32 SndProcSaveThread(void *Arg)
{
	mt_s32 s32Ret;
	SND_PCM_SAVE_ATTR_S *pstThreadArg = (SND_PCM_SAVE_ATTR_S *)Arg;
	//SND_PCM_SAVE_ATTR_S stThreadArg;

	//use cast
	MT_UNF_SND_CAST_ATTR_S stCastAttr;
	AO_Cast_Create_Param_S  stCastParam;
    AO_Cast_Enable_Param_S stEnableAttr;
	AO_Cast_Info_Param_S stCastInfo;
	AO_Cast_Data_Param_S stCastData;
    mt_handle hHandle = MT_INVALID_HANDLE;
	MT_UNF_SND_E  enSound;
	struct file *fileHandle;
	struct file *devfileHandle;

	//stThreadArg.enSound = pstThreadArg->enSound;
	//stThreadArg.fileHandle = pstThreadArg->fileHandle;
	//stThreadArg.devfileHandle = pstThreadArg->devfileHandle;

	enSound = SND_CARD_GetSnd(pstThreadArg->pCard);
	fileHandle = pstThreadArg->pCard->fileHandle;
	devfileHandle = pstThreadArg->devfileHandle;

	CHECK_AO_SNDCARD_OPEN(enSound);
	CHECK_AO_NULL_PTR(fileHandle);

    s32Ret = down_interruptible(&g_AoMutex);
	bSaveThread2SuspendFlag = MT_TRUE;

	s32Ret = AO_Cast_GetDefAttr(&stCastAttr);
	if(MT_SUCCESS != s32Ret)
	{
		up(&g_AoMutex);
		goto Close_File;
	}

	stCastParam.enSound = enSound;
	memcpy(&stCastParam.stCastAttr, &stCastAttr, sizeof(MT_UNF_SND_CAST_ATTR_S));
	//MT_ERR_AO(" u32PcmFrameMaxNum=%d \n", stCastParam.stCastAttr.u32PcmFrameMaxNum);
	//MT_ERR_AO(" u32PcmSamplesPerFrame=%d \n", stCastParam.stCastAttr.u32PcmSamplesPerFrame);

	if (MT_SUCCESS == AO_Cast_AllocHandle(&hHandle, fileHandle, &stCastParam.stCastAttr))
	{
		s32Ret = AO_Cast_Create(enSound, &stCastParam.stCastAttr, &s_stAoDrv.astCastEntity[hHandle & AO_CAST_CHNID_MASK].stRbfMmz,
							 hHandle);
		if (MT_SUCCESS != s32Ret)
		{
			AO_Cast_FreeHandle(hHandle);
			up(&g_AoMutex);
			goto Close_File;
		}
		//AO_Cast_SaveSuspendAttr(stCastParam.enSound, hHandle, &stCastParam.stCastAttr);   //NO resume
		stCastParam.u32ReqSize = s_stAoDrv.astCastEntity[hHandle & AO_CAST_CHNID_MASK].u32ReqSize;
		stCastParam.hCast = hHandle;
	}
	else
	{
		up(&g_AoMutex);
		goto Close_File;
	}

	stCastInfo.hCast = stCastParam.hCast;
    s32Ret = AO_Cast_GetInfo(stCastInfo.hCast, &stCastInfo);
	if(MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO(" AO_Cast_GetInfo Failed\n");
		up(&g_AoMutex);
		goto Destory_Cast;
	}
	//MT_ERR_AO(" stCastInfo u32KernelVirtAddr=0x%x \n", stCastInfo.u32KernelVirtAddr);

	stEnableAttr.hCast = stCastParam.hCast;
    stEnableAttr.bCastEnable = MT_TRUE;
	s32Ret = AO_Cast_SetEnable(stEnableAttr.hCast, stEnableAttr.bCastEnable);
	if(MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO(" AO_Cast_SetEnable Enable Failed\n");
		up(&g_AoMutex);
		//return MT_FAILURE;
		goto Destory_Cast;
	}
    up(&g_AoMutex);

	stCastData.hCast = stCastParam.hCast;
	while(MT_TRUE == bSaveThreadRunFlag) // NO !kthread_should_stop() to avoid dead lock
	{
		mt_u32 u32PcmSize;

		s32Ret = down_interruptible(&g_AoMutex);
		if(bSuspend2SaveThreadFlag == MT_TRUE)
		{
			//MT_ERR_AO("bSuspend2SaveThreadFlag  True!\n");
			up(&g_AoMutex);
			goto Destory_Cast;
		}

        s32Ret = AO_Cast_ReadData(stCastData.hCast, &stCastData);
		up(&g_AoMutex);
		if(MT_SUCCESS == s32Ret)
		{
			if(stCastData.stAOFrame.u32PcmSamplesPerFrame == 0)
			{
				msleep(5);
				continue;
			}
			else
			{
				//MT_ERR_AO(" Once Length : %d\n", u32PcmSize);
				//MT_ERR_AO(" Once Offset : %d\n", stCastData.u32DataOffset);
				u32PcmSize = stCastData.stAOFrame.u32PcmSamplesPerFrame * stCastData.stAOFrame.u32Channels * stCastData.stAOFrame.s32BitPerSample / 8;
				if(fileHandle)
				{
					mt_s32 s32Len = mt_drv_file_write(fileHandle, (mt_s8 *)(stCastInfo.u32KernelVirtAddr + stCastData.u32DataOffset) , u32PcmSize);
					if (s32Len != u32PcmSize)
					{
						MT_ERR_AO("mt_drv_file_write failed!\n");
						pstThreadArg->pCard->enSaveState = SND_DEBUG_CMD_CTRL_STOP;
						goto Destory_Cast;
					}

				}
				else
				{
					MT_ERR_AO("stThreadArg.fileHandle is NULL!\n");
					goto Destory_Cast;
				}
				s32Ret = down_interruptible(&g_AoMutex);
				if(bSuspend2SaveThreadFlag == MT_TRUE)
				{
					//MT_ERR_AO("bSuspend2SaveThreadFlag  True!\n");
					up(&g_AoMutex);
					goto Destory_Cast;
				}

				s32Ret = AO_Cast_ReleseData(stCastData.hCast, &stCastData);
				up(&g_AoMutex);
				if(MT_SUCCESS != s32Ret)
				{
					goto Close_File;
				}
			}
		}
		else
		{
			goto Close_File;
		}
	}

Destory_Cast:
	CHECK_AO_CAST_OPEN(stCastParam.hCast);
    s32Ret = down_interruptible(&g_AoMutex);
	s32Ret = AO_Cast_Destory(stCastParam.hCast);
	AO_Cast_FreeHandle(stCastParam.hCast);
	up(&g_AoMutex);
Close_File:
	if(fileHandle)
	{
		mt_drv_file_close(fileHandle);
	}
	s32Ret = AO_Snd_Kclose(enSound, devfileHandle);
	if(MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO("AO_Snd_Kclose %d failed \n", (mt_u32)enSound);
	}
	s32Ret = AO_DRV_Krelease(devfileHandle);
	if(MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO("AO_DRV_Krelease\n");
	}
	bSaveThread2SuspendFlag = MT_FALSE;
	return MT_SUCCESS;

}

mt_s32 SND_WriteProc(SND_CARD_STATE_S *pCard, SND_DEBUG_CMD_CTRL_E enCmd)
{
    mt_char szPath[AO_SOUND_PATH_NAME_MAXLEN + AO_SOUND_FILE_NAME_MAXLEN] = {0};
	MT_UNF_SND_E  enSound;
	static struct  task_struct	*g_pstSndSaveThread = NULL;	//name todo
	static SND_PCM_SAVE_ATTR_S  stThreadArg;
	static AO_SND_Open_Param_S  stSndOpenParam;
	static struct file   g_file;	//just a dev handle no use
	struct tm now;
	mt_s32 s32Ret;
	enSound = SND_CARD_GetSnd(pCard);
	if(SND_DEBUG_CMD_CTRL_START == enCmd && pCard->enSaveState == SND_DEBUG_CMD_CTRL_STOP)
	{
        if(MT_SUCCESS != mt_drv_file_get_storepath(szPath, AO_SOUND_PATH_NAME_MAXLEN))
        {
            MT_ERR_AO("get store path failed\n");
            return MT_FAILURE;
        }

        time64_to_tm(ktime_get_seconds(), 0, &now);
        snprintf(szPath, sizeof(szPath), "%s/sound%d_%02u_%02u_%02u.pcm", szPath, (mt_u32)enSound, now.tm_hour, now.tm_min, now.tm_sec);

        pCard->fileHandle = mt_drv_file_open(szPath, 1);
        if (!pCard->fileHandle)
        {
            MT_ERR_AO("open %s error\n", szPath);
            return MT_FAILURE;
        }
		//stThreadArg.enSound = enSound;
		//stThreadArg.fileHandle = pCard->fileHandle;
		stThreadArg.pCard = pCard;
		stThreadArg.devfileHandle = &g_file;

		stSndOpenParam.enSound = enSound;
		up(&g_AoMutex);
		s32Ret = AO_DRV_Kopen(&g_file);
		if(MT_SUCCESS != s32Ret)
		{
			MT_ERR_AO("AO_DRV_Kopen failed\n");
		}

		s32Ret = AO_Snd_Kopen(&stSndOpenParam, &g_file);	//never first open
		if(MT_SUCCESS != s32Ret)
		{
			MT_ERR_AO("AO_Snd_Kopen failed\n");
		}
		s32Ret = down_interruptible(&g_AoMutex);

		bSaveThreadRunFlag = MT_TRUE;
		g_pstSndSaveThread = kthread_create(SndProcSaveThread, &stThreadArg, "AoSndProcSave");		//Name To Do
        if(MT_NULL == g_pstSndSaveThread)
		{
            MT_ERR_AO("creat sound proc write thread failed\n");
            return MT_FAILURE;
		}
		pCard->enSaveState = enCmd;
		wake_up_process(g_pstSndSaveThread);

		pCard->u32SaveCnt++;

	}

	if(SND_DEBUG_CMD_CTRL_STOP == enCmd && pCard->enSaveState == SND_DEBUG_CMD_CTRL_START)
	{
		bSaveThreadRunFlag = MT_FALSE;
	    //kthread_stop(g_pstSndSaveThread);
		g_pstSndSaveThread = MT_NULL;

		//Warnning : mt_drv_file_close called in Thread, To avoid hold mutex lock long time
		pCard->enSaveState = enCmd;
	}

//	if(pCard)
//		pCard->enSaveState = enCmd;
	return MT_SUCCESS;
}
static mt_s32 AOReadSndProc( struct seq_file* p, MT_UNF_SND_E enSnd )
{
    mt_u32 i;
    MT_UNF_SND_ATTR_S* pstSndAttr;
    SND_CARD_STATE_S* pCard;

    pCard = SND_CARD_GetCard(enSnd);
    if (MT_NULL == pCard)
    {
        printk("\n\t\t\t---Sound[%d] Not Open---\n", (mt_u32)enSnd );
        return MT_SUCCESS;
    }

    PROC_PRINT( p, "\n\t\t\t---Sound[%d] Status---\n", (mt_u32)enSnd );
    pstSndAttr = &pCard->stUserOpenParam;

    PROC_PRINT( p,"mute         :%s\n",(MT_TRUE==pCard->ao_record.is_mute)?"yes":"no");
    PROC_PRINT( p,"trackmode    :%s\n",AUTIL_TrackMode2Name(pCard->ao_record.trackmode));
    PROC_PRINT( p,"volume       :%d\n",pCard->ao_record.volume);
    PROC_PRINT( p,"SampleRate   :%d\n",pstSndAttr->enSampleRate);
    PROC_PRINT( p,"buffersize   :%x\n",pstSndAttr->u32MasterOutputBufSize);

    if (SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
    {
        PROC_PRINT( p,
                    "SPDIF Status :UserSetMode(%s) DataFormat(%s)\n",
                    AUTIL_SpdifMode2Name(pCard->enUserSpdifMode),
                    AUTIL_Format2Name(pCard->u32SpdifDataFormat));
    }
    if (SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
    {
        PROC_PRINT( p,
                    "HDMI Status  :UserSetMode(%s) DataFormat(%s)\n",
                    AUTIL_HdmiMode2Name(pCard->enUserHdmiMode),
                    AUTIL_Format2Name(pCard->u32HdmiDataFormat));
    }

    PROC_PRINT( p, "\n\t\t\t---OutPort Status---\n");
    for (i = 0; i < pstSndAttr->u32PortNum; i++)
    {
        PROC_PRINT(p, "op[%d]       :%s\n",i,AUTIL_Port2Name(pstSndAttr->stOutport[i].enOutPort));
        //SND_ReadOpProc( p, pCard, pstSndAttr->stOutport[i].enOutPort );
    }
    #ifdef MT_SND_CAST_SUPPORT
    if(s_stAoDrv.u32CastNum>0)
    {
        PROC_PRINT( p, "\n\t\t\t---Cast Status---\n" );
        CAST_ReadProc(p,pCard);
    }
    #endif

    PROC_PRINT( p, "\n\t\t\t---Track Status---\n" );
    Track_ReadProc( p, pCard );

#if defined(CMTP_TYPE_hi3751v100)
    PROC_PRINT( p, "\n\t\t\t---Audio Effect Status\n" );
    SND_ReadAefProc( p, pCard );
#endif

    return MT_SUCCESS;
}

mt_s32 AO_DRV_ReadProc( struct seq_file* p, mt_void* v )
{
    mt_u32 u32Snd=0;
    mt_proc_entry_t *pstProcItem;

    pstProcItem = p->private;

    (mt_void)sscanf(pstProcItem->entry_name, "sound%1d", &u32Snd);

    if(u32Snd >= AO_MAX_TOTAL_SND_NUM)
    {
        PROC_PRINT(p, "Invalid Sound ID:%d.\n", u32Snd);
        return MT_FAILURE;
    }

    AOReadSndProc( p, (MT_UNF_SND_E)u32Snd );

    return MT_SUCCESS;
}

static mt_void AO_Hdmi_Debug(SND_CARD_STATE_S* pCard)
{
    if(MT_FALSE == pCard->bHdmiDebug)
    {
        pCard->bHdmiDebug = MT_TRUE;
    }
    else
    {
        pCard->bHdmiDebug = MT_FALSE;
    }
}

mt_s32 AO_DRV_WriteProc(struct file * file, const char __user * buf, size_t count, loff_t *ppos)
{
    mt_s32 s32Ret;
    mt_u32 u32Snd;
    SND_CARD_STATE_S* pCard;
    mt_u32 u32TrackId = AO_MAX_TOTAL_TRACK_NUM;
    SND_DEBUG_CMD_PROC_E enProcCmd;
    SND_DEBUG_CMD_CTRL_E enCtrlCmd;
    mt_char szBuf[48];
    mt_char *pcBuf = szBuf;
    mt_char *pcStartCmd = "start";
    mt_char *pcStopCmd = "stop";
    mt_char *pcSaveTrackCmd = "save_track";
    mt_char *pcSaveSoundCmd = "save_sound";

    mt_char *pcHelpCmd = "help";
    mt_char *pcHdmiCmd = "hdmi";
    struct seq_file *p = file->private_data;
    mt_proc_entry_t *pstProcItem = p->private;

    s32Ret = down_interruptible(&g_AoMutex);

    if (copy_from_user(szBuf, buf, count))
    {
        MT_ERR_AO("copy from user failed\n");
        up(&g_AoMutex);
        return MT_FAILURE;
    }

    (mt_void)sscanf(pstProcItem->entry_name, "sound%1d", &u32Snd);
    if(u32Snd >= AO_MAX_TOTAL_SND_NUM)
    {
        MT_ERR_AO("Invalid Sound ID:%d.\n", u32Snd);
        goto SAVE_CMD_FAULT;
    }

    pCard = SND_CARD_GetCard((MT_UNF_SND_E)u32Snd);
    if(MT_NULL == pCard)
    {
        MT_ERR_AO("Sound %d is not open\n", u32Snd);
        goto SAVE_CMD_FAULT;
    }

    AO_STRING_SKIP_BLANK(pcBuf);
    if (strstr(pcBuf,pcSaveTrackCmd))
    {
        enProcCmd = SND_DEBUG_CMD_PROC_SAVE_TRACK;
        pcBuf += strlen(pcSaveTrackCmd);
    }
    else if (strstr(pcBuf,pcHelpCmd))
    {
        AO_DEBUG_SHOW_HELP(u32Snd);
        up(&g_AoMutex);
        return count;
    }
    else if (strstr(pcBuf,pcHdmiCmd))
    {
        AO_Hdmi_Debug(pCard);
        up(&g_AoMutex);
        return count;
    }
    else if (strstr(pcBuf,pcSaveSoundCmd))
    {
        enProcCmd = SND_DEBUG_CMD_PROC_SAVE_SOUND;
        pcBuf += strlen(pcSaveSoundCmd);
    }
    else
    {
        goto SAVE_CMD_FAULT;
    }

    AO_STRING_SKIP_BLANK(pcBuf);
    if(SND_DEBUG_CMD_PROC_SAVE_TRACK == enProcCmd)
    {
        if (pcBuf[0] < '0' || pcBuf[0] > '9')//do not have param
        {
            goto SAVE_CMD_FAULT;
        }
        u32TrackId = (mt_u32)simple_strtoul(pcBuf, &pcBuf, 10);
        if(u32TrackId >= AO_MAX_TOTAL_TRACK_NUM)
        {
            goto SAVE_CMD_FAULT;
        }
        AO_STRING_SKIP_NON_BLANK(pcBuf);
        AO_STRING_SKIP_BLANK(pcBuf);
    }

    if (strstr(pcBuf,pcStartCmd))
    {
        enCtrlCmd = SND_DEBUG_CMD_CTRL_START;
    }
    else if (strstr(pcBuf,pcStopCmd))
    {
        enCtrlCmd = SND_DEBUG_CMD_CTRL_STOP;
    }
    else
    {
        goto SAVE_CMD_FAULT;
    }

    if(SND_DEBUG_CMD_PROC_SAVE_TRACK == enProcCmd)
    {
        s32Ret = TRACK_WriteProc(pCard, u32TrackId, enCtrlCmd);
        if (s32Ret != MT_SUCCESS)
        {
            goto SAVE_CMD_FAULT;
        }
    }

	if(SND_DEBUG_CMD_PROC_SAVE_SOUND == enProcCmd)
	{
		s32Ret = SND_WriteProc(pCard, enCtrlCmd);

		if (s32Ret != MT_SUCCESS)
		{
			goto SAVE_CMD_FAULT;
		}
	}
    up(&g_AoMutex);
    return count;

SAVE_CMD_FAULT:
    MT_ERR_AO("proc cmd is fault\n");
    AO_DEBUG_SHOW_HELP(u32Snd);
    up(&g_AoMutex);
    return MT_FAILURE;
}

static mt_s32 AO_RegProc(mt_u32 u32Snd)
{
    mt_char aszBuf[16];
    mt_proc_entry_t*  pProcItem;

    /* Check parameters */
    if (MT_NULL == s_stAoDrv.pstProcParam)
    {
        return MT_FAILURE;
    }

    /* Create proc */
    snprintf(aszBuf, sizeof(aszBuf), "sound%d", u32Snd);
    pProcItem = mt_drv_proc_add_module(aszBuf, MT_NULL, MT_NULL);
    if (!pProcItem)
    {
        MT_FATAL_AO("Create ade proc entry fail!\n");
        return MT_FAILURE;
    }

    /* Set functions */
    pProcItem->read  = s_stAoDrv.pstProcParam->pfnReadProc;
    pProcItem->write = s_stAoDrv.pstProcParam->pfnWriteProc;

    MT_INFO_AO("Create Ao proc entry for OK!\n");
    return MT_SUCCESS;
}

static mt_void AO_UnRegProc(mt_u32 u32Snd)
{
    mt_char aszBuf[16];
    snprintf(aszBuf, sizeof(aszBuf), "sound%d", u32Snd);

    mt_drv_proc_rm_module(aszBuf);
    return;
}
#if defined(MT_AIAO_VERIFICATION_SUPPORT)
DRV_PROC_EX_S stAIAOCbbOpt =
{
     .fnRead = AIAO_VERI_ProcRead,
};
#endif

static mt_s32 AO_OpenDev(mt_void)
{
    mt_u32 i;

    //mt_s32 s32Ret;

    /* Init global track parameter */
    for (i = 0; i < AO_MAX_TOTAL_TRACK_NUM; i++)
    {
        atomic_set(&s_stAoDrv.astTrackEntity[i].atmUseCnt, 0);
        s_stAoDrv.astTrackEntity[i].u32File = 0;
    }

    s_stAoDrv.u32SndNum = 0;
    /* Init global snd parameter */
    for (i = 0; i < AO_MAX_TOTAL_SND_NUM; i++)
    {
        atomic_set(&s_stAoDrv.astSndEntity[i].atmUseTotalCnt, 0);
    }

    //s_stAoDrv.pAdspFunc = MT_NULL;
    s_stAoDrv.pstPDMFunc = MT_NULL;

    /* Get pdm functions */
#if 0 //minnan remove useless
    s32Ret = mt_drv_module_getfunction(MT_ID_PDM, (mt_void**)&s_stAoDrv.pstPDMFunc);
    if (MT_SUCCESS != s32Ret)
    {
        MT_FATAL_AO("Get pdm function err:%#x!\n", s32Ret);
        goto err;
    }
#endif

    /* Get adsp functions */
#if 0  //minnan remove
    s32Ret = mt_drv_module_getfunction(MT_ID_ADSP, (mt_void**)&s_stAoDrv.pAdspFunc);
    if (MT_SUCCESS != s32Ret)
    {
        MT_FATAL_AO("Get adsp function err:%#x!\n", s32Ret);
        goto err;
    }

    /* HAL_AOE_Init , Init aoe hardare */
    if (s_stAoDrv.pAdspFunc && s_stAoDrv.pAdspFunc->pfnADSP_LoadFirmware)
    {
        s32Ret = (s_stAoDrv.pAdspFunc->pfnADSP_LoadFirmware)(ADSP_CODE_AOE);
        if (MT_SUCCESS != s32Ret)
        {
            goto err;
        }
        if (s_stAoDrv.pAdspFunc && s_stAoDrv.pAdspFunc->pfnADSP_GetAoeFwmInfo)
        {
            ADSP_FIRMWARE_AOE_INFO_S stAoeInfo;
            s32Ret = (s_stAoDrv.pAdspFunc->pfnADSP_GetAoeFwmInfo)(ADSP_CODE_AOE,&stAoeInfo);
            if (MT_SUCCESS != s32Ret)
            {
                s32Ret = (s_stAoDrv.pAdspFunc->pfnADSP_UnLoadFirmware)(ADSP_CODE_AOE);
                goto err;
            }
            HAL_AOE_Init(stAoeInfo.bAoeSwFlag);
        }
    }

#endif
    /* HAL_AIAO_Init, Init aiao hardare */
    HAL_AIAO_Init();

#ifdef MT_SND_CAST_SUPPORT
    /* HAL_CAST_Init , Init cast hardare */
    HAL_CAST_Init();
#endif

#if defined(MT_AIAO_VERIFICATION_SUPPORT)
{
     mt_proc_entry_t *item;
     AIAO_VERI_Open();
     item = mt_drv_proc_add_module(AIAO_VERI_PROC_NAME, &stAIAOCbbOpt, NULL);
     if (!item)
     {
        MT_WARN_AIAO("add proc aiao_port failed\n");
     }
}
#endif

    /* Set ready flag */
    s_stAoDrv.bReady = MT_TRUE;

    MT_INFO_AO("AO_OpenDev OK.\n");
    return MT_SUCCESS;
//err:
//    return MT_FAILURE;
}

static mt_s32 AO_CloseDev(mt_void)
{
    mt_u32 i,j;

    /* Reentrant */
    if (s_stAoDrv.bReady == MT_FALSE)
    {
        return MT_SUCCESS;
    }

    /* Set ready flag */
    s_stAoDrv.bReady = MT_FALSE;

#ifdef MT_SND_CAST_SUPPORT
    /* Free all Cast */
    for (i = 0; i < AO_MAX_CAST_NUM; i++)
    {
        {
            if (atomic_read(&s_stAoDrv.astCastEntity[i].atmUseCnt))
            {
                (mt_void)AO_Cast_Destory( i );
                AO_Cast_FreeHandle(i);
            }
        }
    }
#endif
    /* Free all track */
    for (i = 0; i < AO_MAX_TOTAL_TRACK_NUM; i++)
    {
        {
            if (atomic_read(&s_stAoDrv.astTrackEntity[i].atmUseCnt))
            {
                (mt_void)AO_Track_Destory( i );
                AO_Track_FreeHandle(i);
            }
        }
    }

    /* Free all snd */
    for (i = 0; i < AO_MAX_TOTAL_SND_NUM; i++)
    {
        if (s_stAoDrv.astSndEntity[i].pCard)
        {
            for(j =0; j<SND_MAX_OPEN_NUM;j++)
            {
                if(s_stAoDrv.astSndEntity[i].u32File[j] != 0)
                {
                    if(1 == atomic_read(&s_stAoDrv.astSndEntity[i].atmUseTotalCnt))
                    {
                        (mt_void)AO_SND_Close( i, MT_FALSE );
                    }
                    AO_Snd_FreeHandle(i, (struct file *)(s_stAoDrv.astSndEntity[i].u32File[j]));
                }
            }
        }
    }

    /* HAL_AOE_DeInit */
#if 0
    if (s_stAoDrv.pAdspFunc && s_stAoDrv.pAdspFunc->pfnADSP_UnLoadFirmware)
    {
        HAL_AOE_DeInit( );
        s32Ret = (s_stAoDrv.pAdspFunc->pfnADSP_UnLoadFirmware)(ADSP_CODE_AOE);
    }
#endif
#ifdef MT_SND_CAST_SUPPORT
    /* HAL_CAST_DeInit  */
    HAL_CAST_DeInit();
#endif

    /* HAL_AIAO_DeInit */
    HAL_AIAO_DeInit();

#if defined(MT_AIAO_VERIFICATION_SUPPORT)
     mt_drv_proc_rm_module(AIAO_VERI_PROC_NAME);
     AIAO_VERI_Release();
#endif

    return MT_SUCCESS;
}

static mt_s32 AO_ProcessCmd( struct inode *inode, struct file *file, mt_u32 cmd, mt_void *arg )
{
    mt_s32 Ret = MT_SUCCESS;

    mt_handle hHandle = MT_INVALID_HANDLE;
    MT_UNF_SND_E enSound = MT_UNF_SND_BUTT;
#if defined(MT_AIAO_VERIFICATION_SUPPORT)
    if((cmd&0xff)>=CMD_AIAO_VERI_IOCTL)
    {
        return AIAO_VERI_ProcessCmd(inode,file,cmd,arg);
    }
#endif
// mzhu_fpga
	return Ret;

    /* Check parameter in this switch */
    switch (cmd)
    {
    case CMD_AO_TRACK_DESTROY:
    case CMD_AO_TRACK_START:
    case CMD_AO_TRACK_STOP:
    case CMD_AO_TRACK_PAUSE:
    case CMD_AO_TRACK_FLUSH:
    {
        if (MT_NULL == arg)
        {
            MT_ERR_AO("CMD %x : %px Bad arg!\n",cmd, (mt_void*)arg);
            return MT_ERR_AO_INVALID_PARA;
        }

        hHandle = *(mt_handle *)arg & AO_TRACK_CHNID_MASK;
        CHECK_AO_TRACK(hHandle);
        break;
    }
    case CMD_AO_TRACK_GETDEFATTR:
    case CMD_AO_TRACK_SETATTR:
    case CMD_AO_TRACK_GETATTR:
    case CMD_AO_TRACK_CREATE:
    case CMD_AO_TRACK_SETWEITHT:
    case CMD_AO_TRACK_GETWEITHT:
    case CMD_AO_TRACK_SETABSGAIN:
    case CMD_AO_TRACK_GETABSGAIN:
    case CMD_AO_TRACK_SETMUTE:
    case CMD_AO_TRACK_GETMUTE:
	case CMD_AO_TRACK_SETCHANNELMODE:
	case CMD_AO_TRACK_GETCHANNELMODE:
    case CMD_AO_TRACK_SETSPEEDADJUST:
    case CMD_AO_TRACK_GETDELAYMS:
    case CMD_AO_TRACK_SETEOSFLAG:
    case CMD_AO_TRACK_SENDDATA:
    case CMD_AO_GETSNDDEFOPENATTR:
    case CMD_AO_TRACK_ATTACHAI:
    case CMD_AO_TRACK_DETACHAI:
    case CMD_AO_SND_OPEN:
    case CMD_AO_SND_CLOSE:
    case CMD_AO_SND_SETADAC:
    case CMD_AO_SND_SETMUTE:
    case CMD_AO_SND_GETMUTE:
    case CMD_AO_SND_SETHDMIMODE:
    case CMD_AO_SND_GETHDMIMODE:
    case CMD_AO_SND_SETSPDIFMODE:
    case CMD_AO_SND_GETSPDIFMODE:
    case CMD_AO_SND_SETVOLUME:
    case CMD_AO_SND_GETVOLUME:
    case CMD_AO_SND_SETSAMPLERATE:
    case CMD_AO_SND_GETSAMPLERATE:
    case CMD_AO_SND_SETTRACKMODE:
    case CMD_AO_SND_GETTRACKMODE:
    case CMD_AO_SND_SETALLTRACKMUTE:
    case CMD_AO_SND_GETALLTRACKMUTE:
    case CMD_AO_SND_SETSMARTVOLUME:
    case CMD_AO_SND_GETSMARTVOLUME:
    case CMD_AO_SND_ATTACHAEF:
    case CMD_AO_SND_DETACHAEF:
    case CMD_AO_SND_SETAEFBYPASS:
    case CMD_AO_SND_GETAEFBYPASS:
    case CMD_AO_SND_SETSPDIFSCMSMODE:
    case CMD_AO_SND_GETSPDIFSCMSMODE:
    case CMD_AO_SND_SETSPDIFCATEGORYCODE:
    case CMD_AO_SND_GETSPDIFCATEGORYCODE:
    case CMD_AO_SND_GETXRUNCOUNT:
#ifdef MT_SND_CAST_SUPPORT
    case CMD_AO_CAST_GETDEFATTR:
    case CMD_AO_CAST_CREATE:
    case CMD_AO_CAST_DESTROY:
    case CMD_AO_CAST_SETENABLE:
    case CMD_AO_CAST_GETENABLE:
    case CMD_AO_CAST_SETINFO:
    case CMD_AO_CAST_GETINFO:
    case CMD_AO_CAST_ACQUIREFRAME:
    case CMD_AO_CAST_RELEASEFRAME:
    case CMD_AO_CAST_SETABSGAIN:
    case CMD_AO_CAST_GETABSGAIN:
    case CMD_AO_CAST_SETMUTE:
    case CMD_AO_CAST_GETMUTE:
#endif
    {
        if (MT_NULL == arg)
        {
            MT_ERR_AO("CMD %x : %px Bad arg!\n",cmd,  (mt_void*)arg);
            return MT_ERR_AO_INVALID_PARA;
        }

        break;
    }
    }

    switch (cmd)
    {
        //Snd CMD TYPE(call hal_aiao)
    case CMD_AO_GETSNDDEFOPENATTR:
    {
        Ret = AOGetSndDefOpenAttr((AO_SND_OpenDefault_Param_S_PTR)arg);
        break;
    }
    case CMD_AO_SND_OPEN:
    {
        //DRV_AO_STATE_S *pAOState = file->private_data;
        AO_SND_Open_Param_S_PTR pstSndParam = ( AO_SND_Open_Param_S_PTR )arg;
        enSound = pstSndParam->enSound;
        CHECK_AO_SNDCARD( enSound );

        Ret = AO_Snd_AllocHandle(enSound, file); //find handle,malloc pcard
        if (MT_SUCCESS == Ret)
        {
            if (1 == atomic_read(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt))
            {
                Ret = AO_SND_Open( enSound, &pstSndParam->stAttr, NULL, MT_FALSE);
                if (MT_SUCCESS != Ret)
                {
                    AO_Snd_FreeHandle(enSound, file);
                    break;
                }
            }
        }
        else //cooperadd
        {
          MT_INFO_AO("\n\n cooper  !!!!  AO_Snd_AllocHandle errrr \n\n\n");
          break;
        }

        break;
    }
    case CMD_AO_SND_CLOSE:
    {
        //DRV_AO_STATE_S *pAOState = file->private_data;
        enSound = *( MT_UNF_SND_E *)arg;
        CHECK_AO_SNDCARD_OPEN( enSound );


        if (1 == atomic_read(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt))
        {
            Ret = AO_SND_Close( enSound, MT_FALSE );
            if (MT_SUCCESS != Ret)
            {
                break;
            }
            AO_Snd_FreeHandle(enSound, file);
        }

        Ret = MT_SUCCESS;
        break;
    }
    
    case CMD_AO_SND_SETADAC:
    {
        MT_BOOL bEn = *(MT_BOOL*)arg;
        CHECK_AO_SNDCARD_OPEN(0);
        Ret = AO_SND_SetAdacOnOff(MT_UNF_SND_0, bEn);
        break;
    }
    
    case CMD_AO_SND_SETMUTE:
    {
        AO_SND_Mute_Param_S_PTR pstMute = (AO_SND_Mute_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstMute->enSound );
        Ret = AO_SND_SetMute(pstMute->enSound, pstMute->enOutPort, pstMute->bMute);
        break;
    }

    case CMD_AO_SND_GETMUTE:
    {
        AO_SND_Mute_Param_S_PTR pstMute = (AO_SND_Mute_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstMute->enSound );
        Ret = AO_SND_GetMute(pstMute->enSound, pstMute->enOutPort, &pstMute->bMute);
        break;
    }

    case CMD_AO_SND_SETHDMIMODE:
    {
        AO_SND_HdmiMode_Param_S_PTR pstMode = (AO_SND_HdmiMode_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstMode->enSound );
        Ret = AO_SND_SetHdmiMode(pstMode->enSound, pstMode->enOutPort, pstMode->enMode);

	MT_INFO_AO("------\n");
        break;
    }
    case CMD_AO_SND_GETHDMIMODE:
    {
        AO_SND_HdmiMode_Param_S_PTR pstMode = (AO_SND_HdmiMode_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstMode->enSound );
        Ret = AO_SND_GetHdmiMode(pstMode->enSound, pstMode->enOutPort, &pstMode->enMode);
        break;
    }

    case CMD_AO_SND_SETSPDIFMODE:
    {
        AO_SND_SpdifMode_Param_S_PTR pstMode = (AO_SND_SpdifMode_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstMode->enSound );
        Ret = AO_SND_SetSpdifMode(pstMode->enSound, pstMode->enOutPort, pstMode->enMode);
        break;
    }
    case CMD_AO_SND_GETSPDIFMODE:
    {
        AO_SND_SpdifMode_Param_S_PTR pstMode = (AO_SND_SpdifMode_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstMode->enSound );
        Ret = AO_SND_GetSpdifMode(pstMode->enSound, pstMode->enOutPort, &pstMode->enMode);
        break;
    }

    case CMD_AO_SND_SETVOLUME:
    {
        AO_SND_Volume_Param_S_PTR pstVolume = (AO_SND_Volume_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstVolume->enSound );
        Ret = AO_SND_SetVolume(pstVolume->enSound, pstVolume->enOutPort, pstVolume->stGain);
        break;
    }

    case CMD_AO_SND_GETVOLUME:
    {
        AO_SND_Volume_Param_S_PTR pstVolume = (AO_SND_Volume_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstVolume->enSound );
        Ret = AO_SND_GetVolume(pstVolume->enSound, pstVolume->enOutPort, &pstVolume->stGain);
        break;
    }

    case CMD_AO_SND_SETSPDIFSCMSMODE:
    {
        AO_SND_SpdifSCMSMode_Param_S_PTR pstSCMSMode = (AO_SND_SpdifSCMSMode_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstSCMSMode->enSound );
        Ret = AO_SND_SetSpdifSCMSMode(pstSCMSMode->enSound, pstSCMSMode->enOutPort, pstSCMSMode->enSCMSMode);
        break;
    }
    case CMD_AO_SND_GETSPDIFSCMSMODE:
    {
        AO_SND_SpdifSCMSMode_Param_S_PTR pstSCMSMode = (AO_SND_SpdifSCMSMode_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstSCMSMode->enSound );
        Ret = AO_SND_GetSpdifSCMSMode(pstSCMSMode->enSound, pstSCMSMode->enOutPort, &pstSCMSMode->enSCMSMode);
        break;
    }

    case CMD_AO_SND_SETSPDIFCATEGORYCODE:
    {
        AO_SND_SpdifCategoryCode_Param_S_PTR pstCategoryCode = (AO_SND_SpdifCategoryCode_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstCategoryCode->enSound );
        Ret = AO_SND_SetSpdifCategoryCode(pstCategoryCode->enSound, pstCategoryCode->enOutPort, pstCategoryCode->enCategoryCode);
        break;
    }
    case CMD_AO_SND_GETSPDIFCATEGORYCODE:
    {
        AO_SND_SpdifCategoryCode_Param_S_PTR pstCategoryCode = (AO_SND_SpdifCategoryCode_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstCategoryCode->enSound );
        Ret = AO_SND_GetSpdifCategoryCode(pstCategoryCode->enSound, pstCategoryCode->enOutPort, &pstCategoryCode->enCategoryCode);
        break;
    }
    case CMD_AO_SND_SETSAMPLERATE:
    {
        AO_SND_SampleRate_Param_S_PTR pstSampleRate = (AO_SND_SampleRate_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstSampleRate->enSound );
        Ret = AO_SND_SetSampleRate(pstSampleRate->enSound, pstSampleRate->enOutPort, pstSampleRate->enSampleRate);
        break;
    }

    case CMD_AO_SND_GETSAMPLERATE:
    {
        AO_SND_SampleRate_Param_S_PTR pstSampleRate = (AO_SND_SampleRate_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstSampleRate->enSound );
        Ret = AO_SND_GetSampleRate(pstSampleRate->enSound, pstSampleRate->enOutPort, &pstSampleRate->enSampleRate);
        break;
    }

    case CMD_AO_SND_SETTRACKMODE:
    {
        AO_SND_TrackMode_Param_S_PTR pstMode = (AO_SND_TrackMode_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstMode->enSound );
        Ret = AO_SND_SetTrackMode(pstMode->enSound, pstMode->enOutPort, pstMode->enMode);
        break;
    }

    case CMD_AO_SND_GETTRACKMODE:
    {
        AO_SND_TrackMode_Param_S_PTR pstMode = (AO_SND_TrackMode_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstMode->enSound );
        Ret = AO_SND_GetTrackMode(pstMode->enSound, pstMode->enOutPort, &pstMode->enMode);
        break;
    }

    case CMD_AO_SND_SETALLTRACKMUTE:
	{
        AO_SND_AllTrackMute_Param_S_PTR pstAllMute = (AO_SND_AllTrackMute_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstAllMute->enSound );
        Ret = AO_SND_SetAllTrackMute(pstAllMute->enSound, pstAllMute->bMute);
        break;
	}
    case CMD_AO_SND_GETALLTRACKMUTE:
	{
        AO_SND_AllTrackMute_Param_S_PTR pstAllMute = (AO_SND_AllTrackMute_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstAllMute->enSound );
        Ret = AO_SND_GetAllTrackMute(pstAllMute->enSound, &pstAllMute->bMute);
        break;
	}
    case CMD_AO_SND_SETSMARTVOLUME:
    {
        AO_SND_SmartVolume_Param_S_PTR pstSmartVol = (AO_SND_SmartVolume_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstSmartVol->enSound );
        Ret = AO_SND_SetSmartVolume(pstSmartVol->enSound, pstSmartVol->enOutPort, pstSmartVol->bSmartVolume);
        break;
    }

    case CMD_AO_SND_GETSMARTVOLUME:
    {
        AO_SND_SmartVolume_Param_S_PTR pstSmartVol = (AO_SND_SmartVolume_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN( pstSmartVol->enSound );
        Ret = AO_SND_GetSmartVolume(pstSmartVol->enSound, pstSmartVol->enOutPort, &pstSmartVol->bSmartVolume);
        break;
    }

    case CMD_AO_SND_ATTACHAEF:
    {
        AO_SND_AttAef_Param_S_PTR pstSndAttAef = (AO_SND_AttAef_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN(pstSndAttAef->enSound);
        Ret = AO_SND_AttachAef(pstSndAttAef->enSound, pstSndAttAef->u32AefId, &pstSndAttAef->u32AefProcAddr);
        break;
    }

    case CMD_AO_SND_DETACHAEF:
    {
        AO_SND_AttAef_Param_S_PTR pstSndAttAef = (AO_SND_AttAef_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN(pstSndAttAef->enSound);
        Ret = AO_SND_DetachAef(pstSndAttAef->enSound, pstSndAttAef->u32AefId);
        break;
    }

    case CMD_AO_SND_SETAEFBYPASS:
    {
        AO_SND_AefBypass_Param_S_PTR pstAefBypass = (AO_SND_AefBypass_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN(pstAefBypass->enSound);
        Ret = AO_SND_SetAefBypass(pstAefBypass->enSound, pstAefBypass->enOutPort, pstAefBypass->bBypass);
        break;
    }

    case CMD_AO_SND_GETAEFBYPASS:
    {
        AO_SND_AefBypass_Param_S_PTR pstAefBypass = (AO_SND_AefBypass_Param_S_PTR)arg;
        CHECK_AO_SNDCARD_OPEN(pstAefBypass->enSound);
        Ret = AO_SND_GetAefBypass(pstAefBypass->enSound, pstAefBypass->enOutPort, &pstAefBypass->bBypass);
        break;
    }
    case CMD_AO_SND_GETXRUNCOUNT:
    {
        AO_SND_Get_Xrun_Param_S_PTR pstXrunStatus = (AO_SND_Get_Xrun_Param_S_PTR)arg;

        CHECK_AO_SNDCARD_OPEN(pstXrunStatus->enSound);
        Ret = AO_Snd_GetXRunCount(pstXrunStatus->enSound, &pstXrunStatus->u32Count);

        break;
    }
    case CMD_AO_SND_SETRENDERINGRATE:
    {
        mt_u32 rate = *(mt_u32 *)arg;
        mt_u32 val = 0;
        
        if(rate != 0) {            
            val = reg_sym_linux_get_clk1_reg_div_bit();
            reg_sym_linux_clk1_reg_div_bit(val * rate);
            
            val = reg_sym_linux_get_clk1_reg_div_bit2();
            reg_sym_linux_clk1_reg_div_bit2(val * rate);
        } else {
            Ret = MT_FAILURE;
        }

        break;
    }
//#ifdef MT_SND_CAST_SUPPORT//minnan remove
    case CMD_AO_CAST_GETDEFATTR:
    {
        MT_UNF_SND_CAST_ATTR_S *pstDefAttr = (MT_UNF_SND_CAST_ATTR_S *)arg;
        Ret = AO_Cast_GetDefAttr(pstDefAttr);
        break;
    }

    case CMD_AO_CAST_CREATE:
    {
        AO_Cast_Create_Param_S_PTR  pstCastAttr = (AO_Cast_Create_Param_S_PTR)arg;

        //MT_ERR_AO("CMD_AO_CAST_CREATE\n");
        CHECK_AO_SNDCARD_OPEN( pstCastAttr->enSound);

        if (MT_SUCCESS == AO_Cast_AllocHandle(&hHandle, file, &pstCastAttr->stCastAttr))
        {
            Ret = AO_Cast_Create(pstCastAttr->enSound, &pstCastAttr->stCastAttr, &s_stAoDrv.astCastEntity[hHandle
                                                                                                          & AO_CAST_CHNID_MASK].stRbfMmz,
                                 hHandle);
            if (MT_SUCCESS != Ret)
            {
                AO_Cast_FreeHandle(hHandle);
                break;
            }

            AO_Cast_SaveSuspendAttr(pstCastAttr->enSound, hHandle, &pstCastAttr->stCastAttr);
            pstCastAttr->u32ReqSize = s_stAoDrv.astCastEntity[hHandle & AO_CAST_CHNID_MASK].u32ReqSize;
            pstCastAttr->hCast = hHandle;
        }
        break;
    }
    case CMD_AO_CAST_DESTROY:
    {
        mt_handle  hCast = *(mt_handle *)arg;
        //MT_ERR_AO("CMD_AO_CAST_DESTORY\n");

        CHECK_AO_CAST_OPEN(hCast);
        Ret = AO_Cast_Destory(hCast);
        if (MT_SUCCESS != Ret)
        {
            break;
        }
        AO_Cast_FreeHandle(hCast);
        break;
    }

    case CMD_AO_CAST_SETINFO:
    {
        AO_Cast_Info_Param_S_PTR pstInfo = (AO_Cast_Info_Param_S_PTR)arg;
        CHECK_AO_CAST_OPEN(pstInfo->hCast);

        //MT_ERR_AO("CMD_AO_CAST_SETINFO  CastID=0x%x, u32UserVirtAddr=0x%x\n",pstInfo->hCast, pstInfo->u32UserVirtAddr);
        Ret = AO_Cast_SetInfo(pstInfo->hCast, pstInfo->u32UserVirtAddr);
        break;
    }
    case CMD_AO_CAST_GETINFO:
    {
        AO_Cast_Info_Param_S_PTR pstInfo = (AO_Cast_Info_Param_S_PTR)arg;
        //MT_ERR_AO("pstInfo->hCast=0x%x \n", pstInfo->hCast);
        CHECK_AO_CAST_OPEN(pstInfo->hCast);
        Ret = AO_Cast_GetInfo(pstInfo->hCast, pstInfo);
        //MT_ERR_AO("CMD_AO_CAST_GETINFO  CastID=0x%x, u32UserVirtAddr=0x%x Ret=0x%x\n",pstInfo->hCast, pstInfo->u32UserVirtAddr, Ret);
        break;
    }

    case CMD_AO_CAST_SETENABLE:
    {
        AO_Cast_Enable_Param_S_PTR pstEnable = (AO_Cast_Enable_Param_S_PTR)arg;
        CHECK_AO_CAST_OPEN(pstEnable->hCast);
        //MT_ERR_AO("CMD_AO_CAST_SETENABLE  CastID=0x%x, bCastEnable=0x%x\n",pstEnable->hCast, pstEnable->bCastEnable);
        Ret = AO_Cast_SetEnable(pstEnable->hCast, pstEnable->bCastEnable);
        break;
    }

    case CMD_AO_CAST_GETENABLE:
    {
        AO_Cast_Enable_Param_S_PTR pstEnable = (AO_Cast_Enable_Param_S_PTR)arg;
        CHECK_AO_CAST_OPEN(pstEnable->hCast);
        //MT_ERR_AO("CMD_AO_CAST_GETENABLE CastID=0x%x, bCastEnable=0x%x\n",pstEnable->hCast, pstEnable->bCastEnable);
        Ret = AO_Cast_GetEnable(pstEnable->hCast, &pstEnable->bCastEnable);
        break;
    }

    case CMD_AO_CAST_ACQUIREFRAME:
    {
        AO_Cast_Data_Param_S_PTR pstCastData = (AO_Cast_Data_Param_S_PTR)arg;
        CHECK_AO_CAST_OPEN(pstCastData->hCast);

        Ret = AO_Cast_ReadData(pstCastData->hCast, pstCastData);
        break;
    }
    case CMD_AO_CAST_RELEASEFRAME:
    {
        AO_Cast_Data_Param_S_PTR pstCastData = (AO_Cast_Data_Param_S_PTR)arg;
        CHECK_AO_CAST_OPEN(pstCastData->hCast);
        //MT_ERR_AO("CMD_AO_CAST_RELEASEFRAME  CastID=0x%x\n",pstCastData->hCast);
        Ret = AO_Cast_ReleseData(pstCastData->hCast, pstCastData);

        break;
    }
    case CMD_AO_CAST_SETABSGAIN:
    {
        AO_Cast_AbsGain_Param_S_PTR pstAbsGain = (AO_Cast_AbsGain_Param_S_PTR)arg;
        CHECK_AO_CAST_OPEN(pstAbsGain->hCast);
        Ret = AO_Cast_SetAbsGain(pstAbsGain->hCast, pstAbsGain->stCastAbsGain);
        break;
    }
    case CMD_AO_CAST_GETABSGAIN:
    {
        AO_Cast_AbsGain_Param_S_PTR pstAbsGain = (AO_Cast_AbsGain_Param_S_PTR)arg;
        CHECK_AO_CAST_OPEN(pstAbsGain->hCast);
        Ret = AO_Cast_GetAbsGain(pstAbsGain->hCast, &pstAbsGain->stCastAbsGain);
        break;
    }
    case CMD_AO_CAST_SETMUTE:
    {
        AO_Cast_Mute_Param_S_PTR pstMute = (AO_Cast_Mute_Param_S_PTR)arg;
        CHECK_AO_CAST_OPEN(pstMute->hCast);
        Ret = AO_Cast_SetMute(pstMute->hCast, pstMute->bMute);
        break;
    }
    case CMD_AO_CAST_GETMUTE:
    {
        AO_Cast_Mute_Param_S_PTR pstMute = (AO_Cast_Mute_Param_S_PTR)arg;
        CHECK_AO_CAST_OPEN(pstMute->hCast);
        Ret = AO_Cast_GetMute(pstMute->hCast, &pstMute->bMute);
        break;
    }

//#endif

    case CMD_AO_SND_ATTACHTRACK:
        break;
    case CMD_AO_SND_DETACHTRACK:
        break;

        //Track CMD TYPE (call hal_aoe)
    case CMD_AO_TRACK_GETDEFATTR:
    {
        MT_UNF_AUDIOTRACK_ATTR_S *pstDefAttr = (MT_UNF_AUDIOTRACK_ATTR_S *)arg;
        Ret = AO_Track_GetDefAttr(pstDefAttr);
        break;
    }
    case CMD_AO_TRACK_SETATTR:
    {
        AO_Track_Attr_Param_S_PTR pstTrackAttr = (AO_Track_Attr_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstTrackAttr->hTrack);
        Ret = AO_Track_PreSetAttr(pstTrackAttr->hTrack, &pstTrackAttr->stAttr);
        break;
    }
    case CMD_AO_TRACK_GETATTR:
    {
        AO_Track_Attr_Param_S_PTR pstTrackAttr = (AO_Track_Attr_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstTrackAttr->hTrack);
        Ret = AO_Track_GetAttr(pstTrackAttr->hTrack, &pstTrackAttr->stAttr);
        break;
    }

    case CMD_AO_TRACK_CREATE:
    {
        AO_Track_Create_Param_S_PTR pstTrack = (AO_Track_Create_Param_S_PTR)arg;
        if (MT_SUCCESS == AO_Track_AllocHandle(&hHandle, file, pstTrack))
        {
            Ret = AO_Track_PreCreate(pstTrack->enSound, &pstTrack->stAttr, pstTrack->bAlsaTrack,
                                  &pstTrack->stBuf,
                                  hHandle);
            if (MT_SUCCESS != Ret)
            {
                AO_Track_FreeHandle(hHandle);
                break;
            }
            AO_TRACK_SaveSuspendAttr(hHandle, pstTrack);

            pstTrack->hTrack = hHandle;
        }
        else
        {
            Ret = MT_FAILURE;
        }
        break;
    }

    case CMD_AO_TRACK_DESTROY:
    {
        mt_handle hTrack = *(mt_handle *)arg;
        CHECK_AO_TRACK_OPEN(hTrack);
        Ret = AO_Track_Destory( hTrack );
        if (MT_SUCCESS != Ret)
        {
            break;
        }

        AO_Track_FreeHandle(hTrack);

        break;
    }
    case CMD_AO_TRACK_START:
    {
        mt_handle hTrack = *(mt_handle *)arg;
        CHECK_AO_TRACK_OPEN(hTrack);
        Ret = AO_Track_Start(hTrack);
        break;
    }

    case CMD_AO_TRACK_STOP:
    {
        mt_handle hTrack = *(mt_handle *)arg;
        CHECK_AO_TRACK_OPEN(hTrack);
        Ret = AO_Track_Stop(hTrack);
        break;
    }

    case CMD_AO_TRACK_PAUSE:
    {
        mt_handle hTrack = *(mt_handle *)arg;
        CHECK_AO_TRACK_OPEN(hTrack);
        Ret = AO_Track_Pause(hTrack);
        break;
    }

    case CMD_AO_TRACK_FLUSH:
    {
        mt_handle hTrack = *(mt_handle *)arg;
        CHECK_AO_TRACK_OPEN(hTrack);
        Ret = AO_Track_Flush(hTrack);
        break;
    }

    case CMD_AO_TRACK_SENDDATA:
    {
        AO_Track_SendData_Param_S_PTR pstData = (AO_Track_SendData_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstData->hTrack);
        Ret = AO_Track_SendData(pstData->hTrack, &pstData->stAOFrame);
        if (MT_SUCCESS != Ret)
        {
            //to do
        }

        break;
    }

    case CMD_AO_TRACK_SETWEITHT:
    {
        AO_Track_Weight_Param_S_PTR pstWeight = (AO_Track_Weight_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstWeight->hTrack);
        Ret = AO_Track_SetWeight(pstWeight->hTrack, pstWeight->stTrackGain);
        break;
    }

    case CMD_AO_TRACK_GETWEITHT:
    {
        AO_Track_Weight_Param_S_PTR pstWeight = (AO_Track_Weight_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstWeight->hTrack);
        Ret = AO_Track_GetWeight(pstWeight->hTrack, &pstWeight->stTrackGain);
        break;
    }

    case CMD_AO_TRACK_SETABSGAIN:
    {
        AO_Track_AbsGain_Param_S_PTR pstAbsGain = (AO_Track_AbsGain_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstAbsGain->hTrack);
        Ret = AO_Track_SetAbsGain(pstAbsGain->hTrack, pstAbsGain->stTrackAbsGain);
        break;
    }
    case CMD_AO_TRACK_GETABSGAIN:
    {
        AO_Track_AbsGain_Param_S_PTR pstAbsGain = (AO_Track_AbsGain_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstAbsGain->hTrack);
        Ret = AO_Track_GetAbsGain(pstAbsGain->hTrack, &pstAbsGain->stTrackAbsGain);
      break;
    }
    case CMD_AO_TRACK_SETMUTE:
    {
        AO_Track_Mute_Param_S_PTR pstMute = (AO_Track_Mute_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstMute->hTrack);
        Ret = AO_Track_SetMute(pstMute->hTrack, pstMute->bMute);
        break;
    }
    case CMD_AO_TRACK_GETMUTE:
    {
        AO_Track_Mute_Param_S_PTR pstMute = (AO_Track_Mute_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstMute->hTrack);
        Ret = AO_Track_GetMute(pstMute->hTrack, &pstMute->bMute);
        break;
    }

	case CMD_AO_TRACK_SETCHANNELMODE:
	{
        AO_Track_ChannelMode_Param_S_PTR pstChannelMode = (AO_Track_ChannelMode_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstChannelMode->hTrack);
        Ret = AO_Track_SetChannelMode(pstChannelMode->hTrack, pstChannelMode->enMode);
		break;
	}
	case CMD_AO_TRACK_GETCHANNELMODE:
	{
        AO_Track_ChannelMode_Param_S_PTR pstChannelMode = (AO_Track_ChannelMode_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstChannelMode->hTrack);
        Ret = AO_Track_GetChannelMode(pstChannelMode->hTrack, &pstChannelMode->enMode);
		break;
	}

    case CMD_AO_TRACK_SETSPEEDADJUST:
    {
        AO_Track_SpeedAdjust_Param_S_PTR pstSpeed = (AO_Track_SpeedAdjust_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstSpeed->hTrack);
        Ret = AO_Track_SetSpeedAdjust(pstSpeed->hTrack, pstSpeed->enType, pstSpeed->s32Speed);
        break;
    }

    case CMD_AO_TRACK_GETDELAYMS:
    {
        AO_Track_DelayMs_Param_S_PTR pstDelayMs = (AO_Track_DelayMs_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstDelayMs->hTrack);
        Ret = AO_Track_GetDelayMs(pstDelayMs->hTrack, &pstDelayMs->u32DelayMs);
        break;
    }

    case CMD_AO_TRACK_ISBUFEMPTY:
    {
        AO_Track_BufEmpty_Param_S_PTR pstBufEmpty = (AO_Track_BufEmpty_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstBufEmpty->hTrack);
        Ret = AO_Track_IsBufEmpty(pstBufEmpty->hTrack, &pstBufEmpty->bEmpty);
        break;
    }

    case CMD_AO_TRACK_SETEOSFLAG:
    {
        AO_Track_EosFlag_Param_S_PTR pstEosFlag = (AO_Track_EosFlag_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstEosFlag->hTrack);
        Ret = AO_Track_SetEosFlag(pstEosFlag->hTrack, pstEosFlag->bEosFlag);
        break;
    }

    case CMD_AO_TRACK_ATTACHAI:
    {
        AO_Track_AttAi_Param_S_PTR pstTrackAttAi = (AO_Track_AttAi_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstTrackAttAi->hTrack);
        Ret = AO_Track_AttachAi(pstTrackAttAi->hTrack, pstTrackAttAi->hAi);
        break;
    }

    case CMD_AO_TRACK_DETACHAI:
    {
        AO_Track_AttAi_Param_S_PTR pstTrackDetAi = (AO_Track_AttAi_Param_S_PTR)arg;
        CHECK_AO_TRACK_OPEN(pstTrackDetAi->hTrack);
        Ret = AO_Track_DetachAi(pstTrackDetAi->hTrack, pstTrackDetAi->hAi);
        break;
    }

    default:
        Ret = MT_FAILURE;
        {
            MT_WARN_AO("unknown cmd: 0x%x\n", cmd);
        }
        break;
    }

    return Ret;
}

long AO_DRV_Ioctl(struct file *file, mt_u32 cmd, unsigned long arg)
{
    long s32Ret = MT_SUCCESS;

    s32Ret = down_interruptible(&g_AoMutex);

    //cmd process
    s32Ret = (long)mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, AO_ProcessCmd);

    up(&g_AoMutex);

    return s32Ret;
}

mt_s32 AO_DRV_Open(struct inode *inode, struct file  *filp)
{
    mt_s32 s32Ret;
    mt_u32 cnt;
    DRV_AO_STATE_S *pAOState = MT_NULL;
    MT_S32 mt_idx = iminor(inode);
    AO_PRIV_DATA_S *ao_priv_data = get_mt_priv(mt_idx);

    if (!IS_ERR_OR_NULL(ao_priv_data->audoutclk))
        clk_prepare_enable(ao_priv_data->audoutclk);
    if (!IS_ERR_OR_NULL(ao_priv_data->audoutaxiclk))
        clk_prepare_enable(ao_priv_data->audoutaxiclk);

    if(!filp)
    {
        MT_FATAL_AO("file handle is null.\n");
        return MT_FAILURE;
    }

    s32Ret = down_interruptible(&g_AoMutex);

    pAOState = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(DRV_AO_STATE_S), GFP_KERNEL);
    if (!pAOState)
    {
        MT_FATAL_AO("malloc pAOState failed.\n");
        up(&g_AoMutex);
        return MT_FAILURE;
    }
    for(cnt = 0; cnt < AO_MAX_TOTAL_SND_NUM; cnt++)
	{
        atomic_set(&(pAOState->atmUserOpenCnt[cnt]), 0);
        pAOState->u32FileId[cnt] = AO_SND_FILE_NOUSE_FLAG;
	}

    if (atomic_inc_return(&s_stAoDrv.atmOpenCnt) == 1)
    {
        /* Init device */
        if (MT_SUCCESS != AO_OpenDev())
        {
            MT_FATAL_AO("AO_OpenDev err!\n" );
            goto err;
        }
    }

    filp->private_data = pAOState;

    up(&g_AoMutex);
    return MT_SUCCESS;
err:
    AUTIL_AO_FREE(MT_ID_AO, pAOState);
    atomic_dec(&s_stAoDrv.atmOpenCnt);
    up(&g_AoMutex);
    return MT_FAILURE;
}

mt_s32 AO_DRV_Release(struct inode *inode, struct file  *filp)
{
    mt_u32 i;
    mt_u32 j;
    MT_S32 mt_idx = iminor(inode);
    AO_PRIV_DATA_S *ao_priv_data = get_mt_priv(mt_idx);

    long s32Ret = MT_SUCCESS;
    DRV_AO_STATE_S *pAOState = filp->private_data;

    s32Ret = down_interruptible(&g_AoMutex);

    /* Not the last close, only close the track & snd match with the 'filp' */
    if (atomic_dec_return(&s_stAoDrv.atmOpenCnt) != 0)
    {
#ifdef MT_SND_CAST_SUPPORT
        /* Free all Cast */
        for (i = 0; i < AO_MAX_CAST_NUM; i++)
        {
            if (s_stAoDrv.astCastEntity[i].u32File == ((long)filp))
            {
                if (atomic_read(&s_stAoDrv.astCastEntity[i].atmUseCnt))
                {
                    if (MT_SUCCESS != AO_Cast_Destory(i))
                    {
                        atomic_inc(&s_stAoDrv.atmOpenCnt);
                        up(&g_AoMutex);
                        return MT_FAILURE;
                    }
                    AO_Cast_FreeHandle(i);
                }
            }
        }
#endif
        /* Free all track */
        for (i = 0; i < AO_MAX_TOTAL_TRACK_NUM; i++)
        {
            if (s_stAoDrv.astTrackEntity[i].u32File == ((long)filp))
            {
                if (atomic_read(&s_stAoDrv.astTrackEntity[i].atmUseCnt))
                {
                    if (MT_SUCCESS != AO_Track_Destory(i))
                    {
                        atomic_inc(&s_stAoDrv.atmOpenCnt);
                        up(&g_AoMutex);
                        return MT_FAILURE;
                    }

                    AO_Track_FreeHandle(i);
                }
            }
        }

        /* Free all snd */
        for (i = 0; i < AO_MAX_TOTAL_SND_NUM; i++)
        {
            for(j = 0; j < SND_MAX_OPEN_NUM; j++)
            {
                if (s_stAoDrv.astSndEntity[i].u32File[j] == ((long)filp))
                {
                        mt_u32 u32UserOpenCnt = 0;

                        if (s_stAoDrv.astSndEntity[i].pCard)
                        {
                           u32UserOpenCnt = atomic_read(&pAOState->atmUserOpenCnt[i]);
                            if(atomic_sub_and_test(u32UserOpenCnt, &s_stAoDrv.astSndEntity[i].atmUseTotalCnt))
                            {
                                if (MT_SUCCESS != AO_SND_Close(i, MT_FALSE))
                                {
                                    atomic_inc(&s_stAoDrv.astSndEntity[i].atmUseTotalCnt);
                                    atomic_inc(&s_stAoDrv.atmOpenCnt);
                                    up(&g_AoMutex);
                                    return MT_FAILURE;
                                }
                            }

                            AO_Snd_FreeHandle(i, (struct file *)(s_stAoDrv.astSndEntity[i].u32File[j]));
                        }
                }
           }

        }
    }
    /* Last close */
    else
    {
        AO_CloseDev();
    }

    AUTIL_AO_FREE(MT_ID_AO, pAOState);
    up(&g_AoMutex);

    if (!IS_ERR_OR_NULL(ao_priv_data->audoutclk))
        clk_disable_unprepare(ao_priv_data->audoutclk);
    if (!IS_ERR_OR_NULL(ao_priv_data->audoutaxiclk))
        clk_disable_unprepare(ao_priv_data->audoutaxiclk);

    return MT_SUCCESS;
}

/* drv open  kernel intf */
mt_s32 AO_DRV_Kopen(struct file  *file)
{
    mt_s32 s32Ret;
    mt_u32 cnt;
    DRV_AO_STATE_S *pAOState = MT_NULL;

    if(!file)
    {
        MT_FATAL_AO("file handle is null.\n");
        return MT_FAILURE;
    }

    s32Ret = down_interruptible(&g_AoMutex);

    pAOState = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(DRV_AO_STATE_S), GFP_KERNEL);
    if (!pAOState)
    {
        MT_FATAL_AO("malloc pAOState failed.\n");
        up(&g_AoMutex);
        return MT_FAILURE;
    }
    for(cnt = 0; cnt < AO_MAX_TOTAL_SND_NUM; cnt++)
	{
        atomic_set(&(pAOState->atmUserOpenCnt[cnt]), 0);
        pAOState->u32FileId[cnt] = AO_SND_FILE_NOUSE_FLAG;
    }

    if (atomic_inc_return(&s_stAoDrv.atmOpenCnt) == 1)
    {
       /* Init device */
        if (MT_SUCCESS != AO_OpenDev())
        {
            MT_FATAL_AO("AO_OpenDev err!\n" );
            goto err;
        }
    }

    file->private_data = pAOState;

    up(&g_AoMutex);
    return MT_SUCCESS;
err:
    AUTIL_AO_FREE(MT_ID_AO, pAOState);
    atomic_dec(&s_stAoDrv.atmOpenCnt);
    up(&g_AoMutex);
    return MT_FAILURE;
}
/*drv close kernel intf */
mt_s32 AO_DRV_Krelease(struct file  *file)
{
    mt_u32 i;
    mt_u32 j;
    long s32Ret = MT_SUCCESS;
    DRV_AO_STATE_S *pAOState = file->private_data;

    s32Ret = down_interruptible(&g_AoMutex);

    /* Not the last close, only close the track & snd match with the 'filp' */
    if (atomic_dec_return(&s_stAoDrv.atmOpenCnt) != 0)
    {
        /* Free all track */
        for (i = 0; i < AO_MAX_TOTAL_TRACK_NUM; i++)
        {
            if (s_stAoDrv.astTrackEntity[i].u32File == ((long)file))
            {
                if (atomic_read(&s_stAoDrv.astTrackEntity[i].atmUseCnt))
                {
                    if (MT_SUCCESS != AO_Track_Destory(i))
                    {
                        atomic_inc(&s_stAoDrv.atmOpenCnt);
                        up(&g_AoMutex);
                        return MT_FAILURE;
                    }

                    AO_Track_FreeHandle(i);
                }
            }
        }

        /* Free all snd */
        for (i = 0; i < AO_MAX_TOTAL_SND_NUM; i++)
        {
            for(j = 0; j < SND_MAX_OPEN_NUM; j++)
            {
                if (s_stAoDrv.astSndEntity[i].u32File[j] == ((long)file))
                {
                        mt_u32 u32UserOpenCnt = 0;
                        if (s_stAoDrv.astSndEntity[i].pCard)
                        {
                           u32UserOpenCnt = atomic_read(&pAOState->atmUserOpenCnt[i]);
                            if(atomic_sub_and_test(u32UserOpenCnt, &s_stAoDrv.astSndEntity[i].atmUseTotalCnt))
                            {
                                if (MT_SUCCESS != AO_SND_Close(i, MT_FALSE))
                                {
                                    atomic_inc(&s_stAoDrv.astSndEntity[i].atmUseTotalCnt);
                                    atomic_inc(&s_stAoDrv.atmOpenCnt);
                                    up(&g_AoMutex);
                                    return MT_FAILURE;
                                }
                            }

                            AO_Snd_FreeHandle(i, (struct file *)(s_stAoDrv.astSndEntity[i].u32File[j]));
                        }
                }
           }

        }
    }
    /* Last close */
    else
    {
        AO_CloseDev();
    }

    AUTIL_AO_FREE(MT_ID_AO, pAOState);
    up(&g_AoMutex);
    return MT_SUCCESS;
}
#if defined (MT_ALSA_I2S_ONLY_SUPPORT)|| defined (MT_ALSA_HDMI_ONLY_SUPPORT)
 mt_s32 AO_DRV_Krelease(struct file  *file);
mt_s32 AOSetProcStatistics(AIAO_IsrFunc *pFunc)//only for alsa use
{
    HAL_AIAO_P_SetTxI2SDfAttr(AIAO_PORT_TX0,pFunc);  //pIsrFunc is the same for all ports
    return MT_SUCCESS;
}
 mt_s32 AOGetProcStatistics(AIAO_IsrFunc **pFunc)//only for alsa use
 {
     AIAO_PORT_USER_CFG_S pAttr;
     HAL_AIAO_P_GetTxI2SDfAttr(AIAO_PORT_TX0,&pAttr);  //pIsrFunc is the same for all ports
     *pFunc = pAttr.pIsrFunc;
     return MT_SUCCESS;
 }
mt_s32 AOGetEnport(MT_UNF_SND_E enSound,AIAO_PORT_ID_E *enPort,SND_OUTPUT_TYPE_E enOutType)  //jiaxi check
{
     mt_handle        hSndOp;
     SND_OP_STATE_S   *state;
     AIAO_PORT_ID_E   enAOPort;
     SND_CARD_STATE_S *pCard;
     pCard = SND_CARD_GetCard(enSound);
     if(pCard!= MT_NULL)
     {
        if(enOutType == SND_OUTPUT_TYPE_HDMI)
           hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_HDMI);
        else
           hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_I2S);
     }
     else
     {
        goto _GET_ERR;
     }
     state = (SND_OP_STATE_S *)hSndOp;
     if(state != MT_NULL)
     {
        enAOPort = state->enPortID[state->ActiveId];
     }
     else
     {
        goto _GET_ERR;
     }
     *enPort = enAOPort;
    return MT_SUCCESS;
_GET_ERR:
        MT_FATAL_AO("Get Enpot Error\n");
        return MT_FAILURE;
}
mt_s32 AOGetHandel(MT_UNF_SND_E enSound,mt_handle *hSndOp,SND_OUTPUT_TYPE_E enOutType)  //jiaxi check
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);
     if(pCard!= MT_NULL)
     {
        if(enOutType == SND_OUTPUT_TYPE_HDMI)//MT_ALSA_HDMI_ONLY_SUPPORT
           *hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_HDMI);
        else
           *hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_I2S);
     }
     else
     {
        goto _GET_ERR;
     }
    if(hSndOp == MT_NULL)
    {
        goto _GET_ERR;
    }
    return MT_SUCCESS;
_GET_ERR:
    MT_FATAL_AO("Get AOGetHandel Error\n");
    return MT_FAILURE;
}
mt_s32 Alsa_AO_OpenDev(struct file  *file,void *p)
{
   if (MT_SUCCESS !=AO_DRV_Kopen(file))
   {
       MT_FATAL_AO("AO_DRV_Kopen err!\n" );
       goto err;
   }
   if (MT_SUCCESS !=AO_Snd_Kopen((AO_SND_Open_Param_S*)p, file))
   {
       AO_DRV_Krelease(file);
       MT_FATAL_AO("\n AO_Snd_Kopen err\n");
       goto err;
   }
   return MT_SUCCESS;
err:
    return MT_FAILURE;
}
mt_s32 Alsa_AO_CloseDev(struct file  *file,MT_UNF_SND_E snd_idx)
{
    if (MT_SUCCESS !=AO_Snd_Kclose(snd_idx,file))
    {
        MT_FATAL_AO("AO_Snd_Kclose rr!\n" );
        goto err;
    }
    if (MT_SUCCESS !=AO_DRV_Krelease(file))
    {
        MT_FATAL_AO("AO_DRV_Krelease err!\n" );
        goto err;
    }
return MT_SUCCESS;
err:
    return MT_FAILURE;
}
#endif

mt_s32 AO_DRV_RegisterProc(AO_REGISTER_PARAM_S * pstParam)
{
    mt_u32 i;

    /* Check parameters */
    if (MT_NULL == pstParam)
    {
        return MT_FAILURE;
    }

    s_stAoDrv.pstProcParam = pstParam;

    /* Create proc when use, if MCE open snd , reg proc here*/
	for (i = 0; i < AO_MAX_TOTAL_SND_NUM; i++)
    {
        if (s_stAoDrv.astSndEntity[i].pCard)
        {
            AO_RegProc(i);
        }
    }
    return MT_SUCCESS;
}

mt_void AO_DRV_UnregisterProc(mt_void)
{

    /* Clear param */
    s_stAoDrv.pstProcParam = MT_NULL;
    return;
}

#if defined (MT_SND_DRV_SUSPEND_SUPPORT)
static mt_s32 AO_TRACK_GetSettings(mt_handle hTrack, SND_TRACK_SETTINGS_S* pstSndSettings)
{
    SND_CARD_STATE_S *pCard;

    hTrack &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(hTrack);
    if(pCard)
    {
        return TRACK_GetSetting(pCard, hTrack, pstSndSettings);
    }
    else
    {
        MT_ERR_AO("Track(%d) don't attach card!\n",hTrack);
        return MT_FAILURE;
    }
}

static mt_s32 AO_TRACK_RestoreSettings(mt_handle hTrack, SND_TRACK_SETTINGS_S* pstSndSettings)
{
    SND_CARD_STATE_S *pCard;

    hTrack &= AO_TRACK_CHNID_MASK;
    pCard = TRACK_CARD_GetCard(hTrack);
    if(pCard)
    {
        return TRACK_RestoreSetting(pCard, hTrack, pstSndSettings);
    }
    else
    {
        MT_ERR_AO("Track(%d) don't attach card!\n",hTrack);
        return MT_FAILURE;
    }
}

static mt_s32 AO_SND_GetSettings(MT_UNF_SND_E enSound, SND_CARD_SETTINGS_S* pstSndSettings)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_NULL_PTR(pstSndSettings);

    pstSndSettings->bAllTrackMute = pCard->bAllTrackMute;
    if(MT_SUCCESS != SND_GetOpSetting(pCard, pstSndSettings))
    {
        return MT_FAILURE;
    }

    if(MT_SUCCESS != AEF_GetSetting(pCard, pstSndSettings))
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 AO_SND_RestoreSettings(MT_UNF_SND_E enSound, SND_CARD_SETTINGS_S* pstSndSettings)
{
    SND_CARD_STATE_S *pCard = SND_CARD_GetCard(enSound);

    CHECK_AO_NULL_PTR(pCard);
    CHECK_AO_NULL_PTR(pstSndSettings);
	pCard->bAllTrackMute = pstSndSettings->bAllTrackMute; //All track mute restore in Track restore.

    if(MT_SUCCESS != SND_RestoreOpSetting(pCard, pstSndSettings))
    {
        return MT_FAILURE;
    }

    if(MT_SUCCESS != AEF_RestoreSetting(pCard, pstSndSettings))
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

#endif

mt_s32 MT_DRV_AO_Init(mt_void)
{
    return AO_DRV_Init();
}

mt_void MT_DRV_AO_DeInit(mt_void)
{
    AO_DRV_Exit();
}

mt_s32 MT_DRV_AO_SND_Init(struct file  *pfile)
{
    struct file  *pstfilp;

    if(MT_NULL == pfile)
	{
        pstfilp = &g_filp;
    }
	else
    {
        pstfilp = pfile;
    }

	MT_INFO_AO("\nMT_DRV_AO_SND_Init file=%p\n", pfile);

    return AO_DRV_Open(NULL, pstfilp);
}

mt_s32 MT_DRV_AO_SND_DeInit(struct file  *pfile)
{
    struct file  *pstfilp;

    if(MT_NULL == pfile)
    {
        pstfilp = &g_filp;
    }
    else
    {
        pstfilp = pfile;
    }
    MT_INFO_AO("\nMT_DRV_AO_SND_DeInit file=%p\n", pfile);

    return AO_DRV_Release(NULL, pstfilp);
}

mt_s32 MT_DRV_AO_SND_GetDefaultOpenAttr(MT_UNF_SND_E enSound, MT_UNF_SND_ATTR_S *pstAttr)
{
    mt_s32 Ret;
	AO_SND_OpenDefault_Param_S stSndDefaultAttr;
    CHECK_AO_SNDCARD( enSound );

	stSndDefaultAttr.enSound = enSound;
	Ret = AOGetSndDefOpenAttr(&stSndDefaultAttr);
	if(MT_SUCCESS != Ret)
		return Ret;

	memcpy(pstAttr, &stSndDefaultAttr.stAttr, sizeof(MT_UNF_SND_ATTR_S));
	return Ret;
}

#if 0
mt_s32 MT_DRV_AO_SND_Open(MT_UNF_SND_E enSound, MT_UNF_SND_ATTR_S *pstAttr, struct file  *pfile)
{
    mt_s32 Ret;
    DRV_AO_STATE_S *pAOState;
    struct file  *pstfile;

    if(MT_NULL == pfile)
    {
       pAOState = g_filp.private_data;
       pstfile = &g_filp;
    }
    else
    {
       pAOState = pfile->private_data;
       pstfile = pfile;
    }

    CHECK_AO_SNDCARD( enSound );

	MT_INFO_AO("\nMT_DRV_AO_SND_Open file=%p\n", pfile);

    Ret = AO_Snd_AllocHandle(enSound, pstfile);
    if (MT_SUCCESS == Ret)
    {
        if (0 == atomic_read(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt))
        {
                Ret = AO_SND_Open(enSound, pstAttr, NULL, MT_FALSE);
                if (MT_SUCCESS != Ret)
                {
                    AO_Snd_FreeHandle(enSound, pstfile);
                    return Ret;
                }
        }
    }

    atomic_inc(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt);
    atomic_inc(&pAOState->atmUserOpenCnt[enSound]);

    return Ret;
}

mt_s32 MT_DRV_AO_SND_Close(MT_UNF_SND_E enSound, struct file  *pfile)
{
    mt_s32 Ret;
    DRV_AO_STATE_S *pAOState;
    struct file  *pstfilp;

    if(MT_NULL == pfile)
    {
       pAOState = g_filp.private_data;
       pstfilp = &g_filp;
    }
    else
    {
       pAOState = pfile->private_data;
       pstfilp = pfile;
    }

    CHECK_AO_SNDCARD_OPEN( enSound );

	MT_INFO_AO("\nMT_DRV_AO_SND_Close file=%p\n", pfile);

    if(atomic_dec_and_test(&pAOState->atmUserOpenCnt[enSound]))
    {
            if (atomic_dec_and_test(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt))
            {
                Ret = AO_SND_Close( enSound, MT_FALSE );
                if (MT_SUCCESS != Ret)
                {
                    atomic_inc(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt);
                    atomic_inc(&pAOState->atmUserOpenCnt[enSound]);
                    return Ret;
                }

                AO_Snd_FreeHandle(enSound, pstfilp);
            }
    }
    else
    {
        atomic_dec(&s_stAoDrv.astSndEntity[enSound].atmUseTotalCnt);
    }

    return MT_SUCCESS;
}
#endif

 mt_s32 MT_DRV_AO_Snd_GetXRunCount(MT_UNF_SND_E enSound, mt_u32 *pu32Count)
{
    CHECK_AO_SNDCARD_OPEN(enSound);
    return AO_Snd_GetXRunCount(enSound, pu32Count);

}
mt_s32 MT_DRV_AO_SND_SetVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_GAIN_ATTR_S stGain)
{
    CHECK_AO_SNDCARD_OPEN( enSound );
    return AO_SND_SetVolume(enSound, enOutPort, stGain);

}

mt_s32 MT_DRV_AO_SND_GetVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_GAIN_ATTR_S *pstGain)
{
    CHECK_AO_SNDCARD_OPEN( enSound );
    return AO_SND_GetVolume(enSound, enOutPort, pstGain);
}

mt_s32 MT_DRV_AO_Track_GetDefaultOpenAttr(MT_UNF_SND_TRACK_TYPE_E enTrackType, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr)
{
    pstAttr->enTrackType = enTrackType;

    return AO_Track_GetDefAttr(pstAttr);
}

mt_s32 MT_DRV_AO_Track_Create(MT_UNF_SND_E enSound, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr, MT_BOOL bAlsaTrack, struct file  *pfile, mt_handle *phTrack)
{
    mt_s32 Ret = MT_SUCCESS;
    mt_handle hHandle = MT_INVALID_HANDLE;
    struct file  *pstfilp;

    if(MT_NULL == pfile)
	{
       pstfilp = &g_filp;
    }
    else
    {
       pstfilp = pfile;
    }
	MT_INFO_AO("\nMT_DRV_AO_Track_Create bAlsaTrack=%d file=%p\n", bAlsaTrack, pfile);

    //Ret = AO_Track_AllocHandle(&hHandle,pstfilp);
    if(MT_SUCCESS != Ret)
    {
        return Ret;
    }

    Ret = AO_Track_PreCreate(enSound, pstAttr, bAlsaTrack, NULL, hHandle);
    if (MT_SUCCESS != Ret)
    {
        AO_Track_FreeHandle(hHandle);
        return Ret;
    }

    *phTrack = hHandle;

    return Ret;
}

mt_s32 MT_DRV_AO_Track_Destroy(mt_handle hSndTrack)
{
    mt_s32 Ret = MT_SUCCESS;
    CHECK_AO_TRACK_OPEN(hSndTrack);

    Ret = AO_Track_Destory(hSndTrack);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }

    AO_Track_FreeHandle(hSndTrack);

    return Ret;
}

mt_s32 MT_DRV_AO_Track_Flush(mt_handle hSndTrack)
{
    CHECK_AO_TRACK_OPEN(hSndTrack);
    return AO_Track_Flush(hSndTrack);
}

mt_s32 MT_DRV_AO_Track_Start(mt_handle hSndTrack)
{
    CHECK_AO_TRACK_OPEN(hSndTrack);
    return AO_Track_Start(hSndTrack);
}

mt_s32 MT_DRV_AO_Track_Stop(mt_handle hSndTrack)
{
    CHECK_AO_TRACK_OPEN(hSndTrack);
    return AO_Track_Stop(hSndTrack);
}

mt_s32 MT_DRV_AO_Track_GetDelayMs(mt_handle hSndTrack, mt_u32 *pDelayMs)
{
    CHECK_AO_TRACK_OPEN(hSndTrack);
    return AO_Track_GetDelayMs(hSndTrack, pDelayMs);
}

mt_s32 MT_DRV_AO_Track_SendData(mt_handle hSndTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame)
{
    CHECK_AO_TRACK_OPEN(hSndTrack);
    return AO_Track_SendData(hSndTrack, pstAOFrame);
}

mt_s32 MT_DRV_AO_Track_AttachAi(mt_handle hSndTrack, mt_handle hAi)
{
    CHECK_AO_TRACK_OPEN(hSndTrack);
    return AO_Track_AttachAi(hSndTrack, hAi);
}

mt_s32 MT_DRV_AO_Track_DetachAi(mt_handle hSndTrack, mt_handle hAi)
{
    CHECK_AO_TRACK_OPEN(hSndTrack);
    return AO_Track_DetachAi(hSndTrack, hAi);
}

mt_s32 AO_DRV_Suspend(basedev_s * pdev,
                      pm_message_t   state)
{
    AO_PRIV_DATA_S *ao_priv_data = dev_get_platdata(&pdev->dev);
    MT_PRINT("AO suspend OK\n");
    
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    if(!g_audio_mute){
        drv_gpio_set_value(GPIO_7, GPIO_VALUE_LOW_LEVEL);//turn down amplifier power to avoid pop noise
    }
#endif
    if (!IS_ERR_OR_NULL(ao_priv_data->audoutclk))
        clk_disable_unprepare(ao_priv_data->audoutclk);
    if (!IS_ERR_OR_NULL(ao_priv_data->audoutaxiclk))
        clk_disable_unprepare(ao_priv_data->audoutaxiclk);

    return MT_SUCCESS;
}

mt_s32 AO_DRV_Resume(basedev_s * pdev)
{
    AO_PRIV_DATA_S *ao_priv_data = dev_get_platdata(&pdev->dev);
    mt_u32 i,data;
    if (!IS_ERR_OR_NULL(ao_priv_data->audoutclk))
        clk_prepare_enable(ao_priv_data->audoutclk);
    if (!IS_ERR_OR_NULL(ao_priv_data->audoutaxiclk))
    {
        clk_prepare_enable(ao_priv_data->audoutaxiclk);
    }

	//{       
        if(!g_audio_reg_base)
        	g_audio_reg_base = mt_get_audio_base();

        //audio  reg init
        {
          reg_sym_linux_00_reg_32_bit(1 ); //p_regg->bitc.pcm_32b_flag= 1;
          reg_sym_linux_04_reg_justified_bit( 2); // p_reg->bitc.justified_mode = 2;
          //set src
          for(i = 0;i < 256;i++)
          {
        	data = srcflt_coef[i] & 0x001fffff; //low 21bits
        	data |= 0x80000000;  //wr_coef_en
        	data |= i << 21;	   //wr_coef_addr
        	*((volatile u32 *)(g_audio_reg_base + REG_AUD_SNT_SRC)) = data;
          }
        }
	//}

    MT_PRINT("AO resume OK\n");
#ifdef MT_ALSA_AO_SUPPORT
	if(MT_NULL != hisi_snd_device && MT_TRUE == bu32shallowSuspendActive)
	{
		MT_INFO_AO("\nAO ALSA shallow resume \n");

		bu32shallowSuspendActive = MT_FALSE;
	    s32Ret = snd_soc_resume(&hisi_snd_device->dev);
	    if (MT_SUCCESS != s32Ret)
	    {
	       MT_ERR_AO("AO ALSA shallow resume fail.\n");
	    }

		MT_PRINT("AO ALSA shallow resume OK.\n");
	}
#endif
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    if(!g_audio_mute){
        drv_gpio_set_value(GPIO_7, GPIO_VALUE_HIGH_LEVEL);
    }
#endif

    return MT_SUCCESS;
}

mt_s32 AO_DRV_Init(mt_void)
{
    mt_s32 s32Ret;

    s32Ret = down_interruptible(&g_AoMutex);
    s32Ret = mt_drv_module_register(MT_ID_AO, AO_NAME,
                                    (mt_void*)&s_stAoDrv.stExtFunc);
    if (MT_SUCCESS != s32Ret)
    {
        MT_FATAL_AO("Reg module fail:%#x!\n", s32Ret);
        up(&g_AoMutex);
        return s32Ret;
    }

#ifdef ENA_AO_IRQ_PROC
    /* register ade ISR */
    if (0
        != request_irq(AO_IRQ_NUM, AO_IntVdmProc,
                       0, "aiao",
                       MT_NULL))
    {
        MT_FATAL_AO("FATAL: request_irq for VDI VDM err!\n");
        up(&g_AoMutex);
        return MT_FAILURE;
    }
#endif


    up(&g_AoMutex);
    return MT_SUCCESS;
}

mt_void AO_DRV_Exit(mt_void)
{
    mt_s32 s32Ret;

    s32Ret = down_interruptible(&g_AoMutex);
#ifdef ENA_AO_IRQ_PROC
    free_irq(AO_IRQ_NUM, MT_NULL);
#endif
    mt_drv_module_unregister(MT_ID_AO);

    up(&g_AoMutex);
    return;
}

#ifdef __cplusplus
 #if __cplusplus
 #endif
#endif /* End of #ifdef __cplusplus */
