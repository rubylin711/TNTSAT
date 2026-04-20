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
* @brief API testing API
*
*****************************************************************************/

#ifndef __API_TESTING_API_H__
#define __API_TESTING_API_H__

#include "si_lib_obj_api.h"

typedef struct {
	SiiInst_t        instTx;
	SiiInst_t        instSiIMonHandle;
	SiiInst_t		 instTxCra;
	SiiInst_t		 instTpg;
	SiiInst_t		 scdcInst;

	bool_t			 bSimmonHandlingEn;
	bool_t			 bTpgEn;
	bool_t			 bProgrammPebbelsEn;
	bool_t			 bCecEn;
	bool_t           bScDcEn;
} TestModulesInstInfo_t;

void SiiApiTestModuleCreate(SiiPlatformInterface_t *pInterface, TestModulesInstInfo_t *pInfo );
void SiiApiTestModuleDelete( void );

#endif //__API_TESTING_API_H__
