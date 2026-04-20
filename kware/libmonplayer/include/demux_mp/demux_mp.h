/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
#ifndef __DEMUX_MP_H__
#define __DEMUX_MP_H__

#define OLD_CODE 1
typedef enum MP2FFMPEG_CMD{
	MP_INIT_FFMPEG_MEM,
	MP_DEINIT_FFMPEG_MEM,
	MP_STATE_EXT,
	MP_FFMPEG_MOV_SEG_NUM,
	MP_FFMPEG_MOV_KEYGEN,
	MP_FFMPEG_MOV_KEYGEN_LEN,
	MP_FFMPEG_SET_FILEPLAY_SPEED,
	MP_FFMPEG_SET_FAKE_MPD,
	MP_DEINIT_AVFMT,
} MP2FFMPEG_CMD_E;

typedef enum MP2FFMPEG_CMD_OPT{
	MP_DO_CALLBACK,
	MP_DO_SET_PARAM,
	MP_DO_SET_AVFMT,
	MP_DO_GET_PARAM,
} MP2FFMPEG_CMD_OPT_E;

#define DEMUX_SEL_GET_NEXT_PKT                  0
#define DEMUX_SEL_PUSH_CUR_PKT                  1
#define DEMUX_SEL_PUSH_CUR_END_PKT              2
#define DEMUX_SEL_SAVE_AND_PUSH_PKT             3
#define DEMUX_SEL_SAVE_PTS_AND_PUSH_PKT         4
#define DEMUX_SEL_PUSH_SAVED_KEY_PKT            5
#define DEMUX_SEL_MERGE_AND_PUSH_PKT            6
#define DEMUX_SEL_SLEEP_AND_PUSH_PKT            10
#define MAX_DEMUX_MP_STREAM_CNT                 32


/*
	this struct is for ps_trick
*/
typedef struct{
    int seek_before_tick;
    int seek_after_tick;
}ps_trick_t;



/*!
  This structure defines the postprocessing mode
  */
typedef struct
{
    /*!
  This structure defines the postprocessing mode
  */
    int progid;
    /*!
  pcr pid
  */
    int pcr_pid;
    /*!
  This structure defines the postprocessing mode
  */
    int es_cnt;
    /*!
  This structure defines the postprocessing mode
  */
    struct pmt_ts_es
    {
	/*!
  This structure defines the postprocessing mode
  */
	int pid;
	/*!
  This structure defines the postprocessing mode
  */
	int type;
	/*!
  This structure defines the postprocessing mode
  */
	char lang[4];
	/*!
  This structure defines the eac3 mode
  */
	char audio_eac3_flag;
    } es[64];
    /*!
  audio es stream count
  */
    int aud_stream_cnt;
    /*!
  video es stream array
  */
    struct pmt_ts_es aud_stream[MAX_DEMUX_MP_STREAM_CNT];
    /*!
  video es stream count
  */
    int vid_stream_cnt;
    /*!
  video es stream array
  */
    struct pmt_ts_es vid_stream[MAX_DEMUX_MP_STREAM_CNT];
    /*!
   subtitle count
  */
    int sub_title_cnt;
    /*!
  subtitle array
  */
    struct pmt_ts_es sub_title[MAX_DEMUX_MP_STREAM_CNT];

} pmt_ts_t;
/*!
  This structure defines the postprocessing mode
  */
typedef struct
{
    /*!
  This structure defines the postprocessing mode
  */
    pmt_ts_t *p_pmt;
    /*!
  This structure defines the postprocessing mode
  */
    int pmt_cnt;

} ts_info_t;
/*!
 xxxxxxxxxxxxx
*/
void ts_get_pcm_info(demuxer_t *demuxer,
    int *is_big_endian, int *bits, int *channel, int *sample_rate);

/*!
 xxxxxxxxxxxxx
*/
void ds_get_video_codec_type(demux_stream_t *ds,
                             int *p_video_type, int *vpid, int *pcr_pid);
/*!
 xxxxxxxxxxxxx
*/
int ds_get_video_bps(demux_stream_t *ds);
/*!
 xxxxxxxxxxxxx
*/
demuxer_t *demux_mp_open(stream_t *vs, int file_format, int audio_id_mp,
                         int video_id_mp, int dvdsub_id_mp, char *filename);
/*!
 xxxxxxxxxxxxx
*/
ts_info_t *ds_ts_prog(demuxer_t *demuxer);
/*!
 xxxxxxxxxxxxx
*/
int ds_get_audio_count(demuxer_t *d);
/*!
 xxxxxxxxxxxxx
*/
int ds_get_video_count(demuxer_t *d);
/*!
 xxxxxxxxxxxxx
*/
int ds_get_packet_video(demux_stream_t *ds, unsigned char **start, int8_t speed);
/*!
 xxxxxxxxxxxxx
*/
int ts_get_audio_track_pid(
    demuxer_t *demuxer, ts_info_t *p_ts_priv, int id);
int ds_get_audio_pid(demux_stream_t *ds,
    ts_info_t *ds_ts_priv, const int id_num, int *pid_list);
int ds_get_video_pid(demux_stream_t *ds,
    ts_info_t *ds_ts_priv, const int id_num, int *pid_list);
int ds_get_network_bitrate(
    demux_stream_t *ds, long long *bitrate);

/*!
 xxxxxxxxxxxxx
*/
void set_ts_pmt_avs_info(ts_info_t *p_ts_priv);
/*!
 xxxxxxxxxxxxx
*/
int ds_get_audio_bps(demux_stream_t *ds);
#ifdef OLD_CODE
/*!
 xxxxxxxxxxxxx
*/
int ds_get_audio_codec_info(demux_stream_t *ds, unsigned int *p_audio_type, int *pcm_be,
                            unsigned int *acodec_id, int *apid, AUDIO_OUT_MODE aout_mode);
#endif
/*!
 xxxxxxxxxxxxx
*/
int ds_get_packet_audio(demux_stream_t *ds, unsigned char **start,
                        uint8_t **extra_buf, uint8_t *extra_size);
/*!
 xxxxxxxxxxxxx
*/
void mp_ffmpeg_ext_cmd(int cmd, int opt, void *val);
void mp_ffmpeg422_get_drm_index(int *index1, int *index2);
void mp_ffmpeg422_clear_drm_index(void);
void wma_get_dec_info(demuxer_t *demuxer, void * param);

u32 is_avi_stream(demuxer_t* demuxer);
void ds_get_vfilter_type(demux_stream_t *ds);
int ds_ts_check_audio_aid(demuxer_t *demuxer,int apid);
void ds_detect_hls_reset(void);
int ds_is_codec_h264(unsigned int id);
void ds_reset_trickplay_para(demuxer_t *demuxer);
int ds_packet_seekforward(demux_stream_t *ds, int speed_cntl);
#endif
