#include <linux/version.h>
#include <linux/seq_file.h>
#include "mt_type.h"
#include "mt_drv_log.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_sys.h"
#include "mt_drv_mmz.h"
#include "mt_drv_module.h"
#include "mt_debug.h"


/* Use "strings mt_xx.ko | grep "SDK_VERSION"" to get the version */
/*MT_CHAR g_ModuleVersion[160] ="SDK_VERSION:["\
    MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
    __DATE__", "__TIME__"]";
*/

mt_s32 __init mt_drv_commoninit(mt_void)
{
    mt_s32 ret;

    ret = mt_drv_mmz_init();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("DRV_MMZ_ModInit failed:%#x!\n", ret);
        goto ErrExit_MMZ;
    }

#if 0
	ret = mt_drv_dev_init();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("CMPI_DEV_ModInit_0 failed:%#x!\n", ret);
        goto ErrExit_DEV;
    }
#endif
    ret = mt_drv_log_kinit();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("MT_DRV_LOG_KInit failed:%#x!\n", ret);
       // goto ErrExit_LOG;
    }
    ret = mt_drv_proc_kinit();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("MT_DRV_PROC_KInit failed:%#x!\n", ret);
       // goto ErrExit_PROC;
    }
    ret = mt_drv_stat_kinit();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("MT_DRV_STAT_KInit failed:%#x!\n", ret);
        goto ErrExit_STAT;
    }
    ret = mt_drv_sys_kinit();
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("MT_DRV_STAT_KInit failed:%#x!\n", ret);
        goto ErrExit_SYS;
    }
    ret = mt_drv_mmngr_init(MT_KMODULE_MAX_COUNT, MT_KMODULE_MEM_MAX_COUNT);
    if(MT_SUCCESS != ret)
    {
        MT_ERR_SYS("KModuleMgr_Init failed:%#x!\n", ret);
        goto ErrExit_SYS;
    }
    MT_INFO_LOG("====mt_drv_commoninit=====777=======\n");
    return MT_SUCCESS;

ErrExit_SYS:
    mt_drv_stat_kexit();

ErrExit_STAT:
    mt_drv_proc_kexit();

//ErrExit_PROC:
    mt_drv_log_kexit();

//ErrExit_LOG:
    mt_drv_dev_exit();

//ErrExit_DEV:
    mt_drv_mmz_exit();

ErrExit_MMZ:

	MT_INFO_LOG("====mt_drv_commoninit=====555=======\n");
    return ret;
}

mt_void mt_drv_commonexit(mt_void)
{
    mt_drv_mmngr_exit();

    mt_drv_sys_kexit();

    mt_drv_stat_kexit();

    mt_drv_log_kexit();

    mt_drv_proc_kexit();

    mt_drv_dev_exit();

    return;
}


