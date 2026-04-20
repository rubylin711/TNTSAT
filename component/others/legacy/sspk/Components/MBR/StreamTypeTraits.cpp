///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MediaStreamType.h"
#include "StreamTypeTraits.h"

const STREAM_TYPE_TRAITS* GetStreamTypeTraits( _In_ MediaStreamType type )
{
    switch( type )
    {
    case MediaStreamTypeAudio:
        {
            static const STREAM_TYPE_TRAITS _audioStreamTraits =
            {
                true,           // isValid
                L"audio",       // pszDefaultName
                false,          // canBeSparse
                false,          // requiresFourCC
                true,           // requiresBitrate
            };

            return( &_audioStreamTraits );
        }

    case MediaStreamTypeVideo:
        {
            static const STREAM_TYPE_TRAITS _videoStreamTraits =
            {
                true,           // isValid
                L"video",       // pszDefaultName
                false,          // canBeSparse
                true,           // requiresFourCC
                true,           // requiresBitrate
            };

            return( &_videoStreamTraits );
        }

    case MediaStreamTypeText:
        {
            static const STREAM_TYPE_TRAITS _textStreamTraits =
            {
                true,           // isValid
                L"text",        // pszDefaultName
                true,           // canBeSparse
                false,          // requiresFourCC
                false,          // requiresBitrate
            };

            return( &_textStreamTraits );
        }

    case MediaStreamTypeBinary:
        {
            static const STREAM_TYPE_TRAITS _binaryStreamTraits =
            {
                true,           // isValid
                L"binary",      // pszDefaultName
                true,           // canBeSparse
                false,          // requiresFourCC
                false,          // requiresBitrate
            };

            return( &_binaryStreamTraits );
        }

    default:
        {
            static const STREAM_TYPE_TRAITS _invalidStreamTrais =
            {
                false,          // isValid
                L"",            // pszDefaultName
                false,          // canBeSparse
                false,          // requiresFourCC
                false,          // requiresBitrate
            };

            return( &_invalidStreamTrais );
        }
    }
}

