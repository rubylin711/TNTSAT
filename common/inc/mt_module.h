/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/** @addtogroup   MODULES  */
/** @{ */ /** <!-- [MODULES] */
#ifndef __MT_MODULE_H__
#define __MT_MODULE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MT_INVALID_MODULE_ID (0xffffffff)
#define MT_MAX_USER_MODULE_NUMBER (256)

/** Module ID flags */
typedef enum mt_mod_id {
    MT_ID_STB = 0,

    /**< common. */ /**< CNcomment: 系统通用枚举数据常量 */
    MT_ID_SYS = 1,
    MT_ID_MODULE,
    MT_ID_LOG,
    MT_ID_PROC,
    MT_ID_MEM,
    MT_ID_STAT,
    MT_ID_PDM,
    MT_ID_MEMDEV,
    MT_ID_DEMUX = 0x0A,
    MT_ID_MISC = 0x0B,
	MT_ID_AMPSHM,

    /**< audio. */ /**< CNcomment: 音频部分常量区 */
    MT_ID_ADEC = 0x10,
    MT_ID_AO,
    MT_ID_SIO_AI,
    MT_ID_SIO_AO,
    MT_ID_SIO,
    MT_ID_AI,
    MT_ID_AENC,
    MT_ID_SRC,
    MT_ID_AIAO,
    MT_ID_AFLT,
    MT_ID_ADSP,
    MT_ID_AMP,
    MT_ID_SIF,
    MT_ID_SND,
    MT_ID_ADEC2,

    /**< video. */ /**< CNcomment: 视频部分常量区 */
    MT_ID_VFMW = 0x20,
    MT_ID_SVDEC,
    MT_ID_DISP,
    MT_ID_HDMI,
    MT_ID_VO,
    MT_ID_VPSS,
    MT_ID_VDEC,
    MT_ID_VI,
    MT_ID_VENC,
    MT_ID_PQ,
    MT_ID_EDID,
    MT_ID_VICAP,
    MT_ID_HDMIRX,
    MT_ID_VO_FW,
	MT_ID_SL_HDR,
    /**< graphics. */ /**< CNcomment: 图形部分常量区 */
    MT_ID_TDE = 0x30,
    MT_ID_JPGDEC,
    MT_ID_JPGENC,
    MT_ID_MTFB,
    MT_ID_PNG,
    MT_ID_MTGO,
    MT_ID_OSD,

    /**< player. */ /**< CNcomment: 播放相关部分常量区 */
    MT_ID_PVR = 0x40,
    MT_ID_AVPLAY,
    MT_ID_SYNC,
    MT_ID_VSYNC,
    MT_ID_ASYNC,
    MT_ID_FASTPLAY,
    MT_ID_SUPLAYER,

    /**< ecs. */ /**< CNcomment: 外设部分常量区 */
    MT_ID_FLASH = 0x50,
    MT_ID_IR,
    MT_ID_RTC,
    MT_ID_I2C,
    MT_ID_SCI,
    MT_ID_ETH,
    MT_ID_USB_PROTECT,
    MT_ID_WDG = 0x57, /* watch dog used 'W' */
    MT_ID_GPIO,
    MT_ID_GPIO_I2C,
    MT_ID_DMAC,
    MT_ID_PMOC,
    MT_ID_FRONTEND,
    MT_ID_KEYLED,
    MT_ID_E2PROM,
    MT_ID_CIPHER,
    MT_ID_OTP = 0x60,
    MT_ID_CA,
    MT_ID_PM,
    MT_ID_CI,
    MT_ID_CIMAXPLUS,
    MT_ID_TVP5150,
    MT_ID_SIL9293,
    MT_ID_PWM,
    MT_ID_SPI,
    MT_ID_TIMER,
    MT_ID_ATIMER,
    MT_ID_SPDMA,

    MT_ID_KT = 0x70,
    MT_ID_KEYLADDER,
    MT_ID_CE,
    MT_ID_VSS,

    /**< voip, bluetooth,alsa. */ /**<  CNcomment: VOIP、蓝牙部分常量区*/
    MT_ID_VOIP_HME = 0x80,
    MT_ID_NDPT,
    MT_ID_AUDADP,
    MT_ID_BT,
    MT_ID_ALSA,
    MT_ID_3G,

    /**< vp. */ /**<  CNcomment: VP常量区*/
    MT_ID_VP = 0x90,
    MT_ID_HDCP,

    /**< subtitle. */ /**<  CNcomment: 字幕常量区*/
    MT_ID_SUBT = 0x98,
    MT_ID_TTX,
    MT_ID_CC,

    /**< loader. */ /**< CNcomment: loader */
    MT_ID_LOADER = 0xA0,

    /**< opentv5. */ /**<  CNcomment: opentv5*/
    MT_ID_O5 = 0xA1,
    MT_ID_O5_AUDDEC,
    MT_ID_O5_CRYPTO,
    MT_ID_O5_DMX,
    MT_ID_O5_FPCHAR,
    MT_ID_O5_HDMI,
    MT_ID_O5_INJECT,
    MT_ID_O5_LED,
    MT_ID_O5_LINKER,
    MT_ID_O5_NOCS,
    MT_ID_O5_PD_WRITER,
    MT_ID_O5_RFMOD,
    MT_ID_O5_SCART,
    MT_ID_O5_SMARTCARD,
    MT_ID_O5_STB,
    MT_ID_O5_TUNER,
    MT_ID_O5_VIDDEC,
    MT_ID_O5_VIDENC,
    MT_ID_O5_SOC,
    MT_ID_O5_ENDDEF = 0xB4,

    /**< user definition. */ /**< CNcomment: 为整机保留的自定义区 */
    MT_ID_USR_START = 0xC0,
    MT_ID_USR_END = 0xFE,
    MT_ID_BUTT = 0xFF
} mt_mod_id_e;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

/** @} */ /** <!-- ==== group Definition end ==== */
