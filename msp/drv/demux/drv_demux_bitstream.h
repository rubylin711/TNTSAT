/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _PVR_BITSTREAM_HEAD_
#define	_PVR_BITSTREAM_HEAD_

#ifdef __cplusplus
extern "C" {
#endif

#include "mt_type.h"

typedef struct _PVR_BITSTREAM{
	mt_u8	*pHead;
	mt_u8	*pTail;
	mt_u32  Bufa;
	mt_u32  Bufb;
	mt_s32	BsLen;
	mt_s32  BufPos;
	mt_s32  TotalPos;
}PVR_BS, *LP_PVR_BS;


mt_void PvrBsInit( PVR_BS *pBS, mt_u8 *pInput, mt_s32 length );
mt_s32  PvrBsGet( PVR_BS *pBS, mt_s32 nBits );
mt_s32  PvrBsShow( PVR_BS *pBS, mt_s32 nBits );
mt_s32  PvrBsPos( PVR_BS *pBS );
mt_s32  PvrBsSkip( PVR_BS *pBS, mt_s32 nBits );
mt_s32  PvrBsBack( PVR_BS *pBS, mt_s32 nBits );
mt_s32  PvrBsToNextByte( PVR_BS *pBS );
mt_s32  PvrBsBitsToNextByte( PVR_BS *pBS );
mt_s32  PvrBsResidBits( PVR_BS *pBS );
mt_s32  PvrBsIsByteAligned( PVR_BS *pBS );
mt_s32  PvrBsNextBitsByteAligned( PVR_BS *pBS, mt_s32 nBits );
mt_s32  PvrBsLongSkip(PVR_BS *pBS, mt_s32 nBits);
mt_s32  PvrBsSkipWithoutCount( PVR_BS *pBS, mt_s32 nBits );
mt_u8*  PvrBsGetNextBytePtr( PVR_BS *pBS );
mt_u32  PvrZerosMS_32(mt_u32 data);


#ifdef __cplusplus
}
#endif

#endif

