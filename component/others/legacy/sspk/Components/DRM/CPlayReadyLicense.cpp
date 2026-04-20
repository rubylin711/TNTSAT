///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CPlayReadyLicense.h"
#include "IXDrm.h"
#include "Trace.h"
#include "pkExecutive.h"

#include <string>
using namespace std;

//#define PRLICENSE_SPEW
#if defined(PRLICENSE_SPEW)
//#define PRLICENSE_MSG(x) TRACE(x)
#define PRLICENSE_MSG( printf_exp ) ( Executive_DebugPrintf printf_exp )
#else
#define PRLICENSE_MSG(x)
#endif

class CXDrmManager
{
public:
    CXDrmManager()
        : _libXDrm(NULL)
        , _fnCreateInstance(NULL)
        , _fnDestroyInstance(NULL)
    {
    }

    ~CXDrmManager()
    {
    }

    static CXDrmManager* Instance()
    {
        if (!_instance)
        {
            _instance = new CXDrmManager;
            ASSERT(_instance);
        }
        return _instance;
    }

    IXDrm* Get(void)
    {
        if (!Load())
            return NULL;

        IXDrm *pIXDrm = NULL;
        pkRESULT pkResult = _fnCreateInstance(&pIXDrm);
        if (pkFAILED(pkResult))
        {
            TRACE_ERROR(("CXDrmManager::Get(): _fnCreateInstance FAILED (pkResult = 0x%08x)", pkResult));
            return NULL;
        }

        return pIXDrm;
    }

    void Release(IXDrm* pIXDrm)
    {
        if (_fnDestroyInstance)
        {
            pkRESULT pkResult = _fnDestroyInstance(pIXDrm);
            if (pkFAILED(pkResult))
            {
                TRACE_ERROR(("CXDrmManager::Release(): _fnDestroyInstance FAILED (pkResult = 0x%08x)", pkResult));
                return;
            }
        }
    }

private:
    bool Load()
    {
        _fnCreateInstance = (pfn_XDRM_CreateInstance)XDRM_CreateInstance;
        _fnDestroyInstance = (pfn_XDRM_DestroyInstance)XDRM_DestroyInstance;
        return true;
    }

private:
    void*                    _libXDrm;
    pfn_XDRM_CreateInstance  _fnCreateInstance;
    pfn_XDRM_DestroyInstance _fnDestroyInstance;

    static CXDrmManager*     _instance;
};

CXDrmManager* CXDrmManager::_instance = NULL;
extern "C" IXDRM_HRESULT XDrmOPLCallback(XDRM_OPL_DATA *pOPLData, const void* pvContext)
{
    assert(pvContext && pOPLData);

    CPlayReadyLicense* pContext = (CPlayReadyLicense*)pvContext;
    pContext->OPLCallback(pOPLData);
    return pkS_OK;
}

CPlayReadyLicense::CPlayReadyLicense()
    : _pIXDrm(NULL)
    , _pCallbackSink(NULL)
{
    memset(_drmTimes, 0, sizeof(_drmTimes));
    memset(&_guidResID, 0, sizeof(GUID));
}

CPlayReadyLicense::CPlayReadyLicense(std::string &CustomData, std::string &LicenseServerUri)
    : _pIXDrm(NULL)
    , _pCallbackSink(NULL)
    , ChallengeCustomData(CustomData)
    , LicenseServerUriOverride(LicenseServerUri)
{
    memset(_drmTimes, 0, sizeof(_drmTimes));
    memset(&_guidResID, 0, sizeof(GUID));
}

CPlayReadyLicense::CPlayReadyLicense(std::string &CustomData, std::string &LicenseServerUri, std::string &CertPath)
    : _pIXDrm(NULL)
    , _pCallbackSink(NULL)
    , ChallengeCustomData(CustomData)
    , LicenseServerUriOverride(LicenseServerUri)
    , CertPathOverride(CertPath)
{
    memset(_drmTimes, 0, sizeof(_drmTimes));
    memset(&_guidResID, 0, sizeof(GUID));
}
CPlayReadyLicense::~CPlayReadyLicense()
{
    Cleanup();
}

void CPlayReadyLicense::Cleanup(void)
{
    _pCallbackSink = NULL;

    if (_pIXDrm)
    {
        _pIXDrm->ReleaseDecryptContext( _defaultKeyID.Size(), _defaultKeyID.Data() );

        CXDrmManager::Instance()->Release(_pIXDrm);
        _pIXDrm = NULL;
    }

    _defaultKeyID.Clear();
}

#define Chk( x ) do { if( pkFAILED( pkResult = x )) goto exit; } while( FALSE );
#define Chk2( x, y ) do { if (pkFAILED( pkResult = x )) { sErr = y; goto exit; } } while( FALSE );

bool CPlayReadyLicense::Init(
    _In_ size_t cbHdr,
    _In_count_(cbHdr) const byte* pbHdr,
    _In_ size_t cbKeyId,
    _In_count_(cbKeyId) const byte* pbKeyId,
    _In_opt_ IDrmCallbackSink* pCallback,
    _In_ bool fInitOnly,
    _Out_opt_ bool *pfIsDecrypterOnDemand )
{
    bool fCallEndTransaction = false;
    if (!_pIXDrm)
    {
        _pIXDrm = CXDrmManager::Instance()->Get();
        if (!_pIXDrm)
        {
            return false;
        }
    }
    const char* pszCertPath = (0 == CertPathOverride.length()) ? NULL : CertPathOverride.c_str();

    pkRESULT pkResult;
    string sErr = "";

    _pCallbackSink = pCallback;
    PRLICENSE_MSG(("CPlayReadyLicense::Init() - Init fInitOnly=%d, pbKeyId=0x%x, pfIsDecrypterOnDemand=%x", fInitOnly, pbKeyId, pfIsDecrypterOnDemand));
    Chk2(_pIXDrm->Init(XDrmOPLCallback, (void*) this), "Init");

    if (pszCertPath)
    {
        _pIXDrm->SetCertPath(pszCertPath);
    }

    _pIXDrm->BeginTransaction();
    fCallEndTransaction = true;
    PRLICENSE_MSG(("CPlayReadyLicense::Init() - SetEnhancedData"));

    Chk2(_pIXDrm->SetEnhancedData(cbHdr, pbHdr, cbKeyId, pbKeyId), "SetEnhancedData");
    PRLICENSE_MSG(("CPlayReadyLicense::Init() - SetRights"));
    Chk2(_pIXDrm->SetRights(XDRM_RIGHT_PLAYBACK), "SetRights");

    if( pfIsDecrypterOnDemand != NULL )
    {
        *pfIsDecrypterOnDemand = IsDecrypterOnDemand();
    }

    if (fInitOnly)
    {
        goto exit;
    }

    switch (_pIXDrm->GetDRMVersionCode())
    {
    case XDRM_VERSIONCODE_PLAYREADY:
        {
            void *pvDecryptContext = NULL;
            _type = DrmEncryptionType_PlayReady;
            if (pbKeyId == NULL)
            {
                ASSERT(cbKeyId == 0);
                Chk(_pIXDrm->AcquireDecryptContext(0, NULL, &pvDecryptContext));
                Chk(AcquireLicense(pvDecryptContext));
            }
            else
            {
                ASSERT(cbKeyId > 0);

                if( !_defaultKeyID.Set( pbKeyId, cbKeyId ) )
                {
                    Chk(pkE_OUTOFMEMORY);
                }

                Chk(_pIXDrm->AcquireDecryptContext( _defaultKeyID.Size(), _defaultKeyID.Data(), &pvDecryptContext ));

                Chk2(_pIXDrm->CanDecrypt(pvDecryptContext, true), "CanDecrypt");
            }
            Chk2(_pIXDrm->Commit(), "Commit");
        }
        break;
    default:
        // Unsupported license type
        ASSERT(false);
        _type = DrmEncryptionType_Unknown;
        Chk( pkE_FAIL );
        break;
    }

exit:
    if( fCallEndTransaction )
    {
        //
        // Make sure to call before Cleanup below to ensure _pIXDrm is still valid
        //
        _pIXDrm->EndTransaction();
    }

    if (pkFAILED(pkResult))
    {
        TRACE_ERROR(("CPlayReadyLicense::Init(%s) failed (pkResult = 0x%x)", sErr.empty() ? "" : sErr.c_str(), pkResult));
        Cleanup();
    }

    return (pkResult == pkS_OK);
}

pkRESULT CPlayReadyLicense::AcquireLicense(void *pvDecryptContext)
{
    pkRESULT pkResult;
    string sErr = "";
    CHECK_ALLOC(_pIXDrm);

    char* pszUrl           = NULL;
    char* pszChallenge     = NULL;
    char* pszResponse      = NULL;
    const char* pszCustomData = (0 == ChallengeCustomData.length()) ? NULL : ChallengeCustomData.c_str();
    const char* pszLicenseServerUri = (0 == LicenseServerUriOverride.length()) ? NULL : LicenseServerUriOverride.c_str();
    std::string licServerUri;

    _drmTimes[DrmLicenseTimes_Start] = Executive_GetTickCount();
    PRLICENSE_MSG(("CPlayReadyLicense::AcquireLicense Entry\n"));

    // Check if license is already available
    pkResult = _pIXDrm->CanDecrypt(pvDecryptContext, false);

    if (pkResult == pkS_OK)
    {
        PRLICENSE_MSG(("CPlayReadyLicense::AcquireLicense CanDecrypt=pkS_OK\r\n"));
        goto exit;
    }
    PRLICENSE_MSG(("CPlayReadyLicense::AcquireLicense pszCustomData=\r\n%s\n", pszCustomData));
    PRLICENSE_MSG(("CPlayReadyLicense::AcquireLicense pszLicenseServerUri=\r\n%s\n", pszLicenseServerUri));

    Chk2(_pIXDrm->GenerateChallenge( &pszUrl, pszCustomData, &pszChallenge ), "GenerateChallenge");
    if (NULL != pszLicenseServerUri)
    {
        licServerUri = pszLicenseServerUri;
    }
    else
    {
        licServerUri = pszUrl;
    }

    PRLICENSE_MSG(("CPlayReadyLicense::AcquireLicense licServerUri=\r\n%s\r\n pszChallenge=\r\n%s\n", licServerUri.c_str(), pszChallenge));

    _drmTimes[DrmLicenseTimes_GenerateChallenge] = Executive_GetTickCount();
    //Chk2(_pIXDrm->SendHttp( PLAYREADY_GETLICENSE_CONTENT_TYPE, pszChallenge, &pszResponse, "http://40.70.71.156/pr/svc/rightsmanager.asmx" ), "SendHttp(GETLICENSE)");
    Chk2(_pIXDrm->SendHttp( PLAYREADY_GETLICENSE_CONTENT_TYPE, pszChallenge, &pszResponse, licServerUri.c_str() ), "SendHttp(GETLICENSE)");
    _drmTimes[DrmLicenseTimes_GetLicense] = Executive_GetTickCount();
    PRLICENSE_MSG(("CPlayReadyLicense::AcquireLicense pszResponse=\r\n%s\n", pszResponse));
    Chk2(_pIXDrm->ProcessResponse( pszResponse ), "ProcessResponse");
    _drmTimes[DrmLicenseTimes_ProcessResponse] = Executive_GetTickCount();

    Chk2(_pIXDrm->CanDecrypt(pvDecryptContext, true), "CanDecrypt");

exit:
    _drmTimes[DrmLicenseTimes_End] = Executive_GetTickCount();
    if (pkFAILED(pkResult))
    {
        TRACE_ERROR(("CPlayReadyLicense::AcquireLicense(%s) failed (pkResult = 0x%x)", sErr.empty() ? "" : sErr.c_str(), pkResult));
    }

    if (pszUrl)
    {
        _pIXDrm->MemFree((void*)pszUrl);
    }
    if (pszChallenge)
    {
        _pIXDrm->MemFree((void*)pszChallenge);
    }
    if (pszResponse)
    {
        _pIXDrm->MemFree((void*)pszResponse);
    }
    PRLICENSE_MSG(("CPlayReadyLicense::AcquireLicense Exit\n"));
    return pkResult;
}

void CPlayReadyLicense::GetLicenseTimes( _Out_ DrmLicenseTiming* pTimesOut )
{
    if( pTimesOut != NULL )
    {
        memcpy_s(pTimesOut, sizeof(DrmLicenseTiming), _drmTimes, sizeof(DrmLicenseTiming));
    }
}

void CPlayReadyLicense::Dispose()
{
    delete this;
}

void CPlayReadyLicense::OPLCallback(XDRM_OPL_DATA* pOPLData)
{
    if (_pCallbackSink)
    {
        _pCallbackSink->OPLCallback(pOPLData);
    }
}

bool CPlayReadyLicense::IsDecrypterOnDemand()
{
    pkRESULT pkResult = pkS_OK;
    uint32_t dwValue = 0;
    uint32_t cbValue = sizeof(dwValue);

    CHECK_ALLOC(_pIXDrm);

    Chk(_pIXDrm->GetContentProperty(XDRM_PROP_DECRYPTORSETUP, (uint8_t *)&dwValue, &cbValue));

exit:
    return dwValue == 1; /* DRM_DECRYPTORSETUP_ONDEMAND */
}
