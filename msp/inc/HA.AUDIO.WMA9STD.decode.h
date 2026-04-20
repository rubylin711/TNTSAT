/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_WMA_H__
#define __MTSI_AUDIO_DECODER_WMA_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HA_WMA9STD_ID 0x0006        

typedef struct hiWMA_FORMAT_S
{
    mt_u16 wFormatTag;          /* format type,0x160->WMAV1,0x161->WMAV2, 0x162->WMAV3 */
    mt_u16 nChannels;            /* number of channels (i.e. mono, stereo...) */
    mt_u32 nSamplesPerSec;   /* sample rate */
    mt_u32 nAvgBytesPerSec;  /* for buffer estimation */
    mt_u16 nBlockAlign;          /* block size of data */
    mt_u16 wBitsPerSample;   /* number of bits per sample of mono data */
    mt_u16 cbSize;                /* the count in bytes of the size of */
    mt_u16 cbExtWord[16];       /* extra information (after cbSize).
                                WMAV1: need  4 Bytes extra information at least
                                    WMAV2: need 10 Bytes extra information at least
                                WMAV3: need 18 Bytes extra information at least
                                 */
} WMA_FORMAT_S;

#define HA_WMA_DecGetDefalutOpenParam(pOpenParam, pstPrivateParams) \
do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_RAWPCM; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_FALSE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
    { \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
    } \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(WMA_FORMAT_S); \
}while(0)

/*=====================================================================
	CMP demux, only use for debug
=====================================================================*/

#if 1
#define HA_WMA_ParseCmp(pfcmp, pstWfx) \
({ mt_s32 nRead; \
    nRead=0; \
    fread(&(((WMA_FORMAT_S *)pstWfx)->wFormatTag), sizeof(mt_u16), 1, (FILE *)pfcmp); \
    fread(&(((WMA_FORMAT_S *)pstWfx)->nChannels), sizeof(mt_u16), 1, (FILE *)pfcmp); \
    fread(&(((WMA_FORMAT_S *)pstWfx)->nSamplesPerSec), sizeof(mt_u32), 1, (FILE *)pfcmp); \
    fread(&(((WMA_FORMAT_S *)pstWfx)->nAvgBytesPerSec), sizeof(mt_u32), 1, (FILE *)pfcmp); \
    fread(&(((WMA_FORMAT_S *)pstWfx)->nBlockAlign), sizeof(mt_u16), 1, (FILE *)pfcmp); \
    fread(&(((WMA_FORMAT_S *)pstWfx)->wBitsPerSample), sizeof(mt_u16), 1, (FILE *)pfcmp); \
    fread(&(((WMA_FORMAT_S *)pstWfx)->cbSize), sizeof(mt_u16), 1, (FILE *)pfcmp); \
    nRead += 7*sizeof(mt_u16)+ 2*sizeof(mt_u32); \
    if (((WMA_FORMAT_S *)pstWfx)->wFormatTag != 1) \
    { \
    } \
    if (((WMA_FORMAT_S *)pstWfx)->cbSize == 4) \
    { \
        ((WMA_FORMAT_S *)pstWfx)->wFormatTag = 0x160; \
        fread(&(((WMA_FORMAT_S *)pstWfx)->cbExtWord), 1, ((WMA_FORMAT_S *)pstWfx)->cbSize, (FILE *)pfcmp); \
       	nRead +=((WMA_FORMAT_S *)pstWfx)->cbSize; \
    } \
    else if ((((WMA_FORMAT_S *)pstWfx)->cbSize == 10) || (((WMA_FORMAT_S *)pstWfx)->cbSize == (10 + 22))) \
    { \
        ((WMA_FORMAT_S *)pstWfx)->wFormatTag = 0x161; \
        fread(&(((WMA_FORMAT_S *)pstWfx)->cbExtWord), 1, ((WMA_FORMAT_S *)pstWfx)->cbSize, (FILE *)pfcmp); \
       	nRead +=((WMA_FORMAT_S *)pstWfx)->cbSize; \
    } \
    else \
    { \
    } \
    nRead; \
})

#else
static mt_s32 HA_WMA_ParseCmp(FILE *pfcmp, WMA_FORMAT_S *pstWfx)
{
       mt_s32 nRead=0;

#if 1                                                                 
	fread(&(pstWfx->wFormatTag), sizeof(mt_u16), 1, pfcmp);       
	fread(&(pstWfx->nChannels), sizeof(mt_u16), 1, pfcmp);        
	fread(&(pstWfx->nSamplesPerSec), sizeof(mt_u32), 1, pfcmp);   
	fread(&(pstWfx->nAvgBytesPerSec), sizeof(mt_u32), 1, pfcmp);  
	fread(&(pstWfx->nBlockAlign), sizeof(mt_u16), 1, pfcmp);      
	fread(&(pstWfx->wBitsPerSample), sizeof(mt_u16), 1, pfcmp);   
	fread(&(pstWfx->cbSize), sizeof(mt_u16), 1, pfcmp);           
	nRead += 7*sizeof(mt_u16)+ 2*sizeof(mt_u32);                  
#endif                                                                
	if (pstWfx->wFormatTag != 1) // not v1 or v2
	{
		//HA_PRINT("Support only CMP V1 format wFormatTag=0x%x!\n",pstWfx->wFormatTag);
		return -1;
	}

	if (pstWfx->cbSize == 4)  //g45208
	{
		//WMA v1 bitstream
		pstWfx->wFormatTag = 0x160;
		fread(&pstWfx->cbExtWord, 1, pstWfx->cbSize, pfcmp);
       	nRead +=pstWfx->cbSize;
	}
	else if ((pstWfx->cbSize == 10) || (pstWfx->cbSize == (10 + 22) ) )  //g45208
	{
		//WMA v2/v3 bitstream
		pstWfx->wFormatTag = 0x161;
		fread(&pstWfx->cbExtWord, 1, pstWfx->cbSize, pfcmp);
       	nRead +=pstWfx->cbSize;
	}
	else
	{
		//HA_PRINT("Error cbSize=%d\n",pstWfx->cbSize);
		return -1;
	}
	return nRead;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_WMA_H__ */

