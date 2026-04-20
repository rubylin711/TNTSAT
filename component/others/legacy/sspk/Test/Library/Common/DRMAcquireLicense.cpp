///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "pkPAL.h"
#include "pkExecutive.h"

#include "IXDrm.h"
#include "AutoLock.h"
#include "strsafe.h"

#include "DRMAcquireLicense.h"

#include <new>
#include <string>

using namespace SSPK;

static const int s_MaxLicAcqRedirects = 3;

#ifdef DEBUG
#define TRACE_ENABLE
#endif

#if defined(TRACE_ENABLE)
#define TRACE_ERROR( printf_exp ) ( Executive_DebugPrintf printf_exp )
#else
#define TRACE_ERROR( printf_exp )   ( void ) 0
#endif


#define Chk( x ) do { if( FAILED( hr = x )) goto exit; } while( FALSE );
#define Chk2( x, y ) do { if (FAILED( hr = x )) { sErr = y; goto exit; } } while( FALSE );
#define ChkNull2( x, y ) do { if (NULL == x) { sErr = y; hr = E_POINTER; goto exit; } } while( FALSE );


// ================================================================================================

CLicenseAcquirer::CLicenseAcquirer()
{
    _poXDrm = NULL;
}

CLicenseAcquirer::~CLicenseAcquirer()
{
    if (_poXDrm)
    {
        XDRM_DestroyInstance(_poXDrm);
    }
}

HRESULT CLicenseAcquirer::Initialize()
{
    return XDRM_CreateInstance(&_poXDrm);
}



// ================================================================================================

HRESULT CLicenseAcquirer::AcquireLicense( 
            const char*  pszLicenseServerUri, 
            const char*  pszBase64KeyId, 
            std::string* pResponseCustomData )
{
    HRESULT hr = E_FAIL;

    if (NULL == pszLicenseServerUri || NULL == pszBase64KeyId)
    {
        hr = E_INVALIDARG;
        goto errorExit;
    }

    // Generate DRM header string and convert to PR header blob
    {
        static const char s_szPlayReadyHeaderTemplate[] = 
            "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\">"
            "<DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>%s</KID>"
            "<LA_URL>%s</LA_URL></DATA></WRMHEADER>";
        
        char* szDrmHeader = NULL;
        uint16_t* u16szDrmHeader = NULL;
        
        size_t cchDrmHeader = 
            strlen(s_szPlayReadyHeaderTemplate)
            + strlen(pszBase64KeyId)
            + strlen(pszLicenseServerUri)
            + 1;
    
        szDrmHeader = NEW_NO_THROW char[cchDrmHeader];
        u16szDrmHeader = NEW_NO_THROW uint16_t[cchDrmHeader];
    
        if (NULL == szDrmHeader || NULL == u16szDrmHeader)
        {
            TRACE_ERROR(("AcquireLicense FAILED to allocate buffers of length %u\n", cchDrmHeader));
            hr = E_OUTOFMEMORY;
            goto exitDrmSetup;
        }
        
        hr = StringCchPrintfA( 
                    szDrmHeader, 
                    cchDrmHeader, 
                    s_szPlayReadyHeaderTemplate, 
                    pszBase64KeyId, 
                    pszLicenseServerUri );
        
        if (pkFAILED(hr))
        {
            TRACE_ERROR(("AcquireLicense FAILED to StringCchPrintfA DRM header 0x%X\n", hr));
            goto exitDrmSetup;
        }
    
        // Convert header from zero-term char* to length of 16-bit characters
        // Then Init PlayReady license to invoke license acquisition
        {
            size_t cch;
            for (cch = 0; (cch < cchDrmHeader) && (0 != szDrmHeader[cch]); cch++)
            {
                u16szDrmHeader[cch] = szDrmHeader[cch];
            }
    
            hr = AcquireLicense(sizeof(uint16_t) * cch, (const byte*)u16szDrmHeader, pResponseCustomData);
                
            if (FAILED(hr))
            {
                TRACE_ERROR(("AcquireLicense FAILED to CPlayReadyLicense::AcquireLicense [0x%X] for DRM header\n%s\nCustom data:\n%s\n", 
                            hr, szDrmHeader, ChallengeCustomData.c_str()));
                goto exitDrmSetup;
            }
        }
    exitDrmSetup:
        delete [] szDrmHeader;
        delete [] u16szDrmHeader;
    }

errorExit: 
    if (FAILED(hr))
    {
        TRACE_ERROR(("AcquireLicense FAILED [0x%X]\n", hr));
    }
    
    return hr;
}


// ================================================================================================

HRESULT CLicenseAcquirer::AcquireLicense( 
            size_t         cbChallengeHdr,  
            const uint8_t* pbChallengeHdr, 
            std::string*   pResponseCustomData )
{
    HRESULT hr = E_FAIL;
    std::string sErr = ""; // for Chk macros
    bool fCallEndTransaction = false;
    char* pszChallenge     = NULL;
    char* pszChalResponse  = NULL;
    char* pszRedirectUrl   = NULL;
    const char* pszCustomData = (0 == ChallengeCustomData.length()) ? NULL : ChallengeCustomData.c_str();
    std::string licServerUri;

    ChkNull2(_poXDrm, "poXDrm");
    
    Chk2(_poXDrm->Init(NULL, NULL), "Init");

    _poXDrm->BeginTransaction();
    
    fCallEndTransaction = true;

    Chk2(_poXDrm->SetEnhancedData(cbChallengeHdr, pbChallengeHdr, 0, NULL), "SetEnhancedData");

    {
        char* pszServerUri = NULL;

        Chk2(_poXDrm->GenerateChallenge( &pszServerUri, pszCustomData, &pszChallenge ), "GenerateChallenge");

        ChkNull2(pszServerUri, "GenerateError(ServerUri)");
        ChkNull2(pszChallenge, "GenerateError(Challenge)");
        
        if (0 != LicenseServerUriOverride.length())
        {
            licServerUri = LicenseServerUriOverride;
        }
        else 
        {
            licServerUri = pszServerUri;
        }
        _poXDrm->MemFree(pszServerUri);
        pszServerUri = NULL;
    }
    
    for (int cRedir = 0; cRedir < s_MaxLicAcqRedirects; cRedir++)
    {      
        Chk2(_poXDrm->SendHttp( PLAYREADY_GETLICENSE_CONTENT_TYPE, pszChallenge, &pszChalResponse, licServerUri.c_str() ), "SendHttp");

        ChkNull2(pszChalResponse, "SendHttp(Response)");
        
        Chk2(_poXDrm->ProcessResponse( pszChalResponse ), "ProcessResponse");            

        hr = _poXDrm->GetPropertyFromResponse( (const uint8_t *)pszChalResponse, strlen(pszChalResponse), XDRM_RESPONSE_REDIRECT_URL, &pszRedirectUrl );
        if (FAILED(hr) || NULL == pszRedirectUrl)
        {
            break; // no redirect so exit loop
        }
        
        // switch to new URL and repeat request
        TRACE_ERROR(("CPlayReadyLicense::AcquireLicense redirect to %s\n", pszRedirectUrl));

        licServerUri = pszRedirectUrl;
        _poXDrm->MemFree(pszRedirectUrl);
        _poXDrm->MemFree(pszChalResponse);
    }
    
    if (NULL != pResponseCustomData)
    {
        char* pszCustDataResponse  = NULL;
        Chk2(_poXDrm->GetPropertyFromResponse( (const uint8_t *)pszChalResponse, strlen(pszChalResponse), XDRM_RESPONSE_CUSTOM_DATA, &pszCustDataResponse ), "GetResponseData");
        if (pszCustDataResponse)
        {
            *pResponseCustomData = pszCustDataResponse;
            _poXDrm->MemFree(pszCustDataResponse);
            pszCustDataResponse = NULL;
        }            
    }
    
exit:
    if (pszChallenge)
    {
        _poXDrm->MemFree(pszChallenge);
    }
    if (pszChalResponse)
    {
        _poXDrm->MemFree(pszChalResponse);
    }
    if( fCallEndTransaction )
    {
        _poXDrm->EndTransaction();
    }

    if (FAILED(hr))
    {
        TRACE_ERROR(("AcquireLicense(%s) failed (hr = 0x%x)\n", 
            sErr.empty() ? "" : sErr.c_str(), hr));
    }

    return hr;
}

