/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/******************************************************************************
  File Name     : mt_mpi_vdec_mjpeg.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/03
  Description   :
  History       :
  1.Date        : 2015/12/03
    Author      :
    Modification: Created file

******************************************************************************/

/******************************* Include Files *******************************/

/* Sys headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <pthread.h>

/* Other headers */
#include "mt_type.h"
#include "mt_module.h"
#include "mt_module_debug.h"
#include "mt_codec.h"
#include "mt_drv_vdec_ioctl.h"
#include "mt_mpi_stat.h"


/****************************** Macro Definition *****************************/

#define MT_MJPEG_MAX_CHANNEL (MT_VDEC_MAX_INSTANCE_NEW)
#define MT_MJPEG_OUTPUT_SEMIPLANAR_444 (1)

#define MT_ERR_CODEC(fmt...) \
    MT_ERR_PRINT(MT_ID_VDEC, fmt)
#define MT_WARN_CODEC(fmt...) \
    MT_WARN_PRINT(MT_ID_VDEC, fmt)
#define MT_INFO_CODEC(fmt...) \
    MT_INFO_PRINT(MT_ID_VDEC, fmt)



#define JPEG_DEC_6b_ENABLE
//#define JPEG_DEC_CAPA_ENABLE
//#define JPEG_DEBUG_ENABLE

/************************ Static Structure Definition ************************/

typedef struct
{
    MT_BOOL bUsed;
    mt_u32 u32Width;
    mt_u32 u32Height;
}MT_MJPEG_PARAM_S;

/***************************** Global Definition *****************************/

mt_u32 g_lowDelayFrameIndex[MT_VDEC_MAX_INSTANCE_NEW];
extern mt_u32 g_lowDelayVdecHandle[MT_VDEC_MAX_INSTANCE_NEW];

/***************************** Static Definition *****************************/

mt_s32 MJPEG_GetCap(MT_CODEC_CAP_S *pstCodecCap);
mt_s32 MJPEG_Create(mt_handle* phInst, const MT_CODEC_OPENPARAM_S * pstParam);
mt_s32 MJPEG_Destroy(mt_handle hInst);
mt_s32 MJPEG_SetAttr(mt_handle hInst, const MT_CODEC_ATTR_S* pstAttr);
mt_s32 MJPEG_GetAttr(mt_handle hInst, MT_CODEC_ATTR_S* pstAttr);
mt_s32 MJPEG_DecodeFrame(mt_handle hInst, MT_CODEC_STREAM_S * pstIn, MT_CODEC_FRAME_S * pstOut);
mt_s32 MJPEG_GetStreamInfo(mt_handle hInst, MT_CODEC_STREAMINFO_S *pstStreamInfo);

static MT_MJPEG_PARAM_S s_stMjpegParam[MT_MJPEG_MAX_CHANNEL];
static pthread_mutex_t  s_stMjpegMutex = PTHREAD_MUTEX_INITIALIZER;        /* Mutex */

static MT_CODEC_SUPPORT_S s_stCodecSupport =
{
    .u32Type        = MT_CODEC_TYPE_DEC,
    .enID           = MT_CODEC_ID_VIDEO_MJPEG,
    .pstNext        = MT_NULL
};

static MT_CODEC_S mt_codec_entry =
{
    .pszName		= "MJPEG",
    .unVersion		= {.stVersion = {1, 0, 0, 0}},
    .pszDescription = "MontageLZ MJPEG codec",

    .GetCap			= MJPEG_GetCap,
    .Create			= MJPEG_Create,
    .Destroy		= MJPEG_Destroy,
    .Start			= MT_NULL,
    .Stop			= MT_NULL,
    .Reset			= MT_NULL,
    .SetAttr		= MJPEG_SetAttr,
    .GetAttr		= MJPEG_GetAttr,
    .DecodeFrame	= MJPEG_DecodeFrame,
    .EncodeFrame	= MT_NULL,
    .GetStreamInfo	= MJPEG_GetStreamInfo,
    .Control		= MT_NULL,
};

/*********************************** Code ************************************/

MT_CODEC_S* VDEC_MJPEG_Codec(mt_void)
{
    return &mt_codec_entry;
}

mt_s32 MJPEG_GetCap(MT_CODEC_CAP_S *pstCodecCap)
{
    if (MT_NULL == pstCodecCap)
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    pstCodecCap->u32CapNumber = MT_CODEC_CAP_DRIVENOUTSIDE | MT_CODEC_CAP_OUTPUT2SPECADDR;
    pstCodecCap->pstSupport = &s_stCodecSupport;
    MT_INFO_CODEC("MJPEG_GetCap success.\n");
    return MT_SUCCESS;
}

mt_s32 MJPEG_Create(mt_handle* phInst, const MT_CODEC_OPENPARAM_S * pstParam)
{
    mt_s32 i;

    VDEC_LOCK(s_stMjpegMutex);
    for (i=0; i<MT_MJPEG_MAX_CHANNEL; i++)
    {
        if (!s_stMjpegParam[i].bUsed)
        {
            s_stMjpegParam[i].u32Width = 0;
            s_stMjpegParam[i].u32Height = 0;
            s_stMjpegParam[i].bUsed = MT_TRUE;
            VDEC_UNLOCK(s_stMjpegMutex);
            *phInst = (mt_handle)i;
            MT_INFO_CODEC("MJPEG_Create success.\n");
            return MT_SUCCESS;
        }
    }
    VDEC_UNLOCK(s_stMjpegMutex);

    return MT_ERR_CODEC_NOENOUGHRES;
}

mt_s32 MJPEG_Destroy(mt_handle hInst)
{
    if (hInst < MT_MJPEG_MAX_CHANNEL)
    {
        VDEC_LOCK(s_stMjpegMutex);
        s_stMjpegParam[hInst].bUsed = MT_FALSE;
        VDEC_UNLOCK(s_stMjpegMutex);
        MT_INFO_CODEC("MJPEG_Destroy success.\n");
    }
	if(((hInst&0xff) < MT_VDEC_MAX_INSTANCE_NEW) && ((hInst&0xff) > 0))
    {
        g_lowDelayFrameIndex[(hInst&0xff)] = 0;
    }
    return MT_SUCCESS;
}

mt_s32 MJPEG_SetAttr(mt_handle hInst, const MT_CODEC_ATTR_S* pstAttr)
{
    MT_INFO_CODEC("MJPEG_SetAttr success.\n");
    return MT_SUCCESS;
}

mt_s32 MJPEG_GetAttr(mt_handle hInst, MT_CODEC_ATTR_S* pstAttr)
{
    if ((hInst >= MT_MJPEG_MAX_CHANNEL) || (MT_NULL == pstAttr))
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    pstAttr->enID = MT_CODEC_ID_VIDEO_MJPEG;
    MT_INFO_CODEC("MJPEG_GetAttr success.\n");
    return MT_SUCCESS;
}

mt_s32 MJPEG_GetStreamInfo(mt_handle hInst, MT_CODEC_STREAMINFO_S *pstStreamInfo)
{
    if ((hInst >= MT_MJPEG_MAX_CHANNEL) || (MT_NULL == pstStreamInfo))
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    pstStreamInfo->stVideo.enCodecID = MT_CODEC_ID_VIDEO_MJPEG;
    pstStreamInfo->stVideo.enSubStandard = MT_CODEC_VIDEO_SUB_STANDARD_UNKNOWN;
    pstStreamInfo->stVideo.u32SubVersion = 0;
    pstStreamInfo->stVideo.u32Profile = 0;
    pstStreamInfo->stVideo.u32Level = 0;
    pstStreamInfo->stVideo.enDisplayNorm = MT_CODEC_ENC_FMT_BUTT;
    pstStreamInfo->stVideo.bProgressive = MT_TRUE;
    pstStreamInfo->stVideo.u32AspectWidth = 0;
    pstStreamInfo->stVideo.u32AspectHeight = 2;
    pstStreamInfo->stVideo.u32bps = 0;
    pstStreamInfo->stVideo.u32FrameRateInt = 0;
    pstStreamInfo->stVideo.u32FrameRateDec = 0;
    pstStreamInfo->stVideo.u32Width = s_stMjpegParam[hInst].u32Width;
    pstStreamInfo->stVideo.u32Height = s_stMjpegParam[hInst].u32Height;
    pstStreamInfo->stVideo.u32DisplayWidth = s_stMjpegParam[hInst].u32Width;
    pstStreamInfo->stVideo.u32DisplayHeight = s_stMjpegParam[hInst].u32Height;
    pstStreamInfo->stVideo.u32DisplayCenterX = s_stMjpegParam[hInst].u32Width / 2;
    pstStreamInfo->stVideo.u32DisplayCenterY = s_stMjpegParam[hInst].u32Height / 2;

    MT_INFO_CODEC("MJPEG_GetStreamInfo success.\n");


	return MT_SUCCESS;


}

/**====================================================================================
					MJPEG解码适应配
=======================================================================================**/
#ifdef JPEG_DEC_CAPA_ENABLE
/** 性能测试 **/
/**
 **(1)localplay 测试发现有9ms降低到sms，6b性能提高了3倍
 **(2)esplay    测试发现有12ms降低到6ms，6b性能提高了2倍
 **/
#include <sys/time.h>

#define DEC_TINIT()   struct timeval tv_start, tv_end; unsigned int time_cost,line_start
#define DEC_TSTART()  gettimeofday(&tv_start, NULL);line_start = __LINE__
#define DEC_TEND()    \
gettimeofday(&tv_end, NULL); \
time_cost = ((tv_end.tv_usec - tv_start.tv_usec)/1000 + (tv_end.tv_sec - tv_start.tv_sec)*1000); \
fprintf(stderr,"=============================================================================\n"); \
fprintf(stderr,"FROM LINE: %d TO LINE: %d COST: %d ms\n",line_start, __LINE__, time_cost);         \
fprintf(stderr,"=============================================================================\n")
#endif

#ifndef JPEG_DEC_6b_ENABLE
/** 非标准的 **/

#include <unistd.h>
#include "mt_mt_type.h"
#include "mt_jpg_errcode.h"
#include "jpg_decctrl.h"

#define JPGDEC_ERR MT_FAILURE
#define JPGDEC_OK MT_SUCCESS

#define JPG_SURFACE_ALIGN     128

#define JPGHDEC_MCU_ALIGN8  8
#define JPGHDEC_MCU_ALIGN16 16

extern mt_s32  MT_JPG_Open(mt_void);

/* data structure for MemSize calculation */
typedef struct hiDECMem_INFO_S
{
    JPG_SOURCEFMT_E  SrcFmt;    /* coding format */
    mt_u32           Height;    /* original image height */
    mt_u32           Width;     /* original image width */
    mt_u32           YMemSize;  /* mem size for Y(luma) component */
    mt_u32           CMemSize;  /* mem size for Chrom components */
    mt_u32           YStride;   /* luma Stride*/
    mt_u32           CStride;   /* chrom Stride*/
    JPG_MBCOLORFMT_E MbFmt;
} JPGDECMEM_INFO_S;


mt_s32  CheckJpgFileAvail(mt_u8 *pu8Stream, mt_u32 u32StreamSize)
{
    if (u32StreamSize < 2)
    {
        return JPGDEC_ERR;
    }

    if(MT_SUCCESS != JPG_Probe(0, pu8Stream, 2))
    {
        return JPGDEC_ERR;
    }

    return JPGDEC_OK;
}

mt_void SendStream(JPG_HANDLE Handle, mt_u8 *pu8Stream, mt_s32 s32StreamSize, mt_s32 *ps32DatOfst)
{
    JPGDEC_WRITESTREAM_S StreamInfo;
    mt_u32 CopyLen = 0;
    mt_u32 FreeSize = 0;
    MT_BOOL EndFlag = MT_FALSE;
    mt_void *pAddr = MT_NULL_PTR;
    mt_s32 Ret;

    while (1)
    {
        Ret = JPG_IsNeedStream(Handle, &pAddr, &FreeSize);
		if(Ret != MT_SUCCESS)
		{
			break;
		}

        /** FreeSize=0 means the stream data is sufficient, do not need any more, so sleep some time */
        if (0 == FreeSize)
        {
            (mt_void)MT_USLEEP(1000);
            continue;
        }

        /*  try to push 'FreeSize' bytes into the mem pointed by 'pAddr', then calclulate the var 'CopyLen'
          and 'EndFlag' by the relationship between the left data size and 'FreeSize' */
        if ((mt_s32)(s32StreamSize - *ps32DatOfst) > (mt_s32)FreeSize)
        {
            CopyLen = FreeSize;
            EndFlag = MT_FALSE;
        }
        else
        {
            CopyLen = (mt_u32)(s32StreamSize - *ps32DatOfst);
            EndFlag = MT_TRUE;

        }
        memcpy(pAddr, pu8Stream + *ps32DatOfst, CopyLen);

        *ps32DatOfst += CopyLen;

        /** push stream data into the decoder */
        StreamInfo.pStreamAddr = pAddr;
        StreamInfo.StreamLen = CopyLen;
        StreamInfo.CopyLen = 0;
        StreamInfo.NeedCopyFlag = MT_FALSE;
        StreamInfo.EndFlag = EndFlag;

        Ret = JPG_SendStream(Handle, &StreamInfo);

        if (StreamInfo.CopyLen < StreamInfo.StreamLen)
        {
            *ps32DatOfst = *ps32DatOfst - (mt_s32)StreamInfo.StreamLen + (mt_s32)StreamInfo.CopyLen;
        }
        /** stop seeding stream if meet the end of file  */
        if ((MT_TRUE == EndFlag) && (StreamInfo.CopyLen >= StreamInfo.StreamLen))
        {
            return;
        }

        if (0 == StreamInfo.CopyLen)
        {
            (mt_void)MT_USLEEP(1000);
            continue;
        }
    }

    return;
}

static void GetDecMemInfo(JPGDECMEM_INFO_S *pMemSizeInfo)
{
    mt_u32 YHeightTmp = 0;
    mt_u32 CHeightTmp = 0;/*l00165842*/

    /*YStride need to be aligned to 64 bytes */
    pMemSizeInfo->YStride = (pMemSizeInfo->Width + JPG_SURFACE_ALIGN - 1)
                             & (~(JPG_SURFACE_ALIGN - 1));

    switch (pMemSizeInfo->SrcFmt)
    {
        case JPG_SOURCE_COLOR_FMT_YCBCR400:
        {
            /* height need to be aligned by multiple of MCU's height */
            YHeightTmp = (pMemSizeInfo->Height + JPGHDEC_MCU_ALIGN8 - 1)
                       & (~(JPGHDEC_MCU_ALIGN8 - 1));

            /* parameters for Chrom components should be set to 0 */
            pMemSizeInfo->CStride  = 0;
            pMemSizeInfo->CMemSize = 0;
            pMemSizeInfo->MbFmt    = JPG_MBCOLOR_FMT_JPG_YCbCr400MBP;
            break;
        }
        case JPG_SOURCE_COLOR_FMT_YCBCR420:
        {
            /* height need to be aligned by multiple of MCU's height */
            YHeightTmp = (pMemSizeInfo->Height + JPGHDEC_MCU_ALIGN16 - 1)
                       & (~(JPGHDEC_MCU_ALIGN16 - 1));
            /* height for chrom is half of the luma's height, 'Stride' is the same as luma */
            CHeightTmp = YHeightTmp >> 1;
            pMemSizeInfo->CStride = pMemSizeInfo->YStride;
            pMemSizeInfo->MbFmt = JPG_MBCOLOR_FMT_JPG_YCbCr420MBP;
            break;
        }
        case JPG_SOURCE_COLOR_FMT_YCBCR422BHP:
        {
            /* height need to be aligned by multiple of MCU's height */
            YHeightTmp = (pMemSizeInfo->Height + JPGHDEC_MCU_ALIGN8 - 1)
                         & (~(JPGHDEC_MCU_ALIGN8 - 1));

            /* chrom and luma have the same height and stride */
            CHeightTmp = YHeightTmp;
            pMemSizeInfo->CStride = pMemSizeInfo->YStride;
            pMemSizeInfo->MbFmt = JPG_MBCOLOR_FMT_JPG_YCbCr422MBHP;
            break;
        }
        case JPG_SOURCE_COLOR_FMT_YCBCR422BVP:
        {
            /* height need to be aligned by multiple of MCU's height */
            YHeightTmp = (pMemSizeInfo->Height + JPGHDEC_MCU_ALIGN16 - 1)
                         & (~(JPGHDEC_MCU_ALIGN16 - 1));

            /* height for chrom is half of the luma's, but stride is double */
            CHeightTmp = YHeightTmp >> 1;
            pMemSizeInfo->CStride = pMemSizeInfo->YStride << 1;

            pMemSizeInfo->MbFmt = JPG_MBCOLOR_FMT_JPG_YCbCr422MBVP;
            break;
        }
        default:   /*JPGHAL_ENCFMT_444:*/
        {
            /* height need to be aligned by multiple of MCU's height */
            YHeightTmp = (pMemSizeInfo->Height + JPGHDEC_MCU_ALIGN8 - 1)
                         & (~(JPGHDEC_MCU_ALIGN8 - 1));


            /* height for chrom is the same as the luma's, but stride is double */
            CHeightTmp = YHeightTmp;
            pMemSizeInfo->CStride = pMemSizeInfo->YStride << 1;
            pMemSizeInfo->MbFmt = JPG_MBCOLOR_FMT_JPG_YCbCr444MBP;
            break;
        }
    }

    /* calculate MemSize */
    pMemSizeInfo->YMemSize = YHeightTmp * pMemSizeInfo->YStride;
    pMemSizeInfo->CMemSize = CHeightTmp * pMemSizeInfo->CStride;
    return;
}

mt_s32  MJPEG_DecodeFrame(mt_handle hInst, MT_CODEC_STREAM_S * pstIn, MT_CODEC_FRAME_S * pstOut)
{
    mt_s32   s32Ret;
    JPG_HANDLE  Handle = (JPG_HANDLE)-1;
    JPG_PICINFO_S  PicInfo;
    JPG_SURFACE_S  Surface;
    JPGDECMEM_INFO_S  MemInfo;
    JPG_STATE_E  State = JPG_STATE_STOP;
    mt_u32  StateIndex;
    mt_s32  s32DatOfst = 0;
    mt_s32  cnt;
    MT_LD_Event_S  LdEvent;
    mt_u32 hVdecHandle = 0;
    if ((hInst >= MT_MJPEG_MAX_CHANNEL) || (MT_NULL == pstIn) || (MT_NULL == pstOut))
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }

    /* 1. check the validity of the jpeg file */
    s32Ret = CheckJpgFileAvail(pstIn->pu8Addr, pstIn->u32Size);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("this is not a jpeg file!\n");
        goto exit_JpgDecode;
    }

    VDEC_LOCK(s_stMjpegMutex);

	#ifdef JPEG_DEC_CAPA_ENABLE
	DEC_TINIT();
	DEC_TSTART();
	#endif

    /* 2. create jpeg decoder  */
    s32Ret = MT_JPG_Open();
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("open jpeg dev fail\n");
        goto exit_JpgDecode;
    }

    s32Ret = JPG_CreateDecoder(&Handle, JPG_IMGTYPE_NORMAL, pstIn->u32Size);
    if (MT_SUCCESS != s32Ret)
    {
        MT_WARN_VDEC("create jpeg decoder error, %#x\n", s32Ret);
        goto exit_JpgDecode;
    }

    /* 3. obtain the information of the jpeg file */
    memset(&PicInfo, 0, sizeof(JPG_PICINFO_S));
    s32Ret = JPG_GetPicInfo(Handle, &PicInfo, 0);
    SendStream(Handle, pstIn->pu8Addr, (mt_s32)pstIn->u32Size, &s32DatOfst);
    s32Ret = JPG_GetPicInfo(Handle, &PicInfo, 0);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("JPG_GetPicInfo s32Ret = %#x\n", s32Ret);
        goto exit_JpgDecode;
    }

    /* 4. create surface according to the jpeg file */
    memset(&MemInfo, 0, sizeof(MemInfo));
    MemInfo.SrcFmt = PicInfo.EncodeFormat;
    MemInfo.Width  = PicInfo.Width;
    MemInfo.Height = PicInfo.Height;
    GetDecMemInfo(&MemInfo);
    Surface.OutType = JPG_OUTTYPE_MACROBLOCK;
    Surface.SurfaceInfo.MbSurface.YPhyAddr     = pstOut->stOutputAddr.u32Phy;
    Surface.SurfaceInfo.MbSurface.YVirtAddr    = (mt_void*)pstOut->stOutputAddr.u32Vir;

    Surface.SurfaceInfo.MbSurface.YStride      = MemInfo.YStride;
    Surface.SurfaceInfo.MbSurface.CbCrStride   = MemInfo.CStride;
    Surface.SurfaceInfo.MbSurface.CbCrPhyAddr  = pstOut->stOutputAddr.u32Phy + MemInfo.YMemSize;
    Surface.SurfaceInfo.MbSurface.CbCrVirtAddr = (mt_void*)(pstOut->stOutputAddr.u32Vir + MemInfo.YMemSize);
    Surface.SurfaceInfo.MbSurface.YHeight      = MemInfo.Height;
    Surface.SurfaceInfo.MbSurface.YWidth       = MemInfo.Width;
    Surface.SurfaceInfo.MbSurface.MbFmt        = MemInfo.MbFmt;

    /* record low delay event */
	hVdecHandle = g_lowDelayVdecHandle[hInst];
	LdEvent.evt_id = EVENT_VDEC_FRM_IN;
	LdEvent.frame = g_lowDelayFrameIndex[hVdecHandle&0xff];
	LdEvent.handle = ((MT_ID_VDEC << 16) | hVdecHandle);
	(mt_void)MT_SYS_GetTimeStampMs(&(LdEvent.time));

	(mt_void)MT_MPI_STAT_NotifyLowDelayEvent(&LdEvent);
	g_lowDelayFrameIndex[hVdecHandle&0xff]++;
    /* 5. decode the jpeg file */
    s32Ret = JPG_Decode(Handle, &Surface, 0);
    SendStream(Handle, pstIn->pu8Addr, pstIn->u32Size, &s32DatOfst);
    s32Ret = JPG_GetStatus(Handle, &State, &StateIndex);
    for (cnt = 0; cnt < 500; cnt++)
    {
        mt_u8 *pBufAddr;
        mt_u32 BufSize;

        s32Ret = JPG_IsNeedStream(Handle, (mt_void**)&pBufAddr, &BufSize);
        s32Ret = JPG_GetStatus(Handle, &State, &StateIndex);
        if(State == JPG_STATE_DECODEERR || State == JPG_STATE_DECODED)
        {
            if (State == JPG_STATE_DECODEERR)
            {
                MT_WARN_VDEC("decode error!!!\n");
            }
            break;
        }
        MT_USLEEP(1000);
    }

    if (State == JPG_STATE_DECODED)
    {
        switch  (Surface.SurfaceInfo.MbSurface.MbFmt)
        {
            case JPG_MBCOLOR_FMT_JPG_YCbCr400MBP:
                pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_400;
                break;
            case JPG_MBCOLOR_FMT_JPG_YCbCr422MBHP :
                pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_422_2X1;
                break;
            case JPG_MBCOLOR_FMT_JPG_YCbCr422MBVP :
                pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_422_1X2;
                break;
            case JPG_MBCOLOR_FMT_MP1_YCbCr420MBP :
            case JPG_MBCOLOR_FMT_MP2_YCbCr420MBP :
            case JPG_MBCOLOR_FMT_MP2_YCbCr420MBI :
            case JPG_MBCOLOR_FMT_JPG_YCbCr420MBP :
                pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_420;
                break;
            case JPG_MBCOLOR_FMT_JPG_YCbCr444MBP :
                pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_444;
                break;
            default:
                pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_BUTT;
        }
        pstOut->unInfo.stVideo.u32Width = Surface.SurfaceInfo.MbSurface.YWidth;
        pstOut->unInfo.stVideo.u32Height = Surface.SurfaceInfo.MbSurface.YHeight;

        pstOut->unInfo.stVideo.u32YAddr = Surface.SurfaceInfo.MbSurface.YPhyAddr;
        pstOut->unInfo.stVideo.u32YStride = Surface.SurfaceInfo.MbSurface.YStride;

        pstOut->unInfo.stVideo.u32UAddr = Surface.SurfaceInfo.MbSurface.CbCrPhyAddr;
        pstOut->unInfo.stVideo.u32UStride = Surface.SurfaceInfo.MbSurface.CbCrStride;

        pstOut->unInfo.stVideo.u32VAddr = 0;
        pstOut->unInfo.stVideo.u32VStride = 0;


        pstOut->s64SrcPtsMs = pstOut->s64PtsMs = pstIn->s64PtsMs;
        pstOut->unInfo.stVideo.enFrameType = MT_CODEC_VIDEO_FRAME_TYPE_I;
        pstOut->unInfo.stVideo.bProgressive = MT_TRUE;
        pstOut->unInfo.stVideo.enFieldMode = MT_CODEC_VIDEO_FIELD_ALL;
        pstOut->unInfo.stVideo.bTopFieldFirst = MT_FALSE;
        pstOut->unInfo.stVideo.enFramePackingType = MT_CODEC_VIDEO_FRAME_PACKING_NONE;
        pstOut->unInfo.stVideo.u32FrameRate = 0;
        pstOut->unInfo.stVideo.u32AspectWidth = 0;
        pstOut->unInfo.stVideo.u32AspectHeight = 2;
        pstOut->unInfo.stVideo.pu8UserData = MT_NULL;
        pstOut->unInfo.stVideo.u32UserDataSize = 0;

        s_stMjpegParam[hInst].u32Width = pstOut->unInfo.stVideo.u32Width;
        s_stMjpegParam[hInst].u32Height = pstOut->unInfo.stVideo.u32Height;
    }
    else
    {
        MT_WARN_VDEC("jpeg decode error\n");
    }

    /* 6. destroy the jpeg decoder, and exit */
exit_JpgDecode:

    if (Handle != (JPG_HANDLE)-1)
    {
        s32Ret = JPG_DestroyDecoder(Handle);
        if (s32Ret != 0)
        {
            VDEC_UNLOCK(s_stMjpegMutex);
            MT_WARN_VDEC("destroy jpeg decoder error\n");
            return MT_ERR_CODEC_OPERATEFAIL;
        }
    }

    VDEC_UNLOCK(s_stMjpegMutex);

	#ifdef JPEG_DEC_CAPA_ENABLE
	DEC_TEND();
	#endif

    if (State == JPG_STATE_DECODED)
    {
        return MT_SUCCESS;
    }
    else if (MT_ERR_JPG_DEC_BUSY == s32Ret)
    {
        return MT_ERR_CODEC_BUSY;
    }
    else
    {
        return MT_ERR_CODEC_OPERATEFAIL;
    }
}



#else



/** 包含的头文件 **/
#include "mt_jpeglib.h"
#include "mt_jpeg_api.h"

/** 跳转函数变量 **/
static jmp_buf s_stJmpBuf;

/*****************************************************************************
* func			: JPEG_Error
* description	: CNcomment:解码错误的跳转函数  CNend\n
* param[in] 	: cinfo      CNcomment: 解码对象   CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
mt_void JPEG_Error (j_common_ptr cinfo)
{
    (*cinfo->err->output_message)(cinfo);
    longjmp(s_stJmpBuf, 1);
}

/*****************************************************************************
* func			: MJPEG_DecodeFrame
* description	: CNcomment:帧解码 CNend\n
* param[in] 	: hInst    CNcomment:  CNend\n
* param[in] 	: *pstIn   CNcomment:  CNend\n
* param[in] 	: *pstOut  CNcomment:  CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
mt_s32 MJPEG_DecodeFrame(mt_handle hInst, MT_CODEC_STREAM_S * pstIn, MT_CODEC_FRAME_S * pstOut)
{

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr stErrMsg;

	MT_JPEG_SURFACE_DESCRIPTION_S stSurfaceDesc;
	MT_JPEG_INFO_S stJpegInfo;

    mt_s32 s32Ret = MT_SUCCESS;
    mt_ld_event_s  LdEvent;
    mt_u32 hVdecHandle = 0;

	memset(&cinfo,0,sizeof(struct jpeg_decompress_struct));
    if ((hInst >= MT_MJPEG_MAX_CHANNEL) || (MT_NULL == pstIn) || (MT_NULL == pstOut))
    {
        return MT_ERR_CODEC_INVALIDPARAM;
    }
    /* record low delay event */
	hVdecHandle = g_lowDelayVdecHandle[hInst];
	LdEvent.evt_id = EVENT_VDEC_FRM_IN;
	LdEvent.frame = g_lowDelayFrameIndex[hVdecHandle&0xff];
	LdEvent.handle = ((MT_ID_VDEC << 16) | hVdecHandle);
	(mt_void)MT_SYS_GetTimeStampMs(&(LdEvent.time));

	(mt_void)MT_MPI_STAT_NotifyLowDelayEvent(&LdEvent);
	g_lowDelayFrameIndex[hVdecHandle&0xff]++;

	VDEC_LOCK(s_stMjpegMutex);

	#ifdef JPEG_DEC_CAPA_ENABLE
	DEC_TINIT();
	DEC_TSTART();
	#endif

	/** begin to decode **/
	cinfo.err = jpeg_std_error(&stErrMsg);
	stErrMsg.error_exit = JPEG_Error;
	if (setjmp(s_stJmpBuf))
	{
		goto DEC_ERR;
	}

	jpeg_create_decompress(&cinfo);

	MT_JPEG_SetStreamPhyMem(&cinfo,(mt_char*)pstIn->u32PhyAddr);
	jpeg_mem_src(&cinfo, (mt_uchar*)(pstIn->pu8Addr), pstIn->u32Size);
	jpeg_read_header(&cinfo, TRUE);

	stJpegInfo.bOutInfo = MT_FALSE;
	s32Ret = MT_JPEG_GetJpegInfo(&cinfo,&stJpegInfo);
	/*********************************************************************/
	cinfo.scale_num   = 1 ;
	cinfo.scale_denom = 1;
	/*********************************************************************/
	switch(stJpegInfo.enFmt)
	{
		case JPEG_FMT_YUV400:
			  cinfo.out_color_space = JCS_YUV400_SP;
			  pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_400;
			  break;
		case JPEG_FMT_YUV420:
			  cinfo.out_color_space = JCS_YUV420_SP;
			  pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_420;
			  break;
		case JPEG_FMT_YUV444:
			  cinfo.out_color_space = JCS_YUV444_SP;
			  pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_444;
			  break;
		case JPEG_FMT_YUV422_12:
			  cinfo.out_color_space = JCS_YUV422_SP_12;
			  pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_422_1X2;
			  break;
		case JPEG_FMT_YUV422_21:
			  cinfo.out_color_space = JCS_YUV422_SP_21;
			  pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_422_2X1;
			  //fprintf(stderr,"========== JCS_YUV422_SP_21\n");
			  break;
		default:
			  pstOut->unInfo.stVideo.enColorFormat = MT_CODEC_COLOR_FORMAT_YUV_BUTT;
			  goto DEC_ERR;

	}
	stJpegInfo.bOutInfo = MT_TRUE;
	s32Ret = MT_JPEG_GetJpegInfo(&cinfo,&stJpegInfo);

	memset(&stSurfaceDesc,0,sizeof(MT_JPEG_SURFACE_DESCRIPTION_S));
	stSurfaceDesc.stOutSurface.pOutPhy[0]  = (mt_char*)pstOut->stOutputAddr.u32Phy;
	stSurfaceDesc.stOutSurface.pOutPhy[1]  = (mt_char*)(pstOut->stOutputAddr.u32Phy + stJpegInfo.u32OutSize[0]);
	/** 硬件解码失败转软解使用的**/
	stSurfaceDesc.stOutSurface.pOutVir[0]  = (mt_char*)pstOut->stOutputAddr.u32Vir;
	stSurfaceDesc.stOutSurface.pOutVir[1]  = (mt_char*)(pstOut->stOutputAddr.u32Vir + stJpegInfo.u32OutSize[0]);

	stSurfaceDesc.stOutSurface.bUserPhyMem = MT_TRUE;
	stSurfaceDesc.stOutSurface.u32OutStride[0] = stJpegInfo.u32OutStride[0];
	stSurfaceDesc.stOutSurface.u32OutStride[1] = stJpegInfo.u32OutStride[1];

	s32Ret = MT_JPEG_SetOutDesc(&cinfo, &stSurfaceDesc);
	if(MT_SUCCESS != s32Ret)
	{
	   MT_ERR_VDEC("MT_JPEG_SetOutDesc failure\n");
	   goto DEC_ERR;
	}

   jpeg_start_decompress(&cinfo);

	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, NULL, 1);
	}

    jpeg_finish_decompress(&cinfo);

    jpeg_destroy_decompress(&cinfo);

	pstOut->unInfo.stVideo.u32Width   = stJpegInfo.u32Width[0];
	pstOut->unInfo.stVideo.u32Height  = stJpegInfo.u32Height[0];
	pstOut->unInfo.stVideo.u32YStride = stJpegInfo.u32OutStride[0];
	pstOut->unInfo.stVideo.u32UStride = stJpegInfo.u32OutStride[1];
	pstOut->unInfo.stVideo.u32YAddr   = pstOut->stOutputAddr.u32Phy;
	pstOut->unInfo.stVideo.u32UAddr   = pstOut->stOutputAddr.u32Phy + stJpegInfo.u32OutSize[0];
	pstOut->unInfo.stVideo.u32VAddr   = 0;
	pstOut->unInfo.stVideo.u32VStride = 0;

	/** 底下这些不动的 **/
	pstOut->s64PtsMs    = pstIn->s64PtsMs;
	pstOut->s64SrcPtsMs = pstIn->s64PtsMs;
	pstOut->unInfo.stVideo.enFrameType        = MT_CODEC_VIDEO_FRAME_TYPE_I;
	pstOut->unInfo.stVideo.bProgressive       = MT_TRUE;
	pstOut->unInfo.stVideo.enFieldMode        = MT_CODEC_VIDEO_FIELD_ALL;
	pstOut->unInfo.stVideo.bTopFieldFirst     = MT_FALSE;
	pstOut->unInfo.stVideo.enFramePackingType = MT_CODEC_VIDEO_FRAME_PACKING_NONE;
	pstOut->unInfo.stVideo.u32FrameRate    = 0;
	pstOut->unInfo.stVideo.u32AspectWidth  = 0;
	pstOut->unInfo.stVideo.u32AspectHeight = 2;
	pstOut->unInfo.stVideo.pu8UserData     = MT_NULL;
	pstOut->unInfo.stVideo.u32UserDataSize = 0;

	s_stMjpegParam[hInst].u32Width  = pstOut->unInfo.stVideo.u32Width;
	s_stMjpegParam[hInst].u32Height = pstOut->unInfo.stVideo.u32Height;

	#ifdef JPEG_DEBUG_ENABLE
	fprintf(stderr,"\n===============================================================\n");
	fprintf(stderr,"motion jpeg decode success !\n");
	fprintf(stderr,"\n===============================================================\n");
	#endif

	#ifdef JPEG_DEC_CAPA_ENABLE
	DEC_TEND();
	#endif
	//fprintf(stderr,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	VDEC_UNLOCK(s_stMjpegMutex);

    return MT_SUCCESS;

DEC_ERR:

	jpeg_destroy_decompress(&cinfo);

	VDEC_UNLOCK(s_stMjpegMutex);

	MT_ERR_VDEC("motion jpeg decode failure!\n");

	return MT_FAILURE;


}

#endif
