/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sdt.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include "mt_ftrace.h"

static int fd_trace_marker = -1;
static int fd_tracing_on = -1;
static int fd_mt_sched_callchain_enable = -1;

static void mt_ftrace_init(void)
{
	fd_trace_marker = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY | O_CLOEXEC, 0200);
	fd_tracing_on = open("/sys/kernel/debug/tracing/tracing_on", O_WRONLY | O_CLOEXEC, 0200);
	fd_mt_sched_callchain_enable = open("/sys/kernel/debug/mt_sched_callchain_enable", O_WRONLY | O_CLOEXEC, 0200);
}

void mt_ftrace_mark0(void)
{
	if (fd_trace_marker == -1) {
		mt_ftrace_init();
	}

	if (fd_trace_marker != -1) {
		lseek(fd_trace_marker, 0, SEEK_SET);
		write(fd_trace_marker, MT_FTRACE_MARK0, strlen(MT_FTRACE_MARK0));
	}
}

void mt_ftrace_mark1(void)
{
	if (fd_trace_marker == -1) {
		mt_ftrace_init();
	}

	if (fd_trace_marker != -1) {
		lseek(fd_trace_marker, 0, SEEK_SET);
		write(fd_trace_marker, MT_FTRACE_MARK1, strlen(MT_FTRACE_MARK1));
	}
}

void mt_ftrace_mark2(void)
{
	if (fd_trace_marker == -1) {
		mt_ftrace_init();
	}

	if (fd_trace_marker != -1) {
		lseek(fd_trace_marker, 0, SEEK_SET);
		write(fd_trace_marker, MT_FTRACE_MARK2, strlen(MT_FTRACE_MARK2));
	}
}

void mt_ftrace_mark_string(char *s)
{
	if (fd_trace_marker == -1) {
		mt_ftrace_init();
	}

	if (fd_trace_marker != -1) {
		lseek(fd_trace_marker, 0, SEEK_SET);
		write(fd_trace_marker, s, strlen(s));
	}
}

void mt_usdt_mark0(void)
{
	DTRACE_PROBE(MT_USDT_NAMESPACE, MT_USDT_MARK0);
}

void mt_usdt_mark1(void)
{
	DTRACE_PROBE(MT_USDT_NAMESPACE, MT_USDT_MARK1);
}

void mt_usdt_mark2(void)
{
	DTRACE_PROBE(MT_USDT_NAMESPACE, MT_USDT_MARK2);
}

void mt_usdt_mark_x(void)
{
	DTRACE_PROBE(MT_USDT_NAMESPACE, mt_xxx);
}

void mt_usdt_mark_y(void)
{
	DTRACE_PROBE(MT_USDT_NAMESPACE, mt_yyy);
}

void mt_usdt_mark_z(void)
{
	DTRACE_PROBE(MT_USDT_NAMESPACE, mt_zzz);
}

void mt_ftrace_stop(void)
{
	if (fd_trace_marker == -1) {
		mt_ftrace_init();
	}

	if (fd_tracing_on != -1) {
		lseek(fd_tracing_on, 0, SEEK_SET);
		write(fd_tracing_on, "0", strlen("0"));
	}

	if (fd_mt_sched_callchain_enable != -1) {
		lseek(fd_mt_sched_callchain_enable, 0, SEEK_SET);
		write(fd_mt_sched_callchain_enable, "0", strlen("0"));
	}
}
