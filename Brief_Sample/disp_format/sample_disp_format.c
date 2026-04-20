/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*********************************add include here******************************/

#include <fcntl.h>
#include <unistd.h>

#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_unf_demux.h"
#include "mt_unf_descrambler.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "mt_drv_disp.h"
#include "mt_unf_disp.h"
#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_unf_ecs.h"
#include "mt_unf_vo.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_mpi_demux.h"
#include "mt_adp_demux.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
#include "mt_cmdline.h"

/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_DISP_FORMAT_DEBUG

#define MT_DISP_FORMAT_PRINT   printf
#else

#define MT_DISP_FORMAT_PRINT

#endif

#define SAMPLE_DISP_FORMAT_FUNCTION_ENTER()     MT_DISP_FORMAT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DISP_FORMAT_FUNCTION_EXIT()      MT_DISP_FORMAT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DISP_FORMAT_FATAL_PRINT(fmt...)      MT_DISP_FORMAT_PRINT(" [FATAL] " fmt)
#define SAMPLE_DISP_FORMAT_ERR_PRINT(fmt...)        MT_DISP_FORMAT_PRINT(" [ERROR] " fmt)
#define SAMPLE_DISP_FORMAT_WARN_PRINT(fmt...)       MT_DISP_FORMAT_PRINT(" [WARN] "  fmt)
#define SAMPLE_DISP_FORMAT_INFO_PRINT(fmt...)       MT_DISP_FORMAT_PRINT(" [INFO] "  fmt)
#define SAMPLE_DISP_FORMAT_DBG_PRINT(fmt...)        MT_DISP_FORMAT_PRINT(" [DEBUG] " fmt)

#define SAMPLE_DISP_FORMAT_PRINT   printf


#define DMX_ID_0 0
#define TUNER_ID_0 (0)

#define INVALID_TSPID (0x1fff)

#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
/*************************** Structure Definition ****************************/
typedef enum input_sig_type_t {
    MT_INPUT_SIG_TYPE_CAB = 1,
    /**<Cable signal*/ /**<CNcomment:DVB_C信号*/
    MT_INPUT_SIG_TYPE_SAT = 2,
    /**<Satellite signal*/ /**<CNcomment:卫星信号*/
    MT_INPUT_SIG_TYPE_DVB_T = 3,
    /**<Terrestrial signal*/ /**<CNcomment:地面信号*/
    MT_INPUT_SIG_TYPE_FILE = 4,
    /**<local file */ /**<CNcomment:本地文件*/
}MT_INPUR_SIG_TYPE_T;

typedef struct
{
    MT_U32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    MT_U32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
} mt_input_cab_para_t;

typedef struct
{
    MT_U32 freq; /* frequency kHz */
    MT_U32 sym_rate;
    MT_U8 port_type;     //!<differ DVBS/DVBS2/AUTO from eatchother
    MT_U8 onoff_22k;                     //!< 22K on/off
    MT_U8 polarization;                  //!< Polarization
} mt_input_sat_para_t;

typedef struct
{
    MT_U32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    MT_U32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
    MT_U8 port_type;
} mt_input_ter_para_t;


typedef struct tagInput_Param_T
{
    MT_U8 file_name[256];
}mt_input_file_para_t;

typedef struct
{
    MT_INPUR_SIG_TYPE_T sig_type;
    union
    {
        mt_input_cab_para_t cab;
        mt_input_ter_para_t ter;
        mt_input_sat_para_t sat;
        mt_input_file_para_t file;
    } input_param;

} mt_input_para_t;
typedef struct
{
    MT_U8 file_name[256];
}source_file_param_t;

typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
    pthread_t          stInjectTSThread;
    PMT_COMPACT_TBL    *pProgTbl;
    mt_input_para_t    sInputParam;
} MT_RATIO_RUN_INFO;

void MT_DispFmtSetVedioResolution(disp_sys_t vid_format);
MT_UNF_ENC_FMT_E MT_DispFmtVID2UNF(disp_sys_t old);
#ifdef MT_SAMPLE_APP
MT_S32 MT_DispFmtMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_RATIO_RUN_INFO format_run_info;
static u8 last_vid_format = VID_SYS_MAX;
static mt_bool g_openAutoFmt = MT_FALSE;

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif


#ifdef CONFIG_MT_CHIP_SYMPHONY4
static MT_UNF_ENC_FMT_E g_hdmi_fmt[] = {MT_UNF_ENC_FMT_1080P_60, MT_UNF_ENC_FMT_1080P_50,
                                        MT_UNF_ENC_FMT_1080P_30,MT_UNF_ENC_FMT_1080P_25,
                                        MT_UNF_ENC_FMT_1080P_24,
                                        MT_UNF_ENC_FMT_1080i_60, MT_UNF_ENC_FMT_1080i_50,
                                        MT_UNF_ENC_FMT_720P_60, MT_UNF_ENC_FMT_720P_50,
                                        MT_UNF_ENC_FMT_576P_50,
                                        MT_UNF_ENC_FMT_480P_60,
                                        MT_UNF_ENC_FMT_PAL,
                                        MT_UNF_ENC_FMT_NTSC,
                                        MT_UNF_ENC_FMT_PAL_N,
                                        MT_UNF_ENC_FMT_NTSC_PAL_M};
#elif defined CONFIG_MT_CHIP_SYMPHONY6
static MT_UNF_ENC_FMT_E g_hdmi_fmt[] = {MT_UNF_ENC_FMT_1080P_60, MT_UNF_ENC_FMT_1080P_50,
                                        MT_UNF_ENC_FMT_1080P_30,MT_UNF_ENC_FMT_1080P_25,
                                        MT_UNF_ENC_FMT_1080P_24,
                                        MT_UNF_ENC_FMT_1080i_60, MT_UNF_ENC_FMT_1080i_50,
                                        MT_UNF_ENC_FMT_720P_60, MT_UNF_ENC_FMT_720P_50,
                                        MT_UNF_ENC_FMT_576P_50,
                                        MT_UNF_ENC_FMT_480P_60,
                                        MT_UNF_ENC_FMT_PAL,
                                        MT_UNF_ENC_FMT_NTSC,
                                        MT_UNF_ENC_FMT_PAL_N,
                                        MT_UNF_ENC_FMT_NTSC_PAL_M,
                                        MT_UNF_ENC_FMT_3840X2160_24,
                                        MT_UNF_ENC_FMT_3840X2160_25,
                                        MT_UNF_ENC_FMT_3840X2160_30,
                                        MT_UNF_ENC_FMT_3840X2160_50,
                                        MT_UNF_ENC_FMT_3840X2160_60,
                                        MT_UNF_ENC_FMT_4096X2160_24,
                                        MT_UNF_ENC_FMT_4096X2160_25,
                                        MT_UNF_ENC_FMT_4096X2160_30,
                                        MT_UNF_ENC_FMT_4096X2160_50,
                                        MT_UNF_ENC_FMT_4096X2160_60,
                                        
                                        MT_UNF_ENC_FMT_1080i_59_94,
                                        MT_UNF_ENC_FMT_1080P_59_94,
                                        MT_UNF_ENC_FMT_1080P_29_97,
                                        MT_UNF_ENC_FMT_720P_59_94,
                                        MT_UNF_ENC_FMT_3840X2160_59_94,
                                        MT_UNF_ENC_FMT_3840X2160_29_97,
                                        MT_UNF_ENC_FMT_4096X2160_59_94,
                                        MT_UNF_ENC_FMT_4096X2160_29_97,
};
#endif



/******************************* API declaration *****************************/

#ifndef MT_SAMPLE_APP
static MT_S32 MT_DispFmtCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("The symbol rate is not in range\n");
        return MT_FAILURE;
    }

    switch(p_cab_in->mod_type)
    {
        case 16:
            break;
        case 32:
            break;
        case 64:
            break;
        case 128:
            break;
        case 256:
            break;
        default:
            SAMPLE_DISP_FORMAT_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static MT_S32 MT_DispFmtCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_DISP_FORMAT_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}



/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_DispFmtDmxInit(mt_input_para_t *pInputParam)
{
    MT_S32 s32Ret = MT_SUCCESS;
    mt_sys_version_s stSysChipInfo;

    SAMPLE_DISP_FORMAT_FUNCTION_ENTER();

    s32Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        (MT_VOID) MT_UNF_DMX_DeInit();

        return MT_FAILURE;
    }


    if(MT_INPUT_SIG_TYPE_FILE == pInputParam->sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            (MT_VOID) MT_UNF_DMX_DeInit();
            return MT_FAILURE;
        }
    }
    else
    {
         memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
        s32Ret = mt_sys_get_version(&stSysChipInfo);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to mt_sys_get_version\n");
            return MT_FAILURE;
        }

        if(MT_INPUT_SIG_TYPE_CAB == pInputParam->sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            }
            else
            {
                MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_0);
            }
        }
        else if(MT_INPUT_SIG_TYPE_SAT == pInputParam->sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
            }
            else
            {
                MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            }
        }
    }

    SAMPLE_DISP_FORMAT_FUNCTION_EXIT();


    return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_DispFmtDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
    (MT_VOID)MT_UNF_DMX_DeInit();
    return MT_SUCCESS;
}
#endif

static disp_sys_t MT_DispFmtGetFormat(MT_UNF_VIDEO_FRAME_INFO_S *p_info)
{
    disp_sys_t fps_fmt = VID_SYS_NTSC_M;
    disp_sys_t res_fmt = VID_SYS_MAX;
 #if 1
    mt_u32 fieldRate = 0;
    if(p_info == NULL)
    {
        return VID_SYS_MAX;
    }
    fieldRate = p_info->stFrameRate.u32fpsInteger;

    if((fieldRate >= 11) && (fieldRate <= 13)) /**12.5**/
    {
        fps_fmt = VID_SYS_PAL;
    }
    else if((fieldRate >= 14) && (fieldRate <= 16)) /**15**/
    {
        fps_fmt = VID_SYS_NTSC_M;
    }
    else if((fieldRate >= 23) && (fieldRate <= 27)) /**25**/
    {
        fps_fmt = VID_SYS_PAL;
    }
    else if((fieldRate >= 28) && (fieldRate <= 32)) /**30**/
    {
        fps_fmt = VID_SYS_NTSC_M;
    }
    else if((fieldRate >= 48) && (fieldRate <= 52)) /**50**/
    {
        fps_fmt = VID_SYS_PAL;
    }
    else if((fieldRate >= 58) && (fieldRate <= 62)) /**60**/
    {
        fps_fmt = VID_SYS_NTSC_M;
    }
    else if((fieldRate >= 98) && (fieldRate <= 102)) /**100**/
    {
        fps_fmt = VID_SYS_PAL;
    }
    else if((fieldRate >= 118) && (fieldRate <= 122)) /**120**/
    {
        fps_fmt = VID_SYS_NTSC_M;
    }
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    if(p_info->u32Width >= 4096)
    {
        if((fieldRate >= 23) && (fieldRate < 25)) /**24**/
        {
            res_fmt = VID_SYS_4096X2160_24HZ;
        }
        else if((fieldRate >= 25) && (fieldRate <= 27)) /**25**/
        {
            res_fmt = VID_SYS_4096X2160_25HZ;
        }
        else if((fieldRate >= 28) && (fieldRate <= 32)) /**30**/
        {

            res_fmt = VID_SYS_4096X2160_30HZ;
        }
        else if((fieldRate >= 48) && (fieldRate <= 52)) /**50**/
        {

            res_fmt = VID_SYS_4096X2160_50HZ;
        }
        else if((fieldRate >= 58) && (fieldRate <= 62)) /**60**/
        {
            res_fmt = VID_SYS_4096X2160_60HZ;
        }

    }

    if(p_info->u32Width >= 3840 && p_info->u32Width < 4096)
    {
        if((fieldRate >= 23) && (fieldRate < 25)) /**24**/
        {
            res_fmt = VID_SYS_3840X2160_24HZ;
        }
        else if((fieldRate >= 25) && (fieldRate <= 27)) /**25**/
        {
            res_fmt = VID_SYS_3840X2160_25HZ;
        }
        else if((fieldRate >= 28) && (fieldRate <= 32)) /**30**/
        {
            res_fmt = VID_SYS_3840X2160_30HZ;
        }
        else if((fieldRate >= 48) && (fieldRate <= 52)) /**50**/
        {
            res_fmt = VID_SYS_3840X2160_50HZ;
        }
        else if((fieldRate >= 58) && (fieldRate <= 62)) /**60**/
        {
            res_fmt = VID_SYS_3840X2160_60HZ;
        }

    }
#endif
    if(p_info->u32Width >= 1440 && p_info->u32Width < 3840)
    {
        res_fmt = VID_SYS_1080P;
        if(fps_fmt == VID_SYS_PAL)
        {
            if(p_info->bProgressive)
            {
                res_fmt = VID_SYS_1080P_50HZ;
            }
            else
            {
                res_fmt = VID_SYS_1080I_50HZ;
            }
        }
        else
        {
            if(p_info->bProgressive)
            {
                res_fmt = VID_SYS_1080P;
            }
            else
            {
                res_fmt = VID_SYS_1080I;
            }
        }
    }

    if(p_info->u32Width >= 960 && p_info->u32Width < 1440)
    {
        res_fmt = VID_SYS_720P;
        if(fps_fmt == VID_SYS_PAL)
        {
            res_fmt = VID_SYS_720P_50HZ;
        }
    }

    if(p_info->u32Width < 960)
    {
        res_fmt = VID_SYS_480P;
        if(p_info->u32Height > 528)
        {
            res_fmt = VID_SYS_576P_50HZ;
        }

        if((res_fmt == VID_SYS_480P)
            && (fps_fmt == VID_SYS_NTSC_M))
        {
            if (!p_info->bProgressive)
            {
                res_fmt = VID_SYS_NTSC_443;
            }
        }

        if((res_fmt == VID_SYS_576P_50HZ)
            && (fps_fmt == VID_SYS_PAL))
        {
            if (!p_info->bProgressive)
            {
                res_fmt = VID_SYS_PAL;
            }
        }

    }
#else
    res_fmt = p_info->vid_format
#endif

    return res_fmt;
}

static mt_s32 MT_DispFmtSetFormat(MT_UNF_ENC_FMT_E enFormat)
{
    mt_s32                      Ret = 0;

    /* set display1 format*/
    Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, enFormat);
    if (Ret != MT_SUCCESS)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("call MT_UNF_DISP_Attach failed, Ret=%#x.\n", Ret);
        return Ret;
    }

    if ((MT_UNF_ENC_FMT_1080P_60 == enFormat)
        ||(MT_UNF_ENC_FMT_1080i_60 == enFormat)
        ||(MT_UNF_ENC_FMT_1080P_30 == enFormat)
        ||(MT_UNF_ENC_FMT_1080P_24 == enFormat)
        ||(MT_UNF_ENC_FMT_720P_60 == enFormat)
        ||(MT_UNF_ENC_FMT_480P_60 == enFormat)
        ||(MT_UNF_ENC_FMT_NTSC == enFormat)
        ||(MT_UNF_ENC_FMT_4096X2160_24 == enFormat)
        ||(MT_UNF_ENC_FMT_3840X2160_30 == enFormat)
        ||(MT_UNF_ENC_FMT_3840X2160_24 == enFormat))
    {
        Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_NTSC);
        if (MT_SUCCESS != Ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", Ret);
            return Ret;
        }
    }

    if ((MT_UNF_ENC_FMT_1080P_50 == enFormat)
        ||(MT_UNF_ENC_FMT_1080i_50 == enFormat)
        ||(MT_UNF_ENC_FMT_1080P_25 == enFormat)
        ||(MT_UNF_ENC_FMT_720P_50 == enFormat)
        ||(MT_UNF_ENC_FMT_576P_50 == enFormat)
        ||(MT_UNF_ENC_FMT_PAL == enFormat)
        ||(MT_UNF_ENC_FMT_3840X2160_25 == enFormat))
    {
        Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_PAL);
        if (MT_SUCCESS != Ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", Ret);
            return Ret;
        }
    }

    return MT_SUCCESS;
}

static void MT_DispFmtCheckFrameInfo(void *p_frame)
{
    MT_UNF_VIDEO_FRAME_INFO_S *p_info = (MT_UNF_VIDEO_FRAME_INFO_S *)p_frame;
    disp_sys_t vid_format;//disp_sys_t

    vid_format = MT_DispFmtGetFormat(p_info);

    if(last_vid_format != vid_format)
    {
        SAMPLE_DISP_FORMAT_INFO_PRINT("fieldRate: %d , bProgressive[%d]\n", p_info->stFrameRate.u32fpsInteger, p_info->bProgressive);
        SAMPLE_DISP_FORMAT_INFO_PRINT(" source: %d x %d  resolution: %d x %d\n", p_info->u32Width,  p_info->u32Height, p_info->u32DisplayWidth, p_info->u32DisplayHeight);

        SAMPLE_DISP_FORMAT_INFO_PRINT("New vid_format: %d \n", vid_format);
        MT_DispFmtSetVedioResolution(vid_format);

        last_vid_format = vid_format;
    }
}


MT_UNF_ENC_FMT_E MT_DispFmtVID2UNF(disp_sys_t old)
{
    MT_UNF_ENC_FMT_E new_format = MT_UNF_ENC_FMT_1080i_50;

    switch (old)
    {
        case VID_SYS_NTSC_J:
            new_format = MT_UNF_ENC_FMT_NTSC_J;
            break;
        case VID_SYS_NTSC_M:
            new_format = MT_UNF_ENC_FMT_NTSC_PAL_M;
            break;
        case VID_SYS_NTSC_443:
            new_format = MT_UNF_ENC_FMT_NTSC;
            break;
        case VID_SYS_PAL:
            new_format = MT_UNF_ENC_FMT_PAL;
            break;
        case VID_SYS_PAL_N:
        case VID_SYS_PAL_M:
            new_format = MT_UNF_ENC_FMT_PAL_N;
            break;
        case VID_SYS_PAL_NC:
            new_format = MT_UNF_ENC_FMT_PAL_Nc;
            break;

        case VID_SYS_SECAM:
            new_format = MT_UNF_ENC_FMT_SECAM_SIN;
            break;
        case VID_SYS_1080I:
            new_format = MT_UNF_ENC_FMT_1080i_60;
            break;
        case VID_SYS_1080I_50HZ:
            new_format = MT_UNF_ENC_FMT_1080i_50;
            break;
        case VID_SYS_1080P:
            new_format = MT_UNF_ENC_FMT_1080P_60;
            break;
        case VID_SYS_1080P_30HZ:
            new_format = MT_UNF_ENC_FMT_1080P_30;
            break;
        case VID_SYS_1080P_24HZ:
            new_format = MT_UNF_ENC_FMT_1080P_24;
            break;
        case VID_SYS_1080P_25HZ:
            new_format = MT_UNF_ENC_FMT_1080P_25;
            break;
        case VID_SYS_1080P_50HZ:
            new_format = MT_UNF_ENC_FMT_1080P_50;
            break;

        case VID_SYS_720P:
        case VID_SYS_720P_30HZ:
            new_format = MT_UNF_ENC_FMT_720P_60;
            break;
        case VID_SYS_720P_24HZ:
        case VID_SYS_720P_25HZ:
        case VID_SYS_720P_50HZ:
            new_format = MT_UNF_ENC_FMT_720P_50;
            break;
        case VID_SYS_576P_50HZ:
            new_format = MT_UNF_ENC_FMT_576P_50;
            break;
        case VID_SYS_480P:
            new_format = MT_UNF_ENC_FMT_480P_60;
            break;
#ifdef CONFIG_MT_CHIP_SYMPHONY6
        case VID_SYS_3840X2160_24HZ:
            new_format = MT_UNF_ENC_FMT_3840X2160_24;
            break;
        case VID_SYS_3840X2160_25HZ:
            new_format = MT_UNF_ENC_FMT_3840X2160_25;
            break;
        case VID_SYS_3840X2160_30HZ:
            new_format = MT_UNF_ENC_FMT_3840X2160_30;
            break;
        case VID_SYS_3840X2160_50HZ:
            new_format = MT_UNF_ENC_FMT_3840X2160_50;
            break;
        case VID_SYS_3840X2160_60HZ:
            new_format = MT_UNF_ENC_FMT_3840X2160_60;
            break;
        case VID_SYS_4096X2160_24HZ:
            new_format = MT_UNF_ENC_FMT_4096X2160_24;
            break;
        case VID_SYS_4096X2160_25HZ:
            new_format = MT_UNF_ENC_FMT_4096X2160_25;
            break;
        case VID_SYS_4096X2160_30HZ:
            new_format = MT_UNF_ENC_FMT_4096X2160_30;
            break;
        case VID_SYS_4096X2160_50HZ:
            new_format = MT_UNF_ENC_FMT_4096X2160_50;
            break;
        case VID_SYS_4096X2160_60HZ:
            new_format = MT_UNF_ENC_FMT_4096X2160_60;
            break;
#endif
        case VID_SYS_AUTO:
        case VID_SYS_240P_60HZ:
        case VID_SYS_288P_50HZ:
        case VID_SYS_1250I_50HZ:
        default:
            break;
      }

    return new_format;
}

void MT_DispFmtSetVedioResolution(disp_sys_t vid_format)
{
    MT_UNF_ENC_FMT_E new_hd_res = MT_UNF_ENC_FMT_1080i_50;

    new_hd_res = MT_DispFmtVID2UNF(vid_format);

    SAMPLE_DISP_FORMAT_INFO_PRINT("resolution new_hd_res[%d] vid_format[%d]. \n", new_hd_res, vid_format);

    MT_DispFmtSetFormat(new_hd_res);
}

static void MT_DispFmtSetAutoFmtFlag(mt_bool autoFmtFlag)
{
    g_openAutoFmt = autoFmtFlag;
}

static void MT_DispFmtGetAutoFmtFlag(mt_bool *autoFmtFlag)
{
     *autoFmtFlag = g_openAutoFmt;
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
static MT_S32 MT_DispFmtAvplayEventFunction(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent, mt_u32 u32Para)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
static MT_S32 MT_DispFmtAvplayEventFunction(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent, ulong u32Para)
#endif
{

    if (MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("=====phAvplay == NULL=====\n");
        return MT_FAILURE;
    }

    //SAMPLE_DISP_FORMAT_INFO_PRINT("recv event: %d \n", enEvent);
    switch(enEvent)
    {
      case MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME:
        case MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME:
        {
            MT_UNF_VIDEO_FRAME_INFO_S *p_VdecUnfFrm = (MT_UNF_VIDEO_FRAME_INFO_S *)u32Para;
            mt_bool autoFmtFlag;

            MT_DispFmtGetAutoFmtFlag(&autoFmtFlag);
            if (autoFmtFlag != MT_FALSE)
            {
                MT_DispFmtCheckFrameInfo((void *)p_VdecUnfFrm);
            }
        }
        break;
        case MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT:
            //avctl_vdec_set_unsupport_flag(TRUE);
            SAMPLE_DISP_FORMAT_INFO_PRINT("Recv MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT env. \n");
        break;

        default:
        break;
    }

    return MT_SUCCESS;
}



/*
 @brief Audio and video playback init
 @param[out] phSoundTrack,Pointer to the outgoing SoundTrack handle
 @param[out] hWin, Pointer to the outgoing Window handle
 @param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
#ifndef MT_SAMPLE_APP
static MT_S32 MT_DispFmtAVplayInit(MT_HANDLE *p_hAvplay, MT_HANDLE *P_hWin, MT_HANDLE *p_hSoundTrack)
{
    MT_S32      ret = MT_SUCCESS;
    MT_HANDLE   hAvplay = MT_INVALID_HANDLE;
    MT_HANDLE   hWin = MT_INVALID_HANDLE;
    MT_HANDLE   hsoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *p_hAvplay = hAvplay;
    *P_hWin = hWin;
    *p_hSoundTrack = hsoundTrack;

    return MT_SUCCESS;


ERR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);
ERR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);
ERR6:
    (MT_VOID)MT_UNF_SND_Detach(hsoundTrack, hAvplay);
ERR5:
    (MT_VOID)MT_UNF_SND_DestroyTrack(hsoundTrack);
ERR4:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
ERR3:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
ERR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);
ERR1:
    (MT_VOID)MT_UNF_AVPLAY_DeInit();

    return MT_FAILURE;

}


/*
 @brief Audio and video playback Deinit
 @param[in] hWin, A pointer to the Window handle passed in
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_VOID  MT_DispFmtAvplayDeInit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{


    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("phAvplay is null.\n");

    }

    if(MT_INVALID_HANDLE == hWin)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("phWin is null.\n");

    }

    if(MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("phSoundTrack is null.\n");

    }


    (MT_VOID)MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);
    (MT_VOID)MT_UNF_SND_Detach(hSoundTrack, hAvplay);
    (MT_VOID)MT_UNF_SND_DestroyTrack(hSoundTrack);
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);
    (MT_VOID)MT_UNF_AVPLAY_DeInit();
}

/*
@brief Audio and video decoding , synchronous
@param[in] phAvplay,A pointer to the Avplay handle passed in
@param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_S32 MT_DispFmtAVPlay_Start(MT_HANDLE hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
{
    MT_U32 VidPid = 0;
    MT_U32 AudPid = 0;
    MT_U32 u32AudType = 0;
    MT_S32 ret = 0;
    MT_UNF_VCODEC_TYPE_E enVidType = { 0 };
    MT_UNF_VCODEC_ATTR_S VcodecAttr = { 0 };
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0X00;

    if (MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("=====phAvplay == NULL=====\n");
        return MT_FAILURE;
    }

    if(NULL == p_ProgInfo)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("p_ProgInfo is NULL!\n");
        return MT_FAILURE;
    }

    if(p_ProgInfo->VElementNum > 0 )
    {
        VidPid = p_ProgInfo->VElementPid;
        enVidType = p_ProgInfo->VideoType;
    }
    else
    {
        VidPid = INVALID_TSPID;
        enVidType = MT_UNF_VCODEC_TYPE_BUTT;
    }

    if(p_ProgInfo->AElementNum > 0)
    {
        AudPid  = p_ProgInfo->AElementPid;
        u32AudType = p_ProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    SAMPLE_DISP_FORMAT_INFO_PRINT("%s ====%d  vidpid = %x AudPid=%x \n",__FILE__,__LINE__, VidPid, AudPid);

    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
            return ret;
        }
        /* The type of code stream supported by the decoder*/
        VcodecAttr.enType = enVidType;
        VcodecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VcodecAttr.u32ErrCover = 100;
        VcodecAttr.u32Priority = 3;
        VcodecAttr.u32UseDescInfoFlag = 1;

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_DISP_FORMAT_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_DISP_FORMAT_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


    }


    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
        return ret;
    }


    //dmx sync
    if((INVALID_TSPID != VidPid) || (INVALID_TSPID != AudPid))
    {
        MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync = { 0 };
        memset(&DmxAvsync, 0, sizeof(DmxAvsync));
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay,MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC,(MT_VOID *)&DmxAvsync);
        if(ret != MT_SUCCESS)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
            return ret;
        }
    }


    //av sync
    if((INVALID_TSPID != VidPid) || (INVALID_TSPID != AudPid))
    {
        MT_UNF_SYNC_ATTR_S   SyncAttr = { 0 };
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
            return ret;
        }
        SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        SyncAttr.stSyncStartRegion.s32VidPlusTime = 20;
        SyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        SyncAttr.bQuickOutput = MT_TRUE;
        SyncAttr.stSyncStartRegion.bSmoothPlay  = MT_TRUE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
            return ret;
        }
    }

    return MT_SUCCESS;
}
#endif

/*
@brief Read path file contents into g_hTsBuffer
@param[in] args, Structure of file
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
#ifndef MT_SAMPLE_APP
static MT_S32 MT_DispFmtInjectTsTask(MT_VOID *args)
{
    MT_S32  ret = MT_SUCCESS;
    MT_U32  Readlen = 0;
    MT_HANDLE hTsBuffer = MT_INVALID_HANDLE;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    source_file_param_t *pstParam = (source_file_param_t *)(args);

    SAMPLE_DISP_FORMAT_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->file_name);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT( "file %s open error!!\n", pstParam->file_name);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        g_bTaskQuit = MT_TRUE;
        SAMPLE_DISP_FORMAT_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        return MT_FAILURE;
    }

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*1000, &StreamBuf, 1000);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT( "failed to MT_UNF_DMX_GetTSBuffer  ret= %x \n", ret);
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_DISP_FORMAT_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_DISP_FORMAT_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }

    MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);

    return MT_SUCCESS;
}
#endif

static MT_U8 * MT_DispFmt2Str(MT_UNF_ENC_FMT_E format)
{
    switch(format)
    {
        case MT_UNF_ENC_FMT_1080P_60:
            return "FMT_1080P_60";
        case MT_UNF_ENC_FMT_1080P_59_94:
            return "FMT_1080P_59_94";
        case MT_UNF_ENC_FMT_1080P_50:
            return "FMT_1080P_50";
        case MT_UNF_ENC_FMT_1080P_30:
            return "FMT_1080P_30";
        case MT_UNF_ENC_FMT_1080P_29_97:
            return "FMT_1080P_29_97";
        case MT_UNF_ENC_FMT_1080P_25:
            return "FMT_1080P_25";
        case MT_UNF_ENC_FMT_1080P_24:
            return "FMT_1080p_24";
        case MT_UNF_ENC_FMT_1080i_60:
            return "FMT_1080i_60";
        case MT_UNF_ENC_FMT_1080i_59_94:
            return "FMT_1080i_59_94";
        case MT_UNF_ENC_FMT_1080i_50:
            return "FMT_1080i_50";
        case MT_UNF_ENC_FMT_720P_60:
            return "FMT_720P_60";
        case MT_UNF_ENC_FMT_720P_59_94:
            return "FMT_720P_59_94";
        case MT_UNF_ENC_FMT_720P_50:
            return "FMT_720P_50";
        case MT_UNF_ENC_FMT_576P_50:
            return "FMT_576P_50";
        case MT_UNF_ENC_FMT_480P_60:
            return "FMT_480P_60";
        case MT_UNF_ENC_FMT_PAL:
            return "FMT_PAL(FMT_576i_50)";
        case MT_UNF_ENC_FMT_NTSC:
            return "FMT_NTSC(FMT_480i_60)";
        case MT_UNF_ENC_FMT_PAL_N:
            return "FMT_576i_50";
        case MT_UNF_ENC_FMT_NTSC_PAL_M:
            return "FMT_480i_60";
#ifdef CONFIG_MT_CHIP_SYMPHONY6
        case MT_UNF_ENC_FMT_3840X2160_24:
            return "FMT_3840X2160_24";
        case MT_UNF_ENC_FMT_3840X2160_25:
            return "FMT_3840X2160_25";
        case MT_UNF_ENC_FMT_3840X2160_29_97:
            return "FMT_3840X2160_29_97";
        case MT_UNF_ENC_FMT_3840X2160_30:
            return "FMT_3840X2160_30";
        case MT_UNF_ENC_FMT_3840X2160_50:
            return "FMT_3840X2160_50";
        case MT_UNF_ENC_FMT_3840X2160_59_94:
            return "FMT_3840X2160_59_94";
        case MT_UNF_ENC_FMT_3840X2160_60:
            return "FMT_3840X2160_60";
        case MT_UNF_ENC_FMT_4096X2160_24:
            return "FMT_4096X2160_24";
        case MT_UNF_ENC_FMT_4096X2160_25:
            return "FMT_4096X2160_25";
        case MT_UNF_ENC_FMT_4096X2160_29_97:
            return "FMT_4096X2160_29_97";
        case MT_UNF_ENC_FMT_4096X2160_30:
            return "FMT_4096X2160_30";
        case MT_UNF_ENC_FMT_4096X2160_50:
            return "FMT_4096X2160_50";
        case MT_UNF_ENC_FMT_4096X2160_59_94:
            return "FMT_4096X2160_59_94";
        case MT_UNF_ENC_FMT_4096X2160_60:
            return "FMT_4096X2160_60";
#endif

        default:
            return "No Suport!";

    }
}



/*
@brief stop to play
@param[in] avplay, Player handle
@return void
*/
#ifndef MT_SAMPLE_APP
static MT_VOID MT_DispFmtStopplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };

    if (MT_INVALID_HANDLE == avplay)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("=====phAvplay == NULL=====\n");
    }

    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    (MT_VOID)MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
}
#endif

static MT_VOID MT_DispFmtExit(void)
{
    g_bTaskQuit = MT_TRUE;
#ifndef MT_SAMPLE_APP
    /** Stop playing the show */
    (MT_VOID)MT_DispFmtStopplay(format_run_info.hAvPlay);

    /** Audio and video player deinitialization */
    (MT_VOID)MT_DispFmtAvplayDeInit(format_run_info.hAvPlay, format_run_info.hWin, format_run_info.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(format_run_info.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE == format_run_info.sInputParam.sig_type)
    {
        /** Wait for the thread to end */
        pthread_join(format_run_info.stInjectTSThread, NULL);
    }

    (MT_VOID)MT_DispFmtDmxDeInit();

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE != format_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
#endif
    memset(&format_run_info, 0xff, sizeof(format_run_info));
}

#ifndef MT_SAMPLE_APP
static MT_VOID MT_DispFmtPrintDvbcHelp(MT_CHAR *name)
{
    SAMPLE_DISP_FORMAT_PRINT("\nDvbc Usage:\n");
    SAMPLE_DISP_FORMAT_PRINT("example:\n");
    SAMPLE_DISP_FORMAT_PRINT("    %s -c 314 6875 64\n", name);
}

static MT_VOID MT_DispFmtPrintDvbsHelp(MT_CHAR *name)
{
    SAMPLE_DISP_FORMAT_PRINT("\nDvbs Usage:\n");
    SAMPLE_DISP_FORMAT_PRINT("example:\n");
    SAMPLE_DISP_FORMAT_PRINT("    %s -s 3840 27500 1 0 0\n",name);
}

static MT_VOID MT_DispFmtPrintFileHelp(MT_CHAR *name)
{
    SAMPLE_DISP_FORMAT_PRINT("\nFile Usage:\n");
    SAMPLE_DISP_FORMAT_PRINT("example:\n");
    SAMPLE_DISP_FORMAT_PRINT("    %s -f ./ttx.ts\n",name);
}
#endif

static MT_VOID MT_DispFmtPrint_help(MT_CHAR *name)
{
#ifndef MT_SAMPLE_APP
    SAMPLE_DISP_FORMAT_PRINT("Lack of parameters\n");
    SAMPLE_DISP_FORMAT_PRINT("\nUsage:\n");
    SAMPLE_DISP_FORMAT_PRINT("%s\n", name);
    SAMPLE_DISP_FORMAT_PRINT("    -f: path of the stream file\n");
    SAMPLE_DISP_FORMAT_PRINT("    -c: DVBC locks frequency\n");
    SAMPLE_DISP_FORMAT_PRINT("    -s: DVBS locks frequency\n");
#else
    SAMPLE_DISP_FORMAT_PRINT("    -q: Exit the background\n");
#endif
#ifndef MT_SAMPLE_APP
    SAMPLE_DISP_FORMAT_PRINT("example:\n");
    SAMPLE_DISP_FORMAT_PRINT("    %s -f ./ttx.ts\n",name);
    SAMPLE_DISP_FORMAT_PRINT("    %s -c 314 6875 64\n",name);
    SAMPLE_DISP_FORMAT_PRINT("    %s -s 3840 27500 1 0 0\n",name);
#endif
}

static MT_VOID MT_DispFmtMenuHelp(MT_U32 prog_num)
{
    SAMPLE_DISP_FORMAT_PRINT("\n");
#ifndef MT_SAMPLE_APP
    SAMPLE_DISP_FORMAT_PRINT(" 1 - %d : select the program \n", prog_num);
#endif
    SAMPLE_DISP_FORMAT_PRINT("     f : Change HDMI format \n");
    SAMPLE_DISP_FORMAT_PRINT("     d : Change 3DMode \n");
    SAMPLE_DISP_FORMAT_PRINT("     l : get current HDMI format \n");
    SAMPLE_DISP_FORMAT_PRINT("     a : Auto HDMI format \n");
    SAMPLE_DISP_FORMAT_PRINT("     o : Optimal resolution output \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_DISP_FORMAT_PRINT("     b : background run \n");
#endif
    SAMPLE_DISP_FORMAT_PRINT("     h : help \n");
    SAMPLE_DISP_FORMAT_PRINT("     q : quit \n");
    SAMPLE_DISP_FORMAT_PRINT("=============================\n");
    SAMPLE_DISP_FORMAT_PRINT("Disp_fmt>> ");
}

static MT_VOID MT_DispFmtFormatHelp(void)
{
    SAMPLE_DISP_FORMAT_PRINT("  ===Change HDMI Format===\n");
    SAMPLE_DISP_FORMAT_PRINT("\t0:  1080P_60\n");
    SAMPLE_DISP_FORMAT_PRINT("\t1:  1080P_50\n");
    SAMPLE_DISP_FORMAT_PRINT("\t2:  1080P_30\n");
    SAMPLE_DISP_FORMAT_PRINT("\t3:  1080P_25\n");
    SAMPLE_DISP_FORMAT_PRINT("\t4:  1080P_24\n");
    SAMPLE_DISP_FORMAT_PRINT("\t5:  1080i_60\n");
    SAMPLE_DISP_FORMAT_PRINT("\t6:  1080i_50\n");
    SAMPLE_DISP_FORMAT_PRINT("\t7:  720P_60\n");
    SAMPLE_DISP_FORMAT_PRINT("\t8:  720P_50\n");
    SAMPLE_DISP_FORMAT_PRINT("\t9:  576P_50\n");
    SAMPLE_DISP_FORMAT_PRINT("\t10:  480P_60\n");
    SAMPLE_DISP_FORMAT_PRINT("\t11:  PAL(576i_50)\n");
    SAMPLE_DISP_FORMAT_PRINT("\t12:  NTSC(480i_60)\n");
    SAMPLE_DISP_FORMAT_PRINT("\t13: PAL_N(576i_50)\n");
    SAMPLE_DISP_FORMAT_PRINT("\t14: PAL_M(480i_60)\n");
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    SAMPLE_DISP_FORMAT_PRINT("\t15: 3840X2160_24\n");
    SAMPLE_DISP_FORMAT_PRINT("\t16: 3840X2160_25\n");
    SAMPLE_DISP_FORMAT_PRINT("\t17: 3840X2160_30\n");
    SAMPLE_DISP_FORMAT_PRINT("\t18: 3840X2160_50\n");
    SAMPLE_DISP_FORMAT_PRINT("\t19: 3840X2160_60\n");
    SAMPLE_DISP_FORMAT_PRINT("\t20: 4096X2160_24\n");
    SAMPLE_DISP_FORMAT_PRINT("\t21: 4096X2160_25\n");
    SAMPLE_DISP_FORMAT_PRINT("\t22: 4096X2160_30\n");
    SAMPLE_DISP_FORMAT_PRINT("\t23: 4096X2160_50\n");
    SAMPLE_DISP_FORMAT_PRINT("\t24: 4096X2160_60\n");

    SAMPLE_DISP_FORMAT_PRINT("\t25:  1080i_59.94\n");
    SAMPLE_DISP_FORMAT_PRINT("\t26:  1080P_59.94\n");
    SAMPLE_DISP_FORMAT_PRINT("\t27:  1080P_29.97\n");
    SAMPLE_DISP_FORMAT_PRINT("\t28:  720P_59.94\n");
    SAMPLE_DISP_FORMAT_PRINT("\t29:  3840X2160_59.94\n");
    SAMPLE_DISP_FORMAT_PRINT("\t30:  3840X2160_29.97\n");   
    SAMPLE_DISP_FORMAT_PRINT("\t31:  4096X2160_59.94\n");
    SAMPLE_DISP_FORMAT_PRINT("\t32:  4096X2160_29.97\n");
   
#endif
    SAMPLE_DISP_FORMAT_PRINT("=============================\n");
    SAMPLE_DISP_FORMAT_PRINT("Set format>> ");
}

static MT_VOID MT_DispFmt3DModeHelp(void)
{
    SAMPLE_DISP_FORMAT_PRINT("  ===Change 3DMode===\n");
    SAMPLE_DISP_FORMAT_PRINT("\t0:  3D_NONE\n");
    SAMPLE_DISP_FORMAT_PRINT("\t1:  3D_FRAME_PACKING\n");
    SAMPLE_DISP_FORMAT_PRINT("\t2:  3D_SIDE_BY_SIDE_HALF\n");
    SAMPLE_DISP_FORMAT_PRINT("\t3:  3D_TOP_AND_BOTTOM\n");
    SAMPLE_DISP_FORMAT_PRINT("\t4:  3D_FIELD_ALTERNATIVE\n");
    SAMPLE_DISP_FORMAT_PRINT("\t5:  3D_LINE_ALTERNATIVE\n");
    SAMPLE_DISP_FORMAT_PRINT("\t6:  3D_SIDE_BY_SIDE_FULL\n");
    SAMPLE_DISP_FORMAT_PRINT("\t7:  3D_L_DEPTH\n");
    SAMPLE_DISP_FORMAT_PRINT("\t8:  3D_L_DEPTH_GRAPHICS_GRAPHICS_DEPTH\n");
    SAMPLE_DISP_FORMAT_PRINT("=============================\n");
    SAMPLE_DISP_FORMAT_PRINT("Set 3DMode>> ");
}


/*
@brief Toggle modes and exit play
@param[in] hAvPlay,A pointer to the Avplay handle passed in
@param[in] ppProgTable,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_VOID MT_DispFmtCmdTask(MT_HANDLE hAvPlay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_CHAR            *pfgetret = NULL;
    MT_CHAR            inputCmd[32] ={ 0 };
    MT_U32             en3DMode = 0;
    MT_U32             enEncodingFormat = 0;
    MT_U32             input3DMode = 0;
    MT_U32             inputFormat = 0;
    MT_S32             ret = MT_FAILURE;
    MT_U32             index = 0;
#ifndef MT_SAMPLE_APP
    MT_U32             u32ProgNum = 0;
    PMT_COMPACT_PROG   *pstCurrentProgInfo = NULL;
#endif
    MT_UNF_ENC_FMT_E   currFmt = MT_UNF_ENC_FMT_BUTT;
    mt_bool            autoFmtFlag;

#ifndef MT_SAMPLE_APP
    if (MT_INVALID_HANDLE == hAvPlay)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("=====phAvplay == NULL=====\n");
    }

    if(NULL == pProgTbl)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("p_ProgInfo is NULL!\n");
    }
#endif

    while(1)
    {
        MT_DispFmtMenuHelp(pProgTbl->prog_num);

        pfgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        pfgetret = pfgetret;

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_DISP_FORMAT_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_DISP_FORMAT_INFO_PRINT("Disp_fmt play in back!\n");
            break;
        }
#else
        /* Switch between programs*/
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);
            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);
                (MT_VOID)MT_DispFmtStopplay(hAvPlay);

                SAMPLE_DISP_FORMAT_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);
                ret  = MT_DispFmtAVPlay_Start(hAvPlay, pstCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DISP_FORMAT_ERR_PRINT("Switching shows failed\n");
                    continue;
                }
            }
            else
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
            }
        }
#endif
        else if ('f' == inputCmd[0])
        {
            MT_DispFmtFormatHelp();

            MT_DispFmtGetAutoFmtFlag(&autoFmtFlag);
            if (autoFmtFlag != MT_FALSE)
            {
                MT_DispFmtSetAutoFmtFlag(MT_FALSE);
                MT_UNF_AVPLAY_UnRegisterEvent(hAvPlay, MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME);
                MT_UNF_AVPLAY_UnRegisterEvent(hAvPlay, MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME);
                MT_UNF_AVPLAY_UnRegisterEvent(hAvPlay, MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT);
            }

            scanf("%d", &index);
            getchar();
            if(index >= sizeof(g_hdmi_fmt)/sizeof(MT_UNF_ENC_FMT_E))
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("Input [%d] is overflow, Use default format [1080P_60] \n", index);
                index = 0;
            }

            ret = MT_UNF_DISP_GetFormat(MT_UNF_DISPLAY1, &currFmt);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("MT_UNF_DISP_GetFormat failed. ret = 0x%x\n", ret);
                continue;
            }
            SAMPLE_DISP_FORMAT_INFO_PRINT(" Change format from [%s] to [%s] \n", MT_DispFmt2Str(currFmt), MT_DispFmt2Str(g_hdmi_fmt[index]));
            ret = MT_DispFmtSetFormat(g_hdmi_fmt[index]);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("MTADP_Disp_Init failed. ret = 0x%x\n", ret);
                continue;
            }
        }
        else if ('d' == inputCmd[0])
        {
            (MT_VOID)MT_DispFmtFormatHelp();

            MT_DispFmtGetAutoFmtFlag(&autoFmtFlag);
            if (autoFmtFlag != MT_FALSE)
            {
                MT_DispFmtSetAutoFmtFlag(MT_FALSE);
                MT_UNF_AVPLAY_UnRegisterEvent(hAvPlay, MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME);
                MT_UNF_AVPLAY_UnRegisterEvent(hAvPlay, MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME);
                MT_UNF_AVPLAY_UnRegisterEvent(hAvPlay, MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT);
            }

            scanf("%d", &inputFormat);
            getchar();
            if(inputFormat >= sizeof(g_hdmi_fmt)/sizeof(MT_UNF_ENC_FMT_E))
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("Input [%d] is overflow, Use default format [1080P_60] \n", inputFormat);
                inputFormat = 0;
            }

            (MT_VOID)MT_DispFmt3DModeHelp();
            scanf("%d", &input3DMode);
            getchar();

            MT_UNF_DISP_Get3DMode(MT_UNF_DISPLAY1, &en3DMode, &enEncodingFormat);

            if(enEncodingFormat != g_hdmi_fmt[inputFormat] || en3DMode != input3DMode)
            {
                MT_UNF_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                MT_USLEEP(32000);
            }

            MT_UNF_DISP_Set3DMode(MT_UNF_DISPLAY1, input3DMode, g_hdmi_fmt[inputFormat]);

            MT_USLEEP(200000);
            MT_UNF_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_FALSE);
        }
        else if ('l' == inputCmd[0])
        {
            ret = MT_UNF_DISP_GetFormat(MT_UNF_DISPLAY1, &currFmt);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("MT_UNF_DISP_GetFormat failed. ret = 0x%x\n", ret);
                continue;
            }

            SAMPLE_DISP_FORMAT_INFO_PRINT(" current format is [%s] \n", MT_DispFmt2Str(currFmt));
        }
        else if ('a' == inputCmd[0])
        {
            SAMPLE_DISP_FORMAT_INFO_PRINT("Set Auto HDMI Formart \n");
            last_vid_format = VID_SYS_MAX;

            MT_DispFmtGetAutoFmtFlag(&autoFmtFlag);
            if (autoFmtFlag != MT_TRUE)
            {
                MT_DispFmtSetAutoFmtFlag(MT_TRUE);
                MT_UNF_AVPLAY_RegisterEvent(hAvPlay, MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME, MT_DispFmtAvplayEventFunction);
                MT_UNF_AVPLAY_RegisterEvent(hAvPlay, MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME, MT_DispFmtAvplayEventFunction);
                MT_UNF_AVPLAY_RegisterEvent(hAvPlay, MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT, MT_DispFmtAvplayEventFunction);
            }
        }
        else if ('o' == inputCmd[0])
        {
            MT_UNF_EDID_BASE_INFO_S edid_info;
            memset(&edid_info, 0, sizeof(MT_UNF_EDID_BASE_INFO_S));
            ret = MT_UNF_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_0, &edid_info);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("MT_UNF_HDMI_GetSinkCapability failed. ret = 0x%x\n", ret);
                continue;
            }

            ret = MT_UNF_DISP_GetFormat(MT_UNF_DISPLAY1, &currFmt);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("MT_UNF_DISP_GetFormat failed. ret = 0x%x\n", ret);
                continue;
            }

            ret = MT_DispFmtSetFormat(edid_info.enNativeFormat);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("MTADP_Disp_Init failed. ret = 0x%x\n", ret);
                continue;
            }

            SAMPLE_DISP_FORMAT_INFO_PRINT(" Change format from [%s] to [%s] \n", MT_DispFmt2Str(currFmt), MT_DispFmt2Str(edid_info.enNativeFormat));
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_DISP_FORMAT_INFO_PRINT("Print help info \n");
            continue;
        }

    }
}




/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_DispFmtParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

#ifndef MT_SAMPLE_APP
    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_DispFmtPrint_help(argv[0]);
        return MT_FAILURE;
    }
#endif
    while((opt = MTADP_Getopt(argc, argv, ":?hHf:c:s:q")) != -1)
    {
        switch(opt)
        {

            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_DispFmtPrint_help(argv[0]);
                return MT_FAILURE;
#ifndef MT_SAMPLE_APP
            case 'f':
                if(argc != 3)
                {
                    (MT_VOID)MT_DispFmtPrintFileHelp(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;

            case 's':
                if(argc != 7)
                {
                    (MT_VOID)MT_DispFmtPrintDvbsHelp(argv[0]);
                    return MT_FAILURE;
                }

                pInputParam->sig_type = MT_INPUT_SIG_TYPE_SAT;

                pInputParam->input_param.sat.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.sat.sym_rate = strtol(argv[3], 0, 0);
                pInputParam->input_param.sat.onoff_22k = strtol(argv[4], 0, 0);
                pInputParam->input_param.sat.polarization = strtol(argv[5], 0, 0);
                pInputParam->input_param.sat.port_type = strtol(argv[6], 0, 0);

                return MT_SUCCESS;

            case 'c':
                if(argc != 5)
                {
                    (MT_VOID)MT_DispFmtPrintDvbcHelp(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                return MT_SUCCESS;
#endif
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_DispFmtExit();
                }
                return MT_TASK_EXIT;

            default:
                (MT_VOID)MT_DispFmtPrint_help(argv[0]);
                return MT_FAILURE;
        }
    }


    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_DispFmtMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif

{
    MT_S32     ret = MT_SUCCESS;
#ifndef MT_SAMPLE_APP
    PMT_COMPACT_PROG  *pstCurrentProgInfo = NULL;
#endif
    mt_bool autoFmtFlag;

    ret = MT_DispFmtParase_args(argc, argv,&format_run_info.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_DISP_FORMAT_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        g_bTaskQuit = MT_FALSE;
#ifndef MT_SAMPLE_APP
        /** System initialization */
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to mt_sys_init\n");
            return ret;
        }

        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to StartDmx\n");
            goto ERR0;
        }

        sleep(1);

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR1;
        }


        if(MT_INPUT_SIG_TYPE_FILE != format_run_info.sInputParam.sig_type)
        {
            ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("MTADP_Fe_Init failed, ret = %d\n", ret);
                goto ERR2;
            }

            if (MT_INPUT_SIG_TYPE_CAB == format_run_info.sInputParam.sig_type)
            {     //dvbc
                ret = MT_DispFmtCheckDvbcParam(&format_run_info.sInputParam.input_param.cab);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DISP_FORMAT_ERR_PRINT("Input sat parameter error!, ret = %d\n", ret);
                    goto ERR3;
                }
                ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                            format_run_info.sInputParam.input_param.cab.freq,
                                            format_run_info.sInputParam.input_param.cab.sym_rate,
                                            format_run_info.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == format_run_info.sInputParam.sig_type)
            {
                ret = MT_DispFmtCheckDvbsParam(&format_run_info.sInputParam.input_param.sat);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DISP_FORMAT_ERR_PRINT("Input sat parameter error!, ret = %d\n", ret);
                    goto ERR3;
                }
                ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                            format_run_info.sInputParam.input_param.sat.freq,
                                            format_run_info.sInputParam.input_param.sat.sym_rate,
                                            format_run_info.sInputParam.input_param.sat.onoff_22k,
                                            format_run_info.sInputParam.input_param.sat.polarization,
                                            format_run_info.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("MTADP_Fe_Connect failed, ret = %d\n", ret);
                goto ERR3;
            }
        }


        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR3;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR4;
        }

        ret = MT_DispFmtDmxInit(&format_run_info.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT( "failed to StartDmx\n");
            goto ERR5;
        }

        if(MT_INPUT_SIG_TYPE_FILE == format_run_info.sInputParam.sig_type)
        {
            ret = pthread_create(&format_run_info.stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_DispFmtInjectTsTask, &format_run_info.sInputParam.input_param.file);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DISP_FORMAT_ERR_PRINT("failed to pthread_create\n");
                goto ERR6;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR6;
            }
        }

        (MT_VOID)MTADP_Search_Init();
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &format_run_info.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR8;
        }

        ret = MT_DispFmtAVplayInit(&format_run_info.hAvPlay, &format_run_info.hWin, &format_run_info.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to StartAVPlay\n");
            goto ERR9;
        }
        pstCurrentProgInfo = format_run_info.pProgTbl->proginfo;
        ret = MT_DispFmtAVPlay_Start(format_run_info.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DISP_FORMAT_ERR_PRINT("failed to MTADP_AVPlay_PlayProg\n");
            goto ERR10;
        }
#endif
#ifdef MT_SAMPLE_APP
        format_run_info.hAvPlay = avplayHandle.hAvPlay;
#endif

    }

    /* Play the first program on the program list*/
    (MT_VOID) MT_DispFmtCmdTask(format_run_info.hAvPlay, format_run_info.pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    MT_DispFmtGetAutoFmtFlag(&autoFmtFlag);
    if (autoFmtFlag != MT_FALSE)
    {
        (MT_VOID)MT_UNF_AVPLAY_UnRegisterEvent(format_run_info.hAvPlay, MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME);
        (MT_VOID)MT_UNF_AVPLAY_UnRegisterEvent(format_run_info.hAvPlay, MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME);
        (MT_VOID)MT_UNF_AVPLAY_UnRegisterEvent(format_run_info.hAvPlay, MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT);
    }

#ifndef MT_SAMPLE_APP
    (MT_VOID)MT_DispFmtStopplay(format_run_info.hAvPlay);
    SAMPLE_DISP_FORMAT_INFO_PRINT("stop to play\n");

ERR10:
    (MT_VOID)MT_DispFmtAvplayDeInit(format_run_info.hAvPlay, format_run_info.hWin, format_run_info.hSoundTrack);
ERR9:
    (MT_VOID)MTADP_Search_FreeAllPmt(format_run_info.pProgTbl);
ERR8:
    (MT_VOID)MTADP_Search_DeInit();
ERR7:
    if(MT_INPUT_SIG_TYPE_FILE == format_run_info.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(format_run_info.stInjectTSThread, NULL);
    }
ERR6:
    (MT_VOID)MT_DispFmtDmxDeInit();
ERR5:
    (MT_VOID)MTADP_VO_DeInit();
ERR4:
    (MT_VOID)MTADP_Snd_DeInit();

ERR3:
    if(MT_INPUT_SIG_TYPE_FILE != format_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR2:
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&format_run_info, 0xff, sizeof(format_run_info));

    return MT_SUCCESS;
}
