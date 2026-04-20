#ifndef TDE_BOOT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h> /* mmap */
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/types.h>
#include "mt_drv_tde.h"
#else
#include "tde_osictl.h"
#endif
#include "mt_tde_api.h"
#include "mt_debug.h"

#include "gpe.h"
#include <pthread.h>    
#include <semaphore.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef TDE_BOOT
#define TDE_CHECK_FD()                      \
    do {                                    \
	if (-1 == g_s32TdeFd) {             \
	    return MT_ERR_TDE_DEV_NOT_OPEN; \
	}                                   \
    } while (0)

static const char *g_pszTdeDevName = "/dev/mt_tde";

mt_s32 g_s32TdeFd = -1; /* tde device handle */

static mt_s32 g_s32TDeRef = 0;

static const mt_u8 s_szTDEVersion[] __attribute__((used)) = "SDK_VERSION:[" MKMARCOTOSTR(SDK_VERSION) "] Build Time:[" __DATE__ ", " __TIME__ "]";

static pthread_mutex_t tde_mutex = PTHREAD_MUTEX_INITIALIZER;

 /*static pthread_mutex_t gpe_mutex;*/
static char SEM_NAME[]= "sem_gpe";
static sem_t *gpe_mutex = NULL;  //Bug 125972
#define SUPPORT_CMDFIFO  

#ifdef SUPPORT_CMDFIFO
static pthread_mutex_t gpe_cf_mutex;
#endif

#ifdef USE_USER_SPACE_GPE
ulong gpe_reg_virt_addr;
ulong sys_reg_virt_addr;
#ifdef CONFIG_MT_CHIP_ARIA
  MT_CHIP_VERSION_E   ChipVersion = MT_CHIP_VERSION_V200;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
  MT_CHIP_VERSION_E   ChipVersion = MT_CHIP_SYMPHONY_A1;
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
  MT_CHIP_VERSION_E   ChipVersion = MT_CHIP_SYMPHONY4_A0;
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
  MT_CHIP_VERSION_E   ChipVersion = MT_CHIP_SYMPHONY6_A0;
#endif
#endif

#else
#define TDE_CHECK_FD()                      \
    do {                                    \
	if (0 == g_s32TDeRef) {             \
	    return MT_ERR_TDE_DEV_NOT_OPEN; \
	}                                   \
    } while (0)

static mt_s32 g_s32TDeRef = 0;
extern int tde_osr_init(void);
extern void tde_osr_deinit(void);
#endif

#if 0
#define MT_TDE2_FUN_IN printf("%s, LINE IN: %d\n", __FUNCTION__, __LINE__)
#define MT_TDE2_FUN_OUT printf("%s, LINE OUT: %d\n", __FUNCTION__, __LINE__)
#define MT_TDE2_LOG printf
#define MT_TDE2_LINE printf("%s, LINE: %d\n", __FUNCTION__, __LINE__)
#else
#define DUMP_LOG \
    do {         \
    } while (0)
#define MT_TDE2_FUN_IN DUMP_LOG
#define MT_TDE2_FUN_OUT DUMP_LOG
#define MT_TDE2_LOG(...) DUMP_LOG
#define MT_TDE2_LINE DUMP_LOG
#endif

#ifdef USE_USER_SPACE_GPE
#define MT_TDE2_RETURN()   \
    \
{                   \
	return MT_SUCCESS; \
    \
}
#else
#define MT_TDE2_RETURN() \
    do {                 \
    } while (0)
#endif
mt_s32 get_tde_handle(mt_void);

mt_s32 MT_TDE2_Open(mt_void)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_sys_version_s    SysVersion;

    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();
    pthread_mutex_lock(&tde_mutex);
#ifndef TDE_BOOT
    if (-1 != g_s32TdeFd) {
        g_s32TDeRef++;
        pthread_mutex_unlock(&tde_mutex);
        return MT_SUCCESS;
    }

    g_s32TdeFd = open(g_pszTdeDevName, O_RDWR, 0);
    if (g_s32TdeFd < 0) {
        pthread_mutex_unlock(&tde_mutex);
        return MT_ERR_TDE_DEV_OPEN_FAILED;
    }

#ifdef USE_USER_SPACE_GPE
    /*pthread_mutex_init(&gpe_mutex, NULL); */
    //create & initialize semaphore
    gpe_mutex = sem_open(SEM_NAME,O_CREAT,0644,1);
    if(gpe_mutex == SEM_FAILED)
    {
        MT_TDE2_LOG("unable to create semaphore");
        sem_unlink(SEM_NAME);
        pthread_mutex_unlock(&tde_mutex);
        return MT_ERR_TDE_DEV_OPEN_FAILED;
    }
#ifdef SUPPORT_CMDFIFO
    pthread_mutex_init(&gpe_cf_mutex, NULL);
#endif
    gpe_reg_virt_addr = (ulong)mmap(NULL, 0x400, PROT_READ | PROT_WRITE, MAP_SHARED, g_s32TdeFd, 0);
    if ((void *)gpe_reg_virt_addr == MAP_FAILED) {
        MT_TDE2_LOG("[VDI] fail to map vpu registers \n");
        pthread_mutex_unlock(&tde_mutex);
        return MT_ERR_TDE_DEV_OPEN_FAILED;
    }    

    GpeOsiScaleCoeff_New();
#if 0    
    mpi_memdev_init();    
    MT_TDE_Create_Mid_Mem(1920*1080*4);
#endif    
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    s32Ret = mt_sys_map_register(SYMPHONY_IO_PA(0xbf50a10c), 4, (mt_void*)&sys_reg_virt_addr);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    s32Ret = mt_sys_map_register(SYMPHONY_IO_PA(0xbf510014), 4, (mt_void*)&sys_reg_virt_addr);
#endif   
    MT_TDE2_LOG("MT_TDE2_Open: gpe_base = 0x%lx sys_base = 0x%lx \n", gpe_reg_virt_addr, sys_reg_virt_addr);

    if (MT_SUCCESS != s32Ret)
    {
        pthread_mutex_unlock(&tde_mutex);
        return MT_ERR_TDE_DEV_OPEN_FAILED;
    }
    memset(&SysVersion,0,sizeof(mt_sys_version_s));
    s32Ret = mt_sys_get_version(&SysVersion);
    if (MT_SUCCESS == s32Ret)
    {
        ChipVersion = SysVersion.enChipVersion;
    }
#ifdef SUPPORT_CMDFIFO
    s32Ret = gpe_create_cmdfifo_memory();    
    if (MT_SUCCESS != s32Ret)
    {
        pthread_mutex_unlock(&tde_mutex);
        return MT_ERR_TDE_DEV_OPEN_FAILED;
    }   
#endif
#endif
#else
    if (g_s32TDeRef) {
        g_s32TDeRef++;
        pthread_mutex_unlock(&tde_mutex);
        return MT_SUCCESS;
    }

    /* 3?¨º??¡¥?¨²¡ä??¡é??¡ä??¡Â?¡é¡Á¡ä¨¬??¨² */
    if (tde_osr_init() < 0) {
        pthread_mutex_unlock(&tde_mutex);
        return MT_ERR_TDE_DEV_OPEN_FAILED;
    }
#endif
    g_s32TDeRef++;
    pthread_mutex_unlock(&tde_mutex);
    return MT_SUCCESS;
}


mt_void MT_TDE2_Close(mt_void)
{
    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();
    pthread_mutex_lock(&tde_mutex);
#ifndef TDE_BOOT
    if (-1 == g_s32TdeFd) {
        pthread_mutex_unlock(&tde_mutex);
        return;
    }
    g_s32TDeRef--;

    if (g_s32TDeRef > 0) {
        pthread_mutex_unlock(&tde_mutex);
        return;
    } else {
        g_s32TDeRef = 0;
    }

    close(g_s32TdeFd);

    g_s32TdeFd = -1;

#ifdef USE_USER_SPACE_GPE
    if (gpe_reg_virt_addr != 0) {
        munmap((void *)gpe_reg_virt_addr, 0x400);
        gpe_reg_virt_addr = 0;
        MT_TDE2_LOG("MT_TDE2_Close: munmap ok\n");
    }

    GpeOsiScaleCoeff_Del();
#if defined(CONFIG_MT_CHIP_SYMPHONY6) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    mt_sys_unmap_register((mt_void*)sys_reg_virt_addr);
#endif
    /*pthread_mutex_destroy(&gpe_mutex); */
    sem_close(gpe_mutex);
    sem_unlink(SEM_NAME);
#ifdef SUPPORT_CMDFIFO
    pthread_mutex_destroy(&gpe_cf_mutex);
    gpe_destroy_cmdfifo_memory();   
#endif

#endif

#else
    if (g_s32TDeRef == 0) {
        pthread_mutex_unlock(&tde_mutex);
        return;
    }

    g_s32TDeRef--;

    if (g_s32TDeRef > 0) {
        pthread_mutex_unlock(&tde_mutex);
        return;
    }

    /* ¨¨£¤3?¨º??¡¥?¨²¡ä??¡é??¡ä??¡Â?¡é¡Á¡ä¨¬??¨² */
    tde_osr_deinit();
#endif
    pthread_mutex_unlock(&tde_mutex);
    return;
}

mt_s32 get_tde_handle(mt_void)
{
  return g_s32TdeFd;
}

TDE_HANDLE MT_TDE2_BeginJob(mt_void)
{
    TDE_HANDLE s32Handle;

    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();
#ifndef TDE_BOOT

#ifndef USE_USER_SPACE_GPE
    if (ioctl(g_s32TdeFd, TDE_BEGIN_JOB, &s32Handle) < 0) {
	return MT_ERR_TDE_INVALID_HANDLE;
    }
#else
	/*pthread_mutex_lock(&gpe_mutex); */
    sem_wait(gpe_mutex);

    if (GpeOsiBeginJob(&s32Handle) < 0) {
		//for bug 104373
		/*pthread_mutex_destroy(&gpe_mutex); */
        sem_post(gpe_mutex);
		return MT_ERR_TDE_INVALID_HANDLE;
    }
#endif

#else
    if (TdeOsiBeginJob(&s32Handle) < 0) {
	return MT_ERR_TDE_INVALID_HANDLE;
    }
#endif
    return s32Handle;
}

mt_s32 MT_TDE_Create_Mid_Mem(mt_u32 mem_size)
{
	return gpe_create_mid_memory(mem_size);
}

mt_s32 MT_TDE_Destroy_Mid_Mem(mt_void)
{
	return gpe_destroy_mid_memory();
}

mt_s32 MT_TDE2_Bitblit(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
        TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect, TDE2_SURFACE_S *pstDst,
        TDE2_RECT_S *pstDstRect,
        TDE2_OPT_S *pstOpt)
{
#ifndef TDE_BOOT
#ifndef USE_USER_SPACE_GPE

    TDE_BITBLIT_CMD_S stBlitCmd = { 0 };

    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    stBlitCmd.s32Handle = s32Handle;

    if ((NULL == pstBackGround)) {
        stBlitCmd.u32NullIndicator = (1 << 1);
    } else {
        memcpy(&stBlitCmd.stBackGround, pstBackGround, sizeof(TDE2_SURFACE_S));
    }

    if (NULL == pstBackGroundRect) {
        stBlitCmd.u32NullIndicator |= (1 << 2);
    } else {
        memcpy(&stBlitCmd.stBackGroundRect, pstBackGroundRect, sizeof(TDE2_RECT_S));
    }

    if ((NULL == pstForeGround)) {
        stBlitCmd.u32NullIndicator |= (1 << 3);
    } else {
        memcpy(&stBlitCmd.stForeGround, pstForeGround, sizeof(TDE2_SURFACE_S));
    }

    if (NULL == pstForeGroundRect) {
        stBlitCmd.u32NullIndicator |= (1 << 4);
    } else {
        memcpy(&stBlitCmd.stForeGroundRect, pstForeGroundRect, sizeof(TDE2_RECT_S));
    }

    if ((NULL == pstDst)) {
        return MT_ERR_TDE_NULL_PTR;
    } else {
        memcpy(&stBlitCmd.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    }

    if (NULL == pstDstRect) {
        return MT_ERR_TDE_NULL_PTR;
    } else {
        memcpy(&stBlitCmd.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));
    }

    if (NULL == pstOpt) {
        stBlitCmd.u32NullIndicator |= (1 << 7);
    } else {
        memcpy(&stBlitCmd.stOpt, pstOpt, sizeof(TDE2_OPT_S));
    }


    return ioctl(g_s32TdeFd, TDE_BIT_BLIT, &stBlitCmd);
#else
    mt_u32 clip_en = 0;

    if(pstOpt->enClipMode == TDE2_CLIPMODE_INSIDE)
    {
        if(pstOpt->stClipRect.s32Xpos <= pstDstRect->s32Xpos &&
                pstOpt->stClipRect.s32Ypos <= pstDstRect->s32Ypos &&
                (mt_s32)pstOpt->stClipRect.u32Width + pstOpt->stClipRect.s32Xpos >= (mt_s32)pstDstRect->u32Width + pstDstRect->s32Xpos &&
                (mt_s32)pstOpt->stClipRect.u32Height + pstOpt->stClipRect.s32Ypos >= (mt_s32)pstDstRect->u32Height + pstDstRect->s32Ypos)
            clip_en = 0;	
        else
        {
            clip_en = 1;	  
        }
    }
    else if(pstOpt->enClipMode == TDE2_CLIPMODE_OUTSIDE)
    {
        if(pstOpt->stClipRect.s32Xpos > (mt_s32)pstDstRect->u32Width + pstDstRect->s32Xpos ||
                pstOpt->stClipRect.s32Ypos > (mt_s32)pstDstRect->u32Height + pstDstRect->s32Ypos ||
                pstOpt->stClipRect.s32Xpos + (mt_s32)pstOpt->stClipRect.u32Width < pstDstRect->s32Xpos ||
                pstOpt->stClipRect.s32Ypos + (mt_s32)pstOpt->stClipRect.u32Height < pstDstRect->s32Ypos)
            clip_en = 0;
        else
            clip_en = 1;
    }
    else if(pstOpt->enClipMode == TDE2_CLIPMODE_NONE)
        clip_en = 0;

    if(clip_en == 0)
        return GpeOsiBlit(s32Handle, pstBackGround, pstBackGroundRect, pstForeGround, pstForeGroundRect, pstDst, pstDstRect, pstOpt);
    else
    {
        TDE2_SURFACE_S maskSurface = {0};
        mt_mmz_buf_s  psMBuf;
        mt_s32 ret;
        TDE2_RECT_S maskRect = {0};
        TDE2_RECT_S overlapRect = {0};
        TDE2_RECT_S src_rect = {0};
        TDE2_RECT_S dst_rect = {0};
        TDE2_RECT_S bg_rect = {0};
        MT_BOOL scale_flag = MT_FALSE;

        overlapRect.s32Xpos = pstOpt->stClipRect.s32Xpos > pstDstRect->s32Xpos ? pstOpt->stClipRect.s32Xpos : pstDstRect->s32Xpos;
        overlapRect.s32Ypos = pstOpt->stClipRect.s32Ypos > pstDstRect->s32Ypos ? pstOpt->stClipRect.s32Ypos : pstDstRect->s32Ypos;
        if(pstOpt->stClipRect.s32Xpos + (mt_s32)pstOpt->stClipRect.u32Width < pstDstRect->s32Xpos + (mt_s32)pstDstRect->u32Width)
            overlapRect.u32Width = (mt_u32)(pstOpt->stClipRect.s32Xpos + (mt_s32)pstOpt->stClipRect.u32Width - overlapRect.s32Xpos);
        else
            overlapRect.u32Width = (mt_u32)(pstDstRect->s32Xpos + (mt_s32)pstDstRect->u32Width - overlapRect.s32Xpos);
        if(pstOpt->stClipRect.s32Ypos + (mt_s32)pstOpt->stClipRect.u32Height < pstDstRect->s32Ypos + (mt_s32)pstDstRect->u32Height)
            overlapRect.u32Height= (mt_u32)(pstOpt->stClipRect.s32Ypos + (mt_s32)pstOpt->stClipRect.u32Height - overlapRect.s32Ypos);
        else
            overlapRect.u32Height= (mt_u32)(pstDstRect->s32Ypos + (mt_s32)pstDstRect->u32Height - overlapRect.s32Ypos);	

        overlapRect.s32Xpos -= pstDstRect->s32Xpos;
        overlapRect.s32Ypos -= pstDstRect->s32Ypos;

        if(pstOpt->enClipMode == TDE2_CLIPMODE_INSIDE)
        {
            if(pstForeGroundRect != NULL && pstForeGround != NULL)
            {
                if((pstForeGroundRect->u32Width == 0) && (pstForeGroundRect->u32Height == 0) && (pstForeGround != NULL))
                {
                    src_rect.s32Xpos = 0;
                    src_rect.s32Ypos = 0;
                    src_rect.u32Width = pstForeGround->u32Width;
                    src_rect.u32Height = pstForeGround->u32Height;
                }
                else
                {
                    src_rect.s32Xpos = (mt_u32)(pstForeGroundRect->s32Xpos);
                    src_rect.s32Ypos = (mt_u32)(pstForeGroundRect->s32Ypos);
                    src_rect.u32Width = pstForeGroundRect->u32Width;
                    src_rect.u32Height = pstForeGroundRect->u32Height;
                }      
            }
            if((pstDstRect->u32Width == 0) && (pstDstRect->u32Height == 0))
            {
                dst_rect.s32Xpos = 0;
                dst_rect.s32Ypos = 0;
                dst_rect.u32Width = pstDst->u32Width;
                dst_rect.u32Height = pstDst->u32Height;
            }
            else
            {
                dst_rect.s32Xpos = (mt_u32)(pstDstRect->s32Xpos);
                dst_rect.s32Ypos = (mt_u32)(pstDstRect->s32Ypos);
                dst_rect.u32Width = pstDstRect->u32Width;
                dst_rect.u32Height = pstDstRect->u32Height;
            }

            if(pstBackGroundRect != NULL && pstBackGround != NULL)
            {
                if((pstBackGroundRect->u32Width == 0) && (pstBackGroundRect->u32Height == 0))
                {
                    bg_rect.s32Xpos = 0;
                    bg_rect.s32Ypos = 0;
                    bg_rect.u32Width = pstBackGround->u32Width;
                    bg_rect.u32Height = pstBackGround->u32Height;
                }
                else
                {
                    bg_rect.s32Xpos = (mt_u32)(pstBackGroundRect->s32Xpos);
                    bg_rect.s32Ypos = (mt_u32)(pstBackGroundRect->s32Ypos);
                    bg_rect.u32Width = pstBackGroundRect->u32Width;
                    bg_rect.u32Height = pstBackGroundRect->u32Height;
                }
            }   
            if(((src_rect.u32Width != dst_rect.u32Width) ||(src_rect.u32Height != dst_rect.u32Height)) 
                    && (pstOpt->enRotator == TDE2_ROTATE_NONE)
                    && (!(pstOpt->enPaint == MT_TRUE && pstOpt->stPaintOpt.paint_type == TDE2_PAINT_TYPE_PATTERN)))
                scale_flag = MT_TRUE;
            if(pstForeGround == NULL)
                scale_flag = MT_FALSE;    

            if(scale_flag == MT_FALSE)
            {
                src_rect.s32Xpos += overlapRect.s32Xpos;
                src_rect.s32Ypos += overlapRect.s32Ypos;
                src_rect.u32Width = overlapRect.u32Width;
                src_rect.u32Height = overlapRect.u32Height;

                bg_rect.s32Xpos += overlapRect.s32Xpos;
                bg_rect.s32Ypos += overlapRect.s32Ypos;
                bg_rect.u32Width = overlapRect.u32Width;
                bg_rect.u32Height = overlapRect.u32Height;     

                dst_rect.s32Xpos += overlapRect.s32Xpos;
                dst_rect.s32Ypos += overlapRect.s32Ypos;
                dst_rect.u32Width = overlapRect.u32Width;
                dst_rect.u32Height = overlapRect.u32Height;     
                return GpeOsiBlit(s32Handle, pstBackGround, &bg_rect, pstForeGround, &src_rect, pstDst, &dst_rect, pstOpt);
            }
        }

        maskSurface.enColorFmt = TDE2_COLOR_FMT_A8;
        maskSurface.u32Width = pstDstRect->u32Width;
        maskSurface.u32Height = pstDstRect->u32Height;
        maskSurface.u32Stride = pstDstRect->u32Width;

        memset(&psMBuf, 0x00, sizeof(mt_mmz_buf_s));
        psMBuf.bufsize = maskSurface.u32Stride * maskSurface.u32Height;
        ret = mt_mmz_malloc(&psMBuf);
        if(ret != MT_SUCCESS)
        {
            return MT_ERR_TDE_NO_MEM;
        }  
        maskSurface.u32PhyAddr = psMBuf.phyaddr;
        maskSurface.u32VirAddr = (ulong)(psMBuf.user_viraddr);

        maskRect.u32Width = maskSurface.u32Width;
        maskRect.u32Height = maskSurface.u32Height;

        if(pstOpt->enClipMode == TDE2_CLIPMODE_INSIDE)
        {
            GpeClipMask(s32Handle, NULL, &maskSurface, NULL, &maskRect, TDE2_CLEAR_MASK);
            GpeClipMask(s32Handle, NULL, &maskSurface, NULL, &overlapRect, TDE2_FILL_MASK);
        }
        else
        {
            GpeClipMask(s32Handle, NULL, &maskSurface, NULL, &maskRect, TDE2_FILL_MASK);
            GpeClipMask(s32Handle, NULL, &maskSurface, NULL, &overlapRect, TDE2_CLEAR_MASK);
        }	
        pstOpt->enAMapLogical = MT_TRUE;
        ret = GpeOsiBlit_3src(s32Handle, pstBackGround, pstBackGroundRect, 
                pstForeGround, pstForeGroundRect, &maskSurface, &maskRect, pstDst, pstDstRect, pstOpt);
        mt_mmz_free(&psMBuf);
        return ret;
    }
#endif

#else
    return TdeOsiBlit(s32Handle, pstBackGround, pstBackGroundRect, pstForeGround, pstForeGroundRect, pstDst, pstDstRect, pstOpt);
#endif
}

mt_s32 MT_TDE2_Bitblit_3src(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                       TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect, 
                       TDE2_SURFACE_S *pstExGround, TDE2_RECT_S *pstExGroundRect, 
                       TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                       TDE2_OPT_S *pstOpt)
{
#ifndef TDE_BOOT
#ifndef USE_USER_SPACE_GPE

    TDE_BITBLIT_CMD_S stBlitCmd = { 0 };

    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    stBlitCmd.s32Handle = s32Handle;

    if ((NULL == pstBackGround)) {
	stBlitCmd.u32NullIndicator = (1 << 1);
    } else {
	memcpy(&stBlitCmd.stBackGround, pstBackGround, sizeof(TDE2_SURFACE_S));
    }

    if (NULL == pstBackGroundRect) {
	stBlitCmd.u32NullIndicator |= (1 << 2);
    } else {
	memcpy(&stBlitCmd.stBackGroundRect, pstBackGroundRect, sizeof(TDE2_RECT_S));
    }

    if ((NULL == pstForeGround)) {
	stBlitCmd.u32NullIndicator |= (1 << 3);
    } else {
	memcpy(&stBlitCmd.stForeGround, pstForeGround, sizeof(TDE2_SURFACE_S));
    }

    if (NULL == pstForeGroundRect) {
	stBlitCmd.u32NullIndicator |= (1 << 4);
    } else {
	memcpy(&stBlitCmd.stForeGroundRect, pstForeGroundRect, sizeof(TDE2_RECT_S));
    }

    if ((NULL == pstDst)) {
	return MT_ERR_TDE_NULL_PTR;
    } else {
	memcpy(&stBlitCmd.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    }

    if (NULL == pstDstRect) {
	return MT_ERR_TDE_NULL_PTR;
    } else {
	memcpy(&stBlitCmd.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));
    }

    if (NULL == pstOpt) {
	stBlitCmd.u32NullIndicator |= (1 << 7);
    } else {
	memcpy(&stBlitCmd.stOpt, pstOpt, sizeof(TDE2_OPT_S));
    }


    return 0;
#else
    return GpeOsiBlit_3src(s32Handle, pstBackGround, pstBackGroundRect, 
                              pstForeGround, pstForeGroundRect, pstExGround, pstExGroundRect, pstDst, pstDstRect, pstOpt);
#endif

#else
    return 0;
#endif
}

#ifndef TDE_BOOT
mt_s32 MT_TDE2_QuickCopy(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
                         TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect)
{
    TDE_QUICKCOPY_CMD_S stQuickCopy = { 0 };

    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    if ((NULL == pstSrc) || (NULL == pstSrcRect) || (NULL == pstDst) || (NULL == pstDstRect)) {
	return MT_ERR_TDE_NULL_PTR;
    }

    stQuickCopy.s32Handle = s32Handle;

    memcpy(&stQuickCopy.stSrc, pstSrc, sizeof(TDE2_SURFACE_S));
    memcpy(&stQuickCopy.stSrcRect, pstSrcRect, sizeof(TDE2_RECT_S));
    memcpy(&stQuickCopy.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    memcpy(&stQuickCopy.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));

#ifndef USE_USER_SPACE_GPE
    return ioctl(g_s32TdeFd, TDE_QUICK_COPY, &stQuickCopy);
#else
    return GpeOsiQuickCopy(s32Handle, pstSrc, pstSrcRect, pstDst, pstDstRect);
#endif

}
#endif

/*****************************************************************************
* Function:      MT_TDE2_QuickFill
* Description:   Fill quickly fixed value to target bitmap
* Input:         s32Handle: Task handle
*                pDst: Target bitmap info struct
*                u32FillData: Fill information, pixel format accord with target bitmap
* Output:        none
* Return:        >0: current task id; <0: Failure
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_QuickFill(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                         mt_u32 u32FillData)
{
    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    if ((NULL == pstDst) || (NULL == pstDstRect)) {
	return MT_ERR_TDE_NULL_PTR;
    }

#ifndef TDE_BOOT
    TDE_QUICKFILL_CMD_S stQuickFill = { 0 };
    memcpy(&stQuickFill.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    memcpy(&stQuickFill.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));
    stQuickFill.s32Handle = s32Handle;
    stQuickFill.u32FillData = u32FillData;

#ifndef USE_USER_SPACE_GPE
    return ioctl(g_s32TdeFd, TDE_QUICK_FILL, &stQuickFill);
#else
    return GpeOsiQuickFill(s32Handle, pstDst, pstDstRect, u32FillData);
#endif

#else
    return TdeOsiQuickFill(s32Handle, pstDst, pstDstRect, u32FillData);
#endif
}

/*****************************************************************************
* Function:      MT_TDE2_QuickResize
* Description:   Zoom source bitmap to the size fixed by target bitmap,source and target bitmap can be the same
* Input:         s32Handle: Task handle
*                pSrc: Source bitmap info struct
*                pDst: Target bitmap info struct
* Output:        None
* Return:        >0: current task id; <0: Faiure
* Others:        None
*****************************************************************************/
#ifndef TDE_BOOT
mt_s32 MT_TDE2_QuickResize(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
                           TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect)
{
    TDE_QUICKRESIZE_CMD_S stQuickResize = { 0 };

    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    if ((NULL == pstSrc) || (NULL == pstSrcRect) || (NULL == pstDst) || (NULL == pstDstRect)) {
	return MT_ERR_TDE_NULL_PTR;
    }

    stQuickResize.s32Handle = s32Handle;

    memcpy(&stQuickResize.stSrc, pstSrc, sizeof(TDE2_SURFACE_S));
    memcpy(&stQuickResize.stSrcRect, pstSrcRect, sizeof(TDE2_RECT_S));
    memcpy(&stQuickResize.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    memcpy(&stQuickResize.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));

#ifndef USE_USER_SPACE_GPE
    return ioctl(g_s32TdeFd, TDE_QUICK_RESIZE, &stQuickResize);
#else
    return GpeOsiQuickResize(s32Handle, pstSrc, pstSrcRect, pstDst, pstDstRect);
#endif
}

/*****************************************************************************
* Function:      MT_TDE2_QuickFlicker
* Description:   Deflicker source bitmap and export to target bitmap, source and target bitmap can be the same
* Input:         s32Handle: Task handle
*                pSrc: Source bitmap info struct
*                pDst: Target bitmap info struct
* Output:        None
* Return:        >0: Current Task id; <0: Failure
* Others:        None
*****************************************************************************/
mt_s32 MT_TDE2_QuickDeflicker(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
                              TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect)

{
//    TDE_QUICKDEFLICKER_CMD_S stQuickDeflicker = { 0 };

    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#if 0
    TDE_CHECK_FD();

    if ((NULL == pstSrc) || (NULL == pstSrcRect) || (NULL == pstDst) || (NULL == pstDstRect)) {
	return MT_ERR_TDE_NULL_PTR;
    }

    stQuickDeflicker.s32Handle = s32Handle;

    memcpy(&stQuickDeflicker.stSrc, pstSrc, sizeof(TDE2_SURFACE_S));
    memcpy(&stQuickDeflicker.stSrcRect, pstSrcRect, sizeof(TDE2_RECT_S));
    memcpy(&stQuickDeflicker.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    memcpy(&stQuickDeflicker.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));

    return ioctl(g_s32TdeFd, TDE_QUICK_DEFLICKER, &stQuickDeflicker);
#endif    
}

/*****************************************************************************
* Function:      MT_TDE2_SolidDraw
* Description:   Operate src1 with src2 and export the result to pDst,set operation in pOpt
*                If src bitmap is mb format, it just support single source, which say is only set either pSrc1 or pSrc2
* Input:         s32Handle: Task handle
*                pSrc: source1 bitmap info struct
*                pstDst: target bitmap information struct
*                pstFillColor:  target bitmap info struct
*                pstOpt:  operate argument setting struct
* Output:        none
* Return:        >0: return task id of current operate; <0: fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_SolidDraw(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                         TDE2_SURFACE_S *pstDst,
                         TDE2_RECT_S *pstDstRect, TDE2_FILLCOLOR_S *pstFillColor,
                         TDE2_OPT_S *pstOpt)
{
//    TDE_SOLIDDRAW_CMD_S stSolidDraw = { 0 };

    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#if 0
    TDE_CHECK_FD();

    stSolidDraw.s32Handle = s32Handle;

    if ((NULL == pstForeGround)) {
	stSolidDraw.u32NullIndicator = (1 << 1);
    } else {
	memcpy(&stSolidDraw.stForeGround, pstForeGround, sizeof(TDE2_SURFACE_S));
    }

    if (NULL == pstForeGroundRect) {
	stSolidDraw.u32NullIndicator |= (1 << 2);
    } else {
	memcpy(&stSolidDraw.stForeGroundRect, pstForeGroundRect, sizeof(TDE2_RECT_S));
    }

    if ((NULL == pstDst)) {
	return MT_ERR_TDE_NULL_PTR;
    } else {
	memcpy(&stSolidDraw.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    }

    if (NULL == pstDstRect) {
	return MT_ERR_TDE_NULL_PTR;
    } else {
	memcpy(&stSolidDraw.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));
    }

    if (NULL == pstFillColor) {
	stSolidDraw.u32NullIndicator |= (1 << 5);
    } else {
	memcpy(&stSolidDraw.stFillColor, pstFillColor, sizeof(TDE2_FILLCOLOR_S));
    }

    if (NULL == pstOpt) {
	stSolidDraw.u32NullIndicator |= (1 << 6);
    } else {
	memcpy(&stSolidDraw.stOpt, pstOpt, sizeof(TDE2_OPT_S));
    }

    return ioctl(g_s32TdeFd, TDE_SOLID_DRAW, &stSolidDraw);
#endif    
}
#endif

mt_s32 MT_TDE2_Draw_3d_Trapez(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                         TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect, 
                         TDE2_TRAPEZ_OPT_S *pstOpt)
{
    MT_TDE2_FUN_IN;

    TDE_CHECK_FD();

#ifndef USE_USER_SPACE_GPE
    return 0;
#else
    return GpeOsiDraw_3d_trapez(s32Handle, pstForeGround, pstForeGroundRect, pstDst, pstDstRect, pstOpt);
#endif

}


mt_s32 ADP_TDEMBSurfaceToTDESurface(TDE2_MB_S *pTDEMBSurface, TDE2_SURFACE_S *pTDESurface)
{
	if (NULL == pTDESurface)
	{
		return MT_FAILURE;
	}

	switch(pTDEMBSurface->enMbFmt)
	{
	case TDE2_MB_COLOR_FMT_JPG_YCbCr400MBP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr400MBP;
		break;
	case TDE2_MB_COLOR_FMT_JPG_YCbCr422MBHP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr422MBHP;
		break;
	case TDE2_MB_COLOR_FMT_JPG_YCbCr422MBVP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr422MBVP;	
		break;
	case TDE2_MB_COLOR_FMT_MP1_YCbCr420MBP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_MP1_YCbCr420MBP;	
		break;
	case TDE2_MB_COLOR_FMT_MP2_YCbCr420MBP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_MP2_YCbCr420MBP;
		break;
	case TDE2_MB_COLOR_FMT_MP2_YCbCr420MBI:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_MP2_YCbCr420MBI;	
		break;
	case TDE2_MB_COLOR_FMT_JPG_YCbCr420MBP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr420MBP;	
		break;
	case TDE2_MB_COLOR_FMT_JPG_YCbCr444MBP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr444MBP;		
		break;
	case TDE2_MB_COLOR_FMT_JPG_SP_CMYK:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_SP_CMYK;		
		break;		
#ifdef CONFIG_MT_FPGA_GPE	
	case TDE2_MB_COLOR_FMT_TILE:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_TILE;		
		break;		
#endif		
	default:
		break;
	}
	
	pTDESurface->u32PhyAddr = pTDEMBSurface->u32YPhyAddr;
	pTDESurface->u32CbCrPhyAddr = pTDEMBSurface->u32CbCrPhyAddr;
	pTDESurface->u32VirAddr = pTDEMBSurface->u32YVirAddr;
	pTDESurface->u32CbCrVirAddr = pTDEMBSurface->u32CbCrVirAddr;
	pTDESurface->u32Stride = pTDEMBSurface->u32YStride;
	pTDESurface->u32CbCrStride = pTDEMBSurface->u32CbCrStride;
    pTDESurface->u32Height = pTDEMBSurface->u32YHeight;
    pTDESurface->u32Width = pTDEMBSurface->u32YWidth;
    pTDESurface->u32Stride = pTDEMBSurface->u32YStride;

	pTDESurface->pu8ClutPhyAddr = 0;
	pTDESurface->bYCbCrClut = MT_FALSE;

    /** below to be modified*/
    pTDESurface->bAlphaMax255 = MT_TRUE;
    pTDESurface->bAlphaExt1555 = MT_TRUE;
    pTDESurface->u8Alpha0 = 0;
    pTDESurface->u8Alpha1 = 255;
    return MT_SUCCESS;
}

mt_s32 ADP_TDEMBOptToTDEOpt(TDE2_MBOPT_S *pStMbOpt, TDE2_OPT_S *pStOpt)
{
	pStOpt->bResize = pStMbOpt->enResize;
	pStOpt->enClipMode = pStMbOpt->enClipMode;
	pStOpt->enDeflickerMode = pStMbOpt->bDeflicker;
	if(pStMbOpt->bSetOutAlpha)
	{
		pStOpt->enOutAlphaFrom = TDE2_OUTALPHA_FROM_GLOBALALPHA;
		pStOpt->u8GlobalAlpha = pStMbOpt->u8OutAlpha;
	}
	else
	{
		pStOpt->enOutAlphaFrom = TDE2_OUTALPHA_FROM_GLOBALALPHA;
		pStOpt->u8GlobalAlpha = 0xff;
	}
	pStOpt->enClipMode = pStMbOpt->enClipMode;
	pStOpt->stClipRect.s32Xpos = pStMbOpt->stClipRect.s32Xpos;
	pStOpt->stClipRect.s32Ypos = pStMbOpt->stClipRect.s32Ypos;
	pStOpt->stClipRect.u32Width = pStMbOpt->stClipRect.u32Width;
	pStOpt->stClipRect.u32Height = pStMbOpt->stClipRect.u32Height;
	return MT_SUCCESS;
}

/*****************************************************************************
* Function:      MT_TDE2_MbBlit
* Description:   MB blit interface
* Input:         s32Handle: task handle
*                pY:    brightness info struct
*                pCbCr: chroma information struct
*                pDst:  target bitmap inforamtion struct
*                pMbOpt: operate argument setting struct
* Output:        none
* Return:        >0: return task id of current operate; <0: fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_MbBlit(TDE_HANDLE s32Handle, TDE2_MB_S *pstMB, TDE2_RECT_S *pstMbRect, TDE2_SURFACE_S *pstDst,
                      TDE2_RECT_S *pstDstRect,
                      TDE2_MBOPT_S *pstMbOpt)
{
	TDE2_OPT_S stOpt = {0};
	TDE2_SURFACE_S TDESurface;

    MT_TDE2_FUN_IN;
//    MT_TDE2_RETURN();

    TDE_CHECK_FD();

    if ((NULL == pstMB) || (NULL == pstDst) || (NULL == pstMbOpt)) {
	return MT_ERR_TDE_NULL_PTR;
    }	
#ifndef TDE_BOOT
#ifndef USE_USER_SPACE_GPE
    TDE_MBBITBLT_CMD_S stMbBlit = { 0 };
    stMbBlit.s32Handle = s32Handle;
    memcpy(&stMbBlit.stMB, pstMB, sizeof(TDE2_MB_S));
    memcpy(&stMbBlit.stMbRect, pstMbRect, sizeof(TDE2_RECT_S));
    memcpy(&stMbBlit.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    memcpy(&stMbBlit.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));
    memcpy(&stMbBlit.stMbOpt, pstMbOpt, sizeof(TDE2_MBOPT_S));

    return ioctl(g_s32TdeFd, TDE_MB_BITBLT, &stMbBlit);
#else

	ADP_TDEMBSurfaceToTDESurface(pstMB, &TDESurface);
	ADP_TDEMBOptToTDEOpt(pstMbOpt, &stOpt);	
	return MT_TDE2_Bitblit(s32Handle, NULL, NULL,
	                       &TDESurface, pstMbRect, pstDst,
	                       pstDstRect, &stOpt);

#endif
#else
    return TdeOsiMbBlit(s32Handle, pstMB, pstMbRect, pstDst, pstDstRect, pstMbOpt);
#endif
}

/*****************************************************************************
* Function:      MT_TDE2_EndJob
* Description:   submit TDE2 task
* Input:         s32Handle: task handle
*                bSync: if synchronization
*                bBlock: if block
*                u32TimeOut: timeout value(unit by 10ms)
* Output:        none
* Return:        success/fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_EndJob(TDE_HANDLE s32Handle, MT_BOOL bSync, MT_BOOL bBlock, mt_u32 u32TimeOut)
{
    MT_TDE2_FUN_IN;
	mt_s32 ret;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();
#ifndef TDE_BOOT
#ifndef USE_USER_SPACE_GPE
    TDE_ENDJOB_CMD_S stEndJob;
    /* Disable sync function */
    bSync = MT_FALSE;

    stEndJob.s32Handle = s32Handle;
    stEndJob.bSync = bSync;
    stEndJob.bBlock = bBlock;
    stEndJob.u32TimeOut = u32TimeOut;

    return ioctl(g_s32TdeFd, TDE_END_JOB, &stEndJob);
#else
    ret = GpeOsiEndJob(s32Handle, bBlock, u32TimeOut, bSync, MT_NULL, MT_NULL);

	/*pthread_mutex_unlock(&gpe_mutex);   */
    sem_post(gpe_mutex);
	return ret;

#endif

#else
    return TdeOsiEndJob(s32Handle, bBlock, u32TimeOut, bSync, MT_NULL, MT_NULL);
#endif
}

/*****************************************************************************
* Function:      MT_TDE2_WaitForDone
* Description:   wait for completion of sumit TDE2 operate
* Input:         s32Handle: task handle
* Output:        none
* Return:        true: assigned task completed  / false: assigned task not completed
* Others:        none
*****************************************************************************/
#ifndef TDE_BOOT
mt_s32 MT_TDE2_WaitForDone(TDE_HANDLE s32Handle)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#ifndef USE_USER_SPACE_GPE

    TDE_CHECK_FD();

    return ioctl(g_s32TdeFd, TDE_WAITFORDONE, &s32Handle);
#endif    
}

/*****************************************************************************
* Function:      MT_TDE2_WaitAllDone
* Description:   wait for all TDE operate if all be done
* Input:         none
* Output:        none
* Return:        success / fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_WaitAllDone()
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#ifndef USE_USER_SPACE_GPE

    TDE_CHECK_FD();

    return ioctl(g_s32TdeFd, TDE_WAITALLDONE);
#endif    
}

/*****************************************************************************
* Function:      MT_TDE2_Reset
* Description:   reset TDE all states
* Input:         none
* Output:        none
* Return:        success / fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_Reset(mt_void)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#ifndef USE_USER_SPACE_GPE

    TDE_CHECK_FD();

    return ioctl(g_s32TdeFd, TDE_RESET);
#endif    
}

/*****************************************************************************
* Function:      MT_TDE2_CancelJob
* Description:   delete TDE2 tasks created, effective called before endjob
* Input:         s32Handle: task handle
* Output:        none
* Return:        success / fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_CancelJob(TDE_HANDLE s32Handle)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#ifndef USE_USER_SPACE_GPE

    TDE_CHECK_FD();

    return ioctl(g_s32TdeFd, TDE_CANCEL_JOB, &s32Handle);
#endif    
}

/*****************************************************************************
* Function:      MT_TDE2_BitmapMaskRop
* Description:   Ropmask source2 with mask bitmap at first, Ropmask source1 with middle bitmap at then
*                output result into target bitmap.
* Input:         s32Handle: task handle
*                pstBackGround: background bitmap information struct
*                pstBackGroundRect: background bitmap operate rect
*                pstForeGround: foreground bitmap information struct
*                pstForeGroundRect: foreground bitmap operate rect
*                pstMask: bitmap info struct of doing mask operate
*                pstMaskRect: bitmap operate rect of  mask operate
*                pstDst:  target bitmap information struct
*                pstDstRect:  target bitmap operate rect
* Output:        none
* Return:        >0: return task id of current operate; <0: fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_BitmapMaskRop(TDE_HANDLE s32Handle,
                             TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                             TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                             TDE2_SURFACE_S *pstMask, TDE2_RECT_S *pstMaskRect,
                             TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                             TDE2_ROP_CODE_E enRopCode_Color, TDE2_ROP_CODE_E enRopCode_Alpha)
{
#ifndef USE_USER_SPACE_GPE
    TDE_BITMAP_MASKROP_CMD_S stBitmapMaskRop;    

    MT_TDE2_FUN_IN;
//    MT_TDE2_RETURN();

    TDE_CHECK_FD();
    if ((NULL == pstBackGround) || (NULL == pstBackGroundRect) || (NULL == pstForeGround) || (NULL == pstForeGroundRect) || (NULL == pstMask) || (NULL == pstMaskRect) || (NULL == pstDst) || (NULL == pstDstRect)) {
	return MT_ERR_TDE_NULL_PTR;
    }

    stBitmapMaskRop.s32Handle = s32Handle;
    stBitmapMaskRop.enRopCode_Alpha = enRopCode_Alpha;
    stBitmapMaskRop.enRopCode_Color = enRopCode_Color;
    memcpy(&stBitmapMaskRop.stBackGround, pstBackGround, sizeof(TDE2_SURFACE_S));
    memcpy(&stBitmapMaskRop.stBackGroundRect, pstBackGroundRect, sizeof(TDE2_RECT_S));
    memcpy(&stBitmapMaskRop.stForeGround, pstForeGround, sizeof(TDE2_SURFACE_S));
    memcpy(&stBitmapMaskRop.stForeGroundRect, pstForeGroundRect, sizeof(TDE2_RECT_S));
    memcpy(&stBitmapMaskRop.stMask, pstMask, sizeof(TDE2_SURFACE_S));
    memcpy(&stBitmapMaskRop.stMaskRect, pstMaskRect, sizeof(TDE2_RECT_S));
    memcpy(&stBitmapMaskRop.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    memcpy(&stBitmapMaskRop.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));

    return ioctl(g_s32TdeFd, TDE_BITMAP_MASKROP, &stBitmapMaskRop);
#else
    TDE2_OPT_S stOpt = {0};
    stOpt.enAluCmd = TDE2_ALUCMD_ROP;    
    stOpt.enRopCode_Alpha = enRopCode_Alpha;
    stOpt.enRopCode_Color = enRopCode_Color;
    stOpt.enAMapLogical = MT_TRUE;
    return GpeOsiBlit_3src(s32Handle, pstBackGround, pstBackGroundRect, 
                              pstForeGround, pstForeGroundRect, pstMask, pstMaskRect, pstDst, pstDstRect, &stOpt);
#endif
}

/*****************************************************************************
* Function:      MT_TDE2_BitmapMaskBlend
* Description:   blendmask source2 with mask bitmap at first, and then blend source1 with middle bitmap
*                put output result into target bitmap
* Input:         s32Handle: task handle
*                pstBackGround: background bitmap information struct
*                pstBackGroundRect: background bitamp operate rect
*                pstForeGround: foreground bitmap information struct
*                pstForeGroundRect: foreground bitamp operate rect
*                pstMask:  bitmap info struct of doing mask operate
*                pstMaskRect: bitmap operate rect of  mask operate
*                pstDst:  target bitmap information struct
*                pstDstRect:  target bitmap operate rect
*                u8Alpha:  alpha value is operated
* Output:        none
* Return:        >0: return task id of current operate; <0: fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_BitmapMaskBlend(TDE_HANDLE s32Handle,
                               TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                               TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                               TDE2_SURFACE_S *pstMask, TDE2_RECT_S *pstMaskRect,
                               TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                               mt_u8 u8Alpha, TDE2_ALUCMD_E enBlendMode)
{
//    TDE_BITMAP_MASKBLEND_CMD_S stBitmapMaskBlend;

    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#if 0
    TDE_CHECK_FD();
    if ((NULL == pstBackGround) || (NULL == pstBackGroundRect) || (NULL == pstForeGround) || (NULL == pstForeGroundRect) || (NULL == pstMask) || (NULL == pstMaskRect) || (NULL == pstDst) || (NULL == pstDstRect)) {
	MT_TDE2_FUN_OUT;
	return MT_ERR_TDE_NULL_PTR;
    }

    if (TDE2_ALUCMD_BLEND != enBlendMode) {
	MT_TDE2_FUN_OUT;
	return MT_ERR_TDE_INVALID_PARA;
    }

    stBitmapMaskBlend.s32Handle = s32Handle;
    stBitmapMaskBlend.u8Alpha = u8Alpha;
    stBitmapMaskBlend.enBlendMode = enBlendMode;

    memcpy(&stBitmapMaskBlend.stBackGround, pstBackGround, sizeof(TDE2_SURFACE_S));
    memcpy(&stBitmapMaskBlend.stBackGroundRect, pstBackGroundRect, sizeof(TDE2_RECT_S));
    memcpy(&stBitmapMaskBlend.stForeGround, pstForeGround, sizeof(TDE2_SURFACE_S));
    memcpy(&stBitmapMaskBlend.stForeGroundRect, pstForeGroundRect, sizeof(TDE2_RECT_S));
    memcpy(&stBitmapMaskBlend.stMask, pstMask, sizeof(TDE2_SURFACE_S));
    memcpy(&stBitmapMaskBlend.stMaskRect, pstMaskRect, sizeof(TDE2_RECT_S));
    memcpy(&stBitmapMaskBlend.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    memcpy(&stBitmapMaskBlend.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));

    MT_TDE2_FUN_OUT;
    return ioctl(g_s32TdeFd, TDE_BITMAP_MASKBLEND, &stBitmapMaskBlend);
#endif    
}

/*****************************************************************************
* Function:      MT_TDE2_ImageMultiply
* Description:   image multiply/stencil source1 bitmap at first, and then blend/rop source2 with dst bitmap
*                put output result into target bitmap
* Input:         s32Handle: task handle
*                pstBackGround: background bitmap information struct
*                pstBackGroundRect: background bitamp operate rect
*                pstForeGround: foreground bitmap information struct
*                pstForeGroundRect: foreground bitamp operate rect
*                pstPattern:  bitmap info struct of doing multiply operate
*                pstPatternRect: bitmap operate rect of  multiply operate
*                pstDst:  target bitmap information struct
*                pstDstRect:  target bitmap operate rect
*                pMultiplyOpt:  multiply option
* Output:        none
* Return:        >0: return task id of current operate; <0: fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_ImageMultiply(TDE_HANDLE s32Handle,
                             TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                             TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                             TDE2_SURFACE_S *pstPattern, TDE2_RECT_S *pstPatternRect,
                             TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                             TDE2_MULTIPLY_OPT_S *pMultiplyOpt)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#ifndef USE_USER_SPACE_GPE
    TDE_IMAGE_MULTIPLY_CMD_S stImageMultipyCmd;

    TDE_CHECK_FD();
    if ((NULL == pstBackGround) || (NULL == pstBackGroundRect) || (NULL == pstForeGround) || (NULL == pstForeGroundRect) || (NULL == pstPattern) || (NULL == pstPatternRect) || (NULL == pstDst) || (NULL == pstDstRect)) {
	return MT_ERR_TDE_NULL_PTR;
    }

    stImageMultipyCmd.s32Handle = s32Handle;
    memcpy(&stImageMultipyCmd.opt, pMultiplyOpt, sizeof(TDE2_MULTIPLY_OPT_S));

    memcpy(&stImageMultipyCmd.stBackGround, pstBackGround, sizeof(TDE2_SURFACE_S));
    memcpy(&stImageMultipyCmd.stBackGroundRect, pstBackGroundRect, sizeof(TDE2_RECT_S));
    memcpy(&stImageMultipyCmd.stForeGround, pstForeGround, sizeof(TDE2_SURFACE_S));
    memcpy(&stImageMultipyCmd.stForeGroundRect, pstForeGroundRect, sizeof(TDE2_RECT_S));
    memcpy(&stImageMultipyCmd.stPattern, pstPattern, sizeof(TDE2_SURFACE_S));
    memcpy(&stImageMultipyCmd.stPatternRect, pstPatternRect, sizeof(TDE2_RECT_S));
    memcpy(&stImageMultipyCmd.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    memcpy(&stImageMultipyCmd.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));

    return ioctl(g_s32TdeFd, TDE_IMAGE_MULTIPLY, &stImageMultipyCmd);
#endif    
}

mt_s32 MT_TDE2_SetDeflickerLevel(TDE_DEFLICKER_LEVEL_E enDeflickerLevel)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();

#if 0
    TDE_CHECK_FD();
    if (TDE_DEFLICKER_BUTT <= enDeflickerLevel) {
	return MT_ERR_TDE_INVALID_PARA;
    }

    return ioctl(g_s32TdeFd, TDE_SET_DEFLICKERLEVEL, &enDeflickerLevel);
#endif    
}

mt_s32 MT_TDE2_GetDeflickerLevel(TDE_DEFLICKER_LEVEL_E *pDeflickerLevel)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#if 0
    TDE_CHECK_FD();
    if (MT_NULL == pDeflickerLevel) {
	return MT_ERR_TDE_NULL_PTR;
    }

    return ioctl(g_s32TdeFd, TDE_GET_DEFLICKERLEVEL, pDeflickerLevel);
#endif    
}
#endif

mt_s32 MT_TDE2_SetAlphaThresholdValue(mt_u8 u8ThresholdValue)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();

#if 0
    TDE_CHECK_FD();
#ifndef TDE_BOOT
    return ioctl(g_s32TdeFd, TDE_SET_ALPHATHRESHOLD_VALUE, &u8ThresholdValue);
#else
    return TdeOsiSetAlphaThresholdValue(u8ThresholdValue);
#endif
#endif
}

#ifndef TDE_BOOT
mt_s32 MT_TDE2_GetAlphaThresholdValue(mt_u8 *pu8ThresholdValue)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#if 0
    TDE_CHECK_FD();
    if (MT_NULL == pu8ThresholdValue) {
	return MT_ERR_TDE_NULL_PTR;
    }

    return ioctl(g_s32TdeFd, TDE_GET_ALPHATHRESHOLD_VALUE, pu8ThresholdValue);
#endif    
}
#endif

mt_s32 MT_TDE2_SetAlphaThresholdState(MT_BOOL bEnAlphaThreshold)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#if 0
    TDE_CHECK_FD();
#ifndef TDE_BOOT
    return ioctl(g_s32TdeFd, TDE_SET_ALPHATHRESHOLD_STATE, &bEnAlphaThreshold);
#else
    return TdeOsiSetAlphaThresholdState(bEnAlphaThreshold);
#endif
#endif
}

#ifndef TDE_BOOT
mt_s32 MT_TDE2_GetAlphaThresholdState(MT_BOOL *p_bEnAlphaThreshold)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#if 0

    TDE_CHECK_FD();

    if (MT_NULL == p_bEnAlphaThreshold) {
	return MT_ERR_TDE_NULL_PTR;
    }

    return ioctl(g_s32TdeFd, TDE_GET_ALPHATHRESHOLD_STATE, p_bEnAlphaThreshold);
#endif    
}

mt_s32 MT_TDE2_PatternFill(TDE_HANDLE s32Handle,
                           TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                           TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                           TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                           TDE2_PATTERN_FILL_OPT_S *pstOpt)
{
 //   TDE_PATTERN_FILL_CMD_S stPatternFillCmd = { 0 };

    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#if 0
    TDE_CHECK_FD();

    stPatternFillCmd.s32Handle = s32Handle;

    if (MT_NULL == pstBackGround) {
	stPatternFillCmd.u32NullIndicator |= (0x1 << 1);
    } else {
	memcpy(&stPatternFillCmd.stBackGround, pstBackGround, sizeof(TDE2_SURFACE_S));
    }

    if (MT_NULL == pstBackGroundRect) {
	stPatternFillCmd.u32NullIndicator |= (0x1 << 2);
    } else {
	memcpy(&stPatternFillCmd.stBackGroundRect, pstBackGroundRect, sizeof(TDE2_RECT_S));
    }
    if (MT_NULL == pstForeGround) {
	stPatternFillCmd.u32NullIndicator |= (0x1 << 3);
    } else {
	memcpy(&stPatternFillCmd.stForeGround, pstForeGround, sizeof(TDE2_SURFACE_S));
    }

    if (MT_NULL == pstForeGroundRect) {
	stPatternFillCmd.u32NullIndicator |= (0x1 << 4);
    } else {
	memcpy(&stPatternFillCmd.stForeGroundRect, pstForeGroundRect, sizeof(TDE2_RECT_S));
    }
    if (MT_NULL == pstDst) {
	stPatternFillCmd.u32NullIndicator |= (0x1 << 5);
    } else {
	memcpy(&stPatternFillCmd.stDst, pstDst, sizeof(TDE2_SURFACE_S));
    }

    if (MT_NULL == pstDstRect) {
	stPatternFillCmd.u32NullIndicator |= (0x1 << 6);
    } else {
	memcpy(&stPatternFillCmd.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));
    }

    if (MT_NULL == pstOpt) {
	stPatternFillCmd.u32NullIndicator |= (0x1 << 7);
    } else {
	memcpy(&stPatternFillCmd.stOpt, pstOpt, sizeof(TDE2_PATTERN_FILL_OPT_S));
    }

    return ioctl(g_s32TdeFd, TDE_PATTERN_FILL, &stPatternFillCmd);
#endif 
}

mt_s32 MT_TDE2_EnableRegionDeflicker(MT_BOOL bRegionDeflicker)
{
    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();
#if 0
    return ioctl(g_s32TdeFd, TDE_ENABLE_REGIONDEFLICKER, &bRegionDeflicker);
#endif
}

mt_s32 MT_TDE2_MultiBlending(TDE_HANDLE s32Handle, TDE_SURFACE_LIST_S *pstSurfaceList)
{
    mt_s32 i = 0;

    MT_TDE2_FUN_IN;
    MT_TDE2_RETURN();

    if ((TDE_HANDLE)MT_ERR_TDE_INVALID_HANDLE == s32Handle) {
        return MT_FAILURE;
    }

    if (NULL == pstSurfaceList) {
        return MT_FAILURE;
    }

    mt_s32 blitnum = (mt_s32)(pstSurfaceList->u32SurfaceNum);
    TDE2_SURFACE_S dstSur = { 0 };
    memcpy(&dstSur, pstSurfaceList->pDstSurface, sizeof(TDE2_SURFACE_S));

    for (i = 0; i < blitnum; i++) {
        TDE2_SURFACE_S srcSur = { 0 };
        TDE2_RECT_S srcRect = { 0 };
        TDE2_RECT_S dstRect = { 0 };
        TDE2_OPT_S stOpt;

        memset(&stOpt, 0, sizeof(TDE2_OPT_S));
        memcpy(&srcSur, &(pstSurfaceList->pstComposor[i].stSrcSurface), sizeof(TDE2_SURFACE_S));
        memcpy(&srcRect, &(pstSurfaceList->pstComposor[i].stInRect), sizeof(TDE2_RECT_S));
        memcpy(&dstRect, &(pstSurfaceList->pstComposor[i].stOutRect), sizeof(TDE2_RECT_S));
        memcpy(&stOpt, &(pstSurfaceList->pstComposor[i].stOpt), sizeof(TDE2_OPT_S));

        mt_s32 s32Ret = MT_TDE2_Bitblit(s32Handle, &dstSur, &dstRect, &srcSur, &srcRect,
                &dstSur, &dstRect, &stOpt);

        if (MT_SUCCESS != s32Ret) {
            MT_TDE2_CancelJob(s32Handle);
            return s32Ret;
        }
    }

    return MT_SUCCESS;
}


mt_s32 MT_TDE2_VScreenCapture(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstDst,
                       TDE2_RECT_S *pstDstRect,
                       TDE2_OPT_S *pstOpt)
{

    return GpeOsiVideoScreenCapture(s32Handle, pstDst, pstDstRect, pstOpt);
}


mt_s32 MT_TDE2_ClipMask(TDE_HANDLE s32Handle,
										   TDE2_SURFACE_S *pstForeGround,
										   TDE2_SURFACE_S *pstDst,
										   TDE2_RECT_S *pstForeGroundRect,
										   TDE2_RECT_S *pstDstRect,
										   TDE2_MASK_OPT_E enMaskOpt)
{
	return GpeClipMask(s32Handle, pstForeGround, pstDst, pstForeGroundRect, pstDstRect, enMaskOpt);
}


mt_s32 MT_TDE2_RotateAribtraryAngle(TDE_HANDLE s32Handle,
                                        TDE2_SURFACE_S *pstSrc,
                                        TDE2_SURFACE_S *pstDst,
                                        TDE2_RECT_S *pstSrcRect,
                                        TDE2_POS_S *pDstPos,
                                        TDE2_ROTATOR_ANGLE_S *pAngle,
                                        TDE2_TRAPEZ_OPT_S *pOpt)
{
 	return GpeOsiRotateAribtraryAngle(s32Handle, pstSrc, pstDst, pstSrcRect,
                                        pDstPos, pAngle, pOpt);
}

mt_s32 MT_TDE2_ExpendColorkey(TDE2_FILLCOLOR_S *pstFillColor, TDE2_COLORKEY_U *pKeyValue)
{
	mt_u32 colorkey;
	colorkey = colorkey_expend(pstFillColor->enColorFmt, pstFillColor->u32FillColor);
	return getColorkeyMinMax(colorkey,pstFillColor->enColorFmt, pKeyValue);
}


#endif

#ifdef SUPPORT_CMDFIFO
TDE_HANDLE MT_TDE2_CmdFifo_CreateBegin(mt_void)
{
    mt_s32 ret;
    cmdfifo_cfg_t cmdfifo_cfg = {0};
    TDE_HANDLE s32Handle = 0;

    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    pthread_mutex_lock(&gpe_cf_mutex); 
    cmdfifo_cfg.is_sync = MT_FALSE;
    cmdfifo_cfg.is_round = MT_FALSE;
    cmdfifo_cfg.async_conf_rst = 0;
    cmdfifo_cfg.async_conf_mod = 0;
    cmdfifo_cfg.async_conf_recovery = 0;
    ret = spn_cmdfifo_create_list_begin(&s32Handle, &cmdfifo_cfg);
    if (ret != MT_SUCCESS) {
      pthread_mutex_unlock(&gpe_cf_mutex); 
    }
    return s32Handle;
}
TDE_HANDLE MT_TDE2_CmdFifo_CreateBeginEx(MT_BOOL syncMode)
{
    mt_s32 ret;
    cmdfifo_cfg_t cmdfifo_cfg = {0};
    TDE_HANDLE s32Handle = 0;

    MT_TDE2_FUN_IN;
    TDE_CHECK_FD();

    pthread_mutex_lock(&gpe_cf_mutex); 
    if(syncMode == MT_FALSE)
    {
	    cmdfifo_cfg.is_sync = MT_FALSE;
	    cmdfifo_cfg.is_round = MT_FALSE;
	    cmdfifo_cfg.async_conf_rst = 0;
	    cmdfifo_cfg.async_conf_mod = 0;
	    cmdfifo_cfg.async_conf_recovery = 0;
    }
    else
    {
		cmdfifo_cfg.is_sync = MT_TRUE;
		cmdfifo_cfg.is_round = MT_FALSE;
		cmdfifo_cfg.sync_signal = 1;
		cmdfifo_cfg.sync_num = 0;
		cmdfifo_cfg.sync_delay_counter = 0;
		cmdfifo_cfg.halt_mode = 0;
    }
    ret = spn_cmdfifo_create_list_begin(&s32Handle, &cmdfifo_cfg);
    if (ret != MT_SUCCESS) {
      pthread_mutex_unlock(&gpe_cf_mutex); 
    }
    return s32Handle;
}

mt_s32 MT_TDE2_CmdFifo_CreateEnd(TDE_HANDLE s32Handle)
{
    mt_s32 ret;
	if (s32Handle == (TDE_HANDLE)NULL)
	{
	  pthread_mutex_unlock(&gpe_cf_mutex); 
	  return MT_FAILURE;
	}

    MT_TDE2_FUN_IN;
    TDE_CHECK_FD();

    ret = spn_cmdfifo_create_list_end(s32Handle);
    if (ret != MT_SUCCESS) {
      spn_cmdfifo_destroy(s32Handle);
      pthread_mutex_unlock(&gpe_cf_mutex); 
    }
    return ret;
}
mt_s32 MT_TDE2_CmdFifo_Run(TDE_HANDLE s32Handle)
{
  mt_s32 ret;	
  if (s32Handle == (TDE_HANDLE)NULL)
  	return MT_FAILURE;

  ret = spn_cmdfifo_run(s32Handle);
  return ret;

}
mt_s32 MT_TDE2_CmdFifo_Destroy(TDE_HANDLE s32Handle)
{
  if (s32Handle == (TDE_HANDLE)NULL)
  {
	pthread_mutex_unlock(&gpe_cf_mutex); 
	return MT_FAILURE;
  }
  spn_cmdfifo_destroy(s32Handle);
  pthread_mutex_unlock(&gpe_cf_mutex); 
  return MT_SUCCESS;
}
mt_u32 MT_TDE2_CmdFifo_getNodeCnt(mt_void)
{
  return spn_cmdfifo_get_node_cnt();
}

mt_s32 MT_TDE2_CmdFifo_BatchFill(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                         mt_u32 *pFillData, mt_u32 num, mt_u32 *ret_num)
{
    mt_u32 ret = MT_SUCCESS;
    mt_u32 i = 0;
    mt_u32 num_op = 0;
    mt_u32 num_finished = 0;
    MT_TDE2_FUN_IN;

    TDE_CHECK_FD();

    if ((NULL == pstDst) || (NULL == pstDstRect) || (NULL == pFillData) || (NULL == ret_num)) {
        return MT_ERR_TDE_NULL_PTR;
    }
    if(0 == num){
        return MT_ERR_TDE_INVALID_PARA;
    }
    num_op = (num > MAX_NODE_NUM) ? MAX_NODE_NUM : num;
    for(i = 0; i < num_op; i++) 
    {
        ret = GpeOsiQuickFill(s32Handle, pstDst, &pstDstRect[i], pFillData[i]);
        if(ret)
        {
            printf("MT_TDE2_BATCHFILL: Exit job unexpectly\n");
            break;
        }
        num_finished++;
    }
    *ret_num = num_finished;
    return ret;
}

mt_s32 MT_TDE2_CmdFifo_BatchBlit(TDE_HANDLE s32Handle, TDE_BLIT_LIST_S *pstBlitList, mt_u32 *ret_num)
{
    mt_s32 i = 0;
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u32 num_finished = 0;

    MT_TDE2_FUN_IN;

    if ((TDE_HANDLE)MT_ERR_TDE_INVALID_HANDLE == s32Handle) 
    {
        return MT_FAILURE;
    }

    if (NULL == pstBlitList) 
    {
        return MT_FAILURE;
    }

    mt_s32 blitnum = (mt_s32)(pstBlitList->u32BlitNum);
    TDE2_SURFACE_S *pDstSur = pstBlitList->pDstSurface;

    for (i = 0; i < blitnum; i++) 
    {
        TDE2_SURFACE_S *pSrcSur = pstBlitList->pstComposor[i].pstSrcSur;
        TDE2_RECT_S *pSrcRect = pstBlitList->pstComposor[i].pstInRect;
        TDE2_SURFACE_S *pBgSur = pstBlitList->pstComposor[i].pstBgSur;
        TDE2_RECT_S *pBgRect = pstBlitList->pstComposor[i].pstBgRect;
        TDE2_SURFACE_S *pMaskSur = pstBlitList->pstComposor[i].pstMaskSur;
        TDE2_RECT_S *pMaskRect = pstBlitList->pstComposor[i].pstMaskRect;
        TDE2_RECT_S *pDstRect = pstBlitList->pstComposor[i].pstOutRect;
        TDE2_OPT_S *pstOpt = pstBlitList->pstComposor[i].pstOpt;

        if(pMaskSur != NULL && pMaskRect->u32Height != 0 && pMaskRect->u32Width != 0)
        {
            s32Ret = MT_TDE2_Bitblit_3src(s32Handle, pBgSur, pBgRect, pSrcSur, pSrcRect, pMaskSur, pMaskRect,
                    pDstSur, pDstRect, pstOpt);
        }
        else
        {
            s32Ret = MT_TDE2_Bitblit(s32Handle, pBgSur, pBgRect, pSrcSur, pSrcRect,
                    pDstSur, pDstRect, pstOpt);
        }

        if (MT_SUCCESS != s32Ret) 
        {
            printf("MT_TDE2_BATCHFILL: Exit job unexpectly\n");
            break;
        }
        num_finished++;
    }

    *ret_num = num_finished;

    return s32Ret;
}

#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
