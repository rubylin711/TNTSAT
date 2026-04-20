///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MP4Feed.h"
#include "MP4DataType.h"

#include <vector>
using namespace std;

#define MAKEATOMTYPE(ch0, ch1, ch2, ch3)    \
    ((uint32)(char)(ch3) | ((uint32)(char)(ch2) << 8) |    \
    ((uint32)(char)(ch1) << 16) | ((uint32)(char)(ch0) << 24 ))

inline void INT32TOSTR( _In_ uint32 i, _Out_ char (&s)[5] )
{
    s[0] = ((i >> 24) & 0xFF);
    s[1] = ((i >> 16) & 0xFF);
    s[2] = ((i >> 8) & 0xFF);
    s[3] = (i & 0xFF);
    s[4] = 0;
}

class MP4AtomBasic;
class BaseMP4Info;

class MP4Atom
{
public:
    enum EAtomType
    {
        eAtomType_mdat    = MAKEATOMTYPE('m', 'd', 'a', 't'), // mdat
        eAtomType_moov    = MAKEATOMTYPE('m', 'o', 'o', 'v'), // moov
        eAtomType_tref    = MAKEATOMTYPE('t', 'r', 'e', 'f'), // tref
        eAtomType_mdia    = MAKEATOMTYPE('m', 'd', 'i', 'a'), // mdia
        eAtomType_minf    = MAKEATOMTYPE('m', 'i', 'n', 'f'), // minf
        eAtomType_mvhd    = MAKEATOMTYPE('m', 'v', 'h', 'd'), // mvhd
        eAtomType_trak    = MAKEATOMTYPE('t', 'r', 'a', 'k'), // trak
        eAtomType_tkhd    = MAKEATOMTYPE('t', 'k', 'h', 'd'), // tkhd
        eAtomType_load    = MAKEATOMTYPE('l', 'o', 'a', 'd'), // load
        eAtomType_tmcd    = MAKEATOMTYPE('t', 'm', 'c', 'd'), // tmcd
        eAtomType_chap    = MAKEATOMTYPE('c', 'h', 'a', 'p'), // chap
        eAtomType_sync    = MAKEATOMTYPE('s', 'y', 'n', 'c'), // sync
        eAtomType_scpt    = MAKEATOMTYPE('s', 'c', 'p', 't'), // scpt
        eAtomType_ssrc    = MAKEATOMTYPE('s', 's', 'r', 'c'), // ssrc
        eAtomType_hint    = MAKEATOMTYPE('h', 'i', 'n', 't'), // hint
        eAtomType_mdhd    = MAKEATOMTYPE('m', 'd', 'h', 'd'), // mdhd
        eAtomType_hdlr    = MAKEATOMTYPE('h', 'd', 'l', 'r'), // hdlr
        eAtomType_vmhd    = MAKEATOMTYPE('v', 'm', 'h', 'd'), // vmhd
        eAtomType_stbl    = MAKEATOMTYPE('s', 't', 'b', 'l'), // stbl
        eAtomType_stsd    = MAKEATOMTYPE('s', 't', 's', 'd'), // stsd
        eAtomType_stts    = MAKEATOMTYPE('s', 't', 't', 's'), // stts
        eAtomType_stss    = MAKEATOMTYPE('s', 't', 's', 's'), // stss
        eAtomType_stsc    = MAKEATOMTYPE('s', 't', 's', 'c'), // stsc
        eAtomType_stsz    = MAKEATOMTYPE('s', 't', 's', 'z'), // stsz
        eAtomType_stco    = MAKEATOMTYPE('s', 't', 'c', 'o'), // stco
        eAtomType_ctts    = MAKEATOMTYPE('c', 't', 't', 's'), // ctts
        eAtomType_moof    = MAKEATOMTYPE('m', 'o', 'o', 'f'), // moof
        eAtomType_mfhd    = MAKEATOMTYPE('m', 'f', 'h', 'd'), // mfhd
        eAtomType_traf    = MAKEATOMTYPE('t', 'r', 'a', 'f'), // traf
        eAtomType_tfhd    = MAKEATOMTYPE('t', 'f', 'h', 'd'), // tfhd
        eAtomType_trun    = MAKEATOMTYPE('t', 'r', 'u', 'n'), // trun
        eAtomType_sdtp    = MAKEATOMTYPE('s', 'd', 't', 'p'), // sdtp
        eAtomType_uuid    = MAKEATOMTYPE('u', 'u', 'i', 'd'), // uuid
        eAtomType_drIV    = MAKEATOMTYPE('d', 'r', 'I', 'V'), // drIV
        eAtomType_wave    = MAKEATOMTYPE('w', 'a', 'v', 'e'), // wave
        eAtomType_esds    = MAKEATOMTYPE('e', 's', 'd', 's'), // esds
        eAtomType_avcC    = MAKEATOMTYPE('a', 'v', 'c', 'C'), // avcC
        eAtomType_soun    = MAKEATOMTYPE('s', 'o', 'u', 'n'), // soun
        eAtomType_vide    = MAKEATOMTYPE('v', 'i', 'd', 'e'), // vide
        eAtomType_text    = MAKEATOMTYPE('t', 'e', 'x', 't'), // text
        eAtomType_pssh    = MAKEATOMTYPE('p', 's', 's', 'h'), // pssh
        eAtomType_sbgp    = MAKEATOMTYPE('s', 'b', 'g', 'p'), // sbgp
        eAtomType_sgpd    = MAKEATOMTYPE('s', 'g', 'p', 'd'), // sgpd
        eAtomType_saiz    = MAKEATOMTYPE('s', 'a', 'i', 'z'), // saiz
        eAtomType_saio    = MAKEATOMTYPE('s', 'a', 'i', 'o'), // saio
        eAtomType_cenc    = MAKEATOMTYPE('c', 'e', 'n', 'c'), // cenc
        eAtomType_seig    = MAKEATOMTYPE('s', 'e', 'i', 'g'), // seig
        eAtomType_senc    = MAKEATOMTYPE('s', 'e', 'n', 'c'), // senc
    };

    MP4Atom(MP4Feed* data_feed, BaseMP4Info* info_collector, bool has_child, bool keep_in_memory);
    virtual ~MP4Atom();

    // where should parsing stop?
    void SetStopOnAtom(uint32 type);
    // create a new atom
    MP4Atom* CreateNewAtom(uint32 size, uint32 type);

    // create a UUID atom, which has to be a basic atom.
    virtual MP4Atom* CreateUUIDAtom(uint32 size, uint32 type) { return 0; }
    // parse the atom
    virtual bool Parse();
    // parse the properties of the atom
    virtual bool ParseProperties();
    // parse the child atoms of the atom
    virtual bool ParseChildren();
    // should stop parsing?
    virtual bool ShouldStop(uint32 type);

    // does the atom have child atom?
    bool HasChild() const;
    // does the atom need to stay in memory after parsing?
    bool KeepInMemory() const;

    MP4AtomBasic* GetAtom(uint32 type);

    MP4Feed* GetDataFeed() { ASSERT(_data_feed); return _data_feed; }

protected:
    // the data feed
    MP4Feed* _data_feed;
    // info collector
    BaseMP4Info* _info_collector;
    // stop parsing after this atom
    uint32 _stop_on_atom;

    bool _has_child;
    vector<MP4Atom*> _children;

    bool _keep_in_memory;
};

class MP4AtomBasic : public MP4Atom
{
public:
    MP4AtomBasic(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);
    virtual ~MP4AtomBasic();

    // overides
    __override MP4Atom* CreateUUIDAtom(uint32 size, uint32 type);
    __override bool Parse();
    __override bool ParseChildren();
    __override bool ParseProperties();

    uint32 Type() const { return _type; }

    bool ReadInt8( _Out_ uint8* field );
    bool ReadInt16( _Out_ uint16* field );
    bool ReadInt24( _Out_ uint32* field );
    bool ReadInt32( _Out_ uint32* field );
    bool ReadInt64( _Out_ uint64* field );
    bool ReadInt64N( _Out_ uint64* field, _In_ int cb );
    bool ReadFixed16( _Out_ Fixed_16* field );
    bool ReadFixed32( _Out_ Fixed_32* field );
    bool ReadGuid( _Out_ MP4_Guid* field );
    bool ReadArray( _Out_ uint8* field, _Out_ int32 length );
    bool Skip(uint32 count);

protected:
    uint32 _size;
    uint32 _type;
    char _type_name[5];
    uint32 _parsed_bytes;
};

class MP4AtomFull : public MP4AtomBasic
{
public:
    MP4AtomFull(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

    bool ReadTimeField(uint64* field);
    bool SkipTimeField(int32 count);

protected:
    uint8 _version;
    uint8 _flags[3];
};

/*
class MP4Atom_moov : public MP4AtomBasic
{
public:
    MP4Atom_moov(uint32 size, uint32 type, bool has_child, bool keep_in_memory)
        : MP4AtomBasic(size, type, has_child, keep_in_memory)
    {}
};
*/

class MP4Atom_mvhd : public MP4AtomFull
{
public:
    MP4Atom_mvhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    uint32 _time_scale;
    uint64 _duration;
    Fixed_32 _preferred_rate;
    uint32 _next_track_id;
};

class MP4Atom_trak : public MP4AtomBasic
{
public:
    MP4Atom_trak(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);
    virtual ~MP4Atom_trak();
};

class MP4Atom_tkhd : public MP4AtomFull
{
public:
    MP4Atom_tkhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    uint32 _track_id;
    uint64 _track_duration;
    Fixed_16 _track_volume;
    Fixed_32 _track_width;
    Fixed_32 _track_height;
};

class MP4Atom_load : public MP4AtomBasic
{
public:
    MP4Atom_load(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    uint32 _preload_start_time;
    uint32 _preload_duration;
    uint32 _preload_flags;
    uint32 _default_hints;
};

class MP4Atom_trak_ref : public MP4AtomBasic
{
public:
    MP4Atom_trak_ref(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    vector<uint32> _track_ids;
};

class MP4Atom_mdhd : public MP4AtomFull
{
public:
    MP4Atom_mdhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

    void LanguageCode16Int2Letter();

protected:
    uint32 _mdhd_time_scale;
    uint64 _mdhd_duration;
    uint16 _language_code;
    uint8 _iso_language_code[4];
    uint16 _quality;
};

class MP4Atom_hdlr : public MP4AtomFull
{
public:
    MP4Atom_hdlr(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);
    ~MP4Atom_hdlr();

    // overides
    __override bool ParseProperties();

protected:
    uint32 _component_type;
    uint32 _component_subtype;
    uint8* _component_name;
    uint32 _component_name_len;
};

class MP4Atom_vmhd : public MP4AtomFull
{
public:
    MP4Atom_vmhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    uint16 _graphics_mode;
    uint16 _opcolor_r;
    uint16 _opcolor_g;
    uint16 _opcolor_b;
};

class MP4Atom_Sample_Descriptor : public MP4AtomBasic
{
public:
    MP4Atom_Sample_Descriptor(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type);
    virtual ~MP4Atom_Sample_Descriptor();

    // overides
    __override bool ParseProperties();
    virtual bool ParseProperties_MediaSpecific();
    virtual bool ParseDescriptorExtension(uint32 count);
    virtual bool ParseMediaDescriptor(uint32 count);

protected:
    uint16 _data_ref_index;
    uint8* _mp4_descriptor;
};

class MP4Atom_Sample_Descriptor_Audio : public MP4Atom_Sample_Descriptor
{
public:
    MP4Atom_Sample_Descriptor_Audio(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type);

    // overides
    __override bool ParseProperties_MediaSpecific();

protected:
    uint16 _version;
    uint16 _channel_count;
    uint16 _sample_size;
    uint16 _compression_id;
    Fixed_32 _sample_rate;

    // version 1 member
    uint32 _samples_per_packet;
    uint32 _bytes_per_packet;
    uint32 _bytes_per_frame;
    uint32 _bytes_per_sample;
};

class MP4Atom_Sample_Descriptor_Video : public MP4Atom_Sample_Descriptor
{
public:
    MP4Atom_Sample_Descriptor_Video(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type);

    // overides
    __override bool ParseProperties_MediaSpecific();

protected:
    uint16 _width; // in pixel
    uint16 _height; // in pixel
    Fixed_32 _hori_reso; // pixel per inch
    Fixed_32 _vert_reso; // pixel per inch
    uint16 _frame_count; // in each sample
    uint8 _compressor_name_len;
    uint8 _compressor_name[31];
    uint16 _depth;
    uint16 _color_table_id;
};

class MP4Atom_stsd : public MP4AtomFull
{
friend class MP4Atom_stbl;
public:
    MP4Atom_stsd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);
    ~MP4Atom_stsd();

    // overides
    __override bool ParseProperties();

protected:
    vector<MP4Atom_Sample_Descriptor*> _description_table;
    uint32 _table_length;
};

typedef struct Time2SampleTableEntry_tag
{
    uint32 _sample_count;
    uint32 _sample_duration;
}
Time2SampleTableEntry;

typedef vector<Time2SampleTableEntry> Time2SampleTable;

class MP4Atom_stts : public MP4AtomFull
{
friend class MP4Atom_stbl;
public:
    MP4Atom_stts(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    Time2SampleTable _time2sample_table;
    uint32 _table_length;
};

typedef struct CompTime2SampleTableEntry_tag
{
    uint32 _sample_count;
    uint32 _samples_offset;
}
CompTime2SampleTableEntry;

typedef vector<CompTime2SampleTableEntry> CompTime2SampleTable;

class MP4Atom_ctts : public MP4AtomFull
{
friend class MP4Atom_stbl;
public:
    MP4Atom_ctts(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    CompTime2SampleTable _comp_time2sample_table;
    uint32 _table_length;
};

typedef vector<uint32> SyncSampleTable;

class MP4Atom_stss : public MP4AtomFull
{
friend class MP4Atom_stbl;
public:
    MP4Atom_stss(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    SyncSampleTable _sync_sample_table;
    uint32 _table_length;
};

typedef struct Sample2ChunkTableEntry_tag
{
    uint32 _first_chunk;
    uint32 _samples_per_chunk;
    uint32 _sample_desc_id;
}
Sample2ChunkTableEntry;

typedef vector<Sample2ChunkTableEntry> Sample2ChunkTable;

class MP4Atom_stsc : public MP4AtomFull
{
friend class MP4Atom_stbl;
public:
    MP4Atom_stsc(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    Sample2ChunkTable _sample2chunk_table;
    uint32 _table_length;
};

typedef vector<uint32> SampleSizeTable;

class MP4Atom_stsz : public MP4AtomFull
{
friend class MP4Atom_stbl;
public:
    MP4Atom_stsz(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    uint32 _sample_size;
    SampleSizeTable _sample_size_table;
    uint32 _table_length;
};

typedef vector<uint32> ChunkOffsetTable;

class MP4Atom_stco : public MP4AtomFull
{
friend class MP4Atom_stbl;
public:
    MP4Atom_stco(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    ChunkOffsetTable _chunk_offset_table;
    uint32 _table_length;
};

class MP4Atom_stbl : public MP4AtomBasic
{
public:
    MP4Atom_stbl(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);
    virtual ~MP4Atom_stbl();

    // overides
    __override bool Parse();

protected:
    ChunkOffsetTable* GetChunkOffsetTable();
    SampleSizeTable* GetSampleSizeTable();
    Sample2ChunkTable* GetSample2ChunkTable();
    Time2SampleTable* GetTime2SampleTable();
    CompTime2SampleTable* GetCompTime2SampleTable();
    SyncSampleTable* GetSyncSampleTable();
    uint32 GetSingleSampleSize();
};

// f-mp4 classes
class MP4Atom_moof : public MP4AtomBasic
{
public:
    MP4Atom_moof(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);
};

class MP4Atom_mfhd : public MP4AtomFull
{
public:
    MP4Atom_mfhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    uint32 _sequence_number;
};

class MP4Atom_tfhd : public MP4AtomFull
{
public:
    MP4Atom_tfhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

protected:
    uint32 _track_id;
    uint64 _base_data_offset;
    uint32 _sample_description_index;
    uint32 _default_sample_duration;
    uint32 _default_sample_size;
    uint32 _default_sample_flags;
};

class FMP4TrackInfo;

class MP4Atom_trun : public MP4AtomFull
{
public:
    MP4Atom_trun(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

    bool ReadSampleTable(FMP4TrackInfo* track);

protected:
    uint32 _sample_count;
    uint32 _data_offset;
    uint32 _first_sample_flags;
};

class MP4Atom_sdtp : public MP4AtomFull
{
public:
    MP4Atom_sdtp(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();
};

class MP4Atom_uuid : public MP4AtomFull
{
public:
    MP4Atom_uuid(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory, MP4_Guid& guid);

    // overides
    __override bool ParseProperties();

protected:
    MP4_Guid _guid;
};

// sample encryption (uuid or senc)
class MP4Atom_senc : public MP4AtomFull
{
public:
    MP4Atom_senc(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory, bool fromGuid);

    // overides
    __override bool ParseProperties();

private:
    bool _fromGuid;
};

// track fragment extended header (uuid)
class MP4Atom_uuid_tfeh : public MP4Atom_uuid
{
public:
    MP4Atom_uuid_tfeh(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory, MP4_Guid& guid);

    // overides
    __override bool ParseProperties();
};

// track fragment reference box (uuid)
class MP4Atom_uuid_tfrb : public MP4Atom_uuid
{
public:
    MP4Atom_uuid_tfrb(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory, MP4_Guid& guid);

    // overides
    __override bool ParseProperties();
};

// sample presentation time offset (uuid)
class MP4Atom_uuid_sptf : public MP4Atom_uuid
{
public:
    MP4Atom_uuid_sptf(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory, MP4_Guid& guid);

    // overides
    __override bool ParseProperties();
};

// unknown (uuid)
class MP4Atom_uuid_unknown : public MP4Atom_uuid
{
public:
    MP4Atom_uuid_unknown(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, MP4_Guid& guid);

    // overides
    __override bool ParseProperties();
};

class MP4Atom_drIV : public MP4AtomBasic
{
public:
    MP4Atom_drIV(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();
};

// protection system specific header
class MP4Atom_pssh : public MP4AtomFull
{
public:
    MP4Atom_pssh(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();
};

class MP4Atom_sbgp : public MP4AtomFull
{
public:
    MP4Atom_sbgp(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();
};

class MP4Atom_sgpd : public MP4AtomFull
{
public:
    MP4Atom_sgpd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();

private:
    void ReadSampleGroupDescriptionEntries(int32 version);
};

class MP4Atom_saiz : public MP4AtomFull
{
public:
    MP4Atom_saiz(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();
};

class MP4Atom_saio : public MP4AtomFull
{
public:
    MP4Atom_saio(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory);

    // overides
    __override bool ParseProperties();
};
