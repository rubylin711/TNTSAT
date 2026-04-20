/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __FLASH_API_INTERNAL_H__
#define __FLASH_API_INTERNAL_H__

int cmdline_parts_init(char *bootargs);
mt_s32 find_flash_part(char *cmdline_string, const char *media_name, char *ptn_name, mt_u64 *start, mt_u64 *length);
mt_s32 get_part_info(mt_u8 partnum, mt_u64 *start, mt_u64 *size);
mt_s32 find_part_from_devname(char *media_name, char *bootargs, char *devname, mt_u64 *start, mt_u64 *size);
#endif /* __FLASH_API_INTERNAL_H__ */
