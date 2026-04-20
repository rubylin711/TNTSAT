/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_KT_IF_H__
#define __DRV_KT_IF_H__

#include <linux/slab.h>
#include "mt_drv_dev.h"
#include "mt_mpi_kt.h"
#include "drv_kt_ioctl.h"
#include "kt_result.h"

#define MT_FATAL_KT(fmt...)		MT_FATAL_PRINT(MT_ID_KT, fmt)
#define MT_ERR_KT(fmt...)		MT_ERR_PRINT(MT_ID_KT, fmt)
#define MT_WARN_KT(fmt...)		MT_WARN_PRINT(MT_ID_KT, fmt)
#define MT_INFO_KT(fmt...)		MT_INFO_PRINT(MT_ID_KT, fmt)
#define MT_DBG_KT(fmt...)		MT_DBG_PRINT(MT_ID_KT, fmt)

//==========================AUXILIARY================================
typedef struct semaphore	os_sem_t;
typedef struct _MT_KT_AUX_S
{
  os_sem_t sem_kt;
} MT_KT_AUX_S;

void drv_kt_aux_init(MT_KT_AUX_S *p_aux);
void drv_kt_aux_deinit(MT_KT_AUX_S *p_aux);

void *drv_kt_get_handle(void);

int kt_status_init(void);

void drv_kt_slot_request(void *priv, mt_u32 *p_slot_id);
void drv_kt_slot_request_multi(void *priv, mt_u32 num, mt_u32 *p_slot_id);
void drv_kt_slot_release(void *priv, mt_u32 slot_id);
void drv_kt_slot_active(void *priv, mt_u32 slot_id, MT_KT_SLOT_ACTIVE_E active);
void drv_kt_write_attribute(void *priv, mt_u32 slot_id, MT_KT_KEY_ATTR_S key_attr);
void drv_kt_read_attribute(void *priv, mt_u32 slot_id, MT_KT_KEY_ATTR_S *p_attr);
void drv_kt_write_key(void *priv, mt_u32 slot_id, mt_u8 *p_key, MT_KT_SLOT_SIZE_E size);
void drv_kt_read_key(void *priv, mt_u32 slot_id, mt_u8 *p_key, MT_KT_SLOT_SIZE_E size);
void drv_kt_write_iv(void *priv, mt_u32 slot_id,  mt_u8 *p_iv, MT_KT_SLOT_SIZE_E size);
void drv_kt_read_iv(void *priv, mt_u32 slot_id, mt_u8 *p_iv, MT_KT_SLOT_SIZE_E size);

#ifdef CONFIG_MT_CHIP_SYMPHONY6
void drv_kt_read_metadata(void *priv, mt_u32 slot_id, mt_u32 *metadata);
#endif

void drv_kt_get_state(void *priv, mt_u32 slot_id, MT_KT_SLOT_STATE_E *state);

void drv_kt_slot_info_get(void *priv, mt_u32 slot_id,
    mt_u32 *status, mt_u32 *valid, mt_u32 *attr, mt_u32 *teedata);
void drv_kt_slot_info(mt_u32 slot_id);

void drv_kt_control_get(mt_u32 *size, mt_u32 *control);

#endif	/*__DRV_KT_IF_H__*/

