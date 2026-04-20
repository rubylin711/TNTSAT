///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __pkTestFramework_h__
#define __pkTestFramework_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    TF_E_VIDEO_OUT_NTSC_M,       // 480i  59.94Hz
    TF_E_VIDEO_OUT_PAL_BG,       // 576i  50.00Hz
    TF_E_VIDEO_OUT_480p59,       // 480p  59.94Hz
    TF_E_VIDEO_OUT_720p60,       // 720p  60.00Hz
    TF_E_VIDEO_OUT_720p59,       // 720p  59.94Hz
    TF_E_VIDEO_OUT_1080i60,      // 1080i 60.00Hz
    TF_E_VIDEO_OUT_1080i59,      // 1080i 59.94Hz
    TF_E_VIDEO_OUT_1080p60,      // 1080p 60.00Hz
    TF_E_VIDEO_OUT_1080p59,      // 1080p 59.94Hz
    TF_E_VIDEO_OUT_HDMI_480p,    // 480p  59.94Hz
    TF_E_VIDEO_OUT_HDMI_720p,    // 720p  59.94Hz
    TF_E_VIDEO_OUT_HDMI_1080i,   // 1080i 59.94Hz
    TF_E_VIDEO_OUT_HDMI_1080p,   // 1080i 59.94Hz
    TF_E_VIDEO_OUT_HDMI_AUTO     // auto select
} TF_E_VIDEO_OUT_STANDARD;

// NOTE: The TF_Config structure can have new fields added without
//       having to update all the platforms. However, existing fields
//       must not be changed (other than their order in the struct).

typedef struct tagTF_Config
{
    uint32_t size; // this field must always be first

    TF_E_VIDEO_OUT_STANDARD eVideoOutStandard;
    uint32_t graphicsWidth;
    uint32_t graphicsHeight;

} TF_Config;

// ================================================
// Implemented by the test

// Test_GetConfig
//
// Test uses command line arguments as needed and fills out any needed TF_Config fields.
// Note: the *pConfig structure is pre-initialized to zero.
//
// Return of failed or pkS_FALSE result suppresses further setup and call to Test_Run.
// Return pkS_FALSE when only "usage" is output.
//
extern pkRESULT pkAPI Test_GetConfig(int argc, char *argv[], TF_Config *pConfig);

// Test_Run
//
// Test primary execution thread (using runThreadStackSize)
//
// Note: If test runs for more than a few seconds, Test_Abandon should be able to stop it.
//
extern pkRESULT pkAPI Test_Run(void);

// Test_Abandon
//
// Note: If Test_Run is active, it must clean up and exit when Test_Abandon is called.
//
extern pkRESULT pkAPI Test_Abandon(void);


// ================================================
// Implemented by the platform

// Input of "test vector" binary data

extern pkRESULT pkAPI TF_TestVectors_Open
    (
        pkHANDLE* phVector,
        const char* szVectorURI // Note: URI scheme support is platform dependent
    );

typedef enum
{
    TF_TestVectors_Seek_Set, // absolute position
    TF_TestVectors_Seek_Cur, // relative to current position
    TF_TestVectors_Seek_End  // relative to end of stream
}
TF_TestVectors_SeekWhence;

extern pkRESULT pkAPI TF_TestVectors_Seek
    (
        pkHANDLE hVector,
        int32_t offset,
        TF_TestVectors_SeekWhence whence
    );

extern pkRESULT pkAPI TF_TestVectors_GetOffset
    (
        pkHANDLE hVector,
        int32_t* pOffset
    );

extern pkRESULT pkAPI TF_TestVectors_ReadBlock
    (
        pkHANDLE hVector,
        uint8_t* vectBuffer,
        size_t* pVectorBufferSize
    );

extern pkRESULT pkAPI TF_TestVectors_Close
    (
        pkHANDLE hVector
    );


// Input of "test scripting" strings

extern pkRESULT pkAPI TF_TestScripting_Open
    (
        pkHANDLE* phScript,
        const char* szScriptURI // Note: URI scheme support is platform dependent
    );

extern pkRESULT pkAPI TF_TestScripting_ReadString
    (
        pkHANDLE hScript,
        char* szBuffer, // always null terminated
        size_t cbBuffer // including trailing null terminator
    );
    // Note: Semantics conform to the C99 fgets standard.
    // Returns pkS_FALSE upon reaching end-of-file

extern pkRESULT pkAPI TF_TestScripting_Close
    (
        pkHANDLE hScript
    );


//
// Log file operations
//

extern pkRESULT pkAPI TF_Logging_Open
    (
        pkHANDLE* phLog,
        const char* szLogURI // Note: URI scheme support is platform dependent
    );

extern pkRESULT pkAPI TF_Logging_Printf
    (
        pkHANDLE hLog,
        const char* szFormat,
        ...
    );

extern pkRESULT pkAPI TF_Logging_Close
    (
        pkHANDLE hLog
    );

//
// Standard output operations
//

extern pkRESULT pkAPI TF_Print( const char *psz );

extern pkRESULT pkAPI TF_Printf( const char *pszFmt, ... );

extern pkRESULT pkAPI TF_Printf_NONE( const char *pszFmt, ... );

#ifdef __cplusplus
}
#endif

#endif //__pkTestFramework_h__


