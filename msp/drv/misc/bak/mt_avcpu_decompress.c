/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifdef CONFIG_DECOMPRESS_LZMA
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include "mt_common.h"

#include "drv_sys_misc.h"

#include "mt_mach/clock.h"

#include "drv_sym4_reset.h"

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/chipinfo.h"

#include "mt_cache.h"
#include <linux/slab.h>
#include <linux/decompress/unlzma.h>

typedef unsigned int SizeT;


#define DECOMPRESS_PRINTK printk
//#define DECOMPRESS_PRINTK(...) do{}while(0)

#define LZMA_PROPS_SIZE 5

//#define LZMA_PROPERTIES_OFFSET 0
#define LZMA_SIZE_OFFSET       LZMA_PROPS_SIZE
#define LZMA_DATA_OFFSET       LZMA_SIZE_OFFSET+sizeof(u64)

extern struct mtd_info *get_mtd_device_nm(const char *name);
extern int mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen,u_char *buf);

unsigned int avcpu_getlzmauncompresssize(unsigned char *inStream)
{
    SizeT outSize;
    SizeT outSizeHigh;
	int i;
	unsigned int uncompress_size=0;
	SizeT outSizeFull = 0xFFFFFFFF;
	
	outSize = 0;
    outSizeHigh = 0;
    /* Read the uncompressed size */
    for (i = 0; i < 8; i++) {
        unsigned char b = inStream[LZMA_SIZE_OFFSET + i];
            if (i < 4) {
                outSize     += (unsigned int)(b) << (i * 8);
        } else {
                outSizeHigh += (unsigned int)(b) << ((i - 4) * 8);
        }
    }

    outSizeFull = (SizeT)outSize;
    if (sizeof(SizeT) >= 8) {
        /*
         * SizeT is a 64 bit uint => We can manage files larger than 4GB!
         *
         */
            outSizeFull |= (((SizeT)outSizeHigh << 16) << 16);
    } else if (outSizeHigh != 0 || (unsigned int)(SizeT)outSize != outSize) {
        /*
         * SizeT is a 32 bit uint => We cannot manage files larger than
         * 4GB!  Assume however that all 0xf values is "unknown size" and
         * not actually a file of 2^64 bits.
         *
         */
        if (outSizeHigh != (SizeT)-1 || outSize != (SizeT)-1) {
            printk ("LZMA: 64bit support not enabled.\n");
            return 0;
        }
    }

	uncompress_size=(unsigned int)outSizeFull;
	printk("uncompress_size :0x%x\n",uncompress_size);
	return uncompress_size;
}

int avcpu_do_decompress(u8 *input, int len, u8 *output, void (*error)(char *x))
{
	//return __decompress(input, len, NULL, NULL, output, 0, NULL, error);
	int ret=0;
	DECOMPRESS_PRINTK("len1:0x%x,input:0x%x,output:0x%x\n",len,input,output);
	ret = unlzma(input, (long)len, NULL, NULL, output, 0, error);
	DECOMPRESS_PRINTK("ret:%d\n",ret);
	//getlzmaparam();
	return ret;
}

void avcpu_error(char *x)
{
	DECOMPRESS_PRINTK("error\n");
	printk(x);
	
}

//extern int avcpu_do_decompress(u8 *input, int len, u8 *output, void (*error)(char *x));
u32 avcpu_decompress(u32 ddr_addr)
{
 	u32 offset = 0;
    unsigned char sizehdr[4];
    u32 true_size = 0;
	u32 jieya_addr = 0;
	u32 inflate_len = 0;
    struct mtd_info *mtd =NULL;
	u32 retlen=0;
	int ret = 0;
	u8 encrypt = 0;
	u32 tmp_addr=0;
	u32 compress_size=0;
	void *ktmp_addr; 
	
	
    DECOMPRESS_PRINTK("crm_avcpu_decompress enter\n");
	mtd = get_mtd_device_nm("av_cpu.img");
	if(IS_ERR(mtd))
		return 0;
	 	 
    // 1,get 4B,get true_size
    DECOMPRESS_PRINTK("offset:0x%x,raddr:0x%x\n",offset,ddr_addr);        
    ret=mtd_read(mtd, offset, 4,&retlen, sizehdr);

	DECOMPRESS_PRINTK("crm_avcpu_decompress 2\n");
    if(ret ||(retlen != 4))
		return 0;
	
    true_size = (sizehdr[0] << 24) |(sizehdr[1] << 16) |(sizehdr[2] << 8) |(sizehdr[3]);
    if(true_size == 0)
       return 0;
    DECOMPRESS_PRINTK("Imagsize is 0x%x from image header1\n", true_size);

    {
       // 2,get all avcpu to addr			       
	   compress_size= ALIGN((true_size + 4), CACHE_LINE_SIZE);
       ktmp_addr = kmalloc(compress_size + CACHE_LINE_SIZE, GFP_KERNEL);	
	   tmp_addr =(u32)ktmp_addr;
	   DECOMPRESS_PRINTK("compress_size:0x%x,tmp_addr:0x%x\n",compress_size,tmp_addr);
	   ret=mtd_read(mtd, offset, compress_size,&retlen, tmp_addr);
			
	   DECOMPRESS_PRINTK("retlen:0x%x\n",retlen);
	   if(ret ||(retlen != compress_size))
		 return 0;
    }  
    mt_dcache_flush(tmp_addr,compress_size);
    DECOMPRESS_PRINTK("Image size is 0x%x\n", true_size);
	#if 1
	    inflate_len = avcpu_getlzmauncompresssize((unsigned char *)(tmp_addr+4));
	    jieya_addr =  ioremap(ddr_addr, inflate_len);
		DECOMPRESS_PRINTK("jieya_addr:0x%x,ddr:0x%x,inflate_len:0x%x\n",jieya_addr,ddr_addr,inflate_len);
		ret=avcpu_do_decompress((unsigned char *)(tmp_addr+4),true_size,(unsigned char *)jieya_addr,avcpu_error);
	#else	
    jieya_addr =  ioremap(ddr_addr, getlzmauncompresssize((unsigned char *)(tmp_addr+4)));
	DECOMPRESS_PRINTK("jieya_addr:0x%x,ddr:0x%x\n",jieya_addr,ddr_addr);
    ret = lzmaBuffToBuffDecompress((unsigned char *)jieya_addr, &inflate_len, (unsigned char *)(tmp_addr+4), true_size); 
	#endif
	//DECOMPRESS_PRINTK("inflate_len:0x%x\n",inflate_len);
	if((ret != 0)||(inflate_len == 0))
        return 0;
		  
	inflate_len=ALIGN(inflate_len, CACHE_LINE_SIZE);
	DECOMPRESS_PRINTK("Decompressed image size is 0x%x\n", inflate_len);
    mt_dcache_flush(jieya_addr,inflate_len);
             
	wmb();
	iounmap(jieya_addr);
	kfree(ktmp_addr);
	DECOMPRESS_PRINTK("avcpu decompress end\n");

	return 1;
}
#endif
