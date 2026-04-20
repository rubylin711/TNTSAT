/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_MISC_H__
#define __MT_UNF_MISC_H__

/* !!! CAUTION: MUST same with "clock.h" !!! */

/* HAL Module(Analog/Clock) ID */
enum
{
/*!
  CPU0
 */
  HAL_CPU0 = 0,
/*!
  CPU1
 */
  HAL_CPU1,
/*!
  CPU2
 */
  HAL_CPU2,
/*!
  High speed bus
 */
  HAL_HB,
/*!
  Peripheral low speed bus
 */
  HAL_PB,
/*!
  Graphic
 */
  HAL_GPE,
/*!
  Audio output
 */
  HAL_AUDIO_OUT,
 /*!
  SPDIF output
 */
  HAL_SPDF,
 /*!
  VBI
 */
  HAL_VBI,
/*!
  SD video
 */
  HAL_SD_VIDEO,
/*!
  HD video
 */
  HAL_HD_VIDEO,
/*!
  OCP Clock
 */
  HAL_OCP,
/*!
  VDEC Clock
 */
  HAL_VDEC,
/*!
  JPEG Clock
 */
  HAL_JPEG,
/*!
  Display Clock
 */
  HAL_DISPLAY,
/*!
  DI Clock
 */
  HAL_DI,
  /*!
 Osdc  Clock
 */
  HAL_OSDC,
 /*!
  DMA Clock
 */
  HAL_DMA,
/*!
  TSI Clock
 */
  HAL_TSI,
/*!
  TSI CSA3.0
 */
  HAL_TSI_CSA30,
/*!
  DS DES symphony2
 */
  HAL_DS_DES,
/*!
  DS TDES symphony2
 */
  HAL_DS_TDES,
/*!
  DS AES symphony2
 */
  HAL_DS_AES,
/*!
  DS CSA3 symphony2
 */
  HAL_DS_CSA3,
/*!
  DS CSA2 symphony2
 */
  HAL_DS_CSA2,
/*!
  DS SECHD1 symphony2
 */
  HAL_DS_SECHD1,
/*!
  KL PVR symphony2
 */
  HAL_KL_PVR,
/*!
  CRYPTO DES symphony2
 */
  HAL_CRYPTO_DES,
/*!
  CRYPTO TDES symphony2
 */
  HAL_CRYPTO_TDES,
/*!
  CRYPTO AES symphony2
 */
  HAL_CRYPTO_AES,
/*!
  CRYPTO SHA symphony2
 */
  HAL_CRYPTO_SHA,
/*!
  CRYPTO RSA symphony2
 */
  HAL_CRYPTO_RSA,
/*!
  SECHD0 symphony2
 */
  HAL_SECHD0,
/*!
  KT symphony2
 */
  HAL_KT,
/*!
  KL CW symphony2
 */
  HAL_KL_CW,
/*!
  Descramble,
 */
  HAL_DS,
/*!
  KDF symphony2
 */
  HAL_KDF,
/*!
  OTP PRELOAD symphony2
 */
  HAL_OTP_PRELOAD,
/*!
  SECURE Clock
 */
  HAL_SECURE,
/*!
  UART0 Clock
 */
  HAL_UART0,
/*!
  UART1 Clock
 */
  HAL_UART1,
 /*!
  SMC0 Clock
 */
  HAL_SMC0,
/*!
  SMC1 Clock
 */
  HAL_SMC1,
 /*!
  SPI0 Clock
 */
  HAL_SPI0,
/*!
  SPI1 Clock
 */
  HAL_SPI1,
/*!
  SPI2 Clock
 */
  HAL_SPI2,
/*!
  SDMMC Clock
 */
  HAL_SDMMC,
/*!
  IRDA Clock
 */
  HAL_IRDA,
/*!
  LEDKB Clock
 */
  HAL_LEDKB,
/*!
  EPI Clock
 */
  HAL_EPI,
/*!
  I2C Clock
 */
  HAL_I2C,
 /*!
  I2C Clock
 */
  HAL_I2C0,
  /*!
  I2C Clock
 */
  HAL_I2C1,
  /*!
  I2C debug Clock
 */
  HAL_I2C_DEBUG,
/*!
  TIMER Clock
 */
  HAL_TIMER,
/*!
  WATCHDOG Clock
 */
  HAL_WATCHDOG,
  /*!
  CRYPTO
 */
  HAL_CRYPTO,
  /*!
  GLITCH_DET symphony2
 */
  HAL_GLITCH_DET,

#if 0
   /*!
  SPDIF
 */
  HAL_CLK_SPDIF,
#endif

  /*!
  HDMI
 */
  HAL_HDMI,
  /*!
  MAC
 */
  HAL_MAC,
  /*!
  MAC RMII
 */
  HAL_MAC_RMII,
  /*!
  USB0
 */
  HAL_USB0,				/* Port 0, USB Protocol 2.0 */
  /*!
  USB1
 */
  HAL_USB1,				/* Port 1, USB Protocol 3.0 */
  /*!
  AO LPMSET symphony2
 */
  HAL_AO_LPMSET,
  /*!
  AO PINMUX symphony2
 */
  HAL_AO_PINMUX,
  /*!
  AO GPIO symphony2
 */
  HAL_AO_GPIO,
  /*!
  AO ANA symphony2
 */
  HAL_AO_ANA,
  /*!
  AO RECRAM symphony2
 */
  HAL_AO_RECRAM,
  /*!
  AO KADC symphony2
 */
  HAL_AO_KADC,
  /*!
  AO RTC symphony2
 */
  HAL_AO_RTC,
  /*!
  AO MAILBOX symphony2
 */
  HAL_AO_MAILBOX,
  /*!
  AO TIMER1 symphony2
 */
  HAL_AO_TIMER1,
  /*!
  AO TIMER0 symphony2
 */
  HAL_AO_TIMER0,
  /*!
  AO AOMCU symphony2
 */
  HAL_AO_AOMCU,
  /*!
  AO IRDA symphony2
 */
  HAL_AO_IRDA,
  /*!
  AO LEDKB symphony2
 */
  HAL_AO_LEDKB,
  /*!
  AO FPI2C symphony2
 */
  HAL_AO_FPI2C,
  /*!
  AO FPSPI symphony2
 */
  HAL_AO_FPSPI,
   /*!
  AO FPSPI symphony2
 */
  HAL_AO_FPSPIREG,
  /*!
  AO MNT
 */
  HAL_AO_MNT,
  /*!
  AO CEC
 */
  HAL_AO_CEC,
  /*!
  AO AVS
 */
  HAL_AO_AVS,
  /*!
  AO AGTIMER
 */
  HAL_AO_AGTIMER,
  /*!
  AO WATCHDOG
 */
  HAL_AO_WDOG,
  /*!
  AO MCU
 */
  HAL_AO_MCU,
  /*!
  DEMO symphony2
 */
  HAL_DEMO,
   /*!
  DEMO-C
 */
  HAL_DEMO_C,
  /*!
  DEMO-S
 */
  HAL_DEMO_S,
    /*!
  CADC
 */
  HAL_CADC,
    /*!
  SADC
 */
  HAL_SADC,
    /*!
  ADAC
 */
  HAL_ADAC,
   /*!
  VDAC0
 */
  HAL_VDAC0,
    /*!
  VDAC1
 */
  HAL_VDAC1,
    /*!
  VDAC2
 */
  HAL_VDAC2,
    /*!
  VDAC3
 */
  HAL_VDAC3,
  /*!
  RNG
 */
  HAL_RNG,
  /*!
  RNG2 symphony2
 */
  HAL_RNG2,
  /*!
  TSENSOR symphony2
 */
  HAL_TSENSOR,
  /*!
  Process Monitor symphony2
 */
  HAL_PM,
  /*!
  CPU_PLL(ADC_PLL)
 */
  HAL_PLL_CPU,
  /*!
  arch timer symphony4
 */
  HAL_ARCH_TIMER,
  /*!
  SPDMA Clock
 */
  HAL_SPDMA,
  /*!
  hal intf
 */
  HAL_INTF,
  /*!
  hal ddr
 */
  HAL_DDRMC,
  /*!
  hal mnt
 */
  HAL_MNT,
  /*!
  hal sdio
 */
  HAL_SDIO0,
  /*!
  hal sdio
 */
  HAL_SDIO1,
  /*!
  hal pnand
 */
  HAL_PNAND,
  /*!
  hal lcd
 */
  HAL_LCD,
  /*!
  hal png
 */
  HAL_PNG,
  /*!
  hal dai
 */
  HAL_DAI,
  /*!
  hal ci
 */
  HAL_CI,
  /*!
  hal ci
 */
  HAL_CITSIN,
  /*!
  hal xtal
 */
  HAL_XTAL,
  /*!
  hal m2m
 */
  HAL_M2M,
  /*!
  hal pka
 */
  HAL_PKA,
  /*!
  hal ifcp glb
 */
  HAL_IFCP_GLB,
   /*!
  hal ifcp glb
 */
  HAL_IFCP_KLM,
    /*!
  hal ifcp glb
 */
  HAL_IFCP_CRYPTO,
    /*!
  hal ifcp glb
 */
  HAL_IFCP_SYS,
/*!
  TIMER Clock 0
 */
  HAL_TIMER0,
/*!
  TIMER Clock 1
 */
  HAL_TIMER1,
/*!
  TIMER Clock 2
 */
  HAL_TIMER2,
/*!
  TIMER Clock 3
 */
  HAL_TIMER3,

/*!
  WATCHDOG Clock
 */
  HAL_WATCHDOG0,

/*!
  WATCHDOG Clock
 */
  HAL_WATCHDOG1,


 /*!
  ts0
 */
  HAL_TS0,

 /*!
  ts1
 */
  HAL_TS1,

 /*!
  ts2
 */
  HAL_TS2,

 /*!
  ts3
 */
  HAL_TS3,

 /*!
  demux
 */
  HAL_DEMUX,

   /*!
  t2mi
 */
  HAL_T2MI,

  /*!
  av sync
 */
  HAL_TSI_AVSYNC,
  /*!
  tsi sf
 */
  HAL_TSI_SF,
  /*!
  tsi sf
 */
 HAL_TSI_TRPP,
  /*!
  tsi swtsi
  */
  HAL_TSI_SWTSI,
  /*!
  tsi tspool
  */
  HAL_TSI_TSPOOL,
  /*!
  smc
  */
  HAL_SMC,
  /*!
  axi
  */
  HAL_AXI,
  /*!
  axi debug
  */
  HAL_AXI_DEBUG,
  /*!
  axi reg
  */
  HAL_AXI_REG,
  /*!
  lcdc
  */
  HAL_LCDC,
  /*!
  lcdhd
  */
  HAL_LCDHD,
  /*!
  lcd2x
  */
  HAL_LCD2X,
  /*!
  xtal mode
  */
  HAL_XTAL_MODE,
  /*!
  audio mclk
  */
  HAL_AUDIO_MCLK,
  /*!
  tempsensor
  */
  /*HAL_TEMPSENSOR,*/
  /*!
  panther2
  */
  HAL_PANTHER2,
  /*!
  demod j83b
  */
  HAL_DEMO_J83B,
  /*!
  GPU
  */
  HAL_GPU,
  /*!
  GMAC
  */
  HAL_GMAC,
  /*!
  AO UART
  */
  HAL_AO_UART,

/* 20250115 Added */
  //HAL_ADAC_DIG,	  	/* ADAC(AOUT) Digital */
  //HAL_USB0_DIG,		/* USB0 Digital */
  //HAL_USB1_DIG,	  	/* USB1 Digital */
  //HAL_HDMI_DIG,	 	/* HDMI Digital */

  //HAL_EPHY,
  HAL_EPHYPLL,
  //HAL_AUDIOPLL,
  HAL_DEMO_BUS,
  //HAL_USB1_30PHY,
  HAL_USB1_P20,			/* Port 1, USB Protocol 2.0 */

  /*!
  Maximal module number
  */
  HAL_MODULES_NUM,

  HAL_AVCPU = HAL_CPU1,
  HAL_USB0_P20 = HAL_USB0,
  HAL_USB1_P30 = HAL_USB1,

  HAL_ADCPLL = HAL_PLL_CPU,

};

/**
 @deprecate
 */
#if 0
typedef enum{
    MAC_CLK_MODE_25M_MII_E = 0,
    MAC_CLK_MODE_50M_RMII_E = 1,
}mac_clk_mode_e;
typedef enum{
    BUS_AXI_CLOCK = 0,
    BUS_AXI_CLOCK_1
}bus_axi_clk_mode_e;
typedef enum{
    AXI_CLK_DPHY_DIV_2 = 0,
    AXI_CLK_DPHY_DIV_2D5,
    AXI_CLK_DPHY_DIV_2D75,
    AXI_CLK_DPHY_DIV_3,
    AXI_CLK_DPHY_DIV_3D5,
    AXI_CLK_DPHY_DIV_8,
    AXI_CLK_RESV = 8,
    AXI_CLK_240M,
    AXI_CLK_120M,
    AXI_CLK_60M,
    AXI_CLK_30M,
    AXI_CLK_444M
}axi_clk_sel_e;
typedef enum{
    APB_CLK_80M = 0,
    APB_CLK_64M,
    APB_CLK_90M,
    APB_CLK_45M
}apb_clk_sel_e;
typedef enum{
    AHB_CLK_206M = 0,
    AHB_CLK_RESV,
    AHB_CLK_144M,
    AHB_CLK_90M,
    AHB_CLK_60M,
    AHB_CLK_30M,
}ahb_clk_sel_e;
typedef enum{
    CPU_CLK_DIV2= 0,
    CPU_CLK_DIV3,
    CPU_CLK_DIV4,
    CPU_CLK_DIV6,
    CPU_CLK_DIV8
}apcpu_debug_clk_sel_e;
typedef enum{
    APBACKUP_CLK_1440M= 0,
    APBACKUP_CLK_1080M,
    APBACKUP_CLK_960M,
    APBACKUP_CLK_720M,
    APBACKUP_CLK_480M,
    APBACKUP_CLK_1280M,
    APBACKUP_CLK_1000M,
    APBACKUP_CLK_810M,
    APBACKUP_CLK_576M,
    APBACKUP_CLK_360M,
    APBACKUP_CLK_120M
}apcpu_clk_sel0_e;

typedef enum{
    VCO_SEL_3G= 0,
    VCO_SEL_2G,
    VCO_SEL_1D6G,
    VCO_SEL_1G
}apcpu_vco_sel_e;

typedef enum{
    DC_TEST_VERF_BOTTOM= 0,
    DC_TEST_VERF_TOP,
    DC_TEST_VDD_VCO,
    DC_TEST_VCON
}dc_test_sel_e;

typedef enum{
    DC_TEST_CLK_FIX_DIG= 0,
    DC_TEST_CLK_REF=0x8,
    DC_TEST_VDD_VCO_DIV3=0xA,
    DC_TEST_VDD_VCO_DIV1=0xC,
    DC_TEST_VDD_VCO_DIV2=0xD,
    DC_TEST_VDD_VCO_DIV4=0XE,
    DC_TEST_VDD_VCO_DIV8=0XF
}apcpu_clk_sel1_e;

typedef enum{
    NCOREPORESET0= 0,
    NCOREPORESET1,
    NCORERESET0,
    NCORERESET1,
    NDBGRESET0,
    NDBGRESET1,
    I2CRESET,
    MBISTRESET_RSTN,
    APCPU_APB,
    APCPU_AHB,
    APCPU_CSRST,
    APCPU_NPOTRST,
    APCPU_NTRST
}apcpu_reset_e;

/*module clk set ,sel  or mode set */
typedef enum{
 AVCPU_CLKSEL_RESV=0,
 AVCPU_CLKSEL_667M,
 AVCPU_CLKSEL_576M,
 AVCPU_CLKSEL_480M,
 AVCPU_CLKSEL_360M,
 AVCPU_CLKSEL_288M,
 AVCPU_CLKSEL_180M,
 AVCPU_CLKSEL_90M,
}avcpu_clksel_e;

typedef enum{
 DMA_CLKESEL_262M=0,
 DMA_CLKESEL_288M,
 DMA_CLKESEL_131M,
 DMA_CLKESEL_206M,
 DMA_CLKESEL_XTAL = 0x6
}sys_dma_clksel_e;

typedef enum{
 UARTPHY_CLKSEL_XTAL=0,//24M/27M/40M
 UARTPHY_CLKSEL_120M
}intf_uartphy_clksel_e;

typedef enum{
 SDIO_CLKSEL_100M=0,
 SDIO_CLKSEL_90M
}intf_sdio_clksel_e;


typedef enum{
 PNAND_CLKSEL_XTAL=0,
 PNAND_CLKSEL_288M,
 PNAND_CLKSEL_262M,
 PNAND_CLKSEL_200M
}intf_pnand_clksel_e;

typedef enum{
 SPI_CLKSEL_XTAL=0,
 SPI_CLKSEL_360M,
 SPI_CLKSEL_288M,
 SPI_CLKSEL_400M
}intf_spi_clksel_e;

typedef enum{
 SMC_CLKSEL_90M=0,
 SMC_CLKSEL_XTAL_DIV2
}intf_smc_phyclksel_e;

typedef enum{
 GRA_CLKSEL_262M=0,
 GRA_CLKSEL_360M,
 GRA_CLKSEL_RESV,
 GRA_CLKSEL_320M,
 GRA_CLKSEL_XTAL=0x7
}gra_clksel_e;


typedef enum{
 JPG_CLKSEL_RESV0=0,
 JPG_CLKSEL_206M,
 JPG_CLKSEL_144M,
 JPG_CLKSEL_RESV1,
 JPG_CLKSEL_XTAL=0x6,
}jpg_clksel_e;


typedef enum{
 DISPDI_CLKSEL_288M=0,
 DISPDI_CLKSEL_262M,
 DISPDI_CLKSEL_160M,
 DISPDI_CLKSEL_131M,
 DISPDI_CLKSEL_80M,
 DISPDI_CLKSEL_60M
}dispdi_clksel_e;

typedef enum{
 DISPOSDC_CLKSEL_206M=0,
 DISPOSDC_CLKSEL_250M,
 DISPOSDC_CLKSEL_288M,
 DISPOSDC_CLKSEL_144M,
 DISPOSDC_CLKSEL_80M,
 DISPOSDC_CLKSEL_60M
}disposdc_clksel_e;


typedef enum{
 DISPCORE_CLKSEL_RESV=0,
 DISPCORE_CLKSEL_206M,
 DISPCORE_CLKSEL_180M,
 DISPCORE_CLKSEL_131M,
 DISPCORE_CLKSEL_40M,
 DISPCORE_CLKSEL_60M
}dispcore_clksel_e;

typedef enum{
 VDEC_CLKSEL_262M=0,
 VDEC_CLKSEL_206M,
 VDEC_CLKSEL_131M,
 VDEC_CLKSEL_45M
}vdec_clksel_e;

typedef enum{
 AOUT_CLKSEL_288M=0,
 AOUT_CLKSEL_262M,
 AOUT_CLKSEL_206M,
 AOUT_CLKSEL_131M,
 AOUT_CLKSEL_XTAL = 0x7,
}aout_clksel_e;

typedef enum{
 VOUT_LCDC_CLKSEL_LCD2XCLK_DIV1=0,
 VOUT_LCDC_CLKSEL_LCD2XCLK_DIV2=2,
 VOUT_LCDC_CLKSEL_LCD2XCLK_DIV4=4,
 VOUT_LCDC_CLKSEL_LCD2XCLK_DIV6=6,
}vout_lcdc_clksel_e;

typedef enum{
 VOUT_LCDHD_CLKSEL_HDVENC_EN=0,
 VOUT_LCDHD_CLKSEL_LCDC_EN,
}vout_lcdhd_clksel_e;

typedef enum{
 VOUT_LCD_CLKSEL_64M=0,
 VOUT_LCD_CLKSEL_72M,
 VOUT_LCD_CLKSEL_80M,
 VOUT_LCD_CLKSEL_131M,
 VOUT_LCD_CLKSEL_138M,
 VOUT_LCD_CLKSEL_54M,
 VOUT_LCD_CLKSEL_VENC_OSCLK
}vout_lcd2x_clksel_e;

typedef enum{
 VOUT_HDVENC_CLKSEL_VENCOSCLK=0,
 VOUT_HDVENC_CLKSEL_VENCOSCLK_DIV2=4,
 VOUT_HDVENC_CLKSEL_VENCOSCLK_DIV4,
 VOUT_HDVENC_CLKSEL_VENCOSCLK_DIV6,
 VOUT_HDVENC_CLKSEL_VENCOSCLK_DIV8,
}vout_hdvenc_clksel_e;


typedef enum{
 PNG_CLKSEL_RESV0=0,
 PNG_CLKSEL_206M,
 PNG_CLKSEL_144M,
 PNG_CLKSEL_RESV1,
 PNG_CLKSEL_XTAL = 0x6,
}png_clksel_e;

typedef enum{
 TSI_CLKSEL_RESV=0,
 TSI_CLKSEL_262M,
 TSI_CLKSEL_206M,
 TSI_CLKSEL_131M
}tsi_clksel_e;

typedef enum{
 TS3_CLKSEL_PAD_CLK=0,
 TS3_CLKSEL_CICAM_TSOUT_CLK,
 TS3_CLKSEL_PAD_CLK_REVERSE,
 TS3_CLKSEL_CICAM_TSOUT_CLK_REVERSE
}ts3_clksel_mode_e;

typedef enum{
 TS2_CLKSEL_PAD_CLK=0,
 TS2_CLKSEL_DEMOC_CLK,
 TS2_CLKSEL_PAD_CLK_REVERSE,
 TS2_CLKSEL_DEMOC_CLK_REVERSE
}ts2_clksel_mode_e;

typedef enum{
 TS1_CLKSEL_PAD_CLK=0,
 TS1_CLKSEL_DEMOS_CLK,
 TS1_CLKSEL_PAD_CLK_REVERSE,
 TS1_CLKSEL_DEMOS_CLK_REVERSE
}ts1_clksel_mode_e;

typedef enum{
 TS0_CLKSEL_PAD_TS0_CLK=0,
 TS0_CLKSEL_PAD_TS0_CLK_REVERSE
}ts0_clksel_mode_e;

typedef enum{
 CI_CLKSEL_80M=0,
 CI_CLKSEL_120M
}ci_clksel_e;

typedef enum{
 CITSIN_CLKSEL_TS0=0,
 CITSIN_CLKSEL_TS1,
 CITSIN_CLKSEL_DEMOS_TS1,
 CITSIN_CLKSEL_DEMOC_TS2,
 CITSIN_CLKSEL_TS0_REVERT,
 CITSIN_CLKSEL_TS1_REVERT,
 CITSIN_CLKSEL_DEMOS_TS1_REVERT,
 CITSIN_CLKSEL_DEMOC_TS2_REVERT
}citsin_clksel_mode_e;

typedef enum{
 SECURE_M2M_CIPHER_CLKSEL_262M=0,
 SECURE_M2M_CIPHER_CLKSEL_240M,
 SECURE_M2M_CIPHER_CLKSEL_206M,
 SECURE_M2M_CIPHER_CLKSEL_131M
}secure_m2m_chiper_clksel_e;

typedef enum{
 SECURE_CLKSEL_262M=0,
 SECURE_CLKSEL_206M,
 SECURE_CLKSEL_240M,
 SECURE_CLKSEL_131M,
 SECURE_CLKSEL_XTAL = 7
}secure_clksel_e;

typedef enum{
 IFCP_CRYPTO_CLKSEL_RESV=0,
 IFCP_CRYPTO_CLKSEL_206M,
 IFCP_CRYPTO_CLKSEL_120M,
 IFCP_CRYPTO_CLKSEL_240M
}ifcp_crypto_clksel_e;

typedef enum{
 IFCP_SYS_CLKSEL_RESV0=0,
 IFCP_SYS_CLKSEL_RESV1,
 IFCP_SYS_CLKSEL_RESV2,
 IFCP_SYS_CLKSEL_288M,
 IFCP_SYS_CLKSEL_240M,
 IFCP_SYS_CLKSEL_180M,
 IFCP_SYS_CLKSEL_90M,
 IFCP_SYS_CLKSEL_24M
}ifcp_sys_clksel_e;


typedef enum{
 DDR_PLL_CLK_POSITIVE=0,
 DDR_PLL_CLK_NEGATIVE
}sys_ddr_pll_clk_e;


typedef enum{
    CLK_SEL_RET_E = 0,
    CLK_SEL_REVERT_RET_E = 1,
    CLK_SEL_ORIG_E = 2,
    CLK_SEL_REVERT_ORIG_E = 3,
}mac_rmii_clk_mode_e;


typedef enum{
    XTAL_CLK_OSC = 0,
    XTAL_CLK_SECURITY_OSC = 1,
}crm_xtal_sel_e;

#define MT_UNF_SW_PIN_COUNT		106
#define MT_UNF_AO_PIN_COUNT		9

/*!
 symphony PINMUX reg
  */
enum
{
  /*!
    0.	SW_PIN0, symphony SW_PIN_BASE + 0x00
    */
  MT_UNF_SW_PIN0 = 0,
  /*!
    1.	SW_PIN1, symphony SW_PIN_BASE + 0x04
    */
  MT_UNF_SW_PIN1,
   /*!
    2.	SW_PIN2, symphony SW_PIN_BASE + 0x08
    */
  MT_UNF_SW_PIN2,
  /*!
    3.	SW_PIN3, symphony SW_PIN_BASE + 0x0c
    */
  MT_UNF_SW_PIN3,
   /*!
    4.	SW_PIN4, symphony SW_PIN_BASE + 0x10
    */
  MT_UNF_SW_PIN4,
     /*!
    5.	SW_PIN5, symphony SW_PIN_BASE + 0x14
    */
  MT_UNF_SW_PIN5,
     /*!
    6.	SW_PIN6, symphony SW_PIN_BASE + 0x18
    */
  MT_UNF_SW_PIN6,
     /*!
    7.	SW_PIN7, symphony SW_PIN_BASE + 0x1c
    */
  MT_UNF_SW_PIN7,

  /*!
   Maximum SW_PIN Index(exclude)
  */
  MT_UNF_SW_PIN_MAX = MT_UNF_SW_PIN0 + MT_UNF_SW_PIN_COUNT,

  /*!
    0x100.	AO_PIN0, symphony AO_PIN_BASE + 0x00
    */
  MT_UNF_AO_PIN0 = 0x100,
  /*!
    0x101.	AO_PIN1, symphony AO_PIN_BASE + 0x04
   */
  MT_UNF_AO_PIN1,

  /*!
   Maximum AO_PIN Index(exclude)
  */
  MT_UNF_AO_PIN_MAX = MT_UNF_AO_PIN0 + MT_UNF_AO_PIN_COUNT,

  /*!
    0x200.	I2C_PIN2, symphony I2C_PIN_BASE + 0x00
    */
  MT_UNF_I2C_PIN0 = 0x200,

  /*!
    0x300.	SMC_PIN0, symphony4 SMC_PIN_BASE + 0x00
    */
  MT_UNF_SMC_PIN0 = 0x300

};

#endif

/* Misc device name */
#define MT_MISC_DEVNAME "/dev/mt_misc"

/**
* brief get PINMUX value:
* attention:
* param[in] idx        index of the PINMUX.
* 		see PINMUX_INDEX_E(mt_drv_pinctrl.h).
* param[in] offset     (not used)
* param[in] len        (not used)
* param[out] val       PINMUX function mux value.
* retval :: MT_SUCCESS/MT_FAILURE  success/failure.
**/
int mt_unf_misc_pinmux_get(int idx, int offset, int len, unsigned int *val);

/**
* brief set PINMUX value.
* attention:
* param[in] idx        index of the PINMUX.
* 		see PINMUX_INDEX_E(mt_drv_pinctrl.h).
* param[in] offset     (not used)
* param[in] len        (not used)
* param[in] val        PINMUX function mux value.
* retval :: MT_SUCCESS/MT_FAILURE  success/failure.
**/
int mt_unf_misc_pinmux_set(int idx, int offset, int len, unsigned int val);

/**
 @deprecate
 */
int mt_unf_misc_pinmux_init(void);

/**
* brief get register bits value.
* attention:
* param[in] idx        address of the register.
* param[in] offset     offset of bits, 0-31.
* param[in] len        bits width, 1-32.
* param[out] val       register bits value.
* retval :: MT_SUCCESS/MT_FAILURE  success/failure.
**/
int mt_unf_misc_reg_get(unsigned int idx, int offset, int len, unsigned int *val);

/**
* brief set register bits value.
* attention:
* param[in] idx        address of the register.
* param[in] offset     offset of bits, 0-31.
* param[in] len        bits width, 1-32.
* param[in] val        register bits value.
* retval :: MT_SUCCESS/MT_FAILURE  success/failure.
**/
int mt_unf_misc_reg_set(unsigned int idx, int offset, int len, unsigned int val);

/**
* brief enable or disable module.
* attention:
* param[in] mid        module id, 0-HAL_MODULES_NUM.
* param[in] on         module status, 0: disable, 1: enable.
* retval :: MT_SUCCESS/MT_FAILURE  success/failure.
**/
int mt_unf_misc_module_set(int mid, int on);

/**
* brief reset module.
* attention:
* param[in] mid        module id, 0-HAL_MODULES_NUM.
* retval :: MT_SUCCESS/MT_FAILURE  success/failure.
**/
int mt_unf_misc_module_reset(int mid);

/**
* brief get module status.
* attention:
* param[in] mid        module id, 0-HAL_MODULES_NUM.
* param[out] en        module status, 0: disable, 1: enable.
* retval :: MT_SUCCESS/MT_FAILURE  success/failure.
**/
int mt_unf_misc_module_is_enabled(int mid, int *en);

/**
 @deprecate
 */
int mt_unf_misc_module_clk_set(int mid, int clk_idx);

/**
* brief get module clock frequency.
* attention:
* param[in] mid        module id, 0-HAL_MODULES_NUM.
* param[out] clk       module clock frequency in unit of HZ.
* retval :: MT_SUCCESS/MT_FAILURE  success/failure.
**/
int mt_unf_misc_module_clk_get(int mid, unsigned long *clk);

/**
* brief read soc temperature
* param[out] temperature address to store temperature result.
*				the real temperature is "temperature/1000.temperature%1000" Celsius,
*				such as: 65123 => 65.123(C), -65123 => -65.123(C).
* retval :: MT_SUCCESS/MT_FAILURE  success/failed.
**/
int mt_unf_misc_temperature_get(long *temperature);

/**
 @deprecate
 */
int mt_unf_misc_chip_productinfo_get(unsigned char*buff,unsigned char len);

/**
* brief misc module initialize
* retval :: MT_SUCCESS/MT_FAILURE  success/failed.
**/
int mt_unf_misc_init(void);

/**
* brief misc module release
* retval :: none
**/
void mt_unf_misc_release(void);

#endif

