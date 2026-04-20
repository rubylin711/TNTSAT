/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __PVR_API_H_
#define __PVR_API_H_

#include "mt_unf_avplay.h"


/*!
  MUL_ERR_PVR_MODULE
  */
#define MUL_ERR_PVR_MODULE (0x8030)
/*!
  FIXME
  */
#define MUL_ERR_PVR_NOT_INIT                  ((MUL_ERR_PVR_MODULE << 16) + 0001) 
/*!
  FIXME
  */
#define MUL_ERR_PVR_INVALID_PARA          ((MUL_ERR_PVR_MODULE << 16) + 0002)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_NMUL_PTR                   ((MUL_ERR_PVR_MODULE << 16) + 0003)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_CHN_NOT_INIT          ((MUL_ERR_PVR_MODULE << 16) + 0004)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_INVALID_CHNID         ((MUL_ERR_PVR_MODULE << 16) + 0005)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_NO_CHN_LEFT           ((MUL_ERR_PVR_MODULE << 16) + 0006)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_ALREADY                   ((MUL_ERR_PVR_MODULE << 16) + 0007)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_BUSY                        ((MUL_ERR_PVR_MODULE << 16) + 0008)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_NO_MEM                   ((MUL_ERR_PVR_MODULE << 16) + 0009)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_NOT_SUPPORT           ((MUL_ERR_PVR_MODULE << 16) + 000A)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_RETRY                       ((MUL_ERR_PVR_MODULE << 16) + 000B)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_EXIST                ((MUL_ERR_PVR_MODULE << 16) + 0011)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_NOT_EXIST        ((MUL_ERR_PVR_MODULE << 16) + 0012)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_CANT_OPEN        ((MUL_ERR_PVR_MODULE << 16) + 0013)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_CANT_CLOSE       ((MUL_ERR_PVR_MODULE << 16) + 0014)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_CANT_SEEK          ((MUL_ERR_PVR_MODULE << 16) + 0015)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_CANT_WRITE        ((MUL_ERR_PVR_MODULE << 16) + 0016)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_CANT_READ          ((MUL_ERR_PVR_MODULE << 16) + 0017)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_INVALID_FNAME   ((MUL_ERR_PVR_MODULE << 16) + 0018)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_TILL_START          ((MUL_ERR_PVR_MODULE << 16) + 0019)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_TILL_END              ((MUL_ERR_PVR_MODULE << 16) + 001A)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_FILE_DISC_FULL            ((MUL_ERR_PVR_MODULE << 16) + 001B)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_REC_INVALID_STATE       ((MUL_ERR_PVR_MODULE << 16) + 0021)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_REC_INVALID_DMXID     ((MUL_ERR_PVR_MODULE << 16) + 0022)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_REC_INVALID_FSIZE    ((MUL_ERR_PVR_MODULE << 16) + 0023)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_REC_INVALID_UDSIZE   ((MUL_ERR_PVR_MODULE << 16) + 0024)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_PLAY_INVALID_STATE   ((MUL_ERR_PVR_MODULE << 16) + 0031)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_PLAY_INVALID_DMXID   ((MUL_ERR_PVR_MODULE << 16) + 0032)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_INDEX_CANT_MKIDX   ((MUL_ERR_PVR_MODULE << 16) + 0041)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_INDEX_FORMAT_ERR   ((MUL_ERR_PVR_MODULE << 16) + 0042) 
/*!
  FIXME
  */
#define MUL_ERR_PVR_INDEX_DATA_ERR       ((MUL_ERR_PVR_MODULE << 16) + 0043)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_INTF_EVENT_INVAL   ((MUL_ERR_PVR_MODULE << 16) + 0051)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_INTF_EVENT_NOREG   ((MUL_ERR_PVR_MODULE << 16) + 0052)  

/*!
  FIXME
  */
#define MUL_ERR_PVR_EVA_FILTER_ADD_FAILED   ((MUL_ERR_PVR_MODULE << 16) + 0061)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_EVA_FILTER_INSERT_CHN   ((MUL_ERR_PVR_MODULE << 16) + 0062)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_EVA_FILTER_NO_PIN          ((MUL_ERR_PVR_MODULE << 16) + 0062)  
/*!
  FIXME
  */
#define MUL_ERR_PVR_EVA_CONNECT_FAILED     ((MUL_ERR_PVR_MODULE << 16) + 0062)  

#define MUL_ERR_PVR_CIPHER_KEY_ERROR     ((MUL_ERR_PVR_MODULE << 16) + 0064) 

/*!
  FIXME
  */
#define PVR_MAX_CIPHER_KEY_LEN (8)
/*!
  FIXME
  */
#define PVR_MAX_FILENAME_LEN (256)


/*!
  FIXME
  */
typedef enum 
{
  /*!
    FIXME
    */
  MUL_PVR_REC_INDEX_TYPE_NONE,      
  /*!
    FIXME
    */
  MUL_PVR_REC_INDEX_TYPE_VIDEO,     
  /*!
    FIXME
    */
  MUL_PVR_REC_INDEX_TYPE_AUDIO,     
  /*!
    FIXME
    */
  MUL_PVR_REC_INDEX_TYPE_END       
}mul_pvr_rec_index_type_t;


/*!
  FIXME
  */
typedef enum  
{
  /*!
    FIXME
    */
  MUL_PVR_STREAM_TYPE_TS,
  /*!
    FIXME
    */
  MUL_PVR_STREAM_TYPE_PES,
  /*!
    FIXME
    */
  MUL_PVR_STREAM_TYPE_ALL_TS,
  /*!
    FIXME
    */
  MUL_PVR_STREAM_TYPE_SCRAMBLE_TS,
  /*!
    FIXME
    */
  MUL_PVR_STREAM_TYPE_OTHER,
  /*!
    FIXME
    */
  MUL_PVR_STREAM_TYPE_END  
}mul_pvr_stream_type_t;

/*!
  FIXME
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
  MUL_CIPHER_ALG_END   
}mul_cipher_alg_t;


/*!
  tag_mul_pvr_cipher
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
  FIXME
  */
typedef struct 
{
  /*!
    FIXME
    */
  u16 v_pid; 
  /*!
    FIXME
    */
  u16 a_pid; 
  /*!
    vdec_src_fmt_t
    */
  u16 video_type; 
  /*!
    adec_src_fmt_vsb_t
    */
  u16 audio_type;
  /*!
    FIXME
    */
  u16 pcr_pid;
  /*!
    FIXME
    */
  u16 pmt_pid;
  /*!
    service_idx
    */
  u32 service_idx : 8;
  /*!
    channel id
    */
  u32 channel_id : 8;
  /*!
    FIXME
    */
  u32 extern_num : 16;
  /*!
    FIXME
    */
  extern_pid_t extern_pid[REC_MAX_EXTERN_PID];
}mul_rec_media_t;


/*!
  FIXME
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
  vfs_time_t start;
  /*!
    FIXME
    */
  mul_rec_media_t media_info;
  /*!
    see dmx_input_type_t
    */
  u16 demux_index;
  /*!
    see dmx_rec_path_t
    */
  u16 rec_in;
/*!
  FIXME
  */
  u32 stk_size;
  /*!
    FIXME
    */
  u32 user_data_size; 
  /*!
    FIXME
    */
  mul_pvr_stream_type_t stream_type; 
  /*!
    rec buffer, align 8
    */
  u8 *p_rec_buffer;
  /*!
    rec min size(800 * 188 * 6)
    timeshift min size(800 * 188 * 16)
    */
  u32 rec_buffer_size;
  /*!
    p_ridx_buf
    */
  u8 *p_ridx_buf;
  /*!
    ridx_buf_size
    */
  u32 ridx_buf_size;
  /*!
    FIXME
    */
  u64 max_file_size;
  /*!
    FIXME
    */
  u32 b_clear_stream : 1;
  /*!
    FIXME
    */
  u32 b_rewind : 1;
  /*!
    block write
    */
  u32 b_block_write : 1;
  /*!
    record null packet
    */
  u32 b_rec_null_packet : 1;
  /*!
    is dvr
    */
  u32 b_is_dvr : 1;
  /*!
    is lock
    */
  u32 b_is_lock : 1;
  /*!
    reserved
    */
  u32 reserved : 26;
  /*!
    FIXME
    */
  u32 file_name_len;
  /*!
    FIXME
    */
  u16 file_name[PVR_MAX_FILENAME_LEN]; 
  /*!
    biss key cfg
    */
  mul_pvr_cipher_t  key_cfg;
  u8 user_data[IDX_INFO_USER_DATA_LENGTH];
}mul_pvr_rec_attr_t;

/*!
  FIXME
  */
typedef enum 
{
  /*!
    FIXME
    */
  MUL_PVR_REC_STATE_INVALID,        
  /*!
    FIXME
    */
  MUL_PVR_REC_STATE_CREATED,           
  /*!
    FIXME
    */
  MUL_PVR_REC_STATE_RUNNING,        
  /*!
    FIXME
    */
  MUL_PVR_REC_STATE_PAUSE,          
  /*!
    FIXME
    */
  MUL_PVR_REC_STATE_STOPPING,       
  /*!
    FIXME
    */
  MUL_PVR_REC_STATE_STOP,           
  /*!
    FIXME
    */
  MUL_PVR_REC_STATE_END
}mul_pvr_rec_state_t;

/*!
  FIXME
  */
typedef struct tag_pvr_buf_status
{
  /*!
    buffer_size
    */
  u32 buffer_size;
  /*!
    used_size
    */
  u32 used_size;
}mul_pvr_buf_status_t;

/*!
  FIXME
  */
typedef struct tag_pvr_rec_status
{
  /*!
  通道所处的状态。 
    */
  mul_pvr_rec_state_t state; 
  /*!
  当前录制文件的大小（单位：kbyte）。 
    */
 
  u32 cur_write_point; 
  /*!
  当前录制的帧数。 
    */
  u32  cur_write_frame;
  /*!
  当前录制的毫秒数。其值为录制文件的实际有效长度。 
    */
 
  u32 cur_time_ms;
  /*!
  录制文件的实际起始点。录制回绕前，为第一帧时间；录制回绕后，为回绕点的时间。 
    */
  u32 start_time_ms;
  /*!
  录制文件中结束帧的时间，没有环回前即最后一帧的时间。 
    */
 
  u32 end_time_ms;
  /*!
  录制通道缓冲区状态。 
    */
  mul_pvr_buf_status_t  rec_buffer_state; 
}mul_pvr_rec_status_t;

/*!
  FIXME
  */
typedef struct
{
  /*!
    FIXME
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
    ts seq fifo
    */
  u8 *p_parse_buf;
  /*!
    fifo size
    */
  u32 parse_buf_size;
  /*!
    buffer
    */
  u8 *p_play_buffer;
  /*!
    buffer size
    */
  u32 play_buffer_size;
  /*!
    buffer
    */
  u8 *p_ridx_buf;
  /*!
    buffer size
    */
  u32 ridx_buf_size;
  /*!
    stream_type
    */
  mul_pvr_stream_type_t  stream_type;
  /*!
     is_clear_stream
     */
  MT_BOOL  is_clear_stream;
  MT_BOOL  is_dvr;
  /*!
   key_cfg
    */ 
  mul_pvr_cipher_t  key_cfg;
  /*!
    file_name
     */
  u16 file_name[PVR_MAX_FILENAME_LEN];
  /*!
    file_name_len
     */
  u32 file_name_len;
}mul_pvr_play_attr_t;


/*!
  FIXME
  */
typedef enum tag_t_av_play_stop_mode
{
  /*!
    FIXME
    */
  MUL_AVPLAY_STOP_MODE_STILL = 0,  
  /*!
    FIXME
    */
  MUL_AVPLAY_STOP_MODE_BLACK = 1,  
  /*!
    FIXME
    */
  MUL_AVPLAY_STOP_MODE_END
}mul_av_play_stop_mode_t;

/*!
  FIXME
  */
typedef struct
{
  /*!
    timeout_ms
    */
  u32  timeout_ms; 
  /*!
    enMode
    */
  mul_av_play_stop_mode_t  enMode;
}mul_av_play_stop_opt_t;

/*!
  FIXME
  */
typedef enum
{

  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_NORMAL,                    
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_2X_FAST_FORWARD,      
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_4X_FAST_FORWARD,      
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_8X_FAST_FORWARD,      
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_16X_FAST_FORWARD,     
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_32X_FAST_FORWARD,     
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_2X_FAST_BACKWARD,     
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_4X_FAST_BACKWARD,     
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_8X_FAST_BACKWARD,     
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_16X_FAST_BACKWARD,    
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_32X_FAST_BACKWARD,    
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_2X_SLOW_FORWARD,      
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_4X_SLOW_FORWARD,      
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_8X_SLOW_FORWARD,      
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_16X_SLOW_FORWARD,     
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_32X_SLOW_FORWARD,     
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_2X_SLOW_BACKWARD,     
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_4X_SLOW_BACKWARD,     
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_8X_SLOW_BACKWARD,     
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_16X_SLOW_BACKWARD,    
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_32X_SLOW_BACKWARD,    
  /*!
    FIXME
    */
  MUL_PVR_PLAY_SPEED_END                   
}mul_pvr_play_speed_t;

/*!
  FIXME
  */
typedef struct
{
  /*!
    FIXME
    */
  mul_pvr_play_speed_t  speed; 
}mul_pvr_play_mode_t;

/*!
  FIXME
  */
typedef enum
{
  /*!
    FIXME
    */
  MUL_PVR_PLAY_POS_TYPE_SIZE,              
  /*!
    FIXME
    */
  MUL_PVR_PLAY_POS_TYPE_TIME,              
  /*!
    FIXME
    */
  MUL_PVR_PLAY_POS_TYPE_FRAME,             
  /*!
    FIXME
    */
  MUL_PVR_PLAY_POS_TYPE_END             
}mul_pvr_play_pos_type_t;

/*!
  player seek type
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
  FIXME
  */
typedef struct
{
  /*!
    pos_type
   */
  mul_pvr_play_pos_type_t  pos_type; 
  /*!
    offset
    */
  s32  offset; 

  /*!
    whence: SEEK_SET/SEEK_CUR/SEEK_END。  
    */
  mul_pvr_seek_op_t  whence;
}mul_pvr_play_position_t;

/*!
  FIXME
  */
typedef enum
{
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_INVALID,       
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_INIT,          
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_PLAY,          
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_PAUSE,         
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_FF,            
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_FB,            
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_SF,
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_SB,
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_STEPF,         
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_STEPB,         
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_STOP,          
  /*!
    FIXME
    */
  MUL_PVR_PLAY_STATE_END          
}mul_pvr_play_state_t;

/*!
  FIXME
  */
typedef struct
{
  /*!
    state
    */
  mul_pvr_play_state_t state;
  /*!
    speed
    */
  mul_pvr_play_speed_t speed; 
  /*!
    cur_play_pos
    */
  u32  cur_play_pos;
  /*!
    cur_play_frame
    */
  u32  cur_play_frame;
  /*!
    cur_play_time_ms
    */
  u32  cur_play_time_ms;
}mul_pvr_play_status_t;

/*!
  FIXME
  */
typedef struct
{
  /*!
    idx_type
    */
  mul_pvr_rec_index_type_t idx_type; 
  /*!
    frame_num
    */
  u32 frame_num; 
  /*!
    start_time_ms
    */
  u32 start_time_ms;
  /*!
    end_time_ms
    */
  u32 end_time_ms;
  /*!
    valid_size_byte
    */
  u32 valid_size_byte;
}mul_pvr_file_attr_t;

/*!
  FIXME
  */
typedef enum
{
  /*!
    FIXME
    */
  MUL_PVR_EVENT_PLAY_EOF,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_PLAY_SOF,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_PLAY_ERROR,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_PLAY_REACH_REC,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_PLAY_RESV,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_PLAY_TIME_UPDATE,
  /*!
    event value 0 is mem full, 1 is write error
    */
  MUL_PVR_EVENT_REC_DISKFULL,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_REC_ERROR,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_REC_OVER_FIX,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_REC_REACH_PLAY,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_REC_DISK_SLOW,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_REC_READY_FOR_PLAY,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_REC_TIME_UPDATE,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_REC_RESV,
  /*!
    FIXME
    */
  MUL_PVR_EVENT_END
}mul_pvr_event_t;

/*!
  FIXME
  */
typedef struct
{
  /*!
  usr_data_info_len
  */
  u32  usr_data_info_len; 
  /*!
  idx
  */
  u32  idx; 
  /*!
  idx_type
  */
  mul_pvr_rec_index_type_t  idx_type;
}mul_pvr_gen_idx_attr_t;

/*!
  FIXME
  */
typedef struct
{ 
   /*!
    file_name
     */
  u16 file_name[PVR_MAX_FILENAME_LEN];
   
  /*!
    file_name_len
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
  FIXME
  */
typedef struct
{ 
  /*
     u32 version
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
  FIXME
  */
typedef void(*event_call_back)(u32 chnid,
                                mul_pvr_event_t event_type, s32 event_value, void *p_args);

/*!
  FIXME
  */
RET_CODE mul_pvr_init(void); 

RET_CODE mul_pvr_rec_get_default_attr(u16 usb_letter, u8 asc_pg_name[PROGRAM_NAME_MAX],
  mul_pvr_rec_attr_t *p_rec_attr);

 RET_CODE mul_pvr_rec_get_timeshift_attr(u16 usb_letter, mul_pvr_rec_attr_t *p_rec_attr);
/*!
  FIXME
  */
RET_CODE mul_pvr_rec_create_chn(u32 *p_chnid, const mul_pvr_rec_attr_t *p_rec_attr); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_rec_destroy_chn (u32 chnid); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_rec_set_chn (u32 chnid, const mul_pvr_rec_attr_t *p_rec_attr); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_rec_get_chn (u32 chanid, mul_pvr_rec_attr_t *p_rec_attr); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_rec_start_chn (u32 chanid); 

/*!
  FIXME
  */
RET_CODE mul_pvr_rec_pause_chn(u32 chanid);

/*!
  FIXME
  */
RET_CODE mul_pvr_rec_resume_chn(u32 chanid);

/*!
  FIXME
  */
RET_CODE mul_pvr_rec_change_pid(u32 chanid, mul_rec_media_t *p_media);

/*!
  FIXME
  */
RET_CODE mul_pvr_rec_stop_chn (u32 chanid); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_rec_get_status (u32 chanid, mul_pvr_rec_status_t *p_rec_status); 

/*!
  mul_pvr_rec_get_status_h
  */
mul_pvr_rec_status_t *mul_pvr_rec_get_status_h(u32 chanid);


RET_CODE mul_pvr_play_get_attr(u16 *p_file_name, mul_pvr_play_attr_t *p_play_attr);

/*!
  FIXME
  */
RET_CODE mul_pvr_play_create_chn (u32 *p_chnid, const mul_pvr_play_attr_t *p_play_attr); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_destroy_chn (u32 chanid); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_start_timeshift (u32 *p_play_chnid,
                                  u32 rec_chnid, mul_pvr_play_attr_t *p_play_attr); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_stop_timeshift (u32 play_chnid, const mul_av_play_stop_opt_t *p_stop_opt); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_set_chn (u32 chanid, const mul_pvr_play_attr_t *p_play_attr); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_get_chn (u32 chanid, mul_pvr_play_attr_t *p_play_attr); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_start_chn (u32 chanid); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_stop_chn (u32 chanid, const mul_av_play_stop_opt_t *p_stop_opt); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_pause_chn (u32 chanid); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_resume_chn (u32 chanid); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_trick_play (u32 chanid, const mul_pvr_play_mode_t *p_trick_mode); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_seek (u32 chanid, const mul_pvr_play_position_t *p_position); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_play_step (u32 chanid, u32 direction); 

/*!
  FIXME
  */
RET_CODE mul_pvr_play_change_audio(u32 chanid, u16 a_pid, u8 fmt);

/*!
  FIXME
  */
RET_CODE mul_pvr_play_get_status (u32 chanid, mul_pvr_play_status_t *p_status); 

/*!
  mul_pvr_play_get_status_h
  */
mul_pvr_play_status_t *mul_pvr_play_get_status_h(u32 chanid);
 
/*!
  FIXME
  */
RET_CODE mul_pvr_register_event (u32 chanid, event_call_back callBack, void *p_args); 
 
/*!
  FIXME
  */
RET_CODE mul_pvr_unregister_event (u32 chanid); 

/*!
  FIXME
  */
void  mul_pvr_config_debug_info (u8 debug_level);

/*
  configure cipher engine
*/
RET_CODE mul_pvr_config_cipher_engine(int engine, u8 *p_key, u8 keylength);


/*
   get current cipher engine and key
 */
/*
add key and key length
*/
RET_CODE mul_pvr_get_cipher_key(u8 *p_engine, u8 **p_key, u8 *p_keylength);
/*!
  get current program subtitle pid
*/
u32 mul_pvr_get_subt_pids(u16 *pids);
/*
  mul_pvr_play_set_user_attr
*/
RET_CODE mul_pvr_play_set_user_attr(mul_pvr_play_user_attr_t *p_attr);
/*
  mul_pvr_play_get_user_attr
*/
RET_CODE mul_pvr_play_get_user_attr(mul_pvr_play_user_attr_t *p_attr);
#endif //__PVR_API_H_

