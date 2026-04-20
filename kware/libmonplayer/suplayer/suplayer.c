/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#define MODULE_TAG "SUP"
#include "mutil.h"
#include "mlog.h"
#include "mt_type.h"
#include "drv_adp.h"
#include "mtos_sem.h"
#include "mtos_task.h"
#include "mtsu_type.h"
#include "mt_unf_suplayer.h"
#include "register_net_stream.h"
#include "mtsu_svr_player.h"
#include "file_playback_sequence.h"
#include "suplayer_internal.h"

char *g_playready_cert               = NULL;
static pthread_mutex_t g_suplayMutex = PTHREAD_MUTEX_INITIALIZER;
#define MT_SUPLAY_LOCK()                 (void)pthread_mutex_lock(&g_suplayMutex);
#define MT_SUPLAY_UNLOCK()               (void)pthread_mutex_unlock(&g_suplayMutex);
#define MT_SUPLAY_SEM_CREATE(sem, mutex) mtos_sem_create((os_sem_t *)&sem, mutex)
#define MT_SUPLAY_SEM_TAKE(sem, timeout) mtos_sem_take((os_sem_t *)&sem, timeout)
#define MT_SUPLAY_SEM_GIVE(sem)          mtos_sem_give((os_sem_t *)&sem)
#define MT_SUPLAY_SEM_DESTROY(sem, opt)  mtos_sem_destroy((os_sem_t *)&sem, opt)
/*++++++++++++++++ mplayer start +++++++++++++*/
typedef struct mtUNF_MPLAYER_PRIVATE {
    MT_U32  load_success;
    MT_HANDLE hdl;
    FILE_SEQ_T *p_file_seq;
} mtUNF_MPLAYER_PRIVATE_S;

#define SP_TASK_HEAP_MEM_LEN             (15*1024*1024)
#define SP_PULL_DATA_THREAD_STACK_LEN    (128*1024)
#define SP_PULL_DATA_THREAD__PRIORITY    (13)
#define SP_NET_TASK_PRIORITY             (75)
#define SP_PRELOAD_DATA_THREAD_STACK_LEN (128*1024)
#define SP_MAX_PATH_LEN                  (1024)
#define SP_FILM_URL_MAX_SIZE             (4096)
#define SP_FILM_ARRAY_LEN                (128)
#define SP_TMP_BUF_MAX_LEN               (FILM_URL_MAX_SIZE*FILM_ARRAY_LEN)
#define SP_PRELOAD_AUDIO_BUF_SIZE        (512*1024)
#define SP_PRELOAD_VIDEO_BUF_SIZE        (4 * 1024 *1024)

#define S2H(s,h)                         h = (MT_PLAYBACK_INTERNAL_T *)((mtUNF_MPLAYER_PRIVATE_S *)(((mtUNF_SUPLAYER_STATUS_S *)s)->pri))->hdl
#define S2P(s,p)                         p = (mtUNF_MPLAYER_PRIVATE_S *)(((mtUNF_SUPLAYER_STATUS_S *)s)->pri)
#define H2I(h)                           ((h&SUPLAYER_INSTANCE_MASK)-1)
#define H2T(h,type)                      type=(h&SUPLAYER_PLAYER_TYPE_MASK)>>SUPLAYER_PLAYER_TYPE_SHIT;
#define H2S(h,s)                         s=(mtUNF_SUPLAYER_STATUS_S *)(&suplayer_status[H2I(h)])
#define HtP(h)                           suplayer_status[H2I(h)].pri
static MT_HANDLE p_hPlayer = (MT_HANDLE)NULL;
#define CHECKSTSBYHANDLE(h, type, ret)                                                         \
    if(H2I(h) >= MT_SRV_SUPLAYER_INSTANCE_MAX){                                                \
        MLOGD("%s_%d, ih=%x\n", __func__, __LINE__, h);                                        \
        return (ret);                                                                          \
    }                                                                                          \
    if(suplayer_status[H2I(h)].instance != h){                                                 \
        MLOGD("%s_%d, ih=%x,oh=%x\n",__func__, __LINE__, h, suplayer_status[H2I(h)].instance); \
        return (ret);                                                                          \
    }                                                                                          \
    if(suplayer_status[H2I(h)].status == type){                                                \
        MLOGD("%s_%d\n", __func__, __LINE__);                                                  \
        return (ret);                                                                          \
    }

#define MT_SVR_PLAYER_CHECK_STS(a, b, c, d)                                                    \
    if(MT_SVR_PLAYER_CHECK_STATE((a), (b), (c)) == (MT_S32)MT_FAILURE) {                       \
        if((d) && (b)) {                                                                       \
            MT_SUPLAY_SEM_GIVE(b->suplock);                                                    \
        }                                                                                      \
        if((c)) {                                                                              \
            return ((MT_S32)MT_FAILURE);                                                       \
        } else {                                                                               \
            return ((MT_S32)MT_SUCCESS);                                                       \
        }                                                                                      \
    }

static  PB_SEQ_PARAM_T pb_seq_param       = {0};
static void *p_mem_mplayer_globle         = NULL;
static unsigned char subtitle_flag        = 1;
static unsigned long load_success         = -1;
static int fp_state                       = 0;
static void *pmplayer                     = NULL;
static unsigned long g_last_callback_vpts = 0;
MT_U32 g_cur_duration                     = 0;
mt_handle g_drv_sup_hdl                   = 0;
static mtUNF_SUPLAYER_STATUS_S suplayer_status[MT_SRV_SUPLAYER_INSTANCE_MAX] = {{0}};
typedef MT_U32(*CALLBACK_FUNC)(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_S *pstruEvent);

static void mplayer_event_async(
    void *pmplayer, MT_SVR_PLAYER_STATE_E event)
{
    CALLBACK_FUNC pfunc = NULL;
    MT_PLAYBACK_INTERNAL_T *pplayer;
    if (!pmplayer) {
        return;
    }
    S2H(pmplayer, pplayer);
    if (!pplayer || !pplayer->cb) {
        return;
    }

    MT_SVR_PLAYER_EVENT_S event_s;
    MT_SVR_PLAYER_ERROR_E data = {0};
    data            = event;
    event_s.eEvent  = MT_SVR_PLAYER_EVENT_ASYNC_SETMEDIA_FINISH;
    event_s.pu8Data = (MT_U8 *)(&data);
    event_s.u32Len  = sizeof(u32);
    pfunc           = pplayer->cb;
    (*pfunc)(p_hPlayer, &event_s);
}

static MT_U32 mplayer_event_callback(FILE_PLAY_EVENT_E event, unsigned long param)
{
    unsigned long vpts     = 0;
    unsigned long cur_hour = 0;
    unsigned long cur_min  = 0;
    unsigned long cur_sec  = 0;
    CALLBACK_FUNC pfunc    = NULL;
    MT_PLAYBACK_INTERNAL_T *pplayer;
    MT_SVR_PLAYER_EVENT_S event_s;
    MT_SVR_PLAYER_ERROR_E data;

    MLOGD("func [%s], line [%d], event [%d]\n", __FUNCTION__, __LINE__, event);
    switch (event) {
        case FILE_PLAYBACK_SEQ_GET_VPTS:
            vpts  =  param;   //millisecond
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    MT_SVR_PLAYER_PROGRESS_S data;
                    data.s64BufferSize = 0;
                    data.s64Duration = vpts;
                    data.u32Progress = 0;
                    event_s.eEvent = MT_SVR_PLAYER_EVENT_PROGRESS;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len = sizeof(MT_SVR_PLAYER_PROGRESS_S);
                    pfunc = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }

            cur_hour = (vpts / 1000) / 3600;
            cur_min  = ((vpts / 1000) - (cur_hour * 3600)) / 60;
            cur_sec  = (vpts / 1000) - (cur_hour * 3600) - cur_min * 60;
            MLOGD("[%ld:%ld:%ld],  %ld ms\n", cur_hour, cur_min, cur_sec, (vpts));
            g_last_callback_vpts = vpts;
            break;
        case FILE_PLAYBACK_SEQ_RECEIVE_VIDEO_INFO:
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    VIDEO_W_H_FPS video_whfps;
                    mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
                    S2P(pmplayer, mplayer_pri);
                    FILM_INFO_T film;
                    mplayer_pri->p_file_seq->get_film_info((void *)mplayer_pri->p_file_seq, &film);
                    video_whfps.video_disp_w = film.video_disp_w;
                    video_whfps.video_disp_h = film.video_disp_h;
                    event_s.eEvent           = MT_SVR_PLAYER_EVENT_FILE_INFO;
                    event_s.pu8Data          = (MT_U8 *)(&video_whfps);
                    event_s.u32Len           = sizeof(VIDEO_W_H_FPS);
                    pfunc = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }

            break;
        case FILE_PLAYBACK_NEW_SUB_DATA_RECEIVE:
            MLOGD("receive new subtitle packet message ! \n");
            if (subtitle_flag == 1) {
                if (pmplayer) {
                    S2H(pmplayer, pplayer);
                    if (pplayer) {
                        MT_SVR_PLAYER_SUBT_DATA_S subt_data;
                        MT_U32 size;
                        MT_SVR_PLAYER_Get_Subt_Data((MT_HANDLE)pplayer, &subt_data.subt_array, &subt_data.pts, &size);
                        event_s.eEvent  = MT_SVR_PLAYER_EVENT_NEW_SUBTITLE_RECEIVED;
                        event_s.pu8Data = (MT_U8 *)(&subt_data);
                        event_s.u32Len  = sizeof(MT_SVR_PLAYER_STATE_E);
                        pfunc           = pplayer->cb;
                        (*pfunc)(p_hPlayer, &event_s);
                    }
                }
            }
            break;

        case FILE_PLAYBACK_SEQ_STOP:
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PLAYER_STATE_STOP;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_STATE_CHANGED;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(MT_SVR_PLAYER_STATE_E);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
                ((mtUNF_SUPLAYER_STATUS_S *)pmplayer)->status = MT_SVR_PLAYER_STATE_STOP;
            }
            fp_state = 2;
            break;

        case FILE_PLAYBACK_SEQ_FILL_ES_TASK_EXIT:
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PLAYER_STATE_ES_TASK_EXIT;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_STATE_CHANGED;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(MT_SVR_PLAYER_STATE_E);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
                ((mtUNF_SUPLAYER_STATUS_S *)pmplayer)->status = MT_SVR_PLAYER_STATE_STOP;
            }
            fp_state = 2;
            break;

        case FILE_PLAYBACK_SEQ_LOAD_MEDIA_ERROR:

            load_success = FILE_PLAYBACK_SEQ_LOAD_MEDIA_ERROR;
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PLAYER_ERROR_UNKNOW;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_ERROR;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(MT_SVR_PLAYER_ERROR_E);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
                ((mtUNF_SUPLAYER_STATUS_S *)pmplayer)->status = MT_SVR_PLAYER_STATE_STOP;
            }
            fp_state = 2;
            MLOGE("%s %d LOAD media ERROR\n", __func__, __LINE__);
            break;
        case FILE_PLAYBACK_SEQ_LOAD_MEDIA_EXIT:
            load_success = FILE_PLAYBACK_SEQ_LOAD_MEDIA_EXIT;
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PLAYER_ERROR_UNKNOW;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_ERROR;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(MT_SVR_PLAYER_ERROR_E);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
                ((mtUNF_SUPLAYER_STATUS_S *)pmplayer)->status = MT_SVR_PLAYER_STATE_STOP;
            }
            fp_state = 2;
            MLOGI("%s LOAD media exit\n", __func__);
            break;
        case FILE_PLAYBACK_SEQ_LOAD_MEDIA_SUCCESS:
            load_success = FILE_PLAYBACK_SEQ_LOAD_MEDIA_SUCCESS;
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PLAYER_STATE_LOADED;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_STATE_CHANGED;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(MT_SVR_PLAYER_STATE_E);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            MLOGI("%s LOAD media SUCCESS\n", __func__);
            break;
        case FILE_PLAYBACK_CHECK_TRICKPLAY:
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data = MT_SVR_PLAYER_STATE_PLAY;//default is normal play
                    if (param == 0) {
                        ((mtUNF_SUPLAYER_STATUS_S *)pmplayer)->status = MT_SVR_PLAYER_STATE_PLAY;
                        data = MT_SVR_PLAYER_STATE_PLAY;//param == 0, backward to normal play
                    }
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_SOF;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(MT_SVR_PLAYER_STATE_E);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        case FILE_PLAYBACK_SWITCH_AUDIO_SPDIF_MODE:
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    u32 audioType   = *(u32 *)param;
                    data            = MT_SVR_PLAYER_STATE_BACKWARD;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_SWITCH_AUDIO_SPDIF_MODE;
                    event_s.pu8Data = (MT_U8 *)(&audioType);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                    *(u32 *)param = audioType;
                }
            }
            break;
        case FILE_PLAYBACK_UNSUPPORT_VIDEO:
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PALYER_ERROR_UNSUPPORT_VIDEO;
                    event_s.eEvent  = MT_SVR_PLAYBACK_UNSUPPORT_VIDEO;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        case FILE_PLAYBACK_SEQ_ASYNC_OPEN: {
            MLOGI("Async start\n");
            mplayer_event_async(pmplayer, MT_SVR_PLAYER_STATE_ASYNC_OPEN);
            break;
        }
        case FILE_PLAYBACK_SEQ_ASYNC_DONE: {
            MLOGI("Async done\n");
            mplayer_event_async(pmplayer, MT_SVR_PLAYER_STATE_ASYNC_DONE);
            break;
        }
        case FILE_PLAYBACK_SEEK_FINISHED:
            MLOGD("FILE_PLAYBACK_SEEK_FINISHED\n", __func__, __LINE__);
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PLAYER_ERROR_NON;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_SEEK_FINISHED;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        case FILE_PLAYBACK_UNSUPPORT_SEEK:
            MLOGD("FILE_PLAYBACK_UNSUPPORT_SEEK\n", __func__, __LINE__);
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PLAYER_ERROR_NOT_SUPPORT;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_SEEK_FINISHED;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;

        case FILE_PLAYBACK_TRICKMODE_ENTER:
            MLOGD("MT_SVR_PLAYBACK_TRICKMODE_ENTER\n", __func__, __LINE__);
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer && (!pplayer->is_trickmode)) {
                    pplayer->is_trickmode = !pplayer->is_trickmode;
                    data            = MT_SVR_PLAYER_ERROR_NON;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_TRICKMODE_ENTER;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        case FILE_PLAYBACK_TRICKMODE_LEAVE:
            MLOGD("MT_SVR_PLAYBACK_TRICKMODE_ENTER\n", __func__, __LINE__);
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer && pplayer->is_trickmode) {
                    pplayer->is_trickmode = !pplayer->is_trickmode;
                    data            = MT_SVR_PLAYER_ERROR_NON;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_TRICKMODE_LEAVE;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        case FILE_PLAYBACK_PLAY_EOF:
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PLAYER_ERROR_NON;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_EOF;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        case FILE_PLAYBACK_PLAY_NEW_VID_FRAME:
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PLAYER_ERROR_NON;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_NEW_VID_FRAME;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        case FILE_PLAYBACK_NOT_ENOUGH_MEMORY:
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PALYER_ERROR_NOT_ENOUGH_MEMORY;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_ERROR;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
                ((mtUNF_SUPLAYER_STATUS_S *)pmplayer)->status = MT_SVR_PLAYER_STATE_STOP;
            }
            break;
        case FILE_PLAYBACK_SEQ_UPDATE_BPS:{
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    MT_SVR_PLAYER_NETWORKLOAD_S data;
                    mt_u32  bps_percent_val = (mt_u32)param;
                    data.u32Bps = bps_percent_val&0x0000ffff;
                    data.u32Percentage = (bps_percent_val&0xffff0000) >> 16;
                    event_s.eEvent = MT_SVR_PLAYER_EVENT_NETWORK_LOAD;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len = sizeof(MT_SVR_PLAYER_NETWORKLOAD_S);
                    pfunc = pplayer->cb;

                    MLOGD(">>>11 VES [%d KB/sec]  [%d]!!!<<<\n", data.u32Bps, data.u32Percentage);
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        }
        case FILE_PLAYBACK_SEQ_START_BUFFERING:{
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PALYER_NETWORK_START;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_NETWORK_BUFFERING;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;

                    MLOGD("NETWORK BUFFER START!!!!!!\n");
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        }
        case FILE_PLAYBACK_SEQ_FINISH_BUFFERING:{
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PALYER_NETWORK_END;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_NETWORK_BUFFERING;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;

                    MLOGD("NETWORK BUFFER END!!!!!!\n");
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        }
        case FILE_PLAYBACK_LIVE_SERVER_ERROR:{
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    int error_code = (int) param;
                    event_s.eEvent =  MT_SVR_PLAYER_EVENT_SERVER_ERROR;
                    event_s.pu8Data = (MT_U8 *)(&error_code);
                    event_s.u32Len = sizeof(int);
                    pfunc = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        }
        case FILE_PLAYBACK_DRM_DECRYPT_FAIL:{
            if (pmplayer) {
                S2H(pmplayer, pplayer);
                if (pplayer) {
                    data            = MT_SVR_PALYE_DRV_DECRYPT_FAIL;
                    event_s.eEvent  = MT_SVR_PLAYER_EVENT_DRM;
                    event_s.pu8Data = (MT_U8 *)(&data);
                    event_s.u32Len  = sizeof(u32);
                    pfunc           = pplayer->cb;
                    (*pfunc)(p_hPlayer, &event_s);
                }
            }
            break;
        }
        default :
            break;
    }

    return  0;
}
/*--------------------- mplayer end -------------------*/
static MT_S32 MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_E sts, mtUNF_SUPLAYER_STATUS_S *phdl, MT_S32 opt)
{
    MT_S32 ret = (MT_S32)MT_FAILURE;
    if (phdl == NULL) {
        goto EXIT;
    }
    MLOGD("[%s_%d] sts=%d,%d\n", __func__, __LINE__, sts, phdl->status);
    if (opt) { //check if match, return fail if not match
        if (phdl->status == sts) {
            ret = (MT_S32)MT_SUCCESS;
        } else {
            ret = (MT_S32)MT_FAILURE;
            MLOGD("[%s_%d] sts=%d,%d\n", __func__, __LINE__, sts, phdl->status);
        }
    } else { //check if unmatch, return fail if match
        if (phdl->status == sts) {
            ret = (MT_S32)MT_FAILURE;
            MLOGD("[%s_%d] sts=%d,%d\n", __func__, __LINE__, sts, phdl->status);
        } else {
            ret = (MT_S32)MT_SUCCESS;
        }
    }
EXIT:
    if (ret == MT_FAILURE) {
        MLOGD("[%s_%d]sts=%d\n", __func__, __LINE__, sts);
    }
    return ret;
}

MT_VOID *MT_SVR_PLAYER_Init(mt_void *args)
{
    mtUNF_SUPLAYER_STATUS_S *ret     = NULL;
    mtUNF_SUPLAYER_IN_ARG_S *in_args = (mtUNF_SUPLAYER_IN_ARG_S *)args;
    MT_U32 i, j;

    if (NULL == args || MT_SUPLAYER_MPLAYER != in_args->ptype) {
        MLOGE("%s para error\n", __FUNCTION__);
        return NULL;
    }

    MLOGI("%s type:%d\n", __FUNCTION__, in_args->ptype);
    MT_SUPLAY_LOCK();

    for (i = 0; i < MT_SRV_SUPLAYER_INSTANCE_MAX; i++) {
        if (suplayer_status[i].ptype == MT_SUPLAYER_MPLAYER) {
            MLOGE("%s fail, ptype existed!\n", __FUNCTION__);
            goto INIT_FAIL2;
        } else if (suplayer_status[i].ptype == MT_SUPLAYER_UNKNOWN) {
            j = i;
            if (i == MT_SRV_SUPLAYER_INSTANCE_MAX - 1) {
                break;
            }
        }
    }
    if (i == MT_SRV_SUPLAYER_INSTANCE_MAX) {
        MLOGE("%s max player instace!\n", __FUNCTION__);
        goto INIT_FAIL2;
    }

    ret           = &suplayer_status[j];
    pmplayer      =  ret;
    ret->ptype    = MT_SUPLAYER_MPLAYER;
    ret->instance = (j + 1) | (ret->ptype << 16);
    ret->status   = MT_SVR_PLAYER_STATE_INIT;
    ret->pri      = NULL;
    MT_SUPLAY_SEM_CREATE(ret->suplock, TRUE);
    fp_state      = 0;
    MLOGD("suplayer type=%d,%d,%d\n", suplayer_status[j].ptype, ret->ptype, j);
    register_http_stream();
    register_rtsp_stream();

    MT_SUPLAY_UNLOCK();
    return (MT_VOID *)ret;
INIT_FAIL2:
    MT_SUPLAY_UNLOCK();
    return NULL;
}

MT_S32 MT_SVR_PLAYER_Create(const MT_SVR_PLAYER_PARAM_S *pstruParam, MT_HANDLE *phPlayer)
{
    mtUNF_SUPLAYER_STATUS_S *iargs = NULL;
    MT_S32  ret = SUCCESS;
    MT_PLAYBACK_INTERNAL_T  *hdl;
    MT_U32 mem_size;

    if (pstruParam == NULL) {
        ret = ERR_FAILURE;
        MLOGE("%s para NULL\n", __FUNCTION__);
        goto FAIL;
    }

    iargs = (mtUNF_SUPLAYER_STATUS_S *)pstruParam->suplayer_status;
    if (iargs == NULL) {
        ret = ERR_FAILURE;
        MLOGE("%s suplayer_status NULL\n", __FUNCTION__);
        goto FAIL;
    }

    MLOGI("%s type:%d\n", __FUNCTION__, iargs->ptype);
    MT_SUPLAY_SEM_TAKE(iargs->suplock, 0);
    MT_SVR_PLAYER_CHECK_STS(MT_SVR_PLAYER_STATE_INIT, iargs, 1, 1);
    g_last_callback_vpts = 0;
    g_cur_duration       = 0;
    if (iargs->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        if ((MT_HANDLE *)(*phPlayer) == NULL) {
            hdl = mtsu_malloc(sizeof(MT_PLAYBACK_INTERNAL_T));
            if (hdl == NULL) {
                MLOGE("%s handle alloc error\n", __FUNCTION__);
                goto FAIL0;
            }
            memset(hdl, 0, sizeof(MT_PLAYBACK_INTERNAL_T));
            mplayer_pri = (mtUNF_MPLAYER_PRIVATE_S *)mtsu_malloc(sizeof(mtUNF_MPLAYER_PRIVATE_S));
            if (mplayer_pri == NULL) {
                MLOGE("%s mplayer_pri NULL\n", __FUNCTION__);
                goto FAIL1;
            }
            MLOGD("%s %d 0x%p %p ins=0x%x\n", __FUNCTION__, __LINE__, mplayer_pri, hdl, iargs->instance);
            mplayer_pri->hdl = (MT_HANDLE)hdl;
            mplayer_pri->load_success = -1;
            mplayer_pri->p_file_seq = NULL;
            HtP(iargs->instance) = (MT_VOID *)(mplayer_pri);
            H2S(iargs->instance, hdl->suplayer_status);
            *phPlayer = iargs->instance;
            MLOGD("%s %d 0x%p %p ins=0x%x\n", __FUNCTION__, __LINE__, mplayer_pri, hdl, iargs->instance);
        } else {
            MLOGE("%s player handle not NULL \n", __FUNCTION__);
            goto FAIL0;
        }

        if (pstruParam->heapMemSize == 0) {
            mem_size = SP_TASK_HEAP_MEM_LEN;
        } else {
            mem_size = pstruParam->heapMemSize;
        }

        if (pstruParam->pb_heap_mem_start == NULL) {
#if defined(FP_USE_SYSTEM_MEM)
            pb_seq_param.pb_seq_mem_size = 0;
            pb_seq_param.pb_seq_mem_start = 0;
#else
            if (p_mem_mplayer_globle) {
                mtsu_free(p_mem_mplayer_globle);
            }

            p_mem_mplayer_globle = mtsu_malloc(mem_size);
            MLOGD("%s %d malloc mem_size %d\n", __FUNCTION__, __LINE__, mem_size);
            if (p_mem_mplayer_globle == NULL) {
                MLOGE("Alloc mplayer mem fail!!!\n");
                goto FAIL2;
            }
            memset(p_mem_mplayer_globle, 0, mem_size);
            pb_seq_param.pb_seq_mem_start = (unsigned long)p_mem_mplayer_globle;
#endif
        } else {
            if (p_mem_mplayer_globle) {
                mtsu_free(p_mem_mplayer_globle);
                p_mem_mplayer_globle = NULL;
            }
            pb_seq_param.pb_seq_mem_start = (unsigned long)pstruParam->pb_heap_mem_start;
        }
        pb_seq_param.pb_seq_mem_size = mem_size;
        pb_seq_param.audio_output_mode = (AUDIO_OUT_MODE)pstruParam->audio_output;
        pb_seq_param.task_smart_http_priority[0] = 0;//open smart http, add by libin

        pb_seq_param.transport_desdec_ctx  = pstruParam->transport_desdec_ctx;
        pb_seq_param.transport_desdec_func = pstruParam->transport_desdec_func;
        mplayer_pri->p_file_seq = file_seq_create(&pb_seq_param);
        ret = (MT_S32)set_tplay_normal_play_num(NULL, MT_SVR_PLAYER_PLAY_SPEED_NORMAL);
        if (mplayer_pri->p_file_seq == NULL) {
            MLOGE("file sequence creat error\n");
            goto FAIL2;
        }

        mplayer_pri->p_file_seq->register_event_cb(mplayer_pri->p_file_seq, mplayer_event_callback);
        MLOGD("[%s] file sequence creat ok! \n", __func__);
    }
    {
        int ptr;
        int ret_tmp;
        MLOGD("%s %d create suplayer proc !!!\n", __FUNCTION__, __LINE__);
        ret_tmp = MT_UNF_SUPLAYER_Init();
        if (ret_tmp != MT_SUCCESS) {
            MLOGE("%s %d create suplayer proc fail!!!\n", __FUNCTION__, __LINE__);
        }
        ret_tmp = MT_UNF_SUPLAYER_Create(&ptr, &g_drv_sup_hdl);
        file_seq_suplayer_handle_set(g_drv_sup_hdl);
        if (ret_tmp != MT_SUCCESS) {
            MLOGE("%s %d create suplayer proc fail!!!\n", __FUNCTION__, __LINE__);
        }
    }
    iargs->status = MT_SVR_PLAYER_STATE_CREATE;
    MT_SUPLAY_SEM_GIVE(iargs->suplock);
    return ret;

FAIL2:
    if (HtP(iargs->instance)) {
        mtsu_free(HtP(iargs->instance));
        HtP(iargs->instance) = NULL;
    }
FAIL1:
    if ((MT_HANDLE *)(*phPlayer) == NULL) {
        if (hdl) {
            mtsu_free(hdl);
        }
    }
FAIL0:
    ret = ERR_FAILURE;
    MT_SUPLAY_SEM_GIVE(iargs->suplock);
FAIL:
    return ret;
}

MT_S32 MT_SVR_PLAYER_Get_Media_Info_Nolock(
							MT_HANDLE hPlayer, void *pResult)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    MT_S32 ret = (MT_S32)MT_SUCCESS;
    CHECKSTSBYHANDLE(hPlayer, MT_SVR_PLAYER_STATE_STOP, MT_FAILURE);
    H2S(hPlayer, phdl);

    if (MT_SUPLAYER_MPLAYER != phdl->ptype) {
        MLOGE("%s, type error:%d\n", __func__, phdl->ptype);
        return ret;
    }

    mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
    FILM_INFO_T  film_info;
    file_seq_video_subtitle_t *subt = NULL;
    if (pResult == NULL) {
        MLOGE("%s_%d:input args\n", __func__, __LINE__);
        ret = (MT_S32)MT_FAILURE;
        goto EXIT;
    }

    S2P(phdl, mplayer_pri);
    mplayer_pri->p_file_seq->get_subt_info((void *)mplayer_pri->p_file_seq, (void **)&subt);
    ((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->subt = (MT_SVR_PLAYER_SUBT_S *)subt;
    mplayer_pri->p_file_seq->get_audio_track_lang((void *)mplayer_pri->p_file_seq, NULL);
    ((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->aud_lang = (MT_SVR_PLAYER_TRACK_LANG_S *)mplayer_pri->p_file_seq->audio_lang_array;
    mplayer_pri->p_file_seq->get_film_info((void *)mplayer_pri->p_file_seq, &film_info);
    /*Make sure FILE_INFO_S is started with FILM_INFO_T*/
    if (sizeof(MT_SVR_PLAYER_FILM_S) == sizeof(FILM_INFO_T)) {
        ((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->film = *((MT_SVR_PLAYER_FILM_S *)&film_info);
    } else {
        MLOGE("%s_%d:structure different!!!\n", __func__, __LINE__);
    }
    ((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->aud_lang_cnt = mplayer_pri->p_file_seq->audio_track_num;
EXIT:
    return ret;
}

MT_S32 MT_SVR_PLAYER_LOADMEDIA_GetFileInfo(MT_HANDLE hPlayer, MT_FORMAT_FILE_INFO_S **ppstruInfo)
{
	int ret = -1;
	mtUNF_SUPLAYER_STATUS_S *phdl;
	mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
	MT_SVR_PLAYER_FILE_INFO_S file_info;

    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    MT_SVR_PLAYER_CHECK_STS(MT_SVR_PLAYER_STATE_SETPATH, phdl, 1, 1);

    if (MT_SUPLAYER_MPLAYER != phdl->ptype) {
        MLOGE("%s, type error:%d\n", __func__, phdl->ptype);
        return ret;
    }

    S2P(phdl, mplayer_pri);

    MLOGD("%s %d \n", __FUNCTION__, __LINE__);
    load_success = -1;
    fp_state = 1;
    g_cur_duration = 0;
    g_last_callback_vpts = 0;
    mplayer_pri->p_file_seq->loadmedia_task((void *)mplayer_pri->p_file_seq);
    do {
        mtos_task_sleep(100);
        MLOGD("wait load media task exit !!!\n");
        if (load_success == FILE_PLAYBACK_SEQ_LOAD_MEDIA_SUCCESS
            || load_success == FILE_PLAYBACK_SEQ_LOAD_MEDIA_ERROR
            || load_success == FILE_PLAYBACK_SEQ_LOAD_MEDIA_EXIT) {
            break ;
        }
    } while (load_success == -1);
    MT_SVR_PLAYER_Get_Media_Info_Nolock(hPlayer, &file_info);
    ret = (load_success == FILE_PLAYBACK_SEQ_LOAD_MEDIA_SUCCESS) ? MT_SUCCESS : MT_FAILURE;

    if (phdl->status != MT_SVR_PLAYER_STATE_SETPATH) {
        MLOGD("%s %d zx phdl->status != MT_SVR_PLAYER_STATE_SETPATH\n", __FUNCTION__, __LINE__);
        ret = MT_FAILURE;
    } else if (ret == (MT_S32)MT_SUCCESS) {
        phdl->status = MT_SVR_PLAYER_STATE_LOADED;
    }
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    MLOGD("%s %d %d %d\n", __FUNCTION__, __LINE__, ret, load_success);
    return ret;
}

MT_S32 MT_SVR_PLAYER_SetMedia(MT_HANDLE hPlayer, MT_U32 eType, MT_SVR_PLAYER_MEDIA_S *pstruMedia)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    MT_PLAYBACK_INTERNAL_T *p_MonPlayer;
    MT_S32 ret = (MT_S32)MT_SUCCESS;

	if (!pstruMedia) {
		return MT_FAILURE;
	}

    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_CREATE, phdl, 1) == MT_FAILURE
        && MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_STOP, phdl, 1) == MT_FAILURE) {
        ret = (MT_S32)MT_FAILURE;
        MLOGE("%s player state error\n", __FUNCTION__);
        goto EXIT;
    }

    S2H(phdl, p_MonPlayer);
    memset(p_MonPlayer->url, 0, URL_LEN);
    memcpy(p_MonPlayer->url, pstruMedia->aszUrl, strlen(pstruMedia->aszUrl));

    MLOGI("[%p] %s %s\n", p_MonPlayer, __FUNCTION__, p_MonPlayer->url);
    if (strlen(pstruMedia->aszCertPath) > 0) {
        if (g_playready_cert == NULL) {
            g_playready_cert = (char *)mtsu_malloc(MT_FORMAT_MAX_URL_LEN);
        }
        memset(g_playready_cert, 0, MT_FORMAT_MAX_URL_LEN);
        memcpy(g_playready_cert, pstruMedia->aszCertPath, strlen(pstruMedia->aszCertPath));
    }
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        FILE_SEQ_T *p_file_seq = mplayer_pri->p_file_seq;
        if (MT_SVR_PLAYER_MEDIA_SOURCE_EXTIO == eType) {
            MT_SVR_PLAYER_EXTIO_CONTEXT_S *extio_cb = pstruMedia->extio_cb;
            ret = sup_seq_set_extio_data_source((void *) p_file_seq, extio_cb);
        } else {
            p_file_seq->set_file_path(p_file_seq, p_MonPlayer->url);
            (void) sup_set_input_para((void *) p_file_seq, &(pstruMedia->para));
        }
        if (pstruMedia->para.max_stream_probe_size       > 0 &&
            pstruMedia->para.max_stream_analyze_duration > 0) {
            (void) sup_seq_cfg_stream_probe_para((void *) p_file_seq,
                pstruMedia->para.max_stream_probe_size,  pstruMedia->para.max_stream_analyze_duration);
        }
    }
    phdl->status = MT_SVR_PLAYER_STATE_SETPATH;
EXIT:
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_PLAYER_Play(MT_HANDLE hPlayer, MT_S64 start_msec)
{
    MT_U32 ret = SUCCESS;
    mtUNF_SUPLAYER_STATUS_S *phdl;

    H2S(hPlayer, phdl);
    MLOGI("%s\n", __FUNCTION__);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    MT_SVR_PLAYER_CHECK_STS(MT_SVR_PLAYER_STATE_LOADED, phdl, 1, 1);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        MT_U32 t_sec = (MT_U32)start_msec / 1000;
        S2P(phdl, mplayer_pri);
        ret = mplayer_pri->p_file_seq->start((void *)mplayer_pri->p_file_seq, t_sec);
        if (ret != 0) {
            MLOGE("[%s] fail to start !!!!!!!!!\n", __func__);
            ret = ERR_FAILURE;
        } else {
            MLOGD("[%s] status[%d]..!!\n", __func__, mplayer_pri->p_file_seq->get_status((void *)mplayer_pri->p_file_seq));
        }
    }

    phdl->status = MT_SVR_PLAYER_STATE_PLAY;
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_PLAYER_Stop(MT_HANDLE hPlayer)
{
    MT_S32 ret = (MT_S32)MT_SUCCESS;
    mtUNF_SUPLAYER_STATUS_S *phdl;

    MLOGI("%p %s\n", (void *)hPlayer, __FUNCTION__);
    H2S(hPlayer, phdl);
    phdl->status = MT_SVR_PLAYER_STATE_PRESTOP;
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        ret = mplayer_pri->p_file_seq->stop((void *)mplayer_pri->p_file_seq);
    } else if (phdl->ptype == MT_SUPLAYER_GSTREAMER) {
    } else {
    }

    return ret;
}

MT_S32 MT_SVR_PLAYER_Destroy(MT_HANDLE hPlayer)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    MT_PLAYBACK_INTERNAL_T *p_MonPlayer;
    MT_U32  tout_cnt     = 0;
    MT_U32  tout_cnt_max = 1000;
    MT_S32  ret          = (MT_S32)MT_SUCCESS;

    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    MT_SVR_PLAYER_CHECK_STS(MT_SVR_PLAYER_STATE_IDLE, phdl, 0, 1);
    phdl->status = MT_SVR_PLAYER_STATE_PREDESTROY;
    S2H(phdl, p_MonPlayer);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        MLOGI("%s, fp_state=%d\n", __func__, fp_state);
        if (fp_state == 1) {
            mplayer_pri->p_file_seq->stop((void *)mplayer_pri->p_file_seq);
            while (fp_state == 1 && tout_cnt < tout_cnt_max) {
                mtos_task_sleep(10);
                tout_cnt++;
            }
            if (fp_state == 1) {
                MT_SUPLAY_SEM_GIVE(phdl->suplock);
                MLOGE("%s_%d:Destroy mplayer fail, fp_state=%d\n", __func__, __LINE__, fp_state);
                ret = (MT_S32)MT_FAILURE;
                goto EXIT;
            }
        }
        fp_state = 0;
        file_seq_destroy();
        if (p_mem_mplayer_globle) {
            mtsu_free(p_mem_mplayer_globle);
            p_mem_mplayer_globle = NULL;
        }
        if (g_playready_cert) {
            mtsu_free(g_playready_cert);
            g_playready_cert = NULL;
        }
        mtsu_free(mplayer_pri);
        phdl->ptype = MT_SUPLAYER_UNKNOWN;
        mtsu_free(p_MonPlayer);
    }
    {
        int ret_tmp;
        MLOGD("%s %d destroy suplayer proc\n", __FUNCTION__, __LINE__);
        ret_tmp = MT_UNF_SUPLAYER_Destroy(g_drv_sup_hdl);
        if (ret_tmp != (MT_S32)MT_SUCCESS) {
            MLOGE("%s %d destroy suplayer proc fail!!!\n", __FUNCTION__, __LINE__);
        }
        ret_tmp = MT_UNF_SUPLAYER_DeInit();
        if (ret_tmp != (MT_S32)MT_SUCCESS) {
            MLOGE("%s %d destroy suplayer proc fail!!!\n", __FUNCTION__, __LINE__);
        }
        file_seq_suplayer_handle_set((MT_HANDLE)NULL);
        MLOGD("%s %d destroy suplayer proc end\n", __FUNCTION__, __LINE__);
    }
    phdl->status = MT_SVR_PLAYER_STATE_IDLE;
    unregister_http_stream();
    unregister_rtsp_stream();

EXIT:
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    MT_SUPLAY_SEM_DESTROY(phdl->suplock, 0);
    return ret;
}
MT_S32 MT_SVR_PLAYER_RegCallback(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_FN pfnCallback)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    MT_PLAYBACK_INTERNAL_T *p_MonPlayer;

    CHECKSTSBYHANDLE(hPlayer, MT_SVR_PLAYER_STATE_STOP, MT_FAILURE);
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    S2H(phdl, p_MonPlayer);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        p_MonPlayer->cb = pfnCallback;
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_Pause(MT_HANDLE hPlayer)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;

    CHECKSTSBYHANDLE(hPlayer, MT_SVR_PLAYER_STATE_STOP, MT_FAILURE);
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        mplayer_pri->p_file_seq->pause((void *)mplayer_pri->p_file_seq);
    }

    phdl->status = MT_SVR_PLAYER_STATE_PAUSE;
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_Resume(MT_HANDLE hPlayer)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;

    CHECKSTSBYHANDLE(hPlayer, MT_SVR_PLAYER_STATE_STOP, MT_FAILURE);
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        mplayer_pri->p_file_seq->resume((void *)mplayer_pri->p_file_seq);
    }

    phdl->status = MT_SVR_PLAYER_STATE_PLAY;
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_TPlay(MT_HANDLE hPlayer, MT_S32 s32Speed)
{
    MT_S32 ret = (MT_S32)MT_SUCCESS;
    mtUNF_SUPLAYER_STATUS_S *phdl;

    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_PLAY, phdl, 1) == MT_FAILURE
        && MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_PAUSE, phdl, 1) == MT_FAILURE
        && MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_FORWARD, phdl, 1) == MT_FAILURE
        && MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_BACKWARD, phdl, 1) == MT_FAILURE) {
        ret = (MT_S32)MT_FAILURE;
        goto EXIT;
    }

    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        mplayer_pri->p_file_seq->set_speed((void *)mplayer_pri->p_file_seq, s32Speed);
    }

    if (s32Speed > 1) {
        phdl->status = MT_SVR_PLAYER_STATE_FORWARD;
    } else if (s32Speed < 0) {
        phdl->status = MT_SVR_PLAYER_STATE_BACKWARD;
    } else {
        phdl->status = MT_SVR_PLAYER_STATE_PLAY;
    }

EXIT:
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_PLAYER_Wait_TPlay_Finish(MT_HANDLE hPlayer, mt_u32 timeout_ms)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;

    CHECKSTSBYHANDLE(hPlayer, MT_SVR_PLAYER_STATE_STOP, MT_FAILURE);
    H2S(hPlayer, phdl);

    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        mplayer_pri->p_file_seq->wait_tplay_finish((void *)mplayer_pri->p_file_seq, timeout_ms);
        MT_SUPLAY_SEM_GIVE(phdl->suplock);
    }

    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_Seek(MT_HANDLE hPlayer, mt_s64 s64TimeInMs)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    MT_S32 ret = (MT_S32)MT_SUCCESS;

    CHECKSTSBYHANDLE(hPlayer, MT_SVR_PLAYER_STATE_STOP, MT_FAILURE);
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_PLAY, phdl, 1) == MT_FAILURE
        && MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_PAUSE, phdl, 1) == MT_FAILURE
        && MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_LOADED, phdl, 1) == MT_FAILURE
        && MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_FORWARD, phdl, 1) == MT_FAILURE
        && MT_SVR_PLAYER_CHECK_STATE(MT_SVR_PLAYER_STATE_BACKWARD, phdl, 1) == MT_FAILURE) {
        printf("func [%s], line [%d]\n", __FUNCTION__, __LINE__);
        ret = (MT_S32)MT_FAILURE;
        goto EXIT;
    }
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        int t_sec = (MT_U32)s64TimeInMs / 1000;
        S2P(phdl, mplayer_pri);
        mplayer_pri->p_file_seq->play_at_time((void *)mplayer_pri->p_file_seq, t_sec);
    }

EXIT:
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_PLAYER_Wait_Seek_Finish(MT_HANDLE hPlayer, mt_u32 timeout_ms)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;

    CHECKSTSBYHANDLE(hPlayer, MT_SVR_PLAYER_STATE_STOP, MT_FAILURE);
    H2S(hPlayer, phdl);

    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        mplayer_pri->p_file_seq->wait_seek_finish((void *)mplayer_pri->p_file_seq, timeout_ms);
        MT_SUPLAY_SEM_GIVE(phdl->suplock);
    }

    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_SeekPos(MT_HANDLE hPlayer, mt_s64 s64Offset)
{
#if 0 //not support now
    MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
    mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        //int t_sec = (MT_U32)s64TimeInMs/1000;
        mplayer_pri = (mtUNF_MPLAYER_PRIVATE_S *)phdl->pri;
        mplayer_pri->fseq->play_at_time((void *)mplayer_pri->fseq, t_sec);
    } else if (phdl->ptype == MT_SUPLAYER_GSTREAMER) {
    } else {
    }
#endif
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_Deinit(MT_HANDLE hPlayer)
{
    MLOGI("%s \n", __FUNCTION__);
    MT_SUPLAY_LOCK();
    MT_SUPLAY_UNLOCK();
    return 0;
}

MT_S32 MT_SVR_PLAYER_SetParam(MT_HANDLE hPlayer, MT_SVR_PLAYER_ATTR_E eAttrId, const MT_VOID *pArg)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        switch (eAttrId) {
            case MT_SVR_PLAYER_ATTR_STREAMID: {
                MT_SVR_PLAYER_STREAMID_S *tmp = (MT_SVR_PLAYER_STREAMID_S *)pArg;
				if(!tmp) {
					 MT_SUPLAY_SEM_GIVE(phdl->suplock);
					 return MT_FAILURE;
				}
                mplayer_pri->p_file_seq->change_audio_track((void *)mplayer_pri->p_file_seq, tmp->u16AudStreamId);
                mplayer_pri->p_file_seq->change_video_track((void *)mplayer_pri->p_file_seq, tmp->u16VidStreamId);
                mplayer_pri->p_file_seq->set_subt_id((void *)mplayer_pri->p_file_seq, tmp->u16VidStreamId);
                }
                break;
            case MT_SVR_PLAYER_ATTR_SYNC:
                break;
            default:
                break;
        }
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_GetParam(MT_HANDLE hPlayer, MT_SVR_PLAYER_ATTR_E eAttrId, MT_VOID *pArg)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    MT_S32 ret = MT_SUCCESS;
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        switch (eAttrId) {
            case MT_SVR_PLAYER_ATTR_STREAMID: {
                    MT_SVR_PLAYER_STREAMID_S *tmp = (MT_SVR_PLAYER_STREAMID_S *)pArg;
                    FILM_INFO_T film;
                    if (tmp == NULL) {
                        ret = MT_FAILURE;
                        goto EXIT;
                    }
                    mplayer_pri->p_file_seq->get_film_info((void *)mplayer_pri->p_file_seq, &film);
                    tmp->u16AudStreamId = film.audio_track_id;
                    tmp->u16VidStreamId = film.video_track_num;
                    tmp->u16SubStreamId = mplayer_pri->p_file_seq->subt_id;
                }
                break;
            case MT_SVR_PLAYER_ATTR_AUD_PID: {
                MT_SVR_PLAYER_PID_LIST_S *lists = (MT_SVR_PLAYER_PID_LIST_S *)pArg;
				if (lists == NULL) {
                        ret = MT_FAILURE;
                        goto EXIT;
                }
                (void) file_seq_get_audio_pid(mplayer_pri->p_file_seq, lists->pid_num, lists->pids);
                break;
            }
            case MT_SVR_PLAYER_ATTR_VID_PID: {
                MT_SVR_PLAYER_PID_LIST_S *lists = (MT_SVR_PLAYER_PID_LIST_S *)pArg;
				if (lists == NULL) {
                        ret = MT_FAILURE;
                        goto EXIT;
                }
                (void) file_seq_get_video_pid(mplayer_pri->p_file_seq, lists->pid_num, lists->pids);
                break;
            }
            case MT_SVR_PLAYER_ATTR_NET_BITRATE: {
                long long bitrate = 0;
                if (pArg && MT_SUCCESS ==
                    sup_get_network_bitrate(mplayer_pri->p_file_seq, &bitrate)) {
                    *(long long *) pArg = bitrate;
                } else {
                    ret = MT_FAILURE;
                }
                break;
            }
            case MT_SVR_PLAYER_ATTR_WINDOW_HDL:
                break;
            case MT_SVR_PLAYER_ATTR_AVPLAYER_HDL:
                break;
            case MT_SVR_PLAYER_ATTR_SUBTITLE_HDL:
                break;
            case MT_SVR_PLAYER_ATTR_SO_HDL:
                break;
            case MT_SVR_PLAYER_ATTR_AUDTRACK_HDL:
                break;
            case MT_SVR_PLAYER_ATTR_SYNC:
                break;
            case MT_SVR_PLAYER_ATTR_VSINK_HDL:
                break;
            default:
                break;
        }
    }

EXIT:
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_PLAYER_Set_Aud_Track(MT_HANDLE hPlayer, int track_id)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;

    H2S(hPlayer, phdl);
    MLOGI("%s_%d:set audio_id=%d\n", __FUNCTION__, __LINE__, track_id);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        mplayer_pri->p_file_seq->change_audio_track((void *)mplayer_pri->p_file_seq, track_id);
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_Set_Subtitle(MT_HANDLE hPlayer, int sub_id)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;

    H2S(hPlayer, phdl);

    MLOGI("%s_%d:set subtitle_id=%d\n", __FUNCTION__, __LINE__, sub_id);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;

        S2P(phdl, mplayer_pri);
        mplayer_pri->p_file_seq->set_subt_id((void *)mplayer_pri->p_file_seq, sub_id);
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_GetAdaptivePlaylist(MT_HANDLE hPlayer,
    int list_array_num, MT_SVR_PLAYER_ADAPTIVE_PLAYLIST_S *list)
{
    int ret = MT_SUCCESS;
    mtUNF_SUPLAYER_STATUS_S *phdl;

    MLOGI("Get adaptive playlist, num:%d list:%p\n", list_array_num, list);
    if (list_array_num < 0 || !list) {
        return MT_FAILURE;
    }

    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        ret = sup_get_adaptive_playlist(
            mplayer_pri->p_file_seq, list_array_num, NULL, NULL, list);
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_PLAYER_SetAdaptivePlaylist(MT_HANDLE hPlayer, int playlist_index)
{
    int ret = MT_SUCCESS;
    mtUNF_SUPLAYER_STATUS_S *phdl;

    H2S(hPlayer, phdl);
    MLOGI("%s index %d\n", __FUNCTION__, playlist_index);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
        S2P(phdl, mplayer_pri);
        ret = sup_switch_adaptive_playlist((void *)mplayer_pri->p_file_seq, playlist_index);
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_DUMP_VIDEO_DATA(MT_HANDLE hPlayer)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;

    H2S(hPlayer, phdl);
    MLOGD("[%s] [%d]\n", __FUNCTION__, __LINE__);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;

        S2P(phdl, mplayer_pri);
        mplayer_pri->p_file_seq->mp_dump();
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_DUMP_AUDIO_DATA(MT_HANDLE hPlayer)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;

    H2S(hPlayer, phdl);
    MLOGD("[%s] [%d]\n", __FUNCTION__, __LINE__);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;

        S2P(phdl, mplayer_pri);
        mplayer_pri->p_file_seq->mp_dumpaudio();
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}


MT_S32 MT_SVR_PLAYER_Get_Media_Info(MT_HANDLE hPlayer, void *pResult)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    MT_S32 ret = (MT_S32)MT_SUCCESS;
	mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
	FILM_INFO_T  film_info;
	file_seq_video_subtitle_t *subt = NULL;

	if (pResult == NULL) {
            MLOGE("%s_%d:Error input args\n", __func__, __LINE__);
            ret = (MT_S32)MT_FAILURE;
			  return ret;
	}

    CHECKSTSBYHANDLE(hPlayer, MT_SVR_PLAYER_STATE_STOP, MT_FAILURE);
    H2S(hPlayer, phdl);
	S2P(phdl, mplayer_pri);

	if (mplayer_pri->p_file_seq->get_status((void *)mplayer_pri->p_file_seq) == FILE_SEQ_LOADMEDIA) {
		MLOGI("media is loading,please wait for a moment\n");
		return MT_FAILURE;
	}

    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {

        mplayer_pri->p_file_seq->get_subt_info((void *)mplayer_pri->p_file_seq, (void **)&subt);
        ((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->subt = (MT_SVR_PLAYER_SUBT_S *)subt;
        mplayer_pri->p_file_seq->get_audio_track_lang((void *)mplayer_pri->p_file_seq, NULL);
        ((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->aud_lang = (MT_SVR_PLAYER_TRACK_LANG_S *)mplayer_pri->p_file_seq->audio_lang_array;
        mplayer_pri->p_file_seq->get_film_info((void *)mplayer_pri->p_file_seq, &film_info);
        /*Make sure FILE_INFO_S is started with FILM_INFO_T*/
        if (sizeof(MT_SVR_PLAYER_FILM_S) == sizeof(FILM_INFO_T)) {
            ((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->film = *((MT_SVR_PLAYER_FILM_S *)&film_info);
        } else {
            MLOGE("%s_%d:Error structure different!!!\n", __func__, __LINE__);
        }
        ((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->aud_lang_cnt = mplayer_pri->p_file_seq->audio_track_num;
        MT_SVR_PLAYER_FILE_INFO_S *info = (MT_SVR_PLAYER_FILE_INFO_S *) pResult;
        (void) sup_get_adaptive_playlist(mplayer_pri->p_file_seq, 0,
            &info->adaptive_playlist_num, &info->adaptive_playlist_idx, NULL);
    }

EXIT:
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_PLAYER_Get_Subt_Data(MT_HANDLE hPlayer, void *subt, void *pts, MT_U32 *size)
{
    MT_S32 ret = (MT_S32)MT_SUCCESS;
    mtUNF_SUPLAYER_STATUS_S *phdl;

    CHECKSTSBYHANDLE(hPlayer, MT_SVR_PLAYER_STATE_STOP, MT_FAILURE);
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;

        S2P(phdl, mplayer_pri);

        if (subt == NULL || pts == NULL || size == NULL) {
            MLOGE("%s: invalid param(subt %p, pts %p, size %p)!\n", __FUNCTION__, subt, pts, size);
            ret = (MT_S32)MT_FAILURE;
            goto EXIT;
        }

        *size = mplayer_pri->p_file_seq->get_subt((void *)mplayer_pri->p_file_seq, (unsigned char **)subt, pts);
        ret = (MT_S32)MT_SUCCESS;
    }
EXIT:
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_PLAYER_GetPlayerInfo(MT_HANDLE hPlayer, MT_SVR_PLAYER_INFO_S *pstruInfo)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    FILM_INFO_T  film_info;
    H2S(hPlayer, phdl);
    mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;
    MT_S32 ret = (MT_S32)MT_SUCCESS;
	if(!pstruInfo) {
		return MT_FAILURE;
	}
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    S2P(phdl, mplayer_pri);
    pstruInfo->eStatus = phdl->status;

    pstruInfo->u64TimePlayed = g_last_callback_vpts;// for CI EOF, check pts (bug 19879)  move here

    MT_SVR_PLAYER_CHECK_STS(MT_SVR_PLAYER_STATE_STOP, phdl, 0, 1);
    MT_SVR_PLAYER_CHECK_STS(MT_SVR_PLAYER_STATE_IDLE, phdl, 0, 1);
    MT_SVR_PLAYER_CHECK_STS(MT_SVR_PLAYER_STATE_CREATE, phdl, 0, 1);
    MT_SVR_PLAYER_CHECK_STS(MT_SVR_PLAYER_STATE_SETPATH, phdl, 0, 1);
    MT_SVR_PLAYER_CHECK_STS(MT_SVR_PLAYER_STATE_LOADED, phdl, 0, 1);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        if (mplayer_pri->p_file_seq->m_play_state != FILE_SEQ_STOP) {
            //pstruInfo->u64TimePlayed = g_last_callback_vpts;
            pstruInfo->eStatus = phdl->status;
            pstruInfo->s32Speed = mplayer_pri->p_file_seq->cur_speed;
            if (g_cur_duration == 0) {
                mplayer_pri->p_file_seq->get_film_info((void *)mplayer_pri->p_file_seq, &film_info);
                CHECKSTSBYHANDLE(hPlayer, MT_SVR_PLAYER_STATE_STOP, MT_FAILURE);
                pstruInfo->film_duration = film_info.film_duration;
				g_cur_duration = film_info.film_duration;
				MLOGD("played [%lld], duraton [%d] ms\n", pstruInfo->u64TimePlayed, pstruInfo->film_duration);
            } else {
                pstruInfo->film_duration = g_cur_duration;
            }
        }
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_PLAYER_SetAvplayHdl(MT_HANDLE hPlayer, MT_HANDLE hAvplay, MT_HANDLE hTrack)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        file_seq_avplay_handle_set(hAvplay, hTrack);
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_SetAvplayHdl_Only(MT_HANDLE hPlayer, MT_HANDLE hAvplay)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        file_seq_avplay_handle_set(hAvplay, 0);
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_SetVoHdl(MT_HANDLE hPlayer, int vHandle)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        file_seq_vo_handle_set(vHandle);
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_Set_T1xbase_Num(MT_HANDLE hPlayer,
								int t1xnum)
{
    MT_S32 ret = (MT_S32)MT_SUCCESS;
    mtUNF_SUPLAYER_STATUS_S *phdl;
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return ret;
}

MT_S32 MT_SVR_PLAYER_GetTagInfo(MT_HANDLE hPlayer, char *p_tag_key, char *p_tag_value, int tag_value_len)
{
    MT_BOOL ret = MT_FALSE;
    mtUNF_SUPLAYER_STATUS_S *phdl;

    MLOGD("func [%s], line [%d]\n", __FUNCTION__, __LINE__);
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);
    // MT_SVR_PLAYER_CHECK_STS(MT_SVR_PLAYER_STATE_LOADED, phdl, 1, 1);

    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;

        S2P(phdl, mplayer_pri);

        MLOGD("[%s] mplayer_pri [%p], p_tag_key [%p], p_tag_value ??%p??, tag_value_len [%d]\n", __func__, mplayer_pri, p_tag_key, p_tag_value, tag_value_len);
        if (NULL == p_tag_key || NULL == p_tag_value || 0 == tag_value_len) {
            MLOGE("%s: invalid param(p_tag_key %p, p_tag_value %p, tag_value_len %d)!\n", __FUNCTION__, p_tag_key, p_tag_value, tag_value_len);
            ret = MT_FALSE;
            goto EXIT;
        }

        ret = mplayer_pri->p_file_seq->get_mp_tag((void *)mplayer_pri->p_file_seq, p_tag_key, p_tag_value, tag_value_len);
    }

EXIT:
    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return (MT_TRUE == ret) ? MT_SUCCESS : MT_FAILURE;
}

MT_S32 MT_SVR_PLAYER_SetSupportSeekFinish(MT_HANDLE hPlayer)
{
    mtUNF_SUPLAYER_STATUS_S *phdl;

    MLOGD("func [%s], line [%d]\n", __FUNCTION__, __LINE__);
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);

    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;

        S2P(phdl, mplayer_pri);

        mplayer_pri->p_file_seq->set_support_seek_finish((void *)mplayer_pri->p_file_seq);
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_SET_NETWORK_BUFFERING(MT_HANDLE hPlayer, MT_BOOL open) {
    mtUNF_SUPLAYER_STATUS_S *phdl;

    MLOGD("func [%s], line [%d]\n", __FUNCTION__, __LINE__);
    H2S(hPlayer, phdl);
    MT_SUPLAY_SEM_TAKE(phdl->suplock, 0);

    if (phdl->ptype == MT_SUPLAYER_MPLAYER) {
        mtUNF_MPLAYER_PRIVATE_S *mplayer_pri;

        S2P(phdl, mplayer_pri);

        mplayer_pri->p_file_seq->set_network_buffering((void *)mplayer_pri->p_file_seq, open);
    }

    MT_SUPLAY_SEM_GIVE(phdl->suplock);
    return MT_SUCCESS;
}

/* +++++++++ WB auto test interface start +++++++*/
MT_S32 MT_SVR_PLAYER_Init_Adec_Param_Ptr(void)
{
    MT_S32 ret = (MT_S32)MT_FAILURE;
    ret = (MT_S32)wb_auto_aud_init_dec_param();
    return ret;
}

MT_S32 MT_SVR_PLAYER_Get_Adec_Param_Ptr(void *ptr, int size)
{
    MT_S32 ret = (MT_S32)MT_FAILURE;
    ret = (MT_S32)wb_auto_aud_get_dec_param(ptr, size);
    if ((MT_S32)MT_FAILURE == ret) {
        memset(ptr, 0, size);
    }
    return ret;
}

MT_S32 MT_SVR_PLAYER_Init_Es_Ptr(int max_frame, int max_asize)
{
    MT_S32 ret = (MT_S32)MT_FAILURE;
    ret = (MT_S32)wb_auto_init_es_check_sum(max_frame, max_asize);
    return ret;
}

MT_S32 MT_SVR_PLAYER_Get_Es_Magic_Num(void *ves, void *aes, void *ses)
{
    MT_S32 ret = (MT_S32)MT_FAILURE;
    ret = (MT_S32)wb_auto_get_es_check_sum(ves, aes, ses);
    return ret;
}
/*-------------WB auto test interface end ----------*/
