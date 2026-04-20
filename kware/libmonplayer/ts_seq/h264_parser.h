/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef  __H264_PARSER_H__
#define  __H264_PARSER_H__
/*!
  xxx
*/
int ts_backward_sequence_parser(ts_seq_t * p_TsSeqHandle, ts_detail_info_t *p_ts_detail);
/*!
  xxx
  */
int  ts_forward_sequence_parser(ts_seq_t * p_TsSeqHandle, ts_detail_info_t *p_ts_detail);

slice_t  x_check_slice_type3(char *p_ebsp,int start_pos,int end_pos,
    int *frame_mbs_only,int *field_flag, int *bottom_field_flag, int *frame_num);

slice_t  x_check_spsframe(char * p_ebsp,int start_pos,int end_pos,int *frame_mbs_only);

int  x_check_recovery_point(char *p_ebsp,int start_pos,int end_pos);

unsigned int  GetUeValue(unsigned char *pBuff, int  nLen, int * p_nStartBit);
int x_check_get_bits (unsigned char buffer[],int totbitoffset,int *info, 
							int bytecount, int numbits);
int x_check_get_ue (unsigned char buffer[],int totbitoffset,
				int *info, int bytecount);
int x_check_get_se(unsigned char buffer[],int totbitoffset,int *info, int bytecount);
void x_check_Scaling_List(int *scalingList, int sizeOfScalingList, 
    char *UseDefaultScalingMatrix, unsigned char *p_tmp_buf,int end_pos);

slice_t  x_check_slice_type2(char *p_ebsp,int start_pos,int end_pos,
    int *frame_mbs_only,int *filed_flag, int *bottom_field_flag);
slice_t  x_check_slice_type1(char *p_ebsp,int start_pos,int end_pos,
    int *frame_mbs_only,int *filed_flag);
slice_t  x_check_slice_type(char * p_ebsp,int start_pos,int end_pos);

nalu_t  x_check_nalu_type2(char * p_ts_packet, char *head_frame_type, 
                                 int len/*188*/, slice_t * p_slice_type);
nalu_t  x_check_nalu_type1(char * p_ts_packet , int len/*188*/,
								slice_t * p_slice_type);
nalu_t  x_check_nalu_type(char * p_ts_packet , int len/*188*/,
								slice_t * p_slice_type);
int do_parse_h264_sequence1(void * p_Handle);


#endif

