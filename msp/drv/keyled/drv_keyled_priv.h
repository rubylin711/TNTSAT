/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_KEYLED_PRIV_H__
#define __DRV_KEYLED_PRIV_H__
#include <mt_type.h>
#include <mt_unf_keyled.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>
#include <drv_keyled_ioctl.h>

typedef struct _keyled_ops_s
{
   MT_VOID (*keyled_init)(void *dev);
   MT_U32 (*get_keyval)(MT_VOID);
   MT_VOID (*display)(ulong param);
   MT_VOID (*display_lbd)(ulong param);
   MT_VOID (*release)(MT_VOID);
   MT_VOID (*set_brightness)(keyled_display_led_bright_t bright);
   MT_VOID (*display_asc)(ulong param);
   MT_VOID (*set_led_pos)(keyled_param_s param);
   MT_VOID (*set_led_map)(keyled_param_s param);
   MT_VOID (*display_special_ch)(ulong param);
   MT_VOID (*vfd_display_ch)(ulong param);
   /*init_kadc  release_kadc for x+kadc, begin*/
   MT_VOID (*keyled_init_kadc)(void *dev);
   MT_VOID (*release_kadc)(MT_VOID);
   /*init_kadc  release_kadc for x+kadc, end*/
#ifdef CONFIG_MT_FPGA
   MT_VOID (*vfd_reset)(MT_VOID);
#endif
} keyled_ops_s;

#define KEYLED_KEYSTATUS_PRESS 0x12
#define KEYLED_KEYSTATUS_IDLE   0x34
typedef struct _keyled_key_info_s
{
    MT_U8 key_status;
    MT_U32 keyval;
    MT_U32 jiffies;
} keyled_key_info_s;


typedef struct _keyled_info_s
{
    MT_U8 type;
    MT_U8 is_attach;
    MT_U8 keyup_en;
    MT_U8 repkey_en;
    MT_U8 bright_level;
    MT_U32 repkey_timeout;
    MT_U32 idle_keyval;
    keyled_ops_s kl_ops;
    keyled_key_info_s key_info;
    MT_UNF_KEYLED_KADC_TYPR_E kadc_type;

	MT_U8 is_init;	//indicate whether the keyled is inited, 1:has inited, 0:not
} keyled_info_s;

#define KEYLED_KEYVAL_RINGBUF_SIZE  128

typedef struct _keyled_keyfifo_s
{
	keyled_keyval_s kv_ringbuf[KEYLED_KEYVAL_RINGBUF_SIZE];
	int wp;
	int rp;
	int count;
	spinlock_t lock;
} keyled_keyfifo_s;

struct mt_ledkb_dev
{
	struct device *dev;
	void __iomem *base; /* virtual */
	struct clk *clk;
	u32 clk_khz;     /* Speed of freq in Khz */
};

struct mt_keyled_device {
	struct mt_ledkb_dev ledkb_dev;
	struct device *dev;
	struct cdev cdev;
	struct class *keyled_class;
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};


#define KEYLED_IRQ_NO   (69+32)

#if 0
#define  WRITE_REG_B(Addr, Value) ((*(volatile mt_u8 *)(Addr)) = (Value))
#define  READ_REG_B(Addr) (*(volatile mt_u8 *)(Addr))
#define  WRITE_REG(Addr, Value) ((*(volatile mt_u32 *)(Addr)) = (Value))
#define  READ_REG(Addr) (*(volatile mt_u32 *)(Addr))
#endif

MT_VOID keyled_fd650_attach(keyled_info_s *klinfo, int kadc);
MT_VOID keyled_ct1642_attach(keyled_info_s *klinfo, int kadc);
MT_VOID keyled_keyadc_attach(keyled_info_s *klinfo);
MT_VOID keyled_tt1629b_attach(keyled_info_s *klinfo, int kadc);
MT_VOID keyled_pt6393_attach(keyled_info_s *klinfo, int kadc);
MT_VOID keyled_fd612_attach(keyled_info_s *klinfo, int kadc);

#endif
