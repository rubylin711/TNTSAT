/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/stddef.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <asm/atomic.h>
#include <asm/bitops.h>
//#include <asm/system.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <asm/pgtable.h>

#include "mt_mach/irq.h"

#include "mt_osal.h"
#include "mt_type.h"
#include "mt_jpeg_config.h"
//#include "mt_gfx_comm_k.h"
#include "mt_drv_jpeg_reg.h"
#include "mt_jpeg_hal_api.h"
#include "jpg_hal.h"
#include "jpg_suspend.h"

#include "mt_type.h"
#ifdef CONFIG_MT_CHIP_ARIA
#include <mach/aria_io.h>
#else
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
#include "../../dma/hal_dma_regs.h"
#endif
#ifdef CONFIG_JPEG_PROC_ENABLE
	#include "jpg_proc.h"
#endif

#ifdef CONFIG_JPEG_USE_SDK_CRG_ENABLE
#include "mt_reg_common.h"
#include "mt_drv_reg.h"
#endif

#include "drv_jpeg_ext.h"
#if (defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) //sym6)
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif


/***************************** Macro Definition ******************************/


/** module register name */
/** CNcomment:向SDK注册模块名 */
#define JPEGNAME                  "mt_jpeg_irq"
#define JPEGDEVNAME               "jpeg"

#if defined(CONFIG_MT_CHIP_ARIA)
#define JPGD_IRQ_NUM				      (44 + 32)
#define JPGD_REG_BASEADDR				  (0xffd40000)
#else
#define JPGD_IRQ_NUM				      IRQ_JPEG_ID
#define JPGD_REG_BASEADDR	        SYMPHONY_IO_PA(0xbf420000)
#define JPGD_REG_BASEPHYADDR	        (0xbf420000)

#endif

/*************************** Structure Definition ****************************/


/** jpeg device imformation */
/** CNcomment:jpeg设备信息 */
typedef struct mtJPG_OSRDEV_S
{

	MT_BOOL bSuspendSignal;      /**< whether get suspend signal  *//**<CNcomment:获取待机信号       */
	MT_BOOL bResumeSignal;	       /**< whether get resume signal   *//**<CNcomment:获取待机唤醒信号  */
	MT_BOOL bEngageFlag;          /**< whether be occupied, MT_TRUE if be occupied */
	MT_BOOL bDecTask;             /**< whether have jpeg dec task   *//**<CNcomment:是否有jpeg解码任务  */
    struct semaphore   SemGetDev; /**< protect the device to occupy the operation singnal */
    struct file        *pFile;
    JPG_INTTYPE_E      IntType;    /**< lately happened halt type  */
    wait_queue_head_t  QWaitInt;   /**< waite halt queue           */

}JPG_OSRDEV_S;



/** dispose close device */
/** CNcomment:关设备处理 */
typedef struct mtJPG_DISPOSE_CLOSE_S
{

     MT_S32 s32SuspendClose;
     MT_S32 s32DecClose;
     MT_BOOL bOpenUp;
     MT_BOOL bSuspendUp;
     MT_BOOL bRealse;

}JPG_DISPOSE_CLOSE_S;



/** private data info of device */
/** CNcomment: */
typedef struct mtJPG_PRIV_DATA_S
{
    struct clk *jpgclk;
}JPG_PRIV_DATA_S;


/********************** Global Variable declaration **************************/

extern MT_JPEG_PROC_INFO_S s_stJpeg6bProcInfo;

#ifndef CONFIG_JPEG_USE_SDK_CRG_ENABLE
static volatile MT_U32  *s_pJpegCRG     = MT_NULL;
#endif
static volatile MT_U32  *s_pJpegRegBase = MT_NULL;
static JPG_OSRDEV_S *s_pstruJpgOsrDev   = MT_NULL;

MT_GFX_DECLARE_MUTEX(s_JpegMutex);      /**< dec muxtex     *//**<CNcomment:解码多线程保护 */
MT_GFX_DECLARE_MUTEX(s_SuspendMutex);   /**< suspend muxtex *//**<CNcomment:待机多线程保护 */

/******************************* API forward declarations *******************/
static MT_S32 jpg_osr_open(struct inode *inode, struct file *file);
static MT_S32 jpg_osr_close( struct inode *inode, struct file *file);
static MT_S32 jpg_osr_mmap(struct file * filp, struct vm_area_struct *vma);
#ifndef CONFIG_GFX_BVT_SDK
static MT_S32 jpg_osr_suspend(basedev_s *pdev, pm_message_t state);
static MT_S32 jpg_osr_resume(basedev_s *pdev);
#endif
static long jpg_osr_ioctl(struct file *file, MT_U32 Cmd, unsigned long Arg);


//DECLARE_GFX_NODE(JPEGDEVNAME,jpg_osr_open, jpg_osr_close,jpg_osr_mmap,jpg_osr_ioctl,jpg_osr_suspend, jpg_osr_resume);

/******************************* API realization *****************************/


/***************************************************************************************
* func			: jpg_do_cancel_reset
* description	: cancel reset jpeg register
				  CNcomment: 测消复位 CNend\n
* param[in] 	: MT_VOID
* retval		: NA
* others:		: NA
***************************************************************************************/
static MT_VOID jpg_do_cancel_reset(MT_VOID)
{
#if defined(CONFIG_MT_CHIP_ARIA)

    int sys_ctrl_reg = mt_get_sys_ctrl_base() + 0x14;
    int dtmp = 0;

    dtmp = *((volatile unsigned int *)(sys_ctrl_reg));
    dtmp |= 0x7;
    *((volatile unsigned int *)(sys_ctrl_reg)) = dtmp;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6


#endif
}

#define ReadRegister(base, addr)		(*(volatile unsigned int *)(base + addr))
#define WriteRegister(base, addr, val) (*(volatile unsigned int *)(base + addr)) = (unsigned int)(val)

/***************************************************************************************
* func			: jpg_do_reset
* description	: reset jpeg register
				  CNcomment: 复位 CNend\n
* param[in] 	: MT_VOID
* retval		: NA
* others:		: NA
***************************************************************************************/

static MT_VOID jpg_do_reset(MT_VOID)
{
#if defined(CONFIG_MT_CHIP_ARIA)

  mt_u32 ptmp = 0;
  mt_u32 dtmp = 0;
  mt_u8 reg_id = 1;
  mt_u32 base = mt_get_sys_ctrl_base();

  ptmp = 0x14;
  //AXI
  dtmp = ReadRegister(base, ptmp);
  dtmp &= ~ (0x1 << 1);
  WriteRegister(base, ptmp, dtmp);
  //AHB
  dtmp = ReadRegister(base, ptmp);
  dtmp &= ~ (0x1 << 2);
  WriteRegister(base, ptmp, dtmp);
  //CORE
  dtmp = ReadRegister(base, ptmp);
  dtmp &= ~ (0x1 << 0);
  WriteRegister(base, ptmp, dtmp);

  mdelay(1); ;
  //CORE
  dtmp = ReadRegister(base, ptmp);
  dtmp |= ~ (0x1 << 0);
  WriteRegister(base, ptmp, dtmp);
  //AHB
  dtmp = ReadRegister(base, ptmp);
  dtmp |= ~ (0x1 << 2);
  WriteRegister(base, ptmp, dtmp);
  //AXI
  dtmp = ReadRegister(base, ptmp);
  dtmp |= ~ (0x1 << 1);
  WriteRegister(base, ptmp, dtmp);

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
  mt_u32 ptmp = 0;
  volatile mt_u32 dtmp = 0;
  mt_u8 reg_id = 1;
  mt_u32 base;
  u8 reset_bit = 16;

  base = R_RST_REQ(reg_id);
  dtmp = ReadRegister(base, ptmp);
  dtmp |= 0x7 << reset_bit;
  WriteRegister(base, 0, dtmp);

  base = R_RST_ALLOW(reg_id);
  do
  {
    dtmp = ReadRegister(base, ptmp);
  }
  while (((dtmp >> reset_bit) & 0x7) != 0x7);

  base = R_RST_CTRL(reg_id);
  //AXI
  dtmp = ReadRegister(base, ptmp);
  dtmp &= ~ (0x1 << (reset_bit + 2));
  WriteRegister(base, ptmp, dtmp);
  //AHB
  dtmp = ReadRegister(base, ptmp);
  dtmp &= ~ (0x1 << (reset_bit + 1));
  WriteRegister(base, ptmp, dtmp);
  //CORE
  dtmp = ReadRegister(base, ptmp);
  dtmp &= ~ (0x1 << (reset_bit + 0));
  WriteRegister(base, ptmp, dtmp);

  mdelay(1); ;
  //CORE
  dtmp = ReadRegister(base, ptmp);
  dtmp |= ~ (0x1 << (reset_bit + 0));
  WriteRegister(base, ptmp, dtmp);
  //AHB
  dtmp = ReadRegister(base, ptmp);
  dtmp |= ~ (0x1 << (reset_bit + 1));
  WriteRegister(base, ptmp, dtmp);
  //AXI
  dtmp = ReadRegister(base, ptmp);
  dtmp |= ~ (0x1 << (reset_bit + 2));
  WriteRegister(base, ptmp, dtmp);

  base = R_RST_REQ(reg_id);
  dtmp = ReadRegister(base, ptmp);
  dtmp &= ~ (0x7 << reset_bit);
  WriteRegister(base, ptmp, dtmp);
#elif defined(CONFIG_MT_CHIP_SYMPHONY4) 
  HAL_PUT_U32((volatile MT_U32 *)(mt_get_crm_base() + (0xa20c)), 0);
  mdelay(1);
  HAL_PUT_U32((volatile MT_U32 *)(mt_get_crm_base() + (0xa20c)), 0x7);  
  JPGDRV_WRITE_REG((ulong)s_pJpegRegBase, JCODEC_CLOCK_GATED, 0xf); //close clock gate

#elif defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
  mt_u32 dtmp = 0;
  dtmp = JPGDRV_READ_REG((ulong)s_pJpegRegBase, JCODEC_CLOCK_GATED);
  HAL_PUT_U32((volatile MT_U32 *)(mt_get_crm_base() + (0xa20c)), 0);
  mdelay(1);
  HAL_PUT_U32((volatile MT_U32 *)(mt_get_crm_base() + (0xa20c)), 0x7);  
  JPGDRV_WRITE_REG((ulong)s_pJpegRegBase, JCODEC_CLOCK_GATED, dtmp); //close clock gate
#endif
}


/***************************************************************************************
* func			: jpg_do_clock_off
* description	: close the jpeg clock
				  CNcomment: 关闭jpeg时钟 CNend\n
* param[in] 	: MT_VOID
* retval		: NA
* others:		: NA
***************************************************************************************/
static MT_VOID jpg_do_clock_off(MT_VOID)
{

}

/***************************************************************************************
* func			: jpg_do_clock_on
* description	: open the jpeg clock
				  CNcomment: 打开jpeg时钟 CNend\n
* param[in] 	: MT_VOID
* retval		: NA
* others:		: NA
***************************************************************************************/
static MT_VOID jpg_do_clock_on(MT_VOID)
{
	MT_U32 dtmp =0;
	dtmp = JPGDRV_READ_REG((ulong)s_pJpegRegBase, JCODEC_CLOCK_GATED);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) 
	dtmp |= 0xf;
#elif defined(CONFIG_MT_CHIP_SYMPHONY6) 
	dtmp = 0x11110000;
#endif
	JPGDRV_WRITE_REG((ulong)s_pJpegRegBase, JCODEC_CLOCK_GATED, dtmp); //close clock gate
}

 /***************************************************************************************
 * func 		 : jpg_select_clock_frep
 * description	 : select the clock frequence
				   CNcomment: jpeg时钟频率选择 CNend\n
 * param[in]	 : MT_VOID
 * retval		 : NA
 * others:		 : NA
 ***************************************************************************************/
#if 0
static MT_VOID jpg_select_clock_frep(MT_VOID)
 {

 }
#endif

#ifndef CONFIG_GFX_BVT_SDK
/***************************************************************************************
* func 		 : jpg_osr_suspend
* description: get the suspend signale.
			   CNcomment: 收到待机信号 CNend\n
* param[in]	 : *pdev
* param[in]	 : state
* retval	 : MT_SUCCESS 成功
* retval	 : MT_FAILURE 失败
* others:	 : NA
***************************************************************************************/
static MT_S32 jpg_osr_suspend(basedev_s *pdev, pm_message_t state)
{
    JPG_PRIV_DATA_S *jpg_priv_data = dev_get_platdata(&pdev->dev);

#ifdef CONFIG_JPEG_SUSPEND

	 MT_S32 Ret  = 0;

	 /** if you continue suspend and resume,tmts can be protected */
	 /** CNcomment:如果不停的待机唤醒，这里会起保护作用，始终使待机与唤醒配对操作 */
	 Ret  = down_interruptible(&s_SuspendMutex);
	 if(MT_TRUE == s_pstruJpgOsrDev->bDecTask)
	 {
		 JPG_WaitDecTaskDone();
		 /** tell the api received suspend signal */
		 /** CNcomment:通知应用层有待机信号了 */
		 s_pstruJpgOsrDev->bSuspendSignal = MT_TRUE;
		 #if defined(CONFIG_JPEG_FPGA_TEST_ENABLE) && defined(CONFIG_JPEG_FPGA_TEST_SUSPEND_ENABLE)
		 JPEG_TRACE("\n======================================\n");
		 JPEG_TRACE("=== %s 设置待机信号成功!\n",__FUNCTION__);
		 JPEG_TRACE("======================================\n");
		 #endif
	 }
#endif
     jpg_do_reset();
     jpg_do_clock_off();

    if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk))
        clk_disable_unprepare(jpg_priv_data->jpgclk);

	 return MT_SUCCESS;


}

/***************************************************************************************
* func 		 : jpg_osr_resume
* description: get the resume signale.
			   CNcomment: 收到待机唤醒信号 CNend\n
* param[in]	 : *pdev
* retval	 : MT_SUCCESS 成功
* retval	 : MT_FAILURE 失败
* others:	 : NA
***************************************************************************************/
static MT_S32 jpg_osr_resume(basedev_s *pdev)
{
    JPG_PRIV_DATA_S *jpg_priv_data = dev_get_platdata(&pdev->dev);

    if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk))
        clk_prepare_enable(jpg_priv_data->jpgclk);

#ifdef CONFIG_JPEG_SUSPEND

	 /** tell the api received resume signal */
	 /** CNcomment:通知应用层有待机唤醒信号了 */
	 if(MT_TRUE == s_pstruJpgOsrDev->bDecTask)
	 {
		 s_pstruJpgOsrDev->bResumeSignal  = MT_TRUE;
		 #if defined(CONFIG_JPEG_FPGA_TEST_ENABLE) && defined(CONFIG_JPEG_FPGA_TEST_SUSPEND_ENABLE)
		 JPEG_TRACE("\n======================================\n");
		 JPEG_TRACE("=== %s 设置待机唤醒信号成功!\n",__FUNCTION__);
		 JPEG_TRACE("======================================\n");
		 #endif
	 }
	 up(&s_SuspendMutex);
#endif

	 /** if suspend resume,the clock should open,if not open **/
	 /** when you read and write register,the system will no work**/
	 /** CNcomment:由于待机已经把时钟关闭了，要是唤醒的时候没有打开，则
      **           系统会挂死而无法正常工作 **/
	 jpg_do_clock_on();
     jpg_do_cancel_reset();

	 return MT_SUCCESS;


}
#endif

 /***************************************************************************************
 * func 		 : JpgOsrISR
 * description	 : the halt function
				   CNcomment: 中断响应函数 CNend\n
 * param[in]	 : irq
 * param[in]	 : * devId
 * param[in]	 : * ptrReg
 * retval		 : MT_SUCCESS 成功
 * retval		 : MT_FAILURE 失败
 * others:		 : NA
 ***************************************************************************************/
static irqreturn_t JpgOsrISR(MT_S32 irq, MT_VOID * devId)
{

        MT_U32 IntType = 0;
        MT_BOOL bWakeUp = MT_FALSE;

        /** get and set the halt status */
		/** CNcomment:获取当前的中断状态 */
        JpgHalGetIntStatus(&IntType);
		/** get and set the halt status */
		/** CNcomment:重新设置中断状态 */
        JpgHalSetIntStatus(IntType);

        if (IntType & 0x1)
        {
            s_pstruJpgOsrDev->IntType = JPG_INTTYPE_FINISH;
            bWakeUp = MT_TRUE;
        }
        if (IntType & 0x100)
        {
            //JPEG_TRACE("=== jpeg interrupt is err !\n");
            s_pstruJpgOsrDev->IntType |= JPG_INTTYPE_MARKER;
            bWakeUp = MT_TRUE;
        }
        if (IntType & 0x2)
        {
            s_pstruJpgOsrDev->IntType |= JPG_INTTYPE_CONTINUE;
            bWakeUp = MT_TRUE;
        }
        printk("JpgOsrISR : %x\n",IntType);
        if(bWakeUp == MT_TRUE)
            wake_up_interruptible(&s_pstruJpgOsrDev->QWaitInt);
        return IRQ_HANDLED;


}

/***************************************************************************************
* func			: Jpg_Request_irq
* description	: register the halt function
				  CNcomment: 根据中断号注册中断响应函数 CNend\n
* param[in] 	: MT_VOID
* retval		: NA
* others:		: NA
***************************************************************************************/
static MT_VOID Jpg_Request_irq(MT_VOID)
{

	    MT_S32 Ret = -1;
	    Ret = request_irq(JPGD_IRQ_NUM, (irq_handler_t)JpgOsrISR, IRQF_TRIGGER_HIGH, JPEGNAME, s_pstruJpgOsrDev);
	    if(MT_SUCCESS != Ret )
	    {
			MT_GFX_KFREE(MT_ID_JPGDEC, (MT_VOID *)s_pstruJpgOsrDev);
	        s_pstruJpgOsrDev = MT_NULL;
	    }
}


/***************************************************************************************
* func			: Jpg_Free_irq
* description	: free the halt
				  CNcomment: 销毁中断响应 CNend\n
* param[in] 	: MT_VOID
* retval		: NA
* others:		: NA
***************************************************************************************/
static MT_VOID Jpg_Free_irq(MT_VOID)
{
    free_irq(JPGD_IRQ_NUM, (MT_VOID *)s_pstruJpgOsrDev);
}


/***************************************************************************************
* func			: JpgOsrDeinit
* description	: when remout driver,call tmts deinit function
				  CNcomment: 卸载设备的时候去初始化 CNend\n
* param[in] 	: *pOsrDev
* retval		: NA
* others:		: NA
***************************************************************************************/
static MT_VOID JpgOsrDeinit(JPG_OSRDEV_S *pOsrDev)
{


	    /** use to initial waitqueue head and mutex */
		/** CNcomment:初始参数 */
	    pOsrDev->bEngageFlag = MT_FALSE;
	    pOsrDev->pFile       = MT_NULL;
	    pOsrDev->IntType     = JPG_INTTYPE_NONE;

	    /** initial the waiting halt waiting queue  */
		/** CNcomment: */
	    init_waitqueue_head(&pOsrDev->QWaitInt);

	    /** initial device occupy operation singnal */
		/** CNcomment: */
		MT_GFX_INIT_MUTEX(&pOsrDev->SemGetDev);
	    MT_GFX_INIT_MUTEX(&s_SuspendMutex);

        #ifdef CONFIG_JPEG_PROC_ENABLE
	    JPEG_Proc_Cleanup();
        #endif

	    /** unmap the register address and set s_u32JpgRegAddr with zero */
		/** CNcomment: */
	    JpgHalExit();

		#ifdef CONFIG_JPEG_SUSPEND
        JPG_SuspendExit();
		#endif

}

static JPEG_EXPORT_FUNC_S s_JpegExportFuncs =
{
	.pfnJpegSuspend		= jpg_osr_suspend,
	.pfnJpegResume		= jpg_osr_resume,
};

static mt_device_s g_JpgdRegisterData;
static JPG_PRIV_DATA_S jpg_priv_data_info;

static baseops_s jpgd_drvops =
{
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = jpg_osr_suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = jpg_osr_resume,
};


static struct file_operations JPGD_FOPS =
{
	owner: THIS_MODULE,
	open: jpg_osr_open,
	unlocked_ioctl: jpg_osr_ioctl,
	mmap: jpg_osr_mmap,
	release: jpg_osr_close,
};

 /***************************************************************************************
 * func 		 : JPEG_DRV_ModExit
 * description	 : remount the jpeg driver
				   CNcomment: 卸载设备 CNend\n
 * param[in]	 : *pOsrDev
 * retval		 : NA
 * others:		 : NA
 ***************************************************************************************/
static MT_VOID do_jpeg_drv_modexit(MT_VOID)
{


	    JPG_OSRDEV_S *pDev = s_pstruJpgOsrDev;

        if (!IS_ERR_OR_NULL(jpg_priv_data_info.jpgclk))
            clk_prepare_enable(jpg_priv_data_info.jpgclk);

	    /** unregister the jpeg from sdk */
		/** CNcomment: 将jpeg模块从SDK去除 */
		MT_GFX_MODULE_UnRegister(MT_ID_JPGDEC);

	    /** uninstall the device  */
		/** CNcomment: 卸载设备   */
		mt_drv_dev_unregister(&g_JpgdRegisterData);
	    /** free the halt  */
		/** CNcomment: 释放中断  */
        Jpg_Free_irq();

		jpg_do_clock_off();

	    JpgOsrDeinit(pDev);

		MT_GFX_KFREE(MT_ID_JPGDEC, (MT_VOID *)pDev);

	    s_pstruJpgOsrDev = MT_NULL;

#if (!defined(CONFIG_MT_CHIP_SYMPHONY4) && !defined(CONFIG_MT_CHIP_SYMPHONY6)) //sym6) 
        MT_GFX_REG_UNMAP((MT_VOID*)s_pJpegRegBase);
#endif
		s_pJpegRegBase  = NULL;

		#ifndef CONFIG_JPEG_USE_SDK_CRG_ENABLE
        MT_GFX_REG_UNMAP((MT_VOID*)s_pJpegCRG);
		s_pJpegCRG      = NULL;
		#endif

        if (!IS_ERR_OR_NULL(jpg_priv_data_info.jpgclk)) {
            clk_disable_unprepare(jpg_priv_data_info.jpgclk);
            clk_put(jpg_priv_data_info.jpgclk);
        }

	    return;


}

MT_VOID __exit JPEG_DRV_ModExit(MT_VOID)
{
	do_jpeg_drv_modexit();
}

static void jpeg_core_clk_high(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) 
        volatile mt_u32 value = 0;
        value = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA((ulong)0xBF50A204));
        value &= ~(0x3);//clear bit[1:0]
        value |= (0x1);//bit[1:0] set 1
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA((ulong)0xBF50A204), value);
        //printk("jpg clk high\n");
#endif
}

static void jpeg_core_clk_low(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    volatile mt_u32 value = 0;
    value = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA((ulong)0xbf50f818));
    value |= (0x1 << 8);//bit8 set 1
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA((ulong)0xbf50f818), value);
    
    value = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA((ulong)0xBF50A204));
    value &= ~(0x3);//clear bit[1:0]
    value |= (0x2);//bit[1:0] set 2
    HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA((ulong)0xBF50A204), value);
    //printk("jpg clk low\n");
#endif
}

/***************************************************************************************
* func			: JpgOsrInit
* description	: when insmod the driver call tmts function
				  CNcomment: 加载设备初始化 CNend\n
* param[in] 	: *pOsrDev
* retval		: MT_SUCCESS
* retval		: MT_FAILURE
* others:		: NA
***************************************************************************************/
static MT_S32 JpgOsrInit(JPG_OSRDEV_S *pOsrDev)
{


	    /** display the version message  */
		/** CNcomment: 显示版本号  */
//        MT_GFX_ShowVersionK(MT_ID_JPGDEC);

        #ifdef CONFIG_JPEG_PROC_ENABLE
		JPEG_Proc_init();
        #endif

        MT_GFX_INIT_MUTEX(&s_JpegMutex);

        /** trun the halt status  */
		/** CNcomment:   */
        JpgHalSetIntMask(0x0);

        /** request halt  */
		/** CNcomment:   */
         Jpg_Request_irq();

        /** use to initial waitqueue head and mutex */
		/** CNcomment:   */
        pOsrDev->bEngageFlag  = MT_FALSE;
        pOsrDev->pFile       = MT_NULL;
        pOsrDev->IntType     = JPG_INTTYPE_NONE;

        /** initial the waiting halt waiting queue */
		/** CNcomment:   */
        init_waitqueue_head(&pOsrDev->QWaitInt);

        /** initial device occupy operation singnal  */
		/** CNcomment:   */
	    MT_GFX_INIT_MUTEX(&pOsrDev->SemGetDev);
	    MT_GFX_INIT_MUTEX(&s_SuspendMutex);

        jpeg_core_clk_low();
        
        return MT_SUCCESS;


}


/***************************************************************************************
* func			: JPEG_DRV_ModInit
* description	: when insmod the driver call tmts function
				  CNcomment: 加载设备初始化 CNend\n
* param[in] 	: NA
* retval		: MT_SUCCESS
* retval		: MT_FAILURE
* others:		: NA
***************************************************************************************/
MT_S32 __init JPEG_DRV_ModInit(MT_VOID)
{


        MT_S32 Ret = MT_FAILURE;
        MT_U32 dtmp =0;
#if !defined(CONFIG_MT_CHIP_SYMPHONY4) && !defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
		ulong u64BaseAddr = JPGD_REG_BASEADDR;
#endif
        jpg_priv_data_info.jpgclk = clk_get(NULL, "jpgclk");
        if (!IS_ERR_OR_NULL(jpg_priv_data_info.jpgclk))
            clk_prepare_enable(jpg_priv_data_info.jpgclk);

		/** if operation, return failure -EBUSY  */
		/** CNcomment:   */
        if (MT_NULL != s_pstruJpgOsrDev)
        {
            return -EBUSY;
        }

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
      s_pJpegRegBase = (volatile MT_U32*)SYMPHONY_JPEG_BASE;
#else
	    s_pJpegRegBase = (volatile MT_U32*)MT_GFX_REG_MAP(u64BaseAddr, JPGD_REG_LENGTH);
#endif
		/** cancle the reset,now can work  */
		/** CNcomment: 撤消复位使之能够工作 */
		jpg_do_cancel_reset();

        /** malloc and initial the struct that drive needed to s_pstruJpgOsrDev,if malloc failure, return -NOMEM  */
		/** CNcomment:  */
        s_pstruJpgOsrDev = (JPG_OSRDEV_S *)MT_GFX_KMALLOC(MT_ID_JPGDEC,sizeof(JPG_OSRDEV_S),GFP_KERNEL);
        if ( MT_NULL == s_pstruJpgOsrDev )
        {
            return -ENOMEM;
        }
        memset(s_pstruJpgOsrDev, 0x0, sizeof(JPG_OSRDEV_S));

        JpgHalInit((ulong)s_pJpegRegBase);
		#ifdef CONFIG_JPEG_SUSPEND
        JPG_SuspendInit((ulong)s_pJpegRegBase);
		#endif

       /** call JpgOsrInit to initial OSR modual, if failure should release the
        ** resource and return failure
        **/

        Ret = JpgOsrInit(s_pstruJpgOsrDev);
        if (MT_SUCCESS != Ret)
        {
		   MT_GFX_KFREE(MT_ID_JPGDEC, (MT_VOID *)s_pstruJpgOsrDev);
           s_pstruJpgOsrDev = MT_NULL;
           return Ret;
        }
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
       JPGDRV_WRITE_REG((ulong)s_pJpegRegBase, JCODEC_CLOCK_GATED, 0xf); //close clock gate
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
           dtmp = JPGDRV_READ_REG((ulong)s_pJpegRegBase, JCODEC_CLOCK_GATED);
          // dtmp |= 0xf;
            dtmp = 0x11110000;
           JPGDRV_WRITE_REG((ulong)s_pJpegRegBase, JCODEC_CLOCK_GATED, dtmp); //close clock gate
            dtmp = JPGDRV_READ_REG((ulong)s_pJpegRegBase, JCODEC_CLOCK_GATED);
#endif

    mt_osal_snprintf(g_JpgdRegisterData.devfs_name, sizeof(g_JpgdRegisterData.devfs_name), UMAP_DEVNAME_JPGD);
    g_JpgdRegisterData.minor  = UMAP_MIN_MINOR_JPGD;
    g_JpgdRegisterData.owner  = THIS_MODULE;
    g_JpgdRegisterData.drvops = &jpgd_drvops;
    g_JpgdRegisterData.fops = &JPGD_FOPS;
	g_JpgdRegisterData.priv = (void *)&jpg_priv_data_info;

        if (mt_drv_dev_register(&g_JpgdRegisterData) < 0)
        {
			MT_GFX_KFREE(MT_ID_JPGDEC, (MT_VOID *)s_pstruJpgOsrDev);
            s_pstruJpgOsrDev = MT_NULL;
            return MT_FAILURE;
        }

		Ret = MT_GFX_MODULE_Register(MT_ID_JPGDEC, JPEGDEVNAME, &s_JpegExportFuncs);
        if(MT_SUCCESS != Ret)
        {
            do_jpeg_drv_modexit();
	        return MT_FAILURE;
        }

        if (!IS_ERR_OR_NULL(jpg_priv_data_info.jpgclk))
            clk_disable_unprepare(jpg_priv_data_info.jpgclk);

        return MT_SUCCESS;


}


/***************************************************************************************
* func			: jpg_osr_open
* description	: open jpeg device
				  CNcomment: 打开jpeg设备 CNend\n
* param[in] 	: *inode
* param[in] 	: *file
* retval		: MT_SUCCESS
* retval		: MT_FAILURE
* others:		: NA
***************************************************************************************/
static MT_S32 jpg_osr_open(struct inode *inode, struct file *file)
{
        MT_S32 mt_idx = iminor(inode);
        JPG_PRIV_DATA_S *jpg_priv_data = get_mt_priv(mt_idx);
	    JPG_DISPOSE_CLOSE_S *sDisposeClose = NULL;

        if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk)) {
            clk_prepare_enable(jpg_priv_data->jpgclk);
        }

	    sDisposeClose = (JPG_DISPOSE_CLOSE_S *)MT_GFX_KMALLOC(MT_ID_JPGDEC,           \
			                                                   sizeof(JPG_DISPOSE_CLOSE_S),\
			                                                   GFP_KERNEL);
		if ( MT_NULL == sDisposeClose )
	    {
	        return -ENOMEM;
	    }

	    memset(sDisposeClose, 0x0, sizeof(JPG_DISPOSE_CLOSE_S));
	    file->private_data             = sDisposeClose;
	    sDisposeClose->s32DecClose     = MT_SUCCESS;
	    sDisposeClose->s32SuspendClose = MT_FAILURE;
	    sDisposeClose->bOpenUp         = MT_FALSE;
	    sDisposeClose->bSuspendUp      = MT_FALSE;
	    sDisposeClose->bRealse         = MT_FALSE;


        jpeg_core_clk_high();
	    return MT_SUCCESS;

}

 /***************************************************************************************
 * func 		 : jpg_osr_close
 * description	 : close jpeg device
				   CNcomment: 关闭jpeg设备 CNend\n
 * param[in]	 : *inode
 * param[in]	 : *file
 * retval		 : MT_SUCCESS
 * retval		 : MT_FAILURE
 * others:		 : NA
 ***************************************************************************************/
static MT_S32 jpg_osr_close( struct inode *inode, struct file *file)
{
        MT_S32 mt_idx = iminor(inode);
        JPG_PRIV_DATA_S *jpg_priv_data = get_mt_priv(mt_idx);
        JPG_DISPOSE_CLOSE_S *sDisposeClose = NULL;

        sDisposeClose = file->private_data;
        if(NULL == sDisposeClose)
        {
           if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk))
               clk_disable_unprepare(jpg_priv_data->jpgclk);
           return MT_FAILURE;
        }

		/**
		 **if device has not initial, return failure
		 **/
		if (MT_NULL == s_pstruJpgOsrDev)
		{
			up(&s_JpegMutex);
            if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk))
                clk_disable_unprepare(jpg_priv_data->jpgclk);
			return MT_FAILURE;
		}
        jpeg_core_clk_low();
        /**
	     **解码任务完成
	     **/
		 s_pstruJpgOsrDev->bDecTask = MT_FALSE;
		 s_pstruJpgOsrDev->bSuspendSignal = MT_FALSE;
		 s_pstruJpgOsrDev->bResumeSignal  = MT_FALSE;

        /** if suspend dispose */
		/** CNcomment: 如果是待机则将待机需要的设备关回掉即可 */
        if(MT_SUCCESS==sDisposeClose->s32SuspendClose)
		{
             if(MT_TRUE == sDisposeClose->bSuspendUp)
			 {
                up(&s_JpegMutex);
             }
			 MT_GFX_KFREE(MT_ID_JPGDEC, (MT_VOID *)sDisposeClose);
             if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk))
                 clk_disable_unprepare(jpg_priv_data->jpgclk);
             return MT_SUCCESS;
        }

        if(MT_SUCCESS==sDisposeClose->s32DecClose)
		{
             if(MT_TRUE == sDisposeClose->bOpenUp)
			 {
                up(&s_JpegMutex);
             }
			 MT_GFX_KFREE(MT_ID_JPGDEC, (MT_VOID *)sDisposeClose);
             if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk))
                 clk_disable_unprepare(jpg_priv_data->jpgclk);
             return MT_SUCCESS;
        }


        /**
         **  if call realse, should not call tmts
         **/
        if(MT_FALSE == sDisposeClose->bRealse)
        {
            /**
             ** set file private data to MT_NULL
             **/
            MT_GFX_KFREE(MT_ID_JPGDEC, (MT_VOID *)sDisposeClose);

            /**
             ** if the file occupy the device, set tmts device to not occupied,
             ** wake up waiting halt waiting queue
             **/
            if(down_interruptible(&s_pstruJpgOsrDev->SemGetDev))
            {
              /*notmtng to do!*/
            }

            if ((MT_TRUE == s_pstruJpgOsrDev->bEngageFlag) && (file == s_pstruJpgOsrDev->pFile))
            {

                s_pstruJpgOsrDev->bEngageFlag = MT_FALSE;
                (MT_VOID)wake_up_interruptible(&s_pstruJpgOsrDev->QWaitInt);

            }
            /**
             ** to JPG reset operation, open the clock
             **/
            if(s_pstruJpgOsrDev->bEngageFlag != MT_FALSE)
			{
				jpg_do_cancel_reset();
                up(&s_pstruJpgOsrDev->SemGetDev);
                up(&s_JpegMutex);
                if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk))
                    clk_disable_unprepare(jpg_priv_data->jpgclk);
        		return MT_FAILURE;
        	}
            if(s_pstruJpgOsrDev->IntType != JPG_INTTYPE_NONE)
			{
				jpg_do_cancel_reset();
                up(&s_pstruJpgOsrDev->SemGetDev);
                up(&s_JpegMutex);
                if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk))
                    clk_disable_unprepare(jpg_priv_data->jpgclk);
        		return MT_FAILURE;
            }

			jpg_do_cancel_reset();

            up(&s_JpegMutex);

            up(&s_pstruJpgOsrDev->SemGetDev);

            if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk))
                clk_disable_unprepare(jpg_priv_data->jpgclk);

            return MT_SUCCESS;


        }

        /**
         ** set file private data to MT_NULL
         **/
		MT_GFX_KFREE(MT_ID_JPGDEC, (MT_VOID *)sDisposeClose);

        if (!IS_ERR_OR_NULL(jpg_priv_data->jpgclk))
            clk_disable_unprepare(jpg_priv_data->jpgclk);

        return MT_SUCCESS;


}


 /***************************************************************************************
 * func 		 : jpg_osr_mmap
 * description	 : mmap jpeg device
				   CNcomment: 映射jpeg设备 CNend\n
 * param[in]	 : *filp
 * param[in]	 : *vma
 * retval		 : MT_SUCCESS
 * retval		 : MT_FAILURE
 * others:		 : NA
 ***************************************************************************************/
static MT_S32 jpg_osr_mmap(struct file * filp, struct vm_area_struct *vma)
{

		/** if api call mmap,will call tmts function */
		/** CNcomment: 上层map jpeg设备的时候调用 */
        unsigned long Phys = 0;
        phys_addr_t phyBaseAddr = 0;
        
        phyBaseAddr = JPGD_REG_BASEPHYADDR;
        /**
         ** set map parameter
         **/
	  Phys = (phyBaseAddr >> PAGE_SHIFT);
        vm_flags_set(vma, VM_LOCKED | VM_IO);

        /** cancel map **/
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

        if (remap_pfn_range(vma, vma->vm_start, Phys, vma->vm_end - vma->vm_start,
                            vma->vm_page_prot))
        {
            return -EAGAIN;
        }

        return MT_SUCCESS;


}

 /*****************************************************************************
* func            : jpg_osr_ioctl
* description     : jpeg device control interface
* param[in]       : inode
* param[in]       : flip    device file message
* param[in]       : Cmd
* param[in]       : Arg
* output          : none
* retval          : MT_SUCCESS
* retval          : MT_ERR_JPG_DEC_BUSY
* retval          : -EINVAL
* retval          : -EAGAIN
* others:	      : notmtng
*****************************************************************************/
static long jpg_osr_ioctl(struct file *file, MT_U32 Cmd, unsigned long Arg)
{



        MT_U32 u32StartTimeMs = 0; /** ms **/
		MT_U32 u32EndTimeMs   = 0; /** ms **/
        MT_S32 IRQ_NUM         = JPGD_IRQ_NUM;
        #ifdef CONFIG_JPEG_SUSPEND
        MT_JPG_SAVEINFO_S stSaveInfo = {0};
		#endif
	    switch(Cmd)
	    {

	        case CMD_JPG_GETDEVICE:
	        {

	            JPG_DISPOSE_CLOSE_S *sDisposeClose = NULL;
	            sDisposeClose = file->private_data;

	        	/********if jpeg has not close, so jpeg is busy, you should suspend now **/
	        	if(down_interruptible(&s_JpegMutex)){ /** Mutex initial with 1, and after tmts func,the
	        	                             ** Mutex is zero, so has not mutex, next time should
	        	                             ** wait here, only the mutex is no zero, followed can
	        	                             ** operation
	        	                             **/
	        	      sDisposeClose->bOpenUp = MT_FALSE;
	                  return -ERESTARTSYS;
	            }
	        	/*************************************************************************/

	            /**
	             ** if has not initial device, return failure
	             **/
	            if (MT_NULL == s_pstruJpgOsrDev)
	            {
	                return MT_FAILURE;
	            }

	            /**
	             ** locked the occupied device
	             **/
	            if(down_interruptible(&s_pstruJpgOsrDev->SemGetDev))
	            {
	               /*notmtng to do!*/
	            }

	            s_pstruJpgOsrDev->bEngageFlag = MT_TRUE;
	            s_pstruJpgOsrDev->IntType    = JPG_INTTYPE_NONE;
	            s_pstruJpgOsrDev->pFile      = file;

	            sDisposeClose->s32DecClose   = MT_FAILURE;
	            sDisposeClose->bOpenUp       = MT_TRUE;
	            sDisposeClose->bRealse       = MT_FALSE;
	            /**
	             ** to JPG reset operation, open the clock
	             **/
				 jpg_do_reset();

	             up(&s_pstruJpgOsrDev->SemGetDev);

                 /**
                  **开始解码任务
                  **/
				 s_pstruJpgOsrDev->bDecTask = MT_TRUE;

	             break;

	        }
	        case CMD_JPG_RELEASEDEVICE:
	        {

	            JPG_DISPOSE_CLOSE_S *sDisposeClose = NULL;
	            sDisposeClose = file->private_data;
	            /**
	             **if device has not initial, return failure
	             **/
	            if (MT_NULL == s_pstruJpgOsrDev)
	            {
	                up(&s_JpegMutex);
	                return MT_FAILURE;
	            }
	            /**
	             ** if the file occupy the device, set tmts device to not occupied,
	             ** wake up waiting halt waiting queue
	             **/
	            if(down_interruptible(&s_pstruJpgOsrDev->SemGetDev))
	            {
				   /*notmtng to do!*/
				}

	            if ((MT_TRUE == s_pstruJpgOsrDev->bEngageFlag) && (file == s_pstruJpgOsrDev->pFile))
	            {

	                s_pstruJpgOsrDev->bEngageFlag = MT_FALSE;
	                (MT_VOID)wake_up_interruptible(&s_pstruJpgOsrDev->QWaitInt);

	            }

	            /**
	             ** to JPG reset operation, open the clock
	             **/
	            if(s_pstruJpgOsrDev->bEngageFlag != MT_FALSE)
				{
	                up(&s_pstruJpgOsrDev->SemGetDev);
	                up(&s_JpegMutex);
	        		return MT_FAILURE;
	        	}
	            if(s_pstruJpgOsrDev->IntType != JPG_INTTYPE_NONE)
				{
	                up(&s_pstruJpgOsrDev->SemGetDev);
	                up(&s_JpegMutex);
	        		return MT_FAILURE;
	            }

				jpg_do_cancel_reset();

	            up(&s_JpegMutex);
	            sDisposeClose->bRealse = MT_TRUE;

			   /**
				**解码任务结束
				**/
				s_pstruJpgOsrDev->bDecTask = MT_FALSE;

	            up(&s_pstruJpgOsrDev->SemGetDev);

	            break;

	        }
	        case CMD_JPG_SUSPEND:
	        {
				 #ifdef CONFIG_JPEG_SUSPEND
                	pm_message_t state = {0};
				 	jpg_osr_suspend(NULL,state);
				 #endif
	             break;
	        }
	        case CMD_JPG_RESUME:
	        {
				 #ifdef CONFIG_JPEG_SUSPEND
	             	jpg_osr_resume(NULL);
				 #endif
	             break;
	        }
	        case CMD_JPG_GETINTSTATUS:
	        {


	            JPG_GETINTTYPE_S IntInfo;
	            MT_S32 Ret = 0;
	            MT_S32 loop = 0;
	            MT_U32 FirstCount = 1;
	            /**
	             ** checkt parameter
	             **/
	            if (0 == Arg)
	            {
	                return MT_FAILURE;
	            }

	            /**
	             ** copy input parameter
	             **/
	           if(copy_from_user((MT_VOID *)&IntInfo, (MT_VOID *)Arg, sizeof(JPG_GETINTTYPE_S)))
			   {
	                return -EFAULT;
	           	}

	            disable_irq(IRQ_NUM);

	           /**
	            ** get the halt type
	            **/
	            if (    (JPG_INTTYPE_NONE != s_pstruJpgOsrDev->IntType)
	                 || (0 == IntInfo.TimeOut))
	            {


	                IntInfo.IntType = s_pstruJpgOsrDev->IntType;
	                s_pstruJpgOsrDev->IntType = JPG_INTTYPE_NONE;
	                enable_irq(IRQ_NUM);

	                if(copy_to_user((MT_VOID *)Arg, (MT_VOID *)&IntInfo, sizeof(JPG_GETINTTYPE_S)))
	  		        {
	                    return -EFAULT;
	           	    }
	                break;
	            }
	            enable_irq(IRQ_NUM);

	            do
	            {
	               /**
	                ** if the value of overtime, to overtime waitiong
	                **/
	                #if 0 /** CONFIG_MT_FPGA_JPEG_VERSION **/
					/** FPGA test hard function, no set overtime **/
					Ret = wait_event_interruptible(s_pstruJpgOsrDev->QWaitInt,JPG_INTTYPE_NONE != s_pstruJpgOsrDev->IntType);
					#else
	                Ret = wait_event_interruptible_hrtimeout(s_pstruJpgOsrDev->QWaitInt,
	                              JPG_INTTYPE_NONE != s_pstruJpgOsrDev->IntType,
	                              ms_to_ktime(IntInfo.TimeOut));
				    #endif

	                loop = 0;

	                if(Ret > 0 || (JPG_INTTYPE_NONE != s_pstruJpgOsrDev->IntType))
	                {

	                    disable_irq(IRQ_NUM);
	                    IntInfo.IntType = s_pstruJpgOsrDev->IntType;
	                    s_pstruJpgOsrDev->IntType = JPG_INTTYPE_NONE;
	                    enable_irq(IRQ_NUM);
	                    break;
	                }
	                else if( -ERESTARTSYS == Ret)
	                {

	                    if(FirstCount)
	                    {
							MT_GFX_GetTimeStamp(&u32StartTimeMs,NULL);
	                        FirstCount = 0;
	                        loop = 1;
	                    }
	                    else
	                    {
							MT_GFX_GetTimeStamp(&u32EndTimeMs,NULL);
	                        /** avoid dead lock **/
                            loop = ((u32EndTimeMs - u32StartTimeMs) <  IntInfo.TimeOut)?1:0;
	                        /** check timeout **/
							if(!loop)
	                        {
	                        	 return MT_FAILURE;
	                        }
	                    }

	                }
	                else /** == 0(wait timeout) and others **/
	                {
	                    return MT_FAILURE;
	                }

	            }while(loop);

	            /**
	             ** get halt status and return
	             **/
	            if(copy_to_user((MT_VOID *)Arg, (MT_VOID *)&IntInfo,sizeof(JPG_GETINTTYPE_S)))
			    {
	                return -EFAULT;
	           	}

	            break;
	        }
	        case CMD_JPG_READPROC:
	        {

                 #ifdef CONFIG_JPEG_PROC_ENABLE
		            MT_BOOL bIsProcOn = MT_FALSE;
					JPEG_Get_Proc_Status(&bIsProcOn);
		            if(MT_TRUE == bIsProcOn)
		            {
			            if (0 == Arg)
			            {
			                return MT_FAILURE;
			            }

			            if(copy_from_user((MT_VOID *)&s_stJpeg6bProcInfo, (MT_VOID *)Arg, sizeof(MT_JPEG_PROC_INFO_S)))
					    {
			                return -EFAULT;
			           	}
		            }
				#endif

	            break;

	        }
	        case CMD_JPG_GETRESUMEVALUE:
	        {/** 获取待机唤醒信息 **/
				#ifdef CONFIG_JPEG_SUSPEND
	            if (0 == Arg)
	            {
	                return MT_FAILURE;
	            }
	            JPG_GetResumeValue(&stSaveInfo);
	            if(copy_to_user((MT_VOID *)Arg, (MT_VOID *)&stSaveInfo,sizeof(stSaveInfo)))
			    {
	                return -EFAULT;
	           	}
				s_pstruJpgOsrDev->bSuspendSignal = MT_FALSE;
    			s_pstruJpgOsrDev->bResumeSignal  = MT_FALSE;
				#endif
	            break;

	        }
	        case CMD_JPG_GETSUSPEND:
	        { /** 获取待机信息 **/

                #ifdef CONFIG_JPEG_SUSPEND
	            if (0 == Arg)
	            {
	                return MT_FAILURE;
	            }
	            if(copy_to_user((MT_VOID *)Arg, (MT_VOID *)&s_pstruJpgOsrDev->bSuspendSignal,sizeof(s_pstruJpgOsrDev->bSuspendSignal)))
			    {
	                return -EFAULT;
	           	}
				#endif
	            break;

	        }
	        case CMD_JPG_GETRESUME:
	        {/** 获取待机唤醒信息 **/
				#ifdef CONFIG_JPEG_SUSPEND
	            if (0 == Arg)
	            {
	                return MT_FAILURE;
	            }

	            if(copy_to_user((MT_VOID *)Arg, (MT_VOID *)&s_pstruJpgOsrDev->bResumeSignal,sizeof(s_pstruJpgOsrDev->bResumeSignal)))
			    {
	                return -EFAULT;
	           	}
				#endif
	            break;

	        }
            case CMD_JPG_CANCEL_RESET:
			{
                 jpg_do_cancel_reset();
				 break;
            }
            case CMD_JPG_RESET:
			{
                 jpg_do_reset();
				 break;
            }
	        default:
	        {
	            return -EINVAL;
	        }

	    }

	    return MT_SUCCESS;


}

#ifdef __cplusplus
    #if __cplusplus
}
    #endif  /* __cplusplus */
#endif  /* __cplusplus */
