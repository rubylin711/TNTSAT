///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <list>
#include <vector>

// ===============================================================================================================
// ===============================================================================================================

enum StreamType
{
    StreamType_None                  = 0x00,

    StreamType_Video                 = 0x01,
    StreamType_Video_Constrained     = 0x02,
    StreamType_Audio_11172           = 0x03,  //MPEG 1
    StreamType_Audio_13818_3         = 0x04,  //MPEG 2
    StreamType_Pes_Private           = 0x06,
    StreamType_MHEG                  = 0x07,
    StreamType_DSMCC                 = 0x08,

    StreamType_DSMCC_TypeA           = 0x0A,
    StreamType_DSMCC_TypeB           = 0x0B,
    StreamType_DSMCC_TypeC           = 0x0C,
    StreamType_DSMCC_TypeD           = 0x0D,

    StreamType_Auxiliary             = 0x0E,
    StreamType_Audio_AAC             = 0x0F,
    StreamType_Audio_HEAAC           = 0x11,

    StreamType_Video_AVC             = 0x1B,  //H.264

    StreamType_Audio_AC3             = 0x81,
    StreamType_Audio_PCM             = 0x83,

    StreamType_Audio_DTS             = 0x85,

    StreamType_Audio_EnhancedAC3     = 0x87,

    StreamType_Audio_VC9             = 0x88,
    StreamType_Video_VC1             = 0xEA,
    StreamType_Video_WMV9            = 0xEB,
    StreamType_Audio_WMA             = 0xE6,
    StreamType_Audio_WMAPRO          = 0xE7,

    StreamType_Audio_WMA_WITH_HEADER = 0xFF,  //Special WMA stream type to differentiate the fact that
                                              //each WMA packet is preceded by a WMA header in the PES

    StreamType_ECM                   = 0x100, //FIXME:  Not sure if this is kosher...
    StreamType_DFXP                  = 0x101
};

StreamType  FourCCToStreamType(uint32 fourCC, StreamType defaultType = StreamType_None);

// ===============================================================================================================
// ===============================================================================================================

std::string ISO639ToString(uint32 n);
uint32      StringToISO639(const std::string& s);

// ===============================================================================================================
// Audio Language Descriptor
// ===============================================================================================================

class CLanguageDescriptor
{
public:
    uint32  Language;
    int     AudioType;

    bool    operator==(const CLanguageDescriptor& ld) const
    {
        return ((Language == ld.Language) && (AudioType == ld.AudioType));
    }

    bool    operator!=(const CLanguageDescriptor& ld) const
    {
        return (!(*this == ld));
    }

    bool    IsValidLanguageDescriptor(void);
};

// ===============================================================================================================
// Subtitle Language Descriptor
// ===============================================================================================================

class CSubtitlingDescriptor
{
public:
    uint32  Language;
    int     SubtitlingType;
    int     CompositionPageId;
    int     AncillaryPageId;

    bool    operator==(const CSubtitlingDescriptor& sd) const
    {
        return ((Language == sd.Language) &&
                (SubtitlingType == sd.SubtitlingType) &&
                (CompositionPageId == sd.CompositionPageId) &&
                (AncillaryPageId == sd.AncillaryPageId));
    }

    bool    operator!=(const CSubtitlingDescriptor& sd) const
    {
        return (!(*this == sd));
    }
};

// ===============================================================================================================
// ===============================================================================================================

enum StreamInfoType
{
    StreamInfoType_Unknown = 0,
    StreamInfoType_Video,
    StreamInfoType_Audio,
    StreamInfoType_AudioDescription,
    StreamInfoType_ECM,
    StreamInfoType_Subtitle,
    StreamInfoType_Teletext,
    StreamInfoType_VPS,
    StreamInfoType_WSS,
    StreamInfoType_MHEG5,
};

// ===============================================================================================================
// Stream language class
// ===============================================================================================================

class CStreamInfo
{
public:
    CStreamInfo();

    void           SetVideoDescriptor(int streamId, uint32 fourCC);
    void           SetAudioDescriptor(int streamId, uint32 fourCC, std::string& language);
    void           SetSubtitleDescriptor(int streamId, std::string& language);

    std::string    GetType(void);

    bool           IsVideo(void) const { return Type == StreamInfoType_Video; }
    bool           IsAudio(void) const { return Type == StreamInfoType_Audio || Type == StreamInfoType_AudioDescription; }
    bool           IsAudioDescription(void) const { return Type == StreamInfoType_AudioDescription; }
    bool           IsECM(void) const { return Type == StreamInfoType_ECM; }
    bool           IsSubtitle(void) const { return Type == StreamInfoType_Subtitle; }
    bool           IsTeletext(void) const { return Type == StreamInfoType_Teletext; }
    bool           IsVPS(void) const { return Type == StreamInfoType_VPS; }
    bool           IsWSS(void) const { return Type == StreamInfoType_WSS; }
    bool           IsMHEG5(void) const { return Type == StreamInfoType_MHEG5; }

public:
    //Cache type as optimization
    StreamInfoType Type;
    //Stream id
    int            StreamId;
    //Media format, e.g. ac3, mpeg2, ... (enum StreamType)
    int            Format;
    //Audio type field in language descriptor
    int            AudioType;
    //Whether it is MP3 type audio
    bool           IsMp3;
    //Whether digital captioning services available on video stream
    bool           HasCaptionServiceDescriptor;
    //Types of digital closed captioning services available
    uint32         Active708Services;
    //Whether there is an initial teletext page signalled
    bool           HasInitialTeletextPage;
    //Initial teletext page
    int            InitialTeletextPage;
    //ISO639 Language
    std::list<CLanguageDescriptor>   Languages;
    //Subtitling Descriptors
    std::list<CSubtitlingDescriptor> Subtitles;
};

typedef std::vector<CStreamInfo> CStreamInfoVector;

// ===============================================================================================================
// ===============================================================================================================

class CStreamInfoList
{
public:
    CStreamInfoList() {}
    CStreamInfoList(const CStreamInfoList& sil);
    CStreamInfoList& operator=(const CStreamInfoList& sil);

private:
    void         Init(const CStreamInfoList& si);

public:
    int          Count(void) { return (int)StreamInfoVector.size(); }
    CStreamInfo& GetStream(int i) { return StreamInfoVector[i]; }

    void         Clear(void) { StreamInfoVector.clear(); }
    void         AddStreamInfo(CStreamInfo& info) { StreamInfoVector.push_back(info); }

    CStreamInfo* Find(int streamId);

    std::string  GenerateXmlOutput(void);

private:
    CStreamInfoVector StreamInfoVector;
};

// ===============================================================================================================
// ===============================================================================================================
