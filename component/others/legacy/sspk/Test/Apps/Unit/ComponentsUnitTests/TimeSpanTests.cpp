///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <PKTestSuite.h>

#include <SSPKTimeSpan.h>

// Auxiliary test class used for enumerating powers of 2,
// positive and negative,
class PowerOf2
{
public:

    int exponent;
    int signal;
    int64_t value;

    PowerOf2()
        : exponent( -1 )   // before the start
        , signal( +1 )
    {
    }

    bool MoveNext()
    {
        ++exponent;

        if( exponent >= 63 )
        {
            if( signal < 0 )
            {
                // End of negative powers of 2.
                // Stop
                return( false );
            }

            // End of positive powers of 2.
            // Flip the signal and start over for the negative values.

            signal = -1;
            exponent = 0;
        }

        value = ( 1LL << exponent ) * signal;

        return( true );
    }
};

static const int64_t MAX = 0x7fffffffffffffffLL;
static const int64_t MIN = 0x8000000000000000LL;

struct
{
    int64_t fromTicks;
    int64_t fromTicksPerSecond;
    int64_t toTicksPerSecond;
    int64_t expectedTicks;
    int64_t expectedRemainder;
}
static const g_rgValidEntries[] =
{
    {        MAX,        MAX,        MAX,                        MAX,                        0LL },
    {        MAX,        MIN,        MAX,     -9223372036854775806LL,                        1LL },
    {        MAX,        MAX,    MAX / 2,                    MAX / 2,                        0LL },
    {        MAX,    MAX / 2,    MAX / 2,                        MAX,                        0LL },
    {        MAX,        MIN,    MAX / 2,     -4611686018427387902LL,      4611686018427387905LL },
    {        MAX,    MIN / 2,    MAX / 2,     -9223372036854775805LL,                        1LL },
    {        MAX,        MIN,        MIN,                        MAX,                        0LL },
    {        MAX,        MAX,    MIN / 2,                    MIN / 2,                        0LL },
    {        MAX,        MIN,    MIN / 2,                    MAX / 2,                    MIN / 2 },
    {        MAX,    MIN / 2,    MIN / 2,                        MAX,                        0LL },
    {        MAX,        MAX,     7549LL,                     7549LL,                        0LL },
    {        MAX,    MAX / 2,     7549LL,                    15098LL,                     7549LL },
    {        MAX,        MIN,     7549LL,                    -7548LL,      9223372036854768259LL },
    {        MAX,    MIN / 2,     7549LL,                   -15097LL,      4611686018427380355LL },
    {        MAX,     7549LL,     7549LL,                        MAX,                        0LL },
    {        MAX,    14173LL,     7549LL,      4912667431469463244LL,                     9831LL },
    {        MAX, 10000000LL,     7549LL,         6962723550621670LL,                  2567043LL },
    {        MAX,    90000LL,     7549LL,       773635950069074472LL,                    87043LL },
    {        MAX,   104729LL,     7549LL,       664832429472416451LL,                    70264LL },
    {        MAX,        MAX,    14173LL,                    14173LL,                        0LL },
    {        MAX,    MAX / 2,    14173LL,                    28346LL,                    14173LL },
    {        MAX,        MIN,    14173LL,                   -14172LL,      9223372036854761635LL },
    {        MAX,    MIN / 2,    14173LL,                   -28345LL,      4611686018427373731LL },
    {        MAX,    14173LL,    14173LL,                        MAX,                        0LL },
    {        MAX, 10000000LL,    14173LL,        13072285187834273LL,                  7512611LL },
    {        MAX,    90000LL,    14173LL,      1452476131981585972LL,                    32611LL },
    {        MAX,   104729LL,    14173LL,      1248201089271765580LL,                    84791LL },
    {        MAX,        MAX, 10000000LL,                 10000000LL,                        0LL },
    {        MAX,    MAX / 2, 10000000LL,                 20000000LL,                 10000000LL },
    {        MAX,        MIN, 10000000LL,                 -9999999LL,      9223372036844775808LL },
    {        MAX,    MIN / 2, 10000000LL,                -19999999LL,      4611686018417387904LL },
    {        MAX, 10000000LL, 10000000LL,                        MAX,                        0LL },
    {        MAX,        MAX,    90000LL,                    90000LL,                        0LL },
    {        MAX,    MAX / 2,    90000LL,                   180000LL,                    90000LL },
    {        MAX,        MIN,    90000LL,                   -89999LL,      9223372036854685808LL },
    {        MAX,    MIN / 2,    90000LL,                  -179999LL,      4611686018427297904LL },
    {        MAX, 10000000LL,    90000LL,        83010348331692982LL,                  2630000LL },
    {        MAX,    90000LL,    90000LL,                        MAX,                        0LL },
    {        MAX,   104729LL,    90000LL,      7926204616838982732LL,                    90372LL },
    {        MAX,        MAX,   104729LL,                   104729LL,                        0LL },
    {        MAX,    MAX / 2,   104729LL,                   209458LL,                   104729LL },
    {        MAX,        MIN,   104729LL,                  -104728LL,      9223372036854671079LL },
    {        MAX,    MIN / 2,   104729LL,                  -209457LL,      4611686018427283175LL },
    {        MAX, 10000000LL,   104729LL,        96595453004776381LL,                  5491303LL },
    {        MAX,   104729LL,   104729LL,                        MAX,                        0LL },
    {    MAX / 2,        MAX,    MAX / 2,                  MAX / 4LL,      2305843009213693952LL },
    {    MAX / 2,    MAX / 2,    MAX / 2,                    MAX / 2,                        0LL },
    {    MAX / 2,        MIN,    MAX / 2,     -2305843009213693951LL,                        1LL },
    {    MAX / 2,    MIN / 2,    MAX / 2,     -4611686018427387902LL,                        1LL },
    {    MAX / 2,        MAX,        MIN,     -4611686018427387903LL,     -4611686018427387903LL },
    {    MAX / 2,        MIN,        MIN,                    MAX / 2,                        0LL },
    {    MAX / 2,    MIN / 2,        MIN,      9223372036854775806LL,                        0LL },
    {    MAX / 2,        MAX,    MIN / 2,     -2305843009213693951LL,     -6917529027641081855LL },
    {    MAX / 2,    MAX / 2,    MIN / 2,                    MIN / 2,                        0LL },
    {    MAX / 2,        MIN,    MIN / 2,                  MAX / 4LL,                    MIN / 2 },
    {    MAX / 2,    MIN / 2,    MIN / 2,                    MAX / 2,                        0LL },
    {    MAX / 2,        MAX,     7549LL,                     3774LL,      4611686018427384129LL },
    {    MAX / 2,    MAX / 2,     7549LL,                     7549LL,                        0LL },
    {    MAX / 2,        MIN,     7549LL,                    -3774LL,      4611686018427380355LL },
    {    MAX / 2,    MIN / 2,     7549LL,                    -7548LL,      4611686018427380355LL },
    {    MAX / 2,     7549LL,     7549LL,                    MAX / 2,                        0LL },
    {    MAX / 2,    14173LL,     7549LL,      2456333715734731622LL,                     1141LL },
    {    MAX / 2, 10000000LL,     7549LL,         3481361775310835LL,                  1279747LL },
    {    MAX / 2,    90000LL,     7549LL,       386817975034537236LL,                    39747LL },
    {    MAX / 2,   104729LL,     7549LL,       332416214736208225LL,                    83722LL },
    {    MAX / 2,        MAX,    14173LL,                     7086LL,      4611686018427380817LL },
    {    MAX / 2,    MAX / 2,    14173LL,                    14173LL,                        0LL },
    {    MAX / 2,        MIN,    14173LL,                    -7086LL,      4611686018427373731LL },
    {    MAX / 2,    MIN / 2,    14173LL,                   -14172LL,      4611686018427373731LL },
    {    MAX / 2,     7549LL,    14173LL,      8658289301784523612LL,                     2231LL },
    {    MAX / 2,    14173LL,    14173LL,                    MAX / 2,                        0LL },
    {    MAX / 2, 10000000LL,    14173LL,         6536142593917136LL,                  8749219LL },
    {    MAX / 2,    90000LL,    14173LL,       726238065990792986LL,                     9219LL },
    {    MAX / 2,   104729LL,    14173LL,       624100544635882790LL,                    35309LL },
    {    MAX / 2,        MAX, 10000000LL,                  4999999LL,      9223372036849775807LL },
    {    MAX / 2,    MAX / 2, 10000000LL,                 10000000LL,                        0LL },
    {    MAX / 2,        MIN, 10000000LL,                 -4999999LL,      9223372036844775808LL },
    {    MAX / 2,    MIN / 2, 10000000LL,                 -9999999LL,      4611686018417387904LL },
    {    MAX / 2, 10000000LL, 10000000LL,                    MAX / 2,                        0LL },
    {    MAX / 2,        MAX,    90000LL,                    44999LL,      9223372036854730807LL },
    {    MAX / 2,    MAX / 2,    90000LL,                    90000LL,                        0LL },
    {    MAX / 2,        MIN,    90000LL,                   -44999LL,      9223372036854685808LL },
    {    MAX / 2,    MIN / 2,    90000LL,                   -89999LL,      4611686018427297904LL },
    {    MAX / 2, 10000000LL,    90000LL,        41505174165846491LL,                  1270000LL },
    {    MAX / 2,    90000LL,    90000LL,                    MAX / 2,                        0LL },
    {    MAX / 2,   104729LL,    90000LL,      3963102308419491366LL,                      186LL },
    {    MAX / 2,        MAX,   104729LL,                    52364LL,      4611686018427335539LL },
    {    MAX / 2,    MAX / 2,   104729LL,                   104729LL,                        0LL },
    {    MAX / 2,        MIN,   104729LL,                   -52364LL,      4611686018427283175LL },
    {    MAX / 2,    MIN / 2,   104729LL,                  -104728LL,      4611686018427283175LL },
    {    MAX / 2, 10000000LL,   104729LL,        48297726502388190LL,                  7693287LL },
    {    MAX / 2,    90000LL,   104729LL,      5366414055820910085LL,                    43287LL },
    {    MAX / 2,   104729LL,   104729LL,                    MAX / 2,                        0LL },
    {        MIN,        MAX,    MIN / 2,      4611686018427387904LL,      4611686018427387904LL },
    {        MIN,        MIN,    MIN / 2,                    MIN / 2,                        0LL },
    {        MIN,        MAX,     7549LL,                    -7549LL,                    -7549LL },
    {        MIN,    MAX / 2,     7549LL,                   -15098LL,                   -15098LL },
    {        MIN,        MIN,     7549LL,                     7549LL,                        0LL },
    {        MIN,    MIN / 2,     7549LL,                    15098LL,                        0LL },
    {        MIN,    14173LL,     7549LL,     -4912667431469463245LL,                    -3207LL },
    {        MIN, 10000000LL,     7549LL,        -6962723550621670LL,                 -2574592LL },
    {        MIN,    90000LL,     7549LL,      -773635950069074473LL,                    -4592LL },
    {        MIN,   104729LL,     7549LL,      -664832429472416451LL,                   -77813LL },
    {        MIN,        MAX,    14173LL,                   -14173LL,                   -14173LL },
    {        MIN,    MAX / 2,    14173LL,                   -28346LL,                   -28346LL },
    {        MIN,        MIN,    14173LL,                    14173LL,                        0LL },
    {        MIN,    MIN / 2,    14173LL,                    28346LL,                        0LL },
    {        MIN, 10000000LL,    14173LL,       -13072285187834273LL,                 -7526784LL },
    {        MIN,    90000LL,    14173LL,     -1452476131981585972LL,                   -46784LL },
    {        MIN,   104729LL,    14173LL,     -1248201089271765580LL,                   -98964LL },
    {        MIN,        MAX, 10000000LL,                -10000000LL,                -10000000LL },
    {        MIN,    MAX / 2, 10000000LL,                -20000000LL,                -20000000LL },
    {        MIN,        MIN, 10000000LL,                 10000000LL,                        0LL },
    {        MIN,    MIN / 2, 10000000LL,                 20000000LL,                        0LL },
    {        MIN,        MAX,    90000LL,                   -90000LL,                   -90000LL },
    {        MIN,    MAX / 2,    90000LL,                  -180000LL,                  -180000LL },
    {        MIN,        MIN,    90000LL,                    90000LL,                        0LL },
    {        MIN,    MIN / 2,    90000LL,                   180000LL,                        0LL },
    {        MIN, 10000000LL,    90000LL,       -83010348331692982LL,                 -2720000LL },
    {        MIN,   104729LL,    90000LL,     -7926204616838982733LL,                   -75643LL },
    {        MIN,        MAX,   104729LL,                  -104729LL,                  -104729LL },
    {        MIN,    MAX / 2,   104729LL,                  -209458LL,                  -209458LL },
    {        MIN,        MIN,   104729LL,                   104729LL,                        0LL },
    {        MIN,    MIN / 2,   104729LL,                   209458LL,                        0LL },
    {        MIN, 10000000LL,   104729LL,       -96595453004776381LL,                 -5596032LL },
    {    MIN / 2,        MAX,    MIN / 2,      2305843009213693952LL,      2305843009213693952LL },
    {    MIN / 2,    MAX / 2,    MIN / 2,      4611686018427387905LL,                        1LL },
    {    MIN / 2,        MIN,    MIN / 2,                  MIN / 4LL,                        0LL },
    {    MIN / 2,    MIN / 2,    MIN / 2,                    MIN / 2,                        0LL },
    {    MIN / 2,        MAX,     7549LL,                    -3774LL,     -4611686018427391678LL },
    {    MIN / 2,    MAX / 2,     7549LL,                    -7549LL,                    -7549LL },
    {    MIN / 2,        MIN,     7549LL,                     3774LL,                    MIN / 2 },
    {    MIN / 2,    MIN / 2,     7549LL,                     7549LL,                        0LL },
    {    MIN / 2,     7549LL,     7549LL,                    MIN / 2,                        0LL },
    {    MIN / 2,    14173LL,     7549LL,     -2456333715734731622LL,                    -8690LL },
    {    MIN / 2, 10000000LL,     7549LL,        -3481361775310835LL,                 -1287296LL },
    {    MIN / 2,    90000LL,     7549LL,      -386817975034537236LL,                   -47296LL },
    {    MIN / 2,   104729LL,     7549LL,      -332416214736208225LL,                   -91271LL },
    {    MIN / 2,        MAX,    14173LL,                    -7086LL,     -4611686018427394990LL },
    {    MIN / 2,    MAX / 2,    14173LL,                   -14173LL,                   -14173LL },
    {    MIN / 2,        MIN,    14173LL,                     7086LL,                    MIN / 2 },
    {    MIN / 2,    MIN / 2,    14173LL,                    14173LL,                        0LL },
    {    MIN / 2,     7549LL,    14173LL,     -8658289301784523614LL,                    -1306LL },
    {    MIN / 2,    14173LL,    14173LL,                    MIN / 2,                        0LL },
    {    MIN / 2, 10000000LL,    14173LL,        -6536142593917136LL,                 -8763392LL },
    {    MIN / 2,    90000LL,    14173LL,      -726238065990792986LL,                   -23392LL },
    {    MIN / 2,   104729LL,    14173LL,      -624100544635882790LL,                   -49482LL },
    {    MIN / 2,        MAX, 10000000LL,                 -5000000LL,                 -5000000LL },
    {    MIN / 2,    MAX / 2, 10000000LL,                -10000000LL,                -10000000LL },
    {    MIN / 2,        MIN, 10000000LL,                  5000000LL,                        0LL },
    {    MIN / 2,    MIN / 2, 10000000LL,                 10000000LL,                        0LL },
    {    MIN / 2, 10000000LL, 10000000LL,                    MIN / 2,                        0LL },
    {    MIN / 2,        MAX,    90000LL,                   -45000LL,                   -45000LL },
    {    MIN / 2,    MAX / 2,    90000LL,                   -90000LL,                   -90000LL },
    {    MIN / 2,        MIN,    90000LL,                    45000LL,                        0LL },
    {    MIN / 2,    MIN / 2,    90000LL,                    90000LL,                        0LL },
    {    MIN / 2, 10000000LL,    90000LL,       -41505174165846491LL,                 -1360000LL },
    {    MIN / 2,    90000LL,    90000LL,                    MIN / 2,                        0LL },
    {    MIN / 2,   104729LL,    90000LL,     -3963102308419491366LL,                   -90186LL },
    {    MIN / 2,        MAX,   104729LL,                   -52364LL,     -4611686018427440268LL },
    {    MIN / 2,    MAX / 2,   104729LL,                  -104729LL,                  -104729LL },
    {    MIN / 2,        MIN,   104729LL,                    52364LL,                    MIN / 2 },
    {    MIN / 2,    MIN / 2,   104729LL,                   104729LL,                        0LL },
    {    MIN / 2, 10000000LL,   104729LL,       -48297726502388190LL,                 -7798016LL },
    {    MIN / 2,    90000LL,   104729LL,     -5366414055820910086LL,                   -58016LL },
    {    MIN / 2,   104729LL,   104729LL,                    MIN / 2,                        0LL },
    {     7549LL,        MAX,     7549LL,                        0LL,                 56987401LL },
    {     7549LL,    MAX / 2,     7549LL,                        0LL,                 56987401LL },
    {     7549LL,        MIN,     7549LL,                        0LL,                 56987401LL },
    {     7549LL,    MIN / 2,     7549LL,                        0LL,                 56987401LL },
    {     7549LL,     7549LL,     7549LL,                     7549LL,                        0LL },
    {     7549LL,    14173LL,     7549LL,                     4020LL,                    11941LL },
    {     7549LL, 10000000LL,     7549LL,                        5LL,                  6987401LL },
    {     7549LL,    90000LL,     7549LL,                      633LL,                    17401LL },
    {     7549LL,   104729LL,     7549LL,                      544LL,                    14825LL },
    {     7549LL,        MAX,    14173LL,                        0LL,                106991977LL },
    {     7549LL,    MAX / 2,    14173LL,                        0LL,                106991977LL },
    {     7549LL,        MIN,    14173LL,                        0LL,                106991977LL },
    {     7549LL,    MIN / 2,    14173LL,                        0LL,                106991977LL },
    {     7549LL,     7549LL,    14173LL,                    14173LL,                        0LL },
    {     7549LL,    14173LL,    14173LL,                     7549LL,                        0LL },
    {     7549LL, 10000000LL,    14173LL,                       10LL,                  6991977LL },
    {     7549LL,    90000LL,    14173LL,                     1188LL,                    71977LL },
    {     7549LL,   104729LL,    14173LL,                     1021LL,                    63668LL },
    {     7549LL,        MAX, 10000000LL,                        0LL,              75490000000LL },
    {     7549LL,    MAX / 2, 10000000LL,                        0LL,              75490000000LL },
    {     7549LL,        MIN, 10000000LL,                        0LL,              75490000000LL },
    {     7549LL,    MIN / 2, 10000000LL,                        0LL,              75490000000LL },
    {     7549LL,     7549LL, 10000000LL,                 10000000LL,                        0LL },
    {     7549LL,    14173LL, 10000000LL,                  5326324LL,                     9948LL },
    {     7549LL, 10000000LL, 10000000LL,                     7549LL,                        0LL },
    {     7549LL,    90000LL, 10000000LL,                   838777LL,                    70000LL },
    {     7549LL,   104729LL, 10000000LL,                   720812LL,                    80052LL },
    {     7549LL,        MAX,    90000LL,                        0LL,                679410000LL },
    {     7549LL,    MAX / 2,    90000LL,                        0LL,                679410000LL },
    {     7549LL,        MIN,    90000LL,                        0LL,                679410000LL },
    {     7549LL,    MIN / 2,    90000LL,                        0LL,                679410000LL },
    {     7549LL,     7549LL,    90000LL,                    90000LL,                        0LL },
    {     7549LL,    14173LL,    90000LL,                    47936LL,                    13072LL },
    {     7549LL, 10000000LL,    90000LL,                       67LL,                  9410000LL },
    {     7549LL,    90000LL,    90000LL,                     7549LL,                        0LL },
    {     7549LL,   104729LL,    90000LL,                     6487LL,                    32977LL },
    {     7549LL,        MAX,   104729LL,                        0LL,                790599221LL },
    {     7549LL,    MAX / 2,   104729LL,                        0LL,                790599221LL },
    {     7549LL,        MIN,   104729LL,                        0LL,                790599221LL },
    {     7549LL,    MIN / 2,   104729LL,                        0LL,                790599221LL },
    {     7549LL,     7549LL,   104729LL,                   104729LL,                        0LL },
    {     7549LL,    14173LL,   104729LL,                    55782LL,                      935LL },
    {     7549LL, 10000000LL,   104729LL,                       79LL,                   599221LL },
    {     7549LL,    90000LL,   104729LL,                     8784LL,                    39221LL },
    {     7549LL,   104729LL,   104729LL,                     7549LL,                        0LL },
    {    14173LL,        MAX,    14173LL,                        0LL,                200873929LL },
    {    14173LL,    MAX / 2,    14173LL,                        0LL,                200873929LL },
    {    14173LL,        MIN,    14173LL,                        0LL,                200873929LL },
    {    14173LL,    MIN / 2,    14173LL,                        0LL,                200873929LL },
    {    14173LL,     7549LL,    14173LL,                    26609LL,                     2588LL },
    {    14173LL,    14173LL,    14173LL,                    14173LL,                        0LL },
    {    14173LL, 10000000LL,    14173LL,                       20LL,                   873929LL },
    {    14173LL,    90000LL,    14173LL,                     2231LL,                    83929LL },
    {    14173LL,   104729LL,    14173LL,                     1918LL,                     3707LL },
    {    14173LL,        MAX, 10000000LL,                        0LL,             141730000000LL },
    {    14173LL,    MAX / 2, 10000000LL,                        0LL,             141730000000LL },
    {    14173LL,        MIN, 10000000LL,                        0LL,             141730000000LL },
    {    14173LL,    MIN / 2, 10000000LL,                        0LL,             141730000000LL },
    {    14173LL,     7549LL, 10000000LL,                 18774672LL,                     1072LL },
    {    14173LL,    14173LL, 10000000LL,                 10000000LL,                        0LL },
    {    14173LL, 10000000LL, 10000000LL,                    14173LL,                        0LL },
    {    14173LL,    90000LL, 10000000LL,                  1574777LL,                    70000LL },
    {    14173LL,   104729LL, 10000000LL,                  1353302LL,                    34842LL },
    {    14173LL,        MAX,    90000LL,                        0LL,               1275570000LL },
    {    14173LL,    MAX / 2,    90000LL,                        0LL,               1275570000LL },
    {    14173LL,        MIN,    90000LL,                        0LL,               1275570000LL },
    {    14173LL,    MIN / 2,    90000LL,                        0LL,               1275570000LL },
    {    14173LL,     7549LL,    90000LL,                   168972LL,                      372LL },
    {    14173LL,    14173LL,    90000LL,                    90000LL,                        0LL },
    {    14173LL, 10000000LL,    90000LL,                      127LL,                  5570000LL },
    {    14173LL,    90000LL,    90000LL,                    14173LL,                        0LL },
    {    14173LL,   104729LL,    90000LL,                    12179LL,                    75509LL },
    {    14173LL,        MAX,   104729LL,                        0LL,               1484324117LL },
    {    14173LL,    MAX / 2,   104729LL,                        0LL,               1484324117LL },
    {    14173LL,        MIN,   104729LL,                        0LL,               1484324117LL },
    {    14173LL,    MIN / 2,   104729LL,                        0LL,               1484324117LL },
    {    14173LL,     7549LL,   104729LL,                   196625LL,                     1992LL },
    {    14173LL,    14173LL,   104729LL,                   104729LL,                        0LL },
    {    14173LL, 10000000LL,   104729LL,                      148LL,                  4324117LL },
    {    14173LL,    90000LL,   104729LL,                    16492LL,                    44117LL },
    {    14173LL,   104729LL,   104729LL,                    14173LL,                        0LL },
    { 10000000LL,        MAX, 10000000LL,                        0LL,          100000000000000LL },
    { 10000000LL,    MAX / 2, 10000000LL,                        0LL,          100000000000000LL },
    { 10000000LL,        MIN, 10000000LL,                        0LL,          100000000000000LL },
    { 10000000LL,    MIN / 2, 10000000LL,                        0LL,          100000000000000LL },
    { 10000000LL,     7549LL, 10000000LL,              13246787653LL,                     7503LL },
    { 10000000LL,    14173LL, 10000000LL,               7055669230LL,                     3210LL },
    { 10000000LL, 10000000LL, 10000000LL,                 10000000LL,                        0LL },
    { 10000000LL,    90000LL, 10000000LL,               1111111111LL,                    10000LL },
    { 10000000LL,   104729LL, 10000000LL,                954845362LL,                    83102LL },
    { 10000000LL,        MAX,    90000LL,                        0LL,             900000000000LL },
    { 10000000LL,    MAX / 2,    90000LL,                        0LL,             900000000000LL },
    { 10000000LL,        MIN,    90000LL,                        0LL,             900000000000LL },
    { 10000000LL,    MIN / 2,    90000LL,                        0LL,             900000000000LL },
    { 10000000LL,     7549LL,    90000LL,                119221088LL,                     6688LL },
    { 10000000LL,    14173LL,    90000LL,                 63501023LL,                     1021LL },
    { 10000000LL, 10000000LL,    90000LL,                    90000LL,                        0LL },
    { 10000000LL,    90000LL,    90000LL,                 10000000LL,                        0LL },
    { 10000000LL,   104729LL,    90000LL,                  8593608LL,                    27768LL },
    { 10000000LL,        MAX,   104729LL,                        0LL,            1047290000000LL },
    { 10000000LL,    MAX / 2,   104729LL,                        0LL,            1047290000000LL },
    { 10000000LL,        MIN,   104729LL,                        0LL,            1047290000000LL },
    { 10000000LL,    MIN / 2,   104729LL,                        0LL,            1047290000000LL },
    { 10000000LL,     7549LL,   104729LL,                138732282LL,                     3182LL },
    { 10000000LL,    14173LL,   104729LL,                 73893318LL,                     3986LL },
    { 10000000LL, 10000000LL,   104729LL,                   104729LL,                        0LL },
    { 10000000LL,    90000LL,   104729LL,                 11636555LL,                    50000LL },
    { 10000000LL,   104729LL,   104729LL,                 10000000LL,                        0LL },
    {    90000LL,        MAX,    90000LL,                        0LL,               8100000000LL },
    {    90000LL,    MAX / 2,    90000LL,                        0LL,               8100000000LL },
    {    90000LL,        MIN,    90000LL,                        0LL,               8100000000LL },
    {    90000LL,    MIN / 2,    90000LL,                        0LL,               8100000000LL },
    {    90000LL,     7549LL,    90000LL,                  1072989LL,                     6039LL },
    {    90000LL,    14173LL,    90000LL,                   571509LL,                     2943LL },
    {    90000LL, 10000000LL,    90000LL,                      810LL,                        0LL },
    {    90000LL,    90000LL,    90000LL,                    90000LL,                        0LL },
    {    90000LL,   104729LL,    90000LL,                    77342LL,                    49682LL },
    {    90000LL,        MAX,   104729LL,                        0LL,               9425610000LL },
    {    90000LL,    MAX / 2,   104729LL,                        0LL,               9425610000LL },
    {    90000LL,        MIN,   104729LL,                        0LL,               9425610000LL },
    {    90000LL,    MIN / 2,   104729LL,                        0LL,               9425610000LL },
    {    90000LL,     7549LL,   104729LL,                  1248590LL,                     4090LL },
    {    90000LL,    14173LL,   104729LL,                   665039LL,                    12253LL },
    {    90000LL, 10000000LL,   104729LL,                      942LL,                  5610000LL },
    {    90000LL,    90000LL,   104729LL,                   104729LL,                        0LL },
    {    90000LL,   104729LL,   104729LL,                    90000LL,                        0LL },
    {   104729LL,        MAX,   104729LL,                        0LL,              10968163441LL },
    {   104729LL,    MAX / 2,   104729LL,                        0LL,              10968163441LL },
    {   104729LL,        MIN,   104729LL,                        0LL,              10968163441LL },
    {   104729LL,    MIN / 2,   104729LL,                        0LL,              10968163441LL },
    {   104729LL,     7549LL,   104729LL,                  1452929LL,                     2420LL },
    {   104729LL,    14173LL,   104729LL,                   773877LL,                     4720LL },
    {   104729LL, 10000000LL,   104729LL,                     1096LL,                  8163441LL },
    {   104729LL,    90000LL,   104729LL,                   121868LL,                    43441LL },
    {   104729LL,   104729LL,   104729LL,                   104729LL,                        0LL },
};

static const size_t g_cValidEntries = sizeof(g_rgValidEntries)/sizeof(g_rgValidEntries[0]);


////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( TimeSpanTests )
{
    //
    // This checks the conversions for all combinations of values that are
    // a power of 2, positive and negative, for tick count, source frequency
    // and destination frequency.
    //

    PKTEST_METHOD( AllPowersOf2 )
    {
        SSPK::TimeSpanGeneric fixture;

        int64_t remainder;
        int iteration = 0;

        //
        // Check all the combinations of powers of 2
        //

        PowerOf2 a;

        while( a.MoveNext() )
        {
            PowerOf2 b;

            while( b.MoveNext() )
            {
                PowerOf2 div;

                while( div.MoveNext() )
                {
                    ++iteration;

                    int exponentOfResult = a.exponent + b.exponent - div.exponent;

                    pkRESULT pkr = fixture.Set( a.value, div.value, b.value, &remainder );

                    if( exponentOfResult < 0 )
                    {
                        // The result is fractional,
                        // so there's a remainder

                        int exponentOfRemainder = a.exponent + b.exponent;

                        int64_t expectedRemanider =
                                    ( 1LL << exponentOfRemainder )
                                    * ( a.signal ) * ( b.signal ) * ( div.signal );

                        PKTEST_ASSERT_EXIT( pkSUCCEEDED( pkr ) );
                        PKTEST_ASSERT_EXIT( fixture.Ticks() == 0 );
                        PKTEST_ASSERT_EXIT( fixture.TicksPerSecond() == b.value );

                        PKTEST_ASSERT_EXIT( remainder == expectedRemanider );
                    }
                    else if( exponentOfResult < 63 )
                    {
                        // The result is a signed integer that fits into a int64_t

                        int64_t expectedResult =
                                    ( 1LL << exponentOfResult )
                                    * ( a.signal ) * ( b.signal ) * ( div.signal );

                        PKTEST_ASSERT_EXIT( pkSUCCEEDED( pkr ) );
                        PKTEST_ASSERT_EXIT( fixture.Ticks() == expectedResult );
                        PKTEST_ASSERT_EXIT( fixture.TicksPerSecond() == b.value );
                        PKTEST_ASSERT_EXIT( remainder == 0 );
                    }
                    else
                    {
                        // The result overflows an int64_t.
                        // On overflow expect the value to be set to maximum

                        int64_t expectedResult =
                                    ( ( a.signal * b.signal * div.signal ) > 0 )
                                        ? (int64_t)fixture.MAX_TICKS
                                        : (int64_t)fixture.MIN_TICKS;

                        PKTEST_ASSERT_EXIT( FAILED( pkr ) );
                        PKTEST_ASSERT_EXIT( fixture.Ticks() == expectedResult );
                        PKTEST_ASSERT_EXIT( fixture.TicksPerSecond() == b.value );
                    }
                }
            }
        }

    exit:

        return;
    }

    PKTEST_METHOD( Operators )
    {
        using namespace SSPK;

        static const int64_t values[] = { -10, -20, 0, 20, 10 };

        for( int ia = 0; ia < sizeof(values)/sizeof(values[0]); ++ia )
        {
            for( int ib = 0; ib < sizeof(values)/sizeof(values[0]); ++ib )
            {
                int64_t a = values[ia];
                int64_t b = values[ib];

                TimeSpan_NTP ta = TimeSpan_NTP::FromTicks( a );
                TimeSpan_NTP tb = TimeSpan_NTP::FromTicks( b );

                // Comparators

                PKTEST_ASSERT_EXIT( ( a < b ) == ( ta < tb ) );
                PKTEST_ASSERT_EXIT( ( a > b ) == ( ta > tb ) );
                PKTEST_ASSERT_EXIT( ( a <= b ) == ( ta <= tb ) );
                PKTEST_ASSERT_EXIT( ( a >= b ) == ( ta >= tb ) );
                PKTEST_ASSERT_EXIT( ( a == b ) == ( ta == tb ) );
                PKTEST_ASSERT_EXIT( ( a != b ) == ( ta != tb ) );

                // Arithmetic

                PKTEST_ASSERT_EXIT( ( a + b ) == ( ta + tb ).Ticks() );
                PKTEST_ASSERT_EXIT( ( a - b ) == ( ta - tb ).Ticks() );
            }
        }

    exit:
        return;
    }

    PKTEST_METHOD( InstantiationThroughTheFromMethod )
    {
        // Nothing too fancy here, just the minimum to ensure
        // the From method works well enough for a fixed destination frequency.

        using namespace SSPK;

        int cConversions = 0;

        for( int i = 0; i < g_cValidEntries; ++i )
        {
            TimeSpan_hns fixture;

            if( g_rgValidEntries[i].toTicksPerSecond != fixture.TicksPerSecond() )
            {
                continue;
            }

            fixture = TimeSpan_hns::ConvertFrom( g_rgValidEntries[i].fromTicks, g_rgValidEntries[i].fromTicksPerSecond );

            PKTEST_ASSERT_EXIT( fixture.Ticks() == g_rgValidEntries[i].expectedTicks );

            ++cConversions;
        }

        PKTEST_ASSERT_EXIT( cConversions > 10 );    // expect at least 10 conversions

    exit:
        return;
    }

    PKTEST_METHOD( ValidConversions )
    {
        for( int i = 0; i < g_cValidEntries; ++i )
        {
            SSPK::TimeSpanGeneric fixture;
            int64_t remainder;

            PKTEST_HRESULT_EXIT(
                    fixture.Set(
                                g_rgValidEntries[i].fromTicks,
                                g_rgValidEntries[i].fromTicksPerSecond,
                                g_rgValidEntries[i].toTicksPerSecond,
                                &remainder )
                                );

            PKTEST_ASSERT_EXIT( fixture.Ticks() == g_rgValidEntries[i].expectedTicks );
        }

    exit:
        return;
    }
};
