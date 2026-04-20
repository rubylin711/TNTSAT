/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifdef WIN32
#include <windows.h>
#include <assert.h>
#endif
#include "string.h"
#include "mt_type.h"
//#include "sys_define.h"
//#include "sys_cfg.h"
#include "drv_adp.h"
#if 1
#include "mtos_mem.h"
#include "mtos_sem.h"
#include "mtos_printk.h"
#include "mtos_task.h"
#endif

#include "fifo_kw.h"
#include "ts_sequence.h"
#include "h264_parser.h"


#define MT_ASSERT

//#define DEBUG_H264_PARSER
//#define PKT_STATSTICS

//#define my_printf mtos_printk
//#define OS_PRINTF mtos_printk


#ifdef   DEBUG_H264_PARSER
/*!
  xxx
 */
typedef char *va_list;
/*!
  xxx
 */
#define VA_START(ap, p) (ap = (char *)(&(p)+1))
/*!
  xxx
 */
#define VA_ARG(ap, type)    ((type *) (ap += sizeof(type)))[-1]
/*!
  xxx
 */
#define VA_END(ap)

static int my_printf(const char *fmt, ...)
{
    //ar buffer[1024] = {0};
    va_list arg;
    VA_START (arg, fmt);
    //printf(buffer, fmt, arg);
    VA_END (arg);
    //tputDebugStringA(buffer);
    return 0;
}
#else
static int my_printf(const char *fmt, ...)
{
    return 0;
}
#endif



#define  MY_LOG        printf
#define  MY_DEBUG    printf
#define  MY_ERROR    printf


#define MULTIPLY_SLICE 0xff
//Start code and Emulation Prevention need this to be defined in
//identical manner at encoder and decoder
#define  ZEROBYTES_SHORTSTARTCODE 2 
//indicates the number of zero bytes in the short start-code prefix
#define  INTERNAL_FIFO_SIZE   (3*1024*1024)
#define   FINE_GRAINED_INTERVAL   (2)
#define   COARSE_GRAINDED_INTERVAL (4)
#define   OFFSET_ONE   (1)
#define   OFFSET_TWO  (2)
#define   OFFSET_THREE  (3)
/*!
  ts packet id
 */
#define TS_PID(data) \
    ((unsigned short)(((unsigned char)(data[2])) | \
        (((unsigned short)((unsigned char)(data[1] & 0x1F))) << 8)))

/*!
  payload unit  start
 */
#define PAYLOAD_UNIT_START(data) ((data[1] & 0x40) == 0x40)

/*!
  video pes header
 */
#define V_PES_HEADER(data)\
    (((((char *)data)[0] == 0) && (((char *)data)[1] == 0) && (((char *)data)[2] == 0x01)) && \
     ((((unsigned char *)data)[3] >= 0xE0) && (((unsigned char *)data)[3] <= 0xEF)))

/*!
  video pre start code
 */
#define IDENTIFY_NALU_STARTCODE(x) \
    ((((unsigned char)(x)[0] == 0x00)) && (((unsigned char)(x)[1]) == 0x00) && \
     (((unsigned char)(x)[2]) == 0x01))
#if 0
/*!
  xxx
 */
typedef  struct{
    /*!
      xxx
     */
    ts_seq_play_mode_t  mode;
    /*!
      xxx
     */
    int      split_factor;
}playmode_to_index_t;

static  playmode_to_index_t  PLAYMODE_TO_INDEX_TABLE[] = {
    {TS_SEQ_FAST_PLAY_2X, 2},
    {TS_SEQ_REV_FAST_PLAY_2X, 2},
    {TS_SEQ_FAST_PLAY_4X, 4},
    {TS_SEQ_REV_FAST_PLAY_4X, 4},
    {TS_SEQ_FAST_PLAY_8X, 8},
    {TS_SEQ_REV_FAST_PLAY_8X, 8},
    {TS_SEQ_FAST_PLAY_16X, 16},
    {TS_SEQ_REV_FAST_PLAY_16X, 16},

    {TS_SEQ_FAST_PLAY_32X, 32},
    {TS_SEQ_REV_FAST_PLAY_32X, 32},
};
#endif
static char * pes_packet_start(char * p_data, unsigned long * p_payload)
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
            return (p_data + adaptation_field_length);
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
static char * pes_data_start(char * p_data, unsigned long * p_payload)
{
    unsigned char pes_header_length = 0;

    pes_header_length = p_data[8];
    pes_header_length += 9;
    (*p_payload) -= pes_header_length;

    return (char *)(p_data + pes_header_length);
}



static void print_nalu_type(nalu_t  type)
{
#ifdef DEBUG_H264_PARSER
    switch(type)
    {
        case NONE_USE:
            OS_PRINTF("NONE_USE  ");
            break;
        case NONE_IDR:
            OS_PRINTF("NONE_IDR ");
            break;
        case PARTITION_DATA_A:
            OS_PRINTF("PARTITION_DATA_A  ");
            break;
        case PARTITION_DATA_B:
            OS_PRINTF("PARTITION_DATA_B  ");
            break;
        case PARTITION_DATA_C:
            OS_PRINTF("PARTITION_DATA_C  ");
            break;
        case IDR_TYPE:
            OS_PRINTF("@@@@ IDR_TYPE @@@\n");
            break;
        case SEI_TYPE:
            OS_PRINTF("SEI_TYPE  ");
            break;
        case SPS_TYPE:
            OS_PRINTF("\nSPS_TYPE\n");
            break;
        case PPS_TYPE:
            OS_PRINTF("PPS_TYPE\n");
            break;
        case SPLIT_NALU_TYPE:
            OS_PRINTF("SPLIT_TYPE  ");
            break;
        case END_SEQ_TYPE:
            OS_PRINTF("END_SEQ_TYPE\n");
            break;
        case END_TS_TYPE:
            OS_PRINTF("END_TS_TYPE\n");
            break;
        case PADDING_TYPE:
            OS_PRINTF("PADDING_TYPE\n");
            break;
        case RESEVED_START_TYPE:
            OS_PRINTF("RESEVED_START_TYPE\n");
            break;
        case RESEVED_END_TYPE:
            OS_PRINTF("RESEVED_END_TYPE\n");
            break;
        case INVALID_NALU_TYPE:
            OS_PRINTF("INVALID_NALU_TYPE\n");
            break;

        default:
            OS_PRINTF("INVALID_NALU_TYPE\n");
            break;
    }
#endif
}


static void print_slice_type(slice_t type)
{
#ifdef DEBUG_H264_PARSER
    switch(type)
    {
        case P_SLICE:
            OS_PRINTF("(P)  ");
            break;
        case B_SLICE:
            OS_PRINTF("(B)  ");
            break;
        case I_SLICE:
            OS_PRINTF("(I)   ");
            break;
        case SP_SLICE:
            OS_PRINTF("(SP)  ");
            break;
        case SI_SLICE:
            OS_PRINTF("(SI) ");
            break;
        case P_SLICE_2:
            OS_PRINTF("(P2)   ");
            break;
        case B_SLICE_2:
            OS_PRINTF("(B2)   ");
            break;
        case I_SLICE_2:
            OS_PRINTF("(I2)   ");
            break;
        case SP_SLICE_2:
            OS_PRINTF("(SP2)   ");
            break;
        case SI_SLICE_2:
            OS_PRINTF("(SI2)   ");
            break;


        default:
            OS_PRINTF("INVALID SLICE  ");
            break;
    }
#endif
}
static int EBSPtoRBSP(unsigned char *streamBuffer, int end_bytepos, int begin_bytepos)
{
    int i = 0;
    int j = 0;
    int count = 0;

    if(end_bytepos < begin_bytepos)
    {
        return end_bytepos;
    }

    j = begin_bytepos;

    for(i = begin_bytepos; i < end_bytepos; i++)
    { //starting from begin_bytepos to avoid header information
        //in NAL unit, 0x000000, 0x000001 or 0x000002 shall not occur at any byte-aligned position
        if(count == ZEROBYTES_SHORTSTARTCODE && streamBuffer[i] < 0x03) 
        {
            return -1;
        }

        if(count == ZEROBYTES_SHORTSTARTCODE && streamBuffer[i] == 0x03)
        {
            if((i < end_bytepos - 1) && (streamBuffer[i + 1] > 0x03))
            {
                return -1; 
            }

            if(i == end_bytepos - 1)
            {
                return j;
            }

            i++;
            count = 0;
        }

        streamBuffer[j] = streamBuffer[i];

        if(streamBuffer[i] == 0x00)
        {
            count++;
        }
        else
        {
            count = 0;
        }

        j++;

    }

    return j;

}

//static int g_cur_video_pid = 101;
#if 0
static int resort_packetstart_array(char  * p_array[MAX_SEQUENCE_LEN] , int array_len)
{
    char  *p_bak_array[MAX_SEQUENCE_LEN] = {NULL};
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


static int resort_slicetype_array(video_frame_t *p_array,int array_len)
{
    //video_frame_t p_bak_array[MAX_GOP_LEN] ={INVALID_FRAME_TYPE};
    slice_t p_bak_array[MAX_SEQUENCE_LEN] ={INVALID_SLICE_TYPE};
    int i = 0;

    if(array_len < 1)
    {
        return 0;
    }

    memcpy(p_bak_array, p_array,array_len * sizeof(slice_t));
    memset(p_array, 0, (array_len * sizeof(slice_t)));

    for(i = 0;i < array_len; i++)
    {
        p_array[i] = p_bak_array[array_len - i - 1];
        //log_printf("bbb p_array[%d]:%d\n",i,p_array[i]);
    }

    return 0;

}
#endif
/*
   !************
 * name:     GetUeValue
 * pBuff :    point to bitstream which will be decoded
 * nlen:      len fo bitstram
 * p_nStartBit: 
 [IN]:      *p_nStartBit :  the first decoding bit offset  from the  the first bit of pBuff[0]
 [OUT]:   *p_nStartBit :  the next decoding bit  offset from the the first bit of pBuff[0]
return:  the codeNum
 */

unsigned int  GetUeValue(unsigned char *pBuff, int  nLen, int * p_nStartBit)
{
    unsigned int nZeroNum = 0;
    int nStartBit = *p_nStartBit;
    int dwRet = 0;
    int i = 0;

    while (nStartBit < nLen * 8)
    {
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        {
            break;
        }
        nZeroNum++;
        nStartBit++;
    }
    nStartBit ++;
    for (i = 0; i < nZeroNum; i++)
    {
        dwRet <<= 1;
        if(pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        {
            dwRet += 1;
        }
        nStartBit++;
    }

    *p_nStartBit = nStartBit;

    return (1 << nZeroNum) - 1 + dwRet;

}
int x_check_get_bits (unsigned char buffer[],int totbitoffset,int *info, 
							int bytecount, int numbits)
{
    int inf = 0;
    int byteoffset = 0;      // byte from start of buffer
    int bitoffset = 0;      // bit from start of byte

    int bitcounter = numbits;

    byteoffset = totbitoffset / 8;
    bitoffset = 7 - (totbitoffset % 8);

    while(numbits)
    {
        inf <<= 1;
        inf |= (buffer[byteoffset] & (0x01 << bitoffset)) >> bitoffset;
        numbits--;
        bitoffset--;
        if (bitoffset < 0)
        {
            byteoffset++;
            bitoffset += 8;
            if (byteoffset > bytecount)
            {
                return -1;
            }
        }
    }

    *info = inf;
    return bitcounter;           // return absolute offset in bit from start of frame
}     
int x_check_get_ue (unsigned char buffer[],int totbitoffset,int *info, int bytecount)
{
    int inf = 0;
    long byteoffset = 0;      // byte from start of buffer
    int bitoffset = 0;      // bit from start of byte
    int ctr_bit = 0;      // control bit for current bit posision
    int bitcounter = 1;
    int len = 1;
    int info_bit = 0;

    byteoffset = totbitoffset >> 3;
    bitoffset = 7 - (totbitoffset & 7);
    ctr_bit = (buffer[byteoffset] & (0x01 << bitoffset));   // set up control bit

    while (ctr_bit == 0)
    {   
        // find leading 1 bit
        len++;
        bitoffset -= 1;           
        bitcounter++;
        if (bitoffset < 0)
        {                 
            // finish with current byte ?
            bitoffset = bitoffset + 8;
            byteoffset++;
        }
        ctr_bit = buffer[byteoffset] & (0x01 << (bitoffset));
    }
    // make infoword
    inf = 0;                          // shortest possible code is 1, then info is always 0
    for(info_bit = 0; info_bit < (len - 1); info_bit++)
    {
        bitcounter++;
        bitoffset -= 1;
        if(bitoffset < 0)
        {                 
            // finished with current byte ?
            bitoffset = bitoffset + 8;
            byteoffset++;
        }
        if (byteoffset > bytecount)
        {
            return -1;
        }
        inf = (inf << 1);
        if(buffer[byteoffset] & (0x01 << bitoffset))
        inf |= 1;
    }
    *info = (int)(1 << (bitcounter >> 1)) + inf - 1;
    
    return bitcounter;           
}     
int x_check_get_se(unsigned char buffer[],int totbitoffset,int *info, int bytecount)
{
    int inf = 0;
    long byteoffset = 0;      // byte from start of buffer
    int bitoffset = 0;      // bit from start of byte
    int ctr_bit = 0;      // control bit for current bit posision
    int bitcounter = 1;
    int len = 1;
    int info_bit = 0;
  
    byteoffset = totbitoffset >> 3;
    bitoffset = 7 - (totbitoffset & 7);
    ctr_bit = (buffer[byteoffset] & (0x01 << bitoffset));   // set up control bit
  
    while (ctr_bit == 0)
    {                 // find leading 1 bit
        len++;
        bitoffset -= 1;           
        bitcounter++;
        if(bitoffset < 0)
        {                 // finish with current byte ?
            bitoffset = bitoffset + 8;
            byteoffset++;
        }
        ctr_bit = buffer[byteoffset] & (0x01 << (bitoffset));
    }
    // make infoword
    inf = 0;                          // shortest possible code is 1, then info is always 0
    for(info_bit = 0;(info_bit < (len - 1)); info_bit++)
    {
        bitcounter++;
        bitoffset -= 1;
        if(bitoffset < 0)
        {                 
            // finished with current byte ?
            bitoffset = bitoffset + 8;
            byteoffset++;
        }
        if(byteoffset > bytecount)
        {
            return -1;
        }
        inf = (inf << 1);
        if(buffer[byteoffset] & (0x01 << bitoffset))
            inf |= 1;
    }
    inf = (int)(1 << (bitcounter >> 1)) + inf - 1;
    *info = (inf + 1) / 2;
    if((inf & 0x01) == 0)                           // lsb is signed bit
        *info = -*info;
    return bitcounter;           
}     
char x_check_ZZ_SCAN[16]  =
{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

char x_check_ZZ_SCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};
int x_check_ScalingList4x4[6][16];                             
int x_check_ScalingList8x8[2][64];                             
char    x_check_UseDefaultScalingMatrix4x4Flag[6];
char    x_check_UseDefaultScalingMatrix8x8Flag[2];
int x_bitoffset = 0;
int x_check_log2maxframe = 0;
void x_check_Scaling_List(int *scalingList, int sizeOfScalingList, 
    char *UseDefaultScalingMatrix, unsigned char *p_tmp_buf,int end_pos)
{
    int j  = 0;
    int scanj = 0;
    int delta_scale = 0;
    int lastScale = 8;
    int nextScale = 8;
  
    for(j = 0; j < sizeOfScalingList; j++)
    {
        scanj = (sizeOfScalingList == 16) ? x_check_ZZ_SCAN[j] : x_check_ZZ_SCAN8[j];
    
        if(nextScale != 0)
        {
            x_bitoffset += x_check_get_se(p_tmp_buf,x_bitoffset,&delta_scale,end_pos);
            nextScale = (lastScale + delta_scale + 256) % 256;
            *UseDefaultScalingMatrix = (scanj == 0 && nextScale == 0);
        }
    
        scalingList[scanj] = (nextScale == 0) ? lastScale : nextScale;
        lastScale = scalingList[scanj];
    }
}
slice_t  x_check_spsframe(char * p_ebsp,int start_pos,int end_pos,int *frame_mbs_only)
{
    int type = 0;
    int temp = 0;
    int value =0;
    int i = 0;
    unsigned char profile_idc = 0;
    unsigned char *p_tmp_buf = (unsigned char *)p_ebsp;

    EBSPtoRBSP(p_ebsp,start_pos,end_pos);
    x_bitoffset = 0;
    *frame_mbs_only = 0;
    value = end_pos - start_pos;
    while(value--)
    {
        if((p_tmp_buf[0] == 0) && (p_tmp_buf[1] == 0) && (p_tmp_buf[2] == 1) 
            && ((p_tmp_buf[3] & 0x1f) == SPS_TYPE))
            break;
        p_tmp_buf++;
    }
    p_tmp_buf += 4;
    
    profile_idc = p_tmp_buf[0];
    x_bitoffset = 24;
    x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&value,end_pos);//sps_id
    if((profile_idc == 100) || (profile_idc == 110) 
        || (profile_idc == 122) || (profile_idc == 144))
    {
        x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&value,end_pos);//chroma_idc
        if(value == 3)
            x_bitoffset += x_check_get_bits(p_tmp_buf,x_bitoffset,&value,end_pos,1);//dct_transform
        x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&value,end_pos);
        x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&value,end_pos);
        x_bitoffset += x_check_get_bits(p_tmp_buf,x_bitoffset,&value,end_pos,1);//dct_transform
        x_bitoffset += x_check_get_bits(p_tmp_buf,x_bitoffset,&value,end_pos,1);//seq_matrix
        if(value)
        {
            for(i = 0; i < 8; i++)
            {
                x_bitoffset += x_check_get_bits(p_tmp_buf,x_bitoffset,&value,end_pos,1);
                if(value)
                {
                    if(i < 6)
                    {
                        x_check_Scaling_List(x_check_ScalingList4x4[i], 16, 
                        (char *)&x_check_UseDefaultScalingMatrix4x4Flag[i], p_tmp_buf,end_pos);
                    }
                    else
                    {
                        x_check_Scaling_List(x_check_ScalingList8x8[i - 6], 64, 
                        (char *)&x_check_UseDefaultScalingMatrix8x8Flag[i - 6], p_tmp_buf,end_pos);
                    
                    }
                
                }
            }
        }
    }
    x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&value,end_pos);
    x_check_log2maxframe = value + 4;
    x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&value,end_pos);//poc
    if(value == 0)
    {
        x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&value,end_pos);//poc
    }
    else if(value == 1)
    {
    }
    else
    {
        x_bitoffset += x_check_get_bits(p_tmp_buf,x_bitoffset,&value,end_pos,1);
        x_bitoffset += x_check_get_se(p_tmp_buf,x_bitoffset,&value,end_pos);
        x_bitoffset += x_check_get_se(p_tmp_buf,x_bitoffset,&value,end_pos);
        x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&temp,end_pos);
        for(i = 0;i < temp;i++)
            x_bitoffset += x_check_get_se(p_tmp_buf,x_bitoffset,&value,end_pos);
    }
    x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&value,end_pos);
    x_bitoffset += x_check_get_bits(p_tmp_buf,x_bitoffset,&value,end_pos,1);
    x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&value,end_pos);
    x_bitoffset += x_check_get_ue(p_tmp_buf,x_bitoffset,&value,end_pos);
    x_bitoffset += x_check_get_bits(p_tmp_buf,x_bitoffset,frame_mbs_only,end_pos,1);

    return (slice_t)type;
}

slice_t  x_check_slice_type2(char *p_ebsp,int start_pos,int end_pos,
    int *frame_mbs_only,int *filed_flag, int *bottom_field_flag)
{
    int mbNum = 0;
    int type = 0;
    int value = 0;
    unsigned char *p_tmp = (unsigned char *)p_ebsp;
    EBSPtoRBSP(p_ebsp,start_pos,end_pos);
    x_bitoffset = 0;
    *filed_flag = 0;
    mbNum = end_pos - start_pos;
    while(mbNum--)
    {
        if((p_tmp[0] == 0)&&(p_tmp[1] == 0)&&(p_tmp[2] == 1)&&(((p_tmp[3] & 0x1f) == 1)
            ||((p_tmp[3] & 0x1f) == 5)))
            break;
        p_tmp ++;
    }
    p_tmp += 4;
    x_bitoffset += x_check_get_ue(p_tmp,x_bitoffset,&mbNum,end_pos - start_pos);
    if(!mbNum)
        x_bitoffset += x_check_get_ue(p_tmp,x_bitoffset,&type,end_pos - start_pos);
    else{
        type = MULTIPLY_SLICE;
        return (slice_t)type;
    }
    x_bitoffset += x_check_get_ue(p_tmp,x_bitoffset,&mbNum,end_pos - start_pos);
    x_bitoffset += x_check_get_bits(p_tmp,x_bitoffset,&value,end_pos,x_check_log2maxframe);
    if(!*frame_mbs_only)
        x_bitoffset += x_check_get_bits(p_tmp,x_bitoffset,filed_flag,end_pos,1);
    if(*filed_flag)
        x_bitoffset += x_check_get_bits(p_tmp,x_bitoffset,bottom_field_flag,end_pos,1);

    return (slice_t)type;

}


slice_t  x_check_slice_type1(char *p_ebsp,int start_pos,int end_pos,
    int *frame_mbs_only,int *filed_flag)
{
    int mbNum = 0;
    int type = 0;
    int value = 0;
    unsigned char *p_tmp = (unsigned char *)p_ebsp;
    EBSPtoRBSP(p_ebsp,start_pos,end_pos);
    x_bitoffset = 0;
    *filed_flag = 0;
    mbNum = end_pos - start_pos;
    while(mbNum--)
    {
        if((p_tmp[0] == 0)&&(p_tmp[1] == 0)&&(p_tmp[2] == 1)&&(((p_tmp[3] & 0x1f) == 1)
            ||((p_tmp[3] & 0x1f) == 5)))
            break;
        p_tmp ++;
    }
    p_tmp += 4;
    x_bitoffset += x_check_get_ue(p_tmp,x_bitoffset,&mbNum,end_pos - start_pos);
    if(!mbNum)
    {
        x_bitoffset += x_check_get_ue(p_tmp,x_bitoffset,&type,end_pos - start_pos);
        if(type > 4)
            type -= 5;
    }
    else
        type = INVALID_SLICE_TYPE;
    x_bitoffset += x_check_get_ue(p_tmp,x_bitoffset,&mbNum,end_pos - start_pos);
    x_bitoffset += x_check_get_bits(p_tmp,x_bitoffset,&value,end_pos,x_check_log2maxframe);
    if(!*frame_mbs_only)
        x_bitoffset += x_check_get_bits(p_tmp,x_bitoffset,filed_flag,end_pos,1);
    return (slice_t)type;

}

slice_t  x_check_slice_type(char * p_ebsp,int start_pos,int end_pos)
{
    int mbNum = 0;
    int type = 0;
    int startBit = 0;

    EBSPtoRBSP(p_ebsp,start_pos,end_pos);

    //I think the two fields 
    mbNum = GetUeValue(p_ebsp,16, &startBit);
    if(!mbNum)
    {
        mbNum = !(p_ebsp[0] & 0x80);
    }
    // my_printf("[x_check_slice_type]  first_mb_in_slice:%d  startBit:%d\n",mbNum,startBit);
    if(!mbNum)
    {
        type = GetUeValue(p_ebsp,16, &startBit);
        if(type > 4)
            type -= 5;
    }
    else
    {
        type = INVALID_SLICE_TYPE;
    }
    // my_printf("[x_check_slice_type]  type:%d startBit:%d\n",type,startBit);
    print_slice_type(type);

    return (slice_t)type;

}

//jqw@20180928 for trickplay
slice_t  x_check_slice_type3(char *p_ebsp,int start_pos,int end_pos,
    int *frame_mbs_only,int *field_flag, int *bottom_field_flag, int *frame_num)
{
    int mbNum = 0;
    int type = 0;
//    int value = 0;
    unsigned char *p_tmp = (unsigned char *)p_ebsp;
    EBSPtoRBSP(p_ebsp,start_pos,end_pos);
    x_bitoffset = 0;
    *field_flag = 0;
    mbNum = end_pos - start_pos;
    while(mbNum--)
    {
        if((p_tmp[0] == 0)&&(p_tmp[1] == 0)&&(p_tmp[2] == 1)&&(((p_tmp[3] & 0x1f) == 1)
            ||((p_tmp[3] & 0x1f) == 5)))
            break;
        p_tmp ++;
    }
    p_tmp += 4;
    x_bitoffset += x_check_get_ue(p_tmp,x_bitoffset,&mbNum,end_pos - start_pos);
    if(!mbNum)
        x_bitoffset += x_check_get_ue(p_tmp,x_bitoffset,&type,end_pos - start_pos);
    else{
        type = MULTIPLY_SLICE;
        return (slice_t)type;
    }
    x_bitoffset += x_check_get_ue(p_tmp,x_bitoffset,&mbNum,end_pos - start_pos);
    x_bitoffset += x_check_get_bits(p_tmp,x_bitoffset,frame_num,end_pos,x_check_log2maxframe);
    if(!*frame_mbs_only)
        x_bitoffset += x_check_get_bits(p_tmp,x_bitoffset,field_flag,end_pos,1);
    if(*field_flag)
        x_bitoffset += x_check_get_bits(p_tmp,x_bitoffset,bottom_field_flag,end_pos,1);

    return (slice_t)type;

}


//jqw@20181022 for trickplay
int  x_check_recovery_point(char *p_ebsp,int start_pos,int end_pos)
{
    int byte = 0;
    int value = 0;
	int payloadType = 0;
    int payloadSize = 0;
	int recovery_point_flag = 0;
    
    unsigned char *p_tmp = (unsigned char *)p_ebsp;
    EBSPtoRBSP(p_ebsp,start_pos,end_pos);
    x_bitoffset = 0;

    byte = end_pos - start_pos;
#if 0	
	OS_PRINTF("\r\n ~~~sei:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x", 
		p_tmp[0], p_tmp[1],p_tmp[2],p_tmp[3],p_tmp[4],p_tmp[5], p_tmp[6],p_tmp[7],p_tmp[8],p_tmp[9],
		p_tmp[10], p_tmp[11],p_tmp[12],p_tmp[13],p_tmp[14]);
#endif
    while(byte--)
    {
        if((p_tmp[0] == 0) && (p_tmp[1] == 0) && (p_tmp[2] == 1) 
            && ((p_tmp[3] & 0x1f) == SEI_TYPE))
            break;
        p_tmp++;
    }
    p_tmp += 4;
	byte = 0;
    do
    {
        payloadType = 0;
		while(p_tmp[byte] == 0xff)
		{
			x_bitoffset += 8;
			byte++;			
			payloadType += 255;
		}

		value = p_tmp[byte];
		x_bitoffset += 8;
		byte++;
		payloadType += value;
		payloadSize = 0;

		while(p_tmp[byte] == 0xff)
		{
			x_bitoffset += 8;
			byte++;
			payloadSize += 255;
		}	
		
		value = p_tmp[byte];
		x_bitoffset += 8;
		byte++;
		payloadSize += value;	
//	mtos_printk("\r\n ~~~~payloadType:%d, payloadSize:%d", payloadType, payloadSize);
		if(payloadType == 6)
		{
			recovery_point_flag = 1;
			
//			OS_PRINTF("\r\n recovery point found!!!!!");
			break;
		}
		x_bitoffset += payloadSize / 8;
		byte += payloadSize;
    }while((byte < (end_pos - start_pos)) && p_tmp[byte] != 0x80);
	return recovery_point_flag;
}


static int ref_bit = 0;
static int h264_frame_mbs_only = 0;
static int h264_field_flag = 0;
static int h264_bottom_flag = 0;
nalu_t  x_check_nalu_type2(char * p_ts_packet, char *head_frame_type, 
                                 int len/*188*/, slice_t * p_slice_type)
{
    char    *p_pes_data_start = NULL;
    unsigned long payload = 0;
    MT_BOOL    is_nalu_startcode = FALSE;
    unsigned char  tmp_buf[188] = {0};

    char      *p_data =  NULL;
    char      *p_es_payload_start = NULL;

    //slice_t     slice_type = INVALID_SLICE_TYPE;
    nalu_t     nalu_type = INVALID_NALU_TYPE;
    int          ts_header_len = 0;
    int          pes_header_len = 0;
    int          es_payload_len = 0;
    int          i = 0;
    int          start_pos = 0;
    int          end_pos = 0;
    char        temp[8] = {0};
    char        *p_temp = temp;
    //if (PAYLOAD_UNIT_START(p_ts_packet) == 1)
    int start_bit = PAYLOAD_UNIT_START(p_ts_packet);
    if(1)
    {
        p_pes_data_start = pes_packet_start(p_ts_packet, &payload);
        ts_header_len = p_pes_data_start - p_ts_packet;
        payload = len - ts_header_len;


        if ((payload >= 4))
        {
            if(start_bit)
            {
                  //must have pes head
               p_es_payload_start = pes_data_start(p_pes_data_start, &payload);
               pes_header_len = p_es_payload_start - p_pes_data_start;
   
               p_data = p_es_payload_start;
               es_payload_len = len - pes_header_len - ts_header_len;
               
            }
            else
            {
               //may not have pes head only es data
               //p_es_payload_start = pes_data_start(p_pes_data_start, &payload);
               //pes_header_len = p_es_payload_start - p_pes_data_start;
   
               p_data = p_pes_data_start;// p_es_payload_start;
               es_payload_len = (int)payload;// len - pes_header_len - ts_header_len;
            }
            memcpy(p_temp, head_frame_type, 4);
            memcpy(p_temp + 4, p_data, 4);
            memcpy(head_frame_type,&p_data[es_payload_len - 4],4);
            i = 4;
            while(!is_nalu_startcode && i)
            {
                is_nalu_startcode = IDENTIFY_NALU_STARTCODE(p_temp);
                i--;
                p_temp++;
                
            }
            if(is_nalu_startcode)
            {
                p_temp--;
                nalu_type = p_temp[3] & 0X1F;
                {
                    is_nalu_startcode = FALSE;
                }
                if(nalu_type == NONE_IDR || nalu_type == IDR_TYPE || nalu_type == PARTITION_DATA_A)
                {
                    //nalu_type = p_data[3] & 0X1F;
                    start_pos = 4 - (p_temp - temp);
                    memcpy(tmp_buf, &p_temp[4], 4 - (p_temp - temp));
                    memcpy(tmp_buf + start_pos, p_data + 4, payload - 4);
                    end_pos = start_pos + payload - 4;
                    start_pos = 0;
                    OS_PRINTF("x_check_slice_type!\n");
                    *p_slice_type = x_check_slice_type(tmp_buf,start_pos,end_pos);
                }
                return nalu_type;
            }
            is_nalu_startcode = IDENTIFY_NALU_STARTCODE(p_data);
            if(nalu_type == SPLIT_NALU_TYPE)
            {
                is_nalu_startcode = FALSE;
            }

            //while((!is_nalu_startcode) && (i + 3 < es_payload_len))
            while((i + 3 < es_payload_len))
            {
                is_nalu_startcode = IDENTIFY_NALU_STARTCODE(p_data);
                if(is_nalu_startcode)
                {
                    ref_bit = (p_data[3] & 0x60) >> 5;
                    nalu_type = p_data[3] & 0X1F;
                    {
                        is_nalu_startcode = FALSE;
                    }
                    print_nalu_type(nalu_type);
                }
                if((nalu_type >= NONE_IDR && nalu_type <= IDR_TYPE) || nalu_type == SPS_TYPE)
                {
                    break;
                }
                p_data ++;
                i ++;    //i is only equeal 1 or 0
                
            }
        }
    }
    else
    {
        return nalu_type;
    }

    if(nalu_type == NONE_IDR || nalu_type == IDR_TYPE || nalu_type == PARTITION_DATA_A)
    {
        //nalu_type = p_data[3] & 0X1F;
        memcpy(tmp_buf, p_data + 4, payload - 4 - i);
        start_pos = 0;
        end_pos = (payload - 4 - i) - 1;
        OS_PRINTF("x_check_slice_type!\n");
        *p_slice_type = x_check_slice_type(tmp_buf,start_pos,end_pos);
    }

    return nalu_type;

}
nalu_t  x_check_nalu_type1(char * p_ts_packet , int len/*188*/,
								slice_t * p_slice_type)
{
    char    *p_pes_data_start = NULL;
    unsigned long payload = 0;
    MT_BOOL    is_nalu_startcode = FALSE;
    unsigned char  tmp_buf[188] = {0};

    char      *p_data =  NULL;
    char      *p_es_payload_start = NULL;

    //slice_t     slice_type = INVALID_SLICE_TYPE;
    nalu_t     nalu_type = INVALID_NALU_TYPE;
    int          ts_header_len = 0;
    int          pes_header_len = 0;
    int          es_payload_len = 0;
    int          i = 0;
    int          start_pos = 0;
    int          end_pos = 0;
    //if (PAYLOAD_UNIT_START(p_ts_packet) == 1)
    int start_bit = PAYLOAD_UNIT_START(p_ts_packet);
    if(1)
    {
        p_pes_data_start = pes_packet_start(p_ts_packet, &payload);
        ts_header_len = p_pes_data_start - p_ts_packet;
        payload = len - ts_header_len;


        if ((payload >= 4))
        {
            if(start_bit)
            {
                  //must have pes head
               p_es_payload_start = pes_data_start(p_pes_data_start, &payload);
               pes_header_len = p_es_payload_start - p_pes_data_start;
   
               p_data = p_es_payload_start;
               es_payload_len = len - pes_header_len - ts_header_len;
               
            }
            else
            {
               //may not have pes head only es data
               //p_es_payload_start = pes_data_start(p_pes_data_start, &payload);
               //pes_header_len = p_es_payload_start - p_pes_data_start;
   
               p_data = p_pes_data_start;// p_es_payload_start;
               es_payload_len = (int)payload;// len - pes_header_len - ts_header_len;
            }

            is_nalu_startcode = IDENTIFY_NALU_STARTCODE(p_data);
            if(nalu_type == SPLIT_NALU_TYPE)
            {
                is_nalu_startcode = FALSE;
            }

            //while((!is_nalu_startcode) && (i + 3 < es_payload_len))
            while((i + 3 < es_payload_len))
            {
                is_nalu_startcode = IDENTIFY_NALU_STARTCODE(p_data);
                if(is_nalu_startcode)
                {
                    ref_bit = (p_data[3] & 0x60) >> 5;
                    nalu_type = p_data[3] & 0X1F;
                    {
                        is_nalu_startcode = FALSE;
                    }
                    print_nalu_type(nalu_type);
                }
                if((nalu_type >= NONE_IDR && nalu_type <= IDR_TYPE))
                {
                    break;
                }
                p_data ++;
                i ++;    //i is only equeal 1 or 0
            }
        }
    }
    else
    {
        return nalu_type;
    }

    if(nalu_type == NONE_IDR || nalu_type == IDR_TYPE || nalu_type == PARTITION_DATA_A)
    {
        //nalu_type = p_data[3] & 0X1F;
        memcpy(tmp_buf, p_data + 4, payload - 4 - i);
        start_pos = 0;
        end_pos = (payload - 4 - i) - 1;
        OS_PRINTF("x_check_slice_type!\n");
        *p_slice_type = x_check_slice_type(tmp_buf,start_pos,end_pos);
    }

    return nalu_type;

}
static int find_last_sps = 0;
nalu_t  x_check_nalu_type(char * p_ts_packet , int len/*188*/,
									slice_t * p_slice_type)
{
    char    *p_pes_data_start = NULL;
    unsigned long payload = 0;
    MT_BOOL    is_nalu_startcode = FALSE;
//    unsigned char  tmp_buf[188] = {0};

    char      *p_data =  NULL;
    char      *p_es_payload_start = NULL;

    //slice_t     slice_type = INVALID_SLICE_TYPE;
    nalu_t     nalu_type = INVALID_NALU_TYPE;
    int          ts_header_len = 0;
    int          pes_header_len = 0;
    int          es_payload_len = 0;
    int          i = 0;
    int          start_pos = 0;
    int          end_pos = 0;
//    char      *p_data1 = NULL;
//    int         last_nalu = 0;

    //if (PAYLOAD_UNIT_START(p_ts_packet) == 1)
    int start_bit = PAYLOAD_UNIT_START(p_ts_packet);
    if(1)
    {
        p_pes_data_start = pes_packet_start(p_ts_packet, &payload);
        ts_header_len = p_pes_data_start - p_ts_packet;
        payload = len - ts_header_len;


        if ((payload >= 4))
        {
            if(start_bit)
            {
                  //must have pes head
               p_es_payload_start = pes_data_start(p_pes_data_start, &payload);
               pes_header_len = p_es_payload_start - p_pes_data_start;
   
               p_data = p_es_payload_start;
               es_payload_len = len - pes_header_len - ts_header_len;
               
            }
            else
            {
               //may not have pes head only es data
               //p_es_payload_start = pes_data_start(p_pes_data_start, &payload);
               //pes_header_len = p_es_payload_start - p_pes_data_start;
   
               p_data = p_pes_data_start;// p_es_payload_start;
               es_payload_len = (int)payload;// len - pes_header_len - ts_header_len;
            }

            is_nalu_startcode = IDENTIFY_NALU_STARTCODE(p_data);
            if(nalu_type == SPLIT_NALU_TYPE)
            {
                is_nalu_startcode = FALSE;
            }

            //while((!is_nalu_startcode) && (i + 3 < es_payload_len))
            while((i + 3 < es_payload_len))
            {
                is_nalu_startcode = IDENTIFY_NALU_STARTCODE(p_data);
                if(is_nalu_startcode)
                {
                    ref_bit = 0;
                    nalu_type = p_data[3] & 0X1F;
                    {
                        is_nalu_startcode = FALSE;
                    }
                    print_nalu_type(nalu_type);
                    if(start_bit && nalu_type == SPS_TYPE)
                    {
                       //i think it is a sps
                       h264_bottom_flag = 0;
                       x_check_spsframe(p_data,
                                                    0,
                                                    p_ts_packet + 188 - p_data,
                                                    &h264_frame_mbs_only);
                    }
                    if(nalu_type == SPS_TYPE)
                    {
//                         p_data1 = p_data;
                    }
                 if(start_bit && nalu_type == SPS_TYPE)
                {
                   //i think they in the same TS PACKET 188 bytes
                   find_last_sps = 1;
                   *p_slice_type = I_SLICE;
                   ref_bit = (p_data[3] & 0x60) >> 5;
//                      last_nalu = nalu_type;
                   break;
                }                   
//                    last_nalu = nalu_type;
                }

                if(nalu_type >= NONE_IDR && nalu_type <= IDR_TYPE)
                {
                    ref_bit = (p_data[3] & 0x60) >> 5;
                    break;
                }
                p_data ++;
                i ++;    //i is only equeal 1 or 0
            }
        }
    }
    else
    {
        return nalu_type;
    }

    if(nalu_type == NONE_IDR || nalu_type == IDR_TYPE || nalu_type == PARTITION_DATA_A)
    {
#if 0
        //nalu_type = p_data[3] & 0X1F;
        memcpy(tmp_buf, p_data + 4, payload - 4 - i);
        start_pos = 0;
        end_pos = (payload - 4 - i) - 1;
        *p_slice_type = x_check_slice_type(tmp_buf,start_pos,end_pos);
#else
        start_pos = 0;
        end_pos = (payload - i) - 1;
        *p_slice_type = x_check_slice_type2(p_data,start_pos,end_pos,
                     &h264_frame_mbs_only,&h264_field_flag,&h264_bottom_flag);

        if(*p_slice_type == MULTIPLY_SLICE)
        {
          nalu_type = INVALID_NALU_TYPE;
          *p_slice_type = INVALID_SLICE_TYPE;
        }
        if(find_last_sps)
        {
           if(*p_slice_type == I_SLICE || *p_slice_type == I_SLICE_2)
           {
              *p_slice_type = INVALID_SLICE_TYPE;
              nalu_type = INVALID_NALU_TYPE;     
           }

           find_last_sps = 0;
           return nalu_type;
        }
#endif
    }

    return nalu_type;

}

static int x_split_hd_sequence(ts_seq_t * p_TsSeqHandle, slice_t slice_type)
{
 //check the frame drop or not
   sequence_detail_t *p_AU = &p_TsSeqHandle->cur_sequence_detail;
   int index = p_AU->totalSlice;

   switch(p_TsSeqHandle->play_mode)
   {
      case TS_SEQ_SLOW_PLAY_2X:
      case TS_SEQ_SLOW_PLAY_4X:
      case TS_SEQ_NORMAL_PLAY:
      case TS_SEQ_FAST_PLAY_2X:
          //MT_ASSERT(0);
          break;
 
      case TS_SEQ_FAST_PLAY_4X:
      case TS_SEQ_FAST_PLAY_8X:
      case TS_SEQ_FAST_PLAY_16X:
      case TS_SEQ_FAST_PLAY_32X:
      case TS_SEQ_REV_FAST_PLAY_2X:
      case TS_SEQ_REV_FAST_PLAY_4X:
      case TS_SEQ_REV_FAST_PLAY_8X:
      case TS_SEQ_REV_FAST_PLAY_16X:
      case TS_SEQ_REV_FAST_PLAY_32X:
      {
         if(slice_type == B_SLICE || slice_type == P_SLICE)
        {
           p_AU->frame_dropbit[index] = 1;
        }
         break;
      }
      
      default:
      break;
   }

   return 0;
}
static int x_split_sequence(ts_seq_t * p_TsSeqHandle, slice_t slice_type)
{
 //check the frame drop or not
   sequence_detail_t *p_AU = &p_TsSeqHandle->cur_sequence_detail;
   int index = p_AU->totalSlice;
   int total_index = p_TsSeqHandle->totalframes;
   int ref_idc = p_AU->frame_refbit[index];
 
   switch(p_TsSeqHandle->play_mode)
   {
      case TS_SEQ_SLOW_PLAY_2X:
      case TS_SEQ_SLOW_PLAY_4X:
      case TS_SEQ_NORMAL_PLAY:
      case TS_SEQ_FAST_PLAY_2X:
          //MT_ASSERT(0);
          break;
 
      case TS_SEQ_FAST_PLAY_4X:
        if(ref_idc == 0)
        {
         p_AU->frame_dropbit[index] = 1;
        }
      break;
      case TS_SEQ_FAST_PLAY_8X:
      {
         int dropmask = 2;
         if(p_TsSeqHandle->play_mode == TS_SEQ_FAST_PLAY_8X)
            dropmask = 4;
         
         if(total_index % dropmask)
         {
         //need drop frame
           if(ref_idc == 0)
           {
             p_AU->frame_dropbit[index] = 1;
           }
           else
            {               
              //find the previous B frame and drop
              int i = index;
              int needdropone = 1;
              while(i)
              {
                  i--;
                  if(p_AU->frame_dropbit[i] == 0 &&
                   p_AU->frame_refbit[i] == 0)
                   {
                     p_AU->frame_dropbit[i] = 1;
 
                     needdropone = 0;
                     break;
                   }
              }
              if(needdropone)
              {
                 p_TsSeqHandle->needdropnum ++;
              }
         
            }
         }
         else//not drop frame
         {
            if(p_TsSeqHandle->needdropnum)
             {
                 if(ref_idc == 0)
                 {
                    p_AU->frame_dropbit[index] = 1;
 
                    p_TsSeqHandle->needdropnum --;
                 }
             }
         }           
         break;
      }
      case TS_SEQ_FAST_PLAY_16X:
      case TS_SEQ_FAST_PLAY_32X:
      {
          int dropmask = 16;
          if(p_TsSeqHandle->play_mode == TS_SEQ_FAST_PLAY_16X)
            dropmask = 8;

          if(total_index % dropmask)
          {
  
           //need drop frame
             if(ref_idc == 0)
             {
               p_AU->frame_dropbit[index] = 1;
             }
             else
             {               
               //find the previous B frame and drop

               int needdropone = 1;
               if(slice_type == I_SLICE)
              {
                 int i = index;
                 while(i)
                 {
                     i--;
                     if(p_AU->frame_dropbit[i] == 0 &&
                      p_AU->slice_type_array[i]== B_SLICE)
                      {
                        p_AU->frame_dropbit[i] = 1;
     
                        needdropone = 0;
                        break;
                      }
                 }
                 if(needdropone)
                 { 
                   int i = index;
                   while(i)
                   {
                       i--;
                       if(p_AU->frame_dropbit[i] == 0 &&
                        p_AU->slice_type_array[i]== P_SLICE)
                        {
                          p_AU->frame_dropbit[i] = 1;
       
                          needdropone = 0;
                          break;
                        }
                   }
                }
              }
               if(needdropone)
                 p_TsSeqHandle->needdropnum ++;
             }
           
          }
          else//not drop frame
          {
             if(ref_idc == 0)
             {
                p_AU->frame_dropbit[index] = 1;
             }
          }
          
          break;
      }
      case TS_SEQ_REV_FAST_PLAY_2X:
      case TS_SEQ_REV_FAST_PLAY_4X:
      case TS_SEQ_REV_FAST_PLAY_8X:
      {
          int dropmask = 2;
          if(p_TsSeqHandle->play_mode == TS_SEQ_REV_FAST_PLAY_4X)
          {
            dropmask = 4;
          }
          if(p_TsSeqHandle->play_mode == TS_SEQ_REV_FAST_PLAY_8X)
          {
            dropmask = 8;
          }
          if(total_index % dropmask)
          {
            if(slice_type == B_SLICE || slice_type == P_SLICE)
            {
              p_AU->frame_dropbit[index] = 1;          
            }
            else
            {
               //find the previous B P frame and drop
               int i = index;
               int needdropone = 1;
               while(i)
               {
                   i--;
                   if(p_AU->frame_dropbit[i] == 0)
                    {
                      p_AU->frame_dropbit[i] = 1;
                      
                      needdropone = 0;
                      break;
                    }
               }
               if(needdropone)
               {
                  p_TsSeqHandle->needdropnum ++;
               }
            }
          }
          else
          {
               if(p_TsSeqHandle->needdropnum)
              {
                  if(slice_type == B_SLICE || slice_type == P_SLICE)
                  {
                     p_AU->frame_dropbit[index] = 1;
  
                     p_TsSeqHandle->needdropnum --;
                  }
              }         
          }
          break;
      }
      case TS_SEQ_REV_FAST_PLAY_16X:
      case TS_SEQ_REV_FAST_PLAY_32X:
      {
        int dropmask = 16;
        if(p_TsSeqHandle->play_mode == TS_SEQ_REV_FAST_PLAY_32X)
        {
          dropmask = 32;
        }
        if(total_index % dropmask)
        {
        //need drop frame
          if(slice_type == B_SLICE || slice_type == P_SLICE)
          {
            p_AU->frame_dropbit[index] = 1;
          }
          else //this is i frame
          {
             //find the previous B frame and drop
             if(p_TsSeqHandle->needdropnum)
             {
               p_AU->frame_dropbit[index] = 1;
             }
             else
             {
                p_TsSeqHandle->needdropnum ++;
             } 
          }
        }
        else//not drop frame
        {
           if(p_TsSeqHandle->needdropnum)
            {
    
               p_AU->frame_dropbit[index] = 1;
    
               p_TsSeqHandle->needdropnum --;
    
            }
           else if(slice_type == B_SLICE || slice_type == P_SLICE)
           {
            p_AU->frame_dropbit[index] = 1;
           }
        }
        break;
      }
      default:
      break;
   }
 
   return 0;
}

int do_parse_h264_sequence1(void * p_Handle)
{
    ts_seq_t *p_TsSeqHandle =     (ts_seq_t *)p_Handle;
    char *    p_data =                        p_TsSeqHandle->p_data;
    int        data_len =                   p_TsSeqHandle->data_len;
    int        cur_video_pid =           p_TsSeqHandle->video_pid;
//    char *    p_tmp_buffer_wp =         p_TsSeqHandle->p_tmp_buffer_wp;
    int         left_data_len  =           p_TsSeqHandle->left_data_len;

    unsigned long packet_cnt = 0;
    int                array_index = 0;
    char            *p_temp = NULL;
    int               pid = 0xffffff;
    //video_frame_t cur_video_frame_type;
    int                 packet_index = 0;
    MT_BOOL  processLeftData = FALSE;
    slice_t    cur_slice_type = INVALID_SLICE_TYPE;
    nalu_t    cur_nalu_type = INVALID_NALU_TYPE;

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
            MY_ERROR("[ERROR][do_parse_H264] find invalid ts paket !!!\n");
            MY_ERROR("[ERROR][do_parse_H264] not find sync code [0x47] !!!\n");
            continue;
        }

        if(processLeftData)
        {
            p_TsSeqHandle->left_data_len -= 188;
        }
        pid = TS_PID(p_temp);
        //<1> video pes
        if(pid == cur_video_pid)
        {
            cur_nalu_type = INVALID_NALU_TYPE;
            cur_slice_type = INVALID_SLICE_TYPE;
            cur_nalu_type = x_check_nalu_type(p_temp,188,&cur_slice_type);

            /* 1-1    video pes && start unit && NALU && IDR frame */
            if(cur_nalu_type == IDR_TYPE || 
                cur_nalu_type == PPS_TYPE ||
                cur_slice_type == I_SLICE || cur_slice_type == I_SLICE_2 ||
                cur_slice_type == P_SLICE || cur_slice_type == P_SLICE_2 ||
                cur_slice_type == B_SLICE || cur_slice_type == B_SLICE_2)
            {
                if(h264_field_flag == 1)
                {
                    if(h264_bottom_flag)
                    {
                      //this is the bottom field,not remember
                      continue;
                    }
                }
                array_index = p_TsSeqHandle->cur_sequence_detail.totalSlice;
                p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index] 
                    = p_temp;

                if(cur_nalu_type == IDR_TYPE || cur_nalu_type == PPS_TYPE)
                {
                   cur_nalu_type = IDR_TYPE;
                   cur_slice_type = I_SLICE;
                   p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = I_SLICE;
                }
                else if(cur_slice_type == I_SLICE || cur_slice_type == I_SLICE_2)
                {
                   cur_slice_type = I_SLICE;
                   p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = I_SLICE;
                }
                else if(cur_slice_type == P_SLICE || cur_slice_type == P_SLICE_2)
                {
                   cur_slice_type = P_SLICE;
                   p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = P_SLICE;
                }
                else if(cur_slice_type == B_SLICE || cur_slice_type == B_SLICE_2)
                {
                  cur_slice_type = B_SLICE;
                  p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = B_SLICE;
                }

                p_TsSeqHandle->cur_sequence_detail.frame_refbit[array_index] = ref_bit;
                
                if(p_TsSeqHandle->input_mode == TS_SEQ_INPUT_TS_HD)
                {
                  x_split_hd_sequence(p_TsSeqHandle, cur_slice_type);
                }
                else
                {
                  x_split_sequence(p_TsSeqHandle, cur_slice_type);
                }
#ifdef DEBUG_H264_PARSER
       OS_PRINTF("FRME index %d ,NALU type %d ,slice type %d,dropbit %d, addr %x\n",
           p_TsSeqHandle->cur_sequence_detail.totalSlice,
           cur_nalu_type,
           cur_slice_type,
           p_TsSeqHandle->cur_sequence_detail.frame_dropbit[array_index],
           p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index]);
#endif
                p_TsSeqHandle->totalframes ++;
                p_TsSeqHandle->cur_sequence_detail.totalSlice++;
                
            }            
            /* 1-4     no VCL NALU*/
            else
            {
                ;//do nothing
            }

        }
        //<2>  no video pes 
        else
        {
            ;//do nothing
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

#if 0
static int last_video_frame_type = 0;
int do_parse_h264_sequence(void * p_Handle)
{
    ts_seq_t *p_TsSeqHandle =     (ts_seq_t *)p_Handle;
    char *    p_data =                        p_TsSeqHandle->p_data;
    int        data_len =                   p_TsSeqHandle->data_len;
    int        cur_video_pid =           p_TsSeqHandle->video_pid;
    char *    p_tmp_buffer_wp =         p_TsSeqHandle->p_tmp_buffer_wp;
    int         left_data_len  =           p_TsSeqHandle->left_data_len;

    unsigned long packet_cnt = 0;
    int                array_index = 0;
    char            *p_temp = NULL;
    int               pid = 0xffffff;
    //video_frame_t cur_video_frame_type;
    int                 packet_index = 0;
    MT_BOOL  processLeftData = FALSE;
    slice_t    cur_slice_type = INVALID_SLICE_TYPE;
    nalu_t    cur_nalu_type = INVALID_NALU_TYPE;

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

#ifdef DEBUG_H264_PARSER


#ifdef WIN32
        assert(p_temp[0] == 0x47);
#else
        MY_ASSERT(p_temp[0] == 0x47);
#endif

#endif

        if(processLeftData)
        {
            p_TsSeqHandle->left_data_len -= 188;
        }
        pid = TS_PID(p_temp);
        //<1> video pes
        if(pid == cur_video_pid)
        {
            cur_nalu_type = INVALID_NALU_TYPE;
            cur_slice_type = INVALID_SLICE_TYPE;
            cur_nalu_type = x_check_nalu_type(p_temp,188,&cur_slice_type);

            /* 1-1    video pes && start unit && NALU && IDR frame */
            if(cur_nalu_type == IDR_TYPE)
            {
                last_video_frame_type = I_FRAME;
                if(p_TsSeqHandle->isFindFirstIDR == FALSE
                        &&  p_TsSeqHandle->isFindSecondIDR == FALSE)
                {
                    //find the  key frame of current gop
                    p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
                    p_TsSeqHandle->isFindFirstIDR = TRUE;
                    array_index = p_TsSeqHandle->cur_sequence_detail.totalSlice;
                    p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = I_SLICE;
                    p_TsSeqHandle->cur_sequence_detail.totalSlice++;

#ifdef WIN32
                    assert(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
                    assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
                    MT_ASSERT(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
                    MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
                    memcpy(p_tmp_buffer_wp,p_temp,188);

                    p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index] 
                        = p_tmp_buffer_wp;

                    p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
                    p_TsSeqHandle->tmp_buffer_free_space -= 188;

                }
                else if(p_TsSeqHandle->isFindFirstIDR == TRUE
                        && p_TsSeqHandle->isFindSecondIDR == FALSE)
                {
                    //find the tail of current gop or the key frame of next gop
                    p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
                    p_TsSeqHandle->isFindSecondIDR = TRUE;
                    array_index = p_TsSeqHandle->cur_sequence_detail.totalSlice;
                    p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = I_SLICE;

#ifdef WIN32
                    assert(p_tmp_buffer_wp + 188  <= p_TsSeqHandle->p_tmp_buffer_end);
                    assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
                    MT_ASSERT(p_tmp_buffer_wp + 188  <= p_TsSeqHandle->p_tmp_buffer_end);
                    MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif

                    memcpy(p_tmp_buffer_wp,p_temp,188);
                    p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index] 
                        = p_tmp_buffer_wp;

                    p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
                    p_TsSeqHandle->tmp_buffer_free_space -= 188;

                    MY_LOG("one sequence  is ok!!!!\n");
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
            /* 1-2    video pes && start unit && P Slice */
            else if(cur_slice_type == P_SLICE || cur_slice_type == P_SLICE_2)
            {
                last_video_frame_type = P_FRAME;
                if(p_TsSeqHandle->isFindFirstIDR)
                {
                    p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
                    array_index = p_TsSeqHandle->cur_sequence_detail.totalSlice;
                    p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = P_SLICE;
                    p_TsSeqHandle->cur_sequence_detail.totalSlice++;
#ifdef WIN32
                    assert(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
                    assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
                    MT_ASSERT(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
                    MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
#ifdef PLAY_LESS_MEM
                    p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index]
                        = p_tmp_buffer_wp;
#else
                    memcpy(p_tmp_buffer_wp,p_temp,188);
                    p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index] 
                        = p_tmp_buffer_wp;
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
            /* 1-3     video pes && start unit && B Slice */
            else if(cur_slice_type == B_SLICE || cur_slice_type == B_SLICE_2)
            {
                last_video_frame_type = B_FRAME;
                if(p_TsSeqHandle->isFindFirstIDR)
                {
                    p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
                    array_index = p_TsSeqHandle->cur_sequence_detail.totalSlice;
                    p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = B_SLICE;
                    p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index] 
                        = p_tmp_buffer_wp;
                    p_TsSeqHandle->cur_sequence_detail.totalSlice++;
#ifdef WIN32
                    assert(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
                    assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
                    MT_ASSERT(p_tmp_buffer_wp + 188  <=  p_TsSeqHandle->p_tmp_buffer_end);
                    MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
#ifndef PLAY_LESS_MEM
                    memcpy(p_tmp_buffer_wp,p_temp,188);
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
            /* 1-4     no VCL NALU*/
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
        //<2>  no video pes 
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

static int do_revert_parse_h264_sequence(void *p_Handle)
{
    //
    ts_seq_t *p_TsSeqHandle = (ts_seq_t *)p_Handle;
    int          data_len =                  p_TsSeqHandle->data_len;
    char  *p_data =  p_TsSeqHandle->p_data + data_len -188;
    int          cur_video_pid =           p_TsSeqHandle->video_pid;
    char *p_tmp_buffer_wp =     p_TsSeqHandle->p_tmp_buffer_wp;
    int         left_data_len  =            p_TsSeqHandle->left_data_len;


    unsigned long packet_cnt = 0;
    int                array_index = 0;
    char *p_temp = NULL;
    int               pid = 0xffffff;
    slice_t    cur_slice_type = INVALID_SLICE_TYPE;
    nalu_t     cur_nalu_type = INVALID_NALU_TYPE;
    int                 packet_index = 0;
    MT_BOOL             processLeftData = FALSE;

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


#ifdef DEBUG_H264_PARSER

#ifdef WIN32
        assert(p_temp[0] == 0x47);
#else
        MY_ASSERT(p_temp[0] == 0x47);
#endif
#endif
        pid = TS_PID(p_temp);
        //video pes
        if(pid == cur_video_pid)
        {
            cur_nalu_type = INVALID_NALU_TYPE;
            cur_slice_type = INVALID_SLICE_TYPE;
            cur_nalu_type = x_check_nalu_type(p_temp,188,&cur_slice_type);
            //video pes && start unit && Key frame
            if(cur_nalu_type == IDR_TYPE)
            {
                if(p_TsSeqHandle->isFindFirstIDR  == FALSE)
                {
                    //find the  tail  of current gop
                    p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
                    p_TsSeqHandle->isFindFirstIDR = TRUE;
                    array_index = p_TsSeqHandle->cur_sequence_detail.totalSlice;
                    p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = I_SLICE;
                    p_TsSeqHandle->cur_sequence_detail.totalSlice++;

#ifdef WIN32
                    assert(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
                    assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
                    MT_ASSERT(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
                    MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
                    memcpy(p_tmp_buffer_wp,p_temp,188);

                    p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index] 
                        = p_tmp_buffer_wp;

                    p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp -188;
                    p_TsSeqHandle->tmp_buffer_free_space -= 188;
                    break;
                }
            }
            //video pes && start unit && P Frame
            else if(cur_slice_type == P_SLICE || cur_slice_type == P_SLICE_2)
            {

                p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
                array_index = p_TsSeqHandle->cur_sequence_detail.totalSlice;
                p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = P_SLICE;
                p_TsSeqHandle->cur_sequence_detail.totalSlice++;
#ifdef WIN32
                assert(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
                assert(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#else
                MT_ASSERT(p_tmp_buffer_wp - 188  >=  p_TsSeqHandle->p_tmp_buffer_end);
                MT_ASSERT(p_TsSeqHandle->tmp_buffer_free_space >= 188);
#endif
                memcpy(p_tmp_buffer_wp,p_temp,188);
                p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index] 
                    = p_tmp_buffer_wp;
                p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp - 188;
                p_TsSeqHandle->tmp_buffer_free_space -= 188;
#ifdef PLAY_LESS_MEM
                //this is not iframe ,reset tmp buf
                p_TsSeqHandle->p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_start - 187;
                p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;
                p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index]
                                             = p_TsSeqHandle->p_tmp_buffer_start + 1;
#endif


            }
            //video pes && start unit && B frame
            else if(cur_slice_type == B_SLICE || cur_slice_type == B_SLICE_2)
            {

                p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
                array_index = p_TsSeqHandle->cur_sequence_detail.totalSlice;
                p_TsSeqHandle->cur_sequence_detail.slice_type_array[array_index] = B_SLICE;
                p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index]
                    = p_tmp_buffer_wp;
                p_TsSeqHandle->cur_sequence_detail.totalSlice++;
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
                p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[array_index]
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
                MY_ERROR("[do_revert_parse_h264_sequence]:[ERROR][ERROR]\n");
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
#if 0
static void printf_tmp_buffer(ts_seq_t * p_TsSeqHandle)
{
    int secIDRIndex = 0;
    int  useBufSize = 0;
    int  validUseBufSize  = 0;
    useBufSize = p_TsSeqHandle->p_tmp_buffer_wp - p_TsSeqHandle->p_tmp_buffer_start;
    secIDRIndex = p_TsSeqHandle->cur_sequence_detail.totalSlice;
    validUseBufSize = p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[secIDRIndex]
        - p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[0];
    MY_DEBUG("tmp_buffer_start: 0X%x\n",p_TsSeqHandle->p_tmp_buffer_start);
    MY_DEBUG("tmp_buffer_end: 0X%x\n",p_TsSeqHandle->p_tmp_buffer_end);
    MY_DEBUG("tmp_buffer_wp: 0X%x\n",p_TsSeqHandle->p_tmp_buffer_wp);

    MY_DEBUG("one sequence use buffer size: %d\n",validUseBufSize);
    MY_DEBUG("use buffer size : %d\n",useBufSize);


}

#endif
static void set_divide_factor(ts_seq_t * p_TsSeqHandle)
{
    int table_len = 0;
    ts_seq_play_mode_t  cur_play_mode = p_TsSeqHandle->play_mode;
    int i = 0;
    table_len = sizeof(PLAYMODE_TO_INDEX_TABLE) / sizeof(playmode_to_index_t);
    for(i = 0; i < table_len  - 1 ; i++)
    {
        if(cur_play_mode ==  PLAYMODE_TO_INDEX_TABLE[ i ].mode)
        {
            break;
        }
    }

    p_TsSeqHandle->devide_factor = PLAYMODE_TO_INDEX_TABLE[i].split_factor;

}

static int print_drop_slice_param(ts_seq_t * p_TsSeqHandle)
{

    MY_DEBUG("[totalParsedSlice:%d]\n",p_TsSeqHandle->totalParsedSlice);
    MY_DEBUG("[totalDropSlice:%d]\n",p_TsSeqHandle->totalDropSlice);
    MY_DEBUG("[Left Slice:%d]\n",p_TsSeqHandle->totalSaveSlice);
    MY_DEBUG("[currentSequencLen:%d]\n",p_TsSeqHandle->cur_sequence_detail.totalSlice);
    return 0;

}

static int  do_nX_split(ts_seq_t * p_TsSeqHandle)
{
    int sequenceLen = 0;
    char  **p_slice_addr_array = NULL;
    slice_t *p_slice_type_array = NULL;
    int      cpNum = 0;
    //int i = 0;
    int split_factor = 0;
    //int table_len = 0;
    int split_point_index = 0;
    int totalExceptDropSlice = 0;
    int totalExceptSaveSlice = 0;
    int totalExceptParsedSlice = 0;
    int  modify_drop_policy_interval = COARSE_GRAINDED_INTERVAL;
    int needDropSliceNum = 0;
    MT_BOOL needSplit = TRUE;
    static MT_BOOL  g_ForwardFindPslice = TRUE;
    static unsigned char baseFactor = 0;
    int curgop = p_TsSeqHandle->gop_num - 1;

    //printf_tmp_buffer(p_TsSeqHandle);
    if(p_TsSeqHandle->cur_sequence_detail.totalSlice < 2)
    {

        MY_DEBUG("@@@[ERROR] totalSlice is too short !!!!!!!!!\n");
        return -1;
    }

    sequenceLen = p_TsSeqHandle->cur_sequence_detail.totalSlice;
    p_slice_addr_array = p_TsSeqHandle->cur_sequence_detail.video_packet_start_array;
    p_slice_type_array = p_TsSeqHandle->cur_sequence_detail.slice_type_array;
    p_TsSeqHandle->totalParsedSequence ++ ;


    if(!p_TsSeqHandle->is_init_devide_factor)
    {
        set_divide_factor(p_TsSeqHandle);
        p_TsSeqHandle->is_init_devide_factor = TRUE;
    }

    if(sequenceLen < p_TsSeqHandle->devide_factor)
    {
        modify_drop_policy_interval = FINE_GRAINED_INTERVAL;
    }

    split_factor = p_TsSeqHandle->devide_factor;

    //in every 3 times of  do_8X_split, we will adjust drop frame policy one time. 
    if((baseFactor ++) % modify_drop_policy_interval == 1)
    {
        totalExceptParsedSlice = p_TsSeqHandle->totalParsedSlice 
            + (p_TsSeqHandle->cur_sequence_detail.totalSlice);
        totalExceptSaveSlice = (totalExceptParsedSlice) / split_factor;
        totalExceptDropSlice = (totalExceptParsedSlice) - totalExceptSaveSlice;
        //drop the total sequence
        if(p_TsSeqHandle->totalDropSlice + sequenceLen < totalExceptDropSlice)
        {
            cpNum =  0;
            split_point_index =  0;
            MY_DEBUG("@@@drop an entire picture sequence !!!!\n");

            needSplit = FALSE;
        }
        //save total sequence
        else if(p_TsSeqHandle->totalDropSlice  >= totalExceptDropSlice)
        {
            MY_DEBUG("@@@ preserve an entire picture sequence!!!!\n");
            cpNum =  p_slice_addr_array[sequenceLen] - p_slice_addr_array[0];
            split_point_index =  sequenceLen;
            needSplit = FALSE;
        }
        else if(modify_drop_policy_interval == 2)
        {
            needDropSliceNum = totalExceptDropSlice - p_TsSeqHandle->totalDropSlice;
            if(needDropSliceNum > 0 && needDropSliceNum <= sequenceLen)
            {
                split_point_index = sequenceLen - needDropSliceNum;

                if(p_slice_type_array[split_point_index] == P_SLICE 
                        && split_point_index + 1 < sequenceLen)
                {
                    cpNum =  p_slice_addr_array[split_point_index + 1] - p_slice_addr_array[0];
                    split_point_index = split_point_index + 1;
                }
                else if(p_slice_type_array[split_point_index + 1] == P_SLICE 
                        && split_point_index + 2 < sequenceLen)
                {
                    cpNum =  p_slice_addr_array[split_point_index + 2] - p_slice_addr_array[0];
                    split_point_index = split_point_index + 2;
                }
                else if(p_slice_type_array[split_point_index + 2] == P_SLICE 
                        && split_point_index + 3< sequenceLen)
                {
                    cpNum =  p_slice_addr_array[split_point_index + 3] - p_slice_addr_array[0];
                    split_point_index = split_point_index + 3;
                }
                else if(split_point_index - 1 >= 1 
                        && p_slice_type_array[split_point_index - 1] == P_SLICE)
                {
                    cpNum =  p_slice_addr_array[split_point_index] - p_slice_addr_array[0];
                }
                else if(split_point_index - 2 >= 1 
                        && p_slice_type_array[split_point_index - 2] == P_SLICE)
                {
                    cpNum =  p_slice_addr_array[split_point_index - 1] - p_slice_addr_array[0];
                    split_point_index = split_point_index - 1;
                }
                else
                {
                    MY_LOG("[ERROR][do_nX_split] : == fail 1111\n");
                    cpNum =  p_slice_addr_array[1] - p_slice_addr_array[0];
                    split_point_index = 1;
                }


                needSplit = FALSE;

            }
        }
        //drop part of current sequence

    }


    if(needSplit){

        if(g_ForwardFindPslice == FALSE)
        {
            //find the split point a PSlice which will be the tail of the new sequence
            // and should make sure that the slice after the PSlice must be valid
            if(p_slice_type_array[sequenceLen  / split_factor] == P_SLICE 
                    && ((sequenceLen  / split_factor) + 1) < sequenceLen)
            {
                cpNum =  p_slice_addr_array[(sequenceLen  / split_factor) + 1]
                    - p_slice_addr_array[0];
                split_point_index = (sequenceLen  / split_factor) + 1;
            }
            else if(p_slice_type_array[(sequenceLen  / split_factor) + 1] == P_SLICE 
                    && ((sequenceLen  / split_factor) + 2) < sequenceLen)
            {
                cpNum =  p_slice_addr_array[(sequenceLen  / split_factor) + 2]
                    - p_slice_addr_array[0];
                split_point_index = (sequenceLen  / split_factor) + 2;
            }
            else if(p_slice_type_array[(sequenceLen  / split_factor) + 2] == P_SLICE 
                    && ((sequenceLen  / split_factor) + 3) < sequenceLen)
            {
                cpNum =  p_slice_addr_array[(sequenceLen  / split_factor) + 3] 
                    - p_slice_addr_array[0];
                split_point_index = (sequenceLen  / split_factor) + 3;
            }
            else if(p_slice_type_array[(sequenceLen  / split_factor) + 3] == P_SLICE 
                    && ((sequenceLen  / split_factor) + 4) < sequenceLen)
            {
                cpNum =  p_slice_addr_array[(sequenceLen  / split_factor) + 4]
                    - p_slice_addr_array[0];
                split_point_index = (sequenceLen  / split_factor) + 4;
            }
            else if(sequenceLen  / split_factor == 0 && sequenceLen >= 1)
            {
                cpNum =  p_slice_addr_array[1] - p_slice_addr_array[0];
                split_point_index = 1;

            }
            else
            {
                MY_ERROR("[do_8X_split][ERROR] ==11  Between 2 PSlice,More Than 3 BSlices!\n");
                MY_ERROR("[do_8X_split][ERROR] ==11  do nothing !!!\n");
                return -1;
            }

            g_ForwardFindPslice = TRUE;

        }
        else
        {
            //find the split point  a PSlice which will be the tail of the new sequence
            // and should make sure that the slice after the PSlice must be valid
            if(p_slice_type_array[sequenceLen  / split_factor] == P_SLICE 
                    && ((sequenceLen  / split_factor) + 1) < sequenceLen)
            {
                cpNum =  p_slice_addr_array[(sequenceLen  / split_factor) + 1]
                    - p_slice_addr_array[0];
                split_point_index = (sequenceLen  / split_factor) + 1;
            }
            else if(p_slice_type_array[(sequenceLen  / split_factor) - 1] == P_SLICE 
                    && (sequenceLen  / split_factor) < sequenceLen)
            {
                cpNum =  p_slice_addr_array[(sequenceLen  / split_factor)]
                    - p_slice_addr_array[0];
                split_point_index = (sequenceLen  / split_factor);
            }
            else if(p_slice_type_array[(sequenceLen  / split_factor) - 2] == P_SLICE 
                    && ((sequenceLen  / split_factor) - 1) < sequenceLen)
            {
                cpNum =  p_slice_addr_array[(sequenceLen  / split_factor) -1]
                    - p_slice_addr_array[0];
                split_point_index = (sequenceLen  / split_factor) - 1;
            }
            else if(p_slice_type_array[(sequenceLen  / split_factor) - 3] == P_SLICE 
                    && ((sequenceLen  / split_factor) - 2) < sequenceLen)
            {
                cpNum =  p_slice_addr_array[(sequenceLen  / split_factor) - 2]
                    - p_slice_addr_array[0];
                split_point_index = (sequenceLen  / split_factor) - 2;
            }
            else if(p_slice_type_array[(sequenceLen  / split_factor) - 1] == I_SLICE 
                    || p_slice_type_array[(sequenceLen  / split_factor) - 2] == I_SLICE)
            {
                cpNum =  p_slice_addr_array[1] - p_slice_addr_array[0];
                split_point_index = 1;
            }
            else if(sequenceLen  / split_factor == 0 && sequenceLen >= 1)
            {
                cpNum =  p_slice_addr_array[1] - p_slice_addr_array[0];
                split_point_index = 1;
            }
            else{
                MY_ERROR("[do_8X_split][ERROR] == Between 2 PSlice,More Than 3 BSlices!\n");
                MY_ERROR("[do_8X_split][ERROR] == do nothing !!!\n");
                return -1;

            }

            g_ForwardFindPslice = FALSE;

        }
    }
    MY_DEBUG("[do_8X_split] : write :%d bytes to fifo\n ",cpNum);
    if(cpNum)
    {
#ifdef PLAY_LESS_MEM
        cpNum = p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[1] - 
                        p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[0];

        p_TsSeqHandle->i_frame_len[curgop] = cpNum;
        p_TsSeqHandle->bpframe_cpnum[curgop] = split_point_index - 1;
#endif
        write_fifo_kw(p_TsSeqHandle
                ,p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[0],cpNum);
    }

    p_TsSeqHandle->totalParsedSlice += p_TsSeqHandle->cur_sequence_detail.totalSlice;
    p_TsSeqHandle->totalDropSlice += (p_TsSeqHandle->cur_sequence_detail.totalSlice) 
        - split_point_index;
    p_TsSeqHandle->totalSaveSlice = p_TsSeqHandle->totalParsedSlice 
        - p_TsSeqHandle->totalDropSlice;

    print_drop_slice_param(p_TsSeqHandle);

    return 0;


}


static int  do_2X_split(ts_seq_t * p_TsSeqHandle)
{
    do_nX_split(p_TsSeqHandle);
    return 0;
}


static int do_4X_split(ts_seq_t * p_TsSeqHandle)
{
    do_nX_split(p_TsSeqHandle);
    return 0;
}

static int do_8X_split(ts_seq_t * p_TsSeqHandle)
{
    do_nX_split(p_TsSeqHandle);
    return 0; 
}

static int do_16X_split(ts_seq_t * p_TsSeqHandle)
{
    do_nX_split(p_TsSeqHandle);
    return 0;
}
static int do_32X_split(ts_seq_t * p_TsSeqHandle)
{
    do_nX_split(p_TsSeqHandle);
    return 0;
}
#if 0
static int do_64X_split(ts_seq_t * p_TsSeqHandle)
{
    do_nX_split(p_TsSeqHandle);
    return 0;
}

static int x_split_sequence(ts_seq_t * p_TsSeqHandle,ts_seq_play_mode_t  play_mode)
{

    switch(play_mode)
    {
        case TS_SEQ_FAST_PLAY_2X:
        case TS_SEQ_REV_FAST_PLAY_2X:
            do_2X_split(p_TsSeqHandle);
            break;

        case  TS_SEQ_FAST_PLAY_4X:
        case  TS_SEQ_REV_FAST_PLAY_4X:
            do_4X_split(p_TsSeqHandle);
            break;

        case  TS_SEQ_FAST_PLAY_8X:
        case TS_SEQ_REV_FAST_PLAY_8X:
            do_8X_split(p_TsSeqHandle);
            break;

        case  TS_SEQ_FAST_PLAY_16X:
        case TS_SEQ_REV_FAST_PLAY_16X:
            do_16X_split(p_TsSeqHandle);
            break;

        case  TS_SEQ_FAST_PLAY_32X:
        case TS_SEQ_REV_FAST_PLAY_32X:
            do_32X_split(p_TsSeqHandle);
            break;

        default:
            break;
    }

    return 0;
}
#endif

#endif

#ifdef PKT_STATSTICS
static int total_rev = 0;
static int total_rev_send = 0;
static int total_ff = 0;
static int total_ff_send = 0;
#endif

static int last_rev_drop = 0;

static int last_playmode = 0;
static int  rev_update_ts_detail_info(ts_seq_t * p_TsSeqHandle,ts_detail_info_t *p_ts_detail)
{
  sequence_detail_t *p_AU = &(p_TsSeqHandle->cur_sequence_detail);

  int goplen = p_AU->totalSlice;
  int dropbit = 0;
  int i = 0;
  int i_frame_index = 0;
  int pos = 0;
  ulong addr_start = 0;
  ulong addr_end = 0;
//int leftsize = p_TsSeqHandle->tmp_buffer_len - p_TsSeqHandle->tmp_buffer_offset;
  int tmpsize = 0;

#ifdef DEBUG_H264_PARSER  
OS_PRINTF("gop len %d\n",goplen);
  for(i = 0; i < goplen; i++)
    {
OS_PRINTF("@@@@FRME index %d ,type %d ,dropbit %d, addr %x\n",
                               i,
                               p_AU->slice_type_array[i],
                               p_AU->frame_dropbit[i],
                                p_AU->video_packet_start_array[i]);
 
    }
#endif
  MT_ASSERT(goplen >= 0);

  memset(p_ts_detail,0,sizeof(ts_detail_info_t));

  //update the tmpbuf
  if(goplen == 0)
  {
     tmpsize = p_TsSeqHandle->data_len;
  }
  else
  {
    tmpsize = (u32)(p_AU->video_packet_start_array[0]  - p_TsSeqHandle->p_data);
  }
  if(last_playmode == TS_SEQ_INPUT_TS &&
    p_TsSeqHandle->input_mode == TS_SEQ_INPUT_TS_HD)
  {
    p_TsSeqHandle->last_ts_dropbit = 1;
  }
  last_playmode = p_TsSeqHandle->input_mode;
  if(p_TsSeqHandle->last_ts_dropbit)
  {
    //this is the first gop of this playmode 
    goplen = goplen - 1;
    p_TsSeqHandle->last_ts_dropbit = 0;
  }

#if 0
  if(tmpsize)
  {
     if(tmpsize > leftsize)
     {
       p_TsSeqHandle->last_ts_dropbit = 1;
       p_TsSeqHandle->tmp_buffer_offset = 0;
       OS_PRINTF("$$$$skip frame\n");
     }
     else
     {
       memcpy(p_TsSeqHandle->p_tmp_buffer + p_TsSeqHandle->tmp_buffer_offset,
         p_TsSeqHandle->p_data, tmpsize);
       p_TsSeqHandle->tmp_buffer_offset += tmpsize;
     }
   }  
#else
  p_ts_detail->reverse_offset = tmpsize;
  if(p_ts_detail->reverse_offset >= p_TsSeqHandle->data_len)
  {
    OS_PRINTF("####reveroffset %d datalen %d\n",
      p_ts_detail->reverse_offset,p_TsSeqHandle->data_len);
    p_ts_detail->reverse_offset = 0;
    p_TsSeqHandle->last_ts_dropbit = 1;
  }
#endif

  //start with the last frame
  //fined the first i frame
  for(i = goplen; i > 0; i --)
  {
    if(p_AU->slice_type_array[i - 1] == I_SLICE)
    {
        i_frame_index = i ;
        addr_start = (ulong)p_AU->video_packet_start_array[i - 1];
        if(i == goplen)
        {
          if(goplen == p_AU->totalSlice)
          {
            addr_end = (ulong)(p_TsSeqHandle->p_data + p_TsSeqHandle->data_len);
          }
          else
          {
            addr_end = (ulong)p_AU->video_packet_start_array[i];
          }
          
        }
        else
        {
           addr_end = (ulong)p_AU->video_packet_start_array[i];
        }
        break;
    }
    
  }
  if(addr_start == 0 || addr_end == 0)
  {
    if(p_TsSeqHandle->input_mode != TS_SEQ_INPUT_TS_HD)
    { 
      switch(p_TsSeqHandle->play_mode)
      {
        case TS_SEQ_REV_FAST_PLAY_2X:
            last_rev_drop += goplen / 4;
            break;
        case TS_SEQ_REV_FAST_PLAY_4X:
            last_rev_drop += goplen / 8;
            break;       
        default:
            last_rev_drop = 0;
            break;
      }
    }
    //OS_PRINTF("NO I Frame fount in the stream %d,%d,\n",addr_start,addr_end);
#ifdef PKT_STATSTICS
    total_rev += goplen;
    OS_PRINTF("total num %d send %d\n",total_rev,total_rev_send);
#endif
    return 0;
  }

  while(last_rev_drop > 0)
  {
    pos = p_ts_detail->fragment_num;
    p_ts_detail->fragment_offset[pos] = addr_start - (ulong)p_TsSeqHandle->p_data;
    p_ts_detail->fragment_size[pos] = addr_end - addr_start;
  
    p_ts_detail->fragment_num ++; 

    last_rev_drop --;

  }
  for(i = goplen; i > 0 ; i--)
  {
    dropbit = p_AU->frame_dropbit[i - 1];

    if(dropbit == 0)
    {
      //write down the ptr
      if(p_AU->slice_type_array[i - 1] == I_SLICE && i < i_frame_index)
      {
        //update the copy i frame addr
        addr_start = (ulong)p_AU->video_packet_start_array[i - 1];
        addr_end = (ulong)p_AU->video_packet_start_array[i];
      }

      pos = p_ts_detail->fragment_num;
      p_ts_detail->fragment_offset[pos] = addr_start - (ulong)p_TsSeqHandle->p_data;
      p_ts_detail->fragment_size[pos] = addr_end - addr_start;

      p_ts_detail->fragment_num ++;


#ifdef DEBUG_H264_PARSER
    OS_PRINTF("ts_detail_info num %d ,offset %x,size %x \n",
        p_ts_detail->fragment_num,
        p_ts_detail->fragment_offset[p_ts_detail->fragment_num - 1],
        p_ts_detail->fragment_size[p_ts_detail->fragment_num - 1]);
#endif
    }

  }

  if(p_TsSeqHandle->input_mode == TS_SEQ_INPUT_TS_HD)
  {
    if(p_ts_detail->fragment_num)
        p_TsSeqHandle->last_ts_dropbit = 1;
  }
  //OS_PRINTF("ts_detail_info total num %d \n",p_ts_detail->fragment_num);
#ifdef PKT_STATSTICS
  total_rev += goplen;
  total_rev_send += p_ts_detail->fragment_num;
  OS_PRINTF("total num %d send %d\n",total_rev,total_rev_send);
#endif
  return 0;
}

static int  ff_update_ts_detail_info(ts_seq_t * p_TsSeqHandle,ts_detail_info_t *p_ts_detail)
{
  sequence_detail_t *p_AU = &p_TsSeqHandle->cur_sequence_detail;
  int goplen = p_AU->totalSlice;

  int dropbit = p_TsSeqHandle->last_ts_dropbit;

  ulong addr_start = (ulong)p_TsSeqHandle->p_data;
  ulong addr_end = 0;

  int i = 0;
  int pos = 0;
#ifdef DEBUG_H264_PARSER
  OS_PRINTF("gop len %d addr %x\n",goplen,p_TsSeqHandle->p_data);
  for(i = 0; i < goplen; i ++)
  {
     OS_PRINTF("@@@@FRME index %d ,type %d ,dropbit %d, addr %x\n",
                              i,
                              p_AU->slice_type_array[i],
                              p_AU->frame_dropbit[i],
                              p_AU->video_packet_start_array[i]);
 
  }
#endif  
  MT_ASSERT(goplen >= 0);
  if(goplen == 0)
  {
    addr_end = (ulong)(p_TsSeqHandle->p_data + p_TsSeqHandle->data_len);
  }
  else
  {
    addr_end = (ulong)(p_AU->video_packet_start_array[0]);
  }

  memset(p_ts_detail,0,sizeof(ts_detail_info_t));

  if(dropbit == 0)
  {
    //write down the ptr
    p_ts_detail->fragment_offset[0] = 0;
    p_ts_detail->fragment_size[0] = addr_end - addr_start;

    p_ts_detail->fragment_num ++;
#ifdef DEBUG_H264_PARSER
    OS_PRINTF("ts_detail_info num %d ,offset %x,size %x \n",
        p_ts_detail->fragment_num,
        p_ts_detail->fragment_offset[0],
        p_ts_detail->fragment_size[0]);
#endif
  }

  for(i = 0; i < goplen ; i++)
  {
    dropbit = p_AU->frame_dropbit[i];

    addr_start = (ulong)p_AU->video_packet_start_array[i];
    if(i == goplen - 1)//last frame
    {
      addr_end = (ulong)(p_TsSeqHandle->p_data + p_TsSeqHandle->data_len);

      if(p_TsSeqHandle->input_mode == TS_SEQ_INPUT_TS_HD)
      {
        dropbit = 1;
        if(p_AU->slice_type_array[i] == I_SLICE)
           p_ts_detail->reverse_offset = addr_start - (ulong)p_TsSeqHandle->p_data;
      }

      p_TsSeqHandle->last_ts_dropbit = dropbit;
    }
    else
    {
      addr_end = (ulong)(p_AU->video_packet_start_array[i + 1]);
    }

    if(dropbit == 0)
    {
      //write down the ptr
      pos = p_ts_detail->fragment_num;
      p_ts_detail->fragment_offset[pos] = addr_start - (ulong)p_TsSeqHandle->p_data;
      p_ts_detail->fragment_size[pos] = addr_end - addr_start;

      p_ts_detail->fragment_num ++;
#ifdef DEBUG_H264_PARSER
    OS_PRINTF("ts_detail_info num %d ,offset %x,size %x \n",
        p_ts_detail->fragment_num,
        p_ts_detail->fragment_offset[p_ts_detail->fragment_num - 1],
        p_ts_detail->fragment_size[p_ts_detail->fragment_num - 1]);
#endif
    }

  }
#ifdef PKT_STATSTICS
  total_ff += goplen;
  total_ff_send += p_ts_detail->fragment_num;
  OS_PRINTF("total num %d send %d\n",total_ff,total_ff_send);
#endif
  //OS_PRINTF("ts_detail_info total num %d \n",p_ts_detail->fragment_num);

  return 0;
}


int  ts_forward_sequence_parser(ts_seq_t * p_TsSeqHandle, ts_detail_info_t *p_ts_detail)
{
/*
    int second_idr_index = 0;
    char  *p_second_idr = NULL;
    int i = 0;
    slice_t *p_slice_type_array = NULL;
    MT_BOOL isFindFirstKeyIDR  =   FALSE;
    MT_BOOL isFindSecondIDR =  FALSE;
    int  useBufSize = 0;
*/
    do
    {
        do_parse_h264_sequence1(p_TsSeqHandle);

        if(p_TsSeqHandle->isExit)
        {
            break;
        }

      ff_update_ts_detail_info(p_TsSeqHandle,p_ts_detail);


#if 0
        isFindFirstKeyIDR = p_TsSeqHandle->isFindFirstIDR;
        isFindSecondIDR =  p_TsSeqHandle->isFindSecondIDR;

        //find two IDR
        if(isFindFirstKeyIDR && isFindSecondIDR)
        {
            p_TsSeqHandle->gop_num ++;        

            x_split_sequence(p_TsSeqHandle, p_TsSeqHandle->play_mode);

            //reset field of p_TsSeqHandle
            p_TsSeqHandle->isFindFirstIDR = FALSE;
            p_TsSeqHandle->isFindSecondIDR = FALSE;
            //print_cur_gop();
            useBufSize = 
                p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[second_idr_index]
                - p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[0];
            MY_LOG("last sequence need tmpbuffer size:%d\n",useBufSize);

            //the second key frame became the first key frame of current gop
            second_idr_index = p_TsSeqHandle->cur_sequence_detail.totalSlice;
            p_second_idr = 
                p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[second_idr_index];
            for(i = 0; i < MAX_SEQUENCE_LEN; i++)
            {
                p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[i] = NULL;
            }

            memcpy(p_TsSeqHandle->p_tmp_buffer_start,p_second_idr,188);
            p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len - 188;
            p_TsSeqHandle->p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_start + 188; 
            p_TsSeqHandle->cur_sequence_detail.totalSlice = 0;
            p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[0] = 
                p_TsSeqHandle->p_tmp_buffer_start;

            p_slice_type_array = NULL;
            p_slice_type_array = p_TsSeqHandle->cur_sequence_detail.slice_type_array;
            memset(p_slice_type_array, 0 ,MAX_SEQUENCE_LEN * sizeof(slice_t));
            p_TsSeqHandle->cur_sequence_detail.slice_type_array[0] =  I_SLICE;

            p_TsSeqHandle->cur_sequence_detail.totalSlice++;
            p_TsSeqHandle->isFindFirstIDR = TRUE;


            g_totalSequence++;
            MY_DEBUG("p_TsSeqHandle->p_tmp_buffer_start:%x\n",p_TsSeqHandle->p_tmp_buffer_start);
            MY_DEBUG("p_TsSeqHandle->p_tmp_buffer_wp:%x\n",p_TsSeqHandle->p_tmp_buffer_wp);
            MY_DEBUG("p_TsSeqHandle->p_tmp_buffer_end:%x\n",p_TsSeqHandle->p_tmp_buffer_end);
            MY_DEBUG("p_TsSeqHandle->tmp_buffer_free_space:%d\n",
                p_TsSeqHandle->tmp_buffer_free_space);
            MY_DEBUG("g_totalSequence:%d\n",g_totalSequence);


        }
#endif
    }
    while(p_TsSeqHandle->left_data_len  && p_TsSeqHandle->isExit == FALSE);

    return 0;

}



int ts_backward_sequence_parser(ts_seq_t * p_TsSeqHandle, ts_detail_info_t *p_ts_detail)
{
/*
    int i = 0;
    slice_t *p_slice_type_array = NULL;
    MT_BOOL isFindFirstIDR  =   FALSE;
*/
    do
    {
        do_parse_h264_sequence1(p_TsSeqHandle);


        if(p_TsSeqHandle->isExit)
        {
            break;
        }

        rev_update_ts_detail_info(p_TsSeqHandle,p_ts_detail);

#if 0
        isFindFirstIDR = p_TsSeqHandle->isFindFirstIDR;
        if(isFindFirstIDR)
        {
            resort_slicetype_array(
              (video_frame_t *)p_TsSeqHandle->cur_sequence_detail.slice_type_array,
                    p_TsSeqHandle->cur_sequence_detail.totalSlice);

            resort_packetstart_array(p_TsSeqHandle->cur_sequence_detail.video_packet_start_array
                    , p_TsSeqHandle->cur_sequence_detail.totalSlice);

            p_TsSeqHandle->isFindFirstIDR = FALSE;

            p_TsSeqHandle->gop_num ++;

            x_split_sequence(p_TsSeqHandle, p_TsSeqHandle->play_mode);

            p_TsSeqHandle->p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_start - 187;
            p_TsSeqHandle->cur_sequence_detail.totalSlice = 0;
            p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;

            p_slice_type_array = NULL;
            p_slice_type_array = p_TsSeqHandle->cur_sequence_detail.slice_type_array;
            memset(p_slice_type_array, 0, MAX_SEQUENCE_LEN * sizeof(slice_t));

            for(i = 0; i < MAX_SEQUENCE_LEN; i++)
            {
                p_TsSeqHandle->cur_sequence_detail.video_packet_start_array[i] = NULL;
            }

        }
#endif
    }
    while(p_TsSeqHandle->left_data_len);

    return 0;

}

