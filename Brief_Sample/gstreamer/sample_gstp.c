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
#include "mt_unf_disp.h"
#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_unf_ecs.h"
#include "mt_unf_vo.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_mpi_demux.h"

#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"


#include <stdint.h>
#include "mt_type.h"
#include "mlog.h"
#include "mutil.h"
#include "mlzplayer.h"
#include "mlzmedia.h"
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_GSTP_DEBUG

#define MT_GSTP_PRINT   printf
#else

#define MT_GSTP_PRINT

#endif

#define SAMPLE_GSTP_FUNCTION_ENTER()        MT_GSTP_PRINT("[GSTP][%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_GSTP_FUNCTION_EXIT()         MT_GSTP_PRINT("[GSTP][%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_GSTP_FATAL_PRINT(fmt...)     MT_GSTP_PRINT(" [GSTP][FATAL] " fmt)
#define SAMPLE_GSTP_ERR_PRINT(fmt...)       MT_GSTP_PRINT(" [GSTP][ERROR] " fmt)
#define SAMPLE_GSTP_WARN_PRINT(fmt...)      MT_GSTP_PRINT(" [GSTP][WARN] "  fmt)
#define SAMPLE_GSTP_INFO_PRINT(fmt...)      MT_GSTP_PRINT(" [GSTP][INFO] "  fmt)
#define SAMPLE_GSTP_DBG_PRINT(fmt...)       MT_GSTP_PRINT(" [GSTP][DEBUG] " fmt)

#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
#define DMX_ID_0            0
#define THREAD_NAME         "Monitor"
#define DEBUG_AUDIO_CRC_BUF_SIZE    (4*30*10)
/*************************** Structure Definition ****************************/
typedef struct tagSource_Param_T
{
    mt_u8 FileName[256];
    mt_u8 Avsync;
    mt_u8 LicenseUrl[256];//for DRM stream test
}source_param_t;
typedef struct Gsp_Device_s {
    mt_handle av_handle;
    mt_handle track_handle;
    mt_handle win_handle;
}Gsp_Device;
typedef struct {
    MlzPlayer         *mzp;
    int               state;
    mlzp_thread_t     tid;
    unsigned int      use_thread;
    Gsp_Device        *av_handle;
} MainCtx;
/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
MainCtx g_ctx = { 0 };
/******************************* API declaration *****************************/
static MT_VOID MT_GstpStop(MainCtx *ctx);
static MT_VOID MT_GstpSeek(MainCtx *ctx, MT_S32 seek_ms);

static MT_VOID time_extract(mt_s64 time_us, mt_u8 *hour, mt_u8 *minute, mt_u8 *second, mt_u16 *ms)
{
    mt_s64 time_reamin;
    const static mt_s64 US_TO_DAY_DEN    = (mt_s64) 24 * 60 * 60 * 1000 * 1000;
    const static mt_s64 US_TO_HOUR_DEN   = (mt_s64) 1000 * 1000 * 60 * 60;
    const static mt_s64 US_TO_MINUTE_DEN = (mt_s64) 1000 * 1000 * 60;
    const static mt_s64 US_TO_SECOND_DEN = (mt_s64) 1000 * 1000;

    /* convert time to that of a day */
    time_reamin = time_us % US_TO_DAY_DEN;
    if(NULL != hour)
    {
        *hour = (mt_u8)(time_reamin / US_TO_HOUR_DEN);
    }

    time_reamin = time_reamin % US_TO_HOUR_DEN;
    if(NULL != minute)
    {
        *minute = (mt_u8)(time_reamin / US_TO_MINUTE_DEN);
    }

    time_reamin = time_reamin % US_TO_MINUTE_DEN;
    if(NULL != second)
    {
        *second = (mt_u8)(time_reamin / US_TO_SECOND_DEN);
    }

    time_reamin = time_reamin % US_TO_SECOND_DEN;
    if(NULL != ms)
    {
        *ms = (mt_u16)(time_reamin / 1000);
    }
}

static const char *get_sub_track_name(unsigned int type)
{
    const static struct {
        const char *name;
        unsigned int type;
    } array[] = {
        {"Ass" , MLZ_CODEC_SUB_ASS}, /**< ASS subtitle */
        {"Ssa" , MLZ_CODEC_SUB_SSA}, /**< SSA subtitle */
        {"Txt", MLZ_CODEC_SUB_TXT}, /**< TXT subtitle */
    };
    for (int idx = 0; idx < ARRAY_CNT(array); idx++) {
        if (type == array[idx].type) {
            return array[idx].name;
        }
    }
    return NULL;
}

static const char *get_stream_lang_name(unsigned int type)
{
    const static struct {
        unsigned int type;
        const char *name;
    } array[] = {
        {MLZ_STREAM_LANG_CHINESE    , "Chinese"},
        {MLZ_STREAM_LANG_GERMAN     , "German"},
        {MLZ_STREAM_LANG_ENGLISH    , "English"},
        {MLZ_STREAM_LANG_FRENCH     , "French"},
        {MLZ_STREAM_LANG_JAPANESE   , "Japanese"},
        {MLZ_STREAM_LANG_VIETNAMESE , "Vietnamese"},
    };

    for (int idx = 0; idx < ARRAY_CNT(array); idx++) {
        if (type == array[idx].type) {
            return array[idx].name;
        }
    }
    return NULL;
}

static const char *get_aud_track_name(unsigned int type)
{
    const static struct {
        const char *name;
        unsigned int type;
    } array[] = {
        {"Mmpg"  , MLZ_CODEC_AUD_MP2   },  /**< MPEG audio layer 1, 2*/
        {"Mp3"   , MLZ_CODEC_AUD_MP3   },  /**< MPEG audio layer 3.*/
        {"AAC"   , MLZ_CODEC_AUD_AAC   },
        {"AC3"   , MLZ_CODEC_AUD_AC3   },
        {"DTS"   , MLZ_CODEC_AUD_DTS   },
        {"APE"   , MLZ_CODEC_AUD_APE   },
        {"PCM"   , MLZ_CODEC_AUD_PCM   },
        {"OPUS"  , MLZ_CODEC_AUD_OPUS  },
        {"FLAC"  , MLZ_CODEC_AUD_FLAC  },
        {"VORBIS", MLZ_CODEC_AUD_VORBIS},
    };
    for (int idx = 0; idx < ARRAY_CNT(array); idx++) {
        if (type == array[idx].type) {
            return array[idx].name;
        }
    }
    return NULL;
}

static MT_S32 player_get_state(MainCtx *ctx)
{
    return ctx->state;
}


static MT_VOID player_show_curr_time(MainCtx *ctx)
{
    long long curr_time = 0;

    (MT_VOID)mlz_player_get_position(ctx->mzp, &curr_time);

    mt_u16 ms;
    mt_u8 hour, minute, second;
    time_extract(curr_time / 1000, &hour, &minute, &second, &ms);
    SAMPLE_GSTP_INFO_PRINT("[Time] %02d:%02d:%02d\n", (int)hour, (int)minute, (int)second);
}


static MT_VOID *player_monitor_thread_entry(MT_VOID *arg)
{
    mt_s64 end;
    int state    = MLZ_PLAYER_PAUSED;
    MainCtx *ctx = (MainCtx*)arg;
    mt_s64 start = mclock_get_utime();
    while(MLZ_PLAYER_STOPPED != state)
    {
        usleep(100 * 1000);
        state = player_get_state(ctx);
        if(MLZ_PLAYER_STARTED != state)
        {
            continue;
        }
        end = mclock_get_utime();
        if(end - start >= SECOND_TO_MICROSECOND_BASE)
        {
            player_show_curr_time(ctx);
            start = end;
        }
    }

    memset(&ctx->tid, 0, sizeof(mlzp_thread_t));
    ctx->use_thread = 0;
    return NULL;
}


static MT_S32 player_handle_media_error(MainCtx *ctx, const MlzPlayerValue *value)
{
    int type = (int) value->data[0];

    if (MLZ_PLAYER_ERROR_UNSUPPORT_AUDIO == type)
    {
        SAMPLE_GSTP_ERR_PRINT("Audio codec type not support\n");
    }
    else if(MLZ_PLAYER_ERROR_UNSUPPORT_VIDEO == type)
    {
        SAMPLE_GSTP_ERR_PRINT("Video codec type not support\n");
    }
    else if(MLZ_PLAYER_ERROR_UNSUPPORT_SUBTITLE == type)
    {
        SAMPLE_GSTP_ERR_PRINT("Subtitle codec type not support\n");
    }
    else if(MLZ_PLAYER_ERROR_UNSUPPORT_URI == type)
    {
        SAMPLE_GSTP_ERR_PRINT("Not support URI\n");
        g_bTaskQuit = MT_TRUE;
        MT_GstpStop(ctx);
    }

    return MLZ_PLAYER_ERROR;
}


static MT_VOID player_handle_info(MainCtx *ctx, const MlzPlayerValue *value)
{
    int type = (int)(value->data[0]);

    if(type == MLZ_PLAYER_INFO_MEDIA_INFO)
    {

    }
    else if(type == MLZ_PLAYER_INFO_BUFFERING)
    {
        // player_mutex_lock(&ctx->mutex);
        //ctx->buffering_percent = value->data[1];
        // player_mutex_unlock(&ctx->mutex);
        SAMPLE_GSTP_INFO_PRINT("Buffering percent:%d\n", value->data[1]);
    }
}

static void player_set_av_handle(MainCtx *ctx)
{
    MlzPlayerValue  value = {0};
    value.pointer = ctx->av_handle;

    mlz_player_set_parameter(ctx->mzp, MLZ_PARA_AVHANDLE_INFO, &value);
}


static MT_VOID player_notify_handler(MT_VOID *ctx, int type, const MlzPlayerValue *value)
{
    MainCtx    *main_ctx = (MainCtx *)ctx;

    switch(type)
    {
        case MLZ_PLAYER_PREPARED:
            SAMPLE_GSTP_INFO_PRINT("Player Prepared!\n");
            player_set_av_handle(main_ctx);
            break;
        case MLZ_PLAYER_PLAYBACK_COMPLETE:
            SAMPLE_GSTP_INFO_PRINT("Player Complete!\n");
            MT_GstpSeek(ctx, 0);
            break;
        case MLZ_PLAYER_SEEK_COMPLETE:
            main_ctx->state = MLZ_PLAYER_STARTED;
            break;
        case MLZ_PLAYER_INFO:
            player_handle_info(ctx, value);
            break;
        case MLZ_PLAYER_ERROR:
            (MT_VOID)player_handle_media_error(ctx, value);
            break;
        default:
            break;
    }
}


static MT_S32 MT_GstpSeturi(MainCtx *ctx, char *para)
{
    const char *cmd_ptr = para;

    if(NULL == cmd_ptr || '\0' == *cmd_ptr)
    {
        SAMPLE_GSTP_ERR_PRINT("[%p] Start para error:%s\n", ctx, cmd_ptr);
        return MT_FAILURE;
    }

    ctx->mzp = mlz_player_create(0, player_notify_handler, ctx);

    (MT_VOID)mlz_player_set_source_uri(ctx->mzp, cmd_ptr);

    return MT_SUCCESS;
}


static MT_VOID MT_GstpPrepare(MainCtx *ctx)
{
    (MT_VOID)mlz_player_prepare(ctx->mzp);

    ctx->state = MLZ_PLAYER_PREPARED;
}

static void MT_GstpSubTrack(MainCtx *ctx, MT_S32 stream_index)
{
    mlz_player_set_subtitle_track(ctx->mzp, stream_index);
}

static MT_VOID MT_GstpSubcfg(MainCtx *ctx)
{
    if (!ctx || !(ctx->mzp)) {
        return;
    }

    MlzPlayerValue    value = { 0 };
    MlzPlayerSubDispCfg cfg = { 0 };
    cfg.color = 0xff000000;

    cfg.pos.x = 400;

    cfg.pos.y = 500;

    value.pointer = &cfg;
    SAMPLE_GSTP_INFO_PRINT("Set subtitle display info:"
                           "color:0x%x x:%d y:%d\n", cfg.color, cfg.pos.x, cfg.pos.y);
    (void) mlz_player_set_subtitle_surface(ctx->mzp, &value);
}

static MT_S32 MT_GstpStart(MainCtx *ctx)
{
    int ret;
    ret = mlz_player_start(ctx->mzp);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("mlz_player_start error\n");
    }

    ret = mlzp_thread_create(&(ctx->tid), THREAD_NAME, player_monitor_thread_entry, (MT_VOID *)ctx);
    if(0 == ret)
    {
        ctx->use_thread = 1;
    }
    else
    {
        return MT_FAILURE;
    }

    ctx->state = MLZ_PLAYER_STARTED;

    return MT_SUCCESS;
}


static MT_VOID MT_GstpStop(MainCtx *ctx)
{
    ctx->state = MLZ_PLAYER_STOPPED;

    if(ctx->use_thread)
    {
        mlzp_thread_join(ctx->tid, THREAD_NAME);
        ctx->use_thread = 0;
    }

    mlz_player_stop(ctx->mzp);
}


static MT_VOID MT_GstpDestroy(MainCtx *ctx)
{
    (MT_VOID) mlz_player_destory(ctx->mzp);
    ctx->mzp = NULL;
}


static MT_VOID MT_GstpPause(MainCtx *ctx)
{
    ctx->state = MLZ_PLAYER_PAUSED;
    mlz_player_pause(ctx->mzp);
}

static MT_VOID MT_GstpTrack(MainCtx *ctx, MT_S32 stream_index)
{
    mlz_player_set_audio_track(ctx->mzp, stream_index);
}

static MT_VOID MT_GstpResume(MainCtx *ctx)
{
    ctx->state = MLZ_PLAYER_STARTED;
    mlz_player_start(ctx->mzp);
}

static MT_VOID MT_GstpSeek(MainCtx *ctx, MT_S32 seek_ms)
{
    ctx->state = MLZ_PLAYER_SEEK_COMPLETE;
    seek_ms = seek_ms * 1000;
    mlz_player_seek_to_time(ctx->mzp, seek_ms);
}


static MT_VOID MT_GstpExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;

    (MT_VOID)MT_GstpStop(&g_ctx);

    (MT_VOID)MT_GstpDestroy(&g_ctx);

    memset(&g_ctx, 0xff, sizeof(g_ctx));
}

/*!
@brief audio and video player initialization
@param[out] hWin                the input window handler
@param[out] phAvplay            Handle to AV player
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_GstpAvplayInit(Gsp_Device *hPlayer,mt_u8 avSync)
{
    MT_S32                   ret = MT_FAILURE;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E     VdecType = MT_UNF_VCODEC_TYPE_BUTT;
    MT_UNF_SYNC_ATTR_S       AvSyncAttr = { 0 };

    SAMPLE_GSTP_FUNCTION_ENTER();

    if(NULL == hPlayer)
    {
        SAMPLE_GSTP_ERR_PRINT("The input address is empty!\n");
        return ret;
    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(ret != MT_SUCCESS)
    {
        SAMPLE_GSTP_ERR_PRINT("MTADP_AVPlay_RegADecLib failed, ret = %x\n", ret);
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_AVPLAY_Init failed, ret = %x\n", ret);
        return ret;
    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_AVPLAY_GetDefaultConfig failed, ret = %x\n", ret);
        goto ERROR1;
    }

    /** Defines the playing attributes of the AV player */
    AvplayAttr.stStreamAttr.u32DebugAudCrcBufSize = DEBUG_AUDIO_CRC_BUF_SIZE;
    /** Create AV player based on attributes */
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hPlayer->av_handle);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_AVPLAY_Create failed, ret = %x\n", ret);
        goto ERROR1;
    }

    ret = MT_UNF_AVPLAY_GetAttr(hPlayer->av_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    //AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    AvSyncAttr.enSyncRef = avSync;
    ret |= MT_UNF_AVPLAY_SetAttr(hPlayer->av_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR2;
    }

    MT_UNF_AVPLAY_OPEN_OPT_S stMaxCapbility;

    if (MT_UNF_VCODEC_TYPE_MVC == VdecType) {
        stMaxCapbility.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
        stMaxCapbility.enDecType = MT_UNF_VCODEC_DEC_TYPE_BUTT;
        stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MVC;
    } else {
        stMaxCapbility.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_4096x2160;
        stMaxCapbility.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
        stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_BUTT;
    }
    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hPlayer->av_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stMaxCapbility);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR2;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hPlayer->av_handle, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %x\n", ret);
        goto ERROR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, ret = %x\n", ret);
        goto ERROR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hPlayer->track_handle);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_SND_CreateTrack failed, ret = %x\n", ret);
        goto ERROR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hPlayer->track_handle, hPlayer->av_handle);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_SND_Attach failed, ret = %x\n", ret);
        goto ERROR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hPlayer->win_handle);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MTADP_VO_CreatWin failed, ret = %x\n", ret);
        goto ERROR6;
    }

    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hPlayer->win_handle, hPlayer->av_handle);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_VO_AttachWindow failed, ret = %x\n", ret);
        goto ERROR7;
    }

    /** Enable/disable windows */
    ret = MT_UNF_VO_SetWindowEnable(hPlayer->win_handle, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_VO_SetWindowEnable failed, ret = %x\n", ret);
        goto ERROR8;
    }

    /** Enable    audio he-aac */
    ret = MT_UNF_AVPLAY_Enable_AudioHEAAC(hPlayer->av_handle);;
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSTP_ERR_PRINT("MT_UNF_AVPLAY_Enable_AudioHEAAC failed, ret = %x\n", ret);
        goto ERROR9;
    }

    SAMPLE_GSTP_FUNCTION_EXIT();

    return MT_SUCCESS;

ERROR9:
    (MT_VOID)MT_UNF_VO_SetWindowEnable(hPlayer->win_handle, MT_FALSE);
ERROR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hPlayer->win_handle, hPlayer->av_handle);
ERROR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hPlayer->win_handle);
ERROR6:
    (MT_VOID)MT_UNF_SND_Detach(hPlayer->track_handle, hPlayer->av_handle);
ERROR5:
    (MT_VOID)MT_UNF_SND_DestroyTrack(hPlayer->track_handle);
ERROR4:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hPlayer->av_handle, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
ERROR3:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hPlayer->av_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
ERROR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hPlayer->av_handle);
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
static MT_VOID MT_GstpAvplayDeinit(MT_HANDLE hAvplay, MT_HANDLE hWin, MT_HANDLE hSoundTrack)
{
    SAMPLE_GSTP_FUNCTION_ENTER();

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

    SAMPLE_GSTP_FUNCTION_EXIT();
}


static MT_VOID MT_GstpPrintMenu(void)
{
    MT_GSTP_PRINT("commond:\n");
    MT_GSTP_PRINT("     s : seek to play at the specified time\n");
    MT_GSTP_PRINT("     p : pause\n");
    MT_GSTP_PRINT("     n : normal play\n");
    MT_GSTP_PRINT("     t : switch audio tracks\n");
    MT_GSTP_PRINT("     z: switch subtitle\n");
#ifdef MT_SAMPLE_APP
    MT_GSTP_PRINT("     b : background run\n");
#endif
    MT_GSTP_PRINT("     h : help\n");
    MT_GSTP_PRINT("     q : quit\n");
    MT_GSTP_PRINT("GSTP>> ");
}


static MT_VOID MT_GstpCmdTask(MainCtx *ctx)
{
    MT_S32  seek_ms = 0;
    MT_CHAR inputCmd[32] = { 0 };
    MT_BOOL PauseStatus = MT_FALSE;
    MT_S32  trackId = 0;
    MT_S32  subTrackId = 0;

    while(g_bTaskQuit != MT_TRUE)
    {
        (MT_VOID)MT_GstpPrintMenu();

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_GSTP_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        else if('p' == inputCmd[0])
        {
            SAMPLE_GSTP_INFO_PRINT("Pause\n");
            MT_GstpPause(ctx);
            PauseStatus = MT_TRUE;
            continue;
        }
        else if('n' == inputCmd[0])
        {
            SAMPLE_GSTP_INFO_PRINT("Resume\n");
            MT_GstpResume(ctx);
            PauseStatus = MT_FALSE;
            continue;
        }
        else if('z' == inputCmd[0])
        {
            MT_S32 cur_sub_index = -1;
            MlzPlayerValue value = { 0 };
            MlzPlayerStreamInfo *stream_info = NULL;
            MlzPayerSubtitleInfo *sub_info = NULL;

            mlz_player_get_parameter(ctx->mzp, MLZ_PARA_SUB_TRACK_INFO, &value);
            stream_info = (MlzPlayerStreamInfo *)value.pointer;
            sub_info = (MlzPayerSubtitleInfo *)stream_info->info;
            if (stream_info->num <= 0) {
                continue;
            }

            SAMPLE_GSTP_INFO_PRINT("Please enter the subtitle you want to play(0~%d): \n", stream_info->num - 1);

            scanf("%d", &subTrackId);
            getchar();
            MT_GstpSubTrack(ctx, subTrackId);
            usleep(500*1000);
            mlz_player_get_cur_subtitle_track(ctx->mzp, &cur_sub_index);
            SAMPLE_GSTP_INFO_PRINT("Sub Track info: [%d]:%-4s lang:%s\n", cur_sub_index,
                get_sub_track_name(sub_info[cur_sub_index].codec_id),
                get_stream_lang_name(sub_info[cur_sub_index].lang));
            continue;
        }
        else if('t' == inputCmd[0])
        {
            MT_S32 cur_aud_index = -1;
            MlzPlayerValue value = { 0 };
            MlzPlayerStreamInfo *stream_info = NULL;
            MlzPayerAudioInfo   *aud_info = NULL;

            mlz_player_get_parameter(ctx->mzp, MLZ_PARA_AUD_TRACK_INFO, &value);
            stream_info = (MlzPlayerStreamInfo *)value.pointer;
            aud_info = (MlzPayerAudioInfo *)stream_info->info;
            if (stream_info->num <= 0) {
                continue;
            }

            SAMPLE_GSTP_INFO_PRINT("Please enter the audio track you want to play(0~%d): \n", stream_info->num - 1);

            scanf("%d", &trackId);
            getchar();
            MT_GstpTrack(ctx, trackId);
            usleep(500*1000);
            mlz_player_get_cur_audio_track(ctx->mzp, &cur_aud_index);
            SAMPLE_GSTP_INFO_PRINT("Aud Track info: [%d]%-6s ch:%d sr:%d\n", cur_aud_index,
                get_aud_track_name(aud_info[cur_aud_index].codec_id),
                aud_info[cur_aud_index].channels, aud_info[cur_aud_index].sample_rate);

            continue;
        }
        else if('s' == inputCmd[0])
        {
            if(MT_TRUE == PauseStatus)
            {
                SAMPLE_GSTP_INFO_PRINT("Please resume normal playback!\n");
                continue;
            }
            SAMPLE_GSTP_INFO_PRINT("Seek to play at the specified time\n");
            MT_GSTP_PRINT("Please enter the time of the jump: \n");
            scanf("%d", &seek_ms);
            getchar();
            MT_GstpSeek(ctx, seek_ms);
            continue;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_GSTP_INFO_PRINT("suplay play in back!\n");
            break;
        }
    #endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_GSTP_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}


/*
@brief help
@return MT_VOID
*/
static MT_VOID MT_Gstp_Print_help(MT_CHAR *name)
{
    MT_GSTP_PRINT("Lack of parameters\n");
    MT_GSTP_PRINT("\nUsage:\n");
    MT_GSTP_PRINT("%s\n", name);
#ifdef MT_SAMPLE_APP
    MT_GSTP_PRINT("    -q: Exit the background\n");
#endif
    MT_GSTP_PRINT("    -f: File path  -s: avsync(1 or 0) -l: License url for DRM\n");
    MT_GSTP_PRINT("example:\n");
    MT_GSTP_PRINT("    %s -f ./ttx.ts\n", name);
    MT_GSTP_PRINT("    %s -f ./ttx.ts -s 1\n", name);
    MT_GSTP_PRINT("    %s -f ./https://xxx/ttx.ts -s 1 -l https://xxxx.xx\n", name);
}


static mt_s32 MT_GstpParase_args(int argc, char *argv[], source_param_t *pInparam)
{
    int opt = 0;

    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_Gstp_Print_help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:s:l:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_Gstp_Print_help(argv[0]);
                return MT_FAILURE;

            case 'f':
                strcpy((mt_char*)pInparam->FileName, mt_optarg);
                break;
            case 's':
                pInparam->Avsync = (mt_u8)strtol(mt_optarg, 0, 0);
                SAMPLE_GSTP_INFO_PRINT("set play Avsync %d \n", pInparam->Avsync);
                if(pInparam->Avsync >= MT_UNF_AVPLAY_SYNC_REF_BUTT)
                {
                    SAMPLE_GSTP_ERR_PRINT("input avsync=%d error!reset to audio master\n",pInparam->Avsync);
                    pInparam->Avsync = MT_UNF_SYNC_REF_AUDIO;
                }                
                break;
            case 'l':
                MTADP_Strncpy((mt_char*)pInparam->LicenseUrl, mt_optarg, sizeof(pInparam->LicenseUrl));
                SAMPLE_GSTP_INFO_PRINT("set play LicenseUrl %s \n", pInparam->LicenseUrl);
                break;    
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_GstpExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_Gstp_Print_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_GstpMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32     ret = MT_SUCCESS;
    static Gsp_Device hPlayer ={0};
    source_param_t stParam = { 0 };
    mt_u32 lurl_len = 0;

    memset(&stParam,0,sizeof(source_param_t));
    stParam.Avsync = MT_UNF_SYNC_REF_AUDIO; //default audio master

    ret = MT_GstpParase_args(argc, argv, &stParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_GSTP_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_GSTP_INFO_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    lurl_len = (mt_u32)strlen((char *)stParam.LicenseUrl);
    SAMPLE_GSTP_INFO_PRINT("play FileName %s \n", stParam.FileName);
    SAMPLE_GSTP_INFO_PRINT("play Avsync %d \n", stParam.Avsync);
    SAMPLE_GSTP_INFO_PRINT("play lurl_len %d sizeof LicenseUrl=%d\n", lurl_len,(mt_u32)sizeof(stParam.LicenseUrl));
    if(lurl_len)
    {
        SAMPLE_GSTP_INFO_PRINT("play LicenseUrl %s \n", stParam.LicenseUrl);
        setenv("LICENSE_URL", (char *)stParam.LicenseUrl, 1);
        SAMPLE_GSTP_INFO_PRINT("License URL:%s \n", getenv("LICENSE_URL"));
    }
    else
    {
        unsetenv("LICENSE_URL");
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSTP_ERR_PRINT("failed to MT_SYS_Init\n");
            return ret;
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSTP_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR1;
        }

        sleep(1);
#endif
        g_bTaskQuit = MT_FALSE;

        /** VO device initialization */
        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSTP_ERR_PRINT("MTADP_VO_Init failed, ret = %x\n", ret);
            goto ERR2;
        }

        /** Audio device initialization */
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSTP_ERR_PRINT("MTADP_Snd_Init failed, ret = %x\n", ret);
            goto ERR3;
        }

        /** audio and video player initialization */
        ret = MT_GstpAvplayInit(&hPlayer,stParam.Avsync);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSTP_ERR_PRINT("MT_DvbcAvplayInit failed, ret = %x\n", ret);
            goto ERR4;
        }
        g_ctx.av_handle = &hPlayer;
        ret = MT_GstpSeturi(&g_ctx, (char*)stParam.FileName);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSTP_ERR_PRINT("failed to MT_GstpSeturi\n");
            goto ERR5;
        }

        sleep(1);
        MT_GstpPrepare(&g_ctx);
        if(MT_TRUE == g_bTaskQuit)
        {
            goto ERR6;
        }

        sleep(1);
        MT_GstpSubcfg(&g_ctx);
        sleep(2);

        ret = MT_GstpStart(&g_ctx);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSTP_ERR_PRINT("failed to MT_GstpStart\n");
            goto ERR6;
        }
    }

    (MT_VOID)MT_GstpCmdTask(&g_ctx);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    (MT_VOID)MT_GstpStop(&g_ctx);
ERR6:
    (MT_VOID)MT_GstpDestroy(&g_ctx);
ERR5:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_GstpAvplayDeinit(hPlayer.av_handle, hPlayer.win_handle, hPlayer.track_handle);
ERR4:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();
ERR3:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();
ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR1:
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&g_ctx, 0x00, sizeof(g_ctx));

    return MT_SUCCESS;
}
