
#ifndef __MT_DRV_REG_H__
#define __MT_DRV_REG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

/*************** !!! important !!! ***************/
/*
never write reg like this:

    Result = 0x300;
    MT_REG_WRITE32(SYS_PERI_CRG3_ADDR, Result); <-------wrong!

always write reg like this:

    MT_REG_READ32(SYS_PERI_CRG3_ADDR, Result);  <----read reg first.
    Result = Result | 0x300;                    <----change the bits you want to set.
    MT_REG_WRITE32(SYS_PERI_CRG3_ADDR, Result); <----write the value back to reg.
*/
#ifndef MT_REG_READ8
#define MT_REG_READ8(addr,result)  ((result) = HAL_GET_U8((volatile u8 *)addr))
#endif

#ifndef MT_REG_READ16
#define MT_REG_READ16(addr,result)  ((result) = HAL_GET_U16((volatile u16 *)addr))
#endif

#ifndef MT_REG_READ32
#define MT_REG_READ32(addr,result)  ((result) = HAL_GET_U32((volatile u32 *)addr))
#endif

#ifndef MT_REG_WRITE8
#define MT_REG_WRITE8(addr,result)  (HAL_PUT_U8((volatile u8 *)(addr)), result)
#endif

#ifndef MT_REG_WRITE16
#define MT_REG_WRITE16(addr,result)  (HAL_PUT_U16((volatile u16 *)(addr)), result)
#endif

#ifndef MT_REG_WRITE32
#define MT_REG_WRITE32(addr,result)  (HAL_PUT_U32((volatile u32 *)(addr)), result)
#endif

#ifndef MT_REG_READ
#define MT_REG_READ MT_REG_READ32
#endif

#ifndef MT_REG_WRITE
#define MT_REG_WRITE MT_REG_WRITE32
#endif

#ifndef MT_REG_WriteBits
#define MT_REG_WriteBits(phyAddr, value, mask) do{ \
            MT_U32 __reg; \
            MT_REG_READ(IO_ADDRESS(phyAddr), __reg); \
            __reg = (__reg & ~(mask)) | ((value) & (mask)); \
            MT_REG_WRITE(IO_ADDRESS(phyAddr), __reg); \
        }while(0)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_REG_H__ */

