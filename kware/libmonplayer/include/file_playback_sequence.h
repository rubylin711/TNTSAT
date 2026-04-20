/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
@file file_playback_sequence.h
@brief Main stuctures and apis of file playback
@details Define the main structures and APIs of file playback
*/

#ifndef  __FILE_PALYBACK_SEQ_H_
#define  __FILE_PALYBACK_SEQ_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "mt_type.h"
#define SUBT_BUF_SIZE_MAX 32*1024//tizhang@20180822 for 104045
/*!
@brief max input path number
@details define the number of multi input paths
*/
#define  MAX_PATH_NUM      (128)
#define  FP_USE_SYS_MEM_DBG (0)
/*!
  @brief file playback status
  */
typedef enum {
    /*!
      invalid status
      */
    FILE_SEQ_UNKNOWN,

    /*!
      stop status
      */
    FILE_SEQ_STOP,
    /*!
      play status
      */
    FILE_SEQ_PLAY,
    /*!
    has played status
    */
    FILE_SEQ_PLAYED,
    /*!
      pause status
      */
    FILE_SEQ_PAUSE,
    /*!
    load status
    */
    FILE_SEQ_LOADMEDIA,
    /*!
    has loaded status
    */
    FILE_SEQ_LOADED,
    /*!
     resume status
      */
    FILE_SEQ_RESUME,
    /*!
      preview status
      */
    FILE_SEQ_PREVIEW,
    /*!
    exit status
    */
    FILE_SEQ_EXIT,

    /*!
    invalid status
    */
    FILE_SEQ_STATUS_MAX,
} FILE_SEQ_STATUS;

typedef enum {
    STREAM_TYPE_NULL = 0x0,
    STREAM_TYPE_AUD,
    STREAM_TYPE_VID,
    STREAM_TYPE_SUB,
    STREAM_TYPE_RAW,
    STREAM_TYPE_BUTT
} STREAM_TYPE_E;

/*!
@brief internal commands for playback
*/
typedef struct {
    /*!
    cmd type
    */
    int  type;
    /*!
    pararm for cmd
    */
    u32 param;

} FP_USER_CMD_T;

/*!
  @brief media info
  */
typedef struct {
    /*!
      audio track num
      */
    int totalAudioTrack;
    /*!
      video track num
      */
    int totalVideoTrack;
    /*!
      audio type string
      */
    unsigned char **pp_audio_type;
    /*!
      video type string
      */
    unsigned char **pp_video_type;
    /*!
      play duration
      */
    unsigned long long  totalDuration;

} MEDIA_INFO;
/*!
  @brief user cmd type
  */
typedef enum {
    /*!
      stop
      */
    STOP_PLAYBACK = 0,
    /*!
      pause
      */
    PAUSE_PLAYBACK = 1,
    /*!
      resume
      */
    RESUME_PLAYBACK = 2,
    /*!
      fastforward 2 speed
      */
    TRICK_PLAY_FF2X = 3,
    /*!
      fastforward 4 speed
      */
    TRICK_PLAY_FF4X = 4,
    /*!
      fastforward 8 speed
      */
    TRICK_PLAY_FF8X = 5,
    /*!
      fastforward 16 speed
      */
    TRICK_PALY_FF16X = 6,
    /*!
      fastforward 32 speed
      */
    TRICK_PLAY_FF32X = 7,
    /*!
       fastforward 64 speed
      */
    TRICK_PLAY_FF64X = 8,
    /*!
      fastbackward 2 speed
      */
    TRICK_PLAY_FR2X = 9,
    /*!
      fastbackward 4 speed
      */
    TRICK_PLAY_FR4X = 10,
    /*!
      fastbackward 8 speed
      */
    TRICK_PLAY_FR8X = 11,
    /*!
          fastbackward 16 speed
      */
    TRICK_PLAY_FR16X = 12,
    /*!
          fastbackward 32 speed
      */
    TRICK_PLAY_FR32X = 13,
    /*!
      invalid cmd
      */
    INVALID_USER_CMD,

} USER_CMD_TYPE;
/*!
  @brief capacitiy of av es buffer
  */
typedef  struct {
    /*!
      max ves num of vdec
      */
    unsigned int   max_ves_num;
    /*!
       max aes num of adec
      */
    unsigned int   max_aes_num;

} AV_DEC_CAP;
/*!
  @brief film info
  */
typedef struct {
    /*!
      film duration
      */
    u32   film_duration;
    /*!
      audio type
      */
    int    audio_type;
    /*!
      video type
      */
    int    video_type;
    /*!
      track num of audio
      */
    int    audio_track_num;
    /*!
       track num of video
      */
    int    video_track_num;
    /*!
      whether support trick play
      */
    MT_BOOL  canTrickPlay;
    /*!
      width of display
      */
    int video_disp_w;
    /*!
       height of display
    */
    int video_disp_h;
    /*!
    video fps
    */
    int video_fps;
    /*!
    video bps
    */
    int  video_bps;//bytes per secondes
    /*!
      size of file
      */
    u64 file_size;
    /*!
    track id of audio
    */
    int audio_track_id;
    /*!
    audio language
    */
    char *audio_language;  // pointers to a modification forbidden memory space
    /*!
    audio bps
    */
    int audio_bps;
    /*!
    samplerate of audio
    */
    int audio_samplerate;
    /*!
    file name
    */
    char *file_name;  // pointers to a modification forbidden memory space
} FILM_INFO_T;
/*!
  @brief video display info
  */
typedef struct {
    /*!
       width of display
      */
    int video_disp_w;
    /*!
       height of display
      */
    int video_disp_h;
    /*!
      video fps
      */
    int video_fps;

} VIDEO_W_H_FPS;

/*!
  @brief track language info
  */
typedef struct {
    /*!
      track id
      */
    int track_id;
    /*!
      language
      */
    char *lang;
    /*!
      title
      */
    char *title;
    /*!
      format in mp
      */
    unsigned int format;
} TRACK_LANG;

typedef enum {
    FILE_SEQ_PLAYLIST_FLAGS_NONE   = 0x00,
    FILE_SEQ_PLAYLIST_FLAGS_ENABLE = 0x01,  /**< Is it enabled, 1:enable */
} FILE_SEQ_PLAYLIST_FLAGS_E;

/*!
  @brief playlist info
  */
typedef struct  {
    int index;
    int width;
    int height;
    int bandwidth;
    struct {
        int num; ///< Numerator
        int den; ///< Denominator
    } framerate;
    /* codec_tag is a "four" character codec identifier for a compression format,
     * A character in this context is a 1 byte/8 bit value,
     * The four characters is generally limited to be within the human readable characters in the ASCII table.
     * Such as "H264", "H264" and so on.
     */
    unsigned int codec_tag;
    FILE_SEQ_PLAYLIST_FLAGS_E flags;
} FILE_SEQ_ADAPTIVE_PLAYLIST_T;

/*!
  @brief adaptive playlist information
  */
typedef struct {
    int num;      /* total number of playlist */
    int curr_idx; /* current played playlist index */
    FILE_SEQ_ADAPTIVE_PLAYLIST_T *list;
} FILE_PLAYBACK_ADAPTIVE_PLAYLIST_T;

/*
 * Network parameter
 *
*/
typedef struct FILE_PLAYBACK_NETWORK_PARA {
    char *headers;    /* set custom HTTP headers, can override built in default headers */
    char *user_agent; /* override User-Agent header */
    char *cookies;    /* holds newline (\n) delimited Set-Cookie header field values (without the "Set-Cookie: " field name) */
} FILE_PLAYBACK_NETWORK_PARA_T;

/** Callback for checking whether to abort blocking functions.
  * During blocking operations, callback is called with void *callback_ctx as parameter.
  * If the callback returns non-zero value, the blocking operation will be aborted.
  */
typedef struct FILE_PLAYBACK_USER_INTERRUPT_CB {
    int (*callback)(void*);
    void *callback_ctx;
} FILE_PLAYBACK_USER_INTERRUPT_CB_T;

/*
 * External user defined i/o context
 *
 * Set NULL if callback functions not available
*/
typedef struct FILE_PLAYBACK_EXTIO_CONTEXT {
    /**
     * External user defined i/o context handle, passed to the read/seek/... functions..
     */
    void *opaque;
    /**
     * Initialize user defined input/output context
     *
     * @param opaque     A private user io handle pointer, passed to the read/seek/... functions..
     * @return >=0 on success or negative on error.
     */
    int (*init) (void *opaque);
    /**
     * Close the resource accessed by user defined input/output context and free it.
     *
     * @param opaque     A private user io handle pointer
     * @return >=0 on success or negative on error.
     */
    int (*deinit) (void *opaque);
    /**
     * Read size bytes from opaque context into buf.
     *
     * @param opaque     A private user io handle pointer
     * @param buf        buffer for keeping data
     * @param buf_size   buffer size
     *
     * @return number of bytes read on success or negative on error.
     */
    int (*read) (void *opaque, unsigned char *buf, int buf_size);
    /**
     * Seek to a given byte position in stream with the specified whence
     * as fseek defined in header <stdio.h>:SEEK_CUR/SEEK_SET/SEEK_END
     *
     * @param opaque     A private user io handle pointer
     * @param offset     A specified byte position in stream
     * @param whence     position to which offset is added.
     *                   It can have one of the following values: SEEK_SET, SEEK_CUR, SEEK_END
     *                      SEEK_SET:argument to seek indicating seeking from beginning of the file
     *                      SEEK_CUR:argument to seek indicating seeking from the current file position
     *                      SEEK_END:argument to seek indicating seeking from end of the file
     *
     * @return new position or negative on error.
     */
    long long (*seek) (void *opaque, long long offset, int whence);
    /**
     * Checks if the end of the given stream has been reached.
     *
     * @param opaque     A private user io handle pointer
     *
     * @return non zero if data EOF, otherwise 0
     */
    int (*eof) (void *opaque);
    /**
     * Get the filesize of the given stream.
     *
     * @param opaque     A private user io handle pointer
     *
     * @return the filesize upon success, or negative on error.
     */
    long long (*get_size) (void *opaque);
    /**
     * Obtains the file position for the file stream and stores them in the object pointed to by pos.
     *
     * @param opaque     A private user io handle pointer
     * @param pos        Pointer to a long long object to store the file position indicator to
     *
     * return 0 upon success, nonzero value otherwise.
     */
    int (*get_pos) (void *opaque, long long *pos);
    /**
     * Checks the given stream seekable or not.
     *
     * @param opaque     A private user io handle pointer
     *
     * @return non zero if seekable, otherwise 0
     */
    int (*get_seekable) (void *opaque);
    /**
     * Get the file type of the given stream.
     *
     * @param opaque     A private user io handle pointer
     *
     * @return media type on success or negative on error.
     */
    int (*get_media_type) (void *opaque);
} FILE_PLAYBACK_EXTIO_CONTEXT_T;

/*!
  @brief define the type of event delivered to app
  */
typedef enum {
    /*!
      when play to the end of the file, send this event
       app may look at this event as 'EOF'
      */
    FILE_PLAYBACK_SEQ_STOP = 0,
    /*!
    fail to play file and 'fill_es' task exit exceptionally, send this event
    app may look at this event as 'EOF'
    */
    FILE_PLAYBACK_SEQ_ABNORMAL_STOP = 0,
    /*!
    display info has updated, such as high and width of picture
    */
    FILE_PLAYBACK_SEQ_RECEIVE_VIDEO_INFO,
    /*!
       new video pts arrive
      */
    FILE_PLAYBACK_SEQ_GET_VPTS,
    /*!
      fail to excute the action 'loadmedia'
    */
    FILE_PLAYBACK_SEQ_LOAD_MEDIA_EXIT,
    /*!
       no use (reserved for future !!!!!)
     */
    FILE_PLAYBACK_SEQ_LOAD_MEDIA_ERROR,
    /*!
     success to loadmedia
    */
    FILE_PLAYBACK_SEQ_LOAD_MEDIA_SUCCESS,
    /*!
     success to load advertisement file
    */
    FILE_PLAYBACK_SEQ_LOAD_ADSFILE_SUCCESS,
    /*!
     success to load expectation media
    */
    FILE_PLAYBACK_SEQ_LOAD_EXPFILE_ERROR,
    /*!
     reset vpts after playing advertisement
    */
    FILE_PLAYBACK_SEQ_ADSFILE_RESET_VPTS,
    /*!
     * dash and hls contains more playlists sometimes,
     * which will occupy more initial time and postpone play,
     * so we parse one playlist for every a/v/s stream before play,
     * after that, we send async open message and parse other playlists continually.
     * we send async done after parse.
     * so, if you get async open message, you should get the file inforamtion after async done.
     */
    FILE_PLAYBACK_SEQ_ASYNC_OPEN,
    /*!
     * see aync open note
    */
    FILE_PLAYBACK_SEQ_ASYNC_DONE,
    /*!
    no use (reserved for future !!!!!)
    */
    FILE_PLAYBACK_SEQ_NOT_READY,
    /*!
      player can't support this type of audio codec
     */
    FILE_PLAYBACK_UNSUPPORT_AUDIO,
    /*!
     player  can't support this type of video codec
     */
    FILE_PLAYBACK_UNSUPPORT_VIDEO,
    /*!
      whether player is in trick play mode
     */
    FILE_PLAYBACK_CHECK_TRICKPLAY,
    /*!
     no enough memory for decoding such type of video es
     */
    FILE_PLAYBACK_UNSUPPORT_MEMORY,
    /*!
      new subtitle data has arrived
     */
    FILE_PLAYBACK_NEW_SUB_DATA_RECEIVE,
    /*!
     can't support the feature 'seek'
     */
    FILE_PLAYBACK_UNSUPPORT_SEEK,
    /*!
     can't support the feature 'trick play'
     */
    FILE_PLAYBACK_UNSUPPORT_TRICK,
    /*!
      the thread 'fill es' exit normally
     */
    FILE_PLAYBACK_SEQ_FILL_ES_TASK_EXIT,
    /*!
     no use (reserved for future !!!!!)
    */
    FILE_PLAYBACK_SEQ_ES_TASK_DEAD,

    /*!
      pause av decoder and continue to buffer data
     */
    FILE_PLAYBACK_SEQ_START_BUFFERING,
    /*!
    seek finished
    */
    FILE_PLAYBACK_SEEK_FINISHED,
    /*!
    resume av decoder
    */
    FILE_PLAYBACK_SEQ_FINISH_BUFFERING,
    /*!
    current download speed
    */
    FILE_PLAYBACK_SEQ_UPDATE_BPS,
    /*!
    * es water level is too low and notify up layer change source
    */
    FILE_PLAYBACK_SEQ_REQUEST_CHANGE_SRC,

    /*!
    * fail to set current path so app can try to set this path again
    */
    FILE_PLAYBACK_SEQ_SET_PATH_FAIL,

    /*!
    * update total seconds for loading media
    */
    FILE_PLAYBACK_SEQ_LOAD_MEDIA_TIME,

    /*!
    * notify upper layer that player has already finish updating bps
    */
    FILE_PLAYBACK_FINISH_UPDATE_BPS,
    /*!
    * notify upper layer that video decoder unsupport current hd video
    */
    FILE_PLAYBACK_HD_UNSUPPORT,
    /*!
      * notify Supper layer that  switch audio spdif out mode
      */
    FILE_PLAYBACK_SWITCH_AUDIO_SPDIF_MODE,
    /*!
    * Enter to trick mode state, user should mute the audio when this message received.
    */
    FILE_PLAYBACK_TRICKMODE_ENTER,

    /*!
    * Leave from trick mode state, user should recover to previously mute state.
    */
    FILE_PLAYBACK_TRICKMODE_LEAVE,
    /*!
    * notify eof.
    */
    FILE_PLAYBACK_PLAY_EOF,
    /*!
    * notify new video frame.
    */
    FILE_PLAYBACK_PLAY_NEW_VID_FRAME,
    /*!
      invalid type
      */
    FILE_PLAYBACK_SEQ_INVALID,
    /*!
      NOT ENOUGH MEMORY
      */
    FILE_PLAYBACK_NOT_ENOUGH_MEMORY,
    /*DRM decrypt failed*/
    FILE_PLAYBACK_DRM_DECRYPT_FAIL,
    /* server error exit */
    FILE_PLAYBACK_LIVE_SERVER_ERROR,
} FILE_PLAY_EVENT_E;
/*!
@brief stream type of video es
*/
typedef enum {
    /*!
      unkown type
      */
    vUNKNOWN        = -1,
    /*!
      mpeg1 video
      */
    vVIDEO_MPEG1    = 0,
    /*!
      mpeg2 video
      */
    vVIDEO_MPEG2,
    /*!
      mpeg4 video
      */
    vVIDEO_MPEG4,
    /*!
      h264 video
      */
    vVIDEO_H264,
    /*!
      avc video
      */
    vVIDEO_AVC,
    /*!
      hevc video
      */
    vVIDEO_HEVC,
    /*!
      dirac video
      */
    vVIDEO_DIRAC,
    /*!
      vc1 video, Advanced Profile
      */
    vVIDEO_VC1,
    /*!
      vc1 video, smple and main profile
    */
    vVIDEO_VC1_WMV3,
    /*!
      vp8 video
      */
    vVIDEO_VP8,
    /*!
      rv30 video
      */
    vVIDEO_RV30,
    /*!
      rv40 video
      */
    vVIDEO_RV40,
    /*!
      avs video
      */
    vVIDEO_AVS,
    /*!
      motion jpeg video
      */
    vVIDEO_MJPEG,
    /*!
      vp9 video
      */
    vVIDEO_VP9,
    /*!
    VC1SMP5
    */
    vVIDEO_VC1SMP5,
    vVIDEO_TYPE_END,
} ves_stream_type_t;
/*!
@brief stream type of audio es
*/
typedef enum {
    /*!
      unkonwn audio
      */
    aUNKNOWN        = -1,
    /*!
      mp2 audio
      */
    aAUDIO_MP2 = 0,
    /*!
      mp3 audio
    */
    aAUDIO_MP3,
    /*!
      ac3 audio
      */
    aAUDIO_AC3,
    /*!
      ac4 audio
      */
    aAUDIO_AC4,
    /*!
      audio vivid (av3a audio)
      */
    aAUDIO_AV3A,
    /*!
      dts audio
      */
    aAUDIO_DTS,
    /*!
      be lpcm audio
      */
    aAUDIO_LPCM_BE,
    /*!
      aac audio
      */
    aAUDIO_AAC,
    /*!
      latm aac audio
      */
    aAUDIO_AAC_LATM,
    /*!
      truehd audio
      */
    aAUDIO_TRUEHD,
    /*!
      s302m audio
      */
    aAUDIO_S302M,
    /*!
      br pcm audio
      */
    aAUDIO_PCM_BR,
    /*!
      adpcm audio
      */
    aAUDIO_ADPCM,
    /*!
      xxxxxxxx
      */
    aAUDIO_WMA,

    aAUDIO_FLAC,
    aAUDIO_VORBIS,
    aAUDIO_TYPE_END,
} aes_stream_type_t;

/*!
  @brief define memory config for firmware
  */
typedef struct {
    /*!
      point to start address of ves buffer
      */
    unsigned long      p_vdec_start;
    /*!
       size of ves buffer
      */
    unsigned int      vdec_mem_size;
    /*!
      point to start address of aes buffer
      */
    unsigned  long     p_audio_start;
    /*!
       size of aes buffer
      */
    unsigned int      audio_mem_size;

} FW_MEM_CFG_T;

/*
  FUNCTION:      the function pointer of load media
  PARAM1:         handle to file_playback_seq
  */
/*!
@brief  The function pointer of load media function
@param[in] 1 The handle to file_playback_seq
@return Return the load result
0: success -1:failed
*/
typedef  int (* file_seq_loadmedia_fun_t)(void *);
/*
  FUNCTION:      the function pointer to load media  multi-times
  PARAM1:         handle to file_playback_seq,PARAM2 refer to load media times
  */
/*!
@brief  the function pointer of load media muti-times
@param[in] 1 The handle to file_playback_seq
@param[in] 2 The load media times
@return Return the load result
0: success -1:failed
*/
typedef  int (* file_seq_loadmedia_times_fun_t)(void *, int);
/*
  FUNCTION:      unload media function
  PARAM1:         handle to file_playback_seq
  */
/*!
@brief  The function pointer of unload media function
@param[in] 1 The handle to file_playback_seq
@return void
*/
typedef  void (* file_seq_unloadmedia_fun_t)(void *);
/*
  * FUNCTION:         start to play
  * PARAM:              PARAM1 refer to handle, PARAM2 refer to play position (seconds)
  * RETURN VALUE:   0 means ok; other means fail
  */
/*!
@brief  the function pointer of start
@param[in] 1 The handle to file_playback_seq
@param[in] 2 start position(seconds)
@return Return the  result
0: success others:failed
*/
typedef  int (* file_seq_start_fun_t)(void *, unsigned int);
/*
  FUNCTION:      require player to stop
  PARAM1:         handle to file_playback_seq
  NOTICE:          stop will be excuted in asynchronous mode

  */
/*!
@brief  the function pointer of stop
@details require player to stop, stop will be excuted in asynchronous mode
@param[in] 1 The handle to file_playback_seq
@return Return the  result
0: success others:failed
*/
typedef  int (* file_seq_stop_fun_t)(void *);
/*
  FUNCTION:      require player to pause
  PARAM1:         handle to file_playback_seq
  */
/*!
@brief  the function pointer of pause
@details require player to pause
@param[in] 1 The handle to file_playback_seq
@return Return the  result
0: success others:failed
*/
typedef  int (* file_seq_pause_fun_t)(void *);
/*
  FUNCTION:      require player to resume
  PARAM1:         handle to file_playback_seq
  */
/*!
@brief  the function pointer of resume
@details require player to resume
@param[in] 1 The handle to file_playback_seq
@return Return the  result
0: success others:failed
*/
typedef  int (* file_seq_resume_fun_t)(void *);
/*
  FUNCTION:      require player to set speed
  PARAM1:         handle to file_playback_seq,,PARAM2 refer to speed index
  */
/*!
@brief  the function pointer of set play speed
@details require player to set speed
@param[in] 1 The handle to file_playback_seq
@param[in] 2 Refer to speed index
@return Return the  result
0: success others:failed
*/
typedef  int (* file_seq_set_speed_fun_t)(void *, int);
/*
  FUNCTION:      wait trick play finish.
  PARAM1:         handle to file_playback_seq, PARAM2 timeout ms.
  */
/*!
@brief  wait seek finish.
@details wait seek finish.
@param[in] 1 The handle to file_playback_seq
@param[in] 2 wait timeout ms.
@return void
*/
typedef  void (*file_seq_wait_tplay_finish)(void *, int);
/*
  FUNCTION:      get mp tag.
  PARAM1:         handle to file_playback_seq, PARAM2 tag key, PARAM3 tag value, PARAM4 tag value len.
  */
/*!
@brief  get mp tag.
@details get mp tag.
@param[in] 1 The handle to file_playback_seq
@param[in] 2 tag key.
@param[out] 3 tag value.
@param[in] 4 tag value len.
@return get result.
true: get tag success, false: get tag failure.
*/
typedef MT_BOOL (*file_seq_get_mp_tag)(void *, char *, char *, int);
/*
  FUNCTION:      support seek finish.
  PARAM1:         handle to file_playback_seq
  */
/*!
@brief  support seek finish.
@details support seek finish.
@param[in] 1 The handle to file_playback_seq
@return void
*/
typedef void (*file_seq_set_support_seek_finish)(void *);
/*
   FUNCTION:      require player to preview
   PARAM1:         handle to file_playback_seq
  */
/*!
@brief  the function pointer of preview
@details require player to preview
@param[in] 1 The handle to file_playback_seq
@return Return the  result
0: success others:failed
*/
typedef  int (* file_seq_preview_fun_t)(void *);
/*
   FUNCTION:      require player to get video pts
   PARAM1:         handle to file_playback_seq
  */
/*!
@brief  the function pointer of get video pts
@details require player to get video pts
@param[in] 1 The handle to file_playback_seq
@return Return the pts
*/
typedef  unsigned long int (* file_seq_get_vpts_fun_t)(void *);
/*
    FUNCTION:      require player to play at the given time
    PARAM1:         handle to file_playback_seq,PARAM2 refer to the given time(second)
  */
/*!
@brief  the function pointer of play at time
@param[in] 1 The handle to file_playback_seq
@param[in] 2 start positon(seconds)
@return Return the  result
0: success others:failed
*/
typedef  int (* file_seq_play_at_time_fun_t)(void *, int);
/*
   FUNCTION:      require player to get status
   PARAM1:         handle to file_playback_seq
  */
/*!
@brief  the function pointer of getting play status
@param[in] 1 The handle to file_playback_seq
@return Return the  playing status
*/
typedef  FILE_SEQ_STATUS(* file_seq_get_status_fun_t)(void *);
/*
    FUNCTION:      require player to change the audio track
    PARAM1:         handle to file_playback_seq,PARAM2 refer to the track id
  */
/*!
@brief  the function pointer of change audio track
@details require player to change the audio track
@param[in] 1 The handle to file_playback_seq
@param[in] 2 The track id, id=0,1,...
@return Return the  result
0: success others:failed
*/
typedef  int (* file_seq_change_audio_track_fun_t)(void *, int);
/*
    FUNCTION:      require player to get  the audio track langugae
    PARAM1:         handle to file_playback_seq,PARAM2 refer to languge struct pointer
  */
/*!
@brief  the function pointer of get audio language
@details require player to get  the audio track language
@param[in] 1 The handle to file_playback_seq
@param[in] 2  Refer to languge struct pointer
@return Return the  result
0: success others:failed
*/
typedef  int (* file_seq_get_audio_track_lang_fun_t)(void *, TRACK_LANG *);
/*
    FUNCTION:      require player to change the video track
    PARAM1:         handle to file_playback_seq,PARAM2 refer to the track id
  */
/*!
@brief  the function pointer of change video track
@details require player to change the video track
@param[in] 1 The handle to file_playback_seq
@param[in] 2  Refer to the track id
@return Return the  result
0: success others:failed
*/
typedef  int (* file_seq_change_video_track_fun_t)(void *, int);
/*
   FUNCTION:      require player to get the film info
   PARAM1:         handle to file_playback_seq,PARAM2 refer to film info struct pointer
 */
/*!
@brief  the function pointer of get the film info
@details require player to get the film info
@param[in] 1 The handle to file_playback_seq
@param[in] 2 Refer to film info struct pointer
@return Return the  result
1: success 0:failed
*/
typedef  MT_BOOL (* file_seq_get_film_info_fun_t)(void *, FILM_INFO_T *);
/*
   FUNCTION:      require player to set the play path
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the play path
  */
/*!
@brief  the function pointer of set file path
@details require player to set the play path
@param[in] 1 The handle to file_playback_seq
@param[in] 2 Refer to the play path
@return void
*/
typedef  void (* file_seq_set_file_path_fun_t)(void *, char *);
/*
   FUNCTION:      require player to set the play url
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the play path,PARAM3 refer to the path unique id
  */
/*!
@brief  the function pointer of set file pathes
@details require player to set the play pathes
@param[in] 1 The handle to file_playback_seq
@param[in] 2 Refer to the play path
@param[in] 3 Refer to the path unique id
@return void
*/
typedef  void (* file_seq_set_file_path_ex_fun_t)(void *, char *, u32);
/*
   FUNCTION:      require player to set the play paths
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the play paths,PARAM3 refer to the path unique id
  */
/*!
@brief  the function pointer of set muti file pathes
@details require player to set the play paths
@param[in] 1 The handle to file_playback_seq
@param[in] 2 Refer to the play pathes
@param[in] 3 Refer to the path unique id
@return void
*/
typedef  void (* file_seq_set_muti_file_path_fun_t)(void *, char *[], u32);
/*
   FUNCTION:      require player to set the ads play path
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the  ads play path,
                         PARAM3 refer to the path unique id
  */
/*!
@brief  the function pointer of set the ads play path
@details require player to set the ads play path
@param[in] 1 The handle to file_playback_seq
@param[in] 2 Refer to the  ads play path
@param[in] 3 Refer to the path unique id
@return void
*/
typedef  void (* file_seq_set_ads_file_path_fun_t)(void *, char *, u32);
/*
   FUNCTION:      require player to get av codec type
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the video type,PARAM3 refer to the audio type
  */
/*!
@brief  the function pointer of get av codec type
@details require player to get av codec type
@param[in] 1 The handle to file_playback_seq
@param[in] 2 refer to the video type
@param[in] 3 refer to the audio type
@return void
*/
typedef  void (* file_seq_av_codec_type_fun_t)(void *, int, int);
/*
   FUNCTION:      require player to do cmd
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the cmd type
  */
/*!
@brief  the function pointer of do cmd
@details require player to do cmd
@param[in] 1 The handle to file_playback_seq
@param[in] 2 refer to the cmd type
@return void
*/
typedef  void (* file_seq_do_cmd_fun_t)(void *, int);
/*
   FUNCTION:      require player to dump es data
   PARAM1:         handle to file_playback_seq
  */
/*!
@brief  the function pointer of dump es data
@details require player to dump es data
@param[in] void
@return void
*/
typedef  void (* file_seq_mp_dump_fun_t)(void);
/*
   FUNCTION:      require player to run memory dectecter
   PARAM1:         is not run mem dectecter
  */
/*!
@brief  the function pointer of run memory dectector
@details require player to run memory dectector for checking memory leak
@param[in] 1 is not run mem dectector
@return void
*/
typedef  int (* file_seq_run_mem_fun_t)(MT_BOOL);
/*
   FUNCTION:      require player to init memory
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the memory address,PARAM3 refer to the memory size
  */
/*!
@brief  the function pointer of setting memory heap for demuxer
@details set memory heap for internal demuxer
@param[in] 1 The handle to file_playback_seq
@param[in] 2 refer to the start address of heap
@param[in] 3 refer to the  size of heap
@return void
*/
typedef  void (*file_seq_mem_init_fun_t)(void *, u8 *, u32);
/*
   FUNCTION:      require player to allocate  memory
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the memory size
  */
/*!
@brief  the function pointer of memory alloc
@details  allocate memory resource from internal heap memory
@param[in] 1 The handle to file_playback_seq
@param[in] 2 Refer to the size of allocation
@return void
*/
typedef  void                         *(*file_seq_mem_alloc_fun_t)(void *, u32);
/*
   FUNCTION:      require player to free memory
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the memory address
  */
/*!
@brief  the function pointer of memory free
@details free internal heap memory resource
@param[in] 1 The handle to file_playback_seq
@param[in] 2 Refer to the memory chunk to be reclaimed
@return  void
*/
typedef  void (*file_seq_mem_free_fun_t)(void *, void *);
/*
   FUNCTION:      require player to  release memory
   PARAM1:         handle to file_playback_seq
  */
/*!
@brief  the function pointer of release the whole heap memory
@details require player to release internal heap memory
@param[in] 1 The handle to file_playback_seq
@return void
*/
typedef void (*file_seq_mem_release_fun_t)(void *);
/*
   FUNCTION:      require player to  get subtitle info
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the handle of subtitle info
  */
/*!
@brief  the function pointer of getting subtitle info
@details require player to get subtitle info
@param[in] 1 The handle to file_playback_seq
@param[in] 2 refer to the handle of subtitle info
@return void
*/
typedef void (* file_seq_get_subt_info_fun_t)(void *, void **);
/*
   FUNCTION:      require player to  set subtitle id
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the index of subtitle
  */
/*!
@brief  the function pointer of setting subtitle id
@details require player to set relative subtitle
@param[in] 1 The handle to file_playback_seq
@param[in] 2 refer to the index of subtitle
@return void
*/
typedef void (* file_seq_set_subt_id_fun_t)(void *, int);
/*
   FUNCTION:      require player to  get subtitle data
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the subtitle packet,PARAM3 refer to the subtitle pts
  */
/*!
@brief  the function pointer of getting subtitle data
@details require player to get subtitle data
@param[in] 1 The handle to file_playback_seq
@param[in] 2 refer to the subtitle packet
@param[in] 3 refer to the subtitle pts
@return void
*/
typedef  int (*file_seq_get_subt_fun_t)(void *, unsigned char **, int *);
/*
   FUNCTION:      require player to  get the extra subtitle data
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the subtitle packet,PARAM3 refer to the subtitle pts
  */
/*!
@brief  the function pointer of getting the extra subtitle data
@details require player to get the extra subtitle data
@param[in] 1 The handle to file_playback_seq
@param[in] 2 refer to the subtittle packet
@param[in] 3 refer to the subtittle pts
@return Return the subtittle size
*/
typedef  int (*file_seq_get_exsubt_fun_t)(void *, unsigned char **, int *);
/*
   FUNCTION:      callback function about file_seq
   PARAM1:         refer to the callback  type ,PARAM2 refer to the callback input
  */
/*!
@brief  the function pointer of callback function about file_seq
@details callback function about file_seq
@param[in] 1 refer to the callback  type
@param[in] 2 refer to the callback input
@return Return the ret result
*/
typedef RET_CODE(*file_seq_event_cb_fun_t)(FILE_PLAY_EVENT_E, unsigned long);

/*
   FUNCTION:      require player to  register callback function
   PARAM1:         handle to file_playback_seq,PARAM2 refer to the function pointer
  */
/*!
@brief  the function pointer of register callback function
@details require player to  register callback function
@param[in] 1 The handle to file_playback_seq
@param[in] 2 refer to the function pointer
@return Return the ret result
*/
typedef   void (*file_seq_register_event_cb_func_t)(void *, void *);

/*
  FUNCTION:      require player to  set tv sysfmt automatic
  PARAM1:         handle to file_playback_seq,PARAM2 refer to is or not set tv sysfmt automatic
  */
/*!
@brief  the function pointer of set tv system format automatically
@details require player to set tv sysfmt automatically
@param[in] 1 The handle to file_playback_seq
@param[in] 2 refer to flag which means whether to set tv sysfmt automatically
@return void
*/
typedef   void (*file_seq_set_vfmt_fun_t)(void *, MT_BOOL);
/*
  FUNCTION:      require player to  identify whether the stream is based on network
  PARAM1:         handle to file_playback_seq
  */
/*!
@brief  the function pointer of get the current stream type
@details require player to  identify whether the current stream is based on network
@param[in] 1 The handle to file_playback_seq
@return Return stream flag
false:local stream  true:network stream
*/
typedef  MT_BOOL (*isNetworkStream_fun_t)(void *);
/*
  FUNCTION:      require player to  set current stream is or not live broadcast stream
  PARAM1:         handle to file_playback_seq, PARAM2 refet to the flag that is or not live broadcast stream
  */
/*!
@brief  the function pointer of setting the flag of live stream
@details require player to set live stream flag
@param[in] 1 The handle to file_playback_seq
@param[in] 2 Refer to the flag whether the stream is live or not
@return void
*/
typedef  void (*file_seq_set_live_broadcast_fun_t)(void *, MT_BOOL);
/*
  FUNCTION:      require player to check whether the background task is alive
  PARAM1:         handle to file_playback_seq
  */
/*!
@brief  the function pointer of get the background task live status
@details require player to check whether the background task is alive
@param[in] 1 The handle to file_playback_seq
@return Return live flag
true:alive false:dead
*/
typedef  MT_BOOL (*file_seq_check_bg_task_alive)(void *);
/*
  FUNCTION:      require player to  set ott playmode
  PARAM1:         handle to file_playback_seq, PARAM2 refet to the ott play mode
  */
/*!
@brief  the function pointer of set ott playmode
@details require player to set  ott playmode
@param[in] 1 The handle to file_playback_seq
@param[in] 2 Refer to the ott play mode
@return void
*/
typedef  void (*file_seq_set_ott_playmode_t)(void *, int);

/*
  FUNCTION:      wait seek finish.
  PARAM1:         handle to file_playback_seq, PARAM2 timeout ms.
  */
/*!
@brief  wait seek finish.
@details wait seek finish.
@param[in] 1 The handle to file_playback_seq
@param[in] 2 wait timeout ms.
@return void
*/
typedef  void (*file_seq_wait_seek_finish)(void *, int);

/*
  FUNCTION:       Open network buffering function
  PARAM1:         handle to file_playback_seq, PARAM2 MT_TRUE or MT_FALSE.
  */
/*!
@brief  Open network buffering function.
@details Open network buffering function.
@param[in] 1 The handle to file_playback_seq
@param[in] 2 MT_TRUE or MT_FALSE.
@return void
*/
typedef  void (*file_seq_set_network_buffering)(void *, MT_BOOL);
/*!
  @brief subtitle info string lenth
  @details define the length of subtitle info string
  */
#define FILE_SEQ_SUBTITLE_LEN 20
/*!
  @brief subtitle count in one film
  @details define the max count of subtittle in one film
  */
#define FILE_SEQ_SUBTITLE_CNT 20
/*!
    @brief video decoder fetch bytes of ves every time
    @details define the video decoder fetch bytes of ves every time
  */
#define  VES_SEG_LEN   (1024)
/*!
  @brief define subtitle info struct
  */
typedef struct {
    /*!
      language about subtitle
      */
    char lang[FILE_SEQ_SUBTITLE_LEN];
    /*!
    title string
    */
    char title[FILE_SEQ_SUBTITLE_LEN];
    /*!
    codec type
    */
    char code[FILE_SEQ_SUBTITLE_LEN];
    /*!
    subtitle index id
    */
    int    id;
} file_seq_subtitle_t;

/*!
  @brief video subtitle info
  */
typedef struct {
    /*!
    subtitle cnt in film
    */
    int cnt;
    /*!
    subtitle info
    */
    file_seq_subtitle_t subtitle[FILE_SEQ_SUBTITLE_CNT];
} file_seq_video_subtitle_t;

/*!
  @brief local file path length
  @details define length of  local file path
  */
#define  LOCAL_FILE_PATH_LEN  (1024*4)
/*!
  @brief temp audio es buffer length
  @details define the temp audio es buffer length
  */
#define  AUD_TMP_ESBUF_LEN   (70)
/*!
  @brief define subtitle data struct
  @details define subtitle data struct
  */
typedef struct {
    /*!
      size of subtitle packet
      */
    int size;
    /*!
      pts of subtitle packet
      */
    int pts;
    /*!
      data of subtitle packet
      */
    unsigned char *data;

} SUBT_DATA;


/*!
  @brief audio output mode
  */
typedef enum {
    /*!
    lpcm mode
    */
    AUDIO_LPCM = 0,
    /*!
    bypass mode
    */
    AUDIO_BYPASS = 1,
    /*!
    convert DD+ to DD
    */
    AUDIO_CONVERT = 2,

} AUDIO_OUT_MODE;

/*!
  @brief define fileplay parameter struct
  */
typedef struct {
    /*!
      internal heap memory size
      */
    unsigned int   pb_seq_mem_size;
    /*!
    start address of internal heap
    */
    unsigned long   pb_seq_mem_start;
    /*!
      player's audio output mode
      */
    AUDIO_OUT_MODE audio_output_mode;

    /*!
     is or not  direct url
     */
    unsigned char   is_direct_url;

    /*!
    vedio decoder policy
    */
    unsigned int   vdec_policy;


    /*!
        whether player's main task work in idle mode
        */
    int task_idle;

    /*!
      smart http use. 0-schedule task, 1-4 download task
      */
    unsigned int   task_smart_http_priority[5];
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
} PB_SEQ_PARAM_T;

/*!
  @brief preload buffer struct
  */
typedef struct {
    /*!
      buffer start address
      */
    u8 *buffer_start;
    /*!
      buffer length
      */
    u32 buffer_len;
    /*!
      write position in buffer
      */
    u32 write_pos;
    /*!
      read position in buffer
      */
    u32 read_pos;
} preload_buffer_t;

/*!
  @brief es packet data struct
  */
typedef struct es_pkt_data_s {
    /*!
      es packet index
      */
    u32 index;
    /*!
      next es packet prt
      */
    struct es_pkt_data_s *next_pkt;
    /*!
      prev es packet prt
      */
    struct es_pkt_data_s *prev_pkt;
    /*!
      end of file flag
      */
    u32  eof;
    /*!
      pts of es packet
      */
    double pts;
    /*!
       length of es packet
      */
    u32 data_len;
    /*!
      extra data length of es packet
      */
    u32 extra_data_len;
    /*!
      point to es packet
      */
    u8 *data;
    /*!
      point to extra data of es packet
      */
    u8 *extra_data;
} es_pkt_data_t;

/*!
  @brief define ott play mode
  */
typedef enum {
    /*!
      vod mode
      */
    OTT_VOD_MODE = 3,
    /*!
      live mode
      */
    OTT_LIVE_MODE,
    /*!
      invalid mode
      */
    OTT_INVALID_MODE,

} OTT_PLAY_MODE_T;
/*!
  @brief define stream type mode
  */
typedef enum {
    /*!
      live stream
      */
    STREAM_LIVE,

    /*!
      vod stream
      */
    STREAM_VOD,
    /*!
      invalid stream type
      */
    STREAM_TYPE_MAX,

} FILE_STREAM_TYPE;
/*!
  @brief define check  water level of ves buffer
  */
typedef enum {
    /*!
      'check_water_level' task is dead
      */
    CHECK_ES_FALSE        = 0,
    /*!
    start 'check_water_level' task
    */
    CHECK_ES_TRUE,
    /*!
      'check_water_level' task is running
      */
    CHECK_ES_RUNNING,
    /*!
      'check_water_level' task is idle
      */
    CHECK_ES_IDLE,
    /*!
    'check_water_level' task is already exit
    */
    CHECK_ES_EXIT,

} CHECK_ES_STATE;

/*!
     @brief HLS url
     */
typedef struct {
    /*!
      url string
      */
    char url[4096];
} HLS_URL;

/*!
   @brief HLS load status
 */
typedef enum {
    /*!
      current ts is not loaded
      */
    HLS_STATUS_UNLOAD = 0,
    /*!
      current ts need to be loaded
      */
    HLS_STATUS_NEED_TO_LOAD = 1,
    /*!
      load current ts successfully
      */
    HLS_STATUS_LOAD_SUCCESS = 2,
    /*!
    load task is finished
    */
    HLS_STATUS_LOAD_FINISHED = 3,

} HLS_LOAD_STATUS;
/*!
    @brief dash load status
  */

typedef enum {
    /*!
         current stream is not loaded
      */
    dash_STATUS_UNLOAD = 0,
    /*!
        current stream need to be loaded
      */
    dash_STATUS_NEED_TO_LOAD = 1,
    /*!
      load current stream successfully
      */
    dash_STATUS_LOAD_SUCCESS = 2,
    /*!
       load task is finished
    */
    dash_STATUS_LOAD_FINISHED = 3,

} DASH_LOAD_STATUS;

/*!
     @brief HLS parser result
     */
typedef struct {

    /*********************************
    this below is modified by both side
    **********************************/

    /*!
        load status about hls parser
       */
    HLS_LOAD_STATUS ts_load_status;

    /*!
      current seq num
      */
    int cur_seq_num;    //init inside


    /***************************************
    this below is modified only by hls inside
    ***************************************/

    /*!
      hls parser enable flag
      */
    int enable;             //init inside


    /*!
      hls parser is_live flag
      */
    int is_live;
    /*!
      start sequence num
      */
    int start_seq_num;
    /*!
        m3u8 url
       */
    HLS_URL m3u8_ur;
    /*!
      num of url
      */
    int url_num;
    /*!
      ts url
      */
    HLS_URL *ts_url;

} HLS_PARSER_RESULT;
/*!
     @brief dash parser result
     */

typedef struct {

    /*********************************
    this below is modified by both side
    **********************************/

    /*!
       ts load status
       */
    DASH_LOAD_STATUS ts_load_status;

    /*!
      current seq num
      */
    int cur_seq_num;    //init inside


    /***************************************
    this below is modified only by hls inside
    ***************************************/

    /*!
      enable flag
      */
    int enable;             //init inside


    /*!
      is or not live
      */
    int is_live;
    /*!
      start seq num
      */
    int start_seq_num;
    /*!
       m3u8 url
       */
    HLS_URL m3u8_ur;
    /*!
      url num
      */
    int url_num;
    /*!
      ts url
      */
    HLS_URL *ts_url;

} DASH_PARSER_RESULT;
/*!
 @brief define fifo http state
 */
typedef enum {
    /*!
              init mode
          */
    HTTP_INT        = 0,
    /*!
      connect mode
      */
    HTTP_CONNECT,

    /*!
      recv mode
      */
    HTTP_RECV,
    /*!
      stop mode
      */
    HTTP_STOP,
    /*!
      failed mode
      */
    HTTP_FAILED,
    /*!
      reconnect mode
      */
    HTTP_RECONNECT,

} PB_FIFO_HTTP_STATE;

typedef struct file_seq_event {
    u32 event_lock;   /* lock for playback event */
    s64 base_pts;     /* the smallest first pts of all streams */
    s64 pts_notify_time;/* unit: millisecond,ms*/
} file_seq_event_t;

/*!
  @brief define FILE_SEQ struct
  @details FILE_SEQ is the most important structure in file playback. It is the structure of playback handle.
  */
typedef struct {

    /*!
      save file aboslute path
      */
    char *m_path[MAX_PATH_NUM];
    /*!
       path id: app will use this path id and driver will not use it
      */
    unsigned long  path_id;

    /*!
          pointer to the 'BUFFERING_PARAM_T' structure
         */
    /*!
       stream type (live or vod)
    */
    FILE_STREAM_TYPE stream_type;
    /*!
       this flag identify whether av decoder is in a stable state
      */
    MT_BOOL  is_stable;
    /*!
      point to audio es packet link list
      */
    es_pkt_data_t *p_audio_buf;
    /*!
      point to video es packet link list
      */
    es_pkt_data_t *p_video_buf;
    /*!
    *  length of video es buffer, used by video decoder
    *
    */
    u32 max_ves_size;

    /*!
      total paths (app can input more than one file path one time)
      */
    int total_path;
    /*!
    forced_stop flag
    */
    int force_stopped;
    /*!
      current play url index
      */
    int cur_play_index;
    /*!
      point to current demuxer
      */
    void *p_cur_demuxer ;
    /*!
      point to current stream
      */
    void *p_cur_stream ;
    /*!
      point to current audio  stream
      */
    void *p_cur_ds_audio;
    /*!
      point to current video  stream
      */
    void *p_cur_ds_video;
    /*!
      point to current subtitle stream
      */
    void *p_cur_ds_sub;

    unsigned int m_audio_codec_type;
    /*!
      current video  codec type
      */
    unsigned int m_video_codec_type;
    /*!
        hint whether variours of av device handles has been initialized
      */
    MT_BOOL init_av_dev;
    /*!
        show what internal event should be processed
      */
    unsigned int internal_event ;
    /*!
        show what command should be processed
      */
    unsigned int m_user_cmd ;
    /*!
        identify whether there is only audio stream
    */
    MT_BOOL  only_audio_mode;
    /*!
    audio output mode
    */
    AUDIO_OUT_MODE audio_output_mode;
    /*!
        identify whether current stream is based on rtsp protocol
    */
    MT_BOOL  rtsp_play_mode;

    /*!
      internal heap memory size for playback
      */
    unsigned int   file_seq_mem_size;
    /*!
     start address of internal heap memory for playback
    */
    ulong    file_seq_mem_start;
    /*!
     play status
      */
    FILE_SEQ_STATUS   m_play_state ;// 1:play  0:stop 2:pause
    /*!
      backup the last play status
      */
    FILE_SEQ_STATUS   m_play_state_backup ;// 1:play  0:stop 2:pause
    /*!
      preload status
      */
    FILE_SEQ_STATUS   m_preload_state ;// 1:play  0:stop 2:pause
    /*!
    seek time integer part
    */
    int   m_seek_cnt ;//
    /*!
    seek time fractional part
    */
    int   m_seek_adj ;//

    /*!
      the left aes size
      */
    int   m_aes_left;
    /*!
       to ac3/aac codec, output of demuxer is only raw data without header
       so , audio es raw data and packet header must be assembled in this buffer
      */

    /*!
      handle for display  device
      */
    void *p_disp_dev;
    /*!
      handle for vdec  device
      */
    void *p_vdec_dev;
    /*!
      handle for audio  device
      */
    void *p_audio_dev ;
    /*!
      subtitle fifo handle
      */

    void *p_sub_fifo_handle ;
    /*!
       mutex for subtitle fifo
      */
    unsigned long sub_fifo_mutex;
    /*!
        lock for file playback
      */
    unsigned long lock;
    /*!
        lock for heap alloc and free memory
      */
    unsigned long heap_lock;
    /*!
      subtitle buffer
      */
    unsigned char subt_buf[2048];
    /*!
        audio es warter level is too high
      */
    MT_BOOL isAudioBufferFull ;
    /*!
        video  es warter level is too high
      */
    MT_BOOL isVideoBufferFull ;
    /*!
        audio decoder is start
      */
    MT_BOOL isAudioDecoderStart;
    /*!
        video decoder is start
      */
    MT_BOOL isVideoDecoderStart;
    /*!
        audio stream is over
      */
    MT_BOOL isAudioEsEnd ;
    /*!
       vido stream is over
      */
    MT_BOOL isVideoEsEnd ;
    /*!
      identify whether audio and video codec type is supported
      */
    MT_BOOL  checkAvTypeOK;
    /*!
      is or not check definition ok
      */
    MT_BOOL  checkDefinitionOK;

    /*!
      identify whether load media successfully
      */
    MT_BOOL  loadMedaiOK;
    /*!
      identify whether playback support trickplay for current stream
      */
    MT_BOOL  canTrickPlay;
    /*!
      identify whether the input url is a valid play url
    */
    MT_BOOL   needTranslateUrl;
    /*!
    *  if length of video packet is shorter than  VES_SEG_LEN
    *    pad them in 1024 bytes
    */
    unsigned char *ves_seg_buf;

    /*!
      temp video es buf postion
      */
    int  m_tmp_ves_buf_pos;
    /*!
      last play speed
      */
    s8  last_speed;
    /*!
      current play speed
      */
    s8  cur_speed;
    /*!
      saved   temp speed
      */
    int  tmp_speed;
    /*!
      Not used anymore
      */
    MT_BOOL isAudMute;
    /*!
      identify whether current work mode is trickplay
      */
    MT_BOOL isTrickPlay;
    /*!
      identify whether current work mode is normal play
      */
    MT_BOOL isNormalPlay;
    /*!
      identify whether current play mode need to be translated from trick to normal
      */
    MT_BOOL isTrickToNormal;
    /*!
      This flag show whether file sequencer should require new video packet from demuxer in this loop
      */
    MT_BOOL needNewVideoData;
    /*!
      This flag show whether file sequencer should require new audio packet from demuxer in this loop
      */
    MT_BOOL needNewAudioData;
    /*!
      identify whether playback use external heap
      */
    MT_BOOL use_ext_heap ;
    /*!
        identify whether fill es task is running or has already exited
      */
    MT_BOOL is_task_alive;
    /*!
        identify whether load media task is running or has already exited
      */
    MT_BOOL is_load_task_alive;
    /*!
        load media state
      */
    unsigned  int   load_media_state ;
    /*!
    identify whether check task is running or has already exited
    */
    CHECK_ES_STATE check_task_state;
    /*!
        identify whether preload task is running or has already exited
      */
    MT_BOOL  is_check_buffering_task_alive;

    /*!
        current   ves number in es buffer
      */
    unsigned  int   totalVesNum ;
    /*!
      current   aes number in es buffer
      */
    unsigned  int   totalAesNum ;
    /*!
      refer to current demuxer
      */
    void *p_demuxer ;
    /*!
      refer to second demuxer (only for debug)
      */
    void *p_demuxer2 ;
    /*!
      refer to current  file stream
      */
    void *p_stream ;
    /*!
      refer  to audio stream of demuxer
      */
    void *p_ds_audio;
    /*!
      refer to video  stream of demuxer
      */
    void *p_ds_video;
    /*!
       refer to  subtitle  stream of demuxer
      */
    void *p_ds_sub;
    /*!
     point to stack memory of fill_es task
      */
    void *pstack;
    /*!
    movie file duration
    */
    int *p_m_duration;


    /*!
      ts priv pointer
      */
    void *ts_priv;
    /*!
      is or not ts flag
      */
    int is_ts;
    /*!
      bit0: 1(audio codec support) 0(audio codec not support)
      bit1: 1(video codec support) 0(video codec not support)
      */
    u32 is_av_codec_support;
    /*!
      auudio format
      */
    //   int audio_format;
    /*!
      audio's samplerate
      */
    int audio_samplerate;
    /*!
      audio's channel count
      */
    int audio_channels;
    /*!
      audio's sample size
      */
    int audio_sample_bits;
    /*!
      audio's bps
      */
    int  audio_bps;//bytes per secondes
    /*!
      audio's language
      */
    char *audio_language;
    /*!
    album string ptr
      */
    char *album;
    /*!
       title string ptr
      */
    char *title;
    /*!
       artist string ptr
      */
    char *artist;
    /*!
      genre string ptr
      */
    char *genre;
    /*!
      comments string ptr
      */
    char *comments;
    /*!
      video's format
      */
    //  int video_format;
    /*!
      display width about video
      */
    int video_disp_w;
    /*!
      display height about video
      */
    int video_disp_h;
    /*!
      video's fps
      */
    int video_fps;
    /*!
      video's bps
      */
    u32  video_bps;//bytes per secondes
    /*!
      for uploading
      */
    VIDEO_W_H_FPS video_whfps;
    /*!
      start time about moive file
      */
    double file_start_time;
    /*!
      total duration about moive file
      */
    double file_duration;

    /*!
      file size
      */
    u64   file_size;

    /*!
      play is or not in play at time mode
      */
    int is_play_at_time;
    /*!
      seek time(seconds)
      */
    float seek_seconds;
    /*!
      start in setted time(second)
      */
    unsigned int start_seconds;
    /*!
      is or not play to the file end
      */
    int is_play_to_end;
    /*!
      audio track id
      */
    int audio_track_id;
    /*!
     audio track num
     */
    int audio_track_num;
    /*!
      audio track language
      */
    TRACK_LANG *audio_lang_array;
    /*!
     adaptive playlist
     */
    FILE_PLAYBACK_ADAPTIVE_PLAYLIST_T adaptive_playlist;
    FILE_PLAYBACK_USER_INTERRUPT_CB_T user_int_cb;
    /*!
      video average bps
      */
    int  video_average_bps;
    /*!
      last video average bps
      */
    int  last_v_average_bps;
    /*!
      last audio average bps
      */
    int  audio_average_bps;
    /*!
      max audio pts
      */
    double  max_audio_pts;
    /*!
      max video pts
      */
    double  max_video_pts;
    /*!
      first reference audio pts
      */
    float  ref_first_audio_pts;
    /*!
       first reference video pts
      */
    float  ref_first_video_pts;
    /*!
      available audio es  number
      */
    u32  available_aes_bytes;
    /*!
      available video es  number
      */
    u32  available_ves_bytes;

    /*!
      extra subtitle
      */
    int exsubtitle;
    /*!
      subtitle id
      */
    int subt_id;
    /*!
      subtitle info pointer
      */
    void *sub_info;
    /*!
      external heap pointer
      */
    void *p_ext_heap_hdl;
    /*!
      decoder capacity
      */
    AV_DEC_CAP  dec_cap;
    /*!
      film duration
      */
    double  duration;//seconds
    /*!
    max film duration
    */
    double  max_duration;//seconds
    /*!
      video pid
      */
    int   video_pid;
    /*!
      audio pid
      */
    int   audio_pid;
    /*!
      pcr pid
      */
    int   pcr_pid;
    /*!
    subtitle pid
    */
    int   subtitle_pid;

    /*!
      subtitle count
      */
    int subtitle_count;
    /*!
    subtitle path ptr
    */
    char **subtitle_path;
    /*!
      task priority
      */
    u32   task_prio;

    /*!
      http task priority
      */
    u32   http_task_prio;

    /*!
     check es task priority
    */
    u32   task_checkes_prio;
    /*!
       check es task stack size
      */
    u32   task_checkes_size;

    /*!
      audio pts from sw demuxer, millisecond
      */
    double   orig_apts;//milli second
    /*!
      system audio pts which was transfromed from origin audio pts
      */
    u64   sys_apts; //micro second
    /*!
      video pts to upload
      */
    u64 vpts_upload;
    /*!
      audio pts to upload (ms)
      */
    u64 apts_upload;
    /*!
      video pts from sw demuxer, millisecond
      */
    double   first_vpts;
    /*!
      video pts from sw demuxer, millisecond
      */
    double   orig_vpts;//milli second
	/*!
      video start pts from sw demuxer, millisecond
      */
	double video_start_pts;
    /*!
      system audio pts which was transfromed from origin video pts
      */
    u64   sys_vpts;//micro second
    /*!
       subtitle's pts from sw demuxer, millisecond
      */
    double   orig_spts;
    /*!
      current video paket to be pushed to es buffer of firmware
      */
    unsigned char *p_v_pkt_start;
    /*!
      current audio paket to be pushed to es buffer of firmware
      */
    unsigned char *p_a_pkt_start;
    /*!
      current subtitle paket to be pushed to es buffer of firmware
      */
    unsigned char *p_sub_pkt_start;

    /*!
      left video data to be pushed to ved tmp buffer
      */
    unsigned int left_v_pkt_bytes;
    /*!
      left audio data to be pushed to ved tmp buffer
      */
    unsigned int left_a_pkt_bytes;

    /*!
      extra audo data, such as, to ac3 ,we should add some audio packet header
      */

    unsigned char *p_extra_aud_buf;
    /*!
      len of extra data
      */

    unsigned char extra_audio_size ;
    /* information about event should be put in */
    file_seq_event_t event;
    /*!
      callback function , caller should register it
      */
    file_seq_event_cb_fun_t event_cb;
    /*!
       current tv system
    */
    unsigned char  cur_tv_hd_fmt;


    /*!
    * the following field is only for internal command buffer
    *
    */

    /*!
       read position
    */
    int  rd_pos;
    /*!
        write position
    */
    int wr_pos;
    /*!
       command buffer fifo
    */
    FP_USER_CMD_T *cmd_fifo;
    /*!
       identify whether fifo is full
    */
    u8 is_cmd_buf_full;
    /*!
       identify whether fifo is full
    */
    int load_media_times;


    /*!
    network play mode
    */
    int ott_playmode;

    /*!
    add by  xxia 1220, for the hls
    */
    HLS_PARSER_RESULT hls_parser;
    /*!
    dash parser
    */

    DASH_PARSER_RESULT dash_parser;
    /*!
      set ott play mode
      */
    file_seq_set_ott_playmode_t set_ott_playmode;

    /*!
      load media function
      */
    file_seq_loadmedia_fun_t loadmedia;
    /*!
      load media in task function
      */
    file_seq_loadmedia_fun_t loadmedia_task;
    /*!
      loat meida times function
      */
    file_seq_loadmedia_times_fun_t loadmedia_times;

    /*!
      unload media function
      */
    file_seq_unloadmedia_fun_t unloadmedia;
    /*!
      start function
      */
    file_seq_start_fun_t      start;
    /*!
      stop function
      */
    file_seq_stop_fun_t       stop;
    /*!
        force stop function
    */
    file_seq_stop_fun_t       force_stop;
    /*!
      pause function
      */
    file_seq_pause_fun_t     pause;
    /*!
      resume function
      */
    file_seq_resume_fun_t   resume;
    /*!
      setspeed function
      */
    file_seq_set_speed_fun_t set_speed;
    /*!
      wait trickplay finish with timeout.
      */
    file_seq_wait_tplay_finish wait_tplay_finish;
    /*!
      preview function
      */
    file_seq_preview_fun_t      preview;
    /*!
      get vpts function
      */
    file_seq_get_vpts_fun_t  get_vpts;
    /*!
      play at time function
      */
    file_seq_play_at_time_fun_t play_at_time;
    /*!
      get status function
      */
    file_seq_get_status_fun_t  get_status;
    /*!
      change audio track function
      */
    file_seq_change_audio_track_fun_t change_audio_track;
    /*!
      get audio track language function
      */
    file_seq_get_audio_track_lang_fun_t get_audio_track_lang;
    /*!
      change video track function
      */
    file_seq_change_video_track_fun_t  change_video_track;
    /*!
      get film info function
      */
    file_seq_get_film_info_fun_t     get_film_info;
    /*!
      set play file path
      */
    file_seq_set_file_path_fun_t     set_file_path;
    /*!
      set play file path  extra function
      */
    file_seq_set_file_path_ex_fun_t     set_file_path_ex;

    /*!
      set av codec type
      */
    file_seq_av_codec_type_fun_t   set_av_codec_type;
    /*!
      do cmd function
      */
    file_seq_do_cmd_fun_t   do_cmd;
    /*!
      dump es function
      */
    file_seq_mp_dump_fun_t mp_dump;//only for debug
    /*!
      dum audio es function
      */
    file_seq_mp_dump_fun_t mp_dumpaudio;//for dump audio from mp3 file
    /*!
      check memory function
      */
    file_seq_run_mem_fun_t  check_mem;//only for debug
    /*!
      get subtitle es packet
      */
    file_seq_get_subt_fun_t get_subt;
    /*!
      get subtitle info
      */
    file_seq_get_subt_info_fun_t get_subt_info;
    /*!
      set subtitle index id
      */
    file_seq_set_subt_id_fun_t set_subt_id;
    /*!
      memory for player init
      */
    file_seq_mem_init_fun_t mem_init;
    /*!
      alloc memory
      */
    file_seq_mem_alloc_fun_t mem_alloc;
    /*!
      free memory
      */
    file_seq_mem_free_fun_t mem_free;
    /*!
      memory for player release
      */
    file_seq_mem_release_fun_t mem_release;
    /*!
      get extra subtitle es packet
      */
    file_seq_get_exsubt_fun_t get_exsubt;


    /*!
         upper layer code call this function to register member variable 'event_cb'
      */
    file_seq_register_event_cb_func_t  register_event_cb;

    /*!
      set tv sysfmt
      */
    file_seq_set_vfmt_fun_t set_tv_sys;

    /*!
      is network stream playing now
      */
    isNetworkStream_fun_t is_network_stream;
    /*!
      set liv_broadcast mode for current player
      */
    file_seq_set_live_broadcast_fun_t set_live_broadcast;

    /*!
      check bg task alive or not
      */
    file_seq_check_bg_task_alive   check_bg_task_alive;

    /*!
      wait seek finish with timeout.
      */
    file_seq_wait_seek_finish wait_seek_finish;

    /*!
      get mp tag.
      */
    file_seq_get_mp_tag get_mp_tag;

    /*!
      set support seek finish.
      */
    file_seq_set_support_seek_finish set_support_seek_finish;

    /*!
      Open network buffering.
      */
    file_seq_set_network_buffering set_network_buffering;

    /*!
      flag for auto set tv sysfmt
      */
    MT_BOOL tv_sys_auto_set;

    /*!
      is or not support trickplay
      */
    int unable_trickplay;

    /*!
    policy for video decoder
    */
    unsigned int vdec_policy;


    /*!
     pb_intrenal struct ptr
     */
    void *pb_internal;

    char *subt_buf_extra;//tizhang@20180822 for 104045
    /*!
     vdec resolution callback
     */
    u8 vdec_resolution_callback;

    int drm_eDrmIndex;//just for playread3000 MTDrm_GetDecryptType

    /*!
     support seek finish.
     */
    MT_BOOL is_support_seek_finish;
    MT_BOOL is_audio_deecoder_error;
    double  last_audio_pts;
} FILE_SEQ_T;

/*!
@addtogroup file_seq  Playback API
@{
@brief Playback API introduction
*/

/*
*
*  the following API should be exposed to other code
*
*            201304-23 PeacerTsui
*
*/
/*!
@brief Get playback handle
@return Return the handle of playback
*/
FILE_SEQ_T *file_seq_get_instance(void);
/*!
@brief check whether playback thread has exited or not
@details check whether playback thread has exited or not
@return Return the status of player
1:player has exited  0:player has not exited
*/
MT_BOOL is_file_seq_exit(void);
/*!
  @brief Create handle of playback
  @details Create handle of playback
  @param[in] p_param Key parameter pointer of playback which configurate the player
  @return Return the handle of playback
  */
void *file_seq_create(PB_SEQ_PARAM_T *p_param);
/*!
  @brief Destroy handle of playback
  @details Destroy handle of playback
  @return void
  */
void  file_seq_destroy(void);

char *get_demuxer_tag_info(void *demux_handle, char *tag);

void *calloc33(u32 n, u32 size);
void *malloc33(u32 size);
void *realloc33(void *old_ptr, u32 size);
void free33(void *ptr);
char *strdup33(const char *s);

void file_seq_vo_handle_set(mt_handle vHandle);
void file_seq_get_vo_handle(mt_handle *vHandle);

MT_BOOL fp_is_loadmedia_state(void);
int  fp_is_timeshift_file(void);

FILE_SEQ_T *x_get_cur_instance(void);
int fp_set_codec_blacklist(FILE_SEQ_T *p_file_seq,
    STREAM_TYPE_E stream_type, int num, int *list);
int file_seq_get_audio_pid(
    FILE_SEQ_T *p_file_seq, const int id_num, int *pid_list);
int file_seq_get_video_pid(
    FILE_SEQ_T *p_file_seq, const int id_num, int *pid_list);
int file_seq_get_network_bitrate(
    FILE_SEQ_T *p_file_seq, long long *bitrate);
int file_seq_get_playlist(FILE_SEQ_T *p_file_seq);
int file_seq_switch_playlist(FILE_SEQ_T *p_file_seq, int playlist_index);
int file_seq_set_extio_data_source(
    FILE_SEQ_T *p_file_seq, FILE_PLAYBACK_EXTIO_CONTEXT_T *extio);
int file_seq_cfg_stream_probe_para(FILE_SEQ_T *p_file_seq,
    int max_stream_probe_size, int max_stream_analyze_duration);
int file_seq_set_network_parameter(
    FILE_SEQ_T *p_file_seq, FILE_PLAYBACK_NETWORK_PARA_T *para);

MT_BOOL check_task_finish(FILE_SEQ_T *p_file_seq);
int file_seq_set_http_header_useragent(char *useragent);
int mp_set_mov_desc_key(char *value);
int mp_set_mov_desc_key_len(char * len);



#ifdef __cplusplus
}
#endif

#endif




