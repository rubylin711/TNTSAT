///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

// ===============================================================================================================
// ===============================================================================================================

class ITunerSessionCallback
{
public:
    virtual ~ITunerSessionCallback() {}
    virtual void StatusCallback(_In_ const std::string& status, _In_opt_ bool shouldCallCallback = true) = 0;
};

// ===============================================================================================================
// ITunerSession interfaces used by application to control a rendering session
// ===============================================================================================================

class IReceiverControl;
class ISpliceEngine;
class IDiagsEvent;

class ITunerSession
{
public:
    //Destructor
    virtual ~ITunerSession() {}

    //API mainly used for handling multiple active pipes and their mapping to derived ITunerSessions
    virtual uint32             GetPipeIdN(void) = 0;

    //Managed ITunerSession facing APIs
    virtual void                    PrepareToTune(void) = 0;
    virtual void                    TuneDone(void) = 0;
    virtual void                    Tune(const std::string& url, int action, const int* timings) = 0;
    virtual void                    TuneRefresh(const std::string& url) = 0;

    enum DetuneReason
    {
        eDetuneClose,
        eDetuneNoClose
    };
    virtual void                    Detune(bool forceDetune, bool teardownPicture, bool channelChange, DetuneReason reason) = 0;
    virtual void                    SetBufferLength(uint64 startTime, uint64 endTime) = 0;
    virtual void                    SetClipPlay(uint64 clipStartTime, bool clipStartTimeSet, uint64 clipEndTime, bool clipEndTimeSet, uint32 clipLimits) = 0;
    virtual bool                    PauseLive(const std::string& url) = 0;
    virtual bool                    Pause(void) = 0;
    virtual bool                    Play(void) = 0;
    virtual void                    SetAccessControl(bool blocked, uint64 startTime, uint64 endTime) = 0;
    virtual void                    SetSecondaryAudioPreference(bool usesap) = 0;
    virtual void                    SetAudioLanguages(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type) = 0;
    virtual void                    SetAudioLanguage(int iso639LanguageCode, int type, int pid) = 0;
    virtual void                    SetAudioDescriptionLanguages(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type) = 0;
    virtual void                    SetAudioDescriptionLanguage(int iso639LanguageCode, int type, int pid) = 0;
    virtual void                    SetAudioDescriptionVolume(int volume) = 0;
    virtual void                    SetSubtitleLanguages(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type) = 0;
    virtual void                    SetSubtitleLanguage(int iso639LanguageCode, int type, int pid) = 0;
    virtual void                    SetCCChannel(int cc608, int cc708) = 0;
    virtual uint64                  GetCurrentMediaTime(bool smooth) = 0;

    //Generic interface to send custom commands
    virtual bool                    Command(const std::string& command, const std::vector<std::string>& args) = 0;
    virtual bool                    IsStreamActive(void) = 0;
    virtual IDiagsEvent*            RetrieveDiagnostics(void) = 0;
    virtual IDiagsEvent*            RetrieveTunerDiagnostics(void) = 0;
    virtual ITunerSessionCallback*  GetStatusCallback(void) = 0;
};

// ===============================================================================================================
// ===============================================================================================================

class ITunerSessionFactory
{
public:
    //Destructor
    virtual ~ITunerSessionFactory() {}
    //Creates a tuner session given a pipe id
    virtual ITunerSession*     CreateTunerSession(ITunerSessionCallback* callback, const char* id, uint32 featuresEnabled, uint32& handle) = 0;
    //Destroys a tuner session from cache given its handle
    virtual void               DestroyTunerSession(ITunerSession* tunerSession) = 0;
    //Destroys a tuner session from cache given its handle
    virtual void               DestroyTunerSession(uint32 handle) = 0;
    //Returns a tuner session instance given its handle
    virtual ITunerSession*     GetTunerSession(uint32 handle) = 0;
    //Returns a tuner session isntance given it's pipeid
    virtual ITunerSession*     GetTunerSession(const std::wstring& pipeId) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
