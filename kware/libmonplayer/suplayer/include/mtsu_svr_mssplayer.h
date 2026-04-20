/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/**
 \file
 \brief Server (SVR) player mss module. CNcomment:svr mss player模块CNend
 \author Montage Technologies Co., Ltd.
 \date 2006-2018
 \version 1.0
 \author
 \date 2017-11-10
 */

#ifndef __MTSU_SVR_MSSPLAYER_H__
#define __MTSU_SVR_MSSPLAYER_H__

#include "mtsu_svr_player.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** Input media file */
/** CNcomment:输入的媒体文件 */
typedef struct mtSVR_PLAYER_MSS_MEDIA_S
{
  MT_CHAR aszUrl[MT_FORMAT_MAX_URL_LEN]; /**< File path, absolute file path, such as /mnt/filename.ts. */                                  /**< CNcomment:文件路径，绝对路径，如/mnt/filename.ts */
  MT_S32 s32PlayMode; /**< Set the mode of the player. The parameter is ::MT_SVR_PLAYER_PLAY_MODE_E */                                     /**< CNcomment:设置播放模式,参数::MT_SVR_PLAYER_PLAY_MODE_E */
  //MT_U32 u32ExtSubNum; /**< Number of subtitle files */                                                                                    /**< CNcomment:字幕文件个数 */
  //MT_CHAR aszExtSubUrl[MT_FORMAT_MAX_LANG_NUM][MT_FORMAT_MAX_URL_LEN]; /**< Absolute path of a subtitle file, such as /mnt/filename.ts. */ /**< CNcomment:字幕文件路径，绝对路径，如/mnt/filename.ts */
  MT_U32 u32UserData;                                                                                                                      /**< Create a handle by calling the fmt_open function, send the user data to the DEMUX by calling the fmt_invoke, and then call the fmt_find_stream function. */
                                                                                                                                           /**< CNcomment:用户数据，SuPlayer仅作透传，调用解析器fmt_open之后，通过fmt_invoke接口传递给解析器，再调用fmt_find_stream接口 */
  MT_CHAR aszLicenseUrl[MT_FORMAT_MAX_URL_LEN]; /**< File path, absolute file path, such as /mnt/filename.ts. */                           /**< CNcomment:文件路径，绝对路径，如/mnt/filename.ts */
  MT_CHAR aszCustomData[MT_FORMAT_MAX_URL_LEN]; /**< File path, absolute file path, such as /mnt/filename.ts. */                           /**< CNcomment:文件路径，绝对路径，如/mnt/filename.ts */
  MT_CHAR aszCertPath[MT_FORMAT_MAX_URL_LEN]; /**< File path, absolute file path, such as /mnt/filename.ts. */                           /**< CNcomment:???·????????·??????/mnt/filename.ts */
} MT_SVR_PLAYER_MSS_MEDIA_S;

/**
\brief Initialize the mss player. CNcomment:初始化Player CNend
\attention \n
The MT_S32 MT_SVR_PLAYER_MSS_Init interface can be called to return a success message after the initialization is successful. Multiple processes are not supported
. This interface must be called prior to other interfaces.
CNcomment:初始化成功后再调用该接口返回成功，不支持多进程，调用其它接口前必须先调用该接口CNend
\param None. CNcomment:无CNend

\retval ::MT_SUCCESS The initialization is successful. CNcomment:初始化成功CNend
\retval ::MT_FAILURE The initialization fails. CNcomment:初始化失败CNend

\see \n
None.
*/
MT_VOID* MT_SVR_PLAYER_MSS_Init(mt_void* args);

/**
\brief Deinitialize the mss player module by calling the MT_S32 MT_SVR_PLAYER_MSS_Deinit interface. The player module is not used any more. CNcomment:去初始化player模块，不再使用player模块，调用该接口去初始化player模块CNend
\attention \n
The ::MT_SVR_PLAYER_Destroy interface must be called to release the created player first. Otherwise, a failure is returned. The Deinit interface does not release the player resource. \n
This interface can be called to return a success message after the deinitialization is successful.
CNcomment:必须先调用::MT_SVR_PLAYER_Destroy接口释放掉创建的播放器，再调用该接口，否则会返回失败，Deinit不负责释放\n
播放器资源。去初始化成功后再调用该接口返回成功CNend
\param None. CNcomment:无CNend

\retval ::MT_SUCCESS The deinitialization is successful. CNcomment:去初始化成功CNend
\retval ::MT_FAILURE The deinitialization fails and the created player is not released. CNcomment:去初始化失败，没有释放掉创建的播放器CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_Deinit(MT_HANDLE hPlayer);

/**
\brief Create a mss player. CNcomment:创建一个播放器CNend
\attention \n
This interface must be called after initialization. Only one player can be created. \n
CNcomment:该接口必须在初始化后调用，支持最多创建一个播放器\n CNend
\param[in] pstruParam player initialization attribute. CNcomment:播放器初始化属性CNend
\param[out] phPlayer handle of the created player. CNcomment:创建的播放器句柄CNend

\retval ::MT_SUCCESS A player is created successfully and the player handle is valid. CNcomment:创建成功，播放器句柄有效CNend
\retval ::MT_FAILURE A player fails to be created. The parameters are invalid or resources are insufficient. CNcomment:创建失败，参数非法或资源不够CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_Create(const MT_SVR_PLAYER_PARAM_S *pstruParam, MT_HANDLE *phPlayer);
/**
\brief Destroy a mss player instance. CNcomment:销毁一个播放器实例CNend
\attention \n
The MT_S32 MT_SVR_PLAYER_MSS_Destroy interface is called to destroy the player resource after the ::MT_SVR_PLAYER_Create interface is called to create a player.
CNcomment:调用::MT_SVR_PLAYER_Create创建播放器后，调用该接口销毁播放器资源CNend
\param[in] hPlayer player handle. CNcomment:播放器句柄CNend

\retval ::MT_SUCCESS The player is released successfully. CNcomment:播放器释放成功CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_Destroy(MT_HANDLE hPlayer);

/**
\brief Start playing. CNcomment:开始播放CNend
\attention \n
If this interface is called after successful playing, MT_SUCCESS is returned. \n
Values returned by this interface cannot be used to check whether the playing is successful. The player notifies the APP of playing success or failure by using the ::MT_SVR_PLAYER_EVENT_STATE_CHANGED event. \n
The event parameter value is ::MT_SVR_PLAYER_STATE_PLAY.This interface can be called to restart playing after the playing stops.

CNcomment:播放成功后再调用该接口返回MT_SUCCESS，\n
不能通过该接口返回值来判断播放器是否播放成功，播放器会通过::MT_SVR_PLAYER_EVENT_STATE_CHANGED 事件通知\n
app播放成功，事件参数值为::MT_SVR_PLAYER_STATE_PLAY。停止播放后，可以调用该接口重新播放CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend

\retval ::MT_SUCCESS The operation is valid. CNcomment:合法操作CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_Play(MT_HANDLE hPlayer, MT_PCHAR url);

/**
\brief Stop mss playing. CNcomment:停止播放CNend
\attention \n
This interface is an asynchronous interface. It can be called to stop playing during playing, fast forward, rewind, and pause. Values returned by this interface cannot be used to check whether playing is stopped successfully. \n
The player notifies the APP of stop success or failure by using the ::MT_SVR_PLAYER_EVENT_STATE_CHANGED event. The event parameter value is ::MT_SVR_PLAYER_STATE_STOP. \n
The ::MT_SVR_PLAYER_MSS_Play interface can be called to restart playing after playing is stopped.

CNcomment:异步接口，播放、快进、快退、暂停过程中都可以调用该接口停止播放，不能通过该接口返回值来判断播放器\n
是否停止成功，播放器会通过::MT_SVR_PLAYER_EVENT_STATE_CHANGED 事件通知app停止成功，事件参数值为::MT_SVR_PLAYER_STATE_STOP。\n
停止播放后，可以调用::MT_SVR_PLAYER_Play接口重新播放CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend

\retval ::MT_SUCCESS The operation is valid. CNcomment:合法操作CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_Stop(MT_HANDLE hPlayer);

/**
\brief Pause mss playing.  CNcomment:暂停播放CNend
\attention \n
This interface is an asynchronous interface. It can be called to pause playing but cannot be called during fast forward and rewind. Values returned by this interface cannot be used to check whether playing is paused successfully. \n
The player notifies the APP of pause success or failure by using the ::MT_SVR_PLAYER_EVENT_STATE_CHANGED event. The event parameter value is ::MT_SVR_PLAYER_STATE_PAUSE. \n
The ::MT_SVR_PLAYER_MSS_Resume interface can be called to resume playing after a successful pause.
CNcomment:异步接口，播放过程中可以调用该接口暂停播放，快进、快退状态下不能调用该接口，不能通过该接口返回值来判断播放器\n
是否暂停成功，播放器会通过::MT_SVR_PLAYER_EVENT_STATE_CHANGED 事件通知app停止成功，事件参数值为::MT_SVR_PLAYER_STATE_PAUSE。\n
暂停成功后，可以调用::MT_SVR_PLAYER_Resume继续播放CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend

\retval ::MT_SUCCESS The operation is valid. CNcomment:合法操作CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_Pause(MT_HANDLE hPlayer);

/**
\brief Resume mss playing. CNcomment:恢复播放CNend
\attention \n
This interface is an asynchronous interface. It can be called to resume playing during pause, fast forward, and rewind. Values returned by this interface cannot be used to check whether playing is resumed successfully. \n
The player notifies the APP of resumption success or failure by using the ::MT_SVR_PLAYER_EVENT_STATE_CHANGED event. The event parameter value is ::MT_SVR_PLAYER_STATE_PLAY.

CNcomment:异步接口，暂停、快进、快退状态下，调用该接口恢复正常播放，不能通过该接口返回值来判断播放器是否恢复播放\n
播放器会通过::MT_SVR_PLAYER_EVENT_STATE_CHANGED 事件通知app恢复成功，事件参数值为::MT_SVR_PLAYER_STATE_PLAY。CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend

\retval ::MT_SUCCESS The operation is valid. CNcomment:合法操作CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_Resume(MT_HANDLE hPlayer);

/**
\brief Fast forward and rewind. CNcomment:快进、快退CNend
\attention \n
This interface is an asynchronous interface. It must be called in play, pause or tplay status. The negative value of s32Speed indicates rewind and the
positive value of s32Speed indicates fast forward. Values returned by the interface cannot be used to check whether the fast forward or rewind is successful.
\n
The player notifies the APP of the fast forward or rewind status by using the ::MT_SVR_PLAYER_EVENT_STATE_CHANGED event. \n
The event parameter value is ::MT_SVR_PLAYER_STATE_FORWARD or ::MT_SVR_PLAYER_STATE_BACKWARD. The ::MT_SVR_PLAYER_Resume interface can be called to \n
resume normal playing after fast forward or rewind.
CNcomment:异步接口，该函数必须播放、暂停或快进/快退状态下调用，s32Speed为负表示快退，为正表示快进，不能通过该接口\n
返回值来判断是否快进或快退成功，播放器会通过::MT_SVR_PLAYER_EVENT_STATE_CHANGED 事件通知app，事件参数值为\n
::MT_SVR_PLAYER_STATE_FORWARD或::MT_SVR_PLAYER_STATE_BACKWARD，快进、快退后通过调用::MT_SVR_PLAYER_Resume\n
接口恢复正常播放。CNend

\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] s32Speed playing speed. The value is MT_SVR_PLAYER_PLAY_SPEED_E. CNcomment:播放倍数,值为MT_SVR_PLAYER_PLAY_SPEED_E CNend

\retval ::MT_SUCCESS The operation is valid. CNcomment:合法操作CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_TPlay(MT_HANDLE hPlayer, MT_S32 s32Speed);


/**
\brief Seek to a specified location for playing. CNcomment:跳到指定位置播放CNend
\attention \n
It can be called to jump to a specified time point for playing.
\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] s64TimeInMs seeking time. The unit is ms. CNcomment:seek时间，单位ms CNend

\retval ::MT_SUCCESS The operation is successful. CNcomment:Seek成功CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_Seek(MT_HANDLE hPlayer, mt_s32 s32TimeStampSec);
/**
\brief Seek to a specified location to show frame. CNcomment:跳到指定位置播放CNend
\attention \n
It can be called to jump to a specified time point for playing.
\param[in] hPlayer player handle. CNcomment:播放器句柄CNend
\param[in] s64TimeInMs seeking time. The unit is ms. CNcomment:seek时间，单位ms CNend

\retval ::MT_SUCCESS The operation is successful. CNcomment:Seek成功CNend
\retval ::MT_FAILURE The operation is invalid. CNcomment:非法操作CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_Frame_Seek(MT_HANDLE hPlayer, mt_s32 s32TimeStampSec);

/**
\brief Set the URL of the media file to be played. The URL format is absolute path + media file name. CNcomment:设置要播放的媒体文件url地址，url格式为:绝对路径+媒体文件名CNend
\attention \n
The MT_SVR_PLAYER_SetMedia interface is a synchronous interface. It must be called after the ::MT_SVR_PLAYER_Create interface is called or after the ::
MT_SVR_PLAYER_Stop interface is called to stop the player.
This interface performs the following operations:
1. Queries the media file set by the DEMUX during parsing.
2. Queries subtitle files in the same directory as the audio and video media file automatically, and the queried subtitle files' name are the same as the name of the media file.
3. Performs the following steps if no AVPlay is specified:
   Creates an AVPlay and sets the audio and video attributes based on the parsed file attributes such as the AV encoding type.
   Creates a window, set the window size, and bind the window to the AVPlay.
   Creates a audio track(use device MT_UNF_SND_0),and bind it to the AVPlay.
The window display location and mixheight set for binding audio track to the AVPlay are specified when the ::MT_SVR_PLAYER_Create interface is called.
The window display location and mixheight are invalid if the AVPlay is specified externally.
CNcomment:同步接口，该接口必须在调用::MT_SVR_PLAYER_Create之后调用，或者调用::MT_SVR_PLAYER_Stop接口停止播放器后调用
该接主要执行以下处理:
1、查找解析器解析设置的媒体文件
2、在与媒体文件相同的路径下查找与媒体文件同名的字幕文件
3、如果没有指定avplay则
   创建avplay，根据解析出的文件属性设置音视频属性，如音视频编码类型
   创建window，设置window窗口位置，将window绑定到avplay
   创建audio track(使用设备MT_UNF_SND_0)，并将audio track绑定到avplay
window显示位置，绑定到avplay的audio track的mixheight(如果avplay是外部指定，则显示位置和mixheight参数无效)，
在调用::MT_SVR_PLAYER_Create接口时指定CNend

\param[in] hPlayer handle of the player is created by calling the ::MT_SVR_PLAYER_Create interface. CNcomment:通过调用::MT_SVR_PLAYER_Create接口创建的播放器句柄CNend
\param[in] eType media file. If only AV media files are specified, set this parameter to MT_SVR_PLAYER_MEDIA_STREAMFILE.\n
           If a subtitle file is specified, set this parameter to MT_SVR_PLAYER_MEDIA_STREAMFILE | MT_SVR_PLAYER_MEDIA_SUBTITLE.
           CNcomment:媒体文件。只指定音视频媒体文件，则该参数设置为MT_SVR_PLAYER_MEDIA_STREAMFILE，\n
            如果还指定了字幕文件则设置(MT_SVR_PLAYER_MEDIA_STREAMFILE | MT_SVR_PLAYER_MEDIA_SUBTITLE) CNend
\param[in] pstruMedia media file information. Only absolute path is supported.  \n
           The player searches for a subtitle file whose name is the same as the name of the audio and video media file in the same directory as the audio and video media file automatically.
            CNcomment:媒体文件信息，只支持绝对路径。播放器会自动在音视频媒\n
            体文件所在目录下查找与音视频媒体文件名相同的字幕文件CNend

\retval ::MT_SUCCESS The media file is set successfully. The ::MT_SVR_PLAYER_Play interface can be called to start playing the file. CNcomment:媒体文件设置成功，此时调用::MT_SVR_PLAYER_Play接口可以开始播放CNend
\retval ::MT_FAILURE The media file fails to be set. CNcomment:媒体文件设置失败CNend
\retval ::MT_ERRNO_NOT_SUPPORT_FORMAT The file format is not supported. CNcomment:不支持的文件格式CNend
\retval ::MT_ERRNO_NOT_SUPPORT_PROTOCOL The protocol is not supported. CNcomment:不支持的协议CNend

\see \n
None.
*/
MT_S32 MT_SVR_PLAYER_MSS_SetMedia(MT_HANDLE hPlayer, MT_U32 eType, MT_SVR_PLAYER_MSS_MEDIA_S *pstruMedia);
/** @} */ /** <!-- ==== API declaration end ==== */



MT_S32 MT_SVR_PLAYER_MSS_RegCallback(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_FN pfnCallback);


MT_S32 MT_SVR_PLAYER_MSS_Set_Aud_Track(MT_HANDLE hPlayer, int track_id);
MT_S32 MT_SVR_PLAYER_MSS_Set_Subtitle(MT_HANDLE hPlayer, int sub_id);
MT_S32 MT_SVR_PLAYER_MSS_Get_Media_Info(MT_HANDLE hPlayer, void * pResult);
MT_S32 MT_SVR_PLAYER_MSS_Get_Subt_Data(MT_HANDLE hPlayer, void * subt, int *pts, MT_U32 *size);
MT_S32 MT_SVR_PLAYER_MSS_SetAvplayHdl(MT_HANDLE hPlayer, MT_HANDLE hAvplay, MT_HANDLE hTrack);
MT_S32 MT_SVR_PLAYER_MSS_SetVoHdl(MT_HANDLE hPlayer,int vHandle);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSU_SVR_PLAYER_H__ */
