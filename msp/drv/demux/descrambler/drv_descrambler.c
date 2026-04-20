/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/major.h>
#include <asm/types.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "mt_type.h"
#include "mt_drv_stat.h"
#include "mt_kernel_adapt.h"

#include "mt_unf_descrambler.h"
#include "drv_descrambler.h"
#include "mt_drv_descrambler.h"

//#include "../demux_debug.h"
#include "mt_module_debug.h"

#include "drv_demux_ioctl.h"
#include "drv_demux_config.h"
#include "drv_demux_define.h"
#include "drv_descrambler_func.h"
#include "drv_descrambler_ioctl.h"

#define DMX_KEYID(KeyHandle)    ((KeyHandle) & 0xff)

#define DMX_KEYHANDLE(KeyId)    ((KeyId) | 0x00000300 | (MT_ID_DEMUX << 16))

#define DMX_CHECK_KEYHANDLE(KeyHandle)                                  \
    do                                                                  \
    {                                                                   \
        if (   (DMX_KEYID(KeyHandle) >= DMX_KEY_CNT)                    \
            || (((KeyHandle) & 0xffffff00) != DMX_KEYHANDLE(0)) )       \
        {                                                               \
            MT_WARN_DEMUX("Invalid KeyHandle 0x%x\n", KeyHandle);       \
            return MT_ERR_DMX_INVALID_PARA;                             \
        }                                                               \
    } while (0)

typedef struct
{
    ulong  KeyFile[DMX_KEY_CNT];
} DMX_DEV_DSCR_S;

static DMX_DEV_DSCR_S   s_DmxDscrFile;

mt_s32 DMXKeyIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;

    switch (cmd)
    {
        case CMD_DEMUX_KEYS_NEW:
        {
            DMX_NewKey_S *Param = (DMX_NewKey_S*)arg;

            ret = MT_DRV_DMX_CreateDescrambler(Param->DmxId, &Param->DesramblerAttr, &Param->KeyHandle, (ulong)file);

            break;
        }
	case CMD_DEMUX_KEYS_NEW_PRO:
        {
            DMX_NewKeyPro_S *Param = (DMX_NewKeyPro_S*)arg;

            ret = MT_DRV_DMX_CreateDescramblerPro(Param->DmxId, &Param->DesramblerAttrPro, &Param->KeyHandle, (ulong)file);

            break;
        }
	
        case CMD_DEMUX_KEYS_DEL:
        {
            ret = MT_DRV_DMX_DestroyDescrambler(*(mt_handle*)arg);

            break;
        }

        case CMD_DEMUX_KEYS_GET_DESCRAMBLERATTR:
        {
            DMX_DescramblerAttr_S *param = (DMX_DescramblerAttr_S *)arg;
            ret = MT_DRV_DMX_GetDescramblerAttr(param->hKeyHandle, &param->stDescramblerAttr);

            break;
        }

        case CMD_DEMUX_KEYS_SET_DESCRAMBLERATTR:
        {
            DMX_DescramblerAttr_S *param = (DMX_DescramblerAttr_S *)arg;
            ret = MT_DRV_DMX_SetDescramblerAttr(param->hKeyHandle, &param->stDescramblerAttr);

            break;
        }

        case CMD_DEMUX_KEYS_SET_EVEN:
        {
            DMX_KeySet_S *Param = (DMX_KeySet_S*)arg;

            ret = MT_DRV_DMX_SetDescramblerEvenKey(Param->KeyHandle, Param->Key);

            //mt_drv_stat_event(STAT_EVENT_CWSET, 0);

            break;
        }

        case CMD_DEMUX_KEYS_SET_ODD:
        {
            DMX_KeySet_S *Param = (DMX_KeySet_S*)arg;

            ret = MT_DRV_DMX_SetDescramblerOddKey(Param->KeyHandle, Param->Key);

            break;
        }

        case CMD_DEMUX_KEYS_SET_EVEN_KEYSLOT:
        {
            DMX_KeySlotSet_S *Param = (DMX_KeySlotSet_S*)arg;

            ret = MT_DRV_DMX_SetDescramblerEvenKeySlot(Param->KeyHandle, Param->KeySlot);

            //mt_drv_stat_event(STAT_EVENT_CWSET, 0);

            break;
        }

        case CMD_DEMUX_KEYS_SET_ODD_KEYSLOT:
        {
            DMX_KeySlotSet_S *Param = (DMX_KeySlotSet_S*)arg;

            ret = MT_DRV_DMX_SetDescramblerOddKeySlot(Param->KeyHandle, Param->KeySlot);

            break;
        }

#ifdef DMX_DESCRAMBLER_VERSION_1
        case CMD_DEMUX_KEYS_SET_IVEVEN:
        {
            DMX_KeySet_S *Param = (DMX_KeySet_S*)arg;

            ret = MT_DRV_DMX_SetDescramblerEvenIVKey(Param->KeyHandle, Param->Key);

            break;
        }

        case CMD_DEMUX_KEYS_SET_IVODD:
        {
            DMX_KeySet_S *Param = (DMX_KeySet_S*)arg;

            ret = MT_DRV_DMX_SetDescramblerOddIVKey(Param->KeyHandle, Param->Key);

            break;
        }
#endif

        case CMD_DEMUX_KEYS_ATTACH:
        {
            DMX_KeyAttach_S *Param = (DMX_KeyAttach_S*)arg;

            ret = MT_DRV_DMX_AttachDescrambler(Param->KeyHandle, Param->ChanHandle);

            break;
        }

        case CMD_DEMUX_KEYS_DETACH:
        {
            DMX_KeyAttach_S *Param = (DMX_KeyAttach_S*)arg;

            ret = MT_DRV_DMX_DetachDescrambler(Param->KeyHandle, Param->ChanHandle);

            break;
        }

        case CMD_DEMUX_KEYS_GET_ID:
        {
            DMX_KeyAttach_S *Param = (DMX_KeyAttach_S*)arg;

            ret = MT_DRV_DMX_GetDescramblerKeyHandle(Param->ChanHandle, &Param->KeyHandle);

            break;
        }

        case CMD_DEMUX_KEYS_GET_FREE:
        {
            DMX_FreeKeyGet_S *Param = (DMX_FreeKeyGet_S*)arg;

            ret = MT_DRV_DMX_GetFreeDescramblerKeyCount(Param->DmxId, &Param->FreeCount);

            break;
        }

        default:
        {
            MT_WARN_DEMUX("unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}

mt_void DmxDestroyAllDescrambler(ulong file)
{
    mt_u32 i;

    for (i = 0; i < DMX_KEY_CNT; i++)
    {
        if (s_DmxDscrFile.KeyFile[i] == file)
        {
            MT_DRV_DMX_DestroyDescrambler(DMX_KEYHANDLE(i));
        }
    }
}

#ifdef MT_DEMUX_PROC_SUPPORT
mt_s32 DMXKeyProcRead(struct seq_file *p, mt_void *v)
{
    mt_u32 KeyId;

    PROC_PRINT(p, "Id ChanCnt EvenKey                             OddKey\n");

    for (KeyId = 0; KeyId < DMX_KEY_CNT; KeyId++)
    {
        DMX_KeyInfo_S *KeyInfo;

        KeyInfo = DMX_OsiGetKeyProc(KeyId);
        if (!KeyInfo)
        {
            continue;
        }

        PROC_PRINT(p, "%2u   %2u    %08x %08x %08x %08x %08x %08x %08x %08x\n",
                KeyId,
                KeyInfo->ChanCount,
                KeyInfo->EvenKey[0],
                KeyInfo->EvenKey[1],
                KeyInfo->EvenKey[2],
                KeyInfo->EvenKey[3],
                KeyInfo->OddKey[0],
                KeyInfo->OddKey[1],
                KeyInfo->OddKey[2],
                KeyInfo->OddKey[3]
            );
    }

    return MT_SUCCESS;
}
#endif

mt_s32 MT_DRV_DMX_CreateDescrambler(mt_u32 DmxId, MT_UNF_DMX_DESCRAMBLER_ATTR_S *DescAttr, mt_handle *KeyHandle, ulong file)
{
    mt_s32 ret;
    mt_u32 KeyId;

    CHECKDMXID(DmxId);
    CHECKPOINTER(DescAttr);
    CHECKPOINTER(KeyHandle);

    ret = DMX_OsiDescramblerCreate(&KeyId, DescAttr);
    if (MT_SUCCESS == ret)
    {
        s_DmxDscrFile.KeyFile[KeyId] = (ulong)file;

        *KeyHandle = DMX_KEYHANDLE(KeyId);
    }

    return ret;
}

mt_s32 MT_DRV_DMX_CreateDescramblerPro(mt_u32 DmxId, MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S *DescAttr, mt_handle *KeyHandle, ulong file)
{
    mt_s32 ret;
    mt_u32 KeyId;

    CHECKDMXID(DmxId);
    CHECKPOINTER(DescAttr);
    CHECKPOINTER(KeyHandle);

    ret = DMX_OsiDescramblerProCreate(&KeyId, DescAttr);
    if (MT_SUCCESS == ret)
    {
        s_DmxDscrFile.KeyFile[KeyId] = (ulong)file;

        *KeyHandle = DMX_KEYHANDLE(KeyId);
    }
    MT_DBG_DEMUX("KeyHandle = 0x%x\n",*KeyHandle);
    return ret;
}


mt_s32 MT_DRV_DMX_DestroyDescrambler(mt_handle KeyHandle)
{
    mt_s32 ret;

    DMX_CHECK_KEYHANDLE(KeyHandle);

    ret = DMX_OsiDescramblerDestroy(DMX_KEYID(KeyHandle));
    if (MT_SUCCESS == ret)
    {
        s_DmxDscrFile.KeyFile[DMX_KEYID(KeyHandle)] = 0;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_GetDescramblerAttr(mt_handle KeyHandle, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDescramblerAttr)
{
    DMX_CHECK_KEYHANDLE(KeyHandle);

    return DMX_OsiDescramblerGetAttr(DMX_KEYID(KeyHandle), pstDescramblerAttr);
}

mt_s32 MT_DRV_DMX_SetDescramblerAttr(mt_handle KeyHandle, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDescramblerAttr)
{
	DMX_CHECK_KEYHANDLE(KeyHandle);

    return DMX_OsiDescramblerSetAttr(DMX_KEYID(KeyHandle), pstDescramblerAttr);
}

mt_s32 MT_DRV_DMX_SetDescramblerEvenKeySlot(mt_handle KeyHandle, mt_u8 KeySlot)
{
    //printk("MT_DRV_DMX_SetDescramblerEvenKey >>> KeyHandle= 0x%x\n",KeyHandle);
    DMX_CHECK_KEYHANDLE(KeyHandle);
    return DMX_OsiDescramblerSetKeySlot(DMX_KEYID(KeyHandle), DMX_KEY_TYPE_EVEN, KeySlot);
}

mt_s32 MT_DRV_DMX_SetDescramblerOddKeySlot(mt_handle KeyHandle, mt_u8 KeySlot)
{
    //printk("MT_DRV_DMX_SetDescramblerOddKey >>> KeyHandle= 0x%x\n",KeyHandle);
    DMX_CHECK_KEYHANDLE(KeyHandle);
    return DMX_OsiDescramblerSetKeySlot(DMX_KEYID(KeyHandle), DMX_KEY_TYPE_ODD, KeySlot);
}

mt_s32 MT_DRV_DMX_SetDescramblerEvenKey(mt_handle KeyHandle, mt_u8 *Key)
{
    //printk("MT_DRV_DMX_SetDescramblerEvenKey >>> KeyHandle= 0x%x\n",KeyHandle);
    DMX_CHECK_KEYHANDLE(KeyHandle);
    return DMX_OsiDescramblerSetKey(DMX_KEYID(KeyHandle), DMX_KEY_TYPE_EVEN, Key);
}

mt_s32 MT_DRV_DMX_SetDescramblerOddKey(mt_handle KeyHandle, mt_u8 *Key)
{
    //printk("MT_DRV_DMX_SetDescramblerOddKey >>> KeyHandle= 0x%x\n",KeyHandle);
    DMX_CHECK_KEYHANDLE(KeyHandle);
    return DMX_OsiDescramblerSetKey(DMX_KEYID(KeyHandle), DMX_KEY_TYPE_ODD, Key);
}

#ifdef DMX_DESCRAMBLER_VERSION_1
mt_s32 MT_DRV_DMX_SetDescramblerEvenIVKey(mt_handle KeyHandle, mt_u8 *Key)
{
    DMX_CHECK_KEYHANDLE(KeyHandle);

    return DMX_OsiDescramblerSetIVKey(DMX_KEYID(KeyHandle), DMX_KEY_TYPE_EVEN, Key);
}

mt_s32 MT_DRV_DMX_SetDescramblerOddIVKey(mt_handle KeyHandle, mt_u8 *Key)
{
    DMX_CHECK_KEYHANDLE(KeyHandle);

    return DMX_OsiDescramblerSetIVKey(DMX_KEYID(KeyHandle), DMX_KEY_TYPE_ODD, Key);
}
#endif

mt_s32 MT_DRV_DMX_AttachDescrambler(mt_handle KeyHandle, mt_handle ChanHandle)
{
    DMX_CHECK_KEYHANDLE(KeyHandle);
    DMX_CHECK_CHANHANDLE(ChanHandle);

    return DMX_OsiDescramblerAttach(DMX_KEYID(KeyHandle), DMX_CHANID(ChanHandle));
}

mt_s32 MT_DRV_DMX_DetachDescrambler(mt_handle KeyHandle, mt_handle ChanHandle)
{
    DMX_CHECK_KEYHANDLE(KeyHandle);
    DMX_CHECK_CHANHANDLE(ChanHandle);

    return DMX_OsiDescramblerDetach(DMX_KEYID(KeyHandle), DMX_CHANID(ChanHandle));
}

mt_s32 MT_DRV_DMX_GetDescramblerKeyHandle(mt_handle ChanHandle, mt_handle *KeyHandle)
{
    mt_s32  ret;
    mt_u32  KeyId;

    CHECKPOINTER(KeyHandle);
    DMX_CHECK_CHANHANDLE(ChanHandle);

    ret = DMX_OsiDescramblerGetKeyId(DMX_CHANID(ChanHandle), &KeyId);
    if (MT_SUCCESS == ret)
    {
        *KeyHandle = DMX_KEYHANDLE(KeyId);
    }

    return ret;
}

mt_s32 MT_DRV_DMX_GetFreeDescramblerKeyCount(mt_u32 DmxId, mt_u32 *FreeCount)
{
    CHECKDMXID(DmxId);
    CHECKPOINTER(FreeCount);

    return DMX_OsiDescramblerGetFreeKeyNum(FreeCount);
}
//for OpenTV5
EXPORT_SYMBOL(MT_DRV_DMX_CreateDescrambler);
EXPORT_SYMBOL(MT_DRV_DMX_DestroyDescrambler);
EXPORT_SYMBOL(MT_DRV_DMX_SetDescramblerEvenKey);
EXPORT_SYMBOL(MT_DRV_DMX_SetDescramblerOddKey);
#ifdef DMX_DESCRAMBLER_VERSION_1
EXPORT_SYMBOL(MT_DRV_DMX_SetDescramblerEvenIVKey);
EXPORT_SYMBOL(MT_DRV_DMX_SetDescramblerOddIVKey);
#endif
EXPORT_SYMBOL(MT_DRV_DMX_AttachDescrambler);
EXPORT_SYMBOL(MT_DRV_DMX_DetachDescrambler);
EXPORT_SYMBOL(MT_DRV_DMX_GetDescramblerKeyHandle);
EXPORT_SYMBOL(MT_DRV_DMX_GetFreeDescramblerKeyCount);


