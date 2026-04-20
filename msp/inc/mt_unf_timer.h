/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_UNF_TIMER_H__
#define __MT_UNF_TIMER_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef void (*timer_fn)(void *para); 

#define mt_unf_timer_open     mt_unf_timer_init
#define mt_unf_timer_close    mt_unf_timer_deinit

/******************************* API Declaration *****************************/
/** \addtogroup      TIMER*/
/** @{*/  /** <!-- [TIMER] */

/**
 \brief Starts the WDG device.
CNcomment:\brief 初始化TIMER 设备。CNend

 \param N/A          	CNcomment:无。CNend
 \retval 0 Success.   	CNcomment:成功。CNend
 \retval ::MT_ERR_TIMER_FAILED_INIT	open failed
 \see \n
N/A
 */
mt_s32 mt_unf_timer_init(mt_s32 *ps32TimerDevFd);

/**
 \brief Stops the WDG device.
CNcomment:\brief 去初始化TIMER设备。CNend

 \attention \n
N/A
 \param N/A          	CNcomment:无。CNend
 \retval 0 Success.   	CNcomment:成功。CNend
  \retval ::MT_ERR_TIMER_FAILED_DEINIT	open failed
 \see \n
N/A
 */
mt_s32 mt_unf_timer_deinit(mt_void);

/**
 \brief Release the timer.
CNcomment:\brief 释放相应的timer。CNend

 \attention \n
You can call this API to release the timer if you you no longer use the timer.
CNcomment:如果不再需要使用timer，调用该接口释放定时器。CNend

 \param[out] u32TimerID  The number of Timer need to release        	CNcomment:需要释放的定时器编号。CNend
 \retval 0 Success  CNcomment:成功 CNend
 \retval ::MT_ERR_TIMER_NOT_INIT The timer device is not initialized.       CNcomment:Timer 设备未初始化。CNend
 \retval ::MT_ERR_TIMER_INVALID_ID The Parameter u32TimerID is wrong.       CNcomment:定时器ID错误。CNend
 \retval ::MT_ERR_TIMER_FAILED_RELEASEL Release error.       CNcomment:定时器释放失败。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_timer_release(mt_u32 u32TimerID);

/**
 \brief Get the current timer count.
CNcomment:\brief 获取定时器当前计数值。CNend

 \attention \n
Release the timer and correspond to request.
CNcomment:释放的定时器和申请的相对应。CNend

 \param[in] u32TimerID timer id.        	CNcomment:需要读取的定时器编号。CNend
 \param[in] pu32Value  The current count value of the timer.             	CNcomment:定时器的当前计数值。CNend
 \retval 0 Success.                                                  	CNcomment:成功。CNend
 \retval ::MT_ERR_TIMER_NOT_INIT  The timer device is not initialized.     	CNcomment:Timer 设备未初始化。CNend
 \retval ::MT_ERR_TIMER_INVALID_ID The Parameter u32TimerID is wrong.       CNcomment:定时器ID错误。CNend
 \retval ::MT_ERR_TIMER_INVALID_POINT  The Timer input pointer is invalid.  	CNcomment:Timer输入指针无效。CNend
 \retval ::MT_ERR_TIMER_GETCNT_FAILED get timer count failed.			  	CNcomment:Timer获取计数失败。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_timer_read_cnt(mt_u32 u32TimerID, mt_u32 *pu32Value);


/**
 \brief Request a timer.
CNcomment:\brief 申请一个定时器。CNend

 \attention \n
Call the interface for request a timer, timer start to work after the application is successful.
CNcomment:调用该接口申请一个定时器，定时器在申请成功之后便开始工作。CNend

 \param[in] u32Timerms time  for request timer, in ms.        	CNcomment:定时器定时时间，单位毫秒。CNend
 \param[in] cycled Whether need to cycle count. 0: no;1:yes        	CNcomment:定时器是否需要循环工作0:仅计数一次；1:循环计数。CNend
 \param[in] pu32TimerID  The timer ID got from request .        	CNcomment:定时器申请成功后，返回的ID。CNend
 \retval 0 Success.  	CNcomment:成功。CNend
 \retval ::MT_ERR_TIMER_NOT_INIT  The timer device is not initialized.     	CNcomment:Timer 设备未初始化。CNend
 \retval ::MT_ERR_TIMER_INVALID_PARA The Parameter u32Timerms is wrong, should not be zero.       CNcomment:定时器定时参数错误。CNend
 \retval ::MT_ERR_TIMER_INVALID_POINT  The Timer input pointer is invalid.  	CNcomment:Timer输入指针无效。CNend
 \retval ::MT_ERR_TIMER_REQUEST_FAILED request timer failed.				CNcomment:申请定时器失败。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_timer_request(mt_u32 u32Timerms,  mt_u32 cycled, mt_u32 *pu32TimerID);

/**
 * @brief Create a cycled SW timer based on timerfd.
 *
 * @param[in] u32Timerms timer interval in ms.
 * @param[in] callback timer callback.
 * @param[in] priv timer callback private data.
 * @param[out] pu32TimerID timer ID.
 *
 * @retval
 *   MT_SUCCESS: success
 *   Others: failure
 */
mt_s32 mt_unf_timerfd_create(mt_u32 u32Timerms, timer_fn callback, void *priv, mt_u32 *pu32TimerID);

/**
 * @brief Start a timer.
 *
 * @param[in] u32TimerID timer ID.
 *
 * @retval
 *   MT_SUCCESS: success
 *   Others: failure
 */
mt_s32 mt_unf_timerfd_start(mt_u32 u32TimerID);

/**
 * @brief Stop a timer.
 *
 * @param[in] u32TimerID timer ID.
 *
 * @retval
 *   MT_SUCCESS: success
 *   Others: failure
 */
mt_s32 mt_unf_timerfd_stop(mt_u32 u32TimerID);

/**
 * @brief Delete a timer.
 *
 * @param[in] u32TimerID timer ID.
 *
 * @retval
 *   MT_SUCCESS: success
 *   Others: failure
 */
mt_s32 mt_unf_timerfd_delete(mt_u32 u32TimerID);

/** @} */  /** <!-- ==== API Declaration End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __mt_unf_wdg_H__ */
