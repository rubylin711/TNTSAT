///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CAccessControl.h"
#include "CAlternateLanguage.h"
#include "CStreamInfo.h"
#include <string>
#include <map>

// ===============================================================================================================
// CRendererState class
//
// All IRenderer and downstream components are expected to use this class to keep persistent state and
// access properties that they need to playback content based on user settings etc.
// ===============================================================================================================

class IReceiverControl;
class IDecoderFactory;
class CReceiverDiagnostics;
class IDecoder;
class CTuneRequest;

class CRendererState
{
public:
    //Various pipeline features specific to individual pipe
    enum FeatureEnabled
    {
        eFeature_FullscreenMode     = 0x00000001,
        eFeature_RenderingPipe      = 0x00000002,
        eFeature_DiagsProvider      = 0x00000008,
        eFeature_ClosedCaptioning   = 0x00000010,
        eFeature_Teletext           = 0x00000020,
        eFeature_StreamNotification = 0x00000040,
        eFeature_Splicing           = 0x00000080,
        eFeature_ClockCorrection    = 0x00000200,
        eFeature_ClockValidation    = 0x00000400,
        eFeature_VideoOPL           = 0x00001000,
        eFeature_PlayreadyOPL       = 0x00002000,
    };

public:
    //Constructor
    CRendererState(IReceiverControl* receiverControl, const std::string& pipeId, uint32 featuresEnabled);
    //Destructor
    ~CRendererState();

public:
    //Called when receiver is reset
    void                   OnReset(void);
    //Called when receiver is resync for stream error recovery
    void                   OnSync(void);
    //Called when the receiver is started and being prepped to receive a new stream
    void                   OnStart(CTuneRequest& tuneRequest);
    //Called when receiver is being detuned
    void                   OnDetune(void);
    //Called on timeslice
    void                   OnTimeslice(void);

    //Whether a given feature is enabled
    bool                   IsFeatureEnabled(uint32 feature) { return (mFeatureEnabled & feature) == feature; }
    void                   EnableFeature(uint32 feature)    { mFeatureEnabled |= feature; }
    void                   DisableFeature(uint32 feature)   { mFeatureEnabled &= ~feature; }

private:
    //Receiver control to use
    IReceiverControl*      mReceiverControl;
    //Decoder factory to use
    IDecoderFactory*       mDecoderFactory;
    //Diagnostics to use
    CReceiverDiagnostics&  mDiagnostics;

public:
    //Pipe id
    std::string            mPipeId;
    uint32                 mPipeIdN;
    //Whether features enabled for this pipe
    uint32                 mFeatureEnabled;
    //Are we rendering fullscreen AV?
    bool                   IsFullScreen;

    //Immediate mode
    bool                   IsImmediateMode;
    //IFrameOnly mode
    bool                   IsIFrameOnlyMode;

    //Rate of playback
    float                 PlaybackRate;

    //Start and end time in the stream
    uint64                 StartTime;
    uint64                 EndTime;

    //Default program number
    int                    DefaultProgram;

    //Teardown picture when detuning
    bool                   TearDownPicture;

    //MHEG mode flag
    //
    //Set the new mode when doing a still frame rendering for MHEG
    //Note that the mode is expected to change to true before we start rendering
    //an still I-Frame for MHEG App and is expected to be restored back once it
    //is rendered.
    bool                   IsMHEGMode;

    // ===========================================================================================================
    // Various flags used during splcing of streams (ad insertion/replacement)
    // ===========================================================================================================
public:
    //Whether to flush decoders on release to HAL decoder factory
    bool                   FlushVideoDecoderOnRelease;
    bool                   FlushAudioDecoderOnRelease;
    //Receiver stopped for ad splicing
    bool                   IsStoppedForAd;
    //Ignore DRM keys while playing ads
    bool                   IsPlayingAd;

    // ===========================================================================================================
    // Access (parental) control settings
    // ===========================================================================================================
public:
    //Called to upadte access control parameters
    void                   OnSetAccessControl(bool blocked, uint64 startTime, uint64 endTime);
public:
    //Access control info to be used by video decoder
    CAccessControl         VideoAccessControl;
    //Access control info to be used by audio decoder
    CAccessControl         AudioAccessControl;
    //Access control info to be used by audio description decoder
    CAccessControl         AudioDescriptionAccessControl;

    // ===========================================================================================================
    // Streams available and various stream settings
    // ===========================================================================================================
public:
    //Check audio language settings against StreamInfo
    int                    UpdateAudioLanguage(bool isRequested = true);
    //Check audio description language settings against StreamInfo
    int                    UpdateAudioDescriptionLanguage(bool isRequested = true);
    //Check subtitle language settings against StreamInfo
    int                    UpdateSubtitleLanguage(bool isRequested = true);
    //Send PMT status event to media transport using latest StreamInfo
    void                   UpdateStreamInfoStatus(void);
public:
    //Whether to play primary or secndary audio program (SAP selection)
    bool                   UseSap;
    //Audio language handling
    CAudioLanguage         AudioLanguage;
    //Audio language for audio description track
    CAudioLanguage         AudioDescriptionLanguage;
    //Subtitle language handling
    CSubtitleLanguage      SubtitleLanguage;
    //List of stream info that are available for currently playing content
    CStreamInfoList        StreamInfo;

    // ===========================================================================================================
    // Currently active decoders
    // ===========================================================================================================
public:
    //Current video decoder in action
    void                   SetVideoContext(IDecoder* decoder);
    //Current audio decoder in action
    void                   SetAudioContext(IDecoder* decoder);
    //Current audio description decoder in action
    void                   SetAudioDescriptionContext(IDecoder* decoder);
    //Current closed captioning decoder in action
    void                   SetClosedCaptionContext(IDecoder* decoder);
public:
    //Video decoder in use
    IDecoder*              ActiveVideoDecoder;
    //Audio decoder in use
    IDecoder*              ActiveAudioDecoder;
    //Audio description decoder in use
    IDecoder*              ActiveAudioDescriptionDecoder;
    //Closed Captioning decoder in use
    IDecoder*              ActiveClosedCaptionDecoder;

    // ===========================================================================================================
    // Decoder stall check
    // ===========================================================================================================
public:
    //Have any of the decoders stalled
    bool                   HaveDecodersStalled(void);
private:
    //Tracking decoder stalls
    DWORD                  LastDecoderStallCheck;

    // ===========================================================================================================
    // Audio description volume setting
    // ===========================================================================================================
public:
    //Set the new audio description volume
    void                   OnSetAudioDescriptionVolume(int volume);
    //Returns audio description volume from -100 to 100, 0 if audio description is disabled
    int                    GetAudioDescriptionVolume(void) const;
    //The fade value parsed from the AD descriptor, which is used to fade main audio
    uint8                  GetAudioDescriptionFade(void) const;
private:
    //Audio description volume set by UI, relative to main audio
    int                    AudioDescriptionVolume;

    // ===========================================================================================================
    // Closed caption settings
    // ===========================================================================================================
public:
    //Setting closed captioning channels
    void                   SetCCChannel(int cc608, int cc708);
    //Current CC state
    int                    CC608;
    int                    CC708;

    // ===========================================================================================================
    // Handling hanging audio decoders
    // ===========================================================================================================
public:
    //Save the hanging audio decoder which needs to continue to play audio until expiration
    void                   SaveHangingAudioDecoder(IDecoder* decoder);
    //Delete the hanging audio decoder becaise it is of no more use
    void                   DeleteHangingAudioDecoder(void);
    //Save the hanging audio descriptiondecoder which needs to continue to play audio until expiration
    void                   SaveHangingAudioDescriptionDecoder(IDecoder* decoder);
    //Delete the hanging audio description decoder becaise it is of no more use
    void                   DeleteHangingAudioDescriptionDecoder(void);
private:
    //Audio decoders which are still expected to rendered buffer data until done
    IDecoder*              HangingAudioDecoder;
    IDecoder*              HangingAudioDescriptionDecoder;

    // ===========================================================================================================
    // Programme Ident Label or PIL is 20 bits, day month hour minute: dddd dmmm mhhh hhmm mmmm
    // Last Programme Ident Label (PIL) parsed from service
    // ===========================================================================================================
private:
    //Initializes last PIL
    void                   InitLastPil(void);
public:
    //Set last known PIL
    void                   SetLastPil(uint32 Pil);
    //Returns last known PIL
    //Also handles timeout if last pil is not set for a while (configurable)
    uint32                 GetLastPil(void);
private:
    uint32                 LastPilTickCount;
    uint32                 LastPil;

    // ===========================================================================================================
    // Handling OPL (output protection level)
    // ===========================================================================================================
private:
    //Initialize OPLs on channel changes
    void                   OPL_Initialize(void);
public:
    //Setting Macrovision levels from different supported sources
    void                   OPL_SetDRMMacrovision(uint32 level);
    void                   OPL_SetECMMacrovision(uint32 level);
    void                   OPL_SetWSSMacrovision(uint32 level);
    //Retrieve worst case Macrovision levels from all supported sources
    bool                   OPL_GetMacrovision(uint32& level);
    //Setting CGMS-A levels from different supported sources
    void                   OPL_SetDRMCGMSA(uint32 level);
    void                   OPL_SetECMCGMSA(uint32 level);
    void                   OPL_SetWSSCGMSA(uint32 level);
    //Retrieve worst case CGMS-A levels from all supported sources
    bool                   OPL_GetCGMSA(uint32& level);
private:
    //Macrovision level
    uint32                 MacrovisionDRMLevel;
    uint32                 MacrovisionECMLevel;
    uint32                 MacrovisionWSSLevel;
    uint32                 MacrovisionCurrentLevel;
    //CGMS/A level
    uint32                 CGMSADRMLevel;
    uint32                 CGMSAECMLevel;
    uint32                 CGMSAWSSLevel;
    uint32                 CGMSACurrentLevel;
};

// ===============================================================================================================
// ===============================================================================================================
