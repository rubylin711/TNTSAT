/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */

#include <stdio.h>

#include "mt_type.h"
#include "mt_debug.h"
//#include "demux_debug.h"
#include "mt_module_debug.h"

#include "mt_mpi_descrambler.h"
#include "mt_unf_descrambler.h"

mt_s32 MT_UNF_DMX_CreateDescrambler(mt_u32 u32DmxId, mt_handle *phKey)
{
    MT_UNF_DMX_DESCRAMBLER_ATTR_S stAttr;

    stAttr.enCaType = MT_UNF_DMX_CA_NORMAL;
    stAttr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2;
    stAttr.enEntropyReduction = MT_UNF_DMX_CA_ENTROPY_REDUCTION_OPEN;
    return MT_MPI_DMX_CreateDescrambler(u32DmxId, &stAttr, phKey);
}

mt_s32 MT_UNF_DMX_CreateDescramblerExt(mt_u32 u32DmxId, const MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDesramblerAttr, mt_handle *phKey)
{
    return MT_MPI_DMX_CreateDescrambler(u32DmxId, pstDesramblerAttr, phKey);
}

mt_s32 MT_UNF_DMX_CreateDescramblerPro(mt_u32 u32DmxId, const MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S *pstDesramblerAttr, mt_handle *phKey)
{
    return MT_MPI_DMX_CreateDescramblerPro(u32DmxId, pstDesramblerAttr, phKey);
}


mt_s32 MT_UNF_DMX_DestroyDescrambler(mt_handle hKey)
{
    return MT_MPI_DMX_DestroyDescrambler(hKey);
}

mt_s32 MT_UNF_DMX_SetDescramblerAttr(mt_handle hKey, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstAttr)
{
	return MT_MPI_DMX_SetDescramblerAttr(hKey, pstAttr);
}

mt_s32 MT_UNF_DMX_GetDescramblerAttr(mt_handle hKey, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstAttr)
{
	return MT_MPI_DMX_GetDescramblerAttr(hKey, pstAttr);
}

mt_s32 MT_UNF_DMX_SetDescramblerEvenKeySlot(mt_handle hKey, mt_u8 evenKeySlot)
{
    return MT_MPI_DMX_SetDescramblerEvenKeySlot(hKey, evenKeySlot);
}

mt_s32 MT_UNF_DMX_SetDescramblerOddKeySlot(mt_handle hKey,  mt_u8 oddKeySlot)
{
    return MT_MPI_DMX_SetDescramblerOddKeySlot(hKey, oddKeySlot);
}

mt_s32 MT_UNF_DMX_SetDescramblerEvenKey(mt_handle hKey, const mt_u8 *pu8EvenKey)
{
    return MT_MPI_DMX_SetDescramblerEvenKey(hKey, pu8EvenKey);
}

mt_s32 MT_UNF_DMX_SetDescramblerOddKey(mt_handle hKey, const mt_u8 *pu8OddKey)
{
    return MT_MPI_DMX_SetDescramblerOddKey(hKey, pu8OddKey);
}

mt_s32 MT_UNF_DMX_SetDescramblerEvenIVKey(mt_handle hKey, const mt_u8 *pu8IVKey)
{
    return MT_MPI_DMX_SetDescramblerEvenIVKey(hKey, pu8IVKey);
}

mt_s32 MT_UNF_DMX_SetDescramblerOddIVKey(mt_handle hKey, const mt_u8 *pu8IVKey)
{
    return MT_MPI_DMX_SetDescramblerOddIVKey(hKey, pu8IVKey);
}

mt_s32 MT_UNF_DMX_AttachDescrambler(mt_handle hKey, mt_handle hChannel)
{
    return MT_MPI_DMX_AttachDescrambler(hKey, hChannel);
}

mt_s32 MT_UNF_DMX_DetachDescrambler(mt_handle hKey, mt_handle hChannel)
{
    return MT_MPI_DMX_DetachDescrambler(hKey, hChannel);
}

mt_s32 MT_UNF_DMX_GetDescramblerKeyHandle(mt_handle hChannel, mt_handle *phKey)
{
    return MT_MPI_DMX_GetDescramblerKeyHandle(hChannel, phKey);
}

mt_s32 MT_UNF_DMX_GetFreeDescramblerKeyCount(mt_u32 u32DmxId, mt_u32 *pu32FreeCount)
{
    return MT_MPI_DMX_GetFreeDescramblerKeyCount(u32DmxId, pu32FreeCount);
}
