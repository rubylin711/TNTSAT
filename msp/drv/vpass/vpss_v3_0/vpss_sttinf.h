/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_STTINF_H__
#define __VPSS_STTINF_H__
#include "vpss_common.h"
#include "mt_drv_vpss.h"
#include "vpss_osal.h"

/* DIE ST */
#define VPSS_DIE_MAX_NODE 3
typedef struct hiVPSS_DIE_DATA_S
{
    mt_u32 u32PhyAddr;
    LIST node;
} VPSS_DIE_DATA_S;

typedef struct hiVPSS_DIESTINFO_S
{
    MT_BOOL bInit;                     //初始化标识
    mt_u32 u32Cnt;
    mt_u32 u32Width;
    mt_u32 u32Height;
    mt_u32 u32Stride;
    mt_u32 u32DieInfoSize;
    VPSS_DIE_DATA_S stDataList[VPSS_DIE_MAX_NODE];
    mmz_buffer_s stMMZBuf;
    LIST* pstFirstRef;
} VPSS_DIESTINFO_S;

typedef struct hiVPSS_DIESTCFG_S
{
    mt_u32 u32CurAddr;
    mt_u32 u32PreAddr;
    mt_u32 u32PPreAddr;
    mt_u32 u32Stride;
} VPSS_DIESTCFG_S;

mt_s32 VPSS_STTINFO_DieInit(VPSS_DIESTINFO_S* pstDieStInfo, mt_u32 u32Width, mt_u32 u32Heigth);
mt_s32 VPSS_STTINFO_DieDeInit(VPSS_DIESTINFO_S* pstDieStInfo);
mt_s32 VPSS_STTINFO_DieGetInfo(VPSS_DIESTINFO_S* pstDieStInfo, VPSS_DIESTCFG_S* pstDieStCfg);
mt_s32 VPSS_STTINFO_DieComplete(VPSS_DIESTINFO_S* pstDieStInfo);
mt_s32 VPSS_STTINFO_DieReset(VPSS_DIESTINFO_S* pstDieStInfo);


/* CCCL CNT */
#define VPSS_CCCL_MAX_NODE 3
typedef struct hiVPSS_CCCL_DATA_S
{
    mt_u32 u32YPhyAddr;
    mt_u32 u32CPhyAddr;
    LIST node;
} VPSS_CCCL_DATA_S;

typedef struct hiVPSS_CCCLCNTINFO_S
{
    MT_BOOL bInit;                     //初始化标识
    mt_u32 u32Cnt;
    mt_u32 u32Width;
    mt_u32 u32Height;
    mt_u32 u32CCCLCntSize;
    mt_u32 u32ccnt_stride;
    mt_u32 u32ycnt_stride;
    VPSS_CCCL_DATA_S stDataList[VPSS_CCCL_MAX_NODE];
    mmz_buffer_s stMMZBuf;
    LIST* pstFirstRef;

} VPSS_CCCLCNTINFO_S;

typedef struct hiVPSS_CCCLCNTCFG_S
{
    mt_u32 u32Ycnt_raddr;
    mt_u32 u32Ccnt_raddr;
    mt_u32 u32Ycnt_waddr;
    mt_u32 u32Ccnt_waddr;

    mt_u32 u32Ccnt_stride;
    mt_u32 u32ycnt_stride;
} VPSS_CCCLCNTCFG_S;

mt_s32 VPSS_STTINFO_CcclInit(VPSS_CCCLCNTINFO_S* pstCcclCntInfo, mt_u32 u32Width, mt_u32 u32Heigth);
mt_s32 VPSS_STTINFO_CcclDeInit(VPSS_CCCLCNTINFO_S* pstCcclCntInfo);
mt_s32 VPSS_STTINFO_CcclGetInfo(VPSS_CCCLCNTINFO_S* pstCcclCntInfo,
                               VPSS_CCCLCNTCFG_S* pstCcclCntCfg);
mt_s32 VPSS_STTINFO_CcclComplete(VPSS_CCCLCNTINFO_S* pstCcclCntInfo);
mt_s32 VPSS_STTINFO_CcclReset(VPSS_CCCLCNTINFO_S* pstCcclCntInfo);


/* NR MAD */

#define VPSS_NR_MAX_NODE 4
typedef struct hiVPSS_NR_DATA_S
{
    mt_u32 u32PhyAddr;
    LIST node;
} VPSS_NR_DATA_S;

typedef enum hiVPSS_NR_MODE_E
{
    NR_MODE_FRAME  = 0,
    NR_MODE_5FIELD,
    NR_MODE_3FIELD,
    NR_MODE_BUTT
} VPSS_NR_MODE_E;


typedef struct hiVPSS_NR_ATTR_S
{
    mt_u32 u32Width;
    mt_u32 u32Height;
    VPSS_NR_MODE_E enMode;
} VPSS_NR_ATTR_S;

typedef struct hiVPSS_NRMADINFO_S
{
    MT_BOOL bInit;                     //初始化标识
    mt_u32 u32Cnt;
    VPSS_NR_ATTR_S stAttr;
    mt_u32 u32NRMADSize;
    mt_u32 u32madstride;
    VPSS_NR_DATA_S stDataList[VPSS_NR_MAX_NODE];
    mmz_buffer_s stMMZBuf;
    LIST* pstFirstRef;
} VPSS_NRMADINFO_S;

typedef struct hiVPSS_NRMADCFG_S
{
    mt_u32 u32Tnrmad_raddr;
    mt_u32 u32Tnrmad_waddr;
    mt_u32 u32Snrmad_raddr;
    mt_u32 u32madstride;
} VPSS_NRMADCFG_S;

mt_s32 VPSS_STTINFO_NrInit(VPSS_NRMADINFO_S* pstNrMadInfo, VPSS_NR_ATTR_S *pstAttr);
mt_s32 VPSS_STTINFO_NrDeInit(VPSS_NRMADINFO_S* pstNrMadInfo);
mt_s32 VPSS_STTINFO_NrGetInfo(VPSS_NRMADINFO_S* pstNrMadInfo,
                             VPSS_NRMADCFG_S* pstNrMadCfg);
mt_s32 VPSS_STTINFO_NrComplete(VPSS_NRMADINFO_S* pstNrMadInfo);
mt_s32 VPSS_STTINFO_NrReset(VPSS_NRMADINFO_S* pstNrMadInfo);

typedef struct hiVPSS_STTWBC_S
{
    MT_BOOL bInit;                     //初始化标识
    mt_u32 u32Cnt;
    mmz_buffer_s stMMZBuf;

} VPSS_STTWBC_S;

/* STT WBC */
#define VPSS_STTWBC_SIZE (4*1024)


mt_s32 VPSS_STTINFO_SttWbcInit(VPSS_STTWBC_S* psttWbc);
mt_s32 VPSS_STTINFO_SttWbcDeInit(VPSS_STTWBC_S* psttWbc);
mt_s32 VPSS_STTINFO_SttWbcGetInfo(VPSS_STTWBC_S* psttWbc,
                                    mt_u32* pu32stt_w_phy_addr,
                                    mt_u32* pu32stt_w_vir_addr);
mt_s32 VPSS_STTINFO_SttWbcComplete(VPSS_STTWBC_S* psttWbc);
mt_s32 VPSS_STTINFO_SttWbcReset(VPSS_STTWBC_S* psttWbc);

#endif
