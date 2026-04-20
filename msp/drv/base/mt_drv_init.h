/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_DRV_INIT_H__
#define __MT_DRV_INIT_H__

#include "mt_common.h"

#ifdef MT_ANDROID_SECURITY_L2_SYSTEM_CHECK
extern int system_verify_init(void);
#else
static int __init system_verify_init(void) {return 0;}
#endif

#ifdef CONFIG_MT_MTEST
extern int mtest_drv_init(void);
#else
static int __init mtest_drv_init(void) {return 0;}
#endif

extern mt_s32 VPSS_DRV_ModInit(mt_void);

extern mt_s32 mtimer_drv_modinit(mt_void);

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
extern mt_s32 misc_drv_modinit(mt_void);
#else
static mt_s32 __init misc_drv_modinit(mt_void) {return 0;}
#endif

#ifdef CONFIG_MT_CHIP_ARIA
extern mt_s32 always_timer_drv_modinit(mt_void);
#else
static mt_s32 __init always_timer_drv_modinit(mt_void) {return 0;}
#endif

extern int HDMI_DRV_ModInit(void);
extern int sci_drv_modinit(void);

#ifdef CONFIG_MT_DEMUX
extern int DMX_DRV_ModInit(void);
#else
static int __init DMX_DRV_ModInit(void) {return 0;}
#endif

//TODO:
#ifdef CONFIG_MT_VO
extern int VDP_DRV_ModInit(void);
#else
static int __init VDP_DRV_ModInit(void) {return 0;}
#endif

extern int PQ_DRV_ModInit(void);
extern int i2c_drv_modinit(void);
extern int otp_modinit(void);

#ifdef CONFIG_MT_AVPLAY
extern int AVPLAY_DRV_ModInit(void);
#else
static int __init AVPLAY_DRV_ModInit(void) {return 0;}
#endif

#ifdef CONFIG_MT_VDEC
extern int VDEC_DRV_ModInit(void);
#else
static int __init VDEC_DRV_ModInit(void) {return 0;}
#endif

#ifdef CONFIG_MT_ADEC
extern int adec_drv_init(void);
#else
static int __init adec_drv_init(void) {return 0;}
#endif

#ifdef CONFIG_MT_SYNC
extern int SYNC_DRV_ModInit(void);
#else
static int __init SYNC_DRV_ModInit(void) {return 0;}
#endif

#ifdef CONFIG_MT_AIAO
extern int AIAO_DRV_ModInit(void);
#else
static int __init AIAO_DRV_ModInit(void) {return 0;}
#endif

#ifdef CONFIG_MT_ADEC
extern int ADEC_DRV_ModInit(void);
#else
#if !defined(CONFIG_MT_FPGA)
static int __init ADEC_DRV_ModInit(void) {return 0;}
#endif
#endif

#ifdef CONFIG_MT_MTFB
extern int MTFB_DRV_ModInit(void);
#else
static int __init MTFB_DRV_ModInit(void) {return 0;}
#endif

#ifdef CONFIG_MT_OSD
extern int drv_osd_layer_ModInit(void);
#else
static int __init drv_osd_layer_ModInit(void) {return 0;}
#endif

extern int dmac_modinit(void);
extern int JPEG_DRV_ModInit(void);
extern int IR_DRV_ModInit(void);

#ifdef CONFIG_MT_SPDMA
extern int spdma_drv_modinit(void);
#else
static int __init spdma_drv_modinit(void) {return 0;}
#endif


#ifdef CONFIG_MT_FRONTEND
extern int fe_drv_modinit(void);
#else
static int __init fe_drv_modinit(void) {return 0;}
#endif

extern int gpio_modinit(void);
extern int STANDBY_DRV_ModInit(void);

#ifdef CONFIG_MT_KEYLED
extern int KEYLED_DRV_ModInit(void);
#else
static int __init KEYLED_DRV_ModInit(void) {return 0;}
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)  || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
extern int ipc_modinit(void);
#else
static int __init ipc_modinit(void) {return 0;}
#endif

/* Symphony4/6: No RPC */
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
extern int rpc_modinit(void);
#endif

#if (defined (CONFIG_MT_PVR_SUPPORT)) && (defined (CONFIG_MT_PVR))
extern int PVR_DRV_ModInit(void);
#else
static int __init PVR_DRV_ModInit(void) {return 0;}
#endif

#if defined(CONFIG_MT_IPCS)
extern int ipcs_setup(void);
#else
static int __init ipcs_setup(void) {return 0;}
#endif

#if defined(CONFIG_MT_VSS)
extern int vss_setup(void);
#else
static int __init vss_setup(void) {return 0;}
#endif

#if (defined (CONFIG_MT_CIPHER_SCPU) && defined(CONFIG_MT_CIPHER_SUPPORT))
extern int cipher_scpu_init(void);
#else
static int __init cipher_scpu_init(void) {return 0;}
#endif

#if defined (CONFIG_MT_CRYPTOENGINE)
extern int crypto_engine_setup(void);
#else
static int __init crypto_engine_setup(void) {return 0;}
#endif

#if defined (CONFIG_MT_KEYLADDER)
extern int keyladder_init(void);
#else
static int __init keyladder_init(void) {return 0;}
#endif

#if defined (CONFIG_MT_CERT)
extern int cert_drv_init(void);
#else
static int __init cert_drv_init(void) {return 0;}
#endif

#if defined(CONFIG_MT_KEYTABLE)
extern int keytable_setup(void);
#else
static int __init keytable_setup(void) {return 0;};
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
extern int mt_ampshm_init(void);
#else
static int mt_ampshm_init(void) {return 0;}
#endif

#ifdef CONFIG_MT_TDE
extern mt_s32 TDE_DRV_ModInit(mt_void);
#else
static mt_s32 __init TDE_DRV_ModInit(mt_void) {return 0;}
#endif

#ifdef CONFIG_MT_WDG_SUPPORT
extern mt_s32 wdg_drv_modinit(mt_void);
#else
static mt_s32 __init wdg_drv_modinit(mt_void) {return 0;}
#endif

#ifdef CONFIG_MT_LXC_IPC
extern int lxc_ipc_drv_modinit(void);
#else
static int __init lxc_ipc_drv_modinit(void) {return 0;}
#endif

#ifdef CONFIG_MT_PNG
extern int PNG_DRV_ModInit(void);
#else
static int __init PNG_DRV_ModInit(void) {return 0;}
#endif

extern int SUPLAYER_DRV_ModInit(void);

extern int mt_analog_mod_init(void);
extern int mt_crm_mod_init(void);
extern int mt_pinctrl_mod_init(void);
extern int drv_snd_ao_module_init(void);

#ifdef CONFIG_MT_CHIP_VERIFICATION
extern int chipverify_mss_drv_init(void);
#else
static int __init chipverify_mss_drv_init(void) {return 0;}
#endif

#if defined(CONFIG_MT_OPC)
extern int mt_opc_setup(void);
#else
static int __init mt_opc_setup(void) {return 0;}
#endif

#endif /* __MT_DRV_INIT_H__ */
