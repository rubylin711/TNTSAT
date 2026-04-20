/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_DRV_AI_H__
 #define __MT_DRV_AI_H__

#ifdef __cplusplus
#if __cplusplus
 extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_unf_ai.h"

#define  AI_MAX_TOTAL_NUM          (4)
#define  AI_MAX_HANDLE_ID          ((MT_ID_AI << 16) | AI_MAX_TOTAL_NUM)
#define  AI_MIN_HANDLE_ID          (MT_ID_AI << 16)
#define  AI_CHNID_MASK             0xffff


/*Define Debug Level For MT_ID_AI                     */

#define CHECK_AI_NULL_PTR(p)                                \
    do {                                                    \
            if(MT_NULL == p)                                \
            {                                               \
                MT_ERR_AI("NULL pointer \n");               \
                return MT_ERR_AI_NULL_PTR;                   \
            }                                               \
         } while(0)

#define CHECK_AI_CREATE(state)                              \
    do                                                      \
    {                                                       \
        if (0 > state)                                      \
        {                                                   \
            MT_ERR_AI("AI  device not open!\n");           \
            return MT_ERR_AI_NOT_INIT;                \
        }                                                   \
    } while (0)
#if  0
#define CHECK_AI_PORT(port)                                  \
    do                                                          \
    {                                                           \
        if (MT_UNF_AI_BUTT <= port)                            \
        {                                                       \
            MT_WARN_AI(" Invalid Ai id %d\n", port);           \
            return MT_ERR_AI_INVALID_ID;                       \
        }                                                       \
    } while (0)
#endif

#define CHECK_AI_ID(AiHandle)                                  \
    do                                                          \
    {                                                           \
        if ((AI_MAX_HANDLE_ID <= AiHandle) || (AI_MIN_HANDLE_ID > AiHandle)) \
        {                                                       \
            MT_ERR_AI(" Invalid Ai id 0x%x\n", AiHandle);       \
            return MT_ERR_AI_INVALID_ID;                       \
        }                                                       \
    } while (0)

#define CHECK_AI_CHN_OPEN(AiHandle) \
    do                                                         \
    {                                                          \
        AiHandle &= AI_CHNID_MASK;                            \
        if (NULL == g_pstGlobalAIRS.pstAI_ATTR_S[AiHandle])   \
        {                                                       \
            MT_ERR_AI(" Invalid AI id %d(not open)\n", AiHandle);        \
            return MT_ERR_AI_INVALID_PARA;                       \
        }                                                       \
    } while (0)


#define CHECK_AI_SAMPLERATE(outrate )                   \
    do                                                  \
    {                                                   \
        switch (outrate)                                \
        {                                               \
        case  MT_UNF_SAMPLE_RATE_8K:                    \
        case  MT_UNF_SAMPLE_RATE_11K:                   \
        case  MT_UNF_SAMPLE_RATE_12K:                   \
        case  MT_UNF_SAMPLE_RATE_16K:                   \
        case  MT_UNF_SAMPLE_RATE_22K:                   \
        case  MT_UNF_SAMPLE_RATE_24K:                   \
        case  MT_UNF_SAMPLE_RATE_32K:                   \
        case  MT_UNF_SAMPLE_RATE_44K:                   \
        case  MT_UNF_SAMPLE_RATE_48K:                   \
        case  MT_UNF_SAMPLE_RATE_88K:                   \
        case  MT_UNF_SAMPLE_RATE_96K:                   \
        case  MT_UNF_SAMPLE_RATE_176K:                  \
        case  MT_UNF_SAMPLE_RATE_192K:                  \
            break;                                      \
        default:                                        \
            MT_WARN_AI("invalid sample out rate %d\n", outrate);    \
            return MT_ERR_AI_INVALID_PARA;                        \
        }                                                       \
    } while (0)


#define CHECK_AI_BCLKDIV(BclkDiv)                   \
    do                                                  \
    {                                                   \
        switch (BclkDiv)                                \
        {                                               \
        case  MT_UNF_I2S_BCLK_1_DIV:                    \
        case  MT_UNF_I2S_BCLK_2_DIV:                    \
        case  MT_UNF_I2S_BCLK_3_DIV:                    \
        case  MT_UNF_I2S_BCLK_4_DIV:                    \
        case  MT_UNF_I2S_BCLK_6_DIV:                    \
        case  MT_UNF_I2S_BCLK_8_DIV:                    \
        case  MT_UNF_I2S_BCLK_12_DIV:                   \
        case  MT_UNF_I2S_BCLK_24_DIV:                   \
        case  MT_UNF_I2S_BCLK_32_DIV:                   \
        case  MT_UNF_I2S_BCLK_48_DIV:                   \
        case  MT_UNF_I2S_BCLK_64_DIV:                   \
            break;                                      \
        default:                                        \
            MT_WARN_AI("invalid bclkDiv %d\n", BclkDiv);    \
            return MT_ERR_AI_INVALID_PARA;                        \
        }                                                       \
    } while (0)

#define CHECK_AI_MCLKDIV(MclkSel)                   \
    do                                                  \
    {                                                   \
        switch (MclkSel)                                \
        {                                               \
        case  MT_UNF_I2S_MCLK_128_FS:                    \
        case  MT_UNF_I2S_MCLK_256_FS:                    \
        case  MT_UNF_I2S_MCLK_384_FS:                    \
        case  MT_UNF_I2S_MCLK_512_FS:                    \
        case  MT_UNF_I2S_MCLK_768_FS:                    \
        case  MT_UNF_I2S_MCLK_1024_FS:                    \
            break;                                      \
        default:                                        \
            MT_WARN_AI("invalid mclk sel %d\n", MclkSel);    \
            return MT_ERR_AI_INVALID_PARA;                        \
        }                                                       \
    } while (0)

#define CHECK_AI_CHN(Chn)                   \
    do                                                  \
    {                                                   \
        switch (Chn)                                    \
        {                                               \
        case  MT_UNF_I2S_CHNUM_1:                       \
        case  MT_UNF_I2S_CHNUM_2:                       \
        case  MT_UNF_I2S_CHNUM_8:                       \
            break;                                      \
        default:                                        \
            MT_WARN_AI("invalid chn %d\n", Chn);    \
            return MT_ERR_AI_INVALID_PARA;                        \
        }                                                       \
    } while (0)


#define CHECK_AI_BITDEPTH(BitDepth )                   \
    do                                                  \
    {                                                   \
        switch (BitDepth)                               \
        {                                               \
        case  MT_UNF_I2S_BIT_DEPTH_16:                  \
        case  MT_UNF_I2S_BIT_DEPTH_24:                  \
            break;                                      \
        default:                                        \
            MT_WARN_AI("invalid bitDepth %d\n", BitDepth);    \
            return MT_ERR_AI_INVALID_PARA;                        \
        }                                                       \
    } while (0)


#define CHECK_AI_PCMDELAY(PcmDelayCycle)                   \
    do                                                  \
    {                                                   \
        switch (PcmDelayCycle)                          \
        {                                               \
        case  MT_UNF_I2S_PCM_0_DELAY:                   \
        case  MT_UNF_I2S_PCM_1_DELAY:                   \
        case  MT_UNF_I2S_PCM_8_DELAY:                   \
        case  MT_UNF_I2S_PCM_16_DELAY:                  \
        case  MT_UNF_I2S_PCM_17_DELAY:                  \
        case  MT_UNF_I2S_PCM_24_DELAY:                  \
        case  MT_UNF_I2S_PCM_32_DELAY:                  \
            break;                                      \
        default:                                        \
            MT_WARN_AI("invalid pcmDelayCycle %d\n", PcmDelayCycle);    \
            return MT_ERR_AI_INVALID_PARA;                        \
        }                                                       \
    } while (0)

#define CHECK_AI_HdmiDataFormat(HdmiDataFormat)         \
    do                                                  \
    {                                                   \
        switch (HdmiDataFormat)                               \
        {                                               \
        case  MT_UNF_AI_HDMI_FORMAT_LPCM:               \
        case  MT_UNF_AI_HDMI_FORMAT_LBR:                  \
        case  MT_UNF_AI_HDMI_FORMAT_HBR:                  \
            break;                                      \
        default:                                        \
            MT_ERR_AI("invalid Hdmi DataFormat %d\n", HdmiDataFormat);    \
            return MT_ERR_AI_INVALID_PARA;                        \
        }                                                       \
    } while (0)

 #define MT_AI_LOCK(mutex) (void)pthread_mutex_lock(mutex);
 #define MT_AI_UNLOCK(mutex) (void)pthread_mutex_unlock(mutex);

 typedef struct hiAI_BUF_ATTR_S
 {
     phys_addr_t      u32PhyBaseAddr;
     mt_u32      u32Read;//read size one time:0xf00,one frame size per ch (example:960*4)
     mt_u32      u32Write;
     mt_u32      u32Size;// one frame size (960 * 4)
     /* user space virtual address */
     ulong u32UserVirBaseAddr;
     /* kernel space virtual address */
     ulong u32KernelVirBaseAddr;
     //TO DO
     //MMZ Handle

 } AI_BUF_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

 #endif
