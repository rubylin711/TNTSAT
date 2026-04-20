/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/stacktrace.h>
#include <linux/kernel.h>

#define CREATE_TRACE_POINTS
#include <trace/events/mt.h>

#include "mt_ftrace.h"

#define MT_SCHED_BUF_SIZE 512
#define MT_SCHED_BUF_TAIL_SIZE 10
#define MT_SCHED_ENTRIES 16

static u8 mt_sched_callchain_enable = 0;

void mt_ftrace_k_mark0(void)
{
	trace_mt_handler_entry(MT_FTRACE_MARK0);
}
EXPORT_SYMBOL(mt_ftrace_k_mark0);

void mt_ftrace_k_mark1(void)
{
	trace_mt_handler_exit(MT_FTRACE_MARK1);
}
EXPORT_SYMBOL(mt_ftrace_k_mark1);

void mt_ftrace_k_mark2(void)
{
	trace_mt_handler_exit(MT_FTRACE_MARK2);
}
EXPORT_SYMBOL(mt_ftrace_k_mark2);

void mt_ftrace_k_mark_string(char *s)
{
	trace_mt_handler_string(s);
}
EXPORT_SYMBOL(mt_ftrace_k_mark_string);

void mt_ftrace_k_stop(void)
{
	tracing_off();
	mt_sched_callchain_enable = 0;
}
EXPORT_SYMBOL(mt_ftrace_k_stop);

static void noinline real_mt_sched_callchain(void)
{
#if defined(CONFIG_STACKTRACE)
	unsigned long mt_sched_entries[MT_SCHED_ENTRIES];
	char mt_sched_buf[MT_SCHED_BUF_SIZE];
	int i;
	int len = 0;
	unsigned int nr_entries;

	nr_entries = stack_trace_save(mt_sched_entries, ARRAY_SIZE(mt_sched_entries), 0);
	mt_sched_buf[0] = '\0';
	for (i = 0; (i < nr_entries) && (len < sizeof(mt_sched_buf)); i++) {
		len += snprintf(mt_sched_buf + len, sizeof(mt_sched_buf) - len, "\n => %pS", (void *)mt_sched_entries[i]);
	}
	strncpy(&mt_sched_buf[MT_SCHED_BUF_SIZE - MT_SCHED_BUF_TAIL_SIZE], MT_SCHED_CALLCHAIN_STRING_TAIL, MT_SCHED_BUF_TAIL_SIZE);
	mt_sched_buf[MT_SCHED_BUF_SIZE - 1] = '\0';
	trace_mt_sched_callchain(mt_sched_buf);
#endif
}

void mt_sched_callchain(void)
{
	if (mt_sched_callchain_enable) {
		/* to reduce stack usage */
		real_mt_sched_callchain();
	}
}
EXPORT_SYMBOL(mt_sched_callchain);

static int __init mt_sched_callchain_debugfs_init(void)
{
	debugfs_create_u8("mt_sched_callchain_enable", S_IRUSR | S_IWUSR | S_IRUGO,
						NULL, &mt_sched_callchain_enable);
	return 0;
}
late_initcall(mt_sched_callchain_debugfs_init);

void mt_ftrace_mdelay_start(unsigned long n)
{
	trace_mt_mdelay_start(n);
}
EXPORT_SYMBOL(mt_ftrace_mdelay_start);

void mt_ftrace_mdelay_end(unsigned long n)
{
	trace_mt_mdelay_end(n);
}
EXPORT_SYMBOL(mt_ftrace_mdelay_end);

bool mt_check_memory_overflow_linear_vaddr(char *ok, char *fail)
{
	bool ret = true;
#ifdef CONFIG_MT_DETECT_LINEAR_OVERFLOW
	bool print = (ok && fail) ? true : false;
	char buf[512];
	char *s;
	ulong mt_check_molva_tmp = mt_check_molva;
	static u32 fail_cnt = 0;

	if (!mt_check_molva_tmp) {
		return true;
	}

	if ((ok && !fail) || (!ok && fail)) {
		panic("ok and fail all should NULL or all should be string");
	}

	if (mt_check_molva_sz == sizeof(u64)) {
		ret = ((mt_check_molva_rv == *(u64 *)mt_check_molva_tmp) ? true : false);
	} else if (mt_check_molva_sz == sizeof(u32)) {
		ret = ((mt_check_molva_rv == *(u32 *)mt_check_molva_tmp) ? true : false);
	} else if (mt_check_molva_sz == sizeof(u16)) {
		ret = ((mt_check_molva_rv == *(u16 *)mt_check_molva_tmp) ? true : false);
	} else {
		ret = ((mt_check_molva_rv == *(u8 *)mt_check_molva_tmp) ? true : false);
	}

	if (print) {
		s = (ret ? ok : fail);
		if (mt_check_molva_sz == sizeof(u64)) {
			snprintf(buf, sizeof(buf), "%s, mt_check_molva_rv = 0x%llx, *(u64 *)mt_check_molva = 0x%llx, mt_check_molva_sz = %d, mt_check_molva = 0x%lx", s, mt_check_molva_rv, *(u64 *)mt_check_molva_tmp, mt_check_molva_sz, mt_check_molva_tmp);
		} else if (mt_check_molva_sz == sizeof(u32)) {
			snprintf(buf, sizeof(buf), "%s, mt_check_molva_rv = 0x%llx, *(u32 *)mt_check_molva = 0x%x, mt_check_molva_sz = %d, mt_check_molva = 0x%lx", s, mt_check_molva_rv, *(u32 *)mt_check_molva_tmp, mt_check_molva_sz, mt_check_molva_tmp);
		} else if (mt_check_molva_sz == sizeof(u16)) {
			snprintf(buf, sizeof(buf), "%s, mt_check_molva_rv = 0x%llx, *(u16 *)mt_check_molva = 0x%x, mt_check_molva_sz = %d, mt_check_molva = 0x%lx", s, mt_check_molva_rv, *(u16 *)mt_check_molva_tmp, mt_check_molva_sz, mt_check_molva_tmp);
		} else {
			snprintf(buf, sizeof(buf), "%s, mt_check_molva_rv = 0x%llx, *(u8 *)mt_check_molva = 0x%x, mt_check_molva_sz = %d, mt_check_molva = 0x%lx", s, mt_check_molva_rv, *(u8 *)mt_check_molva_tmp, mt_check_molva_sz, mt_check_molva_tmp);
		}
		mt_ftrace_k_mark_string(buf);
	}

	if (!ret) {
		fail_cnt++;
	}
	if (fail_cnt >= 5) {
		mt_ftrace_k_stop();
		mt_check_molva = 0;
	}
#endif
	return ret;
}
EXPORT_SYMBOL(mt_check_memory_overflow_linear_vaddr);
