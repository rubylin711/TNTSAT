/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTSI_AUDIO_CODEC_AMRWB_H__
#define __MTSI_AUDIO_CODEC_AMRWB_H__

#include "mt_type.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define    HA_AMRWB_ID 0x0110

#ifndef AMRWB_MAGIC_NUMBER
 #define AMRWB_MAGIC_NUMBER "#!AMR-WB\n" /*magic number for AMR-WB*/
#endif

#if 1
typedef enum
{
    AMRWB_MR66 = 0,                /* AMR-WB 6.60 kbit/s  */
    AMRWB_MR885,                    /* AMR-WB 8.85 kbit/s  */
    AMRWB_MR1265,                  /* AMR-WB 12.65 kbit/s */
    AMRWB_MR1425,                  /* AMR-WB 14.25 kbit/s */
    AMRWB_MR1585,                  /* AMR-WB 15.85 kbit/s */
    AMRWB_MR1825,                  /* AMR-WB 18.25 kbit/s */
    AMRWB_MR1985,                  /* AMR-WB 19.85 kbit/s */
    AMRWB_MR2305,                  /* AMR-WB 23.05 kbit/s */
    AMRWB_MR2385                   /* AMR-WB 23.85 kbit/s */
}AMRWB_MODE_E;
#endif

typedef enum
{
    AMRWB_FORMAT_MIME, 
    AMRWB_FORMAT_IF2
} AMRWB_FORMAT_E;

typedef struct
{
    AMRWB_FORMAT_E enFormat;

    AMRWB_MODE_E enMode;

    MT_BOOL bDTX;  /* MT_TRUE=enable dtx, MT_FALSE=disable dtx  */
} AMRWB_ENCODE_OPENCONFIG_S;

typedef struct
{
    AMRWB_FORMAT_E enFormat;
} AMRWB_DECODE_OPENCONFIG_S;

typedef enum
{
    AMRWB_CONFIGCMD_MODE = 0,
}  AMRWB_CONFIG_COMMAND_E;

/* struct for 
    MT_HA_ERRORTYPE_E (*EncodeSetConfig)(mt_void * hEncoder, mt_void *pstConfigStructure);
*/
typedef struct
{
    AMRWB_CONFIG_COMMAND_E enCmd;

    mt_void *pstPrivateParams;
} AMRWB_ENCODE_CONFIG_MODE_S;

#define HA_AMRWB_GetEncDefaultOpenParam(pOpenParam, pstPrivateParams) \
do{ ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32DesiredOutChannels = 1; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->bInterleaved  = MT_TRUE; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->s32BitPerSample = 16; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32DesiredSampleRate = 16000; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32SamplePerFrame = 320; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HAENCODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(AMRWB_ENCODE_OPENCONFIG_S); \
}while(0)

#define HA_AMRWB_GetDecDefalutOpenParam(pOpenParam, pstPrivateParams) \
do{ mt_s32 i; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->enDecMode = HD_DEC_MODE_RAWPCM; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredOutChannels = 1; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.bInterleaved  = MT_TRUE; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32BitPerSample = 16; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.u32DesiredSampleRate = 16000; \
    for (i = 0; i < HA_AUDIO_MAXCHANNELS; i++) \
    { \
        ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->sPcmformat.enChannelMapping[i] = HA_AUDIO_ChannelNone; \
    } \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->pCodecPrivateData = (mt_void*)pstPrivateParams; \
    ((MT_HADECODE_OPENPARAM_S *)(pOpenParam))->u32CodecPrivateDataSize = sizeof(AMRWB_DECODE_OPENCONFIG_S); \
}while(0)

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MTSI_AUDIO_CODEC_AMRWB_H__ */
