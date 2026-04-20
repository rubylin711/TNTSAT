///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include <string>
#include <time.h>

using namespace std;

namespace SSPKTest
{
    static const double FIRST_RANDOM_WITHOUT_TIME = 41;
    class STUtilities
    {
    public:
        STUtilities()
        {
        }

        template <class T>
        const static T Rand( T nMax )
        {
            return Rand((T)0, nMax);
        }

        template <class T>
        const static T Rand( T first, T second )
        {
            if(first < second)
            {
                return RandomIndex( second - first) + first;
            }
            return RandomIndex( first - second) + second;

        }

        template <class T>
        const static T RandomIndex( T nMax )
        {
            if(nMax == 0)
            {
                return 0;
            }
            else
            {
                double d = rand();
                if(d == FIRST_RANDOM_WITHOUT_TIME)
                {
                    srand( (unsigned int)time( NULL ) );
                    d = rand();
                }
                d = ( d / RAND_MAX ) * ( nMax ) ;

                return (T)d % nMax;
            }
        }

        const static int64_t GetSystemTime();
        const static string ConvertToTimeString(int64_t time);
    };
};

