///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//===========================================================================
// Includes
//===========================================================================
#include <pkPAL.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

#include <pkTestFramework.h>
#include <TLCommon.h>

// Override the ERRTRACE print method
#define ERRTRACE_PRINTMSG(printf_exp)   TF_Printf printf_exp
#include <errTrace.h>

//===========================================================================
// Globals
//===========================================================================

// Embedded Files

static int g_cEmbeddedFiles = 0;

static TLC_FILE_EMBEDDED *g_rgEmbeddedFiles = NULL;


//===========================================================================
// CFileEmbedded
//===========================================================================

class CFileEmbedded : public ITLCFile
{
public:
    CFileEmbedded();
    ~CFileEmbedded();
    virtual pkRESULT FOpen(const char *pszFilename, const char *pszMode);
    virtual pkRESULT FClose(void);
    virtual pkRESULT GetFileSize(size_t *piSize);
    virtual pkRESULT FRead(size_t *pcItemsRead, void *pbBuffer, size_t iSize,
        size_t iCount);
    virtual pkRESULT FSeek(int32_t offset, TF_TestVectors_SeekWhence whence);

private:
    TLC_FILE_EMBEDDED *m_pEmbed;
    size_t m_idxCurrent;
};

CFileEmbedded::CFileEmbedded()
{
    m_pEmbed = NULL;
    m_idxCurrent = 0;
}

CFileEmbedded::~CFileEmbedded()
{
    FClose();
}

pkRESULT CFileEmbedded::FOpen(const char *pszFilename, const char *pszMode)
{
    pkRESULT pkRes = pkS_OK;
    int i;

    if (NULL == pszMode || 'r' != pszMode[0]) // only read allowed
    {
        pkRes = TraceResult(pkE_INVALIDARG);
        goto exit;
    }

    FClose();

    // Find a match for this embedded file
    for (i = 0; i < g_cEmbeddedFiles; i++)
    {
        if (0 == strcmp(pszFilename, g_rgEmbeddedFiles[i].pszEmbedFilename))
        {
            // Found a match
            m_pEmbed = &g_rgEmbeddedFiles[i];
            break;
        }
    }

    if (NULL == m_pEmbed)
    {
        // Did not find a match for this file, fail the fopen
        pkRes = TraceResult(pkE_FAIL);
        goto exit;
    }

    m_idxCurrent = 0;

exit:
    return pkRes;
}

pkRESULT CFileEmbedded::FClose(void)
{
    m_pEmbed = NULL;
    return pkS_OK;
}

pkRESULT CFileEmbedded::GetFileSize(size_t *piSize)
{
    pkRESULT pkRes = pkS_OK;

    if (NULL == m_pEmbed)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    *piSize = *m_pEmbed->pcbFileData;

exit:
    return pkRes;
}


pkRESULT CFileEmbedded::FRead(size_t *pcItemsRead, void *pbBuffer, size_t iSize,
                              size_t iCount)
{
    pkRESULT pkRes = pkS_OK;
    size_t cbBytesToRead;

    if (NULL == m_pEmbed)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    if (m_idxCurrent >= *m_pEmbed->pcbFileData)
        cbBytesToRead = 0;
    else
    {
        cbBytesToRead = *m_pEmbed->pcbFileData -m_idxCurrent;
        if (cbBytesToRead > iSize * iCount)
        {
            cbBytesToRead = iSize * iCount;
        }
    }
    memcpy(pbBuffer, &m_pEmbed->pbFileData[m_idxCurrent],
        cbBytesToRead);
    *pcItemsRead = cbBytesToRead / iSize; // Truncate down
    m_idxCurrent += cbBytesToRead;

exit:
    return pkRes;
}

pkRESULT CFileEmbedded::FSeek(int32_t offset, TF_TestVectors_SeekWhence whence)
{
    pkRESULT pkRes = pkS_OK;

    if (NULL == m_pEmbed)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    int32_t position;
    switch (whence)
    {
    case TF_TestVectors_Seek_Set:
        position = offset;
        break;
    case TF_TestVectors_Seek_Cur:
        position = m_idxCurrent + offset;
        break;
    case TF_TestVectors_Seek_End:
        position = *m_pEmbed->pcbFileData + offset;
        break;
    default:
        pkRes = pkE_INVALIDARG;
        goto exit;
    }

    if (position < 0 || (uint32_t)position > *m_pEmbed->pcbFileData)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    m_idxCurrent = position;

exit:
    return pkRes;
}


//===========================================================================
// CFileFOpen
//===========================================================================
class CFileFOpen : public ITLCFile
{
public:
    CFileFOpen();
    ~CFileFOpen();
    virtual pkRESULT FOpen(const char *pszFilename, const char *pszMode);
    virtual pkRESULT FClose(void);
    virtual pkRESULT GetFileSize(size_t *piSize);
    virtual pkRESULT FRead(size_t *pcItemsRead, void *pbBuffer, size_t iSize,
        size_t iCount);
    virtual pkRESULT FSeek(int32_t offset, TF_TestVectors_SeekWhence whence);

private:
    pkHANDLE m_hFile;
};

CFileFOpen::CFileFOpen()
{
    m_hFile = NULL;
}

CFileFOpen::~CFileFOpen()
{
    FClose();
}

pkRESULT CFileFOpen::FOpen(const char *pszFilename, const char *pszMode)
{
    pkRESULT pkRes = pkS_OK;

    if (NULL == pszMode || 'r' != pszMode[0]) // only read allowed
    {
        pkRes = TraceResult(pkE_INVALIDARG);
        goto exit;
    }

    pkRes = TF_TestVectors_Open(&m_hFile, pszFilename);
    if (pkFAILED(pkRes))
    {
        TraceResult(pkRes);
        goto exit;
    }

exit:
    return pkRes;
}

pkRESULT CFileFOpen::FClose(void)
{
    pkRESULT pkRes = pkS_OK;

    pkRes = TF_TestVectors_Close(m_hFile);
    if (pkFAILED(pkRes))
    {
        // Trace but ignore error
        TraceResult(pkRes);
        pkRes = pkS_OK;
    }
    m_hFile = NULL;

    return pkRes;
}

pkRESULT CFileFOpen::GetFileSize(size_t *piSize)
{
    pkRESULT pkRes = pkS_OK;
    int32_t lCurr;
    int32_t offset;

    if (NULL == m_hFile)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    TRACEPK_EXIT(pkRes, TF_TestVectors_GetOffset(m_hFile, &lCurr));

    TRACEPK_EXIT(pkRes, TF_TestVectors_Seek(m_hFile, 0, TF_TestVectors_Seek_End));

    TRACEPK_EXIT(pkRes, TF_TestVectors_GetOffset(m_hFile, &offset));

    TRACEPK_EXIT(pkRes, TF_TestVectors_Seek(m_hFile, lCurr, TF_TestVectors_Seek_Set));

    *piSize = (size_t)offset;

exit:
    return pkRes;
}

pkRESULT CFileFOpen::FRead(size_t *pcItemsRead, void *pvBuffer, size_t iSize,
                           size_t iCount)
{
    pkRESULT pkRes = pkS_OK;

    size_t cbBuffer = iSize * iCount;

    if (NULL == m_hFile || 0 == cbBuffer)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    if (NULL == pvBuffer)
    {
        pkRes = TraceResult(pkE_POINTER);
        goto exit;
    }

    TRACEPK_EXIT(pkRes, TF_TestVectors_ReadBlock(m_hFile, (uint8_t *)pvBuffer, &cbBuffer));

    if (NULL != pcItemsRead)
    {
        *pcItemsRead = cbBuffer / iSize;
    }

exit:
    return pkRes;
}

pkRESULT CFileFOpen::FSeek(int32_t offset, TF_TestVectors_SeekWhence whence)
{
    pkRESULT pkRes = pkS_OK;

    if (NULL == m_hFile)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    TRACEPK_EXIT(pkRes, TF_TestVectors_Seek(m_hFile, offset, whence));

exit:
    return pkRes;
}

//===========================================================================
// Public Functions
//===========================================================================

ITLCFile::~ITLCFile()
{
}

//
// Function: TLC_AddEmbeddedFiles
//
// Purpose: Initializes support for embedded files and namespace remappings.
//   This function is optional and does not need to be called if no embedded
//   files exist. Note that this function has global effect.
//
// Arguments:
//   rgEmbeddedFiles [in] - array of embedded file descriptors. This array
//     must remain valid for the duration of program execution.
//   cEmbeddedFiles [in] - number of entries in rgEmbeddedFiles.
//
// Returns:
//   pkRESULT indicating success or failure.
//
// An example of parameter setup:
//   static TLC_FILE_EMBEDDED s_rgEmbeddedFiles[] = {
//       {URISCHEME_EMBEDDEDFILE "myEmbedFileFoo.ext", RW_READONLY, pbMyEmbedFileFoo, &cbMyEmbedFileFoo},
//       {URISCHEME_EMBEDDEDFILE "myEmbedFileBar.ext", RW_READONLY, pbMyEmbedFileBar, &cbMyEmbedFileBar}
//   }
//   static int s_cEmbeddedFiles = ARRAYSIZE(s_rgEmbeddedFiles);

pkRESULT TLC_AddEmbeddedFiles(TLC_FILE_EMBEDDED *rgEmbeddedFiles, int cEmbeddedFiles)
{
    pkRESULT pkRes = pkS_OK;

    static bool_t s_fInitialized = FALSE;

    if (s_fInitialized)
    {
        // In future, we may support multiple calls (so that multiple embedded file
        // sources could be chained together, for instance), but for now, one call is
        // all we do
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    g_rgEmbeddedFiles = rgEmbeddedFiles;
    g_cEmbeddedFiles = cEmbeddedFiles;
    s_fInitialized = TRUE;

exit:
    return pkRes;
}


pkRESULT TLC_CreateFile(const char *pszFilename, ITLCFile **ppITLCFile)
{
    pkRESULT pkRes = pkS_OK;

    // Check if the name refers to an embedded file
    if (0 == strncmp(pszFilename, URISCHEME_EMBEDDEDFILE, sizeof(URISCHEME_EMBEDDEDFILE) - 1))
    {
        *ppITLCFile = new CFileEmbedded;
        TRACEPTR_EXIT(pkRes, *ppITLCFile);
    }
    else
    {
        *ppITLCFile = new CFileFOpen;
        TRACEPTR_EXIT(pkRes, *ppITLCFile);
    }

exit:
    return pkRes;
}

