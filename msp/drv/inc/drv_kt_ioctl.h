/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __KT_IOCTL_H__
#define __KT_IOCTL_H__
#include <linux/types.h>
#include "mt_module.h"
#include "mt_mpi_kt.h"

#define KT_REQUEST_MULTI_MAX 4

typedef struct _CMD_KT_SLOT_S
{
  MT_KT_SLOT_ID_E slot_id;
} CMD_KT_SLOT_REQUEST_S, CMD_KT_SLOT_RELEASE_S;

typedef struct _CMD_KT_SLOT_MULTI_S
{
  mt_u8 num;
  MT_KT_SLOT_ID_E slot_ids[KT_REQUEST_MULTI_MAX];
} CMD_KT_SLOT_REQUEST_MULTI_S;

typedef struct _CMD_KT_SLOT_ACTIVE_S
{
  MT_KT_SLOT_ID_E slot_id;
  MT_KT_SLOT_ACTIVE_E active;
} CMD_KT_SLOT_ACTIVE_S;

typedef struct _CMD_KT_ATTR_S
{
  MT_KT_SLOT_ID_E slot_id;
  MT_KT_KEY_ATTR_S attr;
} CMD_KT_WRITE_ATTR_S, CMD_KT_READ_ATTR_S;

typedef struct _CMD_KT_KEY_S
{
  MT_KT_SLOT_ID_E slot_id;
  mt_u8 key[32];
  MT_KT_SLOT_SIZE_E size;
} CMD_KT_WRITE_KEY_S, CMD_KT_READ_KEY_S;

typedef struct _CMD_KT_IV_S
{
  MT_KT_SLOT_ID_E slot_id;
  mt_u8 iv[16];
  MT_KT_SLOT_SIZE_E size;
} CMD_KT_WRITE_IV_S, CMD_KT_READ_IV_S;

typedef struct _CMD_KT_INFO_S
{
  MT_KT_SLOT_ID_E slot_id;
  mt_u32 status;
  mt_u32 valid;
  mt_u32 attr;
  mt_u32 teedata;  
} CMD_KT_INFO_S;

typedef struct _CMD_KT_GET_STATE_S
{
  MT_KT_SLOT_ID_E slot_id;
  MT_KT_SLOT_STATE_E state;
} CMD_KT_GET_STATE_S;

typedef struct _CMD_KT_CONTROL_S
{
  mt_u32 size;
  mt_u32 control[8];
} CMD_KT_CONTROL_S;

#ifdef CONFIG_MT_CHIP_SYMPHONY6
typedef struct _CMD_KT_METADATA_S
{
  MT_KT_SLOT_ID_E slot_id;
  mt_u32 metadata;
} CMD_KT_READ_METADATA_S;
#endif

#define KT_IOC_SLOT_REQUEST				_IOR(MT_ID_KT, 0x0, CMD_KT_SLOT_REQUEST_S)
#define KT_IOC_SLOT_RELEASE				_IOW(MT_ID_KT, 0x1, CMD_KT_SLOT_RELEASE_S)
#define KT_IOC_SLOT_ACTIVE					_IOW(MT_ID_KT, 0x2, CMD_KT_SLOT_ACTIVE_S)
#define KT_IOC_WRITE_ATTR					_IOW(MT_ID_KT, 0x3, CMD_KT_WRITE_ATTR_S)
#define KT_IOC_READ_ATTR					_IOWR(MT_ID_KT, 0x4, CMD_KT_READ_ATTR_S)
#define KT_IOC_WRITE_KEY					_IOW(MT_ID_KT, 0x5, CMD_KT_WRITE_KEY_S)
#define KT_IOC_READ_KEY					_IOWR(MT_ID_KT, 0x6, CMD_KT_READ_KEY_S)
#define KT_IOC_WRITE_IV					_IOW(MT_ID_KT, 0x7, CMD_KT_WRITE_IV_S)
#define KT_IOC_READ_IV						_IOWR(MT_ID_KT, 0x8, CMD_KT_READ_IV_S)
#define KT_IOC_SLOT_REQUEST_MULTI		_IOWR(MT_ID_KT, 0x9, CMD_KT_SLOT_REQUEST_MULTI_S)
#define KT_IOC_SLOT_INFO					_IOWR(MT_ID_KT, 0xa, CMD_KT_INFO_S)
#define KT_IOC_GET_STATE					_IOWR(MT_ID_KT, 0xb, CMD_KT_GET_STATE_S)
#define KT_IOC_READ_METADATA			_IOWR(MT_ID_KT, 0xc, CMD_KT_READ_METADATA_S)
#define KT_IOC_CONTROL					_IOWR(MT_ID_KT, 0xd, CMD_KT_CONTROL_S)

#endif

