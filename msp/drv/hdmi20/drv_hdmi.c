/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/poll.h>
//#include <mach/hardware.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "drv_hdmi.h"

#include "mt_drv_hdmi.h"

#include "mt_unf_hdmi.h"

#include "mt_unf_disp.h"
#include "drv_disp_ext.h"
#include "mt_kernel_adapt.h"
//#include "drv_cipher_ext.h"
#include "mt_drv_sys.h"

#include "drv_global.h"
#include "drv_reg_proc.h"
#include "drv_compatibility.h"
#include "mt_drv_module.h"
#include "si_misc.h"
#include "si_lib_seq_api.h"
#include "si_hdmitx.h"
#include "si_drv_tx_api.h"
#include "si_lib_edid_api.h"
#include "si_app_cec.h"
#include "si_drv_cpi_api.h"
#include "si_drv_tx_regs.h"
#include "si_vidpath_regs.h"
#include "mt_drv_mmz.h"
#include "mt_drv_dma.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_drv_analog.h"
#define NO_DUMP_HDMI_STATE
#include "analog/mt_analog_hdmi.h"
#include "analog/mt_analog_parameter.h"

#ifdef ANDROID_SUPPORT
	#include <linux/switch.h>
	extern MT_BOOL g_switchOk;
	extern struct switch_dev hdmi_tx_sdev;
#endif

#define HDCP_KEY_CHECK_OK 0xa
#define SII_INFOFRAME_AVI_MAX_LEN 17

typedef enum hiHDMI_VIDEO_TIMING_E {
	VIDEO_TIMING_UNKNOWN,
	VIDEO_TIMING_640X480P_59940,        /* 1: 640x480p  @ 59.94Hz  No Repetition */
	VIDEO_TIMING_640X480P_60000,        /* 1: 640x480p  @ 60Hz     No Repetition */
	#if defined (DVI_SUPPORT)
	VIDEO_TIMING_720X480P_59940,        /* 2: 720x480p @ 59.94Hz  No Repetition */
	#endif
	VIDEO_TIMING_720X480P_60000,        /* 2: 720x480p  @ 60Hz     No Repetition */
	#if defined (DVI_SUPPORT)
	VIDEO_TIMING_1280X720P_59940,       /* 4: 1280x720p @ 59.94Hz  No Repetition */
	#endif
	VIDEO_TIMING_1280X720P_60000,       /* 4: 1280x720p @ 60Hz     No Repetition */
	VIDEO_TIMING_1920X1080I_60000,      /* 5: 1920x1080i@ 59.94Hz  No Repetition */
	#if defined (DVI_SUPPORT)
	VIDEO_TIMING_1920X1080I_59940,      /* 5: 1920x1080i@ 60Hz     No Repetition */
	VIDEO_TIMING_720X480I_59940,        /* 6: 720x480i  @ 59.94Hz  pixel sent 2 times */
	#endif
	VIDEO_TIMING_720X480I_60000,        /* 6: 720x480i  @ 60Hz     pixel sent 2 times */
	#if defined (DVI_SUPPORT)
	VIDEO_TIMING_720X240P_59940,        /* 8: 720x240p  @ 59.94Hz  pixel sent 2 times */
	VIDEO_TIMING_720X240P_60000,        /* 8: 720x240p  @ 60Hz     pixel sent 2 times */
	VIDEO_TIMING_2880X480I_59940,       /* 10:2880x480i @ 59.94Hz  pixel sent 1 to 10 times */
	VIDEO_TIMING_2880X480I_60000,       /* 10:2880x480i @ 60Hz     pixel sent 1 to 10 times */
	VIDEO_TIMING_2880X240P_59940,       /* 12:2880x240p @ 59.94Hz  pixel sent 1 to 10 timesn */
	VIDEO_TIMING_2880X240P_60000,       /* 12:2880x240p @ 60Hz     pixel sent 1 to 10 times */
	VIDEO_TIMING_1440X480P_59940,       /* 14:1440x480p @ 59.94Hz  pixel sent 1 to 2 times */
	VIDEO_TIMING_1440X480P_60000,       /* 14:1440x480p @ 60Hz     pixel sent 1 to 2 times */
	VIDEO_TIMING_1920X1080P_59940,      /* 16:1920x1080p@ 59.94Hz  No Repetition */
	#endif
	VIDEO_TIMING_1920X1080P_60000,      /* 16:1920x1080p@ 60Hz     No Repetition */
	VIDEO_TIMING_720X576P_50000,        /* 17:720x576p  @ 50Hz     No Repetition */
	VIDEO_TIMING_1280X720P_50000,       /* 19:1280x720p @ 50Hz     No Repetition */
	VIDEO_TIMING_1920X1080I_50000,      /* 20:1920x1080i@ 50Hz     No Repetition */
	VIDEO_TIMING_720X576I_50000,        /* 21:720x576i  @ 50Hz     pixel sent 2 times */
	#if defined (DVI_SUPPORT)
	VIDEO_TIMING_720X288P_50000,        /* 23:720x288p @ 50Hz      pixel sent 2 times */
	VIDEO_TIMING_2880X576I_50000,       /* 25:2880x576i @ 50Hz     pixel sent 1 to 10 times */
	VIDEO_TIMING_2880X288P_50000,       /* 27:2880x288p @ 50Hz     pixel sent 1 to 10 times */
	VIDEO_TIMING_1440X576P_50000,       /* 29:1440x576p @ 50Hz     pixel sent 1 to 2 times */
	#endif
	VIDEO_TIMING_1920X1080P_50000,      /* 31:1920x1080p @ 50Hz    No Repetition */
	#if defined (DVI_SUPPORT)
	VIDEO_TIMING_1920X1080P_23980,      /* 32:1920x1080p @ 23.98Hz No Repetition */
	#endif
	VIDEO_TIMING_1920X1080P_24000,      /* 32:1920x1080p @ 24Hz   No Repetition */
	VIDEO_TIMING_1920X1080P_25000,      /* 33:1920x1080p @ 25Hz    No Repetition */
	#if defined (DVI_SUPPORT)
	VIDEO_TIMING_1920X1080P_29980,      /* 34:1920x1080p @ 29.98Hz No Repetition */
	#endif
	VIDEO_TIMING_1920X1080P_30000,      /* 34:1920x1080p @ 30Hz    No Repetition */
	#if defined (DVI_SUPPORT)
	VIDEO_TIMING_2880X480P_59940,       /* 35:2880x480p @ 59.94Hz  pixel sent 1, 2 or 4 times */
	VIDEO_TIMING_2880X480P_60000,       /* 35:2880x480p @ 60Hz     pixel sent 1, 2 or 4 times */
	VIDEO_TIMING_2880X576P_50000,       /* 37:2880x576p @ 50Hz     pixel sent 1, 2 or 4 times*/
	#endif
	VIDEO_TIMING_3840X2160P_24000,      /* 93:3840x2160p @ 24Hz   No Repetition */
	VIDEO_TIMING_3840X2160P_25000,      /* 94:3840x2160p @ 25Hz   No Repetition */
	VIDEO_TIMING_3840X2160P_30000,      /* 95:3840x2160p @ 30Hz   No Repetition */
	VIDEO_TIMING_3840X2160P_50000,      /* 96:3840x2160p @ 50Hz   No Repetition */
	VIDEO_TIMING_3840X2160P_60000,      /* 97:3840x2160p @ 60Hz   No Repetition */
	VIDEO_TIMING_4096X2160P_24000,      /* 98:4096x2160p @ 24Hz   No Repetition */
	VIDEO_TIMING_4096X2160P_25000,      /* 99:4096x2160p @ 25Hz   No Repetition */
	VIDEO_TIMING_4096X2160P_30000,      /* 100:4096x2160p @ 30Hz   No Repetition */
	VIDEO_TIMING_4096X2160P_50000,      /* 101:4096x2160p @ 50Hz   No Repetition */
	VIDEO_TIMING_4096X2160P_60000,      /* 102:4096x2160p @ 60Hz   No Repetition */
	VIDEO_TIMING_MAX
} DRV_HDMI_VIDEO_TIMING_E;

enum {
	HDMI_CALLBACK_NULL,
	HDMI_CALLBACK_USER,
	HDMI_CALLBACK_KERNEL
};

typedef struct {
	mt_u32                      VidIdCode;
	DRV_HDMI_VIDEO_TIMING_E     Mode;
	mt_u32                      FrameRate;
	mt_u32                      Active_X;
	mt_u32                      Active_Y;
	mt_u32                      Active_W;
	mt_u32                      Active_H;
	VIDEO_SAMPLE_TYPE_E  ScanType;
	MT_UNF_HDMI_ASPECT_RATIO_E       AspectRatio;
	mt_u32                      PixelRepetition;
} hdmi_VideoIdentification_t;

static hdmi_VideoIdentification_t  VideoCodes[] = {
	/* {Video Identification Code, Timing Mode, x, y, w, h, Frame Rate, Scan Type, Aspect Ratio, Pixel Repetition} */
	{0,  VIDEO_TIMING_UNKNOWN,              0, 0, 0,    0,    0, VIDEO_SAMPLE_TYPE_UNKNOWN,      MT_UNF_HDMI_ASPECT_RATIO_NO_DATA, 0},
	{1,  VIDEO_TIMING_640X480P_59940,   59940, 0, 0,  640,  480, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_4TO3,  0},
	{1,  VIDEO_TIMING_640X480P_60000,   60000, 0, 0,  640,  480, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_4TO3,  0},
	#if defined (DVI_SUPPORT)
	{2,  VIDEO_TIMING_720X480P_59940,   59940, 0, 0,  720,  480, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_4TO3,  0},
	#endif
	{2,  VIDEO_TIMING_720X480P_60000,   60000, 0, 0,  720,  480, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_4TO3,  0},
	#if defined (DVI_SUPPORT)
	{4,  VIDEO_TIMING_1280X720P_59940,  59940, 0, 0, 1280,  720, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9,  0},
	#endif
	{4,  VIDEO_TIMING_1280X720P_60000,  60000, 0, 0, 1280,  720, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{5,  VIDEO_TIMING_1920X1080I_60000, 60000, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_INTERLACE,    MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	#if defined (DVI_SUPPORT)
	{5,  VIDEO_TIMING_1920X1080I_59940, 59940, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_INTERLACE,    MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{6,  VIDEO_TIMING_720X480I_59940,   59940, 0, 0,  720,  480, VIDEO_SAMPLE_TYPE_INTERLACE,    MT_UNF_HDMI_ASPECT_RATIO_4TO3, 1},
	#endif
	{6,  VIDEO_TIMING_720X480I_60000,   60000, 0, 0,  720,  480, VIDEO_SAMPLE_TYPE_INTERLACE,    MT_UNF_HDMI_ASPECT_RATIO_4TO3, 1},
	#if defined (DVI_SUPPORT)
	{8,  VIDEO_TIMING_720X240P_59940,   59940, 0, 0,  720,  240, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 1},
	{8,  VIDEO_TIMING_720X240P_60000,   60000, 0, 0,  720,  240, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/,  1},
	{10, VIDEO_TIMING_2880X480I_59940,  59940, 0, 0, 2280,  480, VIDEO_SAMPLE_TYPE_INTERLACE,    MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/,  1/*1 to 10 times*/},
	{10, VIDEO_TIMING_2880X480I_60000,  60000, 0, 0, 2280,  480, VIDEO_SAMPLE_TYPE_INTERLACE,    MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 1/*1 to 10 times*/},
	{12, VIDEO_TIMING_2880X240P_59940,  59940, 0, 0, 2280,  240, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 1/*1 to 10 times*/},
	{12, VIDEO_TIMING_2880X240P_60000,  60000, 0, 0, 2280,  240, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 1/*1 to 10 times*/},
	{14, VIDEO_TIMING_1440X480P_59940,  59940, 0, 0, 1440,  480, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 1/*1 to 2 times*/},
	{14, VIDEO_TIMING_1440X480P_60000,  60000, 0, 0, 1440,  480, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/,  1/*1 to 2 times*/},
	{16, VIDEO_TIMING_1920X1080P_59940, 59940, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	#endif
	{16, VIDEO_TIMING_1920X1080P_60000, 60000, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{17, VIDEO_TIMING_720X576P_50000,   50000, 0, 0,  720,  576, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_4TO3, 0},
	{19, VIDEO_TIMING_1280X720P_50000,  50000, 0, 0, 1280,  720, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{20, VIDEO_TIMING_1920X1080I_50000, 50000, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_INTERLACE,    MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{21, VIDEO_TIMING_720X576I_50000,   50000, 0, 0,  720,  576, VIDEO_SAMPLE_TYPE_INTERLACE,    MT_UNF_HDMI_ASPECT_RATIO_4TO3,  1},
	#if defined (DVI_SUPPORT)
	{23, VIDEO_TIMING_720X288P_50000,   50000, 0, 0,  720,  288, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 1},
	{25, VIDEO_TIMING_2880X576I_50000,  50000, 0, 0, 2880,  576, VIDEO_SAMPLE_TYPE_INTERLACE,    MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 1/*1 to 10 times*/},
	{27, VIDEO_TIMING_2880X288P_50000,  50000, 0, 0, 2880,  288, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 1/*1 to 10 times*/},
	{29, VIDEO_TIMING_1440X576P_50000,  50000, 0, 0, 1440,  576, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 1/*1 to 2 times*/},
	#endif
	{31, VIDEO_TIMING_1920X1080P_50000, 50000, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	#if defined (DVI_SUPPORT)
	{32, VIDEO_TIMING_1920X1080P_23980, 23980, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	#endif
	{32, VIDEO_TIMING_1920X1080P_24000, 24000, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{33, VIDEO_TIMING_1920X1080P_25000, 25000, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	#if defined (DVI_SUPPORT)
	{34, VIDEO_TIMING_1920X1080P_29980, 29980, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	#endif
	{34, VIDEO_TIMING_1920X1080P_30000, 30000, 0, 0, 1920, 1080, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	#if defined (DVI_SUPPORT)
	{35, VIDEO_TIMING_2880X480P_59940,  59940, 0, 0, 2280,  480, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 0/*1, 2 or 4 times*/},
	{35, VIDEO_TIMING_2880X480P_60000,  60000, 0, 0, 2280,  480, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 0/*1, 2 or 4 times*/},
	{37, VIDEO_TIMING_2880X576P_50000,  50000, 0, 0, 2280,  576, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_NO_DATA/*?*/, 0/*1, 2 or 4 times*/},
	#endif
	{93, VIDEO_TIMING_3840X2160P_24000,	24000, 0, 0, 3840,	2160, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{94, VIDEO_TIMING_3840X2160P_25000,	25000, 0, 0, 3840,	2160, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{95, VIDEO_TIMING_3840X2160P_30000,	30000, 0, 0, 3840,	2160, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{96, VIDEO_TIMING_3840X2160P_50000,	50000, 0, 0, 3840,	2160, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{97, VIDEO_TIMING_3840X2160P_60000,	60000, 0, 0, 3840,	2160, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{98, VIDEO_TIMING_4096X2160P_24000,	24000, 0, 0, 4096,	2160, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{99, VIDEO_TIMING_4096X2160P_25000,	25000, 0, 0, 4096,	2160, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{100, VIDEO_TIMING_4096X2160P_30000,	30000, 0, 0, 4096,	2160, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{101, VIDEO_TIMING_4096X2160P_50000,	50000, 0, 0, 4096,	2160, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
	{102, VIDEO_TIMING_4096X2160P_60000,	60000, 0, 0, 4096,	2160, VIDEO_SAMPLE_TYPE_PROGRESSIVE,  MT_UNF_HDMI_ASPECT_RATIO_16TO9, 0},
};

#define HDMI_CHECK_NULL_PTR(ptr)  do{                         \
		if (NULL == (ptr))                                    \
		{                                                     \
			return MT_ERR_HDMI_NUL_PTR;                       \
		}                                                     \
	}while(0)

#define HDMI_CHECK_ID(l_enHdmi)  do{                          \
		if ((l_enHdmi) >= MT_UNF_HDMI_ID_BUTT)                \
		{                                                     \
			COM_INFO("enHdmi %d is invalid.\n", l_enHdmi); \
			return MT_ERR_HDMI_INVALID_PARA;                  \
		}                                                     \
	}while(0)

#define HDMI_CheckChnOpen(l_HdmiID) do{                       \
		if (!DRV_Get_IsChnOpened(l_HdmiID))      \
		{                                                     \
			COM_INFO("enHdmi %d is NOT open.\n", l_HdmiID);\
			return MT_ERR_HDMI_DEV_NOT_OPEN;                  \
		}                                                     \
	}while(0)

#define HDMI_CheckChnOpenNoPrint(l_HdmiID) do{                       \
		if (!DRV_Get_IsChnOpened(l_HdmiID)) 	 \
		{													  \
			return MT_ERR_HDMI_DEV_NOT_OPEN;				  \
		}													  \
	}while(0)

static mt_u32 hdmi_Mutex_Event_Count = 0;
MT_DECLARE_MUTEX(g_HDMIEventMutex);
#define HDMI_EVENT_LOCK()                                     \
	do{                                                           \
		hdmi_Mutex_Event_Count ++;                                \
		/*MT_INFO_HDMI("hdmi_Mutex_Event_Count:%d\n", hdmi_Mutex_Count);*/  \
		if (down_interruptible(&g_HDMIEventMutex))                \
		{}                                                         \
	}while(0)                                                     \

#define HDMI_EVENT_UNLOCK()                                   \
	do{                                                           \
		hdmi_Mutex_Event_Count --;                                \
		/*MT_INFO_HDMI("hdmi_Mutex_Event_Count:%d\n", hdmi_Mutex_Count); */ \
		up(&g_HDMIEventMutex);                                    \
	}while(0)                                                     \

MT_DECLARE_MUTEX(g_HDMIAttrMutex);
#define HDMI_ATTR_LOCK()                                     \
	do{                                                           \
		if ( !in_atomic() ) {\
			if (down_interruptible(&g_HDMIAttrMutex))                \
				;                                                         \
		}\
	}while(0)                                                     \

#define HDMI_ATTR_UNLOCK()                                   \
	do{                                                           \
		if ( !in_atomic() ) {\
			up(&g_HDMIAttrMutex);                                     \
		}\
	}while(0)                                                     \

MT_DECLARE_MUTEX(g_HDMIAvparamsMutex);
#define HDMI_AVPARAMS_LOCK()                                     \
	do{ 														  \
		if ( !in_atomic() ) {\
			if (down_interruptible(&g_HDMIAvparamsMutex))				 \
				;														  \
		}\
	}while(0);													  \

#define HDMI_AVPARAMS_UNLOCK()                                   \
	do{ 														  \
		if ( !in_atomic() ) { \
			up(&g_HDMIAvparamsMutex);									  \
		}\
	}while(0);													  \

static mt_u32 hdmi_Create_AVI_Infoframe(MT_UNF_HDMI_AVI_INFOFRAME_VER2_S *punAVIInfoFrame, MT_U8 *pu8AviInfoFrame);
static mt_u32 hdmi_Create_Audio_Infoframe(MT_UNF_HDMI_AUD_INFOFRAME_VER1_S *punAUDInfoFrame, MT_U8 *pu8AudioInfoFrame);
//static mt_u32 hdmi_SetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_ATTR_S *pstHDMIAttr, MT_BOOL UpdateFlag);
static mt_u32 hdmi_SetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame);
static mt_u32 hdmi_GetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame);
static mt_u32 hdmi_AdjustAVIInfoFrame(MT_UNF_HDMI_ID_E enHdmi);
static mt_u32 hdmi_AdjustVSDBInfoFrame(MT_UNF_HDMI_ID_E enHdmi);
static mt_u32 hdmi_AdjustAUDInfoFrame(MT_UNF_HDMI_ID_E enHdmi);
static mt_void hdmi_SetAndroidState(mt_s32 PlugState);
mt_void DRV_HDMI_Vcfg_Status(mt_u32 sts);
mt_u32 DRV_HDMI_Get_Update_Tvsys(mt_void);
mt_void DRV_HDMI_Set_Update_Tvsys(mt_u32 flg);
static mt_void hdmi_set_infoframe_cksum(info_struct_t *info);

static void hdmi_ProcEvent(MT_UNF_HDMI_EVENT_TYPE_E event, mt_u32 procID);
mt_s32 DRV_HDMI_HdcpMute(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bAvMute);

static mt_u32 g_HDMIWaitFlag[MAX_PROCESS_NUM];
static wait_queue_head_t g_astHDMIWait;

//One Proccess can only init/open one HDMI Instance!
static mt_u32 g_HDMIKernelInitNum = 0; //Kernel HDMI Init Num
static mt_u32 g_HDMIUserInitNum   = 0; //User HDMI Init Num
static mt_u32 g_HDMIOpenNum       = 0; //HDMI Open Num
static mt_u32 g_UserCallbackFlag  = HDMI_CALLBACK_NULL;
//static mt_u32 g_UCallbackAddr     = 0; //save callback pointer Address

mt_u32 unStableTimes = 0;
SiiInst_t sInstTx_Drv			= SII_INST_NULL;
static SiiDrvTxConfig_t   sTxConfig;
static mt_u32 g_hdmi_sw_reset = 0;
extern SiiLibTimeMilli_t g_tvformat_time[2];

extern unsigned int suspend_flag;
extern unsigned int mt_suspend_flag;
extern void hdmi_MCE_ProcHotPlug(MT_UNF_HDMI_ID_E hHdmi);
extern ulong HdmiDrvRegBaseGet(void *p, HDMI_RELATED_REG_MOD_E mod);
extern ulong HdmiDrvRegPhyBaseGet(void *p, HDMI_RELATED_REG_MOD_E mod);
extern HDMI20_EMP_MODE_T hdmi20_emp_mode_get(void);
extern mmz_buffer_s hdmi20_emp_dma_buff_get(u32 idx);
extern struct hdmi20_params *hdmi20_params_get(void);
extern void SI_TX_SetHDMIMode(MT_U8 Enabled);
extern void mta_hdmi_clk_cfg_v2(mt_u32 tmds_clk, mt_u32 ssc_mode, mt_u32 pd_pi_mode, mt_u32 gate_vclk, mt_u32 os_clk);
#if HDMI_EMP_VERIFY_TEST
	#include "drv_hdmi_emp_test.h"
#else
	#define DRV_HDMI_Emp_Assert_Endpix(a,b)
	#define DRV_HDMI_Emp_Assert_Delaycnt()
	#define DRV_HDMI_Emp_Assert_Data(a,b)
	#define DRV_HDMI_Debug_Func()
#endif

SiiInst_t DRV_HDMI_Get_TxInst(void)
{
	return sInstTx_Drv;
}

#include "drv_hdmi_si_adapter.c"
bool_t getScdcEnable(void)
{
	return sTxConfig.bScdcEn;
}

mt_s32 Hdmi_KThread_Timer(void* pParm)
{
	while ( 1 ) {
		if (kthread_should_stop()) {
			break;
		}

		if (!siiIsTClockStable() && (!DRV_Get_IsThreadStoped()) && !SI_IsHDMIResetting()) {
			unStableTimes++;
		}

		if (!DRV_Get_IsChnOpened(MT_UNF_HDMI_ID_0) || DRV_Get_IsThreadStoped() || SI_IsHDMIResetting() || suspend_flag) {
			SiiLibTimeMilliDelay(10);
			continue;
		}

		DRV_HDMI_Debug_Func();
		#ifndef __HDMI_INTERRUPT__
		SI_TimerHandler();
		#endif
		SiiLibTimeMilliDelay(1);
	}

	return MT_SUCCESS;
}

#if defined (CEC_SUPPORT)
mt_s32 Hdmi_KThread_CEC(void* pParm)
{
	//pParm = pParm;

	while ( 1 ) {
		if (kthread_should_stop()) {
			break;
		}

		/* HDMI do not start, Just sleep */
		if (!DRV_Get_IsCECEnable(MT_UNF_HDMI_ID_0)) {
			SiiLibTimeMilliDelay(100);
			continue;
		}

		if (!DRV_Get_IsCECStart(MT_UNF_HDMI_ID_0)) {
			mt_u32 u32Status = 0, index;
			MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
			MT_UNF_HDMI_CEC_STATUS_S     *pstCECStatus;
			//HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();

			{
				uint8_t Plug_Staus1;
				SiiDrvTx_Get_Plug_Status(DRV_HDMI_Get_TxInst(), &Plug_Staus1);
				//printk(KERN_EMERG "plug status:%x\n",Plug_Staus1);
				if (0==(Plug_Staus1&0x5)) {
					SiiLibTimeMilliDelay(200);
					continue;
				}
			}
			#if 0
			/* Only Check CEC 5 time */
			if (pstChnAttr[MT_UNF_HDMI_ID_0].u8CECCheckCount < 100) {
				pstChnAttr[MT_UNF_HDMI_ID_0].u8CECCheckCount ++;
			} else {
				/* NO CEC Response, Just sleep */
				SiiLibTimeMilliDelay(100);
				continue;
			}
			#endif
			/*We need to do AutoPing */
			SI_CEC_AudioPing(&u32Status);
			u32Status = 0x01;
			{	//read edid.
				uint8_t flg = 0;
				SiiDrvTxReadEDIDFromSink(DRV_HDMI_Get_TxInst(), (uint8_t *)(&flg));
			}
			HDMI_DEBUG_TRACE("do autoping\n");
			if (0x01 == (u32Status & 0x01)) {
				//Build up CEC Status!
				if (!DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
					continue;
				}

				//Physical Address
				pstCECStatus = DRV_Get_CecStatus(MT_UNF_HDMI_ID_0);

				if (!pSinkCap->stCECAddr.u8PhyAddrA) {
					continue; //Bad CEC Phaycail Address
				}

				pstCECStatus->u8PhysicalAddr[0] = pSinkCap->stCECAddr.u8PhyAddrA;
				pstCECStatus->u8PhysicalAddr[1] = pSinkCap->stCECAddr.u8PhyAddrB;
				pstCECStatus->u8PhysicalAddr[2] = pSinkCap->stCECAddr.u8PhyAddrC;
				pstCECStatus->u8PhysicalAddr[3] = pSinkCap->stCECAddr.u8PhyAddrD;

				//CEC Network
				for (index = 0; index < MT_UNF_CEC_LOGICALADD_BUTT; index ++) {
					if ((u32Status & (1 << index)) != 0) {
						pstCECStatus->u8Network[index] = MT_TRUE;
						CEC_INFO("cec ping logAddr[0x%02x] Ack \n", index);
					} else {
						CEC_INFO("cec ping logAddr[0x%02x] No Ack \n", index);
					}
				}

				//Logical Address
				if (pstCECStatus->u8Network[MT_UNF_CEC_LOGICALADD_TUNER_1] == MT_TRUE) {         //bit3
					if (pstCECStatus->u8Network[MT_UNF_CEC_LOGICALADD_TUNER_2] == MT_TRUE) {     //bit6
						if (pstCECStatus->u8Network[MT_UNF_CEC_LOGICALADD_TUNER_3] == MT_TRUE) { //bit7
							if (pstCECStatus->u8Network[MT_UNF_CEC_LOGICALADD_TUNER_4] == MT_TRUE) { //bit10
								pstCECStatus->u8LogicalAddr = 0x0f;    //Brocast Address!
							} else {
								pstCECStatus->u8LogicalAddr = MT_UNF_CEC_LOGICALADD_TUNER_4;
							}
						} else {
							pstCECStatus->u8LogicalAddr = MT_UNF_CEC_LOGICALADD_TUNER_3;
						}
					} else {
						pstCECStatus->u8LogicalAddr = MT_UNF_CEC_LOGICALADD_TUNER_2;
					}
				} else {
					pstCECStatus->u8LogicalAddr = MT_UNF_CEC_LOGICALADD_TUNER_1;
				}

				pstCECStatus->bEnable =  MT_TRUE;
				CEC_INFO("CEC is build up *****\n");

				//SI_CEC_Open(pstCECStatus->u8LogicalAddr);
				SI_CEC_Open();
				//Should send out Brocast messsage of <Report Physical Address> !
				#if defined (CEC_SUPPORT)
				SI_CEC_Enum(1, 1);
				#endif
				//Should send out Brocast message of <Vendor Device ID>!
				DRV_Set_CECStart(MT_UNF_HDMI_ID_0, MT_TRUE);
				CEC_INFO("\n-------CEC Started-------\n");
			}

			SiiLibTimeMilliDelay(100);
			continue;
		} else {
			//SI_CEC_Event_Handler();
			Si_CEC_Start(1);
			SiiLibTimeMilliDelay(50);
		}

		SiiLibTimeMilliDelay(100);
	}
	return MT_SUCCESS;
}

#endif

static hdmi_VideoIdentification_t * hdmi_GetVideoCode(MT_DRV_DISP_FMT_E enTimingMode)
{
	mt_u32 Index, VideoTimingMode;

	switch (enTimingMode) {
		case MT_DRV_DISP_FMT_3840X2160_24:
			VideoTimingMode = VIDEO_TIMING_3840X2160P_24000;
			break;
		case MT_DRV_DISP_FMT_3840X2160_25:
			VideoTimingMode = VIDEO_TIMING_3840X2160P_25000;
			break;
		case MT_DRV_DISP_FMT_3840X2160_30:
			VideoTimingMode = VIDEO_TIMING_3840X2160P_30000;
			break;
		case MT_DRV_DISP_FMT_3840X2160_50:
			VideoTimingMode = VIDEO_TIMING_3840X2160P_50000;
			break;
		case MT_DRV_DISP_FMT_3840X2160_60:
			VideoTimingMode = VIDEO_TIMING_3840X2160P_60000;
			break;
		case MT_DRV_DISP_FMT_4096X2160_24:
			VideoTimingMode = VIDEO_TIMING_4096X2160P_24000;
			break;
		case MT_DRV_DISP_FMT_4096X2160_25:
			VideoTimingMode = VIDEO_TIMING_4096X2160P_25000;
			break;
		case MT_DRV_DISP_FMT_4096X2160_30:
			VideoTimingMode = VIDEO_TIMING_4096X2160P_30000;
			break;
		case MT_DRV_DISP_FMT_4096X2160_50:
			VideoTimingMode = VIDEO_TIMING_4096X2160P_50000;
			break;
		case MT_DRV_DISP_FMT_4096X2160_60:
			VideoTimingMode = VIDEO_TIMING_4096X2160P_60000;
			break;
		case MT_DRV_DISP_FMT_1080P_60:
			VideoTimingMode = VIDEO_TIMING_1920X1080P_60000;
			break;
		case MT_DRV_DISP_FMT_1080P_50:
			VideoTimingMode = VIDEO_TIMING_1920X1080P_50000;
			break;
		case MT_DRV_DISP_FMT_1080P_30:
			VideoTimingMode = VIDEO_TIMING_1920X1080P_30000;
			break;
		case MT_DRV_DISP_FMT_1080P_25:
			VideoTimingMode = VIDEO_TIMING_1920X1080P_25000;
			break;
		case MT_DRV_DISP_FMT_1080P_24:
			VideoTimingMode = VIDEO_TIMING_1920X1080P_24000;
			break;
		case MT_DRV_DISP_FMT_1080i_60:
			VideoTimingMode = VIDEO_TIMING_1920X1080I_60000;
			break;
		case MT_DRV_DISP_FMT_1080i_50:
			VideoTimingMode = VIDEO_TIMING_1920X1080I_50000;
			break;
		case MT_DRV_DISP_FMT_720P_60:
			VideoTimingMode = VIDEO_TIMING_1280X720P_60000;
			break;
		case MT_DRV_DISP_FMT_720P_50:
			VideoTimingMode = VIDEO_TIMING_1280X720P_50000;
			break;
		case MT_DRV_DISP_FMT_576P_50:
			VideoTimingMode = VIDEO_TIMING_720X576P_50000;
			break;
		case MT_DRV_DISP_FMT_480P_60:
			VideoTimingMode = VIDEO_TIMING_720X480P_60000;
			break;
		case MT_DRV_DISP_FMT_PAL:
		case MT_DRV_DISP_FMT_PAL_B:
		case MT_DRV_DISP_FMT_PAL_B1:
		case MT_DRV_DISP_FMT_PAL_D:
		case MT_DRV_DISP_FMT_PAL_D1:
		case MT_DRV_DISP_FMT_PAL_G:
		case MT_DRV_DISP_FMT_PAL_H:
		case MT_DRV_DISP_FMT_PAL_K:
		case MT_DRV_DISP_FMT_PAL_I:
		case MT_DRV_DISP_FMT_PAL_M:
		case MT_DRV_DISP_FMT_PAL_N:
		case MT_DRV_DISP_FMT_PAL_Nc:
		case MT_DRV_DISP_FMT_PAL_60:

		case MT_DRV_DISP_FMT_SECAM_SIN:
		case MT_DRV_DISP_FMT_SECAM_COS:
		case MT_DRV_DISP_FMT_SECAM_L:
		case MT_DRV_DISP_FMT_SECAM_B:
		case MT_DRV_DISP_FMT_SECAM_G:
		case MT_DRV_DISP_FMT_SECAM_D:
		case MT_DRV_DISP_FMT_SECAM_K:
		case MT_DRV_DISP_FMT_SECAM_H:
			VideoTimingMode = VIDEO_TIMING_720X576I_50000;
			break;
		case MT_DRV_DISP_FMT_NTSC:
		case MT_DRV_DISP_FMT_NTSC_J:
		case MT_DRV_DISP_FMT_NTSC_443:
			VideoTimingMode = VIDEO_TIMING_720X480I_60000;
			break;
		case MT_DRV_DISP_FMT_861D_640X480_60:
			VideoTimingMode = VIDEO_TIMING_640X480P_60000;
			break;
		default:
			COM_INFO("Non CEA video timing:%d\n", enTimingMode);
			// 4k2k && vesa
			VideoTimingMode = VIDEO_TIMING_UNKNOWN;
			break;
	}

	for (Index = 0; Index < VIDEO_TIMING_MAX; Index ++) {
		if (VideoCodes[Index].Mode == VideoTimingMode) {
			break;
		}
	}

	if (Index >= VIDEO_TIMING_MAX) {
		Index = 0;
	}

	COM_INFO("Get Video Code index:%d, Mode:0x%x, VidIdCode:0x%x\n", Index, VideoCodes[Index].Mode, VideoCodes[Index].VidIdCode);
	return (&(VideoCodes[Index]));
}

MT_UNF_ENC_FMT_E hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_E SrcFmt)
{
	MT_UNF_ENC_FMT_E dstFmt = MT_UNF_ENC_FMT_BUTT;
	switch (SrcFmt) {
		case MT_DRV_DISP_FMT_1080P_60 :
			dstFmt = MT_UNF_ENC_FMT_1080P_60;
			break;
		case MT_DRV_DISP_FMT_1080P_50 :
			dstFmt = MT_UNF_ENC_FMT_1080P_50;
			break;
		case MT_DRV_DISP_FMT_1080P_30 :
			dstFmt = MT_UNF_ENC_FMT_1080P_30;
			break;
		case MT_DRV_DISP_FMT_1080P_25 :
			dstFmt = MT_UNF_ENC_FMT_1080P_25;
			break;
		case MT_DRV_DISP_FMT_1080P_24 :
		case MT_DRV_DISP_FMT_1080P_24_FP:
			dstFmt = MT_UNF_ENC_FMT_1080P_24;
			break;
		case MT_DRV_DISP_FMT_1080i_60 :
			dstFmt = MT_UNF_ENC_FMT_1080i_60;
			break;
		case MT_DRV_DISP_FMT_1080i_50 :
			dstFmt = MT_UNF_ENC_FMT_1080i_50;
			break;
		case MT_DRV_DISP_FMT_720P_60 :
		case MT_DRV_DISP_FMT_720P_60_FP:
			dstFmt = MT_UNF_ENC_FMT_720P_60;
			break;
		case MT_DRV_DISP_FMT_720P_50 :
		case MT_DRV_DISP_FMT_720P_50_FP:
			dstFmt = MT_UNF_ENC_FMT_720P_50;
			break;
		case MT_DRV_DISP_FMT_576P_50 :
			dstFmt = MT_UNF_ENC_FMT_576P_50;
			break;
		case MT_DRV_DISP_FMT_480P_60 :
			dstFmt = MT_UNF_ENC_FMT_480P_60;
			break;
		case MT_DRV_DISP_FMT_PAL:
		case MT_DRV_DISP_FMT_PAL_B:
		case MT_DRV_DISP_FMT_PAL_B1:
		case MT_DRV_DISP_FMT_PAL_D:
		case MT_DRV_DISP_FMT_PAL_D1:
		case MT_DRV_DISP_FMT_PAL_G:
		case MT_DRV_DISP_FMT_PAL_H:
		case MT_DRV_DISP_FMT_PAL_K:
		case MT_DRV_DISP_FMT_PAL_I:
		case MT_DRV_DISP_FMT_PAL_M:
		case MT_DRV_DISP_FMT_PAL_N:
		case MT_DRV_DISP_FMT_PAL_Nc:
		case MT_DRV_DISP_FMT_PAL_60:
		case MT_DRV_DISP_FMT_1440x576i_50:

		case MT_DRV_DISP_FMT_SECAM_SIN:
		case MT_DRV_DISP_FMT_SECAM_COS:
		case MT_DRV_DISP_FMT_SECAM_L:
		case MT_DRV_DISP_FMT_SECAM_B:
		case MT_DRV_DISP_FMT_SECAM_G:
		case MT_DRV_DISP_FMT_SECAM_D:
		case MT_DRV_DISP_FMT_SECAM_K:
		case MT_DRV_DISP_FMT_SECAM_H:
			dstFmt = MT_UNF_ENC_FMT_PAL;
			break;
		case MT_DRV_DISP_FMT_NTSC :
		case MT_DRV_DISP_FMT_NTSC_J :
		case MT_DRV_DISP_FMT_NTSC_443 :
		case MT_DRV_DISP_FMT_1440x480i_60:
			dstFmt = MT_UNF_ENC_FMT_NTSC;
			break;
		case MT_DRV_DISP_FMT_861D_640X480_60 :
			dstFmt = MT_UNF_ENC_FMT_861D_640X480_60;
			break;
		case MT_DRV_DISP_FMT_VESA_800X600_60 :
			dstFmt = MT_UNF_ENC_FMT_VESA_800X600_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1024X768_60 :
			dstFmt = MT_UNF_ENC_FMT_VESA_1024X768_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1280X720_60 :
			dstFmt = MT_UNF_ENC_FMT_VESA_1280X720_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1280X800_60 :
			dstFmt = MT_UNF_ENC_FMT_VESA_1280X800_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1280X1024_60 :
			dstFmt = MT_UNF_ENC_FMT_VESA_1280X1024_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1360X768_60 :
			dstFmt = MT_UNF_ENC_FMT_VESA_1360X768_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1366X768_60 :
			dstFmt = MT_UNF_ENC_FMT_VESA_1366X768_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1400X1050_60:
			dstFmt = MT_UNF_ENC_FMT_VESA_1400X1050_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1440X900_60:
			dstFmt = MT_UNF_ENC_FMT_VESA_1440X900_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1440X900_60_RB:
			dstFmt = MT_UNF_ENC_FMT_VESA_1440X900_60_RB;
			break;
		case MT_DRV_DISP_FMT_VESA_1600X900_60_RB:
			dstFmt = MT_UNF_ENC_FMT_VESA_1600X900_60_RB;
			break;
		case MT_DRV_DISP_FMT_VESA_1600X1200_60:
			dstFmt = MT_UNF_ENC_FMT_VESA_1600X1200_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1680X1050_60:
			dstFmt = MT_UNF_ENC_FMT_VESA_1680X1050_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1920X1080_60:
			dstFmt = MT_UNF_ENC_FMT_VESA_1920X1080_60;
			break;
		case MT_DRV_DISP_FMT_VESA_1920X1200_60:
			dstFmt = MT_UNF_ENC_FMT_VESA_1920X1200_60;
			break;
		case MT_DRV_DISP_FMT_VESA_2048X1152_60:
			dstFmt = MT_UNF_ENC_FMT_VESA_2048X1152_60;
			break;
		case MT_DRV_DISP_FMT_VESA_2560X1440_60_RB:
			dstFmt = MT_UNF_ENC_FMT_VESA_2560X1440_60_RB;
			break;
		case MT_DRV_DISP_FMT_VESA_2560X1600_60_RB:
			dstFmt = MT_UNF_ENC_FMT_VESA_2560X1600_60_RB;
			break;
		case MT_DRV_DISP_FMT_3840X2160_24:
			dstFmt = MT_UNF_ENC_FMT_3840X2160_24;
			break;
		case MT_DRV_DISP_FMT_3840X2160_25:
			dstFmt = MT_UNF_ENC_FMT_3840X2160_25;
			break;
		case MT_DRV_DISP_FMT_3840X2160_30:
			dstFmt = MT_UNF_ENC_FMT_3840X2160_30;
			break;
		case MT_DRV_DISP_FMT_3840X2160_50:
			dstFmt = MT_UNF_ENC_FMT_3840X2160_50;
			break;
		case MT_DRV_DISP_FMT_3840X2160_60:
			dstFmt = MT_UNF_ENC_FMT_3840X2160_60;
			break;
		case MT_DRV_DISP_FMT_4096X2160_24:
			dstFmt = MT_UNF_ENC_FMT_4096X2160_24;
			break;
		case MT_DRV_DISP_FMT_4096X2160_25:
			dstFmt = MT_UNF_ENC_FMT_4096X2160_25;
			break;
		case MT_DRV_DISP_FMT_4096X2160_30:
			dstFmt = MT_UNF_ENC_FMT_4096X2160_30;
			break;
		case MT_DRV_DISP_FMT_4096X2160_50:
			dstFmt = MT_UNF_ENC_FMT_4096X2160_50;
			break;
		case MT_DRV_DISP_FMT_4096X2160_60:
			dstFmt = MT_UNF_ENC_FMT_4096X2160_60;
			break;
		default:
			dstFmt = MT_UNF_ENC_FMT_BUTT;
			break;
	}
	return dstFmt;
}

MT_DRV_DISP_FMT_E hdmi_ENC2DispFmt(MT_UNF_ENC_FMT_E SrcFmt)
{
	MT_DRV_DISP_FMT_E dstFmt = MT_DRV_DISP_FMT_BUTT;
	switch (SrcFmt) {
		case MT_UNF_ENC_FMT_1080P_60 :
			dstFmt = MT_DRV_DISP_FMT_1080P_60;
			break;
		case MT_UNF_ENC_FMT_1080P_50 :
			dstFmt = MT_DRV_DISP_FMT_1080P_50;
			break;
		case MT_UNF_ENC_FMT_1080P_30 :
			dstFmt = MT_DRV_DISP_FMT_1080P_30;
			break;
		case MT_UNF_ENC_FMT_1080P_25 :
			dstFmt = MT_DRV_DISP_FMT_1080P_25;
			break;
		case MT_UNF_ENC_FMT_1080P_24 :
			dstFmt = MT_DRV_DISP_FMT_1080P_24;
			break;
		case MT_UNF_ENC_FMT_1080i_60 :
			dstFmt = MT_DRV_DISP_FMT_1080i_60;
			break;
		case MT_UNF_ENC_FMT_1080i_50 :
			dstFmt = MT_DRV_DISP_FMT_1080i_50;
			break;
		case MT_UNF_ENC_FMT_720P_60 :
		case MT_UNF_ENC_FMT_720P_60_FRAME_PACKING:
			dstFmt = MT_DRV_DISP_FMT_720P_60;
			break;
		case MT_UNF_ENC_FMT_720P_50 :
		case MT_UNF_ENC_FMT_720P_50_FRAME_PACKING:
			dstFmt = MT_DRV_DISP_FMT_720P_50;
			break;
		case MT_UNF_ENC_FMT_576P_50 :
			dstFmt = MT_DRV_DISP_FMT_576P_50;
			break;
		case MT_UNF_ENC_FMT_480P_60 :
			dstFmt = MT_DRV_DISP_FMT_480P_60;
			break;
		case MT_UNF_ENC_FMT_PAL:
		case MT_UNF_ENC_FMT_PAL_N:
		case MT_UNF_ENC_FMT_PAL_Nc:
		case MT_UNF_ENC_FMT_NTSC_PAL_M:

		case MT_UNF_ENC_FMT_SECAM_SIN:
		case MT_UNF_ENC_FMT_SECAM_COS:
			dstFmt = MT_DRV_DISP_FMT_PAL;
			break;
		case MT_UNF_ENC_FMT_NTSC :
		case MT_UNF_ENC_FMT_NTSC_J :
			dstFmt = MT_DRV_DISP_FMT_NTSC;
			break;
		case MT_UNF_ENC_FMT_861D_640X480_60 :
			dstFmt = MT_DRV_DISP_FMT_861D_640X480_60;
			break;
		case MT_UNF_ENC_FMT_VESA_800X600_60 :
			dstFmt = MT_DRV_DISP_FMT_VESA_800X600_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1024X768_60 :
			dstFmt = MT_DRV_DISP_FMT_VESA_1024X768_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1280X720_60 :
			dstFmt = MT_DRV_DISP_FMT_VESA_1280X720_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1280X800_60 :
			dstFmt = MT_DRV_DISP_FMT_VESA_1280X800_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1280X1024_60 :
			dstFmt = MT_DRV_DISP_FMT_VESA_1280X1024_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1360X768_60 :
			dstFmt = MT_DRV_DISP_FMT_VESA_1360X768_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1366X768_60 :
			dstFmt = MT_DRV_DISP_FMT_VESA_1366X768_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1400X1050_60:
			dstFmt = MT_DRV_DISP_FMT_VESA_1400X1050_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1440X900_60:
			dstFmt = MT_DRV_DISP_FMT_VESA_1440X900_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1440X900_60_RB:
			dstFmt = MT_DRV_DISP_FMT_VESA_1440X900_60_RB;
			break;
		case MT_UNF_ENC_FMT_VESA_1600X900_60_RB:
			dstFmt = MT_DRV_DISP_FMT_VESA_1600X900_60_RB;
			break;
		case MT_UNF_ENC_FMT_VESA_1600X1200_60:
			dstFmt = MT_DRV_DISP_FMT_VESA_1600X1200_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1680X1050_60:
			dstFmt = MT_DRV_DISP_FMT_VESA_1680X1050_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1920X1080_60:
			dstFmt = MT_DRV_DISP_FMT_VESA_1920X1080_60;
			break;
		case MT_UNF_ENC_FMT_VESA_1920X1200_60:
			dstFmt = MT_DRV_DISP_FMT_VESA_1920X1200_60;
			break;
		case MT_UNF_ENC_FMT_VESA_2048X1152_60:
			dstFmt = MT_DRV_DISP_FMT_VESA_2048X1152_60;
			break;
		case MT_UNF_ENC_FMT_VESA_2560X1440_60_RB:
			dstFmt = MT_DRV_DISP_FMT_VESA_2560X1440_60_RB;
			break;
		case MT_UNF_ENC_FMT_VESA_2560X1600_60_RB:
			dstFmt = MT_DRV_DISP_FMT_VESA_2560X1600_60_RB;
			break;
		case MT_UNF_ENC_FMT_3840X2160_24:
			dstFmt = MT_DRV_DISP_FMT_3840X2160_24;
			break;
		case MT_UNF_ENC_FMT_3840X2160_25:
			dstFmt = MT_DRV_DISP_FMT_3840X2160_25;
			break;
		case MT_UNF_ENC_FMT_3840X2160_30:
			dstFmt = MT_DRV_DISP_FMT_3840X2160_30;
			break;
		case MT_UNF_ENC_FMT_3840X2160_50:
			dstFmt = MT_DRV_DISP_FMT_3840X2160_50;
			break;
		case MT_UNF_ENC_FMT_3840X2160_60:
			dstFmt = MT_DRV_DISP_FMT_3840X2160_60;
			break;
		case MT_UNF_ENC_FMT_4096X2160_24:
			dstFmt = MT_DRV_DISP_FMT_4096X2160_24;
			break;
		case MT_UNF_ENC_FMT_4096X2160_25:
			dstFmt = MT_DRV_DISP_FMT_4096X2160_25;
			break;
		case MT_UNF_ENC_FMT_4096X2160_30:
			dstFmt = MT_DRV_DISP_FMT_4096X2160_30;
			break;
		case MT_UNF_ENC_FMT_4096X2160_50:
			dstFmt = MT_DRV_DISP_FMT_4096X2160_50;
			break;
		case MT_UNF_ENC_FMT_4096X2160_60:
			dstFmt = MT_DRV_DISP_FMT_4096X2160_60;
			break;
		default:
			dstFmt = MT_DRV_DISP_FMT_BUTT;
			break;
	}
	return dstFmt;
}

static mt_void hdmi_SetAndroidState(mt_s32 PlugState)
{
	#ifdef ANDROID_SUPPORT
	if (g_switchOk == MT_TRUE) {
		switch_set_state(&hdmi_tx_sdev, PlugState);
	}
	#endif
}

/*we may delete this func , because all the complex-use  is controlled by excel table in uboot-stage.*/
mt_void HDMI_PinConfig(mt_void)
{
	return ;
}

static mt_u32 hdmi_Get_FmtVIC(MT_DRV_DISP_FMT_E enEncFmt)
{
	mt_u32 u32VIC = 0;
	switch (enEncFmt) {
		case MT_DRV_DISP_FMT_1080P_60:
			u32VIC = 0x10;
			break;
		case MT_DRV_DISP_FMT_1080P_50:
			u32VIC = 0x1f;
			break;
		case MT_DRV_DISP_FMT_1080P_30:
			u32VIC = 0x22;
			break;
		case MT_DRV_DISP_FMT_1080P_25:
			u32VIC = 0x21;
			break;
		case MT_DRV_DISP_FMT_1080P_24:
			u32VIC = 0x20;
			break;
		case MT_DRV_DISP_FMT_1080i_60:
			u32VIC = 0x05;
			break;
		case MT_DRV_DISP_FMT_1080i_50:
			u32VIC = 0x14;
			break;
		case MT_DRV_DISP_FMT_720P_60:
			u32VIC = 0x04;
			break;
		case MT_DRV_DISP_FMT_720P_50:
			u32VIC = 0x13;
			break;
		case MT_DRV_DISP_FMT_576P_50:
			u32VIC = 0x11;
			break;
		case MT_DRV_DISP_FMT_480P_60:
			u32VIC = 0x02;
			break;
		case MT_DRV_DISP_FMT_PAL:
		case MT_DRV_DISP_FMT_PAL_B:
		case MT_DRV_DISP_FMT_PAL_B1:
		case MT_DRV_DISP_FMT_PAL_D:
		case MT_DRV_DISP_FMT_PAL_D1:
		case MT_DRV_DISP_FMT_PAL_G:
		case MT_DRV_DISP_FMT_PAL_H:
		case MT_DRV_DISP_FMT_PAL_K:
		case MT_DRV_DISP_FMT_PAL_I:
		case MT_DRV_DISP_FMT_PAL_M:
		case MT_DRV_DISP_FMT_PAL_N:
		case MT_DRV_DISP_FMT_PAL_Nc:
		case MT_DRV_DISP_FMT_PAL_60:

		case MT_DRV_DISP_FMT_SECAM_SIN:
		case MT_DRV_DISP_FMT_SECAM_COS:
		case MT_DRV_DISP_FMT_SECAM_L:
		case MT_DRV_DISP_FMT_SECAM_B:
		case MT_DRV_DISP_FMT_SECAM_G:
		case MT_DRV_DISP_FMT_SECAM_D:
		case MT_DRV_DISP_FMT_SECAM_K:
		case MT_DRV_DISP_FMT_SECAM_H:
		case MT_DRV_DISP_FMT_1440x576i_50:
			u32VIC = 0x15;
			break;
		case MT_DRV_DISP_FMT_NTSC:
		case MT_DRV_DISP_FMT_NTSC_J:
		case MT_DRV_DISP_FMT_NTSC_443:
		case MT_DRV_DISP_FMT_1440x480i_60:
			u32VIC = 0x06;
			break;
		default:
			u32VIC = 0x00;
			break;
	}
	COM_INFO("hdmi vic:0x%x\n", u32VIC);
	return u32VIC;
}

mt_u32 DRV_HDMI_Init(mt_u32 FromUserSpace)
{
	mt_u32               u32Vic = 0;
	MT_BOOL              bOpenAlready = MT_FALSE;          //Judge whether HDMI is setup already
	mt_u32               u32Ret = MT_SUCCESS;
	HDMI_COMM_ATTR_S    *pstCommAttr = DRV_Get_CommAttr();
	HDMI_CHN_ATTR_S     *pstChnAttr = DRV_Get_ChnAttr();
	MT_DRV_DISP_FMT_E enEncFmt = MT_DRV_DISP_FMT_BUTT;
	#if HDMI_DISPLAY_READY
	DISP_EXPORT_FUNC_S  *disp_func_ops = MT_NULL;
	#endif

	if (DRV_HDMI_GetInitNum(MT_UNF_HDMI_ID_0) == 0) {
		/* Need to reset this  param */
		DRV_Set_Mce2App(MT_FALSE);
		DRV_Set_OpenedInBoot(MT_FALSE);
	}

	HDMI20_DRV_HDMI_PRINTK("****Enter DRV_HDMI_Init*****\n");

	#if HDMI_DISPLAY_READY
	u32Ret = mt_drv_module_getfunction(MT_ID_DISP, (mt_void**)&disp_func_ops);
	/*if func ops and func ptr is null, then return ;*/
	if ((NULL == disp_func_ops) || (NULL == disp_func_ops->pfnDispGetFormat) || (u32Ret != MT_SUCCESS)) {
		HDMI20_DRV_HDMI_PRINTK("No disp, hdmi init err\n");
		return MT_FAILURE;
	}
	#endif

	/*if FromUserSpace == MT_TRUE,Setup from User,or Setup from Kerne */
	if (MT_TRUE != FromUserSpace) {
		if (g_HDMIKernelInitNum) {
			g_HDMIKernelInitNum ++;
			HDMI20_DRV_HDMI_PRINTK("From Kernel:HDMI has been inited!\n");
			return MT_ERR_HDMI_INIT_ALREADY;
		}
	} else {
		if (g_HDMIUserInitNum) {
			g_HDMIUserInitNum ++;
			HDMI20_DRV_HDMI_PRINTK("FromUser:HDMI has been inited!\n");
			return MT_ERR_HDMI_CALLBACK_ALREADY;
		}
	}

	g_UserCallbackFlag = HDMI_CALLBACK_NULL;

	/* Judge whether it has opened already */
	bOpenAlready = SI_HDMI_Setup_INBoot(&u32Vic);
	HDMI20_DRV_HDMI_PRINTK("bOpenAlready:%d, VIC:%d\n", bOpenAlready, u32Vic);

	if (bOpenAlready == MT_TRUE) {

		#if HDMI_DISPLAY_READY
		if (disp_func_ops && disp_func_ops->pfnDispGetFormat) {
			disp_func_ops->pfnDispGetFormat(MT_DRV_DISPLAY_1, &enEncFmt);
		} else {
			HDMI20_DRV_HDMI_PRINTK("Can't Get disp_func_ops \n");
		}
		#endif

		HDMI20_DRV_HDMI_PRINTK("\nPower on fmt:DispFmt:%d,DispVic:%d, hdmiVic:%d\n", (mt_u32)enEncFmt, hdmi_Get_FmtVIC(enEncFmt),u32Vic);
		if (hdmi_Get_FmtVIC(enEncFmt) == u32Vic && (enEncFmt < MT_DRV_DISP_FMT_BUTT)) {
			bOpenAlready = MT_TRUE;
		}
	}

	HDMI20_DRV_HDMI_PRINTK("\nbOpenAlready:%d,%d\n", (mt_u32)bOpenAlready, u32Vic);
	//bOpenAlready = 0; //for test, need to be done
	if (bOpenAlready == MT_TRUE) {

		if ((g_HDMIUserInitNum > 0) || (g_HDMIKernelInitNum > 0)) {
			if (MT_TRUE == FromUserSpace) {
				g_HDMIUserInitNum ++;
			} else {
				g_HDMIKernelInitNum ++;
			}
			HDMI20_DRV_HDMI_PRINTK("Open Already in MCE!\n");
			/* open already flag */
			DRV_Set_Mce2App(MT_TRUE);

			HDMI20_DRV_HDMI_PRINTK("****green channel change %d *****\n", DRV_Get_IsMce2App());
			return MT_SUCCESS;   //Open Already in MCE!
		} else {
			DRV_Set_OpenedInBoot(MT_TRUE);
			//Open Already in FastBoot!
			HDMI20_DRV_HDMI_PRINTK("HDMI is setup in fastboot!!\n");
		}
		SI_Uboot2mainSmooth(1);
		u32Ret = SiiTxCreate_uboot2app();
		SiiDrvTxHdcpMute(DRV_HDMI_Get_TxInst(), MT_TRUE); //when hdcp on, will self set to false internal
		if ( MT_SUCCESS != u32Ret ) {
			HDMI20_DRV_HDMI_PRINTK("\nSiHdmi20TxCreate sw FAIL!!!\n");
			return MT_FAILURE;
		}
	} else { /* Normal Setup */
		COM_INFO("come to SI_HW_ResetHDMITX\n");
		SI_DisableHdmiDevice();
		SI_HW_ResetHDMITX();
		SI_SW_ResetHDMITX();
		{
		extern void SiiModTxHdcpOtpWrite(void);
		SiiModTxHdcpOtpWrite();
		}
		u32Ret = SiiTxCreate();
		if ( MT_SUCCESS != u32Ret ) {
			HDMI20_DRV_HDMI_PRINTK("\nSiHdmi20TxCreate FAIL!!!\n");
			return MT_FAILURE;
		}
	}

	if (DRV_HDMI_GetInitNum(MT_UNF_HDMI_ID_0) == 0) {
		u32Ret = SI_OpenHdmiDevice();
		#if defined (CEC_SUPPORT)
		/* Enable CEC_SETUP */
		SI_CEC_SetUp();
		#endif
		memset((void *)pstChnAttr, 0, sizeof(HDMI_CHN_ATTR_S) * MT_UNF_HDMI_ID_BUTT);

		//need move to channel Attr??
		memset(g_HDMIWaitFlag, 0, sizeof(mt_u32) * MAX_PROCESS_NUM);
		memset(g_Event_Count, 0, sizeof(mt_u32) * MAX_PROCESS_NUM);
		DRV_Set_ThreadStop(MT_FALSE);

		COM_INFO("SetDefaultAttr \n");
		DRV_HDMI_SetDefaultAttr();
		if (bOpenAlready == MT_TRUE) {
			uint8_t flg = 0;
			info_struct_t info;
			SiiInfoFrame_t info_st = {0};
			HDMI_APP_ATTR_S     *pstAppAttr = DRV_Get_AppAttr(MT_UNF_HDMI_ID_0);
			HDMI_APP_ATTRMT_S     *pstAppAttrMt = DRV_Get_AppAttrMt(MT_UNF_HDMI_ID_0);
			HDMI_UBOOT_AVINFO_T avinfo = {0};
			MT_UNF_HDMI_AVI_INFOFRAME_VER2_S *pstAviInfoFrm = DRV_Get_AviInfoFrm(MT_UNF_HDMI_ID_0);

			SI_GetHdmiHalParams(&avinfo);
			pstAppAttr->enVidOutMode = avinfo.enVidOutMode;
			pstAppAttrMt->stAppAttr.enVidOutMode = avinfo.enVidOutMode;
			pstAviInfoFrm->enOutputType = avinfo.enVidOutMode;
			pstAppAttr->enDeepColorMode = avinfo.enDeepColorMode;
			pstAppAttrMt->stAppAttr.enDeepColorMode = avinfo.enDeepColorMode;
			memset(&info, 0, sizeof(info_struct_t));
			memset(&info_st, 0, sizeof(SiiInfoFrame_t));

			SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, 0);
			SiiDrvTxReadEDIDFromSink(DRV_HDMI_Get_TxInst(), (uint8_t *)(&flg));
			{
				int i;
				info.type = 0x82;
				info.version = 0x02;
				info.length = 0x0D;
				for (i=0;i<SII_INFOFRAME_AVI_MAX_LEN-4;i++) {
					info.pb_byte[1+i] = SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + 4 + i);;
				}
			}
			hdmi_set_infoframe_cksum(&info);
			info_st.ifId = SII_INFO_FRAME_ID__AVI;
			SII_MEMCPY((uint8_t*)&info_st.b, &info, SII_INFOFRAME_AVI_MAX_LEN);
			SiiDrvTxInfoframeSet(DRV_HDMI_Get_TxInst(), &info_st);
			DRV_Set_ChnStart(MT_UNF_HDMI_ID_0, MT_TRUE);
		}

		init_waitqueue_head(&g_astHDMIWait);
		/* create hdmi task */
		if (pstCommAttr->kThreadTimer == NULL) {
			COM_INFO("create Timer task kThreadTimer \n");
			pstCommAttr->kThreadTimer = kthread_create(Hdmi_KThread_Timer, NULL, "MT_HDMI_kThread");
			if (IS_ERR(pstCommAttr->kThreadTimer)) {
				COM_ERR("start hdmi kernel thread failed.\n");
			} else {
				wake_up_process(pstCommAttr->kThreadTimer);
			}
		}
		#if defined (CEC_SUPPORT)
		if (pstCommAttr->kCECRouter == NULL) {
			/* create CEC task */
			pstCommAttr->kCECRouter = kthread_create(Hdmi_KThread_CEC, NULL, "MT_HDMI_kCEC");
			if (IS_ERR(pstCommAttr->kCECRouter)) {
				COM_ERR("Unable to start hdmi kernel thread.\n");
			} else {
				wake_up_process(pstCommAttr->kCECRouter);
			}
		}
		#endif
		/* Normal init HDMI */
		COM_INFO("WriteDefaultConfigToEEPROM\n");
		//SI_WriteDefaultConfigToEEPROM();

		COM_INFO("Leave DRV_HDMI_Init\n");
	}

	if (MT_TRUE == FromUserSpace) {
		g_HDMIUserInitNum ++;
	} else {
		g_HDMIKernelInitNum ++;
	}

	//DRV_PrintCommAttr();
	hdmi_SetAndroidState(STATE_PLUG_UNKNOWN);
	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_DeInit(mt_u32 FromUserSpace)
{
	int ret = 0;
	mt_s32 i;
	HDMI_COMM_ATTR_S *pstCommAttr = DRV_Get_CommAttr();

	COM_INFO("Enter DRV_HDMI_DeInit g_HDMIUserInitNum:0x%x\n", g_HDMIUserInitNum);
	if (DRV_HDMI_GetInitNum(MT_UNF_HDMI_ID_0) == 0) {
		COM_INFO("HDMI has been deInited!\n");
		return MT_SUCCESS;
	}

	if ( (FromUserSpace == MT_TRUE) ) {
		g_HDMIUserInitNum --;
	}

	if ( (FromUserSpace == MT_FALSE) ) {
		g_HDMIKernelInitNum --;
	}

	if (g_HDMIUserInitNum == 0) {
		g_UserCallbackFlag = HDMI_CALLBACK_KERNEL;
	}
	//We only do HDMI Deinit when UserLevel Count is 0.
	COM_INFO("after g_HDMIUserInitNum:0x%x\n", g_HDMIUserInitNum);

	if (DRV_HDMI_GetInitNum(MT_UNF_HDMI_ID_0) != 0) {
		COM_INFO("\n ignore DeInit \n");
		return MT_SUCCESS;
	}

	COM_INFO("stop hdmi task\n");

	if (!IS_ERR_OR_NULL(pstCommAttr->kThreadTimer)) {
		ret = kthread_stop(pstCommAttr->kThreadTimer);
		COM_INFO("end HDMI Timer thread. ret = %d\n", ret);
	}
	pstCommAttr->kThreadTimer = NULL;
	#if defined (CEC_SUPPORT)
	if (!IS_ERR_OR_NULL(pstCommAttr->kCECRouter)) {
		ret = kthread_stop(pstCommAttr->kCECRouter);
		COM_INFO("end HDMI CEC thread. ret = %d\n", ret);
	}
	pstCommAttr->kCECRouter = NULL;
	#endif
	SiiTxDelete();

	for (i = 0; i < MT_UNF_HDMI_ID_BUTT; i++) {
		ret = DRV_HDMI_Close(i);
	}
	SI_HW_ResetHDMITX();
	SI_SW_ResetHDMITX();
	//Disable HDMI IP
	SI_DisableHdmiDevice();
	SI_CloseHdmiDevice();
	//It will power down the whole HDMI IP.
	SI_PoweDownHdmiDevice();

	//force to clear hdmi count
	g_HDMIKernelInitNum = 0;
	g_HDMIUserInitNum   = 0;

	COM_INFO("Leave DRV_HDMI_DeInit\n");
	return MT_SUCCESS;
}

mt_void hdmi_OpenNotify(mt_u32 u32ProcID, MT_UNF_HDMI_EVENT_TYPE_E event)
{
	COM_INFO("\nhdmi_OpenNotify-----u32ProcID %d,event 0x%x\n", u32ProcID, event);
	if (DRV_Get_IsChnOpened(MT_UNF_HDMI_ID_0)) {
		if ((event == MT_UNF_HDMI_EVENT_HOTPLUG)) {
			#if defined (CEC_SUPPORT)
			HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();
			#endif

			// Enable Interrupts: VSync, Ri check, HotPlug
			SI_EnableInterrupts();

			#if defined (CEC_SUPPORT)
			/* Enable CEC_SETUP */
			SI_CEC_SetUp();
			pstChnAttr[MT_UNF_HDMI_ID_0].u8CECCheckCount = 0;
			#endif
			SI_HPD_SetHPDUserCallbackCount();

			hdmi_SetAndroidState(STATE_HOTPLUGIN);

		} else if (event == MT_UNF_HDMI_EVENT_NO_PLUG) {
			if (g_HDMIUserInitNum) {
				#if defined (CEC_SUPPORT)
				HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();
				#endif
				//0x72:0x08 powerdown is only needed in isr && cec
				/* Close HDMI Output */
				//SI_PowerDownHdmiTx();

				SI_DisableHdmiDevice();
				DRV_Set_ChnStart(MT_UNF_HDMI_ID_0, MT_FALSE);
				#if defined (HDCP_SUPPORT)
				/*Set HDCP Off */
				//SI_WriteByteEEPROM(EE_TX_HDCP, 0x00);
				#endif
				#if defined (CEC_SUPPORT)
				//Close CEC
				SI_CEC_Close();
				//DRV_Set_CECStart(MT_UNF_HDMI_ID_0, MT_FALSE);
				memset(&(pstChnAttr[MT_UNF_HDMI_ID_0].stCECStatus), 0, sizeof(MT_UNF_HDMI_CEC_STATUS_S));
				#endif
			}
			hdmi_SetAndroidState(STATE_HOTPLUGOUT);
		} else {
			COM_ERR("Unknow Event:0x%x\n", event);
		}
		COM_INFO("line:%d,event:%d\n", __LINE__, event);

		hdmi_ProcEvent(event, u32ProcID);
	}
}

mt_u32 DRV_HDMI_Open(MT_UNF_HDMI_ID_E enHdmi, HDMI_OPEN_S *pOpen, mt_u32 FromUserSpace, mt_u32 u32ProcID)
{
	//mt_u32 Ret;
	MT_BOOL bOpenLastTime = DRV_Get_IsChnOpened(enHdmi);
	HDMI_APP_ATTR_S     *pstAppAttr = DRV_Get_AppAttr(enHdmi);
	HDMI_APP_ATTRMT_S     *pstAppAttrMt = DRV_Get_AppAttrMt(enHdmi);

	COM_INFO("Enter DRV_HDMI_Open\n");
	HDMI_CHECK_ID(enHdmi);

	g_HDMIOpenNum ++; //record open num

	/* Need to set this  param */
	DRV_Set_DefaultOutputMode(enHdmi, pOpen->enDefaultMode);

	//set output mode
	COM_INFO("enForceMode:%d, %d  %d\n", DRV_Get_DefaultOutputMode(enHdmi),DRV_Get_IsOpenedInBoot(),SI_TX_IsHDMImode());

	//In Mce or Boot,we setted hdmi mode.so we use Hdmi in reg.
	if (DRV_Get_IsMce2App() || DRV_Get_IsOpenedInBoot()) {
		if (SI_TX_IsHDMImode()) {
			pstAppAttr->bEnableHdmi = MT_TRUE;
		} else {
			pstAppAttr->bEnableHdmi = MT_FALSE;
		}
	}
	//In None Mce mode, we use User setting defalut mode defaut mode
	else {
		if (!DRV_Get_IsValidSinkCap(enHdmi)) {
			if (MT_UNF_HDMI_DEFAULT_ACTION_DVI != DRV_Get_DefaultOutputMode(enHdmi)) {
				pstAppAttr->bEnableHdmi = MT_TRUE;
			}
		}
	}
	pstAppAttrMt->stAppAttr.bEnableHdmi = pstAppAttr->bEnableHdmi;

	DRV_Set_ChnOpen(enHdmi, MT_TRUE);

	if ((pOpen->initParam & HDMI_INIT_OUT_HDCP_KEY) || (pOpen->initParam & HDMI_INIT_INNER_HDCP_KEY)) {
		pstAppAttr->bHDCPEnable = MT_TRUE;
		pstAppAttrMt->stAppAttr.bHDCPEnable = MT_TRUE;
		if ( mt_suspend_flag == 0 ) {
			DRV_HDMI_HdcpMute(enHdmi, MT_TRUE);
			SI_hdcp_en(MT_TRUE, 0);
		}
	}
	if (g_HDMIUserInitNum) {
		g_UserCallbackFlag = HDMI_CALLBACK_USER;
		if (g_HDMIKernelInitNum > 0) {
			/* It means we have initalized in Kernel and user */
			//We need to Reset a HotPlug Event to User
			if (MT_TRUE == SI_HPD_Status()) {
				hdmi_OpenNotify(u32ProcID, MT_UNF_HDMI_EVENT_HOTPLUG);
			} else {
				hdmi_OpenNotify(u32ProcID, MT_UNF_HDMI_EVENT_NO_PLUG);
			}
		} else {
			COM_INFO("SI_HPD_Status():%d\n", SI_HPD_Status());
		}
	} else if (g_HDMIKernelInitNum) {
		g_UserCallbackFlag = HDMI_CALLBACK_KERNEL;
	}

	mt_suspend_flag = 0;
	if (!DRV_Get_IsMce2App()) {
		if (bOpenLastTime) {
			return MT_SUCCESS;
		}
	}

	COM_INFO("Leave DRV_HDMI_Open\n");
	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_Close(MT_UNF_HDMI_ID_E enHdmi)
{
	COM_INFO("Enter DRV_HDMI_Close g_HDMIOpenNum:%d\n", g_HDMIOpenNum);
	HDMI_CHECK_ID(enHdmi);

	//Stop HDMI IP
	if ( g_HDMIOpenNum > 1) {
		g_HDMIOpenNum --;
		return MT_SUCCESS;
	} else if (g_HDMIOpenNum == 1) {
		g_HDMIOpenNum = 0;
	} else {
		//Just do following action.
	}

	//DRV_HDMI_HdcpMute(enHdmi, MT_TRUE);
	//DRV_HDMI_SetAVMute(enHdmi, MT_TRUE);
	if (DRV_Get_IsChnStart(enHdmi)) {
		SI_SetHdmiVideo(MT_FALSE);
		SI_SetHdmiAudio(MT_FALSE);
		SI_PowerDownHdmiTx();
		#if defined (HDCP_SUPPORT)
		/*Set HDCP Off */
		//SI_WriteByteEEPROM(EE_TX_HDCP, 0x00);
		SI_hdcp_en(MT_FALSE, 0);
		SiiLibTimeMilliDelay(50);
		#endif
		DRV_Set_ChnStart(enHdmi, MT_FALSE);
	}
	//Close HDMI
	if (DRV_Get_IsChnOpened(enHdmi)) {
		DRV_Set_ChnOpen(enHdmi, MT_FALSE);
	}
	//No callback
	g_UserCallbackFlag = HDMI_CALLBACK_NULL;
	//Disable HDMI IP
	SI_DisableHdmiDevice();
	SI_CloseHdmiDevice();

	COM_INFO("Leave DRV_HDMI_Close\n");
	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_EDID_BASE_INFO_S *pstSinkAttr)
{
	//COM_INFO("Enter DRV_HDMI_GetSinkCapability\n");
	HDMI_CHECK_NULL_PTR(pstSinkAttr);
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	if (MT_SUCCESS != SI_GetHdmiSinkCaps(pstSinkAttr)) {
		COM_WARN("GetHdmiSinkCaps error.\n");
		return MT_FAILURE;
	}

	//COM_INFO("Leave DRV_HDMI_GetSinkCapability\n");
	return MT_SUCCESS;
}

static mt_u32 hdmi_VideoAttrChanged(MT_UNF_HDMI_ID_E enHdmi, HDMI_VIDEO_ATTR_S *pstAttr1, HDMI_VIDEO_ATTR_S *pstAttr2,
									MT_BOOL *pVUpdate)
{
	*pVUpdate = MT_FALSE;

	/* HDMI has not started, we set this value */
	if (!DRV_Get_IsChnStart(enHdmi)) {
		*pVUpdate = MT_TRUE;
		return MT_SUCCESS;
	}

	if ( (pstAttr1->enVideoFmt          != pstAttr2->enVideoFmt)
			|| (pstAttr1->b3DEnable           != pstAttr2->b3DEnable)
			|| (pstAttr1->u83DParam           != pstAttr2->u83DParam)
	   ) {
		*pVUpdate = MT_TRUE;
	} else {
		COM_INFO("We do not need to update Video!\n");
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

static mt_u32 hdmi_AudioAttrChanged(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstAttr1, HDMI_AUDIO_ATTR_S *pstAttr2,
									MT_BOOL *pAUpdate)
{
	*pAUpdate = MT_TRUE;

	/* HDMI Has not started, we set this value */
	if (!DRV_Get_IsChnStart(enHdmi)) {
		*pAUpdate = MT_TRUE;
		return MT_SUCCESS;
	}

	/* Same setting, return directly */
	if ( (pstAttr1->enSoundIntf                == pstAttr2->enSoundIntf)
			&& (pstAttr1->bIsMultiChannel     == pstAttr2->bIsMultiChannel)
			&& (pstAttr1->enSampleRate        == pstAttr2->enSampleRate)
			&& (pstAttr1->u8DownSampleParm    == pstAttr2->u8DownSampleParm)
			&& (pstAttr1->enBitDepth          == pstAttr2->enBitDepth)
			&& (pstAttr1->u8I2SCtlVbit        == pstAttr2->u8I2SCtlVbit)
	   ) {
		*pAUpdate = MT_FALSE;
		COM_INFO("Same as before, do not need to setting!\n");
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

//when need update audio or hdcpflag,but did not need update video,then return update flag false
//when change video,we need recfg all
//return MT_FAILUR need not update anyone
//return MT_SUCCESS && pUpdate == TRUE  need recfg all
//return MT_SUCCESS && pUpdate == FALSE only update audio && hdcp
static mt_u32 hdmi_AppAttrChanged(MT_UNF_HDMI_ID_E enHdmi, HDMI_APP_ATTR_S *pstAttr1, HDMI_APP_ATTR_S *pstAttr2,
								  MT_BOOL *pUpdate)
{
	*pUpdate = MT_FALSE;

	if (!DRV_Get_IsChnStart(enHdmi)) {
		if (DRV_Get_IsMce2App() || DRV_Get_IsOpenedInBoot()) {
			if ((pstAttr1->bEnableHdmi != pstAttr2->bEnableHdmi)
					|| (pstAttr1->enVidOutMode != pstAttr2->enVidOutMode)) {
				*pUpdate = MT_TRUE;
				DRV_Set_Mce2App(MT_FALSE);
				DRV_Set_OpenedInBoot(MT_FALSE);
			} else {
				*pUpdate = MT_FALSE;
			}
		} else {
			*pUpdate = MT_TRUE;
		}
		return MT_SUCCESS;
	}

	if ((pstAttr1->bEnableHdmi            != pstAttr2->bEnableHdmi)
			|| (pstAttr1->bEnableVideo        != pstAttr2->bEnableVideo)
			|| (pstAttr1->enVidOutMode        != pstAttr2->enVidOutMode)
			|| (pstAttr1->enDeepColorMode     != pstAttr2->enDeepColorMode)
			|| (pstAttr1->bxvYCCMode          != pstAttr2->bxvYCCMode)
	   ) {
		COM_INFO("App Attr need update video!\n");
		*pUpdate = MT_TRUE;
	} else if ((pstAttr1->bEnableAudio        != pstAttr2->bEnableAudio)
			   || (pstAttr1->bEnableAviInfoFrame != pstAttr2->bEnableAviInfoFrame)
			   || (pstAttr1->bEnableAudInfoFrame != pstAttr2->bEnableAudInfoFrame)
			   || (pstAttr1->bEnableSpdInfoFrame != pstAttr2->bEnableSpdInfoFrame)
			   || (pstAttr1->bEnableMpegInfoFrame != pstAttr2->bEnableMpegInfoFrame)
			   || (pstAttr1->bDebugFlag          != pstAttr2->bDebugFlag)
			   || (pstAttr1->bHDCPEnable         != pstAttr2->bHDCPEnable)) {
		COM_INFO("App Attr need not update video!\n");
	} else {
		COM_INFO("App Attr need not update anything!\n");
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_BOOL get_current_rgb_mode(MT_UNF_HDMI_ID_E enHdmi)
{
	HDMI_APP_ATTR_S *pstAppAttr = DRV_Get_AppAttr(enHdmi);
	return (MT_UNF_HDMI_VIDEO_MODE_RGB444 == pstAppAttr->enVidOutMode ) ? MT_TRUE : MT_FALSE;
}

mt_void DRV_HDMI_CrAdaptive(MT_UNF_HDMI_ID_E enHdmi)
{
	HDMI_TMDS_CLK_RATE_T tmds_clk;

	tmds_clk = get_hdmi_tmds_clk(enHdmi);
	if ( (tmds_clk > HDMI_TMDS_CLK_297MHZ_1X && tmds_clk < HDMI_TMDS_CLK_594MHZ_1D25X) || \
			(tmds_clk > HDMI_TMDS_CLK_297_1P001MHZ_1X && tmds_clk < HDMI_TMDS_CLK_594_1P001MHZ_1D25X)) {
		SiiDrvTxScdcSrcCrSet(DRV_HDMI_Get_TxInst(), 1);
	} else {
		SiiDrvTxScdcSrcCrSet(DRV_HDMI_Get_TxInst(), 0);
	}
	if (getScdcEnable()) {
		SiiDrvTxScdcScrambleenable(DRV_HDMI_Get_TxInst(), NULL);
	}
}

mt_u32 DRV_HDMI_Need_Update_Clk(MT_UNF_HDMI_ID_E enHdmi, HDMI_APP_ATTR_S *pstAttr_old)
{
	HDMI_APP_ATTR_S *pstAppAttr = DRV_Get_AppAttr(enHdmi);
	mt_u32 NeedUpdateClk = 1;

	if ( (pstAttr_old->enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_YCBCR444 && pstAppAttr->enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_RGB444) || \
		(pstAttr_old->enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_RGB444 && pstAppAttr->enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_YCBCR444)) {
		NeedUpdateClk = 0;
	}
	COM_INFO("Set Dc/Csc:old[%d %d],new[%d %d]\n",pstAttr_old->enVidOutMode,pstAttr_old->enDeepColorMode,\
				pstAppAttr->enVidOutMode,pstAppAttr->enDeepColorMode);
	return NeedUpdateClk;
}

mt_s32 DRV_HDMI_HdcpMute(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bAvMute)
{
	SiiDrvDsHdcpVersion_t cap = SII_DRV_DS_HDCP_VER__NONE;
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	SiiDrvTxHdcpCapGet(DRV_HDMI_Get_TxInst(), &cap);
	if ( cap != SII_DRV_DS_HDCP_VER__NONE ) {
		SiiDrvTxHdcpMute(DRV_HDMI_Get_TxInst(), bAvMute);
	}
	return MT_SUCCESS;
}

extern mt_s32 disp_tvsys_force_update(void);
static mt_void DRV_HDMI_Update_Clk(void)
{
	DRV_HDMI_Set_Update_Tvsys(1);
	(void)disp_tvsys_force_update();
	while( DRV_HDMI_Get_Update_Tvsys()	 != 0 ) {
		//COM_INFO("Vcfg sts:%d\n", DRV_HDMI_Get_Update_Tvsys());
		SiiLibTimeMilliDelay(10); //wait
	}
}

mt_u32 DRV_HDMI_SetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_ATTR_S *pstAttr)
{
	mt_u32 Ret = MT_SUCCESS;
	mt_u32 ForceUpdateFlag = MT_FALSE;
	mt_u32 PartUpdateFlag = MT_FALSE;
	HDMI_APP_ATTR_S pstAttr_old;
	HDMI_APP_ATTR_S *pstAppAttr = DRV_Get_AppAttr(enHdmi);
	HDMI_APP_ATTRMT_S *pstAppAttrMt = DRV_Get_AppAttrMt(enHdmi);
	HDMI_APP_ATTR_S pstAppAttrTmp = pstAppAttrMt->stAppAttr;

	COM_INFO("Enter DRV_HDMI_SetAttr\n");

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pstAttr);

	COM_INFO("Hdmi en=%d, hdcp en=%d, dc=%d, vout=%d\n", pstAttr->stAppAttr.bEnableHdmi, \
			 pstAttr->stAppAttr.bHDCPEnable, pstAttr->stAppAttr.enDeepColorMode, \
			 pstAttr->stAppAttr.enVidOutMode);

	// SetAppAttr only cfg hdcp,
	// the other attr will be config in DRV_HDMI_SetVO/AOAttr && hdmi_AdjustAVI/AUDInfoFrame
	pstAttr_old = *pstAppAttr;
	DRV_HDMI_SetAPPAttr(enHdmi, &pstAttr->stAppAttr, MT_FALSE);
	memcpy(&pstAppAttrMt->stAppAttr, &pstAttr->stAppAttr, sizeof(HDMI_APP_ATTR_S));
	pstAppAttrMt->stAppAttr.enDeepColorMode = pstAppAttrTmp.enDeepColorMode;
	pstAppAttrMt->stAppAttr.enVidOutMode = pstAppAttrTmp.enVidOutMode;
	if (pstAttr_old.bHDCPEnable == MT_FALSE && pstAttr->stAppAttr.bHDCPEnable == MT_TRUE) {
		DRV_HDMI_HdcpMute(enHdmi, MT_TRUE);
	}

	{
		if ( pstAttr->stAppAttr.enVidOutMode != pstAttr_old.enVidOutMode || \
			pstAttr->stAppAttr.enDeepColorMode != pstAttr_old.enDeepColorMode || \
			DRV_Get_IsChnStart(enHdmi) == MT_FALSE ) {
			if (DRV_HDMI_Need_Update_Clk(enHdmi, &pstAttr_old)) {
				DRV_HDMI_Update_Clk();
				return MT_SUCCESS;
			} else {
				if ( pstAttr->stAppAttr.enVidOutMode < MT_UNF_HDMI_VIDEO_MODE_AUTO ) {
					pstAppAttrMt->stAppAttr.enVidOutMode = pstAttr->stAppAttr.enVidOutMode;
				}
			}
		}
	}
	//Force update flag can changed in set format && DRV_HDMI_SetAPPAttr
	ForceUpdateFlag = DRV_Get_IsNeedForceUpdate(enHdmi);
	if (ForceUpdateFlag == MT_TRUE) {
		PartUpdateFlag = MT_TRUE;
	} else {
		PartUpdateFlag = DRV_Get_IsNeedPartUpdate(enHdmi);
	}
	COM_INFO("ForceUpdateFlag %d, PartUpdateFlag %d \n", ForceUpdateFlag, PartUpdateFlag);

	if (DRV_Get_IsOpenedInBoot()) {
		COM_INFO("Green Channel First Time \n");

		//when opend in boot,then audio not be configed.
		DRV_HDMI_SetAOAttr(enHdmi, &pstAttr->stAudioAttr, PartUpdateFlag);
		hdmi_AdjustAUDInfoFrame(enHdmi);
		return MT_SUCCESS;
	} else if (DRV_Get_IsMce2App()) {
		COM_INFO(" Mce2App  \n");
		return MT_SUCCESS;
	}

	DRV_Set_ThreadStop(MT_TRUE);
	DRV_HDMI_SetVOAttr(enHdmi, &pstAttr->stVideoAttr, ForceUpdateFlag);
	DRV_HDMI_SetAOAttr(enHdmi, &pstAttr->stAudioAttr, PartUpdateFlag);

	hdmi_AdjustAVIInfoFrame(enHdmi);
	hdmi_AdjustVSDBInfoFrame(enHdmi);
	hdmi_AdjustAUDInfoFrame(enHdmi);
	DRV_Set_ThreadStop(MT_FALSE);

	//SiiLibTimeMilliDelay(100);
	//DRV_HDMI_Phy_Rst(1);
	//DRV_HDMI_Phy_Rst(0);
	#ifdef HDCP_SUPPORT
	if (DRV_Get_IsChnStart(MT_UNF_HDMI_ID_0)) {
		/* Set HDMI HDCP Enable flag */
		HDCP_INFO("bHDCPEnable:0x%x, bDebugFlag:0x%x\n", pstAttr->stAppAttr.bHDCPEnable, pstAttr->stAppAttr.bDebugFlag);
		if (pstAttr->stAppAttr.bHDCPEnable == MT_TRUE) {
			SI_hdcp_en(MT_TRUE, 0);
		} else {
			DRV_HDMI_HdcpMute(enHdmi, MT_FALSE);
			SI_hdcp_en(MT_FALSE, 1);
			DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_FALSE);
		}

		SiiLibTimeMilliDelay(60);
	}
	#endif

	DRV_Set_ForceUpdateFlag(enHdmi, MT_FALSE);
	DRV_Set_PartUpdateFlag(enHdmi, MT_FALSE);

	COM_INFO("Leave DRV_HDMI_SetAttr\n");
	return Ret;
}

mt_u32 DRV_HDMI_GetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_ATTR_S *pstAttr)
{
	HDMI_ATTR_S *pstHDMIAttr = DRV_Get_HDMIAttr(enHdmi);
	COM_INFO("Enter DRV_HDMI_GetAttr\n");
	SI_timer_count();
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pstAttr);

	pstHDMIAttr->stAppAttr.bEnableVideo = MT_TRUE;
	memcpy(pstAttr, pstHDMIAttr, sizeof(HDMI_ATTR_S));

	COM_INFO("Leave DRV_HDMI_GetAttr\n");
	return MT_SUCCESS;
}

#if defined (CEC_SUPPORT)
mt_u32 DRV_HDMI_CECStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_STATUS_S  *pStatus)
{
	MT_UNF_HDMI_CEC_STATUS_S *pstCecStatus = DRV_Get_CecStatus(enHdmi);
	memset(pStatus, 0, sizeof(MT_UNF_HDMI_CEC_STATUS_S));
	memcpy(pStatus, pstCecStatus, sizeof(MT_UNF_HDMI_CEC_STATUS_S));

	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_GetCECAddress(MT_U8 *pPhyAddr, MT_U8 *pLogicalAddr)
{
	MT_UNF_HDMI_CEC_STATUS_S *pstCecStatus = DRV_Get_CecStatus(MT_UNF_HDMI_ID_0);
	//Only invoke in private mode
	if (!DRV_Get_IsCECStart(MT_UNF_HDMI_ID_0)) {
		return MT_ERR_HDMI_DEV_NOT_OPEN;
	}

	memcpy(pPhyAddr, pstCecStatus->u8PhysicalAddr, 4);
	*pLogicalAddr = pstCecStatus->u8LogicalAddr;

	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_SetCECCommand(MT_UNF_HDMI_ID_E enHdmi, const MT_UNF_HDMI_CEC_CMD_S  *pCECCmd)
{
	MT_UNF_HDMI_CEC_STATUS_S *pstCecStatus = DRV_Get_CecStatus(enHdmi);
	mt_u32 Ret = MT_SUCCESS;

	COM_INFO("Enter DRV_HDMI_SetCECCommand\n");
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pCECCmd);

	if (!DRV_Get_IsCECStart(enHdmi)) {
		CEC_ERR("CEC do not start\n");
		return MT_ERR_HDMI_DEV_NOT_OPEN;
	}

	if (pCECCmd->enSrcAdd != pstCecStatus->u8LogicalAddr) {
		COM_INFO("Invalid enSrcAdd:0x%x, 0x%x\n", pCECCmd->enSrcAdd, pstCecStatus->u8LogicalAddr);
		//return MT_ERR_HDMI_INVALID_PARA;
	}
	Ret = SI_CEC_SendCommand((MT_UNF_HDMI_CEC_CMD_S *)pCECCmd);

	CEC_INFO("Leave DRV_HDMI_SetCECCommand\n");
	return Ret;
}

//extern unsigned int  get_cec_msg(MT_UNF_HDMI_CEC_CMD_S *rx_cmd, unsigned int num, mt_u32 timeout);
mt_u32 DRV_HDMI_GetCECCommand(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S  *pCECCmd, mt_u32 timeout)
{
	CEC_ERR("%s dont support, some cec msg will notify!\n", __func__);
	return MT_FAILURE;
}
#endif

static mt_u32 hdmi_Create_AVI_Infoframe(MT_UNF_HDMI_AVI_INFOFRAME_VER2_S *punAVIInfoFrame, MT_U8 *pu8AviInfoFrame)
{
	MT_U8  u8AviInfoFrameByte = 0;
	hdmi_VideoIdentification_t *pstVidCode;
	mt_s32 retval = MT_SUCCESS;
	mt_u32 VidIdCode;

	MT_DRV_DISP_FMT_E encFmt;

	/* HDMI AVI Infoframe is use Version = 0x02 in HDMI1.3 */

	/* Fill Data Byte 1 */
	u8AviInfoFrameByte = 0;
	/* Scan information bits 0-1:S0,S1 */
	/*
	   S1 S0 Scan Information
	   0   0    No Data
	   0   1   overscanned
	   1   0   underscanned
	   1   1   Future
	   */
	switch (punAVIInfoFrame->enScanInfo) {
		case HDMI_SCAN_INFO_NO_DATA :
			u8AviInfoFrameByte |= (MT_U8)HDMI_SCAN_INFO_NO_DATA;
			break;
		case HDMI_SCAN_INFO_OVERSCANNED :
			u8AviInfoFrameByte |= (MT_U8)HDMI_SCAN_INFO_OVERSCANNED;
			break;
		case HDMI_SCAN_INFO_UNDERSCANNED :
			u8AviInfoFrameByte |= (MT_U8)HDMI_SCAN_INFO_UNDERSCANNED;
			break;
		default :
			retval = MT_FAILURE;
			break;
	}
	/* Bar Information bits 2-3:B0,B1 */
	/*
	   B1 B0  Bar Info
	   0   0  not valid
	   0   1  Vert. Bar Info valid
	   1   0  Horiz.Bar Info Valid
	   1   1  Vert. and Horiz. Bar Info valid
	   */
	switch (punAVIInfoFrame->enBarInfo) {
		case HDMI_BAR_INFO_NOT_VALID :
			u8AviInfoFrameByte |= (MT_U8) 0x00;
			break;
		case HDMI_BAR_INFO_V :
			u8AviInfoFrameByte |= (MT_U8) 0x04;
			break;
		case HDMI_BAR_INFO_H :
			u8AviInfoFrameByte |= (MT_U8) 0x08;
			break;
		case HDMI_BAR_INFO_VH :
			u8AviInfoFrameByte |= (MT_U8) 0x0C;
			break;
		default :
			retval = MT_FAILURE;
			break;
	}
	/* Active information bit 4:A0 */
	/*
	   A0 Active Format Information Present
	   0        No Data
	   1      Active Format(R0��R3) Information valid
	   */
	if (punAVIInfoFrame->bActive_Infor_Present) {
		u8AviInfoFrameByte |= (MT_U8)0x10;  /* Active Format Information Valid */
	} else {
		u8AviInfoFrameByte &= ~(MT_U8)0x10;  /* Active Format Information Valid */
	}
	/* Output Type bits 5-6:Y0,Y1 */
	/*
	   Y1 Y0  RGB orYCbCr
	   0  0   RGB (default)
	   0  1   YCbCr 4:2:2
	   1  0   YCbCr 4:4:4
	   1  1    Future
	   */
	COM_INFO("punAVIInfoFrame->enOutputType:%d\n", punAVIInfoFrame->enOutputType);
	switch (punAVIInfoFrame->enOutputType) {
		case MT_UNF_HDMI_VIDEO_MODE_RGB444 :
			u8AviInfoFrameByte |= (MT_U8)0x00;
			break;
		case MT_UNF_HDMI_VIDEO_MODE_YCBCR422 :
			u8AviInfoFrameByte |= (MT_U8)0x20;
			break;
		case MT_UNF_HDMI_VIDEO_MODE_YCBCR444 :
			u8AviInfoFrameByte |= (MT_U8)0x40;
			break;
		case MT_UNF_HDMI_VIDEO_MODE_YCBCR420:
			u8AviInfoFrameByte |= (MT_U8)0x60;
			break;
		default :
			COM_INFO("Error Output format *******\n");
			retval = MT_FAILURE;
			break;
	}
	pu8AviInfoFrame[0] = (MT_U8)(u8AviInfoFrameByte & 0x7F);

	/* Fill Data byte 2 */
	u8AviInfoFrameByte = 0;
	/* Active Format aspect ratio bits 0-3:R0...R3 */
	/*
	   R3 R2 R1 R0  Active Format Aspect Ratio
	   1  0  0  0   Same as picture aspect ratio
	   1  0  0  1   4:3 (Center)
	   1  0  1  0   16:9 (Center)
	   1  0  1  1   14:9 (Center)
	   */

	COM_INFO("Active Format aspect ratio  set to 0x1000:Same as picture aspect ratio:%d\n", punAVIInfoFrame->enAspectRatio);
	u8AviInfoFrameByte |= (MT_U8) 0x08;

	switch (punAVIInfoFrame->enAspectRatio) {
		case MT_UNF_HDMI_ASPECT_RATIO_4TO3 :
			u8AviInfoFrameByte |= (MT_U8) 0x10;
			break;
		case MT_UNF_HDMI_ASPECT_RATIO_16TO9 :
			u8AviInfoFrameByte |= (MT_U8) 0x20;
			break;
		default :
			u8AviInfoFrameByte |=  (MT_U8) 0x00;
			break;
	}

	/* Colorimetry bits 6-7 of data byte2:C0,C1 */
	/*
	   C1 C0    Colorim
	   0   0    No Data
	   0   1    SMPTE 170M[1] ITU601 [5]
	   1   0    ITU709 [6] 1 0 16:9
	   1   1    Extended Colorimetry Information Valid (colorimetry indicated in bits EC0, EC1,
	   EC2. See Table 11)
	   */
	switch (punAVIInfoFrame->enColorimetry) {
		case HDMI_COLORIMETRY_ITU601 :
			u8AviInfoFrameByte |= (MT_U8)0x40;
			break;
		case HDMI_COLORIMETRY_ITU709 :
			u8AviInfoFrameByte |= (MT_U8)0x80;
			break;
		case HDMI_COLORIMETRY_XVYCC_601 :
		case HDMI_COLORIMETRY_XVYCC_709 :
		case HDMI_COLORIMETRY_EXTENDED :
		case HDMI_COLORIMETRY_BT2020_C:
		case HDMI_COLORIMETRY_BT2020_NC:
			u8AviInfoFrameByte |= (MT_U8)0xC0;
			break;
		default :
			u8AviInfoFrameByte |= (MT_U8)0x00;
			break;
	}
	pu8AviInfoFrame[1] = (MT_U8)(u8AviInfoFrameByte & 0XFF);

	/* Fill data Byte 3: Picture Scaling bits 0-1:SC0,SC1 */
	u8AviInfoFrameByte = 0;
	/*
	   SC1  SC0   Non-Uniform Picture Scaling
	   0     0    No Known non-uniform Scaling
	   0     1    Picture has been scaled horizontally
	   1     0    Picture has been scaled vertically
	   1     1    Picture has been scaled horizontally and vertically
	   */
	switch (punAVIInfoFrame->enPictureScaling) {
		case HDMI_PICTURE_NON_UNIFORM_SCALING :
			u8AviInfoFrameByte |= (MT_U8)0x00;
			break;
		case HDMI_PICTURE_SCALING_H :
			u8AviInfoFrameByte |= (MT_U8)0x01;
			break;
		case HDMI_PICTURE_SCALING_V :
			u8AviInfoFrameByte |= (MT_U8)0x02;
			break;
		case HDMI_PICTURE_SCALING_HV :
			u8AviInfoFrameByte |= (MT_U8)0x03;
			break;
		default :
			retval = MT_FAILURE;
			break;
	}
	/* Fill data Byte 3: RGB quantization range bits 2-3:Q0,Q1 */
	/*
	   Q1  Q0  RGB Quantization Range
	   0   0   Default (depends on video format)
	   0   1   Limited Range
	   1   0   Full Range
	   1   1   Reserved
	   */
	switch (punAVIInfoFrame->enRGBQuantization) {
		case HDMI_RGB_QUANTIZATION_DEFAULT_RANGE :
			u8AviInfoFrameByte |= (MT_U8)0x00;
			break;
		case HDMI_RGB_QUANTIZATION_LIMITED_RANGE :
			u8AviInfoFrameByte |= (MT_U8)0x04;
			break;
		case HDMI_RGB_QUANTIZATION_FULL_RANGE :
			u8AviInfoFrameByte |= (MT_U8)0x08;
			break;
		default :
			retval = MT_FAILURE;
			break;
	}
	/* Fill data Byte 3: Extended colorimtery range bits 4-6:EC0,EC1,EC2 */
	/*
	   EC2 EC1 EC0   Extended Colorimetry
	   0   0   0      xvYCC601
	   0   0   1      xvYCC709
	   -   -   -      All other values reserved
	   */
	/*
	   xvYCC601 is based on the colorimetry defined in ITU-R BT.601.
	   xvYCC709 is based on the colorimetry defined in ITU-R BT.709.
	   */
	switch (punAVIInfoFrame->enColorimetry) {
		case HDMI_COLORIMETRY_XVYCC_601 :
			u8AviInfoFrameByte |= (MT_U8)0x00;
			break;
		case HDMI_COLORIMETRY_XVYCC_709 :
			u8AviInfoFrameByte |= (MT_U8)0x10;
			break;
		case HDMI_COLORIMETRY_BT2020_C :
			u8AviInfoFrameByte |= (MT_U8)0x50;
			break;
		case HDMI_COLORIMETRY_BT2020_NC :
			u8AviInfoFrameByte |= (MT_U8)0x60;
			break;
		default:
			break;
	}

	/* Fill data Byte 3: IT content bit 7:ITC
	   ITC  IT content
	   0    No data
	   1    IT content
	   */
	if (punAVIInfoFrame->bIsITContent) {
		u8AviInfoFrameByte |= 0x80;
	} else {
		u8AviInfoFrameByte &= ~0x80;
	}

	pu8AviInfoFrame[2] = (MT_U8)(u8AviInfoFrameByte & 0XFF);

	/* Fill Data byte 4: Video indentification data Code, Bit0~7:VIC0 ~ VIC6 */
	u8AviInfoFrameByte = 0;
	encFmt = hdmi_ENC2DispFmt(punAVIInfoFrame->enTimingMode);
	pstVidCode = hdmi_GetVideoCode(encFmt);

	VidIdCode = pstVidCode->VidIdCode;
	if (MT_UNF_HDMI_ASPECT_RATIO_16TO9 == punAVIInfoFrame->enAspectRatio) {
		if (encFmt == MT_DRV_DISP_FMT_480P_60) {
			VidIdCode = 3;
			COM_INFO("Sepcail setting:change pstVidCode(480p_60 16:9):%d-->%d\n", pstVidCode->VidIdCode, VidIdCode);
		} else if (encFmt == MT_DRV_DISP_FMT_576P_50) {
			VidIdCode = 18;
			COM_INFO("Sepcail setting:change pstVidCode(576p_50 16:9):%d-->%d\n", pstVidCode->VidIdCode, VidIdCode);
		} else if ((encFmt == MT_DRV_DISP_FMT_PAL) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_B) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_B1) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_D) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_D1) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_G) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_H) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_K) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_I) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_M) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_N) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_Nc) ||
				   (encFmt == MT_DRV_DISP_FMT_PAL_60) ||
				   (encFmt == MT_DRV_DISP_FMT_1440x576i_50) ||
				   (encFmt == MT_DRV_DISP_FMT_SECAM_SIN) ||
				   (encFmt == MT_DRV_DISP_FMT_SECAM_COS) ||
				   (encFmt == MT_DRV_DISP_FMT_SECAM_L) ||
				   (encFmt == MT_DRV_DISP_FMT_SECAM_B) ||
				   (encFmt == MT_DRV_DISP_FMT_SECAM_G) ||
				   (encFmt == MT_DRV_DISP_FMT_SECAM_D) ||
				   (encFmt == MT_DRV_DISP_FMT_SECAM_K) ||
				   (encFmt == MT_DRV_DISP_FMT_SECAM_H))

		{
			VidIdCode = 22;
			COM_INFO("Sepcail setting:change pstVidCode(576i_50 16:9):%d-->%d\n", pstVidCode->VidIdCode, VidIdCode);
		} else if ((encFmt == MT_DRV_DISP_FMT_NTSC) ||
				   (encFmt == MT_DRV_DISP_FMT_NTSC_J) ||
				   (encFmt == MT_DRV_DISP_FMT_NTSC_443) ||
				   (encFmt == MT_DRV_DISP_FMT_1440x480i_60)) {
			VidIdCode = 7;
			COM_INFO("Sepcail setting:change pstVidCode(480i_60 16:9):%d-->%d\n", pstVidCode->VidIdCode, VidIdCode);
		} else if (DRV_Get_Is4KFmt(encFmt)) {
			HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(MT_UNF_HDMI_ID_0);
			SiiLibEdidPar_t parseEdid = {0};

			SiiDrvTxEdidParseGet(DRV_HDMI_Get_TxInst(), &parseEdid);
			if ( (pstVidAttr->b3DEnable) == 0 && parseEdid.scdc.bUHD_VIC == 0) {
				VidIdCode = 0;
				COM_INFO("2D mode, Sepcail setting:change pstVidCode(4K):%d-->%d\n", pstVidCode->VidIdCode, VidIdCode);
			} else {
				COM_INFO("do not need to change VIC\n");
			}
		} else {
			COM_INFO("do not need to change VIC\n");
		}
	}

	pu8AviInfoFrame[3] = (MT_U8)(VidIdCode & 0x7F);
	/* Fill Data byte 5: Pixel repetition, Bit0~3:PR0~PR3 */
	/*
	   PR3 PR2 PR1 PR0 Pixel Repetition Factor
	   0   0   0    0   No Repetition (i.e., pixel sent once)
	   0   0   0    1   pixel sent 2 times (i.e., repeated once)
	   0   0   1    0   pixel sent 3 times
	   0   0   1    1   pixel sent 4 times
	   0   1   0    0   pixel sent 5 times
	   0   1   0    1   pixel sent 6 times
	   0   1   1    0   pixel sent 7 times
	   0   1   1    1   pixel sent 8 times
	   1   0   0    0   pixel sent 9 times
	   1   0   0    1   pixel sent 10 times
	   0Ah-0Fh          Reserved
	   */
	u8AviInfoFrameByte = (MT_U8)(punAVIInfoFrame->u32PixelRepetition & 0x0F);

	/* Fill Data byte 5: Content Type, Bit4~5:CN0~CN1 */
	/*
	   ITC  CN1 CN0 Pixel Repetition Factor
	   (1)   0    0   Graphics
	   (1)   0    1   Photo
	   (1)   1    0   Cinema
	   (1)   1    1   Game
	   */
	switch (punAVIInfoFrame->enContentType) {
		case HDMI_CONTNET_GRAPHIC:
			u8AviInfoFrameByte |= (MT_U8)0x00;
			break;
		case HDMI_CONTNET_PHOTO:
			u8AviInfoFrameByte |= (MT_U8)0x10;
			break;
		case HDMI_CONTNET_CINEMA:
			u8AviInfoFrameByte |= (MT_U8)0x20;
			break;
		case HDMI_CONTNET_GAME:
			u8AviInfoFrameByte |= (MT_U8)0x30;
			break;
		default:
			u8AviInfoFrameByte |= (MT_U8)0x00;
			break;
	}
	/* Fill Data byte 5: YCC Full Range, Bit6~7:YQ0~YQ1 */
	/*
	   YQ1 YQ0 Pixel Repetition Factor
	   0    0   Limitation Range
	   0    1   Full Range
	   */
	switch (punAVIInfoFrame->enYCCQuantization) {
		case HDMI_YCC_QUANTIZATION_LIMITED_RANGE:
			u8AviInfoFrameByte |= (MT_U8)0x00;
			break;
		case HDMI_YCC_QUANTIZATION_FULL_RANGE:
			u8AviInfoFrameByte |= (MT_U8)0x40;
			break;
		default:
			u8AviInfoFrameByte |= (MT_U8)0x00;
			break;
	}
	pu8AviInfoFrame[4] = (MT_U8)(u8AviInfoFrameByte & 0XFF);

	if ( (0 == punAVIInfoFrame->u32LineNEndofTopBar) && (0 == punAVIInfoFrame->u32LineNStartofBotBar)
			&& (0 == punAVIInfoFrame->u32PixelNEndofLeftBar) && (0 == punAVIInfoFrame->u32PixelNStartofRightBar) ) {
		punAVIInfoFrame->u32LineNEndofTopBar      = pstVidCode->Active_X;
		punAVIInfoFrame->u32LineNStartofBotBar    = pstVidCode->Active_H;
		punAVIInfoFrame->u32PixelNEndofLeftBar    = pstVidCode->Active_Y;
		punAVIInfoFrame->u32PixelNStartofRightBar = pstVidCode->Active_W;
	}
	if (punAVIInfoFrame->enBarInfo == HDMI_BAR_INFO_NOT_VALID) {
		punAVIInfoFrame->u32LineNEndofTopBar      = 0;
		punAVIInfoFrame->u32LineNStartofBotBar    = 0;
		punAVIInfoFrame->u32PixelNEndofLeftBar    = 0;
		punAVIInfoFrame->u32PixelNStartofRightBar = 0;
	}
	/* Fill Data byte 6  */
	pu8AviInfoFrame[5] = (MT_U8)(punAVIInfoFrame->u32LineNEndofTopBar & 0XFF);

	/* Fill Data byte 7  */
	pu8AviInfoFrame[6] = (MT_U8)((punAVIInfoFrame->u32LineNEndofTopBar >> 8) & 0XFF);

	/* Fill Data byte 8  */
	pu8AviInfoFrame[7] = (MT_U8)(punAVIInfoFrame->u32LineNStartofBotBar & 0XFF);

	/* Fill Data byte 9  */
	pu8AviInfoFrame[8] = (MT_U8)((punAVIInfoFrame->u32LineNStartofBotBar >> 8) & 0XFF);

	/* Fill Data byte 10  */
	pu8AviInfoFrame[9] = (MT_U8)(punAVIInfoFrame->u32PixelNEndofLeftBar & 0XFF);

	/* Fill Data byte 11  */
	pu8AviInfoFrame[10] = (MT_U8)((punAVIInfoFrame->u32PixelNEndofLeftBar >> 8) & 0XFF);

	/* Fill Data byte 12  */
	pu8AviInfoFrame[11] = (MT_U8)(punAVIInfoFrame->u32PixelNStartofRightBar & 0XFF);

	/* Fill Data byte 13  */
	pu8AviInfoFrame[12] = (MT_U8)((punAVIInfoFrame->u32PixelNStartofRightBar >> 8) & 0XFF);

	return retval;
}

static mt_u32 hdmi_Create_Audio_Infoframe(MT_UNF_HDMI_AUD_INFOFRAME_VER1_S *punAUDInfoFrame, MT_U8 *pu8AudioInfoFrame)
{
	MT_U8 u8AudioInfoFrameByte = 0;
	mt_s32 retval = MT_SUCCESS;
	u8AudioInfoFrameByte = 0;

	switch (punAUDInfoFrame->u32ChannelCount) {
		case 2 :
			u8AudioInfoFrameByte |= 0x01;
			break;
		case 3 :
			u8AudioInfoFrameByte |= 0x02;
			break;
		case 4 :
			u8AudioInfoFrameByte |= 0x03;
			break;
		case 5 :
			u8AudioInfoFrameByte |= 0x04;
			break;
		case 6 :
			u8AudioInfoFrameByte |= 0x05;
			break;
		case 7 :
			u8AudioInfoFrameByte |= 0x06;
			break;
		case 8 :
			u8AudioInfoFrameByte |= 0x07;
			break;
		default :
			u8AudioInfoFrameByte |= 0x00;
			break;
	}

	switch (punAUDInfoFrame->enCodingType) {
		#if 0
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM :
			u8AudioInfoFrameByte |= 0x10;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3 :
			u8AudioInfoFrameByte |= 0x20;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_MPEG1 :
			u8AudioInfoFrameByte |= 0x30;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_MP3 :
			u8AudioInfoFrameByte |= 0x40;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_MPEG2 :
			u8AudioInfoFrameByte |= 0x50;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_AAC :
			u8AudioInfoFrameByte |= 0x60;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS :
			u8AudioInfoFrameByte |= 0x70;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_ATRAC :
			u8AudioInfoFrameByte |= 0x80;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_ONE_BIT :
			u8AudioInfoFrameByte |= 0x90;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP :
			u8AudioInfoFrameByte |= 0xA0;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS_HD :
			u8AudioInfoFrameByte |= 0xB0;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_MAT :
			u8AudioInfoFrameByte |= 0xC0;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_DST :
			u8AudioInfoFrameByte |= 0xD0;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_WMA_PRO :
			u8AudioInfoFrameByte |= 0xE0;
			break;
		#endif
		default :
			u8AudioInfoFrameByte |= 0x00;
			break;
	}
	pu8AudioInfoFrame[0] = (MT_U8)(u8AudioInfoFrameByte & 0xF7);

	u8AudioInfoFrameByte = 0;
	/* Fill Sample Size (Data Byte 2) bits2: 0~1*/
	/*
	   SS1 SS0    Sample Size
	   0   0      Refer to Stream header
	   0   1      16 bit
	   1   0      20 bit
	   1   1      24 bit
	   */
	switch (punAUDInfoFrame->u32SampleSize) {
		case 16 :
			u8AudioInfoFrameByte |= 0x01;
			break;
		case 20 :
			u8AudioInfoFrameByte |= 0x02;
			break;
		case 24 :
			u8AudioInfoFrameByte |= 0x03;
			break;
		default :
			u8AudioInfoFrameByte |= 0x00;
			break;
	}

	/* Fill Sample Frequency (Data Byte 2)bits3: 2~4*/
	/*
	   SF2 SF1 SF0 Sampling Frequency
	   0   0   0   Refer to Stream Header
	   0   0   1   32 kHz
	   0   1   0   44.1 kHz (CD)
	   0   1   1   48 kHz
	   1   0   0   88.2 kHz
	   1   0   1   96 kHz
	   1   1   0   176.4 kHz
	   1   1   1   192 kHz
	   */
	switch (punAUDInfoFrame->u32SamplingFrequency) {
		case 32000 :
			u8AudioInfoFrameByte |= 0x04;
			break;
		case 44100 :
			u8AudioInfoFrameByte |= 0x08;
			break;
		case 48000 :
			u8AudioInfoFrameByte |= 0x0C;
			break;
		case 88200 :
			u8AudioInfoFrameByte |= 0x10;
			break;
		case 96000 :
			u8AudioInfoFrameByte |= 0x14;
			break;
		case 176400 :
			u8AudioInfoFrameByte |= 0x18;
			break;
		case 192000 :
			u8AudioInfoFrameByte |= 0x1C;
			break;
		default :
			u8AudioInfoFrameByte |= 0x00;
			break;
	}
	pu8AudioInfoFrame[1] = (MT_U8)(u8AudioInfoFrameByte & 0x1F);

	u8AudioInfoFrameByte = 0;
	/* Fill the Bit rate coefficient for the compressed audio format (Data byte 3)*/
	switch (punAUDInfoFrame->enCodingType) {
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3 :
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS :
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_MPEG1 :
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_MPEG2 :
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_MP3 :
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_AAC :
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_MAT :
			//pu8AudioInfoFrame[2] = (MT_U8)0XFF;//? Data Byte 3 is reserved and shall be zero.
			pu8AudioInfoFrame[2] = 0X00;
			break;
		case MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM :
		default :
			pu8AudioInfoFrame[2] = 0X00;
			break;
	}

	/* Data Bytes 4 and 5 apply only to multi-channel (i.e., more than two channels) uncompressed audio. */
	/* Fill Channel allocation (Data Byte 4) */
	/*
	   CA(binary)       CA(hex)  Channel Number
	   7 6 5 4 3 2 1 0          8 7 6 5 4 3 2 1
	   0 0 0 0 0 0 0 0  00      - - - - - - FR FL
	   0 0 0 0 0 0 0 1  01      - - - - - LFE FR FL
	   0 0 0 0 0 0 1 0  02      - - - - FC - FR FL
	   0 0 0 0 0 0 1 1  03      - - - - FC LFE FR FL
	   0 0 0 0 0 1 0 0  04      - - - RC - - FR FL
	   0 0 0 0 0 1 0 1  05      - - - RC - LFE FR FL
	   0 0 0 0 0 1 1 0  06      - - - RC FC - FR FL
	   0 0 0 0 0 1 1 1  07      - - - RC FC LFE FR FL
	   0 0 0 0 1 0 0 0  08      - - RR RL - - FR FL
	   0 0 0 0 1 0 0 1  09      - - RR RL - LFE FR FL
	   0 0 0 0 1 0 1 0  0A      - - RR RL FC - FR FL
	   0 0 0 0 1 0 1 1  0B      - - RR RL FC LFE FR FL
	   0 0 0 0 1 1 0 0  0C      - RC RR RL - - FR FL
	   0 0 0 0 1 1 0 1  0D      - RC RR RL - LFE FR FL
	   0 0 0 0 1 1 1 0  0E      - RC RR RL FC - FR FL
	   0 0 0 0 1 1 1 1  0F      - RC RR RL FC LFE FR FL
	   0 0 0 1 0 0 0 0  10      RRC RLC RR RL - - FR FL
	   0 0 0 1 0 0 0 1  11      RRC RLC RR RL - LFE FR FL
	   0 0 0 1 0 0 1 0  12      RRC RLC RR RL FC - FR FL
	   0 0 0 1 0 0 1 1  13      RRC RLC RR RL FC LFE FR FL
	   0 0 0 1 0 1 0 0  14      FRC FLC - - - - FR FL
	   0 0 0 1 0 1 0 1  15      FRC FLC - - - LFE FR FL
	   0 0 0 1 0 1 1 0  16      FRC FLC - - FC - FR FL
	   0 0 0 1 0 1 1 1  17      FRC FLC - - FC LFE FR FL
	   0 0 0 1 1 0 0 0  18      FRC FLC - RC - - FR FL
	   0 0 0 1 1 0 0 1  19      FRC FLC - RC - LFE FR FL
	   0 0 0 1 1 0 1 0  1A      FRC FLC - RC FC - FR FL
	   0 0 0 1 1 0 1 1  1B      FRC FLC - RC FC LFE FR FL
	   0 0 0 1 1 1 0 0  1C      FRC FLC RR RL - - FR FL
	   0 0 0 1 1 1 0 1  1D      FRC FLC RR RL - LFE FR FL
	   0 0 0 1 1 1 1 0  1E      FRC FLC RR RL FC - FR FL
	   0 0 0 1 1 1 1 1  1F      FRC FLC RR RL FC LFE FR FL
	   */
	pu8AudioInfoFrame[3] = (MT_U8)(punAUDInfoFrame->u32ChannelAlloc & 0XFF);

	/* Fill Level Shift (Data Byte 5) bits4:3~7 */
	/*
	   LSV3 LSV2 LSV1 LSV0 Level Shift Value
	   0     0    0    0     0dB
	   0     0    0    1     1dB
	   0     0    1    0     2dB
	   0     0    1    1     3dB
	   0     1    0    0     4dB
	   0     1    0    1     5dB
	   0     1    1    0     6dB
	   0     1    1    1     7dB
	   1     0    0    0     8dB
	   1     0    0    1     9dB
	   1     0    1    0    10dB
	   1     0    1    1    11dB
	   1     1    0    0    12dB
	   1     1    0    1    13dB
	   1     1    1    0    14dB
	   1     1    1    1    15dB
	   */
	switch (punAUDInfoFrame->u32LevelShift) {
		case 0 :
			u8AudioInfoFrameByte |= 0x00;
			break;
		case 1 :
			u8AudioInfoFrameByte |= 0x08;
			break;
		case 2 :
			u8AudioInfoFrameByte |= 0x10;
			break;
		case 3 :
			u8AudioInfoFrameByte |= 0x18;
			break;
		case 4 :
			u8AudioInfoFrameByte |= 0x20;
			break;
		case 5 :
			u8AudioInfoFrameByte |= 0x28;
			break;
		case 6 :
			u8AudioInfoFrameByte |= 0x30;
			break;
		case 7 :
			u8AudioInfoFrameByte |= 0x38;
			break;
		case 8 :
			u8AudioInfoFrameByte |= 0x40;
			break;
		case 9 :
			u8AudioInfoFrameByte |= 0x48;
			break;
		case 10 :
			u8AudioInfoFrameByte |= 0x50;
			break;
		case 11 :
			u8AudioInfoFrameByte |= 0x58;
			break;
		case 12 :
			u8AudioInfoFrameByte |= 0x60;
			break;
		case 13 :
			u8AudioInfoFrameByte |= 0x68;
			break;
		case 14 :
			u8AudioInfoFrameByte |= 0x70;
			break;
		case 15 :
			u8AudioInfoFrameByte |= 0x78;
			break;
		default :
			retval = MT_FAILURE;
			break;
	}
	/* Fill Down mix inhibit flag bit7 */
	if (punAUDInfoFrame->u32DownmixInhibit) {
		u8AudioInfoFrameByte |= 0x80;
	} else {
		u8AudioInfoFrameByte &= ~0x80;
	}
	pu8AudioInfoFrame[4] = (MT_U8)(u8AudioInfoFrameByte & 0xF8);

	return retval;
}

static mt_void hdmi_set_infoframe_cksum(info_struct_t *info)
{
	mt_u8 cksum = 0;
	mt_u8 i;

	info->pb_byte[0] = 0; //clear cksum

	cksum += info->type;
	cksum += info->version;
	cksum += info->length;
	for (i = 1; i < info->length; i++) {
		cksum += info->pb_byte[i];
	}
	cksum = 256 - cksum;
	info->pb_byte[0] = cksum;
}

static mt_u32 hdmi_SetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame)
{
	mt_s32 siRet = MT_SUCCESS;
	info_struct_t info;
	SiiInfoFrame_t info_st = {0};

	HDMI_CHECK_NULL_PTR(pstInfoFrame);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	COM_INFO("pstInfoFrame->enInfoFrameType %d \n", pstInfoFrame->enInfoFrameType);
	memset(&info, 0, sizeof(info_struct_t));
	memset(&info_st, 0, sizeof(SiiInfoFrame_t));
	switch (pstInfoFrame->enInfoFrameType) {
		case MT_INFOFRAME_TYPE_AVI: {
			/*The InfoFrame provided by HDMI is limited to 30 bytes plus a checksum byte.*/
			MT_U8 pu8AviInfoFrame[32];
			MT_UNF_HDMI_AVI_INFOFRAME_VER2_S *pstAviInfoFrm = DRV_Get_AviInfoFrm(enHdmi);

			memset(pu8AviInfoFrame, 0, 32);
			siRet = hdmi_Create_AVI_Infoframe((MT_UNF_HDMI_AVI_INFOFRAME_VER2_S *) & (pstInfoFrame->unInforUnit.stAVIInfoFrame), pu8AviInfoFrame);
			memcpy(pstAviInfoFrm, &(pstInfoFrame->unInforUnit.stAVIInfoFrame), sizeof(MT_UNF_HDMI_AVI_INFOFRAME_VER2_S));

			if (DRV_Get_IsMce2App()) {
				//printk("%s.%d \n",__FUNCTION__,__LINE__);
				//return MT_SUCCESS;
			}

			/* Set relative Register in HDMI IP */
			info.type = 0x82;
			info.version = 0x02;
			info.length = 0x0D;
			memcpy(&(info.pb_byte[1]), pu8AviInfoFrame, SII_INFOFRAME_AVI_MAX_LEN - 4);
			hdmi_set_infoframe_cksum(&info);
			info_st.ifId = SII_INFO_FRAME_ID__AVI;
			SII_MEMCPY((uint8_t*)&info_st.b, &info, SII_INFOFRAME_AVI_MAX_LEN);
			SiiDrvTxInfoframeSet(DRV_HDMI_Get_TxInst(), &info_st);
			siRet = MT_SUCCESS;
			break;
		}
		case MT_INFOFRAME_TYPE_SPD:
			break;
		case MT_INFOFRAME_TYPE_AUDIO: {
			MT_U8 pu8AudioInfoFrame[32];
			MT_UNF_HDMI_AUD_INFOFRAME_VER1_S *pstAudInfoFrm = DRV_Get_AudInfoFrm(enHdmi);

			memset(pu8AudioInfoFrame, 0, 32);
			hdmi_Create_Audio_Infoframe((MT_UNF_HDMI_AUD_INFOFRAME_VER1_S *) & (pstInfoFrame->unInforUnit.stAUDInfoFrame), pu8AudioInfoFrame);
			memcpy(pstAudInfoFrm, &(pstInfoFrame->unInforUnit.stAUDInfoFrame), sizeof(MT_UNF_HDMI_AUD_INFOFRAME_VER1_S));

			info.type = 0x84;
			info.version = 0x01;
			info.length = 0x0A;
			memcpy(&(info.pb_byte[1]), pu8AudioInfoFrame, SII_INFOFRAME_AVI_MAX_LEN - 4);
			hdmi_set_infoframe_cksum(&info);
			info_st.ifId = SII_INFO_FRAME_ID__AUDIO;
			SII_MEMCPY((uint8_t*)&info_st.b, &info, SII_INFOFRAME_AVI_MAX_LEN);
			SiiDrvTxInfoframeSet(DRV_HDMI_Get_TxInst(), &info_st);
			break;
		}
		case MT_INFOFRAME_TYPE_MPEG:
			break;
		case MT_INFOFRAME_TYPE_VENDORSPEC:
			break;
		default:
			break;
	}
	return siRet;
}

static mt_u32 hdmi_GetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame)
{
	mt_s32 siRet = MT_SUCCESS;

	HDMI_CHECK_NULL_PTR(pstInfoFrame);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	memset(pstInfoFrame, 0, sizeof(MT_UNF_HDMI_INFOFRAME_S));
	switch (enInfoFrameType) {
		case MT_INFOFRAME_TYPE_AVI: {
			MT_UNF_HDMI_AVI_INFOFRAME_VER2_S *pstAviInfoFrm = DRV_Get_AviInfoFrm(enHdmi);

			pstInfoFrame->enInfoFrameType = MT_INFOFRAME_TYPE_AVI;
			memcpy(&(pstInfoFrame->unInforUnit.stAVIInfoFrame), pstAviInfoFrm, sizeof(MT_UNF_HDMI_AVI_INFOFRAME_VER2_S));
			break;
		}
		case MT_INFOFRAME_TYPE_AUDIO: {
			MT_UNF_HDMI_AUD_INFOFRAME_VER1_S *pstAudInfoFrm = DRV_Get_AudInfoFrm(enHdmi);

			pstInfoFrame->enInfoFrameType = MT_INFOFRAME_TYPE_AUDIO;
			memcpy(&(pstInfoFrame->unInforUnit.stAUDInfoFrame), pstAudInfoFrm, sizeof(MT_UNF_HDMI_AUD_INFOFRAME_VER1_S));
			break;
		}
		default:
			siRet = MT_FAILURE;
			break;
	}

	return siRet;
}

mt_u32 DRV_HDMI_SetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame)
{
	mt_u32 Ret;
	COM_INFO("Enter DRV_HDMI_SetInfoFrame\n");
	SI_timer_count();
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pstInfoFrame);

	if (pstInfoFrame->unInforUnit.stAVIInfoFrame.enOutputType == MT_UNF_HDMI_VIDEO_MODE_YCBCR422) {
		COM_INFO("%s.%d : SetInfoFrame YCBCR422 return \n", __FUNCTION__, __LINE__);
		//return MT_ERR_HDMI_INVALID_PARA;
	}

	Ret = hdmi_SetInfoFrame(enHdmi, pstInfoFrame);
	SI_timer_count();
	COM_INFO("Leave DRV_HDMI_SetInfoFrame\n");
	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_GetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame)
{
	mt_u32 Ret;
	COM_INFO("Enter DRV_HDMI_GetInfoFrame\n");
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pstInfoFrame);

	Ret = hdmi_GetInfoFrame(enHdmi, enInfoFrameType, pstInfoFrame);

	COM_INFO("Leave DRV_HDMI_GetInfoFrame\n");
	return MT_SUCCESS;
}
#if defined (HDCP_SUPPORT)
	static mt_u32 HDCP_FailCount = 0;
#endif

#ifdef DEBUG_NOTIFY_COUNT
	mt_u32 hpd_changeCount = 0;
#endif
mt_u32 DRV_HDMI_ReadEvent(MT_UNF_HDMI_ID_E enHdmi, mt_u32 procID)
{
	mt_s32 s32Ret = -1;
	mt_u32 index, event = 0;

	HDMI_PROC_EVENT_S *pEventList = DRV_Get_EventList(enHdmi);
	mt_u32 u32EventNo = pEventList[procID].CurEventNo;

	if (procID >= MAX_PROCESS_NUM) {
		COM_ERR("Invalid procID in ReadEvent\n");
		return MT_UNF_HDMI_EVENT_BUTT;
	}

	//We deal with HDMI Event only after HDMI opened!
	if (DRV_Get_IsChnOpened(enHdmi)) {
		//DEBUG_PRINTK("msecs_to_jiffies(100):%d\n", msecs_to_jiffies(100));

		//s32Ret = wait_event_interruptible_timeout(g_astHDMIWait, g_HDMIWaitFlag, (mt_u32)msecs_to_jiffies(100));
		s32Ret = wait_event_interruptible_timeout(g_astHDMIWait, g_HDMIWaitFlag[procID], (mt_u32)msecs_to_jiffies(10));
		if (s32Ret <= 0) {
			return 0;
		}

		if (g_Event_Count[procID] > MIX_EVENT_COUNT) {
			g_Event_Count[procID]--;
		} else {
			g_HDMIWaitFlag[procID] = MT_FALSE;
		}

		COM_INFO("Ha we get a event!!!!!!\n");
		SI_timer_count();

		#ifdef DEBUG_EVENTLIST
		COM_WARN("read start\n");
		COM_WARN("\n procID %d CurEventNo %d \n", procID, pEventList[procID].CurEventNo);
		for (index = 0; index < PROC_EVENT_NUM; index++) {
			COM_WARN("____eventlist %d: 0x%x_____\n", index, pEventList[procID].Event[index]);
		}
		#endif

		HDMI_EVENT_LOCK();
		event = 0;
		for (index = 0; index < PROC_EVENT_NUM; index ++) {

			if ((pEventList[procID].Event[u32EventNo] >= MT_UNF_HDMI_EVENT_HOTPLUG)
					&& (pEventList[procID].Event[u32EventNo] < MT_UNF_HDMI_EVENT_MAX)) {
				event = pEventList[procID].Event[u32EventNo];
				pEventList[procID].Event[u32EventNo] = 0;
				if (event == MT_UNF_HDMI_EVENT_HOTPLUG) {
					if (MT_TRUE == SI_Is_HPDKernelCallback_DetectHPD()) {
						#ifdef DEBUG_NOTIFY_COUNT
						hpd_changeCount++;
						HPD_ERR("Warnning:Detect Hotplug Event,But hotplug signal is low! %d times \n", hpd_changeCount);
						#endif
						event = 0;
					}
					break;
				}
				#if defined (HDCP_SUPPORT)
				else if (event == MT_UNF_HDMI_EVENT_HDCP_FAIL) {
					HDCP_FailCount ++;
					if (HDCP_FailCount >= 50) {
						HDCP_FailCount = 0;
						HDCP_ERR("HDCP Authentication Fail times:50!\n");
					}
					break;
				}
				#endif
				else {
					COM_WARN("event:%d\n", event);
					break;
				}
			}
			u32EventNo = (u32EventNo + 1) % PROC_EVENT_NUM;
			#ifdef DEBUG_EVENTLIST
			COM_WARN("u32EventNo : %d \n", u32EventNo);
			#endif
		}
		HDMI_EVENT_UNLOCK();
	}
	COM_INFO("line:%d,event:%d\n", __LINE__, event);

	#ifdef DEBUG_EVENTLIST
	COM_WARN("read over\n");
	COM_WARN("\n procID %d CurEventNo %d \n", procID, pEventList[procID].CurEventNo);
	for (index = 0; index < PROC_EVENT_NUM; index++) {
		COM_WARN("____eventlist %d: 0x%x_____\n", index, pEventList[procID].Event[index]);
	}
	#endif

	return event;
}

static void hdmi_ProcEvent(MT_UNF_HDMI_EVENT_TYPE_E event, mt_u32 procID)
{
	HDMI_PROC_EVENT_S *pEventList = DRV_Get_EventList(MT_UNF_HDMI_ID_0);
	COM_INFO("line:%d,event:%d,g_UserCallbackFlag:%d\n", __LINE__, event, g_UserCallbackFlag);

	/*
	//for avoid Silicon image 40Pll phy oe problem
	  when hotplug occured,we need stop open oe in any time.
	  but in boot,we can not detect hotplug changed,so display blank in some tv
	  we need close & open oe first time in these TV
	*/
	Check1stOE(MT_UNF_HDMI_ID_0);

	if (procID >= MAX_PROCESS_NUM) {
		COM_ERR("Invalid procID:%d in hdmi_ProcEvent\n", procID);
		return;
	}

	if (g_UserCallbackFlag == HDMI_CALLBACK_USER) { //app
		mt_u32 u32CurEvent = pEventList[procID].CurEventNo;
		#ifdef DEBUG_EVENTLIST
		int i;
		#endif
		HDMI_EVENT_LOCK();
		g_Event_Count[procID]++;

		pEventList[procID].Event[u32CurEvent] = event;
		pEventList[procID].CurEventNo = (u32CurEvent + 1) % PROC_EVENT_NUM;

		#ifdef DEBUG_EVENTLIST
		MT_PRINT("\n procID %d CurEventNo %d \n", procID, pEventList[procID].CurEventNo);
		for (i = 0; i < PROC_EVENT_NUM; i++) {
			MT_PRINT("____eventlist %d: 0x%x_____\n", i, pEventList[procID].Event[i]);
		}
		#endif

		HDMI_EVENT_UNLOCK();
		g_HDMIWaitFlag[procID] = MT_TRUE;
		wake_up(&g_astHDMIWait);
		COM_INFO("callback finish wake up event g_HDMIWaitFlag:0x%x\n", g_HDMIWaitFlag[procID]);
	} else if (g_UserCallbackFlag == HDMI_CALLBACK_KERNEL) { //mce
		switch ( event ) {
			case MT_UNF_HDMI_EVENT_HOTPLUG:
				//when unf_init after load ko too fast, maybe two hotplug will be trigered in the same time.
				HDMI_EVENT_LOCK();
				hdmi_MCE_ProcHotPlug(MT_UNF_HDMI_ID_0);
				HDMI_EVENT_UNLOCK();
				break;
				#if defined (HDCP_SUPPORT)
			case MT_UNF_HDMI_EVENT_HDCP_SUCCESS:
				MT_PRINT("\n MCE HDMI event: HDCP_SUCCESS!\n");
				break;
				#endif
			default:
				break;
		}
	}

	return;
}

#ifdef DEBUG_NOTIFY_COUNT
	mt_u32 NotifyCount = 0;
#endif

void DRV_HDMI_NotifyEvent(MT_UNF_HDMI_EVENT_TYPE_E event)
{
	mt_u32 u32ProcIndex = 0;
	HDMI_PROC_EVENT_S *pEventList = DRV_Get_EventList(MT_UNF_HDMI_ID_0);

	#ifdef DEBUG_NOTIFY_COUNT
	if (event != MT_UNF_HDMI_EVENT_HDCP_USERSETTING) {
		NotifyCount++;
		MT_PRINT("\n **** Notify %d times : event 0x%x **** \n", NotifyCount, event);
	}
	#endif

	COM_INFO("HDMI EVENT TYPE:0x%x\n", event);
	if (DRV_Get_IsChnOpened(MT_UNF_HDMI_ID_0)) {
		if ((event == MT_UNF_HDMI_EVENT_HOTPLUG)) {
			#if defined (CEC_SUPPORT)
			HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();
			#endif

			SI_EnableInterrupts();
			#if defined (CEC_SUPPORT)
			SI_CEC_SetUp();
			pstChnAttr[MT_UNF_HDMI_ID_0].u8CECCheckCount = 0;
			#endif
			SI_HPD_SetHPDUserCallbackCount();

			//hdmi_AdjustAVIInfoFrame(MT_UNF_HDMI_ID_0);
			hdmi_SetAndroidState(STATE_HOTPLUGIN);
		}
		#if defined (HDCP_SUPPORT)
		else if (event == MT_UNF_HDMI_EVENT_HDCP_USERSETTING) {
			//special doing!!
			//DRV_HDMI_Start(MT_UNF_HDMI_ID_0);
			return;
		}
		#endif
		else if (event == MT_UNF_HDMI_EVENT_NO_PLUG) {
			if (g_HDMIUserInitNum) {
				#if defined (CEC_SUPPORT)
				HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();
				#endif
				//0x72:0x08 powerdown is only needed in isr && cec
				/* Close HDMI Output */
				//SI_PowerDownHdmiTx();
				//SI_SetHdmiVideo(MT_FALSE);
				//SI_SetHdmiAudio(MT_FALSE);
				//SI_DisableHdmiDevice();
				//DRV_Set_ChnStart(MT_UNF_HDMI_ID_0, MT_FALSE);
				#if defined (HDCP_SUPPORT)
				/*Set HDCP Off */
				//SI_WriteByteEEPROM(EE_TX_HDCP, 0x00);
				#endif
				#if defined (CEC_SUPPORT)
				//Close CEC
				SI_CEC_Close();
				//DRV_Set_CECStart(MT_UNF_HDMI_ID_0, MT_FALSE);
				pstChnAttr[MT_UNF_HDMI_ID_0].u8CECCheckCount = 0;
				memset(&(pstChnAttr[MT_UNF_HDMI_ID_0].stCECStatus), 0, sizeof(MT_UNF_HDMI_CEC_STATUS_S));
				#endif
			} else if (g_HDMIKernelInitNum) {
				//if oe not matching hotplug,some err will occured in phy

				//0x72:0x08 powerdown is only needed in isr && cec
				/* Close HDMI Output */
				//SI_PowerDownHdmiTx();
				//SI_SetHdmiVideo(MT_FALSE);
				//SI_SetHdmiAudio(MT_FALSE);
				//SI_DisableHdmiDevice();
			}

			hdmi_SetAndroidState(STATE_HOTPLUGOUT);
		} else if (event == MT_UNF_HDMI_EVENT_EDID_FAIL) {

		}
		#if defined (HDCP_SUPPORT)
		else if (event == MT_UNF_HDMI_EVENT_HDCP_FAIL) {

		} else if (event == MT_UNF_HDMI_EVENT_HDCP_SUCCESS) {
			SI_timer_stop();
		}
		#endif
		else if (event == MT_UNF_HDMI_EVENT_RSEN_CONNECT) {
			COM_INFO("CONNECT Event:0x%x\n", event);
		} else if (event == MT_UNF_HDMI_EVENT_RSEN_DISCONNECT) {
			COM_INFO("DISCONNECT Event:0x%x\n", event);
		} else {
			COM_INFO("Unknow Event:0x%x\n", event);
		}
		COM_INFO("line:%d,event:%d\n", __LINE__, event);
		for (u32ProcIndex = 0; u32ProcIndex < MAX_PROCESS_NUM; u32ProcIndex++) {
			if (MT_TRUE == pEventList[u32ProcIndex].bUsed) {
				COM_INFO("proc id %d bUsed %d\n", u32ProcIndex, pEventList[u32ProcIndex].bUsed);
				hdmi_ProcEvent(event, u32ProcIndex);
			}
		}

		SI_timer_count();
	}
}

mt_u32 DRV_HDMI_Start(MT_UNF_HDMI_ID_E enHdmi)
{
	HDMI_APP_ATTR_S     *pstAppAttr = DRV_Get_AppAttr(enHdmi);
	mt_u32 skip_hdcp = 0;

	COM_INFO("Enter DRV_HDMI_Start\n");
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	SI_timer_count();

	if (DRV_Get_IsMce2App() || DRV_Get_IsOpenedInBoot()) {
		DRV_Set_Mce2App(MT_FALSE);
		DRV_Set_OpenedInBoot(MT_FALSE);
		DRV_Set_ChnStart(enHdmi, MT_TRUE);
		SI_EnableHdmiDevice();
	} else {
		/* Enable HDMI Ouptut */
		if (MT_TRUE == pstAppAttr->bEnableHdmi) {
			if (!SI_TX_IsHDMImode()) {
				COM_INFO("-->start: SI_Start_HDMITX\n");
				SI_Start_HDMITX();
				SI_TX_SetHDMIMode(ON);    //for hdmi
			}
		}
		#if defined (DVI_SUPPORT)
		else {
			if (SI_TX_IsHDMImode()) {
				MT_PRINT("-->start: SI_Init_DVITX\n");
				SI_Init_DVITX();
				SI_TX_SetHDMIMode(OFF);    //for dvi
			}
		}
		#endif

		SI_timer_count();

		/* Now we wake up HDMI Output */
		SI_WakeUpHDMITX();
		#if defined (HDCP_SUPPORT)
		if ( MT_TRUE == pstAppAttr->bHDCPEnable ) {
			SiiDrvHdcpStatus_t sts = SII_DRV_HDCP_STATUS__OFF;
			SiiDrvTxHdcpStateStatusGet(DRV_HDMI_Get_TxInst(), &sts);
			COM_INFO("hddd:%d\n",sts);
			if ( sts == SII_DRV_HDCP_STATUS__SUCCESS_1X || sts == SII_DRV_HDCP_STATUS__SUCCESS_22 ) {
				skip_hdcp = 1;
			}
		}
		#endif
		if ( skip_hdcp == 0 && MT_TRUE == pstAppAttr->bHDCPEnable) {
			SI_SendCP_Packet(MT_TRUE);
		}
		SI_EnableHdmiDevice();
		SiiLibTimeMilliDelay(100);
		DRV_Set_ChnStart(enHdmi, MT_TRUE);
		SI_timer_count();

		if ( skip_hdcp == 0 ) {
			#if defined (HDCP_SUPPORT)
			if (MT_TRUE == pstAppAttr->bHDCPEnable) {
				if (MT_TRUE == SI_Is_HPDKernelCallback_DetectHPD()) {
					/* HPD again before we try to open Auth 1*/
					HDCP_ERR("HPD Callback detect new HPD\n");
				} else if (MT_TRUE == SI_Is_HPDUserCallback_DetectHPD()) {
					/* HPD again before we try to open Auth 2*/
					HDCP_ERR("HPD Callback with Auth detect new HPD\n");
				} else {
					HDCP_INFO("try to open HDCP Auth\n");
					//SI_WriteByteEEPROM(EE_TX_HDCP, 0xFF);
					SI_hdcp_en(MT_TRUE, 1);
				}
			} else {
				SI_hdcp_en(MT_FALSE, 0);
				//SI_SetHdmiAudio(pstAppAttr->bEnableAudio);
				//SI_SetHdmiVideo(pstAppAttr->bEnableVideo);
				SI_SendCP_Packet(MT_FALSE);
			}
			SI_timer_count();

			if (MT_TRUE != pstAppAttr->bHDCPEnable) {
				SI_timer_stop();
			}
			#else
			//SI_SetHdmiAudio(pstAppAttr->bEnableAudio);
			//SI_SetHdmiVideo(pstAppAttr->bEnableVideo);
			SI_SendCP_Packet(MT_FALSE);
			#endif
		}else {
			SI_SendCP_Packet(MT_FALSE);
		}
	}
	COM_INFO("Leave DRV_HDMI_Start\n");
	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_Stop(MT_UNF_HDMI_ID_E enHdmi)
{
	COM_INFO("Enter DRV_HDMI_Stop\n");
	HDMI_CHECK_ID(enHdmi);

	//only in (init num == 1) case ,we can close output hdmi
	COM_INFO("Enter DRV_HDMI_Stop\n");
	if (DRV_HDMI_GetInitNum(enHdmi) != 1) {
		COM_INFO("DRV_HDMI_GetInitNum != 1 return \n");
		return MT_SUCCESS;
	}

	if (!DRV_Get_IsChnStart(enHdmi)) {
		COM_INFO("         == 0 return:%d \n", (mt_u32)DRV_Get_IsChnStart(enHdmi));
		return MT_SUCCESS;
	}

#if defined (HDCP_SUPPORT)
	{
		//Disable HDCP
		HDMI_APP_ATTR_S 	*pstAppAttr = DRV_Get_AppAttr(enHdmi);
		if (pstAppAttr->bHDCPEnable == MT_TRUE) {
			SI_hdcp_en(MT_FALSE, 0);
		}
	}
#endif

	SI_SendCP_Packet(ON);
	//SiiLibTimeMilliDelay(40);
	//SI_SetHdmiVideo(MT_FALSE);
	SI_SetHdmiAudio(MT_FALSE);
	SI_DisableHdmiDevice();
	//disable oe not need 10ms
	SI_PowerDownHdmiTx();

	//SiiLibTimeMilliDelay(400);
	DRV_Set_ChnStart(enHdmi, MT_FALSE);
	COM_INFO("Leave DRV_HDMI_Stop\n");
	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_SetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E enDeepColor)
{
	HDMI_ATTR_S *pstHDMIAttr = DRV_Get_HDMIAttr(MT_UNF_HDMI_ID_0);
	mt_u32               RetError = MT_SUCCESS;
	HDMI_ATTR_S pstHDMIAttrTmp = *pstHDMIAttr;

	COM_INFO("Enter DRV_HDMI_SetDeepColor\n");
	pstHDMIAttrTmp.stAppAttr.enDeepColorMode = enDeepColor;
	DRV_HDMI_SetAttr(MT_UNF_HDMI_ID_0, &pstHDMIAttrTmp);

	return RetError;
}

mt_u32 DRV_HDMI_GetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E *penDeepColor)
{
	HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();
	*penDeepColor = pstChnAttr->stAppAttrMt.stAppAttr.enDeepColorMode;
	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_SetxvYCCMode(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bEnable)
{
	//mt_u32 Ret;
	MT_U8 u8Data;
	HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(enHdmi);

	if (MT_TRUE == bEnable) {
		COM_INFO("enable xvYCC\n");
		/* enable Gamut Metadata InfoFrame transmission */

		/* Gamut boundary descriptions (GBD) and other gamut-related metadata
		   are carried using the Gamut Metadata Packet.*/
		if ( pstVidAttr->enVideoFmt <= MT_DRV_DISP_FMT_720P_50) {
			//Ret = SI_SendGamutMeta_Packet(MT_TRUE);
		} else {
			//Ret = SI_SendGamutMeta_Packet(MT_FALSE);
		}
		u8Data = 0x07;
	} else {
		COM_INFO("Disable xvYCC\n");
		/* disable Gamut Metadata InfoFrame transmission */
	}
	COM_INFO("end of MT_UNF_HDMI_SetxvYCCMode\n");

	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bAvMute)
{
	COM_INFO("Enter DRV_HDMI_SetAVMute, bAvMute:%d\n", bAvMute);
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	if (bAvMute) {
		SI_SendCP_Packet(MT_TRUE);
	} else {
		SI_SendCP_Packet(MT_FALSE);
	}

	COM_INFO("Leave DRV_HDMI_SetAVMute\n");
	return MT_SUCCESS;
}

static mt_u32 g_vcfg_state_flg = 0;
static mt_u32 g_force_update_tvsys = 0;
mt_void DRV_HDMI_Vcfg_Status(mt_u32 sts)
{
	HDMI20_DRV_HDMI_PRINTK("Vcfg state:last%d cur%d\n",g_vcfg_state_flg,sts);
	switch ( sts ) {
		case 0: //IDLE
			g_vcfg_state_flg = sts;
			break;
		case 1: //Pre set format
			if (g_vcfg_state_flg != 0) {
				HDMI20_DRV_HDMI_PRINTK("Vcfg Err state:last%d cur%d\n",g_vcfg_state_flg,sts);
			}
			g_vcfg_state_flg = sts;
			break;
		case 2: //Pre set video
			if (g_vcfg_state_flg != 1) {
				HDMI20_DRV_HDMI_PRINTK("Vcfg Err state:last%d cur%d\n",g_vcfg_state_flg,sts);
			}
			g_vcfg_state_flg = sts;
			break;
		case 3: //Set clock
			if (g_vcfg_state_flg != 2) {
				HDMI20_DRV_HDMI_PRINTK("Vcfg Err state:last%d cur%d\n",g_vcfg_state_flg,sts);
			}
			g_vcfg_state_flg = sts;
			break;
		case 4: //Set video
			if (g_vcfg_state_flg != 3) {
				HDMI20_DRV_HDMI_PRINTK("Vcfg Err state:last%d cur%d\n",g_vcfg_state_flg,sts);
			}
			g_vcfg_state_flg = sts;
			break;
		case 5: //start set format
			if (g_vcfg_state_flg != 4) {
				HDMI20_DRV_HDMI_PRINTK("Vcfg Err state:last%d cur%d\n",g_vcfg_state_flg,sts);
			}
			g_vcfg_state_flg = sts;
			break;
		case 6: //end set format
			if (g_vcfg_state_flg != 5) {
				HDMI20_DRV_HDMI_PRINTK("Vcfg Err state:last%d cur%d\n",g_vcfg_state_flg,sts);
			}
			g_vcfg_state_flg = 0;
			break;
		default:
			HDMI20_DRV_HDMI_PRINTK("Vcfg Err state:last%d cur%d\n",g_vcfg_state_flg,sts);
			break;

	}
}

mt_void DRV_HDMI_Set_Update_Tvsys(mt_u32 flg)
{
	g_force_update_tvsys = flg;
}

mt_u32 DRV_HDMI_Get_Update_Tvsys(mt_void)
{
	return g_force_update_tvsys;
}

mt_u32 DRV_HDMI_SwRst_Status_Get(mt_void)
{
	return (g_hdmi_sw_reset || g_vcfg_state_flg);
}

mt_u32 DRV_HDMI_Sw_Status_Get(mt_void)
{
	return g_vcfg_state_flg;
}
// The Procedures for SetFormat
// Set AV mute
// HW Reset controller
// Hw Reset Phy
// delay 50us
// Release Hw reset Phy
// cfg Phy
// Release HW reset for controller
// Delay 5ms for pll stable
mt_u32 DRV_HDMI_SetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enFmt, MT_DRV_DISP_STEREO_E enStereo)
{
	HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(enHdmi);
	HDMI_AUDIO_ATTR_S   *pstAudAttr = DRV_Get_AudioAttr(enHdmi);
	HDMI_APP_ATTR_S     *pstAppAttr = &(DRV_Get_AppAttrMt(enHdmi)->stAppAttr);

	MT_UNF_EDID_BASE_INFO_S    *pSinkCap = DRV_Get_SinkCap(enHdmi);
	MT_DRV_DISP_FMT_E                 enEncodingFormat = enFmt;
	mt_u32 delayTime = 0;

	#if defined (DEBUG_TIMER)
	mt_s32 start = 0, end = 0;

	SI_timer_start();
	start = SI_timer_count();
	#endif
	mt_s32 tout = 1000;//1s

	COM_INFO("Enter DRV_HDMI_SetFormat enEncodingFormat:%d enStereo %d \n", enEncodingFormat, enStereo);

	SI_timer_count();

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	if ((enEncodingFormat == MT_DRV_DISP_FMT_PAL) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_B) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_B1) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_D) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_D1) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_G) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_H) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_K) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_I) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_M) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_N) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_Nc) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_60) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_1440x576i_50) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_SIN) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_COS) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_L) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_B) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_G) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_D) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_K) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_H)) {
		enEncodingFormat = MT_DRV_DISP_FMT_PAL;
	} else if ((enEncodingFormat == MT_DRV_DISP_FMT_NTSC) ||
			   (enEncodingFormat == MT_DRV_DISP_FMT_NTSC_J) ||
			   (enEncodingFormat == MT_DRV_DISP_FMT_1440x480i_60) ||
			   (enEncodingFormat == MT_DRV_DISP_FMT_NTSC_443)) {
		enEncodingFormat = MT_DRV_DISP_FMT_NTSC;
	} else if (enEncodingFormat == MT_DRV_DISP_FMT_1080P_24_FP) {
		enEncodingFormat = MT_DRV_DISP_FMT_1080P_24;
	} else if (enEncodingFormat == MT_DRV_DISP_FMT_720P_60_FP) {
		enEncodingFormat = MT_DRV_DISP_FMT_720P_60;
	} else if (enEncodingFormat == MT_DRV_DISP_FMT_720P_50_FP) {
		enEncodingFormat = MT_DRV_DISP_FMT_720P_50;
	}

	pstVidAttr->b3DEnable = MT_TRUE;
	if (DISP_STEREO_FPK == enStereo) {
		pstVidAttr->u83DParam = MT_UNF_EDID_3D_FRAME_PACKETING;
	} else if (DISP_STEREO_SBS_HALF == enStereo) {
		pstVidAttr->u83DParam = MT_UNF_EDID_3D_SIDE_BY_SIDE_HALF;
	} else if (DISP_STEREO_TAB == enStereo) {
		pstVidAttr->u83DParam = MT_UNF_EDID_3D_TOP_AND_BOTTOM;
	} else {
		pstVidAttr->b3DEnable = MT_FALSE;
		pstVidAttr->u83DParam = MT_UNF_EDID_3D_BUTT;
	}

	COM_INFO("FMT:%d,3DFlag:%d, 3dParm:%d\n", enEncodingFormat, pstVidAttr->b3DEnable, pstVidAttr->u83DParam);

	if (DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
		if (MT_TRUE == pSinkCap->bSupportFormat[hdmi_Disp2EncFmt(enEncodingFormat)]) {
			COM_INFO("From EDID, sink can receive this format!!!\n");
		} else {
			COM_ERR("Warring:From EDID, Sink CAN NOT receive this format*******\n");
			//return MT_FAILURE;
		}
	} else {
		COM_INFO("Invalid capability, we can't know support fmt \n");
	}

	DRV_HDMI_Vcfg_Status(5);
	//pstAppAttr->bEnableHdmi = bHDMIMode;        //HDMI or DVI
	pstVidAttr->enVideoFmt = enEncodingFormat;
	{
		mt_s32 rrr;
		DISP_HDMI_PARAM_T pin = {0};
		DISP_HDMI_PARAM_T pout = {0};
		hdmi_video_config_t *vcfg = NULL;
		vcfg = DRV_Get_Av_Params(MT_UNF_HDMI_ID_0);
		pin.disp2hdmi_vcfg = *vcfg;
		pin.drv_disp_fmt = enFmt;
		pin.drv_disp_stereo = enStereo;
		rrr = DRV_HDMI_Vcfg_Check(enHdmi, &pin, &pout);
		if (rrr) {
			COM_INFO("Disp input param err!\n");
		}
	}

#if defined (HDCP_SUPPORT)
	//Disable HDCP
	if (pstAppAttr->bHDCPEnable == MT_TRUE) {
		//DRV_Set_ThreadStop(MT_FALSE);
		SI_wait_hdcp_off();
	}
#endif
	//set_tvsys_switch(enEncodingFormat);

	if (MT_TRUE != SI_HPD_Status()) {
		//when undetect hotplug,don't need to set fmt,we process it in hotplug callbackfunc
		COM_INFO("hot plug not detected!\n");

		//setting format in unplug means, video did not configed.so we need config video,in next hotplug callbackfunc
		DRV_Set_ForceUpdateFlag(enHdmi, MT_TRUE);
		DRV_HDMI_Set_Update_Tvsys(0);
		//DRV_HDMI_HdcpMute(enHdmi, MT_FALSE);
		MT_ANALOG_UP_ATTR(MT_ANA_INDEX_HDMITX_R2, struct mt_analog_hdmitx_r2_attr, d_reg, 0);
		return MT_SUCCESS;
	}

	DRV_Set_ThreadStop(MT_TRUE);
	//power up,and need 1ms for clk stable
	if (MT_TRUE == SI_RSEN_Status()) {
		//SI_TX_PHY_PowerDown(MT_FALSE);
		SiiLibTimeMilliDelay(1);
	}

	COM_INFO("Video stable0:%d\n", (uint32_t)siiIsTClockStable());
	while ( !siiIsTClockStable() && (tout > 0)) {
		SiiLibTimeMilliDelay(5);
		tout -= 5;
	}
	COM_INFO("Video stable1:%d\n", (uint32_t)siiIsTClockStable());
	DRV_HDMI_SetVOAttr(enHdmi, pstVidAttr, MT_TRUE);
	DRV_HDMI_SetAOAttr(enHdmi, pstAudAttr, MT_TRUE);

	COM_INFO("attr.bEnableHdmi:0x%x\n", pstAppAttr->bEnableHdmi);

	hdmi_AdjustAVIInfoFrame(enHdmi);
	hdmi_AdjustVSDBInfoFrame(enHdmi);
	hdmi_AdjustAUDInfoFrame(enHdmi);
	DRV_Set_ThreadStop(MT_FALSE);

	SiiLibTimeMilliDelay(50);
	{
		uint8_t dc;
		uint8_t DcChange = 0;
		SI_GetHdmiHalDc(&dc);
		if ((dc == 0) != ( pstAppAttr->enDeepColorMode == MT_UNF_HDMI_DEEP_COLOR_24BIT ) ) {
			DcChange = 1;
		}
		if ( DcChange ) {
			DRV_HDMI_Phy_Rst(1);
			DRV_HDMI_Phy_Rst(0);
		}
	}
	SI_EnableHdmiDevice();
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_HDMITX_R2, struct mt_analog_hdmitx_r2_attr, d_reg, 0);

	GetFormatDelay(enHdmi, &delayTime);
	if ( delayTime ) {
		SiiLibTimeMilliDelay(delayTime);
	}

	SiiLibTimeMilliDelay(150);
	#if defined (HDCP_SUPPORT)
	if (pstAppAttr->bHDCPEnable == MT_TRUE) {
		if ( DRV_Get_IsChnStart(enHdmi) ) {//HPD on, on in start
			SI_hdcp_en(MT_TRUE, 0);
		}
	} else {
		DRV_HDMI_HdcpMute(enHdmi, MT_FALSE);
		SI_hdcp_en(MT_FALSE, 0);
		DRV_HDMI_SetAVMute(enHdmi, MT_FALSE);
		{
			SiiLibTimeMilli_t tdiff = 0;
			g_tvformat_time[1] = SiiLibTimeMilliGet();
			tdiff = g_tvformat_time[1] - g_tvformat_time[0];
			if ( g_tvformat_time[0] != 0 ) {
			  COM_FATAL("hoff tvsys switch, time: %d(ms)\n",tdiff);
			}
			g_tvformat_time[1] = 0;
			g_tvformat_time[0] = 0;
		}
	}

	#else
	DRV_HDMI_SetAVMute(enHdmi, MT_FALSE);
	#endif
	//DRV_HDMI_HdcpMute(enHdmi, MT_FALSE);

	DRV_HDMI_Vcfg_Status(6);
	DRV_HDMI_Set_Update_Tvsys(0);
	#if defined (DEBUG_TIMER)
	end = SI_timer_count();
	SI_timer_stop();

	MT_PRINT("SetFormat Cost %dms \n", end - start);
	#endif

	COM_INFO("Leave DRV_HDMI_SetFormat\n");

	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_PreFormat(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enEncodingFormat)
{
	#if defined (DEBUG_TIMER)
	mt_s32 start = 0, end = 0;

	SI_timer_start();
	start = SI_timer_count();
	#endif

	HDMI_APP_ATTR_S     *pstAppAttr = DRV_Get_AppAttr(enHdmi);
	HDMI_APP_ATTRMT_S     *pstAppAttrMt = DRV_Get_AppAttrMt(enHdmi);
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	COM_INFO("DRV_HDMI_PreFormat \n");
	g_tvformat_time[0] = SiiLibTimeMilliGet();
	DRV_HDMI_Vcfg_Status(1);
	pstAppAttrMt->fmt = enEncodingFormat;
	DRV_HDMI_HdcpMute(enHdmi, MT_TRUE);
	DRV_HDMI_SetAVMute(enHdmi, MT_TRUE);
	//DRV_Set_ThreadStop(MT_TRUE);

	#if defined (HDCP_SUPPORT)
	//Disable HDCP
	if (pstAppAttr->bHDCPEnable == MT_TRUE) {
		SI_hdcp_en(MT_FALSE, 0);
	}
	#endif

	//SI_DisableHdmiDevice();
	//SiiLibTimeMilliDelay(1);

	//SI_TX_PHY_PowerDown(MT_TRUE);

	COM_INFO("DRV_HDMI_PreFormat leave\n");

	#if defined (DEBUG_TIMER)
	end = SI_timer_count();
	SI_timer_stop();

	MT_PRINT("PreFormat Cost %dms \n", end - start);
	#endif

	return 0;
}

static void DRV_HDMI_Set3DStruct(HDMI_EDID_S *pEDID, SiiLibEdidPar_t *parseEdid, mt_u8 order, D3_STRUCT_T d3_stuct)
{
	mt_u8 i;
	mt_u8 vid;

	//EDID_INFO("%s_%d:%d,%d\n",__func__,__LINE__,(mt_u32)order,(mt_u32)d3_stuct);
	if (order < parseEdid->vdb.size) {
		i = order;
		vid = parseEdid->vdb.svd[i].p[0] & 0x7F;
		switch (vid) {
			case 1:
				break;
			case 2:
			case 3:
				pEDID->stEdidParsed.supported_3d_struc_480p_60Hz |= d3_stuct;
				break;
			case 4:
				pEDID->stEdidParsed.supported_3d_struc_720p_60Hz |= d3_stuct;
				break;
			case 5:
				pEDID->stEdidParsed.supported_3d_struc_1080i_60Hz |= d3_stuct;
				break;
			case 6:
			case 7:
				pEDID->stEdidParsed.supported_3d_struc_480i_60Hz |= d3_stuct;
				break;
			case 16:
				pEDID->stEdidParsed.supported_3d_struc_1080p_60Hz |= d3_stuct;
				break;
			case 17:
			case 18:
				pEDID->stEdidParsed.supported_3d_struc_576p_50Hz |= d3_stuct;
				break;
			case 19:
				pEDID->stEdidParsed.supported_3d_struc_720p_50Hz |= d3_stuct;
				break;
			case 20:
				pEDID->stEdidParsed.supported_3d_struc_1080i_50Hz |= d3_stuct;
				break;
			case 21:
			case 22:
				pEDID->stEdidParsed.supported_3d_struc_576i_50Hz |= d3_stuct;
				break;
			case 31:
				pEDID->stEdidParsed.supported_3d_struc_1080p_50Hz |= d3_stuct;
				break;
			case 32:
				pEDID->stEdidParsed.supported_3d_struc_1080p_24Hz |= d3_stuct;
				break;
			case 33:
				pEDID->stEdidParsed.supported_3d_struc_1080p_25Hz |= d3_stuct;
				break;
			case 34:
				pEDID->stEdidParsed.supported_3d_struc_1080p_30Hz |= d3_stuct;
				break;
			case 93:
			case 103:
				pEDID->stEdidParsed.supported_3d_struc_3840x2160p_24Hz |= d3_stuct;
				break;
			case 94:
			case 104:
				pEDID->stEdidParsed.supported_3d_struc_3840x2160p_25Hz |= d3_stuct;
				break;
			case 95:
			case 105:
				pEDID->stEdidParsed.supported_3d_struc_3840x2160p_30Hz |= d3_stuct;
				break;
			case 96:
			case 106:
				pEDID->stEdidParsed.supported_3d_struc_3840x2160p_50Hz |= d3_stuct;
				break;
			case 97:
			case 107:
				pEDID->stEdidParsed.supported_3d_struc_3840x2160p_60Hz |= d3_stuct;
				break;
			case 98:
				pEDID->stEdidParsed.supported_3d_struc_4096x2160p_24Hz |= d3_stuct;
				break;
			case 99:
				pEDID->stEdidParsed.supported_3d_struc_4096x2160p_25Hz |= d3_stuct;
				break;
			case 100:
				pEDID->stEdidParsed.supported_3d_struc_4096x2160p_30Hz |= d3_stuct;
				break;
			case 101:
				pEDID->stEdidParsed.supported_3d_struc_4096x2160p_50Hz |= d3_stuct;
				break;
			case 102:
				pEDID->stEdidParsed.supported_3d_struc_4096x2160p_60Hz |= d3_stuct;
				break;
			default:
				break;
		}
	}
}

#define	D3_SUPPORT_50HZ			(0x02)
#define	D3_SUPPORT_60HZ			(0x01)
static void DRV_HDMI_Get3DInfo(HDMI_EDID_S *pEDID, SiiLibEdidPar_t *parseEdid, mt_u8 Hz50_60, mt_u16 hver)
{
	mt_u8 i;
	mt_u16 d3_mask = 0xffff;
	D3_STRUCT_T d3_stuct = D3_INFO_IDLE;
	//mt_u16 hver = 0x14b;

	if ( 0x14b == hver) {
		if ( parseEdid->d3Info.supported_3d ) {
			pEDID->stEdidParsed.supported_3d_struc_1080p_24Hz |= D3_INFO_FRAME_PACKING | D3_INFO_TOP_AND_BOTTOM;
			if ( Hz50_60 & D3_SUPPORT_60HZ ) {
				pEDID->stEdidParsed.supported_3d_struc_720p_60Hz |= D3_INFO_FRAME_PACKING | D3_INFO_TOP_AND_BOTTOM;
				pEDID->stEdidParsed.supported_3d_struc_1080i_60Hz |= D3_INFO_SIDE_BY_SIDE_HALF;
			}
			if ( Hz50_60 & D3_SUPPORT_50HZ ) {
				pEDID->stEdidParsed.supported_3d_struc_720p_50Hz |= D3_INFO_FRAME_PACKING | D3_INFO_TOP_AND_BOTTOM;
				pEDID->stEdidParsed.supported_3d_struc_1080i_50Hz |= D3_INFO_SIDE_BY_SIDE_HALF;
			}
		}

		if ( 0x01 == parseEdid->d3Info.supported_3d_multi || 0x02 == parseEdid->d3Info.supported_3d_multi) {
			if ( parseEdid->d3Info.Struc_all_3d & 0x1 ) {
				d3_stuct |= D3_INFO_FRAME_PACKING;
			}
			if ( parseEdid->d3Info.Struc_all_3d & (0x1 << 6) ) {
				d3_stuct |= D3_INFO_TOP_AND_BOTTOM;
			}
			if ( parseEdid->d3Info.Struc_all_3d & (0x1 << 8) ) {
				d3_stuct |= D3_INFO_SIDE_BY_SIDE_HALF;
			}
		}

		if ( 0x01 == parseEdid->d3Info.supported_3d_multi ) {
			d3_mask = 0xffff;
		} else if ( 0x02 == parseEdid->d3Info.supported_3d_multi ) {
			d3_mask = parseEdid->d3Info.mask_3d;
		} else {
			d3_mask = 0;
		}

		for ( i = 0; i < 16; i++) {
			if ( d3_mask & (1 << i) ) {
				DRV_HDMI_Set3DStruct(pEDID, parseEdid, i, d3_stuct);
			}
		}
		for ( i = 0; i < parseEdid->d3Info.Edid3DVicOrderCnt; i++) {
			d3_stuct = 0;
			switch ( parseEdid->d3Info.vic_ord[i].struc_3d ) {
				case 0:
					d3_stuct = D3_INFO_FRAME_PACKING;
					break;
				case 6:
					d3_stuct = D3_INFO_TOP_AND_BOTTOM;
					break;
				case 8:
					d3_stuct = D3_INFO_SIDE_BY_SIDE_HALF;
					break;
				default:
					EDID_INFO("Not support 3D structure:%d\n", (mt_u32)parseEdid->d3Info.vic_ord[i].struc_3d);
					break;
			}
			DRV_HDMI_Set3DStruct(pEDID, parseEdid, parseEdid->d3Info.vic_ord[i].vic_order, d3_stuct);
		}
		#if 0
		EDID_INFO("%-36s :\n", "3D Info");

		EDID_INFO("%-36s : %s\n", "480i60", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_480i_60Hz));
		EDID_INFO("%-36s : %s\n", "480p60", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_480p_60Hz));
		EDID_INFO("%-36s : %s\n", "720p60", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_720p_60Hz));
		EDID_INFO("%-36s : %s\n", "1080i60", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_1080i_60Hz));
		EDID_INFO("%-36s : %s\n", "1080p60", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_1080p_60Hz));
		EDID_INFO("%-36s : %s\n", "576i50", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_576i_50Hz));
		EDID_INFO("%-36s : %s\n", "576p50", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_576p_50Hz));
		EDID_INFO("%-36s : %s\n", "720p50", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_720p_50Hz));
		EDID_INFO("%-36s : %s\n", "1080i50", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_1080i_50Hz));
		EDID_INFO("%-36s : %s\n", "1080p50", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_1080p_50Hz));
		EDID_INFO("%-36s : %s\n", "1080p24", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_1080p_24Hz));
		EDID_INFO("%-36s : %s\n", "1080p25", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_1080p_25Hz));
		EDID_INFO("%-36s : %s\n", "1080p30", GET_3D_STRUCT_STRING(pEDID->stEdidParsed.supported_3d_struc_1080p_30Hz));
		#endif
	} else if ( 0x14a == hver ) {
		if ( parseEdid->d3Info.supported_3d ) {
			pEDID->stEdidParsed.supported_3d_struc_1080p_24Hz |= D3_INFO_FRAME_PACKING;
			if ( Hz50_60 & D3_SUPPORT_60HZ ) {
				pEDID->stEdidParsed.supported_3d_struc_720p_60Hz |= D3_INFO_FRAME_PACKING;
			}
			if ( Hz50_60 & D3_SUPPORT_50HZ ) {
				pEDID->stEdidParsed.supported_3d_struc_720p_50Hz |= D3_INFO_FRAME_PACKING;
			}
		}
	} else {
		/* Need to be done */
		;
	}

}

mt_u32 DRV_HDMI_Force_GetEDID(HDMI_EDID_S *pEDID)
{
#define EDID_COMP_SYM4		(0)
	SiiLibEdidPar_t parseEdid = {0};
	mt_u8 i;
	mt_u8 vid, temp;
	mt_u32 yuv420cmdb = 0;
	mt_u32 yuv420support = 0;
	mt_u8 Hz50_60 = 0;

	EDID_INFO("Enter DRV_HDMI_Extern_GetEDID\n");
	HDMI_CHECK_ID(pEDID->enHdmi);
	HDMI_CheckChnOpen(pEDID->enHdmi);

	if ( DRV_Get_IsValidSinkCap((MT_UNF_HDMI_ID_E)0) == MT_FALSE) {
		EDID_ERR("Force Get EDID fail!\n");
		return MT_FAILURE;
	}

	memset(pEDID, 0, sizeof(HDMI_EDID_S));
	SiiDrvTxEdidParseGet(DRV_HDMI_Get_TxInst(), &parseEdid);

	pEDID->u8EdidValid = MT_TRUE;
	pEDID->stEdidParsed.is_hdmi = (parseEdid.ieee_id == 0x000C03) || (parseEdid.ieee_id == 0xC45DD8);
	pEDID->stEdidParsed.ycbcr444_supported = parseEdid.Yuv444;
	pEDID->stEdidParsed.ycbcr422_supported = parseEdid.Yuv422;

	yuv420cmdb = parseEdid.yuv420CMDB[0] | (parseEdid.yuv420CMDB[1] << 8) | \
				 (parseEdid.yuv420CMDB[2] << 16) | (parseEdid.yuv420CMDB[3] << 24);

	pEDID->stEdidParsed.ycbcr420_supported = (yuv420cmdb != 0) || (parseEdid.yuv420Vdb.size != 0);
	pEDID->stEdidParsed.maxclk = parseEdid.maxTmds;
	pEDID->stEdidParsed.maxlum = parseEdid.hdrInfo.MaxLum;
	pEDID->stEdidParsed.maxlum_vivid = parseEdid.hdr10p_vivid.MaxLum;
	pEDID->stEdidParsed.hdr_et = ((parseEdid.hdrInfo.Gamma_HDR!=0)<<1) | (parseEdid.hdrInfo.Gamma_SDR!=0);
	for (i = 0; i < parseEdid.vdb.size; i++) {
		vid = parseEdid.vdb.svd[i].p[0] & 0x7F;
		temp = parseEdid.vdb.svd[i].p[0] >> 7;
		if ( i < 32 ) {
			if ( parseEdid.maxTmds >= 297000000 ) {
				yuv420support = yuv420cmdb & ( 1 << i);
			} else {
				yuv420support = 0;
			}
		} else {
			yuv420support = 0;
		}
		switch (vid) {
			case 1:
				pEDID->stEdidParsed.supported_640x480p_60Hz = 1;
				Hz50_60 |= D3_SUPPORT_60HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_640x480p_60Hz = temp;
				if (pEDID->stEdidParsed.preferred_format == HDMI_VIDEO_UNDEFINED) {
					pEDID->stEdidParsed.preferred_format = HDMI_VIDEO_640X480P_60;
				}
				#endif
				break;
			case 4:
				pEDID->stEdidParsed.supported_720p_60Hz = 1;
				Hz50_60 |= D3_SUPPORT_60HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_720p_60Hz = temp;
				if (pEDID->stEdidParsed.preferred_format == HDMI_VIDEO_UNDEFINED) {
					pEDID->stEdidParsed.preferred_format = HDMI_VIDEO_720P_60;
				}
				#endif
				break;
			case 19:
				pEDID->stEdidParsed.supported_720p_50Hz = 1;
				Hz50_60 |= D3_SUPPORT_50HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_720p_50Hz = temp;
				if (pEDID->stEdidParsed.preferred_format == HDMI_VIDEO_UNDEFINED) {
					pEDID->stEdidParsed.preferred_format = HDMI_VIDEO_720P_50;
				}
				#endif
				break;
			case 16:
				pEDID->stEdidParsed.supported_1080p_60Hz = 1;
				Hz50_60 |= D3_SUPPORT_60HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_1080p_60Hz = temp;
				if (pEDID->stEdidParsed.preferred_format == HDMI_VIDEO_UNDEFINED) {
					pEDID->stEdidParsed.preferred_format = HDMI_VIDEO_1080P_60;
				}
				#endif
				break;
			case 31:
				pEDID->stEdidParsed.supported_1080p_50Hz = 1;
				Hz50_60 |= D3_SUPPORT_50HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_1080p_50Hz = temp;
				#endif
				break;
			case 5:
				pEDID->stEdidParsed.supported_1080i_60Hz = 1;
				Hz50_60 |= D3_SUPPORT_60HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_1080i_60Hz = temp;
				if (pEDID->stEdidParsed.preferred_format == HDMI_VIDEO_UNDEFINED) {
					pEDID->stEdidParsed.preferred_format = HDMI_VIDEO_1080I_60;
				}
				#endif
				break;
			case 20:
				pEDID->stEdidParsed.supported_1080i_50Hz = 1;
				Hz50_60 |= D3_SUPPORT_50HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_1080i_50Hz = temp;
				#endif
				break;
			case 2:
			case 3:
				pEDID->stEdidParsed.supported_720x480p_60Hz = 1;
				Hz50_60 |= D3_SUPPORT_60HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_720x480p_60Hz = temp;
				if (pEDID->stEdidParsed.preferred_format == HDMI_VIDEO_UNDEFINED) {
					pEDID->stEdidParsed.preferred_format = HDMI_VIDEO_720X480P_60;
				}
				#endif
				break;
			case 6:
			case 7:
				pEDID->stEdidParsed.supported_720x480i_60Hz = 1;
				Hz50_60 |= D3_SUPPORT_60HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_720x480i_60Hz = temp;
				if (pEDID->stEdidParsed.preferred_format == HDMI_VIDEO_UNDEFINED) {
					pEDID->stEdidParsed.preferred_format = HDMI_VIDEO_720X480I_60;
				}
				#endif
				break;
			case 17:
			case 18:
				pEDID->stEdidParsed.supported_576p_50Hz = 1;
				Hz50_60 |= D3_SUPPORT_50HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_576p_50Hz = temp;
				if (pEDID->stEdidParsed.preferred_format == HDMI_VIDEO_UNDEFINED) {
					pEDID->stEdidParsed.preferred_format = HDMI_VIDEO_576P_50;
				}
				#endif
				break;
			case 21:
			case 22:
				pEDID->stEdidParsed.supported_576i_50Hz = 1;
				Hz50_60 |= D3_SUPPORT_50HZ;
				#if EDID_COMP_SYM4
				pEDID->stEdidParsed.is_native_576i_50Hz = temp;
				if (pEDID->stEdidParsed.preferred_format == HDMI_VIDEO_UNDEFINED) {
					pEDID->stEdidParsed.preferred_format = HDMI_VIDEO_576I_50;
				}
				#endif
				break;
			case 93:
			case 103:
				pEDID->stEdidParsed.supported_3840x2160p_24Hz = 1;
				break;
			case 94:
			case 104:
				pEDID->stEdidParsed.supported_3840x2160p_25Hz = 1;
				break;
			case 95:
			case 105:
				pEDID->stEdidParsed.supported_3840x2160p_30Hz = 1;
				break;
			case 96:
			case 106:
				pEDID->stEdidParsed.supported_3840x2160p_50Hz = 1;
				Hz50_60 |= D3_SUPPORT_50HZ;
				if ( yuv420support ) {
					pEDID->stEdidParsed.supported_3840x2160p_50Hz |= 0x2;
				}
				break;
			case 97:
			case 107:
				pEDID->stEdidParsed.supported_3840x2160p_60Hz = 1;
				Hz50_60 |= D3_SUPPORT_60HZ;
				if ( yuv420support ) {
					pEDID->stEdidParsed.supported_3840x2160p_60Hz |= 0x2;
				}
				break;
			case 98:
				pEDID->stEdidParsed.supported_4096x2160p_24Hz = 1;
				break;
			case 99:
				pEDID->stEdidParsed.supported_4096x2160p_25Hz = 1;
				break;
			case 100:
				pEDID->stEdidParsed.supported_4096x2160p_30Hz = 1;
				break;
			case 101:
				pEDID->stEdidParsed.supported_4096x2160p_50Hz = 1;
				Hz50_60 |= D3_SUPPORT_50HZ;
				if ( yuv420support ) {
					pEDID->stEdidParsed.supported_4096x2160p_50Hz |= 0x2;
				}
				break;
			case 102:
				pEDID->stEdidParsed.supported_4096x2160p_60Hz = 1;
				Hz50_60 |= D3_SUPPORT_60HZ;
				if ( yuv420support ) {
					pEDID->stEdidParsed.supported_4096x2160p_60Hz |= 0x2;
				}
				break;
			default:
				break;
		}
	}
	{
		MT_UNF_EDID_BASE_INFO_S    *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
		for (i = 0; i < MT_UNF_ENC_FMT_BUTT; i++) {
			if ( pSinkCap->bSupportFormat[i] == MT_TRUE) {
				switch ( i ) {
					case MT_UNF_ENC_FMT_1080P_60:
						pEDID->stEdidParsed.supported_1080p_60Hz = 1;
						break;
					case MT_UNF_ENC_FMT_1080P_50:
						pEDID->stEdidParsed.supported_1080p_50Hz = 1;
						break;
					case MT_UNF_ENC_FMT_1080P_30:
						break;
					case MT_UNF_ENC_FMT_1080P_25:
						break;
					case MT_UNF_ENC_FMT_1080P_24:
						break;
					case MT_UNF_ENC_FMT_1080i_60:
						pEDID->stEdidParsed.supported_1080i_60Hz = 1;
						break;
					case MT_UNF_ENC_FMT_1080i_50:
						pEDID->stEdidParsed.supported_1080i_50Hz = 1;
						break;
					case MT_UNF_ENC_FMT_720P_60:
						pEDID->stEdidParsed.supported_720p_60Hz = 1;
						break;
					case MT_UNF_ENC_FMT_720P_50:
						pEDID->stEdidParsed.supported_720p_50Hz = 1;
						break;
					case MT_UNF_ENC_FMT_576P_50:
						pEDID->stEdidParsed.supported_576p_50Hz = 1;
						break;
					case MT_UNF_ENC_FMT_480P_60:
						pEDID->stEdidParsed.supported_720x480p_60Hz = 1;
						break;
					case MT_UNF_ENC_FMT_PAL:
						pEDID->stEdidParsed.supported_576i_50Hz = 1;
						break;
					case MT_UNF_ENC_FMT_NTSC:
						pEDID->stEdidParsed.supported_720x480i_60Hz = 1;
						break;
					case MT_UNF_ENC_FMT_3840X2160_24:
						pEDID->stEdidParsed.supported_3840x2160p_24Hz = 1;
						break;
					case MT_UNF_ENC_FMT_3840X2160_25:
						pEDID->stEdidParsed.supported_3840x2160p_25Hz = 1;
						break;
					case MT_UNF_ENC_FMT_3840X2160_30:
						pEDID->stEdidParsed.supported_3840x2160p_30Hz = 1;
						break;
					case MT_UNF_ENC_FMT_3840X2160_50:
						pEDID->stEdidParsed.supported_3840x2160p_50Hz |= 1;
						break;
					case MT_UNF_ENC_FMT_3840X2160_60:
						pEDID->stEdidParsed.supported_3840x2160p_60Hz |= 1;
						break;
					case MT_UNF_ENC_FMT_4096X2160_24:
						pEDID->stEdidParsed.supported_4096x2160p_24Hz = 1;
						break;
					case MT_UNF_ENC_FMT_4096X2160_25:
						pEDID->stEdidParsed.supported_4096x2160p_25Hz = 1;
						break;
					case MT_UNF_ENC_FMT_4096X2160_30:
						pEDID->stEdidParsed.supported_4096x2160p_30Hz = 1;
						break;
					case MT_UNF_ENC_FMT_4096X2160_50:
						pEDID->stEdidParsed.supported_4096x2160p_50Hz |= 1;
						break;
					case MT_UNF_ENC_FMT_4096X2160_60:
						pEDID->stEdidParsed.supported_4096x2160p_60Hz |= 1;
						break;
					default:
						break;
				}
			}
		}
	}
	DRV_HDMI_Get3DInfo(pEDID, &parseEdid, Hz50_60, 0x14b);
	if ( parseEdid.maxTmds >= 297000000 ) {
		for (i = 0; i < parseEdid.yuv420Vdb.size; i++) {
			vid = parseEdid.yuv420Vdb.svd[i].p[0] & 0x7F;
			switch ( vid ) {
				case 96:
				case 106:
					pEDID->stEdidParsed.supported_3840x2160p_50Hz = 0x2;
					break;
				case 97:
				case 107:
					pEDID->stEdidParsed.supported_3840x2160p_60Hz = 0x2;
					break;
				case 101:
					pEDID->stEdidParsed.supported_4096x2160p_50Hz = 0x2;
					break;
				case 102:
					pEDID->stEdidParsed.supported_4096x2160p_60Hz = 0x2;
					break;
				default:
					EDID_INFO("Support yuv420 only format, NOT storaged:%d\n", vid);
					break;
			}
		}
	}

	pEDID->stEdidParsed.rgb30bit = parseEdid.b444DC10;
	pEDID->stEdidParsed.rgb36bit = parseEdid.b444DC12;
	pEDID->stEdidParsed.rgb48bit = parseEdid.b444DC16;
	pEDID->stEdidParsed.dc_y444 = parseEdid.bY444;
	pEDID->stEdidParsed.y420_30bit = parseEdid.scdc.bDc30bit420;
	pEDID->stEdidParsed.y420_36bit = parseEdid.scdc.bDc36bit420;
	pEDID->stEdidParsed.y420_48bit = parseEdid.scdc.bDc48bit420;
	pEDID->stEdidParsed.supported_xvycc601 = parseEdid.colorimetry.xvYCC601;
	pEDID->stEdidParsed.supported_xvycc709 = parseEdid.colorimetry.xvYCC709;

	pEDID->stEdidParsed.supported_bt2020cycc = parseEdid.colorimetry.BT2020cYCC;
	pEDID->stEdidParsed.supported_bt2020ycc = parseEdid.colorimetry.BT2020YCC;
	pEDID->stEdidParsed.supported_bt2020rgb = parseEdid.colorimetry.BT2020RGB;
	pEDID->stEdidParsed.supported_hdr10 = parseEdid.hdrInfo.Smpte2084;
	pEDID->stEdidParsed.supported_hlg = parseEdid.hdrInfo.HLG;
	pEDID->stEdidParsed.supported_hdr10p_vsif = parseEdid.hdr10p_vsif;
	pEDID->stEdidParsed.supported_hdr10p_vivid = parseEdid.hdr10p_vivid.vivid;
	pEDID->stEdidParsed.hdr10p_emp = (mt_u8)(parseEdid.hdr10p_emp > 0);//parseEdid.hdr10p_emp_2 | parseEdid.hdr10p_emp_4;//NEED to be done
	pEDID->stEdidParsed.DdMat48K = parseEdid.DdVsadb.SinkCap; //DD MAT PCM 48K Only

	#if 1
	pEDID->stEdidParsed.supported_4k2k_30 = parseEdid.d3Info.supported_4k2k_30;
	pEDID->stEdidParsed.supported_4k2k_25 = parseEdid.d3Info.supported_4k2k_25;
	pEDID->stEdidParsed.supported_4k2k_24 = parseEdid.d3Info.supported_4k2k_24;
	pEDID->stEdidParsed.supported_4k2k_smpte_24 = parseEdid.d3Info.supported_4k2k_smpte_24;
	pEDID->stEdidParsed.audioformat_cnt = parseEdid.audInfo.audformat_cnt;

	//smemcpy(pEDID->stEdidParsed.tv_id_info, parseEdid.tv_id_info, 10);

	memcpy(pEDID->stEdidParsed.audioformat, parseEdid.audInfo.audformat, sizeof(pEDID->stEdidParsed.audioformat));
	memcpy(pEDID->stEdidParsed.audiochannel, parseEdid.audInfo.audchannel, sizeof(pEDID->stEdidParsed.audiochannel));
	memcpy(pEDID->stEdidParsed.audiofs, parseEdid.audInfo.audfs, sizeof(pEDID->stEdidParsed.audiofs));
	memcpy(pEDID->stEdidParsed.audiolength, parseEdid.audInfo.audlen, sizeof(pEDID->stEdidParsed.audiolength));

	pEDID->stEdidParsed.speakerformat = parseEdid.audInfo.speakerformat;
	pEDID->stEdidParsed.cec_phy_addr = ((parseEdid.cecAddr.sub[0] << 12) | (parseEdid.cecAddr.sub[1] << 8)
										| (parseEdid.cecAddr.sub[2] << 4) | parseEdid.cecAddr.sub[3]);
	#endif
	EDID_INFO("Leave DRV_HDMI_Extern_GetEDID\n");
	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_GetRawEdidInfo(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *rawEdidInfo)
{
	mt_s32 ret = MT_SUCCESS;
	SiiEdid_t edid;
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	SiiDrvTxEdidGet(DRV_HDMI_Get_TxInst(), &edid);
	memcpy(rawEdidInfo, edid.b, SII_EDID_MAX_LEN);
	return ret;
}

mt_u32 DRV_HDMI_GetPlayStatus(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *pu32Stutus)
{
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	*pu32Stutus = DRV_Get_IsChnStart(enHdmi);
	return 0;
}

mt_u32 DRV_HDMI_LoadKey(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_LOAD_KEY_S *pstLoadKey)
{
	mt_u32 u32Ret = MT_SUCCESS;
	u32Ret = (mt_u32)SiiModTxHdcpLoadKey((SiiInst_t)NULL, (uint8_t *)pstLoadKey->pu8InputEncryptedKey, (uint32_t)pstLoadKey->u32KeyLength);
	return MT_SUCCESS;
}

extern ulong g_ProcHandle ;
mt_s32 DRV_HDMI_GetProcID(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *pu32ProcID)
{
	mt_s32 u32Ret = MT_FAILURE;
	mt_u32 u32ProcIndex = 0;
	HDMI_PROC_EVENT_S *pEventList = DRV_Get_EventList(MT_UNF_HDMI_ID_0);
	COM_INFO("DRV_HDMI_GetProcID\n");

	//������
	//û����ͨ��ID
	//HDMI_CHECK_ID(enHdmi);

	if (pu32ProcID == NULL) {
		COM_WARN("Null ProcID pointer! \n");
		return u32Ret;
	}

	for (u32ProcIndex = 0; u32ProcIndex < MAX_PROCESS_NUM; u32ProcIndex++) {
		if (MT_TRUE != pEventList[u32ProcIndex].bUsed) {
			*pu32ProcID = u32ProcIndex;
			pEventList[u32ProcIndex].bUsed = MT_TRUE;
			pEventList[u32ProcIndex].u32ProcHandle =  g_ProcHandle;
			u32Ret = MT_SUCCESS;
			break;
		}
	}

	COM_INFO("Getted ProcID %d\n", *pu32ProcID);
	return u32Ret;
}

mt_s32 DRV_HDMI_ReleaseProcID(MT_UNF_HDMI_ID_E enHdmi, mt_u32 u32ProcID)
{
	mt_s32 u32Ret = MT_FAILURE;
	HDMI_PROC_EVENT_S *pEventList = DRV_Get_EventList(MT_UNF_HDMI_ID_0);

	COM_INFO("DRV_HDMI_ReleaseProcID %d\n", u32ProcID);

	//������
	//HDMI_CHECK_ID(enHdmi);

	if (u32ProcID >= MAX_PROCESS_NUM) {
		return u32Ret;
	}

	memset(&pEventList[u32ProcID], 0, sizeof(HDMI_PROC_EVENT_S));

	u32Ret = MT_SUCCESS;

	return u32Ret;
}

mt_s32 DRV_HDMI_AudioChange(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr)
{
	mt_s32                            Ret = MT_SUCCESS;
	MT_UNF_HDMI_AUD_INFOFRAME_VER1_S *pstAUDInfoframe = DRV_Get_AudInfoFrm(enHdmi);
	HDMI_AUDIO_ATTR_S                *pstAudAttr = DRV_Get_AudioAttr(enHdmi);
	HDMI_APP_ATTR_S     *pstAppAttr = DRV_Get_AppAttr(enHdmi);

	COM_INFO("DRV_HDMI_AudioChange : enSoundIntf:%d,enSampleRate:%d,u32Channels:%d \n", pstHDMIAOAttr->enSoundIntf, pstHDMIAOAttr->enSampleRate, pstHDMIAOAttr->u32Channels);
	COM_INFO("enBitDepth:%d,u8DownSampleParm:%d,u8I2SCtlVbit:%d\n", pstHDMIAOAttr->enBitDepth, pstHDMIAOAttr->u8DownSampleParm, pstHDMIAOAttr->u8I2SCtlVbit);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	pstAppAttr->bEnableAudio = MT_TRUE;
	//we force set bit depth to 16bit
	pstHDMIAOAttr->enBitDepth = MT_UNF_BIT_DEPTH_16;

	if (pstHDMIAOAttr->enSoundIntf == pstAudAttr->enSoundIntf &&
			pstHDMIAOAttr->enSampleRate == pstAudAttr->enSampleRate &&
			pstHDMIAOAttr->u32Channels == pstAUDInfoframe->u32ChannelCount &&
			pstHDMIAOAttr->enBitDepth == pstAudAttr->enBitDepth &&
			pstHDMIAOAttr->enAudioCode == pstAUDInfoframe->enCodingType) {
		COM_INFO("%s.%d : The same Audio Attr \n", __FUNCTION__, __LINE__);
		//SI_SetHdmiAudio(pstAppAttr->bEnableAudio);
		return MT_SUCCESS;
	}

	if (HDMI_AUDIO_INTERFACE_I2S == pstHDMIAOAttr->enSoundIntf
			|| HDMI_AUDIO_INTERFACE_SPDIF == pstHDMIAOAttr->enSoundIntf
			|| HDMI_AUDIO_INTERFACE_HBR == pstHDMIAOAttr->enSoundIntf) {
		pstAudAttr->enSoundIntf = pstHDMIAOAttr->enSoundIntf;
	} else {
		COM_ERR("Error input Audio interface(%d)\n", pstHDMIAOAttr->enSoundIntf);
		//SI_SetHdmiAudio(pstAppAttr->bEnableAudio);
		return MT_FAILURE;
	}

	COM_INFO("\n\n\n audio Sample Rate : %d\n\n\n", pstHDMIAOAttr->enSampleRate);
	switch (pstHDMIAOAttr->enSampleRate) {
		case MT_UNF_SAMPLE_RATE_32K:
		case MT_UNF_SAMPLE_RATE_44K:
		case MT_UNF_SAMPLE_RATE_48K:
		case MT_UNF_SAMPLE_RATE_64K:
		case MT_UNF_SAMPLE_RATE_88K:
		case MT_UNF_SAMPLE_RATE_96K:
		case MT_UNF_SAMPLE_RATE_128K:
		case MT_UNF_SAMPLE_RATE_176K:
		case MT_UNF_SAMPLE_RATE_192K:
			pstAudAttr->enSampleRate = pstHDMIAOAttr->enSampleRate;
			break;
		default:
			COM_ERR("Error input Audio Frequency(%d)\n", pstHDMIAOAttr->enSampleRate);
			//SI_SetHdmiAudio(pstAppAttr->bEnableAudio);
			return MT_FAILURE;
	}

	#if 0 /*--old solution--*/
	/* Set Audio infoframe */
	/* New function to set Audio Infoframe */
	/* HDMI requires the CT, SS and SF fields to be set to 0 ("Refer to Stream Header")
	   as these items are carried in the audio stream.*/
	//InfoFrame.enInfoFrameType = MT_INFOFRAME_TYPE_AUDIO;
	pstAUDInfoframe->u32ChannelCount      = pstHDMIAOAttr->u32Channels;

	//Refer to Stream Header
	pstAUDInfoframe->enCodingType         = MT_UNF_HDMI_DEFAULT_SETTING;
	#endif

	if (pstHDMIAOAttr->enSoundIntf == HDMI_AUDIO_INTERFACE_SPDIF
			|| pstHDMIAOAttr->enSoundIntf == HDMI_AUDIO_INTERFACE_HBR) {
		COM_INFO("Audio channel refer from stream \n");
		pstAUDInfoframe->u32ChannelCount      = 0;// refer from stream head
	} else {
		COM_INFO("Audio channel %d \n", pstHDMIAOAttr->u32Channels);
		pstAUDInfoframe->u32ChannelCount      = pstHDMIAOAttr->u32Channels; //PCM maybe 2 or 8;
	}

	if (pstHDMIAOAttr->enAudioCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3
			|| pstHDMIAOAttr->enAudioCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS
			|| pstHDMIAOAttr->enAudioCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP
			|| pstHDMIAOAttr->enAudioCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS_HD
			|| pstHDMIAOAttr->enAudioCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_MAT) {
		COM_INFO("Aduio Code : %d \n", pstHDMIAOAttr->enAudioCode);
		pstAUDInfoframe->enCodingType     = pstHDMIAOAttr->enAudioCode;
	} else {
		COM_INFO("Aduio Code : %d \n", pstHDMIAOAttr->enAudioCode);
		pstAUDInfoframe->enCodingType     = MT_UNF_EDID_AUDIO_FORMAT_CODE_RESERVED;
	}

	//Refer to Stream Header
	pstAUDInfoframe->u32SampleSize        = MT_UNF_HDMI_DEFAULT_SETTING;
	//Refer to Stream Header
	pstAUDInfoframe->u32SamplingFrequency = MT_UNF_HDMI_DEFAULT_SETTING;

	switch (pstHDMIAOAttr->u32Channels) {  //HDMI channel map
		case 3:
			pstAUDInfoframe->u32ChannelAlloc = 0x01;
			break;
		case 6:
			pstAUDInfoframe->u32ChannelAlloc = 0x0b;
			break;
		case 8:
			pstAUDInfoframe->u32ChannelAlloc = 0x13;
			break;
		default:
			pstAUDInfoframe->u32ChannelAlloc = 0x00;
			break;
	}
	pstAUDInfoframe->u32LevelShift        = 0;
	pstAUDInfoframe->u32DownmixInhibit    = MT_FALSE;
	COM_INFO("***MT_UNF_HDMI_SetInfoFrame for AUDIO Infoframe\n");

	if ((pstHDMIAOAttr->u32Channels > 2)
			&& (HDMI_AUDIO_INTERFACE_I2S == pstHDMIAOAttr->enSoundIntf)) {
		pstAudAttr->bIsMultiChannel = MT_TRUE;
	} else {
		pstAudAttr->bIsMultiChannel = MT_FALSE;
	}

	pstAudAttr->u32Channels = pstHDMIAOAttr->u32Channels;
	pstAudAttr->enBitDepth = pstHDMIAOAttr->enBitDepth;//MT_UNF_BIT_DEPTH_16;

	Ret = DRV_HDMI_SetAOAttr(enHdmi, pstAudAttr, MT_TRUE);

	hdmi_AdjustAUDInfoFrame(enHdmi);

	if (Ret != MT_SUCCESS) {
		COM_ERR("Set HDMI Audio Attr failed\n");
	}

	return Ret;
}

mt_s32 DRV_HDMI_GetAOAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr)
{
	mt_s32  Ret = MT_SUCCESS;
	//HDMI_ATTR_S stHDMIAttr;
	HDMI_AUDIO_ATTR_S  *pstAudAttr = DRV_Get_AudioAttr(enHdmi);
	//HDMI_AUDIO_ATTR_S stAudAttr;
	//MT_UNF_HDMI_INFOFRAME_S     InfoFrame;
	MT_UNF_HDMI_AUD_INFOFRAME_VER1_S *pstAUDInfoframe = DRV_Get_AudInfoFrm(enHdmi);

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	memcpy(pstHDMIAOAttr, pstAudAttr, sizeof(HDMI_AUDIO_ATTR_S));
	pstHDMIAOAttr->u32Channels = pstAUDInfoframe->u32ChannelCount;

	return Ret;
}

//��Ҫ����edid��������
static mt_u32 hdmi_AdjustAVIInfoFrame(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_u32 Ret = MT_SUCCESS;
	//HDMI_ATTR_S stHDMIAttr;
	HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(enHdmi);
	MT_UNF_HDMI_COLORSPACE_E enColorimetry;
	MT_UNF_HDMI_ASPECT_RATIO_E enAspectRate;
	mt_u32 u32PixelRepetition;
	mt_u32 enRGBQuantization;
	MT_DRV_DISP_FMT_E enEncodingFormat;
	MT_UNF_HDMI_INFOFRAME_S           stInfoFrame;
	MT_UNF_HDMI_AVI_INFOFRAME_VER2_S  *pstVIDInfoframe;
	//MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
	HDMI_APP_ATTR_S     *pstAppAttr = &(DRV_Get_AppAttrMt(enHdmi)->stAppAttr);
	hdmi_video_config_t	*hdmi_av_params = NULL;

	COM_INFO("adjust AVI InfoFrame start \n");

	if (SI_IsHDMIResetting() || MT_FALSE == pstAppAttr->bEnableHdmi) {
		COM_INFO("no need cfg AVIInfoFrame : return\n");
		return MT_SUCCESS;
	}

	hdmi_av_params = DRV_Get_Av_Params(MT_UNF_HDMI_ID_0);

	/* New function to set AVI Infoframe */
	hdmi_GetInfoFrame(enHdmi, MT_INFOFRAME_TYPE_AVI, &stInfoFrame);

	pstVIDInfoframe = (MT_UNF_HDMI_AVI_INFOFRAME_VER2_S *) & (stInfoFrame.unInforUnit.stAVIInfoFrame);

	enEncodingFormat = pstVidAttr->enVideoFmt;
	COM_INFO("change DISP Timing to enEncodingFormat:%d\n", enEncodingFormat);

	enColorimetry      = HDMI_COLORIMETRY_ITU709;
	enAspectRate       = MT_UNF_HDMI_ASPECT_RATIO_16TO9;
	u32PixelRepetition = MT_FALSE;
	enRGBQuantization  = HDMI_RGB_QUANTIZATION_DEFAULT_RANGE;

	if (MT_DRV_DISP_FMT_1080P_60 == enEncodingFormat) {
		COM_INFO("Set 1920X1080P_60000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_1080P_60);
	} else if (MT_DRV_DISP_FMT_1080P_50 == enEncodingFormat) {
		COM_INFO("Set 1920X1080P_50000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_1080P_50);
	} else if (MT_DRV_DISP_FMT_1080P_30 == enEncodingFormat) {
		COM_INFO("Set 1920X1080P_30000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_1080P_30);
	} else if (MT_DRV_DISP_FMT_1080P_25 == enEncodingFormat) {
		COM_INFO("Set 1920X1080P_25000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_1080P_25);
	} else if (MT_DRV_DISP_FMT_1080P_24 == enEncodingFormat) {
		COM_INFO("Set 1920X1080P_24000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_1080P_24);
	} else if (MT_DRV_DISP_FMT_1080i_60 == enEncodingFormat) {
		COM_INFO("Set 1920X1080i_60000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_1080i_60);
	} else if (MT_DRV_DISP_FMT_1080i_50 == enEncodingFormat) {
		COM_INFO("Set 1920X1080i_50000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_1080i_50);
	} else if (MT_DRV_DISP_FMT_720P_60 == enEncodingFormat) {
		COM_INFO("Set 1280X720P_60000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_720P_60);
	} else if (MT_DRV_DISP_FMT_720P_50 == enEncodingFormat) {
		COM_INFO("Set 1280X720P_50000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_720P_50);
	} else if (MT_DRV_DISP_FMT_576P_50 == enEncodingFormat) {
		COM_INFO("Set 720X576P_50000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_576P_50);
		enColorimetry = HDMI_COLORIMETRY_ITU601;
		if ( hdmi_av_params->output_v_cfg.shape != HDMI_SHAPE_16X9 ) {
			enAspectRate  = MT_UNF_HDMI_ASPECT_RATIO_4TO3;
		}
	} else if (MT_DRV_DISP_FMT_480P_60 == enEncodingFormat) {
		COM_INFO("Set 720X480P_60000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_480P_60);
		enColorimetry = HDMI_COLORIMETRY_ITU601;
		if ( hdmi_av_params->output_v_cfg.shape != HDMI_SHAPE_16X9 ) {
			enAspectRate  = MT_UNF_HDMI_ASPECT_RATIO_4TO3;
		}
	} else if ((MT_DRV_DISP_FMT_PAL == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_B == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_B1 == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_D == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_D1 == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_G == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_H == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_K == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_I == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_M == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_N == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_Nc == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_PAL_60 == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_SECAM_SIN == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_SECAM_COS == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_SECAM_L == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_SECAM_B == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_SECAM_G == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_SECAM_D == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_SECAM_K == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_SECAM_H == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_1440x576i_50  == enEncodingFormat)) {
		COM_INFO("Set PAL 576I_50000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_PAL);
		enColorimetry = HDMI_COLORIMETRY_ITU601;
		if ( hdmi_av_params->output_v_cfg.shape != HDMI_SHAPE_16X9 ) {
			enAspectRate  = MT_UNF_HDMI_ASPECT_RATIO_4TO3;
		}
		u32PixelRepetition = MT_TRUE;
	} else if ((MT_DRV_DISP_FMT_NTSC == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_NTSC_J == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_NTSC_443 == enEncodingFormat) ||
			   (MT_DRV_DISP_FMT_1440x480i_60  == enEncodingFormat)) {
		COM_INFO("Set NTS 480I_60000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_NTSC);
		enColorimetry = HDMI_COLORIMETRY_ITU601;
		if ( hdmi_av_params->output_v_cfg.shape != HDMI_SHAPE_16X9 ) {
			enAspectRate  = MT_UNF_HDMI_ASPECT_RATIO_4TO3;
		}
		u32PixelRepetition = MT_TRUE;
	} else if (MT_DRV_DISP_FMT_861D_640X480_60 == enEncodingFormat) {
		COM_INFO("Set 640X480P_60000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_861D_640X480_60);
		enColorimetry      = HDMI_COLORIMETRY_ITU601;
		if ( hdmi_av_params->output_v_cfg.shape != HDMI_SHAPE_16X9 ) {
			enAspectRate  = MT_UNF_HDMI_ASPECT_RATIO_4TO3;
		}
		u32PixelRepetition = MT_FALSE;

		enRGBQuantization  = HDMI_RGB_QUANTIZATION_FULL_RANGE;
		//enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
	}
	#if defined (DVI_SUPPORT)
	else if (DRV_Get_IsLCDFmt(enEncodingFormat)) {
		COM_INFO("DVI timing mode enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		//DEBUG_PRINTK("Force to DVI Mode\n");
		//bHDMIMode = MT_FALSE;

		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_861D_640X480_60);
		enColorimetry      = HDMI_COLORIMETRY_ITU601;
		enAspectRate       = MT_UNF_HDMI_ASPECT_RATIO_4TO3;
		u32PixelRepetition = MT_FALSE;

		enRGBQuantization  = HDMI_RGB_QUANTIZATION_FULL_RANGE;

		//enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
	}
	#endif
	else if (DRV_Get_Is4KFmt(enEncodingFormat)) {
		enColorimetry      = HDMI_COLORIMETRY_ITU709;
		enAspectRate       = MT_UNF_HDMI_ASPECT_RATIO_16TO9;
		u32PixelRepetition = MT_FALSE;
		enRGBQuantization  = HDMI_RGB_QUANTIZATION_DEFAULT_RANGE;
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(enEncodingFormat);
	} else {
		COM_INFO("Set NTS 480I_60000 enTimingMode:0x%x\n", pstVIDInfoframe->enTimingMode);
		pstVIDInfoframe->enTimingMode = hdmi_Disp2EncFmt(enEncodingFormat);
	}

	HDMI_AVPARAMS_LOCK();
	//xvYcc??
	//pstAVIInfoFrame->enTimingMode = stHDMIAttr.stAttr.enVideoFmt;
	pstVIDInfoframe->enOutputType = pstAppAttr->enVidOutMode;
	pstVIDInfoframe->bActive_Infor_Present = MT_TRUE;
	pstVIDInfoframe->enBarInfo = HDMI_BAR_INFO_NOT_VALID;
	pstVIDInfoframe->enColorimetry = enColorimetry;
	pstVIDInfoframe->enAspectRatio = enAspectRate;
	pstVIDInfoframe->enActiveAspectRatio = enAspectRate;
	pstVIDInfoframe->enPictureScaling = HDMI_PICTURE_NON_UNIFORM_SCALING;
	pstVIDInfoframe->enRGBQuantization = enRGBQuantization;
	pstVIDInfoframe->bIsITContent = MT_FALSE;
	pstVIDInfoframe->u32PixelRepetition = u32PixelRepetition;
	pstVIDInfoframe->enYCCQuantization = HDMI_YCC_QUANTIZATION_LIMITED_RANGE;
	switch (hdmi_av_params->output_v_cfg.std ) {
		case SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS:
			pstVIDInfoframe->enColorimetry = HDMI_COLORIMETRY_BT2020_NC;
			break;
		case SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS:
			pstVIDInfoframe->enColorimetry = HDMI_COLORIMETRY_BT2020_C;
			break;
		case SII_DRV_CONV_STD__BT_709:
			pstVIDInfoframe->enColorimetry = HDMI_COLORIMETRY_ITU709;
			break;
		case SII_DRV_CONV_STD__BT_601:
			pstVIDInfoframe->enColorimetry = HDMI_COLORIMETRY_ITU601;
			break;
		default:
			break;
	}
	HDMI_AVPARAMS_UNLOCK();

	#if 0 /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
	if (DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
		HDMI_PRIVATE_EDID_S *pPriSinkCap = DRV_Get_PriSinkCap(MT_UNF_HDMI_ID_0);
		if (pPriSinkCap->bUnderScanDev) {
			pstVIDInfoframe->enScanInfo =  HDMI_SCAN_INFO_UNDERSCANNED;
		} else {
			pstVIDInfoframe->enScanInfo =  HDMI_SCAN_INFO_OVERSCANNED;
		}
	} else {
		pstVIDInfoframe->enScanInfo = HDMI_SCAN_INFO_NO_DATA;
	}
	#else
	pstVIDInfoframe->enScanInfo = HDMI_SCAN_INFO_NO_DATA;
	#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

	pstVIDInfoframe->u32LineNEndofTopBar = 0;
	pstVIDInfoframe->u32LineNStartofBotBar = 0;
	pstVIDInfoframe->u32PixelNEndofLeftBar = 0;
	pstVIDInfoframe->u32PixelNStartofRightBar = 0;

	Ret |= DRV_HDMI_SetInfoFrame(enHdmi, &stInfoFrame);

	COM_INFO("adjust AVI InfoFrame over \n");

	return MT_SUCCESS;
}

static mt_u32 hdmi_AdjustVSDBInfoFrame(MT_UNF_HDMI_ID_E enHdmi)
{
	HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(enHdmi);
	SiiLibEdidPar_t parseEdid = {0};

	SiiDrvTxEdidParseGet(DRV_HDMI_Get_TxInst(), &parseEdid);
	if (pstVidAttr->b3DEnable) {
		SI_VSDB_Setting(VSDB_MODE_3D, (mt_u32)pstVidAttr->u83DParam);
	} else if (DRV_Get_Is4KFmt(pstVidAttr->enVideoFmt) && (parseEdid.scdc.bUHD_VIC == 0)) {
		SI_VSDB_Setting(VSDB_MODE_4K, (mt_u32)pstVidAttr->enVideoFmt);
	} else {
		SI_VSDB_Setting(VSDB_MODE_NONE, (mt_u32)0xff);
	}

	return MT_SUCCESS;
}

static mt_u32 hdmi_AdjustAUDInfoFrame(MT_UNF_HDMI_ID_E enHdmi)
{
	MT_UNF_HDMI_INFOFRAME_S           stInfoFrame;
	HDMI_APP_ATTR_S     *pstAppAttr = DRV_Get_AppAttr(enHdmi);

	if (SI_IsHDMIResetting() || MT_FALSE == pstAppAttr->bEnableHdmi || MT_FALSE == pstAppAttr->bEnableAudio) {
		COM_INFO("no need cfg AudInfoFrame : return\n");
		return MT_SUCCESS;
	}

	hdmi_GetInfoFrame(enHdmi, MT_INFOFRAME_TYPE_AUDIO, &stInfoFrame);

	DRV_HDMI_SetInfoFrame(enHdmi, &stInfoFrame);

	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_GetInitNum(MT_UNF_HDMI_ID_E enHdmi)
{
	return (g_HDMIKernelInitNum + g_HDMIUserInitNum);
}

mt_s32 DRV_HDMI_GetProcNum(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 u32ProcCount = 0;
	mt_s32 index;
	HDMI_PROC_EVENT_S *pEventList = DRV_Get_EventList(MT_UNF_HDMI_ID_0);

	for (index = 0; index < MAX_PROCESS_NUM; index++) {
		if (MT_TRUE == pEventList[index].bUsed) {
			u32ProcCount++;
		}
	}

	return u32ProcCount;
}

mt_s32 DRV_HDMI_SetAPPAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_APP_ATTR_S *pstHDMIAppAttr, MT_BOOL UpdateFlag)
{
	MT_BOOL AppForceUpdate = MT_FALSE;
	HDMI_APP_ATTR_S *pstAppAttr = DRV_Get_AppAttr(enHdmi);
	COM_INFO("Enter DRV_HDMI_SetAPPAttr\n");

	if (pstHDMIAppAttr->enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_YCBCR422) {
		COM_ERR("Not support MT_UNF_HDMI_VIDEO_MODE_YCBCR422 return \n");
		//return MT_ERR_HDMI_INVALID_PARA;
	}

	if (UpdateFlag == MT_TRUE) {
		AppForceUpdate = MT_TRUE;
	} else if (MT_SUCCESS != hdmi_AppAttrChanged(enHdmi, pstAppAttr, pstHDMIAppAttr, &AppForceUpdate)) {
		COM_INFO("Set APPAttr With No video update \n");
		return MT_SUCCESS;
	} else {
		COM_INFO("app attr has change \n");
	}

	COM_INFO("HDMI: %d , Aud: %d , Vid: %d \n", pstHDMIAppAttr->bEnableHdmi, pstHDMIAppAttr->bEnableAudio, pstHDMIAppAttr->bEnableVideo);

	if (pstHDMIAppAttr->bEnableHdmi == MT_FALSE) {
		pstHDMIAppAttr->bEnableAudio = MT_FALSE;
		pstHDMIAppAttr->bEnableAudInfoFrame = MT_FALSE;
		pstHDMIAppAttr->bEnableAviInfoFrame = MT_FALSE;
		pstHDMIAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
	}

	memcpy(pstAppAttr, pstHDMIAppAttr, sizeof(HDMI_APP_ATTR_S));

	/* Set HDMI Video Enable flag */
	COM_INFO("SET hdmi video status Flag:%d\n", pstAppAttr->bEnableVideo);

	COM_INFO("AppForceUpdate %d \n", AppForceUpdate);
	if (AppForceUpdate == MT_TRUE) {
		DRV_Set_ForceUpdateFlag(enHdmi, MT_TRUE);
	} else {
		DRV_Set_PartUpdateFlag(enHdmi, MT_TRUE);
	}
	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_SetAOAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr, MT_BOOL UpdateFlag)
{
	mt_u32 Ret = MT_SUCCESS;

	HDMI_AUDIO_ATTR_S *pstAudAttr = DRV_Get_AudioAttr(enHdmi);
	MT_BOOL AUpdate = MT_TRUE;
	SiiAudioFormat_t pAudioFormat;

	memset(&pAudioFormat, 0, sizeof(SiiAudioFormat_t));
	COM_INFO("Enter DRV_HDMI_SetAOAttr\n");
	SI_timer_count();
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pstHDMIAOAttr);

	if (UpdateFlag != MT_TRUE) {
		if (MT_SUCCESS != hdmi_AudioAttrChanged(enHdmi, pstHDMIAOAttr, pstAudAttr, &AUpdate)) {
			COM_INFO("vo attr no change return \n");
			//SI_SetHdmiAudio(pstAppAttr->bEnableAudio);
			return MT_SUCCESS;
		}
	}

	COM_INFO("AUpdate %d  SetAOAttr\n", AUpdate);

	memcpy(pstAudAttr, pstHDMIAOAttr, sizeof(HDMI_AUDIO_ATTR_S));

	switch (pstAudAttr->enSampleRate) {
		case MT_UNF_SAMPLE_RATE_22K:
			pAudioFormat.audioFs = SII_AUDIO_FS__22_05KHZ;
			break;
		case MT_UNF_SAMPLE_RATE_24K:
			pAudioFormat.audioFs = SII_AUDIO_FS__24KHZ;
			break;
		case MT_UNF_SAMPLE_RATE_32K:
			pAudioFormat.audioFs = SII_AUDIO_FS__32KHZ;
			break;
		case MT_UNF_SAMPLE_RATE_44K:
			pAudioFormat.audioFs = SII_AUDIO_FS__44_1KHZ;
			break;
		case MT_UNF_SAMPLE_RATE_48K:
			pAudioFormat.audioFs = SII_AUDIO_FS__48KHZ;
			break;
		case MT_UNF_SAMPLE_RATE_64K:
			pAudioFormat.audioFs = SII_AUDIO_FS__64KHZ;
			break;
		case MT_UNF_SAMPLE_RATE_88K:
			pAudioFormat.audioFs = SII_AUDIO_FS__88_2KHZ;
			break;
		case MT_UNF_SAMPLE_RATE_96K:
			pAudioFormat.audioFs = SII_AUDIO_FS__96KHZ;
			break;
		case MT_UNF_SAMPLE_RATE_128K:
			pAudioFormat.audioFs = SII_AUDIO_FS__128KHZ;
			break;
		case MT_UNF_SAMPLE_RATE_176K:
			pAudioFormat.audioFs = SII_AUDIO_FS__176_4KHZ;
			break;
		case MT_UNF_SAMPLE_RATE_192K:
			pAudioFormat.audioFs = SII_AUDIO_FS__192KHZ;
			break;
		default:
			pAudioFormat.audioFs = SII_AUDIO_FS__48KHZ;
			break;
	}
	pAudioFormat.layout1 = (uint8_t)pstAudAttr->u32Channels;
	pAudioFormat.dsd = (uint8_t)0;
	switch (pstAudAttr->enSoundIntf) {
		case HDMI_AUDIO_INTERFACE_I2S:
			pAudioFormat.i2s = (uint8_t)1;
			break;
		case HDMI_AUDIO_INTERFACE_SPDIF:
			pAudioFormat.spdif = (uint8_t)1;
			break;
		case HDMI_AUDIO_INTERFACE_HBR:
			pAudioFormat.hbrA = (uint8_t)1;
			pAudioFormat.i2s = (uint8_t)1;
			pAudioFormat.layout1 = AUDIO_FORMAT__8CH;
			break;
		default:
			pAudioFormat.i2s = (uint8_t)1;
			break;
	}
	pAudioFormat.downSample = (uint8_t)pstAudAttr->u8DownSampleParm;
	drv_aud_hdmi_acr_cfg(enHdmi);
	SiiDrvTxAudioFormatSet(DRV_HDMI_Get_TxInst(), &pAudioFormat);

	COM_INFO("Leave hdmi_SetAoAttr\n");

	return Ret;
}

mt_s32 DRV_HDMI_SetVOAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_VIDEO_ATTR_S *pstHDMIVOAttr, MT_BOOL UpdateFlag)
{
	mt_u32 Ret = MT_SUCCESS;
	MT_U8 bVideoMode;               /* Hdmi Video mode index define in vmtables.c */
	HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(enHdmi);
	HDMI_APP_ATTR_S     *pstAppAttrUser = DRV_Get_AppAttr(enHdmi);
	HDMI_APP_ATTR_S     *pstAppAttr = &(DRV_Get_AppAttrMt(enHdmi)->stAppAttr);
	hdmi_video_config_t	*hdmi_av_params = NULL;
	mt_u32 DcChange = 0;

	MT_BOOL VUpdate = MT_TRUE;

	hdmi_av_params = DRV_Get_Av_Params(MT_UNF_HDMI_ID_0);

	COM_INFO("Enter DRV_HDMI_SetVOAttr \n");
	SI_timer_count();
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pstHDMIVOAttr);

	if (pstAppAttr->bEnableVideo != MT_TRUE) {
		COM_ERR("bEnableVideo Must be set to MT_TRUE!\n");
		pstAppAttr->bEnableVideo = MT_TRUE;
		pstAppAttrUser->bEnableVideo = MT_TRUE;
	}

	if (UpdateFlag != MT_TRUE) {
		if (MT_SUCCESS != hdmi_VideoAttrChanged(enHdmi, pstHDMIVOAttr, pstVidAttr, &VUpdate)) {
			COM_INFO("vo attr no change return \n");
			return MT_SUCCESS;
		}
	}

	COM_INFO("VUpdate %d\n", VUpdate);
	memcpy(pstVidAttr, pstHDMIVOAttr, sizeof(HDMI_VIDEO_ATTR_S));

	/* Adjust Video Path Param */
	if ((VUpdate == MT_TRUE) && (pstAppAttr->bEnableVideo == MT_TRUE) && (!DRV_Get_IsMce2App())) {
		/* Write VIDEO MODE INDEX */
		/* Transfer_VideoTimingFromat_to_VModeIndex() is get g_s32VmodeOfUNFFormat[] */
		bVideoMode = Transfer_VideoTimingFromat_to_VModeTablesIndex(hdmi_Disp2EncFmt(pstHDMIVOAttr->enVideoFmt));
		if (0 == bVideoMode) {
			COM_INFO("The HDMI FMT (%d) is invalid.\n", pstHDMIVOAttr->enVideoFmt);
		} else {
			COM_INFO("The HDMI_VMode of MT_UNF_ENC_FMT(%d) is %d.\n", pstHDMIVOAttr->enVideoFmt, bVideoMode);
		}

		DRV_Set_VIDMode(MT_UNF_HDMI_VIDEO_MODE_YCBCR444); //always YCbCr444 12bit
		if (MT_UNF_HDMI_VIDEO_MODE_YCBCR444 == DRV_Get_VIDMode()) {
			if (MT_UNF_HDMI_VIDEO_MODE_RGB444 == pstAppAttr->enVidOutMode) {
				COM_INFO("HDMI Input YCBCR444, Output RGB444\n");
			} else if (MT_UNF_HDMI_VIDEO_MODE_YCBCR444 == pstAppAttr->enVidOutMode) {
				COM_INFO("HDMI Input YCBCR444, Output YCBCR444\n");
			} else if (MT_UNF_HDMI_VIDEO_MODE_YCBCR420 == pstAppAttr->enVidOutMode) {
				COM_INFO("HDMI Input YCBCR444, Output YCBCR420\n");
			} else if (MT_UNF_HDMI_VIDEO_MODE_YCBCR422 == pstAppAttr->enVidOutMode) {
				COM_INFO("HDMI Input YCBCR444, Output YCBCR422\n");
			} else {
				MT_PRINT("Input YCBCR444, Output %d\n",pstAppAttr->enVidOutMode);
				//Ret = MT_FAILURE;
			}
		}
		g_hdmi_sw_reset = 1;
		HDMI_AVPARAMS_LOCK();
		if ( DRV_HDMI_Sw_Status_Get() ) {
			uint8_t dc;
			SI_GetHdmiHalDc(&dc);
			if ((dc == 0) != ( pstAppAttr->enDeepColorMode == MT_UNF_HDMI_DEEP_COLOR_24BIT ) ) {
				DcChange = 1;
			}
			if ( DcChange ) {
				hdmi_soft_rst(1);
			}
		}
		{
			SiiDrvTxColorInfoCfg_t FmtVidIn = {0};
			SiiDrvClrSpc_t clrSpc = SII_DRV_CLRSPC__RGB_FULL;
			SiiDrvBitDepth_t bitDepth;
			SiiQuantLevel_t Quant;
			SiiDrvConvStd_t std = SII_DRV_CONV_STD__BT_709;
			{
				//set input color params
				switch (DRV_Get_VIDMode()) {
					case MT_UNF_HDMI_VIDEO_MODE_YCBCR444:
						FmtVidIn.inputClrConvStd = hdmi_av_params->input_v_cfg.std;
						FmtVidIn.inputClrSpc = hdmi_av_params->input_v_cfg.csc;
						FmtVidIn.inputVidDcDepth = hdmi_av_params->input_v_cfg.bitDepth;
						Quant = (SiiQuantLevel_t)hdmi_av_params->input_v_cfg.video_full_range;
						break;
					default:
						FmtVidIn.inputClrConvStd = hdmi_av_params->input_v_cfg.std;
						FmtVidIn.inputClrSpc = hdmi_av_params->input_v_cfg.csc;
						FmtVidIn.inputVidDcDepth = hdmi_av_params->input_v_cfg.bitDepth;
						Quant = (SiiQuantLevel_t)hdmi_av_params->input_v_cfg.video_full_range;
						break;
				}
				SiiDrvTxInputQuantSet(DRV_HDMI_Get_TxInst(), &Quant);
				SiiDrvTxColorInfoConfig(DRV_HDMI_Get_TxInst(), &FmtVidIn);
			}
			{
				//set output color params
				HDMI_EDID_S pEDID;
				memset(&pEDID, 0, sizeof(HDMI_EDID_S));
				if (DRV_HDMI_Force_GetEDID(&pEDID) == MT_SUCCESS) {
					;
				}
				#if 1
				if (DRV_HDMI_Sicsc2Mtcsc(hdmi_av_params->output_v_cfg.csc) != pstAppAttr->enVidOutMode) {
					HDMI20_DRV_HDMI_PRINTK("\n Csc deosnt match: Disp %d,Attr:%d,oStd=%d\n", (mt_u32)hdmi_av_params->output_v_cfg.csc, (mt_u32)pstAppAttr->enVidOutMode, (mt_u32)hdmi_av_params->output_v_cfg.std);
				}
				if (DRV_HDMI_Sidc2Mtdc(hdmi_av_params->output_v_cfg.bitDepth) != pstAppAttr->enDeepColorMode) {
					HDMI20_DRV_HDMI_PRINTK("\n Dc deosnt match: Disp %d,Attr:%d\n", (mt_u32)hdmi_av_params->output_v_cfg.bitDepth, (mt_u32)pstAppAttr->enDeepColorMode);
				}
				switch (pstAppAttr->enVidOutMode) {
					case MT_UNF_HDMI_VIDEO_MODE_RGB444:
						clrSpc = SII_DRV_CLRSPC__RGB_LIMITED;
						#if 0
						if ( hdmi_av_params->output_v_cfg.csc == SII_DRV_CLRSPC__RGB_LIMITED) {
							clrSpc = SII_DRV_CLRSPC__RGB_LIMITED;
						} else {
							clrSpc = SII_DRV_CLRSPC__RGB_FULL;
						}
						#endif
						break;
					case MT_UNF_HDMI_VIDEO_MODE_YCBCR422:
						switch ( hdmi_av_params->output_v_cfg.std ) {
							case SII_DRV_CONV_STD__BT_601:
								clrSpc = SII_DRV_CLRSPC__YC422_601;
								break;
							case SII_DRV_CONV_STD__BT_709:
								clrSpc = SII_DRV_CLRSPC__YC422_709;
								break;
							case SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS:
							case SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS:
								clrSpc = SII_DRV_CLRSPC__YC422_2020;
								break;
							default:
								clrSpc = SII_DRV_CLRSPC__YC422_601;
								break;
						}
						break;
					case MT_UNF_HDMI_VIDEO_MODE_YCBCR444:
						switch ( hdmi_av_params->output_v_cfg.std ) {
							case SII_DRV_CONV_STD__BT_601:
								clrSpc = SII_DRV_CLRSPC__YC444_601;
								break;
							case SII_DRV_CONV_STD__BT_709:
								clrSpc = SII_DRV_CLRSPC__YC444_709;
								break;
							case SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS:
							case SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS:
								clrSpc = SII_DRV_CLRSPC__YC444_2020;
								break;
							default:
								clrSpc = SII_DRV_CLRSPC__YC444_601;
								break;
						}
						break;
						#if DISP_SUPPORT_4K_SOLUTION
					case MT_UNF_HDMI_VIDEO_MODE_YCBCR420:
						switch ( hdmi_av_params->output_v_cfg.std ) {
							case SII_DRV_CONV_STD__BT_601:
								clrSpc = SII_DRV_CLRSPC__YC420_601;
								break;
							case SII_DRV_CONV_STD__BT_709:
								clrSpc = SII_DRV_CLRSPC__YC420_709;
								break;
							case SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS:
							case SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS:
								clrSpc = SII_DRV_CLRSPC__YC420_2020;
								break;
							default:
								clrSpc = SII_DRV_CLRSPC__YC420_709;
								break;
						}
						break;
						#endif
					default:
						if (pEDID.stEdidParsed.ycbcr444_supported) {
							switch ( hdmi_av_params->output_v_cfg.std ) {
								case SII_DRV_CONV_STD__BT_601:
									clrSpc = SII_DRV_CLRSPC__YC444_601;
									break;
								case SII_DRV_CONV_STD__BT_709:
									clrSpc = SII_DRV_CLRSPC__YC444_709;
									break;
								case SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS:
								case SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS:
									clrSpc = SII_DRV_CLRSPC__YC444_2020;
									break;
								default:
									clrSpc = SII_DRV_CLRSPC__YC444_601;
									break;
							}
						} else {
							if ( hdmi_av_params->output_v_cfg.csc == SII_DRV_CLRSPC__RGB_LIMITED) {
								clrSpc = SII_DRV_CLRSPC__RGB_LIMITED;
							} else {
								clrSpc = SII_DRV_CLRSPC__RGB_FULL;
							}
						}
						break;
				}
				#else
				clrSpc = hdmi_av_params->output_v_cfg.csc;
				#endif
				std = hdmi_av_params->output_v_cfg.std;
				SiiDrvTxOutputColorimetrySet(DRV_HDMI_Get_TxInst(), &std);
				SiiDrvTxOutputColorSpaceSet(DRV_HDMI_Get_TxInst(), &clrSpc);
			}
			{
				//set output bit depth
				#if 1
				switch (pstAppAttr->enDeepColorMode) {
					case MT_UNF_HDMI_DEEP_COLOR_24BIT:
						bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
						break;
					case MT_UNF_HDMI_DEEP_COLOR_30BIT:
						bitDepth = SII_DRV_BIT_DEPTH__10_BIT;
						break;
					case MT_UNF_HDMI_DEEP_COLOR_36BIT:
						bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
						break;
					default:
						bitDepth = SII_DRV_BIT_DEPTH__PASSTHOUGH;
						break;
				}
				#else
				bitDepth = hdmi_av_params->output_v_cfg.bitDepth;
				#endif
				SiiDrvTxOutputBitDepthSet(DRV_HDMI_Get_TxInst(), bitDepth);
			}
			HDMI20_DRV_HDMI_PRINTK("\n Color input: std=%d, cs=%d, depth=%d\noutput std=%d,cs=%d, depth=%d\n", (mt_u32)FmtVidIn.inputClrConvStd, (mt_u32)FmtVidIn.inputClrSpc, (mt_u32)FmtVidIn.inputVidDcDepth, \
								(mt_u32)std,(mt_u32)clrSpc, (mt_u32)bitDepth);
		}
		HDMI_AVPARAMS_UNLOCK();
	}

	if ( DRV_HDMI_Sw_Status_Get() && DcChange ) {
		hdmi_soft_rst(0);
		hdmi_rcfg_info(DRV_HDMI_Get_TxInst());
#if defined (CEC_SUPPORT)
		{
		SiiCecPowerstatus_t sts;
		uint16_t pa;
		MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
		sts = SiiCecGetPowerState();
		if ( pSinkCap->stCECAddr.bPhyAddrValid ) {
			pa = ((pSinkCap->stCECAddr.u8PhyAddrA & 0xf) << 12) | \
				((pSinkCap->stCECAddr.u8PhyAddrB & 0xf) << 8) | \
				((pSinkCap->stCECAddr.u8PhyAddrC & 0xf) << 4) | \
				((pSinkCap->stCECAddr.u8PhyAddrD & 0xf) << 0);
		} else {
			pa = 0x1000;
		}
		HDMI20_DRV_HDMI_PRINTK("\n%s_%d: CEC Power:%d,Pa=0x%x, sss:%d DC:%d [%d %2x%2x%2x%2x]\n",__func__,__LINE__,(int)sts,(int)pa,(int)DRV_HDMI_Sw_Status_Get(),(int)DcChange,\
			pSinkCap->stCECAddr.bPhyAddrValid,pSinkCap->stCECAddr.u8PhyAddrA,pSinkCap->stCECAddr.u8PhyAddrB,pSinkCap->stCECAddr.u8PhyAddrC,pSinkCap->stCECAddr.u8PhyAddrD);
		SI_CEC_Close();
		SiiCecUpdatePowerState(sts);
		SiiCecSetDevicePA(pa);
		SI_CEC_Open();
		}
#endif
	}
	g_hdmi_sw_reset = 0;
	DRV_HDMI_CrAdaptive(enHdmi);
	return Ret;
}

mt_s32 DRV_HDMI_GetStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_STATUS_S *pHdmiStatus)
{
	SiiDrvHdcpStatus_t HdcpStatus = {0};
	SiiDrvHdcpKsvList_t 	bksvList = {0};
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pHdmiStatus);

	pHdmiStatus->bConnected = SI_HPD_Status();
	pHdmiStatus->bSinkPowerOn = SI_RSEN_Status();
	pHdmiStatus->sink_cpower_status = SiiCecGetSinkPowerOnStatus();
	SiiDrvTxHdcpStateStatusGet(DRV_HDMI_Get_TxInst(), &HdcpStatus);
	switch ( HdcpStatus ) {
		case SII_DRV_HDCP_STATUS__SUCCESS_1X:
		case SII_DRV_HDCP_STATUS__SUCCESS_22:
			pHdmiStatus->bAuthed = MT_TRUE;
			break;
		default:
			pHdmiStatus->bAuthed = MT_FALSE;
			break;
	}
	SiiDrvTxHdcpKsvListGet(DRV_HDMI_Get_TxInst(), &bksvList);
	if ( bksvList.length >= 5 ) {
		memcpy(pHdmiStatus->u8Bksv, bksvList.pListStart, 5);
	}

	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_GetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pHdmiDelay)
{
	HDMI_CHECK_ID(enHdmi);
	//HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pHdmiDelay);

	pHdmiDelay->bForceFmtDelay = IsForceFmtDelay();
	pHdmiDelay->bForceMuteDelay = IsForceMuteDelay();
	pHdmiDelay->u32FmtDelay = GetGlobalFmtDelay();
	pHdmiDelay->u32MuteDelay = GetGlobalsMuteDelay();
	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_SetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pHdmiDelay)
{
	HDMI_CHECK_ID(enHdmi);
	//HDMI_CheckChnOpen(enHdmi);
	HDMI_CHECK_NULL_PTR(pHdmiDelay);

	SetForceDelayMode(pHdmiDelay->bForceFmtDelay, pHdmiDelay->bForceMuteDelay);
	SetGlobalFmtDelay(pHdmiDelay->u32FmtDelay);
	SetGlobalMuteDelay(pHdmiDelay->u32MuteDelay);

	return MT_SUCCESS;
}
MT_U8 g_avi_info[13] = {0};
MT_U8 g_audio_info[13] = {0};

mt_void DRV_O5_HDMI_PutBinInfoFrame(MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, mt_void* infor_ptr)
{
	if (enInfoFrameType == MT_INFOFRAME_TYPE_AVI) {
		memcpy(g_avi_info, infor_ptr, 13);
	} else if (enInfoFrameType == MT_INFOFRAME_TYPE_AUDIO) {
		memcpy(g_audio_info, infor_ptr, 5);
	}
	return ;
}

mt_void DRV_O5_HDMI_GetBinInfoFrame(MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, mt_void *infor_ptr)
{
	if (enInfoFrameType == MT_INFOFRAME_TYPE_AVI) {
		memcpy(infor_ptr, g_avi_info, 13);
	} else if (enInfoFrameType == MT_INFOFRAME_TYPE_AUDIO) {
		memcpy(infor_ptr, g_audio_info, 5);
	}
}

mt_s32 DRV_HDMI_Set_Notify(mt_u32 *param)
{
	hdmi_notify_info_t *info;
	hdmi_notify_info_t *hdl;
	mt_s32 ret;

	info = (hdmi_notify_info_t *)param;
	if ( info == NULL ) {
		ret = -1;
	} else {
		HDMI20_DRV_HDMI_PRINTK("id:%d,notify:%p,evt:0x%x\n", info->id_type, info->notify, info->events);
		hdl = DRV_Get_NotifyHdl(0);
		if ( (info->id_type) < DRV_HDMI_NOTIFY_MAX_NUMBER ) {
			memcpy(&(hdl[info->id_type]), info, sizeof(hdmi_notify_info_t));
			ret = 0;
		} else {
			ret = -2;
		}
	}
	return ret;
}

mt_u32 DRV_HDMI_Registers_Dump(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_u32 i;
	mt_u32 reg_num = 4 << 10;

	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	HDMI20_DRV_HDMI_PRINTK("\n");
	for (i = 0; i < reg_num; i++) {
		if (i == 0xf4 || i == 0xd2 || i == 0x642 || i == 0xf8f || i == 0xf90) {
			HDMI20_DRV_HDMI_PRINTK("0x%lx:   ----fifo reg, NOT dump \n", HdmiDrvRegBaseGet(NULL, HDMI_MOD_DIG) + i);
		} else {
			HDMI20_DRV_HDMI_PRINTK("0x%lx:   0x%x \n", HdmiDrvRegBaseGet(NULL, HDMI_MOD_DIG) + i, SiiDrvCraRdReg8_4dump((SiiInst_t)NULL, i));
		}
	}
	HDMI20_DRV_HDMI_PRINTK("dump hdmi20 registers success!!!");
	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_Register_Read(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u8 *data)
{
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	*data = (mt_u8)SiiDrvCraRdReg8((SiiInst_t)0, (SiiDrvCraAddr_t)addr);

	return MT_SUCCESS;
}

mt_u32 DRV_HDMI_Register_Write(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u32 data)
{
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	SiiDrvCraWrReg8((SiiInst_t)0, (SiiDrvCraAddr_t)addr, (uint8_t)data);

	return MT_SUCCESS;
}

mt_void DRV_HDMI_HDR_Cfg(mt_u32 *param, mt_u32 lock_avp)
{
	mt_u8				SinkHdr = 0;
	bool_t				SrcHdr = false;
	hdmi_output_video_cfg_t hdmi_vout_old = {0};
	hdmi_output_video_cfg_t hdmi_vout_cfg = { 0 };
	hdmi_video_config_t	*hdmi_av_params = NULL;

	hdmi_av_params = DRV_Get_Av_Params(MT_UNF_HDMI_ID_0);

	if ( DRV_HDMI_Get_TxInst() == 0 ) {
		return;
	}
	if ( lock_avp ) {
		HDMI_AVPARAMS_LOCK();
	}
	hdmi_vout_old = hdmi_av_params->output_v_cfg;
	*hdmi_av_params = *((hdmi_video_config_t *)param);
	hdmi_vout_cfg = hdmi_av_params->output_v_cfg;
	//if (hdmi_vout_old.hdr_onoff != hdmi_vout_cfg.hdr_onoff ||
	//		hdmi_vout_cfg.hdr_onoff ) {
	{
		SiiInfoFrame_t pInfoFrame = {0};
		uint8_t i = 0;
		uint8_t cksum = 0;
		uint8_t*	infp = NULL;
		HDMI_EDID_S pEDID;

		memset(&pEDID, 0, sizeof(HDMI_EDID_S));
		if (MT_SUCCESS == DRV_HDMI_Force_GetEDID(&pEDID)) {
			SinkHdr = (pEDID.stEdidParsed.supported_hdr10 << 0) | (pEDID.stEdidParsed.supported_hlg << 1) | \
					  (pEDID.stEdidParsed.supported_hdr10p_vsif << 2) | (pEDID.stEdidParsed.hdr10p_emp << 3);
			if (SinkHdr == 0) {
				HDMI20_DRV_HDMI_PRINTK("Sink doesnt support any HDR\n");
			}
		}
		pInfoFrame.b[0] = 0x87;
		pInfoFrame.b[1] = 0x01;
		pInfoFrame.b[2] = 0x1A;
		pInfoFrame.b[3] = 0;
		infp = (uint8_t *)(&(hdmi_vout_cfg.usrdf_info[0].pb_byte[0]));
		if ((((SinkHdr & 0x1) == 0) && ((SinkHdr & 0xC) == 0) && pInfoFrame.b[4] == 2) || (((SinkHdr & 0x2) == 0) && pInfoFrame.b[4] == 3)) {
			hdmi_vout_cfg.hdr_onoff = 0;
		}
		if ( hdmi_vout_cfg.hdr_onoff ) {
			memcpy(&(pInfoFrame.b[4]), infp, SII_INFOFRAME_MAX_LEN - 4);
		}
		for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
			cksum += pInfoFrame.b[i];
		}
		cksum = 256 - cksum;
		pInfoFrame.b[3] = cksum;
		pInfoFrame.ifId = SII_INFO_FRAME_ID__HDR;
		if ( SinkHdr ) {
			SrcHdr = true;
		} else {
			SrcHdr = false;
		}
		SiiDrvTxInfoframeSet(DRV_HDMI_Get_TxInst(), &pInfoFrame);
		SiiDrvTxInfoframeOnOffSet(DRV_HDMI_Get_TxInst(), SII_INFO_FRAME_ID__HDR, SrcHdr);
		HDMI20_DRV_HDMI_PRINTK("[%s_%d] hdr info: %d, 0x%x, %d-%d\n", __func__, __LINE__, \
							   hdmi_vout_cfg.hdr_onoff, ((mt_u32 *)(pInfoFrame.b))[0], (mt_u32)SinkHdr, (mt_u32)SrcHdr);
	}
	if ( lock_avp ) {
		HDMI_AVPARAMS_UNLOCK();
	}
}

mt_void DRV_HDMI_Set_AspectRatio(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_ASPECT_RATIO_E ar)
{
	mt_u32 Ret;
	MT_UNF_HDMI_INFOFRAME_S 		  stInfoFrame;
	MT_UNF_HDMI_AVI_INFOFRAME_VER2_S  *pstVIDInfoframe;
	HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(enHdmi);
	mt_u32 issd = 0;

	Ret = hdmi_GetInfoFrame(enHdmi, MT_INFOFRAME_TYPE_AVI, &stInfoFrame);
	pstVIDInfoframe = (MT_UNF_HDMI_AVI_INFOFRAME_VER2_S *) & (stInfoFrame.unInforUnit.stAVIInfoFrame);
	switch (pstVidAttr->enVideoFmt) {
		case MT_DRV_DISP_FMT_576P_50:
		case MT_DRV_DISP_FMT_480P_60:
		case MT_DRV_DISP_FMT_PAL:
		case MT_DRV_DISP_FMT_NTSC:
			issd = 1;
			break;
		default:
			issd = 0;
			break;
	}

	if ( issd ) {
		pstVIDInfoframe->enAspectRatio = ar;
		Ret = DRV_HDMI_SetInfoFrame(enHdmi, &stInfoFrame);
		SiiDrvTxInfoframeOnOffSet(DRV_HDMI_Get_TxInst(), SII_INFO_FRAME_ID__AVI, true);
	}
}

mt_void DRV_HDMI_Correct_Colorimetry(hdmi_video_config_t	*vcfg)
{
	MT_UNF_HDMI_VIDEO_MODE_E csc = MT_UNF_HDMI_VIDEO_MODE_RGB444;
	HDMI_EDID_S pEDID;
	HDMI_APP_ATTR_S 	*pstAppAttr = &(DRV_Get_AppAttrMt(MT_UNF_HDMI_ID_0)->stAppAttr);

	vcfg->output_v_cfg.std = vcfg->input_v_cfg.std;
	if (DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
		memset(&pEDID, 0, sizeof(HDMI_EDID_S));
		if(DRV_HDMI_Force_GetEDID(&pEDID) != MT_SUCCESS) {
			return;
		}
		csc = pstAppAttr->enVidOutMode; //DRV_HDMI_Sicsc2Mtcsc(vcfg->output_v_cfg.csc);
		HDMI20_DRV_HDMI_PRINTK("In std:%d,dc:%d\n",vcfg->output_v_cfg.std,pstAppAttr->enDeepColorMode);
		//check colorimetry
		switch (vcfg->output_v_cfg.std) {
			case SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS:
				if (( pstAppAttr->enDeepColorMode < MT_UNF_HDMI_DEEP_COLOR_30BIT || pstAppAttr->enDeepColorMode > MT_UNF_HDMI_DEEP_COLOR_48BIT ) && csc != MT_UNF_HDMI_VIDEO_MODE_YCBCR422) {
					vcfg->output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
				} else if (csc == MT_UNF_HDMI_VIDEO_MODE_RGB444) {
					if ( pEDID.stEdidParsed.supported_bt2020rgb) {
						vcfg->output_v_cfg.std = SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS;
					} else {
						vcfg->output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
					}
				} else if ( 0 == pEDID.stEdidParsed.supported_bt2020cycc) {
					if ( pEDID.stEdidParsed.supported_bt2020ycc ) {
						vcfg->output_v_cfg.std = SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS;
					} else {
						vcfg->output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
					}
				}
				break;
			case SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS:
				if (( pstAppAttr->enDeepColorMode < MT_UNF_HDMI_DEEP_COLOR_30BIT || pstAppAttr->enDeepColorMode > MT_UNF_HDMI_DEEP_COLOR_48BIT ) && csc != MT_UNF_HDMI_VIDEO_MODE_YCBCR422) {
					vcfg->output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
				} else if (csc == MT_UNF_HDMI_VIDEO_MODE_RGB444) {
					if ( 0 == pEDID.stEdidParsed.supported_bt2020rgb ) {
						vcfg->output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
					}
				} else if ( 0 == pEDID.stEdidParsed.supported_bt2020ycc ) {
					if ( pEDID.stEdidParsed.supported_bt2020cycc ) {
						vcfg->output_v_cfg.std = SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS;
					} else {
						vcfg->output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
					}
				}
				break;
			default:
				break;
		}
		HDMI20_DRV_HDMI_PRINTK("Out std:%d,dc:%d\n",vcfg->output_v_cfg.std,pstAppAttr->enDeepColorMode);
	}
}

mt_s32 DRV_HDMI_Set_Video(mt_u32 *param, MT_UNF_HDMI_ID_E enHdmi)
{
	hdmi_output_video_cfg_t hdmi_vout_old = {0};
	//hdmi_output_video_cfg_t hdmi_vout_cfg = { 0 };
	hdmi_video_config_t	*hdmi_av_params = NULL;
	bool_t		cfg_chg = false;
	hdmi_video_config_t hdmi_av_params_old = {0};
	HDMI_CHECK_ID(enHdmi);
	HDMI_CheckChnOpen(enHdmi);

	hdmi_av_params = DRV_Get_Av_Params(enHdmi);

	DRV_HDMI_Vcfg_Status(4);
	HDMI_AVPARAMS_LOCK();
	//remember to add mutex lock
	hdmi_av_params_old = *hdmi_av_params;
	hdmi_vout_old = hdmi_av_params->output_v_cfg;
	*hdmi_av_params = *((hdmi_video_config_t *)param);
	if ( hdmi_av_params->output_v_cfg.fmt == hdmi_vout_old.fmt ) {
		cfg_chg = memcmp((void *)hdmi_av_params, (void *)(&hdmi_av_params_old), sizeof(hdmi_video_config_t));
	}
	HDMI20_DRV_HDMI_PRINTK("[%s_%d] params new : [%d %d %d %d], [%d %d %d %d - %d %d %d]\n", __func__, __LINE__, \
						   hdmi_av_params->input_v_cfg.std, hdmi_av_params->input_v_cfg.csc, hdmi_av_params->input_v_cfg.bitDepth, hdmi_av_params->input_v_cfg.pixel_rpt,
						   hdmi_av_params->output_v_cfg.std, hdmi_av_params->output_v_cfg.csc, hdmi_av_params->output_v_cfg.bitDepth, hdmi_av_params->output_v_cfg.pixel_rpt,
						   hdmi_av_params->output_v_cfg.shape, hdmi_av_params->output_v_cfg.hdr_onoff, hdmi_av_params->output_v_cfg.fmt);
	HDMI20_DRV_HDMI_PRINTK("[%s_%d] params old : [%d %d %d %d], [%d %d %d %d - %d %d %d]\n", __func__, __LINE__, \
						   hdmi_av_params->input_v_cfg.std, hdmi_av_params->input_v_cfg.csc, hdmi_av_params->input_v_cfg.bitDepth, hdmi_av_params->input_v_cfg.pixel_rpt,
						   hdmi_vout_old.std, hdmi_vout_old.csc,hdmi_vout_old.bitDepth, hdmi_vout_old.pixel_rpt,
						   hdmi_vout_old.shape, hdmi_vout_old.hdr_onoff, hdmi_vout_old.fmt);
	//hdmi_vout_cfg = hdmi_av_params->output_v_cfg;
	DRV_HDMI_HDR_Cfg(param, 0);
	DRV_HDMI_Correct_Colorimetry(hdmi_av_params);

	//pstAppAttr->enDeepColorMode = DRV_HDMI_Sidc2Mtdc(hdmi_vout_cfg.bitDepth);
	//pstAppAttr->enVidOutMode = DRV_HDMI_Sicsc2Mtcsc(hdmi_av_params->output_v_cfg.csc);
	HDMI_AVPARAMS_UNLOCK();
	if ( cfg_chg ) {
		//DRV_HDMI_SetAttr(0, &stAttr);
		if ( hdmi_av_params->output_v_cfg.std != hdmi_vout_old.std ) {
			hdmi_AdjustAVIInfoFrame(enHdmi);
		}
	}
	if (hdmi_av_params->output_v_cfg.shape != hdmi_vout_old.shape) {
		DRV_HDMI_Set_AspectRatio(enHdmi, (MT_UNF_HDMI_ASPECT_RATIO_E)hdmi_av_params->output_v_cfg.shape);
	}
	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_Output_Set(mt_u32 param)
{
	HDMI_APP_ATTR_S 	*pstAppAttr = DRV_Get_AppAttr(MT_UNF_HDMI_ID_0);
    if ( param == 0) {
		DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
		SI_PoweDownHdmiDevice();
#if defined (HDCP_SUPPORT)
	//Disable HDCP
	if (pstAppAttr->bHDCPEnable == MT_TRUE) {
		SI_hdcp_en(MT_FALSE, 0);
	}
#endif
	} else {
		SI_EnableHdmiDevice();
#if defined (HDCP_SUPPORT)
		//Disable HDCP
		if (pstAppAttr->bHDCPEnable == MT_TRUE) {
			SI_hdcp_en(MT_TRUE, 0);
		}
#endif
	}
	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_UserPacketRptEnable(MT_UNF_HDMI_ID_E hdmi_id, mt_u8 packet_id, MT_BOOL enable)
{
	HDMI_CHECK_ID(hdmi_id);
	HDMI_CheckChnOpen(hdmi_id);

	SiiDrvTxInfoframeOnOffSet(DRV_HDMI_Get_TxInst(), SII_INFO_FRAME_ID__HDR, false);
	return MT_SUCCESS;
}

static mt_void DRV_HDMI_DC_Dec(MT_UNF_HDMI_ID_E hdmi_id, HDMI_CLK_CFG_S* cfg_info)
{
	HDMI_APP_ATTR_S     *pstAppAttr = &(DRV_Get_AppAttrMt(hdmi_id)->stAppAttr);
	HDMI_APP_ATTR_S *pstAppAttrUser = DRV_Get_AppAttr(hdmi_id);
	HDMI_APP_ATTR_S     pstAppAttrTmp = *pstAppAttrUser;
	mt_u32 edid_vld = 0;
	HDMI_EDID_S pEDID = {0};
	mt_u32 clk;

	edid_vld = (DRV_HDMI_Force_GetEDID(&pEDID) == MT_SUCCESS);
	if ( edid_vld ) {
		if ( pstAppAttrTmp.enDeepColorMode == MT_UNF_HDMI_DEEP_COLOR_BUTT ) {
			//Auto Deep Color
			switch ( pstAppAttrTmp.enVidOutMode ) {
				case MT_UNF_HDMI_VIDEO_MODE_RGB444:
					if ( pEDID.stEdidParsed.rgb30bit ) {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_30BIT;
					} else if ( pEDID.stEdidParsed.rgb36bit ) {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_36BIT;
					} else {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
					}
					break;
				case MT_UNF_HDMI_VIDEO_MODE_YCBCR444:
					if ( pEDID.stEdidParsed.rgb30bit && pEDID.stEdidParsed.dc_y444 ) {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_30BIT;
					} else if ( pEDID.stEdidParsed.rgb36bit && pEDID.stEdidParsed.dc_y444 ) {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_36BIT;
					} else {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
					}
					break;
				case MT_UNF_HDMI_VIDEO_MODE_YCBCR420:
					if ( pEDID.stEdidParsed.y420_30bit ) {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_30BIT;
					} else if ( pEDID.stEdidParsed.y420_36bit ) {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_36BIT;
					} else {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
					}
					break;
				case MT_UNF_HDMI_VIDEO_MODE_AUTO:
				case MT_UNF_HDMI_VIDEO_MODE_BUTT:
					if ( pEDID.stEdidParsed.rgb30bit) {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_30BIT;
					} else if ( pEDID.stEdidParsed.rgb36bit) {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_36BIT;
					} else {
						pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
					}
					break;
				default:
					pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
					break;
			}
		} else if ( pstAppAttrTmp.enDeepColorMode > MT_UNF_HDMI_DEEP_COLOR_24BIT ) {
			switch ( pstAppAttrTmp.enDeepColorMode ) {
				case MT_UNF_HDMI_DEEP_COLOR_30BIT:
					switch ( pstAppAttr->enVidOutMode ) {
						case MT_UNF_HDMI_VIDEO_MODE_RGB444:
							if ( pEDID.stEdidParsed.rgb30bit ) {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_30BIT;
							} else {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
							}
							break;
						case MT_UNF_HDMI_VIDEO_MODE_YCBCR444:
							if ( pEDID.stEdidParsed.rgb30bit && pEDID.stEdidParsed.dc_y444 ) {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_30BIT;
							} else {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
							}
							break;
						case MT_UNF_HDMI_VIDEO_MODE_YCBCR420:
							if ( pEDID.stEdidParsed.y420_30bit ) {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_30BIT;
							} else {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
							}
							break;
						case MT_UNF_HDMI_VIDEO_MODE_AUTO:
						case MT_UNF_HDMI_VIDEO_MODE_BUTT:
							if ( pEDID.stEdidParsed.rgb30bit) {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_30BIT;
							} else {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
							}
							break;
						default:
							pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
							break;
					}
					break;
				case MT_UNF_HDMI_DEEP_COLOR_36BIT:
					switch ( pstAppAttrTmp.enVidOutMode ) {
						case MT_UNF_HDMI_VIDEO_MODE_RGB444:
							if ( pEDID.stEdidParsed.rgb36bit ) {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_36BIT;
							} else {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
							}
							break;
						case MT_UNF_HDMI_VIDEO_MODE_YCBCR444:
							if ( pEDID.stEdidParsed.rgb36bit && pEDID.stEdidParsed.dc_y444 ) {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_36BIT;
							} else {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
							}
							break;
						case MT_UNF_HDMI_VIDEO_MODE_YCBCR420:
							if ( pEDID.stEdidParsed.y420_36bit ) {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_36BIT;
							} else {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
							}
							break;
						case MT_UNF_HDMI_VIDEO_MODE_AUTO:
						case MT_UNF_HDMI_VIDEO_MODE_BUTT:
							if ( pEDID.stEdidParsed.rgb36bit) {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_36BIT;
							} else {
								pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
							}
							break;
						default:
							pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
							break;
					}
					break;
				default:
					break;
			}
		}else {
			pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
		}
	}else {
		pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
	}

	clk = cfg_info->clk;
	if ( cfg_info->clk <= 297000000 && pstAppAttrTmp.enVidOutMode != MT_UNF_HDMI_VIDEO_MODE_YCBCR422) {
		switch ( pstAppAttr->enDeepColorMode ) {
			case MT_UNF_HDMI_DEEP_COLOR_30BIT:
				clk = (cfg_info->clk*5)>>2;
				break;
			case MT_UNF_HDMI_DEEP_COLOR_36BIT:
				clk = (cfg_info->clk*3)>>1;
				break;
			default:
				break;
		}
	}

	if ( (clk > pEDID.stEdidParsed.maxclk || clk > 297000000) && pEDID.stEdidParsed.maxclk > 0 && pEDID.stEdidParsed.maxclk <= 297000000) {
		HDMI20_DRV_HDMI_PRINTK("[%s_%d] Info clk%d > Maxclk%d\n",__func__,__LINE__,cfg_info->clk,pEDID.stEdidParsed.maxclk);
		if ( pstAppAttr->enDeepColorMode > MT_UNF_HDMI_DEEP_COLOR_24BIT ) { //M41h sometimes sends error Maxclk
			pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
		}
	}
	HDMI20_DRV_HDMI_PRINTK("[%s_%d] dc:%d\n",__func__,__LINE__,pstAppAttr->enDeepColorMode);
}

static mt_void DRV_HDMI_CS_Dec(MT_UNF_HDMI_ID_E hdmi_id, HDMI_CLK_CFG_S* cfg_info)
{
	HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(hdmi_id);
	HDMI_APP_ATTR_S     *pstAppAttr = &(DRV_Get_AppAttrMt(hdmi_id)->stAppAttr);
	HDMI_APP_ATTR_S *pstAppAttrUser = DRV_Get_AppAttr(hdmi_id);
	HDMI_APP_ATTR_S     pstAppAttrTmp = *pstAppAttrUser;
	mt_u32 edid_vld = 0;
	HDMI_EDID_S pEDID = {0};

	edid_vld = (DRV_HDMI_Force_GetEDID(&pEDID) == MT_SUCCESS);
	HDMI20_DRV_HDMI_PRINTK("[%s_%d] [cs:%d %d, dc:%d][%d %d %d]\n",__func__,__LINE__,pstAppAttrTmp.enVidOutMode,pstAppAttr->enVidOutMode,pstAppAttr->enDeepColorMode,cfg_info->clk,cfg_info->clk_os,pEDID.stEdidParsed.maxclk);
	if ( edid_vld ) {
		if (pstAppAttrTmp.enVidOutMode >= MT_UNF_HDMI_VIDEO_MODE_BUTT) {
			if (pEDID.stEdidParsed.ycbcr444_supported) {
				pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
			} else {
				pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
			}
		} else if ( pstAppAttrTmp.enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_YCBCR444 ) {
			pstAppAttr->enVidOutMode = pstAppAttrTmp.enVidOutMode;
			if (!pEDID.stEdidParsed.ycbcr444_supported) {
				pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
			}
		} else if ( pstAppAttrTmp.enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_YCBCR422 ) {
			pstAppAttr->enVidOutMode = pstAppAttrTmp.enVidOutMode;
			if (!pEDID.stEdidParsed.ycbcr422_supported && pEDID.stEdidParsed.ycbcr444_supported) {
				pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
			} else if (!pEDID.stEdidParsed.ycbcr422_supported && !pEDID.stEdidParsed.ycbcr444_supported) {
				pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
			}
		} else if ( pstAppAttrTmp.enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_RGB444 ) {
			pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
		}
		HDMI20_DRV_HDMI_PRINTK("[%s_%d] [cs:%d %d, dc:%d][%d %d %d]\n",__func__,__LINE__,pstAppAttrTmp.enVidOutMode,pstAppAttr->enVidOutMode,pstAppAttr->enDeepColorMode,cfg_info->clk,cfg_info->clk_os,pEDID.stEdidParsed.maxclk);

		if ( pEDID.stEdidParsed.maxclk >= 297000000 && ( cfg_info->clk == 594000000 && cfg_info->clk_os == 594000000 && pEDID.stEdidParsed.ycbcr420_supported) && \
			(pstAppAttrTmp.enVidOutMode > MT_UNF_HDMI_VIDEO_MODE_YCBCR444 || pstAppAttr->enDeepColorMode > MT_UNF_HDMI_DEEP_COLOR_24BIT)) {
			cfg_info->clk_os = 594000000;
			cfg_info->clk = 297000000;
			pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR420;
		} else if ( cfg_info->clk != 594000000 || cfg_info->clk_os != 594000000) {
			if ((pstAppAttrTmp.enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_YCBCR420) || (pstAppAttr->enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_YCBCR420 && pstAppAttrTmp.enVidOutMode > MT_UNF_HDMI_VIDEO_MODE_YCBCR420)) {
				if ( pEDID.stEdidParsed.ycbcr444_supported ) {
					pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
				} else {
					pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
				}
			} else if (pstAppAttr->enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_YCBCR420 && pstAppAttrTmp.enVidOutMode < MT_UNF_HDMI_VIDEO_MODE_YCBCR420) {
				pstAppAttr->enVidOutMode = pstAppAttrTmp.enVidOutMode;
			}
		} else if ( pEDID.stEdidParsed.maxclk >= 297000000 && cfg_info->clk > pEDID.stEdidParsed.maxclk ) {
			if ( cfg_info->clk_os == 594000000 && cfg_info->clk == 594000000 && (pstVidAttr->b3DEnable == 0 || pstVidAttr->u83DParam != MT_UNF_EDID_3D_FRAME_PACKETING) ) {
				//Assume that the setting is from tv format list
				cfg_info->clk = 297000000;
				pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR420;
			}
		}
	}else {
		pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
	}
	HDMI20_DRV_HDMI_PRINTK("[%s_%d] cs:%d\n",__func__,__LINE__,pstAppAttr->enVidOutMode);
}

static mt_void DRV_HDMI_Clk_Dec(MT_UNF_HDMI_ID_E hdmi_id, HDMI_CLK_CFG_S* cfg_info)
{
	HDMI_APP_ATTR_S     *pstAppAttr = &(DRV_Get_AppAttrMt(hdmi_id)->stAppAttr);
	HDMI_APP_ATTR_S *pstAppAttrUser = DRV_Get_AppAttr(hdmi_id);
	HDMI_APP_ATTR_S     pstAppAttrTmp = *pstAppAttrUser;
	mt_u32 edid_vld = 0;
	HDMI_EDID_S pEDID = {0};

	edid_vld = (DRV_HDMI_Force_GetEDID(&pEDID) == MT_SUCCESS);
	if ( cfg_info->clk == 594000000 && cfg_info->clk_os == 594000000 && pstAppAttr->enVidOutMode == MT_UNF_HDMI_VIDEO_MODE_YCBCR420 ) {
		cfg_info->clk = 297000000;
	}
	if ( edid_vld ) {
		if ( cfg_info->clk <= 297000000 && pstAppAttrTmp.enVidOutMode != MT_UNF_HDMI_VIDEO_MODE_YCBCR422) {
			switch ( pstAppAttr->enDeepColorMode ) {
				case MT_UNF_HDMI_DEEP_COLOR_30BIT:
					cfg_info->clk = (cfg_info->clk*5)>>2;
					break;
				case MT_UNF_HDMI_DEEP_COLOR_36BIT:
					cfg_info->clk = (cfg_info->clk*3)>>1;
					break;
				default:
					break;
			}
		}

		if ( cfg_info->clk > pEDID.stEdidParsed.maxclk && pEDID.stEdidParsed.maxclk > 0 ) {
			HDMI20_DRV_HDMI_PRINTK("[%s_%d] Info clk%d > Maxclk%d\n",__func__,__LINE__,cfg_info->clk,pEDID.stEdidParsed.maxclk);
			if ( pstAppAttr->enDeepColorMode > MT_UNF_HDMI_DEEP_COLOR_24BIT && \
				(!(cfg_info->clk == 297000000 && pEDID.stEdidParsed.maxclk > 340000000))) { //M41h sometimes sends error Maxclk
				pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
				switch ( cfg_info->clk ) {
					case 445500000:
					case 371250000:
						cfg_info->clk = 297000000;
						break;
					case 222750000:
					case 185625000:
						cfg_info->clk = 148500000;
						break;
					case 92812500:
					case 111375000:
						cfg_info->clk = 74250000;
						break;
					case 33750000:
					case 40500000:
						cfg_info->clk = 27000000;
						break;
					default:
						//cfg_info->clk = 148500000;
						break;
				}
			}
		}
	}
	if ( cfg_info->clk >= 594000000 && cfg_info->clk_os >= 594000000 && \
		pstAppAttr->enDeepColorMode > MT_UNF_HDMI_DEEP_COLOR_24BIT ) {
		pstAppAttr->enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;
	}
}

static mt_s32 DRV_HDMI_Clk_Cfg_Calibration(MT_UNF_HDMI_ID_E hdmi_id, HDMI_CLK_CFG_S* cfg_info)
{
	HDMI_APP_ATTR_S     *pstAppAttr = &(DRV_Get_AppAttrMt(hdmi_id)->stAppAttr);
	HDMI_APP_ATTR_S *pstAppAttrUser = DRV_Get_AppAttr(hdmi_id);
	HDMI_APP_ATTR_S     pstAppAttrTmp = *pstAppAttrUser;
	HDMI_APP_ATTRMT_S     *pstAppAttrMt = DRV_Get_AppAttrMt(hdmi_id);
	HDMI_VIDEO_ATTR_S	*pstVidAttr = DRV_Get_VideoAttr(hdmi_id);

	HDMI_CHECK_ID(hdmi_id);
	HDMI_CheckChnOpen(hdmi_id);

	HDMI20_DRV_HDMI_PRINTK("%s_%d, hid%d,in clk/os{%d/%d} cs/dp{%d/%d,%d/%d} gate:%d\n",__func__,__LINE__,\
		hdmi_id,cfg_info->clk,cfg_info->clk_os,pstAppAttr->enVidOutMode,pstAppAttr->enDeepColorMode,\
		pstAppAttrTmp.enVidOutMode,pstAppAttrTmp.enDeepColorMode,cfg_info->vout_clk_gate_on);
	DRV_HDMI_DC_Dec(hdmi_id, cfg_info);
	DRV_HDMI_CS_Dec(hdmi_id, cfg_info);
	DRV_HDMI_Clk_Dec(hdmi_id, cfg_info);

	if ( pstAppAttrMt->fmt != pstVidAttr->enVideoFmt ) {
		cfg_info->vout_clk_gate_on = 1;
	} else {
		cfg_info->vout_clk_gate_on = 0;
	}
	pstAppAttrMt->tmds_clk = cfg_info->clk;
	pstAppAttrMt->os_clk = cfg_info->clk_os;
	HDMI20_DRV_HDMI_PRINTK("%s_%d, hid%d,out clk/os{%d/%d} cs/dp{%d/%d} gate:%d\n",__func__,__LINE__,hdmi_id,cfg_info->clk,cfg_info->clk_os,pstAppAttr->enVidOutMode,pstAppAttr->enDeepColorMode,cfg_info->vout_clk_gate_on);
	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_Clk_Cfg(MT_UNF_HDMI_ID_E hdmi_id, HDMI_CLK_CFG_S* cfg_info)
{
	HDMI_CLK_CFG_S cfg_info_cal;
	hdmi20_params_t *params = NULL;

	HDMI_CHECK_ID(hdmi_id);
	if (cfg_info == NULL) {
		return MT_FAILURE;
	}
	DRV_HDMI_Vcfg_Status(3);
	cfg_info_cal = *cfg_info;
	(void)DRV_HDMI_Clk_Cfg_Calibration(hdmi_id, &cfg_info_cal);
	params = hdmi20_params_get();
	cfg_info->pdpi_on = params->clk_cfg & 0x01;
	if ( params->clk_cfg ) {
		cfg_info->ssc_on = (params->clk_cfg>>1) & 0x01;
	} else {
		if ( cfg_info->clk < 297000000 ) {
			cfg_info->ssc_on = 1;
		} else {
			cfg_info->ssc_on = 0;
		}
	}
	HDMI20_DRV_HDMI_PRINTK("ana clkcfg: orig[%d %d] new[%d %d] [%d %d %d]\n",cfg_info->clk,cfg_info->clk_os,cfg_info_cal.clk,cfg_info_cal.clk_os,\
		cfg_info->pdpi_on,cfg_info->ssc_on,cfg_info->vout_clk_gate_on);
	mta_hdmi_clk_cfg_v2(cfg_info_cal.clk, cfg_info->ssc_on, cfg_info->pdpi_on, cfg_info_cal.vout_clk_gate_on, cfg_info_cal.clk_os);
	MT_ANALOG_UP_ATTR(MT_ANA_INDEX_HDMITX_R2, struct mt_analog_hdmitx_r2_attr, d_reg, 2);
	return MT_SUCCESS;
}

static mt_void DRV_HDMI_Emp_Mtw_Cfg(mt_u32 opt)
{
	mt_u32 start_line = 0;
	mt_u32 endline = 0;
	mt_u32 vblank = 0;
	mt_u32 vsync_high = 0;
	mt_u32 vsync_low = 0;
	mt_u32 vfront = 0;
	mt_u32 vback = 0;
	mt_u32 vactive = 0;
	mt_u32 interlace = 0;

	interlace = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_STATUS) & BIT_MSK__VP__FDET_STATUS__INTERLACED;
	switch ( opt ) {
		case 0:
			start_line = 0;
			endline += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN);
			endline += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN + 1) << 8;
			endline += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN);
			endline += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN + 1) << 8;
			if ( endline ) {
				endline -= 1;
			}
			//endline = start_line + 1; //only for test send fail
			break;
		case 1:
			start_line = 1;
			vsync_high += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN);
			vsync_high += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN + 1) << 8;
			vsync_low = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN);
			vsync_low += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN + 1) << 8;
			vfront = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VFRONT_COUNT_EVEN);
			vfront += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VFRONT_COUNT_EVEN + 1) << 8;
			vback = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VBACK_COUNT_EVEN);
			vback += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VBACK_COUNT_EVEN + 1) << 8;
			if ((SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_STATUS) & 0x3) == 0) {
				vblank = vsync_low + vfront + vback;
				vactive = vsync_high - vfront - vback;
			} else {
				vblank = vsync_high + vfront + vback;
				vactive = vsync_low - vfront - vback;
			}
			endline = vactive + (vblank >> 1);
			if ( endline ) {
				endline -= 1;
			}
			break;
		case 2:
			vsync_high += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN);
			vsync_high += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN + 1) << 8;
			vsync_low = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN);
			vsync_low += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN + 1) << 8;
			vfront = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VFRONT_COUNT_EVEN);
			vfront += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VFRONT_COUNT_EVEN + 1) << 8;
			vback = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VBACK_COUNT_EVEN);
			vback += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VBACK_COUNT_EVEN + 1) << 8;
			if ((SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_STATUS) & 0x3) == 0) {
				vblank = vsync_low + vfront + vback;
				vactive = vsync_high - vfront - vback;
			} else {
				vblank = vsync_high + vfront + vback;
				vactive = vsync_low - vfront - vback;
			}
			#if 0
			start_line = vactive;
			endline = vactive + (vblank >> 1);
			#else
			start_line = vactive+1-(interlace>0);
			endline = vactive + (vblank >> 1);
			#endif
			if ( endline ) {
				endline -= 1;
			}
			break;
		default:
			break;
	}
	SiiDrvCraWrReg8((SiiInst_t)0, REG_ADDR__EMP_START_LINE_LOW, (uint8_t)(start_line & 0xff));
	SiiDrvCraWrReg8((SiiInst_t)0, REG_ADDR__EMP_START_LINE_HIGH, (uint8_t)((start_line >> 8) & 0xff));
	DRV_HDMI_Emp_Assert_Endpix(endline, start_line);
	SiiDrvCraWrReg8((SiiInst_t)0, REG_ADDR__EMP_END_LINE_LOW, (uint8_t)(endline & 0xff));
	SiiDrvCraWrReg8((SiiInst_t)0, REG_ADDR__EMP_END_LINE_HIGH, (uint8_t)((endline >> 8) & 0xff));
	#if 0
	HDMI20_DRV_HDMI_PRINTK("[%s_%d] start_line=%d,end_line=%d,vsync_high=%d,vsync_low=%d,vfront=%d,vback=%d,va=%d,opt=%d\n", \
						   __func__, __LINE__, start_line, endline, vsync_high, vsync_low, vfront, vback, vactive, opt);
	#endif
}

mt_void DRV_HDMI_Emp_Dma_Mtw_Cfg(mt_u32 opt)
{
	mt_u32 start_line = 0;
	mt_u32 endline = 0;
	mt_u32 start_pix = 0;
	mt_u32 endpix = 0;
	mt_u32 vblank = 0;
	mt_u32 vsync_high = 0;
	mt_u32 vsync_low = 0;
	mt_u32 vfront = 0;
	mt_u32 vback = 0;
	mt_u32 vactive = 0;
	mt_u32 htotal_high = 0;
	mt_u32 htotal_low = 0;
	SiiDrvCraAddr_t reg_addr;

	reg_addr = REG_ADDR__EMP_CTRL1;
	if ( opt ) {
		SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_EMP_MTW_MODE, BIT_MSK__REG_EMP_MTW_MODE);
	} else {
		SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_EMP_MTW_MODE, 0);
	}
	switch ( opt ) {
		case 0:
			// in two frames
			start_line = 0;
			endline += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN);
			endline += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN + 1) << 8;
			endline += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN);
			endline += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN + 1) << 8;
			if ( endline ) {
				endline -= 1;
			}

			htotal_low += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_HSYNC_LOW_COUNT);
			htotal_low += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_HSYNC_LOW_COUNT + 1) << 8;
			htotal_high += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_HSYNC_HIGH_COUNT);
			htotal_high += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_HSYNC_HIGH_COUNT + 1) << 8;
			if (start_pix < endpix) {
				//error
			}

			break;
		case 1:
			// in one frame
			start_line = 1;
			vsync_high += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN);
			vsync_high += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN + 1) << 8;
			vsync_low = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN);
			vsync_low += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN + 1) << 8;
			vfront = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VFRONT_COUNT_EVEN);
			vfront += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VFRONT_COUNT_EVEN + 1) << 8;
			vback = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VBACK_COUNT_EVEN);
			vback += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_VBACK_COUNT_EVEN + 1) << 8;
			if ((SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_STATUS) & 0x3) == 0) {
				vblank = vsync_low + vfront + vback;
				vactive = vsync_high - vfront - vback;
			} else {
				vblank = vsync_high + vfront + vback;
				vactive = vsync_low - vfront - vback;
			}
			endline = vactive + (vblank >> 1);
			if ( endline ) {
				endline -= 1;
			}
			htotal_low += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_HSYNC_LOW_COUNT);
			htotal_low += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_HSYNC_LOW_COUNT + 1) << 8;
			htotal_high += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_HSYNC_HIGH_COUNT);
			htotal_high += SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_HSYNC_HIGH_COUNT + 1) << 8;
			if (start_pix > endpix) {
				//error
			}
			break;
		default:
			break;
	}

	reg_addr = REG_ADDR__EMP_MTW_START_CFG;
	start_pix = start_pix * (htotal_high + htotal_low);
	SiiDrvCraWrReg32((ulong)NULL, reg_addr, start_pix);
	reg_addr = REG_ADDR__EMP_MTW_END_CFG;
	endpix = endline * (htotal_high + htotal_low);
	DRV_HDMI_Emp_Assert_Endpix(endpix, start_pix);
	SiiDrvCraWrReg32((ulong)NULL, reg_addr, endpix);
	#if 0
	HDMI20_DRV_HDMI_PRINTK("[%s_%d] start_line=%d,end_line=%d,vsync_high=%d,vsync_low=%d,vfront=%d,vback=%d,va=%d,opt=%d\n", \
						   __func__, __LINE__, start_line, endline, vsync_high, vsync_low, vfront, vback, vactive, opt);
	#endif
}

static mt_void hdmi_pack_emp(EMP_TYPE_T type, uint8_t *md, uint32_t len, uint8_t *emp_buf, EMP_EM_T *emp_em, uint32_t hlen)
{
	EMP_T	emp_pack;
	uint32_t	i;
	uint8_t	pd_offset = 0;

	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] md ptr=%p,len=%d,emp_buf=%p,itype=%d\n",__func__,__LINE__,md,len,emp_buf,type);
	memset(&emp_pack, 0, sizeof(EMP_T));
	switch (type) {
		case EMP_TYPE_HDR_DYNAMIC:
			emp_pack.hb[0] = 0x7f; //fixed
			emp_pack.hb[1] = (emp_em->first << 7) | (emp_em->last << 6 );
			emp_pack.hb[2] = emp_em->seq_idx;

			if (emp_em->seq_idx == 0) {
				emp_em->DS_Type = 1;
				emp_em->Sync = 1;
				emp_em->VFR = 1;
				emp_em->AFR = 0;
				emp_em->New = 1;//the payload is the same as the last sent.
				emp_em->end = 0;//Need to be done
				emp_em->Organization_ID = 2;
				emp_pack.pb[0] = (emp_em->New << 7) | (emp_em->end << 6) | ((emp_em->DS_Type & 0x03) << 4) | \
								 (emp_em->AFR << 3) | (emp_em->VFR << 2) | (emp_em->Sync << 1);
				emp_pack.pb[2] = emp_em->Organization_ID;
				emp_pack.pb[3] = emp_em->Data_Set_Tag_MSB;
				emp_pack.pb[4] = emp_em->Data_Set_Tag_LSB;
				emp_pack.pb[5] = emp_em->Data_Set_Length_MSB;
				emp_pack.pb[6] = emp_em->Data_Set_Length_LSB;
				pd_offset = 7;
			}
			for (i = 0; i < len; i++) {
				emp_pack.pb[pd_offset + i] = md[i];
			}
			memcpy(emp_buf, &(emp_pack.hb[0]), 3 + hlen);
			memcpy(emp_buf + 3 + hlen, &(emp_pack.pb[0]), 28);
			//HDMI20_DRV_HDMI_PRINTK("[%s_%d] pd_offset=%d\n",__func__,__LINE__,pd_offset);
			break;
		case EMP_TYPE_COMP_VT:
			emp_pack.hb[0] = 0x7f; //fixed
			emp_pack.hb[1] = (emp_em->first << 7) | (emp_em->last << 6 );
			emp_pack.hb[2] = emp_em->seq_idx;
			if (emp_em->seq_idx == 0) {
				emp_em->DS_Type = 0;
				emp_em->Sync = 0;
				emp_em->VFR = 1;
				emp_em->AFR = 0;
				emp_em->New = 1;//the payload is the same as the last sent.
				emp_em->end = 0;//Need to be done
				emp_em->Organization_ID = 1;
				emp_pack.pb[0] = (emp_em->New << 7) | (emp_em->end << 6) | ((emp_em->DS_Type & 0x03) << 4) | \
								 (emp_em->AFR << 3) | (emp_em->VFR << 2) | (emp_em->Sync << 1);
				emp_pack.pb[2] = emp_em->Organization_ID;
				emp_pack.pb[3] = 0x00;//emp_em->Data_Set_Tag_MSB;
				emp_pack.pb[4] = 0x01;//emp_em->Data_Set_Tag_LSB;
				emp_pack.pb[5] = 0x00;//emp_em->Data_Set_Length_MSB;
				emp_pack.pb[6] = 0x04;//emp_em->Data_Set_Length_LSB;
				pd_offset = 7;
			}
			for (i = 0; i < len; i++) {
				emp_pack.pb[pd_offset + i] = md[i];
			}
			memcpy(emp_buf, &(emp_pack.hb[0]), 3 + hlen);
			memcpy(emp_buf + 3 + hlen, &(emp_pack.pb[0]), 28);
			break;
		default:
			break;
	}
}

static uint32_t hdmi_set_vsem_pack(void *md, uint32_t len, void *emp_buf, uint32_t itype, uint32_t hlen)
{
#define MAX_MD_PAYLOAD_SIZE	(36*28+21)
	uint32_t i, j;
	EMP_EM_T	emp_em;
	uint8_t md_num;
	uint32_t len_t;
	uint32_t len_valid;
	uint32_t md_offset = 0;
	uint32_t buf_offset = 0;

	len_valid = len;
	if ( len_valid > MAX_MD_PAYLOAD_SIZE ) {
		len_valid = MAX_MD_PAYLOAD_SIZE;
	}

	len_t = len_valid;
	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] md ptr=%p,len=%d,emp_buf=%p,itype=%d\n",__func__,__LINE__,md,len,emp_buf,itype);
	memset(&emp_em, 0, sizeof(EMP_EM_T));

	j = 0;
	for (i = 0; i < len_valid;) {
		if (i == 0) {
			if (len_t > 21) {
				md_num = 21;
				emp_em.last = false;
			} else {
				md_num = len_t;
				emp_em.last = true;
			}
			emp_em.first = true;
			emp_em.Data_Set_Tag_LSB = itype & 0xff;
			emp_em.Data_Set_Tag_MSB = (itype >> 8) & 0xff;
			emp_em.Data_Set_Length_LSB = len_valid & 0xff;
			emp_em.Data_Set_Length_MSB = (len_valid >> 8) & 0xff;
		} else if ( len_t > 28 ) {
			md_num = 28;
			emp_em.first = false;
			emp_em.last = false;
		} else {
			md_num = len_t;
			emp_em.first = false;
			emp_em.last = true;
		}
		//HDMI20_DRV_HDMI_PRINTK("[%s_%d] md_num=%d,left=%d,acc=%d,idx=%d\n",__func__,__LINE__,md_num,len_t-md_num, i+md_num,emp_em.seq_idx);
		hdmi_pack_emp(EMP_TYPE_HDR_DYNAMIC, md + md_offset, md_num, emp_buf + buf_offset, &emp_em, hlen);
		//hdmi_pack_emp(EMP_TYPE_COMP_VT, md+md_offset, md_num, emp_buf + buf_offset, &emp_em);
		len_t -= md_num;
		md_offset += md_num;
		emp_em.seq_idx++;
		if ( hlen && ((i + md_num) >= len_valid || j == 36)) {
			//the last packet
			((uint8_t *)emp_buf)[buf_offset + 3] = 0x01;
		}
		buf_offset += 31 + hlen;
		i += md_num;
		j++;
		if ( j >= 37 ) {
			break;
		}
	}
	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] emp len=%d\n",__func__,__LINE__,buf_offset);
	return buf_offset;
}

#if	HDMI20_EMP_DMA
static mt_s32 hdmi20_mem_to_sram_dma(mt_u32 dst_offset, mmz_buffer_s mmz_src, mt_u32 len)
{
	mt_s32 ret = 0;
	mt_u32 count = 0, loops = 0;
	mt_u8 *vir_src_addr = (mt_u8*)mmz_src.startVirAddr;
	hal_dma_param_t dma_param = {0};
	int chn_id = 0;
	mt_u32 hdmi_ram_reg_base = 0;

	count = len;

	if (NULL == vir_src_addr) {
		printk(KERN_EMERG "smc dma mmz buf==NULL\n");
		return -1;
	}

	hdmi_ram_reg_base = HdmiDrvRegPhyBaseGet(NULL, HDMI_MOD_DIG) + REG_ADDR__EMP_RAM + dst_offset;

	dma_param.mode = DMA_MODE_2D;
	dma_param.alu_fill_data = 0;
	dma_param.alu_fill_width = 0;
	dma_param.len = count;
	dma_param.dst_width = 0;
	dma_param.phy_dst_addr = hdmi_ram_reg_base;
	dma_param.vir_dst_addr = (void *)SYMPHONY_IO_VA(hdmi_ram_reg_base);
	dma_param.dst_leap = 0;
	dma_param.src_width = 0;
	dma_param.phy_src_addr = mmz_src.startPhyAddr;
	dma_param.vir_src_addr = mmz_src.startVirAddr;
	dma_param.src_leap = 0;
	dma_param.config.dst_peripheral = 0xd; // HDMI
	dma_param.config.src_peripheral = 0xf; // memory
	dma_param.config.dst_endian = 0; // big endian
	dma_param.config.src_endian = 0; // big endian
	dma_param.config.dst_clk = 1; // AHB clock
	dma_param.config.src_clk = 0; // AXI clock
	dma_param.config.dst_i = DMA_ADDR_FIX;
	dma_param.config.src_i = DMA_ADDR_INC;
	dma_param.config.dst_usize = DMA_USIZE_32BIT;
	dma_param.config.src_usize = DMA_USIZE_64BIT;
	dma_param.config.dst_bsize = DMA_BURST_NUM1;
	dma_param.config.src_bsize = DMA_BURST_NUM8;
	dma_param.control.int_link_en = 1;
	dma_param.control.int_node_en = 1;
	dma_param.control.chn_param_reg_en = 0;
	dma_param.p_next = NULL;
	chn_id = 0;//hal_dma_get_free_channel(0);

	//printk(KERN_EMERG "hdmi dma start 1 %x,%x\n",count,chn_id);
	ret = hal_dma_start(chn_id, &dma_param, NULL);
	//printk(KERN_EMERG "hdmi dma start 2:%x\n",ret);
	loops = 0;
	while (DMA_STATUS_STOP != hal_dma_check(chn_id)) {
		loops++;
		//if (loops > 1000000) {
		if (loops > 100) {
			HDMI20_DRV_HDMI_PRINTK(KERN_EMERG "hdmi dma send timeout\n");
			ret = -1;
			break;
		}
		udelay(100);
	}
	hal_dma_stop(chn_id);

	return ret;
}
#endif

mt_u8 DRV_HDMI_HDR10p_Select(EXTENDED_INFOFRAME_TYPE_T eitype)
{
	MT_UNF_EDID_BASE_INFO_S sinkAttr;
	mt_u8 ret = 0;

	memset(&sinkAttr, 0, sizeof(MT_UNF_EDID_BASE_INFO_S));
	if (MT_SUCCESS == DRV_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_0, &sinkAttr)) {
		if ( sinkAttr.bSupportHDR10pVSIF ) {
			ret = 0;
		} else if ( (sinkAttr.u16SupportEMP) & eitype ) {
			ret = 1;
		} else {
			HDMI20_DRV_HDMI_PRINTK("[%s_%d] Unsupport HDR10p format:%x\n", __func__, __LINE__, (mt_u32)eitype);
		}
	}
	return ret;
}

mt_s32 DRV_HDMI_Emp_Cfg_Cpu_Only(MT_UNF_HDMI_ID_E hdmi_id, void* md_ptr, mt_u32 len)
{
	uint32_t i;
	uint8_t	emp_buff[REG__EMP_RAM_SIZE * 4];
	uint32_t	emp_len = 0xff;
	uint32_t 	emp_val = 0;
	EMP_DATA_IN_T din = {0};

	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] Enter\n",__func__,__LINE__);
	HDMI_CHECK_ID(hdmi_id);
	HDMI_CheckChnOpenNoPrint(hdmi_id);

	if ( md_ptr == NULL ) {
		return MT_FAILURE;
	}

	din = *((EMP_DATA_IN_T *)md_ptr);
	HDMI20_DRV_HDMI_EMP7_PRINTK("[%s_%d] mode:%d,%d\n", __func__, __LINE__, din.mode, din.itype);
	if ( DRV_HDMI_HDR10p_Select(din.itype)) {
		din.mode = HDR_DATA_TYPE_EMP;
	}

	if ( din.mode == HDR_DATA_TYPE_VSIF ) {
		SiiInfoFrame_t info_st = {0};

		memset(&info_st, 0, sizeof(SiiInfoFrame_t));
		memset(emp_buff, 0, sizeof(emp_buff));
		SII_MEMCPY((uint8_t*)emp_buff, din.data1p, 31);
		SII_MEMCPY((uint8_t*)&info_st.b, din.data1p, 31);
		#if 0
		{
			static uint8_t	emp_buff_back[REG__EMP_RAM_SIZE * 4];
			static uint32_t emp_buff_cnt = 0;
			if ( memcmp((void *)emp_buff_back, (void *)emp_buff, 31)) {
				HDMI20_DRV_HDMI_EMP7_PRINTK("[%s_%d] vsif change:%2x_%2x_%2x_%2x [%d]\n", __func__, __LINE__, emp_buff[3], emp_buff[2], emp_buff[1], emp_buff[0],emp_buff_cnt);
			}
			SII_MEMCPY((uint8_t*)emp_buff_back, (uint8_t*)emp_buff, 31);
			emp_buff_cnt++;
		}
		#endif
	}

	#if 1
	SiiDrvCraPutBit8((SiiInst_t)0, REG_ADDR__INTR_MASK, BIT_MSK_REG__EMP_SUCCESS_HDMI_MASK | \
					 BIT_MSK_REG__EMP_ERR_HDMI_MASK | BIT_MSK_REG__EMP_ERR_CPU_MASK | \
					 BIT_MSK_REG__MTW_FALLING_EDGE_MASK | BIT_MSK_REG__DMA_DONE_MASK,
					 BIT_MSK_REG__EMP_SUCCESS_HDMI_MASK | \
					 BIT_MSK_REG__EMP_ERR_HDMI_MASK | BIT_MSK_REG__EMP_ERR_CPU_MASK);
	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] mask:0x%x\n",__func__,__LINE__,SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__INTR_MASK));
	#endif
	//if(cfg_info == NULL)  return MT_FAILURE;
	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] Execute0...\n",__func__,__LINE__);
	if (SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__INTR_STATUS) & (BIT_MSK_REG__EMP_ERR_HDMI | BIT_MSK_REG__EMP_ERR_CPU)) {
		SiiDrvCraSetBit8((SiiInst_t)0, REG_ADDR__INTR_STATUS, BIT_MSK_REG__EMP_ERR_HDMI | BIT_MSK_REG__EMP_ERR_CPU);
		//HDMI20_DRV_HDMI_PRINTK("[%s_%d] EMP FAIL one time\n",__func__,__LINE__);
	}
	DRV_HDMI_Emp_Mtw_Cfg(din.mtw_mode);

	if ( din.repeat ) {
		SiiDrvCraSetBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_EMP_REPEAT);
		return MT_SUCCESS;
	} else {
		SiiDrvCraClrBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_EMP_REPEAT);
	}

	{
		mt_u32 delay;
		mt_u8 interlace;
		interlace = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_STATUS) & BIT_MSK__VP__FDET_STATUS__INTERLACED;
		if ( interlace ) {
			delay = 50;
		} else {
			delay = 2;
		}
		udelay(delay);
	}

	//config mtw

	SiiDrvCraSetBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_AUD_EMP_PRIORITY);

	SiiDrvCraSetBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_EMP_START);

	if ( din.mode == HDR_DATA_TYPE_EMP ) {
		emp_len = hdmi_set_vsem_pack(din.data1p, len, emp_buff, (uint32_t)din.itype, 0);
		//HDMI20_DRV_HDMI_EMP8_PRINTK("[%s_%d] %2x_%2x_%2x_%2x\n", __func__, __LINE__, (mt_u32)emp_buff[3], (mt_u32)emp_buff[2], (mt_u32)emp_buff[1], (mt_u32)emp_buff[0]);
	} else if ( din.mode == HDR_DATA_TYPE_VSIF ) {
		emp_len =  31;
	} else {
		HDMI20_DRV_HDMI_PRINTK("\nError HDR mode:%d \n", din.mode);
		return MT_FAILURE;
	}

	for (i = 0; i < emp_len; ) {
		emp_val = emp_buff[i] | (emp_buff[i + 1] << 8) | (emp_buff[i + 2] << 16) | (emp_buff[i + 3] << 24);
		//HDMI20_DRV_HDMI_PRINTK("[%s_%d] i=%d\n",__func__,__LINE__,i);
		SiiDrvCraWrReg32((SiiInst_t)0, REG_ADDR__EMP_RAM + i, emp_val);
		i += 4;
	}

	DRV_HDMI_Emp_Assert_Delaycnt();
	{
		SiiDrvCraSetBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_EMP_FINISH);
	}

	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] One frame done\n",__func__,__LINE__);
	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_Emp_Cfg_Dma(MT_UNF_HDMI_ID_E hdmi_id, void* md_ptr, mt_u32 len, HDMI20_EMP_MODE_T mode)
{
	uint32_t i;
	uint8_t	emp_buff[REG__EMP_DMA_RAM_SIZE];
	uint32_t	emp_len;
	uint32_t 	emp_val = 0;
	EMP_DATA_IN_T din = {0};
	SiiDrvCraAddr_t reg_addr;
	uint8_t	emp_dma_pkt_num = 0;
	static uint8_t 	emp_dma_ram_sel = 0;

	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] Enter\n",__func__,__LINE__);
	HDMI_CHECK_ID(hdmi_id);
	HDMI_CheckChnOpenNoPrint(hdmi_id);

	if ( md_ptr == NULL ) {
		return MT_FAILURE;
	}

	#if 1
	SiiDrvCraPutBit8((SiiInst_t)0, REG_ADDR__INTR_MASK, BIT_MSK_REG__EMP_SUCCESS_HDMI_MASK | \
					 BIT_MSK_REG__EMP_ERR_HDMI_MASK | BIT_MSK_REG__EMP_ERR_CPU_MASK | \
					 BIT_MSK_REG__MTW_FALLING_EDGE_MASK | BIT_MSK_REG__DMA_DONE_MASK,
					 BIT_MSK_REG__MTW_FALLING_EDGE_MASK | BIT_MSK_REG__DMA_DONE_MASK);
	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] mask:0x%x\n",__func__,__LINE__,SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__INTR_MASK));
	#endif
	din = *((EMP_DATA_IN_T *)md_ptr);

	if ( DRV_HDMI_HDR10p_Select(din.itype) ) {
		din.mode = HDR_DATA_TYPE_EMP;
	}
	reg_addr = REG_ADDR__HDR10PLUS_CFG;
	if (((EMP_DATA_IN_T *)md_ptr)->mode == HDR_DATA_TYPE_VSIF) {
		SiiDrvCraPutBit8((ulong)NULL, reg_addr, BIT_MSK__HDR10P_WIN, 0x01);
	} else if (((EMP_DATA_IN_T *)md_ptr)->mode == HDR_DATA_TYPE_EMP) {
		SiiDrvCraPutBit8((ulong)NULL, reg_addr, BIT_MSK__HDR10P_WIN, 0x00);
	} else {
		SiiDrvCraPutBit8((ulong)NULL, reg_addr, BIT_MSK__HDR10P_WIN, 0x00);
		HDMI20_DRV_HDMI_PRINTK("\nError HDR mode:%d \n", ((EMP_DATA_IN_T *)md_ptr)->mode);
		return MT_FAILURE;
	}

	reg_addr = REG_ADDR__EMP_CTRL1;
	din.dma_bank_auto = 0;//force to manual mode, IC suggest to use manual mode.
	if ( din.dma_bank_auto ) {
		SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_MEM_BANK_MODE | BIT_MSK__REG_WR_MEM_BANK_MODE, 0x00);
	} else {
		SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_MEM_BANK_MODE | BIT_MSK__REG_WR_MEM_BANK_MODE, \
						  BIT_MSK__REG_MEM_BANK_MODE | BIT_MSK__REG_WR_MEM_BANK_MODE);
	}

	{
		uint32_t reg_t = 0;
		reg_addr = REG_ADDR__EMP_CTRL1;
		reg_t = SiiDrvCraRdReg32((ulong)NULL, reg_addr);
		if (( reg_t & BIT_MSK__REG_EMP_DONE ) == 0) {
			HDMI20_DRV_HDMI_PRINTK("\n EMP Not Done\n");
		}
		if ( mode == HDMI20_EMP_DMA_DMA ) {
			if (( reg_t & BIT_MSK__REG_DMA_DONE ) == 0) {
				HDMI20_DRV_HDMI_PRINTK("\n EMP DMA Not Done\n");
				if ( din.emp_dma_start == 0 && 0) {
					SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_DMA_STOP, BIT_MSK__REG_DMA_STOP);
					while ((SiiDrvCraRdReg32((ulong)NULL, reg_addr) & BIT_MSK__REG_DMA_DONE) == 0) {
						;
					}
				}
			} else {
				HDMI20_DRV_HDMI_PRINTK("\n EMP DMA Done\n");
			}
		}
	}

	if ( din.repeat ) {
		SiiDrvCraPutBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_EMP_START | BIT_MSK__REG_DMA_EN | BIT_MSK__REG_EMP_EN | BIT_MSK__REG_EMP_REPEAT_1, \
						 BIT_MSK__REG_EMP_START | BIT_MSK__REG_EMP_EN | BIT_MSK__REG_EMP_REPEAT_1);
		return MT_SUCCESS;
	} else {
		SiiDrvCraPutBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_EMP_REPEAT_1, 0);
	}

	if (din.emp_dma_start) {
		emp_dma_ram_sel = 0;
	} else {
		emp_dma_ram_sel = 1 - emp_dma_ram_sel;
	}

	if ( din.mode == HDR_DATA_TYPE_EMP ) {
		if ( len <= 21 ) {
			emp_dma_pkt_num = 1;
		} else {
			emp_dma_pkt_num = (len - 21) / 28 + ((len - 21) % 28 > 0) + 1;
			if (emp_dma_pkt_num > 37) {
				emp_dma_pkt_num = 37;
			}
		}
		reg_addr = REG_ADDR__EMP_CTRL1;
		memset(emp_buff, 0, sizeof(emp_buff));
		SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_DMA_PKT_NUM, emp_dma_pkt_num << 8);
		emp_len = hdmi_set_vsem_pack(din.data1p, len, emp_buff, (uint32_t)din.itype, 1);
	} else if ( din.mode == HDR_DATA_TYPE_VSIF ) {
		SiiInfoFrame_t info_st = {0};

		memset(&info_st, 0, sizeof(SiiInfoFrame_t));
		memset(emp_buff, 0, sizeof(emp_buff));
		emp_buff[3] = 0x01;	//EOF = 1
		SII_MEMCPY((uint8_t*)emp_buff, din.data1p, 3);
		SII_MEMCPY((uint8_t*)emp_buff + 4, din.data1p + 3, 28);
		SII_MEMCPY((uint8_t*)&info_st.b, din.data1p, 31);
		emp_dma_pkt_num = 1;
		emp_len =  32;
		reg_addr = REG_ADDR__EMP_CTRL1;
		SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_DMA_PKT_NUM, emp_dma_pkt_num << 8);
	}

	switch (mode) {
		case HDMI20_EMP_DMA_CPU:
			reg_addr = REG_ADDR__EMP_CTRL1;
			SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_CPU_EN, BIT_MSK__REG_CPU_EN);

			DRV_HDMI_Emp_Dma_Mtw_Cfg(din.mtw_mode);
			reg_addr = REG_ADDR__EMP_CTRL1;
			SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_WR_MEM_BANK_CFG, emp_dma_ram_sel ? 0 : BIT_MSK__REG_WR_MEM_BANK_CFG);
			SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_MEM_BANK_CFG, emp_dma_ram_sel ? BIT_MSK__REG_MEM_BANK_CFG : 0);
			for (i = 0; i < emp_len; ) {
				emp_val = emp_buff[i] | (emp_buff[i + 1] << 8) | (emp_buff[i + 2] << 16) | (emp_buff[i + 3] << 24);
				//HDMI20_DRV_HDMI_PRINTK("[%s_%d] i=%d, val[%d]:0x%x\n",__func__,__LINE__,i,i,emp_val);
				#if HDMI20_EMP_DMA_ADDR_FIX
				//addr should be fixed in golden version
				SiiDrvCraWrReg32((SiiInst_t)0, REG_ADDR__EMP_RAM, emp_val);
				#else
				SiiDrvCraWrReg32((SiiInst_t)0, REG_ADDR__EMP_RAM + i, emp_val);
				#endif
				i += 4;
			}
			break;
		case HDMI20_EMP_DMA_DMA:
			reg_addr = REG_ADDR__EMP_CTRL1;
			SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_CPU_EN, 0);
			DRV_HDMI_Emp_Dma_Mtw_Cfg(din.mtw_mode);
			reg_addr = REG_ADDR__EMP_CTRL1;
			SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_WR_MEM_BANK_CFG, emp_dma_ram_sel ? 0 : BIT_MSK__REG_WR_MEM_BANK_CFG);
			SiiDrvCraPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_MEM_BANK_CFG, emp_dma_ram_sel ? BIT_MSK__REG_MEM_BANK_CFG : 0);
			break;
		default:
			break;
	}

	{
		mt_u32 delay;
		mt_u8 interlace;
		interlace = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__VP__FDET_STATUS) & BIT_MSK__VP__FDET_STATUS__INTERLACED;
		if ( interlace ) {
			delay = 50;
		} else {
			delay = 2;
		}
		udelay(delay);
	}

	//config mtw
	SiiDrvCraSetBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_AUD_EMP_PRIORITY);

	if ( mode == HDMI20_EMP_DMA_CPU ) {
		if ( din.emp_dma_start ) {
			SiiDrvCraPutBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_EMP_START | BIT_MSK__REG_DMA_EN | BIT_MSK__REG_EMP_EN | BIT_MSK__REG_EMP_REPEAT_1, \
							 BIT_MSK__REG_EMP_START);
		} else {
			SiiDrvCraPutBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_EMP_START | BIT_MSK__REG_DMA_EN | BIT_MSK__REG_EMP_EN | BIT_MSK__REG_EMP_REPEAT_1, \
							 BIT_MSK__REG_EMP_START | BIT_MSK__REG_EMP_EN);
		}
	} else {
		if (din.emp_dma_start) {
			SiiDrvCraPutBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_EMP_START | BIT_MSK__REG_DMA_EN | BIT_MSK__REG_EMP_EN, \
							 BIT_MSK__REG_EMP_START | BIT_MSK__REG_DMA_EN);
		} else {
			SiiDrvCraSetBit8((SiiInst_t)0, REG_ADDR__EMP_CTRL, BIT_MSK__REG_EMP_START | BIT_MSK__REG_EMP_EN | BIT_MSK__REG_DMA_EN);
		}
		{
			mmz_buffer_s  mmz_mem_src = {0};
			mmz_mem_src = hdmi20_emp_dma_buff_get(0);
			#if 0
			HDMI20_DRV_HDMI_PRINTK("[%s_%d] %lu,%llx,%llu  686:0x%x\n", __func__, __LINE__, \
								   mmz_mem_src.size, (uint64_t)mmz_mem_src.startVirAddr, mmz_mem_src.startPhyAddr, (uint32_t)SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__EMP_CTRL));
			#endif
			memcpy(mmz_mem_src.startVirAddr, emp_buff, emp_dma_pkt_num << 5);
			hdmi20_mem_to_sram_dma(0, mmz_mem_src, emp_dma_pkt_num << 5);
		}
	}
	#if 0
	HDMI20_DRV_HDMI_PRINTK("[%s_%d] One frame done :%d %d 0x%x %d %d\n", __func__, __LINE__, \
						   (uint32_t)mode, (uint32_t)din.emp_dma_start, (uint32_t)SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__EMP_CTRL), din.repeat, din.data1l);
	#endif
	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_Emp_Cfg(MT_UNF_HDMI_ID_E hdmi_id, void* md_ptr, mt_u32 len)
{
	HDMI20_EMP_MODE_T emp_mode;
	mt_s32 ret = MT_SUCCESS;
	EMP_DATA_IN_T *datainp = (EMP_DATA_IN_T *)md_ptr;
	static uint32_t	emp_dma_flg = 0;
	SiiDrvCraAddr_t reg_addr;

	HDMI_CHECK_ID(hdmi_id);
	HDMI_CheckChnOpenNoPrint(hdmi_id);
	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] Enter\n",__func__,__LINE__);

	if ( md_ptr == NULL || (DRV_HDMI_SwRst_Status_Get() && DRV_HDMI_Sw_Status_Get() != 4) ) {
		HDMI20_DRV_HDMI_PRINTK("[%s_%d] emp return:%d\n",__func__,__LINE__,DRV_HDMI_Sw_Status_Get());
		return MT_FAILURE;
	}

	DRV_HDMI_Emp_Assert_Data(md_ptr, len);
	emp_mode = hdmi20_emp_mode_get();
	HDMI20_DRV_HDMI_EMP8_PRINTK("[%s_%d] %2d\n", __func__, __LINE__, emp_mode);
	switch ( emp_mode ) {
		case HDMI20_EMP_CPU_ONLY:
			reg_addr = REG_ADDR__HDR10PLUS_CFG;
			SiiDrvCraPutBit8((ulong)NULL, reg_addr, BIT_MSK__HDR10P_WIN, 0x02);
			datainp->mtw_mode = MTW_CFG_MODE_2;
			ret = DRV_HDMI_Emp_Cfg_Cpu_Only(hdmi_id, md_ptr, len);
			break;
		case HDMI20_EMP_DMA_DMA:
			emp_dma_flg = (datainp->end == 0);
			ret = DRV_HDMI_Emp_Cfg_Dma(hdmi_id, md_ptr, len, emp_mode);
			break;
		case HDMI20_EMP_DMA_CPU:
			reg_addr = REG_ADDR__EMP_CTRL;
			SiiDrvCraPutBit8((ulong)NULL, reg_addr, BIT_MSK__REG_DMA_EN, 0);
			ret = DRV_HDMI_Emp_Cfg_Dma(hdmi_id, md_ptr, len, emp_mode);
			break;
		default:
			ret = MT_FAILURE;
			break;
	}

	//HDMI20_DRV_HDMI_PRINTK("[%s_%d] One frame done\n",__func__,__LINE__);
	return ret;
}

mt_s32 DRV_HDMI_SetOsdName(MT_UNF_HDMI_ID_E hdmi_id, mt_u8 *osd_name)
{
	HDMI_CHECK_ID(hdmi_id);
	//HDMI_CheckChnOpen(hdmi_id);

	if (NULL == osd_name) {
		return MT_FALSE;
	}
	SiiCecUpdateOsdName(osd_name);
	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_Vcfg_Check(MT_UNF_HDMI_ID_E enHdmi, void *param_in, void *param_out)
{

	DISP_HDMI_PARAMS_CHECK_STATUS_T ret = DISP_HDMI_PARAMS_STS_SUCCESS;
	MT_UNF_EDID_BASE_INFO_S    *pSinkCap = DRV_Get_SinkCap(enHdmi);
	DISP_HDMI_PARAM_T *pin = (DISP_HDMI_PARAM_T *)param_in;
	DISP_HDMI_PARAM_T *pout = (DISP_HDMI_PARAM_T *)param_out;
	MT_DRV_DISP_FMT_E  enEncodingFormat = ((DISP_HDMI_PARAM_T *)param_in)->drv_disp_fmt;
	mt_u32 d3flg = 0;
	mt_u32 tmds_clk = 0;
	mt_u32 is4kp5060 = 0;
	mt_u32 d2flg = 0;
	MT_UNF_HDMI_VIDEO_MODE_E csc = MT_UNF_HDMI_VIDEO_MODE_RGB444;

	if (param_in == NULL || param_out == NULL) {
		HDMI20_DRV_HDMI_PRINTK("Err input NULL pointer %p %p\n",param_in,param_out);
		ret |= DISP_HDMI_PARAMS_STS_PTR_ERR;
	} else {
		if (DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
			HDMI_EDID_S pEDID;

			memset(&pEDID, 0, sizeof(HDMI_EDID_S));
			if(DRV_HDMI_Force_GetEDID(&pEDID) != MT_SUCCESS)
			{
				ret |= DISP_HDMI_PARAMS_STS_EDID_ERR;
				*pout = *pin;
				//HDMI20_DRV_HDMI_PRINTK("get EDID Err\n");
				return ret;
			}

			//check tv format
			if (MT_FALSE == pSinkCap->bSupportFormat[hdmi_Disp2EncFmt(enEncodingFormat)]) {
				ret |= DISP_HDMI_PARAMS_STS_FMT_ERR;
			}

			//check 3D flag/CSC
			is4kp5060 = 0;
			switch (pin->drv_disp_fmt) {
				case MT_DRV_DISP_FMT_3840X2160_60:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_3840x2160p_60Hz;
					tmds_clk = 594000000;
					is4kp5060 = 1;
					d2flg = (mt_u32)pEDID.stEdidParsed.supported_3840x2160p_60Hz;
					break;
				case MT_DRV_DISP_FMT_3840X2160_50:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_3840x2160p_50Hz;
					tmds_clk = 594000000;
					is4kp5060 = 1;
					d2flg = (mt_u32)pEDID.stEdidParsed.supported_3840x2160p_50Hz;
					break;
				case MT_DRV_DISP_FMT_3840X2160_30:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_3840x2160p_30Hz;
					tmds_clk = 297000000;
					break;
				case MT_DRV_DISP_FMT_3840X2160_25:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_3840x2160p_25Hz;
					tmds_clk = 297000000;
					break;
				case MT_DRV_DISP_FMT_3840X2160_24:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_3840x2160p_24Hz;
					tmds_clk = 297000000;
					break;
				case MT_DRV_DISP_FMT_4096X2160_60:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_4096x2160p_60Hz;
					tmds_clk = 594000000;
					is4kp5060 = 1;
					d2flg = (mt_u32)pEDID.stEdidParsed.supported_4096x2160p_60Hz;
					break;
				case MT_DRV_DISP_FMT_4096X2160_50:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_4096x2160p_50Hz;
					tmds_clk = 594000000;
					is4kp5060 = 1;
					d2flg = (mt_u32)pEDID.stEdidParsed.supported_4096x2160p_50Hz;
					break;
				case MT_DRV_DISP_FMT_4096X2160_30:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_4096x2160p_30Hz;
					tmds_clk = 297000000;
					break;
				case MT_DRV_DISP_FMT_4096X2160_25:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_4096x2160p_25Hz;
					tmds_clk = 297000000;
					break;
				case MT_DRV_DISP_FMT_4096X2160_24:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_4096x2160p_24Hz;
					tmds_clk = 297000000;
					break;
				case MT_DRV_DISP_FMT_1080P_60:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_1080p_60Hz;
					tmds_clk = 148500000;
					break;
				case MT_DRV_DISP_FMT_1080P_50:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_1080p_50Hz;
					tmds_clk = 148500000;
					break;
				case MT_DRV_DISP_FMT_1080P_30:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_1080p_30Hz;
					tmds_clk = 74250000;
					break;
				case MT_DRV_DISP_FMT_1080P_25:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_1080p_25Hz;
					tmds_clk = 74250000;
					break;
				case MT_DRV_DISP_FMT_1080P_24:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_1080p_24Hz;
					tmds_clk = 74250000;
					break;
				case MT_DRV_DISP_FMT_1080i_60:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_1080i_60Hz;
					tmds_clk = 74250000;
					break;
				case MT_DRV_DISP_FMT_1080i_50:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_1080i_50Hz;
					tmds_clk = 74250000;
					break;
				case MT_DRV_DISP_FMT_720P_60:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_720p_60Hz;
					tmds_clk = 74250000;
					break;
				case MT_DRV_DISP_FMT_720P_50:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_720p_50Hz;
					tmds_clk = 74250000;
					break;
				case MT_DRV_DISP_FMT_480P_60:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_480p_60Hz;
					tmds_clk = 27000000;
					break;
				case MT_DRV_DISP_FMT_576P_50:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_576p_50Hz;
					tmds_clk = 27000000;
					break;
				case MT_DRV_DISP_FMT_PAL:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_576i_50Hz;
					tmds_clk = 27000000;
					break;
				case MT_DRV_DISP_FMT_NTSC:
					d3flg = (mt_u32)pEDID.stEdidParsed.supported_3d_struc_480i_60Hz;
					tmds_clk = 27000000;
					break;
				default:
					d3flg = 0;
					tmds_clk = 0;
					break;
			}
			if (pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__YC420_601 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__YC420_709 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__XVYCC420_601 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__XVYCC420_709 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__YC420_2020) {
					if ( is4kp5060 == 0) {
						ret |= DISP_HDMI_PARAMS_STS_CSC_ERR;
					} else {
						if ((d2flg & 0x2) == 0 || pin->drv_disp_stereo == DISP_STEREO_FPK ) {
							ret |= DISP_HDMI_PARAMS_STS_CSC_ERR;
						} else {
							tmds_clk = 297000000;
						}
					}
					csc = MT_UNF_HDMI_VIDEO_MODE_YCBCR420;
			} else if ( pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__YC422_601 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__YC422_709 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__XVYCC422_601 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__XVYCC422_709 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__YC422_2020) {
				if (pEDID.stEdidParsed.ycbcr422_supported == 0) {
					ret |= DISP_HDMI_PARAMS_STS_CSC_ERR;
				}
				csc = MT_UNF_HDMI_VIDEO_MODE_YCBCR422;
			} else if ( pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__YC444_601 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__YC444_709 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__XVYCC444_601 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__XVYCC444_709 || \
				pin->disp2hdmi_vcfg.output_v_cfg.csc == SII_DRV_CLRSPC__YC444_2020) {
				if (pEDID.stEdidParsed.ycbcr444_supported == 0) {
					ret |= DISP_HDMI_PARAMS_STS_CSC_ERR;
				}
				csc = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
			}
			switch (pin->drv_disp_stereo) {
				case DISP_STEREO_FPK:
					if ((d3flg & D3_INFO_FRAME_PACKING) == 0) {
						ret |= DISP_HDMI_PARAMS_STS_3D_ERR;
					} else if ( tmds_clk == 27000000) {
						ret |= DISP_HDMI_PARAMS_STS_3D_ERR;
					}
					tmds_clk <<= 1;
					break;
				case DISP_STEREO_SBS_HALF:
					if ((d3flg & D3_INFO_SIDE_BY_SIDE_HALF) == 0) {
						ret |= DISP_HDMI_PARAMS_STS_3D_ERR;
					}
					break;
				case DISP_STEREO_TAB:
					if ((d3flg & D3_INFO_TOP_AND_BOTTOM) == 0) {
						ret |= DISP_HDMI_PARAMS_STS_3D_ERR;
					}
					break;
				default:
					break;
			}

			//check output bitdepth
			switch (pin->disp2hdmi_vcfg.output_v_cfg.bitDepth) {
				case SII_DRV_BIT_DEPTH__10_BIT:
					if (tmds_clk == 594000000) {
						ret |= DISP_HDMI_PARAMS_STS_BIT_DEPTH_ERR;
					} else {
						switch (csc) {
							case MT_UNF_HDMI_VIDEO_MODE_YCBCR420:
								if (pEDID.stEdidParsed.y420_30bit == 0) {
									ret |= DISP_HDMI_PARAMS_STS_BIT_DEPTH_ERR;
								}
								break;
							case MT_UNF_HDMI_VIDEO_MODE_YCBCR422:
							case MT_UNF_HDMI_VIDEO_MODE_YCBCR444:
								if (pEDID.stEdidParsed.rgb30bit == 0 || pEDID.stEdidParsed.dc_y444 == 0 ) {
									ret |= DISP_HDMI_PARAMS_STS_BIT_DEPTH_ERR;
								}
								break;
							case MT_UNF_HDMI_VIDEO_MODE_RGB444:
								if (pEDID.stEdidParsed.rgb30bit == 0) {
									ret |= DISP_HDMI_PARAMS_STS_BIT_DEPTH_ERR;
								}
								break;
							default:
								break;
						}
					}
					tmds_clk = tmds_clk*5/4;
					break;
				case SII_DRV_BIT_DEPTH__12_BIT:
					if (tmds_clk == 594000000) {
						ret |= DISP_HDMI_PARAMS_STS_BIT_DEPTH_ERR;
					} else {
						switch (csc) {
							case MT_UNF_HDMI_VIDEO_MODE_YCBCR420:
								if (pEDID.stEdidParsed.y420_36bit == 0) {
									ret |= DISP_HDMI_PARAMS_STS_BIT_DEPTH_ERR;
								}
								break;
							case MT_UNF_HDMI_VIDEO_MODE_YCBCR422:
							case MT_UNF_HDMI_VIDEO_MODE_YCBCR444:
								if (pEDID.stEdidParsed.rgb36bit == 0 || pEDID.stEdidParsed.dc_y444 == 0 ) {
									ret |= DISP_HDMI_PARAMS_STS_BIT_DEPTH_ERR;
								}
								break;
							case MT_UNF_HDMI_VIDEO_MODE_RGB444:
								if (pEDID.stEdidParsed.rgb36bit == 0) {
									ret |= DISP_HDMI_PARAMS_STS_BIT_DEPTH_ERR;
								}
								break;
							default:
								break;
						}
					}
					tmds_clk = tmds_clk*3/2;
					break;
				default:
					break;
			}

			if (tmds_clk > pEDID.stEdidParsed.maxclk && pEDID.stEdidParsed.maxclk >= 27000000) {
				ret |= DISP_HDMI_PARAMS_STS_MAXCLK_ERR;
			}

			//check colorimetry
			switch (pin->disp2hdmi_vcfg.output_v_cfg.std) {
				case SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS:
					if (csc == MT_UNF_HDMI_VIDEO_MODE_RGB444 || \
						pEDID.stEdidParsed.supported_bt2020cycc == 0 || \
						pin->disp2hdmi_vcfg.output_v_cfg.bitDepth <= SII_DRV_BIT_DEPTH__8_BIT) {
						ret |= DISP_HDMI_PARAMS_STS_STD_ERR;
					}
					break;
				case SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS:
					if ((csc == MT_UNF_HDMI_VIDEO_MODE_RGB444 && pEDID.stEdidParsed.supported_bt2020rgb == 0) || \
						(csc != MT_UNF_HDMI_VIDEO_MODE_RGB444 && pEDID.stEdidParsed.supported_bt2020ycc == 0) || \
						pin->disp2hdmi_vcfg.output_v_cfg.bitDepth <= SII_DRV_BIT_DEPTH__8_BIT) {
						ret |= DISP_HDMI_PARAMS_STS_STD_ERR;
					}
					break;
				default:
					break;
			}

			//check HDR
			switch (pin->mode) {
				case HDR_DATA_TYPE_HDR:
					if (pEDID.stEdidParsed.supported_hdr10 == 0) {
						ret |= DISP_HDMI_PARAMS_STS_HDR_ERR;
					}
					break;
				case HDR_DATA_TYPE_HLG:
					if (pEDID.stEdidParsed.supported_hlg == 0) {
						ret |= DISP_HDMI_PARAMS_STS_HDR_ERR;
					}
					break;
				case HDR_DATA_TYPE_VSIF:
					if (pEDID.stEdidParsed.supported_hdr10p_vsif == 0) {
						ret |= DISP_HDMI_PARAMS_STS_HDR_ERR;
					}
					break;
				default:
					break;
			}
		} else {
			ret |= DISP_HDMI_PARAMS_STS_EDID_ERR;
			*pout = *pin;
		}
	}
#if 1
	HDMI20_DRV_HDMI_PRINTK("Video input params:[%d %d %d %d %d] [%d %d %d] [%d %d %d]\n",pin->mode,pin->itype,pin->emp_type,pin->drv_disp_stereo,pin->drv_disp_fmt,
								pin->disp2hdmi_vcfg.hdcp_on_off,pin->disp2hdmi_vcfg.hdmi_3d_cfg.hdmi_3d_res,pin->disp2hdmi_vcfg.hdmi_3d_cfg.hdmi_3d_ext,
								pin->disp2hdmi_vcfg.output_v_cfg.bitDepth,pin->disp2hdmi_vcfg.output_v_cfg.csc,pin->disp2hdmi_vcfg.output_v_cfg.std);
	HDMI20_DRV_HDMI_PRINTK("Video output params:[%d %d %d %d %d] [%d %d %d] [%d %d %d]\n",pout->mode,pout->itype,pout->emp_type,pout->drv_disp_stereo,pout->drv_disp_fmt,
								pout->disp2hdmi_vcfg.hdcp_on_off,pout->disp2hdmi_vcfg.hdmi_3d_cfg.hdmi_3d_res,pout->disp2hdmi_vcfg.hdmi_3d_cfg.hdmi_3d_ext,
								pout->disp2hdmi_vcfg.output_v_cfg.bitDepth,pout->disp2hdmi_vcfg.output_v_cfg.csc,pout->disp2hdmi_vcfg.output_v_cfg.std);
	if (ret) {
		HDMI20_DRV_HDMI_PRINTK("Check Status Err:0x%x\n",ret);
	}
#endif
	return ret;
}

