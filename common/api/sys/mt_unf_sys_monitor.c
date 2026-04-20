/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2023, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_unf_sys_monitor.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2023/11/30
 * Description    : MT UNF System Monitor
 * History        :
 * 1.Date         : 2023/11/30
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "mt_common.h"
#include "mt_unf_timer.h"
#include "drv_pm_ioctl.h"

#include "mt_module_debug.h"

//#define DEBUG_MON

#if 0
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
/* sym4/6 */
#define ANA_AO_REG0							0xBF157000U
#define REG_TSENSOR_DATA					0xBF5D00F0U
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
/* sym1/2/3 */
#define ANA_AO_REG0							0x1F157000U
#define REG_TSENSOR_DATA					0x1F5D00F0U
#else
#error	"Please Config Select one of Symphony 1/2/4/6!"
#endif

/* bit 27: temp sensor pd */
#define TEMP_PD_SHIFT						27
#define TEMP_PD_MASK						(0x1 << TEMP_PD_SHIFT)

/* bit 0-12 */
#define TEMP_DATA_MASK						0x1FFF
#endif

/* timer interval */
#define TIMER_INTERVAL						10000	/* 10s */

/* temperature threshold */
#ifdef DEBUG_MON
#define TEMP_THRESHOLD_YELLOW				65
#define TEMP_THRESHOLD_RED					70
#else
#define TEMP_THRESHOLD_YELLOW				110
#define TEMP_THRESHOLD_RED					118
#endif

/* red threshold env */
#define ENV_TEMP_THRESHOLD_RED				"temp_threshold_red"

/* max temperature >= TEMP_THRESHOLD_RED times */
#define MAX_TEMP_STANDBY_PROTECT_COUNT		3

/* sleep time before enter standby */
#define SLEEP_BEFORE_SBY					3	/* 3s */

/* system monitor */
struct mt_sys_monitor
{
	int started;

	mt_u32 timer_id;

	mt_sys_event_cb_func callbacks[MT_SYS_EVENT_MAX];

	int standby_protect_counter;
};

/* <CNComment>只支持单例 */
static struct mt_sys_monitor local_sys_monitor = {0};
static struct mt_sys_monitor *plmon = &local_sys_monitor;

static pthread_mutex_t monitor_mutex = PTHREAD_MUTEX_INITIALIZER;

int mt_mpi_sys_monitor_start(void);
int mt_mpi_sys_monitor_stop(void);

/*
 * get temp threshold red value
 *   1. from env: "temp_threshold_red"
 *   2. from macro: TEMP_THRESHOLD_RED
 */
static int get_temp_threshold_red(void)
{
	char *env = NULL;
	long value;

	env = getenv(ENV_TEMP_THRESHOLD_RED);
	if (env != NULL)
	{
		value = strtol(env, NULL, 0);
		if ((value != LONG_MIN) && (value != LONG_MAX))
		{
#ifdef DEBUG_MON
			printf("get env %s: %ld\n", ENV_TEMP_THRESHOLD_RED, value);
#endif
			return (int)value;
		}
	}

	return TEMP_THRESHOLD_RED;
}

static void do_enter_standby(void)
{
	int fd;
	int setdev_flag = 0;
	FILE *file;
	char line[256] = {0};

#ifdef DEBUG_MON
	printf("%s: Sys Monitor - Enter Standby Caused by Temperature!!!\n", __FUNCTION__);
#endif
	MT_ERR_MISC("Enter Standby Caused by Temperature!!!\n");

	sleep(1);

#if 0
	if (MT_UNF_PMOC_SwitchSystemMode() != MT_SUCCESS)
	{
		MT_ERR_MISC("Enter Standby Failed!!!\n");
	}

#else

	fd = open("/dev/mt_pm",  O_RDWR | O_CLOEXEC);
	if (fd < 0)
	{
		MT_ERR_MISC("Open PM dev failed!!!\n");
		return;
	}

	file = fopen("/proc/msp/pm","r");
	if (NULL != file)
	{
		while (fgets(line,sizeof(line),file))
		{
			line[255] = 0;
			if (strstr(line,"unsupport"))
			{
				setdev_flag = 1;
				break;
			}
		}
		fclose(file);
	}
	else
	{
		setdev_flag = 1;
		MT_ERR_MISC("Error fopen file:/proc/msp/pm!!!\n");
	}

	if (1 == setdev_flag)
	{
		MT_ERR_MISC("Warn app not register monitor callback and cfg standbyinfo in callback!!!\n");
		if (ioctl(fd,CMD_PM_SET_FPDEV_TYPE,NOFP) < 0)
		{
			MT_ERR_MISC("IOCTL PM dev set type failed!!!\n");
		}
	}

	if (ioctl(fd, CMD_PM_IN_LOW_POWER, 0) < 0)
	{
		MT_ERR_MISC("IOCTL PM dev failed!!!\n");
		close(fd);
	}
#endif
}

#if 0
/* Check is Temp Sensor Power Down */
static int is_tsensor_pd(void)
{
	mt_u32 val = 0;

	mt_sys_read_register(ANA_AO_REG0, &val);
	if ((val & TEMP_PD_MASK) != 0)
	{
		//MT_ERR_MISC("Temp Sensor is power down!\n");
		return 1;
	}
	else
	{
		return 0;
	}
}

/* Temp Sensor Power Up */
static void tsensor_pu(void)
{
	mt_u32 val = 0;

	mt_sys_read_register(ANA_AO_REG0, &val);

	val &= ~TEMP_PD_MASK;

	mt_sys_write_register(ANA_AO_REG0, val);
}

/* Temp Sensor Power Down */
static void tsensor_pd(void)
{
	mt_u32 val = 0;

	mt_sys_read_register(ANA_AO_REG0, &val);

	val |= TEMP_PD_MASK;

	mt_sys_write_register(ANA_AO_REG0, val);
}

static mt_s32 get_temperature(int *temp)
{
	mt_u32 val = 0;

	if (is_tsensor_pd())
	{
		tsensor_pu();
		MT_USLEEP(150*1000);
	}

	mt_sys_read_register(REG_TSENSOR_DATA, &val);

	tsensor_pd();

	val &= TEMP_DATA_MASK;

#if defined(CONFIG_MT_CHIP_SYMPHONY2)

	/* 761.73*HEX2DEC(value)/8192-280.68 */
	*temp = (int)((long long)761730 * (long long)val / (long long)8192 - 280680);

#elif defined(CONFIG_MT_CHIP_SYMPHONY4)

	/* 783.46*HEX2DEC(value)/8192-290 */
	*temp = (int)((long long)783460 * (long long)val / (long long)8192 - 290000);

#elif defined(CONFIG_MT_CHIP_SYMPHONY6)

	/* 795.45*HEX2DEC(value)/8192-298.02 */
	*temp = (int)((long long)795450 * (long long)val / (long long)8192 - 298020);

#else

	return MT_FAILURE;

#endif

#ifdef DEBUG_MON
	printf("%s: reg val %x, temp %d\n", __FUNCTION__, val, *temp);
#endif


	return MT_SUCCESS;
}
#endif

/* temperature process */
static void temperature_proc(struct mt_sys_monitor *pmon)
{
	int ret;
	int temp = 0;
	int temp_int;
	int temp_dec;
	mt_sys_event_cb_func callback;
	int standby_protect_counter;
	int temp_threshold_red;

	//ret = mt_unf_misc_temperature_get(&temp);
	//ret = get_temperature(&temp);
	ret = mt_sys_get_temp(&temp);
	if (ret != MT_SUCCESS)
	{
		MT_WARN_MISC("sys monitor get temperature failed!\n");
		return;
	}

	temp_threshold_red = get_temp_threshold_red();

	temp_int = temp / 1000;
	temp_dec = temp % 1000;
	if (temp_dec < 0)
		temp_dec = -temp_dec;

#ifdef DEBUG_MON
	printf("%s: temperature - %d.%03d(C)\n", __FUNCTION__,
			temp_int,
			temp_dec
			);
#endif

	if (temp_int >= TEMP_THRESHOLD_YELLOW)
	{
		//>= 110

		if (temp_int >= temp_threshold_red)
		{
			//>= 118

			pthread_mutex_lock(&monitor_mutex);
			callback = pmon->callbacks[MT_SYS_EVENT_TEMP_HIGH_RED];
			pmon->standby_protect_counter ++;
			standby_protect_counter = pmon->standby_protect_counter;
			pthread_mutex_unlock(&monitor_mutex);

#ifdef DEBUG_MON
			printf("%s: Temp >= %d, Counter = %d!!!\n", __FUNCTION__, temp_threshold_red, standby_protect_counter);
#endif
			MT_ERR_MISC("Temperature(%d.%03d) >= %d - %d!!!\n",
				temp_int,
				temp_dec,
				temp_threshold_red, standby_protect_counter);

#ifdef DEBUG_MON
			printf("%s: Temp >= %d, Event = %d.\n", __FUNCTION__, temp_threshold_red, MT_SYS_EVENT_TEMP_HIGH_RED);
#endif
			if (callback)
				callback(MT_SYS_EVENT_TEMP_HIGH_RED, &temp);

			if (standby_protect_counter >= MAX_TEMP_STANDBY_PROTECT_COUNT)
			{
				sleep(SLEEP_BEFORE_SBY);

				do_enter_standby();
				if(standby_protect_counter >= (MAX_TEMP_STANDBY_PROTECT_COUNT + 1))
				{
					MT_ERR_MISC("Temperature(%d.%03d) >= %d, system will reboot !\n",temp_int, temp_dec, temp_threshold_red);
					system("reboot");
				}
			}
		}
		else
		{
			//>= 110, < 118
#ifdef DEBUG_MON
			printf("%s: %d <= Temp < %d, Event = %d.\n", __FUNCTION__,
				TEMP_THRESHOLD_YELLOW, temp_threshold_red, MT_SYS_EVENT_TEMP_HIGH_YELLOW);
#endif

			pthread_mutex_lock(&monitor_mutex);
			callback = pmon->callbacks[MT_SYS_EVENT_TEMP_HIGH_YELLOW];
			pmon->standby_protect_counter = 0;
			pthread_mutex_unlock(&monitor_mutex);

			if (callback)
				callback(MT_SYS_EVENT_TEMP_HIGH_YELLOW, &temp);
		}
	}
	else
	{
		//< 110
#ifdef DEBUG_MON
		printf("%s: Temp < %d, Event = %d.\n", __FUNCTION__, TEMP_THRESHOLD_YELLOW, MT_SYS_EVENT_TEMP);
#endif

		pthread_mutex_lock(&monitor_mutex);
		pmon->standby_protect_counter = 0;
		callback = pmon->callbacks[MT_SYS_EVENT_TEMP];
		pthread_mutex_unlock(&monitor_mutex);

		if (callback)
			callback(MT_SYS_EVENT_TEMP, &temp);
	}
}

/* timer callback */
static void sys_mon_timer_callback(void *priv)
{
	temperature_proc((struct mt_sys_monitor *)priv);

	//TODO: add your proc here:
}

/**
 * @Hide
 */
int mt_mpi_sys_monitor_start(void)
{
	int ret = MT_FAILURE;

	pthread_mutex_lock(&monitor_mutex);

	if (plmon->started)
	{
		MT_WARN_MISC("sys monitor already started!\n");
		ret = MT_SUCCESS;
		goto ERR;
	}

	if (mt_unf_timerfd_create(TIMER_INTERVAL, sys_mon_timer_callback, plmon, &plmon->timer_id) != MT_SUCCESS)
	{
		MT_ERR_MISC("create timer failed!\n");
		goto ERR;
	}

	plmon->standby_protect_counter = 0;

	if (mt_unf_timerfd_start(plmon->timer_id) != MT_SUCCESS)
	{
		mt_unf_timerfd_delete(plmon->timer_id);

		MT_ERR_MISC("start timer failed!\n");
		goto ERR;
	}

	plmon->started = 1;

#ifdef DEBUG_MON
	printf("%s: sys monitor start success.\n", __FUNCTION__);
#endif
	MT_INFO_MISC("sys monitor start success.\n");

	ret = MT_SUCCESS;

ERR:
	pthread_mutex_unlock(&monitor_mutex);
	return ret;
}

/**
 * @Hide
 */
int mt_mpi_sys_monitor_stop(void)
{
	int ret = MT_FAILURE;

	pthread_mutex_lock(&monitor_mutex);

	if (!plmon->started)
	{
		MT_WARN_MISC("sys monitor already stopped!\n");
		ret = MT_SUCCESS;
		goto ERR;
	}

	mt_unf_timerfd_stop(plmon->timer_id);

	mt_unf_timerfd_delete(plmon->timer_id);

	plmon->timer_id = (mt_u32)(-1);

	plmon->started = 0;

#ifdef DEBUG_MON
	printf("%s: sys monitor stop success.\n", __FUNCTION__);
#endif
	MT_INFO_MISC("sys monitor stop success.\n");

	ret = MT_SUCCESS;

ERR:
	pthread_mutex_unlock(&monitor_mutex);
	return ret;
}

int mt_unf_sys_monitor_register_event(MT_SYS_EVENT_T event, mt_sys_event_cb_func callback)
{
	if (event < 0 || event >= MT_SYS_EVENT_MAX)
	{
		MT_ERR_MISC("invalid event %d!\n", event);
		return MT_FAILURE;
	}

	pthread_mutex_lock(&monitor_mutex);

	plmon->callbacks[event] = callback;

#ifdef DEBUG_MON
	printf("%s: sys monitor register event(%d, %p) success.\n", __FUNCTION__, event, callback);
#endif

	MT_INFO_MISC("sys monitor register event(%d, %p) success.\n", event, callback);

	pthread_mutex_unlock(&monitor_mutex);

	return MT_SUCCESS;
}

int mt_unf_sys_monitor_register(mt_sys_event_cb_func callback)
{
	mt_unf_sys_monitor_register_event(MT_SYS_EVENT_TEMP_HIGH_YELLOW, callback);
	mt_unf_sys_monitor_register_event(MT_SYS_EVENT_TEMP_HIGH_RED, callback);

	return MT_SUCCESS;
}

