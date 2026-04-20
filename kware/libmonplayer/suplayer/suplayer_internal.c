/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#define MODULE_TAG "SUPI"
#include "mutil.h"
#include "mlog.h"
#include "file_playback_sequence.h"
#include "mtsu_svr_player.h"
#include "suplayer_internal.h"
#include "mt_audio_codec.h"
#include "mt_unf_video.h"

#if MT_DES("Internal function", 1)
#endif

#if MT_DES("External API Definition", 1)

int sup_get_network_bitrate(void *file_seq, long long *bitrate)
{
    if (!bitrate) {
        return MT_FAILURE;
    }
    return file_seq_get_network_bitrate(file_seq, bitrate);
}

int sup_get_adaptive_playlist(void *file_seq, int list_array_num,
    int *total_num, int *current_idx, MT_SVR_PLAYER_ADAPTIVE_PLAYLIST_S *list)
{
    FILE_SEQ_T *p_file_seq = file_seq;
    if (!p_file_seq) {
        return MT_FAILURE;
    }

    int ret = file_seq_get_playlist(p_file_seq);
    if (MT_SUCCESS != ret) {
        return ret;
    }
    if (total_num) {
        *total_num = p_file_seq->adaptive_playlist.num;
    }
    if (current_idx) {
        *current_idx = p_file_seq->adaptive_playlist.curr_idx;
    }
    if (list_array_num > 0 && list) {
        int copy_num =
            (list_array_num > p_file_seq->adaptive_playlist.num) ?
                p_file_seq->adaptive_playlist.num : list_array_num;
        FILE_SEQ_ADAPTIVE_PLAYLIST_T *srcl = p_file_seq->adaptive_playlist.list;
        for (int num = 0; num < copy_num; num++) {
            list[num].index  = srcl[num].index;
            list[num].flags  = srcl[num].flags;
            list[num].width  = srcl[num].width;
            list[num].height = srcl[num].height;
            list[num].codec_tag = srcl[num].codec_tag;
            list[num].bandwidth = srcl[num].bandwidth;
            list[num].framerate.num = srcl[num].framerate.num;
            list[num].framerate.den = srcl[num].framerate.den;
        }
    }
    return MT_SUCCESS;
}

int sup_switch_adaptive_playlist(void *file_seq, int playlist_index)
{
    FILE_SEQ_T *p_file_seq = file_seq;
    if (!p_file_seq || playlist_index < 0) {
        return MT_FAILURE;
    }
    return file_seq_switch_playlist(p_file_seq, playlist_index);
}

int sup_seq_set_extio_data_source(
    void *file_seq, MT_SVR_PLAYER_EXTIO_CONTEXT_S *extio)
{
    FILE_SEQ_T *p_file_seq = file_seq;
    if (!p_file_seq || !extio) {
        return MT_FAILURE;
    }

    FILE_PLAYBACK_EXTIO_CONTEXT_T ioctx = {
        .opaque         = extio->opaque,
        .init           = extio->init,
        .deinit         = extio->deinit,
        .read           = extio->read ,
        .seek           = extio->seek,
        .eof            = extio->eof,
        .get_size       = extio->get_size,
        .get_pos        = extio->get_pos,
        .get_seekable   = extio->get_seekable,
        .get_media_type = extio->get_media_type,
    };

    return file_seq_set_extio_data_source(p_file_seq, &ioctx);
}

int sup_seq_cfg_stream_probe_para(void *file_seq,
    int max_stream_probe_size, int max_stream_analyze_duration)
{
    FILE_SEQ_T *p_file_seq = file_seq;
    if (!p_file_seq || max_stream_probe_size <= 0 || max_stream_analyze_duration <= 0) {
        return MT_FAILURE;
    }

    return file_seq_cfg_stream_probe_para(
        file_seq, max_stream_probe_size, max_stream_analyze_duration);
}

int sup_set_input_para(void *file_seq, MT_SVR_PLAYER_IN_PARA_S *para)
{
    int idx, jdx;
    MT_SVR_PLAYER_PARA_LIST_S *lists = NULL;
    FILE_SEQ_T *p_file_seq           = file_seq;
    STREAM_TYPE_E stream_type[MT_FORMAT_DATA_BUTT][2] = {
        {MT_FORMAT_DATA_NULL,  STREAM_TYPE_NULL},
        {MT_FORMAT_DATA_AUD ,  STREAM_TYPE_AUD },
        {MT_FORMAT_DATA_VID ,  STREAM_TYPE_VID },
        {MT_FORMAT_DATA_SUB ,  STREAM_TYPE_SUB },
        {MT_FORMAT_DATA_RAW ,  STREAM_TYPE_RAW },
        {MT_FORMAT_DATA_BUTT,  STREAM_TYPE_BUTT},
    };

    if (NULL == p_file_seq || NULL == para) {
        MLOGW("Set sup input para error\n");
        return MT_FAILURE;
    }

    /* set stream para */
    idx = 0;
    while (idx < MT_FORMAT_DATA_BUTT) {
        lists = para->stream_para[idx].codec_blacklist;
        if (NULL != lists && lists->num > 0 && NULL != lists->list) {
            /* stream config disorder */
            for (jdx = 0; jdx < MT_FORMAT_DATA_BUTT; jdx++) {
                if (para->stream_para[idx].stream_type == stream_type[jdx][0]) {
                    (void) fp_set_codec_blacklist(p_file_seq,
                        stream_type[jdx % MT_FORMAT_DATA_BUTT][1], lists->num, lists->list);
                }
            }
        }
        idx++;
    }

    FILE_PLAYBACK_NETWORK_PARA_T netpara = {
        .headers    = para->net.headers,
        .user_agent = para->net.user_agent,
        .cookies    =  para->net.cookies,
    };
    p_file_seq->user_int_cb.callback = para->int_cb.callback;
    p_file_seq->user_int_cb.callback_ctx = para->int_cb.callback_ctx;
    return file_seq_set_network_parameter(p_file_seq, &netpara);
}

#endif
