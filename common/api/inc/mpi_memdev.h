

#ifndef __MPI_MEMDEV_H__
#define __MPI_MEMDEV_H__

/******************************* Include Files *******************************/

/* add include here */
#include "mt_type.h"
#include "mt_module.h"
#include "mt_debug.h"
#include "mt_drv_memdev.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

mt_s32 mpi_memdev_init(mt_void);
mt_s32 mpi_memdev_deinit(mt_void);
/**
 * @brief write register
 *
 * @param[in] RegAddr register physical address(not mapped address)
 */
mt_s32 mpi_memdev_write_register(phys_addr_t RegAddr, mt_u32 u32Value);
/**
 * @brief read register
 *
 * @param[in] RegAddr register physical address(not mapped address)
 */
mt_s32 mpi_memdev_read_register(phys_addr_t RegAddr, mt_u32 *pu32Value);
mt_s32 mpi_memdev_map_register(phys_addr_t RegAddr, mt_u32 u32Length, mt_void **pVirAddr);
mt_s32 mpi_memdev_unmap_register(mt_void *pVirAddr);

#ifndef __KERNEL__
#ifdef CONFIG_MT_ARCH_ARM
#define __iowmb() asm volatile ("dsb st" : : : "memory")
#define __iormb(v) asm volatile ("dsb" : : : "memory")
#define __smp_mb() asm volatile ("dmb ish" : : : "memory")
#define __smp_rmb() __smp_mb()
#define __smp_wmb() asm volatile ("dmb ishst" : : : "memory")
#elif defined(CONFIG_MT_ARCH_AARCH64)
#define __iowmb() asm volatile ("dsb st" : : : "memory")
#define __iormb(v)							\
({									\
	unsigned long tmp;						\
									\
	asm volatile ("dsb ld" : : : "memory");				\
									\
	/*								\
	 * Create a dummy control dependency from the IO read to any	\
	 * later instructions. This ensures that a subsequent call to	\
	 * udelay() will be ordered due to the ISB in get_cycles().	\
	 */								\
	asm volatile("eor	%0, %1, %1\n"				\
		     "cbnz	%0, ."					\
		     : "=r" (tmp) : "r" ((unsigned long)(v))		\
		     : "memory");					\
})
#define __smp_mb() asm volatile ("dmb ish" : : : "memory")
#define __smp_rmb() asm volatile ("dmb ishld" : : : "memory")
#define __smp_wmb() asm volatile ("dmb ishst" : : : "memory")
#else /* CONFIG_MT_ARCH_MIPS */
#define __iowmb() asm volatile ("" : : : "memory")
#define __iormb(v) asm volatile ("" : : : "memory")
#define __smp_mb() asm volatile ("" : : : "memory")
#define __smp_rmb() asm volatile ("" : : : "memory")
#define __smp_wmb() asm volatile ("" : : : "memory")
#endif
#endif

/**
 * @brief read register
 *
 * @param[in] RegUsrVirtAddr mapped register address(not physical address)
 */
static inline mt_u32 mpi_read_reg32(void *RegUsrVirtAddr)
{
	mt_u32 __v = *(volatile mt_u32 *)(RegUsrVirtAddr);
	__iormb(__v);
	return __v;
}

/**
 * @brief write register
 *
 * @param[in] RegUsrVirtAddr mapped register address(not physical address)
 */
static inline mt_void mpi_write_reg32(void *RegUsrVirtAddr, mt_u32 u32Value)
{
	__iowmb();
	*(volatile mt_u32 *)(RegUsrVirtAddr) = u32Value;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MPI_MEMDEV_H__ */

