///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CAlternateLanguage.h"
#include "CStreamInfo.h"
#include "CRendererState.h"
#include "StringUtils.h"
#include "Trace.h"
#include <string>
#include <list>
using namespace std;

//#define LANGUAGE_SPEW
#if defined(LANGUAGE_SPEW)
#define LANGUAGE_MSG(x) TRACE(x)
#else
#define LANGUAGE_MSG(x)
#endif

// ===============================================================================================================
// Managing language settings as expected by the application and current language in use
// ===============================================================================================================

CAlternateLanguage::CAlternateLanguage(uint32 pipeIdN)
    : mPipeIdN(pipeIdN)
{
    PreferredLanguages = NULL;
    ClearPreferred();
    ClearExpected();
    ClearUsed();
}

CAlternateLanguage::~CAlternateLanguage()
{
    ClearPreferred();
}

void CAlternateLanguage::ClearPreferred(void)
{
    if (PreferredLanguages != NULL)
    {
        delete PreferredLanguages;
    }
    PreferredLanguages = NULL;
    PreferredLanguagesNum = 0;
    PreferredType = 0;
    PreferredValid = false;
}

void CAlternateLanguage::ClearExpected(void)
{
    ExpectedLanguage = 0;
    ExpectedType = 0;
    ExpectedPid = 0;
    ExpectedValid = false;
}

void CAlternateLanguage::ClearUsed(void)
{
    UsedLanguage = 0;
    UsedType = 0;
    UsedPid = 0;
    UsedValid = false;
    UsedMode = 0;
}

void CAlternateLanguage::SetPreferred(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type)
{
    LANGUAGE_MSG(("[%s][%08x]: SetPreferred(%06x, %d, %d)", GetStatusType().c_str(), mPipeIdN, (iso639LanguageCodes && (iso639LanguageCodesNum > 0)) ? iso639LanguageCodes[0] : 0, iso639LanguageCodesNum, type));

    //Delete any existing preferred list of language descriptors
    ClearPreferred();
    //For now clear the expected language because managed AV
    //will supply one immediately if one is available
    ClearExpected();
    ClearUsed();

    //Set the new preferred list of language descriptors
    if (iso639LanguageCodes != NULL && iso639LanguageCodesNum > 0)
    {
        PreferredLanguages = new uint32[iso639LanguageCodesNum];
        CHECK_ALLOC(PreferredLanguages);
        memcpy_s(PreferredLanguages, iso639LanguageCodesNum * sizeof(uint32), iso639LanguageCodes, iso639LanguageCodesNum * sizeof(uint32));
        PreferredLanguagesNum = iso639LanguageCodesNum;
        PreferredType = type;
        PreferredValid = true;
    }
}

void CAlternateLanguage::SetExpected(int iso639LanguageCode, int type, int pid)
{
    LANGUAGE_MSG(("[%s][%08x]: SetExpected(%06x, %d, %d)", GetStatusType().c_str(), mPipeIdN, iso639LanguageCode, type, pid));

    ExpectedLanguage = UsedLanguage = iso639LanguageCode;
    ExpectedType = UsedType = type;
    ExpectedPid = UsedPid = pid;
    ExpectedValid = UsedValid = true;
    UsedMode = 0;
}

void CAlternateLanguage::SetUsed(int iso639LanguageCode, int type, int pid, int mode)
{
    LANGUAGE_MSG(("[%s][%08x]: SetUsed(%06x, %d, %d, %d)", GetStatusType().c_str(), mPipeIdN, iso639LanguageCode, type, pid, mode));

    UsedLanguage = iso639LanguageCode;
    UsedType = type;
    UsedPid = pid;
    UsedValid = true;
    UsedMode = mode;
}

void CAlternateLanguage::SetUsedPid(int pid)
{
    LANGUAGE_MSG(("[%s][%08x]: SetUsedPid(%d)", GetStatusType().c_str(), mPipeIdN, pid));

    UsedPid = pid;
    UsedValid = true;
}

void CAlternateLanguage::SetUsed(int mode)
{
    UsedLanguage = ExpectedLanguage;
    UsedType = ExpectedType;
    UsedPid = ExpectedPid;
    UsedValid = true;
    UsedMode = mode;
}

void CAlternateLanguage::GetUsed(int& language, int& type, int& pid, int& mode)
{
    //Get current used language descriptor
    language = UsedLanguage;
    type = UsedType;
    pid = UsedPid;
    mode = UsedMode;
}

string CAlternateLanguage::SendToMediaTransport(bool isRequested)
{
    //Prepare message to be sent to media transport
    string status;
    status = "status=" + GetStatusType();
    status += "&language=" + ISO639ToString(UsedLanguage);
    status += "&type=" + toString(UsedType);
    status += "&pid=" + toString(UsedPid);
    status += "&valid=" + string(UsedValid ? "true" : "false");
    status += "&requested=" + string(isRequested ? "true" : "false");
    return status;
}

// ===============================================================================================================
// Handling audio language selection
// ===============================================================================================================

void CAudioLanguage::Find(CRendererState& rendererState)
{
    bool foundAudio = false;

    UsedMode = 0;
    if (ExpectedValid)
    {
        LANGUAGE_MSG(("[%s][%08x]: Looking for pid %d, language = '%s', type = %d", GetStatusType().c_str(), mPipeIdN, ExpectedPid, ISO639ToString(ExpectedLanguage).c_str(), ExpectedType));

        foundAudio = FindPid(rendererState);
        if (!foundAudio)
        {
            foundAudio = FindDescriptor(rendererState);
        }
    }
    if (!foundAudio)
    {
        if (PreferredValid)
        {
            LANGUAGE_MSG(("[%s][%08x]: Looking for type = %d and following descriptors", GetStatusType().c_str(), mPipeIdN, PreferredType));
            for (int j = 0; j < PreferredLanguagesNum; j++)
            {
                LANGUAGE_MSG(("[%s][%08x]:    descriptor = '%s'", GetStatusType().c_str(), mPipeIdN, ISO639ToString(PreferredLanguages[j]).c_str()));
            }
            foundAudio = FindPreferred(rendererState);
        }
    }
    if (!foundAudio)
    {
        if (!IsAudioDescription)
        {
            LANGUAGE_MSG(("[%s][%08x]: Looking for default selection", GetStatusType().c_str(), mPipeIdN));
            foundAudio = FindDefault(rendererState);
        }
        else
        {
            //Don't try to find a default AD stream; if we didn't find a specified language,
            //or a language wasn't specified, then disable AD decoding
            ClearUsed();
        }
    }

    if (foundAudio)
    {
        LANGUAGE_MSG(("[%s][%08x][%d]: pid %d with language descriptor '%s' and type %d found", GetStatusType().c_str(), mPipeIdN, UsedMode, UsedPid, ISO639ToString(UsedLanguage).c_str(), UsedType));
    }
    else
    {
        LANGUAGE_MSG(("[%s][%08x][%d]: no matching stream found", GetStatusType().c_str(), mPipeIdN, UsedMode));
    }
}

bool CAudioLanguage::FindPid(CRendererState& rendererState)
{
    CStreamInfoList& streamList = rendererState.StreamInfo;
    int count = streamList.Count();

    //Search the elementary stream info table for the specified audio language PID
    for (int i = 0; i < count; ++i)
    {
        CStreamInfo& info = streamList.GetStream(i);
        if (info.IsAudio() && info.StreamId == ExpectedPid)
        {
            SetUsed(1);
            return true;
        }
    }
    return false;
}

bool CAudioLanguage::FindDescriptor(CRendererState& rendererState)
{
    CStreamInfoList& streamList = rendererState.StreamInfo;
    int count = streamList.Count();

    //Search the elementary stream info table for the above
    for (int i = 0; i < count; ++i)
    {
        CStreamInfo& info = streamList.GetStream(i);
        if (info.IsAudio() && !info.Languages.empty())
        {
            //Look for an exact match of preferred language descriptor and audio type
            for (list<CLanguageDescriptor>::iterator it = info.Languages.begin(); it != info.Languages.end(); ++it)
            {
                if ((*it).Language == ExpectedLanguage && (*it).AudioType == ExpectedType)
                {
                    SetUsed((*it).Language, (*it).AudioType, info.StreamId, 2);
                    return true;
                }
            }

            //Look for an exact match of preferred language descriptor and audio type of 0
            for (list<CLanguageDescriptor>::iterator it = info.Languages.begin(); it != info.Languages.end(); ++it)
            {
                if ((*it).Language == ExpectedLanguage && (*it).AudioType == 0)
                {
                    SetUsed((*it).Language, (*it).AudioType, info.StreamId, 3);
                    return true;
                }
            }

            //Look for an exact match of preferred language descriptor only
            for (list<CLanguageDescriptor>::iterator it = info.Languages.begin(); it != info.Languages.end(); ++it)
            {
                if ((*it).Language == ExpectedLanguage)
                {
                    SetUsed((*it).Language, (*it).AudioType, info.StreamId, 4);
                    return true;
                }
            }
        }
    }
    return false;
}

bool CAudioLanguage::FindPreferred(CRendererState& rendererState)
{
    CStreamInfoList& streamList = rendererState.StreamInfo;
    int count = streamList.Count();

    //Search the elementary stream info table for the above
    for (int i = 0; i < count; ++i)
    {
        CStreamInfo& info = streamList.GetStream(i);
        if (info.IsAudio() && !info.Languages.empty())
        {
            //Look for an exact match of preferred language descriptor and audio type
            for (list<CLanguageDescriptor>::iterator it = info.Languages.begin(); it != info.Languages.end(); ++it)
            {
                for (int j = 0; j < PreferredLanguagesNum; j++)
                {
                    if ((*it).Language == PreferredLanguages[j] && (*it).AudioType == PreferredType)
                    {
                        SetUsed((*it).Language, (*it).AudioType, info.StreamId, 5);
                        return true;
                    }
                }
            }

            //Don't match on other audio types if we prefer an audio description stream
            if (!IsAudioDescription)
            {
                //Look for an exact match of preferred language descriptor and audio type of 0
                for (list<CLanguageDescriptor>::iterator it = info.Languages.begin(); it != info.Languages.end(); ++it)
                {
                    for (int j = 0; j < PreferredLanguagesNum; j++)
                    {
                        if ((*it).Language == PreferredLanguages[j] && (*it).AudioType == 0)
                        {
                            SetUsed((*it).Language, (*it).AudioType, info.StreamId, 6);
                            return true;
                        }
                    }
                }

                //Don't pick an audio description stream if we're not specifically looking for it
                if (!info.IsAudioDescription())
                {
                    //Look for an exact match of preferred language descriptor only
                    for (list<CLanguageDescriptor>::iterator it = info.Languages.begin(); it != info.Languages.end(); ++it)
                    {
                        for (int j = 0; j < PreferredLanguagesNum; j++)
                        {
                            if ((*it).Language == PreferredLanguages[j])
                            {
                                SetUsed((*it).Language, (*it).AudioType, info.StreamId, 7);
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool CAudioLanguage::FindDefault(CRendererState& rendererState)
{
    CStreamInfoList& streamList = rendererState.StreamInfo;
    int count = streamList.Count();

    //Number of audio streams
    int audio_pid_count = 0;
    //Language descriptor selected
    int lowest_pid_with_iso639language_descriptor_descriptor = 0;
    //Language type selected
    int lowest_pid_with_iso639language_descriptor_type = 0;
    //The lowest audio pid that has 639 language descriptor
    int lowest_pid_with_iso639language_descriptor = 0x7FFF;
    //The lowest audio pid that does not have 639 language descriptor
    int lowest_pid_without_iso639language_descriptor = 0x7FFF;
    //The second lowest audio pid that does not have 639 language descriptor
    int second_lowest_pid_without_iso639language_descriptor = 0x7FFF;

    //Search the elementary stream info table for the above
    for (int i = 0; i < count; ++i)
    {
        CStreamInfo& info = streamList.GetStream(i);
        if (info.IsAudio())
        {
            //One more audio pid found
            audio_pid_count++;

            //Look for lowest pid where language descriptors are available
            if (!info.Languages.empty())
            {
                if (info.StreamId < lowest_pid_with_iso639language_descriptor)
                {
                    list<CLanguageDescriptor>::iterator it = info.Languages.begin();
                    if (it != info.Languages.end())
                    {
                        lowest_pid_with_iso639language_descriptor_descriptor = (*it).Language;
                        lowest_pid_with_iso639language_descriptor_type = (*it).AudioType;
                    }
                    else
                    {
                        lowest_pid_with_iso639language_descriptor_descriptor =  0;
                        lowest_pid_with_iso639language_descriptor_type = 0;
                    }
                    lowest_pid_with_iso639language_descriptor = info.StreamId;
                }
            }
            else
            //Look for lowest and second lowest pids where language descriptors are not available
            {
                if (info.StreamId < lowest_pid_without_iso639language_descriptor)
                {
                    second_lowest_pid_without_iso639language_descriptor = lowest_pid_without_iso639language_descriptor;
                    lowest_pid_without_iso639language_descriptor = info.StreamId;
                }
                else
                if (info.StreamId < second_lowest_pid_without_iso639language_descriptor)
                {
                    second_lowest_pid_without_iso639language_descriptor = info.StreamId;
                }
            }
        }
    }

    //First go for the default selection of lowest pid with language descriptors
    if (lowest_pid_with_iso639language_descriptor != 0x7FFF)
    {
        SetUsed(lowest_pid_with_iso639language_descriptor_descriptor, lowest_pid_with_iso639language_descriptor_type, lowest_pid_with_iso639language_descriptor, 8);
        return true;
    }

    //Language pid selected
    int language_descriptor = 0;
    int language_pid = 0;
    int selection_mode = 0;
    //Make the secondary language selection if user has selected SAP and
    //multiple languages without language descriptors are available
    if (rendererState.UseSap && audio_pid_count > 1)
    {
        language_descriptor = ((int)'2') << 16;
        language_pid = second_lowest_pid_without_iso639language_descriptor;

        selection_mode = 9;
    }
    //Otherwise select primary stream - that is the lowest PID without language descriptor
    else
    if (audio_pid_count > 0)
    {
        language_descriptor = ((int)'1') << 16;
        language_pid = lowest_pid_without_iso639language_descriptor;

        selection_mode = 10;
    }
    //Set the selected language type
    if (language_pid != 0)
    {
        SetUsed(language_descriptor, 0, language_pid, selection_mode);
        return true;
    }

    ClearUsed();
    return false;
}

// ===============================================================================================================
// Handling subtitle language selection
// ===============================================================================================================

void CSubtitleLanguage::Find(CRendererState& rendererState)
{
    bool foundSubtitles = false;

    UsedMode = 0;
    if (ExpectedValid)
    {
        LANGUAGE_MSG(("[%s][%08x]: Looking for subtitle pid %d, language = '%s', type = %d", GetStatusType().c_str(), mPipeIdN, ExpectedPid, ISO639ToString(ExpectedLanguage).c_str(), ExpectedType));

        foundSubtitles = FindPid(rendererState);
        if (!foundSubtitles)
        {
            ClearUsed();
        }
    }

    if (foundSubtitles)
    {
        LANGUAGE_MSG(("[%s][%08x][%d]: pid %d with language descriptor '%s' and type %d found", GetStatusType().c_str(), mPipeIdN, UsedMode, UsedPid, ISO639ToString(UsedLanguage).c_str(), UsedType));
    }
    else
    {
        LANGUAGE_MSG(("[%s][%08x][%d]: no matching stream found", GetStatusType().c_str(), mPipeIdN, UsedMode));
    }
}

bool CSubtitleLanguage::FindPid(CRendererState& rendererState)
{
    CStreamInfoList& streamList = rendererState.StreamInfo;
    int count = streamList.Count();

    //Search the elementary stream info table for the specified subtitle language PID
    for (int i = 0; i < count; ++i)
    {
        CStreamInfo& info = streamList.GetStream(i);
        if (info.IsSubtitle() && info.StreamId == ExpectedPid)
        {
            SetUsed(1);
            return true;
        }
    }
    return false;
}

// ===============================================================================================================
// ===============================================================================================================
