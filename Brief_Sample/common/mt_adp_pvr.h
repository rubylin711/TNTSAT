/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

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
#define PVR_DMX_PORT_ID_DVB         MT_UNF_DMX_PORT_TSI_0
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

typedef struct
{
    MT_BOOL             bkey;
    MT_U8               au8Key[PVR_MAX_CIPHER_KEY_LEN];
    MT_U32              u32KeyLen;
    MT_CIPHER_ALGORITHM_E enType;
}mt_pvr_rec_cipher_t;


/******************************* API declaration *****************************/
mt_s32 MTADP_PVR_checkIdx(char *pfileName);
mt_s32 MTADP_PVR_SavePorgInfo(PVR_PROG_INFO_S *pstProgInfo, mt_char *pszPvrRecFile);
mt_s32 MTADP_PVR_GetPorgInfo(PVR_PROG_INFO_S *pstProgInfo, const mt_char *pszPvrRecFile);

mt_s32 MTADP_PVR_RecStart(char *path, PMT_COMPACT_PROG *pstProgInfo, mt_u32 u32DemuxID,
            MT_BOOL bRewind, MT_BOOL bDoCipher, mt_u64 maxSize, mt_u32 *pRecChn,MT_BOOL bDIO, mt_pvr_rec_cipher_t rec_cipher, MT_BOOL bInfo);
mt_s32 MTADP_PVR_RecStop(mt_u32 u32RecChnID);


mt_s32 MTADP_PVR_StartLivePlay(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo);
mt_s32 MTADP_PVR_StopLivePlay(mt_handle hAvplay);


mt_s32 MTADP_PVR_StartPlayBack(const mt_char *pszFileName, mt_u32 *pu32PlayChn, mt_handle hAvplay);
mt_s32 MTADP_PVR_StopPlayLive(mt_handle hAvplay);
mt_void MTADP_PVR_StopPlayBack(mt_u32 playChn);
mt_s32 MTADP_PVR_SwitchDmxSource(mt_u32 dmxId, mt_u32 protId);

mt_s32 MTADP_PVR_AvplayInit(mt_handle hWin, mt_handle *phAvplay, mt_handle* phSoundTrack);
mt_s32  MTADP_PVR_AvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack);

mt_s32 MTADP_PVR_RestoreAc4PlayAttrInfo(mt_handle hAvplay);

mt_void MTADP_PVR_CallBack(mt_u32 u32ChnID, MT_UNF_PVR_EVENT_E EventType, mt_s32 s32EventValue, mt_void *args);
mt_s32 MTADP_PVR_RegisterCallBacks(MT_HANDLE  *       hAvPlay);
mt_s32 MTADP_PVR_UnRegisterCallBacks(mt_void);
mt_s32 MTADP_PVR_CheckKeyladder(MT_UNF_PVR_CIPHER_S* PvrCipher);
mt_u8* MTADP_PVR_GetEventTypeStringByID(MT_UNF_PVR_EVENT_E eEventID);

mt_s32 MTADP_PVR_RecCopy(char *path, PMT_COMPACT_PROG *pstProgInfo, mt_u32 u32DemuxID,
            MT_BOOL bRewind, MT_BOOL bDoCipher, mt_u64 maxSize, mt_u32 *pRecChn,mt_u32 pRecChnSrc,MT_BOOL bDIO);

mt_s32 MTADP_PVR_Normal_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize);

mt_s32 MTADP_PVR_Normal_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize);
#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_s32 MTADP_PVR_Crypto_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize);

mt_s32 MTADP_PVR_Crypto_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize);

#elif defined CONFIG_MT_CHIP_SYMPHONY6
mt_s32 MTADP_PVR_Crypto_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  ulong u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, ulong u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize);

mt_s32 MTADP_PVR_Crypto_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  ulong u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, ulong u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize);
#endif
#ifdef __cplusplus
}
#endif
#endif /* __SAMPLE_PVR_COMMON_H__ */


