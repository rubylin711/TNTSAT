/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#include "drv_sys_misc.h"

#include <errno.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "mt_type.h"
#include "drv_sys_misc.h"
#include "mt_unf_misc.h"
#include "mt_module_debug.h"

#if 1
pthread_mutex_t misc_mutex;
static int misc_init_flag = 0;
static int g_misc_fd = 0;

int mt_unf_misc_pinmux_get(int idx, int offset, int len, unsigned int *val)
{
    struct drv_misc_ioctl misc_info;
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    misc_info.id = (unsigned int)idx;
    misc_info.offset = (unsigned int)offset;
    misc_info.len = (unsigned int)len;
    misc_info.val = 0;
    ret = ioctl(g_misc_fd, MISC_IOCTL_PINMUX_GET, &misc_info);

    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    *val = misc_info.val;
    MT_INFO_MISC("idx: %d offset:%d len:%d value:0x%x \n",idx, offset, len, misc_info.val);
    return MT_SUCCESS;
}
int mt_unf_misc_pinmux_set(int idx, int offset, int len, unsigned int val)
{
    struct drv_misc_ioctl misc_info;
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    misc_info.id = (unsigned int)idx;
    misc_info.offset = (unsigned int)offset;
    misc_info.len = (unsigned int)len;
    misc_info.val = (unsigned int)val;
    ret = ioctl(g_misc_fd, MISC_IOCTL_PINMUX_SET, &misc_info);

    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    MT_INFO_MISC("idx: %d offset:%d len:%d value:0x%x \n",idx, offset, len, misc_info.val);
    return MT_SUCCESS;
}
int mt_unf_misc_pinmux_init(void)
{
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    ret = ioctl(g_misc_fd, MISC_IOCTL_PINMUX_PRESET, NULL);
    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    return MT_SUCCESS;
}
int mt_unf_misc_reg_get(unsigned int idx, int offset, int len, unsigned int *val)
{
    struct drv_misc_ioctl misc_info;
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    misc_info.id = idx;
    misc_info.offset = (unsigned int)offset;
    misc_info.len = (unsigned int)len;
    misc_info.val = 0;
    ret = ioctl(g_misc_fd, MISC_IOCTL_REG_GET, &misc_info);

    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    MT_INFO_MISC("idx: 0x%x offset:%d len:%d value:0x%x \n",idx, offset, len, misc_info.val);
    *val = misc_info.val;
    return MT_SUCCESS;
}
int mt_unf_misc_reg_set(unsigned int idx, int offset, int len, unsigned int val)
{
    struct drv_misc_ioctl misc_info;
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    misc_info.id = (unsigned int)idx;
    misc_info.offset = (unsigned int)offset;
    misc_info.len = (unsigned int)len;
    misc_info.val = (unsigned int)val;
    ret = ioctl(g_misc_fd, MISC_IOCTL_REG_SET, &misc_info);

    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    MT_INFO_MISC("idx: 0x%x offset:%d len:%d value:0x%x \n",idx, offset, len, misc_info.val);
    return MT_SUCCESS;
}

int mt_unf_misc_module_set(int mid, int on)
{
    struct drv_misc_ioctl misc_info;
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    misc_info.id = (unsigned int)mid;
    misc_info.offset = 0;
    misc_info.len = 0;
    misc_info.val = (unsigned int)on;
    ret = ioctl(g_misc_fd, MISC_IOCTL_MODULE_ONOFF, &misc_info);
    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    return MT_SUCCESS;
}

int mt_unf_misc_module_is_enabled(int mid, int *val)
{
    struct drv_misc_ioctl misc_info;
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    misc_info.id = (unsigned int)mid;
    misc_info.offset = 0;
    misc_info.len = 0;
    misc_info.val = 0;
    ret = ioctl(g_misc_fd, MISC_IOCTL_MODULE_CLK_IS_ENABLED, &misc_info);
    *val = misc_info.val;
    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    return MT_SUCCESS;
}

int mt_unf_misc_module_clk_set(int mid, int clk_idx)
{
    struct drv_misc_ioctl misc_info;
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    misc_info.id = (unsigned int)mid;
    misc_info.offset = 0;
    misc_info.len = 0;
    misc_info.val = (unsigned int)clk_idx;
    ret = ioctl(g_misc_fd, MISC_IOCTL_MODULE_CLK_SET, &misc_info);
    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    return MT_SUCCESS;
}

int mt_unf_misc_module_clk_get(int mid, unsigned long *clk)
{
    struct drv_misc_ioctl misc_info;
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    misc_info.id = (unsigned int)mid;
    misc_info.offset = 0;
    misc_info.len = 0;
    misc_info.val = 0;
    ret = ioctl(g_misc_fd, MISC_IOCTL_MODULE_CLK_GET, &misc_info);
    *clk = misc_info.val;
    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    return MT_SUCCESS;
}

int mt_unf_misc_temperature_get(long *temperature)
{
    struct drv_misc_ioctl misc_info;
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    memset(&misc_info, 0, sizeof(struct drv_misc_ioctl));
    ret = ioctl(g_misc_fd, MISC_IOCTL_TEMPERATURE_GET, &misc_info);
    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    *temperature = (long)misc_info.val;
    return MT_SUCCESS;
}

int mt_unf_misc_chip_productinfo_get(unsigned char*buff,unsigned char len)
{
	unsigned char info[64];
	int ret = 0;

	if((NULL == buff) || (0 == len))
	{
		return MT_FAILURE;
	}
	pthread_mutex_lock(&misc_mutex);
	memset(info, 0, 64);
	ret = ioctl(g_misc_fd, MISC_IOCTL_CHIP_PRODUCTINFO_GET, info);
	pthread_mutex_unlock(&misc_mutex);
	if(ret != 0)
		return MT_FAILURE;
	memcpy(buff,info,len);
	return MT_SUCCESS;
}

#if 0
int mt_unf_misc_module_state_dump(void)
{
    int ret = 0;

    pthread_mutex_lock(&misc_mutex);
    ret = ioctl(g_misc_fd, MISC_IOCTL_MODULE_DBG, NULL);
    pthread_mutex_unlock(&misc_mutex);
    if(ret != 0)
        return MT_FAILURE;
    return MT_SUCCESS;
}
#endif

int mt_unf_misc_init(void)
{

    if(misc_init_flag > 0)
    {
        return MT_SUCCESS;
    }
    g_misc_fd = open(MT_MISC_DEVNAME, O_RDWR|O_EXCL);

    if(g_misc_fd < 0)
    {
        MT_ERR_MISC("fd = %d\n",g_misc_fd);
        return MT_FAILURE;
    }
    pthread_mutex_init(&misc_mutex, NULL);
    misc_init_flag++;
    return MT_SUCCESS;
}
void mt_unf_misc_release(void)
{
    if(misc_init_flag <= 0)
    {
        return;
    }
    pthread_mutex_destroy(&misc_mutex);
    misc_init_flag = 0;
    close(g_misc_fd);
}
#endif

