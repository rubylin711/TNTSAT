/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MPEG_PARSER_H__
#define __MPEG_PARSER_H__

/*!
  xxx
  */
 int ts_backward_gop_parser(ts_seq_t * p_TsSeqHandle, ts_detail_info_t *p_ts_detail);
/*!
   xxx
  */
 int  ts_forward_gop_parser(ts_seq_t * p_TsSeqHandle, ts_detail_info_t *p_ts_detail);

video_frame_t  x_check_frame_type(char * p_ts_packet , int len/*188*/);
int ts_backward_gop_parser(ts_seq_t * p_TsSeqHandle,
									ts_detail_info_t *p_ts_detail);
int  ts_forward_gop_parser(ts_seq_t * p_TsSeqHandle,
									ts_detail_info_t *p_ts_detail);


#endif

