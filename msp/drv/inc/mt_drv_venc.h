/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
  File Name     : mt_drv_venc.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/3
  Description   :
  History       :
  1.Date        : 2015/12/3
    Author      :
    Modification: Created file

******************************************************************************/

#ifndef __MT_DRV_VENC_H__
#define __MT_DRV_VENC_H__

#include "mt_unf_venc.h"
#include "mt_drv_file.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define VENC_MAX_CHN_NUM 8

#define MT_FATAL_VENC(fmt...) MT_FATAL_PRINT(MT_ID_VENC, fmt)
#define MT_ERR_VENC(fmt...) MT_ERR_PRINT(MT_ID_VENC, fmt)
#define MT_WARN_VENC(fmt...) MT_WARN_PRINT(MT_ID_VENC, fmt)
#define MT_INFO_VENC(fmt...) MT_INFO_PRINT(MT_ID_VENC, fmt)
#define MT_DBG_VENC(fmt...) MT_DBG_PRINT(MT_ID_VENC, fmt)

#define PTR_COMPAT_T  void*

/*********************************************************************/
/* for omxvenc struction                                             */
/*********************************************************************/
/* VENC msg response types */
#define VENC_MSG_RESP_BASE 		        0xA0000
//#define VENC_MSG_RESP_OPEN              (VENC_MSG_RESP_BASE + 0x1)
#define VENC_MSG_RESP_START_DONE        (VENC_MSG_RESP_BASE + 0x1)
#define VENC_MSG_RESP_STOP_DONE        	(VENC_MSG_RESP_BASE + 0x2)
#define VENC_MSG_RESP_PAUSE_DONE        (VENC_MSG_RESP_BASE + 0x3)
#define VENC_MSG_RESP_RESUME_DONE	    (VENC_MSG_RESP_BASE + 0x4)
#define VENC_MSG_RESP_FLUSH_INPUT_DONE  (VENC_MSG_RESP_BASE + 0x5)
#define VENC_MSG_RESP_FLUSH_OUTPUT_DONE (VENC_MSG_RESP_BASE + 0x6)
#define VENC_MSG_RESP_INPUT_DONE        (VENC_MSG_RESP_BASE + 0x7)          //改帧可以还
#define VENC_MSG_RESP_OUTPUT_DONE       (VENC_MSG_RESP_BASE + 0x8)          //已经填满?
#define VENC_MSG_RESP_MSG_STOP_DONE	    (VENC_MSG_RESP_BASE + 0x9)

typedef enum VENC_DRV_CONTROLRATETYPE
{
    VENC_DRV_ControlRateDisable,
    VENC_DRV_ControlRateVariable,
    VENC_DRV_ControlRateConstant,
    VENC_DRV_ControlRateVariableSkipFrames,
    VENC_DRV_ControlRateConstantSkipFrames,
    VENC_DRV_ControlRateKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */
    VENC_DRV_ControlRateVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    VENC_DRV_ControlRateMax = 0x7FFFFFFF
} VENC_DRV_CONTROLRATETYPE_E;

#if 0
typedef struct venc_chan_cfg_s
{
    mt_u32 protocol;      /* VEDU_H264, VEDU_H263 or VEDU_MPEG4 */
    mt_u32 frame_width;    /* width	in pixel, 96 ~ 2048 */
    mt_u32 frame_height;   /* height in pixel, 96 ~ 2048 */
    mt_u32 CapLevel;
    MT_UNF_H264_PROFILE_E   VencProfile;

    mt_u32 rotation_angle; /* venc don't care */

    mt_u32 priority;
    mt_u32 streamBufSize;

    mt_u16 SlcSplitEn;    /* 0 or 1, slice split enable */
    //mt_u32 SplitSize;     /* 512 ~ max, bytes @ H264 & MP4, H263 don't care  */
    mt_u32 Gop;
    mt_u16 QuickEncode;

    mt_u32  TargetBitRate;
    mt_u32  TargetFrmRate;
    mt_u32  InputFrmRate;

    mt_u32  MinQP;
    mt_u32  MaxQP;
    ////////////////////////////////////////// just for omx priv
    mt_u32 h264Level;
    mt_u32 ControlRateType;    //match the enum OMX_VIDEO_CONTROLRATETYPE

    MT_BOOL bAutoRequestIFrm;
} venc_chan_cfg;
#endif
typedef struct venc_chan_cfg_s
{
    MT_UNF_VENC_CHN_ATTR_S VencUnfAttr;

    ////////////////////////////////////////// now just for omx priv
    MT_BOOL bAutoRequestIFrm;
    MT_BOOL bOmxChn;
    VENC_DRV_CONTROLRATETYPE_E ControlRateType;    //match the enum OMX_VIDEO_CONTROLRATETYPE
    mt_u32 h264Level;
    //mt_u32 SplitSize;     /* 512 ~ max, bytes @ H264 & MP4, H263 don't care  */
    //mt_u32 SplitType;
} venc_chan_cfg;

enum venc_port_dir
{
    PORT_DIR_INPUT,
    PORT_DIR_OUTPUT,
    PORT_DIR_BOTH = 0xFFFFFFFF
};

typedef struct venc_metadata_buf_s
{

    void*  bufferaddr;     //虚拟地址
    mt_u32 bufferaddr_Phy;
    mt_u32 vir2phy_offset;   //kernel VirAddr - PhyAddr
    mt_u32 buffer_size;    //buffer alloc size
    //mt_u32 offset_YC;    //YC分量的偏移
    //mt_u32 offset_YCr;   //YCr offset
    //mt_u32 offset;         //
    //mt_u32 data_len;      //filled len

    //void *ion_handle; /*used for ion*/

} venc_metadata_buf;

typedef struct venc_user_buf_s
{

    void*  bufferaddr;     //虚拟地址
    mt_u32 bufferaddr_Phy;
    mt_u32 vir2phy_offset;   //kernel VirAddr - PhyAddr
    mt_u32 buffer_size;    //buffer alloc size
    mt_u32 offset_YC;    //YC分量的偏移
    mt_u32 offset_YCr;   //YCr offset
    mt_u32 offset;         //
    mt_u32 data_len;      //filled len

    mt_u32 strideY;
    mt_u32 strideC;

    mt_u32  store_type;
    mt_u32  sample_type;
    mt_u32  package_sel;
    mt_u32 timestamp0;
    mt_u32 timestamp1;
    mt_u32  flags;

    mt_u32 picWidth;
    mt_u32 picHeight;
    enum venc_port_dir dir;

    mt_u32 MetaDateFlag;
#if 1
    venc_metadata_buf stMetaData;
#endif

    void* ion_handle; /*used for ion*/
    mt_s32 pmem_fd;
    unsigned long mmaped_size;

    //union user_buf_extra_info info;
    mt_u32 client_data;
} venc_user_buf;

typedef struct venc_user_info_s
{
    /*user state should be use*/
    void* bufferaddr;     //虚拟地址
    void* ion_handle;
    void* metabufferaddr;     //虚拟地址

    /*kernel state should be use*/
    venc_user_buf user_buf;
}venc_user_info;


typedef struct venc_msginfo_s
{
    mt_u32 status_code;          //记录操作的返回值(success/failure)
    mt_u32 msgcode;              //自定义的上行消息返回值，定义在此处
    venc_user_buf buf;     //
    mt_u32 msgdatasize;
} venc_msginfo;
#define OMXVENC_BUFFERFLAG_EOS 0x00000001
#define OMXVENC_BUFFERFLAG_STARTTIME 0x00000002
#define OMXVENC_BUFFERFLAG_DECODEONLY 0x00000004
#define OMXVENC_BUFFERFLAG_DATACORRUPT 0x00000008
#define OMXVENC_BUFFERFLAG_ENDOFFRAME 0x00000010
#define OMXVENC_BUFFERFLAG_SYNCFRAME 0x00000020
#define OMXVENC_BUFFERFLAG_EXTRADATA 0x00000040
#define OMXVENC_BUFFERFLAG_CODECCONFIG 0x00000080


typedef struct DRV_VIDEO_PPS_SPS_DATA
{
    mt_u8 nSize;
    mt_u8 Data[100];
} DRV_VIDEO_PPS_SPS_DATA;
/*********************************************************************/
/* for omxvenc struction  ->end                                      */
/*********************************************************************/

mt_s32 MT_DRV_VENC_Init(mt_void);
mt_s32 MT_DRV_VENC_DeInit(mt_void);
mt_s32 MT_DRV_VENC_GetDefaultAttr(MT_UNF_VENC_CHN_ATTR_S* pstAttr);
mt_s32 MT_DRV_VENC_Create(mt_handle* phVencChn, MT_UNF_VENC_CHN_ATTR_S* pstAttr, MT_BOOL bOMXChn, struct file*  pfile);
mt_s32 MT_DRV_VENC_Destroy(mt_handle hVenc);
mt_s32 MT_DRV_VENC_AttachInput(mt_handle hVenc, mt_handle hSrc);
mt_s32 MT_DRV_VENC_DetachInput(mt_handle hVencChn);
mt_s32 MT_DRV_VENC_Start(mt_handle hVenc);
mt_s32 MT_DRV_VENC_Stop(mt_handle hVenc);
mt_s32 MT_DRV_VENC_AcquireStream(mt_handle hVenc, MT_UNF_VENC_STREAM_S* pstStream, mt_u32 u32TimeoutMs);
mt_s32 MT_DRV_VENC_ReleaseStream(mt_handle hVenc, MT_UNF_VENC_STREAM_S* pstStream);

mt_s32 MT_DRV_VENC_SetAttr(mt_handle hVenc, MT_UNF_VENC_CHN_ATTR_S* pstAttr);
mt_s32 MT_DRV_VENC_GetAttr(mt_handle hVenc, MT_UNF_VENC_CHN_ATTR_S* pstAttr);
mt_s32 MT_DRV_VENC_RequestIFrame(mt_handle hVenc);
mt_s32 MT_DRV_VENC_QueueFrame(mt_handle hVenc, MT_UNF_VIDEO_FRAME_INFO_S* pstFrameinfo);
mt_s32 MT_DRV_VENC_DequeueFrame(mt_handle hVenc, MT_UNF_VIDEO_FRAME_INFO_S* pstFrameinfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif //__MT_DRV_VENC_H__
