/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/******************************************************************************
  File Name     : mt_mpi_vdec.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/3
  Description   :
  History       :
  1.Date        : 2015/12/3
    Author      :
    Modification: Created file
  2.Date        : 2015/12/3
    Author      :
    Modification: Reconstruction

******************************************************************************/

/******************************* Include Files *******************************/

/* Sys headers */
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>

/* Unf headers */
#include "mt_unf_avplay.h"
#include "mt_video_codec.h"

/* Mpi headers */
#include "mt_mpi_mem.h"
#include "mt_error_mpi.h"
#include "mt_codec.h"
#include "mt_mpi_vdec.h"
#include "mt_mpi_demux.h"
#include "mt_mpi_vdec_adapter.h"
#include "mt_mpi_vdec_vpu.h"
#include "mt_module_debug.h"
/* Drv headers */
#include "mt_drv_vdec.h"
#include "mt_drv_video.h"
#include "list.h"
#include "mt_drv_mmz.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

/****************************** Macro Definition *****************************/
static const mt_char s_szVdecVersion[] __attribute__((used)) = "SDK_VERSION:["\
                            MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
                            __DATE__", "__TIME__"]";

#define VDEC_FIND_INST(hFindVdec, pVDECInst) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        VDEC_INST_S* pstTmp; \
        VDEC_LOCK(s_stVdecParam.stMutex); \
        if (!list_empty(&s_stVdecParam.stVdecHead)) \
        { \
            list_for_each_safe(pos, n, &s_stVdecParam.stVdecHead) \
            { \
                pstTmp = list_entry(pos, VDEC_INST_S, stVdecNode); \
                if (hFindVdec == pstTmp->hVdec) \
                { \
                    pVDECInst = pstTmp; \
                    break; \
                } \
            } \
        } \
        VDEC_UNLOCK(s_stVdecParam.stMutex); \
    }

#define VDEC_CHECK_INIT \
    VDEC_LOCK(s_stVdecParam.stMutex); \
    if (s_stVdecParam.u8InitCount == 0) \
    { \
        VDEC_UNLOCK(s_stVdecParam.stMutex); \
        return MT_ERR_VDEC_NOT_INIT; \
    } \
    VDEC_UNLOCK(s_stVdecParam.stMutex);

/************************ Static Structure Definition ************************/
//add by l00225186
typedef mt_s32 (*FN_VPSS_Control)(mt_handle hInst, mt_u32 u32CMD, mt_void * pParam);


/* The parameters of a video codec instance */
typedef struct tagVDEC_INST_S
{
    mt_handle                   hVdec;          /* Vdec handle */
    mt_handle                   hStreamBuf;     /* Stream buffer handle, if none, it will be MT_INVALID_HANDLE */
    mt_handle                   hFrameBuf;      /* Frame buffer handle, if none, it will be MT_INVALID_HANDLE */
    mt_handle                   hCodecInst;     /* Codec instance handle */
    mt_handle                   hDmxVidChn;     /* If stream from demux, this value should be set */
    MT_CODEC_S*                 pstCodec;       /* Pointer of MT_CODEC_S, can be understood as codec handle */
    MT_BOOL                     bIsVFMW;        /* For VFMW */
    MT_UNF_AVPLAY_OPEN_OPT_S    stOpenParam;    /* Codec instance parameter */
    MT_UNF_VCODEC_ATTR_S        stCurAttr;      /* Current attribute */
    
    mt_u32                      u32StrmBufSize; /* Stream buffer size */
	phys_addr_t              	u32PhyAddr;
    mt_u32                      ves_buffer_channel_id; /*ves buffer channel id*/
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) || (MT_VDEC_VPU_SUPPORT == 1)
    pthread_t                   stSoftCodec;    /* For soft codec */
    pthread_mutex_t             stMutex;        /* Mutex */
    MT_BOOL                     bThreadExist;   /* For soft codec */
    MT_BOOL                     bThreadStop;    /* For soft codec */
    mt_u32                      u32ErrStrmNum;  /* Error stream packets number */
    mt_u32                      u32ErrFrmNum;   /* Error frames number */
    MT_BOOL                     bNewFrame;      /* New frame */
    mt_u32                      u32EosFlag;     /* EOS flag: 0 Normal 1 User SetEosFlag 2 Consume stream over */
    MT_BOOL                     bFrmSizeChangeFlag; /*frame size change flag: 0:frame size not change 1:frame size change*/
    mt_u32                      u32CurrentFrmWidth; /*current frame width*/
    mt_u32                      u32CurrentFrmHeight;/*current frame height*/
    mt_u32                      u32LastFrmWidth;    /*last frame width*/
    mt_u32                      u32LastFrmHeight;   /*last frame height*/
#endif
    
    MT_BOOL                     bIsStarted;
    FN_VPSS_Control             pfnVpssControl;
    mt_handle                   hVpss;
    struct list_head            stVdecNode;     /* List node */
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
	Lcevc_priv_pipe  pLecvc_priv;
#endif	
} VDEC_INST_S;

/* Global parameters of this file */
typedef struct tagVDEC_GLOBAL_S
{
    pthread_mutex_t      stMutex;               /* Mutex */
    MT_UNF_VCODEC_ATTR_S stDefAttr;             /* Default attribute */
    struct list_head     stVdecHead;            /* List head */
    mt_u8                u8InitCount;           /* Init counter */
} VDEC_GLOBAL_S;

/***************************** Global Definition *****************************/

extern MT_CODEC_S* VDEC_MJPEG_Codec(mt_void);

mt_u32 g_lowDelayFrameIndex[MT_VDEC_MAX_INSTANCE_NEW] = {0};
mt_u32 g_lowDelayVdecHandle[MT_VDEC_MAX_INSTANCE_NEW] = {0};

/***************************** Static Definition *****************************/

static VDEC_GLOBAL_S s_stVdecParam =
{
    .stMutex           = PTHREAD_MUTEX_INITIALIZER,
    .stDefAttr         =
    {
        MT_UNF_VCODEC_TYPE_H264,
        .unExtAttr     =
        {
            .stVC1Attr = {MT_FALSE, 0}
        },
        MT_UNF_VCODEC_MODE_NORMAL,
        100,
        15,
        MT_FALSE,
        0,
        MT_NULL
    },
    .stVdecHead        = {&s_stVdecParam.stVdecHead,&s_stVdecParam.stVdecHead},
    .u8InitCount       =                         0
};

static mt_s32 VDEC_ConvertError(mt_s32 s32Err);

/*********************************** Code ************************************/

static mt_s32 VDEC_ConvertError(mt_s32 s32Err)
{
    switch (s32Err)
    {
    case MT_SUCCESS:
        return MT_SUCCESS;
    case MT_ERR_CODEC_NOENOUGHRES:
        return MT_ERR_VDEC_MALLOC_FAILED;
    case MT_ERR_CODEC_INVALIDPARAM:
        return MT_ERR_VDEC_INVALID_PARA;
    case MT_ERR_CODEC_INPUTCORRUPT:
        return MT_FAILURE;
    case MT_ERR_CODEC_NOENOUGHDATA:
        return MT_FAILURE;
    case MT_ERR_CODEC_INVALIDMODE:
        return MT_ERR_VDEC_INVALID_PARA;
    case MT_ERR_CODEC_UNSUPPORT:
        return MT_ERR_VDEC_NOT_SUPPORT;
    case MT_ERR_CODEC_VERSIONUNMATCH:
        return MT_ERR_VDEC_INVALID_PARA;
    case MT_ERR_CODEC_UNKNOWN:
        return MT_FAILURE;
    default:
        return MT_FAILURE;
    }
}

static mt_s32 VDEC_VFMWSpecCMD(mt_handle hVdec, VFMW_CMD_E enCMD, mt_void* pstParam)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    mt_s32 s32Ret;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Special VFMW command */
    if (pstVdec->pstCodec)
    {
        /* Call function */
        if ((pstVdec->bIsVFMW) && (pstVdec->pstCodec->Control))
        {
            s32Ret = pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst),
                                                enCMD, (mt_void*)pstParam);
            return VDEC_ConvertError(s32Ret);
        }
    }
    else
    {
        switch (enCMD)
        {
            /* Return success if hadn't create codec instance */
            case VFMW_CMD_CHECKEVT:
            case VFMW_CMD_READNEWFRAME:
            case VFMW_CMD_READUSRDATA:
            case VFMW_CMD_GETSTATUSINFO:
                return MT_SUCCESS;

            case VFMW_CMD_SETFRAMERATE:
			case VFMW_CMD_SETFRAMERATE_DECLEAR:
            case VFMW_CMD_GETFRAMERATE:
            case VFMW_CMD_SETEOSFLAG:
            case VFMW_CMD_DISCARDFRAME:
            case VFMW_CMD_IFRAMEDECODE:
            case VFMW_CMD_IFRAMERELEASE:
            case VFMW_CMD_RECEIVEFRAME:
            case VFMW_CMD_RELEASEFRAME:
            case VFMW_CMD_ATTACHBUF:
            case VFMW_CMD_DETACHBUF:
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
            case VFMW_CMD_ACQUSERDATA:
            case VFMW_CMD_RLSUSERDATA:
            case VFMW_CMD_RSTUSERDATABUF:
#endif
            case VFMW_CMD_DROPSTREAM:
            case VFMW_CMD_GETINFO:
            case VFMW_CMD_SETTPLAYOPT:
            case VFMW_CMD_SETCTRLINFO:
            case VFMW_CMD_SET_PROGRESSIVE:
            case VFMW_CMD_SET_BUFCLEAR:
            default:
                return MT_FAILURE;
        }
    }

    return MT_FAILURE;
}

//add by l00225186
static mt_s32 VDEC_VPSSCMD(mt_handle hVdec, VPSS_CMD_E enCMD, mt_void* pstParam)
{
    mt_s32 s32Ret;
    mt_handle hVpss;
    VDEC_INST_S* pstVdec = MT_NULL;
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    /*vpss的创建是在MT_MPI_VDEC_SetChanAttr函数中执行的,并在创建的时候将vpss句柄保存在pstVdec->hVpss中*/
    /*在vdec通道上创建port，应该是通过vdec句柄找到这个vdec对应的vpss句柄*/

    hVpss = pstVdec->hVpss;
    if (MT_INVALID_HANDLE == hVpss)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    /* Call function */
    if (pstVdec->pfnVpssControl)
    {
        s32Ret =pstVdec->pfnVpssControl(hVpss, enCMD, (mt_void*)pstParam);
        return s32Ret;
    }

    return MT_FAILURE;
}


mt_s32 MT_MPI_VDEC_Init(mt_void)
{
    mt_s32 s32Ret = MT_SUCCESS;

    VDEC_LOCK(s_stVdecParam.stMutex);

    if (0 == s_stVdecParam.u8InitCount)
    {
        s_stVdecParam.stVdecHead.next = &s_stVdecParam.stVdecHead;
        s_stVdecParam.stVdecHead.prev = &s_stVdecParam.stVdecHead;
        s32Ret  = VDEC_OpenDevFile();
        s32Ret |= MT_CODEC_Init();
#if (1 == MT_VDEC_MJPEG_SUPPORT)
        s32Ret |= MT_CODEC_Register(VDEC_MJPEG_Codec());
#endif
        s32Ret |= MT_CODEC_Register(VDEC_VFMW_Codec());
#if (1 == MT_VDEC_VPU_SUPPORT)
        s32Ret |= MT_CODEC_Register(VDEC_VPU_Codec());
#endif
    }

    s_stVdecParam.u8InitCount++;
    VDEC_UNLOCK(s_stVdecParam.stMutex);

    return s32Ret;
}

mt_s32 MT_MPI_VDEC_DeInit(mt_void)
{
    mt_s32 s32Ret = MT_SUCCESS;

    VDEC_LOCK(s_stVdecParam.stMutex);

    if (1 == s_stVdecParam.u8InitCount)
    {
        s32Ret = MT_CODEC_UnRegister(VDEC_VFMW_Codec());
#if (1 == MT_VDEC_MJPEG_SUPPORT)
        s32Ret = MT_CODEC_UnRegister(VDEC_MJPEG_Codec());
#endif
#if (1 == MT_VDEC_VPU_SUPPORT)
        s32Ret |= MT_CODEC_UnRegister(VDEC_VPU_Codec());
        s32Ret |= VDEC_CloseVPU();
#endif

        s32Ret |= MT_CODEC_DeInit();
        s32Ret |= VDEC_CloseDevFile();
        s_stVdecParam.stVdecHead.next = &s_stVdecParam.stVdecHead;
        s_stVdecParam.stVdecHead.prev = &s_stVdecParam.stVdecHead;
    }

    if (s_stVdecParam.u8InitCount > 0)
    {
        s_stVdecParam.u8InitCount--;
    }

    VDEC_UNLOCK(s_stVdecParam.stMutex);

    return s32Ret;
}

static mt_s32 VDEC_CreateCodec(VDEC_INST_S* pstVdec, MT_CODEC_ID_E enID)
{
    mt_s32 s32Ret;
    MT_CODEC_OPENPARAM_S stOpenParam;
    VFMW_STREAMBUF_S stStrmBuf;
    const mt_char* CodecName = MT_NULL;

    stOpenParam.enType = MT_CODEC_TYPE_DEC;
    stOpenParam.enID = enID;
    stOpenParam.unParam.stVdec.pPlatformPriv = &(pstVdec->stOpenParam);

    /* For VFMW, pass vdec handle to codec */
    pstVdec->hCodecInst = pstVdec->hVdec;

    /* Create codec instance */
    pstVdec->pstCodec = MT_CODEC_Create(&pstVdec->hCodecInst, &stOpenParam);
    if ((MT_NULL == pstVdec->pstCodec) || (MT_INVALID_HANDLE == pstVdec->hCodecInst))
    {
        pstVdec->hCodecInst = MT_INVALID_HANDLE;
        return MT_ERR_VDEC_SETATTR_FAILED;
    }

    /*
     * Need thread or not?
     * VFMW doesn't need thread.
     */
    CodecName = MT_CODEC_GetName(pstVdec->hCodecInst);
    if (MT_NULL == CodecName)
    {
        MT_ERR_VDEC("Can't find codec name!\n");
        (mt_void)MT_CODEC_Destory(pstVdec->hCodecInst);
        return MT_ERR_VDEC_SETATTR_FAILED;
    }

    if (0 != strncmp("VFMW", CodecName, 4))
    {
        pstVdec->bIsVFMW = MT_FALSE;
#if (MT_VDEC_VPU_SUPPORT == 1)
        if (0 == strncmp("VPU", CodecName, 3))
        {
            s32Ret = pthread_mutex_init(&pstVdec->stMutex, MT_NULL);
            if (MT_SUCCESS != s32Ret)
            {
                (mt_void)MT_CODEC_Destory(pstVdec->hCodecInst);
                (mt_void)pthread_mutex_destroy(&pstVdec->stMutex);
                return MT_ERR_VDEC_SETATTR_FAILED;
            }
            return MT_SUCCESS;
        }
#endif

#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1)
        s32Ret = pthread_mutex_init(&pstVdec->stMutex, MT_NULL);
        if (MT_SUCCESS != s32Ret)
        {
            (mt_void)MT_CODEC_Destory(pstVdec->hCodecInst);
            return MT_ERR_VDEC_SETATTR_FAILED;
        }

        /* Create frame buffer */
        if (MT_CODEC_NeedFrameBuf(pstVdec->hCodecInst))
        {
            /* For VFMW frame buffer, pass vdec handle */
            pstVdec->hFrameBuf = pstVdec->hVdec;
            s32Ret = VDEC_CreateFrameBuf(&(pstVdec->hFrameBuf));
            if (MT_SUCCESS != s32Ret)
            {
                (mt_void)pthread_mutex_destroy(&pstVdec->stMutex);
                (mt_void)MT_CODEC_Destory(pstVdec->hCodecInst);
                return MT_ERR_VDEC_SETATTR_FAILED;
            }
        }
#endif
    }
    else
    {
        pstVdec->bIsVFMW = MT_TRUE;

        /* If it's VFMW and stream buffer had been created, need attach to instance */
        if ((MT_INVALID_HANDLE != pstVdec->hDmxVidChn) || (MT_INVALID_HANDLE != pstVdec->hStreamBuf))
        {
            stStrmBuf.u32BufSize = pstVdec->u32StrmBufSize;
            stStrmBuf.hDmxVidChn = pstVdec->hDmxVidChn;
            stStrmBuf.hStrmBuf = pstVdec->hStreamBuf;

            s32Ret = VDEC_VFMWSpecCMD(pstVdec->hVdec, VFMW_CMD_ATTACHBUF, (mt_void*)&stStrmBuf);
            if (MT_SUCCESS != s32Ret)
            {
                s32Ret = MT_CODEC_Destory(pstVdec->hCodecInst);
                return MT_ERR_VDEC_SETATTR_FAILED;
            }
        }
    }
    return MT_SUCCESS;
}

static mt_s32 VDEC_DestroyCodec(VDEC_INST_S* pstVdec)
{
    mt_s32 s32Ret = MT_SUCCESS;

    /*
     * Don't destroy stream buffer here.
     * User calls MT_MPI_VDEC_ChanBufferInit create stream buffer, so they should
     *  call MT_MPI_VDEC_ChanBufferDeInit to destroy it themselves.
     */

    /* Free codec instance */
    if (MT_INVALID_HANDLE != pstVdec->hCodecInst)
    {
        s32Ret = MT_CODEC_Destory(pstVdec->hCodecInst);
        if (MT_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        pstVdec->pstCodec = MT_NULL;
    }

#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) ||(MT_VDEC_VPU_SUPPORT == 1)
    /* Free frame buffer */
    if (MT_INVALID_HANDLE != pstVdec->hFrameBuf)
    {
        s32Ret = VDEC_DestroyFrameBuf(pstVdec->hFrameBuf);
        pstVdec->hFrameBuf = MT_INVALID_HANDLE;
    }

    (mt_void)pthread_mutex_destroy(&pstVdec->stMutex);
#endif

    return s32Ret;
}

static mt_s32 VDEC_ChanStop(VDEC_INST_S* pstVdec)
{
    mt_s32 s32Ret;

#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) ||(MT_VDEC_VPU_SUPPORT == 1)
    /* Soft codec */
    if (pstVdec->bThreadExist)
    {
        pstVdec->bThreadStop = MT_TRUE;
        (mt_void)pthread_join(pstVdec->stSoftCodec, MT_NULL);
        pstVdec->bThreadExist = MT_FALSE;
        VDEC_LOCK(pstVdec->stMutex);
        pstVdec->u32EosFlag = 0;
        VDEC_UNLOCK(pstVdec->stMutex);
    }
#endif

    /* Stop instance */
    if ((pstVdec->pstCodec) && (pstVdec->pstCodec->Stop))
    {
        s32Ret = pstVdec->pstCodec->Stop(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst));
        return VDEC_ConvertError(s32Ret);
    }

    return MT_SUCCESS;
}


#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1)
static mt_void VDEC_MoveYUVData(MT_CODEC_FRAME_S* pstFrame)
{
    mt_u32 u32YAddr;
    mt_u32 u32UAddr;
    mt_u32 u32VAddr;

    u32YAddr = pstFrame->stOutputAddr.u32Vir;
    memcpy((mt_void*)u32YAddr, (mt_void*)pstFrame->unInfo.stVideo.u32YAddr,
        pstFrame->unInfo.stVideo.u32YStride*pstFrame->unInfo.stVideo.u32Height);
    pstFrame->unInfo.stVideo.u32YAddr = pstFrame->stOutputAddr.u32Phy;

    switch (pstFrame->unInfo.stVideo.enColorFormat)
    {
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_400:
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_400:
    case MT_CODEC_COLOR_FORMAT_YUV_PACKAGE_UYVY422:
    case MT_CODEC_COLOR_FORMAT_YUV_PACKAGE_YUYV422:
    case MT_CODEC_COLOR_FORMAT_YUV_PACKAGE_YVYU422:
        pstFrame->unInfo.stVideo.u32UAddr = 0;
        pstFrame->unInfo.stVideo.u32VAddr = 0;
        pstFrame->unInfo.stVideo.u32UStride = 0;
        pstFrame->unInfo.stVideo.u32VStride = 0;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_411:
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_420:
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_422_1X2:
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_422_2X1:
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_444:
        u32UAddr = u32YAddr + pstFrame->unInfo.stVideo.u32YStride*pstFrame->unInfo.stVideo.u32Height;
        memcpy((mt_void*)u32UAddr, (mt_void*)pstFrame->unInfo.stVideo.u32UAddr,
            pstFrame->unInfo.stVideo.u32UStride*pstFrame->unInfo.stVideo.u32Height);
        pstFrame->unInfo.stVideo.u32YAddr = pstFrame->stOutputAddr.u32Phy;
        pstFrame->unInfo.stVideo.u32UAddr =
            pstFrame->unInfo.stVideo.u32YAddr + pstFrame->unInfo.stVideo.u32YStride*pstFrame->unInfo.stVideo.u32Height;
        pstFrame->unInfo.stVideo.u32VAddr = 0;
        pstFrame->unInfo.stVideo.u32VStride = 0;
        break;

    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_411:
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_420:
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_422_1X2:
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_422_2X1:
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_444:
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_410:
    default:
        u32UAddr = u32YAddr + pstFrame->unInfo.stVideo.u32YStride*pstFrame->unInfo.stVideo.u32Height;
        u32VAddr = u32UAddr + pstFrame->unInfo.stVideo.u32UStride*pstFrame->unInfo.stVideo.u32Height;
        memcpy((mt_void*)u32UAddr, (mt_void*)pstFrame->unInfo.stVideo.u32UAddr,
            pstFrame->unInfo.stVideo.u32UStride*pstFrame->unInfo.stVideo.u32Height);
        memcpy((mt_void*)u32VAddr, (mt_void*)pstFrame->unInfo.stVideo.u32VAddr,
            pstFrame->unInfo.stVideo.u32VStride*pstFrame->unInfo.stVideo.u32Height);
        pstFrame->unInfo.stVideo.u32YAddr = pstFrame->stOutputAddr.u32Phy;
        pstFrame->unInfo.stVideo.u32UAddr =
            pstFrame->unInfo.stVideo.u32YAddr + pstFrame->unInfo.stVideo.u32YStride*pstFrame->unInfo.stVideo.u32Height;
        pstFrame->unInfo.stVideo.u32VAddr =
            pstFrame->unInfo.stVideo.u32UAddr + pstFrame->unInfo.stVideo.u32VStride*pstFrame->unInfo.stVideo.u32Height;
        break;
    }
}

static inline mt_void VDEC_ConvertFormat(MT_CODEC_COLOR_FORMAT_E enCodecFormat, MT_UNF_VIDEO_FORMAT_E* penUNFFormt)
{
    switch (enCodecFormat)
    {
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_400:
        *penUNFFormt = MT_UNF_FORMAT_YUV_SEMIPLANAR_400;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_411:
        *penUNFFormt = MT_UNF_FORMAT_YUV_SEMIPLANAR_411;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_420:
        *penUNFFormt = MT_UNF_FORMAT_YUV_SEMIPLANAR_420;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_422_1X2:
        *penUNFFormt = MT_UNF_FORMAT_YUV_SEMIPLANAR_422_1X2;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_422_2X1:
        *penUNFFormt = MT_UNF_FORMAT_YUV_SEMIPLANAR_422;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_444:
        *penUNFFormt = MT_UNF_FORMAT_YUV_SEMIPLANAR_444;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_400:
        *penUNFFormt = MT_UNF_FORMAT_YUV_PLANAR_400;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_411:
        *penUNFFormt = MT_UNF_FORMAT_YUV_PLANAR_411;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_420:
        *penUNFFormt = MT_UNF_FORMAT_YUV_PLANAR_420;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_422_1X2:
        *penUNFFormt = MT_UNF_FORMAT_YUV_PLANAR_422_1X2;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_422_2X1:
        *penUNFFormt = MT_UNF_FORMAT_YUV_PLANAR_422_2X1;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_444:
        *penUNFFormt = MT_UNF_FORMAT_YUV_PLANAR_444;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_PLANAR_410:
        *penUNFFormt = MT_UNF_FORMAT_YUV_PLANAR_410;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_PACKAGE_UYVY422:
        *penUNFFormt = MT_UNF_FORMAT_YUV_PACKAGE_UYVY;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_PACKAGE_YUYV422:
        *penUNFFormt = MT_UNF_FORMAT_YUV_PACKAGE_YUYV;
        break;
    case MT_CODEC_COLOR_FORMAT_YUV_PACKAGE_YVYU422:
        *penUNFFormt = MT_UNF_FORMAT_YUV_PACKAGE_YVYU;
        break;
    default:
        *penUNFFormt = MT_UNF_FORMAT_YUV_SEMIPLANAR_420;
        break;
    }
}

static mt_void* VDEC_SoftCodec(mt_void* phVdec)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    MT_CODEC_S* pstCodec;
    VDEC_ES_BUF_S stStrm = {0};
    MT_DRV_VDEC_FRAME_BUF_S stFrmGet = {0};
    MT_DRV_VDEC_USR_FRAME_S stFrmPut = {0};
    MT_CODEC_STREAM_S stCodecStrm;
    MT_CODEC_STREAM_S* pstCodecStrm;
    MT_CODEC_FRAME_S stCodecFrm = {{0}, 0, 0, {{0}}};
    MT_CODEC_CAP_S stCap;
    MT_UNF_ES_BUF_S stDmxBuf = {0};

    mt_s32 s32Ret;
    MT_BOOL bNeedCopyFrame;
    MT_BOOL bGetFrameBuf = MT_FALSE;
    MT_BOOL bRecvStream = MT_FALSE;
    MT_BOOL bDecodeSucc = MT_FALSE;
	mt_u32  u32Count = 0, i = 0;

    VDEC_FIND_INST((*(mt_handle*)phVdec), pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_NULL;
    }

    pstCodec = pstVdec->pstCodec;
    if ((!pstCodec) || (!pstCodec->DecodeFrame))
    {
        return MT_NULL;
    }
    mt_set_pthread_name(__FUNCTION__);
    /* GetCap */
    (mt_void)pstCodec->GetCap(&stCap);
    bNeedCopyFrame = (0 == (stCap.u32CapNumber & MT_CODEC_CAP_OUTPUT2SPECADDR)) ? MT_TRUE : MT_FALSE;

	if (0 == strncmp("VPU", pstCodec->pszName, 3))
	{
		u32Count = 20;
	}
	else
	{
		u32Count = 1;
	}
    while (!pstVdec->bThreadStop)
    {
        VDEC_LOCK(pstVdec->stMutex);

        if (0 != strncmp("VPU", pstCodec->pszName, 3))
        {
            /* Alloc frame buffer */
            if (!bGetFrameBuf)
            {
                s32Ret = VDEC_GetFrameBuf(pstVdec->hFrameBuf, &stFrmGet);
                if (MT_SUCCESS == s32Ret)
                {
                    /* Make stCodecFrm */
                    stCodecFrm.stOutputAddr.u32Phy = stFrmGet.u32PhyAddr;
                    stCodecFrm.stOutputAddr.u32Vir = (mt_u32)MT_MEM_Map(stFrmGet.u32PhyAddr, stFrmGet.u32Size);
                    stCodecFrm.stOutputAddr.u32Size = stFrmGet.u32Size;
                    bGetFrameBuf = MT_TRUE;
                }
                else
                {
                    /*MT_WARN_VDEC("Soft Codec GetFrameBuf fail.\n");*/
                    VDEC_UNLOCK(pstVdec->stMutex);
                    MT_USLEEP(10*1000);
                    continue;
                }
            }
        }
        else
        {
            if(pstCodec->RegFrameBuffer)
            {
                s32Ret= pstCodec->RegFrameBuffer(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst),MT_NULL);
            }
        }

        /* Receive stream */
        pstCodecStrm = MT_NULL;
        if (2 != pstVdec->u32EosFlag)   /* If not end of stream */
        {
            /* For MJPEG, don't receive stream if last frame decode fail due to decoder busy */
            if (bRecvStream)
            {
                pstCodecStrm = &stCodecStrm;
            }
            /* Other: receive stream */
            else
            {
                if (MT_INVALID_HANDLE != pstVdec->hStreamBuf)
                {
                    s32Ret = VDEC_RecvStream(pstVdec->hStreamBuf, &stStrm);
                    if (MT_SUCCESS == s32Ret)
                    {
                        /* Make stCodecStrm */
                        stCodecStrm.pu8Addr = stStrm.pu8Addr;
                        stCodecStrm.s64PtsMs = (mt_s64)stStrm.u64Pts;
                        stCodecStrm.u32Size = stStrm.u32BufSize;
						stCodecStrm.u32PhyAddr = stStrm.u32PhyAddr;
                        pstCodecStrm = &stCodecStrm;
                        bRecvStream = MT_TRUE;
                    }
                    else
                    {
                        MT_WARN_VDEC("Soft Codec RecvStreamBuf fail.\n");
                        if (1 == pstVdec->u32EosFlag)
                        {
                            pstVdec->u32EosFlag = 2;
                            s32Ret = VDEC_VPSSCMD(*((mt_handle*)phVdec), VPSS_CMD_SENDEOS, MT_NULL);
                            if (MT_SUCCESS != s32Ret)
                            {
                                MT_ERR_VDEC("MT_MPI_VDEC_SendEos fialed:%d.\n",s32Ret);
                            }
                            if (0 == strncmp("VPU", pstCodec->pszName, 3))
                            {
                                s32Ret = pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), VPU_CMD_SETSTREAMENDFLAG, (mt_void*)(&(pstVdec->u32EosFlag)));
                                if (MT_SUCCESS != s32Ret)
                                {
                                    MT_ERR_VDEC("VPU_SetStreamEndFlag fialed:%d.\n",s32Ret);
                                }
                            }
                        }
                    }
                }
                else if (MT_INVALID_HANDLE != pstVdec->hDmxVidChn)
                {
			for (i = 0; i < u32Count; i++)
			{
                    s32Ret = MT_MPI_DMX_AcquireEs(pstVdec->hDmxVidChn, &stDmxBuf);
                    if (MT_SUCCESS == s32Ret)
                    {
                        /* Make stCodecStrm */
                        stCodecStrm.pu8Addr = stDmxBuf.pu8Buf;
                        stCodecStrm.s64PtsMs = (mt_s64)stDmxBuf.u64PtsMs;
                        stCodecStrm.u32Size = stDmxBuf.u32BufLen;
                        pstCodecStrm = &stCodecStrm;
                        bRecvStream = MT_TRUE;
							if (0 == strncmp("VPU", pstCodec->pszName, 3))
							{
								s32Ret = pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), VPU_CMD_FIllRAWPACKAGE, (mt_void*)pstCodecStrm);
								if (MT_SUCCESS == s32Ret)
								{
									if (MT_INVALID_HANDLE != pstVdec->hDmxVidChn)
									{
										(mt_void)MT_MPI_DMX_ReleaseEs(pstVdec->hDmxVidChn, &stDmxBuf);
									}
									bRecvStream = MT_FALSE;
									pstCodecStrm = MT_NULL;
								}
								else
								{
									break;
								}
							}
                    }
                    else
                    {
                        MT_WARN_VDEC("Soft Codec AcquireES fail.\n");
                        if (1 == pstVdec->u32EosFlag)
                        {
                            pstVdec->u32EosFlag = 2;
                            s32Ret = VDEC_VPSSCMD(*((mt_handle*)phVdec), VPSS_CMD_SENDEOS, MT_NULL);
                            if (MT_SUCCESS != s32Ret)
                            {
                                MT_ERR_VDEC("MT_MPI_VDEC_SendEos fialed:%d.\n",s32Ret);
                            }
							}
							if (0 == strncmp("VPU", pstCodec->pszName, 3))
							{
								s32Ret = pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), VPU_CMD_SETSTREAMENDFLAG, (mt_void*)(&(pstVdec->u32EosFlag)));
								if (MT_SUCCESS != s32Ret)
								{
									MT_ERR_VDEC("VPU_SetStreamEndFlag fialed:%d.\n",s32Ret);
								}
							}
							break;
                        }
                    }
                }
            }
        }
        g_lowDelayVdecHandle[pstVdec->hCodecInst] = (*(mt_handle*)phVdec);
        /* Decode frame */
        s32Ret = pstCodec->DecodeFrame(
                MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), pstCodecStrm, &stCodecFrm);
        if (MT_SUCCESS == s32Ret || VPU_STR_BUF_INSUFFICIENT_SUCESS == s32Ret)
        {
            bDecodeSucc = MT_TRUE;

            /* If didn't decode to frame address, copy here */
            if (bNeedCopyFrame)
            {
                VDEC_MoveYUVData(&stCodecFrm);
            }
            stFrmPut.bFrameValid = MT_TRUE;
            VDEC_ConvertFormat(stCodecFrm.unInfo.stVideo.enColorFormat, &stFrmPut.enFormat);
            stFrmPut.u32Pts = (mt_u32)stCodecFrm.s64PtsMs;
            stFrmPut.s32YWidth = (mt_s32)stCodecFrm.unInfo.stVideo.u32Width;
            stFrmPut.s32YHeight = (mt_s32)stCodecFrm.unInfo.stVideo.u32Height;
            stFrmPut.s32LumaPhyAddr = (mt_s32)stCodecFrm.unInfo.stVideo.u32YAddr;
            stFrmPut.s32CbPhyAddr = (mt_s32)stCodecFrm.unInfo.stVideo.u32UAddr;
            stFrmPut.s32CrPhyAddr = (mt_s32)stCodecFrm.unInfo.stVideo.u32VAddr;
            stFrmPut.s32LumaStride = (mt_s32)stCodecFrm.unInfo.stVideo.u32YStride;
            stFrmPut.s32ChromStride = (mt_s32)stCodecFrm.unInfo.stVideo.u32UStride;
            stFrmPut.s32ChromCrStride = (mt_s32)stCodecFrm.unInfo.stVideo.u32VStride;
            stFrmPut.bEndOfStream = MT_FALSE;

            /* Free frame buffer */
            (mt_void)VDEC_PutFrameBuf(pstVdec->hFrameBuf, &stFrmPut);

            if (0 != strncmp("VPU", pstCodec->pszName, 3))
            {
                (mt_void)MT_MEM_Unmap((mt_void *)stCodecFrm.stOutputAddr.u32Vir);
            }

            bGetFrameBuf = MT_FALSE;

            pstVdec->bNewFrame = MT_TRUE;
            pstVdec->u32CurrentFrmHeight = stFrmPut.s32YHeight;
            pstVdec->u32CurrentFrmWidth  = stFrmPut.s32YWidth;
            if((pstVdec->u32CurrentFrmHeight != pstVdec->u32LastFrmHeight) || ((pstVdec->u32CurrentFrmWidth!= pstVdec->u32LastFrmWidth)))
            {
                pstVdec->bFrmSizeChangeFlag = MT_TRUE;
                pstVdec->u32LastFrmHeight   = pstVdec->u32CurrentFrmHeight;
                pstVdec->u32LastFrmWidth    = pstVdec->u32CurrentFrmWidth;
            }
            // TODO: Support USERDATA
        }
        else
        {
            bDecodeSucc = MT_FALSE;

            if (MT_ERR_CODEC_NOENOUGHDATA != s32Ret)
            {
                pstVdec->u32ErrStrmNum++;
            }
            if (2 == pstVdec->u32EosFlag)
            {
                stFrmPut.bEndOfStream = MT_TRUE;
                stFrmPut.bFrameValid = MT_FALSE;
                stFrmPut.s32LumaPhyAddr = (mt_s32)stFrmGet.u32PhyAddr;
                (mt_void)VDEC_PutFrameBuf(pstVdec->hFrameBuf, &stFrmPut);

                if (0 != strncmp("VPU", pstCodec->pszName, 3))
                {
                    (mt_void)MT_MEM_Unmap((mt_void *)stCodecFrm.stOutputAddr.u32Vir);
                }

                bGetFrameBuf = MT_FALSE;
            }
        }

        /* Free stream */
        /* For MJPEG: Don't free stream if decoder busy */
        if ((bRecvStream) && (!((MT_UNF_VCODEC_TYPE_MJPEG == pstVdec->stCurAttr.enType) && (MT_ERR_CODEC_BUSY == s32Ret)))
             && (!((VPU_STR_BUF_INSUFFICIENT_SUCESS == s32Ret) || (VPU_STR_BUF_INSUFFICIENT_FAILURE == s32Ret))))
        {
            if (MT_INVALID_HANDLE != pstVdec->hStreamBuf)
            {
                (mt_void)VDEC_RlsStream(pstVdec->hStreamBuf, &stStrm);
            }
            else if (MT_INVALID_HANDLE != pstVdec->hDmxVidChn)
            {
                (mt_void)MT_MPI_DMX_ReleaseEs(pstVdec->hDmxVidChn, &stDmxBuf);
            }
            bRecvStream = MT_FALSE;
        }

        VDEC_UNLOCK(pstVdec->stMutex);

        if (0 == strncmp("VPU", pstCodec->pszName, 3))
        {
            if (!bDecodeSucc)
            {
                MT_USLEEP(8*1000);
            }
        }
        else
        {
        /* If get stream fail or decode frame fail, sleep */
        if ((!bRecvStream) || (!bDecodeSucc))
        {
            MT_USLEEP(10*1000);
        }
        }
    }

    if (bGetFrameBuf)
    {
        stFrmPut.bFrameValid = MT_FALSE;
        stFrmPut.s32LumaPhyAddr = (mt_s32)stFrmGet.u32PhyAddr;
        (mt_void)VDEC_PutFrameBuf(pstVdec->hFrameBuf, &stFrmPut);
        (mt_void)MT_MEM_Unmap((mt_void *)stCodecFrm.stOutputAddr.u32Vir);
    }

    return MT_NULL;
}
#endif

mt_s32 MT_MPI_VDEC_RegisterVcodecLib(const mt_char *pszCodecDllName)
{
#if (MT_VDEC_REG_CODEC_SUPPORT == 1)
    return MT_CODEC_RegisterLib(pszCodecDllName);
#else
    return MT_ERR_VDEC_NOT_SUPPORT;
#endif
}

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
#include "mpi_mmz.h"
mt_s32 VDEC_GeLcevcMem(mt_handle hInst, mmz_buffer_s  * mmz_buf);
mt_s32 MT_MPI_VDEC_LcevcMem(mt_handle hVdec, ulong* pLcevcKvirAddr , phys_addr_t *pLcevcPhyAddr, ulong *pLcevcsize)
{
    VDEC_INST_S* pstVdec = MT_NULL;
	mmz_buffer_s  mmz_buf;
    mt_s32 s32Ret;
    VDEC_CHECK_INIT;
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
		
	memset((void*)&pstVdec->pLecvc_priv, 0, sizeof(pstVdec->pLecvc_priv));
    s32Ret = VDEC_GeLcevcMem(hVdec, &mmz_buf);
	if (MT_SUCCESS == s32Ret)
	{
		*pLcevcPhyAddr = mmz_buf.startPhyAddr; 
		*pLcevcKvirAddr = (ulong) mmz_buf.startVirAddr; 
		*pLcevcsize = mmz_buf.size;
	}
	(void)LCEVC_INIT(&pstVdec->pLecvc_priv);
	return s32Ret;
}


mt_s32 MT_MPI_VDEC_CheckLcevcData(mt_handle hVdec)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    mt_s32 s32Ret = MT_ERR_VDEC_RECEIVE_FAILED;
	MT_VDEC_LCEVC_DATA_S lecvc_data;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_NOT_ENABLE;
    }
	if(MT_NULL == pstVdec->pLecvc_priv.uVirLcevcAddr)
	{
        return MT_ERR_VDEC_NOT_ENABLE;
	}
	if(MT_NULL == pstVdec->pLecvc_priv.pLcevc_SEIpipe)
	{
        return MT_ERR_VDEC_NOT_ENABLE;
	}

	//recive SEI data
	if(0 != pstVdec->pLecvc_priv.pLcevc_SEIpipe->push_cnt)
	{
		//alloc yuv data in dvpCallbackFunction  LCEVC_DVP_OUTPUT_FORMAT_CHANGED
		if(0 == pstVdec->pLecvc_priv.LcecvYuv_size)   
		{
				pstVdec->pLecvc_priv.LcecvYuv_size = LCEVC_MAX_YUV_SIZE;
				pstVdec->pLecvc_priv.LcecvYuv_phy = mt_mmz_new(pstVdec->pLecvc_priv.LcecvYuv_size, 128, MMZ_ZONE_DDR, "LcevcYUV");
				if (0  == pstVdec->pLecvc_priv.LcecvYuv_phy)
				{
					MT_ERR_VDEC("Mem not enough! \n");
					return MT_ERR_VDEC_MALLOC_FAILED;
				}
				pstVdec->pLecvc_priv.LcecvYuv_vir = (ulong)mt_mmz_map(pstVdec->pLecvc_priv.LcecvYuv_phy, 0);
		}
		// check new SEI data
		if(LCEVC_RING_SIZE != pstVdec->pLecvc_priv.pLcevc_SEIpipe->freesize)
		{
			//get SEI from driver(FW)
		    s32Ret =  VDEC_GetLcevcSEIData(hVdec, &lecvc_data);

			if(MT_SUCCESS == s32Ret)
			{
				ulong offset = (ulong)lecvc_data.data - pstVdec->pLecvc_priv.LcevckVirAddr;;
				unsigned char * data = (unsigned char*)((ulong)pstVdec->pLecvc_priv.uVirLcevcAddr + (ulong)offset);
				//send SEI data to LCEVC decoder
				LCEVC_SendSEIData(&pstVdec->pLecvc_priv, lecvc_data.PTS, data, (size_t)lecvc_data.data_size);
			}
		}
		// check new decoder yuv data and push to display queue.
		return MT_SUCCESS;
	}
	
	return MT_SUCCESS;
}

void MT_MPI_VDEC_LcevcStop(mt_handle hVdec)
{
	VDEC_INST_S* pstVdec = MT_NULL;
	VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return ;
    }
	pstVdec->pLecvc_priv.lcevc_stat = VDEC_LCEVC_STOP ;
//	LCEVC_Flush(pstVdec->pLecvc_priv.lcevcDecoder);
//  some time when chanel change call LCEVC_Flush hang
	pstVdec->pLecvc_priv.m_pts_count = 0 ;
	MT_WARN_VDEC("call MT_MPI_VDEC_LcevcStop  \n");
}

void MT_MPI_VDEC_LcevcStart(mt_handle hVdec)
{
	VDEC_INST_S* pstVdec = MT_NULL;
	VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return ;
    }
	MT_WARN_VDEC("call MT_MPI_VDEC_LcevcStart  \n");
	pstVdec->pLecvc_priv.lcevc_stat = VDEC_LCEVC_INIT ;
}

mt_s32 MT_MPI_VDEC_GetLcevcStat(mt_handle hVdec)
{
	VDEC_INST_S* pstVdec = MT_NULL;
	VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return 0;
    }
	return (mt_s32)pstVdec->pLecvc_priv.lcevc_stat;
}

void MT_MPI_VDEC_LcevcPause(mt_handle hVdec, mt_u32 u32PauseFlag)
{
	VDEC_INST_S* pstVdec = MT_NULL;
	VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return ;
    }
	MT_WARN_VDEC("call MT_MPI_VDEC_LcevcPause  \n");
	if(u32PauseFlag)
	{
		pstVdec->pLecvc_priv.lcevc_stat = VDEC_LCEVC_PAUSE;
	}
	else
	{
		pstVdec->pLecvc_priv.lcevc_stat = VDEC_LCEVC_INIT;
	}
}

#endif
mt_s32 MT_MPI_VDEC_AllocChan(mt_handle *phHandle, const MT_UNF_AVPLAY_OPEN_OPT_S *pstMaxCapbility)
{
    VDEC_INST_S* pstVdec;
    mt_s32 s32Ret;

    VDEC_CHECK_INIT;

    if (MT_NULL == phHandle)
    {
        MT_ERR_VDEC("Bad param.\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_SUCCESS != VDEC_AllocHandle(phHandle))
    {
        MT_ERR_VDEC("Alloc handle fail.\n");
        return MT_ERR_VDEC_CREATECH_FAILED;
    }

    /* Allocate resource */
    pstVdec = (VDEC_INST_S*)MT_MALLOC_VDEC(sizeof(VDEC_INST_S));
    if (MT_NULL == pstVdec)
    {
        VDEC_FreeHandle(*phHandle);
        MT_ERR_VDEC("Malloc fail.\n");
        return MT_ERR_VDEC_MALLOC_FAILED;
    }

    /* Init parameter */
    VDEC_LOCK(s_stVdecParam.stMutex);
    pstVdec->hVdec = *phHandle;
    pstVdec->hStreamBuf = MT_INVALID_HANDLE;
    pstVdec->hFrameBuf  = MT_INVALID_HANDLE;
    pstVdec->hCodecInst = MT_INVALID_HANDLE;
    pstVdec->pstCodec = MT_NULL;
    pstVdec->bIsVFMW = MT_FALSE;
    pstVdec->u32StrmBufSize = 0;
    pstVdec->hDmxVidChn = MT_INVALID_HANDLE;
    pstVdec->pfnVpssControl = MT_NULL;
    pstVdec->hVpss = MT_INVALID_HANDLE;

    /*create vpss*/
    if(MT_INVALID_HANDLE == pstVdec->hVpss)
    {
        pstVdec->pfnVpssControl = VPSS_Control;
        s32Ret = VPSS_Control(pstVdec->hVdec,VPSS_CMD_CREATEVPSS,&(pstVdec->hVpss));
        if(MT_SUCCESS != s32Ret)
        {
            VDEC_UNLOCK(s_stVdecParam.stMutex);
            VDEC_FreeHandle(*phHandle);
            MT_FREE_VDEC(pstVdec);
            return s32Ret;
        }
    }
    pstVdec->stCurAttr = s_stVdecParam.stDefAttr;
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) ||(MT_VDEC_VPU_SUPPORT == 1)
    /* Set thread flag */
    pstVdec->u32ErrStrmNum = 0;
    pstVdec->u32ErrFrmNum = 0;
    pstVdec->bThreadStop = MT_TRUE;
    pstVdec->bThreadExist = MT_FALSE;
    pstVdec->bNewFrame = MT_FALSE;
    pstVdec->u32EosFlag = 0;
    pstVdec->u32LastFrmWidth = 0;
    pstVdec->u32LastFrmHeight =0;
    pstVdec->u32CurrentFrmWidth = 0;
    pstVdec->u32CurrentFrmHeight =0;
    pstVdec->bFrmSizeChangeFlag = MT_FALSE;
#endif

    if (MT_NULL == pstMaxCapbility)
    {
#if 0
        mt_sys_version_s    SysVersion;
#ifdef CONFIG_MT_CHIP_ARIA
        MT_CHIP_TYPE_E      ChipType    = MT_CHIP_TYPE_MT_ARIA;
        MT_CHIP_VERSION_E   ChipVersion = MT_CHIP_VERSION_V200;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
        MT_CHIP_TYPE_E      ChipType    = MT_CHIP_TYPE_MT_SYMPHONY;
        MT_CHIP_VERSION_E   ChipVersion;
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
        MT_CHIP_TYPE_E      ChipType    = MT_CHIP_TYPE_MT_SYMPHONY4;
        MT_CHIP_VERSION_E   ChipVersion;
#endif

        memset(&SysVersion,0,sizeof(mt_sys_version_s));

        s32Ret = mt_sys_get_version(&SysVersion);
        if (MT_SUCCESS == s32Ret)
        {
            ChipType    = SysVersion.enChipTypeHardWare;
            ChipVersion = SysVersion.enChipVersion;
        }
#endif

        pstVdec->stOpenParam.enDecType  = MT_UNF_VCODEC_DEC_TYPE_NORMAL;

        pstVdec->stOpenParam.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_H264;

#ifdef CONFIG_MT_CHIP_ARIA
        pstVdec->stOpenParam.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_4096x2160;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
		pstVdec->stOpenParam.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
#else
		pstVdec->stOpenParam.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
#endif
    }
    else
    {
        pstVdec->stOpenParam = *pstMaxCapbility;
        if (pstMaxCapbility->enDecType >= MT_UNF_VCODEC_DEC_TYPE_BUTT)
        {
            pstVdec->stOpenParam.enDecType  = MT_UNF_VCODEC_DEC_TYPE_NORMAL;
        }
        if (pstMaxCapbility->enCapLevel >= MT_UNF_VCODEC_CAP_LEVEL_BUTT)
        {
            pstVdec->stOpenParam.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
        }
        if (pstMaxCapbility->enProtocolLevel >= MT_UNF_VCODEC_PRTCL_LEVEL_BUTT)
        {
            pstVdec->stOpenParam.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_H264;
        }
    }

    list_add_tail(&pstVdec->stVdecNode, &s_stVdecParam.stVdecHead);
    VDEC_UNLOCK(s_stVdecParam.stMutex);
    *phHandle = pstVdec->hVdec;
    MT_INFO_VDEC("Alloc handle = %d\n", *phHandle);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_FreeChan(mt_handle hVdec)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
     /*Destroy vpss*/
    if(MT_INVALID_HANDLE != pstVdec->hVpss)
    {
        (mt_void)VPSS_Control(pstVdec->hVdec,VPSS_CMD_DESTORYVPSS,&(pstVdec->hVpss));

    }
    (mt_void)VDEC_DestroyCodec(pstVdec);


    (mt_void)VDEC_FreeHandle(pstVdec->hVdec);

    /* Delete node from list */
    VDEC_LOCK(s_stVdecParam.stMutex);
    list_del(&pstVdec->stVdecNode);
    VDEC_UNLOCK(s_stVdecParam.stMutex);

    /* Free memory resource */
    MT_FREE_VDEC(pstVdec);

    return MT_SUCCESS;
}
mt_s32 MT_MPI_VDEC_SetChanBufferMode(mt_handle hVdec,VDEC_FRAMEBUFFER_MODE_E enFrameBufferMode)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    if(enFrameBufferMode >= VDEC_BUF_TYPE_BUTT)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    return VDEC_VPSSCMD(hVdec, VPSS_CMD_SETBUFFERMODE, (mt_void*)&enFrameBufferMode);

}
/* set the video decode channel attribute(protocol type, decoder mode,  ErrorCover upper limit, priority) */
mt_s32 MT_MPI_VDEC_SetChanAttr(mt_handle hVdec, const MT_UNF_VCODEC_ATTR_S *pstAttr)
{
    mt_s32 s32Ret;
    //mt_s32 s32ConvertRet;
    VDEC_INST_S* pstVdec = MT_NULL;
    MT_CODEC_ID_E enID;
    MT_CODEC_ATTR_S stAttr;
    VDEC_CHECK_INIT;
    if (MT_NULL == pstAttr)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Convert UNF to MT_CODEC_ID_E */
    enID = VDEC_UNF2CodecId(pstAttr->enType);

    /* Didn't create codec instance */
    if (MT_INVALID_HANDLE == pstVdec->hCodecInst)
    {
        s32Ret = VDEC_CreateCodec(pstVdec, enID);
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_VDEC("Create Codec fail.\n");
            return MT_ERR_VDEC_SETATTR_FAILED;
        }
    }
    /* Type change */
    else if (VDEC_UNF2CodecId(pstVdec->stCurAttr.enType) != enID)
    {
        /* Only VFMW support type switching dynamically */
        if ((!pstVdec->bIsVFMW) || (!MT_CODEC_SupportDecode(pstVdec->pstCodec, enID)) || \
            (MT_CODEC_ID_VIDEO_H263 == enID) || (MT_CODEC_ID_VIDEO_SORENSON == enID))//l00273086
        {
            if(MT_NULL != pstVdec->pstCodec)
            {
                 s32Ret = VDEC_ChanStop(pstVdec);
                 s32Ret |= VDEC_DestroyCodec(pstVdec);
                 if (MT_SUCCESS != s32Ret)
                 {
                     MT_ERR_VDEC("VDEC_ChanStop or VDEC_DestroyCodec fail.\n");
                 }
            }
            s32Ret = VDEC_CreateCodec(pstVdec, enID);
            if (MT_SUCCESS != s32Ret)
            {
                MT_ERR_VDEC("Create Codec fail.\n");
                return MT_ERR_VDEC_SETATTR_FAILED;
            }
        }
    }

    /* Check codec structure */
    if (MT_NULL == pstVdec->pstCodec)
    {
        MT_ERR_VDEC("Invalid Codec\n");
        return MT_ERR_VDEC_SETATTR_FAILED;
    }


    stAttr.enID = enID;
    stAttr.unAttr.stVdec.pPlatformPriv = (mt_void*)pstAttr;
    stAttr.unAttr.stVdec.pCodecContext = pstAttr->pCodecContext;
    pstVdec->stCurAttr = *pstAttr;

    /* Set instance attribute */
    if (MT_NULL != pstVdec->pstCodec->SetAttr)
    {

        s32Ret = pstVdec->pstCodec->SetAttr(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), &stAttr);

        return VDEC_ConvertError(s32Ret);
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec->SetAttr is null!\n");
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_GetChanAttr(mt_handle hVdec, MT_UNF_VCODEC_ATTR_S *pstAttr)
{
    mt_s32 s32Ret;
    VDEC_INST_S* pstVdec = MT_NULL;
    MT_CODEC_ATTR_S stAttr;

    VDEC_CHECK_INIT;

    if (MT_NULL == pstAttr)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Set saved attribute first */
    *pstAttr = pstVdec->stCurAttr;

    /* Get instance attribute */
    if ((pstVdec->pstCodec) && (pstVdec->pstCodec->GetAttr))
    {
        stAttr.unAttr.stVdec.pPlatformPriv = pstAttr;
        s32Ret = pstVdec->pstCodec->GetAttr(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), &stAttr);
        return VDEC_ConvertError(s32Ret);
    }

    pstAttr->u32UseDescInfoFlag = 1;

    /* If didn't create codec instance, will get default attribute */
    return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_ResetChan(mt_handle hVdec, const MT_CODEC_RESETPARAM_S *pstParam)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    mt_s32 s32Ret = MT_SUCCESS;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Reset instance */
    if (pstVdec->pstCodec)
    {
        if (MT_NULL != pstVdec->pstCodec->Reset)
        {
            s32Ret = pstVdec->pstCodec->Reset(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), pstParam);
        }
	    else
	    {
	        MT_ERR_VDEC("ERR, Codec->Reset is null!\n");
	    }

        if (pstVdec->bIsVFMW)
        {
            if (MT_INVALID_HANDLE != pstVdec->hStreamBuf)
            {
                (mt_void)VDEC_ResetStreamBuf(pstVdec->hStreamBuf);
            }
        }
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) ||(MT_VDEC_VPU_SUPPORT == 1)
        else
        {
            VDEC_LOCK(pstVdec->stMutex);
            if (MT_INVALID_HANDLE != pstVdec->hStreamBuf)
            {
                (mt_void)VDEC_ResetStreamBuf(pstVdec->hStreamBuf);
            }
            if (MT_INVALID_HANDLE != pstVdec->hFrameBuf)
            {
                (mt_void)VDEC_ResetFrameBuf(pstVdec->hFrameBuf);
            }
            VDEC_UNLOCK(pstVdec->stMutex);
        }
#endif

        (mt_void)VDEC_VPSSCMD(hVdec, VPSS_CMD_RESETVPSS, MT_NULL);
    }

    return VDEC_ConvertError(s32Ret);
}

mt_s32 MT_MPI_VDEC_ChanStart(mt_handle hVdec)
{
    mt_s32 s32Ret;
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL != pstVdec->pstCodec)
    {
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) ||(MT_VDEC_VPU_SUPPORT == 1)
        /* For soft codec */
        if ((!pstVdec->bIsVFMW) && (!pstVdec->bThreadExist))
        {
            pstVdec->bThreadStop = MT_FALSE;

            /* Create soft codec thread */
            s32Ret = pthread_create(&pstVdec->stSoftCodec, MT_NULL, VDEC_SoftCodec, &(pstVdec->hVdec));
            if (MT_SUCCESS != s32Ret)
            {
                return MT_FAILURE;
            }
            
            pstVdec->bIsStarted   = MT_TRUE;
            pstVdec->bThreadExist = MT_TRUE;
        }
#endif

        /* Start instance */
        if (pstVdec->pstCodec->Start)
        {
            s32Ret = pstVdec->pstCodec->Start(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst));
            if(s32Ret == MT_SUCCESS)
            {
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
				mt_s32 sret = 0;
				sret = MT_MPI_VDEC_LcevcMem(pstVdec->hVdec, &pstVdec->pLecvc_priv.LcevckVirAddr, &pstVdec->pLecvc_priv.LcevcPhyAddr, &pstVdec->pLecvc_priv.Lcevcsize);
				if (sret == MT_SUCCESS)
				{
					MT_WARN_VDEC("%s %d phy %lx size %lx \n", __func__, __LINE__, (ulong)pstVdec->pLecvc_priv.LcevcPhyAddr, (ulong)pstVdec->pLecvc_priv.Lcevcsize);
					pstVdec->pLecvc_priv.uVirLcevcAddr = mt_mmap(pstVdec->pLecvc_priv.LcevcPhyAddr, pstVdec->pLecvc_priv.Lcevcsize);
					pstVdec->pLecvc_priv.pLcevc_SEIpipe = (Lcevc_SEIpipe * )(pstVdec->pLecvc_priv.uVirLcevcAddr + 640*1024);
				}
				else
				{	
					pstVdec->pLecvc_priv.uVirLcevcAddr = NULL;
					pstVdec->pLecvc_priv.pLcevc_SEIpipe = NULL;
				}
				
				MT_WARN_VDEC("call MT_MPI_VDEC_ChanStart  \n");
				pstVdec->pLecvc_priv.lcevc_stat = VDEC_LCEVC_INIT ;
#endif
			pstVdec->bIsStarted  = MT_TRUE;
            }
            return VDEC_ConvertError(s32Ret);
        }

        return MT_SUCCESS;
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }

    return MT_FAILURE;
}

mt_s32 MT_MPI_VDEC_ChanStop(mt_handle hVdec)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    mt_s32 s32Ret;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
		MT_WARN_VDEC("call MT_MPI_VDEC_ChanStop  \n");
		pstVdec->pLecvc_priv.lcevc_stat = VDEC_LCEVC_STOP ;
		pstVdec->pLecvc_priv.m_pts_count = 0 ;
#endif
    s32Ret = VDEC_ChanStop(pstVdec);
    if(s32Ret == MT_SUCCESS)
    {
        pstVdec->bIsStarted  = 0;
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
	LCEVC_DEINIT(&pstVdec->pLecvc_priv);	

	if(0 != pstVdec->pLecvc_priv.LcecvYuv_size)
	{
		(mt_void)mt_mmz_unmap((void*)(ulong)pstVdec->pLecvc_priv.LcecvYuv_vir);
		(mt_void)mt_mmz_delete(pstVdec->pLecvc_priv.LcecvYuv_phy);
		pstVdec->pLecvc_priv.LcecvYuv_size = 0;
	}
	
	if(NULL !=	pstVdec->pLecvc_priv.uVirLcevcAddr)
	{
		(mt_void)mt_munmap(pstVdec->pLecvc_priv.uVirLcevcAddr);
		pstVdec->pLecvc_priv.uVirLcevcAddr = NULL;
		pstVdec->pLecvc_priv.pLcevc_SEIpipe = NULL;
	}
#endif	
    }
    return s32Ret;
}

mt_s32 MT_MPI_VDEC_GetChanStatusInfo(mt_handle hVdec, VDEC_STATUSINFO_S *pstStatusInfo)
{
    VDEC_INST_S* pstVdec = MT_NULL;
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) || (MT_VDEC_VPU_SUPPORT == 1)
    mt_s32 s32Ret;
    MT_DRV_VDEC_STREAMBUF_STATUS_S stStrmStatus;
    MT_DRV_VDEC_FRAMEBUF_STATUS_S stFrmStatus;

    memset(&stStrmStatus,0,sizeof(MT_DRV_VDEC_STREAMBUF_STATUS_S));
    memset(&stFrmStatus,0,sizeof(MT_DRV_VDEC_FRAMEBUF_STATUS_S));
#endif

    VDEC_CHECK_INIT;

    if (MT_NULL == pstStatusInfo)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    memset(pstStatusInfo, 0, sizeof(VDEC_STATUSINFO_S));

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW && pstVdec->bIsStarted)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_GETSTATUSINFO, (mt_void*)pstStatusInfo);
        }
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) || (MT_VDEC_VPU_SUPPORT == 1)
        else
        {
            if (0 == strncmp("VPU", pstVdec->pstCodec->pszName, 3))
            {
                s32Ret = VDEC_GetStreamBufStatus(pstVdec->hStreamBuf, &stStrmStatus);
                s32Ret = pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), VPU_CMD_GETCHANSTATUSINFO, (mt_void*)(&stFrmStatus));
            }
            else
            {
                s32Ret = VDEC_GetStreamBufStatus(pstVdec->hStreamBuf, &stStrmStatus);
                s32Ret |= VDEC_GetFrameBufStatus(pstVdec->hFrameBuf, &stFrmStatus);
            }

            if (MT_SUCCESS != s32Ret)
            {
                return MT_FAILURE;
            }
            //s32Ret =  VDEC_VPSSCMD(hVdec, VPSS_CMD_GETPORTSTATE, (mt_void*)&bAllPortCompleteFrm);
            pstStatusInfo->u32BufferSize = stStrmStatus.u32Size;
            pstStatusInfo->u32BufferAvailable = stStrmStatus.u32Available;
            pstStatusInfo->u32BufferUsed = stStrmStatus.u32Used;
            pstStatusInfo->u32VfmwFrmNum = 0;
            pstStatusInfo->u32VfmwStrmSize = 0;
            pstStatusInfo->u32StrmInBps = 0;
            pstStatusInfo->u32TotalDecFrmNum = stFrmStatus.u32TotalDecFrameNum;
            pstStatusInfo->u32FrameBufNum = stFrmStatus.u32FrameBufNum;
            pstStatusInfo->bAllPortCompleteFrm = stFrmStatus.bAllPortCompleteFrm;
            pstStatusInfo->u32TotalErrFrmNum = pstVdec->u32ErrFrmNum;
            pstStatusInfo->u32TotalErrStrmNum = pstVdec->u32ErrStrmNum;
            pstStatusInfo->bEndOfStream = (2==pstVdec->u32EosFlag) ? MT_TRUE : MT_FALSE;
        }
#endif
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_GetCiTestInfo(mt_handle hVdec, MT_UNF_AVPLAY_CI_TEST_INFO_S *pstInfo)
{
    VDEC_INST_S* pstVdec = MT_NULL;
	mt_s32 s32Ret = MT_SUCCESS;

	VDEC_CHECK_INIT;

	if (MT_NULL == pstInfo){
		return MT_ERR_VDEC_INVALID_PARA;
	}

	VDEC_FIND_INST(hVdec, pstVdec);
	if (MT_NULL == pstVdec){
		return MT_ERR_VDEC_INVALID_PARA;
	}

	if (MT_NULL != pstVdec->pstCodec){
//#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) || (MT_VDEC_VPU_SUPPORT == 1)
		s32Ret = VDEC_GetCiTestInfo(hVdec, pstInfo);
		if (MT_SUCCESS != s32Ret){
			return MT_FAILURE;
		}
//#endif
	}else{
		MT_ERR_VDEC("ERR, VCodec is null!\n");
	}

	return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_GetChanFrmStatusInfo(mt_handle hVdec, mt_handle  hPort,VDEC_FRMSTATUSINFO_S *pstVdecFrmStatus)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_INST_S* pstVdec = MT_NULL;
    VDEC_FRMSTATUSINFOWITHPORT_S stVdecFrmStatusInfo;
    VDEC_CHECK_INIT;

    if (MT_NULL == pstVdecFrmStatus)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    memset(pstVdecFrmStatus, 0, sizeof(VDEC_FRMSTATUSINFO_S));
    memset(&stVdecFrmStatusInfo, 0, sizeof(VDEC_FRMSTATUSINFOWITHPORT_S));
    stVdecFrmStatusInfo.hPort = hPort;
    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            s32Ret = VPSS_Control(hVdec,VPSS_CMD_GETSTATUSINFO,(mt_void*)&stVdecFrmStatusInfo);
            if (MT_SUCCESS == s32Ret)
            {
                memcpy(pstVdecFrmStatus,&stVdecFrmStatusInfo.stVdecFrmStatus,sizeof(VDEC_FRMSTATUSINFO_S));
            }
        }
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }
    return s32Ret;
}

mt_s32 MT_MPI_VDEC_GetChanStreamInfo(mt_handle hVdec, MT_UNF_VCODEC_STREAMINFO_S *pstStreamInfo)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    MT_CODEC_STREAMINFO_S stInfo;
    mt_s32 s32Ret;

    VDEC_CHECK_INIT;

    if (MT_NULL == pstStreamInfo)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    memset(pstStreamInfo, 0, sizeof(MT_UNF_VCODEC_STREAMINFO_S));

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Get instance stream info */
    if ((pstVdec->pstCodec) && (pstVdec->pstCodec->GetStreamInfo))
    {
        s32Ret = pstVdec->pstCodec->GetStreamInfo(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), &stInfo);
        if (MT_SUCCESS != s32Ret)
        {
            return VDEC_ConvertError(s32Ret);
        }

        /* Convert info */
        pstStreamInfo->enVCodecType  = VDEC_CodecId2UNF(stInfo.stVideo.enCodecID);
        pstStreamInfo->enSubStandard = (MT_UNF_VIDEO_SUB_STANDARD_E)stInfo.stVideo.enSubStandard;
        pstStreamInfo->u32SubVersion = stInfo.stVideo.u32SubVersion;
        pstStreamInfo->u32Profile = stInfo.stVideo.u32Profile;
        pstStreamInfo->u32Level = stInfo.stVideo.u32Level;
        pstStreamInfo->enDisplayNorm =VDEC_DisplayFmt2UNF(stInfo.stVideo.enDisplayNorm);
        pstStreamInfo->bProgressive = stInfo.stVideo.bProgressive;
        pstStreamInfo->u32AspectWidth = stInfo.stVideo.u32AspectWidth;
        pstStreamInfo->u32AspectHeight = stInfo.stVideo.u32AspectHeight;
        pstStreamInfo->u32bps = stInfo.stVideo.u32bps;
        pstStreamInfo->u32fpsInteger = stInfo.stVideo.u32FrameRateInt;
        pstStreamInfo->u32fpsDecimal = stInfo.stVideo.u32FrameRateDec;
        pstStreamInfo->u32Width  = stInfo.stVideo.u32Width;
        pstStreamInfo->u32Height = stInfo.stVideo.u32Height;
        pstStreamInfo->u32DisplayWidth   = stInfo.stVideo.u32DisplayWidth;
        pstStreamInfo->u32DisplayHeight  = stInfo.stVideo.u32DisplayHeight;
        pstStreamInfo->u32DisplayCenterX = stInfo.stVideo.u32DisplayCenterX;
        pstStreamInfo->u32DisplayCenterY = stInfo.stVideo.u32DisplayCenterY;
        pstStreamInfo->u32IsHDR = stInfo.stVideo.u32IsHDR;
    }
    else
    {
        pstStreamInfo->enVCodecType = MT_UNF_VCODEC_TYPE_BUTT;
        pstStreamInfo->enDisplayNorm = MT_UNF_ENC_FMT_BUTT;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_CheckNewEvent(mt_handle hVdec, VDEC_EVENT_S *pstNewEvent)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    mt_s32 s32Ret = MT_SUCCESS;
    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL == pstNewEvent)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    memset(pstNewEvent, 0, sizeof(VDEC_EVENT_S));

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_CHECKEVT, (mt_void*)pstNewEvent);
        }
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) ||(MT_VDEC_VPU_SUPPORT == 1)
        else
        {
            if (pstVdec->bNewFrame)
            {
                pstNewEvent->bNewFrame = pstVdec->bNewFrame;
                pstVdec->bNewFrame = MT_FALSE;
            }
            if (0 == strncmp("VPU", pstVdec->pstCodec->pszName, 3))
            {
				s32Ret =  pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), VPU_CMD_CHECKEVT, (mt_void*)pstNewEvent);
				if(pstVdec->bFrmSizeChangeFlag)
				{
				    pstNewEvent->bNormChange = MT_TRUE;
					pstNewEvent->stNormChangeParam.u32ImageWidth = (pstVdec->u32LastFrmWidth+1)/2*2;
					pstNewEvent->stNormChangeParam.u32ImageHeight= (pstVdec->u32LastFrmHeight+3)/4*4;
					pstVdec->bFrmSizeChangeFlag = MT_FALSE;
            }
				return s32Ret;
            }
            if(pstVdec->bFrmSizeChangeFlag)
            {
                pstNewEvent->bNormChange = MT_TRUE;
                pstNewEvent->stNormChangeParam.u32ImageWidth = (pstVdec->u32LastFrmWidth+1)/2*2;
                pstNewEvent->stNormChangeParam.u32ImageHeight= (pstVdec->u32LastFrmHeight+3)/4*4;
                pstVdec->bFrmSizeChangeFlag = MT_FALSE;
            }
        }
#endif
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }

    return s32Ret;
}

mt_s32 MT_MPI_VDEC_ReadNewFrame(mt_handle hVdec, MT_DRV_VIDEO_FRAME_S *pstNewFrame)
{

    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL == pstNewFrame)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    memset(pstNewFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_READNEWFRAME, (mt_void*)pstNewFrame);
        }
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1) || (MT_VDEC_VPU_SUPPORT == 1)
        else
        {
            if (0 == strncmp("VPU", pstVdec->pstCodec->pszName, 3))
            {
                pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), VPU_CMD_READNEWFRAME, (mt_void*)(pstNewFrame));
            }
            else
            {
                return VDEC_GetNewFrm(pstVdec->hFrameBuf, pstNewFrame);
            }
        }
#endif
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_ChanRecvUsrData(mt_handle hVdec, MT_UNF_VIDEO_USERDATA_S *pstUsrData)
{
    VDEC_CHECK_INIT;

    if (MT_NULL == pstUsrData)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    memset(pstUsrData, 0, sizeof(MT_UNF_VIDEO_USERDATA_S));

    return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_READUSRDATA, (mt_void*)pstUsrData);
}

mt_s32 MT_MPI_VDEC_SetChanFrmPackType(mt_handle hVdec, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *pFrmPackingType)
{
    VDEC_CHECK_INIT;

    return VDEC_VPSSCMD(hVdec, VPSS_CMD_SETCHAN_FRMPACKTYPE, (mt_void*)pFrmPackingType);
}

mt_s32 MT_MPI_VDEC_GetChanFrmPackType(mt_handle hVdec, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *pFrmPackingType)
{
    VDEC_CHECK_INIT;

    return VDEC_VPSSCMD(hVdec, VPSS_CMD_GETCHAN_FRMPACKTYPE, (mt_void*)pFrmPackingType);
}

mt_s32 MT_MPI_VDEC_SetChanFrmRate(mt_handle hVdec, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL == pstFrmRate)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_SETFRAMERATE, (mt_void*)pstFrmRate);
        }
#if (MT_VDEC_VPU_SUPPORT == 1)
        else if (0 == strncmp("VPU", pstVdec->pstCodec->pszName, 3))
        {
            return pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), VPU_CMD_SETFRAMERATE, (mt_void*)pstFrmRate);
        }
#endif
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1)
        else if (MT_INVALID_HANDLE != pstVdec->hFrameBuf)
        {
            return VDEC_SetFrmRate(pstVdec->hFrameBuf, pstFrmRate);
        }
#endif
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }

    return MT_FAILURE;
}

mt_s32 MT_MPI_VDEC_SetChanFrmRateDeclear(mt_handle hVdec, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL == pstFrmRate)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_SETFRAMERATE_DECLEAR, (mt_void*)pstFrmRate);
        }
#if (MT_VDEC_VPU_SUPPORT == 1)
        else if (0 == strncmp("VPU", pstVdec->pstCodec->pszName, 3))
        {
            return pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), VPU_CMD_SETFRAMERATE, (mt_void*)pstFrmRate);
        }
#endif
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1)
        else if (MT_INVALID_HANDLE != pstVdec->hFrameBuf)
        {
            return VDEC_SetFrmRate(pstVdec->hFrameBuf, pstFrmRate);
        }
#endif
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }

    return MT_FAILURE;
}

mt_s32 MT_MPI_VDEC_SetBuffClearnComp(mt_handle hVdec, MT_BOOL bVOClearnFlag)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    MT_BOOL* pstVOClearFlag = &bVOClearnFlag;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_SET_BUFCLEAR, (mt_void*)pstVOClearFlag);
        }
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }

    return MT_FAILURE;
}

mt_s32 MT_MPI_VDEC_GetChanFrmRate(mt_handle hVdec, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL == pstFrmRate)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_GETFRAMERATE, (mt_void*)pstFrmRate);
        }
#if (MT_VDEC_VPU_SUPPORT == 1)
        else if (0 == strncmp("VPU", pstVdec->pstCodec->pszName, 3))
        {
            return pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst), VPU_CMD_GETFRAMERATE, (mt_void*)pstFrmRate);
        }
#endif
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1)
        else if (MT_INVALID_HANDLE != pstVdec->hFrameBuf)
        {
            return VDEC_GetFrmRate(pstVdec->hFrameBuf, pstFrmRate);
        }
#endif
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }

    return MT_FAILURE;
}

mt_s32 MT_MPI_VDEC_ChanIFrameDecode(mt_handle hVdec, MT_UNF_AVPLAY_I_FRAME_S *pstIFrameStream,
                                    MT_DRV_VIDEO_FRAME_S *pstVoFrameInfo, MT_BOOL bCapture)
{
    mt_s32 s32Ret;
    VDEC_INST_S* pstVdec = MT_NULL;
    VFMW_IFRAME_PARAM_S stIFrameParam;
    MT_UNF_VCODEC_ATTR_S stDefAttr;

    VDEC_CHECK_INIT;

    if ((MT_NULL == pstIFrameStream) || (MT_NULL == pstVoFrameInfo))
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Set type here */
    if (MT_INVALID_HANDLE == pstVdec->hCodecInst)
    {
        s32Ret = MT_MPI_VDEC_GetChanAttr(hVdec, &stDefAttr);
        stDefAttr.enType = pstIFrameStream->enType;
        s32Ret |= MT_MPI_VDEC_SetChanAttr(hVdec, &stDefAttr);
        if (MT_SUCCESS != s32Ret)
        {
            return s32Ret;
        }
    }

    stIFrameParam.pstIFrameStream = pstIFrameStream;
    stIFrameParam.pstVoFrameInfo = pstVoFrameInfo;
    stIFrameParam.bCapture = bCapture;
    s32Ret = VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_IFRAMEDECODE, (mt_void*)&stIFrameParam);
    return s32Ret;
}

mt_s32 MT_MPI_VDEC_ChanIFrameRelease(mt_handle hVdec, MT_DRV_VIDEO_FRAME_S *pstVoFrameInfo)
{
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    if (MT_NULL == pstVoFrameInfo)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_IFRAMERELEASE, (mt_void*)pstVoFrameInfo);
        }
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }

    return MT_FAILURE;
}

mt_s32 MT_MPI_VDEC_ChanPauseDecode(mt_handle hVdec, mt_u32 u32PauseFlag)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_INST_S* pstVdec = MT_NULL;
    VDEC_CHECK_INIT;
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    s32Ret = VDEC_SetDecPause(hVdec, &u32PauseFlag);
    return s32Ret;
}


mt_s32 MT_MPI_VDEC_DecFrmType(mt_handle hVdec, MT_UNF_DEC_FRM_TYPE_E eDecFrmType)
{
	mt_s32 s32Ret = MT_FAILURE;
	VDEC_INST_S* pstVdec = MT_NULL;
	VDEC_CHECK_INIT;
	VDEC_FIND_INST(hVdec, pstVdec);
	if (MT_NULL == pstVdec)
	{
		return MT_ERR_VDEC_INVALID_PARA;
	}
	s32Ret = VDEC_SetDecFrmType(hVdec, &eDecFrmType);
	return s32Ret;
}

mt_s32 MT_MPI_VDEC_SetTrickCfg(mt_handle hVdec, MT_UNF_DEC_TRICK_PARAM_S *pstTrickParam)
{
	mt_s32 s32Ret = MT_FAILURE;
	VDEC_INST_S* pstVdec = MT_NULL;
	VDEC_CHECK_INIT;
	VDEC_FIND_INST(hVdec, pstVdec);
	if (MT_NULL == pstVdec)
	{
		return MT_ERR_VDEC_INVALID_PARA;
	}
	s32Ret = VDEC_SetTrickCfg(hVdec, pstTrickParam);
	return s32Ret;
}


//add by l00225186
mt_s32 MT_MPI_VDEC_CreatePort(mt_handle hVdec, mt_handle *phPort, VDEC_PORT_ABILITY_E ePortAbility)
{
    VDEC_CHECK_INIT;
    if (MT_NULL == phPort)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    VDEC_PORT_CFG_S s_PortCfg;
    s_PortCfg.phPort = phPort;
    s_PortCfg.ePortAbility = ePortAbility;
    return VDEC_VPSSCMD(hVdec, VPSS_CMD_CREATEPORT, (mt_void*)&s_PortCfg);
}
//add by l00225186
mt_s32 MT_MPI_VDEC_DestroyPort(mt_handle hVdec,mt_handle hPort)
{
    VDEC_CHECK_INIT;
    #if 0
    if(hPort < 0 || hPort > MAX_VPSS_PORT_NUM)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    #endif
    return VDEC_VPSSCMD(hVdec, VPSS_CMD_DESTORYPORT, (mt_void*)&hPort);
}
//add by l00225186
mt_s32 MT_MPI_VDEC_EnablePort(mt_handle hVdec,mt_handle hPort)
{
    VDEC_CHECK_INIT;
    return VDEC_VPSSCMD(hVdec, VPSS_CMD_ENABLEPORT, (mt_void*)&hPort);
}
//add by l00225186
mt_s32 MT_MPI_VDEC_DisablePort(mt_handle hVdec,mt_handle hPort)
{
    VDEC_CHECK_INIT;
    return VDEC_VPSSCMD(hVdec, VPSS_CMD_DISABLEPORT, (mt_void*)&hPort);
}
//add by l00225186
mt_s32 MT_MPI_VDEC_GetPortParam(mt_handle hVdec, mt_handle hPort, VDEC_PORT_PARAM_S *pstParam)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_PORT_PARAM_WITHPORT_S stVdecPortParamWithPort;
    VDEC_CHECK_INIT;
    if (MT_NULL == pstParam || MT_INVALID_HANDLE == hPort)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    memset(pstParam, 0, sizeof(VDEC_PORT_PARAM_S));
    memset(&stVdecPortParamWithPort, 0, sizeof(VDEC_PORT_PARAM_WITHPORT_S));
    stVdecPortParamWithPort.hPort = hPort;
    s32Ret =  VDEC_VPSSCMD(hVdec, VPSS_CMD_GETPORTPARAM, (mt_void*)&stVdecPortParamWithPort);
    if (MT_SUCCESS == s32Ret)
    {
        memcpy(pstParam,&stVdecPortParamWithPort.stVdecPortParam,sizeof(VDEC_PORT_PARAM_S));
    }
    return s32Ret;
}
#if 0
mt_s32 MT_MPI_VDEC_SetMainPort(mt_handle hVdec, mt_handle hPort)
{
    VDEC_CHECK_INIT;

    return VDEC_VPSSCMD(hVdec, VPSS_CMD_SETMAINPORT, (mt_void*)&hPort);
}
#endif

mt_s32 MT_MPI_VDEC_SetPortType(mt_handle hVdec, mt_handle hPort, VDEC_PORT_TYPE_E enPortType)
{
    VDEC_CHECK_INIT;
    VDEC_PORT_TYPE_WITHPORT_S stPortTypeWithPortHandle;
    memset(&stPortTypeWithPortHandle, 0, sizeof(VDEC_PORT_TYPE_WITHPORT_S));
    stPortTypeWithPortHandle.hPort = hPort;
    stPortTypeWithPortHandle.enPortType = enPortType;
    return VDEC_VPSSCMD(hVdec, VPSS_CMD_SETPORTTYPE, (mt_void*)&stPortTypeWithPortHandle);
}

mt_s32 MT_MPI_VDEC_GetPortAttr(mt_handle hVdec, mt_handle hPort, MT_DRV_VPSS_PORT_CFG_S *pstPortCfg)
{
    mt_s32                          Ret;
    VDEC_PORT_ATTR_WITHHANDLE_S     stAttrWithHandle;

    VDEC_CHECK_INIT;
    memset(&stAttrWithHandle,0,sizeof(VDEC_PORT_ATTR_WITHHANDLE_S));
    stAttrWithHandle.hPort = hPort;

    Ret = VDEC_VPSSCMD(hVdec, VPSS_CMD_GETPORTATTR, (mt_void*)&stAttrWithHandle);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_VDEC("VPSS_CMD_GETPORTATTR ERR, Ret=%#x\n", Ret);
        return Ret;
    }

    *pstPortCfg = stAttrWithHandle.stPortCfg;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_SetPortAttr(mt_handle hVdec, mt_handle hPort, MT_DRV_VPSS_PORT_CFG_S *pstPortCfg)
{
    VDEC_PORT_ATTR_WITHHANDLE_S     stAttrWithHandle;

    VDEC_CHECK_INIT;

    stAttrWithHandle.hPort = hPort;
    stAttrWithHandle.stPortCfg = *pstPortCfg;

    return VDEC_VPSSCMD(hVdec, VPSS_CMD_SETPORTATTR, (mt_void*)&stAttrWithHandle);

}

//add by l00225186
mt_s32 MT_MPI_VDEC_CancleMainPort(mt_handle hVdec, mt_handle hPort)
{
    VDEC_CHECK_INIT;

    return VDEC_VPSSCMD(hVdec, VPSS_CMD_CANCLEMAINPORT, (mt_void*)&hPort);
}
#define RETRY_TIMES 8
//add by l00225186
mt_s32 MT_MPI_VDEC_ReceiveFrame(mt_handle hVdec, MT_DRV_VIDEO_FRAME_PACKAGE_S *pFrmPack)
{
    mt_s32 s32Ret;
    VDEC_CHECK_INIT;
    if(MT_NULL == pFrmPack)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
	VDEC_INST_S* pstVdec = MT_NULL;
	VDEC_FIND_INST(hVdec, pstVdec);
#endif	
    /*get port frames*/
    /*CNcomment:主要是从对应的PORT的队列里面取东西*/
    s32Ret = VDEC_VPSSCMD(hVdec, VPSS_CMD_RECEIVEFRAME, (mt_void*)pFrmPack);
	
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
	
	MT_INFO_VDEC("s32Ret %d lcevc_stat %d  \n",  s32Ret, pstVdec->pLecvc_priv.lcevc_stat);
	if ((MT_SUCCESS == s32Ret) && (pstVdec->pLecvc_priv.lcevc_stat == VDEC_LCEVC_INIT))
	{
		mt_s32 ret;
		int i = 0;
		MT_MPI_VDEC_CheckLcevcData(hVdec);
		if(pstVdec->pLecvc_priv.pLcevc_SEIpipe->push_cnt > 0)
		{
	
			while(pstVdec->pLecvc_priv.m_pts_count < 4)
			{
				usleep(8*1000);
				MT_INFO_VDEC("usleep \n");
				MT_MPI_VDEC_CheckLcevcData(hVdec);

				if(pstVdec->pLecvc_priv.lcevc_stat != VDEC_LCEVC_INIT)
					break;
			}

			for(i=0; i<RETRY_TIMES; i++)
			{
				 MT_MPI_VDEC_CheckLcevcData(hVdec);
				 ret = pop_pts(&pstVdec->pLecvc_priv, &pFrmPack->stFrame[0].stFrameVideo);
				 
				 if(pstVdec->pLecvc_priv.lcevc_stat != VDEC_LCEVC_INIT)
					 break;

				 if(MT_SUCCESS != ret)
				 {
					usleep(10*1000);
					MT_MPI_VDEC_CheckLcevcData(hVdec);
					ret = pop_pts(&pstVdec->pLecvc_priv, &pFrmPack->stFrame[0].stFrameVideo);
					if(MT_SUCCESS == ret)
					{
						break;
					}
				 }
				 else
				 {	
				 	break;
				 }
			}
			if(i >=RETRY_TIMES)
			{
					#if 0
					for(i=0; i<MAX_FRAME; i++)
					{
						MT_WARN_VDEC("[%d] used %d vpts %d  pts %d , displayed %d \n",
							i, pipe->m_pts_data[i].used,  (pFrmPack->stFrame[0].stFrameVideo.u32Pts/1000)*45,
							(pipe->m_pts_data[i].pts/1000)*45, pipe->m_pts_data[i].displayed);
					
					}
					#endif
				MT_ERR_VDEC("pop_pts_ff %d   index %d sei_cnt %d  pts_count %d freezie %d \n",
				pFrmPack->stFrame[0].stFrameVideo.u32Pts, pFrmPack->stFrame[0].stFrameVideo.u32FrameIndex,
				pstVdec->pLecvc_priv.pLcevc_SEIpipe->push_cnt , pstVdec->pLecvc_priv.m_pts_count ,
				pstVdec->pLecvc_priv.pLcevc_SEIpipe->freesize);
			}
		}
	}
#endif
    return s32Ret;
}
//add by l00225186
mt_s32 MT_MPI_VDEC_ReleaseFrame(mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pVideoFrame)
{
    mt_s32 s32Ret;
    VDEC_CHECK_INIT;
    if(MT_NULL == pVideoFrame)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    /*release port frame*/
    /*CNcomment:主要是从对应的PORT的队列里面释放数据*/
    s32Ret = VPSS_ReleaseFrm(hPort,pVideoFrame);
    return s32Ret;
}

mt_s32 MT_MPI_VDEC_SetLowDelay(mt_handle hVdec, MT_UNF_AVPLAY_LOW_DELAY_ATTR_S *pstAttr)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_INST_S* pstVdec = MT_NULL;
    VDEC_CHECK_INIT;
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }
    s32Ret = VDEC_SetLowDelay(hVdec,pstAttr);
    return s32Ret;
}

mt_s32 MT_MPI_VDEC_ChanDropStream(mt_handle hVdec, mt_u64 *pSeekPts, mt_u32 u32Gap)
{
    mt_s32 s32Ret = MT_FAILURE;
    VFMW_SEEKPTS_PARAM_S stVfmwSeekPts;
    memset(&stVfmwSeekPts,0,sizeof(VFMW_SEEKPTS_PARAM_S));

    if (NULL == pSeekPts)
    {
        MT_ERR_VDEC("pSeekPts is NULL pointer\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
    stVfmwSeekPts.pu64SeekPts = pSeekPts;
    stVfmwSeekPts.u32Gap = u32Gap;
    s32Ret = VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_DROPSTREAM, (mt_void*)&stVfmwSeekPts);
    return s32Ret;
}

mt_s32 MT_MPI_VDEC_SetExternBuffer(mt_handle hVdec, VDEC_BUFFER_ATTR_S* pstBufAttr)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_INST_S* pstVdec = MT_NULL;
    VDEC_CHECK_INIT;
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        MT_ERR_VDEC("------------------\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
    if(MT_NULL == pstBufAttr)
    {
        MT_ERR_VDEC("------------------\n");
        return MT_ERR_VDEC_NULL_PTR;
    }
    s32Ret = VDEC_VPSSCMD(hVdec, VPSS_CMD_SETEXTBUFFER, (mt_void*)pstBufAttr);
    return s32Ret;
}
mt_s32 MT_MPI_VDEC_CheckAndDeleteExtBuffer(mt_handle hVdec,mt_u32 u32PhyAddr,VDEC_FRAMEBUFFER_STATE_E* penBufState)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_INST_S* pstVdec = MT_NULL;
    VDEC_BUFFER_INFO_S stBufferInfo;
    VDEC_CHECK_INIT;
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        MT_ERR_VDEC("the channel did not create\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
    if(MT_NULL == penBufState)
    {
        MT_ERR_VDEC("the param is null\n");
        return MT_ERR_VDEC_NULL_PTR;
    }
    memset(&stBufferInfo,0,sizeof(VDEC_BUFFER_INFO_S));
    stBufferInfo.u32PhyAddr  = u32PhyAddr;
    stBufferInfo.penBufState = penBufState;
    s32Ret = VDEC_VPSSCMD(hVdec, VPSS_CMD_CHECKANDDELBUFFER, (mt_void*)&stBufferInfo);
    return s32Ret;
}
mt_s32 MT_MPI_VDEC_SetExternBufferState(mt_handle hVdec, VDEC_EXTBUFFER_STATE_E enExtBufferState)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_INST_S* pstVdec = MT_NULL;
    VDEC_CHECK_INIT;
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        MT_ERR_VDEC("the channel did not create\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
    s32Ret = VDEC_VPSSCMD(hVdec, VPSS_CMD_SETEXTBUFFERSTATE, (mt_void*)&enExtBufferState);
    return s32Ret;
}
mt_s32 MT_MPI_VDEC_SetResolution(mt_handle hVdec,VDEC_RESOLUTION_ATTR_S* pstResolution)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_INST_S* pstVdec = MT_NULL;
    VDEC_CHECK_INIT;
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        MT_ERR_VDEC("the channel did not create\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }
    s32Ret = VDEC_VPSSCMD(hVdec, VPSS_CMD_SETRESOLUTION, (mt_void*)pstResolution);
    return s32Ret;
}
mt_s32 MT_MPI_VDEC_SetEosFlag(mt_handle hVdec)
{
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_SETEOSFLAG, MT_NULL);
        }
#if (MT_VDEC_REG_CODEC_SUPPORT == 1) || (MT_VDEC_MJPEG_SUPPORT == 1 || (MT_VDEC_VPU_SUPPORT == 1))
        else
        {
            VDEC_LOCK(pstVdec->stMutex);
            pstVdec->u32EosFlag = 1;
            VDEC_UNLOCK(pstVdec->stMutex);
        }
#endif
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_DiscardFrame(mt_handle hVdec, VDEC_DISCARD_FRAME_S * pstParam)
{
    VDEC_CHECK_INIT;
    return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_DISCARDFRAME, (mt_void*)pstParam);
}

mt_s32 MT_MPI_VDEC_Invoke(mt_handle hVdec, MT_CODEC_VIDEO_CMD_S* pstParam)
{
    mt_s32 s32Ret;
    VDEC_INST_S* pstVdec = MT_NULL;
    MT_DRV_VDEC_STREAMBUF_STATUS_S stEsBufStat;
    MT_UNF_AVPLAY_VDEC_INFO_S* pstInfo;

    if ((MT_NULL == pstParam) ||
        (pstParam->u32CmdID >= VFMW_CMD_BUTT) ||
        (pstParam->u32CmdID < VFMW_CMD_GETINFO))
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    VDEC_CHECK_INIT;
    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if ((pstVdec->pstCodec) && (pstVdec->pstCodec->Control))
    {
        if(pstVdec->bIsVFMW)
        {
            s32Ret = pstVdec->pstCodec->Control(MT_CODEC_INST_HANDLE(pstVdec->hCodecInst),
                                                    pstParam->u32CmdID, (mt_void*)pstParam->pPara);
        }
        else
        {
            memset((MT_UNF_AVPLAY_VDEC_INFO_S*)pstParam->pPara, 0, sizeof(MT_UNF_AVPLAY_VDEC_INFO_S));
            s32Ret = MT_SUCCESS;
        }

        /* For VFMW_CMD_GETINFO, need add undecoded frame number of ES Buffer here */
        if ((VFMW_CMD_GETINFO == pstParam->u32CmdID) && (MT_SUCCESS == s32Ret))
        {
            if (MT_INVALID_HANDLE != pstVdec->hStreamBuf)
            {
                s32Ret = VDEC_GetStreamBufStatus(pstVdec->hStreamBuf, &stEsBufStat);
                if (MT_SUCCESS == s32Ret)
                {
                    pstInfo = (MT_UNF_AVPLAY_VDEC_INFO_S*)pstParam->pPara;
                    pstInfo->u32UndecFrmNum += stEsBufStat.u32DataNum;
                }
                else
                {
                    return MT_FAILURE;
                }
            }
        }

        return VDEC_ConvertError(s32Ret);
    }
    else if(pstParam->u32CmdID == VFMW_CMD_SET_PROGRESSIVE)
    {
         return VFMW_SetProgressive(hVdec,(MT_BOOL*)pstParam->pPara);
    }

    return MT_FAILURE;
}

mt_s32 MT_MPI_VDEC_ChanRecvFrm(mt_handle hVdec, MT_DRV_VIDEO_FRAME_S* pstFrameInfo)
{
    VDEC_CHECK_INIT;

    if (MT_NULL == pstFrameInfo)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_RECEIVEFRAME, (mt_void*)pstFrameInfo);
}

mt_s32 MT_MPI_VDEC_ChanRlsFrm(mt_handle hVdec, MT_DRV_VIDEO_FRAME_S* pstFrameInfo)
{
    VDEC_CHECK_INIT;

    if (MT_NULL == pstFrameInfo)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_RELEASEFRAME, (mt_void*)pstFrameInfo);
}

mt_s32 MT_MPI_VDEC_AcqUserData(mt_handle hVdec,
                MT_UNF_VIDEO_USERDATA_S* pstUserData, MT_UNF_VIDEO_USERDATA_TYPE_E* penType)
{
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    VDEC_INST_S* pstVdec = MT_NULL;
    VFMW_USERDATA_S stParam;

    VDEC_CHECK_INIT;

    if ((MT_NULL == pstUserData) || (MT_NULL == penType))
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            stParam.pstData = pstUserData;
            stParam.penType = penType;
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_ACQUSERDATA, (mt_void*)&stParam);
        }
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }
#endif

    return MT_FAILURE;
}

mt_s32 MT_MPI_VDEC_RlsUserData(mt_handle hVdec, MT_UNF_VIDEO_USERDATA_S* pstUserData)
{
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    if (MT_NULL == pstUserData)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_RLSUSERDATA, (mt_void*)pstUserData);
        }
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }
#endif

    return MT_FAILURE;
}

mt_s32 MT_MPI_VDEC_RstUserDataBuffer(mt_handle hVdec)
{
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    if (MT_NULL != pstVdec->pstCodec)
    {
        if (pstVdec->bIsVFMW)
        {
            return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_RSTUSERDATABUF, MT_NULL);
        }
    }
    else
    {
        MT_ERR_VDEC("ERR, Codec is null!\n");
    }
#endif

    return MT_FAILURE;
}


mt_s32 MT_MPI_VDEC_ChanBufferInit(mt_handle hVdec, mt_u32 u32BufSize, mt_handle hDmxVidChn, mt_u32 pip_en)
{
    VDEC_INST_S* pstVdec = MT_NULL;
    VFMW_STREAMBUF_S stStrmBuf;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Stream from buffer(non-demux) */
    if (MT_INVALID_HANDLE == hDmxVidChn)
    {
        /* Create stream buffer */
        if (MT_SUCCESS != VDEC_CreateStreamBuf(hVdec, &pstVdec->hStreamBuf, &pstVdec->u32PhyAddr, u32BufSize, pip_en))
        {
            return MT_ERR_VDEC_BUFFER_ATTACHED;
        }
        pstVdec->hDmxVidChn = MT_INVALID_HANDLE;
    }
    else
    {
        /* Use demux buffer */
        pstVdec->hStreamBuf = MT_INVALID_HANDLE;
        pstVdec->hDmxVidChn = hDmxVidChn;
    }

    pstVdec->u32StrmBufSize = u32BufSize;

    /* If codec instance had been created and it's VFMW, attach buffer to it */
    if (pstVdec->bIsVFMW)
    {
        stStrmBuf.u32BufSize = pstVdec->u32StrmBufSize;
        stStrmBuf.hDmxVidChn = pstVdec->hDmxVidChn;
        stStrmBuf.hStrmBuf = pstVdec->hStreamBuf;
        return VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_ATTACHBUF, (mt_void*)&stStrmBuf);
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_ChanBufferDeInit(mt_handle hVdec)
{
    mt_s32 s32Ret = MT_SUCCESS;
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* If VFMW, need detach stream buffer firstly */
    if (pstVdec->bIsVFMW)
    {
        s32Ret |= VDEC_VFMWSpecCMD(hVdec, VFMW_CMD_DETACHBUF, MT_NULL);
    }

    /* Destroy stream buffer */
    if (MT_INVALID_HANDLE != pstVdec->hStreamBuf)
    {
        s32Ret |= VDEC_DestroyStreamBuf(pstVdec->hStreamBuf);
    }

    return s32Ret;
}

mt_s32 MT_MPI_VDEC_ChanGetBuffer(mt_handle hVdec, mt_u32 u32RequestSize, VDEC_ES_BUF_S *pstBuf)
{
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    return VDEC_GetStreamBuf(pstVdec->hStreamBuf, u32RequestSize, pstBuf);
}

mt_s32 MT_MPI_VDEC_ChanPutBuffer(mt_handle hVdec, VDEC_ES_BUF_S *pstBuf)
{
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    return VDEC_PutStreamBuf(pstVdec->hStreamBuf, pstBuf);
}

mt_s32 MT_MPI_VDEC_GetChanOpenParam(mt_handle hVdec, MT_UNF_AVPLAY_OPEN_OPT_S *pstOpenPara)
{
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    memcpy(pstOpenPara, &pstVdec->stOpenParam, sizeof(MT_UNF_AVPLAY_OPEN_OPT_S));

    return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_ChanGetEsBuffer(mt_handle hVdec, phys_addr_t *u32PhyAddr, mt_u32 *u32BufSize)
{
    VDEC_INST_S* pstVdec = MT_NULL;

    VDEC_CHECK_INIT;

    VDEC_FIND_INST(hVdec, pstVdec);
    if (MT_NULL == pstVdec)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

	*u32PhyAddr = pstVdec->u32PhyAddr;
	*u32BufSize = pstVdec->u32StrmBufSize;
	
    return MT_SUCCESS;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */


