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

//
// #define USE_MOVE_SEMANTICS_FOR_AUTOREFPTR   1
//
// :TODO: decide how/when to enable this for the PK.
// USE_MOVE_SEMANTICS_FOR_AUTOREFPTR should be defined in terms of some global
// build switch when the compiler can support C++0x r-value semantics.
//

#include <AutoRefPtr.h>

#include <vector>
#include <map>
#include <algorithm>

//
// Define some interface to use in the AutoRefPtr tests
//

struct ISomeInterface
{
    virtual void AddRef() = 0;
    virtual void Release() = 0;
};

//
// Implement that interface in a class that tracks references
//

class CRefCountedFixture
    : public ISomeInterface
{
public:

    //
    // ISomeInterface
    //

    virtual void AddRef()
    {
        ++m_cAddRefs;
    }

    virtual void Release()
    {
        ++m_cReleases;
    }

    //
    // Constructor
    //

    CRefCountedFixture()
        : m_cAddRefs( 0 )
        , m_cReleases( 0 )
        , m_cExpectedAddRefs( 0 )
        , m_cExpectedReleases( 0 )
    {
    }

    void ExpectAddRef()
    {
        ++m_cExpectedAddRefs;
    }

    void ExpectRelease()
    {
        ++m_cExpectedReleases;
    }

    bool PostCondition() const
    {
        return(
            ( m_cAddRefs == m_cExpectedAddRefs )
            && ( m_cReleases == m_cExpectedReleases ) );
    }

    void LogCounters()
    {
        LogTestComment(
            "AddRef %d (expected %d), Release %d (expected %d)",
            m_cAddRefs, m_cExpectedAddRefs, m_cReleases, m_cExpectedReleases
            );
    }

private:
    uint32 m_cAddRefs;
    uint32 m_cReleases;
    uint32 m_cExpectedAddRefs;
    uint32 m_cExpectedReleases;
};


////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( AutoRefPtrTests )
{
    ////////////////////////////////////////
    PKTEST_METHOD( NegativeTestForPostConditionDetector )
    {
        CRefCountedFixture fixture;

        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

        {
            fixture.ExpectAddRef();
            PKTEST_ASSERT_EXIT( !fixture.PostCondition() );
            fixture.AddRef();
            PKTEST_ASSERT_EXIT( fixture.PostCondition() );
        }

        {
            fixture.ExpectRelease();
            PKTEST_ASSERT_EXIT( !fixture.PostCondition() );
            fixture.Release();
            PKTEST_ASSERT_EXIT( fixture.PostCondition() );
        }

        {
            fixture.ExpectAddRef();
            fixture.ExpectRelease();
            PKTEST_ASSERT_EXIT( !fixture.PostCondition() );

            fixture.Release();

            PKTEST_ASSERT_EXIT( !fixture.PostCondition() );

            fixture.AddRef();

            PKTEST_ASSERT_EXIT( fixture.PostCondition() );
        }

    exit:

        return;
    }

    ////////////////////////////////////////
    PKTEST_METHOD( Constructors )
    {
        CRefCountedFixture fixture;

        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

        {
            //
            // Default constructor
            //

            AutoRefPtr<ISomeInterface> ap1;

            PKTEST_ASSERT_EXIT( ap1 == NULL );
        }

        {
            //
            // Constructor from naked pointer;
            //

            fixture.ExpectAddRef();

            AutoRefPtr<ISomeInterface> ap1( &fixture );

            PKTEST_ASSERT_EXIT( ap1 == &fixture );
            PKTEST_ASSERT_EXIT( fixture.PostCondition() );

            //
            // Copy constructor from another AutoRefPtr
            //

            {
                fixture.ExpectAddRef();

                AutoRefPtr<ISomeInterface> ap2( ap1 );

                PKTEST_ASSERT_EXIT( fixture.PostCondition() );

                fixture.ExpectRelease();
            }

            PKTEST_ASSERT_EXIT( fixture.PostCondition() );

            //
            // Move constructor
            //

            {
#if USE_MOVE_SEMANTICS_FOR_AUTOREFPTR

                fixture.ExpectAddRef();

                AutoRefPtr<ISomeInterface> ap2( &fixture );

                PKTEST_ASSERT_EXIT( fixture.PostCondition() );

                AutoRefPtr<ISomeInterface> ap3( std::move( ap2 ) );

                PKTEST_ASSERT_EXIT( fixture.PostCondition() );

                fixture.ExpectRelease();
#endif
            }

            PKTEST_ASSERT_EXIT( fixture.PostCondition() );

            fixture.ExpectRelease();
        }

        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

    exit:

        return;
    }

    ////////////////////////////////////////
    PKTEST_METHOD( CopyOperators )
    {
        CRefCountedFixture fixture;

        fixture.ExpectAddRef();

        AutoRefPtr<ISomeInterface> ap1( &fixture );

        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

        {
            //
            // Copy operator
            //

            AutoRefPtr<ISomeInterface> ap2;

            PKTEST_ASSERT_EXIT( ap2 == NULL );
            PKTEST_ASSERT_EXIT( ap2 != ap1 );

            fixture.ExpectAddRef();

            ap2 = ap1;

            PKTEST_ASSERT_EXIT( ap2 == ap1 );

            PKTEST_ASSERT_EXIT( fixture.PostCondition() );

            fixture.ExpectRelease();
        }

        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

        {
#if USE_MOVE_SEMANTICS_FOR_AUTOREFPTR

            //
            // Move assignment
            //

            fixture.ExpectAddRef();

            AutoRefPtr<ISomeInterface> ap3( &fixture );
            AutoRefPtr<ISomeInterface> ap4;

            PKTEST_ASSERT_EXIT( fixture.PostCondition() );

            PKTEST_ASSERT_EXIT( ap3 == &fixture );
            PKTEST_ASSERT_EXIT( ap4 == NULL );

            ap4 = std::move( ap3 );

            PKTEST_ASSERT_EXIT( fixture.PostCondition() );

            PKTEST_ASSERT_EXIT( ap3 == NULL );
            PKTEST_ASSERT_EXIT( ap4 == &fixture );

            fixture.ExpectRelease();
#endif
        }

        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

    exit:

        return;
    }

    ////////////////////////////////////////
    PKTEST_METHOD( SetAndRelease )
    {
        CRefCountedFixture fixture;
        AutoRefPtr<ISomeInterface> ap1;

        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

        fixture.ExpectAddRef();
        ap1.Set( &fixture );

        PKTEST_ASSERT_EXIT( ap1 == &fixture );
        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

        fixture.ExpectRelease();
        ap1.Release();

        PKTEST_ASSERT_EXIT( ap1 == NULL );
        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

    exit:

        return;
    }

    ////////////////////////////////////////
    PKTEST_METHOD( VectorPreallocatedPushBackAndCleanup )
    {
        const size_t NUM_OF_ELEMENTS = 5;

        CRefCountedFixture fixtures[NUM_OF_ELEMENTS];

        {
            std::vector< AutoRefPtr<ISomeInterface> > v1;

            // Pre-reserve to avoid reallocations

            v1.reserve( NUM_OF_ELEMENTS );

            for( size_t i = 0; i < NUM_OF_ELEMENTS; ++i )
            {
#if !USE_MOVE_SEMANTICS_FOR_AUTOREFPTR
                // for the push_back param
                fixtures[i].ExpectAddRef();
                fixtures[i].ExpectRelease();
#endif

                fixtures[i].ExpectAddRef();

                v1.push_back( AutoRefPtr<ISomeInterface>( &fixtures[i] ) );

                PKTEST_ASSERT_EXIT( v1[i] == &fixtures[i] );
            }

            for( size_t i = 0; i < NUM_OF_ELEMENTS; ++i )
            {
                PKTEST_ASSERT_EXIT( fixtures[i].PostCondition() );
            }

            for( size_t i = 0; i < NUM_OF_ELEMENTS; ++i )
            {
                fixtures[i].ExpectRelease();
            }
        }

        for( size_t i = 0; i < NUM_OF_ELEMENTS; ++i )
        {
            PKTEST_ASSERT_EXIT( fixtures[i].PostCondition() );
        }

    exit:
        return;
    }

#if USE_MOVE_SEMANTICS_FOR_AUTOREFPTR
    ////////////////////////////////////////
    PKTEST_METHOD( VectorPushFront )
    {
        //
        // Only doing this test if move semantics are enabled,
        // otherwise the copy operations used by std::vector are
        // too complex to make assertive expectations.
        //

        const size_t NUM_OF_ELEMENTS = 5;

        CRefCountedFixture fixtures[NUM_OF_ELEMENTS];

        {
            std::vector<AutoRefPtr<ISomeInterface>> v1;

            for( size_t i = 0; i < NUM_OF_ELEMENTS; ++i )
            {
                fixtures[i].ExpectAddRef();

                v1.insert( v1.begin(), AutoRefPtr<ISomeInterface>( &fixtures[i] ) );

                PKTEST_ASSERT_EXIT( v1[0] == &fixtures[i] );
            }

            for( size_t i = 0; i < NUM_OF_ELEMENTS; ++i )
            {
                PKTEST_ASSERT_EXIT( fixtures[i].PostCondition() );
            }
        }

    exit:

        return;
    }
#endif

    ////////////////////////////////////////
    PKTEST_METHOD( MapInsertionAndCleanup )
    {
        const char* rgKeys[] =
        {
            "wug",
            "blicket",
            "dax",
            "toma",
            "pimwit",
            "zav",
        };

        const char* rgEmptyKeys[] =
        {
            "speff",
            "tulver",
            "gazzer",
            "fem",
            "fendle",
            "tupa",
        };

        const size_t NUM_OF_ELEMENTS = sizeof(rgKeys)/sizeof(rgKeys[0]);

        CRefCountedFixture fixtures[NUM_OF_ELEMENTS];

        {
            typedef std::map< std::string, AutoRefPtr<ISomeInterface> > SomeInterfaceMap;

            SomeInterfaceMap m1;

            // Insert the elements

            for( size_t i = 0; i < NUM_OF_ELEMENTS; ++i )
            {
#if !USE_MOVE_SEMANTICS_FOR_AUTOREFPTR
                // For the temp r-value
                fixtures[i].ExpectAddRef();
                fixtures[i].ExpectRelease();
#endif

                fixtures[i].ExpectAddRef();

                m1[ rgKeys[i] ] = AutoRefPtr<ISomeInterface>( &fixtures[i] );
            }

            // Check the elements exist in the map

            for( size_t i = 0; i < NUM_OF_ELEMENTS; ++i )
            {
                SomeInterfaceMap::iterator it = m1.find( rgKeys[i] );

                PKTEST_ASSERT_EXIT( it != m1.end() );

                PKTEST_ASSERT_EXIT( it->first.compare( rgKeys[i] ) == 0 );
                PKTEST_ASSERT_EXIT( it->second == &fixtures[i] );

                PKTEST_ASSERT_EXIT( m1[ rgKeys[i] ] == &fixtures[i] );
            }

            // Try to get the non-existing elements.
            // This should not affect the ref counting of what's already in the map

            for( size_t i = 0; i < sizeof(rgEmptyKeys)/sizeof(rgEmptyKeys[0]); ++i )
            {
                PKTEST_ASSERT_EXIT( m1.find( rgEmptyKeys[i] ) == m1.end() );
            }

            // The map [] operator will construct emtpy entries for the accessed items.
            // Again, this should not affect the ref counting of what's already in the map

            for( size_t i = 0; i < sizeof(rgEmptyKeys)/sizeof(rgEmptyKeys[0]); ++i )
            {
                SomeInterfaceMap::iterator it;

                // Accessing the item creates the default one.

                PKTEST_ASSERT_EXIT( m1[ rgEmptyKeys[i] ] == NULL );

                // Now the item exists, but it's a NULL pointer

                it = m1.find( rgEmptyKeys[i] );

                PKTEST_ASSERT_EXIT( it != m1.end() );

                PKTEST_ASSERT_EXIT( it->first.compare( rgEmptyKeys[i] ) == 0 );
                PKTEST_ASSERT_EXIT( it->second == NULL );
            }

            for( size_t i = 0; i < NUM_OF_ELEMENTS; ++i )
            {
                fixtures[i].ExpectRelease();
            }
        }

        for( size_t i = 0; i < NUM_OF_ELEMENTS; ++i )
        {
            PKTEST_ASSERT_EXIT( fixtures[i].PostCondition() );
        }

    exit:

        return;
    }

    ////////////////////////////////////////
    PKTEST_METHOD( TransferOfOwnership )
    {
        CRefCountedFixture fixture;
        AutoRefPtr<ISomeInterface> ap1;
        AutoRefPtr<ISomeInterface> ap2;
        ISomeInterface* pTemp = NULL;

        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

        fixture.ExpectAddRef();
        ap1.Set( &fixture );

        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

        // Hand off. Don't expect changes in ref-counting.

        PKTEST_ASSERT_EXIT( ap1 == &fixture );

        pTemp = ap1.HandOffRef();
        PKTEST_ASSERT_EXIT( fixture.PostCondition() );
        PKTEST_ASSERT_EXIT( pTemp == &fixture );

        PKTEST_ASSERT_EXIT( ap1 == NULL );

        // Adopt. Don't expect changes in ref-counting.

        PKTEST_ASSERT_EXIT( ap2 == NULL );

        ap2.AdoptRef( pTemp );
        PKTEST_ASSERT_EXIT( fixture.PostCondition() );

        PKTEST_ASSERT_EXIT( ap2 == &fixture );

    exit:

        return;
    }
};
