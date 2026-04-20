/******************************************************************************
*
* Copyright (C) 2014 Montage Technologies Co., Ltd.  All rights reserved.
*
* This program is confidential and proprietary to Montage  Technologies Co., Ltd. (Montage),
* and may not be copied, reproduced, modified, disclosed to others, published or used, in
* whole or in part, without the express prior written permission of Montage.
*
******************************************************************************
File Name	    : mt_gfx_comm.h
Version		    : version 1.0
Author		    :
Created		    : 2014/06/20
Description	    : Describes adp file. CNcomment:API跨平台适配 CNend\n
Function List 	:

History       	:
Date				Author        		Modification
2014/06/20		    y00181162
******************************************************************************/


#ifndef  _MT_GFX_COMM_H_
#define  _MT_GFX_COMM_H_


/***************************** SDK Version Macro Definition *********************/

/** \addtogroup 	GFX COMM */
/** @{ */  /** <!-- 【GFX COMM】 */

/** @} */	/*! <!-- Macro Definition end */


/*********************************add include here******************************/
#include "mt_type.h"

#define CONFIG_GFX_STB_SDK

#if defined(CONFIG_GFX_STB_SDK) || defined(CONFIG_GFX_TV_SDK)
   #include "mt_common.h"
#elif defined(CONFIG_GFX_ANDROID_SDK)
   #include <sys/ioctl.h>
   #include <utils/Log.h>
   #include <sys/syscall.h>
   #include <linux/ion.h>
   #include <sys/mman.h>
   #include <fcntl.h>
#elif defined(CONFIG_GFX_BVT_SDK)
	#include <time.h>
	#include <sys/ioctl.h>
    #include <sys/mman.h>
	#include <string.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include "drv_mmz_ioctl.h"
#else

#endif

#ifdef CONFIG_GFX_256BYTE_ALIGN /** 定义到Makefile和Android Makefile中 **/
#include "mt_math.h"
#endif

/***************************** Macro Definition ******************************/

/** \addtogroup 	GFX COMM */
/** @{ */  /** <!-- 【GFX COMM】 */
/** this macro define at CFG_MT_CFLAGS,so Makefile should include CFG_MT_CFLAGS **/
#ifdef MT_ADVCA_FUNCTION_RELEASE
#define  CONFIG_GFX_ADVCA_RELEASE
#endif

#ifdef CONFIG_GFX_ADVCA_RELEASE
/** char disable */
/** CNcomment:char使能 CNend */
#define  CONFIG_GFX_COMM_STR_DISABLE
#endif

/** close the string function */
/** CNcomment:关闭字符串功能,DEBUG必须关闭 CNend */
#ifdef   CONFIG_GFX_COMM_STR_DISABLE
/** LOG disable */
/** CNcomment:log使能 CNend */
#define  CONFIG_GFX_COMM_DEBUG_DISABLE
#endif


#ifdef CONFIG_GFX_COMM_STR_DISABLE
	#define GFX_Printf( fmt,args...)
#else
	#ifdef CONFIG_GFX_ANDROID_SDK
		#if 0
		/** this is defined at *.c that will used **/
		#define LOG_TAG    "libjpeg"
		#endif
		#define GFX_Printf( fmt, args... )\
		do { \
				ALOGE(fmt, ##args );\
		} while (0)
    #else
		#define GFX_Printf( fmt, args... )\
		do { \
				fprintf(stderr,fmt, ##args );\
		} while (0)
	#endif
#endif

#if defined(CONFIG_GFX_STB_SDK) || defined(CONFIG_GFX_TV_SDK)

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

#elif defined(CONFIG_GFX_ANDROID_SDK)

	#define ConvertID(module_id) (module_id + MT_ID_TDE - MTGFX_TDE_ID)

	#ifdef CONFIG_GFX_COMM_DEBUG_DISABLE
		#define MT_GFX_COMM_LOG_FATAL(module_id,fmt...)
		#define MT_GFX_COMM_LOG_ERROR(module_id,fmt...)
		#define MT_GFX_COMM_LOG_WARNING(module_id,fmt...)
		#define MT_GFX_COMM_LOG_INFO(module_id,fmt...)
	#else
		#define MT_GFX_COMM_LOG_FATAL(module_id, fmt...)    MT_TRACE(MT_LOG_LEVEL_FATAL, ConvertID(module_id), fmt)
		#define MT_GFX_COMM_LOG_ERROR(module_id,fmt...)	  MT_TRACE(MT_LOG_LEVEL_ERROR, ConvertID(module_id), fmt)
		#define MT_GFX_COMM_LOG_WARNING(module_id,fmt...)   MT_TRACE(MT_LOG_LEVEL_WARNING, ConvertID(module_id), fmt)
		#define MT_GFX_COMM_LOG_INFO(module_id,fmt...)	  MT_TRACE(MT_LOG_LEVEL_INFO, ConvertID(module_id), fmt)
	#endif

#elif defined(CONFIG_GFX_BVT_SDK)

	#define ConvertID(module_id) (module_id + MT_ID_TDE - MTGFX_TDE_ID)

	#ifdef CONFIG_GFX_COMM_DEBUG_DISABLE
	    #define MT_GFX_COMM_LOG_FATAL(module_id,fmt...)
	    #define MT_GFX_COMM_LOG_ERROR(module_id,fmt...)
	    #define MT_GFX_COMM_LOG_WARNING(module_id,fmt...)
	    #define MT_GFX_COMM_LOG_INFO(module_id,fmt...)

	#else
		#define MT_GFX_COMM_LOG_FATAL(module_id, fmt...)        MT_TRACE(MT_LOG_LEVEL_FATAL,	  ConvertID(module_id), fmt)
		#define MT_GFX_COMM_LOG_ERROR(module_id,fmt...)	      MT_TRACE(MT_LOG_LEVEL_ERROR,	  ConvertID(module_id), fmt)
		#define MT_GFX_COMM_LOG_WARNING(module_id,fmt...)       MT_TRACE(MT_LOG_LEVEL_WARNING,  ConvertID(module_id), fmt)
		#define MT_GFX_COMM_LOG_INFO(module_id,fmt...)	      MT_TRACE(MT_LOG_LEVEL_INFO,	  ConvertID(module_id), fmt)
    #endif

#else


#endif




#ifndef CLOCK_MONOTONIC_RAW
	#define CLOCK_MONOTONIC_RAW    4
#endif

/** @} */	/*! <!-- Macro Definition end */


/*************************** Enum Definition ****************************/

/** \addtogroup 	 GFX COMM */
/** @{ */  /** <!-- 【GFX COMM】 */

/** enum of the module ID */
/** CNcomment:每个模块的ID号 CNend */
typedef enum tagMTGFX_MODE_ID_E
{

	MTGFX_TDE_ID      = 0,    /**< TDE ID         */
	MTGFX_JPGDEC_ID,          /**< JPEG DECODE ID */
	MTGFX_JPGENC_ID,          /**< JPEG_ENCODE ID */
	MTGFX_FB_ID,              /**<  FRAMEBUFFER ID */
	MTGFX_PNG_ID,             /**< PNG ID          */
	MTGFX_BUTT_ID,

}MTGFX_MODE_ID_E;


/** @} */  /*! <!-- enum Definition end */

/*************************** Structure Definition ****************************/


/** \addtogroup 	 GFX COMM */
/** @{ */  /** <!-- 【GFX COMM】 */

#ifdef CONFIG_GFX_ANDROID_SDK
typedef struct
{
	struct ion_handle *pIonHandle;
	mt_s32 s32MapFd;
	phys_addr_t u32Phyaddr;
	mt_void *pViraddr;
	mt_u32 u32Size;
}MT_GFX_MEM_HANDLE_S;
#endif


/** @} */  /*! <!-- Structure Definition end */


/********************** Global Variable declaration **************************/


/******************************* API declaration *****************************/

/** \addtogroup 	 GFX COMM */
/** @{ */  /** <!-- 【GFX COMM】 */


/**
\brief get the system time,not use gettimeofday to get time. CNcomment:获取系统时间，不使用gettimeofday的原因在于这个函数获取的时间有可能被客户的后台程序修改 CNend
\attention \n

\param[out] *pu32TimeMs  CNcomment:获取到的时间ms CNend\n
\param[out] *pu32TimeUs  CNcomment:获取到的时间us CNend\n
\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GFX_GetTimeStamp
*/
static inline mt_s32 MT_GFX_GetTimeStamp(mt_u32 *pu32TimeMs, mt_u32 *pu32TimeUs)
{

		mt_s32 ret;
		struct timespec timenow = {0, 0};
		clockid_t id = CLOCK_MONOTONIC_RAW;

		if(MT_NULL == pu32TimeMs)
		{
			return MT_FAILURE;
		}

		ret = clock_gettime(id, &timenow);
		if(ret < 0)
		{
			return MT_FAILURE;
		}

		*pu32TimeMs = (mt_u32)(timenow.tv_sec*1000 + timenow.tv_nsec/1000000);

		return MT_SUCCESS;

}

/**
\brief get stride from input width. CNcomment:根据图片的宽度来获取stride大小 CNend
\attention \n

\param[out] *pu32TimeMs  CNcomment:获取到的时间ms CNend\n
\param[out] *pu32TimeUs  CNcomment:获取到的时间us CNend\n
\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GFX_GetStride
*/
static inline mt_void MT_GFX_GetStride(mt_u32 u32SrcW, mt_u32 *pu32Stride,mt_u32 u32Align)
{

	#ifdef CONFIG_GFX_256BYTE_ALIGN
		*pu32Stride = MT_SYS_GET_STRIDE(u32SrcW);
	#else
		*pu32Stride = (u32SrcW + u32Align - 1) & (~(u32Align - 1));
	#endif
}


#if defined(CONFIG_GFX_STB_SDK) || defined(CONFIG_GFX_TV_SDK)


/**
\brief free the mem that has alloced. CNcomment:释放分配过的内存 CNend
\attention \n

\param[in]	u32Phyaddr. CNcomment:物理地址 CNend\n

\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GFX_FreeMem
*/

static inline mt_s32 MT_GFX_FreeMem(phys_addr_t  u32Phyaddr)
{
	return mt_mmz_delete(u32Phyaddr);
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

static inline phys_addr_t MT_GFX_AllocMem(mt_u32 u32Size , mt_u32 u32Align, mt_char* pZoneName, mt_char *pName)
{


		phys_addr_t pAddr = 0;
#ifdef CONFIG_GFX_256BYTE_ALIGN
		mt_u32 u32MMZAlign = 256;
#else
		mt_u32 u32MMZAlign  = u32Align;
#endif

/* FIX: hack 'jpeg', 'graphics' mmz zone allocate fail, effect performance! */
#if 0
		pAddr = mt_mmz_new(u32Size, u32MMZAlign, pZoneName, pName);
		if(NULL != pAddr)
		{
		    return pAddr;
		}

		pAddr = mt_mmz_new(u32Size, u32MMZAlign, "graphics", pName);
		if(NULL != pAddr)
		{
		    return pAddr;
		}
#endif
		pAddr = mt_mmz_new(u32Size, u32MMZAlign, NULL, pName);

		return pAddr;

}


static inline mt_void *MT_GFX_Map(phys_addr_t  u32PhyAddr)
{
	return mt_mmz_map(u32PhyAddr,MT_FALSE);
}

static inline mt_void *MT_GFX_MapCached(phys_addr_t u32PhyAddr)
{
	return mt_mmz_map(u32PhyAddr,MT_TRUE);
}
static inline mt_s32 MT_GFX_Unmap(void * vAddr)
{
	return mt_mmz_unmap(vAddr);
}

static inline mt_s32 MT_GFX_Flush(void * vAddr, ulong  offset, ulong  size)
{
	/** linux use flush all **/
	return mt_mmz_flush((void *)vAddr, offset, size);
}

static inline mt_s32 MT_GFX_GetPhyaddr(mt_void * pVir, phys_addr_t *pu32Phyaddr, ulong *pu32Size)
{
	return mt_mmz_get_phyaddr(pVir, pu32Phyaddr, pu32Size);
}

#elif defined(CONFIG_GFX_ANDROID_SDK)

static mt_s32 gfx_mem_open()
{
        int fd = open("/dev/ion", O_RDWR);
        if (fd < 0)
        {
             GFX_Printf("open /dev/ion failed!\n");
        }
        return fd;
}

static mt_s32 gfx_mem_close(int fd)
{
        return close(fd);
}

static mt_s32 gfx_ion_ioctl(int fd, int req, void *arg)
{
        int ret = ioctl(fd, req, arg);
        if (ret < 0)
		{
           GFX_Printf("ioctl %x failed with code %d: %s\n", req,ret, strerror(errno));
           return -errno;
        }
        return ret;
}


static mt_s32 gfx_ion_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
	      unsigned int flags, struct ion_handle **handle)
{

        int ret;
        struct ion_allocation_data data =
			       {
                        .len   = len,
                        .align = align,
		                .heap_id_mask = heap_mask,
                        .flags = flags,
                    };

        ret = gfx_ion_ioctl(fd, ION_IOC_ALLOC, &data);
        if (ret < 0)
        {
             return ret;
		}
        *handle = data.handle;

        return ret;

}


static mt_s32 gfx_ion_free(int fd, struct ion_handle *handle)
{
    struct ion_handle_data data =
		    {
                 .handle = handle,
            };
    return gfx_ion_ioctl(fd, ION_IOC_FREE, &data);
}

static mt_s32 gfx_ion_phys(int fd, struct ion_handle *handle, unsigned long *phys_addr, size_t *len)
{
    struct ion_phys_data data =
		    {
                .handle = handle,
            };

    int ret = gfx_ion_ioctl(fd, ION_IOC_PHYS, &data);

    if (ret < 0)
    {
       return ret;
	}
    *phys_addr = data.phys_addr;
    *len = data.len;
    return ret;
}

static mt_s32 gfx_ion_map(int fd, struct ion_handle *handle, size_t length, int prot,
            int flags, off_t offset, unsigned char **ptr, int *map_fd)
{
        struct ion_fd_data data =
			{
                .handle = handle,
            };

        int ret = gfx_ion_ioctl(fd, ION_IOC_MAP, &data);
        if (ret < 0)
        {
            return ret;
        }

        *map_fd = data.fd;

        if (*map_fd < 0)
		{
            GFX_Printf("map ioctl returned negative fd\n");
            return -EINVAL;
        }
#ifdef ANDROID
        *ptr = mt_mmap_alias(NULL, length, prot, flags, *map_fd, offset);
#else
        *ptr = mmap(NULL, length, prot, flags, *map_fd, offset);
#endif
        if (*ptr == MAP_FAILED)
		{
            GFX_Printf("mmap failed: %s\n", strerror(errno));
            return -errno;
        }
        return ret;

}

static mt_s32 gfx_ion_sync_fd(int fd, int handle_fd)
{
    struct ion_fd_data data =
	{
        .fd = handle_fd,
    };
    return gfx_ion_ioctl(fd, ION_IOC_SYNC, &data);
}


/**
\brief free the mem that has alloced. CNcomment:释放分配过的内存 CNend
\attention \n

\param[in]	u32Phyaddr. CNcomment:物理地址 CNend\n

\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GFX_FreeMem
*/

static inline mt_s32 MT_GFX_FreeMem(mt_s32 s32MMZDev,phys_addr_t u32Phyaddr, mt_void *pMemHandle)
{
	mt_s32 s32Ret;

	s32Ret = gfx_ion_free(s32MMZDev, ((MT_GFX_MEM_HANDLE_S *)pMemHandle)->pIonHandle);
	if (s32Ret)
	{
		return MT_FAILURE;
	}
	free(pMemHandle);
	return MT_SUCCESS;

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

static inline phys_addr_t MT_GFX_AllocMem(mt_s32 s32MMZDev,mt_u32 u32Size , mt_u32 u32Align, mt_char* pZoneName, mt_char *pName, mt_void **ppMemHandle)
{
	MT_GFX_MEM_HANDLE_S *pstMemHandle = NULL;
	phys_addr_t u32Phyaddr;
	mt_s32 s32Ret;

#ifdef CONFIG_GFX_256BYTE_ALIGN
	mt_u32 u32MMZAlign = 256;
#else
	mt_u32 u32MMZAlign	= u32Align;
#endif

	pstMemHandle = (MT_GFX_MEM_HANDLE_S *)malloc(sizeof(MT_GFX_MEM_HANDLE_S));
	if (NULL == pstMemHandle)
	{
		*ppMemHandle = NULL;
		return NULL;
	}

	s32Ret = gfx_ion_alloc(s32MMZDev, u32Size, u32MMZAlign, ION_HEAP(ION_MTS_ID_DDR), ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC, &(pstMemHandle->pIonHandle));
	if (s32Ret)
	{
		GFX_Printf("ion_alloc failed!\n");
		free(pstMemHandle);
		*ppMemHandle = NULL;
		return NULL;
	}

	s32Ret = gfx_ion_phys(s32MMZDev, pstMemHandle->pIonHandle, &u32Phyaddr, &u32Size);
	if (s32Ret)
	{
		GFX_Printf("ion_phys failed!\n");
		gfx_ion_free(s32MMZDev, pstMemHandle->pIonHandle);
		free(pstMemHandle);
		*ppMemHandle = NULL;
		return NULL;
	}

	pstMemHandle->u32Phyaddr = u32Phyaddr;
	pstMemHandle->u32Size = u32Size;
	*ppMemHandle = pstMemHandle;

	return u32Phyaddr;
}


static inline mt_void *MT_GFX_Map(mt_s32 s32MMZDev,phys_addr_t u32PhyAddr, mt_void *pMemHandle)
{
	unsigned char *ptr = MT_NULL;
	mt_s32 s32Ret;

	s32Ret = gfx_ion_map(s32MMZDev, ((MT_GFX_MEM_HANDLE_S *)pMemHandle)->pIonHandle,
		((MT_GFX_MEM_HANDLE_S *)pMemHandle)->u32Size, PROT_READ | PROT_WRITE, MAP_SHARED,
		0, &ptr, &(((MT_GFX_MEM_HANDLE_S *)pMemHandle)->s32MapFd));
	if (s32Ret)
	{
		GFX_Printf("ion_map failed!\n");
		return NULL;
	}

	((MT_GFX_MEM_HANDLE_S *)pMemHandle)->pViraddr = ptr;
	return ptr;
}

static inline mt_void *MT_GFX_MapCached(mt_s32 s32MMZDev,phys_addr_t u32PhyAddr, mt_void *pMemHandle)
{
	return MT_GFX_Map(s32MMZDev,u32PhyAddr,pMemHandle);
}

static inline mt_s32 MT_GFX_Unmap(mt_s32 s32MMZDev,void * vAddr, mt_void *pMemHandle)
{
	munmap(((MT_GFX_MEM_HANDLE_S *)pMemHandle)->pViraddr, ((MT_GFX_MEM_HANDLE_S *)pMemHandle)->u32Size);
	close(((MT_GFX_MEM_HANDLE_S *)pMemHandle)->s32MapFd);
	((MT_GFX_MEM_HANDLE_S *)pMemHandle)->s32MapFd = 0;

	return MT_SUCCESS;

}

static inline mt_s32 MT_GFX_Flush(mt_s32 s32MMZDev,void * u32VirAddr, mt_void *pMemHandle)
{
	return gfx_ion_sync_fd(s32MMZDev, ((MT_GFX_MEM_HANDLE_S *)pMemHandle)->s32MapFd);
}

static inline mt_s32 MT_GFX_GetPhyaddr(mt_s32 s32MMZDev,mt_void * pVir, mt_u32 *pu32Phyaddr, mt_u32 *pu32Size, mt_void *pMemHandle)
{
	return MT_FAILURE;
}


#elif defined(CONFIG_GFX_BVT_SDK)

static inline mt_s32 gfx_mem_open()
{
        int fd = open("/dev/mmz_userdev", O_RDWR);
        if (fd < 0)
        {
             GFX_Printf("open /dev/mmz_userdev failed!\n");
        }
        return fd;
}

static inline mt_s32 gfx_mem_close(int fd)
{
        return close(fd);
}

static inline phys_addr_t GFX_MMZ_New(mt_s32 mmz,mt_u32 size,mt_u32 align,mt_char *mmz_name, mt_char *mmb_name)
{

	      mmb_info   mmi;

	      memset(&mmi,0,sizeof(mmi));

	      mmi.size = size;
	      mmi.align =align;

	      if (mmz_name != NULL)
	      {
	          strncpy(mmi.mmz_name, mmz_name, strlen(mmi.mmz_name));
			  mmi.mmz_name[strlen(mmz_name)]='\0';
	      }

	      if (mmb_name != NULL)
	      {
	          strncpy(mmi.mmb_name, mmb_name, strlen(mmi.mmb_name));
			  mmi.mmz_name[strlen(mmb_name)]='\0';
	      }

	      if (ioctl(mmz, IOC_MMB_ALLOC, &mmi) !=0)
	      {
	    	 return NULL;
	      }

	      return mmi.phys_addr;

}

static inline mt_void *GFX_MMZ_Map(mt_s32 mmz, phys_addr_t phyAddr, mt_s32 cached)
{


		  int s32Ret;

	      mmb_info   mmi;
	      memset(&mmi,0,sizeof(mmi));

		  if(cached != 0 && cached != 1)
		  {
		    return NULL;
		  }
	      mmi.prot = PROT_READ | PROT_WRITE;
	      mmi.flags = MAP_SHARED;
	      mmi.phys_addr = phyAddr;

		  if(cached)
		  {
		      s32Ret = ioctl(mmz,IOC_MMB_USER_REMAP_CACHED, &mmi);
	          if (s32Ret!=0)
	          {
	    		 return NULL;
	          }

		  }
		  else
		  {
		      s32Ret = ioctl(mmz,IOC_MMB_USER_REMAP, &mmi);
	          if (s32Ret!=0)
	          {
	    		 return NULL;
	          }
		  }

	      return (void *)mmi.mapped;


}

static inline mt_s32 GFX_MMZ_UnMap(mt_s32 mmz, mt_void *virAddr)
{

      mmb_info   mmi;
      memset(&mmi,0,sizeof(mmi));
  //    mmi.phys_addr = (unsigned long)phyAddr;
      mmi.mapped = virAddr;
      return ioctl(mmz, IOC_MMB_USER_UNMAP, &mmi);

}
static inline mt_s32 GFX_MMZ_Delete(mt_s32 mmz, phys_addr_t phyAddr)
{

      mmb_info   mmi;
      memset(&mmi,0,sizeof(mmi));
      mmi.phys_addr = phyAddr;
      return ioctl(mmz, IOC_MMB_FREE, &mmi);

}
static inline mt_s32 GFX_MMZ_Flush(mt_s32 mmz,ulong u32VirAddr, mt_u32 offset, mt_u32 size)
{
	struct cache_op_mmz_area coa;

    if (!u32VirAddr)
    {
        return ioctl(mmz, IOC_MMB_FLUSH_DCACHE, NULL);
    }
    else
    {
		coa.pbase = (void *)u32VirAddr;
		coa.offset = offset;
		coa.size = size;
        return ioctl(mmz, IOC_MMB_FLUSH_DCACHE, &coa);
    }

}
static inline mt_s32 GFX_MMZ_GetPhyaddr(mt_s32 mmz,mt_void * pVir, phys_addr_t *pu32Phyaddr, mt_u32 *pu32Size)
{

	int ret;
	mmb_info   mmi;
    memset(&mmi,0,sizeof(mmi));

	mmi.mapped = pVir;

	ret = ioctl(mmz, IOC_MMB_USER_GETPHYADDR, &mmi);
	if (ret)
	{
		return -1;
	}
	if (pu32Phyaddr)
	{
		*pu32Phyaddr = mmi.phys_addr;
	}
	if (pu32Size)
	{
		*pu32Size = mmi.size;
	}
	return 0;


}


/**
\brief free the mem that has alloced. CNcomment:释放分配过的内存 CNend
\attention \n

\param[in]	u32Phyaddr. CNcomment:物理地址 CNend\n

\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GFX_FreeMem
*/

static inline mt_s32 MT_GFX_FreeMem(mt_s32 s32MMZDev,phys_addr_t u32Phyaddr, mt_void *pMemHandle)
{
    return GFX_MMZ_Delete(s32MMZDev,u32Phyaddr);
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

static inline phys_addr_t MT_GFX_AllocMem(mt_s32 s32MMZDev,mt_u32 u32Size , mt_u32 u32Align, mt_char* pZoneName, mt_char *pName, mt_void **ppMemHandle)
{

	phys_addr_t pAddr = 0;

#ifdef CONFIG_GFX_256BYTE_ALIGN
	mt_u32 u32MMZAlign = 256;
#else
	mt_u32 u32MMZAlign	= u32Align;
#endif

	pAddr = GFX_MMZ_New(s32MMZDev,u32Size, u32MMZAlign, pZoneName, pName);
	if(NULL != pAddr)
	{
		return pAddr;
	}

	pAddr = GFX_MMZ_New(s32MMZDev,u32Size, u32MMZAlign, "graphics", pName);
	if(NULL != pAddr)
	{
		return pAddr;
	}

	pAddr = GFX_MMZ_New(s32MMZDev,u32Size, u32MMZAlign, NULL, pName);

	return pAddr;

}


static inline mt_void *MT_GFX_Map(ulong s32MMZDev,phys_addr_t u32PhyAddr, mt_void *pMemHandle)
{
	return GFX_MMZ_Map(s32MMZDev,u32PhyAddr,MT_FALSE);
}

static inline mt_void *MT_GFX_MapCached(ulong s32MMZDev,phys_addr_t u32PhyAddr, mt_void *pMemHandle)
{
	return GFX_MMZ_Map(s32MMZDev,u32PhyAddr,MT_TRUE);
}

static inline mt_s32 MT_GFX_Unmap(ulong s32MMZDev,mt_void * vAddr, mt_void *pMemHandle)
{	
    return GFX_MMZ_UnMap(s32MMZDev,vAddr);
}
static inline mt_s32 MT_GFX_Flush(ulong s32MMZDev,ulong u32VirAddr, mt_void *pMemHandle, mt_u32 offset, mt_u32 size)
{	
    return GFX_MMZ_Flush(s32MMZDev,u32VirAddr, offset, size);
}
static inline mt_s32 MT_GFX_GetPhyaddr(ulong s32MMZDev,mt_void * pVir, phys_addr_t *pu32Phyaddr, mt_u32 *pu32Size, mt_void *pMemHandle)
{	
    return GFX_MMZ_GetPhyaddr(s32MMZDev,pVir, pu32Phyaddr, pu32Size);
}
#else


#endif

/** @} */  /*! <!-- API declaration end */


#endif /*_MT_GFX_COMM_H_ */
