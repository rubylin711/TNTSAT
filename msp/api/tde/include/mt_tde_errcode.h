/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_TDE_ERRCODE_H__
#define __MT_TDE_ERRCODE_H__

//#include "mt_debug.h"
#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

//#define MT_ID_TDE 100
/* tde start err no. */ 
#define MT_ERR_TDE_BASE  ((mt_s32)( ((0x80UL + 0x20UL)<<24) | (100 << 16 ) | (4 << 13) | 1 ))

enum 
{
    MT_ERR_TDE_DEV_NOT_OPEN = MT_ERR_TDE_BASE,  /**<  tde device not open yet */ 
    MT_ERR_TDE_DEV_OPEN_FAILED,                 /**<  open tde device failed */
    MT_ERR_TDE_NULL_PTR,                        /**<  input parameters contain null ptr */
    MT_ERR_TDE_NO_MEM,                          /**<  malloc failed  */
    MT_ERR_TDE_INVALID_HANDLE,                  /**<  invalid job handle */
    MT_ERR_TDE_INVALID_PARA,                    /**<  invalid parameter */
    MT_ERR_TDE_NOT_ALIGNED,                     /**<  aligned error for position, stride, width */
    MT_ERR_TDE_MINIFICATION,                    /**<  invalid minification */
    MT_ERR_TDE_CLIP_AREA,                       /**<  clip area and operation area have no intersection */
    MT_ERR_TDE_JOB_TIMEOUT,                     /**<  blocked job wait timeout */
    MT_ERR_TDE_UNSUPPORTED_OPERATION,           /**<  unsupported operation */
    MT_ERR_TDE_QUERY_TIMEOUT,                    /**<  query time out */
    MT_ERR_TDE_INTERRUPT              /* blocked job was interrupted */
};
    

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_TDE_ERRCODE_H__*/


