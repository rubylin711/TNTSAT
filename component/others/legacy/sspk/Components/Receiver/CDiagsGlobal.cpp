///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CDiagsGlobal.h"
#include "Trace.h"

// ===============================================================================================================
// Global parameters across all streams (not reset on retune)
// Static diags if no live tuner avaialble. Retrieves static total counters only.
// ===============================================================================================================

//Counter for tracking total packets received across all streams
uint32 CDiagsGlobal::TotalPacketsReceived = 0;
//Counters for tracking holes
uint32 CDiagsGlobal::TotalPacketsExpired = 0;
uint32 CDiagsGlobal::TotalHolePackets = 0;

// ===============================================================================================================
// ===============================================================================================================
