/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#include "mt_type.h"
#include "exports.h"
#include "hifb_debug.h"
#include "jpg_common.h"

#ifndef WIN32

//static mt_s32 g_s32fd = 0;

#define CHECK_INIT() 	do{ if( g_s32fd <=0) g_s32fd =open("/dev/mtmedia/mmz_userdev", O_RDWR); \
												if( g_s32fd <=0) return MT_FAILURE; \
										  }while(0); 

#define CHECK_INIT2() do{ if( g_s32fd <=0) g_s32fd =open("/dev/mtmedia/mmz_userdev", O_RDWR); \
												if( g_s32fd <=0) return NULL; \
										  }while(0); 

#define JPG_MEM_BASE_ADDR   0xc3000005
#define JPG_MEM_SIZE        (8*1024*1024)

mt_u32  g_JpgMemOffset = 0;

//extern mt_u32 g_JpgDecAddr;
static phys_addr_t g_JpgDecPhyAddr = 0;
static mt_void *g_JpgDecAddr = NULL;
static mt_u32 g_JpgDecLen = 0;

mt_s32 MT_MMB_Init(phys_addr_t uStartAddr , mt_u32 uLen)
{
    g_JpgDecPhyAddr = uStartAddr;
    g_JpgDecLen = uLen;
	//g_JpgDecAddr = mmap(g_JpgDecPhyAddr);
    return MT_SUCCESS;
}

mt_s32 MT_MMB_DeInit(mt_void)
{
#ifdef MT_MINIBOOT_SUPPORT
    mmu_cache_enable();
#else
    dcache_enable(0);
#endif

    VCOS_memset(g_JpgDecAddr, 0, g_JpgDecLen);

#ifdef MT_MINIBOOT_SUPPORT
    mmu_cache_disable();
#else
    dcache_disable();
#endif

    g_JpgMemOffset = 0;
    return MT_SUCCESS;
}

phys_addr_t MT_MMB_New(mt_u32 size , mt_u32 align, mt_u8 *mmz_name, mt_u8 *mmb_name )
{
    phys_addr_t  StartPhyAddr;
	StartPhyAddr = g_JpgDecPhyAddr + g_JpgMemOffset;
    if( align > 0 )
    {
	    StartPhyAddr = (StartPhyAddr + align - 1);
		StartPhyAddr = (StartPhyAddr / align) * align;
	}
    if(StartPhyAddr + size  >= g_JpgDecPhyAddr + g_JpgDecLen)
    {
        Debug("MT_MMB_New failed. TotalLen 0x%x offset 0x%x acquiresize 0x%x", g_JpgDecLen,g_JpgMemOffset,size);
        return MT_NULL;
    }
    
	 g_JpgMemOffset = StartPhyAddr + size - g_JpgDecPhyAddr;
    return StartPhyAddr;
}

mt_void *MT_MMB_Map(phys_addr_t phys_addr, mt_u32 cached)
{
	return g_JpgDecAddr + (ulong)(phys_addr - g_JpgDecPhyAddr);

}

mt_s32 MT_MMB_Unmap(mt_void *vaddr)
{
	return MT_SUCCESS;
}
 
mt_s32 MT_MMB_Delete(phys_addr_t phys_addr)
{
	return MT_SUCCESS;
}

mt_s32 MT_MMB_Flush(mt_void)
{
	return MT_SUCCESS;
}

#endif

