/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
@file pvr_api.h
@brief Describes the information about the pvr module
*/
#ifndef __PVR_API_H_
#define __PVR_API_H_




//fix start

/*! 
  UTC format time definition
  */
typedef struct
{
  /*!
    Year
    */
  u16 year;
  /*!
    Month
    */
  u8 month;
  /*!
    Day
    */
  u8 day;
  /*!
    Hour
    */
  u8 hour;
  /*!
    Minute
    */
  u8 minute;
  /*!
    Sec
    */
  u8 second;  
  /*!
    researved.
    */
  u8 reserved;
} utc_time_t;

//fix end

/***********************************pvr api***********************************/
/*!
  MUL_ERR_PVR_MODULE
  */
#define MUL_ERR_PVR_MODULE (0x8030)
/*!
  PVR_MAX_CIPHER_KEY_LEN
  */
#define PVR_MAX_CIPHER_KEY_LEN (8)
/*!
  PVR_MAX_FILENAME_LEN
  */
#define PVR_MAX_FILENAME_LEN (256)

/***********************************structure definition***********************************/

/*!
  @brief Defines the error code of pvr
  */
typedef enum
{
  MUL_ERR_PVR_NOT_INIT = ((MUL_ERR_PVR_MODULE << 16) + 1),
  MUL_ERR_PVR_INVALID_PARA,
  MUL_ERR_PVR_NMUL_PTR,
  MUL_ERR_PVR_CHN_NOT_INIT,
  MUL_ERR_PVR_INVALID_CHNID,
  MUL_ERR_PVR_NO_CHN_LEFT,
  MUL_ERR_PVR_ALREADY,
  MUL_ERR_PVR_BUSY,
  MUL_ERR_PVR_NO_MEM,
  MUL_ERR_PVR_NOT_SUPPORT,//!<10
  MUL_ERR_PVR_RETRY,
  MUL_ERR_PVR_FILE_EXIST,
  MUL_ERR_PVR_FILE_NOT_EXIST,
  MUL_ERR_PVR_FILE_CANT_OPEN,
  MUL_ERR_PVR_FILE_CANT_CLOSE,
  MUL_ERR_PVR_FILE_CANT_SEEK,
  MUL_ERR_PVR_FILE_CANT_WRITE,
  MUL_ERR_PVR_FILE_CANT_READ,
  MUL_ERR_PVR_FILE_INVALID_FNAME,
  MUL_ERR_PVR_FILE_TILL_START,//!<20
  MUL_ERR_PVR_FILE_TILL_END,
  MUL_ERR_PVR_FILE_DISC_FULL,
  MUL_ERR_PVR_REC_INVALID_STATE,
  MUL_ERR_PVR_REC_INVALID_DMXID,
  MUL_ERR_PVR_REC_INVALID_FSIZE,
  MUL_ERR_PVR_REC_INVALID_UDSIZE,
  MUL_ERR_PVR_PLAY_INVALID_STATE,
  MUL_ERR_PVR_PLAY_INVALID_DMXID,
  MUL_ERR_PVR_INDEX_CANT_MKIDX,
  MUL_ERR_PVR_INDEX_FORMAT_ERR,//!<30
  MUL_ERR_PVR_INDEX_DATA_ERR,
  MUL_ERR_PVR_INTF_EVENT_INVAL,
  MUL_ERR_PVR_INTF_EVENT_NOREG,
  MUL_ERR_PVR_EVA_FILTER_ADD_FAILED,
  MUL_ERR_PVR_EVA_FILTER_INSERT_CHN,
  MUL_ERR_PVR_EVA_FILTER_NO_PIN,
  MUL_ERR_PVR_EVA_CONNECT_FAILED,
  MUL_ERR_PVR_CIPHER_KEY_ERROR,
}mul_pvr_err_code_t;

/*!
  @brief Defines the input/output stream type of pvr
  */
typedef enum  
{
  /*!
    Ts stream
    */
  MUL_PVR_STREAM_TYPE_TS,
  /*!
    Pes stream, not support now
    */
  MUL_PVR_STREAM_TYPE_PES,
  /*!
    Full TP ts stream
    */
  MUL_PVR_STREAM_TYPE_ALL_TS,
  /*!
    Scramble ts stream
    */
  MUL_PVR_STREAM_TYPE_SCRAMBLE_TS,
  /*!
    Other ts stream, not support now
    */
  MUL_PVR_STREAM_TYPE_OTHER,
  /*!
    end
    */
  MUL_PVR_STREAM_TYPE_END  
}mul_pvr_stream_type_t;

/*!
  @brief Defines the type of security algorithm
  */
typedef enum
{
  /*!
    DES
    */
  MUL_CIPHER_ALG_DES,
  /*!
    3DES
    */
  MUL_CIPHER_ALG_3DES,
  /*!
    AES
    */
  MUL_CIPHER_ALG_AES,
  /*!
    end
    */
  MUL_CIPHER_ALG_END,
}mul_cipher_alg_t;


/*!
  @brief Defines the attributes of pvr cipher
  */
typedef struct tag_mul_pvr_cipher
{
  /*!
    reserved
    */
  u32 reserved : 15;
  /*!
    is biss key
    */
  u32 is_biss_key : 1;
  /*!
    audio odd key len
    */
  u32 audio_key_odd_len : 4;
  /*!
    audio even key length
    */
  u32 audio_key_even_len : 4;
  /*!
    video odd key length
    */
  u32 video_key_odd_len : 4;
  /*!
    video even key length
    */
  u32 video_key_even_len : 4;
  /*!
    audio odd key
    */
  u8 audio_key_odd[PVR_MAX_CIPHER_KEY_LEN];
  /*!
    audio even key
    */
  u8 audio_key_even[PVR_MAX_CIPHER_KEY_LEN];
  /*!
    video odd key
    */
  u8 video_key_odd[PVR_MAX_CIPHER_KEY_LEN];
  /*!
    video even key
    */
  u8 video_key_even[PVR_MAX_CIPHER_KEY_LEN];
}mul_pvr_cipher_t;


/*!
  @brief Defines the attributes of record media
  */
typedef struct 
{
  /*!
    video pid
    */
  u16 v_pid; 
  /*!
    audio pid
    */
  u16 a_pid; 
  /*!
    video type
    */
//  vdec_src_fmt_t video_type; 
  int video_type; 
  /*!
    audio type
    */
//  adec_src_fmt_vsb_t audio_type;
  int audio_type;
  /*!
    pcr pid
    */
  u16 pcr_pid;
  /*!
    pmt pid
    */
  u16 pmt_pid;
  /*!
    reserved
    */
  u16 reserved;
  /*!
    extern pid number
    */
  u16 extern_num;
  /*!
    extern pids
    */
  extern_pid_t extern_pid[REC_MAX_EXTERN_PID];
}mul_rec_media_t;

/*!
  @brief Defines the function callback of pvr data encrypt
  */
typedef void(*data_encrypt_callback)(u8 *p_data, u32 length);

/*!
  @brief Defines the function callback of pvr data encrypt
  */
typedef void(*data_encrypt_callback_ex)(u8 *p_data, u32 length, void *p_args);

/*!
  @brief Defines the attributes of record
  */
typedef struct tag_pvr_rec_attr
{
  /*!
    program name
    */
  u16 program[PROGRAM_NAME_MAX];
  /*!
    start time
    */
  utc_time_t start;
  /*!
    record media info
    */
  mul_rec_media_t media_info;
  /*!
    demux index, see dmx_input_type_t
    */
  u16 demux_index;
  /*!
    record index, see dmx_rec_path_t
    */
  u16 rec_in;
  /*!
    record stack size
    */
  u32 stk_size;
  /*!
    user data size
    */
  u32 user_data_size; 
  /*!
    record clear stream or not
    */
  MT_BOOL b_clear_stream;
  /*!
    stream type
    */
  mul_pvr_stream_type_t stream_type; 
  /*!
    record buffer
    */
  u8 *p_rec_buffer;
  /*!
    record buffer size
    if set record buffer, buffer size should be more than (800 * 188 * 16)
    */
  u32 rec_buffer_size;
  /*!
    record index buffer
    */
  u8 *p_ridx_buf;
  /*!
    record index buffer size
    */
  u32 ridx_buf_size;
  /*!
    max record file size
    */
  u64 max_file_size;
  /*!
    if rewind or not, TRUE for timeshift
    */
  MT_BOOL b_rewind;
  /*!
    if block write or not, TRUE for special used
    */
  MT_BOOL b_block_write;
  /*!
    if record null ts packet or not
    */
  MT_BOOL b_rec_null_packet;
  /*!
    file name length
    */
  u32 file_name_len;
  /*!
    record saved file name, full path
    */
  u16 file_name[PVR_MAX_FILENAME_LEN]; 
  /*!
    biss key config
    */
  mul_pvr_cipher_t  key_cfg;
  /*!
    user data
    */
  u8 user_data[IDX_INFO_USER_DATA_LENGTH];
  /*!
    data_encrypt_callback
    */
  data_encrypt_callback encrypt_cb;
  /*!
    data_encrypt_callback_ex
    */
  data_encrypt_callback_ex encrypt_cb_ex;
  /*!
    args for encrypt
    */
  void *p_encrypt_args;
}mul_pvr_rec_attr_t;

/*!
  @brief Defines the type of record state
  */
typedef enum 
{
  /*!
    invalid record state
    */
  MUL_PVR_REC_STATE_INVALID,        
  /*!
    record chain is created
    */
  MUL_PVR_REC_STATE_CREATED,           
  /*!
    record chain is running
    */
  MUL_PVR_REC_STATE_RUNNING,        
  /*!
    record chain is pause
    */
  MUL_PVR_REC_STATE_PAUSE,          
  /*!
    record chain is stopping
    */
  MUL_PVR_REC_STATE_STOPPING,       
  /*!
    record chain is stopped
    */
  MUL_PVR_REC_STATE_STOP,           
  /*!
    end
    */
  MUL_PVR_REC_STATE_END
}mul_pvr_rec_state_t;

/*!
  @brief Defines the attributes of record buffer
  */
typedef struct tag_pvr_buf_status
{
  /*!
    buffer size
    */
  u32 buffer_size;
  /*!
    used size
    */
  u32 used_size;
}mul_pvr_buf_status_t;

/*!
  @brief Defines the attributes of record status
  */
typedef struct tag_pvr_rec_status
{
  /*!
    record state
    */
  mul_pvr_rec_state_t state; 
  /*!
    current write size(kbytes)
    */
  u32 cur_write_point; 
  /*!
    current write frame count
    */
  u32  cur_write_frame;
  /*!
    current record time(ms)
    */
  u32 cur_time_ms;
  /*!
    time stamp of the oldest data, for timeshift
    */
  u32 start_time_ms;
  /*!
    time stamp of the newest data, for timeshift
    */
  u32 end_time_ms;
  /*!
    record buffer status
    */
  mul_pvr_buf_status_t  rec_buffer_state; 
}mul_pvr_rec_status_t;

/*!
  @brief Defines the attributes of saved record info
  */
typedef struct tag_mul_pvr_rec_info
{
  /*!
    program name
    */
  u16 program[PROGRAM_NAME_MAX];
  /*!
    start time
    */
  utc_time_t start;
  /*!
    pmt_pid
    */
  u32 pmt_pid : 16;
  /*!
    encrypt_flag
    */
  u32 encrypt_flag : 8;
  /*!
    lock_flag
    */
  u32 lock_flag : 8;
  /*!
    total time (second)
    */
  u32 total_time;
  /*!
    total size
    */
  u32 total_size;
  /*!
    video pid
    */
  u32 v_pid : 16;
  /*!
    audio pid
    */
  u32 a_pid : 16;
  /*!
    pcr pid
    */
  u32 pcr_pid : 16;
  /*!
    vdec_src_fmt_t
    */
  u32 video_fmt : 8;
  /*!
    adec_src_fmt_vsb_t
    */
  u32 audio_fmt : 8;
  /*!
    ext pid count
    */
  u32 ext_pid_cnt : 8;
  /*!
    user data len
    */
  u32 user_data_len : 8;
  /*!
    audio_key_len
    */
  u32 audio_key_odd_len : 4;
  /*!
    audio_key_len
    */
  u32 audio_key_even_len : 4;
  /*!
    video_key_len
    */
  u32 video_key_odd_len : 4;
  /*!
    video_key_len
    */
  u32 video_key_even_len : 4;
  /*!
    audio odd key
    */
  u8 audio_key_odd[CAS_CH_KEY_MAX];
  /*!
    audio even key
    */
  u8 audio_key_even[CAS_CH_KEY_MAX];
  /*!
    video odd key
    */
  u8 video_key_odd[CAS_CH_KEY_MAX];
  /*!
    video even key
    */
  u8 video_key_even[CAS_CH_KEY_MAX];
  /*!
    ext pid
    */
  extern_pid_t ext_pid[REC_MAX_EXTERN_PID];
  /*!
    user data
    */
  u8 user_data[IDX_INFO_USER_DATA_LENGTH];
}mul_pvr_rec_info_t;

/*!
  @brief Defines the callback of get mute state
  */
typedef MT_BOOL (*pvr_get_mute_func)(void);

/*!
  @brief Defines the function callback of pvr data decrypt
  */
typedef void(*data_decrypt_callback)(u8 *p_data, u32 length);

/*!
  @brief Defines the function callback of pvr data decrypt
  */
typedef void(*data_decrypt_callback_ex)(u8 *p_data, u32 length, void *p_args);

/*!
  @brief Defines the attributes of playback
  */
typedef struct
{
  /*!
    play stack size
    */
  u32 stk_size;
  /*!
    video pid
    */
  u16 v_pid;
  /*!
    audio pid
    */
  u16 a_pid;
  /*!
    video type see vdec_src_fmt_t
    */
  u8 v_type;
  /*!
    audio type see adec_src_fmt_vsb_t
    */
  u8 a_type;
  /*!
    pcr pid
    */
  u16 pcr_pid;
  /*!
    parse buffer for ts seq
    */
  u8 *p_parse_buf;
  /*!
    parse buffer size(490 * 188)
    */
  u32 parse_buf_size;
  /*!
    play buffer
    */
  u8 *p_play_buffer;
  /*!
    play buffer size, normal about(2 * 376 * 1024)
    */
  u32 play_buffer_size;
  /*!
    index buffer
    */
  u8 *p_ridx_buf;
  /*!
    index buffer size, max about 1MBytes
    */
  u32 ridx_buf_size;
  /*!
    stream type
    */
  mul_pvr_stream_type_t  stream_type;
  /*!
    if clear stream or not
    */
  MT_BOOL  is_clear_stream;
  /*!
    key config for Scrambled stream
    */
  mul_pvr_cipher_t  key_cfg;
  /*!
    file name
    */
  u16 file_name[PVR_MAX_FILENAME_LEN];
  /*!
    file name length
    */
  u32 file_name_len;
  /*!
    get_mute
    */
  pvr_get_mute_func get_mute;
  /*!
    data_decrypt_callback
    */
  data_decrypt_callback decrypt_cb;
  /*!
    data_decrypt_callback_ex
    */
  data_decrypt_callback_ex decrypt_cb_ex;
  /*!
    args for decrypt
    */
  void *p_decrypt_args;
}mul_pvr_play_attr_t;


/*!
  @brief Defines the type of playback stop
  */
typedef enum tag_t_av_play_stop_mode
{
  /*!
    freeze video when stop
    */
  MUL_AVPLAY_STOP_MODE_STILL = 0,  
  /*!
    black video when stop
    */
  MUL_AVPLAY_STOP_MODE_BLACK = 1,  
  /*!
    end
    */
  MUL_AVPLAY_STOP_MODE_END
}mul_av_play_stop_mode_t;

/*!
  @brief Defines the attributes of playback stop operation
  */
typedef struct
{
  /*!
    stop operation timeout
    */
  u32  timeout_ms; 
  /*!
    av stop mode
    */
  mul_av_play_stop_mode_t  enMode;
}mul_av_play_stop_opt_t;

/*!
  @brief Defines the type of play speed
  */
typedef enum
{
  MUL_PVR_PLAY_SPEED_NORMAL,
  MUL_PVR_PLAY_SPEED_2X_FAST_FORWARD,
  MUL_PVR_PLAY_SPEED_4X_FAST_FORWARD,
  MUL_PVR_PLAY_SPEED_8X_FAST_FORWARD,
  MUL_PVR_PLAY_SPEED_16X_FAST_FORWARD,
  MUL_PVR_PLAY_SPEED_32X_FAST_FORWARD,
  MUL_PVR_PLAY_SPEED_2X_FAST_BACKWARD,
  MUL_PVR_PLAY_SPEED_4X_FAST_BACKWARD,
  MUL_PVR_PLAY_SPEED_8X_FAST_BACKWARD,
  MUL_PVR_PLAY_SPEED_16X_FAST_BACKWARD,
  MUL_PVR_PLAY_SPEED_32X_FAST_BACKWARD,
  MUL_PVR_PLAY_SPEED_2X_SLOW_FORWARD,
  MUL_PVR_PLAY_SPEED_4X_SLOW_FORWARD,
  MUL_PVR_PLAY_SPEED_8X_SLOW_FORWARD,//!<not support now
  MUL_PVR_PLAY_SPEED_16X_SLOW_FORWARD,//!<not support now
  MUL_PVR_PLAY_SPEED_32X_SLOW_FORWARD,//!<not support now
  MUL_PVR_PLAY_SPEED_2X_SLOW_BACKWARD,//!<not support now
  MUL_PVR_PLAY_SPEED_4X_SLOW_BACKWARD,//!<not support now
  MUL_PVR_PLAY_SPEED_8X_SLOW_BACKWARD,//!<not support now
  MUL_PVR_PLAY_SPEED_16X_SLOW_BACKWARD,//!<not support now
  MUL_PVR_PLAY_SPEED_32X_SLOW_BACKWARD,//!<not support now
  MUL_PVR_PLAY_SPEED_END
}mul_pvr_play_speed_t;

/*!
  @brief Defines the attributes of play mode
  */
typedef struct
{
  /*!
    play speed
    */
  mul_pvr_play_speed_t  speed; 
}mul_pvr_play_mode_t;

/*!
  @brief Defines the type of play position
  */
typedef enum
{
  /*!
    position type size
    */
  MUL_PVR_PLAY_POS_TYPE_SIZE,
  /*!
    position type time
    */
  MUL_PVR_PLAY_POS_TYPE_TIME,
  /*!
    position type frame, not support now
    */
  MUL_PVR_PLAY_POS_TYPE_FRAME,    
  /*!
    end
    */
  MUL_PVR_PLAY_POS_TYPE_END             
}mul_pvr_play_pos_type_t;

/*!
  @brief Defines the type of seek operation
  */
typedef enum
{
  /*!
    seek set
    */
  MUL_PVR_SEEK_SET,
  /*!
    seek current
    */
  MUL_PVR_SEEK_CUR,
  /*!
    seek end, need set total time&total size
    */
  MUL_PVR_SEEK_END,
}mul_pvr_seek_op_t;


/*!
  @brief Defines the attributes of play position
  */
typedef struct
{
  /*!
    position type
    */
  mul_pvr_play_pos_type_t  pos_type; 
  /*!
    position offset, if time unit is ms, if size unit is byte
    */
  s32  offset; 
  /*!
    seek type 
    */
  mul_pvr_seek_op_t  whence;
}mul_pvr_play_position_t;

/*!
  @brief Defines the type of play state
  */
typedef enum
{
  /*!
    invalid play state
    */
  MUL_PVR_PLAY_STATE_INVALID,
  /*!
    init
    */
  MUL_PVR_PLAY_STATE_INIT,
  /*!
    normal playing
    */
  MUL_PVR_PLAY_STATE_PLAY,
  /*!
    pause
    */
  MUL_PVR_PLAY_STATE_PAUSE,
  /*!
    fast forward playing
    */
  MUL_PVR_PLAY_STATE_FF,
  /*!
    fast backward playing
    */
  MUL_PVR_PLAY_STATE_FB,
  /*!
    slow forward playing
    */
  MUL_PVR_PLAY_STATE_SF,
  /*!
    slow backward playing
    */
  MUL_PVR_PLAY_STATE_SB,
  /*!
    step forward playing
    */
  MUL_PVR_PLAY_STATE_STEPF,
  /*!
    step backward playing
    */
  MUL_PVR_PLAY_STATE_STEPB,
  /*!
    stopped
    */
  MUL_PVR_PLAY_STATE_STOP,
  /*!
    end
    */
  MUL_PVR_PLAY_STATE_END
}mul_pvr_play_state_t;

/*!
  @brief Defines the attributes of play status
  */
typedef struct
{
  /*!
    play state
    */
  mul_pvr_play_state_t state;
  /*!
    play speed
    */
  mul_pvr_play_speed_t speed; 
  /*!
    current play position(kbytes)
    */
  u32  cur_play_pos;
  /*!
    current play frame
    */
  u32  cur_play_frame;
  /*!
    current play time
    */
  u32  cur_play_time_ms;
  /*!
    total play time
    */
  u32  total_play_time_ms;
}mul_pvr_play_status_t;

/*!
  @brief Defines the type of pvr event
  */
typedef enum
{
  /*!
    file play end
    */
  MUL_PVR_EVENT_PLAY_EOF,
  /*!
    trick play auto change to normal play
    */
  MUL_PVR_EVENT_PLAY_SOF,
  /*!
    play error
    */
  MUL_PVR_EVENT_PLAY_ERROR,
  /*!
    timeshift play reach record, not support now
    */
  MUL_PVR_EVENT_PLAY_REACH_REC,
  /*!
    reserved play event
    */
  MUL_PVR_EVENT_PLAY_RESV,
  /*!
    play time update
    */
  MUL_PVR_EVENT_PLAY_TIME_UPDATE,
  /*!
    event value 0 is disk full, 1 is write error
    */
  MUL_PVR_EVENT_REC_DISKFULL,
  /*!
    record error
    */
  MUL_PVR_EVENT_REC_ERROR,
  /*!
    record over fixed size, not support now
    */
  MUL_PVR_EVENT_REC_OVER_FIX,
  /*!
    timeshift record(write) reach play(read)
    */
  MUL_PVR_EVENT_REC_REACH_PLAY,
  /*!
    disk slow
    */
  MUL_PVR_EVENT_REC_DISK_SLOW,
  /*!
    record data ready for play
    */
  MUL_PVR_EVENT_REC_READY_FOR_PLAY,
  /*!
    record time update
    */
  MUL_PVR_EVENT_REC_TIME_UPDATE,
  /*!
    record save a new file, event value is file index:0, 1, 2..
    */
  MUL_PVR_EVENT_REC_RESV,
  /*!
    end
    */
  MUL_PVR_EVENT_END
}mul_pvr_event_t;

/*!
  @brief Defines the attribute of user play data
  */
typedef struct
{ 
  /*!
    pvr file name
    */
  u16 file_name[PVR_MAX_FILENAME_LEN];
   
  /*!
    file name length
    */
  u32 file_name_len;
  
  /*!
    default audio channel
    */
  u8 audio_channel;
  /*
    0: stereo, 1: left, 2: right, 3: mono
    */
  u8 audio_track;
  /*!
    reserve
    */
  u16  reserve;
}mul_pvr_play_user_attr_t;

/*!
  @brief Defines the attribute of saved user play data
  */
typedef struct
{ 
  /*
    version
    */
  u32 version;
  /*!
    default audio channel
    */
  u8 audio_channel;
  /*
    0: stereo, 1: left, 2: right, 3: mono
    */
  u8 audio_track;
  /*!
    reserve
    */
  u16  reserve;
}mul_pvr_play_user_save_attr_t;

/*!
  @brief Defines the function callback of pvr
  */
typedef void(*event_call_back)(u32 chnid,
                                mul_pvr_event_t event_type, s32 event_value, void *p_args);

/******************************* API declaration *****************************/


/*!
@~english
@brief Initializes the pvr module.
@param [in] void none
@return SUCCESS Success
        ERR_FAILURE Fail
*/
RET_CODE mul_pvr_init(void); 

/*!
@~english
@brief Get the default record attributes.
@param [in] usb_letter  Usb partition to save pvr file.
@param [in] asc_pg_name The program name, used to gen record file name. 
@param [in] is_timeshift If timeshift mode.
@param [out] p_rec_attr Pointer to record attributes. For details, see the description of ::mul_pvr_rec_attr_t. 
@return SUCCESS Success
        ERR_FAILURE The input parameter is invalid.
        MUL_ERR_PVR_FILE_DISC_FULL The usb partition free size is too small.
*/
RET_CODE mul_pvr_rec_get_attr(u16 usb_letter, u8 asc_pg_name[PROGRAM_NAME_MAX],
  MT_BOOL is_timeshift, mul_pvr_rec_attr_t *p_rec_attr);

/*!
@~english
@brief Create a record chain.
@param [out] p_chnid Pointer to the handle of a created record chain.
@param [in] p_rec_attr Pointer to record attributes. For details, see the description of ::mul_pvr_rec_attr_t. 
@return SUCCESS Success
        MUL_ERR_PVR_NOT_INIT  The pvr module not init.
        MUL_ERR_PVR_INVALID_PARA  The input parameter is invalid.
        MUL_ERR_PVR_NO_CHN_LEFT   No chain left for pvr.
        MUL_ERR_PVR_EVA_FILTER_ADD_FAILED Eva add filter fial.
        MUL_ERR_PVR_EVA_FILTER_INSERT_CHN Eva insert filter to chain fail.
        MUL_ERR_PVR_EVA_FILTER_NO_PIN Eva filter no pin for connect.
        MUL_ERR_PVR_NO_MEM  Pvr alloc memory fail.
        MUL_ERR_PVR_EVA_CONNECT_FAILED  Eva pin connect fail.
*/
RET_CODE mul_pvr_rec_create_chn(u32 *p_chnid, const mul_pvr_rec_attr_t *p_rec_attr); 
 
/*!
  @~english
  @brief Destroys record chain. 
  @param [in] chnid  Chain handle.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
*/
RET_CODE mul_pvr_rec_destroy_chn(u32 chnid); 
 
/*!
  @~english
  @brief Set record chain attributes, used for special requirement. 
  @param [in] chnid  Chain handle.
  @param [in] p_rec_attr Pointer to record attributes. For details, see the description of ::mul_pvr_rec_attr_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid. 
*/
RET_CODE mul_pvr_rec_set_chn(u32 chnid, const mul_pvr_rec_attr_t *p_rec_attr); 
 
/*!
  @~english
  @brief Get record chain attributes. 
  @param [in] chnid  Chain handle.
  @param [out] p_rec_attr Pointer to record attributes. For details, see the description of ::mul_pvr_rec_attr_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
*/
RET_CODE mul_pvr_rec_get_chn(u32 chnid, mul_pvr_rec_attr_t *p_rec_attr); 
 
/*!
  @~english
  @brief Start the record chain. 
  @param [in] chnid  Chain handle.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          MUL_ERR_PVR_FILE_CANT_OPEN  Do chain open fail.
          ERR_FAILURE     Demux config fail.
*/
RET_CODE mul_pvr_rec_start_chn(u32 chnid); 

/*!
  @~english
  @brief Pause record chain. 
  @param [in] chnid  Chain handle.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          ERR_FAILURE  Chain do cmd fail.
*/
RET_CODE mul_pvr_rec_pause_chn(u32 chnid);

/*!
  @~english
  @brief Resume the record chain. 
  @param [in] chnid  Chain handle.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          ERR_FAILURE  Chain do cmd fail.
*/
RET_CODE mul_pvr_rec_resume_chn(u32 chnid);

/*!
  @~english
  @brief Reset pids while recording. 
  @details The function used for recording dynamic pid program.
  @param [in] chnid  Chain handle.
  @param [in] p_media Pointer to record media attributes. For details, see the description of ::mul_rec_media_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          ERR_FAILURE     Demux config fail.
*/
RET_CODE mul_pvr_rec_change_pid(u32 chnid, mul_rec_media_t *p_media);

/*!
  @~english
  @brief Stop the record chain. 
  @param [in] chnid  Chain handle.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          ERR_FAILURE   Chain do cmd fail.
*/
RET_CODE mul_pvr_rec_stop_chn(u32 chnid); 
 
/*!
  @~english
  @brief Get record status. 
  @param [in] chnid  Chain handle.
  @param [out] p_rec_status  Pointer to record status. For details, see the description of ::mul_pvr_rec_status_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
*/
RET_CODE mul_pvr_rec_get_status(u32 chnid, mul_pvr_rec_status_t *p_rec_status); 

/*!
  @~english
  @brief Get pointer of record status. 
  @param [in] chnid  Chain handle.
  @return if fail return NULL, else return pointer of record status.
*/
mul_pvr_rec_status_t *mul_pvr_rec_get_status_h(u32 chnid);


/*!
  @~english
  @brief Get saved record info. 
  @param [in] p_file_name  Pvr file full path name.
  @param [out] p_rec_info  Pointer to record info attributes. For details, see the description of ::mul_pvr_rec_info_t. 
  @return SUCCESS Success  
          ERR_FAILURE      Parse ridx file fail. 
*/
RET_CODE mul_pvr_get_rec_info(u16 *p_file_name, mul_pvr_rec_info_t *p_rec_info);

/*!
  @~english
  @brief Get default play attributes. 
  @param [in] p_file_name  Pvr file full path name.
  @param [out] p_play_attr  Pointer to play attributes. For details, see the description of ::mul_pvr_play_attr_t. 
  @return SUCCESS Success  
          ERR_FAILURE      Parse idx file fail. 
*/
RET_CODE mul_pvr_play_get_attr(u16 *p_file_name, mul_pvr_play_attr_t *p_play_attr);

/*!
@~english
@brief Create a play chain.
@param [out] p_chnid Pointer to the handle of a created play chain.
@param [in] p_rec_attr Pointer to play attributes. For details, see the description of ::mul_pvr_play_attr_t. 
@return SUCCESS Success
        MUL_ERR_PVR_NOT_INIT  The pvr module not init.
        MUL_ERR_PVR_NO_CHN_LEFT   No chain left for pvr.
        MUL_ERR_PVR_EVA_FILTER_ADD_FAILED Eva add filter fial.
        MUL_ERR_PVR_EVA_FILTER_INSERT_CHN Eva insert filter to chain fail.
        MUL_ERR_PVR_EVA_FILTER_NO_PIN Eva filter no pin for connect.
        MUL_ERR_PVR_EVA_CONNECT_FAILED  Eva pin connect fail.
*/
RET_CODE mul_pvr_play_create_chn(u32 *p_chnid, const mul_pvr_play_attr_t *p_play_attr); 
 
/*!
  @~english
  @brief Destroy the play chain. 
  @param [in] chnid  Play chain handle.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
*/
RET_CODE mul_pvr_play_destroy_chn(u32 chnid); 
 
/*!
@~english
@brief Create a timeshift play chain and start the chain.
@param [out] p_play_chnid Pointer to the handle of a created play chain.
@param [in] rec_chnid  Record chain handle.
@param [in] p_play_attr Pointer to play attributes. For details, see the description of ::mul_pvr_play_attr_t. 
@return SUCCESS Success
        MUL_ERR_PVR_NOT_INIT  The pvr module not init.
        MUL_ERR_PVR_NO_CHN_LEFT   No chain left for pvr.
        MUL_ERR_PVR_EVA_FILTER_ADD_FAILED Eva add filter fial.
        MUL_ERR_PVR_EVA_FILTER_INSERT_CHN Eva insert filter to chain fail.
        MUL_ERR_PVR_EVA_FILTER_NO_PIN Eva filter no pin for connect.
        MUL_ERR_PVR_EVA_CONNECT_FAILED  Eva pin connect fail.
*/
RET_CODE mul_pvr_play_start_timeshift(u32 *p_play_chnid,
                                  u32 rec_chnid, mul_pvr_play_attr_t *p_play_attr);
 
/*!
  @~english
  @brief Stop and destroy the play chain. 
  @param [in] chnid  Play chain handle.
  @param [in] p_stop_opt  Pointer to play stop attributes. For details, see the description of ::mul_av_play_stop_opt_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
*/
RET_CODE mul_pvr_play_stop_timeshift(u32 play_chnid, const mul_av_play_stop_opt_t *p_stop_opt); 
 
/*!
  @~english
  @brief Set play chain attributes, used for special requirement.
  @param [in] chnid  Chain handle.
  @param [in] p_play_attr Pointer to play attributes. For details, see the description of ::mul_pvr_play_attr_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid. 
*/
RET_CODE mul_pvr_play_set_chn(u32 chnid, const mul_pvr_play_attr_t *p_play_attr); 
 
/*!
  @~english
  @brief Get play chain attributes. 
  @param [in] chnid  Chain handle.
  @param [out] p_play_attr Pointer to play attributes. For details, see the description of ::mul_pvr_play_attr_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
*/
RET_CODE mul_pvr_play_get_chn(u32 chnid, mul_pvr_play_attr_t *p_play_attr); 
 
/*!
  @~english
  @brief Start the play chain. 
  @param [in] chnid  Chain handle.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          MUL_ERR_PVR_NO_MEM  Pvr alloc memory fail.
          MUL_ERR_PVR_CIPHER_KEY_ERROR Config cipher key error.
          MUL_ERR_PVR_FILE_CANT_OPEN  Do chain open fail.
          ERR_FAILURE     Demux config fail.
*/
RET_CODE mul_pvr_play_start_chn(u32 chnid); 
 
/*!
  @~english
  @brief Stop the play chain. 
  @param [in] chnid  Chain handle.
  @param [in] p_stop_opt  Pointer to play stop attributes. For details, see the description of ::mul_av_play_stop_opt_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          ERR_FAILURE   Chain do cmd fail.
*/
RET_CODE mul_pvr_play_stop_chn(u32 chnid, const mul_av_play_stop_opt_t *p_stop_opt); 
 
/*!
  @~english
  @brief Pause the play chain. 
  @param [in] chnid  Chain handle.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          ERR_FAILURE  Chain do cmd fail.
*/
RET_CODE mul_pvr_play_pause_chn(u32 chnid); 
 
/*!
  @~english
  @brief Resume the play chain. 
  @param [in] chnid  Chain handle.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          ERR_FAILURE  Chain do cmd fail.
*/
RET_CODE mul_pvr_play_resume_chn(u32 chnid); 
 
/*!
  @~english
  @brief Set trick play mode. 
  @param [in] chnid  Chain handle.
  @param [in] p_trick_mode  Pointer to play mode attributes. For details, see the description of ::mul_pvr_play_mode_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          ERR_FAILURE  Chain do cmd fail.
*/
RET_CODE mul_pvr_play_trick_play(u32 chnid, const mul_pvr_play_mode_t *p_trick_mode); 
 
/*!
  @~english
  @brief Start play at set position. 
  @param [in] chnid  Chain handle.
  @param [in] p_trick_mode  Pointer to play position attributes. For details, see the description of ::mul_pvr_play_position_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          ERR_FAILURE  Chain do cmd fail.
*/
RET_CODE mul_pvr_play_seek(u32 chnid, const mul_pvr_play_position_t *p_position); 

/*!
  @~english
  @brief Change audio pid for multi track recorded program. 
  @param [in] chnid  Chain handle.
  @param [in] a_pid  Audio pid.
  @param [in] fmt  Audio format. For details, see the description of ::adec_src_fmt_vsb_t
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
          ERR_FAILURE  Chain do cmd fail.
*/
RET_CODE mul_pvr_play_change_audio(u32 chnid, u16 a_pid, u8 fmt);


/*!
  @~english
  @brief Get play status. 
  @param [in] chnid  Chain handle.
  @param [out] p_status  Pointer to play status. For details, see the description of ::mul_pvr_play_status_t. 
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
*/
RET_CODE mul_pvr_play_get_status(u32 chnid, mul_pvr_play_status_t *p_status); 

/*!
  @~english
  @brief Get pointer of play status. 
  @param [in] chnid  Chain handle.
  @return if fail return NULL, else return pointer of play status.
*/
mul_pvr_play_status_t *mul_pvr_play_get_status_h(u32 chnid);
 
/*!
  @~english
  @brief Register callback for pvr chain. 
  @param [in] chnid  Chain handle.
  @param [in] callBack  Pvr callback. For details, see the description of ::event_call_back. 
  @param [in] p_args  Callback arguments.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
*/
RET_CODE mul_pvr_register_event(u32 chnid, event_call_back callBack, void *p_args); 
 
/*!
  @~english
  @brief Unregister callback for pvr chain. 
  @param [in] chnid  Chain handle.
  @return SUCCESS Success  
          MUL_ERR_PVR_NOT_INIT      Pvr moudle is not initialized. 
          MUL_ERR_PVR_INVALID_PARA     The input parameter is invalid.  
*/
RET_CODE mul_pvr_unregister_event(u32 chnid); 

/*!
  @~english
  @brief Set cipher engine for security pvr, used for special requirement. 
  @param [in] engine  Cipher engine.
  @param [in] p_key  Cipher engine key.
  @param [in] keylength  Key length.
  @return SUCCESS Success  
          ERR_FAILURE  Fail. 
*/
RET_CODE mul_pvr_config_cipher_engine(int engine, u8 *p_key, u8 keylength);

/*!
  @~english
  @brief Get cipher engine for security pvr, used for special requirement. 
  @param [out] p_engine  Pointer to cipher engine.
  @param [out] pp_key  Pointer to cipher engine key.
  @param [out] p_keylength  Pointer to key length.
  @return SUCCESS Success  
          ERR_FAILURE  Fail. 
*/
RET_CODE mul_pvr_get_cipher_key(u8 *p_engine, u8 **pp_key, u8 *p_keylength);

/*!
  @~english
  @brief Get current program subtitle pids, used for special requirement. 
  @param [out] pids  Pointer to pids array.
  @return pids count. 
*/
u32 mul_pvr_get_subt_pids(u16 *pids);


/*!
  @~english
  @brief Set pvr play user attributes, used for special requirement. 
  @param [in] p_attr  Pointer to play user attributes. For details, see the description of ::mul_pvr_play_user_attr_t. 
  @return SUCCESS Success  
          ERR_FAILURE  Fail. 
*/
RET_CODE mul_pvr_play_set_user_attr(mul_pvr_play_user_attr_t *p_attr);

/*!
  @~english
  @brief Get pvr play user attributes, used for special requirement. 
  @param [out] p_attr  Pointer to play user attributes. For details, see the description of ::mul_pvr_play_user_attr_t. 
  @return SUCCESS Success  
          ERR_FAILURE  Fail. 
*/
RET_CODE mul_pvr_play_get_user_attr(mul_pvr_play_user_attr_t *p_attr);

#endif //__PVR_API_H_

