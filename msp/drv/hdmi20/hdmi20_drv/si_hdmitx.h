/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/***********************************************************************************/
/*  Copyright (c) 2002-2006, Silicon Image, Inc.  All rights reserved.             */
/*  No part of this work may be reproduced, modified, distributed, transmitted,    */
/*  transcribed, or translated into any language or computer format, in any form   */
/*  or by any means without written permission of: Silicon Image, Inc.,            */
/*  1060 East Arques Avenue, Sunnyvale, California 94085                           */
/***********************************************************************************/
#include "si_datatypes.h"
#include "mt_type.h"
#include "drv_reg_proc.h"
#include "si_drv_tx_regs.h"

#ifndef __HDMI_TX_H__
#define __HDMI_TX_H__

void SI_Init_DVITX(void);
void SI_SetHdmiVideo(MT_U8 Enabled);
void SI_SetHdmiAudio(MT_U8 Enabled);
void SI_Start_HDMITX(void);
void SI_HW_ResetHDMITX( void );
MT_BOOL SI_HDMI_Setup_INBoot(MT_U32 *VIC);
void SI_WakeUpHDMITX(void);
void SI_PowerDownHdmiTx(void);
MT_U8 SI_IsTXInHDMIMode( void );
void SI_SW_ResetHDMITX(void);
MT_BOOL SI_IsHDMIResetting(void);

#define GetSysStat() ((SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_HPD_RSEN) \
					   & ((BIT_MSK__TPI_HPD_RSEN__HPD_STATE & SII_BIT2) | BIT_MSK__TPI_HPD_RSEN__HPD)) \
					  == ((BIT_MSK__TPI_HPD_RSEN__HPD_STATE & SII_BIT2) | BIT_MSK__TPI_HPD_RSEN__HPD))
#define GetRsenStat() ((SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_HPD_RSEN) \
							   & ((BIT_MSK__TPI_HPD_RSEN__RSEN_STATE & SII_BIT6) | BIT_MSK__TPI_HPD_RSEN__RSEN)) \
							  == ((BIT_MSK__TPI_HPD_RSEN__RSEN_STATE & SII_BIT6) | BIT_MSK__TPI_HPD_RSEN__RSEN))

MT_U32 SI_HPD_Status( void );
MT_U32 SI_Is_HPDKernelCallback_DetectHPD( void );
MT_U32 SI_HPD_SetHPDUserCallbackCount( void );
MT_U32 SI_Is_HPDUserCallback_DetectHPD( void );

MT_S32 SI_TX_IsHDMImode(void);

MT_VOID SI_HW_ResetCtrl(int iEnable);
MT_VOID SI_HW_ResetPhy(int iEnable);
void SI_SetHdmiAudioPkg(MT_U8 Enabled);
#endif

