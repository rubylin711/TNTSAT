/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef  _MT_GFX_COMM_K_H_
#define  _MT_GFX_COMM_K_H_

#ifndef CONFIG_GFX_STB_SDK
#define CONFIG_GFX_STB_SDK
#endif

/***************************** SDK Version Macro Definition *********************/

/** \addtogroup 	GFX COMM */
/** @{ */  /** <!-- 【GFX COMM】 */


/** @} */	/*! <!-- Macro Definition end */


/*********************************add include here******************************/

#include "mt_type.h"
#include "mt_drv_log.h"
#include "mt_debug.h"

#include <linux/sched/clock.h>

#if defined(CONFIG_GFX_STB_SDK) || defined(CONFIG_GFX_ANDROID_SDK) || defined(CONFIG_GFX_TV_SDK)
#include "mt_debug.h"
#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_drv_dev.h"
#include "mt_drv_mmz.h"
#include "mt_drv_mem.h"
#include "drv_disp_ext.h"
#include "mt_drv_proc.h"
#include "mt_kernel_adapt.h"
#include "mt_drv_sys.h"
#elif defined(CONFIG_GFX_BVT_SDK)
#include <linux/fs.h>
#endif


/***************************** Macro Definition ******************************/

/** \addtogroup 	GFX COMM */
/** @{ */  /** <!-- 【GFX COMM】 */

/** this macro define at CFG_MT_KMOD_CFLAGS,so Makefile should include CFG_MT_KMOD_CFLAGS **/
#ifdef MT_ADVCA_FUNCTION_RELEASE
#define  CONFIG_GFX_ADVCA_RELEASE
#endif


#ifdef CONFIG_GFX_ADVCA_RELEASE
/** char disable */
/** CNcomment:char使能 CNend */
#define  CONFIG_GFX_COMM_STR_DISABLE
/** PROC disable */
/** CNcomment:proc使能 CNend */
#define  CONFIG_GFX_COMM_PROC_DISABLE
/** version info disable */
/** CNcomment:版本信息使能 CNend */
#define  CONFIG_GFX_COMM_VERSION_DISABLE
#endif

#ifdef CONFIG_GFX_BVT_SDK
#define  CONFIG_GFX_COMM_PROC_DISABLE
#endif
/** close the string function */
/** CNcomment:关闭字符串功能,DEBUG必须关闭 CNend */
#ifdef   CONFIG_GFX_COMM_STR_DISABLE
/** LOG disable */
/** CNcomment:log使能 CNend */
#define  CONFIG_GFX_COMM_DEBUG_DISABLE
#endif

/** pm disable */
/** CNcomment:pm使能 CNend */
//#define  CONFIG_GFX_COMM_PM_DISABLE


/** register mammap operate */
/** CNcomment:寄存器映射操作 CNend */
#define MT_GFX_REG_MAP(base, size)                    ioremap_nocache((base), (size))
/** register unmap operate */
/** CNcomment:寄存器逆映射操作 CNend */
#define MT_GFX_REG_UNMAP(base) 	                    iounmap((mt_void*)(base))


#ifdef CONFIG_GFX_256BYTE_ALIGN   /** 定义到Makefile和Android Makefile中 **/
#define GFX_MMZ_ALIGN_BYTES     256
#else
#define GFX_MMZ_ALIGN_BYTES     16
#endif


#ifdef CONFIG_GFX_COMM_STR_DISABLE
	#define SEQ_Printf(fmt...)                        {do{}while(0);}
	#define GFX_Printk(fmt...)                        {do{}while(0);}
#else
	#define SEQ_Printf                                  PROC_PRINT
	#ifndef CONFIG_GFX_BVT_SDK
	  #define GFX_Printk                                  MT_PRINT
	#else
	  #define GFX_Printk                                  MT_INFO_LOG
	#endif
#endif


/** the mutex init */
/** CNcomment:信号量初始化 CNend */
#define MT_GFX_INIT_MUTEX(x)                           sema_init(x, 1)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
    #define MT_GFX_DECLARE_MUTEX(x)                        MT_DECLARE_MUTEX(x)
#else
    #define MT_GFX_DECLARE_MUTEX(x)                        DEFINE_SEMAPHORE(x, 1)
#endif


#if defined(CONFIG_GFX_STB_SDK) || defined(CONFIG_GFX_ANDROID_SDK) || defined(CONFIG_GFX_TV_SDK)
	/** kmalloc mem */
	/** CNcomment:内核分配内存 CNend */
	#define MT_GFX_KMALLOC(module_id, size, flags)      MT_KMALLOC(ConvertID(module_id), size, flags)
	/** kfree mem */
	/** CNcomment:内核释放内存 */
	#define MT_GFX_KFREE(module_id, addr)                MT_KFREE(ConvertID(module_id), addr)
	#define MT_GFX_VMALLOC(module_id, size)              MT_VMALLOC(ConvertID(module_id), size)
	#define MT_GFX_VFREE(module_id, addr)                MT_VFREE(ConvertID(module_id), addr)
    #define ConvertID(module_id) (module_id + MT_ID_TDE - MTGFX_TDE_ID)

	#ifdef CONFIG_GFX_COMM_DEBUG_DISABLE
	    #define MT_GFX_COMM_LOG_FATAL(module_id,fmt...)
	    #define MT_GFX_COMM_LOG_ERROR(module_id,fmt...)
	    #define MT_GFX_COMM_LOG_WARNING(module_id,fmt...)
	    #define MT_GFX_COMM_LOG_INFO(module_id,fmt...)
	#else
	    #define MT_GFX_COMM_LOG_FATAL(module_id, fmt...)   MT_TRACE(MT_LOG_LEVEL_FATAL, ConvertID(module_id), fmt)
	    #define MT_GFX_COMM_LOG_ERROR(module_id,fmt...)    MT_TRACE(MT_LOG_LEVEL_ERROR, ConvertID(module_id), fmt)
	    #define MT_GFX_COMM_LOG_WARNING(module_id,fmt...)  MT_TRACE(MT_LOG_LEVEL_WARNING, ConvertID(module_id), fmt)
	    #define MT_GFX_COMM_LOG_INFO(module_id,fmt...)     MT_TRACE(MT_LOG_LEVEL_INFO, ConvertID(module_id), fmt)
	#endif

	#ifndef CONFIG_GFX_COMM_PM_DISABLE
    	#define DECLARE_GFX_NODE(gfx_name,gfx_open, gfx_release, gfx_mmap, gfx_ioctl, gfx_suspend, gfx_resume) \
        static struct file_operations gfx_fops =\
        {\
            .owner   = THIS_MODULE,      \
            .unlocked_ioctl = gfx_ioctl,\
            .open    = gfx_open,        \
            .release = gfx_release,     \
            .mmap	 = gfx_mmap         \
        };\
        static basedrv_s gfx_drvops = \
        {\
            .suspend      = gfx_suspend,\
            .resume       = gfx_resume  \
        };\
        static PM_DEVICE_S gfx_dev = \
        {\
            .name  = gfx_name,            \
            .minor = MT_DYNAMIC_MINOR,\
            .owner = THIS_MODULE,          \
            .app_ops = &gfx_fops,          \
            .base_ops = &gfx_drvops        \
        }
    #else
        #define DECLARE_GFX_NODE(gfx_name, gfx_open, gfx_release, gfx_mmap, gfx_ioctl, gfx_suspend, gfx_resume)  \
        static struct file_operations gfx_fops =\
        {\
            .owner   = THIS_MODULE,       \
            .unlocked_ioctl = gfx_ioctl, \
            .open    = gfx_open,         \
            .release = gfx_release,      \
            .mmap	 = gfx_mmap          \
        };\
        static basedrv_s gfx_drvops = \
        {\
            .suspend      = gfx_suspend,\
            .resume       = gfx_resume  \
        };\
        static PM_DEVICE_S gfx_dev = \
        {\
            .name  = gfx_name,            \
            .minor = MT_DYNAMIC_MINOR,\
            .owner = THIS_MODULE,          \
            .app_ops = &gfx_fops           \
        }
 	#endif

#elif defined(CONFIG_GFX_BVT_SDK)
	/** kmalloc mem */
	/** CNcomment:内核分配内存 CNend */
   	#define MT_GFX_KMALLOC(module_id, size, flags)       kmalloc(size, flags)
    #define MT_GFX_KFREE(module_id, addr)                 kfree(addr)
    #define MT_GFX_VMALLOC(module_id, size)               vmalloc(size)
    #define MT_GFX_VFREE(module_id, addr)                 vfree(addr)
    #define MMB_ADDR_INVALID (~0UL)

	/** kfree mem */
	/** CNcomment:内核释放内存 CNend */
	#define ConvertID(module_id) (module_id + MT_ID_TDE - MTGFX_TDE_ID)

	#ifdef CONFIG_GFX_COMM_DEBUG_DISABLE
	    #define MT_GFX_COMM_LOG_FATAL(module_id,fmt...)
	    #define MT_GFX_COMM_LOG_ERROR(module_id,fmt...)
	    #define MT_GFX_COMM_LOG_WARNING(module_id,fmt...)
	    #define MT_GFX_COMM_LOG_INFO(module_id,fmt...)

	#else
	    #define MT_GFX_COMM_LOG_FATAL(module_id, fmt...)   MT_TRACE(MT_LOG_LEVEL_FATAL,    ConvertID(module_id), fmt)
	    #define MT_GFX_COMM_LOG_ERROR(module_id,fmt...)    MT_TRACE(MT_LOG_LEVEL_ERROR,    ConvertID(module_id), fmt)
	    #define MT_GFX_COMM_LOG_WARNING(module_id,fmt...)  MT_TRACE(MT_LOG_LEVEL_WARNING,  ConvertID(module_id), fmt)
	    #define MT_GFX_COMM_LOG_INFO(module_id,fmt...)     MT_TRACE(MT_LOG_LEVEL_INFO,     ConvertID(module_id), fmt)
    #endif

	#define DECLARE_GFX_NODE(gfx_name,gfx_open, gfx_release, gfx_mmap, gfx_ioctl, gfx_suspend, gfx_resume) \
    struct file_operations gfx_fops =\
    {\
        .owner   = THIS_MODULE,      \
        .unlocked_ioctl = gfx_ioctl,\
        .open    = gfx_open,        \
        .release = gfx_release,     \
        .mmap	 = gfx_mmap         \
    };\
    static struct miscdevice gfx_dev =\
    {\
        MISC_DYNAMIC_MINOR,\
        gfx_name,         \
        &gfx_fops          \
    }
	#define MKSTR(exp) # exp
	#define MKMARCOTOSTR(exp) MKSTR(exp)
#else

#endif


/** @} */	/*! <!-- Macro Definition end */


/*************************** Enum Definition ****************************/

/** \addtogroup 	 GFX COMM */
/** @{ */  /** <!-- 【GFX COMM】 */


/** enum of the chip type */
/** CNcomment:芯片类型枚举 CNend */
typedef enum tagMTGFX_CHIP_TYPE_E
{
    MTGFX_CHIP_TYPE_MT_ARIA = 0,   /**< aria */
    MTGFX_CHIP_TYPE_MT_SYMPHONY,   /**< symphony */

    MTGFX_CHIP_TYPE_BUTT        = 400  /**< Invalid Chip */

}MTGFX_CHIP_TYPE_E;


/** enum of the module ID */
/** CNcomment:每个模块的ID号 CNend */
typedef enum tagMTGFX_MODE_ID_E
{

	MTGFX_TDE_ID      = 0,    /**< TDE ID         */
	MTGFX_JPGDEC_ID,          /**< JPEG DECODE ID */
	MTGFX_JPGENC_ID,          /**< JPEG_ENCODE ID */
	MTGFX_FB_ID,              /**<  FRAMEBUFFER ID */
	MTGFX_PNG_ID,             /**< PNG ID          */
	MTGFX_MTGO_ID,
	MTGFX_GFX2D_ID,
	MTGFX_BUTT_ID,

}MTGFX_MODE_ID_E;


/** @} */  /*! <!-- enum Definition end */

/*************************** Structure Definition ****************************/


/** \addtogroup 	 GFX COMM */
/** @{ */  /** <!-- 【GFX COMM】 */


/** Structure of proc item */
/** CNcomment:proc相关函数操作 CNend */
typedef struct struGFX_PROC_ITEM
{
	mt_s32 (*fnRead)(struct seq_file *, mt_void *);
	mt_s32 (*fnWrite)(struct file * file,  const char __user * buf, size_t count, loff_t *ppos);
	mt_s32 (*fnIoctl)(struct seq_file *, mt_u32 cmd, mt_u32 arg);
}GFX_PROC_ITEM_S;


/** @} */  /*! <!-- Structure Definition end */


/********************** Global Variable declaration **************************/


/******************************* API declaration *****************************/

/** \addtogroup 	 GFX COMM */
/** @{ */  /** <!-- 【GFX COMM】 */


/**
\brief show sdk version. CNcomment:显示模块版本号 CNend\n
\attention \n
if you want to show module version when insmod ko,call this function.\n
CNcomment:当要显示模块版本号的时候调用该接口 CNend\n

\param[in]	ModID. CNcomment:模块ID CNend

\retval ::NA

\see \n
::MT_GFX_ShowVersionK
*/
//static inline mt_void MT_GFX_ShowVersionK(MTGFX_MODE_ID_E ModID)
//{
	/* comment by HY, add later
	#if !defined(CONFIG_GFX_COMM_VERSION_DISABLE) && !defined(CONFIG_GFX_COMM_DEBUG_DISABLE)

    	mt_char MouleName[7][10] = {"tde","jpegdec","jpegenc","fb","png", "mtgo", "gfx2d"};
        mt_char Version[160] ="SDK_VERSION:["MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
		__DATE__", "__TIME__"]";

    	if (ModID >= HIGFX_BUTT_ID)
    		return;

	if ((HIGFX_JPGDEC_ID == ModID) || (HIGFX_JPGENC_ID == ModID))
		GFX_Printk("Load mt_%s.ko success.\t(%s)\n", MouleName[ModID],Version);
	else
		GFX_Printk("Load mt_%s.ko success.\t\t(%s)\n", MouleName[ModID],Version);

	return;

	#endif
	*/
//}

/**
\brief get time function. CNcomment:获取时间函数 CNend\n
\attention \n

\param[in]	ModID. CNcomment:模块ID CNend\n

\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GFX_GetTimeStamp
*/
static inline mt_s32 MT_GFX_GetTimeStamp(mt_u32 *pu32TimeMs, mt_u32 *pu32TimeUs)
{
	mt_u64 u64TimeNow;
    mt_u64 ns;

    if(MT_NULL == pu32TimeMs)
	{
		return MT_FAILURE;
	}

	u64TimeNow = sched_clock();

	*pu32TimeMs = (mt_u32)iter_div_u64_rem(u64TimeNow,1000000,&ns);

	return MT_SUCCESS;
}


#if defined(CONFIG_GFX_STB_SDK) || defined(CONFIG_GFX_ANDROID_SDK) || defined(CONFIG_GFX_TV_SDK)

/**
\brief free the mem that has alloced. CNcomment:释放分配过的内存 CNend
\attention \n

\param[in]	u32Phyaddr. CNcomment:物理地址 CNend\n

\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GFX_FreeMem
*/
static inline mt_void MT_GFX_FreeMem(phys_addr_t u32Phyaddr)
{
        mmz_buffer_s stBuffer;
        stBuffer.startPhyAddr = u32Phyaddr;
        mt_drv_mmz_release(&stBuffer);
}


/**
\brief alloc the mem that need. CNcomment:分配需要的内存 CNend\n
\attention \n

\param[in]	pName.        CNcomment:模块名   CNend\n
\param[in]	pZoneName.
\param[in]	u32LayerSize. CNcomment:内存大小 CNend\n

\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GFX_AllocMem
*/
static inline phys_addr_t MT_GFX_AllocMem(mt_char *pName, mt_char* pZoneName, mt_u32 u32LayerSize)
{

	    phys_addr_t addr;
	    mmz_buffer_s stBuffer;

	    if ((u32LayerSize == 0) || (u32LayerSize > 0x40000000))
	    {
	        return 0;
	    }
	    /* three time mem mangent*/
	    if(MT_SUCCESS == mt_drv_mmz_alloc(pName, pZoneName, u32LayerSize, GFX_MMZ_ALIGN_BYTES, &stBuffer))
	    {
	        addr = stBuffer.startPhyAddr;
	    }
	    else if(MT_SUCCESS == mt_drv_mmz_alloc(pName, "graphics", u32LayerSize, GFX_MMZ_ALIGN_BYTES, &stBuffer))
	    {
	        addr = stBuffer.startPhyAddr;
	    }
	    else if(MT_SUCCESS == mt_drv_mmz_alloc(pName, NULL, u32LayerSize, GFX_MMZ_ALIGN_BYTES, &stBuffer))
	    {
	        addr = stBuffer.startPhyAddr;
	    }
	    else
		{
	        addr = 0;
		}
		return addr;

}


static inline mt_void *MT_GFX_Map(phys_addr_t u32PhyAddr)
{

	    mmz_buffer_s stBuffer;

	    stBuffer.startPhyAddr = u32PhyAddr;
	    if(MT_SUCCESS == mt_drv_mmz_map(&stBuffer))
	     {
	         return ((unsigned char *)stBuffer.startVirAddr);
	     }
	     else
	     {
	         return MT_NULL;
	     }
}

static inline mt_void *MT_GFX_MapCached(phys_addr_t u32PhyAddr)
{

	    mmz_buffer_s stBuffer;
	    stBuffer.startPhyAddr = u32PhyAddr;
	    if(MT_SUCCESS == mt_drv_mmz_map_cache(&stBuffer))
	    {
	        return ((unsigned char *)stBuffer.startVirAddr);
	    }
	    else
	    {
	        return MT_NULL;
	    }
}
static inline mt_s32 MT_GFX_Unmap(mt_void *pViraddr)
{

    mmz_buffer_s stBuffer;
    stBuffer.startVirAddr = pViraddr;
    mt_drv_mmz_unmap(&stBuffer);

    return MT_SUCCESS;

}


static inline mt_s32 MT_GFX_PROC_AddModule(mt_char * pEntry_name, GFX_PROC_ITEM_S* pProcItem, mt_void *pData)
{
    #ifndef  CONFIG_GFX_COMM_PROC_DISABLE
		mt_drv_proc_t stProcItem;
		stProcItem.fnIoctl =  pProcItem->fnIoctl;
		stProcItem.fnRead = pProcItem->fnRead;
		stProcItem.fnWrite= pProcItem->fnWrite;
		mt_drv_proc_add_module(pEntry_name, &stProcItem, pData);
    #endif
	return 0;

}

static inline mt_void MT_GFX_PROC_RemoveModule(mt_char *pEntry_name)
{
    #ifndef  CONFIG_GFX_COMM_PROC_DISABLE
	  mt_drv_proc_rm_module(pEntry_name);
    #endif
}

static inline mt_s32 MT_GFX_MODULE_Register(mt_u32 u32ModuleID, const mt_char * pszModuleName, mt_void *pData)
{
	return mt_drv_module_register(ConvertID(u32ModuleID), pszModuleName, pData);
}

static inline mt_s32 MT_GFX_MODULE_UnRegister(mt_u32 u32ModuleID)
{
	return mt_drv_module_unregister(ConvertID(u32ModuleID));
}

#define MT_GFX_PM_Register()  mt_drv_dev_register(&gfx_dev);

#define MT_GFX_PM_UnRegister()  mt_drv_dev_unregister(&gfx_dev);

static inline mt_void MT_GFX_SYS_GetChipVersion(MTGFX_CHIP_TYPE_E *penChipType)
{
#ifdef CONFIG_MT_CHIP_ARIA
    MT_CHIP_TYPE_E    ChipType = MT_CHIP_TYPE_MT_ARIA;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)  || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
    MT_CHIP_TYPE_E    ChipType = MT_CHIP_TYPE_MT_SYMPHONY;
#endif
    MT_CHIP_VERSION_E ChipVersion = MT_CHIP_VERSION_BUTT;

    mt_drv_sys_getchipversion(&ChipType, &ChipVersion);

#ifdef CONFIG_MT_CHIP_ARIA
    if ((MT_CHIP_TYPE_MT_ARIA == ChipType) && (ChipVersion == MT_CHIP_VERSION_V100))
    {
        *penChipType = MTGFX_CHIP_TYPE_MT_ARIA;
		return;
    }
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
    *penChipType = MTGFX_CHIP_TYPE_MT_SYMPHONY;
#endif
}


#elif defined(CONFIG_GFX_BVT_SDK)


/**
\brief free the mem that has alloced. CNcomment:释放分配过的内存 CNend\n
\attention \n

\param[in]	u32Phyaddr. CNcomment:物理地址 CNend\n

\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GFX_FreeMem
*/
static inline mt_void MT_GFX_FreeMem(phys_addr_t u32Phyaddr)
{
}

/**
\brief alloc the mem that need. CNcomment:分配需要的内存 CNend\n
\attention \n

\param[in]	pName.        CNcomment:模块名   CNend\n
\param[in]	pZoneName.
\param[in]	u32LayerSize. CNcomment:内存大小 CNend\n

\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GFX_AllocMem
*/
static inline phys_addr_t MT_GFX_AllocMem(mt_char *pName, mt_char* pZoneName, mt_u32 u32LayerSize)
{
	return 0;
}


static inline mt_void *MT_GFX_Map(phys_addr_t u32PhyAddr)
{
    return MT_NULL;
}

static inline mt_void *MT_GFX_MapCached(phys_addr_t u32PhyAddr)
{
    return MT_NULL;
}
static inline mt_s32 MT_GFX_Unmap(mt_void *pViraddr)
{
    return MT_SUCCESS;
}

static inline mt_s32 MT_GFX_PROC_AddModule(mt_char * pEntry_name, GFX_PROC_ITEM_S* pProcItem, mt_void *pData)
{
    #ifndef  CONFIG_GFX_COMM_PROC_DISABLE
	    CMPI_PROC_ITEM_S *pProcItem;
	    pProcItem = CMPI_CreateProc(pEntry_name, pProcItem->fnRead, pData);
	    pProcItem->write = pProcItem->fnWrite
	    pProcItem->pData = pData;
    #endif
	return 0;

}

static inline mt_void MT_GFX_PROC_RemoveModule(mt_char *pEntry_name)
{
    #ifndef  CONFIG_GFX_COMM_PROC_DISABLE
	  CMPI_RemoveProc(pEntry_name);
    #endif
}

static inline mt_s32 MT_GFX_MODULE_Register(mt_u32 u32ModuleID, const mt_char * pszModuleName,mt_void *pData)
{
	return MT_SUCCESS;
}

static inline  mt_s32 MT_GFX_MODULE_UnRegister(mt_u32 u32ModuleID)
{
	return MT_SUCCESS;
}

#define MT_GFX_PM_Register()    misc_register(&gfx_dev);
#define MT_GFX_PM_UnRegister()  misc_deregister(&gfx_dev);

static inline mt_void MT_GFX_SYS_GetChipVersion(HIGFX_CHIP_TYPE_E *penChipType)
{
    *penChipType = MTGFX_CHIP_TYPE_MT_ARIA;
}
#else


#endif

/** @} */  /*! <!-- API declaration end */


#endif /*_MT_GFX_COMM_K_H_ */
