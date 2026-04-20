/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _MT_UTILS_H_
#define _MT_UTILS_H_

#include "mt_type.h"

#ifndef STR
#define _STR(name) #name
#define STR(name) _STR(name)
#endif

#ifndef MIN
#define MIN(x, y) __extension__ ({ \
    __typeof__(x) _x = (x); \
    __typeof__(y) _y = (y); \
    (void)(&_x == &_y); \
    _x < _y ? _x : _y; \
    })
#endif

#ifndef MAX
#define MAX(x, y) __extension__ ({ \
    __typeof__(x) _x = (x); \
    __typeof__(y) _y = (y); \
    (void)(&_x == &_y); \
    _x > _y ? _x : _y; \
    })
#endif

#ifndef REG32
#define REG32(addr) (*(volatile unsigned int *)(addr))
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifdef LINUX
#include <linux/unaligned/packed_struct.h>

static inline mt_u32 get_unaligned_u32(const void *p)
{
    return __get_unaligned_cpu32(p);
}

static inline void put_unaligned_u32(mt_u32 val, void *p)
{
    __put_unaligned_cpu32(val, p);
}

#else

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) sizeof(a)/sizeof((a)[0])
#endif

struct __una_u32 {mt_u32 x;} __attribute__((__packed__));

static inline mt_u32 get_unaligned_u32(const void *p)
{
    const struct __una_u32 *ptr = (const struct __una_u32 *)p;
    return ptr->x;
}

static inline void put_unaligned_u32(mt_u32 val, void *p)
{
    struct __una_u32 *ptr = (struct __una_u32 *)p;
    ptr->x = val;
}
#endif

static inline mt_u32 get_u32(const void *p)
{
    if ((mt_u32)p & 0x3)
        return get_unaligned_u32(p);

    return REG32(p);
}

static inline void put_u32(mt_u32 val, void *p)
{
    if ((mt_u32)p & 0x3)
        put_unaligned_u32(val, p);
    else
        REG32(p) = val;
}

#ifdef LINUX
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

static inline mt_u32 io_read32(mt_u32 addr)
{
    return HAL_GET_U32((volatile void *)addr);
}

static inline void io_write32(mt_u32 addr, mt_u32 val)
{
    HAL_PUT_U32((volatile void *)addr, val);
}
#else
static inline mt_u32 io_read32(mt_u32 addr)
{
    return REG32(addr);
}

static inline void io_write32(mt_u32 addr, mt_u32 val)
{
    REG32(addr) = val;
}

static inline void *virt_to_phys(const volatile void *addr)
{
    return (void *)addr;
}
#endif

void byteswap(void *dst, const void *src, mt_u32 len);
void buf_to_le_reg(void *reg, const void *buf, mt_u32 len);
void buf_to_be_reg(void *reg, const void *buf, mt_u32 len);
void le_reg_to_buf(void *buf, const void *reg, mt_u32 len);
void be_reg_to_buf(void *buf, const void *reg, mt_u32 len);

void delay(mt_u32 count);

#endif /*_MT_UTILS_H_*/
