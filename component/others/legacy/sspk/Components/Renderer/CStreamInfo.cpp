///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CStreamInfo.h"
#include "StringUtils.h"
#include "Trace.h"
#include <algorithm>
#include <string>
#include <list>
using namespace std;

//#define STREAMINFO_SPEW
#if defined(STREAMINFO_SPEW)
#define STREAMINFO_MSG(x) TRACE(x)
#else
#define STREAMINFO_MSG(x)
#endif

// ===============================================================================================================
// ===============================================================================================================
#define PRINTFOURCC(fourCC)  if(fourCC <= 0xffff) {STREAMINFO_MSG(("\n%s() fourCC=0x%x\n", __FUNCTION__, fourCC)); } \
                             else {STREAMINFO_MSG(("\n%s() fourCC=0x%x,%c%c%c%c\n", __FUNCTION__, fourCC, \
                             ((char*)&fourCC)[0], ((char*)&fourCC)[1], ((char*)&fourCC)[2], ((char*)&fourCC)[3]));}

StreamType FourCCToStreamType(uint32 fourCC, StreamType defaultType/* = StreamType_None*/)
{
    PRINTFOURCC(fourCC);
    
    switch (fourCC)
    {
    //Video types
    //
    //VC1
    case MAKEFOURCC('W','M','V','A'):
    case MAKEFOURCC('W','V','C','1'):
        return StreamType_Video_VC1;

    //AVC
    case MAKEFOURCC('H','2','6','4'):
    case MAKEFOURCC('A','V','C','1'):
    case MAKEFOURCC('X','2','6','4'):
    case MAKEFOURCC('D','A','V','C'):
    case MAKEFOURCC('A','V','C','A'):
    case MAKEFOURCC('A','V','C','B'):
        return StreamType_Video_AVC;

    //WMV9
    case MAKEFOURCC('W','M','V','3'):
        return StreamType_Video_WMV9;

    //Audio types
    //
    //MP3
    case 0x00000055:
        return StreamType_Audio_11172;

    //AAC
    case 0x00001601:
    case 0x000000FF:
    case MAKEFOURCC('A','A','C', 0 ):
    case MAKEFOURCC('A','A','C',' '):
    case MAKEFOURCC('A','A','C','L'):
    case MAKEFOURCC('A','A','C','H'):
    case MAKEFOURCC('A','A','C','P'):
        return StreamType_Audio_AAC;

    //AC3
    case 0x00000092:
    case MAKEFOURCC('A','C','3',' '):
        return StreamType_Audio_AC3;

    //EAC3
    case MAKEFOURCC('E','A','C','3'):
    case MAKEFOURCC('E','C','-','3'):
        return StreamType_Audio_EnhancedAC3;

    //WMA
    case 0x00000160:
    case 0x00000161:
    case 0x00005052:
    case MAKEFOURCC('W','M','A', 0 ):
    case MAKEFOURCC('W','M','A',' '):
    case MAKEFOURCC('W','M','A','2'):
        return StreamType_Audio_WMA;

    //WMA Pro
    case 0x00000162:
    case MAKEFOURCC('W','M','A','P'):
        return StreamType_Audio_WMAPRO;
    }

    STREAMINFO_MSG(("%s() defaultType=%d\n", __FUNCTION__, defaultType));
    return defaultType;
}

// ===============================================================================================================
// ===============================================================================================================

string ISO639ToString(uint32 n)
{
    char s[4];
    s[0] = (char)(n >> 16);
    s[1] = (char)(n >> 8);
    s[2] = (char)(n);
    s[3] = 0;
    return string(s);
}

uint32 StringToISO639(const std::string& s)
{
    int len = s.length();
    if (len > 4)
    {
        len = 4;
    }
    
    uint32 n = 0;
    for (int i = 0; i < len; i++)
    {
        n = (n << 8) | (uint32) s[i];
    }
    return n;
}

// ===============================================================================================================
// CLanguageDescriptor
// ===============================================================================================================

bool CLanguageDescriptor::IsValidLanguageDescriptor(void)
{
    string language_str = ISO639ToString(Language);

    //Treat empty language descriptor as 'no descriptor'
    if (language_str.size() == 0)
        return false;

    //Treat any descriptor containing non-alphanumeric charactor as 'no descriptor'
    for (uint32 i=0; i<language_str.size(); ++i)
    {
        if (!((language_str[i] >= 'a' && language_str[i] <= 'z') ||
              (language_str[i] >= 'A' && language_str[i] <= 'Z') ||
              (language_str[i] >= '0' && language_str[i] <= '9')))
            return false;
    }
    return true;
}

// ===============================================================================================================
// Stream language class
// ===============================================================================================================

CStreamInfo::CStreamInfo()
{
    Type = StreamInfoType_Unknown;
    StreamId = 0;
    Format = StreamType_None;
    AudioType = 0;
    IsMp3 = false;
    HasCaptionServiceDescriptor = false;
    Active708Services = 0;
    HasInitialTeletextPage = false;
    InitialTeletextPage = 0;
    Languages.clear();
    Subtitles.clear();
}

void CStreamInfo::SetVideoDescriptor(int streamId, uint32 fourCC)
{
    Type = StreamInfoType_Video;
    StreamId = streamId;
    Format = FourCCToStreamType(fourCC, StreamType_Video);
}

void CStreamInfo::SetAudioDescriptor(int streamId, uint32 fourCC, string& language)
{
    Type = StreamInfoType_Audio;
    StreamId = streamId;
    Format = FourCCToStreamType(fourCC, StreamType_Audio_WMA);

    if (!language.empty())
    {
        CLanguageDescriptor ld;
        ld.Language = StringToISO639(language);
        ld.AudioType = AudioType;
        if (ld.IsValidLanguageDescriptor())
        {
            Languages.push_back(ld);
        }
    }
}

void CStreamInfo::SetSubtitleDescriptor(int streamId, string& language)
{
    const int SUBTITLE_NO_ASPECT_RATIO = 0x10; // set to NORMAL_NO_ASPECT_RATIO. TODO: do we need to set an aspect ratio here?

    Type = StreamInfoType_Subtitle;
    StreamId = streamId;
    Format = StreamType_Pes_Private;
    {
        CSubtitlingDescriptor sd;
        sd.Language = StringToISO639(language);
        sd.SubtitlingType = SUBTITLE_NO_ASPECT_RATIO;
        sd.CompositionPageId = 0;
        sd.AncillaryPageId = 0;
        Subtitles.push_back(sd);
    }
}

string CStreamInfo::GetType(void)
{
    switch (Type)
    {
        case StreamInfoType_Video:            return "video";
        case StreamInfoType_Audio:            return "audio";
        case StreamInfoType_AudioDescription: return "audio_desc";
        case StreamInfoType_ECM:              return "ecm";
        case StreamInfoType_Subtitle:         return "subtitle";
        case StreamInfoType_Teletext:         return "teletext";
        case StreamInfoType_VPS:              return "vps";
        case StreamInfoType_WSS:              return "wss";
        case StreamInfoType_MHEG5:            return "mheg5";
        default: break;
    }
    return "unknown";
}

// ===============================================================================================================
// ===============================================================================================================

CStreamInfoList::CStreamInfoList(const CStreamInfoList& sil)
{
    Init(sil);
}

CStreamInfoList& CStreamInfoList::operator=(const CStreamInfoList& sil)
{
    if (this != &sil)
    {
        Init(sil);
    }
    return *this;
}

void CStreamInfoList::Init(const CStreamInfoList& sil)
{
    StreamInfoVector.clear();
    for (uint32 i=0; i < sil.StreamInfoVector.size(); ++i)
    {
        StreamInfoVector.push_back(sil.StreamInfoVector[i]);
    }
}

CStreamInfo* CStreamInfoList::Find(int streamId)
{
    for (uint32 i=0; i < StreamInfoVector.size(); ++i)
    {
        if (StreamInfoVector[i].StreamId == streamId)
            return &StreamInfoVector[i];
    }
    return NULL;
}

static bool CompareStreamId(const CStreamInfo& info1, const CStreamInfo& info2)
{
    return info1.StreamId < info2.StreamId;
}

string CStreamInfoList::GenerateXmlOutput(void)
{
    //Sort CStreamInfo by StreamId from low to high
    sort(StreamInfoVector.begin(), StreamInfoVector.end(), CompareStreamId);

    //TODO: Reformat XML tags to not use SI specific terms (i.e. pmt, esitem, pid)
    //Sample Output:
    //
    //<pmt>
    //    <esitem type='audio' pid='601' lang='eng' format='3' audio_type='0'/>
    //    <esitem type='audio' pid='602' lang='eng' format='3' audio_type='0'/>
    //    <esitem type='audio' pid='603' lang='eng,fra' format='4' audio_type='0'/>
    //    <esitem type='audio' pid='604' lang='fra' format='129' audio_type='0'/>
    //    <esitem type='audio_desc' pid='610' lang='eng' format='3' audio_type='3'/>
    //    <esitem type='subtitle' pid='605' lang='eng' format='6' subtitle_type='16'/>
    //    <esitem type='subtitle' pid='605' lang='deu' format='6' subtitle_type='16'/>
    //    <esitem type='subtitle' pid='605' lang='fra' format='6' subtitle_type='16'/>
    //    <esitem type='subtitle' pid='607' lang='eng' format='6'/>
    //    <esitem type='teletext' pid='606' lang='' format='6'/>
    //</pmt>
    string ret = "<pmt>";

    //Index for audio stream that does not have ISO639 code
    int audioIdx = 1;
    for (uint32 i=0; i<StreamInfoVector.size(); ++i)
    {
        CStreamInfo& si = StreamInfoVector[i];
        if (si.IsSubtitle())
        {
            //List each subtitle descriptor as separate esitem
            for (list<CSubtitlingDescriptor>::iterator iter = si.Subtitles.begin(); iter != si.Subtitles.end(); ++iter)
            {
                ret += "<esitem type='" + si.GetType()
                    + "' pid='" + toString(si.StreamId)
                    + "' lang='" + ISO639ToString(iter->Language)
                    + "' format='" + toString(si.Format)
                    + "' subtitle_type='" + toString(iter->SubtitlingType)
                    + "'/>";
            }
        }
        else
        if (si.IsAudio())
        {
            //type, pid
            ret += "<esitem type='" + si.GetType()
                + "' pid='" + toString(si.StreamId);

            //lang
            string langstr;
            for (list<CLanguageDescriptor>::const_iterator iter = si.Languages.begin(); iter != si.Languages.end(); ++iter)
            {
                if (iter->Language)
                {
                    if (!langstr.empty())
                        langstr += ',';

                    langstr += ISO639ToString(iter->Language);
                }
            }
            if (langstr.empty())
            {
                ret += "' lang='" + toString(audioIdx++);
            }
            else
            {
                ret += "' lang='" + langstr;
            }

            //format
            ret += "' format='" + toString(si.Format);

            //audio_type
            ret += "' audio_type='" + toString(si.AudioType);

            //Finish the current line
            ret += "'/>";
        }
        else
        if (si.IsTeletext() || si.IsECM() || si.IsMHEG5())
        {
            ret += "<esitem type='" + si.GetType()
                + "' pid='" + toString(si.StreamId)
                + "' format='" + toString(si.Format)
                + "'/>";
        }
    }

    //Now close the xml
    ret += "</pmt>";
    return ret;
}

// ===============================================================================================================
// ===============================================================================================================
