/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mt_unf_sci.h"
#include "icc_nagra_smart_card_test_suit.h"
/***************************** Macro Definition ******************************/
#define ATR_PRO_TYPE_T0 0 /* Protocol type T=0 */
#define ATR_PRO_TYPE_T1 1 /* Protocol type T=1 */
#define ATR_PRO_TYPE_T2 2 /* Protocol type T=2 */
#define ATR_PRO_TYPE_T3 3 /* Protocol type T=3 */
#define ATR_PRO_TYPE_T14  14  /* Protocol type T=14 */
#define ATR_INF_BYTE_TA 0 /* Interface byte TAi */
#define ATR_INF_BYTE_TB 1 /* Interface byte TBi */
#define ATR_INF_BYTE_TC 2 /* Interface byte TCi */
#define ATR_INF_BYTE_TD 3 /* Interface byte TDi */
/*!Encodes clockrate conversion*/
#define SMART_FI_DEFAULT           1
/*!Encodes bitrate conversion*/
#define SMART_DI_DEFAULT           1

#ifdef  MT_SAMPLE_SMC_DEBUG
#define MT_ICC_SMC_PRINT   printf
#else
#define MT_ICC_SMC_PRINT
#endif

#define SAMPLE_ICC_SMC_FUNCTION_ENTER()       MT_ICC_SMC_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_ICC_SMC_FUNCTION_EXIT()        MT_ICC_SMC_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_ICC_SMC_FATAL_PRINT(fmt...)    MT_ICC_SMC_PRINT(" [FATAL] " fmt)
#define SAMPLE_ICC_SMC_ERR_PRINT(fmt...)      MT_ICC_SMC_PRINT(" [ERROR] " fmt)
#define SAMPLE_ICC_SMC_WARN_PRINT(fmt...)     MT_ICC_SMC_PRINT(" [WARN] "  fmt)
#define SAMPLE_ICC_SMC_INFO_PRINT(fmt...)     MT_ICC_SMC_PRINT(" [INFO] "  fmt)
#define SAMPLE_ICC_SMC_DBG_PRINT(fmt...)      MT_ICC_SMC_PRINT(" [DEBUG] " fmt)

/********************** Global Variable declaration **************************/
static s32 g_FiMap[] =
{
    372,    372,    558,    744,    1116,   1488,   1860,   -1,
    -1, 512,    768,    1024,   1536,   2048,   -1, -1,
};

static s32 g_DiMap[] =
{
    -1, 1,  2,  4,  8,  16, 32, 64,
    12, 20, -1, -1, -1, -1, -1, -1,
};

static u8 g_smctest_sdata[5]={0x00,0xc1,0x01,0xfe,0x3e};
static u8 g_smctest_sdata1[24]={0x00,0x00,0x14,0x00,0xa4,0x04,0x00,0x0e,
    0x31,0x50,0x41,0x59,0x2e,0x53,0x59,0x53,
    0x2e,0x44,0x44,0x46,0x30,0x31,0x00,0xdd};
static u8 g_smctest_sdata2[9]={0x00,0x40,0x05,0x00,0xb2,0x04,0x01,0x00,
    0xf2};
static u8 g_smctest_sdata3[26]={0x00,0x00,0x16,0x00,0xa4,0x04,0x00,0x10,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x00,0xa6};
static u8 g_smctest_sdata4[42]={0x00,0x40,0x26,0x00,0xa4,0x04,0x00,0x20,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x00,0xe6};
static u8 g_smctest_sdata5[74]={0x00,0x00,0x46,0x00,0xa4,0x04,0x00,0x40,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
    0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x00,0xa6};
static u8 g_smctest_sdata6[106]={0x00,0x40,0x66,0x00,0xa4,0x04,0x00,0x60,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
    0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
    0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
    0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
    0x00,0xe6};
static u8 g_smctest_sdata7[138]={0x00,0x00,0x86,0x00,0xa4,0x04,0x00,0x80,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
    0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
    0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
    0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
    0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
    0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
    0x00,0xa6};
static u8 g_smctest_sdata8[14]={0x00,0x00,0x0a,0x00,0xa4,0x04,0x00,0x04,
    0x00,0x01,0x02,0x03,0x00,0xae};
static u8 g_smctest_sdata9[14]={0x00,0x40,0x0a,0x00,0xa4,0x04,0x00,0x04,
    0x00,0x01,0x02,0x03,0x00,0xee};
static u8 g_smctest_sdata10[9]={0x00,0x00,0x05,0x00,0xb2,0x04,0x02,0x00,
    0xb1};
static u8 g_smctest_sdata11[42]={0x00,0x00,0x26,0x00,0xa4,0x04,0x01,0x20,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x00,0xa7};
static u8 g_smctest_sdata12[9]={0x00,0x40,0x05,0x00,0xb2,0x04,0x02,0x00,
    0xf1};

static u8 g_smctest_rdata[5]={0x00,0xe1,0x01,0xfe,0x1e};
static u8 g_smctest_rdata1[11]={0x00,0x00,0x07,0x00,0x70,0x00,0x00,0x00,
    0x90,0x00,0xe7};
static u8 g_smctest_rdata2[11]={0x00,0x00,0x07,0x00,0xb2,0x04,0x01,0x00,
    0x90,0x00,0x20};
static u8 g_smctest_rdata3[11]={0x00,0x40,0x07,0x00,0x70,0x00,0x00,0x00,
    0x90,0x00,0xa7};
static u8 g_smctest_rdata4[28]={0x00,0x40,0x18,0x00,0xa4,0x04,0x00,0x10,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x00,0x90,0x00,0x78};
static u8 g_smctest_rdata5[44]={0x00,0x00,0x28,0x00,0xa4,0x04,0x00,0x20,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x00,0x90,0x00,0x38};
static u8 g_smctest_rdata6[76]={0x00,0x40,0x48,0x00,0xa4,0x04,0x00,0x40,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
    0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x00,0x90,0x00,0x78};
static u8 g_smctest_rdata7[108]={0x00,0x00,0x68,0x00,0xa4,0x04,0x00,0x60,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
    0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
    0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
    0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
    0x00,0x90,0x00,0x38};
static u8 g_smctest_rdata8[140]={0x00,0x40,0x88,0x00,0xa4,0x04,0x00,0x80,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
    0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
    0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
    0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
    0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
    0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
    0x00,0x90,0x00,0x78};
static u8 g_smctest_rdata9[16]={0x00,0x40,0x0c,0x00,0xa4,0x04,0x00,0x04,
    0x00,0x01,0x02,0x03,0x00,0x90,0x00,0x78};
static u8 g_smctest_rdata10[16]={0x00,0x00,0x0c,0x00,0xa4,0x04,0x00,0x04,
    0x00,0x01,0x02,0x03,0x00,0x90,0x00,0x38};
static u8 g_smctest_rdata11[11]={0x00,0x40,0x07,0x00,0xb2,0x04,0x02,0x00,
    0x90,0x00,0x63};
static u8 g_smctest_rdata12[44]={0x00,0x40,0x28,0x00,0xa4,0x04,0x01,0x20,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x00,0x90,0x00,0x79};
static u8 g_smctest_rdata13[11]={0x00,0x00,0x07,0x00,0xb2,0x04,0x02,0x00,
    0x90,0x00,0x23};
static u8 g_smctest_rdata14[11]={0x00,0x40,0x07,0x00,0x72,0x00,0x00,0x00,
    0x90,0x00,0xa5};
static u8 g_smctest_rdata15[11]={0x00,0x40,0x07,0x00,0x71,0x00,0x00,0x00,
    0x90,0x00,0xa6};
static u8 g_smctest_rdata16[11]={0x00,0x00,0x07,0x00,0x77,0x00,0x00,0x00,
    0x90,0x00,0xe0};

static u8 g_case_5010rec[9]={30,30,67,70,81,90,38,45,27};
static u8 g_case_5020rec[10]={30,30,67,70,92,90,38,71,45,27};
static u8 g_case_5030rec[9]={30,30,77,31,81,90,38,45,27};
static u8 g_case_5040rec[10]={30,30,77,31,92,90,38,71,45,27};
static u8 g_case_5050rec[9]={30,30,78,32,81,90,38,45,27};
static u8 g_case_5060rec[10]={30,30,78,32,92,90,38,71,45,27};
static u8 g_case_5070rec[8]={30,30,62,81,90,38,45,27};
static u8 g_case_5080rec[9]={30,30,62,92,90,38,71,45,27};
static u8 g_case_5090rec[8]={30,30,85,81,90,38,45,27};
static u8 g_case_5100rec[9]={30,30,85,92,90,38,71,45,27};
static u8 g_case_5150rec[9]={30,30,85,77,81,90,38,45,27};
static u8 g_case_5160rec[10]={30,30,85,77,92,90,38,71,45,27};
static u8 g_case_5170rec[9]={30,30,85,76,81,90,38,45,27};
static u8 g_case_5180rec[10]={30,30,85,76,92,90,38,71,45,27};
static u8 g_case_5190rec[9]={30,30,86,46,81,90,38,45,27};
static u8 g_case_5200rec[10]={30,30,86,46,92,90,38,71,45,27};
static u8 g_case_5210rec[10]={30,30,59,49,98,90,38,71,38,27};
static u8 g_case_5220rec[10]={30,30,60,49,98,90,38,71,38,27};
static u8 g_case_5230rec[10]={30,30,57,44,103,90,38,71,38,27};
static u8 g_case_5260rec[10]={30,30,57,44,100,90,38,71,38,27};
static u8 g_case_5310rec[10]={30,30,57,44,109,90,38,71,38,27};
static u8 g_case_5370rec[10]={30,30,78,34,81,90,38,71,38,27};
static u8 g_case_5390rec[10]={30,30,78,40,81,90,38,71,38,27};
static u8 g_case_5430rec[10]={30,30,69,109,62,141,38,71,45,27};

static u8 g_case_5010atr[15]={0x3f,0xd4,0x11,0xff,0x91,0x81,0x71,0xa0,
    0x47,0x00,0x50,0x10,0x81,0x07,0x7a};
static u8 g_case_5020atr[15]={0x3f,0xd4,0x11,0xff,0x91,0x81,0x71,0xa0,
    0x47,0x00,0x50,0x20,0x81,0x07,0x4a};
static u8 g_case_5030atr[15]={0x3d,0xd4,0x11,0xff,0x91,0x81,0x71,0xa0,
    0x47,0x00,0x50,0x30,0x81,0x07,0x5a};
static u8 g_case_5040atr[15]={0x43,0xd4,0x77,0x00,0x76,0x7e,0x71,0xfa,
    0x1d,0xff,0xf5,0xfd,0x7e,0x1f,0xab};
static u8 g_case_5050atr[15]={0x3f,0xd4,0xfe,0xff,0x91,0x81,0x71,0xa0,
    0x47,0x00,0x50,0x50,0x81,0x07,0xd5};
static u8 g_case_5190atr[15]={0x3f,0xd4,0x11,0xff,0x91,0x81,0x51,0xa0,
    0x47,0x47,0x00,0x51,0x90,0x81,0x9b};
static u8 g_case_5210atr[16]={0x3f,0xd4,0x11,0xff,0x91,0x81,0x31,0xa0,
    0x47,0x52,0x10,0x41,0x07,0xF8,0x90,0x00};
static u8 g_case_5230atr[14]={0x3f,0xc4,0xff,0x91,0x81,0x71,0xa0,0x47,
    0x00,0x52,0x30,0x41,0x07,0x99};
static u8 g_case_5250atr[14]={0x3f,0x94,0x11,0x91,0x81,0x71,0xa0,0x47,
    0x00,0x52,0x50,0x41,0x07,0x47};
static u8 g_case_5290atr[15]={0x3f,0xd4,0x11,0xff,0x91,0x81,0x71,0xa0,
    0x47,0x00,0x52,0x90,0x41,0x07,0x38};
static u8 g_case_5300atr[15]={0x3f,0xd4,0x11,0xff,0x91,0x81,0x71,0xa0,
    0x47,0x00,0x53,0x00,0x51,0x07,0xb9};
static u8 g_case_5310atr[14]={0x3f,0xd4,0x11,0xff,0x91,0x81,0x61,0x47,
    0x00,0x53,0x10,0x41,0x07,0x09};
static u8 g_case_5330atr[14]={0x3f,0xd4,0x11,0xff,0x91,0x81,0x51,0xa0,
    0x00,0x53,0x30,0x41,0x07,0xfe};
static u8 g_case_5350atr[14]={0x3f,0xd4,0x11,0xff,0x91,0x81,0x31,0xa0,
    0x47,0x53,0x50,0x41,0x07,0xb9};
static u8 g_case_5370atr[15]={0x3f,0xf4,0x11,0x05,0xff,0x91,0x81,0x31,
    0xa0,0x47,0x53,0x70,0x41,0x07,0xbc};
static u8 g_case_5390atr[15]={0x3f,0xd4,0x11,0xff,0xb1,0x81,0x00,0x31,
    0xa0,0x47,0x53,0x90,0x41,0x07,0x59};
static u8 g_case_5400atr[15]={0x3f,0xd4,0x11,0xff,0xb1,0x81,0x00,0x31,
    0xa0,0x47,0x54,0x00,0x41,0x07,0xce};
static u8 g_case_5430atr[14]={0x3f,0xd4,0x11,0xff,0x91,0x81,0x31,0xa0,
    0x47,0x54,0x30,0x81,0x07,0x1e};

static u8 atr_num_ib_table[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

static struct icc_info_struct   icc_info={0};
static unsigned char g_atr_len = 0;

/******************************* API declaration *****************************/

static RET_CODE parse_atr(u8 *ATR, u32 Len, ATRInfo_S *ATRInfo)
{
    u8  *atr = ATR;
    u8  AtrMask = 0x0;
    u8  AtrCheck = 0,Checklen = 0,i = 0,size = 0;
    u32 pos = 1;
    double clock_mhz = (double)icc_info.clk_freq / 1000;

    if (atr[0] != 0x3b && atr[0] != 0x3f)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("[ATR] First byte: 0x%02X.\n", atr[0]);
        return -1;
    }

    if ((Len < 2) || (Len > ICC_ATR_MAX_LEN))
    {
        SAMPLE_ICC_SMC_ERR_PRINT("[ATR] Len = %d is error!\n", Len);
        return -1;
    }

    SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TS:0x%x\n", atr[0]);
    SAMPLE_ICC_SMC_DBG_PRINT("[ATR] T0:0x%x\n", atr[1]);
    size = 2;
    if ((ATR[pos] & 0x0f) > 0)
    {
        ATRInfo->historyExist = 1;
    }

    /*interface byte*/
    AtrMask = (ATR[pos] & 0xf0) >> 3;
    if ((AtrMask >> 1) & 0x01)
    {
        ATRInfo->groupInfo[0][ATR_GROUP_TA].exist = 1;
        ATRInfo->groupInfo[0][ATR_GROUP_TA].value = atr[++pos];
        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TA1: 0x%x\n", ATRInfo->groupInfo[0][ATR_GROUP_TA].value);
        ATRInfo->TA1 = ATRInfo->groupInfo[0][ATR_GROUP_TA].value;
        ATRInfo->FI = (ATRInfo->groupInfo[ATR_GROUP_TA][0].value & 0xf0) >> 4;
        ATRInfo->DI = ATRInfo->groupInfo[ATR_GROUP_TA][0].value & 0x0f;
        ATRInfo->Fi = g_FiMap[ATRInfo->FI];
        ATRInfo->Di = g_DiMap[ATRInfo->DI];
        size++;
    }
    else
    {
        ATRInfo->FI = 1;
        ATRInfo->DI = 1;
        ATRInfo->Fi = 372;
        ATRInfo->Di = 1;
    }

    if ((AtrMask >> 2) & 0x01)
    {
        ATRInfo->groupInfo[0][ATR_GROUP_TB].exist = 1;
        ATRInfo->groupInfo[0][ATR_GROUP_TB].value = atr[++pos];
        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TB1: 0x%x\n", ATRInfo->groupInfo[0][ATR_GROUP_TB].value);
        size++;
    }

    if ((AtrMask >> 3) & 0x01)
    {
        ATRInfo->groupInfo[0][ATR_GROUP_TC].exist = 1;
        ATRInfo->groupInfo[0][ATR_GROUP_TC].value = atr[++pos];
        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TC1: 0x%x\n", ATRInfo->groupInfo[0][ATR_GROUP_TC].value);
        size++;
    }

    if ((AtrMask >> 4) & 0x01)
    {
        ATRInfo->groupInfo[0][ATR_GROUP_TD].exist = 1;
        ATRInfo->groupInfo[0][ATR_GROUP_TD].value = atr[++pos];
        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TD1: 0x%x   ", ATRInfo->groupInfo[0][ATR_GROUP_TD].value);
        size++;
        if ((ATRInfo->groupInfo[0][ATR_GROUP_TD].value & 0x0f) == 1)
        {
            ATRInfo->T1Exist = 1;
            SAMPLE_ICC_SMC_DBG_PRINT("[ATR] This is T1 protocol !!!\n");
        }
        else if ((ATRInfo->groupInfo[0][ATR_GROUP_TD].value& 0x0f) == 0)
        {
            SAMPLE_ICC_SMC_DBG_PRINT("[ATR] This is T0 protocol !!!\n");
        }

        ATRInfo->protocol_type =
          (ATRInfo->groupInfo[0][ATR_GROUP_TD].value & 0x0f);
        AtrMask = (ATRInfo->groupInfo[0][ATR_GROUP_TD].value & 0xf0) >> 3;
    }
    else
    {
        ATRInfo->protocol_type = 0; // T0 protocol
        AtrMask = 0x0;
    }

    if ((AtrMask >> 1) & 0x01)
    {
        u8 TA2;

        ATRInfo->groupInfo[1][ATR_GROUP_TA].exist = 1;
        ATRInfo->groupInfo[1][ATR_GROUP_TA].value = atr[++pos];
        ATRInfo->is_specific_mode = TRUE;

        size++;
        TA2 = ATRInfo->groupInfo[1][ATR_GROUP_TA].value;
        ATRInfo->protocol_type = TA2 & 0x0F;

        /*
         * If bit 5 is set to 0, then the integers Fi and Di
         * defined above by TA1 shall apply
         */
        if (!(TA2 & 0x10))
        {
            if (ATRInfo->groupInfo[0][ATR_GROUP_TA].exist)
            {
                ATRInfo->FI = ATRInfo->TA1 >> 4;
                ATRInfo->DI = ATRInfo->TA1 & 0x0f;
                ATRInfo->Fi = g_FiMap[ATRInfo->FI];
                ATRInfo->Di = g_DiMap[ATRInfo->DI];
            }
            else
            {
                ATRInfo->FI = 1;
                ATRInfo->DI = 1;
                ATRInfo->Fi = 372;
                ATRInfo->Di = 1;
            }
        }
        else
        {
            SAMPLE_ICC_SMC_DBG_PRINT("[ATR] Specific mode: speed 'implicitly defined', "
                            "not sure how to proceed, assuming default values\n");
            ATRInfo->FI = 1;
            ATRInfo->DI = 1;
            ATRInfo->Fi = 372;
            ATRInfo->Di = 1;
        }

        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TA2: 0x%x   "
                        "(Specific mode: T%i, F=%d, D=%d)\n",
                        TA2, ATRInfo->protocol_type, ATRInfo->Fi, ATRInfo->Di);
    }

    /* Calculate the ETU in us */
    double etu;
    etu = ATRInfo->Fi / ATRInfo->Di / clock_mhz;
    ATRInfo->etu = (u32)etu;
    SAMPLE_ICC_SMC_DBG_PRINT("[ATR] Clock = %fMHz, Fi = %d, Di = %d, ETU = %dus\n",
                    clock_mhz, ATRInfo->Fi, ATRInfo->Di, ATRInfo->etu);

    if ((AtrMask >> 2) & 0x01)
    {
        ATRInfo->groupInfo[1][ATR_GROUP_TB].exist = 1;
        ATRInfo->groupInfo[1][ATR_GROUP_TB].value = atr[++pos];
        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TB2: 0x%x\n", ATRInfo->groupInfo[1][ATR_GROUP_TB].value);
        size++;
    }

    if ((AtrMask >> 3) & 0x01)
    {
        ATRInfo->groupInfo[1][ATR_GROUP_TC].exist = 1;
        ATRInfo->groupInfo[1][ATR_GROUP_TC].value = atr[++pos];
        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TC2:0x%x (noly for T0 show IC card max char timeout)\n",
                        ATRInfo->groupInfo[1][ATR_GROUP_TC].value);
        size++;
    }

    if ((AtrMask >> 4) & 0x01)
    {
        ATRInfo->groupInfo[1][ATR_GROUP_TD].exist = 1;
        ATRInfo->groupInfo[1][ATR_GROUP_TD].value = atr[++pos];
        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TD2: 0x%x\n", ATRInfo->groupInfo[1][ATR_GROUP_TD].value);
        AtrMask = (ATRInfo->groupInfo[1][ATR_GROUP_TD].value & 0xf0) >> 3;
        size++;
    }
    else
    {
        AtrMask = 0x0;
    }

    if ((AtrMask >> 1) & 0x01)
    {
        ATRInfo->groupInfo[2][ATR_GROUP_TA].exist = 1;
        ATRInfo->groupInfo[2][ATR_GROUP_TA].value = atr[++pos];
        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TA3: 0x%x(INF length)\n", ATRInfo->groupInfo[2][ATR_GROUP_TA].value);
        size++;
    }

    if ((AtrMask >> 2) & 0x01)
    {
        ATRInfo->groupInfo[2][ATR_GROUP_TB].exist = 1;
        ATRInfo->groupInfo[2][ATR_GROUP_TB].value = atr[++pos];
        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TB3: 0x%x(b5~b8:BWI  b1~b4:CWI)\n", ATRInfo->groupInfo[2][ATR_GROUP_TB].value);
        size++;
    }

    if ((AtrMask >> 3) & 0x01)
    {
        ATRInfo->groupInfo[2][ATR_GROUP_TC].exist = 1;
        ATRInfo->groupInfo[2][ATR_GROUP_TC].value = atr[++pos];
        SAMPLE_ICC_SMC_DBG_PRINT("[ATR] TC3: 0x%x\n", ATRInfo->groupInfo[2][ATR_GROUP_TC].value);
        size++;
    }

    Checklen = (u8)(pos + (atr[1] & 0x0f) + 2);
    size =(u8)((atr[1] & 0x0f) + size);

    /*TCK check*/
    if (ATRInfo->T1Exist == 1)
    {
        AtrCheck = 0;
        for (i = 1; i < Checklen; i++)
        {
            AtrCheck ^= atr[i];
        }

        if (0 != AtrCheck)
        {
            SAMPLE_ICC_SMC_ERR_PRINT("[ATR] Check Tck is error!\n");
            return -1;
        }
    }

    AtrCheck = atr[1] & 0x0f;   //HistoryByte len
    Checklen = 0;
    /*HistoryByte*/
    if (ATRInfo->historyExist)
    {
        if (ATRInfo->T1Exist == 1)
        {
            ++pos;
            MT_ICC_SMC_PRINT("[ATR] HistoryByte:");
            while (pos < size)
            {
                MT_ICC_SMC_PRINT("%#x ",atr[pos]);
                pos++;
                Checklen++;
            }
            MT_ICC_SMC_PRINT("\nTCK:%#x\n",atr[pos]);
        }
        else
        {
            ++pos;
            MT_ICC_SMC_PRINT("[ATR] HistoryByte:");
            while (pos < size+1)
            {
                MT_ICC_SMC_PRINT("%#x ",atr[pos]);
                pos++;
                Checklen++;
            }
            MT_ICC_SMC_PRINT("\n");
        }
    }

    if (AtrCheck != Checklen)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("[ATR] Check Parameter is error!\n");
        return -1;
    }

    /*TC1*/
    if (ATRInfo->T1Exist == 1)
    {
        ATRInfo->GT = 11;//SMC_GT_T1//add by zwu 20180803
    }
    else
    {
        ATRInfo->GT = 12; //SMC_GT_T0//add by zwu 20180803
    }

    if (ATRInfo->groupInfo[0][ATR_GROUP_TC].exist )
    {
        if (ATRInfo->groupInfo[0][ATR_GROUP_TC].value >= 0 &&
            ATRInfo->groupInfo[0][ATR_GROUP_TC].value < 0xff)
        {
            ATRInfo->GT =( ATRInfo->GT+ATRInfo->groupInfo[0][ATR_GROUP_TC].value);//add by zwu 20180803
        }
    }

    /* Calculate the WT in us */
    u8 wi;
    double WT;

    if (ATRInfo->groupInfo[1][ATR_GROUP_TC].exist)
    {
        // TC2
        wi = ATRInfo->groupInfo[1][ATR_GROUP_TC].value & 0xff;
    }
    else
    {
        wi = 10;
    }

    WT = wi * 960 * ATRInfo->Fi / clock_mhz;
    ATRInfo->WT = (u32)WT;
    SAMPLE_ICC_SMC_DBG_PRINT("[ATR] WI = %d, WT = %dus\n", wi, ATRInfo->WT);

    /* Calculate the PPS in us */
    double pps_timeout;

    pps_timeout = 9600 * 372 / clock_mhz;
    ATRInfo->PPSTiemout = (u32)pps_timeout;
    ATRInfo->PPSTiemout += 50000; //timeout offset
    SAMPLE_ICC_SMC_DBG_PRINT("[ATR] PPS timeout = %dus\n", ATRInfo->PPSTiemout);

    /*TA3     IFSC*/
    if (ATRInfo->groupInfo[2][ATR_GROUP_TA].exist )
    {
        if (((ATRInfo->groupInfo[2][ATR_GROUP_TA].value & 0xff) > 0x0f)&&
            ((ATRInfo->groupInfo[2][ATR_GROUP_TA].value & 0xff) < 0xff))
        {
            ATRInfo->IFSC = ATRInfo->groupInfo[2][ATR_GROUP_TA].value & 0xff;
        }
    }

    /* Calculate the CWT and the BWT in us */
    u8 cwi, bwi;
    double bwt;

    if (ATRInfo->groupInfo[2][ATR_GROUP_TB].exist) // TB3
    {
        u8 TB3 = ATRInfo->groupInfo[2][ATR_GROUP_TB].value;
        cwi = TB3 & 0x0F;
        bwi = TB3 >> 4;
    }
    else
    {
        cwi = 13;
        bwi = 4;
    }

    // Set CWT = (11+(2^CWI)) * work etu
    ATRInfo->CWT = (u32)((12 + (1 << cwi)) * ATRInfo->etu);

    // Set BWT = (2^BWI * 960 * 372 / clockspeed in mhz) us + 11 * work etu
    bwt = 11 * ATRInfo->etu + (1 << bwi) * 960 * 372 / clock_mhz;
    ATRInfo->BWT = (u32)bwt;

    SAMPLE_ICC_SMC_DBG_PRINT("[ATR] CWI = %u, BWI = %u, CWT = %uus, BWT = %uus\n",
                    cwi, bwi, ATRInfo->CWT, ATRInfo->BWT);

    return 0;
}

static void _calculate_etu(smc_working_param_t *p_working_param)
{
    u16 Fi = 372, Di = 1;
    u32 MaxClock = 4 * MHZ;
    u32 WWT = 0;

    /* Calculate work waiting time in work etus */
    WWT = 960 * p_working_param->wi;

    /* Next translate our rate conversion factor Interger into F.
    * Used conversion table in IS0 7816-3 for values, coded this way to
    * make it easier to read.
    */
    switch(p_working_param->FI)
    {
        case 0: /* Internal clock, treat differently */
            Fi = 372;
            MaxClock = 4*MHZ;
            break;
        case 1:
            Fi = 372;
            MaxClock = 5*MHZ;
            break;
        case 2:
            Fi = 558;
            MaxClock = 6*MHZ;
            break;
        case 3:
            Fi = 744;
            MaxClock = 8*MHZ;
            break;
        case 4:
            Fi = 1116;
            MaxClock = 12*MHZ;
            break;
        case 5:
            Fi = 1488;
            MaxClock = 16*MHZ;
            break;
        case 6:
            Fi = 1860;
            MaxClock = 20*MHZ;
            break;
        case 9:
            Fi = 512;
            MaxClock = 5*MHZ;
            break;
        case 10:
            Fi = 768;
            MaxClock = 7500000;
            break;
        case 11:
            Fi = 1024;
            MaxClock = 10*MHZ;
            break;
        case 12:
            Fi = 1536;
            MaxClock = 15*MHZ;
            break;
        case 13:
            Fi = 2048;
            MaxClock = 20*MHZ;
            break;
        default:
            Fi = 372;
            MaxClock = 0;
            break;
    }

    /* Now adjust using the Bit rate adjustment factor D.
    * This is also taken from ISO7816-3 and coded this way
    * to make it easier to read.
    */
    switch(p_working_param->DI)
    {
        case 1:
            Di = 1;
            break;
        case 2:
            Di = 2;
            WWT = WWT * 2;
            break;
        case 3:
            Di = 4;
            WWT = WWT * 4;
            break;
        case 4:
            Di = 8;
            WWT = WWT * 8;
            break;
        case 5:
            Di = 16;
            WWT = WWT * 16;
            break;
        case 6:
            Di = 32;
            WWT = WWT * 32;
            break;
        case 8:
            Di = 12;
            WWT = WWT * 12;
            break;
        case 9:
            Di = 20;
            WWT = WWT * 20;
            break;
        case 10:
            Di = 1;
            Fi = Fi / 2;
            WWT = WWT / 2;
            break;
        case 11:
            Di = 1;
            Fi = Fi / 4;
            WWT = WWT / 4;
            break;
        case 12:
            Di = 1;
            Fi = Fi / 8;
            WWT = WWT / 8;
            break;
        case 13:
            Di = 1;
            Fi = Fi / 16;
            WWT = WWT / 16;
            break;
        case 14:
            Di = 1;
            Fi = Fi / 32;
            WWT = WWT / 32;
            break;
        case 15:
            Di = 1;
            Fi = Fi / 64;
            WWT = WWT / 64;
            break;
        default:
            Di = 1;
            break;
    }

    /* Set maximum clock frequency */
    p_working_param->clk_max = MaxClock;

    /* Set subsequent clock frequency */
    p_working_param->Fi = Fi;
    p_working_param->Di = Di;

    /* Set new WWT */
    p_working_param->wwt = WWT;
}

static RET_CODE scard_pro_atr_parse(scard_atr_desc_t *p_atr, scard_pro_cfg_t *p_cfg)
{
    u8 TDi = 0;
    u8 pointer = 0, pn = 0;
    u8 i = 0;
    u8 modeT = 0, FirstT = 0;
    u8 *p_buf = p_atr->p_buf;

    memset(p_cfg, 0, sizeof(scard_pro_cfg_t));
    p_cfg->work_param.wi = 10;

    /* Check size of p_buf */
    if (p_atr->atr_len < 2)
    {
        return ERR_FAILURE;
    }

    /* Store T0 and TS */
    p_cfg->TS = p_buf[0];
    if ((p_cfg->TS != 0x3b) && (p_cfg->TS != 0x3f))
    {
        SAMPLE_ICC_SMC_INFO_PRINT("TS error\n");
        return ERR_FAILURE;
    }

    p_cfg->T0 = TDi = p_buf[1];
    pointer = 1;

    /* Store number of historical bytes */
    p_cfg->hbn = TDi & 0x0F;

    /* TCK is not present by default */
    (p_cfg->TCK).present = FALSE;

    /* Extract interface bytes */
    while (TDi != 0)
    {
        /* Check p_buf is long enought */
        if (pointer + atr_num_ib_table[(0xF0 & TDi) >> 4] >= p_atr->atr_len)
        {
            return ERR_FAILURE;
        }

        /* Check TAi is present */
        if ((TDi | 0xEF) == 0xFF)
        {
            pointer++;
            p_cfg->ib[pn][ATR_INF_BYTE_TA].value = p_buf[pointer];
            p_cfg->ib[pn][ATR_INF_BYTE_TA].present = TRUE;
            if (pn == 0)
            {
                //TA1
                p_cfg->work_param.FI = (p_buf[pointer] & 0xF0) >> 4;
                p_cfg->work_param.DI = p_buf[pointer] & 0x0F;
            }
            else if(pn == 1)
            {
                //TA2
                p_cfg->work_param.SpecificMode = 1;
                p_cfg->work_param.SpecificType = p_buf[pointer] & 0x0F;
                p_cfg->work_param.SpecificTypeChangable = (p_buf[pointer] & 0x80) ? 1 : 0;
            }
            else if (pn > 1 && modeT == 1)
            {
                p_cfg->work_param.IFSC = p_buf[pointer];
            }
        }
        else
        {
            p_cfg->ib[pn][ATR_INF_BYTE_TA].present = FALSE;
        }

        /* Check TBi is present */
        if ((TDi | 0xDF) == 0xFF)
        {
            pointer++;
            p_cfg->ib[pn][ATR_INF_BYTE_TB].value = p_buf[pointer];
            p_cfg->ib[pn][ATR_INF_BYTE_TB].present = TRUE;
            if (pn == 0)
            {
                //TB1
                p_cfg->work_param.IInt = (p_buf[pointer] & 0x60) >> 5;
                p_cfg->work_param.PInt1 = p_buf[pointer] & 0x1f;
            }
            else if (pn == 1)
            {
                //TB2
                p_cfg->work_param.PInt2 = p_buf[pointer];
            }
            else if ((pn > 1) && (modeT == 1))
            {
                p_cfg->work_param.cwi = p_buf[pointer] & 0x0f;
                p_cfg->work_param.bwi = p_buf[pointer] >> 4;
            }
        }
        else
        {
            p_cfg->ib[pn][ATR_INF_BYTE_TB].present = FALSE;
        }

        /* Check TCi is present */
        if ((TDi | 0xBF) == 0xFF)
        {
            pointer++;
            p_cfg->ib[pn][ATR_INF_BYTE_TC].value = p_buf[pointer];
            p_cfg->ib[pn][ATR_INF_BYTE_TC].present = TRUE;
            if (pn == 0)
            {
                //TC1
                p_cfg->work_param.N = p_buf[pointer];
            }
            else if (pn == 1)
            {
                //TC2
                p_cfg->work_param.wi = p_buf[pointer];
            }
            else if((pn > 1) && (modeT == 1))
            {
                /* bit0==1 use CRC, ==0 use LRC */
                p_cfg->work_param.RC = p_buf[pointer] & 0x01;
            }
        }
        else
        {
            p_cfg->ib[pn][ATR_INF_BYTE_TC].present = FALSE;
        }

        /* Read TDi if present */
        if ((TDi | 0x7F) == 0xFF)
        {
            pointer++;
            TDi = p_cfg->ib[pn][ATR_INF_BYTE_TD].value = p_buf[pointer];
            modeT = p_buf[pointer] & 0x0F;
            p_cfg->ib[pn][ATR_INF_BYTE_TD].present = TRUE;
            (p_cfg->TCK).present = ((TDi & 0x0F) != ATR_PRO_TYPE_T0);
            if (pn >= ATR_MAX_PROTOCOLS)
            {
                return ERR_FAILURE;
            }
            pn++;
        }
        else
        {
            if (pn == 0)
            {
                modeT = 0;
            }

            p_cfg->ib[pn][ATR_INF_BYTE_TD].present = FALSE;
            TDi = 0;
        }

        p_cfg->work_param.SupportedProtocolTypes |= (u16)(0x01 << modeT);
        if (pn == 1) //fix bug#100374
        {
            FirstT = modeT;
        }
    }

    /* Store number of protocols */
    p_cfg->pn = (u8)(pn + 1);

    /* Store historical bytes */
    if (pointer + p_cfg->hbn >= p_atr->atr_len)
    {
        return ERR_FAILURE;
    }

    //  memcpy (atr->hb, p_buf + pointer + 1, atr->hbn);
    for (i = 0; i < p_cfg->hbn; i++)
    {
        *(p_cfg->hb + i) = *(p_buf + pointer + 1 + i);
    }

    pointer += (p_cfg->hbn);

    /* Store TCK  */
    if ((p_cfg->TCK).present)
    {
        if ((pointer + 1) >= p_atr->atr_len)
        {
            SAMPLE_ICC_SMC_INFO_PRINT("actually no Tck\n");
        }
        else
        {
            pointer++;
            (p_cfg->TCK).value = p_buf[pointer];
        }
    }

    if (p_cfg->work_param.SpecificMode)
    {
        /* Choose specific mode */
        p_cfg->work_param.WorkingType = p_cfg->work_param.SpecificType;
    }
    else
    {
        /* Choose first offered protocol */
        p_cfg->work_param.WorkingType = FirstT;

        /* Use default FInt and Dint */
        if (p_cfg->ib[0][ATR_INF_BYTE_TA].present == FALSE)
        {
            p_cfg->work_param.FI = SMART_FI_DEFAULT;
            p_cfg->work_param.DI = SMART_DI_DEFAULT;
        }
    }

    /* Calculate work etu */
    _calculate_etu(&p_cfg->work_param);

    /* config convention */
    if (p_atr->p_buf[0] == 0x3f)
    {
        p_cfg->work_param.convention = 1;
    }

    MT_ICC_SMC_PRINT("ATR raw data: ");
    for (i = 0; i < p_atr->atr_len; i++)
    {
        MT_ICC_SMC_PRINT("0x%02x ", p_atr->p_buf[i]);
    }
    MT_ICC_SMC_PRINT("\n");

    SAMPLE_ICC_SMC_INFO_PRINT("ATR:hb[%s],con[%d],Fi[%d],Di[%d],clk[%d],T[%d],PI1[%d],PI2[%d],II[%d],N[%d]\n",
    p_cfg->hb, p_cfg->work_param.convention, p_cfg->work_param.Fi, p_cfg->work_param.Di,
    p_cfg->work_param.clk_max, p_cfg->work_param.WorkingType, p_cfg->work_param.PInt1,
    p_cfg->work_param.PInt2, p_cfg->work_param.IInt ,p_cfg->work_param.N);
    p_cfg->length = (u8)(pointer + 1);

    return SUCCESS;
}

/*
 * This function is a internal implementation of iccSmartcardReset().
 * It must be protected by the mutex of icc_mutex.
 */
static TIccStatus smartcard_reset(TBoolean xColdReset)
{
    RET_CODE ret;
    ATRInfo_S atr_info;
    scard_atr_desc_t atr = { 0 };
    MT_UNF_SCI_STATUS_E enStatus;
    mt_u8 ATRBuf[38] = { 0 };
    mt_u8 u8ATRCount = 0;

    ret = mt_unf_sci_getcardstatus(ICC_TEST_PORT, &enStatus);
    if (enStatus == MT_UNF_SCI_STATUS_NOCARD)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccSmartcardReset ===> icc_info.card_status == DRV_SMART_STATUS_REMOVE\n");
        return ICC_ERROR_CARD_REMOVED;
    }

    if (icc_info.access_mode != ICC_ACCESS_EXCLUSIVE)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccSmartcardReset ===> Current access mode is "
        "%d. Only ICC_ACCESS_EXCLUSIVE mode can be reset.\n", icc_info.access_mode);
        return ICC_ERROR_MODE;
    }

    if (xColdReset == TRUE)
    {
        ret = mt_unf_sci_configtype(ICC_TEST_PORT, MT_UNF_SCI_INVERSE_CARD);   //fix inverse card for nagra
        ret |= mt_unf_sci_resetcard(ICC_TEST_PORT, 0);
        if (SUCCESS != ret)
        {
            SAMPLE_ICC_SMC_ERR_PRINT("+++re reset err\n");
            return ICC_ERROR_MODE;
        }
    }
    else
    {
        ret = mt_unf_sci_configtype(ICC_TEST_PORT,MT_UNF_SCI_INVERSE_CARD);   //fix inverse card for nagra
        ret |= mt_unf_sci_resetcard(ICC_TEST_PORT, 1);
        if (SUCCESS != ret)
        {
            SAMPLE_ICC_SMC_ERR_PRINT("+++re reset err\n");
            return ICC_ERROR_MODE;
        }
    }

    ret = mt_unf_sci_getatr(ICC_TEST_PORT, ATRBuf, 33, &u8ATRCount);
    if(SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccSmartcardReset ===> scard_active atr error %d\n", __LINE__);
        goto CARD_MUTE;
    }

    atr.p_buf = ATRBuf;
    atr.atr_len = u8ATRCount;
    memset(&atr_info, 0, sizeof(ATRInfo_S));
    if (parse_atr(atr.p_buf, atr.atr_len, &atr_info) != 0)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccSmartcardReset ===> parse_atr %d\n", __LINE__);
        goto CARD_MUTE;
    }

    if ((atr_info.Di == -1) || (atr_info.Fi == -1))
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: Fi Di %d,%x,%x\n", __LINE__, atr_info.Fi, atr_info.Di);
        goto CARD_MUTE;
    }

    icc_info.etu = atr_info.etu;
    icc_info.WT = atr_info.WT;
    icc_info.CWT = atr_info.CWT;
    icc_info.BWT = atr_info.BWT;
    icc_info.BGT = 22 * atr_info.etu;
    if (atr_info.etu)
    {
        icc_info.BWT_etu = (atr_info.BWT/atr_info.etu) + 15;
        icc_info.CWT_etu = (atr_info.CWT/atr_info.etu) + 15;
    }
    else
    {
        icc_info.BWT_etu = 0;
        icc_info.CWT_etu = 0;
    }

    SAMPLE_ICC_SMC_INFO_PRINT("BWT_etu = %d CWT_etu = %d\n", icc_info.BWT_etu, icc_info.CWT_etu);
    icc_info.error_value = 0;
    if (atr_info.GT < 14)
    {
        atr_info.GT+=1;
        icc_info.error_value = 1;
    }

    ret = mt_unf_sci_setguardtime(ICC_TEST_PORT, atr_info.GT);
    ret|= mt_unf_sci_setetufactor(ICC_TEST_PORT, atr_info.Fi, atr_info.Di);
    if (SUCCESS != ret) {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccSmartcardReset ===> scard_set_config %d\n", __LINE__);
        goto CARD_MUTE;
    }

    if ((atr.p_buf[1] & 0x10) == 0) //if TA1 is not occur in the ATR.
    {
        //No TA1, Do not need to negotiate PPS!
        SAMPLE_ICC_SMC_DBG_PRINT("[SC] No TA1, Do not need to negotiate PPS!\n");
        goto NOT_NEED_PPS;
    }

    // For the Nagra specific requirement(RQ_M-DAL-ICC-CN-0010)
    // Nagravision smart cards do not support the negotiable mode.
    // They work in specific mode. A given smart card supports one
    // single protocol only (T=0 or T=1)
    // So we must not do PPS negotiable.

    mtos_task_delay_ms(1);
NOT_NEED_PPS:
    if (atr.atr_len >= ICC_ATR_MAX_LEN)
    {
        memcpy(icc_info.atr, atr.p_buf, ICC_ATR_MAX_LEN);
    }
    else
    {
        memcpy(icc_info.atr, atr.p_buf, atr.atr_len);
    }

    icc_info.active = 1;
    icc_info.card_status = DRV_SMART_STATUS_INSERT;

    g_atr_len = atr.atr_len;

    return ICC_NO_ERROR;
CARD_MUTE:
    icc_info.active = 0;

    return ICC_ERROR_CARD_MUTE;
}

#if ICC_T1_EXCHANGE_DEBUG
static void data_debug_dump(const unsigned char *data, unsigned int len)
{
    unsigned int i, j;

    for (i = 0; i < len; i += j) {
        for (j = 0; ((j < 16) && (i + j < len)); j++) {
            MT_ICC_SMC_PRINT("0x%02x ", data[i + j]);
        }
        MT_ICC_SMC_PRINT("\n");
    }
}
#endif

/**
 *  @brief
 *    This function allows the CA to change the smartcard access mode. It may
 *    be called at any time between insertion and extraction notifications.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard session to modify, as given in the
 *             event notification callback.
 *  @param   xMode
 *             New smartcard access mode. ICC_ACCESS_NONE is used to release
 *             the smartcard.
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session id doesn't exist.
 *  @retval   ICC_ERROR_MODE
 *              The requested access mode is not supported.
 *  @retval   ICC_ERROR_CONFLICT
 *              The requested access mode is in conflict with another
 *              application.
*/
static TIccStatus iccModeChange
(
  TIccSessionId       xSessionId,
  TIccAccessMode      xMode
)
{
    if (xSessionId != SESSION_ID)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccModeChange ===> xSessionId != SESSION_ID\n");
        return ICC_ERROR_SESSION_ID;
    }

    mtos_sem_take(&smartcard_sem, 0);
    if (xMode == ICC_ACCESS_NONE)
    {
        icc_info.active = 0;
        icc_info.access_mode = ICC_ACCESS_NONE;
    }
    else if (xMode == ICC_ACCESS_EXCLUSIVE)
    {
        icc_info.active = 1;
        icc_info.access_mode = ICC_ACCESS_EXCLUSIVE;
    }
    else
    {
        mtos_sem_give(&smartcard_sem);

        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccModeChange ===> Unsupport mode %d.\n",xMode);
        return ICC_ERROR_MODE;
    }

    mtos_sem_give(&smartcard_sem);

    return ICC_NO_ERROR;
}

/**
 *  @brief
 *    This function allows the CA to reset the smartcard.
 *
 *    It may be called at any time between insertion and extraction
 *    notifications. This call is only allowed if the application
 *    communicates with the smartcard in exclusive mode.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface session to reset,
 *             as given in the event notification callback.
 *  @param   xColdReset
 *             TRUE if the driver has to initiate a cold reset.
 *             FALSE if the driver has to initiate a warm reset.
 *
 *  @retval   ICC_NO_ERROR
 *              The smartcard has been successfully reset.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session id doesn't exist.
 *  @retval   ICC_ERROR_MODE
 *              The current session is as shared session and thus the
 *              smartcard cannot be reset.
 *  @retval   ICC_ERROR_REMOVED
 *              The smart card is not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *
 *  @remarks
 *    -# This function is synchronous. The ATR record must have been updated
 *       when this function returns.
*/
static TIccStatus iccSmartcardReset
(
  TIccSessionId       xSessionId,
  TBoolean            xColdReset
)
{
    TIccStatus  ret;

    if (xSessionId != SESSION_ID)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccSmartcardReset ===> xSessionId != SESSION_ID\n");
        return ICC_ERROR_SESSION_ID;
    }

    mtos_sem_take(&smartcard_sem, 0);
    ret = smartcard_reset(xColdReset);
    mtos_sem_give(&smartcard_sem);

    return ret;
}

static TIccStatus iccT1RawExchange
(
        TIccSessionId     xSessionId,
        TSize             xSendLen,
  const TUnsignedInt8*    pxSendBlock,
        TSize             xReplyMaxLen,
        TSize*            pxReplyLen,
        TUnsignedInt8*    pxReplyBlock
)
{
    TIccStatus status = ICC_NO_ERROR;
    mt_s32 ret = 0;
    mt_u32 waiting_time = 0;
    mt_u32 transfer_data = 0;
    mt_u32 i = 0;
    mt_u32 waittimes = 0;

    if (xSessionId != SESSION_ID)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccT1RawExchange ===> xSessionId != SESSION_ID\n");
        return ICC_ERROR_SESSION_ID;
    }

    if (!xSendLen)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccT1RawExchange ===> "
          "The sending data length is zero.\n");
        return ICC_ERROR;
    }

    if (xReplyMaxLen && (!pxReplyBlock || !pxReplyLen))
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccT1RawExchange ===> "
          "This transfer expert to get %ld bytes replying data. "
          "The receiving buffer = %p and the repling length pointer = %p.\n",
          xReplyMaxLen, pxReplyBlock, pxReplyLen);
        return ICC_ERROR;
    }

    if (icc_info.card_status == DRV_SMART_STATUS_REMOVE)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccT1RawExchange ===> "
          "icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD\n");
        return ICC_ERROR_CARD_REMOVED;
    }

    if (icc_info.active == 0)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccT1RawExchange ===> icc_info.active == 0\n");
        return ICC_ERROR_CARD_MUTE;
    }

    if (!pxSendBlock)
    {
        goto receive;
    }

#if ICC_T1_EXCHANGE_DEBUG
    SAMPLE_ICC_SMC_DBG_PRINT("Debug: CWT = %d BWT = %d iccT1RawExchange ===> Send %d bytes to smartcard:\n",
                    icc_info.CWT, icc_info.BWT, xSendLen);
    data_debug_dump((unsigned char *)pxSendBlock, xSendLen);
#endif

    waittimes = (icc_info.BWT + (xSendLen * icc_info.CWT));
    ret = mt_unf_sci_send(ICC_TEST_PORT, (mt_u8 *)pxSendBlock, xSendLen,
                          &transfer_data, icc_info.CWT * xSendLen);
    if (ret || (transfer_data != xSendLen))
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccT1RawExchange ===> mt_unf_sci_send failed. "
          "Try to send %ld bytes, send successfully %u bytes. "
          "Return value = %d.\n", xSendLen, transfer_data, ret);
        return ICC_ERROR;
    }

receive:
    if (!xReplyMaxLen)
    {
        return ICC_NO_ERROR;
    }

    if (icc_info.error_value == 1)
    {
        mt_unf_sci_setblktimeout(ICC_TEST_PORT, icc_info.BWT_etu + (xSendLen + 23));
    }
    else
    {
        mt_unf_sci_setblktimeout(ICC_TEST_PORT, icc_info.BWT_etu);
    }

    for (i = 0, waiting_time = waittimes;
         i < xReplyMaxLen;
         i++, waiting_time = icc_info.CWT)
     {
        ret = mt_unf_sci_receive(ICC_TEST_PORT, (mt_u8 *)pxReplyBlock + i, 1,
                                 &transfer_data, waiting_time);
        if (ret == MT_FAILURE)
        {
            /* Receive timeout */
            break;
        }

        if (ret)
        {
            SAMPLE_ICC_SMC_ERR_PRINT("Error: iccT1RawExchange ===> "
              "mt_unf_sci_receive failed, return value = %d.\n", ret);
            return ICC_ERROR;
        }
    }

    mt_unf_sci_setblktimeout(ICC_TEST_PORT, 0);
    *pxReplyLen = i;
    if (!*pxReplyLen)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccT1RawExchange ===> "
          "mt_unf_sci_receive can't receive any data.\n");
        if (!pxSendBlock)
        {
            return ICC_ERROR;
        }
        else
        {
            return ICC_ERROR_CARD_MUTE;
        }
    }

    if (*pxReplyLen != xReplyMaxLen)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("Error: iccT1RawExchange ===> "
          "mt_unf_sci_receive timeout, expect to receive %lu bytes "
          "but actually receive %lu bytes.\n", xReplyMaxLen, *pxReplyLen);
        status = ICC_ERROR_TIMEOUT;
    }

    usleep(icc_info.BGT);
#if ICC_T1_EXCHANGE_DEBUG
    SAMPLE_ICC_SMC_DBG_PRINT("Debug: iccT1RawExchange ===> "
      "Receive %d bytes data from smartcard:\n", *pxReplyLen);
    data_debug_dump((unsigned char *)pxReplyBlock, *pxReplyLen);
#endif

    return status;
}

static void T1GenerateLRC(u8 * pBuffer, u32 Count, u8 * pLRC)
{
    u32 i;

    *pLRC = 0;

    for (i = 0; i < Count; i++)
    {
        *pLRC ^= pBuffer[i];
    }
}

static int T1_PairedTest(u8 *param, u32 Count, u8 *endcheck,u8 endlen)
{
    unsigned int i = 0, n = 0, m = 0;
    TSize rlen = 0,receive_len = 0;
    TUnsignedInt8 rdata[512] = { 0 };
    TIccStatus ret = ICC_NO_ERROR;
    TUnsignedInt8 t1_sendbuff[256] = { 0 };
    TUnsignedInt8 lrc = 0;

    ret = iccT1RawExchange(SESSION_ID, 5, g_smctest_sdata, 5, &rlen, rdata);
    if (ICC_NO_ERROR != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d communication fail!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    for (i = 0; i < 5; i++)
    {
        if (rdata[i] != g_smctest_rdata[i])
        {
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d data error i = %d! p_rdata[i] = 0x%02x\n", __FUNCTION__, __LINE__, i, rdata[i]);
            return -1;
        }
    }

    memset(rdata, 0, 512);
    ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 37, &rlen, rdata);
    if (ICC_NO_ERROR != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d communication fail!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    T1GenerateLRC(rdata, 36, &lrc);
    if (lrc != rdata[36])
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d lrc fail!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    memset(t1_sendbuff, 0, 256);
    t1_sendbuff[0] = 0x00;
    t1_sendbuff[1] = 0x40;
    n = rdata[2] - 2;
    t1_sendbuff[2] = n;
    memcpy(&t1_sendbuff[3], &rdata[3], n);
    T1GenerateLRC(t1_sendbuff, n + 3, &lrc);
    t1_sendbuff[n+3] = lrc;
    memset(rdata, 0, 512);
    for (m = 0; m < Count; m++)
    {
        receive_len = param[m];
#if SMC_TEST_DATA_DISPLAY
        MT_ICC_SMC_PRINT("send len = %d data:", n+4);
        for (i = 0; i < (n + 4); i++)
        {
            if ((i % 8) == 0)
            {
                MT_ICC_SMC_PRINT("\n");
            }
            MT_ICC_SMC_PRINT("0x%02x ", t1_sendbuff[i]);
        }
        MT_ICC_SMC_PRINT("\n");
#endif
        ret = iccT1RawExchange(SESSION_ID, n + 4, t1_sendbuff, receive_len, &rlen, rdata);
        if (ICC_NO_ERROR != ret)
        {
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d receive_len = %lu communication fail!\n", __FUNCTION__, __LINE__, receive_len);
            return -1;
        }

#if SMC_TEST_DATA_DISPLAY
        MT_ICC_SMC_PRINT("receive len = %d data:", receive_len);
        for (i = 0; i < receive_len; i++)
        {
            if ((i %8 ) == 0)
            {
                MT_ICC_SMC_PRINT("\n");
            }
            MT_ICC_SMC_PRINT("0x%02x ",rdata[i]);
        }
        MT_ICC_SMC_PRINT("\n");
#endif
        T1GenerateLRC(rdata, receive_len - 1, &lrc);
        if (lrc != rdata[receive_len-1])
        {
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d lrc fail!\n", __FUNCTION__, __LINE__);
            return -1;
        }

        memset(t1_sendbuff, 0, 256);
        t1_sendbuff[0] = 0x00;
        if ((m & 0x01) == 0x01)
        {
            t1_sendbuff[1] = 0x40;
        }
        else
        {
            t1_sendbuff[1] = 0x00;
        }

        n = rdata[2] - 2;
        t1_sendbuff[2] = n;
        memcpy(&t1_sendbuff[3], &rdata[3], n);
        T1GenerateLRC(t1_sendbuff,n+3,&lrc);
        t1_sendbuff[n+3] = lrc;
        memset(rdata, 0, 512);
    }

#if SMC_TEST_DATA_DISPLAY
    MT_ICC_SMC_PRINT("send len = %d data:", n + 4);
    for (i = 0; i < (n + 4); i++)
    {
        if ((i % 8) == 0)
        {
            MT_ICC_SMC_PRINT("\n");
        }
        MT_ICC_SMC_PRINT("0x%02x ", t1_sendbuff[i]);
    }
    MT_ICC_SMC_PRINT("\n");
#endif

    ret = iccT1RawExchange(SESSION_ID, n + 4, t1_sendbuff, 11, &rlen, rdata);
    if (ICC_NO_ERROR != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d communication fail!\n", __FUNCTION__, __LINE__);
        return -1;
    }

#if SMC_TEST_DATA_DISPLAY
    MT_ICC_SMC_PRINT("receive len = %d data:", 11);
    for(i = 0; i < 11; i++)
    {
        if ((i % 8) == 0)
        {
            MT_ICC_SMC_PRINT("\n");
        }
        MT_ICC_SMC_PRINT("0x%02x ", rdata[i]);
    }
    MT_ICC_SMC_PRINT("\n");
#endif

    for (i = 0;i < endlen; i++)
    {
        if (rdata[i] != endcheck[i])
        {
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d data error i = %d! rdata[i] = 0x%02x\n", __FUNCTION__, __LINE__, i, rdata[i]);
            return -1;
        }
    }

    mtos_task_sleep(1000);

    return 0;
}

static int T1_PairedTest_Check(u8 *endcheck,u8 endlen)
{
    unsigned int i = 0;
    TIccStatus ret = ICC_NO_ERROR;
    TSize rlen = 0;
    TUnsignedInt8 rdata[512] = { 0 };

    ret = iccT1RawExchange(SESSION_ID, 5, g_smctest_sdata, 5, &rlen, rdata);
    if (ICC_NO_ERROR != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d communication fail!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    for (i = 0; i < 5; i++)
    {
        if (rdata[i] != g_smctest_rdata[i])
        {
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d data error i = %d! rdata[i] = 0x%02x\n", __FUNCTION__, __LINE__, i, rdata[i]);
            return -1;
        }
    }

    memset(rdata, 0, 512);
    ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, rdata);
    if (ICC_NO_ERROR != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d communication fail!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    for (i = 0; i < endlen; i++)
    {
        if (rdata[i] != endcheck[i])
        {
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d data error i = %d! rdata[i] = 0x%02x\n", __FUNCTION__, __LINE__, i, rdata[i]);
            return -1;
        }
    }

    return 0;
}

//max case count = 46
int iccSmartcard_T1ProtocolTest(unsigned int count, u32 detect, u32 vcc, u32 fre)
{
    unsigned int i = 0, j = 0, caseid = 0, checkid = 0, warmreset = 0, n = 0;
    TSize rlen = 0;
    TUnsignedInt8 *p_rdata = NULL;
    TIccStatus ret = ICC_NO_ERROR;
    scard_atr_desc_t atr_info;
    scard_pro_cfg_t atr_info_cfg;
    TUnsignedInt8 tmp_atr[ICC_ATR_MAX_LEN] = { 0 };

    memset(&icc_info, 0x00, sizeof(struct icc_info_struct));
    p_rdata = mtos_align_malloc(512,8);
    if (NULL == p_rdata)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %dmtos_align_malloc is error p_rdata = NULL !\n", __FUNCTION__, __LINE__);
        return -1;
    }

    memset(&atr_info, 0x00, sizeof(scard_atr_desc_t));
    atr_info.p_buf = tmp_atr;
    iccModeChange(SESSION_ID, ICC_ACCESS_EXCLUSIVE);

    icc_info.clk_freq = fre;//CLOCK_FREQ_KHZ;
    ret = mt_unf_sci_open(ICC_TEST_PORT, MT_UNF_SCI_PROTOCOL_T1, icc_info.clk_freq);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d mt_unf_sci_open Fail!\n", __FUNCTION__, __LINE__);
        goto err0;
    }

    ret = mt_unf_sci_setetufactor(ICC_TEST_PORT, 372, 1);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d mt_unf_sci_setetufactor Fail!\n", __FUNCTION__, __LINE__);
        goto err1;
    }

    ret = mt_unf_sci_setguardtime(ICC_TEST_PORT,0);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d mt_unf_sci_setguardtime Fail!\n", __FUNCTION__, __LINE__);
        goto err1;
    }

    ret = mt_unf_sci_configvccen(ICC_TEST_PORT, vcc);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d mt_unf_sci_configvccen Fail!\n", __FUNCTION__, __LINE__);
        goto err1;
    }

    ret = mt_unf_sci_configdetect(ICC_TEST_PORT, detect);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d mt_unf_sci_configdetect Fail!\n", __FUNCTION__, __LINE__);
        goto err1;
    }

    while (1)
    {
        if (i >= count)
        {
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d Smartcard Test End !\n", __FUNCTION__, __LINE__);
            break;
        }

        g_atr_len = 0;
        mtos_task_sleep(5000);
        if(SUCCESS != iccSmartcardReset(SESSION_ID, 1))
        {
            mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,1) Fail!\n", __FUNCTION__, __LINE__);
            i++;
            mtos_task_sleep(10000);
            continue;
        }

        memset(tmp_atr, 0, ICC_ATR_MAX_LEN);
        MT_ICC_SMC_PRINT("Cold Reset ATR len = %d:\n", g_atr_len);
        for (j = 0;j < g_atr_len; j++)
        {
            MT_ICC_SMC_PRINT("0x%02x ",icc_info.atr[j]);
            atr_info.p_buf[j] = icc_info.atr[j];
        }
        MT_ICC_SMC_PRINT("\n");

        atr_info.atr_len = g_atr_len;
        scard_pro_atr_parse(&atr_info, &atr_info_cfg);
        caseid = ((atr_info_cfg.hb[0] & 0xf0) >> 4)*1000+
            ((atr_info_cfg.hb[0] & 0x0f)*100)+
            ((atr_info_cfg.hb[1] & 0xf0) >> 4)*10 +
            ((atr_info_cfg.hb[1] & 0x0f));

        if((3321 == caseid) || (3361 == caseid) || (3362 == caseid)
            || (3363 == caseid) || (3364 == caseid) || (3366 == caseid)
            || (3422 == caseid) || (3442 == caseid) || (3462 == caseid)
            || (3482 == caseid) || (3522 == caseid) || (3526 == caseid)
            || (3681 == caseid) || (3682 == caseid) || (3683 == caseid)
            || (3684 == caseid) || (3686 == caseid) || (3700 == caseid)
            || (3720 == caseid) || (3740 == caseid) || (3886 == caseid))
        {
            warmreset = 1;
            for(j = 0;j < ICC_ATR_MAX_LEN;j++)
            {
                icc_info.atr[j] = 0;
            }

            mtos_task_sleep(1000);
            if (SUCCESS != iccSmartcardReset(SESSION_ID,0))
            {
                mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                break;
            }

            memset(tmp_atr,0,ICC_ATR_MAX_LEN);
            MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
            for(j = 0; j < g_atr_len; j++)
            {
                MT_ICC_SMC_PRINT("0x%02x ",icc_info.atr[j]);
                atr_info.p_buf[j] = icc_info.atr[j];
            }
            MT_ICC_SMC_PRINT("\n");

            atr_info.atr_len = g_atr_len;
            scard_pro_atr_parse(&atr_info,&atr_info_cfg);
            checkid = ((atr_info_cfg.hb[0] & 0xf0) >> 4)*1000+
                ((atr_info_cfg.hb[0] & 0x0f)*100)+
                ((atr_info_cfg.hb[1] & 0xf0) >> 4)*10 +
                ((atr_info_cfg.hb[1] & 0x0f));
            if(checkid != caseid)
            {
                SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) ATR Fail! checkid = %d\n",
                    __FUNCTION__, __LINE__, checkid);
            }
        }

        SAMPLE_ICC_SMC_INFO_PRINT("%s %d cur caseid = %d reset warm = %d error_value = %d\n",
            __FUNCTION__,__LINE__,caseid,warmreset,icc_info.error_value);
        warmreset = 0;
        memset(p_rdata, 0, 512);
        ret = iccT1RawExchange(SESSION_ID, 5, g_smctest_sdata, 5, &rlen , p_rdata);
        if (ICC_NO_ERROR != ret)
        {
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
            continue;
        }

        for (j = 0; j < 5;j++)
        {
            if(p_rdata[j] != g_smctest_rdata[j])
            {
                SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n", __FUNCTION__, __LINE__,caseid,j,p_rdata[j]);
                break;
            }
        }

        memset(p_rdata, 0, 512);
        switch (caseid)
        {
            case 3311:
            case 3321:
                ret = iccT1RawExchange(SESSION_ID,24, g_smctest_sdata1, 11, &rlen,p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata1[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n", __FUNCTION__, __LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }
                break;
            case 3351:
            case 3352:
            case 3353:
            case 3354:
            case 3356:
            case 3361:
            case 3362:
            case 3363:
            case 3364:
            case 3366:
                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0;j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata2[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n", __FUNCTION__, __LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata2, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for(j = 0;j < 11; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata3[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }
                break;
            case 3412:
            case 3416:
            case 3422:
            case 3432:
            case 3442:
            case 3452:
            case 3462:
            case 3472:
            case 3482:
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0; j < 11; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata2[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata2, 28, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0; j < 28; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata4[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n", __FUNCTION__, __LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 26, g_smctest_sdata3, 44, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for(j = 0; j < 44;j++)
                {
                    if (p_rdata[j] != g_smctest_rdata5[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 42, g_smctest_sdata4, 76, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for(j = 0;j < 76; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata6[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 74, g_smctest_sdata5, 108, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0; j < 108;j++)
                {
                    if(p_rdata[j] != g_smctest_rdata7[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 106, g_smctest_sdata6, 140, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0;j < 140;j++)
                {
                    if(p_rdata[j] != g_smctest_rdata8[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 138, g_smctest_sdata7, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0; j < 11;j++)
                {
                    if (p_rdata[j] != g_smctest_rdata1[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                break;
            case 3512:
            case 3516:
            case 3522:
            case 3526:
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for(j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata2[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata2, 16, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0; j < 16; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata9[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                for(n = 0; n < 3; n++)
                {
                    memset(p_rdata, 0, 512);
                    ret = iccT1RawExchange(SESSION_ID, 14, g_smctest_sdata8, 16, &rlen, p_rdata);
                    if (ICC_NO_ERROR != ret)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                        break;
                    }

                    for (j = 0; j < 16; j++)
                    {
                        if (p_rdata[j] != g_smctest_rdata10[j])
                        {
                            SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                                __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                            break;
                        }
                    }

                    memset(p_rdata, 0, 512);
                    ret = iccT1RawExchange(SESSION_ID, 14, g_smctest_sdata9, 16, &rlen, p_rdata);
                    if (ICC_NO_ERROR != ret)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                        break;
                    }

                    for (j = 0; j < 16; j++)
                    {
                        if(p_rdata[j] != g_smctest_rdata9[j])
                        {
                            SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                                __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                            break;
                        }
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 14, g_smctest_sdata8, 16, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0;j < 16;j++)
                {
                    if (p_rdata[j] != g_smctest_rdata10[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata,0,512);
                ret = iccT1RawExchange(SESSION_ID, 14, g_smctest_sdata9, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0;j < 11;j++)
                {
                    if(p_rdata[j] != g_smctest_rdata3[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }
                break;
            case 3671:
            case 3681:
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0;j < 11; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata2[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata,0,512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata2, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0;j<11;j++)
                {
                    if(p_rdata[j] != g_smctest_rdata11[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata10, 44, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0; j < 44; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata5[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 42, g_smctest_sdata4, 44, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for(j = 0; j < 44; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata12[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 42, g_smctest_sdata11, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0; j < 11;j++)
                {
                    if (p_rdata[j] != g_smctest_rdata1[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }
                break;
            case 3672:
            case 3673:
            case 3674:
            case 3676:
            case 3682:
            case 3683:
            case 3684:
            case 3686:
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for(j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata2[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata2, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for(j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata11[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata10, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for(j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata13[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j , p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata12, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for(j = 0; j < 11;j++)
                {
                    if (p_rdata[j] != g_smctest_rdata11[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata10, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for(j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata1[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }
                break;
            case 3690:
            case 3700:
            case 3710:
            case 3720:
            case 3730:
            case 3740:
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for(j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata1[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }
                break;
            case 3871:
            case 3886:
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for(j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata2[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata2, 140, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for(j = 0; j < 140;j ++)
                {
                    if (p_rdata[j] != g_smctest_rdata8[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 138, g_smctest_sdata7, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for (j = 0; j <11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata1[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }
                break;
            case 7000:
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11 , &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for (j = 0; j < 11; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata2[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata2, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for (j = 0; j < 11; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata14[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                if(SUCCESS != iccSmartcardReset(SESSION_ID, 1))
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,1) Fail!\n", __FUNCTION__, __LINE__);
                    break;
                }

                ret = iccT1RawExchange(SESSION_ID, 5, g_smctest_sdata, 5, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for (j = 0; j < 5; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for (j = 0; j < 11;j++)
                {
                    if (p_rdata[j] != g_smctest_rdata2[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__,__LINE__,caseid,j,p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata2, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for (j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata14[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }
                break;
            case 7010:
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11,&rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__,caseid);
                    break;
                }

                for (j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata2[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata2, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for (j = 0; j < 11; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata15[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                if(SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,1) Fail!\n", __FUNCTION__, __LINE__);
                    break;
                }

                ret = iccT1RawExchange(SESSION_ID, 5, g_smctest_sdata, 5, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for(j = 0;j < 5;j++)
                {
                    if(p_rdata[j] != g_smctest_rdata[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for(j = 0;j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata2[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 9, g_smctest_sdata2, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for(j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata15[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }
                break;
            case 9999:
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for (j = 0; j < 11; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata16[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }
                break;
            default:
                break;
        }

        i++;
        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
        if (caseid == 9999)
        {
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = 9999 Smartcard Test End !\n", __FUNCTION__, __LINE__);
            break;
        }
        mtos_task_sleep(5000);
    }

err1:
    mt_unf_sci_close(ICC_TEST_PORT);
err0:
    mtos_align_free(p_rdata);
    p_rdata = NULL;

    mtos_sem_destroy(&smartcard_sem, 0);

    return 0;
}

int iccSmartcard_T1PairedTest(unsigned int count, u32 detect, u32 vcc, u32 fre)
{
    unsigned int i = 0, j = 0, caseid = 0, checkid = 0;
    TSize rlen = 0;
    TUnsignedInt8 *p_rdata = NULL;
    TIccStatus ret = ICC_NO_ERROR;
    scard_atr_desc_t atr_info;
    scard_pro_cfg_t atr_info_cfg;
    TUnsignedInt8 tmp_atr[ICC_ATR_MAX_LEN] = { 0 };

    memset(&icc_info, 0x00, sizeof(struct icc_info_struct));
    p_rdata = mtos_align_malloc(512, 8);
    if (NULL == p_rdata)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %dmtos_align_malloc is error p_rdata =s NULL !\n", __FUNCTION__, __LINE__);
        return -1;
    }

    atr_info.p_buf = tmp_atr;
    iccModeChange(SESSION_ID, ICC_ACCESS_EXCLUSIVE);

    icc_info.clk_freq = fre;//CLOCK_FREQ_KHZ;
    ret = mt_unf_sci_open(ICC_TEST_PORT, MT_UNF_SCI_PROTOCOL_T1, icc_info.clk_freq);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d mt_unf_sci_open Fail!\n", __FUNCTION__, __LINE__);
        goto err0;
    }

    ret = mt_unf_sci_setetufactor(ICC_TEST_PORT, 372, 1);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d mt_unf_sci_setetufactorS Fail!\n", __FUNCTION__, __LINE__);
        goto err1;
    }

    ret= mt_unf_sci_setguardtime(ICC_TEST_PORT, 0);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("%s %d mt_unf_sci_setguardtime Fail!\n", __FUNCTION__, __LINE__);
        goto err1;
    }

    ret = mt_unf_sci_configvccen(ICC_TEST_PORT, vcc);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("mt_unf_sci_configvccen error ! sci_dev = %d enSciLevel = %d\n", ICC_TEST_PORT, vcc);
        goto err1;
    }

    ret = mt_unf_sci_configdetect(ICC_TEST_PORT, detect);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_ICC_SMC_ERR_PRINT("mt_unf_sci_configdetect error ! sci_dev = %d enSciLevel = %d\n", ICC_TEST_PORT, detect);
        goto err1;
    }

    while (1)
    {
        if (i >= count)
        {
            SAMPLE_ICC_SMC_INFO_PRINT("%s %d Smartcard Test End !\n", __FUNCTION__, __LINE__);
            break;
        }

        g_atr_len = 0;
        if (SUCCESS != iccSmartcardReset(SESSION_ID, 1))
        {
            mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,1) Fail! caseid = %d\n", __FUNCTION__, __LINE__,caseid);
            mtos_task_sleep(5000);
            i++;
            continue;
        }

        memset(tmp_atr, 0, ICC_ATR_MAX_LEN);
        MT_ICC_SMC_PRINT("Cold Reset ATR len = %d:\n", g_atr_len);
        for (j = 0; j < g_atr_len; j++)
        {
            MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
            atr_info.p_buf[j] = icc_info.atr[j];
        }
        MT_ICC_SMC_PRINT("\n");

        atr_info.atr_len = g_atr_len;
        scard_pro_atr_parse(&atr_info, &atr_info_cfg);
        caseid = ((atr_info_cfg.hb[0] & 0xf0) >> 4)*1000+
            ((atr_info_cfg.hb[0] & 0x0f)*100)+
            ((atr_info_cfg.hb[1] & 0xf0) >> 4)*10 +
            ((atr_info_cfg.hb[1] & 0x0f));
        SAMPLE_ICC_SMC_INFO_PRINT("%s %d cur caseid = %d error_value = %d\n", __FUNCTION__, __LINE__, caseid, icc_info.error_value);

        memset(p_rdata, 0, 512);
        switch (caseid)
        {
            case 5010:
                for(j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5010atr[j])
                    {
                        break;
                    }
                }

                if(j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("5010.2 Success\n");
                    break;
                }

                T1_PairedTest(g_case_5010rec, 9, g_smctest_rdata1, 11);
                break;
            case 5020:
                g_case_5020atr[12] = 0x91;
                g_case_5020atr[14] = 0x5a;
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5020atr[j])
                    {
                        break;
                    }
                }

                g_case_5020atr[12] = 0x81;
                g_case_5020atr[14] = 0x4a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    memset(tmp_atr, 0, ICC_ATR_MAX_LEN);
                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for(j = 0;j<g_atr_len;j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    atr_info.atr_len = g_atr_len;
                    scard_pro_atr_parse(&atr_info, &atr_info_cfg);
                    checkid = ((atr_info_cfg.hb[0] & 0xf0) >> 4)*1000+
                        ((atr_info_cfg.hb[0] & 0x0f)*100)+
                        ((atr_info_cfg.hb[1] & 0xf0) >> 4)*10 +
                        ((atr_info_cfg.hb[1] & 0x0f));
                    if (checkid != caseid)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) ATR Fail! checkid = %d\n", __FUNCTION__, __LINE__, checkid);
                    }

                    for (j = 0; j < 15; j++)
                    {
                        if(atr_info.p_buf[j] != g_case_5020atr[j])
                        {
                            break;
                        }
                    }

                    if (j != 15)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                    }

                    break;
                }

                T1_PairedTest(g_case_5020rec, 10, g_smctest_rdata3, 11);
                break;
            case 5030:
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j]!=g_case_5030atr[j])
                    {
                        break;
                    }
                }

                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("5030.2 Success\n");
                    break;
                }

                T1_PairedTest(g_case_5030rec, 9, g_smctest_rdata1, 11);
                break;
            case 5040:
                g_case_5020atr[11] = 0x40;
                g_case_5020atr[12] = 0x91;
                g_case_5020atr[14] = 0x3a;
                for (j = 0;j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5020atr[j])
                    {
                        break;
                    }
                }

                g_case_5020atr[11] = 0x20;
                g_case_5020atr[12] = 0x81;
                g_case_5020atr[14] = 0x4a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    memset(tmp_atr, 0, ICC_ATR_MAX_LEN);
                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    atr_info.atr_len = g_atr_len;
                    for (j = 0;j < 15; j++)
                    {
                        if (atr_info.p_buf[j] != g_case_5040atr[j])
                        {
                            break;
                        }
                    }

                    if (j != 15)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                    }

                    break;
                }

                T1_PairedTest(g_case_5040rec, 10, g_smctest_rdata3, 11);
                break;
            case 5050:
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5050atr[j])
                    {
                        break;
                    }
                }

                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("5050.2 Success\n");
                    break;
                }

                T1_PairedTest(g_case_5050rec, 9, g_smctest_rdata1, 11);
                break;
            case 5060:
                g_case_5020atr[11] = 0x60;
                g_case_5020atr[12] = 0x91;
                g_case_5020atr[14] = 0x1a;
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] !=g_case_5020atr[j])
                    {
                        break;
                    }
                }

                g_case_5020atr[11] = 0x20;
                g_case_5020atr[12] = 0x81;
                g_case_5020atr[14] = 0x4a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    memset(tmp_atr, 0, ICC_ATR_MAX_LEN);
                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for(j = 0;j < g_atr_len;j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    atr_info.atr_len = g_atr_len;
                    g_case_5050atr[11] = 0x60;
                    g_case_5050atr[14] = 0xe5;
                    for (j = 0; j < 15; j++)
                    {
                        if (atr_info.p_buf[j] != g_case_5050atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5050atr[11] = 0x50;
                    g_case_5050atr[14] = 0xd5;
                    if (j != 15)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                    }
                    break;
                }

                T1_PairedTest(g_case_5060rec, 10, g_smctest_rdata3, 11);
                break;
            case 5070:
                g_case_5010atr[11] = 0x70;
                g_case_5010atr[14] = 0xe5;
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5010atr[j])
                    {
                        break;
                    }
                }

                g_case_5010atr[11] = 0x10;
                g_case_5010atr[14] = 0x7a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("5070.2 Success\n");
                    break;
                }
                T1_PairedTest(g_case_5070rec, 8, g_smctest_rdata3, 11);
                break;
            case 5080:
                g_case_5020atr[11] = 0x80;
                g_case_5020atr[12] = 0x91;
                g_case_5020atr[14] = 0xfa;
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5020atr[j])
                    {
                        break;
                    }
                }

                g_case_5020atr[11] = 0x20;
                g_case_5020atr[12] = 0x81;
                g_case_5020atr[14] = 0x4a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    memset(tmp_atr, 0, ICC_ATR_MAX_LEN);
                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    g_case_5020atr[11] = 0x80;
                    g_case_5020atr[14] = 0x15;
                    for (j = 0; j < 15; j++)
                    {
                        if(atr_info.p_buf[j] != g_case_5020atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5020atr[11] = 0x20;
                    g_case_5020atr[14] = 0x4a;
                    if (j != 15)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                    }
                    break;
                }

                T1_PairedTest(g_case_5080rec, 9, g_smctest_rdata1, 11);
                break;
            case 5090:
                T1_PairedTest(g_case_5090rec, 8, g_smctest_rdata3, 11);
                break;
            case 5100:
                g_case_5020atr[10] = 0x51;
                g_case_5020atr[11] = 0x00;
                g_case_5020atr[12] = 0x91;
                g_case_5020atr[14] = 0x7b;
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5020atr[j])
                    {
                        break;
                    }
                }

                g_case_5020atr[10] = 0x50;
                g_case_5020atr[11] = 0x20;
                g_case_5020atr[12] = 0x81;
                g_case_5020atr[14] = 0x4a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    memset(tmp_atr, 0, ICC_ATR_MAX_LEN);
                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    break;
                }

                T1_PairedTest(g_case_5100rec, 9, g_smctest_rdata1, 11);
                break;
            case 5150:
                g_case_5010atr[10] = 0x51;
                g_case_5010atr[11] = 0x50;
                g_case_5010atr[14] = 0x3b;
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5010atr[j])
                    {
                        break;
                    }
                }

                g_case_5010atr[10] = 0x50;
                g_case_5010atr[11] = 0x10;
                g_case_5010atr[14] = 0x7a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("5150.2 Success\n");
                    break;
                }

                T1_PairedTest(g_case_5150rec, 9, g_smctest_rdata1, 11);
                break;
            case 5160:
                g_case_5020atr[10] = 0x51;
                g_case_5020atr[11] = 0x60;
                g_case_5020atr[12] = 0x91;
                g_case_5020atr[14] = 0x1b;
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5020atr[j])
                    {
                        break;
                    }
                }

                g_case_5020atr[10] = 0x50;
                g_case_5020atr[11] = 0x20;
                g_case_5020atr[12] = 0x81;
                g_case_5020atr[14] = 0x4a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if(SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    memset(tmp_atr, 0, ICC_ATR_MAX_LEN);
                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    atr_info.atr_len = g_atr_len;
                    g_case_5020atr[10] = 0x51;
                    g_case_5020atr[11] = 0x60;
                    g_case_5020atr[14] = 0x1b;
                    for (j = 0; j < 15; j++)
                    {
                        if(atr_info.p_buf[j] != g_case_5020atr[j])
                        {
                            break;
                        }
                    }
                    g_case_5020atr[10] = 0x50;
                    g_case_5020atr[11] = 0x20;
                    g_case_5020atr[14] = 0x4a;
                    if (j != 15)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                    }
                    break;
                }

                T1_PairedTest(g_case_5160rec, 10, g_smctest_rdata3, 11);
                break;
            case 5170:
                g_case_5010atr[10] = 0x51;
                g_case_5010atr[11] = 0x70;
                g_case_5010atr[14] = 0x1b;
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5010atr[j])
                    {
                        break;
                    }
                }

                g_case_5010atr[10] = 0x50;
                g_case_5010atr[11] = 0x10;
                g_case_5010atr[14] = 0x7a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("5170.2 Success\n");
                    break;
                }

                T1_PairedTest(g_case_5170rec, 9, g_smctest_rdata1, 11);
                break;
            case 5180:
                g_case_5020atr[10] = 0x51;
                g_case_5020atr[11] = 0x80;
                g_case_5020atr[12] = 0x91;
                g_case_5020atr[14] = 0xfb;
                for (j = 0; j  < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5020atr[j])
                    {
                        break;
                    }
                }

                g_case_5020atr[10] = 0x50;
                g_case_5020atr[11] = 0x20;
                g_case_5020atr[12] = 0x81;
                g_case_5020atr[14] = 0x4a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    memset(tmp_atr, 0, ICC_ATR_MAX_LEN);
                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    atr_info.atr_len = g_atr_len;
                    g_case_5020atr[10] = 0x51;
                    g_case_5020atr[11] = 0x80;
                    g_case_5020atr[14] = 0xeb;
                    for (j = 0; j < 15; j++)
                    {
                        if(atr_info.p_buf[j] != g_case_5020atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5020atr[10] = 0x50;
                    g_case_5020atr[11] = 0x20;
                    g_case_5020atr[14] = 0x4a;
                    if (j != 15)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                    }
                    break;
                }

                T1_PairedTest(g_case_5180rec, 10, g_smctest_rdata3, 11);
                break;
            case 5190:
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5190atr[j])
                    {
                        break;
                    }
                }

                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("5190.2 Success\n");
                    break;
                }

                T1_PairedTest(g_case_5190rec, 9, g_smctest_rdata1, 11);
                break;
            case 5200:
                g_case_5020atr[10] = 0x52;
                g_case_5020atr[11] = 0x00;
                g_case_5020atr[12] = 0x91;
                g_case_5020atr[14] = 0x78;
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5020atr[j])
                    {
                        break;
                    }
                }

                g_case_5020atr[10] = 0x50;
                g_case_5020atr[11] = 0x20;
                g_case_5020atr[12] = 0x81;
                g_case_5020atr[14] = 0x4a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    atr_info.atr_len = g_atr_len;
                    g_case_5020atr[6] = 0x51;
                    g_case_5020atr[10] = 0x52;
                    g_case_5020atr[11] = 0x00;
                    g_case_5020atr[12] = 0x91;
                    g_case_5020atr[14] = 0x58;
                    for (j = 0; j < 15; j++)
                    {
                        if(atr_info.p_buf[j] != g_case_5020atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5020atr[6] = 0x71;
                    g_case_5020atr[10] = 0x50;
                    g_case_5020atr[11] = 0x20;
                    g_case_5020atr[12] = 0x81;
                    g_case_5020atr[14] = 0x4a;
                    if (j != 15)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                    }
                    break;
                }

                T1_PairedTest(g_case_5200rec, 10, g_smctest_rdata3, 11);
                break;
            case 5210:
                for (j = 0; j < 16; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5210atr[j])
                    {
                        break;
                    }
                }

                if (j == 16)
                {
                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (0 == ret)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("5210.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5210rec, 10, g_smctest_rdata3, 11);
                break;
            case 5220:
                g_case_5010atr[10] = 0x52;
                g_case_5010atr[11] = 0x20;
                g_case_5010atr[12] = 0x51;
                g_case_5010atr[14] = 0x98;
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5010atr[j])
                    {
                        break;
                    }
                }

                g_case_5010atr[10] = 0x50;
                g_case_5010atr[11] = 0x10;
                g_case_5010atr[12] = 0x81;
                g_case_5010atr[14] = 0x7a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ",icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    g_case_5210atr[10] = 0x20;
                    g_case_5210atr[13] = 0xc8;
                    for (j = 0; j < 16; j++)
                    {
                        if (atr_info.p_buf[j] != g_case_5210atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5210atr[10] = 0x10;
                    g_case_5210atr[13] = 0xf8;
                    if (j != 16)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                        break;
                    }

                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5220.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5220rec, 10, g_smctest_rdata3, 11);
                break;
            case 5230:
                for (j = 0; j < 14; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5230atr[j])
                    {
                        break;
                    }
                }

                if (j == 14)
                {
                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5230.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5230rec, 10, g_smctest_rdata3, 11);
                break;
            case 5240:
                g_case_5010atr[2] = 0x95;
                g_case_5010atr[10] = 0x52;
                g_case_5010atr[11] = 0x40;
                g_case_5010atr[12] = 0x51;
                g_case_5010atr[14] = 0x7c;
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5010atr[j])
                    {
                        break;
                    }
                }

                g_case_5010atr[2] = 0x11;
                g_case_5010atr[10] = 0x50;
                g_case_5010atr[11] = 0x10;
                g_case_5010atr[12] = 0x81;
                g_case_5010atr[14] = 0x7a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for(j = 0;j<g_atr_len;j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ",icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    g_case_5230atr[10] = 0x40;
                    g_case_5230atr[13] = 0xe9;
                    for (j = 0; j < 14; j++)
                    {
                        if (atr_info.p_buf[j]!=g_case_5230atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5230atr[10] = 0x30;
                    g_case_5230atr[13] = 0x99;
                    if (j != 14)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                        break;
                    }

                    ret = T1_PairedTest_Check(g_smctest_rdata1,11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5240.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5230rec, 10, g_smctest_rdata3, 11);
                break;
            case 5250:
                for (j = 0; j < 14; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5250atr[j])
                    {
                        break;
                    }
                }

                if (j == 14)
                {
                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5250.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5230rec, 10, g_smctest_rdata3, 11);
                break;
            case 5260:
                g_case_5010atr[10] = 0x52;
                g_case_5010atr[11] = 0x60;
                g_case_5010atr[12] = 0x51;
                g_case_5010atr[14] = 0xd8;
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5010atr[j])
                    {
                        break;
                    }
                }

                g_case_5010atr[10] = 0x50;
                g_case_5010atr[11] = 0x10;
                g_case_5010atr[12] = 0x81;
                g_case_5010atr[14] = 0x7a;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    g_case_5250atr[10] = 0x60;
                    g_case_5250atr[13] = 0x77;
                    for (j = 0; j < 14; j++)
                    {
                        if(atr_info.p_buf[j] != g_case_5250atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5250atr[10] = 0x50;
                    g_case_5250atr[13] = 0x47;
                    if (j != 14)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                        break;
                    }

                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5260.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5260rec, 10, g_smctest_rdata3, 11);
                break;
            case 5290:
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5290atr[j])
                    {
                        break;
                    }
                }

                if (j == 15)
                {
                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5290.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5260rec, 10, g_smctest_rdata3, 11);
                break;
            case 5300:
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5300atr[j])
                    {
                        break;
                    }
                }

                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    g_case_5300atr[12] = 0x41;
                    g_case_5300atr[14] = 0xa9;
                    for (j = 0; j < 15; j++)
                    {
                        if(atr_info.p_buf[j] != g_case_5300atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5300atr[12] = 0x51;
                    g_case_5300atr[14] = 0xb9;
                    if (j != 15)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                        break;
                    }

                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5300.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5260rec, 10, g_smctest_rdata3, 11);
                break;
            case 5310:
                for (j = 0; j < 14; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5310atr[j])
                    {
                        break;
                    }
                }

                if (j == 14)
                {
                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5310.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5310rec, 10, g_smctest_rdata3, 11);
                break;
            case 5320:
                g_case_5300atr[11] = 0x20;
                g_case_5300atr[14] = 0x99;
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5300atr[j])
                    {
                        break;
                    }
                }

                g_case_5300atr[11] = 0x00;
                g_case_5300atr[14] = 0xb9;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0;j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    g_case_5310atr[10] = 0x20;
                    g_case_5310atr[13] = 0x39;
                    for (j = 0; j < 14; j++)
                    {
                        if (atr_info.p_buf[j] != g_case_5310atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5310atr[10] = 0x10;
                    g_case_5310atr[13] = 0x09;
                    if (j != 14)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                        break;
                    }

                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5320.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5310rec, 10, g_smctest_rdata3, 11);
                break;
            case 5330:
                for (j = 0; j < 14; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5330atr[j])
                    {
                        break;
                    }
                }

                if (j == 14)
                {
                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5330.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5260rec, 10, g_smctest_rdata3, 11);
                break;
            case 5340:
                g_case_5300atr[11] = 0x40;
                g_case_5300atr[14] = 0xf9;
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5300atr[j])
                    {
                        break;
                    }
                }

                g_case_5300atr[11] = 0x00;
                g_case_5300atr[14] = 0xb9;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    g_case_5330atr[10] = 0x40;
                    g_case_5330atr[13] = 0x8e;
                    for (j = 0; j < 14; j++)
                    {
                        if (atr_info.p_buf[j] != g_case_5330atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5330atr[10] = 0x30;
                    g_case_5330atr[13] = 0xfe;
                    if (j != 14)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                        break;
                    }

                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5340.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5260rec, 10, g_smctest_rdata3, 11);
                break;
            case 5350:
                for (j = 0; j < 14; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5350atr[j])
                    {
                        break;
                    }
                }

                if (j == 14)
                {
                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5350.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5260rec, 10, g_smctest_rdata3, 11);
                break;
            case 5360:
                g_case_5300atr[11] = 0x60;
                g_case_5300atr[14] = 0xd9;
                for (j = 0; j < 15;j++)
                {
                    if(atr_info.p_buf[j] != g_case_5300atr[j])
                    {
                        break;
                    }
                }

                g_case_5300atr[11] = 0x00;
                g_case_5300atr[14] = 0xb9;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    g_case_5350atr[10] = 0x60;
                    g_case_5350atr[13] = 0x89;
                    for (j = 0; j < 14; j++)
                    {
                        if(atr_info.p_buf[j] != g_case_5350atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5350atr[10] = 0x50;
                    g_case_5350atr[13] = 0xb9;
                    if (j != 14)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                        break;
                    }

                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5360.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5260rec, 10, g_smctest_rdata3, 11);
                break;
            case 5370:
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5370atr[j])
                    {
                        break;
                    }
                }

                if (j == 15)
                {
                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5370.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5370rec, 10, g_smctest_rdata3, 11);
                break;
            case 5380:
                g_case_5300atr[11] = 0x80;
                g_case_5300atr[14] = 0x39;
                for (j = 0; j < 15; j++)
                {
                    if(atr_info.p_buf[j] != g_case_5300atr[j])
                    {
                        break;
                    }
                }

                g_case_5300atr[11] = 0x00;
                g_case_5300atr[14] = 0xb9;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for(j = 0;j<g_atr_len;j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    g_case_5370atr[11] = 0x80;
                    g_case_5370atr[14] = 0x4c;
                    for (j = 0; j < 15; j++)
                    {
                        if(atr_info.p_buf[j] != g_case_5370atr[j])
                        {
                            break;
                        }
                    }

                    g_case_5370atr[11] = 0x70;
                    g_case_5370atr[14] = 0xbc;
                    if (j != 15)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                        break;
                    }

                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5380.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5370rec,10, g_smctest_rdata3, 11);
                break;
            case 5390:
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5390atr[j])
                    {
                        break;
                    }
                }

                if (j == 15)
                {
                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5390.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5390rec, 10, g_smctest_rdata3, 11);
                break;
            case 5400:
                g_case_5300atr[10] = 0x54;
                g_case_5300atr[14] = 0xbe;
                for (j = 0; j < 15; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5300atr[j])
                    {
                        break;
                    }
                }

                g_case_5300atr[10] = 0x53;
                g_case_5300atr[14] = 0xb9;
                if (j == 15)
                {
                    SAMPLE_ICC_SMC_INFO_PRINT("%s %d caseid = %d warm reset!\n", __FUNCTION__, __LINE__, caseid);
                    for (j = 0; j < ICC_ATR_MAX_LEN; j++)
                    {
                        icc_info.atr[j] = 0;
                    }

                    mtos_task_sleep(10);
                    if (SUCCESS != iccSmartcardReset(SESSION_ID, 0))
                    {
                        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d iccSmartcardReset(SESSION_ID,0) Fail!\n", __FUNCTION__, __LINE__);
                        break;
                    }

                    MT_ICC_SMC_PRINT("Warm Reset ATR len = %d:\n", g_atr_len);
                    for (j = 0; j < g_atr_len; j++)
                    {
                        MT_ICC_SMC_PRINT("0x%02x ", icc_info.atr[j]);
                        atr_info.p_buf[j] = icc_info.atr[j];
                    }
                    MT_ICC_SMC_PRINT("\n");

                    for (j = 0;j < 15; j++)
                    {
                        if (atr_info.p_buf[j] != g_case_5400atr[j])
                        {
                            break;
                        }
                    }

                    if (j != 15)
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d error!\n", __FUNCTION__, __LINE__, caseid);
                        break;
                    }

                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5400.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5390rec,10,g_smctest_rdata3,11);
                break;
            case 5430:
                for (j = 0; j < 14; j++)
                {
                    if (atr_info.p_buf[j] != g_case_5430atr[j])
                    {
                        break;
                    }
                }

                if (j == 14)
                {
                    ret = T1_PairedTest_Check(g_smctest_rdata1, 11);
                    if (ret == 0)
                    {
                        SAMPLE_ICC_SMC_INFO_PRINT("5430.2 Success\n");
                    }
                    break;
                }

                T1_PairedTest(g_case_5430rec, 10, g_smctest_rdata3, 11);
                break;
            case 9999:
                ret = iccT1RawExchange(SESSION_ID, 5, g_smctest_sdata, 5, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    continue;
                }

                for (j = 0; j < 5; j++)
                {
                    if (p_rdata[j] != g_smctest_rdata[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata [j]);
                        break;
                    }
                }

                memset(p_rdata, 0, 512);
                ret = iccT1RawExchange(SESSION_ID, 24, g_smctest_sdata1, 11, &rlen, p_rdata);
                if (ICC_NO_ERROR != ret)
                {
                    SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d communication fail!\n", __FUNCTION__, __LINE__, caseid);
                    break;
                }

                for (j = 0; j < 11; j++)
                {
                    if(p_rdata[j] != g_smctest_rdata16[j])
                    {
                        SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = %d data error j = %d! p_rdata[j] = 0x%02x\n",
                            __FUNCTION__, __LINE__, caseid, j, p_rdata[j]);
                        break;
                    }
                }
                break;
        }

        i++;
        mt_unf_sci_deactivecard(ICC_TEST_PORT);//scard_deactive(pt_smc0);
        if (caseid == 9999)
        {
            SAMPLE_ICC_SMC_ERR_PRINT("%s %d caseid = 9999 Smartcard Test End !\n", __FUNCTION__, __LINE__);
            break;
        }
        mtos_task_sleep(4000);
    }

err1:
    mt_unf_sci_close(ICC_TEST_PORT);
err0:
    mtos_align_free(p_rdata);
    p_rdata = NULL;

    mtos_sem_destroy(&smartcard_sem, 0);

    return 0;
}

