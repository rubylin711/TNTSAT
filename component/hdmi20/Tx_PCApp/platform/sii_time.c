// This is for delay or time stamp use library
#include "mt_hdmi20_cfg.h"
#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__ || __HDMI_OS_RTOS__)
	#include <stdio.h>
	#include <time.h>
	#if ( __HDMI_UBOOT__ != 1 )
	#include <unistd.h>
	#include <sys/time.h>
	#endif
#else
	#include <linux/ktime.h>
	#include <linux/delay.h>
#endif
#include "sii_time.h"
#include "si_datatypes.h"
#if (__HDMI_OS_LINUX__ || __HDMI_OS_KERNEL__)
	#include "mt_type.h"
	#include "si_lib_log_api.h"
#endif

#if __HDMI_OS_RTOS__
	#include <sys/timeb.h>
	#include "sys_types.h"
	#include "mtos_misc.h"
	extern void mtos_task_sleep(u32 ms);
#endif // __HDMI_OS_RTOS__
#if (__HDMI_OS_LINUX__)
	#include <sys/timeb.h>
#elif (__HDMI_UBOOT__)
	#include <time.h>
#endif

static unsigned long processInitTime;

unsigned long Sii_Get_Process_Time(void)
{
	unsigned long globalTime = SiI_get_global_time();
	if (globalTime > processInitTime) {
		return (globalTime - processInitTime);
	} else {
		processInitTime = globalTime;
	}
	return globalTime;
}

// This function will return time with ms
//
extern unsigned long SiI_get_global_time( void )
{
	#if (__HDMI_OS_KERNEL__)
	struct timespec64 ts;
	ktime_t kt;
	s64 ms;

	memset(&ts, 0, sizeof(struct timespec64));
	ktime_get_real_ts64(&ts);
	kt = timespec64_to_ktime(ts);
	ms = ktime_to_ms(kt);

	return (unsigned long)ms;
	#elif __HDMI_UBOOT__
	unsigned long ms;
	ms = timer_get_us() / 1000;
	return ms;
	#elif __HDMI_OS_RTOS__
	u32  s = 0;  // s++  to 0xFFFFFFFF;
	u32  ms = 0; // 0-1000 loop
	u32  us = 0; //
	u32  ms_t =0;
	mtos_systime_get(&s, &ms, &us);
	ms_t = s*1000 + ms;
	return (unsigned long)ms_t;
	#else
	struct timeval tv = {0};
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
	#endif
}
unsigned long SiI_difftime( unsigned long t )
{
	return SiI_get_global_time() - t;
}

#if (__HDMI_OS_LINUX__)
	//extern int usleep (unsigned int useconds);
#elif (__HDMI_OS_KERNEL__)
	#define Sleep msleep
#elif (__HDMI_UBOOT__)
	extern void udelay(unsigned long usec);
#endif

extern void SiI_DelayMS( unsigned long delay )
{
	#if __HDMI_OS_LINUX__
	unsigned long t0,t1,t2,td;
	t0 = Sii_Get_Process_Time();
	td = delay;
	do {
		usleep(td * 1000);
		t1 = Sii_Get_Process_Time();
		if (t1 > t0) {
			t2 = t1 - t0;
		} else {
			t2 = delay;
		}
		if ( t2 >= delay ) {
			td = 0;
		} else {
			td = delay - t2;
		}
		if (td) {
			//printf("compt:%ld %ld %ld %ld %ld (ms)\n",td,delay,t2,t1,t0);
		}
	} while (td);
	#elif __HDMI_UBOOT__
	udelay(delay * 1000);
	#elif __HDMI_OS_KERNEL__
	if (!in_atomic()) {
		Sleep( delay );
	} else {
		udelay(delay * 1000);
	}
	#elif __HDMI_OS_RTOS__
	mtos_task_sleep(delay);
	#else
	Sleep( delay );
	#endif
}

/*****************************************************************************/
/**
*  @brief  Waits for the specified number of milliseconds
*
*  @param[in]  baseTime: Set the baseTime that start to delay
*
*  @param[in]  delay: number of milliseconds
*
*  @return     If delay expired, return true
*
*****************************************************************************/
extern char SiI_TimerDelay(unsigned long baseTime, unsigned long delay)
{
	uint32_t diff_time = SiI_difftime(baseTime);

	if ( diff_time < delay) {
		return false;
	} else {
		return true;
	}
}

void Sii_Init_ProcessTime(void)
{
	processInitTime = SiI_get_global_time();
}

void SiI_DelayUS( unsigned long delay )
{
	#if __HDMI_OS_LINUX__
	usleep(delay);
	#elif __HDMI_UBOOT__
	udelay(delay);
	#else
	#if __HDMI_OS_KERNEL__
	if (!in_atomic()) {
		if (delay >= 1000) {
			delay /= 1000;
		} else {
			delay = 1;
		}
		msleep(delay);   //
	} else {
		udelay(delay);
	}
	#elif __HDMI_OS_RTOS__
	mtos_task_sleep(delay);
	#else
	msleep(delay);   //
	#endif
	#endif
}

