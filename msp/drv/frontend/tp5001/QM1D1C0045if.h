/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#ifdef _USE_TP5001_CHIP_
/******************************************************
QM1D1C0045if.h						
----------------------------------------------------
Rf IC control functions				

<Revision History>
'11/02/09 : OKAMOTO [QM1D1C0045] First release
----------------------------------------------------
Copyright(C) 2010 SHARP CORPORATION			
******************************************************/
#ifndef	SHARP_DEBUG_4R5GEN_FOR_CYGWIN
//#define	SHARP_DEBUG_4R5GEN_FOR_CYGWIN
#endif

#ifdef	SHARP_DEBUG_4R5GEN_FOR_CYGWIN
#define _STR(x)      #x
#define _STR2(x)     _STR(x)
#define __SLINE__    _STR2(__LINE__)
#define HERE         __FILE__ "(" __SLINE__ ")  "
#endif

typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;

typedef signed char         INT8;
typedef short               INT16;
typedef int                 INT32;
typedef long long           INT64;

#ifndef _QM1D1C0045IF_H
#define _QM1D1C0045IF_H

#include "mt_type.h"

#include "commdef.h"

typedef enum _QM1D1C0045_INIT_CFG_DATA{
	QM1D1C0045_LOCAL_FREQ,
	QM1D1C0045_XTAL_FREQ,
	QM1D1C0045_CHIP_ID,
	QM1D1C0045_LPF_WAIT_TIME,
	QM1D1C0045_FAST_SEARCH_WAIT_TIME,
	QM1D1C0045_NORMAL_SEARCH_WAIT_TIME,
	QM1D1C0045_INI_CGF_MAX
}QM1D1C0045_INIT_CFG_DATA, *PQM1D1C0045_INIT_CFG_DATA ;

typedef enum _QM1D1C0045_INIT_REG_DATA{
	QM1D1C0045_REG_00 = 0,
	QM1D1C0045_REG_01,
	QM1D1C0045_REG_02,
	QM1D1C0045_REG_03,
	QM1D1C0045_REG_04,
	QM1D1C0045_REG_05,
	QM1D1C0045_REG_06,
	QM1D1C0045_REG_07,
	QM1D1C0045_REG_08,
	QM1D1C0045_REG_09,
	QM1D1C0045_REG_0A,
	QM1D1C0045_REG_0B,
	QM1D1C0045_REG_0C,
	QM1D1C0045_REG_0D,
	QM1D1C0045_REG_0E,
	QM1D1C0045_REG_0F,
	QM1D1C0045_REG_10,
	QM1D1C0045_REG_11,
	QM1D1C0045_REG_12,
	QM1D1C0045_REG_13,
	QM1D1C0045_REG_14,
	QM1D1C0045_REG_15,
	QM1D1C0045_REG_16,
	QM1D1C0045_REG_17,
	QM1D1C0045_REG_18,
	QM1D1C0045_REG_19,
	QM1D1C0045_REG_1A,
	QM1D1C0045_REG_1B,
	QM1D1C0045_REG_1C,
	QM1D1C0045_REG_1D,
	QM1D1C0045_REG_1E,
	QM1D1C0045_REG_1F,
	QM1D1C0045_INI_REG_MAX
}QM1D1C0045_INIT_REG_DATA, *PQM1D1C0045_INIT_REG_DATA ;

typedef enum _QM1D1C0045_LPF_FC{
	QM1D1C0045_LPF_FC_4MHz=0,	//0000:4MHz
	QM1D1C0045_LPF_FC_6MHz, 	//0001:6MHz
	QM1D1C0045_LPF_FC_8MHz, 	//0010:8MHz
	QM1D1C0045_LPF_FC_10MHz,	//0011:10MHz
	QM1D1C0045_LPF_FC_12MHz,	//0100:12MHz
	QM1D1C0045_LPF_FC_14MHz,	//0101:14MHz
	QM1D1C0045_LPF_FC_16MHz,	//0110:16MHz
	QM1D1C0045_LPF_FC_18MHz,	//0111:18MHz
	QM1D1C0045_LPF_FC_20MHz,	//1000:20MHz
	QM1D1C0045_LPF_FC_22MHz,	//1001:22MHz
	QM1D1C0045_LPF_FC_24MHz,	//1010:24MHz
	QM1D1C0045_LPF_FC_26MHz,	//1011:26MHz
	QM1D1C0045_LPF_FC_28MHz,	//1100:28MHz
	QM1D1C0045_LPF_FC_30MHz,	//1101:30MHz
	QM1D1C0045_LPF_FC_32MHz,	//1110:32MHz
	QM1D1C0045_LPF_FC_34MHz,	//1111:34MHz
	QM1D1C0045_LPF_FC_MAX,
}QM1D1C0045_LPF_FC;

typedef enum _QM1D1C0045_LPF_ADJUSTMENT_CURRENT{
	QM1D1C0045_LPF_ADJUSTMENT_CURRENT_25UA=0,	//0x1B b[1:0] = b00
	QM1D1C0045_LPF_ADJUSTMENT_CURRENT_DUMMY1,	//0x1B b[1:0] = b01
	QM1D1C0045_LPF_ADJUSTMENT_CURRENT_37R5UA,	//0x1B b[1:0] = b10
	QM1D1C0045_LPF_ADJUSTMENT_CURRENT_DUMMY2,	//0x1B b[1:0] = b11
}QM1D1C0045_LPF_ADJUSTMENT_CURRENT;

typedef struct _QM1D1C0045_CONFIG_STRUCT {
	unsigned int		ui_QM1D1C0045_RFChannelkHz;	/* direct channel */
	unsigned int		ui_QM1D1C0045_XtalFreqKHz;
	MT_BOOL				b_QM1D1C0045_fast_search_mode;
	MT_BOOL				b_QM1D1C0045_loop_through;
	MT_BOOL				b_QM1D1C0045_tuner_standby;
	MT_BOOL				b_QM1D1C0045_head_amp;
	QM1D1C0045_LPF_FC	QM1D1C0045_lpf;
	unsigned int		ui_QM1D1C0045_LpfWaitTime;
	unsigned int		ui_QM1D1C0045_FastSearchWaitTime;
	unsigned int		ui_QM1D1C0045_NormalSearchWaitTime;
} QM1D1C0045_CONFIG_STRUCT, *PQM1D1C0045_CONFIG_STRUCT;


//=========================================================================
// GLOBAL VARIALBLES
//=========================================================================
typedef MT_BOOL (*QM1D1C0045_I2C_RD_HANDLER)(UINT8, UINT8 *, UINT16 *);
typedef MT_BOOL (*QM1D1C0045_I2C_WR_HANDLER)(UINT8, UINT8 *, UINT16 *);

#ifdef __cplusplus
extern "C" {
#endif
//=========================================================================
// FUNCTIONS
//=========================================================================
extern MT_BOOL QM1D1C0045_Initialize(PQM1D1C0045_CONFIG_STRUCT	apConfig) ;
extern MT_BOOL QM1D1C0045_LocalLpfCutOffSetting(PQM1D1C0045_CONFIG_STRUCT	apConfig) ;
extern MT_BOOL QM1D1C0045_LocalLpfTuning(PQM1D1C0045_CONFIG_STRUCT	apConfig);
extern MT_BOOL QM1D1C0045_register_real_read(UINT8 RegAddr, UINT8 *apData);
extern MT_BOOL QM1D1C0045_register_real_write(UINT8 RegAddr, UINT8 RegData);
extern MT_BOOL QM1D1C0045_set_i2c_handler(QM1D1C0045_I2C_WR_HANDLER apWriteHandler, QM1D1C0045_I2C_RD_HANDLER apReadHandler);
extern UINT8 QM1D1C0045_i2c_slave_addr_set(UINT8 SlaveAddr);
extern MT_BOOL QM1D1C0045_Set_Operation_Param(PQM1D1C0045_CONFIG_STRUCT	apConfig);
extern void QM1D1C0045_get_lock_status(MT_BOOL* pbLock);

extern UINT8 QM1D1C0045_d_reg[QM1D1C0045_INI_REG_MAX];

extern int QM1D1C0045_init(UINT32 channel_freq,unsigned int rate);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _QM1D1C0045IF_H */
#endif // _USE_TP5001_CHIP_
