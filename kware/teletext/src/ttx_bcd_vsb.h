/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __TTX_BCD_VSB_H__
#define __TTX_BCD_VSB_H__

static inline u32 vbi_dec2bcd_vsb(u32 dec)
{
    return (dec % 10) + ((dec / 10) % 10) * 16 + ((dec / 100) % 10) * 256;
}

static inline u32 vbi_bcd2dec_vsb(u32 bcd)
{
    return (bcd & 15) + ((bcd >> 4) & 15) * 10 + ((bcd >> 8) & 15) * 100;
}

static inline u32 vbi_add_bcd_vsb(u32 a, u32 b)
{
    u32 t = 0;
    a += 0x06666666;
    t  = a + b;
    b ^= a ^ t;
    b  = (~b & 0x11111110) >> 3;
    b |= b * 2;

    return t - b;
}
#endif
