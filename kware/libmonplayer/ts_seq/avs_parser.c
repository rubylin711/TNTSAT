/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include "mt_type.h"
//#include "sys_define.h"
//#include "sys_cfg.h"
#include "drv_adp.h"
#include "mtos_mem.h"
#include "mtos_sem.h"
#include "mtos_printk.h"
#include "mtos_task.h"

#include "fifo_kw.h"
#include "ts_sequence.h"
#include "avs_parser.h"

#define TS_SEQ_DEBUG  printf

#define MT_ASSERT

//#define DEBUG_TS_SEQUENCE
#ifdef   DEBUG_TS_SEQUENCE

  #define  MY_LOG      printf
  #define  MY_DEBUG    printf
  #define  MY_ERROR    printf

#else

  #define  MY_LOG      printf
  #define  MY_DEBUG    printf
  #define  MY_ERROR    printf

#endif

#define INTERNAL_FIFO_SIZE   (2 * 188 * 2560)
 /*!
  ts packet id
  */
#define AVS_TS_PID(data) \
  ((unsigned short)(((unsigned char)(data[2])) | \
         (((unsigned short)((unsigned char)(data[1] & 0x1F))) << 8)))

/*!
  payload unit  start
  */
#define AVS_PAYLOAD_UNIT_START(data) ((data[1] & 0x40) == 0x40)

/*!
  video pre start code
  */
#define AVS_PRE_START_CODE(x) \
  ((((unsigned char)(x)[0] == 0x00)) && (((unsigned char)(x)[1]) == 0x00) && \
      (((unsigned char)(x)[2]) == 0x01))

/*!
  is AVS I pic start code
  */
#define IS_AVS_I_PICTURE_START_CODE(x) \
  (AVS_PRE_START_CODE(x) && ((unsigned char)(x)[3] == 0xB3))

/*!
  is AVS PB pic start code
  */
#define IS_AVS_PB_PICTURE_START_CODE(x) \
  (AVS_PRE_START_CODE(x) && ((unsigned char)(x)[3] == 0xB6))


char * avs_pes_packet_start(char * p_data, unsigned long * p_payload)
{
unsigned char adaptation_field_control = 0;
unsigned char adaptation_field_length = 0;

adaptation_field_control = p_data[3] & 0x30;
adaptation_field_control >>= 4;

if (adaptation_field_control >= 2)
{
adaptation_field_length = p_data[4];
adaptation_field_length += 5;
if (adaptation_field_length > 187)
{
(*p_payload) = 0;
return (p_data + 4);
}
(*p_payload) = 188 - adaptation_field_length;
return (p_data + adaptation_field_length);
}
else
{
(*p_payload) = 184;
return (p_data + 4);
}

}

char * avs_pes_data_start(char * p_data, unsigned long * p_payload)
{
char pes_header_length = 0;

pes_header_length = p_data[8];
pes_header_length += 9;
(*p_payload) -= pes_header_length;

return (char *)(p_data + pes_header_length);
}

static  video_frame_t  avs_x_check_frame_type(char * p_ts_packet , int len/*188*/)
{
char  *p_pes_data_start = NULL;
unsigned long payload = 0;
MT_BOOL is_i_picture_start_code = FALSE;
MT_BOOL is_pb_picture_start_code = FALSE;

char  *p_data =  NULL;
char  *p_es_payload_start = NULL;

video_frame_t picture_coding_type = 0;
int  ts_header_len = 0;
int  pes_header_len = 0;
int es_payload_len = 0;
int i = 0;


if (AVS_PAYLOAD_UNIT_START(p_ts_packet) == 1)
{
p_pes_data_start = avs_pes_packet_start(p_ts_packet, &payload);
ts_header_len = p_pes_data_start - p_ts_packet;
payload = len - ts_header_len;

if ((payload >= 4))
{

p_es_payload_start = avs_pes_data_start(p_pes_data_start, &payload);
pes_header_len = p_es_payload_start - p_pes_data_start;

p_data = p_es_payload_start;
es_payload_len = len - pes_header_len - ts_header_len;

is_i_picture_start_code = IS_AVS_I_PICTURE_START_CODE(p_data);
is_pb_picture_start_code = IS_AVS_PB_PICTURE_START_CODE(p_data);

//do{
while((i + 3 < es_payload_len) && ((!is_i_picture_start_code) && (!is_pb_picture_start_code)))
{
p_data ++;
i ++;
is_i_picture_start_code = IS_AVS_I_PICTURE_START_CODE(p_data);
is_pb_picture_start_code = IS_AVS_PB_PICTURE_START_CODE(p_data);
}

//find gop start code
if(is_i_picture_start_code)
{
picture_coding_type = I_FRAME;

}
//find picture start code
else if(is_pb_picture_start_code)
{
if((i + 6) < es_payload_len)
{

if((p_data[6] & 0xC0) == 0x40)
{
picture_coding_type = P_FRAME;
}
else if((p_data[6] & 0xC0) == 0x80)
{
picture_coding_type = B_FRAME;
}
else
{
MY_DEBUG("NNNNNNNNNNNN  picture_coding_type:%d\n",picture_coding_type);

}

}
else if((i + 6) >=  es_payload_len)
{
MY_DEBUG("MMMMMMMMMMM  picture_coding_type:%d\n",picture_coding_type);

}
}
else//not find start code
{
picture_coding_type = 0;
}


}
return picture_coding_type;
}
return 0;
}

static int avs_do_revert_parse_gop(ts_seq_t * p_Handle)
{
//
ts_seq_t *p_TsSeqHandle = p_Handle;
int          data_len =                  p_TsSeqHandle->data_len;
char  *p_data =  p_TsSeqHandle->p_data + data_len -188;
int          cur_video_pid =           p_TsSeqHandle->video_pid;
char *p_tmp_buffer_wp =     p_TsSeqHandle->p_tmp_buffer_wp;
int         left_data_len  =            p_TsSeqHandle->left_data_len;


unsigned long packet_cnt = 0;
int                array_index = 0;
char *p_temp = NULL;
int               pid = 0xffffff;
video_frame_t cur_video_frame_type;
int                 packet_index = 0;
MT_BOOL  processLeftData = FALSE;

//process last left data
if(left_data_len)
{
packet_cnt = (left_data_len / 188);
//p_data = p_TsSeqHandle->p_data + (data_len-left_data_len);
p_data = p_TsSeqHandle->p_data + left_data_len -188;
processLeftData = TRUE;
}
//process input ts packet firstly
else
{
packet_cnt = data_len / 188;
processLeftData = FALSE;
}


//parse per ts packet
while(packet_index < packet_cnt)
{
p_temp = p_data - packet_index * 188;
packet_index ++;

if(processLeftData)
{
p_TsSeqHandle->left_data_len -= 188;
}

pid = AVS_TS_PID(p_temp);
//video pes
if(pid == cur_video_pid)
{
cur_video_frame_type = avs_x_check_frame_type(p_temp,188);
//video pes && start unit && Key frame
if(cur_video_frame_type == I_FRAME)
{
if(p_TsSeqHandle->isFindFirstIFrame  == FALSE)
//	&& p_TsSeqHandle->isFindSecondIFrame == FALSE)
{
//find the  tail  of current gop
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
p_TsSeqHandle->isFindFirstIFrame = TRUE;
array_index = p_TsSeqHandle->cur_gop_type.len;
p_TsSeqHandle->cur_gop_type.frame_type_array[array_index] = I_FRAME;
p_TsSeqHandle->cur_gop_type.len++;

#ifdef WIN32
assert(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
MT_ASSERT(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
memcpy(p_tmp_buffer_wp,p_temp,188);

p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index] = p_tmp_buffer_wp;

p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp -188;
p_TsSeqHandle->tmp_buffer_free_space -= 188;
break;
}
}
//video pes && start unit && P Frame
else if(cur_video_frame_type == P_FRAME)
{

p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
array_index = p_TsSeqHandle->cur_gop_type.len;
p_TsSeqHandle->cur_gop_type.frame_type_array[array_index] = P_FRAME;
p_TsSeqHandle->cur_gop_type.len++;
#ifdef WIN32
assert(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
MT_ASSERT(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
memcpy(p_tmp_buffer_wp,p_temp,188);
p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index] = p_tmp_buffer_wp;
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp - 188;
p_TsSeqHandle->tmp_buffer_free_space -= 188;

#ifdef PLAY_LESS_MEM
//this is not iframe ,reset tmp buf
p_TsSeqHandle->p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_start - 187;
p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;
p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index]
         = p_TsSeqHandle->p_tmp_buffer_start + 1;
#endif
}
//video pes && start unit && B frame
else if(cur_video_frame_type == B_FRAME)
{

p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
array_index = p_TsSeqHandle->cur_gop_type.len;
p_TsSeqHandle->cur_gop_type.frame_type_array[array_index] = B_FRAME;
p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index] = p_tmp_buffer_wp;
p_TsSeqHandle->cur_gop_type.len++;
#ifdef WIN32
assert(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
MT_ASSERT(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
memcpy(p_tmp_buffer_wp,p_temp,188);
//p_tmp_buffer_wp+=188;
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp - 188;
p_TsSeqHandle->tmp_buffer_free_space -=  188;

#ifdef PLAY_LESS_MEM
//this is not iframe ,reset tmp buf
p_TsSeqHandle->p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_start - 187;
p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;
p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index]
         = p_TsSeqHandle->p_tmp_buffer_start + 1;
#endif

}
//vidoe pes && none start unit
else
{
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
#ifdef WIN32
assert(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
MT_ASSERT(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
memcpy(p_tmp_buffer_wp,p_temp,188);
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp - 188;
p_TsSeqHandle->tmp_buffer_free_space -=188;
}

}
//none video pes
else
{
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
if((p_tmp_buffer_wp - 188)  <  p_TsSeqHandle->p_tmp_buffer_end)
{
TS_SEQ_DEBUG("not find GOP in 1 M bytes!!!!!\n");

}
else
{


memcpy(p_tmp_buffer_wp,p_temp,188);
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp -188;
p_TsSeqHandle->tmp_buffer_free_space -= 188;
}
}

}

if(packet_index == packet_cnt)
{
p_TsSeqHandle->left_data_len = 0;
}
else//packet_index != packet_cnt
{
if(processLeftData == FALSE)
{
p_TsSeqHandle->left_data_len = p_TsSeqHandle->data_len - packet_index * 188;
}
}

return 0;
}
static video_frame_t last_video_frame_type = 0;
static int avs_do_parse_gop(ts_seq_t * p_Handle)
{
ts_seq_t *p_TsSeqHandle = p_Handle;
char *p_data =                     p_TsSeqHandle->p_data;
int        data_len =                   p_TsSeqHandle->data_len;
int        cur_video_pid =           p_TsSeqHandle->video_pid;
char *p_tmp_buffer_wp =    p_TsSeqHandle->p_tmp_buffer_wp;
int         left_data_len  =          p_TsSeqHandle->left_data_len;


unsigned long packet_cnt = 0;
int                array_index = 0;
char *p_temp = NULL;
int               pid = 0xffffff;
video_frame_t cur_video_frame_type;
int                 packet_index = 0;
MT_BOOL  processLeftData = FALSE;


//process last left data
if(left_data_len)
{
packet_cnt = left_data_len / 188;
p_data = p_TsSeqHandle->p_data + (data_len - left_data_len);
processLeftData = TRUE;
}
//process input ts packet firstly
else
{
packet_cnt = data_len / 188;
processLeftData = FALSE;
}


//parse per ts packet
while(packet_index < packet_cnt  && (!p_TsSeqHandle->isExit))
{

p_temp = p_data + packet_index * 188;
packet_index ++;

if(p_temp[0] != 0x47)
{
MY_ERROR("[ERROR][do_parse_gop] find invalid ts paket !!!\n");
MY_ERROR("[ERROR][do_parse_gop] not find sync code [0x47] !!!\n");
continue;
}

if(processLeftData)
{
p_TsSeqHandle->left_data_len -= 188;
}

pid = AVS_TS_PID(p_temp);
//video pes
if(pid == cur_video_pid)
{
cur_video_frame_type = avs_x_check_frame_type(p_temp,188);
//video pes && start unit && Key frame
if(cur_video_frame_type == I_FRAME)
{
last_video_frame_type = I_FRAME;
if(p_TsSeqHandle->isFindFirstIFrame == FALSE
  &&  p_TsSeqHandle->isFindSecondIFrame == FALSE)
{
//find the  key frame of current gop
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
p_TsSeqHandle->isFindFirstIFrame = TRUE;
array_index = p_TsSeqHandle->cur_gop_type.len;
p_TsSeqHandle->cur_gop_type.frame_type_array[array_index] = I_FRAME;
p_TsSeqHandle->cur_gop_type.len++;

#ifdef WIN32
assert(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
MT_ASSERT(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
memcpy(p_tmp_buffer_wp,p_temp,188);

p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index] = p_tmp_buffer_wp;

p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
p_TsSeqHandle->tmp_buffer_free_space -= 188;

}
else if(p_TsSeqHandle->isFindFirstIFrame == TRUE
 && p_TsSeqHandle->isFindSecondIFrame == FALSE)
{
//find the tail of current gop or the key frame of next gop
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
p_TsSeqHandle->isFindSecondIFrame = TRUE;
array_index = p_TsSeqHandle->cur_gop_type.len;
p_TsSeqHandle->cur_gop_type.frame_type_array[array_index] = I_FRAME;

#ifdef WIN32
assert(p_tmp_buffer_wp + 188  <= p_TsSeqHandle->p_tmp_buffer_end);
assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
MT_ASSERT(p_tmp_buffer_wp + 188  <= p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif

memcpy(p_tmp_buffer_wp,p_temp,188);
p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index] = p_tmp_buffer_wp;

p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
p_TsSeqHandle->tmp_buffer_free_space -= 188;

MY_LOG("one gop is ok!!!!\n");
break;

}
else
{
// errro: you should not come here !!!!
MY_LOG("error error!!!!\n");
#ifdef WIN32
assert(0);
#else
MT_ASSERT(0);
#endif
}
}
//video pes && start unit && P Frame
else if(cur_video_frame_type == P_FRAME)
{
last_video_frame_type = P_FRAME;
if(p_TsSeqHandle->isFindFirstIFrame)
{
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
array_index = p_TsSeqHandle->cur_gop_type.len;
p_TsSeqHandle->cur_gop_type.frame_type_array[array_index] = P_FRAME;
p_TsSeqHandle->cur_gop_type.len++;
#ifdef WIN32
assert(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
MT_ASSERT(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif

#ifdef PLAY_LESS_MEM
p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index]
    = p_tmp_buffer_wp;
#else
memcpy(p_tmp_buffer_wp,p_temp,188);
p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index] = p_tmp_buffer_wp;
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
p_TsSeqHandle->tmp_buffer_free_space -= 188;
#endif
}
else
{
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;

#ifdef WIN32
assert((p_tmp_buffer_wp + 188)  <=  p_TsSeqHandle->p_tmp_buffer_end);
assert((p_TsSeqHandle->tmp_buffer_free_space) >= 188);
#else
MT_ASSERT((p_tmp_buffer_wp + 188)  <=  p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT((p_TsSeqHandle->tmp_buffer_free_space) >= 188);
#endif

#ifndef PLAY_LESS_MEM
memcpy(p_tmp_buffer_wp,p_temp,188);
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
p_TsSeqHandle->tmp_buffer_free_space -= 188;
#endif
}

}
//video pes && start unit && B frame
else if(cur_video_frame_type == B_FRAME)
{
last_video_frame_type = B_FRAME;
if(p_TsSeqHandle->isFindFirstIFrame)
{
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
array_index = p_TsSeqHandle->cur_gop_type.len;
p_TsSeqHandle->cur_gop_type.frame_type_array[array_index] = B_FRAME;
p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index] = p_tmp_buffer_wp;
p_TsSeqHandle->cur_gop_type.len ++;
#ifdef WIN32
assert(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
MT_ASSERT(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
#ifndef PLAY_LESS_MEM
memcpy(p_tmp_buffer_wp,p_temp,188);
//p_tmp_buffer_wp+=188;
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
p_TsSeqHandle->tmp_buffer_free_space -=  188;
#endif
}
else
{
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;

#ifdef WIN32
assert((p_tmp_buffer_wp + 188)  <=  p_TsSeqHandle->p_tmp_buffer_end);
assert((p_TsSeqHandle->tmp_buffer_free_space) >= 188);
#else
MT_ASSERT((p_tmp_buffer_wp + 188)  <=  p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT((p_TsSeqHandle->tmp_buffer_free_space) >= 188);
#endif

#ifndef PLAY_LESS_MEM
memcpy(p_tmp_buffer_wp , p_temp , 188);
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
p_TsSeqHandle->tmp_buffer_free_space -= 188;
#endif
}


}
//vidoe pes && none start unit
else
{
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;

#ifdef WIN32
assert(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
MT_ASSERT(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif

#ifdef PLAY_LESS_MEM
if(last_video_frame_type == I_FRAME)
{
//only copy I_frm to tmp buf
memcpy(p_tmp_buffer_wp,p_temp,188);
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
p_TsSeqHandle->tmp_buffer_free_space -=188;
}
#else
memcpy(p_tmp_buffer_wp,p_temp,188);
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
p_TsSeqHandle->tmp_buffer_free_space -=188;
#endif
}

}
//none video pes
else
{
p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
if((p_tmp_buffer_wp + 188)  >  p_TsSeqHandle->p_tmp_buffer_end)
{
MY_ERROR("[ERROR] not find GOP in 1 M bytes!!!!!\n");
}
else
{
#ifdef PLAY_LESS_MEM
if(last_video_frame_type == I_FRAME)
{
//only copy I_frm to tmp buf
memcpy(p_tmp_buffer_wp,p_temp,188);
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
p_TsSeqHandle->tmp_buffer_free_space -=188;
}
#else
memcpy(p_tmp_buffer_wp , p_temp , 188);
p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
p_TsSeqHandle->tmp_buffer_free_space -= 188;
#endif
}
}
}

if(packet_index == packet_cnt)
{
p_TsSeqHandle->left_data_len = 0;
}
else//packet_index != packet_cnt
{
if(processLeftData == FALSE)
{
p_TsSeqHandle->left_data_len = p_TsSeqHandle->data_len - packet_index * 188;
}
}


return 0;

}

static int avs_x_split_gop_SPECIAL(ts_seq_t * p_TsSeqHandle ,ts_seq_play_mode_t play_mode)
{
video_frame_t cur_video_frame_type;
int index = 0;
int i = 0;
int framelen = 0;
int goplen = 0;
int IframeNum = 0;
int BframeNum = 0;
int PframeNum = 0;
int cpINum = 0;
int cpBNum = 0 ;
int cpPNum = 0;
int cpIMax = 0;
int cpBMax = 0;
int cpPMax = 0;
int cpMax = 0;
int dropMask = 0;
int dropNum = 0;
int restframe = 0;
int curgopnum = p_TsSeqHandle->gop_num - 1;

goplen = p_TsSeqHandle->cur_gop_type.len;

//find the first i frame,normal is index 0
for(i = 0 ; i < goplen ; i++)
{
cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[i];
if(cur_video_frame_type == I_FRAME)
{
index = i;
IframeNum ++;
}
else if(cur_video_frame_type == B_FRAME)
{
BframeNum ++;
}
else if(cur_video_frame_type == P_FRAME)
{
PframeNum ++;
}
}

cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

//if drop num is 0, that means no need to drop the current GOP
/*
if(p_TsSeqHandle->drop_segment_num == 0)
{
p_TsSeqHandle->dropOneGop = FALSE;
}
*/
/* I PP PP PP PP P*/
/*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
else goplen > 12 ,then drop rest and the next 3 gops,*/

switch(play_mode)
{
case TS_SEQ_FAST_PLAY_2X:
case TS_SEQ_REV_FAST_PLAY_2X:
dropMask = 2;
break;
case TS_SEQ_FAST_PLAY_4X:
case TS_SEQ_REV_FAST_PLAY_4X:
dropMask = 4;
break;
case TS_SEQ_FAST_PLAY_8X:
case TS_SEQ_REV_FAST_PLAY_8X:
dropMask = 8;
break;
case TS_SEQ_FAST_PLAY_16X:
case TS_SEQ_REV_FAST_PLAY_16X:
dropMask = 16;
break;
case TS_SEQ_FAST_PLAY_32X:
case TS_SEQ_REV_FAST_PLAY_32X:
dropMask = 32;
break;
default:
dropMask = 1;
break;
}

if(goplen > p_TsSeqHandle->drop_segment_num)
{
//do process in this gop
restframe = goplen - p_TsSeqHandle->drop_segment_num;
if(dropMask >=  restframe)
{
p_TsSeqHandle->drop_segment_num = dropMask - restframe;
cpMax = 1;
cpIMax = 1;
cpBMax = 0;
cpPMax = 0;
}
else
{
cpMax = restframe / dropMask;
dropNum = restframe % dropMask;
if(dropNum == 0)
{
dropNum = dropMask;
}
else
{
cpMax += 1; //when restframe big than dropMask,copy one more!
}

p_TsSeqHandle->drop_segment_num = dropMask - dropNum;

cpIMax = 1;
cpPMax = cpMax - cpIMax;
if(cpPMax > PframeNum)
{
cpBMax = cpPMax - PframeNum;
cpPMax = PframeNum;
}
else
{
cpBMax = 0;
}
}

}
else
{
//the drop total gop
p_TsSeqHandle->drop_segment_num -= goplen;
cpMax = 0;
cpIMax = 0;
cpBMax = 0;
cpPMax = 0;
}

for(i = 0 ; i < goplen ; i++)
{
//do data proccess
//write ()
//skip the b frame
if(cur_video_frame_type == B_FRAME)
{
if(cpBNum < cpBMax)
{
#ifdef PLAY_LESS_MEM
                //fwrite(p_I_frame,1,I_framelen,pfileOut);
                //copy the last I frame
                //write_fifo_kw(p_TsSeqHandle, p_I_frame,I_framelen);
                p_TsSeqHandle->bpframe_cpnum[curgopnum] ++;
#else
framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
 - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
//fwrite(p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],1,framelen,pfileOut);
//write_fifo_kw(p_TsSeqHandle
// , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
#endif
cpBNum ++;
}

}
else if(cur_video_frame_type == P_FRAME)
{

if(cpPNum < cpPMax)
{
#ifdef PLAY_LESS_MEM
                //fwrite(p_I_frame,1,I_framelen,pfileOut);
                //copy the last I frame
                //write_fifo_kw(p_TsSeqHandle, p_I_frame,I_framelen);
                p_TsSeqHandle->bpframe_cpnum[curgopnum] ++;
#else
framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
 - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
//fwrite(p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],1,framelen,pfileOut);
//write_fifo_kw(p_TsSeqHandle
// , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
#endif
cpPNum ++;
}

}
else{
//i frame
if(cpINum < cpIMax)
{
framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
 - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
//fwrite(p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],1,framelen,pfileOut);
//write_fifo_kw(p_TsSeqHandle
// , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
cpINum ++;
#ifdef PLAY_LESS_MEM
                p_TsSeqHandle->i_frame_len[curgopnum] = framelen;
#endif
}
}

index ++;
cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

//end when the second i frame

}

// TODO:
//MY_DEBUG("x_split_gop_SPECIAL: !!!!\n");
return 0;
}

static int totalGOP = 0;

static void   avs_print_cur_gop(ts_seq_t * p_TsSeqHandle)
{
#ifdef DEBUG_TS_SEQUENCE
int i = 0;
totalGOP++;
MY_DEBUG("totalGOP:%d\n",totalGOP);
MY_DEBUG("GOP len:%d\n",p_TsSeqHandle->cur_gop_type.len);

for(i = 0; i < p_TsSeqHandle->cur_gop_type.len; i++)
{
switch(p_TsSeqHandle->cur_gop_type.frame_type_array[i])
{
case I_FRAME:
MY_DEBUG("I--");
break;
case B_FRAME:
MY_DEBUG("B--");
break;
case P_FRAME:
MY_DEBUG("P--");
break;
default:
MY_DEBUG("NONO");
break;
}
}
MY_DEBUG("\n\n");
#endif

}

static int avs_resort_packetstart_array(char  * p_array[MAX_GOP_LEN] , int array_len)
{
char  *p_bak_array[MAX_GOP_LEN] = {NULL};
int i = 0;
if(array_len < 1)
{
return 0;
}

memcpy((char  *)p_bak_array, p_array , array_len * sizeof(char *));
memset((char  *)p_array, 0 ,(array_len * sizeof(char *)));

for(i = 0; i < array_len; i++)
{
p_array[i] = p_bak_array[array_len - i - 1];
}
return 0;
}


static int avs_resort_frametype_array(video_frame_t *p_array,int array_len)
{
video_frame_t p_bak_array[MAX_GOP_LEN] ={INVALID_FRAME_TYPE};
int i = 0;

if(array_len < 1)
{
return 0;
}

memcpy(p_bak_array, p_array,array_len * sizeof(video_frame_t));
memset(p_array, 0, (array_len * sizeof(video_frame_t)));

for(i = 0;i < array_len; i++)
{
p_array[i] = p_bak_array[array_len - i - 1];
//log_printf("bbb p_array[%d]:%d\n",i,p_array[i]);
}

return 0;

}

 int avs_ts_backward_gop_parser(ts_seq_t * p_TsSeqHandle)
{
int i = 0;
MT_BOOL isFindFirstKeyFrame  =   FALSE;
video_frame_t *p_frame_type_array = NULL;

do
{
avs_do_revert_parse_gop(p_TsSeqHandle);

isFindFirstKeyFrame = p_TsSeqHandle->isFindFirstIFrame;
if(isFindFirstKeyFrame)
{
avs_resort_frametype_array(p_TsSeqHandle->cur_gop_type.frame_type_array,
   p_TsSeqHandle->cur_gop_type.len);
avs_resort_packetstart_array(p_TsSeqHandle->cur_gop_type.video_packet_start_array
   , p_TsSeqHandle->cur_gop_type.len);
p_TsSeqHandle->isFindFirstIFrame = FALSE;

avs_print_cur_gop(p_TsSeqHandle);

p_TsSeqHandle->gop_num ++;

avs_x_split_gop_SPECIAL(p_TsSeqHandle,p_TsSeqHandle->play_mode);

p_TsSeqHandle->p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_start - 187;
p_TsSeqHandle->cur_gop_type.len = 0;
p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;

p_frame_type_array = NULL;
p_frame_type_array = p_TsSeqHandle->cur_gop_type.frame_type_array;
memset(p_frame_type_array,0,MAX_GOP_LEN * sizeof(video_frame_t));

for(i = 0; i < MAX_GOP_LEN; i++)
{
p_TsSeqHandle->cur_gop_type.video_packet_start_array[i] = NULL;
}

}

}
while(p_TsSeqHandle->left_data_len);

return 0;
}

  int  avs_ts_forward_gop_parser(ts_seq_t * p_TsSeqHandle)
{
int second_keyframe_index = 0;
char  *p_second_key_frame = NULL;
int i = 0;
video_frame_t *p_frame_type_array = NULL;
MT_BOOL isFindFirstKeyFrame  =   FALSE;
MT_BOOL isFindSecondKeyFrame =  FALSE;

do
{
avs_do_parse_gop(p_TsSeqHandle);

if(p_TsSeqHandle->isExit)
{
break;
}


isFindFirstKeyFrame = p_TsSeqHandle->isFindFirstIFrame;
isFindSecondKeyFrame = p_TsSeqHandle->isFindSecondIFrame;


//find two key frame
if(isFindFirstKeyFrame && isFindSecondKeyFrame)
{
//do nothing before these is no one total gop in tmp buffer

avs_print_cur_gop(p_TsSeqHandle);

p_TsSeqHandle->gop_num ++;

avs_x_split_gop_SPECIAL(p_TsSeqHandle,p_TsSeqHandle->play_mode);

//reset field of p_TsSeqHandle
p_TsSeqHandle->isFindFirstIFrame = FALSE;
p_TsSeqHandle->isFindSecondIFrame = FALSE;
//print_cur_gop();

//the second key frame became the first key frame of current gop
second_keyframe_index = p_TsSeqHandle->cur_gop_type.len;
p_second_key_frame = p_TsSeqHandle->cur_gop_type.video_packet_start_array[second_keyframe_index];
for(i = 0; i < MAX_GOP_LEN; i++)
{
p_TsSeqHandle->cur_gop_type.video_packet_start_array[i] = NULL;
}


memcpy(p_TsSeqHandle->p_tmp_buffer_start,p_second_key_frame,188);
p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len - 188;
p_TsSeqHandle->p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_start + 188;
p_TsSeqHandle->cur_gop_type.len = 0;
p_TsSeqHandle->cur_gop_type.video_packet_start_array[0] = p_TsSeqHandle->p_tmp_buffer_start;

p_frame_type_array = NULL;
p_frame_type_array = p_TsSeqHandle->cur_gop_type.frame_type_array;
memset(p_frame_type_array, 0 ,MAX_GOP_LEN * sizeof(video_frame_t));
p_TsSeqHandle->cur_gop_type.frame_type_array[0] =  I_FRAME;

p_TsSeqHandle->cur_gop_type.len++;
p_TsSeqHandle->isFindFirstIFrame = TRUE;


totalGOP++;
MY_DEBUG("p_TsSeqHandle->tmp_buffer_start:%p\n",p_TsSeqHandle->p_tmp_buffer_start);
MY_DEBUG("p_TsSeqHandle->tmp_buffer_wp:%p\n",p_TsSeqHandle->p_tmp_buffer_wp);
MY_DEBUG("p_TsSeqHandle->tmp_buffer_end:%p\n",p_TsSeqHandle->p_tmp_buffer_end);
MY_DEBUG("p_TsSeqHandle->tmp_buffer_free_space:%d\n",p_TsSeqHandle->tmp_buffer_free_space);
MY_DEBUG("totalGOP:%d\n",totalGOP);


}

}
while(p_TsSeqHandle->left_data_len  && p_TsSeqHandle->isExit == FALSE);

return 0;

}



