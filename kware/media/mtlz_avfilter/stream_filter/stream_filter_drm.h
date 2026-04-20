
/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __STREAM_FILTER_DRM_H_H__
#define __STREAM_FILTER_DRM_H_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#include "MTDrmApi.h"

/* current only buffer info, named ctx for future expansion */
typedef struct {
    unsigned char *system_id;
    unsigned int system_id_size;
    union {
        MTAVSAudioInfo audio;
        MTAVSVideoInfo video;
    } codec;
    MTDRM_PROTECT_INFO info;
} MTAV_CSFDrmFilterInitInfo;

#define SYSTEM_ID_SIZE_DEFAULT                (16)
#define SYSTEM_ID_SIZE_CONTENT_PROTECTION     (36)

/* ID from ContentProtection  */
#define CLEARKEY_CPRO_SYSTEM_ID             "1077efec-c0b2-4d02-ace3-3c1e52e2fb4b"
#define WIDEVINE_CPRO_SYSTEM_ID             "edef8ba9-79d6-4ace-a3c8-27dcd51d21ed"
#define PLAYREADY_CPRO_SYSTEM_ID            "9a04f079-9840-4286-ab92-e65be0885f95"
#define CHINADRM_CPRO_SYSTEM_ID             "3d5e6d35-9b9a-41e8-b843-dd3c6e72c42c"
#define VERIMATRIX_CPRO_SYSTEM_ID           "9a27dd82-fde2-4725-8cbc-4234aa06ec09"

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __STREAM_FILTER_DRM_H_H__ */
