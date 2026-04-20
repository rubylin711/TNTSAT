/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2023, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_otp_object.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2023/08/02
 * Description    : MT OTP Object function.
 * History        :
 * 1.Date         : 2023/08/02
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifdef __UBOOT__
#include <common.h>

#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_otp.h>

#define DUMP_LOG			printf

#define CONFIG_MT_CHIP_SYMPHONY6

#elif defined(__KERNEL__)
#include "mach/chipinfo.h"
#include "mt_drv_proc.h"
#include "../crm/mt_log.h"
#include "mt_drv_otp.h"

#define DUMP_LOG			DP_LOG

extern int (*symphony_otp_read)(u32,u8,u32*);

#else	/*AVCPU*/
#include <string.h>
#include "sys_types.h"
#include "av_misc.h"
#include "hal_misc.h"
#include "hal_otp.h"

#define MT_LOGE				AV_PRINTF
#define MT_LOGI				AV_PRINTF
#define MT_LOGV(...)		do{}while(0)

#define DUMP_LOG			AV_PRINTF
#endif

#include "mt_otp_object.h"

#ifdef CONFIG_MT_CHIP_SYMPHONY6
extern struct mt_otp_object g_mt_otp_objects_sym6_a0[];
extern struct mt_otp_object g_mt_otp_objects_sym6[];
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
extern struct mt_otp_object g_mt_otp_objects_sym4[];
#else
extern struct mt_otp_object g_mt_otp_objects_sym1[];
extern struct mt_otp_object g_mt_otp_objects_sym2[];
#endif

//TODO
#define CHECK_OBJ_RET(obj)

static struct mt_otp_object *get_obj_by_idx(int idx)
{
#ifdef CONFIG_MT_CHIP_SYMPHONY6
#ifdef __UBOOT__
	if (MT_CHIP_SYMPHONY6_A0 == mt_get_chip_id())
		return &g_mt_otp_objects_sym6_a0[idx];
	else
		return &g_mt_otp_objects_sym6[idx];
#elif defined(__KERNEL__)
	if (CHIP_SYMPHONY6_A0 == symphony_get_chip_rev())
		return &g_mt_otp_objects_sym6_a0[idx];
	else
		return &g_mt_otp_objects_sym6[idx];
#endif
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	return &g_mt_otp_objects_sym4[idx];
#else
	if (hal_chip_is_symphony1()) {
		return &g_mt_otp_objects_sym1[idx];
	} else {
		return &g_mt_otp_objects_sym2[idx];
	}
#endif
}

static struct mt_otp_object *get_obj_by_name(const char *name)
{
	int i;
	struct mt_otp_object *obj;

	if (name == NULL)
		return NULL;

	for (i=0;;i++)
	{
		obj = get_obj_by_idx(i);

		if (obj == NULL || obj->name == NULL)
			return NULL;

		if (strcmp(name, obj->name) == 0)
			return obj;
	}

	return NULL;
}

/* read Obj's OTP value */
static unsigned int otp_read_obj(struct mt_otp_object *obj)
{
	u32 bit_addr;
	u8 len;
	u32 value;
	s32 ret = -1;

	if (obj == NULL)
		return (unsigned int)(-1);

	CHECK_OBJ_RET(obj);

	if (obj->bit_addr_flag)
		bit_addr = obj->offset + obj->shift;
	else
		bit_addr = (obj->offset << 3) + obj->shift;		/* offset: byte offset ==> bit address */

	len = obj->width;

#ifdef __UBOOT__
	ret = otp_read_random(bit_addr, len, &value);
#elif defined(__KERNEL__)
	if (symphony_otp_read == NULL)
	{
		MT_LOGE("[ERROR]Can NOT read OTP, pls open OTP first!\n");
		return (unsigned int)(-1);
	}

	BUG_ON(symphony_otp_read == NULL);

	ret = symphony_otp_read(bit_addr, len, &value);
#else
	ret = hal_otp_read_random(bit_addr, len, &value);
#endif
	if (ret == 0)
	{
		MT_LOGV("otp_read_obj: %s (0x%X, %u, %u, 0x%X) = 0x%X\r\n",
			obj->name,
			obj->offset,
			obj->shift,
			obj->width,
			obj->mask,
			value);

		return (value & obj->mask);
	}
	else
	{
		MT_LOGE("[ERROR]OTP read(%u, %u) return failed!!!\n", bit_addr, len);
		BUG_ON(1);
		return (unsigned int)(-1);
	}
}

/* get description by value */
static const char *get_description(struct mt_otp_object *obj, unsigned int value)
{
	unsigned int i;

	if (obj && obj->description)
	{
		for (i=0; i<value; i++)
		{
			if (obj->description[i] == NULL)
				return NULL;
		}

		// i = value;
		return obj->description[i];
	}
	else
	{
		return NULL;
	}
}

/*
 * Get OTP Object information
 *
 * @param[in] name OTP object name
 * @param[out] value OTP object value
 * @param[out] description OTP object detail description
 *
 * @return
 *     0: success
 *    !0: failure
 */
int mt_otp_get_obj(const char *name, unsigned int *value, const char **description)
{
	struct mt_otp_object *obj;

	if (value == NULL)
	{
		MT_LOGE("mt_otp_get_obj: invalid arguments!\r\n");
		return (-1);
	}

	//FIXME: OTP default value '1'?
	*value = (unsigned int)(-1);

	if (name == NULL)
	{
		MT_LOGE("mt_otp_get_obj: invalid arguments!\r\n");
		return (-1);
	}

	obj = get_obj_by_name(name);
	if (obj == NULL)
	{
		MT_LOGE("mt_otp_get_obj: invalid OTP name %s!\r\n", name);
		return (-1);
	}

	*value = otp_read_obj(obj);

	if (description)
		*description = get_description(obj, *value);

	return 0;
}

int mt_otp_get_chip_silkscreen(mt_chip_silkscreen_t *silkscr)
{
	int ret;
	char *s;
	unsigned int val;
	const char *desc;
	int i;

	const char *otp_name[] =
	{
		NULL,
		OTP_ProductType,			//-> product_line
		OTP_ChipFamily,				//-> product_family
		OTP_ChipGeneration,			//-> generation_code
		OTP_DisplayResolution,		//-> segment
		OTP_SubFeature,				//-> sub_segment

		OTP_PackageInfo,			//-> package_type
		OTP_SiPDRAMSize,			//-> sip_info
		OTP_CAVendor,				//-> cas_vendor
		OTP_Chipset_Version,		//-> chipset_version
		OTP_Bin_HWIP_Info,			//-> drm_watermark_cas_extension_bin
		OTP_IPLicense,				//-> license_royalty
	};

	if (silkscr == NULL)
	{
		return (-1);
	}

	memset(silkscr, 0, sizeof(mt_chip_silkscreen_t));
	s = silkscr->string;

	s[0] = 'M';		//1st character: fixed 'M'

	for (i=1; i<MAX_OTP_SILKSCREEN_SIZE; i++)
	{
		desc = NULL;

		ret = mt_otp_get_obj(otp_name[i], &val, &desc);
		if (ret != 0 || desc == NULL)
		{
			return (-1);
		}

		s[i] = desc[0];	/* first byte of the description */
	}

	return 0;
}

static void dump_obj(void *s, struct mt_otp_object *obj)
{
	unsigned int value;
	const char *desc = NULL;

	value = otp_read_obj(obj);
	desc = get_description(obj, value);

	DUMP_LOG("%30s 0x%4X %2u %2u 0x%8X 0x%8X %s - %s\r\n",
		obj->name,
		obj->offset,
		obj->shift,
		obj->width,
		obj->mask,
		value,
		obj->help==NULL?" N/A":obj->help,
		desc==NULL?"N/A":desc);
}

/*
 * Dump OTP state
 *
 * @param[in] s seq_file for Kernel
 *              NULL for Uboot
 *
 * @return void
 *
 */
void mt_otp_dump_state(void *s)
{
	int i;
	struct mt_otp_object *obj;

	DUMP_LOG("========================================[OTP]========================================\r\n");

	DUMP_LOG("%30s %6s %2s %2s %10s %10s %s - %s\r\n",
			"Name",
			"Offset",
			"Bi",
			"Wd",
			"Mask",
			"Value",
			"Help",
			"Description");

	for (i=0;;i++)
	{
		obj = get_obj_by_idx(i);

		if (obj == NULL || obj->name == NULL)
			break;

		dump_obj(s, obj);
	}

	DUMP_LOG("-------------------------------------------------------------------------------------\r\n");
}

#ifdef __UBOOT__
static int do_cmd_otp_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	mt_chip_silkscreen_t silk;

	mt_otp_get_chip_silkscreen(&silk);

	printf("OTP Chip Silk Screen: %s\r\n", silk.string);

	mt_otp_dump_state(NULL);

	return 0;
}

U_BOOT_CMD(
	otp_info,	1,	0,	do_cmd_otp_info,
	"print OTP information",
	""
);
#endif

