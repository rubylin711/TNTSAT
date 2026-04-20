///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSPKTimeSpan.h"
#include "SSPKDefines.h"

namespace SSPK
{

pkRESULT TimeSpanGeneric::Set(
    _In_ int64_t fromTicks,
    _In_ int64_t fromTicksPerSecond,
    _In_ int64_t toTicksPerSecond,
    _Out_opt_ int64_t* pRemainder )
{
    pkRESULT pkr = pkS_OK;

    static const uint64_t FACTOR32 = ( 1ULL << 32 );
    static const uint64_t BIT63_MASK = ( 1ULL << 63 );

    bool fNegative = ( fromTicks < 0 ) ^ ( toTicksPerSecond < 0 ) ^ ( fromTicksPerSecond < 0 );

    _ticksPerSecond = toTicksPerSecond;

    if( fromTicksPerSecond == toTicksPerSecond )
    {
        //
        // No need for conversion
        //

        _ticks = fromTicks;

        if( pRemainder != NULL )
        {
            *pRemainder = 0;
        }
    }
    else
    {
        //
        // Convert from the source frequency to the destination frequency.
        // This is basically a calculation in the form of ( a * b ) / div
        // where a = fromTicks, b = toTicksPerSecond, div = fromTicksPerSecond.
        // The calculation is easier with unsigned integers.
        //

        uint64_t ua = ( fromTicks < 0 ) ? -fromTicks : +fromTicks;
        uint64_t ub = ( toTicksPerSecond < 0 ) ? -toTicksPerSecond : +toTicksPerSecond;
        uint64_t udiv = ( fromTicksPerSecond < 0 ) ? -fromTicksPerSecond : +fromTicksPerSecond;

        //
        // First, let's do ( a * b ), which in 128 bits looks like:
        //
        //      ( aH * bH ) * ( 1 << 64 ) + ( aH * bL + aL * bH ) * ( 1 << 32 ) + ( aL * bL )
        //
        // Let's split it into high and low 64 bit parts
        //

        uint64_t mid = ( ua / FACTOR32 ) * ( ub % FACTOR32 ) + ( ua % FACTOR32 ) * ( ub / FACTOR32 );

        uint64_t low = ( ua % FACTOR32 ) * ( ub % FACTOR32 );

        uint64_t high = ( ua / FACTOR32 ) * ( ub / FACTOR32 );

        mid = mid + ( low / FACTOR32 );

        low = ( low % FACTOR32 ) | ( ( mid % FACTOR32 ) * FACTOR32 );

        high = high + ( mid / FACTOR32 );

        //
        // Second, let's do ( a * b ) / div.
        //

        if( udiv <= high )
        {
            //
            // Error: The division would overflow a int64_t
            // (this covers the case of division by zero too)
            //

            if( pRemainder != NULL )
            {
                *pRemainder = 0;
            }

            pkr = E_INVALIDARG;
            goto exit;
        }

        if( high == 0 )
        {
            //
            // The multiplication didn't overflow a uint64_t,
            // so this is a simple division
            //

            if( pRemainder != NULL )
            {
                *pRemainder = fNegative ? -(int64_t)( low % udiv ) : +(int64_t)( low % udiv );
            }

            _ticks = (int64_t)( low / udiv );
        }
        else
        {
            //
            // The multiplication overflows a uint64_t,
            // but the division brings it back into a uint64_t,
            // so we need to compute the division in long form.
            //

            uint64_t result = 0;

            for( int iBit = 0; iBit < 64; ++iBit )
            {
                result <<= 1;

                high <<= 1;

                if( low & BIT63_MASK )
                {
                    ++high;
                }

                low <<= 1;

                if( udiv <= high )
                {
                    high -= udiv;
                    ++result;
                }
            }

            if( pRemainder != NULL )
            {
                *pRemainder = fNegative ? -(int64_t)high : +(int64_t)high;
            }

            _ticks = (int64_t)result;
        }

        //
        // Now apply the signal
        //

        if( _ticks < 0 )
        {
            // The computation didn't overflow an unsigned 64 int,
            // but it did overflow when converted to a signed 64 int.
            pkr = pkE_INVALIDARG;
            goto exit;
        }

        if( fNegative )
        {
            _ticks = -_ticks;
        }
    }

exit:

    if( FAILED(pkr) )
    {
        // On overflow set the _ticks to the max value according
        // to signal
        _ticks = fNegative ? (int64_t)MIN_TICKS : (int64_t)MAX_TICKS;
    }

    return( pkr );
}

};  // namespace SSPK
