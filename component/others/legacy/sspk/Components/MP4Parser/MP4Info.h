///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MP4Atom.h"

typedef enum
{
    kMP4TrackType_None = 0,
    kMP4TrackType_Video = MP4Atom::eAtomType_vide,
    kMP4TrackType_Audio = MP4Atom::eAtomType_soun,
    kMP4TrackType_Text = MP4Atom::eAtomType_text
}
MP4TrackType;


struct SampleEncryptionSubSampleInfo
{
    SampleEncryptionSubSampleInfo();
    ~SampleEncryptionSubSampleInfo();

    bool AllocateIV(uint8 iv_size);
    bool AllocateEntries(uint16 entry_count);

    // Note: This implementation requires initialization vector data to be 8 bytes or less.
    //       If for example a IV up to 16 bytes in length is required, then a 128 bit
    //       unsigned integer data type will need to be created and converted from the
    //       ISO big-endian representation to the proper CPU representation.

    uint8 _iv_size;         // length of iv
    uint64 _iv;             // iv data
    uint16 _entry_count;    // length of clear/encrypted data
    uint16* _clear_data;    // clear data
    uint32* _encrypted_data; // encrypted data
};

struct SampleGroupingEntry
{
    uint32 _sampleCount;
    uint32 _gpIndex;
};

struct SampleGroupingData
{
    SampleGroupingData();
    ~SampleGroupingData();

    uint32 _entryCount;
    SampleGroupingEntry** _entries;
};

struct CencSampleEncryptionInformationAudioGroupEntry
{
    CencSampleEncryptionInformationAudioGroupEntry();
    ~CencSampleEncryptionInformationAudioGroupEntry();

    uint32 _descriptionLength;
    uint32 _algId;
    uint8 _ivSize;
    MP4_Guid* _kid;
};

struct SampleGroupDescriptionData
{
    SampleGroupDescriptionData();
    ~SampleGroupDescriptionData();

    uint32 _entryCount;
    uint32 _defaultLength;
    CencSampleEncryptionInformationAudioGroupEntry** _entries;
};

struct SampleAuxiliaryInformationOffsetData
{
    SampleAuxiliaryInformationOffsetData();
    ~SampleAuxiliaryInformationOffsetData();

    uint32 _entryCount;
    uint32* _offsets;
    uint64* _loffsets;
};

struct SampleAuxiliaryInformationSizeData
{
    SampleAuxiliaryInformationSizeData();
    ~SampleAuxiliaryInformationSizeData();

    uint8 _defaultSampleInfoSize;
    uint32 _sampleCount;
    uint8* _sampleInfoSize;
};

struct SampleEncryptionInfo
{
    SampleEncryptionInfo();
    ~SampleEncryptionInfo();

    uint32 _algorithm_id;
    uint8 _iv_size;
    MP4_Guid* _kid;
    uint32 _sample_count;
    uint8* _senc_sub_sample_startPos;
    SampleEncryptionSubSampleInfo* _sub_sample_table;
    SampleGroupingData* _cencSampleGroupingData;
    SampleGroupDescriptionData* _cencSampleGroupDescriptionData;
    SampleAuxiliaryInformationOffsetData* _sampleAuxInfoOffsetData;
    SampleAuxiliaryInformationSizeData* _sampleAuxInfoSizeData;
};


struct ProtectionSystemSpecificInfo
{
    ProtectionSystemSpecificInfo();
    ~ProtectionSystemSpecificInfo();

    MP4_Guid* _system_id;
    uint32 _data_size;
    uint8* _data;
};

struct DrmInfo
{
    DrmInfo();
    ~DrmInfo();

    uint32 _sample_count;
    uint32* _iv_length_table;
    uint8** _iv_data_table;
};




struct MP4FrameInfo
{
    MP4FrameInfo();

    uint64          pts;            // frame's presentation time stamp
    uint64          dts;            // frame's decoding time stamp
    uint64          duration;       // frame duration
    uint32          offset;         // offset of the frame from the implicit or explicit base data offset
    uint32          len;            // length of the frame / the highest bit indicates whether it is a sync frame
    MP4TrackType    media_type;     // track type of the media, value of MP4TrackType
    uint32          frame_index;    // frame index in the track
    bool            is_sync;        // is the frame a sync point?
    bool            is_rap;         // is the frame a RAP (Random Access Point)?

    SampleEncryptionSubSampleInfo* se_info; // sample encryption info
};


typedef vector<uint32> ChunkOffsetTable;
typedef vector<uint32> SampleSizeTable;
typedef vector<Sample2ChunkTableEntry> Sample2ChunkTable;
typedef vector<Time2SampleTableEntry> Time2SampleTable;
typedef vector<CompTime2SampleTableEntry> CompTime2SampleTable;
typedef vector<uint32> SyncSampleTable;


class BaseTrackInfo
{
public:
    BaseTrackInfo();
    virtual ~BaseTrackInfo();

    // fucntions and variables for frame info calculation and retrieving
    virtual bool StartRetrieveFrameInfo(uint64 base_ts = 0) = 0;
    // return the frame info without advancing the internal index
    virtual bool PeekNextFrame(MP4FrameInfo& frame) = 0;
    // advance the internal index
    virtual void MoveToNextFrame(MP4FrameInfo& frame) = 0;
    // return the frame info and also advance the internal index.
    virtual bool GetNextFrame(MP4FrameInfo& frame) = 0;

    void SetMediaType(uint32 media_type);
    void SetMediaFormat(uint32 media_format);

    uint32 _media_type;
    char _media_type_name[5];
    uint32 _media_format;
    char _media_format_name[5];
    uint32 _frames_count;
    uint64 _track_duration; // total number of track-time-scale-unit in the track
    uint32 _track_time_scale; // number of track-time-scale-unit per second
    uint8* _media_descriptor;
    uint32 _media_descriptor_len;

    // audio
    uint16 _channel_count;
    uint32 _sample_rate;
};

class MP4TrackInfo : public BaseTrackInfo
{
public:
    // fucntions and variables for frame info calculation and retrieving
    bool StartRetrieveFrameInfo(uint64 base_ts = 0);
    // not implemented yet. a lot more complicated than the f-mp4 case.
    bool PeekNextFrame(MP4FrameInfo& frame) { return false; }
    // not implemented yet.
    void MoveToNextFrame(MP4FrameInfo& frame) {}
    // return the frame info and also advance the internal index.
    bool GetNextFrame(MP4FrameInfo& frame);

    ChunkOffsetTable        _chunk_offset_table;
    SampleSizeTable         _sample_size_table;
    Sample2ChunkTable       _sample2chunk_table;
    Time2SampleTable        _time2sample_table;
    CompTime2SampleTable    _comp_time2sample_table;
    SyncSampleTable         _sync_sample_table;
    uint32                  _single_sample_size;

    uint32 sample2chunk_idx, sc_sample_idx_start, sc_sample_idx_end;
    uint32 chunk_idx, chunk_idx_base, chunk_idx_inc;
    uint32 chunk_offset; // frame offset within a chunk
    uint32 sync_sample_idx;
    uint32 time2sample_idx, ts_sample_idx_start;
    uint64 accumulated_decode_ts;
    uint32 comp_time2sample_idx, cts_sample_idx_start;
    uint32 i; // frame index being retrieved
};

class FMP4TrackInfo : public BaseTrackInfo
{
public:
    FMP4TrackInfo();
    virtual ~FMP4TrackInfo();

    // fucntions and variables for frame info calculation and retrieving
    bool StartRetrieveFrameInfo(uint64 base_ts = 0);
    // return the frame info without advancing the internal index
    bool PeekNextFrame(MP4FrameInfo& frame);
    // advance the internal index
    void MoveToNextFrame(MP4FrameInfo& frame);
    // return the frame info and also advance the internal index.
    bool GetNextFrame(MP4FrameInfo& frame);

    bool AllocateSampleTable(uint32 sample_count, uint8 flags);
    void ReleaseSampleTable();

    bool AllocateFragmentTable(uint8 fragment_count);
    void ReleaseFragmentTable();

    bool GetSampleEncryptionInfo(_Out_ std::vector<CencSampleEncryptionInformationAudioGroupEntry*>* sampleEncryptionInfoEntries,
                                 _Out_ std::vector<int32>* sampleEncryptionInfoIndexes);

    uint32 _sequence_number; // 1 based fragment index

    uint32 _track_id; // trick id
    uint64 _base_data_offset; // byte count from the beginning of the moof box to the start of the actual data.

    uint32 _sample_description_index;
    uint32 _default_sample_duration;
    uint32 _default_sample_size;
    uint32 _default_sample_flags;

    uint32* _sample_duration_table;
    uint32* _sample_size_table;
    uint32* _sample_flags_table;
    int32* _sample_comptimeoffset_table;
    uint32 _sample_table_len;
    uint8* _sdtp_table;
    uint32* _sample_ptso_table; // table from the ptso uuid box

    uint32 i; // frame index being retrieved
    uint32 data_offset;
    uint64 accumulated_decode_ts;

    SampleEncryptionInfo* _se_info;
    ProtectionSystemSpecificInfo* _pss_info;
    DrmInfo* _drm_info;

    uint64 _live_frag_absolute_time;
    uint64 _live_frag_duration;
    uint64 _live_frag_ntp_timestamp;

    uint8 _live_frags_ahead;
    uint64* _fragment_absolute_time_table;
    uint64* _fragment_duration_table;
};

class BaseMP4Info
{
public:
    BaseMP4Info();
    virtual ~BaseMP4Info();

    virtual void Cleanup();
    virtual uint32 GetFrameCount(MP4TrackType type = kMP4TrackType_None);
    virtual BaseTrackInfo* GetTrackInfo(MP4TrackType type = kMP4TrackType_None);

    virtual uint64 GetDuration() = 0; // get duration in 100ns units
    virtual bool StartRetrieveFrameInfo(uint64 base_ts = 0) = 0;
    virtual bool PeekNextFrame(MP4FrameInfo& frame) = 0;
    virtual void MoveToNextFrame(MP4FrameInfo& frame) = 0;
    virtual bool GetNextFrame(MP4FrameInfo& frame) = 0;
    virtual bool IsFMP4() = 0;

#ifdef DEBUG
    virtual void DumpFramesInfo() = 0;
#endif

private:
    void SelfCleanup();

public:
    vector<BaseTrackInfo*> _tracks;
    uint32 _bytesParsed;

#ifdef DEBUG
    // layer, for debug purpose
    int32 _layer;
#endif
};

class MP4Info : public BaseMP4Info
{
public:
    MP4Info();
    ~MP4Info();

    void Cleanup();
    uint64 GetDuration(); // get duration in 100ns units
    bool StartRetrieveFrameInfo(uint64 base_ts = 0);
    bool PeekNextFrame(MP4FrameInfo& frame) { return false; }
    void MoveToNextFrame(MP4FrameInfo& frame) {}
    bool GetNextFrame(MP4FrameInfo& frame);
    bool IsFMP4() { return false; }

#ifdef DEBUG
    virtual void DumpFramesInfo();
#endif

private:
    void SelfCleanup();

public:
    bool ParseAVCSeqHeader(__in_bcount(media_descriptor_len) const uint8* media_descriptor, uint32 media_descriptor_len);

public:
    // video specific
    uint8* _avc_seq_header;
    uint32 _avc_seq_header_length;
    uint16 _avc_sps_len;
    uint16 _avc_pps_len;
    uint8 _avc_nal_unit_length;

    // member variables for calculating/retrieving frame info
    uint32 _total_frame_count, _vid_frame_count, _aud_frame_count;

    BaseTrackInfo* _audio_track_info;
    BaseTrackInfo* _video_track_info;

    MP4FrameInfo _vid_frame_info;
    MP4FrameInfo _aud_frame_info;

    uint32 _nextFrameIdx;
    uint32 _vidIdx;
    uint32 _audIdx;

    bool _vidInfoSaved;
    bool _audInfoSaved;

    uint32 _mdat_size;
};

class FMP4Info : public BaseMP4Info
{
public:
    FMP4Info(uint32 fragment_media_type);

    uint64 GetDuration(); // get duration in 100ns units
    bool StartRetrieveFrameInfo(uint64 base_ts = 0);
    bool PeekNextFrame(MP4FrameInfo& frame);
    void MoveToNextFrame(MP4FrameInfo& frame);
    bool GetNextFrame(MP4FrameInfo& frame);
    bool IsFMP4() { return true; }

#ifdef DEBUG
    virtual void DumpFramesInfo();
#endif

public:
    uint32 _fragment_media_type;

    ProtectionSystemSpecificInfo _pss_info;

    // member variables for calculating/retrieving frame info
    BaseTrackInfo* _track_info;
};
