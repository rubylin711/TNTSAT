///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////////

#include <strsafe.h>    // for va_list

#pragma once

#if 0   // The following section is sample code

    //
    // Example of writing a test using the elements defined in this header
    //

    PKTEST_GROUP(BasicTestGroup)
    {
        PKTEST_METHOD(TestCase)
        {
            PKTEST_ASSERT_EXIT( true );
            PKTEST_HRESULT_EXIT( S_OK );

        exit:
            return;
        }

        PKTEST_METHOD_EX(
            TestWithProperties,
                PKTEST_PROPERTY( "Ignore", "false" )
                PKTEST_PROPERTY( "Bug", "1234" )
                )
        {
        }
    };

    //
    // Test properties supported by the Test Suite:
    //
    //  "Ignore"
    //      Accepted values:
    //          "true" - the test will be marked as ignored and will not run
    //          "false" (default) - the test will be executed
    //  "Bug"
    //      The bug # associated with the test
    //  "Data:<x>"
    //      Defines a data driven test. "<x>" is an identified chosen by the
    //      test author. The suite will run the test method once for each
    //      property in this format. The test method can read the data
    //      of this property with the PKTest_GetTestData function.
    //

#endif

////////////////////////////////////////////////////////////////////////////////
//
// Macros to define test classes and test methods
//
////////////////////////////////////////////////////////////////////////////////

#define PKTEST_GROUP( Name ) \
    struct Name; \
    static const char g_PKTestGroupTraitsName_##Name[] = PKTEST_MAKE_STRA( #Name ); \
    CPKTestGroupTraitsImpl<Name> g_PKTestGroupTraits_##Name( g_PKTestGroupTraitsName_##Name ); \
    struct Name : public CPKTestGroupImpl<Name>

#define PKTEST_METHOD_EX( Name, Properties ) \
    PKTEST_MAKE_METHOD_START( Name ) \
        Properties \
    PKTEST_MAKE_METHOD_END( Name )

#define PKTEST_METHOD( Name ) \
    PKTEST_MAKE_METHOD_START( Name ) \
    PKTEST_MAKE_METHOD_END( Name )

#define PKTEST_PROPERTY( Name, Value )  { Name, Value },

////////////////////////////////////////////////////////////////////////////////
//
// Macros to use during test execution
//
////////////////////////////////////////////////////////////////////////////////
#define PKTEST_ASSERT_MSG_EXIT( e, format, ... ) \
        do \
        { \
            if( !static_cast<bool>(e) ) \
            { \
                LogTestError( format " at  Assert: %s", ## __VA_ARGS__ , PKTEST_MAKE_STRA( #e ) ); \
                goto exit; \
            } \
        }while(0)

#define PKTEST_HRESULT_MSG_EXIT( e, format, ... ) \
        do{ \
            pkRESULT _pkrCallResult = ( e ); \
            if( pkFAILED(_pkrCallResult) ) \
            { \
                LogTestError( format " Failed with( 0x%x ): %s", ## __VA_ARGS__ , _pkrCallResult, PKTEST_MAKE_STRA( #e ) ); \
                goto exit; \
            }\
        }while(0)

#define PKTEST_MSG( format, ... ) \
        do \
        { \
            LogTestComment( format, ## __VA_ARGS__ ); \
        }while(0)


#define PKTEST_ASSERT_EXIT( e ) \
        if( !static_cast<bool>(e) ) \
        { \
            LogTestError( "Assert: %s", PKTEST_MAKE_STRA( #e ) ); \
            goto exit; \
        }

#define PKTEST_HRESULT_EXIT( e ) \
        { \
            pkRESULT _pkrCallResult = ( e ); \
            if( pkFAILED(_pkrCallResult) ) \
            { \
                LogTestError( "Failed( 0x%x ): %s", \
                    _pkrCallResult, PKTEST_MAKE_STRA( #e ) ); \
                goto exit; \
            }\
        }

#define PKTEST_FUNC_EXIT( e ) \
        e; \
        if( CPKTestGroup::CurrentTestHasFailed() ) \
        { \
            LogTestError( "Failure relay" ); \
            goto exit; \
        }

#define PKTEST_CHECK_FAIL_EXIT( ) \
        if( CPKTestGroup::CurrentTestHasFailed() ) \
        { \
            LogTestError( "Failure relay" ); \
            goto exit; \
        }

////////////////////////////////////////////////////////////////////////////////
//
// Test Functions
//
////////////////////////////////////////////////////////////////////////////////

//
// Returns a string containing the data associated with this data driven
// test or NULL if this is not a data driven test
//
const char* PKTest_GetTestData();

const char* PKTest_GetTestFullName();

////////////////////////////////////////////////////////////////////////////////
//
// Log macros
//
////////////////////////////////////////////////////////////////////////////////

enum LOG_LEVEL
{
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_COMMENT,
};

#define LogTestError    ( CPKTestLogger( LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__ ) )

#define LogTestWarning  ( CPKTestLogger( LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__ ) )

#define LogTestComment  ( CPKTestLogger( LOG_LEVEL_COMMENT, __FILE__, __LINE__, __FUNCTION__ ) )


////////////////////////////////////////////////////////////////////////////////
//
// Suite Functions
//
////////////////////////////////////////////////////////////////////////////////

pkRESULT PKTestSuite_ListTests(
    _In_ int argc,
    _In_ char* argv[],
    _In_opt_ const char* pszTestFilter,
    _In_opt_ const char* pszPropertyFilter);

pkRESULT PKTestSuite_RunTests(
    _In_ int argc,
    _In_ char* argv[],
    _In_ bool fBreakOnFailure,
    _In_opt_ const char* pszTestFilter,
    _In_opt_ const char* pszPropertyFilter);

pkRESULT PKTestSuite_AbortTests();

pkRESULT PKTest_OpenGlobalLogFile(
    _In_ const char* pszFileName );

void PKTest_SetLogLevel(
    _In_ LOG_LEVEL logLevel );

////////////////////////////////////////////////////////////////////////////////
//
// Simple List classes to keep things in a list
//
////////////////////////////////////////////////////////////////////////////////

template< class SELF > class CPKTestListToken;
template< class T > class CPKTestList;

///////////////////////////////////////////////////////////////////////////////
template< class SELF >
class CPKTestListToken
{
public:

    CPKTestListToken()
        : m_pPrev( NULL )
        , m_pNext( NULL )
    {
    }

private:

    friend class CPKTestList<SELF>;

    CPKTestListToken* m_pPrev;
    CPKTestListToken* m_pNext;
};

///////////////////////////////////////////////////////////////////////////////
template< class T >
class CPKTestList
{
public:

    CPKTestList()
        : m_pHead( NULL )
        , m_pTail( NULL )
    {
    }

    void Add( _In_ T* pElem )
    {
        // ASSERT( pElem->m_pPrev = NULL );
        // ASSERT( pElem->m_pNext = NULL );

        if( m_pTail == NULL )
        {
            pElem->m_pNext = NULL;
            pElem->m_pPrev = NULL;
            m_pHead = pElem;
            m_pTail = pElem;
        }
        else
        {
            pElem->m_pNext = NULL;
            pElem->m_pPrev = m_pTail;
            m_pTail->m_pNext = pElem;
            m_pTail = pElem;
        }
    }

    T* First() const
    {
        return( static_cast<T*>( m_pHead ) );
    }

    T* NextOf( _In_opt_ T* pElement ) const
    {
        return( ( pElement != NULL ) ? static_cast<T*>( pElement->m_pNext ) : NULL );
    }

private:

    CPKTestListToken<T>* m_pHead;
    CPKTestListToken<T>* m_pTail;
};


////////////////////////////////////////////////////////////////////////////////
//
// Classes and interfaces definitions
//
////////////////////////////////////////////////////////////////////////////////

class CPKTestGroupTraits;
class CPKTestGroup;
class CPKTestMethod;

struct PKTEST_PROPERTY_ENTRY
{
    const char* pszName;
    const char* pszValue;
};

// This is an array of PKTEST_PROPERTY_ENTRY terminated by
// a null entry (i.e., { NULL, NULL } )
typedef const PKTEST_PROPERTY_ENTRY* PKTEST_PROPERTY_LIST;

struct IPKTestEvaluator
{
    virtual pkRESULT BeginTestGroup( _In_ const char* pszGroupName ) = 0;

    virtual bool WantsToRunTestMethod(
                        _In_ const PKTEST_PROPERTY_LIST propertyList,
                        _In_ const char* pszFullTestName ) = 0;

    virtual pkRESULT BeginTestMethod(
                        _In_ const char* pszFullTestName,
                        _In_opt_ const char* pszBugID ) = 0;

    virtual void IgnoreTestMethod(
                        _In_ const char* pszFullTestName,
                        _In_opt_ const char* pszBugID ) = 0;

    virtual void SetTestData( _In_opt_ const char* pszTestData ) = 0;

    virtual void IncrementTestCount() = 0;

    virtual pkRESULT EndTestMethod() = 0;

    virtual pkRESULT EndTestGroup() = 0;
};

////////////////////////////////////////////////////////////////////////////////
//
// Classes
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
class CPKTestMethod
    : public CPKTestListToken<CPKTestMethod>
{
public:

    virtual const char* GetName() const = 0;

    virtual PKTEST_PROPERTY_LIST GetProperties() const = 0;

    virtual void Run() = 0;
};

///////////////////////////////////////////////////////////////////////////////
class CPKTestGroup
    : public CPKTestListToken<CPKTestGroup>
{
protected:

    CPKTestGroup()
    {
    }

    virtual ~CPKTestGroup()
    {
    }

    CPKTestList<CPKTestMethod> m_TestMethods;

    virtual bool TestSetup()      { return( true ); };
    virtual bool TestCleanup()    { return( true ); };

    static bool CurrentTestHasFailed();

public:

    pkRESULT EvaluateTests(
                _In_ const char* pszGroupName,
                _In_ IPKTestEvaluator* pEvaluator );
};

///////////////////////////////////////////////////////////////////////////////
template< class SELF >
class CPKTestGroupImpl
    : public CPKTestGroup
{
public:

    typedef SELF ThisTestGroup;
};

///////////////////////////////////////////////////////////////////////////////
//
// Classes used to register test classes in the module
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
class CPKTestGroupTraits
    : public CPKTestListToken<CPKTestGroupTraits>
{
public:

    CPKTestGroupTraits( const char* pszClassName );

    const char* GetName() const
    {
        return( m_pszClassName );
    }

    virtual pkRESULT EvaluateTests( _In_ IPKTestEvaluator* pEvaluator ) = 0;

private:

    const char* const m_pszClassName;
};

///////////////////////////////////////////////////////////////////////////////
template< class T >
class CPKTestGroupTraitsImpl
    : public CPKTestGroupTraits
{
public:

    CPKTestGroupTraitsImpl( const char* pszClassName )
        : CPKTestGroupTraits( pszClassName )
    {
    }

    virtual pkRESULT EvaluateTests( _In_ IPKTestEvaluator* pEvaluator )
    {
        T testGroupInstance;
        return( testGroupInstance.EvaluateTests( this->GetName(), pEvaluator ) );
    }
};


///////////////////////////////////////////////////////////////////////////////
//
// Secondary macros used for implementing the primary macros
//
///////////////////////////////////////////////////////////////////////////////

#define PKTEST_MAKE_STRA2( str )  str
#define PKTEST_MAKE_STRA( str )   PKTEST_MAKE_STRA2( str )

#define PKTEST_MAKE_METHOD_START( Name ) \
    struct TestMethod_##Name : public CPKTestMethod \
    { \
        TestMethod_##Name() { OwnerOf##Name( this )->m_TestMethods.Add( this ); } \
        const char* GetName() const { return( PKTEST_MAKE_STRA( #Name ) ); } \
        PKTEST_PROPERTY_LIST GetProperties() const \
        { const static PKTEST_PROPERTY_ENTRY s_props[] = {


#define PKTEST_MAKE_METHOD_END( Name ) \
                { NULL, NULL } }; return( s_props ); }\
        virtual void Run()  { OwnerOf##Name( this )->Name(); } \
    } \
    \
    m_TestMethodInstance_##Name; \
    \
    static size_t OffsetOf##Name( CPKTestGroup* pGroup ) \
    { \
        ThisTestGroup* pTypedGroup = static_cast<ThisTestGroup*>( pGroup ); \
        return( reinterpret_cast<char*>( &pTypedGroup->m_TestMethodInstance_##Name ) \
                - reinterpret_cast<char*>( pTypedGroup ) ); \
    } \
    static ThisTestGroup* OwnerOf##Name( TestMethod_##Name* pMethod ) \
    { \
        return( reinterpret_cast<ThisTestGroup*>( reinterpret_cast<char*>( pMethod ) - OffsetOf##Name( NULL ) ) ); \
    } \
    void Name()

///////////////////////////////////////////////////////////////////////////////
//
// Logger class and utilities
//
///////////////////////////////////////////////////////////////////////////////

class CPKTestLogger
{
public:

    CPKTestLogger(
        _In_ LOG_LEVEL level,
        _In_ const char* pszFileName,
        _In_ int nLineNumber,
        _In_ const char* pszFunctionName )
        : m_level( level )
        , m_pszFileName( pszFileName )
        , m_nLineNumber( nLineNumber )
        , m_pszFunctionName( pszFunctionName )
    {
    }

    void operator() ( _In_ const char* pszFmt, ... )
    {
        va_list arglist;
        va_start(arglist,pszFmt);
        Log( pszFmt, arglist );
        va_end(arglist);
    }

    void Log(
            _In_ const char* pszFmt,
            _In_ va_list arglist );

private:

    LOG_LEVEL m_level;
    const char* m_pszFileName;
    int m_nLineNumber;
    const char* m_pszFunctionName;
};

pkRESULT EscapeXmlStringInPlace(
        _Inout_count_(cchBuffer) char* pszBuffer,
        _In_ size_t cchBuffer );
