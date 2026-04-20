/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "vpss_mt.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#define HIS_MAD_MAX_WIDTH 1920
#define HIS_MAD_MAX_HEIGHT 1080


mt_s32 VPSS_MT_FLUSHDATA(mmz_buffer_s* pstMBuf, mt_u32 u32Data)
{
    mt_u32 u32Numb;
    mt_u32 u32Count;
    mt_u32* pu32Pos;
    u32Numb = (pstMBuf->u32Size + 3) / 4;

    pu32Pos = (mt_u32*)pstMBuf->u32StartVirAddr;

    for (u32Count = 0; u32Count < u32Numb; u32Count ++)
    {
        *pu32Pos = u32Data;
        pu32Pos = pu32Pos + 1;
    }

    return MT_SUCCESS;

}
mt_s32 VPSS_MT_Init(VPSS_MT_INFO_S *pstMtInfo)
{
//FIXME: Montage has no HW VPSS module

//FAKE
    mt_u32 ii;

	memset(&(pstMtInfo->stMadMtInfo.stMBuf), 0, sizeof(mmz_buffer_s));
    for (ii = 0; ii < 3; ii++)
    {
        pstMtInfo->stMadMtInfo.u32MadMvAddr[ii] = 0;
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_MT_DeInit(VPSS_MT_INFO_S *pstMtInfo)
{
    HIS_MAD_MEM_S *pstMadMem;

    pstMadMem = &(pstMtInfo->stMadMtInfo);

    //release MAD motion-infomation memory
    if (pstMadMem->stMBuf.u32StartVirAddr != 0)
    {
        mt_drv_mmz_unmap_and_release(&(pstMadMem->stMBuf));
        pstMadMem->stMBuf.u32StartVirAddr = 0;
        pstMadMem->stMBuf.u32Size = 0;
    }
    else
    {
//FIXME: Montage has no HW VPSS module
//no "VPSS_MADMotionInfoBuf"
//do nothing
        //VPSS_FATAL("Release MadBuf Error\n");
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_MT_GetAddr(VPSS_MT_INFO_S *pstMtInfo,VPSS_MT_ADDR_S *pstAddr)
{
    HIS_MAD_MEM_S *pstMadMem;

    mt_u32 *pu32Addr;
    mt_u32 i;

    mt_u32 u32Addrtmp;

    pstMadMem = &(pstMtInfo->stMadMtInfo);

    pstAddr->u32RPhyAddr = pstMadMem->u32MadMvAddr[2];
    pstAddr->u32WPhyAddr = pstMadMem->u32MadMvAddr[0];
    pstAddr->u32Stride = ((HIS_MAD_MAX_WIDTH + 31) & 0xffffffe0L) /2;

    //motion infor address
    pu32Addr = pstMadMem->u32MadMvAddr;

    u32Addrtmp = pu32Addr[2];
    for(i=2;i>0;i--)
    {
        pu32Addr[i] = pu32Addr[i-1];
    }
    pu32Addr[0] = u32Addrtmp;

    return MT_SUCCESS;
}
#ifdef __cplusplus
 #if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

