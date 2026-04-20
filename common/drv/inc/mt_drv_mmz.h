#ifndef __MT_DRV_MMZ_H__
#define __MT_DRV_MMZ_H__

#ifdef __KERNEL__

/* add include here */
#include <linux/version.h>

#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
//#include <linux/devfs_fs_kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/seq_file.h>
#include <linux/list.h>

#include <linux/uaccess.h>
//#include <asm/hardware.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
//#include <asm/arch/arm/mach-x5hd/include/mach/hardware.h>
//#include <linux/autoconf.h>
//#include <asm/sizes.h>
#include "../../inc/mt_type.h"

#endif

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/** @addtogroup H_MMZ */
/** @{ */

/***************************** Macro Definition ******************************/

/** MMZ attribute type/subtype definition */
#define MT_MMZ_TYPE_UND			0
	#define MT_MMZ_SUBTYPE_UND			0
#define MT_MMZ_TYPE_VIDEO		1
	#define MT_MMZ_SUBTYPE_VIDEO_ES		1
	#define MT_MMZ_SUBTYPE_VIDEO_FRAME 	2
#define MT_MMZ_TYPE_AUDIO		2
	#define MT_MMZ_SUBTYPE_AUDIO_ES		1
	#define MT_MMZ_SUBTYPE_AUDIO_PCM	2
#define MT_MMZ_TYPE_DISP		3
#define MT_MMZ_TYPE_GRAPHIC		4
#define MT_MMZ_TYPE_PVR			5
	#define MT_MMZ_SUBTYPE_PVR_REC		1
/* add your mmz type definition here: */
#define MT_MMZ_TYPE_MAX			6

/** MMZ notifier OP */
#define MT_MMZ_NOTIFY_ALLOC		0
#define MT_MMZ_NOTIFY_FREE		1

/*************************** Structure Definition ****************************/

/*media memory map structure*/
typedef struct mtmmz_buffer_s
{
#ifndef __KERNEL__
    mt_kern_ulong_t startVirAddr;
#else
    void *startVirAddr;
#endif
    phys_addr_t startPhyAddr;
#ifndef __KERNEL__
    mt_kern_ulong_t size;
#else
    ulong size;
#endif
}mmz_buffer_s;

#ifndef MMZ_BUFFER_S
#define MMZ_BUFFER_S mmz_buffer_s
#endif

/** extended MMZ attribute for security etc. */
typedef struct mt_drv_mmz_security_attr_s {
	mt_u32 flags;		/* security flags */
	mt_s32 type;		/* main type, such as MT_MMZ_TYPE_VIDEO/MT_MMZ_TYPE_AUDIO/... */
	mt_s32 subtype;		/* sub type, such as MT_MMZ_SUBTYPE_VIDEO_ES/MT_MMZ_SUBTYPE_AUDIO_ES/... */

	/* reserved for future usage */
	mt_u32 reserved[5];

} mmz_security_attr_s;

/** MMZ Notifier type definition. */
typedef void (*mt_drv_mmz_notify_t)(int op,	/* MT_MMZ_NOTIFY_ALLOC or MT_MMZ_NOTIFY_FREE */
									const char *zone_name,
									const char *name,
									unsigned long phys_addr, unsigned long length,
									mmz_security_attr_s *attr,
									void *priv);

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
/*alloc mmz memory, get physic address and map kernel-state address*/
/*CNcomment:申请mmz内存，得到物理地址，并做内核态地址的映射*/
mt_s32  mt_drv_mmz_alloc_and_map(const char *name, char *mmzzonename, ulong size, int align, mmz_buffer_s *psmbuf);

/*unmap kernel-state address, release mmz memory*/
/*CNcomment:解除内核态地址的映射，并释放mmz内存*/
mt_void mt_drv_mmz_unmap_and_release(mmz_buffer_s *psmbuf);

/*Only alloc mmz memory, return physic address, but not map kernel-state address*/
/*CNcomment:只申请mmz内存，返回物理地址，不做内核态地址的映射*/
mt_s32  mt_drv_mmz_alloc(const char *bufname, char *zone_name, ulong size, int align, mmz_buffer_s *psmbuf);

/*Only alloc mmz memory, return physic address, but not map kernel-state address*/
/*CNcomment:只申请mmz内存，返回物理地址，不做内核态地址的映射*/
mt_s32  mt_drv_mmz_alloc_secure(const char *bufname, char *zone_name, ulong size, int align, mmz_buffer_s *psmbuf,
								mmz_security_attr_s *attr);

/*map kernel-state address after alloc mmz memory for cache, and flushing cache with MT_DRV_MMZ_Flush*/
/*CNcomment:申请mmz可Cache内存后，进行内核态地址的映射, 并使用MT_DRV_MMZ_Flush进行cache同步 */
mt_s32 mt_drv_mmz_map_cache(mmz_buffer_s *psmbuf);

/*flush cache data to memory, needed to call when map memory with MT_DRV_MMZ_MapCache*/
/*CNcomment:使用MT_DRV_MMZ_MapCache时，需主动调用MT_DRV_MMZ_Flush进行cache数据同步 */
mt_s32 mt_drv_mmz_flush(mmz_buffer_s *psmbuf);

/*invalid cache, needed to call when map memory with MT_DRV_MMZ_MapCache*/
/*CNcomment:使用MT_DRV_MMZ_MapCache时，需主动调用MT_DRV_MMZ_Flush进行cache数据同步 */
mt_s32 mt_drv_mmz_invalid(mmz_buffer_s *psmbuf);

/*alloc mmz memory, and map kernel-state address*/
/*CNcomment:申请mmz内存后，进行内核态地址的映射*/
mt_s32  mt_drv_mmz_map(mmz_buffer_s *psmbuf);

/*unmap kernel-state address*/
/*CNcomment:解除内核态地址的映射*/
mt_void mt_drv_mmz_unmap(mmz_buffer_s *psmbuf);

/*release unmapped mmz memory */
/*CNcomment:解除映射后，或没有进行内核态映射的mmz内存进行释放*/
mt_void mt_drv_mmz_release(mmz_buffer_s *psmbuf);

/** register MMZ Notifier. */
mt_s32 mt_drv_mmz_register_notify(mt_drv_mmz_notify_t notifier, void *priv);

/** unregister MMZ Notifier. */
void mt_drv_mmz_unregister_notify(mt_drv_mmz_notify_t notifier, void *priv);

/** get MMZ Notifier. */
mt_drv_mmz_notify_t mt_drv_mmz_get_notify(void **priv);

void *mmz_va(mmz_buffer_s *psMBuf, phys_addr_t pa);
phys_addr_t mmz_pa(mmz_buffer_s *psMBuf, void *va);


/** @} */

int mt_drv_mmz_init(void);
void mt_drv_mmz_exit(void);
int drv_mmz_modinit(mt_void);
mt_void drv_mmz_modexit(mt_void);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif

