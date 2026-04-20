/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_DECODER_DOLBY_TRUEHD_H__
#define __MTSI_AUDIO_DECODER_DOLBY_TRUEHD_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */  


typedef enum hiTRUEHD_DRC_STATEMODE_TYPE_E
{
    TRUEHD_DRC_STATEMODE_TYPE_OFF,
    TRUEHD_DRC_STATEMODE_TYPE_FOLLOW,      /**<default*/ /**<Ä¬ÈÏÅäÖÃ */
    TRUEHD_DRC_STATEMODE_TYPE_ON
} TRUEHD_DRC_STATEMODE_TYPE_E;


typedef struct hiTRUEHD_DECODE_OPENCONFIG_S
{
    mt_s32            				startByte ;
    mt_s32           				s32chanflag;
    MT_BOOL           			    fbaChannelOrder;

    MT_BOOL           			    lossless;
    TRUEHD_DRC_STATEMODE_TYPE_E     enDrcMode;
    mt_s32            				drcBoost;
    mt_s32            				drcCut;            
    MT_BOOL           			    verbose;
    mt_s32			  				lenSamp ;
    mt_s32            				lenByte ;
    MT_BOOL           			    archive;
} TRUEHD_DECODE_OPENCONFIG_S;


#define HA_DOLBY_TRUEHD_DecGetDefalutOpenConfig(pConfigParam) \
do { ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->s32chanflag = 0x2; \
     ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->startByte   = 0; \
     ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->lenByte   = 0x7fffffff; \
	 ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->lenSamp   = 0x7fffffff; \
     ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->fbaChannelOrder = MT_FALSE; \
     ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->lossless = MT_TRUE; \
     ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->enDrcMode   = TRUEHD_DRC_STATEMODE_TYPE_FOLLOW; \
     ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->drcBoost  = 100; \
     ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->drcCut = 100; \
     ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->verbose = MT_FALSE; \
     ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->archive = MT_FALSE; \
} while (0)

#define HA_DOLBY_TRUEHD_DecGetDefalutOpenParam(pOpenParam, pstPrivateParams) \
do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_RAWPCM; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_TRUE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 24; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
    { \
        ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
    } \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(TRUEHD_DECODE_OPENCONFIG_S); \
}while(0)

#define HA_DOLBY_CONVERT_DecGetDefalutOpenConfig(pConfigParam) \
    do { ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->s32chanflag = 0x2; \
         ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->startByte   = 0; \
         ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->lenByte   = 0x7fffffff; \
       ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->lenSamp   = 0x7fffffff; \
         ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->fbaChannelOrder = MT_FALSE; \
         ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->lossless = MT_TRUE; \
         ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->enDrcMode   = TRUEHD_DRC_STATEMODE_TYPE_FOLLOW; \
         ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->drcBoost  = 100; \
         ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->drcCut = 100; \
         ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->verbose = MT_FALSE; \
         ((TRUEHD_DECODE_OPENCONFIG_S *)(pConfigParam))->archive = MT_FALSE; \
    } while (0)
    
#define HA_DOLBY_CONVERT_DecGetDefalutOpenParam(pOpenParam, pstPrivateParams) \
    do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_SIMUL; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 2; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_TRUE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 24; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 48000; \
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
    { \
        ((MT_HADECODE_OPENPARAM_S *)pOpenParam)->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
    } \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(TRUEHD_DECODE_OPENCONFIG_S); \
}while(0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_DECODER_TRUEHD_H__ */

