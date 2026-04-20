/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage-LZ Group                                                          */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2021 Montage-LZ Group Limited. All Rights Reserved.         */
/*****************************************************************************/

#ifndef __MT_FE_SONY_TN_RAFAEL_COMMON_H__
#define __MT_FE_SONY_TN_RAFAEL_COMMON_H__

#define RAFAEL_COMMON_HEADER 1

#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif


typedef enum _Rafael_ErrCode
{
	RT_Success = TRUE,
	RT_Fail = FALSE
}Rafael_ErrCode;


enum XTAL_PWR_VALUE
{
	XTAL_LOWEST = 0,
    XTAL_LOW,
    XTAL_HIGH,
    XTAL_HIGHEST,
	XTAL_CHECK_SIZE
};

typedef enum _Rafael_Xtal_Div_TYPE
{
	XTAL_DIV1 = 0,
	XTAL_DIV1_2,	//1st_div2=0(R34[0]), 2nd_div2=1(R34[1])  ; same AGC clock
	XTAL_DIV2_1,	//1st_div2=1(R34[0]), 2nd_div2=0(R34[1])  ; diff AGC clock
	XTAL_DIV4
}Rafael_Xtal_Div_TYPE;

typedef enum _Rafael_RF_Gain_TYPE
{
	RF_AUTO = 0,
	RF_MANUAL
}Rafael_RF_Gain_TYPE;

typedef enum _Rafael_ClkOutMode_Type
{
	CLK_OUT_OFF = 0,
	CLK_OUT_ON
}Rafael_ClkOutMode_Type;

typedef enum _Rafael_LoopThrough_Type
{
	LT_ON = TRUE,
	LT_OFF = FALSE
}Rafael_LoopThrough_Type;

#endif


