/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_MPI_PVR_H__
#define __MT_MPI_PVR_H__


#include "mt_type.h"

#include "mt_unf_pvr.h"

#include "mt_unf_demux.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PVR_FIFO_DIO_LEN     512
#define PVR_FIFO_SECTOR_SIZE 512
#define PVR_FIFO_WRITE_BLOCK_SIZE (PVR_FIFO_DIO_LEN * 2 * 188LLU) /* 188k:the bigger block make more efficient*/


/***********************************************************
                API Declaration
 ***********************************************************/

/***** APIs for PVR recode *****/

/* initialize and de-initialize of record module                            */
MT_S32 MT_PVR_RecInit(MT_VOID);
MT_S32 MT_PVR_RecDeInit(MT_VOID);

/* applay and release new record channel                                    */
MT_S32 MT_PVR_RecCreateChn(MT_U32 *pu32ChnID, const MT_UNF_PVR_REC_ATTR_S *pstRecAttr);
MT_S32 MT_PVR_RecDestroyChn(MT_U32 u32ChnID);

/* set and get attributes of record channel                                 */
MT_S32 MT_PVR_RecSetChn(MT_U32 u32ChnID, const MT_UNF_PVR_REC_ATTR_S *pstRecAttr);
MT_S32 MT_PVR_RecGetChn(MT_U32 u32ChnID, MT_UNF_PVR_REC_ATTR_S *pstRecAttr);

/* start and stop record channel                                            */
MT_S32 MT_PVR_RecStartChn(MT_U32 u32ChnID);
MT_S32 MT_PVR_RecStopChn(MT_U32 u32ChnID);
MT_S32 MT_PVR_RecPauseChn(MT_U32 u32ChnID);
MT_S32 MT_PVR_RecResumeChn(MT_U32 u32ChnID);


/* get record status                                                        */
MT_S32 MT_PVR_RecGetStatus(MT_U32 u32ChnID, MT_UNF_PVR_REC_STATUS_S *pstRecStatus);


/***** APIs for PVR play *****/

/* initialize and de-initialize of play module                              */
MT_S32 MT_PVR_PlayInit(MT_VOID);
MT_S32 MT_PVR_PlayDeInit(MT_VOID);

/* apply and release new play channel                                       */
MT_S32 MT_PVR_PlayCreateChn(MT_U32 *pChn, const MT_UNF_PVR_PLAY_ATTR_S *pAttr, MT_HANDLE hAvplay, MT_HANDLE hTsBuffer);
MT_S32 MT_PVR_PlayDestroyChn(MT_U32 u32ChnID);

/* start and stop time shift                                                */
MT_S32 MT_PVR_PlayStartTimeShift(MT_U32 *pu32PlayChnID, MT_U32 u32RecChnID, MT_HANDLE hAvplay, MT_HANDLE hTsBuffer);
MT_S32 MT_PVR_PlayStopTimeShift(MT_U32 u32PlayChnID, const MT_UNF_AVPLAY_STOP_OPT_S *pstStopOpt);

/* set and get attributes of play channel                                   */
MT_S32 MT_PVR_PlaySetChn(MT_U32 u32ChnID, const MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr);
MT_S32 MT_PVR_PlayGetChn(MT_U32 u32ChnID, MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr);

/* start and stop play channel                                              */
MT_S32 MT_PVR_PlayStartChn(MT_U32 u32ChnID);
MT_S32 MT_PVR_PlayStopChn(MT_U32 u32ChnID, const MT_UNF_AVPLAY_STOP_OPT_S *pstStopOpt);

/* pause and resume play channel                                            */
MT_S32 MT_PVR_PlayPauseChn(MT_U32 u32ChnID);
MT_S32 MT_PVR_PlayResumeChn(MT_U32 u32ChnID);

/* get play status                                                          */
MT_S32 MT_PVR_PlayGetStatus(MT_U32 u32ChnID, MT_UNF_PVR_PLAY_STATUS_S *pstStatus);

/* locate play position                                                     */
MT_S32 MT_PVR_PlaySeek(MT_U32 u32ChnID, const MT_UNF_PVR_PLAY_POSITION_S *pstPosition);

/* start trick mode of playing                                              */
MT_S32 MT_PVR_PlayTrickMode(MT_U32 u32ChnID, const MT_UNF_PVR_PLAY_MODE_S *pstTrickMode);

/* start step back frame by frame                                           */
MT_S32 MT_PVR_PlayStep(MT_U32 u32ChnID, MT_S32 s32Direction);

/* get file info */
MT_S32 MT_PVR_PlayGetFileAttr(MT_U32 u32ChnID, MT_UNF_PVR_FILE_ATTR_S *pAttr);

/* get file attr, no need to new play channel */
MT_S32 MT_PVR_GetFileAttrByFileName(const MT_CHAR *pFileName, MT_UNF_PVR_FILE_ATTR_S *pAttr);

MT_S32 MT_PVR_SetUsrDataInfoByFileName(const MT_CHAR *pFileName, MT_U8 *pInfo, MT_U32 u32UsrDataLen);

MT_S32 MT_PVR_GetUsrDataInfoByFileName(const MT_CHAR *pFileName, MT_U8 *pInfo, MT_U32 u32BufLen, MT_U32* pUsrDataLen);

//MT_S32 MT_PVR_SetCAData(const MT_CHAR *pFileName, MT_U8 *pInfo, MT_U32 u32CADataLen);
MT_S32 MT_PVR_SetCAData(const MT_CHAR *pIdxFileName, MT_U8 *pInfo, MT_U32 u32CADataLen,  MT_U8  u8IsAPPEND);


MT_S32 MT_PVR_GetCAData(const MT_CHAR *pFileName, MT_U8 *pInfo, MT_U32 u32BufLen, MT_U32* u32CADataLen);

/***** APIs for PVR event callback *****/

/* register and un-register envent callback function    AI7D02612                    */
MT_S32 MT_PVR_RegisterEvent(MT_UNF_PVR_EVENT_E enEventType, eventCallBack callBack, MT_VOID *args);
MT_S32 MT_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_E enEventType);

MT_S32 MT_PVR_CreateIdxFile(MT_U8* pstTsFileName, MT_U8* pstIdxFileName, MT_UNF_PVR_GEN_IDX_ATTR_S* pAttr);

MT_VOID MT_PVR_ConfigDebugInfo(MT_LOG_LEVEL_E u32DebugLevel);

MT_VOID MT_PVR_RemoveFile(const MT_CHAR *pFileName);

MT_S32 MT_PVR_RegisterExtraCallback(MT_U32 u32ChnID, MT_UNF_PVR_EXTRA_CALLBACK_E eExtraCallbackType, ExtraCallBack fCallback, MT_VOID *args);

MT_S32 MT_PVR_UnRegisterExtraCallBack(MT_U32 u32ChnID, MT_UNF_PVR_EXTRA_CALLBACK_E eExtraCallbackType);

MT_S32 MT_PVR_SetPlayThreadattr(MT_S32 type,MT_S32 schedpolicy, MT_S32 priority,MT_S32 stacksize);

MT_S32 MT_PVR_SetIndexThreadattr(MT_S32 type,MT_S32 schedpolicy, MT_S32 priority,MT_S32 stacksize);

MT_S32 MT_PVR_Add_Pid(mt_u32 chnId, mt_u32 u32DmxId, int pid, MT_UNF_DMX_CHAN_TYPE_E chnType);

/*get av handle by pvrchan*/
MT_S32 MT_PVR_GetAvHandle(MT_U32 u32Chn, MT_U32  *avhandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // __MT_MPI_PVR_H__

