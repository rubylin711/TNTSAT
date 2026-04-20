/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_FRONTEND_H__
#define __MT_UNF_FRONTEND_H__


#include "mt_common.h"

//#include "mt_unf_i2c.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#if 1

#define MT_I2C_MAX_NUM_USER (4) /** Maximum I2C channel ID*/                     /** CNcomment:最大I2C通道号*/
#define MT_UNF_DISEQC_MSG_MAX_LENGTH (8) /** DiSEqC message length*/             /** CNcomment:DiSEqC消息长度*/
#define MT_UNF_DISEQC_MAX_REPEAT_TIMES (4) /** DiSEqC message max repeat times*/ /** CNcomment:DiSEqC消息最大重复发送次数*/
#define MAX_TS_LINE 3 /** The NO of ts lines that can be configured*/            /** CNcomment:可配置的ts信号线数量*/
#define TER_MAX_TP (20)

#define DISEQC_MAX_MOTOR_PISITION (255) /** DiSEqC motor max stored position*/ /** CNcomment:DiSEqC马达最大存储星位个数*/

/*************************** Structure Definition ****************************/
/** \addtogroup      FRONTEND */
/** @{ */ /** <!-- [FRONTEND] */

#define MAX_BS_TP_NUM_PER_SAT 512
#define DIRECTTV_SCID_FILTER_MAX_COUNT     15

/*!
@~english
@brief Error code
*/
typedef enum _unf_fe_error_t {
    /*!
    Did not call the MT_UNF_TUNER_Init
    */
    MT_UNF_TUNER_ERR_UNINIT = -0x100,
    /*!
    More than the largest tuner ID
    */
    MT_UNF_TUNER_ERR_ID_OVER = -0x101,
    /*!
    Did not call the MT_UNF_TUNER_SetAttr
    */
    MT_UNF_TUNER_ERR_ATTR_UNSET = -0x102,
    /*!
    MT_UNF_TUNER_Open call failed
    */
    MT_UNF_TUNER_ERR_OPEN_FAIL = -0x103,
    /*!
    MT_UNF_TUNER_Close call failed
    */
    MT_UNF_TUNER_ERR_CLOSE_FAIL = -0x104,
    /*!
    @~chinese
    没有调用MT_UNF_TUNER_Open接口
    @~english
    Did not call the MT_UNF_TUNER_Open
    */
    MT_UNF_TUNER_ERR_UNOPEN = -0x105,
    /*!
    @~chinese
    锁频失败
    @~english
    Locking failure
    */
    MT_UNF_TUNER_ERR_CONNECT_FAIL = -0x106,
    /*!
    @~chinese
    获取默认超时值失败
    @~english
    Gets the default timeout value failed
    */
    MT_UNF_TUNER_ERR_DEF_TIMEOUT_FAIL = -0x107,
    /*!
    @~chinese
    获取TUNER状态失败
    @~english
    Failed to obtain Tuner status
    */
    MT_UNF_TUNER_ERR_GET_STATUS_FAIL = -0x108,
    /*!
    @~chinese
    获取误码率失败
    @~english
    Gets error rate failed
    */
    MT_UNF_TUNER_ERR_GET_BER_FAIL = -0x109,
    /*!
    @~chinese
    获取信噪比失败
    @~english
    Gets signal noise ratio failed
    */
    MT_UNF_TUNER_ERR_GET_SNR_FAIL = -0x10a,
    /*!
    @~chinese
    获取信号强度失败
    @~english
    Gets signal strengh failed
    */
    MT_UNF_TUNER_ERR_GET_SIG_STRENGTH_FAIL = -0x10b,
    /*!
    @~chinese
    获取信号质量失败
    @~english
    Gets signal quality failed
    */
    MT_UNF_TUNER_ERR_GET_SIG_QUALITY_FAIL = -0x10c,
    /*!
    @~chinese
    低功耗失败
    @~english
    Enter standby failure
    */
    MT_UNF_TUNER_ERR_STANDBY_FAIL = -0x10d,
    /*!
    @~chinese
    退出低功耗失败
    @~english
    Exit standby failure
    */
    MT_UNF_TUNER_ERR_WAKEUP_FAIL = -0x10e,
    /*!
    @~chinese
    设置LNB电压开关失败
    @~english
    Set LNB voltage switch failure
    */
    MT_UNF_TUNER_ERR_SET_LNBONOFF = -0x10f,
    MT_UNF_TUNER_ERR_CHECK_LNB_SC_PROTECT = -0x110,
    MT_UNF_TUNER_ERR_SET_22KONOFF = -0x111,
    MT_UNF_TUNER_ERR_SET_POLORIZATION = -0x112,
    MT_UNF_TUNER_ERR_BLINDSCAN = -0x113,
    MT_UNF_TUNER_ERR_CHECK_BLIND_SCAN_STATUS = -0x114,
    MT_UNF_TUNER_ERR_BLIND_SCAN_CANCEL = -0x115,
    MT_UNF_TUNER_ERR_GET_BLIND_SCAN_NOTIFY = -0x116,
    MT_UNF_TUNER_ERR_DISEQC_CTRL = -0x117,
    MT_UNF_TUNER_ERR_SET_CHANNEL_INFO = -0x118,
    MT_UNF_TUNER_ERR_GET_ELECTRICAL_LEVEL = -0x119,
    MT_UNF_TUNER_ERR_GET_BIT_RATE = -0x11A,
} mt_unf_fe_error_t;

/** Sample data, complex format*/
/** CNcomment: 采集数据, 复格式*/
typedef struct mtunf_fe_sample_data_t
{
    mt_s32 data_ip; /*sample data, i component*/ /**<CNcomment:采集数据的I分量*/
    mt_s32 data_qp; /*sample data, q component*/ /**<CNcomment:采集数据的Q分量*/
} mt_unf_fe_sample_data_t;

/** Sample data length*/
/** CNcomment: 采数长度*/
typedef enum mtunf_fe_sample_datalen_t {
    MT_UNF_FE_SAMPLE_DATALEN_32,  /*sample 32 pts*/
    MT_UNF_FE_SAMPLE_DATALEN_64,  /*sample 64 pts*/
    MT_UNF_FE_SAMPLE_DATALEN_128, /*sample 128 pts*/
    MT_UNF_FE_SAMPLE_DATALEN_256, /*sample 256 pts*/
    MT_UNF_FE_SAMPLE_DATALEN_512,
    /*sample 512 pts*/ /**<CNcomment:采集512点*/
    MT_UNF_FE_SAMPLE_DATALEN_1024,
    /*sample 1024 pts*/ /**<CNcomment:采集1024点*/
    MT_UNF_FE_SAMPLE_DATALEN_2048,
    /*sample 2048 pts*/                               /**<CNcomment:采集2048点*/
    MT_UNF_FE_SAMPLE_DATALEN_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_sample_datalen_t;

/** Modulation mode*/
/** CNcomment: 调制方式*/
typedef enum mtunf_qam_type_t {
    MT_UNF_MOD_TYPE_DEFAULT,
    /**<Default QAM mode. The default QAM mode is MT_UNF_MOD_TYPE_QAM_64 at present.*/ /**<CNcomment:默认的QAM类型, 当前系统默认为MT_UNF_MOD_TYPE_QAM_64 */
    MT_UNF_MOD_TYPE_QAM_4 = 0x80,		/* Mainly for DTMB & CTTB (DMB-T, ADTB-T)  */
    /**<Enumeration corresponding to the 4QAM mode*/ /**<CNcomment:4QAM对应的枚举值*/
    MT_UNF_MOD_TYPE_QAM_4_NR,
    /**<Enumeration corresponding to the 4QAM NR mode*/ /**<CNcomment:4QAM NR对应的枚举值*/

    MT_UNF_MOD_TYPE_QAM_16 = 0x100,		/* Mainly for DVB-C, T, T2 */
    /**<Enumeration corresponding to the 16QAM mode*/ /**<CNcomment:16QAM对应的枚举值*/
    MT_UNF_MOD_TYPE_QAM_32,
    /**<Enumeration corresponding to the 32QAM mode*/ /**<CNcomment:32QAM对应的枚举值*/
    MT_UNF_MOD_TYPE_QAM_64,
    /**<Enumeration corresponding to the 64QAM mode*/ /**<CNcomment:64QAM对应的枚举值*/
    MT_UNF_MOD_TYPE_QAM_128,
    /**<Enumeration corresponding to the 128QAM mode*/ /**<CNcomment:128QAM对应的枚举值*/
    MT_UNF_MOD_TYPE_QAM_256,
    /**<Enumeration corresponding to the 256QAM mode*/ /**<CNcomment:256QAM对应的枚举值*/
    MT_UNF_MOD_TYPE_QAM_512,
    /**<Enumeration corresponding to the 512QAM mode*/ /**<CNcomment:512QAM对应的枚举值*/

    MT_UNF_MOD_TYPE_BPSK = 0x200,

    /**<Enumeration corresponding to the binary phase shift keying (BPSK) mode. */ /**<CNcomment:BPSK对应的枚举值*/
    MT_UNF_MOD_TYPE_QPSK = 0x300,		/* Mainly for DVB-S, S2 and S2X */
    /**<Enumeration corresponding to the quaternary phase shift keying (QPSK) mode. */ /**<CNcomment:QPSK对应的枚举值*/
    MT_UNF_MOD_TYPE_DQPSK,
    MT_UNF_MOD_TYPE_8PSK,
    /**<Enumeration corresponding to the 8 phase shift keying (8PSK) mode*/ /**<CNcomment:8PSK对应的枚举值*/
    MT_UNF_MOD_TYPE_16APSK,
    /**<Enumeration corresponding to the 16-Ary Amplitude and Phase Shift Keying (16APSK) mode*/ /**<CNcomment:16APSK对应的枚举值*/
    MT_UNF_MOD_TYPE_32APSK,
    /**<Enumeration corresponding to the 32-Ary Amplitude and Phase Shift Keying (32APSK) mode*/ /**<CNcomment:32APSK对应的枚举值*/

    MT_UNF_MOD_TYPE_64APSK = 0x380,		/* Mainly for DVB-S2X */
    /**<Enumeration corresponding to the 64-Ary Amplitude and Phase Shift Keying (64APSK) mode*/ /**<CNcomment:64APSK对应的枚举值*/
    MT_UNF_MOD_TYPE_128APSK,
    /**<Enumeration corresponding to the 128-Ary Amplitude and Phase Shift Keying (128APSK) mode*/ /**<CNcomment:128APSK对应的枚举值*/
    MT_UNF_MOD_TYPE_256APSK,
    /**<Enumeration corresponding to the 256-Ary Amplitude and Phase Shift Keying (256APSK) mode*/ /**<CNcomment:256APSK对应的枚举值*/
    MT_UNF_MOD_TYPE_8APSK_L,
    /**<Enumeration corresponding to the 8 Phase Shift Keying (8PSK_L) mode*/ /**<CNcomment:8APSK_L对应的枚举值*/
    MT_UNF_MOD_TYPE_16APSK_L,
    /**<Enumeration corresponding to the 16-Ary Amplitude and Phase Shift Keying (16APSK_L) mode*/ /**<CNcomment:16APSK_L对应的枚举值*/
    MT_UNF_MOD_TYPE_32APSK_L,
    /**<Enumeration corresponding to the 32-Ary Amplitude and Phase Shift Keying (32APSK_L) mode*/ /**<CNcomment:32APSK_L对应的枚举值*/
    MT_UNF_MOD_TYPE_64APSK_L,
    /**<Enumeration corresponding to the 64-Ary Amplitude and Phase Shift Keying (64APSK_L) mode*/ /**<CNcomment:64APSK_L对应的枚举值*/
    MT_UNF_MOD_TYPE_128APSK_L,
    /**<Enumeration corresponding to the 128-Ary Amplitude and Phase Shift Keying (128APSK_L) mode*/ /**<CNcomment:128APSK_L对应的枚举值*/
    MT_UNF_MOD_TYPE_256APSK_L,
    /**<Enumeration corresponding to the 256-Ary Amplitude and Phase Shift Keying (256APSK_L) mode*/ /**<CNcomment:256APSK_L对应的枚举值*/

    MT_UNF_MOD_TYPE_8VSB = 0x400,		/* Mainly for ATSC */
    /**<Enumeration corresponding to (8VSB) mode*/ /**<CNcomment:8VSB对应的枚举值*/
    MT_UNF_MOD_TYPE_16VSB,
    /**<Enumeration corresponding to (16VSB) mode*/ /**<CNcomment:16VSB对应的枚举值*/

    /**<Enumeration corresponding to the auto mode. For DVB-S/S2, if detect modulation type fail, it will return auto*/
    /**<CNcomment:卫星信号调制方式自动检测，如果检测失败返回AUTO*/
    MT_UNF_MOD_TYPE_AUTO,
    MT_UNF_MOD_TYPE_BUTT /**<Invalid Modulation mode*/ /**<CNcomment:非法的调制类型枚举值*/
} mt_unf_modulation_type_t;

/** Frequency locking status of the tuner*/
/** CNcomment:TUNER锁频状态*/
typedef enum mtunf_fe_lock_status_t {
    MT_UNF_FE_SIGNAL_DROPPED = 0,
    /**<The signal is not locked.*/ /**<CNcomment:信号未锁定*/
    MT_UNF_FE_SIGNAL_LOCKED,
    /**<The signal is locked.*/               /**<CNcomment:信号已锁定*/
    MT_UNF_FE_SIGNAL_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_lock_status_t;


/** Frequency locking status of the tuner*/
/** CNcomment:TUNER unlock reason*/
typedef enum mtunf_fe_unlock_reason_t {
    MT_UNF_FE_STATE_UNLOCKED = 0,
    /**<The state is unlocked.*/ /**<CNcomment:信号未锁定*/
    MT_UNF_FE_STATE_WAITING,
    /**<The state is waiting signal to lock or unlock.*/               /**<CNcomment:信号正在try*/
    MT_UNF_FE_STATE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_unlock_reason_t;


/** Output mode of the tuner*/
/** CNcomment:TUNER输出模式*/
typedef enum unf_fe_output_mode_t {
    MT_UNF_FE_OUTPUT_MODE_DEFAULT,
    /**<Default mode*/              /**<CNcomment:默认模式*/
    MT_UNF_FE_OUTPUT_MODE_PARALLEL, //!<@~chinese TS并行输出模式//!< @~english TS parallel output mode
    MT_UNF_FE_OUTPUT_MODE_PARALLEL_MODE_A,
    /**<Parallel mode A*/ /**<CNcomment:并行模式A*/
    MT_UNF_FE_OUTPUT_MODE_PARALLEL_MODE_B,
    /**<Parallel mode B*/ /**<CNcomment:并行模式B*/
    MT_UNF_FE_OUTPUT_MODE_SERIAL,
    /**<Serial mode 74.25M*/ /**<CNcomment:串行模74.25M*/
    MT_UNF_FE_OUTPUT_MODE_SERIAL_50,
    /**<Serial mode 50M*/ /**<CNcomment:串行模50M*/
    MT_UNF_FE_OUTPUT_MODE_SERIAL_2BIT,
    /**<Serial mode witch 2 bit data line*/        /**<CNcomment:2bit串行*/
    MT_UNF_FE_OUTPUT_MODE_BUTT /**<Invalid value*/ /**<CNcomment:非法值*/
} mt_unf_fe_output_mode_t;

/** Output order*/
/** CNcomment:TUNER输出线序 */
typedef enum unf_fe_output_ts_t {
    MT_UNF_FE_OUTPUT_TSDAT0,
    /**<data0*/ /**<CNcomment:数据线0bit*/
    MT_UNF_FE_OUTPUT_TSDAT1,
    /**<data1*/ /**<CNcomment:数据线1bit*/
    MT_UNF_FE_OUTPUT_TSDAT2,
    /**<data2*/ /**<CNcomment:数据线2bit*/
    MT_UNF_FE_OUTPUT_TSDAT3,
    /**<data3*/ /**<CNcomment:数据线3bit*/
    MT_UNF_FE_OUTPUT_TSDAT4,
    /**<data4*/ /**<CNcomment:数据线4bit*/
    MT_UNF_FE_OUTPUT_TSDAT5,
    /**<data5*/ /**<CNcomment:数据线5bit*/
    MT_UNF_FE_OUTPUT_TSDAT6,
    /**<data6*/ /**<CNcomment:数据线6bit*/
    MT_UNF_FE_OUTPUT_TSDAT7,
    /**<data7*/ /**<CNcomment:数据线7bit*/
    MT_UNF_FE_OUTPUT_TSSYNC,
    /**<sync*/ /**<CNcomment:sync信号线*/
    MT_UNF_FE_OUTPUT_TSVLD,
    /**<valid*/ /**<CNcomment:valid信号线*/
    MT_UNF_FE_OUTPUT_TSERR,
    /**<err*/                                 /**<CNcomment:err信号线*/
    MT_UNF_FE_OUTPUT_BUTT /**<Invalid value*/ /**<CNcomment:非法值*/
} mt_unf_fe_output_ts_t;

/** ts output port order*/
/** CNcomment:ts接口输出线序 */
typedef struct unf_fe_tsout_t
{
    mt_unf_fe_output_ts_t ts_output[MAX_TS_LINE]; /**<ts output port order*/ /**<CNcomment:ts接口输出线序*/
} mt_unf_fe_ts_out_t;

/** Signal type of the tuner*/
/** CNcomment:TUNER信号类型*/
typedef enum mtunf_fe_sig_type_t {
    MT_UNF_FE_SIG_TYPE_CAB = 1,
    /**<Cable signal*/ /**<CNcomment:ITU-T J.83 ANNEX A/C(DVB_C)信号信号*/
    MT_UNF_FE_SIG_TYPE_SAT = 2,
    /**<Satellite signal*/ /**<CNcomment:卫星信号*/
    MT_UNF_FE_SIG_TYPE_SAT_2 = 4,
    /**<Satellite signal*/ /**<CNcomment:卫星信号*/
    MT_UNF_FE_SIG_TYPE_DVB_T = 8,
    /**<Terrestrial signal*/ /**<CNcomment:地面信号*/
    MT_UNF_FE_SIG_TYPE_DVB_T2 = 16,
    /**<Terrestrial signal*/ /**<CNcomment:地面信号*/
    MT_UNF_FE_SIG_TYPE_ISDB_T = 32,
    /**<Terrestrial signal*/ /**<CNcomment:地面信号*/
    MT_UNF_FE_SIG_TYPE_ATSC_T = 64,
    /**<Terrestrial signal*/ /**<CNcomment:地面信号*/
    MT_UNF_FE_SIG_TYPE_DTMB = 128,
    /**<Terrestrial signal*/ /**<CNcomment:地面信号*/
    MT_UNF_FE_SIG_TYPE_J83B = 256,
    /**<Cable signal*/ /**<CNcomment:ITU-T J.83 ANNEX B(US Cable)信号*/
    MT_UNF_FE_SIG_TYPE_CTTB = 512,
    MT_UNF_FE_SIG_TYPE_DVBT_AUTO = 1024,
    MT_UNF_FE_SIG_TYPE_DVBS_AUTO = 2048,
    MT_UNF_FE_SIG_TYPE_DIRECTV = 4096,
    /**<DSS signal*/ /**<CNcomment:DirecTV DSS信号*/
    MT_UNF_FE_SIG_TYPE_BUTT /**<Invalid value*/ /**<CNcomment:非法值*/
} mt_unf_fe_sig_type_t;

/** type of TUNER device*/
/** CNcomment:TUNER设备类型*/
typedef enum unf_tuner_type_t {
    MT_UNF_TUNER_TYPE_M88TC2800,          //  0 !< M88TC2800, DVB-C
    MT_UNF_TUNER_TYPE_M88TC3800_FOR_WASU, //  1 !< M88TC3800 for WASU, DVB-C
    MT_UNF_TUNER_TYPE_R836,               //  2 !< R836, DVB-C
    MT_UNF_TUNER_TYPE_TDA18250A,          //  3 !< TDA18250A, DVB-C
    MT_UNF_TUNER_TYPE_MXL_608,            //  4 !< MxL608, DVB-C
    MT_UNF_TUNER_TYPE_M88TC3800,          //  5 !< M88TC3800, DVB-C
    MT_UNF_TUNER_TYPE_M88TC6800,          //  6 !< M88TC6800, DVB-C
    MT_UNF_TUNER_TYPE_XG_3BL,             //  7
    /**<XG_3BL*/ /**<CNcomment:支持旭光射频芯片*/
    MT_UNF_TUNER_TYPE_CD1616,
    /**<CD1616*/ /**<CNcomment:支持CD1616射频芯片*/
    MT_UNF_TUNER_TYPE_ALPS_TDAE,
    /**<ALPS_TDAE*/ /**<CNcomment:支持ALPS_TDAE射频芯片*/
    MT_UNF_TUNER_TYPE_TDCC,               // 10
    /**<TDCC*/ /**<CNcomment:支持TDCC射频芯片*/
    MT_UNF_TUNER_TYPE_TDA18250,
    /**<TDA18250*/ /**<CNcomment:支持TDA18250射频芯片*/
    MT_UNF_TUNER_TYPE_CD1616_DOUBLE,
    /**<CD1616 with double agc*/ /**<CNcomment:支持CD1616带双agc射频芯片*/
    MT_UNF_TUNER_TYPE_MT2081,
    /**<MT2081*/ /**<CNcomment:支持MT2081射频芯片*/
    MT_UNF_TUNER_TYPE_TMX7070X,
    /**<THOMSON7070X*/ /**<CNcomment:支持THOMSON7070X射频芯片*/
    MT_UNF_TUNER_TYPE_R820C,
    /**<R820C*/ /**<CNcomment:支持R820C射频芯片*/
    MT_UNF_TUNER_TYPE_MXL203,             // 16
    /**<MXL203 */ /**<CNcomment:支持MXL203射频芯片*/
    MT_UNF_TUNER_TYPE_AV2011,             // 17
    /**<AV2011*/ /**<CNcomment:支持AV2011射频芯片*/
    MT_UNF_TUNER_TYPE_SHARP7903,
    /**<SHARP7903*/ /**<CNcomment:支持SHARP7903射频芯片*/
    MT_UNF_TUNER_TYPE_MXL101,
    /**<MXL101*/ /**<CNcomment:支持MXL101射频芯片*/
    MT_UNF_TUNER_TYPE_MXL603,             // 20
    /**<MXL603*/ /**<CNcomment:支持MXL603射频芯片*/
    MT_UNF_TUNER_TYPE_IT9170,
    /**<IT9170*/ /**<CNcomment:支持IT9170射频芯片*/
    MT_UNF_TUNER_TYPE_IT9133,
    /**<IT9133*/ /**<CNcomment:支持IT9133射频芯片*/
    MT_UNF_TUNER_TYPE_TDA6651,
    /**<TDA6651*/ /**<CNcomment:支持TDA6651射频芯片*/
    MT_UNF_TUNER_TYPE_TDA18250B,
    /**<TDA18250B*/ /**<CNcomment:支持TDA18250B射频芯片*/
    MT_UNF_TUNER_TYPE_RDA5815,            // 25
    /**<RDA5815*/ /**<CNcomment:支持RDA5815射频芯片*/
    MT_UNF_TUNER_TYPE_MXL254,
    /**<MXL254*/ /**<CNcomment:支持MXL254射频芯片*/
    MT_UNF_TUNER_TYPE_CXD2861,
    /**<CXD2861*/ /**<CNcomment:支持CXD2861射频芯片*/
    MT_UNF_TUNER_TYPE_SI2147,
    /**<Si2147*/ /**<CNcomment:支持Si2147射频芯片*/
    MT_UNF_TUNER_TYPE_RAFAEL836,          // 29
    /**<Rafael836*/ /**<CNcomment:支持Rafael836射频芯片*/
    MT_UNF_TUNER_TYPE_MXL608,             // 30
    /**<MXL608*/ /**<CNcomment:支持MXL608射频芯片*/
    MT_UNF_TUNER_TYPE_MXL214,
    /**<MXL214*/ /**<CNcomment:支持MXL214射频芯片*/
    MT_UNF_TUNER_TYPE_TDA18280,
    /**<TDA18280*/ /**<CNcomment:支持TDA18280射频芯片*/
    MT_UNF_TUNER_TYPE_M88TS2022       = 33,      // 33
    MT_UNF_TUNER_TYPE_M88TS6011       = 34,      // 34
    MT_UNF_TUNER_DEV_TYPE_M88TC6920   = 35,      // 35  Montage-LZ DVB-C, dual tuner
    MT_UNF_TUNER_DEV_TYPE_M88TC6930,             // 36  Montage-LZ DVB-C, dual tuner
    MT_UNF_TUNER_DEV_TYPE_M88TC6960,             // 37  Montage-LZ DVB-C, three tuner
    MT_UNF_TUNER_DEV_TYPE_M88RC6800,             // 38  Montage-LZ DVB-C
    MT_UNF_TUNER_DEV_TYPE_M88RC6900,             // 39  Montage-LZ DVB-C
    MT_UNF_TUNER_TYPE_R850            = 50,      // 50  Rafael R850, DVB-C/T/T2, J83B, single tuner
    MT_UNF_TUNER_TYPE_R858C           = 51,      // 51  Rafael R858C, DVB-C/T/T2, J83B, dual tuner
    MT_UNF_TUNER_TYPE_RT720           = 61,      // 61  Rafael RT720, DVB-S/S2, single tuner
    MT_UNF_TUNER_TYPE_M88RS6060       = 80,      // 80  Montage-LZ DVB-S/S2, single tuner part of receiver M88RS6060

    MT_UNF_TUNER_TYPE_SHARP6306       = 90,      // 90  Sharp 6306, DVB-S/S2, signle tuner
    MT_UNF_TUNER_TYPE_SHARP7306       = 91,      // 91  Sharp 7306, DVB-S/S2, signle tuner
    MT_UNF_TUNER_TYPE_SHARP6903       = 92,      // 92  Sharp 6903, DVB-S/S2, signle tuner
    MT_UNF_TUNER_TYPE_RDA5812         = 95,      // 95  RDA 5812, DVB-S/S2, single tuner
    MT_UNF_TUNER_TYPE_RDA5815S        = 96,      // 96  RDA 5815S, DVB-S/S2, single tuner
    MT_UNF_TUNER_TYPE_RDA5815M        = 97,      // 97  RDA 5815M, DVB-S/S2, single tuner
    MT_UNF_TUNER_TYPE_S305            = 98,      // 98  S305, DVB-S/S2, single tuner
    MT_UNF_TUNER_TYPE_AV2018          = 99,      // 99  AV2018, DVB-S/S2, single tuner
    MT_UNF_TUNER_TYPE_AV2020          = 100,     // 100 AV2020, DVB-S/S2, single tuner
    MT_UNF_TUNER_TYPE_AV2022          = 101,     // 101 AV2022, DVB-S/S2, single tuner
    MT_UNF_TUNER_TYPE_AV2012          = 102,     // 102 AV2012, DVB-S/S2, single tuner

    MT_UNF_TUNER_TYPE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_tuner_type_t;

/** Type of the demod device*/
/** CNcomment:demod设备类型*/
typedef enum unf_demod_type_t {
    MT_UNF_DEMOD_TYPE_NONE,
    /**<Not supported*/ /**<CNcomment:不支持*/
                        //MT_UNF_DEMOD_TYPE_3130I = 0x100,        /**<Internal QAM*/          /**<CNcomment:内部QAM*/
    MT_UNF_DEMOD_TYPE_M88DD3K = 0x100,
    MT_UNF_DEMOD_DEV_TYPE_M88CC6000,     //!<M88CC6000 Demod, DVB-C
    MT_UNF_DEMOD_DEV_TYPE_M88DC2800,     //!<M88DC2800, DVB-C
    MT_UNF_DEMOD_DEV_TYPE_M88DD3K,       //!<M88DD3K, DVB-C & DTMB(CTTB)
    MT_UNF_DEMOD_DEV_TYPE_M88DM6K,       // 0x104 = 260
    MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT, // 0x105 = 261, Montage-LZ, Sym1 internel demod, DVB-S, DVB-S2
    MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB, // 0x106 = 262, Montage-LZ, Sym1 internel demod, DVB-C
    MT_UNF_DEMOD_TYPE_DS3103,            // 0x107 = 263
    MT_UNF_DEMOD_TYPE_3130E,
    /**<External Hi3130*/ /**<CNcomment:外部QAM hi3130芯片*/
    MT_UNF_DEMOD_TYPE_3130I,
    /**<External Hi3130*/ /**<CNcomment:外部QAM hi3130芯片*/
    MT_UNF_DEMOD_TYPE_J83B,
    /**<suppoort j83b*/ /**<CNcomment:支持j83b*/
    MT_UNF_DEMOD_TYPE_AVL6211,
    /**<Avalink 6211*/ /**<CNcomment:支持Avalink 6211*/
    MT_UNF_DEMOD_TYPE_MXL101,
    /**<Maxlinear mxl101*/ /**<CNcomment:支持Maxlinear mxl101*/
    MT_UNF_DEMOD_TYPE_MN88472,
    /**<PANASONIC mn88472*/ /**<CNcomment:支持PANASONIC mn88472*/
    MT_UNF_DEMOD_TYPE_IT9170,
    /**<ITE it9170*/ /**<CNcomment:支持ITE it9170*/
    MT_UNF_DEMOD_TYPE_IT9133,
    /**<ITE it9133*/ /**<CNcomment:支持ITE it9133*/
    MT_UNF_DEMOD_TYPE_3136,
    /**<External Hi3136*/ /**<CNcomment:外部hi3136芯片*/
    MT_UNF_DEMOD_TYPE_3136I,
    /**<Internal Hi3136*/ /**<CNcomment:内部hi3136芯片*/
    MT_UNF_DEMOD_TYPE_MXL254,
    /**<External mxl254*/ /**<CNcomment:外部MXL254芯片*/
    MT_UNF_DEMOD_TYPE_CXD2837,
    /**Sony cxd2837*/ /**<CNcomment:支持sony cxd2837*/
    MT_UNF_DEMOD_TYPE_3137,
    /**External Hi3137*/ /**<CNcomment:支持外部hi3137芯片*/
    MT_UNF_DEMOD_TYPE_MXL214,
    /**<External mxl214*/ /**<CNcomment:外部MXL214芯片*/
    MT_UNF_DEMOD_TYPE_TDA18280,
    /**<External tda18280*/ /**<CNcomment:外部tda18280芯片*/
    MT_UNF_DEMOD_TYPE_M88DVBC,
    /**<External M88DVBC*/                     /**<CNcomment:外部M88DVBC芯片*/
    MT_UNF_DEMOD_DEV_TYPE_M88CT8K,               // 0x118 = 280, Montage-LZ Sym2 internel demod, DVB-T, DVB-T2, DVB-C, J.83B, DVB-S, DVB-S2, DVB-S2X
    MT_UNF_DEMOD_DEV_TYPE_M88TC6920,             // 0x119 = 281, Montage-LZ DVB-C, single demod
    MT_UNF_DEMOD_DEV_TYPE_M88TC6930,             // 0x11a = 282, Montage-LZ DVB-C, dual demod
    MT_UNF_DEMOD_DEV_TYPE_M88TC6960,             // 0x11b = 283, Montage-LZ DVB-C, dual demod
    MT_UNF_DEMOD_DEV_TYPE_M88RC6800,             // 0x11a = 284, Montage-LZ DVB-C
    MT_UNF_DEMOD_DEV_TYPE_M88RC6900,             // 0x11a = 285, Montage-LZ DVB-C
    MT_UNF_DEMOD_DEV_TYPE_M88CS8800 = 0x120,     // 0x120 = 288, Montage-LZ Sym4 internel demod, DVB-C, J.83B, DVB-S, DVB-S2, DVB-S2X
    MT_UNF_DEMOD_DEV_TYPE_M88RS6060 = 0x150,     // 0x150 = 336, Montage-LZ DVB-S/S2/S2X externel receiver(demod + tuenr), M88RS6060
    MT_UNF_DEMOD_DEV_TYPE_M88DS6103 = 0x160,     // 0x160 = 352, Montage-LZ DVB-S/S2/S2X externel demodulator, M88DS6103
    MT_UNF_DEMOD_DEV_TYPE_M88DS6113 = 0x161,     // 0x161 = 353, Montage-LZ DVB-S/S2/S2X externel demodulator, M88DS6113
    MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856 = 0x180,  // 0x180 = 384, Sony CXD2856, DVB-T, DVB-T2
    MT_UNF_DEMOD_DEV_TYPE_TP5001 = 0x181,        // 0x181 = 385, TelePath TP5001, ABS-S
    MT_UNF_DEMOD_DEV_TYPE_HD2502 = 0x183,        // 0x183 = 387, HDIC HD2502, ABS-S
    MT_UNF_DEMOD_DEV_TYPE_SONY_CXD_FAMILY = 0x190,  // 0x190 = 400, Sony CXD Family Chip,cxd2856/2878/6822 DVB-T, DVB-T2,ATSC3.0
    MT_UNF_DEMOD_TYPE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_demod_type_t;                         //MT_UNF_DEMOD_DEV_TYPE_E;

/** Defines the cable transmission signal.*/
/** CNcomment:定义CABLE传输信号*/
typedef struct
{
    mt_u32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    mt_unf_modulation_type_t mod_type; /**<QAM mode*/ /**<CNcomment:QAM调制方式*/
    mt_u32 band_width; /**<bandwidth in KHz*/         /**<CNcomment:带宽，单位kHz*/
    MT_BOOL b_reverse; /**<Spectrum reverse mode*/    /**<CNcomment:频谱倒置处理方式*/
} mt_unf_fe_cab_connect_para_t;

/** Guard interval of OFDM*/
/** CNcomment:多载波调制下的保护间隔*/
typedef enum unf_fe_guard_intv_t {
    MT_UNF_FE_GUARD_INTV_DEFALUT = 0,
    /**<default guard interval mode*/ /**<CNcomment:保护间隔默认模式*/
    MT_UNF_FE_GUARD_INTV_1_128,
    /**<1/128*/ /**<CNcomment:保护间隔1/128模式*/
    MT_UNF_FE_GUARD_INTV_1_32,
    /**<1/32*/ /**<CNcomment:保护间隔1/32模式*/
    MT_UNF_FE_GUARD_INTV_1_16,
    /**<1/16*/ /**<CNcomment:保护间隔1/16模式*/
    MT_UNF_FE_GUARD_INTV_1_8,
    /**<1/8*/ /**<CNcomment:保护间隔1/8模式*/
    MT_UNF_FE_GUARD_INTV_1_4,
    /**<1/4*/ /**<CNcomment:保护间隔1/4模式*/
    MT_UNF_FE_GUARD_INTV_19_128,
    /**<19/128*/ /**<CNcomment:保护间隔19/128模式*/
    MT_UNF_FE_GUARD_INTV_19_256,
    /**<19/256*/                                  /**<CNcomment:保护间隔19/256模式*/
    MT_UNF_FE_GUARD_INTV_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_guard_intv_t;

/** OFDM Mode, used in multi-carrier modulation*/
/** CNcomment:OFDM模式，用于多载波调制模式下*/
typedef enum unf_fe_fft_t {
    MT_UNF_FE_FFT_DEFAULT = 0,
    /**<default mode*/ /**<CNcomment:默认模式*/
    MT_UNF_FE_FFT_1K,
    /**<1k mode*/ /**<CNcomment:1k模式*/
    MT_UNF_FE_FFT_2K,
    /**<2k mode*/ /**<CNcomment:2k模式*/
    MT_UNF_FE_FFT_4K,
    /**<4k mode*/ /**<CNcomment:4k模式*/
    MT_UNF_FE_FFT_8K,
    /**<8k mode*/ /**<CNcomment:8k模式*/
    MT_UNF_FE_FFT_16K,
    /**<16k mode*/ /**<CNcomment:16k模式*/
    MT_UNF_FE_FFT_32K,
    /**<32k mode*/ /**<CNcomment:32k模式*/
    MT_UNF_FE_FFT_64K,
    /**<64k mode*/ /**<CNcomment:64k模式*/
    MT_UNF_FE_FFT_8E,
    /**<8e mode*/ /**<CNcomment:8e模式*/
    MT_UNF_FE_FFT_16E,
    /**<16e mode*/ /**<CNcomment:16e模式*/
    MT_UNF_FE_FFT_32E,
    /**<32e mode*/ /**<CNcomment:32e模式*/
    MT_UNF_FE_FFT_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_fft_t;                         //MT_UNF_TUNER_FE_FFT_E;

/** Hierarchical modulation mode, only used in DVB-T*/
/** CNcomment:仅用于DVB-T*/
typedef enum unf_fe_mterarchy_t {
    MT_UNF_FE_HIERARCHY_DEFAULT = 0,
    /**<hierarchical modulation default mode*/ /**<CNcomment:默认模式*/
    MT_UNF_FE_HIERARCHY_NO,
    /**<no hierarchical modulation mode*/ /**<CNcomment:不分级别模式*/
    MT_UNF_FE_HIERARCHY_ALHPA1,
    /**<hierarchical mode, alpha = 1*/ /**<CNcomment:分级别模式, alpha = 1*/
    MT_UNF_FE_HIERARCHY_ALHPA2,
    /**<hierarchical mode, alpha = 2*/ /**<CNcomment:分级别模式, alpha = 2*/
    MT_UNF_FE_HIERARCHY_ALHPA4,
    /**<hierarchical mode, alpha = 4*/           /**<CNcomment:分级别模式, alpha = 4*/
    MT_UNF_FE_HIERARCHY_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_mterarchy_t;

/** TS Priority, only used in DVB-T*/
/** CNcomment:仅用于DVB-T*/
typedef enum unf_fe_ts_priority_t {
    MT_UNF_FE_TS_PRIORITY_NONE = 0,
    /**<no priority mode*/ /**<CNcomment:无优先级模式*/
    MT_UNF_FE_TS_PRIORITY_HP,
    /**<high priority mode*/ /**<CNcomment:高优先级模式*/
    MT_UNF_FE_TS_PRIORITY_LP,
    /**<low priority mode*/                        /**<CNcomment:低优先级模式*/
    MT_UNF_FE_TS_PRIORITY_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_ts_priority_t;

/** base channel or lite channel, only used in DVB-T2*/
/** CNcomment:仅用于DVB-T2*/
typedef enum unf_fe_ter_mode_t {
    MT_UNF_FE_TER_MODE_BASE = 0,
    /**< the channel is base mode*/ /**<CNcomment:通道中仅支持base信号*/
    MT_UNF_FE_TER_MODE_LITE,
    /**< the channel is lite mode*/             /**<CNcomment:通道中需要支持lite信号*/
    MT_UNF_FE_TER_MODE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_ter_mode_t;

/** TS clock polarization*/
/** CNcomment:TS时钟极性*/
typedef enum unf_fe_tsclk_polar_t {
    MT_UNF_FE_TSCLK_POLAR_FALLING,
    /**<Falling edge*/ /**<CNcomment:下降沿*/
    MT_UNF_FE_TSCLK_POLAR_RISING,
    /**<Rising edge*/                              /**<CNcomment:上升沿*/
    MT_UNF_FE_TSCLK_POLAR_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_tsclk_polar_t;

/** TS format*/
/** CNcomment:TS格式*/
typedef enum unf_fe_ts_format_t {
    MT_UNF_FE_TS_FORMAT_TS,
    /**<188*/ /**<CNcomment:188字节格式*/
    MT_UNF_FE_TS_FORMAT_TSP,
    /**<204*/                                    /**<CNcomment:204字节格式*/
    MT_UNF_FE_TS_FORMAT_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_ts_format_t;

/** TS serial PIN*/
/** CNcomment:串行TS数据管脚*/
typedef enum unf_fe_ts_serial_pin_t {
    MT_UNF_FE_TS_SERIAL_PIN_0,
    /**<Serial pin 0, default*/ /**<CNcomment:串行模式，数据线使用第0bit*/
    MT_UNF_FE_TS_SERIAL_PIN_7,
    /**<Serial pin 7*/                               /**<CNcomment:串行模式，数据线使用第7bit*/
    MT_UNF_FE_TS_SERIAL_PIN_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_ts_serial_pin_t;

/** Tuner RF AGC mode*/
/** CNcomment:agc控制模式*/
typedef enum unf_fe_rfagc_mode_t {
    /**< Inverted polarization, default.This setting is used for a tuner whose gain decreases with increased AGC voltage*/ /**<CNcomment:agc反向控制模式*/
    MT_UNF_FE_RFAGC_INVERT,

    /**< Normal polarization. This setting is used for a tuner whose gain increases with increased AGC voltage*/ /**<CNcomment:agc正向控制模式*/
    MT_UNF_FE_RFAGC_NORMAL,
    MT_UNF_FE_RFAGC_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_rfagc_mode_t;

/** TS sync head length */
typedef enum unf_fe_ts_sync_head_t {
    MT_UNF_FE_TS_SYNC_HEAD_AUTO,
    MT_UNF_FE_TS_SYNC_HEAD_8BIT,
    MT_UNF_FE_TS_SYNC_HEAD_BUTT
} mt_unf_fe_ts_sync_head_t;

/** Tuner IQ spectrum mode*/
/** CNcomment:IQ模式*/
typedef enum unf_fe_iqspectrum_mode_t {
    MT_UNF_FE_IQSPECTRUM_NORMAL,
    /**<The received signal spectrum is not inverted*/ /**<CNcomment:IQ不反转*/
    MT_UNF_FE_IQSPECTRUM_INVERT,
    /**<The received signal spectrum is inverted*/ /**<CNcomment:IQ反转*/
    MT_UNF_FE_IQSPECTRUM_BUTT /**<Invalid value*/  /**<CNcomment:非法边界值*/
} mt_unf_fe_iqspectrum_mode_t;

/** DiSEqC Wave Mode*/
/** CNcomment:DiSEqC模式*/
typedef enum unf_fe_diseqcwave_mode_t {
    MT_UNF_FE_DISEQCWAVE_NORMAL,
    /**<Waveform produced by demod*/ /**<CNcomment:波形由demod产生*/
    MT_UNF_FE_DISEQCWAVE_ENVELOPE,
    /**<Waveform produced by LNB control device*/ /**<CNcomment:波形由控制芯片产生*/
    MT_UNF_FE_DISEQCWAVE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_diseqcwave_mode_t;

/** LNB power supply and control device*/
/** CNcomment:LNB供电和控制芯片*/
typedef enum unf_fe_lnbctrl_type_t {
    MT_UNF_LNBCTRL_DEV_TYPE_NONE,
    /*!
          lnb control by default mode
       */
    MT_UNF_LNBCTRL_DEV_TYPE_DEFAULT,
    /*!
          lnb control by GPIO
      */
    MT_UNF_LNBCTRL_DEV_TYPE_GPIO,
    /*!
          lnb control by external regulator A8304
       */
    MT_UNF_LNBCTRL_DEV_TYPE_A8304,

    /**<No LNB control device*/ /**<CNcomment:无控制芯片*/
    MT_UNF_LNBCTRL_DEV_TYPE_MPS8125,
    /**<MPS8125*/ /**<CNcomment:MPS8125*/
    MT_UNF_LNBCTRL_DEV_TYPE_ISL9492,
    /**<ISL9492*/                                    /**<CNcomment:ISL9492*/
    MT_UNF_LNBCTRL_DEV_TYPE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_lnbctrl_type_t;

/** Satellite extended attribution*/
/** CNcomment:卫星机附加属性*/
typedef struct unf_fe_sat_attr_t
{
    mt_u32 dmd_clk; /**<Demod reference clock freq, KHz*/                           /**<CNcomment:demod参考时钟频率，单位MHz*/
    mt_u16 tuner_max_lpf; /**<Tuner max LPF, MHz*/                                  /**<CNcomment:tuner低通滤波器通带频率，单位MHz*/
    mt_u16 tuner_i2c_clk; /**<Tuner I2C clock, kHz*/                                /**<CNcomment:tuner i2c时钟频率*/
    mt_unf_fe_rfagc_mode_t en_rfagc; /**<Tuner RF AGC mode*/                        /**<CNcomment:agc模式*/
    mt_unf_fe_iqspectrum_mode_t iq_spectrum; /**<Tuner IQ spectrum mode*/           /**<CNcomment:IQ模式*/
    mt_unf_fe_tsclk_polar_t ts_clk_polar; /**<TS clock polarization*/               /**<CNcomment:ts时钟极性*/
    mt_unf_fe_ts_format_t ts_format; /**<TS format*/                                /**<CNcomment:ts格式*/
    mt_unf_fe_ts_serial_pin_t ts_serial_pin; /**<TS serial PIN*/                    /**<CNcomment:ts串行数据线*/
    mt_unf_fe_diseqcwave_mode_t diseqc_wave; /**<DiSEqC Wave Mode*/                 /**<CNcomment:DiSEqC模式*/
    mt_unf_fe_lnbctrl_type_t lnbctrl_dev; /**<LNB power supply and control device*/ /**<CNcomment:LNB控制芯片*/
    mt_u16 lnb_dev_addr; /**<LNB control device address*/                           /**<CNcomment:LNB控制芯片i2c地址*/

    mt_u8 vsel_when_13v;
    mt_u8 vsel_when_lnb_off;
    mt_u8 diseqc_out_when_lnb_off;

} mt_unf_fe_sat_attr_t;

/** Satellite extended attribution*/
/** CNcomment:卫星机附加属性*/
typedef struct unf_fe_extra_cmd_t
{
    mt_u32 data[4]; /**<Demod reference clock freq, KHz*/ /**<CNcomment:demod参考时钟频率，单位MHz*/
} mt_unf_fe_extra_cmd_t;

/** Terrestrial extended attribution*/
/** CNcomment:地面机附加属性*/
typedef struct unf_fe_ter_attr_t
{
    mt_u32 dmd_clk; /**<Demod reference clock freq, KHz*/                 /**<CNcomment:demod参考时钟频率，单位MHz*/
    mt_u32 reset_gpio_no;                                                 /**< Demod reset GPIO NO. */
    mt_u16 tuner_max_lpf; /**<Tuner max LPF, MHz*/                        /**<CNcomment:tuner低通滤波器通带频率，单位MHz*/
    mt_u16 tun_i2c_clk; /**<Tuner I2C clock, kHz*/                        /**<CNcomment:tuner i2c时钟频率*/
    mt_unf_fe_rfagc_mode_t en_rfagc; /**<Tuner RF AGC mode*/              /**<CNcomment:agc模式*/
    mt_unf_fe_iqspectrum_mode_t iq_spectrum; /**<Tuner IQ spectrum mode*/ /**<CNcomment:IQ模式*/
    mt_unf_fe_tsclk_polar_t ts_clk_polar; /**<TS clock polarization*/     /**<CNcomment:ts时钟极性*/
    mt_unf_fe_ts_format_t ts_format; /**<TS format*/                      /**<CNcomment:ts格式*/
    mt_unf_fe_ts_serial_pin_t ts_serial_pin; /**<TS serial PIN*/          /**<CNcomment:ts串行数据线*/
    mt_unf_fe_ts_sync_head_t ts_sync_head; /**<TS sync head length*/      /**<CNcomment:ts同步头长度*/
} mt_unf_fe_ter_attr_t;

/** Polarization type*/
/** CNcomment:极化方式*/
typedef enum unf_fe_polar_t {
    MT_UNF_FE_POLARIZATION_H,
    /**<Horizontal Polarization*/ /**<CNcomment:水平极化*/
    MT_UNF_FE_POLARIZATION_V,
    /**<Vertical Polarization*/ /**<CNcomment:垂直极化*/
    MT_UNF_FE_POLARIZATION_L,
    /**<Left-hand circular Polarization*/ /**<CNcomment:左旋圆极化*/
    MT_UNF_FE_POLARIZATION_R,
    /**<Right-hand circular Polarization*/ /**<CNcomment:右旋圆极化*/
    MT_UNF_FE_POLARIZATION_BUTT,
    /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_polar_t;

/** FEC Type*/
/** CNcomment:卫星标准*/
typedef enum unf_fe_fec_type_t {
    MT_UNF_FE_DVBS,
    /**<DVB-S*/ /**<CNcomment:DVB-S标准*/
    MT_UNF_FE_DVBS2,
    /**<DVB-S2*/ /**<CNcomment:DVB-S2标准*/
    MT_UNF_FE_DIRECTV,
    /**<DIRECTV*/                      /**<CNcomment:DIRECTV标准*/
    MT_UNF_FE_DVBC,
    /**<DVB-C*/ /**<CNcomment:DVB-C J83AC标准*/
    MT_UNF_FE_J83B,
    /**<J83B*/ /**<CNcomment:DVB-C J83B标准*/
    MT_UNF_FE_DVBT,
    /**<DVB-T*/ /**<CNcomment:DVB-T标准*/
    MT_UNF_FE_DVBT2,
    /**<DVB-T2*/ /**<CNcomment:DVB-T2标准*/
    MT_UNF_FE_DTMB,
    /**<DTMB*/ /**<CNcomment:DTMB标准*/
    MT_UNF_FE_CTTB,
    /**<CTTB*/ /**<CNcomment:CTTB标准*/
    MT_UNF_FE_DVBS_AUTO,
    /**<CTTB*/ /**<CNcomment:DVB-S标准 or DVB-S2标准*/
    MT_UNF_FE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_fec_type_t;

/** FEC code Rate*/
/** CNcomment:FEC码率*/
typedef enum unf_fe_fecrate_t {
    MT_UNF_FE_FEC_AUTO = 0,
    MT_UNF_FE_FEC_1_2,
    /**<1/2*/ /**<CNcomment:1/2码率*/
    MT_UNF_FE_FEC_2_3,
    /**<2/3*/ /**<CNcomment:2/3码率*/
    MT_UNF_FE_FEC_3_4,
    /**<3/4*/ /**<CNcomment:3/4码率*/
    MT_UNF_FE_FEC_4_5,
    /**<4/5*/ /**<CNcomment:4/5码率*/
    MT_UNF_FE_FEC_5_6,
    /**<5/6*/ /**<CNcomment:5/6码率*/
    MT_UNF_FE_FEC_6_7,
    /**<6/7*/ /**<CNcomment:6/7码率*/
    MT_UNF_FE_FEC_7_8,
    /**<7/8*/ /**<CNcomment:7/8码率*/
    MT_UNF_FE_FEC_8_9,
    /**<8/9*/ /**<CNcomment:8/9码率*/
    MT_UNF_FE_FEC_9_10,
    /**<9/10*/ /**<CNcomment:9/10码率*/
    MT_UNF_FE_FEC_1_4,
    /**<1/4*/ /**<CNcomment:1/4码率*/
    MT_UNF_FE_FEC_1_3,
    /**<1/3*/ /**<CNcomment:1/3码率*/
    MT_UNF_FE_FEC_2_5,
    /**<2/5*/ /**<CNcomment:2/5码率*/
    MT_UNF_FE_FEC_3_5,
    /**<3/5*/ /**<CNcomment:3/5码率*/
    MT_UNF_FE_FEC_5_9,
    /**<5/9*/ /**<CNcomment:5/9码率*/
    MT_UNF_FE_FEC_7_9,
    /**<7/9*/ /**<CNcomment:7/9码率*/
    MT_UNF_FE_FEC_4_15,
    /**<4/15*/ /**<CNcomment:4/15码率*/
    MT_UNF_FE_FEC_7_15,
    /**<7/15*/ /**<CNcomment:7/15码率*/
    MT_UNF_FE_FEC_8_15,
    /**<8/15*/ /**<CNcomment:8/15码率*/
    MT_UNF_FE_FEC_11_15,
    /**<11/15*/ /**<CNcomment:11/15码率*/
    MT_UNF_FE_FEC_13_18,
    /**<13/18*/ /**<CNcomment:13/18码率*/
    MT_UNF_FE_FEC_9_20,
    /**<9/20*/ /**<CNcomment:9/20码率*/
    MT_UNF_FE_FEC_11_20,
    /**<11/20*/ /**<CNcomment:11/20码率*/
    MT_UNF_FE_FEC_23_36,
    /**<23/36*/ /**<CNcomment:23/36码率*/
    MT_UNF_FE_FEC_25_36,
    /**<25/36*/ /**<CNcomment:25/36码率*/
    MT_UNF_FE_FEC_11_45,
    /**<11/45*/ /**<CNcomment:11/45码率*/
    MT_UNF_FE_FEC_13_45,
    /**<13/45*/ /**<CNcomment:13/45码率*/
    MT_UNF_FE_FEC_14_45,
    /**<14/45*/ /**<CNcomment:14/45码率*/
    MT_UNF_FE_FEC_26_45,
    /**<26/45*/ /**<CNcomment:26/45码率*/
    MT_UNF_FE_FEC_28_45,
    /**<28/45*/ /**<CNcomment:28/45码率*/
    MT_UNF_FE_FEC_29_45,
    /**<29/45*/ /**<CNcomment:29/45码率*/
    MT_UNF_FE_FEC_31_45,
    /**<31/45*/ /**<CNcomment:31/45码率*/
    MT_UNF_FE_FEC_32_45,
    /**<32/45*/ /**<CNcomment:32/45码率*/
    MT_UNF_FE_FEC_77_90,
    /**<77/90*/ /**<CNcomment:77/90码率*/
    MT_UNF_FE_FEC_RESERVED,
    /**<RESERVED*/ /**<CNcomment:RESERVED 5/6/7码率, for DVB-T/T2*/
    MT_UNF_FE_FEC_UNDEF,
    MT_UNF_FE_FECRATE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_fecrate_t;

/** FE Roll-Off */
/** CNcomment:滚降系数*/
typedef enum unf_fe_roll_off_t {
    MT_UNF_FE_ROLL_OFF_UNDEF = 0,
    MT_UNF_FE_ROLL_OFF_0P35,
    /**<0.35*/ /**<CNcomment:0.35滚降系数*/
    MT_UNF_FE_ROLL_OFF_0P25,
    /**<0.25*/ /**<CNcomment:0.25滚降系数*/
    MT_UNF_FE_ROLL_OFF_0P20,
    /**<0.20*/ /**<CNcomment:0.20滚降系数*/
    MT_UNF_FE_ROLL_OFF_0P15,
    /**<0.15*/ /**<CNcomment:0.15滚降系数*/
    MT_UNF_FE_ROLL_OFF_0P10,
    /**<0.10*/ /**<CNcomment:0.10滚降系数*/
    MT_UNF_FE_ROLL_OFF_0P05,
    /**<0.05*/ /**<CNcomment:0.05滚降系数*/
    MT_UNF_FE_ROLL_OFF_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_roll_off_t;

/** LNB type*/
/** CNcomment:LNB类型*/
typedef enum unf_fe_lnb_type_t {
    MT_UNF_FE_LNB_SINGLE_FREQUENCY,
    /**<Single LO freq*/ /**<CNcomment:单本振*/
    MT_UNF_FE_LNB_DUAL_FREQUENCY,
    /**<Dual LO freq*/ /**<CNcomment:双本振*/
    MT_UNF_FE_LNB_UNICABLE,
    /**<Unicable LNB */                         /**<CNcomment:unicable高频头*/
    MT_UNF_FE_LNB_TYPE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_lnb_type_t;

/** LNB band type*/
/** CNcomment:卫星信号频段*/
typedef enum unf_fe_lnb_band_t {
    MT_UNF_FE_LNB_BAND_C,
    /**<C */ /**<CNcomment:C波段*/
    MT_UNF_FE_LNB_BAND_KU,
    /**<Ku */                                   /**<CNcomment:Ku波段*/
    MT_UNF_FE_LNB_BAND_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_lnb_band_t;

/** LNB power control*/
/** CNcomment:高频头供电控制*/
typedef enum unf_fe_lnb_power_t {
    MT_UNF_FE_LNB_POWER_OFF,
    /**<LNB power off*/ /**<CNcomment:关断*/
    MT_UNF_FE_LNB_POWER_ON,
    /**<LNB power auto, 13V/18V, default*/ /**<CNcomment:默认的13/18V供电*/
    MT_UNF_FE_LNB_POWER_ENHANCED,
    /**<LNB power auto, 14V/19V, some LNB control device can support.*/ /**<CNcomment:加强供电*/
    MT_UNF_FE_LNB_POWER_BUTT /**<Invalid value*/                        /**<CNcomment:非法边界值*/
} mt_unf_fe_lnb_power_t;

typedef enum unf_fe_ter_anttena_power_t {
    MT_UNF_FE_TER_ANTENNA_POWER_OFF,
    MT_UNF_FE_TER_ANTENNA_POWER_ON,
    MT_UNF_FE_TER_ANTENNA_POWER_BUTT
} mt_unf_fe_ter_antenna_power_t;

typedef enum unf_fe_demod_status_t {
    MT_UNF_FE_DEMODE_WAKE_UP = 0,
    MT_UNF_FE_DEMODE_STANDBY,
    MT_UNF_FE_DEMOD_STATUS_BUTT
} mt_unf_fe_demod_status_t;

/** LNB 22K tone status, for Ku band LNB*/
/** CNcomment:22k信号状态，用于Ku双本振*/
typedef enum unf_fe_lnb_22k_t {
    MT_UNF_FE_LNB_22K_OFF,
    /**<22k off*/ /**<CNcomment:22k信号关，选择低本振*/
    MT_UNF_FE_LNB_22K_ON,
    /**<22k on*/                               /**<CNcomment:22k信号开，选择高本振*/
    MT_UNF_FE_LNB_22K_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_lnb_22k_t;

/** 0/12V switch*/
/** CNcomment:0/12V开关*/
typedef enum unf_fe_switch_0_12v_t {
    MT_UNF_FE_SWITCH_0_12V_NONE,
    /**< None, default*/ /**<CNcomment:不接开关状态*/
    MT_UNF_FE_SWITCH_0_12V_0,
    /**< 0V*/ /**<CNcomment:0V状态*/
    MT_UNF_FE_SWITCH_0_12V_12,
    /**< 12V*/                                      /**<CNcomment:12V状态*/
    MT_UNF_FE_SWITCH_0_12V_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_switch_0_12v_t;

/** 22KHz switch*/
/** CNcomment:22K开关*/
typedef enum unf_fe_switch_22k_t {
    MT_UNF_FE_SWITCH_22K_NONE,
    /**< None, default*/ /**<CNcomment:不接开关状态*/
    MT_UNF_FE_SWITCH_22K_0,
    /**< 0*/ /**<CNcomment:0kHz端口*/
    MT_UNF_FE_SWITCH_22K_22,
    /**< 22KHz*/                                  /**<CNcomment:22kHz端口*/
    MT_UNF_FE_SWITCH_22K_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_switch_22k_t;

/** Tone burst switch*/
/** CNcomment:Tone burst开关*/
typedef enum unf_fe_switch_toneburst_t {
    MT_UNF_FE_SWITCH_TONEBURST_NONE,
    /**< Don't send tone burst, default*/ /**<CNcomment:不接开关状态*/
    MT_UNF_FE_SWITCH_TONEBURST_0,
    /**< Tone burst 0*/ /**<CNcomment:0 port*/
    MT_UNF_FE_SWITCH_TONEBURST_1,
    /**< Tone burst 1*/                                 /**<CNcomment:1 port*/
    MT_UNF_FE_SWITCH_TONEBURST_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_switch_toneburst_t;

/** scan range*/
/** CNcomment:地面机扫描范围配置*/
typedef enum unf_fe_ter_scan_mode_t {
    MT_UNF_FE_TER_SCAN_DVB_T2 = 0,
    /**< scan only DVB-T2*/ /**<CNcomment:仅扫描DVB-T2信号*/
    MT_UNF_FE_TER_SCAN_DVB_T,
    /**< scan only DVB-T*/ /**<CNcomment:仅扫描DVB-T信号*/
    MT_UNF_FE_TER_SCAN_DVB_T_T2_ALL,
    /**< scan both DVB-T2 and DVB-T*/                    /**<CNcomment:不但扫描DVB-T2信号，还要扫描DVB-T信号*/
    MT_UNF_FE_TER_SCAN_DVB_T_T2_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_ter_scan_mode_t;

/** pilot pattern */
typedef enum unf_fe_ter_pilot_pattern_t {
    MT_UNF_FE_T2_PILOT_PATTERN_PP1 = 0, /**< pilot pattern pp1 */
    MT_UNF_FE_T2_PILOT_PATTERN_PP2,     /**< pilot pattern pp2 */
    MT_UNF_FE_T2_PILOT_PATTERN_PP3,     /**< pilot pattern pp3 */
    MT_UNF_FE_T2_PILOT_PATTERN_PP4,     /**< pilot pattern pp4 */
    MT_UNF_FE_T2_PILOT_PATTERN_PP5,     /**< pilot pattern pp5 */
    MT_UNF_FE_T2_PILOT_PATTERN_PP6,     /**< pilot pattern pp6 */
    MT_UNF_FE_T2_PILOT_PATTERN_PP7,     /**< pilot pattern pp7 */
    MT_UNF_FE_T2_PILOT_PATTERN_PP8,     /**< pilot pattern pp8 */
    MT_UNF_FE_T2_PILOT_PATTERN_BUTT
} mt_unf_fe_ter_pilot_pattern_t;

/** information for channel mode*/
/** CNcomment:通道模式信息*/
typedef enum unf_fe_ter_channel_mode_t {
    MT_UNF_FE_TER_PURE_CHANNEL = 0,
    /**< pure channel*/ /**<CNcomment:纯通道模式*/
    MT_UNF_FE_TER_MIXED_CHANNEL,
    /**< mixed channel*/                                /**<CNcomment:混合通道模式*/
    MT_UNF_FE_TER_CHANNEL_MODE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_ter_channel_mode_t;

/** information for carrier mode*/
/** CNcomment:载波模式信息*/
typedef enum unf_fe_ter_carrier_mode_t {
    MT_UNF_FE_TER_EXTEND_CARRIER = 0,
    /**< extend carrier*/ /**<CNcomment:扩展载波*/
    MT_UNF_FE_TER_NORMAL_CARRIER,
    /**< normal carrier*/                               /**<CNcomment:正常载波*/
    MT_UNF_FE_TER_CARRIER_MODE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_ter_carrier_mode_t;

/** information for constellation mode*/
/** CNcomment:星座是否旋转*/
typedef enum unf_fe_ter_constellation_mode_t {
    MT_UNF_FE_CONSTELLATION_STANDARD = 0,
    /**< standard constellation*/ /**<CNcomment:不旋转星座*/
    MT_UNF_FE_CONSTELLATION_ROTATION,
    /**< rotation constellation*/                         /**<CNcomment:旋转星座*/
    MT_UNF_FE_CONSTELLATION_MODE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_ter_constellation_mode_t;

/** FEC frame length*/
/** CNcomment:FEC帧长*/
typedef enum unf_fe_ter_fecframe_mode_t {
    MT_UNF_FE_TER_FEC_FRAME_NORMAL = 0,
    /**< normal fec frame*/ /**<CNcomment:普通长度的fec帧*/
    MT_UNF_FE_TER_FEC_FRAME_SHORT,
    /**< short fec frame*/                                /**<CNcomment:fec短帧*/
    MT_UNF_FE_TER_FEC_FRAME_MODE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_ter_fecframe_mode_t;                          //MT_UNF_TUNER_TER_FEC_FRAME_MODE_E;

/** Structure of the satellite transmission signal's detailed information.*/
/** CNcomment:卫星信号详细信息*/
typedef struct unf_fe_sat_signalinfo_t
{
    mt_u32 freq; /**<Downlink freq, in kHz*/                                   /**<CNcomment:下行频率，单位：kHz*/
    mt_u32 symbol_rate; /**<Symbol rate, in Symb/s*/                           /**<CNcomment:符号率，单位Symb/s*/
    mt_unf_modulation_type_t mode_type; /**<Modulation type*/                  /**<CNcomment:调制方式*/
    mt_unf_fe_polar_t polar; /**<Polarization type*/                           /**<CNcomment:极化方式*/
    mt_unf_fe_fec_type_t sat_type; /**<Saterllite standard, DVB-S/S2/DIRECTV*/ /**<CNcomment:卫星标准， 支持DVB-S/S2/DIRECTV*/
    mt_unf_fe_fecrate_t fec_rate; /**<FEC rate*/                               /**<CNcomment:前向纠错码率*/
    mt_u8 pilot_mode; /**<Pilot mode, 1: On, 0: Off*/                          /**<CNcomment:Pilot On/Off*/
    mt_unf_fe_roll_off_t roll_off; /**<Roll off*/                              /**<CNcomment:Roll Off 滚降系数*/
} mt_unf_fe_sat_signal_info_t;

/*!
@brief DVBC nim type
*/
typedef struct unf_fe_cab_signalinfo_t
{
    mt_u32 freq; /**<Downlink freq, in kHz*/                                   /**<CNcomment:下行频率，单位：kHz*/
    mt_u32 symbol_rate; /**<Symbol rate, in Symb/s*/                           /**<CNcomment:符号率，单位Symb/s*/
    mt_unf_modulation_type_t mode_type; /**<Modulation type*/                  /**<CNcomment:调制方式*/
    mt_unf_fe_iqspectrum_mode_t iq_mode; /**<IQ mode*/                         /**<CNcomment:频谱反转*/
    mt_unf_fe_fec_type_t cab_type; /**<Cable standard, DVB-C/J83B*/            /**<CNcomment:卫星标准， 支持DVB-C/J83B*/
} mt_unf_fe_cab_signal_info_t;


/*!
@brief DVBT nim type
*/
typedef enum _unf_fe_dvbt_type_t {
    MT_UNF_PORT_TYPE_DVBT,
    MT_UNF_PORT_TYPE_DVBT2,
    MT_UNF_PORT_TYPE_DVBT_AUTO,
    MT_UNF_PORT_TYPE_DVBT_BUTT,
} mt_unf_fe_dvbt_type_t;

/** Terestrial connect param*/
/** CNcomment:地面信号锁台参数*/
typedef struct mt_unf_fe_ter_connect_para_t
{
    mt_u32 freq;                        /**<freq in KHz */
    mt_u32 band_width;                  /**<bandwidth in KHz */
    mt_unf_modulation_type_t mode_type; /**< */
    mt_unf_fe_dvbt_type_t port_type;    /** differ DVBT/DVBT2/AUTO from eatchother */
    mt_u8 plp_id;
    mt_u8 data_plp_number;
    mt_u8 plp_index;
    mt_u8 data_plpid_array[255];
    MT_BOOL b_reverse; /**<Spectrum reverse mode*/ /**<CNcomment:频谱翻转处理方式*/
    mt_unf_fe_ter_mode_t channel_mode;             //dvb-t2
    mt_unf_fe_ts_priority_t dvbt_prio;             //dvb-t
    mt_u16 cell_id;
} mt_unf_fe_ter_connect_para_t;

/** PLP type of DVB-T2.*/
/** CNcomment:T2下物理层管道类型*/
typedef enum unf_fe_t2_plp_type_t {
    MT_UNF_FE_T2_PLP_TYPE_COM = 0,
    /**<common type*/ /**<CNcomment:普通*/
    MT_UNF_FE_T2_PLP_TYPE_DAT1,
    /**<data1 type*/ /**<CNcomment:数据1*/
    MT_UNF_FE_T2_PLP_TYPE_DAT2,
    /**<data2 type*/                               /**<CNcomment:数据2*/
    MT_UNF_FE_T2_PLP_TYPE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_t2_plp_type_t;

/** Structure of the terrestrial transmission signal's detailed information.*/
/** CNcomment:地面信号详细信息*/
typedef struct unf_fe_ter_signaltype_t
{
    mt_u32 freq; /**<Frequency, in kHz*/                      /**<CNcomment:频率，单位：kHz*/
    mt_u32 band_width; /**<Band width, in KHz*/               /**<CNcomment:带宽，单位KHz */
    mt_unf_modulation_type_t enModType; /**<Modulation type*/ /**<CNcomment:调制方式*/
	mt_unf_fe_fec_type_t ter_type; /**<Terrestrial standard, DVB-T/T2*/              /**<CNcomment:地面标准， 支持DVB-T/T2*/
    mt_unf_fe_fecrate_t enFECRate; /**<FEC rate*/             /**<CNcomment:前向纠错码率*/
    mt_unf_fe_fecrate_t enLowPriFECRate;
    mt_unf_fe_guard_intv_t enGuardIntv; /**<GI mode*/                                /**<CNcomment:保护间隔模式*/
    mt_unf_fe_fft_t enFFTMode; /**<FFT mode*/                                        /**<CNcomment:FFT模式*/
    mt_unf_fe_mterarchy_t enHierMod;                                                 /**<Hierarchical Modulation and alpha, only used in DVB-T*/
                                                                                     /**<CNcomment:hierachical模式和alpha值*/
    mt_unf_fe_ts_priority_t enTsPriority;                                            /**<The TS priority, only used in DVB-T*/
                                                                                     /**<CNcomment:ts priority, 仅用于DVB-T模式下*/
    mt_unf_fe_t2_plp_type_t enPLPType; /**<PLP type*/                                /**<CNcomment:物理层管道类型*/
    mt_unf_fe_ter_pilot_pattern_t enPilotPattern; /**<pilot pattern*/                /**<CNcomment:导频模式*/
    mt_unf_fe_ter_carrier_mode_t enCarrierMode; /**<carrier mode*/                   /**<CNcomment:载波模式*/
    mt_unf_fe_ter_constellation_mode_t enConstellationMode; /**<constellation mode*/ /**<CNcomment:星座是否旋转*/
    mt_unf_fe_ter_fecframe_mode_t enFECFrameMode; /**<FEC frame length*/             /**<CNcomment:FEC帧长*/

    mt_u16 cell_id;
} mt_unf_fe_ter_signal_type_t;

/** signal information.*/
/** CNcomment:TUNER信号属性*/
typedef struct unf_fe_signal_info_t
{
    mt_unf_fe_sig_type_t sig_type; /**<Signal transmission type*/ /**<CNcomment:信号类型*/

    union
    {
        mt_unf_fe_sat_signal_info_t sat; /**<Signal info of satellite*/   /**<CNcomment:卫星信号信息*/
        mt_unf_fe_ter_signal_type_t ter; /**<Signal info of terrestrial*/ /**<CNcomment:地面信号信息*/
        mt_unf_fe_cab_signal_info_t cab; /**<Signal info of cable*/       /**<CNcomment:有线信号信息*/
    } sig_info;
} mt_unf_fe_signal_info_t;

typedef enum _unf_fe_lnb_polar {
    /*!
        LNB porlarity horizontal
      */
    PORT_PORLAR_HORIZONTAL = 0x00,
    /*!
        LNB porlarity vertical
      */
    PORT_PORLAR_VERTICAL = 0x01,
    /*!
        LNB porlarity left
      */
    PORT_PORLAR_LEFT = 0x02,
    /*!
        LNB porlarity right
      */
    PORT_PORLAR_RIGHT = 0x03,
    /*!
        All LNB porlarity
      */
    PORT_PORLAR_ALL = 0x04,
} mt_unf_fe_lnb_polar;

/** unicable multi-switch port.*/
/** CNcomment:unicable开关端口枚举*/
typedef enum unf_fe_sat_position_t {
    MT_UNF_FE_SATPOSN_A,
    /**<unicable switch port A*/ /**<CNcomment:端口A*/
    MT_UNF_FE_SATPOSN_B,
    /**<unicable switch port B*/              /**<CNcomment:端口B*/
    MT_UNF_FE_SATPOSN_BUT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_sat_position_t;

/** LNB configurating parameters*/
/** CNcomment:LNB配置参数*/
typedef struct unf_fe_lnb_config_t
{
    mt_unf_fe_lnb_type_t lnb_type; /**<LNB type*/                                 /**<CNcomment:LNB类型*/
    mt_u32 low_lo; /**< Low Local Oscillator Frequency, MHz */                    /**<CNcomment:LNB低本振频率，单位MHz*/
    mt_u32 high_lo; /**< High Local Oscillator Frequency, MHz*/                   /**<CNcomment:LNB高本振频率，单位MHz*/
    mt_unf_fe_lnb_band_t lnb_band; /**< LNB band, C or Ku */                      /**<CNcomment:LNB波段：C或Ku*/
    mt_u8 unicable_scr_no; /**< SCR number, 0-31 */                               /**<CNcomment:SCR序号，取值为0-31, 1.0 (0~7), 2.0(0~31)*/
    mt_u32 unicable_if_freq_mhz; /**< SCR IF freq, unit MHz */                    /**<CNcomment:SCR中频频率，单位MHz*/
    mt_u32 unicable_bank; /**< SCR IF bank index, 0-7 */                          /**<CNcomment:unicable bank index, 0~7*/
    mt_unf_fe_sat_position_t unicable_port_no; /**< unicable multi-switch port */ /**<CNcomment:unicable开关端口号*/
    mt_u8 unicable_ver_no; /**<Unicable Version, 2: 2.0; 1 or others: 1.0 */      /**<CNcomment:Unicable 版本号， 1.0或2.0*/
    mt_u8 lnb_agent_mode; /**<LNB agend mode, 0: disable, 1, enable */            /**<CNcomment:LNB代理模式*/
    mt_u8 lnb_agent_id; /**<LNB agend port id */                                  /**<CNcomment:LNB代理模块选择*/
} mt_unf_fe_lnb_config_t;

/*!
@brief NIM on-board pin level configuration
  */
typedef struct _unf_fe_pin_config_para_t
{
    /*!
    NIM lock indication pin's active level, 0:low active, 1:high active
    */
    mt_u8 lock_indicate : 1;
    /*!
    NIM voltage selection pin's active level, 0:select 13v when the pin is low, 1:select
    13v when the pin is high
    */
    mt_u8 vsel_when_13v : 1;
    /*!
    VSEL pin's active level when LNB is in standby state, 0: low active, 1:high active
    */
    mt_u8 vsel_when_lnb_off : 1;
    /*!
    DiSEqC out pin's active level when LNB is in standby state, 0: low active, 1:high active
    */
    mt_u8 diseqc_out_when_lnb_off : 1;
    /*!
    LNB enable pin's active level, 0: low active, 1:high active
    */
    mt_u8 lnb_enable : 1;
    /*!
    LNB short circuit protection pin's level, 0: protect enable(short circuit happened) on low level,
    1:protect enable on high level
    */
    mt_u8 lnb_prot_level : 1;
    /*!
    LNB enable(power enable or short circuit protection) pin's control by mcu or nim, 1: by mcu, 0:by nim
    */
    mt_u8 lnb_enable_by_mcu : 1;
    /*!
    LNB short circuit protection by mcu or nim, 1: by mcu, 0:by nim
    */
    mt_u8 lnb_prot_by_mcu : 1;
    /*!
    LNB enable pin's number
    */
    mt_u8 lnb_enable_pin;
    /*!
    LNB short circuit protection pin's number
    */
    mt_u8 lnb_prot_pin;
    /*!
    flag to indicate if lock pin is by mcu's gpio
    */
    mt_u8 lock_pin_by_mcu : 1;
    /*!
    lock gpio pin number
    */
    mt_u8 lock_pin : 7;
    /*!
    flag to indicate if voltage select pin is by mcu's gpio
    */
    mt_u8 lnb_vol_pin_by_mcu : 1;
    /*!
    voltage select gpio pin number
    */
    mt_u8 lnb_vol_pin : 7;
    /*!
    demod reset gpio pin number
    */
    mt_u8 demod_reset_pin;
    /*!
    lnb voltage selection pin mode, is only valid when lnb_vol_pin_by_mcu is 1
    0:default(demo board); 1:custom mode1; 2:custom mode2; ...
    */
    mt_u8 lnb_vol_pin_mode;
    /*!
    t2 antenna short circuit protection pin's level, 0: protect enable(short circuit happened) on low level,
    1:protect enable on high level
    */
    mt_u8 ant_prot_level : 1;
    /*!
    LNB enable(power enable or short circuit protection) pin's control by mcu or nim, 1: by mcu, 0:by nim
    */
    //mt_u8 lnb_enable_by_mcu : 1;
    /*!
    t2 antenna short circuit protection by mcu or ap cpu, 1: by mcu, 0:by ap cpu
    */
    mt_u8 ant_prot_by_mcu : 1;
    /*!
    t2 antenna enable pin's number
    */
    //mt_u8 ant_enable_pin;
    /*!
    t2 antenna short circuit protection pin's number
    */
    mt_u8 ant_prot_pin;
    /*!
      DiSEqC RX mode: 0, from demod; 1, from GPIO //diseqc2
      */
    mt_u8 diseqc_rx_mode;
    /*!
      DiSEqC RX GPIO pin, when (diseqc_rx_mode == 1)
      */
    mt_u8 diseqc_rx_gpio_pin;
} mt_unf_fe_pin_config_para_t;

/*!
  @brief NIM device configuration struct
  */
typedef struct _unf_fe_config_para_t
{
    /*!
    TS output mode, serial or parallel, see enum nim_ts_mode.
    */
    mt_u32 ts_mode;
    /*!
    Tuner's 2-wire bus address(for s/s2 use when board has two tuners)
    */
    mt_u8 tun_addr;
    /*!
    Tuner's 2-wire bus address(for t/t2 use when board has two tuners)
    */
    mt_u8 tun2_addr;
    /*!
    For t/t2 use(0:none, 1:using, other:for future use)
    */
    mt_u8 lna_type;
    /*!
    The tuner supported by this NIM driver, see enum nim_tuner_ver_t
    */
    mt_u8 tun_support;

    mt_u32 tun2_type;
    /*!
    The times of blind scan
    */
    mt_u8 bs_times;
    /*!
    The on-board pin's level configuration
    */
    mt_unf_fe_pin_config_para_t pin_config;
    /*!
    number of set limit lock ferquency
    */
    mt_u32 freq_offset_limit;
    /*!
	clock out type 0-close, 1-from tuner xtalout pin to demodulator use, 2-from tuner clkout pin to other chip use
    */
    //mt_u8 tun_clk_out_cfg;

    mt_unf_fe_lnbctrl_type_t lnbctrl_dev; /**<LNB power supply and control device*/ /**<CNcomment:LNB控制芯片*/
    mt_u16 lnb_dev_addr; /**<LNB control device address*/                           /**<CNcomment:LNB控制芯片i2c地址*/
    mt_u16 lnb_i2c_id;
} mt_unf_fe_config_para_t;

/*!
  @brief The port dm6k config
  */
typedef struct unf_fe_port_dm6k_config_t
{
    unsigned int x_crystal;        /* NIM_FE_XTAL */
    unsigned int udvbt_tuner;      /* tuner for dvbt */
    unsigned int tun2_crystal;     /* tc3800 crystal */
    unsigned int tun2_loop;        /* tc3800 loop through */
    unsigned int tun2_clk_out;     /* tc3800 clock out on/off */
    unsigned int udvbs_tuner;      /* tuner for dvbs */
    unsigned int udvbt_serialtsno; /* c/t/t2 ts output position */
    unsigned int udvbs_serialtsno; /* s/s2 ts output position */
} mt_unf_fe_port_dm6k_config_t;

typedef struct unf_fe_attr_t
{
    mt_unf_fe_sig_type_t sig_type;
    mt_unf_tuner_type_t tuner_type;
    mt_u32 tuner_addr;
    mt_unf_demod_type_t demod_dev_type;
    mt_u32 demod_addr;
    mt_unf_fe_output_mode_t output_mode;
    mt_u8 demod_i2c_id;
    mt_u8 tuner_i2c_id[MT_I2C_MAX_NUM_USER];
    mt_unf_fe_config_para_t fe_config;
    mt_unf_fe_port_dm6k_config_t privte; //!<@~chinese 私有配置 //!<@~english private configure
    mt_u8 no_need_init;
} mt_unf_fe_attr_t;

typedef union
{
  /*
     PLS Gold Code:
        if(PLSType == 2)
          PLSCodeAll = (PLSType << 24) + (PLSGoldCode & 0xFFFFFF);
     PLS normal Code:
        if(PLSType == 1)
          PLSCodeAll = (PLSType << 24) + (PLSCode[2] << 16) + (PLSCode[1] << 8) + (PLSCode[0] << 0);
     */
  mt_u32 PLSCodeAll;

  struct
  {
    /*
        PLS gold code, or PLS code details
       */
    mt_u8 PLSCode[3];

    /*
        Have PLS Code or not, 0: no PLS; 1: have normal PLS codes; 2: have PLS gold code
       */
    mt_u8 PLSType;
  } PLSDetail;
} mt_unf_fe_sat_pls_code_t;

/*!
    @brief The channel performance structure
  */
typedef struct _unf_fe_channel_perf_t
{
    /*!
      Channel is locked or not
      */
    mt_u8 lock;
    /*!
      Signal strength from NIM AGC gain
      */
    mt_u32 agc;
    /*!
      Signal noise rate in percentage
      */
    mt_u32 snr;
    /*!
      Bit error rate, normally it is a very little number.
      */
    mt_double ber;
} mt_unf_fe_channel_perf_t;

typedef struct _unf_fe_channel_info_t
{
    /*!
    channel is locked or not, see MT_UNF_FE_CHANNEL_LOCK_S
    */
    mt_u8 lock;
    /*!
    spectral I/Q porlarity, see nim_spectral_polar_t
    */
    mt_u8 spectral_polar;
    /*!
    forward error correction, see enum nim_code_rate_t
    */
    //mt_u8  fec_inner;
    /*!
    NIM type, see enum nim_type_t
    */
    mt_unf_fe_fec_type_t port_type;
    /*!
    Channel frequency in KHz
    */
    mt_u32 frequency;
    /*!
    symbol rate in Symbols per second, in KSs
    */
    mt_u32 symbol_rate;
    /*!
    forward error correction, see enum nim_code_rate_t
    */
    mt_u8 fec_inner;
    /*!
    uni-cable parameter
    */
    //MT_UNF_FE_UNICABLE_PARAM_S uc_param;
    /*!
    performance
    */
    mt_unf_fe_channel_perf_t perf;

    /*!
    Total TS number
    */
    mt_u8 DataTsNumber;
    /*!
    Current ts id
    */
    mt_u8 ts_id;
    /*!
    TS index of ts array, reserved for future use
    */
    mt_u8 ts_index;
    /*!
    TS list array, maxium 32 TS
    */
    mt_u8 DataTsIdArray[32];
    /*!
    PLS code info
    */
    mt_unf_fe_sat_pls_code_t PLS;
} mt_unf_fe_channel_info_t;

/*!
    @brief The channel setting information
  */
typedef struct _unf_fe_channel_set_info_t
{
    /*!
      the flag to indicate if this channel setting is in blind scan
      */
    mt_u8 for_scan;
    /*!
      the delay time for locking this channel, in microsecond
      */
    mt_u32 lock_time;
} mt_unf_fe_channel_set_info_t;

/*!
    @brief Uni-cable paramter
  */
typedef struct _unf_fe_unicable_param_t
{
    /*!
    the flag to indicate if use uni-cable
    */
    mt_u8 use_uc : 1;
    /*!
    the bank number, from 0 to 7
    */
    mt_u8 bank : 7;
    /*!
    the user band number, from 0 to 11
    */
    mt_u8 user_band;
    /*!
    the user band frequency in MHz.
    */
    mt_u16 ub_freq_mhz;
    /*!
    the version of unicable, 0: no unicable; 2: 2.0; 1 or other values: 1.0
    */
    mt_u8 ub_ver;
} mt_unf_fe_unicable_param_t;

/*!
@brief DVBS nim type
*/
typedef enum _unf_fe_dvbs_type_t
{
    MT_UNF_PORT_TYPE_DVBS,
    MT_UNF_PORT_TYPE_DVBS2,
    MT_UNF_PORT_TYPE_DVBS_AUTO,
    MT_UNF_PORT_TYPE_DIRECTV,
    MT_UNF_PORT_TYPE_DVBS_BUTT,
} mt_unf_fe_dvbs_type_t;

typedef struct _unf_fe_sat_connect_para_t
{
    mt_u32 freq; /* frequency kHz */
    mt_u32 sym_rate;
    mt_unf_fe_dvbs_type_t port_type;     //!<differ DVBS/DVBS2/AUTO from eatchother
    mt_u8 onoff_22k;                     //!< 22K on/off
    mt_u8 polarization;                  //!< Polarization
    mt_unf_fe_unicable_param_t uc_param; //!<Uni-cable paramter
    mt_u8 lnb_status;	//1:lnb has been set, 0:not set

    mt_u8 DataTsNumber;			// Total TS number
    mt_u8 ts_id;				// current ts id
    mt_u8 ts_index;				// ts_id index of ts array
    mt_u8 DataTsIdArray[32];	// ts id array, maxium 16 TS

    mt_unf_fe_sat_pls_code_t PLS;		// PLS code info

    mt_u16 AMC_index;			// AMC index value, for DSS only
    mt_s16 AMC_offset;			// AMC offset value, for DSS only
} mt_unf_fe_sat_connect_para_t;

/** Locking mode parameters of the frontend module*/
/** CNcomment:FE锁频模式*/
typedef enum _unf_fe_connect_mode_t
{
    MT_UNF_FE_CONNECT_MODE_NORMAL         = 0,    // normal conect mode, set tuner & demod
    MT_UNF_FE_CONNECT_MODE_SET_TUNER_ONLY = 1,    // set tuner only
    MT_UNF_FE_CONNECT_MODE_SET_DEMOD_ONLY = 2,    // set demod only
    MT_UNF_FE_CONNECT_MODE_UNDEF          = 0xFF
} mt_unf_fe_connect_mode_t;

/** Frequency locking parameters of the frontend module*/
/** CNcomment:FE锁频参数*/
typedef struct
{
    mt_unf_fe_sig_type_t sig_type;
    mt_unf_fe_channel_info_t channel_info;         //new add
    mt_unf_fe_channel_set_info_t channel_set_info; //new add
    union
    {
        mt_unf_fe_cab_connect_para_t cab;
        mt_unf_fe_ter_connect_para_t ter;
        mt_unf_fe_sat_connect_para_t sat;
    } connect_param; //!<@~chinese 锁频参数//!<@~english frequency parameters

    mt_unf_fe_connect_mode_t connect_mode;  // connect mode -- 1: set tuner only; 2: set demod only; 0 or other: normal mode, set tuner & demod
} mt_unf_fe_connect_para_t;

/** Frequency locking status and parameters of the tuner*/
/** CNcomment:TUNER锁频状态和锁频参数*/
typedef struct
{
    mt_unf_fe_lock_status_t lock_status; /**<Frequency locking status*/  /**<CNcomment:锁频状态*/
    mt_unf_fe_connect_para_t param; /**<Actual freq locking parameters*/ /**<CNcomment:实际锁频参数*/
    mt_unf_fe_unlock_reason_t unlock_reason; /**<Frequency unlock reason*/
} mt_unf_fe_status_t;

/** Frequency locking parameters of the tuner*/
/** CNcomment:TUNER锁频参数*/
typedef struct _mt_unf_fe_tuner_param_t
{
    mt_unf_fe_sig_type_t sig_type;     /* signal type of the tuner */
    mt_s32               freq_KHz;	   /* tuner frequency, unit: KHz */
    mt_s32               sym_rate_KSs; /* symbol rate, unit: KS/s */
} mt_unf_fe_tuner_param_t;

/** Tuner blind scan type*/
/** CNcomment:TUNER盲扫方式*/
typedef enum unf_fe_blindscan_mode_t {
    MT_UNF_FE_BLINDSCAN_MODE_AUTO = 0,
    /**<Blind scan automatically*/ /**<CNcomment:自动扫描*/
    MT_UNF_FE_BLINDSCAN_MODE_MANUAL,
    /**<Blind scan manually*/                         /**<CNcomment:手动扫描*/
    MT_UNF_FE_BLINDSCAN_MODE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_blindscan_mode_t;

/** Definition of blind scan event type*/
/** CNcomment:TUNER盲扫事件*/
typedef enum unf_fe_blindscan_evt_t {
    MT_UNF_FE_BLINDSCAN_EVT_STATUS,
    /**<New status*/ /**<CNcomment:状态变化*/
    MT_UNF_FE_BLINDSCAN_EVT_PROGRESS,
    /**<New Porgress */ /**<CNcomment:进度变化*/
    MT_UNF_FE_BLINDSCAN_EVT_NEWRESULT,
    /**<FREQ LOCKED */ /**<CNcomment:锁定频点，带频点参数*/
    MT_UNF_FE_BLINDSCAN_EVT_LOCKED,
    /**<FREQ UNLOCKED */ /**<CNcomment:未锁定频点，带频点参数*/
    MT_UNF_FE_BLINDSCAN_EVT_UNLOCK,
     /**<FIND TP */ /**<CNcomment:盲扫前扫描频谱阶段，带进度信息*/
    MT_UNF_FE_BLINDSCAN_EVT_FINDTP,
    /**<Find new channel*/                           /**<CNcomment:新频点*/
    MT_UNF_FE_BLINDSCAN_EVT_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_blindscan_evt_t;

/** Definition of tuner blind scan status*/
/** CNcomment:TUNER盲扫状态*/
typedef enum unf_fe_blindscan_status_t {
    MT_UNF_FE_BLINDSCAN_STATUS_IDLE,
    /**<Idel*/ /**<CNcomment:空闲*/
    MT_UNF_FE_BLINDSCAN_STATUS_SCANNING,
    /**<Scanning*/ /**<CNcomment:扫描中*/
    MT_UNF_FE_BLINDSCAN_STATUS_FINISH,
    /**<Finish*/ /**<CNcomment:成功完成*/
    MT_UNF_FE_BLINDSCAN_STATUS_QUIT,
    /**<User quit*/ /**<CNcomment:用户退出*/
    MT_UNF_FE_BLINDSCAN_STATUS_FAIL,
    /**<Scan fail*/                                     /**<CNcomment:扫描失败*/
    MT_UNF_FE_BLINDSCAN_STATUS_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_blindscan_status_t;

/** Definition of tuner blind scan status*/
/** CNcomment:地面TUNER搜台状态*/
typedef enum unf_fe_ter_scan_status_t {
    MT_UNF_FE_TER_SCAN_STATUS_IDLE,
    /**<Idel*/ /**<CNcomment:空闲*/
    MT_UNF_FE_TER_SCAN_STATUS_SCANNING,
    /**<Scanning*/ /**<CNcomment:扫描中*/
    MT_UNF_FE_TER_SCAN_STATUS_FINISH,
    /**<Finish*/ /**<CNcomment:成功完成*/
    MT_UNF_FE_TER_SCAN_STATUS_QUIT,
    /**<User quit*/ /**<CNcomment:用户退出*/
    MT_UNF_FE_TER_SCAN_STATUS_FAIL,
    /**<Scan fail*/                                    /**<CNcomment:扫描失败*/
    MT_UNF_FE_TER_SCAN_STATUS_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_ter_scan_status_t;

/** Structure of satellite TP*/
/** CNcomment:TUNER扫出TP信息*/
typedef struct unf_fe_sat_tpinfo_t
{
    mt_u32 freq; /**<Downlink freq, in kHz*/         /**<CNcomment:下行频率，单位：kHz*/
    mt_u32 symbol_rate; /**<Symbol rate, in bit/s*/  /**<CNcomment:符号率，单位bps */
    mt_unf_fe_polar_t polar; /**<Polarization type*/ /**<CNcomment:极化方式*/
#if 0
    mt_unf_fe_polar_t  polar;            /**<Polarization type*/             /**<CNcomment:极化方式*/
    mt_u8               cbs_reliablity;     /**<TP reliability*/                /**<CNcomment:TP的可靠度*/
#endif
    mt_u8 dvb_type; /**<Downlink freq, in kHz*/  /**<CNcomment:下行频率，单位：kHz*/
    mt_u8 code_rate; /**<Symbol rate, in bit/s*/ /**<CNcomment:符号率，单位bps */
    mt_u8 spectrum; /*<Spectrum inverted>, bool*/ /**<CNcomment:频谱反转，0 or 1 */

    mt_u8 DataTsNumber;         /**<CNcomment: multi-stream TS总数, Total TS number，<= 32*/
    mt_u8 ts_id;                /**<CNcomment: 当前TS ID，current ts id*/
    mt_u8 ts_index;             /**<CNcomment: 当前TS index，ts_id index of ts array， reserved*/
    mt_u8 DataTsIdArray[32];    /**<CNcomment: TS列表数组，ts id array, maxium 32 TS*/
} mt_unf_fe_sat_tpinfo_t;

/** Structure of terrestrial tp information*/
/** CNcomment:地面机频点信息*/
typedef struct unf_fe_ter_channel_attr_t
{
    mt_u32 freq; /**<freq of TP,unit KHz*/                           /**<CNcomment:频点的频率，单位是KHz*/
    mt_u32 band_width; /**<BandWidth of TP,unit KHz*/                /**<CNcomment:频点的带宽，单位是KHz*/
    mt_u8 dvbt_mode; /**<signal type.0--DVB-T2,1--DVB-T*/            /**<CNcomment:信号类型.0--DVB-T2,1--DVB-T*/
    mt_u8 plp_index; /**<plp index*/                                 /**<CNcomment:物理层管道索引号*/
    mt_u8 plp_id; /**<plp id*/                                       /**<CNcomment:物理层管道ID*/
    mt_u8 com_id; /**<common plp id*/                                /**<CNcomment:共享物理层管道ID*/
    mt_u8 combination; /**<plp combination flag*/                    /**<CNcomment:数据物理层管道和共享物理层管道是否组合标志*/
    mt_unf_fe_ter_mode_t channel_mode; /**<channel attribute*/       /**<CNcomment:通道属性*/
    mt_unf_fe_ts_priority_t ts_pri; /**<TS priority, only in DVB-T*/ /**<CNcomment:TS优先级模式，仅用于DVB-T*/
} mt_unf_fe_ter_channel_attr_t;

/** Notify structure of tuner blind scan */
/** CNcomment:TUNER盲扫通知信息*/
typedef union unf_fe_blindscan_notify_t
{
    mt_unf_fe_blindscan_status_t *status; /**<Scanning status*/ /**<CNcomment:盲扫状态*/
    mt_u16 *progress_percent; /**<Scanning progress*/           /**<CNcomment:盲扫进度*/
    mt_unf_fe_sat_tpinfo_t *result; /**<Scanning result*/       /**<CNcomment:盲扫结果*/
} mt_unf_fe_blindscan_notify_t;

/** Notify structure of tuner blind scan */
/** CNcomment:地面TUNER搜台通知信息*/
typedef union unf_fe_ter_scan_notify_t
{
    mt_unf_fe_ter_scan_status_t *status; /**<Scanning status*/  /**<CNcomment:搜台状态*/
    mt_u16 *progress_percent; /**<Scanning progress*/           /**<CNcomment:搜台进度*/
    mt_unf_fe_ter_channel_attr_t *result; /**<Scanning result*/ /**<CNcomment:搜台结果*/
} mt_unf_fe_ter_scan_notify_t;

/** Parameter of the satellite tuner blind scan */
/** CNcomment:卫星TUNER盲扫参数*/
typedef struct unf_fe_sat_blindscan_para_t
{
    /**<LNB Polarization type, only take effect in manual blind scan mode*/
    /**<CNcomment:LNB极化方式，自动扫描模式设置无效*/
    mt_unf_fe_polar_t polar;

    /**<LNB 22K signal status, for Ku band LNB which has dual LO, 22K ON will select high LO and 22K off select low LO,
        only take effect in manual blind scan mode*/
    /**<CNcomment:LNB 22K状态，对于Ku波段双本振LNB，ON选择高本振，OFF选择低本振，自动扫描模式设置无效*/
    mt_unf_fe_lnb_22k_t lnb_22k;

    /**<Blind scan start IF, in kHz, only take effect in manual blind scan mode */
    /**<CNcomment:盲扫起始频率(中频)，单位：kHz，自动扫描模式设置无效*/
    mt_u32 start_freq;

    /**<Blind scan stop IF, in kHz, only take effect in manual blind scan mode */
    /**<CNcomment:盲扫结束频率(中频)，单位：kHz，自动扫描模式设置无效*/
    mt_u32 stop_freq;

    /*!
    the start frequency(in KHz) in blind scan, will return the next start
    frequency auto-detected by NIM after every scan window.
    */
    //MT_U32 start_freq;
    /*!
    the end frequency(in KHz) in blind scan
    */
    //MT_U32 end_freq;
    /*!
    the channel information in blind scan, this array is malloced by app and used by driver
    */
    mt_unf_fe_channel_info_t *p_channel_info;
    /*!
    the max channel count can be stored in p_channel_info, it is set by app
    */
    MT_U32 max_count;
    /*!
    this time channel number scaned stored in the channel_info, it will be set by driver
    */
    MT_U32 channel_num_cur;
    /*!
    all channel number scaned stored in the channel_info, it will be set by driver
    */
    MT_U32 channel_num_total;
    /*!
    the uni-cable paramter, see nim_unicable_param_t
    */
    mt_unf_fe_unicable_param_t uc_param;

#if 1
    /**<The execution of the blind scan may change the 13/18V or 22K status.
        If you use any DiSEqC device which need send command when 13/18V or 22K status change,
        you should registe a callback here. Otherwise, you can set NULL here.*/
    /**<CNcomment:盲扫过程可能会切换极化方式和22K，如果你用了某些DiSEqC设备需要设置13/18V和22K的，
        请注册这个回调，如果没有用，请可传NULL */
    mt_void (*diseqc_set)(mt_u32 port, mt_unf_fe_polar_t polar,
                          mt_unf_fe_lnb_22k_t lnb_22k);

    /**<Callback when scan status change, scan progress change or find new channel.*/
    /**<CNcomment:扫描状态或进度百分比发生变化时、发现新的频点时回调*/
    //mt_void (*scan_notify)(mt_u32 port, mt_unf_fe_blindscan_evt_t evt, mt_unf_fe_blindscan_notify_t * p_notify);
    //mt_void (*scan_notify)(void *handle, mt_unf_fe_blindscan_evt_t msg, void *p_param);
    mt_void (*scan_notify)(mt_u32 port, mt_unf_fe_blindscan_evt_t msg, void *p_param);
#endif

    mt_u8 lnb_mannual;	//0:auto set lnb in autoscan, 1:set lnb by app
    mt_void (*p_lnbout_and_22k)(mt_u32 port, mt_unf_fe_polar_t polar,
                          mt_unf_fe_lnb_22k_t lnb_22k);//set lnb callback
} mt_unf_fe_sat_blindscan_para_t;

/** Structure of terrestrial scan*/
/** CNcomment:配置TUNER扫描DVB-T/T2信号*/
typedef struct unf_fe_ter_scan_attr_t
{
    mt_u32 freq; /**<Scanning freq,unit KHz*/                 /**<CNcomment:扫描频点的频率，单位是KHz*/
    mt_u32 band_width; /**<Scanning band width,unit KHz*/     /**<CNcomment:扫描频点的带宽，单位是KHz*/
    mt_unf_fe_ter_scan_mode_t scan_mode; /**<Scanning range*/ /**<CNcomment:扫描范围配置*/
    MT_BOOL scan_lite;                                        /**<whether to scan lite signal,0--not scan lite,1--scan base and lite*/
                                                              /**<CNcomment:是否扫描lite信号，0--不搜索lite信号，1--扫描base和lite信号*/
    mt_void (*scan_notify)(mt_u32 port, mt_unf_fe_ter_scan_status_t evt, mt_unf_fe_ter_scan_notify_t *p_notify);
} mt_unf_fe_ter_scan_attr_t;

/** Parameter of the tuner blind scan */
/** CNcomment:TUNER盲扫参数*/
typedef struct unf_fe_blindscan_para_t
{
    mt_unf_fe_blindscan_mode_t mode; /**<Scanning mode*/ /**<CNcomment:盲扫模式*/
    union
    {
	mt_unf_fe_sat_blindscan_para_t sat; /**<Scanning parameter*/ /**<CNcomment:盲扫参数*/
    } scan_para;
} mt_unf_fe_blindscan_para_t;

typedef struct unf_fe_ter_scan_para_t
{
    mt_unf_fe_ter_scan_attr_t ter;
    mt_unf_fe_ter_channel_attr_t chan_array[TER_MAX_TP]; /**<result of tp array*/ /**<CNcomment:扫描频点用数组形式存储*/
    mt_u32 chan_num; /**<number of tp*/                                           /**<CNcomment:频点数目*/
} mt_unf_fe_ter_scan_para_t;

/** Attribute of PLP */
/** CNcomment:物理层管道属性*/
typedef struct unf_fe_ter_plp_attr_t
{
    mt_u8 plp_index; /**<PLP index*/                 /**<CNcomment:物理层管道索引号*/
    mt_u8 plp_id; /**<PLP id*/                       /**<CNcomment:物理层管道ID*/
    mt_u8 plp_grpid; /**<PLP group id*/              /**<CNcomment:物理层管道组ID*/
    mt_unf_fe_t2_plp_type_t plp_type; /**<PLP type*/ /**<CNcomment:物理层管道类型*/
} mt_unf_fe_ter_plp_attr_t;

/** configure lock tp PLP attribute,only in DVB-T2 */
/** CNcomment:在DVB-T2时，配置要锁频点的物理层管道属性*/
typedef struct unf_fe_ter_acc_t
{
    mt_u8 plp_id; /**<PLP id*/                                 /**<CNcomment:物理层管道ID*/
    mt_u8 comm_plpid; /**<common PLP id*/                      /**<CNcomment:共享物理层管道ID*/
    mt_u8 combination; /**<PLP combination*/                   /**<CNcomment:数据物理层管道和共享物理层管道是否组合标志*/
    mt_unf_fe_ter_mode_t channel_attr; /**<channel attribute*/ /**<CNcomment:是否搜索lite信号*/
} mt_unf_fe_ter_acc_t;

/** result of terrestrial scan */
/** CNcomment:地面机单频点扫描结果*/
typedef struct unf_fe_ter_tpinfo_t
{
    mt_u8 progNum; /**<program number*/                             /**<CNcomment:节目数量*/
    mt_u8 dvbt_mode; /**<DVB-T or DVB-T2*/                          /**<CNcomment:信号类型,0--DVB-T2,1--DVB-T*/
    mt_u8 dvbt_hier; /**<delaminate or not*/                        /**<CNcomment:是否分层,0--分层,1--不分层*/
    mt_unf_fe_ter_mode_t channel_attr; /**<base or lite*/           /**<CNcomment:是否带lite信号*/
    mt_unf_fe_ter_channel_mode_t channel_mode; /**<channel mode*/   /**<CNcomment:通道模式*/
    mt_unf_fe_ter_plp_attr_t plp_attr[16]; /**<information of plp*/ /**<CNcomment:物理层管道信息*/
} mt_unf_fe_ter_tpinfo_t;

/** DiSEqC Level*/
/** CNcomment:DiSEqC设备版本*/
typedef enum unf_fe_diseqc_level_t {
    MT_UNF_FE_DISEQC_LEVEL_1_X,
    /**<1.x, one way*/ /**<CNcomment:1.x，单向 */
    MT_UNF_FE_DISEQC_LEVEL_2_X,
    /**<2.x, two way, support reply*/               /**<CNcomment:2.x，双向，支持Reply*/
    MT_UNF_FE_DISEQC_LEVEL_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_diseqc_level_t;

/** Receive status of DiSEqC reply massage*/
/** CNcomment:DiSEqC消息接收状态*/
typedef enum unf_fe_diseqc_recv_status_t {
    MT_UNF_FE_DISEQC_RECV_OK,
    /**<Receive successfully*/ /**<CNcomment:接收成功*/
    MT_UNF_FE_DISEQC_RECV_UNSUPPORT,
    /**<Device don't support reply*/ /**<CNcomment:设备不支持回传*/
    MT_UNF_FE_DISEQC_RECV_TIMEOUT,
    /**<Receive timeout*/ /**<CNcomment:接收超时*/
    MT_UNF_FE_DISEQC_RECV_ERROR,
    /**<Receive fail*/                             /**<CNcomment:接收出错*/
    MT_UNF_FE_DISEQC_RECV_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_diseqc_recv_status_t;

/** Structure of the DiSEqC send massage */
/** CNcomment:DiSEqC发送消息结构*/
typedef struct unf_fe_diseqc_sendmsg_t
{
    mt_unf_fe_diseqc_level_t level; /**<Device level*/           /**<CNcomment:器件版本*/
    mt_unf_fe_switch_toneburst_t tone_burst; /**<Tone Burst */   /**<CNcomment:tone信号状态*/
    mt_u8 data[MT_UNF_DISEQC_MSG_MAX_LENGTH]; /**<Message data*/ /**<CNcomment:消息字*/
    mt_u8 len; /**<Message length*/                              /**<CNcomment:信息长度*/
    mt_u8 repeat_times; /**<Message repeat times*/               /**<CNcomment:重传次数*/
} mt_unf_fe_diseqc_sendmsg_t;

/** Structure of the DiSEqC reply massage */
/** CNcomment:DiSEqC接收消息结构*/
typedef struct unf_fe_diseqc_recvmsg_t
{
    mt_unf_fe_diseqc_recv_status_t status; /**<Recieve status*/         /**<CNcomment:接收状态*/
    mt_u8 msg[MT_UNF_DISEQC_MSG_MAX_LENGTH]; /**<Recieve message data*/ /**<CNcomment:接收数据缓存*/
    mt_u8 len; /**<Recieve message length*/                             /**<CNcomment:接收数据长度*/
} mt_unf_fe_diseqc_recvmsg_t;

/** DiSEqC Switch port*/
/** CNcomment:DiSEqC开关端口枚举*/
typedef enum unf_fe_diseqc_switch_port_t {
    MT_UNF_FE_DISEQC_SWITCH_NONE = 0,
    /**<none*/ /**<CNcomment:不接开关*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_1,
    /**<port1*/ /**<CNcomment:端口1*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_2,
    /**<port2*/ /**<CNcomment:端口2*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_3,
    /**<port3*/ /**<CNcomment:端口3*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_4,
    /**<port4*/ /**<CNcomment:端口4*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_5,
    /**<port5*/ /**<CNcomment:端口5*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_6,
    /**<port6*/ /**<CNcomment:端口6*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_7,
    /**<port7*/ /**<CNcomment:端口7*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_8,
    /**<port8*/ /**<CNcomment:端口8*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_9,
    /**<port9*/ /**<CNcomment:端口9*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_10,
    /**<port10*/ /**<CNcomment:端口10*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_11,
    /**<port11*/ /**<CNcomment:端口11*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_12,
    /**<port12*/ /**<CNcomment:端口12*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_13,
    /**<port13*/ /**<CNcomment:端口13*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_14,
    /**<port14*/ /**<CNcomment:端口14*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_15,
    /**<port15*/ /**<CNcomment:端口15*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_16,
    /**<port16*/                                          /**<CNcomment:端口16*/
    MT_UNF_FE_DISEQC_SWITCH_PORT_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_diseqc_switch_port_t;

/** Parameter for DiSEqC 1.0/2.0 switch
   Some DiSEqC device need set polarization(13/18V) and 22K, you can set them here. */
/** CNcomment:DiSEqC 1.0/2.0 开关参数
   有些DiSEqC设备需要设置极化方式和22K的，如果用了这种设备，需要在这里设置 */
typedef struct unf_fe_diseqc_switch4port_t
{
    mt_unf_fe_diseqc_level_t level; /**<DiSEqC device level*/     /**<CNcomment:器件版本*/
    mt_unf_fe_diseqc_switch_port_t port; /**<DiSEqC switch port*/ /**<CNcomment:选通端口号*/
    mt_unf_fe_polar_t polar; /**<Polarization type */             /**<CNcomment:极化方式*/
    mt_unf_fe_lnb_22k_t lnb_22k; /**<22K status*/                 /**<CNcomment:22k状态*/
} mt_unf_fe_diseqc_switch4port_t;

/** Parameter for DiSEqC 1.1/2.1 switch */
/** CNcomment:DiSEqC 1.1/2.1 开关参数 */
typedef struct unf_fe_diseqc_switch16port_t
{
    mt_unf_fe_diseqc_level_t level; /**<DiSEqC device level*/     /**<CNcomment:器件版本*/
    mt_unf_fe_diseqc_switch_port_t port; /**<DiSEqC switch port*/ /**<CNcomment:选通端口号*/
} mt_unf_fe_diseqc_switch16port_t;

/** DiSEqC motor limit setting*/
/** CNcomment:DiSEqC马达极限设置*/
typedef enum unf_fe_diseqc_dir_limit_t {
    MT_UNF_FE_DISEQC_LIMIT_OFF,
    /**<Disable Limits*/ /**<CNcomment:无限制*/
    MT_UNF_FE_DISEQC_LIMIT_EAST,
    /**<Set East Limit*/ /**<CNcomment:东向限制*/
    MT_UNF_FE_DISEQC_LIMIT_WEST,
    /**<Set West Limit*/                            /**<CNcomment:西向限制*/
    MT_UNF_FE_DISEQC_LIMIT_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_diseqc_dir_limit_t;

/** Difinition of DiSEqC motor move direction*/
/** CNcomment:DiSEqC马达移动方向*/
typedef enum unf_fe_diseqc_movedir_t {
    MT_UNF_FE_DISEQC_MOVE_DIR_EAST,
    /**<Move east*/ /**<CNcomment:向东移动*/
    MT_UNF_FE_DISEQC_MOVE_DIR_WEST,
    /**<Move west*/                                    /**<CNcomment:向西移动*/
    MT_UNF_FE_DISEQC_MOVE_DIR_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_diseqc_move_dir_t;

/** Difinition of DiSEqC motor move type*/
/** CNcomment:DiSEqC马达移动方式*/
typedef enum unf_fe_diseqc_move_type_t {
    MT_UNF_FE_DISEQC_MOVE_STEP_SLOW,
    /**<1 step one time, default*/ /**<CNcomment:缓慢移动*/
    MT_UNF_FE_DISEQC_MOVE_STEP_FAST,
    /**<5 step one time*/ /**<CNcomment:快速移动*/
    MT_UNF_FE_DISEQC_MOVE_CONTINUE,
    /**<Continuous moving*/                             /**<CNcomment:连续移动*/
    MT_UNF_FE_DISEQC_MOVE_TYPE_BUTT /**<Invalid value*/ /**<CNcomment:非法边界值*/
} mt_unf_fe_diseqc_move_type_t;

/** Parameter for DiSEqC motor store position*/
/** CNcomment:天线存储位置参数*/
typedef struct unf_fe_diseqc_position_t
{
    mt_unf_fe_diseqc_level_t level; /**<DiSEqC device level*/ /**<CNcomment:器件版本*/
    mt_u32 pos; /**<Index of position, 0-255*/                /**<CNcomment:位置序号*/
} mt_unf_fe_diseqc_position_t;

/** Parameter for DiSEqC motor limit setting*/
/** CNcomment:天线Limit设置参数*/
typedef struct unf_fe_diseqc_limit_t
{
    mt_unf_fe_diseqc_level_t level; /**<DiSEqC device level*/ /**<CNcomment:器件版本*/
    mt_unf_fe_diseqc_dir_limit_t limit; /**<Limit setting*/   /**<CNcomment:限制设定*/
} mt_unf_fe_diseqc_limit_t;

/** Parameter for DiSEqC motor moving*/
/** CNcomment:DiSEqC马达移动参数*/
typedef struct unf_fe_diseqc_move_t
{
    mt_unf_fe_diseqc_level_t level; /**<DiSEqC device level*/ /**<CNcomment:器件版本*/
    mt_unf_fe_diseqc_move_dir_t dir; /**<Moving direction*/   /**<CNcomment:移动方向*/
    mt_unf_fe_diseqc_move_type_t type; /**<Moving type*/      /**<CNcomment:移动类型(慢速，快速，连续)*/
} mt_unf_fe_diseqc_move_t;

/** Parameter for DiSEqC motor recalculate*/
/** CNcomment:DiSEqC天线重计算参数*/
typedef struct unf_fe_diseqc_recalculate_t
{
    mt_unf_fe_diseqc_level_t level; /**<DiSEqC device level*/ /**<CNcomment:器件版本*/
    mt_u8 para1; /**<Parameter 1*/                            /**<CNcomment:参数1*/
    mt_u8 para2; /**<Parameter 2*/                            /**<CNcomment:参数2*/
    mt_u8 para3; /**<Parameter 3*/                            /**<CNcomment:参数3*/
    mt_u8 reserve; /**<Reserve*/                              /**<CNcomment:保留参数*/
} mt_unf_fe_diseqc_recalculate_t;

/** Parameter for USALS*/
/** CNcomment:USALS 参数*/
typedef struct unf_fe_diseqc_usals_para_t
{
    mt_u16 local_longitude; /**<local longitude, is 10*longitude, in param, E:0-1800, W:1800-3600(3600-longtitude)*/
                            /**<CNcomment:本地经度，单位0.1度，东经取值范围0-1800，西经取值范围1800-3600，值为3600-经度值*/
    mt_u16 local_latitude;  /**<local latitude, is 10*latitude, in param N:0-900, S:900-1800(1800-latitude)*/
                            /**<CNcomment:本地纬度，单位0.1度，北纬取值范围0-900，南纬取值范围900-1800，值为1800-纬度值*/
    mt_u16 sat_longitude;   /**<sat longitude, is 10*longitude, in param, E:0-1800, W:1800-3600(3600-longtitude)*/
                            /**<CNcomment:卫星经度，单位0.1度，东经取值范围0-1800，西经取值范围1800-3600，值为3600-经度值*/
    mt_u16 angular;         /**<calculate result, out param*/
                            /**<CNcomment:计算结果，输出参数*/
} mt_unf_fe_diseqc_usals_para_t;

/** Parameter for USALS goto angular*/
/** CNcomment:USALS角度参数*/
typedef struct unf_fe_diseqc_usals_angular_t
{
    mt_unf_fe_diseqc_level_t level; /**<DiSEqC device level*/                      /**<CNcomment:器件版本*/
    mt_u16 angular; /**<Angular, calculated by MT_UNF_TUNER_DISEQC_CalcAngular()*/ /**<CNcomment:角度值，可通过函数MT_UNF_TUNER_DISEQC_CalcAngular计算得到*/
} mt_unf_fe_diseqc_usals_angular_t;

/** One user band information of unicable*/
/** CNcomment:unicable的单个用户频段信息*/
typedef struct mtunf_fe_scr_ub_t
{
    mt_u32 scr_no; /**<user band number of unicable*/             /**<CNcomment:unicable的用户频段号*/
    mt_s32 center_freq; /**<user band freq of unicable,unit MHz*/ /**<CNcomment:unicable的用户频段中心频率,单位MHz*/
} mt_unf_fe_scr_ub_t;                                             //MT_UNF_TUNER_SCR_UB_S;
//#endif /* CONFIG_MT_DISEQC_SUPPORT */

/** @} */ /** <!-- ==== Structure Definition end ==== */

#endif

/*!
  @brief DiSEqC mode, used for IO control "NIM_IOCTRL_DISEQC1X" and
  "NIM_IOCTRL_DISEQC2X" commands
  */
typedef enum _unf_fe_port_diseqc_mode_t {
    /*!
    Burst mode, on for 12.5mS = 0
    */
    PORT_DISEQC_BURST0 = 0,
    /*!
    Burst mode, modulated 1:2 for 12.5mS = 1
    */
    PORT_DISEQC_BURST1,
    /*!
    Modulated with bytes for DISEQC instructions
    */
    PORT_DISEQC_BYTES
} mt_unf_fe_port_diseqc_mode_t;

/*!
  @brief Polarization, used for IO control "NIM_IOCTRL_SET_PORLAR" command
  */
typedef enum _unf_fe_port_lnb_power_t {
    /*!
        LNB power on
      */
    PORT_LNB_POWER_ON = 0x01,
    /*!
        LNB power off
      */
    PORT_LNB_POWER_OFF = 0x00,
    /*!
        LNB short circuit protection happened
      */
    PORT_LNB_SC_PROTING = 0x02,
    /*!
        LNB short circuit protection not happened
      */
    PORT_LNB_SC_NO_PROTING = 0x03,
} mt_unf_fe_port_lnb_power_t;
/*!
  @brief BBHeader TS/GS mode, used for IO control "NIM_IOCTRL_SET_PORLAR" command
  */
typedef enum _unf_fe_bbh_ts_gs_mode_t {
	BBHEADER_UNKNOW = 0x00,
    /*!
        BBHeader is Transport Mode
      */
    BBHEADER_TRANSPORT_MODE = 0x02,
    /*!
        BBHeader is Generic Packetized Mode
      */
    BBHEADER_GENERIC_CONTINUOUS_MODE = 0x03,
    /*!
        BBHeader is GSE-HEM Mode
      */
    BBHEADER_GSE_HEM_MODE = 0x04,
    /*!
        BBHeader is Generic Packetized Mode
      */
    BBHEADER_GENERIC_PACKETIZED_MODE = 0x05,
} mt_unf_fe_bbh_ts_gs_mode_t;

/*!
  @brief gs_package_mode, used for IO control "NIM_IOCTRL_SET_PORLAR" command
  */
typedef enum _unf_fe_gs_package_mode_t {

     /*!
        gs date process in gse mode
      */
	PROCESS_IN_GSE_MODE = 0x00,
    /*!
        gs date process in bbframe mode
      */
    PROCESS_IN_BBFRAME_MODE = 0x01,
    /*!
        BBHeader is Generic Packetized Mode
      */
    PROCESS_IN_UKNOW_MODE = 0xFF,
} mt_unf_fe_gs_package_mode_t;

/*!
  @brief gse label filter type, used for IO control "" command
  */
typedef enum _unf_fe_ges_label_filter_type_t {

     /*!
        The length of gse label is 3 bytes
      */
	GSE_LABEL_3_BYTE_MODE = 0x00,
    /*!
        The length of gse label is 6 bytes
      */
    GSE_LABEL_6_BYTE_MODE = 0x01,
    /*!
        The unkown length
      */
    GSE_LABEL_UNKONW_MODE = 0xFF,
} mt_unf_fe_ges_label_filter_type_t;
/*!
  @brief bbframe packing padding, used for IO control "" command
  */
typedef enum _unf_fe_bbframe_packing_type_t {

     /*!
        packing bbframe without padding
      */
	PACKING_BBFRAME_WITHOUT_PADDING = 0x00,
    /*!
        packing bbframe include padding
      */
    PACKING_BBFRAME_INCLUDE_PADDING = 0x01,
    /*!
        The unkown 
      */
    PACKING_BBFRAME_UNKONW_MODE = 0xFF,
} mt_unf_fe_bbframe_packing_type_t;
/*!
    NIM types used for "struct nim_config" "nim_type"
  */
#if 0
typedef enum _unf_fe_port_type_t {
    /*!
      NIM for undefined type
     */
    PORT_UNDEF = 0x00,
    /*!
      NIM for DVB-S
     */
    PORT_DVBS = 0x01,
    /*!
      NIM for DVB-S2
     */
    PORT_DVBS2 = 0x02,
    /*!
      NIM for DVB-C
     */
    PORT_DVBC = 0x04,
    /*!
      NIM for ABS-S
     */
    PORT_ABSS = 0x05,
    /*!
      NIM for DVB-T
     */
    PORT_DVBT = 0x06,
    /*!
      NIM for DVB-T2
     */
    PORT_DVBT2 = 0x07,
    /*!
      NIM for test mode
     */
    PORT_TEST = 0x08,
    /*!
      NIM for for DVB-T only
     */
    PORT_DVBT_ONLY = 0x09,
    /*!
      NIM for for DVB-T2 only
     */
    PORT_DVBT2_ONLY = 0x0a,
    /*!
      NIM for for DVBT  auto  will sercher dvbt2 first, if not lock then dvbt,
     */
    PORT_DVBT_AUTO = 0x0b,
    /*!
      NIM for for DVBS  auto  will sercher dvbs2 first, if not lock then dvbs,
     */
    PORT_DVBS_AUTO = 0x0c,
} mt_unf_fe_port_type_t;
#endif

typedef struct _unf_fe_blind_scan_channel_info_t
{
    /*!
    channel is locked or not, see MT_UNF_FE_CHANNEL_LOCK_S
    */
    mt_u8 lock;
    /*!
    NIM type, see enum nim_type_t
    */
    //mt_u8 nim_type;
    /*!
    Channel frequency in KHz
    */
    mt_u32 frequency;
    /*!
    symbol rate in Symbols per second, in KSs
    */
    mt_u32 symbol_rate;
    /*!
    forward error correction, see enum nim_code_rate_t
    */
    //mt_u8 fec_inner;
    /*!
    performance
    */
    //MT_UNF_FE_CHANNEL_PEF_S perf;
} mt_unf_fe_blind_scan_channel_info_t;

/*!
  @brief DiSEqC param, used for IO control "NIM_IOCTRL_DISEQC1X"
         and "NIM_IOCTRL_DISEQC2X" command
  */
typedef struct _unf_fe_diseqc_cmd_t
{
    /*!
    diseqc mode, see nim_diseqc_mode_t
    */
    mt_u8 mode;
    /*!
    DiSEqC 2.x command result, see nim_diseqc_error_t
    */
    mt_u8 result;
    /*!
    DiSEqC TX command string length in bytes
    */
    mt_u8 tx_len;
    /*!
    DiSEqC 2.x RX command string length in bytes
    */
    mt_u8 rx_len;
    /*!
    DiSEqC TX command string
    */
    mt_u8 p_tx_buf[10];
    /*!
    DiSEqC 2.x RX command string
    */
    mt_u8 p_rx_buf[10];
} mt_unf_fe_diseqc_cmd_t;

/*!
@brief tuner query item
*/
typedef struct _unf_fe_query_item_t
{
    mt_unf_tuner_type_t tuner_type;
    mt_u8 i2c_addr;
} mt_unf_fe_query_item_t;

/*!
  NIM notify message
  */
typedef enum _unf_fe_blind_scan_msg {
    /*!
    blind scan find tp
    */
    PORT_BSTpFind,
    /*!
    blind scan tp locked
    */
    PORT_BSTpLocked,
    /*!
    blind scan tp unlocked
    */
    PORT_BSTpUnlock,
    /*!
    blind scan start
    */
    PORT_BSStart,
    /*!
    blind scan finish
    */
    PORT_BSFinish,
    /*!
    blind scan one window start
    */
    PORT_BSOneWinFinish,
    /*!
    blind scan abort
    */
    PORT_BSAbort,
    /*!
    blind scan max item
    */
    PORT_BSMAX
} mt_unf_fe_blind_scan_msg;

typedef struct _unf_fe_port_notify_param
{
    mt_unf_fe_blind_scan_msg msg;
    mt_u8 used;
    mt_unf_fe_blind_scan_channel_info_t bs_channel_info;
} mt_unf_fe_port_notify_param;

typedef struct _unf_fe_port_notify_info
{
    mt_u8 notify_count;
    mt_unf_fe_port_notify_param *p_notify_param;
} mt_unf_fe_port_notify_info, *mt_unf_fe_port_notify_info_t;

/*!
  DISEQC2 message info
  */
typedef struct _unf_fe_diseqc2_rsmsg_info
{
    mt_u8 sendmsg[MT_UNF_DISEQC_MSG_MAX_LENGTH];
    mt_u8 rcvmsg[MT_UNF_DISEQC_MSG_MAX_LENGTH];
    mt_u8 slen;
    mt_u8 rlen;
} mt_unf_fe_diseqc2_rsmsg_info, *mt_unf_fe_diseqc2_rsmsg_info_t;


/**scid Filter attribute*/
/**CNcomment: SCID 过滤器属性*/
typedef struct _unf_fe_dss_scid_filter_t
{
    mt_u8 tuner_id;
    MT_BOOL b_filter_mode;    /**<b_filter_mode. 1:match pattern to filter ;  0:match pattern discard*/ /**< CNcomment: 1 : 匹配就过滤器 0: 匹配就丢弃， 不能同时使用*/
    mt_u16  u16_scid[DIRECTTV_SCID_FILTER_MAX_COUNT];     /**<Matched bytes of a filter */ /**< CNcomment:过滤器匹配字节*/
    mt_u16  u16_mask[DIRECTTV_SCID_FILTER_MAX_COUNT];   /**<Masked bytes of a filter. The conditions are set by bit. 0: no mask. Comparison is required. 1: mask. Comparison is not required.*/ /**< CNcomment:过滤器屏蔽字节,按bit设置, 0:没有mask，要进行比较, 1:mask起作用，不进行比较*/
} mt_unf_fe_dss_scid_filter_t;
/*!
    GSE Label filter info
*/
typedef struct _unf_fe_gse_label_filter_t
{
    MT_BOOL b_filter_mode;    /**<b_filter_mode. 1:match pattern to filter ;  0:match pattern to pass*/ /**< CNcomment: 1 : 匹配就过滤器 0: 匹配就丢弃， 不能同时使用*/
    mt_unf_fe_ges_label_filter_type_t label_mode;
    mt_u8 label[6];
    mt_u8 label_index;
} mt_unf_fe_gse_label_filter;


/*!
@~chinese
@addtogroup tuner_api_declare  Demodulator&Tuner模块
@{
@brief Demodulator&Tuner模块接口说明
@details \n
Demodulator&Tuner模块的使用需要严格按照下面的初始化步骤才能正常使用:
1. 调用MT_UNF_TUNER_Init
2. 调用MT_UNF_TUNER_Open
3. 调用MT_UNF_TUNER_SetAttr
\n
以上步骤都成功后，才可以正常使用TUNER API。

@b 示例:
@~english
@addtogroup tuner_api_declare  Demodulator&Tuner
@{
@brief Demodulator&Tuner modules API description
@details \n
The use of the Demodulator&Tuner module needs to be strictly in accordance with the following
initialization steps to normal use:
1. call MT_UNF_TUNER_Init
2. call MT_UNF_TUNER_Open
3. call MT_UNF_TUNER_SetAttr
\n
After the above steps are successful, just can normally use Tuner.

@b Sample:

@~
@code{.c}

int main(void)
{
    int ret = 0;
    mt_unf_fe_attr_t attr;
    mt_unf_fe_connect_para_t cpara;
    mt_unf_fe_status_t st;
    ret = MT_UNF_TUNER_Init();
    if(ret < 0)
    {
        printf("MT_UNF_TUNER_Init fail err=%d\n", ret);
        return ret;
    }
    ret = MT_UNF_TUNER_Open(0);
    if(ret < 0)
    {
        printf("MT_UNF_TUNER_Open fail err=%d\n", ret);
        return ret;
    }

    ret = MT_UNF_TUNER_GetDeftAttr(0, &attr);
    if(ret < 0)
    {
        printf("MT_UNF_TUNER_GetDeftAttr fail err=%d\n", ret);
        return ret;
    }
    ret = MT_UNF_TUNER_SetAttr(0, &attr);
    if(ret < 0)
    {
        printf("MT_UNF_TUNER_GetDeftAttr fail err=%d\n", ret);
        return ret;
    }
    cpara.enSigType = MT_UNF_TUNER_SIG_TYPE_CAB;
    cpara.unConnectPara.stCab.enModType = MT_UNF_MOD_TYPE_QAM_64;
    cpara.unConnectPara.stCab.u32Freq = 299000;
    cpara.unConnectPara.stCab.u32SymbolRate = 6900000;
    cpara.unConnectPara.stCab.bReverse = 0;
    printf("start lock\n");

    ret = MT_UNF_TUNER_Connect(0, &cpara, 2000);
    if(ret < 0)
    {
        printf("MT_UNF_TUNER_GetDeftAttr fail err=%d\n", ret);
        return ret;
    }
    printf("locked\n");
    return 0;
}
@endcode
*/

/*!
@brief Tuner module initialization
@return ::MT_SUCCESS
*/
mt_s32 mt_unf_fe_init(mt_void);

/*!
@brief Tuner module deinitialize
@return ::MT_SUCCESS
*/
mt_s32 mt_unf_fe_deinit(mt_void);

/*!
@brief Get the default Tuner attribute
@param [in] tuner_id Tuner ID
@param [out] pstTunerAttr Tuner default attribute
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
*/
mt_s32 mt_unf_fe_get_default_attr(mt_u32 tuner_id, mt_unf_fe_attr_t *p_fe_attr);

/*!
@brief Set the tuner attribute
@param [in] tuner_id Tuner ID
@param [in] pstTunerAttr Tuner attribute
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
*/
mt_s32 mt_unf_fe_set_attr(mt_u32 tuner_id, const mt_unf_fe_attr_t *p_fe_attr);

mt_s32 mt_unf_fe_set_sat_attr(mt_u32 tuner_id, const mt_unf_fe_sat_attr_t *p_sat_fe_attr);

mt_s32 mt_unf_fe_set_ter_attr(mt_u32 tuner_id, const mt_unf_fe_ter_attr_t *p_ter_fe_attr);

/*!
@brief Get the tuner attribute
@param [in] tuner_id Tuner ID
@param [out] pstTunerAttr Tuner attribute
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
*/
mt_s32 mt_unf_fe_get_attr(mt_u32 tuner_id, mt_unf_fe_attr_t *p_fe_attr);

/*!
@brief open tuner device
@param [in] tuner_id Tuner ID
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_OPEN_FAIL
*/
mt_s32 mt_unf_fe_open(mt_u32 tuner_id);

/*!
@brief close tuner device
@param [in] tuner_id Tuner ID
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_CLOSE_FAIL

*/
mt_s32 mt_unf_fe_close(mt_u32 tuner_id);

/*!
@brief set tuner frequency
@param [in] tuner_id      ---- Tuner ID
@param [in] p_tuner_param ---- Tuner parameters
@return ::MT_SUCCESS
@return ::MT_ERR_FE_INVALID_PORT
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_ERR_FE_INVALID_POINT
*/

mt_s32 mt_unf_fe_set_tuner_param(mt_u32 tuner_id, mt_unf_fe_tuner_param_t *p_tuner_param);

/*!
@brief frequency locking
@param [in] tuner_id Tuner ID
@param [in] pconn_para frequency parameter
@param [in] u32TimeOut  Lock timeout value ,unit ms
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_UNF_TUNER_ERR_CONNECT_FAIL
*/
mt_s32 mt_unf_fe_connect(mt_u32 tuner_id, mt_unf_fe_connect_para_t *pconn_para, mt_u32 timeout);

mt_s32 mt_unf_fe_set_ts_out(mt_u32 tuner_id, mt_unf_fe_ts_out_t *p_ts_out);

/*!
@brief Get tuner lock status fastly
@details Tuner status only contains lock status.
         This API function is recommended for use in the star finder project.
         For other projects, it is advised to use mt_unf_fe_get_status functions.
@param [in] tuner_id Tuner ID
@param [out] pstTunerStatus Tuner status
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_UNF_TUNER_ERR_GET_STATUS_FAIL
*/
mt_s32 mt_unf_fe_get_fast_lock(mt_u32 tuner_id, mt_unf_fe_status_t *p_status);

/*!
@brief Get tuner status
@details Tuner status contains the current lock parameters and lock status
@param [in] tuner_id Tuner ID
@param [out] pstTunerStatus Tuner status
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_UNF_TUNER_ERR_GET_STATUS_FAIL
*/
mt_s32 mt_unf_fe_get_status(mt_u32 tuner_id, mt_unf_fe_status_t *p_status);

/*!
@~chinese
@brief 获取误码率
@details \n
                pu32BER[0]:误码率底数的整数部分\n
                pu32BER[1]:误码率底数的小数部分乘以1000\n
                pu32BER[2]:误码率指数部分取绝对值
@param [in] tuner_id Tuner ID
@param [out] pu32BER 误码率
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_UNF_TUNER_ERR_GET_BER_FAIL

@~english
@brief Get bit error rate
@details \n
                pu32BER[0]:The integer part of the bit error rate base\n
                pu32BER[1]:The bit error rate of the base fraction multiplied by 1000\n
                pu32BER[2]:Absolute value of bit error rate index
@param [in] tuner_id Tuner ID
@param [out] pu32BER bit error rate
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_UNF_TUNER_ERR_GET_BER_FAIL

*/
mt_s32 mt_unf_fe_get_ber(mt_u32 tuner_id, mt_u32 *p_ber);

/*!
@~chinese
@brief 获取误码率(PRE-BER)
@details \n
                pu32BER[0]:误码率底数的整数部分\n
                pu32BER[1]:误码率底数的小数部分乘以1000\n
                pu32BER[2]:误码率指数部分取绝对值
@param [in] tuner_id Tuner ID
@param [out] pu32BER 误码率
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_UNF_TUNER_ERR_GET_BER_FAIL

@~english
@brief Get pre - bit error rate
@details \n
                pu32BER[0]:The integer part of the bit error rate base\n
                pu32BER[1]:The bit error rate of the base fraction multiplied by 1000\n
                pu32BER[2]:Absolute value of bit error rate index
@param [in] tuner_id Tuner ID
@param [out] pu32BER bit error rate
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_UNF_TUNER_ERR_GET_BER_FAIL

*/
mt_s32 mt_unf_fe_get_pre_ber(mt_u32 tuner_id, mt_u32 *p_ber);
/*!	
@brief Obtain signal to noise ratio	@brief Obtain signal to noise ratio
@param [in] tuner_id Tuner ID	@param [in] tuner_id Tuner ID	
@param [out] ps32SNR signal to noise ratio in 0.001dB	
@return ::MT_SUCCESS	
@return ::MT_UNF_TUNER_ERR_UNINIT	
			@return ::MT_UNF_TUNER_ERR_ID_OVER	
@return ::MT_UNF_TUNER_ERR_UNOPEN	
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET	
@return ::MT_UNF_TUNER_ERR_GET_SNR_FAIL	

*/
mt_s32 mt_unf_fe_get_accurate_snr(mt_u32 tuner_id, mt_s32 *p_snr); /* unit: 0.001dB  */
/*!
@brief Obtain signal to noise ratio
@param [in] tuner_id Tuner ID
@param [out] pu32SNR signal to noise ratio
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_UNF_TUNER_ERR_GET_SNR_FAIL

*/
mt_s32 mt_unf_fe_get_snr(mt_u32 tuner_id, mt_u32 *p_snr);

/*!
@brief Get signal strength
@param [in] tuner_id Tuner ID
@param [out] pu32SignalStrength signal strength
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_UNF_TUNER_ERR_GET_SIG_STRENGTH_FAIL

*/
mt_s32 mt_unf_fe_get_signal_strength(mt_u32 tuner_id, mt_u32 *p_signal_strength);

/*!
@brief Get singal quality
@param [in] tuner_id Tuner ID
@param [out] pu32SignalQuality singal quality
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
@return ::MT_UNF_TUNER_ERR_GET_SIG_QUALITY_FAIL

*/
mt_s32 mt_unf_fe_get_signal_quality(mt_u32 tuner_id, mt_u32 *p_signal_quality);

//mt_s32 mt_unf_fe_get_real_freq_symb(mt_u32 tuner_id, mt_u32 *p_freq, mt_u32 *p_symb);
mt_s32 mt_unf_fe_get_real_freq_symb(mt_u32 tuner_id, mt_u32 *pu32Freq, mt_u32 *pu32Symb, mt_s32 *ps32FreqOffset);
mt_s32 mt_unf_fe_get_s2_multi_stream_info(mt_u32 tuner_id, mt_unf_fe_connect_para_t *p_connect_para);
mt_s32 mt_unf_fe_set_s2_multi_stream_ts_id(mt_u32 tuner_id, mt_u8 ts_id);
mt_s32 mt_unf_fe_get_signal_info(mt_u32 tuner_id, mt_unf_fe_signal_info_t *p_signal_info);
mt_s32 mt_unf_fe_set_lnb_config(mt_u32 tuner_id, mt_unf_fe_lnb_config_t *p_lnb);
mt_s32 mt_unf_fe_set_lnb_power(mt_u32 tuner_id, mt_unf_fe_lnb_power_t en_lnb_power);
mt_s32 mt_unf_fe_set_plpid(mt_u32 tuner_id, mt_u8 plp_id);
mt_s32 mt_unf_fe_set_plp_mode(mt_u32 tuner_id, mt_u8 mode);
mt_s32 mt_unf_fe_set_common_plpid(mt_u32 tuner_id, mt_u8 plpid);
mt_s32 mt_unf_fe_set_common_plp_combination(mt_u32 tuner_id, mt_u8 plpid);
mt_s32 mt_unf_fe_get_plpnum(mt_u32 tuner_id, mt_u8 *p_plpnum);
mt_s32 mt_unf_fe_get_current_plptype(mt_u32 tuner_id, mt_unf_fe_t2_plp_type_t *p_plptype);
mt_s32 mt_unf_fe_get_plpid(mt_u32 tuner_id, mt_u8 *p_plpid);
mt_s32 mt_unf_fe_get_plp_grpid(mt_u32 tuner_id, mt_u8 *p_plpgrpid);

mt_s32 mt_unf_fe_get_hierarchy_num(mt_u32 tuner_id, mt_u8 *p_hier_num);
mt_s32 mt_unf_fe_set_hierarchy_id(mt_u32 tuner_id, mt_u8 hier_id);
mt_s32 mt_unf_fe_get_hierarchy_id(mt_u32 tuner_id, mt_u8 *p_hier_id);

mt_s32 mt_unf_fe_get_t_t2_cell_id(mt_u32 tuner_id, mt_u16 *p_cell_id);
mt_s32 mt_unf_fe_set_antenna_power(mt_u32 tuner_id, mt_unf_fe_ter_antenna_power_t en_power);
mt_s32 mt_unf_fe_get_antenna_power(mt_u32 tuner_id, mt_unf_fe_ter_antenna_power_t *p_en_power);

mt_s32 mt_unf_fe_blind_scan(mt_u32 tuner_id, mt_unf_fe_blindscan_para_t *p_scan_info);
mt_s32 mt_unf_fe_check_bs_status(mt_u32 tuner_id, mt_u8 *status);
mt_s32 mt_unf_fe_blindscan_cancel(mt_u32 tuner_id);
mt_s32 mt_unf_fe_get_blindscan_result(mt_u32 tuner_id, void *para);

mt_s32 mt_unf_fe_blindscan_start(mt_u32 tuner_id, const mt_unf_fe_blindscan_para_t *p_prm);
mt_s32 mt_unf_fe_blindscan_stop(mt_u32 tuner_id);
//mt_s32 mt_unf_fe_ter_scanstart(mt_u32 tuner_id, mt_unf_fe_ter_scan_para_t *p_prm);
//mt_s32 mt_unf_fe_ter_scanstop(mt_u32 tuner_id);
mt_s32 mt_unf_fe_ter_scan_start(mt_u32 tuner_id, mt_unf_fe_ter_scan_para_t *p_terscan);
mt_s32 mt_unf_fe_ter_scan_stop(mt_u32 tuner_id);
mt_s32 mt_unf_fe_standby(mt_u32 tuner_id);
mt_s32 mt_unf_fe_wakeup(mt_u32 tuner_id);

mt_s32 mt_unf_fe_switch_0_12v(mt_u32 tuner_id, mt_unf_fe_switch_0_12v_t en_port);
mt_s32 mt_unf_fe_switch_22k(mt_u32 tuner_id, mt_unf_fe_switch_22k_t en_port);
mt_s32 mt_unf_fe_switch_toneburst(mt_u32 tuner_id, mt_unf_fe_switch_toneburst_t status);
mt_s32 mt_unf_fe_diseqc_switch4port(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch4port_t *p_prm);
mt_s32 mt_unf_fe_diseqc_switch16port(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch16port_t *p_prm);
mt_s32 mt_unf_fe_diseqc_storepos(mt_u32 tuner_id, const mt_unf_fe_diseqc_position_t *p_prm);
mt_s32 mt_unf_fe_diseqc_goto_pos(mt_u32 tuner_id, const mt_unf_fe_diseqc_position_t *p_prm);
mt_s32 mt_unf_fe_diseqc_set_limit(mt_u32 tuner_id, const mt_unf_fe_diseqc_limit_t *p_prm);
mt_s32 mt_unf_fe_diseqc_move(mt_u32 tuner_id, const mt_unf_fe_diseqc_move_t *p_prm);
mt_s32 mt_unf_fe_diseqc_stop(mt_u32 tuner_id, const mt_unf_fe_diseqc_level_t level);
mt_s32 mt_unf_fe_diseqc_recalculate(mt_u32 tuner_id, const mt_unf_fe_diseqc_recalculate_t *p_prm);

mt_s32 mt_unf_fe_diseqc_calc_angular(mt_u32 tuner_id, mt_unf_fe_diseqc_usals_para_t *p_prm);
mt_s32 mt_unf_fe_diseqc_goto_angular(mt_u32 tuner_id, const mt_unf_fe_diseqc_usals_angular_t *p_prm);
mt_s32 mt_unf_fe_diseqc_reset(mt_u32 tuner_id, mt_unf_fe_diseqc_level_t level);
mt_s32 mt_unf_fe_diseqc_standby(mt_u32 tuner_id, mt_unf_fe_diseqc_level_t level);
mt_s32 mt_unf_fe_diseqc_wakeup(mt_u32 tuner_id, mt_unf_fe_diseqc_level_t level);
mt_s32 mt_unf_fe_get_constellation_data(mt_u32 tuner_id, mt_unf_fe_sample_datalen_t data_len, mt_unf_fe_sample_data_t *p_data);
mt_s32 mt_unf_fe_get_spectrum_data(mt_u32 tuner_id, mt_unf_fe_sample_datalen_t data_len, mt_u32 *p_data);
mt_s32 mt_unf_fe_get_default_timeout(mt_u32 tuner_id, const mt_unf_fe_connect_para_t *p_connect_para, mt_u32 *p_timeout_ms);
mt_s32 mt_unf_fe_get_agc(mt_u32 tuner_id, mt_s32 center_freq, mt_s32 *p_agc);

mt_s32 mt_unf_fe_unicable_lofreq(mt_u32 tuner_id, mt_u8 scr_no, mt_u8 lofreq_no);

mt_s32 mt_unf_fe_unicable_power_off(mt_u32 tuner_id, mt_u8 scr_no);
mt_s32 mt_unf_fe_unicable_scrx_on(mt_u32 tuner_id);
mt_s32 mt_unf_fe_unicable_config(mt_u32 tuner_id, mt_u8 scr_no, mt_u8 app_no);

//mt_s32 mt_unf_fe_check_lnb_localfreq(mt_u32 tuner_id, mt_u8 scr_no, mt_u8 lofreq_no);
mt_s32 mt_unf_fe_scan_and_install_unicable(mt_u32 tuner_id);
mt_s32 mt_unf_fe_get_unicable_info(mt_u32 tuner_id, mt_unf_fe_scr_ub_t *p_ubinfo);
mt_s32 mt_unf_fe_set_current_unicable(mt_u32 tuner_id);

/*!
@brief Control the tuner's IO port
@param [in] tuner_id Tuner ID
@param [in] OnOff  Tuner's IO port on or off
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
@return ::MT_UNF_TUNER_ERR_ATTR_UNSET
*/
mt_s32 mt_unf_fe_set_io(mt_u32 tuner_id, MT_BOOL on_off);

/*!
@brief Query the current tuner device
@param [in] tuner_id Tuner ID
@param [in] pItems Reference tuner array
@param [in] item_num Array size
@param [out] pType Matched tuner type
@return ::MT_SUCCESS
@return ::MT_UNF_TUNER_ERR_UNINIT
@return ::MT_UNF_TUNER_ERR_ID_OVER
@return ::MT_UNF_TUNER_ERR_UNOPEN
*/
mt_s32 mt_unf_fe_query(mt_u32 tuner_id, mt_unf_fe_query_item_t *p_items, mt_u8 item_num, mt_unf_tuner_type_t *p_type);
mt_s32 mt_unf_fe_set_lnb_onoff(mt_u32 tuner_id, mt_u8 on_off);
mt_s32 mt_unf_fe_set_22k_onoff(mt_u32 tuner_id, mt_u8 on_off);
mt_s32 mt_unf_fe_set_polarization(mt_u32 tuner_id, mt_unf_fe_lnb_polar polar);
mt_s32 mt_unf_fe_diseqc_ctrl(mt_u32 tuner_id, void *para);


mt_s32 mt_unf_fe_get_tuner_dbg_info(mt_u32 tuner_id, mt_u32 data);
mt_s32 mt_unf_fe_set_s2_set_gse_attr(mt_u32 tuner_id, mt_u32 gse_param);

mt_s32 mt_unf_fe_switch_super_search_mode(mt_u32 tuner_id, mt_u32 super_search_mode);

mt_s32 mt_unf_fe_unicable_retry(mt_u32 tuner_id);

mt_s32 mt_unf_fe_suspend(mt_u32 tuner_id);
mt_s32 mt_unf_fe_resume(mt_u32 tuner_id);

mt_s32 mt_unf_fe_suspend_all(void);
mt_s32 mt_unf_fe_resume_all(void);


mt_s32 mt_unf_fe_diseqc2_srmsg(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch4port_t *p_prm,mt_unf_fe_diseqc2_rsmsg_info *msg);

void mt_unf_fe_dump_connect_param(mt_unf_fe_connect_para_t *p_conn_param);


mt_s32 mt_unf_fe_dss_filter_scid(mt_unf_fe_dss_scid_filter_t  *scid_filter);
mt_s32 mt_unf_fe_get_s2_bbheader_ts_gse_mode(mt_u32 tuner_id, mt_u8 *p_ts_gse_mode);
mt_s32 mt_unf_fe_set_gs_package_mode(mt_u32 tuner_id, mt_u8 *p_gs_package_mode);
mt_s32 mt_unf_fe_set_lowpower(mt_u32 tuner_id,mt_unf_fe_sig_type_t sig_type);
mt_s32 mt_unf_fe_set_bbframe_padding_onoff(mt_u32 tuner_id,mt_unf_fe_bbframe_packing_type_t bbframe_packing_type);
/** @} */ /** <!-- ==== API declaration end ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_UNF_FRONTEND_H__ */
