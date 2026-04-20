/*=============================================================================+
|                                                                              |
| Copyright 2013                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/firmware.h>
#include <linux/netdevice.h>
#include <linux/delay.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/pm_runtime.h>

#include <linux/list.h>

#include "sdio.h"

static struct if_sdio_card * sdio_card;
static u8 int_stop = 0;

#define ALGIN		(7)	/*align transfer data size for workaround buggy host controller*/

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define LYNX_NO_PM  // temp solution
#define CONFIG_LYNX_CHRDEV
#define IF_SDIO_BLOCK_SIZE 512

/* identify firmware images */
#define FIRMWARE_LYNX_2_0_0     "lynx/sdio_app_2.img"
#define FIRMWARE_LYNX_3_0_0     "lynx/sdio_app_3.img"


/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
enum WCI_CMD_ID {
	WCI_H2D_FW_DOWNLOAD = 254,
	WCI_H2D_FW_COMP = 255,
};
struct lynx_wci_hdr {
    u8  cmd_id;
    u8  seq_no;
    u16 payload_len;
};

enum SDIO_CMD_DIR {
	SDIO_DIR_IN = 0,    /* to host */
    SDIO_DIR_OUT        /* to device */
};

struct if_sdio_packet {
    struct list_head list;
    struct sk_buff   *skb;
    u16              nb;
    u8               buffer[0] __attribute__((aligned(4)));
};

struct if_sdio_card {
    struct sdio_func    *func;

    //struct sdio_fw_info_old info;
    //u32                 db_start;
    //u32                 db_size;
    //u32                 db_num;

    spinlock_t          lock;
    struct list_head    list;

	const char *fw_name;
	const struct firmware *firmware;
	struct completion fw_done;

    int         model;
    unsigned long       ioport;
    unsigned int        scratch_reg;
    bool            started;
    wait_queue_head_t   pwron_waitq;

    u8          buffer[65536] __attribute__((aligned(4)));

    struct if_sdio_packet   *packets;

    struct workqueue_struct *workqueue;
    struct work_struct  packet_worker;

    u8          rx_unit;
};

#define SDIO_DRV_NAME "sdio"
#define SDIO_BUFFER_SIZE    (128*1024)
struct sdio_test_card {
    struct device           *dev;
    struct mmc_card         *card;
    struct sdio_func        *func;
    struct cdev             cdev;
    dev_t                   dev_t;
    struct class            *class;
    int                     major;
    int                     irq;

    u8                      *rbuffer;
    u8                      *wbuffer;
    u32                     offset;
    u32                     count;
};

static struct sdio_test_card *tcard = NULL;
static struct sdio_func tfunc0;
static struct sdio_func *func0 = &tfunc0;
static unsigned int func0_cis_start = 0;
static unsigned int twdata = 0x55aa;
static unsigned int is_clock_gated = 0;

enum {
    SDIO_CMD_UNKNOWN = 0,
    SDIO_CMD_F0_READ_52,
    SDIO_CMD_F0_WRITE_52,
    SDIO_CMD_F0_READ_53,
    SDIO_CMD_F0_WRITE_53,
    SDIO_CMD_F1_READ_52,
    SDIO_CMD_F1_WRITE_52,
    SDIO_CMD_F1_READ_53,
    SDIO_CMD_F1_WRITE_53,
    SDIO_CMD_F0_READ_52_LOOP,
    SDIO_CMD_F0_WRITE_52_LOOP,
    SDIO_CMD_F0_READ_53_LOOP,
    SDIO_CMD_F0_WRITE_53_LOOP,
    SDIO_CMD_F1_READ_52_LOOP,
    SDIO_CMD_F1_WRITE_52_LOOP,
    SDIO_CMD_F1_READ_53_LOOP,
    SDIO_CMD_F1_WRITE_53_LOOP,
    SDIO_CMD_F1_READ_53_1_BLOCK_LOOP,
    SDIO_CMD_F1_WRITE_53_1_BLOCK_LOOP,
    SDIO_CMD_F0_READ_WRITE_52_LOOP,
    SDIO_CMD_F0_READ_WRITE_53_LOOP,
    SDIO_CMD_F1_READ_WRITE_52_LOOP,
    SDIO_CMD_F1_READ_WRITE_53_LOOP,
    SDIO_CMD_F1_READ_WRITE_53_1_BLOCK_LOOP,
    SDIO_CMD_F1_READ_53_VAR_LENGTH,
    SDIO_CMD_F1_WRITE_53_VAR_LENGTH,
    SDIO_CMD_F1_READ_WRITE_53_VAR_LENGTH,
    SDIO_CMD_RESET,
    SDIO_CMD_HOST_INT,
    SDIO_CMD_DEVICE_INT,
    SDIO_CMD_CLK_GATE,
    SDIO_CMD_SET_BLKSIZE,
    SDIO_CMD_SET_ECSI,
} sdio_cmd;

enum {
    SDIO_FUNCTION_0 = 0,
    SDIO_FUNCTION_1 = 1,
};
enum {
    SDIO_CMD53 = 0,
    SDIO_CMD52 = 1,
};
enum {
    SDIO_WRITE = 0,
    SDIO_READ  = 1,
};
enum {
    SDIO_NODBG = 0,
    SDIO_DBG   = 1,
};


static const struct sdio_device_id if_sdio_ids[] = {
#ifdef CONFIG_LYNX_ROM3
    { SDIO_DEVICE(SDIO_VENDOR_ID_MONTAGE,
            SDIO_DEVICE_ID_LYNX_ROM3) },
#endif /*CONFIG_LYNX_ROM3*/

#ifdef CONFIG_LYNX_ROM2
    { SDIO_DEVICE(SDIO_VENDOR_ID_MONTAGE,
            SDIO_DEVICE_ID_LYNX_ROM2) },
#endif /*CONFIG_LYNX_ROM2*/

    { SDIO_DEVICE(SDIO_VENDOR_ID_MONTAGE,
            SDIO_DEVICE_ID_LYNX_FPGA) },
    { /* end: all zeroes */             },
};


/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
static void if_sdio_interrupt(struct sdio_func *func);

static void if_sdio_finish_power_on(struct if_sdio_card *card);
static int if_sdio_power_off(struct if_sdio_card *card);

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
#define lynx_dbg(fmt, ...)

static void sdio_hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET,
			16, 1,
			buf, len, false);
}

/********************************************************************/
/* I/O                                                              */
/********************************************************************/


/********************************************************************/
/* Power management                                                 */
/********************************************************************/

/* Finish power on sequence (after firmware is loaded) */
static void if_sdio_finish_power_on(struct if_sdio_card *card)
{
    struct sdio_func *func = card->func;
    int ret;

    sdio_claim_host(func);
    sdio_set_block_size(card->func, IF_SDIO_BLOCK_SIZE);

    /*
     * Set up the interrupt handler late.
     *
     * If we set it up earlier, the (buggy) hardware generates a spurious
     * interrupt, even before the interrupt has been enabled, with
     * CCCR_INTx = 0.
     *
     * We register the interrupt handler late so that we can handle any
     * spurious interrupts, and also to avoid generation of that known
     * spurious interrupt in the first place.
     */
    ret = sdio_claim_irq(func, if_sdio_interrupt);
    if (ret)
        goto release;

    /*
     * Enable interrupts now that everything is set up
     */
//  sdio_writeb(func, 0x0f, IF_SDIO_H_INT_MASK, &ret);
    if (ret)
        goto release_irq;

    sdio_release_host(func);

    lynx_dbg(LYNX_DBG_SDIO, "register irq OK\n");

    return;

release_irq:
    sdio_release_irq(func);
release:
    sdio_release_host(func);

    lynx_dbg(LYNX_DBG_SDIO, "register irq FAIL\n");
}

static int if_sdio_power_on(struct if_sdio_card *card)
{
    struct sdio_func *func = card->func;
    int ret = 0;

    sdio_claim_host(func);

    ret = sdio_enable_func(func);
    if (ret)
        goto release;

    sdio_release_host(func);

//    ret = if_sdio_prog_firmware(card);

    if (ret) {
        sdio_disable_func(func);
        return ret;
    }

    if_sdio_finish_power_on(card);

    return 0;

release:
    sdio_release_host(func);
    return ret;
}

static int if_sdio_power_off(struct if_sdio_card *card)
{
    struct sdio_func *func = card->func;

    sdio_claim_host(func);
    sdio_release_irq(func);
    sdio_disable_func(func);
    sdio_release_host(func);

    return 0;
}

/*******************************************************************/
/* SDIO callbacks                                                  */
/*******************************************************************/

static void if_sdio_interrupt(struct sdio_func *func)
{
    struct if_sdio_card *card;
    unsigned int addr = 0;
    unsigned int count = SDIO_BUFFER_SIZE;
    int err_code = 0;

    pr_err("%s() is started\n", __func__);

    if (int_stop)
        return;

    card = sdio_get_drvdata(func);

    if (!tcard)
        return;
    if (!tcard->rbuffer)
        return;
    if (!tcard->wbuffer)
        return;

    printk("loopback test start\n");

    sdio_claim_host(card->func);
    err_code = sdio_memcpy_fromio(card->func, tcard->rbuffer, addr, count);
    if (err_code) {
        printk("read f%d error at [%d]: err_code [%d]\n", card->func->num, addr, err_code);
    }
    else {
        printk("CMD53 read f%d OK!\n", card->func->num);
//        print_hex_dump(KERN_INFO, "read data: ", DUMP_PREFIX_OFFSET, 16, 1, tcard->rbuffer, count, 0);
        if (!memcmp(tcard->rbuffer, tcard->wbuffer, count))
            printk("loopback test compare data PASS!\n");
        else
            printk("loopback test compare data FAIL!\n");

        sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_CLR_IRQ, SDIO_CCCR_HINT_SET, &err_code);

        while (sdio_f0_readb(card->func, SDIO_CCCR_INTx, &err_code) & 0x02) ;

        printk("loopback test Finish\n");
    }
    sdio_release_host(card->func);
}

static void lynx_test_rw_prepare(void)
{
    char **hw_name = mmc_priv(tcard->func->card->host);

    if (!mmc_host_is_spi(tcard->func->card->host)) {
        if (hw_name && !strncmp("montage-hsmmc", *hw_name, strlen("montage-hsmmc"))) {
            if (is_clock_gated) {
                void __iomem *ioaddr = (void *)0xaf00b02c;
                is_clock_gated = 0;
                writel(readl(ioaddr) | 0x07, ioaddr);
            }
        }
    }
}
static long lynx_test_rw_cmp(unsigned int CMDFN, unsigned int addr, unsigned int count)
{
    int err_code = 0;

    if (CMDFN == SDIO_FUNCTION_0) {
        if (addr < func0_cis_start) {
            if ((addr + count) > func0_cis_start) {
                if (memcmp(tcard->rbuffer + func0_cis_start, tcard->wbuffer + func0_cis_start, (addr + count - func0_cis_start)))
                    err_code = -1;
            }
        }
        else {
            if (memcmp(tcard->rbuffer + addr, tcard->wbuffer + addr, count))
                err_code = -1;
        }
    }
    else {
        if (memcmp(tcard->rbuffer, tcard->wbuffer, count))
            err_code = -1;
    }
    return err_code;
}

static long lynx_test_rw_cmd(unsigned int CMDFN, unsigned int addr, unsigned int count, unsigned int CMDID, unsigned int CMDRW, unsigned int CMDDBG)
{
    unsigned int ncount = count;
    int err_code = 0;
    int i;

    if ((CMDFN == SDIO_FUNCTION_0) && (func0->cur_blksize == 0)) {
        printk("f0 has no blksize\n");
        return -1;
    }
    if (addr >= SDIO_BUFFER_SIZE)
        addr = addr % SDIO_BUFFER_SIZE;
    if (count > SDIO_BUFFER_SIZE)
        count = SDIO_BUFFER_SIZE;
    printk("%s 0x%x length %d\n", (CMDRW == SDIO_READ) ? "read from" : "write to", addr, count);

    if (CMDRW == SDIO_WRITE) {
        memset(tcard->wbuffer, 0, SDIO_BUFFER_SIZE);
        for (i = 0; i < SDIO_BUFFER_SIZE; i++) {
            if (i % 2)
                tcard->wbuffer[i] = (twdata >> 8) & 0xff;
            else
                tcard->wbuffer[i] = twdata & 0xff;
        }
        printk("0x%x%x%x%x\n", tcard->wbuffer[0], tcard->wbuffer[1], tcard->wbuffer[2], tcard->wbuffer[3]);
        twdata ^= 0xffff;
    }
    else {
        memset(tcard->rbuffer, 0, SDIO_BUFFER_SIZE);
    }
    sdio_claim_host(tcard->func);

    if (CMDID == SDIO_CMD52) {
        if (CMDRW == SDIO_READ) {
            for (; ncount > 0; addr++, ncount--) {
                if (CMDFN == SDIO_FUNCTION_0)
                    tcard->rbuffer[addr] = sdio_f0_readb(tcard->func, addr, &err_code);
                else
                    tcard->rbuffer[addr] = sdio_readb(tcard->func, addr, &err_code);

                if (err_code)
                    break;
            }
            if (err_code)
            {
                printk("CMD52 read f%d error at [%d]: err_code [%d]\n", CMDFN, addr, err_code);
                if (CMDDBG == SDIO_DBG)
                    print_hex_dump(KERN_INFO, "read data: ", DUMP_PREFIX_OFFSET, 16, 1, tcard->rbuffer, addr+1, 0);
            }
            else {
                printk("CMD52 read f%d OK!\n", CMDFN);
                if (CMDDBG == SDIO_DBG)
                    print_hex_dump(KERN_INFO, "read data: ", DUMP_PREFIX_OFFSET, 16, 1, tcard->rbuffer, count, 0);
            }
        }
        else {
            for (; ncount > 0; addr++, ncount--) {
                if (CMDFN == SDIO_FUNCTION_0) {
                    if (addr >= func0_cis_start)
                        sdio_f0_writeb(tcard->func, tcard->wbuffer[addr], addr, &err_code);
                }
                else
                    sdio_writeb(tcard->func, tcard->wbuffer[addr], addr, &err_code);

                if (err_code)
                    break;
            }
            if (err_code)
            {
                printk("CMD52 write f%d error at [%d]: err_code [%d]\n", CMDFN, addr, err_code);
            }
            else {
                printk("CMD52 write f%d OK!\n", CMDFN);
            }
        }
    }
    else {/* CMD53 */
        if (CMDRW == SDIO_READ) {
            if (CMDFN == SDIO_FUNCTION_0) {
                if (addr < func0_cis_start) {
                    if ((addr + count) > func0_cis_start) {
                        ncount = (addr + count - func0_cis_start);
                        err_code = sdio_memcpy_fromio(func0, &tcard->rbuffer[count - ncount], func0_cis_start, (ncount + ALGIN) & ((uint)~ALGIN));//(ncount + 3) & ((uint)~3)
                        ncount = (count - ncount);
                    }
                    err_code = sdio_memcpy_fromio(func0, tcard->rbuffer, addr, (ncount + ALGIN) & ((uint)~ALGIN));//(ncount + 3) & ((uint)~3)
                }
                else {
                    err_code = sdio_memcpy_fromio(func0, tcard->rbuffer, addr, (count + ALGIN) & ((uint)~ALGIN));//(count + 3) & ((uint)~3)
                }
            }
            else
                err_code = sdio_memcpy_fromio(tcard->func, tcard->rbuffer, addr, (count + ALGIN) & ((uint)~ALGIN));//(count + 3) & ((uint)~3)

            if (err_code) {
                printk("CMD53 read f%d error at [%d]: err_code [%d]\n", CMDFN, addr, err_code);
            }
            else {
                printk("CMD53 read f%d OK!\n", CMDFN);
                if (CMDDBG == SDIO_DBG)
                    print_hex_dump(KERN_INFO, "read data: ", DUMP_PREFIX_OFFSET, 16, 1, tcard->rbuffer, count, 0);
            }
        }
        else {
            if (CMDFN == SDIO_FUNCTION_0) {
                if (addr < func0_cis_start) {
                    if ((addr + count) > func0_cis_start) {
                        count = (addr + count - func0_cis_start);
                        addr = func0_cis_start;
                        err_code = sdio_memcpy_toio(func0, addr, tcard->wbuffer, (count + ALGIN) & ((uint)~ALGIN));
                    }
                }
                else {
                    err_code = sdio_memcpy_toio(func0, addr, tcard->wbuffer, (count + ALGIN) & ((uint)~ALGIN));
                }
            }
            else
                err_code = sdio_memcpy_toio(tcard->func, addr, tcard->wbuffer, (count + ALGIN) & ((uint)~ALGIN));
            if (err_code) {
                printk("write f%d error at [%d]: err_code [%d]\n", CMDFN, addr, err_code);
            }
            else {
                printk("CMD53 write f%d OK!\n", CMDFN);
            }
        }
    }
    sdio_release_host(tcard->func);
    return err_code;
}
static int lynx_drv_open(struct inode *inode, struct file *file) 
{
    file->private_data = tcard;
    return 0;
}
static ssize_t lynx_drv_read(struct file *file, char __user *buf, size_t size, loff_t *off) 
{
    struct sdio_test_card *tcard = file->private_data;
    unsigned int fn, addr, val[2];
    int err_code = 0, ret = 0;
    u8 data;
    ret = copy_from_user(val, buf, sizeof(val));
    if( ret < 0)
        return -1;
    fn   = val[0];
    addr = val[1];

    lynx_test_rw_prepare();

    sdio_claim_host(tcard->func);
    if (!fn)
        data = sdio_f0_readb(tcard->func, addr, &err_code);
    else
        data = sdio_readb(tcard->func, addr, &err_code);
    sdio_release_host(tcard->func);
    if(err_code < 0)
        return err_code;

    val[0] = data;
    ret = copy_to_user((void __user *)buf, val, sizeof(val)); 
    if(ret < 0)
        return ret;
    return 0;
}
static ssize_t lynx_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *off) 
{
    struct sdio_test_card *tcard = file->private_data;
    unsigned int fn, addr, value, val[3];
    int err_code = 0, ret = 0;
    ret = copy_from_user(val, buf, sizeof(val));
    if( ret < 0)
        return ret;
    fn    = val[0];
    addr  = val[1];
    value = val[2];

    lynx_test_rw_prepare();

    sdio_claim_host(tcard->func);
    if (!fn)
        sdio_f0_writeb(tcard->func, (unsigned char)value, addr, &err_code);
    else
        sdio_writeb(tcard->func, (unsigned char)value, addr, &err_code);
    sdio_release_host(tcard->func);
    if(err_code < 0)
        return err_code;

    if(err_code)
        return err_code;
    return 0;
}
static long lynx_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
    unsigned int fn, addr, value, buf[4];  
    unsigned int offset, blksz, count;
    int err_code = 0, ret = 0;
    int i;

    memset(buf,0,sizeof(buf)/sizeof(buf[0]));
    copy_from_user(buf, (const void __user *)arg, sizeof(int)*sizeof(buf)/sizeof(buf[0]));  
 
    fn    = buf[0];
    addr  = buf[1];
    value = buf[2];
    count = buf[3];

    offset = addr;
    blksz  = value;
    tcard->offset = offset;
    tcard->count  = count;

    lynx_test_rw_prepare();

    switch (cmd) {
    case SDIO_CMD_F0_READ_52:
        ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, count, SDIO_CMD52, SDIO_READ, SDIO_DBG);
        break;
    case SDIO_CMD_F0_WRITE_52:
        ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, count, SDIO_CMD52, SDIO_WRITE, SDIO_DBG);
        break;
    case SDIO_CMD_F0_READ_53:
        ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, count, SDIO_CMD53, SDIO_READ, SDIO_DBG);
        break;
    case SDIO_CMD_F0_WRITE_53:
        ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, count, SDIO_CMD53, SDIO_WRITE, SDIO_DBG);
        break;
    case SDIO_CMD_F1_READ_52:
        ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, count, SDIO_CMD52, SDIO_READ, SDIO_DBG);
        break;
    case SDIO_CMD_F1_WRITE_52:
        ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, count, SDIO_CMD52, SDIO_WRITE, SDIO_DBG);
        break;
    case SDIO_CMD_F1_READ_53:
        ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, count, SDIO_CMD53, SDIO_READ, SDIO_DBG);
        break;
    case SDIO_CMD_F1_WRITE_53:
        ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, count, SDIO_CMD53, SDIO_WRITE, SDIO_DBG);
        break;
    case SDIO_CMD_F0_READ_52_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, SDIO_BUFFER_SIZE, SDIO_CMD52, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD52 read\n", i);
                break;
            }
        }
        break;
    case SDIO_CMD_F0_WRITE_52_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, SDIO_BUFFER_SIZE, SDIO_CMD52, SDIO_WRITE, SDIO_NODBG))) {
                printk("loop=%d error CMD52 write\n", i);
                break;
            }
        }
        break;
    case SDIO_CMD_F0_READ_53_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, SDIO_BUFFER_SIZE, SDIO_CMD53, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD53 read\n", i);
                break;
            }
        }
        break;
    case SDIO_CMD_F0_WRITE_53_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, SDIO_BUFFER_SIZE, SDIO_CMD53, SDIO_WRITE, SDIO_NODBG))) {
                printk("loop=%d error CMD53 write\n", i);
                break;
            }
        }
        break;
    case SDIO_CMD_F1_READ_52_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, SDIO_BUFFER_SIZE, SDIO_CMD52, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD52 read\n", i);
                break;
            }
        }
        break;
    case SDIO_CMD_F1_WRITE_52_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, SDIO_BUFFER_SIZE, SDIO_CMD52, SDIO_WRITE, SDIO_NODBG))) {
                printk("loop=%d error CMD52 write\n", i);
                break;
            }
        }
        break;
    case SDIO_CMD_F1_READ_53_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, SDIO_BUFFER_SIZE, SDIO_CMD53, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD53 read\n", i);
                break;
            }
        }
        break;
    case SDIO_CMD_F1_WRITE_53_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, SDIO_BUFFER_SIZE, SDIO_CMD53, SDIO_WRITE, SDIO_NODBG))) {
                printk("loop=%d error CMD53 write\n", i);
                break;
            }
        }
        break;
    case SDIO_CMD_F1_READ_53_1_BLOCK_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]: cur_blksize[%d]\n", i, tcard->func->cur_blksize);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, i * tcard->func->cur_blksize, tcard->func->cur_blksize, SDIO_CMD53, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD53 read\n", i);
                break;
            }
        }
        break;
    case SDIO_CMD_F1_WRITE_53_1_BLOCK_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]: cur_blksize[%d]\n", i, tcard->func->cur_blksize);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, i * tcard->func->cur_blksize, tcard->func->cur_blksize, SDIO_CMD53, SDIO_WRITE, SDIO_NODBG))) {
                printk("loop=%d error CMD53 write\n", i);
                break;
            }
        }
        break;
    case SDIO_CMD_F0_READ_WRITE_52_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, SDIO_BUFFER_SIZE, SDIO_CMD52, SDIO_WRITE, SDIO_NODBG))) {
                printk("loop=%d error CMD52 write\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, SDIO_BUFFER_SIZE, SDIO_CMD52, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD52 read\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmp(SDIO_FUNCTION_0, 0, SDIO_BUFFER_SIZE))) {
                printk("loop=%d error compare\n", i);
                print_hex_dump(KERN_INFO, "read data: ", DUMP_PREFIX_OFFSET, 16, 1, tcard->rbuffer, SDIO_BUFFER_SIZE, 0);
                break;
            }
        }
        break;
    case SDIO_CMD_F0_READ_WRITE_53_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, SDIO_BUFFER_SIZE, SDIO_CMD53, SDIO_WRITE, SDIO_NODBG))) {
                printk("loop=%d error CMD53 write\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_0, 0, SDIO_BUFFER_SIZE, SDIO_CMD53, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD53 read\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmp(SDIO_FUNCTION_0, 0, SDIO_BUFFER_SIZE))) {
                printk("loop=%d error compare\n", i);
                print_hex_dump(KERN_INFO, "read data: ", DUMP_PREFIX_OFFSET, 16, 1, tcard->rbuffer, SDIO_BUFFER_SIZE, 0);
                break;
            }
        }
        break;
    case SDIO_CMD_F1_READ_WRITE_52_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, SDIO_BUFFER_SIZE, SDIO_CMD52, SDIO_WRITE, SDIO_NODBG))) {
                printk("loop=%d error CMD52 write\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, SDIO_BUFFER_SIZE, SDIO_CMD52, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD52 read\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmp(SDIO_FUNCTION_1, 0, SDIO_BUFFER_SIZE))) {
                printk("loop=%d error compare\n", i);
                print_hex_dump(KERN_INFO, "read data: ", DUMP_PREFIX_OFFSET, 16, 1, tcard->rbuffer, SDIO_BUFFER_SIZE, 0);
                break;
            }
        }
        break;
    case SDIO_CMD_F1_READ_WRITE_53_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, SDIO_BUFFER_SIZE, SDIO_CMD53, SDIO_WRITE, SDIO_NODBG))) {
                printk("loop=%d error CMD53 write\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, SDIO_BUFFER_SIZE, SDIO_CMD53, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD53 read\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmp(SDIO_FUNCTION_1, 0, SDIO_BUFFER_SIZE))) {
                printk("loop=%d error compare\n", i);
                print_hex_dump(KERN_INFO, "read data: ", DUMP_PREFIX_OFFSET, 16, 1, tcard->rbuffer, SDIO_BUFFER_SIZE, 0);
                break;
            }
        }
        break;
    case SDIO_CMD_F1_READ_WRITE_53_1_BLOCK_LOOP:
        for(i = 0; i < count; i++) {
            printk("[%d]: cur_blksize[%d]\n", i, tcard->func->cur_blksize);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, i * tcard->func->cur_blksize, tcard->func->cur_blksize, SDIO_CMD53, SDIO_WRITE, SDIO_NODBG))) {
                printk("loop=%d error CMD53 write\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, i * tcard->func->cur_blksize, tcard->func->cur_blksize, SDIO_CMD53, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD53 read\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmp(SDIO_FUNCTION_1, i * tcard->func->cur_blksize, tcard->func->cur_blksize))) {
                printk("loop=%d error compare\n", i);
                print_hex_dump(KERN_INFO, "read data: ", DUMP_PREFIX_OFFSET, 16, 1, tcard->rbuffer, SDIO_BUFFER_SIZE, 0);
                break;
            }
        }
        break;
    case SDIO_CMD_F1_READ_53_VAR_LENGTH:
        for(i = 1; i <= count; i++) {
            printk("check read length [%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, i, SDIO_CMD53, SDIO_READ, (addr ? SDIO_DBG : SDIO_NODBG)))) {
                printk("loop=%d error CMD53 read\n", i);
                break;
            }
            else {
                printk("length [%d] ******** PASS ********\n", i);
            }
        }
        break;
    case SDIO_CMD_F1_WRITE_53_VAR_LENGTH:
        for(i = 1; i <= count; i++) {
            printk("check write length [%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, i, SDIO_CMD53, SDIO_WRITE, (addr ? SDIO_DBG : SDIO_NODBG)))) {
                printk("loop=%d error CMD53 write\n", i);
                break;
            }
            else {
                printk("length [%d] ******** PASS ********\n", i);
            }
        }
        break;
    case SDIO_CMD_F1_READ_WRITE_53_VAR_LENGTH:
        for(i = 1; i <= count; i++) {
            printk("check read/write length [%d]:\n", i);
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, i, SDIO_CMD53, SDIO_WRITE, (addr ? SDIO_DBG : SDIO_NODBG)))) {
                printk("loop=%d error CMD53 write\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmd(SDIO_FUNCTION_1, 0, i, SDIO_CMD53, SDIO_READ, SDIO_NODBG))) {
                printk("loop=%d error CMD53 read\n", i);
                break;
            }
            if ((ret = lynx_test_rw_cmp(SDIO_FUNCTION_1, 0, i))) {
                printk("loop=%d error compare\n", i);
                print_hex_dump(KERN_INFO, "read data: ", DUMP_PREFIX_OFFSET, 16, 1, tcard->rbuffer, i, 0);
                break;
            }
            else {
                printk("length [%d] ******** PASS ********\n", i);
            }
        }
        break;
    case SDIO_CMD_RESET:
        sdio_claim_host(tcard->func);
        sdio_f0_writeb(tcard->func, (0x08), SDIO_CCCR_ABORT, &err_code);
        sdio_release_host(tcard->func);
        break;
    case SDIO_CMD_HOST_INT:
        sdio_claim_host(tcard->func);
        for (i=0;i<8;i++) {
            if (((1<<i) == SDIO_HINT_SW_RESERVE6) ||
                ((1<<i) == SDIO_HINT_SW_LOOPBACK) ||
                ((1<<i) == SDIO_HINT_SW_CLR_IRQ) ||
                ((1<<i) == SDIO_HINT_SW_ENTER_SP) ) {
                printk("skip do int 0x%x\n", (1<<i));
                continue;
            }
            sdio_f0_writeb(tcard->func, (1<<i), SDIO_CCCR_HINT_SET, &err_code);
            printk("wait device interrupt for 0x%x\n", (1<<i));
            msleep(1000);
        }
        sdio_release_host(tcard->func);
        break;
    case SDIO_CMD_DEVICE_INT:
        /* do SDIO LOOPBACK Test */
        addr = 0;
        for (i = 0; i < SDIO_BUFFER_SIZE; i++) {
            tcard->wbuffer[i] = i;
        }
        memset(tcard->rbuffer, 0x00, SDIO_BUFFER_SIZE/4);
        count = SDIO_BUFFER_SIZE;
        printk("loopback test length %d\n", count);
        sdio_claim_host(tcard->func);
        err_code = sdio_memcpy_toio(tcard->func, addr, tcard->wbuffer, count);
        if (err_code) {
            printk("write f%d error at [%d]: err_code [%d]\n", tcard->func->num, addr, err_code);
        }
        else {
            printk("CMD53 write f%d OK!\n", tcard->func->num);
            sdio_f0_writeb(tcard->func, SDIO_HINT_SW_LOOPBACK, SDIO_CCCR_HINT_SET, &err_code);
            if (err_code)
                printk("CMD52 enable LOOPBACK failed\n");
        }
        sdio_release_host(tcard->func);
        break;
    case SDIO_CMD_CLK_GATE:
        sdio_claim_host(tcard->func);
        sdio_f0_writeb(tcard->func, SDIO_HINT_SW_ENTER_SP, SDIO_CCCR_HINT_SET, &err_code);
        sdio_release_host(tcard->func);
        if (err_code)
            printk("CMD52 enable ENTER_SP(Sleep Mode) failed\n");
        else
            printk("enable ENTER_SP(Sleep Mode) OK!\n");

        if (!mmc_host_is_spi(tcard->func->card->host)) {
            char **hw_name = mmc_priv(tcard->func->card->host);
            if (hw_name && !strncmp("montage-hsmmc", *hw_name, strlen("montage-hsmmc"))) {
                void __iomem *ioaddr = (void *)0xaf00b02c;
                is_clock_gated = 1;
                writel(readl(ioaddr) & (~0x04), ioaddr);
                printk("SDHC Host is Cheetah:%s\nClock Gating!!\n", *hw_name);
            }
            else {
                printk("Don't Suport in this Platformh\n");
            }
        }
        else {
            printk("Don't Need to Test in SPI Host\n");
        }
        break;
    case SDIO_CMD_SET_BLKSIZE:
        sdio_claim_host(tcard->func);
        if (fn) {
	        if (blksz > tcard->func->card->host->max_blk_size)
                printk("Host max_blk_size = [%d]\n", tcard->func->card->host->max_blk_size);
            ret = sdio_set_block_size(tcard->func, blksz);
        }
        else {
            sdio_f0_writeb(tcard->func, blksz & 0xff, SDIO_CCCR_BLKSIZE, &err_code);
            sdio_f0_writeb(tcard->func, (blksz >> 8) & 0xff, SDIO_CCCR_BLKSIZE + 1, &err_code);
            func0->cur_blksize = blksz;
        }
        sdio_release_host(tcard->func);
        if (ret || err_code)
            printk("Set Block Size failed: err_code [%d]\n", ret);
        break;
    case SDIO_CMD_SET_ECSI:
    {
        u8 reg;
        sdio_claim_host(tcard->func);
        reg = sdio_f0_readb(tcard->func, SDIO_CCCR_IF, &err_code);
        if (!err_code) {
            if (value)
                reg |= SDIO_BUS_ECSI;
            else
                reg &= ~SDIO_BUS_ECSI;
            sdio_f0_writeb(tcard->func, reg, SDIO_CCCR_IF, &err_code);
        }
        if (err_code)
            printk("Set ECSI failed: err_code [%d]\n", ret);
        else {
            printk("%s ECSI OK\n", value ? "Set" : "Clear");
        }
        sdio_release_host(tcard->func);
        break;
    }
    default:
        printk("ioctl error cmd %d\n", cmd);
        break;
    }   

    if (ret)
        return ret;
    else
        return 0;
}

static int lynx_drv_release(struct inode *inode, struct file *file) 
{
    return 0;
}

static const struct file_operations lynx_drv_fops = {
    .owner           =    THIS_MODULE,
    .open            =    lynx_drv_open,
    .read            =    lynx_drv_read,
    .write           =    lynx_drv_write,
    .unlocked_ioctl  =    lynx_drv_ioctl,
    .release         =    lynx_drv_release,
};

static int if_sdio_probe(struct sdio_func *func,
        const struct sdio_device_id *id)
{
    struct if_sdio_card *card;
    int i;
    int ret;
    int err;

    lynx_dbg(LYNX_DBG_SDIO, "sdio new func %d vendor 0x%x device 0x%x block 0x%x/0x%x\n",
            func->num, func->vendor, func->device,
            func->max_blksize, func->cur_blksize);

    pr_info("\nfunc info start:\n");
    pr_info("card supported clock %d\n", func->card->cis.max_dtr);
    pr_info("class 0x%x vendor 0x%x, device 0x%x, max_blk_size 0x%x cur_blksize 0x%x\n"
        "enable_timeout 0x%x function_state 0x%x bus_speed 0x%x\n"
        "f_min %u f_max %u f_init %u\n"
        "actual_clock %u \n"
        "max_current_180 %u max_current_300 %u max_current_180 %u\n"
        "MMC_CAP_MMC_HIGHSPEED 0x%lx, MMC_CAP_SD_HIGHSPEED 0x%lx\n"
        "1_8V_DDR 0x%lx, 1_2V_DDR 0x%lx, SDR12 0x%lx, SDR25 0x%lx, SDR50 0x%lx, SDR104 0x%lx, DDR50 0x%lx\n"
        "HS200_1_8V_SDR 0x%x, HS200_1_2V_SDR 0x%x\n"
        "sg supprt %d, max_segs %d, max_seg_size %d, max_req_size %d\n"
        "ocr_avail 0x%x, ocr_avail_sdio 0x%x, ocr_avail_sd 0x%x, ocr_avail_mmc 0x%x\n"
        "func->card->host->ios.timing 0x%x\n",
        func->class, func->vendor, func->device, func->max_blksize, func->cur_blksize,
        func->enable_timeout, func->state,      func->card->sd_bus_speed,
        func->card->host->f_min, func->card->host->f_max, func->card->host->f_init,
        func->card->host->actual_clock,
        func->card->host->max_current_180, func->card->host->max_current_300,
        func->card->host->max_current_180,
        (long unsigned int)(func->card->host->caps & MMC_CAP_MMC_HIGHSPEED),
        (long unsigned int)(func->card->host->caps & MMC_CAP_SD_HIGHSPEED),
        (long unsigned int)(func->card->host->caps & MMC_CAP_1_8V_DDR),
        (long unsigned int)(func->card->host->caps & MMC_CAP_1_2V_DDR),
        (long unsigned int)(func->card->host->caps & MMC_CAP_UHS_SDR12),
        (long unsigned int)(func->card->host->caps & MMC_CAP_UHS_SDR25),
        (long unsigned int)(func->card->host->caps & MMC_CAP_UHS_SDR50),
        (long unsigned int)(func->card->host->caps & MMC_CAP_UHS_SDR104),
        (long unsigned int)(func->card->host->caps & MMC_CAP_UHS_DDR50),
        func->card->host->caps2 & MMC_CAP2_HS200_1_8V_SDR,
        func->card->host->caps2 & MMC_CAP2_HS200_1_2V_SDR,
        (func->card->host->max_segs > 1),
        func->card->host->max_segs,
        func->card->host->max_seg_size,
        func->card->host->max_req_size,
        func->card->host->ocr_avail,
        func->card->host->ocr_avail_sdio,
        func->card->host->ocr_avail_sd,
        func->card->host->ocr_avail_mmc,
        func->card->host->ios.timing);
    pr_info("func info end:\n\n");

    for (i = 0;i < func->card->num_info;i++) {
        lynx_dbg(LYNX_DBG_SDIO, "info[%d]=%s\n", i, func->card->info[i]);
    }

    if (func->card->num_info == 0) {
        pr_err("unable to identify card model\n");
        return -ENODEV;
    }

    func->card->quirks |= MMC_QUIRK_LENIENT_FN0;

    card = os_api_alloc(sizeof(struct if_sdio_card), GFP_KERNEL);
    if (!card)
        return -ENOMEM;

    card->func = func;
    sdio_card = card;

    spin_lock_init(&card->lock);
	INIT_LIST_HEAD(&card->list);

    sdio_set_drvdata(func, card);

/***********************************************************************************/
    tcard = kmalloc(sizeof(struct sdio_test_card), GFP_KERNEL);
    if (!tcard) {
        ret = -ENOMEM;
        goto err_activate_card;
    }
    tcard->func = func;
    tcard->card = func->card;
    tcard->rbuffer = os_api_alloc(SDIO_BUFFER_SIZE, GFP_KERNEL);
    tcard->wbuffer = os_api_alloc(SDIO_BUFFER_SIZE, GFP_KERNEL);
    if (!tcard->rbuffer || !tcard->wbuffer) {
        ret = -ENOMEM;
        kfree(tcard);
        goto err_activate_card;
    }

    ret = alloc_chrdev_region(&tcard->dev_t, 0, 1, SDIO_DRV_NAME);
    if (ret) { 
        printk("Lynx Device alloc_chrdev_region failed %d\n", ret);
        kfree(tcard);
        goto err_activate_card;
    } else {
        tcard->major= MAJOR(tcard->dev_t);
    }

    memset(&tcard->cdev, 0, sizeof(struct cdev));
    cdev_init(&tcard->cdev, &lynx_drv_fops);

    ret = cdev_add(&tcard->cdev, tcard->dev_t, 1);
    if (ret) {
        unregister_chrdev_region(tcard->dev_t, 1);
        printk("Lynx Device cdev_add failed %d\n", ret);
        kfree(tcard);
        goto err_activate_card;
    }

    tcard->class= class_create(THIS_MODULE, SDIO_DRV_NAME);
    device_create(tcard->class, NULL, MKDEV(tcard->major, MINOR(tcard->dev_t)), NULL, SDIO_DRV_NAME);
 
    memcpy(func0, tcard->func, sizeof(struct sdio_func));
    func0->num = 0;
    func0->cur_blksize = 0;
    func0_cis_start = 0;
    sdio_claim_host(tcard->func);
    for (i = 0; i < 3; i++) {
        int err_code = 0;
        unsigned char x = sdio_f0_readb(tcard->func, SDIO_FBR_CIS + i, &err_code);
        if (err_code) {
            printk("read CIS FAIL!\n");
            ret = -EINVAL;
            break;
        }
        func0_cis_start |= x << (i * 8);
    }
    sdio_release_host(tcard->func);
    if (ret)
        goto err_activate_card;
    printk("Lynx Device Common CIS Pointer = 0x%x\n", func0_cis_start);
/***********************************************************************************/

    ret = if_sdio_power_on(card);
    if (ret)
        goto err_activate_card;

    if (mmc_card_uhs(func->card->host->card))
        sdio_f0_writeb(card->func, (unsigned char)SDIO_HINT_SW_RESERVE6, SDIO_CCCR_HINT_SET, &err);

out:
    lynx_dbg(LYNX_DBG_SDIO, "%s ret=%d\n", __func__, ret);

    return ret;

err_activate_card:
    kfree(card);

    goto out;
}

static void if_sdio_remove(struct sdio_func *func)
{
    struct if_sdio_card *card;

    card = sdio_get_drvdata(func);

    if (tcard) {
        unregister_chrdev_region(tcard->dev_t, 1);
        cdev_del(&tcard->cdev);
        device_destroy(tcard->class, tcard->dev_t);
        class_destroy(tcard->class);
        if (tcard->rbuffer)
            kfree(tcard->rbuffer);
        if (tcard->wbuffer)
            kfree(tcard->wbuffer);
        kfree(tcard);
        tcard = NULL;
    }

    /* Undo decrement done above in if_sdio_probe */
    pm_runtime_get_noresume(&func->dev);

    if_sdio_power_off(card);

    kfree(card);

    lynx_dbg(LYNX_DBG_SDIO, "%s\n", __func__);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
static int if_sdio_suspend(struct device *dev)
{
    struct sdio_func *func = dev_to_sdio_func(dev);
    int ret;

    mmc_pm_flag_t flags = sdio_get_host_pm_caps(func);

    dev_info(dev, "%s: suspend: PM flags = 0x%x\n",
         sdio_func_id(func), flags);

    if (!(flags & MMC_PM_KEEP_POWER)) {
        dev_err(dev, "%s: cannot remain alive while host is suspended\n",
            sdio_func_id(func));
        return -ENOSYS;
    }

    ret = sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);
    if (ret)
        return ret;

    return sdio_set_host_pm_flags(func, MMC_PM_WAKE_SDIO_IRQ);
}

static int if_sdio_resume(struct device *dev)
{
    struct sdio_func *func = dev_to_sdio_func(dev);
    int ret = 0;

    dev_info(dev, "%s: resume: we're back\n", sdio_func_id(func));

    return ret;
}

static const struct dev_pm_ops if_sdio_pm_ops = {
    .suspend    = if_sdio_suspend,
    .resume     = if_sdio_resume,
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */

static struct sdio_driver if_sdio_driver = {
    .name       = "lynx_sdio",
    .id_table   = if_sdio_ids,
    .probe      = if_sdio_probe,
    .remove     = if_sdio_remove,
#ifdef LYNX_NO_PM
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    .drv = {
        .pm = &if_sdio_pm_ops,
    },
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */
#endif
};

#ifdef CONFIG_PROC_FS

#define MAX_ARGV 8
static int get_args(const char *string, char *argvs[])
{
    char *p;
    int n;

    argvs[0]=0;
    n = 0;
//  memset ((void*)argvs, 0, MAX_ARGV * sizeof (char *));
    p = (char *) string;
    while (*p == ' ')
        p++;
    while (*p)
    {
        argvs[n] = p;
        while (*p != ' ' && *p)
            p++;
        if (0==*p)
            goto out;
        *p++ = '\0';
        while (*p == ' ' && *p)
            p++;
out:
        n++;
        if (n == MAX_ARGV)
            break;
    }
    return n;
}

static int _sdio_cmd(int argc, char *argv[])
{
    if (!sdio_card)
        goto fail;

    if (argc < 1)
    {
        goto fail;
    }
    else if (argc == 1) {

        if (!strcmp(argv[0], "on")) {

        } else if (!strcmp(argv[0], "tx")) {



        } else if (!strcmp(argv[0], "int")) {
            int err_code = 0;
            sdio_claim_host(sdio_card->func);
            sdio_f0_readb(sdio_card->func, 0xff, &err_code);
            sdio_release_host(sdio_card->func);
         } else if (!strcmp(argv[0], "td")) {


        } else if (!strcmp(argv[0], "stop")) {
            //struct if_sdio_card *card = sdio_card;
            int_stop = !int_stop;
            pr_err("%s() control interrupt to do transfer or not (int_stop : %d)\n", __func__, int_stop);
        }
    }
    else if (argc == 2)
    {
        ;
    }
    return 0;

fail:
    printk(KERN_ERR "sdio cmd is failed!!\n");
    return 0;
}
static ssize_t proc_sdio_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct if_sdio_card *card;
    char *start, *p;
    int ret;
    
    if (!sdio_card)
        return 0;
    if (!(start = os_api_alloc(PAGE_SIZE, GFP_KERNEL)))
        return -ENOMEM;

    card = sdio_card;

    p = start;
    p += sprintf(p, "TX debug Information:\n");
    p += sprintf(p, "Firmware Information:\n");

    ret = simple_read_from_buffer(buf, count, ppos, start, strlen(start));
    kfree(start); 
    return ret;
}
static ssize_t proc_sdio_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char buf[300];
    int rc;
    int argc ;
    char * argv[MAX_ARGV] ;

    if (count > 0 && count < 299) {
        if (copy_from_user(buf, buffer, count))
            return -EFAULT;
        buf[count-1] = '\0';
        argc = get_args( (const char *)buf, argv );
        rc = _sdio_cmd(argc, argv);
    }
    return count;
}

static const struct file_operations sdio_proc_fops = {
    .read       = proc_sdio_read,
    .write      = proc_sdio_write,
    .owner      = THIS_MODULE,
};
#endif

/*******************************************************************/
/* Module functions                                                */
/*******************************************************************/

static int __init if_sdio_init_module(void)
{
    int ret = 0;

#ifdef CONFIG_PROC_FS
        struct proc_dir_entry *res;
    
        if(!(res = proc_create("sdio", S_IWUSR | S_IRUGO, NULL, &sdio_proc_fops)))
            return -ENOMEM;
#endif

    printk(KERN_INFO "Montage SDIO: Lynx SDIO driver\n");

    ret = sdio_register_driver(&if_sdio_driver);

    (void) sdio_hexdump;

    return ret;
}

static void __exit if_sdio_exit_module(void)
{
#ifdef CONFIG_PROC_FS
        remove_proc_entry("sdio", NULL);
#endif

    sdio_unregister_driver(&if_sdio_driver);

    printk(KERN_INFO "Montage SDIO: Lynx SDIO driver unload!!\n");
}

module_init(if_sdio_init_module);
module_exit(if_sdio_exit_module);

MODULE_AUTHOR("Montage");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Lynx driver for 802.11n wireless devices");
MODULE_FIRMWARE(FIRMWARE_LYNX_2_0_0);
MODULE_FIRMWARE(FIRMWARE_LYNX_3_0_0);
MODULE_DEVICE_TABLE(sdio, if_sdio_ids);

