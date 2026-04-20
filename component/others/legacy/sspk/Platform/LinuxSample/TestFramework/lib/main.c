///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include <pkPAL.h>
#include <pkTestFramework.h>

#include <platPrivate.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>


#ifdef ENABLE_APP
//=============================================================================
// L O C A L    F U N C T I O N    D E F I N I T I O N
//=============================================================================
static void s_SignalHandler(int theSig)
{
    // TF_Printf("s_SignalHandler(%d)\n", theSig);
    Test_Abandon();
}

//=============================================================================
// M A I N    F U N C T I O N    D E F I N I T I O N
//=============================================================================
int main(int argc, char *argv[])
{
    int retval=0;

    // Trap the interupt signal
    signal(SIGINT, s_SignalHandler);

    // No buffer is used for quick output
    setvbuf(stdout, NULL, _IONBF, 0);

    retval=TF_Main(argc, argv);

    // Un-trap the interrupt signal
    signal(SIGINT, SIG_DFL);

    return retval;
}
#endif

// ================================================
#ifndef ARRAYSIZE
    #define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

static pkRESULT pkAPI s_FileOpen
    (
        pkHANDLE* phFile,
        const char* szFileURI, // Note: URI scheme support is platform dependent
        const char* szMode
    )
{
    FILE* pFile;
    pkRESULT pkr = pkS_OK;

    const char szURI_file[] = "file:///"; // only empty host name handled
    size_t cchURIScheme = ARRAYSIZE(szURI_file) - sizeof(szURI_file[0]);
    size_t cchURI = 0;

    if (NULL == phFile || NULL == szFileURI)
    {
        pkr = pkE_POINTER;
        goto bail;
    }

    *phFile = NULL;

    // If prefixed with file:///, skip past that, otherwise just try to open it.
    if (0 == strncmp(szFileURI, szURI_file, cchURIScheme))
    {
        cchURI = cchURIScheme;
    }

    pFile =  fopen(&szFileURI[cchURI], szMode);

    if (NULL == pFile)
    {
        pkr = pkE_FILE_NOT_FOUND;
        goto bail;
    }

    *phFile = (pkHANDLE)pFile;

bail:
    return pkr;
}


pkRESULT pkAPI s_FileClose
    (
        pkHANDLE hFile
    )
{
    pkRESULT pkr = pkS_OK;
    int retval = 0;

    if (NULL == hFile)
    {
        pkr = pkE_HANDLE;
        goto bail;
    }

    retval = fclose((FILE *)hFile);
    if (0 != retval)
    {
        pkr = pkE_FAIL;
    }

bail:
    return pkr;
}


// ================================================
 // Input of "test vector" binary data

pkRESULT pkAPI TF_TestVectors_Open
    (
        pkHANDLE* phVector,
        const char* szVectorURI // Note: URI scheme support is platform dependent
    )
{
    return s_FileOpen( phVector, szVectorURI, "rb" );
}

pkRESULT pkAPI TF_TestVectors_Seek
    (
        pkHANDLE hVector,
        int32_t offset,
        TF_TestVectors_SeekWhence eWhence
    )
{
    pkRESULT pkr = pkS_OK;
    int iWhence = -1;

    if (NULL==hVector)
    {
        pkr = pkE_HANDLE;
        goto bail;
    }

    switch (eWhence)
    {
    case TF_TestVectors_Seek_Set: iWhence = SEEK_SET; break;
    case TF_TestVectors_Seek_Cur: iWhence = SEEK_CUR; break;
    case TF_TestVectors_Seek_End: iWhence = SEEK_END; break;
    default:
        pkr = pkE_INVALIDARG;
        goto bail;
    }

    if (0 != fseek((FILE *)hVector, (long)offset, iWhence))
    {
        pkr = pkE_FAIL;
    }

bail:
    return pkr;
}


extern pkRESULT pkAPI TF_TestVectors_GetOffset
    (
        pkHANDLE hVector,
        int32_t* pOffset
    )
{
    pkRESULT pkr = pkS_OK;
    long offset = 0;

    if (NULL==hVector)
    {
        pkr = pkE_HANDLE;
        goto bail;
    }

    if (NULL==pOffset)
    {
        pkr = pkE_POINTER;
        goto bail;
    }

    offset = (int32_t) ftell((FILE *)hVector);
    if (offset < 0)
    {
        pkr = pkE_FAIL;
        goto bail;
    }

    *pOffset = (int32_t)offset;

bail:
    return pkr;
}


pkRESULT pkAPI TF_TestVectors_ReadBlock
    (
        pkHANDLE hVector,
        uint8_t* vectBuffer,
        size_t* pVectorBufferSize
    )
{
    pkRESULT pkr = pkS_OK;

    if (NULL==hVector)
    {
        pkr = pkE_HANDLE;
        goto bail;
    }

    if (NULL==vectBuffer || NULL==pVectorBufferSize)
    {
        pkr = pkE_POINTER;
        goto bail;
    }

    // read the requested number of 1-byte elements
    {
        size_t cbToRead = *pVectorBufferSize;

        *pVectorBufferSize = fread(vectBuffer, 1, cbToRead, (FILE *)hVector);

        if (cbToRead != *pVectorBufferSize)
        {
            pkr = pkS_FALSE;
        }
    }

bail:
    return pkr;
}


pkRESULT pkAPI TF_TestVectors_Close
    (
        pkHANDLE hVector
    )
{
    return s_FileClose(hVector);
}


// ================================================
 // Input of "test scripting" string data

pkRESULT pkAPI TF_TestScripting_Open
    (
        pkHANDLE* phScript,
        const char* szScriptURI // Note: URI scheme support is platform dependent
    )
{
    return s_FileOpen( phScript, szScriptURI, "r" );
}

pkRESULT pkAPI TF_TestScripting_ReadString
    (
        pkHANDLE hScript,
        char* szBuffer,
        size_t cbBuffer
    )
{
    pkRESULT pkr = pkS_OK;

    if (NULL == hScript)
    {
        pkr = pkE_HANDLE;
        goto bail;
    }

    if (NULL == szBuffer)
    {
        pkr = pkE_POINTER;
        goto bail;
    }

    if (0 == cbBuffer || cbBuffer > 10000)
    {
        pkr = pkE_INVALIDARG;
        goto bail;
    }

    if (NULL == fgets(szBuffer, (int)cbBuffer, (FILE *)hScript))
    {
        pkr = feof((FILE *)hScript) ? pkS_FALSE : pkE_FAIL;
    }

bail:
    return pkr;
}

pkRESULT pkAPI TF_TestScripting_Close
    (
        pkHANDLE hScript
    )
{
    return s_FileClose(hScript);
}


// ================================================
// Output of "log" data

pkRESULT pkAPI TF_Logging_Open
    (
        pkHANDLE* phLog,
        const char* szLogURI // Note: URI scheme support is platform dependent
    )
{
    return s_FileOpen( phLog, szLogURI, "w+" );
}

pkRESULT pkAPI TF_Logging_Printf
    (
        pkHANDLE hLog,
        const char* szFormat,
        ...
    )
{
    va_list arglist;
    int retval;

    pkRESULT pkr = pkS_OK;

    va_start(arglist,szFormat);
    retval = vfprintf( (FILE *)hLog, szFormat, arglist);
    va_end(arglist);

    if (retval < 0)
    {
        pkr = pkE_INVALIDARG;
        goto bail;
    }

    fflush((FILE *)hLog);

bail:
    return pkr;
}

pkRESULT pkAPI TF_Logging_Close
    (
        pkHANDLE hLog
    )
{
    return s_FileClose(hLog);
}


// ================================================
// Standard out

pkRESULT pkAPI TF_Print( const char *psz )
{
    puts( psz );
    return( pkS_OK );
}

pkRESULT pkAPI TF_Printf( const char *pszFmt, ... )
{
    va_list arglist;
    int retval;
    pkRESULT pkr;

    va_start(arglist,pszFmt);
    retval = vprintf(pszFmt, arglist);
    va_end(arglist);

    pkr = ( retval >= 0 ) ? pkS_OK : pkE_INVALIDARG;

    return pkr;
}

pkRESULT pkAPI TF_Printf_NONE( const char *pszFmt, ... )
{
    return pkS_OK;
}


