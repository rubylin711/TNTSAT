///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include <pkPAL.h>
#include <pkTestFramework.h>
#include <strsafe.h>
#include "PKTestSuite.h"

///////////////////////////////////////////////////////////////////////////////
//
// CPKTestName_Maker - auxiliary for executers
//
///////////////////////////////////////////////////////////////////////////////

class CPKTestName_Maker
{
public:

    CPKTestName_Maker()
        : m_iEndOfGroupName( 0 )
        , m_iEndOfMethodName( 0 )
        , m_iEndOfStr( 0 )
    {
        m_szStr[0] = L'\0';
    }

    void SetGroupName( _In_ const char* psz )
    {
        m_iEndOfGroupName = 0;
        m_iEndOfMethodName = 0;
        m_iEndOfStr = 0;

        AppendStr( psz );

        m_iEndOfGroupName = m_iEndOfStr;
        m_iEndOfMethodName = m_iEndOfStr;
    }

    void SetMethodName( _In_ const char* psz )
    {
        m_iEndOfMethodName = m_iEndOfGroupName;
        m_iEndOfStr = m_iEndOfGroupName;

        AppendStr( "::" );
        AppendStr( psz );

        m_iEndOfMethodName = m_iEndOfStr;
    }

    void SetInstanceID( _In_ const char* psz )
    {
        m_iEndOfStr = m_iEndOfMethodName;
        AppendStr( "#" );
        AppendStr( psz );
    }

    const char* PStr() const
    {
        return( m_szStr );
    }

private:

    char m_szStr[256];
    size_t m_iEndOfGroupName;
    size_t m_iEndOfMethodName;
    size_t m_iEndOfStr;

    void AppendStr( _In_ const char* psz )
    {
        const size_t MAX_SIZE = sizeof(m_szStr)/sizeof(m_szStr[0]);

        while( ( *psz != '\0' ) && ( m_iEndOfStr < (MAX_SIZE - 1)) )
        {
            m_szStr[m_iEndOfStr++] = *(psz++);
        }

        m_szStr[ m_iEndOfStr ] = '\0';
    }
};

///////////////////////////////////////////////////////////////////////////////
//
// Auxiliary
//
///////////////////////////////////////////////////////////////////////////////

const char* LookupTestMethodProperty(
    _In_ PKTEST_PROPERTY_LIST properties,
    _In_ const char* pszName )
{
    // Traverse the array looking for a name match
    // or the terminator entry.

    while( ( properties->pszName != NULL )
        && ( strcmp( properties->pszName, pszName ) != 0 ) )
    {
        ++properties;
    }

    return( ( properties->pszName != NULL ) ? properties->pszValue : NULL );
}

///////////////////////////////////////////////////////////////////////////////
//
// Executers
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
class CPKTestEvaluator_Filter
    : public IPKTestEvaluator
{
public:

    CPKTestEvaluator_Filter(
        _In_ IPKTestEvaluator* pRealExecuter,
        _In_ int argc,
        _In_ char* argv[],
        _In_opt_ const char* pszTestFilter,
        _In_opt_ const char* pszPropertyFilter)
        : m_pRealEvaluator( pRealExecuter )
        , m_argc( argc )
        , m_argv( argv )
        , m_pszTestFilter( pszTestFilter )
        , m_pszPropertyFilter( pszPropertyFilter )
    {
    }

    //
    // IPKTestEvaluator
    //

    virtual
    pkRESULT BeginTestGroup( _In_ const char* pszGroupName )
    {
        return( m_pRealEvaluator->BeginTestGroup( pszGroupName ) );
    }

    virtual
    bool WantsToRunTestMethod( _In_ const PKTEST_PROPERTY_LIST propertyList, _In_ const char* pszFullTestName )
    {
        m_fCurrentTestIsSelected = HasMatch(propertyList, pszFullTestName );

        if( m_fCurrentTestIsSelected )
        {
            m_fCurrentTestIsSelected = m_pRealEvaluator->WantsToRunTestMethod(propertyList, pszFullTestName );
        }

        return( m_fCurrentTestIsSelected );
    }

    virtual
    pkRESULT BeginTestMethod( _In_ const char* pszFullTestName, _In_opt_ const char* pszBugID )
    {
        return( m_pRealEvaluator->BeginTestMethod( pszFullTestName, pszBugID ) );
    }

    virtual
    void IncrementTestCount()
    {
        return( m_pRealEvaluator->IncrementTestCount() );
    }

    virtual
    void IgnoreTestMethod( _In_ const char* pszFullTestName, _In_opt_ const char* pszBugID )
    {
        return( m_pRealEvaluator->IgnoreTestMethod( pszFullTestName, pszBugID ) );
    }

    virtual
    void SetTestData( _In_opt_ const char* pszTestData )
    {
        return( m_pRealEvaluator->SetTestData( pszTestData ) );
    }

    virtual
    pkRESULT EndTestMethod()
    {
        return( m_pRealEvaluator->EndTestMethod() );
    }

    virtual
    pkRESULT EndTestGroup()
    {
        return( m_pRealEvaluator->EndTestGroup() );
    }

private:

    IPKTestEvaluator* m_pRealEvaluator;
    int m_argc;
    char** m_argv;
    bool m_fCurrentTestIsSelected;
    const char* m_pszTestFilter;
    const char* m_pszPropertyFilter;

    bool HasMatch( _In_ const PKTEST_PROPERTY_LIST propertyList, _In_ const char* pszFullTestName )
    {
        const char* priorityString = "Priority";
        const char delimiter = ':';
        bool fHasPropertyMatch = true;
        bool fHasNameMatch = false;

        for( int iArg = 1; iArg < m_argc; ++iArg )
        {
            if(NULL != m_pszTestFilter)
            {
                if( strcmp( m_argv[iArg], m_pszTestFilter) == 0)
                {
                    ++iArg;
                    if( ( iArg < m_argc ) && ( strstr( pszFullTestName, m_argv[iArg] ) != NULL ) )
                    {
                        fHasNameMatch = true;
                    }
                    continue;
                }
            }
            else
            {
                fHasNameMatch = true;
            }

            if(NULL != m_pszPropertyFilter)
            {
                if( strcmp( m_argv[iArg], m_pszPropertyFilter) == 0)
                {
                    ++iArg;
                    if (iArg >= m_argc)
                    {
                        return false;
                    }

                    char* pch = strchr(m_argv[iArg],delimiter);
                    if( NULL != pch)
                    {
                        int length = strlen(m_argv[iArg]);
                        int index = static_cast<int>(pch - m_argv[iArg]);
                        
                        if( length > index + 1 && index > 0)
                        {
                            char* name = NEW_NO_THROW char[index + 1];
                            char* value = NEW_NO_THROW char[length - (index)];

                            strncpy(name, m_argv[iArg], index );
                            name[index] = '\0';
                            strncpy(value, pch + 1,length - (index + 1));
                            value[length - (index + 1)] = '\0';

                            if(strcmp(name, priorityString) == 0)
                            {
                                int givenPriority;
                                if(StrToInt(value, givenPriority))
                                {
                                    int methodPriority;
                                    const char* priority = LookupTestMethodProperty( propertyList, priorityString );

                                    if(priority != NULL)
                                    {
                                        if( StrToInt(priority , methodPriority ) )
                                        {
                                            if(methodPriority > givenPriority)
                                            {
                                                fHasPropertyMatch =  false;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // If priority property is not present for a method, Don't run those methods when priority filter is specified.
                                        fHasPropertyMatch =  false;
                                    }
                                }
                            }
                            else
                            {
                                const char* propertyValue = LookupTestMethodProperty(propertyList, name);

                                if(( propertyValue == NULL) || (strcmp(propertyValue, value) != 0 ) )
                                {
                                    fHasPropertyMatch =  false;
                                }
                            }

                            delete[] name;
                            delete[] value;
                        }
                        else
                        {
                            fHasPropertyMatch = false;
                        }
                    }
                    else
                    {
                        fHasPropertyMatch = false;
                    }
                }
            }
        }
        return( fHasPropertyMatch && fHasNameMatch );
    }

    bool StrToInt(const char* value, _Out_ int& intValue)
    {
        char* stopPoint;
        intValue = strtol(value, &stopPoint, 0);
        return ( stopPoint != value);
    }

};

///////////////////////////////////////////////////////////////////////////////
class CPKTestEvaluator_RunTests
    : public IPKTestEvaluator
{
public:

    CPKTestEvaluator_RunTests()
        : m_cTestsRun( 0 )
        , m_cTestsOK( 0 )
        , m_cTestIgnored( 0 )
        , m_fLastTestFailed(false)
        , m_hLogFile( NULL )
        , m_pszFullTestName( NULL )
        , m_pszBugID( NULL )
        , m_pCurrentTestData( NULL )
        , m_fBreakOnFailure( false )
        , m_logLevel( LOG_LEVEL_COMMENT )
    {
    }

    ~CPKTestEvaluator_RunTests()
    {
        CloseLogFile();
    }

    pkRESULT CreateLogFile( _In_ const char* pszLogFileName )
    {
        pkRESULT pkr = pkS_OK;

        CloseLogFile();

        pkr = TF_Logging_Open( &m_hLogFile, pszLogFileName );
        if( FAILED(pkr) )
        {
            goto exit;
        }

        TF_Logging_Printf( m_hLogFile, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" );
        TF_Logging_Printf( m_hLogFile, "<PKTest-Logger>\n" );

    exit:

        return( pkr );
    }

    void SetLogLevel( _In_ LOG_LEVEL logLevel )
    {
        m_logLevel = logLevel;
    }

    void CloseLogFile()
    {
        if( m_hLogFile != NULL )
        {
            TF_Logging_Printf( m_hLogFile,
                "\t<Summary\n"
                "\t\tTotal=\"%d\"\n"
                "\t\tPassed=\"%d\"\n"
                "\t\tFailed=\"%d\"\n"
                "\t\tIgnored=\"%d\"\n"
                "\t\t/>\n"
                "</PKTest-Logger>",
                m_cTestsRun,
                m_cTestsOK,
                m_cTestsRun - ( m_cTestsOK + m_cTestIgnored ),
                m_cTestIgnored
                );

            TF_Logging_Close( m_hLogFile );
            m_hLogFile = NULL;
        }
    }

    int TotalTestRun() const { return( m_cTestsRun ); }
    int TotalTestOK() const  { return( m_cTestsOK ); }
    int TotalTestIgnored() const { return( m_cTestIgnored ); }

    void SetBreakOnFailureFlag( bool f ) { m_fBreakOnFailure = f; }

    bool CurrentTestHasFailed() const { return( m_fLastTestFailed ); }

    const char* GetTestData() const { return( m_pCurrentTestData ); }

    const char* GetTestFullName() const { return(m_pszFullTestName); }

private:

    int m_cTestsRun;
    int m_cTestsOK;
    int m_cTestIgnored;
    bool m_fLastTestFailed;
    pkHANDLE m_hLogFile;
    const char* m_pszFullTestName;
    const char* m_pszBugID;
    const char* m_pCurrentTestData;
    bool m_fBreakOnFailure;
    LOG_LEVEL m_logLevel;

public:

    //
    // IPKTestEvaluator
    //

    virtual
    pkRESULT BeginTestGroup( _In_ const char* pszGroupName )
    {
        TF_Printf( "%s\n", pszGroupName );
        
        if( m_hLogFile != NULL )
        {
            TF_Logging_Printf(
                            m_hLogFile,
                            "\t<TestGroup Title=\"%s\" >\n",
                            pszGroupName);
        }
        return( pkS_OK );
    }

    virtual
    bool WantsToRunTestMethod( _In_ const PKTEST_PROPERTY_LIST propertyList, _In_ const char* pszFullTestName )
    {
        return( true );
    }

    virtual
    pkRESULT BeginTestMethod( _In_ const char* pszFullTestName, _In_opt_ const char* pszBugID )
    {
        m_fLastTestFailed = false;
        m_pszBugID = pszBugID;
        m_pszFullTestName = pszFullTestName;

        if( pszBugID != NULL )
        {
            TF_Printf( "Start: %s   Bug: %s\n", pszFullTestName, pszBugID );
        }
        else
        {
            TF_Printf( "Start: %s\n", pszFullTestName );
        }

        if( m_hLogFile != NULL )
        {
            if( m_pszBugID != NULL )
            {
                TF_Logging_Printf(
                        m_hLogFile,
                        "\t\t<TestMethod Title=\"%s\" BugId=\"%s\" >\n",
                        pszFullTestName,
                        m_pszBugID );
            }
            else
            {
                TF_Logging_Printf(
                        m_hLogFile,
                        "\t\t<TestMethod Title=\"%s\" >\n",
                        pszFullTestName );
            }
        }

        return( pkS_OK );
    }

    virtual
    void IgnoreTestMethod( _In_ const char* pszFullTestName, _In_opt_ const char* pszBugID )
    {
        m_cTestIgnored++;

        if( pszBugID != NULL )
        {
            TF_Printf( "Ignore: %s, Bug: %s\n\n", pszFullTestName, pszBugID  );
        }
        else
        {
            TF_Printf( "Ignore: %s\n\n", pszFullTestName );
        }

        if( m_hLogFile != NULL )
        {
            if( pszBugID != NULL )
            {
                TF_Logging_Printf(
                        m_hLogFile,
                        "\t\t<TestMethod Title=\"%s\">\n\t\t\t<Result Title=\"%s\" BugId=\"%s\" >Ignored</Result>\n\t\t</TestMethod>\n",
                        pszFullTestName,
                        pszFullTestName,
                        pszBugID );
            }
            else
            {
                TF_Logging_Printf(
                        m_hLogFile,
                        "\t\t<TestMethod Title=\"%s\">\n\t\t\t<Result Title=\"%s\" >Ignored</Result>\n\t\t</TestMethod>\n",
                        pszFullTestName,
                        pszFullTestName );
            }
        }
    }

    virtual
    void SetTestData( _In_opt_ const char* pszTestData )
    {
        m_pCurrentTestData = pszTestData;
    }

    virtual
    void IncrementTestCount()
    {
        ++m_cTestsRun;
    }

    virtual
    pkRESULT EndTestMethod()
    {
        if( m_pszBugID != NULL )
        {
            m_fLastTestFailed = !m_fLastTestFailed;
        }

        const char* pszResultTag = m_fLastTestFailed ? "Failed" : "Passed";

        TF_Printf(
            "End: %s\n[%s]\n\n",
            m_pszFullTestName,
            pszResultTag );

        if(!m_fLastTestFailed)
        {
            ++m_cTestsOK;
        }

        if( m_hLogFile != NULL )
        {
            if( m_pszBugID != NULL )
            {
                TF_Logging_Printf(
                    m_hLogFile,
                    "\t\t\t<Result Title=\"%s\" BugId=\"%s\" >%s</Result>\n\t\t</TestMethod>\n",
                    m_pszFullTestName,
                    m_pszBugID,
                    pszResultTag );
            }
            else
            {
                TF_Logging_Printf(
                        m_hLogFile,
                        "\t\t\t<Result Title=\"%s\">%s</Result>\n\t\t</TestMethod>\n",
                        m_pszFullTestName,
                        pszResultTag );
            }
        }

        return( pkS_OK );
    }

    virtual
    pkRESULT EndTestGroup()
    {
        if( m_hLogFile != NULL )
        {
            TF_Logging_Printf(
                    m_hLogFile,
                    "\t</TestGroup>\n" );
        }
        return( pkS_OK );
    }

    //
    // Logger
    //

    void Log(
        _In_ LOG_LEVEL level,
        _In_ const char* pszFileName,
        _In_ int nLineNumber,
        _In_ const char* pszFunctionName,
        _In_ const char* pszFmt,
        _In_ va_list arglist )
    {
        pkRESULT pkr = pkS_OK;
        char szMsgBuffer[16*1024];

        if( level > m_logLevel )
        {
            goto exit;
        }

        //
        // Format log message
        //

        pkr = StringCchVPrintfA(
                    szMsgBuffer,
                    sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]),
                    pszFmt,
                    arglist );

        if( FAILED(pkr) )
        {
            pkr = StringCchPrintfA(
                        szMsgBuffer,
                        sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]),
                        "Error 0x%x trying to format log message",
                        pkr );

            pkASSERT( pkSUCCEEDED( pkr ) );
        }

        //
        // To the standard output (console)
        //

        if( level == LOG_LEVEL_ERROR )
        {
            TF_Printf(
                "\tError : [%s]@ln%d\n"
                "\tReason: ",
                pszFileName,
                nLineNumber,
                pszFunctionName );
        }
        else
        {
            TF_Printf( "\t" );
        }

        TF_Print( szMsgBuffer );

        //
        // To the log file
        //

        if( m_hLogFile != NULL )
        {
            pkr = EscapeXmlStringInPlace( szMsgBuffer, sizeof(szMsgBuffer) );
            if( FAILED(pkr) )
            {
                pkr = StringCchPrintfA(
                        szMsgBuffer,
                        sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]),
                        "Error 0x%x trying to escape XML character from error message",
                        pkr );

                pkASSERT( pkSUCCEEDED( pkr ) );
            }

            if( level == LOG_LEVEL_ERROR )
            {
                TF_Logging_Printf(
                        m_hLogFile,
                        "\t\t\t<Error File=\"%s\" Line=\"%d\" Function=\"%s\" Expression=\"%s\"/>\n",
                        pszFileName,
                        nLineNumber,
                        pszFunctionName,
                        szMsgBuffer );
            }
            else
            {
                TF_Logging_Printf(
                        m_hLogFile,
                        "\t\t\t<Msg UserText=\"%s\" Level=\"%d\"/>\n",
                        szMsgBuffer,
                        level );
            }
        }

    exit:

        if( level == LOG_LEVEL_ERROR )
        {
            m_fLastTestFailed = true;
        }

        pkASSERT( !m_fLastTestFailed || !m_fBreakOnFailure );

        return;
    }
};

///////////////////////////////////////////////////////////////////////////////
CPKTestEvaluator_RunTests& GlobalExecuterInstance()
{
    static CPKTestEvaluator_RunTests s_theExecuter;
    return( s_theExecuter );
}

///////////////////////////////////////////////////////////////////////////////
class CPKTestEvaluator_ListTests
    : public IPKTestEvaluator
{
public:

    virtual
    pkRESULT BeginTestGroup( _In_ const char* pszGroupName )
    {
        TF_Printf( "%s\n", pszGroupName );
        return( pkS_OK );
    }

    bool WantsToRunTestMethod( _In_ const PKTEST_PROPERTY_LIST propertyList, _In_ const char* pszFullTestName )
    {
        TF_Printf( "\t%s\n", pszFullTestName );

        return( false );
    }

    virtual
    pkRESULT BeginTestMethod( _In_ const char* pszFullTestName, _In_opt_ const char* pszBugID )
    {
        pkASSERT( 0 );
        return( pkE_UNEXPECTED );
    }

    virtual
    void IncrementTestCount()
    {
        pkASSERT( 0 );
    }

    virtual
    void IgnoreTestMethod( _In_ const char* pszFullTestName, _In_opt_ const char* pszBugID )
    {
        if( pszBugID != NULL )
        {
            TF_Printf( "\tIgnored with Bug:%s : %s\n", pszBugID, pszFullTestName );
        }
        else
        {
            TF_Printf( "\tIgnored : %s\n", pszFullTestName );
        }
    }

    virtual
    void SetTestData( _In_opt_ const char* pszTestData )
    {
    }

    virtual void NotifyTestFailure(
                    _In_ const char* pszFileName,
                    _In_ int nLineNumber,
                    _In_ const char* pszFunctionName,
                    _In_ const char* pszExpression )
    {
        pkASSERT( 0 );
    }

    virtual
    pkRESULT EndTestMethod()
    {
        pkASSERT( 0 );
        return( pkE_UNEXPECTED );
    }

    virtual
    pkRESULT EndTestGroup()
    {
        TF_Printf( "\n" );
        return( pkS_OK );
    }
};


///////////////////////////////////////////////////////////////////////////////
//
// Suite Functions
//
///////////////////////////////////////////////////////////////////////////////

static CPKTestList<CPKTestGroupTraits>& GroupsRegistry()
{
    static CPKTestList<CPKTestGroupTraits> s_theInstance;
    return( s_theInstance );
}

static inline void _PKTestSuite_RegisterTraits( _In_ CPKTestGroupTraits* pTraits )
{
    GroupsRegistry().Add( pTraits );
}

static pkRESULT _PKTestSuite_DirectExecution( _In_ IPKTestEvaluator* pEvaluator )
{
    pkRESULT hr = pkS_OK;

    for(
        CPKTestGroupTraits* pTraits = GroupsRegistry().First();
        pTraits != NULL;
        pTraits = GroupsRegistry().NextOf( pTraits ) )
    {
        hr = pEvaluator->BeginTestGroup( pTraits->GetName() );
        if( FAILED(hr) )
        {
            goto exit;
        }

        hr = pTraits->EvaluateTests( pEvaluator );
        if( FAILED(hr) )
        {
            goto exit;
        }

        hr = pEvaluator->EndTestGroup();
        if( FAILED(hr) )
        {
            goto exit;
        }
    }

exit:

    return( hr );
}

///////////////////////////////////////////////////////////////////////////////
pkRESULT PKTestSuite_ListTests(
    _In_ int argc,
    _In_ char* argv[],
    _In_opt_ const char* pszTestFilter,
    _In_opt_ const char* pszPropertyFilter)
{
    pkRESULT hr = pkS_OK;

    CPKTestEvaluator_ListTests listEvaluator;

    if( NULL == pszTestFilter && NULL == pszPropertyFilter )
    {
        hr = _PKTestSuite_DirectExecution( &listEvaluator );
        if( FAILED(hr) )
        {
            goto exit;
        }
    }
    else
    {
        CPKTestEvaluator_Filter filterExecuter( &listEvaluator, argc, argv, pszTestFilter, pszPropertyFilter );

        hr = _PKTestSuite_DirectExecution( &filterExecuter );
        if( FAILED(hr) )
        {
            goto exit;
        }
    }

exit:

    return( hr );
}

///////////////////////////////////////////////////////////////////////////////
pkRESULT PKTestSuite_RunTests(
    _In_ int argc,
    _In_ char* argv[],
    _In_ bool fBreakOnFailure,
    _In_opt_ const char* pszTestFilter,
    _In_opt_ const char* pszPropertyFilter)
{
    pkRESULT hr = pkS_OK;

    GlobalExecuterInstance().SetBreakOnFailureFlag( fBreakOnFailure );

    if( NULL == pszTestFilter && NULL == pszPropertyFilter )
    {
        hr = _PKTestSuite_DirectExecution( &GlobalExecuterInstance() );
        if( FAILED(hr) )
        {
            goto exit;
        }
    }
    else
    {
        CPKTestEvaluator_Filter filterExecuter( &GlobalExecuterInstance(), argc, argv, pszTestFilter, pszPropertyFilter );

        hr = _PKTestSuite_DirectExecution( &filterExecuter );
        if( FAILED(hr) )
        {
            goto exit;
        }
    }

    TF_Printf( "-------------------------------------------------------------------------------\n" );
    TF_Printf( "Summary:\n" );
    TF_Printf( "    Selected  = %d\n", GlobalExecuterInstance().TotalTestRun() );
    TF_Printf( "    Succeeded = %d\n", GlobalExecuterInstance().TotalTestOK() );
    TF_Printf( "    Failed    = %d\n", GlobalExecuterInstance().TotalTestRun() - ( GlobalExecuterInstance().TotalTestOK() + GlobalExecuterInstance().TotalTestIgnored() ) );
    TF_Printf( "    Ignored   = %d\n", GlobalExecuterInstance().TotalTestIgnored() );

exit:

    return( hr );
}

///////////////////////////////////////////////////////////////////////////////
pkRESULT PKTestSuite_AbortTests()
{
    return( S_OK );
}

///////////////////////////////////////////////////////////////////////////////
pkRESULT PKTest_OpenGlobalLogFile(
    _In_ const char* pszFileName )
{
    return( GlobalExecuterInstance().CreateLogFile( pszFileName ) );
}

void PKTest_SetLogLevel(
    _In_ LOG_LEVEL logLevel )
{
    return( GlobalExecuterInstance().SetLogLevel( logLevel ) );
}


///////////////////////////////////////////////////////////////////////////////
//
// CPKTestGroup
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
pkRESULT CPKTestGroup::EvaluateTests(
    _In_ const char* pszGroupName,
    _In_ IPKTestEvaluator* pEvaluator )
{
    pkRESULT hr = pkS_OK;

    CPKTestName_Maker fullTestName;

    fullTestName.SetGroupName( pszGroupName );

    //
    // For each test method in this group
    //

    for(
        CPKTestMethod* pMethod = m_TestMethods.First();
        ( pMethod != NULL );
        pMethod = m_TestMethods.NextOf( pMethod )
        )
    {
        PKTEST_PROPERTY_LIST properties = pMethod->GetProperties();
        bool fIsDataDriven = false;
        bool fIgnoreTest = false;
        const char* pszBugID = NULL;

        fullTestName.SetMethodName( pMethod->GetName() );

        //
        // Check if the test is to be ignored
        //

        const char* pszValueOfIgnore = LookupTestMethodProperty( properties, "Ignore" );

        if( pszValueOfIgnore != NULL )
        {
            if( strcmp( pszValueOfIgnore, "true" ) == 0 )
            {
                fIgnoreTest = true;
            }
            else if( strcmp( pszValueOfIgnore, "false" ) == 0 )
            {
                fIgnoreTest = false;
            }
            else
            {
                fIgnoreTest = false;
                LogTestError( "Unexpected value for 'Ignore' property '%s'", pszValueOfIgnore );
            }
        }
        
        pszBugID = LookupTestMethodProperty( pMethod->GetProperties(), "Bug" );

        //
        // Execute the test method.
        // If the test is data driven, then there's one instance of the test for each data entry.
        // If the test is NOT data driven, then there's only one instance of the test.
        //

        do
        {
            //
            // Look for the first or next data driven entry in the test method properties.
            // A test method is data driven if it has a property whose name starts with
            // "Data:", then the rest of the property name is the test's instance ID
            // (for logs and filtering), and the property value is the test's instance param
            //

            const char* pszDataDrivenValue = NULL;

            while( properties->pszName != NULL )
            {
                if( strncmp( properties->pszName, "Data:", 5 ) == 0 )
                {
                    // Found one, the test is data driven!
                    // Use the entry as this instance param

                    fullTestName.SetInstanceID( properties->pszName + 5 );

                    pszDataDrivenValue = properties->pszValue;

                    fIsDataDriven = true;

                    ++properties;

                    break;
                }

                ++properties;
            }

            if( fIsDataDriven && ( pszDataDrivenValue == NULL ) )
            {
                // No more entries for data driven test.
                // Get out!
                break;
            }

            //
            // Evaluate the current test instance
            //

            pEvaluator->SetTestData( pszDataDrivenValue );

            if( !pEvaluator->WantsToRunTestMethod(pMethod->GetProperties(), fullTestName.PStr() ) )
            {
                continue;
            }

            pEvaluator->IncrementTestCount();

            if( fIgnoreTest )
            {
                pEvaluator->IgnoreTestMethod( fullTestName.PStr(), pszBugID );
                continue;
            }

            hr = pEvaluator->BeginTestMethod( fullTestName.PStr(), pszBugID );
            if( FAILED(hr) )
            {
                goto exit;
            }

            if( TestSetup() )
            {
                pMethod->Run();
                if(!TestCleanup())
                {
                    LogTestError("TestCleanup() Failed");
                }
            }
            else
            {
                LogTestError("TestSetup() Failed");
            }

            hr = pEvaluator->EndTestMethod();
            if( FAILED(hr) )
            {
                goto exit;
            }
        }
        while( fIsDataDriven );
    }

exit:

    return( hr );
}

///////////////////////////////////////////////////////////////////////////////
bool CPKTestGroup::CurrentTestHasFailed()
{
    return( GlobalExecuterInstance().CurrentTestHasFailed() );
}

///////////////////////////////////////////////////////////////////////////////
//
// CPKTestGroupTraits
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
CPKTestGroupTraits::CPKTestGroupTraits( const char* pszClassName )
    : m_pszClassName( pszClassName )
{
    _PKTestSuite_RegisterTraits( this );
}

///////////////////////////////////////////////////////////////////////////////
//
// CPKTestGroupTraits
//
///////////////////////////////////////////////////////////////////////////////

const char* PKTest_GetTestData()
{
    return( GlobalExecuterInstance().GetTestData() );
}

const char* PKTest_GetTestFullName()
{
    return( GlobalExecuterInstance().GetTestFullName() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Logger class and utilities
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
void CPKTestLogger::Log(
        _In_ const char* pszFmt,
        va_list arglist )
{
    GlobalExecuterInstance()
        .Log(
            m_level,
            m_pszFileName,
            m_nLineNumber,
            m_pszFunctionName,
            pszFmt,
            arglist );
}

///////////////////////////////////////////////////////////////////////////////
pkRESULT EscapeXmlStringInPlace(
        _Inout_count_(cchBuffer) char* pszBuffer,
        _In_ size_t cchBuffer )
{
    pkRESULT pkr = pkS_OK;

    char* pchFirst = pszBuffer;
    char* pchLast = pchFirst + cchBuffer;
    char* pchTerminator = pchFirst;

    while( ( pchTerminator < pchLast )
            && ( *pchTerminator != '\0' ) )
    {
        ++pchTerminator;
    }

    if( pchLast <= pchTerminator )
    {
        // Could not find the null terminator
        // within the given buffer

        pkr = pkE_INVALIDARG;
        goto exit;
    }

    while( pchFirst < pchTerminator )
    {
        if( *pchFirst != '\"'
            && *pchFirst != '<'
            && *pchFirst != '>'
            && *pchFirst != '&' )
        {
            // Skip characters that don't have
            // escape sequences

            ++pchFirst;
            continue;
        }

        const char* pchEscape = NULL;
        size_t cchEscape = 0;

        switch( *pchFirst )
        {
        case '\"':
            pchEscape = "&quot;";
            cchEscape = sizeof("&quot;") - sizeof('\0');
            break;

        case '<':
            pchEscape = "&lt;";
            cchEscape = sizeof("&lt;") - sizeof('\0');
            break;

        case '>':
            pchEscape = "&gt;";
            cchEscape = sizeof("&gt;") - sizeof('\0');
            break;

        case '&':
            pchEscape = "&amp;";
            cchEscape = sizeof("&amp;") - sizeof('\0');
            break;
        }

        pkASSERT( pchEscape != NULL );

        pchTerminator -= 1;         // subtract the character that will be escaped
        pchTerminator += cchEscape; // add the escape sequence that goes in its place

        if( pchLast <= pchTerminator )
        {
            // No room in the buffer to expand the
            // escape sequence

            pkr = pkE_INVALIDARG;
            goto exit;
        }

        // Insert the escape sequence at the character position
        // and advance to the first character after that

        memmove( pchFirst + cchEscape, pchFirst + 1, pchTerminator - ( pchFirst + 1 ) );
        memcpy( pchFirst, pchEscape, cchEscape );

        pchFirst += cchEscape;
    }

exit:

    if( pkFAILED( pkr )
        && ( cchBuffer > 0 ) )
    {
        // Reset the buffer on failure
        pszBuffer[0] = '\0';
    }

    return( pkr );
}

