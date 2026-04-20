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

#include "sony_tuner_r850.h"
#include "../../sony_demod.h"

#include "../I2C_Sys.h"    // "I2C_Sys" is only for SW porting reference.

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
const char* sony_tuner_r8508_version = "1.0.0.0";
sony_r850_t *g_pR850Tuner = NULL;

//static UINT32 gR850_mutex;
static struct mutex gR850_mutex;

extern I2C_TYPE R850_I2C;
extern I2C_LEN_TYPE R850_I2C_Len;



/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_r850_Initialize(sony_tuner_t * pTuner);

static sony_result_t sony_tuner_r850_Tune(sony_tuner_t * pTuner,
                                             uint32_t frequency,
                                             sony_dtv_system_t system,
                                             sony_dtv_bandwidth_t bandwidth);

static sony_result_t sony_tuner_r850_Sleep(sony_tuner_t * pTuner);

static sony_result_t sony_tuner_r850_Shutdown(sony_tuner_t * pTuner);

/*static sony_result_t sony_tuner_r850_Resume(sony_tuner_t * pTuner);*//*Clean warning*/

static sony_result_t sony_tuner_r850_ReadRFLevel(sony_tuner_t * pTuner, int32_t * pRFLevel);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_tuner_r850_Create(sony_tuner_t * pTuner,
                                      uint8_t i2cAddress,
                                      sony_i2c_t * pI2c,
                                      uint32_t configFlags,
                                      int dev_id,
                                      sony_r850_t * pR850Tuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_r850_Create");

    if ((!pI2c) || (!pR850Tuner) || (!pTuner))
	{
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pR850Tuner->state = SONY_R850_STATE_UNKNOWN; /* Chip is not accessed for now. */
    pR850Tuner->pI2c = pI2c;
    pR850Tuner->i2cAddress = i2cAddress;
    pR850Tuner->flags = configFlags;
    pR850Tuner->frequencykHz = 0;
    pR850Tuner->tvSystem = R850_STD_SIZE;
    pR850Tuner->dev_id = dev_id;
    pR850Tuner->user = NULL;

    /* Create local copy of instance data. */
    pTuner->Initialize = sony_tuner_r850_Initialize;
    pTuner->TerrCableTune = sony_tuner_r850_Tune;
	pTuner->SatTune = NULL;
    pTuner->Sleep = sony_tuner_r850_Sleep;
    pTuner->Shutdown = sony_tuner_r850_Shutdown;
    //pTuner->Resume = sony_tuner_r850_Resume;
    pTuner->ReadRFLevel = sony_tuner_r850_ReadRFLevel;
    pTuner->CalcRFLevelFromAGC = NULL;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->i2cAddress = i2cAddress;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;
    pTuner->user = pR850Tuner;

    R850_I2C.I2cAddr = i2cAddress;
    R850_I2C_Len.I2cAddr = i2cAddress;

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_r850_Initialize (sony_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R850_ErrCode rc;

    SONY_TRACE_ENTER ("sony_tuner_r850_Initialize");

    if (!pTuner || !pTuner->user)
    {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    mutex_lock(&gR850_mutex);
    g_pR850Tuner = (sony_r850_t *)pTuner->user;
    /* Call into underlying driver. */
    rc = R850_Init(R850_TUNER_1, g_pR850Tuner->tvSystem);
    if (rc!=RT_Success)
    {
        result=SONY_RESULT_ERROR_OTHER;
    }
    g_pR850Tuner->state = SONY_R850_STATE_SLEEP;
    g_pR850Tuner->tvSystem = R850_STD_SIZE;
    g_pR850Tuner->frequencykHz=0;

    //pTuner->state = SONY_TUNER_STATE_ACTIVE;

    mutex_unlock(&gR850_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_r850_Tune (sony_tuner_t * pTuner,
                                           uint32_t frequency,
                                           sony_dtv_system_t system,
                                           sony_dtv_bandwidth_t bandwidth)
{
    sony_result_t result = SONY_RESULT_OK;
    R850_Standard_Type aSystem;
    R850_Set_Info R850_INFO;
    R850_ErrCode rc;

    SONY_TRACE_ENTER ("sony_tuner_r850_Tune");

    if (!pTuner || !pTuner->user)
    {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Convert system, bandwidth into dtv system. */
    switch (system)
    {
    case SONY_DTV_SYSTEM_DVBC:
        switch (bandwidth)
        {
        case SONY_DTV_BW_6_MHZ:
            aSystem = R850_DVB_C_6M_IF_5M;
            break;
        case SONY_DTV_BW_7_MHZ:
            /* 7MHZ BW setting is the same as 8MHz BW */
        case SONY_DTV_BW_8_MHZ:
            aSystem = R850_DVB_C_8M_IF_5M;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT:
        switch (bandwidth)
        {
        case SONY_DTV_BW_5_MHZ:
        case SONY_DTV_BW_6_MHZ:
            aSystem = R850_DVB_T2_6M_IF_5M;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem = R850_DVB_T2_7M_IF_5M;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = R850_DVB_T_8M_IF_5M;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT2:
        switch (bandwidth)
        {
        case SONY_DTV_BW_1_7_MHZ:
            aSystem = R850_DVB_T2_1_7M_IF_5M;
            break;
        case SONY_DTV_BW_5_MHZ:
        case SONY_DTV_BW_6_MHZ:
            aSystem = R850_DVB_T2_6M_IF_5M;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem = R850_DVB_T2_7M_IF_5M;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = R850_DVB_T2_8M_IF_5M;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

        }
        break;

    case SONY_DTV_SYSTEM_DVBC2:
        switch (bandwidth)
        {
        case SONY_DTV_BW_6_MHZ:
            aSystem = R850_DVB_C_6M_IF_5M;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = R850_DVB_C_8M_IF_5M;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_ISDBT:
        switch (bandwidth)
        {
        case SONY_DTV_BW_6_MHZ:
            aSystem = R850_ISDB_T_IF_5M;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem = R850_DVB_T_8M_IF_5M;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = R850_DVB_T_8M_IF_5M;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_ISDBC:
        aSystem = R850_DVB_C_6M_IF_5M; /* ISDB-C uses DVB-C 6MHz BW setting */
        break;

    case SONY_DTV_SYSTEM_J83B:
        switch (bandwidth) {
        case SONY_DTV_BW_J83B_5_06_5_36_MSPS:
            aSystem = R850_J83B_IF_5M; /* J.83B (5.057, 5.361Msps commonly used in US) uses DVB-C 6MHz BW setting */
            break;
        case SONY_DTV_BW_J83B_5_60_MSPS:
            aSystem = R850_J83B_IF_5M; /* J.83B (5.6Msps used in Japan) uses special setting */
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    /* Intentional fall-through */
    case SONY_DTV_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    R850_INFO.R850_ClkOutMode = CLK_OUT_OFF;
    //R850_INFO.R850_IfAgc_Select = R850_IF_AGC1;
    R850_INFO.R850_LT = LT_OFF;

    mutex_lock(&gR850_mutex);
    g_pR850Tuner = (sony_r850_t *)pTuner->user;
    R850_INFO.RF_KHz = g_pR850Tuner->frequencykHz = frequency;
    R850_INFO.R850_Standard = g_pR850Tuner->tvSystem = aSystem;

    rc = R850_SetPllData(R850_TUNER_1, R850_INFO);
    if (rc != RT_Success)
    {
        mutex_unlock(&gR850_mutex);
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    g_pR850Tuner->state = SONY_R850_STATE_ACTIVE;
    mutex_unlock(&gR850_mutex);

    /* Allow the tuner time to settle */
    SONY_SLEEP(50);

    /* Assign current values. */
    pTuner->system = system;
    pTuner->frequencyKHz = frequency;
    pTuner->bandwidth = bandwidth;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_r850_Sleep (sony_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R850_ErrCode rc;

    SONY_TRACE_ENTER ("sony_tuner_r850_Sleep");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gR850_mutex);

    /*if (pTuner->state == SONY_TUNER_STATE_SLEEP || pTuner->state == SONY_TUNER_STATE_SHUTDOWN) {
        mutex_unlock(&gR850_mutex);
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }*/

    g_pR850Tuner=(sony_r850_t *)pTuner->user;
    rc=R850_Standby(R850_TUNER_1,LT_OFF);
    if (rc != RT_Success) {
        result=SONY_RESULT_ERROR_OTHER;
    }
    g_pR850Tuner->frequencykHz=0;
    g_pR850Tuner->state = SONY_R850_STATE_SLEEP;
    g_pR850Tuner->tvSystem = R850_STD_SIZE;

    //pTuner->state = SONY_TUNER_STATE_SLEEP;

    mutex_unlock(&gR850_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_r850_Shutdown (sony_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R850_ErrCode rc;

    SONY_TRACE_ENTER ("sony_tuner_r850_Shutdown");

    if (!pTuner || !pTuner->user)
	{
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gR850_mutex);
    /*if (pTuner->state == SONY_TUNER_STATE_SLEEP || pTuner->state == SONY_TUNER_STATE_SHUTDOWN)
	{
        mutex_unlock(&gR850_mutex);
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }*/

    g_pR850Tuner=(sony_r850_t *)pTuner->user;
    rc = R850_Standby(R850_TUNER_1, LT_OFF);
    if (rc != RT_Success)
	{
        result=SONY_RESULT_ERROR_OTHER;
    }
    g_pR850Tuner->frequencykHz=0;
    g_pR850Tuner->state = SONY_R850_STATE_SLEEP;
    g_pR850Tuner->tvSystem = R850_STD_SIZE;

    //pTuner->state = SONY_TUNER_STATE_SHUTDOWN;

    mutex_unlock(&gR850_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}
#if 0 /*Clean warning*/
static sony_result_t sony_tuner_r850_Resume(sony_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R850_ErrCode rc = RT_Success;

    SONY_TRACE_ENTER ("sony_tuner_r850_Resume");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gR850_mutex);

    /*if (pTuner->state == SONY_TUNER_STATE_ACTIVE) {
        mutex_unlock(&gR850_mutex);
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }*/

    g_pR850Tuner = (sony_r850_t *) pTuner->user;
    rc = R850_WakeUp(R850_TUNER_1);
    if (rc != RT_Success) {
        result = SONY_RESULT_ERROR_OTHER;
    }
    g_pR850Tuner->frequencykHz = 0;
    g_pR850Tuner->state = SONY_R850_STATE_SLEEP;
    g_pR850Tuner->tvSystem = R850_STD_SIZE;

    //pTuner->state = SONY_TUNER_STATE_ACTIVE;

    mutex_unlock(&gR850_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}
#endif
static sony_result_t sony_tuner_r850_ReadRFLevel (sony_tuner_t * pTuner, int32_t * pRFLevel)
{
    sony_result_t result = SONY_RESULT_OK;
    R850_ErrCode rc;
    SONY_TRACE_ENTER ("sony_tuner_r850_ReadRFLevel");

    if (!pTuner || !pTuner->user || !pRFLevel) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    mutex_lock(&gR850_mutex);
    g_pR850Tuner=(sony_r850_t *)pTuner->user;
    rc = R850_GetTotalRssi(R850_TUNER_1, g_pR850Tuner->frequencykHz, g_pR850Tuner->tvSystem, pRFLevel);
//	rc = R850_GetTotalRssi(R850_TUNER_1, g_pR850Tuner->frequencykHz,pRFLevel);
    mutex_unlock(&gR850_mutex);
    if(rc!=RT_Success){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    /* Add IFOUT value */
    switch (pTuner->system) {
    case SONY_DTV_SYSTEM_DVBT:
    case SONY_DTV_SYSTEM_DVBT2:
    case SONY_DTV_SYSTEM_DVBC2:
        //*pRFLevel -= 4000; /* -4.0dBm */
        *pRFLevel += 7;
        break;

    case SONY_DTV_SYSTEM_DVBC:
    case SONY_DTV_SYSTEM_ISDBC:
    case SONY_DTV_SYSTEM_J83B:
        //*pRFLevel -= 1500; /* -1.5dBm */
        *pRFLevel -= 2;
        break;

    case SONY_DTV_SYSTEM_ISDBT:
        //*pRFLevel -= 4500; /* -4.5dBm */
        *pRFLevel -= 5;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    SONY_TRACE_RETURN (result);
}
