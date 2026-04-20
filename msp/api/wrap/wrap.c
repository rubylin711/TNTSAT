/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "mt_common.h"
#include "mt_drv_stat.h"
#include "mt_drv_vdec.h"
#include "vpuapi.h"
#include "mt_unf_avplay.h"

mt_s32 MT_SYS_GetTimeStampMs(mt_u32 *pu32TimeMs);
mt_s32 MT_MPI_STAT_NotifyLowDelayEvent(mt_ld_event_s *pLdEvent);
mt_s32 MT_MMZ_Malloc(mt_mmz_buf_s *pstBuf);
mt_s32 MT_MMZ_Free(mt_mmz_buf_s *pstBuf);
mt_void *MT_MEM_Map(mt_u32 u32PhyAddr, mt_u32 u32Size);
mt_s32 MT_MEM_Unmap(mt_void *pAddrMapped);

//called in mt_mpi_vdec_mjpeg.c
mt_s32 MT_SYS_GetTimeStampMs(mt_u32 *pu32TimeMs)
{
	return mt_sys_get_time_stamp_ms(pu32TimeMs);
}

mt_s32 MT_MPI_STAT_NotifyLowDelayEvent(mt_ld_event_s *pLdEvent)
{
	return MT_SUCCESS;
}

//called in mt_mpi_vdec_vpu.c
#if	(1 == MT_VDEC_VPU_SUPPORT)
int VPU_GetProductId(int coreIdx)
{
	Uint32 productId = 0;
	VPU_GetVersionInfo((Uint32)coreIdx, NULL, NULL, &productId);
	return (mt_s32)productId;
}
#endif

RetCode VPU_GetFBCOffsetTableSize(
    CodStd codStd,
    int width,
    int height,
    int *ysize,
    int *csize)
{
	*ysize = 0;
	*csize = 0;
	return MT_SUCCESS;
}

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
mt_s32 VDEC_VPU_PtsStop(mt_handle hInst);
mt_s32 VPU_SetFrmRate(mt_handle hVdec, const MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);
mt_s32 VDEC_VPU_GetFrameRateForNewFrm(mt_handle hHandle, mt_u32 *u32FrameRate);
mt_s32 VDEC_VPU_PtsAlloc(mt_handle hInst, mt_handle vpuHandle);
mt_s32 VDEC_VPU_Stop(mt_handle hInst);
mt_s32 VDEC_VPU_ProcStatus(mt_handle hHandle, MT_DRV_VDEC_VPU_STATUS_S *stVPUStatus);
mt_s32 VDEC_VPU_CheckRlsFrameID(mt_handle hVdec,mt_s32 *pID, mt_s32 *ps32Count);
mt_s32 VDEC_VPU_RevertFrameBuf(mt_u32 u32Phyaddr);
mt_s32 VDEC_VPU_SetAttr(mt_handle hVdec,VDEC_VPU_ATTR_S *pstVPUAttr);
mt_s32 VDEC_VPU_PtsReset(mt_handle hInst);
mt_s32 VDEC_VPU_CreateFrameBuf(mt_mmz_buf_s *pStreamBuf);
mt_s32 VDEC_VPU_ReleaseFrameList(mt_handle hVdec);
mt_s32 VDEC_VPU_CreateFrameList(mt_handle hVdec);
mt_s32 VDEC_VPU_Start(mt_handle hInst);
mt_s32 VDEC_VPU_PutFrame(mt_handle hVdec,MT_DRV_VDEC_USR_FRAME_S *pstBuf);
mt_s32 VPU_GetVpssStatusInfo(mt_handle hVdec, MT_BOOL *bAllPortCompleteFrm);
mt_s32 VDEC_VPU_PtsStart(mt_handle hInst);
mt_s32 VDEC_VPU_PtsFree(mt_handle hInst, mt_handle vpuHandle);
mt_s32 VPU_GetFrmRate(mt_handle hVdec, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);

mt_s32 VDEC_VPU_PtsStop(mt_handle hInst)
{
	return MT_SUCCESS;
}

mt_s32 VPU_SetFrmRate(mt_handle hVdec, const MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_GetFrameRateForNewFrm(mt_handle hHandle, mt_u32 *u32FrameRate)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_PtsAlloc(mt_handle hInst, mt_handle vpuHandle)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_Stop(mt_handle hInst)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_ProcStatus(mt_handle hHandle, MT_DRV_VDEC_VPU_STATUS_S *stVPUStatus)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_CheckRlsFrameID(mt_handle hVdec,mt_s32 *pID, mt_s32 *ps32Count)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_RevertFrameBuf(mt_u32 u32Phyaddr)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_SetAttr(mt_handle hVdec,VDEC_VPU_ATTR_S *pstVPUAttr)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_PtsReset(mt_handle hInst)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_CreateFrameBuf(mt_mmz_buf_s *pStreamBuf)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_ReleaseFrameList(mt_handle hVdec)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_CreateFrameList(mt_handle hVdec)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_Start(mt_handle hInst)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_PutFrame(mt_handle hVdec,MT_DRV_VDEC_USR_FRAME_S *pstBuf)
{
	return MT_SUCCESS;
}

mt_s32 VPU_GetVpssStatusInfo(mt_handle hVdec, MT_BOOL *bAllPortCompleteFrm)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_PtsStart(mt_handle hInst)
{
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_PtsFree(mt_handle hInst, mt_handle vpuHandle)
{
	return MT_SUCCESS;
}

mt_s32 VPU_GetFrmRate(mt_handle hVdec, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
	return MT_SUCCESS;
}
#endif

mt_s32 MT_MMZ_Malloc(mt_mmz_buf_s *pstBuf)
{
	return mt_mmz_malloc(pstBuf);
}

mt_s32 MT_MMZ_Free(mt_mmz_buf_s *pstBuf)
{
	return mt_mmz_free(pstBuf);
}

//called in mt_mpi_vdec.c
mt_void *MT_MEM_Map(mt_u32 u32PhyAddr, mt_u32 u32Size)
{
	return mt_mem_map(u32PhyAddr, u32Size);
}

mt_s32 MT_MEM_Unmap(mt_void *pAddrMapped)
{
	return mt_mem_unmap(pAddrMapped);
}

#ifdef ANDROID
//called in mt_unf_sci.c
int pthread_setcancelstate(int state, int *oldstate)
{
	return 0;
}
#endif
