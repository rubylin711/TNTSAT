#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/major.h>
#include <linux/uaccess.h>
#include "mt_type.h"
#include "mt_drv_proc.h"
#include "mt_osal.h"
#include "mt_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */

static  mt_proc_param_t  *procIntfParam = NULL;

mt_s32 mt_drv_proc_set_param(mt_proc_param_t *param)
{
	if (NULL == param)
	{
		//printk("CMPI_PROC_Register param err! \n");
		return MT_FAILURE;
	}
	if( (param->addfunc == NULL) ||
		(param->rmfunc == NULL))
	{
		//printk("CMPI_PROC_Register param err! \n");
		return MT_FAILURE;
	}
	procIntfParam = param;
    return MT_SUCCESS;
}

mt_void mt_drv_proc_clear_param(mt_void)
{
	procIntfParam = NULL;
	return;
}

mt_proc_entry_t* mt_drv_proc_add_module(mt_char *entry_name ,mt_drv_proc_t* pfnOpt, mt_void * data)
{
	if(procIntfParam){
		if(procIntfParam->addfunc){
			return procIntfParam->addfunc(entry_name, pfnOpt, data);
		}
	}
	return NULL;
}

mt_void mt_drv_proc_rm_module(char *entry_name)
{
	if(procIntfParam){
		if(procIntfParam->rmfunc){
			procIntfParam->rmfunc(entry_name);
		}
	}
	return;
}


mt_s32 mt_drv_proc_kinit(void)
{
	procIntfParam = NULL;
    return MT_SUCCESS;
}

mt_void mt_drv_proc_kexit(void)
{
	if(procIntfParam){
		mt_drv_proc_clear_param();
	}
    return ;
}

#if !(0 == MT_PROC_SUPPORT)
 /*
 * echo string to current terminal display(serial console or tty).
 * this implement implicit that current task file handle '0' must be terminal device file.
 * otherwise do nothing.
 */
mt_void mt_drv_proc_echohelpvargs(mt_char *buf, mt_u32 size, const mt_char * fmt, va_list args)
{
#define DEFAULT_ECHO_DEVICE_HANDLE (0)

    struct kstat stat;
    mt_s32 ret;

    if (!buf || 0 == size)
        return;

    ret = vfs_fstat(DEFAULT_ECHO_DEVICE_HANDLE, &stat);
    if (ret)
    {
        MT_PRINT("Default echo device handle(%u) invalid!\n", DEFAULT_ECHO_DEVICE_HANDLE);
        return;
    }

    mt_osal_vsnprintf(buf, size, fmt, args);

    /* echo device must be chrdev and major number must be TTYAUX_MAJOR or UNIX98_PTY_SLAVE_MAJOR */
    if ( S_ISCHR(stat.mode) && (MAJOR(stat.rdev) == TTYAUX_MAJOR || MAJOR(stat.rdev) == UNIX98_PTY_SLAVE_MAJOR || MAJOR(stat.rdev) == TTY_MAJOR ) )
    {
		struct file *file = fget(DEFAULT_ECHO_DEVICE_HANDLE);
		if (file) {
			loff_t pos = 0;

			ret = kernel_write(file, buf, strlen(buf), &pos);
			if (ret < 0) {
				MT_PRINT("write to echo device failed(%d)!\n", ret);
			}

			fput(file);
		}
    }
    else
    {
	MT_PRINT("mode 0x%x rdev (0x%x ) \n",stat.mode,stat.rdev);
        MT_PRINT("Default echo device is invalid!\n");
    }

}
EXPORT_SYMBOL(mt_drv_proc_echohelpvargs);

 /*
 * general echo helper function
 */
mt_void mt_drv_proc_echohelp(const mt_char *fmt, ...)
{
    char buf[512];
    va_list args;

    if (!fmt)
        return;

    va_start(args, fmt);
    mt_drv_proc_echohelpvargs(buf, sizeof(buf), fmt, args);
    va_end(args);
}
EXPORT_SYMBOL(mt_drv_proc_echohelp);

#endif

#ifndef MODULE
EXPORT_SYMBOL(mt_drv_proc_set_param);
EXPORT_SYMBOL(mt_drv_proc_clear_param);
#endif
EXPORT_SYMBOL(mt_drv_proc_add_module);
EXPORT_SYMBOL(mt_drv_proc_rm_module);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

