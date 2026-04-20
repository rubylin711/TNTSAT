
#ifndef __MT_UNF_WDG_H__
#define __MT_UNF_WDG_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define WDG_NO	 0
#define WDG_TIMEOUT_MS 1000

#define mt_unf_wdg_open     mt_unf_wdg_init
#define mt_unf_wdg_close    mt_unf_wdg_deinit

/******************************* API Declaration *****************************/

/**
@brief Open the WDG device.
@attention:
	By default,the WDG device is not running after it is opened.
	In this case,you need to call mt_unf_wdg_Enable to enable(run) it.
@retval 0 Success.
@retval :: MT_ERR_WDG_FAILED_INIT open failed
**/
mt_s32 mt_unf_wdg_init(mt_void);

/**
@brief Close the WDG device.
@retval 0 Success.
**/
mt_s32 mt_unf_wdg_deinit(mt_void);

/**
@brief Get the number of WDG device.
@attention:
	You can call this API to get thenumber of WDG chipset supports after the WDG device is started.
@param[out] pu32WdgNum The number of WDG chipset supports
@retval 0 Success.
@retval :: MT_FAILURE The Parameter pu32WdgNum is NULL.
**/
mt_s32 mt_unf_wdg_get_capability(mt_u32 *pu32WdgNum);

/**
@brief Start(Run) the WDG device.
@attention:
	You must call mt_unf_wdg_Enable after the WDG device is Opened.
@param[in] u32WdgNum WDG No. to operate.
@retval 0 Success.
@retval :: MT_ERR_WDG_NOT_INIT			The WDG device is not initialized.
@retval :: MT_ERR_WDG_INVALID_PARA		The Paramteter is invalid.
@retval :: MT_ERR_WDG_FAILED_ENABLE		Start watchdog failed.
**/
mt_s32 mt_unf_wdg_enable(mt_u32 u32WdgNum);

/**
@brief Stop the WDG device.
@attention:
	After calling this API,feed and reset the WDG is not work.
@param[in] u32WdgNum WDG No. to operate.
@retval 0 Success.
@retval :: MT_ERR_WDG_NOT_INIT			The WDG device is not initialized.
@retval :: MT_ERR_WDG_INVALID_PARA		The Paramteter is invalid.
@retval :: MT_ERR_WDG_FAILED_DISABLE	Stop watchdog failed.
**/
mt_s32 mt_unf_wdg_disable(mt_u32 u32WdgNum);

/**
@brief Obtains the interval of feeding the WDG.
@attention:
	The interval precision is as high as 1000 ms.
@param[in] u32WdgNum WDG No. to operate.
@param[in] pu32Value  Interval of feeding the WDG, in ms.
@retval 0 Success.
@retval :: MT_ERR_WDG_NOT_INIT			The WDG device is not initialized.
@retval :: MT_ERR_WDG_INVALID_PARA		The WDG input pointer is invalid.
@retval :: MT_ERR_WDG_FAILED_SETTIMEOUT	Get timeout failed.
**/
mt_s32 mt_unf_wdg_get_timeout(mt_u32 u32WdgNum, mt_u32 *pu32Value);

/**
@brief Sets the interval of feeding the WDG.
@param[in] u32WdgNum WDG No. to operate.
@param[out] u32Value  Interval of feeding the WDG, in ms.
@retval 0 Success.
@retval :: MT_ERR_WDG_NOT_INIT			The WDG device is not initialized.
@retval :: MT_ERR_WDG_FAILED_SETTIMEOUT	The WDG set timeout failed.
@retval :: MT_ERR_WDG_INVALID_PARA		The WDG input parameter is invalid.
**/
mt_s32 mt_unf_wdg_set_timeout(mt_u32 u32WdgNum, mt_u32 u32Value);

/**
@brief Feeds the WDG.
@param[in] u32WdgNum WDG No. to operate.
@retval 0 Success.
@retval :: MT_ERR_WDG_NOT_INIT			The WDG device is not initialized.
@retval :: MT_ERR_WDG_FAILED_CLEARWDG	The WDG clear watchdog failed.
@retval :: MT_ERR_WDG_INVALID_PARA		The Paramteter is invalid.
**/
mt_s32 mt_unf_wdg_clear(mt_u32 u32WdgNum);

/**
@brief Resets the entire system.
@param[in] u32WdgNum WDG No. to operate.
@retval 0 Success.
@retval :: MT_ERR_WDG_NOT_INIT			The WDG device is not initialized.
@retval :: MT_ERR_WDG_FAILED_RESET		The WDG reset failed.
@retval :: MT_ERR_WDG_INVALID_PARA		The Paramteter is invalid.
**/
mt_s32 mt_unf_wdg_reset(mt_u32 u32WdgNum);

/**
@brief enable WDG device irq.
@attention:
	This fun is NA(to do)
**/
mt_s32 mt_unf_wdg_enable_irq(mt_u32 u32WdgNum);

/**
@brief disable WDG device irq.
@attention:
	This fun is NA(to do)
**/
mt_s32 mt_unf_wdg_disable_irq(mt_u32 u32WdgNum);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __mt_unf_wdg_H__ */
