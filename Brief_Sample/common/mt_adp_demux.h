/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef  _SAMPLE_MT_DEMUX_H
#define  _SAMPLE_MT_DEMUX_H

#include "mt_type.h"
#include "mt_unf_demux.h"
#include <stdio.h>
#include <pthread.h>

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif /* __cplusplus */
#endif  /* __cplusplus */

#define DMX_COUNT                       7

#define DMX_DVBPORT_COUNT               3
#define DMX_RAMPORT_COUNT               4
#define DMX_PORT_COUNT                  (DMX_DVBPORT_COUNT + DMX_RAMPORT_COUNT)
#ifdef MT_SYM4_DSS
#define DMX_DVB_TSI_IN_PORT             MT_UNF_DMX_PORT_TSI_2
#else
#define DMX_DVB_TSI_IN_PORT             MT_UNF_DMX_PORT_TSI_0
#endif

#define MAX_SECTION_LEN 4096
#define MAX_SECTION_NUM 256
#define INVALID_PORT_DMX_ID 0xFFFF
#define MAX_IP_THREAD_CNT 16
#define MAX_FILE_THREAD_CNT 16

#define DEMUX_SECITON_BUF_SIZE (256 * 1024)


typedef mt_s32 (*T_CommSectionCallback)(const mt_u8 *pu8Buffer, mt_s32 s32BufferLength, mt_u8 *pSectionStruct);

/********************************************************************
  mark:SECTIONSVR_DATA_FILTER_S
  type:data struct
  purpose:present SECTION data filter contents, application can use it for special filtrate request
  definition:
 **********************************************************************/
typedef struct mtDEMUX_filter_info_S {

	MT_U8  u8Mask[DMX_FILTER_MAX_DEPTH];
	MT_U8  u8Match[DMX_FILTER_MAX_DEPTH];
	mt_u16 u16FilterDepth;

} DEMUX_FILTER_S;

typedef struct  mtDMX_DATA_FILTER_S
{
    mt_u32 u32TSPID;                /* TSPID */
    mt_u32 u32BufSize;          /* hareware BUFFER request */

    mt_u8 u8SectionType;       /* section type, 0-section 1-PES */
    mt_u8 u8Crcflag;               /* channel CRC open flag, 0-not open; 1-open */

    mt_u8  u8Match[DMX_FILTER_MAX_DEPTH];
    mt_u8  u8Mask[DMX_FILTER_MAX_DEPTH];
    mt_u8  u8Negate[DMX_FILTER_MAX_DEPTH];
    mt_u16 u16FilterDepth;          /* filtrate depth, 0xff-data use all the user set, otherwise, use DVB algorithm(fixme)*/

    mt_u32 u32TimeOut;       /* timeout, in second. 0-permanent wait */

    T_CommSectionCallback funSectionFunCallback;   /* section end callback */
    mt_u8 *pSectionStruct;
} DMX_DATA_FILTER_S;

/* Definition for IP play*/
typedef struct HiIP_PLAY_HANDLE_S
{
    pthread_t g_SocketThd;
    mt_char   cMultiIPAddr[20];
    mt_u16    u32UdpPort;
    mt_u32    u32PortID;
    MT_BOOL   bStopSocketThread;
    MT_BOOL   bUsed;
} IP_PLAY_HANDLE_S;

/* Definition for File play*/
typedef struct HiFILE_PLAY_HANDLE_S
{
    FILE               *g_pTsFile;
    pthread_t           g_FileThd;
    mt_u32              u32PortID;
    MT_BOOL             bStopFileThread;
    MT_BOOL             bUsed;
} FILE_PLAY_HANDLE_S;

/* Definition for File play*/
typedef struct HiES_FILE_PLAY_HANDLE_S
{
    FILE                  *g_pEsFile;
    pthread_t           g_FileThd;
    mt_handle		  havplay;
    MT_BOOL		  type;  // 0 auido  1 video
    MT_BOOL             bStopFileThread;
    MT_BOOL             bUsed;
} ES_FILE_PLAY_HANDLE_S;

mt_s32 DMX_Init(void);
mt_s32 DMX_DeInit(void);
mt_s32 DMX_SectionStartDataFilter(mt_u32 u32DmxId, DMX_DATA_FILTER_S * pstDataFilter);
mt_s32 DMX_Prepare(mt_u32 u32DmxID, mt_u32 u32PortID, mt_u32 u32BufSize);
mt_s32 DMX_IPStartInject(mt_char *pMultiAddr, mt_u32 u32UdpPort, mt_u32 u32PortID);
mt_s32 DMX_IPStopInject(mt_u32 u32PortID);
mt_s32 DMX_FileStartInject(mt_char *path, mt_u32 u32PortID);
mt_s32 DMX_FileStopInject(mt_u32 u32PortID);
mt_s32 DMX_EsFileStartInject(mt_char *path, mt_handle avplay, mt_u32 type);
mt_s32 DMX_EsFileStopInject(mt_u32 index);


MT_S32 MTADP_DEMUX_OpenChannel(mt_u32 u32DmxId, MT_U16  pid, MT_HANDLE *pChannelHandle);
MT_S32 MTADP_DEMUX_CloseChannel(MT_HANDLE channelHandle);
MT_S32 MTADP_DEMUX_StartChannel(MT_HANDLE channelHandle);
MT_S32 MTADP_DEMUX_StopChannel(MT_HANDLE channelHandle);
MT_S32 MTADP_DEMUX_SetFilter(mt_u32 u32DmxId, MT_HANDLE channelHandle, DEMUX_FILTER_S *pFilterInfo, MT_HANDLE *pFilterHandle);
MT_S32 MTADP_DEMUX_CloseFilter(MT_HANDLE channelHandle, MT_HANDLE filterHandle);

mt_s32 MTADP_DEMUX_DataRead(mt_handle hChannel, mt_u32 u32TimeOutms, mt_u8 *pBuf , mt_u32* pAcquiredNum , mt_u32 * pBuffSize);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif /* __cplusplus */
#endif  /* __cplusplus */

#endif /* _SECTIONSVR_PUB_H*/
