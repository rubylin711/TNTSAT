/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_drv_clock.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT clock driver header file.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_DRV_CLOCK_H__
#define __INC_MT_DRV_CLOCK_H__

/* mt_drv_clock.h */
#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__UBOOT__)

#if defined(__KERNEL__)
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/seq_file.h>
#else
/* RTOS */
#include <sys_types.h>
#include <sys_define.h>
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#define ONE_MHZ					1000000UL

#ifndef UINT_MAX
#define UINT_MAX				0xFFFFFFFF
#endif

/* check null pointer */
#ifndef CHECK_NULL_PTR
#if defined(__KERNEL__)
#define CHECK_NULL_PTR(p)		do{ if (!(p)) { printk("[E]%s: null pointer!\n", __FUNCTION__); return (-1); } } while(0)
#else
/* RTOS */
#define CHECK_NULL_PTR(p)		do{ if (!(p)) { OS_PRINTF("[E]%s: null pointer!\n", __FUNCTION__); return (-1); } } while(0)
#endif
#endif

#endif

//---------------------------------------------------------------------------//

/*
 * Clock
 */
typedef char mt_clk_t;

#define MT_CLK_MAC			"mac_clk"
#define MT_CLK_RMII			"rmii_clk"
#define MT_CLK_AXI			"axi_clk"
#define MT_CLK_APB			"apb_clk"
#define MT_CLK_AHB			"ahb_clk"
#define MT_CLK_APCPU		"apcpu_clk"
#define MT_CLK_APBACKUP		"apbackup_clk"
#define MT_CLK_AVCPU		"avcpu_clk"			/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_DMA			"dma_clk"			/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_SPDMA		"spdma_clk"
#define MT_CLK_USB1			"usb1_clk"			/* USB1 All Clocks */
#define MT_CLK_USB0			"usb0_clk"			/* USB0 All Clocks */
#define MT_CLK_SDIO1		"sdio1_clk"
#define MT_CLK_SDIO0		"sdio0_clk"
#define MT_CLK_SPI0			"spi0_clk"
#define MT_CLK_PNAND		"pnand_clk"
#define MT_CLK_UART1		"uart1_clk"
#define MT_CLK_UART0		"uart0_clk"
#define MT_CLK_I2CDEBUG		"i2cdebug_clk"
#define MT_CLK_I2C1			"i2c1_clk"
#define MT_CLK_I2C0			"i2c0_clk"
#define MT_CLK_SMC			"smc_clk"
#define MT_CLK_GRA			"gra_clk"
#define MT_CLK_JPG			"jpg_clk"			/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_DISP			"disp_clk"			/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_DISP_DI		"disp_di_clk"		/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_VDEC			"vdec_clk"			/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_AOUT			"aout_clk"			/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_SPDIF		"spdif_clk"
#define MT_CLK_ADAC			"adac_clk"			/* Aout adac */
#define MT_CLK_HDMI			"hdmi_clk"			/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_HDVENC		"hdvenc_clk"
#define MT_CLK_VBI			"vbi_clk"
#define MT_CLK_SDVENC		"sdvenc_clk"
#define MT_CLK_PNG			"png_clk"			/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_DAI			"dai_clk"
#define MT_CLK_GPU			"gpu_clk"			/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_GMAC			"gmac_clk"
#define MT_CLK_TSI			"tsi_clk"			/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_CI			"ci_clk"

/* Secure */
#define MT_CLK_DSC			"dsc_clk"			/* descramble, <CN>Òþº¬¾§Õñ */
#define MT_CLK_M2M			"m2m_clk"			/* m2m cipher, <CN>Òþº¬¾§Õñ */
#define MT_CLK_PKA			"pka_clk"
#define MT_CLK_GLITCHDET	"glitchdet_clk"
#define MT_CLK_KT			"kt_clk"			/* key table */
#define MT_CLK_SECHD0		"sechd0_clk"
#define MT_CLK_KLE			"kle_clk"			/* key ledder */
#define MT_CLK_KDF			"kdf_clk"
#define MT_CLK_SECURE		"secure_clk"		/* <CN>Òþº¬¾§Õñ */

/* IFCP */
#define MT_CLK_IFCP_KLM		"ifcp_klm_clk"
#define MT_CLK_IFCP_CRYPTO	"ifcp_crypto_clk"	/* <CN>Òþº¬¾§Õñ */
#define MT_CLK_IFCP_SYS		"ifcp_sys_clk"

/* AO(Always On) */
#define MT_CLK_AO_LPM		"ao_lpm_clk"
#define MT_CLK_AO_IRDA		"ao_irda_clk"
#define MT_CLK_AO_LEDKB		"ao_ledkb_clk"
#define MT_CLK_AO_GPIO		"ao_gpio_clk"
#define MT_CLK_AO_KADC		"ao_kadc_clk"
#define MT_CLK_AO_ANAREG	"ao_anareg_clk"
#define MT_CLK_AO_FPI2C		"ao_fpi2c_clk"
#define MT_CLK_AO_FPSPI		"ao_fpspi_clk"
#define MT_CLK_AO_RECRAM	"ao_recram_clk"		/* record ram */
#define MT_CLK_AO_PINMUX	"ao_pinmux_clk"
#define MT_CLK_AO_TIMER		"ao_timer_clk"
#define MT_CLK_AO_MAILBOX	"ao_mailbox_clk"
#define MT_CLK_AO_CEC		"ao_cec_clk"
#define MT_CLK_AO_AVS		"ao_avs_clk"
#define MT_CLK_AO_AGTIMER	"ao_agtimer_clk"
#define MT_CLK_AO_MCU		"ao_mcu_clk"
#define MT_CLK_XTAL			"xtal_clk"

/* Others */
#define MT_CLK_DEMO			"demo_clk"			/* demodulator, demo bus */
#define MT_CLK_DEMO_C		"demo_c_clk"
#define MT_CLK_DEMO_S		"demo_s_clk"
#define MT_CLK_DEMO_J83B	"demo_j83b_clk"

#define MT_CLK_DDRPHY		"ddrphy_clk"

/* Internal Used */
#define HDMI_TMDS_CLK		"hdmi_tmds_clk"
#define HDMI_PIXNX_CLK		"hdmi_pixnx_clk"

/**
 * @brief initilize Clock module
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_clk_init(void);

/**
 * @brief get clock module reset status
 *
 * @param[in] clk clock name
 * @param[out] status module reset status, >0: reset, 0: release, <0: Not available
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_clk_get_reset_status(mt_clk_t *clk, int *status);

/**
 * @brief reset clock module(soft reset)
 *
 * @param[in] clk clock name
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_reset(mt_clk_t *clk);

/**
 * @brief reset clock module(soft reset), but not release
 *
 * @param[in] clk clock name
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_reset_reset(mt_clk_t *clk);

/**
 * @brief release reset clock module
 *
 * @param[in] clk clock name
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_reset_release(mt_clk_t *clk);

/**
 * @brief get Clock gate status
 *
 * @param[in] clk clock name
 * @param[out] status clock gate status
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_clk_get_status(mt_clk_t *clk, u32 *status);

/**
 * @brief enable Clock gate
 *
 * @param[in] clk clock name
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_clk_enable(mt_clk_t *clk);

/**
 * @brief disable Clock gate
 *
 * @param[in] clk clock name
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_clk_disable(mt_clk_t *clk);

/**
 * @brief get Clock rate
 *
 * @param[in] clk clock name
 *
 * @param[out] rate clock frequency, in unit of HZ.
 *            e.g. 720MHz, rate=720000000
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_get_rate(mt_clk_t *clk, unsigned long *rate);

/**
 * @brief set Clock rate
 *
 * @param[in] clk clock name
 * @param[in] rate clock frequency, in unit of HZ.
 *            e.g. 720MHz, rate=720000000
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_set_rate(mt_clk_t *clk, unsigned long rate);

/**
 * @brief get Clock mux select value
 *
 * Rate/Divider also are kinds of Mux,
 * could call this API to get register raw value too.
 *
 * @param[in] clk clock name
 * @param[out] mux clock mux select value
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_get_mux(mt_clk_t *clk, u32 *mux);

/**
 * @brief set Clock mux select value
 *
 * Rate/Divider also are kinds of Mux,
 * could call this API to set register raw value too.
 *
 * @param[in] clk clock name
 * @param[in] mux clock mux select value
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_set_mux(mt_clk_t *clk, u32 mux);

/**
 * @brief get Clock divisor
 *
 * @param[in] clk clock name
 * @param[out] div clock divisor, multiplied 100,
 *                 e.g. 1/2.5, div = 2.5*100 = 250
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_get_div(mt_clk_t *clk, u32 *div);

/**
 * @brief set Clock divisor
 *
 * @param[in] clk clock name
 * @param[in] div clock divisor, multiplied 100,
 *                 e.g. 1/2.5, div = 2.5*100 = 250
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_set_div(mt_clk_t *clk, u32 div);

/**
 * CAUTION!!!
 *     mt_clk_get_attr and mt_clk_set_attr,
 *   are used for get and set extra CLKSEL bits(not gate/rate/mux/divider bits),
 *   such as: "mac_clksel", etc.
 */

/**
 * @brief get extra Clock select bits
 *
 * @param[in] clk clock name
 * @param[out] value extra clock select value
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_get_attr(mt_clk_t *clk, u32 *value);

/**
 * @brief set extra Clock select bits
 *
 * @param[in] clk clock name
 * @param[in] value extra clock select value
 *
 * @return
 *	  0: success
 *	  !0: failure
 */
int mt_clk_set_attr(mt_clk_t *clk, u32 value);

/**
 * @brief enable Clocks auto gate
 *
 * @return
 *	   0: success
 *	  !0: failure
 */
int mt_clk_autogate_enable(void);

/**
 * @brief batch config clock gate/rate/mux/div/attr, etc.
 *
 * @param[in] cmd_line clock config cmd line, such as:
 *                 "clk=?,gate=[0|1],rate=?,mux=?,div=?,attr=?"
 *
 * @return
 *     0: success,
 *	  !0: failure
 */
#if defined(__UBOOT__)
int mt_clk_cfg_cmdline(const char *cmd_line);
#endif

#if defined(__KERNEL__) && !defined(__UBOOT__)

/**
 * @brief backup clock registers when suspend
 *
 * @return none
 */
void mt_clk_suspend(void);

/**
 * @brief restore clock registers when resume
 *
 * @return none
 */
void mt_clk_resume(void);

#endif

/**
 * @brief dump clock autogate state
 */
#if defined(__UBOOT__)
void mt_clk_dump_autogate(void);
#elif defined(__KERNEL__)
void mt_clk_dump_autogate(struct seq_file *s);
#else
/* RTOS */
void mt_clk_dump_autogate(int (*misc_printf)(const char *fmt, ...));
#endif

/**
 * @brief dump clocks state
 */
#if defined(__UBOOT__)
void mt_clk_dump_state(void);
#elif defined(__KERNEL__)
void mt_clk_dump_state(struct seq_file *s);
#else
/* RTOS */
void mt_clk_dump_state(int (*misc_printf)(const char *fmt, ...));
#endif

/**
 * @brief enable all TOP Clocks
 */
void mt_top_clk_enable_all(void);

/**
 * @brief enable TOP Clocks hw auto gate
 */
void mt_top_clk_hw_auto_gate_enable(void);

/*****************************************************************************/
/*                                                                           */
/* Composite(HAL) Module API                                                 */
/*   For Modules have both Analog and Digital Components,                    */
/*   such as: ADAC, USB, SMC, EMAC, etc.                                     */
/*                                                                           */
/*****************************************************************************/

#define FLAG_HAL_MOD_NONE				0x00U
#define FLAG_HAL_MOD_RESET_EXP			0x01U

/**
 * @brief get HAL module ID by name
 *
 * @param[in] name HAL module name, see: mt_unf_misc.h
 *
 * @retval HAL module ID
 */
int mt_hal_mod_get_id(const char *name);

/**
 * @brief get module status
 *
 * @param[in] id HAL module ID, see: mt_unf_misc.h
 * @param[out] status module status, 1: on, 0: off
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_hal_mod_get_status(int id, int *status);

/**
 * @brief enable module
 *
 * @param[in] id HAL module ID, see: mt_unf_misc.h
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_hal_mod_enable(int id);

/**
 * @brief disable module
 *
 * @param[in] id HAL module ID, see: mt_unf_misc.h
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_hal_mod_disable(int id);

/**
 * @brief get module clock rate
 *
 * @param[in] id HAL module ID, see: mt_unf_misc.h
 * @param[out] rate module clock rate
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_hal_mod_get_rate(int id, unsigned long *rate);

/**
 * @brief set module clock rate
 *
 * @param[in] id HAL module ID, see: mt_unf_misc.h
 * @param[in] rate module clock rate
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_hal_mod_set_rate(int id, unsigned long rate);

/**
 * @brief reset module
 *
 * @param[in] id HAL module ID, see: mt_unf_misc.h
 * @param[in] flag reset module flags
 *                 FLAG_HAL_MOD_NONE: module normal reset
 *                 FLAG_HAL_MOD_RESET_EXP: module exception reset
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_hal_mod_reset(int id, unsigned int flag);

/**
 * @brief get module status
 *
 * @param[in] id HAL module ID, see: mt_unf_misc.h
 * @param[out] status module reset status, 1: reset, 0: release
 *
 * @return
 *    0: success
 *    !0: failure
 */
int mt_hal_mod_get_reset_status(int id, int *status);

/**
 * @brief dump module state
 */
#if defined(__UBOOT__)
void mt_hal_mod_dump_state(void);
#elif defined(__KERNEL__)
void mt_hal_mod_dump_state(void *s);
#else
/* RTOS */
void mt_hal_mod_dump_state(int (*misc_printf)(const char *fmt, ...));
#endif

#ifdef __cplusplus
}
#endif

#endif

