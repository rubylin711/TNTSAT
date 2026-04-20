///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MP4Info.h"
#include "Utils.h"

//#define DEBUG_SPEW_FRAME_INFO
#ifdef DEBUG_SPEW_FRAME_INFO
#define TRACE_TABLE(x) TRACE(x)
#else
#define TRACE_TABLE(x)
#endif

// #define DEBUG_INVALID_MP4_DATA
#ifdef DEBUG_INVALID_MP4_DATA
#define MP4_DEBUG_VALIDATE(x) ASSERT(x)
#else
#define MP4_DEBUG_VALIDATE(x)
#endif


SampleEncryptionSubSampleInfo::SampleEncryptionSubSampleInfo()
    : _iv_size(0)
    , _iv(0)
    , _entry_count(0)
    , _clear_data(NULL)
    , _encrypted_data(NULL)
{
}

SampleEncryptionSubSampleInfo::~SampleEncryptionSubSampleInfo()
{
    delete[] _encrypted_data;
    delete[] _clear_data;
}

bool SampleEncryptionSubSampleInfo::AllocateIV(uint8 iv_size)
{
    ASSERT( 0 == _iv_size );

    _iv_size = iv_size;

    if( _iv_size > sizeof(_iv) )
    {
        return( false );
    }

    return true;
}

bool SampleEncryptionSubSampleInfo::AllocateEntries(uint16 entry_count)
{
    ASSERT(NULL == _clear_data);
    ASSERT(NULL == _encrypted_data);

    _entry_count = entry_count;
    if (_entry_count)
    {
        _clear_data = NEW_NO_THROW uint16[_entry_count];
        if (NULL == _clear_data)
        {
            return false;
        }

        _encrypted_data = NEW_NO_THROW uint32[_entry_count];
        if (NULL == _encrypted_data)
        {
            return false;
        }
    }

    return true;
}


SampleGroupingData::SampleGroupingData()
    : _entryCount(0)
    , _entries(NULL)
{
}

SampleGroupingData::~SampleGroupingData()
{
    for (uint32 i = 0; i < _entryCount; ++i)
    {
        delete _entries[i];
    }
    delete[] _entries;
}

CencSampleEncryptionInformationAudioGroupEntry::CencSampleEncryptionInformationAudioGroupEntry()
    : _descriptionLength(0)
    , _algId(0)
    , _ivSize(0)
    , _kid(NULL)
{
}

CencSampleEncryptionInformationAudioGroupEntry::~CencSampleEncryptionInformationAudioGroupEntry()
{
    delete _kid;
}

SampleGroupDescriptionData::SampleGroupDescriptionData()
    : _entryCount(0)
    , _defaultLength(0)
    , _entries(NULL)
{
}

SampleGroupDescriptionData::~SampleGroupDescriptionData()
{
    for (uint32 i = 0; i < _entryCount; ++i)
    {
        delete _entries[i];
    }
    delete[] _entries;
}

SampleAuxiliaryInformationOffsetData::SampleAuxiliaryInformationOffsetData()
    : _entryCount(0)
    , _offsets(NULL)
    , _loffsets(NULL)
{
}

SampleAuxiliaryInformationOffsetData::~SampleAuxiliaryInformationOffsetData()
{
    delete[] _offsets;
    delete[] _loffsets;
}


SampleAuxiliaryInformationSizeData::SampleAuxiliaryInformationSizeData()
    : _defaultSampleInfoSize(0)
    , _sampleCount(0)
    , _sampleInfoSize(NULL)
{
}

SampleAuxiliaryInformationSizeData::~SampleAuxiliaryInformationSizeData()
{
    delete[] _sampleInfoSize;
}

SampleEncryptionInfo::SampleEncryptionInfo()
    : _algorithm_id(0)
    , _iv_size(8) // default value is 8
    , _kid(NULL)
    , _sample_count(0)
    , _senc_sub_sample_startPos(NULL)
    , _sub_sample_table(NULL)
    , _cencSampleGroupingData(NULL)
    , _cencSampleGroupDescriptionData(NULL)
    , _sampleAuxInfoOffsetData(NULL)
    , _sampleAuxInfoSizeData(NULL)
{
}

SampleEncryptionInfo::~SampleEncryptionInfo()
{
    delete[] _sub_sample_table;
    delete _kid;
    delete _cencSampleGroupingData;
    delete _cencSampleGroupDescriptionData;
    delete _sampleAuxInfoOffsetData;
    delete _sampleAuxInfoSizeData;
}



ProtectionSystemSpecificInfo::ProtectionSystemSpecificInfo()
    : _system_id(0)
    , _data_size(0)
    , _data(NULL)
{
}

ProtectionSystemSpecificInfo::~ProtectionSystemSpecificInfo()
{
    delete _system_id;
    delete[] _data;
}

DrmInfo::DrmInfo() : _sample_count(0), _iv_length_table(0), _iv_data_table(0)
{
}

DrmInfo::~DrmInfo()
{
    delete[] _iv_length_table;

    if (NULL != _iv_data_table)
    {
        for (uint32 i=0; i<_sample_count; ++i)
        {
            delete[] _iv_data_table[i];
        }
        delete[] _iv_data_table;
    }
}


MP4FrameInfo::MP4FrameInfo()
    : pts(0)
    , dts(0)
    , duration(0)
    , offset(0)
    , len(0)
    , media_type(kMP4TrackType_None)
    , frame_index((uint32)-1)
    , is_sync(false)
    , is_rap(false)
    , se_info(NULL)
{}



BaseTrackInfo::BaseTrackInfo()
    : _media_type(0)
    , _media_format(0)
    , _frames_count(0)
    , _track_duration(0)
    , _track_time_scale(0)
    , _media_descriptor(0)
    , _media_descriptor_len(0)
    , _channel_count(0)
    , _sample_rate(0)
{
    memset(_media_type_name, 0, sizeof(_media_type_name));
    memset(_media_format_name, 0, sizeof(_media_format_name));
}

BaseTrackInfo::~BaseTrackInfo()
{
    if (_media_descriptor)
        delete[] _media_descriptor;
}

void BaseTrackInfo::SetMediaType(uint32 media_type)
{
    _media_type = media_type;
    INT32TOSTR(_media_type, _media_type_name);
}

void BaseTrackInfo::SetMediaFormat(uint32 media_format)
{
    _media_format = media_format;
    INT32TOSTR(_media_format, _media_format_name);
}

bool MP4TrackInfo::StartRetrieveFrameInfo(uint64 base_ts)
{
    if (_chunk_offset_table.size() == 0 ||
        _sample_size_table.size() == 0 ||
        _sample2chunk_table.size() == 0 ||
        _time2sample_table.size() == 0)
    {
        MP4_DEBUG_VALIDATE(false);
        return false;
    }

    if ((_sample_size_table.size() != 0) && (_sample_size_table.size() != _frames_count))
    {
        MP4_DEBUG_VALIDATE(false);
        return false;
    }

    sample2chunk_idx = 0;
    sc_sample_idx_start = 0;
    sc_sample_idx_end = 0;
    chunk_idx = 0;
    chunk_idx_base = 0;
    chunk_idx_inc = 0;
    chunk_offset = 0; // frame offset within a chunk
    sync_sample_idx = 0;
    time2sample_idx = 0;
    ts_sample_idx_start = 0;
    accumulated_decode_ts = base_ts;
    comp_time2sample_idx = 0;
    cts_sample_idx_start = 0;

    if (_sample2chunk_table.size() > 1)
        sc_sample_idx_end = (_sample2chunk_table.at(1)._first_chunk - _sample2chunk_table.at(0)._first_chunk) * _sample2chunk_table.at(0)._samples_per_chunk;
    else
        sc_sample_idx_end = _frames_count;

    i = 0;

    return true;
}

bool MP4TrackInfo::GetNextFrame(MP4FrameInfo& frame)
{
    if (i >= _frames_count)
        return false;

    // get offset
    if (i < sc_sample_idx_end)
    {
        chunk_idx_inc = (i - sc_sample_idx_start) / _sample2chunk_table.at(sample2chunk_idx)._samples_per_chunk;
        if ((i - sc_sample_idx_start) % _sample2chunk_table.at(sample2chunk_idx)._samples_per_chunk == 0)
            chunk_offset = 0;
    }
    else
    {
        ++sample2chunk_idx;
        sc_sample_idx_start = sc_sample_idx_end;

        if (_sample2chunk_table.size() > sample2chunk_idx+1)
            sc_sample_idx_end = sc_sample_idx_start + (_sample2chunk_table.at(sample2chunk_idx+1)._first_chunk - _sample2chunk_table.at(sample2chunk_idx)._first_chunk) * _sample2chunk_table.at(sample2chunk_idx)._samples_per_chunk;
        else
            sc_sample_idx_end = _frames_count;

        chunk_idx_base = _sample2chunk_table.at(sample2chunk_idx)._first_chunk - 1;
        chunk_idx_inc = 0;

        chunk_offset = 0;
    }

    // chunk index for frame #i
    chunk_idx = chunk_idx_base + chunk_idx_inc;

    // frame offset
    frame.offset = _chunk_offset_table.at(chunk_idx) + chunk_offset;

    // frame size
    if (_sample_size_table.size())
        frame.len = _sample_size_table.at(i);
    else
        frame.len = _single_sample_size;

    // add sample size to chunk_offset, this will be the frame offset for the next frame
    chunk_offset += frame.len;

    // get sync
    if (_sync_sample_table.size())
    {
        if ((sync_sample_idx < _sync_sample_table.size()) && (_sync_sample_table.at(sync_sample_idx) == i+1))
        {
            frame.is_sync = true;
            ++sync_sample_idx;
        }
        else
            frame.is_sync = false;
    }
    else
        frame.is_sync = true;

    // get rap
    frame.is_rap = (0 == chunk_offset);

    // get dts
    frame.dts = accumulated_decode_ts;
    frame.pts = frame.dts;

    // get frame duration
    frame.duration = _time2sample_table.at(time2sample_idx)._sample_duration;

    // add duration of frame #i to accumulated_decode_ts
    accumulated_decode_ts += frame.duration;

    if ((i+1) >= (ts_sample_idx_start + _time2sample_table.at(time2sample_idx)._sample_count))
    {
        ts_sample_idx_start = ts_sample_idx_start + _time2sample_table.at(time2sample_idx)._sample_count;
        ++time2sample_idx;
    }

    // get pts
    if (_comp_time2sample_table.size())
    {
        frame.pts = frame.dts + _comp_time2sample_table.at(comp_time2sample_idx)._samples_offset;

        if ((i+1) >= (cts_sample_idx_start + _comp_time2sample_table.at(comp_time2sample_idx)._sample_count))
        {
            cts_sample_idx_start = cts_sample_idx_start + _comp_time2sample_table.at(comp_time2sample_idx)._sample_count;
            ++comp_time2sample_idx;
        }
    }

    frame.media_type = (MP4TrackType)_media_type;
    frame.frame_index = i;

    ++i;
    return true;
}

FMP4TrackInfo::FMP4TrackInfo()
    : _sequence_number(0)
    , _track_id(0)
    , _base_data_offset(0)
    , _sample_description_index(0)
    , _default_sample_duration(0)
    , _default_sample_size(0)
    , _default_sample_flags(0)
    , _sample_duration_table(NULL)
    , _sample_size_table(NULL)
    , _sample_flags_table(NULL)
    , _sample_comptimeoffset_table(NULL)
    , _sample_table_len(0)
    , _sdtp_table(NULL)
    , _sample_ptso_table(NULL)
    , _se_info(NULL)
    , _pss_info(NULL)
    , _drm_info(NULL)
    , _live_frag_absolute_time(0)
    , _live_frag_duration(0)
    , _live_frag_ntp_timestamp(0)
    , _live_frags_ahead(0)
    , _fragment_absolute_time_table(0)
    , _fragment_duration_table(0)
{
}

FMP4TrackInfo::~FMP4TrackInfo()
{
    if (_drm_info)
        delete _drm_info;

    if (_se_info)
        delete _se_info;

    if (_pss_info)
        delete _pss_info;

    if (_sdtp_table)
        delete [] _sdtp_table;

    if (_sample_ptso_table)
        delete [] _sample_ptso_table;

    ReleaseSampleTable();
    ReleaseFragmentTable();
}

// fucntions and variables for frame info calculation and retrieving
bool FMP4TrackInfo::StartRetrieveFrameInfo(uint64 base_ts)
{
    i = 0;
    data_offset = 0;
    accumulated_decode_ts = base_ts;
    return true;
}

bool FMP4TrackInfo::PeekNextFrame(MP4FrameInfo& frame)
{
    if (i >= _frames_count)
        return false;

    // frame offset
    frame.offset = /*(uint32)_base_data_offset + */data_offset;

    // frame size
    if (_sample_size_table)
        frame.len = _sample_size_table[i];
    else
        frame.len = _default_sample_size;

    // get sync
    if (_sdtp_table)
    {
        char sample_depends_on = (_sdtp_table[i] >> 4) & 3;
        if (sample_depends_on == 2) // I frame
            frame.is_sync = true;
        else
            frame.is_sync = false;
    }
    else
        frame.is_sync = true;

    // get rap
    frame.is_rap = (0 == data_offset);

    // get ts
    frame.dts = accumulated_decode_ts;
    frame.pts = frame.dts;

    // get duration
    if (_sample_duration_table)
        frame.duration = _sample_duration_table[i];
    else
        frame.duration = _default_sample_duration;

    // get composition ts
    if (_sample_comptimeoffset_table)
    {
        frame.pts = frame.dts + _sample_comptimeoffset_table[i];
    }
    else if (_sample_ptso_table)
    {
        frame.pts = frame.dts + _sample_ptso_table[i];
    }

    // drm info
    if (_se_info && i < _se_info->_sample_count)
    {
        frame.se_info = &_se_info->_sub_sample_table[i];
    }
    else
        frame.se_info = 0;

    frame.media_type = (MP4TrackType)_media_type;
    frame.frame_index = i;

    return true;
}

void FMP4TrackInfo::MoveToNextFrame(MP4FrameInfo& frame)
{
    // add sample size to data_offset, this will be the data offset for the next frame
    data_offset += frame.len;

    // add duration of frame #i to accumulated_decode_ts
    accumulated_decode_ts += frame.duration;

    // increment index
    ++i;
}

bool FMP4TrackInfo::GetNextFrame(MP4FrameInfo& frame)
{
    if (PeekNextFrame(frame))
    {
        MoveToNextFrame(frame);
        return true;
    }

    return false;
}

bool FMP4TrackInfo::AllocateSampleTable(uint32 sample_count, uint8 flags)
{
    ASSERT(_sample_duration_table == 0);
    ASSERT(_sample_size_table == 0);
    ASSERT(_sample_flags_table == 0);
    ASSERT(_sample_comptimeoffset_table == 0);

    _sample_table_len = sample_count;

    if (flags & 0x01)
    {
        _sample_duration_table = NEW_NO_THROW uint32[_sample_table_len];
        if (_sample_duration_table == 0)
            return false;
    }

    if (flags & 0x02)
    {
        _sample_size_table = NEW_NO_THROW uint32[_sample_table_len];
        if (_sample_size_table == 0)
            return false;
    }

    if (flags & 0x04)
    {
        _sample_flags_table = NEW_NO_THROW uint32[_sample_table_len];
        if (_sample_flags_table == 0)
            return false;
    }

    if (flags & 0x08)
    {
        _sample_comptimeoffset_table = NEW_NO_THROW int32[_sample_table_len];
        if (_sample_comptimeoffset_table == 0)
            return false;
    }

    return true;
}

void FMP4TrackInfo::ReleaseSampleTable()
{
    _sample_table_len = 0;

    if (_sample_duration_table)
    {
        delete [] _sample_duration_table;
        _sample_duration_table = 0;
    }

    if (_sample_size_table)
    {
        delete [] _sample_size_table;
        _sample_size_table = 0;
    }

    if (_sample_flags_table)
    {
        delete [] _sample_flags_table;
        _sample_flags_table = 0;
    }

    if (_sample_comptimeoffset_table)
    {
        delete [] _sample_comptimeoffset_table;
        _sample_comptimeoffset_table = 0;
    }
}

bool FMP4TrackInfo::AllocateFragmentTable(uint8 fragment_count)
{
    //Release any pre-existing table
    ReleaseFragmentTable();

    if (fragment_count == 0)
        return false;

    _fragment_absolute_time_table = NEW_NO_THROW uint64[fragment_count];
    if (_fragment_absolute_time_table == 0)
        return false;

    _fragment_duration_table = NEW_NO_THROW uint64[fragment_count];
    if (_fragment_duration_table == 0)
        return false;

    _live_frags_ahead = fragment_count;

    return true;
}

void FMP4TrackInfo::ReleaseFragmentTable()
{
    _live_frags_ahead = 0;

    if (_fragment_absolute_time_table)
        delete [] _fragment_absolute_time_table;

    if (_fragment_duration_table)
        delete [] _fragment_duration_table;
}

BaseMP4Info::BaseMP4Info()
    : _bytesParsed(0)
#ifdef DEBUG
    , _layer(0)
#endif
{
}

// return a lookup of sampleIndex to the SampleGroupingData
bool FMP4TrackInfo::GetSampleEncryptionInfo(_Out_ std::vector<CencSampleEncryptionInformationAudioGroupEntry*>* sampleEncryptionInfoEntries,
                                            _Out_ std::vector<int32>* sampleEncryptionInfoIndexes)
{
    static const uint32 kSampleGroupDescriptionStartIndex = 0x10001;

    if(NULL == _se_info )
    {
        TRACE_ERROR(("GetSampleEncryptionInfo: failed _se_info does not exist"));
        return false;
    }

    bool overrideEncryption = (_se_info->_cencSampleGroupingData != NULL && _se_info->_cencSampleGroupDescriptionData != NULL);

    // 'seig' sample grouping is used. Figure out what group description data should be used to figure out the identifier size
    if (overrideEncryption)
    {
        uint32 entryCount = _se_info->_cencSampleGroupingData->_entryCount;
        for (uint32 i = 0; i < entryCount; i++)
        {
            uint32 gpIndex = _se_info->_cencSampleGroupingData->_entries[i]->_gpIndex;
            if (gpIndex < kSampleGroupDescriptionStartIndex)
            {
                TRACE_ERROR(("GetSampleEncryptionInfo: Invalid sample group description index %d specified in Sample-To-Group box",
                    gpIndex));
                return false;
            }

            gpIndex -= kSampleGroupDescriptionStartIndex;
            if (gpIndex >= _se_info->_cencSampleGroupDescriptionData->_entryCount)
            {
                TRACE_ERROR(("GetSampleEncryptionInfo: Sample group description index %d specified in Sample-To-Group box is out of boundary",
                    gpIndex));
                return false;
            }
            sampleEncryptionInfoEntries->push_back(_se_info->_cencSampleGroupDescriptionData->_entries[gpIndex]);
        }

        for (uint32 i = 0; i < entryCount; i++)
        {
            // All the samples in one cencSampleGroupingData entry share the same encryption setting indexed at idxSampleEncryptionInfoEntries
            for (uint32 j = 0; j < _se_info->_cencSampleGroupingData->_entries[i]->_sampleCount; j++)
            {
                sampleEncryptionInfoIndexes->push_back(i);
            }
        }
    }
    return true;
}

BaseMP4Info::~BaseMP4Info()
{
    SelfCleanup();
}

void BaseMP4Info::Cleanup()
{
    SelfCleanup();
}

uint32 BaseMP4Info::GetFrameCount(MP4TrackType type)
{
    for (uint32 i = 0; i<_tracks.size(); ++i)
    {
        if (((MP4TrackType)_tracks[i]->_media_type == type) || (kMP4TrackType_None == type))
            return _tracks[i]->_frames_count;
    }

    MP4_DEBUG_VALIDATE(false);
    return 0;
}

BaseTrackInfo* BaseMP4Info::GetTrackInfo(MP4TrackType type)
{
    for (uint32 i = 0; i<_tracks.size(); ++i)
    {
        if (((MP4TrackType)_tracks[i]->_media_type == type) || (kMP4TrackType_None == type))
            return _tracks[i];
    }

    MP4_DEBUG_VALIDATE(false);
    return 0;
}

void BaseMP4Info::SelfCleanup()
{
    for (uint32 i=0; i<_tracks.size(); ++i)
    {
        if (NULL != _tracks[i])
        {
            delete _tracks[i];
            _tracks[i] = NULL;
        }
    }

    _tracks.clear();

#ifdef DEBUG
    _layer = 0;
#endif
}

MP4Info::MP4Info()
    : _avc_seq_header(0)
    , _avc_seq_header_length(0)
    , _avc_sps_len(0)
    , _avc_pps_len(0)
    , _avc_nal_unit_length(0)
    , _total_frame_count(0)
    , _vid_frame_count(0)
    , _aud_frame_count(0)
    , _audio_track_info(NULL)
    , _video_track_info(NULL)
    , _nextFrameIdx(0)
    , _vidIdx(0)
    , _audIdx(0)
    , _vidInfoSaved(false)
    , _audInfoSaved(false)
    , _mdat_size(0)
{
}

MP4Info::~MP4Info()
{
    SelfCleanup();
}

void MP4Info::Cleanup()
{
    SelfCleanup();
    // clean up base
    BaseMP4Info::Cleanup();
}

// return duration in 100ns units
uint64 MP4Info::GetDuration()
{
    BaseTrackInfo* audio_track_info = GetTrackInfo(kMP4TrackType_Audio);
    BaseTrackInfo* video_track_info = GetTrackInfo(kMP4TrackType_Video);

    uint64 aud_duration = 0, vid_duration = 0;

    if (audio_track_info && audio_track_info->_track_time_scale)
        aud_duration = (uint64)audio_track_info->_track_duration * 10000000 / audio_track_info->_track_time_scale;

    if (video_track_info && video_track_info->_track_time_scale)
        vid_duration = (uint64)video_track_info->_track_duration * 10000000 / video_track_info->_track_time_scale;

    return aud_duration > vid_duration ? aud_duration : vid_duration;
}

void MP4Info::SelfCleanup()
{
    if (_avc_seq_header)
    {
        delete [] _avc_seq_header;
        _avc_seq_header = 0;
    }
}

bool MP4Info::ParseAVCSeqHeader(const uint8* media_descriptor, uint32 media_descriptor_len)
{
    const uint8* avcP = media_descriptor;
    uint32 bytesLeft = media_descriptor_len;
    if (avcP == NULL || bytesLeft < 8)
        return false;
    if (*avcP++ != 1)       // check configuration version
        return false;
    avcP++; //AvcProfileIndication
    avcP++; //AvcProfileCompatibility
    avcP++; //AvcLevelIndication
    _avc_nal_unit_length = ( (*avcP++) & 0x03 ) + 1;
    uint8 numSequenceParameterSets = (*avcP++) & 0x1F;
    if (numSequenceParameterSets != 1)
        return false;

    bytesLeft -= 6;             // have read six bytes so far

    const uint8* seqHdrStart = &avcP[2];

    _avc_sps_len = BigEndian::BytesToHost<uint16,2>( avcP );

    if (bytesLeft < (2 + (uint32)_avc_sps_len + 1))       // make sure we can read through to numOfPictureParameterSets
        return false;
    uint32 numOfPictureParameterSets = avcP[2 + _avc_sps_len];
    if (numOfPictureParameterSets != 1)
        return false;         // we like just one...

    if (bytesLeft < (2 + (uint32)_avc_sps_len + 1 + 2))     // make sure we can read pictureParameterSetLength
        return false;         // we like just one...

    const uint8* picHdrStart = &avcP[2 + _avc_sps_len + 1];

    _avc_pps_len = BigEndian::BytesToHost<uint16,2>( picHdrStart );

    if (bytesLeft < (2 + (uint32)_avc_sps_len + 1 + 2 + (uint32)_avc_pps_len))
        return false;
    picHdrStart += 2;

    _avc_seq_header_length = _avc_sps_len + _avc_pps_len + 8;
    _avc_seq_header = NEW_NO_THROW uint8[_avc_seq_header_length];
    if (_avc_seq_header == NULL)
        return false;

    uint8* bp = _avc_seq_header;

    *bp++ = 0x00;
    *bp++ = 0x00;
    *bp++ = 0x00;
    *bp++ = 0x01;
    memcpy_s(bp, _avc_seq_header_length - 4, seqHdrStart, _avc_sps_len);
    bp += _avc_sps_len;

    *bp++ = 0x00;
    *bp++ = 0x00;
    *bp++ = 0x00;
    *bp++ = 0x01;
    memcpy_s(bp, _avc_seq_header_length - 8, picHdrStart, _avc_pps_len);

    return true;
}

bool MP4Info::StartRetrieveFrameInfo(uint64 base_ts)
{
    _audio_track_info = GetTrackInfo(kMP4TrackType_Audio);
    if (_audio_track_info)
    {
        _aud_frame_count = _audio_track_info->_frames_count;
        if (_audio_track_info->StartRetrieveFrameInfo(base_ts) == false)
            return false;
    }
    else
        _aud_frame_count = 0;

    _video_track_info = GetTrackInfo(kMP4TrackType_Video);
    if (_video_track_info)
    {
        _vid_frame_count = _video_track_info->_frames_count;
        if (_video_track_info->StartRetrieveFrameInfo(base_ts) == false)
            return false;
    }
    else
        _vid_frame_count = 0;

    _total_frame_count = _aud_frame_count + _vid_frame_count;

    memset(&_vid_frame_info, 0, sizeof(MP4FrameInfo));
    memset(&_aud_frame_info, 0, sizeof(MP4FrameInfo));

    _nextFrameIdx = 0;
    _vidIdx = 0;
    _audIdx = 0;

    _vidInfoSaved = true;
    _audInfoSaved = true;

    _mdat_size = 0;

    return true;
}

bool MP4Info::GetNextFrame(MP4FrameInfo& frame)
{
    memset(&frame, 0, sizeof(MP4FrameInfo));

    if (_nextFrameIdx >= _total_frame_count)
        return false; // no more frame to receive

    if (_vidIdx < _vid_frame_count && _vidInfoSaved)
    {
        _video_track_info->GetNextFrame(_vid_frame_info);
        MP4_DEBUG_VALIDATE(_vidIdx == _vid_frame_info.frame_index);
        if (_vidIdx != _vid_frame_info.frame_index)
            return false;
        _vidInfoSaved = false;
    }

    if (_audIdx < _aud_frame_count && _audInfoSaved)
    {
        _audio_track_info->GetNextFrame(_aud_frame_info);
        MP4_DEBUG_VALIDATE(_audIdx == _aud_frame_info.frame_index);
        if (_audIdx != _aud_frame_info.frame_index)
            return false;
        _audInfoSaved = false;
    }

    // compare the two if both are not saved yet, and save the one with smaller offset
    if (_vidInfoSaved == false && _audInfoSaved == false)
    {
        if (_vid_frame_info.offset < _aud_frame_info.offset)
        {
            _vidInfoSaved = true;
            ++_vidIdx;

            // recv video frame
            memcpy_s(&frame, sizeof(MP4FrameInfo), &_vid_frame_info, sizeof(MP4FrameInfo));
            return true;
        }
        else
        {
            _audInfoSaved = true;
            ++_audIdx;

            // recv audio frame
            memcpy_s(&frame, sizeof(MP4FrameInfo), &_aud_frame_info, sizeof(MP4FrameInfo));
            return true;
        }
    }
    // else simply save the one that's not saved yet.
    else
    {
        if (_vidInfoSaved == false)
        {
            _vidInfoSaved = true;
            ++_vidIdx;

            // recv video frame
            memcpy_s(&frame, sizeof(MP4FrameInfo), &_vid_frame_info, sizeof(MP4FrameInfo));
            return true;
        }
        else if (_audInfoSaved == false)
        {
            _audInfoSaved = true;
            ++_audIdx;

            // recv audio frame
            memcpy_s(&frame, sizeof(MP4FrameInfo), &_aud_frame_info, sizeof(MP4FrameInfo));
            return true;
        }
    }

    return false;
}

#ifdef DEBUG
void MP4Info::DumpFramesInfo()
{
    if (StartRetrieveFrameInfo())
    {
        MP4FrameInfo frame;
        while (GetNextFrame(frame))
        {
            if (frame.media_type == kMP4TrackType_Video)
                TRACE_TABLE(("FrameInfoTable[%d], video[%d], offset:%d, len:%d, dts:%lld, pts:%lld, sync:%d.\n",
                    _nextFrameIdx, _vidIdx, frame.offset, frame.len, frame.dts, frame.pts, frame.is_sync));
            else
                TRACE_TABLE(("FrameInfoTable[%d], audio[%d], offset:%d, len:%d, dts:%lld, pts:%lld, sync:%d.\n",
                    _nextFrameIdx, _audIdx, frame.offset, frame.len, frame.dts, frame.pts, frame.is_sync));

            _nextFrameIdx++;
        }
    }
}
#endif

FMP4Info::FMP4Info(uint32 fragment_media_type)
    : _fragment_media_type(fragment_media_type)
    , _track_info(NULL)
{
}

// return duration in 100ns units
uint64 FMP4Info::GetDuration()
{
    _track_info = GetTrackInfo();

    if (_track_info && _track_info->_track_time_scale)
        return (uint64)_track_info->_track_duration * 10000000 / _track_info->_track_time_scale;

    return 0;
}

bool FMP4Info::StartRetrieveFrameInfo(uint64 base_ts)
{
    _track_info = GetTrackInfo();

    if (_track_info && _track_info->StartRetrieveFrameInfo(base_ts))
        return true;

    return false;
}

bool FMP4Info::PeekNextFrame(MP4FrameInfo& frame)
{
    CHECK_ALLOC(_track_info);

    return _track_info->PeekNextFrame(frame);
}

void FMP4Info::MoveToNextFrame(MP4FrameInfo& frame)
{
    CHECK_ALLOC(_track_info);

    _track_info->MoveToNextFrame(frame);
}

bool FMP4Info::GetNextFrame(MP4FrameInfo& frame)
{
    CHECK_ALLOC(_track_info);

    return _track_info->GetNextFrame(frame);
}

#ifdef DEBUG
void FMP4Info::DumpFramesInfo()
{
    if (StartRetrieveFrameInfo())
    {
        MP4FrameInfo frame;
        uint32 frameIdx = 0;
        while (GetNextFrame(frame))
        {
            TRACE_TABLE(("FrameInfoTable[%d], offset:%d, len:%d, dts:%lld, pts:%lld, sync:%d.\n",
                frameIdx, frame.offset, frame.len, frame.dts, frame.pts, frame.is_sync));

            frameIdx++;
        }
    }
}
#endif
