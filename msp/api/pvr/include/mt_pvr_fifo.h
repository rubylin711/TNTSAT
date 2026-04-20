/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_PVR_H__
#define __MT_PVR_H__

#include "mt_pvr_fs_def.h"  //choose by msp\api\pvr\Makefile
#include "mt_unf_pvr.h"


typedef enum PVR_EVENT_STATUS_e {
       PVR_EVENT_IDLE = 0,
	PVR_EVENT_START,   
	PVR_EVENT_RECING,     
	PVR_EVENT_STOP,
	PVR_EVENT_STOP_SAVE,
	PVR_EVENT_RECING_SAVE,
	PVR_EVENT_APPEND_SAVE,
}PVR_EVENT_STATUS_E;

extern MT_CHAR * PVR_Get_EventPath(PVR_FILE64 file);
extern MT_S32 PVR_Rec_GetEventIndexDataEntry(PVR_FILE64 file,
                                                                            MT_CHAR  *pEeventName,
                                                                            PVR_IDX_HEADER_INFO_S *stIdxHeaderInfo, 
                                                                            MT_U64 *s, 
                                                                            MT_U64 *e,  
                                                                            PVR_EVENT_STATUS_E eventStatus);
extern MT_S32 PVR_RecNewEvent(PVR_FILE64 file,MT_CHAR *fname, 
                                                   PVR_TIMESHIFT_EVENT_TYPE  type,  
                                                   MT_U32  u32EventStartTime,
                                                   MT_U8 *preEventName,
                                                   MT_BOOL  preEventIsSaved);


extern MT_U32  PVR_UpdateSingleEventLoopStartFrame(MT_U32 u32ChnID, MT_U32  *startFrame,   MT_U32 *tsFileNodeId);
extern MT_S32 PVR_Rec_SaveTimeShiftEvent0Data(PVR_FILE64 file,  PVR_EVENT_STATUS_E eventStatus);
void PVR_Set_Rewinded(PVR_FILE64 file);
MT_CHAR * PVR_Get_EventTimeshiftName(PVR_FILE64 file);
MT_U32 PVR_SetRecFileSize( MT_U32 u32NewFileSizeInMBytes);
MT_U32 PVR_Get_File_BlockNum(const MT_CHAR *pszFileName);

#endif /* End of #ifdef __MT_PVR_H__ */

