/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "string.h"

#include "mt_type.h"
#include "mt_go_decoder.h"
#include "mtgo_bmp.h"
#include "mtgo_adp_sys.h"
#include "mtgo_io.h"
#include "mtgo_surface.h"

#ifdef TEST_IN_ROOTBOX
#include "mt_go_comm.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTGO_BMP_SUPPORT

/***************************** Macro Definition ******************************/
#define BMP_WORD_LEN sizeof(mt_u16)
#define BMP_DWORD_LEN sizeof(mt_u32)
#define BMP_FILEHEADER_LEN 0x0E

/*************************** Structure Definition ****************************/
typedef struct tag_MTGO_BITMAPFILEHEADER
{
    mt_u16 bfType;
    mt_u32 bfSize;
    mt_u16 bfReserved1;
    mt_u16 bfReserved2;
    mt_u32 bfOffBits;
} MTGO_BITMAPFILEHEADER;

typedef struct tag_MTGO_BITMAPINFOHEADER
{
    mt_u32 biSize;
    mt_s32 biWidth;
    mt_s32 biHeight;
    mt_u16 biPlanes;
    mt_u16 biBitCount;
    mt_u32 biCompression;
    mt_u32 biSizeImage;
    mt_s32 biXPelsPerMeter;
    mt_s32 biYPelsPerMeter;
    mt_u32 biClrUsed;
    mt_u32 biClrImportant;
} MTGO_BITMAPINFOHEADER;

typedef struct BMP_DECODER
{
    IO_HANDLE             hStream;
    MTGO_BITMAPFILEHEADER FileHeader;
    MTGO_BITMAPINFOHEADER InfoHeader;
    u8 *pDibData;
} BMP_DECODER_S;

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
//几何变换的质量
#define		IMAGE_GEOMETRY_NEAREST_NEIGHBOR_INTERPOLATE	0X00050100
#define		IMAGE_GEOMETRY_BILINEAR_INTERPOLATE			0X00050101
#define		IMAGE_GEOMETRY_THREE_ORDER_INTERPOLATE		0X00050102
//图像颜色:RGBA
typedef struct tagPIXELCOLORRGBA
{
	u8 red;
	u8 green;
	u8 blue;
	u8 alpha;
}PIXELCOLORRGBA;

typedef u8 *              LPBYTE;

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif



int m_dwQuality = IMAGE_GEOMETRY_NEAREST_NEIGHBOR_INTERPOLATE;

/*把源文件RGB数据转换为目的ARGB格式*/
/*
p_src : 源数据地址
src_w : 源图片宽
p_dst  : 目的数据地址(argb)
dst_w : 目的图片宽
nlines : 源数据行数量
bpp    : 源图片像素位宽
isRGB565 : 源图片是否为rgb565
*/
static  inline void get_truec_line_data_step(u8 * p_src,
									 u32 src_w,
									 u8 * p_dst,
									 u32 dst_w,
									 u32 nlines,
									 u32 bpp,
									 u8 isRGB565)
{
	int j = 0;
	int i = 0;
	u32 src_pitch = 0;
	u32 dst_pitch = (dst_w + 1)/2 * 2 * 4;
	u8 * p_tmp_dst = NULL;
	u8 * p_tmp_src = NULL;
	switch(bpp)
	{
	case 16:

		src_pitch = ((16 * src_w + 31)/32 * 32) /8;
		for(j = 0; j < nlines; j ++)
		{
			p_tmp_dst = p_dst + dst_pitch * j;
			p_tmp_src = p_src + src_pitch * (nlines - 1 - j);
                    if(isRGB565)
                    {
                        for(i = 0; i < src_w; i++)
                        {
                        	*p_tmp_dst ++ = ((*((u16 *)p_tmp_src)) & 0x001F) << 3;
                        	*p_tmp_dst ++ = ((*((u16 *)p_tmp_src)) & 0x07E0) >> 3;
                        	*p_tmp_dst ++ = ((*((u16 *)p_tmp_src)) & 0xf800) >> 8;
                        	*p_tmp_dst ++ = 0xFF;

                        	p_tmp_src += 2;
                        }
                    }
                    else
                     {
        			for(i = 0; i < src_w; i++)
        			{
        				*p_tmp_dst ++ = ((*((u16 *)p_tmp_src)) & 0x001F) << 3;
        				*p_tmp_dst ++ = ((*((u16 *)p_tmp_src)) & 0x03E0) >> 2;
        				*p_tmp_dst ++ = ((*((u16 *)p_tmp_src)) & 0x7C00) >> 7;
        				*p_tmp_dst ++ = 0xFF;

        				p_tmp_src += 2;
        			}
                     }			
		}
		break;

	case 24:
		src_pitch = ((24 * src_w + 31)/32 * 32) /8;
		for(j = 0; j < nlines; j++)
		{
			p_tmp_dst = p_dst + dst_pitch * j;
			p_tmp_src = p_src + src_pitch * (nlines - 1 - j);
			for(i = 0; i < src_w; i++)
			{
				*p_tmp_dst ++ = *p_tmp_src ++;
				*p_tmp_dst ++ = *p_tmp_src ++;
				*p_tmp_dst ++ = *p_tmp_src ++;
				*p_tmp_dst ++ = 0xFF;
			}
		}
		break;
	case 32:
		src_pitch = src_w * 4;

		for(j = 0; j < nlines; j ++)
		{
			p_tmp_dst = p_dst + dst_pitch * j;
			p_tmp_src = p_src + src_pitch * (nlines - 1 - j);
			memcpy(p_tmp_dst,p_tmp_src,src_pitch);
			for(i = 0; i < src_w; i++)
			{
				p_tmp_dst += 3; 
				*p_tmp_dst ++  = 0xFF;
			}
		}
		break;

	}

}


static PIXELCOLORRGBA interpolate_rgba(LPBYTE lpbySrcXY,  int x,  int y,  float fu,  float fv,  int nScanWidth,  int nScanHeight)
{
	PIXELCOLORRGBA rgba = {0};
  u8* pbySrc = NULL;


	switch(m_dwQuality)
	{
		case IMAGE_GEOMETRY_NEAREST_NEIGHBOR_INTERPOLATE :
			pbySrc = lpbySrcXY;
			rgba.blue = *pbySrc++;
			rgba.green = *pbySrc++;
			rgba.red = *pbySrc++;
			rgba.alpha = *pbySrc++;
			break;

		default:
		  break;
	}
	return rgba;
}

/*
lpbyBitsSrc32 : RGBA 数据源
x : 操作源数据横坐标
y : 操作源数据纵坐标
nWidth : 源数据宽
nHeight : 源数据高
nScanWidth  : 目的数据宽
nScanHeight
lpbyBitsDst32
nWidthImgDst
nHeightImgDst
*/
static bool  scale_rgba(LPBYTE lpbyBitsSrc32,  int x,  int y,  int nWidth,  int nHeight,  int nScanWidth,  int nScanHeight, LPBYTE lpbyBitsDst32, int nWidthImgDst, int nHeightImgDst)
{
	u32 dwWidthBytes, align_dst_pitch = 0;
	u8* pbyDst;
	float fScalex;
	float fScaley;
	float fXInverse, fYInverse;
	int xx,yy;
	float fv,fu ;

	int i,j;
	u8* pbyCurrent,*pbySrc;
	PIXELCOLORRGBA rgba;

	int w = min(nWidth, nScanWidth - x);    /*获取实际源缩放的区域宽高大小*/
	int h = min(nHeight, nScanHeight - y);
	//第一步, 进行参数合法性检测

	if((x > (nScanWidth - 1)) || (y > (nScanHeight - 1))) return FALSE;   /*合法性检查*/

	//m_dwOperation = IMAGE_GEOMETRY_SCALE;
	//有效区域的宽度和高度

	


	//注意事项:
	//第一:
	//如果(w <  nWidth), 或者(h <  nHeight)则表示指的区域比能够有效获取数据的区域要大, 
	//这时程序将放大倍数, 使最后缩放的结果总能达到 nWidthImgDst 宽和 nHeightImgDst 高
	
	//第二:	
	//fScalex, fScaley所表示的缩放比为真实缩放比的倒数
	//之所以这样处理是由于, 作一次除法, 总比一次乘法要慢.

	//宽度缩放比
	fScalex = (float)w / (float)nWidthImgDst;    
	fScaley = (float)h / (float)nHeightImgDst;   

	//行字节数
	//for bug 96834
//	dwWidthBytes = (u32)nScanWidth * 4;
	dwWidthBytes = (u32)(nScanWidth +1) / 2 * 2 * 4;   
	//开始数莼饕?	

       align_dst_pitch = (nWidthImgDst + 1) / 2 * 2 * 4;  

	//指向目标数据
	pbyDst = lpbyBitsDst32;      
	//完成变换
	for(i = 0; i < nHeightImgDst;i++)
	{
		//反向变换后获得的浮点y值
		fYInverse = i * fScaley;   /**/
		//取整
		yy = (int)fYInverse;

		//坐标差值
		fv = fYInverse - yy;
		
		//对应于原图像的y坐标
		yy += y;

		pbySrc = lpbyBitsSrc32 + yy * dwWidthBytes;
              pbyDst = lpbyBitsDst32 + i * align_dst_pitch;
		for(j = 0; j < nWidthImgDst; j++)
		{
			//反向变换后获得的浮点x值
			fXInverse = j * fScalex;
			//取整
			xx = (int)fXInverse;

			//坐标差值
			fu = fXInverse - xx;
			
			//对应于原图像的y坐标
			xx += x;
					
			//获取?			
			pbyCurrent =  pbySrc + 4 * xx;
			rgba = interpolate_rgba(pbyCurrent, xx, yy, fu, fv, nScanWidth,  nScanHeight);
			
			*pbyDst++ = rgba.blue;
			*pbyDst++ = rgba.green;
			*pbyDst++ = rgba.red;
			*pbyDst++ = rgba.alpha;
		}
	}
	return TRUE;
}



/** return the actual resolution */
mt_s32 BMP_GetActualSize(DEC_HANDLE BmpDec, mt_s32 Index, const MT_RECT *pSrcRect, MTGO_SURINFO_S *pSurInfo)
{
    MTGO_DEC_IMGINFO_S ImgInfo;
    mt_s32 ret;
    BMP_DECODER_S *pBmpDec = (BMP_DECODER_S*)BmpDec;

    ret = BMP_DecImgInfo(BmpDec, (mt_u32)Index, &ImgInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }
   if((ImgInfo.Width > 2048) ||  (ImgInfo.Height > 2048))
    {
         pSurInfo->Width = 1920;
         pSurInfo->Height  = 1080;
         pSurInfo->PixelFormat = MTGO_PF_8888;
    }
   else
    {
        pSurInfo->Width = (mt_s32)ImgInfo.Width;
        pSurInfo->Height = (mt_s32)ImgInfo.Height;
        pSurInfo->PixelFormat = ImgInfo.Format;
    }
    pSurInfo->Pitch[0] = ((ImgInfo.Width * pBmpDec->InfoHeader.biBitCount + 31) / 32) * 4;
    return MT_SUCCESS;
}


#ifndef MTGO_CODE_CUT
static mt_s32 BMP_Rle8_Decode(const BMP_DECODER_S *pBmpDec, mt_u8 *pDecData)
{
    mt_s32 ret;
    mt_u8 *pRleData, *pRlePos, *pDecPos;
    mt_u32 ImagePitch, CopyLen = 0;
    mt_u32 RleBytes, DecBytes;
    MT_BOOL EndFlag;

    /**malloc memory for ale data*/
    pRleData = (mt_u8*)MTGO_Malloc(pBmpDec->InfoHeader.biSizeImage);
    if (MT_NULL == pRleData)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    /**get rle data*/
    ret = MTGO_ADP_IOSeek(pBmpDec->hStream, IO_POS_SET, (mt_s32)(pBmpDec->FileHeader.bfOffBits));
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        goto OUT1;
    }

    ret = MTGO_ADP_IORead(pBmpDec->hStream, pRleData, pBmpDec->InfoHeader.biSizeImage, &CopyLen, &EndFlag);
    if ((MT_SUCCESS != ret) || (CopyLen < pBmpDec->InfoHeader.biSizeImage))
    {
        MTGO_ERROR(ret);
        goto OUT1;
    }

    /*decode rle data*/
    ImagePitch = (((mt_u32)pBmpDec->InfoHeader.biWidth + 3) / 4) * 4;
    pRlePos  = pRleData;
    pDecPos  = pDecData;
    RleBytes = DecBytes = 0;
    while (RleBytes < pBmpDec->InfoHeader.biSizeImage)
    {
        if (0 != pRlePos[0]) /*encode runs*/
        {
            memset(pDecPos, pRlePos[1], pRlePos[0]);
            pDecPos  += pRlePos[0];
            DecBytes += pRlePos[0];
            pRlePos  += 2;
            RleBytes += 2;
            continue;
        }
        else
        {
            if (0 == pRlePos[1])
            {
                /*end-of-line-Marker*/
                pDecPos  = pDecData + ((DecBytes - 1) / ImagePitch + 1) * ImagePitch;
                DecBytes = (mt_u32)(pDecPos - pDecData);
                pRlePos  += 2;
                RleBytes += 2;
                continue;
            }

            if (1 == pRlePos[1])
            {
                /*end-of-rledata-Marker*/
                break;
            }

            if (2 == pRlePos[1])
            {
                /*delta Marker*/
                pDecPos  = pDecData + (DecBytes / ImagePitch + pRlePos[4]) * ImagePitch + pRlePos[3];
                DecBytes = (mt_u32)(pDecPos - pDecData);
                pRlePos  += 4;
                RleBytes += 4;
                continue;
            }
            else /*unencode runs*/
            {
                memcpy(pDecPos, pRlePos + 2, pRlePos[1]);
                pDecPos  += pRlePos[1];
                DecBytes += pRlePos[1];
                pRlePos  = pRlePos + ((pRlePos[1] + 1) / 2) * 2 + 2;
                RleBytes = RleBytes + (mt_u32)((pRlePos[1] + 1) / 2) * 2 + 2;
                continue;
            }
        }
    }

OUT1:
    MTGO_Free(pRleData);
    pRleData = NULL;

    return ret;
} /*lint !e818 */

static mt_s32 BMP_Rle4_Decode(const BMP_DECODER_S *pBmpDec, mt_u8 *pDecData)
{
    mt_s32 ret, i, MaskIdx;
    mt_u8 *pRleData, *pRlePos, *pDecPos;
    mt_u32 ImagePitch, CopyLen = 0;
    mt_u32 RleBytes, DecBytes;
    MT_BOOL EndFlag;
    mt_u8 Mask[2] = {0x0F, 0xF0};

    /**malloc memory for ale data*/
    pRleData = (mt_u8*)MTGO_Malloc(pBmpDec->InfoHeader.biSizeImage);
    if (MT_NULL == pRleData)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    /**get rle data*/
    ret = MTGO_ADP_IOSeek(pBmpDec->hStream, IO_POS_SET, (mt_s32)(pBmpDec->FileHeader.bfOffBits));
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        goto OUT1;
    }

    ret = MTGO_ADP_IORead(pBmpDec->hStream, pRleData, pBmpDec->InfoHeader.biSizeImage, &CopyLen, &EndFlag);
    if ((MT_SUCCESS != ret) || (CopyLen < pBmpDec->InfoHeader.biSizeImage))
    {
        MTGO_ERROR(ret);
        goto OUT1;
    }

    /*decode rle data*/
    ImagePitch = (mt_u32)((pBmpDec->InfoHeader.biWidth * 4 + 31) / 32) * 4;
    pRlePos  = pRleData;
    pDecPos  = pDecData;
    RleBytes = DecBytes = 0;
    MaskIdx = 0;
    while (RleBytes < pBmpDec->InfoHeader.biSizeImage)
    {
        if (0 != pRlePos[0])
        {
            /*encode runs*/
            for (i = 0; i < pRlePos[0]; i++)
            {
                pDecPos[0] = pDecPos[0] | (pRlePos[1] & Mask[MaskIdx]);
                MaskIdx = 1 - MaskIdx;
                if (MaskIdx == 0)
                {
                    pDecPos++;
                    DecBytes++;
                }
            }

            pRlePos  += 2;
            RleBytes += 2;
            continue;
        }

        if (0 == pRlePos[1])
        {
            /*end-of-line-Marker*/
            pDecPos  = pDecData + ((DecBytes - 1) / ImagePitch + 1) * ImagePitch;
            DecBytes = (mt_u32)(pDecPos - pDecData);
            MaskIdx   = 0;
            pRlePos  += 2;
            RleBytes += 2;
            continue;
        }

        if (1 == pRlePos[1])
        {
            /*end-of-rledata-Marker*/
            break;
        }

        if (2 == pRlePos[1])
        {
            /*delta Marker*/
            pDecPos  = pDecData + (DecBytes / ImagePitch + pRlePos[4]) * ImagePitch + pRlePos[3];
            DecBytes = (mt_u32)(pDecPos - pDecData);
            MaskIdx   = 0;
            pRlePos  += 4;
            RleBytes += 4;
            continue;
        }
        else /*unencode runs*/
        {
            for (i = 0; i < pRlePos[1]; i++)
            {
                pDecPos[0] = pDecPos[0] | (pRlePos[2 + i / 2] & Mask[MaskIdx]);
                MaskIdx = 1 - MaskIdx;
                if (MaskIdx == 0)
                {
                    pDecPos++;
                    DecBytes++;
                }
            }

            pRlePos  = pRlePos + ((pRlePos[1] * 4 + 15) / 16) * 2 + 2;
            RleBytes = RleBytes + (mt_u32)((pRlePos[1] * 4 + 15) / 16) * 2 + 2;
            continue;
        }
    }

OUT1:
    MTGO_Free(pRleData);
    pRleData = NULL;

    return ret;
} /*lint !e818 */
#endif

static mt_s32 BMP_NoRle_Decode(const BMP_DECODER_S *pBmpDec, mt_u8 *pDecData)
{
    mt_s32 ret;
    mt_u32 CopyLen = 0;
    MT_BOOL EndFlag;
    mt_u32 uNeedReadSize;
    mt_s32 sHeight;
    //struct timeval Begin,End;
    //mt_u32 EscapeTime;

    ret = MTGO_ADP_IOSeek(pBmpDec->hStream, IO_POS_SET, (mt_s32)(pBmpDec->FileHeader.bfOffBits));
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }
    uNeedReadSize = pBmpDec->InfoHeader.biSizeImage;
    if((0 == uNeedReadSize)  || (uNeedReadSize < (pBmpDec->FileHeader.bfSize >> 1)))
    {
        sHeight = pBmpDec->InfoHeader.biHeight;
        if(0 > sHeight)
            sHeight = -sHeight;
        uNeedReadSize = (mt_u32)((pBmpDec->InfoHeader.biWidth * sHeight * pBmpDec->InfoHeader.biBitCount)/8);
    }
    //gettimeofday(&Begin,NULL);
    ret = MTGO_ADP_IORead(pBmpDec->hStream, pDecData, uNeedReadSize, &CopyLen, &EndFlag);
    if ((MT_SUCCESS != ret) || (CopyLen < pBmpDec->InfoHeader.biSizeImage))
    {
        MTGO_ERROR(ret);
        return ret;
    }

    //gettimeofday(&End,NULL);
    //EscapeTime = (End.tv_sec - Begin.tv_sec) * 1000 + End.tv_usec / 1000 - Begin.tv_usec / 1000;
    //printf("read data time:%u ms\n",EscapeTime);

    return MT_SUCCESS;
} /*lint !e818 */

static mt_s32 BMP_ParseFileHeader(IO_HANDLE hStream, MTGO_BITMAPFILEHEADER *pFileHeader)
{
    mt_s32 ret;
    mt_u8 Bytes[4] = {0};
    mt_u32 CopyLen = 0, Reserved;
    MT_BOOL EndFlag;

    /** FileType */
    ret = MTGO_ADP_IORead(hStream, Bytes, 2, &CopyLen, &EndFlag);
    if ((MT_SUCCESS != ret) || (CopyLen < 2))
    {
        MTGO_ERROR(MT_FAILURE);   
        return MT_FAILURE;
    }

    pFileHeader->bfType = (mt_u16)(Bytes[0] | (Bytes[1] << 8));

    /** FileSize */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if ((MT_SUCCESS != ret) || (CopyLen < BMP_DWORD_LEN))
    {
        MTGO_ERROR(MT_FAILURE);     
        return MT_FAILURE;
    }

    pFileHeader->bfSize = (mt_u32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));

    /** Reserver Validation */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if ((MT_SUCCESS != ret) || (CopyLen < BMP_DWORD_LEN))
    {
        MTGO_ERROR(MT_FAILURE);      
        return MT_FAILURE;
    }

    Reserved = (mt_u32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));
    if (0 != Reserved)
    {
        MTGO_ERROR(MT_FAILURE);      
        return MT_FAILURE;
    }

    pFileHeader->bfReserved1 = pFileHeader->bfReserved2 = 0;

    /** OffBits */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if ((MT_SUCCESS != ret) || (CopyLen < BMP_DWORD_LEN))
    {
        MTGO_ERROR(MT_FAILURE);      
        return MT_FAILURE;
    }

    pFileHeader->bfOffBits = (mt_u32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));

    return MT_SUCCESS;
}

static mt_s32 BMP_ParseInfoHeader(IO_HANDLE hStream, MTGO_BITMAPINFOHEADER *pInfoHeader)
{
    mt_s32 ret;
    mt_u8 Bytes[4] = {0};
    mt_u32 CopyLen;
    MT_BOOL EndFlag;

    /** We can use the way of check "HeaderSize" to know bmp version  */

    /** 0x0Ch - BMP Version 2(Window2.x OS/2 1.x)
         0x28h - BMP Version 3(Windows3.1, NT)
         0xF0h - OS/2 2.x
         0x6C - BMP Version 4(Windows 95)*/
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);   
        return MT_FAILURE;
    }

    pInfoHeader->biSize = (mt_u32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));
#if 0 //fix bug 122257   
    if (pInfoHeader->biSize != 0x28) /** only support bmp Version 3 */
    {
        MTGO_ERROR(MT_FAILURE);   
        return MT_FAILURE;
    }
#endif
    /** ImageWidth */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);      
        return MT_FAILURE;
    }

    pInfoHeader->biWidth = (mt_s32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));

    /** ImageHeight*/
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);   
        return MT_FAILURE;
    }

    pInfoHeader->biHeight = (mt_s32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));
    //BM_TRACE("BMP height:%d, 0x%x\n", pInfoHeader->biHeight, pInfoHeader->biHeight);

    /** Planes always is 1 */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_WORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);       
        return MT_FAILURE;
    }

    pInfoHeader->biPlanes = (mt_u16)(Bytes[0] | (Bytes[1] << 8));
    if (1 != pInfoHeader->biPlanes)
    {
        MTGO_ERROR(MT_FAILURE);      
        return MT_FAILURE;
    }

    /** BitCount */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_WORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);      
        return MT_FAILURE;
    }

    pInfoHeader->biBitCount = (mt_u16)(Bytes[0] | (Bytes[1] << 8));

    /** Compression 0-no compress 1- RLE 8  2-RLE 4 3-Bitfields */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);       
        return MT_FAILURE;
    }

    pInfoHeader->biCompression = (mt_u32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));

    /** ImageSize */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);     
        return MT_FAILURE;
    }

    pInfoHeader->biSizeImage = (mt_u32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));

    /** HResolution */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);     
        return MT_FAILURE;
    }

    pInfoHeader->biXPelsPerMeter = (mt_s32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));

    /** VResolution */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);     
        return MT_FAILURE;
    }

    pInfoHeader->biYPelsPerMeter = (mt_s32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));

    /** Colors */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);      
        return MT_FAILURE;
    }

    pInfoHeader->biClrUsed = (mt_u32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));

    /** Colors Important */
    ret = MTGO_ADP_IORead(hStream, Bytes, BMP_DWORD_LEN, &CopyLen, &EndFlag);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);     
        return MT_FAILURE;
    }

    pInfoHeader->biClrImportant = (mt_u32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | (Bytes[3] << 24));

    return MT_SUCCESS;
}

static mt_s32 BMP_ParseFile(IO_HANDLE hStream, BMP_DECODER_S *pDecoder)
{
    mt_s32 ret;

    /** hStream */
    pDecoder->hStream = hStream;

    /** read FileHeader */
    ret = BMP_ParseFileHeader(hStream, &(pDecoder->FileHeader));
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);   
        return MT_FAILURE;
    }

    /** read InfoHeader */
    ret = BMP_ParseInfoHeader(hStream, &(pDecoder->InfoHeader));
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);   
        return MT_FAILURE;
    }
    /**get rle data*/
   ret = MTGO_ADP_IOSeek(hStream, IO_POS_SET, (mt_s32)(pDecoder->FileHeader.bfOffBits));
   if (MT_SUCCESS != ret)
   {  
      MTGO_ERROR(MT_FAILURE);   
      return MT_FAILURE;  
   }
   pDecoder->pDibData = MTGO_ADP_IOGetCurrentPosAddr(hStream);
   if (pDecoder->pDibData == NULL)
   {  
      MTGO_ERROR(MT_FAILURE);   
      return MT_FAILURE;  
   }
    return MT_SUCCESS;
}

/** get color palette  */
static mt_s32 BMP_GetPalette(const BMP_DECODER_S *pBmpDec, MT_PALETTE Palette)
{
    mt_s32 ret;
    mt_u32 ColorUsed, i;
    mt_u8 Bytes[4] = {0};
    mt_u32 CopyLen = 0;
    MT_BOOL EndFlag;

    if (pBmpDec->InfoHeader.biBitCount > 8)
    {
        return MT_SUCCESS;
    }

    if (0 == pBmpDec->InfoHeader.biClrUsed)
    {
        ColorUsed = (mt_u32)(1 << pBmpDec->InfoHeader.biBitCount);
    }
    else
    {
        ColorUsed = pBmpDec->InfoHeader.biClrUsed;
    }

    ret = MTGO_ADP_IOSeek(pBmpDec->hStream, IO_POS_SET,
                          (mt_s32)(pBmpDec->InfoHeader.biSize + BMP_FILEHEADER_LEN));
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);   
        return MT_FAILURE;
    }

    for (i = 0; i < ColorUsed; i++)
    {
        ret = MTGO_ADP_IORead(pBmpDec->hStream, Bytes, 4, &CopyLen, &EndFlag);
        if ((MT_SUCCESS != ret) || (CopyLen < 4))
        {
            MTGO_ERROR(MT_FAILURE);   
            return MT_FAILURE;
        }

        /** Bytes[4] is resver,   we extend it use for  Alpha and set it 0xFF */
        Palette[i] = (mt_u32)(Bytes[0] | (Bytes[1] << 8) | (Bytes[2] << 16) | ((mt_u32)0xFF << 24));
    }

    return MT_SUCCESS;
} /*lint !e818 */

mt_s32 BMP_CreateDecoder(DEC_HANDLE *pBmpDec, const MTGO_DEC_ATTR_S *pSrcDesc)
{
    mt_s32 ret;
    IO_HANDLE hStream = 0;
    IO_DESC_S stIODesc;
    BMP_DECODER_S *pBmpDecoder;

    /**allocate memory for decoder instance  */
    pBmpDecoder = (BMP_DECODER_S*)MTGO_Malloc(sizeof(BMP_DECODER_S));
    if (MT_NULL == pBmpDecoder)
    {
        MTGO_ERROR(MTGO_ERR_NOMEM);   
        return MTGO_ERR_NOMEM;
    }

    MTGO_MemSet(pBmpDecoder, 0, sizeof(BMP_DECODER_S));

    /**create bmp file handle */
#ifdef TEST_IN_ROOTBOX
    stIODesc.Type = (IO_TYPE_E)pSrcDesc->SrcType;
#endif
    stIODesc.IoInfo.MemInfo.pAddr  = pSrcDesc->SrcInfo.MemInfo.pAddr;
    stIODesc.IoInfo.MemInfo.Length = pSrcDesc->SrcInfo.MemInfo.Length;
#ifdef TEST_IN_ROOTBOX
    stIODesc.IoInfo.pFileName = (const mt_void *)pSrcDesc->SrcInfo.pFileName;
#endif
    ret = MTGO_ADP_IOCreate(&hStream, &stIODesc);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);  
        goto ERR1;
    }
    /** parser bmp file, and inital decoder */
    ret = BMP_ParseFile(hStream, pBmpDecoder);
    if (MT_SUCCESS != ret)
    {
        goto ERR2;
    }

    *pBmpDec = (DEC_HANDLE)pBmpDecoder;
    return MT_SUCCESS;
ERR2:
    ret |= MTGO_ADP_IODestroy(hStream);

ERR1:
    if (MT_NULL != pBmpDecoder)
    {
        MTGO_Free(pBmpDecoder);
        pBmpDecoder = MT_NULL;
    }

    return ret;
} /*lint !e818 */

mt_s32 BMP_DestroyDecoder(DEC_HANDLE BmpDec)
{
    mt_s32 ret;
    BMP_DECODER_S *pBmpDecoder = (BMP_DECODER_S*)BmpDec;

    MTGO_ASSERT(MT_NULL != pBmpDecoder);

    /** close IO Stream */
    ret = MTGO_ADP_IODestroy(pBmpDecoder->hStream);

    /** free Decoder */
    MTGO_Free(pBmpDecoder);
    pBmpDecoder = MT_NULL;

    return ret;
}

mt_s32 BMP_ResetDecoder(DEC_HANDLE BmpDec)
{
    return MT_SUCCESS;
}

mt_s32 BMP_DecCommInfo(DEC_HANDLE BmpDec, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo)
{
    BMP_DECODER_S *pBmpDec = (BMP_DECODER_S*)BmpDec;

    MTGO_ASSERT(MT_NULL != pBmpDec);
    MTGO_ASSERT(MT_NULL != pPrimaryInfo);

    pPrimaryInfo->Count     = 1;
    pPrimaryInfo->ImgType   = MTGO_DEC_IMGTYPE_BMP;
    pPrimaryInfo->ScrWidth  = (mt_u32)(pBmpDec->InfoHeader.biWidth);
    if(pBmpDec->InfoHeader.biHeight < 0)
        pPrimaryInfo->ScrHeight = (mt_u32)(-pBmpDec->InfoHeader.biHeight);
    else
        pPrimaryInfo->ScrHeight = (mt_u32)(pBmpDec->InfoHeader.biHeight);
    pPrimaryInfo->IsHaveBGColor = MT_FALSE;

    return MT_SUCCESS;
}

#define IS_565_ColorMask(Mask) \
    (0x1F == Mask[8] && 0x00 == Mask[9] && 0x00 == Mask[10] && 0x00 == Mask[11] \
     && 0xE0 == Mask[4] && 0x07 == Mask[5] && 0x00 == Mask[6] && 0x00 == Mask[7] \
     && 0x00 == Mask[0] && 0xF8 == Mask[1] && 0x00 == Mask[2] && 0x00 == Mask[3])
#define IS_555_ColorMask(Mask) \
    (0x1F == Mask[8] && 0x00 == Mask[9] && 0x00 == Mask[10] && 0x00 == Mask[11] \
     && 0xE0 == Mask[4] && 0x03 == Mask[5] && 0x00 == Mask[6] && 0x00 == Mask[7] \
     && 0x00 == Mask[0] && 0x7C == Mask[1] && 0x00 == Mask[2] && 0x00 == Mask[3])
#define IS_888_ColorMask(Mask) \
    (0xFF == Mask[8] && 0x00 == Mask[9] && 0x00 == Mask[10] && 0x00 == Mask[11] \
     && 0x00 == Mask[4] && 0xFF == Mask[5] && 0x00 == Mask[6] && 0x00 == Mask[7] \
     && 0x00 == Mask[0] && 0x00 == Mask[1] && 0xFF == Mask[2] && 0x00 == Mask[3])

/** 16bit,  32bit use in Windows NT */
/** A Windown NT BMP file will always have a Compression value of 3 */
static MTGO_PF_E PixFmt_Of_NtBmp(const BMP_DECODER_S *pBmpDec)
{
    mt_s32 ret;
    mt_u8 Bytes[12] = {0};
    mt_u32 CopyLen = 0;
    MT_BOOL EndFlag;

    ret = MTGO_ADP_IOSeek(pBmpDec->hStream, IO_POS_SET,
                          (mt_s32)(pBmpDec->InfoHeader.biSize + BMP_FILEHEADER_LEN));
    if (MT_SUCCESS != ret)
    { 
        return MTGO_PF_BUTT;
    }

    ret = MTGO_ADP_IORead(pBmpDec->hStream, Bytes, 12, &CopyLen, &EndFlag);
    if ((MT_SUCCESS != ret) || (CopyLen < 3))
    {
        return MTGO_PF_BUTT;
    }

    /** special case  */
    if (pBmpDec->InfoHeader.biCompression == 0 || pBmpDec->InfoHeader.biCompression == 3)
    {
        if (16 == pBmpDec->InfoHeader.biBitCount)
        {
            return MTGO_PF_0555;
        }
        else if (32 == pBmpDec->InfoHeader.biBitCount)
        {
#ifdef RGB24
            return MTGO_PF_8888;
#else
            return MTGO_PF_0888;
#endif
        }
    }

    /** 565 */
    if (IS_565_ColorMask(Bytes))
    {
        return MTGO_PF_565;
    }

    /** 555 */
    if (IS_555_ColorMask(Bytes))
    {
        return MTGO_PF_0555;
    }

    /** 888 */
    if (IS_888_ColorMask(Bytes))
    {
#ifdef RGB24
                    return MTGO_PF_8888;
#else
                    return MTGO_PF_0888;
#endif

    }

    /** unsupported this version */
    return MTGO_PF_BUTT;
} /*lint !e818 */

// TODO:BMP how to get bmp pixel format?
static mt_void BMP_GetPixelFmt(const BMP_DECODER_S *pBmpDec, MTGO_PF_E *pFormat)
{
    MTGO_ASSERT(MT_NULL != pBmpDec);
    MTGO_ASSERT(MT_NULL != pFormat);

    switch (pBmpDec->InfoHeader.biBitCount)
    {
        /** Window 3.x BMP file */
    case 1:
        *pFormat = MTGO_PF_CLUT1;
        break;
    case 4:
        *pFormat = MTGO_PF_CLUT4;
        break;
    case 8:
        *pFormat = MTGO_PF_CLUT8;
        break;
    case 24:
        *pFormat = MTGO_PF_0888;
        break;

        /** Window NT BMP file */
    case 16:
    case 32:
        *pFormat = PixFmt_Of_NtBmp(pBmpDec);
        break;
    default:
        *pFormat = MTGO_PF_BUTT;
    }

    return;
}

mt_s32 BMP_DecImgInfo(DEC_HANDLE BmpDec, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo)
{
    BMP_DECODER_S *pBmpDec = (BMP_DECODER_S*)BmpDec;

    MTGO_ASSERT(MT_NULL != pBmpDec);
    MTGO_ASSERT(MT_NULL != pImgInfo);
    pImgInfo->OffSetX = pImgInfo->OffSetY = 0;
    pImgInfo->Width  = (mt_u32)pBmpDec->InfoHeader.biWidth;
    pImgInfo->Height = (mt_u32)(pBmpDec->InfoHeader.biHeight);
    if(pBmpDec->InfoHeader.biHeight<0)
        pImgInfo->Height = (mt_u32)(-pBmpDec->InfoHeader.biHeight);
    else
        pImgInfo->Height = (mt_u32)(pBmpDec->InfoHeader.biHeight);
    pImgInfo->Alpha = 255;
    pImgInfo->IsHaveKey = MT_FALSE;
    BMP_GetPixelFmt(pBmpDec, &(pImgInfo->Format));
    pImgInfo->DelayTime = 0;
    pImgInfo->DisposalMethod = 0;

    return MT_SUCCESS;
}

mt_s32 BMP_DecImgData(DEC_HANDLE BmpDec, mt_u32 Index, MTGO_SURFACE_S *pSurface)
{
    mt_s32 ret;
    mt_u32 i, j, DstPitch, SrcPitch, Width, Height;
    mt_u8 *pDstData = NULL, *pDstLine = NULL, *pDstPos = NULL, *pSrcData = NULL, *pSrcLine = NULL, *pSrcPos = NULL;
    MTGO_PF_E PixFmt = MTGO_PF_BUTT;
    BMP_DECODER_S *pBmpDec = (BMP_DECODER_S*)BmpDec;
	mt_u32 SrcLen = 0 ;
    MT_PIXELDATA pData = {0};
    MT_PALETTE Palatte = {0};
    phys_addr_t  phyAddr = 0;
   phys_addr_t  dec_phyAddr = 0;
   mt_u8 *p_dec_buf = NULL;   
   u8 * p_tmp_src = NULL;
   u32 dec_pitch = 0;
   
   float fyscale =  0.0;
   float fYInverse = 0.0;
   u32 y = 0;
   u32 b_order = 0;
   u32 is_RGB565 = 0;
    
    MTGO_ASSERT(MT_NULL != pBmpDec);
 
    /** allocate memory in MMZ */
    Width  = (mt_u32)(pBmpDec->InfoHeader.biWidth);
    if(pBmpDec->InfoHeader.biHeight < 0)
    {
        Height = (mt_u32)(-pBmpDec->InfoHeader.biHeight);
        b_order = 1;
    }
    else
         Height = (mt_u32)(pBmpDec->InfoHeader.biHeight);
    
    BMP_GetPixelFmt(pBmpDec, &PixFmt);
    if(MTGO_PF_BUTT == PixFmt)
    {
        MTGO_ERROR(MT_FAILURE); 
        return MT_FAILURE;
    }
    is_RGB565 = (PixFmt == MTGO_PF_565)? 1 : 0;
	/** allocate memory, and make sure TDE has finish */
    //(mt_void)MTGO_ADP_GfxSync();
//lint -e539
	(mt_void)Surface_LockSurface((MTGO_HANDLE)pSurface, pData);
    DstPitch = pData[0].Pitch;
    pDstData = (mt_u8*)pData[0].pData;
	(mt_void)Surface_UnlockSurface((MTGO_HANDLE)pSurface);
//lint +e539	
    SrcPitch = ((Width * pBmpDec->InfoHeader.biBitCount + 31) / 32) * 4;
    SrcLen = MTGO_MAX((Height * SrcPitch),(pBmpDec->InfoHeader.biSizeImage));

    if(((pBmpDec->InfoHeader.biWidth != pSurface->Width) || (Height != pSurface->Height)) && (pBmpDec->InfoHeader.biBitCount > 8))
    {
        dec_pitch = (pBmpDec->InfoHeader.biWidth + 1)/2 * 2 * 4;
        p_dec_buf = (mt_u8*)MTGO_MMZ_Malloc(dec_pitch * 1, &dec_phyAddr);
        if (MT_NULL == p_dec_buf)
        {
            MTGO_ERROR(MTGO_ERR_NOMEM);
            return MTGO_ERR_NOMEM;
        }
        fyscale =  (float)Height /(float)pSurface->Height;   
        for(i = 0; i <  pSurface->Height; i++)   
        {
            fYInverse = i * fyscale;
            y = (int)fYInverse;

            if(! b_order)
                p_tmp_src = pBmpDec->pDibData + SrcPitch * (Height - y - 1);
            else
                p_tmp_src = pBmpDec->pDibData + SrcPitch * y;
            get_truec_line_data_step(p_tmp_src,
                pBmpDec->InfoHeader.biWidth,
                p_dec_buf,
                pBmpDec->InfoHeader.biWidth,
                1,
                pBmpDec->InfoHeader.biBitCount,
                is_RGB565);
            scale_rgba(p_dec_buf, 0,  0,
                pBmpDec->InfoHeader.biWidth, 1,
                pBmpDec->InfoHeader.biWidth, 1,
                pDstData + i * DstPitch,
                pSurface->Width,
                1);

        }
    }
    else
    {
        pSrcData = (mt_u8*)MTGO_MMZ_Malloc(SrcLen, &phyAddr);
        if (MT_NULL == pSrcData)
        {
            MTGO_ERROR(MTGO_ERR_NOMEM);
            return MTGO_ERR_NOMEM;
        }

        
        MTGO_MemSet(pDstData, 0, pSurface->Height * DstPitch);
        MTGO_MemSet(pSrcData, 0, SrcLen);
        /** get image data */
#ifndef MTGO_CODE_CUT    
        if (1 == pBmpDec->InfoHeader.biCompression)
        {
            ret = BMP_Rle8_Decode(pBmpDec, pSrcData);
        }
        else if (2 == pBmpDec->InfoHeader.biCompression)
        {
            ret = BMP_Rle4_Decode(pBmpDec, pSrcData);
        }
        else
#endif        
        {
            ret = BMP_NoRle_Decode(pBmpDec, pSrcData);
        }

        if (MT_SUCCESS != ret)
        {
            MTGO_ERROR(ret);
            goto ERR2;
        }
        //printf ("bit count:%d, PixFmt:%d\n", pBmpDec->InfoHeader.biBitCount, PixFmt);
        /**transfer to targe image data */
        for (i = 0; i < Height; i++)
        {
            /** calculate tarte address */
            if (!b_order)  
            {
                pDstLine = pDstData + ((Height - 1) - i) * DstPitch;
            }
            else
            {
                pDstLine = pDstData +  i * DstPitch;
            }

            /** calculate source address */
            pSrcLine = pSrcData + i * SrcPitch;

            /** 24bit Bmp SrcPitch and DstPitch is not the same */
#ifdef RGB24
            /**refine the target, and do few copy*/
            if (pBmpDec->InfoHeader.biBitCount == 32)
            {
                pSrcPos = pSrcLine;
                pDstPos = pDstLine;
                for (j = 0; j < Width; j++)
                {
                    MTGO_MemCopy(pDstPos, pSrcPos, 4);
                    pDstPos[3] = 0xff;
                    pDstPos += 4;
                    pSrcPos += 4;
                }
            }
            else
            {  
                MTGO_MemCopy(pDstLine, pSrcLine, SrcPitch);        
            }
#else
            if (pBmpDec->InfoHeader.biBitCount == 24)
            {
                mt_u32 ttt = 0;
                pSrcPos = pSrcLine;
                pDstPos = pDstLine;
                for (j = 0; j < Width; j++)
                {
                    MTGO_MemCopy(pDstPos, pSrcPos, 3);
                    pDstPos[3] = 0;
                    pDstPos += 4;
                    ttt += 4;
                    pSrcPos += 3;
                }
            }
            else
            {
                MTGO_MemCopy(pDstLine, pSrcLine, SrcPitch);
            }
#endif
        }
    }
    /** fill data to color palette */
    if (pBmpDec->InfoHeader.biBitCount <= 8)
    {
        ret = BMP_GetPalette(pBmpDec, Palatte);
        if (MT_SUCCESS != ret)
        {
            MTGO_ERROR(ret);
            goto ERR2;
        }
        (mt_void)Surface_SetSurfacePalette((MTGO_HANDLE)pSurface, Palatte);
    }
    
  if(p_dec_buf)
	MTGO_MMZ_Free(p_dec_buf);
    if(pSrcData)
        MTGO_MMZ_Free(pSrcData);

    return MT_SUCCESS;

ERR2:
    if(p_dec_buf)
	MTGO_MMZ_Free(p_dec_buf);
    
    if(pSrcData)
        MTGO_MMZ_Free(pSrcData);
    return MT_FAILURE;
}

#endif
#ifdef __cplusplus
}
#endif


