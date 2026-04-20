/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __AVS_PARSER_H__
#define __AVS_PARSER_H__


/*
   **************
  forward fastplay function
   **************
*/
int  avs_ts_forward_gop_parser(ts_seq_t * p_TsSeqHandle);

/*
   **************
  backward fastplay function
   **************
*/
int avs_ts_backward_gop_parser(ts_seq_t * p_TsSeqHandle);

char * avs_pes_packet_start(char * p_data, unsigned long * p_payload);
char * avs_pes_data_start(char * p_data, unsigned long * p_payload);
#endif
