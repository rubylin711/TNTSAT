/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 201, Montage Technology Co., Ltd.
 *
 * File Name      : mtlz_types.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : Monage-LZ SW Player type definition.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifdef __UC_OS__
#include "mt_type.h"
#include "sys_define.h"
#include "mtos_printk.h"

#define MTLZ_VERBOSE(...)
#define MTLZ_DEBUG						mtos_printk
#define MTLZ_WARN						mtos_printk
#define MTLZ_ERROR						mtos_printk
#else
#include <stdio.h>
#include <stdbool.h>

#define MTLZ_VERBOSE(...)
#define MTLZ_DEBUG						printf
#define MTLZ_WARN						printf
#define MTLZ_ERROR						printf
#endif

extern void MTLZ_ASSERT_DEBUG(bool expr, const char *function, int line);

#define MTLZ_BUG()						do{MTLZ_ERROR("[BUG]%s@%d!!!\n",__FUNCTION__,__LINE__);}while(0)
#define MTLZ_ASSERT(expr)				do{MTLZ_ASSERT_DEBUG(expr, __FUNCTION__,__LINE__);}while(0)
//#define MTLZ_FTRACE()					do{MTLZ_DEBUG("%s: %d @%lu\n",__FUNCTION__,__LINE__,mtlz_get_tick());}while(0)

//40ms
#define TIME_THRESHOLD					40
#define MTLZ_TRACE_TIME(str, t1, t2)	do { \
											if (t1 != 0 && t2 > t1 + TIME_THRESHOLD) { \
												MTLZ_WARN("[WARNING] %s %lu(%lu-%lu) > %d(ms)!\n", \
															str, t2-t1, t2, t1, TIME_THRESHOLD); \
											} \
										} while (0)

#define MTLZ_INVALID_HANDLE				(-1)
#define MTLZ_INVALID_ID					(-1)
#define MTLZ_INVALID_PID				0x1fff
#define MTLZ_INVALID_FD					(-1)
#define MTLZ_NULL_FD					0

/* return value */
#define MTLZ_SUCCESS					0
#define MTLZ_FAILURE					(!MTLZ_SUCCESS)
#define MTLZ_ENOENT						(-2)
#define MTLZ_EIO						(-5)
#define MTLZ_ENOMEM						(-12)
#define MTLZ_EBUSY						(-16)
#define MTLZ_EINVAL						(-22)

/** Codec return value */
/* need more buffer */
#define MTLZ_CODEC_RET_BUFFER			0
/* decodec one frame */
#define MTLZ_CODEC_RET_FRAME			1
/* unkown error, maybe need reset */
#define MTLZ_CODEC_RET_UNKNOWN_ERROR	2

#ifndef MIN
#define MIN(a, b)		(a)<=(b)?(a):(b)
#endif

#ifndef MAX
#define MAX(a, b)		(a)>=(b)?(a):(b)
#endif

#ifdef __UC_OS__
#ifndef mt_u32
typedef unsigned int mt_u32;
#endif

#ifndef mt_handle
typedef int mt_handle;
#endif
#endif

//-------------------------------- Configuration ----------------------------//

//SD
#define IN_BUFFER_SIZE					0x10000
//HD
//#define IN_BUFFER_SIZE				0x20000

//SD
#define MAX_PACKET_SIZE					0x40000
//HD
//#define MAX_PACKET_SIZE				0x200000

/**
 * Required number of additionally allocated bytes at the end of the input bitstream for decoding.
 * This is mainly needed because some optimized bitstream readers read
 * 32 or 64 bit at once and could read over the end.<br>
 * Note: If the first 23 bits of the additional bytes are not 0, then damaged
 * MPEG bitstreams could cause overread and segfault.
 */
#define PACKET_PADDING_SIZE				32

#define MTLZ_DMX_SOURCE_SUPPORT

#define MTLZ_DISPLAY_SINK_SUPPORT

//CIF: 512
//SD:  1024
//HD:  2048
#define MTLZ_DISPLAY_STRIDE				1024

//CIF: 288
//SD:  576
//HD:  720, 1080
#define MTLZ_DISPLAY_HEIGHT				576

/* effect read/decode/display performance */
#define MTLZPLAYER_SLEEP_INTERVAL		2

//#define FFMPEG_SUPPORT
//ffmpeg 2.x.x
//#define FFMPEG_V2

