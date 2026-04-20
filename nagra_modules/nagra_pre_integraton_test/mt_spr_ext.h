/*
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#ifndef __MT_SPR_EXT_H__
#define __MT_SPR_EXT_H__
#include "mt_common.h"
#include "mt_sec_ext.h"
#include "mt_unf_avplay.h"
#include "mt_unf_common.h"
#include "nocs_sec_impl.h"

/*#define USE_DMX_RECORDING	(1)*/

//#define LOG_TO_FILE

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

#define AAC_ADTS_HEADER_SIZE 7

struct transportSession {
	TTransportSessionId tsid;
	TBoolean isClosing; //To mark this session is now closing
	TUnsignedInt8 tsPort; //see mt_unf_demux.h, PORT_RAM_0 ~ 7
	transportSessionType sessionType;
	mt_u16 emi;
	struct list_head listNode;

	mt_u32 playDmxId;
	mt_u32 recordDmxId;
	mt_u32 recordChannelId;
	MT_UNF_AVPLAY_STREAM_TYPE_E avPlayerStreamType;

	mt_handle tsBufHandle;
	mt_handle avPlayer;
	mt_handle audioTrack;

	TBoolean doesVidChanOpen;
	TBoolean doesAudChanOpen;

	MT_UNF_VCODEC_TYPE_E vidCodecType;
       HA_CODEC_ID_E audCodecType;
	mt_u32 vidPid;
	mt_u32 audPid;
	mt_u32 pcrPid;

	TUnsignedInt8 *tsBuffer;
	TUnsignedInt8 *tsBufferPhy;
	TUnsignedInt32 tsBufferSize; //total buffer size
	TUnsignedInt32 tsDataSize; //actual data size

	TUnsignedInt8 *tsIntBufferVir; //intemediate buffer for m2m processing
	TUnsignedInt8 *tsIntBufferPhy; //intemediate buffer for m2m processing

    TBoolean smpSetFlag;    //TODO:
    TBoolean smpEnabled;    //to mark if this session is SMP-ON

    TUnsignedInt8 *vidExtraData;
    TUnsignedInt32 vidExtraDataSize;

	TUnsignedInt8 *vesInjPointer;
	TUnsignedInt32 vesInjSize;
	TUnsignedInt8 *aesInjPointer;
	TUnsignedInt32 aesInjSize;

	TBoolean videoDecodeStart; //TRUE for video is now decoding
	TBoolean audioDecodeStart; //TRUE for audio is now decoding
	TBoolean isVideoPlaying;

	TUnsignedInt8 aacAdtsHeaderBak[AAC_ADTS_HEADER_SIZE];
	TUnsignedInt8 *aacAdtsHeader;

	TUnsignedInt32 descramblingCount;
	TUnsignedInt32 audiodescramblingCount;
	TBoolean bUseSubLayer;
	TUnsignedInt32 ts_opc_err_status_count;
       char rec_file_name[256];
};


#endif
