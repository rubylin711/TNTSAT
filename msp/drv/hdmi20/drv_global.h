/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_GLOBAL_H__
#define __DRV_GLOBAL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "mt_error_mpi.h"
//#include "mpi_priv_hdmi.h"
#include "mt_drv_hdmi.h"
#include "mt_drv_disp.h"
#include "mt_unf_hdmi.h"
#include "drv_hdmi.h"

#include "drv_hdmi_debug.h"
//#include "test_edid.h"

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#define DISP_SUPPORT_4K_SOLUTION	(1)
#define HDMI_EMP_VERIFY_TEST			(0)
#elif defined(CONFIG_MT_CHIP_ETUDE2)
#define DISP_SUPPORT_4K_SOLUTION	(1)
#define HDMI_EMP_VERIFY_TEST			(0)
#else
#define DISP_SUPPORT_4K_SOLUTION	(1)
#define HDMI_EMP_VERIFY_TEST			(0)
#endif
#define DEF_FILE_NAMELENGTH 32
//#define HDMI_DEBUG
#define HDMI20_DEBUG
#define HDMI20_MISC_DEBUG
#define HDMI20_DRV_REG_PROC_DEBUG
#define HDMI20_INTF_K_DEBUG
#define HDMI20_DRV_HDMI_DEBUG

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#define SI_COM_ALL_PASS			(0)
#else
#define SI_COM_ALL_PASS			(0)
#endif
#define HDMI_DEV_MAGIC			(0xa55aa55a)
#define HDMI_DISPLAY_READY		(1)
#define HDMI20_EMP_DMA		(1)
#define HDCP_SUPPORT
#define HDMI20_EMP_DMA_ADDR_FIX	(1)
#define CEC_SUPPORT
#define ASSERT_HDMI20(_exp)        if (!(_exp)) { printk("ASSERT at %s:%d\n", __FILE__, __LINE__); while(1); }
#define DelayMS					SiiPlatformTimeMilliDelay
extern int g_hdmi_dbg_en;
#define GET_3D_STRUCT_STRING(a) (( a == 1 ) ? "Frame Packing" : \
								 (( a == 2 ) ? "Top And Bottom" : \
								  (( a == 3 ) ? "Frame Packing + Top And Bottom" : \
								   (( a == 4 ) ? "Side by Side" : \
									(( a == 5 ) ? "Frame Packing + Side by Side" : \
									 (( a == 6 ) ? "Top And Bottom + Side by Side" : \
									  (( a == 7 ) ? "Frame Packing + Top And Bottom + Side by Side" : \
									   "-")))))))

/*
**HDMI Debug
*/
#ifdef HDMI_DEBUG

#define EDID_INFO(fmt...)           HDMI_DBG_INFO   (HDMI_DEBUG_PRINT_EDID, fmt)
#define EDID_WARN(fmt...)           HDMI_DBG_WARN   (HDMI_DEBUG_PRINT_EDID, fmt)
#define EDID_ERR(fmt...)            HDMI_DBG_ERR    (HDMI_DEBUG_PRINT_EDID, fmt)
#define EDID_FATAL(fmt...)          HDMI_DBG_FATAL  (HDMI_DEBUG_PRINT_EDID, fmt)

#define HDCP_INFO(fmt...)           HDMI_DBG_INFO   (HDMI_DEBUG_PRINT_HDCP, fmt)
#define HDCP_WARN(fmt...)           HDMI_DBG_WARN   (HDMI_DEBUG_PRINT_HDCP, fmt)
#define HDCP_ERR(fmt...)            HDMI_DBG_ERR    (HDMI_DEBUG_PRINT_HDCP, fmt)
#define HDCP_FATAL(fmt...)          HDMI_DBG_FATAL  (HDMI_DEBUG_PRINT_HDCP, fmt)

#define CEC_INFO(fmt...)            HDMI_DBG_INFO   (HDMI_DEBUG_PRINT_CEC, fmt)
#define CEC_WARN(fmt...)            HDMI_DBG_WARN   (HDMI_DEBUG_PRINT_CEC, fmt)
#define CEC_ERR(fmt...)             HDMI_DBG_ERR    (HDMI_DEBUG_PRINT_CEC, fmt)
#define CEC_FATAL(fmt...)           HDMI_DBG_FATAL  (HDMI_DEBUG_PRINT_CEC, fmt)

#define HPD_INFO(fmt...)            HDMI_DBG_INFO   (HDMI_DEBUG_PRINT_HPD, fmt)
#define HPD_WARN(fmt...)            HDMI_DBG_WARN   (HDMI_DEBUG_PRINT_HPD, fmt)
#define HPD_ERR(fmt...)             HDMI_DBG_ERR    (HDMI_DEBUG_PRINT_HPD, fmt)
#define HPD_FATAL(fmt...)           HDMI_DBG_FATAL  (HDMI_DEBUG_PRINT_HPD, fmt)

#define COM_INFO(fmt...)            HDMI_DBG_INFO   (HDMI_DEBUG_PRINT_COM, fmt)
#define COM_WARN(fmt...)            HDMI_DBG_WARN   (HDMI_DEBUG_PRINT_COM, fmt)
#define COM_ERR(fmt...)             HDMI_DBG_ERR    (HDMI_DEBUG_PRINT_COM, fmt)
#define COM_FATAL(fmt...)           HDMI_DBG_FATAL  (HDMI_DEBUG_PRINT_COM, fmt)

#else
#if 0
#define EDID_INFO(fmt...)           MT_INFO_HDMI    (fmt)
#define EDID_WARN(fmt...)           MT_WARN_HDMI    (fmt)
#define EDID_ERR(fmt...)            MT_ERR_HDMI     (fmt)
#define EDID_FATAL(fmt...)          MT_FATAL_HDMI   (fmt)

#define HDCP_INFO(fmt...)           MT_INFO_HDMI    (fmt)
#define HDCP_WARN(fmt...)           MT_WARN_HDMI    (fmt)
#define HDCP_ERR(fmt...)            MT_ERR_HDMI     (fmt)
#define HDCP_FATAL(fmt...)          MT_FATAL_HDMI   (fmt)

#define CEC_INFO(fmt...)            MT_INFO_HDMI    (fmt)
#define CEC_WARN(fmt...)            MT_WARN_HDMI    (fmt)
#define CEC_ERR(fmt...)             MT_ERR_HDMI     (fmt)
#define CEC_FATAL(fmt...)           MT_FATAL_HDMI   (fmt)

#define HPD_INFO(fmt...)            MT_INFO_HDMI    (fmt)
#define HPD_WARN(fmt...)            MT_WARN_HDMI    (fmt)
#define HPD_ERR(fmt...)             MT_ERR_HDMI     (fmt)
#define HPD_FATAL(fmt...)           MT_FATAL_HDMI   (fmt)

#define COM_INFO(fmt...)            MT_INFO_HDMI    (fmt)
#define COM_WARN(fmt...)            MT_WARN_HDMI    (fmt)
#define COM_ERR(fmt...)             MT_ERR_HDMI     (fmt)
#define COM_FATAL(fmt...)           MT_FATAL_HDMI   (fmt)
#else
#define EDID_INFO(fmt...)           {if ( g_hdmi_dbg_en > 2 ) printk(fmt);}
#define EDID_WARN(fmt...)           {if ( g_hdmi_dbg_en > 1 ) printk(fmt);}
#define EDID_ERR(fmt...)            {if ( g_hdmi_dbg_en > 0 ) printk(fmt);}
#define EDID_FATAL(fmt...)          {if ( 1 ) printk(fmt);}

#define HDCP_INFO(fmt...)           {if ( g_hdmi_dbg_en > 2 ) printk(fmt);}
#define HDCP_WARN(fmt...)           {if ( g_hdmi_dbg_en > 1 ) printk(fmt);}
#define HDCP_ERR(fmt...)            {if ( g_hdmi_dbg_en > 0 ) printk(fmt);}
#define HDCP_FATAL(fmt...)          {if ( 1 ) printk(fmt);}

#define CEC_INFO(fmt...)            {if ( g_hdmi_dbg_en > 2 ) printk(fmt);}
#define CEC_WARN(fmt...)            {if ( g_hdmi_dbg_en > 1 ) printk(fmt);}
#define CEC_ERR(fmt...)             {if ( g_hdmi_dbg_en > 0 ) printk(fmt);}
#define CEC_FATAL(fmt...)           {if ( 1 ) printk(fmt);}

#define HPD_INFO(fmt...)            {if ( g_hdmi_dbg_en > 2 ) printk(fmt);}
#define HPD_WARN(fmt...)            {if ( g_hdmi_dbg_en > 1 ) printk(fmt);}
#define HPD_ERR(fmt...)             {if ( g_hdmi_dbg_en > 0 ) printk(fmt);}
#define HPD_FATAL(fmt...)           {if ( 1 ) printk(fmt);}

#define COM_INFO(fmt...)            {if ( g_hdmi_dbg_en > 2 ) printk(fmt);}
#define COM_WARN(fmt...)            {if ( g_hdmi_dbg_en > 1 ) printk(fmt);}
#define COM_ERR(fmt...)             {if ( g_hdmi_dbg_en > 0 ) printk(fmt);}
#define COM_FATAL(fmt...)           {if ( 1 ) printk(fmt);}
#endif

#endif

#ifdef HDMI20_DRV_HDMI_DEBUG
#define HDMI20_DRV_HDMI_PRINTK(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk(KERN_CRIT args); \
		} \
	} while (0)

#define HDMI20_DRV_HDMI_PRINT_FUNC_ENTER(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk("\n[%s_%s_%d] hdmi20_tag enter...\n",__FILE__,__func__,__LINE__); \
		} \
	} while (0)

#define HDMI20_DRV_HDMI_PRINT_FUNC_EXIT(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk("\n[%s_%s_%d] hdmi20_tag exit...\n",__FILE__,__func__,__LINE__); \
		} \
	} while (0)

#define HDMI20_DRV_HDMI_EMP7_PRINTK(args...) \
	do { \
		if (g_hdmi_dbg_en>6) { \
			printk(KERN_CRIT args); \
		} \
	} while (0)
#define HDMI20_DRV_HDMI_EMP8_PRINTK(args...) \
	do { \
		if (g_hdmi_dbg_en>7) { \
			printk(KERN_CRIT args); \
		} \
	} while (0)
#else
#define HDMI20_DRV_HDMI_PRINTK(...)  do{}while(0)
#define HDMI20_DRV_HDMI_PRINT_FUNC_ENTER(...)  do{}while(0)
#define HDMI20_DRV_HDMI_PRINT_FUNC_EXIT(...)  do{}while(0)
#endif

#ifdef  HDMI20_MISC_DEBUG
#define HDMI20_MISC_PRINTK(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk(KERN_CRIT args); \
		} \
	} while (0)

#define HDMI20_MISC_PRINT_FUNC_ENTER(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk("\n[%s_%s_%d] hdmi20_tag enter...\n",__FILE__,__func__,__LINE__); \
		} \
	} while (0)

#define HDMI20_MISC_PRINT_FUNC_EXIT(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk("\n[%s_%s_%d] hdmi20_tag exit...\n",__FILE__,__func__,__LINE__); \
		} \
	} while (0)

#else
#define HDMI20_MISC_PRINTK(...)  do{}while(0)
#define HDMI20_MISC_PRINT_FUNC_ENTER(...)  do{}while(0)
#define HDMI20_MISC_PRINT_FUNC_EXIT(...)  do{}while(0)
#endif

#ifdef HDMI20_DRV_REG_PROC_DEBUG
#define HDMI20_DRV_REG_PROC_PRINTK(args...) \
	do { \
		if (g_hdmi_dbg_en > 10) { \
			printk(KERN_CRIT args); \
		} \
	} while (0)

#define HDMI20_DRV_REG_PROC_PRINT_FUNC_ENTER(args...) \
	do { \
		if (g_hdmi_dbg_en > 10) { \
			printk("\n[%s_%s_%d] hdmi20_tag enter...\n",__FILE__,__func__,__LINE__); \
		} \
	} while (0)

#define HDMI20_DRV_REG_PROC_PRINT_FUNC_EXIT(args...) \
	do { \
		if (g_hdmi_dbg_en > 10) { \
			printk("\n[%s_%s_%d] hdmi20_tag exit...\n",__FILE__,__func__,__LINE__); \
		} \
	} while (0)

#else
#define HDMI20_DRV_REG_PROC_PRINTK(...)  do{}while(0)
#define HDMI20_DRV_REG_PROC_PRINT_FUNC_ENTER(...)  do{}while(0)
#define HDMI20_DRV_REG_PROC_PRINT_FUNC_EXIT(...)  do{}while(0)
#endif

#ifdef HDMI20_DEBUG
#define HDMI20_PRINTK(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk(KERN_CRIT args); \
		} \
	} while (0)

#define HDMI20_PRINT_FUNC_ENTER(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk("\n[%s_%s_%d] hdmi20_tag enter...\n",__FILE__,__func__,__LINE__); \
		} \
	} while (0)

#define HDMI20_PRINT_FUNC_EXIT(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk("\n[%s_%s_%d] hdmi20_tag exit...\n",__FILE__,__func__,__LINE__); \
		} \
	} while (0)

#else
#define HDMI20_PRINTK(...)  do{}while(0)
#define HDMI20_PRINT_FUNC_ENTER(...)  do{}while(0)
#define HDMI20_PRINT_FUNC_EXIT(...)  do{}while(0)
#endif

#ifdef HDMI20_INTF_K_DEBUG
#define HDMI20_INTF_K_PRINTK(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk(KERN_CRIT args); \
		} \
	} while (0)

#define HDMI20_INTF_K__PRINT_FUNC_ENTER(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk("\n[%s_%s_%d] hdmi20_tag enter...\n",__FILE__,__func__,__LINE__); \
		} \
	} while (0)

#define HDMI20_INTF_K__PRINT_FUNC_EXIT(args...) \
	do { \
		if (g_hdmi_dbg_en>1) { \
			printk("\n[%s_%s_%d] hdmi20_tag exit...\n",__FILE__,__func__,__LINE__); \
		} \
	} while (0)

#else
#define HDMI20_INTF_K_PRINTK(...)  do{}while(0)
#define HDMI20_INTF_K__PRINT_FUNC_ENTER(...)  do{}while(0)
#define HDMI20_INTF_K__PRINT_FUNC_EXIT(...)  do{}while(0)
#endif

//g_stHdmiCommParam

typedef struct {
	MT_BOOL     bOpenMce2App;        /* smooth change Mce to App mode*/
	MT_BOOL     bOpenedInBoot;             /*boot => mce : we only need set default param to attr(no config) and create thread*/
	struct task_struct  *kThreadTimer;    /*timer thread*//*CNcomment:定时器线程 */
	MT_BOOL     kThreadTimerStop;
	struct task_struct  *kCECRouter;      /*CEC thread*//*CNcomment: CEC线程 */
	MT_UNF_HDMI_VIDEO_MODE_E enVidInMode; /*reservation,please setting VIDEO_MODE_YCBCR422 mode*//*CNcomment:保留，请配置为VIDEO_MODE_YCBCR422 */
} HDMI_COMM_ATTR_S;

/** VSDB Mode */
typedef enum {
	VSDB_MODE_NONE = 0x00,
	VSDB_MODE_3D,
	VSDB_MODE_4K,
	VSDB_MODE_BUTT
} VSDB_MODE_E;

HDMI_COMM_ATTR_S *DRV_Get_CommAttr(mt_void);

void DRV_PrintCommAttr(mt_void);

MT_UNF_HDMI_VIDEO_MODE_E DRV_Get_VIDMode(mt_void);
void DRV_Set_VIDMode(MT_UNF_HDMI_VIDEO_MODE_E enVInMode);

mt_s32 DRV_Get_IsMce2App(mt_void);
void DRV_Set_Mce2App(MT_BOOL bSmooth);

mt_s32 DRV_Get_IsOpenedInBoot(mt_void);
void DRV_Set_OpenedInBoot(MT_BOOL bOpened);

mt_s32 DRV_Get_IsThreadStoped(mt_void);
void DRV_Set_ThreadStop(MT_BOOL bStop);
//g_stHdmiCommParam end

#define MAX_PROCESS_NUM 10
#define PROC_EVENT_NUM 5

typedef struct {
	mt_u32      bUsed;
	mt_u32      CurEventNo;
	mt_u32      Event[PROC_EVENT_NUM];
	ulong	u32ProcHandle;
} HDMI_PROC_EVENT_S;

typedef struct {
	MT_BOOL      bUnderScanDev;
} HDMI_PRIVATE_EDID_S;

typedef struct {
	MT_BOOL            bOpen;
	MT_BOOL            bStart;
	MT_BOOL            bValidSinkCap;
	HDMI_PROC_EVENT_S  eventList[MAX_PROCESS_NUM];
	//mt_u32           Event[5];        /*Current Event Array, sequence will be change */
	HDMI_ATTR_S        stHDMIAttr;          /*HDMI implement parameter*//*CNcomment:HDMI 运行参数 */
	MT_BOOL            ForceUpdateFlag;
	MT_BOOL            partUpdateFlag;

	VSDB_MODE_E        enVSDBMode;
	MT_UNF_HDMI_AVI_INFOFRAME_VER2_S   stAVIInfoFrame;
	MT_UNF_HDMI_AUD_INFOFRAME_VER1_S   stAUDInfoFrame;

	MT_BOOL                            bCECEnable;
	MT_BOOL                            bCECStart;
	MT_U8                              u8CECCheckCount;
	MT_UNF_HDMI_CEC_STATUS_S           stCECStatus;

	MT_UNF_HDMI_DEFAULT_ACTION_E       enDefaultMode;
	hdmi_video_config_t					mt_av_params;
	HDMI_APP_ATTRMT_S        stAppAttrMt;          /*HDMI implement parameter*//*CNcomment:HDMI 运行参数 */
	hdmi_notify_info_t      notify_info[DRV_HDMI_NOTIFY_MAX_NUMBER];
} HDMI_CHN_ATTR_S;

typedef struct {
	mt_u32      bEdidLen;
	MT_U8      *u8Edid;
} HDMI_Test_EDID_S;

HDMI_CHN_ATTR_S *DRV_Get_ChnAttr(mt_void);

mt_u32 DRV_HDMI_SetDefaultAttr(mt_void);
HDMI_ATTR_S *DRV_Get_HDMIAttr(MT_UNF_HDMI_ID_E enHdmi);
HDMI_APP_ATTR_S   *DRV_Get_AppAttr(MT_UNF_HDMI_ID_E enHdmi);
HDMI_VIDEO_ATTR_S *DRV_Get_VideoAttr(MT_UNF_HDMI_ID_E enHdmi);
HDMI_AUDIO_ATTR_S *DRV_Get_AudioAttr(MT_UNF_HDMI_ID_E enHdmi);
MT_UNF_EDID_BASE_INFO_S *DRV_Get_SinkCap(MT_UNF_HDMI_ID_E enHdmi);

HDMI_PRIVATE_EDID_S *DRV_Get_PriSinkCap(MT_UNF_HDMI_ID_E enHdmi);

MT_BOOL DRV_Get_IsNeedForceUpdate(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_ForceUpdateFlag(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bupdate);

MT_BOOL DRV_Get_IsNeedPartUpdate(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_PartUpdateFlag(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bupdate);

HDMI_PROC_EVENT_S *DRV_Get_EventList(MT_UNF_HDMI_ID_E enHdmi);
MT_UNF_HDMI_CEC_STATUS_S *DRV_Get_CecStatus(MT_UNF_HDMI_ID_E enHdmi);

MT_UNF_HDMI_AVI_INFOFRAME_VER2_S *DRV_Get_AviInfoFrm(MT_UNF_HDMI_ID_E enHdmi);
MT_UNF_HDMI_AUD_INFOFRAME_VER1_S *DRV_Get_AudInfoFrm(MT_UNF_HDMI_ID_E enHdmi);

MT_BOOL DRV_Get_IsChnOpened(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_ChnOpen(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bChnOpen);

MT_BOOL DRV_Get_IsChnStart(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_ChnStart(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bChnStart);

MT_BOOL DRV_Get_IsCECEnable(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_CECEnable(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bCecEnable);

MT_BOOL DRV_Get_IsCECStart(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_CECStart(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bCecStart);

MT_BOOL DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_E enHdmi);
hdmi_video_config_t *DRV_Get_Av_Params(MT_UNF_HDMI_ID_E enHdmi);
HDMI_APP_ATTRMT_S *DRV_Get_AppAttrMt(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bSinkValid);

MT_UNF_HDMI_DEFAULT_ACTION_E DRV_Get_DefaultOutputMode(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_DefaultOutputMode(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEFAULT_ACTION_E enDefaultMode);

MT_DRV_HDMI_AUDIO_CAPABILITY_S *DRV_Get_OldAudioCap(void);
hdmi_notify_info_t* DRV_Get_NotifyHdl(MT_UNF_HDMI_ID_E enHdmi);

mt_u32 DRV_Get_DDCSpeed(void);
void DRV_Set_DDCSpeed(mt_u32 delayCount);

//
HDMI_EDID_S *DRV_Get_UserEdid(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_UserEdid(MT_UNF_HDMI_ID_E enHdmi, HDMI_EDID_S *pEDID);

//
MT_BOOL DRV_Get_IsUserEdid(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_UserEdidMode(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bUserEdid);

//
VSDB_MODE_E DRV_Get_VSDBMode(MT_UNF_HDMI_ID_E enHdmi);
void DRV_Set_VSDBMode(MT_UNF_HDMI_ID_E enHdmi, VSDB_MODE_E enVSDBMode);

MT_BOOL DRV_Get_Is4KFmt(MT_DRV_DISP_FMT_E enFmt);
MT_BOOL DRV_Get_IsLCDFmt(MT_DRV_DISP_FMT_E enFmt);
MT_BOOL DRV_Get_IsPixelRepeatFmt(MT_DRV_DISP_FMT_E enFmt);

MT_BOOL DRV_Get_IsForceOutput(void);
void DRV_Set_ForceOutputMode(MT_BOOL bForce);

//internal dubug option
//#define DEBUG_EVENTLIST
//#define DEBUG_NOTIFY_COUNT
//#define DEBUG_NEED_RESET

//#define DEBUG_TIMER
//#define DEBUG_PROCID
//#define DEBUG_EDID
//#define ANDROID_SUPPORT
//#define DEBUG_HDCP

#ifdef DEBUG_HDCP
#define HDCP_PRINT MT_PRINT
#else
#define HDCP_PRINT MT_INFO_HDMI
#endif

#define MT_DEVELOP_COMP_MASK	(0)
#define MT_RELATED_MODE_NUM (10)
//#define EDID_FULL_COLOR_RANGE

typedef enum HDMI_RELATED_REG_MOD {
	HDMI_MOD_DIG = 0,
	HDMI_MOD_ANALOG_1F50,
	HDMI_MOD_DIG_KRAM_BASE,
	HDMI_MOD_DIG_KRAM_CTRL0,
	HDMI_MOD_DIG_KRAM_CTRL1,
	HDMI_MOD_ANALOG_1F5D,
	HDMI_MOD_ANALOG_1F57,
	HDMI_MOD_MAX
} HDMI_RELATED_REG_MOD_E;

typedef enum HDMI20_EMP_MODE_E {
	HDMI20_EMP_CPU_ONLY,
	HDMI20_EMP_DMA_DMA,
	HDMI20_EMP_DMA_CPU,
	HDMI20_EMP_MODE_MAX
} HDMI20_EMP_MODE_T;

#define HDMI_ANA_REG_IN_HDMI		(0)
#if HDMI_ANA_REG_IN_HDMI
typedef struct hdmi20_ana_regs {
	u32 ana_hdmi_ao_reg0;		//0x1f570000
	u32 ana_hdmi_tx_reg0;		//0x1f5d006c ANA_HDMI_RANGE_REG
	u32 ana_hdmi_tx_reg1;		//0x1f5d01c0
	u32 ana_hdmi_tx_reg2;		//0x1f5d01c4
	u32 ana_reg_hdmi_test;		//0x1f5d01c8
	u32 ana_clkgen_vhd_reg;		//0x1f5d0058
	u32 ana_clkgen_vhdintp;		//0x1f5d005c
	u32 ana_clkgen_vsdpll;		//0x1f5d0060
	u32 ana_clkgen_vsdintp;		//0x1f5d0064
	u32 ana_clkgen_pdsys_reg;	//0x1f5d009c
	u32 ana_clkgen_vhdssc_reg;	//0x1f5d0140
	u32 ana_clkgen_vsdssc_reg;	//0x1f5d0144
	u32 ana_disp_vout_clk;		//0x1f50a600
	u32 ana_disp_vout_clksel;	//0x1f50a604
	u32 ana_sys_block_rst;		//0x1f50a60c
} hdmi20_ana_regs_t;
#endif

typedef struct hdmi20_params {
	u32 hdcp_en;
	u32 hdcp2x_en;
	u32 cec_en;
	u32 scdc_en;
	HDMI20_EMP_MODE_T emp_mode;
	u32 hdcp_enhance_cfg;
	u32 clk_cfg;
	#if HDMI_ANA_REG_IN_HDMI
	hdmi20_ana_regs_t hdmi_ana_regs;
	#endif
} hdmi20_params_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __DRV_GLOBAL_H__ */

/*------------------------------------END-------------------------------------*/

