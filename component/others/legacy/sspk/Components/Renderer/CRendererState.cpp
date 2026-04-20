///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CRendererState.h"
#include "IReceiverControl.h"
#include "IDecoder.h"
#include "CReceiverConfiguration.h"
#include "CReceiverDiagnostics.h"
#include "CTuneRequest.h"
#include "CPipeId.h"
#include "CRendererStateDiags.h"
#include "Trace.h"
#include <string>
#include <map>
using namespace std;

//#define RENDERER_SPEW
#if defined(RENDERER_SPEW)
#define RENDERER_MSG(x) TRACE(x)
#else
#define RENDERER_MSG(x)
#endif

// ===============================================================================================================
// CRendererState class
//
// All IRenderer and downstream components are expected to use this class to keep persistent state and
// access properties that they need to playback content based on user settings etc.
// ===============================================================================================================

CRendererState::CRendererState(IReceiverControl* receiverControl, const string& pipeId, uint32 featuresEnabled)
    : mReceiverControl(receiverControl)
    , mDecoderFactory(mReceiverControl->GetAVManager()->GetDecoderFactory())
    , mDiagnostics(mReceiverControl->GetDiagnostics())
    , mPipeId(pipeId)
    , mPipeIdN(PipeId_StringToU32(mPipeId))
    , mFeatureEnabled(featuresEnabled)
    , IsFullScreen(IsFeatureEnabled(eFeature_FullscreenMode))
    , IsImmediateMode(false)
    , IsIFrameOnlyMode(false)
    , PlaybackRate(1.0)
    , StartTime(0)
    , EndTime(0)
    , DefaultProgram(-1)
    , TearDownPicture(false)
    , IsMHEGMode(false)
    , FlushVideoDecoderOnRelease(true)
    , FlushAudioDecoderOnRelease(true)
    , IsStoppedForAd(false)
    , IsPlayingAd(false)
    , VideoAccessControl(mPipeIdN, true, false)
    , AudioAccessControl(mPipeIdN, false, false)
    , AudioDescriptionAccessControl(mPipeIdN, false, true)
    , UseSap(false)
    , AudioLanguage(mPipeIdN, false)
    , AudioDescriptionLanguage(mPipeIdN, true)
    , SubtitleLanguage(mPipeIdN)
    , ActiveVideoDecoder(NULL)
    , ActiveAudioDecoder(NULL)
    , ActiveAudioDescriptionDecoder(NULL)
    , ActiveClosedCaptionDecoder(NULL)
    , LastDecoderStallCheck(0)
    , AudioDescriptionVolume(0)
    , CC608(0)
    , CC708(0)
    , HangingAudioDecoder(NULL)
    , HangingAudioDescriptionDecoder(NULL)
{
    //Initialize access control globals
    CAccessControl::InitAccessControl(mPipeIdN);

    //Initialize last PIL
    InitLastPil();

    //Initialize OPL
    OPL_Initialize();
}

CRendererState::~CRendererState()
{
}

void CRendererState::OnReset(void)
{
    //Release any hanging audio decoders which may be sticking around to render their buffers
    DeleteHangingAudioDecoder();
    DeleteHangingAudioDescriptionDecoder();
    //Set the flag to signal whether the picture needs to be torndown
    TearDownPicture = false;

    //Clear stream info on reset
    StreamInfo.Clear();

    //Checking for decoder stalls
    LastDecoderStallCheck = 0;

    //Clear last VPS related information
    InitLastPil();
}

void CRendererState::OnSync(void)
{
    //Release any hanging audio decoders which may be sticking around to render their buffers
    DeleteHangingAudioDecoder();
    DeleteHangingAudioDescriptionDecoder();
}

void CRendererState::OnStart(CTuneRequest& tuneRequest)
{
    //Current Program Id to playback
    DefaultProgram = tuneRequest.GetInt(TUNE_REQUEST_PROGRAMID);

    //Set immediate mode if we are in trick mode
    //Note that we do this only for non VOD cases; otherwise it should be set to false
    IsImmediateMode = tuneRequest.GetBool(TUNE_REQUEST_ISIMMEDIATEMODE);
    //Save IFrameOnly mode
    IsIFrameOnlyMode = tuneRequest.GetBool(TUNE_REQUEST_IFRAMEONLY);

    PlaybackRate = tuneRequest.GetFloat(TUNE_REQUEST_SPEED);
}

void CRendererState::OnDetune(void)
{
    //Clear start end time of the asset
    StartTime = 0;
    EndTime = 0;

    //reinitialize OPL on channel changes
    OPL_Initialize();
}

void CRendererState::OnTimeslice(void)
{
    //Delete the hanging audio decoder when it si done rendering
    if (HangingAudioDecoder && HangingAudioDecoder->IsAcquired() == 0)
    {
        mDecoderFactory->DisposeDecoder(HangingAudioDecoder);
        HangingAudioDecoder = NULL;
    }
    //Delete the hanging audio description decoder when it is done rendering
    if (HangingAudioDescriptionDecoder && HangingAudioDescriptionDecoder->IsAcquired() == 0)
    {
        mDecoderFactory->DisposeDecoder(HangingAudioDescriptionDecoder);
        HangingAudioDescriptionDecoder = NULL;
    }
}

// ===============================================================================================================
// Access (parental) control settings
// ===============================================================================================================

void CRendererState::OnSetAccessControl(bool blocked, uint64 startTime, uint64 endTime)
{
    //Set parental control modes
    ACCESS_TIME_BOUNDARY boundary;
    if (!boundary.SetValue(blocked, startTime, endTime))
    {
        mDiagnostics.PostEvent(new CDiagsReceiverAccessControlBadTimeBoundary());
    }

    TRACE
        ((
        "======================> SetAccessControl (%08x) : %s, %llu time boundary: %llu <============================",
        mPipeIdN, ACCESS_TIME_BOUNDARY::Enum2String(boundary.nBlock), boundary.nTime1, boundary.nTime2
        ));

    VideoAccessControl.SetAccessControl(boundary);
    AudioAccessControl.SetAccessControl(boundary);
    AudioDescriptionAccessControl.SetAccessControl(boundary);
}

// ===============================================================================================================
// ===============================================================================================================

int CRendererState::UpdateAudioLanguage(bool isRequested/* = true*/)
{
    //Delete any hanging audio decoder whan application wants to change language explicitly
    if (isRequested)
    {
        DeleteHangingAudioDecoder();
    }
    //Let the media transport know about current audio being rendered
    if (IsFeatureEnabled(eFeature_StreamNotification))
    {
        mReceiverControl->NotifyStatus(AudioLanguage.SendToMediaTransport(isRequested));
        if (isRequested)
        {
            int language, type, pid, mode;
            AudioLanguage.GetUsed(language, type, pid, mode);
            mDiagnostics.PostEvent(new CDiagsReceiverAudioLanguageEvent(language, type, pid, mode));
        }
    }
    //Return currently selected audio language pid
    return AudioLanguage.GetUsedPid();
}

int CRendererState::UpdateAudioDescriptionLanguage(bool isRequested/* = true*/)
{
    //Delete any hanging audio description decoder whan application wants to change language explicitly
    if (isRequested)
    {
        DeleteHangingAudioDescriptionDecoder();
    }
    //Let the media transport know about current audio desc being rendered
    if (IsFeatureEnabled(eFeature_StreamNotification))
    {
        mReceiverControl->NotifyStatus(AudioDescriptionLanguage.SendToMediaTransport(isRequested));
        if (isRequested)
        {
            int language, type, pid, mode;
            AudioDescriptionLanguage.GetUsed(language, type, pid, mode);
            mDiagnostics.PostEvent(new CDiagsReceiverAudioDescriptionLanguageEvent(language, type, pid, mode));
        }
    }
    //Return currently selected audio description language pid
    return AudioDescriptionLanguage.GetUsedPid();
}

int CRendererState::UpdateSubtitleLanguage(bool isRequested/* = true*/)
{
    //Let the media transport know about current subtitle being rendered
    if (IsFeatureEnabled(eFeature_StreamNotification))
    {
        mReceiverControl->NotifyStatus(SubtitleLanguage.SendToMediaTransport(isRequested));
        if (isRequested)
        {
            int language, type, pid, mode;
            SubtitleLanguage.GetUsed(language, type, pid, mode);
            mDiagnostics.PostEvent(new CDiagsReceiverSubtitleLanguageEvent(language, type, pid, mode));
        }
    }
    //Return currently selected subtitle language pid
    return SubtitleLanguage.GetUsedPid();
}

void CRendererState::UpdateStreamInfoStatus(void)
{
    int language, type, pid, mode;
    if (IsFeatureEnabled(eFeature_StreamNotification))
    {
        //Pass on PMT status to media transport
        mReceiverControl->NotifyStatus("status=pmt&data=" + escape(StreamInfo.GenerateXmlOutput()));

        //Tell media transport about the streams being rendered
        //
        //Let the media transport know about current audio being rendered
        mReceiverControl->NotifyStatus(AudioLanguage.SendToMediaTransport(false));
        AudioLanguage.GetUsed(language, type, pid, mode);
        mDiagnostics.PostEvent(new CDiagsReceiverAudioLanguageEvent(language, type, pid, mode));
        //Let the media transport know about current audio description being rendered
        mReceiverControl->NotifyStatus(AudioDescriptionLanguage.SendToMediaTransport(false));
        AudioDescriptionLanguage.GetUsed(language, type, pid, mode);
        mDiagnostics.PostEvent(new CDiagsReceiverAudioDescriptionLanguageEvent(language, type, pid, mode));
        //Let the media transport know about current subtitle being rendered
        mReceiverControl->NotifyStatus(SubtitleLanguage.SendToMediaTransport(false));
        SubtitleLanguage.GetUsed(language, type, pid, mode);
        mDiagnostics.PostEvent(new CDiagsReceiverSubtitleLanguageEvent(language, type, pid, mode));
    }
}

// ===============================================================================================================
// Currently active decoders
// ===============================================================================================================

void CRendererState::SetVideoContext(IDecoder* decoder)
{
    //Current video decoder in action
    ActiveVideoDecoder = decoder;
}

void CRendererState::SetAudioContext(IDecoder* decoder)
{
    //Current audio decoder in action
    ActiveAudioDecoder = decoder;
}

void CRendererState::SetAudioDescriptionContext(IDecoder* decoder)
{
    //Current audio description decoder in action
    ActiveAudioDescriptionDecoder = decoder;
}

void CRendererState::SetClosedCaptionContext(IDecoder* decoder)
{
    //Current video decoder in action
    ActiveClosedCaptionDecoder = decoder;
}

// ===============================================================================================================
// Decoder stall check
// ===============================================================================================================

bool CRendererState::HaveDecodersStalled(void)
{
    //Check for stall at most 10 times per second
    DWORD now = Executive_GetTickCount();
    
    if ((now - LastDecoderStallCheck) > 100)
    {
        //Update internal timer
        LastDecoderStallCheck = now;

        // Perform CheckStall on all active decoders each time we check because
        //     they may be depending on being polled regularly to generate a result.        
        bool videoIsStalled = (NULL == ActiveVideoDecoder) || ActiveVideoDecoder->CheckStall();
        bool audioIsStalled = (NULL == ActiveAudioDecoder) || ActiveAudioDecoder->CheckStall();
        
        //Only return true when all active decoders are stalled because if one is still running
        //    there may be a discontinuity in only one stream the remaining stream should continue to play. 
        return (videoIsStalled && audioIsStalled);
    }
    return false;
}

// ===============================================================================================================
// Audio description volume setting
// ===============================================================================================================

void CRendererState::OnSetAudioDescriptionVolume(int volume)
{
    //Set the new audio description volume
    AudioDescriptionVolume = volume;
    //Send a diagnostics event
    mDiagnostics.PostEvent(new CDiagsReceiverAudioDescriptionVolumeEvent(volume));
}

int CRendererState::GetAudioDescriptionVolume(void) const
{
    //If we have a preferred language, then AD is enabled, so return the actual volume; else return 0.
    //We want to return the actual volume when enabled even if there is no active AD stream because
    //we don't want main audio volume to vary from channel to channel.
    return AudioDescriptionLanguage.IsValid() ? AudioDescriptionVolume : 0;
}

uint8 CRendererState::GetAudioDescriptionFade(void) const
{
    //Default of 0, indicating no fade of main audio
    return ActiveAudioDescriptionDecoder ? ActiveAudioDescriptionDecoder->GetAudioDescriptionFade() : 0;
}

// ===============================================================================================================
// Closed caption settings
// ===============================================================================================================

void CRendererState::SetCCChannel(int cc608, int cc708)
{
    //Set current closed captioning channels
    CC608 = cc608;
    CC708 = cc708;

    //Pass it down to the active closed captioning decoder
    if (IsFeatureEnabled(eFeature_ClosedCaptioning) && ActiveClosedCaptionDecoder)
    {
        DECODERCC_CHANNELS value;
        value.n608Channel = cc608;
        value.n708Channel = cc708;
        ActiveClosedCaptionDecoder->IoControl(DECODER_CCTPYE, &value, sizeof(value), NULL, 0);
    }
}

// ===============================================================================================================
// Handling hanging audio decoders
// ===============================================================================================================

void CRendererState::SaveHangingAudioDecoder(IDecoder* decoder)
{
    //Called to save the audio decoder which is currently playing audio and
    //we need to let it continue until given time
    HangingAudioDecoder = decoder;
}

void CRendererState::DeleteHangingAudioDecoder(void)
{
    //Dump the hanging audio decoder right away
    if (HangingAudioDecoder)
    {
        mDecoderFactory->DisposeDecoder(HangingAudioDecoder);
        HangingAudioDecoder = NULL;
    }
}

void CRendererState::SaveHangingAudioDescriptionDecoder(IDecoder* decoder)
{
    //Called to save the audio decoder which is currently playing audio and
    //we need to let it continue until given time
    HangingAudioDescriptionDecoder = decoder;
}

void CRendererState::DeleteHangingAudioDescriptionDecoder(void)
{
    //Dump the hanging audio decoder right away
    if (HangingAudioDescriptionDecoder)
    {
        mDecoderFactory->DisposeDecoder(HangingAudioDescriptionDecoder);
        HangingAudioDescriptionDecoder = NULL;
    }
}

// ===============================================================================================================
// Programme Ident Label or PIL is 20 bits, day month hour minute: dddd dmmm mhhh hhmm mmmm
// Last Programme Ident Label (PIL) parsed from service
// ===============================================================================================================

void CRendererState::InitLastPil(void)
{
    //Clear last VPS related information
    LastPilTickCount = 0;
    LastPil = 0;
}

void CRendererState::SetLastPil(uint32 Pil)
{
    LastPilTickCount = Executive_GetTickCount();
    LastPil = Pil;
}

uint32 CRendererState::GetLastPil(void)
{
    if (LastPil != 0 && gReceiverConfiguration.LastPILTimeout != 0)
    {
        if (Executive_GetTickCount() >= (LastPilTickCount + (gReceiverConfiguration.LastPILTimeout * 1000)))
        {
            //Reset the LastPil
            LastPilTickCount = 0;
            LastPil = 0;
        }
    }
    return LastPil;
}

// ===============================================================================================================
// Handling OPL (output protection level)
// ===============================================================================================================

void CRendererState::OPL_Initialize(void)
{
    //Clear Macrovision flags
    MacrovisionDRMLevel = 0;
    MacrovisionECMLevel = 0;
    MacrovisionWSSLevel = 0;
    MacrovisionCurrentLevel = 0;
    //Clear CGMS/A flags
    CGMSADRMLevel = 0;
    CGMSAECMLevel = 0;
    CGMSAWSSLevel = 0;
    CGMSACurrentLevel = 0;
}

void CRendererState::OPL_SetDRMMacrovision(uint32 level)
{
    if (level != MacrovisionDRMLevel)
    {
        MacrovisionDRMLevel = level;
        mDiagnostics.PostEvent(new CDiagsReceiverMacrovisionDRMEvent(level));
    }
}

void CRendererState::OPL_SetECMMacrovision(uint32 level)
{
    if (level != MacrovisionECMLevel)
    {
        MacrovisionECMLevel = level;
        mDiagnostics.PostEvent(new CDiagsReceiverMacrovisionECMEvent(level));
    }
}

void CRendererState::OPL_SetWSSMacrovision(uint32 level)
{
    if (level != MacrovisionWSSLevel)
    {
        MacrovisionWSSLevel = level;
        mDiagnostics.PostEvent(new CDiagsReceiverMacrovisionWSSEvent(level));
    }
}

bool CRendererState::OPL_GetMacrovision(uint32& level)
{
    //We will be applying worst case Macrovision setting soming from
    //either of WSS or ECM or from DRM (PlayReady or WMDRM)
    level = MAX(MacrovisionWSSLevel, MacrovisionECMLevel);
    level = MAX(level, MacrovisionDRMLevel);
    if (level != MacrovisionCurrentLevel)
    {
        MacrovisionCurrentLevel = level;
        return true;
    }
    return false;
}

void CRendererState::OPL_SetDRMCGMSA(uint32 level)
{
    if (level != CGMSADRMLevel)
    {
        CGMSADRMLevel = level;
        mDiagnostics.PostEvent(new CDiagsReceiverCgmsaDRMEvent(level));
    }
}

void CRendererState::OPL_SetECMCGMSA(uint32 level)
{
    if (level != CGMSAECMLevel)
    {
        CGMSAECMLevel = level;
        mDiagnostics.PostEvent(new CDiagsReceiverCgmsaECMEvent(level));
    }
}

void CRendererState::OPL_SetWSSCGMSA(uint32 level)
{
    if (level != CGMSAWSSLevel)
    {
        CGMSAWSSLevel = level;
        mDiagnostics.PostEvent(new CDiagsReceiverCgmsaWSSEvent(level));
    }
}

bool CRendererState::OPL_GetCGMSA(uint32& level)
{
    //We will be applying worst case CGMS-A setting soming from
    //either of WSS or ECM or from DRM (PlayReady or WMDRM)
    level = MAX(CGMSAWSSLevel, CGMSAECMLevel);
    level = MAX(level, CGMSADRMLevel);
    if (level != CGMSACurrentLevel)
    {
        CGMSACurrentLevel = level;
        return true;
    }
    return false;
}

// ===============================================================================================================
// ===============================================================================================================
