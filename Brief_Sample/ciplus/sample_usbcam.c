/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
#include "pthread.h"
#include "mt_cmdline.h"
#include "mt_adp_pvr.h"
#include "mt_unf_pm.h"
#include "mt_unf_ir.h"
#include "mt_adp_config.h"
#include "mt_adp_str.h"
#include "usbcam_route_ts.h"
#include "ci_api_test.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_USBCAM_DEBUG
#define MT_USBCAM_PRINT   printf
#else
#define MT_USBCAM_PRINT
#endif

#define SAMPLE_USBCAM_FUNCTION_ENTER()            MT_USBCAM_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_USBCAM_FUNCTION_EXIT()             MT_USBCAM_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_USBCAM_FATAL_PRINT(fmt...)         MT_USBCAM_PRINT(" [FATAL] " fmt)
#define SAMPLE_USBCAM_ERR_PRINT(fmt...)           MT_USBCAM_PRINT(" [ERROR] " fmt)
#define SAMPLE_USBCAM_WARN_PRINT(fmt...)          MT_USBCAM_PRINT(" [WARN] "  fmt)
#define SAMPLE_USBCAM_INFO_PRINT(fmt...)          MT_USBCAM_PRINT(" [INFO] "  fmt)
#define SAMPLE_USBCAM_DBG_PRINT(fmt...)           MT_USBCAM_PRINT(" [DEBUG] " fmt)

#define SAMPLE_USBCAM_PRINT  printf

#define MT_TASK_RUN        1
#define MT_TASK_EXIT       2

#define MT_DVBC_PARAM_NUM 3
#define MT_J83B_PARAM_NUM 3
#define MT_DVBS_PARAM_NUM 5
#define MT_DVBT_PARAM_NUM 2

/*************************** Structure Definition ****************************/
typedef struct
{
    MT_HANDLE hAvPlay;
    MT_HANDLE hSoundTrack;
    MT_HANDLE hWin;

    PMT_COMPACT_TBL *pProgTbl;
    mt_nim_input_para_info sInputParam;
} mt_play_info;

/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static mt_s32 g_mt_optind = 1;
static mt_u32 g_cur_tuner_id = 0;
static mt_play_info g_playInfo;

const u8 IAtest_105_pmt_data[] = {
    //0x47,0x40,0x24,0x10,0x00,
    0x02, 0xB0, 0x20, 0x01, 0x05, 0xC1, 0x00, 0x00, 0xE4,
    0x10, 0xF0, 0x09, 0x65, 0x01, 0x80, 0x09, 0x04, 0x06,
    0x25, 0xE4, 0x12, 0x02, 0xE4, 0x10, 0xF0, 0x00, 0x04,
    0xE4, 0x11, 0xF0, 0x00, 0x9B, 0x52, 0xF2, 0xA3
};

const u8 IAtest_104_pmt_data[] = {
    //0x47, 0x40, 0x23, 0x11, 0x00,
    0x02, 0xB0, 0x20, 0x01, 0x04, 0xC1, 0x00, 0x00, 0xE2,
    0x30, 0xF0, 0x09, 0x65, 0x01, 0x81, 0x09, 0x04, 0x06,
    0x25, 0xE2, 0x32, 0x02, 0xE2, 0x30, 0xF0, 0x00, 0x04,
    0xE2, 0x31, 0xF0, 0x00, 0x9F, 0x19, 0x6F, 0x63
};

const u8 IAtest_103_pmt_data[] = {
    //0x47, 0x40, 0x22, 0x10, 0x00
    0x02, 0xB0, 0x20, 0x01, 0x03, 0xC1, 0x00, 0x00, 0xE2,
    0x20, 0xF0, 0x09, 0x65, 0x01, 0x02, 0x09, 0x04, 0x06,
    0x25, 0xE2, 0x22, 0x02, 0xE2, 0x20, 0xF0, 0x00, 0x04,
    0xE2, 0x21, 0xF0, 0x00, 0x5C, 0xBB, 0x09, 0x1F
};

const u8 IAtest_102_pmt_data[] = {
    //0x47,0x40,0x21,0x10,0x00
    0x02, 0xB0, 0x1D, 0x01, 0x02, 0xC1, 0x00, 0x00, 0xE2,
    0x10, 0xF0, 0x06, 0x09, 0x04, 0x06, 0x25, 0xE2, 0x12,
    0x02, 0xE2, 0x10, 0xF0, 0x00, 0x04, 0xE2, 0x11, 0xF0,
    0x00, 0x75, 0x29, 0xC9, 0x1D, 0xFF
};

const u8 IAtest_101_pmt_data[] = {
    //0x47, 0x40, 0x20, 0x10, 0x00,
    0x02, 0xB0, 0x17, 0x01, 0x01, 0xC1, 0x00, 0x00, 0xE2,
    0x00, 0xF0, 0x00, 0x02, 0xE2, 0x00, 0xF0, 0x00, 0x04,
    0xE2, 0x01, 0xF0, 0x00, 0xBD, 0x1D, 0xA2, 0x28, 0xFF
};

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

#ifdef MT_SAMPLE_APPs
MT_S32 MT_UsbCamMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/******************************* API declaration *****************************/

static const u8 *get_pmt_data(unsigned index)
{
    switch (index) {
    case 0:
        return IAtest_101_pmt_data;
        break;
    case 1:
        return IAtest_102_pmt_data;
        break;
    case 2:
        return IAtest_103_pmt_data;
        break;
    case 3:
        printf("update program NO.4 pmt\n");
        return IAtest_104_pmt_data;
        break;
    case 4:
        return IAtest_105_pmt_data;
        break;
    default:
        printf("index error\n");
        return NULL;
        break;
    }
}

static mt_s32 MT_UsbCamDvbcCheckParam(mt_nim_dvbc_input dvbc)
{
    SAMPLE_USBCAM_FUNCTION_ENTER();

    if (dvbc.freq < 45 || dvbc.freq > 862)
    {
        SAMPLE_USBCAM_ERR_PRINT("The frequency[%d] is not in range\n", dvbc.freq);
        return MT_FAILURE;
    }

    if (dvbc.sym_rate < 900 || dvbc.sym_rate > 7200)
    {
        SAMPLE_USBCAM_ERR_PRINT("The symbol rate [%d] is not in range\n", dvbc.sym_rate);
        return MT_FAILURE;
    }

    switch (dvbc.mod_type)
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
            SAMPLE_USBCAM_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    SAMPLE_USBCAM_FUNCTION_EXIT();
    return MT_SUCCESS;
}

static mt_s32 MT_UsbCamJ83bCheckParam(mt_nim_j83b_input j83b)
{
    if (j83b.freq < 45 || j83b.freq > 862)
    {
        SAMPLE_USBCAM_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if (j83b.sym_rate < 5057 || j83b.sym_rate > 7560)
    {
        SAMPLE_USBCAM_ERR_PRINT("srate error. srate = %d \n", j83b.sym_rate);
        SAMPLE_USBCAM_ERR_PRINT("The symbol rate is out of range.\n");
        return MT_FAILURE;
    }

    if (j83b.mod_type != 256 && j83b.mod_type != 64)
    {
        SAMPLE_USBCAM_ERR_PRINT("QAM error. QAM = %d \n", j83b.mod_type);
        SAMPLE_USBCAM_ERR_PRINT("QAM must be set 256 or 64.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_UsbCamDvbsCheckParam(mt_nim_dvbs_input dvbs)
{
    if ((dvbs.freq) > 4200 || (dvbs.freq) < 3000)
    {
        SAMPLE_USBCAM_ERR_PRINT("freq error. freq = %d \n", dvbs.freq);
        SAMPLE_USBCAM_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_UsbCamDvbtCheckParam(mt_nim_dvbt_input dvbt)
{
    if ((dvbt.freq) > 900 || (dvbt.freq) < 50)
    {
        SAMPLE_USBCAM_ERR_PRINT("freq error. freq = %d \n", dvbt.freq);
        SAMPLE_USBCAM_ERR_PRINT("freq must be more than 50 and less than 900.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_UsbCamCheckNimParam(mt_nim_input_para_info inputParam)
{
    mt_s32  ret = MT_SUCCESS;

    SAMPLE_USBCAM_FUNCTION_ENTER();

    switch (inputParam.nim_use)
    {
        case MT_NIM_TYPE_DVBC:
            ret = MT_UsbCamDvbcCheckParam(inputParam.dvbc);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_USBCAM_ERR_PRINT("call MT_NimPlayDvbcCheckParam err \n");
                return ret;
            }
            break;
        case MT_NIM_TYPE_J83B:
            ret = MT_UsbCamJ83bCheckParam(inputParam.j83b);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_USBCAM_ERR_PRINT("call MT_NimPlayJ83bCheckParam err \n");
                return ret;
            }
            break;
        case MT_NIM_TYPE_DVBT:
            ret = MT_UsbCamDvbtCheckParam(inputParam.dvbt);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_USBCAM_ERR_PRINT("call MT_NimPlayDvbsCheckParam err \n");
                return ret;
            }
            break;
        case MT_NIM_TYPE_DVBS_IN:
            ret = MT_UsbCamDvbsCheckParam(inputParam.dvbs_in);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_USBCAM_ERR_PRINT("call MT_NimPlayDvbsCheckParam dvbs_in err \n");
                return ret;
            }
            break;
        case MT_NIM_TYPE_DVBS_OUT:
            ret = MT_UsbCamDvbsCheckParam(inputParam.dvbs_out);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_USBCAM_ERR_PRINT("call MT_NimPlayDvbsCheckParam dvbs_out err \n");
                return ret;
            }
            break;
        default:
            SAMPLE_USBCAM_ERR_PRINT("find nim type err \n");
            break;
    }

    SAMPLE_USBCAM_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief Help information
@param[in]  name     Enter the value
@return::MT_VOID
@*/
static MT_VOID MT_UsbCamPrint_Help(MT_CHAR *name)
{
    SAMPLE_USBCAM_PRINT("Lack of parameters\n");
    SAMPLE_USBCAM_PRINT("\nUsage:\n");
    SAMPLE_USBCAM_PRINT("%s\n", name);
    SAMPLE_USBCAM_PRINT("    -c: input dvbc info(freq symbol_rate qam)\n");
    SAMPLE_USBCAM_PRINT("    -j: input j83b info(freq symbol_rate qam)\n");
    SAMPLE_USBCAM_PRINT("    -s: input dvbs_in info(freq symbol_rate 22k polar sig_type)\n");
    SAMPLE_USBCAM_PRINT("    -o: input dvbs_out info(freq symbol_rate 22k polar sig_type)\n");
    SAMPLE_USBCAM_PRINT("    -t: input dvbt info(freq band_width)\n");
#ifdef MT_SAMPLE_APP
    SAMPLE_USBCAM_PRINT("    -q: Exit the background\n");
#endif
    SAMPLE_USBCAM_PRINT("example:\n");
    SAMPLE_USBCAM_PRINT("    %s -c 314 6875 64 \n",name);
}


static MT_VOID MT_UsbCamPrintMenu(MT_U32 prog_num)
{

    SAMPLE_USBCAM_PRINT("\n 1 - %d : select the program \n", prog_num);

    SAMPLE_USBCAM_PRINT("     h : help \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_USBCAM_PRINT("     b : background run \n");
#endif
    SAMPLE_USBCAM_PRINT("     p : pause \n");
    SAMPLE_USBCAM_PRINT("     r : resume \n");
    SAMPLE_USBCAM_PRINT("     z : Channel Switch Mode \n");
    SAMPLE_USBCAM_PRINT("     s : signal strength \n");
    SAMPLE_USBCAM_PRINT("     l : signal quality \n");
    SAMPLE_USBCAM_PRINT("     d : check if audio dolby mono \n");
    SAMPLE_USBCAM_PRINT("     k : set unblank screen mode\n");
    SAMPLE_USBCAM_PRINT("     q : quit \n");
    SAMPLE_USBCAM_PRINT("USBCAM>> ");

}

static void MT_UsbCamFindNimPosition(MT_S32 argc, MT_CHAR *argv[], mt_nim_input_para_info *pInutParam)
{
    mt_s32 idx = 0;

    for (idx = 0; idx < argc; idx++) {
        if (!strcmp("-c", argv[idx])){
            pInutParam->dvbc.positon = idx;
        } else if (!strcmp("-j", argv[idx])) {
            pInutParam->j83b.positon = idx;
        } else if (!strcmp("-s", argv[idx])) {
            pInutParam->dvbs_in.positon = idx;
        } else if (!strcmp("-o", argv[idx])) {
            pInutParam->dvbs_out.positon = idx;
        } else if (!strcmp("-t", argv[idx])) {
            pInutParam->dvbt.positon = idx;
        } else {
        }
    }

    return;
}

static mt_s32 MT_UsbCamGetopt(int* ppara_num, int argc, char *argv[], char *opts)
{
    static mt_s32 sp = 1;
    mt_s32 c;
    mt_char *cp;
    mt_s32 para_num = 0;

    SAMPLE_USBCAM_FUNCTION_ENTER();

    if (sp == 1) {
        if (g_mt_optind >= argc)
            return EOF;
        else if (!strcmp(argv[g_mt_optind], "--")) {
            g_mt_optind++;
            return EOF;
        }
        else if(argv[g_mt_optind][0] != '-' || argv[g_mt_optind][1] == '\0')
        {
            return '?';
        }
    }

    c = argv[g_mt_optind][sp];
    if (c == ':' || (cp = strchr(opts, c)) == NULL) {
        //fprintf(stderr, ": illegal option -- %c\n", c);
        if (argv[g_mt_optind][++sp] == '\0') {
            g_mt_optind++;
        }
        sp = 1;
        return '?';
    }

    if (*++cp == ':') {
        g_mt_optind++;
        while (argv[g_mt_optind][0] != '-') {
            g_mt_optind++;
            para_num++;
            if (g_mt_optind >= argc) {
                break;
            }
        }

        sp = 1;
    } else {
        if (argv[g_mt_optind][++sp] == '\0') {
            sp = 1;
            g_mt_optind++;
        }
    }

    *ppara_num = para_num;

    SAMPLE_USBCAM_FUNCTION_EXIT();

    return c;
}

static mt_s32 MT_UsbCamSetCurUsedTunerId(mt_u32 tuner_id)
{
    g_cur_tuner_id = tuner_id;

    return MT_SUCCESS;
}

static mt_s32 MT_UsbCamGetCurUsedTunerId(mt_u32 *tuner_id)
{
    *tuner_id = g_cur_tuner_id;

    return MT_SUCCESS;
}

/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_UsbCamDmxInit(MT_VOID)
{
    MT_S32  ret = MT_SUCCESS;

    SAMPLE_USBCAM_FUNCTION_ENTER();

    ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_DMX_Init failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_USBCAM_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief Demux module deinitialization
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_VOID MT_UsbCamDmxDeinit(MT_VOID)
{
    SAMPLE_USBCAM_FUNCTION_ENTER();

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();

    SAMPLE_USBCAM_FUNCTION_EXIT();
}

/*!
@brief audio and video player initialization
@param[out] hWin                the input window handler
@param[out] phAvplay            Handle to AV player
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_UsbCamAvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin, MT_HANDLE *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    MT_HANDLE                hAvplay = 0;
    MT_HANDLE                hWin = 0;
    MT_HANDLE                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    SAMPLE_USBCAM_FUNCTION_ENTER();

    if(NULL == phAvplay || NULL == phWin || NULL == phSoundTrack)
    {
        SAMPLE_USBCAM_ERR_PRINT("The input address is empty!\n");
        return ret;
    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(ret != MT_SUCCESS)
    {
        SAMPLE_USBCAM_ERR_PRINT("MTADP_AVPlay_RegADecLib failed, ret = %x\n", ret);
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_Init failed, ret = %x\n", ret);
        return ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Defines the playing attributes of the AV player */
    AvplayAttr.u32DemuxId = DMX_1;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    /** Create AV player based on attributes */
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_Create failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR2;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, ret = %x\n", ret);
        goto ERROR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_SND_CreateTrack failed, ret = %x\n", ret);
        goto ERROR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_SND_Attach failed, ret = %x\n", ret);
        goto ERROR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MTADP_VO_CreatWin failed, ret = %x\n", ret);
        goto ERROR6;
    }

    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_VO_AttachWindow failed, ret = %x\n", ret);
        goto ERROR7;
    }

    /** Enable/disable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, ret = %x\n", ret);
        goto ERROR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    SAMPLE_USBCAM_FUNCTION_EXIT();

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
@param[in]  phAvplay            handle to AV player
@param[in]  hWin                Handle to window
@param[in]  phSoundTrack        Handle to sound track
@return::MT_VOID
@*/
static MT_VOID MT_UsbCamAvplayDeinit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
    SAMPLE_USBCAM_FUNCTION_ENTER();

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

    SAMPLE_USBCAM_FUNCTION_EXIT();
}

/*
@brief Initialize and open the tuner
@param[in] tuner_id, tuner port number
@return MT_SUCCESS
@return MT_FAILURE
*/
mt_s32 MT_UsbCam_Fe_Init(mt_nim_input_para_info sInputParam)
{
    MT_S32 ret = 0;
    mt_u32 tuner_id;
    mt_u32 nim_type;

    ret = mt_unf_fe_init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT(" mt_unf_fe_init failed.ret = 0x%x\n", ret);
        return ret;
    }

    nim_type = sInputParam.nim_use;
    if (nim_type == MT_NIM_TYPE_DVBC) {
        tuner_id = sInputParam.dvbc.tuner_id;
    } else if (nim_type == MT_NIM_TYPE_J83B) {
        tuner_id = sInputParam.j83b.tuner_id;
    } else if (nim_type == MT_NIM_TYPE_DVBT) {
        tuner_id = sInputParam.dvbt.tuner_id;
    } else if (nim_type == MT_NIM_TYPE_DVBS_IN) {
        tuner_id = sInputParam.dvbs_in.tuner_id;
    } else if (MT_NIM_TYPE_DVBS_OUT) {
        tuner_id = sInputParam.dvbs_out.tuner_id;
    } else {
        tuner_id = 0;
    }

    /* open Tuner*/
    SAMPLE_USBCAM_INFO_PRINT("tuner_id: 0x%x\n", tuner_id);
    ret = mt_unf_fe_open(tuner_id);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT(" mt_unf_fe_open failed.ret = 0x%x\n", ret);
        goto err;
    }

    return ret;
err:
    ret = mt_unf_fe_deinit();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT(" mt_unf_fe_deinit failed.ret = 0x%x\n",ret);
    }

    return ret;
}

/*
@brief DeInitialize and close the tuner
@param[in] tuner_id, tuner port number
@return MT_SUCCESS
@return MT_FAILURE
*/
mt_s32 MT_UsbCam_Fe_DeInit(mt_nim_input_para_info sInputParam)
{
    mt_s32 ret;
    mt_u32 tuner_id;
    mt_u32 nim_type;
    mt_nim_config_info nim_info;


    nim_type = sInputParam.nim_use;
    if (nim_type == MT_NIM_TYPE_DVBC) {
        tuner_id = sInputParam.dvbc.tuner_id;
        memset(&nim_info.dvbc, 0, sizeof(mt_nim_dvbc_info));
    } else if (nim_type == MT_NIM_TYPE_J83B) {
        tuner_id = sInputParam.j83b.tuner_id;
        memset(&nim_info.j83b, 0, sizeof(mt_nim_j83b_info));
    } else if (nim_type == MT_NIM_TYPE_DVBT) {
        tuner_id = sInputParam.dvbt.tuner_id;
        memset(&nim_info.dvbt, 0, sizeof(mt_nim_dvbt_info));
    } else if (nim_type == MT_NIM_TYPE_DVBS_IN) {
        tuner_id = sInputParam.dvbs_in.tuner_id;
        memset(&nim_info.dvbs_in, 0, sizeof(mt_nim_dvbs_info));
    } else if (MT_NIM_TYPE_DVBS_OUT) {
        tuner_id = sInputParam.dvbs_out.tuner_id;
        memset(&nim_info.dvbs_out, 0, sizeof(mt_nim_dvbs_info));
    } else {
        tuner_id = 0;
    }

    ret = mt_unf_fe_close(tuner_id);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT(" mt_unf_fe_close failed.ret = 0x%x\n", ret);
    }

    ret = mt_unf_fe_deinit();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT(" mt_unf_fe_deinit failed.ret = 0x%x\n", ret);
    }

    return ret;
}

/*!
@brief Set the PID of the AV player and set the encoder type.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_UsbCamSetAvplayPidAndCodecType(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_S32                           ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_U32                           PcrPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };
    MT_UNF_VCODEC_UNBLANK_E          unblank;

    SAMPLE_USBCAM_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_USBCAM_ERR_PRINT("The input address is empty\n");
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
        AudPid  = pProgInfo->AElementPid;
        u32AudType = pProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    if (u32AudType == HA_AUDIO_ID_PCM)
    {
        mt_s32 i = 0;
        pcm_info_t pcm = { 0 };
        for (i = 0; i < pProgInfo->AElementNum; i++)
        {
            if (pProgInfo->Audioinfo[i].u16AudioPid == AudPid) {
                break;
            }
        }

        pcm = pProgInfo->Audioinfo[i].pcm;
        MTADP_Set_AudPcmInfo(pcm);
    }

    PcrPid = pProgInfo->PcrPid;

    SAMPLE_USBCAM_INFO_PRINT("VidPid = %#x, AudPid = %#x\n", VidPid, AudPid);

    if(INVALID_TSPID != PcrPid)
    {
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_PCR_PID failed.\n");
            return MT_FAILURE;
        }
    }

    if(VidPid != INVALID_TSPID)
    {
        MTADP_Get_VcodeUnblank(&unblank);
        /** Get the video properties of the AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, ret = %x\n", ret);
            return ret;
        }

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
        VdecAttr.enUnBlank = unblank;
        VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VdecAttr.u32ErrCover = 100;
        VdecAttr.s32CtrlOptions = 0;
        VdecAttr.u32Priority = 3;

        /** Set the video properties of the AV player */
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);

        /** Set the video PID properties of AV player */
        ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &VidPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("Set video properties or video PID property failed, ret = %x\n", ret);
            return ret;
        }
    }

    if(AudPid != INVALID_TSPID)
    {
        /** Set audio decoder properties */
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);

        /** Set the audio PID properties of AV player */
        ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("Setting the decoding mode or audio PID property failed, ret = %x\n", ret);
            return ret;
        }
    }

    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        /** Set the audio and video synchronization properties of AV player */
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (MT_VOID *)&DmxAvsync);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, ret = %x\n", ret);
            return ret;
        }
    }

    SAMPLE_USBCAM_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief port The port is bound to demux
@param[in]  sInputParam       input param
@param[in]  idx           nim type idx
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static mt_s32 MT_UsbCamNimConnect(mt_nim_input_para_info sInputParam)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 nim_type;

    nim_type = sInputParam.nim_use;
    if (MT_NIM_TYPE_DVBC == nim_type)
    {
        mt_nim_dvbc_input dvbc = sInputParam.dvbc;

        (MT_VOID)MT_UsbCamSetCurUsedTunerId(dvbc.tuner_id);
        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, dvbc.port);
        ret = MTADP_Fe_Connect_Dvbc(dvbc.tuner_id, dvbc.freq, dvbc.sym_rate, dvbc.mod_type);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MTADP_Fe_Connect_Dvbc failed.\n");
            return ret;
        }
    }
    else if (MT_NIM_TYPE_J83B == nim_type)
    {
        mt_nim_j83b_input j83b = sInputParam.j83b;

        (MT_VOID)MT_UsbCamSetCurUsedTunerId(j83b.tuner_id);
        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, j83b.port);
        ret = MTADP_Fe_Connect_J83b(j83b.tuner_id, j83b.freq, j83b.sym_rate, j83b.mod_type);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MTADP_Fe_Connect_J83b failed.\n");
            return ret;
        }
    }
    else if (MT_NIM_TYPE_DVBT == nim_type)
    {
        mt_nim_dvbt_input dvbt = sInputParam.dvbt;

        (MT_VOID)MT_UsbCamSetCurUsedTunerId(dvbt.tuner_id);
        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, dvbt.port);
        ret = MTADP_Fe_Connect_Dvbt(dvbt.tuner_id, dvbt.freq, dvbt.bandwidth);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MTADP_Fe_Connect_Dvbt failed.\n");
            return ret;
        }
    }
    else if (MT_NIM_TYPE_DVBS_IN == nim_type)
    {
        mt_nim_dvbs_input dvbs_in = sInputParam.dvbs_in;

        (MT_VOID)MT_UsbCamSetCurUsedTunerId(dvbs_in.tuner_id);
        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, dvbs_in.port);
        ret = MTADP_Fe_Connect_Dvbs(dvbs_in.tuner_id, dvbs_in.freq, dvbs_in.sym_rate, dvbs_in.onoff_22k, dvbs_in.polar, dvbs_in.dvbs_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MTADP_Fe_Connect_Dvbs_In failed.\n");
            return ret;
        }
    }
    else if (MT_NIM_TYPE_DVBS_OUT == nim_type)
    {
        mt_nim_dvbs_input dvbs_out = sInputParam.dvbs_out;

        (MT_VOID)MT_UsbCamSetCurUsedTunerId(dvbs_out.tuner_id);
        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, dvbs_out.port);
        ret = MTADP_Fe_Connect_Dvbs(dvbs_out.tuner_id, dvbs_out.freq, dvbs_out.sym_rate, dvbs_out.onoff_22k, dvbs_out.polar, dvbs_out.dvbs_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MTADP_Fe_Connect_Dvbs_Out failed.\n");
            return ret;
        }
    }
    else
    {
    }

    return MT_SUCCESS;
}

/*!
@brief start the AV playback into the start state
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_UsbCamStarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_USBCAM_FUNCTION_ENTER();

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_UsbCamSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UsbCamSetAvplayPidAndCodecType fail! \n");
        return ret;
    }

    if(pProgInfo->AElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_USBCAM_INFO_PRINT("has no audio info \n");
    }

    if(pProgInfo->VElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_USBCAM_INFO_PRINT("has no vide0 info \n");
    }

    if((enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        /** Set the frame rate parameter of AV player, enable vo frame rate detect */
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("Set frame to VO is failed, ret = %x\n", ret);
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("Get avplay sync attr is failed, ret = %x\n", ret);
            return ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("Set avplay sync attr is failed, ret = %x\n", ret);
            return ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    ret  =MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_Start failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_USBCAM_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief stop AV playback into the stop state
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_UsbCamStopToPlay(MT_HANDLE hAvplay, MT_UNF_AVPLAY_STOP_MODE_E enmode)
{
    MT_U32 ret = MT_FAILURE;
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    SAMPLE_USBCAM_FUNCTION_ENTER();

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    MT_USBCAM_PRINT("stop live play ...\n");
    option.enMode = enmode;
    option.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_Stop failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_USBCAM_FUNCTION_EXIT();

    return MT_SUCCESS;
}

static MT_VOID MT_UsbCamExit(MT_VOID)
{
    MT_UNF_VCODEC_UNBLANK_E unblank;

    SAMPLE_USBCAM_FUNCTION_ENTER();
#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    (MT_VOID)MT_UsbCamStopToPlay(g_playInfo.hAvPlay, MT_UNF_AVPLAY_STOP_MODE_BLACK);

    (MT_VOID)MT_UsbCamAvplayDeinit(g_playInfo.hAvPlay, g_playInfo.hWin, g_playInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_DeInit();

    /** Demux module deinitialization */
    (MT_VOID)MT_UsbCamDmxDeinit();


    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MT_UsbCam_Fe_DeInit(g_playInfo.sInputParam);

    memset(&g_playInfo, 0, sizeof(g_playInfo));
    g_bTaskQuit = MT_TRUE;

    MTADP_Get_VcodeUnblank(&unblank);
    if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
    {
        MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
    }
    SAMPLE_USBCAM_FUNCTION_EXIT();
}

static MT_VOID MT_UsbCamReSetOptInd(MT_VOID)
{
    g_mt_optind = 1;

    return;
}

static mt_s32 MT_UsbCamCheckParamNum(mt_s32 chr, mt_s32 para_num)
{
    switch (chr)
    {
        case 'c':
            if (MT_DVBC_PARAM_NUM != para_num)
            {
                SAMPLE_USBCAM_ERR_PRINT("[dvbc|%d|%d] input param number err, exp: -c 314 6875 64 \n", MT_DVBC_PARAM_NUM, para_num);
                return MT_FAILURE;
            }
            break;
        case 'j':
            if (MT_J83B_PARAM_NUM != para_num)
            {
                SAMPLE_USBCAM_ERR_PRINT("[j83b|%d|%d] input param number err, exp: -j 474 5361 256 \n", MT_J83B_PARAM_NUM, para_num);
                return MT_FAILURE;
            }
            break;
        case 's':
            if (MT_DVBS_PARAM_NUM != para_num)
            {
                SAMPLE_USBCAM_ERR_PRINT("[dvbs_in|%d|%d] input param number err, exp: -s 3840 27500 1 0 2 \n", MT_DVBS_PARAM_NUM, para_num);
                return MT_FAILURE;
            }
            break;
        case 'o':
            if (MT_DVBS_PARAM_NUM != para_num)
            {
                SAMPLE_USBCAM_ERR_PRINT("[dvbs_out|%d|%d] input param number err, exp: -o 3840 27500 1 0 2 \n", MT_DVBS_PARAM_NUM, para_num);
                return MT_FAILURE;
            }
            break;
        case 't':
            if (MT_DVBT_PARAM_NUM != para_num)
            {
                SAMPLE_USBCAM_ERR_PRINT("[dvbt|%d|%d] input param number err, exp: -t 585 8 \n", MT_DVBT_PARAM_NUM, para_num);
                return MT_FAILURE;
            }
            break;
        default:
            break;
    }

    return MT_SUCCESS;
}

/*!
@brief gets the external input parameters
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::MT_VOID
@*/
static MT_S32 MT_UsbCamParase_args(MT_S32 argc, MT_CHAR *argv[], mt_nim_input_para_info *pInutParam)
{
    mt_s32 opt = 0;
    mt_s32 para_num = 0;

    SAMPLE_USBCAM_FUNCTION_ENTER();

    if (argc < 4 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_UsbCamPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    /*find out dvbc/dvbt/dvbs/j83b start positon in the input params.*/
    MT_UsbCamFindNimPosition(argc, argv, pInutParam);
    /*reset g_mt_optind to initital value.*/
    MT_UsbCamReSetOptInd();
    while((opt = MT_UsbCamGetopt(&para_num, argc, argv, ":?hHc:j:s:o:t:q")) != -1)
    {
        if (MT_UsbCamCheckParamNum(opt ,para_num))
        {
            return MT_FAILURE;
        }

        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_UsbCamPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    MT_UsbCamExit();
                }
                return MT_TASK_EXIT;
            case 'c':
                pInutParam->dvbc.freq = strtol(argv[pInutParam->dvbc.positon + 1], 0, 0);
                pInutParam->dvbc.sym_rate = strtol(argv[pInutParam->dvbc.positon + 2], 0, 0);
                pInutParam->dvbc.mod_type = strtol(argv[pInutParam->dvbc.positon + 3], 0, 0);
                pInutParam->dvbc.tuner_id = MT_NIM_TUNER_TYPE_DVBC;
                pInutParam->dvbc.port = MT_UNF_DMX_PORT_TSI_1;
                pInutParam->nim_use = MT_NIM_TYPE_DVBC;
                SAMPLE_USBCAM_PRINT("[dvbc] freq:%d sym_rate:%d mode_type:%d \n", pInutParam->dvbc.freq,
                    pInutParam->dvbc.sym_rate, pInutParam->dvbc.mod_type);
                break;

            case 'j':
                pInutParam->j83b.freq = strtol(argv[pInutParam->j83b.positon + 1], 0, 0);
                pInutParam->j83b.sym_rate = strtol(argv[pInutParam->j83b.positon + 2], 0, 0);
                pInutParam->j83b.mod_type = strtol(argv[pInutParam->j83b.positon + 3], 0, 0);
                pInutParam->j83b.tuner_id = MT_NIM_TUNER_TYPE_J83B;
                pInutParam->nim_use = MT_NIM_TYPE_J83B;
                pInutParam->j83b.port = MT_UNF_DMX_PORT_TSI_1;
                SAMPLE_USBCAM_PRINT("[j83b] freq:%d sym_rate:%d mode_type:%d \n", pInutParam->j83b.freq,
                    pInutParam->j83b.sym_rate, pInutParam->j83b.mod_type);
                break;

            case 's':
                pInutParam->dvbs_in.freq = strtol(argv[pInutParam->dvbs_in.positon + 1], 0, 0);
                pInutParam->dvbs_in.sym_rate = strtol(argv[pInutParam->dvbs_in.positon + 2], 0, 0);
                pInutParam->dvbs_in.onoff_22k = strtol(argv[pInutParam->dvbs_in.positon + 3], 0, 0);
                pInutParam->dvbs_in.polar = strtol(argv[pInutParam->dvbs_in.positon + 4], 0, 0);
                pInutParam->dvbs_in.dvbs_type = strtol(argv[pInutParam->dvbs_in.positon + 5], 0, 0);
                pInutParam->dvbs_in.tuner_id = MT_NIM_TUNER_TYPE_DVBS_IN;
                pInutParam->dvbs_in.port = MT_UNF_DMX_PORT_TSI_0;
                pInutParam->nim_use = MT_NIM_TYPE_DVBS_IN;
                SAMPLE_USBCAM_PRINT("[dvbs_in] freq:%d sym_rate:%d 22k:%d polar:%d dvbs_type:%d\n", pInutParam->dvbs_in.freq, pInutParam->dvbs_in.sym_rate,
                    pInutParam->dvbs_in.onoff_22k, pInutParam->dvbs_in.polar, pInutParam->dvbs_in.dvbs_type);
                break;

            case 'o':
                pInutParam->dvbs_out.freq = strtol(argv[pInutParam->dvbs_out.positon + 1], 0, 0);
                pInutParam->dvbs_out.sym_rate = strtol(argv[pInutParam->dvbs_out.positon + 2], 0, 0);
                pInutParam->dvbs_out.onoff_22k = strtol(argv[pInutParam->dvbs_out.positon + 3], 0, 0);
                pInutParam->dvbs_out.polar = strtol(argv[pInutParam->dvbs_out.positon + 4], 0, 0);
                pInutParam->dvbs_out.dvbs_type = strtol(argv[pInutParam->dvbs_out.positon + 5], 0, 0);
                pInutParam->dvbs_out.tuner_id = MT_NIM_TUNER_TYPE_DVBS_OUT;
                pInutParam->dvbs_out.port = MT_UNF_DMX_PORT_TSI_3;
                pInutParam->nim_use = MT_NIM_TYPE_DVBS_OUT;
                SAMPLE_USBCAM_PRINT("[dvbs_out] freq:%d sym_rate:%d 22k:%d polar:%d dvbs_type:%d\n", pInutParam->dvbs_out.freq, pInutParam->dvbs_out.sym_rate,
                    pInutParam->dvbs_out.onoff_22k, pInutParam->dvbs_out.polar, pInutParam->dvbs_out.dvbs_type);
                break;

            case 't':
                pInutParam->dvbt.freq = strtol(argv[pInutParam->dvbt.positon + 1], 0, 0);
                pInutParam->dvbt.bandwidth = strtol(argv[pInutParam->dvbt.positon + 2], 0, 0);
                pInutParam->dvbt.tuner_id = MT_NIM_TUNER_TYPE_DVBT;
                pInutParam->dvbt.port = MT_UNF_DMX_PORT_TSI_2;
                pInutParam->nim_use = MT_NIM_TYPE_DVBT;
                SAMPLE_USBCAM_PRINT("[dvbt] freq:%d bandwidth:%d\n", pInutParam->dvbt.freq, pInutParam->dvbt.bandwidth);
                break;

            default:
                (MT_VOID)MT_UsbCamPrint_Help(argv[0]);
                return MT_FAILURE;
         }
    }

    SAMPLE_USBCAM_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief Task action commands
@param[in]  hAvPlay     handle to AV player
@return::MT_VOID
@*/
static MT_VOID MT_UsbCamCmdTask(MT_HANDLE hAvPlay,
    mt_nim_input_para_info sInputParam, PMT_COMPACT_TBL *pProgTbl)
{
    mt_s32             ret = MT_FAILURE;
    mt_s32             s32ProgNum = 1;
    MT_CHAR            inputCmd[32] = { 0 };
    PMT_COMPACT_PROG   *pstCurrentProgInfo = NULL;
    MT_UNF_AVPLAY_STOP_MODE_E enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    mt_u32 strength = 0;
    mt_s32 agc = 0;
    mt_u32 quality = 0;
    mt_s32 accurate_snr = 0;
    mt_u32 snr = 0;
    mt_u32 tuner_id = 0;
    mt_s32 progNum = 0;
    mt_s32 unblank = MT_UNF_VCODEC_UNBLANK_STABLE;
    usbcam_play_context_s *usbcam_context = NULL;

    if(MT_INVALID_HANDLE == hAvPlay)
    {
        SAMPLE_USBCAM_ERR_PRINT("The input handle is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }

    while(1)
    {
        (MT_VOID)MT_UsbCamPrintMenu(pProgTbl->prog_num);

        fgets((MT_CHAR *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if ('q' == inputCmd[0])
        {
            SAMPLE_USBCAM_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
            {
                MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
            }
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_USBCAM_INFO_PRINT("NimPlay play in back!\n");
            break;
        }

#endif
        else if ('p' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Pause(hAvPlay, NULL);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_Pause failed\n");
            }
        }
        else if ('r' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Resume(hAvPlay, NULL);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_USBCAM_ERR_PRINT("MT_UNF_AVPLAY_Resume failed\n");
            }
        }
        else if ('z' == inputCmd[0])
        {
            if (enmode == MT_UNF_AVPLAY_STOP_MODE_BLACK)
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_STILL;
                SAMPLE_USBCAM_INFO_PRINT("Set mode to Freeze \n");
            }
            else
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
                SAMPLE_USBCAM_INFO_PRINT("Set mode to black \n");
            }

        }
        else if ('s' == inputCmd[0])
        {
            MT_UsbCamGetCurUsedTunerId(&tuner_id);
            memset(&strength, 0, sizeof(strength));
            memset(&agc, 0, sizeof(agc));
            ret = mt_unf_fe_get_signal_strength(tuner_id, &strength);
            ret = mt_unf_fe_get_agc(tuner_id, 0, &agc);

            SAMPLE_USBCAM_PRINT("\t Signal strength = %d, agc = %d\n", strength, agc);
        }
        else if ('l' == inputCmd[0])
        {
            MT_UsbCamGetCurUsedTunerId(&tuner_id);
            memset(&snr, 0, sizeof(snr));
            memset(&quality, 0, sizeof(quality));
            memset(&accurate_snr, 0, sizeof(accurate_snr));
            ret = mt_unf_fe_get_signal_quality(tuner_id, &quality);
            ret = mt_unf_fe_get_snr(tuner_id, &snr);
            ret = mt_unf_fe_get_accurate_snr(tuner_id, &accurate_snr);

            SAMPLE_USBCAM_PRINT("\t Signal quality = %d, snr = %d accurate_snr:%02d.%03d\n",
                quality, snr, accurate_snr/1000, accurate_snr%1000);
        }
        else if (inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            ret = usbcam_route_ts_stop();
            /** Stop AV playback into the stop state */
            ret = MT_UsbCamStopToPlay(hAvPlay, enmode);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_USBCAM_ERR_PRINT("MT_UsbCamStopToPlay failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
            }

            progNum = atoi(inputCmd);
            if (progNum > 0 && progNum <= pProgTbl->prog_num)
            {
                pstCurrentProgInfo = pProgTbl->proginfo + ((progNum - 1) % pProgTbl->prog_num);
                printf("cur program num = %d,", progNum);
                printf("vpid = %x,apid = %x\n",pstCurrentProgInfo->VElementPid, pstCurrentProgInfo->AElementPid);
                const u8 *pmt_data = get_pmt_data((progNum - 1));
                stb_ci_update_pmt(pmt_data);

                /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
                SAMPLE_USBCAM_INFO_PRINT("===== Start play ProgNum: %d \n", progNum);
                // restore ac4    attr info.
                MTADP_AUD_RestoreAc4PlayAttrInfo(hAvPlay);
                
                ret = MT_UsbCamStarToPlay(hAvPlay, pstCurrentProgInfo);
                if (MT_SUCCESS != ret)
                {
                    SAMPLE_USBCAM_ERR_PRINT("Switching shows failed\n");
                }
                ret = usbcam_route_ts_start();
            }
            else
            {
                SAMPLE_USBCAM_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
                continue;
            }

#ifdef MT_SAMPLE_APP
            ret = MTADP_Set_Current_Info(pstCurrentProgInfo);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_USBCAM_ERR_PRINT(" MTADP_Set_Current_Info failed.\n");
            }
#endif
        }
        else if ('d' == inputCmd[0])
        {
            MT_UNF_AUDIOTRACK_ATTR_S trackAttr;
            memset(&trackAttr, 0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
            ret = MT_UNF_SND_GetTrackAttr(g_playInfo.hSoundTrack, &trackAttr);
            if (MT_SUCCESS == ret && MT_FALSE != trackAttr.dolby_dd_ddp && MT_TRUE == trackAttr.dolby_dualmono)
            {
                SAMPLE_USBCAM_INFO_PRINT("Audio dolby info: dolby[%d] Dual-Mono [1+1].\n", trackAttr.dolby_dd_ddp);
            }
        }
        else if ('k' == inputCmd[0])
        {
            SAMPLE_USBCAM_PRINT("input unblank mode(0:fast 1:stable 2:sync):");
            scanf("%d", &unblank);
            getchar();
            SAMPLE_USBCAM_PRINT("unblank: %d \n", unblank);
            unblank = unblank % MT_UNF_VCODEC_UNBLANK_BUTT;
            MTADP_Set_VcodeUnblank(unblank);
        }
        else if ('h' == inputCmd[0])
        {
            SAMPLE_USBCAM_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}

/******************************usbcam normal play  end **************************/
static int ci_callback_func(ci_cb_e event, unsigned long para1, u32 para2)
{
    const u8 *pmt_data = NULL;
    printf("*****event %d happen*****\n", event);
    switch (event) {
    case CI_MENU_SHOW:
        break;
    case CI_MENU_CLOSE:
        break;
    case CI_DATA_UNRECOGNIZE:
        break;
    case CI_CARD_INSERT:
        printf("usbcam insert\n");
        break;
    case CI_CARD_READY:
        printf("usbcam card ready\n");
        break;
    case CI_CARD_REMOVE:
        break;
    case CI_CA_MODULE_UP:
        break;
    default:
        break;
    }

    return 0;
}

static void MT_UsbCamCiStackInit(mt_nim_input_para_info sInputParam)
{
    usbcam_init_parm_t param = {0};
    mt_u32 nim_type;
    MT_UNF_DMX_PORT_E tuner_port;

    ci_module_init();
    ci_api_set_callback(ci_callback_func);

    nim_type = sInputParam.nim_use;
    if (nim_type == MT_NIM_TYPE_DVBC) {
        tuner_port = sInputParam.dvbc.port;
    } else if (nim_type == MT_NIM_TYPE_J83B) {
        tuner_port = sInputParam.j83b.port;
    } else if (nim_type == MT_NIM_TYPE_DVBT) {
        tuner_port = sInputParam.dvbt.port;
    } else if (nim_type == MT_NIM_TYPE_DVBS_IN) {
        tuner_port = sInputParam.dvbs_in.port;
    } else if (MT_NIM_TYPE_DVBS_OUT) {
        tuner_port = sInputParam.dvbs_out.port;
    } else {
        tuner_port = 0;
    }

    param.all_prog_record = 1;
    param.ci_dmxid = DMX_1;
    param.rec_dmxid = DMX_0;
    param.tuner_port = tuner_port;
    param.tsbuf_port = MT_UNF_DMX_PORT_RAM_0;
    param.recbuf_size = 16*1024*1024;
    usbcam_route_param_init(&param);
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_UsbCamMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    mt_s32            ret = MT_SUCCESS;
    usbcam_play_context_s *usbcam_context = NULL;

    SAMPLE_USBCAM_FUNCTION_ENTER();

    /** Get the parameters */
    ret = MT_UsbCamParase_args(argc, argv, &g_playInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_USBCAM_ERR_PRINT("Parase args err. stop window.\n");
        memset(&g_playInfo.sInputParam, 0, sizeof(mt_nim_input_para_info));
        return MT_FAILURE;
    }
    else if (MT_TASK_EXIT == ret)
    {
        SAMPLE_USBCAM_INFO_PRINT("Recv stop command. stop window.\n");
        return MT_SUCCESS;
    }

    if (g_bTaskQuit == MT_TRUE)
    {
        ret = MT_UsbCamCheckNimParam(g_playInfo.sInputParam);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MT_NimPlayCheckNimParam failed.\n");
            memset(&g_playInfo.sInputParam, 0, sizeof(mt_nim_input_para_info));
            return MT_FAILURE;
        }
		
		ret = mt_sys_init();
		if (MT_SUCCESS != ret)
		{
			SAMPLE_USBCAM_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
			return MT_FAILURE;
		}


#ifndef MT_SAMPLE_APP
        
        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", ret);
            goto ERR0;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", ret);
            goto ERR1;
        }
#endif

        ret = MT_UsbCam_Fe_Init(g_playInfo.sInputParam);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MT_UsbCam_Fe_Init failed.\n");
            goto ERR2;
        }

    
		/** Demux initializes and retrieves the PMT and PAT tables in TS */
        ret = MT_UsbCamDmxInit();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MT_UsbCamDmxInit failed, ret = %x\n", ret);
            goto ERR3;
        }

		MT_UsbCamCiStackInit(g_playInfo.sInputParam);
        usleep(500*1000);
		ret = usbcam_route_ts_init();
		if (ret != MT_SUCCESS) {
			SAMPLE_USBCAM_ERR_PRINT("usbcam_route_ts_init failed.\n");
            goto ERR4;
		}

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MTADP_VO_Init failed.\n");
            goto ERR4;
        }

        ret = MTADP_Snd_Init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MTADP_Snd_Init failed.\n");
            goto ERR5;
        }

        ret = MT_UsbCamNimConnect(g_playInfo.sInputParam);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MT_UsbCamNimConnect failed.\n");
            goto ERR6;
        }

        /** Get the PMT table */
        (MT_VOID)MTADP_Search_Init();
        ret = MTADP_Search_GetAllPmt(DMX_0, &g_playInfo.pProgTbl);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
            goto ERR6;
        }

        usbcam_context = get_usbcam_context();
        if (NULL == usbcam_context) {
            SAMPLE_USBCAM_ERR_PRINT("call get_usbcam_context fail.\n");
            goto ERR7;
        }

        usbcam_context->program_tab = (void *)g_playInfo.pProgTbl;
        /** audio and video player initialization */
        ret = MT_UsbCamAvplayInit(&g_playInfo.hAvPlay, &g_playInfo.hWin, &g_playInfo.hSoundTrack);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MT_UsbCamAvplayInit failed, ret = %x\n", ret);
            goto ERR7;
        }

#ifdef MT_SAMPLE_APP
        avplayHandle.hAvPlay = g_playInfo.hAvPlay;
        avplayHandle.hSoundTrack = g_playInfo.hSoundTrack;
        avplayHandle.hWin = g_playInfo.hWin;
#endif

        /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
        ret = MT_UsbCamStarToPlay(g_playInfo.hAvPlay, g_playInfo.pProgTbl->proginfo);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MT_UsbCamStarToPlay failed, ret = %x\n", ret);
            goto ERR8;
        }

        ret = ci_start_play();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_USBCAM_ERR_PRINT("MT_UsbCamStarToPlay failed, ret = %x\n", ret);
            goto ERR9;
        }

        g_bTaskQuit = MT_FALSE;
    }

    (MT_VOID)MT_UsbCamCmdTask(g_playInfo.hAvPlay, g_playInfo.sInputParam, g_playInfo.pProgTbl);

    if (MT_TRUE != g_bTaskQuit)
    {
        return MT_TASK_RUN;
    }

    (MT_VOID)ci_stop_play();

ERR9:
    (MT_VOID)MT_UsbCamStopToPlay(g_playInfo.hAvPlay, MT_UNF_AVPLAY_STOP_MODE_BLACK);
ERR8:
    (MT_VOID)MT_UsbCamAvplayDeinit(g_playInfo.hAvPlay, g_playInfo.hWin, g_playInfo.hSoundTrack);
ERR7:
    if (NULL != g_playInfo.pProgTbl){
        MTADP_Search_FreeAllPmt(g_playInfo.pProgTbl);
        g_playInfo.pProgTbl = NULL;
    }
ERR6:
    (MT_VOID)MTADP_Search_DeInit();
    (MT_VOID)MTADP_Snd_DeInit();
ERR5:
    (MT_VOID)MTADP_VO_DeInit();
	usbcam_route_ts_uninit();
ERR4:
    //(MT_VOID)ci_module_uninit();   CAM is no deinit operation.
    MT_UsbCamDmxDeinit();
ERR3:
    (MT_VOID)MT_UsbCam_Fe_DeInit(g_playInfo.sInputParam);
ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();
#endif

    g_bTaskQuit = MT_TRUE;
    memset(&g_playInfo, 0, sizeof(g_playInfo));

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
#endif

    return ret;
}
