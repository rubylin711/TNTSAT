/******************************************************************************
*
* Copyright 2013, Deco, Inc.  All rights reserved.
* No part of this work may be reproduced, modified, distributed, transmitted,
* transcribed, or translated into any language or computer format, in any form
* or by any means without written permission of
* Deco, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
*
*****************************************************************************/
/**
* @file
*
* @brief Tx API
*
*****************************************************************************/
#ifndef __SI_APP_TX_API_H__
#define __SI_APP_TX_API_H__

/***** #include statements ***************************************************/

/***** public macro definitions **********************************************/

/***** public type definitions ***********************************************/

/***** external functions ****************************************************/

/***** public functions ******************************************************/
bool_t SiiTxCreate(SiiPlatformInterface_t *pInterfaceInfo);
bool_t SiiTxReCreate(SiiPlatformInterface_t *pInterfaceInfo);
void SiiTxDelete(void);
void SiiCecSwEnable(void);
void SiiCecSwDisable(void);
void SiiTxTpgEnable(SiiInst_t inst, uint8_t enable);
void SiiTxTpgFormatSet(SiiInst_t inst, uint8_t format);
void SiiTxTpgPatternSet(SiiInst_t inst, uint8_t pattern);

SiiInst_t getTxCraInstance(void);
SiiInst_t getTxInstance(void);
bool_t getScdcEnable(void);

#endif // __SI_APP_TX_API_H__
