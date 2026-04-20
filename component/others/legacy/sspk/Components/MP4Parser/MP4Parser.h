///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MP4Info.h"
#include "MP4Feed.h"

class BaseMP4Parser
{
public:
    BaseMP4Parser(_In_ MP4Atom::EAtomType stopAtom);
    virtual ~BaseMP4Parser() {};
    virtual void Init(_In_ MP4Feed::MP4FeedType type, _In_ void* source, _In_ uint32 length);
    virtual void Prepare(_In_ MP4Atom* mp4Atom) = 0;
    virtual bool Parse() = 0;
    virtual BaseMP4Info* GetInfo() = 0;

protected:
    bool ParseInternal(_Out_ BaseMP4Info* info);

    MP4Feed::MP4FeedType _feedType;            //File     /Memory              /Stream
    void*                _feedSource;          //Filename /Memory Handle       /IStream-pointer
    uint32               _feedLength;          //0        /Memory Array Length /0
    MP4Atom::EAtomType   _stopAtom;            //Parsing should stop when it hits this atom.
};

class MP4Parser : public BaseMP4Parser
{
public:
    MP4Parser();
    void Init(_In_ MP4Feed::MP4FeedType type, _In_ void* source, _In_ uint32 length);
    bool Parse();
    void Prepare(_In_ MP4Atom* mp4Atom);
    BaseMP4Info* GetInfo() { return &_info; }

private:
    MP4Info _info;     //Information is collected here after parsing.
};

class FMP4Parser : public BaseMP4Parser
{
public:
    // the parser can function just fine without
    // fragmentMediaType being specified. when
    // specified, the media-type will be copied
    // into the frameInfo.
    FMP4Parser(uint32 fragmentMediaType = 0);
    void Init(_In_ MP4Feed::MP4FeedType type, _In_ void* source, _In_ uint32 length);
    bool Parse();
    void Prepare(_In_ MP4Atom* mp4Atom);
    BaseMP4Info* GetInfo() { return &_info; }
    uint32 Prefetch(_In_ MP4Atom::EAtomType atomType, 
                    _In_ MP4Streamer* streamer,  
                    _Out_bytecap_post_bytecount_(length,fetched) uint8* buffer, 
                    _In_ uint32 length, 
                    _Out_ uint32* fetched);
    
private:
    bool ParseDrmData(_In_bytecount_(moofBoxSize) const uint8* moofBoxStartPos, _In_ uint32 moofBoxSize);

    FMP4Info      _info;              //Information is collected here after parsing.
};
