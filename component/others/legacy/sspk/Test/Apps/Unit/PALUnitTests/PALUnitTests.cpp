///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// ComponentsUnitTests.cpp : Defines the entry point for the test application.
//

#include "stdafx.h" // includes pkPAL.h

#include <pkTestFramework.h>
#include <TLCommon.h>


////////////////////////////////////////////////////////////////////////////////
//
// Parameters parsed by Test_GetConfig and used by Test_Run
//
////////////////////////////////////////////////////////////////////////////////

static int g_argc = 0;
static char** g_argv = NULL;
static bool g_fListTests = false;
static bool g_fHasTestFilter = false;
static bool g_fHasPropertyFilter = false;
static bool g_fBreakOnFailure = false;

////////////////////////////////////////////////////////////////////////////////
//
// Called by the Test Framework to parse the input arguments
//
////////////////////////////////////////////////////////////////////////////////

pkRESULT Test_GetConfig(
    _In_ int argc,
    _In_ char *argv[],
    _Out_ TF_Config *pConfig )
{
    pkRESULT hr = pkS_OK;

    g_argc = argc;
    g_argv = argv;

    bool_t fVerboseHelpIsRequested = false;
    bool_t fShowUsage = false;
    const char* pszGlobalTestDataSourceFile = NULL;
    const char* pszLogFile = NULL;
    int nLogLevel = 0;

    TLC_SetCmdLineArgs( argc, argv );

    //
    // TLC_IsHelpRequested doesn't know that no arguments means run all the tests
    //

    if( ( argc > 1 )
        && TLC_IsHelpRequested( &fVerboseHelpIsRequested ) )
    {
        fShowUsage = true;
        goto exit;
    }

    //
    // Parse the arguments
    //

    for( int iArg = 1; iArg < argc; ++iArg )
    {
        const char* pszCmd = argv[iArg];

        if( strcmp( pszCmd, "-list" ) == 0 )
        {
            g_fListTests = true;
        }
        else if( strcmp( pszCmd, "-test" ) == 0 )
        {
            g_fHasTestFilter = true;

            ++iArg;

            if( iArg >= argc )
            {
                TF_Printf( "Expecting test name after -test\n\n" );
                fShowUsage = true;
                break;
            }
        }
        else if( strcmp( pszCmd, "-property" ) == 0 )
        {
            g_fHasPropertyFilter = true;

            iArg ++;

            if( iArg >= argc )
            {
                TF_Printf( "Expecting a value for the property\n\n" );
                fShowUsage = true;
                break;
            }
        }
        else if( strcmp( pszCmd, "-tds" ) == 0 )
        {
            ++iArg;

            if( iArg >= argc )
            {
                TF_Printf( "Expecting name of test data source file after -tds\n\n" );
                fShowUsage = true;
                break;
            }

            pszGlobalTestDataSourceFile = argv[iArg];
        }
        else if( strcmp( pszCmd, "-log" ) == 0 )
        {
            ++iArg;

            if( iArg >= argc )
            {
                TF_Printf( "Expecting name of log file after -log\n\n" );
                fShowUsage = true;
                break;
            }

            pszLogFile = argv[iArg];
        }
        else if( strcmp( pszCmd, "-logLevel" ) == 0 )
        {
            ++iArg;

            if( iArg >= argc )
            {
                TF_Printf( "Expecting log level after -logLevel\n\n" );
                fShowUsage = true;
                break;
            }

            nLogLevel = atoi( argv[iArg] );

            if( !isdigit( argv[iArg][0] )
                || ( nLogLevel < LOG_LEVEL_ERROR )
                || ( LOG_LEVEL_COMMENT < nLogLevel ) )
            {
                TF_Printf( "Invalid log level after -logLevel. Expecing a value between 0 and 2.\n\n" );
                fShowUsage = true;
                break;
            }

            PKTest_SetLogLevel( (LOG_LEVEL)nLogLevel );
        }
        else if( strcmp( pszCmd, "-breakOnFail" ) == 0 )
        {
            g_fBreakOnFailure = true;
        }
        else
        {
            TF_Printf( "Invalid argument: %s.\n\n", pszCmd );
            fShowUsage = true;
            break;
        }
    }

    if( g_fListTests )
    {
        //
        // Data source and log are ignored when listing the tests
        //

        pszGlobalTestDataSourceFile = NULL;
        pszLogFile = NULL;
    }

    if( pszGlobalTestDataSourceFile != NULL )
    {
        hr = PKTest_SetGlobalTestDataSourceFile( pszGlobalTestDataSourceFile );
        if( FAILED(hr) )
        {
            TF_Printf( "Error 0x%x: failed to load data source file [%s]\n", pszGlobalTestDataSourceFile );
            goto exit;
        }
    }

    if( pszLogFile != NULL )
    {
        hr = PKTest_OpenGlobalLogFile( pszLogFile );
        if( FAILED(hr) )
        {
            TF_Printf( "Error 0x%x: failed to open log file [%s]\n", pszLogFile );
            goto exit;
        }
    }

exit:

    if( fShowUsage )
    {
                //|--------------------------------------------------------------------------------|
        TF_Printf("Usage: ComponentsUnitTests <options>\n\n" );
        TF_Printf("-list          Lists the tests in the module hierarchically.\n" );
        TF_Printf("               If this option is present, the tests will not run.\n\n" );
        TF_Printf("-test <spec>   Selects tests that contain <spec> in their names.\n" );
        TF_Printf("               This option can be specified multiple times to select multiple\n" );
        TF_Printf("               tests. If absent, all tests are selected.\n\n" );
        TF_Printf("-tds <file>    The name of the test data source file to use.\n" );
        TF_Printf("-breakOnFail   Break in the debugger on a test failure.\n" );
        TF_Printf("-log <file>    Optional file to log the tests results.\n" );
        TF_Printf("-logLevel <n>  Verbosity of the log, from 0 (least) to 2 (most).\n" );
        TF_Printf("-property <PropertyName>:<PropertyValue> Selects tests that has the given property name and value.");
        TF_Printf("               This option can be specified multiple times to select multiple\n" );

    }

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
//
// Entry point called by the Test Framework to execute the tests
//
////////////////////////////////////////////////////////////////////////////////

pkRESULT Test_Run(void)
{
    if( g_fListTests )
    {
        return( PKTestSuite_ListTests( g_argc, g_argv, g_fHasTestFilter ? "-test" : NULL, g_fHasPropertyFilter ? "-property" : NULL ) );
    }
    else
    {
        return( PKTestSuite_RunTests( g_argc, g_argv, g_fBreakOnFailure, g_fHasTestFilter ? "-test" : NULL, g_fHasPropertyFilter ? "-property" : NULL ) );
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Called by the Test Framework to cancel all tests
//
////////////////////////////////////////////////////////////////////////////////

pkRESULT Test_Abandon(void)
{
    return( PKTestSuite_AbortTests() );
}
