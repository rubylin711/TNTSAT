/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#include <linux/string.h>
#include "mtfb.h"
#include "mtfb_p.h"
#include "mtfb_comm.h"
#include "mtfb_drv.h"
#include "mt_drv_file.h"

#ifndef MT_ADVCA_FUNCTION_RELEASE
//#define CFG_MTFB_PROC_SUPPORT
#endif

#ifdef CFG_MTFB_PROC_SUPPORT
#define MTFB_SNAPSHOT_INFO(fmt...) printk(fmt)
extern MTFB_DRV_OPS_S s_stDrvOps;
extern MTFB_DRV_TDEOPS_S s_stDrvTdeOps;
extern mt_u32 mtfb_getbppbyfmt(MTFB_COLOR_FMT_E enColorFmt);
static MTFB_COLOR_FMT_E gSnapshot_ColorFmt = MTFB_FMT_RGB888;
/* to save layer id and layer size */
extern MTFB_LAYER_S s_stLayer[MTFB_MAX_LAYER_NUM];


//位图头文件结构，注意字节对齐情况

typedef struct  tagBITMAPFILEHEADER{
	mt_u16 u16Type;			/*文件类型，设为0x4D42*/
	mt_u32 u32Size;			/*文件大小，像素数据加上头文件大小sizeof*/
	mt_u16 u16Reserved1;		/*保留位*/
    mt_u16 u16Reserved2;		/*保留位*/
    mt_u32 u32OffBits;			/*文件头到实际位图数据的偏移量*/
}__attribute__((packed)) BMP_BMFHEADER_S;

//位图信息头结构
typedef  struct tagBITMAPINFOHEADER{
	mt_u32 u32Size;			/*位图信息头的大小,sizeof(BMP_BMIHEADER_S)*/
	mt_u32 u32Width;			/*图像宽度*/
	mt_u32 u32Height;			/*图像高度*/		
	mt_u16 u32Planes;			/*位图位面数，设为1*/
	mt_u16 u32PixbitCount;			/*每个像素的位数，如RGB8888就是32*/
	mt_u32 u32Compression;	/*位图数据压缩类型，设为0，表示不会压缩*/
	mt_u32 u32SizeImage;		/*位图数据大小，设为0 */
	mt_u32 u32XPelsPerMeter;	/*位图水平分辨率，与图像宽度相同*/
	mt_u32 u32YPelsPerMeter;	/*位图垂直分辨率，与图像高度相同*/
	mt_u32 u32ClrUsed;		/*说明位图实际使用的彩色表中的颜色索引数，设为0*/
	mt_u32 u32ClrImportant;	/*对图像显示很重要的颜色索引数，设为0*/
} BMP_BMIHEADER_S;

mt_void mtfb_captureimage_fillbuffer(mt_u32 u32LayerID, MTFB_BUFFER_S *pstBuffer)
{
    MTFB_PAR_S *par;
	struct fb_info *info;
	
	info = s_stLayer[u32LayerID].pstInfo;
	par  = (MTFB_PAR_S *)(info->par);

	pstBuffer->stCanvas.enFmt      = par->stExtendInfo.enColFmt;
	pstBuffer->stCanvas.u32Width   = par->stExtendInfo.u32DisplayWidth;
	pstBuffer->stCanvas.u32Height  = par->stExtendInfo.u32DisplayHeight;
	pstBuffer->stCanvas.u32PhyAddr = par->stRunInfo.u32ScreenAddr;
	pstBuffer->stCanvas.u32Pitch   = info->fix.line_length;
	
    if (MTFB_LAYER_BUF_NONE == par->stExtendInfo.enBufMode)
	{
	    pstBuffer->stCanvas.enFmt      = par->stDispInfo.stUserBuffer.stCanvas.enFmt;
	    pstBuffer->stCanvas.u32Pitch   = par->stDispInfo.stUserBuffer.stCanvas.u32Pitch;		
	}

	if (par->bSetStereoMode)
	{
	    memcpy(&pstBuffer->stCanvas, &par->st3DInfo.st3DSurface, sizeof(pstBuffer->stCanvas));
	}

#ifdef CFG_MTFB_PROC_SUPPORT 
	if (par->stProcInfo.bWbcProc)
	{
	    MTFB_SLVLAYER_DATA_S stLayerInfo;
		
	    if (s_stDrvOps.MTFB_DRV_GetSlvLayerInfo(&stLayerInfo))
		{
		    MTFB_ERROR("fail to get layer%d info!\n", par->stBaseInfo.u32LayerID);
			return;
		}

		pstBuffer->stCanvas.enFmt      = MTFB_FMT_ARGB8888;
		pstBuffer->stCanvas.u32Width   = stLayerInfo.stCurWBCBufRect.w;
		pstBuffer->stCanvas.u32Height  = stLayerInfo.stCurWBCBufRect.h;
		pstBuffer->stCanvas.u32PhyAddr = stLayerInfo.u32ReadBufAddr;
		pstBuffer->stCanvas.u32Pitch   = stLayerInfo.u32Stride;
	}
#endif

	MTFB_INFO("mtfb_snapshot colorfmt %d, width %d, height %d, pitch %d, addr 0x%x\n",
		             pstBuffer->stCanvas.enFmt,pstBuffer->stCanvas.u32Width,pstBuffer->stCanvas.u32Height,
		             pstBuffer->stCanvas.u32Pitch, pstBuffer->stCanvas.u32PhyAddr);

	return;
}

mt_void mtfb_captureimage_fromdevice(mt_u32 u32LayerID, MT_BOOL bAlphaEnable)
{
	mt_s32 s32Ret;
	mt_u32 u32Row;
	mt_char name[MTFB_FILE_NAME_MAX_LEN];
	mt_s8 *pData, *pTemp;
	mt_u32 u32BufSize, u32Bpp, u32Stride;
	MTFB_BUFFER_S   stSrcBuffer, stDstBuffer;
	MTFB_BLIT_OPT_S stBlitOpt;
	BMP_BMFHEADER_S sBmpHeader;
	BMP_BMIHEADER_S sBmpInfoHeader;
	mt_char filepath[MTFB_FILE_PATH_MAX_LEN-MTFB_FILE_NAME_MAX_LEN]={0};
	mt_char filename[MTFB_FILE_PATH_MAX_LEN]={0};
	struct file* fp;

	memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));

	mtfb_captureimage_fillbuffer(u32LayerID, &stSrcBuffer);

	stSrcBuffer.UpdateRect.x = 0;
	stSrcBuffer.UpdateRect.y = 0;
	stSrcBuffer.UpdateRect.w = stSrcBuffer.stCanvas.u32Width;
	stSrcBuffer.UpdateRect.h = stSrcBuffer.stCanvas.u32Height;

	memcpy(&stDstBuffer, &stSrcBuffer, sizeof(MTFB_BUFFER_S));
	stDstBuffer.stCanvas.enFmt = gSnapshot_ColorFmt;

	/*alloc dst buffer*/
	u32Bpp = mtfb_getbppbyfmt(stDstBuffer.stCanvas.enFmt);
	u32Stride = ((stSrcBuffer.stCanvas.u32Width * u32Bpp>>3) + 0xf)&0xfffffff0;
	u32BufSize = stSrcBuffer.stCanvas.u32Height * u32Stride; 

	stDstBuffer.stCanvas.u32Pitch = u32Stride;

	snprintf(name, sizeof(name),"MTFB_SnapShot%d", u32LayerID);
	stDstBuffer.stCanvas.u32PhyAddr = mtfb_buf_allocmem(name, u32BufSize);
	if (0 == stDstBuffer.stCanvas.u32PhyAddr)
    {
        MTFB_ERROR("failed to malloc the snapshot memory, size: %d KBtyes!\n", u32BufSize/1024);
        return ;
    }
    else
    {
        /* initialize the virtual address and clear memory */
        pData = (mt_s8*)mtfb_buf_map(stDstBuffer.stCanvas.u32PhyAddr);
        if (MT_NULL == pData)
        {
            MTFB_ERROR("Failed to map snapshot memory.\n");
			return;
        }
        else
        {
            memset(pData, 0x00, u32BufSize);
        }
    }

	MTFB_INFO("mtfb_snapshot srcbuf info:\n\
			   phyadd 0x%x, width %d, height %d, stride %d\n",stSrcBuffer.stCanvas.u32PhyAddr,stSrcBuffer.stCanvas.u32Width,
			   stSrcBuffer.stCanvas.u32Height, stSrcBuffer.stCanvas.u32Pitch);
	MTFB_INFO("mtfb_snapshot dstbuf info:\n\
			   phyadd 0x%x, width %d, height %d, stride %d\n",stDstBuffer.stCanvas.u32PhyAddr,stDstBuffer.stCanvas.u32Width,
			   stDstBuffer.stCanvas.u32Height, stDstBuffer.stCanvas.u32Pitch);

	if (bAlphaEnable)
	{
	    stBlitOpt.stAlpha.bAlphaEnable = MT_TRUE;
	    stBlitOpt.stAlpha.u8GlobalAlpha=0xff;
	}
	
	s32Ret = s_stDrvTdeOps.MTFB_DRV_Blit(&stSrcBuffer, &stDstBuffer, &stBlitOpt, MT_TRUE);
	if (s32Ret < 0)
	{
	    MTFB_ERROR("tde blit error!\n");
	    return;
	}

	/*给每一个数据项赋值*/
    sBmpHeader.u16Type = 0x4D42;
    sBmpHeader.u32Size = u32BufSize + sizeof(BMP_BMFHEADER_S) + sizeof(BMP_BMIHEADER_S);
    sBmpHeader.u16Reserved1 = 0;
    sBmpHeader.u16Reserved2 = 0;
    sBmpHeader.u32OffBits = sizeof(BMP_BMFHEADER_S) + sizeof(BMP_BMIHEADER_S); //+ 2;//

	sBmpInfoHeader.u32Size = sizeof(BMP_BMIHEADER_S);
    sBmpInfoHeader.u32Width = stDstBuffer.stCanvas.u32Width;
    sBmpInfoHeader.u32Height = stDstBuffer.stCanvas.u32Height;
    sBmpInfoHeader.u32Planes = 1;
    sBmpInfoHeader.u32PixbitCount = 24;
    sBmpInfoHeader.u32Compression = 0;
    sBmpInfoHeader.u32SizeImage = 0;
    sBmpInfoHeader.u32XPelsPerMeter = stDstBuffer.stCanvas.u32Width;
    sBmpInfoHeader.u32YPelsPerMeter = stDstBuffer.stCanvas.u32Height;
    sBmpInfoHeader.u32ClrUsed = 256;
    sBmpInfoHeader.u32ClrImportant = 0;
    
	mt_drv_file_get_storepath(filepath, MTFB_FILE_PATH_MAX_LEN-MTFB_FILE_NAME_MAX_LEN);
	snprintf(filename, sizeof(filename),"%s/mtfb_snapshot%d.bmp", filepath,u32LayerID);
	//fp = MT_DRV_FILE_Open(filename, 1);
	fp = MT_NULL;
	fp = filp_open(filename, O_WRONLY | O_CREAT | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (IS_ERR(fp))
	{
		MTFB_ERROR("fail to open file %s.\n",filename);
		return;
	}

	MTFB_INFO("success to create file %s.\n",filename);
	
	if (sizeof(BMP_BMFHEADER_S) != 
			mt_drv_file_write(fp, (mt_s8*)&sBmpHeader, sizeof(BMP_BMFHEADER_S)))
	{ 
		MTFB_ERROR("Write data to file %s failure.\n",filename);
		return;
	}

	if (sizeof(BMP_BMIHEADER_S) != 
			mt_drv_file_write(fp, (mt_s8*)&sBmpInfoHeader, sizeof(BMP_BMIHEADER_S)))
	{
		MTFB_ERROR("Write data to file %s failure.\n",filename);
		return;
	}

	u32Row = stSrcBuffer.stCanvas.u32Height;
	pTemp  = pData;
	pTemp += (u32Stride * (stSrcBuffer.stCanvas.u32Height - 1));
	
	while(u32Row)
	{
		if (u32Stride != mt_drv_file_write(fp, (mt_s8*)pTemp, u32Stride))
		{
			MTFB_ERROR("Write data to file %s failure.\n",filename);
			return;
		}

		pTemp -= u32Stride;
		u32Row--;
	}	
    
	mt_drv_file_close(fp);
	
	mtfb_buf_ummap((mt_void *)pData);
	mtfb_buf_freemem(stDstBuffer.stCanvas.u32PhyAddr);
	
	MT_PRINT("success to capture fb%d, store in file %s.\n", u32LayerID, filename);
		
	return;
}
#endif


