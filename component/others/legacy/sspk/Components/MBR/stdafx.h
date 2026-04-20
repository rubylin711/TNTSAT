///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "SSPKDefines.h"

#include <vector>
#include <AutoRefPtr.h>
#include "CManifestTrack.h"
#include "CManifestChunk.h"
#include "Trace.h"

// ===============================================================================================================
// Helper macros / inlines
// ===============================================================================================================

#ifndef  CHECK_PKRESULT_GOTO
#define CHECK_PKRESULT_GOTO( val, label ) pkResult = (val); if ( pkFAILED(pkResult) ) { goto label; }
#endif

#ifndef  CHECKNULL_GOTO
#define CHECKNULL_GOTO( val, label ) if ( (val) == NULL ) { ASSERT( false ); goto label; }
#endif

#ifndef  CHECKNULL_SET_PKRESULT_GOTO
#define CHECKNULL_SET_PKRESULT_GOTO( val, err, label ) if ( (val) == NULL ) { pkResult = (err); ASSERT( false ); goto label; }
#endif
