/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DSP_UTIL__H__
#define __DSP_UTIL__H__

#include "mt_type.h"
#include "mt_drv_ao.h"
#include "mt_drv_ai.h"
#include "drv_ao_private.h"
#include "hal_aiao_common.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C"
{
 #endif
#endif
#define VOLUME_6dB (0x7f)
#define VOLUME_0dB (0x79)
#define VOLUME_infdB (0x28)
#define VOLUME_MAX_dB (AOE_AIP_VOL_6dB)
#define VOLUME_MIN_dB (AOE_AIP_VOL_infdB)

#define IEC61937_DATATYPE_NULL 0
#define IEC61937_DATATYPE_DOLBY_DIGITAL 1     /* AC3 */
#define IEC61937_DATATYPE_DTS_TYPE_I 11   /* DTS Type 1 */
#define IEC61937_DATATYPE_DTS_TYPE_II 12   /* DTS Type 2 */
#define IEC61937_DATATYPE_DTS_TYPE_III 13   /* DTS Type 3 */
#define IEC61937_DATATYPE_DTS_TYPE_IV 17   /* DTS Type 4 */
#define IEC61937_DATATYPE_DOLBY_DIGITAL_PLUS 21   /* AC3 */
#define IEC61937_DATATYPE_DOLBY_TRUE_HD 22   /* True HD */

#define IEC61937_DATATYPE_71_LPCM 0xf0

#define IEC61937_DATATYPE_DTSCD 0xff         /* DTS CD */
#define IEC61937_DATATYPE_DOLBY_SIMUL 0xfe

////Define IC PLATFORM/////
typedef enum
{
  AUTIL_CMTP_PLATFORM_S40,
  AUTIL_CMTP_TYPE_PLATFORM_S5,
  AUTIL_CMTP_TYPE_PLATFORM_BUTT,
}AUTIL_CMTP_PLATFORM_E;



mt_u32			AUTIL_IEC61937DataType(mt_u16 *pu16IecData, mt_u32 u32IecDataSize);
mt_s32			AUTIL_isIEC61937Hbr(mt_u32 u32IEC61937DataType, mt_u32 uSourceRate);
mt_u32			AUTIL_CalcFrameSize(mt_u32 u32Ch, mt_u32 u32BitDepth);
mt_u32			AUTIL_LatencyMs2ByteSize(mt_u32 u32LatencyMs, mt_u32 u32FrameSize, mt_u32 u32SampleRate);
mt_u32			AUTIL_ByteSize2LatencyMs(mt_u32 u32DataBytes, mt_u32 u32FrameSize, mt_u32 u32SampleRate);
mt_u32			AUTIL_VolumeLinear2RegdB(mt_u32 u32Linear);
mt_u32			AUTIL_VolumedB2RegdB(mt_s32 dBVol);
mt_s32			AUTIL_SetBitZeroOrOne(mt_u32* pu32Val, mt_u32 u32Bit, mt_u32 u32ZeroOrOne);
mt_u32          AUTIL_BclkFclkDiv(MT_UNF_I2S_MCLK_SEL_E enMclkSel, MT_UNF_I2S_BCLK_SEL_E enBclkSel);
mt_u32          AUTIL_MclkFclkDiv(MT_UNF_I2S_MCLK_SEL_E enMclkSel);

const mt_char * AUTIL_Port2Name(MT_UNF_SND_OUTPUTPORT_E enPort);
const MT_UNF_SND_OUTPUTPORT_E AUTIL_PortName2Port(mt_char *pcName);
const mt_char * AUTIL_AiPort2Name(MT_UNF_AI_E enAiPort);
const mt_char * AUTIL_TrackMode2Name(MT_UNF_TRACK_MODE_E enMode);
const AIAO_TRACK_MODE_E AUTIL_TrackModeTransform(MT_UNF_TRACK_MODE_E enMode);
const mt_char * AUTIL_HdmiMode2Name(MT_UNF_SND_HDMI_MODE_E enMode);
const mt_char * AUTIL_SpdifMode2Name(MT_UNF_SND_SPDIF_MODE_E enMode);
const mt_char * AUTIL_Engine2Name(SND_ENGINE_TYPE_E enEngine);
const mt_char * AUTIL_Format2Name(mt_u32 u32Format);
const mt_char * AUTIL_CategoryCode2Name(MT_UNF_SND_SPDIF_CATEGORYCODE_E enCategory);
const mt_char * AUTIL_ScmsMode2Name(MT_UNF_SND_SPDIF_SCMSMODE_E enScms);

mt_void			AUTIL_OS_GetTime(mt_u32 *t_ms);

mt_void*		AUTIL_AO_MALLOC(mt_u32 u32ModuleID, mt_u32 u32Size, mt_s32 flag);
mt_void			AUTIL_AO_FREE(mt_u32 u32ModuleID, mt_void* pMemAddr);
mt_void*		AUTIL_AIAO_MALLOC(mt_u32 u32ModuleID, mt_u32 u32Size, mt_s32 flag);
mt_void			AUTIL_AIAO_FREE(mt_u32 u32ModuleID, mt_void* pMemAddr);


AUTIL_CMTP_PLATFORM_E  AUTIL_GetChipPlatform(mt_void);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif

#endif
