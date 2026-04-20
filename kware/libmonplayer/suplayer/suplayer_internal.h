/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MON_PLAYER_INTERNAL_H__
#define OS_PRINTF MLOGD
#define __MON_PLAYER_INTERNAL_H__
#define URL_LEN 4096
#define SUPLAYER_INSTANCE_MASK (0xffff)
#define SUPLAYER_PLAYER_TYPE_MASK (0xffff0000)
#define SUPLAYER_PLAYER_TYPE_SHIT (16)

typedef struct
{
    void *p_demuxer;
    void *p_stream;
    void *p_demuxer_video;
    void *p_demuxer_audio;
    void *p_demuxer_sub;
    float video_es_pts;
    float audio_es_pts;
    float sub_es_pts;
    int video_es_size;
    int *video_es_start;
    int audio_es_size;
    int *audio_es_start;
    int *p_extra_aud_buf;
    int extra_audio_size;
    void *cb;
    char url[URL_LEN];
    int is_exit;
    int has_audio;
    int has_video;
    int vdec_type;
    int adec_type;
    int is_trickmode;
    void *hAvplay;
    pthread_t g_monEsThd;
    void *suplayer_status;
} MT_PLAYBACK_INTERNAL_T;
#if MT_DES("External API Definition", 1)
int sup_get_network_bitrate(void *file_seq, long long *bitrate);
int sup_get_adaptive_playlist(void *file_seq, int list_array_num,
    int *total_num, int *current_idx, MT_SVR_PLAYER_ADAPTIVE_PLAYLIST_S *list);
int sup_switch_adaptive_playlist(void *file_seq, int playlist_index);
int sup_seq_set_extio_data_source(
    void *file_seq, MT_SVR_PLAYER_EXTIO_CONTEXT_S *extio);
int sup_seq_cfg_stream_probe_para(void *file_seq,
    int max_stream_probe_size, int max_stream_analyze_duration);

int sup_set_input_para(void *file_seq, MT_SVR_PLAYER_IN_PARA_S *para);
#endif

#endif
