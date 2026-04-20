///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CReceiverCommon.h"
#include "IReceiverControl.h"
#include "IReceiver.h"
#include "ISpliceEngine.h"
#include "ITunerSession.h"
#include "ITimeslice.h"
#include "ISpliceControl.h"
#include "IDiagsProvider.h"
#include "CEvent.h"
#include "CRendererState.h"
#include "CReceiverDiagnostics.h"
#include "CTuneRequest.h"
#include "Clock.h"
#include <string>
#include <vector>
#include <list>

// ===============================================================================================================
// ***** LAI IMPORTANT ***** LAI IMPORTANT ***** LAI IMPORTANT ***** LAI IMPORTANT ***** LAI IMPORTANT *****
//
// LAI and SPLICE ENGINE support could be added by globally defining "TV2_LAI_SUPPORT"
// through "client\VsProps\tv2_common_all.vsprops" file or other means...
//
// ***** LAI IMPORTANT ***** LAI IMPORTANT ***** LAI IMPORTANT ***** LAI IMPORTANT ***** LAI IMPORTANT *****
// ===============================================================================================================

// ===============================================================================================================
// CReceiver class
// - holds all stream control properties
// - provides functionality for
//     - Controlling streams (APIs exposed to managed AV)
//     - Node Status generation
//     - Consuming streams from sockets
//     - Periodic diagnostics event
//     - Timeslicing for all objects connected to this pipe, on a request basis
//
// A Note on Clocks
// Clocks are passed into decoders
// If we Release a clock all associate decoders using that clock should be released
// ===============================================================================================================

class ISocket;
class IRenderer;
class IDecoder;
class IPacket;

class CReceiver
    : public IReceiverControl
    , public IReceiver
    , public ITunerSession
    , public ITimeslice
    , public ISpliceControl
    , public IDiagsProvider
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    CReceiver(IAVManager* avManager, ITunerSessionCallback* callback, const std::string& pipeId, uint32 featuresEnabled);
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~CReceiver();

private:
    //Acquire a clock to be used for rendering
    void                               AcquireClock(void);
    //Release the clock used by this renderer
    void                               ReleaseClock(void);
    //Reset the renderer and downstream components
    void                               Reset(bool teardownPicture);

public:
    /// <summary>
    /// Implements <see cref="IReceiverControl.GetAVManager">CReceiver::GetAVManager</see>
    /// Returns the factory of factory
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override IAVManager*             GetAVManager(void) { return mAVManager; }
    /// <summary>
    /// Implements <see cref="IReceiverControl.GetRendererState">CReceiver::GetRendererState</see>
    /// Returns the persistent renderer state used by downstream components
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override CRendererState&         GetRendererState(void) { return *mRendererState; }
    /// <summary>
    /// Implements <see cref="IReceiverControl.GetDiagnostics">CReceiver::GetDiagnostics</see>
    /// Returns the diagnostics being used
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override CReceiverDiagnostics&   GetDiagnostics(void) { return *mDiagnostics; }
    /// <summary>
    /// Implements <see cref="IReceiverControl.GetClock">CReceiver::GetClock</see>
    /// Returns the clock being used
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override Clock&                 GetClock(void) { return *mClock; }
    /// <summary>
    /// Implements <see cref="IReceiverControl.GetSpliceControl">CReceiver::GetSpliceControl</see>
    /// Returns the splice control API
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override ISpliceControl*         GetSpliceControl(void) { return this; }
    /// <summary>
    /// Implements <see cref="IReceiverControl.SetPacketMonitor">CReceiver::SetPacketMonitor</see>
    /// Set a packet monitor to track packets - only one packet monitor supported
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override void                    SetPacketMonitor(IPacketMonitor* packetMonitor) {  mPacketMonitor = packetMonitor; }
    /// <summary>
    /// Implements <see cref="IReceiverControl.IsStreaming">CReceiver::IsStreaming</see>
    /// Whether still streaming from socket
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override bool                    IsStreaming(void) const { return mRendererState->IsStoppedForAd || !Stopped; }
    /// <summary>
    /// Implements <see cref="IReceiverControl.IsRunning">CReceiver::IsRunning</see>
    /// Whether the receiver has been stopped
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override bool                    IsRunning(void) const { return mRendererState->IsStoppedForAd || !Stopped || NearEnd; }
    /// <summary>
    /// Implements <see cref="IReceiverControl.IsTrickMode">CReceiver::IsTrickMode</see>
    /// Whether we're in a VOD/DVR trick mode
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override bool                    IsTrickMode(void) const { return mRendererState->IsImmediateMode || mRendererState->IsIFrameOnlyMode || mTuneRequest.Speed != 1; }
    /// <summary>
    /// Implements <see cref="IReceiverControl.Sync">CReceiver::Sync</see>
    /// Synchronize to next RAP point
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override void                    Sync(SyncReason reason, bool sync, bool cleanStall);
    /// <summary>
    /// Implements <see cref="IReceiverControl.OnRebuffer">CReceiver::OnRebuffer</see>
    /// Called when clock is rebuffering or sync'ing
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override void                    OnRebuffer(SyncReason reason, bool sync, bool cleanStall);
    /// <summary>
    /// Implements <see cref="IReceiverControl.NotifyStatus">CReceiver::NotifyStatus</see>
    /// Send events to managed side
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override void                    NotifyStatus(const std::string& status);
    /// <summary>
    /// Implements <see cref="IReceiverControl.RegisterForTimeslice">CReceiver::RegisterForTimeslice</see>
    /// Registering internal components for timeslice
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override void                    RegisterForTimeslice(ITimeslice* timeslice);
    /// <summary>
    /// Implements <see cref="IReceiverControl.UnRegisterForTimeslice">CReceiver::UnRegisterForTimeslice</see>
    /// Unregistering internal components for timeslice
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override void                    UnRegisterForTimeslice(ITimeslice* timeslice);
    /// <summary>
    /// Implements <see cref="IReceiverControl.NewFrameArrived">CReceiver::NewFrameArrived</see>
    /// Called to update the splice-in and splice-out timings
    ///
    /// This is a temporary API for measuring the length of ad streams since they
    /// are not reliable.  The length of the ad stream is determined based on
    /// the original PTS of these stream as they are being rendered.  Which means
    /// RTP Ad streams need to be played back once in normal playback mode to allow
    /// for seamless splicing while tricking.
    /// </summary>
    /// <seealso cref="IReceiverControl">IReceiverControl</seealso>
    __override void                    NewFrameArrived(uint64 originalPts) { if (mSpliceEngine) mSpliceEngine->SpliceOnGoing(originalPts); }

public:
    /// <summary>
    /// Implements <see cref="IReceiver.AttachSocketToDvrReceiver">CReceiver::AttachSocketToDvrReceiver</see>
    /// </summary>
    /// <seealso cref="IReceiver">IReceiver</seealso>
    __override bool                    AttachSocketToDvrReceiver(const std::string& path, ISocket* socket) { return true; };
    /// <summary>
    /// Implements <see cref="IReceiver.DetachSocketFromDvrReceiver">CReceiver::DetachSocketFromDvrReceiver</see>
    /// </summary>
    /// <seealso cref="IReceiver">IReceiver</seealso>
    __override bool                    DetachSocketFromDvrReceiver(const std::string& path, ISocket* socket) { return true; };
    /// <summary>
    /// Implements <see cref="IReceiver.Start">CReceiver::Start</see>
    /// </summary>
    /// <param name="void"></param>
    /// <seealso cref="IReceiver">IReceiver</seealso>
    __override void                    Start(void);
    /// <summary>
    /// Implements <see cref="IReceiver.SignalStop">CReceiver::SignalStop</see>
    /// </summary>
    /// <param name="void"></param>
    /// <seealso cref="IReceiver">IReceiver</seealso>
    __override void                    SignalStop(void);
    /// <summary>
    /// Implements <see cref="IReceiver.Stop">CReceiver::Stop</see>
    /// </summary>
    /// <param name="bForced">[IN] true to disconnect the tuner when stopping the receiver</param>
    /// <param name="tunerError">[IN] optional details of reason for the stop</param>
    /// <seealso cref="IReceiver">IReceiver</seealso>
    __override void                    Stop(_In_ bool bForced, _In_opt_ eTunerError tunerError = eTunerErrorNone);
    /// <summary>
    /// Implements <see cref="IReceiver.Error">CReceiver::Error</see>
    /// </summary>
    /// <param name="tunerError"></param>
    /// <param name="socketError"></param>
    /// <param name="pkResult"></param>
    /// <param name="httpResponse"></param>
    /// <seealso cref="IReceiver">IReceiver</seealso>
    __override void                    Error(eTunerError tunerError, eSocketError socketError, pkRESULT pkResult, int httpResponse);
    /// <summary>
    /// Implements <see cref="IReceiver.WritePacket">CReceiver::WritePacket</see>
    /// </summary>
    /// <param name="packet">[IN] <see cref="IPacket">IPacket</see> to write to <see cref="CReceiver">CReceiver</see></param>
    /// <return>
    /// <para>true if successful, otherwise false</para>
    /// </return>
    /// <seealso cref="IReceiver">IReceiver</seealso>
    __override bool                    WritePacket(IPacket& packet);
    /// <summary>
    /// Implements <see cref="IReceiver.Notify">CReceiver::Notify</see>
    /// </summary>
    /// <param name="notificationType">[IN] type of notification</param>
    /// <param name="notificationData">[IN] notification specific data</param>
    /// <seealso cref="IReceiver">IReceiver</seealso>
    __override void                    Notify(ReceiverNotificationType notificationType, CReceiverNotificationData* notificationData);
    /// <summary>
    /// Implements <see cref="IReceiver.GetCurrentPlaybackTime">CReceiver::GetCurrentPlaybackTime</see>
    /// </summary>
    /// <param name="void"></param>
    /// <return>
    /// <para>Returns current playback time</para>
    /// </return>
    /// <seealso cref="IReceiver">IReceiver</seealso>
    __override uint64                  GetCurrentPlaybackTime(void);
    /// <summary>
    /// Implements <see cref="IReceiver.GetDecoderBufferStatus">CReceiver::GetDecoderBufferStatus</see>
    /// </summary>
    /// <param name="decoderBufferStatus">[OUT] return the current decoder buffer status</param>
    /// <return>
    /// </return>
    /// <seealso cref="IReceiver">IReceiver</seealso>
    __override void                    GetDecoderBufferStatus( _Out_ DecoderBufferStatus* pDecoderBufferStatus );

    /// <summary>
    /// Checks if the receiver can wait while the request of the next fragment stalls
    /// </summary>
    __override bool                    CanWaitForFragment();

public:
    /// <summary>
    /// Implements <see cref="ITunerSession.GetPipeIdN">ITunerSession::GetPipeIdN</see>
    /// </summary>
    /// <param></param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override uint32                  GetPipeIdN(void) { return mPipeIdN; }
    /// <summary>
    /// Implements <see cref="ITunerSession.PrepareToTune">ITunerSession::PrepareToTune</see>
    /// </summary>
    /// <param></param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    PrepareToTune(void);
    /// <summary>
    /// Implements <see cref="ITunerSession.TuneDone">ITunerSession::TuneDone</see>
    /// </summary>
    /// <param></param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    TuneDone(void);
    /// <summary>
    /// Implements <see cref="ITunerSession.Tune">ITunerSession::Tune</see>
    /// </summary>
    /// <param name="url">[IN] string representation of url to tune to</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    Tune(const std::string& tunerurl, int action, const int* timings);
    /// <summary>
    /// Implements <see cref="ITunerSession.TuneRefresh">ITunerSession::TuneRefresh</see>
    /// </summary>
    /// <param name="url">[IN] string representation of the url to tune to</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    TuneRefresh(const std::string &url);
    /// <summary>
    /// Implements <see cref="ITunerSession.Detune">ITunerSession::Detune</see>
    /// </summary>
    /// <param name="forceDetune">[IN] teardown the existing tuner if one exists</param>
    /// <param name="teardownPicture">[IN] tear down the picture</param>
    /// <param name="channelChange">[IN] indicate whether this detune is from a channel change</param>
    /// <param name="reason">[IN] indicate the reason of the detune</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    Detune(bool forceDetune, bool teardownPicture, bool channelChange, DetuneReason reason);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetBufferLength">ITunerSession::SetBufferLength</see>
    /// </summary>
    /// <param name="startTime">[IN] start time for this stream</param>
    /// <param name="endTime">[IN] end time for this stream</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetBufferLength(uint64 startTime, uint64 endTime);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetClipPlay">ITunerSession::SetClipPlay</see>
    /// </summary>
    /// <param name="clipStartTime">[IN] start time of clip for this stream</param>
    /// <param name="clipStartTimeSet">[IN] whether clip start time is set; otherwise treat this as start of the stream</param>
    /// <param name="clipEndTime">[IN] end time of clip for this stream</param>
    /// <param name="clipEndTimeSet">[IN] whether clip end time is set; otherwise treat this as end of the stream</param>
    /// <param name="clipLimits">[IN] bitmask of speeds for which to respect clip bounds</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetClipPlay(uint64 clipStartTime, bool clipStartTimeSet, uint64 clipEndTime, bool clipEndTimeSet, uint32 clipLimits);
    /// <summary>
    /// Implements <see cref="ITunerSession.PauseLive">ITunerSession::PauseLive</see>
    /// </summary>
    /// <param name="url">[IN] string representation of the trick url</param>
    /// <return>
    /// <para>true if successfull, otherwise false</para>
    /// </return>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override bool                    PauseLive(const std::string& url);
    /// <summary>
    /// Implements <see cref="ITunerSession.Pause">ITunerSession::Pause</see>
    /// </summary>
    /// <param name="void"></param>
    /// <return>
    /// <para>true if successfull, otherwise false</para>
    /// </return>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override bool                    Pause(void);
    /// <summary>
    /// Implements <see cref="ITunerSession.Play">ITunerSession::Play</see>
    /// </summary>
    /// <param name="void"></param>
    /// <return>
    /// <para>true if successfull, otherwise false</para>
    /// </return>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override bool                    Play(void);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetAccessControl">ITunerSession::SetAccessControl</see>
    /// </summary>
    /// <param name="blocked">[IN] true if AV should be blocked, false otherwise</param>
    /// <param name="startTime">[IN] speficies the start time of the blockage</param>
    /// <param name="endTime">[IN] speficies the stop time of the blockage</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetAccessControl(bool blocked, uint64 startTime, uint64 endTime);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetSecondaryAudioPreference">ITunerSession::SetSecondaryAudioPreference</see>
    /// </summary>
    /// <param name="usesap">[IN] true if secondary audio should be set, false otherwise</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetSecondaryAudioPreference(bool usesap);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetAudioLanguages">ITunerSession::SetAudioLanguages</see>
    /// </summary>
    /// <param name="iso639LanguageCodes">[IN] An array of ISO language identifiers, 0 otherwise</param>
    /// <param name="iso639LanguageCodesNum">[IN] Number of language identifies in the array, 0 otherwise</param>
    /// <param name="type">[IN] default audio type</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetAudioLanguages(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetAudioLanguage">ITunerSession::SetAudioLanguage</see>
    /// </summary>
    /// <param name="iso639LanguageCode">[IN] ISO language identifier, 0 otherwise</param>
    /// <param name="type">[IN] type to set, 0 otherwise</param>
    /// <param name="pid">[IN] pid to set, 0 otherwise</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetAudioLanguage(int iso639LanguageCode, int type, int pid);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetAudioDescriptionLanguages">ITunerSession::SetAudioDescriptionLanguages</see>
    /// </summary>
    /// <param name="iso639LanguageCodes">[IN] An array of ISO language identifiers, 0 otherwise</param>
    /// <param name="iso639LanguageCodesNum">[IN] Number of language identifies in the array, 0 otherwise</param>
    /// <param name="type">[IN] default audio type</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetAudioDescriptionLanguages(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetAudioDescriptionLanguage">ITunerSession::SetAudioDescriptionLanguage</see>
    /// </summary>
    /// <param name="iso639LanguageCode">[IN] ISO language identifier, 0 otherwise</param>
    /// <param name="type">[IN] type to set, 0 otherwise</param>
    /// <param name="pid">[IN] pid to set, 0 otherwise</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetAudioDescriptionLanguage(int iso639LanguageCode, int type, int pid);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetAudioDescriptionVolume">ITunerSession::SetAudioDescriptionVolume</see>
    /// </summary>
    /// <param name="volume">[IN] Volume of AD stream relative to main audio, -100 to 100</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetAudioDescriptionVolume(int volume);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetSubtitleLanguages">ITunerSession::SetSubtitleLanguages</see>
    /// </summary>
    /// <param name="iso639LanguageCodes">[IN] An array of ISO language identifiers, 0 otherwise</param>
    /// <param name="iso639LanguageCodesNum">[IN] Number of language identifies in the array, 0 otherwise</param>
    /// <param name="type">[IN] default subtitle type</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetSubtitleLanguages(const int* iso639LanguageCodes, int iso639LanguageCodesNum, int type);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetSubtitleLanguage">ITunerSession::SetSubtitleLanguage</see>
    /// </summary>
    /// <param name="iso639LanguageCode">[IN] ISO language identifier, 0 otherwise</param>
    /// <param name="type">[IN] type to set, 0 otherwise</param>
    /// <param name="pid">[IN] pid to set, 0 otherwise</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetSubtitleLanguage(int iso639LanguageCode, int type, int pid);
    /// <summary>
    /// Implements <see cref="ITunerSession.SetCCChannel">ITunerSession::SetCCChannel</see>
    /// </summary>
    /// <param name="cc608">[IN] 608 channel identifier</param>
    /// <param name="cc708">[IN] 708 channel identifier</param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override void                    SetCCChannel(int cc608, int cc708);
    /// <summary>
    /// Implements <see cref="ITunerSession.GetCurrentMediaTime">ITunerSession::GetCurrentMediaTime</see>
    /// </summary>
    /// <param name="smooth">[IN] run a smoothing algorithm on the reported ntp, false otherwise</param>
    /// <return>
    /// <para>current stream time specified in ntp time</para>
    /// </return>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override uint64                  GetCurrentMediaTime(bool smooth);
    /// <summary>
    /// Implements <see cref="ITunerSession.Command">ITunerSession::Command</see>
    /// Generic interface to send custom commands
    /// </summary>
    /// <return>
    /// <para>name:value pairs specific to commands</para>
    /// </return>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override bool                    Command(const std::string& command, const std::vector<std::string>& args);
    /// <summary>
    /// Implements <see cref="ITunerSession.IsStreamActive">ITunerSession::IsStreamActive</see>
    /// </summary>
    /// <param name="bool"></param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override bool                    IsStreamActive(void);
    /// <summary>
    /// Implements <see cref="ITunerSession.RetrieveDiagnostics">ITunerSession::RetrieveDiagnostics</see>
    /// </summary>
    /// <param name="IDiagsEvent*"></param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override IDiagsEvent*            RetrieveDiagnostics(void);
    /// <summary>
    /// Implements <see cref="ITunerSession.RetrieveTunerDiagnostics">ITunerSession::RetrieveTunerDiagnostics</see>
    /// </summary>
    /// <param name="IDiagsEvent*"></param>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override IDiagsEvent*            RetrieveTunerDiagnostics(void);
    /// <summary>
    /// Implements <see cref="ITunerSession.GetStatusCallback">ITunerSession::GetStatusCallback</see>
    /// </summary>
    /// <seealso cref="ITunerSession">ITunerSession</seealso>
    __override ITunerSessionCallback*  GetStatusCallback(void);

public:
    /// <summary>
    /// Implements <see cref="ITimeSlice.OnTimeslice">ITimeSlice.OnTimeslice</see>
    /// </summary>
    /// <param name="void"></param>
    /// <seealso cref="ITimeSlice">ITimeSlice</seealso>
    __override void                    OnTimeslice(void);

public:
    /// <summary>
    /// Implements <see cref="ISpliceControl.StopForSplice">ITimeSlice.StopForSplice</see>
    /// Stop receiver for a stream splice
    /// </summary>
    /// <seealso cref="ISpliceControl">ISpliceControl</seealso>
    __override void                    StopForSplice(void);
    /// <summary>
    /// Implements <see cref="ISpliceControl.PreflightNextAd">ITimeSlice.PreflightNextAd</see>
    /// Preflight next Ad
    /// </summary>
    /// <seealso cref="ISpliceControl">ISpliceControl</seealso>
    __override bool                    PreflightNextAd(void);
    /// <summary>
    /// Implements <see cref="ISpliceControl.DetuneFromPrimary">ITimeSlice.DetuneFromPrimary</see>
    /// Detunes from primary stream
    /// </summary>
    /// <seealso cref="ISpliceControl">ISpliceControl</seealso>
    __override void                    DetuneFromPrimary(void);
    /// <summary>
    /// Implements <see cref="ISpliceControl.TuneToSecondary">ITimeSlice.TuneToSecondary</see>
    /// Instaniates secondary socket
    /// </summary>
    /// <seealso cref="ISpliceControl">ISpliceControl</seealso>
    __override bool                    TuneToSecondary(std::string& secondaryUrl);
    /// <summary>
    /// Implements <see cref="ISpliceControl.JoinSecondary">ITimeSlice.JoinSecondary</see>
    /// Attaches receiver to secondary socket
    /// </summary>
    /// <seealso cref="ISpliceControl">ISpliceControl</seealso>
    __override void                    JoinSecondary(void);
    /// <summary>
    /// Implements <see cref="ISpliceControl.DetuneFromSecondary">ITimeSlice.DetuneFromSecondary</see>
    /// Detunes from a secondary stream
    /// </summary>
    /// <seealso cref="ISpliceControl">ISpliceControl</seealso>
    __override void                    DetuneFromSecondary(void);
    /// <summary>
    /// Implements <see cref="ISpliceControl.TuneBackToPrimary">ITimeSlice.TuneBackToPrimary</see>
    /// Tune back to primary stream with additional parameters supplied
    /// </summary>
    /// <seealso cref="ISpliceControl">ISpliceControl</seealso>
    __override bool                    TuneBackToPrimary(std::string& urlParameters);
    /// <summary>
    /// Implements <see cref="ISpliceControl.ResetForSplice">ITimeSlice.ResetForSplice</see>
    /// Resets the renderer in preparation for next stream being spliced
    /// </summary>
    /// <seealso cref="ISpliceControl">ISpliceControl</seealso>
    __override void                    ResetForSplice(void);

public:
    /// <summary>
    /// Implements <see cref="IDiagsProvider.DiagsReset">IDiagsProvider::DiagsReset</see>
    /// </summary>
    /// <param name="void"></param>
    /// <seealso cref="IDiagsProvider">IDiagsProvider</seealso>
    __override void                    DiagsReset(void);
    /// <summary>
    /// Implements <see cref="IDiagsProvider.DiagsRetrieve">IDiagsProvider::DiagsRetrieve</see>
    /// </summary>
    /// <param name="diagsEvent">[IN] <see cref="IDiagsEvent">IDiagsEvent</see> to report</param>
    /// <seealso cref="IDiagsProvider">IDiagsProvider</seealso>
    __override void                    DiagsRetrieve(IDiagsEvent* diagsEvent) {}
    /// <summary>
    /// Implements <see cref="IDiagsProvider.DiagsRetrieve">IDiagsProvider::DiagsRetrieve</see>
    /// Expected to be overridden by deriving classes
    /// </summary>
    /// <param name="void"></param>
    /// <seealso cref="IDiagsProvider">IDiagsProvider</seealso>
    __override IDiagsEvent*            DiagsRetrieve(void);

private:
    //Retrieve diagnostics update event
    IDiagsEvent*                       DiagsRetrieveInternal(void);

    //Set internal receiver status - also sends tuner status message
    void                               SetStatus(ReceiverNotificationType newStatus, CReceiverNotificationData* notificationData);

    //Signal that we've stopped streaming data and should start checking if rendering is done
    void                               SignalNearEnd(CReceiverNotificationData *notificationData);

    //Check for decoder stalls that may occur even if buffer isn't full
    bool                               CheckForDecoderStall(void);

    //Keep track of number of decoder stalls
    bool                               TrackDecoderStalls(void);
    //Send heart beat event when near End
    void                               HeartBeatEvent(uint64 curPts = 0);

private:
    //Protects access through external APIs
    mutable Lockable                   Receiver_Lock;

    //Factory of factories
    IAVManager*                        mAVManager;
    //Diags manager
    IDiagsManager*                     mDiagsManager;
    //Timeslice manager
    ITimesliceManager*                 mTimesliceManager;
    //Socket factory to use
    ISocketFactory*                    mSocketFactory;
    //HAL decoder factory to use
    IHalDecoderFactory*                mHalDecoderFactory;
    //Decoder factory to use
    IDecoderFactory*                   mDecoderFactory;
    //Renderer Factory to use
    IRendererFactory*                  mRendererFactory;
    //Splice Engine Factory to use
    ISpliceEngineFactory*              mSpliceEngineFactory;
    //Status callback
    ITunerSessionCallback*             mStatusCallback;

    //Renderer state
    CRendererState*                    mRendererState;

    //Container for all receiver diagnostics
    CReceiverDiagnostics*              mDiagnostics;

    //Clock
    Clock*                             mClock;

    //Pipe id
    uint32                             mPipeIdN;

    //Managing seamless splicing of different streams
    ISpliceEngine*                     mSpliceEngine;

    //Optional packet monitor
    IPacketMonitor*                    mPacketMonitor;

    //Current tune request being processed
    CTuneRequest                       mTuneRequest;

    //Current status
    ReceiverNotificationType           mStatus;

    //Current tuner url
    std::string                        mTunerUrl;
    //Current tuner url
    std::string                        mTunerUrlForNodeStatus;

    //Expected bitrate of currently tuned channel
    uint32                             BitRate;
    //Current bitrate of currently tuned channel
    uint32                             CurrentBitRate;
    //Measured network bandwidth from last socket dispose
    uint32                             mMeasuredNetworkBitsPerSec;

    //Current socket being used
    ISocket*                           mSocket;
    //Current renderer being used
    IRenderer*                         mRenderer;

    //Whether we have detuned away completely
    bool                               IsDetuned;

    //To change the priority of calling thread during the tune process
    HANDLE                             CallingThreadHandle;
    DWORD                              CallingThreadPriority;

    //Unique id assigned to the media transport that owns this AV pipe
    uint32                             MediaTransportId;
    uint32                             UniqueId;

    //Whether tuned for blackout only stream (background tunes by App when tuned to URl service)
    bool                               BlackoutOnlyTune;

    //Should not touch clock when re-creating tuner during PauseLive
    bool                               IsPauseLiveTransition;

    //To keep track of when the first packet was received after a tune
    bool                               FirstPacketReceived;

    //Internal components registered for timeslice
    std::list<ITimeslice*>             TimesliceList;

    //Whether we can retry writing buffers to renderer/decoder
    bool                               mRetry;
    //When Retry is false, whether we should reset on a decoder stall
    bool                               mResetOnStall;
    //Event used to exit the wait while decoder FIFO is full
    CEvent                             WaitWhileStalledEvent;

    //Keeping track of errors during tunes
    eTunerError                        CurrentTunerError;           //Error while tuning
    eSocketError                       CurrentSocketError;          //Socket specific error
    pkRESULT                           CurrentSocketPKResult;       //pkResult, if any
    int                                CurrentSocketHttpResponse;   //http response, if any


    //Everything to do with rendering completion
    bool                               NearEnd;
    CReceiverNotificationData          NearEndNotificationData;
    bool                               NearEndNotificationDataSet;

protected:
    //Whether we have been stopped
    bool                               Stopped;

    //Lock for renderer operations
    Lockable                           Renderer_Lock;

    // ===========================================================================================================
    // Tracking signal loss
    // ===========================================================================================================
private:
    //Initializes signal loss parameters
    void                               SignalLoss_Initialize(void);
    //Enabling signal loss
    void                               SignalLoss_Enable(void);
    //Signal has been reacquired
    void                               SignalLoss_Acquired(void);
    //Tracks where signal lost or reacquired
    void                               SignalLoss_Tracking(void);
private:
    //Tracking of signal loss when playing live.  The state of signal
    //loss is maintained through trick mode transition and a signal
    //acquire event could be sent when a packet is successfully
    //retrieved from tricks.  But note that we will not be tracking
    //signal loss when not live.
    uint32                             SignalReceivedAtTick;
    uint32                             SignalCheckedAtTick;
    bool                               SignalLost;
    bool                               SignalLossCheckEnabled;

    // ===========================================================================================================
    // Managing clip play functionality
    // ===========================================================================================================
private:
    //Clear clipping parameters
    void                               ClipPlay_Initialize(void);
    //Set clip play parameters
    void                               ClipPlay_Set(uint64 clipStartTime, bool clipStartTimeSet, uint64 clipEndTime, bool clipEndTimeSet, uint32 clipLimits);
    //Enable/disable clip play checks
    void                               ClipPlay_Enable(void);
    //Received a new packet - handles clip play
    bool                               ClipPlay_Check(IPacket& packet);
private:
    //Whether clip play is enabled for current stream
    bool                               ClipPlay_Enabled;
    //Clip play settings
    uint64                             ClipPlay_StartTime;
    bool                               ClipPlay_StartTimeSet;
    uint64                             ClipPlay_EndTime;
    bool                               ClipPlay_EndTimeSet;
    //Clip speeds enumeration of speeds for which to respect clip bounds
    //Please make sure that the enum values define matches those which are
    //defined in ITuneRequest.cs file...
    enum ClipPlayLimits
    {
        ClipPlayLimits_All      = 0,
        ClipPlayLimits_Play     = 1,
        ClipPlayLimits_Forward  = 2,
        ClipPlayLimits_Rewind   = 4,
        ClipPlayLimits_Pause    = 8
    };
    //Speeds for which to respect clip bounds
    ClipPlayLimits                     ClipPlay_Limits;
};

// ===============================================================================================================
// ===============================================================================================================
