///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utils.h"

#include "MP4Atom.h"
#include "MP4Info.h"

//#define DEBUG_SPEW_PROPERTIES
#ifdef DEBUG_SPEW_PROPERTIES
#define TRACE_PROPERTY(x) TRACE(x)
#else
#define TRACE_PROPERTY(x)
#endif

// #define DEBUG_INVALID_MP4_DATA
#ifdef DEBUG_INVALID_MP4_DATA
#define MP4_DEBUG_VALIDATE(x) ASSERT(x)
#else
#define MP4_DEBUG_VALIDATE(x)
#endif

//Sample presentation Time Offset Box ('uuid') GUID: {292FACCA-2493-448c-9F0F-C74C33A93771} <<extends FullBox>>
static const MP4_Guid guidPTSOffset = MP4_Guid(0x292FACCA, 0x2493, 0x448c, 0x9F, 0x0F, 0xC7, 0x4C, 0x33, 0xA9, 0x37, 0x71);

//Sample Encryption Box ('uuid') GUID: {A2394F52-5A9B-4F14-A244-6C427C648DF4} //3.2.3.2
static const MP4_Guid guidSampleEncryption = MP4_Guid(0xA2394F52, 0x5A9B, 0x4F14, 0xA2, 0x44, 0x6C, 0x42, 0x7C, 0x64, 0x8D, 0xF4);


//Track Fragment Extended Header ('uuid') GUID: {6D1D9B05-42D5-44E6-80E2-141DAFF757B2} //3.2.3.1
static const MP4_Guid guidTrackFragmentExtendedHeader = MP4_Guid(0x6D1D9B05, 0x42D5, 0x44E6, 0x80, 0xE2, 0x14, 0x1D, 0xAF, 0xF7, 0x57, 0xB2);

//Track Fragment Reference Box ('uuid') GUID: {D4807ef2-CA39-4695-8E54-26CB9E46A79F}
static const MP4_Guid guidTrackFragmentReferenceBox = MP4_Guid(0xD4807ef2, 0xCA39, 0x4695, 0x8E, 0x54, 0x26, 0xCB, 0x9E, 0x46, 0xA7, 0x9F);

static const uint32 kVersionFlagsSizeBytes = 12;

MP4Atom::MP4Atom(MP4Feed* data_feed, BaseMP4Info* info_collector, bool has_child, bool keep_in_memory)
    : _data_feed(data_feed)
    , _info_collector(info_collector)
    , _stop_on_atom(0)
    , _has_child(has_child)
    , _keep_in_memory(keep_in_memory)
{
    ASSERT(NULL != data_feed);
    ASSERT(NULL != info_collector);
}

MP4Atom::~MP4Atom()
{
    // delete all saved children
    for (uint32 i=0; i<_children.size(); ++i)
        delete _children[i];
}

void MP4Atom::SetStopOnAtom(uint32 type)
{
    _stop_on_atom = type;
}

MP4Atom* MP4Atom::CreateNewAtom(uint32 size, uint32 type)
{
    MP4Atom* atom = 0;

    switch (type)
    {
    case eAtomType_mdat:
        // we should never try to parse mdat.
        return 0;
    case eAtomType_moov:
    case eAtomType_tref:
    case eAtomType_mdia:
    case eAtomType_minf:
        atom = new MP4AtomBasic(_data_feed, _info_collector, size, type, true, false);
        break;
    case eAtomType_mvhd:
        atom = new MP4Atom_mvhd(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_trak:
        atom = new MP4Atom_trak(_data_feed, _info_collector, size, type, true, false);
        break;
    case eAtomType_tkhd:
        atom = new MP4Atom_tkhd(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_load:
        atom = new MP4Atom_load(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_tmcd:
    case eAtomType_chap:
    case eAtomType_sync:
    case eAtomType_scpt:
    case eAtomType_ssrc:
    case eAtomType_hint:
        atom = new MP4Atom_trak_ref(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_mdhd:
        atom = new MP4Atom_mdhd(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_hdlr:
        atom = new MP4Atom_hdlr(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_vmhd:
        atom = new MP4Atom_vmhd(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_stbl:
        atom = new MP4Atom_stbl(_data_feed, _info_collector, size, type, true, false);
        break;
    case eAtomType_stsd:
        atom = new MP4Atom_stsd(_data_feed, _info_collector, size, type, false, true);
        break;
    case eAtomType_stts:
        atom = new MP4Atom_stts(_data_feed, _info_collector, size, type, false, true);
        break;
    case eAtomType_stss:
        atom = new MP4Atom_stss(_data_feed, _info_collector, size, type, false, true);
        break;
    case eAtomType_stsc:
        atom = new MP4Atom_stsc(_data_feed, _info_collector, size, type, false, true);
        break;
    case eAtomType_stsz:
        atom = new MP4Atom_stsz(_data_feed, _info_collector, size, type, false, true);
        break;
    case eAtomType_stco:
        atom = new MP4Atom_stco(_data_feed, _info_collector, size, type, false, true);
        break;
    case eAtomType_ctts:
        atom = new MP4Atom_ctts(_data_feed, _info_collector, size, type, false, true);
        break;
    // f-mp4 classes
    case eAtomType_moof:
        atom = new MP4Atom_moof(_data_feed, _info_collector, size, type, true, false);
        break;
    case eAtomType_mfhd:
        atom = new MP4Atom_mfhd(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_traf:
        atom = new MP4AtomBasic(_data_feed, _info_collector, size, type, true, false);
        break;
    case eAtomType_tfhd:
        atom = new MP4Atom_tfhd(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_trun:
        atom = new MP4Atom_trun(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_sdtp:
        atom = new MP4Atom_sdtp(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_uuid:
        atom = CreateUUIDAtom(size, type);
        break;
    case eAtomType_drIV:
        atom = new MP4Atom_drIV(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_pssh:
        atom = new MP4Atom_pssh(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_sbgp:
        atom = new MP4Atom_sbgp(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_sgpd:
        atom = new MP4Atom_sgpd(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_saiz:
        atom = new MP4Atom_saiz(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_saio:
        atom = new MP4Atom_saio(_data_feed, _info_collector, size, type, false, false);
        break;
    case eAtomType_senc:
        atom = new MP4Atom_senc(_data_feed, _info_collector, size, type, false, false, false);
        break;
    default:
        // MP4AtomBasic skips content when no children is specified
        atom = new MP4AtomBasic(_data_feed, _info_collector, size, type, false, false);
        break;
    }

    return atom;
}

bool MP4Atom::Parse()
{
#ifdef DEBUG
    ++_info_collector->_layer;
#endif

    bool isParsed = HasChild() ? ParseChildren() : ParseProperties();

#ifdef DEBUG
    --_info_collector->_layer;
#endif

    return isParsed;
}

bool MP4Atom::ParseProperties()
{
    return true;
}

bool MP4Atom::ParseChildren()
{
    uint32 size;
    uint32 type;
    MP4Atom* atom = 0;

    while(1)
    {
        if (_data_feed->ReadInt32(&size) == false)
            return false;

        if (_data_feed->ReadInt32(&type) == false)
            return false;

        atom = CreateNewAtom(size, type);
        if (atom == 0)
            return false;

        if (ShouldStop(type))
            _data_feed->SetStopOffset(size);

        if (atom->Parse() == false)
        {
            delete atom;
            return false;
        }

        if (atom->KeepInMemory())
            _children.push_back(atom);
        else
            delete atom;

        if (ShouldStop(type))
            break;
    }

    return true;
}

bool MP4Atom::ShouldStop(uint32 type)
{
    return (_stop_on_atom == type);
}

bool MP4Atom::HasChild() const
{
    return _has_child;
}

bool MP4Atom::KeepInMemory() const
{
    return _keep_in_memory;
}

MP4AtomBasic* MP4Atom::GetAtom(uint32 type)
{
    for (uint32 i=0; i<_children.size(); ++i)
    {
        MP4AtomBasic* atom = (MP4AtomBasic*)_children[i];
        if (atom->Type() == type)
            return atom;
    }

    return 0;
}

MP4AtomBasic::MP4AtomBasic(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4Atom(data_feed, info_collector, has_child, keep_in_memory)
    , _size(size)
    , _type(type)
    , _parsed_bytes(0)
{
    INT32TOSTR(_type, _type_name);

#ifdef DEBUG
    for (int32 i=0; i<_info_collector->_layer; ++i)
    {
        TRACE_PROPERTY(("-"));
    }
#endif

    TRACE_PROPERTY(("created atom:%s\n", _type_name));
}

MP4AtomBasic::~MP4AtomBasic()
{
}

MP4Atom* MP4AtomBasic::CreateUUIDAtom(uint32 size, uint32 type)
{
    MP4Atom* atom = 0;

    MP4_Guid guid;

    if (ReadGuid(&guid) == false)
        return 0;

    _parsed_bytes -= sizeof(MP4_Guid);

    if (guid.EqualsTo(guidSampleEncryption))
    {
        atom = new MP4Atom_senc(_data_feed, _info_collector, size, type, false, false, true);
    }
    else if (guid.EqualsTo(guidTrackFragmentExtendedHeader))
    {
        atom = new MP4Atom_uuid_tfeh(_data_feed, _info_collector, size, type, false, false, guid);
    }
    else if (guid.EqualsTo(guidTrackFragmentReferenceBox))
    {
        atom = new MP4Atom_uuid_tfrb(_data_feed, _info_collector, size, type, false, false, guid);
    }
    else if (guid.EqualsTo(guidPTSOffset))
    {
        atom = new MP4Atom_uuid_sptf(_data_feed, _info_collector, size, type, false, false, guid);
    }
    else // ignore UUID
    {
        atom = new MP4Atom_uuid_unknown(_data_feed, _info_collector, size, type, guid);
    }

    return atom;
}

bool MP4AtomBasic::Parse()
{
    if (MP4Atom::Parse() == false)
        return false;

    MP4_DEBUG_VALIDATE(_parsed_bytes == _size - 8);
    if (_parsed_bytes != _size - 8)
    {
        return false;
    }

    return true;
}

bool MP4AtomBasic::ParseProperties()
{
    if (!HasChild())
    {
        if (_size < 8)
            return false;

        if (Skip(_size - 8) == false)
            return false;
    }

    return true;
}

bool MP4AtomBasic::ParseChildren()
{
    uint32 size;
    uint32 type;
    MP4Atom* atom = 0;

    if (_size < 8)
        return false;

    uint32 data_length = _size - 8;

    while(_parsed_bytes < data_length)
    {
        if (ReadInt32(&size) == false)
            return false;

        if (ReadInt32(&type) == false)
            return false;

        if (size > (data_length - _parsed_bytes + 8))
            return false;

        atom = CreateNewAtom(size, type);
        if (atom == 0)
            return false;

        if (atom->Parse() == false)
        {
            delete atom;
            return false;
        }

        if (atom->KeepInMemory())
            _children.push_back(atom);
        else
            delete atom;

        _parsed_bytes += size - 8;

        if (ShouldStop(type))
            break;
    }

    MP4_DEBUG_VALIDATE(_parsed_bytes == data_length);
    if (_parsed_bytes != data_length)
        return false;

    return true;
}

bool MP4AtomBasic::ReadInt8( _Out_ uint8* field )
{
    if (_data_feed->ReadInt8(field) == false)
        return false;

    _parsed_bytes += 1;

    return true;
}

bool MP4AtomBasic::ReadInt16( _Out_ uint16* field )
{
    if (_data_feed->ReadInt16(field) == false)
        return false;

    _parsed_bytes += 2;

    return true;
}

bool MP4AtomBasic::ReadInt24( _Out_ uint32* field )
{
    if (_data_feed->ReadInt24(field) == false)
        return false;

    _parsed_bytes += 3;

    return true;
}

bool MP4AtomBasic::ReadInt32( _Out_ uint32* field )
{
    if (_data_feed->ReadInt32(field) == false)
        return false;

    _parsed_bytes += 4;

    return true;
}

bool MP4AtomBasic::ReadInt64( _Out_ uint64* field )
{
    if (_data_feed->ReadInt64(field) == false)
        return false;

    _parsed_bytes += 8;

    return true;
}

bool MP4AtomBasic::ReadInt64N( _Out_ uint64* field, _In_ int cb )
{
    if (_data_feed->ReadInt64N( field, cb ) == false)
        return false;

    _parsed_bytes += cb;

    return true;
}

bool MP4AtomBasic::ReadFixed16(Fixed_16* field)
{
    // read preferred rate
    if (_data_feed->ReadFixed16(field) == false)
        return false;

    _parsed_bytes += 2;

    return true;
}

bool MP4AtomBasic::ReadFixed32(Fixed_32* field)
{
    // read preferred rate
    if (_data_feed->ReadFixed32(field) == false)
        return false;

    _parsed_bytes += 4;

    return true;
}

bool MP4AtomBasic::ReadGuid(MP4_Guid* field)
{
    // read preferred rate
    if (_data_feed->ReadGuid(field) == false)
        return false;

    _parsed_bytes += 16;

    return true;
}

bool MP4AtomBasic::ReadArray(uint8* field, int32 length)
{
    // read preferred rate
    if (_data_feed->ReadArray(field, length) == false)
        return false;

    _parsed_bytes += length;

    return true;
}

bool MP4AtomBasic::Skip(uint32 count)
{
    // skip the next 'count' bytes
    if (_data_feed->Skip(count) == false)
        return false;

    _parsed_bytes += count;

    return true;
}

MP4AtomFull::MP4AtomFull(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomBasic(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _version(0)
{
    memset(_flags, 0, sizeof(_flags));
}

bool MP4AtomFull::ParseProperties()
{
    if (ReadInt8(&_version) == false)
        return false;

    if (ReadArray(_flags, 3) == false)
        return false;

    return true;
}

bool MP4AtomFull::ReadTimeField(uint64* field)
{
    if (_version == 1)
    {
        if (ReadInt64(field) == false)
            return false;
    }
    else if (_version == 0)
    {
        if (ReadInt32((uint32*)field) == false)
            return false;
    }
    else
        MP4_DEBUG_VALIDATE(false);

    return true;
}

bool MP4AtomFull::SkipTimeField(int32 count)
{
    uint64 field = 0;

    for (int32 i=0; i<count; ++i)
    {
        if (ReadTimeField(&field) == false)
            return false;
    }

    return true;
}

MP4Atom_mvhd::MP4Atom_mvhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _time_scale(0)
    , _duration(0)
    , _preferred_rate(0, 0)
    , _next_track_id(0)
{
}

bool MP4Atom_mvhd::ParseProperties()
{
    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    // skip creation-time/modification-time
    if (SkipTimeField(2) == false)
        return false;

    // read time scale
    if (ReadInt32(&_time_scale) == false)
        return false;

    // read duration
    if (ReadTimeField(&_duration) == false)
        return false;

    // read preferred rate
    if (ReadFixed32(&_preferred_rate) == false)
        return false;

    // skip the next 72 bytes
    if (Skip(72) == false)
        return false;

    // read next track id
    if (ReadInt32(&_next_track_id) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _time_scale:%d\n", _time_scale));
    TRACE_PROPERTY((">>>>>> _duration:%lld\n", _duration));
    TRACE_PROPERTY((">>>>>> _preferred_rate:%f\n", _preferred_rate.ToFloat()));
    TRACE_PROPERTY((">>>>>> _next_track_id:%d\n", _next_track_id));
    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_trak::MP4Atom_trak(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomBasic(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
    MP4TrackInfo* track = new MP4TrackInfo();
    _info_collector->_tracks.push_back((BaseTrackInfo*)track);
}

MP4Atom_trak::~MP4Atom_trak()
{
}

MP4Atom_tkhd::MP4Atom_tkhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _track_id(0)
    , _track_duration(0)
    , _track_volume(0, 0)
    , _track_width(0, 0)
    , _track_height(0, 0)
{
}

bool MP4Atom_tkhd::ParseProperties()
{
    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    // skip creation-time/modification-time
    if (SkipTimeField(2) == false)
        return false;

    // read time scale
    if (ReadInt32(&_track_id) == false)
        return false;

    // skip 4 bytes reserved
    if (Skip(4) == false)
        return false;

    // read duration
    if (ReadTimeField(&_track_duration) == false)
        return false;

    // skip 12 bytes
    if (Skip(12) == false)
        return false;

    // read volume
    if (ReadFixed16(&_track_volume) == false)
        return false;

    // skip the next 38 bytes
    if (Skip(38) == false)
        return false;

    // read track width/height
    if (ReadFixed32(&_track_width) == false)
        return false;

    if (ReadFixed32(&_track_height) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _track_id:%d\n", _track_id));
    TRACE_PROPERTY((">>>>>> _track_duration:%lld\n", _track_duration));
    TRACE_PROPERTY((">>>>>> _track_volume:%f\n", _track_volume.ToFloat()));
    TRACE_PROPERTY((">>>>>> _track_width:%f\n", _track_width.ToFloat()));
    TRACE_PROPERTY((">>>>>> _track_height:%f\n", _track_height.ToFloat()));
    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_load::MP4Atom_load(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomBasic(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _preload_start_time(0)
    , _preload_duration(0)
    , _preload_flags(0)
    , _default_hints(0)
{
}

bool MP4Atom_load::ParseProperties()
{
    // read preload start time
    if (ReadInt32(&_preload_start_time) == false)
        return false;

    // read preload duration
    if (ReadInt32(&_preload_duration) == false)
        return false;

    // read preload flags
    if (ReadInt32(&_preload_flags) == false)
        return false;

    // read default hints
    if (ReadInt32(&_default_hints) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _preload_start_time:%d\n", _preload_start_time));
    TRACE_PROPERTY((">>>>>> _preload_duration:%d\n", _preload_duration));
    TRACE_PROPERTY((">>>>>> _preload_flags:%d\n", _preload_flags));
    TRACE_PROPERTY((">>>>>> _default_hints:%d\n", _default_hints));
    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_trak_ref::MP4Atom_trak_ref(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomBasic(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
}

bool MP4Atom_trak_ref::ParseProperties()
{
    int32 id_num = (_size - 8) / sizeof(uint32);
    uint32 track_id = 0;

    TRACE_PROPERTY(("\nMP4Atom_trak_ref(%s)::ParseProperties():\n", _type_name));
    for (int32 i=0; i<id_num; ++i)
    {
        // read track id
        if (ReadInt32(&track_id) == false)
            return false;

        _track_ids.push_back(track_id);
        TRACE_PROPERTY((">>>>>> _track_id[%d]:%d\n", i, track_id));
    }
    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_mdhd::MP4Atom_mdhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _mdhd_time_scale(0)
    , _mdhd_duration(0)
    , _language_code(0)
    , _quality(0)
{
    _iso_language_code[0] = 0;
}

bool MP4Atom_mdhd::ParseProperties()
{
    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    // skip creation-time/modification-time
    if (SkipTimeField(2) == false)
        return false;

    // read time scale
    if (ReadInt32(&_mdhd_time_scale) == false)
        return false;

    // read duration
    if (ReadTimeField(&_mdhd_duration) == false)
        return false;

    // read language code
    if (ReadInt16(&_language_code) == false)
        return false;
    LanguageCode16Int2Letter();

    // read quality
    if (ReadInt16(&_quality) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _mdhd_time_scale:%d\n", _mdhd_time_scale));
    TRACE_PROPERTY((">>>>>> _mdhd_duration:%lld\n", _mdhd_duration));
    TRACE_PROPERTY((">>>>>> _language_code:%x, iso:%s\n", _language_code, _iso_language_code));
    TRACE_PROPERTY((">>>>>> _quality:%d\n", _quality));
    TRACE_PROPERTY(("\n"));

    BaseTrackInfo* track = _info_collector->_tracks.back();
    track->_track_duration = _mdhd_duration;
    track->_track_time_scale = _mdhd_time_scale;

    return true;
}

void MP4Atom_mdhd::LanguageCode16Int2Letter()
{
    if (_language_code >= 0x800)
    {
        _iso_language_code[0] = ((_language_code >> 10) & 0x001F) + 0x60;
        _iso_language_code[1] = ((_language_code >> 5) & 0x001F) + 0x60;
        _iso_language_code[2] = (_language_code & 0x001F) + 0x60;
        _iso_language_code[3] = 0;
    }
}

MP4Atom_hdlr::MP4Atom_hdlr(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _component_type(0)
    , _component_subtype(0)
    , _component_name(0)
    , _component_name_len(0)
{
    if (size > 32)
    {
        _component_name_len = size-32;
        _component_name = new uint8[_component_name_len+1];
        if (_component_name)
            _component_name[0] = 0;
    }
}

MP4Atom_hdlr::~MP4Atom_hdlr()
{
    if (_component_name)
        delete [] _component_name;
}

bool MP4Atom_hdlr::ParseProperties()
{
    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    // read component type
    if (ReadInt32(&_component_type) == false)
        return false;

    // read component subtype
    if (ReadInt32(&_component_subtype) == false)
        return false;

    // set the component type to the last created track object
    BaseTrackInfo* track = _info_collector->_tracks.back();
    track->SetMediaType(_component_subtype);

    if (Skip(12) == false)
        return false;

    if (ReadArray(_component_name, _component_name_len) == false)
        return false;

    char value[5];
    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    INT32TOSTR(_component_type, value);
    TRACE_PROPERTY((">>>>>> _component_type:%s\n", value));
    INT32TOSTR(_component_subtype, value);
    TRACE_PROPERTY((">>>>>> _component_subtype:%s\n", value));
    if (_component_name)
    {
        TRACE_PROPERTY((">>>>>> _component_name:%s\n", _component_name));
    }
    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_vmhd::MP4Atom_vmhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _graphics_mode(0)
    , _opcolor_r(0)
    , _opcolor_g(0)
    , _opcolor_b(0)
{
}

bool MP4Atom_vmhd::ParseProperties()
{
    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    // read graphcis mode
    if (ReadInt16(&_graphics_mode) == false)
        return false;

    // read opcolor
    if (ReadInt16(&_opcolor_r) == false)
        return false;
    if (ReadInt16(&_opcolor_g) == false)
        return false;
    if (ReadInt16(&_opcolor_b) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _graphics_mode:%d\n", _graphics_mode));
    TRACE_PROPERTY((">>>>>> _opcolor_r:%d\n", _opcolor_r));
    TRACE_PROPERTY((">>>>>> _opcolor_g:%d\n", _opcolor_g));
    TRACE_PROPERTY((">>>>>> _opcolor_b:%d\n", _opcolor_b));
    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_Sample_Descriptor::MP4Atom_Sample_Descriptor(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type)
    : MP4AtomBasic(data_feed, info_collector, size, type, false, false)
    , _data_ref_index(0)
    , _mp4_descriptor(0)
{
}

MP4Atom_Sample_Descriptor::~MP4Atom_Sample_Descriptor()
{
    if (_mp4_descriptor)
        delete _mp4_descriptor;
}

bool MP4Atom_Sample_Descriptor::ParseProperties()
{
    if (Skip(6) == false)
        return false;

    // read graphcis mode
    if (ReadInt16(&_data_ref_index) == false)
        return false;

    if (ParseProperties_MediaSpecific() == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_Sample_Descriptor::ParseProperties():%s\n", _type_name));
    TRACE_PROPERTY((">>>>>> _data_ref_index:%d\n", _data_ref_index));
    TRACE_PROPERTY((">>>>>> descriptor size:%d\n", _size-16));
    TRACE_PROPERTY(("\n"));

    BaseTrackInfo* track = _info_collector->_tracks.back();
    track->SetMediaFormat(_type);

    return true;
}

bool MP4Atom_Sample_Descriptor::ParseProperties_MediaSpecific()
{
    if (Skip(_size - 16) == false)
        return false;

    return true;
}

bool MP4Atom_Sample_Descriptor::ParseMediaDescriptor(uint32 count)
{
    // read media descriptor and store it
    BaseTrackInfo* track = _info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (NULL == track)
        return false;
    
    // multiple descriptor not supported yet
    MP4_DEBUG_VALIDATE(NULL == track->_media_descriptor);
    if (track->_media_descriptor)
        delete [] track->_media_descriptor;

    track->_media_descriptor = NEW_NO_THROW uint8[count];
    CHECK_ALLOC(track->_media_descriptor);
    if (track->_media_descriptor == 0)
        return false;

    track->_media_descriptor_len = count;
    if (ReadArray(track->_media_descriptor, track->_media_descriptor_len) == false)
        return false;

    return true;
}

bool MP4Atom_Sample_Descriptor::ParseDescriptorExtension(uint32 count)
{
    uint32 size = 0, type = 0;

    while (count > 0)
    {
        if (ReadInt32(&size) == false)
            return false;

        MP4_DEBUG_VALIDATE(!(count < size || size < 8));
        if (count < size || size < 8)
        {
            return false;
        }

        if (ReadInt32(&type) == false)
            return false;

        if (type == eAtomType_wave)
        {
            continue;
        }
        else if (type == eAtomType_esds)
        {
            if (Skip(4) == false)
                return false;

            if (ParseMediaDescriptor(size - 12) == false)
                return false;
        }
        else if (type == eAtomType_avcC)
        {
            if (ParseMediaDescriptor(size - 8) == false)
                return false;
        }
        else
        {
            if (Skip(size - 8) == false)
                return false;
        }

        count -= size;
    }

    return true;
}

MP4Atom_Sample_Descriptor_Audio::MP4Atom_Sample_Descriptor_Audio(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type)
    : MP4Atom_Sample_Descriptor(data_feed, info_collector, size, type)
    , _channel_count(0)
    , _sample_size(0)
    , _compression_id(0)
    , _sample_rate(0, 0)
    , _samples_per_packet(0)
    , _bytes_per_packet(0)
    , _bytes_per_frame(0)
    , _bytes_per_sample(0)
{
}

bool MP4Atom_Sample_Descriptor_Audio::ParseProperties_MediaSpecific()
{
    if (ReadInt16(&_version) == false)
        return false;

    if (_version > 1) // version > 1 not supported.
        return false;

    if (Skip(6) == false)
        return false;

    if (ReadInt16(&_channel_count) == false)
        return false;

    if (ReadInt16(&_sample_size) == false)
        return false;

    if (ReadInt16(&_compression_id) == false)
        return false;

    if (Skip(2) == false)
        return false;

    if (ReadFixed32(&_sample_rate) == false)
        return false;

    uint32 bytes_left = 0;

    if (_size > 16 + 20)
        bytes_left = _size - 16 - 20;
    else
        return false;

    if (_version == 1)
    {
        if (ReadInt32(&_samples_per_packet) == false)
            return false;

        if (ReadInt32(&_bytes_per_packet) == false)
            return false;

        if (ReadInt32(&_bytes_per_frame) == false)
            return false;

        if (ReadInt32(&_bytes_per_sample) == false)
            return false;

        if (bytes_left > 16)
            bytes_left -= 16;
        else
            return false;
    }

    if (bytes_left > 0)
    {
        // parse extensions
        if (ParseDescriptorExtension(bytes_left) == false)
            return false;
    }

    TRACE_PROPERTY(("\nMP4Atom_Sample_Descriptor_Audio::ParseProperties():%s\n", _type_name));
    TRACE_PROPERTY((">>>>>> _version:%d\n", _version));
    TRACE_PROPERTY((">>>>>> _channel_count:%d\n", _channel_count));
    TRACE_PROPERTY((">>>>>> _sample_size:%d\n", _sample_size));
    TRACE_PROPERTY((">>>>>> _sample_rate:%f\n", _sample_rate.ToFloat()));
    TRACE_PROPERTY(("\n"));

    BaseTrackInfo* track = _info_collector->_tracks.back();
    track->_channel_count = _channel_count;
    track->_sample_rate = (uint32)_sample_rate.Int16();

    return true;
}

MP4Atom_Sample_Descriptor_Video::MP4Atom_Sample_Descriptor_Video(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type)
    : MP4Atom_Sample_Descriptor(data_feed, info_collector, size, type)
    , _width(0)
    , _height(0)
    , _hori_reso(0, 0)
    , _vert_reso(0, 0)
    , _frame_count(0)
    , _compressor_name_len(0)
    , _depth(0)
    , _color_table_id(0)
{
    memset(_compressor_name, 0, 31);
}

bool MP4Atom_Sample_Descriptor_Video::ParseProperties_MediaSpecific()
{
    if (Skip(16) == false)
        return false;

    if (ReadInt16(&_width) == false)
        return false;

    if (ReadInt16(&_height) == false)
        return false;

    if (ReadFixed32(&_hori_reso) == false)
        return false;

    if (ReadFixed32(&_vert_reso) == false)
        return false;

    if (Skip(4) == false)
        return false;

    if (ReadInt16(&_frame_count) == false)
        return false;

    if (ReadInt8(&_compressor_name_len) == false)
        return false;

    if (ReadArray(_compressor_name, 31) == false)
        return false;

    //_compressor_name[_compressor_name_len] = 0;

    if (ReadInt16(&_depth) == false)
        return false;

    if (ReadInt16(&_color_table_id) == false)
        return false;

    uint32 ct_size = 0, ct_type = 0;
    if (_color_table_id == 0)
    {
        if (ReadInt32(&ct_size) == false)
            return false;

        if (ct_size < 8)
            return false;

        if (ReadInt32(&ct_type) == false)
            return false;

        if (Skip(ct_size - 8) == false)
            return false;
    }

    int32 bytes_left = _size - 16 - 70 - ct_size;

    if (bytes_left > 0)
    {
        // parse extensions
        if (ParseDescriptorExtension(bytes_left) == false)
            return false;
    }

    TRACE_PROPERTY(("\nMP4Atom_Sample_Descriptor_Video::ParseProperties():%s\n", _type_name));
    TRACE_PROPERTY((">>>>>> _width:%d, _height:%d\n", _width, _height));
    TRACE_PROPERTY((">>>>>> _hori_reso:%f, _vert_reso:%f\n", _hori_reso.ToFloat(), _vert_reso.ToFloat()));
    TRACE_PROPERTY((">>>>>> _frame_count:%d\n", _frame_count));
    TRACE_PROPERTY((">>>>>> _compressor_name:%s\n", _compressor_name));
    TRACE_PROPERTY((">>>>>> _depth:%d\n", _depth));
    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_stsd::MP4Atom_stsd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _table_length(0)
{
}

MP4Atom_stsd::~MP4Atom_stsd()
{
    for (uint32 i=0; i<_description_table.size(); ++i)
        delete _description_table[i];
}

bool MP4Atom_stsd::ParseProperties()
{
    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    if (ReadInt32(&_table_length) == false)
        return false;

    uint32 size = 0;
    uint32 type = 0;
    MP4Atom_Sample_Descriptor* atom = 0;

    for (uint32 i=0; i<_table_length; ++i)
    {
        if (ReadInt32(&size) == false)
            return false;

        if (ReadInt32(&type) == false)
            return false;

        BaseTrackInfo* track = _info_collector->_tracks.back();
        if (track->_media_type == eAtomType_soun)
            atom = new MP4Atom_Sample_Descriptor_Audio(_data_feed, _info_collector, size, type);
        else if (track->_media_type == eAtomType_vide)
            atom = new MP4Atom_Sample_Descriptor_Video(_data_feed, _info_collector, size, type);
        else
            atom = new MP4Atom_Sample_Descriptor(_data_feed, _info_collector, size, type);

        if (atom == 0)
            return false;

        if (atom->Parse() == false)
            return false;

        _parsed_bytes += size - 8;

        _description_table.push_back(atom);
    }

    return true;
}

MP4Atom_stts::MP4Atom_stts(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _table_length(0)
{
}

bool MP4Atom_stts::ParseProperties()
{
    MP4TrackInfo* track = (MP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    if (ReadInt32(&_table_length) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _table_length:%d\n", _table_length));

    Time2SampleTableEntry time2sample;
    uint32 total_sample_count = 0;
    for (uint32 i=0; i<_table_length; ++i)
    {
        if (ReadInt32(&time2sample._sample_count) == false)
            return false;

        if (ReadInt32(&time2sample._sample_duration) == false)
            return false;

        track->_time2sample_table.push_back(time2sample);

        total_sample_count += time2sample._sample_count;

        TRACE_PROPERTY((">>>>>> _time2sample_table[%d]:%d, %d\n", i, time2sample._sample_count, time2sample._sample_duration));
    }

    track->_frames_count = total_sample_count;

    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_stss::MP4Atom_stss(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _table_length(0)
{
}

bool MP4Atom_stss::ParseProperties()
{
    MP4TrackInfo* track = (MP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    if (ReadInt32(&_table_length) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _table_length:%d\n", _table_length));

    uint32 sync_sample_number = 0;
    for (uint32 i=0; i<_table_length; ++i)
    {
        if (ReadInt32(&sync_sample_number) == false)
            return false;

        track->_sync_sample_table.push_back(sync_sample_number);

        TRACE_PROPERTY((">>>>>> _sync_sample_table[%d]:%d\n", i, sync_sample_number));
    }

    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_stsc::MP4Atom_stsc(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _table_length(0)
{
}

bool MP4Atom_stsc::ParseProperties()
{
    MP4TrackInfo* track = (MP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    if (ReadInt32(&_table_length) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _table_length:%d\n", _table_length));

    Sample2ChunkTableEntry sample2chunk;
    for (uint32 i=0; i<_table_length; ++i)
    {
        if (ReadInt32(&sample2chunk._first_chunk) == false)
            return false;

        if (ReadInt32(&sample2chunk._samples_per_chunk) == false)
            return false;

        if (ReadInt32(&sample2chunk._sample_desc_id) == false)
            return false;

        track->_sample2chunk_table.push_back(sample2chunk);

        TRACE_PROPERTY((">>>>>> _sample2chunk_table[%d]:%d, %d, %d\n", i, sample2chunk._first_chunk, sample2chunk._samples_per_chunk, sample2chunk._sample_desc_id));
    }

    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_stsz::MP4Atom_stsz(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _sample_size(0)
    , _table_length(0)
{
}

bool MP4Atom_stsz::ParseProperties()
{
    MP4TrackInfo* track = (MP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    if (ReadInt32(&_sample_size) == false)
        return false;

    if (ReadInt32(&_table_length) == false)
        return false;

    if (_sample_size > 0)
        _table_length = 0;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _sample_size:%d\n", _sample_size));
    TRACE_PROPERTY((">>>>>> _table_length:%d\n", _table_length));

    uint32 sample_size = 0;
    for (uint32 i=0; i<_table_length; ++i)
    {
        if (ReadInt32(&sample_size) == false)
            return false;

        track->_sample_size_table.push_back(sample_size);

        TRACE_PROPERTY((">>>>>> _sample_size_table[%d]:%d\n", i, sample_size));
    }

    track->_single_sample_size = _sample_size;

    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_stco::MP4Atom_stco(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _table_length(0)
{
}

bool MP4Atom_stco::ParseProperties()
{
    MP4TrackInfo* track = (MP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    if (ReadInt32(&_table_length) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _table_length:%d\n", _table_length));

    uint32 chunk_offset = 0;
    for (uint32 i=0; i<_table_length; ++i)
    {
        if (ReadInt32(&chunk_offset) == false)
            return false;

        track->_chunk_offset_table.push_back(chunk_offset);

        TRACE_PROPERTY((">>>>>> _chunk_offset_table[%d]:%d\n", i, chunk_offset));
    }

    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_ctts::MP4Atom_ctts(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    ,  _table_length(0)
{
}

bool MP4Atom_ctts::ParseProperties()
{
    MP4TrackInfo* track = (MP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    if (ReadInt32(&_table_length) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _table_length:%d\n", _table_length));

    CompTime2SampleTableEntry comp_time2sample;

    for (uint32 i=0; i<_table_length; ++i)
    {
        if (ReadInt32(&comp_time2sample._sample_count) == false)
            return false;

        if (ReadInt32(&comp_time2sample._samples_offset) == false)
            return false;

        track->_comp_time2sample_table.push_back(comp_time2sample);

        TRACE_PROPERTY((">>>>>> _comp_time2sample_table[%d]:%d, %d\n", i, comp_time2sample._sample_count, comp_time2sample._samples_offset));
    }

    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_stbl::MP4Atom_stbl(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomBasic(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
}

MP4Atom_stbl::~MP4Atom_stbl()
{
}

bool MP4Atom_stbl::Parse()
{
    if (MP4AtomBasic::Parse() == false)
        return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////
// f-mp4 atoms below
////////

MP4Atom_moof::MP4Atom_moof(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomBasic(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
    // IIS Smooth Streaming spec dictates one track per fragment
    FMP4TrackInfo* track = NEW_NO_THROW FMP4TrackInfo();
    CHECK_ALLOC(track);

    track->SetMediaType(((FMP4Info*)_info_collector)->_fragment_media_type);
    _info_collector->_tracks.push_back((BaseTrackInfo*)track);
}


// mfhd
MP4Atom_mfhd::MP4Atom_mfhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _sequence_number(0)
{
}

bool MP4Atom_mfhd::ParseProperties()
{
    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    // read sequence number
    if (ReadInt32(&_sequence_number) == false)
        return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _sequence_number:%d\n", _sequence_number));
    TRACE_PROPERTY(("\n"));

    track->_sequence_number = _sequence_number;

    return true;
}

MP4Atom_tfhd::MP4Atom_tfhd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _track_id(0)
    , _base_data_offset(0)
    , _sample_description_index(0)
    , _default_sample_duration(0)
    , _default_sample_size(0)
    , _default_sample_flags(0)
{
}

bool MP4Atom_tfhd::ParseProperties()
{
    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    // read track id
    if (ReadInt32(&_track_id) == false)
        return false;

    if (_flags[2] & 0x01)
        // read base data offset
        if (ReadInt64(&_base_data_offset) == false)
            return false;

    if (_flags[2] & 0x02)
        // read sample description index
        if (ReadInt32(&_sample_description_index) == false)
            return false;

    if (_flags[2] & 0x08)
        // read default sample duration
        if (ReadInt32(&_default_sample_duration) == false)
            return false;

    if (_flags[2] & 0x10)
        // read default sample size
        if (ReadInt32(&_default_sample_size) == false)
            return false;

    if (_flags[2] & 0x20)
        // read default sample flags
        if (ReadInt32(&_default_sample_flags) == false)
            return false;

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _track_id:%d\n", _track_id));
    TRACE_PROPERTY((">>>>>> _base_data_offset:%lld\n", _base_data_offset));
    TRACE_PROPERTY((">>>>>> _sample_description_index:%d\n", _sample_description_index));
    TRACE_PROPERTY((">>>>>> _default_sample_duration:%d\n", _default_sample_duration));
    TRACE_PROPERTY((">>>>>> _default_sample_size:%d\n", _default_sample_size));
    TRACE_PROPERTY((">>>>>> _default_sample_flags:%d\n", _default_sample_flags));
    TRACE_PROPERTY(("\n"));

    track->_track_id = _track_id;
    track->_base_data_offset = _base_data_offset;
    track->_sample_description_index = _sample_description_index;
    track->_default_sample_duration = _default_sample_duration;
    track->_default_sample_size = _default_sample_size;
    track->_default_sample_flags = _default_sample_flags;

    return true;
}

MP4Atom_trun::MP4Atom_trun(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    ,  _sample_count(0)
    , _data_offset(0)
    , _first_sample_flags(0)
{
}

bool MP4Atom_trun::ParseProperties()
{
    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    // read sample count
    if (ReadInt32(&_sample_count) == false)
        return false;

    if (_flags[2] & 0x01)
        // read base data offset
        if (ReadInt32(&_data_offset) == false)
            return false;

    if (_flags[2] & 0x04)
        // read base data offset
        if (ReadInt32(&_first_sample_flags) == false)
            return false;

    if (ReadSampleTable(track) == false)
    {
        return false;
    }

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY((">>>>>> _sample_count:%d\n", _sample_count));
    TRACE_PROPERTY((">>>>>> _data_offset:%d\n", _data_offset));
    TRACE_PROPERTY((">>>>>> _first_sample_flags:%d\n", _first_sample_flags));
    TRACE_PROPERTY(("\n"));

    track->_frames_count = _sample_count;

    return true;
}

bool MP4Atom_trun::ReadSampleTable(FMP4TrackInfo* track)
{
    CHECK_ALLOC(track);

    // allocate tables
    if (track->AllocateSampleTable(_sample_count, _flags[1]) == false)
    {
        track->ReleaseSampleTable();
        return false;
    }

    // read the table data
    for (uint32 i=0; i<_sample_count; ++i)
    {
        if (track->_sample_duration_table)
            if (ReadInt32(&(track->_sample_duration_table[i])) == false)
                return false;

        if (track->_sample_size_table)
            if (ReadInt32(&(track->_sample_size_table[i])) == false)
                return false;

        if (track->_sample_flags_table)
            if (ReadInt32(&(track->_sample_flags_table[i])) == false)
                return false;

        if (track->_sample_comptimeoffset_table)
            if (ReadInt32((uint32*)&(track->_sample_comptimeoffset_table[i])) == false)
                return false;
    }

    return true;
}



MP4Atom_sdtp::MP4Atom_sdtp(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
}

bool MP4Atom_sdtp::ParseProperties()
{
    if (_info_collector->IsFMP4() == false)
        return MP4AtomBasic::ParseProperties();

    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    MP4_DEBUG_VALIDATE(track->_sdtp_table == 0);
    track->_sdtp_table = NEW_NO_THROW uint8[track->_sample_table_len];
    CHECK_ALLOC(track->_sdtp_table);
    if (track->_sdtp_table == 0)
        return false;

    if (ReadArray(track->_sdtp_table, track->_sample_table_len) == false)
        return false;

    if (_size - _parsed_bytes > 8)
    {
        TRACE_PROPERTY(("MP4Atom_%s::ParseProperties(), uncompliant format, EE3 bug???", _type_name));
        if (Skip(_size-_parsed_bytes-8) == false)
            return false;
    }

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_uuid::MP4Atom_uuid(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory, MP4_Guid& guid)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _guid(guid)
{
}

bool MP4Atom_uuid::ParseProperties()
{
    _parsed_bytes += sizeof(MP4_Guid);

    return true;
}


// sample encryption (uuid or senc)
MP4Atom_senc::MP4Atom_senc(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory, bool fromGuid)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
    , _fromGuid (fromGuid)
{
}

bool MP4Atom_senc::ParseProperties()
{
    if (_fromGuid)
    {
        // This is actually the UUID box with the senc GUID
        // we need to update the parsed bytes since we have already parsed the GUID
        _parsed_bytes += sizeof(MP4_Guid);
    }

    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (NULL == track)
    {
        return false;
    }

    if (NULL == track->_se_info)
    {
        SampleEncryptionInfo* seInfo = NEW_NO_THROW SampleEncryptionInfo;
        CHECK_ALLOC(seInfo);
        if (NULL == seInfo)
        {
            return (false);
        }
        else
        {
            track->_se_info = seInfo;
        }
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    if (_flags[2] & 0x01)
    {
        // This is an unsupported scenario based on PIFF 1.3 spec
        return (false);
    }

    if (!ReadInt32(&track->_se_info->_sample_count))
    {
        return false;
    }

    if (0 != track->_se_info->_sample_count)
    {
        if (NULL != track->_se_info->_sub_sample_table)
        {
            delete[] track->_se_info->_sub_sample_table;
        }
        track->_se_info->_sub_sample_table = new SampleEncryptionSubSampleInfo[track->_se_info->_sample_count];
        if (NULL == track->_se_info->_sub_sample_table)
        {
            return false;
        }
    }

    SampleEncryptionSubSampleInfo* subsample_info;
    uint32 i;

    // get the SampleEncryptionInfo to figure out IV size
    std::vector<CencSampleEncryptionInformationAudioGroupEntry*> sampleEncryptionInfoEntries;
    std::vector<int32> sampleEncryptionInfoIndexes;
    if(!track->GetSampleEncryptionInfo(&sampleEncryptionInfoEntries, &sampleEncryptionInfoIndexes))
    {
        return false;
    }

    // validate the sampleCount
    if ( !sampleEncryptionInfoIndexes.empty())
    {
        if (sampleEncryptionInfoIndexes.size() != track->_se_info->_sample_count)
        {
            TRACE_ERROR(("Sample count in senc %d and sbgp %d box is not equal", 
                    track->_se_info->_sampleAuxInfoSizeData->_sampleCount,
                    sampleEncryptionInfoIndexes.size()));
            return false;
        }
    }

    // record the position of the subSample info
    track->_se_info->_senc_sub_sample_startPos = _data_feed->GetCurrentPos();

    for (i = 0, subsample_info = track->_se_info->_sub_sample_table; i < track->_se_info->_sample_count; ++i, ++subsample_info)
    {
        uint8 sampleIdentifierSize  = track->_se_info->_iv_size;
        if( !sampleEncryptionInfoIndexes.empty() )
        {
            sampleIdentifierSize = sampleEncryptionInfoEntries[sampleEncryptionInfoIndexes[i]]->_ivSize;
        }

        if(sampleIdentifierSize == 0)
        {
            return false;
        }

        if (!subsample_info->AllocateIV(sampleIdentifierSize))
        {
            return false;
        }

        if (!ReadInt64N( &subsample_info->_iv, sampleIdentifierSize ) )
        {
            return false;
        }

        // Use subsample encryption
        if (_flags[2] & 0x02)
        {
            if (!ReadInt16(&subsample_info->_entry_count))
            {
                return false;
            }

            subsample_info->AllocateEntries(subsample_info->_entry_count);

            for (int32 j=0; j<subsample_info->_entry_count; ++j)
            {
                if (!ReadInt16(&subsample_info->_clear_data[j]))
                {
                    return false;
                }

                if (!ReadInt32(&subsample_info->_encrypted_data[j]))
                {
                    return false;
                }
            }
        }
    }

    TRACE_PROPERTY(("\nMP4Atom_uuid_se::ParseProperties():\n"));
    TRACE_PROPERTY(("\n"));

    return true;
}

// track fragment extended header (uuid)
MP4Atom_uuid_tfeh::MP4Atom_uuid_tfeh(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory, MP4_Guid& guid)
    : MP4Atom_uuid(data_feed, info_collector, size, type, has_child, keep_in_memory, guid)
{
}

bool MP4Atom_uuid_tfeh::ParseProperties()
{
    MP4Atom_uuid::ParseProperties();

    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    if (_version == 1)
    {
        if (ReadInt64(&track->_live_frag_absolute_time) == false)
            return false;

        if (ReadInt64(&track->_live_frag_duration) == false)
            return false;
    }
    else
    {
        if (ReadInt32((uint32*)&track->_live_frag_absolute_time) == false)
            return false;

        if (ReadInt32((uint32*)&track->_live_frag_duration) == false)
            return false;
    }

    if (_flags[2] & 0x08)
    {
        if (ReadInt64(&track->_live_frag_ntp_timestamp) == false)
            return false;
    }

    TRACE_PROPERTY(("\nMP4Atom_uuid_tfeh::ParseProperties():\n"));
    TRACE_PROPERTY(("\n"));

    return true;
}

// track fragment reference box (uuid)
MP4Atom_uuid_tfrb::MP4Atom_uuid_tfrb(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory, MP4_Guid& guid)
    : MP4Atom_uuid(data_feed, info_collector, size, type, has_child, keep_in_memory, guid)
{
}

bool MP4Atom_uuid_tfrb::ParseProperties()
{
    MP4Atom_uuid::ParseProperties();

    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    uint8 frag_count = 0;
    if (ReadInt8(&frag_count) == false)
        return false;

    if (frag_count == 0)
        return true;

    if (track->AllocateFragmentTable(frag_count) == false)
        return false;

    for (int32 i=0; i<frag_count; ++i)
    {
        uint64 fragment_absolute_time;
        uint64 fragment_duration;

        if (_version == 1)
        {
            if (ReadInt64(&fragment_absolute_time) == false)
                return false;

            if (ReadInt64(&fragment_duration) == false)
                return false;
        }
        else
        {
            uint32 temp;

            if (ReadInt32(&temp) == false)
                return false;
            fragment_absolute_time = (uint32) temp;

            if (ReadInt32(&temp) == false)
                return false;
            fragment_duration = (uint32) temp;
        }

        track->_fragment_absolute_time_table[i] = fragment_absolute_time;
        track->_fragment_duration_table[i] = fragment_duration;
    }

    TRACE_PROPERTY(("\nMP4Atom_uuid_tfrb::ParseProperties():\n"));
    TRACE_PROPERTY(("\n"));

    return true;
}

// sample presentation time offset (uuid) -- not defined in 2.x spec. obseleted?
MP4Atom_uuid_sptf::MP4Atom_uuid_sptf(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory, MP4_Guid& guid)
    : MP4Atom_uuid(data_feed, info_collector, size, type, has_child, keep_in_memory, guid)
{
}

bool MP4Atom_uuid_sptf::ParseProperties()
{
    MP4Atom_uuid::ParseProperties();

    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    uint32 sample_count;
    if (ReadInt32(&sample_count) == false)
        return false;

    MP4_DEBUG_VALIDATE(sample_count == track->_sample_table_len);
    if (sample_count != track->_sample_table_len)
    {
        return false;
    }

    MP4_DEBUG_VALIDATE(NULL == track->_sdtp_table);
    delete track->_sample_ptso_table;
    track->_sample_ptso_table = NEW_NO_THROW uint32[sample_count];
    CHECK_ALLOC(track->_sample_ptso_table);
    if (track->_sdtp_table == 0)
        return false;

    for (uint32 i=0; i<sample_count; ++i)
    {
        if (ReadInt32(&track->_sample_ptso_table[i]) == false)
            return false;
    }

    TRACE_PROPERTY(("\nMP4Atom_uuid_sptf::ParseProperties():\n"));
    TRACE_PROPERTY(("\n"));

    return true;
}

MP4Atom_uuid_unknown::MP4Atom_uuid_unknown(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, MP4_Guid& guid)
    : MP4Atom_uuid(data_feed, info_collector, size, type, false, false, guid)
{
}

bool MP4Atom_uuid_unknown::ParseProperties()
    {
        MP4Atom_uuid::ParseProperties();
    
        if (_size - _parsed_bytes > 8)
        {
            uint32 bytesToSkip = _size - _parsed_bytes - 8;
            TRACE_PROPERTY(("\nMP4Atom_uuid_unknown::ParseProperties(): skipping %u bytes for GUID[%x]\n", 
                bytesToSkip, _guid.GetData1Ptr()[0]));
            if (!Skip(bytesToSkip))
            {
                return false;
            }
        }

        TRACE_PROPERTY(("\nMP4Atom_uuid_unknown::ParseProperties():\n"));
        TRACE_PROPERTY(("\n"));
    
        return true;
    }


MP4Atom_drIV::MP4Atom_drIV(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomBasic(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
}

bool MP4Atom_drIV::ParseProperties()
{
    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    uint32 fixed_iv_size;
    if (ReadInt32(&fixed_iv_size) == false)
        return false;

    uint32 items_count;
    if (ReadInt32(&items_count) == false)
        return false;

    MP4_DEBUG_VALIDATE(items_count == track->_sample_table_len);
    if (items_count != track->_sample_table_len)
    {
        return false;
    }

    DrmInfo* drmInfo = NEW_NO_THROW DrmInfo;
    CHECK_ALLOC(drmInfo);
    if (drmInfo == 0)
    {
        return false;
    }
    else
        track->_drm_info = drmInfo;

    drmInfo->_sample_count = items_count;

    drmInfo->_iv_data_table = NEW_NO_THROW uint8*[items_count];
    CHECK_ALLOC(drmInfo->_iv_data_table);
    if (drmInfo->_iv_data_table == 0)
        return false;

    drmInfo->_iv_length_table = NEW_NO_THROW uint32[items_count];
    CHECK_ALLOC(drmInfo->_iv_length_table);
    if (drmInfo->_iv_length_table == 0)
        return false;

    for (uint32 i=0; i<items_count; ++i)
    {
        uint32 iv_size = fixed_iv_size;
        if (iv_size == 0)
        {
            if (ReadInt32(&iv_size) == false)
                return false;
        }

        drmInfo->_iv_length_table[i] = iv_size;

        if (iv_size)
        {
            drmInfo->_iv_data_table[i] = NEW_NO_THROW uint8[iv_size];
            CHECK_ALLOC(drmInfo->_iv_data_table[i]);
            if (drmInfo->_iv_data_table[i] == NULL)
                return false;

            if (ReadArray(drmInfo->_iv_data_table[i], iv_size) == false)
                return false;
        }
        else
            drmInfo->_iv_data_table[i] = 0;
    }

    TRACE_PROPERTY(("\nMP4Atom_%s::ParseProperties():\n", _type_name));
    TRACE_PROPERTY(("\n"));

    return true;
}

// protection system specific header
MP4Atom_pssh::MP4Atom_pssh(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
}

bool MP4Atom_pssh::ParseProperties()
{
    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (track == 0)
    {
        return false;
    }

    ProtectionSystemSpecificInfo* pssInfo = NEW_NO_THROW ProtectionSystemSpecificInfo;
    CHECK_ALLOC(pssInfo);
    if (pssInfo == 0)
    {
        return false;
    }
    else
        track->_pss_info = pssInfo;

    // version/flags
    if (MP4AtomFull::ParseProperties() == false)
        return false;

    pssInfo->_system_id = NEW_NO_THROW MP4_Guid;
    CHECK_ALLOC(pssInfo->_system_id);
    if (pssInfo->_system_id == 0)
        return false;

    if (ReadGuid(pssInfo->_system_id) == false)
        return false;

    if (ReadInt32(&pssInfo->_data_size) == false)
        return false;

    if (pssInfo->_data_size)
    {
        pssInfo->_data = NEW_NO_THROW uint8[pssInfo->_data_size];
        CHECK_ALLOC(pssInfo->_data);
        if (pssInfo->_data == 0)
            return false;

        if (ReadArray(pssInfo->_data, pssInfo->_data_size) == false)
            return false;
    }
    else
    {
        pssInfo->_data = 0;
    }

    TRACE_PROPERTY(("\nMP4Atom_pssh::ParseProperties():\n"));
    TRACE_PROPERTY(("\n"));

    return true;
}

// Sample To Group Box
MP4Atom_sbgp::MP4Atom_sbgp(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
}

bool MP4Atom_sbgp::ParseProperties()
{
    uint32 groupingType = 0;
    uint32 i = 0;

    if (!_info_collector->IsFMP4())
    {
        return (MP4AtomBasic::ParseProperties());
    }

    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (NULL == track)
    {
        return (false);
    }

    // version/flags
    if (!MP4AtomFull::ParseProperties())
    {
        return (false);
    }

    if (1 == _version)
    {
        uint32 groupingType = 0;
        // Skip the Grouping Type Parameters
        if (!ReadInt32(&groupingType))
        {
            return (false);
        }
    }

    if (!ReadInt32(&groupingType))
    {
        return (false);
    }

    if (eAtomType_seig == groupingType)
    {
        if (NULL == track->_se_info)
        {
            SampleEncryptionInfo* seInfo = NEW_NO_THROW SampleEncryptionInfo;
            CHECK_ALLOC(seInfo);
            if (NULL == seInfo)
            {
                return (false);
            }
            else
            {
                track->_se_info = seInfo;
            }
        }

        if (NULL != track->_se_info->_cencSampleGroupingData)
        {
            // two sample to group box with grouping type 'seig'
            return (false);
        }

        // seig group, read out the entries
        SampleGroupingData* groupingData = NEW_NO_THROW SampleGroupingData();
        CHECK_ALLOC(groupingData);
        if (NULL == groupingData)
        {
            return (false);
        }

        if (!ReadInt32(&groupingData->_entryCount))
        {
            return (false);
        }

        groupingData->_entries = NEW_NO_THROW SampleGroupingEntry*[groupingData->_entryCount];
        CHECK_ALLOC(groupingData->_entries);
        if (NULL == groupingData->_entries)
        {
            return (false);
        }

        for (i = 0; i < groupingData->_entryCount; ++i)
        {
            groupingData->_entries[i] = NEW_NO_THROW SampleGroupingEntry;
            CHECK_ALLOC(groupingData->_entries[i]);
            if (NULL == groupingData->_entries[i])
            {
                return (false);
            }

            if (!ReadInt32(&groupingData->_entries[i]->_sampleCount))
            {
                return (false);
            }

            if (!ReadInt32(&groupingData->_entries[i]->_gpIndex))
            {
                return (false);
            }
        }
        track->_se_info->_cencSampleGroupingData = groupingData;
    }

    return (true);
}

// Sample Group Description Box
MP4Atom_sgpd::MP4Atom_sgpd(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
}
bool MP4Atom_sgpd::ParseProperties()
{
    uint32 groupingType = 0;
    uint32 i = 0;

    if (!_info_collector->IsFMP4())
    {
        return (MP4AtomBasic::ParseProperties());
    }

    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (NULL == track)
    {
        return (false);
    }

    // version/flags
    if (!MP4AtomFull::ParseProperties())
    {
        return (false);
    }

    if (!ReadInt32(&groupingType))
    {
        return (false);
    }

    if (eAtomType_seig == groupingType)
    {
        if (NULL == track->_se_info)
        {
            SampleEncryptionInfo* seInfo = NEW_NO_THROW SampleEncryptionInfo;
            CHECK_ALLOC(seInfo);
            if (NULL == seInfo)
            {
                return (false);
            }
            else
            {
                track->_se_info = seInfo;
            }
        }

        if (NULL != track->_se_info->_cencSampleGroupDescriptionData)
        {
            // two sample group description box with grouping type 'seig'
            return (false);
        }

        SampleGroupDescriptionData* descriptionData = NEW_NO_THROW SampleGroupDescriptionData();
        CHECK_ALLOC(descriptionData);
        if (NULL == descriptionData)
        {
            return (false);
        }

        if (1 == _version)
        {
            if (!ReadInt32(&descriptionData->_defaultLength))
            {
                return (false);
            }
        }

        if (!ReadInt32(&descriptionData->_entryCount))
        {
            return (false);
        }

        descriptionData->_entries = NEW_NO_THROW CencSampleEncryptionInformationAudioGroupEntry*[descriptionData->_entryCount];
        CHECK_ALLOC(descriptionData->_entries);
        for (i = 0; i < descriptionData->_entryCount; ++i)
        {
            descriptionData->_entries[i] = NEW_NO_THROW CencSampleEncryptionInformationAudioGroupEntry();
            CHECK_ALLOC(descriptionData->_entries[i]);
            if (NULL == descriptionData->_entries[i])
            {
                return (false);
            }

            if (1 == _version && 0 == descriptionData->_defaultLength)
            {
                if (!ReadInt32(&descriptionData->_entries[i]->_descriptionLength))
                {
                    return (false);
                }
            }

            // 20 bytes is the length for Algorithm ID, IV size and Kid
            if (descriptionData->_defaultLength >= 20 || descriptionData->_entries[i]->_descriptionLength >= 20)
            {
                if (!ReadInt24(&descriptionData->_entries[i]->_algId))
                {
                    return (false);
                }

                if (!ReadInt8(&descriptionData->_entries[i]->_ivSize))
                {
                    return (false);
                }

                descriptionData->_entries[i]->_kid = NEW_NO_THROW MP4_Guid;
                CHECK_ALLOC(descriptionData->_entries[i]->_kid);
                if (NULL == descriptionData->_entries[i]->_kid)
                {
                    return (false);
                }

                if (!ReadGuid(descriptionData->_entries[i]->_kid))
                {
                    return (false);
                }
            }
        }
        track->_se_info->_cencSampleGroupDescriptionData = descriptionData;

        // override the algorithm Id and key Id
        track->_se_info->_kid = NEW_NO_THROW MP4_Guid;
        *track->_se_info->_kid = *descriptionData->_entries[0]->_kid;
        track->_se_info->_algorithm_id = descriptionData->_entries[0]->_algId;
    }

    return (true);
}

// Sample Auxiliary Information Box
MP4Atom_saiz::MP4Atom_saiz(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
}

bool MP4Atom_saiz::ParseProperties()
{
    uint32 auxInfoType = 0;
    uint32 auxInfoTypeParam = 0;

    if (!_info_collector->IsFMP4())
    {
        return (MP4AtomBasic::ParseProperties());
    }

    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (NULL == track)
    {
        return (false);
    }

    // version/flags
    if (!MP4AtomFull::ParseProperties())
    {
        return (false);
    }

    if (_flags[2] & 0x01)
    {
        if (!ReadInt32(&auxInfoType))
        {
            return (false);
        }

        if (!ReadInt32(&auxInfoTypeParam))
        {
            return (false);
        }
    }

    bool isSampleAuxInfoSupported = ((auxInfoType == eAtomType_cenc) && (0 == auxInfoTypeParam));

    // we only read the sample aux info size box and sample aux info offset box with 
    // aux_info_type being 'cenc' and aux_info_type_parameter being 0
    if (isSampleAuxInfoSupported)
    {
        if (NULL == track->_se_info)
        {
            SampleEncryptionInfo* seInfo = NEW_NO_THROW SampleEncryptionInfo;
            CHECK_ALLOC(seInfo);
            if (NULL == seInfo)
            {
                return (false);
            }
            else
            {
                track->_se_info = seInfo;
            }
        }

        if (NULL != track->_se_info->_sampleAuxInfoSizeData)
        {
            return (false);
        }

        SampleAuxiliaryInformationSizeData* sampleInfoSizeData = NEW_NO_THROW SampleAuxiliaryInformationSizeData();
        CHECK_ALLOC(sampleInfoSizeData);
        if (NULL == sampleInfoSizeData)
        {
            return (false);
        }

        if (!ReadInt8(&sampleInfoSizeData->_defaultSampleInfoSize))
        {
            return (false);
        }

        if (!ReadInt32(&sampleInfoSizeData->_sampleCount))
        {
            return (false);
        }

        if (0 == sampleInfoSizeData->_defaultSampleInfoSize)
        {
            sampleInfoSizeData->_sampleInfoSize = NEW_NO_THROW uint8[sampleInfoSizeData->_sampleCount];
            CHECK_ALLOC(sampleInfoSizeData->_sampleInfoSize);
            if (NULL == sampleInfoSizeData->_sampleInfoSize)
            {
                return (false);
            }

            if (!ReadArray(sampleInfoSizeData->_sampleInfoSize, sampleInfoSizeData->_sampleCount))
            {
                return (false);
            }
        }
        track->_se_info->_sampleAuxInfoSizeData = sampleInfoSizeData;
    }
    else
    {
        // skip the rest of the box
        if (Skip(_size - kVersionFlagsSizeBytes) == false)
        {
            return false;
        }
    }

    return (true);
}

// Sample Auxiliary Information Offset Box
MP4Atom_saio::MP4Atom_saio(MP4Feed* data_feed, BaseMP4Info* info_collector, uint32 size, uint32 type, bool has_child, bool keep_in_memory)
    : MP4AtomFull(data_feed, info_collector, size, type, has_child, keep_in_memory)
{
}

bool MP4Atom_saio::ParseProperties()
{
    uint32 auxInfoType = 0;
    uint32 auxInfoTypeParam = 0;

    if (!_info_collector->IsFMP4())
    {
        return (MP4AtomBasic::ParseProperties());
    }

    FMP4TrackInfo* track = (FMP4TrackInfo*)_info_collector->_tracks.back();
    MP4_DEBUG_VALIDATE(track);
    if (NULL == track)
    {
        return (false);
    }

    // version/flags
    if (!MP4AtomFull::ParseProperties())
    {
        return (false);
    }

    if (_flags[2] & 0x01)
    {
        if (!ReadInt32(&auxInfoType))
        {
            return (false);
        }

        if (!ReadInt32(&auxInfoTypeParam))
        {
            return (false);
        }
    }

    bool isSampleAuxInfoSupported = ((auxInfoType == eAtomType_cenc) && (0 == auxInfoTypeParam));

    // we only read the sample aux info size box and sample aux info offset box with 
    // aux_info_type being 'cenc' and aux_info_type_parameter being 0
    if (isSampleAuxInfoSupported)
    {
        if (NULL == track->_se_info)
        {
            SampleEncryptionInfo* seInfo = NEW_NO_THROW SampleEncryptionInfo;
            CHECK_ALLOC(seInfo);
            if (NULL == seInfo)
            {
                return (false);
            }
            else
            {
                track->_se_info = seInfo;
            }
        }

        
        if (NULL != track->_se_info->_sampleAuxInfoOffsetData)
        {
            return (false);
        }

        SampleAuxiliaryInformationOffsetData* sampleInfoOffsetData = NEW_NO_THROW SampleAuxiliaryInformationOffsetData();
        CHECK_ALLOC(sampleInfoOffsetData);
        if (!ReadInt32(&sampleInfoOffsetData->_entryCount))
        {
            return (false);
        }

        if (0 == _version)
        {
            sampleInfoOffsetData->_offsets = NEW_NO_THROW uint32[sampleInfoOffsetData->_entryCount];
            CHECK_ALLOC(sampleInfoOffsetData->_offsets);
            for (uint i = 0; i < sampleInfoOffsetData->_entryCount; ++i)
            {
                if (!ReadInt32(&sampleInfoOffsetData->_offsets[i]))
                {
                    return (false);
                }
            }
        }
        else
        {
            sampleInfoOffsetData->_loffsets = NEW_NO_THROW uint64[sampleInfoOffsetData->_entryCount];
            CHECK_ALLOC(sampleInfoOffsetData->_loffsets);
            for (uint i = 0; i < sampleInfoOffsetData->_entryCount; ++i)
            {
                if (ReadInt64(&sampleInfoOffsetData->_loffsets[i]))
                {
                    return (false);
                }
            }
        }
        track->_se_info->_sampleAuxInfoOffsetData = sampleInfoOffsetData;
    }
    else
    {
        // skip the rest of the box
        if (Skip(_size - kVersionFlagsSizeBytes) == false)
        {
            return false;
        }
    }
    return (true);
}
