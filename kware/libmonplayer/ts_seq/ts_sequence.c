/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
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

#include "mpeg_parser.h"
#include "h264_parser.h"
#include "avs_parser.h"
#include "stdio.h"
#include "mt_unf_avplay.h"
//#include "common.h"
//#include "vdec.h"

//err: OS_PRINTF is defined in sys_define.h
//#define OS_PRINTF mtos_printk


#define TS_SEQ_DEBUG  printf

#define MT_ASSERT

#define DEBUG_TS_SEQUENCE
#ifdef   DEBUG_TS_SEQUENCE
#define  MY_LOG      printf
#define  MY_DEBUG    printf
#define  MY_ERROR    printf
#else
#define  MY_LOG      printf
#define  MY_DEBUG    printf
#define  MY_ERROR    printf
#endif
extern slice_t x_check_spsframe(char * p_ebsp, int start_pos, int end_pos, int * frame_mbs_only);
extern slice_t x_check_slice_type1(char * p_ebsp, int start_pos, int end_pos, 
    int * frame_mbs_only, int * filed_flag);

#define INTERNAL_FIFO_SIZE   (2 * 188 * 2560)
#define TS_PID(data) \
  ((u16)(((u8)(data[2])) | \
         (((u16)((u8)(data[1] & 0x1F))) << 8)))
#define PAYLOAD_UNIT_START(data) ((((u8)data[0]) == 0x47) && ((((u8)data[1]) & 0x40) == 0x40))

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

/*!
video pre start code
*/
#define PRE_START_CODE(x) \
((((unsigned char)(x)[0] == 0x00)) && (((unsigned char)(x)[1]) == 0x00) && \
 (((unsigned char)(x)[2]) == 0x01))
 
/*!
is pic start code
*/
#define IS_PICTURE_START_CODE(x) \
(PRE_START_CODE(x) && ((unsigned char)(x)[3] == 0x00))

/*!
is gop  start code
*/
#define IS_GOP_START_CODE(x) \
(PRE_START_CODE(x) && ((unsigned char)(x)[3] == 0xB8))

static void mpeg_picture_coding_extension_parser(char *p_data,
    int len,int *picture_structure)
{
    int i = 0;
    unsigned char *p_tmp = (unsigned char *)p_data;
    int is_find_pic_code_ext = 0;
   *picture_structure = 3;//default frame picture
    
    while(i + 4 < len)
    {
        if((p_tmp[0] == 0) && (p_tmp[1] == 0) && 
            (p_tmp[2] == 1) && (p_tmp[3] == 0xb5) &&
            ((p_tmp[4] & 0xf0) == 0x80))
        {
           is_find_pic_code_ext = 1;
           break;
        }
        i ++;
        p_tmp ++;
    }
    if(is_find_pic_code_ext && i + 2 < len)
    {
      *picture_structure = p_tmp[6] & 0x3;
    }
  return;
}

static video_frame_t  x_check_frame_type1(char * p_ts_packet ,int *iframe_with_gop
                               ,char *head_frame_type,int *pic_struct,int len/*188*/)
{
    char  *p_pes_data_start = NULL;
    unsigned long payload = 0;
    MT_BOOL is_picture_start_code = FALSE;
    MT_BOOL is_gop_start_code = FALSE;

    char  *p_data =  NULL;
    char  *p_es_payload_start = NULL;

    video_frame_t picture_coding_type = 0;
    int  ts_header_len = 0;
    int  pes_header_len = 0;
    int es_payload_len = 0;
//    MT_BOOL  find_code_type = TRUE;
    int i = 0;
    int have_gop = 0;
    int have_gop_last = 0;
    int start_bit = 0;
    char temp[8] = {0};
    char *p_temp_pic = temp;
    char *p_temp_gop = temp;
    
    *iframe_with_gop = 0;
    start_bit = PAYLOAD_UNIT_START(p_ts_packet);
    //if (PAYLOAD_UNIT_START(p_ts_packet) == 1)
    {
        p_pes_data_start = pes_packet_start(p_ts_packet, &payload);
        ts_header_len = p_pes_data_start - p_ts_packet;
        payload = len - ts_header_len;

        if ((payload >= 4))
        {
            if(start_bit)
            {
                p_es_payload_start = pes_data_start(p_pes_data_start, &payload);
                pes_header_len = p_es_payload_start - p_pes_data_start;

                p_data =  p_es_payload_start;
                es_payload_len = len - pes_header_len - ts_header_len;
            }
            else
            {
                p_data = p_pes_data_start;
                es_payload_len = (int)payload;
            }
            
            memcpy(p_temp_pic, head_frame_type, 4);
            memcpy(p_temp_pic + 4, p_data, 4);
            memcpy(head_frame_type,&p_data[es_payload_len - 4],4);
            i = 4;
            while(!is_picture_start_code && i)
            {
                is_picture_start_code = IS_PICTURE_START_CODE(p_temp_pic);
                i--;
                p_temp_pic++;
                
            }
            i = 4;
            while(!is_gop_start_code && i)
            {
                is_gop_start_code = IS_GOP_START_CODE(p_temp_gop);
                i--;
                p_temp_gop++;
                
            }
            if(is_gop_start_code)
            {
                p_temp_gop--;
                mpeg_picture_coding_extension_parser(p_data,es_payload_len,pic_struct);
                have_gop_last = 1;
                is_gop_start_code = 0;
            }
            if(is_picture_start_code)
            {
                p_temp_pic--;
                picture_coding_type = 0xff;
                picture_coding_type = p_temp_pic[5] & 0x38;
                picture_coding_type >>= 3;

                *iframe_with_gop = 2; 
                mpeg_picture_coding_extension_parser(p_data,es_payload_len,pic_struct);
                return picture_coding_type;
            }
            
            is_picture_start_code = IS_PICTURE_START_CODE(p_data);
            is_gop_start_code = IS_GOP_START_CODE(p_data);

            //do{
            i = 0;
            while((i + 3 < es_payload_len) && ((!is_picture_start_code) && (!is_gop_start_code)))
            {
                p_data ++;
                i ++;
                is_picture_start_code = IS_PICTURE_START_CODE(p_data);
                is_gop_start_code = IS_GOP_START_CODE(p_data);
                if(is_gop_start_code)
                {
                    mpeg_picture_coding_extension_parser(p_data,es_payload_len - i,pic_struct);
                    have_gop = 1;
                    is_gop_start_code = 0;
                }
            }
            if(is_picture_start_code)
            {
                if((i + 5) < es_payload_len)
                {
                    picture_coding_type = 0xff;
                    picture_coding_type = p_data[5] & 0x38;

                    picture_coding_type >>= 3;

                    if(picture_coding_type == I_FRAME)
                    {
                        //break;
                    }
                    else if(picture_coding_type == P_FRAME)
                    {
                        //break;
                    }
                    else if(picture_coding_type == B_FRAME)
                    {
                        //break;
                    }
                    else
                    {
                        OS_PRINTF("[get vkey]no picture_coding_type\n");
                        //break;
                    }
                }
                else if((i + 5) >=  es_payload_len)
                {
//                    find_code_type = FALSE;
                    //break;
                }
            }
            else//not find start code
            {
                picture_coding_type = 0;
            }
        }
        if(picture_coding_type == I_FRAME && have_gop_last)
        {
            *iframe_with_gop = 5; 
        }
        else if(picture_coding_type != I_FRAME && have_gop_last)
        {
            *iframe_with_gop = 4; 
        }
        else if(picture_coding_type == I_FRAME && have_gop)
        {
            *iframe_with_gop = 3; 
        }
        else if(picture_coding_type == I_FRAME && !have_gop)
        {
            *iframe_with_gop = 2;        
        }
        else if(picture_coding_type != I_FRAME && have_gop)
        {
            *iframe_with_gop = 1;        
            picture_coding_type = I_FRAME;

        }
        
        return picture_coding_type;

    }


    return 0;

}
extern nalu_t x_check_nalu_type2(char * p_ts_packet,char * head_frame_type, 
                                       int len, slice_t * p_slice_type);
extern nalu_t x_check_nalu_type1(char * p_ts_packet, int len, slice_t * p_slice_type);
/*!
  ts seq init.
  \param[in] prio The ts seq task prio
  \param[in] stack_size The ts seq task stack

  \Return The operation success or not.
  */
void * ts_seq_init(u8 *p_fifo, u32 fifo_size,MT_BOOL isForward, 
                    u8 *p_buf, u32 buffer_size)
{
    ts_seq_t *p_TsSeqHandle = NULL;
    int i = 0;
#ifdef WIN32
    p_TsSeqHandle = (ts_seq_t *)malloc(sizeof(ts_seq_t));
#else
    p_TsSeqHandle = (ts_seq_t *)mtos_malloc(sizeof(ts_seq_t));
#endif
    if(p_TsSeqHandle == NULL)
    {
        // TODO:
#ifdef WIN32
        assert(1);
#else
        MT_ASSERT(0);
#endif
        //printf("ERROR!!!!!!!\n");
        return NULL;
    }

    memset(p_TsSeqHandle, 0, sizeof(ts_seq_t));
    if (p_buf && buffer_size)
    {
        p_TsSeqHandle->p_tmp_buffer = p_buf;
        p_TsSeqHandle->tmp_buffer_len = buffer_size;
    }
    else
    {
      MT_ASSERT(0);
    }

    if(p_TsSeqHandle->p_tmp_buffer == NULL)
    {
        // TODO:
#ifdef WIN32
        assert(1);
#else
        MT_ASSERT(0);
#endif
        MY_DEBUG("ERROR!!!!!!!\n");
        return NULL;
    }
    memset(p_TsSeqHandle->p_tmp_buffer,0,p_TsSeqHandle->tmp_buffer_len);
#if 0
    p_TsSeqHandle->p_TsQueue = init_fifo_kw(p_fifo, fifo_size);
    if(p_TsSeqHandle->p_TsQueue == NULL)
    {
#ifdef WIN32
        assert(1);
#else
        MT_ASSERT(0);
#endif
    }
#endif    
    if(isForward)
    {
        p_TsSeqHandle->p_tmp_buffer_start = p_TsSeqHandle->p_tmp_buffer;
        p_TsSeqHandle->p_tmp_buffer_wp =   p_TsSeqHandle->p_tmp_buffer_start;
        p_TsSeqHandle->p_tmp_buffer_rd = p_TsSeqHandle->p_tmp_buffer_start;
        p_TsSeqHandle->p_tmp_buffer_end
            = p_TsSeqHandle->p_tmp_buffer_start + p_TsSeqHandle->tmp_buffer_len - 1;
        p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;
        p_TsSeqHandle->isForward = TRUE;
        p_TsSeqHandle->data_len = 0;
    }
    else
    {
        p_TsSeqHandle->p_tmp_buffer_end = p_TsSeqHandle->p_tmp_buffer;
        p_TsSeqHandle->p_tmp_buffer_start
            = p_TsSeqHandle->p_tmp_buffer + p_TsSeqHandle->tmp_buffer_len - 1;
        p_TsSeqHandle->p_tmp_buffer_wp =   p_TsSeqHandle->p_tmp_buffer_start - 187;
        p_TsSeqHandle->p_tmp_buffer_rd =  p_TsSeqHandle->p_tmp_buffer_wp;

        p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;
        p_TsSeqHandle->isForward = FALSE;
        p_TsSeqHandle->data_len = 0;

    }

    p_TsSeqHandle->isFindFirstIFrame = FALSE;
    p_TsSeqHandle->isFindSecondIFrame = FALSE;
    p_TsSeqHandle->cur_gop_type.base_frame_type = I_B_B_P;

    p_TsSeqHandle->isFindFirstIDR = FALSE;
    p_TsSeqHandle->isFindSecondIDR = FALSE;
    memset(&(p_TsSeqHandle->cur_sequence_detail),0,sizeof(sequence_detail_t));
    for(i = 0; i < MAX_SEQUENCE_LEN; i++)
    {
        p_TsSeqHandle->cur_sequence_detail.slice_type_array[i] = INVALID_SLICE_TYPE;
    }

    p_TsSeqHandle->seq_parser_status = TS_SEQ_STATUS_IDLE;
    p_TsSeqHandle->isExit = FALSE;

    p_TsSeqHandle->dropOneGop = FALSE;
    p_TsSeqHandle->drop_segment_num = 0;

    p_TsSeqHandle->totalframes = 0;
    p_TsSeqHandle->last_ts_dropbit = 0;
    p_TsSeqHandle->tmp_buffer_offset = 0;
    p_TsSeqHandle->needdropnum = 0;
    
    return (void *)p_TsSeqHandle;

}

/*!
  ts seq deinit.

  \Return The operation success or not.
 */
int ts_seq_deinit(void * p_Handle)
{
    ts_seq_t * p_TsSeqHandle = (ts_seq_t *)p_Handle;

/*    if(p_TsSeqHandle->p_TsQueue)              // for tsscan
    {
        //deinit_fifo_kw(p_TsSeqHandle->p_TsQueue);
    }
*/
    if(p_TsSeqHandle)
    {
#ifdef WIN32
        free(p_TsSeqHandle);
#else
        mtos_free(p_TsSeqHandle);
#endif
        p_TsSeqHandle = NULL;
    }

    return 0;
}


int ts_seq_play1(void * p_Handle, void *p_vHanle, char * p_data
        , unsigned int  length,int    v_pid, ts_seq_play_mode_t play_mode
        , ts_seq_input_mode_t input_mode, ts_detail_info_t *p_ts_detail)
{
    ts_seq_t *p_TsSeqHandle = (ts_seq_t *)p_Handle;

    p_TsSeqHandle->p_data = p_data;
    p_TsSeqHandle->data_len = length;

    p_TsSeqHandle->left_data_len = 0;
    p_TsSeqHandle->play_mode = play_mode;

    p_TsSeqHandle->input_mode = input_mode;
    p_TsSeqHandle->video_pid = v_pid;

    p_TsSeqHandle->seq_parser_status = TS_SEQ_STATUS_BUSY;

    memset(&p_TsSeqHandle->cur_gop_type,0,sizeof(gop_t));

    memset(&p_TsSeqHandle->cur_sequence_detail,0,sizeof(sequence_detail_t));

    memset(p_ts_detail,0,sizeof(ts_detail_info_t));
/*
    OS_PRINTF("vid %d tmpsize %x needdrop %d totalfrms %d\n",
        v_pid,
        p_TsSeqHandle->tmp_buffer_offset,
        p_TsSeqHandle->needdropnum,
        p_TsSeqHandle->totalframes);
*/
    if(p_TsSeqHandle->last_play_mode != play_mode)
    { 
       p_TsSeqHandle->needdropnum = 0;
       //OS_PRINTF("last_ts_dropbit %d\n",p_TsSeqHandle->last_ts_dropbit);
       p_TsSeqHandle->last_ts_dropbit = 1;
       switch (play_mode)
       {       
           case TS_SEQ_FAST_PLAY_2X:
             vdec_set_trick_mode(p_vHanle,MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD , 2);//VDEC_TM_FFWD
             break;
           case TS_SEQ_FAST_PLAY_4X:
             vdec_set_trick_mode(p_vHanle, MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD, 4);//VDEC_TM_FFWD
             break;
           case TS_SEQ_FAST_PLAY_8X:
             vdec_set_trick_mode(p_vHanle, MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD, 8);//VDEC_TM_FFWD
             break;
           case TS_SEQ_FAST_PLAY_16X:
             vdec_set_trick_mode(p_vHanle, MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD, 16);//VDEC_TM_FFWD
             break;
           case TS_SEQ_FAST_PLAY_32X:    
             vdec_set_trick_mode(p_vHanle, MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD, 32);//VDEC_TM_FFWD
             break;
           case TS_SEQ_SLOW_PLAY_2X:
            //vdec_set_trick_mode(p_vHanle,3 , 2);//VDEC_TM_SFWD
            OS_PRINTF("[%s]%d unsupport slow play!!!\n",__func__, __LINE__);
             break;
           case TS_SEQ_SLOW_PLAY_4X:
             //vdec_set_trick_mode(p_vHanle, 3, 4);  //VDEC_TM_SFWD          
             OS_PRINTF("[%s]%d unsupport slow play!!!\n",__func__, __LINE__);
             break;
           case TS_SEQ_REV_FAST_PLAY_2X:
           case TS_SEQ_REV_FAST_PLAY_4X:
           case TS_SEQ_REV_FAST_PLAY_8X:
           case TS_SEQ_REV_FAST_PLAY_16X:
           case  TS_SEQ_REV_FAST_PLAY_32X:
            vdec_set_trick_mode(p_vHanle, MT_UNF_AVPLAY_TPLAY_DIRECT_BACKWARD, 2);//VDEC_TM_FREV
             break;
           default:
             vdec_set_trick_mode(p_vHanle, MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD, 1);//
             break;
        }
    }

    if(p_TsSeqHandle->last_play_mode < TS_SEQ_REV_FAST_PLAY_2X 
        && play_mode >= TS_SEQ_REV_FAST_PLAY_2X)
    {
      //forward to backward
      //first stream last frame is not complete drop
      p_TsSeqHandle->last_ts_dropbit = 1;
      p_TsSeqHandle->tmp_buffer_offset = 0;
    }
    if(play_mode < TS_SEQ_REV_FAST_PLAY_2X 
        && p_TsSeqHandle->last_play_mode >= TS_SEQ_REV_FAST_PLAY_2X)
    {
      //bakcward to forward
      //first stream last frame is not complete drop
      p_TsSeqHandle->last_ts_dropbit = 1;
      p_TsSeqHandle->tmp_buffer_offset = 0;
    }

    switch(play_mode)
    {
        case TS_SEQ_FAST_PLAY_4X:
        case TS_SEQ_FAST_PLAY_8X:
        case TS_SEQ_FAST_PLAY_16X:
        case TS_SEQ_FAST_PLAY_32X:

            (p_TsSeqHandle->p_forward_parser_func)((void *)p_TsSeqHandle,p_ts_detail);

            break;

        case TS_SEQ_REV_FAST_PLAY_2X:
        case TS_SEQ_REV_FAST_PLAY_4X:
        case TS_SEQ_REV_FAST_PLAY_8X:
        case TS_SEQ_REV_FAST_PLAY_16X:
        case TS_SEQ_REV_FAST_PLAY_32X:

           //need add the restbytes of last time
           if(p_TsSeqHandle->tmp_buffer_offset)
           {
              memcpy((u8 *)(p_TsSeqHandle->p_data + length),
                            p_TsSeqHandle->p_tmp_buffer,
                            p_TsSeqHandle->tmp_buffer_offset);
                p_TsSeqHandle->data_len += p_TsSeqHandle->tmp_buffer_offset;
                p_TsSeqHandle->tmp_buffer_offset = 0;
           }
           (p_TsSeqHandle->p_backward_parser_func)((void *)p_TsSeqHandle,p_ts_detail);

            break;

        default :
            p_TsSeqHandle->totalframes = 0;
            p_TsSeqHandle->last_ts_dropbit = 0;
            p_TsSeqHandle->tmp_buffer_offset = 0;
            p_TsSeqHandle->needdropnum = 0;
            
            p_ts_detail->fragment_offset[0] = 0;
            p_ts_detail->fragment_size[0] = length;
            p_ts_detail->fragment_num = 1;
            break;


    }

    p_TsSeqHandle->seq_parser_status = TS_SEQ_STATUS_IDLE;

        p_TsSeqHandle->last_play_mode = play_mode;
    return 0;
}


int  ts_seq_play(void * p_Handle,char * p_data
        , unsigned int  length,  int    v_pid, ts_seq_play_mode_t play_mode
        , ts_seq_input_mode_t input_mode,unsigned int * p_required)
{

    int i = 0;
    int start_index = 0;
    ts_seq_t *p_TsSeqHandle = (ts_seq_t *)p_Handle;


    p_TsSeqHandle->p_data = p_data + start_index;
    p_TsSeqHandle->data_len = length - start_index;
    p_TsSeqHandle->left_data_len = 0;
    p_TsSeqHandle->play_mode = play_mode;
    p_TsSeqHandle->input_mode = input_mode;
    p_TsSeqHandle->video_pid = v_pid;

    p_TsSeqHandle->seq_parser_status = TS_SEQ_STATUS_BUSY;

    p_TsSeqHandle->gop_num = 0;
    for(i = 0; i < MAX_GOP_NUM; i++)
    {
       p_TsSeqHandle->bpframe_cpnum[i] = 0;
       p_TsSeqHandle->i_frame_len[i] = 0;
    }
    switch(play_mode)
    {
        case TS_SEQ_NORMAL_PLAY:
            break;

        case TS_SEQ_REV_FAST_PLAY_2X:
        case TS_SEQ_REV_FAST_PLAY_4X:
        case TS_SEQ_REV_FAST_PLAY_8X:
        case TS_SEQ_REV_FAST_PLAY_16X:
        case TS_SEQ_REV_FAST_PLAY_32X:

            //(p_TsSeqHandle->p_backward_parser_func)((void *)p_TsSeqHandle);

            break;

        case TS_SEQ_FAST_PLAY_2X:
        case TS_SEQ_FAST_PLAY_4X:
        case TS_SEQ_FAST_PLAY_8X:
        case TS_SEQ_FAST_PLAY_16X:
        case TS_SEQ_FAST_PLAY_32X:

            //(p_TsSeqHandle->p_forward_parser_func)((void *)p_TsSeqHandle);

            break;

        default :
            break;


    }

    p_TsSeqHandle->seq_parser_status = TS_SEQ_STATUS_IDLE;

    return 0;

}

int ts_seq_fetch(void * p_Handle , unsigned char * p_data ,int len)
{
#if 0
    int num = 0;
    ts_seq_t *p_TsSeqHandle = (ts_seq_t *)p_Handle; 

#ifdef PLAY_LESS_MEM
    int i = 0;
    int j = 0;
    int gopnum = p_TsSeqHandle->gop_num;
    int cpsize = 0;
    int leftsize = len;
    int avail = tell_fifo_kw(p_TsSeqHandle);

    //OS_PRINTF("%s,%d,gop num %d\n",__FUNCTION__,__LINE__ ,gopnum);
    if(avail == 0)
    {
       return avail;
    }

      for(i = 0; i < gopnum; i++)
      {
         cpsize =  p_TsSeqHandle->i_frame_len[i];
         if(leftsize >= cpsize)
         {
         num  += read_fifo_kw(p_TsSeqHandle,(char *)p_data + num,cpsize);
         leftsize -= cpsize;

         }
      if(leftsize < cpsize)
       {
        break;
      }
      for(j = 0; j < p_TsSeqHandle->bpframe_cpnum[i]; j++)
         {
           if(leftsize >= cpsize)
           {
             memcpy(p_data + num, p_data + num - cpsize, cpsize);
             leftsize -= cpsize;
             num += cpsize;
           }
            if(leftsize < cpsize)
           {
             break;
           }
         }
    }

      avail = tell_fifo_kw(p_TsSeqHandle);
      if(avail > 0)
      {
       //reset the fifo
        OS_PRINTF("%s,%d,ERROR memory %d is not enough for i frame %d\n"
         ,__FUNCTION__,__LINE__ ,len,avail);
         clear_fifo_kw(p_TsSeqHandle);
       }
    //fwrite(p_data,1,num,pfileOut);
    return num;
#else
    num = read_fifo_kw(p_TsSeqHandle,(char *)p_data,len);
#endif
    return num;
#endif
    return 0;
}

ts_seq_status_t ts_seq_query(void * p_Handle)
{
    ts_seq_t *p_TsSeqHandle = (ts_seq_t *)p_Handle;
    return p_TsSeqHandle->seq_parser_status;
}

void  ts_seq_stop(void * p_Handle)
{
    ts_seq_t *p_TsSeqHandle = (ts_seq_t *)p_Handle;
    MT_BOOL isForward = p_TsSeqHandle->isForward;
    p_TsSeqHandle->isExit  = TRUE;

    do
    {
        if(p_TsSeqHandle->seq_parser_status == TS_SEQ_STATUS_IDLE)
        {
            p_TsSeqHandle->isExit = FALSE;
            break;
        }
        else
        {

#ifdef WIN32
            Sleep(200);
#else
            mtos_task_sleep(200);
#endif
        }

    }while(p_TsSeqHandle->seq_parser_status == TS_SEQ_STATUS_BUSY);


    //reset tmp buffer
    if(isForward)
    {
        p_TsSeqHandle->p_tmp_buffer_start = p_TsSeqHandle->p_tmp_buffer;
        p_TsSeqHandle->p_tmp_buffer_wp =   p_TsSeqHandle->p_tmp_buffer_start;
        p_TsSeqHandle->p_tmp_buffer_rd = p_TsSeqHandle->p_tmp_buffer_start;
        p_TsSeqHandle->p_tmp_buffer_end
            = p_TsSeqHandle->p_tmp_buffer_start + p_TsSeqHandle->tmp_buffer_len - 1;
        p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;
        p_TsSeqHandle->isForward = TRUE;
        p_TsSeqHandle->data_len = 0;
    }
    else
    {
        p_TsSeqHandle->p_tmp_buffer_end = p_TsSeqHandle->p_tmp_buffer;
        p_TsSeqHandle->p_tmp_buffer_start
            = p_TsSeqHandle->p_tmp_buffer + p_TsSeqHandle->tmp_buffer_len - 1;
        p_TsSeqHandle->p_tmp_buffer_wp =   p_TsSeqHandle->p_tmp_buffer_start - 187;
        p_TsSeqHandle->p_tmp_buffer_rd =  p_TsSeqHandle->p_tmp_buffer_wp;

        p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;
        p_TsSeqHandle->isForward = FALSE;
        p_TsSeqHandle->data_len = 0;

    }

    p_TsSeqHandle->isFindFirstIFrame = FALSE;
    p_TsSeqHandle->isFindSecondIFrame = FALSE;
    p_TsSeqHandle->cur_gop_type.base_frame_type = I_B_B_P;
    p_TsSeqHandle->seq_parser_status = TS_SEQ_STATUS_IDLE;
    p_TsSeqHandle->isExit = FALSE;

    p_TsSeqHandle->dropOneGop = FALSE;
    p_TsSeqHandle->drop_segment_num = 0;


    //clear intra ring buffer

    //clear_fifo_kw(p_TsSeqHandle);

    return;
}
#if 0
typedef struct _ufs_file{
/*!
  virtual file system
  */
 void *vfs;
/*!
 public attribute
  */
  u32 file_size;
/*!
  0 is not used  1-FATMAX is fat file, FATMAX+1 is other
  */
  u32 file_id;
}ufs_file_t;

/*!
  op_mode_t
  */
typedef enum  {
/*!
  open exit
  */
 UFS_OPEN = 0x00,
/*!
  read
   */
 UFS_READ = 0x01,
 /*!
  write
  */
 UFS_WRITE = 0x02,
 /*!
  create new
  */
 UFS_CREATE_NEW = 0x04,
 /*!
  create new, if exit the file, cover it.
  */
 UFS_CREATE_NEW_COVER = 0x08,

}op_mode_t;

ufs_file_t ufs_test_file_zs;

extern u8 ufs_open(ufs_file_t * p_fp, const u8 * p_path, op_mode_t mode);
extern u8 ufs_write(ufs_file_t * p_fp, void * p_buff, u32 btr, u32 * p_br);
extern u8 ufs_close(ufs_file_t * p_fp);

static void ryan_dump_data(ufs_file_t *p_fp,const u8 *p_path, op_mode_t mode,
                           unsigned char *p_es_packet,
                           unsigned int *p_data_len,u32 written)
{
    OS_PRINTF("ryan_dump_data!!!\n");
    ufs_open(p_fp, p_path, mode);
    ufs_write(p_fp, p_es_packet, *p_data_len, &written);
    ufs_close(p_fp);
    OS_PRINTF("dump_end!!!\n");
    return ;
}

#endif

int ts_seq_get_vkey_frame_fastswitch(char * p_ts_packet, unsigned int  length,
            video_codec_type_t type, int v_pid, ts_detail_info_t *p_ts_detail)
{
	return 0;
}

int ts_seq_get_vkey_frame_vsb(char * p_ts_packet, unsigned int  length,
            video_codec_type_t type, int v_pid, ts_detail_info_t *p_ts_detail)
{
    unsigned char *p_data       = (unsigned char *)p_ts_packet;

    unsigned int  p_data_len     = length;
    int           cur_video_pid = v_pid;
    int           pid          = 0xffffff;
    unsigned char *p_temp       = NULL;
    unsigned char *p_data_start   = NULL;
    
    unsigned long packet_cnt   = 0;

    int           packet_index = 0;

    video_frame_t cur_video_frame_type = INVALID_FRAME_TYPE; 
    nalu_t        cur_nalu_type = INVALID_NALU_TYPE;
    slice_t       cur_slice_type = INVALID_SLICE_TYPE;
    
    MT_BOOL          vkey_first_flag    = FALSE;

    MT_BOOL          h264_sps_flag     = FALSE;

    int           iframe_with_gop = 0;
    int           pic_struct = 0;
    char          head_frame_type[4] = {0xff,0xff,0xff,0xff};
    unsigned char *p1 = NULL;

    int i = 0;
    
    memset(p_ts_detail,0,sizeof(ts_detail_info_t));
    packet_cnt = p_data_len / 188;

   while(packet_index < packet_cnt)
   {
        p_temp = p_data + packet_index * 188;
        packet_index ++;
/*                              // for tsscan
        if(p_temp[0] != 0x47)
        {
            while(p_temp[i] != 0x47 && i < 188)
            {
                i++;
            }
            p_data = p_data + i; 
            OS_PRINTF("ts_seq_get_vkey_frame_vsb NOT find 0x47 \n");   
            continue;
        }
*/
        p1 = p_temp;
        if( p1 && *p1 != 0x47)
        {
            while(p1 && *p1 != 0x47 && i < 188)
            {
                i++;p1 ++;
            }
            p_data = p_data + i;    
            OS_PRINTF("ts_seq_get_vkey_frame_vsb NOT find 0x47 \n");  
            continue;
        }

        pid = TS_PID(p_temp);
        //video pes
        if(pid == cur_video_pid)
        {   
            cur_video_frame_type = INVALID_FRAME_TYPE;
            cur_nalu_type = INVALID_NALU_TYPE;
            cur_slice_type = INVALID_SLICE_TYPE;
            
            switch(type)
            {
                case MPEG_TYPE_CODE:
                    cur_video_frame_type = x_check_frame_type1(p_temp,
                            &iframe_with_gop, head_frame_type,&pic_struct,188);
                    //find I_FRAME
                    if((cur_video_frame_type == I_FRAME) 
                        && (vkey_first_flag == FALSE))
                    {
                        //OS_PRINTF("@1");
                        if(iframe_with_gop == 5)  
                        {
                            //OS_PRINTF("#5\n");
                            vkey_first_flag = TRUE;
                            p_data_start = p_temp - 188;

                            p_ts_detail->fragment_num = 1;
                            p_ts_detail->fragment_offset[0] = (u32)((void *)p_data_start - (void *)p_ts_packet);
                            p_ts_detail->fragment_size[0] = 1;

                        }
                        else if(iframe_with_gop == 4)  
                        {
                            //OS_PRINTF("#4");
                            p_data_start = p_temp - 188;
                            
                            p_ts_detail->fragment_num = 1;
                            p_ts_detail->fragment_offset[0] = (u32)((void *)p_data_start - (void *)p_ts_packet);
                            p_ts_detail->fragment_size[0] = 1;

                        }
                        else if(iframe_with_gop == 3)
                        {
                            //OS_PRINTF("#3\n");
                            vkey_first_flag = TRUE;
                            //start of I_FRAME
                            p_data_start = p_temp;

                            p_ts_detail->fragment_num = 1;
                            p_ts_detail->fragment_offset[0] = (u32)((void *)p_data_start - (void *)p_ts_packet);
                            p_ts_detail->fragment_size[0] = 1;

                        }
                        else if((iframe_with_gop == 2) && (p_data_start != NULL))
                        {
                            //OS_PRINTF("#2\n");
                            vkey_first_flag = TRUE;

                        }
                        else if(iframe_with_gop == 1)
                        {
                            //OS_PRINTF("#1");
                            p_data_start = p_temp;

                            p_ts_detail->fragment_num = 1;
                            p_ts_detail->fragment_offset[0] = (u32)((void *)p_data_start - (void *)p_ts_packet);
                            p_ts_detail->fragment_size[0] = 1;
                            
                        }
                        
                    }

                    break;
                    
                case  H264_TYPE_CODE:
                    cur_nalu_type = x_check_nalu_type2(p_temp,head_frame_type,188,&cur_slice_type);
                    //find SPS and IDR(I)
                    if((cur_nalu_type == SPS_TYPE) && (h264_sps_flag == FALSE))
                    {
                        //OS_PRINTF("SPS\n");
                        cur_nalu_type = x_check_nalu_type1(p_temp,188,&cur_slice_type);
                        if(cur_nalu_type == IDR_TYPE || cur_slice_type == I_SLICE) 
                        {
                            //OS_PRINTF("@1\n");
                            vkey_first_flag = TRUE;
                        }
                        h264_sps_flag = TRUE;
                        p_data_start = p_temp;

                       p_ts_detail->fragment_num = 1;
                       p_ts_detail->fragment_offset[0] = (u32)((void *)p_data_start - (void *)p_ts_packet);
                       p_ts_detail->fragment_size[0] = 1;
                    }
                    else if((cur_nalu_type == IDR_TYPE || cur_slice_type == I_SLICE) 
                        && (h264_sps_flag == TRUE) && (vkey_first_flag == FALSE))
                    {
                        //OS_PRINTF("@1!\n");
                        vkey_first_flag = TRUE;                 
                    }

                    break;
                case  AVS_TYPE_CODE:
                    break;
                default:
                    break;
            }
        }
        
        //already got the key frame start
        if(vkey_first_flag)
            break;
    }
/*
    OS_PRINTF("vkey_frame num %d ,offset %x \n",p_ts_detail->fragment_num,
                        p_ts_detail->fragment_offset[0]);
*/
    return 0;

}

MT_BOOL ts_seq_get_vkey_frame(unsigned char *p_ts_packet, 
                                             unsigned char *p_es_packet,
                                             unsigned int *p_len, 
                                             unsigned int es_data_buffer_size,
                                             int vpid, 
                                             video_codec_type_t type,
                                             unsigned char **pp_first_kframe_addr)
{
    unsigned char *p_data       = p_ts_packet;
    unsigned char *p_es_data    = p_es_packet;
    unsigned int  *p_data_len     = p_len;
    int           cur_video_pid = vpid;
    unsigned char *p_pes_packet_start = NULL;
    unsigned char *p_temp       = NULL;
    unsigned char *p_data_start   = NULL;
    unsigned char *p_data_mid   = NULL;
    unsigned char *p_data_end   = NULL;
    unsigned long packet_cnt   = 0;
    unsigned long payload      = 0;
    unsigned int  data_mid_len     = 0;
//    unsigned char  *p_es_mid_data     = NULL;
    int           es_payload_len = 0;
    int           ts_header_len = 0;
    int           pes_header_len = 0;
    int           packet_index = 0;
    int           pid          = 0xffffff;
    int           i            = 0;
    int           j            = 0;
    unsigned char *p_es_payload_start = NULL;
    video_frame_t cur_video_frame_type = INVALID_FRAME_TYPE;
    nalu_t        cur_nalu_type = INVALID_NALU_TYPE;
    slice_t       cur_slice_type = INVALID_SLICE_TYPE;
    MT_BOOL          vkey_first_flag    = FALSE;
    MT_BOOL          vkey_second_flag    = FALSE;
    MT_BOOL          vkey_third_flag    = FALSE;
    MT_BOOL          h264_sps_flag     = FALSE;
    MT_BOOL          mid_pos_flay      = FALSE;
    int           frame_mbs_only    = 0;
    int           filed_flag    = 0;
    int           iframe_with_gop = 0;
    int           pic_struct = 0;
    char          head_frame_type[4] = {0xff,0xff,0xff,0xff};
    unsigned char *p1 = NULL;
    //u32           written = 0;
    //u16           file11[256]={0};
    //char          file22[]="ryan_test";
    //ufs_asc2uni(file22, file11);
    
    packet_cnt = *p_data_len / 188;
    while(packet_index < packet_cnt)
    {
        p_temp = p_data + packet_index * 188;
        packet_index ++;
/*                              //  for tsscan
        if(p_temp[0] != 0x47)
        {
            while(p_temp[i] != 0x47 && i < 188)
            {
                i++;
            }
            p_data = p_data + i;    
            continue;
        }
*/
        p1 = p_temp;
        if( p1 && *p1 != 0x47)
        {
            while(p1 && *p1 != 0x47 && i < 188)
            {
                i++;p1 ++;
            }
            p_data = p_data + i;    
            continue;
        }

        pid = TS_PID(p_temp);
        //video pes
        if(pid == cur_video_pid)
        {   
            switch(type)
            {
                case MPEG_TYPE_CODE:
                    cur_video_frame_type = x_check_frame_type1(p_temp,
                            &iframe_with_gop, head_frame_type,&pic_struct,188);
                    //find I_FRAME
                    if((cur_video_frame_type == I_FRAME) 
                        && (vkey_first_flag == FALSE))
                    {
                        //OS_PRINTF("@1");
                        if(iframe_with_gop == 5)  
                        {
                            //OS_PRINTF("#5\n");
                            vkey_first_flag = TRUE;
                            p_data_start = p_temp - 188;
                            *pp_first_kframe_addr = p_temp - 188;
                            cur_video_frame_type = INVALID_FRAME_TYPE;
                        }
                        else if(iframe_with_gop == 4)  
                        {
                            //OS_PRINTF("#4");
                            p_data_start = p_temp - 188;
                            *pp_first_kframe_addr = p_temp - 188;
                            cur_video_frame_type = INVALID_FRAME_TYPE;
                        }
                        else if(iframe_with_gop == 3)
                        {
                            //OS_PRINTF("#3\n");
                            vkey_first_flag = TRUE;
                            //start of I_FRAME
                            p_data_start = p_temp;
                            *pp_first_kframe_addr = p_temp;
                            cur_video_frame_type = INVALID_FRAME_TYPE;
                        }
                        else if((iframe_with_gop == 2) && (p_data_start != NULL))
                        {
                            //OS_PRINTF("#2\n");
                            vkey_first_flag = TRUE;
                            cur_video_frame_type = INVALID_FRAME_TYPE;
                        }
                        else if(iframe_with_gop == 1)
                        {
                            //OS_PRINTF("#1");
                            p_data_start = p_temp;
                            *pp_first_kframe_addr = p_temp;
                            cur_video_frame_type = INVALID_FRAME_TYPE;
                        }
                    }
                    else if(((cur_video_frame_type == I_FRAME)
                        || (cur_video_frame_type == P_FRAME) 
                        || (cur_video_frame_type == B_FRAME))
                        && (vkey_first_flag == TRUE && vkey_second_flag == FALSE))
                    {
                        //OS_PRINTF("@2\n");
                        vkey_second_flag = TRUE;
                        p_data_mid = p_temp;
                        cur_video_frame_type = INVALID_FRAME_TYPE;
                    } 
                    else if(((cur_video_frame_type == I_FRAME)
                        || (cur_video_frame_type == P_FRAME) 
                        || (cur_video_frame_type == B_FRAME))
                        && (vkey_second_flag == TRUE && vkey_third_flag == FALSE))
                    {
                        //OS_PRINTF("@3\n");
                        vkey_third_flag = TRUE;
                        p_data_end = p_temp;
                        cur_video_frame_type = INVALID_FRAME_TYPE;
                    } 
                    break;
                case  H264_TYPE_CODE:
                    cur_nalu_type = x_check_nalu_type2(p_temp,head_frame_type,188,&cur_slice_type);
                    //find SPS and IDR(I)
                    if((cur_nalu_type == SPS_TYPE) && (h264_sps_flag == FALSE))
                    {
                        //OS_PRINTF("SPS\n");
                        cur_nalu_type = x_check_nalu_type1(p_temp,188,&cur_slice_type);
                        if(cur_nalu_type == IDR_TYPE || cur_slice_type == I_SLICE) 
                        {
                            //OS_PRINTF("@1\n");
                            vkey_first_flag = TRUE;
                        }
                        h264_sps_flag = TRUE;
                        p_data_start = p_temp;
                        *pp_first_kframe_addr = p_temp;
                        cur_nalu_type = INVALID_NALU_TYPE;
                        cur_slice_type = INVALID_SLICE_TYPE;
                    }
                    else if((cur_nalu_type == IDR_TYPE || cur_slice_type == I_SLICE) 
                        && (h264_sps_flag == TRUE) && (vkey_first_flag == FALSE))
                    {
                        //OS_PRINTF("@1!\n");
                        vkey_first_flag = TRUE;
                        cur_nalu_type = INVALID_NALU_TYPE;
                        cur_slice_type = INVALID_SLICE_TYPE;
                    }
                    else if((cur_slice_type == I_SLICE
                        || cur_slice_type == P_SLICE || cur_slice_type == P_SLICE_2 
                        || cur_slice_type == B_SLICE || cur_slice_type == B_SLICE_2)
                        && (vkey_first_flag == TRUE && vkey_second_flag == FALSE))
                    {
                        //OS_PRINTF("@2\n");
                        vkey_second_flag = TRUE;
                        p_data_mid = p_temp;
                        cur_nalu_type = INVALID_NALU_TYPE;
                        cur_slice_type = INVALID_SLICE_TYPE;
                    }
                    else if((cur_slice_type == I_SLICE
                        || cur_slice_type == P_SLICE || cur_slice_type == P_SLICE_2 
                        || cur_slice_type == B_SLICE || cur_slice_type == B_SLICE_2)
                        && (vkey_second_flag == TRUE && vkey_third_flag == FALSE))
                    {
                        //OS_PRINTF("@3\n");
                        vkey_third_flag = TRUE;
                        p_data_end = p_temp;
                    }
                    break;
                case  AVS_TYPE_CODE:
                    break;
                default:
                    break;
            }
        }
        
        //already got the key frame start & end 
        if(vkey_first_flag && vkey_second_flag && vkey_third_flag)
            break;
    }
    
    if(!(vkey_first_flag && vkey_second_flag && vkey_third_flag))
    {
        if(vkey_first_flag)
        {
            OS_PRINTF("[ERROR][get vkey]should find one more time!!!\n");
            return FALSE;
        }
        //can not find key frame!!!
        OS_PRINTF("[ERROR][get vkey]can't find key frame in ts buffer!!!\n");
        return FALSE;
    }
    
    //get pes data
    *p_data_len = 0;
    while(p_data_start + 188 <= p_data_end) 
    {
        if(p_data_start >= p_data_mid && mid_pos_flay == FALSE)
        {
            mid_pos_flay = TRUE;
//            p_es_mid_data = p_es_data;
            data_mid_len = *p_data_len;
        }
        pid = TS_PID(p_data_start);
        if(pid == cur_video_pid)
        {
            if (PAYLOAD_UNIT_START(p_data_start) == 1)
            {
                payload = 0;
                p_pes_packet_start = pes_packet_start(p_data_start, &payload);
                
                ts_header_len = p_pes_packet_start  - p_data_start;
                payload = 188 - ts_header_len;
                if ((payload >= 4))
                {
                    p_es_payload_start = pes_data_start(p_pes_packet_start , &payload);
                    pes_header_len = p_es_payload_start - p_pes_packet_start ;
                    es_payload_len = 188 - pes_header_len - ts_header_len;
                    *p_data_len += es_payload_len;
#ifdef WIN32
                    assert(*p_data_len <= es_data_buffer_size);
#else
                    MT_ASSERT(*p_data_len <= es_data_buffer_size);
#endif

                    for(j = 0; j < es_payload_len; j ++)
                    {
                        *p_es_data ++ = *p_es_payload_start ++;
                    }
                }
            }
            else
            {
                payload = 0;
                p_pes_packet_start = pes_packet_start(p_data_start, &payload);
                
                ts_header_len = p_pes_packet_start  - p_data_start;
                es_payload_len = 188 - ts_header_len;
                *p_data_len += es_payload_len;
#ifdef WIN32
                assert(*p_data_len <= es_data_buffer_size);
#else
                MT_ASSERT(*p_data_len <= es_data_buffer_size);
#endif

                for(j = 0; j < es_payload_len; j ++)
                {
                    *p_es_data ++ = *p_pes_packet_start ++;
                }
            }
     
        }
        
        p_data_start += 188;    
    }
    
    switch(type)
    {
        case MPEG_TYPE_CODE:
            if(pic_struct == 3)
            {
                *p_data_len = data_mid_len;
            }
            break;
        case  H264_TYPE_CODE:
            x_check_spsframe((char *)p_es_packet, 0, *p_data_len, &frame_mbs_only);
            if(frame_mbs_only == 1)
            {
                *p_data_len = data_mid_len;
            }
            else
            {
                x_check_slice_type1((char *)p_es_packet, 0, data_mid_len, 
                                      &frame_mbs_only, &filed_flag);
                if(filed_flag == 0)
                {
                    *p_data_len = data_mid_len;
                }
            }
            break;
        case  AVS_TYPE_CODE:
            break;
        default:
            break;
    }

    //ryan_dump_data(&ufs_test_file_zs, file11, UFS_WRITE | UFS_CREATE_NEW_COVER,
    //               p_es_packet, p_data_len, written);
    
    return TRUE;
}
void ts_seq_reset_play_direction(void * p_Handle ,MT_BOOL isForward, video_codec_type_t type)
{
    ts_seq_t *p_TsSeqHandle = (ts_seq_t *)p_Handle;
    p_TsSeqHandle->isForward = isForward;
    //reset tmpbuffer
    if(isForward)
    {
        p_TsSeqHandle->p_tmp_buffer_start = p_TsSeqHandle->p_tmp_buffer;
        p_TsSeqHandle->p_tmp_buffer_wp =   p_TsSeqHandle->p_tmp_buffer_start;
        p_TsSeqHandle->p_tmp_buffer_rd = p_TsSeqHandle->p_tmp_buffer_start;
        p_TsSeqHandle->p_tmp_buffer_end
            = p_TsSeqHandle->p_tmp_buffer_start + p_TsSeqHandle->tmp_buffer_len -1;
        p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;
        p_TsSeqHandle->isForward = TRUE;
        p_TsSeqHandle->data_len = 0;
    }
    else
    {
        p_TsSeqHandle->p_tmp_buffer_end = p_TsSeqHandle->p_tmp_buffer;
        p_TsSeqHandle->p_tmp_buffer_start
            = p_TsSeqHandle->p_tmp_buffer + p_TsSeqHandle->tmp_buffer_len - 1;
        p_TsSeqHandle->p_tmp_buffer_wp =   p_TsSeqHandle->p_tmp_buffer_start - 187;
        p_TsSeqHandle->p_tmp_buffer_rd =  p_TsSeqHandle->p_tmp_buffer_wp;

        p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len;
        p_TsSeqHandle->isForward = FALSE;
        p_TsSeqHandle->data_len = 0;

    }

    switch(type)
    {
        case MPEG_TYPE_CODE:
            p_TsSeqHandle->p_forward_parser_func = ts_forward_gop_parser;
            p_TsSeqHandle->p_backward_parser_func = ts_backward_gop_parser;
            break;
        case  H264_TYPE_CODE:
            p_TsSeqHandle->p_forward_parser_func = ts_forward_sequence_parser;
            p_TsSeqHandle->p_backward_parser_func = ts_backward_sequence_parser;
            break;
        case  AVS_TYPE_CODE:
            // TODO:
            break;

        default:
            p_TsSeqHandle->p_forward_parser_func = ts_forward_gop_parser;
            p_TsSeqHandle->p_backward_parser_func = ts_backward_gop_parser;
            break;
    }

    p_TsSeqHandle->isFindFirstIFrame = FALSE;
    p_TsSeqHandle->isFindFirstIFrameBackward = FALSE;
    p_TsSeqHandle->isFindSecondIFrame = FALSE;
    p_TsSeqHandle->cur_gop_type.base_frame_type = I_B_B_P;
    p_TsSeqHandle->seq_parser_status = TS_SEQ_STATUS_IDLE;
    p_TsSeqHandle->isExit = FALSE;

    p_TsSeqHandle->dropOneGop = FALSE;
    p_TsSeqHandle->drop_segment_num = 0;
    p_TsSeqHandle->cur_video_type = type;

    //clear intra ring buffer

    //clear_fifo_kw(p_TsSeqHandle);
    return;

}

void ts_seq_set_sequence_parser(void * pHandle,video_codec_type_t type)
{
    ts_seq_t *p_TsSeqHandle = (ts_seq_t *)pHandle;
    p_TsSeqHandle->cur_video_type = type;

    switch(type)
    {
        case MPEG_TYPE_CODE:
            p_TsSeqHandle->p_forward_parser_func =ts_forward_gop_parser ;
            p_TsSeqHandle->p_backward_parser_func = ts_backward_gop_parser;
            break;

        case H264_TYPE_CODE:
            p_TsSeqHandle->p_forward_parser_func = ts_forward_sequence_parser;
            p_TsSeqHandle->p_backward_parser_func = ts_backward_sequence_parser;
            break;
        case AVS_TYPE_CODE:
            // TODO:
            break;
        default:
            break;
    }

}


unsigned int ts_get_first_pts(unsigned char *p_data, 
                                             unsigned int length, 
                                             unsigned int vpid)
{
  u32 pts = 0;
  u8 *p_temp = NULL;  
  u32 left_data_len = length;
  u32 packet_index = 0;
  u32 packet_cnt = left_data_len / 188;
  u32 pid = 0;
  
  while(packet_index < packet_cnt)
  {
  
      p_temp = p_data + packet_index * 188;
      packet_index ++;
  
      if(p_temp[0] != 0x47)
      {
          OS_PRINTF("[ERROR][do_parse_gop] find invalid ts paket !!!\n");
          OS_PRINTF("[ERROR][do_parse_gop] not find sync code [0x47] !!!\n");
          continue;
      }
  
      pid = TS_PID(p_temp);
      //video pes
      if(pid == vpid)
      {
          //cur_video_frame_type = x_check_frame_type(p_temp,188);
          unsigned long ts_header_len = 0;
          unsigned long payload = 0;
          u8 *p_pes_data_start = NULL;
          //ts start bit must set
          if(PAYLOAD_UNIT_START(p_temp))
          {
            p_pes_data_start = pes_packet_start(p_temp, &payload);
            ts_header_len = p_pes_data_start - p_temp;            
            payload = 188 - ts_header_len;

            if(payload > 13)
            {
               //find the pts
               pts = (p_pes_data_start[9]  << 24)
                         + (p_pes_data_start[10] << 16)
                         + (p_pes_data_start[11] << 8)
                         + p_pes_data_start[12];

               break;
            }            
          }
      }
  }
  return pts;
}

