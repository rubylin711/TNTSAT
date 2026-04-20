///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace SSPK
{
    /// <summary>
    /// Class that represents a time interval as the ratio between a tick
    /// count and a clock frequency (i.e., time scale). Because it requires
    /// extra memory to store the frequency, this class is intended more as
    /// a "function object" to convert tick counts between different clock
    /// frequencies than as a generic way to store and pass time values around
    /// -- for that refer to the TimeSpan template.
    /// </summary>
    class TimeSpanGeneric
    {
    public:

        TimeSpanGeneric();

        /// <summary>
        /// Sets this instance with a time interval. If the source and
        /// destination frequencies differ, the tick count will be converted
        /// to the destination frequency.
        /// </summary>
        /// <param name="fromTicks">source tick count</param>
        /// <param name="fromTicksPerSecond">source tick frequency</param>
        /// <param name="toTicksPerSecond">destination tick frequency</param>
        /// <param name="pRemainder">receives the remainder of the frequency conversion</param>
        /// <returns>
        ///     - pkS_OK on success.
        ///     - pkE_INVALIDARG if conversion causes an overflow.
        /// </returns>
        pkRESULT Set(
                    _In_ int64_t fromTicks,
                    _In_ int64_t fromTicksPerSecond,
                    _In_ int64_t toTicksPerSecond,
                    _Out_opt_ int64_t* pRemainder = NULL );

        /// <returns>
        /// Number of ticks of this instance.
        /// </returns>
        int64_t Ticks() const;

        /// <returns>
        /// Frequency in ticks per second of this instance.
        /// </returns>
        int64_t TicksPerSecond() const;

        static const int64_t MAX_TICKS = 0x7fffffffffffffffLL;
        static const int64_t MIN_TICKS = 0x8000000000000000LL;

    private:

        int64_t _ticks;
        int64_t _ticksPerSecond;
    };

    inline TimeSpanGeneric::TimeSpanGeneric()
        : _ticks( 0 )
        , _ticksPerSecond( 1 )
    {
    }

    inline int64_t TimeSpanGeneric::Ticks() const
    {
        return( _ticks );
    }

    inline int64_t TimeSpanGeneric::TicksPerSecond() const
    {
        return( _ticksPerSecond );
    }

    /// <summary>
    /// Template class that represents a time interval as the ratio between
    /// a tick count and a constant clock frequency. The frequency is a
    /// parameter of the template.
    /// </summary>
    template< int64_t TICKS_PER_SECOND >
    class TimeSpan
    {
    public:

        /// <summary>
        /// Default constructor.
        /// </summary>
        TimeSpan();

        /// <summary>
        /// Instantiates a time span given a tick count
        /// in the type's implicit frequency.
        /// </summary>
        /// <returns>the instance</returns>
        static TimeSpan FromTicks( _In_ int64_t ticks );

        /// <summary>
        /// Instantiates a time span given a tick count and a frequency.
        /// The value is converted to this type's implicit frequency.
        /// This method ignores overflow errors in the conversion between frequencies.
        /// </summary>
        /// <returns>the instance</returns>
        static TimeSpan ConvertFrom( _In_ int64_t ticks, _In_ int64_t ticksPerSecond );

        /// <summary>
        /// Instantiates a time span from another time span instance.
        /// The argument may be from a different time span type
        /// in which case the value is converted to this type's frequency.
        /// </summary>
        /// <returns>the instance</returns>
        template< class T >
        static TimeSpan ConvertFrom( _In_ const T& time );

        /// <returns>
        /// Number of ticks of this instance.
        /// </returns>
        int64_t Ticks() const;

        /// <returns>
        /// Frequency in ticks per second of this instance.
        /// </returns>
        int64_t TicksPerSecond() const;

        /// <summary>
        /// Sets the value of the tick count of this instance to zero.
        /// </summary>
        void ResetTicks();

        /// <summary>
        /// Sets this instance with a time interval given by a tick count and
        /// a tick count frequency. The value is converted to this instance's
        /// implicit frequency.
        /// </summary>
        /// <param name="fromTicks">source tick count</param>
        /// <param name="fromTicksPerSecond">source tick frequency</param>
        /// <param name="pRemainder">receives the remainder of the conversion</param>
        /// <returns>
        ///     - pkS_OK on success.
        ///     - pkE_INVALIDARG if conversion causes an overflow.
        /// </returns>
        pkRESULT Set(
                    _In_ int64_t fromTicks,
                    _In_ int64_t fromTicksPerSecond,
                    _Out_opt_ int64_t* pRemainder = NULL );

        /// <summary>
        /// Sets this instance with a time interval given by another time span
        /// of same implicit frequency. This method is provided for optimization
        /// of the next method when the argument is of the same type.
        /// </summary>
        /// <param name="from">input TimeSpan instance</param>
        /// <param name="pRemainder">receives the remainder of the conversion</param>
        /// <returns>
        ///     - pkS_OK on success.
        ///     - pkE_INVALIDARG if conversion causes an overflow.
        /// </returns>
        pkRESULT Set(
                    _In_ const TimeSpan<TICKS_PER_SECOND>& from,
                    _Out_opt_ int64_t* pRemainder = NULL );

        /// <summary>
        /// Sets this instance with a time interval given by another time span.
        /// The value is converted to this instance's implicit frequency.
        /// </summary>
        /// <param name="from">input TimeSpan instance</param>
        /// <param name="pRemainder">receives the remainder of the conversion</param>
        /// <returns>
        ///     - pkS_OK on success.
        ///     - pkE_INVALIDARG if conversion causes an overflow.
        /// </returns>
        template< class T >
        pkRESULT Set(
                    _In_ const T& from,
                    _Out_opt_ int64_t* pRemainder = NULL );

        /// <summary>
        /// Converts the value of this instance to seconds. The result can
        /// be fractional and there could be rounding errors in the conversion.
        /// </summary>
        /// <returns>
        /// The value of the current instance as whole and fractional number of seconds.
        /// </returns>
        double ToSeconds() const;

        // Compare operators
        bool operator > ( const TimeSpan& that ) const;
        bool operator < ( const TimeSpan& that ) const;
        bool operator >= ( const TimeSpan& that ) const;
        bool operator <= ( const TimeSpan& that ) const;
        bool operator == ( const TimeSpan& that ) const;
        bool operator != ( const TimeSpan& that ) const;

        static const int64_t MAX_TICKS = 0x7fffffffffffffffLL;
        static const int64_t MIN_TICKS = 0x8000000000000000LL;

    private:

        int64_t _ticks;

        explicit TimeSpan( _In_ int64_t ticks );
    };

    template< int64_t TICKS_PER_SECOND >
    inline TimeSpan<TICKS_PER_SECOND>::TimeSpan()
        : _ticks( 0 )
    {
        typedef char TimeSpan_Requires_Frequency_Larger_Than_Zero[ TICKS_PER_SECOND > 0 ? 1 : -1 ];
    }

    template< int64_t TICKS_PER_SECOND >
    inline TimeSpan<TICKS_PER_SECOND> TimeSpan<TICKS_PER_SECOND>::FromTicks( _In_ int64_t ticks )
    {
        return( TimeSpan( ticks ) );
    }

    template< int64_t TICKS_PER_SECOND >
    inline TimeSpan<TICKS_PER_SECOND> TimeSpan<TICKS_PER_SECOND>::ConvertFrom( _In_ int64_t ticks, _In_ int64_t ticksPerSecond )
    {
        TimeSpan<TICKS_PER_SECOND> t;
        t.Set( ticks, ticksPerSecond );
        return( t );
    }

    template< int64_t TICKS_PER_SECOND > template< class T >
    inline TimeSpan<TICKS_PER_SECOND> TimeSpan<TICKS_PER_SECOND>::ConvertFrom( _In_ const T& from )
    {
        TimeSpan<TICKS_PER_SECOND> t;
        t.Set( from );
        return( t );
    }

    template< int64_t TICKS_PER_SECOND >
    inline int64_t TimeSpan<TICKS_PER_SECOND>::Ticks() const
    {
        return( _ticks );
    }

    template< int64_t TICKS_PER_SECOND >
    inline int64_t TimeSpan<TICKS_PER_SECOND>::TicksPerSecond() const
    {
        return( TICKS_PER_SECOND );
    }

    template< int64_t TICKS_PER_SECOND >
    inline void TimeSpan<TICKS_PER_SECOND>::ResetTicks()
    {
        _ticks = 0;
    }

    template< int64_t TICKS_PER_SECOND >
    inline pkRESULT TimeSpan<TICKS_PER_SECOND>::Set(
        _In_ int64_t fromTicks,
        _In_ int64_t fromTicksPerSecond,
        _Out_opt_ int64_t* pRemainder )
    {
        TimeSpanGeneric temp;
        pkRESULT pkr = temp.Set( fromTicks, fromTicksPerSecond, TICKS_PER_SECOND, pRemainder );
        _ticks = temp.Ticks();
        return( pkr );
    }

    template< int64_t TICKS_PER_SECOND >
    inline pkRESULT TimeSpan<TICKS_PER_SECOND>::Set(
        _In_ const TimeSpan<TICKS_PER_SECOND>& from,
        _Out_opt_ int64_t* pRemainder )
    {
        *this = from;

        if( pRemainder != NULL )
        {
            *pRemainder = 0;
        }

        return( pkS_OK );
    }

    template< int64_t TICKS_PER_SECOND > template< class T >
    inline pkRESULT TimeSpan<TICKS_PER_SECOND>::Set(
        _In_ const T& from,
        _Out_opt_ int64_t* pRemainder )
    {
        return( Set( from.Ticks(), from.TicksPerSecond(), pRemainder ) );
    }

    template< int64_t TICKS_PER_SECOND >
    inline double TimeSpan<TICKS_PER_SECOND>::ToSeconds() const
    {
        return( (double) _ticks / TICKS_PER_SECOND );
    }

    template< int64_t TICKS_PER_SECOND >
    inline bool TimeSpan<TICKS_PER_SECOND>::operator > ( const TimeSpan<TICKS_PER_SECOND>& that ) const
    {
        return( _ticks > that._ticks );
    }

    template< int64_t TICKS_PER_SECOND >
    inline bool TimeSpan<TICKS_PER_SECOND>::operator < ( const TimeSpan<TICKS_PER_SECOND>& that ) const
    {
        return( _ticks < that._ticks );
    }

    template< int64_t TICKS_PER_SECOND >
    inline bool TimeSpan<TICKS_PER_SECOND>::operator >= ( const TimeSpan<TICKS_PER_SECOND>& that ) const
    {
        return( _ticks >= that._ticks );
    }

    template< int64_t TICKS_PER_SECOND >
    inline bool TimeSpan<TICKS_PER_SECOND>::operator <= ( const TimeSpan<TICKS_PER_SECOND>& that ) const
    {
        return( _ticks <= that._ticks );
    }

    template< int64_t TICKS_PER_SECOND >
    inline bool TimeSpan<TICKS_PER_SECOND>::operator == ( const TimeSpan<TICKS_PER_SECOND>& that ) const
    {
        return( _ticks == that._ticks );
    }

    template< int64_t TICKS_PER_SECOND >
    inline bool TimeSpan<TICKS_PER_SECOND>::operator != ( const TimeSpan<TICKS_PER_SECOND>& that ) const
    {
        return( _ticks != that._ticks );
    }

    // Arithmetic operators

    template< int64_t TICKS_PER_SECOND >
    TimeSpan<TICKS_PER_SECOND> operator + ( const TimeSpan<TICKS_PER_SECOND>& a, const TimeSpan<TICKS_PER_SECOND>& b )
    {
        return( TimeSpan<TICKS_PER_SECOND>::FromTicks( a.Ticks() + b.Ticks() ) );
    }

    template< int64_t TICKS_PER_SECOND >
    TimeSpan<TICKS_PER_SECOND> operator - ( const TimeSpan<TICKS_PER_SECOND>& a, const TimeSpan<TICKS_PER_SECOND>& b )
    {
        return( TimeSpan<TICKS_PER_SECOND>::FromTicks( a.Ticks() - b.Ticks() ) );
    }

    // Private methods

    template< int64_t TICKS_PER_SECOND >
    inline TimeSpan< TICKS_PER_SECOND >::TimeSpan( _In_ int64_t ticks )
        : _ticks( ticks )
    {
    }

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Predefined common time span types
    //
    ////////////////////////////////////////////////////////////////////////////////

    static const int64_t NTP_TICKS_PER_SECOND =     0x100000000LL;
    static const int64_t HNS_TICKS_PER_SECOND =     10000000;
    static const int64_t PTS_TICKS_PER_SECOND =     90000;
    static const int64_t MILLISECONDS_PER_SECOND =  1000;
    static const int64_t SECONDS_PER_SECOND =       1;

    typedef TimeSpan<NTP_TICKS_PER_SECOND>      TimeSpan_NTP;
    typedef TimeSpan<HNS_TICKS_PER_SECOND>      TimeSpan_hns;
    typedef TimeSpan<PTS_TICKS_PER_SECOND>      TimeSpan_PTS;
    typedef TimeSpan<MILLISECONDS_PER_SECOND>   TimeSpan_ms;
    typedef TimeSpan<SECONDS_PER_SECOND>        TimeSpan_s;
};
