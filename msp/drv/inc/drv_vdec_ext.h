/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_VDEC_EXT_H__
#define __DRV_VDEC_EXT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_type.h"
#include "mt_drv_vdec_ioctl.h"
#include "mt_drv_vdec.h"
#include "mt_mpi_vdec.h"
#include "mt_drv_dev.h"

/**Compress Info*/
typedef struct hiVDEC_COMPRESS_INFO_S
{
    mt_u32 u32CompressFlag;
    mt_s32 s32CompFrameHeight;
    mt_s32 s32CompFrameWidth;
    mt_u32 u32HeadOffset;            /**<DNRInfo head offset */
    mt_u32 u32YHeadAddr;             /**<Y head info when compress is used */
    mt_u32 u32CHeadAddr;             /**<C head info when compress is used */
    mt_u32 u32HeadStride;            /**<YC head info stride when compress is used */
}MT_VDEC_COMPRESS_INFO_S;

/**VC1 Range Info*/
typedef struct hiVDEC_VC1_RANGE_INFO_S
{
    mt_u8 u8PicStructure;     /**< 0: frame, 1: top, 2: bottom, 3: mbaff, 4: field pair */
    mt_u8 u8PicQPEnable;
    mt_u8 u8ChromaFormatIdc;  /**< 0: yuv400, 1: yuv420 */
    mt_u8 u8VC1Profile;

    mt_s32 s32QPY;
    mt_s32 s32QPU;
    mt_s32 s32QPV;
    mt_s32 s32RangedFrm;

    mt_u8 u8RangeMapYFlag;
    mt_u8 u8RangeMapY;
    mt_u8 u8RangeMapUVFlag;
    mt_u8 u8RangeMapUV;
    mt_u8 u8BtmRangeMapYFlag;
    mt_u8 u8BtmRangeMapY;
    mt_u8 u8BtmRangeMapUVFlag;
    mt_u8 u8BtmRangeMapUV;
}MT_VDEC_VC1_RANGE_INFO_S;

/**BTL Info*/
typedef struct hiVDEC_BTL_INFO_S
{
    mt_u32 u32BTLImageID;
    mt_u32 u32Is1D;         /**< 0:2D, 1:1D */
    mt_u32 u32IsCompress;

    mt_u32 u32DNROpen;      /**< 0: DNR close, 1: DNR open */
    mt_u32 u32DNRInfoAddr;  /**< DNR info from BTL */
    mt_u32 u32DNRInfoStride;/**< DNR info stride from BTL */
}MT_VDEC_BTL_INFO_S;

typedef struct hiVDEC_PRIV_FRAMEINFO_S
{
    mt_u32                      u32BeVC1;
    MT_VDEC_COMPRESS_INFO_S     stCompressInfo;
    MT_VDEC_VC1_RANGE_INFO_S    stVC1RangeInfo;
    MT_VDEC_BTL_INFO_S          stBTLInfo;
    MT_UNF_VCODEC_TYPE_E        entype;
    mt_u32                      u32SeqFrameCnt;     /**<Picture ID in a video sequence. The ID of the first frame in each sequence is numbered 0*/ /**<CNcomment: 视频序列中的图像编号，每个序列中第一帧编号为0*/
    mt_u32                      u32DispTime;        /**<PVR Display time*/
    mt_u32                      image_id;
    mt_u32                      image_id_1;
    mt_s32                      s32InterPtsDelta;   /*interleaved source, VPSS module swtich field to frame, need to adjust pts*/
    mt_u8                       u8Repeat;           /**<Times of playing a video frame*/ /**<CNcomment: 视频帧播放次数.*/
    mt_u8                       u8EndFrame;         /**<0 Not end frame; 1 Current frame is the end frame; 2 Prior frame is the end frame */
    mt_u8                       u8TestFlag;         /**<VDEC_OPTMALG_INFO_S.Rwzb*/
    mt_u8                       u8Marker;           /**<Bit0: 1 Resolution change
                                                        Bit1: 1 close deinterlace
                                                    */
    mt_u32                      u32OriFrameRate;  /* 1000*rate */
    mt_s32                      s32GopNum;
    mt_s32                      s32FrameFormat;
    mt_s32                      s32TopFieldFrameFormat;
    mt_s32                      s32BottomFieldFrameFormat;
    mt_s32                      s32FieldFlag;
}MT_VDEC_PRIV_FRAMEINFO_S;


/* Describe a es buffer instance */
typedef struct mtVESINST_S
{
    mt_handle   hBuf;               /* Handle of this buffer instance */
    phys_addr_t      u32PhyAddr;         /* Start physical address of the buffer instance. */
    mt_u8*      pu8UsrVirAddr;      /* Start user virtual address of the buffer instance. */
    mt_u8*      pu8KnlVirAddr;      /* Start kerenl virtual address of the buffer instance. */
    mt_u32      u32Size;            /* Size of the buffer instance */
    mt_u8*      pu8KnlVirDescAddr;
    mt_u32      u32KnlVirDescBufSize;
    mt_u32      u32KnlDescWriterOff;
    mt_u32      u32KnDescReadOff;
    mt_u32      u32VesBufferChannelId;
    phys_addr_t      u32DescPhyAddr;
} VES_INST_S;

typedef mt_s32  (*FN_VDEC_Suspend)(basedev_s *, pm_message_t state);
typedef mt_s32  (*FN_VDEC_Resume)(basedev_s *);

typedef struct tagVDEC_EXPORT_FUNC_S
{
    FN_VDEC_Suspend             pfnVDEC_Suspend;
    FN_VDEC_Resume              pfnVDEC_Resume;
}VDEC_EXPORT_FUNC_S;


mt_s32 VDEC_DRV_ModInit(mt_void);
mt_void VDEC_DRV_ModExit(mt_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __DRV_VDEC_EXT_H__ */

