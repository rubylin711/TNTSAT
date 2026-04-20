/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage-LZ Group                                                          */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2021 Montage-LZ Group Limited. All Rights Reserved.         */
/*****************************************************************************/

/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/08/01
  Modification ID : b30d76210d343216ea52b88e9b450c8fd5c0359f
------------------------------------------------------------------------------*/
//#include <mutex.h>
#include <linux/mutex.h>

#include "sony_tuner_r858.h"
#include "../../sony_demod.h"

#include "../I2C_Sys.h" // "I2C_Sys" is only for SW porting reference.

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
const char *sony_tuner_r8588_version = "1.0.0.0";
sony_r858_t *g_pR858Tuner = NULL;

//static UINT32 gR858_mutex;
static struct mutex gR858_mutex;

extern I2C_TYPE R858_I2C;
extern I2C_LEN_TYPE R858_I2C_Len;
extern UINT8 R858_ADDRESS[R858_MAX_NUM][2]; // = {{0x14, 0x34},{0x14, 0x34}};

UINT8 bR858Initialized = 0;
//R858_Set_Info R858_INFO;


/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_r858_Initialize(sony_tuner_terr_cable_t *pTuner);

static sony_result_t sony_tuner_r858_Tune(sony_tuner_terr_cable_t *pTuner,
                                          uint32_t frequency,
                                          sony_dtv_system_t system,
                                          sony_dtv_bandwidth_t bandwidth);

static sony_result_t sony_tuner_r858_Sleep(sony_tuner_terr_cable_t *pTuner);

static sony_result_t sony_tuner_r858_Shutdown(sony_tuner_terr_cable_t *pTuner);

static sony_result_t sony_tuner_r858_Resume(sony_tuner_terr_cable_t *pTuner);

static sony_result_t sony_tuner_r858_ReadRFLevel(sony_tuner_terr_cable_t *pTuner, int32_t *pRFLevel);

sony_result_t sony_tuner_r858_get_tuner_num(sony_tuner_terr_cable_t *pTuner, R858_ExtTunerNum_Type *pExtTunerNum, R858_IntTunerNum_Type *pIntTunerNum)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_tuner_r858_get_tuner_num");

    if (!pTuner)
    {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    switch (pTuner->i2cAddress)
    {
    case 0xD4:
        *pExtTunerNum = R858_NUM1;
        *pIntTunerNum = R858_TUNER1;
        break;

    case 0xF4:
        *pExtTunerNum = R858_NUM1;
        *pIntTunerNum = R858_TUNER2;
        break;

    case 0x14:
        *pExtTunerNum = R858_NUM1;
        *pIntTunerNum = R858_TUNER1;
        break;

    case 0x34:
        *pExtTunerNum = R858_NUM1;
        *pIntTunerNum = R858_TUNER2;
        break;

    default:
        *pExtTunerNum = R858_NUM1;
        *pIntTunerNum = R858_TUNER1;
        break;
    }

    SONY_TRACE_RETURN(result);
}

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_tuner_r858_Create(sony_tuner_terr_cable_t *pTuner,
                                     uint8_t i2cAddress,
                                     sony_i2c_t *pI2c,
                                     uint32_t configFlags,
                                     int dev_id,
                                     sony_r858_t *pR858Tuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_tuner_r858_Create");

    if ((!pI2c) || (!pR858Tuner) || (!pTuner))
    {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pR858Tuner->state = SONY_R858_STATE_UNKNOWN; /* Chip is not accessed for now. */
    pR858Tuner->pI2c = pI2c;
    pR858Tuner->i2cAddress = i2cAddress;
    pR858Tuner->flags = configFlags;
    pR858Tuner->frequencykHz = 0;
    pR858Tuner->tvSystem1 = R858_STD_SIZE;
    pR858Tuner->tvSystem2 = R858_STD_SIZE;
    pR858Tuner->dev_id = dev_id;
    pR858Tuner->user = NULL;

    /* Create local copy of instance data. */
    pTuner->Initialize = sony_tuner_r858_Initialize;
    pTuner->Tune = sony_tuner_r858_Tune;
    pTuner->Sleep = sony_tuner_r858_Sleep;
    pTuner->Shutdown = sony_tuner_r858_Shutdown;
    pTuner->Resume = sony_tuner_r858_Resume,
    pTuner->ReadRFLevel = sony_tuner_r858_ReadRFLevel;
    pTuner->CalcRFLevelFromAGC = NULL;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->i2cAddress = i2cAddress;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;
    pTuner->user = pR858Tuner;

    R858_I2C.I2cAddr = i2cAddress;
    R858_I2C_Len.I2cAddr = i2cAddress;

    SONY_TRACE_RETURN(result);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_r858_Initialize(sony_tuner_terr_cable_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R858_Set_Info R858_INFO;
    R858_ErrCode rc;

    R858_ExtTunerNum_Type ExtTunerNum;
    R858_IntTunerNum_Type IntTunerNum;

    SONY_TRACE_ENTER("sony_tuner_r858_Initialize");

    if (!pTuner || !pTuner->user)
    {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if (bR858Initialized)
    {
        SONY_TRACE_RETURN(result);
    }

    mutex_lock(&gR858_mutex);
    g_pR858Tuner = (sony_r858_t *)pTuner->user;
    /* Call into underlying driver. */
    //rc = R858_Init(R858_NUM1, R858_TUNER1);

    R858_INFO.R858_Standard = R858_DVB_T2_8M_IF_5M;
    //R858_INFO.R858_Standard_2 = R858_DVB_T2_8M_IF_5M;
    R858_INFO.R858_Standard_2 = R858_DVB_T2_8M_IF_5500;

    result = sony_tuner_r858_get_tuner_num(pTuner, &ExtTunerNum, &IntTunerNum);
    if (result != SONY_RESULT_OK)
    {
        SONY_TRACE_RETURN(result);
    }

    //rc = R858_Init_ALL(R858_NUM2, R858_INFO);
    rc = R858_Init_ALL(R858_NUM1, R858_INFO);
    if (rc != RT_Success)
    {
        result = SONY_RESULT_ERROR_OTHER;
    }

    bR858Initialized = 1;

    g_pR858Tuner->state = SONY_R858_STATE_SLEEP;
    g_pR858Tuner->tvSystem1 = R858_STD_SIZE;
    g_pR858Tuner->tvSystem2 = R858_STD_SIZE;
    g_pR858Tuner->frequencykHz = 0;

    pTuner->state = SONY_TUNER_STATE_ACTIVE;

    mutex_unlock(&gR858_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN(result);
}

static sony_result_t sony_tuner_r858_Tune(sony_tuner_terr_cable_t *pTuner,
                                          uint32_t frequency,
                                          sony_dtv_system_t system,
                                          sony_dtv_bandwidth_t bandwidth)
{
    sony_result_t result = SONY_RESULT_OK;
    R858_Standard_Type aSystem1;
    R858_Standard_Type aSystem2;
    R858_Set_Info R858_INFO;
    R858_ErrCode rc;

    R858_ExtTunerNum_Type ExtTunerNum;
    R858_IntTunerNum_Type IntTunerNum;

    SONY_TRACE_ENTER("sony_tuner_r858_Tune");

    if (!pTuner || !pTuner->user)
    {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    result = sony_tuner_r858_get_tuner_num(pTuner, &ExtTunerNum, &IntTunerNum);
    if (result != SONY_RESULT_OK)
    {
        SONY_TRACE_RETURN(result);
    }

    /* Convert system, bandwidth into dtv system. */
    switch (system)
    {
    case SONY_DTV_SYSTEM_DVBC:
        switch (bandwidth)
        {
        case SONY_DTV_BW_6_MHZ:
            aSystem1 = R858_DVB_C_6M_IF_5M;
            aSystem2 = R858_DVB_C_6M;
            break;
        case SONY_DTV_BW_7_MHZ:
            /* 7MHZ BW setting is the same as 8MHz BW */
        case SONY_DTV_BW_8_MHZ:
            aSystem1 = R858_DVB_C_8M_IF_5M;
            aSystem2 = R858_DVB_C_8M;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT:
        switch (bandwidth)
        {
        case SONY_DTV_BW_5_MHZ:
        case SONY_DTV_BW_6_MHZ:
            aSystem1 = R858_DVB_T_6M_IF_5M;
            aSystem2 = R858_DVB_T_6M_IF_5500;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem1 = R858_DVB_T_7M_IF_5M;
            aSystem2 = R858_DVB_T_7M_IF_5500;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem1 = R858_DVB_T_8M_IF_5M;
            aSystem2 = R858_DVB_T_8M_IF_5500;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT2:
        switch (bandwidth)
        {
        case SONY_DTV_BW_1_7_MHZ:
            aSystem1 = R858_DVB_T2_1_7M_IF_5M;
            aSystem2 = R858_DVB_T2_1_7M;
            break;
        case SONY_DTV_BW_5_MHZ:
        case SONY_DTV_BW_6_MHZ:
            aSystem1 = R858_DVB_T2_6M_IF_5M;
            //aSystem2 = R858_DVB_T2_6M;
            aSystem2 = R858_DVB_T2_6M_IF_5500;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem1 = R858_DVB_T2_7M_IF_5M;
            //aSystem2 = R858_DVB_T2_7M;
            aSystem2 = R858_DVB_T2_7M_IF_5500;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem1 = R858_DVB_T2_8M_IF_5M;
            //aSystem2 = R858_DVB_T2_8M;
            aSystem2 = R858_DVB_T2_8M_IF_5500;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBC2:
        switch (bandwidth)
        {
        case SONY_DTV_BW_6_MHZ:
            aSystem1 = R858_DVB_C_6M_IF_5M;
            aSystem2 = R858_DVB_C_6M;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem1 = R858_DVB_C_8M_IF_5M;
            aSystem2 = R858_DVB_C_8M;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_ISDBT:
        switch (bandwidth)
        {
        case SONY_DTV_BW_6_MHZ:
            aSystem1 = R858_ISDB_T_IF_5M;
            aSystem2 = R858_ISDB_T_IF_5500;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem1 = R858_DVB_T_8M_IF_5M;
            aSystem2 = R858_DVB_T_8M_IF_5500;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem1 = R858_DVB_T_8M_IF_5M;
            aSystem2 = R858_DVB_T_8M_IF_5500;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_ISDBC:
        aSystem1 = R858_DVB_C_6M_IF_5M; /* ISDB-C uses DVB-C 6MHz BW setting */
        aSystem2 = R858_DVB_C_6M; /* ISDB-C uses DVB-C 6MHz BW setting */
        break;

    case SONY_DTV_SYSTEM_J83B:
        switch (bandwidth)
        {
        case SONY_DTV_BW_J83B_5_06_5_36_MSPS:
            aSystem1 = R858_J83B_IF_5M; /* J.83B (5.057, 5.361Msps commonly used in US) uses DVB-C 6MHz BW setting */
            aSystem2 = R858_J83B_IF_5500; /* J.83B (5.057, 5.361Msps commonly used in US) uses DVB-C 6MHz BW setting */
            break;
        case SONY_DTV_BW_J83B_5_60_MSPS:
            aSystem1 = R858_J83B_IF_5M; /* J.83B (5.6Msps used in Japan) uses special setting */
            aSystem2 = R858_J83B_IF_5500; /* J.83B (5.6Msps used in Japan) uses special setting */
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
        }
        break;

    /* Intentional fall-through */
    case SONY_DTV_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    R858_INFO.R858_ClkOutMode = CLK_OUT_OFF;
    //  R858_INFO.R858_IfAgc_Select = R858_IF_AGC1;
    R858_INFO.R858_LT = LT_OFF;

    mutex_lock(&gR858_mutex);
    g_pR858Tuner = (sony_r858_t *)pTuner->user;
    R858_INFO.RF_KHz = g_pR858Tuner->frequencykHz = frequency;
    R858_INFO.R858_Standard = g_pR858Tuner->tvSystem1 = aSystem1;
    R858_INFO.R858_Standard_2 = g_pR858Tuner->tvSystem2 = aSystem2;
    //R858_INFO.R858_Standard = R858_DVB_T2_8M_IF_5M;
    //rc = R858_SetPllData(R858_NUM1, R858_TUNER1, R858_INFO);
    //rc = R858_SetPllData(R858_NUM2, R858_TUNER2, R858_INFO);

    //rc = R858_SetPllData(R858_NUM1, R858_TUNER1, R858_INFO);
    rc = R858_SetPllData(R858_NUM1, IntTunerNum, R858_INFO);
    if (rc != RT_Success)
    {
        mutex_unlock(&gR858_mutex);
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_OTHER);
    }
    g_pR858Tuner->state = SONY_R858_STATE_ACTIVE;
    mutex_unlock(&gR858_mutex);

    /* Allow the tuner time to settle */
    SONY_SLEEP(50);

    /* Assign current values. */
    pTuner->system = system;
    pTuner->frequencyKHz = frequency;
    pTuner->bandwidth = bandwidth;

    SONY_TRACE_RETURN(result);
}

static sony_result_t sony_tuner_r858_Sleep(sony_tuner_terr_cable_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R858_ErrCode rc;

    R858_ExtTunerNum_Type ExtTunerNum;
    R858_IntTunerNum_Type IntTunerNum;

    SONY_TRACE_ENTER("sony_tuner_r858_Sleep");

    if (!pTuner || !pTuner->user)
    {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gR858_mutex);

    if (pTuner->state == SONY_TUNER_STATE_SLEEP || pTuner->state == SONY_TUNER_STATE_SHUTDOWN)
    {
        mutex_unlock(&gR858_mutex);
        SONY_TRACE_RETURN(SONY_RESULT_OK);
    }

    result = sony_tuner_r858_get_tuner_num(pTuner, &ExtTunerNum, &IntTunerNum);
    if (result != SONY_RESULT_OK)
    {
        SONY_TRACE_RETURN(result);
    }

    g_pR858Tuner = (sony_r858_t *)pTuner->user;
    //rc=R858_Standby(R858_NUM1, R858_TUNER1, LT_ON);
    //rc=R858_Standby(R858_NUM2, R858_TUNER2, LT_ON);
    rc = R858_Standby(R858_NUM1, IntTunerNum, LT_ON);
    if (rc != RT_Success)
    {
        result = SONY_RESULT_ERROR_OTHER;
    }
    g_pR858Tuner->frequencykHz = 0;
    g_pR858Tuner->state = SONY_R858_STATE_SLEEP;
    g_pR858Tuner->tvSystem1 = R858_STD_SIZE;
    g_pR858Tuner->tvSystem2 = R858_STD_SIZE;

    pTuner->state = SONY_TUNER_STATE_SLEEP;

    mutex_unlock(&gR858_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN(result);
}

static sony_result_t sony_tuner_r858_Shutdown(sony_tuner_terr_cable_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R858_ErrCode rc;

    R858_ExtTunerNum_Type ExtTunerNum;
    R858_IntTunerNum_Type IntTunerNum;

    SONY_TRACE_ENTER("sony_tuner_r858_Shutdown");

    if (!pTuner || !pTuner->user)
    {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gR858_mutex);
    if (pTuner->state == SONY_TUNER_STATE_SLEEP || pTuner->state == SONY_TUNER_STATE_SHUTDOWN)
    {
        mutex_unlock(&gR858_mutex);
        SONY_TRACE_RETURN(SONY_RESULT_OK);
    }

    result = sony_tuner_r858_get_tuner_num(pTuner, &ExtTunerNum, &IntTunerNum);
    if (result != SONY_RESULT_OK)
    {
        SONY_TRACE_RETURN(result);
    }

    g_pR858Tuner = (sony_r858_t *)pTuner->user;
    //rc=R858_Standby(R858_NUM1, R858_TUNER1, LT_ON);
    //rc=R858_Standby(R858_NUM2, R858_TUNER2, LT_ON);
    rc = R858_Standby(R858_NUM1, IntTunerNum, LT_ON);
    if (rc != RT_Success)
    {
        result = SONY_RESULT_ERROR_OTHER;
    }
    g_pR858Tuner->frequencykHz = 0;
    g_pR858Tuner->state = SONY_R858_STATE_SLEEP;
    g_pR858Tuner->tvSystem1 = R858_STD_SIZE;
    g_pR858Tuner->tvSystem2 = R858_STD_SIZE;

    pTuner->state = SONY_TUNER_STATE_SHUTDOWN;

    mutex_unlock(&gR858_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN(result);
}

static sony_result_t sony_tuner_r858_Resume(sony_tuner_terr_cable_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R858_ErrCode rc = RT_Success;

    R858_ExtTunerNum_Type ExtTunerNum;
    R858_IntTunerNum_Type IntTunerNum;

    SONY_TRACE_ENTER("sony_tuner_r858_Resume");

    if (!pTuner || !pTuner->user)
    {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gR858_mutex);

    if (pTuner->state == SONY_TUNER_STATE_ACTIVE)
    {
        mutex_unlock(&gR858_mutex);
        SONY_TRACE_RETURN(SONY_RESULT_OK);
    }

    result = sony_tuner_r858_get_tuner_num(pTuner, &ExtTunerNum, &IntTunerNum);
    if (result != SONY_RESULT_OK)
    {
        SONY_TRACE_RETURN(result);
    }

    g_pR858Tuner = (sony_r858_t *)pTuner->user;
    //rc = R858_Wakeup(R858_NUM1, R858_TUNER1);
    //rc = R858_Wakeup(R858_NUM2, R858_TUNER2);
    rc = R858_Wakeup(R858_NUM1, IntTunerNum);
    if (rc != RT_Success)
    {
        result = SONY_RESULT_ERROR_OTHER;
    }
    g_pR858Tuner->frequencykHz = 0;
    g_pR858Tuner->state = SONY_R858_STATE_SLEEP;
    g_pR858Tuner->tvSystem1 = R858_STD_SIZE;
    g_pR858Tuner->tvSystem2 = R858_STD_SIZE;

    pTuner->state = SONY_TUNER_STATE_ACTIVE;

    mutex_unlock(&gR858_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN(result);
}

static sony_result_t sony_tuner_r858_ReadRFLevel(sony_tuner_terr_cable_t *pTuner, int32_t *pRFLevel)
{
    sony_result_t result = SONY_RESULT_OK;
    R858_ErrCode rc;

    R858_ExtTunerNum_Type ExtTunerNum;
    R858_IntTunerNum_Type IntTunerNum;

    SONY_TRACE_ENTER("sony_tuner_r858_ReadRFLevel");

    if (!pTuner || !pTuner->user || !pRFLevel)
    {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    mutex_lock(&gR858_mutex);

    result = sony_tuner_r858_get_tuner_num(pTuner, &ExtTunerNum, &IntTunerNum);
    if (result != SONY_RESULT_OK)
    {
        SONY_TRACE_RETURN(result);
    }

    g_pR858Tuner = (sony_r858_t *)pTuner->user;
    //rc = R858_GetTotalRssiX1000(g_pR858Tuner->frequencykHz, g_pR858Tuner->tvSystem, pRFLevel);
    //rc = R858_GetTotalRssi(R858_NUM1, R858_TUNER1, g_pR858Tuner->frequencykHz, pRFLevel);
    //rc = R858_GetTotalRssi(R858_NUM2, R858_TUNER2, g_pR858Tuner->frequencykHz, pRFLevel);
    rc = R858_GetTotalRssi(R858_NUM1, IntTunerNum, g_pR858Tuner->frequencykHz, pRFLevel);
    mutex_unlock(&gR858_mutex);
    if (rc != RT_Success)
    {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_OTHER);
    }

    /* Add IFOUT value */
    switch (pTuner->system)
    {
    case SONY_DTV_SYSTEM_DVBT:
    case SONY_DTV_SYSTEM_DVBT2:
    case SONY_DTV_SYSTEM_DVBC2:
        //*pRFLevel -= 4000; /* -4.0dBm */
        *pRFLevel += 1; /* +1.0dBm */
        break;

    case SONY_DTV_SYSTEM_DVBC:
    case SONY_DTV_SYSTEM_ISDBC:
    case SONY_DTV_SYSTEM_J83B:
        //*pRFLevel -= 1500; /* -1.5dBm */
        *pRFLevel -= 1; /* -1.5dBm */
        break;

    case SONY_DTV_SYSTEM_ISDBT:
        //*pRFLevel -= 4500; /* -4.5dBm */
        *pRFLevel -= 4; /* -4.5dBm */
        break;

    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }

    SONY_TRACE_RETURN(result);
}
