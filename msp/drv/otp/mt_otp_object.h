/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2023, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_otp_object.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2023/08/02
 * Description    : MT OTP Object Definition.
 * History        :
 * 1.Date         : 2023/08/02
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_OTP_OBJECT_H__
#define __INC_MT_OTP_OBJECT_H__

/* OTP Object information definition */
struct mt_otp_object
{
	const char *name;				/* OTP Object name */

	unsigned int bit_addr_flag: 1;	/* 1: offset is bit address, 0: register offset */

	unsigned int offset;			/* offset */
	unsigned int shift;				/* bit shift */
	unsigned int width;				/* bit width */
	unsigned int mask;				/* bit mask */

	const char *help;				/* help information */
	//const char **description[];		/* detail descriptions of OTP object values */
	const char **description;
};

#if 0
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
int mt_otp_get_obj(const char *name, unsigned int *value, const char **description);
#endif

/*
 * Dump OTP state
 *
 * @param[in] s seq_file for Kernel
 *              NULL for Uboot
 *
 * @return void
 *
 */
void mt_otp_dump_state(void *s);

#endif

