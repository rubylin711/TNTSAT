///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define _strnicmp strncasecmp
#define _stricmp strcasecmp
#define _vsnprintf vsnprintf

#ifdef __cplusplus

#ifdef MSPK_USING_STD_TEMPLATE_LIBS

// The reason that we need to #include STL files for GCC here (in the platform layer)
// is because GCC STL uses __in and __out as argument names in their macros and functions.
// This conflicts with #define's for __int and __out in pkSALDefaults.h, which gets
// included after platBase.h. Otherwise we should normally include STL after platform headers.

#include <algorithm>
#include <locale>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <deque>

#endif // MSPK_USING_STD_TEMPLATE_LIBS

#endif // __cplusplus

// a number of Components files assume memset and memcpy are defined
#include <memory.h>

