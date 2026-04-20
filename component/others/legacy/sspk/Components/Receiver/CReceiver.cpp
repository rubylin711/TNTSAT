///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDiagsManager.h"
#include "CReceiver.h"
#include "ITimeslice.h"
#include "ISocket.h"
#include "IRenderer.h"
#include "IHalDecoder.h"
#include "IDecoder.h"
#include "IPacket.h"
#include "IPacketMonitor.h"
#include "CReceiverConfiguration.h"
#include "StringUtils.h"
#include "Trace.h"
#include <string>
#include <vector>
#include <list>
#include "SSPKTimeSpan.h"
#include <sys/time.h>

using namespace std;

//#define RECEIVER_SPEW
#if defined(RECEIVER_SPEW)
#define RECEIVER_MSG(x) TRACE(x)
#else
#define RECEIVER_MSG(x)
#endif

// ===============================================================================================================
// CReceiver class
// ===============================================================================================================

CReceiver::CReceiver(IAVManager* avManager, ITunerSessionCallback* callback, const string& pipeId, uint32 featuresEnabled)
    : mAVManager(avManager)
    , mDiagsManager(mAVManager->GetDiagsManager())
    , mTimesliceManager(mAVManager->GetTimesliceManager())
    , mSocketFactory(mAVManager->GetSocketFactory())
    , mHalDecoderFactory(mAVManager->GetHalDecoderFactory())
    , mDecoderFactory(mAVManager->GetDecoderFactory())
    , mRendererFactory(mAVManager->GetRendererFactory())
    , mSpliceEngineFactory(mAVManager->GetSpliceEngineFactory())
    , mStatusCallback(callback)
    , mRendererState(NULL)
    , mDiagnostics(NULL)
    , mClock(NULL)
    , mPacketMonitor(NULL)
    , mTuneRequest()
    , mStatus(kReceiverNotificationType_Stopped)
    , BitRate(0)
    , CurrentBitRate(0)
    , mMeasuredNetworkBitsPerSec(0)
    , mSocket(NULL)
    , mRenderer(NULL)
    , IsDetuned(true)
    , CallingThreadHandle(NULL)
    , CallingThreadPriority(250)
    , MediaTransportId(0)
    , UniqueId(0)
    , BlackoutOnlyTune(false)
    , IsPauseLiveTransition(false)
    , FirstPacketReceived(false)
    , mRetry(true)
    , mResetOnStall(true)
    , WaitWhileStalledEvent(CEvent::eResetModeAuto)
    , CurrentTunerError(eTunerErrorNone)
    , CurrentSocketError(eSocketErrorNone)
    , CurrentSocketPKResult(pkS_OK)
    , CurrentSocketHttpResponse(0)
    , NearEnd(false)
    , NearEndNotificationDataSet(false)
    , Stopped(true)
{
    RECEIVER_MSG(("[%08x] Construction", mPipeIdN));

    mRendererState = NEW_NO_THROW CRendererState(this, pipeId, featuresEnabled);
    ASSERT(mRendererState);

    mDiagnostics = NEW_NO_THROW CReceiverDiagnostics(this, mRendererState->mPipeIdN);
    ASSERT(mDiagnostics);

    mClock = NEW_NO_THROW Clock(this);
    ASSERT(mClock);

    mPipeIdN = mRendererState->mPipeIdN;

    AcquireClock();

    //Tracking of signal loss
    SignalLoss_Initialize();

    //Initialize clip play parameters
    ClipPlay_Initialize();

    //Acquire a splice engine
    if (mRendererState->IsFeatureEnabled(CRendererState::eFeature_Splicing))
    {
        mSpliceEngine = mSpliceEngineFactory->AcquireSpliceEngine(this);
    }
    else
    {
        mSpliceEngine = NULL;
    }

    //Register renderer for the timeslice callback queue
    mTimesliceManager->Register(this);

    //Register as IDiagsProvider - do not lock me
    if (mRendererState->IsFeatureEnabled(CRendererState::eFeature_DiagsProvider))
    {
        mDiagsManager->Attach(this);
    }

    RECEIVER_MSG(("[%08x] Construction ...done", mPipeIdN));
}

CReceiver::~CReceiver()
{
    RECEIVER_MSG(("[%08x] Destruction", mPipeIdN));

    //Sanity check on state of the renderer on final cleanup
    ASSERT(Stopped == true);
    ASSERT(mSocket == NULL);
    ASSERT(mRenderer == NULL);

    //Unregister as IDiagsProvider - do not lock me
    if (mRendererState->IsFeatureEnabled(CRendererState::eFeature_DiagsProvider))
    {
        mDiagsManager->Detach(this);
    }

    //UnRegister renderer from the timeslice callback queue
    mTimesliceManager->UnRegister(this);

    //Dump all streams
    Reset(true);

    if (mSpliceEngine)
    {
        delete mSpliceEngine;
        mSpliceEngine = NULL;
    }

    //Purge the HAL decoder factory of all decoders attached to this pipe
    //
    //NOTE: Ideally it would be better not to do this here to enable 
    //      deleting receivers without purging the HAL decoder factory.
    
    if (mRendererState->IsFeatureEnabled(CRendererState::eFeature_RenderingPipe))
    {
        mHalDecoderFactory->PurgeDecoders(mPipeIdN);
    }

    ReleaseClock();

    if (mPacketMonitor)
    {
        delete mPacketMonitor;
    }
    
    if (mClock)
    {
        delete mClock;
    }

    if (mDiagnostics)
    {
        delete mDiagnostics;
    }

    if (mRendererState)
    {
        delete mRendererState;
    }

    RECEIVER_MSG(("[%08x] Destruction ...done", mPipeIdN));
}

void CReceiver::AcquireClock(void)
{
    AutoLock lock(&Renderer_Lock);
    if (mRendererState->IsFeatureEnabled(CRendererState::eFeature_RenderingPipe))
    {
        RECEIVER_MSG(("[%08x] Acquiring clock", mPipeIdN));

        mClock->Acquire(mHalDecoderFactory, mRendererState->IsFullScreen);

        RECEIVER_MSG(("[%08x] Acquiring clock ...done(%d)", mPipeIdN));
    }
}

void CReceiver::ReleaseClock(void)
{
    AutoLock lock(&Renderer_Lock);
    if (mRendererState->IsFeatureEnabled(CRendererState::eFeature_RenderingPipe))
    {
        RECEIVER_MSG(("[%08x] Releasing clock", mPipeIdN));

        mClock->Release();

        RECEIVER_MSG(("[%08x] Releasing clock ...done", mPipeIdN));
    }
}

void CReceiver::Reset(bool teardownPicture)
{
    AutoLock lock(&Renderer_Lock);

    RECEIVER_MSG(("[%08x] Reset(teardownPicture:%s)",
        mPipeIdN,
        teardownPicture ? "TRUE" : "FALSE"));

    //Stop and reinitialize the clock
    mClock->Stop();

    //Set the flag to signal whether the picture needs to be torndown
    //This needs to be set before the renderer is destructed
    mRendererState->TearDownPicture = teardownPicture;

    //And release the downstream streamer handlers
    if (mRenderer)
    {
        delete mRenderer;
        mRenderer = NULL;
    }

    //Reset renderer state
    mRendererState->OnReset();

    //Make sure clock is notified
    mClock->OnReset(NULL);

    //Whether we received the first packet
    FirstPacketReceived = false;

    //Clear the tuning errors
    CurrentTunerError = eTunerErrorNone;
    CurrentSocketError = eSocketErrorNone;
    CurrentSocketHttpResponse = 0;
    CurrentSocketPKResult = pkS_OK;

    //Reset the flag for rendering completion check
    NearEnd = false;
    memset(&NearEndNotificationData, 0, sizeof(CReceiverNotificationData));
    NearEndNotificationDataSet = false;

    //Start diagnostics afresh when a new stream start
    DiagsReset();

    RECEIVER_MSG(("[%08x] Reset(%d) ...done", mPipeIdN, teardownPicture));
}

// ===============================================================================================================
// IReceiverControl APIs
// ===============================================================================================================

void CReceiver::Sync(SyncReason reason, bool sync, bool cleanStall)
{
    TRACE(("[%08x] Sync(%d, %d, %d)", mPipeIdN, reason, sync, cleanStall));

    //Handle rebuffer events
    OnRebuffer(reason, sync, cleanStall);

    //Stop and reset the clock
    mClock->Stop();

    //Sync the downstream stream handlers
    if (mRenderer)
    {
        mRenderer->OnSync(sync, cleanStall);
    }

    //Sync renderer state
    mRendererState->OnSync();

    //Let the clock know we are rebuffering
    mClock->OnSync();

    TRACE(("[%08x] Sync(%d, %d, %d) ...done", mPipeIdN, reason, sync, cleanStall));
}

void CReceiver::OnRebuffer(SyncReason reason, bool sync, bool cleanStall)
{
    //Send a diagnostics event
    uint32 count = mDiagnostics->OnRebuffer(reason, sync, cleanStall);

    //Send underrun events to media transport
    if ((reason == kSyncReason_VideoBufferUnderrun) || (reason == kSyncReason_AudioBufferUnderrun))
    {
        NotifyStatus("status=" + (sync ? string("underrun") : string("rebuffer"))
                                + "&count=" + toString(count, true)
                                + "&type="  + (reason == kSyncReason_VideoBufferUnderrun ? string("video") : string("audio")));
    }
}

void CReceiver::NotifyStatus(const string& status)
{
    if (mStatusCallback != NULL)
    {
        mStatusCallback->StatusCallback
            (
            status +
            "&pipe=" + mRendererState->mPipeId +
            "&mtid=" + toString(MediaTransportId, true) +
            "&uid=" + toString(UniqueId, true) +
            "&currenttime=" + toString64(GetCurrentPlaybackTime(), true)
            );
    }
}

void CReceiver::RegisterForTimeslice(ITimeslice* timeslice)
{
    //This method is expected to be called in the decoder object instaintiation
    //path and should already be protected by the renderer lock...
    TimesliceList.push_back(timeslice);
}

void CReceiver::UnRegisterForTimeslice(ITimeslice* timeslice)
{
    //This method is expected to be called in the decoder object instaintiation
    //path and should already be protected by the renderer lock...
    TimesliceList.remove(timeslice);
}

// ===============================================================================================================
// IReceiver APIs
// ===============================================================================================================

void CReceiver::Start(void)
{
    RECEIVER_MSG(("[%08x] Start", mPipeIdN));

    //Just exit if we are already started
    if (!Stopped)
        return;

    //We are starting to do some work now
    Stopped = false;

    //Initialize the downstream components
    {
        AutoLock lock(&Renderer_Lock);

        //Update renderer state on start
        mRendererState->OnStart(mTuneRequest);

        //Whether we received the first packet
        FirstPacketReceived = IsPauseLiveTransition;

        //Whether signal loss check is enabled
        SignalLoss_Enable();

        //Initialize the clock
        mClock->OnStart(!IsPauseLiveTransition);
    }

    RECEIVER_MSG(("[%08x] Start ...done", mPipeIdN));
}

void CReceiver::SignalStop(void)
{
    //Stop doing any work now
    Stopped = true;
}

void CReceiver::Stop(_In_ bool bForced, _In_opt_ eTunerError tunerError /*= eTunerErrorNone*/)
{
    RECEIVER_MSG(("[%08x] Stop", mPipeIdN));

    //Stop doing any work now
    Stopped = true;

    //Handle secondary stream ending scenario
    if (mSpliceEngine && mSpliceEngine->IsSecondaryStreamEnd())
        return;

    //Set the flag for rendering completion check
    if (!bForced)
    {
        CurrentTunerError = tunerError;
        SignalNearEnd(NULL);
    }

    RECEIVER_MSG(("[%08x] Stop ...done", mPipeIdN));
}

void CReceiver::Error(eTunerError tunerError, eSocketError socketError, pkRESULT pkResult, int httpResponse)
{
    //Handle error while streaming secondary stream
    if (mSpliceEngine && mSpliceEngine->IsSecondaryStreamError(tunerError, socketError, httpResponse))
        return;

    //Stop the receiver first
    Stop(true);

    //Detune because there was an error while connecting or receiving stream
    CurrentTunerError = tunerError;
    CurrentSocketError = socketError;
    CurrentSocketPKResult = pkResult;
    CurrentSocketHttpResponse = httpResponse;

    TRACE_ERROR(("[%08x] TunerError=%d SocketError=%d pkResult=0x%08x httpResponse=%d",
        mPipeIdN, CurrentTunerError, CurrentSocketError, CurrentSocketPKResult, CurrentSocketHttpResponse));

    //Post a diagnostics event
    mDiagnostics->OnSocketError(CurrentTunerError, CurrentSocketError, CurrentSocketPKResult, CurrentSocketHttpResponse);
    //Receiver is finished, media may still be playing from elementary stream buffers
    SetStatus(kReceiverNotificationType_Detuned, NULL);
    //Set the flag for rendering completion check
    NearEnd = false;
}

void CReceiver::Notify(ReceiverNotificationType notificationType, CReceiverNotificationData* notificationData)
{
    //Custom handling of events
    //
    //Please make sure that there is always a handler for each new event that gets added
    //
    switch (notificationType)
    {
    //Called when receiver is stopped
    case kReceiverNotificationType_Stopped:
    //Called when data is about to stream from connected socket
    case kReceiverNotificationType_DataReady:
    //Socket has started to play an item from play list
    case kReceiverNotificationType_PlayingItem:
    //Receiver is now paused
    case kReceiverNotificationType_Paused:
    //Receiver has been detuned
    case kReceiverNotificationType_Detuned:
        {
            SetStatus(notificationType, notificationData);
        }
        break;

    //Socket calls to reset receiver/renderer on state change (WMS and MBR only)
    case kReceiverNotificationType_Reset:
        {
            RECEIVER_MSG(("[%08x] Notify Reset", mPipeIdN));

            //Post a tuner update event before quitting
            mDiagsManager->PostEvent(RetrieveTunerDiagnostics());
            //Post an renderer update event before quitting
            mDiagsManager->PostEvent(DiagsRetrieveInternal());

            //Reset clock if any data has been pushed to decoder
            Reset(false);

            if (mTuneRequest.IsWms())
            {
                //Whether WMS content playing in trick mode
                mRendererState->IsIFrameOnlyMode = notificationData->Reset.IsIFrameOnlyMode;
                //Update immediate mode
                mRendererState->IsImmediateMode = false;
                mTuneRequest.SetArg(TUNE_REQUEST_ISIMMEDIATEMODE, "false");

                //Current trick speed
                mTuneRequest.Speed = notificationData->Reset.Speed;

                //Make sure clock is updated after reset
                mClock->OnReset(notificationData);
            }

            RECEIVER_MSG(("[%08x] Notify Reset ...done", mPipeIdN));
        }
        break;

    //Receiver retry mode
    case kReceiverNotificationType_CanRetry:
        {
            mRetry = notificationData->Retry.CanRetry;
            mResetOnStall = notificationData->Retry.ResetOnStall;
        }
        break;

    //Called to set stream buffer delay
    case kReceiverNotificationType_StreamBuffer:
        {
            AutoLock lock(&Renderer_Lock);
            mClock->SetStreamBufferDelay(notificationData->StreamBuffer.Delay);
            mDiagnostics->OnBufferDelay(notificationData->StreamBuffer.Delay);
        }
        break;

    //Called to set stream buffer delay
    case kReceiverNotificationType_StreamBufferDefault:
        {
            AutoLock lock(&Renderer_Lock);
            mClock->SetStreamBufferDefaultDelay(notificationData->StreamBufferDefault.Delay);
        }
        break;

    //Called to set the clock mode
    case kReceiverNotificationType_ClockMode:
        {
            AutoLock lock(&Renderer_Lock);
            mClock->SetClockMode(notificationData->Clock.Mode);
        }
        break;

    //Socket updates current bitrate as is acquired inband after tuning (WMS only)
    case kReceiverNotificationType_BitRate:
        {
            if (notificationData->BitRate.BPS)
            {
                BitRate = notificationData->BitRate.BPS;
            }
            RECEIVER_MSG(("[%08x] BitRate %d", mPipeIdN, notificationData->BitRate.BPS));
            mDiagnostics->OnBitrate(notificationData->BitRate.BPS);
        }
        break;

    //Socket updates current bitrate being streamed
    //For example, smooth streaming video stream bitrate when quality level changes
    case kReceiverNotificationType_CurrentBitRate:
        {
            if (notificationData->BitRate.BPS)
            {
                CurrentBitRate = notificationData->BitRate.BPS;
                string status = "status=currentbitrate&bps=";
                status += toString(CurrentBitRate);
                NotifyStatus(status);
            }
            RECEIVER_MSG(("[%08x] CurrentBitRate %d", mPipeIdN, notificationData->BitRate.BPS));
        }
        break;

    //Called when audio languages become available (WMS only)
    case kReceiverNotificationType_AudioLanguages:
        {
            //Nothing to be handfled here as yet... already handled upstream
        }
        break;

    //Called to notify availability of various types of streams
    //For example, Smooth Streaming manifest will be represented here
    case kReceiverNotificationType_StreamDescUpdate:
        {
            RECEIVER_MSG(("[%08x] Notify StreamDescriptor update", mPipeIdN));
            int language = 0;
            int type = 0;
            int pid = 0;
            int mode = 0;

            if (notificationData->StreamDescUpdate.InitialSelection)
            {
                //Allow language settings to process new stream description vector
                AutoLock lock(&Renderer_Lock);
                //Save the available streams
                mRendererState->StreamInfo = *(notificationData->StreamDescUpdate.StreamInfo);
                //Get and set the expected pid
                mRendererState->AudioLanguage.GetUsed(language,type, pid, mode);
                mRendererState->AudioLanguage.SetExpected(language, type, notificationData->StreamDescUpdate.AudioPID);
                //Find the audio language stream to be rendererd
                mRendererState->AudioLanguage.Find(*mRendererState);
                //Now update the state and get currently selected audio language stream
                notificationData->StreamDescUpdate.AudioPID = mRendererState->UpdateAudioLanguage(false);
                //Find subtitle language stream to be rendered
                mRendererState->SubtitleLanguage.Find(*mRendererState);
                //Now update the state and get currently selected subtitle language stream
                notificationData->StreamDescUpdate.SubtitlePID = mRendererState->UpdateSubtitleLanguage(false);
                //Update the stream availability status to media transport
                mRendererState->UpdateStreamInfoStatus();
            }
            else
            {
                // There is an audio language change
                //Override the existing audio language selection
                mRendererState->AudioLanguage.GetUsed(language, type, pid, mode);
                SetAudioLanguage(language, type, notificationData->StreamDescUpdate.AudioPID);
            }
            RECEIVER_MSG(("[%08x] Notify StreamDescriptor update ...done", mPipeIdN));
        }
        break;

    //Called for PlayReady OPL settings
    case kReceiverNotificationType_DrmOutputProtectionLevel:
        {
            mDiagnostics->OnDrmOutputProtectionLevel(notificationData);

            //This is to support output protection as set by upstream DRM (in the current case WMDRM and PlayReady)
            //TODO: Move this support to CPlayReadyDecrypter?
            mRendererState->OPL_SetDRMCGMSA(notificationData->DrmOutputProtectionLevel.CGMSALevel);
            mRendererState->OPL_SetDRMMacrovision(notificationData->DrmOutputProtectionLevel.MacrovisionLevel);

            //The following two applies only when in fullscreen mode...
            if (mRendererState->IsFeatureEnabled(CRendererState::eFeature_PlayreadyOPL))
            {
                mDecoderFactory->HandleHDCPOutput(mPipeIdN, notificationData->DrmOutputProtectionLevel.HDCPAction);
                mDecoderFactory->HandleComponentVideoOutput(mPipeIdN, notificationData->DrmOutputProtectionLevel.DisableComponentVideo);
            }
        }
        break;

    //Called when sockets need to send events to Media Transport layer
    case kReceiverNotificationType_MediaTransportEvent:
        {
            NotifyStatus(string(notificationData->MediaTransportEvent.EventStringPtr));
        }
        break;

    //Called when socket is connected
    case kReceiverNotificationType_OnConnected:
        {
            mDiagnostics->OnConnected(notificationData);
            if (mSpliceEngine) mSpliceEngine->StreamJoined(notificationData);
        }
        break;

    //Called when socket sees first packet (WMS only)
    case kReceiverNotificationType_OnFirstPacket:
        {
            mDiagnostics->OnFirstPacket(notificationData);
        }
        break;

    //Called when socket sees discontinuity (WMS/MBR only)
    case kReceiverNotificationType_Discontinuity:
        {
            mDiagnostics->OnDiscontinuity(notificationData);
        }
        break;

    //Called when socket state changes (WMS only)
    case kReceiverNotificationType_StateChangeEvent:
        {
            mDiagnostics->OnStateChange(notificationData);
        }
        break;

    //Called to notify socket thread CPU consumption
    case kReceiverNotificationType_ThreadTimes:
        {
            mDiagnostics->OnThreadTiming(notificationData);
        }
        break;

    //Called to notify timing breakdowns of various PlayReady license acquisition breakdowns
    case kReceiverNotificationType_DrmLicenseTimes:
        {
            mDiagnostics->OnDrmLicense(notificationData);

            TRACE(("DrmLicenseTimes[%08x]: %d -> %d (%d ms)", mPipeIdN,
                notificationData->DrmLicense.Timing[DrmLicenseTimes_Start],
                notificationData->DrmLicense.Timing[DrmLicenseTimes_End],
                notificationData->DrmLicense.Timing[DrmLicenseTimes_End] - notificationData->DrmLicense.Timing[DrmLicenseTimes_Start]));
        }
        break;

    //Process Splice-Later Signals...
    case kReceiverNotificationType_SpliceSignalLater:
        if (mSpliceEngine)
        {
            AutoLock lock(&Renderer_Lock);
            mSpliceEngine->HandleSpliceLaterSignal(notificationData->SpliceMessage);
        }
        break;

    //Process splice Splice-Heads-Up and Splice-Now Signals...
    case kReceiverNotificationType_SpliceSignal:
        if (mSpliceEngine)
        {
            AutoLock lock(&Renderer_Lock);
            mSpliceEngine->HandleSpliceSignal(notificationData->SpliceMessage);
        }
        break;

    //Process OTT signal (ad insertion in OTT content like SS)
    case kReceiverNotificationType_SpliceSignalOTT:
        if (mSpliceEngine)
        {
            AutoLock lock(&Renderer_Lock);
            mSpliceEngine->HandleOttSpliceSignal(notificationData->SpliceMessage);
        }
        break;

    //Smooth streaming manifest - no need to handle it here at this time.  ignore it...
    case kReceiverNotificationType_SmoothStreamingManifest:
        break;

    //We should always handle specific events directly in the switch statement above
    default:
        ASSERT(false);
        break;
    }
}

bool CReceiver::WritePacket(IPacket& packet)
{
    //Return immediately if we have been closed or signalled to stop
    if (Stopped)
        return true;

    //Drop the frame if we not interested in blackout packets
    if (BlackoutOnlyTune)
        return true;

    //Received new packet
    if (!mDiagnostics->OnReceivePacket(packet.DataLength))
        return true;

    //Process the incoming packet
    {
        Renderer_Lock.Lock();

        //Allocate a renderer
        if (mRenderer == NULL)
        {
            mRenderer = mRendererFactory->AcquireRenderer(packet, this);
            CHECK_ALLOC(mRenderer);
            mRenderer->Initialize(mTuneRequest);
        }

        //Keeping track of signal loss
        SignalLoss_Acquired();

        //Process the incoming packet
        if (!ClipPlay_Check(packet))
        {
            //Is it the first packet
            if (!FirstPacketReceived)
            {
                //Let the diagnostics track it
                mDiagnostics->OnFirstPacket();
                //Let the renderer handle the first packet as well
                mRenderer->OnFirstPacket(packet);
                //Send an event to media transport that the packets have started to stream
                NotifyStatus("status=streaming");
                //We now have the first packet
                FirstPacketReceived = true;
            }

            //Let the renderer pre-process the packet
            if (mRenderer->ReceivePacket(packet))
            {
                //Pass the packet to packet monitor, if any
                if (mPacketMonitor != NULL)
                {
                    mPacketMonitor->ProcessPacket(packet);
                }

                //Now pass it downstream
                while (!Stopped
                       && ((NULL == mSpliceEngine) || (!mSpliceEngine->ProcessPacket(packet)))
                       && (NULL != mRenderer)
                       && (!mRenderer->ProcessPacket(packet)))
                {
                    //Check for decoders being in bad state
                    if (CheckForDecoderStall())
                        break;

                    //Check for buffer overrun
                    //While in TrackDecoderStalls, a CReceiver::Reset could be called, which will set mRenderer to NULL
                    Renderer_Lock.Unlock();
                    if (TrackDecoderStalls())
                        return false;
                    Renderer_Lock.Lock();
                }
            }
        }
        Renderer_Lock.Unlock();
    }
    return true;
}

uint64 CReceiver::GetCurrentPlaybackTime(void)
{
    return mClock->GetCurrentMediaTime(false);
}

void CReceiver::GetDecoderBufferStatus( _Out_ DecoderBufferStatus* pDecoderBufferStatus )
{
    mClock->GetDecoderBufferStatus( pDecoderBufferStatus );
}

bool CReceiver::CanWaitForFragment()
{
    // Don't allow the downloader to delay the next fragment if
    // the decoder has stalled.

    return( !CheckForDecoderStall() );
}

// ===============================================================================================================
// ITunerSession APIs
// ===============================================================================================================

void CReceiver::PrepareToTune(void)
{
    CE_AV_TUNERSESSION_LOG(CE_AV_TUNERSESSION_TUNEPROCESS_START);
}

void CReceiver::TuneDone(void)
{
    CE_AV_TUNERSESSION_LOG(CE_AV_TUNERSESSION_TUNEPROCESS_DONE);
}

void CReceiver::Tune(const string& tunerurl, int action, const int* timings)
{
    AutoLock lock(&Receiver_Lock);
    CE_AV_TUNERSESSION_LOG(CE_AV_TUNERSESSION_TUNE);

    //Parse the tunerurl and start a tune
    mTunerUrl = tunerurl;
    //Save the url to be sent for node status
    mTunerUrlForNodeStatus = CTuneRequest::TunerStatusUrl(mTunerUrl);
    //Parse and build the tune request from incoming tuner url
    bool retval = mTuneRequest.ParseUrl(mTunerUrl);
    
    (void)retval; // prevent unused warning in release build
    ASSERT(retval);

    //Unique ids associated with this tune
    MediaTransportId = mTuneRequest.MediaTransportId;
    UniqueId = mTuneRequest.UniqueId;

    RECEIVER_MSG(("[%08x] TransportId[%08x] UniqueId[%08X] and Tune url %s", mPipeIdN, MediaTransportId, UniqueId, tunerurl.c_str()));

    //I am not sure why this needs to be done but leaving this as as for timeshift guys
    //to figure out...
    if (mTuneRequest.IsTimeShift())
        IsDetuned = true;

    //We are now tuned
    if (IsDetuned)
    {
        IsDetuned = false;

        //Expected bitrate of currently tuned channel
        BitRate = mTuneRequest.NetworkBandwidthUsage();
        //Current bitrate of currently tuned channel
        CurrentBitRate = 0;

        //Tracking of signal loss
        SignalLoss_Initialize();

        //Update diagnostics on tune
        mDiagnostics->OnTune(true, mTuneRequest, action, timings);
    }
    else
    {
        //Update diagnostics on tune
        mDiagnostics->OnTune(false, mTuneRequest, action, timings);
    }

    //Make sure clock is initialized for tune
    mClock->OnTune(mTuneRequest);

    //Do this only for VODs (RTP and SS) only
    ClipPlay_Enable();

    //Whether tuned for blackout only stream (background tunes by App when tuned to URl service)
    BlackoutOnlyTune = mTuneRequest.GetBool(TUNE_REQUEST_BLACKOUTONLY);

    //Register with the Splicing Engine when tuning to a given channel
    if (mSpliceEngine == NULL || !mSpliceEngine->Tune(mTuneRequest))
    {
        mSocket = mSocketFactory->AcquireSocket(mPipeIdN, mTuneRequest, this, CurrentTunerError, mMeasuredNetworkBitsPerSec);
    }

    // report the error if acquiring the socket fails
    if ( eTunerErrorNone != CurrentTunerError )
    {
        Error(CurrentTunerError, CurrentSocketError, CurrentSocketPKResult, CurrentSocketHttpResponse);
    }

    RECEIVER_MSG(("[%08x] TransportId is %d and Tune url %s ...done", mPipeIdN, MediaTransportId, tunerurl.c_str()));

    CE_AV_TUNERSESSION_LOG(CE_AV_TUNERSESSION_TUNE_DONE);
}

void CReceiver::TuneRefresh(const string& tunerurl)
{
    AutoLock lock(&Receiver_Lock);

    //Refresh the tunerurl - used for VOD when we switch to a new VOD segment
    if (mSocket != NULL)
    {
        mSocket->ConnectRefresh(tunerurl);
    }
}

void CReceiver::Detune(bool forceDetune, bool teardownPicture, bool channelChange, DetuneReason reason)
{
    //Post a tuner update event before quitting
    mDiagsManager->PostEvent(RetrieveTunerDiagnostics());
    //Post an renderer update event before quitting
    mDiagsManager->PostEvent(DiagsRetrieveInternal());

    //Signal the receiver to stop immediately first...
    //We do this outside the lock so that any possible
    //lock contentions could be done away with on the
    //playback receiver front
    SignalStop();
    //Make sure the tuner thread exits if it was waiting
    //while decoder buffers were full
    WaitWhileStalledEvent.Set();
    {
        AutoLock lock(&Receiver_Lock);
        CE_AV_TUNERSESSION_LOG(CE_AV_TUNERSESSION_DETUNE);

        RECEIVER_MSG(("[%08x] Detune", mPipeIdN));

        //Are we being detuned due to channel change
        IsDetuned = channelChange;

        //Detach ourselves from the current tuner
        if (mSocket != NULL)
        {
            mSocketFactory->DisposeSocket(mPipeIdN, mSocket, this, forceDetune, &mMeasuredNetworkBitsPerSec);
            mSocket = NULL;
        }

        //Revoke registration with splicing engine when detuning away from a channel
        if (mSpliceEngine) mSpliceEngine->Detune(channelChange);

        //Clear the event just in case
        WaitWhileStalledEvent.Reset();
        //Reinit the tune request
        mTuneRequest.Init();
        //Reset the renderer
        Reset(teardownPicture);

        // on a close, destroy all decoders
        if ( eDetuneClose == reason )
        {
            // after a close, forget about previous network bandwidth
            mMeasuredNetworkBitsPerSec = 0;
            TRACE(("start PurgeDecoders\n"));
            if ( mHalDecoderFactory->PurgeDecoders(mPipeIdN) )
            {
                TRACE(("Finish PurgeDecoders and Set Status\n"));
                SetStatus(kReceiverNotificationType_DecodersPurged, NULL); 
            }
            TRACE(("Finish PurgeDecoders\n"));
        }

        //Set state to detuned when errors happen otherwise we are starting playback
        SetStatus(kReceiverNotificationType_Stopped, NULL);

        //Detune the clock
        mClock->OnDetune();

        TRACE(("IsDetuned:%d\n", IsDetuned));
        //Clear the channel number if we are detuned away
        if (IsDetuned)
        {
            //Reset diagnostics on detune
            mDiagnostics->OnDetune();

            //Cear the tune url when channel changing
            mTunerUrl = "";
            //Initialize the tunerurl to be sent for node status
            mTunerUrlForNodeStatus = "";
            //Expected bitrate of currently tuned channel
            //Clean on channel change so that the node status will pick it up and release the stream
            BitRate = 0;
            //Current bitrate of currently tuned channel
            CurrentBitRate = 0;

            //Clear rendererstate on detune
            mRendererState->OnDetune();

            //Tracking of signal loss
            SignalLoss_Initialize();

            //Initialize clip play parameters
            ClipPlay_Initialize();

            //We will do this for fullscreen renderers only
            if (mRendererState->IsFeatureEnabled(CRendererState::eFeature_PlayreadyOPL))
            {
                // Reset the state of HDCP and Component output.
                mDecoderFactory->HandleHDCPOutput(mPipeIdN, XDRM_OPL_DISABLE);
                mDecoderFactory->HandleComponentVideoOutput(mPipeIdN, false);
            }
        }

        TRACE(("[%08x] Detune ...done", mPipeIdN));

        CE_AV_TUNERSESSION_LOG(CE_AV_TUNERSESSION_DETUNE_DONE);
    }
}

void CReceiver::SetBufferLength(uint64 startTime, uint64 endTime)
{
    AutoLock lock(&Receiver_Lock);
    mRendererState->StartTime = startTime;
    mRendererState->EndTime = endTime;
    if (mSpliceEngine)
    {
        AutoLock lock2(&Renderer_Lock);
        mSpliceEngine->SetBufferLength(startTime, endTime);
    }
}

void CReceiver::SetClipPlay(uint64 clipStartTime, bool clipStartTimeSet, uint64 clipEndTime, bool clipEndTimeSet, uint32 clipLimits)
{
    AutoLock lock(&Receiver_Lock);
    ClipPlay_Set(clipStartTime, clipStartTimeSet, clipEndTime, clipEndTimeSet, clipLimits);
}

bool CReceiver::PauseLive(const string& url)
{
    AutoLock lock(&Receiver_Lock);

    if (mRendererState->IsPlayingAd)
    {
        RECEIVER_MSG(("[%08x] PauseLive while splicing", mPipeIdN));

        //Pause the renderer
        {
            AutoLock lock(&Renderer_Lock);
            //Stop the clock immediately
            mClock->Pause(true);
        }

        //Save the tuner url if we are pausing live - this will allow us to switch back
        //to DVR file when the splicing ends and we switch back to primary
        mTunerUrl = url;
        //Save the url to be sent for node status
        mTunerUrlForNodeStatus = CTuneRequest::TunerStatusUrl(mTunerUrl);

        //Update current speed
        mTuneRequest.Speed = 0;

        //We are in pause state now
        SetStatus(kReceiverNotificationType_Paused, NULL);

        //Post a pause event
        mDiagnostics->OnPause(GetCurrentMediaTime(false));

        RECEIVER_MSG(("[%08x] PauseLive while splicing ...done", mPipeIdN));
    }
    else
    {
        RECEIVER_MSG(("[%08x] PauseLive(%s)", mPipeIdN, mTunerUrl.c_str()));

        //Transitioning from live to pause
        IsPauseLiveTransition = true;

        //Detach ourselves from the current tuner
        if (mSocket != NULL)
        {
            mSocketFactory->DisposeSocket(mPipeIdN, mSocket, this, false, &mMeasuredNetworkBitsPerSec);
            mSocket = NULL;
        }

        //And pause the renderer
        {
            AutoLock lock(&Renderer_Lock);
            mClock->Pause(true);
        }

        //Get position of the stream based on the file position that we
        //had been tracking.This represents a seek point for packet accurate
        //seeks when resuming from live pause.
        mTunerUrl = url;
        mTunerUrl += (url.find("?") == string::npos ? "?" : "&") + mRenderer->PauseLiveParameters();
        //Save the url to be sent for node status
        mTunerUrlForNodeStatus = CTuneRequest::TunerStatusUrl(mTunerUrl);

        //Parse and build the tune request from incoming tuner url
        bool retval = mTuneRequest.ParseUrl(mTunerUrl);

        (void)retval; // prevent unused warning in release build
        ASSERT(retval);
        //Update current speed
        mTuneRequest.Speed = 0;

        //Acquire the new socket
        mSocket = mSocketFactory->AcquireSocket(mPipeIdN, mTuneRequest, this, CurrentTunerError, mMeasuredNetworkBitsPerSec);

        //We are now in paused from live state
        SetStatus(mSocket == NULL ? kReceiverNotificationType_Detuned : kReceiverNotificationType_Paused, NULL);

        //Post a pause event
        mDiagnostics->OnPause(GetCurrentMediaTime(false));

        //Transitioning from live to pause is now done
        IsPauseLiveTransition = false;

        RECEIVER_MSG(("[%08x] PauseLive(%s) ...done", mPipeIdN, mTunerUrl.c_str()));
    }
    return true;
}

bool CReceiver::Pause(void)
{
    AutoLock lock(&Receiver_Lock);

    RECEIVER_MSG(("[%08x] Pause", mPipeIdN));

    bool retval = mSocket ? mSocket->Pause() : true;
    if (retval)
    {
        RECEIVER_MSG(("[%08x] Pausing", mPipeIdN));

        //Pause the renderer
        {
            AutoLock lock(&Renderer_Lock);
            mClock->Pause(true);
        }

        //Update current speed
        mTuneRequest.Speed = 0;

        //We are in pause state now
        SetStatus(kReceiverNotificationType_Paused, NULL);

        //Post a pause event
        mDiagnostics->OnPause(GetCurrentMediaTime(false));
    }

    RECEIVER_MSG(("[%08x] Pause ...done", mPipeIdN));
    return retval;
}

bool CReceiver::Play(void)
{
    AutoLock lock(&Receiver_Lock);

    RECEIVER_MSG(("[%08x] Play", mPipeIdN));

    bool retval = mSocket ? mSocket->Play() : true;
    if (retval)
    {
        AutoLock lock(&Renderer_Lock);
        if (mRendererState->IsImmediateMode)
        {
            RECEIVER_MSG(("[%08x] Playing from Tricks", mPipeIdN));
        }
        else
        {
            RECEIVER_MSG(("[%08x] Playing", mPipeIdN));

            //Start the clock right away
            mClock->Pause(false);
            //We are coming out of pause state now
            SetStatus(kReceiverNotificationType_DataReady, NULL);
        }
        //Update current speed
        mTuneRequest.Speed = 1;
        //Post a play event
        mDiagnostics->OnPlay();
    }

    RECEIVER_MSG(("[%08x] Play ...done", mPipeIdN));
    return retval;
}

void CReceiver::SetAccessControl(bool blocked, uint64 startTime, uint64 endTime)
{
    AutoLock lock(&Renderer_Lock);

    mRendererState->OnSetAccessControl(blocked, startTime, endTime);
}

void CReceiver::SetSecondaryAudioPreference(bool usesap)
{
    AutoLock lock(&Renderer_Lock);

    //Save the current SAP selection
    mRendererState->UseSap = usesap;
    if (mRenderer)
    {
        mRenderer->SetSecondaryAudioPreference();
    }
}

void CReceiver::SetAudioLanguages(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type)
{
    AutoLock lock(&Receiver_Lock);

    int usedPid;
    {
        AutoLock lock2(&Renderer_Lock);
        //Set the preferred list of audio language descriptors
        mRendererState->AudioLanguage.SetPreferred(iso639LanguageCodes, iso639LanguageCodesNum, type);
        //Find the audio language stream to be rendererd
        mRendererState->AudioLanguage.Find(*mRendererState);
        //Now update the state and get currently seelcted audio language stream
        usedPid = mRendererState->UpdateAudioLanguage(!IsDetuned);
    }

    //Update audio language if selection controlled by socket
    if (mSocket)
    {
        mSocket->SetAudioLanguage(usedPid);
    }

    //Make sure to update the language in case streams were being delivered
    //over MF-RTP since the audio selection may have changed.  This call will
    //be a noop for all other types of stream encapsulations.
    SetSecondaryAudioPreference(mRendererState->UseSap);
}

void CReceiver::SetAudioLanguage(int iso639LanguageCode, int type, int pid)
{
    AutoLock lock(&Receiver_Lock);

    int usedPid;
    {
        AutoLock lock2(&Renderer_Lock);
        //Set preferred audio language
        mRendererState->AudioLanguage.SetExpected(iso639LanguageCode, type, pid);
        //Find the audio language stream to be rendererd
        mRendererState->AudioLanguage.Find(*mRendererState);
        //Now update the state and get currently seelcted audio language stream
        usedPid = mRendererState->UpdateAudioLanguage(!IsDetuned);
    }

    //Update audio language if selection controlled by socket
    if (mSocket)
    {
        mSocket->SetAudioLanguage(usedPid);
    }

    //Make sure to update the language in case streams were being delivered
    //over MF-RTP since the audio selection may have changed.  This call will
    //be a noop for all other types of stream encapsulations.
    SetSecondaryAudioPreference(mRendererState->UseSap);
}

void CReceiver::SetAudioDescriptionLanguages(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type)
{
    AutoLock lock(&Renderer_Lock);

    //Set the preferred list of audio description language descriptors
    mRendererState->AudioDescriptionLanguage.SetPreferred(iso639LanguageCodes, iso639LanguageCodesNum, type);
    //Find the audio description language stream to be rendered
    mRendererState->AudioDescriptionLanguage.Find(*mRendererState);
    //Now update the state
    mRendererState->UpdateAudioDescriptionLanguage(!IsDetuned);
}

void CReceiver::SetAudioDescriptionLanguage(int iso639LanguageCode, int type, int pid)
{
    AutoLock lock(&Renderer_Lock);

    //Set preferred audio description language
    mRendererState->AudioDescriptionLanguage.SetExpected(iso639LanguageCode, type, pid);
    //Find the audio description language stream to be rendered
    mRendererState->AudioDescriptionLanguage.Find(*mRendererState);
    //Now update the state
    mRendererState->UpdateAudioDescriptionLanguage(!IsDetuned);
}

void CReceiver::SetAudioDescriptionVolume(int volume)
{
    AutoLock lock(&Renderer_Lock);
    //Save the volume level for audio description stream
    mRendererState->OnSetAudioDescriptionVolume(volume);
}

void CReceiver::SetSubtitleLanguages(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type)
{
    AutoLock lock(&Receiver_Lock);

    int usedPid;
    {
        AutoLock lock2(&Renderer_Lock);
        //Set the preferred list of subtitle language descriptors
        mRendererState->SubtitleLanguage.SetPreferred(iso639LanguageCodes, iso639LanguageCodesNum, type);
        //Find subtitle language stream to be rendered
        mRendererState->SubtitleLanguage.Find(*mRendererState);
        //And process the subtitle language change downstream
        if (mRenderer)
        {
            mRenderer->UpdateSubtitleLanguage();
        }
        //Now update the state and get currently selected subtitle language stream
        usedPid = mRendererState->UpdateSubtitleLanguage(!IsDetuned);
    }

    //Update subtitle language if selection controlled by socket
    if (mSocket)
    {
        mSocket->SetSubtitleLanguage(usedPid);
    }
}

void CReceiver::SetSubtitleLanguage(int iso639LanguageCode, int type, int pid)
{
    AutoLock lock(&Receiver_Lock);

    int usedPid;
    {
        AutoLock lock2(&Renderer_Lock);
        //Set preferred subtitle language
        mRendererState->SubtitleLanguage.SetExpected(iso639LanguageCode, type, pid);
        //Find subtitle language stream to be rendered
        mRendererState->SubtitleLanguage.Find(*mRendererState);
        //And process the subtitle language change downstream
        if (mRenderer)
        {
            mRenderer->UpdateSubtitleLanguage();
        }
        //Now update the state and get currently selected subtitle language stream
        usedPid = mRendererState->UpdateSubtitleLanguage(!IsDetuned);
    }

    //Update subtitle language if selection controlled by socket
    if (mSocket)
    {
        mSocket->SetSubtitleLanguage(usedPid);
    }
}

void CReceiver::SetCCChannel(int cc608, int cc708)
{
    AutoLock lock(&Renderer_Lock);
    mRendererState->SetCCChannel(cc608, cc708);
}

uint64 CReceiver::GetCurrentMediaTime(bool smooth)
{
    return mClock->GetCurrentMediaTime(smooth);
}

bool CReceiver::Command(const string& command, const vector<string>& args)
{
    //Next, let diagnostics handle it
    {
        if (mDiagnostics->Command(command, args))
            return true;
    }
    //Next, let the renderer handle the command
    {
        AutoLock lock(&Renderer_Lock);
        if (mRenderer && mRenderer->Command(command, args))
            return true;
    }
    //Finally, pass it on to sockets
    {
        AutoLock lock(&Receiver_Lock);
        if (mSocket && mSocket->Command(command, args))
            return true;
    }
    //Command not processed
    return false;
}

bool CReceiver::IsStreamActive(void)
{
    return mDiagnostics->IsStreamActive();
}

IDiagsEvent* CReceiver::RetrieveDiagnostics(void)
{
    return DiagsRetrieve();
}

IDiagsEvent* CReceiver::RetrieveTunerDiagnostics(void)
{
    AutoLock lock(&Receiver_Lock);
    return mSocket != NULL ? mSocket->DiagsRetrieve() : NULL;
}

ITunerSessionCallback* CReceiver::GetStatusCallback(void)
{
    return mStatusCallback;
}

// ===============================================================================================================
// ITimeslice APIs
// ===============================================================================================================

void CReceiver::OnTimeslice(void)
{
    //Handle splicing state machine
    if (mSpliceEngine)
    {
        AutoLock lock(&Receiver_Lock);
        mSpliceEngine->OnTimeslice();
    }

    //Track loss of signal
    SignalLoss_Tracking();

    //No need to process timeslices when already asked to stop
    if (IsRunning())
    {
        AutoLock lock(&Renderer_Lock);

        //Let every registered timeslicer have their day
        for (list<ITimeslice*>::iterator it = TimesliceList.begin(); IsRunning() && it != TimesliceList.end(); ++it)
        {
            (*it)->OnTimeslice();
        }

        //Give a timeslice to renderer state to update itself
        mRendererState->OnTimeslice();

        //Has the streaming from socket stopped
        if (NearEnd)
        {
            //Now check if the stream rendered all the way to the end
            if (mClock->IsRenderingDone())
            {
                NearEnd = false;

                // when rendering is done, signal detuned if error is set
                if(eTunerErrorNone != CurrentTunerError && eTunerErrorEOF != CurrentTunerError)
                {
                    SetStatus(kReceiverNotificationType_Detuned, NULL);
                }
                else
                {
                    SetStatus(kReceiverNotificationType_EndOfStream, NearEndNotificationDataSet ? &NearEndNotificationData : NULL);
                }
                mClock->Stop();
            }
        }
        else
        {
            //Let the clock do some work too
            mClock->OnTimeslice();
        }
        //send heart beat here only when normal speed
        if (!mRendererState->IsImmediateMode)
        {
            HeartBeatEvent(); 
        }
    }
}

#define MIN_HEART_GET_PTS_INTERVAL 100  //ms

void CReceiver::HeartBeatEvent(uint64 curPts)
{
    static TimeSpan_NTP lastNtpTime;
    static uint64 lastTime;
    static int g_last_heart_time = 0;
    //IPTV_HAL_DECODER_VALUE_AUDIO_CURRENTPTS currpts;
    //uint32 size = sizeof(currpts);
    struct timeval tvNow;

    //to avoid get time frequently
    gettimeofday(&tvNow, NULL);
    int cur_time = tvNow.tv_sec*1000 + (uint32_t)tvNow.tv_usec/1000;
    if((cur_time - g_last_heart_time) > MIN_HEART_GET_PTS_INTERVAL)
    {
        g_last_heart_time = cur_time;
        //printf("HeartBeatEvent IsImmediateMode:%d\n",mRendererState.IsImmediateMode);
        if (curPts){
            //printf("HeartBeatEvent curPts:%lld\n",curPts);
            TimeSpan_NTP ntpTime = TimeSpan_NTP::ConvertFrom(TimeSpan_PTS::FromTicks(curPts));
            if (lastNtpTime != ntpTime)
            {
                string eventString = "status=heartbeat&hearttime="+ toString64(ntpTime.Ticks(), true);
                NotifyStatus(eventString);
                lastNtpTime = ntpTime;
            }
        }else{

            uint64 time = GetCurrentPlaybackTime();
            if (lastTime != time)
            {
                string eventString = "status=heartbeat&hearttime="+ toString64(time, true);
                NotifyStatus(eventString);
                lastTime = time;
            }
        }
    }
}

// ===============================================================================================================
// ISpliceControl APIs
// ===============================================================================================================

void CReceiver::StopForSplice(void)
{
    //Stop receiver for a stream splice
    SignalStop();
}

bool CReceiver::PreflightNextAd(void)
{
    if (mSpliceEngine)
    {
        AutoLock lock(&Renderer_Lock);
        //Preflight the tune to secondary stream
        //Returns true if preflighting next secondary stream was successfull
        return mSpliceEngine->PreflightNextAd();
    }
    return false;
}

void CReceiver::DetuneFromPrimary(void)
{
    //Detune from the primary stream socket
    if (mSocket)
    {
        mSocketFactory->DisposeSocket(mPipeIdN, mSocket, this, false, &mMeasuredNetworkBitsPerSec);
        mSocket = NULL;
    }
}

bool CReceiver::TuneToSecondary(string& secondaryUrl)
{
    //Update the tune request to the secondary stream
    if (!mTuneRequest.ParseUrl(secondaryUrl))
    {
        TRACE_ERROR(("[%08x] Failed to parse secondary url %s", mPipeIdN, secondaryUrl.c_str()));
        return false;
    }

    //Instantiates a secondary socket based on the secondary tune request
    //Error while creating secondary url
    eTunerError tunerError = eTunerErrorNone;
    mSocket = mSocketFactory->AcquireSocket(mPipeIdN, mTuneRequest, tunerError);
    if (tunerError != eTunerErrorNone)
    {
        TRACE_ERROR(("[%08x] Secondary stream socket creation failed [%d]", mPipeIdN, tunerError));
        return false;
    }
    return true;
}

void CReceiver::JoinSecondary(void)
{
    //Attach the receiver to the secondary stream socket
    //This will start the data flowing through the system
    mSocket->AddReceiver(this);
}

void CReceiver::DetuneFromSecondary(void)
{
    //Dump secondary stream socket
    if (mSocket)
    {
        //Detach receiver from socket
        mSocket->RemoveReceiver(this, true);
        //And stop the receiver
        Stop(true);
        //Detach receiver from the current socket
        mSocketFactory->DisposeSocket(mPipeIdN, mSocket);
        mSocket = NULL;
    }
}

bool CReceiver::TuneBackToPrimary(string& urlParameters)
{
    //Update the tune request to the secondary stream with additional parameters supplied
    string tunerurl = mTunerUrl + urlParameters;
    mTuneRequest.ParseUrl(tunerurl);

    //Tune back to primary stream
    mSocket = mSocketFactory->AcquireSocket(mPipeIdN, mTuneRequest, this, CurrentTunerError, mMeasuredNetworkBitsPerSec);
    if (CurrentTunerError != eTunerErrorNone)
    {
        TRACE_ERROR(("[%08x] Primary socket creation failed [%d]", mPipeIdN, CurrentTunerError));
        return false;
    }
    return true;
}

void CReceiver::ResetForSplice(void)
{
    //Dump the current steeam renderer
    AutoLock lock(&Renderer_Lock);

    //Get the receiver into a ready state
    TRACE(("[%08x] Splice: Resetting renderer", mPipeIdN));

    //Do not flush HAL decoders when they are released
    //We need the HAL decoders to continue to play currently
    //buffer data while we are transitioning between ad socket
    //and primary stream socket.
    mRendererState->FlushVideoDecoderOnRelease = false;
    mRendererState->FlushAudioDecoderOnRelease = false;

    //And release the downstream streamer handlers
    if (mRenderer)
    {
        delete mRenderer;
        mRenderer = NULL;
    }
    //Clear stream info on reset
    mRendererState->StreamInfo.Clear();

    //Restore the flush decoders on release flag
    mRendererState->FlushVideoDecoderOnRelease = true;
    mRendererState->FlushAudioDecoderOnRelease = true;

    //Note that we are not stopping the clock while we are doing the transition...
    //Let the clock know that it needs to do PTS offset recalculation on next frame
    mClock->RecalculatePtsOffset(true);

    TRACE(("[%08x] Splice: Renderer reset", mPipeIdN));
}

// ===============================================================================================================
// IDiagsProvider APIs
// ===============================================================================================================

void CReceiver::DiagsReset(void)
{
    //Do the base class first
    IDiagsProvider::DiagsReset();
    //Reset diagnostics tracker
    mDiagnostics->OnReset();
}

IDiagsEvent* CReceiver::DiagsRetrieve(void)
{
    //Return if we have reached the end of playback
    return Stopped && !NearEnd ? NULL : DiagsRetrieveInternal();
}

// ===============================================================================================================
// Internal APIs
// ===============================================================================================================

IDiagsEvent* CReceiver::DiagsRetrieveInternal(void)
{
    //Do not send an event if detuned already
    AutoLock lock(&Renderer_Lock);
    return IsDetuned ? NULL : mDiagnostics->OnRetrieve();
}

void CReceiver::SetStatus(ReceiverNotificationType newStatus, CReceiverNotificationData* notificationData)
{
    //Ignore events without a change in status, unless it contains receiver parameters which we want to pass on
    if ((mStatus == newStatus) && (notificationData == NULL))
        return;

    //Save the new status
    mStatus = newStatus;

    //Managed AV doesn't care about this state.  On the other hand
    //we just need to make sure that the internal receiver state transitions
    //are appropriately maintained.
    if (mStatus == kReceiverNotificationType_Stopped)
        return;

    //Prepare the status that needs to be sent to managed AV
    string status;
    switch (mStatus)
    {
        case kReceiverNotificationType_Paused:          status = "paused";      break;
        case kReceiverNotificationType_DataReady:       status = "playing";     break;
        case kReceiverNotificationType_Detuned:         status = "detuned";     break;
        case kReceiverNotificationType_EndOfStream:     status = "mediaended";  break;
        case kReceiverNotificationType_DecodersPurged:  status = "closed";      break;
        default:
            ASSERT(false); // no others should be possible
            return;
    }

    RECEIVER_MSG(("[%08x] SetStatus(), status is:%s", mPipeIdN, status.c_str()));

    //Fill out status parameters if provided
    if (notificationData)
    {
        status +=
            "&starttime=" + toString64(notificationData->PlayParams.StartTime) +
            "&endtime=" + toString64(notificationData->PlayParams.EndTime) +
            "&speed=" + toString(notificationData->PlayParams.Speed) +
            "&playlistindex=" + toString(notificationData->PlayParams.PlayListIndex) +
            "&playlistcount=" + toString(notificationData->PlayParams.PlayListCount);
    }
    else if (mStatus == kReceiverNotificationType_EndOfStream)
    {
        //Always send the direction of playback
        status += "&speed=" + toString(mTuneRequest.Speed);
    }

    //Fill out errors if there is any
    if (mStatus == kReceiverNotificationType_Detuned)
    {
        status += "&tunererror=" + toString(CurrentTunerError) +
                  "&socketerror=" + toString(CurrentSocketError) +
                  "&pkresult=" + toString(CurrentSocketPKResult) +
                  "&httpresponse=" + toString(CurrentSocketHttpResponse);
    }

    //Send it to MediaTransport in managed
    NotifyStatus("status=tuner&state=" + status);
}

void CReceiver::SignalNearEnd(CReceiverNotificationData* notificationData)
{
    NearEnd = true;

    if (notificationData)
    {
        memcpy_s(&NearEndNotificationData, sizeof(CReceiverNotificationData), notificationData, sizeof(CReceiverNotificationData));
        NearEndNotificationDataSet = true;
    }
    else
    {
        memset(&NearEndNotificationData, 0, sizeof(CReceiverNotificationData));
        NearEndNotificationData.EndOfStream.Result = true;
        NearEndNotificationData.EndOfStream.Direction = true;
        NearEndNotificationDataSet = false;
    }

    //Send an event to the trace logs that we have have completed streaming
    mDiagnostics->OnNearEnd();

    mClock->SetEndOfStream();
}

bool CReceiver::CheckForDecoderStall(void)
{
    //Is the clock stopped for any reasons?
    if (mClock->IsStopped())
        return false;

    //Check for stall once a second
    if (mRendererState->HaveDecodersStalled())
    {
        Sync(kSyncReason_DecoderStall, true, true);
        return true;
    }
    return false;
}

bool CReceiver::TrackDecoderStalls(void)
{
    //Have we been asked to stop?
    if (Stopped)
        return false;

    //Do the following in renderer lock but not the wait below
    {
        AutoLock lock(&Renderer_Lock);

        //Check if buffer delay is set too high to fit in video decoder buffer
        if (mClock->ForceBufferDelayStart())
            return false;

        //Check if we can retry the packet
        if (!mRetry)
        {
            if (mResetOnStall)
            {
                //Rebuffer when decoder stalls during live
                TRACE_ERROR(("DECODER STALLING[%08x] While Watching Live - rebuffering", mPipeIdN));
                Sync(kSyncReason_BufferOverrun, true, true);
            }
            return true;
        }
    }

    //Decoder is full, wait a while and retry packet downstream
    WaitWhileStalledEvent.Wait(100);
    //Clear the event just in case
    WaitWhileStalledEvent.Reset();
    return false;
}

// ===============================================================================================================
// Tracking signal loss
// ===============================================================================================================

void CReceiver::SignalLoss_Initialize(void)
{
    //Tracking of signal loos
    SignalReceivedAtTick =
    SignalCheckedAtTick = Executive_GetTickCount();
    SignalLost = false;
    SignalLossCheckEnabled = false;
}

void CReceiver::SignalLoss_Enable(void)
{
    //Whether signal loss check is enabled
    SignalLossCheckEnabled = mTuneRequest.IsLive() || mTuneRequest.IsDvb() || mTuneRequest.IsMpegTs();
}

void CReceiver::SignalLoss_Acquired(void)
{
    //Keep a snapshot of when the last packet was received
    SignalReceivedAtTick = Executive_GetTickCount();

    //And check if the signal is just being reacquired
    if (SignalLost)
    {
        TRACE(("REACQUIRED SIGNAL[%08x]...", mPipeIdN));

        //We have reacquired the signal
        SignalLost = false;
        //And send an event to media transport
        NotifyStatus("status=signal&state=acquired");
        //Send an event to the trace logs that we have re-acquired the signal
        mDiagnostics->OnSignalStatusEvent(SignalLost);
    }
}

void CReceiver::SignalLoss_Tracking(void)
{
    //Quit imemdiately if no need to check the loss of signal
    if (gReceiverConfiguration.SignalLostTime)
    {
        //Check every second
        uint32 ticks = Executive_GetTickCount();
        uint32 delta = ticks - SignalCheckedAtTick;
        if (delta >= 1000)
        {
            //No need to check if socket has stopped
            if (IsStreaming())
            {
                AutoLock lock(&Renderer_Lock);
                if (!SignalLost && SignalLossCheckEnabled)
                {
                    //Did we loose the signal
                    delta = ticks - SignalReceivedAtTick;
                    if (delta >= gReceiverConfiguration.SignalLostTime)
                    {
                        TRACE(("LOST SIGNAL[%08x]...", mPipeIdN));

                        //The signal has been lost
                        SignalLost = true;
                        //Teardown the picture from screen
                        mHalDecoderFactory->TeardownPicture(mPipeIdN);
                        //And send an event to media transport
                        NotifyStatus("status=signal&state=lost");
                        //Send an event to the trace logs that we have lost the signal
                        mDiagnostics->OnSignalStatusEvent(SignalLost);
                    }
                }
            }
            SignalCheckedAtTick = ticks;
        }
    }
}

// ===============================================================================================================
// To check for clip play limits
//
// Note that the NTP/NPT times may not be aligned appropriately between the
// primary stream and secondary stream with respect to the clip positions
// and there is a chance that frams could leak beyond the clip point when
// in trick mode.  More work would be needed in how the VODs are injested
// to make them work correctly with mid-rolls especially with encoded trick
// streams.  No-encode streams are expected to work much better here...
//
// ===============================================================================================================

void CReceiver::ClipPlay_Initialize(void)
{
    ClipPlay_Enabled = false;
    ClipPlay_StartTime = 0;
    ClipPlay_StartTimeSet = false;
    ClipPlay_EndTime = 0;
    ClipPlay_EndTimeSet = false;
    ClipPlay_Limits = ClipPlayLimits_All;
}

void CReceiver::ClipPlay_Set(uint64 clipStartTime, bool clipStartTimeSet, uint64 clipEndTime, bool clipEndTimeSet, uint32 clipLimits)
{
    ClipPlay_StartTime = clipStartTime;
    ClipPlay_StartTimeSet = clipStartTimeSet;
    ClipPlay_EndTime = clipEndTime;
    ClipPlay_EndTimeSet = clipEndTimeSet;
    ClipPlay_Limits = (ClipPlayLimits)clipLimits;
}

void CReceiver::ClipPlay_Enable(void)
{
    //Do this only for VODs (RTP and SS) only
    ClipPlay_Enabled = (mTuneRequest.IsVod() || mTuneRequest.IsMbr()) && (ClipPlay_StartTimeSet || ClipPlay_EndTimeSet);
}

bool CReceiver::ClipPlay_Check(IPacket& packet)
{
    //Whether clip play is enabled for this stream
    if (!ClipPlay_Enabled)
        return false;

    uint64 packetTime = packet.NTP;

    //Skip non-rap packets and packets with no NPT times in them
    //Note that the clipping is currently enabled only for VOD at this time in this path
    //so it is safe to just use the NPT time for checking the clip play timings
    if (!packet.Rap || IS_INVALID_TIME(packetTime))
        return false;

    bool clipAll = ClipPlay_Limits == ClipPlayLimits_All;
    bool clipPlay = (ClipPlay_Limits & ClipPlayLimits_Play) == ClipPlayLimits_Play;
    bool clipFwd = (ClipPlay_Limits & ClipPlayLimits_Forward) == ClipPlayLimits_Forward;
    bool clipRwd = (ClipPlay_Limits & ClipPlayLimits_Rewind) == ClipPlayLimits_Rewind;
    bool clipPause = (ClipPlay_Limits & ClipPlayLimits_Pause) == ClipPlayLimits_Pause;

    if (clipAll
         || (clipPause && mTuneRequest.Speed == 0)
         || (clipPlay && mTuneRequest.Speed == 1)
         || (clipFwd && mTuneRequest.Speed > 1)
         || (clipRwd && mTuneRequest.Speed < 0))
    {
        if (ClipPlay_StartTimeSet && packetTime < ClipPlay_StartTime)
        {
            TRACE(("ClipPlay[%08x] Reached start of clip", mPipeIdN));

            //Stop doing any work now
            SignalStop();
            //Set the flag for rendering completion check
            SignalNearEnd(NULL);
            return true;
        }
        else if (ClipPlay_EndTimeSet && packetTime >= ClipPlay_EndTime)
        {
            TRACE(("ClipPlay[%08x] Reached end of clip", mPipeIdN));

            //Stop doing any work now
            SignalStop();
            //Set the flag for rendering completion check
            SignalNearEnd(NULL);
            return true;
        }
    }
    return false;
}

// ===============================================================================================================
// ===============================================================================================================
