/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_WBC_H__
#define __VPSS_WBC_H__

#include "mt_drv_mmz.h"
#include "vpss_common.h"
#include "mt_drv_vpss.h"
#include <linux/list.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define VPSS_WBC_MAX_NODE 5

typedef struct hiVPSS_WBC_DATA_S
{
    MT_DRV_VIDEO_FRAME_S stWbcFrame;  //??
    LIST node;                        //?
} VPSS_WBC_DATA_S;

typedef enum hiVPSS_WBC_MODE_E
{
    VPSS_WBC_MODE_NORMAL = 2,        //д???дп2??
    VPSS_WBC_MODE_3FIELD = 3,        //д??дп3??
    VPSS_WBC_MODE_5FIELD = 5,        //д峡??дп5??
    VPSS_WBC_MODE_BUTT
} VPSS_WBC_MODE_E;

typedef enum hiVPSS_WBC_REF_MODE_E
{
    VPSS_WBC_REF_MODE_INIT = 1,        //?ο??д??δд?ο?????д?ο??
    VPSS_WBC_REF_MODE_NULL = 2,        //?ο??д??δд?ο???Чд?ο??
    VPSS_WBC_REF_MODE_BUTT
} VPSS_WBC_REF_MODE_E;

typedef struct hiVPSS_WBC_ATTR_S
{
    VPSS_WBC_MODE_E enMode;               //дй??
    VPSS_WBC_REF_MODE_E enRefMode;        //??????
    mt_u32    u32Width;                   //??
    mt_u32    u32Height;                  //??
    MT_DRV_PIX_FORMAT_E ePixFormat;        //????
    MT_DRV_PIXEL_BITWIDTH_E  enBitWidth;  //???bitλ,10 bit or 8 bit
    mt_u32                   u32FrameIndex; //??INDEX?
    mt_u32                   u32Pts;        //??PTS?
} VPSS_WBC_ATTR_S;

typedef struct hiVPSS_WBC_S
{
    MT_BOOL bInit;                     //??
    mt_u32 u32CompleteCount;           //д?
    VPSS_WBC_DATA_S stDataList[VPSS_WBC_MAX_NODE];     //д?
    VPSS_WBC_ATTR_S stWbcAttr;         //д
    mmz_buffer_s stMMZBuf;             //??
    LIST* pstFirstRef;                  //??ο?
} VPSS_WBC_S;


mt_s32 VPSS_WBC_Init(VPSS_WBC_S* pstWbc, VPSS_WBC_ATTR_S* pstAttr);

mt_s32 VPSS_WBC_DeInit(VPSS_WBC_S* pstWbc);

mt_s32 VPSS_WBC_GetRefInfo(VPSS_WBC_S* pstWbc,
                           MT_DRV_VIDEO_FRAME_S** pstDataList
                          );

mt_s32 VPSS_WBC_GetWbcInfo(VPSS_WBC_S* pstWbc, MT_DRV_VIDEO_FRAME_S** pstData);

mt_s32 VPSS_WBC_Complete(VPSS_WBC_S* pstWbc);

mt_s32 VPSS_WBC_Reset(VPSS_WBC_S* pstWbc);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__VPSS_WBC_H__*/
