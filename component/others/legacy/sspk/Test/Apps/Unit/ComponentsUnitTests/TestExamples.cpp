///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <SSPKDefines.h>
#include <PKTestSuite.h>

#if 0

////////////////////////////////////////////////////////////////////////////////
// Example of a Test Group
//
PKTEST_GROUP( TestExamples )
{
    ////////////////////////////////////////
    // Example of Test Methods

    PKTEST_METHOD( MethodA );
    PKTEST_METHOD( MethodB );

    ////////////////////////////////////////
    // Example of a inlined Test Method

    PKTEST_METHOD( MethodC )
    {
        PKTEST_ASSERT_EXIT( true );
    exit:
        return;
    }

    ////////////////////////////////////////
    // Examples of tests method with properties

    PKTEST_METHOD_EX(
            MethodD,
            PKTEST_PROPERTY( "Ignore", "true" )
            )
    {
        return;
    }

    PKTEST_METHOD_EX(
            MethodE,
            PKTEST_PROPERTY( "Ignore", "true" )
            PKTEST_PROPERTY( "Bug", "12345" )
            PKTEST_PROPERTY( "Owner", "John" )
            PKTEST_PROPERTY( "Priority", "0" )
            )
    {
        return;
    }

    ////////////////////////////////////////
    // Example of test method with data
    // driven property

    PKTEST_METHOD_EX(
            MethodF,
            PKTEST_PROPERTY( "Data:0", "X" )
            PKTEST_PROPERTY( "Data:1", "Y" )
            PKTEST_PROPERTY( "Data:2", "Z" )
            PKTEST_PROPERTY( "Data:3", "http://www.example.com/Manifest" )
            )
    {
        LogTestComment( "TestData: %s", PKTest_GetTestData() );
        return;
    }

};

////////////////////////////////////////////////////////////////////////////////
void TestExamples::MethodA()
{
    PKTEST_ASSERT_EXIT( true );

    LogTestWarning( "Log example %d", 0 );
    LogTestComment( "Log example %d", 0 );

    LogTestError( "Log example %d", 0 );    // marks the test as failed

exit:

    return;
}

////////////////////////////////////////////////////////////////////////////////
void GlobalFunction( int x )
{
    PKTEST_ASSERT_EXIT( x > 0 );

exit:

    return;
}

////////////////////////////////////////////////////////////////////////////////
void TestExamples::MethodB()
{
    // This test will fail
    PKTEST_FUNC_EXIT( GlobalFunction( -1 ) );

    pkASSERT( FALSE );  // this should never execute if the previous fails

exit:

    return;
}

#endif
