/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_module_debug.h"
#include "mt_mpi_descrambler.h"

#include "drv_descrambler_ioctl.h"

extern mt_s32 g_s32DmxFd;

#define MPIDscrCheckDeviceFd()          \
    do                                  \
    {                                   \
        if (-1 == g_s32DmxFd)           \
        {                               \
            return MT_ERR_DMX_NOT_INIT; \
        }                               \
    } while (0)

#define MPIDscrCheckPointer(p)          \
    do                                  \
    {                                   \
        if (MT_NULL == p)               \
        {                               \
            return MT_ERR_DMX_NULL_PTR; \
        }                               \
    } while (0)

mt_s32 MT_MPI_DMX_CreateDescrambler(mt_u32 u32DmxId, const MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDesramblerAttr, mt_handle *phKey)
{
    mt_s32          ret = MT_FAILURE;
    DMX_NewKey_S    Param={0,};

    MPIDscrCheckDeviceFd();
    MPIDscrCheckPointer(pstDesramblerAttr);
    MPIDscrCheckPointer(phKey);
    if(MT_UNF_DMX_DESCRAMBLER_TYPE_BUTT == pstDesramblerAttr->enDescramblerType)
    {
	 *phKey = -1;
	 return MT_ERR_DMX_INVALID_PARA;
    }
    Param.DmxId = u32DmxId;
    memcpy(&Param.DesramblerAttr, pstDesramblerAttr, sizeof(MT_UNF_DMX_DESCRAMBLER_ATTR_S));

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_NEW, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *phKey = Param.KeyHandle;
    }

    return ret;
}


mt_s32 MT_MPI_DMX_CreateDescramblerPro(mt_u32 u32DmxId, const MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S *pstDesramblerAttr, mt_handle *phKey)
{
    mt_s32          ret = MT_FAILURE;
    DMX_NewKeyPro_S    Param={0,};

    MPIDscrCheckDeviceFd();
    MPIDscrCheckPointer(pstDesramblerAttr);
    MPIDscrCheckPointer(phKey);

    Param.DmxId = u32DmxId;
    memcpy(&Param.DesramblerAttrPro, pstDesramblerAttr, sizeof(MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S));

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_NEW_PRO, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *phKey = Param.KeyHandle;
    }

    return ret;
}



mt_s32 MT_MPI_DMX_DestroyDescrambler(mt_handle hKey)
{
    MPIDscrCheckDeviceFd();

    return ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_DEL, (ulong)&hKey);
}

mt_s32 MT_MPI_DMX_GetDescramblerAttr(mt_handle hKey, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstAttr)
{
    mt_s32 ret = MT_FAILURE;
    DMX_DescramblerAttr_S stDescAttr;

    MPIDscrCheckDeviceFd();
    MPIDscrCheckPointer(pstAttr);

    stDescAttr.hKeyHandle = hKey;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_GET_DESCRAMBLERATTR, &stDescAttr);
    if(MT_SUCCESS != ret)
    {
        goto out;
    }

    pstAttr->enCaType = stDescAttr.stDescramblerAttr.enCaType;
    pstAttr->enDescramblerType = stDescAttr.stDescramblerAttr.enDescramblerType;
    pstAttr->enEntropyReduction = stDescAttr.stDescramblerAttr.enEntropyReduction;
    
out:
    return ret;
}

mt_s32 MT_MPI_DMX_SetDescramblerAttr(mt_handle hKey, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstAttr)
{
    DMX_DescramblerAttr_S stDescAttr={0,};

    MPIDscrCheckDeviceFd();
    MPIDscrCheckPointer(pstAttr);

    stDescAttr.hKeyHandle = hKey;
    
    stDescAttr.stDescramblerAttr.enCaType = pstAttr->enCaType;
    stDescAttr.stDescramblerAttr.enDescramblerType = pstAttr->enDescramblerType;
    stDescAttr.stDescramblerAttr.enEntropyReduction = pstAttr->enEntropyReduction;

    stDescAttr.stDescramblerAttr.ivMode = pstAttr->ivMode;

    return ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_SET_DESCRAMBLERATTR, &stDescAttr);
}

mt_s32 MT_MPI_DMX_SetDescramblerEvenKeySlot(mt_handle hKey, mt_u8 evenKeySlot)
{
    DMX_KeySlotSet_S  Param = {0,};
    
    MPIDscrCheckDeviceFd();
    Param.KeyHandle = hKey;
    Param.KeySlot = evenKeySlot;
    return ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_SET_EVEN_KEYSLOT, (ulong)&Param);
}


mt_s32 MT_MPI_DMX_SetDescramblerOddKeySlot(mt_handle hKey, mt_u8 oddKeySlot)
{
    DMX_KeySlotSet_S  Param = {0,};
    
    MPIDscrCheckDeviceFd();
    Param.KeyHandle = hKey;
    Param.KeySlot = oddKeySlot;
    return ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_SET_ODD_KEYSLOT, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_SetDescramblerEvenKey(mt_handle hKey, const mt_u8 *pu8EvenKey)
{
    DMX_KeySet_S Param = {0,};

    MPIDscrCheckDeviceFd();
    MPIDscrCheckPointer(pu8EvenKey);

    Param.KeyHandle = hKey;
    memcpy(Param.Key, pu8EvenKey, sizeof(Param.Key));
    return ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_SET_EVEN, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_SetDescramblerOddKey(mt_handle hKey, const mt_u8 *pu8OddKey)
{
    DMX_KeySet_S Param ={0,};

    MPIDscrCheckDeviceFd();
    MPIDscrCheckPointer(pu8OddKey);

    Param.KeyHandle = hKey;
    memcpy(Param.Key, pu8OddKey, sizeof(Param.Key));
    return ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_SET_ODD, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_SetDescramblerEvenIVKey(mt_handle hKey, const mt_u8 *pu8IVKey)
{
    DMX_KeySet_S Param={0,};

    MPIDscrCheckDeviceFd();
    MPIDscrCheckPointer(pu8IVKey);

    Param.KeyHandle = hKey;
    memcpy(Param.Key, pu8IVKey, sizeof(Param.Key));

    return ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_SET_IVEVEN, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_SetDescramblerOddIVKey(mt_handle hKey, const mt_u8 *pu8IVKey)
{
    DMX_KeySet_S Param={0,};

    MPIDscrCheckDeviceFd();
    MPIDscrCheckPointer(pu8IVKey);

    Param.KeyHandle = hKey;
    memcpy(Param.Key, pu8IVKey, sizeof(Param.Key));

    return ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_SET_IVODD, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_AttachDescrambler(mt_handle hKey, mt_handle hChannel)
{
    DMX_KeyAttach_S Param={0,};

    MPIDscrCheckDeviceFd();

    Param.KeyHandle = hKey;
    Param.ChanHandle= hChannel;

    return ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_ATTACH, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_DetachDescrambler(mt_handle hKey, mt_handle hChannel)
{
    DMX_KeyAttach_S Param={0,};

    MPIDscrCheckDeviceFd();

    Param.KeyHandle = hKey;
    Param.ChanHandle= hChannel;

    return ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_DETACH, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_GetDescramblerKeyHandle(mt_handle hChannel, mt_handle *phKey)
{
    mt_s32          ret;
    DMX_KeyAttach_S Param={0,};

    MPIDscrCheckDeviceFd();
    MPIDscrCheckPointer(phKey);

    Param.ChanHandle = hChannel;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_GET_ID, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *phKey = Param.KeyHandle;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_GetFreeDescramblerKeyCount(mt_u32 u32DmxId, mt_u32 *pu32FreeCount)
{
    mt_s32              ret;
    DMX_FreeKeyGet_S    Param={0,};

    MPIDscrCheckDeviceFd();
    MPIDscrCheckPointer(pu32FreeCount);

    Param.DmxId = u32DmxId;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_KEYS_GET_FREE, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *pu32FreeCount = Param.FreeCount;
    }

    return ret;
}

