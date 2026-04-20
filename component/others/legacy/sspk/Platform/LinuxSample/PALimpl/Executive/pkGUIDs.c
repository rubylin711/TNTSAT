///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// pkGUIDs.c
//
// Allocation of common GUIDs declared in platGuids.h

#define INITGUID // cause DEFINE_GUID to allocate memory

#include "platGuids.h"

DEFINE_GUID(GUID_NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

