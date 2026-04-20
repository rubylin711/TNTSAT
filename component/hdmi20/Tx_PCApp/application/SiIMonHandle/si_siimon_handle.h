//***************************************************************************
//!file     si_siimon_handle.h
//!brief    Dino-specific configuration
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Deco, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2013, Deco, Inc.  All rights reserved.
//***************************************************************************/
#ifndef __SI_SIIMON_HANDLE_H__
#define __SI_SIIMON_HANDLE_H__

#include "si_lib_obj_api.h"
#if (MT_SDK_COMPILE_HDMI20 == 0)
	#include <Windows.h>
#endif

SiiInst_t SiIMonHandleCreate(SiiInst_t inst);
void SiIMonHandleDelete(SiiInst_t inst);
bool_t SiIMonHandleStart(SiiInst_t inst, LPCWSTR port);
bool_t SiIMonHandleStop(SiiInst_t inst);

#endif
