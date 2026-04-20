#ifndef MT_DEC_H
#define MT_DEC_H

/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __VDEC_API_H__
#define __VDEC_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#define FW_UINT32 unsigned int
#define FW_UINT8 unsigned char
#define FW_TRUE               1
#define FW_FALSE              0


typedef enum{
  /*!
    H.264/AVC
  */
  FW_VIDEO_H264,

  /*!
    AVS
  */
  FW_VIDEO_AVS,

  /*!
    MPEG1/2
  */
  FW_VIDEO_MPEG2,

  /*!
    MPEG4
  */
  FW_VIDEO_MPEG4,

  /*!
    VC1
  */
  FW_VIDEO_VC1,

  /*!
    VP8
  */
  FW_VIDEO_VP8,

  /*!
    RV34
  */
  FW_VIDEO_RV34,

  /*!
    HEVC
    */
  FW_VIDEO_HEVC
}video_coding_type_t;



typedef enum{
  DISPLAY_MODE_NO_DI,
  DISPLAY_MODE_4_FIELD,
  DISPLAY_MODE_PRESCALE,
  DISPLAY_MODE_3_FIELD
}display_mode_t;


typedef enum{
  /*!
    display frame once
  */
  FW_DIS_FRAME_ONCE,

  /*!
    display frame twice
  */
  FW_DIS_FRAME_TWICE,

  /*!
    display frame triple
  */
  FW_DIS_FRAME_Triple,

  /*!
    display frame triple
  */
  FW_DIS_FRAME_N_time,

  /*!
    display top field, then bottom field
  */
  FW_DIS_TOP_BOT,

  /*!
    display bottom field, then top field
  */
  FW_DIS_BOT_TOP,

  /*!
    display top field, then bottom field, then top filed
  */
  FW_DIS_TOP_BOT_TOP,

  /*!
    display bottom field, then top field, then bottom field
  */
  FW_DIS_BOT_TOP_BOT
}display_order_mode;

typedef struct
{
  FW_UINT32 pts;
  FW_UINT32 addrLuma;  // byte allian
  FW_UINT32 addrChroma;
}DisFieldInfo_t;


typedef struct
{
  FW_UINT32 frm_cnt;
  FW_UINT32 gop_id;      // mpeg2:temporal_reference, avs:picture_distance,mpeg4:time_increment
  FW_UINT32 pts;
  FW_UINT32 pic_width;
  FW_UINT32 pic_height;
  FW_UINT32 row_jump_value;
  FW_UINT32 row_jump_offset;
  FW_UINT32 slot_idx;
  DisFieldInfo_t filedInfoTop;
  DisFieldInfo_t filedInfoBot;
  DisFieldInfo_t filedInfoTopRight;
  DisFieldInfo_t filedInfoBotRight;
  FW_UINT32 display_order_mode_valid;
  display_order_mode display_order_mode;
  FW_UINT32 repeat_frm_num;  // case FW_DIS_FRAME_N_time, repeat_frm_num show repeat times


  FW_UINT8 frame_pic_flag;      // 1:frame 0:field
  FW_UINT8 progressive_frame;
  FW_UINT8 dirty_flag;          // 1: frame dirty, decode with error  0: frame clean
  FW_UINT8 progressive_sequence;// When a new sequence is coming, main_progressive_sequence will
                                // be changed in decode interrupt, but the progreesive_sequence
                                // of this frame may still be used in display interrupt.
                                // So we should store it for this frame.
  FW_UINT8 top_field_first;     // 1: top field first  0: bottom field first
  FW_UINT8 picture_coding_type; // 0:I, 1:P, 2:B
  FW_UINT8 aspect_ratio;
  FW_UINT8 fw_disable_di;
  FW_UINT8 filed_storage_mode;  // 0: two filed merged storage, 1: two filed separate storage
  FW_UINT8 is_3D_flag;          // 0: no 3D, 1: 3D
  video_coding_type_t video_type;
}DisFrameSlotInfo_t;




#ifdef __cplusplus
}
#endif
#endif //__VDEC_API_H__







#endif
