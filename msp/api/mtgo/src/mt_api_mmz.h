/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __MT_API_MMZ_H__
#define __MT_API_MMZ_H__

/* add include here */
#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

mt_s32 MT_MMB_Init(phys_addr_t uStartAddr , mt_u32 uLen);
mt_s32 MT_MMB_DeInit(mt_void);
phys_addr_t MT_MMB_New(mt_u32 size , mt_u32 align, mt_u8 *mmz_name, mt_u8 *mmb_name );
mt_void *MT_MMB_Map(phys_addr_t phys_addr, mt_u32 cached);
mt_s32 MT_MMB_Unmap(mt_void *vaddr);
mt_s32 MT_MMB_Delete(phys_addr_t phys_addr);
mt_s32 MT_MMB_Flush(mt_void);

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

#ifdef __cplusplus
}
#endif

#endif /* __MT_API_MMZ_H__ */


