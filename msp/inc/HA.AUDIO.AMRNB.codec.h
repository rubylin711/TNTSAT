/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_CODEC_AMRNB_H__
#define __MTSI_AUDIO_CODEC_AMRNB_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define    HA_AMRNB_ID 0x0100        


#ifndef AMR_MAGIC_NUMBER
 #define AMR_MAGIC_NUMBER "#!AMR\n"/*magic number for AMR-NB*/
#endif

typedef enum
{
    AMRNB_MR475 = 0,     /*4.75kbit/s*/
    AMRNB_MR515,
    AMRNB_MR59,
    AMRNB_MR67,
    AMRNB_MR74,
    AMRNB_MR795,
    AMRNB_MR102,
    AMRNB_MR122,
    AMRNB_MRDTX,             /*SID mode*/
    AMRNB_N_MODES                /* number of (SPC) modes */
} AMRNB_MODE_E;

typedef enum
{
    AMRNB_MIME = 0,     /*4.75kbit/s*/
    AMRNB_IF1,
    AMRNB_IF2,
} AMRNB_FORMAT_E;

typedef struct
{
    AMRNB_FORMAT_E enFormat;

    AMRNB_MODE_E enMode;

    MT_BOOL bDTX;  /* MT_TRUE=enable dtx, MT_FALSE=disable dtx  */
} AMRNB_ENCODE_OPENCONFIG_S;

typedef struct
{
    AMRNB_FORMAT_E enFormat;
} AMRNB_DECODE_OPENCONFIG_S;

typedef enum
{
    AMRNB_CONFIGCMD_MODE = 0,
}  AMRNB_CONFIG_COMMAND_E;

/* struct for 
    MT_HA_ERRORTYPE_E (*EncodeSetConfig)(mt_void * hEncoder, mt_void *pstConfigStructure);
*/
typedef struct
{
    AMRNB_CONFIG_COMMAND_E enCmd;

    mt_void *pstPrivateParams;
} AMRNB_ENCODE_CONFIG_MODE_S;

#define HA_AMRNB_GetEncDefaultOpenParam(pOpenParam, pstPrivateParams) \
do{ ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32DesiredOutChannels = 1; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->bInterleaved  = MT_TRUE; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->s32BitPerSample = 16; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32DesiredSampleRate = 8000; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32SamplePerFrame = 160; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(AMRNB_ENCODE_OPENCONFIG_S); \
}while(0)

#define HA_AMRNB_GetDecDefalutOpenParam(pOpenParam, pstPrivateParams) \
do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_RAWPCM; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 1; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_TRUE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 8000; \
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
    { \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
    } \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(AMRNB_DECODE_OPENCONFIG_S); \
}while(0)

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_CODEC_AMRNB_H__ */
