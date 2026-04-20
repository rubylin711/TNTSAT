/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_VSS_H__
#define __MT_UNF_VSS_H__
#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

mt_s32 MT_UNF_VSS_Init(mt_void);

mt_s32 MT_UNF_VSS_DeInit(mt_void);

mt_s32 MT_UNF_VSS_Reset(mt_void);

mt_s32 MT_UNF_VSS_SetPID(const mt_u32 pid);

mt_s32 MT_UNF_VSS_CreateMailbox(mt_handle *mailbox);

mt_s32 MT_UNF_VSS_DestroyMailbox(mt_handle mailbox);

mt_s32 MT_UNF_VSS_WriteMailbox(mt_handle mailbox, const mt_void *data, size_t len);

mt_s32 MT_UNF_VSS_ReadMailbox(mt_handle mailbox, mt_void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif	/*__MT_UNF_VSS_H__*/
