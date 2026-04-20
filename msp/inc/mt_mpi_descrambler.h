/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */

#ifndef __MT_MPI_DESCRAMBLER_H__
#define __MT_MPI_DESCRAMBLER_H__

#include "mt_type.h"

#include "mt_unf_descrambler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

mt_s32 MT_MPI_DMX_CreateDescrambler(mt_u32 u32DmxId, const MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDesramblerAttr, mt_handle *phKey);
mt_s32 MT_MPI_DMX_CreateDescramblerPro(mt_u32 u32DmxId, const MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S *pstDesramblerAttr, mt_handle *phKey);

mt_s32 MT_MPI_DMX_DestroyDescrambler(mt_handle hKey);
/*
*   for symphony2  adca
*/
mt_s32 MT_MPI_DMX_SetDescramblerEvenKeySlot(mt_handle hKey, mt_u8 oddKeySlot);
mt_s32 MT_MPI_DMX_SetDescramblerOddKeySlot(mt_handle hKey, mt_u8 oddKeySlot);

mt_s32 MT_MPI_DMX_SetDescramblerEvenKey(mt_handle hKey, const mt_u8 *pu8EvenKey);
mt_s32 MT_MPI_DMX_SetDescramblerOddKey(mt_handle hKey, const mt_u8 *pu8OddKey);
mt_s32 MT_MPI_DMX_SetDescramblerEvenIVKey(mt_handle hKey, const mt_u8 *pu8IVKey);
mt_s32 MT_MPI_DMX_SetDescramblerOddIVKey(mt_handle hKey, const mt_u8 *pu8IVKey);
mt_s32 MT_MPI_DMX_AttachDescrambler(mt_handle hKey, mt_handle hChannel);
mt_s32 MT_MPI_DMX_DetachDescrambler(mt_handle hKey, mt_handle hChannel);
mt_s32 MT_MPI_DMX_GetDescramblerKeyHandle(mt_handle hChannel, mt_handle *phKey);
mt_s32 MT_MPI_DMX_GetFreeDescramblerKeyCount(mt_u32 u32DmxId, mt_u32 *pu32FreeCount);
mt_s32 MT_MPI_DMX_GetDescramblerAttr(mt_handle hKey, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstAttr);
mt_s32 MT_MPI_DMX_SetDescramblerAttr(mt_handle hKey, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstAttr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __MT_MPI_DESCRAMBLER_H__ */

