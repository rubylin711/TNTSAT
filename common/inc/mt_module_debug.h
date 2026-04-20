/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_MODULE_DEBUG_H__
#define __MT_MODULE_DEBUG_H__

#include "mt_module.h"
#include "mt_debug.h"


#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */


/*Define Debug Level For ADEC                 */
#define MT_FATAL_ADEC(fmt...)		MT_FATAL_PRINT(MT_ID_ADEC, fmt)
#define MT_ERR_ADEC(fmt...)		MT_ERR_PRINT(MT_ID_ADEC, fmt)
#define MT_WARN_ADEC(fmt...)		MT_WARN_PRINT(MT_ID_ADEC, fmt)
#define MT_INFO_ADEC(fmt...)		MT_INFO_PRINT(MT_ID_ADEC, fmt)
#define MT_DBG_ADEC(fmt...)		MT_DBG_PRINT(MT_ID_ADEC, fmt)

/*Define Debug Level For AENC                 */
#define MT_FATAL_AENC(fmt...)		MT_FATAL_PRINT(MT_ID_AENC, fmt)
#define MT_ERR_AENC(fmt...)		MT_ERR_PRINT(MT_ID_AENC, fmt)
#define MT_WARN_AENC(fmt...)		MT_WARN_PRINT(MT_ID_AENC, fmt)
#define MT_INFO_AENC(fmt...)		MT_INFO_PRINT(MT_ID_AENC, fmt)
#define MT_DBG_AENC(fmt...)		MT_DBG_PRINT(MT_ID_AENC, fmt)

/*Define Debug Level For AI                 */
#define MT_FATAL_AI(fmt...)		MT_FATAL_PRINT(MT_ID_AI, fmt)
#define MT_ERR_AI(fmt...)		MT_ERR_PRINT(MT_ID_AI, fmt)
#define MT_WARN_AI(fmt...)		MT_WARN_PRINT(MT_ID_AI, fmt)
#define MT_INFO_AI(fmt...)		MT_INFO_PRINT(MT_ID_AI, fmt)
#define MT_DBG_AI(fmt...)		MT_DBG_PRINT(MT_ID_AI, fmt)

/*Define Debug Level For AO                 */
#define MT_FATAL_AO(fmt...)		MT_FATAL_PRINT(MT_ID_AO, fmt)
#define MT_ERR_AO(fmt...)		MT_WARN_PRINT(MT_ID_AO, fmt)
#define MT_WARN_AO(fmt...)		MT_WARN_PRINT(MT_ID_AO, fmt)
#define MT_INFO_AO(fmt...)		MT_INFO_PRINT(MT_ID_AO, fmt)
#define MT_DBG_AO(fmt...)		MT_DBG_PRINT(MT_ID_AO, fmt)

/*Define Debug Level For AVPLAY                 */
#define MT_FATAL_AVPLAY(fmt...)		MT_FATAL_PRINT(MT_ID_AVPLAY, fmt)
#define MT_ERR_AVPLAY(fmt...)		MT_ERR_PRINT(MT_ID_AVPLAY, fmt)
#define MT_WARN_AVPLAY(fmt...)		MT_WARN_PRINT(MT_ID_AVPLAY, fmt)
#define MT_INFO_AVPLAY(fmt...)		MT_INFO_PRINT(MT_ID_AVPLAY, fmt)
#define MT_DBG_AVPLAY(fmt...)		MT_DBG_PRINT(MT_ID_AVPLAY, fmt)

/*Define Debug Level For CIPHER                 */
#define MT_FATAL_CIPHER(fmt...)		MT_FATAL_PRINT(MT_ID_CIPHER, fmt)
#define MT_ERR_CIPHER(fmt...)		MT_ERR_PRINT(MT_ID_CIPHER, fmt)
#define MT_WARN_CIPHER(fmt...)		MT_WARN_PRINT(MT_ID_CIPHER, fmt)
#define MT_INFO_CIPHER(fmt...)		MT_INFO_PRINT(MT_ID_CIPHER, fmt)
#define MT_DBG_CIPHER(fmt...)		MT_DBG_PRINT(MT_ID_CIPHER, fmt)

/*Define Debug Level For DEMUX                 */
#define MT_FATAL_DEMUX(fmt...)		MT_FATAL_PRINT(MT_ID_DEMUX, fmt)
#define MT_ERR_DEMUX(fmt...)		MT_ERR_PRINT(MT_ID_DEMUX, fmt)
#define MT_WARN_DEMUX(fmt...)		MT_WARN_PRINT(MT_ID_DEMUX, fmt)
#define MT_INFO_DEMUX(fmt...)		MT_INFO_PRINT(MT_ID_DEMUX, fmt)
#define MT_DBG_DEMUX(fmt...)		MT_DBG_PRINT(MT_ID_DEMUX, fmt)

/*Define Debug Level For FLASH                 */
#define MT_FATAL_FLASH(fmt...)		MT_FATAL_PRINT(MT_ID_FLASH, fmt)
#define MT_ERR_FLASH(fmt...)		MT_ERR_PRINT(MT_ID_FLASH, fmt)
#define MT_WARN_FLASH(fmt...)		MT_WARN_PRINT(MT_ID_FLASH, fmt)
#define MT_INFO_FLASH(fmt...)		MT_INFO_PRINT(MT_ID_FLASH, fmt)
#define MT_DBG_FLASH(fmt...)		MT_DBG_PRINT(MT_ID_FLASH, fmt)

/*Define Debug Level For FRONTEND                 */
#define MT_FATAL_FRONTEND(fmt...)		MT_FATAL_PRINT(MT_ID_FRONTEND, fmt)
#define MT_ERR_FRONTEND(fmt...)		MT_ERR_PRINT(MT_ID_FRONTEND, fmt)
#define MT_WARN_FRONTEND(fmt...)		MT_WARN_PRINT(MT_ID_FRONTEND, fmt)
#define MT_INFO_FRONTEND(fmt...)		MT_INFO_PRINT(MT_ID_FRONTEND, fmt)
#define MT_DBG_FRONTEND(fmt...)		MT_DBG_PRINT(MT_ID_FRONTEND, fmt)

/*Define Debug Level For HDMI                 */
#define MT_FATAL_HDMI(fmt...)		MT_FATAL_PRINT(MT_ID_HDMI, fmt)
#define MT_ERR_HDMI(fmt...)		MT_ERR_PRINT(MT_ID_HDMI, fmt)
#define MT_WARN_HDMI(fmt...)		MT_WARN_PRINT(MT_ID_HDMI, fmt)
#define MT_INFO_HDMI(fmt...)		MT_INFO_PRINT(MT_ID_HDMI, fmt)
#define MT_DBG_HDMI(fmt...)		MT_DBG_PRINT(MT_ID_HDMI, fmt)

/*Define Debug Level For IR                 */
#define MT_FATAL_IR(fmt...)		MT_FATAL_PRINT(MT_ID_IR, fmt)
#define MT_ERR_IR(fmt...)		MT_ERR_PRINT(MT_ID_IR, fmt)
#define MT_WARN_IR(fmt...)		MT_WARN_PRINT(MT_ID_IR, fmt)
#define MT_INFO_IR(fmt...)		MT_INFO_PRINT(MT_ID_IR, fmt)
#define MT_DBG_IR(fmt...)		MT_DBG_PRINT(MT_ID_IR, fmt)

/*Define Debug Level For MISC                 */
#define MT_FATAL_MISC(fmt...)		MT_FATAL_PRINT(MT_ID_MISC, fmt)
#define MT_ERR_MISC(fmt...)		MT_ERR_PRINT(MT_ID_MISC, fmt)
#define MT_WARN_MISC(fmt...)		MT_WARN_PRINT(MT_ID_MISC, fmt)
#define MT_INFO_MISC(fmt...)		MT_INFO_PRINT(MT_ID_MISC, fmt)
#define MT_DBG_MISC(fmt...)		MT_DBG_PRINT(MT_ID_MISC, fmt)

/*Define Debug Level For MTGO                 */
#define MT_FATAL_MTGO(fmt...)		MT_FATAL_PRINT(MT_ID_MTGO, fmt)
#define MT_ERR_MTGO(fmt...)		MT_ERR_PRINT(MT_ID_MTGO, fmt)
#define MT_WARN_MTGO(fmt...)		MT_WARN_PRINT(MT_ID_MTGO, fmt)
#define MT_INFO_MTGO(fmt...)		MT_INFO_PRINT(MT_ID_MTGO, fmt)
#define MT_DBG_MTGO(fmt...)		MT_DBG_PRINT(MT_ID_MTGO, fmt)

/*Define Debug Level For JPGE                 */
#define MT_FATAL_JPGE(fmt...)		MT_FATAL_PRINT(MT_ID_JPGENC, fmt)
#define MT_ERR_JPGE(fmt...)		MT_ERR_PRINT(MT_ID_JPGENC, fmt)
#define MT_WARN_JPGE(fmt...)		MT_WARN_PRINT(MT_ID_JPGENC, fmt)
#define MT_INFO_JPGE(fmt...)		MT_INFO_PRINT(MT_ID_JPGENC, fmt)
#define MT_DBG_JPGE(fmt...)		MT_DBG_PRINT(MT_ID_JPGENC, fmt)

/*Define Debug Level For JPEG                 */
#define MT_FATAL_JPEG(fmt...)		MT_FATAL_PRINT(MT_ID_JPGDEC, fmt)
#define MT_ERR_JPEG(fmt...)		MT_ERR_PRINT(MT_ID_JPGDEC, fmt)
#define MT_WARN_JPEG(fmt...)		MT_WARN_PRINT(MT_ID_JPGDEC, fmt)
#define MT_INFO_JPEG(fmt...)		MT_INFO_PRINT(MT_ID_JPGDEC, fmt)
#define MT_DBG_JPEG(fmt...)		MT_DBG_PRINT(MT_ID_JPGDEC, fmt)

/*Define Debug Level For PNG                 */
#define MT_FATAL_PNG(fmt...)		MT_FATAL_PRINT(MT_ID_PNG, fmt)
#define MT_ERR_PNG(fmt...)		MT_ERR_PRINT(MT_ID_PNG, fmt)
#define MT_WARN_PNG(fmt...)		MT_WARN_PRINT(MT_ID_PNG, fmt)
#define MT_INFO_PNG(fmt...)		MT_INFO_PRINT(MT_ID_PNG, fmt)
#define MT_DBG_PNG(fmt...)		MT_DBG_PRINT(MT_ID_PNG, fmt)

/*Define Debug Level For OTP                 */
#define MT_FATAL_OTP(fmt...)		MT_FATAL_PRINT(MT_ID_OTP, fmt)
#define MT_ERR_OTP(fmt...)		MT_ERR_PRINT(MT_ID_OTP, fmt)
#define MT_WARN_OTP(fmt...)		MT_WARN_PRINT(MT_ID_OTP, fmt)
#define MT_INFO_OTP(fmt...)		MT_INFO_PRINT(MT_ID_OTP, fmt)
#define MT_DBG_OTP(fmt...)		MT_DBG_PRINT(MT_ID_OTP, fmt)

/*Define Debug Level For PQ                 */
#define MT_FATAL_PQ(fmt...)		MT_FATAL_PRINT(MT_ID_PQ, fmt)
#define MT_ERR_PQ(fmt...)		MT_ERR_PRINT(MT_ID_PQ, fmt)
#define MT_WARN_PQ(fmt...)		MT_WARN_PRINT(MT_ID_PQ, fmt)
#define MT_INFO_PQ(fmt...)		MT_INFO_PRINT(MT_ID_PQ, fmt)
#define MT_DBG_PQ(fmt...)		MT_DBG_PRINT(MT_ID_PQ, fmt)

/*Define Debug Level For PVR                 */
#define MT_FATAL_PVR(fmt...)		MT_FATAL_PRINT(MT_ID_PVR, fmt)
#define MT_ERR_PVR(fmt...)		MT_ERR_PRINT(MT_ID_PVR, fmt)
#define MT_WARN_PVR(fmt...)		MT_WARN_PRINT(MT_ID_PVR, fmt)
#define MT_INFO_PVR(fmt...)		MT_INFO_PRINT(MT_ID_PVR, fmt)
#define MT_DBG_PVR(fmt...)		MT_DBG_PRINT(MT_ID_PVR, fmt)

/*Define Debug Level For SCI                 */
#define MT_FATAL_SCI(fmt...)		MT_FATAL_PRINT(MT_ID_SCI, fmt)
#define MT_ERR_SCI(fmt...)		MT_ERR_PRINT(MT_ID_SCI, fmt)
#define MT_WARN_SCI(fmt...)		MT_WARN_PRINT(MT_ID_SCI, fmt)
#define MT_INFO_SCI(fmt...)		MT_INFO_PRINT(MT_ID_SCI, fmt)
#define MT_DBG_SCI(fmt...)		MT_DBG_PRINT(MT_ID_SCI, fmt)

/*Define Debug Level For TDE                 */
#define MT_FATAL_TDE(fmt...)		MT_FATAL_PRINT(MT_ID_TDE, fmt)
#define MT_ERR_TDE(fmt...)		MT_ERR_PRINT(MT_ID_TDE, fmt)
#define MT_WARN_TDE(fmt...)		MT_WARN_PRINT(MT_ID_TDE, fmt)
#define MT_INFO_TDE(fmt...)		MT_INFO_PRINT(MT_ID_TDE, fmt)
#define MT_DBG_TDE(fmt...)		MT_DBG_PRINT(MT_ID_TDE, fmt)

/*Define Debug Level For VDEC                 */
#define MT_FATAL_VDEC(fmt...)		MT_FATAL_PRINT(MT_ID_VDEC, fmt)
#define MT_ERR_VDEC(fmt...)		MT_ERR_PRINT(MT_ID_VDEC, fmt)
#define MT_WARN_VDEC(fmt...)		MT_WARN_PRINT(MT_ID_VDEC, fmt)
#define MT_INFO_VDEC(fmt...)		MT_INFO_PRINT(MT_ID_VDEC, fmt)
#define MT_DBG_VDEC(fmt...)		MT_DBG_PRINT(MT_ID_VDEC, fmt)

/*Define Debug Level For MTFB                 */
#define MT_FATAL_MTFB(fmt...)		MT_FATAL_PRINT(MT_ID_MTFB, fmt)
#define MT_ERR_MTFB(fmt...)		MT_ERR_PRINT(MT_ID_MTFB, fmt)
#define MT_WARN_MTFB(fmt...)		MT_WARN_PRINT(MT_ID_MTFB, fmt)
#define MT_INFO_MTFB(fmt...)		MT_INFO_PRINT(MT_ID_MTFB, fmt)
#define MT_DBG_MTFB(fmt...)		MT_DBG_PRINT(MT_ID_MTFB, fmt)

/*Define Debug Level For VENC                 */
#define MT_FATAL_VENC(fmt...)		MT_FATAL_PRINT(MT_ID_VENC, fmt)
#define MT_ERR_VENC(fmt...)		MT_ERR_PRINT(MT_ID_VENC, fmt)
#define MT_WARN_VENC(fmt...)		MT_WARN_PRINT(MT_ID_VENC, fmt)
#define MT_INFO_VENC(fmt...)		MT_INFO_PRINT(MT_ID_VENC, fmt)
#define MT_DBG_VENC(fmt...)		MT_DBG_PRINT(MT_ID_VENC, fmt)

/*Define Debug Level For VPSS                 */
#define MT_FATAL_VPSS(fmt...)		MT_FATAL_PRINT(MT_ID_VPSS, fmt)
#define MT_ERR_VPSS(fmt...)		MT_ERR_PRINT(MT_ID_VPSS, fmt)
#define MT_WARN_VPSS(fmt...)		MT_WARN_PRINT(MT_ID_VPSS, fmt)
#define MT_INFO_VPSS(fmt...)		MT_INFO_PRINT(MT_ID_VPSS, fmt)
#define MT_DBG_VPSS(fmt...)		MT_DBG_PRINT(MT_ID_VPSS, fmt)

/*Define Debug Level For TIMER                 */
#define MT_FATAL_TIMER(fmt...)		MT_FATAL_PRINT(MT_ID_TIMER, fmt)
#define MT_ERR_TIMER(fmt...)		MT_ERR_PRINT(MT_ID_TIMER, fmt)
#define MT_WARN_TIMER(fmt...)		MT_WARN_PRINT(MT_ID_TIMER, fmt)
#define MT_INFO_TIMER(fmt...)		MT_INFO_PRINT(MT_ID_TIMER, fmt)
#define MT_DBG_TIMER(fmt...)		MT_DBG_PRINT(MT_ID_TIMER, fmt)

/*Define Debug Level For DMA                 */
#define MT_FATAL_DMA(fmt...)		MT_FATAL_PRINT(MT_ID_DMAC, fmt)
#define MT_ERR_DMA(fmt...)		MT_ERR_PRINT(MT_ID_DMAC, fmt)
#define MT_WARN_DMA(fmt...)		MT_WARN_PRINT(MT_ID_DMAC, fmt)
#define MT_INFO_DMA(fmt...)		MT_INFO_PRINT(MT_ID_DMAC, fmt)
#define MT_DBG_DMA(fmt...)		MT_DBG_PRINT(MT_ID_DMAC, fmt)

/*Define Debug Level For AIAO                 */
#define MT_FATAL_AIAO(fmt...)		MT_FATAL_PRINT(MT_ID_AIAO, fmt)
#define MT_ERR_AIAO(fmt...)		MT_ERR_PRINT(MT_ID_AIAO, fmt)
#define MT_WARN_AIAO(fmt...)		MT_WARN_PRINT(MT_ID_AIAO, fmt)
#define MT_INFO_AIAO(fmt...)		MT_INFO_PRINT(MT_ID_AIAO, fmt)
#define MT_DBG_AIAO(fmt...)		MT_DBG_PRINT(MT_ID_AIAO, fmt)


/*Define Debug Level For VO                 */
#define MT_FATAL_VO(fmt...)		MT_FATAL_PRINT(MT_ID_VO, fmt)
#define MT_ERR_VO(fmt...)		MT_ERR_PRINT(MT_ID_VO, fmt)
#define MT_WARN_VO(fmt...)		MT_WARN_PRINT(MT_ID_VO, fmt)
#define MT_INFO_VO(fmt...)		MT_INFO_PRINT(MT_ID_VO, fmt)
#define MT_DBG_VO(fmt...)		MT_DBG_PRINT(MT_ID_VO, fmt)

/*Define Debug Level For WDG                 */
#define MT_FATAL_WDG(fmt...)		MT_FATAL_PRINT(MT_ID_WDG, fmt)
#define MT_ERR_WDG(fmt...)		MT_ERR_PRINT(MT_ID_WDG, fmt)
#define MT_WARN_WDG(fmt...)		MT_WARN_PRINT(MT_ID_WDG, fmt)
#define MT_INFO_WDG(fmt...)		MT_INFO_PRINT(MT_ID_WDG, fmt)
#define MT_DBG_WDG(fmt...)		MT_DBG_PRINT(MT_ID_WDG, fmt)

/*Define Debug Level For I2C                 */
#define MT_FATAL_I2C(fmt...)		MT_FATAL_PRINT(MT_ID_I2C, fmt)
#define MT_ERR_I2C(fmt...)		MT_ERR_PRINT(MT_ID_I2C, fmt)
#define MT_WARN_I2C(fmt...)		MT_WARN_PRINT(MT_ID_I2C, fmt)
#define MT_INFO_I2C(fmt...)		MT_INFO_PRINT(MT_ID_I2C, fmt)
#define MT_DBG_I2C(fmt...)		MT_DBG_PRINT(MT_ID_I2C, fmt)

/*Define Debug Level For SYNC                 */
#define MT_FATAL_SYNC(fmt...)		MT_FATAL_PRINT(MT_ID_SYNC, fmt)
#define MT_ERR_SYNC(fmt...)		MT_ERR_PRINT(MT_ID_SYNC, fmt)
#define MT_WARN_SYNC(fmt...)		MT_WARN_PRINT(MT_ID_SYNC, fmt)
#define MT_INFO_SYNC(fmt...)		MT_INFO_PRINT(MT_ID_SYNC, fmt)
#define MT_DBG_SYNC(fmt...)		MT_DBG_PRINT(MT_ID_SYNC, fmt)


/*Define Debug Level For OSD                 */
#define MT_FATAL_OSD(fmt...)		MT_FATAL_PRINT(MT_ID_OSD, fmt)
#define MT_ERR_OSD(fmt...)		MT_ERR_PRINT(MT_ID_OSD, fmt)
#define MT_WARN_OSD(fmt...)		MT_WARN_PRINT(MT_ID_OSD, fmt)
#define MT_INFO_OSD(fmt...)		MT_INFO_PRINT(MT_ID_OSD, fmt)
#define MT_DBG_OSD(fmt...)		MT_DBG_PRINT(MT_ID_OSD, fmt)

/*Define Debug Level For KEYLED                 */
#define MT_FATAL_KEYLED(fmt...)		MT_FATAL_PRINT(MT_ID_KEYLED, fmt)
#define MT_ERR_KEYLED(fmt...)		MT_ERR_PRINT(MT_ID_KEYLED, fmt)
#define MT_WARN_KEYLED(fmt...)		MT_WARN_PRINT(MT_ID_KEYLED, fmt)
#define MT_INFO_KEYLED(fmt...)		MT_INFO_PRINT(MT_ID_KEYLED, fmt)
#define MT_DBG_KEYLED(fmt...)		MT_DBG_PRINT(MT_ID_KEYLED, fmt)


/*Define Debug Level For DISP                 */
#define MT_FATAL_DISP(fmt...)		MT_FATAL_PRINT(MT_ID_DISP, fmt)
#define MT_ERR_DISP(fmt...)		MT_ERR_PRINT(MT_ID_DISP, fmt)
#define MT_WARN_DISP(fmt...)		MT_WARN_PRINT(MT_ID_DISP, fmt)
#define MT_INFO_DISP(fmt...)		MT_INFO_PRINT(MT_ID_DISP, fmt)
#define MT_DBG_DISP(fmt...)		MT_DBG_PRINT(MT_ID_DISP, fmt)


/*Define Debug Level For LOG                 */
#define MT_FATAL_LOG(fmt...)		MT_FATAL_PRINT(MT_ID_LOG, fmt)
#define MT_ERR_LOG(fmt...)		MT_ERR_PRINT(MT_ID_LOG, fmt)
#define MT_WARN_LOG(fmt...)		MT_WARN_PRINT(MT_ID_LOG, fmt)
#define MT_INFO_LOG(fmt...)		MT_INFO_PRINT(MT_ID_LOG, fmt)
#define MT_DBG_LOG(fmt...)		MT_DBG_PRINT(MT_ID_LOG, fmt)

/*Define Debug Level For PM                 */
#define MT_FATAL_PM(fmt...)		MT_FATAL_PRINT(MT_ID_PM, fmt)
#define MT_ERR_PM(fmt...)		MT_ERR_PRINT(MT_ID_PM, fmt)
#define MT_WARN_PM(fmt...)		MT_WARN_PRINT(MT_ID_PM, fmt)
#define MT_INFO_PM(fmt...)		MT_INFO_PRINT(MT_ID_PM, fmt)
#define MT_DBG_PM(fmt...)		MT_DBG_PRINT(MT_ID_PM, fmt)

#define MT_FATAL_AMPSHM(fmt...)    MT_FATAL_PRINT(MT_ID_AMPSHM, fmt)
#define MT_ERR_AMPSHM(fmt...)      MT_ERR_PRINT(MT_ID_AMPSHM, fmt)
#define MT_WARN_AMPSHM(fmt...)     MT_WARN_PRINT(MT_ID_AMPSHM, fmt)
#define MT_INFO_AMPSHM(fmt...)     MT_INFO_PRINT(MT_ID_AMPSHM, fmt)

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __MT_MODULE_DEBUG_H__ */
