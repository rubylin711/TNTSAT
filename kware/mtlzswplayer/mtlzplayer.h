/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 201, Montage Technology Co., Ltd.
 *
 * File Name      : mtlzplayer.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : Monage-LZ SW Player Definition.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __MTLZ_PLAYER_H__
#define __MTLZ_PLAYER_H__

#ifdef __cplusplus
#if __cplusplus
	 extern "C"{
#endif
#endif /* __cplusplus */

/* Montage-LZ SW Player Status */
enum mtlzplayer_status_e
{
	MTLZ_PLAYER_STATUS_IDLE,
	MTLZ_PLAYER_STATUS_STOP,
	MTLZ_PLAYER_STATUS_PLAY,
	MTLZ_PLAYER_STATUS_EOS,
};

/* Montage-LZ SW Player status information */
struct mtlzplayer_status_info_st
{
	enum mtlzplayer_status_e state;
};

/**
 * @brief open MT-LZ SW Player
 *
 * @param[in] url url to play
 *                Source: support Demux and local file;
 *                VCodec: support MPEG2, MPEG4, H264 and H265
 *                        (MPEG4/H264/H265 need ffmpeg's libavcodec);
 *                Sink: support Fake(just print log) and Display.
 *            e.g.
 *              "dvb://[*][source=tuner,0&]demuxer=dmx,0&video=201,mpeg2&sink=display,0"
 *              "/media/sda1/mpeg2_422.m2v"
 *
 *              Valid VCodec and Local File Extension Name:
 *                  MPEG2: "mpeg2","mpeg1","mpeg","mpg","m2v";
 *                  MPEG4: "mpeg4","mp4","m4v";
 *                  H264 : "h264","avc";
 *                  H265 : "h265","hevc".
 *
 * @retval
 *    >=0: handle of the player,
 *    <0 : failed
 */
int mtlzplayer_open(const char *url);

/**
 * @brief Close Montage-LZ SW Player
 *
 * @param[in] handle handle of player
 *
 * @retval
 *    MTLZ_SUCCESS: success
 *    others: failed
 */
int mtlzplayer_close(int handle);

/**
 * @brief Start Montage-LZ SW Player
 *
 * @param[in] handle handle of player
 *
 * @retval
 *    MTLZ_SUCCESS: success
 *    others: failed
 */
int mtlzplayer_start(int handle);

/**
 * @brief Stop Montage-LZ SW Player
 *
 * @param[in] handle handle of player
 *
 * @retval
 *    MTLZ_SUCCESS: success
 *    others: failed
 */
int mtlzplayer_stop(int handle);

/**
 * @brief Get Montage-LZ SW Player's status
 *
 * @param[in] handle handle of player
 * @param[out] status status of player
 *
 * @retval
 *    MTLZ_SUCCESS: success
 *    others: failed
 */
int mtlzplayer_get_status(int handle, struct mtlzplayer_status_info_st *status);

#ifdef __UC_OS__
/**
 * @brief Set Montage-LZ SW Player's task priority
 *    call after mtlzplayer_open, and before mtlzplayer_start.
 *
 * @param[in] priority player decoding task priority
 *
 * @retval
 *    MTLZ_SUCCESS: success
 *    others: failed
 */
int mtlzplayer_set_priority(int handle, int priority);
int mtlzplayer_get_priority(int handle);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTLZ_PLAYER_H__ */

