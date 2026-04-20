///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSPKDefines.h"

#include "CManifestTrack.h"             // Components/MBR
#include "CManifestChunk.h"             // Components/MBR
#include "ManifestParser.h"             // Components/MBR
#include "StringUtils.h"
#include <string>
#include <algorithm>
#include <direct.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( ManifestFuzzTest )
{
    CXMLElementsList _xManifests;
    ManifestFuzzTest()
    {
    }

    bool TestSetup()
    {
        CXMLElement xTestData = PKTest_GetDataTable( "FuzzFiles" );
        PKTEST_ASSERT_EXIT( !xTestData.IsNull() );

        _xManifests = xTestData.Elements(L"file");
        PKTEST_ASSERT_MSG_EXIT(_xManifests.Length() > 0,"Couldn't find any elements with name 'file'");

        return true;

    exit:
        return false;
    }

    PKTEST_METHOD(ManifestParserFuzzTest)
    {
        for(int i = 0; i < _xManifests.Length(); i++)
        {
            AutoRefPtr<MBR::CChunkManifest>  currentManifest;
            MBR::CChunkManifest::CreateInstance( currentManifest.DerefOutPtr() );
            MBR::CManifestParser parser( currentManifest );
            CXMLAttributesList currentElementAttributes = _xManifests[i].Attributes();
            std::string strSrc = escapeSpaces( wstring_to_string( currentElementAttributes[L"path"].Value() ) );

            LogTestComment("current Xml file parsing is: %s", strSrc.c_str());

            CTextFileReader reader;
            PKTEST_HRESULT_EXIT( reader.OpenExisting( strSrc.c_str() ) );
            

            HRESULT parserResult = parser.Parse( reader.AsHandle(), reader.ReadCallback );
            if(!parserResult)
            {
                LogTestComment("              Manifest Parser Failed");
            }

            HRESULT validationResult = (currentManifest)->ValidateManifest();
            if(!validationResult)
            {
                LogTestComment("              Invalid Manifest");
            }

            reader.Close();
        }

    exit:
        
        return;
    }
};
