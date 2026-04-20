/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#ifndef RDA5812_h_h
#define RDA5812_h_h

#ifdef __cplusplus
extern "C"
{
#endif

/* Function to Initialize the RDA5812 */
/***************************************************************************/
#define RDA5812_I2C_ADDRESS 0x18

void RDA5812Initial(void);
/*************************************************************************/
/*	Function to Set the RDA5812 */

/*	fPLL:   Frequency        			unit: MHz  from 950 to 2150 */
/*	fSym:   SymbolRate       			unit: KS/s from 1000 to 45000 */
/*  gainHold:  The flag of AGC gain hold, the tuner gain is hold when gainHold == 1 , default please set gainHold = 0   */
/*  return: Frequency offset of PLL  	unit: KHz */
/************************************************************************/
INT32 RDA5812Set(UINT32 fPLL, UINT32 fSym, UINT8 gainHold);

void RDA5812WriteReg(UINT8 Register, UINT8 Value);

#ifdef __cplusplus
}
#endif
#endif

