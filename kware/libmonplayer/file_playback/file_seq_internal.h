/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __FILE_SEQ_INTERNAL_H__
#define __FILE_SEQ_INTERNAL_H__

#include "mutil.h"
#include "ts_seq/ts_sequence.h"
#include "mt_unf_video.h"
#include "mtlz_avfilter.h"

#define  USER_CMD_FIFO_LEN   (16)
/*
 *
 *   all types of command supported by file playback sequence
 *
 */
#define CMD_STOP                   (1L << 1)
#define CMD_START                  (1L << 2)
#define CMD_PAUSE                  (1L << 3)
#define CMD_RESUME                 (1L << 4)
#define CMD_FF2X_PLAY              (1L << 5)
#define CMD_FF4X_PLAY              (1L << 6)
#define CMD_FF8X_PLAY              (1L << 7)
#define CMD_FF16X_PLAY             (1L << 8)
#define CMD_FF32X_PLAY             (1L << 8)
#define CMD_FF64X_PLAY             (1L << 9)
#define CMD_CHECK_MEM              (1L << 10)
#define CMD_CHANGE_SPEED           (1L << 11)
#define CMD_PLAY_AT_TIME           (1L << 12)
#define CMD_CHANGE_AUDIO_TRACK     (1L << 13)
#define CMD_CHANGE_SUBT_ID         (1L << 14)
#define CMD_CHANGE_PLAYLIST        (1L << 15)

#ifndef MP_NOPTS_VALUE
#define MP_NOPTS_VALUE        (-1LL<<63)
#endif

#define    CHECK_AV_ES_WL_BASE  (32)
#define    DEFAULT_ES_WL_VAL  (4) //sec
#define    FILE_PATH_MAX_LEN  (256)
#define   DEFAULT_VIDEO_NAME  "stream.dump"
#define   VES_TMP_BUF_LEN    (1024*128)
#define   DEFAULT_AUDIO_NAME "stream.dump.audio2"
#define   AES_TMP_BUF_LEN    (1024 * 32)
#define   TS_FILE_BUFFER_SIZE   (2*512 * 188)

#define   PACKET_HEADER_LEN (8)
#define   FRAME_SIZE_FIELD_LEN (4)
#define   PTS_FIELD_LEN  (4)
#define   VIDEO_TMP_BUF_LEN   (128*1024)
#define   VIDEO_ES_BUF_OVERFLOW_THRESHOLD  (100) // k bytes
#define   AUDIO_ES_BUF_OVERFLOW_THRESHOLD  (4*1024)
#define   DEFAULT_VIDEO_BPS   (16*1024)
#define   SUB_FIFO_LEN             (128*1024)

#define   PB_FIFO_LEN             (5*1024*1024)
#define   T_PB_FIFO_START     (1*1024*1024)
#define   HTTP_RECV_LEN		20*1024
#define   T_PB_FIFO_BUFFING_TIME  3000//ms


#define     VALID_PTS_MASK     (0x00000000)//(0x80000000)
#define     MAX_PATH_LEN                 (256*3)
#define     AUD_ASSEMBLE_BUF_LEN  (64*1024)
#define     DELIVER_VPTS_INTERVAL    ((1000)*1000) //((1000+500)*1000)         //1.5 seconds
#define     AUDIO_MAX_BPS                (500*1000)
#define     AUDIO_DEFAULT_BPS            (15*1000)

/*
 *
 *   all types of event supported by file playback sequence
 *
 */
#define     CHECK_SYSTEM_MEM   (0x00000001)
#define     CLEAR_AUD_ES_BUF    (0x00000002)
#define     CLEAR_VIDEO_ES_BUF  (0x0000004)
#define     GET_TS_MEDIA_INFO   (0x00000008)
#define     AUTO_DELIVER_VPTS        (0x00000010)
#define   GET_ES_VIDEO_W_H (0x00000020)
#define   CHECK_TRICKPLAY  (0x00000040)
#define     MAX_EVENT                  (0x80000000)

#define    DEFAULT_SD_BPS    (100) //Kbytes/sec
#define    DEFAULT_HD_BPS    (200)//KBytes/sec
#define    DEF_SUPER_HD_BPS    (2*1024)//KBytes/sec
#define    DEFAULT_BUFFERING_SECOND   (2)//default buffer 2 seconds es data

#define    TIME_BASE (1000) //1000 ms

#define BLANK_LEN  (0)
#define UPPER_SIZE(x)  ((((x) + 7) & (~7)) + BLANK_LEN)
#define IS_SET(x,y)        ((x) & (y))
#define ClearFlag(x,y)    ((x) &= ~(y))
#define SetFlag(x,y)    ((x) |= (y))

#define PRELOAD_VIDEO_BUFFER_SIZE               (1*1024*1024)
#define PRELOAD_AUDIO_BUFFER_SIZE               (256*1024)
#define  VES_SEG_LEN  (1024)
#define  AES_SEG_LEN   (64)
#define AUDIO_SEND 2
#define CONTINUE_READ_AUD_PACKETS_THRESHOLD  100
#define CONTINUE_READ_VID_PACKETS_THRESHOLD  50
#define SEG_AUD_SIZE (AUD_ASSEMBLE_BUF_LEN - AUDIO_ES_BUF_OVERFLOW_THRESHOLD)
#define RTSP_EOF_DELAY_DEFAULT 10000000
#define MAX_BLACKLIST_CODEC_CNTS             50

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

typedef struct {
    MT_BOOL is_open;            // TRUE OPEN network buffering function
    MT_BOOL is_enbale;          // calculating playback buffer after starting playback 2 seconds
    u32     fp_start_tick;
    MT_BOOL is_buffering;
    u32     last_used;
    u32     start_used;
    u32     start_tick;

    int     resolution_type;
}NETWORK_BUFFERING_INFO;

typedef struct {

    int size;
    u8  * data;
    u8  * header;
    u32   header_len;
    double        pts;
    void  * p_next;

} AUDIO_PACKET_T;
typedef struct {

    int is_ts;
    int is_stable;
    u8  only_audio_mode;
    u8  demuxer_type;
    u32   vpts_upload;
    int getted_first_vpts;
    double   first_vpts;
    double max_video_pts;
    int audio_bps;
    VIDEO_W_H_FPS video_whfps;
    int checkDefinitionOK;
    int checkAvTypeOK;
} ADSFILE_PARA_T;

/*!
  xxxxxxxx
  */
typedef enum {
    /*!
    *    player is buffering data
    **/
    FILE_SEQ_BUFFERING_END,
    /*!
    *    player is buffering data
    **/
    FILE_SEQ_BUFFERING,


} FILE_SEQ__BUFFERING_STATUS;

typedef enum {
    FILE_SEQ_STATUS_IDLE,
    FILE_SEQ_STATUS_WORK,
} FILE_SEQ_WORK_STATUS;

/*!
  xxxxxxxx
  */
typedef enum {
    /*!
    *    player is buffering data
    **/
    CHIPTYPE_ARIA,
    CHIPTYPE_SYMPHONY1,
    CHIPTYPE_SYMPHONY2,
    CHIPTYPE_SYMPHONY3,
    CHIPTYPE_SYMPHONY4,
    CHIPTYPE_SYMPHONY6,
    CHIPTYPE_BUFF,
} FILE_SEQ_CHIPTYPE;

/* Use for multithead case for track change */
enum {
    FPBI_STATE_NONE = 0,
    FPBI_STATE_FLUSHED,
    FPBI_STATE_SYNC_PTS,
    FPBI_STATE_SYNC_PTS_PRE,
    FPBI_STATE_SYNC_CONTENT,
    FPBI_STATE_SYNC_CONTENT_PRE,
    FPBI_STATE_PLAYLIST_CHANGED,
    FPBI_STATE_DECODER_STOPPED,
};

/* codec_type for external use, and id for internal use  */
typedef struct {
    unsigned int codec_id;
    int width;
    int height;
    double duration;
    m_rational_t frame_rate;
    unsigned int need_insert_header;
    int codec_blacklist[MAX_BLACKLIST_CODEC_CNTS];

    int codec_extradata_size;
    /* some codec extra data need more memory,
     * so we directly use the stream data
     */
    unsigned char *codec_extradata;
    unsigned char is_secure;
    MTAVFilter mtavf;
    /* For synchronization in multithread */
    unsigned int state;
    void *ff_av_pkt;
    FILE *dump_ves_fp;
    /* estimated packet cnt in es buffer */
    int pkt_cnt_in_buf;
    int pkt_cnt_underflow;
    mlzp_mutex_t *mutex;
    /* current played segment start pts, us */
    int64_t segment_start_pts;
} FILE_SEQ_VIDEO_T;

typedef struct {
    int pcm_be;
    unsigned int codec_id;
    unsigned int channels;
    unsigned int sample_rate;
    int codec_blacklist[MAX_BLACKLIST_CODEC_CNTS];

    int codec_extradata_size;
    /* some codec extra data need more memory,
     * so we directly use the stream data
     */
    unsigned char *codec_extradata;
    unsigned char is_secure;
    MTAVFilter mtavf;
    /* For synchronization in multithread */
    unsigned int state;
    void *ff_av_pkt;
    FILE *dump_aes_fp;
    /* current played segment pts, us */
    int64_t segment_start_pts;
    int pts_hold_counters;
} FILE_SEQ_AUDIO_T;

typedef struct {
    int codec_id;
    FILE *dump_ses_fp;
} FILE_SEQ_SUBTITLE_T;

typedef struct {
    /* audio parameter set */
    FILE_SEQ_AUDIO_T    audio;
    /* video parameter set */
    FILE_SEQ_VIDEO_T    video;
    /* subtile parameter set */
    FILE_SEQ_SUBTITLE_T subtile;

	FILE_SEQ__BUFFERING_STATUS buffering_stat;
    int need_async_done_msg;
    int vitual_video_es_max_size;
    int vitual_video_es_cur_size;
    int min_ves_buf_size;

    int vitual_audio_es_max_size;
    int vitual_audio_es_cur_size;

    int is_live;
    int task_idle;
    int is_timeshift;
    int is_audio_support;
    FILE_SEQ_CHIPTYPE chip_type;
    FILE_SEQ_STATUS internal_state;

    int audio_track_cnt;
    int ves_buf_size;

    int gop_seg_num;
    int interlaced;

    unsigned long seek_mutex;
    MT_BOOL audio_get_pkt_fail;
    MT_BOOL video_get_pkt_fail;

    sys_mem_debug_t start_mem_info;
    sys_mem_debug_t stop_mem_info;

    int is_drm;
    int av_demuxer_reset;
    MT_BOOL seek_buffer;
    MT_BOOL is_need_extra_audio;
    int pack_magic;
    MT_BOOL work_status;
    MT_BOOL load_state;
    MT_BOOL self_exit;

    unsigned long vo_handle;

    int ff_buffer_size_mode;    // 0 -- less than 10M memory, ffmpeg IO cache configuration 256K, more than 10M , confige 1M

    NETWORK_BUFFERING_INFO net_buffer;
    MT_BOOL video_decoder_over;
    void *ff_dict_opts;
    FILE_PLAYBACK_EXTIO_CONTEXT_T *extio_ctx;
    int playlist_index;
    /* wait buffer sleep time, unit ms */
    s64 push_idle_time;
    s64 media_load_start_time;
    void *transport_desdec_ctx; /* transport stream descramble or decryption context */
    /*
     * descramble or decrypt bytes after transport_scrambling_control callback function.
     * ctx [in] user callback context.
     * info[in] information maybe used by caller, call be NULL.
     * in  [in] input data.
     * out [in] output data.
     * len [in] input data bytes.
     */
    int (*transport_desdec_func) (void *ctx, const void *info,
        const unsigned char *in, unsigned char *out, int len);

    int max_stream_probe_size;       /* bytes */
    int max_stream_analyze_duration; /* unit s(second) */
} PLAYBACK_INTERNAL_T;

/*++++++++++for file seq debug+++++++++++*/
typedef struct FILE_SEQ_DEBUG_S{
    char dmx_name[10];
    char seek;
    char live_mode;
    char pspeed;
    int v_eswl;
    int a_eswl;
    int av_codec_support;
    int v_fps;
    int v_ctype;
    int a_ctype;
    int a_sample_rate;
    int a_channel;
    int v_es_hdl_case;
    int a_es_hdl_case;
    int v_push_cnt_t;
    int a_push_cnt_t;
    int fill_es_state;
    int v_push_pts;
    int a_push_pts;
}FILE_SEQ_DEBUG_T;
/*------------for file seq debug------------*/
void do_changeAudioTrack(void);
void do_playAtTime(void);

void mutipath_preload_thread(void * p_param);
void adspath_preload_thread(void * p_param);

void check_buffering_thread(void * p_param);

MT_BOOL  x_check_av_codec_type(void * p_handle);
MT_BOOL  x_get_av_dec_cap(void * pHandle);

void x_unmap_es_buffer(void * pHandle);

int x_start_av_decoder(void * pHandle);
void x_init_av_device(void * pHandle);
void x_stop_av_decoder(void * pHandle);

void fill_es_task(void * p_param);
void fill_aud_es_task(void *p_param);

int  file_seq_handle_subtitle_packet(FILE_SEQ_T *p_file_seq);
void file_seq_check_trickplay(void *p_handle);
void *file_seq_mem_alloc(void * p_handle, u32 size);
void file_seq_mem_free(void * p_handle, void * p_buf);
int  file_seq_av_decoder_init(void *p_handle);

MT_BOOL isEsWaterLevelEnough(int sec);
void handle_audio_es(FILE_SEQ_T * p_file_seq);
void handle_video_es(FILE_SEQ_T * p_file_seq);
MT_BOOL isNetworkStream(void * p_handle);
int do_user_cmd(void);
void handle_exit_fill_es_task(FILE_SEQ_T * p_file_seq);
void handle_subtitle_data(FILE_SEQ_T * p_file_seq);
void handle_audio_es_buffering(FILE_SEQ_T * p_file_seq);
MT_BOOL set_demux_media_info(void * p_handle);

void setup_audio_pts(FILE_SEQ_T * p_file_seq,int pkt_size);
MT_BOOL adec_push_audio_es(u8 * start_a, int in_size_a, u64 apts, u32 eos);
int push_audio_es_packet(FILE_SEQ_T * p_file_seq);
int get_audio_es_packet(FILE_SEQ_T * p_file_seq,u32* audio_pts);

int get_video_es_packet(FILE_SEQ_T *p_file_seq, u32 *orig_vpts);
int get_pack_magic_num(FILE_SEQ_T * p_file_seq,int pkt_size, unsigned int orig_vpts);

RET_CODE player_vdec_start(FILE_SEQ_T *p_file_seq, int format, int mode);
void player_vdec_trick_mode(
    FILE_SEQ_T *p_file_seq, int curr_speed, int next_speed);

int set_audio_param_to_vsb(void);

/** For event */
void handle_pending_event(void);
void notify_video_pts(s64 vpts);
int  mt_run_memory(int isflushCache,char *p_cmd_line,
    int *pMemAvailable, int *pMemTotal, int *pTotalFree, int *pMemNowFree, int *pNCached);
/** For command */
void x_reset_cmd_fifo(FILE_SEQ_T *p_file_seq);
void x_clear_cmd_fifo(void * hdl);
int x_push_user_cmd(void * hdl, FP_USER_CMD_T * p_cmd);

int fpi_codec_in_blacklist(int codec,
    const int invalid_data, int *black_list);
void fpi_init_codec_blacklist(PLAYBACK_INTERNAL_T *fpi);
int fpi_set_codec_blacklist(PLAYBACK_INTERNAL_T *fpi,
    unsigned int stream_type, int num, int *list);
int fpi_change_playlist(FILE_SEQ_T *p_file_seq, int need_change);
int fpi_query_idle_time(FILE_SEQ_T *p_file_seq, s64 *idle_time);
int fpi_get_playlist(FILE_SEQ_T *p_file_seq);
int fpi_switch_playlist(FILE_SEQ_T *p_file_seq, int playlist_index);

void fpi_register_avplay_event(FILE_SEQ_T *p_file_seq);
void fpi_unregister_avplay_event(FILE_SEQ_T *p_file_seq);

int fpi_get_ds_audio_packet(FILE_SEQ_T *p_file_seq,
    void *ds, unsigned char **start, uint8_t **extra_buf, uint8_t *extra_size);
int fpi_get_ds_video_packet(FILE_SEQ_T *p_file_seq,
    void *ds_v, unsigned char **start, int8_t speed);
int file_seq_get_reset_state(void);
int file_seq_set_reset_state(int state);
int file_seq_switch_reset(void);


static inline int fp_is_fast_forward(int speed)
{
    return (speed >= TS_SEQ_FAST_PLAY_2X &&
        speed <= TS_SEQ_FAST_PLAY_32X);
}

static inline int fp_is_fast_backward(int speed)
{
   return (speed >= TS_SEQ_REV_FAST_PLAY_2X &&
        speed <= TS_SEQ_REV_FAST_PLAY_32X);
}

static inline int fp_is_trick_play(int speed)
{
    return fp_is_fast_forward(speed) || fp_is_fast_backward(speed);
}

static inline int fp_is_normal_play(int speed)
{
    return (TS_SEQ_NORMAL_PLAY == speed);
}

static inline int fp_is_fast2x_to_normal(
    int curr_speed, int next_speed)
{
    return (TS_SEQ_FAST_PLAY_2X == curr_speed &&
        TS_SEQ_NORMAL_PLAY == next_speed);
}

static inline int fp_is_normal_to_fast2x(
    int curr_speed, int next_speed)
{
    return (TS_SEQ_NORMAL_PLAY == curr_speed &&
        TS_SEQ_FAST_PLAY_2X == next_speed);
}

static void inline fp_enable_stream_file_header(FILE_SEQ_T *p_file_seq)
{
    PLAYBACK_INTERNAL_T *pbi   = (PLAYBACK_INTERNAL_T *)(p_file_seq->pb_internal);
    FILE_SEQ_VIDEO_T    *vinfo = &(pbi->video);
    if (MT_UNF_VCODEC_TYPE_VP8 == p_file_seq->m_video_codec_type ||
        MT_UNF_VCODEC_TYPE_VP9 == p_file_seq->m_video_codec_type) {
        vinfo->need_insert_header = 1;
    }
}

int x_pop_user_cmd(void *hdl,  FP_USER_CMD_T *p_out_cmd);
int is_audio_type_support(void);
int fp_is_support_audio_codec(
    FILE_SEQ_T *p_file_seq, int audio_codec);

#endif
