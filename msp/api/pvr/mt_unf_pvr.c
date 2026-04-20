/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */

//#include "mt_unf_pvr.h"
#include "mt_mpi_pvr.h"
#include "mt_pvr_rec_ctrl.h"
#include "mt_pvr_play_ctrl.h"

#ifdef PVR_UNF_DEBUG
#define ENTER_FUNCTION			do{printf("%s: Enter\n",__FUNCTION__);}while(0)
#define LEAVE_FUNCTION			do{printf("%s: Leave@%d\n",__FUNCTION__,__LINE__);}while(0)
#else
#define ENTER_FUNCTION			do{}while(0)
#define LEAVE_FUNCTION			do{}while(0)
#endif
/***********************************************************
                API Define
 ***********************************************************/

/***** APIs for PVR recode *****/

/* initialize and de-initialize of record module                            */
MT_S32 MT_UNF_PVR_RecInit(MT_VOID)
{
	MT_S32 ret = MT_FAILURE;
    /*w37134 for AI7D05516 pvr need to call cipher function,cipher function is offered inner application not client application , it should be called by inner module  */
      /*CNcomment:需要使用加密接口，而加密接口不提供给客户，需要内部打开*/
#ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
    //(void)MT_UNF_CIPHER_Open();       mt_unf_cipher_init() be called at initfile
#endif

	ENTER_FUNCTION;
    ret = MT_PVR_RecInit();
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_RecDeInit(MT_VOID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    //MT_UNF_CIPHER_Close();
    ret =  MT_PVR_RecDeInit();
	LEAVE_FUNCTION;

	return ret;
}

/* applay and release new record channel                                    */
MT_S32 MT_UNF_PVR_RecCreateChn(MT_U32 *pu32ChnID, const MT_UNF_PVR_REC_ATTR_S *pstRecAttr)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_RecCreateChn(pu32ChnID, pstRecAttr);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_RecCopyChn(MT_U32 *pu32ChnID, MT_U32 u32_SRC_ChnID, const MT_UNF_PVR_REC_ATTR_S *pstRecAttr)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_RecCopyChn(pu32ChnID, u32_SRC_ChnID,pstRecAttr);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_RecDestroyChn(MT_U32 u32ChnID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_RecDestroyChn(u32ChnID);
	LEAVE_FUNCTION;

	return ret;
}

/* set and get attributes of record channel                                 */
MT_S32 MT_UNF_PVR_RecSetChn(MT_U32 u32ChnID, const MT_UNF_PVR_REC_ATTR_S *pstRecAttr)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_RecSetChn(u32ChnID, pstRecAttr);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_RecGetChn(MT_U32 u32ChnID, MT_UNF_PVR_REC_ATTR_S *pstRecAttr)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_RecGetChn(u32ChnID, pstRecAttr);
	LEAVE_FUNCTION;

	return ret;
}

/* start and stop record channel                                            */
MT_S32 MT_UNF_PVR_RecStartChn(MT_U32 u32ChnID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_RecStartChn(u32ChnID);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_RecStopChn(MT_U32 u32ChnID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_RecStopChn(u32ChnID);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_RecPauseChn(MT_U32 u32ChnID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_RecPauseChn(u32ChnID);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_RecResumeChn(MT_U32 u32ChnID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_RecResumeChn(u32ChnID);
	LEAVE_FUNCTION;

	return ret;
}


/* get record status                                                        */
MT_S32 MT_UNF_PVR_RecGetStatus(MT_U32 u32ChnID, MT_UNF_PVR_REC_STATUS_S *pstRecStatus)
{
    return MT_PVR_RecGetStatus(u32ChnID, pstRecStatus);
}

/* set pid                                                       */
MT_S32 MT_UNF_PVR_RecSetPid(mt_u32 chnId, mt_u32 u32DmxId, int pid)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_Add_Pid(chnId, u32DmxId, pid, 0);
	LEAVE_FUNCTION;

	return ret;
}
/* get pids                                                     */
MT_S32 MT_UNF_PVR_RecGetPids(mt_u32 chnId, int *pids,int *num)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_Get_Pids(chnId, pids, num);
	LEAVE_FUNCTION;

	return ret;
}
MT_S32 MT_UNF_PVR_AddDelPid_Ex(mt_u32 chnId, mt_u32 u32DmxId, int pid,MT_BOOL flag_adddel,MT_BOOL flag_index)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_AddDelPid_Ex(chnId, u32DmxId, pid, 0,flag_adddel,flag_index);
	LEAVE_FUNCTION;

	return ret;
}


/***** APIs for PVR play *****/

/* initialize and de-initialize of play module                              */
MT_S32 MT_UNF_PVR_PlayInit(MT_VOID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    /*w37134 for AI7D05516 pvr need to call cipher function,cipher function is offered inner application not client application , it should be called by inner module  */
      /*CNcomment:需要使用加密接口，而加密接口不提供给客户，需要内部打开*/
#ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
    //(void)MT_UNF_CIPHER_Open();   mt_unf_cipher_init() be called at initfile
#endif

    ret =  MT_PVR_PlayInit();
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_PlayDeInit(MT_VOID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    //MT_UNF_CIPHER_Close();
    ret =  MT_PVR_PlayDeInit();
	LEAVE_FUNCTION;

	return ret;
}

/* apply and release new play channel                                       */
MT_S32 MT_UNF_PVR_PlayCreateChn(MT_U32 *pu32ChnID, const MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr, MT_HANDLE hAvplay, MT_HANDLE hTsBuffer)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayCreateChn(pu32ChnID, pstPlayAttr, hAvplay, hTsBuffer);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_PlayDestroyChn(MT_U32 u32ChnID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayDestroyChn(u32ChnID);
	LEAVE_FUNCTION;

	return ret;
}

/* start and stop time shift                                                */
MT_S32 MT_UNF_PVR_PlayStartTimeShift(MT_U32 *pu32PlayChnID, MT_U32 u32RecChnID, MT_HANDLE hAvplay, MT_HANDLE hTsBuffer)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayStartTimeShift(pu32PlayChnID, u32RecChnID, hAvplay, hTsBuffer);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_PlayStopTimeShift(MT_U32 u32PlayChnID,  const MT_UNF_AVPLAY_STOP_OPT_S *pstStopOpt)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayStopTimeShift(u32PlayChnID, pstStopOpt);
	LEAVE_FUNCTION;

	return ret;
}

/* set and get attributes of play channel                                   */
MT_S32 MT_UNF_PVR_PlaySetChn(MT_U32 u32ChnID, const MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlaySetChn(u32ChnID, pstPlayAttr);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_PlayGetChn(MT_U32 u32ChnID, MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayGetChn(u32ChnID, pstPlayAttr);
	LEAVE_FUNCTION;

	return ret;
}

/* start and stop play channel                                              */
MT_S32 MT_UNF_PVR_PlayStartChn(MT_U32 u32ChnID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayStartChn(u32ChnID);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_PlayStopChn(MT_U32 u32ChnID,  const MT_UNF_AVPLAY_STOP_OPT_S *pstStopOpt)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayStopChn(u32ChnID, pstStopOpt);
	LEAVE_FUNCTION;

	return ret;
}

/* pause and resume play channel                                            */
MT_S32 MT_UNF_PVR_PlayPauseChn(MT_U32 u32ChnID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayPauseChn(u32ChnID);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_PlayResumeChn(MT_U32 u32ChnID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayResumeChn(u32ChnID);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_TPlayPauseResume(MT_U32 u32ChnID, MT_BOOL bPause)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
	ret =  MT_PVR_TPlayPauseResume(u32ChnID, bPause);
	LEAVE_FUNCTION;

	return ret;
}


/* get play status                                                          */
MT_S32 MT_UNF_PVR_PlayGetStatus(MT_U32 u32ChnID, MT_UNF_PVR_PLAY_STATUS_S *pstStatus)
{
    return MT_PVR_PlayGetStatus(u32ChnID, pstStatus);
}

/* locate play position                                                     */
MT_S32 MT_UNF_PVR_PlaySeek(MT_U32 u32ChnID, const MT_UNF_PVR_PLAY_POSITION_S *pstPosition)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlaySeek(u32ChnID, pstPosition);
	LEAVE_FUNCTION;

	return ret;
}

/* start trick mode of playing                                              */
MT_S32 MT_UNF_PVR_PlayTPlay(MT_U32 u32ChnID, const MT_UNF_PVR_PLAY_MODE_S *pstTrickMode)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayTrickMode(u32ChnID, pstTrickMode);
	LEAVE_FUNCTION;

	return ret;
}

/* start step back frame by frame                                           */
MT_S32 MT_UNF_PVR_PlayStep(MT_U32 u32ChnID, MT_S32 s32Direction)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret =  MT_PVR_PlayStep(u32ChnID, s32Direction);
	LEAVE_FUNCTION;

	return ret;
}

/* get file info */
MT_S32 MT_UNF_PVR_PlayGetFileAttr(MT_U32 u32ChnID, MT_UNF_PVR_FILE_ATTR_S *pAttr)
{
    return MT_PVR_PlayGetFileAttr(u32ChnID, pAttr);
}

/* get file attr, no need to new play channel */
MT_S32 MT_UNF_PVR_GetFileAttrByFileName(const MT_CHAR *pFileName, MT_UNF_PVR_FILE_ATTR_S *pAttr)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_GetFileAttrByFileName(pFileName, pAttr);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_SetUsrDataInfoByFileName(const MT_CHAR *pFileName, MT_U8 *pInfo, MT_U32 u32UsrDataLen)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_SetUsrDataInfoByFileName(pFileName, pInfo, u32UsrDataLen);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_GetUsrDataInfoByFileName(const MT_CHAR *pFileName, MT_U8 *pInfo, MT_U32 u32BufLen, MT_U32* pUsrDataLen)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_GetUsrDataInfoByFileName(pFileName, pInfo, u32BufLen, pUsrDataLen);
	LEAVE_FUNCTION;

	return ret;
}


/***** APIs for PVR event callback *****/
/* register and un-register envent callback function                        */
MT_S32 MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_E enEventType, eventCallBack callBack, MT_VOID *args)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_RegisterEvent(enEventType, callBack, args);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_E enEventType)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_UnRegisterEvent(enEventType);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_CreateIdxFile(MT_U8* pstTsFileName, MT_U8* pstIdxFileName, MT_UNF_PVR_GEN_IDX_ATTR_S* pAttr)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_CreateIdxFile(pstTsFileName, pstIdxFileName, pAttr);
	LEAVE_FUNCTION;

	return ret;
}

MT_VOID MT_UNF_PVR_ConfigDebugInfo(mt_log_level_e enDebugLevel)
{
	ENTER_FUNCTION;
    MT_PVR_ConfigDebugInfo(enDebugLevel);
	LEAVE_FUNCTION;
}

MT_VOID MT_UNF_PVR_RemoveFile(const MT_CHAR *pFileName)
{
	ENTER_FUNCTION;
    MT_PVR_RemoveFile(pFileName);
	LEAVE_FUNCTION;
}
MT_S32 MT_UNF_PVR_RegisterExtraCallback(MT_U32 u32ChnID, MT_UNF_PVR_EXTRA_CALLBACK_E enExtraCallbackType, ExtraCallBack fCallback, MT_VOID *args)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_RegisterExtraCallback(u32ChnID, enExtraCallbackType, fCallback, args);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_UnRegisterExtraCallBack(MT_U32 u32ChnID, MT_UNF_PVR_EXTRA_CALLBACK_E enExtraCallbackType)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_UnRegisterExtraCallBack(u32ChnID, enExtraCallbackType);
	LEAVE_FUNCTION;

	return ret;
}
MT_S32 MT_UNF_PVR_SetThreadAttr(PVR_THREAD_TYPE type,MT_S32 schedpolicy, MT_S32 priority,MT_S32 stacksize)
{
	ENTER_FUNCTION;
    if(PVR_TS_READ==type){
        MT_PVR_SetPlayThreadattr((MT_S32)type,schedpolicy,priority,stacksize);
    }else{
        MT_PVR_SetIndexThreadattr((MT_S32)type,schedpolicy,priority,stacksize);
    }
	LEAVE_FUNCTION;

    return MT_SUCCESS; 
}
/*
**  timeshift event function
*/

MT_S32 MT_UNF_PVR_Start_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pEeventName, MT_U32  u32StartTimeMs, PVR_TIMESHIFT_EVENT_TYPE  type)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_Start_Event_Rec(u32ChnID,pEeventName, u32StartTimeMs, type);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_Stop_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pEeventName)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_Stop_Event_Rec(u32ChnID, pEeventName);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_Save_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pEeventName)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
    ret = MT_PVR_Save_Event_Rec(u32ChnID,pEeventName,  MT_FALSE);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_Delete_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pTimeShiftPath, MT_CHAR *pEeventName)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
	ret = MT_PVR_Delete_Event_Rec(u32ChnID, pTimeShiftPath, pEeventName);
	LEAVE_FUNCTION;

	return ret;
}

MT_S32 MT_UNF_PVR_StopCache_Event_Rec(MT_U32 u32ChnID)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
	ret = MT_PVR_StopCache_Event_Rec(u32ChnID);
	LEAVE_FUNCTION;

	return ret;
}

/*
**  adv ca pvr function
*/
MT_S32 MT_UNF_PVR_SetCAData(const MT_CHAR *pIdxFileName, MT_U8 *pInfo, MT_U32 u32CADataLen, MT_U8 u8IsAPPEND)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
	ret = MT_PVR_SetCAData(pIdxFileName, pInfo, u32CADataLen, u8IsAPPEND);
	LEAVE_FUNCTION;

	return ret;
}


MT_S32 MT_UNF_PVR_GetCAData(const MT_CHAR *pIdxFileName, MT_U8 *pInfo, MT_U32 u32BufLen, MT_U32* u32CADataLen)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
	ret = MT_PVR_GetCAData(pIdxFileName, pInfo, u32BufLen, u32CADataLen);
	LEAVE_FUNCTION;

	return ret;
}

MT_U32 MT_UNF_PVR_SetRecFileSize( MT_U32 u32NewFileSizeInMBytes)
{
    MT_U32 ret = 0;
	ENTER_FUNCTION;
	ret =  PVR_SetRecFileSize(u32NewFileSizeInMBytes);
	LEAVE_FUNCTION;
	return ret;
}

MT_S32 MT_UNF_PVR_PlayChangeAudioPid(MT_U32 u32ChnID, MT_BOOL aorv, MT_U32 type, MT_U32 pid)
{
	MT_S32 ret = MT_FAILURE;

	ENTER_FUNCTION;
	ret =  MT_PVR_ChangeAudioPid(u32ChnID, aorv, type, pid);
	LEAVE_FUNCTION;

	return ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


