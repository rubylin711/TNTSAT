#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "mt_type.h"
#include "drv_adp.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_video.h"
#include "mt_error_mpi.h"
#include "mtos_mem.h"
#include "mtos_task.h"
#include "HA.AUDIO.G711.codec.h"
#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"
#include "HA.AUDIO.AMRWB.codec.h"
#include "HA.AUDIO.TRUEHDPASSTHROUGH.decode.h"
#include "HA.AUDIO.DOLBYTRUEHD.decode.h"
#include "HA.AUDIO.DTSHD.decode.h"
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
#include "HA.AUDIO.DOLBYPLUS.decode.h"
#endif
#include "HA.AUDIO.AC3PASSTHROUGH.decode.h"
#include "HA.AUDIO.DTSM6.decode.h"

#include "HA.AUDIO.DTSPASSTHROUGH.decode.h"
#include "HA.AUDIO.FFMPEG_DECODE.decode.h"
#include "HA.AUDIO.AAC.encode.h"

#define MODULE_TAG "DRV ADP"
#include <pthread.h>
#include "mutil.h"
#include "mlog.h"
#include "mdrv.h"
#ifdef VMX_OTT_SVP
#include "av-inc/vfmw.h"
#endif
#include "file_playback_sequence.h"
#include "file_seq_internal.h"
#include "mt_unf_suplayer.h"
#include "mt_mpi_vdec_adapter.h"
#include "mt_module_debug.h"
#include "file_seq_misc.h"
#include "mt_unf_dma.h"
#include "libavformat/avformat.h"
#include "libavutil/encryption_info.h"

#ifdef DRM_SMP_ENABLE
#include "MTDrmApi.h"
#endif
#define   AES_TMP_BUF_LEN    (1024 * 32)
#ifdef __cplusplus
extern "C" {
#endif

#define TIME_OUT_MS_ES_PUSH       0//10
#define PUSH_DATA_TIME_COUNT (300)
#define ESBUF_INT_LEN 16*1024
#define MIN_ES_BUFFER_SIZE   (256)

#define MTAPI_RUN_RETURN_INT(api) \
do {\
    mt_s32 errCode; \
    errCode = api; \
    if (errCode != 0)\
    {\
        MLOGD("\033[0;31m" "[Function: %s line: %d] %s failed ret = 0x%x \n" "\033[0m", __FUNCTION__, __LINE__, # api, errCode); \
        return MT_FAILURE; \
    } \
} while (0)


/*
big-endian pcm output format, if extword is 1, choose normal pcm decoder,
if extword is 2, choose wifidsp_lpcm decoder(Frame Header:0xA0,0x06)
if others, fail to decode.
*/
#define NORMAL_PCM_EXTWORD    1
#define WIFIDSP_LPCM_EXTWORD  2

static mt_u8 u8DecOpenBuf1[1024];
typedef enum {
    DECODE_STOP,
    DECODE_START,
    DECODE_PAUSE,
    DECODE_OTHER
} DECODE_STATUS;

typedef void *(*AES_MEMCPY)(void *dst, const void *src, size_t size);
typedef void *(*VES_MEMCPY)(void *dst, const void *src, size_t size);

MT_HANDLE g_avplay_es_handle = MT_NULL;
static MT_HANDLE g_hTrack_sup;
unsigned char   *p_es_ves_buf = 0;
unsigned char   *p_es_aes_buf = 0;
//static pthread_mutex_t VMutex;
static pthread_mutex_t fp_amutex;
static pthread_mutex_t fp_vmutex;
static DECODE_STATUS vdec_status = DECODE_STOP;
static DECODE_STATUS adec_status = DECODE_STOP;
MT_HANDLE g_suplayer_handle = MT_NULL;
static int g_tplay_1xnum = 1;

static MT_BOOL is_frame_setup = FALSE;

typedef struct {
    mt_u8 buf_save_es[ESBUF_INT_LEN];
    short filled_size;
} ESBUF_INTERNAL;
ESBUF_INTERNAL g_esbuf_internal;
int pkt_cnt = 0;
static RET_CODE vdec_set_file_playback_fr2avplay(mt_void);
static int aud_get_es_buf_space_info(void *p_dev, mt_u32 *all_size, mt_u32 *use_size);

#define  WAVE_FORMAT_PCM          1
#define  WAVE_FORMAT_IEEE_FLOAT 0x0003
#define  WAVE_FORMAT_EXTENSIBLE 0xfffe

static mt_s32 MTADP_AVPlay_SetAdecAttr_internal(PLAYBACK_INTERNAL_T *pbi,
    mt_handle hAvplay, mt_u32 enADecType, MT_HA_DECODEMODE_E enMode, mt_s32 isCoreOnly)
{
    MT_UNF_ACODEC_ATTR_S AdecAttr;

    MTAPI_RUN_RETURN_INT(MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr));
    AdecAttr.enType = enADecType;

    if (HA_AUDIO_ID_PCM == AdecAttr.enType) {
        //do nothing, already set by set_dec_param_vsb
    } else if (HA_AUDIO_ID_MP2 == AdecAttr.enType) {
        HA_MP2_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    } else if (HA_AUDIO_ID_AAC == AdecAttr.enType) {
        HA_AAC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    } else if (HA_AUDIO_ID_MP3 == AdecAttr.enType) {
        HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    } else if (HA_AUDIO_ID_AC3PASSTHROUGH == AdecAttr.enType) {
        HA_AC3PASSTHROUGH_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    } else if (HA_AUDIO_ID_DTSPASSTHROUGH ==  AdecAttr.enType) {
        HA_DTSPASSTHROUGH_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    } else if (HA_AUDIO_ID_TRUEHD == AdecAttr.enType) {
        HA_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        if (HD_DEC_MODE_THRU != enMode) {
            MLOGE(" MLP decoder enMode(%d) error (mlp only support hbr Pass-through only).\n", enMode);
            return -1;
        }

        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;        /* truehd just support pass-through */
        MLOGD(" TrueHD decoder(HBR Pass-through only).\n");
    } else if (HA_AUDIO_ID_DOLBY_TRUEHD == AdecAttr.enType) {
        TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf1;
        HA_DOLBY_TRUEHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    } else if (HA_AUDIO_ID_DTSHD == AdecAttr.enType) {
        DTSHD_DECODE_OPENCONFIG_S *pstConfig = (DTSHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf1;
        HA_DTSHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    } else if (HA_AUDIO_ID_DTSM6 == AdecAttr.enType) {
        DTSM6_DECODE_OPENCONFIG_S *pstConfig = (DTSM6_DECODE_OPENCONFIG_S *)u8DecOpenBuf1;
        HA_DTSM6_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSM6_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
    else if (HA_AUDIO_ID_DOLBY_PLUS == AdecAttr.enType) {
        DOLBYPLUS_DECODE_OPENCONFIG_S *pstConfig = (DOLBYPLUS_DECODE_OPENCONFIG_S *)u8DecOpenBuf1;
        HA_DOLBYPLUS_DecGetDefalutOpenConfig(pstConfig);
        pstConfig->pfnEvtCbFunc[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = DDPlusCallBack;
        pstConfig->pAppData[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = &g_stDDpStreamInfo;
        /* Dolby DVB Broadcast default settings */
        pstConfig->enDrcMode = DOLBYPLUS_DRC_RF;
        pstConfig->enDmxMode = DOLBYPLUS_DMX_SRND;
        HA_DOLBYPLUS_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
#endif
    else if (HA_AUDIO_ID_DRA == AdecAttr.enType) {
//       HA_DRA_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        HA_DRA_DecGetOpenParam_MultichPcm(&(AdecAttr.stDecodeParam));

    } else if (HA_AUDIO_ID_COOK == AdecAttr.enType || HA_AUDIO_ID_AMRNB == AdecAttr.enType
               || HA_AUDIO_ID_AMRWB == AdecAttr.enType) {
        //do nothing, already set by set_dec_param_vsb
    } else if (HA_AUDIO_ID_FLAC      == AdecAttr.enType ||
               HA_AUDIO_ID_VORBIS    == AdecAttr.enType ||
               HA_AUDIO_ID_OPUS      == AdecAttr.enType ||
               HA_AUDIO_ID_VVID      == AdecAttr.enType ||
               HA_AUDIO_ID_DOLBY_AC4 == AdecAttr.enType) {
        HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        AdecAttr.stDecodeParam.pCodecPrivateData       = pbi->audio.codec_extradata;
        AdecAttr.stDecodeParam.u32CodecPrivateDataSize = pbi->audio.codec_extradata_size;
    }
    MTAPI_RUN_RETURN_INT(MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr));
    return MT_SUCCESS;
}

static mt_s32 MTADP_AVPlay_SetVdecAttr_internal(mt_handle hAvplay, MT_UNF_VCODEC_TYPE_E enType, MT_UNF_VCODEC_MODE_E enMode)
{
    mt_s32 Ret;
    MT_UNF_VCODEC_ATTR_S        VdecAttr;

    Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    if (MT_SUCCESS != Ret) {
        MLOGE("MT_UNF_AVPLAY_GetAttr failed:%#x\n", Ret);
        return Ret;
    }

    VdecAttr.enType = enType;
    VdecAttr.enMode = enMode;
    VdecAttr.u32ErrCover = 100;
    VdecAttr.u32Priority = 3;

    Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    if (Ret != MT_SUCCESS) {
        MLOGE("call MT_UNF_AVPLAY_SetAttr failed.\n");
        return Ret;
    }

    return Ret;
}


void file_seq_register_event(MT_UNF_AVPLAY_EVENT_E     enEvent,
                             MT_UNF_AVPLAY_EVENT_CB_FN pfnEventCB)
{
    MLOGD("Register event %d cb %p\n", enEvent, pfnEventCB);
    MT_UNF_AVPLAY_RegisterEvent(g_avplay_es_handle, enEvent, pfnEventCB);
}

void file_seq_unregister_event(MT_UNF_AVPLAY_EVENT_E enEvent)
{
    MT_UNF_AVPLAY_UnRegisterEvent(g_avplay_es_handle, enEvent);
}


void file_seq_avplay_handle_set(MT_HANDLE handle_avplay, int handle_track)
{
    // file_seq_set_fw_mem(1, 1, 1, 1);
    MLOGD("%s,%d,g_avplay_es_handle = %d, g_hTrack_sup = %d\n", __func__, __LINE__, handle_avplay, handle_track);
    g_avplay_es_handle = (MT_HANDLE)handle_avplay;
    g_hTrack_sup = (MT_HANDLE)handle_track;
}

void file_seq_suplayer_handle_set(int handle_suplayer)
{
    // file_seq_set_fw_mem(1, 1, 1, 1);
    //MLOGD("%s,%d,g_avplay_es_handle = %d, g_hTrack_sup = %d\n", __func__,__LINE__,handle_suplayer);
    g_suplayer_handle = (MT_HANDLE)handle_suplayer;
}

static int get_adec_status(void *p_dev)
{
    (void) p_dev;
    return adec_status;
}

static int get_vdec_status(void *p_dev)
{
    (void) p_dev;
    return vdec_status;
}

void *dev_find_identifier(void *p_sdev, int ident_type, u32 ident)
{
    MLOGD("%s,%d\n", __func__, __LINE__);
    return (void *)&g_avplay_es_handle;
}

RET_CODE dev_open(void *p_dev, void *p_param)
{
    static char first = 0;
    MLOGD("%s,%d\n", __func__, __LINE__);
    if (first == 0) {
        first = 1;
        //pthread_mutex_init(&VMutex,NULL);
        pthread_mutex_init(&fp_amutex, NULL);
        pthread_mutex_init(&fp_vmutex, NULL);
        MLOGD("pthread_mutex_initpthread_mutex_initpthread_mutex_init\n");
        //init g_esbuf_internal
        memset(&g_esbuf_internal, 0, sizeof(ESBUF_INTERNAL));
    }
    return MT_SUCCESS;
}

RET_CODE vdec_pause(void *p_dev)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_AVPLAY_STATUS_INFO_S status;

    if (vdec_status != DECODE_START) {
        MLOGI("%s,%d vdec_status: %d\n", __func__, __LINE__, vdec_status);
        return ret;
    }

    vdec_status = DECODE_PAUSE;
    MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &status);
    MLOGI("%s,%d Status %d\n", __func__, __LINE__, status.enRunStatus);
    if (status.enRunStatus != MT_UNF_AVPLAY_STATUS_PAUSE) {
        //pthread_mutex_lock(&VMutex);
        MLOGD("%s,%d\n", __func__, __LINE__); //MLOGD
        ret =  MT_UNF_AVPLAY_Pause(g_avplay_es_handle, MT_NULL);
        //pthread_mutex_unlock(&VMutex);
    }

    return ret;
}

RET_CODE vdec_get_stream_info(void *p_dev, MT_UNF_AVPLAY_STREAM_INFO_S *pstStreamInfo)
{
    return MT_UNF_AVPLAY_GetStreamInfo(g_avplay_es_handle, pstStreamInfo);
}

RET_CODE vdec_resume(void *p_dev)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_AVPLAY_STATUS_INFO_S status;

    if (vdec_status != DECODE_PAUSE) {
        MLOGI("%s,%d vdec_status: %d\n", __func__, __LINE__, vdec_status);
        return ret;
    }

    vdec_status = DECODE_START;
    MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &status);
    MLOGI("%s,%d Status %d\n", __func__, __LINE__, (int)status.enRunStatus);
    if (status.enRunStatus != MT_UNF_AVPLAY_STATUS_PAUSE) {
        //change state to PAUSE
        //pthread_mutex_lock(&VMutex);
        ret =  MT_UNF_AVPLAY_Pause(g_avplay_es_handle, MT_NULL);
        //pthread_mutex_unlock(&VMutex);
    }
    MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &status);
    if (status.enRunStatus == MT_UNF_AVPLAY_STATUS_PAUSE || status.enRunStatus == MT_UNF_AVPLAY_STATUS_TPLAY) {
        //pthread_mutex_lock(&VMutex);
        ret =  MT_UNF_AVPLAY_Resume(g_avplay_es_handle, MT_NULL);
        //pthread_mutex_unlock(&VMutex);
    }

    return ret;
}

/**
 * video_decoder_flush
 *
 * @param[in] handle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
RET_CODE vdec_flush(void *p_dev)
{
    //flush both input and output port!!
    MT_UNF_AVPLAY_STATUS_INFO_S status;
    mt_s32 s32Ret = -1;
    MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &status);

    MLOGI("[%s] Status %d\n", __FUNCTION__, status.enRunStatus);
    if((status.enRunStatus == MT_UNF_AVPLAY_STATUS_PLAY)||
        (status.enRunStatus == MT_UNF_AVPLAY_STATUS_PAUSE))
    {
        MLOGI("Flush av begin in vdec_flush\n");
        s32Ret = MT_UNF_AVPLAY_Flush(g_avplay_es_handle);
        MLOGI("Flush av in vdec_flush %s!\n", 0 == s32Ret ? "success" : "fail");
    }

    drv_adp_flush_avif(p_dev);
    return s32Ret;
}
/**
 * video_decoder_reset
 *
 * @param[in] handle video decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */

RET_CODE vdec_reset(void)
{
    //flush both input and output port!!
    MT_UNF_AVPLAY_STATUS_INFO_S status;
    mt_s32 s32Ret = -1;
    MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &status);
    MLOGI("[%s] Status %d\n", __FUNCTION__, status.enRunStatus);
    if ((status.enRunStatus == MT_UNF_AVPLAY_STATUS_PLAY) ||
        (status.enRunStatus == MT_UNF_AVPLAY_STATUS_PAUSE)) {
        s32Ret = MT_UNF_AVPLAY_Reset(g_avplay_es_handle, NULL);
    }
    return s32Ret;
}


RET_CODE vdec_freeze_stop(void *p_dev)
{
    mt_s32 ret = 0;
    MT_UNF_AVPLAY_STOP_OPT_S stop;

    MLOGD("%s,%d\n", __func__, __LINE__);
    if (vdec_status == DECODE_STOP) {
        return MT_SUCCESS;
    }

    MLOGD("%s,%d\n", __func__, __LINE__);
    vdec_status = DECODE_STOP;
    //close avplay
    //pthread_mutex_lock(&VMutex);
    stop.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    stop.u32TimeoutMs = 0;
    pthread_mutex_lock(&fp_vmutex);
    ret = MT_UNF_AVPLAY_Stop(g_avplay_es_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stop);
    pthread_mutex_unlock(&fp_vmutex);
    if (ret != MT_SUCCESS) {
        MLOGE(" MT_UNF_AVPLAY_Stop failed.\n");
    }
    //pthread_mutex_unlock(&VMutex);

    return ret;
}

RET_CODE vdec_file_clearesbuffer(void *p_dev)
{
    MT_UNF_AVPLAY_STATUS_INFO_S status;
    mt_s32 s32Ret = -1;

    MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &status);
    MLOGD("[%s] Status:%d\n", __FUNCTION__, status.enRunStatus);
    MLOGI("Clear es buffer begin\n");
    s32Ret = MT_UNF_AVPLAY_Flush(g_avplay_es_handle);
    MLOGI("Clear es buffer %s!\n", 0 == s32Ret ? "success" : "fail");
    drv_adp_flush_avif(p_dev);
    return s32Ret;
}

static int vdec_get_es_buf_space_info(
    void *p_dev, mt_u32 *all_size, mt_u32 *used_size)
{
    mt_s32 ret;
    MT_UNF_AVPLAY_STATUS_INFO_S pstStatusInfo;
    MT_UNF_AVPLAY_BUF_STATUS_S *pVidBufInfo;

    (void) p_dev;
    ret = MT_UNF_AVPLAY_GetVideoStatusInfo(g_avplay_es_handle, &pstStatusInfo);
    if (ret != MT_SUCCESS) {
        MLOGE(" [%s_%d] fail\n", __FUNCTION__, __LINE__);
        return ret;
    }

    pVidBufInfo = &(pstStatusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID]);
    if (all_size) {
        *all_size = pVidBufInfo->u32BufSize;
    }
    if (used_size) {
        *used_size = pVidBufInfo->u32UsedSize;
    }

    return MT_SUCCESS;
}

RET_CODE vdec_get_es_buf_space(void *p_dev, u32 *p_size)
{
    mt_s32 ret;
    u32 all_buf_size, used_size;

    all_buf_size = used_size = 0;
    ret = vdec_get_es_buf_space_info(p_dev, &all_buf_size, &used_size);
    if (ret != MT_SUCCESS) {
        return ret;
    }

    int free_space = (int) (all_buf_size - used_size);
    if (free_space > 0) {
        *p_size = (u32)(free_space >> 10);
    } else {
        *p_size = 0;
        MLOGD("%s,%d no space size[%d]!\n", __func__, __LINE__, free_space);
    }

    return MT_SUCCESS;
}

RET_CODE vdec_file_get_es_buf_info(void *p_dev, mt_u32 *all_size, mt_u32 *use_size)
{
    return vdec_get_es_buf_space_info(p_dev, all_size, use_size);
}

RET_CODE vdec_file_get_water_level_vsb(void *p_dev)
{
    #define ES_BUFFER_WATER_LEVEL   80
    u32 all_buf_size, used_size;
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)p_dev;

    all_buf_size = used_size = 0;
    mt_s32 ret = vdec_get_es_buf_space_info(p_dev, &all_buf_size, &used_size);
    if (ret != MT_SUCCESS) {
        return ret;
    }

    int available_size =
         (int) (all_buf_size * ES_BUFFER_WATER_LEVEL / 100 - used_size);
    if(p_internal != NULL) {
        p_internal->vitual_video_es_cur_size = available_size;
    }

    if(available_size <= 0) {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

RET_CODE vdec_get_es_buf_pkt_num(void *p_dev, int *pkt_num)
{
    mt_s32 ret;
    MT_UNF_AVPLAY_STATUS_INFO_S pstStatusInfo;
    MT_UNF_AVPLAY_BUF_STATUS_S *pVidBufInfo;
    int free_space = 0;

    if (!pkt_num) {
        return MT_FAILURE;
    }

    *pkt_num = -1;
    ret = MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &pstStatusInfo);
    if (ret != MT_SUCCESS) {
        MLOGE("Get es buffer pkt number fail\n");
        return ret;
    }

    pVidBufInfo = &(pstStatusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID]);
    *pkt_num = (int) pVidBufInfo->u32VideoESFrameNumber;

    return MT_SUCCESS;
}

RET_CODE vdec_set_avsync_mode(void *p_dev, u32 mode)
{
    mt_s32 ret = MT_SUCCESS;
#if 0
    MLOGD("%s,%d\n", __func__, __LINE__);
    pthread_mutex_lock(&VMutex);
    ret = MT_UNF_AVPLAY_SetAvsyncMode(g_avplay_es_handle, mode);
    pthread_mutex_unlock(&VMutex);
#else
    MLOGD("%s,%d\n", __func__, __LINE__);
#endif
    return ret;
}

unsigned int drv_adp_video_codec_id;//same video_codec_id, for sample compile
RET_CODE vdec_start(void *p_dev, int format, int mode)
{
    MT_UNF_SYNC_ATTR_S   AvSyncAttr = {0};
    MT_UNF_VCODEC_ATTR_S VcodecAttr = {0};

    MT_BOOL bAdvancedProfil = 1;
    mt_s32 u32CodecVersion  = 8;
    mt_s32                  ret = MT_SUCCESS;

    MLOGI("%s,%d, format:%s\n", __func__, __LINE__, mdrv_vdec_type_to_str(format));
    if (vdec_status == DECODE_START) {
        MLOGI("[%s] vdec_status: DECODE_START, return\n", __func__);
        return MT_SUCCESS;
    }

    ret = MT_UNF_AVPLAY_GetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    ret = MT_UNF_AVPLAY_GetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
    if (MT_UNF_VCODEC_TYPE_VC1 == format) {
        if (drv_adp_video_codec_id == vVIDEO_VC1SMP5) {
            bAdvancedProfil = 0; /* old wmv3 */
        }
        VcodecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = bAdvancedProfil;
        VcodecAttr.unExtAttr.stVC1Attr.u32CodecVersion  = (mt_u32)u32CodecVersion;
    }

    if (MT_UNF_VCODEC_TYPE_VP6 == format) {
        VcodecAttr.unExtAttr.stVP6Attr.bReversed = 0;
    }

    VcodecAttr.enType = format;
    VcodecAttr.u32UseDescInfoFlag = 1;
#ifdef CONFIG_MT_ENABLE_MSS_PLAYER
    //Support Dynamic Resolution for MSS
    VcodecAttr.u32ErrCover = 100;
    VcodecAttr.u32Priority = 3;
#endif

    ret |= MT_UNF_AVPLAY_SetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
    if (MT_SUCCESS != ret) {
        MLOGE("call MT_UNF_AVPLAY_SetAttr not success %s %d\n", __FUNCTION__, __LINE__);
    }

    vdec_set_file_playback_fr2avplay();
    ret = MTADP_AVPlay_SetVdecAttr_internal(g_avplay_es_handle, format, MT_UNF_VCODEC_MODE_NORMAL);
    if (ret != MT_SUCCESS) {
        MLOGE("call MT_UNF_AVPLAY_SetAttr not success %s %d\n", __FUNCTION__, __LINE__);
    }

    ret = MT_UNF_AVPLAY_Start(g_avplay_es_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if (ret != MT_SUCCESS) {
        return ret;
    }
    vdec_status = DECODE_START;

    return ret;
}

RET_CODE set_video_decoder_framerate(uint32_t frame_rate)
{
    MT_S32 ret = 0;
    if(frame_rate == 0)
    return ret;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S frame_rate_param = {0};

    frame_rate_param.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_USER;
    frame_rate_param.stSetFrmRate.u32fpsInteger = frame_rate;
    frame_rate_param.stSetFrmRate.u32fpsDecimal = 0;
    MLOGI("%s,%d, set frame_rate to video decoder:%d\n", __func__,__LINE__,frame_rate);
    ret = MT_UNF_AVPLAY_SetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_DECLEAR_FRMRATE_PARAM, (mt_void *)(&frame_rate_param));

    return ret;
}


int push_fcnt = 0;
RET_CODE vdec_stop(void *p_dev)
{
    mt_s32 ret = 0;

    MT_UNF_AVPLAY_STOP_OPT_S stop;
    MLOGI("%s,%d\n", __func__, __LINE__);
    push_fcnt = 0;

    if (vdec_status == DECODE_STOP) {
        MLOGI("[%s] vdec_status: DECODE_STOP, return\n", __func__);
        return MT_SUCCESS;
    }

    vdec_status = DECODE_STOP;
    stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stop.u32TimeoutMs = 0;
    pthread_mutex_lock(&fp_vmutex);
    ret = MT_UNF_AVPLAY_Stop(g_avplay_es_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stop);
    pthread_mutex_unlock(&fp_vmutex);
    if (ret != MT_SUCCESS) {
        MLOGD(" MT_UNF_AVPLAY_Stop failed.\n");
    }
    //pthread_mutex_unlock(&VMutex);
    MLOGD("%s,%d\n", __func__, __LINE__);

    return ret;
}

RET_CODE vdec_set_file_playback_fr2avplay(mt_void)
{
    mt_s32 ret = 0;
#if 0
    /* According to 114865, frame will be set by fw and player does not set the frame rate*/
    MT_UNF_AVPLAY_FRMRATE_PARAM_S frame_rate_param;
    frame_rate_param.enFrmRateType = mplayer_fr.enFrmRateType;
    frame_rate_param.stSetFrmRate.u32fpsInteger = mplayer_fr.stSetFrmRate.u32fpsInteger;
    frame_rate_param.stSetFrmRate.u32fpsDecimal = mplayer_fr.stSetFrmRate.u32fpsDecimal;

    MLOGD("%s,%d\n", __func__, __LINE__);
    //pthread_mutex_lock(&VMutex);
    MLOGD("%s,%d,%d\n", __func__, __LINE__, mplayer_fr.stSetFrmRate.u32fpsInteger);
    ret = MT_UNF_AVPLAY_SetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, (mt_void *)(&frame_rate_param));
    MLOGD("%s,%d,r=%d\n", __func__, __LINE__, ret);
    //pthread_mutex_unlock(&VMutex);
#endif

    return ret;
}

RET_CODE vdec_get_es_buf_size(void *p_dev, u32 *p_size)
{
    mt_s32 ret;
    MT_UNF_AVPLAY_ATTR_S avplay_attr;
#if 0
    MT_UNF_AVPLAY_BUF_INFO_S bufinfo;
    MLOGD("%s,%d\n", __func__, __LINE__);
    pthread_mutex_lock(&VMutex);
    MT_UNF_AVPLAY_GetBufInfo(g_avplay_es_handle, MT_UNF_AVPLAY_VID_ES_BUF, &bufinfo);
    *p_size = bufinfo.totalSize >> 10;
    pthread_mutex_unlock(&VMutex);
#else
    MLOGD("%s,%d\n", __func__, __LINE__);
    //pthread_mutex_lock(&VMutex);
    //MT_UNF_AVPLAY_GetBufInfo(g_avplay_es_handle,MT_UNF_AVPLAY_VID_ES_BUF,&bufinfo);
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)p_dev;
    if (p_internal->vitual_video_es_max_size == 0) {
        ret = MT_UNF_AVPLAY_GetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_STREAM_MODE, &avplay_attr);
        if (ret == MT_SUCCESS) {
            *p_size = avplay_attr.stStreamAttr.u32VidBufSize >> 10;
            MLOGD("%s,%d vid buf size: %d k\n", __func__, __LINE__, avplay_attr.stStreamAttr.u32VidBufSize);

        } else {
            MLOGE("%s,%d error: %d\n", __func__, __LINE__, ret);
        }
        p_internal->vitual_video_es_max_size = (int)(*p_size);
        //p_internal->min_ves_buf_size = (p_internal->vitual_video_es_max_size)>>2;
        p_internal->min_ves_buf_size = VIDEO_ES_BUF_OVERFLOW_THRESHOLD;
        //MLOGD("zx p_internal->vitual_video_es_max_size %d min_ves_buf_size %d\n",
        //    p_internal->vitual_video_es_max_size, p_internal->min_ves_buf_size);
    } else {
        *p_size = (u32)p_internal->vitual_video_es_max_size;
    }

    //pthread_mutex_unlock(&VMutex);
#endif
    return MT_SUCCESS;
}

RET_CODE vdec_get_info(void *p_dev, void *p_state)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S pPrivateInfo;

    //unf_vdec_info_t vstate;
    MLOGD("%s,%d\n", __func__, __LINE__);
    //pthread_mutex_lock(&VMutex);
    ret = MT_UNF_AVPLAY_Invoke(g_avplay_es_handle, MT_UNF_AVPLAY_INVOKE_GET_PRIV_PLAYINFO, &pPrivateInfo);
    if (ret != MT_SUCCESS) {
        MLOGE("@@E: get video info error @line %d,fun %s", __LINE__, __func__);
    }
    return MT_SUCCESS;
}

RET_CODE vdec_get_pts(void *p_dev, void *p_state)
{
    mt_s32 ret = MT_SUCCESS;
    //MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S pPrivateInfo;
    MT_UNF_AVPLAY_STATUS_INFO_S status;
#if 1//doreen delete for  MT_UNF_AVPLAY_Invoke failed
    MLOGD("%s,%d\n", __func__, __LINE__);
    //pthread_mutex_lock(&VMutex);
    //ret = MT_UNF_AVPLAY_Invoke(g_avplay_es_handle, MT_UNF_AVPLAY_INVOKE_GET_PRIV_PLAYINFO, &pPrivateInfo);
    ret = MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &status);
    if (ret != MT_SUCCESS) {
        ((drv_pts_info_t *)p_state)->pts = 0;
        ((drv_pts_info_t *)p_state)->playtime = 0;
        ((drv_pts_info_t *)p_state)->apts = 0;
        ((drv_pts_info_t *)p_state)->aframe_count = 0;
        MLOGE("@@E: get video info error @line %d,fun %s", __LINE__, __func__);
    } else {

        MLOGD("%s_%d:vpts=%lld,gvpts=%d, apts=%lld\n",__FUNCTION__,__LINE__,
              status.stSyncStatus.u64LastVidPts,((drv_pts_info_t *)p_state)->pts, status.stSyncStatus.u64LastAudPts);
        ((drv_pts_info_t *)p_state)->pts = 0;
        ((drv_pts_info_t *)p_state)->playtime = 0;
        ((drv_pts_info_t *)p_state)->apts = 0;
        ((drv_pts_info_t *)p_state)->aframe_count = 0;
        if (((mt_s64)status.stSyncStatus.u64LastVidPts) > 0) {
            ((drv_pts_info_t *)p_state)->pts = status.stSyncStatus.u64LastVidPts;    //pPrivateInfo.u64LastPts/1000;//
        }

        if (((mt_s64)status.stSyncStatus.u64PlayTime) > 0) {
            ((drv_pts_info_t *)p_state)->playtime = status.stSyncStatus.u64PlayTime;    //pPrivateInfo.u32LastPlayTime;
        }

        if (((mt_s64)status.stSyncStatus.u64LastAudPts) > 0) {
            ((drv_pts_info_t *)p_state)->apts = status.stSyncStatus.u64LastAudPts;
        }
        if (((mt_s64)status.u32AuddFrameCount) > 0) {
            ((drv_pts_info_t *)p_state)->aframe_count = status.u32AuddFrameCount;
        }

        //pthread_mutex_unlock(&VMutex);
        //MLOGD("zx vpts up =%lld\n", status.stSyncStatus.u64LastVidPts);
    }

#endif
    return MT_SUCCESS;
}

RET_CODE vdec_get_frame_info(void *p_dev, void *p_state)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_AVPLAY_VIDEO_FRAME_INFO_S pVideoFrameInfo = {0};

    MLOGD("%s,%d\n", __func__, __LINE__);
    //pthread_mutex_lock(&VMutex);
    ret = MT_FAILURE;//MT_UNF_AVPLAY_GetVideoFrameInfo(g_avplay_es_handle, &pVideoFrameInfo);
    if (ret != MT_SUCCESS) {
        MLOGE("@@E: get video info error @line %d,fun %s", __LINE__, __func__);
    } else {
        memcpy(p_state, &pVideoFrameInfo, sizeof(MT_UNF_AVPLAY_VIDEO_FRAME_INFO_S));
    }

    //pthread_mutex_unlock(&VMutex);

    return ret;   // xingwei @20190312 for bug 107892
}


RET_CODE vdec_set_trick_mode(void *p_dev, int mode, u8 sr)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_AVPLAY_TPLAY_OPT_S pstTplayOpt;

    MLOGD("%s,%d\n", __func__, __LINE__);
    pstTplayOpt.enTplayDirect = mode;
    if (mode > MT_UNF_AVPLAY_TPLAY_DIRECT_BUTT || mode < MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD) {
        MLOGW("%s,%d\n unknow mode ! Please check!", __func__, __LINE__);
        return MT_FAILURE;

    }
    pstTplayOpt.u32SpeedInteger = sr;
    pstTplayOpt.u32SpeedDecimal = 0;

    //pthread_mutex_lock(&VMutex);
    ret =  MT_UNF_AVPLAY_Tplay(g_avplay_es_handle,  &pstTplayOpt);
    //pthread_mutex_unlock(&VMutex);

    return ret;
}

RET_CODE vdec_set_trick_mode_2(mt_s32 playrate)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_AVPLAY_TPLAY_OPT_S pstTplayOpt;
//    MT_UNF_DEC_TRICK_PARAM_S stTrickParam;

    MLOGD("%s,%d, playrate %d\n", __func__, __LINE__, playrate);
    if (playrate >= 0) {
        pstTplayOpt.enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
//        stTrickParam.trick_mode = MT_UNF_DEC_TM_FFWD;
    } else if (playrate < 0) {
        pstTplayOpt.enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_BACKWARD;
//        stTrickParam.trick_mode = MT_UNF_DEC_TM_FREV;
    }

    if (playrate == 0 || playrate == 1) {
//        stTrickParam.is_incomplete_stream = 0;
//        stTrickParam.trick_mode = MT_UNF_DEC_TM_NORMAL;
        pstTplayOpt.u32SpeedInteger = 1;
    } else {
        pstTplayOpt.u32SpeedInteger = 2;
//        stTrickParam.is_incomplete_stream = 1;
    }
    pstTplayOpt.u32SpeedDecimal = 0;

    MLOGD("%s,%d, enTplayDirect %d, u32SpeedInteger %d\n",
          __func__, __LINE__, pstTplayOpt.enTplayDirect, pstTplayOpt.u32SpeedInteger);
    //pthread_mutex_lock(&VMutex);
    ret =  MT_UNF_AVPLAY_Tplay(g_avplay_es_handle,  &pstTplayOpt);
    //pthread_mutex_unlock(&VMutex);


    //MT_MPI_VDEC_SetTrickCfg(p_file_seq->p_vdec_dev, &stTrickParam);
    return ret;
}

void vdec_get_pts_2(s64 *out_pts)
{
    MT_S32 ret = MT_SUCCESS;
    MT_UNF_AVPLAY_STATUS_INFO_S statusInfo;
    ret = MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &statusInfo);
    if (ret != MT_SUCCESS) {
        MLOGE("@@E: get video info error @line %d,fun %s", __LINE__, __func__);
        *out_pts = 0;
        return;
    }

    *out_pts = (s64)statusInfo.stSyncStatus.u64LastVidPts;
    //MLOGD("%s: u64LastVidPts[%lld]\n",__func__, (long long)*out_pts);
    return;
}

RET_CODE vdec_set_avsync_mode_2(void *p_dev, u32 mode)
{
    mt_s32 Ret;
    MT_UNF_SYNC_ATTR_S AvSyncAttr;
    Ret = MT_UNF_AVPLAY_GetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    //AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
    AvSyncAttr.enSyncRef = mode;
    Ret |= MT_UNF_AVPLAY_SetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);

    if (MT_SUCCESS != Ret) {
        MLOGD("MT_UNF_AVPLAY_SetAttr not success %s %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

#define V_ES_BUFFER_EMPTY_THRESHOLD       1024
RET_CODE vdec_check_exit(void)
{
    MT_UNF_AVPLAY_STATUS_INFO_S statusInfo;
    MT_S32 empty = MT_FALSE;;
    MT_S32 ret = MT_TRUE;
    ret = MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &statusInfo);
    if (ret == MT_SUCCESS) {
        if (statusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream
            || statusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize < V_ES_BUFFER_EMPTY_THRESHOLD) { //only video
            empty = MT_TRUE;    //Video EOS Reached!
        }
    }
    //MLOGD("%s: p_empty[%d], u32UsedSize[%d] bEndOfStream[%d]\n", __FUNCTION__, *p_empty,
    //    statusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize, statusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream);
    return empty;
}

static unsigned long long g_ves_sum = 0;
static unsigned long long g_aes_sum = 0;
static unsigned long long g_ses_sum = 0;
static int g_check_sum_max_frame = 0;
static int g_check_sum_max_asize = 0;
static int g_check_sum_acnt = 0;
static int g_get_es_checksum_valid = 0;
RET_CODE wb_auto_init_es_check_sum(int max_frame, int max_asize)
{
    g_ves_sum = 0;
    g_aes_sum = 0;
    g_ses_sum = 0;
    g_check_sum_max_frame = max_frame;
    g_check_sum_max_asize = max_asize;
    g_check_sum_acnt = 0;
    g_get_es_checksum_valid = 1;
    return (RET_CODE)MT_SUCCESS;
}

RET_CODE wb_auto_get_es_check_sum(void *ves, void *aes, void *ses)
{
    RET_CODE ret = (RET_CODE)MT_SUCCESS;
    *((unsigned long long *)ves) = g_ves_sum;
    *((unsigned long long *)aes) = (g_aes_sum & 0xfffffffffLLU) | (((unsigned long long)g_check_sum_acnt & 0xfffffffLLU) << 36);
    *((unsigned long long *)ses) = g_ses_sum;
    //MLOGD("[%s_%d][0x%llx 0x%llx 0x%llx] [0x%llx 0x%llx 0x%llx]\n",__func__,__LINE__,g_ves_sum,g_aes_sum,g_ses_sum,
    //*((unsigned long long *)ves),*((unsigned long long *)aes),*((unsigned long long *)ses));
    return ret;
}

unsigned long long get_pack_magic_num_wb(unsigned char *src, int pkt_size, u64 pts, unsigned long long last_sum, int cnt, int max_asize, int mode)
{
#define MAGIC_WAY_SELECT_EXCLUSION_NOR ^
#define MAGIC_WAY_SELECT_ADD +
#define MAGIC_WAY_SELECT_AlGORITHM MAGIC_WAY_SELECT_ADD
#define MAGIC_WAY_WINDOW (8)
    unsigned char *tmp = NULL;
    unsigned long long ret = 0LLU;
    int i;
    unsigned long long ret_tmp = 0LLU;
    switch (mode) {
        case 0:// ^
            if (pkt_size > 20) {
                tmp = src;
                for (i = 0; i < MAGIC_WAY_WINDOW; i++) {
                    ret |= (unsigned long long)tmp[i] << (i << 3);
                }
                tmp = src + (pkt_size >> 1);
                for (i = 0; i < MAGIC_WAY_WINDOW; i++) {
                    ret_tmp |= (unsigned long long)tmp[i] << (i << 3);
                }
                ret = ret MAGIC_WAY_SELECT_AlGORITHM ret_tmp;
                tmp = src + pkt_size - MAGIC_WAY_WINDOW;
                for (i = 0; i < MAGIC_WAY_WINDOW; i++) {
                    ret_tmp |= (unsigned long long)tmp[i] << (i << 3);
                }
                ret = ret MAGIC_WAY_SELECT_AlGORITHM ret_tmp;
                ret = ret MAGIC_WAY_SELECT_AlGORITHM((unsigned long long)pkt_size + (unsigned long long)pts);
            } else {
                ret = (unsigned long long)pkt_size + (unsigned long long)pts;
            }
            ret = ret MAGIC_WAY_SELECT_AlGORITHM last_sum;
            ret &= 0xfffffffffffff000LLU;
            ret += cnt;
            //MLOGD("[%s_%d]0x%llx,%d,0x%llx\n",__func__,__LINE__,ret,pkt_size,pts);
            break;
        case 1: { //checksum
            i = 0;
            while (g_check_sum_acnt < g_check_sum_max_asize && i < pkt_size) {
                ret += (unsigned long long)(*(src + i));
                g_check_sum_acnt++;
                i++;
            }
        }
        break;
        default:
            break;
    }
    //MLOGD("[%s_%d]0x%llx,%d\n",__func__,__LINE__,ret,cnt);
    return ret;
}

static int decoder_has_stopped(
    PLAYBACK_INTERNAL_T *pbi, MT_UNF_AVPLAY_BUFID_E id)
{
    if (MT_UNF_AVPLAY_BUF_ID_ES_AUD == id) {
        return (DECODE_STOP == get_adec_status(pbi));
    }

    if (MT_UNF_AVPLAY_BUF_ID_ES_VID == id) {
        return (DECODE_STOP == get_vdec_status(pbi));
    }
    return MT_TRUE;
}

static int get_es_buff_free_space(
    PLAYBACK_INTERNAL_T *pbi, MT_UNF_AVPLAY_BUFID_E id)
{
    mt_u32 all_size  = 0;
    mt_u32 used_size = 0;

    if (MT_UNF_AVPLAY_BUF_ID_ES_VID == id) {
        if (MT_SUCCESS != vdec_get_es_buf_space_info
            ((void *) pbi, &all_size, &used_size)) {
            return -1;
        }
    }

    if (MT_UNF_AVPLAY_BUF_ID_ES_AUD == id) {
        if (MT_SUCCESS != aud_get_es_buf_space_info
            ((void *) pbi, &all_size, &used_size)) {
            return -1;
        }
    }

    return (int) (all_size - used_size);
}

static int get_avplay_es_buf(void *p_dev, MT_HANDLE handle,
     MT_UNF_AVPLAY_BUFID_E id, unsigned int len, MT_UNF_STREAM_BUF_S *stream_buf)
{
    mt_s32 ret;
    const int MAX_RETRY_CNT = 500;
    int cnt = MAX_RETRY_CNT;
    PLAYBACK_INTERNAL_T *pbi = p_dev;

    stream_buf->u32Size = 0;
    stream_buf->pu8Data = NULL;
    ret = MT_UNF_AVPLAY_GetBuf(handle, id, len, stream_buf, TIME_OUT_MS_ES_PUSH);

    int first_free_size = -1;
    if (MT_SUCCESS != ret) {
        first_free_size = get_es_buff_free_space(pbi, id);
    }

    s64 push_idle_time = 0;
    while ((cnt-- > 0) && (!decoder_has_stopped(p_dev, id)) &&
           ((MT_SUCCESS != ret) || (0 == stream_buf->u32Size))) {
        mlzp_msleep(10);/* delay 10 ms */
        push_idle_time += 10;
        ret = MT_UNF_AVPLAY_GetBuf(handle, id, len, stream_buf, TIME_OUT_MS_ES_PUSH);
        /* check space free over 1s */
        if (MT_SUCCESS != ret && push_idle_time > 1000) {
            int curr_free_size =
                get_es_buff_free_space(pbi, id);
            if (curr_free_size == first_free_size && curr_free_size <= MIN_ES_BUFFER_SIZE) { /* not decode */
                if (MT_UNF_AVPLAY_BUF_ID_ES_VID == id) {
                    pbi->video.state = FPBI_STATE_DECODER_STOPPED;
                } else if (MT_UNF_AVPLAY_BUF_ID_ES_AUD == id) {
                    pbi->audio.state = FPBI_STATE_DECODER_STOPPED;
                }
                MLOGW("%s not decode over 1s, buffer %d bytes free, need %d bytes, decoder may be stopped\n",
                    (MT_UNF_AVPLAY_BUF_ID_ES_VID == id) ? "Video" : "Audio", curr_free_size, len);
                break;
            }
        }
    }

    pbi->push_idle_time += push_idle_time;
    ret = ((NULL == stream_buf->pu8Data) || (0 == stream_buf->u32Size)) ? MT_FAILURE : MT_SUCCESS;
    return (int) ret;
}

#define	AUD_DMA_TEST_CH	(0)
#define	VID_DMA_TEST_CH	(1)
extern unsigned int MT_VMXGetTAMallocPhyAddr(unsigned int vir_addr);

#ifdef DRM_SMP_ENABLE
void* ves_dmacpy(void *dst, const void *src, unsigned long size) {
	MTDRM_RESULT ret = MTDRM_SUCCESS;
	ret = MTDrm_DMACopy(DMTRM_EDMA_CH_1, dst, src, size);
   	if(ret == MTDRM_SUCCESS)
		return dst;
	else
		return NULL;
}

void* aes_dmacpy(void *dst, const void *src, unsigned long size) {
	MTDRM_RESULT ret = MTDRM_SUCCESS;
	ret = MTDrm_DMACopy(DMTRM_EDMA_CH_0, dst, src, size);
	if(ret == MTDRM_SUCCESS)
		return dst;
	else
		return NULL;
}
#endif

static int push_ves_putbuf(void *p_dev, MT_HANDLE hAvplay, const u8 *header,
    const int header_len, const u8 *es_addr, const u32 es_size, u64 vpts, int eos_flag)
{
    int ret;
    int data_remain, copy_len = 0;
    MT_UNF_STREAM_BUF_S stream_buf;
    unsigned char *es_ptr   = es_addr;
    unsigned char *hd_ptr   = header;
    unsigned char *push_buf_ptr = NULL;
    int header_len_reamin = header_len;
    u64 push_pts = (MP_NOPTS_VALUE != vpts) ? vpts : ADP_FW_INVALID_PTS;
    MT_UNF_SYNC_ATTR_S AvSyncAttr    = {0};
    MT_UNF_AVPLAY_PUTBUFEX_OPT_S opt = {
        .bContinue     = MT_TRUE,
        .bEndOfFrm     = MT_FALSE,
        .u32FrameFinsh = MT_FALSE,
        .u32EosFlag    = MT_FALSE
    };

#if defined(DRM_SMP_ENABLE)
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *)p_dev;
#endif

    VES_MEMCPY ves_memcpy;

    data_remain = (NULL != hd_ptr && header_len > 0) ? es_size + header_len : es_size;
    (void) MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    /* pts should set when the first push */
    opt.u32PtsValide = (MT_UNF_SYNC_REF_NONE == AvSyncAttr.enSyncRef) ? MT_FALSE : MT_TRUE;
    while (data_remain > 0) {
        ret = get_avplay_es_buf(p_dev, hAvplay,
                                MT_UNF_AVPLAY_BUF_ID_ES_VID, (unsigned int) data_remain, &stream_buf);
        if (MT_SUCCESS != ret) {
            return MT_FAILURE;
        }

        copy_len = (int) stream_buf.u32Size;
#if defined(DRM_SMP_ENABLE)
        if(pbi->is_drm || pbi->video.is_secure) {
            push_buf_ptr = (unsigned char*)(ulong)stream_buf.u32PhyData;
            ves_memcpy = (VES_MEMCPY) ves_dmacpy;
        } else {
            push_buf_ptr = stream_buf.pu8Data;
            ves_memcpy = (VES_MEMCPY) memcpy;
        }
#elif defined(VMX_OTT_SVP)
		push_buf_ptr = stream_buf.u32PhyData;
        ves_memcpy = (VES_MEMCPY) memcpy;
#else
        push_buf_ptr = stream_buf.pu8Data;
        ves_memcpy = (VES_MEMCPY) memcpy;
#endif
        if (stream_buf.u32Size >= (unsigned int) data_remain) {
            opt.bEndOfFrm      = MT_TRUE;
            opt.u32FrameFinsh  = MT_TRUE;
            opt.u32EosFlag     = (unsigned int) eos_flag;
            copy_len           = data_remain;
            stream_buf.u32Size = (unsigned int) data_remain;
        }

#ifdef VMX_OTT_SVP
        unsigned int phy_srcaddr = 0, phy_size = 0, psrc_mmz = 0, phy_dstaddr = 0;

        unsigned int psrc_mmz_bk = psrc_mmz;
        phy_dstaddr = push_buf_ptr;

        ret |= mt_unf_dma_check(VID_DMA_TEST_CH);
        ret |= mt_unf_dma_request_channel(VID_DMA_TEST_CH);

        /* copy file or frame header  */
        if (NULL != hd_ptr && header_len_reamin > 0) {
            int copy_header_size = MIN(header_len_reamin, copy_len);
            //ves_memcpy(push_buf_ptr, hd_ptr, (unsigned int) copy_header_size);
            printf("video Error  hd_ptr = 0x%x, copy_header_size = 0x%x\n",hd_ptr, copy_header_size);
		while(1);
            memcpy(psrc_mmz, hd_ptr, copy_header_size);
            ret |= mt_unf_dma_memcpy(VID_DMA_TEST_CH, phy_srcaddr, phy_dstaddr, copy_header_size);

    		hd_ptr       += copy_header_size;
            push_buf_ptr += copy_header_size;
    		phy_dstaddr  += copy_header_size;
    		phy_srcaddr  += copy_header_size;
    		psrc_mmz     += copy_header_size;

            copy_len          -= copy_header_size;
            data_remain       -= copy_header_size;
            header_len_reamin -= copy_header_size;
        }

        if (copy_len > 0) {
            phy_srcaddr = MT_VMXGetTAMallocPhyAddr(es_ptr);
            ret = mt_unf_dma_memcpy(VID_DMA_TEST_CH, phy_srcaddr, phy_dstaddr, copy_len);

            data_remain -= copy_len;
            es_ptr      += copy_len;
            phy_srcaddr      += copy_len;
        }

	mt_unf_dma_release_channel(VID_DMA_TEST_CH);
	//mt_unf_cipher_free(psrc_mmz_bk);
#else
        /* copy file or frame header  */
        if (NULL != hd_ptr && header_len_reamin > 0) {
            int copy_header_size = MIN(header_len_reamin, copy_len);
            ves_memcpy(push_buf_ptr, hd_ptr, (unsigned int) copy_header_size);
            hd_ptr       += copy_header_size;
            push_buf_ptr += copy_header_size;

            copy_len          -= copy_header_size;
            data_remain       -= copy_header_size;
            header_len_reamin -= copy_header_size;
        }

        if (copy_len > 0) {
            ves_memcpy(push_buf_ptr, es_ptr, (unsigned int) copy_len);
            data_remain -= copy_len;
            es_ptr      += copy_len;
        }

        write_es_dump_file((PLAYBACK_INTERNAL_T *)p_dev,
            STREAM_TYPE_VID, (void *)stream_buf.pu8Data, stream_buf.u32Size);

#endif
        ret = MT_UNF_AVPLAY_PutBuf64(hAvplay,
            MT_UNF_AVPLAY_BUF_ID_ES_VID, stream_buf.u32Size, push_pts, &opt);
        if (MT_SUCCESS != ret) {
            MLOGE("Put vdec es buf failed!\n");
            return (int)((intptr_t)es_ptr - (intptr_t)es_addr);
        }
        push_pts = ADP_FW_INVALID_PTS;
        opt.u32PtsValide = MT_FALSE;
    }
    return (int) es_size;
}

RET_CODE vdec_dec_push_es(void *p_dev, unsigned char* src_addr, u32 size, u64 vpts, int eos_flag)
{
    mt_s32 ret = MT_SUCCESS;
    MT_HANDLE hAvplay = g_avplay_es_handle;
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *) p_dev;

    MLOGD("push ves, pts:%llu sz:%d\n", vpts, size);
    if (suplayer_is_stop_send_video() == 0) {
        return MT_SUCCESS;
    }

    push_fcnt++;
    pthread_mutex_lock(&fp_vmutex);
    if (g_get_es_checksum_valid && push_fcnt < g_check_sum_max_frame) {
        g_ves_sum = get_pack_magic_num_wb((unsigned char *)src_addr, (int)size, vpts, g_ves_sum, push_fcnt, 0, 0);
    }

#if defined(VMX_OTT_SVP) || !defined(CFG_ENABLE_FFMPEG_422)
    ret = push_ves_putbuf((void *)pbi, hAvplay, NULL, 0,
        (u8 *)((intptr_t)src_addr), size, vpts, eos_flag);
#else
    /* Set data Null when Push last padding 0 of size bytes */
    AVPacket packet = {.data = eos_flag ? NULL : src_addr, .size = size, .pts = vpts};
    AVPacket *pkt = pbi->video.ff_av_pkt;

    if (!pkt || pkt->data != src_addr) {
        pkt = &packet;
    }
    ret =  drv_adp_push_video((void *) pbi, &pbi->video.mtavf,
        (void *) pkt, (u8 *)((intptr_t)src_addr), size, vpts, eos_flag);
#endif

    pthread_mutex_unlock(&fp_vmutex);
    if (ret < 0) {
        return ret;
    } else {
        return size;
    }
}

#if MT_DES("VPX PUSH", 1)
#define VPX_START_CODE_SIZE     16
#define VPX_FILE_HEADER_SIZE    48
#define VPX_FRAME_HEAER_SZIE    28
#define VPX_MAX_SUPER_FRAME_NUM 8

typedef struct vp9_superframe_info {
    int superframe_num;
    int size[VPX_MAX_SUPER_FRAME_NUM];
} vp9_superframe_info_t;

typedef int (*vpx_superframe_parser_t)(
    const unsigned char *, const unsigned int, vp9_superframe_info_t *);

static void vpx_init_file_header(
    FILE_SEQ_VIDEO_T *vinfo, u8 *buff)
{
    u8      *ivf_hdr        = buff;
    int total_frames        = 0;
    m_rational_t frame_rate = vinfo->frame_rate;

    if (frame_rate.den > 0 && frame_rate.num > 0) {
        total_frames = (int)((vinfo->duration *
                              m_round(m_q2d(frame_rate))) / 1000);
    }

    MWR_LE32(ivf_hdr, MK_TAG32('D', 'K', 'I', 'F'));
    ivf_hdr += 4;
    MWR_LE16(ivf_hdr, 0);  // version
    ivf_hdr += 2;
    MWR_LE16(ivf_hdr, VPX_FILE_HEADER_SIZE - VPX_START_CODE_SIZE); // header length
    ivf_hdr += 2;
    if (vinfo->codec_id == vVIDEO_VP8) {
        MWR_LE32(ivf_hdr, MK_TAG32('V', 'P', '8', '0'));
        ivf_hdr += 4;
    } else if (vinfo->codec_id == vVIDEO_VP9) {
        MWR_LE32(ivf_hdr, MK_TAG32('V', 'P', '9', '0'));
        ivf_hdr += 4;
    }
    MWR_LE16(ivf_hdr, (unsigned short) vinfo->width);
    ivf_hdr += 2;
    MWR_LE16(ivf_hdr, (unsigned short) vinfo->height);
    ivf_hdr += 2;
    MWR_LE32(ivf_hdr, frame_rate.num); // frame rate
    ivf_hdr += 4;
    MWR_LE32(ivf_hdr, frame_rate.den); // time scale
    ivf_hdr += 4;
    MWR_LE32(ivf_hdr, total_frames);   // number of frames in file
    ivf_hdr += 4;
    MWR_LE32(ivf_hdr, 0);              // unused
    ivf_hdr += 4;

    if (frame_rate.den > 0 && frame_rate.num > 0) {
        vinfo->need_insert_header = 0;
        MLOGD("Vpx frame rate(%d %d)\n", frame_rate.num, frame_rate.den);
    }
}

static void vp9_init_frame_header(
    int pkt_size, u64 ipts, u8 *out_data)
{
    int i;
    u64 pts  = ipts;
    int size = pkt_size;
    u8 *hdr  = out_data;

    for (i = 0; i < sizeof(size); i++) {
        *hdr++ = size & 0xff;
        size   = size >> 8;
    }

    for (i = 0; i < sizeof(pts); i++) {
        *hdr++ = pts & 0xff;
        pts    = pts >> 8;
    }
}

static int parse_vp9_superframe_info(const unsigned char *in_data,
    const unsigned int in_size, vp9_superframe_info_t *info)
{
    int i, j, marker;
    unsigned char *data = in_data;

    marker = data[in_size - 1];
    /* 1. parsing the final byte of the chunk and
     * checking that the superframe_marker equals 0b110
     */
    if ((marker & 0xe0) != 0xc0) {
        return MT_SUCCESS;
    }

    int bytes_per_framesize  = 1 + ((marker >> 3) & 0x3);
    int frames_in_superframe = 1 + (marker & 0x7);
    /* 2. setting the total size of the superframe_index equal to 2 + NumFrames * SzBytes */
    int superframe_index_size = 2 + frames_in_superframe * bytes_per_framesize;
    int first_superframe_header_pos = in_size - superframe_index_size;
    /* 3. checking that the first byte of the superframe_index matches the final byte */
    if (in_size < superframe_index_size ||
        data[first_superframe_header_pos] != marker) {
        return MT_SUCCESS;
    }

    int64_t total_size = 0;
    data += first_superframe_header_pos + 1;
    for (i = 0; i < frames_in_superframe; i++) {
        unsigned int frame_size = 0;
        for (j = 0; j < bytes_per_framesize; j++) {
            frame_size |= *data++ << (j * 8);
        }

        total_size += (int64_t) frame_size;
        if (frame_size < 0 || total_size > in_size - superframe_index_size) {
            MLOGW("Invalid frame size in superframe: %d\n", frame_size);
            return MT_FAILURE;
        }
        info->size[i] = (int) frame_size;
    }
    MLOGD("Get superframe, num:%d\n", frames_in_superframe);
    info->superframe_num = frames_in_superframe;
    return MT_SUCCESS;
}

static vpx_superframe_parser_t vpx_get_superframe_parser(void)
{
#ifdef DRM_SMP_ENABLE
    FILE_SEQ_T * p_file_seq = file_seq_get_instance();
    if (MTDrm_GetDecryptType(p_file_seq->drm_eDrmIndex)) {
        return (vpx_superframe_parser_t) MTDrm_ParseVp9SuperframeInfo;
    }
    return (vpx_superframe_parser_t) parse_vp9_superframe_info;
#else
    return (vpx_superframe_parser_t) parse_vp9_superframe_info;
#endif
}

RET_CODE vdec_dec_push_vpx_es(void *p_dev, u8 *addr, u32 size, u64 vpts, int eos_flag)
{
    int i;
    int ret  = MT_SUCCESS;
    int need_push_size  = 0;
    int total_push_size = 0;
    int header_len      = VPX_FRAME_HEAER_SZIE;
    MT_HANDLE hAvplay   = g_avplay_es_handle;
    vp9_superframe_info_t superframe_info = {1, {size}};
    PLAYBACK_INTERNAL_T *pbi   = (PLAYBACK_INTERNAL_T *)p_dev;
    FILE_SEQ_VIDEO_T    *vinfo = &(pbi->video);
    struct {
        unsigned char file[VPX_FILE_HEADER_SIZE];
        unsigned char frame[VPX_FRAME_HEAER_SZIE];
    } header = {
        "SHANGHAIMONTSOC\x0", "SHANGHAIMONTSOC\x1"
    };
    unsigned char *hd_ptr =
        (unsigned char *)((uintptr_t)(&(header.frame[0])));

#if !defined(VMX_OTT_SVP) && defined(CFG_ENABLE_FFMPEG_422)
    AVPacket packet = {.data = eos_flag ? NULL : addr, .size = size, .pts = vpts};
    AVPacket *pkt = pbi->video.ff_av_pkt;

    if (!pkt || pkt->data != addr) {
        pkt = &packet;
    }
    ret =  drv_adp_push_video((void *) pbi, &pbi->video.mtavf,
        (void *) pkt, (u8 *)((intptr_t)addr), size, vpts, eos_flag);
    if (ret < 0) {
        return ret;
    }else{
        return size;
    }
#endif

    if (vVIDEO_VP9 == vinfo->codec_id) {
        ret = vpx_get_superframe_parser()(addr, size, &superframe_info);
        if (MT_SUCCESS != ret) {
            return total_push_size;
        }
    }

    if (vinfo->need_insert_header) {
        hd_ptr = (u8 *)((uintptr_t)(&(header.file[0])));
        vpx_init_file_header(vinfo, (u8 *)((uintptr_t)hd_ptr + VPX_START_CODE_SIZE));
        header_len += VPX_FILE_HEADER_SIZE;
    }

    for (i = 0; i < superframe_info.superframe_num; i++) {
here:
        need_push_size = superframe_info.size[i];
        if (0 != i) {
            hd_ptr = (u8 *)((uintptr_t)(&(header.frame[0])));
            header_len = VPX_FRAME_HEAER_SZIE;
        }

        if (vdec_status == DECODE_STOP) {
            return MT_FALSE;
        }

        vp9_init_frame_header(need_push_size, vpts,
                              (u8 *)((uintptr_t)(&(header.frame[0])) + VPX_START_CODE_SIZE));

        MLOGD("Push vpx ves, pts:%llu sz:%d\n", vpts, need_push_size);
        int curr_push_size = push_ves_putbuf(p_dev, hAvplay, hd_ptr,
            header_len, addr + total_push_size, need_push_size, vpts, eos_flag);
        if(MT_FAILURE == curr_push_size) {
            mtos_task_sleep(10);
            goto here;
        }

        total_push_size += curr_push_size;
        if (curr_push_size < need_push_size) {
            break;
        }
    }

    return total_push_size;
}
#endif
RET_CODE vdec_do_avsync_cmd(void *p_dev, int cmd, u32 data)
{
    RET_CODE ret = MT_SUCCESS;
#if 0
    MT_UNF_VDEC_AVSYNC_CMD_t avsync_cmd;
    MLOGD("%s,%d\n", __func__, __LINE__);
    pthread_mutex_lock(&VMutex);
    avsync_cmd.cmd = cmd;
    avsync_cmd.data = data;
    ret = MT_UNF_AVPLAY_vdec_AvsyncCmd(g_avplay_es_handle, avsync_cmd);
    pthread_mutex_unlock(&VMutex);
#else
    MLOGD("%s,%d\n", __func__, __LINE__);
#endif
    return ret;

}

static char g_backup_aud_param[128];
static int g_backup_aud_param_valid = 0;
RET_CODE wb_auto_aud_init_dec_param(void)
{
    memset(g_backup_aud_param, 0, sizeof(g_backup_aud_param));
    g_backup_aud_param_valid = 1;
    return (RET_CODE)MT_SUCCESS;
}

RET_CODE wb_auto_aud_get_dec_param(void *ptr, int size)
{
    RET_CODE ret = (RET_CODE)MT_FAILURE;
    if (size > sizeof(g_backup_aud_param)) {
        memcpy(ptr, g_backup_aud_param, sizeof(g_backup_aud_param));
    } else {
        memcpy(ptr, g_backup_aud_param, size);
    }
    if (g_backup_aud_param_valid == 2) {
        ret = (RET_CODE)MT_SUCCESS;
    }
    g_backup_aud_param_valid = 0;
    return ret;
}

RET_CODE aud_set_dec_param_vsb(void *p_dev, const void *p_param, int aType)
{
    MT_UNF_ACODEC_ATTR_S AdecAttr;
    HA_CODEC_ID_E atype = aType;
    WAV_FORMAT_S stWavFormat;
    mt_s32 s32Ret;
    MT_UNF_AVPLAY_STOP_OPT_S stop = {0};

    MT_UNF_AVPLAY_GetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_ADEC, (mt_void *)&AdecAttr);
    if (HA_AUDIO_ID_PCM == atype) {

        const WAV_FORMAT_S *pstWavFormat = (const WAV_FORMAT_S *)p_param;
        stWavFormat.nSamplesPerSec = pstWavFormat->nSamplesPerSec;
        stWavFormat.nChannels = pstWavFormat->nChannels;  //support multi-channels
        stWavFormat.wBitsPerSample = pstWavFormat->wBitsPerSample;
        stWavFormat.cbExtWord[0] = pstWavFormat->cbExtWord[0];
        stWavFormat.cbExtWord[1] = pstWavFormat->cbExtWord[1];

        stWavFormat.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        if (g_backup_aud_param_valid) {
            if (sizeof(WAV_FORMAT_S) < sizeof(g_backup_aud_param)) {
                memset(g_backup_aud_param, 0, sizeof(g_backup_aud_param));
                ((WAV_FORMAT_S *)g_backup_aud_param)->nSamplesPerSec = pstWavFormat->nSamplesPerSec;
                ((WAV_FORMAT_S *)g_backup_aud_param)->nChannels = pstWavFormat->nChannels;
                ((WAV_FORMAT_S *)g_backup_aud_param)->wBitsPerSample = pstWavFormat->wBitsPerSample;
                ((WAV_FORMAT_S *)g_backup_aud_param)->cbExtWord[0] = pstWavFormat->cbExtWord[0];
                ((WAV_FORMAT_S *)g_backup_aud_param)->cbExtWord[1] = pstWavFormat->cbExtWord[1];
                ((WAV_FORMAT_S *)g_backup_aud_param)->wFormatTag = 0xffee;
            }
            g_backup_aud_param_valid = 2;
        }
        HA_PCM_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), &stWavFormat);
        MLOGI("%s, ch %d bits %d, sr %d\n", __func__,
              (int) stWavFormat.nChannels, (int) stWavFormat.wBitsPerSample, stWavFormat.nSamplesPerSec);
    } else if (HA_AUDIO_ID_COOK == atype   ||
               HA_AUDIO_ID_AMRNB == atype  ||
               HA_AUDIO_ID_AMRWB == atype) {
        HA_FFMPEG_DECODE_OPENCONFIG_S *pstConfig = (HA_FFMPEG_DECODE_OPENCONFIG_S *)u8DecOpenBuf1;
        const HA_FFMPEG_DECODE_OPENCONFIG_S *pffmpge_cfg = (const HA_FFMPEG_DECODE_OPENCONFIG_S *)p_param;
        HA_FFMPEG_DecGetDefalutOpenConfig(pstConfig);

        pstConfig->hAvCtx = pffmpge_cfg->hAvCtx;
        if (g_backup_aud_param_valid) {
            if (sizeof(HA_FFMPEG_DECODE_OPENCONFIG_S) < sizeof(g_backup_aud_param)) {
                memset(g_backup_aud_param, 0, sizeof(g_backup_aud_param));
            }
            g_backup_aud_param_valid = 2;
        }
        HA_FFMPEGC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    } else if (HA_AUDIO_ID_WMA9STD == atype) {
        if (g_backup_aud_param_valid) {
            memset(g_backup_aud_param, 0, sizeof(g_backup_aud_param));
            if (sizeof(WMA_FORMAT_S) < sizeof(g_backup_aud_param)) {
                memcpy(g_backup_aud_param, p_param, sizeof(WMA_FORMAT_S));
            } else {
                memcpy(g_backup_aud_param, p_param, sizeof(g_backup_aud_param));
            }
            g_backup_aud_param_valid = 2;
        }
        HA_WMA_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), p_param);
    }

    s32Ret = MT_UNF_AVPLAY_SetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_ADEC, (mt_void *)&AdecAttr);
    //Sometimes the audio is still running, and the setting will fail, which will result in no sound
    if(s32Ret == MT_ERR_AVPLAY_INVALID_OPT)
    {
        stop.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
        stop.u32TimeoutMs = 0;
        MT_UNF_AVPLAY_Stop(g_avplay_es_handle, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stop);
        MT_UNF_AVPLAY_SetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_ADEC, (mt_void *)&AdecAttr);
        MT_UNF_AVPLAY_Start(g_avplay_es_handle, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    }
    return MT_SUCCESS;
}


RET_CODE aud_pause_vsb(void *p_dev)
{
    mt_s32 ret = 0;

    MT_UNF_AVPLAY_STATUS_INFO_S status;
    if (adec_status != DECODE_START) {
        MLOGW("%s,%d adec_status: %d, return\n", __func__, __LINE__, adec_status);
        return ret;
    }

    MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &status);
    MLOGI("%s,%d Status:%d\n", __func__, __LINE__, status.enRunStatus);
    if (status.enRunStatus != MT_UNF_AVPLAY_STATUS_PAUSE) {
        MLOGW("%s fail, try again\n", __func__, __LINE__);
        ret =  MT_UNF_AVPLAY_Pause(g_avplay_es_handle, MT_NULL);
        //pthread_mutex_unlock(&VMutex);
    }
    adec_status = DECODE_PAUSE;

    return ret;
}

RET_CODE aud_resume_vsb(void *p_dev)
{
    mt_s32 ret = 0;
    MT_UNF_AVPLAY_STATUS_INFO_S status;

    if (adec_status != DECODE_PAUSE) {
        MLOGW("%s adec_status %d\n", __func__, adec_status);
        return ret;
    }

    MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &status);
    MLOGI("%s Status:%d\n", __func__, status.enRunStatus);
    if (status.enRunStatus == MT_UNF_AVPLAY_STATUS_PAUSE) {
        MLOGW("%s fail, try again\n", __func__, __LINE__);
        ret =  MT_UNF_AVPLAY_Resume(g_avplay_es_handle, MT_NULL);
    }
    adec_status = DECODE_START;

    return ret;
}

RET_CODE aud_start_vsb(void *p_dev, int type)
{
    mt_s32                  ret = MT_SUCCESS;
    mt_s32 s32DtsDtsCoreOnly = 0;
    MT_HA_DECODEMODE_E enAudioDecMode = HD_DEC_MODE_RAWPCM;
    mt_s32 AdecType = type;

    MLOGI("%s type:%s, adec_status:%d\n", __func__, mdrv_adec_type_to_str(type), adec_status);
    if (adec_status == DECODE_START) {
        MLOGW("[%s] adec_status: DECODE_START, return\n", __func__);
        return MT_SUCCESS;
    }

    FILE_SEQ_T *p_file_seq   = (FILE_SEQ_T *)p_dev;
    PLAYBACK_INTERNAL_T *pbi = p_file_seq->pb_internal;
    ret = MTADP_AVPlay_SetAdecAttr_internal((PLAYBACK_INTERNAL_T *) pbi,
            g_avplay_es_handle, (mt_u32)AdecType, enAudioDecMode, s32DtsDtsCoreOnly);
    if (ret != MT_SUCCESS) {
        MLOGE("MT_UNF_AVPLAY_SetAttr failed\n");
    }
    ret = MT_UNF_AVPLAY_Start(g_avplay_es_handle, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (ret != MT_SUCCESS) {
        MLOGE(" MT_UNF_AVPLAY_Start audio failed.\n");
    }
    adec_status = DECODE_START;
    //pthread_mutex_unlock(&VMutex);

    return ret;
}

int push_aud_cnt = 0;
RET_CODE aud_stop_vsb(void *p_dev)
{
    mt_s32 ret = 0;
    MT_UNF_AVPLAY_STOP_OPT_S stop;

    MLOGI("%s,%d, adec_status:%d\n", __func__, __LINE__, adec_status);
    if (adec_status == DECODE_STOP) {
        MLOGW("[%s] adec_status: DECODE_STOP, return\n", __func__);
        return MT_SUCCESS;
    }

    push_aud_cnt = 0;
    adec_status = DECODE_STOP;
    stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stop.u32TimeoutMs = 0;
    pthread_mutex_lock(&fp_amutex);
    ret = MT_UNF_AVPLAY_Stop(g_avplay_es_handle, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stop);
    pthread_mutex_unlock(&fp_amutex);
    if (ret != MT_SUCCESS) {
        MLOGE(" MT_UNF_AVPLAY_Stop failed.\n");
    }
    return ret;
}

RET_CODE aud_file_getleft_ao_pcm_bytes(void *p_dev, u32 *p_size)
{
    int ret = MT_FAILURE;
    if (!p_dev || !p_size) {
        return ret;
    }

#ifdef CONFIG_MT_MONTAGE_PLATFORM
    MT_UNF_SND_BUF_INFO_S info = {0};
    ret = MT_UNF_SND_GetBufInfo(MT_UNF_SND_0, &info);
    if (MT_SUCCESS != ret) {
        MLOGW("MT_UNF_SND_GetBufInfo fail\n");
        return ret;
    }

    MLOGD("[Ao] pcm bytes:%d valid:%d spd size:%d valid:%d\n",
        info.pcm_size, info.pcm_valid, info.spd_size, info.spd_valid);
    *p_size = (u32) info.pcm_valid;
#else
    *p_size = 1024;
#endif
    return ret;
}

static int aud_get_es_buf_space_info(void *p_dev, mt_u32 *all_size, mt_u32 *use_size)
{
    mt_s32 ret;
    MT_UNF_AVPLAY_STATUS_INFO_S pstStatusInfo;
    MT_UNF_AVPLAY_BUF_STATUS_S *pAudBufInfo;

    (void) p_dev;
    ret = MT_UNF_AVPLAY_GetAudioStatusInfo(g_avplay_es_handle, &pstStatusInfo);
    if (ret != MT_SUCCESS) {
        MLOGW("MT_UNF_AVPLAY_GetStatusInfo fail\n");
        return ret;
    }

    pAudBufInfo = &(pstStatusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD]);
    if (all_size) {
        *all_size = pAudBufInfo->u32BufSize;
    }
    if (use_size) {
        *use_size = pAudBufInfo->u32UsedSize;
    }

    return MT_SUCCESS;
}

RET_CODE aud_file_getleftesbuffer_vsb(void *p_dev, u32 *p_size)
{
    u32 all_buf_size, used_size;

    all_buf_size = used_size = 0;
    if (MT_SUCCESS != aud_get_es_buf_space_info(
            p_dev, &all_buf_size, &used_size)) {
        return MT_FAILURE;
    }

    int free_space = all_buf_size - used_size;
    if (free_space > 0) {
        *p_size = (u32) free_space;
    } else {
        *p_size = 0;
        MLOGW("No audio es buffer!\n");
    }

    return MT_SUCCESS;
}

RET_CODE aud_file_get_water_level_vsb(void *p_dev)
{
    #define ES_BUFFER_WATER_LEVEL   80
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)p_dev;
    u32 all_buf_size, used_size;

    if (MT_SUCCESS != aud_get_es_buf_space_info(
            p_dev, &all_buf_size, &used_size)) {
        return MT_FAILURE;
    }

    if(p_internal != NULL) {
        p_internal->vitual_audio_es_cur_size = all_buf_size*ES_BUFFER_WATER_LEVEL/100 - used_size;
    }

    if(used_size > all_buf_size*ES_BUFFER_WATER_LEVEL/100) {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

RET_CODE aud_file_get_es_buf_info(void *p_dev, mt_u32 *all_size, mt_u32 *use_size)
{
    u32 all_buf_size, used_size;
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)p_dev;

    all_buf_size = used_size = 0;
    if (MT_SUCCESS != aud_get_es_buf_space_info(
            p_dev, &all_buf_size, &used_size)) {
        return MT_FAILURE;
    }

    if (all_size) {
        *all_size = all_buf_size;
    }
    if (use_size) {
        *use_size = used_size;
    }

    return MT_SUCCESS;
}

RET_CODE aud_file_gettotalesbuffer_vsb(void *p_dev, u32 *p_size)
{
    mt_s32 ret;
    MT_UNF_AVPLAY_ATTR_S avplay_attr;

    MLOGA("%s,%d\n", __func__, __LINE__);
    PLAYBACK_INTERNAL_T *p_internal = (PLAYBACK_INTERNAL_T *)p_dev;

    if (p_internal == NULL || p_size == NULL) {
        return MT_FAILURE;
    }

    if (p_internal->vitual_audio_es_max_size == 0) {
        //pthread_mutex_lock(&VMutex);
        //MT_UNF_AVPLAY_GetBufInfo(g_avplay_es_handle,MT_UNF_AVPLAY_VID_ES_BUF,&bufinfo);
        ret = MT_UNF_AVPLAY_GetAttr(g_avplay_es_handle, MT_UNF_AVPLAY_ATTR_ID_STREAM_MODE, &avplay_attr);
        if (ret == MT_SUCCESS) {
            *p_size = avplay_attr.stStreamAttr.u32AudBufSize;
            MLOGD("%s,%d aud buf size: %d k\n", __func__, __LINE__, avplay_attr.stStreamAttr.u32AudBufSize);

        } else {
            MLOGE("%s,%d error: %d\n", __func__, __LINE__, ret);
        }
        //pthread_mutex_unlock(&VMutex);
        p_internal->vitual_audio_es_max_size = (int)(*p_size);
        //MLOGD("zx p_internal->vitual_audio_es_max_size %d\n", p_internal->vitual_audio_es_max_size);
    } else {
        *p_size = (u32)(p_internal->vitual_audio_es_max_size);
    }

    return MT_SUCCESS;
}

static int aud_file_pushesbuffer_vsb_vmx(void *p_dev, u8 *src_addr,
			u32 size, u64 apts, u32 eos)
{
 	unsigned int AUDIO_FRAME_MAX = 0x10000;
	mt_s32 pushsize;
	mt_s32 ret  = MT_SUCCESS;
	int audioFrameLen  = (int)size;
	MT_UNF_STREAM_BUF_S StreamBuf;
	MT_HANDLE hAvplay  = g_avplay_es_handle;
	mt_s32 totalsize   = 0;
	mt_u8 *p_addr      = (mt_u8 *)src_addr;

	MT_UNF_AVPLAY_PUTBUFEX_OPT_S putopt = {
        .bContinue     = MT_TRUE,
        .bEndOfFrm     = MT_FALSE,
        .u32PtsValide  = MT_TRUE,
        .u32FrameFinsh = MT_FALSE,
        .u32EosFlag    = MT_FALSE
    };

#if defined(DRM_SMP_ENABLE)
    PLAYBACK_INTERNAL_T *pbi = p_dev;

    VES_MEMCPY aes_memcpy;
    if (pbi->is_drm || pbi->audio.is_secure) {
        aes_memcpy = (VES_MEMCPY) aes_dmacpy;
    } else {
        aes_memcpy = (VES_MEMCPY) memcpy;
    }
#endif

	while (audioFrameLen > 0) {

		if (adec_status == DECODE_STOP) {
			break;
		}

		pushsize = audioFrameLen;
		if (pushsize > AUDIO_FRAME_MAX) {
		pushsize = AUDIO_FRAME_MAX;
		}
        ret = MT_UNF_AVPLAY_GetBuf(hAvplay,
           MT_UNF_AVPLAY_BUF_ID_ES_AUD, (mt_u32)pushsize, &StreamBuf, TIME_OUT_MS_ES_PUSH);

#ifdef DRM_SMP_ENABLE
        phys_addr_t phy_addr1 = 0;
        phys_addr_t phy_addr2 = 0;
        if (pbi->is_drm || pbi->audio.is_secure) {
            phy_addr1 = StreamBuf.u32PhyData;
        } else {
            phy_addr1 = (phys_addr_t)(ulong)StreamBuf.pu8Data;
        }

        if (StreamBuf.u32PhyData2 != NULL) {
            if (pbi->is_drm || pbi->audio.is_secure) {
                phy_addr2 = StreamBuf.u32PhyData2;
            } else {
                phy_addr2 = (phys_addr_t)(ulong)StreamBuf.pu8Data2;
            }
        }

        if (MT_SUCCESS != ret || \
            StreamBuf.u32Size <= 0 || \
            (0 == phy_addr1) || \
            (StreamBuf.u32Size + StreamBuf.u32Size2) != pushsize) {
            mtos_task_sleep(100);
            continue;
        }

		aes_memcpy((void *)(ulong)(phy_addr1),(void *)(p_addr + totalsize), StreamBuf.u32Size);

        if (phy_addr2 != NULL) {
            aes_memcpy((void *)(ulong)(phy_addr2),
                       (void *)(ulong)(p_addr + totalsize + StreamBuf.u32Size), StreamBuf.u32Size2);
        }
#else
        if (MT_SUCCESS != ret || StreamBuf.u32Size <= 0) {
            mtos_task_sleep(100);
            continue;
        }
#ifdef VMX_OTT_SVP
        /*push es data*/
        unsigned int phy_srcaddr = 0, phy_size = 0, psrc_mmz = 0, phy_dstaddr = 0;
        ret = mt_mmz_get_phyaddr((void *)StreamBuf.pu8Data, &phy_dstaddr, &phy_size);
        phy_srcaddr = MT_VMXGetTAMallocPhyAddr(p_addr);
        ret |= mt_unf_dma_check(AUD_DMA_TEST_CH);
        ret |= mt_unf_dma_request_channel(AUD_DMA_TEST_CH);
        ret |= mt_unf_dma_memcpy(AUD_DMA_TEST_CH, phy_srcaddr, phy_dstaddr, pushsize);
        mt_unf_dma_release_channel(AUD_DMA_TEST_CH);
#else
        if (pushsize == StreamBuf.u32Size) { // fix20223 by xingwei@20230625
            memcpy(StreamBuf.pu8Data, p_addr + totalsize, (unsigned int)pushsize);
            write_es_dump_file((PLAYBACK_INTERNAL_T *)p_dev, STREAM_TYPE_AUD, (void *)StreamBuf.pu8Data, pushsize);
        } else { //for rollback case
            if (pushsize > StreamBuf.u32Size) {
                MLOGD("pushsize %d u32Size %d StreamBuf.u32Size2 %d \n", pushsize, StreamBuf.u32Size, StreamBuf.u32Size2);

                if ((StreamBuf.u32Size2 != 0) && ((StreamBuf.u32Size + StreamBuf.u32Size2) == pushsize)) {
                    memcpy(StreamBuf.pu8Data, p_addr + totalsize, (unsigned int)StreamBuf.u32Size);
                    write_es_dump_file((PLAYBACK_INTERNAL_T *)p_dev, STREAM_TYPE_AUD, (void *)StreamBuf.pu8Data, StreamBuf.u32Size);

                    memcpy(StreamBuf.pu8Data2, p_addr + totalsize + StreamBuf.u32Size, (unsigned int)StreamBuf.u32Size2);
                    write_es_dump_file((PLAYBACK_INTERNAL_T *)p_dev, STREAM_TYPE_AUD, (void *)StreamBuf.pu8Data2, StreamBuf.u32Size2);
                } else {
                    MLOGD("can't happend LINE:%d\n", __LINE__);
                    mtos_task_sleep(20);
                    continue;
                }
            } else {
                MLOGD("can't happend LINE:%d\n", __LINE__);
                mtos_task_sleep(20);
                continue;
            }
        }
#endif
#endif

#ifdef VMX_OTT_SVP  /*set MT_UNF_AVPLAY_PUTBUFEX_OPT_S putopt  */
        memset(&putopt, 0, sizeof(MT_UNF_AVPLAY_PUTBUFEX_OPT_S));
        putopt.bContinue = MT_TRUE;
        putopt.bEndOfFrm = MT_TRUE;
        putopt.u32EosFlag = eos;
        putopt.u32FrameFinsh = 0;
        putopt.u32PtsValide = 1;
#else
        if (audioFrameLen <= StreamBuf.u32Size) {
            putopt.bEndOfFrm  = MT_TRUE;
            putopt.u32EosFlag = eos;
        }
#endif
        ret = MT_UNF_AVPLAY_PutBuf64(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, (mt_u32)pushsize, apts, &putopt);

        if (MT_SUCCESS != ret) {
            MLOGE("Put adec es buf failed!\n");
        }

#ifdef VMX_OTT_SVP
        apts = (u64)(-1LL);
#endif
        audioFrameLen -= pushsize;
        totalsize     += pushsize;
        putopt.u32PtsValide = MT_FALSE;
    }
	return ret;
}

RET_CODE aud_file_pushesbuffer_vsb(void *p_dev, u8 *src_addr, u32 size, u64 apts, u32 eos)
{
    mt_s32 ret  = MT_SUCCESS;

    MLOGD("push aes, pts:%llu sz:%d\n", apts, size);
    if (suplayer_is_stop_send_audio() == 0) {
        MLOGD("\nDont push audio data!!!\n");
        return ret;
    }

	PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *)p_dev;

    push_aud_cnt++;
    pthread_mutex_lock(&fp_amutex);
    if (g_get_es_checksum_valid && g_check_sum_acnt < g_check_sum_max_asize) {
        g_aes_sum = get_pack_magic_num_wb((unsigned char *)src_addr, (int)size, apts, g_aes_sum, push_aud_cnt, g_check_sum_max_asize, 1);
    }
#if !defined(VMX_OTT_SVP) && defined(CFG_ENABLE_FFMPEG_422)
    /* Set data Null when Push last padding 0 of size bytes */
    AVPacket packet = {.data = (eos ? NULL : src_addr), .size = size, .pts = apts};
    AVPacket *pkt = pbi->audio.ff_av_pkt ;

    if (!pkt || pkt->data != src_addr) {
        pkt = &packet;
    }
    ret = drv_adp_push_audio((void *) pbi, &pbi->audio.mtavf,
        (void *) pkt, (u8 *)((intptr_t)src_addr), size, apts, eos);
    pthread_mutex_unlock(&fp_amutex);
    return ret;
#endif

	ret = aud_file_pushesbuffer_vsb_vmx((void *) pbi,src_addr,size,apts,eos);
    pthread_mutex_unlock(&fp_amutex);
    return ret;
}

RET_CODE disp_set_tv_sys(void *p_dev, int ch, int fmt)
{
    return 0;
}
RET_CODE disp_get_tv_sys(void *p_dev, int ch, int *p_fmt)
{
    return 0;
}
unsigned char *audio_free_es_tmp_buf(void *p_dev)
{
    MLOGD("%s,%d\n", __func__, __LINE__);
    if (p_es_aes_buf) {
        mtos_free(p_es_aes_buf);
        p_es_aes_buf = 0;
    }

    return MT_SUCCESS;
}

unsigned char *audio_get_es_tmp_buf(void *p_dev)
{
    MLOGD("%s,%d\n", __func__, __LINE__);
    p_es_aes_buf = mtos_malloc(AES_TMP_BUF_LEN);
    MLOGD("%s,%d, AES_TMP_BUF_LEN:%d\n", __func__, __LINE__, AES_TMP_BUF_LEN);
    return p_es_aes_buf;
}

RET_CODE aud_get_pts(void *p_dev, void *p_state)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_AVPLAY_STATUS_INFO_S status = {{0}};
    drv_pts_info_t *pts_info = (drv_pts_info_t *) p_state;

    if (NULL == p_state) {
        return MT_FAILURE;
    }

    pts_info->pts = 0;
    pts_info->playtime = 0;
    ret = MT_UNF_AVPLAY_GetStatusInfo(g_avplay_es_handle, &status);
    if (ret != MT_SUCCESS) {
        MLOGE("get audio info error!\n");
        return ret;
    }

    if (((mt_s64)status.stSyncStatus.u64LastAudPts) > 0) {
        pts_info->pts = status.stSyncStatus.u64LastAudPts;
    }

    if (((mt_s64)status.stSyncStatus.u64PlayTime) > 0) {
        pts_info->playtime = status.stSyncStatus.u64PlayTime;
    }

    return ret;
}

unsigned short *Convert_Utf8_To_Unicode(unsigned char *putf8, unsigned short *out)
{
    //peacer add
    int i = 0, k = 0, len = 0;
    MLOGD("%s,%d\n", __func__, __LINE__);
    len = (int)strlen((char *)putf8);

    unsigned short *result = (unsigned short *)out;
    //mtos_printk("%s %d !!!!!!!!!\n", putf8, len);

    /*
        result = (unsigned short *)putf8;
        mtos_printk("[%s : %d ]input str = %s  outstr = %s !!!!!!!!!\n", __func__,__LINE__,putf8, result);
        return  result ;
    */

    if (result) {
        k = 0;

        if (k >= 256) {
            MLOGE("[%s] file name is too long !!!!!!!\n", __func__);
            return NULL;
        }

        for (i = 0; i < len;) {
            if (0 == (0x80 & putf8[i])) {//one byte
                result[k++] = putf8[i++];
                continue;
            }

            if (0xe0 == (0xe0 & putf8[i])) {//three byte
                result[k] = (unsigned short)((putf8[i] & 0x1F) << 12);
                result[k] |= (unsigned short)((putf8[i + 1] & 0x3F) << 6);
                result[k++] |= (unsigned short)(putf8[i + 2] & 0x3F);
                i += 3;
                continue;
            }

            if (0xc0 == (0xc0 & putf8[i])) {//two byte
                result[k] = (putf8[i] & 0x1F) << 8;
                result[k++] |= (unsigned short)((putf8[i + 1] & 0x3F));
                i += 2;
            }
        }

        result[k++] = 0;
    }
    return result;
}


RET_CODE vdec_set_dec_frm_type(void *p_dev, MT_UNF_DEC_FRM_TYPE_E dec_frm_type)
{
    return MT_UNF_AVPLAY_DecFrmType(g_avplay_es_handle, dec_frm_type);
}

RET_CODE vdec_set_trick_cfg(void *p_dev, MT_UNF_DEC_TRICK_PARAM_S *pstTrickParam)
{
    return MT_UNF_AVPLAY_SetTrickCfg(g_avplay_es_handle, pstTrickParam);
}

void vdec_set_seek_frame_setup_mode(MT_BOOL is_setup)
{
    is_frame_setup = is_setup;
}

MT_BOOL vdec_get_seek_frame_setup_mode(void)
{
    return is_frame_setup;
}

RET_CODE suplayer_is_stop_send_audio(void) {
    RET_CODE ret = 1;
    if(access("/tmp/fp_stop_send_audio", F_OK) == 0) {
        ret = 0;
    }

    return ret;
}

RET_CODE suplayer_is_stop_send_video(void) {
    RET_CODE ret = 1;
    if(access("/tmp/fp_stop_send_video", F_OK) == 0) {
        ret = 0;
    }

    return ret;
}

RET_CODE suplayer_get_dump_aud_flag(void) {
    RET_CODE ret = 0;
    if(access("/tmp/fp_dump_audio_es", F_OK) == 0) {
        system("rm /tmp/fp_dump_audio_es");
        ret = 1;
    }

    return ret;
}

RET_CODE suplayer_get_dump_vid_flag(void) {
    RET_CODE ret = 0 ;

    if(access("/tmp/fp_dump_video_es", F_OK) == 0) {
        system("rm /tmp/fp_dump_video_es");
        ret = 1;
    }

    return ret;
}

RET_CODE suplayer_get_dump_subt_flag(void) {
    RET_CODE ret = 0;

    if(access("/tmp/fp_dump_subtitle_es", F_OK) == 0) {
        system("rm /tmp/fp_dump_subtitle_es");
        ret = 1;
    }

    return ret;
}

RET_CODE set_hdr_info(void *p_dev, MT_UNF_VIDEO_DISP_HDR_INFO_S *hdrInfo)
{
    mt_s32 ret = MT_SUCCESS;
    //unf_vdec_info_t vstate;
    MLOGD("%s,%d\n", __func__, __LINE__);
    //pthread_mutex_lock(&VMutex);
    //MT_UNF_VIDEO_HDR_INFO_S hdrInfo;
    MT_CODEC_VIDEO_CMD_S param;
    param.u32CmdID = (mt_u32)VFMW_CMD_SET_DISP_HDRINFO;
    param.pPara = (void *)hdrInfo;

    ret = MT_UNF_AVPLAY_Invoke(g_avplay_es_handle, MT_UNF_AVPLAY_INVOKE_VCODEC, (void *)&param);
    if (ret != MT_SUCCESS) {
        MLOGE("Get video info error @line %d,fun %s\n", __LINE__, __func__);
    }
    return (RET_CODE)ret;
}

RET_CODE set_tplay_normal_play_num(void *p_dev, mt_u32 t1xnum)
{
    mt_s32 ret = MT_SUCCESS;
    g_tplay_1xnum = t1xnum;
    return (RET_CODE)ret;
}


RET_CODE get_tplay_normal_play_num(void)
{
	return g_tplay_1xnum;
}

RET_CODE fileplay_set_avsync_mode(MT_UNF_SYNC_REF_E enAVsyncMode)
{
    //printf("%s enAVsyncMode %d\n",__func__, enAVsyncMode);
    return MT_UNF_AVPLAY_SetAVsyncMode(g_avplay_es_handle, enAVsyncMode);
}

#include "drv_adp_avif.c"

#ifdef __cplusplus
}
#endif

