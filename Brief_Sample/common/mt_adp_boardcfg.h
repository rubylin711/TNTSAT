#ifndef __SAMPLE_MT_BOARDCFG_H__
#define __SAMPLE_MT_BOARDCFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TUNER_USE 0 //MT_TUNER0_ID

#define MTADP_FE_GET_CONFIG(tuner_id, stTunerAttr)                            \
    \
{                                                                      \
	if (0 == tuner_id) {                                                  \
	    stTunerAttr.sig_type = MT_UNF_FE_SIG_TYPE_CAB;                    \
	    stTunerAttr.tuner_type = MT_UNF_TUNER_TYPE_M88TC3800;             \
	    stTunerAttr.tuner_addr = 0xc2;                                    \
	    stTunerAttr.demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB; \
	    stTunerAttr.demod_addr = 0x38;                                    \
	    stTunerAttr.output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL;         \
	    stTunerAttr.i2c_id = 0;                                           \
	    stTunerAttr.no_need_init = 0;                                     \
	} else if (1 == tuner_id) {                                           \
	    stTunerAttr.sig_type = MT_UNF_FE_SIG_TYPE_CAB;                    \
	    stTunerAttr.tuner_type = MT_UNF_TUNER_TYPE_M88TC6800;             \
	    stTunerAttr.tuner_addr = 0xc6;                                    \
	    stTunerAttr.demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CC6000;     \
	    stTunerAttr.demod_addr = 0xd0;                                    \
	    stTunerAttr.output_mode = MT_UNF_FE_OUTPUT_MODE_SERIAL;           \
	    stTunerAttr.i2c_id = 0;                                           \
	} else if (2 == tuner_id) {                                           \
	    stTunerAttr.sig_type = MT_UNF_FE_SIG_TYPE_CAB;                    \
	    stTunerAttr.tuner_type = MT_UNF_TUNER_TYPE_M88TC6800;             \
	    stTunerAttr.tuner_addr = 0xc6;                                    \
	    stTunerAttr.demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CC6000;     \
	    stTunerAttr.demod_addr = 0xd0;                                    \
	    stTunerAttr.output_mode = MT_UNF_FE_OUTPUT_MODE_SERIAL;           \
	    stTunerAttr.i2c_id = 0;                                           \
	} else if (3 == tuner_id) {                                           \
	    stTunerAttr.sig_type = MT_UNF_FE_SIG_TYPE_CAB;                    \
	    stTunerAttr.tuner_type = MT_UNF_TUNER_TYPE_M88TC6800;             \
	    stTunerAttr.tuner_addr = 0xc6;                                    \
	    stTunerAttr.demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CC6000;     \
	    stTunerAttr.demod_addr = 0xd0;                                    \
	    stTunerAttr.output_mode = MT_UNF_FE_OUTPUT_MODE_SERIAL;           \
	    stTunerAttr.i2c_id = 0;                                           \
	}                                                                     \
    \
}

#if 0 //(MT_FE_SIGNAL_TYPE == 2)
#define GET_SAT_TUNER_CONFIG(tuner_id, stSatTunerAttr) \
    \
{                                               \
	if (0 == tuner_id) {                           \
	    stSatTunerAttr.dmd_clk = 0;                \
	    stSatTunerAttr.tuner_max_lpf = 0;          \
	    stSatTunerAttr.tuner_i2c_clk = 0;          \
	    stSatTunerAttr.en_rfagc = 0;               \
	    stSatTunerAttr.iq_spectrum = 0;            \
	    stSatTunerAttr.ts_clk_polar = 0;           \
	    stSatTunerAttr.ts_format = 0;              \
	    stSatTunerAttr.ts_serial_pin = 0;          \
	    stSatTunerAttr.diseqc_wave = 0;            \
	    stSatTunerAttr.lnbctrl_dev = 0;            \
	    stSatTunerAttr.lnb_dev_addr = 0;           \
	}                                              \
    \
}
#endif

#if 0 //(MT_FE_SIGNAL_TYPE == 2)
#define GET_SAT_TUNER1_CONFIG(u32TunerId, stSatTunerAttr)           \
    \
{                                                            \
	if (1 == u32TunerId) {                                      \
	    stSatTunerAttr.u32DemodClk = MT_DEMOD1_REF_CLOCK;       \
	    stSatTunerAttr.u16TunerMaxLPF = MT_TUNER1_MAX_LPF;      \
	    stSatTunerAttr.u16TunerI2CClk = MT_TUNER1_I2C_CLOCK;    \
	    stSatTunerAttr.enRFAGC = MT_TUNER1_RFAGC;               \
	    stSatTunerAttr.enIQSpectrum = MT_TUNER1_IQSPECTRUM;     \
	    stSatTunerAttr.enTSClkPolar = MT_TUNER1_TSCLK_POLAR;    \
	    stSatTunerAttr.enTSFormat = MT_TUNER1_TS_FORMAT;        \
	    stSatTunerAttr.enTSSerialPIN = MT_TUNER1_TS_SERIAL_PIN; \
	    stSatTunerAttr.enDiSEqCWave = MT_TUNER1_DISEQCWAVE;     \
	    stSatTunerAttr.enLNBCtrlDev = MT_LNBCTRL1_DEV_TYPE;     \
	    stSatTunerAttr.u16LNBDevAddress = MT_LNBCTRL1_DEV_ADDR; \
	}                                                           \
    \
}
#endif

#if 0 //(MT_FE_SIGNAL_TYPE == 2)
#define GET_SAT_TUNER2_CONFIG(u32TunerId, stSatTunerAttr)           \
    \
{                                                            \
	if (2 == u32TunerId) {                                      \
	    stSatTunerAttr.u32DemodClk = MT_DEMOD2_REF_CLOCK;       \
	    stSatTunerAttr.u16TunerMaxLPF = MT_TUNER2_MAX_LPF;      \
	    stSatTunerAttr.u16TunerI2CClk = MT_TUNER2_I2C_CLOCK;    \
	    stSatTunerAttr.enRFAGC = MT_TUNER2_RFAGC;               \
	    stSatTunerAttr.enIQSpectrum = MT_TUNER2_IQSPECTRUM;     \
	    stSatTunerAttr.enTSClkPolar = MT_TUNER2_TSCLK_POLAR;    \
	    stSatTunerAttr.enTSFormat = MT_TUNER2_TS_FORMAT;        \
	    stSatTunerAttr.enTSSerialPIN = MT_TUNER2_TS_SERIAL_PIN; \
	    stSatTunerAttr.enDiSEqCWave = MT_TUNER2_DISEQCWAVE;     \
	    stSatTunerAttr.enLNBCtrlDev = MT_LNBCTRL2_DEV_TYPE;     \
	    stSatTunerAttr.u16LNBDevAddress = MT_LNBCTRL2_DEV_ADDR; \
	}                                                           \
    \
}
#endif

#if 0 //(MT_FE_SIGNAL_TYPE == 2)
#define GET_SAT_TUNER3_CONFIG(u32TunerId, stSatTunerAttr)           \
    \
{                                                            \
	if (3 == u32TunerId) {                                      \
	    stSatTunerAttr.u32DemodClk = MT_DEMOD3_REF_CLOCK;       \
	    stSatTunerAttr.u16TunerMaxLPF = MT_TUNER3_MAX_LPF;      \
	    stSatTunerAttr.u16TunerI2CClk = MT_TUNER3_I2C_CLOCK;    \
	    stSatTunerAttr.enRFAGC = MT_TUNER3_RFAGC;               \
	    stSatTunerAttr.enIQSpectrum = MT_TUNER3_IQSPECTRUM;     \
	    stSatTunerAttr.enTSClkPolar = MT_TUNER3_TSCLK_POLAR;    \
	    stSatTunerAttr.enTSFormat = MT_TUNER3_TS_FORMAT;        \
	    stSatTunerAttr.enTSSerialPIN = MT_TUNER3_TS_SERIAL_PIN; \
	    stSatTunerAttr.enDiSEqCWave = MT_TUNER3_DISEQCWAVE;     \
	    stSatTunerAttr.enLNBCtrlDev = MT_LNBCTRL3_DEV_TYPE;     \
	    stSatTunerAttr.u16LNBDevAddress = MT_LNBCTRL3_DEV_ADDR; \
	}                                                           \
    \
}
#endif

#if 0 //(MT_FE_SIGNAL_TYPE == 4) || (MT_TUNER_SIGNAL_TYPE == 8)
#define GET_TER_TUNER_CONFIG(u32TunerId, stTerTunerAttr)               \
    \
{                                                               \
	if (0 == u32TunerId) {                                         \
	    stTerTunerAttr.u32DemodClk = MT_TER_DEMOD_REF_CLOCK;       \
	    stTerTunerAttr.reset_gpio_no = MT_DEMOD_RESET_GPIO;        \
	    stTerTunerAttr.u16TunerMaxLPF = MT_TER_TUNER_MAX_LPF;      \
	    stTerTunerAttr.u16TunerI2CClk = MT_TER_TUNER_I2C_CLOCK;    \
	    stTerTunerAttr.enRFAGC = MT_TER_TUNER_RFAGC;               \
	    stTerTunerAttr.enIQSpectrum = MT_TER_TUNER_IQSPECTRUM;     \
	    stTerTunerAttr.enTSClkPolar = MT_TER_TUNER_TSCLK_POLAR;    \
	    stTerTunerAttr.enTSFormat = MT_TER_TUNER_TS_FORMAT;        \
	    stTerTunerAttr.enTSSerialPIN = MT_TER_TUNER_TS_SERIAL_PIN; \
	    stTerTunerAttr.enTSSyncHead = MT_TER_TUNER_TS_SYNC_HEAD;   \
	}                                                              \
    \
}
#endif

#if 0 //(MT_TUNER1_SIGNAL_TYPE == 4) || (MT_TUNER1_SIGNAL_TYPE == 8)
#define GET_TER_TUNER1_CONFIG(u32TunerId, stTerTunerAttr)               \
    \
{                                                                \
	if (1 == u32TunerId) {                                          \
	    stTerTunerAttr.u32DemodClk = MT_TER_DEMOD1_REF_CLOCK;       \
	    stTerTunerAttr.reset_gpio_no = MT_DEMOD1_RESET_GPIO;        \
	    stTerTunerAttr.u16TunerMaxLPF = MT_TER_TUNER1_MAX_LPF;      \
	    stTerTunerAttr.u16TunerI2CClk = MT_TER_TUNER1_I2C_CLOCK;    \
	    stTerTunerAttr.enRFAGC = MT_TER_TUNER1_RFAGC;               \
	    stTerTunerAttr.enIQSpectrum = MT_TER_TUNER1_IQSPECTRUM;     \
	    stTerTunerAttr.enTSClkPolar = MT_TER_TUNER1_TSCLK_POLAR;    \
	    stTerTunerAttr.enTSFormat = MT_TER_TUNER1_TS_FORMAT;        \
	    stTerTunerAttr.enTSSerialPIN = MT_TER_TUNER1_TS_SERIAL_PIN; \
	    stTerTunerAttr.enTSSyncHead = MT_TER_TUNER1_TS_SYNC_HEAD;   \
	}                                                               \
    \
}
#endif

#if 0 //(MT_TUNER2_SIGNAL_TYPE == 4) || (MT_TUNER2_SIGNAL_TYPE == 8)
#define GET_TER_TUNER2_CONFIG(u32TunerId, stTerTunerAttr)               \
    \
{                                                                \
	if (2 == u32TunerId) {                                          \
	    stTerTunerAttr.u32DemodClk = MT_TER_DEMOD2_REF_CLOCK;       \
	    stTerTunerAttr.reset_gpio_no = MT_DEMOD2_RESET_GPIO;        \
	    stTerTunerAttr.u16TunerMaxLPF = MT_TER_TUNER2_MAX_LPF;      \
	    stTerTunerAttr.u16TunerI2CClk = MT_TER_TUNER2_I2C_CLOCK;    \
	    stTerTunerAttr.enRFAGC = MT_TER_TUNER2_RFAGC;               \
	    stTerTunerAttr.enIQSpectrum = MT_TER_TUNER2_IQSPECTRUM;     \
	    stTerTunerAttr.enTSClkPolar = MT_TER_TUNER2_TSCLK_POLAR;    \
	    stTerTunerAttr.enTSFormat = MT_TER_TUNER2_TS_FORMAT;        \
	    stTerTunerAttr.enTSSerialPIN = MT_TER_TUNER2_TS_SERIAL_PIN; \
	    stTerTunerAttr.enTSSyncHead = MT_TER_TUNER2_TS_SYNC_HEAD;   \
	}                                                               \
    \
}
#endif

#if 0 //(MT_TUNER3_SIGNAL_TYPE == 4) || (MT_TUNER3_SIGNAL_TYPE == 8)
#define GET_TER_TUNER3_CONFIG(u32TunerId, stTerTunerAttr)               \
    \
{                                                                \
	if (3 == u32TunerId) {                                          \
	    stTerTunerAttr.u32DemodClk = MT_TER_DEMOD3_REF_CLOCK;       \
	    stTerTunerAttr.reset_gpio_no = MT_DEMOD3_RESET_GPIO;        \
	    stTerTunerAttr.u16TunerMaxLPF = MT_TER_TUNER3_MAX_LPF;      \
	    stTerTunerAttr.u16TunerI2CClk = MT_TER_TUNER3_I2C_CLOCK;    \
	    stTerTunerAttr.enRFAGC = MT_TER_TUNER3_RFAGC;               \
	    stTerTunerAttr.enIQSpectrum = MT_TER_TUNER3_IQSPECTRUM;     \
	    stTerTunerAttr.enTSClkPolar = MT_TER_TUNER3_TSCLK_POLAR;    \
	    stTerTunerAttr.enTSFormat = MT_TER_TUNER3_TS_FORMAT;        \
	    stTerTunerAttr.enTSSerialPIN = MT_TER_TUNER3_TS_SERIAL_PIN; \
	    stTerTunerAttr.enTSSyncHead = MT_TER_TUNER3_TS_SYNC_HEAD;   \
	}                                                               \
    \
}
#endif

/********
例如TS_OUT0管脚输出data7信号，则stTSOut.enTSOutput[0] = MT_UNF_TUNER_OUTPUT_TSDAT7
********/
#ifdef MT_TUNER_OUTPUT_PIN0
#define GET_TUNER0_TSOUT_CONFIG(stTSOut)                \
    \
{                                                \
	stTSOut.enTSOutput[0] = MT_TUNER_OUTPUT_PIN0;   \
	stTSOut.enTSOutput[1] = MT_TUNER_OUTPUT_PIN1;   \
	stTSOut.enTSOutput[2] = MT_TUNER_OUTPUT_PIN2;   \
	stTSOut.enTSOutput[3] = MT_TUNER_OUTPUT_PIN3;   \
	stTSOut.enTSOutput[4] = MT_TUNER_OUTPUT_PIN4;   \
	stTSOut.enTSOutput[5] = MT_TUNER_OUTPUT_PIN5;   \
	stTSOut.enTSOutput[6] = MT_TUNER_OUTPUT_PIN6;   \
	stTSOut.enTSOutput[7] = MT_TUNER_OUTPUT_PIN7;   \
	stTSOut.enTSOutput[8] = MT_TUNER_OUTPUT_PIN8;   \
	stTSOut.enTSOutput[9] = MT_TUNER_OUTPUT_PIN9;   \
	stTSOut.enTSOutput[10] = MT_TUNER_OUTPUT_PIN10; \
    \
}
#endif

#ifdef MT_TUNER1_OUTPUT_PIN0
#define GET_TUNER1_TSOUT_CONFIG(stTSOut)                 \
    \
{                                                 \
	stTSOut.enTSOutput[0] = MT_TUNER1_OUTPUT_PIN0;   \
	stTSOut.enTSOutput[1] = MT_TUNER1_OUTPUT_PIN1;   \
	stTSOut.enTSOutput[2] = MT_TUNER1_OUTPUT_PIN2;   \
	stTSOut.enTSOutput[3] = MT_TUNER1_OUTPUT_PIN3;   \
	stTSOut.enTSOutput[4] = MT_TUNER1_OUTPUT_PIN4;   \
	stTSOut.enTSOutput[5] = MT_TUNER1_OUTPUT_PIN5;   \
	stTSOut.enTSOutput[6] = MT_TUNER1_OUTPUT_PIN6;   \
	stTSOut.enTSOutput[7] = MT_TUNER1_OUTPUT_PIN7;   \
	stTSOut.enTSOutput[8] = MT_TUNER1_OUTPUT_PIN8;   \
	stTSOut.enTSOutput[9] = MT_TUNER1_OUTPUT_PIN9;   \
	stTSOut.enTSOutput[10] = MT_TUNER1_OUTPUT_PIN10; \
    \
}
#endif

#ifdef MT_TUNER2_OUTPUT_PIN0
#define GET_TUNER2_TSOUT_CONFIG(stTSOut)                 \
    \
{                                                 \
	stTSOut.enTSOutput[0] = MT_TUNER2_OUTPUT_PIN0;   \
	stTSOut.enTSOutput[1] = MT_TUNER2_OUTPUT_PIN1;   \
	stTSOut.enTSOutput[2] = MT_TUNER2_OUTPUT_PIN2;   \
	stTSOut.enTSOutput[3] = MT_TUNER2_OUTPUT_PIN3;   \
	stTSOut.enTSOutput[4] = MT_TUNER2_OUTPUT_PIN4;   \
	stTSOut.enTSOutput[5] = MT_TUNER2_OUTPUT_PIN5;   \
	stTSOut.enTSOutput[6] = MT_TUNER2_OUTPUT_PIN6;   \
	stTSOut.enTSOutput[7] = MT_TUNER2_OUTPUT_PIN7;   \
	stTSOut.enTSOutput[8] = MT_TUNER2_OUTPUT_PIN8;   \
	stTSOut.enTSOutput[9] = MT_TUNER2_OUTPUT_PIN9;   \
	stTSOut.enTSOutput[10] = MT_TUNER2_OUTPUT_PIN10; \
    \
}
#endif
#ifdef MT_TUNER3_OUTPUT_PIN0
#define GET_TUNER3_TSOUT_CONFIG(stTSOut)                 \
    \
{                                                 \
	stTSOut.enTSOutput[0] = MT_TUNER3_OUTPUT_PIN0;   \
	stTSOut.enTSOutput[1] = MT_TUNER3_OUTPUT_PIN1;   \
	stTSOut.enTSOutput[2] = MT_TUNER3_OUTPUT_PIN2;   \
	stTSOut.enTSOutput[3] = MT_TUNER3_OUTPUT_PIN3;   \
	stTSOut.enTSOutput[4] = MT_TUNER3_OUTPUT_PIN4;   \
	stTSOut.enTSOutput[5] = MT_TUNER3_OUTPUT_PIN5;   \
	stTSOut.enTSOutput[6] = MT_TUNER3_OUTPUT_PIN6;   \
	stTSOut.enTSOutput[7] = MT_TUNER3_OUTPUT_PIN7;   \
	stTSOut.enTSOutput[8] = MT_TUNER3_OUTPUT_PIN8;   \
	stTSOut.enTSOutput[9] = MT_TUNER3_OUTPUT_PIN9;   \
	stTSOut.enTSOutput[10] = MT_TUNER3_OUTPUT_PIN10; \
    \
}
#endif

//#endif
#define MT_DEMUX_PORT 0
#define DEFAULT_DVB_PORT (MT_DEMUX_PORT)

#define MT_DAC_CVBS 0 //Rock_hu 自己增加定义以后删除
#define MT_DAC_YPBPR_Y 1
#define MT_DAC_YPBPR_PB 2
#define MT_DAC_YPBPR_PR 3

/* DAC */
#define DAC_CVBS MT_DAC_CVBS
#define DAC_YPBPR_Y MT_DAC_YPBPR_Y
#define DAC_YPBPR_PB MT_DAC_YPBPR_PB
#define DAC_YPBPR_PR MT_DAC_YPBPR_PR

#ifdef __cplusplus
}
#endif
#endif
