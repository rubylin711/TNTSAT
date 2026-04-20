#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/fcntl.h>  /* O_ACCMODE */
#include <linux/cdev.h>
#include <linux/uaccess.h> /* copy_*_user */
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/workqueue.h>
#include <linux/clk.h>
#include <asm/io.h>

#include "mt_mach/irq.h"

#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_drv_mem.h"
#include "drv_tde_ext.h"

#include "mt_tde_type.h"
#include "mt_drv_tde.h"
#include "tde_osictl.h"
#include "tde_osilist.h"
#include "tde_hal.h"
#include "tde_handle.h"
#include "wmalloc.h"
#include "tde_define.h"
#include "tde_config.h"
#include "mt_gfx_comm_k.h"
#include "mt_reg_common.h"
#include "mt_module_debug.h"
#include "tde_proc.h"
typedef unsigned long MT_UL;

#define TDE_NAME "MT_TDE"

//#define TDE_DEBUG_DISABLE_6
#if defined(TDE_DEBUG_DISABLE) || defined(TDE_DEBUG_DISABLE_6)
#define DUMP_LOG \
    do {         \
    } while (0)
#define TDE_FUN_IN DUMP_LOG
#define TDE_FUN_OUT DUMP_LOG
#define TDE_LOG(...) DUMP_LOG
#define TDE_LINE DUMP_LOG
#else
#define TDE_FUN_IN MT_INFO_TDE("------------in--------------\n")
#define TDE_FUN_OUT MT_INFO_TDE("------------out------------\n")
#define TDE_LOG MT_INFO_TDE
#define TDE_LINE MT_INFO_TDE("------------------\n")
#endif

STATIC spinlock_t s_taskletlock;
extern struct miscdevice gfx_dev;

STATIC int tde_osr_isr(int irq, void *dev_id);
STATIC void tde_tasklet_func(struct tasklet_struct * int_status);

/* TDE equipment quoted count */
STATIC atomic_t g_TDECount = ATOMIC_INIT(0);

#ifdef CONFIG_TDE_PM_ENABLE
int tde_pm_suspend(basedev_s *pdev, pm_message_t state);
int tde_pm_resume(basedev_s *pdev);
#endif
#ifdef TDE_TIME_COUNT
TDE_timeval_s g_stTimeStart;
TDE_timeval_s g_stTimeEnd;
mt_u64 g_u64TimeDiff;
#endif

DECLARE_TASKLET(tde_tasklet, tde_tasklet_func);
#ifdef CONFIG_TDE_TDE_EXPORT_FUNC
static TDE_EXPORT_FUNC_S s_TdeExportFuncs =
    {
        .pfnTdeOpen = TdeOsiOpen,
        .pfnTdeClose = TdeOsiClose,
        .pfnTdeBeginJob = TdeOsiBeginJob,
        .pfnTdeEndJob = TdeOsiEndJob,
        .pfnTdeCancelJob = TdeOsiCancelJob,
        .pfnTdeWaitForDone = TdeOsiWaitForDone,
        .pfnTdeWaitAllDone = TdeOsiWaitAllDone,
        .pfnTdeQuickCopy = TdeOsiQuickCopy,
        .pfnTdeQuickFill = TdeOsiQuickFill,
        .pfnTdeQuickResize = TdeOsiQuickResize,
        .pfnTdeQuickFlicker = TdeOsiQuickFlicker,
        .pfnTdeBlit = TdeOsiBlit,
        .pfnTdeMbBlit = TdeOsiMbBlit,
        .pfnTdeSolidDraw = TdeOsiSolidDraw,
        .pfnTdeSetDeflickerLevel = TdeOsiSetDeflickerLevel,
        .pfnTdeEnableRegionDeflicker = TdeOsiEnableRegionDeflicker,
        .pfnTdeSetAlphaThresholdValue = TdeOsiSetAlphaThresholdValue,
        .pfnTdeSetAlphaThresholdState = TdeOsiSetAlphaThresholdState,
        .pfnTdeGetAlphaThresholdState = TdeOsiGetAlphaThresholdState,
        .pfnTdeCalScaleRect = TdeCalScaleRect,
#ifdef CONFIG_TDE_PM_ENABLE
        .pfnTdeSuspend = tde_pm_suspend,
        .pfnTdeResume = tde_pm_resume,
#endif
    };
#endif

mt_s32 MT_TDE_MODULE_GetFunction(TDE_EXPORT_FUNC_S **ppFunc)
{
    TDE_FUN_IN;
    *ppFunc = &s_TdeExportFuncs;
    TDE_FUN_OUT;
    return 0;
};

mt_void tde_cleanup_module_k(mt_void);

extern void mt_tde_device_register(void);

typedef struct
{
    mt_u32  tdeFinishFlag;
    wait_queue_head_t    tdeFinishEvent;
}TDE_FINISH_INFO_S;

static TDE_FINISH_INFO_S s_TdeFinish;
static TDE_FINISH_INFO_S s_TdeCfFinish;
static TDE_FINISH_INFO_S s_TdeCfSyncFinish;

mt_void tde_queue_init(mt_void)
{
  s_TdeFinish.tdeFinishFlag = 0;
  init_waitqueue_head(&s_TdeFinish.tdeFinishEvent);
  s_TdeCfFinish.tdeFinishFlag = 0;
  init_waitqueue_head(&s_TdeCfFinish.tdeFinishEvent);  
  s_TdeCfSyncFinish.tdeFinishFlag = 0;
   init_waitqueue_head(&s_TdeCfSyncFinish.tdeFinishEvent);  
}

mt_s32 tde_wait_finish(mt_void)
{ 
  //mt_u32 Ct;
  mt_s32 ret;
  //mt_drv_sys_gettimestampms((mt_u32 *)&Ct);

//  printk("\r\n ~~~~~~~~%s, %d, %d", __FUNCTION__,__LINE__, Ct);
  s_TdeFinish.tdeFinishFlag = 0;
  TdeHalStart();
#ifdef CONFIG_MT_FPGA_GPE  
  ret = wait_event_interruptible_timeout(s_TdeFinish.tdeFinishEvent, s_TdeFinish.tdeFinishFlag, 5000);
#else
	ret = wait_event_interruptible_timeout(s_TdeFinish.tdeFinishEvent, s_TdeFinish.tdeFinishFlag, 100);
#endif  
  //mt_drv_sys_gettimestampms((mt_u32 *)&Ct);
//  printk("\r\n ~xx~~~~~~~%s, %d, %d, ret:%d, -ERESTARTSYS:%d", __FUNCTION__,__LINE__, Ct, ret, -ERESTARTSYS);
  if(ret == 0)
  {
#ifdef CONFIG_MT_FPGA_GPE
     printk("\r\n ~~~~~~~~%s, %d Err Timeout", __FUNCTION__,__LINE__);
#endif	 
     return -1;
  }
  else
    return 0;
}

mt_s32 tde_wait_cf_finish(mt_void)
{ 
  //mt_u32 Ct;
  mt_s32 ret;
  //mt_drv_sys_gettimestampms((mt_u32 *)&Ct);

//  printk("\r\n ~~~~~~~~%s, %d, %d", __FUNCTION__,__LINE__, Ct);

  s_TdeCfFinish.tdeFinishFlag = 0;
#ifndef CONFIG_MT_FPGA_GPE
  TdeHalCfStart();
  ret = wait_event_interruptible_timeout(s_TdeCfFinish.tdeFinishEvent, s_TdeCfFinish.tdeFinishFlag, 100);
#else
  ret = wait_event_interruptible_timeout(s_TdeCfFinish.tdeFinishEvent, s_TdeCfFinish.tdeFinishFlag, 5000);
#endif
  //mt_drv_sys_gettimestampms((mt_u32 *)&Ct);
//  printk("\r\n ~xx~~~~~~~%s, %d, %d, ret:%d, -ERESTARTSYS:%d", __FUNCTION__,__LINE__, Ct, ret, -ERESTARTSYS);
  if(ret == 0)
  {    
#ifdef CONFIG_MT_FPGA_GPE  
    printk("\r\n ~~~~~~~~%s, %d Err Timeout", __FUNCTION__,__LINE__);
#endif	
    return -1;
  }
  else
    return 0;
}
mt_s32 tde_wait_sync_cf_finish(mt_void)
{ 
  mt_s32 ret;

  s_TdeCfSyncFinish.tdeFinishFlag = 0;
  TdeHalCfSyncStart();
  ret = wait_event_interruptible_timeout(s_TdeCfSyncFinish.tdeFinishEvent, s_TdeCfSyncFinish.tdeFinishFlag, 100);
  if(ret == 0)
  {	
    return -1;
  }
  else
    return 0;
}

mt_s32 tde_init_module_k(mt_void)
{
    int ret;

    TDE_FUN_IN;
    if (TdeHalInit(TDE_REG_BASEADDR) < 0)
    {
        TDE_FUN_OUT;
        return -1;
    }
    tde_queue_init();

    if (0 != request_irq(TDE_INTNUM, (irq_handler_t)tde_osr_isr,
                         IRQF_TRIGGER_HIGH, "mt_tde_irq", MT_NULL))
    {
        TDE_TRACE(TDE_KERN_ERR, "request_irq for TDE failure!\n");
        TdeHalRelease();
        TDE_FUN_OUT;
        return -1;
    }
#ifndef USE_USER_SPACE_GPE  
    TdeOsiListInit();
#endif
    ret = mt_drv_module_register(MT_ID_TDE, TDE_NAME, &s_TdeExportFuncs);
    if (MT_SUCCESS != ret)
    {
        TDE_TRACE(TDE_KERN_ERR, "register module failed!\n");
#ifndef USE_USER_SPACE_GPE         
        tde_cleanup_module_k();
#endif
        TDE_FUN_OUT;
        return ret;
    }

    mt_tde_device_register();

    spin_lock_init(&s_taskletlock);

    TDE_FUN_OUT;
    return 0;
}

mt_void tde_cleanup_module_k(mt_void)
{
    TDE_FUN_IN;
    mt_drv_module_unregister(MT_ID_TDE);

#ifndef USE_USER_SPACE_GPE 

    TdeOsiListTerm();
#endif

    free_irq(TDE_INTNUM, NULL);
    TdeHalRelease();

    TDE_FUN_OUT;
    return;
}

int tde_open(struct inode *finode, struct file *ffile)
{
    MT_S32 mt_idx = iminor(finode);
    TED_PRIV_DATA_S *tde_priv_data = get_mt_priv(mt_idx);

    if (!IS_ERR_OR_NULL(tde_priv_data->gxaclk))
        clk_prepare_enable(tde_priv_data->gxaclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->hdclk))
        clk_prepare_enable(tde_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->sdclk_27m))
        clk_prepare_enable(tde_priv_data->sdclk_27m);

    //printk("tde_open: %s: %d\n", __FUNCTION__, __LINE__);
    //*(volatile unsigned int *)(0xf8af0018) = 0xffffffff;
    TDE_FUN_IN;
    if (1 == atomic_inc_return(&g_TDECount))
    {
        (mt_void) TdeHalOpen();
    }

    TDE_FUN_OUT;
    return 0;
}

int tde_release(struct inode *finode, struct file *ffile)
{
    MT_S32 mt_idx = iminor(finode);
    TED_PRIV_DATA_S *tde_priv_data = get_mt_priv(mt_idx);

    TDE_FUN_IN;
    if (atomic_dec_and_test(&g_TDECount))
    {
  //todo:
  //tasklet_kill(&tde_tasklet);
    }
#ifndef USE_USER_SPACE_GPE     
    TdeFreePendingJob();
#endif
    if (atomic_read(&g_TDECount) < 0)
    {
        atomic_set(&g_TDECount, 0);
    }
    TDE_FUN_OUT;

    if (!IS_ERR_OR_NULL(tde_priv_data->gxaclk))
        clk_disable_unprepare(tde_priv_data->gxaclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->hdclk))
        clk_disable_unprepare(tde_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->sdclk_27m))
        clk_disable_unprepare(tde_priv_data->sdclk_27m);

    return 0;
}

static long tde_set_proc(unsigned long arg)
{
	MT_TDE_PROC_INFO_S *proc = NULL;
	void __user *argp = (void __user *)arg;
	proc = (MT_TDE_PROC_INFO_S *)TDE_MALLOC(sizeof(MT_TDE_PROC_INFO_S));
	if(NULL == proc)
	{
		return -ENOMEM;
	}
	if (copy_from_user((MT_VOID *)proc, argp, sizeof(MT_TDE_PROC_INFO_S)))
	{
			return -EFAULT;
	} 
						
	TDEProcSetInfo(proc);
	TDE_FREE(proc);
	return 0;
}


static long tde_bit_blit(unsigned long arg)
{
	TDE_BITBLIT_CMD_S stBitBlt;
	TDE2_SURFACE_S *pstBackGround;
	TDE2_RECT_S *pstBackGroundRect;
	TDE2_SURFACE_S *pstForeGround;
	TDE2_RECT_S *pstForeGroundRect;
	TDE2_OPT_S *pstOpt;
	void __user *argp = (void __user *)arg;

	if (copy_from_user(&stBitBlt, argp, sizeof(TDE_BITBLIT_CMD_S)))
	{
			return -EFAULT;
	}
	pstBackGround = ((stBitBlt.u32NullIndicator >> 1) & 1) ? NULL : &stBitBlt.stBackGround;
	pstBackGroundRect = ((stBitBlt.u32NullIndicator >> 2) & 1) ? NULL : &stBitBlt.stBackGroundRect;
	pstForeGround = ((stBitBlt.u32NullIndicator >> 3) & 1) ? NULL : &stBitBlt.stForeGround;
	pstForeGroundRect = ((stBitBlt.u32NullIndicator >> 4) & 1) ? NULL : &stBitBlt.stForeGroundRect;
	pstOpt = ((stBitBlt.u32NullIndicator >> 7) & 1) ? NULL : &stBitBlt.stOpt;

	return TdeOsiBlit(stBitBlt.s32Handle, pstBackGround, pstBackGroundRect,
														pstForeGround, pstForeGroundRect, &stBitBlt.stDst, &stBitBlt.stDstRect,
														pstOpt);

}

static long tde_solid_draw(unsigned long arg)
{
	TDE_SOLIDDRAW_CMD_S stSolidDraw;
	TDE2_SURFACE_S *pstForeGround;
	TDE2_RECT_S *pstForeGroundRect;
	TDE2_FILLCOLOR_S *pstFillColor;
	TDE2_OPT_S *pstOpt;	
	void __user *argp = (void __user *)arg;

	if (copy_from_user(&stSolidDraw, argp, sizeof(TDE_SOLIDDRAW_CMD_S)))
	{
			return -EFAULT;
	}
	pstForeGround = ((stSolidDraw.u32NullIndicator >> 1) & 1) ? NULL : &stSolidDraw.stForeGround;
	pstForeGroundRect = ((stSolidDraw.u32NullIndicator >> 2) & 1) ? NULL : &stSolidDraw.stForeGroundRect;
	pstFillColor = ((stSolidDraw.u32NullIndicator >> 5) & 1) ? NULL : &stSolidDraw.stFillColor;
	pstOpt = ((stSolidDraw.u32NullIndicator >> 6) & 1) ? NULL : &stSolidDraw.stOpt;

	return TdeOsiSolidDraw(stSolidDraw.s32Handle, pstForeGround, pstForeGroundRect,
												 &stSolidDraw.stDst,
												 &stSolidDraw.stDstRect, pstFillColor, pstOpt);
}

static long tde_bitmap_maskrop(unsigned long arg)
{
		TDE_BITMAP_MASKROP_CMD_S stBmpMaskRop;
		void __user *argp = (void __user *)arg;
		
		if (copy_from_user(&stBmpMaskRop, argp, sizeof(TDE_BITMAP_MASKROP_CMD_S)))
		{
				return -EFAULT;
		}
		return TdeOsiBitmapMaskRop(stBmpMaskRop.s32Handle,
															 &stBmpMaskRop.stBackGround, &stBmpMaskRop.stBackGroundRect,
															 &stBmpMaskRop.stForeGround, &stBmpMaskRop.stForeGroundRect,
															 &stBmpMaskRop.stMask, &stBmpMaskRop.stMaskRect,
															 &stBmpMaskRop.stDst, &stBmpMaskRop.stDstRect,
															 stBmpMaskRop.enRopCode_Color, stBmpMaskRop.enRopCode_Alpha);
}

static long tde_pattern_fill(unsigned long arg)
{
		TDE_PATTERN_FILL_CMD_S stPatternFillCmd = { 0 };
		TDE2_SURFACE_S *pstBackGround;
		TDE2_RECT_S *pstBackGroundRect;
		TDE2_SURFACE_S *pstForeGround;
		TDE2_RECT_S *pstForeGroundRect;
		TDE2_SURFACE_S *pstDst;
		TDE2_RECT_S *pstDstRect;
		TDE2_PATTERN_FILL_OPT_S *pstOpt;
		void __user *argp = (void __user *)arg;

		if (copy_from_user(&stPatternFillCmd, argp, sizeof(TDE_PATTERN_FILL_CMD_S)))
		{
				return -EFAULT;
		}

		pstBackGround = ((stPatternFillCmd.u32NullIndicator >> 1) & 1) ? NULL : &stPatternFillCmd.stBackGround;
		pstBackGroundRect = ((stPatternFillCmd.u32NullIndicator >> 2) & 1) ? NULL : &stPatternFillCmd.stBackGroundRect;
		pstForeGround = ((stPatternFillCmd.u32NullIndicator >> 3) & 1) ? NULL : &stPatternFillCmd.stForeGround;
		pstForeGroundRect = ((stPatternFillCmd.u32NullIndicator >> 4) & 1) ? NULL : &stPatternFillCmd.stForeGroundRect;
		pstDst = ((stPatternFillCmd.u32NullIndicator >> 5) & 1) ? NULL : &stPatternFillCmd.stDst;
		pstDstRect = ((stPatternFillCmd.u32NullIndicator >> 6) & 1) ? NULL : &stPatternFillCmd.stDstRect;
		pstOpt = ((stPatternFillCmd.u32NullIndicator >> 7) & 1) ? NULL : &stPatternFillCmd.stOpt;
		return TdeOsiPatternFill(stPatternFillCmd.s32Handle, pstBackGround,
														 pstBackGroundRect, pstForeGround, pstForeGroundRect,
														 pstDst, pstDstRect, pstOpt);
}

static long tde_bitmap_maskblend(unsigned long arg)
{
		TDE_BITMAP_MASKBLEND_CMD_S stBmpMaskBlend;
		void __user *argp = (void __user *)arg;
		
		if (copy_from_user(&stBmpMaskBlend, argp, sizeof(TDE_BITMAP_MASKBLEND_CMD_S)))
		{
				return -EFAULT;
		}
		return TdeOsiBitmapMaskBlend(stBmpMaskBlend.s32Handle, &stBmpMaskBlend.stBackGround, &stBmpMaskBlend.stBackGroundRect,
																 &stBmpMaskBlend.stForeGround, &stBmpMaskBlend.stForeGroundRect, &stBmpMaskBlend.stMask, &stBmpMaskBlend.stMaskRect,
																 &stBmpMaskBlend.stDst, &stBmpMaskBlend.stDstRect, stBmpMaskBlend.u8Alpha, stBmpMaskBlend.enBlendMode);
}

static long tde_image_multiply(unsigned long arg)
{
		TDE_IMAGE_MULTIPLY_CMD_S stImageMultiply;
		void __user *argp = (void __user *)arg;
		
		if (copy_from_user(&stImageMultiply, argp, sizeof(TDE_IMAGE_MULTIPLY_CMD_S)))
		{
				return -EFAULT;
		}
		return TdeOsiImageMultiply(stImageMultiply.s32Handle, &stImageMultiply.stBackGround, &stImageMultiply.stBackGroundRect,
															 &stImageMultiply.stForeGround, &stImageMultiply.stForeGroundRect, &stImageMultiply.stPattern, &stImageMultiply.stPatternRect,
															 &stImageMultiply.stDst, &stImageMultiply.stDstRect, &stImageMultiply.opt);
}

static long tde_quick_deflicker(unsigned long arg)
{
		TDE_QUICKDEFLICKER_CMD_S stDeflicker;
		void __user *argp = (void __user *)arg;

		if (copy_from_user(&stDeflicker, argp, sizeof(TDE_QUICKDEFLICKER_CMD_S)))
		{
				return -EFAULT;
		}
		return TdeOsiQuickFlicker(stDeflicker.s32Handle, &stDeflicker.stSrc, &stDeflicker.stSrcRect, &stDeflicker.stDst,
															&stDeflicker.stDstRect);
}

static long tde_quick_copy(unsigned long arg)
{
    TDE_QUICKCOPY_CMD_S stQuickCopy;
		void __user *argp = (void __user *)arg;

    if (copy_from_user(&stQuickCopy, argp, sizeof(TDE_QUICKCOPY_CMD_S)))
    {
        return -EFAULT;
    }
    return TdeOsiQuickCopy(stQuickCopy.s32Handle, &stQuickCopy.stSrc, &stQuickCopy.stSrcRect, &stQuickCopy.stDst,
                           &stQuickCopy.stDstRect);
}


long tde_ioctl(struct file *ffile, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;

    TDE_LOG("%s:%d [%x]\n", __FUNCTION__, __LINE__, cmd);
    switch (cmd)
    {
    case TDE_WAIT_FINISH:
      return tde_wait_finish();
    case TDE_CF_WAIT_FINISH:
      return tde_wait_cf_finish();
    case TDE_CF_WAIT_SYNC_FINISH:
      return tde_wait_sync_cf_finish();

#ifndef CONFIG_TDE_PROC_DISABLE   
    case TDE_SET_PROC:
    {

			return tde_set_proc(arg);
    }
#endif    
    case TDE_BEGIN_JOB:
    {
        TDE_HANDLE s32Handle;
        mt_s32 ret;
        if ((ret = TdeOsiBeginJob(&s32Handle)) < 0)
        {
            return ret;
        }

        if (copy_to_user(argp, &s32Handle, sizeof(TDE_HANDLE)))
        {
            return -EFAULT;
        }
        return 0;
    }

    case TDE_BIT_BLIT:
    {
        return tde_bit_blit(arg);
    }

    case TDE_SOLID_DRAW:
    {
        return tde_solid_draw(arg);
    }

    case TDE_QUICK_DEFLICKER:
    {	
        return tde_quick_deflicker(arg);
    }

    case TDE_QUICK_COPY:
    {
        return tde_quick_copy(arg);
    }

    case TDE_QUICK_RESIZE:
    {
        TDE_QUICKRESIZE_CMD_S stQuickResize;

        if (copy_from_user(&stQuickResize, argp, sizeof(TDE_QUICKRESIZE_CMD_S)))
        {
            return -EFAULT;
        }
        return TdeOsiQuickResize(stQuickResize.s32Handle, &stQuickResize.stSrc, &stQuickResize.stSrcRect,
                                 &stQuickResize.stDst,
                                 &stQuickResize.stDstRect);
    }

    case TDE_QUICK_FILL:
    {
        TDE_QUICKFILL_CMD_S stQuickFill;

        if (copy_from_user(&stQuickFill, argp, sizeof(TDE_QUICKFILL_CMD_S)))
        {
            return -EFAULT;
        }
        return TdeOsiQuickFill(stQuickFill.s32Handle, &stQuickFill.stDst, &stQuickFill.stDstRect,
                               stQuickFill.u32FillData);
    }

    case TDE_END_JOB:
    {
        TDE_ENDJOB_CMD_S stEndJob;

        if (copy_from_user(&stEndJob, argp, sizeof(TDE_ENDJOB_CMD_S)))
        {
            return -EFAULT;
        }
        return TdeOsiEndJob(stEndJob.s32Handle, stEndJob.bBlock, stEndJob.u32TimeOut, stEndJob.bSync, NULL, NULL);
    }

    case TDE_MB_BITBLT:
    {
        TDE_MBBITBLT_CMD_S stMbBitblt;
        if (copy_from_user(&stMbBitblt, argp, sizeof(TDE_MBBITBLT_CMD_S)))
        {
            return -EFAULT;
        }
        return TdeOsiMbBlit(stMbBitblt.s32Handle, &stMbBitblt.stMB, &stMbBitblt.stMbRect, &stMbBitblt.stDst, &stMbBitblt.stDstRect, &stMbBitblt.stMbOpt);
    }

    case TDE_WAITFORDONE:
    {
        /* AI7D02876 beg set timeout value according by instruct number */
        TDE_HANDLE s32Handle;

        if (copy_from_user(&s32Handle, argp, sizeof(TDE_HANDLE)))
        {
            return -EFAULT;
        }
        return TdeOsiWaitForDone(s32Handle, TDE_MAX_WAIT_TIMEOUT);
        /* AI7D02876 end */
    }

    case TDE_WAITALLDONE:
    {
        return TdeOsiWaitAllDone(MT_FALSE);
    }

    case TDE_RESET:
    {
        TdeOsiReset();
        return MT_SUCCESS;
    }

    case TDE_CANCEL_JOB:
    {
        TDE_HANDLE s32Handle;

        if (copy_from_user(&s32Handle, argp, sizeof(TDE_HANDLE)))
        {
            return -EFAULT;
        }
        return TdeOsiCancelJob(s32Handle);
    }

    case TDE_BITMAP_MASKROP:
    {
    
        return tde_bitmap_maskrop(arg);
    }

    case TDE_BITMAP_MASKBLEND:
    {
       return tde_bitmap_maskblend(arg);
    }

    case TDE_SET_DEFLICKERLEVEL:
    {
        TDE_DEFLICKER_LEVEL_E eDeflickerLevel;
        if (copy_from_user(&eDeflickerLevel, argp, sizeof(TDE_DEFLICKER_LEVEL_E)))
        {
            return -EFAULT;
        }
        return TdeOsiSetDeflickerLevel(eDeflickerLevel);
    }

    case TDE_GET_DEFLICKERLEVEL:
    {
        TDE_DEFLICKER_LEVEL_E eDeflickerLevel;

        if (TdeOsiGetDeflickerLevel(&eDeflickerLevel) != MT_SUCCESS)
        {
            return MT_FAILURE;
        }

        if (copy_to_user(argp, &eDeflickerLevel, sizeof(TDE_DEFLICKER_LEVEL_E)))
        {
            return -EFAULT;
        }
        return MT_SUCCESS;
    }

    case TDE_SET_ALPHATHRESHOLD_VALUE:
    {
        mt_u8 u8ThresholdValue;

        if (copy_from_user(&u8ThresholdValue, argp, sizeof(mt_u8)))
        {
            return -EFAULT;
        }
        return TdeOsiSetAlphaThresholdValue(u8ThresholdValue);
    }

    case TDE_GET_ALPHATHRESHOLD_VALUE:
    {
        mt_u8 u8ThresholdValue;

        if (TdeOsiGetAlphaThresholdValue(&u8ThresholdValue))
        {
            return MT_FAILURE;
        }

        if (copy_to_user(argp, &u8ThresholdValue, sizeof(mt_u8)))
        {
            return -EFAULT;
        }
        return MT_SUCCESS;
    }

    case TDE_SET_ALPHATHRESHOLD_STATE:
    {
        MT_BOOL bEnAlphaThreshold;

        if (copy_from_user(&bEnAlphaThreshold, argp, sizeof(MT_BOOL)))
        {
            return -EFAULT;
        }
        return TdeOsiSetAlphaThresholdState(bEnAlphaThreshold);
    }

    case TDE_GET_ALPHATHRESHOLD_STATE:
    {
        MT_BOOL bEnAlphaThreshold;

        TdeOsiGetAlphaThresholdState(&bEnAlphaThreshold);

        if (copy_to_user(argp, &bEnAlphaThreshold, sizeof(MT_BOOL)))
        {
            return -EFAULT;
        }
        return MT_SUCCESS;
    }

    case TDE_PATTERN_FILL:
    {
        return tde_pattern_fill(arg);
    }

    case TDE_ENABLE_REGIONDEFLICKER:
    {
        MT_BOOL bRegionDeflicker;

        if (copy_from_user(&bRegionDeflicker, argp, sizeof(MT_BOOL)))
        {
            return -EFAULT;
        }
        return TdeOsiEnableRegionDeflicker(bRegionDeflicker);
    }

    case TDE_IMAGE_MULTIPLY:
    {
        return tde_image_multiply(arg);
    }

    default:
        return -ENOIOCTLCMD;
    }

    return 0;
}

#ifdef TDE_COREDUMP_DEBUG
extern volatile mt_u32 *s_pu32BaseVirAddr;

#define TDE_READ_REG(base, offset) \
    (*(volatile unsigned int *)((unsigned int)(base) + (offset)))
#endif

#ifdef USE_USER_SPACE_GPE
static int tde_osr_isr(int irq, void *dev_id)
{
    mt_u32 int_status = 0;    
    int_status = TdeHalCtlIntStats();
#ifdef CONFIG_MT_FPGA_GPE
   // printk("\r\n tde_osr_isr int_status:%x\n", int_status);
#endif
    if((int_status & (1 << 31)) == (1 << 31))
    {
      if(s_TdeFinish.tdeFinishFlag == 0)
      {
        s_TdeFinish.tdeFinishFlag = 1;
        wake_up_interruptible(&s_TdeFinish.tdeFinishEvent);        
      }
    }
    if((int_status & (1 << 8)) == (1 << 8))
    {
      if(s_TdeCfFinish.tdeFinishFlag == 0)
      {
        s_TdeCfFinish.tdeFinishFlag = 1;
        wake_up_interruptible(&s_TdeCfFinish.tdeFinishEvent);
      }
    }  

    if((int_status & (1 << 0)) == (1 << 0))
    {
      if(s_TdeCfSyncFinish.tdeFinishFlag == 0)
      {
        s_TdeCfSyncFinish.tdeFinishFlag = 1;
        wake_up_interruptible(&s_TdeCfSyncFinish.tdeFinishEvent);
      }
    }    
    TdeHalClearInt(int_status);
    return IRQ_HANDLED;
}
#else
STATIC int tde_osr_isr(int irq, void *dev_id)
{
    mt_u32 int_status = 0;
    //U_PERI_CRG37 unTempValue;
    //mt_u32 i;
    //unTempValue.u32 = g_pstRegCrg->PERI_CRG37.u32;

    TDE_FUN_IN;

#ifdef TDE_TIME_COUNT
    (mt_void) TDE_gettimeofday(&g_stTimeStart);
#endif

    int_status = TdeHalCtlIntStats();
#if 0
    /* AI7D02547 Interrupt handling while suspend to die */
    if(int_status & 0x80000000)
    {
#ifdef TDE_COREDUMP_DEBUG
        mt_u32 u32ReadStats = 0;
        for(i=0;i<74;i++)
        {
            u32ReadStats = TDE_READ_REG(s_pu32BaseVirAddr, (0x800 + i*4));
            printk("\n--------- ADDR:0x%x Value:0x%x---------\n",(0x800 + i*4),u32ReadStats);
        }
#endif
        unTempValue.bits.tde_srst_req = 0x1;
        g_pstRegCrg->PERI_CRG37.u32 = unTempValue.u32;
        for(i=0;i<100;i++)
        {
            ;
        }
        unTempValue.bits.tde_srst_req = 0x0;
        g_pstRegCrg->PERI_CRG37.u32 = unTempValue.u32;
        TDE_TRACE(TDE_KERN_ERR, "tde interrupts coredump!\n");
        TdeHalResumeInit();

        return IRQ_HANDLED;
    }
#endif

    TdeHalClearInt(int_status);

    TDE_TRACE(TDE_KERN_DEBUG, "tde_osr_isr status: 0x%x!\n", (mt_u32)int_status);

    /*
      if((int_status & 0x80000000) != 0x80000000)
      {
         TDE_FUN_OUT;
         return IRQ_HANDLED;
      }
      */

    tde_tasklet.data = tde_tasklet.data | ((MT_UL)int_status);

    tasklet_schedule(&tde_tasklet);

    TDE_FUN_OUT;
    return IRQ_HANDLED;
}
#endif
STATIC void tde_tasklet_func(struct tasklet_struct *int_status)
{
    mt_size_t lockflags;

    TDE_FUN_IN;
    TDE_LOCK(&s_taskletlock, lockflags);
    tde_tasklet.data &= (~(ulong)int_status);
    TDE_UNLOCK(&s_taskletlock, lockflags);

#ifdef TDE_TIME_COUNT
    (mt_void) TDE_gettimeofday(&g_stTimeEnd);

    g_u64TimeDiff = (g_stTimeEnd.tv_sec - g_stTimeStart.tv_sec) * 1000000 + (g_stTimeEnd.tv_usec - g_stTimeStart.tv_usec);
    TDE_TRACE(TDE_KERN_DEBUG, "tde int status: 0x%x, g_u64TimeDiff:%d!\n", (ulong)int_status, (mt_u32)g_u64TimeDiff);
#endif

    //if(int_status&TDE_DRV_INT_NODE_COMP_AQ)
    if ((ulong)int_status & 0x80000000)
    {
        //if(int_status & 0x100)  // async cmf fifo list is finished
        {
           TdeOsiListNodeComp();
        }
    }

    TDE_FUN_OUT;
}

#ifdef CONFIG_TDE_PM_ENABLE
/* tde wait for start  */
int tde_pm_suspend(basedev_s *pdev, pm_message_t state)
{
    TED_PRIV_DATA_S *tde_priv_data = dev_get_platdata(&pdev->dev);

    TDE_FUN_IN;
    TdeOsiWaitAllDone(MT_FALSE);

    TdeHalSuspend();

    MT_PRINT("TDE suspend OK\n");
    TDE_FUN_OUT;

    if (!IS_ERR_OR_NULL(tde_priv_data->gxaclk))
        clk_disable_unprepare(tde_priv_data->gxaclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->hdclk))
        clk_disable_unprepare(tde_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->sdclk_27m))
        clk_disable_unprepare(tde_priv_data->sdclk_27m);

    return 0;
}

/* wait for resume */
int tde_pm_resume(basedev_s *pdev)
{
    TED_PRIV_DATA_S *tde_priv_data = dev_get_platdata(&pdev->dev);

    if (!IS_ERR_OR_NULL(tde_priv_data->gxaclk))
        clk_prepare_enable(tde_priv_data->gxaclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->hdclk))
        clk_prepare_enable(tde_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(tde_priv_data->sdclk_27m))
        clk_prepare_enable(tde_priv_data->sdclk_27m);

    TDE_FUN_IN;
    TdeHalResumeInit();

    MT_PRINT("TDE resume OK\n");
    TDE_FUN_OUT;
    return 0;
}
#endif

/*****************************************************************************
 Prototype       : TdeOsiOpen
 Description     : open TDE equipment
 Input           : I_VOID  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/5/26
    Author       : wming
    Modification : Created function

*****************************************************************************/
mt_s32 TdeOsiOpen(mt_void)
{
    return tde_open(NULL, NULL);
}

/*****************************************************************************
 Prototype       : TdeOsiClose
 Description     : close TDE equipment
 Input           : I_VOID  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/5/26
    Author       : wming
    Modification : Created function

*****************************************************************************/
mt_s32 TdeOsiClose(mt_void)
{
    return tde_release(NULL, NULL);
}

#ifdef CONFIG_TDE_PM_ENABLE
EXPORT_SYMBOL(tde_pm_suspend);
EXPORT_SYMBOL(tde_pm_resume);
#endif
EXPORT_SYMBOL(TdeOsiOpen);
EXPORT_SYMBOL(TdeOsiClose);
EXPORT_SYMBOL(tde_ioctl);
EXPORT_SYMBOL(tde_open);
EXPORT_SYMBOL(tde_release);
EXPORT_SYMBOL(tde_init_module_k);
EXPORT_SYMBOL(tde_cleanup_module_k);
