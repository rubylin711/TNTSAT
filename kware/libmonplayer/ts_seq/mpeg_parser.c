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

//#define OS_PRINTF mtos_printk

#define TS_SEQ_DEBUG  printf

#define MT_ASSERT

//#define DEBUG_MPEG_PARSER
//#define PKT_STATSTICS
#ifdef   DEBUG_MPEG_PARSER
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

static int lastdatasize = 0;
static int lastdatapos = 0;
static int last_gop_start_find = 0;
static int picture_first_field = 0;//default is top field
static int picture_current_field = 1;//default is top field
static int last_frame_type_err = 0;
video_frame_t  x_check_frame_type(char * p_ts_packet , int len/*188*/)
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
    int pic_struct = 0;

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

            lastdatapos = p_data - p_ts_packet;
            is_picture_start_code = IS_PICTURE_START_CODE(p_data);
            is_gop_start_code = IS_GOP_START_CODE(p_data);

            if(last_frame_type_err)
            {
               //is_picture_start_code = 1;
            }
           
            while((i + 3 < es_payload_len) && ((!is_picture_start_code) && (!is_gop_start_code)))
            {
                p_data ++;
                i ++;
                is_picture_start_code = IS_PICTURE_START_CODE(p_data);
                is_gop_start_code = IS_GOP_START_CODE(p_data);

                if(is_gop_start_code)
               {
                  mpeg_picture_coding_extension_parser(p_data,es_payload_len - i,&pic_struct);
                   
                   if(pic_struct != 3)
                   {
                      picture_first_field = 1;
                   
                      picture_current_field = 1;

                   }
                   else
                   {
                      picture_first_field = 0;
                   
                      picture_current_field = 1;
                   }
                }              
             }
            lastdatasize = p_data - p_ts_packet - lastdatapos;
            
            //find gop start code
            if(is_gop_start_code)
            {
                picture_coding_type = I_FRAME;
                lastdatasize = 0;
                last_gop_start_find = 1;
            }
            //find picture start code
            else if(is_picture_start_code)
            {
                if((i + 5) < es_payload_len)
                {
                    picture_coding_type = 0xff;
                    if(last_frame_type_err)
                    {
                      //picture_coding_type = p_data[1] & 0x38;
                    }
                    else
                    {
                      picture_coding_type = p_data[5] & 0x38;
                    }
                    picture_coding_type >>= 3;
                    last_frame_type_err = 0;
                    //MY_DEBUG("picture_coding_type:%d\n",picture_coding_type);

                    if(picture_coding_type == I_FRAME)
                    {
                        if(last_gop_start_find)
                            picture_coding_type = 0;
                        //MY_DEBUG(" IIIII picture_coding_type:%d\n",picture_coding_type);
                        //break;
                    }
                    else if(picture_coding_type == P_FRAME){
                        //MY_DEBUG("PPPPP picture_coding_type:%d\n",picture_coding_type);
                        //break;
                    }
                    else if(picture_coding_type == B_FRAME){
                        //MY_DEBUG("BBBBBBBB  picture_coding_type:%d\n",picture_coding_type);
                        //break;
                    }
                    else
                    {
                      //MY_DEBUG("NNNNNNNNNNNN  picture_coding_type:%d\n",picture_coding_type);
                      //break;
                    }

                }
                else if((i + 5) >=  es_payload_len)
                {
//                    find_code_type = FALSE;
                    picture_coding_type = P_FRAME;
                    //last_frame_type_err = 1;
                   //break;
                }

                last_gop_start_find = 0;
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

static int x_split_hd_frame(ts_seq_t * p_TsSeqHandle, video_frame_t video_frame_type)
{
//check the frame drop or not
  gop_t *p_gop = &p_TsSeqHandle->cur_gop_type;
  int index = p_gop->len;
  //int lastgoplen = p_TsSeqHandle->last_gop_len;

  video_frame_t cur_video_frame_type = video_frame_type;

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
        if(cur_video_frame_type != I_FRAME)
        {
           p_gop->frame_dropbit[index] = 1;
        }
        break;
     }  
     
     default:
        break;

  }

  return 0;
}

static int x_split_frame(ts_seq_t * p_TsSeqHandle, video_frame_t video_frame_type)
{
//check the frame drop or not
  gop_t *p_gop = &p_TsSeqHandle->cur_gop_type;
  int index = p_gop->len;
  unsigned int total_index = p_TsSeqHandle->totalframes;
  //int lastgoplen = p_TsSeqHandle->last_gop_len;

  video_frame_t cur_video_frame_type = video_frame_type;

  switch(p_TsSeqHandle->play_mode)
  {
     case TS_SEQ_SLOW_PLAY_2X:
     case TS_SEQ_SLOW_PLAY_4X:
     case TS_SEQ_NORMAL_PLAY:
     case TS_SEQ_FAST_PLAY_2X:
         //MT_ASSERT(0);
         break;
#if 0
     case TS_SEQ_FAST_PLAY_4X:
     {
        int dropmask = 2;
        if(total_index % dropmask)
        {
        //need drop frame
          if(cur_video_frame_type == B_FRAME)
          {
            p_gop->frame_dropbit[index] = 1;
          }
          else if(cur_video_frame_type == P_FRAME)
          {
             //find the previous B frame and drop
             int i = index;
             int needdropone = 1;
             while(i)
             {
                 i--;
                 if(p_gop->frame_dropbit[i] == 0 &&
                  p_gop->frame_type_array[i] == B_FRAME)
                  {
                    p_gop->frame_dropbit[i] = 1;

                    needdropone = 0;
                    break;
                  }
             }
             if(needdropone)
             {
                p_TsSeqHandle->needdropnum ++;
             }
          }
          else //this is i frame
          {
             //find the previous B frame and drop
             //drop the next b frame
             p_TsSeqHandle->needdropnum ++;

          }
        }
        else//not drop frame
        {
           if(p_TsSeqHandle->needdropnum)
            {
                if(cur_video_frame_type == B_FRAME)
                {
                   p_gop->frame_dropbit[index] = 1;

                   p_TsSeqHandle->needdropnum --;
                }
            }
        }
        break;
     }
#endif
     case TS_SEQ_FAST_PLAY_4X:
     case TS_SEQ_FAST_PLAY_8X:
     case TS_SEQ_FAST_PLAY_16X:
     {
        int dropmask = 4;
        if(p_TsSeqHandle->play_mode == TS_SEQ_FAST_PLAY_16X)
            dropmask = 8;
        if(p_TsSeqHandle->play_mode == TS_SEQ_FAST_PLAY_4X)
            dropmask = 2;
        if(total_index % dropmask)
        {
        //need drop frame
          if(cur_video_frame_type == B_FRAME)
          {
            p_gop->frame_dropbit[index] = 1;
          }
          else if(cur_video_frame_type == P_FRAME)
          {
             //find the previous B frame and drop
             int i = index;
             int needdropone = 1;
             while(i)
             {
                 i--;
                 if(p_gop->frame_dropbit[i] == 0 &&
                  p_gop->frame_type_array[i] == B_FRAME)
                  {
                    p_gop->frame_dropbit[i] = 1;

                    if(p_TsSeqHandle->needdropnum)
                    {
                      p_TsSeqHandle->needdropnum --;
                    }
                    else
                    {
                      needdropone = 0;
                      break;
                    }
                  }
             }
             if(needdropone)
             {
                p_TsSeqHandle->needdropnum ++;
             }
          }
          else //this is i frame
          {
             //find the previous B frame and drop
             int i = index;
             int needdropone = 1;
             while(i)
             {
                 i--;
                 if(p_gop->frame_dropbit[i] == 0 &&
                  p_gop->frame_type_array[i] == B_FRAME)
                  {
                    p_gop->frame_dropbit[i] = 1;

                    if(p_TsSeqHandle->needdropnum)
                    {
                      p_TsSeqHandle->needdropnum --;
                    }
                    else
                    {
                      needdropone = 0;
                      break;
                    }
                  }
             }
             if(needdropone)
             {
                i = index;
                //find the previous P frame  NEXT TO I and drop
                while(i)
                {
                    i--;
                    if(p_gop->frame_dropbit[i] == 0 &&
                     p_gop->frame_type_array[i] == P_FRAME)
                     {
                       p_gop->frame_dropbit[i] = 1;
   
                      if(p_TsSeqHandle->needdropnum)
                      {
                        p_TsSeqHandle->needdropnum --;
                      }
                      else
                      {
                        needdropone = 0;
                        break;
                      }
                   }
                }
             } 
             //remember drop the next b frame    
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
                if(cur_video_frame_type == B_FRAME)
                {
                   p_gop->frame_dropbit[index] = 1;

                   p_TsSeqHandle->needdropnum --;
                }
                else if(cur_video_frame_type == P_FRAME)
                {
                   //find the previous B frame and drop
                   int i = index;
//                   int needdropone = 1;
                   while(i)
                   {
                       i--;
                       if(p_gop->frame_dropbit[i] == 0 &&
                        p_gop->frame_type_array[i] == B_FRAME)
                        {
                          p_gop->frame_dropbit[i] = 1;
      
                          if(p_TsSeqHandle->needdropnum)
                          {
                            p_TsSeqHandle->needdropnum --;
                          }
                          else
                          {
//                            needdropone = 0;
                            break;
                          }
                        }
                   }
                }
                else// iframe
                {
                   //find the previous B frame and drop
                   int i = index;
                   int needdropone = 1;
                   while(i)
                   {
                       i--;
                       if(p_gop->frame_dropbit[i] == 0 &&
                        p_gop->frame_type_array[i] == B_FRAME)
                        {
                          p_gop->frame_dropbit[i] = 1;
      
                          if(p_TsSeqHandle->needdropnum)
                          {
                            p_TsSeqHandle->needdropnum --;
                          }
                          else
                          {
                            needdropone = 0;
                            break;
                          }
                        }
                   }
                   if(needdropone)
                   {
                      i = index;
                      //find the previous P frame  NEXT TO I and drop
                      while(i)
                      {
                          i--;
                          if(p_gop->frame_dropbit[i] == 0 &&
                           p_gop->frame_type_array[i] == P_FRAME)
                           {
                             p_gop->frame_dropbit[i] = 1;
         
                            if(p_TsSeqHandle->needdropnum)
                            {
                              p_TsSeqHandle->needdropnum --;
                            }
                            else
                            {
                              needdropone = 0;
                              break;
                            }
                         }
                      }
                   } 
                }
            }
        }
        break;
     }        
     case TS_SEQ_FAST_PLAY_32X:
      {
        int dropmask = 16;
        if(total_index % dropmask)
        {
        //need drop frame
          if(cur_video_frame_type == B_FRAME || cur_video_frame_type == P_FRAME)
          {
            p_gop->frame_dropbit[index] = 1;
          }
          else //this is i frame
          {
/*
             if(p_TsSeqHandle->needdropnum)
             {
               p_gop->frame_dropbit[index] = 1;
             }
             else
             {
                p_TsSeqHandle->needdropnum ++;
             }
*/
             p_TsSeqHandle->needdropnum ++;
          }
        }
        else//not drop frame
        {
            if(cur_video_frame_type != I_FRAME)
            {
              p_gop->frame_dropbit[index] = 1;
 
            }

/*
           if(p_TsSeqHandle->needdropnum)
            {

               p_gop->frame_dropbit[index] = 1;

               p_TsSeqHandle->needdropnum --;

            }
           else if(cur_video_frame_type != I_FRAME)
           {
            p_gop->frame_dropbit[index] = 1;
           }
*/
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
          if(cur_video_frame_type != I_FRAME)
          {
            p_gop->frame_dropbit[index] = 1;          
          }
          else
          {
             //find the previous B P frame and drop
             int i = index;
             int needdropone = 1;
             while(i)
             {
                 i--;
                 if(p_gop->frame_dropbit[i] == 0)
                  {
                    p_gop->frame_dropbit[i] = 1;
                    
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
                if(cur_video_frame_type != I_FRAME)
                {
                   p_gop->frame_dropbit[index] = 1;

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
          if(cur_video_frame_type == B_FRAME || cur_video_frame_type == P_FRAME)
          {
            p_gop->frame_dropbit[index] = 1;
          }
          else //this is i frame
          {
             //find the previous B frame and drop
             if(p_TsSeqHandle->needdropnum)
             {
               p_gop->frame_dropbit[index] = 1;
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

               p_gop->frame_dropbit[index] = 1;

               p_TsSeqHandle->needdropnum --;

            }
           else if(cur_video_frame_type != I_FRAME)
           {
            p_gop->frame_dropbit[index] = 1;
           }
        }
        break;
     }
     
     default:
        break;

  }

  return 0;
}

#if 0
static int do_revert_parse_gop(ts_seq_t * p_Handle)
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

        pid = TS_PID(p_temp);
        //video pes
        if(pid == cur_video_pid)
        {
            cur_video_frame_type = x_check_frame_type(p_temp,188);
            //video pes && start unit && Key frame
            if(cur_video_frame_type == I_FRAME)
            {
                if(p_TsSeqHandle->isFindFirstIFrame  == FALSE
                        && p_TsSeqHandle->isFindFirstIFrameBackward == FALSE)
                {
                    //find the tail of current gop and then break
                    p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
                    p_TsSeqHandle->isFindFirstIFrame = TRUE;
                    p_TsSeqHandle->isFindFirstIFrameBackward = TRUE;
                    p_TsSeqHandle->left_data_len = 0;
                    array_index = 0;

                    p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp - 188;
                    p_TsSeqHandle->tmp_buffer_free_space -= 188;
                    break;
                }
                else if(p_TsSeqHandle->isFindSecondIFrame == FALSE)
                {
                    //find the  head  of current gop
                    p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
                    p_TsSeqHandle->isFindSecondIFrame = TRUE;
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
                    //OS_PRINTF("1p_tmp_buffer_wp is %d",p_tmp_buffer_wp);
                    p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index]
                        = p_tmp_buffer_wp;

                    p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp - 188;
                    p_TsSeqHandle->tmp_buffer_free_space -= 188;
                    break;
                }        }
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
                //OS_PRINTF("2p_tmp_buffer_wp is %d",p_tmp_buffer_wp);
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
                //OS_PRINTF("3p_tmp_buffer_wp is %d",p_tmp_buffer_wp);
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
#endif
static int do_parse_gop1(ts_seq_t * p_Handle)
{
    ts_seq_t *p_TsSeqHandle = p_Handle;
    char *p_data =                     p_TsSeqHandle->p_data;
    int        data_len =                   p_TsSeqHandle->data_len;
    int        cur_video_pid =           p_TsSeqHandle->video_pid;
//    char *p_tmp_buffer_wp =    p_TsSeqHandle->p_tmp_buffer_wp;
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

        pid = TS_PID(p_temp);
        //video pes
        if(pid == cur_video_pid)
        {
            cur_video_frame_type = x_check_frame_type(p_temp,188);

            if(cur_video_frame_type == I_FRAME ||
                cur_video_frame_type == P_FRAME ||
                cur_video_frame_type == B_FRAME)
            {

                if(picture_first_field == 1)
                {
                   if(picture_current_field == 2)
                    {
                      picture_current_field = 1;
                      continue;
                    }
                   else
                    {
                      picture_current_field = 2;
                    }
                }
                //printf("frm type %d\n",cur_video_frame_type);
               array_index = p_TsSeqHandle->cur_gop_type.len;
               p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index]
                   = p_temp;

               p_TsSeqHandle->cur_gop_type.frame_type_array[array_index] = cur_video_frame_type;

               p_TsSeqHandle->cur_gop_type.frame_startpos[array_index]
                     = lastdatapos;
               p_TsSeqHandle->cur_gop_type.frame_deletesize[array_index]
                     = lastdatasize;

               if(p_TsSeqHandle->input_mode == TS_SEQ_INPUT_TS_HD)
               {
                 x_split_hd_frame(p_TsSeqHandle,cur_video_frame_type);
               }
               else
               {
                  x_split_frame(p_TsSeqHandle,cur_video_frame_type);
               }
               
#ifdef DEBUG_MPEG_PARSER
               OS_PRINTF("FRME index %d ,type %d ,dropbit %d, addr %x\n",
                      p_TsSeqHandle->cur_gop_type.len,
                      cur_video_frame_type,
                      p_TsSeqHandle->cur_gop_type.frame_dropbit[array_index],
                      p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index]);
#endif
               p_TsSeqHandle->totalframes ++;
               p_TsSeqHandle->cur_gop_type.len ++;

            }
            else ////vidoe pes && none start unit
            {
                 ;//do nothing;
            }
        }
        else ////none video pes
        {
           ;//do nothing;
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
static video_frame_t last_video_frame_type = 0;
static int do_parse_gop(ts_seq_t * p_Handle,ts_detail_info_t *p_ts_detail)
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

        pid = TS_PID(p_temp);
        //video pes
        if(pid == cur_video_pid)
        {
            cur_video_frame_type = x_check_frame_type(p_temp,188);
            //video pes && start unit && Key frame
            if(cur_video_frame_type == I_FRAME)
            {
                p_TsSeqHandle->last_video_frame_type = I_FRAME;
                if(p_TsSeqHandle->isFindFirstIFrame == FALSE
                        &&  p_TsSeqHandle->isFindSecondIFrame == FALSE)
                {
#if 1
                    p_TsSeqHandle->isFindFirstIFrame = TRUE;


#else
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

                    p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index]
                        = p_tmp_buffer_wp;

                    p_TsSeqHandle->p_tmp_buffer_wp = p_tmp_buffer_wp + 188;
                    p_TsSeqHandle->tmp_buffer_free_space -= 188;

#endif
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
                    p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index]
                        = p_tmp_buffer_wp;

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
                    p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index]
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
            //video pes && start unit && B frame
            else if(cur_video_frame_type == B_FRAME)
            {
                last_video_frame_type = B_FRAME;
                if(p_TsSeqHandle->isFindFirstIFrame)
                {
                    p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_wp;
                    array_index = p_TsSeqHandle->cur_gop_type.len;
                    p_TsSeqHandle->cur_gop_type.frame_type_array[array_index] = B_FRAME;
                    p_TsSeqHandle->cur_gop_type.video_packet_start_array[array_index]
                        = p_tmp_buffer_wp;
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
                //the gop is big than tmp buf
               //most because of the none start unit stream!!!!
               //reset
/*              if(p_tmp_buffer_wp + 188  >  p_TsSeqHandle->p_tmp_buffer_end)
              {
                int i = 0;
                //reset field of p_TsSeqHandle
                p_TsSeqHandle->isFindFirstIFrame = FALSE;
                p_TsSeqHandle->isFindSecondIFrame = FALSE;

                for(i = 0; i < MAX_GOP_LEN; i++)
                {
                    p_TsSeqHandle->cur_gop_type.video_packet_start_array[i] = NULL;
                }

                p_TsSeqHandle->tmp_buffer_free_space = p_TsSeqHandle->tmp_buffer_len ;
                p_TsSeqHandle->p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_start ;
                p_TsSeqHandle->cur_gop_type.len = 0;

                memset(p_TsSeqHandle->cur_gop_type.frame_type_array,
                                 0 ,MAX_GOP_LEN * sizeof(video_frame_t));
                last_video_frame_type = 0;
              }
*/
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
#endif
#if 0
int x_split_gop_IBBP_2X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index =0, i = 0;
    int framelen =0, goplen = 0;
    int cpBNum = 0, cpBMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
#if 0
    if(isInitSplitFile == FALSE)
    {
        pfileOut = fopen("split.ts","wb");
        if(pfileOut == NULL)
        {
            MY_DEBUG("fail to open split.ts!!!\n");
            return ;
        }

        isInitSplitFile = TRUE;
    }
#endif

    //find the first i frame,normal is index 0
    for(index = 0 ; index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BBP BBP BBP BBP BB*/
    /*2X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest ,
      else if goplen around 8 ,cp the first i, then drop rest gops,
      else goplen > 12 ,cp first i p,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpBMax = 0;
            cpBNum = 0;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpBMax = 1;
            cpBNum = 0;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpBMax = 2;
            cpBNum = 0;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpBMax = 0;
        cpBNum = 0;
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
                //do record the b frame
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                //fwrite(p_TsSeqHandle->cur_gop_type.video_packet_start_array[index]
                ,1,framelen,pfileOut);
                write_fifo_kw(p_TsSeqHandle
                        ,p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);


                cpBNum ++;
            }
        }
        else{
            framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
            //fwrite(p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],1
            ,framelen,pfileOut);
            write_fifo_kw(p_TsSeqHandle
                    , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
        }
        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        //end when the second i frame
    }

    return 0;

}

int x_split_gop_IBBP_4X(ts_seq_t *  p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index =0, i = 0 ;
    //int framelen;
    int framelen =0, goplen = 0;
    int cpNum = 0,cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0; index < MAX_GOP_LEN; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BBP BBP BBP BBP BB*/
    /*4X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest ,
      else if goplen around 8 ,cp the first i, then drop rest gops,
      else goplen > 12 ,cp first i p,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 2;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 3;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        ,p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }
        else{
            //i frame ,pframe
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        ,p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }
        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }
    return 0;
}
int x_split_gop_IBBP_8X(ts_seq_t  * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ;
    int i = 0;
    //int framelen;
    int framelen = 0;
    int goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;

    //find the first i frame,normal is index 0
    for(index = 0 ; index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BBP BBP BBP BBP BB*/
    /*8X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 1 gops,
      else if goplen around 8 ,cp the first i, then drop rest gops,
      else goplen > 12 ,cp first i p,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 2;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    }
    return 0;
}

int x_split_gop_IBBP_16X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index =0;
    int i = 0;
    //int framelen;
    int framelen = 0, goplen = 0;
    int cpNum = 0 , cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;

    //find the first i frame,normal is index 0
    for(index = 0 ; index < MAX_GOP_LEN; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BBP BBP BBP BBP BB*/
    /*16X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 3 gops,
      else if goplen around 8 ,cp the first i, hen drop rest and the next 1 gops,
      else goplen > 12 ,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 3;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}

int x_split_gop_IBBP_32X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 , i = 0 ;
    //int framelen;
    int framelen = 0, goplen = 0;
    int cpNum =0 , cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;

    //find the first i frame,normal is index 0
    for(index = 0; index < MAX_GOP_LEN  ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BBP BBP BBP BBP BB*/
    /*32X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 7 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 3 gops,
      else goplen > 12 ,then drop rest and the next 1 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 7;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 5;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}

int x_split_gop_IBBP_64X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0, i = 0;
    //int framelen;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;

    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN ;index ++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BBP BBP BBP BBP BB*/
    /*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 15;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 7;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 3;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}

int x_split_gop_IBBP(ts_seq_t * p_Handle,ts_seq_play_mode_t play_mode)
{
    switch(play_mode)
    {
        case TS_SEQ_FAST_PLAY_2X:
        case TS_SEQ_REV_FAST_PLAY_2X:
            x_split_gop_IBBP_2X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_4X:
        case TS_SEQ_REV_FAST_PLAY_4X:
            x_split_gop_IBBP_4X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_8X:
        case TS_SEQ_REV_FAST_PLAY_8X:
            x_split_gop_IBBP_8X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_16X:
        case TS_SEQ_REV_FAST_PLAY_16X:
            x_split_gop_IBBP_16X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_32X:
        case TS_SEQ_REV_FAST_PLAY_32X:
            x_split_gop_IBBP_32X(p_Handle);
            break;


            // TODO:   FF4X  RF4X  FF8X  RF8X  FF16X  RF16X ...
        default:
            break;
    }
    return 0;
}

int x_split_gop_IBP_2X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;

    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BBP BBP BBP BBP BB*/
    /*4X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest ,
      else if goplen around 8 ,cp the first i, then drop rest gops,
      else goplen > 12 ,cp first i p,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        p_TsSeqHandle->drop_segment_num = 0;

        cpMax = goplen /2;

    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame ,pframe
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }
    return 0;
}
int x_split_gop_IBP_4X(ts_seq_t *  p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0; index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BBP BBP BBP BBP BB*/
    /*4X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest ,
      else if goplen around 8 ,cp the first i, then drop rest gops,
      else goplen > 12 ,cp first i p,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 2;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 3;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame ,pframe
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }
        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

    }
    return 0;
}
int x_split_gop_IBP_8X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;

    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BP BP BP BP B*/
    /*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 2;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}

int x_split_gop_IBP_16X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0; index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BP BP BP BP B*/
    /*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 3;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}

int x_split_gop_IBP_32X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0; index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BP BP BP BP B*/
    /*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 7;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 3;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}

int x_split_gop_IBP_64X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;

    //find the first i frame,normal is index 0
    for(index = 0; index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BP BP BP BP B*/
    /*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 15;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 7;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 3;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }
    for(i = 0 ; i < goplen ;  i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame
        }
        else if(cur_video_frame_type == P_FRAME)
        {
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}
int x_split_gop_IBP(ts_seq_t * p_Handle, ts_seq_play_mode_t play_mode)
{
    switch(play_mode)
    {
        case TS_SEQ_FAST_PLAY_2X:
        case TS_SEQ_REV_FAST_PLAY_2X:
            x_split_gop_IBP_2X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_4X:
        case TS_SEQ_REV_FAST_PLAY_4X:
            x_split_gop_IBP_4X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_8X:
        case TS_SEQ_REV_FAST_PLAY_8X:
            x_split_gop_IBP_8X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_16X:
        case TS_SEQ_REV_FAST_PLAY_16X:
            x_split_gop_IBP_16X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_32X:
        case TS_SEQ_REV_FAST_PLAY_32X:
            x_split_gop_IBP_32X(p_Handle);
            break;


            // TODO:   FF4X  RF4X  FF8X  RF8X  FF16X  RF16X ...
        default:
            break;
    }
    return 0;
}



int x_split_gop_IPIP_2X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;

    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I PPPPPPPPPPPPPPPPPP*/
    /*4X if goplen around 4 ==> I PPP  ,cp the first i,then drop rest ,
      else if goplen around 8 ,cp the first i, then drop rest gops,
      else goplen > 12 ,cp first i p,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        p_TsSeqHandle->drop_segment_num = 0;
        //GOPLEN >= 2
        cpMax = goplen / 2;

    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame ,pframe
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }
    return 0;
}

int x_split_gop_IPIP_4X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;

    //find the first i frame,normal is index 0
    for(index = 0; index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I PPPPPPPPPPPPPPPPPP*/
    /*4X if goplen around 4 ==> I PPP  ,cp the first i,then drop rest ,
      else if goplen around 8 ,cp the first i, then drop rest gops,
      else goplen > 12 ,cp first i p,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
        else if(goplen > 5 && goplen < 10)
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 2;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 3;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;
        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame
        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame ,pframe
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        //end when the second i frame

    }
    return 0;
}
int x_split_gop_IPIP_8X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0; index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I PPPPPPPPPPPP*/
    /*8X if goplen around 4 ==> I PPP  ,cp the first i,then drop rest and the next 1 gops,
      else if goplen around 8 ,cp the first i, then drop rest gops,
      else goplen > 12 ,cp first i p,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 2;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }
    return 0;
}

int x_split_gop_IPIP_16X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I PPPPPPPPPPPPPPPPPPP*/
    /*16X if goplen around 4 ==> I PPP  ,cp the first i,then drop rest and the next 3 gops,
      else if goplen around 8 ,cp the first i, hen drop rest and the next 1 gops,
      else goplen > 12 ,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 3;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}

int x_split_gop_IPIP_32X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I PPPPPPPPPPPPPPPPPPP*/
    /*32X if goplen around 4 ==> IPPPP  ,cp the first i,then drop rest and the next 7 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 3 gops,
      else goplen > 12 ,then drop rest and the next 1 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 7;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 5;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}

int x_split_gop_IPIP_64X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN;index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I PPP PP PPP PPP PP*/
    /*64X if goplen around 4 ==> I PPP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 15;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 7;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 3;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}

int x_split_gop_IPIP(ts_seq_t * p_Handle,ts_seq_play_mode_t play_mode)
{
    switch(play_mode)
    {
        case TS_SEQ_FAST_PLAY_2X:
        case TS_SEQ_REV_FAST_PLAY_2X:
            x_split_gop_IPIP_2X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_4X:
        case TS_SEQ_REV_FAST_PLAY_4X:
            x_split_gop_IPIP_4X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_8X:
        case TS_SEQ_REV_FAST_PLAY_8X:
            x_split_gop_IPIP_8X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_16X:
        case TS_SEQ_REV_FAST_PLAY_16X:
            x_split_gop_IPIP_16X(p_Handle);
            break;

        case TS_SEQ_FAST_PLAY_32X:
        case TS_SEQ_REV_FAST_PLAY_32X:
            x_split_gop_IPIP_32X(p_Handle);
            break;


            // TODO:   FF4X  RF4X  FF8X  RF8X  FF16X  RF16X ...
        default:
            break;
    }
    return 0;
}

int x_split_gop_SPECIAL(ts_seq_t * p_TsSeqHandle ,ts_seq_play_mode_t play_mode)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0 ,goplen = 0;
    int IframeNum =0;
    int BframeNum = 0;
    int PframeNum = 0;
    int cpINum = 0;
    int cpBNum = 0 ;
    int cpPNum = 0;
    int cpIMax = 0,cpBMax = 0,cpPMax = 0,cpMax = 0;

    goplen = p_TsSeqHandle->cur_gop_type.len;

    //find the first i frame,normal is index 0
    for(i = 0; i < goplen ; i++)
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
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I PP PP PP PP P*/
    /*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpIMax = 1;
        switch(play_mode)
        {
            case TS_SEQ_FAST_PLAY_2X:
            case TS_SEQ_REV_FAST_PLAY_2X:
                cpMax = goplen / 2;
                p_TsSeqHandle->drop_segment_num = 0;

                break;
            case TS_SEQ_FAST_PLAY_4X:
            case TS_SEQ_REV_FAST_PLAY_4X:
                cpMax = 1;
                if(goplen < 6)
                {
                    p_TsSeqHandle->drop_segment_num = 0;
                }
                else if(goplen > 5 && goplen < 12)
                {
                    p_TsSeqHandle->drop_segment_num = 0;
                    cpMax = 2;
                }
                else
                {
                    p_TsSeqHandle->drop_segment_num = 0;
                    cpMax = 3;
                }

                break;
            case TS_SEQ_FAST_PLAY_8X:
            case TS_SEQ_REV_FAST_PLAY_8X:
                cpMax = 1;
                if(goplen < 6)
                {
                    p_TsSeqHandle->drop_segment_num = 1;
                }
                else if(goplen > 5 && goplen < 12)
                {
                    p_TsSeqHandle->drop_segment_num = 0;
                }
                else
                {
                    p_TsSeqHandle->drop_segment_num = 0;
                    cpMax = 2;
                }

                break;
            case TS_SEQ_FAST_PLAY_16X:
            case TS_SEQ_REV_FAST_PLAY_16X:
                cpMax = 1;
                if(goplen < 6)
                {
                    p_TsSeqHandle->drop_segment_num = 3;
                }
                else if(goplen > 5 && goplen < 12)
                {
                    p_TsSeqHandle->drop_segment_num = 1;
                }
                else
                {
                    p_TsSeqHandle->drop_segment_num = 0;
                }

                break;
            case TS_SEQ_FAST_PLAY_32X:
            case TS_SEQ_REV_FAST_PLAY_32X:
                cpMax = 1;
                if(goplen < 6)
                {
                    p_TsSeqHandle->drop_segment_num = 7;
                }
                else if(goplen > 5 && goplen < 12)
                {
                    p_TsSeqHandle->drop_segment_num = 3;
                }
                else
                {
                    p_TsSeqHandle->drop_segment_num = 1;
                }
                break;
            default:
                break;

        }

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
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

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
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle->p_TsQueue,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpBNum ++;
            }

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpPNum < cpPMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle->p_TsQueue
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpPNum ++;
            }

        }
        else{
            //i frame
            if(cpINum < cpIMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle->p_TsQueue
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpINum ++;
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

int x_split_gop_IBB_2X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BBP BBP BBP BBP BB*/
    /*4X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest ,
      else if goplen around 8 ,cp the first i, then drop rest gops,
      else goplen > 12 ,cp first i p,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        p_TsSeqHandle->drop_segment_num = 0;

        cpMax = goplen /2;

    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1] -
                    p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame ,pframe
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }
    return 0;
}
int x_split_gop_IBB_4X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN;index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BB BB BB BB BB*/
    /*4X if goplen around 4 ==> I BBB ,cp the first i,then drop rest ,
      else if goplen around 8 ,cp the first i, then drop rest gops,
      else goplen > 12 ,cp first i p,then drop rest  gop,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 2;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 3;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;

            }

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame ,pframe
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }
    return 0;
}
int x_split_gop_IBB_8X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I PP PP PP PP P*/
    /*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
            cpMax = 2;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}
int x_split_gop_IBB_16X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN;index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BP BP BP BP B*/
    /*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 3;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 0;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}
int x_split_gop_IBB_32X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN;index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BP BP BP BP B*/
    /*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 7;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 3;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 1;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen ; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}
int x_split_gop_IBB_64X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN ; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I BP BP BP BP B*/
    /*64X if goplen around 4 ==> I BBP  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        if(goplen < 6)
        {
            p_TsSeqHandle->drop_segment_num = 15;
        }
        else if(goplen > 5 && goplen < 12)
        {
            p_TsSeqHandle->drop_segment_num = 7;
        }
        else
        {
            p_TsSeqHandle->drop_segment_num = 3;
        }
    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else if(cur_video_frame_type == P_FRAME)
        {

            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }

        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}
int x_split_gop_IBB(ts_seq_t * p_TsSeqHandle ,ts_seq_play_mode_t play_mode)
{
    switch(play_mode)
    {
        case TS_SEQ_FAST_PLAY_2X:
        case TS_SEQ_REV_FAST_PLAY_2X:
            x_split_gop_IBB_2X(p_TsSeqHandle);
            break;

        case TS_SEQ_FAST_PLAY_4X:
        case TS_SEQ_REV_FAST_PLAY_4X:
            x_split_gop_IBB_4X(p_TsSeqHandle);
            break;

        case TS_SEQ_FAST_PLAY_8X:
        case TS_SEQ_REV_FAST_PLAY_8X:
            x_split_gop_IBB_8X(p_TsSeqHandle);
            break;

        case TS_SEQ_FAST_PLAY_16X:
        case TS_SEQ_REV_FAST_PLAY_16X:
            x_split_gop_IBB_16X(p_TsSeqHandle);
            break;

        case TS_SEQ_FAST_PLAY_32X:
        case TS_SEQ_REV_FAST_PLAY_32X:
            x_split_gop_IBB_32X(p_TsSeqHandle);
            break;


            // TODO:   FF4X  RF4X  FF8X  RF8X  FF16X  RF16X ...
        default:
            break;
    }
    return 0;

}

int x_split_gop_IIII_2X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN;index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I IIIIIIIIIIIIIIIIII*/
    /*64X if goplen around 4 ==> I III  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    //at this point goplen = 1;

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        p_TsSeqHandle->drop_segment_num = 1;

    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {
            //do record the p frame
        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}
int x_split_gop_IIII_4X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN; index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I IIIIIIIIIIIIIIIIII*/
    /*64X if goplen around 4 ==> I III  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    //at this point goplen = 1;

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        p_TsSeqHandle->drop_segment_num = 3;

    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {
            //do record the p frame
        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}
int x_split_gop_IIII_8X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN;index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I IIIIIIIIIIIIIIIIII*/
    /*64X if goplen around 4 ==> I III  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    //at this point goplen = 1;

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        p_TsSeqHandle->drop_segment_num = 7;

    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {
            //do record the p frame
        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}
int x_split_gop_IIII_16X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN;index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I IIIIIIIIIIIIIIIIII*/
    /*64X if goplen around 4 ==> I III  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    //at this point goplen = 1;

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        p_TsSeqHandle->drop_segment_num = 15;

    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {
            //do record the p frame
        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}
int x_split_gop_IIII_32X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN;index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I IIIIIIIIIIIIIIIIII*/
    /*64X if goplen around 4 ==> I III  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    //at this point goplen = 1;

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        p_TsSeqHandle->drop_segment_num = 31;

    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {
            //do record the p frame
        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}
int x_split_gop_IIII_64X(ts_seq_t * p_Handle)
{
    video_frame_t cur_video_frame_type;
    int index = 0 ,i = 0;
    int framelen = 0, goplen = 0;
    int cpNum = 0, cpMax = 0;
    ts_seq_t *p_TsSeqHandle = p_Handle;
    //find the first i frame,normal is index 0
    for(index = 0;index < MAX_GOP_LEN;index++)
    {
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
        if(cur_video_frame_type == I_FRAME)
        {
            break;
        }
    }
    if(index == MAX_GOP_LEN)
    {
        //err printf "no i frame"
        return -1;
    }

    cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];
    goplen = p_TsSeqHandle->cur_gop_type.len;

    //if drop num is 0, that means no need to drop the current GOP
    if(p_TsSeqHandle->drop_segment_num == 0)
    {
        p_TsSeqHandle->dropOneGop = FALSE;
    }

    /* I IIIIIIIIIIIIIIIIII*/
    /*64X if goplen around 4 ==> I III  ,cp the first i,then drop rest and the next 15 gops,
      else if goplen around 8 ,cp the first i, then drop rest and the next 7 gops,
      else goplen > 12 ,then drop rest and the next 3 gops,*/

    //at this point goplen = 1;

    if(p_TsSeqHandle->dropOneGop == FALSE)
    {
        //the first gop
        p_TsSeqHandle->dropOneGop = TRUE;
        cpMax = 1;
        cpNum = 0;

        p_TsSeqHandle->drop_segment_num = 63;

    }
    else
    {
        //the drop GOP
        p_TsSeqHandle->drop_segment_num --;

        cpMax = 0;
        cpNum = 0;
    }

    for(i = 0 ; i < goplen; i++)
    {
        //do data proccess
        //write ()
        //skip the b frame
        if(cur_video_frame_type == B_FRAME)
        {
            //do record the b frame

        }
        else if(cur_video_frame_type == P_FRAME)
        {
            //do record the p frame
        }
        else{
            //i frame
            if(cpNum < cpMax)
            {
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                write_fifo_kw(p_TsSeqHandle,
                        p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpNum ++;
            }
        }

        index ++;
        cur_video_frame_type = p_TsSeqHandle->cur_gop_type.frame_type_array[index];

        //end when the second i frame

    }

    return 0;
}

int x_split_gop_IIII(ts_seq_t * p_TsSeqHandle,ts_seq_play_mode_t play_mode)
{
    switch(play_mode)
    {
        case TS_SEQ_FAST_PLAY_2X:
        case TS_SEQ_REV_FAST_PLAY_2X:
            x_split_gop_IIII_2X(p_TsSeqHandle);
            break;

        case TS_SEQ_FAST_PLAY_4X:
        case TS_SEQ_REV_FAST_PLAY_4X:
            x_split_gop_IIII_4X(p_TsSeqHandle);
            break;

        case TS_SEQ_FAST_PLAY_8X:
        case TS_SEQ_REV_FAST_PLAY_8X:
            x_split_gop_IIII_8X(p_TsSeqHandle);
            break;

        case TS_SEQ_FAST_PLAY_16X:
        case TS_SEQ_REV_FAST_PLAY_16X:
            x_split_gop_IIII_16X(p_TsSeqHandle);
            break;

        case TS_SEQ_FAST_PLAY_32X:
        case TS_SEQ_REV_FAST_PLAY_32X:
            x_split_gop_IIII_32X(p_TsSeqHandle);
            break;


            // TODO:   FF4X  RF4X  FF8X  RF8X  FF16X  RF16X ...
        default:
            break;
    }
    return 0;
}
#else

#if 0
static int x_split_gop_SPECIAL(ts_seq_t * p_TsSeqHandle ,ts_seq_play_mode_t play_mode)
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
                //fwrite(p_TsSeqHandle->cur_gop_type.video_packet_start_array[index]
                //,1,framelen,pfileOut);
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
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
                p_TsSeqHandle->bpframe_cpnum[curgopnum] ++;;
#else
                framelen = p_TsSeqHandle->cur_gop_type.video_packet_start_array[index + 1]
                    - p_TsSeqHandle->cur_gop_type.video_packet_start_array[index];
                //fwrite(p_TsSeqHandle->cur_gop_type.video_packet_start_array[index]
                //,1,framelen,pfileOut);
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
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
                //if(framelen < 0)
                //   framelen = 0;
                //fwrite(p_TsSeqHandle->cur_gop_type.video_packet_start_array[index]
                //,1,framelen,pfileOut);
                write_fifo_kw(p_TsSeqHandle
                        , p_TsSeqHandle->cur_gop_type.video_packet_start_array[index],framelen);
                cpINum ++;
                //OS_PRINTF("IIIIIIIII I Frame Size %d \n",framelen);
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
#endif


#endif


static void   print_cur_gop(ts_seq_t * p_TsSeqHandle)
{
#if 0
    static int totalGOP = 0;
    int i = 0;
    //totalGOP++;
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
#if 0
static int resort_packetstart_array(char  * p_array[MAX_GOP_LEN] , int array_len)
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


static int resort_frametype_array(video_frame_t *p_array,int array_len)
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
#endif
#if 0
static  int  x_check_gop_type(ts_seq_t * p_TsSeqHandle)
{
    int cur_gop_len =  p_TsSeqHandle->cur_gop_type.len;

    if(p_TsSeqHandle->last_gop_len != cur_gop_len)
    {
        if(p_TsSeqHandle->cur_gop_type.frame_type_array[0] == I_FRAME)
        {
            if((cur_gop_len >= 4) && p_TsSeqHandle->cur_gop_type.frame_type_array[1] == B_FRAME
                    && p_TsSeqHandle->cur_gop_type.frame_type_array[2] == B_FRAME
                    && p_TsSeqHandle->cur_gop_type.frame_type_array[3] == P_FRAME)
            {
                p_TsSeqHandle->cur_gop_type.base_frame_type = I_B_B_P;
                MY_LOG("[x_check_gop_type]: I_B_B_P !!!\n");
            }
            else if((cur_gop_len >= 3) && p_TsSeqHandle->cur_gop_type.frame_type_array[1]
                    == B_FRAME
                    && p_TsSeqHandle->cur_gop_type.frame_type_array[2] == P_FRAME)
            {
                p_TsSeqHandle->cur_gop_type.base_frame_type = I_B_P;
                MY_LOG("[x_check_gop_type]: I_B_P !!!\n");
            }
            else if((cur_gop_len >= 2) && p_TsSeqHandle->cur_gop_type.frame_type_array[1]
                    == P_FRAME)
            {
                p_TsSeqHandle->cur_gop_type.base_frame_type = I_P_P;
                MY_LOG("[x_check_gop_type]: I_P_P !!!\n");
            }
            else if((cur_gop_len == 3) && p_TsSeqHandle->cur_gop_type.frame_type_array[1]
                    == B_FRAME
                    && p_TsSeqHandle->cur_gop_type.frame_type_array[2] == B_FRAME)
            {
                p_TsSeqHandle->cur_gop_type.base_frame_type = I_B_B;
                MY_LOG("[x_check_gop_type]: I_B_B !!!\n");
            }
            else if(cur_gop_len == 1)
            {
                p_TsSeqHandle->cur_gop_type.base_frame_type = I_I_I_I;
                MY_LOG("[x_check_gop_type]: I_I_I_I !!!\n");
            }
            // TODO: other gop type !!!
            else if(cur_gop_len >= 2)
            {
                p_TsSeqHandle->cur_gop_type.base_frame_type = I_SPECIAL;
                MY_LOG("[x_check_gop_type]: I_SPECIAL !!!\n");

            }
            else
            {
                MY_LOG("[x_check_gop_type][ERROR]: INVALID_GOP_TYPE !!!\n");
                p_TsSeqHandle->cur_gop_type.base_frame_type = INVALID_GOP;
                return -1;
            }

            p_TsSeqHandle->last_gop_len = p_TsSeqHandle->cur_gop_type.len;
            return 0;

        }

    }


    return 0;

}
#endif

static int rev_flash_ts_gop(ts_seq_t *p_TsSeqHandle)
{
  gop_t *p_gop = &(p_TsSeqHandle->cur_gop_type);
  int goplen = p_gop->len;
  video_frame_t lastframetype = 0;
  int i = 0;
  int size = 0;
  int pos = 0;
  //MT_ASSERT(goplen >= 0);
  if(goplen == 0)
  {
    return 0;
  }

  //only process i frame
  for(i = 0; i < goplen ; i++)
  {
    if(lastframetype == I_FRAME)
    {
       pos = p_gop->frame_startpos[i];
       size = p_gop->frame_deletesize[i];
       memset(p_gop->video_packet_start_array[i] + pos + size,
      0,
      188 - size - pos);
      p_gop->video_packet_start_array[i] += 188;      
    }

    lastframetype = p_gop->frame_type_array[i];
  }
  return 0;
}

#ifdef PKT_STATSTICS
static int total_rev = 0;
static int total_rev_send = 0;
static int total_ff = 0;
static int total_ff_send = 0;
#endif

static int last_rev_drop = 0;

static int  rev_update_ts_detail_info(ts_seq_t * p_TsSeqHandle,ts_detail_info_t *p_ts_detail)
{
  gop_t *p_gop = &(p_TsSeqHandle->cur_gop_type);

  int goplen = p_gop->len;
  int dropbit = 0;
  int i = 0;
  int i_frame_index = 0;
  int pos = 0;
  ulong addr_start = 0;
  ulong addr_end = 0;
//int leftsize = p_TsSeqHandle->tmp_buffer_len - p_TsSeqHandle->tmp_buffer_offset;
  int tmpsize = 0;

#ifdef DEBUG_MPEG_PARSER
OS_PRINTF("gop len %d addr %x,size %d \n",goplen,
    p_TsSeqHandle->p_data,p_TsSeqHandle->data_len);
  for(i = 0; i < goplen; i++)
    {
OS_PRINTF("@@@@FRME index %d ,type %d ,dropbit %d, addr %x\n",
                               i,
                               p_gop->frame_type_array[i],
                               p_gop->frame_dropbit[i],
                                p_gop->video_packet_start_array[i]);
 
    }
#endif   
  MT_ASSERT(goplen >= 0);

  rev_flash_ts_gop(p_TsSeqHandle);

  memset(p_ts_detail,0,sizeof(ts_detail_info_t));

  goplen = goplen - 1;
  if(goplen < 0)
  {
    goplen = 0;
    OS_PRINTF("####find a goplen = 0 when fastback!!\n");
  }
  //update the tmpbuf
  if(goplen == 0)
  {
     tmpsize = p_TsSeqHandle->data_len;
  }
  else
  {
    tmpsize = (u32)(p_gop->video_packet_start_array[0]  + 188 - p_TsSeqHandle->p_data);
  }

  if(p_TsSeqHandle->last_ts_dropbit == 1)
  {
    p_TsSeqHandle->last_ts_dropbit = 0;

    if(p_gop->frame_type_array[p_gop->len - 1] == I_FRAME)
    {
       OS_PRINTF("$$$$skip frame is i frame\n");      
    }

  }

#if 0  
  if(tmpsize)
  {
     if(tmpsize > leftsize)
     {
       p_TsSeqHandle->last_ts_dropbit = 1;
       p_TsSeqHandle->tmp_buffer_offset = 0;
       OS_PRINTF("$$$$skip frame %d total %d\n",
           tmpsize,p_TsSeqHandle->tmp_buffer_len);
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
  }
#endif
  //start with the last frame
  //find the first i frame
  for(i = goplen; i > 0; i --)
  {
    if(p_gop->frame_type_array[i - 1] == I_FRAME)
    {
        i_frame_index = i ;
        addr_start = (ulong)p_gop->video_packet_start_array[i - 1];
        if(i == goplen)
        {
          if(goplen == p_gop->len)
          {
            addr_end = (ulong)(p_TsSeqHandle->p_data + p_TsSeqHandle->data_len);
          }
          else
          {
            addr_end = (ulong)p_gop->video_packet_start_array[i];
          }
          
        }
        else
        {
           addr_end = (ulong)p_gop->video_packet_start_array[i];
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
    dropbit = p_gop->frame_dropbit[i - 1];

    if(dropbit == 0)
    {
      //write down the ptr
      if(p_gop->frame_type_array[i - 1] == I_FRAME && i < i_frame_index)
      {
        //update the copy i frame addr
        addr_start = (ulong)p_gop->video_packet_start_array[i - 1];
        addr_end = (ulong)p_gop->video_packet_start_array[i];
      }

      pos = p_ts_detail->fragment_num;
      p_ts_detail->fragment_offset[pos] = addr_start - (ulong)p_TsSeqHandle->p_data;
      p_ts_detail->fragment_size[pos] = addr_end - addr_start;

      p_ts_detail->fragment_num ++;
#ifdef DEBUG_MPEG_PARSER
    OS_PRINTF("ts_detail_info num %d ,offset %x,size %x \n",
        p_ts_detail->fragment_num,
        p_ts_detail->fragment_offset[p_ts_detail->fragment_num - 1],
        p_ts_detail->fragment_size[p_ts_detail->fragment_num - 1]);
#endif
    }

  }

  //OS_PRINTF("ts_detail_info total num %d \n",p_ts_detail->fragment_num);
#ifdef PKT_STATSTICS
  total_rev += goplen;
  total_rev_send += p_ts_detail->fragment_num;
  OS_PRINTF("total num %d send %d\n",total_rev,total_rev_send);
#endif
  return 0;
}

static int ff_flash_ts_gop(ts_seq_t *p_TsSeqHandle)
{
  gop_t *p_gop = &(p_TsSeqHandle->cur_gop_type);

  int goplen = p_gop->len;

  int dropbit = p_TsSeqHandle->last_ts_dropbit;
  int lastdropbit = dropbit;
  int i = 0;
  int size = 0;
  int pos = 0;
  //MT_ASSERT(goplen >= 0);
  if(goplen == 0)
  {
    return 0;
  }
  
  for(i = 0; i < goplen ; i++)
  {
    lastdropbit = dropbit;
    dropbit = p_gop->frame_dropbit[i];
    pos = p_gop->frame_startpos[i];
    size = p_gop->frame_deletesize[i];

    if(lastdropbit == 1 && dropbit == 0)
    {
      //first few Bytes need drop
      memset(p_gop->video_packet_start_array[i] + pos,0,size);
    }
    if(lastdropbit == 0 && dropbit == 1)
    {
      //first few Bytes MUST NOT drop
      memset(p_gop->video_packet_start_array[i] + pos + size,
      0,
      188 - size - pos);
      p_gop->video_packet_start_array[i] += 188;      
    }
  }
  return 0;
}

static int  ff_update_ts_detail_info(ts_seq_t * p_TsSeqHandle,ts_detail_info_t *p_ts_detail)
{
  gop_t *p_gop = &(p_TsSeqHandle->cur_gop_type);

  int goplen = p_gop->len;

  int dropbit = p_TsSeqHandle->last_ts_dropbit;

  ulong addr_start = (ulong)p_TsSeqHandle->p_data;
  ulong addr_end = 0;

  int i = 0;
  int pos = 0;
#ifdef DEBUG_MPEG_PARSER
OS_PRINTF("gop len %d addr %x,size %d \n",goplen,
    p_TsSeqHandle->p_data,p_TsSeqHandle->data_len);
  for(i = 0; i < goplen; i++)
  {
     OS_PRINTF("@@@@FRME index %d ,type %d ,dropbit %d, addr %x\n",
                              i,
                               p_gop->frame_type_array[i],
                              p_gop->frame_dropbit[i],
                                p_gop->video_packet_start_array[i]);
 
  }
#endif  
  MT_ASSERT(goplen >= 0);

  ff_flash_ts_gop(p_TsSeqHandle);

  if(goplen == 0)
  {
    addr_end = (ulong)(p_TsSeqHandle->p_data + p_TsSeqHandle->data_len);
  }
  else
  {
    addr_end = (ulong)(p_gop->video_packet_start_array[0]);
  }

  memset(p_ts_detail,0,sizeof(ts_detail_info_t));

  if(dropbit == 0)
  {
    //write down the ptr
    p_ts_detail->fragment_offset[0] = 0;
    p_ts_detail->fragment_size[0] = addr_end - addr_start;

    p_ts_detail->fragment_num ++;
#ifdef DEBUG_MPEG_PARSER
    OS_PRINTF("ts_detail_info num %d ,offset %x,size %x \n",
        p_ts_detail->fragment_num,
        p_ts_detail->fragment_offset[0],
        p_ts_detail->fragment_size[0]);
#endif
  }

  for(i = 0; i < goplen ; i++)
  {
    dropbit = p_gop->frame_dropbit[i];

    addr_start = (ulong)p_gop->video_packet_start_array[i];
    if(i == goplen - 1)//last frame
    {
      addr_end = (ulong)(p_TsSeqHandle->p_data + p_TsSeqHandle->data_len);

      if(p_TsSeqHandle->input_mode == TS_SEQ_INPUT_TS_HD)
      {
        dropbit = 1;
        if(p_gop->frame_type_array[i] == I_FRAME)
           p_ts_detail->reverse_offset = addr_start - (ulong)p_TsSeqHandle->p_data;
      }
      p_TsSeqHandle->last_ts_dropbit = dropbit;
    }
    else
    {
      addr_end = (ulong)(p_gop->video_packet_start_array[i + 1]);
    }

    if(dropbit == 0)
    {
      //write down the ptr
      pos = p_ts_detail->fragment_num;
      p_ts_detail->fragment_offset[pos] = addr_start - (ulong)p_TsSeqHandle->p_data;
      p_ts_detail->fragment_size[pos] = addr_end - addr_start;

      p_ts_detail->fragment_num ++;
#ifdef DEBUG_MPEG_PARSER
    OS_PRINTF("ts_detail_info num %d ,offset %x,size %x \n",
        p_ts_detail->fragment_num,
        p_ts_detail->fragment_offset[p_ts_detail->fragment_num - 1],
        p_ts_detail->fragment_size[p_ts_detail->fragment_num - 1]);
#endif
    }

  }

  //OS_PRINTF("ts_detail_info total num %d \n",p_ts_detail->fragment_num);
#ifdef PKT_STATSTICS
  total_ff += goplen;
  total_ff_send += p_ts_detail->fragment_num;
  OS_PRINTF("total num %d send %d\n",total_ff,total_ff_send);
#endif
  return 0;
}


int ts_backward_gop_parser(ts_seq_t * p_TsSeqHandle,
							ts_detail_info_t *p_ts_detail)
{


do
{
    do_parse_gop1(p_TsSeqHandle);

    if(p_TsSeqHandle->isExit)
    {
        break;
    }

    print_cur_gop(p_TsSeqHandle);

    //update ts detail
    rev_update_ts_detail_info(p_TsSeqHandle,p_ts_detail);

}
while(p_TsSeqHandle->left_data_len  && p_TsSeqHandle->isExit == FALSE);

return 0;

}


#if 0
int ts_backward_gop_parser(ts_seq_t * p_TsSeqHandle, ts_detail_info_t *p_ts_detail))
{
    int i = 0;
    MT_BOOL isFindSecondKeyFrame  =   FALSE;
    video_frame_t *p_frame_type_array = NULL;

    do
    {
        do_revert_parse_gop(p_TsSeqHandle);

        if(p_TsSeqHandle->isFindFirstIFrameBackward)
        {
            //clear all mark and then continue
            ts_seq_reset_play_direction(p_TsSeqHandle,FALSE,MPEG_TYPE_CODE);

            p_TsSeqHandle->p_tmp_buffer_wp = p_TsSeqHandle->p_tmp_buffer_start - 187;
            p_TsSeqHandle->cur_gop_type.len = 0;

            p_frame_type_array = NULL;
            p_frame_type_array = p_TsSeqHandle->cur_gop_type.frame_type_array;
            memset(p_frame_type_array,0,MAX_GOP_LEN * sizeof(video_frame_t));

            for(i = 0; i < MAX_GOP_LEN; i++)
            {
                p_TsSeqHandle->cur_gop_type.video_packet_start_array[i] = NULL;
            }

            p_TsSeqHandle->isFindFirstIFrameBackward = FALSE;
            p_TsSeqHandle->isFindFirstIFrame = TRUE;

            continue;
        }
        isFindSecondKeyFrame = p_TsSeqHandle->isFindSecondIFrame;
        if(isFindSecondKeyFrame)
        {
            resort_frametype_array(p_TsSeqHandle->cur_gop_type.frame_type_array,
                    p_TsSeqHandle->cur_gop_type.len);
            resort_packetstart_array(p_TsSeqHandle->cur_gop_type.video_packet_start_array
                    , p_TsSeqHandle->cur_gop_type.len);
            p_TsSeqHandle->isFindSecondIFrame = FALSE;

            print_cur_gop(p_TsSeqHandle);

            p_TsSeqHandle->gop_num ++;

            x_split_gop_SPECIAL(p_TsSeqHandle,p_TsSeqHandle->play_mode);

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
    totalGOP--;
    return 0;
}
#endif


int  ts_forward_gop_parser(ts_seq_t * p_TsSeqHandle,
							ts_detail_info_t *p_ts_detail)
{


do
{
    do_parse_gop1(p_TsSeqHandle);

    if(p_TsSeqHandle->isExit)
    {
        break;
    }

     print_cur_gop(p_TsSeqHandle);

    //update ts detail
    ff_update_ts_detail_info(p_TsSeqHandle,p_ts_detail);
#if 0
    isFindFirstKeyFrame = p_TsSeqHandle->isFindFirstIFrame;
    isFindSecondKeyFrame = p_TsSeqHandle->isFindSecondIFrame;


    //find two key frame
    if(isFindFirstKeyFrame && isFindSecondKeyFrame)
    {
        //do nothing before these is no one total gop in tmp buffer
        print_cur_gop(p_TsSeqHandle);

        p_TsSeqHandle->gop_num ++;

        x_split_gop_SPECIAL(p_TsSeqHandle,p_TsSeqHandle->play_mode);

        //reset field of p_TsSeqHandle
        p_TsSeqHandle->isFindFirstIFrame = FALSE;
        p_TsSeqHandle->isFindSecondIFrame = FALSE;
        //print_cur_gop();

        //the second key frame became the first key frame of current gop
        second_keyframe_index = p_TsSeqHandle->cur_gop_type.len;
        p_second_key_frame = p_TsSeqHandle
            ->cur_gop_type.video_packet_start_array[second_keyframe_index];
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
        MY_DEBUG("p_TsSeqHandle->p_tmp_buffer_start:%x\n",p_TsSeqHandle->p_tmp_buffer_start);
        MY_DEBUG("p_TsSeqHandle->p_tmp_buffer_wp:%x\n",p_TsSeqHandle->p_tmp_buffer_wp);
        MY_DEBUG("p_TsSeqHandle->p_tmp_buffer_end:%x\n",p_TsSeqHandle->p_tmp_buffer_end);
        MY_DEBUG("p_TsSeqHandle->tmp_buffer_free_space:%d\n",p_TsSeqHandle->tmp_buffer_free_space);
        MY_DEBUG("totalGOP:%d\n",totalGOP);


    }
#endif
}
while(p_TsSeqHandle->left_data_len  && p_TsSeqHandle->isExit == FALSE);

return 0;

}

