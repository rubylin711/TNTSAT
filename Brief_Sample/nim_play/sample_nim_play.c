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
#include "mt_cmdline.h"
#include "mt_adp_str.h"
#ifdef CFG_MT_SAMPLE_NAGRA
#include "mt_unf_flash.h"
#include "mt_unf_hdcp.h"
#endif
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_NIM_PLAY_DEBUG
#define MT_NIM_PLAY_PRINT   printf
#else
#define MT_NIM_PLAY_PRINT
#endif

#define SAMPLE_NIM_PLAY_FUNCTION_ENTER()            MT_NIM_PLAY_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_NIM_PLAY_FUNCTION_EXIT()             MT_NIM_PLAY_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_NIM_PLAY_FATAL_PRINT(fmt...)         MT_NIM_PLAY_PRINT(" [FATAL] " fmt)
#define SAMPLE_NIM_PLAY_ERR_PRINT(fmt...)           MT_NIM_PLAY_PRINT(" [ERROR] " fmt)
#define SAMPLE_NIM_PLAY_WARN_PRINT(fmt...)          MT_NIM_PLAY_PRINT(" [WARN] "  fmt)
#define SAMPLE_NIM_PLAY_INFO_PRINT(fmt...)          MT_NIM_PLAY_PRINT(" [INFO] "  fmt)
#define SAMPLE_NIM_PLAY_DBG_PRINT(fmt...)           MT_NIM_PLAY_PRINT(" [DEBUG] " fmt)

#define SAMPLE_NIM_PLAY_PRINT  printf

#define MT_TASK_RUN        1
#define MT_TASK_EXIT       2

#define MT_DVBC_PARAM_NUM 3
#define MT_J83B_PARAM_NUM 3
#define MT_DVBS_PARAM_NUM 5
#define MT_DVBT_PARAM_NUM 2

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#define KEY_STANDBY_STR_OLD 0xf50a7f80

#define KEY_STANDBY_STR_NEW 0xb748fd01
#elif defined CONFIG_MT_CHIP_SYMPHONY6
#define KEY_STANDBY_STR_OLD 0x800a

#define KEY_STANDBY_STR_NEW 0x1fd48
#endif
/*************************** Structure Definition ****************************/
typedef struct
{
    mt_nim_input_para_info sInputParam;
    mt_s32 curtp;
} mt_nim_input_param_s;

/********************** Global Variable declaration **************************/
static MT_BOOL    g_bTaskQuit = MT_TRUE;
static MT_BOOL    g_bStrTaskQuit = MT_TRUE;
static pthread_t  g_StrThd;
static mt_s32 g_mt_optind = 1;
static mt_u32 g_cur_tuner_id = 0;

#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

#ifdef MT_SAMPLE_APP
MT_S32 MT_NimPlayMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/******************************* API declaration *****************************/

static mt_s32 MT_NimPlayDvbcCheckParam(mt_nim_dvbc_input dvbc)
{
    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

    if (dvbc.freq < 45 || dvbc.freq > 862)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("The frequency[%d] is not in range\n", dvbc.freq);
        return MT_FAILURE;
    }

    if (dvbc.sym_rate < 900 || dvbc.sym_rate > 7200)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("The symbol rate [%d] is not in range\n", dvbc.sym_rate);
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
            SAMPLE_NIM_PLAY_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();
    return MT_SUCCESS;
}

static mt_s32 MT_NimPlayJ83bCheckParam(mt_nim_j83b_input j83b)
{
    if (j83b.freq < 45 || j83b.freq > 862)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if (j83b.sym_rate < 5057 || j83b.sym_rate > 7560)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("srate error. srate = %d \n", j83b.sym_rate);
        SAMPLE_NIM_PLAY_ERR_PRINT("The symbol rate is out of range.\n");
        return MT_FAILURE;
    }

    if (j83b.mod_type != 256 && j83b.mod_type != 64)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("QAM error. QAM = %d \n", j83b.mod_type);
        SAMPLE_NIM_PLAY_ERR_PRINT("QAM must be set 256 or 64.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_NimPlayDvbsCheckParam(mt_nim_dvbs_input dvbs)
{
    if ((dvbs.freq) > 4200 || (dvbs.freq) < 3000)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("freq error. freq = %d \n", dvbs.freq);
        SAMPLE_NIM_PLAY_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 MT_NimPlaysDvbtCheckParam(mt_nim_dvbt_input dvbt)
{
    if ((dvbt.freq) > 900 || (dvbt.freq) < 50)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("freq error. freq = %d \n", dvbt.freq);
        SAMPLE_NIM_PLAY_ERR_PRINT("freq must be more than 50 and less than 900.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_NimPlayCheckNimParam(mt_nim_input_para_info inputParam)
{
    mt_s32  ret = MT_SUCCESS;

    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

    if ((MT_NIM_TYPE_DVBC & inputParam.nim_use) && (MT_NIM_TYPE_J83B & inputParam.nim_use))
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("j83b and dvbc use conflict !!!\n");
        return MT_FAILURE;
    }

    if ((MT_NIM_TYPE_DVBS_IN & inputParam.nim_use) && (MT_NIM_TYPE_J83B & inputParam.nim_use))
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("j83b and dvbs-in use conflict !!!\n");
        return MT_FAILURE;
    }

    for (mt_s32 i = 0; i < inputParam.nim_total; i++)
    {
        if (MT_NIM_TYPE_DVBC == inputParam.nim_type_arr[i])
        {
            ret = MT_NimPlayDvbcCheckParam(inputParam.dvbc);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("call MT_NimPlayDvbcCheckParam err \n");
                return ret;
            }
        }
        else if (MT_NIM_TYPE_J83B == inputParam.nim_type_arr[i])
        {
            ret = MT_NimPlayJ83bCheckParam(inputParam.j83b);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("call MT_NimPlayJ83bCheckParam err \n");
                return ret;
            }
        }
        else if (MT_NIM_TYPE_DVBT == inputParam.nim_type_arr[i])
        {
            ret = MT_NimPlaysDvbtCheckParam(inputParam.dvbt);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("call MT_NimPlayDvbsCheckParam err \n");
                return ret;
            }
        }
        else if (MT_NIM_TYPE_DVBS_IN == inputParam.nim_type_arr[i])
        {
            ret = MT_NimPlayDvbsCheckParam(inputParam.dvbs_in);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("call MT_NimPlayDvbsCheckParam dvbs_in err \n");
                return ret;
            }
        }
        else if (MT_NIM_TYPE_DVBS_OUT == inputParam.nim_type_arr[i])
        {
            ret = MT_NimPlayDvbsCheckParam(inputParam.dvbs_out);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("call MT_NimPlayDvbsCheckParam dvbs_out err \n");
                return ret;
            }
        }
        else
        {
        }
    }

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief Help information
@param[in]  name     Enter the value
@return::MT_VOID
@*/
static MT_VOID MT_NimPlayPrint_Help(MT_CHAR *name)
{
    SAMPLE_NIM_PLAY_PRINT("Lack of parameters\n");
    SAMPLE_NIM_PLAY_PRINT("\nUsage:\n");
    SAMPLE_NIM_PLAY_PRINT("%s\n", name);
    SAMPLE_NIM_PLAY_PRINT("    -c: input dvbc info(freq symbol_rate qam)\n");
    SAMPLE_NIM_PLAY_PRINT("    -j: input j83b info(freq symbol_rate qam)\n");
    SAMPLE_NIM_PLAY_PRINT("    -s: input dvbs_in info(freq symbol_rate 22k polar sig_type)\n");
    SAMPLE_NIM_PLAY_PRINT("    -o: input dvbs_out info(freq symbol_rate 22k polar sig_type)\n");
    SAMPLE_NIM_PLAY_PRINT("    -t: input dvbt info(freq band_width)\n");
#ifdef MT_SAMPLE_APP
    SAMPLE_NIM_PLAY_PRINT("    -q: Exit the background\n");
#endif
    SAMPLE_NIM_PLAY_PRINT("example:\n");
    SAMPLE_NIM_PLAY_PRINT("    %s -c dvbc_info -j j83b_info -s dvbs_in_info -o dvbs_out_info -t dvbt_info\n",name);
    SAMPLE_NIM_PLAY_PRINT("    %s -c 314 6875 64 -j 474 5361 256 -s 3840 27500 1 0 2 -o 3840 27500 1 0 2 -t 585 8\n",name);
}


static MT_VOID MT_NimPlayPrintMenu(MT_U32 prog_num)
{

    SAMPLE_NIM_PLAY_PRINT("\n 1 - %d : select the program \n", prog_num);

    SAMPLE_NIM_PLAY_PRINT("     h : help \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_NIM_PLAY_PRINT("     b : background run \n");
#endif
    SAMPLE_NIM_PLAY_PRINT("     p : pause \n");
    SAMPLE_NIM_PLAY_PRINT("     r : resume \n");
    SAMPLE_NIM_PLAY_PRINT("     z : Channel Switch Mode \n");
    SAMPLE_NIM_PLAY_PRINT("     s : signal strength \n");
    SAMPLE_NIM_PLAY_PRINT("     l : signal quality \n");
    SAMPLE_NIM_PLAY_PRINT("     d : check if audio dolby mono \n");
    SAMPLE_NIM_PLAY_PRINT("     e : enter str suspend \n");
    SAMPLE_NIM_PLAY_PRINT("     k : set unblank screen mode\n");
    SAMPLE_NIM_PLAY_PRINT("     g : get first video frame show and avsync done cost time \n");
    SAMPLE_NIM_PLAY_PRINT("     q : quit \n");
    SAMPLE_NIM_PLAY_PRINT("NIMPLAY>> ");

}

static void MT_NimPlayFindNimPosition(MT_S32 argc, MT_CHAR *argv[], mt_nim_input_para_info *pInutParam)
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

static mt_s32 MT_NimPlayGetopt(int* ppara_num, int argc, char *argv[], char *opts)
{
    static mt_s32 sp = 1;
    mt_s32 c;
    mt_char *cp;
    mt_s32 para_num = 0;

    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

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

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();

    return c;
}

static mt_s32 MT_NimPlayUsedNimArr(mt_nim_input_para_info *sInputParam)
{
    mt_s32 idx = 0;
    mt_s32 i = 0;
    mt_nim_type nim_type[] = {
        MT_NIM_TYPE_DVBC,
        MT_NIM_TYPE_J83B,
        MT_NIM_TYPE_DVBT,
        MT_NIM_TYPE_DVBS_IN,
        MT_NIM_TYPE_DVBS_OUT};
    mt_nim_type temp_type;

    if (sInputParam == NULL) {
        SAMPLE_NIM_PLAY_ERR_PRINT(" Input param err !!! \n");
        return MT_FAILURE;
    }

    mt_u32 *ptr = (mt_u32 *)malloc(sInputParam->nim_total * sizeof(mt_u32));
    while (idx < sInputParam->nim_total) {
        temp_type = sInputParam->nim_use & nim_type[i];
        if (temp_type < MT_NIM_TYPE_DVBC|| temp_type > MT_NIM_TYPE_DVBS_OUT) {
                i++;
                continue;
        }

        switch (temp_type) {
            case MT_NIM_TYPE_DVBC:
                ptr[idx] = MT_NIM_TYPE_DVBC;
                break;
            case MT_NIM_TYPE_J83B:
                ptr[idx] = MT_NIM_TYPE_J83B;
                break;
            case MT_NIM_TYPE_DVBT:
                ptr[idx] = MT_NIM_TYPE_DVBT;
                break;
            case MT_NIM_TYPE_DVBS_IN:
                ptr[idx] = MT_NIM_TYPE_DVBS_IN;
                break;
            case MT_NIM_TYPE_DVBS_OUT:
                ptr[idx] = MT_NIM_TYPE_DVBS_OUT;
                break;
            default:
                ptr[idx] = 0;
                break;
        }

        idx++;
        i++;
    }

    sInputParam->nim_type_arr = ptr;

    return MT_SUCCESS;
}

static mt_s32 MT_NimPlaySetCurUsedTunerId(mt_u32 tuner_id)
{
    g_cur_tuner_id = tuner_id;

    return MT_SUCCESS;
}

static mt_s32 MT_NimPlayGetCurUsedTunerId(mt_u32 *tuner_id)
{
    *tuner_id = g_cur_tuner_id;

    return MT_SUCCESS;
}

/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_NimPlayDmxInit(MT_VOID)
{
    MT_S32  ret = MT_SUCCESS;

    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

    /** Initializes the demux module */
    if (play_resource.demux_use != MT_TRUE)
    {
        ret = MT_UNF_DMX_Init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_DMX_Init failed, ret = %x\n", ret);
            return ret;
        }
    }

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief Demux module deinitialization
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_VOID MT_NimPlayDmxDeinit(MT_VOID)
{
    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();
}

/*!
@brief audio and video player initialization
@param[out] hWin                the input window handler
@param[out] phAvplay            Handle to AV player
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_NimPlayAvplayInit(MT_HANDLE *phAvplay, MT_HANDLE *phWin, MT_HANDLE *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    MT_HANDLE                hAvplay = 0;
    MT_HANDLE                hWin = 0;
    MT_HANDLE                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

    if(NULL == phAvplay || NULL == phWin || NULL == phSoundTrack)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("The input address is empty!\n");
        return ret;
    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(ret != MT_SUCCESS)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_AVPlay_RegADecLib failed, ret = %x\n", ret);
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_Init failed, ret = %x\n", ret);
        return ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Defines the playing attributes of the AV player */
    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    /** Create AV player based on attributes */
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_Create failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR2;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, ret = %x\n", ret);
        goto ERROR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_SND_CreateTrack failed, ret = %x\n", ret);
        goto ERROR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_SND_Attach failed, ret = %x\n", ret);
        goto ERROR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_VO_CreatWin failed, ret = %x\n", ret);
        goto ERROR6;
    }

    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_VO_AttachWindow failed, ret = %x\n", ret);
        goto ERROR7;
    }

    /** Enable/disable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, ret = %x\n", ret);
        goto ERROR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();

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
static MT_VOID MT_NimPlayAvplayDeinit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

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

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();
}

/*
@brief Initialize and open the tuner
@param[in] tuner_id, tuner port number
@return MT_SUCCESS
@return MT_FAILURE
*/
mt_s32 MT_NimPlay_Fe_Init(mt_nim_input_para_info sInputParam)
{
    MT_S32 ret = 0;
    mt_u32 tuner_id;
    mt_u32 nim_type;
    int i, j;

    ret = mt_unf_fe_init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT(" mt_unf_fe_init failed.ret = 0x%x\n", ret);
        return ret;
    }

    for (i = 0; i < sInputParam.nim_total; i++)
    {
        nim_type = sInputParam.nim_type_arr[i];
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
        SAMPLE_NIM_PLAY_INFO_PRINT("tuner_id: 0x%x\n", tuner_id);
        ret = mt_unf_fe_open(tuner_id);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT(" mt_unf_fe_open failed.ret = 0x%x\n", ret);
            goto err;
        }
    }

    return ret;

err:
    for (j = i - 1; j >= 0; j--)
    {
        nim_type = sInputParam.nim_type_arr[j];
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
        }

        ret = mt_unf_fe_close(tuner_id);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT(" mt_unf_fe_close failed.ret = 0x%x\n",ret);
        }
    }

    ret = mt_unf_fe_deinit();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT(" mt_unf_fe_deinit failed.ret = 0x%x\n",ret);
    }

    if (sInputParam.nim_type_arr)
    {
        free(sInputParam.nim_type_arr);
        sInputParam.nim_type_arr = NULL;
    }

    return ret;
}

/*
@brief DeInitialize and close the tuner
@param[in] tuner_id, tuner port number
@return MT_SUCCESS
@return MT_FAILURE
*/
mt_s32 MT_NimPlay_Fe_DeInit(mt_nim_input_para_info sInputParam)
{
    mt_s32 ret;
    mt_s32 i;
    mt_u32 tuner_id;
    mt_u32 nim_type;
    mt_nim_config_info nim_info;

    MTADP_Str_GetNimInfo(&nim_info);

    for (i = 0; i < sInputParam.nim_total; i++)
    {
        nim_type = sInputParam.nim_type_arr[i];
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
            SAMPLE_NIM_PLAY_ERR_PRINT(" mt_unf_fe_close failed.ret = 0x%x\n", ret);
        }
    }

    ret = mt_unf_fe_deinit();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT(" mt_unf_fe_deinit failed.ret = 0x%x\n", ret);
    }

    if (sInputParam.nim_type_arr)
    {
        free(sInputParam.nim_type_arr);
        sInputParam.nim_type_arr = NULL;
    }

    nim_info.cur_nim_use = 0;
    MTADP_Str_SetNimInfo(nim_info);

    return ret;
}

/*!
@brief Set the PID of the AV player and set the encoder type.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static MT_S32 MT_NimPlaySetAvplayPidAndCodecType(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
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

    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("The input address is empty\n");
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

    SAMPLE_NIM_PLAY_INFO_PRINT("VidPid = %#x, AudPid = %#x\n", VidPid, AudPid);

    if(INVALID_TSPID != PcrPid)
    {
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_PCR_PID failed.\n");
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
            SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, ret = %x\n", ret);
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
            SAMPLE_NIM_PLAY_ERR_PRINT("Set video properties or video PID property failed, ret = %x\n", ret);
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
            SAMPLE_NIM_PLAY_ERR_PRINT("Setting the decoding mode or audio PID property failed, ret = %x\n", ret);
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
            SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, ret = %x\n", ret);
            return ret;
        }
    }

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief port The port is bound to demux
@param[in]  sInputParam       input param
@param[in]  idx           nim type idx
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error
@*/
static mt_s32 MT_NimPlayNimConnect(mt_nim_input_para_info *sInputParam, mt_s32 idx)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 nim_type;
    mt_nim_config_info nim_info;

    nim_type = sInputParam->nim_type_arr[idx];
    if (MT_NIM_TYPE_DVBC == nim_type)
    {
        mt_nim_dvbc_input dvbc = sInputParam->dvbc;

        (MT_VOID)MT_NimPlaySetCurUsedTunerId(dvbc.tuner_id);
        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, dvbc.port);

        MTADP_Str_GetNimInfo(&nim_info);
        nim_info.dvbc.tuner_id = dvbc.tuner_id;
        nim_info.dvbc.freq = dvbc.freq;
        nim_info.dvbc.sym_rate = dvbc.sym_rate;
        nim_info.dvbc.mod_type = dvbc.mod_type;
        nim_info.cur_nim_use = MT_NIM_TYPE_DVBC;
        MTADP_Str_SetNimInfo(nim_info);

        if (MT_NIM_TUNER_CONNECTED == dvbc.connect_status)
        {
            return MT_SUCCESS;
        }

        ret = MTADP_Fe_Connect_Dvbc(dvbc.tuner_id, dvbc.freq, dvbc.sym_rate, dvbc.mod_type);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Fe_Connect_Dvbc failed.\n");
            return ret;
        }

        sInputParam->dvbc.connect_status = MT_NIM_TUNER_CONNECTED;
    }
    else if (MT_NIM_TYPE_J83B == nim_type)
    {
        mt_nim_j83b_input j83b = sInputParam->j83b;

        (MT_VOID)MT_NimPlaySetCurUsedTunerId(j83b.tuner_id);
        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, j83b.port);

        MTADP_Str_GetNimInfo(&nim_info);
        nim_info.j83b.tuner_id = j83b.tuner_id;
        nim_info.j83b.freq = j83b.freq;
        nim_info.j83b.sym_rate = j83b.sym_rate;
        nim_info.j83b.mod_type = j83b.mod_type;
        nim_info.cur_nim_use = MT_NIM_TYPE_J83B;
        MTADP_Str_SetNimInfo(nim_info);

        if (MT_NIM_TUNER_CONNECTED == j83b.connect_status)
        {
            return MT_SUCCESS;
        }

        ret = MTADP_Fe_Connect_J83b(j83b.tuner_id, j83b.freq, j83b.sym_rate, j83b.mod_type);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Fe_Connect_J83b failed.\n");
            return ret;
        }

        sInputParam->j83b.connect_status = MT_NIM_TUNER_CONNECTED;
    }
    else if (MT_NIM_TYPE_DVBT == nim_type)
    {
        mt_nim_dvbt_input dvbt = sInputParam->dvbt;

        (MT_VOID)MT_NimPlaySetCurUsedTunerId(dvbt.tuner_id);
        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, dvbt.port);

        MTADP_Str_GetNimInfo(&nim_info);
        nim_info.dvbt.tuner_id = dvbt.tuner_id;
        nim_info.dvbt.freq = dvbt.freq;
        nim_info.dvbt.bandwidth = dvbt.bandwidth;
        nim_info.cur_nim_use = MT_NIM_TYPE_DVBT;
        MTADP_Str_SetNimInfo(nim_info);

        if (MT_NIM_TUNER_CONNECTED == dvbt.connect_status)
        {
            return MT_SUCCESS;
        }

        ret = MTADP_Fe_Connect_Dvbt(dvbt.tuner_id, dvbt.freq, dvbt.bandwidth);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Fe_Connect_Dvbt failed.\n");
            return ret;
        }

        sInputParam->dvbt.connect_status = MT_NIM_TUNER_CONNECTED;
    }
    else if (MT_NIM_TYPE_DVBS_IN == nim_type)
    {
        mt_nim_dvbs_input dvbs_in = sInputParam->dvbs_in;

        (MT_VOID)MT_NimPlaySetCurUsedTunerId(dvbs_in.tuner_id);
        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, dvbs_in.port);

        MTADP_Str_GetNimInfo(&nim_info);
        nim_info.dvbs_in.tuner_id = dvbs_in.tuner_id;
        nim_info.dvbs_in.freq = dvbs_in.freq;
        nim_info.dvbs_in.sym_rate = dvbs_in.sym_rate;
        nim_info.dvbs_in.onoff_22k = dvbs_in.onoff_22k;
        nim_info.dvbs_in.polar = dvbs_in.polar;
        nim_info.dvbs_in.dvbs_type = dvbs_in.dvbs_type;
        nim_info.cur_nim_use = MT_NIM_TYPE_DVBS_IN;
        MTADP_Str_SetNimInfo(nim_info);

        if (MT_NIM_TUNER_CONNECTED == dvbs_in.connect_status)
        {
            return MT_SUCCESS;
        }

        ret = MTADP_Fe_Connect_Dvbs(dvbs_in.tuner_id, dvbs_in.freq, dvbs_in.sym_rate, dvbs_in.onoff_22k, dvbs_in.polar, dvbs_in.dvbs_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Fe_Connect_Dvbs_In failed.\n");
            return ret;
        }

        sInputParam->dvbs_in.connect_status = MT_NIM_TUNER_CONNECTED;
    }
    else if (MT_NIM_TYPE_DVBS_OUT == nim_type)
    {
        mt_nim_dvbs_input dvbs_out = sInputParam->dvbs_out;

        (MT_VOID)MT_NimPlaySetCurUsedTunerId(dvbs_out.tuner_id);
        (MT_VOID)MT_UNF_DMX_AttachTSPort(DMX_ID_0, dvbs_out.port);

        MTADP_Str_GetNimInfo(&nim_info);
        nim_info.dvbs_out.tuner_id = dvbs_out.tuner_id;
        nim_info.dvbs_out.freq = dvbs_out.freq;
        nim_info.dvbs_out.sym_rate = dvbs_out.sym_rate;
        nim_info.dvbs_out.onoff_22k = dvbs_out.onoff_22k;
        nim_info.dvbs_out.polar = dvbs_out.polar;
        nim_info.dvbs_out.dvbs_type = dvbs_out.dvbs_type;
        nim_info.cur_nim_use = MT_NIM_TYPE_DVBS_OUT;
        MTADP_Str_SetNimInfo(nim_info);

        if (MT_NIM_TUNER_CONNECTED == dvbs_out.connect_status)
        {
            return MT_SUCCESS;
        }

        ret = MTADP_Fe_Connect_Dvbs(dvbs_out.tuner_id, dvbs_out.freq, dvbs_out.sym_rate, dvbs_out.onoff_22k, dvbs_out.polar, dvbs_out.dvbs_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Fe_Connect_Dvbs_Out failed.\n");
            return ret;
        }

        sInputParam->dvbs_out.connect_status = MT_NIM_TUNER_CONNECTED;
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
static MT_S32 MT_NimPlayStarToPlay(MT_HANDLE hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_NimPlaySetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlaySetAvplayPidAndCodecType fail! \n");
        return ret;
    }

    if(pProgInfo->AElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_NIM_PLAY_INFO_PRINT("has no audio info \n");
    }

    if(pProgInfo->VElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_NIM_PLAY_INFO_PRINT("has no vide0 info \n");
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
            SAMPLE_NIM_PLAY_ERR_PRINT("Set frame to VO is failed, ret = %x\n", ret);
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("Get avplay sync attr is failed, ret = %x\n", ret);
            return ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.stSyncStartRegion.bSmoothPlay = MT_TRUE;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("Set avplay sync attr is failed, ret = %x\n", ret);
            return ret;
        }
    }

    /** Start the AV player into the start state, param[enMediaType] Simultaneous playback of audio and video */
    ret  =MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_Start failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*!
@brief stop AV playback into the stop state
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_NimPlayStopToPlay(MT_HANDLE hAvplay, MT_UNF_AVPLAY_STOP_MODE_E enmode)
{
    MT_U32 ret = MT_FAILURE;
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

    /** Stop AV playback into the stop state, Keep the last frame after stopping */
    MT_NIM_PLAY_PRINT("stop live play ...\n");
    option.enMode = enmode;
    option.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_Stop failed, ret = %x\n", ret);
        return ret;
    }

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();

    return MT_SUCCESS;
}

static MT_VOID MT_NimPlayExit(MT_VOID)
{
    MT_UNF_VCODEC_UNBLANK_E unblank;

    SAMPLE_NIM_PLAY_FUNCTION_ENTER();
#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    (MT_VOID)MT_NimPlayStopToPlay(g_nimPlayInfo.hAvPlay, MT_UNF_AVPLAY_STOP_MODE_BLACK);

    (MT_VOID)MT_NimPlayAvplayDeinit(g_nimPlayInfo.hAvPlay, g_nimPlayInfo.hWin, g_nimPlayInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_DeInit();

    if(play_resource.rec_status != MT_TRUE)
    {
        /** Demux module deinitialization */
        (MT_VOID)MT_NimPlayDmxDeinit();
        play_resource.demux_use = MT_FALSE;
    }
    else
    {
        play_resource.demux_use = MT_TRUE;
        SAMPLE_NIM_PLAY_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        SAMPLE_NIM_PLAY_INFO_PRINT("PVR is recording now \n");
        SAMPLE_NIM_PLAY_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();
    if(play_resource.rec_status != MT_TRUE)
    {
        (MT_VOID)MT_NimPlay_Fe_DeInit(g_nimPlayInfo.sInputParam);
    }

    memset(&g_nimPlayInfo, 0, sizeof(g_nimPlayInfo));
    g_bTaskQuit = MT_TRUE;

    MTADP_Get_VcodeUnblank(&unblank);
    if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
    {
        MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
    }
    SAMPLE_NIM_PLAY_FUNCTION_EXIT();
}

static MT_VOID MT_NimPlayReSetOptInd(MT_VOID)
{
    g_mt_optind = 1;

    return;
}

static mt_s32 MT_NimPlayCheckParamNum(mt_s32 chr, mt_s32 para_num)
{
    switch (chr)
    {
        case 'c':
            if (MT_DVBC_PARAM_NUM != para_num)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("[dvbc|%d|%d] input param number err, exp: -c 314 6875 64 \n", MT_DVBC_PARAM_NUM, para_num);
                return MT_FAILURE;
            }
            break;
        case 'j':
            if (MT_J83B_PARAM_NUM != para_num)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("[j83b|%d|%d] input param number err, exp: -j 474 5361 256 \n", MT_J83B_PARAM_NUM, para_num);
                return MT_FAILURE;
            }
            break;
        case 's':
            if (MT_DVBS_PARAM_NUM != para_num)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("[dvbs_in|%d|%d] input param number err, exp: -s 3840 27500 1 0 2 \n", MT_DVBS_PARAM_NUM, para_num);
                return MT_FAILURE;
            }
            break;
        case 'o':
            if (MT_DVBS_PARAM_NUM != para_num)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("[dvbs_out|%d|%d] input param number err, exp: -o 3840 27500 1 0 2 \n", MT_DVBS_PARAM_NUM, para_num);
                return MT_FAILURE;
            }
            break;
        case 't':
            if (MT_DVBT_PARAM_NUM != para_num)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("[dvbt|%d|%d] input param number err, exp: -t 585 8 \n", MT_DVBT_PARAM_NUM, para_num);
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
static MT_S32 MT_NimPlayParase_args(MT_S32 argc, MT_CHAR *argv[], mt_nim_input_para_info *pInutParam)
{
    mt_s32 opt = 0;
    mt_s32 para_num = 0;

    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

    if (argc < 4 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_NimPlayPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    /*find out dvbc/dvbt/dvbs/j83b start positon in the input params.*/
    MT_NimPlayFindNimPosition(argc, argv, pInutParam);
    /*reset g_mt_optind to initital value.*/
    MT_NimPlayReSetOptInd();
    while((opt = MT_NimPlayGetopt(&para_num, argc, argv, ":?hHc:j:s:o:t:q")) != -1)
    {
        if (MT_NimPlayCheckParamNum(opt ,para_num))
        {
            return MT_FAILURE;
        }

        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_NimPlayPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    MT_NimPlayExit();
                }
                return MT_TASK_EXIT;
            case 'c':
                pInutParam->dvbc.freq = strtol(argv[pInutParam->dvbc.positon + 1], 0, 0);
                pInutParam->dvbc.sym_rate = strtol(argv[pInutParam->dvbc.positon + 2], 0, 0);
                pInutParam->dvbc.mod_type = strtol(argv[pInutParam->dvbc.positon + 3], 0, 0);
                pInutParam->dvbc.tuner_id = MT_NIM_TUNER_TYPE_DVBC;
                pInutParam->dvbc.port = MT_UNF_DMX_PORT_TSI_1;
                pInutParam->dvbc.connect_status &= MT_NIM_TUNER_NOT_CONNECTED;
                pInutParam->nim_use |= MT_NIM_TYPE_DVBC;
                pInutParam->nim_total++;
                SAMPLE_NIM_PLAY_PRINT("[dvbc] freq:%d sym_rate:%d mode_type:%d \n", pInutParam->dvbc.freq,
                    pInutParam->dvbc.sym_rate, pInutParam->dvbc.mod_type);
                break;

            case 'j':
                pInutParam->j83b.freq = strtol(argv[pInutParam->j83b.positon + 1], 0, 0);
                pInutParam->j83b.sym_rate = strtol(argv[pInutParam->j83b.positon + 2], 0, 0);
                pInutParam->j83b.mod_type = strtol(argv[pInutParam->j83b.positon + 3], 0, 0);
                pInutParam->j83b.tuner_id = MT_NIM_TUNER_TYPE_J83B;
                pInutParam->j83b.port = MT_UNF_DMX_PORT_TSI_1;
                pInutParam->j83b.connect_status &= MT_NIM_TUNER_NOT_CONNECTED;
                pInutParam->nim_use |= MT_NIM_TYPE_J83B;
                pInutParam->nim_total++;
                SAMPLE_NIM_PLAY_PRINT("[j83b] freq:%d sym_rate:%d mode_type:%d \n", pInutParam->j83b.freq,
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
                pInutParam->dvbs_in.connect_status &= MT_NIM_TUNER_NOT_CONNECTED;
                pInutParam->nim_use |= MT_NIM_TYPE_DVBS_IN;
                pInutParam->nim_total++;
                SAMPLE_NIM_PLAY_PRINT("[dvbs_in] freq:%d sym_rate:%d 22k:%d polar:%d dvbs_type:%d\n", pInutParam->dvbs_in.freq, pInutParam->dvbs_in.sym_rate,
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
                pInutParam->dvbs_out.connect_status &= MT_NIM_TUNER_NOT_CONNECTED;
                pInutParam->nim_use |= MT_NIM_TYPE_DVBS_OUT;
                pInutParam->nim_total++;
                SAMPLE_NIM_PLAY_PRINT("[dvbs_out] freq:%d sym_rate:%d 22k:%d polar:%d dvbs_type:%d\n", pInutParam->dvbs_out.freq, pInutParam->dvbs_out.sym_rate,
                    pInutParam->dvbs_out.onoff_22k, pInutParam->dvbs_out.polar, pInutParam->dvbs_out.dvbs_type);
                break;

            case 't':
                pInutParam->dvbt.freq = strtol(argv[pInutParam->dvbt.positon + 1], 0, 0);
                pInutParam->dvbt.bandwidth = strtol(argv[pInutParam->dvbt.positon + 2], 0, 0);
                pInutParam->dvbt.tuner_id = MT_NIM_TUNER_TYPE_DVBT;
                pInutParam->dvbt.port = MT_UNF_DMX_PORT_TSI_2;
                pInutParam->dvbt.connect_status &= MT_NIM_TUNER_NOT_CONNECTED;
                pInutParam->nim_use |= MT_NIM_TYPE_DVBT;
                pInutParam->nim_total++;
                SAMPLE_NIM_PLAY_PRINT("[dvbt] freq:%d bandwidth:%d\n", pInutParam->dvbt.freq, pInutParam->dvbt.bandwidth);
                break;

            default:
                (MT_VOID)MT_NimPlayPrint_Help(argv[0]);
                return MT_FAILURE;
         }
    }

    MT_NimPlayUsedNimArr(pInutParam);

    SAMPLE_NIM_PLAY_FUNCTION_EXIT();

    return MT_SUCCESS;
}

static mt_s32 MT_NimPlayStrConnectCurNim(void)
{
    mt_s32 ret = MT_SUCCESS;
    mt_nim_config_info nim_info;
    mt_nim_type nim_type;

    MTADP_Str_GetNimInfo(&nim_info);
    nim_type = nim_info.cur_nim_use;
    if (MT_NIM_TYPE_DVBC == nim_type)
    {
        mt_nim_dvbc_info dvbc = nim_info.dvbc;

        ret = MTADP_Fe_Connect_Dvbc(dvbc.tuner_id, dvbc.freq, dvbc.sym_rate, dvbc.mod_type);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Fe_Connect_Dvbc failed.\n");
            return ret;
        }
        g_nimPlayInfo.sInputParam.dvbc.connect_status = MT_NIM_TUNER_CONNECTED;
    }
    else if (MT_NIM_TYPE_J83B == nim_type)
    {
        mt_nim_j83b_info j83b = nim_info.j83b;

        ret = MTADP_Fe_Connect_J83b(j83b.tuner_id, j83b.freq, j83b.sym_rate, j83b.mod_type);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Fe_Connect_J83b failed.\n");
            return ret;
        }
        g_nimPlayInfo.sInputParam.j83b.connect_status = MT_NIM_TUNER_CONNECTED;
    }
    else if (MT_NIM_TYPE_DVBT == nim_type)
    {
        mt_nim_dvbt_info dvbt = nim_info.dvbt;

        ret = MTADP_Fe_Connect_Dvbt(dvbt.tuner_id, dvbt.freq, dvbt.bandwidth);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Fe_Connect_Dvbt failed.\n");
            return ret;
        }
        g_nimPlayInfo.sInputParam.dvbt.connect_status = MT_NIM_TUNER_CONNECTED;
    }
    else if (MT_NIM_TYPE_DVBS_IN == nim_type)
    {
        mt_nim_dvbs_info dvbs_in = nim_info.dvbs_in;

        ret = MTADP_Fe_Connect_Dvbs(dvbs_in.tuner_id, dvbs_in.freq, dvbs_in.sym_rate, dvbs_in.onoff_22k, dvbs_in.polar, dvbs_in.dvbs_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Fe_Connect_Dvbs_In failed.\n");
            return ret;
        }
        g_nimPlayInfo.sInputParam.dvbs_in.connect_status = MT_NIM_TUNER_CONNECTED;
    }
    else if (MT_NIM_TYPE_DVBS_OUT == nim_type)
    {
        mt_nim_dvbs_info dvbs_out = nim_info.dvbs_out;

        ret = MTADP_Fe_Connect_Dvbs(dvbs_out.tuner_id, dvbs_out.freq, dvbs_out.sym_rate, dvbs_out.onoff_22k, dvbs_out.polar, dvbs_out.dvbs_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Fe_Connect_Dvbs_Out failed.\n");
            return ret;
        }
        g_nimPlayInfo.sInputParam.dvbs_out.connect_status = MT_NIM_TUNER_CONNECTED;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_NimPlaySavePlayStatus(void)
{
    mt_s32 s32Ret = 0;
    MT_UNF_ENC_FMT_E fmt;
    MT_UNF_HDMI_ATTR_S hdmi_attr;
    mt_str_play_status play_status;

    s32Ret = MT_UNF_DISP_GetFormat(MT_UNF_DISPLAY1, &fmt);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_DISP_GetFormat failed\n");
    }

    s32Ret = MT_UNF_HDMI_GetAttr(MT_UNF_HDMI_ID_0, &hdmi_attr);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_HDMI_GetAttr failed\n");
    }

    MTADP_STR_GetPlayStatus(&play_status);
    play_status.fmt = fmt;
    play_status.hdcp_enable = hdmi_attr.bHDCPEnable;
    MTADP_STR_SetPlayStatus(play_status);

    return s32Ret;
}

static mt_s32 MT_NimPlayTunerStandby(void)
{
    mt_s32 s32Ret = 0;
    mt_nim_input_para_info nim_input_para;
    mt_u32 tuner_id = 0;

    nim_input_para = g_nimPlayInfo.sInputParam;
    for (int i = 0; i < nim_input_para.nim_total; i++)
    {
        if (MT_NIM_TYPE_DVBC & nim_input_para.nim_type_arr[i])
        {
            tuner_id = nim_input_para.dvbc.tuner_id;
            g_nimPlayInfo.sInputParam.dvbc.connect_status = MT_NIM_TUNER_NOT_CONNECTED;
        }
        else if (MT_NIM_TYPE_J83B & nim_input_para.nim_type_arr[i])
        {
            tuner_id = nim_input_para.j83b.tuner_id;
            g_nimPlayInfo.sInputParam.j83b.connect_status = MT_NIM_TUNER_NOT_CONNECTED;
        }
        else if (MT_NIM_TYPE_DVBT & nim_input_para.nim_type_arr[i])
        {
            tuner_id = nim_input_para.dvbt.tuner_id;
            g_nimPlayInfo.sInputParam.dvbt.connect_status = MT_NIM_TUNER_NOT_CONNECTED;
        }
        else if (MT_NIM_TYPE_DVBS_IN & nim_input_para.nim_type_arr[i])
        {
            tuner_id = nim_input_para.dvbs_in.tuner_id;
            g_nimPlayInfo.sInputParam.dvbs_in.connect_status = MT_NIM_TUNER_NOT_CONNECTED;
        }
        else if (MT_NIM_TYPE_DVBS_OUT & nim_input_para.nim_type_arr[i])
        {
            tuner_id = nim_input_para.dvbs_out.tuner_id;
            g_nimPlayInfo.sInputParam.dvbs_out.connect_status = MT_NIM_TUNER_NOT_CONNECTED;
        }

        s32Ret = mt_unf_fe_standby(tuner_id);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("mt_unf_fe_standby failed\n");
        }
    }

    return s32Ret;
}

static mt_s32 MT_NimPlayTunerResume(void)
{
    mt_s32 s32Ret = 0;
    mt_nim_input_para_info nim_input_para;
    mt_u32 tuner_id = 0;

    nim_input_para = g_nimPlayInfo.sInputParam;
    for (int i = 0; i < nim_input_para.nim_total; i++)
    {
        if (MT_NIM_TYPE_DVBC & nim_input_para.nim_type_arr[i])
        {
            tuner_id = nim_input_para.dvbc.tuner_id;
        }
        else if (MT_NIM_TYPE_J83B & nim_input_para.nim_type_arr[i])
        {
            tuner_id = nim_input_para.j83b.tuner_id;
        }
        else if (MT_NIM_TYPE_DVBT & nim_input_para.nim_type_arr[i])
        {
            tuner_id = nim_input_para.dvbt.tuner_id;
        }
        else if (MT_NIM_TYPE_DVBS_IN & nim_input_para.nim_type_arr[i])
        {
            tuner_id = nim_input_para.dvbs_in.tuner_id;
        }
        else if (MT_NIM_TYPE_DVBS_OUT & nim_input_para.nim_type_arr[i])
        {
            tuner_id = nim_input_para.dvbs_out.tuner_id;
        }

        s32Ret = mt_unf_fe_wakeup(tuner_id);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("mt_unf_fe_wakeup failed\n");
        }
    }

    return s32Ret;
}

static mt_s32 MT_NimPlayRequestPlayResource(MT_HANDLE *hAvplay, MT_HANDLE *hWin, MT_HANDLE *hSoundTrack)
{
    mt_s32 s32Ret = MT_SUCCESS;
    PMT_COMPACT_PROG   *pstCurrentProgInfo = NULL;
    mt_str_play_status play_status;

    (mt_void)MTADP_STR_GetPlayStatus(&play_status);
    (mt_void)MTADP_HDMI_Set_HdcpEnable(play_status.hdcp_enable);
    s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, play_status.fmt);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", s32Ret);
        goto ERR0;
    }

    /** Display initialization */
    s32Ret = MTADP_Disp_Init(play_status.fmt);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", s32Ret);
        goto ERR1;
    }

    s32Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_VO_Init failed.\n");
        goto ERR2;
    }

    s32Ret = MTADP_Snd_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Snd_Init failed.\n");
        goto ERR3;
    }

    /** audio and video player initialization */
    s32Ret = MT_NimPlayAvplayInit(hAvplay, hWin, hSoundTrack);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayAvplayInit failed, ret = %x\n", s32Ret);
        goto ERR4;
    }

    s32Ret = MTADP_Get_Current_Info(&pstCurrentProgInfo);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Get_Current_Info ERR !!\n");
        goto ERR5;
    }

    /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
    s32Ret = MT_NimPlayStarToPlay(*hAvplay, pstCurrentProgInfo);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayStarToPlay failed, ret = %x\n", s32Ret);
        goto ERR5;
    }

    return s32Ret;

ERR5:
    (MT_VOID)MT_NimPlayAvplayDeinit(*hAvplay, *hWin, *hSoundTrack);
ERR4:
    (MT_VOID)MTADP_Snd_DeInit();
ERR3:
    (MT_VOID)MTADP_VO_DeInit();
ERR2:
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();

    return s32Ret;
}


static void MT_NimPlayReleasePlayResource(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
    (MT_VOID)MT_NimPlaySavePlayStatus();

    /** Stop AV playback and enter the stop state */
    (MT_VOID)MT_NimPlayStopToPlay(hAvplay, MT_UNF_AVPLAY_STOP_MODE_BLACK);

    MT_NimPlayAvplayDeinit(hAvplay, hWin, hSoundTrack);

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();

    (MT_VOID)MTADP_Disp_DeInit();

    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

    (MT_VOID)mt_sys_deinit();

    return;
}

static void* MT_NimWakeupThread_function(void* arg)
{
    mt_s32 s32Ret = 0;

    s32Ret = MT_NimPlayTunerResume();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayTunerStandbyResume  failed\n");
    }

    s32Ret = MT_NimPlayStrConnectCurNim();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayStrConnectCurNim failed.\n");
    }

    return NULL;
}
#ifdef CFG_MT_SAMPLE_NAGRA
static mt_s32 nagra_read_hdcp_key(mt_u8 *p_key, mt_u32 key_len)
{
    MT_U32 mtddev = 8;//DEFAULT_MTDDEV;
    char mtddev_name[64] = {0};
    MT_HANDLE gmtd_handle = 0;
    mt_s32 ret = MT_FAILURE;

    mt_unf_flash_init();
    sprintf(mtddev_name,"/dev/mtd%d",mtddev);
    ret = mt_unf_flash_open(mtddev_name, &gmtd_handle);
    if (MT_SUCCESS == ret)
    {
        ret = mt_unf_flash_read(gmtd_handle, 0, p_key, key_len);
    }
	mt_unf_flash_close(gmtd_handle);
    return ret;
}
#endif

static void MT_NimPlayStartStrStandby(void)
{
    mt_s32 s32Ret = 0;
    sty_wakeup_conf_t wconfig = { 0 };
    pthread_t thread;

    s32Ret = MT_UNF_PMOC_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_PMOC_Init failed\n");
        return;
    }

    s32Ret = MT_UNF_PMOC_SetCecConfig(g_cec_cfg);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_PMOC_SetCecConfig failed\n");
    }

    wconfig.w_key.fp_wkey = 0x1;
    s32Ret = MT_UNF_PMOC_SetWakeUpAttr(wconfig);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_PMOC_SetWakeUpAttr  failed\n");
    }

    s32Ret = MT_NimPlayTunerStandby();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayTunerStandbyResume  failed\n");
    }

    /*First close hdmi/disp/sys module, otherwise can not enter str suspend.*/
    MT_NimPlayReleasePlayResource(g_nimPlayInfo.hAvPlay, g_nimPlayInfo.hWin, g_nimPlayInfo.hSoundTrack);
    /*Check whether the udhcpc process exists, and kill it if it does.*/
    s32Ret = system("pidof udhcpc");
    if(MT_SUCCESS == s32Ret)
    {
        s32Ret = system("kill $(pidof udhcpc)");
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("kill udhcpc failed\n");
        }
    }

    s32Ret = system("killall -SIGUSR1 audio_ta_service");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("killall -SIGUSR1 audio_ta_service failed\n");
    }

    s32Ret = system("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor failed\n");
    }

    mt_msleep(200);
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    s32Ret = system("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed failed\n");
    }

#else
    //symphony4
    s32Ret = system("echo 720000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("echo 720000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed failed\n");
    }
#endif

    SAMPLE_NIM_PLAY_INFO_PRINT("enter system suspend \n");
    s32Ret = system("echo mem > /sys/power/state");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("echo mem > /sys/power/state failed\n");
    }
#ifdef CFG_MT_SAMPLE_NAGRA
	{
		u8 key[400];
		int ret = nagra_read_hdcp_key(key,304);
		if(0 == ret){
			printf("-----%x,%x,%x,%x\n",key[0],key[1],key[2],key[3]);
			MT_UNF_HDCP_load_hdcpkey(key);
		} else {
			printf("read hdcp key error\n");
		}
	}
#endif

    s32Ret = system("optee_load_avfw -l 1 -f /usr/local/stb/avfw/avfw.bin");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("optee_load_avfw -l 1 -f /usr/local/stb/avfw/avfw.bin failed\n");
    }

    s32Ret = system("killall -SIGUSR2 audio_ta_service");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("killall -SIGUSR2 audio_ta_service failed\n");
    }

	//fix bug31220
	s32Ret = system("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq");
	s32Ret |= system("echo 1200000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq");
	s32Ret |= system("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
	if(MT_SUCCESS != s32Ret)
	{
		SAMPLE_NIM_PLAY_ERR_PRINT("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor failed\n");
	}

    s32Ret = mt_sys_init();
    if (MT_SUCCESS != s32Ret)
    {
       SAMPLE_NIM_PLAY_ERR_PRINT("mt_sys_init error. ret=0x%x \n", s32Ret);
    }

    SAMPLE_NIM_PLAY_INFO_PRINT("exit system suspend \n");
    s32Ret = pthread_create(&thread, NULL, MT_NimWakeupThread_function, NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("call pthread_creates failed\n");
    }

    s32Ret = MT_NimPlayRequestPlayResource(&g_nimPlayInfo.hAvPlay, &g_nimPlayInfo.hWin, &g_nimPlayInfo.hSoundTrack);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("call MT_NimPlayRequestPlayResource failed\n");
    }

    (MT_VOID)pthread_join(thread, NULL);

    SAMPLE_NIM_PLAY_INFO_PRINT("%s: system resume success.\n", __FUNCTION__);
}

static mt_s32 MT_NimPlayStrReceiveTask(void *param)
{
    mt_s32 ret =  0;
    MT_UNF_KEY_STATUS_E press_status = { 0 };
    MT_U64 u64KeyId = 0;
    char name[64] = { 0 };
    ir_wavefilter_config_s wavefiler = { 0 };
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    MT_U8 protocol = IRDA_NEC;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    MT_U8 protocol = RC_PROTO_NEC_;
    irda_protocol_t tmpx[]={RC_PROTO_NEC_,RC_PROTO_NECX_,RC_PROTO_RCMM32_,RC_PROTO_RC5_};

#endif
    SAMPLE_NIM_PLAY_INFO_PRINT("Use the power button of the remote control to  Str standby mode and wakeup\n");
    while(g_bStrTaskQuit != MT_TRUE)
    {
        ret = MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId, name, sizeof(name),3000);
        if(MT_SUCCESS == ret)
        {
            SAMPLE_NIM_PLAY_INFO_PRINT("u64KeyId = 0x%llx\n", u64KeyId);

            wavefiler.irda_protocol = protocol;
            wavefiler.irda_wfilt_channel = 4;

#ifdef CONFIG_MT_CHIP_SYMPHONY4
            wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
            wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = 0x7F800AF5;
            wavefiler.irda_wfilt_channel_cfg[0].protocol = IRDA_NEC;
            wavefiler.irda_wfilt_channel_cfg[1].addr_len = 32;
            wavefiler.irda_wfilt_channel_cfg[1].wfilt_code = 0xfd0148b7;
            wavefiler.irda_wfilt_channel_cfg[1].protocol = IRDA_NEC;
            (MT_VOID)MT_UNF_IR_SetWaveFilter(wavefiler);
            (MT_VOID)MT_UNF_IR_SetKeycode(0);
            (MT_VOID)MT_UNF_IR_SetUsercode(0);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
            MT_UNF_IR_Config_Protocols_ByType(tmpx,4,1);
            wavefiler.irda_wfilt_channel_cfg[1].protocol = RC_PROTO_NECX_;
            wavefiler.irda_wfilt_channel_cfg[1].addr_len = 32;
            wavefiler.irda_wfilt_channel_cfg[1].wfilt_code = 0x1fd48;

            wavefiler.irda_wfilt_channel_cfg[2].protocol = RC_PROTO_RCMM32_;
            wavefiler.irda_wfilt_channel_cfg[2].addr_len = 32;
            wavefiler.irda_wfilt_channel_cfg[2].wfilt_code = 0x29c0260c;

            wavefiler.irda_wfilt_channel_cfg[0].protocol = RC_PROTO_NEC_;
            wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
            wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = 0x800a;
            (MT_VOID)MT_UNF_IR_SetWaveFilter(&wavefiler);
#endif
            if ((press_status == 2) && ((u64KeyId == KEY_STANDBY_STR_OLD) || (u64KeyId == KEY_STANDBY_STR_NEW)))
            {
                (MT_VOID)MT_NimPlayStartStrStandby();
            }
        }
    }
    return 0;
}

static mt_s32 MT_NimPlayOpenStrFunc(void)
{
    mt_s32             ret = MT_SUCCESS;
    MT_UNF_KEY_STATUS_E press_status = MT_UNF_KEY_STATUS_BUTT;
    MT_U64 u64KeyId = 0;
    char name[64] = { 0 };

    ret = MT_UNF_IR_Init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_IR_Init ret = %d\n", ret);
        goto END;
    }

    ret = MT_UNF_IR_SetRepKeyTimeoutAttr(300);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_IR_SetRepKeyTimeoutAttr ret = %d\n", ret);
        goto END;
    }

    ret = MT_UNF_IR_EnableKeyUp(MT_TRUE);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_IR_EnableKeyUp ret = %d\n", ret);
        goto END;
    }

    ret = MT_UNF_IR_EnableRepKey(MT_FALSE);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_IR_EnableRepKey ret = %d\n", ret);
        goto END;
    }

    ret = MT_UNF_IR_SetFetchMode(0);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_IR_SetFetchMode ret = %d\n", ret);
        goto END;
    }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    ret = MT_UNF_IR_Enable(MT_TRUE, IRDA_NEC);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    ret = MT_UNF_IR_Enable(MT_TRUE, RC_PROTO_NEC_);
#endif
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_IR_SetFetchMode ret = %d\n", ret);
        goto END;
    }

    while(MT_SUCCESS == MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId,name, sizeof(name), 1000)) //Clear IR DataCache
    {
        MT_USLEEP(5000);
    }

    g_bStrTaskQuit = MT_FALSE;
    ret = pthread_create(&g_StrThd, NULL, (void * (*)(void *))MT_NimPlayStrReceiveTask, NULL);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("ErrorCode=0x%x\n",ret);
        goto END;
    }

    return MT_SUCCESS;
END:
    (MT_VOID)MT_UNF_IR_DeInit();

    return ret;
}

static mt_s32 MT_NimPlayCloseStrFunc(void)
{
    g_bStrTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_StrThd, NULL);

    (MT_VOID)MT_UNF_IR_DeInit();

    return MT_SUCCESS;
}

/*!
@brief Task action commands
@param[in]  hAvPlay     handle to AV player
@return::MT_VOID
@*/
static MT_VOID MT_NimPlayCmdTask(MT_HANDLE hAvPlay, mt_u32 total_prog_num,
    mt_nim_input_para_info *sInputParam, mt_prog_info *prog_info)
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
    mt_s32 i = 0;
    mt_s32 currentTp = 0;
    mt_s32 progNum = 0;
    mt_s32 input_progNum = 0;
    mt_s32 open_str = 0;
    mt_s32 unblank = MT_UNF_VCODEC_UNBLANK_STABLE;
    mt_s64 first_vid_frm_show_time, avsync_done_time;

    if(MT_INVALID_HANDLE == hAvPlay)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("The input handle is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }

    currentTp = g_nimPlayInfo.tpIndex;
#ifdef MT_SAMPLE_APP
		s32ProgNum = play_resource.s32ProgNum;
#endif


    while(1)
    {
        (MT_VOID)MT_NimPlayPrintMenu(total_prog_num);

        fgets((MT_CHAR *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if ('q' == inputCmd[0])
        {
            SAMPLE_NIM_PLAY_INFO_PRINT("now exit!\n");
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
            SAMPLE_NIM_PLAY_INFO_PRINT("NimPlay play in back!\n");
            break;
        }

#endif
        else if ('p' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Pause(hAvPlay, NULL);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_Pause failed\n");
            }
        }
        else if ('r' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Resume(hAvPlay, NULL);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("MT_UNF_AVPLAY_Resume failed\n");
            }
        }
        else if ('z' == inputCmd[0])
        {
            if (enmode == MT_UNF_AVPLAY_STOP_MODE_BLACK)
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_STILL;
                SAMPLE_NIM_PLAY_INFO_PRINT("Set mode to Freeze \n");
            }
            else
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
                SAMPLE_NIM_PLAY_INFO_PRINT("Set mode to black \n");
            }

        }
        else if ('s' == inputCmd[0])
        {
            MT_NimPlayGetCurUsedTunerId(&tuner_id);
            memset(&strength, 0, sizeof(strength));
            memset(&agc, 0, sizeof(agc));
            ret = mt_unf_fe_get_signal_strength(tuner_id, &strength);
            ret = mt_unf_fe_get_agc(tuner_id, 0, &agc);

            SAMPLE_NIM_PLAY_PRINT("\t Signal strength = %d, agc = %d\n", strength, agc);
        }
        else if ('l' == inputCmd[0])
        {
            MT_NimPlayGetCurUsedTunerId(&tuner_id);
            memset(&snr, 0, sizeof(snr));
            memset(&quality, 0, sizeof(quality));
            memset(&accurate_snr, 0, sizeof(accurate_snr));
            ret = mt_unf_fe_get_signal_quality(tuner_id, &quality);
            ret = mt_unf_fe_get_snr(tuner_id, &snr);
            ret = mt_unf_fe_get_accurate_snr(tuner_id, &accurate_snr);

            SAMPLE_NIM_PLAY_PRINT("\t Signal quality = %d, snr = %d accurate_snr:%02d.%03d\n",
                quality, snr, accurate_snr/1000, accurate_snr%1000);
        }
        else if (inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            progNum = atoi(inputCmd);
            input_progNum = progNum;
            if (progNum > 0 && progNum <= total_prog_num)
            {
                for (i = 0; i < sInputParam->nim_total; i++)
                {
                    s32ProgNum = progNum - (prog_info[i].program_num);
                    if(s32ProgNum > 0)
                    {
                        progNum = s32ProgNum;
                    }
                    else
                    {
                        break;
                    }
                }

                if (currentTp != i)
                {
                    currentTp = i;
                    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

                    MT_NimPlayNimConnect(sInputParam, currentTp);
                    g_nimPlayInfo.tpIndex = currentTp;
                }
                /** Stop AV playback into the stop state */
                ret = MT_NimPlayStopToPlay(hAvPlay, enmode);
                if (MT_SUCCESS != ret)
                {
                    SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayStopToPlay failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
                }

                SAMPLE_NIM_PLAY_INFO_PRINT("===== Start play ProgNum: %d \n", progNum);
                // restore ac4    attr info.
                MTADP_AUD_RestoreAc4PlayAttrInfo(hAvPlay);

                /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
                pstCurrentProgInfo = prog_info[currentTp].prog + progNum - 1;
                ret = MT_NimPlayStarToPlay(hAvPlay, pstCurrentProgInfo);
                if (MT_SUCCESS != ret)
                {
                    SAMPLE_NIM_PLAY_ERR_PRINT("Switching shows failed\n");
                }
#ifdef MT_SAMPLE_APP
                play_resource.s32ProgNum = input_progNum;
#endif
            }
            else
            {
                SAMPLE_NIM_PLAY_ERR_PRINT(" prog_num the biggest is %d \n\n", total_prog_num);
                continue;
            }

#ifdef MT_SAMPLE_APP
            ret = MTADP_Set_Current_Info(pstCurrentProgInfo);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT(" MTADP_Set_Current_Info failed.\n");
            }
#endif
        }
        else if ('d' == inputCmd[0])
        {
            MT_UNF_AUDIOTRACK_ATTR_S trackAttr;
            memset(&trackAttr, 0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
            ret = MT_UNF_SND_GetTrackAttr(g_nimPlayInfo.hSoundTrack, &trackAttr);
            if (MT_SUCCESS == ret && MT_FALSE != trackAttr.dolby_dd_ddp && MT_TRUE == trackAttr.dolby_dualmono)
            {
                SAMPLE_NIM_PLAY_INFO_PRINT("Audio dolby info: dolby[%d] Dual-Mono [1+1].\n", trackAttr.dolby_dd_ddp);
            }
        }
        else if ('e' == inputCmd[0])
        {
            open_str = !open_str;
            if (open_str)
            {
                SAMPLE_NIM_PLAY_INFO_PRINT("Open str standby.... \n");
                MT_NimPlayOpenStrFunc();
            }
            else
            {
                SAMPLE_NIM_PLAY_INFO_PRINT("Close str standby.... \n");
                MT_NimPlayCloseStrFunc();
            }
        }
        else if ('k' == inputCmd[0])
        {
            SAMPLE_NIM_PLAY_PRINT("input unblank mode(0:fast 1:stable 2:sync):");
            scanf("%d", &unblank);
            getchar();
            SAMPLE_NIM_PLAY_PRINT("unblank: %d \n", unblank);
            unblank = unblank % MT_UNF_VCODEC_UNBLANK_BUTT;
            MTADP_Set_VcodeUnblank(unblank);
        }
        else if('g' == inputCmd[0])
        {
            (MT_VOID)MTADP_ReadPlayStat(&first_vid_frm_show_time, &avsync_done_time);
            SAMPLE_NIM_PLAY_PRINT("[time] first video frame showed cost time: %lldms \n", first_vid_frm_show_time);
            SAMPLE_NIM_PLAY_PRINT("[time] avsync done cost time: %lldms \n", avsync_done_time);
        }
        else if ('h' == inputCmd[0])
        {
            SAMPLE_NIM_PLAY_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}

#ifdef MT_SAMPLE_APP
MT_S32 MT_NimPlayMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    mt_s32            ret = MT_SUCCESS;
    PMT_COMPACT_TBL  *progTbl = NULL;
    PMT_COMPACT_PROG *stCurrentProgInfo = NULL;
    mt_s32 i,j;

    SAMPLE_NIM_PLAY_FUNCTION_ENTER();

    /** Get the parameters */
    ret = MT_NimPlayParase_args(argc, argv, &g_nimPlayInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("Parase args err. stop window.\n");
        memset(&g_nimPlayInfo.sInputParam, 0, sizeof(mt_nim_input_para_info));
        return MT_FAILURE;
    }
    else if (MT_TASK_EXIT == ret)
    {
        SAMPLE_NIM_PLAY_INFO_PRINT("Recv stop command. stop window.\n");
        return MT_SUCCESS;
    }

    if (g_bTaskQuit == MT_TRUE)
    {
        ret = MT_NimPlayCheckNimParam(g_nimPlayInfo.sInputParam);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayCheckNimParam failed.\n");
            memset(&g_nimPlayInfo.sInputParam, 0, sizeof(mt_nim_input_para_info));
            return MT_FAILURE;
        }

#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
            return MT_FAILURE;
        }

        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", ret);
            goto ERR0;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", ret);
            goto ERR1;
        }
#endif

        ret = MT_NimPlay_Fe_Init(g_nimPlayInfo.sInputParam);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlay_Fe_Init failed.\n");
            goto ERR2;
        }

        /** Demux initializes and retrieves the PMT and PAT tables in TS */
        ret = MT_NimPlayDmxInit();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayDmxInit failed, ret = %x\n", ret);
            goto ERR3;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_VO_Init failed.\n");
            goto ERR4;
        }

        ret = MTADP_Snd_Init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Snd_Init failed.\n");
            goto ERR5;
        }

        /** The search module is initialized */
        (MT_VOID)MTADP_Search_Init();

        for (i = 0; i < g_nimPlayInfo.sInputParam.nim_total; i++)
        {
            ret = MT_NimPlayNimConnect(&g_nimPlayInfo.sInputParam, i);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayNimConnect failed.\n");
                goto ERR6;
            }

            /** Get the PMT table */
            ret = MTADP_Search_GetAllPmt(DMX_ID_0, &progTbl);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_NIM_PLAY_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
                goto ERR6;
            }

            for(j = 0; j < progTbl->prog_num; j++)
            {
                stCurrentProgInfo = progTbl->proginfo + j;
                g_nimPlayInfo.prog_info[i].program_num = progTbl->prog_num;
                g_nimPlayInfo.prog_info[i].prog[j].ProgID = stCurrentProgInfo->ProgID;
                g_nimPlayInfo.prog_info[i].prog[j].PmtPid = stCurrentProgInfo->PmtPid;
                g_nimPlayInfo.prog_info[i].prog[j].PcrPid = stCurrentProgInfo->PcrPid;
                g_nimPlayInfo.prog_info[i].prog[j].VideoType = stCurrentProgInfo->VideoType;
                g_nimPlayInfo.prog_info[i].prog[j].VElementNum = stCurrentProgInfo->VElementNum;
                g_nimPlayInfo.prog_info[i].prog[j].VElementPid = stCurrentProgInfo->VElementPid;
                g_nimPlayInfo.prog_info[i].prog[j].AudioType = stCurrentProgInfo->AudioType;
                g_nimPlayInfo.prog_info[i].prog[j].AElementNum = stCurrentProgInfo->AElementNum;
                g_nimPlayInfo.prog_info[i].prog[j].AElementPid = stCurrentProgInfo->AElementPid;
                g_nimPlayInfo.prog_info[i].prog[j].u16CANum = stCurrentProgInfo->u16CANum;
                memcpy(g_nimPlayInfo.prog_info[i].prog[j].CASystem, stCurrentProgInfo->CASystem, sizeof(stCurrentProgInfo->CASystem));
                memcpy(g_nimPlayInfo.prog_info[i].prog[j].Audioinfo, stCurrentProgInfo->Audioinfo, sizeof(stCurrentProgInfo->Audioinfo));
                g_nimPlayInfo.prog_info[i].prog[j].SubtType = stCurrentProgInfo->SubtType;
                g_nimPlayInfo.prog_info[i].prog[j].u16SubtitlingNum = stCurrentProgInfo->u16SubtitlingNum;
                memcpy(g_nimPlayInfo.prog_info[i].prog[j].SubtitingInfo, stCurrentProgInfo->SubtitingInfo, sizeof(stCurrentProgInfo->SubtitingInfo));
                g_nimPlayInfo.prog_info[i].prog[j].u16SCTESubtNum = stCurrentProgInfo->u16SCTESubtNum;
                memcpy(g_nimPlayInfo.prog_info[i].prog[j].stSCTESubtInfo, stCurrentProgInfo->stSCTESubtInfo, sizeof(stCurrentProgInfo->stSCTESubtInfo));
                g_nimPlayInfo.prog_info[i].prog[j].u16ClosedCaptionNum = stCurrentProgInfo->u16ClosedCaptionNum;
                memcpy(g_nimPlayInfo.prog_info[i].prog[j].stClosedCaption, stCurrentProgInfo->stClosedCaption, sizeof(stCurrentProgInfo->stClosedCaption));
                g_nimPlayInfo.prog_info[i].prog[j].u16ARIBCCPid = stCurrentProgInfo->u16ARIBCCPid;
                g_nimPlayInfo.prog_info[i].prog[j].u16TtxNum = stCurrentProgInfo->u16TtxNum;
                memcpy(g_nimPlayInfo.prog_info[i].prog[j].stTtxInfo, stCurrentProgInfo->stTtxInfo, sizeof(stCurrentProgInfo->stTtxInfo));
                memcpy(g_nimPlayInfo.prog_info[i].prog[j].u8PmtData, stCurrentProgInfo->u8PmtData, sizeof(stCurrentProgInfo->u8PmtData));
                g_nimPlayInfo.prog_info[i].prog[j].u32PmtLen = stCurrentProgInfo->u32PmtLen;
            }

            (MT_VOID)MTADP_Search_FreeAllPmt(progTbl);
            g_nimPlayInfo.total_prog_num += g_nimPlayInfo.prog_info[i].program_num;
            (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        }

        ret = MT_NimPlayNimConnect(&g_nimPlayInfo.sInputParam, 0);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayNimConnect failed.\n");
            goto ERR6;
        }

        /** audio and video player initialization */
        ret = MT_NimPlayAvplayInit(&g_nimPlayInfo.hAvPlay, &g_nimPlayInfo.hWin, &g_nimPlayInfo.hSoundTrack);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayAvplayInit failed, ret = %x\n", ret);
            goto ERR6;
        }

#ifdef MT_SAMPLE_APP
        avplayHandle.hAvPlay = g_nimPlayInfo.hAvPlay;
        avplayHandle.hSoundTrack = g_nimPlayInfo.hSoundTrack;
        avplayHandle.hWin = g_nimPlayInfo.hWin;
#endif

        /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
        g_nimPlayInfo.tpIndex = 0;
        ret = MT_NimPlayStarToPlay(g_nimPlayInfo.hAvPlay, g_nimPlayInfo.prog_info[0].prog);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayStarToPlay failed, ret = %x\n", ret);
            goto ERR7;
        }

        g_bTaskQuit = MT_FALSE;
    }

    (MT_VOID)MT_NimPlayCmdTask(g_nimPlayInfo.hAvPlay, g_nimPlayInfo.total_prog_num, &g_nimPlayInfo.sInputParam, g_nimPlayInfo.prog_info);
    if (MT_TRUE != g_bTaskQuit)
    {
        return MT_TASK_RUN;
    }

    /** Stop AV playback and enter the stop state */
    ret = MT_NimPlayStopToPlay(g_nimPlayInfo.hAvPlay, MT_UNF_AVPLAY_STOP_MODE_BLACK);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NIM_PLAY_ERR_PRINT("MT_NimPlayStopToPlay failed, ret = %d\n", ret);
    }

    if (MT_TRUE != g_bStrTaskQuit)
    {
        MT_NimPlayCloseStrFunc();
    }

ERR7:
    (MT_VOID)MT_NimPlayAvplayDeinit(g_nimPlayInfo.hAvPlay, g_nimPlayInfo.hWin, g_nimPlayInfo.hSoundTrack);
ERR6:
    (MT_VOID)MTADP_Search_DeInit();
    (MT_VOID)MTADP_Snd_DeInit();
ERR5:
    (MT_VOID)MTADP_VO_DeInit();
ERR4:
    if (play_resource.rec_status != MT_TRUE)
    {
        /** Demux module deinitialization */
        MT_NimPlayDmxDeinit();
        play_resource.demux_use = MT_FALSE;
    }
    else
    {
        play_resource.demux_use = MT_TRUE;
        SAMPLE_NIM_PLAY_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        SAMPLE_NIM_PLAY_INFO_PRINT("PVR is recording now \n");
        SAMPLE_NIM_PLAY_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }
ERR3:
    (MT_VOID)MT_NimPlay_Fe_DeInit(g_nimPlayInfo.sInputParam);
ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();
#endif

    g_bTaskQuit = MT_TRUE;
    memset(&g_nimPlayInfo, 0, sizeof(g_nimPlayInfo));

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
#endif

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    return ret;
}
