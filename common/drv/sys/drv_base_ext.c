/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include "mt_type.h"
#include "mt_drv_log.h"
#include "mt_drv_struct.h"
#include "mt_drv_sys.h"
#include "drv_log.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "drv_sys_ioctl.h"
#include "drv_stat_ioctl.h"
#include "mt_drv_module.h"
#include "drv_base_ext_k.h"
#include "mt_drv_mmz.h"
#include "mt_drv_memdev.h"
#include "mt_drv_userproc.h"
#include "mt_drv_dev.h"
#include "mt_drv_log.h"
#include "mt_debug.h"
#include "mt_cache.h"
#include "mt_ftrace.h"

/* Use "strings mt_xx.ko | grep "SDK_VERSION"" to get the version */
/*MTCHAR g_ModuleVersion[160] ="SDK_VERSION:["\
    MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
    __DATE__", "__TIME__"]";
*/





#ifdef CMN_TEST_SUPPORTED
extern mt_void MT_DRV_TEST_Init(mt_void);
extern mt_void MT_DRV_TEST_Exit(mt_void);
#endif

extern  void __init mt_drv_env_init(mt_void);

static int __init common_drv_modinit(void)
{
    mt_s32 ret;

	MT_INFO_LOG("====common_drv_modinit=======\n");

    mt_dcache_line_size_init();

#if 1//run before init
    mt_drv_env_init();
#if 0
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("DRV_PM_ModInit failed:%#x!\n", ret);
        return MT_FAILURE;
    }
#endif
#endif
    ret = drv_mmz_modinit();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("DRV_MMZ_ModInit failed:%#x!\n", ret);
        goto ErrorExit_MMZ;
    }

#if !defined (MT_MCE_SUPPORT) && !defined(MT_KEYLED_CT1642_KERNEL_SUPPORT)
    ret = mt_drv_commoninit();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("MT_DRV_CommonInit failed:%#x!\n", ret);
        goto ErrorExit_Common;
    }
#endif
    ret = mt_drv_proc_init();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("MT_DRV_PROC_Init failed:%#x!\n", ret);
        goto ErrorExit_PROC;
    }

#if (defined(MT_LOG_SUPPORT) && (0 == MT_LOG_SUPPORT))
#else
    ret = mt_drv_log_init();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("MT_DRV_LOG_Init failed:%#x!\n", ret);
        goto ErrorExit_LOG;
    }
#endif

    ret = mt_drv_sys_init();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("MT_DRV_SYS_Init failed:%#x!\n", ret);
        goto ErrorExit_SYS;
    }
    ret = mt_drv_stat_init();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("MT_DRV_STAT_Init failed:%#x!\n", ret);
        goto ErrorExit_STAT;
    }
    ret = mmngr_drv_modinit(MT_KMODULE_MAX_COUNT, MT_KMODULE_MEM_MAX_COUNT);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_SYS("KModuleMgr_Init failed:%#x!\n", ret);
        goto ErrorExit_Module;
    }

    ret = memdev_drv_modinit();
    if (MT_SUCCESS != ret)
    {
        MT_ERR_SYS("memdev init failed:%#x!\n", ret);
        goto ErrorExit_MEMDEV;
    }

	ret = mt_proc_drv_mod_init();
	if (MT_SUCCESS != ret) {
		MT_ERR_SYS("proc drv mod init failed:%#x!\n", ret);
		goto ErrorExit_USRPROC;
	}

#ifdef CMN_TEST_SUPPORTED
    //MT_DRV_TEST_Init();
#endif

    return MT_SUCCESS;

ErrorExit_USRPROC:
    memdev_drv_modexit();

ErrorExit_MEMDEV:
    mmngr_drv_modexit();

ErrorExit_Module:

    mt_drv_stat_exit();

ErrorExit_STAT:

    mt_drv_sys_exit();

ErrorExit_SYS:

    mt_drv_log_exit();

ErrorExit_LOG:

    mt_drv_proc_exit();

ErrorExit_PROC:

#if !defined (MT_MCE_SUPPORT) && !defined(KEYLED_CT1642_KERNEL_SUPPORT)
    mt_drv_commonexit();

ErrorExit_Common:
#endif

    drv_mmz_modexit();

ErrorExit_MMZ:
    //drv_pm_modexit();

    return MT_FAILURE;
}

static mt_void common_drv_modexit (mt_void)
{
#ifdef CMN_TEST_SUPPORTED
    //MT_DRV_TEST_Exit();
#endif
	MT_INFO_LOG("====common_drv_modexit=====OOO=======\n");
	mt_proc_drv_mod_exit();

    memdev_drv_modexit();

    mmngr_drv_modexit();

    mt_drv_stat_exit();

    mt_drv_sys_exit();

    mt_drv_log_exit();

    mt_drv_proc_exit();

    drv_mmz_modexit();
    //drv_pm_modexit();

#if !defined (MT_MCE_SUPPORT) && !defined(MT_KEYLED_CT1642_KERNEL_SUPPORT)
    mt_drv_commonexit ();
#endif

    return;
}

module_init(common_drv_modinit);
module_exit(common_drv_modexit);

MODULE_AUTHOR("MONTAGE");
MODULE_LICENSE("GPL");
