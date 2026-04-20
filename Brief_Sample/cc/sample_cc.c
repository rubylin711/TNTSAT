/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_demux.h"
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
#include "pthread.h"
#include "mt_unf_cc.h"
#include "mt_cmdline.h"
#include "sample_cc_out.h"
#include "sample_cc_data.h"
#include "sample_cc_xds.h"
#include "sample_cc_common.h"
/***************************** Macro Definition ******************************/
#define DMX_ID_0            0
#define TUNER_ID_0          0
#define SYN_USING_LOCALTIME 0
#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
/*************************** Structure Definition ****************************/
typedef enum input_sig_type_t {
    MT_INPUT_SIG_TYPE_CAB = 1,
    /**<Cable signal*/
    MT_INPUT_SIG_TYPE_SAT = 2,
    /**<Satellite signal*/
    MT_INPUT_SIG_TYPE_DVB_T = 3,
    /**<Terrestrial signal*/
    MT_INPUT_SIG_TYPE_FILE = 4,
    /**<local file */
}MT_INPUR_SIG_TYPE_T;

typedef struct
{
    MT_U32 freq; /**<Frequency, in kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/
    MT_U32 mod_type; /**<QAM mode*/
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
    MT_U32 freq; /**<Frequency, in kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/
    MT_U32 mod_type; /**<QAM mode*/
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
    MT_HANDLE          hCCData;
    MT_HANDLE          hVBI;
    MT_HANDLE          hcc;
    MT_HANDLE          hOutput;
    pthread_t          usrRecvThread;
    pthread_t          stInjectTSThread;
    PMT_COMPACT_TBL    *pProgTbl;
    mt_input_para_t    sInputParam;
    MT_UNF_CC_DATA_TYPE_E enCCDataType;
} MT_CC_RUN_INFO;
/********************** Global Variable declaration **************************/
static MT_BOOL   g_bTaskQuit = MT_TRUE;
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif
static MT_CC_RUN_INFO cc_run_info;
/******************************* API declaration *****************************/
static MT_VOID MT_CCExit(void);

#ifdef MT_SAMPLE_APP
MT_S32 MT_CCMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*!
@brief Get current pts callback function
@param[in]  u32UserData                User Data
@param[out] ps64CurrentPts             The current pts value
@return::MT_SUCCESS
@*/
#ifdef CONFIG_MT_CHIP_SYMPHONY4
static MT_S32 GetCurPts(mt_u32 u32UserData, mt_s64 *ps64CurrentPts)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
static MT_S32 GetCurPts(ulong u32UserData, mt_s64 *ps64CurrentPts)
#endif
{
    MT_S32                      s32Ret = MT_FAILURE;
    static MT_U32               u32PrePtsTime = 0;
    MT_UNF_AVPLAY_STATUS_INFO_S stStatusInfo = { 0 };

    s32Ret = MT_UNF_AVPLAY_GetStatusInfo(cc_run_info.hAvPlay, &stStatusInfo);
    if(s32Ret != MT_SUCCESS)
    {
        SAMPLE_CC_ERR_PRINT("failed to MT_UNF_AVPLAY_GetStatusInfo\n");
    }

#if SYN_USING_LOCALTIME
    *ps64CurrentPts = stStatusInfo.stSyncStatus.u64LocalTime;
#else
    *ps64CurrentPts = stStatusInfo.stSyncStatus.u64LastVidPts;
#endif

    if(*ps64CurrentPts < u32PrePtsTime)
    {
        SAMPLE_CC_INFO_PRINT("Get PTS: %llu, pre time is %u\n", *ps64CurrentPts, u32PrePtsTime);
    }

    u32PrePtsTime = *ps64CurrentPts;

    return MT_SUCCESS;
}


/*!
@brief Sets the color of the text or background
@param[in] u8flag                Choose to set text or background
@return::MT_VOID
@*/
static MT_VOID SwitchColor(MT_U8 u8flag)
{
    MT_S8             inPutCmd[32] = { 0 };
    MT_S32            type = 0;
    MT_S32            s32Ret = MT_SUCCESS;
    MT_UNF_CC_ATTR_S  stCCAttr = { 0 };
    MT_UNF_CC_COLOR_E ColorMap[] =
    {
        MT_UNF_CC_COLOR_BLACK, MT_UNF_CC_COLOR_WHITE, MT_UNF_CC_COLOR_RED,
        MT_UNF_CC_COLOR_GREEN, MT_UNF_CC_COLOR_BLUE, MT_UNF_CC_COLOR_YELLOW,
        MT_UNF_CC_COLOR_MAGENTA, MT_UNF_CC_COLOR_CYAN, MT_UNF_CC_COLOR_DEFAULT
    };

    if(MT_SUCCESS == MT_UNF_CC_GetAttr(cc_run_info.hcc, &stCCAttr))
    {
        MT_CC_PRINT("\n");
        MT_CC_PRINT("0 ------ black\n");
        MT_CC_PRINT("1 ------ white\n");
        MT_CC_PRINT("2 ------ red\n");
        MT_CC_PRINT("3 ------ green\n");
        MT_CC_PRINT("4 ------ blue\n");
        MT_CC_PRINT("5 ------ yellow\n");
        MT_CC_PRINT("6 ------ megenta\n");
        MT_CC_PRINT("7 ------ cyan\n");
        MT_CC_PRINT("8 ------ default\n");
        MT_CC_PRINT("CMD>> ");
        fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);
        type = atoi((char*)inPutCmd);

        if(type > 8)
        {
            SAMPLE_CC_ERR_PRINT("Input error!\n");
            return;
        }

        if(1 == u8flag)
        {
            if(MT_UNF_CC_DATA_TYPE_608 == stCCAttr.enCCDataType)
            {
                stCCAttr.unCCConfig.stCC608ConfigParam.u32CC608TextColor = ColorMap[type];
            }
            else if(MT_UNF_CC_DATA_TYPE_708 == stCCAttr.enCCDataType)
            {
                stCCAttr.unCCConfig.stCC708ConfigParam.u32CC708TextColor = ColorMap[type];
            }

        }
        else
        {
            if(MT_UNF_CC_DATA_TYPE_608 == stCCAttr.enCCDataType)
            {
                stCCAttr.unCCConfig.stCC608ConfigParam.u32CC608BgColor = ColorMap[type];
            }
            else if(MT_UNF_CC_DATA_TYPE_708 == stCCAttr.enCCDataType)
            {
                stCCAttr.unCCConfig.stCC708ConfigParam.u32CC708BgColor = ColorMap[type];
            }
        }

        s32Ret = MT_UNF_CC_SetAttr(cc_run_info.hcc, &stCCAttr);
        if(MT_SUCCESS == s32Ret)
        {
            SAMPLE_CC_INFO_PRINT("<The color is set successfully>\n");
        }
        else
        {
            SAMPLE_CC_ERR_PRINT("failed to MT_UNF_CC_SetAttr\n");
        }
    }

}


/*!
@brief Start or stop the CC module
@param[in] MT_VOID
@return::MT_VOID
@*/
static MT_VOID SwitchCaptionOut(MT_VOID)
{
    static MT_BOOL s_bStartFlag = MT_TRUE;
    if(MT_FALSE == s_bStartFlag)
    {
        MT_UNF_CC_Start(cc_run_info.hcc);
        s_bStartFlag = MT_TRUE;
        SAMPLE_CC_INFO_PRINT("<cc started>\n");
    }
    else
    {
        MT_UNF_CC_Stop(cc_run_info.hcc);
        s_bStartFlag = MT_FALSE;
        SAMPLE_CC_INFO_PRINT("<cc stoped>\n");
    }
}


/*!
@brief Filter data callback function
@param[in] u32UserData         User data
@param[in] pu8Data             PES data address
@param[in] u32DataLength       PES data length
@return::s32Ret                The return value of the error or success
@*/
static MT_S32 FilterDataCallback(MT_U32 u32UserData, MT_U8 *pu8Data, MT_U32 u32DataLength)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_U32 u32ARIBCCPID = u32UserData;

    if(cc_run_info.hcc)
    {
        if(u32ARIBCCPID)
        {
            //printf("<<<<<<<<<<<< ARIBCCPID = %#x,DataLength = %d\n",u32ARIBCCPID,u32DataLength);
            s32Ret = MT_UNF_CC_InjectPESData(cc_run_info.hcc, pu8Data, u32DataLength);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("failed to MT_UNF_CC_InjectPESData, u32ARIBCCPID = 0x%x\n", u32ARIBCCPID);
            }
        }
    }

    return s32Ret;
}


/*!
@brief CC data injection thread
@param[in] arg                 MT_VOID
@return::MT_VOID
@*/
static MT_VOID *UsrDataInject(MT_VOID *arg)
{
    MT_S32                       s32Ret = MT_FAILURE;
    static MT_U32                time = 0;
    MT_UNF_CC_USERDATA_S         stUserData = { 0 };
    MT_UNF_DISP_VBI_DATA_S       stVBIData = { 0 };
    MT_UNF_VIDEO_USERDATA_S      stUsrData = { 0 };
    MT_UNF_VIDEO_USERDATA_TYPE_E enType = { 0 };

    while(MT_FALSE == g_bTaskQuit)
    {
#ifdef MT_SAMPLE_APP
        cc_run_info.hAvPlay = avplayHandle.hAvPlay;
        if(0 == cc_run_info.hAvPlay)
        {
            g_bTaskQuit = MT_TRUE;
            (mt_void)MT_CCExit();
        }
#endif
        if(cc_run_info.hAvPlay)
        {
            s32Ret = MT_UNF_AVPLAY_AcqUserData(cc_run_info.hAvPlay, &stUsrData, &enType);
            if(MT_SUCCESS == s32Ret)
            {
                time = 0;
                if(MT_UNF_VIDEO_USERDATA_DVB1_CC == enType)
                {
                    if(cc_run_info.hcc)
                    {
                        stUserData.pu8userdata = stUsrData.pu8Buffer;
                        stUserData.u32dataLen = stUsrData.u32Length;
                        s32Ret = MT_UNF_CC_InjectUserData(cc_run_info.hcc, &stUserData);
                        if(MT_SUCCESS != s32Ret)
                        {
                            SAMPLE_CC_ERR_PRINT("failed to MT_UNF_CC_InjectUserData, u32Length = 0x%x\n", stUsrData.u32Length);
                        }
                        stVBIData.enType = MT_UNF_DISP_VBI_TYPE_CC;
                        stVBIData.pu8DataAddr = stUsrData.pu8Buffer;
                        stVBIData.u32DataLen = stUsrData.u32Length;
                        MT_UNF_DISP_SendVBIData(cc_run_info.hVBI, &stVBIData);
                    }
                }

                s32Ret = MT_UNF_AVPLAY_RlsUserData(cc_run_info.hAvPlay, &stUsrData);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_CC_ERR_PRINT("MT_UNF_AVPLAY_RlsUserData return %#x\n", s32Ret);
                }
            }
            else
            {
                time++;
                if(time > 5000)
                {
                    SAMPLE_CC_ERR_PRINT("MT_UNF_AVPLAY_AcqUserData failed\n");
                    time = 0;
                }
                MT_USLEEP(50*1000);
            }
        }
    }
    return (MT_VOID *)0;
}

#ifndef MT_SAMPLE_APP

/*!
@brief The thread that receives the file stream data
@param[in] args                 TS file name
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 InjectTsTask(MT_VOID *args)
{
    MT_S32                 s32Ret = MT_SUCCESS;
    MT_U32                 Readlen = 0;
    MT_CHAR                *fileName = NULL;
    MT_HANDLE              hTsBuffer = MT_INVALID_HANDLE;
    FILE                   *pTsFile = NULL;
    MT_UNF_STREAM_BUF_S    StreamBuf = { 0 };

    fileName = (MT_CHAR*)args;
    pTsFile = fopen(fileName, "rb");
    if(NULL == pTsFile)
    {
        SAMPLE_CC_ERR_PRINT( "file %s open error!!\n", fileName);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    if(MT_INPUT_SIG_TYPE_FILE == cc_run_info.sInputParam.sig_type)
    {
        s32Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
            g_bTaskQuit = MT_TRUE;
            fclose(pTsFile);
            pTsFile = NULL;
            return s32Ret;
        }
    }

    /** loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        /** Obtains a TS buffer to input data */
        s32Ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*200, &StreamBuf, 1000);
        if(MT_SUCCESS != s32Ret)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(MT_S8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            if(cc_run_info.hAvPlay)
            {
                /** Resets an AVPLAY. In this case */
                (MT_VOID)MT_UNF_AVPLAY_Reset(cc_run_info.hAvPlay, NULL);
            }

            /** Resets a TS buffer to clear its data */
            (MT_VOID)MT_UNF_DMX_ResetTSBuffer(hTsBuffer);

            SAMPLE_CC_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER...............!\n");
            rewind(pTsFile);
            continue;
        }

        /** Updates the write pointer of a TS buffer after the TS data is input */
        s32Ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != s32Ret)
        {
           SAMPLE_CC_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    if(MT_INPUT_SIG_TYPE_FILE == cc_run_info.sInputParam.sig_type)
    {
        /** Destroys an existing TS buffer */
        (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);
    }

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }

    return MT_SUCCESS;
}


/*!
@brief Check if the QAM matches
@param[in]  mod_type            QAM
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static MT_S32 MT_CCModeCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_CC_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_CC_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_CC_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*!
@brief Check if the DVBS param matches
@param[in]  p_sat_in            DVBS param
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static mt_s32 MT_CCModeCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_CC_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_CC_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*!
@brief Set the PID of the AV player and set the encoder type.
@param[in]  hAvplay             Handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error
@*/
static MT_S32 MT_CCModeSetAvplayPidAndCodecType(MT_HANDLE hAvplay      , const PMT_COMPACT_PROG *pProgInfo)
{
    MT_S32                           s32Ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };

    if(MT_INVALID_HANDLE == hAvplay || NULL == pProgInfo)
    {
        SAMPLE_CC_ERR_PRINT("The input address is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    if(pProgInfo->VElementNum > 0)
    {
        VidPid = pProgInfo->VElementPid;
        enVidType = pProgInfo->VideoType;
    }
    else
    {
        VidPid = INVALID_TSPID;
        enVidType = MT_UNF_VCODEC_TYPE_BUTT;
    }

    if(pProgInfo->AElementNum > 0)
    {
        AudPid = pProgInfo->AElementPid;
        u32AudType = pProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    SAMPLE_CC_INFO_PRINT("VidPid = %#x, AudPid = %#x-------<%s> line: %d\n", VidPid, AudPid, __FUNCTION__, __LINE__);

    if(VidPid != INVALID_TSPID)
    {
        /** Get the video properties of the AV player */
        s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }

        /** Set the video properties of the AV player */
        if (MT_UNF_VCODEC_TYPE_VC1 == enVidType)
        {
            VdecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = 1;
            VdecAttr.unExtAttr.stVC1Attr.u32CodecVersion = 8;
        }

        if (MT_UNF_VCODEC_TYPE_VP6 == enVidType)
        {
            VdecAttr.unExtAttr.stVP6Attr.bReversed = 0;
        }

        VdecAttr.enType = enVidType;
        VdecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
        VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VdecAttr.u32ErrCover = 100;
        VdecAttr.s32CtrlOptions = 0;
        VdecAttr.u32Priority = 3;
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);

        /** Set the video PID properties of AV player */
        s32Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &VidPid);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("Set video properties or video PID property failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    if(AudPid != INVALID_TSPID)
    {
        /** Set audio decoder properties */
        s32Ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);

        /** Set the audio PID properties of AV player */
        s32Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("Setting the decoding mode or audio PID property failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        /** Set the audio and video synchronization properties of AV player */
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (MT_VOID *)&DmxAvsync);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state
@param[in]  hAvplay             Handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error
@*/
static MT_S32 MT_CCModeStarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        s32Ret = MT_FAILURE;
    MT_U32                        pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    if(MT_INVALID_HANDLE == hAvplay || NULL == pProgInfo)
    {
        SAMPLE_CC_ERR_PRINT("The input address is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    /** Set the PID of the AV player and set the encoder type */
    MT_CCModeSetAvplayPidAndCodecType(hAvplay, pProgInfo);

    /** Get the audio PID properties of AV player */
    s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
    if((MT_SUCCESS == s32Ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_CC_ERR_PRINT("Has no audio stream!, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

    /** Get the video PID properties of AV player */
    s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if((MT_SUCCESS == s32Ret) && (0x1fff != pid))
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_CC_ERR_PRINT("Has no video stream!, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

    if((enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        /** Set the frame rate parameter of AV player, enable vo frame rate detect */
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("Set frame to VO is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }

        /** Get synchronization properties of AV player */
        s32Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("Get avplay sync attr is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        s32Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("Set avplay sync attr is failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            return s32Ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    s32Ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_AVPLAY_Start failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    return MT_SUCCESS;
}


/*!
@brief stop AV playback into the stop state
@param[in] hAvplay              Handle to AV player
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_CCModeStopToPlay(MT_HANDLE hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_CC_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    option.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    option.u32TimeoutMs = 0;
    SAMPLE_CC_INFO_PRINT("stop live play ...\n");
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}


/*!
@brief audio and video player initialization
@param[out] phAvplay            Handle to AV player
@param[out] phWin               The input window handler
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_CCModeAvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin, MT_HANDLE *phSoundTrack)
{
    MT_S32                   s32Ret = MT_FAILURE;
    MT_HANDLE                hAvplay = MT_INVALID_HANDLE;
    MT_HANDLE                hWin = MT_INVALID_HANDLE;
    MT_HANDLE                hSoundTrack = MT_INVALID_HANDLE;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    if(NULL == phAvplay || NULL == phWin || NULL == phSoundTrack)
    {
        SAMPLE_CC_ERR_PRINT("The input address is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Audio decoder */
    s32Ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MTADP_AVPlay_RegADecLib failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** AV player initialization */
    s32Ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_AVPLAY_Init failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    s32Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR1;
    }

    /** Create AV player based on attributes */
    s32Ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_AVPLAY_Create failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR2;
    }

    /** Open the audio channel of the AV player */
    s32Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    s32Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR4;
    }

    /** Create a track based on the audio device model */
    s32Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_SND_CreateTrack failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR4;
    }

    /** Attaches the SND module to an AV player */
    s32Ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_SND_Attach failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR5;
    }

    /** Create a window */
    s32Ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MTADP_VO_CreatWin failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR6;
    }

    /** Bind AV player to the window */
    s32Ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_VO_AttachWindow failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR7;
    }

    /** Enable/disable windows */
    s32Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        goto ERROR8;
    }

    *phWin = hWin;
    *phSoundTrack = hSoundTrack;
    *phAvplay = hAvplay;

    return MT_SUCCESS;

ERROR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);
ERROR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);
ERROR6:
    (MT_VOID)MT_UNF_SND_Detach(hSoundTrack, hAvplay);
ERROR5:
    (MT_VOID)MT_UNF_SND_DestroyTrack(hSoundTrack);
ERROR4:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
ERROR3:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
ERROR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);
ERROR1:
    (MT_VOID)MT_UNF_AVPLAY_DeInit();

    return MT_FAILURE;
}


/*!
@brief audio and video player deinitialization
@param[in]  hAvplay             handle to AV player
@param[in]  hWin                Handle to window
@param[in]  hSoundTrack         Handle to sound track
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_VOID MT_CCModeAvplayDeInit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
    if(MT_INVALID_HANDLE == hAvplay || MT_INVALID_HANDLE == hWin || MT_INVALID_HANDLE == hSoundTrack)
    {
        SAMPLE_CC_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }

    /** Enable/disable windows */
    (MT_VOID)MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);

    /** Unbind the window and AV player */
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

    /** Destroy window */
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);

    /** Contact the binding of track and AV player */
    (MT_VOID)MT_UNF_SND_Detach(hSoundTrack, hAvplay);

    /** Destroy a Track */
    (MT_VOID)MT_UNF_SND_DestroyTrack(hSoundTrack);

    /** Turn off the video channel */
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

    /** Turn off the audio channel */
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

    /** Destroy the AV player */
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);

    /** Deinitializes the AV player module */
    (MT_VOID)MT_UNF_AVPLAY_DeInit();
}

/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_CCModeDmxInit(MT_INPUR_SIG_TYPE_T sig_type)
{
    MT_S32           s32Ret = MT_FAILURE;
    mt_sys_version_s stSysChipInfo;

    /** Initializes the demux module */
    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_DMX_Init failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    if(MT_INPUT_SIG_TYPE_FILE == sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            (MT_VOID)MT_UNF_DMX_DeInit();
            return s32Ret;
        }
    }
    else
    {
        memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
        s32Ret = mt_sys_get_version(&stSysChipInfo);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to mt_sys_get_version\n");
            return MT_FAILURE;
        }

        if(MT_INPUT_SIG_TYPE_CAB == sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            }
            else
            {
                s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_0);
            }
        }
        else if(MT_INPUT_SIG_TYPE_SAT == sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
            }
            else
            {
                s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            }
        }
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}


/*!
@brief Demux module deinitialization
@return::MT_VOID
@*/
static MT_VOID MT_CCModeDmxDeinit(MT_VOID)
{
    /** Unbind demux from the port */
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();
}
#endif

/*!
@brief Set CC parameters and start the CC module
@param[in]  pProgTbl            PMT program schedule
@param[in]  u8ProgNo            Program number
@param[in]  dataType            The data type of CC
@return::s32Ret                 The return value of the error or success.
@*/
static MT_S32 MT_CCModeCCStart(MT_UNF_CC_DATA_TYPE_E dataType)
{
    MT_S32            s32Ret = MT_FAILURE;
    MT_UNF_CC_ATTR_S  stCCAttr = { 0 };
    MT_UNF_CC_PARAM_S stCCParam = { 0 };

    memset(&stCCAttr, 0, sizeof(MT_UNF_CC_ATTR_S));
    memset(&stCCParam, 0, sizeof(MT_UNF_CC_PARAM_S));

    if(MT_UNF_CC_DATA_TYPE_708 == dataType)
    {
        SAMPLE_CC_INFO_PRINT("<Used CC 708>\n");

        /** Get default attribution in CC module */
        stCCAttr.enCCDataType = MT_UNF_CC_DATA_TYPE_708;
        MT_UNF_CC_GetDefaultAttr(&stCCAttr);
        stCCAttr.unCCConfig.stCC708ConfigParam.enCC708DispFormat = MT_UNF_CC_DF_1280X720;
    }
    else if(MT_UNF_CC_DATA_TYPE_ARIB == dataType)
    {
        SAMPLE_CC_INFO_PRINT("<Used ARIB CC>\n");

        /** Get default attribution in CC module */
        stCCAttr.enCCDataType = MT_UNF_CC_DATA_TYPE_ARIB;
        MT_UNF_CC_GetDefaultAttr(&stCCAttr);
    }
    else
    {
        SAMPLE_CC_INFO_PRINT("<Used CC 608>\n");

        /** Get default attribution in CC module */
        stCCAttr.enCCDataType = MT_UNF_CC_DATA_TYPE_608;
        MT_UNF_CC_GetDefaultAttr(&stCCAttr);
        stCCAttr.unCCConfig.stCC608ConfigParam.enCC608DispFormat = MT_UNF_CC_DF_1280X720;
    }

    /** Open CC module */
    stCCParam.pfnCCDisplay = CC_Output_OnDraw;
    stCCParam.stCCAttr = stCCAttr;
    stCCParam.pfnCCGetTextSize = CC_Output_GetTextSize;
    stCCParam.pfnBlit = CC_Output_Blit;
    stCCParam.pfnVBIOutput = CC_Output_VBIOutput;
    stCCParam.pfnCCGetPts = GetCurPts;
    stCCParam.pfnXDSOutput = CC608_XDS_Decode;
    s32Ret = MT_UNF_CC_Create(&stCCParam,&cc_run_info.hcc);
    if(MT_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    /** Start CC module */
    s32Ret = MT_UNF_CC_Start(cc_run_info.hcc);
    if(MT_SUCCESS != s32Ret)
    {
        /** Close cc module */
        MT_UNF_CC_Destroy(cc_run_info.hcc);
        return s32Ret;
    }

    return MT_SUCCESS;

}


static MT_VOID MT_CCModeCCStop(MT_VOID)
{
    /** Stop cc module */
    (MT_VOID)MT_UNF_CC_Stop(cc_run_info.hcc);

    /** close cc module */
    (MT_VOID)MT_UNF_CC_Destroy(cc_run_info.hcc);
}


static MT_VOID MT_CCExit()
{
    g_bTaskQuit = MT_TRUE;

    if(MT_UNF_CC_DATA_TYPE_ARIB == cc_run_info.enCCDataType)
    {
        /** Close the relevant DEMUX channel */
        (MT_VOID)CC_Data_Uninstall(cc_run_info.hCCData);

        /** The CC data acquisition module is deinitialized */
        (MT_VOID)CC_Data_DeInit();
    }
    else
    {
        /** Wait for the thread to end */
        pthread_join(cc_run_info.usrRecvThread, NULL);

        /** Destroy VBI data channel */
        (MT_VOID)MT_UNF_DISP_DestroyVBI(cc_run_info.hVBI);
    }


    (MT_VOID)MT_CCModeCCStop();

    /** CC module output deinitialization */
    (MT_VOID)CC_Output_DeInit(cc_run_info.hOutput);

    /** DeInitialize cc module */
    (MT_VOID)MT_UNF_CC_DeInit();

    memset(&cc_run_info, 0xff, sizeof(cc_run_info));
}


/*!
@brief Print information for ARIB subtitles
@param[in] MT_VOID
@return::MT_VOID
@*/
static MT_VOID MT_CCModePrintAribCaptionInfo(MT_VOID)
{
    MT_UNF_CC_ARIB_INFO_S *pstCCAribInfo = NULL;

    pstCCAribInfo = (MT_UNF_CC_ARIB_INFO_S *)malloc(sizeof(MT_UNF_CC_ARIB_INFO_S));

    if(MT_SUCCESS == MT_UNF_CC_GetARIBCCInfo(cc_run_info.hcc, pstCCAribInfo))
    {
        MT_CC_PRINT("===================CC_PrintAribCaptionInfo===================\n");
        SAMPLE_CC_INFO_PRINT("enCCAribTMD: %d\n", pstCCAribInfo->enCCAribTMD);
        SAMPLE_CC_INFO_PRINT("u32NumLanguage: %d\n", pstCCAribInfo->u32NumLanguage);
        for(MT_U8 i = 0; i < pstCCAribInfo->u32NumLanguage; i++)
        {
            SAMPLE_CC_INFO_PRINT("u8LanguageTag:        %d\n", pstCCAribInfo->stCCAribInfonode[i].u8LanguageTag);
            SAMPLE_CC_INFO_PRINT("enCCAribDMF:          %#x\n", pstCCAribInfo->stCCAribInfonode[i].enCCAribDMF);
            SAMPLE_CC_INFO_PRINT("acISO639LanguageCode: %s\n", pstCCAribInfo->stCCAribInfonode[i].acISO639LanguageCode);
            SAMPLE_CC_INFO_PRINT("enCCAribDF:           %d\n", pstCCAribInfo->stCCAribInfonode[i].enCCAribDF);
            SAMPLE_CC_INFO_PRINT("enCCAribTCS:          %d\n", pstCCAribInfo->stCCAribInfonode[i].enCCAribTCS);
            SAMPLE_CC_INFO_PRINT("enCCAribRollup:       %d\n", pstCCAribInfo->stCCAribInfonode[i].enCCAribRollup);
        }
        MT_CC_PRINT("\n=====================================================\n");
    }
    else
    {
        SAMPLE_CC_ERR_PRINT("MT_UNF_CC_GetARIBCCCaptionInfo fail!\n");
    }

     free(pstCCAribInfo);
     pstCCAribInfo = NULL;
}


/*!
@brief Start filtering the ARIB data
@param[in] pSubTitleProgTbl    PMT program schedule
@param[in] u8ProgNo            Program number
@return::MT_SUCCESS            Success.
@return::s32Ret                The return value of the error.
@*/
static MT_S32 MT_CCModeStartDataFilter(PMT_COMPACT_PROG        *Proginfo)
{
    MT_S32                  s32Ret = MT_FAILURE;
    PMT_COMPACT_PROG        *pstProginfo = Proginfo;
    CC_DATA_INSTALL_PARAM_S stInstallParam = { 0 };

    /** Destroy the threads and channels that read the ES stream data */
    s32Ret = CC_Data_DeInit();

    /** Create a thread to read the ES stream data */
    s32Ret |= CC_Data_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("failed to CC_Data_Init\n");
        return s32Ret;
    }

    /** Create and set up DMX channels */
    stInstallParam.u32DmxID = 0;
    stInstallParam.pfnCallback = FilterDataCallback;
    stInstallParam.u16CCPID = pstProginfo->u16ARIBCCPid;
    stInstallParam.u32UserData = (MT_U32)pstProginfo->u16ARIBCCPid;
    s32Ret |= CC_Data_Install(&stInstallParam, &cc_run_info.hCCData);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("failed to CC_Data_Install\n");

        /** Destroy the threads and channels that read the ES stream data */
        (MT_VOID)CC_Data_DeInit();
        return s32Ret;
    }

    return MT_SUCCESS;
}


static MT_VOID MT_CCModePrintMenu(PMT_COMPACT_TBL *pProgTbl, MT_UNF_CC_DATA_TYPE_E enCCDataType)
{
    MT_CC_PRINT("commond: \n");
    MT_CC_PRINT("     w: start/stop caption \n");

    if(MT_UNF_CC_DATA_TYPE_608 == enCCDataType || MT_UNF_CC_DATA_TYPE_708 == enCCDataType)
    {
        MT_CC_PRINT("     a: select text color.\n");
        MT_CC_PRINT("     s: select text bg color.\n");
    }
    else if(MT_UNF_CC_DATA_TYPE_ARIB == enCCDataType)
    {
        MT_CC_PRINT("     i: print arib cc caption information.\n");
    }
#ifdef MT_SAMPLE_APP
    MT_CC_PRINT("     b : background run \n");
#endif
    MT_CC_PRINT("     h: help \n");
    MT_CC_PRINT("     q: quit \n");
#ifndef MT_SAMPLE_APP
    MT_CC_PRINT("'1-%d': select a program\n", pProgTbl->prog_num);
#endif
    MT_CC_PRINT("CC>> ");
}


/*!
@brief The thread on which the command was entered
@param[in]  pProgTbl            PMT program schedule
@param[in]  u8ProgNo            Program number
@param[out] enCCDataType        The data type of CC
@return::MT_VOID
@*/
static MT_VOID MT_CCModeCmdTask(MT_HANDLE hAvPlay, PMT_COMPACT_TBL *pProgTbl, MT_UNF_CC_DATA_TYPE_E enCCDataType)
{
    MT_U32           u32ProgNum = 0;
    MT_CHAR            inPutCmd[32] = { 0 };
#ifndef MT_SAMPLE_APP
    MT_S32           s32Ret = MT_SUCCESS;
    PMT_COMPACT_PROG *pstCurrentProgInfo = NULL;
#endif

#ifndef MT_SAMPLE_APP
    if(NULL == pProgTbl)
    {
        SAMPLE_CC_ERR_PRINT("The input address is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        g_bTaskQuit = MT_TRUE;
        return;
    }
#endif
    while(1)
    {
        (MT_VOID)MT_CCModePrintMenu(pProgTbl, enCCDataType);

        fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);

        if('q' == inPutCmd[0])
        {
            SAMPLE_CC_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inPutCmd[0])
        {
            SAMPLE_CC_INFO_PRINT("Cc play in back!\n");
            break;
        }
#endif

        u32ProgNum = atoi(inPutCmd);
        if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
        {
#ifndef MT_SAMPLE_APP
            pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1) % pProgTbl->prog_num);
            MT_CCModeStopToPlay(hAvPlay);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("failed to StopAVPlay\n");
            }

            s32Ret = MT_CCModeStarToPlay(hAvPlay, pstCurrentProgInfo);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("Switching shows failed\n");
            }

            s32Ret = MT_UNF_CC_Stop(cc_run_info.hcc);
            s32Ret |= MT_UNF_CC_Reset(cc_run_info.hcc);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("failed to MT_UNF_CC_Reset\n");
            }

            if(MT_UNF_CC_DATA_TYPE_ARIB == enCCDataType)
            {
                s32Ret = MT_CCModeStartDataFilter(pstCurrentProgInfo);
                if(MT_SUCCESS != s32Ret)
                {
                   SAMPLE_CC_ERR_PRINT("failed to MT_CCModeStartDataFilter\n");
                }
            }

            s32Ret = MT_UNF_CC_Start(cc_run_info.hcc);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("failed to MT_UNF_CC_Start\n");
            }
#endif
        }
        else if('w' == inPutCmd[0])
        {
            (MT_VOID)SwitchCaptionOut();
        }
        else if(MT_UNF_CC_DATA_TYPE_ARIB != enCCDataType && 'a' == inPutCmd[0])
        {
            (MT_VOID)SwitchColor(1);
        }
        else if(MT_UNF_CC_DATA_TYPE_ARIB != enCCDataType && 's' == inPutCmd[0])
        {
            (MT_VOID)SwitchColor(0);

        }
        else if(MT_UNF_CC_DATA_TYPE_ARIB == enCCDataType && 'i' == inPutCmd[0])
        {
            (MT_VOID)MT_CCModePrintAribCaptionInfo();
        }
        else if('h' == inPutCmd[0])
        {
            SAMPLE_CC_INFO_PRINT("Print help info\n");
        }
    }
}


static MT_VOID MT_CCModePrint_Help(MT_CHAR *name)
{
    MT_CC_PRINT("Lack of parameters\n");
    MT_CC_PRINT("\nUsage:\n");
    MT_CC_PRINT("%s\n", name);
    MT_CC_PRINT("    -f: path of the stream file\n");
    MT_CC_PRINT("    -c: DVBC locks frequency\n");
    MT_CC_PRINT("    -s: DVBS locks frequency\n");
#ifdef MT_SAMPLE_APP
    MT_CC_PRINT("    -q: Exit the background\n");
#endif
    MT_CC_PRINT("example:\n");
    MT_CC_PRINT("    %s -f ./677-CC-12.ts\n", name);
    MT_CC_PRINT("    %s -c 314 6875 64\n", name);
    MT_CC_PRINT("    %s -s 3840 27500 1 0 0\n", name);
}


/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_CCModeParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

#ifndef MT_SAMPLE_APP
    if(argc < 2 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_CCModePrint_Help(argv[0]);
        return MT_FAILURE;
    }
#endif

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:s:c:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_CCModePrint_Help(argv[0]);
                return MT_FAILURE;
#ifndef MT_SAMPLE_APP
            case 'f':
                if(argc != 3)
                {
                    (MT_VOID)MT_CCModePrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy(pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;

            case 's':
                if(argc != 7)
                {
                    (MT_VOID)MT_CCModePrint_Help(argv[0]);
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
                    (MT_VOID)MT_CCModePrint_Help(argv[0]);
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
                    (MT_VOID)MT_CCExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_CCModePrint_Help(argv[0]);
                return MT_FAILURE;
        }

    }

    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_CCMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
#ifndef MT_SAMPLE_APP
    MT_U8                  count = 0;
#endif
    MT_S32                 Num = 0;
    MT_S32                 s32Ret = MT_SUCCESS;
    PMT_COMPACT_PROG       *pstCurrentProgInfo = MT_NULL;
    MT_UNF_DISP_VBI_CFG_S  stVBICfg = { 0 };

    s32Ret = MT_CCModeParase_args(argc, argv, &cc_run_info.sInputParam);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_CC_INFO_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        /** System initialization */
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to mt_sys_init\n");
            return s32Ret;
        }

        /** HDMI initialization */
        s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to StartDmx\n");
            goto ERR0;
        }

        sleep(1);

        /** Display initialization */
        s32Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR1;
        }
        g_bTaskQuit = MT_FALSE;
        if(MT_INPUT_SIG_TYPE_FILE != cc_run_info.sInputParam.sig_type)
        {
            s32Ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }

            if (MT_INPUT_SIG_TYPE_CAB == cc_run_info.sInputParam.sig_type)
            {
                s32Ret = MT_CCModeCheckDvbcParam(&cc_run_info.sInputParam.input_param.cab);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_CC_ERR_PRINT("MT_CCModeCheckDvbcParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                    goto ERR3;
                }

                s32Ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                            cc_run_info.sInputParam.input_param.cab.freq,
                                            cc_run_info.sInputParam.input_param.cab.sym_rate,
                                            cc_run_info.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == cc_run_info.sInputParam.sig_type)
            {
                s32Ret = MT_CCModeCheckDvbsParam(&cc_run_info.sInputParam.input_param.sat);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_CC_ERR_PRINT("MT_CCModeCheckDvbsParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                    goto ERR3;
                }

                s32Ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                            cc_run_info.sInputParam.input_param.sat.freq,
                                            cc_run_info.sInputParam.input_param.sat.sym_rate,
                                            cc_run_info.sInputParam.input_param.sat.onoff_22k,
                                            cc_run_info.sInputParam.input_param.sat.polarization,
                                            cc_run_info.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR3;
            }
        }

        /** VO device initialization */
        s32Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR3;
        }

        /** Sound module initialization */
        s32Ret = MTADP_Snd_Init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR4;
        }

        /** DMX module initialization */
        s32Ret = MT_CCModeDmxInit(cc_run_info.sInputParam.sig_type);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to StartDmx\n");
            goto ERR5;
        }

        if(MT_INPUT_SIG_TYPE_FILE == cc_run_info.sInputParam.sig_type)
        {
            s32Ret = pthread_create(&cc_run_info.stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))InjectTsTask, &cc_run_info.sInputParam.input_param.file);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("failed to pthread_create\n");
                goto ERR6;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR6;
            }
        }

        (MT_VOID)MTADP_Search_Init();

        /** Get the PMT table */
        while(MT_SUCCESS != MTADP_Search_GetAllPmt(DMX_ID_0, &cc_run_info.pProgTbl))
        {
            count++;
            SAMPLE_CC_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            MT_USLEEP(100000);
            if(2 == count)
            {
                goto ERR8;
            }
        }

        /** AVPLAY initialization */
        s32Ret = MT_CCModeAvplayInit(&cc_run_info.hAvPlay, &cc_run_info.hWin, &cc_run_info.hSoundTrack);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to StartAVPlay\n");
            goto ERR9;
        }

        /** Start playing the show */
        pstCurrentProgInfo = cc_run_info.pProgTbl->proginfo;
        s32Ret = MT_CCModeStarToPlay(cc_run_info.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to MT_CCModeStarToPlay\n");
            goto ERR10;
        }
#endif
        g_bTaskQuit = MT_FALSE;
#ifdef MT_SAMPLE_APP
        cc_run_info.hAvPlay = avplayHandle.hAvPlay;
        if(0 == cc_run_info.hAvPlay)
        {
            SAMPLE_CC_ERR_PRINT("Didn't get avplay handle\n");
            goto ERR12;
        }

        s32Ret = MTADP_Get_Current_Info(&pstCurrentProgInfo);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to MTADP_Get_Current_Info\n");
            goto ERR11;
        }
#endif

        /** Initialize cc module */
        s32Ret = MT_UNF_CC_Init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to MT_UNF_CC_Init\n");
            goto ERR11;
        }

        /** CC module output initialization */
        s32Ret = CC_Output_Init(&cc_run_info.hOutput);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("CC_Output_Init failed!\n");
            goto ERR12;
        }

        /** Set the CC parameters and start CC */
        CC608_XDS_Init();
        if(pstCurrentProgInfo->u16ClosedCaptionNum > 0)
        {
            for(int j = 0; j < pstCurrentProgInfo->u16ClosedCaptionNum; j++)
            {
                if(pstCurrentProgInfo->stClosedCaption[j].u8IsDigitalCC)
                {
                    SAMPLE_CC_INFO_PRINT("%d: Closed Captioning 708\n", j);
                    cc_run_info.enCCDataType = MT_UNF_CC_DATA_TYPE_708;
                }
                else
                {
                    SAMPLE_CC_INFO_PRINT("%d: Closed Captioning 608\n", j);
                    cc_run_info.enCCDataType = MT_UNF_CC_DATA_TYPE_608;
                }
            }

            if(pstCurrentProgInfo->u16ClosedCaptionNum > 1)
            {
                MT_CC_PRINT("Please select Closed Captioning: ");
                scanf("%d", &Num);
                getchar();
                if(pstCurrentProgInfo->stClosedCaption[Num].u8IsDigitalCC)
                {
                    cc_run_info.enCCDataType = MT_UNF_CC_DATA_TYPE_708;
                }
                else
                {
                    cc_run_info.enCCDataType = MT_UNF_CC_DATA_TYPE_608;

                }
            }
        }

        s32Ret = MT_CCModeCCStart(cc_run_info.enCCDataType);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_CC_ERR_PRINT("failed to MT_CCModeCCStart\n");
            goto ERR13;
        }

        if(MT_UNF_CC_DATA_TYPE_ARIB == cc_run_info.enCCDataType)
        {
            /** Start data filtering */
            s32Ret = MT_CCModeStartDataFilter(pstCurrentProgInfo);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("failed to MT_CCModeStartDataFilter\n");
                goto ERR14;
            }
        }
        else
        {
            stVBICfg.enType = MT_UNF_DISP_VBI_TYPE_CC;

            /** Create VBI data channel */
            s32Ret = MT_UNF_DISP_CreateVBI(MT_UNF_DISPLAY0, &stVBICfg, &cc_run_info.hVBI);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("failed to MT_UNF_DISP_CreateVBI\n");
                goto ERR14;
            }

            /** CC data injection thread */
            pthread_create(&cc_run_info.usrRecvThread, NULL, UsrDataInject, NULL);
        }
    }

    /** Enter the instruction operation thread */
    (MT_VOID)MT_CCModeCmdTask(cc_run_info.hAvPlay, cc_run_info.pProgTbl, cc_run_info.enCCDataType);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    if(MT_UNF_CC_DATA_TYPE_ARIB == cc_run_info.enCCDataType)
    {
        /** Close the relevant DEMUX channel */
        (MT_VOID)CC_Data_Uninstall(cc_run_info.hCCData);

        /** The CC data acquisition module is deinitialized */
        (MT_VOID)CC_Data_DeInit();
    }
    else
    {
        g_bTaskQuit = MT_TRUE;

        /** Wait for the thread to end */
        pthread_join(cc_run_info.usrRecvThread, NULL);

        /** Destroy VBI data channel */
        (MT_VOID)MT_UNF_DISP_DestroyVBI(cc_run_info.hVBI);
    }

ERR14:
    (MT_VOID)MT_CCModeCCStop();
ERR13:
    /** CC module output deinitialization */
    (MT_VOID)CC_Output_DeInit(cc_run_info.hOutput);
ERR12:
    /** DeInitialize cc module */
    (MT_VOID)MT_UNF_CC_DeInit();
ERR11:
#ifndef MT_SAMPLE_APP
    /** Stop playing the show */
    (MT_VOID)MT_CCModeStopToPlay(cc_run_info.hAvPlay);
ERR10:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_CCModeAvplayDeInit(cc_run_info.hAvPlay, cc_run_info.hWin, cc_run_info.hSoundTrack);
ERR9:
    (MT_VOID)MTADP_Search_FreeAllPmt(cc_run_info.pProgTbl);
ERR8:
    (MT_VOID)MTADP_Search_DeInit();
ERR7:
    if(MT_INPUT_SIG_TYPE_FILE == cc_run_info.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;

        /** Wait for the thread to end */
        pthread_join(cc_run_info.stInjectTSThread, NULL);
    }
ERR6:
    (MT_VOID)MT_CCModeDmxDeinit();
ERR5:
    (MT_VOID)MTADP_Snd_DeInit();
ERR4:
    (MT_VOID)MTADP_VO_DeInit();
ERR3:
    if(MT_INPUT_SIG_TYPE_FILE != cc_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR2:
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&cc_run_info, 0xff, sizeof(cc_run_info));

    return s32Ret;
}
