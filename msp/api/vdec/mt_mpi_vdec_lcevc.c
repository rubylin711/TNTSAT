/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

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
#include "mt_mpi_disp.h"

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT

#include "ResidualDecoder.h"
#include <pthread.h>

static 	LCEVC_UpsampleKernel g_kernel = {0};
static pthread_t g_lcevcThd;
int g_StopLcevcThread = 0;
pthread_mutex_t   g_LcevcMutex = PTHREAD_MUTEX_INITIALIZER;
static int64_t g_max_sei_pts = 0;
static int  g_stream_loop = 0;

#define VNCheckB 


static void printLog(void* userData, const char* log, size_t size, LCEVC_LogLevel logLevel)
{
    char lv;
    if ((LCEVC_LogLevel)1 < logLevel) {
        return;
    }

    switch (logLevel) {
        case LCEVC_LogLevel_Verbose: lv = 'V'; break;
        case LCEVC_LogLevel_Info: lv = 'I'; break;
        case LCEVC_LogLevel_Debug: lv = 'D'; break;
        case LCEVC_LogLevel_Warning: lv = 'W'; break;
        case LCEVC_LogLevel_Error: lv = 'E'; break;
        default: break;
    }
    if (log != NULL) {
        printf("[%c LCEVC]:: %.*s", lv, (int32_t)size, log);
    }
}

static void releaseUserdataCallbackFunction(LCEVC_DecoderHandle lcevcDecoder, int64_t timestamp,
                                            void* userdata, void* clientContext)
{
}


/*******************************************************************************************/
void cheeck_need_freefb(Lcevc_priv_pipe *pipe);

void push_pts(Lcevc_priv_pipe *pipe, int64_t pts)
{
	int i = 0;
	
    pthread_mutex_lock(&g_LcevcMutex);
	for(i=0; i<MAX_FRAME; i++)
	{
		if((1 == pipe->m_pts_data[i].used)	&& (0 == pipe->m_pts_data[i].displayed))
		{
			if(pipe->m_pts_data[i].pts < pipe->pLcevc_SEIpipe->m_pop_max_pts )
			{
				pipe->m_pts_data[i].displayed = 1;
			}
		}
	}
    pthread_mutex_unlock(&g_LcevcMutex);

	cheeck_need_freefb(pipe);
	
    pthread_mutex_lock(&g_LcevcMutex);
	for(i=0; i<MAX_FRAME; i++)
	{
		if(0 == pipe->m_pts_data[i].used)
		{
			pipe->m_pts_data[i].pts = pts;
			pipe->m_pts_data[i].used = 1;
			pipe->m_pts_data[i].displayed = 0;
			pipe->m_pts_count++;
			break;
		}
	}
	
	if(i == MAX_FRAME)
	{
		MT_ERR_VDEC("push_pts_f pts %ld m_pts_count = %d	sei_cnt %d   pipe->m_last_display_pts %ld \n", 
			pts, pipe->m_pts_count, pipe->pLcevc_SEIpipe->push_cnt, pipe->pLcevc_SEIpipe->m_last_display_pts/45);
		int j = 0;
		for(j=0; j<MAX_FRAME; j++)
		{
			MT_ERR_VDEC("used [%d]  used %d  pts %ld , displayed %d \n", j , pipe->m_pts_data[j].used,
				 pipe->m_pts_data[j].pts, pipe->m_pts_data[j].displayed);
		}
	}
	else
	{
		MT_WARN_VDEC("push_pts_o pts %ld lst_pts %ld m_pts_count = %d	sei_cnt %d \n", 
			pts, pipe->pLcevc_SEIpipe->m_last_display_pts/45, pipe->m_pts_count, pipe->pLcevc_SEIpipe->push_cnt);
	}

	
    pthread_mutex_unlock(&g_LcevcMutex);
	return ;
}

void cheeck_need_freefb(Lcevc_priv_pipe *pipe)
{
	//check remove unused fb
	int j =0;
	for(j=0; j<pipe->m_total_fb_count; j++)
	{   
		pthread_mutex_lock(&g_LcevcMutex);
		if((1 == pipe->m_pts_data[j].used) && (1 == pipe->m_pts_data[j].displayed))
		{
				if((pipe->pLcevc_SEIpipe->m_last_display_pts >	(ulong)((pipe->m_pts_data[j].pts/1000)*45)) 
				|| (pipe->pLcevc_SEIpipe->m_frame_id > pipe->m_pts_data[j].frameIndex))
				{
					MT_INFO_VDEC("free pts %ld ms  frameindex %d \n",  (ulong)((pipe->m_pts_data[j].pts)), pipe->m_pts_data[j].frameIndex );
					pipe->m_pts_data[j].used = 0;
					pipe->m_pts_data[j].displayed = 0;
					pipe->m_pts_data[j].pts = 0;
				}

				if(abs (pipe->pLcevc_SEIpipe->m_last_display_pts -(ulong)((pipe->m_pts_data[j].pts/1000)*45)) > 2000)
				{
					pipe->m_pts_data[j].used = 0;
					pipe->m_pts_data[j].displayed = 0;
					pipe->m_pts_data[j].pts = 0;
	
				}
		}
		pthread_mutex_unlock(&g_LcevcMutex);
	}
}

int get_free_yuvfb(Lcevc_priv_pipe *pipe)
{
	int i = 0;

	if(0 == pipe->m_total_fb_count)
	{
		return -1;
	}

	cheeck_need_freefb(pipe);

	for(i=0; i<pipe->m_total_fb_count; i++)
	{
		if((0 == pipe->m_pts_data[i].used) && (0 == pipe->m_pts_data[i].displayed))
		{
			break;
		}
	}
	
	if(i == pipe->m_total_fb_count)
	{
		return -1;
	}
	return i;
}

MT_DRV_DISP_FMT_E   g_currFmt = MT_DRV_DISP_FMT_BUTT;

mt_s32 pop_pts(Lcevc_priv_pipe * pipe, MT_DRV_VIDEO_FRAME_S * vframe)
{
	int i = 0;
	mt_s32 iRet = 0;
	mt_s32 match = 0;

	if(g_stream_loop == 1)
	{
		if(vframe->u32Pts > g_max_sei_pts)
		{
			return MT_SUCCESS;
		}
		else
		{
			g_stream_loop = 0;
		}
	}
	
	if(pipe->u32LastPts == vframe->u32Pts)
	{
		return MT_SUCCESS;
	}
	
	if(pipe->pLcevc_SEIpipe->m_pop_max_pts < vframe->u32Pts)
	{
		pipe->pLcevc_SEIpipe->m_pop_max_pts = vframe->u32Pts;
	}

	cheeck_need_freefb(pipe);
    pthread_mutex_lock(&g_LcevcMutex);

	for(i=0; i<MAX_FRAME; i++)
	{
		if((1 == pipe->m_pts_data[i].used) && (vframe->u32Pts == pipe->m_pts_data[i].pts) && (0 == pipe->m_pts_data[i].displayed))
		{
			if((vframe->u32FrameIndex % 10) == 0)
			{	
				MT_DRV_DISP_STEREO_MODE_E penStereo = 0;
				iRet = MT_MPI_DISP_GetFormat(MT_DRV_DISPLAY_1, &penStereo, &g_currFmt);
				if(MT_SUCCESS == iRet)
				{
					if(pipe->residualPlaneInfo.width == 3840)
					{
						if((g_currFmt >= MT_DRV_DISP_FMT_3840X2160_24) && (g_currFmt <= MT_DRV_DISP_FMT_3840X2160_60))
						{
							match = 1;
						}
					}
					
					if(pipe->residualPlaneInfo.width == 1920)
					{
						if((g_currFmt >= MT_DRV_DISP_FMT_1080P_60) && (g_currFmt <= MT_DRV_DISP_FMT_1080i_50))
						{
							match = 1;
						}
					}
					
					if(match)
					{
						pipe->yuv_tvsysMatch = 1;
					}
					else
					{
						pipe->yuv_tvsysMatch = 0;
					}
				
				}
			}
			
			pipe->m_pts_data[i].displayed = 1;
			pipe->m_pts_data[i].frameIndex = vframe->u32FrameIndex;
			//jace set to display queue
			vframe->stLcevcBufAddr.u32PhyAddr_Y = pipe->m_pts_data[i].yuvDataPhy;
			vframe->stLcevcBufAddr.u32PhyAddr_C = pipe->m_pts_data[i].yuvDataPhy;
			vframe->stLcevcBufAddr.u32Stride_Y = pipe->m_pts_data[i].stride;
			vframe->stLcevcBufAddr.u32Stride_C = pipe->m_pts_data[i].stride;
			vframe->bIsLcevc = pipe->yuv_tvsysMatch;
			vframe->u32LcevcPicWidth = pipe->m_pts_data[i].width;
			vframe->u32LcevcPicHeight = pipe->m_pts_data[i].height ;
			MT_DBG_VDEC("MT_UNF_DISP_GetFormat  g_currFmt = 0x%x width %d vframe->bIsLcevc %d vframe->u32FrameIndex %d \n",
				g_currFmt, pipe->residualPlaneInfo.width, vframe->bIsLcevc, vframe->u32FrameIndex);

            switch(pipe->m_pts_data[i].packing)
            {
              case LCEVC_ResidualY8bitS:
                vframe->u8LcevcFormat = 5; // 400
                break;

              default:
                vframe->u8LcevcFormat = 5; // 400
                break;
            }

			memcpy((void*)vframe->k, (void*) g_kernel.k, sizeof(vframe->k));
			vframe->k_len = g_kernel.len;
			break;
		}
	}

    pthread_mutex_unlock(&g_LcevcMutex);

	if(i == MAX_FRAME)
	{
		if(pipe->m_pts_count !=0)
		{
			vframe->bIsLcevc = 0;
			return MT_FAILURE;
		}
	}

	pipe->u32LastPts = vframe->u32Pts;
	MT_WARN_VDEC(" pop_pts_ok %d ! m_last_display_pts	= %ld	u32FrameIndex  %d  displayed %d \n", (vframe->u32Pts),
		(pipe->pLcevc_SEIpipe->m_last_display_pts/45)*1000,  vframe->u32FrameIndex, pipe->m_pts_data[i].displayed );
	return MT_SUCCESS;
}

mt_s32 MT_MPI_VDEC_CheckLcevcData(mt_handle hVdec);

int LCEVC_RecvYUVData(LCEVC_DecoderHandle lcevcDecoder, Lcevc_priv_pipe * pipe);

mt_void *lcevcThd(mt_void *args)
{
	mt_s32 s32LcevcRet;
	Lcevc_priv_pipe  *pLecvc_priv = (Lcevc_priv_pipe*) (ulong)(args);
	usleep(20*1000);
    while (!g_StopLcevcThread)
    {
		if(pLecvc_priv->pLcevc_SEIpipe == NULL)
		{
			usleep(8*1000);
			continue;
		}

		if(pLecvc_priv->lcevc_stat != VDEC_LCEVC_INIT)
		{
			usleep(8*1000);
			continue;
		}
		
		if(0 != pLecvc_priv->pLcevc_SEIpipe->push_cnt)
		{
			s32LcevcRet = LCEVC_RecvYUVData(pLecvc_priv->lcevcDecoder,  pLecvc_priv);
			if((LCEVC_Again == s32LcevcRet) && (!g_StopLcevcThread))
			{
				usleep(1000);
				s32LcevcRet = LCEVC_RecvYUVData(pLecvc_priv->lcevcDecoder,	pLecvc_priv);
			}
		}
		 
		if(MT_SUCCESS == s32LcevcRet)
		{
			continue;
		}

		usleep(8*1000);
    }

	return NULL;
}

void  LCEVC_INIT(Lcevc_priv_pipe  *pLecvc_priv)
{
    LCEVC_ReturnCode ret = LCEVC_Success;
	LCEVC_DecoderHandle lcevcDecoder = NULL;

	if ((ret = LCEVC_CreateDecoder(&printLog, LCEVC_LogLevel_Error, NULL,
								   &lcevcDecoder)) != LCEVC_Success) {
		MT_ERR_VDEC("Failed to create LCEVC decoder instance: %d\n", ret);
		return ;
	}
	MT_WARN_VDEC("lcevcDecoder %lx \n", (ulong)lcevcDecoder);

    /* --------------------------------------------------------- */
    // Create the LCEVC Configuration information
    // Set up blocking mode
    VNCheckB(LCEVC_ConfigureDecoderBool(lcevcDecoder, "blockingReceiveNextCommandBuffers", false));
    VNCheckB(LCEVC_ConfigureDecoderBool(lcevcDecoder, "blockingFinishEnhancementLayer", true));
    VNCheckB(LCEVC_ConfigureDecoderInt(lcevcDecoder, "threadCount", 2));
    VNCheckB(LCEVC_ConfigureDecoderInt(lcevcDecoder, "outputPacking", LCEVC_ResidualY8bitS));
    VNCheckB(LCEVC_ConfigureDecoderInt(lcevcDecoder, "packingMode", LCEVC_PackingModeLinear));
    VNCheckB(LCEVC_ConfigureDecoderBool(lcevcDecoder, "addChromaInCopy", 1));
    // Because we are using LCEVC_RequestLCEVCInputBuffer
    VNCheckB(LCEVC_ConfigureDecoderBool(lcevcDecoder, "externalLcevcCopy", true));
    VNCheckB(LCEVC_ConfigureDecoderInt(lcevcDecoder, "reorderCount", 8));
    VNCheckB(LCEVC_ConfigureDecoderPointer(lcevcDecoder, "releaseUserdataCallback",
                                             releaseUserdataCallbackFunction));
 
	if ((ret = LCEVC_InitializeDecoder(lcevcDecoder)) != LCEVC_Success) {
		MT_ERR_VDEC("LCEVC_InitializeDecoder failed: %d\n", ret);
		
		pLecvc_priv->lcevcDecoder = NULL;
		return ;
	}
	const char* versionString = NULL;
	 if (LCEVC_GetDecoderPropertyString(lcevcDecoder, "version", &versionString) != LCEVC_Success) {
		 MT_ERR_VDEC("LCEVC_GetDecoderPropertyString for version failed\n");
	 } else {
		 printf("DVP version: <%s>\n", versionString);
	 }

	pLecvc_priv->numReorderPics = 0;
	pLecvc_priv->yuv_tvsysMatch = 0;
	g_StopLcevcThread = 0;
	pthread_create(&g_lcevcThd, NULL , (mt_void *)lcevcThd, (mt_void *)pLecvc_priv);
	pLecvc_priv->lcevcDecoder = lcevcDecoder;
		
	MT_WARN_VDEC("%s done: %d lcevcDecoder %p \n", __func__, ret, lcevcDecoder);
}

int  LCEVC_DEINIT(Lcevc_priv_pipe  *pLecvc_priv)
{
    g_StopLcevcThread = MT_TRUE;
    pthread_join(g_lcevcThd, MT_NULL);
	g_max_sei_pts = 0;
	g_stream_loop = 0;
	pLecvc_priv->m_pts_count = 0 ;

	LCEVC_DestroyDecoder(pLecvc_priv->lcevcDecoder);
	pLecvc_priv->lcevcDecoder = NULL;
	return 0;
}

int LCEVC_SendSEIData(Lcevc_priv_pipe *pipe, int64_t timestamp,const uint8_t* data, size_t size)
{
	LCEVC_ReturnCode rVal = LCEVC_Success;
	uint8_t* bufferHandle = NULL;
	uint32_t retryCounter = 0;
	uint32_t lcevcMaxRetrySendCount = 0;
	uint32_t sendFailed = 0 ;

	// might have to wait until memory becomes available
	while (!g_StopLcevcThread) {
		rVal = LCEVC_RequestLCEVCInputBuffer(pipe->lcevcDecoder, size, &bufferHandle);
		if (rVal != LCEVC_Again) {
			break;
		}
		retryCounter++;
		if (retryCounter > lcevcMaxRetrySendCount) {
			lcevcMaxRetrySendCount = retryCounter;
		}
		usleep(500); // wait a short while and try again
	}

	
	if (rVal != LCEVC_Success) {
		
		printf("LCEVC_RequestLCEVCInputBuffer failed rVal %d ! \n", rVal);
	}
	memcpy(bufferHandle, data, size);
	
	if(timestamp > g_max_sei_pts)
	{
		
		g_max_sei_pts = timestamp;
	}

	//for stream loop
		if(g_max_sei_pts > timestamp)
		{
			if(g_max_sei_pts - timestamp >	5*1000*100)
			{
				int j =0;
				printf("stream loop, plush max_pts %ld , timestamp %ld\n",  g_max_sei_pts, timestamp); 
			    if(pipe->pLcevc_SEIpipe == NULL)
				{
					return 0;
				}
				pipe->pLcevc_SEIpipe->m_pop_max_pts = 0;
				g_stream_loop = 1;
				g_max_sei_pts = 0;
//				pipe->m_pts_count = 0;
				pthread_mutex_lock(&g_LcevcMutex);
				for(j=0; j<pipe->m_total_fb_count; j++)
				{	
					pipe->m_pts_data[j].used = 0;
					pipe->m_pts_data[j].displayed = 0;
					pipe->m_pts_data[j].pts = 0;
				}
				pthread_mutex_unlock(&g_LcevcMutex);
				LCEVC_Flush(pipe->lcevcDecoder);
			}
		}
	//for stream loop end


	while (!g_StopLcevcThread) {
		rVal = LCEVC_SendDecoderEnhancementData(pipe->lcevcDecoder,  timestamp,
										 bufferHandle,	size, &pipe->lcevcDecoder);
		if (rVal != LCEVC_Again) {
			break;
		}
		if(sendFailed++ > 1000)
		{
			printf("LCEVC_SendDecoderEnhancementData failed  rVal %d sendFailed %d  \n", rVal, sendFailed);
			sendFailed = 0;
//			LCEVC_Flush(pipe->lcevcDecoder);
			break;
		}
		usleep(5000); // wait a short while and try again
	}
	

	if(pipe->numReorderPics != pipe->pLcevc_SEIpipe->numReorderPics)
	{
		pipe->numReorderPics = pipe->pLcevc_SEIpipe->numReorderPics;
		LCEVC_ConfigureReorderCount(pipe->lcevcDecoder, pipe->numReorderPics);
	}
	if(LCEVC_Success != rVal)
	{
		MT_ERR_VDEC("%s %d failed !\n", __func__, __LINE__ );
		LCEVC_ReleaseLCEVCInputBuffer(pipe->lcevcDecoder, bufferHandle);
	}
	else
	{
		
		MT_INFO_VDEC("%s %d ok !timestamp %ld \n", __func__, __LINE__ , timestamp/1000);
	}
	return (int)rVal;
}

static int32_t bytesPerPixelFromPacking(LCEVC_ResidualPacking packing)
{
    switch (packing) {
        case LCEVC_ResidualY8bitS:
        case LCEVC_ResidualUYVY8bitS:
        case LCEVC_ResidualYVYU8bitS: return 1;
        default: break;
    }
    return 2; // assume 2
}

static bool processCommandBuffer(Lcevc_priv_pipe * pipe, LCEVC_DecoderHandle lcevcDecoder,
								LCEVC_ResidualPlaneInfo* residualPlaneInfo,
                                LCEVC_ResidualPlaneInfo* lastResidualPlaneInfo,
                                LCEVC_ResidualExtract* residualOutput, LCEVC_CommandBuffer* cmdBuffer)
{
	int index = 0;
	int doSkip = 0;
    LCEVC_ReturnCode rVal = LCEVC_Success;

    rVal = LCEVC_GetResidualPlaneInfo(lcevcDecoder, cmdBuffer->timestamp, residualPlaneInfo);
    if (rVal != LCEVC_Success) {
        MT_ERR_VDEC("tryProcessCommandBuffer: PTS %ld  failed LCEVC_GetResidualPlaneInfo %d\n",
                 (ulong)cmdBuffer->timestamp, rVal);
        doSkip = 1; // only call skip as we don't have any idea how big the plane will be
    }
     rVal = LCEVC_GetUpsampleKernel(lcevcDecoder, cmdBuffer->timestamp, &g_kernel);
    if (rVal != LCEVC_Success) {
        MT_ERR_VDEC("tryProcessCommandBuffer: PTS %ld  failed LCEVC_GetUpsampleKernel %d\n",
                 (ulong)cmdBuffer->timestamp, rVal);
        doSkip = 1; // only call skip as we don't have any idea what the upscale kernel should be
    }

	
    if (rVal == LCEVC_Success) {
        // only if we have somewhere to put the output
            if (residualPlaneInfo->size != lastResidualPlaneInfo->size) {
                printf("LCEVC size changed %dx%d -> %dx%d, byteSize %zu -> %zu  frame_cnt %d  packing %d \n",
                      lastResidualPlaneInfo->width, lastResidualPlaneInfo->height,
                      residualPlaneInfo->width, residualPlaneInfo->height,
                      lastResidualPlaneInfo->size, residualPlaneInfo->size,
                      (int)( LCEVC_MAX_YUV_SIZE/residualPlaneInfo->size),
                      residualPlaneInfo->packing);
//init yuvdata address
					  pipe->m_total_fb_count =	MAX_FRAME; // (int)( LCEVC_MAX_YUV_SIZE/residualPlaneInfo->size);
#if 1
				int i = 0;
				for(i=0; i<pipe->m_total_fb_count; i++)
				{
					pipe->m_pts_data[i].yuvDataVir = pipe->LcecvYuv_vir + i*residualPlaneInfo->size ;
					pipe->m_pts_data[i].yuvDataPhy = pipe->LcecvYuv_phy + i*residualPlaneInfo->size ;
					pipe->m_pts_data[i].yuvsize = residualPlaneInfo->size ;
				//	printf("yuvphy addr %lx \n", 	(ulong)pipe->m_pts_data[i].yuvDataPhy);
				}
#else
				 residualOutput->data = realloc(residualOutput->data, residualOutput->size);
#endif			
                *lastResidualPlaneInfo = *residualPlaneInfo;
				
  
                // now resize the unpacked buffer (16bit values)
                size_t byteWidth = residualPlaneInfo->width;
                if (bytesPerPixelFromPacking(pipe->residualPacking) > 1) {
                    byteWidth *= 2;
                }

				//need implemte jace
                pipe->unpackedSize = byteWidth * residualPlaneInfo->height;
				printf("byteWidth %ld residualPlaneInfo->height %d \n", (ulong)byteWidth, residualPlaneInfo->height);
               // pipe->unpackedData = realloc( pipe->unpackedData,  pipe->unpackedSize);
            } else if (lastResidualPlaneInfo->size == 0) {
                printf("No residuals found size %dx%d, packing %d, size %zu\n", residualPlaneInfo->width,
                      residualPlaneInfo->height, residualPlaneInfo->packing, residualPlaneInfo->size);
            }

    } else {
        printf("Failed LCEVC_GetResidualPlaneInfo, result %d\n", rVal);
		return false;
    }

	if(pipe->m_total_fb_count == 0)
	{
		MT_ERR_VDEC("%s %d not inited yet \n", __func__, __LINE__);
		return false;
	}

	int i = 0;
	while(!g_StopLcevcThread)
	{
		if(pipe->lcevc_stat == VDEC_LCEVC_STOP)
		{
			return MT_SUCCESS;
		}

		index = get_free_yuvfb(pipe);
		i++;
		if(index < 0)
		{
		
			usleep(20*1000);
		}
		else
		{
			break;
		}
		if(i>=10)
		{
		
			MT_ERR_VDEC("%s %d no more fb ! timeout break ! doSkip ! \n", __func__, __LINE__);
			index = 0;
			doSkip = 1;
			break;
		}
	}
	
    LCEVC_ResidualPlaneBuffer outputBuffer = {0};
    outputBuffer.size =  residualPlaneInfo->size;
    outputBuffer.data =  (void*) (ulong)(pipe->m_pts_data[index].yuvDataVir);
	outputBuffer.dataHandle = (uint64_t)(pipe->m_pts_data[index].yuvDataPhy);
    outputBuffer.stride = residualPlaneInfo->stride;


    if (doSkip) {
        rVal = LCEVC_StartSkipEnhancementLayer(lcevcDecoder, *cmdBuffer);
    } else {
        rVal = LCEVC_StartGenerateEnhancementLayer(lcevcDecoder, *cmdBuffer, &outputBuffer);
    }

    if (rVal == LCEVC_Success) {
        rVal = LCEVC_FinishEnhancementLayer(lcevcDecoder, residualOutput);
        if (rVal != LCEVC_Success) {
			printf("LCEVC_FinishEnhancementLayer failed line %d \n", __LINE__);
            return false;
        }
    } else {
		printf("LCEVC_StartGenerateEnhancementLayer failed line %d \n", __LINE__);
        return false;
    }

	pipe->m_pts_data[index].stride = residualPlaneInfo->stride;	
	pipe->m_pts_data[index].width = residualPlaneInfo->width;
	pipe->m_pts_data[index].height = residualPlaneInfo->height;
	pipe->m_pts_data[index].packing = residualPlaneInfo->packing;

	push_pts(pipe, cmdBuffer->timestamp);
    return true;
}

int LCEVC_RecvYUVData(LCEVC_DecoderHandle lcevcDecoder, Lcevc_priv_pipe * pipe)
{
    LCEVC_ReturnCode rVal = LCEVC_Success;
	LCEVC_ResidualExtract residualOutputPtr = {0};

	LCEVC_CommandBuffer commandBuffer = {0};
	rVal = LCEVC_ReceiveNextCommandBuffers(lcevcDecoder, &commandBuffer);
	if (rVal == LCEVC_Success) {

		processCommandBuffer(pipe, lcevcDecoder, &pipe->residualPlaneInfo,
							&pipe->lastResidualPlaneInfo, &residualOutputPtr, &commandBuffer);
	}
	return rVal;
}
#endif  // end CONFIG_MT_VDEC_LCEVC_SUPPORT

