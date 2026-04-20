/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_DTSM6_H__
#define __MTSI_AUDIO_DECODER_DTSM6_H__

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
    MT_BOOL coreOnly;             /**< true: core decode only, false: dts hd decode,default value is false */
    mt_u32  outputBitWidth;       /**< 16: 16bit, 24: 24bit, 0,:native, defalut is 16 */
    mt_u32  DRCPercent;           /* 0~100 - default is 0 ,no DRC */
    MT_BOOL enableHDPassThrough;  /* true: enable , default value is true */
    MT_BOOL enableSPDIFOutput;    /* true: enable , default value is true */
    MT_BOOL enableTransEncode;    /* true: enable , default value is false */
    mt_u32  transEncodeMode;      /* 0,DTSTRANSCODEMODE_5_1_MIXOUT;1,DTSTRANSCODEMODE_FULL_MIXOUT;default 1 */
    MT_BOOL allowUalignWord;      /* true: allow , default value is true */
} DTSM6_DECODE_OPENCONFIG_S;

#define HA_DTSM6_DecGetDefalutOpenConfig(pConfigParam) \
    do {  \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->spkrOut = 2; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->coreOnly = MT_FALSE; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->outputBitWidth  = 24; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->DRCPercent = 0; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->enableHDPassThrough = MT_TRUE; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->enableSPDIFOutput = MT_TRUE; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->enableTransEncode = MT_FALSE; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->transEncodeMode = 1; \
        ((DTSM6_DECODE_OPENCONFIG_S *)(pConfigParam))->allowUalignWord = MT_TRUE; \
    } while (0)

#define HA_DTSM6_DecGetDefalutOpenParam(pOpenParam, pstPrvateConfig) \
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
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(DTSM6_DECODE_OPENCONFIG_S); \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_DTSM6_H__ */
