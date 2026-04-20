/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_AO_PRIVATE_H__
#define __DRV_AO_PRIVATE_H__

#include "mt_type.h"
#include "mt_module.h"
#include "mt_drv_sys.h"
#include "mt_drv_dev.h"
#include "mt_drv_mmz.h"
#include "mt_drv_mem.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_module.h"
#include "drv_hdmi_ext.h"
//#include "drv_adsp_ext.h"
#include "drv_ao_ext.h"
//#include "drv_gpio_ext.h"
#include "drv_ao_ioctl.h"//MT_ALSA_I2S_ONLY_SUPPORT

#include "drv_pdm_ext.h"

#include "hal_aoe_common.h"
//#include "mt_audsp_aflt.h" //TODO for AFLT_MAX_CHAN_NUM

//#include "../../hdmi/mta_hdmi_hdmi.h"
#include "../../hdmi20/drv_hdmi.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */
#define AO_MAX_TOTAL_SND_NUM (MT_UNF_SND_BUTT)

#define AO_SND_FILE_NOUSE_FLAG  0xFFFFFFFF

#define AO_SOUND_PATH_NAME_MAXLEN 256		//for proc write
#define AO_SOUND_FILE_NAME_MAXLEN 256
#ifdef ENA_AO_IRQ_PROC
 #define AO_IRQ_NUM (38 + 32)           /*interrupt adetor*/
#endif
#define AO_NAME "MT_AO"

#define SND_MAX_OPEN_NUM 32

#define AFLT_MAX_CHAN_NUM 8//minnan

#ifdef MT_SND_MUTECTL_SUPPORT
#define AO_SND_MUTE_RESUME_DISABLE_TIMEMS 500
#define AO_SND_MUTE_BOOT_DISABLE_TIMEMS   400
#endif

#define AO_STRING_SKIP_BLANK(str)       \
    while (str[0] == ' ')           \
    {                               \
        (str)++;                    \
    }

#define AO_STRING_SKIP_NON_BLANK(str)       \
    while (str[0] != ' ' && str[0] != '\0') \
    {                               \
        (str)++;                    \
    }

#define AO_DEBUG_SHOW_HELP(u32Snd) \
    do                                                         \
    {                                                          \
        mt_drv_proc_echohelp("\nfunction: save pcm data from track\n"); \
        mt_drv_proc_echohelp("commad:   echo save_track track_id start|stop > /proc/msp/sound%d\n", u32Snd); \
        mt_drv_proc_echohelp("example:  echo save_track 0 start > /proc/msp/sound%d\n", u32Snd); \
        mt_drv_proc_echohelp("function: \nfunction: save pcm data from sound\n"); \
        mt_drv_proc_echohelp("commad:   echo save_sound start|stop > /proc/msp/sound%d\n", u32Snd); \
        mt_drv_proc_echohelp("example:  echo save_sound start > /proc/msp/sound%d\n\n", u32Snd); \
    } while (0)

typedef enum
{
    SND_DEBUG_CMD_CTRL_START = 0,
    SND_DEBUG_CMD_CTRL_STOP,
    SND_DEBUG_CMD_CTRL_BUTT
} SND_DEBUG_CMD_CTRL_E;

typedef enum
{
    SND_DEBUG_CMD_PROC_SAVE_TRACK = 0,
    SND_DEBUG_CMD_PROC_SAVE_SOUND,
    SND_DEBUG_CMD_PROC_BUTT
} SND_DEBUG_CMD_PROC_E;

typedef enum
{
    SND_PCM_OUTPUT_CERTAIN = 0,  //attach pcm outport. for example adac i2s etc.
    SND_PCM_OUTPUT_VIR_SPDIFORHDMI,  //only attach spdif or hdmi
} SND_PCM_OUTPUT_E;

typedef enum
{
    SND_HDMI_MODE_NONE = 0,  //don't attatch hdmi
    SND_HDMI_MODE_PCM,
    SND_HDMI_MODE_LBR,
    SND_HDMI_MODE_HBR,
} SND_HDMI_MODE_E;

typedef enum
{
    SND_SPDIF_MODE_NONE = 0,  //don't attatch spdif
    SND_SPDIF_MODE_PCM,
    SND_SPDIF_MODE_LBR,
} SND_SPDIF_MODE_E;

typedef enum
{
    SND_ENGINE_TYPE_PCM = 0,
    SND_ENGINE_TYPE_SPDIF_RAW = 1,
    SND_ENGINE_TYPE_HDMI_RAW = 2,
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    SND_ENGINE_TYPE_MIXBUF_DD = SND_ENGINE_TYPE_HDMI_RAW,
#endif

    SND_ENGINE_TYPE_BUTT
} SND_ENGINE_TYPE_E;

typedef enum
{
    SND_TRACK_STATUS_STOP = 0,
    SND_TRACK_STATUS_START,
    SND_TRACK_STATUS_PAUSE,
    SND_TRACK_STATUS_BUTT,
} SND_TRACK_STATUS_E;

typedef enum
{
    SND_TRACK_ATTR_RETAIN = 0,
    SND_TRACK_ATTR_MODIFY,
    SND_TRACK_ATTR_MASTER2SLAVE,        //RECREATE
    SND_TRACK_ATTR_SLAVE2MASTER,        //RECREATE
    SND_TRACK_ATTR_BUTT,
} SND_TRACK_ATTR_SETTING_E;

typedef enum
{
    SND_OP_STATUS_STOP = 0,
    SND_OP_STATUS_START,
    SND_OP_STATUS_CAST_BUTT,
} SND_OP_STATUS_E;

/*track aoe engine state*/
typedef struct
{
    AOE_ENGINE_CHN_ATTR_S stUserEngineAttr;

    /* internal state */
    AOE_ENGINE_ID_E   enEngine;
    SND_ENGINE_TYPE_E enEngineType;
} SND_ENGINE_STATE_S;

#if 1//def MT_ALSA_I2S_ONLY_SUPPORT
 typedef struct
 {
  phys_addr_t  u32BufPhyAddr;
  ulong  u32BufVirAddr;
  mt_u32  u32BufSize;
  mt_u32  u32PeriodByteSize;
  mt_u32  u32Periods;
 } AO_ALSA_BUF_ATTR_S;
 typedef struct mtAO_ALSA_Param_S //MT_ALSA_I2S_ONLY_SUPPORT
 {
     MT_BOOL                  bAlsaI2sUse; //if MT_ALSA_I2S_ONLY_SUPPORT //0730
     MT_UNF_SAMPLE_RATE_E       enRate;
     AO_ALSA_BUF_ATTR_S       stBuf; //for  alsa  mmap dma buffer
     void                     *IsrFunc;    //for alsa ISR func
     void                     *substream;  //for alsa ISR func params
 }AO_ALSA_I2S_Param_S;
#endif
typedef struct
{
    MT_BOOL is_mute;
    MT_UNF_TRACK_MODE_E trackmode;
    MT_UNF_TRACK_MODE_E trackmode_now;    //trackmode now
    int channles_now;                     //chans now
    mt_u8 volume;
    #ifdef CONFIG_MT_AUDIO_AD
    mt_u8 volume_ad;      //ad' volume
    mt_u8 volume_ad_now;      //ad' now volume
    #endif
}AO_RECORD_S;
typedef struct
{
    AO_ALSA_I2S_Param_S     stUserOpenParamI2s;//for i2s only card resume  MT_ALSA_I2S_ONLY_SUPPORT
    MT_UNF_SND_ATTR_S       stUserOpenParam;
    MT_UNF_SAMPLE_RATE_E    enUserSampleRate;
    MT_UNF_SND_HDMI_MODE_E  enUserHdmiMode;
    MT_UNF_SND_SPDIF_MODE_E enUserSpdifMode;

    /* internal state */
    SND_PCM_OUTPUT_E        enPcmOutput;          /*0(pcm output surely), 1(whether pcm output according spdif or hdmi)*/
    SND_HDMI_MODE_E         enHdmiPassthrough;   /*0(no hdmi),  1(pcm), 2(lbr), 3(hbr/7.1 lpcm)*/
    SND_SPDIF_MODE_E        enSpdifPassthrough;  /*0(no spdif),  1(pcm), 2(lbr)*/
    mt_u32                  u32HdmiDataFormat;
    mt_u32                  u32SpdifDataFormat;
    HDMI_EXPORT_FUNC_S      *pstHdmiFunc;
	mt_u32 					hdmi_acfg_flag;
    mt_u32          hdmi_acfg_usrchg;             //do only once

#ifdef MT_SND_MUTECTL_SUPPORT
    GPIO_EXT_FUNC_S         *pstGpioFunc;
    struct timer_list       stMuteDisableTimer;
#endif

    mt_u32                  uSndTrackInitFlag;
    mt_handle               hSndOp[MT_UNF_SND_OUTPUTPORT_MAX];
    mt_handle               hSndTrack[AO_MAX_TOTAL_TRACK_NUM];
    MT_BOOL					bAllTrackMute;	//exclude alsa track
    mt_handle               hSndEngine[SND_ENGINE_TYPE_BUTT];
    mmz_buffer_s            stTrackRbfMmz[SND_ENGINE_TYPE_BUTT];
    mt_u32                  u32AttAef;     //attached aef (bit)
    mt_handle               hAefProc[AFLT_MAX_CHAN_NUM];

    MT_BOOL                 bHdmiDebug;
    MT_BOOL                 bSndDestoryFlag;	/*for suspent popfree, Destory snd flag,when suspent or normal */
//#ifdef MT_SND_CAST_SUPPORT
    mt_u32    uSndCastInitFlag;
    mt_handle hCast[AO_MAX_CAST_NUM];
    mt_handle hCastOp[AO_MAX_CAST_NUM]; /*op cast used */
//#endif
	/*save pcm*/
	SND_DEBUG_CMD_CTRL_E	enSaveState;
	mt_u32 					u32SaveCnt;
	struct file *			fileHandle;
    AO_RECORD_S             ao_record;
} SND_CARD_STATE_S;

typedef struct tagAO_REGISTER_PARAM_S
{
    mt_proc_read_func  pfnReadProc;
    mt_drv_proc_write_func pfnWriteProc;
} AO_REGISTER_PARAM_S;

typedef struct
{
    MT_UNF_SND_E                enSound;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr;
    MT_BOOL                     bAlsaTrack;
    AO_BUF_ATTR_S               stBufAttr;
    MT_UNF_SND_ABSGAIN_ATTR_S   stTrackAbsGain;
    MT_BOOL                     bMute;
    MT_UNF_TRACK_MODE_E 		enChannelMode;
    SND_TRACK_STATUS_E          enCurnStatus;
    AO_SND_SPEEDADJUST_TYPE_E   enType;
    mt_s32                      s32Speed;
    MT_BOOL                     bAttAi;
    mt_handle                   hAi;
} SND_TRACK_SETTINGS_S;

//#if defined(MT_SND_DRV_SUSPEND_SUPPORT)//minnan
typedef struct
{
    MT_UNF_SND_ABSGAIN_ATTR_S   stCastAbsGain;
    MT_BOOL                     bMute;
    MT_UNF_SND_E           enSound;
    MT_BOOL                bUserEnableSetting;
    mt_u32                 u32UserVirtAddr;
    MT_UNF_SND_CAST_ATTR_S stCastAttr;
} SND_CAST_SETTINGS_S;

typedef struct
{
    MT_UNF_SND_GAIN_ATTR_S stUserGain;
    MT_UNF_TRACK_MODE_E    enUserTrackMode;
    mt_u32                 u32UserMute;
    MT_BOOL                bBypass;
    SND_OP_STATUS_E        enCurnStatus;
} SND_OUTPORT_ATTR_S;

typedef struct
{
    AO_ALSA_I2S_Param_S     stUserOpenParamI2s;//for i2s only card resume  MT_ALSA_I2S_ONLY_SUPPORT
    MT_UNF_SND_ATTR_S       stUserOpenParam;
    MT_UNF_SND_HDMI_MODE_E  enUserHdmiMode;
    MT_UNF_SND_SPDIF_MODE_E enUserSpdifMode;
    SND_OUTPORT_ATTR_S      stPortAttr[MT_UNF_SND_OUTPUTPORT_MAX];
    MT_BOOL                 bAllTrackMute;
    mt_u32                  u32AttAef;
    mt_handle               hAefProc[AFLT_MAX_CHAN_NUM];
} SND_CARD_SETTINGS_S;
//#endif

/* Echo proc save sound pcm */
typedef struct
{
//	MT_UNF_SND_E enSound;
//	struct file *fileHandle;
    SND_CARD_STATE_S *pCard;
    struct file *devfileHandle;

} SND_PCM_SAVE_ATTR_S;

/* private dev state Save AO Resource opened */
typedef struct
{
    mt_u32      u32SndBit;
    mt_u32      u32TrackBit;
    //todo
} AO_STATE_S;

//#ifdef MT_SND_CAST_SUPPORT
/* Cast entity */
typedef struct tagAO_Cast_ENTITY_S
{
    mt_u32       u32ReqSize;
    mmz_buffer_s stRbfMmz;     /* mmz dont release at suspend */
 //#if defined (MT_SND_DRV_SUSPEND_SUPPORT)
    SND_CAST_SETTINGS_S stSuspendAttr;
 //#endif
    long   u32File;          /* File handle */
    atomic_t atmUseCnt;        /* Cast use count, only support single user */
} AO_CAST_ENTITY_S;
//#endif

/* Track entity */
typedef struct tagAO_CHAN_ENTITY_S
{
//minnan remove
//#if defined (MT_SND_DRV_SUSPEND_SUPPORT)
    SND_TRACK_SETTINGS_S stSuspendAttr;
//#endif
    long   u32File;          /* File handle */
    atomic_t atmUseCnt;        /* Track use count, only support single user */
} AO_CHAN_ENTITY_S;

/* Snd entity */
typedef struct tagAO_SND_ENTITY_S
{
    SND_CARD_STATE_S *    pCard;                /* Snd structure pointer */
//#if defined (MT_SND_DRV_SUSPEND_SUPPORT)
    SND_CARD_SETTINGS_S stSuspendAttr;
//#endif
    ulong   u32File[SND_MAX_OPEN_NUM];                    /* File handle */
    atomic_t atmUseTotalCnt;     /* Snd use count, support multi user */
	atomic_t first_open;
    SND_CARD_STATE_S *    static_pCard;                /* Snd structure pointer */
} AO_SND_ENTITY_S;

/* Global parameter */
typedef struct
{
//#ifdef MT_SND_CAST_SUPPORT
    mt_u32           u32CastNum;        /* Record AO Cast num */
    AO_CAST_ENTITY_S astCastEntity[AO_MAX_CAST_NUM];
//#endif
    mt_u32           u32TrackNum;        /* Record AO track num */
    AO_CHAN_ENTITY_S astTrackEntity[AO_MAX_TOTAL_TRACK_NUM];   /* Track parameter */

    mt_u32          u32SndNum;           /* Record AO snd num */
    AO_SND_ENTITY_S astSndEntity[AO_MAX_TOTAL_SND_NUM];   /* Snd parameter */

    atomic_t               atmOpenCnt;      /* Open times */
    MT_BOOL                bReady;          /* Init flag */
    AO_REGISTER_PARAM_S*   pstProcParam;    /* AO Proc functions */
    //minnan remove
    //ADSP_EXPORT_FUNC_S*    pAdspFunc;       /* AO need ADSP extenal functions */
	PDM_EXPORT_FUNC_S*	   pstPDMFunc;		/* AO need PDM extenal functions */

    AIAO_EXPORT_FUNC_S     stExtFunc;       /* AO provide extenal functions */
} AO_GLOBAL_PARAM_S;

typedef struct mtDRV_AO_STATE_S
{
    atomic_t atmUserOpenCnt[AO_MAX_TOTAL_SND_NUM];      /*user snd Open times */
    mt_s32  u32FileId[AO_MAX_TOTAL_SND_NUM];
}DRV_AO_STATE_S;

typedef struct mtAO_PRIV_DATA_S
{
    struct clk *audoutclk;
    struct clk *audoutaxiclk;
} AO_PRIV_DATA_S;

mt_s32	AO_DRV_Init(mt_void);
mt_void AO_DRV_Exit(mt_void);
long    AO_DRV_Ioctl(struct file *file, mt_u32 cmd, unsigned long arg);
mt_s32	AO_DRV_Open(struct inode *inode, struct file  *filp);
mt_s32	AO_DRV_Release(struct inode *inode, struct file  *filp);
mt_s32	AO_DRV_RegisterProc(AO_REGISTER_PARAM_S *pstParam);
mt_void AO_DRV_UnregisterProc(mt_void);
mt_s32	AO_DRV_Suspend(basedev_s *pdev, pm_message_t state);
mt_s32	AO_DRV_Resume(basedev_s *pdev);
mt_s32  AO_DRV_WriteProc(struct file * file,
                                 const char __user * buf, size_t count, loff_t *ppos);

mt_s32  AO_DRV_ReadProc(struct seq_file *p, mt_void *v);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif
