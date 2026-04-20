///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MP4Parser.h"
#include "MP4Feed.h"
#include "MP4Atom.h"
#include "Trace.h"

#include "MarshallingUtils.h"

//#define MP4PARSER_SPEW
#ifdef MP4PARSER_SPEW
#define MP4PARSER_TRACE(x) TRACE(x)
#else
#define MP4PARSER_TRACE(x)
#endif

BaseMP4Parser::BaseMP4Parser(_In_ MP4Atom::EAtomType stopAtom)
    : _feedType(MP4Feed::MP4Feed_None)
    , _feedSource(0)
    , _feedLength(0)
    , _stopAtom(stopAtom)
{
}

void BaseMP4Parser::Init(_In_ MP4Feed::MP4FeedType type, _In_ void* source, _In_ uint32 length)
{
    _feedType = type;
    _feedSource = source;
    _feedLength = length;
}

bool BaseMP4Parser::ParseInternal(_Out_ BaseMP4Info* info)
{
    if (NULL == _feedSource || NULL == info)
    {
        ASSERT(false);
        return false;
    }

    //Create feed object
    MP4Feed* feed = MP4Feed::CreateMP4Feed(_feedType, _feedSource, _feedLength);
    if (NULL == feed)
    {
        return false;
    }

    MP4Atom* mp4Atom = NEW_NO_THROW MP4Atom(feed, info, true, false); // root atom has children
    CHECK_ALLOC(mp4Atom);
    if (NULL == mp4Atom)
    {
        delete feed;
        return false;
    }

    //Prepare to parse
    mp4Atom->SetStopOnAtom(_stopAtom);

    //Parse ...
    bool ret = mp4Atom->Parse();

    // how many bytes we've parsed
    info->_bytesParsed = feed->TotalReadBytes();

    //Don't need the atom object anymore
    delete mp4Atom;

    //Don't need the feed anymore
    delete feed;

    return ret;
}

MP4Parser::MP4Parser()
    : BaseMP4Parser(MP4Atom::eAtomType_moov)
{
}

void MP4Parser::Init(_In_ MP4Feed::MP4FeedType type, _In_ void* source, _In_ uint32 length)
{
    _info.Cleanup();
    BaseMP4Parser::Init(type, source, length);
}

bool MP4Parser::Parse()
{
    return ParseInternal(&_info);
}

void MP4Parser::Prepare(_In_ MP4Atom* mp4Atom)
{
    CHECK_ALLOC(mp4Atom);
    mp4Atom->SetStopOnAtom(MP4Atom::eAtomType_moov);
}

FMP4Parser::FMP4Parser(_In_ uint32 fragmentMediaType)
    : BaseMP4Parser(MP4Atom::eAtomType_moof)
    , _info(fragmentMediaType)
{
}

void FMP4Parser::Init(_In_ MP4Feed::MP4FeedType type, _In_ void* source, _In_ uint32 length)
{
    _info.Cleanup();
    BaseMP4Parser::Init(type, source, length);
}

bool FMP4Parser::Parse()
{
    uint8* srcStart = (uint8*)_feedSource;
    uint32 srcLen = _feedLength;

    return ParseInternal(&_info) && ParseDrmData(srcStart, srcLen);
}

void FMP4Parser::Prepare(MP4Atom* mp4Atom)
{
}

//Fetch the data via streamer up to the specified atomType, into the buffer.
//It will stop at the beginning of the actual data of the specified atom,
//(i.e. the atom-size and atom-type will be fetched into the buffer.)
//or when the incoming buffer is completely filled, whichever condition
//happens first.
uint32 FMP4Parser::Prefetch(_In_ MP4Atom::EAtomType atomType,
                            _In_ MP4Streamer* streamer,
                            _Out_bytecap_post_bytecount_(length,fetched) uint8* buffer,
                            _In_ uint32 length,
                            _Out_ uint32* fetched)
{
    uint32 offset = 0;
    bool atomFound = false;

    uint32 boxType;
    uint32 boxSize = 0;

    if (NULL == streamer || NULL == buffer || NULL == fetched)
    {
        ASSERT(false);
        return false;
    }

    *fetched = 0;

    // This pre-parser loop is needed to buffer up to the mdat header to support sample streaming
    // This handles out-of-order boxes, but the mdat box must still be the last box in the chunk
    while (offset < length)
    {
        if (streamer->RecvCount(&buffer[offset], length - offset, 8) != 8)
            break;

        boxSize = BigEndian::BytesToHost<uint32,4>( &buffer[offset] );
        offset += 4;

        boxType = BigEndian::BytesToHost<uint32,4>( &buffer[offset] );
        offset += 4;

        if (boxSize == 1) // <large-size> syntax
        {
            uint64 boxSize64;
            if (streamer->RecvCount(&buffer[offset], length - offset, 8) != 8)
                break;

            boxSize64 = BigEndian::BytesToHost<uint64,8>( &buffer[offset] );
            offset += 8;

            // we can only handle compact size within a large size
            if ((boxSize64 >> 32) == 0 && boxSize64 > 8)
                boxSize = (uint32)boxSize64 - 8;
            else
                break;
        }

        if (boxType == (uint32) atomType)
        {
            atomFound = true;
            break;
        }

        if (boxSize < 8 || boxSize > length - offset + 8)
            break;    // invalid box or box size larger than buffer size

        boxSize -= 8;
        if (streamer->RecvCount(&buffer[offset], length - offset, boxSize) != (int32) boxSize)
            break;
        offset += boxSize;
    }

    *fetched = offset;

    return (atomFound ? boxSize : 0);
}

// After the moof has been parsed,ParseDrmData will re-parse the moof to get the drm data
bool FMP4Parser::ParseDrmData(_In_bytecount_(moofBoxSize) const uint8* moofBoxStartPos, _In_ uint32 moofBoxSize)
{
    // parsing DrmData is only done when a parse is called on a moof box, confirm this is a moof box
    uint32 boxType = BigEndian::BytesToHost<uint32,4>( moofBoxStartPos + sizeof(uint32) );

    if(MP4Atom::eAtomType_moof != boxType)
    {
        return true;
    }

    FMP4TrackInfo* track = (FMP4TrackInfo*)_info.GetTrackInfo();

    if (NULL == track)
    {
        TRACE_ERROR(("ParseDrmData: failed to get track"));
        return false;
    }

    if (NULL == track->_se_info)
    {
        MP4PARSER_TRACE(("ParseDrmData: skipping since se_info is not there"));
        return true;
    }

    // don't continue if sample aux info boxes don't exist
    if ( (NULL == track->_se_info->_sampleAuxInfoSizeData) ||
         (NULL == track->_se_info->_sampleAuxInfoOffsetData) )
    {
        // use the data out of the senc/uuid box
        if ( track->_se_info->_sub_sample_table )
        {
            MP4PARSER_TRACE(("ParseDrmData: skipping since subsample table already exists"));
            return true;
        }
        else
        {
            if(track->_se_info->_cencSampleGroupDescriptionData)
            {
                uint32 entryCount = track->_se_info->_cencSampleGroupDescriptionData->_entryCount;
                for (uint32 i = 0; i < entryCount; i++)
                {
                    if(track->_se_info->_cencSampleGroupDescriptionData->_entries[i]->_algId != 0)
                    {
                        TRACE_ERROR(("ParseDrmData: failed saio/saiz does not exist and sgpd algorithmId is non-zero %d",
                            track->_se_info->_cencSampleGroupDescriptionData->_entries[i]->_algId));
                        return false;
                    }
                }
            }
            MP4PARSER_TRACE(("ParseDrmData: skipping since there is no saio/saiz data"));
            return true;
        }
    }

    // there should only be one offset
    if (track->_se_info->_sampleAuxInfoOffsetData->_entryCount != 1)
    {
        TRACE_ERROR(("ParseDrmData: saio box has %d entries, but it must have only one entry",
                   track->_se_info->_sampleAuxInfoOffsetData->_entryCount));
        return false;
    }

    uint64 sampleAuxInfoOffsetData = 0;
    
    if ( NULL != track->_se_info->_sampleAuxInfoOffsetData->_offsets )
    {
        sampleAuxInfoOffsetData = track->_se_info->_sampleAuxInfoOffsetData->_offsets[0];
    }
    else if ( NULL != track->_se_info->_sampleAuxInfoOffsetData->_loffsets )
    {
        sampleAuxInfoOffsetData = track->_se_info->_sampleAuxInfoOffsetData->_loffsets[0];
    }

    if ( track->_se_info->_sub_sample_table )
    {
        // check if the subSample data parsed out of senc is the same data as the data point to by saio
        if ( (track->_se_info->_senc_sub_sample_startPos > moofBoxStartPos) &&
             ((uint64)(track->_se_info->_senc_sub_sample_startPos - moofBoxStartPos) == sampleAuxInfoOffsetData) )
        {
            MP4PARSER_TRACE(("ParseDrmData: skipping since subsample table has the same data"));
            return true;
        }
        else
        {
            // saio points to different data, need to re-parse
             MP4PARSER_TRACE(("ParseDrmData: reparsing subsample table since saio %p is different from senc %p",
                 sampleAuxInfoOffsetData, track->_se_info->_senc_sub_sample_startPos - moofBoxStartPos));
            delete[] track->_se_info->_sub_sample_table;
        }
    }

    std::vector<CencSampleEncryptionInformationAudioGroupEntry*> sampleEncryptionInfoEntries;
    std::vector<int32> sampleEncryptionInfoIndexes;
    if(!track->GetSampleEncryptionInfo(&sampleEncryptionInfoEntries, &sampleEncryptionInfoIndexes))
    {
        TRACE_ERROR(("ParseDrmData: failed to get SampleEncryptionInfo"));
        return false;
    }

    // validate the sampleCount
    if ( !sampleEncryptionInfoIndexes.empty())
    {
        if (sampleEncryptionInfoIndexes.size() != track->_se_info->_sampleAuxInfoSizeData->_sampleCount)
        {
            TRACE_ERROR(("ParseDrmData: Sample count in saiz %d and sbgp %d box is not equal",
                    track->_se_info->_sampleAuxInfoSizeData->_sampleCount,
                    sampleEncryptionInfoIndexes.size()));
            return false;
        }
    }
    track->_se_info->_sample_count = track->_se_info->_sampleAuxInfoSizeData->_sampleCount;

    // determine if subSample encryption is used and get the total sampleInfo size
    bool useSubSampleEncryption = false;
    uint32 totalSampleInfoSize = 0;
    for (uint32 i = 0; i < track->_se_info->_sample_count; ++i)
    {
        // figure out IV size
        uint8 sampleIdentifierSize = track->_se_info->_iv_size;
        if( !sampleEncryptionInfoIndexes.empty() )
        {
            sampleIdentifierSize = sampleEncryptionInfoEntries[sampleEncryptionInfoIndexes[i]]->_ivSize;
        }

        if ( 0 == track->_se_info->_sampleAuxInfoSizeData->_defaultSampleInfoSize )
        {
            totalSampleInfoSize += track->_se_info->_sampleAuxInfoSizeData->_sampleInfoSize[i];
            if ( track->_se_info->_sampleAuxInfoSizeData->_sampleInfoSize[i] > sampleIdentifierSize)
            {
                useSubSampleEncryption = true;
            }
        }
        else
        {
            totalSampleInfoSize += track->_se_info->_sampleAuxInfoSizeData->_defaultSampleInfoSize;
            if ( track->_se_info->_sampleAuxInfoSizeData->_defaultSampleInfoSize > sampleIdentifierSize)
            {
                useSubSampleEncryption = true;
            }
        }
    }

    // validate that the total SampleInfo is not beyond the moof box
    if(totalSampleInfoSize > moofBoxSize)
    {
        TRACE_ERROR(("ParseDrmData: totalSampleInfoSize %d greater than moofBoxSize %d", totalSampleInfoSize, moofBoxSize));
        return false;
    }

    // create subSample table
    track->_se_info->_sub_sample_table = NEW_NO_THROW SampleEncryptionSubSampleInfo[track->_se_info->_sample_count];
    CHECK_ALLOC(track->_se_info->_sub_sample_table);
    if (NULL == track->_se_info->_sub_sample_table)
    {
        TRACE_ERROR(("ParseDrmData: failed to allocate SampleEncryptionSubSampleInfo"));
        return false;
    }

    // move to the offset position
    const uint8* currentPosition = moofBoxStartPos + sampleAuxInfoOffsetData;

    SampleEncryptionSubSampleInfo* subsample_info;
    uint32 i;
    for (i = 0, subsample_info = track->_se_info->_sub_sample_table; i < track->_se_info->_sample_count; ++i, ++subsample_info)
    {
        // figure out the sampleInfo size
        uint32 sampleInfoSize = track->_se_info->_sampleAuxInfoSizeData->_defaultSampleInfoSize;
        if( 0 == sampleInfoSize )
        {
            sampleInfoSize = track->_se_info->_sampleAuxInfoSizeData->_sampleInfoSize[i];
        }

        uint32 dataOffset = 0;

        if (sampleEncryptionInfoIndexes.empty() || sampleEncryptionInfoEntries[sampleEncryptionInfoIndexes[i]]->_algId != 0)
        {
            uint8 sampleIdentifierSize = track->_se_info->_iv_size;
            if (!sampleEncryptionInfoIndexes.empty())
            {
                sampleIdentifierSize = sampleEncryptionInfoEntries[sampleEncryptionInfoIndexes[i]]->_ivSize;
            }

            if (sampleInfoSize < sampleIdentifierSize)
            {
                TRACE_ERROR(("ParseDrmData: SampleInfoSize %d in saiz must not be less than IV size %d",
                        sampleInfoSize,
                        sampleIdentifierSize));

                return false;
            }

            if(sampleIdentifierSize == 0)
            {
                TRACE_ERROR(("ParseDrmData: sampleIdentifierSize is zero"));
                return false;
            }

            if (!subsample_info->AllocateIV(sampleIdentifierSize))
            {
                TRACE_ERROR(("ParseDrmData: failed to allocate subsample IV"));
                return false;
            }

            // get the identifier

            subsample_info->_iv = BigEndian::BytesToHost<uint64>( currentPosition + dataOffset, sampleIdentifierSize );
            dataOffset += sampleIdentifierSize;

            MP4PARSER_TRACE(("ParseDrmData IV: 0x%llx IV_Size: %u", subsample_info->_iv, sampleIdentifierSize));

            if( useSubSampleEncryption )
            {
                // get the entry count
                subsample_info->_entry_count = BigEndian::BytesToHost<uint16,2>( currentPosition + dataOffset );
                dataOffset += sizeof(uint16);

                if( !subsample_info->AllocateEntries(subsample_info->_entry_count) )
                {
                    TRACE_ERROR(("ParseDrmData: failed to allocate subsample entries"));
                    return false;
                }

                // get the clear and encrypted lengths
                for (int32 j=0; j<subsample_info->_entry_count; ++j)
                {
                    subsample_info->_clear_data[j] = BigEndian::BytesToHost<uint16,2>( currentPosition + dataOffset );
                    dataOffset += sizeof(uint16);

                    subsample_info->_encrypted_data[j] = BigEndian::BytesToHost<uint32,4>( currentPosition + dataOffset );
                    dataOffset += sizeof(uint32);

                    MP4PARSER_TRACE(("ParseDrmData clear: 0x%x encrypted: 0x%x", subsample_info->_clear_data[j], subsample_info->_encrypted_data[j]));
                }
            }
        }
        else
        {
            dataOffset += sampleInfoSize;
        }

        //check if entire sampleInfoSize has been consumed
        if(dataOffset == sampleInfoSize)
        {
            currentPosition += sampleInfoSize;
        }
        else
        {
            TRACE_ERROR(("ParseDrmData: only %d bytes consumed when sampleInfoSize %d", dataOffset, sampleInfoSize));
            return false;
        }
    }

    return true;
}

