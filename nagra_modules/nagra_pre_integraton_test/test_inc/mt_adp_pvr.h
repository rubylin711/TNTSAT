/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __SAMPLE_PVR_COMMON_H__
#define __SAMPLE_PVR_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#include <assert.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_unf_pvr.h"

#include "mt_adp_search.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Macro Definition ******************************/
#define PVR_FS_DIR              "/tmpfs/"
#define PVR_CIPHER_KEY          "688PVR-KEY-123456789"

#define PVR_DMX_ID_LIVE             1
#define PVR_DMX_PORT_ID_IP          MT_UNF_DMX_PORT_RAM_1
#define PVR_DMX_PORT_ID_DVB MT_UNF_DMX_PORT_TSI_0
#define PVR_DMX_PORT_ID_PLAYBACK    MT_UNF_DMX_PORT_RAM_0
#define PVR_DMX_ID_REC              0

#define PVR_PROG_INFO_MAGIC  0xABCDAA55


/*************************** Structure Definition ****************************/
typedef struct hiPVR_PROG_INFO_S
{
    MT_U32                  u32MagicNumber;
    PMT_COMPACT_PROG        stProgInfo;
    MT_UNF_PVR_REC_ATTR_S   stRecAttr;
}PVR_PROG_INFO_S;


/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/
mt_s32 PVR_checkIdx(char *pfileName);
mt_s32 PVR_SavePorgInfo(PVR_PROG_INFO_S *pstProgInfo, mt_char *pszPvrRecFile);
mt_s32 PVR_GetPorgInfo(PVR_PROG_INFO_S *pstProgInfo, const mt_char *pszPvrRecFile);

mt_s32 PVR_RecStart(char *path, PMT_COMPACT_PROG *pstProgInfo, mt_u32 u32DemuxID,
            MT_BOOL bRewind, MT_BOOL bDoCipher, mt_u64 maxSize, mt_u32 *pRecChn,MT_BOOL bDIO);
mt_s32 PVR_RecStop(mt_u32 u32RecChnID);


mt_s32 PVR_StartLivePlay(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo);
mt_s32 PVR_StopLivePlay(mt_handle hAvplay);


mt_s32 PVR_StartPlayBack(const mt_char *pszFileName, mt_u32 *pu32PlayChn, mt_handle hAvplay);
mt_s32 PVR_StopPlayLive(mt_handle hAvplay);
mt_void PVR_StopPlayBack(mt_u32 playChn);
mt_s32 PVR_SwitchDmxSource(mt_u32 dmxId, mt_u32 protId);

mt_s32 PVR_AvplayInit(mt_handle hWin, mt_handle *phAvplay, mt_handle* phSoundTrack);
mt_s32  PVR_AvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack);

mt_void PVR_CallBack(mt_u32 u32ChnID, MT_UNF_PVR_EVENT_E EventType, mt_s32 s32EventValue, mt_void *args);
mt_s32 PVR_RegisterCallBacks(mt_void);
mt_s32 PVR_CheckKeyladder(MT_UNF_PVR_CIPHER_S* PvrCipher);
mt_u8* PVR_GetEventTypeStringByID(MT_UNF_PVR_EVENT_E eEventID);

mt_s32 Normal_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr, 
										mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr, 
										mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr, 
										mt_u32 u32Offset, 
										mt_u32 u32DataSize);

mt_s32 Normal_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr, 
										mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr, 
										mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr, 
										mt_u32 u32Offset, 
										mt_u32 u32DataSize);

mt_s32 Crypto_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr, 
										mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr, 
										mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr, 
										mt_u32 u32Offset, 
										mt_u32 u32DataSize);

mt_s32 Crypto_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr, 
										mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr, 
										mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr, 
										mt_u32 u32Offset, 
										mt_u32 u32DataSize);

#ifdef __cplusplus
}
#endif
#endif /* __SAMPLE_PVR_COMMON_H__ */


