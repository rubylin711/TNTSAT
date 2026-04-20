/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/ 
#ifndef __DRV_DESCRAMBLER_IOCTL_H__
#define __DRV_DESCRAMBLER_IOCTL_H__

#include "mt_type.h"

#include "mt_unf_descrambler.h"

typedef struct
{
    mt_u32                          DmxId;
    MT_UNF_DMX_DESCRAMBLER_ATTR_S   DesramblerAttr;
    mt_handle                       KeyHandle;
} DMX_NewKey_S;

typedef struct
{
    mt_u32                          DmxId;
    MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S   DesramblerAttrPro;
    mt_handle                       KeyHandle;
} DMX_NewKeyPro_S;

typedef struct
{
    mt_handle                       hKeyHandle;
    MT_UNF_DMX_DESCRAMBLER_ATTR_S   stDescramblerAttr;
} DMX_DescramblerAttr_S;


#define DMX_KEY_SIZE    32

typedef struct
{
    mt_handle   KeyHandle;
    mt_u8       Key[DMX_KEY_SIZE];
} DMX_KeySet_S;

typedef struct
{
    mt_handle   KeyHandle;
    mt_u8       KeySlot;
} DMX_KeySlotSet_S;

typedef struct
{
    mt_handle   KeyHandle;
    mt_handle   ChanHandle;
} DMX_KeyAttach_S;

typedef struct
{
    mt_u32  DmxId;
    mt_u32  FreeCount;
} DMX_FreeKeyGet_S;

#define CMD_DEMUX_KEYS_NEW                  _IOWR(MT_ID_DEMUX, 0x50, DMX_NewKey_S)              /* apply for a free key */
#define CMD_DEMUX_KEYS_DEL                  _IOW (MT_ID_DEMUX, 0x51, mt_handle)                 /* delete an allocated key */
#define CMD_DEMUX_KEYS_SET_EVEN             _IOW (MT_ID_DEMUX, 0x52, DMX_KeySet_S)              /* set even key */
#define CMD_DEMUX_KEYS_SET_ODD              _IOW (MT_ID_DEMUX, 0x53, DMX_KeySet_S)              /* set odd key */
#define CMD_DEMUX_KEYS_ATTACH               _IOW (MT_ID_DEMUX, 0x54, DMX_KeyAttach_S)           /* attach a key to a channel */
#define CMD_DEMUX_KEYS_DETACH               _IOW (MT_ID_DEMUX, 0x55, DMX_KeyAttach_S)           /* detach a key from a channel */
#define CMD_DEMUX_KEYS_GET_ID               _IOWR(MT_ID_DEMUX, 0x56, DMX_KeyAttach_S)           /* get key id on a channel */
#define CMD_DEMUX_KEYS_GET_FREE             _IOWR(MT_ID_DEMUX, 0x57, DMX_FreeKeyGet_S)          /* get free key count */
#define CMD_DEMUX_KEYS_SET_IVEVEN           _IOW (MT_ID_DEMUX, 0x58, DMX_KeySet_S)              /* set iv even key */
#define CMD_DEMUX_KEYS_SET_IVODD            _IOW (MT_ID_DEMUX, 0x59, DMX_KeySet_S)              /* set iv odd key */
#define CMD_DEMUX_KEYS_GET_DESCRAMBLERATTR  _IOWR (MT_ID_DEMUX, 0x5a, DMX_DescramblerAttr_S)     /* get descrambler attr */
#define CMD_DEMUX_KEYS_SET_DESCRAMBLERATTR  _IOW (MT_ID_DEMUX, 0x5b, DMX_DescramblerAttr_S)     /* set descrambler attr */
//add for set keySlotId
#define CMD_DEMUX_KEYS_SET_EVEN_KEYSLOT             _IOW (MT_ID_DEMUX, 0x5c, DMX_KeySlotSet_S)              /* set even key slot*/
#define CMD_DEMUX_KEYS_SET_ODD_KEYSLOT              _IOW (MT_ID_DEMUX, 0x5d, DMX_KeySlotSet_S)              /* set odd key slot*/

#define CMD_DEMUX_KEYS_NEW_PRO                  _IOWR(MT_ID_DEMUX, 0x5e, DMX_NewKeyPro_S)              /* apply for a free key */

#endif  // __DRV_DESCRAMBLER_IOCTL_H__

