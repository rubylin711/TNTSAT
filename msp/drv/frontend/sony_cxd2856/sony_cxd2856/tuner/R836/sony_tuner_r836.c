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

#include "sony_tuner_r836.h"
#include "../../sony_demod.h"

#include "../I2C_Sys.h"    // "I2C_Sys" is only for SW porting reference.

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
const char* sony_tuner_r840_version = "1.0.0.0";
sony_r840_t *g_pR840Tuner = NULL;

//static UINT32 gR840_mutex;
static struct mutex gR840_mutex;

extern I2C_TYPE R840_I2C;
extern I2C_LEN_TYPE R840_I2C_Len;



/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_r840_Initialize(sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_r840_Tune(sony_tuner_terr_cable_t * pTuner,
                                             uint32_t frequency,
                                             sony_dtv_system_t system,
                                             sony_dtv_bandwidth_t bandwidth);

static sony_result_t sony_tuner_r840_Sleep(sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_r840_Shutdown(sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_r840_Resume(sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_r840_ReadRFLevel(sony_tuner_terr_cable_t * pTuner, int32_t * pRFLevel);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_tuner_r840_Create(sony_tuner_terr_cable_t * pTuner,
                                      uint8_t i2cAddress,
                                      sony_i2c_t * pI2c,
                                      uint32_t configFlags,
                                      int dev_id,
                                      sony_r840_t * pR840Tuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_r840_Create");

    if ((!pI2c) || (!pR840Tuner) || (!pTuner))
    {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pR840Tuner->state = SONY_R840_STATE_UNKNOWN; /* Chip is not accessed for now. */
    pR840Tuner->pI2c = pI2c;
    pR840Tuner->i2cAddress = i2cAddress;
    pR840Tuner->flags = configFlags;
    pR840Tuner->frequencykHz = 0;
    pR840Tuner->tvSystem = R840_STD_SIZE;
    pR840Tuner->dev_id = dev_id;
    pR840Tuner->user = NULL;

    /* Create local copy of instance data. */
    pTuner->Initialize = sony_tuner_r840_Initialize;
    pTuner->Tune = sony_tuner_r840_Tune;
    pTuner->Sleep = sony_tuner_r840_Sleep;
    pTuner->Shutdown = sony_tuner_r840_Shutdown;
    pTuner->Resume = sony_tuner_r840_Resume,
    pTuner->ReadRFLevel = sony_tuner_r840_ReadRFLevel;
    pTuner->CalcRFLevelFromAGC = NULL;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->i2cAddress = i2cAddress;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;
    pTuner->user = pR840Tuner;

    R840_I2C.I2cAddr = i2cAddress;
    R840_I2C_Len.I2cAddr = i2cAddress;

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_r840_Initialize (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R840_ErrCode rc;

    SONY_TRACE_ENTER ("sony_tuner_r840_Initialize");

    if (!pTuner || !pTuner->user)
    {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    mutex_lock(&gR840_mutex);
    g_pR840Tuner = (sony_r840_t *)pTuner->user;
    /* Call into underlying driver. */
    rc = R840_Init();
    if (rc!=RT_Success)
    {
        result=SONY_RESULT_ERROR_OTHER;
    }
    g_pR840Tuner->state = SONY_R840_STATE_SLEEP;
    g_pR840Tuner->tvSystem = R840_STD_SIZE;
    g_pR840Tuner->frequencykHz = 0;

    pTuner->state = SONY_TUNER_STATE_ACTIVE;

    mutex_unlock(&gR840_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_r840_Tune (sony_tuner_terr_cable_t * pTuner,
                                           uint32_t frequency,
                                           sony_dtv_system_t system,
                                           sony_dtv_bandwidth_t bandwidth)
{
    sony_result_t result = SONY_RESULT_OK;
    R840_Standard_Type aSystem;
    R840_Set_Info R840_INFO;
    R840_ErrCode rc;

    SONY_TRACE_ENTER ("sony_tuner_r840_Tune");

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
            aSystem = R840_DVB_C_6M_IF_5M;
            break;
        case SONY_DTV_BW_7_MHZ:
            /* 7MHZ BW setting is the same as 8MHz BW */
        case SONY_DTV_BW_8_MHZ:
            aSystem = R840_DVB_C_8M_IF_5M;
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
            aSystem = R840_DVB_T2_6M_IF_5M;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem = R840_DVB_T2_7M_IF_5M;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = R840_DVB_T_8M_IF_5M;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT2:
        switch (bandwidth)
        {
        case SONY_DTV_BW_1_7_MHZ:
            aSystem = R840_DVB_T2_1_7M_IF_5M;
            break;
        case SONY_DTV_BW_5_MHZ:
        case SONY_DTV_BW_6_MHZ:
            aSystem = R840_DVB_T2_6M_IF_5M;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem = R840_DVB_T2_7M_IF_5M;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = R840_DVB_T2_8M_IF_5M;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

        }
        break;

    case SONY_DTV_SYSTEM_DVBC2:
        switch (bandwidth)
        {
        case SONY_DTV_BW_6_MHZ:
            aSystem = R840_DVB_C_6M_IF_5M;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = R840_DVB_C_8M_IF_5M;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_ISDBT:
        switch (bandwidth)
        {
        case SONY_DTV_BW_6_MHZ:
            aSystem = R840_ISDB_T_IF_5M;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem = R840_DVB_T_8M_IF_5M;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = R840_DVB_T_8M_IF_5M;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_ISDBC:
        aSystem = R840_DVB_C_6M_IF_5M; /* ISDB-C uses DVB-C 6MHz BW setting */
        break;

    case SONY_DTV_SYSTEM_J83B:
        switch (bandwidth) {
        case SONY_DTV_BW_J83B_5_06_5_36_MSPS:
            aSystem = R840_J83B_IF_5M; /* J.83B (5.057, 5.361Msps commonly used in US) uses DVB-C 6MHz BW setting */
            break;
        case SONY_DTV_BW_J83B_5_60_MSPS:
            aSystem = R840_J83B_IF_5M; /* J.83B (5.6Msps used in Japan) uses special setting */
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
    R840_INFO.R840_ClkOutMode = R840_CLK_OUT_ON;//R840_CLK_OUT_OFF;
    R840_INFO.R840_IfAgc_Select = R840_IF_AGC1;
    R840_INFO.R840_LT = R840_LT_OFF;

    mutex_lock(&gR840_mutex);
    g_pR840Tuner = (sony_r840_t *)pTuner->user;
    R840_INFO.RF_KHz = g_pR840Tuner->frequencykHz = frequency;
    R840_INFO.R840_Standard = g_pR840Tuner->tvSystem = aSystem;

    rc = R840_SetPllData(R840_INFO);
    if (rc != RT_Success)
    {
        mutex_unlock(&gR840_mutex);
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    g_pR840Tuner->state = SONY_R840_STATE_ACTIVE;
    mutex_unlock(&gR840_mutex);

    /* Allow the tuner time to settle */
    SONY_SLEEP(50);

    /* Assign current values. */
    pTuner->system = system;
    pTuner->frequencyKHz = frequency;
    pTuner->bandwidth = bandwidth;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_r840_Sleep (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R840_ErrCode rc;

    SONY_TRACE_ENTER ("sony_tuner_r840_Sleep");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gR840_mutex);

    if (pTuner->state == SONY_TUNER_STATE_SLEEP || pTuner->state == SONY_TUNER_STATE_SHUTDOWN) {
        mutex_unlock(&gR840_mutex);
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    g_pR840Tuner=(sony_r840_t *)pTuner->user;
    rc=R840_Standby(R840_LT_OFF);
    if (rc != RT_Success) {
        result=SONY_RESULT_ERROR_OTHER;
    }
    g_pR840Tuner->frequencykHz=0;
    g_pR840Tuner->state = SONY_R840_STATE_SLEEP;
    g_pR840Tuner->tvSystem = R840_STD_SIZE;

    pTuner->state = SONY_TUNER_STATE_SLEEP;

    mutex_unlock(&gR840_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_r840_Shutdown (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R840_ErrCode rc;

    SONY_TRACE_ENTER ("sony_tuner_r840_Shutdown");

    if (!pTuner || !pTuner->user)
	{
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gR840_mutex);
    if (pTuner->state == SONY_TUNER_STATE_SLEEP || pTuner->state == SONY_TUNER_STATE_SHUTDOWN)
	{
        mutex_unlock(&gR840_mutex);
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    g_pR840Tuner=(sony_r840_t *)pTuner->user;
    rc = R840_Standby(R840_LT_OFF);
    if (rc != RT_Success)
	{
        result=SONY_RESULT_ERROR_OTHER;
    }
    g_pR840Tuner->frequencykHz=0;
    g_pR840Tuner->state = SONY_R840_STATE_SLEEP;
    g_pR840Tuner->tvSystem = R840_STD_SIZE;

    pTuner->state = SONY_TUNER_STATE_SHUTDOWN;

    mutex_unlock(&gR840_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_r840_Resume(sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    R840_ErrCode rc = RT_Success;

    SONY_TRACE_ENTER ("sony_tuner_r840_Resume");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gR840_mutex);

    if (pTuner->state == SONY_TUNER_STATE_ACTIVE) {
        mutex_unlock(&gR840_mutex);
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    g_pR840Tuner = (sony_r840_t *) pTuner->user;
    rc = R840_WakeUp();
    if (rc != RT_Success) {
        result = SONY_RESULT_ERROR_OTHER;
    }
    g_pR840Tuner->frequencykHz = 0;
    g_pR840Tuner->state = SONY_R840_STATE_SLEEP;
    g_pR840Tuner->tvSystem = R840_STD_SIZE;

    pTuner->state = SONY_TUNER_STATE_ACTIVE;

    mutex_unlock(&gR840_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_r840_ReadRFLevel (sony_tuner_terr_cable_t * pTuner, int32_t * pRFLevel)
{
    sony_result_t result = SONY_RESULT_OK;
    R840_ErrCode rc;
    SONY_TRACE_ENTER ("sony_tuner_r840_ReadRFLevel");

    if (!pTuner || !pTuner->user || !pRFLevel) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    mutex_lock(&gR840_mutex);
    g_pR840Tuner=(sony_r840_t *)pTuner->user;
    rc = R840_GetTotalRssi(g_pR840Tuner->frequencykHz, g_pR840Tuner->tvSystem, pRFLevel);
    mutex_unlock(&gR840_mutex);
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
