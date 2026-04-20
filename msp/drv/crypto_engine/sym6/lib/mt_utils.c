/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_utils.h"
#include "mt_lib.h"

void byteswap(void *dst, const void *src, mt_u32 len)
{
    mt_u8 *d = (mt_u8 *)dst;
    const mt_u8 *s = (const mt_u8 *)src + len -1;

    while (len--) {
        *d = *s;
        d++;
        s--;
    }
}

void buf_to_le_reg(void *reg, const void *buf, mt_u32 len)
{
    mt_u32 reg_size = len >> 2;
    mt_u32 *dst = (mt_u32 *)reg;
    const mt_u8 *p = (const mt_u8 *)buf + len - 4;
    mt_u32 value = 0;

    while (reg_size--) {
        value = p[3] | (p[2] << 8) | (p[1] << 16) | (p[0] << 24);
        io_write32((ulong)dst, value);
        dst++;
        p -= 4;
    }
}

void buf_to_be_reg(void *reg, const void *buf, mt_u32 len)
{
    mt_u32 reg_size = len >> 2;
    mt_u32 *dst = (mt_u32 *)reg;
    const mt_u8 *p = (const mt_u8 *)buf;
    mt_u32 value = 0;

    while (reg_size--) {
        value = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
        io_write32((ulong)dst, value);
        dst++;
        p += 4;
    }
}

void le_reg_to_buf(void *buf, const void *reg, mt_u32 len)
{
    mt_u32 reg_size = len >> 2;
    const mt_u32 *src = (const mt_u32 *)reg + reg_size - 1;
    mt_u8 *p = (mt_u8 *)buf;
    mt_u32 value = 0;

    while (reg_size--) {
        value = io_read32((ulong)src);
        src--;
        *p++ = (value >> 24) & 0xff;
        *p++ = (value >> 16) & 0xff;
        *p++ = (value >> 8) & 0xff;
        *p++ = value & 0xff;
    }
}

void be_reg_to_buf(void *buf, const void *reg, mt_u32 len)
{
    mt_u32 reg_size = len >> 2;
    const mt_u32 *src = (const mt_u32 *)reg;
    mt_u8 *p = (mt_u8 *)buf;
    mt_u32 value = 0;

    while (reg_size--) {
        value = io_read32((ulong)src);
        src++;
        *p++ = value & 0xff;
        *p++ = (value >> 8) & 0xff;
        *p++ = (value >> 16) & 0xff;
        *p++ = (value >> 24) & 0xff;
    }
}

void delay(mt_u32 count)
{
    volatile mt_u32 cnt = count;
    while (cnt--) {
        /* do nothing */
        ;
    }
}
