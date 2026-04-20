///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <PKTestSuite.h>

#include "MarshallingUtils.h"

//////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( BigEndianConversionTests )
{
    PKTEST_METHOD( BytesToHostVariableSize )
    {
        static const uint8_t buffer[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };

        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint8_t>( buffer, 1 ) == 0x12 );

        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint16_t>( buffer, 1 ) == 0x12 );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint16_t>( buffer, 2 ) == 0x1234 );

        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint32_t>( buffer, 1 ) == 0x12 );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint32_t>( buffer, 2 ) == 0x1234 );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint32_t>( buffer, 3 ) == 0x123456 );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint32_t>( buffer, 4 ) == 0x12345678 );

        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint64_t>( buffer, 1 ) == 0x12 );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint64_t>( buffer, 2 ) == 0x1234 );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint64_t>( buffer, 3 ) == 0x123456 );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint64_t>( buffer, 4 ) == 0x12345678 );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint64_t>( buffer, 5 ) == 0x123456789AULL );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint64_t>( buffer, 6 ) == 0x123456789ABCULL );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint64_t>( buffer, 7 ) == 0x123456789ABCDEULL );
        PKTEST_ASSERT_EXIT( BigEndian::BytesToHost<uint64_t>( buffer, 8 ) == 0x123456789ABCDEF0ULL );

    exit:

        return;
    }

    PKTEST_METHOD( BytesToHostFixedSize )
    {
        static const uint8_t buffer[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };

        PKTEST_ASSERT_EXIT( ( BigEndian::BytesToHost<uint8_t,1>( buffer ) == 0x12 ) );

        PKTEST_ASSERT_EXIT( ( BigEndian::BytesToHost<uint16_t,2>( buffer ) == 0x1234 ) );

        PKTEST_ASSERT_EXIT( ( BigEndian::BytesToHost<uint32_t,3>( buffer ) == 0x123456 ) );
        PKTEST_ASSERT_EXIT( ( BigEndian::BytesToHost<uint32_t,4>( buffer ) == 0x12345678 ) );

        PKTEST_ASSERT_EXIT( ( BigEndian::BytesToHost<uint64_t,8>( buffer ) == 0x123456789ABCDEF0ULL ) );

    exit:

        return;
    }

    PKTEST_METHOD( ToHost )
    {
        static const uint8_t buffer[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };

        PKTEST_ASSERT_EXIT( BigEndian::ToHost<uint8_t>( (uint8_t&)buffer ) == 0x12 );
        PKTEST_ASSERT_EXIT( BigEndian::ToHost<uint16_t>( (uint16_t&)buffer ) == 0x1234 );
        PKTEST_ASSERT_EXIT( BigEndian::ToHost<uint32_t>( (uint32_t&)buffer ) == 0x12345678 );
        PKTEST_ASSERT_EXIT( BigEndian::ToHost<uint64_t>( (uint64_t&)buffer ) == 0x123456789ABCDEF0ULL );

    exit:

        return;
    }

    PKTEST_METHOD( GuidToHost )
    {
        static const uint8_t buffer[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x08, 0x19, 0x2A, 0x3B, 0x4C, 0x5D, 0x6E, 0x7F };

        static const GUID expected = { 0x12345678, 0x9ABC, 0xDEF0, { 0x08, 0x19, 0x2A, 0x3B, 0x4C, 0x5D, 0x6E, 0x7F } };

        GUID result = BigEndian::ToHost<GUID>( (GUID&)buffer );

        PKTEST_ASSERT_EXIT( memcmp( &expected, &result, sizeof(GUID) ) == 0 );

    exit:

        return;
    }
};

//////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( LittleEndianConversionTests )
{
    PKTEST_METHOD( BytesToHostVariableSize )
    {
        static const uint8_t buffer[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };

        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint8_t>( buffer, 1 ) == 0x12 );

        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint16_t>( buffer, 1 ) == 0x12 );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint16_t>( buffer, 2 ) == 0x3412 );

        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint32_t>( buffer, 1 ) == 0x12 );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint32_t>( buffer, 2 ) == 0x3412 );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint32_t>( buffer, 3 ) == 0x563412 );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint32_t>( buffer, 4 ) == 0x78563412 );

        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint64_t>( buffer, 1 ) == 0x12 );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint64_t>( buffer, 2 ) == 0x3412 );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint64_t>( buffer, 3 ) == 0x563412 );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint64_t>( buffer, 4 ) == 0x78563412 );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint64_t>( buffer, 5 ) == 0x9A78563412ULL );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint64_t>( buffer, 6 ) == 0xBC9A78563412ULL );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint64_t>( buffer, 7 ) == 0xDEBC9A78563412ULL );
        PKTEST_ASSERT_EXIT( LittleEndian::BytesToHost<uint64_t>( buffer, 8 ) == 0xF0DEBC9A78563412ULL );

    exit:

        return;
    }

    PKTEST_METHOD( BytesToHostFixedSize )
    {
        static const uint8_t buffer[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };

        PKTEST_ASSERT_EXIT( ( LittleEndian::BytesToHost<uint8_t,1>( buffer ) == 0x12 ) );

        PKTEST_ASSERT_EXIT( ( LittleEndian::BytesToHost<uint16_t,2>( buffer ) == 0x3412 ) );

        PKTEST_ASSERT_EXIT( ( LittleEndian::BytesToHost<uint32_t,3>( buffer ) == 0x563412 ) );
        PKTEST_ASSERT_EXIT( ( LittleEndian::BytesToHost<uint32_t,4>( buffer ) == 0x78563412 ) );

        PKTEST_ASSERT_EXIT( ( LittleEndian::BytesToHost<uint64_t,8>( buffer ) == 0xF0DEBC9A78563412ULL ) );

    exit:

        return;
    }

    PKTEST_METHOD( ToHost )
    {
        static const uint8_t buffer[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };

        PKTEST_ASSERT_EXIT( LittleEndian::ToHost<uint8_t>( (uint8_t&)buffer ) == 0x12 );
        PKTEST_ASSERT_EXIT( LittleEndian::ToHost<uint16_t>( (uint16_t&)buffer ) == 0x3412 );
        PKTEST_ASSERT_EXIT( LittleEndian::ToHost<uint32_t>( (uint32_t&)buffer ) == 0x78563412 );
        PKTEST_ASSERT_EXIT( LittleEndian::ToHost<uint64_t>( (uint64_t&)buffer ) == 0xF0DEBC9A78563412ULL );

    exit:

        return;
    }

    PKTEST_METHOD( GuidToHost )
    {
        static const uint8_t buffer[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x08, 0x19, 0x2A, 0x3B, 0x4C, 0x5D, 0x6E, 0x7F };

        static const GUID expected = { 0x78563412, 0xBC9A, 0xF0DE, { 0x08, 0x19, 0x2A, 0x3B, 0x4C, 0x5D, 0x6E, 0x7F } };

        GUID result = LittleEndian::ToHost<GUID>( (GUID&)buffer );

        PKTEST_ASSERT_EXIT( memcmp( &expected, &result, sizeof(GUID) ) == 0 );

    exit:

        return;
    }
};
