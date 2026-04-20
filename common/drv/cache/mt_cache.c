/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#include <asm/barrier.h>
#else /* CONFIG_MIPS */
#include <asm/cpu-features.h>
#endif
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/mt_mmz.h>
#include "mt_type.h"
#include "mt_cache.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#ifdef CONFIG_OUTER_CACHE
#error "not support outer cache"
#endif

#undef TEST_MT_CACHE_API
#undef TEST_LOCAL_BUS
#undef TEST_AVPLAY
//#undef CONFIG_SMP
#undef DO__CANNOT_DO



#ifdef TEST_MT_CACHE_API

#undef MT_CACHE_BUS_LOG_ENABLE
#ifdef MT_CACHE_BUS_LOG_ENABLE
#define MT_CACHE_BUS_LOG(format, ...) {printk(KERN_EMERG "cpu : %d, " format, smp_processor_id(), ##__VA_ARGS__);}
#else
#define MT_CACHE_BUS_LOG(format, ...)
#endif

#undef MT_CACHE_BUS_LOG_ENABLE_1
#ifdef MT_CACHE_BUS_LOG_ENABLE_1
#define MT_CACHE_BUS_LOG_1(format, ...) {printk(KERN_EMERG "cpu : %d, " format, smp_processor_id(), ##__VA_ARGS__);}
#else
#define MT_CACHE_BUS_LOG_1(format, ...)
#endif

struct mt_dc_test_completion {
	struct completion kernel_completion;
	struct completion user_completion;
	u32 cpu;
};

struct task_struct *dcache_test_task0;
struct mt_dc_test_completion mdtc0;
#ifdef CONFIG_SMP
struct task_struct *dcache_test_task1;
struct mt_dc_test_completion mdtc1;
#endif
static unsigned long cache_and_bus_test_enable = 1;

static int random_gap_test_times;
static bool test_run = false;
static ulong dvfs_test_percpu = -1;

static void _mt_dcache_flush(void *vaddr, size_t bytes);
static void _mt_dcache_clean(void *vaddr, size_t bytes);
static void _mt_dcache_invalid(void *vaddr, size_t bytes);

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#define TEST_BUFFER_SIZE 0x100000
#define AP_SRAM_BOOT_EN 0x10
#define BSRAM_EN_1 (1 << 1)
#define BSRAM_EN_0 (1 << 0)

#ifdef TEST_LOCAL_BUS
#define FLUSH_DCACHE
#define TEST_LOCAL_BUS_PERIOD	(3 * HZ)

static void __mt_local_bus_test(void *buf_cacheh, void *buf_uncacheh, void *phyh, void *buf_cachel, void *buf_uncachel, void *phyl, u32 cpu)
{
	u32 count = 0;
	u32 time;
	u32 i, j;
	u32 flush_times;
	u64 timeout;
#ifndef TEST_AVPLAY
	unsigned long val;
#endif

	volatile void *p;
	volatile void *up;
	volatile u8 *d0;
	volatile u16 *d1;
	volatile u32 *d2;
	volatile u64 *d3;

	volatile void *ph;		/* cached pointer, not physical */
	volatile void *uph;		/* uncache pointer */
	volatile u8 *d0h;
	volatile u16 *d1h;
	volatile u32 *d2h;
	volatile u64 *d3h;

	volatile void *pl;
	volatile void *upl;
	volatile u8 *d0l;
	volatile u16 *d1l;
	volatile u32 *d2l;
	volatile u64 *d3l;

	u64 r;
	u8 off;

	off = (get_random_u32() & 0x7);

	MT_CACHE_BUS_LOG("\33[44;32m unalign off: off = %d \33[0m \n", off);

	ph = buf_cacheh;
	uph = buf_uncacheh;
	pl = buf_cachel;
	upl = buf_uncachel;

	MT_CACHE_BUS_LOG("\33[44;31m test ddr uncache random gap \33[0m \n");
	time = 0;
	while (time < random_gap_test_times) {
		unsigned long n;
		unsigned long m[128];
		unsigned long pos[128];
		bool found;

		time++;
		n = (get_random_u64() & 0x7f) + 1;
		MT_CACHE_BUS_LOG_1("n = %lld\n", (u64)n);

		count = ((0xf + 1) * (0x7f + 1)) / sizeof(*d0);
		d0h = (u8 *)uph;
		d0l = (u8 *)upl;
		memset((void *)d0h, 0, count);
		memset((void *)d0l, 0xff, count);

		for (i = 0; i < n; i++) {
			m[i] = (get_random_u64() & 0xf) + 1;
			pos[i] = i == 0 ? m[i] : (pos[i - 1] + m[i]);
			MT_CACHE_BUS_LOG_1("m[%d] = %lld\n", i, (u64)m[i]);
			MT_CACHE_BUS_LOG_1("pos[%d] = %lld\n", i, (u64)pos[i]);
			*(d0h + pos[i]) = 0x55;
			*(d0l + pos[i]) = ~0x55;
		}

		for (i = 0; i < count; i++) {
			found = false;
			for (j = 0; j < n; j++) {
				if (i == pos[j]) {
					if (*(d0h + i) != 0x55) {
						panic("test ddr %ld 1byte uncache random gap h pos failed, i = %lld\n", (ulong)sizeof(*d0), (u64)i);
					}
					if (*(d0l + i) == ~0x55) {
						panic("test ddr %ld 1byte uncache random gap l pos failed, i = %lld\n", (ulong)sizeof(*d0), (u64)i);
					}
					found = true;
					break;
				}
			}
			if (!found) {
				if (*(d0h + i) != 0) {
					panic("test ddr %ld 1byte uncache random gap h no-pos failed, i = %lld\n", (ulong)sizeof(*d0), (u64)i);
				}
				if (*(d0l + i) != 0xff) {
					panic("test ddr %ld 1byte uncache random gap l no-pos failed, i = %lld\n", (ulong)sizeof(*d0), (u64)i);
				}
			}
		}

		msleep(1);
	}

	MT_CACHE_BUS_LOG("\33[44;31m test ddr uncache \33[0m \n");
	timeout = jiffies + TEST_LOCAL_BUS_PERIOD;
	time = 0;
	while (jiffies < timeout) {
		time++;

		count = (TEST_BUFFER_SIZE - off) / sizeof(*d0);
		d0h = (u8 *)(uph + off);
		d0l = (u8 *)(upl + off);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d0) << 3); j++) {
				*d0h = (u8)((u64)1 << j);
				*d0l = (u8)(~((u64)1 << ((sizeof(*d0) << 3) - 1 - j)));
				if (*d0h != (u8)((u64)1 << j)) {
					panic("test ddr %ld 1byte uncache h failed at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)((u64)1 << j), (u64)i);
				}
				if (*d0l != (u8)(~((u64)1 << ((sizeof(*d0) << 3) - 1 - j)))) {
					panic("test ddr %ld 1byte uncache l failed at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)(~((u64)1 << ((sizeof(*d0) << 3) - 1 - j))), (u64)i);
				}
			}
			d0h++;
			d0l++;
		}

		count = (TEST_BUFFER_SIZE - off) / sizeof(*d1);
		d1h = (u16 *)(uph + off);
		d1l = (u16 *)(upl + off);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d1) << 3); j++) {
				*d1h = (u16)((u64)1 << j);
				*d1l = (u16)(~((u64)1 << ((sizeof(*d1) << 3) - 1 - j)));
				if (*d1h != (u16)((u64)1 << j)) {
					panic("test ddr %ld 2byte uncache h failed at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)((u64)1 << j), (u64)i);
				}
				if (*d1l != (u16)(~((u64)1 << ((sizeof(*d1) << 3) - 1 - j)))) {
					panic("test ddr %ld 2byte uncache l failed at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)(~((u64)1 << ((sizeof(*d1) << 3) - 1 - j))), (u64)i);
				}
			}
			d1h++;
			d1l++;
		}

		count = (TEST_BUFFER_SIZE - off) / sizeof(*d2);
		d2h = (u32 *)(uph + off);
		d2l = (u32 *)(upl + off);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d2) << 3); j++) {
				*d2h = (u32)((u64)1 << j);
				*d2l = (u32)(~((u64)1 << ((sizeof(*d2) << 3) - 1 - j)));
				if (*d2h != (u32)((u64)1 << j)) {
					panic("test ddr %ld 4byte uncache h failed at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)((u64)1 << j), (u64)i);
				}
				if (*d2l != (u32)(~((u64)1 << ((sizeof(*d2) << 3) - 1 - j)))) {
					panic("test ddr %ld 4byte uncache l failed at %x, i = %lld\n", (ulong)sizeof(*d2),(u32)( ~((u64)1 << ((sizeof(*d2) << 3) - 1 - j))), (u64)i);
				}
			}
			d2h++;
			d2l++;
		}

		count = (TEST_BUFFER_SIZE - off) / sizeof(*d3);
		d3h = (u64 *)(uph + off);
		d3l = (u64 *)(upl + off);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d3) << 3); j++) {
				*d3h = (u64)((u64)1 << j);
				*d3l = (u64)(~((u64)1 << ((sizeof(*d3) << 3) - 1 - j)));
				if (*d3h != (u64)((u64)1 << j)) {
					panic("test ddr %ld 8byte uncache h failed at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)((u64)1 << j), (u64)i);
				}
				if (*d3l != (u64)(~((u64)1 << ((sizeof(*d3) << 3) - 1 - j)))) {
					panic("test ddr %ld 8byte uncache l failed at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~((u64)1 << ((sizeof(*d3) << 3) - 1 - j))), (u64)i);
				}
			}
			d3h++;
			d3l++;
		}

		count = (TEST_BUFFER_SIZE - off) / sizeof(*d3);
		d3h = (u64 *)(uph + off);
		d3l = (u64 *)(upl + off);
		for (i = 0; i < count; i++) {
			for (j = 0; j < 16; j++) {
				r = get_random_u64();
				*d3h = (u64)r;
				*d3l = (u64)(~r);
				if (*d3h != (u64)r) {
					panic("test ddr %ld 8byte random uncache h failed at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)r, (u64)i);
				}
				if (*d3l != (u64)(~r)) {
					panic("test ddr %ld 8byte random uncache l failed at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~r), (u64)i);
				}
			}
			d3h++;
			d3l++;
		}

		msleep(1);
	}
	MT_CACHE_BUS_LOG("\33[44;31m test ddr uncache %d times \33[0m \n", time);

	MT_CACHE_BUS_LOG("\33[44;31m test ddr cache \33[0m \n");
	timeout = jiffies + TEST_LOCAL_BUS_PERIOD;
	time = 0;
	while (jiffies < timeout) {
		time++;

		count = (TEST_BUFFER_SIZE - off) / sizeof(*d0);
		d0h = (u8 *)(ph + off);
		d0l = (u8 *)(pl + off);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d0) << 3); j++) {
				*d0h = (u8)((u64)1 << j);
				*d0l = (u8)(~((u64)1 << ((sizeof(*d0) << 3) - 1 - j)));
				if (*d0h != (u8)((u64)1 << j)) {
					panic("test ddr %ld 1byte cache h failed 0 at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)((u64)1 << j), (u64)i);
				}
				if (*d0l != (u8)(~((u64)1 << ((sizeof(*d0) << 3) - 1 - j)))) {
					panic("test ddr %ld 1byte cache l failed 0 at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)(~((u64)1 << ((sizeof(*d0) << 3) - 1 - j))), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d0h, (ulong)sizeof(*d0));
				_mt_dcache_flush((void *)d0l, (ulong)sizeof(*d0));
				flush_times++;
				if (*(u8 *)((void *)d0h - ph + uph) != (u8)((u64)1 << j)) {
					panic("test ddr %ld 1byte cache h failed 1 at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)((u64)1 << j), (u64)i);
				}
				if (*(u8 *)((void *)d0l - pl + upl) != (u8)(~((u64)1 << ((sizeof(*d0) << 3) - 1 - j)))) {
					panic("test ddr %ld 1byte cache l failed 1 at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)(~((u64)1 << ((sizeof(*d0) << 3) - 1 - j))), (u64)i);
				}
#endif
			}
			d0h++;
			d0l++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = (TEST_BUFFER_SIZE - off) / sizeof(*d1);
		d1h = (u16 *)(ph + off);
		d1l = (u16 *)(pl + off);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d1) << 3); j++) {
				*d1h = (u16)((u64)1 << j);
				*d1l = (u16)(~((u64)1 << ((sizeof(*d1) << 3) - 1 - j)));
				if (*d1h != (u16)((u64)1 << j)) {
					panic("test ddr %ld 2byte cache h failed 0 at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)((u64)1 << j), (u64)i);
				}
				if (*d1l != (u16)(~((u64)1 << ((sizeof(*d1) << 3) - 1 - j)))) {
					panic("test ddr %ld 2byte cache l failed 0 at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)(~((u64)1 << ((sizeof(*d1) << 3) - 1 - j))), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d1h, (ulong)sizeof(*d1));
				_mt_dcache_flush((void *)d1l, (ulong)sizeof(*d1));
				flush_times++;
				if (*(u16 *)((void *)d1h - ph + uph) != (u16)((u64)1 << j)) {
					panic("test ddr %ld 2byte cache h failed 1 at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)((u64)1 << j), (u64)i);
				}
				if (*(u16 *)((void *)d1l - pl + upl) != (u16)(~((u64)1 << ((sizeof(*d1) << 3) - 1 - j)))) {
					panic("test ddr %ld 2byte cache l failed 1 at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)(~((u64)1 << ((sizeof(*d1) << 3) - 1 - j))), (u64)i);
				}
#endif
			}
			d1h++;
			d1l++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = (TEST_BUFFER_SIZE - off) / sizeof(*d2);
		d2h = (u32 *)(ph + off);
		d2l = (u32 *)(pl + off);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d2) << 3); j++) {
				*d2h = (u32)((u64)1 << j);
				*d2l = (u32)(~((u64)1 << ((sizeof(*d2) << 3) - 1 - j)));
				if (*d2h != (u32)((u64)1 << j)) {
					panic("test ddr %ld 4byte cache h failed 0 at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)((u64)1 << j), (u64)i);
				}
				if (*d2l != (u32)(~((u64)1 << ((sizeof(*d2) << 3) - 1 - j)))) {
					panic("test ddr %ld 4byte cache l failed 0 at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)(~((u64)1 << ((sizeof(*d2) << 3) - 1 - j))), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d2h, (ulong)sizeof(*d2));
				_mt_dcache_flush((void *)d2l, (ulong)sizeof(*d2));
				flush_times++;
				if (*(u32 *)((void *)d2h - ph + uph) != (u32)((u64)1 << j)) {
					panic("test ddr %ld 4byte cache h failed 1 at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)((u64)1 << j), (u64)i);
				}
				if (*(u32 *)((void *)d2l - pl + upl) != (u32)(~((u64)1 << ((sizeof(*d2) << 3) - 1 - j)))) {
					panic("test ddr %ld 4byte cache l failed 1 at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)(~((u64)1 << ((sizeof(*d2) << 3) - 1 - j))), (u64)i);
				}
#endif
			}
			d2h++;
			d2l++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = (TEST_BUFFER_SIZE - off) / sizeof(*d3);
		d3h = (u64 *)(ph + off);
		d3l = (u64 *)(pl + off);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d3) << 3); j++) {
				*d3h = (u64)((u64)1 << j);
				*d3l = (u64)(~((u64)1 << ((sizeof(*d3) << 3) - 1 - j)));
				if (*d3h != (u64)((u64)1 << j)) {
					panic("test ddr %ld 8byte cache h failed 0 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)((u64)1 << j), (u64)i);
				}
				if (*d3l != (u64)(~((u64)1 << ((sizeof(*d3) << 3) - 1 - j)))) {
					panic("test ddr %ld 8byte cache l failed 0 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~((u64)1 << ((sizeof(*d3) << 3) - 1 - j))), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d3h, (ulong)sizeof(*d3));
				_mt_dcache_flush((void *)d3l, (ulong)sizeof(*d3));
				flush_times++;
				if (*(u64 *)((void *)d3h - ph + uph) != (u64)((u64)1 << j)) {
					panic("test ddr %ld 8byte cache h failed 1 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)((u64)1 << j), (u64)i);
				}
				if (*(u64 *)((void *)d3l - pl + upl) != (u64)(~((u64)1 << ((sizeof(*d3) << 3) - 1 - j)))) {
					panic("test ddr %ld 8byte cache l failed 1 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~((u64)1 << ((sizeof(*d3) << 3) - 1 - j))), (u64)i);
				}
#endif
			}
			d3h++;
			d3l++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = (TEST_BUFFER_SIZE - off) / sizeof(*d3);
		d3h = (u64 *)(ph + off);
		d3l = (u64 *)(pl + off);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < 16; j++) {
				r = get_random_u64();
				*d3h = (u64)r;
				*d3l = (u64)(~r);
				if (*d3h != r) {
					panic("test ddr %ld 8byte random cache h failed 0 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)r, (u64)i);
				}
				if (*d3l != ~r) {
					panic("test ddr %ld 8byte random cache l failed 0 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~r), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d3h, (ulong)sizeof(*d3));
				_mt_dcache_flush((void *)d3l, (ulong)sizeof(*d3));
				flush_times++;
				if (*(u64 *)((void *)d3h - ph + uph) != (u64)r) {
					panic("test ddr %ld 8byte random cache h failed 1 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)r, (u64)i);
				}
				if (*(u64 *)((void *)d3l - pl + upl) != (u64)(~r)) {
					panic("test ddr %ld 8byte random cache l failed 1 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~r), (u64)i);
				}
#endif
			}
			d3h++;
			d3l++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}
	}
	MT_CACHE_BUS_LOG("\33[44;31m test ddr cache %d times \33[0m \n", time);

#ifndef TEST_AVPLAY
	p = ioremap_cache((phys_addr_t)SYMPHONY_BSRAM_RAM_PHYS_BASE + (cpu * (SYMPHONY_BSRAM_RAM_MAPPING_SIZE >> 1)),
		(SYMPHONY_BSRAM_RAM_MAPPING_SIZE >> 1));
	up = (void *)SYMPHONY_BSRAM_RAM_VIRT_BASE + (cpu * (SYMPHONY_BSRAM_RAM_MAPPING_SIZE >> 1));

	val = readl((void *)(SYMPHONY_BOOT_REGISTER_VIRT_BASE + AP_SRAM_BOOT_EN));
	val |= BSRAM_EN_1 | BSRAM_EN_0;
	writel(val, (void *)(SYMPHONY_BOOT_REGISTER_VIRT_BASE + AP_SRAM_BOOT_EN));

	MT_CACHE_BUS_LOG("\33[44;32m sram support unalign access \33[0m \n");
	MT_CACHE_BUS_LOG("\33[44;31m test sram uncache random gap \33[0m \n");
	time = 0;
	while (time < random_gap_test_times) {
		unsigned long n;
		unsigned long m[128];
		unsigned long pos[128];
		bool found;

		time++;
		n = (get_random_u64() & 0x7f) + 1;
		MT_CACHE_BUS_LOG_1("n = %lld\n", (u64)n);

		count = ((0xf + 1) * (0x7f + 1)) / sizeof(*d0);
		d0 = (u8 *)up;
		memset((void *)d0, 0xab, count);

		for (i = 0; i < n; i++) {
			m[i] = (get_random_u64() & 0xf) + 1;
			pos[i] = i == 0 ? m[i] : (pos[i - 1] + m[i]);
			MT_CACHE_BUS_LOG_1("m[%d] = %lld\n", i, (u64)m[i]);
			MT_CACHE_BUS_LOG_1("pos[%d] = %lld\n", i, (u64)pos[i]);
			*(d0 + pos[i]) = 0x55;
		}

		for (i = 0; i < count; i++) {
			found = false;
			for (j = 0; j < n; j++) {
				if (i == pos[j]) {
					if (*(d0 + i) != 0x55) {
						panic("test sram %ld 1byte uncache random gap h pos failed, i = %lld\n", (ulong)sizeof(*d0), (u64)i);
					}

					found = true;
					break;
				}
			}
			if (!found) {
				if (*(d0 + i) != 0xab) {
					panic("test sram %ld 1byte uncache random gap h no-pos failed, i = %lld\n", (ulong)sizeof(*d0), (u64)i);
				}
			}
		}

		msleep(1);
	}

	MT_CACHE_BUS_LOG("\33[44;31m test sram uncache \33[0m \n");
	timeout = jiffies + TEST_LOCAL_BUS_PERIOD;
	time = 0;
	while (jiffies < timeout) {
		time++;

		count = ((SYMPHONY_BSRAM_RAM_MAPPING_SIZE / 2) - off) / sizeof(*d0);
		d0 = (u8 *)(up + off);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d0) << 3); j++) {
				*d0 = (u8)((u64)1 << j);
				if (*d0 != (u8)((u64)1 << j)) {
					panic("test sram %ld 1byte uncache failed at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)((u64)1 << j), (u64)i);
				}
			}
			d0++;
		}

		count = ((SYMPHONY_BSRAM_RAM_MAPPING_SIZE / 2) - off) / sizeof(*d1);
		d1 = (u16 *)(up + off);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d1) << 3); j++) {
				*d1 = (u16)(~((u64)1 << j));
				if (*d1 != (u16)(~((u64)1 << j))) {
					panic("test sram %ld 2byte uncache failed at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)(~((u64)1 << j)), (u64)i);
				}
			}
			d1++;
		}

		count = ((SYMPHONY_BSRAM_RAM_MAPPING_SIZE / 2) - off) / sizeof(*d2);
		d2 = (u32 *)(up + off);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d2) << 3); j++) {
				*d2 = (u32)((u64)1 << j);
				if (*d2 != (u32)((u64)1 << j)) {
					panic("test sram %ld 4byte uncache failed at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)((u64)1 << j), (u64)i);
				}
			}
			d2++;
		}

		count = ((SYMPHONY_BSRAM_RAM_MAPPING_SIZE / 2) - off) / sizeof(*d3);
		d3 = (u64 *)(up + off);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d3) << 3); j++) {
				*d3 = (u64)(~((u64)1 << j));
				if (*d3 != (u64)(~((u64)1 << j))) {
					panic("test sram %ld 8byte uncache failed at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~((u64)1 << j)), (u64)i);
				}
			}
			d3++;
		}

		count = ((SYMPHONY_BSRAM_RAM_MAPPING_SIZE / 2) - off) / sizeof(*d3);
		d3 = (u64 *)(up + off);
		for (i = 0; i < count; i++) {
			for (j = 0; j < 16; j++) {
				r = get_random_u64();
				*d3 = (u64)r;
				if (*d3 != (u64)r) {
					panic("test sram %ld 8byte random uncache failed at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)r, (u64)i);
				}
			}
			d3++;
		}

		msleep(1);
	}
	MT_CACHE_BUS_LOG("\33[44;31m test sram uncache %d times \33[0m \n", time);

	MT_CACHE_BUS_LOG("\33[44;31m test sram cache \33[0m \n");
	timeout = jiffies + TEST_LOCAL_BUS_PERIOD;
	time = 0;
	while (jiffies < timeout) {
		time++;

		count = ((SYMPHONY_BSRAM_RAM_MAPPING_SIZE / 2) - off) / sizeof(*d0);
		d0 = (u8 *)(p + off);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d0) << 3); j++) {
				*d0 = (u8)((u64)1 << j);
				if (*d0 != (u8)((u64)1 << j)) {
					panic("test sram %ld 1byte cache failed 0 at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)((u64)1 << j), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d0, (ulong)sizeof(*d0));
				flush_times++;
				if (*(u8 *)((void *)d0 - p + up) != (u8)((u64)1 << j)) {
					panic("test sram %ld 1byte cache failed 1 at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)((u64)1 << j), (u64)i);
				}
#endif
			}
			d0++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = ((SYMPHONY_BSRAM_RAM_MAPPING_SIZE / 2) - off) / sizeof(*d1);
		d1 = (u16 *)(p + off);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d1) << 3); j++) {
				*d1 = (u16)(~((u64)1 << j));
				if (*d1 != (u16)(~((u64)1 << j))) {
					panic("test sram %ld 2byte cache failed 0 at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)(~((u64)1 << j)), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d1, (ulong)sizeof(*d1));
				flush_times++;
				if (*(u16 *)((void *)d1 - p + up) != (u16)(~((u64)1 << j))) {
					panic("test sram %ld 2byte cache failed 1 at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)(~((u64)1 << j)), (u64)i);
				}
#endif
			}
			d1++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = ((SYMPHONY_BSRAM_RAM_MAPPING_SIZE / 2) - off) / sizeof(*d2);
		d2 = (u32 *)(p + off);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d2) << 3); j++) {
				*d2 = (u32)((u64)1 << j);
				if (*d2 != (u32)((u64)1 << j)) {
					panic("test sram %ld 4byte cache failed 0 at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)((u64)1 << j), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d2, (ulong)sizeof(*d2));
				flush_times++;
				if (*(u32 *)((void *)d2 - p + up) != (u32)((u64)1 << j)) {
					panic("test sram %ld 4byte cache failed 1 at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)((u64)1 << j), (u64)i);
				}
#endif
			}
			d2++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = ((SYMPHONY_BSRAM_RAM_MAPPING_SIZE / 2) - off) / sizeof(*d3);
		d3 = (u64 *)(p + off);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d3) << 3); j++) {
				*d3 = (u64)(~((u64)1 << j));
				if (*d3 != (u64)(~((u64)1 << j))) {
					panic("test sram %ld 8byte cache failed 0 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~((u64)1 << j)), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d3, (ulong)sizeof(*d3));
				flush_times++;
				if (*(u64 *)((void *)d3 - p + up) != (u64)(~((u64)1 << j))) {
					panic("test sram %ld 8byte cache failed 1 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~((u64)1 << j)), (u64)i);
				}
#endif
			}
			d3++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = ((SYMPHONY_BSRAM_RAM_MAPPING_SIZE / 2) - off) / sizeof(*d3);
		d3 = (u64 *)(p + off);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < 16; j++) {
				r = get_random_u64();
				*d3 = (u64)r;
				if (*d3 != (u64)r) {
					panic("test sram %ld 8byte random cache failed 0 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)r, (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d3, (ulong)sizeof(*d3));
				flush_times++;
				if (*(u64 *)((void *)d3 - p + up) != (u64)r) {
					panic("test sram %ld 8byte random cache failed 1 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)r, (u64)i);
				}
#endif
			}
			d3++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}
	}
	MT_CACHE_BUS_LOG("\33[44;31m test sram cache %d times \33[0m \n", time);

	iounmap(p);
#endif

	p = ioremap_cache((phys_addr_t)SYMPHONY_RECOVER_RAM_PHYS_BASE + (cpu * (SYMPHONY_RECOVER_RAM_MAPPING_SIZE >> 1)),
		(SYMPHONY_RECOVER_RAM_MAPPING_SIZE >> 1));
	up = (void *)SYMPHONY_RECOVER_RAM_VIRT_BASE + (cpu * (SYMPHONY_RECOVER_RAM_MAPPING_SIZE >> 1));

	MT_CACHE_BUS_LOG("\33[44;32m recovery ram not support unalign access \33[0m \n");
	MT_CACHE_BUS_LOG("\33[44;31m test recovery ram uncache random gap \33[0m \n");
	time = 0;
	while (time < random_gap_test_times) {
		unsigned long n;
		unsigned long m[128];
		unsigned long pos[128];
		bool found;

		time++;
		n = (get_random_u64() & 0x1f) + 1;
		MT_CACHE_BUS_LOG_1("n = %lld\n", (u64)n);

		count = ((0xf + 1) * (0x1f + 1)) / sizeof(*d0);		/* recovery ram only 1k size */
		d0 = (u8 *)up;
		memset((void *)d0, 0xab, count);

		for (i = 0; i < n; i++) {
			m[i] = (get_random_u64() & 0xf) + 1;
			pos[i] = i == 0 ? m[i] : (pos[i - 1] + m[i]);
			MT_CACHE_BUS_LOG_1("m[%d] = %lld\n", i, (u64)m[i]);
			MT_CACHE_BUS_LOG_1("pos[%d] = %lld\n", i, (u64)pos[i]);
			*(d0 + pos[i]) = 0x55;
		}

		for (i = 0; i < count; i++) {
			found = false;
			for (j = 0; j < n; j++) {
				if (i == pos[j]) {
					if (*(d0 + i) != 0x55) {
						panic("test recovery ram %ld 1byte uncache random gap h pos failed, i = %lld\n", (ulong)sizeof(*d0), (u64)i);
					}

					found = true;
					break;
				}
			}
			if (!found) {
				if (*(d0 + i) != 0xab) {
					panic("test recovery ram %ld 1byte uncache random gap h no-pos failed, i = %lld\n", (ulong)sizeof(*d0), (u64)i);
				}
			}
		}

		msleep(1);
	}

	MT_CACHE_BUS_LOG("\33[44;31m test recovery ram uncache \33[0m \n");
	timeout = jiffies + TEST_LOCAL_BUS_PERIOD;
	time = 0;
	while (jiffies < timeout) {
		time++;

		count = (SYMPHONY_RECOVER_RAM_MAPPING_SIZE / 2) / sizeof(*d0);
		d0 = (u8 *)(up);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d0) << 3); j++) {
				*d0 = (u8)((u64)1 << j);
				if (*d0 != (u8)((u64)1 << j)) {
					panic("test recovery ram %ld 1byte uncache failed at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)((u64)1 << j), (u64)i);
				}
			}
			d0++;
		}

		count = (SYMPHONY_RECOVER_RAM_MAPPING_SIZE / 2) / sizeof(*d1);
		d1 = (u16 *)(up);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d1) << 3); j++) {
				*d1 = (u16)(~((u64)1 << j));
				if (*d1 != (u16)(~((u64)1 << j))) {
					panic("test recovery ram %ld 2byte uncache failed at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)(~((u64)1 << j)), (u64)i);
				}
			}
			d1++;
		}

		count = (SYMPHONY_RECOVER_RAM_MAPPING_SIZE / 2) / sizeof(*d2);
		d2 = (u32 *)(up);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d2) << 3); j++) {
				*d2 = (u32)((u64)1 << j);
				if (*d2 != (u32)((u64)1 << j)) {
					panic("test recovery ram %ld 4byte uncache failed at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)((u64)1 << j), (u64)i);
				}
			}
			d2++;
		}

		count = (SYMPHONY_RECOVER_RAM_MAPPING_SIZE / 2) / sizeof(*d3);
		d3 = (u64 *)(up);
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d3) << 3); j++) {
				*d3 = (u64)(~((u64)1 << j));
				if (*d3 != (u64)(~((u64)1 << j))) {
					panic("test recovery ram %ld 8byte uncache failed at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~((u64)1 << j)), (u64)i);
				}
			}
			d3++;
		}

		count = (SYMPHONY_RECOVER_RAM_MAPPING_SIZE / 2) / sizeof(*d3);
		d3 = (u64 *)(up);
		for (i = 0; i < count; i++) {
			for (j = 0; j < 16; j++) {
				r = get_random_u64();
				*d3 = (u64)r;
				if (*d3 != (u64)r) {
					panic("test recovery ram %ld 8byte random uncache failed at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)r, (u64)i);
				}
			}
			d3++;
		}

		msleep(1);
	}
	MT_CACHE_BUS_LOG("\33[44;31m test recovery ram uncache %d times \33[0m \n", time);

	MT_CACHE_BUS_LOG("\33[44;31m test recovery ram cache \33[0m \n");
	timeout = jiffies + TEST_LOCAL_BUS_PERIOD;
	time = 0;
	while (jiffies < timeout) {
		time++;

		count = (SYMPHONY_RECOVER_RAM_MAPPING_SIZE / 2) / sizeof(*d0);
		d0 = (u8 *)(p);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d0) << 3); j++) {
				*d0 = (u8)((u64)1 << j);
				if (*d0 != (u8)((u64)1 << j)) {
					panic("test recovery ram %ld 1byte cache failed 0 at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)((u64)1 << j), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d0, (ulong)sizeof(*d0));
				flush_times++;
				if (*(u8 *)((void *)d0 - p + up) != (u8)((u64)1 << j)) {
					panic("test recovery ram %ld 1byte cache failed 1 at %x, i = %lld\n", (ulong)sizeof(*d0), (u8)((u64)1 << j), (u64)i);
				}
#endif
			}
			d0++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = (SYMPHONY_RECOVER_RAM_MAPPING_SIZE / 2) / sizeof(*d1);
		d1 = (u16 *)(p);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d1) << 3); j++) {
				*d1 = (u16)(~((u64)1 << j));
				if (*d1 != (u16)(~((u64)1 << j))) {
					panic("test recovery ram %ld 2byte cache failed 0 at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)(~((u64)1 << j)), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d1, (ulong)sizeof(*d1));
				flush_times++;
				if (*(u16 *)((void *)d1 - p + up) != (u16)(~((u64)1 << j))) {
					panic("test recovery ram %ld 2byte cache failed 1 at %x, i = %lld\n", (ulong)sizeof(*d1), (u16)(~((u64)1 << j)), (u64)i);
				}
#endif
			}
			d1++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = (SYMPHONY_RECOVER_RAM_MAPPING_SIZE / 2) / sizeof(*d2);
		d2 = (u32 *)(p);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d2) << 3); j++) {
				*d2 = (u32)((u64)1 << j);
				if (*d2 != (u32)((u64)1 << j)) {
					panic("test recovery ram %ld 4byte cache failed 0 at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)((u64)1 << j), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d2, (ulong)sizeof(*d2));
				flush_times++;
				if (*(u32 *)((void *)d2 - p + up) != (u32)((u64)1 << j)) {
					panic("test recovery ram %ld 4byte cache failed 1 at %x, i = %lld\n", (ulong)sizeof(*d2), (u32)((u64)1 << j), (u64)i);
				}
#endif
			}
			d2++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = (SYMPHONY_RECOVER_RAM_MAPPING_SIZE / 2) / sizeof(*d3);
		d3 = (u64 *)(p);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < (sizeof(*d3) << 3); j++) {
				*d3 = (u64)(~((u64)1 << j));
				if (*d3 != (u64)(~((u64)1 << j))) {
					panic("test recovery ram %ld 8byte cache failed 0 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~((u64)1 << j)), (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d3, (ulong)sizeof(*d3));
				flush_times++;
				if (*(u64 *)((void *)d3 - p + up) != (u64)(~((u64)1 << j))) {
					panic("test recovery ram %ld 8byte cache failed 1 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)(~((u64)1 << j)), (u64)i);
				}
#endif
			}
			d3++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}

		count = (SYMPHONY_RECOVER_RAM_MAPPING_SIZE / 2) / sizeof(*d3);
		d3 = (u64 *)(p);
		flush_times = 0;
		for (i = 0; i < count; i++) {
			for (j = 0; j < 16; j++) {
				r = get_random_u64();
				*d3 = (u64)r;
				if (*d3 != (u64)r) {
					panic("test recovery ram %ld 8byte random cache failed 0 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)r, (u64)i);
				}
#ifdef FLUSH_DCACHE
				_mt_dcache_flush((void *)d3, (ulong)sizeof(*d3));
				flush_times++;
				if (*(u64 *)((void *)d3 - p + up) != (u64)r) {
					panic("test recovery ram %ld 8byte random cache failed 1 at %llx, i = %lld\n", (ulong)sizeof(*d3), (u64)r, (u64)i);
				}
#endif
			}
			d3++;
			if (flush_times >= 16384) {
				flush_times = 0;
				msleep(1);
			}
		}
	}
	MT_CACHE_BUS_LOG("\33[44;31m test recovery ram cache %d times \33[0m \n", time);

	iounmap(p);
}
#else
static void __mt_local_bus_test(void *buf_cacheh, void *buf_uncacheh, void *phyh, void *buf_cachel, void *buf_uncachel, void *phyl, u32 cpu)
{

}
#endif

static int ___mt_dcache_api_test(struct mt_dc_test_completion *mdtc, void *phy_low, void *phy_high)
{
	int cpu = mdtc->cpu;
	void *ph = (void *)(phy_high + (TEST_BUFFER_SIZE * cpu));
	volatile int *cache_ph;
	volatile int *uncache_ph;
	void *pl = (void *)(phy_low + (TEST_BUFFER_SIZE * cpu));
	volatile int *cache_pl;
	volatile int *uncache_pl;
	int i;

	cache_ph = ioremap_cache((phys_addr_t)((ulong)ph), TEST_BUFFER_SIZE);
	uncache_ph = ioremap_wc((phys_addr_t)((ulong)ph), TEST_BUFFER_SIZE);
	cache_pl = ioremap_cache((phys_addr_t)((ulong)pl), TEST_BUFFER_SIZE);
	uncache_pl = ioremap_wc((phys_addr_t)((ulong)pl), TEST_BUFFER_SIZE);

	/*
	* note the set of local var cache_ph cache_pl and i etc, impossibility same with cache_ph[i],
	* because stack is fulldown, so the set of local var is at tail of page, and &cache_ph[i] is at head of page.
	* so the local vars will not lead to cache line exchange.
	*/

	MT_CACHE_BUS_LOG("\33[44;31m mt_dcache_api test begin \33[0m \n");
	MT_CACHE_BUS_LOG("phy_high = 0x%px\n", (void *)ph);
	MT_CACHE_BUS_LOG("cache_ph = 0x%px\n", (void *)cache_ph);
	MT_CACHE_BUS_LOG("uncache_ph = 0x%px\n", (void *)uncache_ph);
	MT_CACHE_BUS_LOG("phy_low = 0x%px\n", (void *)pl);
	MT_CACHE_BUS_LOG("cache_pl = 0x%px\n", (void *)cache_pl);
	MT_CACHE_BUS_LOG("uncache_pl = 0x%px\n", (void *)uncache_pl);

#if 0
	MT_CACHE_BUS_LOG("&cache_ph = 0x%px\n", (void *)&cache_ph);
	MT_CACHE_BUS_LOG("&uncache_ph = 0x%px\n", (void *)&uncache_ph);
	MT_CACHE_BUS_LOG("&cache_pl = 0x%px\n", (void *)&cache_pl);
	MT_CACHE_BUS_LOG("&uncache_pl = 0x%px\n", (void *)&uncache_pl);
	MT_CACHE_BUS_LOG("&i = 0x%px\n", (void *)&i);
#endif

	/*
	* disable irq and use CONFIG_PREEMPT_NONE to avoid execute flow switch,
	* for keep bus pressure highest, because the data size of interrupt handle is not big
	*/
	local_irq_disable();

	memset((void *)cache_ph, 0, TEST_BUFFER_SIZE);
	memset((void *)uncache_ph, 0, TEST_BUFFER_SIZE);
	memset((void *)cache_pl, 0xff, TEST_BUFFER_SIZE);
	memset((void *)uncache_pl, 0xff, TEST_BUFFER_SIZE);
	dsb(sy);

	for (i = 0; i < TEST_BUFFER_SIZE / sizeof(*cache_ph); i++) {
		if (cache_ph[i] != uncache_ph[i]) {
			panic("000 h %d diff\n", i);
		}
		if (cache_pl[i] != uncache_pl[i]) {
			panic("000 l %d diff\n", i);
		}
	}

	MT_CACHE_BUS_LOG("\33[44;31m test clean \33[0m \n");
	/*
	* printk will access same global var and buffer, see log_store, do some memcpy
	* so some data from ddr into loadbuffer(between ddr and cache) then to cache.
	* but the following code not depend on the data in buffer of printk flow,
	* so may be the data of printk flow not arrive cache (now in loadbuffer), but the following code run already,
	* and cache_ph[i]'s line may be already in cache, no need load from ddr, so the following code write cache_ph[i]'s line,
	* now cache_ph[i]'s line is newest, and the data of printk flow is in loadbuffer,
	* then the data of printk flow arrive cache, but may be can't found a free line, so replace the cache_ph[i]'s line,
	* so the cache_ph[i]'s line exchanged by the data of printk flow while then arrive cache.
	* so can't call printk.
	*
	* and note, dsb may be have no sync effect for loadbuffer, dsb can wait data arrive ip when write,
	* but for read, just only wait data arrive loadbuffer or bus, but not cpu general register
	*/
	for (i = 0; i < TEST_BUFFER_SIZE / sizeof(*cache_ph); i++) {
		cache_ph[i] = i + 1;
		cache_pl[i] = ~(i + 1);
	}
#ifdef DO__CANNOT_DO
	/*
	* hardware may do speculate fetch (see 406C page 1261, B2.2.2), may lead to cache line exchange.
	* even lockdown cache line, we can't make sure it not be writeback to memory by hardware automaticly (see 406C page 1262).
	*
	* 406C said:
	* An unlocked entry in the cache cannot be relied upon to remain in the cache. If an unlocked entry does remain
	* in the cache, it cannot be relied upon to remain incoherent with the rest of memory. In other words, software
	* must not assume that an unlocked item that remains in the cache remains dirty.
	* A locked entry in the cache can be relied upon to remain in the cache. A locked entry in the cache cannot be
	* relied upon to remain incoherent with the rest of memory, that is, it cannot be relied on to remain dirty.
	*
	* so can compare the following.
	*/

	/*
	* cache_ph[i] and cache_pl[i] may be in same line, so must use cache_pl[i + x],
	* note L1 dcache is vipt nonalias, the gap of cache_ph[i] and cache_pl[i + x] don't be PAGE_SIZE*N,
	* so use cache_ph[i] and cache_pl[i + (CACHE_LINE_SIZE / sizeof(*cache_ph))], keep the sets are contiguous.
	*/
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(*cache_ph); i++) {
		if (cache_ph[i] == uncache_ph[i]) {
			panic("111 h %d same\n", i);
		}
		if (cache_pl[i + (CACHE_LINE_SIZE / sizeof(*cache_ph))] == uncache_pl[i + (CACHE_LINE_SIZE / sizeof(*cache_ph))]) {
			panic("111 l %d same\n", i);
		}
	}
#endif
	_mt_dcache_clean(((void *)&cache_ph[0]), TEST_BUFFER_SIZE);
	_mt_dcache_clean(((void *)&cache_pl[0]), TEST_BUFFER_SIZE);
	for (i = 0; i < TEST_BUFFER_SIZE / sizeof(*cache_ph); i++) {
		if (cache_ph[i] != uncache_ph[i]) {
			panic("222 h %d diff\n", i);
		}
		if (cache_pl[i] != uncache_pl[i]) {
			panic("222 l %d diff\n", i);
		}
	}

	MT_CACHE_BUS_LOG("\33[44;31m test invalid \33[0m \n");
	for (i = 0; i < TEST_BUFFER_SIZE / sizeof(*cache_ph); i++) {
		cache_ph[i] = i + 2;
		cache_pl[i] = ~(i + 2);
	}
#ifdef DO__CANNOT_DO
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(*cache_ph); i++) {
		if (cache_ph[i] == uncache_ph[i]) {
			panic("333 h %d same\n", i);
		}
		if (cache_pl[i + (CACHE_LINE_SIZE / sizeof(*cache_ph))] == uncache_pl[i + (CACHE_LINE_SIZE / sizeof(*cache_ph))]) {
			panic("333 l %d same\n", i);
		}
	}
#endif
	_mt_dcache_invalid(((void *)&cache_ph[0]), TEST_BUFFER_SIZE);
	_mt_dcache_invalid(((void *)&cache_pl[0]), TEST_BUFFER_SIZE);
	for (i = 0; i < TEST_BUFFER_SIZE / sizeof(*cache_ph); i++) {
		if (cache_ph[i] != uncache_ph[i]) {
			panic("444 h %d diff\n", i);
		}
		if (cache_pl[i] != uncache_pl[i]) {
			panic("444 l %d diff\n", i);
		}
	}

	MT_CACHE_BUS_LOG("\33[44;31m test flush: clean \33[0m \n");
	for (i = 0; i < TEST_BUFFER_SIZE / sizeof(*cache_ph); i++) {
		cache_ph[i] = i + 3;
		cache_pl[i] = ~(i + 3);
	}
	_mt_dcache_flush(((void *)&cache_ph[0]), TEST_BUFFER_SIZE);
	_mt_dcache_flush(((void *)&cache_pl[0]), TEST_BUFFER_SIZE);
	for (i = 0; i < TEST_BUFFER_SIZE / sizeof(*cache_ph); i++) {
		if (cache_ph[i] != uncache_ph[i]) {
			panic("555 h %d diff\n", i);
		}
		if (cache_pl[i] != uncache_pl[i]) {
			panic("555 l %d diff\n", i);
		}
	}

	MT_CACHE_BUS_LOG("\33[44;31m test flush: invalid \33[0m \n");
	for (i = 0; i < TEST_BUFFER_SIZE / sizeof(*cache_ph); i++) {
		cache_ph[i] = i + 4;
		cache_pl[i] = ~(i + 4);
	}
	_mt_dcache_flush(((void *)&cache_ph[0]), TEST_BUFFER_SIZE);
	_mt_dcache_flush(((void *)&cache_pl[0]), TEST_BUFFER_SIZE);
	for (i = 0; i < TEST_BUFFER_SIZE / sizeof(*cache_ph); i++) {
		uncache_ph[i] = i + 5;
		uncache_pl[i] = ~(i + 5);
	}
	dsb(sy);
	for (i = 0; i < TEST_BUFFER_SIZE / sizeof(*cache_ph); i++) {
		if (cache_ph[i] != uncache_ph[i]) {
			panic("666 h %d diff\n", i);
		}
		if (cache_pl[i] != uncache_pl[i]) {
			panic("666 l %d diff\n", i);
		}
	}

	local_irq_enable();

	MT_CACHE_BUS_LOG("\33[44;31m mt_dcache_api test done \33[0m \n");

	__mt_local_bus_test((void *)cache_ph, (void *)uncache_ph, ph, (void *)cache_pl, (void *)uncache_pl, pl, mdtc->cpu);

	iounmap(cache_ph);
	iounmap(uncache_ph);
	iounmap(cache_pl);
	iounmap(uncache_pl);

	return 0;
}

#define BUS_TEST_BUFFER_MAX 1024
static int bus_test_buffer_cnt = 0;
static void *bus_test_buffer_addr[BUS_TEST_BUFFER_MAX];

static int __init bus_test_buffer(char *p)
{
	char *endp;
	u64 addr;

	if (bus_test_buffer_cnt >= BUS_TEST_BUFFER_MAX) {
		pr_err("bus_test=addr1_end,addr2_end more than %d\n", BUS_TEST_BUFFER_MAX / 2);
		return 0;
	}

	addr = memparse(p, &endp);
	pr_info("bus_test addr %d at: %px\n", bus_test_buffer_cnt, (void *)addr);
	bus_test_buffer_addr[bus_test_buffer_cnt++] = (void *)addr;
	addr = memparse(endp + 1, NULL);
	pr_info("bus_test addr %d at: %px\n", bus_test_buffer_cnt, (void *)addr);
	bus_test_buffer_addr[bus_test_buffer_cnt++] = (void *)addr;

	return 0;
}
early_param("bus_test", bus_test_buffer);

#else /* CONFIG_MIPS */
#define TEST_SIZE (CACHE_LINE_SIZE << 2)

static int ___mt_dcache_api_test(void *param)
{
	int i;
	static int test_buffer[TEST_SIZE] CACHE_ALIGN;
#ifdef MT_CACHE_BUS_LOG_ENABLE
	int cpu = 0;
#endif

	MT_CACHE_BUS_LOG("\33[44;31m mt_dcache_api test begin \33[0m \n");

	memset((void *)test_buffer, 0, sizeof(test_buffer));
	memset((void *)(UNCAC_ADDR((unsigned int)(void *)test_buffer)), 0, sizeof(test_buffer));

	local_irq_disable();

	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		if (test_buffer[i] != *(int *)(UNCAC_ADDR((unsigned int)((void *)&test_buffer[i])))) {
			panic("000 %d diff\n", i);
		}
	}

	MT_CACHE_BUS_LOG("\33[44;31m test clean \33[0m \n");
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		test_buffer[i] = i + 1;
	}
#ifdef DO__CANNOT_DO
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		if (test_buffer[i] == *(int *)(UNCAC_ADDR((unsigned int)((void *)&test_buffer[i])))) {
			panic("111 %d same\n", i);
		}
	}
#endif
	mt_dcache_clean(((void *)&test_buffer[0]), CACHE_LINE_SIZE);
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		if (test_buffer[i] != *(int *)(UNCAC_ADDR((unsigned int)((void *)&test_buffer[i])))) {
			panic("222 %d diff\n", i);
		}
	}

	MT_CACHE_BUS_LOG("\33[44;31m test invalid \33[0m \n");
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		test_buffer[i] = i + 2;
	}
#ifdef DO__CANNOT_DO
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		if (test_buffer[i] == *(int *)(UNCAC_ADDR((unsigned int)((void *)&test_buffer[i])))) {
			panic("333 %d same\n", i);
		}
	}
#endif
	mt_dcache_invalid(((void *)&test_buffer[0]), CACHE_LINE_SIZE);
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		if (test_buffer[i] != *(int *)(UNCAC_ADDR((unsigned int)((void *)&test_buffer[i])))) {
			panic("444 %d diff\n", i);
		}
	}

	MT_CACHE_BUS_LOG("\33[44;31m test flush: clean \33[0m \n");
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		test_buffer[i] = i + 3;
	}
	mt_dcache_flush(((void *)&test_buffer[0]), CACHE_LINE_SIZE);
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		if (test_buffer[i] != *(int *)(UNCAC_ADDR((unsigned int)((void *)&test_buffer[i])))) {
			panic("555 %d diff\n", i);
		}
	}

	MT_CACHE_BUS_LOG("\33[44;31m test flush: invalid \33[0m \n");
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		test_buffer[i] = i + 4;
	}
	mt_dcache_flush(((void *)&test_buffer[0]), CACHE_LINE_SIZE);
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		*(int *)(UNCAC_ADDR((unsigned int)((void *)&test_buffer[i]))) = i + 5;
	}
	for (i = 0; i < CACHE_LINE_SIZE / sizeof(test_buffer[0]); i++) {
		if (test_buffer[i] != *(int *)(UNCAC_ADDR((unsigned int)((void *)&test_buffer[i])))) {
			panic("666 %d diff\n", i);
		}
	}

	local_irq_enable();

	MT_CACHE_BUS_LOG("\33[44;31m mt_dcache_api test done \33[0m \n");

	return 0;
}
#endif

static int __mt_dcache_api_test(void *param)
{
	struct mt_dc_test_completion *mdtc = (struct mt_dc_test_completion *)param;
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	int i;
#endif

	while (1) {
		wait_for_completion(&mdtc->kernel_completion);

		MT_CACHE_BUS_LOG("\33[44;31m cache and bus test start \33[0m \n");

		/* the address see sample/dvfs-avs/readme.txt */
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
		MT_CACHE_BUS_LOG("bus_test_buffer_cnt = %d\n", bus_test_buffer_cnt);
		for (i = 0; i < bus_test_buffer_cnt; i += 2) {
			___mt_dcache_api_test(mdtc, bus_test_buffer_addr[i], bus_test_buffer_addr[i + 1]);
			MT_CACHE_BUS_LOG("\33[44;31m cache and bus test done %d \33[0m \n\n", (i / 2) + 1);
			msleep(1);
		}
#else
		___mt_dcache_api_test(param);
#endif
		MT_CACHE_BUS_LOG("\33[44;31m cache and bus test ok \33[0m \n");

		complete(&mdtc->user_completion);
	}

	return 0;
}

static int cache_and_bus_test_dbgfs_open(struct inode *inode, struct file *file)
{
	simple_open(inode, file);
	return 0;
}

static ssize_t cache_and_bus_test_dbgfs_read(struct file *file, char __user *buf, size_t cnt, loff_t *postion)
{
	unsigned long rest;
	unsigned char kbuf[512];
	u32 param_cnt = 0;

	if (*postion) {
		return 0;
	}

	param_cnt += snprintf(kbuf + param_cnt, sizeof(kbuf) - param_cnt, "%s",
		"echo N > /sys/kernel/debug/cache_and_bus_test, N is number of random gap test, don't run any other process/thread for keep bus pressure high\n");
	param_cnt += snprintf(kbuf + param_cnt, sizeof(kbuf) - param_cnt, "now N is %d\n", random_gap_test_times);
	param_cnt += snprintf(kbuf + param_cnt, sizeof(kbuf) - param_cnt, "cache and bus test is %s now\n", test_run ? "runing" : "stopped");

	rest = copy_to_user(buf, kbuf, cnt > param_cnt ? param_cnt : cnt);

	if (rest == -EFAULT) {
		return -EFAULT;
	}

	*postion += (param_cnt - rest);

	return param_cnt - rest;
}

static ssize_t cache_and_bus_test_dbgfs_write(struct file *file, const char __user *buf, size_t cnt, loff_t *postion)
{
	unsigned long size;
	unsigned long rest;
	char *endp;
	unsigned char kbuf[256];

	if (!cache_and_bus_test_enable) {
		return cnt;
	}

	if (cnt < 2) {
		return cnt;
	}

	memset(kbuf, '\0', sizeof(kbuf));
	size = (unsigned long)cnt > (sizeof(kbuf) - 1) ? (sizeof(kbuf) - 1) : cnt;
	rest = copy_from_user((void *)kbuf, (void *)buf, size);
	if (rest == -EFAULT) {
		return -EFAULT;
	}

	endp = kbuf;
	random_gap_test_times = simple_strtoul(endp, &endp, 0);

	if (random_gap_test_times > 0) {
		if ((dvfs_test_percpu == -1) || (dvfs_test_percpu == 0)) {
			complete(&mdtc0.kernel_completion);
		}
#ifdef CONFIG_SMP
		if ((dvfs_test_percpu == -1) || (dvfs_test_percpu == 1)) {
			complete(&mdtc1.kernel_completion);
		}
#endif
		test_run = true;

		if ((dvfs_test_percpu == -1) || (dvfs_test_percpu == 0)) {
			wait_for_completion(&mdtc0.user_completion);
		}
#ifdef CONFIG_SMP
		if ((dvfs_test_percpu == -1) || (dvfs_test_percpu == 1)) {
			wait_for_completion(&mdtc1.user_completion);
		}
#endif
		test_run = false;
	}

	*postion += (cnt - rest);

	return cnt - rest;
}

static struct file_operations cache_and_bus_test_dbgfs_fops = {
	.owner = THIS_MODULE,
	.open = cache_and_bus_test_dbgfs_open,
	.read = cache_and_bus_test_dbgfs_read,
	.write = cache_and_bus_test_dbgfs_write,
	.llseek = default_llseek,
};

static void _mt_dcache_api_test(void)
{
	int cpu;

	if ((dvfs_test_percpu == -1) || (dvfs_test_percpu == 0)) {
		init_completion(&mdtc0.kernel_completion);
		init_completion(&mdtc0.user_completion);
	}
	mdtc0.cpu = 0;
#ifdef CONFIG_SMP
	if ((dvfs_test_percpu == -1) || (dvfs_test_percpu == 1)) {
		init_completion(&mdtc1.kernel_completion);
		init_completion(&mdtc1.user_completion);
	}
	mdtc1.cpu = 1;
#endif

	debugfs_create_ulong("cache_and_bus_test_enable", S_IRUSR | S_IWUSR | S_IRUGO,
			NULL, &cache_and_bus_test_enable);

	debugfs_create_file("cache_and_bus_test", S_IRUSR | S_IWUSR | S_IRUGO,
		NULL, NULL, &cache_and_bus_test_dbgfs_fops);

	for_each_online_cpu(cpu) {
		if ((dvfs_test_percpu != -1) && (cpu != dvfs_test_percpu)) {
			continue;
		}
		if (cpu == 0) {
			dcache_test_task0 = kthread_create(__mt_dcache_api_test, (void *)&mdtc0, "mt_dc_test0");
			kthread_bind(dcache_test_task0, 0);
			wake_up_process(dcache_test_task0);
		}
#ifdef CONFIG_SMP
		else if (cpu == 1) {
			dcache_test_task1 = kthread_create(__mt_dcache_api_test, (void *)&mdtc1, "mt_dc_test1");
			kthread_bind(dcache_test_task1, 1);
			wake_up_process(dcache_test_task1);
		}
#endif
	}
}

static int __init cache_bus_test_isolcpus_setup(char *str)
{
	dvfs_test_percpu = simple_strtoul(str, NULL, 0);
	return 1;
}
__setup("dvfs_test_percpu=", cache_bus_test_isolcpus_setup);

#else
static void _mt_dcache_api_test(void)
{

}
#endif

void mt_dcache_line_size_init(void)
{
	if (CACHE_LINE_SIZE != (ulong)cpu_dcache_line_size()) {
		printk("CACHE_LINE_SIZE = %ld\n", CACHE_LINE_SIZE);
		printk("cpu_dcache_line_size() = %d\n", cpu_dcache_line_size());
		panic("CACHE_LINE_SIZE != cpu_dcache_line_size()\n");
	}

	mt_dcache_flush_all();

	_mt_dcache_api_test();
}

#ifdef CONFIG_ARM64
static void mt_uaccess_enable(void)
{
	uaccess_ttbr0_enable();
}

static void mt_uaccess_disable(void)
{
	uaccess_ttbr0_disable();
}
#else
static void mt_uaccess_enable(void)
{

}

static void mt_uaccess_disable(void)
{

}
#endif

static void check_aligned(void *vaddr, size_t bytes)
{
#if 0
	if ((ulong)vaddr & (CACHE_LINE_SIZE - 1)) {
		printk(KERN_WARNING "\33[44;31m start: 0x%lx not aligned with %ld \33[0m \n", (ulong)vaddr, CACHE_LINE_SIZE);
		WARN_ON(1);
	}
	if ((ulong)(vaddr + bytes) & (CACHE_LINE_SIZE - 1)) {
		printk(KERN_WARNING "\33[44;31m stop: 0x%lx not aligned with %ld \33[0m \n", (ulong)(vaddr + bytes), CACHE_LINE_SIZE);
		WARN_ON(1);
	}
#endif
}

static void _mt_dcache_clean(void *vaddr, ulong bytes)
{
#ifdef CONFIG_ARM
	_mt_arm_dcache_clean(vaddr, bytes);
#elif defined(CONFIG_ARM64)
	dcache_clean_poc((ulong)vaddr, (ulong)vaddr + bytes);
#else /* CONFIG_MIPS */
	dma_cache_wback(vaddr, bytes);
#endif
}

static void _mt_dcache_invalid(void *vaddr, ulong bytes)
{
#ifdef CONFIG_ARM
	_mt_arm_dcache_invalid(vaddr, bytes);
#elif defined(CONFIG_ARM64)
	dcache_inval_poc((ulong)vaddr, (ulong)vaddr + bytes);
#else /* CONFIG_MIPS */
	dma_cache_inv(vaddr, bytes);
#endif
}

static void _mt_dcache_flush(void *vaddr, ulong bytes)
{
#ifdef CONFIG_ARM
	dmac_flush_range(vaddr, vaddr + bytes);
#elif defined(CONFIG_ARM64)
	dcache_clean_inval_poc((ulong)vaddr, (ulong)vaddr + bytes);
#else /* CONFIG_MIPS */
	dma_cache_wback_inv(vaddr, bytes);
#endif
}

void mt_user_dcache_clean(void *vaddr, ulong offset, ulong bytes)
{
	check_aligned(vaddr + offset, bytes);
	mt_uaccess_enable();
	_mt_dcache_clean(vaddr + offset, bytes);
	mt_uaccess_disable();
}

void mt_user_dcache_invalid(void *vaddr, ulong offset, ulong bytes)
{
	check_aligned(vaddr + offset, bytes);
	mt_uaccess_enable();
#ifdef CONFIG_ARM64
	/*
	* armv8 DC IVAC may be generate a permission fault
	* DC CIVAC and DC IVAC all may be generate translation fault
	* but DC CIVAC not generate permission fault
	* we can replace _mt_dcache_invalid by _mt_dcache_flush
	* the reason you can see DDI0487D_b_armv8_arm.pdf page 448:
	* When executed at EL1, a  DC IVAC instruction performs a clean and invalidate, meaning it performs the same
	* maintenance as a  DC CIVAC instruction, if all of the following apply:
	* ? EL2 is implemented and enabled in the current Security state.
	* ? The value of HCR_EL2.VM is 1, meaning EL1&0 stage two address translation is enabled.
	*/
	_mt_dcache_flush(vaddr + offset, bytes);
#else
	_mt_dcache_invalid(vaddr + offset, bytes);
#endif
	mt_uaccess_disable();
}

void mt_user_dcache_flush(void *vaddr, ulong offset, ulong bytes)
{
	check_aligned(vaddr + offset, bytes);
	mt_uaccess_enable();
	_mt_dcache_flush(vaddr + offset, bytes);
	mt_uaccess_disable();
}

void mt_dcache_clean(void *vaddr, ulong bytes)
{
	check_aligned(vaddr, bytes);
	_mt_dcache_clean(vaddr, bytes);
}
EXPORT_SYMBOL(mt_dcache_clean);

void mt_dcache_invalid(void *vaddr, ulong bytes)
{
	check_aligned(vaddr, bytes);
	_mt_dcache_invalid(vaddr, bytes);
}
EXPORT_SYMBOL(mt_dcache_invalid);

void mt_dcache_flush(void *vaddr, ulong bytes)
{
	check_aligned(vaddr, bytes);
	_mt_dcache_flush(vaddr, bytes);
}
EXPORT_SYMBOL(mt_dcache_flush);

#ifdef CONFIG_SMP
static void mt_flush_cache_all(void *info)
{
	flush_cache_all();
}
#endif

void mt_dcache_flush_all(void)
{
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	/* flush inner all cache */
#ifdef CONFIG_SMP
	/*
	* flush all base on set/way, not broadcast
	* flush addr base on virtual address, will be broadcast
	* see DEN0024A_v8_architecture_PG.pdf, page 214, table 14-1
	* see DEN0013D_cortex_a_series_PG.pdf, page 252, only refer to by virtual address, not refer to set/way
	*/
	on_each_cpu(mt_flush_cache_all, NULL, 1);
#else
	flush_cache_all();
#endif
#else /* CONFIG_MIPS */
	__flush_cache_all();
#endif
}
EXPORT_SYMBOL(mt_dcache_flush_all);

/* reference persistent_ram_vmap */
void *mt_remap_mmz_k(phys_addr_t paddr, ulong size, bool cached)
{
	pgprot_t prot;
	struct page *page = phys_to_page(paddr);
	struct page **pages;
	unsigned int count = PAGE_ALIGN(size) >> PAGE_SHIFT;
	void *vaddr;
	int i;

	pages = kmalloc_array(count, sizeof(struct page *), GFP_KERNEL);
	if (!pages) {
		return NULL;
	}

	for (i = 0; i < count; i++) {
		pages[i] = nth_page(page, i);
	}

	if (cached) {
		prot = PAGE_KERNEL;
	} else {
		prot = pgprot_writecombine(PAGE_KERNEL);
	}

	vaddr = vmap(pages, count, VM_MAP, prot);
	kfree(pages);

	return vaddr;
}
EXPORT_SYMBOL(mt_remap_mmz_k);

/* reference persistent_ram_free */
void mt_unmap_mmz_k(void *vaddr)
{
	vunmap(vaddr);
}

pgprot_t mt_mmz_noncached_pgprot(pgprot_t prot)
{
	prot = pgprot_writecombine(prot);

	return prot;
}
