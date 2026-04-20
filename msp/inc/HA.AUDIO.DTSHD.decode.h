/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_DTSHD_H__
#define __MTSI_AUDIO_DECODER_DTSHD_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


typedef  struct
{
    mt_u32  spkrOut;              /* set as a result of spkrout ,default is 2(Lo/Ro) */
    MT_BOOL enableDownmix; /* default true */
    MT_BOOL coreOnly;       /**< true: core decode only, false: dts hd decode,default value is true */
	MT_BOOL lfeMixedToFrontWhenNoDedicatedLFEOutput;         /**< true: enable lfe,default value is false */    
    mt_u32 outputBitWidth;        /**< 16: 16bit, 24: 24bit, 0,:native, defalut is 24 */
    MT_BOOL enableDialNorm; /* true: enable , default value is true */
    mt_u32  DRCPercent;      /* 0~100 - default is 0 ,no DRC */
    MT_BOOL enableHDPassThrough;       /* true: enable , default value is true */
    mt_u32  PirvateControl;     
} DTSHD_DECODE_OPENCONFIG_S;

#define HA_DTSHD_DecGetDefalutOpenConfig(pConfigParam) \
    do {  \
         ((DTSHD_DECODE_OPENCONFIG_S *)(pConfigParam))->spkrOut = 2; \
         ((DTSHD_DECODE_OPENCONFIG_S *)(pConfigParam))->enableDownmix = MT_TRUE; \
         ((DTSHD_DECODE_OPENCONFIG_S *)(pConfigParam))->coreOnly = MT_TRUE; \
		 ((DTSHD_DECODE_OPENCONFIG_S *)(pConfigParam))->lfeMixedToFrontWhenNoDedicatedLFEOutput = MT_FALSE; \
         ((DTSHD_DECODE_OPENCONFIG_S *)(pConfigParam))->outputBitWidth   = 24; \
         ((DTSHD_DECODE_OPENCONFIG_S *)(pConfigParam))->enableDialNorm  = MT_TRUE; \
         ((DTSHD_DECODE_OPENCONFIG_S *)(pConfigParam))->DRCPercent = 0; \
		((DTSHD_DECODE_OPENCONFIG_S *)(pConfigParam))->enableHDPassThrough = MT_TRUE; \
         ((DTSHD_DECODE_OPENCONFIG_S *)(pConfigParam))->PirvateControl = 0; \
    } while (0)

#define HA_DTSHD_DecGetDefalutOpenParam(pOpenParam, pstPrvateConfig) \
    do { mt_s32 i; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_SIMUL; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved = MT_TRUE; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
         for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
         { \
             ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
         } \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrvateConfig; \
         ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(DTSHD_DECODE_OPENCONFIG_S); \
    } while (0)

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_DTSHD_H__ */
