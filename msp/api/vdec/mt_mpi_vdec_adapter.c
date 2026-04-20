/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
 /******************************************************************************
  File Name     : mt_mpi_vdec_adapter.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/3
  Description   : Implement for vdec driver
  History       :
  1.Date        : 2015/12/3
    Author      :
    Modification: Created file

*******************************************************************************/

/******************************* Include Files *******************************/

/* Sys headers */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <pthread.h>

/* Unf headers */
#include "mt_video_codec.h"

/* Mpi headers */
#include "mt_error_mpi.h"
#include "mt_mpi_mem.h"
#include "mt_mpi_vdec_adapter.h"
#include "mpi_memdev.h"

/* Drv headers */
#include "mt_drv_struct.h"
#include "vfmw.h"
#include "mt_drv_vdec.h"
#include "mt_drv_video.h"
#include "list.h"
#include "mt_drv_vdec_ioctl.h"
#include "mt_module_debug.h"
#include "mt_drv_mmz.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

/****************************** Macro Definition *****************************/
//FIX: Bug 120872
//#define MT_VDEC_USERDATA_CC_BUFSIZE 4*1024
#define MT_VDEC_USERDATA_CC_BUFSIZE (128*1024)

#define VFMW_INST_HANDLE(hInst) (hInst & 0xFF)

#define VFMW_FIND_INST(hInst, pVFMWInst) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        VFMW_INST_S* pstTmp; \
        if (!list_empty(&s_stVdecAdpParam.stInstHead)) \
        { \
            list_for_each_safe(pos, n, &s_stVdecAdpParam.stInstHead) \
            { \
                pstTmp = list_entry(pos, VFMW_INST_S, stInstNode); \
                if (VFMW_INST_HANDLE(hInst) == pstTmp->hInst) \
                { \
                    pVFMWInst = pstTmp; \
                    break; \
                } \
            } \
        } \
    }

/* Find an instance pointer from list by handle */
#define STRMBUF_FIND_INST(hBuf, pBufInst) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        STREAM_BUF_INST_S* pstTmp; \
        VDEC_LOCK(s_stStrmBufParam.stMutex); \
        if (!list_empty(&s_stStrmBufParam.stBufHead)) \
        { \
            list_for_each_safe(pos, n, &s_stStrmBufParam.stBufHead) \
            { \
                pstTmp = list_entry(pos, STREAM_BUF_INST_S, stBufNode); \
                if (hBuf == pstTmp->hBuf) \
                { \
                    pBufInst = pstTmp; \
                    break; \
                } \
            } \
        } \
        VDEC_UNLOCK(s_stStrmBufParam.stMutex); \
    }

/* Find an instance pointer from list by handle */
#define FRMBUF_FIND_INST(hBuf, pBufInst) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        FRAME_BUF_INST_S* pstTmp; \
        VDEC_LOCK(s_stFrmBufParam.stMutex); \
        if (!list_empty(&s_stFrmBufParam.stBufHead)) \
        { \
            list_for_each_safe(pos, n, &s_stFrmBufParam.stBufHead) \
            { \
                pstTmp = list_entry(pos, FRAME_BUF_INST_S, stBufNode); \
                if (hBuf == pstTmp->hBuf) \
                { \
                    pBufInst = pstTmp; \
                    break; \
                } \
            } \
        } \
        VDEC_UNLOCK(s_stFrmBufParam.stMutex); \
    }

/************************ Static Structure Definition ************************/

/* Describe a VFMW instance */
typedef struct tagVFMW_INST_S
{
    mt_handle        hInst;                         /* Instance handle */
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    mt_u8*           pu8UserDataVirAddr;            /* User data MMZ user space address  */
#endif
    mt_u8            au8UsrData[MAX_USER_DATA_LEN]; /* Userdata buffer */
    struct list_head stInstNode;                    /* Instance list node */
} VFMW_INST_S;

/* Global parameters for VFMW */
typedef struct tagVFMW_GLOBAL_S
{
    const mt_char*      pszDevPath;     /* Device name */
    mt_s32              s32DevFd;       /* Device file handle */
    mt_u32              u32InitCount;   /* Init counter */
    MT_CODEC_CAP_S      stCap;          /* VFMW capability */
    pthread_mutex_t     stMutex;        /* Mutex */
    struct list_head    stInstHead;     /* Instance list head */
} VDEC_ADP_GLOBAL_S;

/* Describe a stream buffer instance */
typedef struct tagSTREAM_BUF_INST_S
{
    mt_handle     hBuf;                 /* BM buffer handle */
    mt_u32        u32Size;              /* Size */
    mt_u8*        pu8MMZVirAddr;        /* MMZ virtual address */
    MT_BOOL       bGetPutFlag;          /* Gut/Put flag */
    VDEC_ES_BUF_S stLastGet;            /* Last get buffer */
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) ||(MT_VDEC_VPU_SUPPORT == 1)
    MT_BOOL       bRecvRlsFlag;         /* Receive/Release flag */
    VDEC_ES_BUF_S stLastRecv;           /* Last Receive buffer */
#endif
    struct list_head stBufNode;         /* Instance list node */
} STREAM_BUF_INST_S;

/* Global parameters for stream buffer */
typedef struct tagSTREAM_BUF_GLOBAL_S
{
    pthread_mutex_t     stMutex;        /* Mutex */
    struct list_head    stBufHead;      /* Instance list head */
} STREAM_BUF_GLOBAL_S;

/* Describe a frame buffer instance */
typedef struct tagFRAME_BUF_INST_S
{
    mt_handle     hBuf;                 /* Frame buffer handle */
    MT_BOOL       bGetPutFlag;          /* Gut/Put flag */
    MT_DRV_VDEC_FRAME_BUF_S stLastGet;         /* Last get buffer */
    struct list_head stBufNode;         /* Instance list node */
} FRAME_BUF_INST_S;

/* Global parameters for stream buffer */
typedef struct tagFRAME_BUF_GLOBAL_S
{
    pthread_mutex_t     stMutex;        /* Mutex */
    struct list_head    stBufHead;      /* Instance list head */
} FRAME_BUF_GLOBAL_S;


/***************************** Global Definition *****************************/


/***************************** Static Definition *****************************/

static mt_s32 VFMW_GetCap(MT_CODEC_CAP_S *pstCodecCap);
static mt_s32 VFMW_Create(mt_handle* phInst, const MT_CODEC_OPENPARAM_S * pstParam);
static mt_s32 VFMW_Destroy(mt_handle hInst);
static mt_s32 VFMW_Start(mt_handle hInst);
static mt_s32 VFMW_Stop(mt_handle hInst);
static mt_s32 VFMW_Reset(mt_handle hInst, const MT_CODEC_RESETPARAM_S *pstParam);
static mt_s32 VFMW_SetAttr(mt_handle hInst, const MT_CODEC_ATTR_S * pstAttr);
static mt_s32 VFMW_GetAttr(mt_handle hInst, MT_CODEC_ATTR_S* pstAttr);
static mt_s32 VFMW_GetStreamInfo(mt_handle hInst, MT_CODEC_STREAMINFO_S *pstStreamInfo);
static mt_s32 VFMW_Control(mt_handle hInst, mt_u32 u32CMD, mt_void * pParam);

static VDEC_ADP_GLOBAL_S s_stVdecAdpParam =
{
    .pszDevPath   = "/dev/" UMAP_DEVNAME_VDEC,
    .s32DevFd     = -1,
    .u32InitCount = 0,
    .stCap        = {MT_CODEC_CAP_DRIVENSELF | MT_CODEC_CAP_OUTPUT2SELFADDR, MT_NULL},
    .stMutex      = PTHREAD_MUTEX_INITIALIZER,
    .stInstHead   = {&s_stVdecAdpParam.stInstHead, &s_stVdecAdpParam.stInstHead}
};

static MT_CODEC_S s_stCodec =
{
    .pszName		= "VFMW",
    .unVersion		= {.stVersion = {1, 0, 0, 0}},
    .pszDescription = "Montage hardware codec",

    .GetCap			= VFMW_GetCap,
    .Create			= VFMW_Create,
    .Destroy		= VFMW_Destroy,
    .Start			= VFMW_Start,
    .Stop			= VFMW_Stop,
    .Reset			= VFMW_Reset,
    .SetAttr		= VFMW_SetAttr,
    .GetAttr		= VFMW_GetAttr,
    .DecodeFrame	= MT_NULL,
    .EncodeFrame	= MT_NULL,
    .GetStreamInfo	= VFMW_GetStreamInfo,
    .Control		= VFMW_Control,
};

static STREAM_BUF_GLOBAL_S s_stStrmBufParam =
{
    .stMutex   = PTHREAD_MUTEX_INITIALIZER,
    .stBufHead = {&s_stStrmBufParam.stBufHead,	&s_stStrmBufParam.stBufHead}
};

static FRAME_BUF_GLOBAL_S s_stFrmBufParam =
{
    .stMutex   = PTHREAD_MUTEX_INITIALIZER,
    .stBufHead = {&s_stFrmBufParam.stBufHead,	&s_stFrmBufParam.stBufHead}
};

mt_s32 VDEC_GetFBMem(mmz_buffer_s  * mmz_buf)
{
	mt_s32 s32Ret;

	VDEC_LOCK(s_stVdecAdpParam.stMutex);

	s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_GETFB_MEM, mmz_buf);

	if (MT_SUCCESS != s32Ret)
	{
		VDEC_UNLOCK(s_stVdecAdpParam.stMutex);
		return MT_FAILURE;
	}

	VDEC_UNLOCK(s_stVdecAdpParam.stMutex);
	return MT_SUCCESS;
}


#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT

mt_s32 VDEC_GeLcevcMem(mt_handle hInst, mmz_buffer_s  * mmz_buf)
{
	mt_s32 s32Ret;
	VDEC_CMD_GET_LEVCMMZ_S stParam;

	if (MT_NULL == mmz_buf)
	{
		MT_ERR_VDEC("Bad param.\n");
		return MT_ERR_CODEC_INVALIDPARAM;
	}

	stParam.hHandle = VFMW_INST_HANDLE(hInst);
	s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_GETLCEVC_MEM, &stParam);
	if (MT_SUCCESS != s32Ret){
	//	MT_ERR_VDEC("Chan %d UMAPC_VDEC_GETLCEVC_MEM err:%x!\n", stParam.hHandle, s32Ret);
		return s32Ret;
	}
	*mmz_buf = stParam.mmz_buf;
	return MT_SUCCESS;

}

mt_s32 VDEC_GetLcevcSEIData(mt_handle hInst, MT_VDEC_LCEVC_DATA_S * plecvc_data)
{
	mt_s32 s32Ret;
	VDEC_CMD_GET_LEVCDATA_S stParam;

	if (MT_NULL == plecvc_data)
	{
		MT_ERR_VDEC("Bad param.\n");
		return MT_ERR_CODEC_INVALIDPARAM;
	}

	stParam.hHandle = VFMW_INST_HANDLE(hInst);
	s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_GET_LCEVC_DATA, &stParam);
	if (MT_SUCCESS != s32Ret){
		if(MT_ERR_VDEC_RECEIVE_FAILED != s32Ret)
		{
			MT_ERR_VDEC("Chan %d GetLcevcData err:%x!\n", stParam.hHandle, s32Ret);
		}
		return s32Ret;
	}
	*plecvc_data = stParam.lcevcData;
	
	return MT_SUCCESS;
}

#endif

/*********************************** Code ************************************/

/* Open VDEC device, initialize all the channel config info and the ES buffer */
mt_s32 VDEC_OpenDevFile(mt_void)
{
    VDEC_LOCK(s_stVdecAdpParam.stMutex);

    if (0 == s_stVdecAdpParam.u32InitCount)
    {
        if (-1 == s_stVdecAdpParam.s32DevFd)
        {
            /* Open dev */
            s_stVdecAdpParam.s32DevFd = open(s_stVdecAdpParam.pszDevPath, O_RDWR | O_NONBLOCK | O_CLOEXEC, 0);
            if (-1 == s_stVdecAdpParam.s32DevFd)
            {
                VDEC_UNLOCK(s_stVdecAdpParam.stMutex);
                MT_FATAL_VDEC("Open video device err!\n");
                return MT_ERR_VDEC_NOT_OPEN;
            }
		#if (1 == MT_VDEC_VPU_SUPPORT)
			if (MT_SUCCESS != VDEC_VPU_Init())
			{
				MT_ERR_VDEC("vdec vpu init failed\n");
			}
		#endif
        }
    }

    s_stVdecAdpParam.u32InitCount++;
    VDEC_UNLOCK(s_stVdecAdpParam.stMutex);
    return MT_SUCCESS;
}

mt_s32 VDEC_CloseDevFile(mt_void)
{
    VDEC_LOCK(s_stVdecAdpParam.stMutex);

    if (1 == s_stVdecAdpParam.u32InitCount)
    {
        /* Free VFMW capability structure */
        if (s_stVdecAdpParam.stCap.pstSupport)
        {
            MT_FREE_VDEC(s_stVdecAdpParam.stCap.pstSupport);
            s_stVdecAdpParam.stCap.pstSupport = MT_NULL;
        }

        /* Close dev */
        if (-1 != s_stVdecAdpParam.s32DevFd)
        {
            close(s_stVdecAdpParam.s32DevFd);
            s_stVdecAdpParam.s32DevFd = -1;
        }
    }

    if (s_stVdecAdpParam.u32InitCount > 0)
    {
        s_stVdecAdpParam.u32InitCount--;
    }

    VDEC_UNLOCK(s_stVdecAdpParam.stMutex);
    return MT_SUCCESS;
}

MT_CODEC_ID_E VDEC_UNF2CodecId(MT_UNF_VCODEC_TYPE_E enType)
{
    switch (enType)
    {
    case MT_UNF_VCODEC_TYPE_MPEG2:
        return MT_CODEC_ID_VIDEO_MPEG2;
    case MT_UNF_VCODEC_TYPE_MPEG4:
        return MT_CODEC_ID_VIDEO_MPEG4;
    case MT_UNF_VCODEC_TYPE_AVS:
        return MT_CODEC_ID_VIDEO_AVS;
	case MT_UNF_VCODEC_TYPE_AVS2:
		return MT_CODEC_ID_VIDEO_AVS2;
    case MT_UNF_VCODEC_TYPE_H263:
        return MT_CODEC_ID_VIDEO_H263;
    case MT_UNF_VCODEC_TYPE_H264:
        return MT_CODEC_ID_VIDEO_H264;
    case MT_UNF_VCODEC_TYPE_REAL8:
        return MT_CODEC_ID_VIDEO_REAL8;
    case MT_UNF_VCODEC_TYPE_REAL9:
        return MT_CODEC_ID_VIDEO_REAL9;
    case MT_UNF_VCODEC_TYPE_VC1:
        return MT_CODEC_ID_VIDEO_VC1;
    case MT_UNF_VCODEC_TYPE_VP6:
        return MT_CODEC_ID_VIDEO_VP6;
    case MT_UNF_VCODEC_TYPE_VP6F:
        return MT_CODEC_ID_VIDEO_VP6F;
    case MT_UNF_VCODEC_TYPE_VP6A:
        return MT_CODEC_ID_VIDEO_VP6A;
    case MT_UNF_VCODEC_TYPE_VP8:
        return MT_CODEC_ID_VIDEO_VP8;
    case MT_UNF_VCODEC_TYPE_MJPEG:
        return MT_CODEC_ID_VIDEO_MJPEG;
    case MT_UNF_VCODEC_TYPE_SORENSON:
        return MT_CODEC_ID_VIDEO_SORENSON;
    case MT_UNF_VCODEC_TYPE_DIVX3:
        return MT_CODEC_ID_VIDEO_DIVX3;
    case MT_UNF_VCODEC_TYPE_RAW:
        return MT_CODEC_ID_VIDEO_RAW;
    case MT_UNF_VCODEC_TYPE_JPEG:
        return MT_CODEC_ID_VIDEO_JPEG;
    case MT_UNF_VCODEC_TYPE_MSMPEG4V1:
        return MT_CODEC_ID_VIDEO_MSMPEG4V1;
    case MT_UNF_VCODEC_TYPE_MSMPEG4V2:
        return MT_CODEC_ID_VIDEO_MSMPEG4V2;
    case MT_UNF_VCODEC_TYPE_MSVIDEO1:
        return MT_CODEC_ID_VIDEO_MSVIDEO1;
    case MT_UNF_VCODEC_TYPE_WMV1:
        return MT_CODEC_ID_VIDEO_WMV1;
    case MT_UNF_VCODEC_TYPE_WMV2:
        return MT_CODEC_ID_VIDEO_WMV2;
    case MT_UNF_VCODEC_TYPE_RV10:
        return MT_CODEC_ID_VIDEO_RV10;
    case MT_UNF_VCODEC_TYPE_RV20:
        return MT_CODEC_ID_VIDEO_RV20;
    case MT_UNF_VCODEC_TYPE_SVQ1:
        return MT_CODEC_ID_VIDEO_SVQ1;
    case MT_UNF_VCODEC_TYPE_SVQ3:
        return MT_CODEC_ID_VIDEO_SVQ3;
    case MT_UNF_VCODEC_TYPE_H261:
        return MT_CODEC_ID_VIDEO_H261;
    case MT_UNF_VCODEC_TYPE_VP3:
        return MT_CODEC_ID_VIDEO_VP3;
    case MT_UNF_VCODEC_TYPE_VP5:
        return MT_CODEC_ID_VIDEO_VP5;
    case MT_UNF_VCODEC_TYPE_CINEPAK:
        return MT_CODEC_ID_VIDEO_CINEPAK;
    case MT_UNF_VCODEC_TYPE_INDEO2:
        return MT_CODEC_ID_VIDEO_INDEO2;
    case MT_UNF_VCODEC_TYPE_INDEO3:
        return MT_CODEC_ID_VIDEO_INDEO3;
    case MT_UNF_VCODEC_TYPE_INDEO4:
        return MT_CODEC_ID_VIDEO_INDEO4;
    case MT_UNF_VCODEC_TYPE_INDEO5:
        return MT_CODEC_ID_VIDEO_INDEO5;
    case MT_UNF_VCODEC_TYPE_MJPEGB:
        return MT_CODEC_ID_VIDEO_MJPEGB;
    case MT_UNF_VCODEC_TYPE_DV:
		return MT_CODEC_ID_VIDEO_DV;
	case MT_UNF_VCODEC_TYPE_HEVC:
		return MT_CODEC_ID_VIDEO_HEVC;
	case MT_UNF_VCODEC_TYPE_VP9:
		return MT_CODEC_ID_VIDEO_VP9;
    default:
        return MT_CODEC_ID_NONE;
    }
}

MT_UNF_VCODEC_TYPE_E VDEC_CodecId2UNF(MT_CODEC_ID_E enCodecId)
{
    switch (enCodecId)
    {
    case MT_CODEC_ID_VIDEO_MPEG2:
        return MT_UNF_VCODEC_TYPE_MPEG2;
    case MT_CODEC_ID_VIDEO_MPEG4:
        return MT_UNF_VCODEC_TYPE_MPEG4;
    case MT_CODEC_ID_VIDEO_AVS:
        return MT_UNF_VCODEC_TYPE_AVS;
    case MT_CODEC_ID_VIDEO_H263:
        return MT_UNF_VCODEC_TYPE_H263;
    case MT_CODEC_ID_VIDEO_H264:
        return MT_UNF_VCODEC_TYPE_H264;
    case MT_CODEC_ID_VIDEO_HEVC:
        return MT_UNF_VCODEC_TYPE_HEVC;
    case MT_CODEC_ID_VIDEO_REAL8:
        return MT_UNF_VCODEC_TYPE_REAL8;
    case MT_CODEC_ID_VIDEO_REAL9:
        return MT_UNF_VCODEC_TYPE_REAL9;
    case MT_CODEC_ID_VIDEO_VC1:
        return MT_UNF_VCODEC_TYPE_VC1;
    case MT_CODEC_ID_VIDEO_VP6:
        return MT_UNF_VCODEC_TYPE_VP6;
    case MT_CODEC_ID_VIDEO_VP6F:
        return MT_UNF_VCODEC_TYPE_VP6F;
    case MT_CODEC_ID_VIDEO_VP6A:
        return MT_UNF_VCODEC_TYPE_VP6A;
    case MT_CODEC_ID_VIDEO_VP8:
        return MT_UNF_VCODEC_TYPE_VP8;
    case MT_CODEC_ID_VIDEO_MJPEG:
        return MT_UNF_VCODEC_TYPE_MJPEG;
    case MT_CODEC_ID_VIDEO_SORENSON:
        return MT_UNF_VCODEC_TYPE_SORENSON;
    case MT_CODEC_ID_VIDEO_DIVX3:
        return MT_UNF_VCODEC_TYPE_DIVX3;
    case MT_CODEC_ID_VIDEO_RAW:
        return MT_UNF_VCODEC_TYPE_RAW;
    case MT_CODEC_ID_VIDEO_JPEG:
        return MT_UNF_VCODEC_TYPE_JPEG;
    case MT_CODEC_ID_VIDEO_MSMPEG4V1:
        return MT_UNF_VCODEC_TYPE_MSMPEG4V1;
    case MT_CODEC_ID_VIDEO_MSMPEG4V2:
        return MT_UNF_VCODEC_TYPE_MSMPEG4V2;
    case MT_CODEC_ID_VIDEO_MSVIDEO1:
        return MT_UNF_VCODEC_TYPE_MSVIDEO1;
    case MT_CODEC_ID_VIDEO_WMV1:
        return MT_UNF_VCODEC_TYPE_WMV1;
    case MT_CODEC_ID_VIDEO_WMV2:
        return MT_UNF_VCODEC_TYPE_WMV2;
    case MT_CODEC_ID_VIDEO_RV10:
        return MT_UNF_VCODEC_TYPE_RV10;
    case MT_CODEC_ID_VIDEO_RV20:
        return MT_UNF_VCODEC_TYPE_RV20;
    case MT_CODEC_ID_VIDEO_SVQ1:
        return MT_UNF_VCODEC_TYPE_SVQ1;
    case MT_CODEC_ID_VIDEO_SVQ3:
        return MT_UNF_VCODEC_TYPE_SVQ3;
    case MT_CODEC_ID_VIDEO_H261:
        return MT_UNF_VCODEC_TYPE_H261;
    case MT_CODEC_ID_VIDEO_VP3:
        return MT_UNF_VCODEC_TYPE_VP3;
    case MT_CODEC_ID_VIDEO_VP5:
        return MT_UNF_VCODEC_TYPE_VP5;
    case MT_CODEC_ID_VIDEO_CINEPAK:
        return MT_UNF_VCODEC_TYPE_CINEPAK;
    case MT_CODEC_ID_VIDEO_INDEO2:
        return MT_UNF_VCODEC_TYPE_INDEO2;
    case MT_CODEC_ID_VIDEO_INDEO3:
        return MT_UNF_VCODEC_TYPE_INDEO3;
    case MT_CODEC_ID_VIDEO_INDEO4:
        return MT_UNF_VCODEC_TYPE_INDEO4;
    case MT_CODEC_ID_VIDEO_INDEO5:
        return MT_UNF_VCODEC_TYPE_INDEO5;
    case MT_CODEC_ID_VIDEO_MJPEGB:
        return MT_UNF_VCODEC_TYPE_MJPEGB;
	case MT_CODEC_ID_VIDEO_DV:
		return MT_UNF_VCODEC_TYPE_DV;
	case MT_CODEC_ID_VIDEO_VP9:
		return MT_UNF_VCODEC_TYPE_VP9;
	case MT_CODEC_ID_VIDEO_AVS2:
		return MT_UNF_VCODEC_TYPE_AVS2;
    default:
        return MT_UNF_VCODEC_TYPE_BUTT;
    }
}

static MT_CODEC_ID_E VDEC_VFMW_STD2CodecId(VID_STD_E enVidStd)
{
    switch (enVidStd)
    {
    case VFMW_H264:
        return MT_CODEC_ID_VIDEO_H264;
    case VFMW_HEVC:
        return MT_CODEC_ID_VIDEO_HEVC;
    case VFMW_VC1:
        return MT_CODEC_ID_VIDEO_VC1;
    case VFMW_MPEG4:
        return MT_CODEC_ID_VIDEO_MPEG4;
    case VFMW_MPEG2:
        return MT_CODEC_ID_VIDEO_MPEG2;
    case VFMW_H263:
        return MT_CODEC_ID_VIDEO_H263;
    case VFMW_DIVX3:
        return MT_CODEC_ID_VIDEO_DIVX3;
    case VFMW_AVS:
        return MT_CODEC_ID_VIDEO_AVS;
	case VFMW_AVS2:
		return MT_CODEC_ID_VIDEO_AVS2;
    case VFMW_JPEG:
        return MT_CODEC_ID_VIDEO_JPEG;
    case VFMW_REAL8:
        return MT_CODEC_ID_VIDEO_REAL8;
    case VFMW_REAL9:
        return MT_CODEC_ID_VIDEO_REAL9;
    case VFMW_VP6:
        return MT_CODEC_ID_VIDEO_VP6;
    case VFMW_VP6F:
        return MT_CODEC_ID_VIDEO_VP6F;
    case VFMW_VP6A:
        return MT_CODEC_ID_VIDEO_VP6A;
    case VFMW_VP8:
        return MT_CODEC_ID_VIDEO_VP8;
    case VFMW_SORENSON:
        return MT_CODEC_ID_VIDEO_SORENSON;
    case VFMW_RAW:
        return MT_CODEC_ID_VIDEO_RAW;
	case VFMW_VP9:
		return MT_CODEC_ID_VIDEO_VP9;
    default:
        return MT_CODEC_ID_NONE;
    }
}

MT_UNF_ENC_FMT_E VDEC_DisplayFmt2UNF(MT_CODEC_ENC_FMT_E enDisplayNorm)
{
    switch(enDisplayNorm)
    {
    case MT_CODEC_ENC_FMT_1080P_60:
        return MT_UNF_ENC_FMT_1080P_60;
    case MT_CODEC_ENC_FMT_1080P_50:
        return MT_UNF_ENC_FMT_1080P_50;
    case MT_CODEC_ENC_FMT_1080P_30:
        return MT_UNF_ENC_FMT_1080P_30;
    case MT_CODEC_ENC_FMT_1080P_25:
        return MT_UNF_ENC_FMT_1080P_25;
    case MT_CODEC_ENC_FMT_1080P_24:
        return MT_UNF_ENC_FMT_1080P_24;

    case MT_CODEC_ENC_FMT_1080i_60:
        return MT_UNF_ENC_FMT_1080i_60;
   case MT_CODEC_ENC_FMT_1080i_50:
        return MT_UNF_ENC_FMT_1080i_50;
    case MT_CODEC_ENC_FMT_720P_60:
        return MT_UNF_ENC_FMT_720P_60;
    case MT_CODEC_ENC_FMT_720P_50:
        return MT_UNF_ENC_FMT_720P_50;

    case MT_CODEC_ENC_FMT_576P_50:
        return MT_UNF_ENC_FMT_576P_50;
    case MT_CODEC_ENC_FMT_480P_60:
        return MT_UNF_ENC_FMT_480P_60;

    case MT_CODEC_ENC_FMT_PAL:
        return MT_UNF_ENC_FMT_PAL;
    case MT_CODEC_ENC_FMT_PAL_N:
        return MT_UNF_ENC_FMT_PAL_N;
    case MT_CODEC_ENC_FMT_PAL_Nc:
        return MT_UNF_ENC_FMT_PAL_Nc;

    case MT_CODEC_ENC_FMT_NTSC:
        return MT_UNF_ENC_FMT_NTSC;
    case MT_CODEC_ENC_FMT_NTSC_J:
        return MT_UNF_ENC_FMT_NTSC_J;
    case MT_CODEC_ENC_FMT_NTSC_PAL_M:
        return MT_UNF_ENC_FMT_NTSC_PAL_M;

    case MT_CODEC_ENC_FMT_SECAM_SIN:
        return MT_UNF_ENC_FMT_SECAM_SIN;
    case MT_CODEC_ENC_FMT_SECAM_COS:
        return MT_UNF_ENC_FMT_SECAM_COS;

    case MT_CODEC_ENC_FMT_1080P_24_FRAME_PACKING:
        return MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING;
    case MT_CODEC_ENC_FMT_720P_60_FRAME_PACKING:
        return MT_UNF_ENC_FMT_720P_60_FRAME_PACKING;
    case MT_CODEC_ENC_FMT_720P_50_FRAME_PACKING:
        return MT_UNF_ENC_FMT_720P_50_FRAME_PACKING;

    case MT_CODEC_ENC_FMT_861D_640X480_60:
        return MT_UNF_ENC_FMT_861D_640X480_60;

    case MT_CODEC_ENC_FMT_VESA_800X600_60:
        return MT_UNF_ENC_FMT_VESA_800X600_60;
    case MT_CODEC_ENC_FMT_VESA_1024X768_60:
        return MT_UNF_ENC_FMT_VESA_1024X768_60;
    case MT_CODEC_ENC_FMT_VESA_1280X720_60:
        return MT_UNF_ENC_FMT_VESA_1280X720_60;
    case MT_CODEC_ENC_FMT_VESA_1280X800_60:
        return MT_UNF_ENC_FMT_VESA_1280X800_60;
    case MT_CODEC_ENC_FMT_VESA_1280X1024_60:
        return MT_UNF_ENC_FMT_VESA_1280X1024_60;
    case MT_CODEC_ENC_FMT_VESA_1360X768_60:
        return MT_UNF_ENC_FMT_VESA_1360X768_60;
    case MT_CODEC_ENC_FMT_VESA_1366X768_60:
        return MT_UNF_ENC_FMT_VESA_1366X768_60;
    case MT_CODEC_ENC_FMT_VESA_1400X1050_60:
        return MT_UNF_ENC_FMT_VESA_1400X1050_60;
    case MT_CODEC_ENC_FMT_VESA_1440X900_60:
        return MT_UNF_ENC_FMT_VESA_1440X900_60;
    case MT_CODEC_ENC_FMT_VESA_1440X900_60_RB:
        return MT_UNF_ENC_FMT_VESA_1440X900_60_RB;
    case MT_CODEC_ENC_FMT_VESA_1600X900_60_RB:
        return MT_UNF_ENC_FMT_VESA_1600X900_60_RB;
    case MT_CODEC_ENC_FMT_VESA_1600X1200_60:
        return MT_UNF_ENC_FMT_VESA_1600X1200_60;
    case MT_CODEC_ENC_FMT_VESA_1680X1050_60:
        return MT_UNF_ENC_FMT_VESA_1680X1050_60;
    case MT_CODEC_ENC_FMT_VESA_1920X1080_60:
        return MT_UNF_ENC_FMT_VESA_1920X1080_60;
    case MT_CODEC_ENC_FMT_VESA_1920X1200_60:
        return MT_UNF_ENC_FMT_VESA_1920X1200_60;
    case MT_CODEC_ENC_FMT_VESA_2048X1152_60:
        return MT_UNF_ENC_FMT_VESA_2048X1152_60;
     default:
        MT_WARN_VDEC("VDEC Unknow Dispfrm\n");
        return MT_UNF_ENC_FMT_BUTT;
    }
}
MT_CODEC_ENC_FMT_E VDEC_UNFDisplayFmt2CODEC(MT_UNF_ENC_FMT_E enDisplayNorm)
{
    switch(enDisplayNorm)
    {
     case MT_UNF_ENC_FMT_1080P_60:
        return MT_CODEC_ENC_FMT_1080P_60;

    case MT_UNF_ENC_FMT_1080P_50:
        return MT_CODEC_ENC_FMT_1080P_50;

    case MT_UNF_ENC_FMT_1080P_30:
        return MT_CODEC_ENC_FMT_1080P_30;

    case MT_UNF_ENC_FMT_1080P_25:
        return MT_CODEC_ENC_FMT_1080P_25;

    case MT_UNF_ENC_FMT_1080P_24:
        return MT_CODEC_ENC_FMT_1080P_24;

    case MT_UNF_ENC_FMT_1080i_60:
        return MT_CODEC_ENC_FMT_1080i_60;

    case MT_UNF_ENC_FMT_1080i_50:
           return MT_CODEC_ENC_FMT_1080i_50;

    case MT_UNF_ENC_FMT_720P_60:
        return MT_CODEC_ENC_FMT_720P_60;

    case MT_UNF_ENC_FMT_720P_50:
        return MT_CODEC_ENC_FMT_720P_50;

    case MT_UNF_ENC_FMT_576P_50:
        return MT_CODEC_ENC_FMT_576P_50;

    case MT_UNF_ENC_FMT_480P_60:
        return MT_CODEC_ENC_FMT_480P_60;

    case MT_UNF_ENC_FMT_PAL:
        return MT_CODEC_ENC_FMT_PAL;

    case MT_UNF_ENC_FMT_PAL_N:
        return MT_CODEC_ENC_FMT_PAL_N;

    case MT_UNF_ENC_FMT_PAL_Nc:
        return MT_CODEC_ENC_FMT_PAL_Nc;

    case MT_UNF_ENC_FMT_NTSC:
        return MT_CODEC_ENC_FMT_NTSC;

    case MT_UNF_ENC_FMT_NTSC_J:
        return MT_CODEC_ENC_FMT_NTSC_J;

    case MT_UNF_ENC_FMT_NTSC_PAL_M:
        return MT_CODEC_ENC_FMT_NTSC_PAL_M;


    case MT_UNF_ENC_FMT_SECAM_SIN:
        return MT_CODEC_ENC_FMT_SECAM_SIN;

    case MT_UNF_ENC_FMT_SECAM_COS:
        return MT_CODEC_ENC_FMT_SECAM_COS;


    case MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING:
        return MT_CODEC_ENC_FMT_1080P_24_FRAME_PACKING;

    case MT_UNF_ENC_FMT_720P_60_FRAME_PACKING:
        return MT_CODEC_ENC_FMT_720P_60_FRAME_PACKING;

    case MT_UNF_ENC_FMT_720P_50_FRAME_PACKING:
        return MT_CODEC_ENC_FMT_720P_50_FRAME_PACKING;

    case MT_UNF_ENC_FMT_861D_640X480_60:
        return MT_CODEC_ENC_FMT_861D_640X480_60;


    case MT_UNF_ENC_FMT_VESA_800X600_60:
        return MT_CODEC_ENC_FMT_VESA_800X600_60;

    case MT_UNF_ENC_FMT_VESA_1024X768_60:
        return MT_CODEC_ENC_FMT_VESA_1024X768_60;

    case MT_UNF_ENC_FMT_VESA_1280X720_60:
        return MT_CODEC_ENC_FMT_VESA_1280X720_60;

    case MT_UNF_ENC_FMT_VESA_1280X800_60:
        return MT_CODEC_ENC_FMT_VESA_1280X800_60;

    case MT_UNF_ENC_FMT_VESA_1280X1024_60:
        return MT_CODEC_ENC_FMT_VESA_1280X1024_60;

    case MT_UNF_ENC_FMT_VESA_1360X768_60:
        return MT_CODEC_ENC_FMT_VESA_1360X768_60;

    case MT_UNF_ENC_FMT_VESA_1366X768_60:
        return MT_CODEC_ENC_FMT_VESA_1366X768_60;

    case MT_UNF_ENC_FMT_VESA_1400X1050_60:
        return MT_CODEC_ENC_FMT_VESA_1400X1050_60;

    case MT_UNF_ENC_FMT_VESA_1440X900_60:
        return MT_CODEC_ENC_FMT_VESA_1440X900_60;

    case MT_UNF_ENC_FMT_VESA_1440X900_60_RB:
        return MT_CODEC_ENC_FMT_VESA_1440X900_60_RB;

    case MT_UNF_ENC_FMT_VESA_1600X900_60_RB:
        return MT_CODEC_ENC_FMT_VESA_1600X900_60_RB;

    case MT_UNF_ENC_FMT_VESA_1600X1200_60:
        return MT_CODEC_ENC_FMT_VESA_1600X1200_60;

    case MT_UNF_ENC_FMT_VESA_1680X1050_60:
        return MT_CODEC_ENC_FMT_VESA_1680X1050_60;

    case MT_UNF_ENC_FMT_VESA_1920X1080_60:
        return MT_CODEC_ENC_FMT_VESA_1920X1080_60;

    case MT_UNF_ENC_FMT_VESA_1920X1200_60:
        return MT_CODEC_ENC_FMT_VESA_1920X1200_60;

    case MT_UNF_ENC_FMT_VESA_2048X1152_60:
        return MT_CODEC_ENC_FMT_VESA_2048X1152_60;

     default:
        MT_WARN_VDEC("VDEC Unknow UNF Dispfrm\n");
        return MT_CODEC_ENC_FMT_BUTT;
    }
}
static mt_s32 VDEC_VFMW_CheckAttr(const MT_UNF_VCODEC_ATTR_S *pstAttr)
{
    if (MT_NULL == pstAttr)
    {
        MT_ERR_VDEC("Bad priv attr!\n");
        return MT_FAILURE;
    }

    if (pstAttr->enType >= MT_UNF_VCODEC_TYPE_BUTT)
    {
        MT_ERR_VDEC("Unsupport protocol: %d!\n", pstAttr->enType);
        return MT_FAILURE;
    }

    if (pstAttr->enMode >= MT_UNF_VCODEC_MODE_BUTT)
    {
        MT_ERR_VDEC("Unsupport mode: %d!\n", pstAttr->enMode);
        return MT_FAILURE;
    }

    if (pstAttr->u32ErrCover > 100)
    {
        MT_ERR_VDEC("Unsupport err_cover: %d!\n", pstAttr->u32ErrCover);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 VFMW_GetCap(MT_CODEC_CAP_S *pstCodecCap)
{
    mt_s32 s32Ret;
    mt_u32 i;
    mt_u32 u32Num = 0;
    VDEC_CAP_S stCap;
    MT_CODEC_SUPPORT_S* pstSupport;
    MT_CODEC_SUPPORT_S* pstTmp;

    VDEC_LOCK(s_stVdecAdpParam.stMutex);

    /* If get capability the first time */
    if (MT_NULL == s_stVdecAdpParam.stCap.pstSupport)
    {
        /* Get VFMW capability */
        s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_GETCAP, &stCap);

        if (MT_SUCCESS != s32Ret)
        {
            VDEC_UNLOCK(s_stVdecAdpParam.stMutex);
            return MT_FAILURE;
        }

        /* Calc standard number */
        while (VFMW_END_RESERVED != stCap.SupportedStd[u32Num++])
        {
        }

        /* Allocate memory for MT_CODEC_SUPPORT_S */
        pstSupport =  (MT_CODEC_SUPPORT_S*)MT_MALLOC_VDEC(sizeof(MT_CODEC_SUPPORT_S) * u32Num);
        if (MT_NULL == pstSupport)
        {
            VDEC_UNLOCK(s_stVdecAdpParam.stMutex);
            return MT_FAILURE;
        }
        pstTmp = pstSupport;
        s_stVdecAdpParam.stCap.pstSupport = pstSupport;


        /* Setup stCap.SupportedStd to s_stVdecAdpParam.stCap.pstSupport */
        for (i = 0; i < u32Num; i++)
        {
            pstSupport = pstTmp;
            pstSupport->u32Type = MT_CODEC_TYPE_DEC;
            pstSupport->enID = VDEC_VFMW_STD2CodecId(stCap.SupportedStd[i]);
            pstSupport->pstNext = pstSupport + 1;
            pstTmp = pstSupport + 1;
        }

        pstSupport->pstNext = MT_NULL;
    }

    *pstCodecCap = s_stVdecAdpParam.stCap;
    VDEC_UNLOCK(s_stVdecAdpParam.stMutex);
    return MT_SUCCESS;
}

/* Create instance */
static mt_s32 VFMW_Create(mt_handle* phInst, const MT_CODEC_OPENPARAM_S * pstParam)
{
    mt_s32 s32Ret;
    MT_UNF_AVPLAY_OPEN_OPT_S*   pstOpenParam;
    VDEC_CMD_ALLOC_S            stParam;
    VFMW_INST_S*                pstVFMWInst;
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    VDEC_CMD_USERDATABUF_S      stUsrData = {0};
    VDEC_CMD_BUF_USERADDR_S     stUserAddr = {0};
#endif

    if (MT_NULL == phInst)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = *phInst;

    /* If MT_NULL == pstParam, use default parameter */
    if (MT_NULL == pstParam)
    {
        stParam.stOpenOpt.enDecType  = MT_UNF_VCODEC_DEC_TYPE_NORMAL;
        stParam.stOpenOpt.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
        stParam.stOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_H264;
    }
    /* Else, use pstParam */
    else
    {
        /* Check parameter */
        pstOpenParam = (MT_UNF_AVPLAY_OPEN_OPT_S*)pstParam->unParam.stVdec.pPlatformPriv;
        if (MT_NULL == pstOpenParam)
        {
            MT_ERR_VDEC("Bad open param!\n");
            return MT_ERR_CODEC_INVALIDPARAM;
        }

        if (pstOpenParam->enDecType >= MT_UNF_VCODEC_DEC_TYPE_BUTT)
        {
            MT_ERR_VDEC("Bad enDecType:%d!\n", pstOpenParam->enDecType);
            return MT_ERR_CODEC_INVALIDPARAM;
        }

        if (pstOpenParam->enCapLevel >= MT_UNF_VCODEC_CAP_LEVEL_BUTT)
        {
            MT_ERR_VDEC("Bad enH264CapLevel:%d!\n", pstOpenParam->enCapLevel);
            return MT_ERR_CODEC_INVALIDPARAM;
        }

        if (pstOpenParam->enProtocolLevel >= MT_UNF_VCODEC_PRTCL_LEVEL_BUTT)
        {
            MT_ERR_VDEC("Bad enProtocolLevel:%d!\n", pstOpenParam->enProtocolLevel);
            return MT_ERR_CODEC_INVALIDPARAM;
        }

        stParam.stOpenOpt.enDecType  = pstOpenParam->enDecType;
        stParam.stOpenOpt.enCapLevel = pstOpenParam->enCapLevel;
        stParam.stOpenOpt.enProtocolLevel = pstOpenParam->enProtocolLevel;
    }

    if(NULL != pstParam)
    {
        #if (1 == MT_VDEC_DFS_SUPPORT)
        if((MT_CODEC_ID_VIDEO_H263 != pstParam->enID) && \
           (MT_CODEC_ID_VIDEO_MJPEG != pstParam->enID) && \
           (MT_CODEC_ID_VIDEO_SORENSON != pstParam->enID))//l00273086
        {
            stParam.u32DFSEnable = 1;
        }
        else
        {
            stParam.u32DFSEnable = 0;
        }
    #else
        stParam.u32DFSEnable = 0;
    #endif

    }


    /* Ioctl UMAPC_VDEC_CHAN_ALLOC */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_ALLOC, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan alloc err:%x!\n", s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    /* Output new handle */
    memcpy(phInst, &stParam, sizeof(mt_handle));
    if (MT_INVALID_HANDLE == *phInst)
    {
        MT_ERR_VDEC("hInst err!\n");
        return MT_ERR_CODEC_OPERATEFAIL;
    }

#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    stUsrData.hHandle = VFMW_INST_HANDLE(*phInst);
    stUsrData.stBuf.u32Size = MT_VDEC_USERDATA_CC_BUFSIZE;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_USERDATAINITBUF, &stUsrData);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Init user data buf fail!\n");
        goto err;
    }

    /* Map MMZ */
    stUserAddr.u32UserAddr = (ulong)mt_mem_map(stUsrData.stBuf.u32PhyAddr, stUsrData.stBuf.u32Size);
    if (MT_NULL == stUserAddr.u32UserAddr)
    {
        MT_ERR_VDEC("MT_MMZ_Map fail.\n");
        goto err;
    }

    /* Told the user virtual to kernel */
    stUserAddr.hHandle = stUsrData.hHandle;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_USERDATASETBUFADDR, &stUserAddr);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_VDEC("Set userdata addr fail.\n");
        goto err;
    }

    MT_INFO_VDEC("USERDATA MMZ %#x map to %#x, %dB\n",
        stUsrData.stBuf.u32PhyAddr, stUserAddr.u32UserAddr, stUsrData.stBuf.u32Size);
#endif

    pstVFMWInst = (VFMW_INST_S*)MT_MALLOC_VDEC(sizeof(VFMW_INST_S));
    if (MT_NULL == pstVFMWInst)
    {
        MT_ERR_VDEC("No memory!\n");
        goto err;
    }

    pstVFMWInst->hInst = (mt_handle)VFMW_INST_HANDLE((*phInst));

#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    pstVFMWInst->pu8UserDataVirAddr = (mt_u8*)stUserAddr.u32UserAddr;
#endif

    VDEC_LOCK(s_stVdecAdpParam.stMutex);
    list_add_tail(&pstVFMWInst->stInstNode, &s_stVdecAdpParam.stInstHead);
    VDEC_UNLOCK(s_stVdecAdpParam.stMutex);

    MT_INFO_VDEC("Alloc chan hInst=%d\n", *phInst);
    return MT_SUCCESS;

err:
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    if (0 != stUserAddr.u32UserAddr)
    {
        (mt_void)mt_mem_unmap((mt_void*)stUserAddr.u32UserAddr);
    }
#endif

    (mt_void)ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_FREE, &stParam);
    *phInst = MT_INVALID_HANDLE;
    return MT_ERR_CODEC_NOENOUGHRES;
}

/* Destroy instance */
static mt_s32 VFMW_Destroy(mt_handle hInst)
{
    mt_s32 s32Ret;
    mt_handle hHandle = VFMW_INST_HANDLE(hInst);
    VFMW_INST_S* pstVFMWInst = MT_NULL;

    VDEC_LOCK(s_stVdecAdpParam.stMutex);

    /* Find instance from list */
    VFMW_FIND_INST(hInst, pstVFMWInst);

    /* If find, delete it from list and free resource */
    if (pstVFMWInst)
    {
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
        if (MT_NULL != pstVFMWInst->pu8UserDataVirAddr)
        {
            (mt_void)mt_mem_unmap(pstVFMWInst->pu8UserDataVirAddr);
        }
#endif

        list_del(&pstVFMWInst->stInstNode);
        MT_FREE_VDEC(pstVFMWInst);
    }
    else
    {
		MT_ERR_VDEC("VFMW Inst NOT Found!\n");
    }

    VDEC_UNLOCK(s_stVdecAdpParam.stMutex);
    
    /* Ioctl UMAPC_VDEC_CHAN_FREE */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_FREE, &hHandle);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d destroy err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d destroy.\n", hHandle);
    return MT_SUCCESS;
}

/* Start instance */
static mt_s32 VFMW_Start(mt_handle hInst)
{
    mt_s32 s32Ret;
    mt_handle hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_START */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_START, &hHandle);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d start err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d start.\n", hHandle);
    return MT_SUCCESS;
}

/* Stop instance */
static mt_s32 VFMW_Stop(mt_handle hInst)
{
    mt_s32 s32Ret;
    mt_handle hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_STOP */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_STOP, &hHandle);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d stop err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d stop.\n", hHandle);
    return MT_SUCCESS;
}

/* Reset instance */
static mt_s32 VFMW_Reset(mt_handle hInst, const MT_CODEC_RESETPARAM_S *pstParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_RESET_S stParam;

    /* Set parameter */
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.enType = VDEC_RESET_TYPE_ALL;
    
    if(pstParam != NULL)
        stParam.resetDQ = pstParam->resetDQ;
    else
        stParam.resetDQ = MT_FALSE;

    /* Ioctl UMAPC_VDEC_CHAN_RESET */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_RESET, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d reset err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d reset.\n", stParam.hHandle);
    return MT_SUCCESS;
}

/* Set attribute */
static mt_s32 VFMW_SetAttr(mt_handle hInst, const MT_CODEC_ATTR_S * pstAttr)
{
    mt_s32 s32Ret;
    VDEC_CMD_ATTR_S stParam;
    MT_UNF_VCODEC_ATTR_S* pstPrivAttr;

    /* Check parameter */
    if (MT_NULL == pstAttr)
    {
        MT_ERR_VDEC("Bad attr!\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    pstPrivAttr = (MT_UNF_VCODEC_ATTR_S*)pstAttr->unAttr.stVdec.pPlatformPriv;
    if (MT_SUCCESS != VDEC_VFMW_CheckAttr(pstPrivAttr))
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Copy parameter */
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stAttr = *pstPrivAttr;

    /* priority will never be 0, but do not return error */
    if (0 == stParam.stAttr.u32Priority)
    {
        stParam.stAttr.u32Priority = 1;
    }

    /* Ioctl UMAPC_VDEC_CHAN_SETATTR */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETATTR, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetAttr err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d SetAttr.\n", stParam.hHandle);
    return MT_SUCCESS;
}

/* Get attribute */
static mt_s32 VFMW_GetAttr(mt_handle hInst, MT_CODEC_ATTR_S* pstAttr)
{
    mt_s32 s32Ret;
    MT_UNF_VCODEC_ATTR_S* pstPrivAttr;
    VDEC_CMD_ATTR_S stParam;

    /* Check parameter */
    if (MT_NULL == pstAttr)
    {
        MT_ERR_VDEC("Bad attr!\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_GETATTR */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_GETATTR, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d GetAttr err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    pstPrivAttr = (MT_UNF_VCODEC_ATTR_S*)pstAttr->unAttr.stVdec.pPlatformPriv;
    *pstPrivAttr = stParam.stAttr;
    pstPrivAttr->pCodecContext = MT_NULL;
    pstAttr->enID = VDEC_UNF2CodecId(stParam.stAttr.enType);
    MT_INFO_VDEC("Chan %d GetAttr.\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_GetVdecCapability(mt_handle hInst, mt_u32 *pCapability)
{
    mt_s32 s32Ret;
	VDEC_CMD_CAPABILITY_S stParam;

    if (MT_NULL == pCapability)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

	stParam.hHandle = VFMW_INST_HANDLE(hInst);
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_GET_DECODING_CAPABILITY, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("VFMW_GetVdecCapability err:%x!\n", s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    *pCapability = stParam.stCapability.u32VdecCapability;
	MT_INFO_VDEC("\r\n ~~~~w:%d,h:%d,fr:%d,ca:%d", stParam.stCapability.u32Width, stParam.stCapability.u32Height, stParam.stCapability.u32FrameRate, stParam.stCapability.u32VdecCapability);
    return MT_SUCCESS;
}

/* Get stream info */
static mt_s32 VFMW_GetStreamInfo(mt_handle hInst, MT_CODEC_STREAMINFO_S *pstStreamInfo)
{
    mt_s32 s32Ret;
    VDEC_CMD_STREAM_INFO_S stParam;

    if (MT_NULL == pstStreamInfo)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Ioctl UMAPC_VDEC_CHAN_STREAMINFO */
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_STREAMINFO, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d GetStreamInfo err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    pstStreamInfo->stVideo.enCodecID = VDEC_UNF2CodecId(stParam.stInfo.enVCodecType);
    pstStreamInfo->stVideo.enSubStandard = (MT_CODEC_VIDEO_SUB_STANDARD_E)stParam.stInfo.enSubStandard;
    pstStreamInfo->stVideo.u32SubVersion = stParam.stInfo.u32SubVersion;
    pstStreamInfo->stVideo.u32Profile = stParam.stInfo.u32Profile;
    pstStreamInfo->stVideo.u32Level = stParam.stInfo.u32Level;
    pstStreamInfo->stVideo.enDisplayNorm = VDEC_UNFDisplayFmt2CODEC(stParam.stInfo.enDisplayNorm);
    pstStreamInfo->stVideo.bProgressive = stParam.stInfo.bProgressive;
    pstStreamInfo->stVideo.u32AspectWidth = stParam.stInfo.u32AspectWidth;
    pstStreamInfo->stVideo.u32AspectHeight = stParam.stInfo.u32AspectHeight;
    pstStreamInfo->stVideo.u32bps = stParam.stInfo.u32bps;

	if(stParam.stInfo.u32fpsInteger > 100)
	{
    	pstStreamInfo->stVideo.u32FrameRateInt = stParam.stInfo.u32fpsInteger/100;
    	pstStreamInfo->stVideo.u32FrameRateDec = (stParam.stInfo.u32fpsInteger%100)*10;
	}
	else
	{
    	pstStreamInfo->stVideo.u32FrameRateInt = stParam.stInfo.u32fpsInteger;
    	pstStreamInfo->stVideo.u32FrameRateDec = stParam.stInfo.u32fpsDecimal;
	}
	
    pstStreamInfo->stVideo.u32Width  = stParam.stInfo.u32Width;
    pstStreamInfo->stVideo.u32Height = stParam.stInfo.u32Height;
    pstStreamInfo->stVideo.u32DisplayWidth   = stParam.stInfo.u32DisplayWidth;
    pstStreamInfo->stVideo.u32DisplayHeight  = stParam.stInfo.u32DisplayHeight;
    pstStreamInfo->stVideo.u32DisplayCenterX = stParam.stInfo.u32DisplayCenterX;
    pstStreamInfo->stVideo.u32DisplayCenterY = stParam.stInfo.u32DisplayCenterY;
    pstStreamInfo->stVideo.u32IsHDR = stParam.stInfo.u32IsHDR;
    MT_INFO_VDEC("Chan %d GetStreamInfo OK\n", stParam.hHandle);


    return MT_SUCCESS;
}

static mt_s32 VFMW_CheckEvt(mt_handle hInst, VDEC_EVENT_S *pstNewEvent)
{
    mt_s32 s32Ret;
    VDEC_CMD_EVENT_S stParam;

    if (MT_NULL == pstNewEvent)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Ioctl UMAPC_VDEC_CHAN_CHECKEVT */
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_CHECKEVT, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d CheckEvt err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    *pstNewEvent = stParam.stEvent;
//    MT_INFO_VDEC("Chan %d CheckEvt OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_ReadNewFrame(mt_handle hInst, MT_DRV_VIDEO_FRAME_S *pstNewFrame)
{
    mt_s32 s32Ret;
    VDEC_CMD_FRAME_S stParam;

    if (MT_NULL == pstNewFrame)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Ioctl UMAPC_VDEC_CHAN_EVNET_NEWFRAME */
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_EVNET_NEWFRAME, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
    //    MT_ERR_VDEC("Chan %d ReadNewFrame err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    *pstNewFrame = stParam.stFrame;
    MT_INFO_VDEC("Chan %d ReadNewFrame OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VDEC_SetBufferClear(mt_handle hInst, MT_BOOL *pstClearFlag)
{
    mt_s32 s32Ret;
    VDEC_CMD_BUFCLEAR_S stParam;

    if (MT_NULL == pstClearFlag)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Ioctl UMAPC_VDEC_CHAN_EVNET_NEWFRAME */
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stBuffClearFlag = *pstClearFlag;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETVOBUFCLEARFLAE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
    //    MT_ERR_VDEC("Chan %d ReadNewFrame err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    return MT_SUCCESS;
}

static mt_s32 VFMW_RecvUsrData(mt_handle hInst, MT_UNF_VIDEO_USERDATA_S *pstUsrData)
{
    mt_s32 s32Ret;
    VFMW_INST_S* pstVFMWInst = MT_NULL;
    VDEC_CMD_USERDATA_S stParam;

    if (MT_NULL == pstUsrData)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    VDEC_LOCK(s_stVdecAdpParam.stMutex);
    VFMW_FIND_INST(hInst, pstVFMWInst);
    VDEC_UNLOCK(s_stVdecAdpParam.stMutex);
    if (pstVFMWInst)
    {
        stParam.hHandle = VFMW_INST_HANDLE(hInst);
        stParam.stUserData.pu8Buffer = pstVFMWInst->au8UsrData;

        /* Ioctl UMAPC_VDEC_CHAN_USRDATA */
        s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_USRDATA, &stParam);
        if (s32Ret != MT_SUCCESS)
        {
            MT_ERR_VDEC("Chan %d RecvUsrData err:%x!\n", stParam.hHandle, s32Ret);
            return MT_ERR_CODEC_INVALIDPARAM;
        }

        *pstUsrData = stParam.stUserData;
        pstUsrData->pu8Buffer = pstVFMWInst->au8UsrData;
        MT_INFO_VDEC("Chan %d RlsFrm OK\n", stParam.hHandle);
        return MT_SUCCESS;
    }

    return MT_ERR_CODEC_INVALIDPARAM;
}

static mt_s32 VFMW_SetFrmRate(mt_handle hInst, const MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    mt_s32 s32Ret;
    VDEC_CMD_FRAME_RATE_S stParam;

    if (MT_NULL == pstFrmRate)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }
#ifdef JQW
    if ((pstFrmRate->enFrmRateType >= MT_UNF_AVPLAY_FRMRATE_TYPE_BUTT) ||
        (pstFrmRate->stSetFrmRate.u32fpsInteger > 60) ||
        (pstFrmRate->stSetFrmRate.u32fpsDecimal >= 1000))
    {
        MT_ERR_VDEC("FPS too large.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }
#else
		if ((pstFrmRate->enFrmRateType >= MT_UNF_AVPLAY_FRMRATE_TYPE_BUTT))
		{
			MT_ERR_VDEC("FPS too large.\n");
			return MT_ERR_CODEC_INVALIDPARAM;
		}

#endif

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stFrameRate = *pstFrmRate;

    /* Ioctl UMAPC_VDEC_CHAN_SETFRMRATE */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETFRMRATE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetFrmRate err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d SetFrmRate OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_SetFrmRateDeclear(mt_handle hInst, const MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    mt_s32 s32Ret;
    VDEC_CMD_FRAME_RATE_S stParam;

    if (MT_NULL == pstFrmRate)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }
#ifdef JQW
    if ((pstFrmRate->enFrmRateType >= MT_UNF_AVPLAY_FRMRATE_TYPE_BUTT) ||
        (pstFrmRate->stSetFrmRate.u32fpsInteger > 60) ||
        (pstFrmRate->stSetFrmRate.u32fpsDecimal >= 1000))
    {
        MT_ERR_VDEC("FPS too large.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }
#else
		if ((pstFrmRate->enFrmRateType >= MT_UNF_AVPLAY_FRMRATE_TYPE_BUTT))
		{
			MT_ERR_VDEC("FPS too large.\n");
			return MT_ERR_CODEC_INVALIDPARAM;
		}

#endif

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stFrameRate = *pstFrmRate;

    /* Ioctl UMAPC_VDEC_CHAN_SETFRMRATE */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETFRMRATE_DECLEAR, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetFrmRate err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d SetFrmRate OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_GetFrmRate(mt_handle hInst, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    mt_s32 s32Ret;
    VDEC_CMD_FRAME_RATE_S stParam;

    if (MT_NULL == pstFrmRate)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Ioctl UMAPC_VDEC_CHAN_GETFRMRATE */
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_GETFRMRATE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d GetFrmRate err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    *pstFrmRate = stParam.stFrameRate;
//    MT_INFO_VDEC("Chan %d GetFrmRate OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static inline mt_u32 dmx_inl(ulong port)
{
	return mpi_read_reg32((void*)port);
}

static inline void dmx_outl(ulong port , mt_u32 val)
{
	mpi_write_reg32((void*)port, val);
}

static mt_s32 VFMW_GetStatusInfo(mt_handle hInst, VDEC_STATUSINFO_S* pstStatusInfo)
{
    mt_s32 s32Ret;
    VDEC_CMD_STATUS_S stParam;

    if (MT_NULL == pstStatusInfo)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Ioctl UMAPC_VDEC_CHAN_STATUSINFO */
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_STATUSINFO, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d GetStatusInfo err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    *pstStatusInfo = stParam.stStatus;
 //   MT_INFO_VDEC("Chan %d GetStatusInfo OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_SetEosFlag(mt_handle hInst)
{
    mt_s32 s32Ret;
    ulong u32Arg;

    u32Arg = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_SETEOSFLAG */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETEOSFLAG, &u32Arg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetEosFlag err:%x!\n", u32Arg, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

//    MT_INFO_VDEC("Chan %d SetEosFlag OK\n", u32Arg);
    return MT_SUCCESS;
}

static mt_s32 VFMW_IFrameDecode(mt_handle hInst, const VFMW_IFRAME_PARAM_S* pstParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_IFRAME_DEC_S stParam;

    if ((MT_NULL == pstParam)
       || (MT_NULL == pstParam->pstIFrameStream->pu8Addr)
       || (MT_NULL == pstParam->pstVoFrameInfo))
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Check type */
    if ((pstParam->pstIFrameStream->enType >= MT_UNF_VCODEC_TYPE_BUTT))
    {
        MT_ERR_VDEC("Unsupport protocol %d!\n", pstParam->pstIFrameStream->enType);
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stIFrame = *(pstParam->pstIFrameStream);
    stParam.bCapture = pstParam->bCapture;

    /* Ioctl UMAPC_VDEC_CHAN_IFRMDECODE */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_IFRMDECODE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d IFrameDecode err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    /* Save data */
    *(pstParam->pstVoFrameInfo) = stParam.stVoFrameInfo;
    MT_INFO_VDEC("Chan %d IFrameDecode OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_IFrameRelease(mt_handle hInst, const MT_DRV_VIDEO_FRAME_S* pstParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_IFRAME_RLS_S stParam;

    if (MT_NULL == pstParam)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stVoFrameInfo = *pstParam;

    /* Ioctl UMAPC_VDEC_CHAN_IFRMRELEASE */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_IFRMRELEASE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d IFrameRelease err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d IFrameRelease OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_RecvFrm(mt_handle hInst, MT_DRV_VIDEO_FRAME_S* pstFrameInfo)
{
    mt_s32 s32Ret;
    VDEC_CMD_VO_FRAME_S stParam;

    if (MT_NULL == pstFrameInfo)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_RCVFRM, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d RecvFrm err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    *pstFrameInfo = stParam.stFrame;
    MT_INFO_VDEC("Chan %d RecvFrm OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_RlsFrm(mt_handle hInst, const MT_DRV_VIDEO_FRAME_S* pstFrameInfo)
{
    mt_s32 s32Ret;
    VDEC_CMD_VO_FRAME_S stParam;

    if (MT_NULL == pstFrameInfo)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stFrame = *pstFrameInfo;

    /* Ioctl UMAPC_VDEC_CHAN_RLSFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_RLSFRM, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d RlsFrm err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d RlsFrm OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_AttachBuf(mt_handle hInst, const VFMW_STREAMBUF_S* pstParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_ATTACH_BUF_S stParam;

    if (MT_NULL == pstParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* The parameters are: Instance handle\buffer size\demux handle\stream buffer handle */
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.u32BufSize = pstParam->u32BufSize;
    stParam.hDmxVidChn = pstParam->hDmxVidChn;
    stParam.hStrmBuf = pstParam->hStrmBuf;


    /* Ioctl UMAPC_VDEC_CHAN_ATTACHBUF */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_ATTACHBUF, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d AttachStreamBuf err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d AttachStreamBuf OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_DetachBuf(mt_handle hInst)
{
    mt_s32 s32Ret;
    ulong u32Arg;

    u32Arg = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_DETACHBUF */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_DETACHBUF, &u32Arg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d DetachStreamBuf err:%x!\n", u32Arg, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d DetachStreamBuf OK\n", u32Arg);
    return MT_SUCCESS;
}

static mt_s32 VFMW_DiscardFrame(mt_handle hInst, const VDEC_DISCARD_FRAME_S* pstParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_DISCARD_FRAME_S stParam;

    if ((MT_NULL == pstParam) || (pstParam->enMode >= VDEC_DISCARD_BUTT))
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stDiscardOpt = *pstParam;

    /* Ioctl UMAPC_VDEC_CHAN_DISCARDFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_DISCARDFRM, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d DiscardFrame err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d DiscardFrame OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
static mt_s32 VFMW_AcqUserData(mt_handle hInst, VFMW_USERDATA_S* pstParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_USERDATA_ACQMODE_S stParam;

    if (MT_NULL == pstParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_ACQUSERDATA */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_ACQUSERDATA, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_WARN_VDEC("Chan %d AcqUserData err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    *(pstParam->pstData) = stParam.stUserData;
    *(pstParam->penType) = stParam.enType;
    MT_INFO_VDEC("Chan %d AcqUserData OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_RlsUserData(mt_handle hInst, const MT_UNF_VIDEO_USERDATA_S* pstParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_USERDATA_S stParam;

    if (MT_NULL == pstParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stUserData = *pstParam;

    /* Ioctl UMAPC_VDEC_CHAN_RLSUSERDATA */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_RLSUSERDATA, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d RlsUserData err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d RlsUserData OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_ResetUserDataBuf(mt_handle hInst)
{
    mt_s32 s32Ret;
    VDEC_CMD_USERDATA_S stParam;

    stParam.hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_RLSUSERDATA */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_RSTUSERDATABUF, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d RstUserData err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d RstUserData OK\n", stParam.hHandle);
    return MT_SUCCESS;
}
#endif

static mt_s32 VFMW_DropStream(mt_handle hInst,VFMW_SEEKPTS_PARAM_S* pstVfmwSeekPts)
{
    mt_s32 s32Ret;
	VDEC_CMD_SEEK_PTS_S stParam;
	if (MT_NULL == pstVfmwSeekPts)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }
	stParam.hHandle = VFMW_INST_HANDLE(hInst);
	stParam.u32Gap = pstVfmwSeekPts->u32Gap;
	stParam.pu64SeekPts = pstVfmwSeekPts->pu64SeekPts;
    /* Ioctl UMAPC_VDEC_CHAN_SEEKPTS */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SEEKPTS, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SeekPts err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d SeekPts OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_GetInfo(mt_handle hInst, MT_UNF_AVPLAY_VDEC_INFO_S* pstParam)
{
    mt_s32 s32Ret;
    VDEC_STATUSINFO_S stStat;

    if (MT_NULL == pstParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    s32Ret = VFMW_GetStatusInfo(hInst, &stStat);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    pstParam->u32DispFrmBufNum = stStat.u32VfmwTotalDispFrmNum;
    pstParam->u32FieldFlag = stStat.u32FieldFlag;
    pstParam->stDecFrmRate = stStat.stVfmwFrameRate;
    pstParam->u32UndecFrmNum = 0;

    MT_INFO_VDEC("Chan %d GetInfo OK\n", hInst);
    return MT_SUCCESS;
}

static mt_s32 VFMW_SetTPlayOpt(mt_handle hInst, MT_UNF_AVPLAY_TPLAY_OPT_S* pstParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_TRICKMODE_OPT_S stParam;

    if ((MT_NULL == pstParam) || (pstParam->enTplayDirect >= MT_UNF_AVPLAY_TPLAY_DIRECT_BUTT))
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stTPlayOpt = *pstParam;

    /* Ioctl UMAPC_VDEC_CHAN_SETTRICKMODE */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETTRICKMODE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetTPlayOpt err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d SetTPlayOpt OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_SetCtrlInfo(mt_handle hInst, MT_UNF_AVPLAY_CONTROL_INFO_S* pstParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_SET_CTRL_INFO_S stParam;

    if (MT_NULL == pstParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.stCtrlInfo = *pstParam;

    /* Ioctl UMAPC_VDEC_CHAN_SETCTRLINFO */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETCTRLINFO, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetCtrlInfo err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d SetCtrlInfo OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

 mt_s32 VFMW_SetProgressive(mt_handle hInst, MT_BOOL* pParam)
{
    mt_s32 s32Ret;
	VDEC_CMD_SET_PROGRESSIVE_S stParam;
    if (MT_NULL == pParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
	stParam.bProgressive = *pParam;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_PROGRSSIVE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetProgressive err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }
    MT_INFO_VDEC("Chan %d SetProgressive OK\n", stParam.hHandle);
    return MT_SUCCESS;

}

mt_s32 VDEC_GetCrcBuf(mt_handle hInst, MT_UNF_AVPLAY_CRC_BUF_S* pstBuf)
{
	mt_s32 s32Ret;
	VDEC_CMD_BUF_CRC_S stParam;

	if (MT_NULL == pstBuf)
	{
		MT_ERR_VDEC("Bad param.\n");
		return MT_ERR_VDEC_INVALID_PARA;
	}

	stParam.hHandle = VFMW_INST_HANDLE(hInst);
	s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_GETCRCBUF, &stParam);

	if (MT_SUCCESS != s32Ret)
	{
        MT_ERR_VDEC("Chan %d UMAPC_VDEC_CHAN_GETCRCBUF err:%x!\n", stParam.hHandle, s32Ret);
		return s32Ret;
	}

	memset(pstBuf, 0, sizeof(MT_UNF_AVPLAY_CRC_BUF_S));
	pstBuf->u32PhyAddr = stParam.stCrcBuf.u32PhyAddr;
	pstBuf->u32Size = stParam.stCrcBuf.u32Size;

	//FIXME: should map&unmap by application.
	//pstBuf->u32UsrVirAddr = (mt_u32)mt_mmap(pstBuf->u32PhyAddr, pstBuf->u32Size);

	MT_INFO_VDEC("phy: 0x%08x, vir: 0x%08x, size: %u\n", pstBuf->u32PhyAddr, pstBuf->u32UsrVirAddr, pstBuf->u32Size);

	return s32Ret;
}

mt_s32 VDEC_SetDPBFullCtrl(mt_handle hInst, MT_BOOL* pParam)
{
    mt_s32 s32Ret;
	VDEC_CMD_SET_DPBFULL_CTRL_S stParam;
    if (MT_NULL == pParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
	stParam.bDPBFullCtrl = *pParam;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_DPBFULL, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d UMAPC_VDEC_CHAN_DPBFULL err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }
    return MT_SUCCESS;

}

mt_s32 VDEC_SetLowDelay(mt_handle hInst, MT_UNF_AVPLAY_LOW_DELAY_ATTR_S* pParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_SET_LOWDELAY_S stParam;
    if (MT_NULL == pParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
	  stParam.bLowdelay= pParam->bEnable;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_LOWDELAY, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetLowdelay err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }
    MT_INFO_VDEC("Chan %d SetLowDelay OK\n", stParam.hHandle);
    return MT_SUCCESS;

}

mt_s32 VDEC_SetDecPause(mt_handle hInst, mt_u32* pstPauseFlag)
{
    mt_s32 s32Ret;
    VDEC_CMD_SET_PAUSE_S stParam;
    if (MT_NULL == pstPauseFlag)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
	  stParam.bPauseFlag = !(!(*pstPauseFlag));
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_PAUSE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetPauseFlag err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }
    MT_INFO_VDEC("Chan %d SetPauseFlag OK PauseFlag = %d\n", stParam.hHandle, *pstPauseFlag);
    return MT_SUCCESS;

}

mt_s32 VDEC_SetColorSpace(mt_handle hInst,MT_UNF_COLOR_SPACE_E*pParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_SET_COLORSPACE_S stParam;
    if (MT_NULL == pParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
    stParam.u32ColorSpace = *pParam;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETCOLORSPACE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetColorSpace err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }
    MT_INFO_VDEC("Chan %d SetColorSpace OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_SetDecFrmType(mt_handle hInst,MT_UNF_DEC_FRM_TYPE_E* pParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_SET_DECFRMTYPE_S stParam;
    if (MT_NULL == pParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
	stParam.decFrmType = *pParam;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETDECFRMTYPE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VDEC_SetDecFrmType err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }
    MT_INFO_VDEC("Chan %d VDEC_SetDecFrmType OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_SetTrickCfg(mt_handle hInst, MT_UNF_DEC_TRICK_PARAM_S* pParam)
{
    mt_s32 s32Ret;
    VDEC_CMD_SET_TRICKCFG_S stParam;
    if (MT_NULL == pParam)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }
    stParam.hHandle = VFMW_INST_HANDLE(hInst);
	stParam.is_incomplete_stream = pParam->is_incomplete_stream;
	stParam.trick_mode = pParam->trick_mode;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETTRICKCFG, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VDEC_SetTrickCfg err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }
    MT_INFO_VDEC("Chan %d VDEC_SetTrickCfg OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

static mt_s32 VFMW_Control(mt_handle hInst, mt_u32 u32CMD, mt_void * pParam)
{
    switch (u32CMD)
    {
    case VFMW_CMD_CHECKEVT:
        return VFMW_CheckEvt(hInst, (VDEC_EVENT_S *)pParam);
    case VFMW_CMD_READNEWFRAME:
        return VFMW_ReadNewFrame(hInst, (MT_DRV_VIDEO_FRAME_S*)pParam);
    case VFMW_CMD_READUSRDATA:
        return VFMW_RecvUsrData(hInst, (MT_UNF_VIDEO_USERDATA_S*)pParam);
    case VFMW_CMD_SETFRAMERATE:
        return VFMW_SetFrmRate(hInst, (MT_UNF_AVPLAY_FRMRATE_PARAM_S*)pParam);
	case VFMW_CMD_SETFRAMERATE_DECLEAR:
        return VFMW_SetFrmRateDeclear(hInst, (MT_UNF_AVPLAY_FRMRATE_PARAM_S*)pParam);
    case VFMW_CMD_GETFRAMERATE:
        return VFMW_GetFrmRate(hInst, (MT_UNF_AVPLAY_FRMRATE_PARAM_S*)pParam);
    case VFMW_CMD_GETSTATUSINFO:
        return VFMW_GetStatusInfo(hInst, (VDEC_STATUSINFO_S*)pParam);
    case VFMW_CMD_SETEOSFLAG:
        return VFMW_SetEosFlag(hInst);
    case VFMW_CMD_IFRAMEDECODE:
        return VFMW_IFrameDecode(hInst, (VFMW_IFRAME_PARAM_S*)pParam);
    case VFMW_CMD_IFRAMERELEASE:
        return VFMW_IFrameRelease(hInst, (MT_DRV_VIDEO_FRAME_S*)pParam);
    case VFMW_CMD_RECEIVEFRAME:
        return VFMW_RecvFrm(hInst, (MT_DRV_VIDEO_FRAME_S*)pParam);
    case VFMW_CMD_RELEASEFRAME:
        return VFMW_RlsFrm(hInst, (MT_DRV_VIDEO_FRAME_S*)pParam);
    case VFMW_CMD_ATTACHBUF:
        return VFMW_AttachBuf(hInst, (VFMW_STREAMBUF_S*)pParam);
    case VFMW_CMD_DETACHBUF:
        return VFMW_DetachBuf(hInst);
    case VFMW_CMD_DISCARDFRAME:
        return VFMW_DiscardFrame(hInst, (VDEC_DISCARD_FRAME_S*)pParam);
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    case VFMW_CMD_ACQUSERDATA:
        return VFMW_AcqUserData(hInst, (VFMW_USERDATA_S*)pParam);
    case VFMW_CMD_RLSUSERDATA:
        return VFMW_RlsUserData(hInst, (MT_UNF_VIDEO_USERDATA_S*)pParam);
    case VFMW_CMD_RSTUSERDATABUF:
        return VFMW_ResetUserDataBuf(hInst);
#endif
    case VFMW_CMD_DROPSTREAM:
		    return VFMW_DropStream(hInst,(VFMW_SEEKPTS_PARAM_S*) pParam);
    case VFMW_CMD_GETINFO:
        return VFMW_GetInfo(hInst, (MT_UNF_AVPLAY_VDEC_INFO_S*)pParam);
    case VFMW_CMD_SETTPLAYOPT:
        return VFMW_SetTPlayOpt(hInst, (MT_UNF_AVPLAY_TPLAY_OPT_S*)pParam);
    case VFMW_CMD_SETCTRLINFO:
        return VFMW_SetCtrlInfo(hInst, (MT_UNF_AVPLAY_CONTROL_INFO_S*)pParam);
	  case VFMW_CMD_SET_PROGRESSIVE:
		    return VFMW_SetProgressive(hInst, (MT_BOOL*)pParam);
    case VFMW_CMD_SETLOWDELAY:
        return VDEC_SetLowDelay(hInst, (MT_UNF_AVPLAY_LOW_DELAY_ATTR_S*)pParam);
    case VFMW_CMD_SET_DPBFULL_CTRL:
		    return VDEC_SetDPBFullCtrl(hInst,(MT_BOOL*)pParam);
    case VFMW_CMD_SET_COLORSPACE:
        return VDEC_SetColorSpace(hInst,(MT_UNF_COLOR_SPACE_E*)pParam);
    case VFMW_CMD_SET_BUFCLEAR:
        return VDEC_SetBufferClear(hInst, (MT_BOOL *)pParam);
	case VFMW_CMD_GET_CAPABILITY:
		return VFMW_GetVdecCapability(hInst, (mt_u32 *)pParam);
    case VFMW_CMD_SET_DISP_HDRINFO:
        return VDEC_SetHdrInfo(hInst, (MT_UNF_VIDEO_DISP_HDR_INFO_S *)pParam);
	case VFMW_CMD_GET_CRCBUF:
		return VDEC_GetCrcBuf(hInst, (MT_UNF_AVPLAY_CRC_BUF_S* )pParam);
    case VFMW_CMD_SET_HDRINFO:
        MT_ERR_VDEC("Command VFMW_CMD_SET_HDRINFO deprecated, please use VFMW_CMD_SET_DISP_HDRINFO instead\n");
    default:
        return MT_ERR_CODEC_UNSUPPORT;
    }
}

//add by l00225186
mt_s32 VPSS_RecvFrm(mt_handle hVpss, MT_DRV_VIDEO_FRAME_PACKAGE_S* pstFrameInfo)
{
	mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam = {0};

    if (MT_NULL == pstFrameInfo)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }

    stParam.hHandle = hVpss;//VPSS_INST_HANDLE(hInst);
	stParam.pstFrame = pstFrameInfo;

    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_RCVVPSSFRM, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_INFO_VDEC("Chan %d VPSS_RecvFrm err:%x!\n", stParam.hHandle, s32Ret);
        // return MT_ERR_CODEC_OPERATEFAIL;
        return MT_FAILURE;
    }

    /* BEGIN: Deleted by z00111416, 2013/7/19 ?t??!!!!!*/
    //*pstFrameInfo = stParam.stFrame;
    /* END:   Deleted by z00111416, 2013/7/19 */
    //memcpy(pstFrameInfo, &(stParam.stFrame), sizeof(MT_DRV_VIDEO_FRAME_PACKAGE_S));
  //  MT_INFO_VDEC("Chan %d RecvVPSSFrm OK\n", stParam.hHandle);
    return s32Ret;
}

mt_s32 VPSS_ReleaseFrm(mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pVideoFrame)
{
    mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam = {0};

    if (MT_NULL == pVideoFrame)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
	stParam.hHandle = hPort;
	stParam.pVideoFrame = pVideoFrame;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_RLSPORTFRM, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_INFO_VDEC("release port %d frame err:%x!\n", stParam.hHandle, s32Ret);
        return MT_FAILURE;
    }
	return s32Ret;
}

//add by l00225186
mt_s32 VPSS_CreateVpss(mt_handle hVdec,mt_handle* phVpss)
{
   	mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == phVpss)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
	  stParam.hHandle = hVdec;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_CREATEVPSS, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("VPSS Create err:%x!\n", hVdec);
        return MT_FAILURE;
    }
    memcpy(phVpss, &stParam, sizeof(mt_handle));
	return s32Ret;
}
//add by l00225186
mt_s32 VPSS_DestoryVpss(mt_handle hVdec,mt_handle* phVpss)
{
   	mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == phVpss)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
	stParam.hHandle = hVdec;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_DESTORYVPSS, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("VPSS Destory err:%d!\n", hVdec);
        return MT_FAILURE;
    }
	return s32Ret;
}
//add by l00225186
mt_s32 VPSS_CreatePort(mt_handle hVpss, VDEC_PORT_CFG_S* psVdecPortCfg)
{
	mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == psVdecPortCfg)
    {
        MT_ERR_VDEC("Bad param.\n");
        //return MT_ERR_CODEC_INVALIDPARAM;
        return MT_ERR_VDEC_INVALID_PARA;
    }
	stParam.hHandle = hVpss;
	stParam.ePortAbility = psVdecPortCfg->ePortAbility;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_CREATEPORT, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_CreatePort err:%x!\n", hVpss, s32Ret);
        return MT_FAILURE;
    }
	*(psVdecPortCfg->phPort) = stParam.hPort;
	return s32Ret;
}
//add by l00225186
mt_s32 VPSS_DestoryPort(mt_handle hVpss, mt_handle* phPort)
{
    mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == phPort)
    {
        MT_ERR_VDEC("Bad param.\n");
        //return MT_ERR_CODEC_INVALIDPARAM;
        return MT_ERR_VDEC_INVALID_PARA;
    }
	//stParam.hHandle = VPSS_INST_HANDLE(hVdec);
	stParam.hHandle = hVpss;
	stParam.hPort  = *phPort;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_DESTROYPORT, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_DestoryPort err:%x!\n", hVpss, s32Ret);
        // return MT_ERR_CODEC_OPERATEFAIL;
        return MT_FAILURE;
    }
	return s32Ret;
}
//add by l00225186
mt_s32 VPSS_EnablePort(mt_handle hVpss, mt_handle* phPort)
{
    mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == phPort)
    {
        MT_ERR_VDEC("Bad param.\n");
        //return MT_ERR_CODEC_INVALIDPARAM;
        return MT_ERR_VDEC_INVALID_PARA;
    }
	//stParam.hHandle = VPSS_INST_HANDLE(hVdec);
	stParam.hHandle = hVpss;
	stParam.hPort  = *phPort;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_ENABLEPORT, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_EnablePort err:%x!\n", hVpss, s32Ret);
        // return MT_ERR_CODEC_OPERATEFAIL;
        return MT_FAILURE;
    }
	return s32Ret;
}
//add by l00225186
mt_s32 VPSS_DisablePort(mt_handle hVpss, mt_handle* phPort)
{
    mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == phPort)
    {
        MT_ERR_VDEC("Bad param.\n");
        //return MT_ERR_CODEC_INVALIDPARAM;
        return MT_ERR_VDEC_INVALID_PARA;
    }
	stParam.hHandle = hVpss;
	stParam.hPort  = *phPort;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_DISABLEPORT, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_DisablePort err:%x!\n", hVpss, s32Ret);
        // return MT_ERR_CODEC_OPERATEFAIL;
        return MT_FAILURE;
    }
	return s32Ret;
}
//add by l00225186
mt_s32 VPSS_SetPortType(mt_handle hVpss, VDEC_PORT_TYPE_WITHPORT_S* pstPortTypeWithPortHandle)
{
    mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == pstPortTypeWithPortHandle)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
	stParam.hHandle = hVpss;
	stParam.hPort  = pstPortTypeWithPortHandle->hPort;
	stParam.enPortType = pstPortTypeWithPortHandle->enPortType;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETPORTTYPE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_SetPortType err:%x!\n", hVpss, s32Ret);
        return MT_FAILURE;
    }
	return s32Ret;
}
//add by l00225186
mt_s32 VPSS_CancleMainPort(mt_handle hVpss, mt_handle* phPort)
{
    mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == phPort)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
	stParam.hHandle = hVpss;
	stParam.hPort  = *phPort;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_CANCLEMAINPORT, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_EnablePort err:%x!\n", hVpss, s32Ret);
        return MT_FAILURE;
    }
	return s32Ret;
}
//add by l00225186
mt_s32 VPSS_GetPortParam(mt_handle hVpss,VDEC_PORT_PARAM_WITHPORT_S *pstParam)
{
	mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == pstParam)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
	stParam.hHandle = hVpss;
	stParam.hPort = pstParam->hPort;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_GETPORTPARAM, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_CreatePort err:%x!\n", hVpss, s32Ret);
        return MT_FAILURE;
    }
	memcpy(&(pstParam->stVdecPortParam), &(stParam.stPortParam), sizeof(VDEC_PORT_PARAM_S));
	return s32Ret;
}
mt_s32 VPSS_SetChanFrmPackType(mt_handle hVpss,MT_UNF_VIDEO_FRAME_PACKING_TYPE_E* pParam)
{
    mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == pParam)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
	stParam.hHandle = hVpss;
	stParam.eFramePackType  = *pParam;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETFRMPACKTYPE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_SetChanFrmPackType err:%x!\n", hVpss, s32Ret);
        return MT_FAILURE;
    }
	return s32Ret;
}
mt_s32 VPSS_GetChanFrmPackType(mt_handle hVpss,MT_UNF_VIDEO_FRAME_PACKING_TYPE_E* pParam)
{
    mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	s32Ret = MT_SUCCESS;
	if (MT_NULL == pParam)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
	stParam.hHandle = hVpss;
    /* Ioctl UMAPC_VDEC_CHAN_RCVFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_GETFRMPACKTYPE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_GetChanFrmPackType err:%x!\n", hVpss, s32Ret);
        return MT_FAILURE;
    }
	*pParam = stParam.eFramePackType;
	return s32Ret;
}
mt_s32 VPSS_SendEos(mt_handle hVdec)
{
	mt_s32 s32Ret;
	s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SENDEOS, &hVdec);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_SendEos err:%x!\n", hVdec, s32Ret);
        return MT_FAILURE;
    }
	return s32Ret;
}

mt_s32 VPSS_GetPortState(mt_handle hVdec,MT_BOOL* bAllPortComplete)
{
	mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	stParam.hHandle = hVdec;

	s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_GETPORTSTATE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_GetPortState err:%x!\n", hVdec, s32Ret);
        return MT_FAILURE;
    }
	*bAllPortComplete = stParam.bAllPortComplete;
	return s32Ret;
}
mt_s32 VPSS_ResetVpss(mt_handle hVpss)
{
    mt_s32 s32Ret;
	s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_RESETVPSS, &hVpss);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_ResetVpss err:%x!\n", hVpss, s32Ret);
        return MT_FAILURE;
    }
	return s32Ret;
}
mt_s32 VPSS_GetStatusInfo(mt_handle hVdec,VDEC_FRMSTATUSINFOWITHPORT_S* pstVdecFrmStatusInfo)
{
	mt_s32 s32Ret;
	VDEC_CMD_VPSS_FRAME_S stParam;
	stParam.hHandle = hVdec;
	stParam.hPort = pstVdecFrmStatusInfo->hPort;
	s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_GETFRMSTATUSINFO, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPSS_GetStatusInfo err:%x!\n", hVdec, s32Ret);
        return MT_FAILURE;
    }

    memcpy(&pstVdecFrmStatusInfo->stVdecFrmStatus,&stParam.stVdecFrmStatusInfo,sizeof(VDEC_FRMSTATUSINFO_S));

    return s32Ret;
}

mt_s32 VPSS_GetPortAttr(mt_handle hVdec, VDEC_PORT_ATTR_WITHHANDLE_S *pstAttrWithHandle)
{
    mt_s32                  Ret;
    VDEC_CMD_VPSS_FRAME_S   stParam;

    memset(&stParam, 0x0, sizeof(VDEC_CMD_VPSS_FRAME_S));

    stParam.hHandle = hVdec;
    stParam.hPort = pstAttrWithHandle->hPort;

    Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_GETPORTATTR, &stParam);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_VDEC("Chan %d Get Port Attr ERR, Ret=%#x\n", hVdec, Ret);
        return Ret;
    }

    pstAttrWithHandle->stPortCfg = stParam.stPortCfg;

    return MT_SUCCESS;
}

mt_s32 VPSS_SetPortAttr(mt_handle hVdec, VDEC_PORT_ATTR_WITHHANDLE_S *pstAttrWithHandle)
{
    mt_s32                  Ret;
    VDEC_CMD_VPSS_FRAME_S   stParam;

    memset(&stParam, 0x0, sizeof(VDEC_CMD_VPSS_FRAME_S));

    stParam.hHandle = hVdec;
    stParam.hPort = pstAttrWithHandle->hPort;
    stParam.stPortCfg = pstAttrWithHandle->stPortCfg;

    Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETPORTATTR, &stParam);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_VDEC("Chan %d Set Port Attr ERR, Ret=%#x\n", hVdec, Ret);
        return Ret;
    }

    return Ret;
}

mt_s32 VPSS_SetExtBuffer(mt_handle handle,VDEC_BUFFER_ATTR_S *pstBufferAttr)
{
    mt_s32                  Ret;
    VDEC_CMD_VPSS_FRAME_S   stParam;
    stParam.hHandle = handle;
    memcpy(&stParam.stBufferAttr,pstBufferAttr,sizeof(VDEC_BUFFER_ATTR_S));
    Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETEXTBUFFER, &stParam);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_VDEC("Chan %d UMAPC_VDEC_CHAN_SETEXTBUFFER ERR, Ret=%#x\n", handle, Ret);
        return Ret;
    }
    return Ret;
}
mt_s32 VPSS_SetBufferMode(mt_handle handle,VDEC_FRAMEBUFFER_MODE_E *penFrameBufferMode)
{
    mt_s32                  Ret;
    VDEC_CMD_SET_BUFFERMODE_S   stParam;
    stParam.hHandle = handle;
    stParam.enFrameBufferMode = *penFrameBufferMode;
    Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETBUFFERMODE, &stParam);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_VDEC("Chan %d UMAPC_VDEC_CHAN_SETBUFFERMODE ERR, Ret=%#x\n", handle, Ret);
        return Ret;
    }
	return MT_SUCCESS;
}
mt_s32 VPSS_CheckAndDelBuffer(mt_handle handle,VDEC_BUFFER_INFO_S *pstBufInfo)
{
    mt_s32                  Ret;
    VDEC_CMD_CHECKANDDELBUFFER_S   stParam;
    stParam.hHandle = handle;
    memcpy(&(stParam.stBufInfo),pstBufInfo,sizeof(VDEC_BUFFER_INFO_S));
    Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_CHECKANDDELBUFFER, &stParam);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_VDEC("Chan %d UMAPC_VDEC_CHAN_CHECKANDDELBUFFER ERR, Ret=%#x\n", handle, Ret);
        return Ret;
    }
	return MT_SUCCESS;
}

mt_s32 VPSS_SetExtBufferState(mt_handle handle,VDEC_EXTBUFFER_STATE_E *pEnExtBufferState)
{
    mt_s32 Ret;
    VDEC_CMD_SETEXTBUFFERTATE_S stParam;
    stParam.hHandle = handle;
	stParam.enExtBufferState = *pEnExtBufferState;
    Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETEXTBUFFERSTATE, &stParam);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_VDEC("Chan %d UMAPC_VDEC_CHAN_SETEXTBUFFERSTATE ERR, Ret=%#x\n", handle, Ret);
        return Ret;
    }
	return MT_SUCCESS;

}

mt_s32 VPSS_SetResolution(mt_handle handle,VDEC_RESOLUTION_ATTR_S* pstResolution)
{
    mt_s32 Ret;
    VDEC_CMD_SETRESOLUTION_S stParam;
    stParam.hHandle = handle;
    memcpy(&(stParam.stResolution),pstResolution,sizeof(VDEC_RESOLUTION_ATTR_S));
    Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETRESOLUTION, &stParam);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_VDEC("Chan %d UMAPC_VDEC_CHAN_SETRESOLUTION ERR, Ret=%#x\n", handle, Ret);
        return Ret;
    }
	return MT_SUCCESS;
}
//add by l00225186
mt_s32 VPSS_Control(mt_handle handle, mt_u32 u32CMD, mt_void * pParam)
{
	switch (u32CMD)
    {
        case VPSS_CMD_CREATEVPSS:
            /*handle is hvdec*/
            return VPSS_CreateVpss(handle,(mt_handle *)pParam);
		case VPSS_CMD_DESTORYVPSS:
			return VPSS_DestoryVpss(handle,(mt_handle *)pParam);
    	case VPSS_CMD_RECEIVEFRAME:
    		return VPSS_RecvFrm(handle, (MT_DRV_VIDEO_FRAME_PACKAGE_S*)pParam);
    	case VPSS_CMD_CREATEPORT:
    		return VPSS_CreatePort(handle,(VDEC_PORT_CFG_S *)pParam);
    	case VPSS_CMD_DESTORYPORT:
    		return VPSS_DestoryPort(handle,(mt_handle *)pParam);
    	case VPSS_CMD_GETPORTPARAM:
    		return VPSS_GetPortParam(handle,(VDEC_PORT_PARAM_WITHPORT_S *)pParam);
		case VPSS_CMD_ENABLEPORT:
			return VPSS_EnablePort(handle,(mt_handle *)pParam);
		case VPSS_CMD_DISABLEPORT:
			return VPSS_DisablePort(handle,(mt_handle *)pParam);
		case VPSS_CMD_SETPORTTYPE:
			return VPSS_SetPortType(handle,(VDEC_PORT_TYPE_WITHPORT_S *)pParam);
		case VPSS_CMD_CANCLEMAINPORT:
			return VPSS_CancleMainPort(handle,(mt_handle *)pParam);
		case VPSS_CMD_SETCHAN_FRMPACKTYPE:
			return VPSS_SetChanFrmPackType(handle,(MT_UNF_VIDEO_FRAME_PACKING_TYPE_E*)pParam);
	    case VPSS_CMD_GETCHAN_FRMPACKTYPE:
			return VPSS_GetChanFrmPackType(handle,(MT_UNF_VIDEO_FRAME_PACKING_TYPE_E*)pParam);
		case VPSS_CMD_RESETVPSS:
			return VPSS_ResetVpss(handle);
		case VPSS_CMD_GETSTATUSINFO:
			/*handle is hvdec*/
			return VPSS_GetStatusInfo(handle,(VDEC_FRMSTATUSINFOWITHPORT_S *)pParam);
		case VPSS_CMD_SENDEOS:
			return VPSS_SendEos(handle);
		case VPSS_CMD_GETPORTSTATE:
			return VPSS_GetPortState(handle,(MT_BOOL*)pParam);
		case VPSS_CMD_GETPORTATTR:
		    return VPSS_GetPortAttr(handle, (VDEC_PORT_ATTR_WITHHANDLE_S *)pParam);
		case VPSS_CMD_SETPORTATTR:
		    return VPSS_SetPortAttr(handle, (VDEC_PORT_ATTR_WITHHANDLE_S *)pParam);
		case VPSS_CMD_SETEXTBUFFER:
			return VPSS_SetExtBuffer(handle,(VDEC_BUFFER_ATTR_S *)pParam);
		case VPSS_CMD_SETBUFFERMODE:
			return VPSS_SetBufferMode(handle,(VDEC_FRAMEBUFFER_MODE_E *)pParam);
		case VPSS_CMD_CHECKANDDELBUFFER:
			return VPSS_CheckAndDelBuffer(handle,(VDEC_BUFFER_INFO_S *)pParam);
		case VPSS_CMD_SETEXTBUFFERSTATE:
			return VPSS_SetExtBufferState(handle,(VDEC_EXTBUFFER_STATE_E*)pParam);
	    case VPSS_CMD_SETRESOLUTION:
			return VPSS_SetResolution(handle,(VDEC_RESOLUTION_ATTR_S*)pParam);
        default:
            return MT_ERR_CODEC_UNSUPPORT;
	}
}
MT_CODEC_S* VDEC_VFMW_Codec(mt_void)
{
    return &s_stCodec;
}

mt_s32 VDEC_AllocHandle(mt_handle *phHandle)
{
    mt_s32 s32Ret;
    mt_handle hHandle = MT_INVALID_HANDLE;

    if (MT_NULL == phHandle)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_ALLOCHANDLE, &hHandle);
    if (MT_SUCCESS == s32Ret)
    {
        *phHandle = hHandle;
    }
    return s32Ret;
}

mt_s32 VDEC_FreeHandle(mt_handle hHandle)
{
    return ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_FREEHANDLE, &hHandle);
}

mt_s32 VDEC_CreateStreamBuf(mt_handle hVdec, mt_handle* phBuf,  phys_addr_t *u32PhyAddr, mt_u32 u32BufSize, mt_u32 pip_en)
{
    mt_s32 s32Ret;
    VDEC_CMD_CREATEBUF_S stBuf;
    VDEC_CMD_BUF_USERADDR_S stUserAddr;
    STREAM_BUF_INST_S* pstBufInst = MT_NULL;

    if ((MT_NULL == phBuf) || (0 == u32BufSize))
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Alloc instance memory */
    pstBufInst = (STREAM_BUF_INST_S*)MT_MALLOC_VDEC(sizeof(STREAM_BUF_INST_S));
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_MALLOC_FAILED;
    }
    memset(pstBufInst, 0, sizeof(STREAM_BUF_INST_S));
    pstBufInst->u32Size = u32BufSize;
    pstBufInst->bGetPutFlag = MT_FALSE;
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) || (MT_VDEC_VPU_SUPPORT == 1)
    pstBufInst->bRecvRlsFlag = MT_FALSE;
#endif
    /* Create buffer manager */
    stBuf.u32Size = u32BufSize;
    stBuf.hVdec = hVdec;
    stBuf.pip_en = pip_en;
	
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CREATE_ESBUF, &stBuf);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_VDEC("UMAPC_VDEC_CREATE_ESBUF fail.\n");
        MT_FREE_VDEC(pstBufInst);
        return s32Ret;
    }

    /* Map MMZ */
    stUserAddr.u32UserAddr = (ulong)mt_mem_map(stBuf.u32PhyAddr, u32BufSize);

    if (MT_NULL == stUserAddr.u32UserAddr)
    {
        MT_ERR_VDEC("MT_MMZ_Map fail.\n");
        (mt_void)ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_DESTROY_ESBUF, &stBuf.hHandle);
        MT_FREE_VDEC(pstBufInst);
        return s32Ret;
    }
    pstBufInst->pu8MMZVirAddr = (mt_u8*)stUserAddr.u32UserAddr;

    /* Told the user virtual to kernel */
    stUserAddr.hHandle = stBuf.hHandle;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_SETUSERADDR, &stUserAddr);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_VDEC("UMAPC_VDEC_SETUSERADDR fail.\n");
        (mt_void)mt_mem_unmap(pstBufInst->pu8MMZVirAddr);
        (mt_void)ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_DESTROY_ESBUF, &stBuf.hHandle);
        MT_FREE_VDEC(pstBufInst);
        return s32Ret;
    }

    /* Save buffer manager handle */
    pstBufInst->hBuf = *phBuf = stBuf.hHandle;
	 *u32PhyAddr = stBuf.u32PhyAddr;
    /* Add instance to list */
    VDEC_LOCK(s_stStrmBufParam.stMutex);
    list_add_tail(&pstBufInst->stBufNode, &s_stStrmBufParam.stBufHead);
    VDEC_UNLOCK(s_stStrmBufParam.stMutex);

    return s32Ret;
}

mt_s32 VDEC_DestroyStreamBuf(mt_handle hBuf)
{
    mt_s32 s32Ret = MT_SUCCESS;
    STREAM_BUF_INST_S* pstBufInst = MT_NULL;

    STRMBUF_FIND_INST(hBuf, pstBufInst);
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Free buffer memory */
    if (MT_NULL != pstBufInst->pu8MMZVirAddr)
    {
        s32Ret = mt_mem_unmap(pstBufInst->pu8MMZVirAddr);
    }
    s32Ret |= ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_DESTROY_ESBUF, &hBuf);
    if (MT_SUCCESS != s32Ret)
    {
        MT_WARN_VDEC("Free memory err.\n");
    }

    /* Delete node and free memory */
	VDEC_LOCK(s_stStrmBufParam.stMutex);
    list_del(&pstBufInst->stBufNode);
    MT_FREE_VDEC(pstBufInst);
    VDEC_UNLOCK(s_stStrmBufParam.stMutex);

    return MT_SUCCESS;
}

mt_s32 VDEC_GetStreamBuf(mt_handle hBuf, mt_u32 u32RequestSize, VDEC_ES_BUF_S *pstBuf)
{
    mt_s32 s32Ret;
    STREAM_BUF_INST_S* pstBufInst = MT_NULL;
    VDEC_CMD_BUF_S stBufParam = {0};

    if (MT_NULL == pstBuf)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    STRMBUF_FIND_INST(hBuf, pstBufInst);
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* If get but not put, return last address */
    if (pstBufInst->bGetPutFlag)
    {
        memcpy(pstBuf, &(pstBufInst->stLastGet), sizeof(VDEC_ES_BUF_S));
        return MT_SUCCESS;
    }

    /* Get */
    stBufParam.hHandle = hBuf;
    stBufParam.stBuf.u32BufSize = u32RequestSize;
    stBufParam.stBuf.u32ScrapSize = pstBuf->u32ScrapSize;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_GETBUF, &stBufParam);
    if (s32Ret != MT_SUCCESS)
    {
        return s32Ret;
    }

    /* Save */
    pstBufInst->stLastGet   = *pstBuf = stBufParam.stBuf;
    pstBufInst->bGetPutFlag = MT_TRUE;

    return MT_SUCCESS;
}

mt_s32 VDEC_PutStreamBuf(mt_handle hBuf, VDEC_ES_BUF_S *pstBuf)
{
    mt_s32 s32Ret;
    STREAM_BUF_INST_S* pstBufInst = MT_NULL;
    VDEC_CMD_BUF_S stBufParam;

    if ((MT_NULL == pstBuf) || (MT_NULL == ((mt_void *)pstBuf->pu8Addr)))
    {
        MT_ERR_VDEC("Bad param!\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }

    STRMBUF_FIND_INST(hBuf, pstBufInst);
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Support put continuously */
    if (!pstBufInst->bGetPutFlag)
    {
        return MT_SUCCESS;
    }

    /* Put */
    stBufParam.hHandle = hBuf;
    stBufParam.stBuf = *pstBuf;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_PUTBUF, &stBufParam);
    pstBuf->u32ScrapSize = stBufParam.stBuf.u32ScrapSize;
    //printf("5555: %x, %x\n", stBufParam.stBuf.u32ScrapSize, pstBuf->u32ScrapSize);

    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_VDEC("Put err\n");
        return MT_FAILURE;
    }

    /* Set flag */
    pstBufInst->bGetPutFlag = MT_FALSE;
    return MT_SUCCESS;
}

mt_s32 VDEC_ResetStreamBuf(mt_handle hBuf)
{
    STREAM_BUF_INST_S* pstBufInst = MT_NULL;

    STRMBUF_FIND_INST(hBuf, pstBufInst);
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    pstBufInst->bGetPutFlag = MT_FALSE;
    memset(&(pstBufInst->stLastGet), 0, sizeof(pstBufInst->stLastGet));

#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) || (MT_VDEC_VPU_SUPPORT == 1)
    pstBufInst->bRecvRlsFlag = MT_FALSE;
    memset(&(pstBufInst->stLastRecv), 0, sizeof(pstBufInst->stLastRecv));
#endif

    return ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_RESET_ESBUF, &hBuf);
}

mt_s32 VDEC_GetStreamBufStatus(mt_handle hBuf, MT_DRV_VDEC_STREAMBUF_STATUS_S* pstStatus)
{
    mt_s32 s32Ret;
    VDEC_CMD_BUF_STATUS_S stParam;

    stParam.hHandle = hBuf;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_GET_ESBUF_STATUS, &stParam);
    if (MT_SUCCESS == s32Ret)
    {
        *pstStatus = stParam.stStatus;
    }
    return s32Ret;
}

mt_s32 VDEC_GetCiTestInfo(mt_handle hInst, MT_UNF_AVPLAY_CI_TEST_INFO_S* pstInfo)
{
	mt_s32 s32Ret;
	VDEC_CMD_CI_TEST_INFO_S stParam;

	if (MT_NULL == pstInfo)
	{
		MT_ERR_VDEC("Bad param.\n");
		return MT_ERR_CODEC_INVALIDPARAM;
	}

	stParam.hHandle = VFMW_INST_HANDLE(hInst);
	s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_GET_CI_TEST_INFO, &stParam);
	if (MT_SUCCESS != s32Ret){
		MT_ERR_VDEC("Chan %d GetCiTestInfo err:%x!\n", stParam.hHandle, s32Ret);
		return MT_ERR_CODEC_OPERATEFAIL;
	}
	*pstInfo = stParam.stInfo;
	
	MT_INFO_VDEC("Chan %d GetCiTestInfo OK\n", stParam.hHandle);
	return MT_SUCCESS;
}

#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1)
mt_s32 VDEC_RecvStream(mt_handle hBuf, VDEC_ES_BUF_S *pstBuf)
{
    mt_s32 s32Ret;
    STREAM_BUF_INST_S* pstBufInst = MT_NULL;
    VDEC_CMD_BUF_S stParam;

    if (MT_NULL == pstBuf)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    STRMBUF_FIND_INST(hBuf, pstBufInst);
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* If request but not release, return last address */
    if (MT_TRUE == pstBufInst->bRecvRlsFlag)
    {
        memcpy((mt_void *)pstBuf, (const mt_void *)&pstBufInst->stLastRecv, sizeof(VDEC_ES_BUF_S));
        return MT_SUCCESS;
    }

    /* Request */
    stParam.hHandle = hBuf;
    memset((mt_void *)&(stParam.stBuf), 0, sizeof(VDEC_ES_BUF_S));
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_RCVBUF, &stParam);
    if (s32Ret != MT_SUCCESS)
    {
        return s32Ret;
    }

    /* Save */
    pstBufInst->stLastRecv   = *pstBuf = stParam.stBuf;
    pstBufInst->bRecvRlsFlag = MT_TRUE;
    return MT_SUCCESS;
}

mt_s32 VDEC_RlsStream(mt_handle hBuf, const VDEC_ES_BUF_S *pstBuf)
{
    mt_s32 s32Ret;
    STREAM_BUF_INST_S* pstBufInst = MT_NULL;
    VDEC_CMD_BUF_S stParam;

    if ((MT_NULL == pstBuf) || (MT_NULL == ((mt_void *)pstBuf->pu8Addr)))
    {
        MT_ERR_VDEC("Bad param!\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }

    STRMBUF_FIND_INST(hBuf, pstBufInst);
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Support release continuously */
    if (!pstBufInst->bRecvRlsFlag)
    {
        return MT_SUCCESS;
    }

    /* Support release 0 */
    if (0 == pstBuf->u32BufSize)
    {
        return MT_SUCCESS;
    }

    /* Release */
    stParam.hHandle = hBuf;
    stParam.stBuf = *pstBuf;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_RLSBUF, &stParam);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_VDEC("Rls err\n");
        return MT_FAILURE;
    }

    /* Set flag */
    pstBufInst->bRecvRlsFlag = MT_FALSE;
    return MT_SUCCESS;
}

mt_s32 VDEC_CreateFrameBuf(mt_handle *phBuf)
{
    mt_s32 s32Ret;
    FRAME_BUF_INST_S* pstBufInst = MT_NULL;
    VDEC_CMD_ALLOC_S stParam;
    VDEC_CMD_ATTR_S stFrmBufAttr = {0};

    if (MT_NULL == phBuf)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Alloc instance memory */
    pstBufInst = (FRAME_BUF_INST_S*)MT_MALLOC_VDEC(sizeof(FRAME_BUF_INST_S));
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_MALLOC_FAILED;
    }
	else
	{
        memset(pstBufInst, 0, sizeof(FRAME_BUF_INST_S));
	}

    stParam.hHandle = *phBuf;
    stParam.stOpenOpt.enDecType  = MT_UNF_VCODEC_DEC_TYPE_NORMAL;
    stParam.stOpenOpt.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
    stParam.stOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_H264;
    stParam.u32DFSEnable = 0;//l00273086

    /* Ioctl UMAPC_VDEC_CHAN_ALLOC */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_ALLOC, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan alloc err:%x!\n", s32Ret);
        MT_FREE_VDEC(pstBufInst);
        return MT_ERR_VDEC_CREATECH_FAILED;
    }

    /* Output handle */
    memcpy(&stFrmBufAttr.hHandle, &stParam, sizeof(mt_handle));
    if (MT_INVALID_HANDLE == stFrmBufAttr.hHandle)
    {
        MT_ERR_VDEC("hBuf err!\n");
	    MT_FREE_VDEC(pstBufInst);
        return MT_ERR_VDEC_CREATECH_FAILED;
    }

    /* Config type MJPEG, vfmw channel will be a frame buffer */
    stFrmBufAttr.stAttr.enType = MT_UNF_VCODEC_TYPE_MJPEG;
    stFrmBufAttr.stAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
    stFrmBufAttr.stAttr.u32ErrCover = 100;
    stFrmBufAttr.stAttr.u32Priority = 15;
    stFrmBufAttr.stAttr.bOrderOutput = MT_TRUE;
    stFrmBufAttr.stAttr.s32CtrlOptions = 0;
    stFrmBufAttr.stAttr.pCodecContext = MT_NULL;

    /* Ioctl UMAPC_VDEC_CHAN_SETATTR */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETATTR, &stFrmBufAttr);
    s32Ret |= ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_START, &stFrmBufAttr.hHandle);
    if (MT_SUCCESS != s32Ret)
    {
        (mt_void)ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_FREE, &stFrmBufAttr.hHandle);
        MT_FREE_VDEC(pstBufInst);
        MT_ERR_VDEC("Chan %d SetAttr err:%x!\n", stFrmBufAttr.hHandle, s32Ret);
        return MT_ERR_VDEC_SETATTR_FAILED;
    }

    /* Save handle */
    pstBufInst->hBuf = *phBuf = stFrmBufAttr.hHandle;

    /* Add instance to list */
    VDEC_LOCK(s_stFrmBufParam.stMutex);
    list_add_tail(&pstBufInst->stBufNode, &s_stFrmBufParam.stBufHead);
    VDEC_UNLOCK(s_stFrmBufParam.stMutex);

    return MT_SUCCESS;
}

mt_s32 VDEC_DestroyFrameBuf(mt_handle hBuf)
{
    mt_s32 s32Ret;
    FRAME_BUF_INST_S* pstBufInst = MT_NULL;
    FRMBUF_FIND_INST(hBuf, pstBufInst);
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_STOP, &hBuf);
    s32Ret |= ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_FREE, &hBuf);
    if (MT_SUCCESS != s32Ret)
    {
        MT_WARN_VDEC("Free channel err.\n");
    }

    /* Delete node and free memory */
	VDEC_LOCK(s_stFrmBufParam.stMutex);
    list_del(&pstBufInst->stBufNode);
    MT_FREE_VDEC(pstBufInst);
    VDEC_UNLOCK(s_stFrmBufParam.stMutex);

    return MT_SUCCESS;
}

mt_s32 VDEC_GetFrameBuf(mt_handle hBuf, MT_DRV_VDEC_FRAME_BUF_S* pstBuf)
{
    mt_s32 s32Ret;
    FRAME_BUF_INST_S* pstBufInst = MT_NULL;
    VDEC_CMD_GET_FRAME_S stParam;

    if (MT_NULL == pstBuf)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }

    FRMBUF_FIND_INST(hBuf, pstBufInst);
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* If get but not put, return last address */
    if (pstBufInst->bGetPutFlag)
    {
        memcpy(pstBuf, &(pstBufInst->stLastGet), sizeof(MT_DRV_VDEC_FRAME_BUF_S));
        return MT_SUCCESS;
    }

    stParam.hHandle = hBuf;

    /* Ioctl UMAPC_VDEC_CHAN_GETFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_GETFRM, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        /*MT_WARN_VDEC("Frame buffer %d GET err:%x!\n", stParam.hHandle, s32Ret);*/
        return MT_FAILURE;
    }

    /* Save */
    pstBufInst->stLastGet   = *pstBuf = stParam.stFrame;
    pstBufInst->bGetPutFlag = MT_TRUE;

    MT_INFO_VDEC("Frame buffer %d GET OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_PutFrameBuf(mt_handle hBuf, const MT_DRV_VDEC_USR_FRAME_S* pstBuf)
{
    mt_s32 s32Ret;
    FRAME_BUF_INST_S* pstBufInst = MT_NULL;
    VDEC_CMD_PUT_FRAME_S stParam;

    if (MT_NULL == pstBuf)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }

    FRMBUF_FIND_INST(hBuf, pstBufInst);
    if (MT_NULL == pstBufInst)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Support put continuously */
    if (!pstBufInst->bGetPutFlag)
    {
        return MT_SUCCESS;
    }

    stParam.hHandle = hBuf;
    stParam.stFrame = *pstBuf;

    /* Ioctl UMAPC_VDEC_CHAN_PUTFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_PUTFRM, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Frame buffer %d PUT err:%x!\n", stParam.hHandle, s32Ret);
        return MT_FAILURE;
    }

    /* Save */
    pstBufInst->bGetPutFlag = MT_FALSE;

    MT_INFO_VDEC("Frame buffer %d PUT OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_ResetFrameBuf(mt_handle hBuf)
{
    mt_s32 s32Ret;
    VDEC_CMD_RESET_S stParam;

    /* Set parameter */
    stParam.hHandle = hBuf;
    stParam.enType = VDEC_RESET_TYPE_ALL;
    stParam.resetDQ = MT_FALSE;

    /* Ioctl UMAPC_VDEC_CHAN_RESET */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_STOP, &hBuf);
    s32Ret |= ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_RESET, &stParam);
    s32Ret |= ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_START, &hBuf);
    return s32Ret;
}

mt_s32 VDEC_GetFrameBufStatus(mt_handle hBuf, MT_DRV_VDEC_FRAMEBUF_STATUS_S* pstStatus)
{
    mt_s32 s32Ret;
    VDEC_CMD_STATUS_S stParam;

    if (MT_NULL == pstStatus)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Ioctl UMAPC_VDEC_CHAN_STATUSINFO */
    stParam.hHandle = hBuf;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_STATUSINFO, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d GetStatusInfo err:%x!\n", stParam.hHandle, s32Ret);
        return MT_FAILURE;
    }

    pstStatus->u32TotalDecFrameNum = stParam.stStatus.u32TotalDecFrmNum;
    pstStatus->u32FrameBufNum = stParam.stStatus.u32FrameBufNum;
	pstStatus->bAllPortCompleteFrm = stParam.stStatus.bAllPortCompleteFrm;
    return MT_SUCCESS;
}

mt_s32 VDEC_GetNewFrm(mt_handle hBuf, MT_DRV_VIDEO_FRAME_S* pstFrm)
{
    if (MT_SUCCESS != VFMW_ReadNewFrame(hBuf, pstFrm))
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 VDEC_SetFrmRate(mt_handle hBuf, const MT_UNF_AVPLAY_FRMRATE_PARAM_S* pstFrmRate)
{
    if (MT_SUCCESS != VFMW_SetFrmRate(hBuf, pstFrmRate))
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 VDEC_GetFrmRate(mt_handle hBuf, MT_UNF_AVPLAY_FRMRATE_PARAM_S* pstFrmRate)
{
    if (MT_SUCCESS != VFMW_GetFrmRate(hBuf, pstFrmRate))
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}
#endif

#if (MT_VDEC_VPU_SUPPORT == 1)
mt_s32 VDEC_VPU_CreateFrameBuf(mt_mmz_buf_s *pStreamBuf)
{
    mt_s32 s32Ret;
	VDEC_CMD_VPU_BUF_CREATE_S stParam;

    if (MT_NULL == pStreamBuf)
    {
        MT_ERR_VDEC("Invalid para in function VDEC_VPU_CreateFrameBuf!\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }

	stParam.u32Size = pStreamBuf->bufsize ;
	/* Ioctl UMAPC_VDEC_CHAN_SETATTR */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_CREATE_VPU_BUF, &stParam);
	if ( MT_SUCCESS != s32Ret)
	{
		return MT_FAILURE;
	}

    pStreamBuf->phyaddr = stParam.u32StartPhyAddr ;
	pStreamBuf->kernel_viraddr = stParam.u32StartVirAddr ;
#ifdef VPU_MEM_MAP
	pStreamBuf->user_viraddr = (mt_u8 *)MT_MEM_Map(pStreamBuf->phyaddr,pStreamBuf->bufsize);
#else
    pStreamBuf->user_viraddr = MT_NULL;
#endif
    return MT_SUCCESS;
}

mt_s32 VDEC_VPU_RevertFrameBuf(mt_u32 u32Phyaddr)
{
#if 0
    mt_s32 s32Ret;
    /* Ioctl UMAPC_VDEC_CHAN_GETFRM */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_VPU_REVERT_FRAME_BUF, &u32Phyaddr);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }
#endif	
	return MT_SUCCESS;
}

mt_s32 VDEC_VPU_CreateFrameList(mt_handle hVdec)
{
    mt_s32 s32Ret;
    VDEC_CMD_CREATE_FRAME_LIST_S stParam;

	stParam.hHandle = hVdec;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_VPU_CREATE_FRAMELIST, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("VDEC_CreateVpuFrameList %d err:%x!\n", stParam.hHandle, s32Ret);
        return MT_FAILURE;
    }

    MT_INFO_VDEC("VDEC_CreateVpuFrameList %d OK\n", stParam.hHandle);
    return MT_SUCCESS;

}

mt_s32 VDEC_VPU_ReleaseFrameList(mt_handle hVdec)
{
    mt_s32 s32Ret;
    VDEC_CMD_RELEASE_FRAME_LIST_S stParam;

	stParam.hHandle = hVdec;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_VPU_RELEASE_FRAMELIST, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("VDEC_VPU_ReleaseFrameList %d err:%x!\n", stParam.hHandle, s32Ret);
        return MT_FAILURE;
    }

    MT_INFO_VDEC("VDEC_VPU_ReleaseFrameList %d OK\n", stParam.hHandle);
    return MT_SUCCESS;

}

mt_s32 VDEC_VPU_PutFrame(mt_handle hVdec,MT_DRV_VDEC_USR_FRAME_S *pstFrame)
{
    mt_s32 s32Ret;
    VDEC_CMD_VPU_PUT_FRAME_S stParam;

	stParam.hHandle = hVdec;
	memcpy(&(stParam.stFrame),pstFrame,sizeof(MT_DRV_VDEC_USR_FRAME_S));
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_VPU_PUT_FRAME, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("VDEC_VPU_PutFrame %d err:%x!\n", stParam.hHandle, s32Ret);
        return MT_FAILURE;
    }

    MT_INFO_VDEC("VDEC_VPU_PutFrame %d OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_VPU_SetAttr(mt_handle hVdec,VDEC_VPU_ATTR_S *pstVPUAttr)
{
    mt_s32 s32Ret;
    VDEC_CMD_VPU_ATTR_S stParam;

	stParam.hHandle = hVdec;
	memcpy(&(stParam.stVPUAttr),pstVPUAttr,sizeof(VDEC_VPU_ATTR_S));
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_VPU_ATTR, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("VDEC_VPU_SetAttr %d err:%x!\n", stParam.hHandle, s32Ret);
        return MT_FAILURE;
    }
    MT_INFO_VDEC("VDEC_VPU_SetAttr %d OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_VPU_CheckRlsFrameID(mt_handle hVdec, mt_s32 *pID, mt_s32 *ps32Count)
{
    mt_s32 s32Ret;
    VDEC_CMD_VPU_CHECK_RLSFRAME_S stParam ={0};

	stParam.hHandle = hVdec;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_VPU_CHECK_RLSFRM, &stParam);

	if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("VDEC_VPU_CheckRlsFrameID %d err:%x!\n", stParam.hHandle, s32Ret);
        return MT_FAILURE;
    }

	*ps32Count = stParam.s32Count;
	memcpy(pID,stParam.as32FrameID,sizeof(mt_s32)*MT_VDEC_MAX_VPU_FRAME_NUM);
    MT_INFO_VDEC("VDEC_VPU_CheckRlsFrameID %d OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_VPU_Start(mt_handle hInst)
{
    mt_s32 s32Ret;
    mt_handle hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_START */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_VPU_START, &hHandle);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPU start err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d VPU start.\n", hHandle);
    return MT_SUCCESS;
}


mt_s32 VDEC_VPU_Stop(mt_handle hInst)
{
    mt_s32 s32Ret;
    mt_handle hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_START */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_VPU_STOP, &hHandle);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPU stop err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d VPU stop.\n", hHandle);
    return MT_SUCCESS;
}


mt_s32 VDEC_VPU_PtsAlloc(mt_handle hInst, mt_handle vpuHandle)
{
    mt_s32 s32Ret;
    VDEC_CMD_VPU_PROC_HANDLE_S stVdecVPUHandle;
    mt_handle hHandle = VFMW_INST_HANDLE(hInst);

    memset(&stVdecVPUHandle, 0x0, sizeof(VDEC_CMD_VPU_PROC_HANDLE_S));

    stVdecVPUHandle.hVdecHandle = hHandle; // TODO check oxff
    stVdecVPUHandle.hVpuHandle  = vpuHandle;

    /* Ioctl UMAPC_VDEC_CHAN_START */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_VPU_PTS_Alloc, &stVdecVPUHandle);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VDEC_VPU_PTS_Alloc err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d VDEC_VPU_PTS_Alloc.\n", hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_VPU_PtsFree(mt_handle hInst, mt_handle vpuHandle)
{
    mt_s32 s32Ret;
    VDEC_CMD_VPU_PROC_HANDLE_S stVdecVPUHandle;
    mt_handle hHandle = VFMW_INST_HANDLE(hInst);

    memset(&stVdecVPUHandle, 0, sizeof(VDEC_CMD_VPU_PROC_HANDLE_S));

    stVdecVPUHandle.hVdecHandle = hHandle; // TODO check oxff
    stVdecVPUHandle.hVpuHandle  = vpuHandle;
    /* Ioctl UMAPC_VDEC_CHAN_START */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_VPU_PTS_Free, &stVdecVPUHandle);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VDEC_VPU_PtsFree err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d VDEC_VPU_PtsFree.\n", hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_VPU_PtsStart(mt_handle hInst)
{
    mt_s32 s32Ret;
    mt_handle hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_START */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_VPU_PTS_Start, &hHandle);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VDEC_VPU_PTS_Start err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d VDEC_VPU_PTS_Start.\n", hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_VPU_PtsStop(mt_handle hInst)
{
    mt_s32 s32Ret;
    mt_handle hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_START */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_VPU_PTS_Stop, &hHandle);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VDEC_VPU_PTS_Stop err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d VDEC_VPU_PTS_Stop.\n", hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_VPU_PtsReset(mt_handle hInst)
{
    mt_s32 s32Ret;
    mt_handle hHandle = VFMW_INST_HANDLE(hInst);

    /* Ioctl UMAPC_VDEC_CHAN_START */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_VPU_PTS_Reset, &hHandle);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VDEC_VPU_PTS_Reset err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d VDEC_VPU_PTS_Reset.\n", hHandle);
    return MT_SUCCESS;
}


mt_s32 VDEC_VPU_GetFrameRateForNewFrm(mt_handle hHandle, mt_u32 *u32FrameRate)
{
    mt_s32 s32Ret;
	VDEC_CMD_VO_FRAME_S stParam;

	stParam.hHandle = hHandle;
    /* Ioctl UMAPC_VDEC_CHAN_START */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_VPU_GET_FRAME_RATE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VDEC_VPU_GetFrameRate err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

	*u32FrameRate = stParam.stFrame.u32FrameRate ;

    MT_INFO_VDEC("Chan %d VDEC_VPU_GetFrameRate.\n", hHandle);
    return MT_SUCCESS;
}



mt_s32 VPU_GetVpssStatusInfo(mt_handle hVdec, MT_BOOL *bAllPortCompleteFrm)
{
    mt_s32 s32Ret;
    VDEC_CMD_VPU_GET_VPSS_STATUSINFO_S stParam;

    memset(&stParam, 0x0, sizeof(VDEC_CMD_VPU_GET_VPSS_STATUSINFO_S));

	stParam.hHandle = hVdec & 0xFF;

    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_VPU_GET_VPSS_STATUSINFO, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPU_GetVpssStatusInfo err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

	*bAllPortCompleteFrm = stParam.bAllPortCompleteFrm;

    MT_INFO_VDEC("Chan %d VPU_GetVpssStatusInfo OK\n", stParam.hHandle);
    return MT_SUCCESS;
}


mt_s32 VPU_SetFrmRate(mt_handle hInst, const MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    mt_s32 s32Ret;
    VDEC_CMD_FRAME_RATE_S stParam;

    if (MT_NULL == pstFrmRate)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }
#ifdef JQW
    if ((pstFrmRate->enFrmRateType >= MT_UNF_AVPLAY_FRMRATE_TYPE_BUTT) ||
        (pstFrmRate->stSetFrmRate.u32fpsInteger > 60) ||
        (pstFrmRate->stSetFrmRate.u32fpsDecimal >= 1000))
    {
        MT_ERR_VDEC("FPS too large.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }
#else
	if ((pstFrmRate->enFrmRateType >= MT_UNF_AVPLAY_FRMRATE_TYPE_BUTT))
	{
		MT_ERR_VDEC("FPS too large.\n");
		return MT_ERR_CODEC_INVALIDPARAM;
	}

#endif
    stParam.hHandle = hInst & 0xFF;
    stParam.stFrameRate = *pstFrmRate;

    /* Ioctl UMAPC_VDEC_CHAN_SETFRMRATE */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETFRMRATE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPU SetFrmRate err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d VPU SetFrmRate OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

mt_s32 VPU_GetFrmRate(mt_handle hInst, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    mt_s32 s32Ret;
    VDEC_CMD_FRAME_RATE_S stParam;

    if (MT_NULL == pstFrmRate)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* Ioctl UMAPC_VDEC_CHAN_GETFRMRATE */
    stParam.hHandle = hInst & 0xFF;
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_GETFRMRATE, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPU GetFrmRate err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    *pstFrmRate = stParam.stFrameRate;
    MT_INFO_VDEC("Chan %d VPU GetFrmRate OK\n", stParam.hHandle);
    return MT_SUCCESS;
}

mt_s32 VDEC_VPU_ProcStatus(mt_handle hHandle, MT_DRV_VDEC_VPU_STATUS_S *pstVPUStatus)
{
    mt_s32 s32Ret;

    VDEC_CMD_VPU_PROC_STATUS_S stVpuProcStatus;

    if (MT_NULL == pstVPUStatus)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stVpuProcStatus.hVdecHandle = hHandle & 0xff;
    memcpy(&(stVpuProcStatus.stVPUStatus), pstVPUStatus, sizeof(MT_DRV_VDEC_VPU_STATUS_S));

    /* Ioctl UMAPC_VDEC_CHAN_START */
    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_VPU_PROC, &stVpuProcStatus);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d VPU err:%x!\n", hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d VPU.\n", hHandle);
    return MT_SUCCESS;
}
#endif

mt_s32 VDEC_SetHdrInfo(mt_handle hInst, MT_UNF_VIDEO_DISP_HDR_INFO_S *pstHdrInfo)
{
    mt_s32 s32Ret;
    VDEC_CMD_SET_HDRINFO_S stParam;

    if (MT_NULL == pstHdrInfo)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    stParam.hHandle = VFMW_INST_HANDLE(hInst);
	memcpy(&stParam.stHdrInfo, pstHdrInfo, sizeof(MT_UNF_VIDEO_DISP_HDR_INFO_S));

    s32Ret = ioctl(s_stVdecAdpParam.s32DevFd, UMAPC_VDEC_CHAN_SETHDRINFO, &stParam);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SetHdrInfo err:%x!\n", stParam.hHandle, s32Ret);
        return MT_ERR_CODEC_OPERATEFAIL;
    }

    MT_INFO_VDEC("Chan %d SetHdrInfo OK\n", stParam.hHandle);

    return MT_SUCCESS;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */
