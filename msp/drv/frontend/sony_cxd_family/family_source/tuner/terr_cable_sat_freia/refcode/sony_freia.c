/*------------------------------------------------------------------------------
  Copyright 2020-2022 Sony Semiconductor Solutions Corporation

  Last Updated  : 2022/08/10
  File Revision : 1.0.4.0
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 Based on FREIA application note 1.2.0
------------------------------------------------------------------------------*/

#include "sony_freia.h"

/*------------------------------------------------------------------------------
 Definitions of static const table
------------------------------------------------------------------------------*/
#define AUTO         (0xFF) /* For IF_OUT_SEL and AGC_SEL, it means that the value is desided by config flags. */
                            /* For RF_GAIN, it means that RF_GAIN_SEL(SubAddr:0x4E) = 1 */
#define OFFSET(ofs)  ((uint8_t)(ofs) & 0x1F)
#define BW_6         (0x00)
#define BW_7         (0x01)
#define BW_8         (0x02)
#define BW_1_7       (0x03)

/**
  @brief Sony silicon tuner setting for each broadcasting system.

         These values are optimized for Sony demodulators.
         The user have to change these values if other demodulators are used.
         Please check Sony silicon tuner application note for detail.
*/
static const sony_freia_terr_adjust_param_t g_terr_param_table[SONY_FREIA_TERR_TV_SYSTEM_NUM] = {
    /*
         IF_BPF_GC                                           BW              BW_OFFSET           IF_OUT_SEL
     RF_GAIN |     RFOVLD_DET_LV1    IFOVLD_DET_LV  IF_BPF_F0 |   FIF_OFFSET     |       AGC_SEL    |
       |     |    (VL)  (VH)  (U)   (VL)  (VH)  (U)    |      |       |          |          |       |             */
    {AUTO, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, BW_6,  OFFSET(0),  OFFSET(0),  AUTO,   AUTO}, /**< SONY_FREIA_TV_SYSTEM_UNKNOWN */
    /* Analog */
    {AUTO, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00, BW_6,  OFFSET(0),  OFFSET(1),  AUTO,   AUTO}, /**< SONY_FREIA_ATV_MN_EIAJ   (System-M (Japan)) */
    {AUTO, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00, BW_6,  OFFSET(0),  OFFSET(1),  AUTO,   AUTO}, /**< SONY_FREIA_ATV_MN_SAP    (System-M (US)) */
    {AUTO, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00, BW_6,  OFFSET(3),  OFFSET(1),  AUTO,   AUTO}, /**< SONY_FREIA_ATV_MN_A2     (System-M (Korea)) */
    {AUTO, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00, BW_7,  OFFSET(11), OFFSET(5),  AUTO,   AUTO}, /**< SONY_FREIA_ATV_BG        (System-B/G) */
    {AUTO, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00, BW_8,  OFFSET(2),  OFFSET(-3), AUTO,   AUTO}, /**< SONY_FREIA_ATV_I         (System-I) */
    {AUTO, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00, BW_8,  OFFSET(2),  OFFSET(-3), AUTO,   AUTO}, /**< SONY_FREIA_ATV_DK        (System-D/K) */
    {AUTO, 0x01, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x00, BW_8,  OFFSET(2),  OFFSET(-3), AUTO,   AUTO}, /**< SONY_FREIA_ATV_L         (System-L) */
    {AUTO, 0x01, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x00, BW_8,  OFFSET(-1), OFFSET(4),  AUTO,   AUTO}, /**< SONY_FREIA_ATV_L_DASH    (System-L DASH) */
    /* Digital */
    {AUTO, 0x06, 0x0E, 0x0E, 0x0E, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-6), OFFSET(-3), AUTO,   AUTO}, /**< SONY_FREIA_DTV_8VSB      (ATSC 8VSB) */
    {AUTO, 0x06, 0x0E, 0x0E, 0x0E, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-9), OFFSET(-5), AUTO,   AUTO}, /**< SONY_FREIA_DTV_ISDBT_6   (ISDB-T 6MHzBW) */
    {AUTO, 0x06, 0x0E, 0x0E, 0x0E, 0x03, 0x03, 0x03, 0x00, BW_7,  OFFSET(-7), OFFSET(-6), AUTO,   AUTO}, /**< SONY_FREIA_DTV_ISDBT_7   (ISDB-T 7MHzBW) */
    {AUTO, 0x06, 0x0E, 0x0E, 0x0E, 0x03, 0x03, 0x03, 0x00, BW_8,  OFFSET(-5), OFFSET(-7), AUTO,   AUTO}, /**< SONY_FREIA_DTV_ISDBT_8   (ISDB-T 8MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-8), OFFSET(-3), AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBT_5    (DVB-T 5MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-8), OFFSET(-3), AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBT_6    (DVB-T 6MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_7,  OFFSET(-6), OFFSET(-5), AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBT_7    (DVB-T 7MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_8,  OFFSET(-4), OFFSET(-6), AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBT_8    (DVB-T 8MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_1_7,OFFSET(-10),OFFSET(-10),AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBT2_1_7 (DVB-T2 1.7MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-8), OFFSET(-3), AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBT2_5   (DVB-T2 5MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-8), OFFSET(-3), AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBT2_6   (DVB-T2 6MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_7,  OFFSET(-6), OFFSET(-5), AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBT2_7   (DVB-T2 7MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_8,  OFFSET(-4), OFFSET(-6), AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBT2_8   (DVB-T2 8MHzBW) */
    {AUTO, 0x03, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-6), OFFSET(-4), AUTO,   AUTO}, /**< SONY_FREIA_DTV_CABLE_6   (DVB-C 6MHzBW/ISDB-C/J.83B) */
    {AUTO, 0x03, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_8,  OFFSET(-2), OFFSET(-3), AUTO,   AUTO}, /**< SONY_FREIA_DTV_CABLE_8   (DVB-C 8MHzBW,7MHzBW) */
    {AUTO, 0x04, 0x09, 0x09, 0x09, 0x02, 0x02, 0x02, 0x00, BW_6,  OFFSET(-6), OFFSET(-2), AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBC2_6   (DVB-C2 6MHzBW) */
    {AUTO, 0x04, 0x09, 0x09, 0x09, 0x02, 0x02, 0x02, 0x00, BW_8,  OFFSET(-2), OFFSET(0),  AUTO,   AUTO}, /**< SONY_FREIA_DTV_DVBC2_8   (DVB-C2 8MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-8), OFFSET(-3), AUTO,   AUTO}, /**< SONY_FREIA_DTV_ATSC3_6   (ATSC 3.0 6MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_7,  OFFSET(-6), OFFSET(-5), AUTO,   AUTO}, /**< SONY_FREIA_DTV_ATSC3_7   (ATSC 3.0 7MHzBW) */
    {AUTO, 0x06, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_8,  OFFSET(-4), OFFSET(-6), AUTO,   AUTO}, /**< SONY_FREIA_DTV_ATSC3_8   (ATSC 3.0 8MHzBW) */
    {AUTO, 0x03, 0x0B, 0x0B, 0x0B, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-5), OFFSET(2),  AUTO,   AUTO}, /**< SONY_FREIA_DTV_SKP_OPT   (J.83B 5.6Msps) */
    {AUTO, 0x06, 0x0E, 0x0E, 0x0E, 0x03, 0x03, 0x03, 0x00, BW_8,  OFFSET(2),  OFFSET(1),  AUTO,   AUTO}  /**< SONY_FREIA_DTV_DTMB      (DTMB) */
};

/*------------------------------------------------------------------------------
 Definitions of static functions
------------------------------------------------------------------------------*/
/**
 @brief Configure the FREIA tuner from Power On to Sleep state.
*/
static sony_result_t X_pon(sony_freia_t *pTuner);
/**
 @brief Configure the FREIA tuner for specified terrestrial broadcasting system.
*/
static sony_result_t TER_tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem, uint8_t vcoCal);
/**
 @brief Configure the FREIA tuner for specified satellite broadcasting system.
*/
static sony_result_t SAT_tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem,  uint32_t symbolRateksps, uint8_t vcoCal);
/**
 @brief Configure the FREIA tuner for specified satellite broadcasting system delivered by cable broadcast.
*/
static sony_result_t TER_IQ_tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem, uint32_t symbolRateksps, uint8_t vcoCal);
/**
 @brief The last part of terrestrial tuning sequence.
*/
static sony_result_t TER_tune_end(sony_freia_t *pTuner);
/**
 @brief The last part of satellite tuning sequence.
*/
static sony_result_t SAT_tune_end(sony_freia_t *pTuner);
/**
 @brief The last part of terrestrial tuning sequence with IQ output.
*/
static sony_result_t TER_IQ_tune_end(sony_freia_t *pTuner);
/**
 @brief Configure the FREIA tuner from terrestrial to Power Save state.
*/
static sony_result_t TER_fin(sony_freia_t *pTuner);
/**
 @brief Configure the FREIA tuner from satellite to Power Save state.
*/
static sony_result_t SAT_fin(sony_freia_t *pTuner);
/**
 @brief Configure the FREIA tuner from IQ output to Power Save state.
*/
static sony_result_t TER_IQ_fin(sony_freia_t *pTuner);

/**
 @brief Configure the FREIA tuner to Oscillation Stop state.
*/
static sony_result_t X_oscdis(sony_freia_t *pTuner);
/**
 @brief Configure the FREIA tuner back from Oscillation Stop state.
*/
static sony_result_t X_oscen(sony_freia_t *pTuner);
/**
 @brief Reading gain information to calculate IF and RF gain levels.
*/
static sony_result_t X_read_agc(sony_freia_t *pTuner, uint8_t *pSatCompensationReg, uint8_t *pTerrCompensationReg,
    uint8_t *pIFAGCReg, uint8_t *pRFAGCReg);

/*------------------------------------------------------------------------------
 Definitions of internal used macros
------------------------------------------------------------------------------*/
/**
 @brief Macro to check that the system is DVB-T/T2 or not. (ATSC 3.0 uses DVB-T2 setting too)
*/
#define SONY_FREIA_IS_DVB_T_T2(tvSystem) ((((tvSystem) >= SONY_FREIA_DTV_DVBT_5) && ((tvSystem) <= SONY_FREIA_DTV_DVBT2_8))\
    || (((tvSystem) >= SONY_FREIA_DTV_ATSC3_6) && ((tvSystem) <= SONY_FREIA_DTV_ATSC3_8)))

/*------------------------------------------------------------------------------
 Implementation of public functions.
------------------------------------------------------------------------------*/

sony_result_t sony_freia_Create(sony_freia_t *pTuner, uint8_t i2cAddress, sony_i2c_t *pI2c, uint32_t flags)
{
    SONY_TRACE_ENTER("sony_freia_Create");

    if((!pTuner) || (!pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* SONY_FREIA_CONFIG_EXT_REF and SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL cannot be used at the same time. */
    if((flags & SONY_FREIA_CONFIG_EXT_REF) && (flags & SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }

    pTuner->state = SONY_FREIA_STATE_UNKNOWN; /* Chip is not accessed for now. */
    pTuner->pI2c = pI2c;
    pTuner->i2cAddress = i2cAddress;
    pTuner->flags = flags;
    pTuner->frequencykHz = 0;
    pTuner->symbolRateksps = 0;
    pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;
    pTuner->chipId = SONY_FREIA_CHIP_ID_UNKNOWN;
    pTuner->isFreesatMode = 0;
    pTuner->isExternalOvldTc = 0;
    pTuner->agcLimiterSetting = 0;
    pTuner->isQDumpOn = 0;
    pTuner->pTerrParamTable = g_terr_param_table;

    /* Xtal OSC reference value */
    if(flags & SONY_FREIA_CONFIG_EXT_REF){
        pTuner->xosc_sel = 0x00;
        pTuner->xosc_cap_set = 0x00;
    }else{
        pTuner->xosc_sel = 0x04;     /* 4 x 25 = 100uA */
        pTuner->xosc_cap_set = 0x1E; /* 30 x 0.25 = 7.5pF(6pF Xtal) */
    }

    pTuner->user = NULL;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_Initialize(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_Initialize");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    {
        uint8_t data = 0x00;

        /* Confirm connected device is FREIA */
        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7F, &data, 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if((data & 0xFC) == 0xF0){
            /* FREIA Plus */
            pTuner->chipId = SONY_FREIA_CHIP_ID_6868ER;
        }else if((data & 0xFC) == 0xF8){
            /* FREIA */
            pTuner->chipId = SONY_FREIA_CHIP_ID_6866AER;
        }else if ((data & 0xFC) == 0xF4){
            /* ASCOT4 */
            pTuner->chipId = SONY_FREIA_CHIP_ID_6866ER;
        } else {
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }
    }

    /* X_pon sequence */
    result = X_pon(pTuner);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

    /* Dummy tune to set RF parameter if terrestrial RF active mode is used */
    if((pTuner->flags & SONY_FREIA_CONFIG_POWERSAVE_TERR_MASK)
        >= SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_DISABLE ){
        result = TER_tune(pTuner, 666000, SONY_FREIA_DTV_DVBT_8, 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        SONY_SLEEP(50);

        result = TER_tune_end(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        result = TER_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
    }

    if((pTuner->flags & SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_FREIA_CONFIG_EXT_REF)){
        /* Disable Xtal */
        result = X_oscdis(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
    }

    pTuner->state = SONY_FREIA_STATE_SLEEP;
    pTuner->frequencykHz = 0;
    pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;
    pTuner->symbolRateksps = 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_terr_Tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_terr_Tune");

    /* Argument check */
    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if(!SONY_FREIA_IS_ATV(tvSystem) && !SONY_FREIA_IS_DTV(tvSystem)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Rough frequency range check */
    if((frequencykHz < 1000) || (frequencykHz > 1200000)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
    }

    /* State check */
    switch(pTuner->state){
    case SONY_FREIA_STATE_SLEEP:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;

        if((pTuner->flags & SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_FREIA_CONFIG_EXT_REF)){
            /* Enable Xtal */
            result = X_oscen(pTuner);
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        }

        break;

    case SONY_FREIA_STATE_ACTIVE_S:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;

        result = SAT_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    case SONY_FREIA_STATE_ACTIVE_T:
        break;

    case SONY_FREIA_STATE_ACTIVE_IQ:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;

        result = TER_IQ_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Broadcasting system dependent setting and tuning. */
    result = TER_tune(pTuner, frequencykHz, tvSystem, 1);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pTuner->state = SONY_FREIA_STATE_ACTIVE_T;
    pTuner->frequencykHz = frequencykHz;
    pTuner->tvSystem = tvSystem;
    pTuner->symbolRateksps = 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_terr_TuneEnd(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_terr_TuneEnd");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* State check (terrestrial only) */
    if(pTuner->state != SONY_FREIA_STATE_ACTIVE_T){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = TER_tune_end(pTuner);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_sat_Tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem, uint32_t symbolRateksps)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_sat_Tune");

    /* Argument check */
    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if(!SONY_FREIA_IS_STV(tvSystem)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Rough frequency range check */
    if((frequencykHz < 500000) || (frequencykHz > 3500000)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
    }

    /* ChipID check */
    if (pTuner->chipId == SONY_FREIA_CHIP_ID_6866ER) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }

    /* Symbol rate is fixed in ISDB-S */
    if(tvSystem == SONY_FREIA_STV_ISDBS){
        symbolRateksps = 28860;
    }else if(tvSystem == SONY_FREIA_STV_ISDBS3){
        symbolRateksps = 33756;
    }

    /* State check */
    switch(pTuner->state){
    case SONY_FREIA_STATE_SLEEP:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;

        if((pTuner->flags & SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_FREIA_CONFIG_EXT_REF)){
            /* Enable Xtal */
            result = X_oscen(pTuner);
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        }

        break;

    case SONY_FREIA_STATE_ACTIVE_T:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;

        result = TER_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    case SONY_FREIA_STATE_ACTIVE_S:
        break;

    case SONY_FREIA_STATE_ACTIVE_IQ:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;

        result = TER_IQ_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Broadcasting system dependent setting and tuning. */
    result = SAT_tune(pTuner, frequencykHz, tvSystem, symbolRateksps, 1);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_SLEEP(10);

    result = SAT_tune_end(pTuner);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pTuner->state = SONY_FREIA_STATE_ACTIVE_S;
    pTuner->frequencykHz = ((frequencykHz + 2) / 4) * 4;
    pTuner->tvSystem = tvSystem;
    pTuner->symbolRateksps = symbolRateksps;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_terr_IQ_Tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem, uint32_t symbolRateksps)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_terr_IQ_Tune");

    /* Argument check */
    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if(!SONY_FREIA_IS_STV(tvSystem)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Rough frequency range check */
    if((frequencykHz < 1000) || (frequencykHz > 1200000)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
    }

    /* State check */
    switch(pTuner->state){
    case SONY_FREIA_STATE_SLEEP:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;

        if((pTuner->flags & SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_FREIA_CONFIG_EXT_REF)){
            /* Enable Xtal */
            result = X_oscen(pTuner);
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        }

        break;

    case SONY_FREIA_STATE_ACTIVE_S:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;

        result = SAT_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    case SONY_FREIA_STATE_ACTIVE_T:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;

        result = TER_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    case SONY_FREIA_STATE_ACTIVE_IQ:

        break;

    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Broadcasting system dependent setting and tuning. */
    result = TER_IQ_tune(pTuner, frequencykHz, tvSystem, symbolRateksps, 1);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pTuner->state = SONY_FREIA_STATE_ACTIVE_IQ;
    pTuner->frequencykHz = frequencykHz;
    pTuner->tvSystem = tvSystem;
    pTuner->symbolRateksps = 0;


    SONY_TRACE_RETURN(result);
}

sony_result_t sony_freia_terr_IQ_TuneEnd(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_terr_IQ_TuneEnd");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* State check (IQ mode only) */
    if(pTuner->state != SONY_FREIA_STATE_ACTIVE_IQ){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = TER_IQ_tune_end(pTuner);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_freia_ShiftFRF(sony_freia_t *pTuner, uint32_t frequencykHz)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_ShiftFRF");

    /* Argument check */
    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* State check */
    switch(pTuner->state){
    case SONY_FREIA_STATE_ACTIVE_T:
        /* Rough frequency range check */
        if((frequencykHz < 1000) || (frequencykHz > 1200000)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
        }

        result = TER_tune(pTuner, frequencykHz, pTuner->tvSystem, 0);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        pTuner->frequencykHz = frequencykHz;

        SONY_SLEEP(10);

        result = TER_tune_end(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    case SONY_FREIA_STATE_ACTIVE_S:
        /* Rough frequency range check */
        if((frequencykHz < 500000) || (frequencykHz > 3500000)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
        }

        result = SAT_tune(pTuner, frequencykHz, pTuner->tvSystem, pTuner->symbolRateksps, 0);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        pTuner->frequencykHz = frequencykHz;

        SONY_SLEEP(10);

        result = SAT_tune_end(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    case SONY_FREIA_STATE_ACTIVE_IQ:
        /* Rough frequency range check */
        if((frequencykHz < 1000) || (frequencykHz > 1200000)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
        }

        result = TER_IQ_tune(pTuner, frequencykHz, pTuner->tvSystem, pTuner->symbolRateksps, 0);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        pTuner->frequencykHz = frequencykHz;

        SONY_SLEEP(10);

        result = TER_IQ_tune_end(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_Sleep(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_Sleep");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* State check */
    switch(pTuner->state){
    case SONY_FREIA_STATE_SLEEP:
        /* Nothing to do */
        SONY_TRACE_RETURN(SONY_RESULT_OK);

    case SONY_FREIA_STATE_ACTIVE_T:
        result = TER_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        break;

    case SONY_FREIA_STATE_ACTIVE_S:
        result = SAT_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        break;

    case SONY_FREIA_STATE_ACTIVE_IQ:
        result = TER_IQ_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        break;

    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    if((pTuner->flags & SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_FREIA_CONFIG_EXT_REF)){
        /* Disable Xtal */
        result = X_oscdis(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
    }

    pTuner->state = SONY_FREIA_STATE_SLEEP;
    pTuner->frequencykHz = 0;
    pTuner->tvSystem = SONY_FREIA_TV_SYSTEM_UNKNOWN;
    pTuner->symbolRateksps = 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_SetGPO(sony_freia_t *pTuner, uint8_t id, uint8_t val)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_SetGPO");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    switch(id){
    case 0:
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x85, (uint8_t)(val ? 0x01 : 0x00));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        break;
    case 1:
        /* GPIO1_IN_SEL = 0 (0x86) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x86, (uint8_t)(val ? 0x01 : 0x00));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_GetGPI(sony_freia_t *pTuner, uint8_t id, uint8_t *pVal)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER("sony_freia_GetGPI");

    if((!pTuner) || (!pTuner->pI2c) || (!pVal)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    switch(id){
    case 0:
        /* GPIO1_IN_SEL = 0 (0x85) */
        result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x85, 0x10, 0x10);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        break;
    case 1:
        /* GPIO1_IN_SEL = 1 (0x86) */
        result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x86, 0x10, 0x10);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Read GPIO1_IN (0x4B) */
    result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x4B, &data, 1);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    switch(id) {
    case 0:
        *pVal = (uint8_t)((data >> 4) & 0x01);
        break;
    case 1:
        *pVal = (uint8_t)(data & 0x01);
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_SetRfExtCtrl(sony_freia_t *pTuner, uint8_t enable)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_SetRfExtCtrl");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* RF_EXT bit setting (0x67) */
    result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x67, (uint8_t)(enable ? 0x01 : 0x00), 0x01);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_SetFreesatMode(sony_freia_t *pTuner, uint8_t enable)
{
    SONY_TRACE_ENTER("sony_freia_SetFreesatMode");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pTuner->isFreesatMode = enable ? 1 : 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_SetExternalOvldTcMode(sony_freia_t *pTuner, uint8_t enable)
{
    SONY_TRACE_ENTER("sony_freia_SetExternalOvldTcMode");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pTuner->isExternalOvldTc = enable ? 1 : 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_SetAgcLimiter(sony_freia_t *pTuner, uint8_t setting)
{
    SONY_TRACE_ENTER("sony_freia_SetAgcLimiter");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pTuner->agcLimiterSetting = setting ? 1 : 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_SetQDump(sony_freia_t *pTuner, uint8_t setting)
{
    SONY_TRACE_ENTER("sony_freia_SetQDump");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pTuner->isQDumpOn = setting ? 1 : 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_terr_ReadRssi(sony_freia_t *pTuner, int32_t *pRssi)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t ifagcreg = 0;
    uint8_t rfagcreg = 0;
    uint8_t sat_compensate_reg = 0;
    uint8_t terr_compensate_reg = 0;
    int32_t ifgain = 0;
    int32_t rfgain = 0;

    SONY_TRACE_ENTER("sony_freia_terr_ReadRssi");

    if((!pTuner) || (!pTuner->pI2c) || (!pRssi)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Terrestrial only */
    if(pTuner->state != SONY_FREIA_STATE_ACTIVE_T){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = X_read_agc(pTuner, &sat_compensate_reg, &terr_compensate_reg, &ifagcreg, &rfagcreg);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /*
        IFGAIN = if(AGC < 0.35){
                     35.28 + IF_BPF_GC - 76.54 * AGC
                 }else if(AGC < 0.4){
                     30.69 + IF_BPF_GC - 63.40 * AGC
                 }else{
                     5.33 + IF_BPF_GC
                 }
        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
        So...
        IFGAIN(100xdB) = if(IFAGCReg * 150 < 8925){
                             3528 + (IF_BPF_GC * 100) - (7654 * (IFAGCReg * 150)) / 25500
                         }else if(IFAGCReg * 150 < 10200){
                             3069 + (IF_BPF_GC * 100) - (6340 * (IFAGCReg * 150)) / 25500
                         }else{
                             533 + (IF_BPF_GC * 100)
                         }
    */
    {
        const int32_t if_bpf_gc_table[] = {-3, -1, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 23, 23};
        uint8_t data = 0;
        int32_t if_bpf_gc_x100 = 0;
        int32_t agcreg_x150 = ifagcreg * 150;

        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x69, &data, 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if_bpf_gc_x100 = if_bpf_gc_table[data & 0x0F] * 100;

        if(agcreg_x150 < 8925){
            ifgain = 3528 + if_bpf_gc_x100 - (7654 * agcreg_x150 + 12750) / 25500; /* Round */
        }else if(agcreg_x150 < 10200){
            ifgain = 3069 + if_bpf_gc_x100 - (6340 * agcreg_x150 + 12750) / 25500; /* Round */
        }else{
            ifgain = 533+ if_bpf_gc_x100;
        }
    }

    if (SONY_FREIA_IS_DVB_T_T2(pTuner->tvSystem)) {
    /*
        RFGAIN = if(max(AGC,RFAGC) < 0.3){
                     RF_GAIN_MAX - (-16.51)  - (1.19  * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.35){
                     RF_GAIN_MAX - (-31.08)  - (49.13 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.40){
                     RF_GAIN_MAX - (-35.95)  - (63.07 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.50){
                     RF_GAIN_MAX - (-44.71)  - (85.48 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.60){
                     RF_GAIN_MAX - (-38.56)  - (72.82 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.67){
                     RF_GAIN_MAX - (-25.73)  - (51.93 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.70){
                     RF_GAIN_MAX - (-35.98)  - (65.87 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.80){
                     RF_GAIN_MAX - (-35.40)  - (65.72 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.95){
                     RF_GAIN_MAX - (-32.14)  - (61.50 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.07){
                     RF_GAIN_MAX - (-52.86)  - (82.91 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.15){
                     RF_GAIN_MAX - (-99.68)  - (126.61 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.25){
                     RF_GAIN_MAX - (-197.89) - (212.31 * max(AGC,RFAGC))
                 }else{
                     RF_GAIN_MAX - 66.00
                 }
        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
                  RFAGC(V) = RFAGCReg(by X_read_ss) * 1.5 / 255
        So...
        RFGAIN(100xdB) = if(maxagcreg * 150 < 7650){
                             RF_GAIN_MAX * 100 + 1651  - (1.19   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 8925){
                             RF_GAIN_MAX * 100 + 3108  - (49.13  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 10200){
                             RF_GAIN_MAX * 100 + 3595  - (63.07  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 12750){
                             RF_GAIN_MAX * 100 + 4471  - (85.48  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 15300){
                             RF_GAIN_MAX * 100 + 3856  - (72.82  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 17085){
                             RF_GAIN_MAX * 100 + 2573  - (51.93  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 17850){
                             RF_GAIN_MAX * 100 + 3598  - (65.87  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 20400){
                             RF_GAIN_MAX * 100 + 3540  - (65.72  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 25225){
                             RF_GAIN_MAX * 100 + 3214  - (61.50  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 27285){
                             RF_GAIN_MAX * 100 + 5286  - (82.91  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 29325){
                             RF_GAIN_MAX * 100 + 9968  - (126.61 * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 31875){
                             RF_GAIN_MAX * 100 + 19789 - (212.31 * (maxagcreg * 150 / 255))
                         }else{
                             RFGAIN_MAX * 100 - 8000
                         }
        (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
    */
        {
            int32_t maxagcreg_x150 = 0;
            int32_t rfgainmax_x100 = 0;

            if(ifagcreg > rfagcreg){
                maxagcreg_x150 = ifagcreg * 150;
            }else{
                maxagcreg_x150 = rfagcreg * 150;
            }

            if(pTuner->frequencykHz < 42000){
                rfgainmax_x100 = 3371;
            }else if(pTuner->frequencykHz < 71500){
                rfgainmax_x100 = 3416;
            }else if(pTuner->frequencykHz < 85500){
                rfgainmax_x100 = 3527;
            }else if(pTuner->frequencykHz < 135500){
                rfgainmax_x100 = 3663;
            }else if(pTuner->frequencykHz < 163500){
                rfgainmax_x100 = 3654;
            }else if(pTuner->frequencykHz < 198500){
                rfgainmax_x100 = 3755;
            }else if(pTuner->frequencykHz < 250001){
                rfgainmax_x100 = 3894;
            }else if(pTuner->frequencykHz < 305500){
                rfgainmax_x100 = 3061;
            }else if(pTuner->frequencykHz < 353500){
                rfgainmax_x100 = 3188;
            }else if(pTuner->frequencykHz < 457500){
                rfgainmax_x100 = 3247;
            }else if(pTuner->frequencykHz < 609500){
                rfgainmax_x100 = 3352;
            }else if(pTuner->frequencykHz < 769500){
                rfgainmax_x100 = 3457;
            }else if(pTuner->frequencykHz < 841500){
                rfgainmax_x100 = 3512;
            }else if(pTuner->frequencykHz < 890000){
                rfgainmax_x100 = 3704;
            }else if(pTuner->frequencykHz < 906000){
                rfgainmax_x100 = 3479;
            }else if(pTuner->frequencykHz < 930000){
                rfgainmax_x100 = 3379;
            }else if(pTuner->frequencykHz < 946000){
                rfgainmax_x100 = 3263;
            }else if(pTuner->frequencykHz < 986000){
            rfgainmax_x100 = 3109;
            }else{
                rfgainmax_x100 = 2952;
            }

            if(maxagcreg_x150 < 7650){
                rfgain = rfgainmax_x100 + 1651  - (119   * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 8925){
                rfgain = rfgainmax_x100 + 3108  - (4913  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 10200){
                rfgain = rfgainmax_x100 + 3595  - (6307  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 12750){
                rfgain = rfgainmax_x100 + 4471  - (8548  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 15300){
                rfgain = rfgainmax_x100 + 3856  - (7282  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 17085){
                rfgain = rfgainmax_x100 + 2573  - (5193  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 17850){
                rfgain = rfgainmax_x100 + 3598  - (6587  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 20400){
                rfgain = rfgainmax_x100 + 3540  - (6572  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 24225){
                rfgain = rfgainmax_x100 + 3214  - (6150  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 27285){
                rfgain = rfgainmax_x100 + 5286  - (8291  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 29325){
                rfgain = rfgainmax_x100 + 9968  - (12661 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 31875){
                rfgain = rfgainmax_x100 + 19789 - (21231 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else{
                rfgain = rfgainmax_x100 - 6600;
            }
        }
    } else {
        /* NOT DVB-T/T2
        RFGAIN = if(max(AGC,RFAGC) < 0.3){
                     RF_GAIN_MAX - (-16.53)  - (0.86   * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.35){
                     RF_GAIN_MAX - (-31.17)  - (49.07  * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.40){
                     RF_GAIN_MAX - (-36.08)  - (63.11  * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.50){
                     RF_GAIN_MAX - (-44.87)  - (85.60  * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.60){
                     RF_GAIN_MAX - (-38.27)  - (72.04  * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.67){
                     RF_GAIN_MAX - (-37.65)  - (70.79  * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.70){
                     RF_GAIN_MAX - (-32.26)  - (62.72  * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.80){
                     RF_GAIN_MAX - (-37.37)  - (69.83  * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.95){
                     RF_GAIN_MAX - (-40.06)  - (72.86  * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.07){
                     RF_GAIN_MAX - (-19.39)  - (51.28  * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.15){
                     RF_GAIN_MAX - (-99.65)  - (126.50 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.25){
                     RF_GAIN_MAX - (-197.75) - (212.11 * max(AGC,RFAGC))
                 }else{
                     RF_GAIN_MAX - 66.00
                 }
        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
                  RFAGC(V) = RFAGCReg(by X_read_ss) * 1.5 / 255
        So...
        RFGAIN(100xdB) = if(maxagcreg * 150 < 7650){
                             RF_GAIN_MAX * 100  + 1653 - (0.86   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 8925){
                             RF_GAIN_MAX * 100 + 3117  - (49.07  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 10200){
                             RF_GAIN_MAX * 100 + 3608  - (63.11  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 12750){
                             RF_GAIN_MAX * 100 + 4487  - (85.60  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 15300){
                             RF_GAIN_MAX * 100 + 3827  - (72.04  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 17085){
                             RF_GAIN_MAX * 100 + 3765  - (70.79  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 17850){
                             RF_GAIN_MAX * 100 + 3226  - (62.72  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 20400){
                             RF_GAIN_MAX * 100 + 3737  - (69.83  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 25225){
                             RF_GAIN_MAX * 100 + 4006  - (72.86  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 27285){
                             RF_GAIN_MAX * 100 + 1939  - (51.28  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 29325){
                             RF_GAIN_MAX * 100 + 9965  - (126.50 * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 31875){
                             RF_GAIN_MAX * 100 + 19775 - (212.11 * (maxagcreg * 150 / 255))
                         }else{
                             RFGAIN_MAX * 100 - 6600
                         }
        (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
    */
        {
            int32_t maxagcreg_x150 = 0;
            int32_t rfgainmax_x100 = 0;

            if(ifagcreg > rfagcreg){
                maxagcreg_x150 = ifagcreg * 150;
            }else{
                maxagcreg_x150 = rfagcreg * 150;
            }

            if(pTuner->frequencykHz < 42000){
                rfgainmax_x100 = 3371;
            }else if(pTuner->frequencykHz < 71500){
                rfgainmax_x100 = 3416;
            }else if(pTuner->frequencykHz < 85500){
                rfgainmax_x100 = 3527;
            }else if(pTuner->frequencykHz < 135500){
                rfgainmax_x100 = 3663;
            }else if(pTuner->frequencykHz < 163500){
                rfgainmax_x100 = 3654;
            }else if(pTuner->frequencykHz < 198500){
                rfgainmax_x100 = 3755;
            }else if(pTuner->frequencykHz < 250001){
                rfgainmax_x100 = 3894;
            }else if(pTuner->frequencykHz < 305500){
                rfgainmax_x100 = 3061;
            }else if(pTuner->frequencykHz < 353500){
                rfgainmax_x100 = 3188;
            }else if(pTuner->frequencykHz < 457500){
                rfgainmax_x100 = 3247;
            }else if(pTuner->frequencykHz < 609500){
                rfgainmax_x100 = 3352;
            }else if(pTuner->frequencykHz < 769500){
                rfgainmax_x100 = 3457;
            }else if(pTuner->frequencykHz < 841500){
                rfgainmax_x100 = 3512;
            }else if(pTuner->frequencykHz < 890000){
                rfgainmax_x100 = 3704;
            }else if(pTuner->frequencykHz < 906000){
                rfgainmax_x100 = 3479;
            }else if(pTuner->frequencykHz < 930000){
                rfgainmax_x100 = 3379;
            }else if(pTuner->frequencykHz < 946000){
                rfgainmax_x100 = 3263;
            }else if(pTuner->frequencykHz < 986000){
                rfgainmax_x100 = 3109;
            }else{
                rfgainmax_x100 = 2952;
            }

            if(maxagcreg_x150 < 7650){
                rfgain = rfgainmax_x100 + 1653  - (86    * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 8925){
                rfgain = rfgainmax_x100 + 3117  - (4907  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 10200){
                rfgain = rfgainmax_x100 + 3608  - (6311  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 12750){
                rfgain = rfgainmax_x100 + 4487  - (8560  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 15300){
                rfgain = rfgainmax_x100 + 3827  - (7204  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 17085){
                rfgain = rfgainmax_x100 + 3765  - (7079  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 17850){
                rfgain = rfgainmax_x100 + 3226  - (6272  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 20400){
                rfgain = rfgainmax_x100 + 3737  - (6983  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 24225){
                rfgain = rfgainmax_x100 + 4006  - (7286  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 27285){
                rfgain = rfgainmax_x100 + 1939  - (5128  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 29325){
                rfgain = rfgainmax_x100 + 9965  - (12650 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 31875){
                rfgain = rfgainmax_x100 + 19775 - (21211 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else{
                rfgain = rfgainmax_x100 - 6600;
            }
        }
    }

    *pRssi =  -ifgain - rfgain - (sony_Convert2SComplement (terr_compensate_reg, 4) * 100);

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_sat_ReadRssi(sony_freia_t *pTuner, int32_t *pRssi)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t ifagcreg = 0;
    uint8_t rfagcreg = 0;
    uint8_t sat_compensate_reg = 0;
    uint8_t terr_compensate_reg = 0;
    int32_t ifgain = 0;
    int32_t rfgain = 0;

    SONY_TRACE_ENTER("sony_freia_sat_ReadRssi");

    if((!pTuner) || (!pTuner->pI2c) || (!pRssi)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Satellite only */
    if(pTuner->state != SONY_FREIA_STATE_ACTIVE_S){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = X_read_agc(pTuner, &sat_compensate_reg, &terr_compensate_reg, &ifagcreg, &rfagcreg);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /*
        - Low band
        IFGAIN = if(AGC < 0.35){
                     45.34 - 76.31 * AGC
                 }else if(AGC < 0.4){
                     46.20 - 79.03 * AGC
                 }else{
                     14.61
                 }
        - High band and High gain Mode
        IFGAIN = if(AGC < 0.33){
                     45.34 - 76.24 * AGC
                 }else if(AGC < 0.38){
                     46.34 - 79.40 * AGC
                 }else{
                     16.17
                 }
        - High band and low gain Mode
        IFGAIN = if(AGC > 0.27){
                     45.35 - 76.43 * AGC
                 }else if(AGC > 0.32){
                     46.34 - 79.40 * AGC
                 }else{
                     20.93
                 }

        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
        So...

        - Low band
        IFGAIN(100xdB) = if(IFAGCReg * 150 < 8925){
                             4534 - (7631 * (IFAGCReg * 150)) / 2550
                         }else if(IFAGCReg * 150 < 10200){
                             4620 - (7903 * (IFAGCReg * 150)) / 2550
                         }else{
                             1461
                         }
        - High band and High gain Mode
        IFGAIN(100xdB) = if(IFAGCReg * 150 < 8415){
                             4534 - (7624 * (IFAGCReg * 150)) / 2550
                         }else if(IFAGCReg * 150 < 9690){
                             4634 - (7940 * (IFAGCReg * 150)) / 2550
                         }else{
                             1617
                         }
        - High band and low gain Mode
        IFGAIN(100xdB) = if(IFAGCReg * 150 < 6885){
                             4535 - (7634 * (IFAGCReg * 150)) / 2550
                         }else if(IFAGCReg * 150 < 8160){
                             4634 - (7940 * (IFAGCReg * 150)) / 2550
                         }else{
                             2093
                         }
    */
    {
        int32_t agcreg_x150 = ifagcreg * 150;
        if (pTuner->frequencykHz <= 2150000) {
            /* Low band */
            if(agcreg_x150 < 8925){
                ifgain = 4534 - ((7631 * agcreg_x150 + 12750) / 25500); /* Round */
            }else if(agcreg_x150 < 10200){
                ifgain = 4620 - ((7903 * agcreg_x150 + 12750) / 25500); /* Round */
            }else{
                ifgain = 1461;
            }
        } else if ((pTuner->frequencykHz > 2150000) && !(pTuner->flags & SONY_FREIA_CONFIG_SAT_LOW_GAIN_MODE)) {
            /* High band and High gain mode */
            if(agcreg_x150 < 8415){
                ifgain = 4534 - ((7624 * agcreg_x150 + 12750) / 25500); /* Round */
            }else if(agcreg_x150 < 9690){
                ifgain = 4634 - ((7940 * agcreg_x150 + 12750) / 25500); /* Round */
            }else{
                ifgain = 1617;
            }
        } else {
            /* High band Low gain mode */
            if(agcreg_x150 < 6885){
                ifgain = 4535 - ((7643 * agcreg_x150 + 12750) / 25500); /* Round */
            }else if(agcreg_x150 < 8160){
                ifgain = 4634 - ((7940 * agcreg_x150 + 12750) / 25500); /* Round */
            }else{
                ifgain = 2093;
            }
        }

    }

    if (pTuner->frequencykHz <= 2150000) {
        int32_t maxagcreg_x150 = 0;
        int32_t rfgainmax_x100 = 0;

        maxagcreg_x150 = ifagcreg * 150;

        if (!(pTuner->flags & SONY_FREIA_CONFIG_SAT_LOW_GAIN_MODE)) {
            /* Low band and High gain mode */
            /*
                RFGAIN = if(AGC < 0.1){
                             RF_GAIN_MAX - (-19.36) - (-19.16 * AGC)
                         }else if(AGC < 0.3){
                             RF_GAIN_MAX - (-20.66) - (-1.68 * AGC)
                         }else if(AGC < 0.35){
                             RF_GAIN_MAX - (-22.79) - (4.91 * AGC)
                         }else if(AGC < 0.4){
                             RF_GAIN_MAX - (-46.49) - (71.95 * AGC)
                         }else if(AGC < 0.45){
                             RF_GAIN_MAX - (-70.68) - (133.72 * AGC)
                         }else if(AGC < 0.5){
                             RF_GAIN_MAX - (-65.04) - (120.64 * AGC)
                         }else if(AGC < 0.6){
                             RF_GAIN_MAX - (-72.18) - (134.89 * AGC)
                         }else if(AGC < 0.7){
                             RF_GAIN_MAX - (-50.26) - (97.89 * AGC)
                         }else if(AGC < 0.8){
                             RF_GAIN_MAX - (-69.68) - (125.62 * AGC)
                         }else if(AGC < 0.85){
                             RF_GAIN_MAX - (-75.68) - (132.63 * AGC)
                         }else if(AGC < 0.9){
                             RF_GAIN_MAX - (-38.77) - (89.09 * AGC)
                         }else if(AGC < 1.0){
                             RF_GAIN_MAX - (-53.46) - (105.28 * AGC)
                         }else if(AGC < 1.1){
                             RF_GAIN_MAX - 26.56    - (25.74  * AGC)
                         }else{
                             RF_GAIN_MAX - 54.37    - (0.03  * AGC)
                        }
                Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
                          RFAGC(V) = RFAGCReg(by X_read_ss) * 1.5 / 255
                So...
                RFGAIN(100xdB) = if(maxagcreg * 150 < 2550){
                                     RFGAIN_MAX * 100 + 1936 - (-1916 * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 7650){
                                     RFGAIN_MAX * 100 + 2066 - (-168  * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 8925){
                                     RFGAIN_MAX * 100 + 2279 - (491   * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 10200){
                                     RFGAIN_MAX * 100 + 4649 - (7195  * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 11475){
                                     RFGAIN_MAX * 100 + 7068 - (13372 * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 12750){
                                     RFGAIN_MAX * 100 + 6504 - (12064 * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 15300){
                                     RFGAIN_MAX * 100 + 6926 - (13489 * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 17850){
                                     RFGAIN_MAX * 100 + 5015 - (9789  * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 20400){
                                     RFGAIN_MAX * 100 + 7318 - (12562 * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 21675){
                                     RFGAIN_MAX * 100 + 8663 - (13263 * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 22950){
                                     RFGAIN_MAX * 100 + 4227 - (8909  * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 25500){
                                     RFGAIN_MAX * 100 + 5821 - (10528 * (maxagcreg * 150 / 25500))
                                 }else if(maxagcreg * 150 < 28050){
                                     RFGAIN_MAX * 100 - 3041 - (2574  * (maxagcreg * 150 / 25500))
                                 }else{
                                     RFGAIN_MAX * 100 - 5437 - (3     * (maxagcreg * 150 / 25500))
                                 }
                (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
            */

            if(pTuner->frequencykHz < 1000000){
                rfgainmax_x100 = 3071;
            }else if(pTuner->frequencykHz < 1070000){
                rfgainmax_x100 = 3166;
            }else if(pTuner->frequencykHz < 1150000){
                rfgainmax_x100 = 3239;
            }else if(pTuner->frequencykHz < 1220000){
                rfgainmax_x100 = 3296;
            }else if(pTuner->frequencykHz < 1330000){
                rfgainmax_x100 = 3287;
            }else if(pTuner->frequencykHz < 1720000){
                rfgainmax_x100 = 3302;
            }else{
                rfgainmax_x100 = 3437;
            }

            if(maxagcreg_x150 < 2550){
                rfgain = rfgainmax_x100 + 1936 - (-1916 * maxagcreg_x150 - 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 7650){
                rfgain = rfgainmax_x100 + 2066 - (-168  * maxagcreg_x150 - 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 8925){
                rfgain = rfgainmax_x100 + 2279 - (491   * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 10200){
                rfgain = rfgainmax_x100 + 4649 - (7195  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 11475){
                rfgain = rfgainmax_x100 + 7068 - (13372 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 12750){
                rfgain = rfgainmax_x100 + 6504 - (12064 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 15300){
                rfgain = rfgainmax_x100 + 7218 - (13489 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 17850){
                rfgain = rfgainmax_x100 + 5026 - (9789  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 20400){
                rfgain = rfgainmax_x100 + 6968 - (12562 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 21675){
                rfgain = rfgainmax_x100 + 7568 - (13263 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 22950){
                rfgain = rfgainmax_x100 + 3877 - (8909  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 25500){
                rfgain = rfgainmax_x100 + 5346 - (10528 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 28050){
                rfgain = rfgainmax_x100 - 2656 - (2574  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else{
                rfgain = rfgainmax_x100 - 5437 - (3     * maxagcreg_x150 + 12750) / 25500; /* Round */;
            }
        } else {
            /* Low band and Low gain mode */
            /*
                RFGAIN = if(AGC < 0.1){
                             RF_GAIN_MAX - (-19.49) - (-7.55 * AGC)
                         }else if(AGC < 0.3){
                             RF_GAIN_MAX - (-20.00) - (-0.55 * AGC)
                         }else if(AGC < 0.35){
                             RF_GAIN_MAX - (-25.68) - (17.61 * AGC)
                         }else if(AGC < 0.41){
                             RF_GAIN_MAX - (-40.88) - (61.95 * AGC)
                         }else if(AGC < 0.45){
                             RF_GAIN_MAX - (-33.27) - (43.29 * AGC)
                         }else if(AGC < 0.5){
                             RF_GAIN_MAX - (-47.41) - (74.94 * AGC)
                         }else if(AGC < 0.55){
                             RF_GAIN_MAX - (-70.30) - (120.67 * AGC)
                         }else if(AGC < 0.6){
                             RF_GAIN_MAX - (-65.29) - (112.38 * AGC)
                         }else if(AGC < 0.72){
                             RF_GAIN_MAX - (-61.45) - (105.01 * AGC)
                         }else if(AGC < 0.75){
                             RF_GAIN_MAX - (-88.62) - (144.20 * AGC)
                         }else if(AGC < 0.8){
                             RF_GAIN_MAX - (-75.42) - (126.56 * AGC)
                         }else if(AGC < 0.85){
                             RF_GAIN_MAX - (-62.76) - (111.22 * AGC)
                         }else if(AGC < 0.9){
                             RF_GAIN_MAX - (-41.02) - (85.27 * AGC)
                         }else if(AGC < 1.0){
                             RF_GAIN_MAX - (-55.34) - (101.22 * AGC)
                         }else if(AGC < 1.1){
                             RF_GAIN_MAX - 33.62 - (11.86 * AGC)
                         }else{
                             RF_GAIN_MAX - 46.42
                        }
                Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
                          RFAGC(V) = RFAGCReg(by X_read_ss) * 1.5 / 255
                So...
                RFGAIN(100xdB) = if(maxagcreg * 150 < 2550){
                                     RFGAIN_MAX * 100 + 1949 - (-7.55  * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 7650){
                                     RFGAIN_MAX * 100 + 2000 - (-0.55  * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 8925){
                                     RFGAIN_MAX * 100 + 2568 - (17.61  * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 10455){
                                     RFGAIN_MAX * 100 + 4088 - (61.95  * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 11475){
                                     RFGAIN_MAX * 100 + 3327 - (43.29  * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 12750){
                                     RFGAIN_MAX * 100 + 4741 - (74.94  * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 14025){
                                     RFGAIN_MAX * 100 + 7030 - (120.67 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 15300){
                                     RFGAIN_MAX * 100 + 6529 - (112.38 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 18360){
                                     RFGAIN_MAX * 100 + 6145 - (105.01 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 19125){
                                     RFGAIN_MAX * 100 + 8862 - (144.20 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 20400){
                                     RFGAIN_MAX * 100 + 7542 - (126.56 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 21675){
                                     RFGAIN_MAX * 100 + 6276 - (111.22 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 22950){
                                     RFGAIN_MAX * 100 + 4102 - (85.27  * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 25500){
                                     RFGAIN_MAX * 100 + 5534 - (101.22 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 28050){
                                     RFGAIN_MAX * 100 - 3362 - (11.86  * (maxagcreg * 150 / 255))
                                 }else{
                                     RFGAIN_MAX * 100 - 4642
                                 }
                (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
            */

            if(pTuner->frequencykHz < 1000000){
                rfgainmax_x100 = 2326;
            }else if(pTuner->frequencykHz < 1070000){
                rfgainmax_x100 = 2411;
            }else if(pTuner->frequencykHz < 1150000){
                rfgainmax_x100 = 2485;
            }else if(pTuner->frequencykHz < 1220000){
                rfgainmax_x100 = 2553;
            }else if(pTuner->frequencykHz < 1330000){
                rfgainmax_x100 = 2510;
            }else if(pTuner->frequencykHz < 1720000){
                rfgainmax_x100 = 2498;
            }else{
                rfgainmax_x100 = 2474;
            }

            if(maxagcreg_x150 < 2550){
                rfgain = rfgainmax_x100 + 1949 - (-755  * maxagcreg_x150 - 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 7650){
                rfgain = rfgainmax_x100 + 2000 - (-55   * maxagcreg_x150 - 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 8925){
                rfgain = rfgainmax_x100 + 2568 - (1761  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 10455){
                rfgain = rfgainmax_x100 + 4088 - (6195  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 11475){
                rfgain = rfgainmax_x100 + 3327 - (4329  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 12750){
                rfgain = rfgainmax_x100 + 4741 - (7494  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 14025){
                rfgain = rfgainmax_x100 + 7030 - (12067 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 15300){
                rfgain = rfgainmax_x100 + 6529 - (11238 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 18360){
                rfgain = rfgainmax_x100 + 6145 - (10501 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 19125){
                rfgain = rfgainmax_x100 + 8862 - (14420 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 20400){
                rfgain = rfgainmax_x100 + 7542 - (12656 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 21675){
                rfgain = rfgainmax_x100 + 6276 - (11122 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 22950){
                rfgain = rfgainmax_x100 + 4102 - (8527  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 25500){
                rfgain = rfgainmax_x100 + 5534 - (10122 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 28050){
                rfgain = rfgainmax_x100 - 3362 - (1186  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else{
                rfgain = rfgainmax_x100 - 4642;
            }
        }
    } else {
        int32_t maxagcreg_x150 = 0;
        int32_t rfgainmax_x100 = 0;

        maxagcreg_x150 = ifagcreg * 150;

        if (!(pTuner->flags & SONY_FREIA_CONFIG_SAT_LOW_GAIN_MODE)) {
            /*
                High band and High gain mode
                RFGAIN = if(AGC < 0.1){
                             RF_GAIN_MAX - (-19.49) - (-35.77  * AGC)
                         }else if(AGC < 0.3){
                             RF_GAIN_MAX - (-22.42) - (-1.83   * AGC)
                         }else if(AGC < 0.35){
                             RF_GAIN_MAX - (-32.72) - (30.18  * AGC)
                         }else if(AGC < 0.40){
                             RF_GAIN_MAX - (-67.69) - (130.39 * AGC)
                         }else if(AGC < 0.45){
                             RF_GAIN_MAX - (-65.85) - (126.33 * AGC)
                         }else if(AGC < 0.5){
                             RF_GAIN_MAX - (-66.55) - (128.02 * AGC)
                         }else if(AGC < 0.6){
                             RF_GAIN_MAX - (-69.03) - (133.26 * AGC)
                         }else if(AGC < 0.7){
                             RF_GAIN_MAX - (-54.79) - (108.03 * AGC)
                         }else if(AGC < 0.8){
                             RF_GAIN_MAX - (-59.62) - (115.45 * AGC)
                         }else if(AGC < 0.9){
                             RF_GAIN_MAX - (-41.69) - (93.71  * AGC)
                         }else if(AGC < 1.0){
                             RF_GAIN_MAX - (-66.01) - (120.5  * AGC)
                         }else if(AGC < 1.1){
                             RF_GAIN_MAX -  35.81   - (18.97  * AGC)
                         }else{
                             RF_GAIN_MAX -  56.30   - (0.02   * AGC)
                        }
                Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
                          RFAGC(V) = RFAGCReg(by X_read_ss) * 1.5 / 255
                So...
                RFGAIN(100xdB) = if(maxagcreg * 150 < 2550){
                                     RFGAIN_MAX * 100 + 1949 - (-35.37 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 7650){
                                     RFGAIN_MAX * 100 + 2242 - (-1.83  * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 8925){
                                     RFGAIN_MAX * 100 + 3272 - (30.18  * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 10200){
                                     RFGAIN_MAX * 100 + 6769 - (130.39 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 11475){
                                     RFGAIN_MAX * 100 + 6585 - (126.33 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 12750){
                                     RFGAIN_MAX * 100 + 6655 - (128.02 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 15300){
                                     RFGAIN_MAX * 100 + 6903 - (133.26 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 17850){
                                     RFGAIN_MAX * 100 + 5479 - (108.03 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 20400){
                                     RFGAIN_MAX * 100 + 5962 - (115.45 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 22950){
                                     RFGAIN_MAX * 100 + 4169 - (93.71  * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 25500){
                                     RFGAIN_MAX * 100 + 6601 - (120.50 * (maxagcreg * 150 / 255))
                                 }else if(maxagcreg * 150 < 28050){
                                     RFGAIN_MAX * 100 - 3581 - (18.97  * (maxagcreg * 150 / 255))
                                 }else{
                                     RFGAIN_MAX * 100 - 5630 - (0.02   * (maxagcreg * 150 / 255))
                                 }
                (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
            */
            if(pTuner->frequencykHz < 2500000){
                rfgainmax_x100 = 3415;
            }else if(pTuner->frequencykHz < 2880000){
                rfgainmax_x100 = 3376;
            }else if(pTuner->frequencykHz < 3130000){
                rfgainmax_x100 = 3357;
            }else{
                rfgainmax_x100 = 3357;
            }

            if(maxagcreg_x150 < 2550){
                rfgain = rfgainmax_x100 + 1949 - (-3537 * maxagcreg_x150 - 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 7650){
                rfgain = rfgainmax_x100 + 2242 - (-183  * maxagcreg_x150 - 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 8925){
                rfgain = rfgainmax_x100 + 3272 - (3018  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 10200){
                rfgain = rfgainmax_x100 + 6769 - (13039 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 11475){
                rfgain = rfgainmax_x100 + 6585 - (12633 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 12750){
                rfgain = rfgainmax_x100 + 6655 - (12802 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 15300){
                rfgain = rfgainmax_x100 + 6903 - (13326 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 17850){
                rfgain = rfgainmax_x100 + 5479 - (10803 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 20400){
                rfgain = rfgainmax_x100 + 5962 - (11545 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 22950){
                rfgain = rfgainmax_x100 + 4169 - (9371  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 25500){
                rfgain = rfgainmax_x100 + 6601 - (12050 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 28050){
                rfgain = rfgainmax_x100 - 3581 - (1897  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else{
                rfgain = rfgainmax_x100 - 5630 - (2     * maxagcreg_x150 + 12750) / 25500; /* Round */;
            }
        } else {
    /*
        High band and Low gain mode
        RFGAIN = if(AGC < 0.23){
                     RF_GAIN_MAX - (-19.63) - (-2.14  * AGC)
                 }else if(AGC < 0.33){
                     RF_GAIN_MAX - (-29.90) - (41.00  * AGC)
                 }else if(AGC < 0.40){
                     RF_GAIN_MAX - (-33.65) - (52.51  * AGC)
                 }else if(AGC < 0.45){
                     RF_GAIN_MAX - (-52.15) - (98.72  * AGC)
                 }else if(AGC < 0.50){
                     RF_GAIN_MAX - (-70.09) - (138.55 * AGC)
                 }else if(AGC < 0.55){
                     RF_GAIN_MAX - (-45.64) - (89.51  * AGC)
                 }else if(AGC < 0.60){
                     RF_GAIN_MAX - (-56.35) - (109.08 * AGC)
                 }else if(AGC < 0.72){
                     RF_GAIN_MAX - (-60.54) - (116.23 * AGC)
                 }else if(AGC < 0.75){
                     RF_GAIN_MAX - (-59.92) - (115.50 * AGC)
                 }else if(AGC < 0.80){
                     RF_GAIN_MAX - (-37.81) - (86.08  * AGC)
                 }else if(AGC < 0.85){
                     RF_GAIN_MAX - (-50.62) - (101.89 * AGC)
                 }else if(AGC < 0.92){
                     RF_GAIN_MAX - (-54.93) - (106.49 * AGC)
                 }else if(AGC < 1.00){
                     RF_GAIN_MAX -  23.49   - (21.35  * AGC)
                 }else if(AGC < 1.10){
                     RF_GAIN_MAX -  43.73   - (0.87   * AGC)
                 }else{
                     RF_GAIN_MAX -  44.67
                }
        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
                  RFAGC(V) = RFAGCReg(by X_read_ss) * 1.5 / 255
        So...
        RFGAIN(100xdB) = if(maxagcreg * 150 < 5865){
                             RFGAIN_MAX * 100 + 1963   - (-2.14  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 8415){
                             RFGAIN_MAX * 100 + 2990 - (41.00  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 10200){
                             RFGAIN_MAX * 100 + 3365 - (52.51  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 11475){
                             RFGAIN_MAX * 100 + 5215 - (98.72  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 12750){
                             RFGAIN_MAX * 100 + 7009 - (138.55 * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 14025){
                             RFGAIN_MAX * 100 + 4564 - (89.51  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 15300){
                             RFGAIN_MAX * 100 + 5635 - (109.08 * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 18360){
                             RFGAIN_MAX * 100 + 6054 - (116.23 * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 19125){
                             RFGAIN_MAX * 100 + 5992 - (115.50 * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 20400){
                             RFGAIN_MAX * 100 + 3781 - (86.08  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 21675){
                             RFGAIN_MAX * 100 + 5062 - (101.89 * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 23460){
                             RFGAIN_MAX * 100 + 5493 - (106.49 * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 25500){
                             RFGAIN_MAX * 100 - 2349 - (21.35  * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 28050){
                             RFGAIN_MAX * 100 - 4373 - (0.87   * (maxagcreg * 150 / 255))
                         }else{
                             RFGAIN_MAX * 100 - 4467
                         }
        (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
    */
            if(pTuner->frequencykHz < 2500000){
                rfgainmax_x100 = 2433;
            }else if(pTuner->frequencykHz < 2880000){
                rfgainmax_x100 = 2373;
            }else if(pTuner->frequencykHz < 3130000){
                rfgainmax_x100 = 2322;
            }else{
                rfgainmax_x100 = 2256;
            }

            if(maxagcreg_x150 < 5865){
                rfgain = rfgainmax_x100 + 1963 - (-214  * maxagcreg_x150 - 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 8415){
                rfgain = rfgainmax_x100 + 2990 - (4100  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 10200){
                rfgain = rfgainmax_x100 + 3365 - (5251  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 11475){
                rfgain = rfgainmax_x100 + 5215 - (9872  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 12750){
                rfgain = rfgainmax_x100 + 7009 - (13855 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 14025){
                rfgain = rfgainmax_x100 + 4564 - (8951  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 15300){
                rfgain = rfgainmax_x100 + 5635 - (10908 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 18360){
                rfgain = rfgainmax_x100 + 6054 - (11623 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 19125){
                rfgain = rfgainmax_x100 + 5992 - (11550 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 20400){
                rfgain = rfgainmax_x100 + 3781 - (8608  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 21675){
                rfgain = rfgainmax_x100 + 5062 - (10189 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 23460){
                rfgain = rfgainmax_x100 + 5493 - (10649 * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 25500){
                rfgain = rfgainmax_x100 - 2349 - (2135  * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else if(maxagcreg_x150 < 28050){
                rfgain = rfgainmax_x100 - 4373 - (87    * maxagcreg_x150 + 12750) / 25500; /* Round */
            }else{
                rfgain = rfgainmax_x100 - 4467;
            }
        }
    }


    *pRssi = - ifgain - rfgain - (sony_Convert2SComplement (sat_compensate_reg, 4) * 100);

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_terr_IQ_ReadRssi(sony_freia_t *pTuner, int32_t *pRssi)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t ifagcreg = 0;
    uint8_t rfagcreg = 0;
    uint8_t sat_compensate_reg = 0;
    uint8_t terr_compensate_reg = 0;
    int32_t ifgain = 0;
    int32_t rfgain = 0;

    SONY_TRACE_ENTER("sony_freia_terr_IQ_ReadRssi");

    if((!pTuner) || (!pTuner->pI2c) || (!pRssi)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* IQ Mode only */
    if(pTuner->state != SONY_FREIA_STATE_ACTIVE_IQ){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = X_read_agc(pTuner, &sat_compensate_reg, &terr_compensate_reg, &ifagcreg, &rfagcreg);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /*
        IFGAIN = if(AGC < 0.35){
                     45.30 - 75.96 * AGC
                 }else{
                     19.34
                 }
        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
        So...
        IFGAIN(100xdB) = if(IFAGCReg * 150 > 8925){
                             4530 - (7596 * (IFAGCReg * 150)) / 25500
                         }else{
                             1934
                         }
    */
    {
        int32_t agcreg_x150 = ifagcreg * 150;

        if(agcreg_x150 < 8925){
            ifgain = 4530 - ((7596 * agcreg_x150 + 12750) / 25500); /* Round */
        }else{
            ifgain = 1934;
        }
    }

    if(!(pTuner->isQDumpOn)){
        int32_t maxagcreg_x150 = 0;
        int32_t rfgainmax_x100 = 0;

        if(ifagcreg > rfagcreg){
            maxagcreg_x150 = ifagcreg * 150;
        }else{
            maxagcreg_x150 = rfagcreg * 150;
        }

		/*
        RFGAIN = if(max(AGC,RFAGC) < 0.3){
                     RF_GAIN_MAX - (-17.74) - (-3.06 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.35){
                     RF_GAIN_MAX - (-33.26) - (48.86 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.40){
                     RF_GAIN_MAX - (-35.33) - (55.30 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.50){
                     RF_GAIN_MAX - (-46.58) - (83.51 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.60){
                     RF_GAIN_MAX - (-42.14) - (75.03 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.67){
                     RF_GAIN_MAX - (-39.33) - (70.04 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.70){
                     RF_GAIN_MAX - (-31.06) - (57.73 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.80){
                     RF_GAIN_MAX - (-39.04) - (69.14 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.95){
                     RF_GAIN_MAX - (-44.34) - (75.34 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.07){
                     RF_GAIN_MAX - (-22.72) - (52.81 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.15){
                     RF_GAIN_MAX - (-106.84) - (131.52 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.25){
                     RF_GAIN_MAX - (-211.80) - (223.09 * max(AGC,RFAGC))
                 }else{
                     RF_GAIN_MAX - 66.00
                 }
        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
                  RFAGC(V) = RFAGCReg(by X_read_ss) * 1.5 / 255
        So...
        RFGAIN(100xdB) = if(maxagcreg * 150 < 7650){
                             RF_GAIN_MAX * 100 + 1774 - (-3.06   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 8925){
                             RF_GAIN_MAX * 100 + 3326 - (48.86   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 10200){
                             RF_GAIN_MAX * 100 + 3533 - (55.30   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 12750){
                             RF_GAIN_MAX * 100 + 4658 - (83.51   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 15300){
                             RF_GAIN_MAX * 100 + 4214 - (75.03   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 17085){
                             RF_GAIN_MAX * 100 + 3933 - (70.04   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 17850){
                             RF_GAIN_MAX * 100 + 3106 - (57.73   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 20400){
                             RF_GAIN_MAX * 100 + 3904 - (69.14   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 24225){
                             RF_GAIN_MAX * 100 + 4434 - (75.34   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 27285){
                             RF_GAIN_MAX * 100 + 2272 - (52.81   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 29325){
                             RF_GAIN_MAX * 100 + 10684 - (131.52 * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 31875){
                             RF_GAIN_MAX * 100 + 21180 - (223.09 * (maxagcreg * 150 / 255))
                         }else{
                             RFGAIN_MAX * 100 - 6700
                         }
        (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
    */
        if(pTuner->frequencykHz < 42000){
            rfgainmax_x100 = 2885;
        }else if(pTuner->frequencykHz < 48000){
            rfgainmax_x100 = 2985;
        }else if(pTuner->frequencykHz < 50500){
            rfgainmax_x100 = 3038;
        }else if(pTuner->frequencykHz < 57500){
            rfgainmax_x100 = 3195;
        }else if(pTuner->frequencykHz < 78500){
            rfgainmax_x100 = 3363;
        }else if(pTuner->frequencykHz < 114500){
            rfgainmax_x100 = 3207;
        }else if(pTuner->frequencykHz < 142500){
            rfgainmax_x100 = 3240;
        }else if(pTuner->frequencykHz < 163500){
            rfgainmax_x100 = 3400;
        }else if(pTuner->frequencykHz < 198500){
            rfgainmax_x100 = 3536;
        }else if(pTuner->frequencykHz < 250001){
            rfgainmax_x100 = 3658;
        }else if(pTuner->frequencykHz < 305500){
            rfgainmax_x100 = 3073;
        }else if(pTuner->frequencykHz < 353500){
            rfgainmax_x100 = 3154;
        }else if(pTuner->frequencykHz < 481500){
            rfgainmax_x100 = 3250;
        }else if(pTuner->frequencykHz < 601500){
            rfgainmax_x100 = 3343;
        }else if(pTuner->frequencykHz < 874000){
            rfgainmax_x100 = 3541;
        }else if(pTuner->frequencykHz < 906000){
            rfgainmax_x100 = 3447;
        }else if(pTuner->frequencykHz < 938000){
            rfgainmax_x100 = 3293;
        }else if(pTuner->frequencykHz < 978000){
            rfgainmax_x100 = 3100;
		}else{
            rfgainmax_x100 = 2926;
        }

        if(maxagcreg_x150 < 7650){
            rfgain = rfgainmax_x100 + 1774 - (-306  * maxagcreg_x150 - 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 8925){
            rfgain = rfgainmax_x100 + 3326 - (4886   * maxagcreg_x150 - 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 10200){
            rfgain = rfgainmax_x100 + 3533 - (5530  * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 12750){
            rfgain = rfgainmax_x100 + 4658 - (8351  * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 15300){
            rfgain = rfgainmax_x100 + 4214 - (7503  * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 17085){
            rfgain = rfgainmax_x100 + 3933 - (7004  * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 17850){
            rfgain = rfgainmax_x100 + 3106 - (5773 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 20400){
            rfgain = rfgainmax_x100 + 3904 - (6914 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 24225){
            rfgain = rfgainmax_x100 + 4434 - (7534 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 27285){
            rfgain = rfgainmax_x100 + 2272 - (5281 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 29325){
            rfgain = rfgainmax_x100 + 10684 - (13152 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 31875){
            rfgain = rfgainmax_x100 + 21180 - (22309 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else {
            rfgain = rfgainmax_x100 - 6600;
		}
    }else{
        int32_t maxagcreg_x150 = 0;
        int32_t rfgainmax_x100 = 0;

        if(ifagcreg > rfagcreg){
            maxagcreg_x150 = ifagcreg * 150;
        }else{
            maxagcreg_x150 = rfagcreg * 150;
        }
    /*
        RFGAIN = if(max(AGC,RFAGC) < 0.3){
                     RF_GAIN_MAX - (-17.19) - (-1.21 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.35){
                     RF_GAIN_MAX - (-32.80) - (50.51 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.40){
                     RF_GAIN_MAX - (-35.16) - (57.73 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.50){
                     RF_GAIN_MAX - (-47.82) - (89.43 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.60){
                     RF_GAIN_MAX - (-41.32) - (76.94 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.67){
                     RF_GAIN_MAX - (-37.51) - (70.21 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.70){
                     RF_GAIN_MAX - (-29.26) - (57.93 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.80){
                     RF_GAIN_MAX - (-37.24) - (69.35 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 0.95){
                     RF_GAIN_MAX - (-42.86) - (75.92 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.07){
                     RF_GAIN_MAX - (-12.64) - (44.61 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.15){
                     RF_GAIN_MAX - (-108.78) - (134.05 * max(AGC,RFAGC))
                 }else if(max(AGC,RFAGC) < 1.25){
                     RF_GAIN_MAX - (-217.68) - (229.66 * max(AGC,RFAGC))
                 }else{
                     RF_GAIN_MAX - 67.00
                 }
        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.5 / 255
                  RFAGC(V) = RFAGCReg(by X_read_ss) * 1.5 / 255
        So...
        RFGAIN(100xdB) = if(maxagcreg * 150 < 7650){
                             RF_GAIN_MAX * 100 + 1719 - (-1.21   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 8925){
                             RF_GAIN_MAX * 100 + 3280 - (50.51   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 10200){
                             RF_GAIN_MAX * 100 + 3516 - (57.73   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 12750){
                             RF_GAIN_MAX * 100 + 4782 - (89.43   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 15300){
                             RF_GAIN_MAX * 100 + 4132 - (76.94   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 17085){
                             RF_GAIN_MAX * 100 + 3751 - (70.21   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 17850){
                             RF_GAIN_MAX * 100 + 2926 - (57.93   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 20400){
                             RF_GAIN_MAX * 100 + 3724 - (69.35   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 24225){
                             RF_GAIN_MAX * 100 + 4286 - (75.92   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 27285){
                             RF_GAIN_MAX * 100 + 1264 - (44.61   * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 29325){
                             RF_GAIN_MAX * 100 + 10878 - (134.05 * (maxagcreg * 150 / 255))
                         }else if(maxagcreg * 150 < 31875){
                             RF_GAIN_MAX * 100 + 21768 - (229.66 * (maxagcreg * 150 / 255))
                         }else{
                             RFGAIN_MAX * 100 - 6700
                         }
        (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
    */
        if(pTuner->frequencykHz < 42000){
            rfgainmax_x100 = 2317;
        }else if(pTuner->frequencykHz < 48000){
            rfgainmax_x100 = 2317;
        }else if(pTuner->frequencykHz < 50500){
            rfgainmax_x100 = 2407;
        }else if(pTuner->frequencykHz < 57500){
            rfgainmax_x100 = 2427;
        }else if(pTuner->frequencykHz < 78500){
            rfgainmax_x100 = 2473;
        }else if(pTuner->frequencykHz < 114500){
            rfgainmax_x100 = 2473;
        }else if(pTuner->frequencykHz < 142500){
            rfgainmax_x100 = 2470;
        }else if(pTuner->frequencykHz < 163500){
            rfgainmax_x100 = 2451;
        }else if(pTuner->frequencykHz < 198500){
            rfgainmax_x100 = 2449;
        }else if(pTuner->frequencykHz < 250001){
            rfgainmax_x100 = 2370;
        }else if(pTuner->frequencykHz < 305500){
            rfgainmax_x100 = 2377;
        }else if(pTuner->frequencykHz < 353500){
            rfgainmax_x100 = 2403;
        }else if(pTuner->frequencykHz < 481500){
            rfgainmax_x100 = 2410;
        }else if(pTuner->frequencykHz < 601500){
            rfgainmax_x100 = 2421;
        }else if(pTuner->frequencykHz < 874000){
            rfgainmax_x100 = 2452;
        }else if(pTuner->frequencykHz < 906000){
            rfgainmax_x100 = 2419;
        }else if(pTuner->frequencykHz < 938000){
            rfgainmax_x100 = 2382;
        }else if(pTuner->frequencykHz < 978000){
            rfgainmax_x100 = 2330;
        }else{
            rfgainmax_x100 = 2267;
        }

        if(maxagcreg_x150 < 7650){
             rfgain = rfgainmax_x100 + 1719 - (-121  * maxagcreg_x150 - 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 8925){
            rfgain = rfgainmax_x100 + 3280 - (5051   * maxagcreg_x150 - 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 10200){
            rfgain = rfgainmax_x100 + 3516 - (5773  * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 12750){
            rfgain = rfgainmax_x100 + 4782 - (8943  * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 15300){
            rfgain = rfgainmax_x100 + 4132 - (7694  * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 17085){
            rfgain = rfgainmax_x100 + 3751 - (7021  * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 17850){
            rfgain = rfgainmax_x100 + 2926 - (5793 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 20400){
            rfgain = rfgainmax_x100 + 3724 - (6935 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 24225){
            rfgain = rfgainmax_x100 + 4286 - (7592 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 27285){
            rfgain = rfgainmax_x100 + 1264 - (4461 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 29325){
            rfgain = rfgainmax_x100 + 10878 - (13405 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else if(maxagcreg_x150 < 31875){
            rfgain = rfgainmax_x100 + 21768 - (22966 * maxagcreg_x150 + 12750) / 25500; /* Round */
        }else{
            rfgain = rfgainmax_x100 - 6700;
		}
    }

    *pRssi = - ifgain - rfgain - (sony_Convert2SComplement (terr_compensate_reg, 4) * 100);

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_freia_RFFilterConfig(sony_freia_t *pTuner, uint8_t vlCoeff, uint8_t vlOffset,
       uint8_t uvhCoeff, uint8_t uvhOffset)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_freia_RFFilterConfig");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if((pTuner->state != SONY_FREIA_STATE_SLEEP) && (pTuner->state != SONY_FREIA_STATE_ACTIVE_T)
        && (pTuner->state != SONY_FREIA_STATE_ACTIVE_S) && (pTuner->state != SONY_FREIA_STATE_ACTIVE_IQ)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    if(pTuner->state == SONY_FREIA_STATE_SLEEP){
        if((pTuner->flags & SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_FREIA_CONFIG_EXT_REF)){
            /* Enable Xtal */
            result = X_oscen(pTuner);
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        }
    }

    /* Clock enable for internal logic block, CPU wake-up (0x87, 0x88) */
    {
        const uint8_t cdata[2] = {0x84, 0x40};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    {
        uint8_t data[3];

        /* Write multiplier for VHF-Low */
        data[0] = vlCoeff;
        data[1] = 0x49;
        data[2] = 0x03;
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x16, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        SONY_SLEEP(1);

        /* Write offset for VHF-Low */
        data[0] = vlOffset;
        data[1] = 0x4B;
        data[2] = 0x03;
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x16, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        SONY_SLEEP(1);
        /* Write multiplier for UHF/VHF-High */
        data[0] = uvhCoeff;
        data[1] = 0x41;
        data[2] = 0x03;
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x16, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        SONY_SLEEP(1);

        /* Write offset for UHF/VHF-High */
        data[0] = uvhOffset;
        data[1] = 0x43;
        data[2] = 0x03;
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x16, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        SONY_SLEEP(1);
    }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0x80);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    if(pTuner->state == SONY_FREIA_STATE_SLEEP){
        if((pTuner->flags & SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_FREIA_CONFIG_EXT_REF)){
            /* Disable Xtal */
            result = X_oscdis(pTuner);
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
 Implementation of static functions
------------------------------------------------------------------------------*/

static sony_result_t X_pon(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("X_pon");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Mode select (0x01) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* RFIN matching in power save (terrestrial) (0x67) */
    /* RFIN matching in power save (satellite) (0x43) */
    /* Power save setting for analog block (0x5E, 0x5F, 0x60) */
    /* Power save setting for analog block (0x0C) */

    {
        uint8_t dataT[4];  /* 0x5E - 0x60, 0x67 */
        uint8_t dataS[2];  /* 0x0C, 0x43 */
        uint8_t data[3] = {0x9E, 0x00, 0x00}; /* 0x79, 0x7A, 0x7B */
        {
            switch(pTuner->flags & SONY_FREIA_CONFIG_POWERSAVE_TERR_MASK){
            case SONY_FREIA_CONFIG_POWERSAVE_TERR_NORMAL_MATCHING_ENABLE:
                dataT[0] = 0x15;
                dataT[1] = 0x00;
                dataT[2] = 0x00;
                dataT[3] = 0x02;
                break;
            case SONY_FREIA_CONFIG_POWERSAVE_TERR_NORMAL_MATCHING_DISABLE:
                dataT[0] = 0x15;
                dataT[1] = 0x00;
                dataT[2] = 0x00;
                dataT[3] = 0x00;
                break;
            case SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_DISABLE:
                dataT[0] = 0x06;
                dataT[1] = 0x00;
                dataT[2] = 0x02;
                dataT[3] = 0x00;
                break;
            case SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_ENABLE:
                dataT[0] = 0x06;
                dataT[1] = 0x00;
                dataT[2] = 0x02;
                dataT[3] = 0x02;
                break;
            default:
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
            }
        }

        {
            uint8_t terrRfActive = ((pTuner->flags & SONY_FREIA_CONFIG_POWERSAVE_TERR_MASK)
                >= SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_DISABLE) ? 1 : 0;
            switch(pTuner->flags & SONY_FREIA_CONFIG_POWERSAVE_SAT_MASK){
            case SONY_FREIA_CONFIG_POWERSAVE_SAT_NORMAL: /* Mode 0 */
                dataS[0] = terrRfActive ? 0x04 : 0x14;     /* 0x0C */
                dataS[1] = 0xC0;     /* 0x43 */
                break;
            case SONY_FREIA_CONFIG_POWERSAVE_SAT_RF_ACTIVE: /* Mode 1 */
                dataS[0] = terrRfActive ? 0x05 : 0x15;
                dataS[1] = 0xC0;
                break;
            case SONY_FREIA_CONFIG_POWERSAVE_SAT_RF_ACTIVE_AGC_FULL: /* Mode 2 */
                dataS[0] = terrRfActive ? 0x05 : 0x15;
                dataS[1] = 0xC1;
                break;
            default:
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
            }
        }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x67, dataT[3]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x43, dataS[1]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, &dataT[0], 3);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0C, dataS[0]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x79, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x99, 0x9A, 0x9B */
    {
        /* XOSC Setting */
        /* Setting for OVLD */
        const uint8_t cdata[] = {0xA9, 0x01, 0x00};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x99, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x81 - 0x94 */
    {
        uint8_t data[20];

        /* Frequency setting for crystal oscillator (0x81) */
        data[0] = 0x18;

        /* Driver current setting for crystal oscillator (0x82) */
        /* Load capacitance setting for crystal oscillator (0x83) */
        if(pTuner->flags & SONY_FREIA_CONFIG_EXT_REF){
            /* XOSC_APC_EN = 0, XOSC_SEL= 0uA */
            data[1] = 0x00;
            /* XOSC_CALC_EN = 0, XOSC_CAP_SET = 0pF */
            data[2] = 0x00;
        }else{
            /* XOSC_APC_EN = 1, XOSC_SEL = xosc_sel (sony_freia_t member) */
            data[1] = (uint8_t)(0x80 | (pTuner->xosc_sel & 0x1F));
            /* XOSC_CALC_EN = 1, XOSC_CAP_SET = xosc_cap_set (sony_freia_t member) */
            data[2] = (uint8_t)(0x80 | (pTuner->xosc_cap_set & 0x3F));
        }

        /* Setting for REFOUT signal output (0x84) */
        switch(pTuner->flags & SONY_FREIA_CONFIG_REFOUT_MASK){
        case 0:
            data[3] = 0x00; /* REFOUT_EN = 0, REFOUT_CNT = 0 */
            break;
        case SONY_FREIA_CONFIG_REFOUT_500mVpp:
            data[3] = 0x80; /* REFOUT_EN = 1, REFOUT_CNT = 0 */
            break;
        case SONY_FREIA_CONFIG_REFOUT_400mVpp:
            data[3] = 0x81; /* REFOUT_EN = 1, REFOUT_CNT = 1 */
            break;
        case SONY_FREIA_CONFIG_REFOUT_600mVpp:
            data[3] = 0x82; /* REFOUT_EN = 1, REFOUT_CNT = 2 */
            break;
        case SONY_FREIA_CONFIG_REFOUT_800mVpp:
            data[3] = 0x83; /* REFOUT_EN = 1, REFOUT_CNT = 3 */
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* GPIO0, GPIO1 port setting (0x85, 0x86) */
        /* GPIO setting should be done by sony_freia_SetGPO/sony_freia_GetGPI after initialization */
        data[4] = 0x00;
        data[5] = 0x00;

        /* Clock enable for internal logic block (0x87) */
        data[6] = 0x84;

        /* Start CPU boot-up (0x88) */
        data[7] = 0x40;

        /* For burst-write (0x89) */
        data[8] = 0x10;

        /* Setting for internal RFAGC (0x8A, 0x8B, 0x8C) */
        data[9] = 0x00;
        data[10] = 0x45;
        data[11] = 0x75;

        /* Setting for analog block (0x8D) */
        data[12] = 0x01;

        /* Initial setting for internal analog block (0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94) */
        data[13] = 0x00;
        data[14] = 0x00;
        data[15] = 0x00;
        data[16] = 0x0A;
        data[17] = 0x0C;
        data[18] = 0x3F;
        data[19] = 0x00;

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x81, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Local block Filter setting */
    {
        const uint8_t cdata[] = {0x00, 0x08};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x22, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Initial setting for RF */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x46, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Wait 10ms */
    SONY_SLEEP(10);

    /* Check CPU_STT (0x1A) */
    {
        uint8_t rdata;

        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1A, &rdata, sizeof(rdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if(rdata != 0x00){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE); /* CPU_STT != 0x00 and CPU_ERR != 0x00 */
        }
    }

    /* SRAM Status check */
    {
        uint8_t data[2] = {0x7F, 0x06};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x17, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Wait 1ms */
    SONY_SLEEP(1);

    /* Filter Setting */
    {
        uint8_t rdata, data;

        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x19, &rdata, sizeof(rdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if(rdata == 0x00){
            data = 0x00;
        } else {
            data = 0x9E; /* MIX_GAIN = 0 */
        }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x79, data);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Disable IF signal output (IF_OUT_SEL setting) (0x74) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x74, 0x02);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    /* IFOUT DC bias setting (0xA0) */
    if(pTuner->flags & SONY_FREIA_CONFIG_IFOUT_DC_BIAS_500mV){
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0xA0, 0x0B);
    } else {
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0xA0, 0x00);
    }
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0x80);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Load capacitance control setting for crystal oscillator (0x80) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x80, 0x01);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Satellite initial setting (0x41, 0x42) */
    {
        const uint8_t cdata[] = {0x00, 0x00};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x41, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Satellite Initial Setting (0x45 - 0x4A, 0xA6, 0xA7*/
    {
        uint8_t data[] = {0x0A, 0x00, 0x00, 0x00, 0x00, 0x03, 0x66, 0x08};
        if(pTuner->flags & SONY_FREIA_CONFIG_SAT_LOW_GAIN_MODE){
            data[3] = 0x44;
        }else{
            data[3] = 0x11;
        }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x45, data, 6);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0xA6, &data[6], 2);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* PFD disable for XOSC issue for Multi tuner usage (0x1E) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1E, 0xA0);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t TER_tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem, uint8_t vcoCal)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("TER_tune");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Mode select (0x01) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    if(vcoCal){
        /* Disable IF signal output (IF_OUT_SEL setting) (0x74) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x74, 0x02);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x85, 0x86 */
    /* GPIO0, GPIO1 is changed by sony_freia_SetGPO/sony_freia_GetGPI */

    /* Clock enable for internal logic block, CPU wake-up (0x87, 0x88) */
    {
        const uint8_t cdata[2] = {0x84, 0x40};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Setting for OVLD block (0x3C - 0x3E) */
    {
        uint8_t data[5];

        if (tvSystem == SONY_FREIA_DTV_CABLE_6 || tvSystem == SONY_FREIA_DTV_CABLE_8 || tvSystem == SONY_FREIA_DTV_SKP_OPT) {
            if (!(pTuner->isExternalOvldTc)) {
                data[0] = 0xFC;
                data[1] = 0x87;
                data[2] = 0xCF;
                data[3] = 0x78;
                data[4] = 0x05;
            } else {
                data[0] = 0xFC;
                data[1] = 0x1C;
                data[2] = 0x0F;
                data[3] = 0x00;
                data[4] = 0x01;
            }
        } else {
            if (!(pTuner->isExternalOvldTc)) {
                data[0] = 0xFC;
                data[1] = 0x9C;
                data[2] = 0x8F;
                data[3] = 0x00;
                data[4] = 0x75;
            } else {
                data[0] = 0xFC;
                data[1] = 0x1C;
                data[2] = 0x0F;
                data[3] = 0x00;
                data[4] = 0x45;
            }
        }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x3c, data, 3);
        if (result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x52, data[3]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x8B, data[4]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Setting for internal analog block (0x8D) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x8D, 0x01);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* 0x91, 0x92 */
    {
        uint8_t data[2];

        /* Initial setting for internal analog block (0x91, 0x92) */
        if(SONY_FREIA_IS_DVB_T_T2(tvSystem)){
            data[0] = 0x00;
            data[1] = 0x02;
        }else{
            data[0] = 0x0A;
            data[1] = 0x0C;
        }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x91, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Setting for PLL block (0x9C, 0x9D) */
    {
        uint8_t data[2];

        if((pTuner->flags & SONY_FREIA_CONFIG_LOOPFILTER_INTERNAL)) {
            if(SONY_FREIA_IS_ATV(tvSystem)) {
                data[0] = 0x80;
                data[1] = 0x81;
            } else {
                data[0] = 0x8C;
                data[1] = 0x01;
            }
        } else {
            if(SONY_FREIA_IS_ATV(tvSystem)) {
                data[0] = 0x00;
                data[1] = 0x00;
            } else {
                data[0] = 0x05;
                data[1] = 0x00;
            }
        }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x9C, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Enable for analog block (0x5E, 0x5F, 0x60) */
    {
        const uint8_t cdata[3] = {0x6E, 0x02, 0x9E};

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7C, 0x01);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7C, 0x00);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1E, 0xA4);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, 0xEE);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x61 - 0x66 */
    {
        uint8_t data[6];
        /* Tuning setting for CPU (0x61) */
        if(vcoCal){
            data[0] = 0x66;
        }else{
            data[0] = 0x44;
        }

        /* Setting for PLL reference divider (REF_R) (0x62) */
        /* Tuning setting for analog block (0x63, 0x64, 0x65, 0x66) */
        if((pTuner->flags & SONY_FREIA_CONFIG_LOOPFILTER_INTERNAL)) {
            if(SONY_FREIA_IS_ATV(tvSystem)) {
                data[1] = 0x06;
                data[2] = 0x0A;
                data[3] = 0x1E;
                data[4] = 0x02;
                data[5] = 0x2C;
            } else {
                data[1] = 0x01;
                data[2] = 0x38;
                data[3] = 0x1E;
                data[4] = 0x02;
                data[5] = 0x24;
            }
        } else {
            if(SONY_FREIA_IS_ATV(tvSystem)) {
                data[1] = 0x30;
                data[2] = 0x38;
                data[3] = 0x78;
                data[4] = 0x08;
                data[5] = 0x1C;
            } else {
                data[1] = 0x01;
                data[2] = 0x5A;
                data[3] = 0x78;
                data[4] = 0x08;
                data[5] = 0x32;
            }
        }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x61, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* LT_AMP_EN should be 0 (0x67) */
    result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x67, 0x00, 0x02);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* 0x68 - 0x78 */
    {
        uint8_t data[17];

        /* Setting for IFOUT_LIMIT (0x68) */
        if(SONY_FREIA_IS_ATV(tvSystem)){
            data[0] = (uint8_t)((pTuner->flags & SONY_FREIA_CONFIG_OUTLMT_ATV_1_2Vpp) ? 0x01 : 0x00);
        }else if(SONY_FREIA_IS_DTV(tvSystem)){
            data[0] = (uint8_t)((pTuner->flags & SONY_FREIA_CONFIG_OUTLMT_DTV_1_2Vpp) ? 0x01 : 0x00);
        }else{
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
        }

        /* Setting for IF BPF buffer gain (0x69) */
        /* RF_GAIN setting */
        if(pTuner->pTerrParamTable[tvSystem].RF_GAIN == AUTO){
            data[1] = 0x80; /* RF_GAIN_SEL = 1 */
        }else{
            data[1] = (uint8_t)((pTuner->pTerrParamTable[tvSystem].RF_GAIN << 4) & 0x70);
        }

        /* IF_BPF_GC setting */
        data[1] |= (uint8_t)(pTuner->pTerrParamTable[tvSystem].IF_BPF_GC & 0x0F);

        /* Setting for internal RFAGC (0x6A, 0x6B, 0x6C) */
        if(!(pTuner->agcLimiterSetting)){
            data[2] = 0x00; /* Normal operation */
        }else{
            data[2] = 0x03; /* When monotonicity of terrestrial AGC is not obtained */
        }
        if(frequencykHz <= 172000){
            data[3] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].RFOVLD_DET_LV1_VL & 0x0F);
            data[4] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].IFOVLD_DET_LV_VL & 0x07);
        }else if(frequencykHz <= 464000){
            data[3] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].RFOVLD_DET_LV1_VH & 0x0F);
            data[4] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].IFOVLD_DET_LV_VH & 0x07);
        }else{
            data[3] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].RFOVLD_DET_LV1_U & 0x0F);
            data[4] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].IFOVLD_DET_LV_U & 0x07);
        }
        if (!(SONY_FREIA_IS_DVB_T_T2(tvSystem))){
            data[4] |= 0x30;
        }

        /* Setting for IF frequency and bandwidth */

        /* IF filter center frequency offset (IF_BPF_F0) (0x6D) */
        data[5] = (uint8_t)((pTuner->pTerrParamTable[tvSystem].IF_BPF_F0 << 4) & 0x30);

        /* IF filter band width (BW) (0x6D) */
        data[5] |= (uint8_t)(pTuner->pTerrParamTable[tvSystem].BW & 0x03);

        /* IF frequency offset value (FIF_OFFSET) (0x6E) */
        data[6] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].FIF_OFFSET & 0x1F);

        /* IF band width offset value (BW_OFFSET) (0x6F) */
        data[7] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].BW_OFFSET & 0x1F);

        /* RF tuning frequency setting (0x70, 0x71, 0x72) */
        data[8]  = (uint8_t)(frequencykHz & 0xFF);         /* FRF_L */
        data[9]  = (uint8_t)((frequencykHz >> 8) & 0xFF);  /* FRF_M */
        data[10] = (uint8_t)((frequencykHz >> 16) & 0x1F); /* FRF_H (bit[4:0]) */

        if(tvSystem == SONY_FREIA_ATV_L_DASH){
            data[10] |= 0x40; /* IS_L_DASH (bit[6]) */
        }

        if(SONY_FREIA_IS_ATV(tvSystem)){
            data[10] |= 0x80; /* IS_FP (bit[7]) */
        }

        /* Tuning command (0x73) */
        if(vcoCal){
            data[11] = 0xFF;
        }else{
            data[11] = 0x8F;
        }

        /* Enable IF output, AGC and IFOUT pin selection (0x74) */
        if(pTuner->chipId == SONY_FREIA_CHIP_ID_6866ER){
            data[12] = 0x00;

            if(pTuner->pTerrParamTable[tvSystem].AGC_SEL == AUTO){
                /* AGC pin setting by config flags */
                if(SONY_FREIA_IS_ATV(tvSystem)){
                    /* Analog */
                    if(pTuner->flags & SONY_FREIA_CONFIG_AGC2_ATV){
                        data[12] |= 0x10;
                    }
                }else{
                    /* Digital */
                    if(pTuner->flags & SONY_FREIA_CONFIG_AGC2_DTV){
                        data[12] |= 0x10;
                    }
                }
            }else{
                /* AGC pin setting from parameter table */
                data[12] |= (uint8_t)((pTuner->pTerrParamTable[tvSystem].AGC_SEL << 4) & 0x30);
            }

            if(pTuner->pTerrParamTable[tvSystem].IF_OUT_SEL == AUTO){
                /* IFOUT pin setting by config flags */
                if(SONY_FREIA_IS_ATV(tvSystem)){
                    /* Analog */
                    if(pTuner->flags & SONY_FREIA_CONFIG_IF2_ATV){
                        data[12] |= 0x01;
                    }
                }else{
                    /* Digital */
                    if(pTuner->flags & SONY_FREIA_CONFIG_IF2_DTV){
                        data[12] |= 0x01;
                    }
                }
            }else{
                /* IFOUT pin setting from parameter table */
                data[12] |= (uint8_t)(pTuner->pTerrParamTable[tvSystem].IF_OUT_SEL & 0x03);
            }
        }else if((pTuner->chipId == SONY_FREIA_CHIP_ID_6868ER) || (pTuner->chipId == SONY_FREIA_CHIP_ID_6866AER)){
            data[12] = 0x00;
        }else{
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* Tuning setting for analog block (0x75, 0x76, 0x77, 0x78) */
        data[13] = 0xF1;
        data[14] = 0x0F;
        data[15] = 0x06;
        data[16] = 0x03;

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x68, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t TER_tune_end(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("TER_tune_end");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0x80);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t SAT_tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem, uint32_t symbolRateksps, uint8_t vcoCal)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("SAT_tune");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if(vcoCal){
        /* Disable IF signal output (IF_OUT_SEL setting) (0x15) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x15, 0x02);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* RFIN matching in Power Save(SAT) reset (0x43) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x43, 0xC0);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Satellite mode select (0x01) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x01);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Reset Analog block setting (0x6B) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x6B, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Tuning setting for CPU (0x40) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x40, (uint8_t)(vcoCal ? 0x06 : 0x04));
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* 0x02, 0x03 */
    /* GPIO0, GPIO1 is changed by sony_freia_SetGPO/sony_freia_GetGPI */

    /* 0x04 - 0x0B */
    {
        uint8_t data[8];

        /* Clock enable for internal logic block, CPU wake-up (0x04, 0x05) */
        data[0] = 0x84;
        data[1] = 0x40;

        /* Setting for PLL reference divider (REF_R) (0x06) */
        /* Tuning Setting for PLL block (0x07 - 0x0B) */
        if((pTuner->flags & SONY_FREIA_CONFIG_LOOPFILTER_INTERNAL)) {
            /* internal loop */
            data[2] = 0x01;
            data[3] = 0x8A;
            data[4] = 0x38;
            data[5] = 0x1E;
            data[6] = 0x02;
            data[7] = 0x24;
        }else{
            /* external loop */
            if (pTuner->isFreesatMode) {
                data[2] = 0x01;
                data[3] = 0x00;
                data[4] = 0x0C;
                data[5] = 0x78;
                data[6] = 0x08;
                data[7] = 0x30;
            } else {
                data[2] = 0x01;
                data[3] = 0x05;
                data[4] = 0x5A;
                data[5] = 0x78;
                data[6] = 0x08;
                data[7] = 0x31;
            }
        }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x04, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x44 */
    if((pTuner->flags & SONY_FREIA_CONFIG_LOOPFILTER_INTERNAL)) {
        /* internal loop */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x44, 0x01);
    } else {
        /* external loop */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x44, 0x00);
    }
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Enable for analog block (0x0C, 0x0D, 0x0E) */
    {
        uint8_t data[3];

        if(pTuner->flags & SONY_FREIA_CONFIG_SAT_LOW_GAIN_MODE){
            data[0] = 0x0E;
        }else{
            data[0] = 0x0F;
        }
        if((pTuner->flags & SONY_FREIA_CONFIG_POWERSAVE_TERR_MASK)
            >= SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_DISABLE){
            data[0] |= 0x60; /* Clear RFOVLD_DET_ENX (Bit[4]) */
        }else{
            data[0] |= 0x70;
        }

        data[1] = 0x02;
        data[2] = 0x9E;

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7C, 0x01);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0C, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7C, 0x00);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1E, 0xA4);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if(pTuner->flags & SONY_FREIA_CONFIG_SAT_LOW_GAIN_MODE){
            data[0] = 0x0E;
        }else{
            data[0] = 0x0F;
        }
        if((pTuner->flags & SONY_FREIA_CONFIG_POWERSAVE_TERR_MASK)
            >= SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_DISABLE){
            data[0] |= 0xE0; /* Clear RFOVLD_DET_ENX (Bit[4]) */
        }else{
            data[0] |= 0xF0;
        }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0C, data[0]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    {
        uint8_t data[7];
        uint32_t frequency4kHz = (frequencykHz + 2) / 4;
        /* Setting for LPF cutoff frequency (0x0F) */
        switch(tvSystem){
        case SONY_FREIA_STV_ISDBS:
        case SONY_FREIA_STV_ISDBS3:
            data[0] = 22; /* 22MHz */
            break;

        case SONY_FREIA_STV_DVBS:
            /*
                rolloff = 0.35

                SR <= 4
                    lpf_cutoff = 5
                4 < SR <= 10
                    lpf_cutoff = SR / 2 * (2 + rolloff) = SR * 1.175 = SR * (47/40)
                10 < SR
                    lpf_cutoff = SR / 2 * (1 + rolloff) + 5 = SR * 0.675 + 5 = SR * (27/40) + 5
                NOTE: The result should be round up.
            */
            if(symbolRateksps <= 4000){
                data[0] = 5;
            }else if(symbolRateksps <= 10000){
                data[0] = (uint8_t)((symbolRateksps * 47 + (40000-1)) / 40000);
            }else{
                data[0] = (uint8_t)((symbolRateksps * 27 + (40000-1)) / 40000 + 5);
            }

            if(data[0] > 36){
                data[0] = 36; /* 5 <= lpf_cutoff <= 36 is valid */
            }
            break;

        case SONY_FREIA_STV_DVBS2:
            /*
                rolloff = 0.2

                SR <= 4
                    lpf_cutoff = 5
                4 < SR <= 10
                    lpf_cutoff = SR / 2 * (2 + rolloff) = SR * 1.1 = SR * (11/10)
                10 < SR
                    lpf_cutoff = SR / 2 * (1 + rolloff) + 5 = SR * 0.6 + 5 = SR * (3/5) + 5
                NOTE: The result should be round up.
            */
            if(symbolRateksps <= 4000){
                data[0] = 5;
            }else if(symbolRateksps <= 10000){
                data[0] = (uint8_t)((symbolRateksps * 11 + (10000-1)) / 10000);
            }else{
                data[0] = (uint8_t)((symbolRateksps * 3 + (5000-1)) / 5000 + 5);
            }

            if(data[0] > 36){
                data[0] = 36; /* 5 <= lpf_cutoff <= 36 is valid */
            }
            break;

        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG); /* Invalid system */
        }

        /* RF tuning frequency setting (0x10, 0x11, 0x12) */
        data[1] = (uint8_t)(frequency4kHz & 0xFF);         /* FRF_L */
        data[2] = (uint8_t)((frequency4kHz >> 8) & 0xFF);  /* FRF_M */
        data[3] = (uint8_t)((frequency4kHz >> 16) & 0x1F); /* FRF_H (bit[4:0]) */

        /* Tuning command (0x13) */
        if(vcoCal){
            data[4] = 0xFF;
        }else{
            data[4] = 0x8F;
        }

        /* Setting for IQOUT_LIMIT (0x14) */
        data[5] = (uint8_t)((pTuner->flags & SONY_FREIA_CONFIG_OUTLMT_STV_0_6Vpp) ? 0x01 : 0x00);

        /* Enable IQ output (0x15) */
        data[6] = 0x01;

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0F, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t SAT_tune_end(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("SAT_tune_end");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Standby setting for CPU (0x05) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x05, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x04) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x04, 0x80);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t TER_IQ_tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem, uint32_t symbolRateksps, uint8_t vcoCal)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("TER_IQ_tune");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Mode select (0x01) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x10);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    if(vcoCal){
        /* Disable IF signal output (IF_OUT_SEL setting) (0x15) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x15, 0x02);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x85, 0x86 */
    /* GPIO0, GPIO1 is changed by sony_freia_SetGPO/sony_freia_GetGPI */

    /* Clock enable for internal logic block, CPU wake-up (0x87, 0x88) */
    {
        const uint8_t cdata[2] = {0x84, 0x40};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Setting for OVLD block */
    {
        uint8_t cdata[3] = {0x4C, 0x85, 0x8A};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x3c, cdata, sizeof(cdata));
        if (result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Setting for OVLD block (0x52) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x52, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /*  Setting for OVLD block (0x8B) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x8B, 0x45);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Setting for internal analog block (0x8D) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x8D, 0x02);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* 0x91, 0x92 */
    {
        const uint8_t cdata[2] = {0x0A, 0x0C};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x91, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Setting for PLL block (0x9C, 0x9D) */
    {
        const uint8_t cdata[2] = {0x8C, 0x01};

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x9C, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }


    /* Enable for analog block (0x7C, 0x5E, 0x5F, 0x60, 0x7C, 0x1E, 0x5E) */
    {
        const uint8_t cdata[3] = {0x6E, 0x02, 0x9E};

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7C, 0x01);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7C, 0x00);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1E, 0xA4);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, 0xEE);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x61 - 0x66 */
    {
        uint8_t data[6];
        /* Tuning setting for CPU (0x61) */
        if(vcoCal){
            data[0] = 0x66;
        }else{
            data[0] = 0x44;
        }

        /* Setting for PLL reference divider (REF_R) (0x62) */
        data[1] = 0x01;
        data[2] = 0x38;
        data[3] = 0x1E;
        data[4] = 0x02;
        data[5] = 0x24;

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x61, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* LT_AMP_EN should be 0 (0x67) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x67, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Setting for IFOUT_LIMIT (0x68) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x68, (uint8_t)((pTuner->flags & SONY_FREIA_CONFIG_OUTLMT_DTV_1_2Vpp) ? 0x01 : 0x00));
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Setting for internal RFAGC (0x6B, 0x6C) */
    {
        const uint8_t cdata[2] = {0x0D, 0x33};

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x6B, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    {
        /* Setting for LPF cutoff frequency (0x0F) */
        uint8_t data = 0;
        switch(tvSystem){
        case SONY_FREIA_STV_ISDBS:
        case SONY_FREIA_STV_ISDBS3:
            data = 22; /* 22MHz */
            break;

        case SONY_FREIA_STV_DVBS:
            /*
                rolloff = 0.35

                SR <= 4
                    lpf_cutoff = 5
                4 < SR <= 10
                    lpf_cutoff = SR / 2 * (2 + rolloff) = SR * 1.175 = SR * (47/40)
                10 < SR
                    lpf_cutoff = SR / 2 * (1 + rolloff) + 5 = SR * 0.675 + 5 = SR * (27/40) + 5
                NOTE: The result should be round up.
            */
            if(symbolRateksps <= 4000){
                data = 5;
            }else if(symbolRateksps <= 10000){
                data = (uint8_t)((symbolRateksps * 47 + (40000-1)) / 40000);
            }else{
                data = (uint8_t)((symbolRateksps * 27 + (40000-1)) / 40000 + 5);
            }

            if(data > 36){
                data = 36; /* 5 <= lpf_cutoff <= 36 is valid */
            }
            break;

        case SONY_FREIA_STV_DVBS2:
            /*
                rolloff = 0.2

                SR <= 4
                    lpf_cutoff = 5
                4 < SR <= 10
                    lpf_cutoff = SR / 2 * (2 + rolloff) = SR * 1.1 = SR * (11/10)
                10 < SR
                    lpf_cutoff = SR / 2 * (1 + rolloff) + 5 = SR * 0.6 + 5 = SR * (3/5) + 5
                NOTE: The result should be round up.
            */
            if(symbolRateksps <= 4000){
                data = 5;
            }else if(symbolRateksps <= 10000){
                data = (uint8_t)((symbolRateksps * 11 + (10000-1)) / 10000);
            }else{
                data = (uint8_t)((symbolRateksps * 3 + (5000-1)) / 5000 + 5);
            }

            if(data > 36){
                data = 36; /* 5 <= lpf_cutoff <= 36 is valid */
            }
            break;

        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG); /* Invalid system */
        }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0F, data);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    if(!pTuner->isQDumpOn){
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x9F, 0x00);
    }else{
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x9F, 0x01);
    }
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* 0x70 - 0x73 */
    {
        uint8_t data[4];

        /* RF tuning frequency setting (0x70, 0x71, 0x72) */
        data[0] = (uint8_t)(frequencykHz & 0xFF);         /* FRF_L */
        data[1] = (uint8_t)((frequencykHz >> 8) & 0xFF);  /* FRF_M */
        data[2] = (uint8_t)((frequencykHz >> 16) & 0x1F); /* FRF_H (bit[4:0]) */

        if(tvSystem == SONY_FREIA_ATV_L_DASH){
            data[2] |= 0x40; /* IS_L_DASH (bit[6]) */
        }

        if(SONY_FREIA_IS_ATV(tvSystem)){
            data[2] |= 0x80; /* IS_FP (bit[7]) */
        }

        /* Tuning command (0x73) */
        if(vcoCal){
            data[3] = 0xFF;
        }else{
            data[3] = 0x8F;
        }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x70, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /*Enable IF output /AGC and IFOUT pin selection (0x15) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x15, 0x01);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* 0x75 - 0x78 */
    {
        const uint8_t cdata[4] = {0xF1, 0x0F, 0x06, 0x03};

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x75, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t TER_IQ_tune_end(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("TER_IQ_tune_end");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0x80);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t TER_fin(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("TER_fin");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Disable IF signal output (IF_OUT_SEL setting) (0x74) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x74, 0x02);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* RFIN matching in power save (terrestrial) (0x67) */
    /* Power save setting for analog block (0x5E, 0x5F, 0x60) */
    {
        uint8_t data[4]; /* 0x5E - 0x60, 0x67 */

        switch(pTuner->flags & SONY_FREIA_CONFIG_POWERSAVE_TERR_MASK){
        case SONY_FREIA_CONFIG_POWERSAVE_TERR_NORMAL_MATCHING_ENABLE:
            data[0] = 0x15;
            data[1] = 0x00;
            data[2] = 0x00;
            data[3] = 0x02;
            break;
        case SONY_FREIA_CONFIG_POWERSAVE_TERR_NORMAL_MATCHING_DISABLE:
            data[0] = 0x15;
            data[1] = 0x00;
            data[2] = 0x00;
            data[3] = 0x00;
            break;
        case SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_DISABLE:
            data[0] = 0x06;
            data[1] = 0x00;
            data[2] = 0x02;
            data[3] = 0x00;
            break;
        case SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_ENABLE:
            data[0] = 0x06;
            data[1] = 0x00;
            data[2] = 0x02;
            data[3] = 0x02;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* Keep RF_EXT bit */
        result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x67, data[3], 0xFE);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, 0x6E);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1E, 0xA0);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, &data[0], 3);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Reset OVLD block (0x3D, 0x3E)*/
    {
        uint8_t cdata[] = {0x00, 0x00};

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x3D, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0x80);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t SAT_fin(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("SAT_fin");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Disable IQ signal output (IF_OUT_SEL setting) (0x15) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x15, 0x02);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* RFIN matching in power save (satellite) (0x43, 0x45) */
    /* Power save setting for analog block (0x0C, 0x0D, 0x0E) */
    {
        uint8_t data[4]; /* 0x0C - 0x0E, 0x43 */
        uint8_t terrRfActive = ((pTuner->flags & SONY_FREIA_CONFIG_POWERSAVE_TERR_MASK)
            >= SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_DISABLE) ? 1 : 0;

        switch(pTuner->flags & SONY_FREIA_CONFIG_POWERSAVE_SAT_MASK){
        case SONY_FREIA_CONFIG_POWERSAVE_SAT_NORMAL: /* Mode 0 */
            data[0] = terrRfActive ? 0x04 : 0x14;
            data[1] = 0x00;
            data[2] = terrRfActive ? 0x02 : 0x00;
            data[3] = 0xC0;
            break;
        case SONY_FREIA_CONFIG_POWERSAVE_SAT_RF_ACTIVE: /* Mode 1 */
            data[0] = terrRfActive ? 0x05 : 0x15;
            data[1] = 0x00;
            data[2] = terrRfActive ? 0x02 : 0x00;
            data[3] = 0xC0;
            break;
        case SONY_FREIA_CONFIG_POWERSAVE_SAT_RF_ACTIVE_AGC_FULL: /* Mode 2 */
            data[0] = terrRfActive ? 0x05 : 0x15;
            data[1] = 0x00;
            data[2] = terrRfActive ? 0x02 : 0x00;
            data[3] = 0xC1;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x43, data[3]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if(pTuner->flags & SONY_FREIA_CONFIG_SAT_LOW_GAIN_MODE){
            result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0C, 0x7E);
        }else{
            result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0C, 0x7F);
        }
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1E, 0xA0);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0C, &data[0], 3);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Return to terrestrial mode (0x01) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for CPU (0x05) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x05, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x04) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x04, 0x80);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t TER_IQ_fin(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("TER_IQ_fin");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Disable IF signal output (0x15) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x15, 0x02);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* RFIN matching in power save (terrestrial) (0x67) */
    /* Power save setting for analog block (0x5E, 0x5F, 0x60) */
    {
        uint8_t data[4]; /* 0x5E - 0x60, 0x67 */

        switch(pTuner->flags & SONY_FREIA_CONFIG_POWERSAVE_TERR_MASK){
        case SONY_FREIA_CONFIG_POWERSAVE_TERR_NORMAL_MATCHING_ENABLE:
            data[0] = 0x15;
            data[1] = 0x00;
            data[2] = 0x00;
            data[3] = 0x02;
            break;
        case SONY_FREIA_CONFIG_POWERSAVE_TERR_NORMAL_MATCHING_DISABLE:
            data[0] = 0x15;
            data[1] = 0x00;
            data[2] = 0x00;
            data[3] = 0x00;
            break;
        case SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_DISABLE:
            data[0] = 0x06;
            data[1] = 0x00;
            data[2] = 0x02;
            data[3] = 0x00;
            break;
        case SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_ENABLE:
            data[0] = 0x06;
            data[1] = 0x00;
            data[2] = 0x02;
            data[3] = 0x02;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* Keep RF_EXT bit */
        result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x67, data[3], 0xFE);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, 0x6E);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1E, 0xA0);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, &data[0], 3);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Return to terrestrial mode (0x01) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Reset OVLD block (0x3D, 0x3E)*/
    {
        uint8_t cdata[] = {0x00, 0x00};

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x3D, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Q_Dump Reset (0x9F) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x9F, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }


    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0x80);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t X_oscdis(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("X_oscdis");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Setting for REFOUT signal output (0x84) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x84, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Driver current setting for crystal oscillator (0x82) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x82, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t X_oscen(sony_freia_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER("X_oscen");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Driver current setting for crystal oscillator (0x82) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x82, 0x9F);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Setting for REFOUT signal output (0x84) */
    switch(pTuner->flags & SONY_FREIA_CONFIG_REFOUT_MASK){
    case 0:
        data = 0x00; /* REFOUT_EN = 0, REFOUT_CNT = 0 */
        break;
    case SONY_FREIA_CONFIG_REFOUT_500mVpp:
        data = 0x80; /* REFOUT_EN = 1, REFOUT_CNT = 0 */
        break;
    case SONY_FREIA_CONFIG_REFOUT_400mVpp:
        data = 0x81; /* REFOUT_EN = 1, REFOUT_CNT = 1 */
        break;
    case SONY_FREIA_CONFIG_REFOUT_600mVpp:
        data = 0x82; /* REFOUT_EN = 1, REFOUT_CNT = 2 */
        break;
    case SONY_FREIA_CONFIG_REFOUT_800mVpp:
        data = 0x83; /* REFOUT_EN = 1, REFOUT_CNT = 3 */
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x84, data);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_SLEEP(10);

    /* Driver current setting for crystal oscillator (0x82) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x82,
        (uint8_t)(0x80 | (pTuner->xosc_sel & 0x1F)));
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t X_read_agc(sony_freia_t *pTuner, uint8_t *pSatCompensationReg, uint8_t *pTerrCompensationReg,
    uint8_t *pIFAGCReg, uint8_t *pRFAGCReg)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("X_read_agc");

    if((!pTuner) || (!pTuner->pI2c) || (!pSatCompensationReg) || (!pTerrCompensationReg) || (!pIFAGCReg) || (!pRFAGCReg)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Clock enable for internal logic block, CPU wake-up (0x87, 0x88) */
    {
        const uint8_t cdata[2] = {0x84, 0x41};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Read Compensation data */
    {
        uint8_t data[2] = {0x9E, 0x06};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x17, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        /* Wait at least 1ms */
        SONY_SLEEP(2);

        /* Confirm CPU_STT is 0x00 */
        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1A, &data[0], 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if(data[0] != 0x00){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x19, &data[0], 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        *pTerrCompensationReg = (data[0] >> 4) & 0x0F;
        *pSatCompensationReg  = data[0] & 0x0F;
    }

    /* Connect IFAGC, Start ADC (0x59, 0x5A) */
    {
        const uint8_t cdata[2] = {0x05, 0x01};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x59, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* ADC read out (0x5B) */
    result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5B, pIFAGCReg, 1);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Connect RFAGC, Start ADC (0x59, 0x5A) */
    {
        const uint8_t cdata[2] = {0x03, 0x01};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x59, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* ADC read out (0x5B) */
    result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5B, pRFAGCReg, 1);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* ADC disable (0x59) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x59, 0x04);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0x80);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}
