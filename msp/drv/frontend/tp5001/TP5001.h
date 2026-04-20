/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#ifndef __TP5001_H__
#define __TP5001_H__

#include <linux/i2c.h>

#include "TP_TYPE.h"
#include "RDA5812.h"
#include "RDA5815.h"
#include "RDA5815M.h"
#include "av2020.h"
#include "sharp6306.h"
#include "sharp6903.h"
#include "S305.h"
#include "IIC.h"
#ifdef _USE_TP5001_CHIP_

// ==========================================================================================================
// Define  & Marco
// 
// ---------------------------------------------------------------------------------------------------------
// Error Code:
// 

#define TP5001_DEVADDR		0x18//0x0c

#define TP_SUCCESS										0
#define TP_IIC_WRERR									1
#define TP_IIC_RDERR									2
#define TP_IIC_WR_TOO_LONG								3
#define TP_IIC_RD_TOO_LONG								4
#define TP_SET_TUNER_ERR								5
#define TP_NOT_SUPPORT									6
#define TP_TUNER_IIC_WR_TOO_LONG						7
#define TP_TUNER_IIC_TIME_OUT							8
#define TP_NOT_LOCK										9
#define TP_PARA_ERR										10
#define TP_TUNER_ID_ERR									11
#define TP_ERROR_DIV0									12
#define TP_IIC_IDLE_ERR                                 13
#define TP_CHIP_ID_ERROR                                14

// ---------------------------------------------------------------------------------------------------------
typedef enum tag_TP_RFAGCPola 
{
	RA_Normal,
	RA_Invert
}TP_RFAGCPola;

typedef enum tag_TP_IQPola 
{
	Normal,
	Invert
}TP_IQPola;

typedef enum tag_TP_SPI_edge
{
	positive_edge,		// 上升沿有效
	negitive_edge		// 下降沿有效
}TP_SPI_edge;

typedef enum tag_TP_SPI_port_type
{
	series_port,		// 串口
	para_port,			// 并口
	out_disable         // 高阻
}TP_SPI_port_type;

// ---------------------------------------------------------------------------------------------------------
// Parameter

#ifdef   __cplusplus
    extern   "C" 
    {
#endif

// ---------------------------------------------------------------------------------------------------------
// ==========================================================================================================


// ==========================================================================================================
// External Function
// 

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_init
// Description: TP芯片，初始化
// Return:
//		TP_SUCCESS: 操作成功
//
extern TP_UINT8 TP_init(void);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_deinit
// Description: TP芯片，反初始化
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_deinit(void);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_version
// Description: TP芯片，得到版本号
// Output:
//		pVersion:  
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_get_version(TP_INT8 * pVersion);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_set_rf_tuner
// Description: TP芯片，设置Tuner
// Input:
//		frequency:  频率
// Return:
//		TP_SUCCESS: 操作成功
//
extern TP_UINT8 TP_set_rf_tuner(TP_UINT32 frequency,TP_UINT32 symbol_rate);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_set_rf_pola
// Description: TP芯片，设置极性
// Input:
//		Pola: 极性，Normal: 正常(VH_SEL管脚为低)，Invert: 反向(VH_SEL管脚为高)
// Return:
//		TP_SUCCESS: 锁定
//
TP_UINT8 TP_set_rf_pola(TP_RFAGCPola Pola); 
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_set_iq_switch
// Description: TP芯片，设置IQ反向
// Input:
//		iq_pola: IQ的极性   Normal:正向   Invert: 反相
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_set_iq_switch(TP_IQPola iq_pola);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_set_symbol_rate
// Description: TP芯片，设置符号率
// Input:
//		symbol_rate:  符号率
// Return:
//		TP_SUCCESS: 操作成功 
//
extern TP_UINT8 TP_set_symbol_rate(TP_UINT32 symbol_rate);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_ts_interface
// Description: TP芯片，设置AGC参数
// Input:
//		active_edge: positive_edge, 上升沿     negitive_edge, 下降沿
//		port_type: series_port, 串口      para_port, 并口
//		clock_div: SPI时钟分频因子,    SPI_CLK = 380 / (clock_div * 4)
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_set_ts_interface(TP_SPI_edge active_edge, TP_SPI_port_type port_type, TP_UINT8 clock_div);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_signal_quality
// Description: TP芯片，得到信号质量
// Output:
//		p_quality_percent:  质量 0: 0%  --   100: 100%
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_get_signal_quality(TP_UINT8 * p_quality_percent);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_signal_quality BD
// Description: TP芯片，得到信号质量
// Output:
//		p_quality_DB:  质量 DB
// Return:
//		TP_SUCCESS: 操作成功
//
//TP_UINT8 TP_get_signal_quality_DB(TP_FLOAT * p_quality_DB);
TP_UINT8 TP_get_signal_quality_DB(TP_INT32 * p_quality_DB);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_signal_strength
// Description: TP芯片，得到信号强度
// Output:
//		p_signal_strength:  信号强度
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_get_signal_strength(TP_UINT8 * p_signal_strength);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_lock_status
// Description: TP芯片，得到锁定状态
// Return:
//		TP_SUCCESS: 锁定
//		TP_NOT_LOCK: 没有锁定
//
TP_UINT8 TP_get_lock_status(void);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_freq_offset
// Description: TP芯片，得到频偏值
// Output:
//		p_freq_offset: 频率偏差值，单位(KHz)
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_get_freq_offset(TP_INT32 * p_freq_offset);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_set_sleep
// Description: TP芯片进入睡眠模式
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_set_sleep(void);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_soft_reset
// Description: TP芯片软复位
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_soft_reset(void);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_SendDiseqcCommand
// Description: TP发送Diseqc命令
// Paramenter:
//			length: input, 发送Diseqc命令的长度
//          pCommands: input, 发送Diseqc命令的数据
//          pReturnLength: Input/Output 接收数据的长度
//          pReturnBuffer: 接收数据的buffer
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_SendDiseqcCommand(TP_UINT8  length, 
                                 TP_UINT8* pCommands, 
                                 TP_UINT8* pReturnLength, 
                                 TP_UINT8* pReturnBuffer);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_statistic_ber_bler
// Description: TP5001芯片，得到BER和BLER统计值
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_get_statistic_ber_bler(TP_UINT64 *pBer_total, TP_UINT64 *pBer_error, TP_UINT64 *pBler_total, TP_UINT64 *pBler_error);
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_clear_statistic_ber_bler
// Description: TP5001芯片，清除BER和BLER统计值
// Return:
//		TP_SUCCESS: 操作成功
//
TP_UINT8 TP_clear_statistic_ber_bler(TP_UINT8 type);
// ---------------------------------------------------------------------------------------------------------

TP_UINT8 TP_iic_tuner_write(TP_UINT8 dev_addr, TP_UINT8 * value, TP_UINT32 length);
TP_UINT8 TP_iic_tuner_read(TP_UINT8 dev_addr, TP_UINT8 reg_addr, TP_UINT8 * value, TP_UINT32 length);
                                 

TP_UINT8 TP_iic_write(TP_UINT8 device_address, TP_UINT16 register_address, TP_UINT8 * value_buffer, TP_UINT32 length);

TP_UINT8 TP_iic_read(TP_UINT8 device_address, TP_UINT16 register_address, TP_UINT8 * value_buffer, TP_UINT32 length);

void TP_Delay( TP_UINT32 uiMS );

void TP_Reset(void);

#ifdef   __cplusplus
    }
#endif
#endif
#endif //_USE_TP5001_CHIP_

