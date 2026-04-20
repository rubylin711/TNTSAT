#ifndef __ANDRIOD__
#include "mt_type.h"
#include "sys_define.h"
#endif
#include "stream.h"
#include "mt_type.h"
#include "mtos_printk.h"
#include "register_net_stream.h"

#define  MAX_STREAM_NUM  (10)
extern const stream_info_t stream_info_ffmpeg;
extern const stream_info_t stream_info_http1;
extern const stream_info_t stream_info_rtsp_sip;
extern const stream_info_t stream_info_asf;
extern const stream_info_t stream_info_fifo;
#ifdef CFG_ENABLE_FFMPEG_422
extern const stream_info_t stream_info_misc;
#endif

extern stream_info_t  **   get_stream_by_index(int index);
int rtmp_protocol_type = 0;

void register_http_stream(void)
{

	int i = 0;
	OS_PRINTF("[%s] -----start!\n",__func__);

#ifdef ENABLE_DEMUX_HTTP

	stream_info_t **   pp_stream = NULL;
	for(i=0; i < MAX_STREAM_NUM; i++)
	{
		pp_stream  = get_stream_by_index(i);
		if(*pp_stream == NULL)
		{
			*pp_stream = &stream_info_ffmpeg;
			OS_PRINTF("[%s] ----[%d]-register ok!\n",__func__,i);
			break;
		}
	}
#else
     mtos_printk("[%s][ERROR][ERROR] --do nothing !!!!!!!!!!\n",__func__);
     mtos_printk("[%s] please config ENABLE_DEMUX_HTTP = Y!!!!!! !!\n",__func__);

#endif

	OS_PRINTF("[%s] -----end !\n",__func__);
}

void unregister_http_stream()
{

	int i = 0;
	OS_PRINTF("[%s] -----start!\n",__func__);

#ifdef ENABLE_DEMUX_HTTP

	stream_info_t **   pp_stream = NULL;
	for(i=0; i < MAX_STREAM_NUM; i++) {
		pp_stream  = get_stream_by_index(i);
		if(*pp_stream == &stream_info_ffmpeg) {
			*pp_stream = NULL;
			OS_PRINTF("[%s] ----[%d]-unregister ok!\n",__func__,i);
			break;
		}
	}
#else
	 mtos_printk("[%s][ERROR][ERROR] --do nothing !!!!!!!!!!\n",__func__);
	 mtos_printk("[%s] please config ENABLE_DEMUX_HTTP = Y!!!!!! !!\n",__func__);

#endif

	OS_PRINTF("[%s] -----end !\n",__func__);
}

int register_http_stream_is(void)
{
	int i = 0;
	OS_PRINTF("[%s] -----start!\n",__func__);

#if defined(ENABLE_DEMUX_HTTP)
	stream_info_t **   pp_stream = NULL;

	for(i=0; i < MAX_STREAM_NUM; i++)
	{
		pp_stream  = get_stream_by_index(i);
		if(*pp_stream == &stream_info_ffmpeg)
		{
			return 1;
		}
	}
  #else
     mtos_printk("[%s][ERROR][ERROR] --do nothing !!!!!!!!!!\n",__func__);
     mtos_printk("[%s] please config ENABLE_DEMUX_HTTP = Y!!!!!! !!\n",__func__);

  #endif


	OS_PRINTF("[%s] -----end !\n",__func__);
       return 0;
}

void register_rtsp_stream(void)
{
#ifdef CFG_ENABLE_FFMPEG_422
	int i = 0;
	stream_info_t **pp_stream = NULL;

	OS_PRINTF("[%s] start!\n",__func__);
    for(i=0; i < MAX_STREAM_NUM; i++) {
        pp_stream = get_stream_by_index(i);
        if(NULL != pp_stream && NULL == *pp_stream) {
            *pp_stream = &stream_info_misc;
            OS_PRINTF("[%s][%d] ok!\n",__func__, i);
            break;
        }
    }
#endif
}

void unregister_rtsp_stream(void)
{
#ifdef CFG_ENABLE_FFMPEG_422
	int i = 0;
	stream_info_t **pp_stream = NULL;

	OS_PRINTF("[%s] start!\n",__func__);
    for(i=0; i < MAX_STREAM_NUM; i++) {
        pp_stream = get_stream_by_index(i);
        if(*pp_stream == &stream_info_misc) {
            *pp_stream = NULL;
			OS_PRINTF("[%s] ----[%d]-unregister ok!\n",__func__,i);
            break;
        }
    }
#endif
}


void register_rtmp_stream(void)
{
	int i = 0;
	OS_PRINTF("[%s] -----start!\n",__func__);
#if defined(ENABLE_DEMUX_HTTP)
	stream_info_t **   pp_stream = NULL;
	for(i=0; i < MAX_STREAM_NUM; i++)
	{
		pp_stream  = get_stream_by_index(i);
		if(*pp_stream == NULL)
		{
			*pp_stream = &stream_info_ffmpeg;
			OS_PRINTF("[%s] ----[%d]-register ok!\n",__func__,i);
			break;
		}
	}
#else
      mtos_printk("[%s][ERROR][ERROR] --do nothing !!!!!!!!!!\n",__func__);
      mtos_printk("[%s] please config ENABLE_DEMUX_RTMP = Y!!!!!! !!\n",__func__);

#endif
	OS_PRINTF("[%s] -----end !\n",__func__);
}


void register_asf_stream(void)
{
//yliu rm:if open fail;socket not close correct
#if 0
	int i = 0;
	OS_PRINTF("[%s] -----start!\n",__func__);

    stream_info_t **   pp_stream = NULL;

	for(i=0; i < MAX_STREAM_NUM; i++)
	{
        pp_stream  = get_stream_by_index(i);
		if(*pp_stream == NULL)
		{
			*pp_stream = &stream_info_asf;
			OS_PRINTF("[%s] ----[%d]-register ok!\n",__func__,i);
			break;
		}
	}

	OS_PRINTF("[%s] -----end !\n",__func__);
#endif
}
//0:rtmp & librtmp ;1:librtmp ; 2 rtmp

void register_rtmp_protocol(int type)
{
//	int i = 0;
	OS_PRINTF("[%s] %d\n",__func__,__LINE__);

      rtmp_protocol_type = type;

	OS_PRINTF("[%s] %d\n",__func__,__LINE__);
}

void register_fifo_stream(void)
{
//	int i = 0;
	OS_PRINTF("[%s] -----start!\n",__func__);
#ifndef __ANDRIOD__
#if defined(ENABLE_DEMUX_HTTP)
	stream_info_t **   pp_stream = NULL;
	for(i=0; i < MAX_STREAM_NUM; i++)
	{
		pp_stream  = get_stream_by_index(i);
		if(*pp_stream == NULL)
		{
			*pp_stream = &stream_info_fifo;
			OS_PRINTF("[%s] ----[%d]-register ok!\n",__func__,i);
			break;
		}
	}
#else
      mtos_printk("[%s][ERROR][ERROR] --do nothing !!!!!!!!!!\n",__func__);
      mtos_printk("[%s] please config ENABLE_DEMUX_RTMP = Y!!!!!! !!\n",__func__);

#endif
#endif
	OS_PRINTF("[%s] -----end !\n",__func__);
}
