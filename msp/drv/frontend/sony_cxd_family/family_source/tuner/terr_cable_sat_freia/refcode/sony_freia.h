/*------------------------------------------------------------------------------
  Copyright 2020-2022 Sony Semiconductor Solutions Corporation

  Last Updated  : 2022/08/10
  File Revision : 1.0.2.0
------------------------------------------------------------------------------*/
/**
 @file    sony_freia.h

          This file provides the FREIA tuner control interface.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_FREIA_H
#define SONY_FREIA_H

#include "sony_common.h"
#include "sony_i2c.h"

/*------------------------------------------------------------------------------
  Defines
------------------------------------------------------------------------------*/
/**
 @brief Version of this driver.
*/
#define SONY_FREIA_VERSION       "1.2.0.0"

/**
 @brief Default I2C slave address of the FREIA tuner.
*/
#define SONY_FREIA_ADDRESS       0xC0

/*------------------------------------------------------------------------------
  Enums
------------------------------------------------------------------------------*/
/**
 @brief FREIA tuner state.
*/
typedef enum {
    SONY_FREIA_STATE_UNKNOWN,     /**< FREIA state is Unknown */
    SONY_FREIA_STATE_SLEEP,       /**< FREIA state is Sleep */
    SONY_FREIA_STATE_ACTIVE_T,    /**< FREIA state is Active (terrestrial) */
    SONY_FREIA_STATE_ACTIVE_S,    /**< FREIA state is Active (satellite) */
    SONY_FREIA_STATE_ACTIVE_IQ    /**< FREIA state is Active (terrestrial with IQ output) */
} sony_freia_state_t;

/**
 @brief FREIA chip type.
*/
typedef enum {
    SONY_FREIA_CHIP_ID_UNKNOWN,   /**< Unknown */
    SONY_FREIA_CHIP_ID_6868ER,    /**< CXD6868ER  (FREIA Plus) */
    SONY_FREIA_CHIP_ID_6866AER,   /**< CXD6866AER (FREIA) */
    SONY_FREIA_CHIP_ID_6866ER,    /**< CXD6866ER  (ASCOT4) */
} sony_freia_chip_id_t;

/**
 @brief Analog/Digital terrestrial, satellite broadcasting system definitions supported by FREIA.

        Fc : Center Frequency, Fp : Picture Frequency
*/
typedef enum {
    SONY_FREIA_TV_SYSTEM_UNKNOWN,
    /* Terrestrial Analog */
    SONY_FREIA_ATV_MN_EIAJ,  /**< System-M (Japan)          (IF: Fp=5.75MHz in default) */
    SONY_FREIA_ATV_MN_SAP,   /**< System-M (US)             (IF: Fp=5.75MHz in default) */
    SONY_FREIA_ATV_MN_A2,    /**< System-M (Korea)          (IF: Fp=5.9MHz in default) */
    SONY_FREIA_ATV_BG,       /**< System-B/G                (IF: Fp=7.3MHz in default) */
    SONY_FREIA_ATV_I,        /**< System-I                  (IF: Fp=7.85MHz in default) */
    SONY_FREIA_ATV_DK,       /**< System-D/K                (IF: Fp=7.85MHz in default) */
    SONY_FREIA_ATV_L,        /**< System-L                  (IF: Fp=7.85MHz in default) */
    SONY_FREIA_ATV_L_DASH,   /**< System-L DASH             (IF: Fp=2.2MHz in default) */
    /* Terrestrial/Cable Digital */
    SONY_FREIA_DTV_8VSB,     /**< ATSC 8VSB                 (IF: Fc=3.7MHz in default) */
    SONY_FREIA_DTV_ISDBT_6,  /**< ISDB-T 6MHzBW             (IF: Fc=3.55MHz in default) */
    SONY_FREIA_DTV_ISDBT_7,  /**< ISDB-T 7MHzBW             (IF: Fc=4.15MHz in default) */
    SONY_FREIA_DTV_ISDBT_8,  /**< ISDB-T 8MHzBW             (IF: Fc=4.75MHz in default) */
    SONY_FREIA_DTV_DVBT_5,   /**< DVB-T 5MHzBW              (IF: Fc=3.6MHz in default) */
    SONY_FREIA_DTV_DVBT_6,   /**< DVB-T 6MHzBW              (IF: Fc=3.6MHz in default) */
    SONY_FREIA_DTV_DVBT_7,   /**< DVB-T 7MHzBW              (IF: Fc=4.2MHz in default) */
    SONY_FREIA_DTV_DVBT_8,   /**< DVB-T 8MHzBW              (IF: Fc=4.8MHz in default) */
    SONY_FREIA_DTV_DVBT2_1_7,/**< DVB-T2 1.7MHzBW           (IF: Fc=3.5MHz in default) */
    SONY_FREIA_DTV_DVBT2_5,  /**< DVB-T2 5MHzBW             (IF: Fc=3.6MHz in default) */
    SONY_FREIA_DTV_DVBT2_6,  /**< DVB-T2 6MHzBW             (IF: Fc=3.6MHz in default) */
    SONY_FREIA_DTV_DVBT2_7,  /**< DVB-T2 7MHzBW             (IF: Fc=4.2MHz in default) */
    SONY_FREIA_DTV_DVBT2_8,  /**< DVB-T2 8MHzBW             (IF: Fc=4.8MHz in default) */
    SONY_FREIA_DTV_CABLE_6,  /**< DVB-C 6MHzBW/ISDB-C/J.83B (IF: Fc=3.7MHz in default) */
    SONY_FREIA_DTV_CABLE_8,  /**< DVB-C 8MHzBW/7MHzBW       (IF: Fc=4.9MHz in default) */
    SONY_FREIA_DTV_DVBC2_6,  /**< DVB-C2 6MHzBW             (IF: Fc=3.7MHz in default) */
    SONY_FREIA_DTV_DVBC2_8,  /**< DVB-C2 8MHzBW             (IF: Fc=4.9MHz in default) */
    SONY_FREIA_DTV_ATSC3_6,  /**< ATSC 3.0 6MHzBW           (IF: Fc=3.6MHz in default) */
    SONY_FREIA_DTV_ATSC3_7,  /**< ATSC 3.0 7MHzBW           (IF: Fc=4.2MHz in default) */
    SONY_FREIA_DTV_ATSC3_8,  /**< ATSC 3.0 8MHzBW           (IF: Fc=4.8MHz in default) */
    SONY_FREIA_DTV_SKP_OPT,  /**< J.83B 5.6Msps             (IF: Fc=3.75MHz in default) */
    SONY_FREIA_DTV_DTMB,     /**< DTMB                      (IF: Fc=5.1MHz in default) */
    /* Satellite */
    SONY_FREIA_STV_ISDBS,    /**< ISDB-S */
    SONY_FREIA_STV_ISDBS3,   /**< ISDB-S3 */
    SONY_FREIA_STV_DVBS,     /**< DVB-S */
    SONY_FREIA_STV_DVBS2,    /**< DVB-S2 */

    SONY_FREIA_ATV_MIN = SONY_FREIA_ATV_MN_EIAJ, /**< Minimum analog terrestrial system */
    SONY_FREIA_ATV_MAX = SONY_FREIA_ATV_L_DASH,  /**< Maximum analog terrestrial system */
    SONY_FREIA_DTV_MIN = SONY_FREIA_DTV_8VSB,    /**< Minimum digital terrestrial system */
    SONY_FREIA_DTV_MAX = SONY_FREIA_DTV_DTMB,    /**< Maximum digital terrestrial system */
    SONY_FREIA_TERR_TV_SYSTEM_NUM,               /**< Number of supported terrestrial broadcasting system */
    SONY_FREIA_STV_MIN = SONY_FREIA_STV_ISDBS,   /**< Minimum satellite system */
    SONY_FREIA_STV_MAX = SONY_FREIA_STV_DVBS2    /**< Maximum satellite system */
} sony_freia_tv_system_t;

/**
 @brief Macro to check that the system is analog terrestrial or not.
*/
#define SONY_FREIA_IS_ATV(tvSystem) (((tvSystem) >= SONY_FREIA_ATV_MIN) && ((tvSystem) <= SONY_FREIA_ATV_MAX))

/**
 @brief Macro to check that the system is digital terrestrial or not.
*/
#define SONY_FREIA_IS_DTV(tvSystem) (((tvSystem) >= SONY_FREIA_DTV_MIN) && ((tvSystem) <= SONY_FREIA_DTV_MAX))

/**
 @brief Macro to check that the system is satellite or not.
*/
#define SONY_FREIA_IS_STV(tvSystem) (((tvSystem) >= SONY_FREIA_STV_MIN) && ((tvSystem) <= SONY_FREIA_STV_MAX))

/*------------------------------------------------------------------------------
  Structs
------------------------------------------------------------------------------*/

/**
 @brief FREIA settings that may need to change depend on customer's system.
*/
typedef struct sony_freia_terr_adjust_param_t {
    uint8_t RF_GAIN;           /**< Addr:0x69 Bit[6:4] : RFVGA gain. 0xFF means Auto. (RF_GAIN_SEL = 1) */
    uint8_t IF_BPF_GC;         /**< Addr:0x69 Bit[3:0] : IF_BPF gain. */
    uint8_t RFOVLD_DET_LV1_VL; /**< Addr:0x6B Bit[3:0] : RF overload RF input detect level. (FRF <= 172MHz) */
    uint8_t RFOVLD_DET_LV1_VH; /**< Addr:0x6B Bit[3:0] : RF overload RF input detect level. (172MHz < FRF <= 464MHz) */
    uint8_t RFOVLD_DET_LV1_U;  /**< Addr:0x6B Bit[3:0] : RF overload RF input detect level. (FRF > 464MHz) */
    uint8_t IFOVLD_DET_LV_VL;  /**< Addr:0x6C Bit[2:0] : Internal RFAGC detect level. (FRF <= 172MHz) */
    uint8_t IFOVLD_DET_LV_VH;  /**< Addr:0x6C Bit[2:0] : Internal RFAGC detect level. (172MHz < FRF <= 464MHz) */
    uint8_t IFOVLD_DET_LV_U;   /**< Addr:0x6C Bit[2:0] : Internal RFAGC detect level. (FRF > 464MHz) */
    uint8_t IF_BPF_F0;         /**< Addr:0x6D Bit[5:4] : IF filter center offset. */
    uint8_t BW;                /**< Addr:0x6D Bit[1:0] : 6MHzBW(0x00) or 7MHzBW(0x01) or 8MHzBW(0x02) or 1.7MHzBW(0x03) */
    uint8_t FIF_OFFSET;        /**< Addr:0x6E Bit[4:0] : 5bit signed. IF offset (kHz) = FIF_OFFSET x 50 */
    uint8_t BW_OFFSET;         /**< Addr:0x6F Bit[4:0] : 5bit signed. BW offset (kHz) = BW_OFFSET x 50 (BW_OFFSET x 10 in 1.7MHzBW) */
    uint8_t AGC_SEL;           /**< Addr:0x74 Bit[5:4] : AGC pin select. (0: AGC1, 1: AGC2) 0xFF means Auto (by config flags) */
    uint8_t IF_OUT_SEL;        /**< Addr:0x74 Bit[0:1] : IFOUT pin select. (0: IFOUT1, 1: IFOUT2) 0xFF means Auto. (by config flags) */
} sony_freia_terr_adjust_param_t;

/**
 @brief The FREIA tuner definition which allows control of the FREIA tuner device
        through the defined set of functions.
*/
typedef struct sony_freia_t {
    uint8_t                 i2cAddress;     /**< I2C slave address of the FREIA tuner (8-bit form - 8'bxxxxxxx0) */
    sony_i2c_t*             pI2c;           /**< I2C API instance. */
    uint32_t                flags;          /**< ORed value of SONY_FREIA_CONFIG_XXXX */

    /* For saving current setting */
    sony_freia_state_t      state;          /**< The driver operating state. */
    uint32_t                frequencykHz;   /**< Currently RF frequency(kHz) tuned. */
    sony_freia_tv_system_t  tvSystem;       /**< Current broadcasting system tuned. */
    uint32_t                symbolRateksps; /**< Current symbol rate(ksym/s) tuned. */

    uint8_t                 isFreesatMode;    /**< UK Freesat mode or not. */

    uint8_t                 isExternalOvldTc;  /**< External OVLD TC mode or not. */
    /**
     @brief AGC Limiter setting for terrestrial. Set to 0 for normal operation. Set to 1 when the monotonicity of terrestrial AGC
            is not obtained.
    */
    uint8_t                 agcLimiterSetting;
    uint8_t                 isQDumpOn;         /**< Q_Dump is On or not. (for IQ mode)*/
    sony_freia_chip_id_t    chipId;            /**< Auto detected chip ID at initialization */
    /**
     @brief Adjustment parameter table (SONY_FREIA_TV_SYSTEM_NUM size)
    */
    const sony_freia_terr_adjust_param_t *pTerrParamTable;

    /* Following Xtal related parameters can be changed if optimization is necessary. */
    uint8_t                 xosc_sel;       /**< Driver current setting for crystal oscillator. (Addr:0x82 Bit[4:0]) */
    uint8_t                 xosc_cap_set;   /**< Driver current setting for crystal oscillator. (Addr:0x83 Bit[5:0]) */

    void*                   user;           /**< User defined data. */
} sony_freia_t;

/*
 Config flag definitions. (ORed value should be set to flags argument of Create API.)
*/
/**
 @brief Use external Xtal

        Should be used for slave FREIA in Xtal shared system.
*/
#define SONY_FREIA_CONFIG_EXT_REF                0x80000000

/**
 @brief Disable Xtal in Sleep state.

        Should NOT be used for Xtal shared system.
        Not only for the master FREIA which has Xtal, but also slave FREIA which receive the clock signal.
        Cannot be used with SONY_FREIA_CONFIG_EXT_REF.
*/
#define SONY_FREIA_CONFIG_SLEEP_DISABLEXTAL      0x40000000

/**
 @brief Internal loop filter setting.

        If this is used, internal loop filter is used for
        digital broadcasting system except for DVB-C.
        For analog and DVB-C, external loop filter will be used.
*/
#define SONY_FREIA_CONFIG_LOOPFILTER_INTERNAL    0x10000000

/**
 @brief Satellite Gain Mode setting.
*/
#define SONY_FREIA_CONFIG_SAT_LOW_GAIN_MODE      0x08000000

/**
 @name  Power Save setting for terrestrial.

        These settings are for avoiding steep input impedance change in power save state.
*/
/**@{*/
#define SONY_FREIA_CONFIG_POWERSAVE_TERR_NORMAL_MATCHING_DISABLE    0x00000000 /**< Normal matching disable (default) */
#define SONY_FREIA_CONFIG_POWERSAVE_TERR_NORMAL_MATCHING_ENABLE     0x00400000 /**< Normal matching enable */
#define SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_DISABLE 0x00800000 /**< Keep RF part active, matching disable */
#define SONY_FREIA_CONFIG_POWERSAVE_TERR_RF_ACTIVE_MATCHING_ENABLE  0x00C00000 /**< Keep RF part active, matching enalbe */
#define SONY_FREIA_CONFIG_POWERSAVE_TERR_MASK                       0x00C00000 /**< Do not use this value */
/**@}*/

/**
 @name  Power Save setting for satellite.

        These settings are for avoiding steep input impedance change in power save state.
*/
/**@{*/
#define SONY_FREIA_CONFIG_POWERSAVE_SAT_NORMAL                    0x00000000 /**< Normal Setting (SAT LNA block disable) */
#define SONY_FREIA_CONFIG_POWERSAVE_SAT_RF_ACTIVE                 0x00100000 /**< Keep RF part active */
#define SONY_FREIA_CONFIG_POWERSAVE_SAT_RF_ACTIVE_AGC_FULL        0x00200000 /**< Keep RF part active(AGC Full attenuation) */
#define SONY_FREIA_CONFIG_POWERSAVE_SAT_MASK                      0x00300000 /**< Do not use this value */
/**@}*/

/**
 @name  Maximum IF and IQ output setting. (for terrestrial analog)
*/
/**@{*/
#define SONY_FREIA_CONFIG_OUTLMT_ATV_1_5Vpp      0x00000000  /**< 1.5Vpp (default) */
#define SONY_FREIA_CONFIG_OUTLMT_ATV_1_2Vpp      0x00080000  /**< 1.2Vpp */
/**@}*/

/**
 @name  Maximum IF and IQ output setting. (for terrestrial digital)
*/
/**@{*/
#define SONY_FREIA_CONFIG_OUTLMT_DTV_1_5Vpp      0x00000000  /**< 1.5Vpp (default) */
#define SONY_FREIA_CONFIG_OUTLMT_DTV_1_2Vpp      0x00040000  /**< 1.2Vpp */
/**@}*/

/**
 @name  Maximum IF and IQ output setting. (for satellite)
*/
/**@{*/
#define SONY_FREIA_CONFIG_OUTLMT_STV_0_75Vpp     0x00000000  /**< 0.75Vpp (default) */
#define SONY_FREIA_CONFIG_OUTLMT_STV_0_6Vpp      0x00020000  /**< 0.6Vpp */
/**@}*/

/**
 @name  REFOUT setting related macros.

        If following values are not used, REFOUT output is disabled.
*/
/**@{*/
/**
 @brief REFOUT enable, output level is 500mVp-p.
*/
#define SONY_FREIA_CONFIG_REFOUT_500mVpp         0x00001000
/**
 @brief REFOUT enable, output level is 400mVp-p.
*/
#define SONY_FREIA_CONFIG_REFOUT_400mVpp         0x00002000
/**
 @brief REFOUT enable, output level is 600mVp-p.
*/
#define SONY_FREIA_CONFIG_REFOUT_600mVpp         0x00003000
/**
 @brief REFOUT enable, output level is 800mVp-p.
*/
#define SONY_FREIA_CONFIG_REFOUT_800mVpp         0x00004000
/**
 @brief Internal use only. Do not use this value.
*/
#define SONY_FREIA_CONFIG_REFOUT_MASK            0x00007000
/**@}*/

/**
 @name IF/AGC pin individually setting. (for CXD6866ER)

       Used if sony_freia_terr_adjust_param_t::IFAGCSEL == 0xFF
       Available CXD6866ER only.
       CXD6868ER/CXD6866AER only use IF1/AGC1.
*/
/**@{*/
#define SONY_FREIA_CONFIG_IF2_ATV                0x00000100  /**< IF2 is used for Analog. (Otherwise IF1 is used.) */
#define SONY_FREIA_CONFIG_AGC2_ATV               0x00000200  /**< AGC2 is used for Analog. (Otherwise AGC1 is used.) */
#define SONY_FREIA_CONFIG_IF2_DTV                0x00000400  /**< IF2 is used for Digital. (Otherwise IF1 is used.) */
#define SONY_FREIA_CONFIG_AGC2_DTV               0x00000800  /**< AGC2 is used for Digital. (Otherwise AGC1 is used.) */
/**@}*/

/**
 @name IF/AGC pin setting for normal cases. (for CXD6866ER)

       Used if sony_freia_terr_adjust_param_t::IFAGCSEL == 0xFF
       Available CXD6866ER only.
       CXD6868ER/CXD6866AER only use IF1/AGC1.
*/
/**@{*/
/**
 @brief IF/AGC 1 is used for both Analog and Digital
*/
#define SONY_FREIA_CONFIG_IFAGC_SEL_ALL1          0x00000000
/**
 @brief IF/AGC 2 is used for both Analog and Digital
*/
#define SONY_FREIA_CONFIG_IFAGC_SEL_ALL2          (SONY_FREIA_CONFIG_IF2_ATV | SONY_FREIA_CONFIG_AGC2_ATV | \
                                                   SONY_FREIA_CONFIG_IF2_DTV | SONY_FREIA_CONFIG_AGC2_DTV)
/**
 @brief IF/AGC 1 is used for Analog, 2 is used for Digital
*/
#define SONY_FREIA_CONFIG_IFAGC_SEL_A1D2          (SONY_FREIA_CONFIG_IF2_DTV | SONY_FREIA_CONFIG_AGC2_DTV)
/**
 @brief IF/AGC 1 is used for Digital, 2 is used for Analog
*/
#define SONY_FREIA_CONFIG_IFAGC_SEL_D1A2          (SONY_FREIA_CONFIG_IF2_ATV | SONY_FREIA_CONFIG_AGC2_ATV)

/**
 @name IFOUT DC bias setting.

       Used for SiP2 (CXD6820 family) configuration.
*/
/**@{*/
#define SONY_FREIA_CONFIG_IFOUT_DC_BIAS_750mV     0x00000000  /**< 750mV (default) */
#define SONY_FREIA_CONFIG_IFOUT_DC_BIAS_500mV     0x00000080  /**< 500mV */
/**@}*/

/*------------------------------------------------------------------------------
  APIs
------------------------------------------------------------------------------*/
/**
 @brief Set up the FREIA tuner driver.

        This MUST be called before calling sony_freia_Initialize.

 @param pTuner      Reference to memory allocated for the FREIA instance.
                    The create function will setup this FREIA instance.
 @param i2cAddress  The FREIA tuner I2C slave address in 8-bit form.
 @param pI2c        The I2C APIs that the FREIA driver will use as the I2C interface.
 @param flags       Configuration flags. It should be ORed value of SONY_FREIA_CONFIG_XXXX.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_Create(sony_freia_t *pTuner, uint8_t i2cAddress, sony_i2c_t *pI2c, uint32_t flags);

/**
 @brief Initialize the FREIA tuner.

        This MUST be called before calling sony_freia_terr_Tune and sony_freia_sat_Tune.

 @param pTuner       The FREIA tuner instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_Initialize(sony_freia_t *pTuner);

/**
 @brief Tune to a given RF frequency with terrestrial broadcasting system.

        To complete tuning, sony_freia_terr_TuneEnd should be called after waiting 50ms.

 @param pTuner       The FREIA tuner instance.
 @param frequencykHz RF frequency to tune. (kHz)
 @param tvSystem     The type of broadcasting system to tune.

 @return SONY_RESULT_OK if tuner successfully tuned.
*/
sony_result_t sony_freia_terr_Tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem);

/**
 @brief Completes the terrestrial tuning sequence.

        This MUST be called after calling sony_freia_terr_Tune and 50ms wait.

 @param pTuner       The FREIA tuner instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_terr_TuneEnd(sony_freia_t *pTuner);

/**
 @brief Tune to a given RF frequency with satellite broadcasting system.

 @param pTuner         The FREIA tuner instance.
 @param frequencykHz   RF frequency to tune. (kHz)
 @param tvSystem       The type of broadcasting system to tune.
 @param symbolRateksps Symbol rate to tune. (ksps)

 @return SONY_RESULT_OK if tuner successfully tuned.
*/
sony_result_t sony_freia_sat_Tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem, uint32_t symbolRateksps);

/**
 @brief Tune to a given RF frequency with satellite broadcasting system delivered by cable broadcast.

        To complete tuning, sony_freia_terr_IQ_TuneEnd should be called after waiting 50ms.

 @param pTuner       The FREIA tuner instance.
 @param frequencykHz RF frequency to tune. (kHz)
 @param tvSystem     The type of broadcasting system to tune.
 @param symbolRateksps Symbol rate to tune. (ksps)

 @return SONY_RESULT_OK if tuner successfully tuned.
*/
sony_result_t sony_freia_terr_IQ_Tune(sony_freia_t *pTuner, uint32_t frequencykHz,
    sony_freia_tv_system_t tvSystem, uint32_t symbolRateksps);

/**
 @brief Completes the terrestrial tuning sequence with IQ output.

        This MUST be called after calling sony_freia_terr_Tune and 50ms wait.

 @param pTuner       The FREIA tuner instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_terr_IQ_TuneEnd(sony_freia_t *pTuner);


/**
 @brief Shift RF frequency.

        This API shift RF frequency without VCO calibration.
        This API is normally useful for analog TV "fine tuning" that
        shift RF frequency without picture distortion.

 @note  Please check the frequency range that VCO calibration is unnecessary.
        (See hardware specification sheet.)

 @param pTuner       The FREIA tuner instance.
 @param frequencykHz RF frequency to tune. (kHz)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_ShiftFRF(sony_freia_t *pTuner, uint32_t frequencykHz);

/**
 @brief Put the FREIA tuner into Sleep state.

        From this state the tuner can be directly tuned by calling sony_freia_Tune.

 @param pTuner       The FREIA tuner instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_Sleep(sony_freia_t *pTuner);

/**
 @brief Write to GPO0 or GPO1.

 @param pTuner       The FREIA tuner instance.
 @param id           Pin ID 0 = GPO0, 1 = GPO1
 @param val          Output logic level, 0 = Low, 1 = High

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_SetGPO(sony_freia_t *pTuner, uint8_t id, uint8_t val);

/**
 @brief Read GPI0 or GPI1 value.

 @param pTuner       The FREIA tuner instance.
 @param id           Pin ID 0 = GPI0, 1 = GPI1
 @param pVal         GPI logic level, 0 = Low, 1 = High

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_GetGPI(sony_freia_t *pTuner, uint8_t id, uint8_t *pVal);

/**
 @brief Set the RF external circuit control pin. (for terrestrial)
        (Set RF_EXT_CTRL register.)

 @param pTuner       The FREIA tuner instance.
 @param enable       The value to set. (Enable(1) or Disable(0))

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_SetRfExtCtrl(sony_freia_t *pTuner, uint8_t enable);

/**
 @brief Set UK Freesat mode.

        If this mode is enabled, Freesat optimized setting will be used in sony_freia_sat_Tune.
        If internal loop filter is used, (SONY_FREIA_CONFIG_LOOPFILTER_INTERNAL is ORed)
        UK Freesat mode has no effect.

 @param pTuner       The FREIA tuner instance.
 @param enable       The value to set. (Enable(1) or Disable(0))

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_SetFreesatMode(sony_freia_t *pTuner, uint8_t enable);

/**
 @brief Set External OVLD TC mode.

        If this mode is enabled, OVLD TC optimized setting will be used in sony_freia_sat_Tune.

 @param pTuner       The FREIA tuner instance.
 @param enable       The value to set. (Enable(1) or Disable(0))

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_SetExternalOvldTcMode(sony_freia_t *pTuner, uint8_t enable);

/**
 @brief Set AGC Limiter.

        Set the AGC limiter setting to Normal or not.

 @param pTuner       The FREIA tuner instance.
 @param setting      The value to set. (Normal(0) or Not Normal(1))

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_SetAgcLimiter(sony_freia_t *pTuner, uint8_t setting);

/**
 @brief Set Q Dump.

        Set the Q_Dump Off/On.

 @param pTuner       The FREIA tuner instance.
 @param setting      The value to set. (Off(0) or On(1))

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_SetQDump(sony_freia_t *pTuner, uint8_t setting);

/**
 @brief Read the RSSI in dBm from the tuner. (for terrestrial)

 @note  Target level of IF signal output (depend on demodulator)
        is not added in default.

 @param pTuner          The FREIA tuner instance.
 @param pRssi           RSSI in dBm * 100

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_terr_ReadRssi(sony_freia_t *pTuner, int32_t *pRssi);

/**
 @brief Read the RSSI in dBm from the tuner. (for satellite)

 @note  Target level of IF signal output (depend on demodulator)
        is not added in default.

 @param pTuner          The FREIA tuner instance.
 @param pRssi           RSSI in dBm * 100

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_sat_ReadRssi(sony_freia_t *pTuner, int32_t *pRssi);

/**
 @brief Read the RSSI in dBm from the tuner. (for terrestrial IQ)

 @note  Target level of IF signal output (depend on demodulator)
        is not added in default.

 @param pTuner          The FREIA tuner instance.
 @param pRssi           RSSI in dBm * 100

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_terr_IQ_ReadRssi(sony_freia_t *pTuner, int32_t *pRssi);

/**
 @brief RF filter compensation setting.
        (Please see RFVGA Description of datasheet.)

        New setting will become effective after next tuning.

        mult = coeff / 128
        (compensated value) = (original value) * mult + offset

 @param pTuner       The FREIA tuner instance.
 @param vlCoeff      Multiplier value. (8bit unsigned)
 @param vlOffset     Additional term. (8bit 2s complement)
 @param uvhCoeff     Multiplier value. (8bit unsigned)
 @param uvhOffset    Additional term. (8bit 2s complement)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_freia_RFFilterConfig(sony_freia_t *pTuner, uint8_t vlCoeff, uint8_t vlOffset,
       uint8_t uvhCoeff, uint8_t uvhOffset);

#endif /* SONY_FREIA_H */
