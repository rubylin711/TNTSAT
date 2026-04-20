/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __REGISTER_NET_STREAM_H__
#define __REGISTER_NET_STREAM_H__
#ifdef __cplusplus
extern "C" {
#endif
/*!
*
*/

void register_http_stream(void);
/*!
*
*/
int register_http_stream_is(void);
/*!
*
*/
void register_rtsp_stream(void);
/*!
*
*/
void register_rtmp_stream(void);

/*!
*
*/
void register_asf_stream(void);

/*!
*
*/
void register_rtmp_protocol(int type);

/*!
*
*/
void register_fifo_stream(void);
void unregister_http_stream(void);
void unregister_rtsp_stream(void);
#ifdef __cplusplus
}
#endif
#endif





