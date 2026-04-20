/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_FTRACE_H__
#define __MT_FTRACE_H__

#ifndef __KERNEL__
#include <sys/ioctl.h>
extern void mt_ftrace_mark0(void);
extern void mt_ftrace_mark1(void);
extern void mt_ftrace_mark2(void);
extern void mt_ftrace_mark_string(char *s);
extern void mt_usdt_mark0(void);
extern void mt_usdt_mark1(void);
extern void mt_usdt_mark2(void);
extern void mt_usdt_mark_x(void);
extern void mt_usdt_mark_y(void);
extern void mt_usdt_mark_z(void);
extern void mt_ftrace_stop(void);
/* note:
* mt_usdt_mark_string(xxx), not mt_usdt_mark_string("xxx")
* mt_usdt_mark_string(xxx_yyy), not mt_usdt_mark_string(xxx yyy), not mt_usdt_mark_string(xxx-yyy)
*/
#define mt_usdt_mark_string(s)	DTRACE_PROBE(MT_USDT_NAMESPACE, s)
#else
extern void mt_ftrace_k_mark0(void);
extern void mt_ftrace_k_mark1(void);
extern void mt_ftrace_k_mark2(void);
extern void mt_ftrace_k_mark_string(char *s);
extern void mt_ftrace_k_stop(void);
extern ulong mt_check_molva;
extern u64 mt_check_molva_rv;
extern u32 mt_check_molva_sz;
#endif /* __KERNEL__ */

#define __mt_stringify_1(x...)	#x
#define __mt_stringify(x...) __mt_stringify_1(x)

#define MT_USDT_NAMESPACE mt_usdt

#define MT_USDT_MARK0 mt0_mt0_mt0_mt0_mt0_mt0
#define MT_USDT_MARK1 mt1_mt1_mt1_mt1_mt1_mt1
#define MT_USDT_MARK2 mt2_mt2_mt2_mt2_mt2_mt2

#define MT_FTRACE_MARK0 __mt_stringify(MT_USDT_MARK0)
#define MT_FTRACE_MARK1 __mt_stringify(MT_USDT_MARK1)
#define MT_FTRACE_MARK2 __mt_stringify(MT_USDT_MARK2)
#define MT_FTRACE_MARK0_N MT_FTRACE_MARK0"\n"
#define MT_FTRACE_MARK1_N MT_FTRACE_MARK1"\n"
#define MT_FTRACE_MARK2_N MT_FTRACE_MARK2"\n"

#define MT_SCHED_CALLCHAIN_STRING_TAIL " ..... EE"

#endif /* __MT_FTRACE_H__ */
