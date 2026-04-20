///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MCX_BUILDOPTION_MEMORYTRACKING
// undefine the malloc and free macros because in the PAL, system
// headers might define function prototypes of these. If they are
// actually used in the PAL module, then palMemTrack.h should be
// #included AFTER the system headers are in order to redefine
// them
#undef malloc
#undef free
#endif

// add here all startup/shutdown function prototypes:

extern void Renderer_SetCmdLineArgs_priv(int argc, char **argv);
extern void Renderer_ClearCmdLineArgs_priv(void);

extern void SetThreadLastSocketError_priv(pkRESULT lastError);
extern pkRESULT GetThreadLastSocketError_priv(void);

extern int IPTV_HAL_Decoder_HALInit_priv();
extern int IPTV_HAL_Decoder_HALExit_priv();

extern int TF_Main(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

