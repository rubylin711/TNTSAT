/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// Configurations
//
///////////////////////////////////////////////////////////////////////////////

/* Test in Linux? */
/*#undef __LINUX__*/
#define __LINUX__

#ifdef __LINUX__

/* dump to usb file while error */
//#define CFG_HAS_DUMP_FILE

#else

/*#define CONFIG_ARM*/
#define CONFIG_MIPS

/*#define CONFIG_MT_CHIP_SYMPHONY4*/
#define CONFIG_MT_CHIP_SYMPHONY2

/* test physical address: 4M */
#define CFG_TEST_PHY_ADDR	0x00400000UL

#endif

#define CFG_TEST_SIZE		0x02800000
#define CFG_TEST_LOOPS		0xFFFFFFFF

/* fixed test dma channel */
#define DMA_CHANNEL			4

///////////////////////////////////////////////////////////////////////////////

#ifdef __LINUX__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mt_type.h"
#include "mt_common.h"
#include "mpi_memdev.h"
#else
/* btinit*/
#include "../include/config.h"
#include "../include/common.h"
#include "../include/printk.h"

#ifdef CONFIG_ARM
#define __iowmb() asm volatile ("dsb st" : : : "memory")
#define __iormb() asm volatile ("dsb" : : : "memory")
#else /* CONFIG_MIPS */
#define __iowmb() asm volatile ("" : : : "memory")
#define __iormb() asm volatile ("" : : : "memory")
#endif

#endif

///////////////////////////////////////////////////////////////////////////////
//porting begin

#define PHY_DEBUG(...)		do{}while(0)
#ifdef __LINUX__
#define PHY_INFO			printf
#define PRINTF				printf
#else
#define PHY_INFO			sram_printk1
#define PRINTF				sram_printk1
#endif

//#define CPU_HANG()			do{}while(1)
#define CPU_HANG()			do{}while(0)

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#define REG_DMA_BASE		0xBF400000UL
#else
//sym2
#ifdef __LINUX__
#define REG_DMA_BASE		0x1F400000
#else
#define REG_DMA_BASE		0xBF400000
#endif
#endif

/* dma channel offset */
#define DMA_CH_OFF(ch)		((ch)*0x40)

#ifdef CFG_HAS_DUMP_FILE
#define DUMP_FILE			"/tmp/media/sda1/dump_mmtest_hard.hex"
#define DUMP_FILE2			"/media/sda1/dump_mmtest_hard.hex"
#endif

typedef unsigned int uint32_t;

static uint32_t randx;
static unsigned long DMA_BASE;
static unsigned long real_phyaddr;


static unsigned long test_start_addr = 0;
static unsigned long test_size = 0;
static long total_failed = 0;
static long dma_error_count = 0;
static long cpu_error_count = 0;

#if defined(__LINUX__) && defined(CFG_HAS_DUMP_FILE)
static int opt_dump_file = 0;
#endif

void read_temp(void);
uint32_t m_rand(void);
int ddr_dma_info(unsigned long start_addr, uint32_t len);
int memtest_hard(unsigned long physaddr, unsigned int mmsize);

#ifndef __LINUX__
int mmtest_hard_main(int argc, const char *argv[]);
#endif

static inline uint32_t rd_mem(unsigned long addr)
{
	return *(volatile uint32_t *)addr;
}

static inline void wr_mem(unsigned long addr, uint32_t val)
{
	*(volatile uint32_t *)addr = val;
}

static inline uint32_t rd_reg(unsigned long addr)
{
	uint32_t val;

	val = *(volatile uint32_t *)addr;
#if defined(CONFIG_MT_ARCH_AARCH64)
	__iormb((unsigned long)val);
#else
	__iormb();
#endif

	return val;
}

static inline void wr_reg(unsigned long addr, uint32_t val)
{
	__iowmb();
	*(volatile uint32_t *)addr = val;
}

static void dump_hex_aligned16(unsigned char *data, unsigned long size)
{
	int i;

	//aligned 16
	data = (unsigned char *)(((unsigned long)data) & (~(15UL)));

	if ((unsigned long)data < test_start_addr)
		data = (unsigned char *)test_start_addr;

	if ((unsigned long)data + size > test_start_addr + test_size)
		size = test_start_addr + test_size - (unsigned long)data;

	for (i=0; i<size; i+=4)
	{
		if ((i%16) == 0)
		{
			PRINTF("\r\n%08lX:", (unsigned long)(data+i));
		}

		PRINTF(" %08X", *(volatile unsigned int*)(data+i));
	}

	PRINTF("\r\n");
}

#ifdef CFG_HAS_DUMP_FILE
static unsigned long dump_file(void *buf, unsigned long size)
{
	FILE *fp;
	size_t bytes;
	const char *fname;

	fname = DUMP_FILE;
	fp = fopen(fname, "wb");
	if (fp == NULL)
	{
		//retry
		fname = DUMP_FILE2;
		fp = fopen(DUMP_FILE2, "wb");
		if (fp == NULL)
		{
			PRINTF("Error: open file %s failed!\r\n", fname);
			return 0;
		}
	}

	bytes = fwrite(buf, sizeof(char), (size_t)size, fp);
	PRINTF("dump %s: %p, 0x%lx, return 0x%x\r\n",
			fname, buf, size, bytes);

	fflush(fp);

	fclose(fp);

	return bytes;
}
#endif

//porting end
///////////////////////////////////////////////////////////////////////////////

#if defined(__LINUX__) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
static const unsigned long ADDR_NO_CACHE = 0x0;  		//sym4 ddr addr, sym2??
#else
static const uint32_t ADDR_NO_CACHE = 0xA0000000;  	//sym2 uncached virtual ddr addr
#endif

int memtest_hard(unsigned long physaddr, unsigned int mmsize)
{
	int ret = 0;
    uint32_t i, lenth, tmp1, tmp2;
    unsigned long j;

    lenth = mmsize * 1024 * 1024;  //mmsize=Mbyte
    i = 10;
    PHY_INFO(" * memtest_hard start, start_addr = 0x%lx, size=0x%x\n", physaddr, lenth >> 1);
    read_temp();
    while (i--)
    {

        tmp1 = (uint32_t)ddr_dma_info(physaddr , lenth >> 1);
        if (tmp1 != 0)
        {
            PHY_INFO("\n memtest_hard FAILURE:rst = 0x%x \r\n", tmp1);
//+++
#ifdef CFG_HAS_DUMP_FILE
			if (opt_dump_file)
			{
            	dump_file((void*)physaddr, (unsigned long)lenth);
            }
#endif
            ret = -1;

            CPU_HANG();
        }
        else
        {
            PHY_INFO(".");
//+++
#ifdef __LINUX__
            fflush(stdout);
#endif

            for(j = (physaddr);  j < (physaddr + lenth); j = j + 4)
            {
                wr_mem(j , (uint32_t)j);
            }
            for(j = physaddr;  j < (physaddr + lenth); j = j + 4)
            {
                tmp2 = rd_mem(j);
                if((uint32_t)j != tmp2)
                {
                	cpu_error_count ++;

                    PHY_INFO("\n memtest_easy FAILURE:0x%lx != 0x%8x , 2nd read= 0x%8x\r\n", j, tmp2, rd_mem(j));
//+++
					/* read retry */
					for (int retry=0; retry<32; retry++)
					{
						PHY_INFO("Re[%d]: [0x%lx] = 0x%08x\r\n",
								retry,
								j-physaddr+real_phyaddr, rd_mem(j));
					}

					dump_hex_aligned16((unsigned char*)(j-32), 96);
//+++
#ifdef CFG_HAS_DUMP_FILE
					if (opt_dump_file)
					{
						dump_file((void*)physaddr, (unsigned long)lenth);
					}
#endif
					ret = -1;

					CPU_HANG();
                }
            }
        }
    }
    PHY_INFO("\n * memtest_hard block test %s\n", (ret==0)?"pass":"fail");
    return ret;
}

/* patterns of generating data */

/* frome mmtest_hard_20210831.c */
static uint32_t generate_pattern_v20210831(uint32_t i)
{
	uint32_t d1;

	if (i < 0x10)
	{
		d1 = (0xffff0000 | (0x01U << i));
	}
	else if ((i > 0xc) && (i < 0x20))
	{
		d1 = 0x5A5AA5A5;
	}
	else if ((i > 0x1c) && (i < 0x30))
	{
		d1 = (0xffff0000 & (~(0x01U << (i - 13))));
	}
	else if ((i > 0x3c) && (i < 0x50))
	{
		d1 = (0x0000ffff);
	}
#if 1
	else if ((i > 0x5c) && (i < 0x70))
	{
		d1 = (0x0000ffff);
	}
#endif
	else if (i % 16 == 8)
	{
		d1 = 0x12345678;
	}
	else if (i % 16 == 12)
	{
		d1 = 0x87654321;
	}
	else
	{
		d1 = m_rand();
	}

	return d1;
}

/* frome mmtest_hard_20210907.c */
static uint32_t generate_pattern_v20210907(uint32_t i)
{
	uint32_t d1;

	if (i < 0x10)
	{
		d1 = (0x0000ffff);	 //dq wave 01010101  pattern
	}
	else if(i == 0x10) d1 = 0x0;
	else if(i == 0x14) d1 = 0xffff0000;   //dq wave 00000100  pattern
	else if(i == 0x18) d1 = 0x0;
	else if(i == 0x1c) d1 = 0x0;
	else if ((i > 0x1c) && (i < 0x30))
	{
		d1 = (0xffff0000);	 //dq wave 10101010  pattern
	}
	else if(i == 0x30) d1 = 0xffffffff;
	else if(i == 0x34) d1 = 0xffffffff;
	else if(i == 0x38) d1 = 0xffff0000;   //dq wave 11101111  pattern
	else if(i == 0x3c) d1 = 0xffffffff;
	else if ((i > 0x3c) && (i < 0x50))
	{
		d1 = (0x5a5aa5a5);
	}
	else if ((i > 0x4c) && (i < 0x60))
	{
		d1 = (0x12345678);
	}
	else
	{
		d1 = m_rand();
	}

	return d1;
}

//#define PATTERN			"v20210831"
//#define GEN_PATTERN		generate_pattern_v20210831
#define PATTERN			"v20210907"
#define GEN_PATTERN		generate_pattern_v20210907

int ddr_dma_info(unsigned long start_addr, uint32_t len)
{
    uint32_t i = 0, curr, rst = 0, randx_cp, rst1 = 0, rst2 = 0;
    unsigned long addr;

    unsigned long src_addr = ADDR_NO_CACHE | start_addr;
    unsigned long dst_addr = (ADDR_NO_CACHE | start_addr) + len;
    uint32_t length   = len ;  //byte
    uint32_t d1, d3;
//+++
    uint32_t counter;

    randx_cp = randx;
    for (i = 0; i < length; i += 4)
    {
        if (1)
        {
            addr = src_addr + i;

            d1 = GEN_PATTERN(i);

            //~ PHY_INFO(" >> ddr.DMA write randx=0x%8x, write_scr 0x%8x=0x%8x \n",randx, addr, d1);
        }
        else
        {
            addr = src_addr + i;
            d1 = m_rand();
        }

        wr_mem(addr, d1);
        //~ PHY_INFO(" >> ddr.DMA write randx=0x%8x, write_scr 0x%8x=0x%8x \n",randx, addr, d1);
    }

    /* dma config */
//---
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#ifdef __LINUX__
	mt_sys_write_register(0xBF508300UL, 0x000001ff);
#else
    wr_reg(0xBF508300, 0x000001ff);  //enable edma clk
#endif
#endif
    //~ abl2_udelay(1);
    #if 0
    wr(DMA_BASE + 0x3b8, 0x00ffffff); //dma mask cpu init
    wr(DMA_BASE + 0x3bc, 0x00ffffff); //dma s cpu init
    wr(DMA_BASE + 0x3c0, 0x00ffffff); //dma sc cpu init
    #endif
//---
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    wr_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x80, 0x0);
#endif

//+++
//    wr_reg(DMA_BASE + 0x0084, src_addr);
//    wr_reg(DMA_BASE + 0x0088, dst_addr);
	wr_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x0084, (uint32_t)real_phyaddr);
	wr_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x0088, (uint32_t)(real_phyaddr + len));

//+++
//    wr_reg(DMA_BASE + 0x008C, 0x04000000);
//    wr_reg(DMA_BASE + 0x0090, 0x04000000);
    wr_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x008C, 0x07000000);
    wr_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x0090, 0x07000000);

    wr_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x0094, 0xff003f44);
    wr_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x0098, length);
//+++
//    wr_reg(DMA_BASE + 0x00a0, 0x5a5a5a5a);
    wr_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x00a0, 0x5a5a5a5a);

    curr = rd_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x009C);
    curr = curr & 0xCFFFFFFF;
    curr = curr | 0x1011;
    wr_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x009c, curr);

    /* dma polling */
    curr = 0x1;
//+++
    counter = 0;
    do
    {
        curr = rd_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x00A4) & 0x1;

//+++
        counter ++;

        if ((counter % 1000000) == 0)
        {
        	PHY_INFO("\r\nwait dma %u", counter);
#ifdef __LINUX__
        	fflush(stdout);
#endif

			//Patch
			uint32_t tmp = rd_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x009C);
			tmp = tmp & ~(0x01U << 12);
			wr_reg(DMA_BASE + DMA_CH_OFF(DMA_CHANNEL) + 0x009c, tmp);
        }
    }
    while (curr == 0x1);

    randx = randx_cp;
    for (i = 0; i < length; i += 4)
    {
        if (1)
        {
            addr = src_addr + i;

            d1 = GEN_PATTERN(i);
        }
        else
        {
            addr = src_addr + i;
            d1 = m_rand();
        }

//---
        /* wr(addr + 0x200000, d1); */

        d3 = rd_mem(dst_addr + i);

        //~ PHY_INFO(" >> ddr.DMA write data=0x%8x, read_dst 0x%8x=0x%8x,randx=0x%8x\n",d1, dst_addr + i, d3, randx);
        PHY_DEBUG(" >> ddr.DMA read 0x%lx=0x%x\n", addr, d3);

        rst2 = d1 ^ d3;
        if (0)
        {
            if (rst2)
                PHY_INFO("\nEr 0x%8x org_dat 0x%8x 1rd_dst 0x%lx = 0x%8x 2rd_dst= 0x%8x 1rd_src= 0x%8x 2rd_src= 0x%8x",
                         rst2, d1, dst_addr + i, d3, rd_mem(dst_addr + i),
                         rd_mem(dst_addr - 0x100000 + i), rd_mem(dst_addr - 0x100000 + i));
        }
        else
        {
            if (rst2)
            {
            	dma_error_count ++;

                PHY_INFO("\nEr 0x%8x org_dat 0x%8x rd_dst 0x%lx = 0x%8x rd_dst= 0x%8x rd_src= 0x%8x\r\n",
                         rst2, d1, dst_addr + i, d3, rd_mem(dst_addr + i),
                         rd_mem(dst_addr - len + i));
                //+++
                /* read retry */
                for (int retry=0; retry<32; retry++)
                {
                	PHY_INFO("Re[%d]: Dat 0x%08x, Off 0x%x, Src[0x%lx] = 0x%08x, Dst[0x%lx] = 0x%08x\r\n",
                			retry,
                			d1, i,
                			real_phyaddr + i, rd_mem(src_addr + i),
                			real_phyaddr + len + i, rd_mem(dst_addr + i));
                }

                dump_hex_aligned16((unsigned char*)(src_addr + i - 32), 96);
                dump_hex_aligned16((unsigned char*)(dst_addr + i - 32), 96);

                //~ while(1);
            }
        }
        rst1 |= rst2;

    }

    /*perf_timer_count();*/
    if (rst1 & 0x000F000F)
        rst |= 1 << 0;
    if (rst1 & 0x00F000F0)
        rst |= 1 << 1;
    if (rst1 & 0x0F000F00)
        rst |= 1 << 2;
    if (rst1 & 0xF000F000)
        rst |= 1 << 3;
    //~ PHY_INFO(" >> ddr.DMA write data=0x%8x, read_dst 0x%8x=0x%8x,randx=0x%8x,rst1=0x%x,rst=0x%x\n",d1, dst_addr + i, d3, randx,rst1, rst);
    return (int)rst;
}


uint32_t m_rand(void)
{
    randx = randx * 0x41C64E6D + 0x6073;
    return randx;
}



void read_temp(void)
{
//---
/*
    uint32_t data;
    uint32_t tmp;

    data = rd(0xbf5d00f0);
    data = data & 0x1FFF;
    //~ tmp = (761.73 * data)/8192 - 290;
    tmp = (762 * data) / 8192 - 290;
    //~ PHY_INFO(" * tmpe_code=%x\n", data);
    PHY_INFO(" * tmperature=%d\n", tmp);
*/
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Usage:
 *    sample_mmtest_hard [size] [loops] [dump file] - for linux
 *    mmtest_hard_main - for non-linux
 */
#ifdef __LINUX__
int main(int argc, char *argv[])
#else
int mmtest_hard_main(int argc, const char *argv[])
#endif
{
	int ret = 0;
	unsigned long phyaddr;
	void *viraddr;
	unsigned long size = CFG_TEST_SIZE;
	unsigned long loops = CFG_TEST_LOOPS;
	unsigned long n = 0;
#if 0
	uint32_t phyaddr_bk;
	void *viraddr_bk;
	unsigned long offset;
#endif

/* LINUX */
#ifdef __LINUX__
	if (argc < 2)
	{
		PRINTF("Usage:\n");
		PRINTF("\tsample_mmtest_hard [size] [loops] [dump file]\n");
		return 0;
	}

	if (argc >= 2)
	{
		size = strtoul(argv[1], NULL, 0);
	}

	if (argc >= 3)
	{
		loops = strtoul(argv[2], NULL, 0);
	}

#ifdef CFG_HAS_DUMP_FILE
	if (argc >= 4)
	{
		opt_dump_file = (int)strtol(argv[3], NULL, 0);
	}
#endif

	if (mt_sys_init() != MT_SUCCESS)
	{
		PRINTF("error: mt_sys_init failed!\r\n");
		return (-1);
	}

	if (mt_sys_map_register(REG_DMA_BASE, 0x10000, (void*)&DMA_BASE) != MT_SUCCESS)
	{
		PRINTF("error: mt_sys_map_register failed!\r\n");
		goto err1;
	}

	phyaddr = (unsigned long)mt_mmz_new(size, 4096, NULL, NULL);
	if (phyaddr == 0)
	{
		PRINTF("error: mt_mmz_new failed!\r\n");
		goto err2;
	}

	real_phyaddr = phyaddr;
	viraddr = mt_mmz_map(phyaddr, 0);

	if (viraddr == NULL)
	{
		PRINTF("error: mt_mmz_map failed!\r\n");
		goto err3;
	}
#else
	DMA_BASE = REG_DMA_BASE;
	phyaddr = CFG_TEST_PHY_ADDR;
	real_phyaddr = phyaddr;
	viraddr = (void*)(phyaddr | ADDR_NO_CACHE);
#endif

#if 0
	phyaddr_bk = real_phyaddr;
	viraddr_bk = viraddr;
#endif

	test_start_addr = (unsigned long)viraddr;
	test_size = size;

	PRINTF("%s: phyaddr 0x%lx, viraddr %p, size 0x%lx\r\n",
			argv[0], phyaddr, viraddr, size);
	PRINTF("%s: dma channel %d\r\n", argv[0], DMA_CHANNEL);
	PRINTF("%s: pattern %s\r\n", argv[0], PATTERN);

/* LINUX */
#ifdef __LINUX__
	randx = (uint32_t)time(NULL);
#else
	//TODO:
	randx = rd_mem((unsigned long)viraddr);
#endif
	PRINTF("%s: randx 0x%08x\r\n", argv[0], randx);

	do
	{
		PRINTF("[%lu]-------------------------------------------------------------\r\n", n++);

#if 1
		int res;
		res = memtest_hard((unsigned long)viraddr, size >> 20);
		ret |= res;

		if (res != 0)
			total_failed ++;

		PRINTF("Total Failed: %ld, DMA Error Count: %ld, CPU RW Error Count: %ld\n",
			total_failed,
			dma_error_count,
			cpu_error_count);
#endif

//2MB each time
#if 0
		real_phyaddr = phyaddr_bk;
		viraddr = viraddr_bk;

		for (offset=0; offset<size; offset+=0x200000)
		{
			ret |= memtest_hard((unsigned int)((unsigned char*)viraddr + offset), 0x200000 >> 20);

			real_phyaddr += 0x200000;
		}
#endif

	} while (loops --);

/* LINUX */
#ifdef __LINUX__
	mt_mmz_unmap((void*)phyaddr);

err3:
	mt_mmz_delete(phyaddr);

err2:
	mt_sys_unmap_register((void*)DMA_BASE);

err1:
	mt_sys_deinit();
#endif

	return ret;
}

