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
#ifdef  MT_SAMPLE_SMC_DEBUG
#define MT_SMC_PRINT   printf
#else
#define MT_SMC_PRINT
#endif

#define SAMPLE_SMC_FUNCTION_ENTER()       MT_SMC_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_SMC_FUNCTION_EXIT()        MT_SMC_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_SMC_FATAL_PRINT(fmt...)    MT_SMC_PRINT(" [FATAL] " fmt)
#define SAMPLE_SMC_ERR_PRINT(fmt...)      MT_SMC_PRINT(" [ERROR] " fmt)
#define SAMPLE_SMC_WARN_PRINT(fmt...)     MT_SMC_PRINT(" [WARN] "  fmt)
#define SAMPLE_SMC_INFO_PRINT(fmt...)     MT_SMC_PRINT(" [INFO] "  fmt)
#define SAMPLE_SMC_DBG_PRINT(fmt...)      MT_SMC_PRINT(" [DEBUG] " fmt)

#define SMC_PORT 0

#define IRDETO_SEND_LEN    5
#define IRDETO_RCV_LEN     16
#define CONAX_SEND_LEN     8
#define CONAX_RCV_LEN      2
/*************************** Structure Definition ****************************/
typedef enum
{
    SMC_INCORRECT_CARD = -2,
    SMC_INCORRECT_DATA = -3,
    SMC_NO_INSERT_CARD = -4,
}return_type_e;
typedef enum
{
    SMC_CARD_CONAX,
    SMC_CARD_IRDETO,
}smart_card_type_list_e;
typedef struct
{
    MT_U8  smcDev;
    MT_S32 freq;
    MT_U32 testTime;
    MT_UNF_SCI_MODE_E clkMode;
    MT_UNF_SCI_PROTOCOL_E protocolType;
    MT_UNF_SCI_LEVEL_E smcVccLevel;
    MT_UNF_SCI_LEVEL_E smcDectLevel;
    smart_card_type_list_e cardTypy;
}mt_sci_info_t;
typedef struct
{
    MT_U8  *pSmcRcvBuf;
    MT_U8  *pSmcSndBuf;
    MT_U32 pSmcRcvLen;
    MT_U32 pSmcSndLen;
}mt_smc_rcv_send_t;
/********************** Global Variable declaration **************************/
static MT_U8 g_smc_dev;
static MT_BOOL g_smc_check_run;

static const MT_U8 t14_irdeto_send[]={ 0x02, 0x02, 0x00, 0x00, 0x00 };
static const MT_U8 t14_irdeto_rcv[]= { 0x06, 0x09, 0x12, 0x06, 0x23, 0x06, 0x24, 0x06, 0x21,
                                 0x06, 0x22, 0x00, 0x00, 0x5a, 0x41, 0x46 };

static const MT_U8 t0_conax_send[]={ 0xdd, 0x26, 0x00, 0x00, 0x03, 0x10, 0x01, 0x40 };
static const MT_U8 t0_conax_rcv[]= { 0x98, 0x11 };
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_SmcMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

static MT_S32 SelectCardType(smart_card_type_list_e CardNo, mt_smc_rcv_send_t *rcvSendData)
{
    switch(CardNo)
    {
        case SMC_CARD_CONAX:
            rcvSendData->pSmcSndBuf = (MT_U8*)t0_conax_send;
            rcvSendData->pSmcSndLen = CONAX_SEND_LEN;
            rcvSendData->pSmcRcvBuf = (MT_U8*)t0_conax_rcv;
            rcvSendData->pSmcRcvLen = CONAX_RCV_LEN;
            break;
        case SMC_CARD_IRDETO:
            rcvSendData->pSmcSndBuf = (MT_U8*)t14_irdeto_send;
            rcvSendData->pSmcSndLen = IRDETO_SEND_LEN;
            rcvSendData->pSmcRcvBuf = (MT_U8*)t14_irdeto_rcv;
            rcvSendData->pSmcRcvLen = IRDETO_RCV_LEN;
            break;
        default:
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}


static MT_VOID *CheckSmc(MT_VOID *args)
{
    MT_S32 Ret;
    MT_UNF_SCI_STATUS_E enStatus = MT_UNF_SCI_STATUS_NOCARD;
    MT_UNF_SCI_STATUS_E curstatus = MT_UNF_SCI_STATUS_NOCARD;
    static MT_U8 t_first = 0;
    SAMPLE_SMC_INFO_PRINT("CheckSmc Run\n");
    while (g_smc_check_run)
    {
        Ret = mt_unf_sci_getcardstatus(g_smc_dev, &enStatus);
        if (MT_SUCCESS != Ret)
        {
            SAMPLE_SMC_ERR_PRINT("%s %d Error s32Ret = %d\n",__FUNCTION__,__LINE__,Ret);
        }
        if(0 == t_first)
        {
            t_first = 1;
            curstatus = enStatus;
            if(MT_UNF_SCI_STATUS_READY == enStatus)
            {
                SAMPLE_SMC_INFO_PRINT("Card%d SmartCard Inserted\n",g_smc_dev);
            }
            else
            {
                SAMPLE_SMC_INFO_PRINT("Card%d SmartCard Removed\n",g_smc_dev);
                mt_unf_sci_deactivecard(g_smc_dev);
            }
        }
        if(enStatus != curstatus)
        {
            curstatus = enStatus;
            if(MT_UNF_SCI_STATUS_READY == enStatus)
            {
                SAMPLE_SMC_INFO_PRINT("Card%d SmartCard Inserted\n",g_smc_dev);
            }
            else
            {
                SAMPLE_SMC_INFO_PRINT("Card%d SmartCard Removed\n",g_smc_dev);
                mt_unf_sci_deactivecard(g_smc_dev);
            }
        }
        MT_USLEEP(2000 * 1000);
    }
    return MT_NULL;
}


static MT_S32 MT_SmcIrdetoATRData(MT_VOID)
{
    MT_U32 i = 0;
    MT_U32 fi = 372;
    MT_U32 di = 1;
    MT_S32 Ret = MT_SUCCESS;
    MT_U8 u8ATRCount = 0;
    MT_U8 ATRBuf[255];
    MT_UNF_SCI_STATUS_E enStatus = MT_UNF_SCI_STATUS_NOCARD;

    Ret = mt_unf_sci_getcardstatus(g_smc_dev, &enStatus);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_SMC_ERR_PRINT("mt_unf_sci_getcardstatus error, Ret = %d\n", Ret);
        return Ret;
    }
    if(MT_UNF_SCI_STATUS_READY != enStatus)
    {
        SAMPLE_SMC_ERR_PRINT("Card%d SmartCard No Inserted\n", g_smc_dev);
        return SMC_NO_INSERT_CARD;
    }

    mt_unf_sci_configtype(g_smc_dev, MT_UNF_SCI_DIRECT_CARD);

    Ret = mt_unf_sci_resetcard(g_smc_dev, 0);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_SMC_ERR_PRINT("mt_unf_sci_resetcard error, Ret = %d\n", Ret);
        return SMC_INCORRECT_CARD;
    }
    else
    {
        SAMPLE_SMC_INFO_PRINT("Reset Card\n");
    }
    MT_USLEEP( 5000);

    Ret = mt_unf_sci_getatr(g_smc_dev, ATRBuf, 100, &u8ATRCount);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_SMC_ERR_PRINT("mt_unf_sci_getatr error, Ret = %d\n", Ret);
        return Ret;
    }

    SAMPLE_SMC_INFO_PRINT("GetATR Count: %d\n", u8ATRCount);
    SAMPLE_SMC_INFO_PRINT("ATRBuf:");
    for(i = 0; i < u8ATRCount; i++)
    {
        MT_SMC_PRINT("%x ",ATRBuf[i]);
    }

    MT_SMC_PRINT("\n");

    fi = 620;
    di = 1;
    SAMPLE_SMC_INFO_PRINT("Fi = %d, Di = %d\n", fi, di);
    mt_unf_sci_setetufactor(g_smc_dev, fi, di);
    if(255 != u8ATRCount)
    {
        Ret = MT_SUCCESS;
    }
    else
    {
        Ret = SMC_INCORRECT_DATA;
    }
    return Ret;

}


static MT_S32 MT_SmcConaxATRData(MT_VOID)
{
    MT_U32 i = 0;
    MT_S32 Ret = MT_SUCCESS;
    MT_U8 u8ATRCount = 0;
    MT_U8 ATRBuf[64];

    MT_UNF_SCI_STATUS_E enStatus = MT_UNF_SCI_STATUS_NOCARD;

    Ret = mt_unf_sci_getcardstatus(g_smc_dev, &enStatus);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_SMC_ERR_PRINT("mt_unf_sci_getcardstatus error, Ret = %d\n", Ret);
        return Ret;
    }
    if(MT_UNF_SCI_STATUS_READY != enStatus)
    {
        SAMPLE_SMC_ERR_PRINT("Card%d SmartCard No Inserted\n", g_smc_dev);
        return SMC_NO_INSERT_CARD;
    }

    mt_unf_sci_configtype(g_smc_dev,MT_UNF_SCI_DIRECT_CARD);

    Ret = mt_unf_sci_resetcard(g_smc_dev, 0);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_SMC_ERR_PRINT("mt_unf_sci_resetcard error, Ret = %d\n", Ret);
        return SMC_INCORRECT_CARD;
    }
    else
    {
        SAMPLE_SMC_INFO_PRINT("Reset Card\n");
    }
    MT_USLEEP( 5000);
    Ret = mt_unf_sci_getatr(g_smc_dev, ATRBuf, 33, &u8ATRCount);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_SMC_ERR_PRINT("mt_unf_sci_getatr error, Ret = %d\n", Ret);
        return Ret;
    }

    SAMPLE_SMC_INFO_PRINT("GetATR Count:%d\n", u8ATRCount);
    SAMPLE_SMC_INFO_PRINT("ATRBuf:");
    for(i = 0; i < u8ATRCount; i++)
    {
        MT_SMC_PRINT("%x ", ATRBuf[i]);
    }
    MT_SMC_PRINT("\n");

    if(255 != u8ATRCount)
    {
        Ret = MT_SUCCESS;
    }
    else
    {
        Ret = SMC_INCORRECT_DATA;
    }

    SAMPLE_SMC_INFO_PRINT("u8ATRCount: %d\n", u8ATRCount);
    return MT_SUCCESS;
}


static MT_S32 MT_SmcCommunication(mt_sci_info_t sciInfo)
{
    MT_U8  u8Result = 0;
    MT_U8  rcv_buf[512] = { 0 };
    MT_S32 Ret = 0;
    MT_U32 i = 0;
    MT_U32 j = 0;
    MT_U32 u32ReadLen = 0;
    MT_U32 sciTestTime = sciInfo.testTime;
    mt_smc_rcv_send_t      rcvSendData = { 0 };
    smart_card_type_list_e manufacturer = sciInfo.cardTypy;
    MT_UNF_SCI_PROTOCOL_E  enProtocolType = sciInfo.protocolType;

    Ret = SelectCardType(manufacturer, &rcvSendData);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_SMC_ERR_PRINT("card type error\n");
        return MT_FAILURE;
    }

    SAMPLE_SMC_INFO_PRINT("we should test %d times\n", sciTestTime);
    for(j = 0; j < sciTestTime; j++)
    {
        SAMPLE_SMC_INFO_PRINT("this is round %d\n", j);
        SAMPLE_SMC_INFO_PRINT("send sequence: ");
        for(i = 0; i < rcvSendData.pSmcSndLen; i++)
        {
            MT_SMC_PRINT("%#x ", rcvSendData.pSmcSndBuf[i]);
        }

        MT_SMC_PRINT("\n");
        if((enProtocolType == MT_UNF_SCI_PROTOCOL_T1) || (enProtocolType == MT_UNF_SCI_PROTOCOL_T14))
        {
            if(SMC_CARD_CONAX == manufacturer || SMC_CARD_IRDETO == manufacturer)
            {
                Ret = mt_unf_sci_transfer(g_smc_dev, (MT_U8 *)rcvSendData.pSmcSndBuf, rcvSendData.pSmcSndLen, rcv_buf, &u32ReadLen);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_SMC_ERR_PRINT("%s->%d, mt_unf_sci_transfer return %d u32ReadLen:%d\n", __func__, __LINE__, Ret, u32ReadLen);
                    return Ret;
                }
                SAMPLE_SMC_INFO_PRINT("expect rcv sequence: \n");
                for(i = 0; i < rcvSendData.pSmcRcvLen; i++)
                {
                    if((i % 16 == 0) && (0 != i))
                    {
                        MT_SMC_PRINT("\n");
                    }
                    MT_SMC_PRINT("0x%02x ", rcvSendData.pSmcRcvBuf[i]);
                }

                MT_SMC_PRINT("\n\n");
                SAMPLE_SMC_INFO_PRINT("actual rcv sequence: \n");
                for(i = 0; i < rcvSendData.pSmcRcvLen; i++)
                {
                    if((i % 16 == 0) && (0 != i))
                    {
                        MT_SMC_PRINT("\n");
                    }
                    MT_SMC_PRINT("0x%02x ", rcv_buf[i]);
                    if((0 == u8Result) && (rcv_buf[i] != rcvSendData.pSmcRcvBuf[i]))
                    {
                        u8Result = 1;
                    }
                }

                if(u8Result)
                {
                    return MT_FAILURE;
                }
                memset(rcv_buf, 0, sizeof(rcv_buf));
                MT_SMC_PRINT("\n\n");
            }
        }
    }

    return MT_SUCCESS;
}


static MT_S32 MT_SmcStart(mt_sci_info_t sciInfo)
{
    MT_S32  Ret = MT_FAILURE;
    MT_U32 u32Freq = sciInfo.freq;
    static MT_U8 first;
    MT_UNF_SCI_PROTOCOL_E enProtocolType = sciInfo.protocolType;
    MT_UNF_SCI_LEVEL_E enSmcVccLevel = sciInfo.smcVccLevel;
    MT_UNF_SCI_LEVEL_E enSmcDectLevel = sciInfo.smcDectLevel;
    pthread_t smcstatus_task;

    g_smc_dev = sciInfo.smcDev;

    Ret = mt_unf_sci_open(g_smc_dev, enProtocolType, u32Freq);
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_SMC_ERR_PRINT("mt_unf_sci_open error ! sci_dev = %d\n", g_smc_dev);
        return Ret;
    }

    Ret = mt_unf_sci_configvccen(g_smc_dev, enSmcVccLevel);
    if (MT_SUCCESS != Ret)
    {
        (MT_VOID)mt_unf_sci_close(g_smc_dev);
        SAMPLE_SMC_ERR_PRINT("mt_unf_sci_configvccen error ! sci_dev = %d enSciLevel = %d\n", g_smc_dev, enSmcVccLevel);
        return Ret;
    }
    Ret = mt_unf_sci_configdetect(g_smc_dev, enSmcDectLevel);
    if (MT_SUCCESS != Ret)
    {
        (MT_VOID)mt_unf_sci_close(g_smc_dev);
        SAMPLE_SMC_ERR_PRINT("mt_unf_sci_configdetect error ! sci_dev = %d enSciLevel = %d\n",g_smc_dev,enSmcDectLevel);
        return Ret;
    }

    if(!first)
    {
        g_smc_check_run = 1;
        Ret = pthread_create(&smcstatus_task, NULL, CheckSmc, NULL);
        if (MT_SUCCESS != Ret)
        {
            (MT_VOID)mt_unf_sci_close(g_smc_dev);
            SAMPLE_SMC_ERR_PRINT("pthread_create error ! Ret = %d\n",Ret);
            g_smc_check_run = 0;
            return Ret;
        }
    }

    first = 1;

    return MT_SUCCESS;

}

static MT_S32 MT_SmcStop(MT_VOID)
{
    return mt_unf_sci_close(g_smc_dev);
}


static MT_VOID MT_SmcCheckReturn(MT_S32 Ret)
{
    MT_SMC_PRINT("\n");
    switch(Ret)
    {
        case SMC_INCORRECT_CARD:
            SAMPLE_SMC_ERR_PRINT("Insert the correct smart card\n");
            break;
        case SMC_INCORRECT_DATA:
            SAMPLE_SMC_ERR_PRINT("The data was not fetched correctly\n");
            break;
        case SMC_NO_INSERT_CARD:
            SAMPLE_SMC_ERR_PRINT("The card is not inserted\n");
            break;
        default:
            SAMPLE_SMC_ERR_PRINT("ATRData acquisition failed\n");
            break;
    }
}

static MT_VOID MT_SmcGetDefaultParam(mt_sci_info_t *sciInfo)
{
    sciInfo->clkMode = MT_UNF_SCI_MODE_OD;
    sciInfo->smcDectLevel = MT_UNF_SCI_LEVEL_LOW;
    sciInfo->smcVccLevel = MT_UNF_SCI_LEVEL_HIGH;
    sciInfo->testTime = 1;
    sciInfo->smcDev = SMC_PORT;
    sciInfo->cardTypy = SMC_CARD_CONAX;
    sciInfo->freq = 3600;
    sciInfo->protocolType = MT_UNF_SCI_PROTOCOL_T1;
}

static MT_VOID MT_SmcPrintMenu(MT_VOID)
{
    MT_SMC_PRINT("commond: \n");
    MT_SMC_PRINT("     c:    test the connection to the Conax card \n");
    MT_SMC_PRINT("     i:    test the connection to the Irdeto card \n");
    MT_SMC_PRINT("     p:    test the Nagra card using the icc test suite \n");
    MT_SMC_PRINT("     h:    help \n");
    MT_SMC_PRINT("     q:    quit \n");
    MT_SMC_PRINT("SMC>> ");
}


/*!
@brief Enter the standby time or wake up standby
@param  MT_VOID
@return::MT_VOID
@*/
static MT_VOID MT_SmcCmdTask(MT_VOID)
{
    MT_S32  Ret = MT_FAILURE;
    MT_CHAR inPut[32] = { 0 };
    mt_sci_info_t sciInfo = { 0 };
    mt_s32 icc_test_type;
    MT_SmcGetDefaultParam(&sciInfo);

    while(1)
    {
        (MT_VOID)MT_SmcPrintMenu();

        fgets((char *)(inPut), (sizeof(inPut) - 1), stdin);
        MT_SMC_PRINT("\n");
        if('q' == inPut[0])
        {
            g_smc_check_run = 0;
            MT_SMC_PRINT("Exit the program!\n");
            break;
        }
        else if('c' == inPut[0])
        {
            sciInfo.cardTypy = SMC_CARD_CONAX;
            sciInfo.freq = 3600;
            sciInfo.protocolType = MT_UNF_SCI_PROTOCOL_T1;

            Ret = MT_SmcStart(sciInfo);
            if(MT_SUCCESS != Ret)
            {
                g_smc_check_run = 0;
                break;
            }

            Ret = MT_SmcConaxATRData();
            if(MT_SUCCESS != Ret)
            {
                (MT_VOID)MT_SmcStop();
                MT_SmcCheckReturn(Ret);
                continue;
            }

            Ret = MT_SmcCommunication(sciInfo);
            if(MT_SUCCESS != Ret)
            {
                g_smc_check_run = 0;
                (MT_VOID)MT_SmcStop();
                SAMPLE_SMC_ERR_PRINT("Communication is not normal\n");
                break;
            }

            (MT_VOID)MT_SmcStop();
        }
        else if('i' == inPut[0])
        {
            sciInfo.cardTypy = SMC_CARD_IRDETO;
            sciInfo.freq = 6000;
            sciInfo.protocolType = MT_UNF_SCI_PROTOCOL_T14;

            Ret = MT_SmcStart(sciInfo);
            if(MT_SUCCESS != Ret)
            {
                g_smc_check_run = 0;
                break;
            }

            Ret = MT_SmcIrdetoATRData();
            if(MT_SUCCESS != Ret)
            {
                (MT_VOID)MT_SmcStop();
                MT_SmcCheckReturn(Ret);
                continue;
            }

            Ret = MT_SmcCommunication(sciInfo);
            if(MT_SUCCESS != Ret)
            {
                g_smc_check_run = 0;
                (MT_VOID)MT_SmcStop();
                SAMPLE_SMC_ERR_PRINT("Communication is not normal\n");
                break;
            }

            (MT_VOID)MT_SmcStop();
        }
        else if('p' == inPut[0])
        {
            SAMPLE_SMC_INFO_PRINT("choice nagra smart card test type(0:ProtocolTest 1:PairedTest):");
            scanf("%d", &icc_test_type);
            getchar();
            if (icc_test_type)
            {
                (MT_VOID)iccSmartcard_T1PairedTest(100, 0, 1, 3600);
            }
            else
            {
                (MT_VOID)iccSmartcard_T1ProtocolTest(100, 0, 1, 3600);
            }
        }
        else if('h' == inPut[0])
        {
            SAMPLE_SMC_INFO_PRINT("Print help info \n");
        }
    }
}

#ifdef MT_SAMPLE_APP
MT_S32 MT_SmcMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32 Ret = MT_SUCCESS;


#ifndef MT_SAMPLE_APP
    Ret = mt_sys_init();
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_SMC_ERR_PRINT("mt_sys_init failed, ret = %d\n", Ret);
        return Ret;
    }
#endif

    Ret = mt_unf_sci_init();
    if(0 != Ret)
    {
        SAMPLE_SMC_ERR_PRINT("mt_unf_sci_init error !\n");
        return Ret;
    }

    (MT_VOID)MT_SmcCmdTask();

    (MT_VOID)mt_unf_sci_deinit();

#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    return Ret;
}

