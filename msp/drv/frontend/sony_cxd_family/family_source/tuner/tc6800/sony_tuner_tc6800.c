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

#include <linux/mutex.h>

#include "sony_tuner_tc6800.h"
#include "../../sony_demod.h"

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
const char* sony_tuner_tc6800_version = "0.1.28";
sony_tc6800_t *g_pTC6800Tuner = NULL;

//static UINT32 gTC6800_mutex;
//static struct mutex gTC6800_mutex;
static struct mutex gTC6800_mutex;



/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_tc6800_Initialize(sony_tuner_t * pTuner);

static sony_result_t sony_tuner_tc6800_Tune(sony_tuner_t * pTuner,
                                             uint32_t frequency,
                                             sony_dtv_system_t system,
                                             sony_dtv_bandwidth_t bandwidth);

static sony_result_t sony_tuner_tc6800_Sleep(sony_tuner_t * pTuner);

static sony_result_t sony_tuner_tc6800_Shutdown(sony_tuner_t * pTuner);

/*static sony_result_t sony_tuner_tc6800_Resume(sony_tuner_t * pTuner);*//*Clean warning*/

static sony_result_t sony_tuner_tc6800_ReadRFLevel(sony_tuner_t * pTuner, int32_t * pRFLevel);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_tuner_tc6800_Create(sony_tuner_t * pTuner,
                                      uint8_t i2cAddress,
                                      sony_i2c_t * pI2c,
                                      uint32_t configFlags,
                                      int dev_id,
                                      sony_tc6800_t * pTC6800Tuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_tc6800_Create");

    if ((!pI2c) || (!pTC6800Tuner) || (!pTuner))
    {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pTC6800Tuner->state = SONY_TC6800_STATE_UNKNOWN; /* Chip is not accessed for now. */
    pTC6800Tuner->pI2c = pI2c;
    pTC6800Tuner->i2cAddress = i2cAddress;
    pTC6800Tuner->flags = configFlags;
    pTC6800Tuner->frequencykHz = 0;
    pTC6800Tuner->tvSystem = SONY_DTV_SYSTEM_ANY;
    pTC6800Tuner->dev_id = dev_id;
    pTC6800Tuner->user = NULL;

    /* Create local copy of instance data. */
    pTuner->Initialize = sony_tuner_tc6800_Initialize;
    pTuner->TerrCableTune = sony_tuner_tc6800_Tune;
    pTuner->Sleep = sony_tuner_tc6800_Sleep;
    pTuner->Shutdown = sony_tuner_tc6800_Shutdown;
    //pTuner->Resume = sony_tuner_tc6800_Resume,
    pTuner->ReadRFLevel = sony_tuner_tc6800_ReadRFLevel;
    pTuner->CalcRFLevelFromAGC = NULL;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->i2cAddress = i2cAddress;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;
    pTuner->user = pTC6800Tuner;

    mutex_init(&gTC6800_mutex);

    //mt_dbg_printf(1, "%s[%d] ---- ReadRFLevel = 0x%08x\n", __FUNCTION__, __LINE__, pTuner->ReadRFLevel);

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_tc6800_Initialize (sony_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_tuner_tc6800_Initialize");

    if (!pTuner || !pTuner->user)
    {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    mutex_lock(&gTC6800_mutex);
    g_pTC6800Tuner = (sony_tc6800_t *)pTuner->user;
    /* Call into underlying driver. */
    result = mt_fe_tn_init_tc6800_sony(g_pTC6800Tuner);

    g_pTC6800Tuner->state = SONY_TC6800_STATE_SLEEP;
    g_pTC6800Tuner->tvSystem = SONY_DTV_SYSTEM_ANY;
    g_pTC6800Tuner->frequencykHz = 0;

    //pTuner->state = SONY_TUNER_STATE_ACTIVE;

    mutex_unlock(&gTC6800_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_tc6800_Tune (sony_tuner_t * pTuner,
                                           uint32_t frequency,
                                           sony_dtv_system_t system,
                                           sony_dtv_bandwidth_t bandwidth)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_tuner_tc6800_Tune");

    if (!pTuner || !pTuner->user)
    {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gTC6800_mutex);
    g_pTC6800Tuner = (sony_tc6800_t *)pTuner->user;

	g_pTC6800Tuner->tvSystem     = system;
	g_pTC6800Tuner->tvBandwidth  = bandwidth;
	g_pTC6800Tuner->frequencykHz = frequency;

	printk("%s[%d] ---- tvSystem = %d, freq = %d, bandwidth = %d\n", __FUNCTION__, __LINE__, system, frequency, bandwidth);

    result = mt_fe_tn_set_freq_tc6800_sony(g_pTC6800Tuner, frequency);

    if (result != SONY_RESULT_OK)
    {
        mutex_unlock(&gTC6800_mutex);
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    g_pTC6800Tuner->state = SONY_TC6800_STATE_ACTIVE;
    mutex_unlock(&gTC6800_mutex);

    /* Allow the tuner time to settle */
    SONY_SLEEP(50);

    /* Assign current values. */
    pTuner->system = system;
    pTuner->frequencyKHz = frequency;
    pTuner->bandwidth = bandwidth;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_tc6800_Sleep (sony_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_tuner_tc6800_Sleep");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gTC6800_mutex);

    /*if (pTuner->state == SONY_TUNER_STATE_SLEEP || pTuner->state == SONY_TUNER_STATE_SHUTDOWN) {
        mutex_unlock(&gTC6800_mutex);
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }*/

    g_pTC6800Tuner = (sony_tc6800_t *)pTuner->user;

    result = mt_fe_tn_sleep_tc6800_sony(g_pTC6800Tuner);

    g_pTC6800Tuner->frequencykHz = 0;
    g_pTC6800Tuner->state = SONY_TC6800_STATE_SLEEP;
    g_pTC6800Tuner->tvSystem = SONY_DTV_SYSTEM_ANY;

    //pTuner->state = SONY_TUNER_STATE_SLEEP;

    mutex_unlock(&gTC6800_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_tc6800_Shutdown (sony_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_tuner_tc6800_Shutdown");

    if (!pTuner || !pTuner->user)
	{
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gTC6800_mutex);
    /*if (pTuner->state == SONY_TUNER_STATE_SLEEP || pTuner->state == SONY_TUNER_STATE_SHUTDOWN)
	{
        mutex_unlock(&gTC6800_mutex);
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }*/

    g_pTC6800Tuner = (sony_tc6800_t *)pTuner->user;

    result = mt_fe_tn_sleep_tc6800_sony(g_pTC6800Tuner);

    g_pTC6800Tuner->frequencykHz = 0;
    g_pTC6800Tuner->state = SONY_TC6800_STATE_SLEEP;
    g_pTC6800Tuner->tvSystem = SONY_DTV_SYSTEM_ANY;

    //pTuner->state = SONY_TUNER_STATE_SHUTDOWN;

    mutex_unlock(&gTC6800_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}
#if 0 /*Clean warning*/
static sony_result_t sony_tuner_tc6800_Resume(sony_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_tuner_tc6800_Resume");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    mutex_lock(&gTC6800_mutex);

    if (pTuner->state == SONY_TUNER_STATE_ACTIVE) {
        mutex_unlock(&gTC6800_mutex);
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    g_pTC6800Tuner = (sony_tc6800_t *) pTuner->user;

    result = mt_fe_tn_wake_up_tc6800_sony(g_pTC6800Tuner);

    g_pTC6800Tuner->frequencykHz = 0;
    g_pTC6800Tuner->state = SONY_TC6800_STATE_SLEEP;
    g_pTC6800Tuner->tvSystem = SONY_DTV_SYSTEM_ANY;

    pTuner->state = SONY_TUNER_STATE_ACTIVE;

    mutex_unlock(&gTC6800_mutex);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}
#endif

static sony_result_t sony_tuner_tc6800_ReadRFLevel (sony_tuner_t * pTuner, int32_t * pRFLevel)
{
    sony_result_t result = SONY_RESULT_OK;
	int8_t strength;
    SONY_TRACE_ENTER ("sony_tuner_tc6800_ReadRFLevel");
    

    if (!pTuner || !pTuner->user || !pRFLevel) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    mutex_lock(&gTC6800_mutex);
    g_pTC6800Tuner = (sony_tc6800_t *)pTuner->user;

	result = mt_fe_tn_get_strength_tc6800_sony(g_pTC6800Tuner, &strength);
	*pRFLevel = strength;

    //mt_dbg_printf(1, "%s[%d] ---- strength = %d\n", __FUNCTION__, __LINE__, strength);

    mutex_unlock(&gTC6800_mutex);

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

