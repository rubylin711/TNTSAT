
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_osal.c
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/

#include "drv_disp_osal.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


mt_void DISP_OS_GetTime(mt_u32 *t_ms)
{
#ifdef __DISP_PLATFORM_SDK__

    MT_DRV_SYS_GetTimeStampMs(t_ms);
	return;
#else

	return;
#endif


}

mt_s32  DISP_OS_MMZ_Alloc(const char *bufname, char *zone_name, mt_u32 size, int align, DISP_MMZ_BUF_S *pstMBuf)
{
//#ifdef __DISP_PLATFORM_SDK__
#if 1
    mmz_buffer_s stMMZ;
    mt_s32 nRet;

    nRet = mt_drv_mmz_alloc(bufname, zone_name, size, align, &stMMZ);
    if (!nRet)
    {
        pstMBuf->u32StartPhyAddr = stMMZ.startPhyAddr;
        pstMBuf->u32StartVirAddr = (ulong)stMMZ.startVirAddr;
        pstMBuf->u32Size = stMMZ.size;
    }

    //printk("mmz addr=0x%x, size=%d, nRet = 0x%x\n", stMMZ.u32StartPhyAddr, stMMZ.u32Size, nRet);

    return nRet;
#else

    return MT_FAILURE;
#endif
}

mt_s32  DISP_OS_MMZ_Map( DISP_MMZ_BUF_S *pstMBuf)
{
//#ifdef __DISP_PLATFORM_SDK__
#if 1
    mmz_buffer_s stMMZ;
    mt_s32 nRet;

    memset((void*)&stMMZ, 0, sizeof(mmz_buffer_s));

    stMMZ.startPhyAddr = pstMBuf->u32StartPhyAddr ;

    nRet =mt_drv_mmz_map(&stMMZ);
    if (!nRet)
    {
        pstMBuf->u32StartPhyAddr = stMMZ.startPhyAddr;
        pstMBuf->u32StartVirAddr = (ulong)stMMZ.startVirAddr;
        pstMBuf->u32Size = stMMZ.size;
    }

    //printk("mmz addr=0x%x, size=%d, nRet = 0x%x\n", stMMZ.u32StartPhyAddr, stMMZ.u32Size, nRet);

    return nRet;
#else

    return MT_FAILURE;
#endif
}

mt_s32  DISP_OS_MMZ_UnMap( DISP_MMZ_BUF_S *pstMBuf)
{
//#ifdef __DISP_PLATFORM_SDK__
#if 1
    mmz_buffer_s stMMZ;
    mt_s32 nRet = MT_SUCCESS;

    stMMZ.startVirAddr = (void *)pstMBuf->u32StartVirAddr ;

    mt_drv_mmz_unmap(&stMMZ);


    //printk("mmz addr=0x%x, size=%d, nRet = 0x%x\n", stMMZ.u32StartPhyAddr, stMMZ.u32Size, nRet);

    return nRet;
#else

    return MT_FAILURE;
#endif
}
mt_void DISP_OS_MMZ_Release(DISP_MMZ_BUF_S *pstMBuf)
{
//#ifdef __DISP_PLATFORM_SDK__
#if 1
    mmz_buffer_s stMMZ;

    stMMZ.startPhyAddr = pstMBuf->u32StartPhyAddr;
    stMMZ.startVirAddr =	(void *)pstMBuf->u32StartVirAddr;
    stMMZ.size = pstMBuf->u32Size;

    mt_drv_mmz_release(&stMMZ);

    return;
#else

    return;
#endif
}


mt_s32 DISP_OS_MMZ_AllocAndMap(const char *bufname, char *zone_name, mt_u32 size, int align, DISP_MMZ_BUF_S *pstMBuf)
{
//#ifdef __DISP_PLATFORM_SDK__
#if 1
    mmz_buffer_s stMMZ;
    mt_s32 nRet;

    nRet = mt_drv_mmz_alloc_and_map(bufname, zone_name, size, align, &stMMZ);
    if (!nRet)
    {
        pstMBuf->u32StartPhyAddr = stMMZ.startPhyAddr;
        pstMBuf->u32StartVirAddr = (ulong)stMMZ.startVirAddr;
        pstMBuf->u32Size = stMMZ.size;
    }

    //printk("mmz addr=0x%x, size=%d, nRet = 0x%x\n", stMMZ.u32StartPhyAddr, stMMZ.u32Size, nRet);

    return nRet;
#else

    return MT_FAILURE;
#endif
}

EXPORT_SYMBOL(DISP_OS_MMZ_AllocAndMap);

mt_void DISP_OS_MMZ_UnmapAndRelease(DISP_MMZ_BUF_S *pstMBuf)
{
//#ifdef __DISP_PLATFORM_SDK__
#if 1
    mmz_buffer_s stMMZ;

    stMMZ.startPhyAddr = pstMBuf->u32StartPhyAddr;
    stMMZ.startVirAddr =	(void *)pstMBuf->u32StartVirAddr;
    stMMZ.size = pstMBuf->u32Size;

	if ((stMMZ.startPhyAddr != 0 || stMMZ.startVirAddr != 0)
		&& stMMZ.size != 0)
	{
		mt_drv_mmz_unmap_and_release(&stMMZ);
	}

    return;
#else

    return;
#endif
}

EXPORT_SYMBOL(DISP_OS_MMZ_UnmapAndRelease);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */




