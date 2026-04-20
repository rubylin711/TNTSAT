/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/******************************************************************************
  File Name             :   mt_mpi_venc.h
  Version               :   Initial Draft
  Author                :   Montage multimedia software group
  Created               :   2015/12/3
  Last Modified         :
  Description           :
  Function List         :
  History               :
  1.Date                :   2015/12/3
    Author              :   
Modification            :   Created file
******************************************************************************/

#ifndef  __MT_MPI_VENC_H__
#define  __MT_MPI_VENC_H__

#include "mt_error_mpi.h"
#include "mt_unf_venc.h"
#include "drv_venc_ext.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif

/********************************Macro Definition********************************/
/** \addtogroup      H_2_2_11 */
/** @{ */  /** <!-- 【VENC】 */

/**<Maximum encoding width*/
/**<CNcomment: 最大编码宽度*/
#define MT_VENC_MAX_WIDTH       (1920)	
/**<Minimum encoding width*/	
/**<CNcomment: 最小编码宽度*/	
#define MT_VENC_MIN_WIDTH       (176)
/**<Maximum encoding height*/			  
/**<CNcomment: 最大编码高度*/
#define MT_VENC_MAX_HEIGTH      (1088)		
/**<Minimum encoding height*/
/**<CNcomment: 最小编码高度*/	
#define MT_VENC_MIN_HEIGTH      (144)	
/**<Picture size alignment (in bytes)*/		  
/**<CNcomment: 图像大小对齐字节*/
#define MT_VENC_PIC_SZIE_ALIGN  (4)	
/**<Maximum buffer (in bytes)*/		  
/**<CNcomment: 最大缓存字节*/
#define MT_VENC_MAX_BUF_SIZE    (20*1024*1024)	
/**<Minimum buffer (in bytes)*/
/**<CNcomment: 最小缓存字节*/ 
#define MT_VENC_MIN_BUF_SIZE    (256*1024)	
/**<Minimum group of picture (GOP)*/
/**<CNcomment: 最小GOP*/
#define MT_VENC_MIN_GOP         (1)		
/**<Maximum output bit rate*/		  
/**<CNcomment: 最大输出码率*/
#define MT_VENC_MAX_bps         (50*1024*1024)
/**<Minimum output bit rate*/	
/**<CNcomment: 最小输出码率*/
#define MT_VENC_MIN_bps         (32*1024)	
#if 0
/**<Maximum split size (in bytes)*/	
/**<CNcomment: 最大Split字节*/
#define MT_VENC_MAX_SPLIT_BYTE_SIZE  (0xFFFF)	
/**<Minimum split size (in bytes)*/	
/**<CNcomment: 最小Split字节*/
#define MT_VENC_MIN_SPLIT_BYTE_SIZE  (512)	

/**<Maximum split size (in bytes)*/	
/**<CNcomment: 最大Split字节*/
#define MT_VENC_MAX_SPLIT_MB_LINE  (511)	
#endif
/**Maximum frame rate*/			  
/**<CNcomment: 最大帧率*/
#define MT_VENC_MAX_fps         (60)
/**Minimum frame rate*/		
/**<CNcomment: 最小帧率*/	  
#define MT_VENC_MIN_fps         (1)		
/**Maximum channel priority*/		
/**<CNcomment: 最大通道优先级*/	  
#define MT_VENC_MAX_PRIORITY    (8)					  
/**<Maximum Quantization Parameter*/		 
/**<CNcomment: 最大量化参数值*/
#define MT_VENC_MAX_QP          (51)
/**<Size of reserved bytes for the bit rate*/		 
/**<CNcomment: 码率保留字节大小*/
#define MT_VENC_STREAM_RESERV_SIZE  (48)
/**<Maximum quantization level*/		  
/**<CNcomment: 最大量化级别*/
#define MT_VENC_MAX_Q_VALUE     (99)	
/**<Minimum quantization level*/ 	 
/**<CNcomment: 最小量化级别*/ 
#define MT_VENC_MIN_Q_VALUE     (1)	
/**<Maximum threshold of  bitrate fluctuation*/		  
/**<CNcomment: 最大编码码率波动阈值*/
#define MT_VENC_MAX_RcThr        (100)



/** @} */  /** <!-- ==== Macro Definition end ==== */


mt_s32 MT_MPI_VENC_Init(mt_void);
mt_s32 MT_MPI_VENC_DeInit(mt_void);
mt_s32 MT_MPI_VENC_SetAttr(mt_handle hVencChn, const MT_UNF_VENC_CHN_ATTR_S *pstAttr);
mt_s32 MT_MPI_VENC_GetAttr(mt_handle hVencChn, MT_UNF_VENC_CHN_ATTR_S *pstAttr);
mt_s32 MT_MPI_VENC_Create(mt_handle *phVencChn, const MT_UNF_VENC_CHN_ATTR_S *pstAttr);
mt_s32 MT_MPI_VENC_Destroy(mt_handle hVencChn);
mt_s32 MT_MPI_VENC_AttachInput(mt_handle hVencChn, mt_handle hSrc);
mt_s32 MT_MPI_VENC_DetachInput(mt_handle hVencChn);
mt_s32 MT_MPI_VENC_AcquireStream(mt_handle hVencChn, MT_UNF_VENC_STREAM_S *pstStream, mt_u32 u32BlockFlag);
mt_s32 MT_MPI_VENC_ReleaseStream(mt_handle hVencChn, const MT_UNF_VENC_STREAM_S *pstStream);
mt_s32 MT_MPI_VENC_Start(mt_handle hVencChn);
mt_s32 MT_MPI_VENC_Stop(mt_handle hVencChn);

mt_s32 MT_MPI_VENC_QueueFrame(mt_handle hVencChn,MT_UNF_VIDEO_FRAME_INFO_S *pstFrameinfo);
mt_s32 MT_MPI_VENC_DequeueFrame(mt_handle hVencChn,MT_UNF_VIDEO_FRAME_INFO_S *pstFrameinfo);
//mt_s32 MT_MPI_VENC_GetFrame(mt_handle hVencChn, MT_UNF_VI_BUF_S *pstFrame);
//mt_s32 MT_MPI_VENC_PutFrame(mt_handle hVencChn, const MT_UNF_VI_BUF_S *pstFrame);
mt_s32 MT_MPI_VENC_RequestIFrame(mt_handle hVencChn);
mt_s32 MT_MPI_VENC_SetSource(mt_handle hSrc, MT_DRV_VENC_SRC_INFO_S *pstSrc);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif

#endif //__MT_MPI_VENC_H__
