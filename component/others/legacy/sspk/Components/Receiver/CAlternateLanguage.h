///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

// ===============================================================================================================
// External classes referenced
// ===============================================================================================================

class CRendererState;

// ===============================================================================================================
// Managing language settings as expected by the application and current language in use
// ===============================================================================================================

class CAlternateLanguage
{
public:
    CAlternateLanguage(uint32 pipeIdN);
    virtual ~CAlternateLanguage();

public:
    //Clear preferred language
    void                   ClearPreferred(void);
    //Clear expected language
    void                   ClearExpected(void);
    //Clear used language
    void                   ClearUsed(void);
    //Set default list of language descriptors and type to be used for making default language selection
    void                   SetPreferred(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type);
    //Set application preferred language
    void                   SetExpected(int iso639LanguageCode, int type, int pid);
    //Set the currently being used language
    void                   SetUsed(int iso639LanguageCode, int type, int pid, int mode);
    //Set UsedPid directly
    void                   SetUsedPid(int pid);
    //Set the used language as expected
    void                   SetUsed(int mode);
    //Do we have a valid language preference
    bool                   IsValid(void) const { return PreferredValid || ExpectedValid || UsedValid; }
    //Get current used language descriptor
    void                   GetUsed(int& language, int& type, int& pid, int& mode);
    //Returns currently used pid
    int                    GetUsedPid(void) { return UsedPid; }
    //Returns currently used language
    int                    GetUsedLanguage(void) { return UsedLanguage; }
    //Returns currently used type
    int                    GetUsedType(void) { return UsedType; }
    //Has used language actually been set or is it just expected?
    bool                   IsUsedSet(void) const { return UsedMode != 0; }

    //Update media transport with currently rendered subtitle stream
    std::string            SendToMediaTransport(bool isRequested);

protected:
    //Returns the stream type to be sent to media transport
    virtual std::string    GetStatusType(void) const { return ""; }

    //Pipe id
    uint32                 mPipeIdN;

    //Preferred array of default language descriptors as well as audio type to be
    //used when making default audio language selection
    uint32*                PreferredLanguages;
    int                    PreferredLanguagesNum;
    int                    PreferredType;
    bool                   PreferredValid;

    //Preferred language descriptors as supplied explicitely by the application
    uint32                 ExpectedLanguage;
    int                    ExpectedType;
    int                    ExpectedPid;
    bool                   ExpectedValid;

    //Language descriptors for language that is currently in use
    uint32                 UsedLanguage;
    int                    UsedType;
    int                    UsedPid;
    bool                   UsedValid;
    //The selection mode
    int                    UsedMode;
};

// ===============================================================================================================
// Handling audio language selection
// ===============================================================================================================

class CAudioLanguage : public CAlternateLanguage
{
public:
    CAudioLanguage(uint32 pipeIdN, bool isAudioDescription) : CAlternateLanguage(pipeIdN) { IsAudioDescription = isAudioDescription; }

public:
    //Whether given pid is same as current PID to be renderered
    bool                   IsUsedPid(int pid) const { return pid == UsedPid; }
    //Find an audio language to be played back
    void                   Find(CRendererState& rendererState);
private:
    //Find an audio language in the PMT which matches the exact PID provided by the application
    bool                   FindPid(CRendererState& rendererState);
    //Find an audio language matching the language descriptor and audio types provided by the application
    bool                   FindDescriptor(CRendererState& rendererState);
    //Find audio language from the PMT based on expected audio settings provided explicitely by the application
    bool                   FindPreferred(CRendererState& rendererState);
    //Make a default audio language selection
    bool                   FindDefault(CRendererState& rendererState);

protected:
    //Returns the stream type to be sent to media transport
    __override std::string GetStatusType(void) const { return IsAudioDescription ? "audio_desc" : "audio"; }

    //Whether or not we prefer an 'audio description' type stream
    bool                   IsAudioDescription;
};

// ===============================================================================================================
// Handling subtitle language selection
// ===============================================================================================================

class CSubtitleLanguage : public CAlternateLanguage
{
public:
    CSubtitleLanguage(uint32 pipeIdN) : CAlternateLanguage(pipeIdN) {}

public:
    //Find a subtitle language to be played back
    void                   Find(CRendererState& rendererState);
    //Find a subtitle language in the PMT which matches the exact PID provided by the application
    bool                   FindPid(CRendererState& rendererState);

protected:
    //Returns the stream type to be sent to media transport
    __override std::string GetStatusType(void) const { return "subtitle"; }
};

// ===============================================================================================================
// ===============================================================================================================
