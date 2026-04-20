///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ClockPK.h"
#include "CTrickConfiguration.h"
#include "IReceiverControl.h"
#include "CRendererState.h"
#include "CReceiverDiagnostics.h"
#include "IReferenceClock.h"
#include "IHalDecoder.h"
#include "CTuneRequest.h"
#include "CReceiverNotification.h"
#include "IDecoder.h"
#include "Trace.h"
#include "RendererDiagEvents.h"

//#define CLOCK_SPEW
#if defined(CLOCK_SPEW)
#define CLOCK_MSG(x) TRACE(x)
#else
#define CLOCK_MSG(x)
#endif

//#define CLOCK_EXTRA_SPEW
#if defined(CLOCK_EXTRA_SPEW)
#define CLOCK_EXTRA_MSG(x) TRACE(x)
#else
#define CLOCK_EXTRA_MSG(x)
#endif

// Check for underrun/rebuffer on timeslice thread should be given several opportunities to check 
// for low buffer condition before the default threshold
static const uint32 c_CheckForUnderrunMinTickDivisor = 5;

// ===============================================================================================================
// ===============================================================================================================

Clock::Clock( _In_ IReceiverControl* pReceiverControl )
    : m_pReceiverControl(pReceiverControl)
    , m_RendererState(pReceiverControl->GetRendererState())
    , m_Diagnostics(pReceiverControl->GetDiagnostics())
    , m_PipeIdN(m_RendererState.mPipeIdN)
    , m_pReferenceClock(pReceiverControl->GetAVManager()->GetReferenceClock())
    , m_pHalClock(NULL)
    , m_pActiveVideoDecoder(NULL)
    , m_pActiveAudioDecoder(NULL)
{
    //Stream Buffer Delay
    m_nBufferDelay = 0;
    //Wall clock (default) buffer delay when no other explicit signaling present
    m_nDefaultBufferDelay = gClockConfiguration[eDefaultBufferDelay_ms];
    //A bit map of different clock modes
    //Called by sockets to set the clock mode
    //Customize clock behavior based stream source
    m_nClockMode = eClockMode_None;
    //Called by media transports to set the clock timing mode
    //Customizes clock timing behavior as expected by app
    m_nClockTimingMode = eClockTimingMode_Unknown;
    //Default trick frame rate is 4 frames/sec
    SetTrickFrameRate(gTrickConfiguration.FramesPerSecondForTricks);
    //Whether the curretly tuned stream has video
    m_bVideoIsPresent = true;

    //Whether the tune was just done and waiting clock to start the first time
    m_bJustTuned = false;

    //Whether we are paused
    m_bPause = false;

    //Initialzie all internal parameters
    Reset();
}

Clock::~Clock()
{
    ASSERT(!m_pHalClock);
}

void Clock::Reset()
{
    AutoLock lock(&m_MediaTimeLock);

    CLOCK_MSG(("[%08x] Resetting", m_PipeIdN));

    m_bRebuffering = false;
    m_iRebufferThreshold = 90 * gClockConfiguration[eAVRebufferThreshold_ms];
    m_bJustChangedStream = false;
    m_nStreamChangeTickCount = 0;

    m_clockStateBitmask = 0;
    m_bStartedSinceReset = false;

    m_bVideoIsPrimaryStream = m_bVideoIsPresent;
    m_nInitialVideoFrameCount = 0;
    m_nInitialAudioFrameCount = 0;

    m_nBaseTickCount = 0;
    m_nPcrPtsExpire = 0;

    m_BaseNTP = INVALID_TIME;
    m_BasePCR = INVALID_TIME;

    m_FirstVideoPts = INVALID_TIME;
    m_FirstAudioPts = INVALID_TIME;

    m_LastVideoPts = INVALID_TIME;
    m_LastAudioPts = INVALID_TIME;
    m_LastPts = INVALID_TIME;

    m_SynthPTS = 0;

    m_LastCheckForUnderrunTickCount = 0;

    RapTable_Init();
}

bool Clock::Acquire(IHalDecoderFactory* halDecoderFactory, bool bMustbeHardwareClock)
{
    AutoLock lock(&m_MediaTimeLock);

    if (!m_pHalClock)
    {
        //Acquire zero's the clock
        m_pHalClock = halDecoderFactory->AcquireHalClock(bMustbeHardwareClock);
        if (m_pHalClock)
        {
            m_pHalClock->Init();
            m_pHalClock->SetTime(0);
        }
    }
    ASSERT(m_pHalClock);

    return (NULL != m_pHalClock);
}

void Clock::Release(void)
{
    AutoLock lock(&m_MediaTimeLock);

    if (m_pHalClock)
    {
        //Stop the clock immediately while releasing the clock
        Start(false);

        //And dump the HAL clock
        m_pHalClock->Release();
        m_pHalClock = NULL;
    }
}

void Clock::SetStc(uint64 val)
{
    m_pHalClock->SetTime(val);

    SetState(eClockState_Set);
}

uint64 Clock::GetStc(void)
{
    return m_pHalClock->GetTime();
}

uint64 Clock::CurrentStc(void)
{
    AutoLock lock(&m_MediaTimeLock);
    return GetStc();
}

//Called by receiver when tune starts
void Clock::OnTune(CTuneRequest& tuneRequest)
{
    //Set clock timing mode
    //This is a very important step; otherwise no timing woyld be reported by the clock
    SetClockTimingMode((eClockTimingMode)tuneRequest.GetInt(TUNE_REQUEST_CLOCKTIMINGMODE));

    //Always update the FPS during trick on tunes.  This will ensure that
    //the numbers are reflected if FPS is changed.
    //
    //Mainly used for usability studies right now...
    SetTrickFrameRate(gTrickConfiguration.FramesPerSecondForTricks);

    //Clock was just tuned
    m_bJustTuned = true;
}

//Called by receiver when receiving start signal from socket
void Clock::OnStart(bool allowStart)
{
    //Make sure clock is not in pause mode any more
    if (allowStart)
    {
        m_bPause = false;
        m_bRebuffering = false;
    }
}

//Called by receiver when receiving stop signal from socket
void Clock::OnStop(void)
{
}

//Called by receiver when detuning
void Clock::OnDetune(void)
{
    m_bJustTuned = false;
}

//Called when resetting
void Clock::OnReset(CReceiverNotificationData* notificationData)
{
}

//Called when a sync happens
void Clock::OnSync(void)
{
    //Reset audio and video buffer status
    m_Diagnostics.ResetBufferStatus(IsSet() ? CurrentStc() : INVALID_TIME);
}

//Called when stream changes (i.e. audio language switch, ad splicing, etc.)
void Clock::OnStreamChange(bool changed)
{
    m_bJustChangedStream = m_bStartedSinceReset && changed;
    m_nStreamChangeTickCount = m_bJustChangedStream ? Executive_GetTickCount() : 0;

    CLOCK_MSG(("[%08x] OnStreamChange(%s)", m_PipeIdN, m_bJustChangedStream ? "on" : "off"));
}

//Called on receiver timeslice
void Clock::OnTimeslice(void)
{
    // Check if buffer has stabilized yet after stream change
    if (m_bJustChangedStream)
    {
        uint64 now = Executive_GetTickCount();

        if ((now - m_nStreamChangeTickCount) > gClockConfiguration[eStreamChangeTransitionThreshold_ms])
        {
            CLOCK_MSG(("[%08x] Stream change timer is turned off after %d ms", m_PipeIdN, (int32)(now - m_nStreamChangeTickCount)));
            m_bJustChangedStream = false;
        }
    }

    // Check if there is an underrun/rebuffer
    if(Executive_GetTickCount() - m_LastCheckForUnderrunTickCount > gClockConfiguration[eAVRebufferThreshold_ms]/c_CheckForUnderrunMinTickDivisor)
    {
        m_LastCheckForUnderrunTickCount = Executive_GetTickCount();

        AutoLock lock(&m_MediaTimeLock);
        if(m_pActiveVideoDecoder)
        {
            CheckForUnderrun(m_pActiveVideoDecoder->GetLastPtsSent(), CurrentStc(), true);
        }
    
        if(m_pActiveAudioDecoder)
        {
            CheckForUnderrun(m_pActiveAudioDecoder->GetLastPtsSent(), CurrentStc(), false);
        }
    }

    //Log the latest buffer status
    m_Diagnostics.LogBuffer(CurrentStc());
}

//Called by sockets to set the buffer delay
void Clock::SetStreamBufferDelay(int bufferDelay)
{
    m_nBufferDelay = (bufferDelay > 0) ? bufferDelay : 0;
    m_iRebufferThreshold = 90 * gClockConfiguration[eAVRebufferThreshold_ms];

    CLOCK_MSG(("[%08x] SetStreamBufferDelay(%d)", m_PipeIdN, m_nBufferDelay));
}

//Called by sockets to set default wall clock buffer delay to use when no other signaling present
void Clock::SetStreamBufferDefaultDelay(int defaultBufferDelay)
{
    m_nDefaultBufferDelay = defaultBufferDelay;

    CLOCK_MSG(("[%08x] SetStreamBufferDefaultDelay(%d)", m_PipeIdN, m_nDefaultBufferDelay));
}

//A bit map of different clock modes
//Called by sockets to set the clock mode
//Customize clock behavior based stream source
void Clock::SetClockMode(uint32 clockMode)
{
    m_nClockMode = clockMode;

    CLOCK_MSG(("[%08x] SetClockMode(%08x)", m_PipeIdN, m_nClockMode));
}

//Called by media transport to set the clock timing mode
//Customizes clock timing behavior as expected by app
void Clock::SetClockTimingMode(eClockTimingMode clockTimingMode)
{
    ASSERT(clockTimingMode > eClockTimingMode_Unknown && clockTimingMode < eClockTimingMode_Max);
    m_nClockTimingMode = clockTimingMode;

    CLOCK_MSG(("[%08x] SetClockTimingMode(%d)", m_PipeIdN, m_nClockTimingMode));
}

//Called set set the frames per second to be rendered when in immediate mode
void Clock::SetTrickFrameRate(uint32 trickFrameRate)
{
    if( trickFrameRate != 0 )
    {
        m_frameDurationInTrickModePTS = 90000 / trickFrameRate;
    }
    else
    {
        // Default to NTSC frame rate
        static const uint32 NTSC_FRAME_DURATION_PTS = 3003;

        m_frameDurationInTrickModePTS = NTSC_FRAME_DURATION_PTS;
    }
}

//Has the clock gone past given pts
bool Clock::HasPtsExpired(uint64& pts)
{
    return IS_INVALID_TIME(pts) || !m_bStartedSinceReset || CurrentStc() >= pts;
}

//Forces a clock start in case ES buffers have filled up
bool Clock::ForceBufferDelayStart(void)
{
    //***** TO BE DONE *****
    //Do we really need this check?
    if (!ClockModeEnabled(eClockMode_UseBufferDelay))
        return false;

    //Quit if clock is already running or if we're paused intentionally
    if (IsRunning() || m_bPause)
        return false;

    uint64 pts = m_bVideoIsPrimaryStream ? m_LastVideoPts : m_LastAudioPts;
    if (IS_INVALID_TIME(pts))
        return false;

    uint64 stc = CurrentStc();
    if (pts < stc)
        return false;

    int newDelay = (int)(pts - stc) / 90;

    TRACE(("[%08x] ForceBufferDelayStart pts %lld delay %d -> %d", m_PipeIdN, pts, m_nBufferDelay, newDelay));

    //Adjust buffer delay value and start clock
    if (newDelay < m_nBufferDelay)
    {
        m_nBufferDelay = MAX(newDelay, m_nDefaultBufferDelay);
    }

    //And force start the clock now...
    StreamDefined_StartClock(m_bVideoIsPrimaryStream, pts);
    //Hopefully the clock has been started now...
    return IsRunning();
}

//Check if pts is falling behind clock
//Also logs the current buffer status in diagnostics
bool Clock::CheckForUnderrun(uint64 pts, uint64 stc, bool bVideo)
{
    if (!IsRunning() 
        || m_bPause 
        || m_RendererState.IsImmediateMode
        || IS_INVALID_TIME(pts))
    {
        return false;
    }

    //Log the buffer status
    m_Diagnostics.LogBuffer(stc);

    //Calculate the delta of this frame with stc
    int64 delta = (int64)(pts - stc);

    //Underrun???
    if (delta + 90 * gClockConfiguration[eAVBufferUnderflowThreshold_ms] < 0)
    {
        if (ClockModeEnabled(eClockMode_SyncOnUnderrun) || (bVideo == m_bVideoIsPrimaryStream))
        {
            TRACE(("[%08x] %s underrun detected - STC = %lld PTS = %lld (delta=%lld) - resetting ...", m_PipeIdN, bVideo ? "Video" : "Audio", stc, pts, delta));
            m_pReceiverControl->Sync(bVideo ? kSyncReason_VideoBufferUnderrun : kSyncReason_AudioBufferUnderrun, true, false);
            return true;
        }
        else
        if (bVideo)
        {
            CLOCK_MSG(("[%08x] Video underrun detected - STC = %lld PTS = %lld (delta=%lld) - passing data anyways ...", m_PipeIdN, stc, pts, delta));
            return false;
        }
        else
        {
            //TRACE(("[%08x] Audio underrun detected - STC = %lld PTS = %lld (delta=%lld) - dropping sample ...", m_PipeIdN, stc, pts, delta));
            //return true;
            CLOCK_MSG(("[%08x] Audio underrun detected - STC = %lld PTS = %lld (delta=%lld) - passing data anyways ...", m_PipeIdN, stc, pts, delta));
            return false;
        }
    }
    else
    {
        //No underrun, but still check if we should rebuffer
        CheckForRebuffer(pts, stc, bVideo);
        return false;
    }
}

//Check if clock should be put into rebuffering mode
void Clock::CheckForRebuffer(uint64 pts, uint64 stc, bool bVideo)
{
    if (!IsRunning() 
        || m_bPause 
        || !ClockModeEnabled(eClockMode_RebufferWhenUnderrun) 
        || m_RendererState.IsImmediateMode
        || IS_INVALID_TIME(pts))
    {
        return;
    }

    //Calculate the delta of this frame with stc
    int64 delta = (int64)(pts - stc);

    //Allow for some time for stream switches
    if (m_bJustChangedStream)
    {
        if (delta < m_iRebufferThreshold)
        {
            return;
        }

        CLOCK_MSG(("[%08x] Rebuffer now allowed since stream switch threshold has expired", m_PipeIdN));
        m_bJustChangedStream = false;
    }

    if (delta < m_iRebufferThreshold)
    {
        TRACE(("[%08x] %s buffer level critical - STC = %lld PTS = %lld (delta=%lld < %d) - rebuffering ...", m_PipeIdN, bVideo ? "Video" : "Audio", stc, pts, delta, m_iRebufferThreshold));

        //Go into rebuffering mode
        m_bRebuffering = true;

        //Pause the clock for rebuffering
        Start(false);

        //Since we're rebuffering, try increasing buffer delay to reduce chance of underrun in the future
        int newDelay = MIN(m_nBufferDelay + gClockConfiguration[eAVRebufferDelayIncrement_ms], gClockConfiguration[eAVRebufferMaxBufferDelay_ms]);
        TRACE(("[%08x] Rebuffer BufferDelay %d ms -> %d ms", m_PipeIdN, m_nBufferDelay, newDelay));
        SetStreamBufferDelay(newDelay);

        //Notify app and log the rebuffering state
        m_pReceiverControl->OnRebuffer(bVideo ? kSyncReason_VideoBufferUnderrun : kSyncReason_AudioBufferUnderrun, false, false);
    }
}

//Check if we are done rebuffering and if so resume clock
void Clock::CheckForRebufferClockStart(uint64 pts)
{
    int delay = MAX(m_nBufferDelay, m_nDefaultBufferDelay);
    uint64 stc = CurrentStc();
    if (pts >= (stc + delay * 90))
    {
        TRACE(("[%08x] StartClock(Rebuffer): pts = %lld stc = %lld delta = %lld ms delay = %d ms", m_PipeIdN, pts, stc, (pts - stc) / 90, delay));
        Start(true);
    }
}

//Track initial frame count and detect if audio is primary clock driver
void Clock::OnInitialFrame(bool bVideo)
{
    if (bVideo)
    {
        //Increment the video frame count
        m_nInitialVideoFrameCount++;
        //Keep track for first frame time
        if (m_nInitialVideoFrameCount == 1)
        {
            m_Diagnostics.OnFirstIFrameTime();
        }
    }
    else
    {
        //Increment the audio frame count
        m_nInitialAudioFrameCount++;
    }

    //Is audio being primary switch allowed for this stream?
    if (!ClockModeEnabled(eClockMode_AutoAudioPrimaryDetection))
        return;

    //Should we switch from video primary to audio primary for this stream?
    //Support for legacy streams that mark video as primary on audio dominant streams (e.g. music choice)
    int delta = m_nInitialAudioFrameCount - m_nInitialVideoFrameCount;
    int ratio = m_nInitialVideoFrameCount ? (m_nInitialAudioFrameCount / m_nInitialVideoFrameCount) : m_nInitialAudioFrameCount;
    if ((delta > 5) && (ratio >= 2))
    {
        TRACE(("[%08x] Switching to audio as primary (A%d,V%d)", m_PipeIdN, m_nInitialAudioFrameCount, m_nInitialVideoFrameCount));
        VideoIsPrimaryStream(false);
    }
}

void Clock::Start(bool bStart)
{
    AutoLock lock(&m_MediaTimeLock);

    if (m_pHalClock)
    {
#ifdef CLOCK_SPEW // stc only used in CLOCK_MSG macros
        uint64 stc = GetStc();
#endif // CLOCK_SPEW

        if (!bStart || m_bPause || !m_bStartedSinceReset)
        {
            if (StateIsSet(eClockState_Running))
            {
                ResetState(eClockState_Running);
                //Stop the clock
                m_pHalClock->Stop();
            }

            CLOCK_MSG(("[%08x] Stop(STC=%lld)", m_PipeIdN, stc));
        }
        else
        {
            //Automatically snap out of rebuffering mode on clock start
            m_bRebuffering = false;

            //Start the clock
            m_pHalClock->Start();

            //Set clock correction mode
            SetState(eClockState_Running);
            if (m_RendererState.IsFeatureEnabled(CRendererState::eFeature_ClockCorrection))
            {
                SetState(eClockState_Correcting);
            }
            if (m_RendererState.IsFeatureEnabled(CRendererState::eFeature_ClockValidation))
            {
                SetState(eClockState_Validating);
            }

            CLOCK_MSG(("[%08x] Start(STC=%lld)", m_PipeIdN, stc));

            //Send timing information for reporting
            m_Diagnostics.OnClockStart();

            //Send an event to media transport that the clock has now started...
            //We do this here because this is sent once at the time of native tune
            //whether it is at the time of channel change or trick mode transitions...
            if (m_bJustTuned)
            {
                CLOCK_MSG(("[%08x] Started Rendering", m_PipeIdN));
                m_pReceiverControl->NotifyStatus("status=rendering");
                m_bJustTuned = false;
            }
        }
    }
}

//Pause/play a stream
void Clock::Pause(bool bPause)
{
    m_bRebuffering = false;
    m_bPause = bPause;
    Start(!m_bPause);
}

//Stop the current stream and get the clock into ready state
void Clock::Stop(void)
{
    //Stop the clock
    Start(false);
    //And reset all internal parameters
    Reset();
}

//Set the base pcr/ntp time pair
void Clock::UpdateBaseTime(uint64 ntp, uint64 pcr)
{
    //Save the first NTP/PCR pair used as base time for stream
    AutoLock lock(&m_MediaTimeLock);
    
    if (IS_INVALID_TIME(m_BaseNTP))
    {
        if (m_RendererState.IsImmediateMode)
        {
            m_BaseNTP = NTP_UINT64TO10MHZ(ntp);
            m_BasePCR = 0;

            CLOCK_MSG(("[%08x] BaseTime [ntp=%llu][pcr=%llu] [%llu, %llu]", m_PipeIdN, ntp, pcr, m_BaseNTP, m_BasePCR));
        }
        else
        {
            m_BaseNTP = NTP_UINT64TO10MHZ(ntp);
            m_BasePCR = pcr;

            CLOCK_MSG(("[%08x] BaseTime [ntp=%llu][pcr=%llu] [%llu, %llu]", m_PipeIdN, ntp, pcr, m_BaseNTP, m_BasePCR));
        }
    }
}

//Push latest correspondences received in the stream - used for clock correction
void Clock::PushCorrespondence(NTP_PCR_PAIR* correspondence)
{
    //Ignore the call if it is not a rendering pipe
    if (!m_RendererState.IsFeatureEnabled(CRendererState::eFeature_RenderingPipe))
        return;

    //Server sometimes send the NTP times as 0 so ignore the correspondences in such cases...
    if (!correspondence->ntp)
    {
        CLOCK_MSG(("[%08x] Invalid Correspondence (%llu, %llu)", m_PipeIdN, correspondence->ntp, correspondence->pcr));
        return;
    }

    //Update base time
    UpdateBaseTime(correspondence->ntp, correspondence->pcr);

    if (m_RendererState.IsImmediateMode)
    {
        //Record NTP & synthesized PTS in rap table
        RapTable_PushPair( correspondence->ntp, m_SynthPTS );
        return;
    }
}

Clock::SampleAction Clock::AddVideoSample( _Inout_ uint64* pts )
{
    SampleAction action = eDeliverSample;

    if( !IS_VALID_TIME(*pts))
    {
        action = IS_VALID_TIME(m_FirstVideoPts) ? eDeliverSample : eDiscardSample;
        goto exit;
    }

    if (m_RendererState.IsImmediateMode)
    {
        if (m_bStartedSinceReset)
        {
            //Clock is already running
            uint64 stc = CurrentStc();
            uint64 fifoLevel90khz = GetLevelFromTimeDiff( m_LastVideoPts, stc );

            //We will limit how much we send to FIFO while in trick mode
            if (fifoLevel90khz > gClockConfiguration[eMaxEsFifoLimitInTrickPlayback_90kHz])
            {
                ReportEvent_LimitVideoFifo( this, m_PipeIdN, gClockConfiguration[eMaxEsFifoLimitInTrickPlayback_90kHz] );

                action = eDelaySample;
                goto exit;
            }

            ReportEvent_AddVideoSample( this, m_PipeIdN, stc, m_SynthPTS, true );

            //Log the video buffer status
            uint64 nextPts = m_SynthPTS + m_frameDurationInTrickModePTS;
            m_Diagnostics.pVideo->LogPts(nextPts, nextPts);
            m_Diagnostics.LogBuffer(stc);
        }
        else
        {
            //Start the clock if not already started
            TRACE(("[%08x] AddVideoSample Immediate mode clock start (pts=%lld)", m_PipeIdN, m_SynthPTS));
            //Set the clock
            SetVideoClock(m_SynthPTS);
            //And start the clock right away
            m_bStartedSinceReset = true;
            Start(true);
        }

        //Synthesize the new pts
        CLOCK_EXTRA_MSG(("[%08x] AddVideoSample Synthesize pts (%lld -> %lld)", m_PipeIdN, *pts, m_SynthPTS));
        *pts = m_SynthPTS;
        m_SynthPTS += m_frameDurationInTrickModePTS;
    }
    else // On Demand (not immediate) mode
    {
        if (m_bRebuffering && !m_bPause)
        {
            //Check to see if we've rebuffered enough data to start clock
            CheckForRebufferClockStart(*pts);
        }
        else if (m_bStartedSinceReset)
        {
            //Clock is already running
            uint64 stc = CurrentStc();

            // Limit how much is sent to the decoder FIFO while in on-demand mode.
            if( m_RendererState.IsFullScreen && ClockModeEnabled(eClockMode_TimeLimitFifo) )
            {
                uint64 fifoLevel90khz = GetLevelFromTimeDiff( m_LastVideoPts, stc );
				//seek long time will case stc is smaller than current pts  the play will not recovery
                if( fifoLevel90khz > gClockConfiguration[eMaxEsFifoLimitInNormalPlayback_90kHz] )
                {
                    ReportEvent_LimitVideoFifo( this, m_PipeIdN, gClockConfiguration[eMaxEsFifoLimitInNormalPlayback_90kHz] );

                    action = eDelaySample;
                    goto exit;
                }
            }

            ReportEvent_AddVideoSample( this, m_PipeIdN, stc, *pts, false );

            //Log video pts
            m_Diagnostics.pVideo->LogPts(*pts, *pts);

            //Check for underrun
            action = CheckForUnderrun( *pts, stc, true ) ? eDiscardSample : eDeliverSample;
        }
        else
        {
            action = AddVideoSample_Internal(*pts) ? eDeliverSample : eDiscardSample;
        }
    }

    if( action != eDiscardSample )
    {
        AutoLock lock(&m_MediaTimeLock);

        //Track pts of last video frame sent to the video decoder
        if (IS_INVALID_TIME(m_LastVideoPts) || *pts > m_LastVideoPts)
        {
            m_LastVideoPts = *pts;
        }
        //Track last valid pts sent to the decoder
        if (IS_INVALID_TIME(m_LastPts) || *pts > m_LastPts)
        {
            m_LastPts = *pts;
        }
    }

exit:

    return( action );
}

bool Clock::AddVideoSample_Internal(uint64 pts)
{
    //Is video the primary stream?
    if (m_bVideoIsPrimaryStream || m_RendererState.IsMHEGMode)
    {
        //Is this the first video frame
        if (IS_INVALID_TIME(m_FirstVideoPts))
        {
            //First video frame on multicast tune. Set the clock with this pts.
            SetVideoClock(pts);

            TRACE(("[%08x] AddVideoSample_Internal First video frame [pts=%lld][pts0=%lld][pts1=%lld]", m_PipeIdN, pts, m_FirstVideoPts, m_FirstAudioPts));
        }

        //Track initial frame count and detect if audio is primary clock driver
        OnInitialFrame(true);

        //Stream defined clock start
        StreamDefined_StartClock(true, pts);
    }

    return true;
}

void Clock::SetVideoClock(uint64 val)
{
    AutoLock lock(&m_MediaTimeLock);
    if (IS_INVALID_TIME(m_FirstVideoPts))
    {
        m_FirstVideoPts = val;
    }
    if ((m_bVideoIsPrimaryStream || m_RendererState.IsMHEGMode) && !StateIsSet(eClockState_Set))
    {
        CLOCK_MSG(("[%08x] SetVideoClock: %lld", m_PipeIdN, val));

        SetStc(val);
    }
}

//Always called with valid PTS.
//Note: this should never be called when we are in trick mode in which case
//we do not play audio at all...
//
//Determine if the clock should be started and start it

Clock::SampleAction Clock::AddAudioSample(
    _In_ uint64 pts,
    _In_ uint64 lastPts,
    _In_ bool isAudioDesc )
{
    SampleAction action = eDeliverSample;

    if (m_bRebuffering && !m_bPause)
    {
        //Check to see if we've rebuffered enough data to start clock
        CheckForRebufferClockStart(pts);
    }
    else if (m_bStartedSinceReset)
    {
        //Clock is already running
        uint64 stc = CurrentStc();

        ReportEvent_AddAudioSample( this, m_PipeIdN, stc, pts );

        //Log audio pts
        if (isAudioDesc)
        {
            m_Diagnostics.pAudioDescription->LogPts(pts, lastPts);
        }
        else
        {
            m_Diagnostics.pAudio->LogPts(pts, lastPts);
        }

        //Check for underrun

        action = CheckForUnderrun( pts, stc, false ) ? eDiscardSample : eDeliverSample;
    }
    else if (!isAudioDesc)
    {
        //Start the clock
        action = AddAudioSample_Internal(pts) ? eDeliverSample : eDiscardSample;
    }
    else
    {
        //Never start the clock for audio description; wait for main audio
        action  = eDiscardSample;
    }

    if( action == eDeliverSample )
    {
        AutoLock lock(&m_MediaTimeLock);

        //Track pts of last audio frame sent to the audio decoder
        if (IS_INVALID_TIME(m_LastAudioPts) || lastPts > m_LastAudioPts)
        {
            m_LastAudioPts = lastPts;
        }
        //Track last valid pts sent to the decoder
        if (IS_INVALID_TIME(m_LastPts) || lastPts > m_LastPts)
        {
            m_LastPts = lastPts;
        }
    }

    return( action );
}

bool Clock::AddAudioSample_Internal(uint64 pts)
{
    //Is video the primary stream?
    if (m_bVideoIsPrimaryStream)
    {
        //Is this the first audio frame?
        if (IS_INVALID_TIME(m_FirstAudioPts))
        {
            //Make sure the audio packet is not behind the video
            if (IS_INVALID_TIME(m_FirstVideoPts) || ((uint64)pts < m_FirstVideoPts))
                return false;

            SetAudioClock(pts);

            TRACE(("[%08x] AddAudioSample_Internal First audio frame [pts=%lld][pts0=%lld][pts1=%lld]", m_PipeIdN, pts, m_FirstVideoPts, m_FirstAudioPts));
        }

        //Track initial frame count and detect if audio is primary clock driver
        OnInitialFrame(false);
    }
    else
    {
        if (IS_INVALID_TIME(m_FirstAudioPts))
        {
            //Set the clock to the first audio packet
            SetAudioClock(pts);
            SetVideoClock(pts);
        }

        //Start the clock
        StreamDefined_StartClock(false, pts);
    }
    return true;
}

void Clock::SetAudioClock(uint64 val)
{
    AutoLock lock(&m_MediaTimeLock);
    if (IS_INVALID_TIME(m_FirstAudioPts))
    {
        m_FirstAudioPts = val;
    }
    if (!m_bVideoIsPrimaryStream && !StateIsSet(eClockState_Set))
    {
        CLOCK_MSG(("[%08x] SetAudioClock: %lld", m_PipeIdN, val));

        SetStc(val);
    }
}

//Called to decide when to start the clock based on information in the incoming stream
void Clock::StreamDefined_StartClock(bool video, uint64 pts)
{
    if (ClockModeEnabled(eClockMode_UseBufferDelay) || m_bRebuffering)
    {
        //Clock start handling for scenarios where buffery delay is defined.
        if (StateIsSet(eClockState_Set))
        {
            int delay = MAX(m_nBufferDelay, m_nDefaultBufferDelay);
            uint64 stc = CurrentStc();
            //TRACE(("[%08x] StreamDefined_StartClock: m_nBufferDelay = %d, m_nDefaultBufferDelay = %d delay = %d", m_PipeIdN, m_nBufferDelay, m_nDefaultBufferDelay, delay));	
            //TRACE(("[%08x] StreamDefined_StartClock: pts = %lld stc = %lld delta = %lld(ms)", m_PipeIdN, pts, stc, (int64)(pts - stc)/90));
            if(pts > 20000000000000){//live media
                //TRACE(("[%08x] 111StreamDefined_StartClock: pts = %lld stc = %lld delta = %lld(ms)", m_PipeIdN, pts, stc, (int64)(pts - stc)/90));
                if (pts >= (stc + delay * 90) && (stc != 0 && stc > 10000000000000))//modify for live play the pts 0 will case start play failed
                {
                    TRACE(("[%08x] Live media StartClock(Preroll Start): pts = %lld stc = %lld delta = %lld(ms)", m_PipeIdN, pts, stc, (int64)(pts - stc)/90));
                    //If correspondence is already set (in the case of Smooth Streaming, where it is always set on the RAP packet),
                    //don't overwrite the base PCR and NTP.
                    if (IS_INVALID_TIME(m_BaseNTP))
                    {
                        AutoLock lock(&m_MediaTimeLock);
                        m_BasePCR = stc;
                        m_BaseNTP = 0;
                    }
                    m_bStartedSinceReset = true;
                    Start(true);
                }
            }else{
                //TRACE(("[%08x] 222StreamDefined_StartClock: pts = %lld stc = %lld delta = %lld(ms)", m_PipeIdN, pts, stc, (int64)(pts - stc)/90));
                if (pts >= (stc + delay * 90) && (stc != 0))//modify for live play the pts 0 will case start play failed
                {
                    TRACE(("[%08x] StartClock(Preroll Start): pts = %lld stc = %lld delta = %lld(ms)", m_PipeIdN, pts, stc, (int64)(pts - stc)/90));
                    //If correspondence is already set (in the case of Smooth Streaming, where it is always set on the RAP packet),
                    //don't overwrite the base PCR and NTP.
                    if (IS_INVALID_TIME(m_BaseNTP))
                    {
                        AutoLock lock(&m_MediaTimeLock);
                        m_BasePCR = stc;
                        m_BaseNTP = 0;
                    }
                    m_bStartedSinceReset = true;
                    Start(true);
                }
            }

        }
    }
    else
    {
        //Baseline the first time we tried to start the clock
        if (!m_nBaseTickCount)
        {
            m_nBaseTickCount = Executive_GetTickCount();
        }

        //Received any correspondence yet?
        if (IS_INVALID_TIME(m_BasePCR))
        {
            //No correspondences but is the clock set?
            if (StateIsSet(eClockState_Set))
            {
                //Start the clock after a default delay
                uint64 stc = CurrentStc();
                int delay = MAX(m_nBufferDelay, m_nDefaultBufferDelay);
                if (pts > stc + (90 * delay))
                {
                    TRACE(("[%08x] StartClock(NoCorrespondence Start): pts = %lld stc = %lld delta = %lld(ms)", m_PipeIdN, pts, stc, (int64)(pts - stc)/90));

                    //Clock starts without correspondence.
                    //initialize basepcr and basentp so we'll still return a non-zero ntp time for a given pts.
                    {
                        AutoLock lock(&m_MediaTimeLock);
                        m_BasePCR = stc;
                        m_BaseNTP = 0;
                    }
                    m_bStartedSinceReset = true;
                    Start(true);
                }
            }
        }
        else
        {
            //use the PCRPTS from the first correspondence (which should be on the first RAP)
            //we use absolute PCRPTS delay (wall clock not stream time) to start the clock
            if (!m_nPcrPtsExpire)
            {
                AutoLock lock(&m_MediaTimeLock);
                m_nPcrPtsExpire = m_nBaseTickCount + ((int64)(pts - m_BasePCR)/90);
            }

            //What is the current time
            uint64 curTime = Executive_GetTickCount();

            //Check if we have satisfied the PCR to PTS delay
            if (curTime > m_nPcrPtsExpire)
            {
                TRACE(("[%08x] StartClock(Correspondences Start): m_nPcrPtsExpire=%lld m_nBaseTickCount=%lld delta=%lld PCRPTS=%lld", m_PipeIdN, m_nPcrPtsExpire, m_nBaseTickCount, m_nPcrPtsExpire - m_nBaseTickCount, pts - m_FirstVideoPts));
                m_bStartedSinceReset = true;
                Start(true);
            }
        }
    }
}

// ===============================================================================================================
// ===============================================================================================================

void Clock::SetVideoContext(IDecoder* decoder)
{
    AutoLock lock(&m_MediaTimeLock);
    //Current video decoder in action
    m_pActiveVideoDecoder = decoder;
}

void Clock::SetAudioContext(IDecoder* decoder)
{
    AutoLock lock(&m_MediaTimeLock);
    //Current audio decoder in action
    m_pActiveAudioDecoder = decoder;
}

bool Clock::GetVideoPts(uint64* pts)
{
    AutoLock lock(&m_MediaTimeLock);
    if (m_pActiveVideoDecoder && m_pActiveVideoDecoder->GetCurrentPTS(pts))
    {
        CLOCK_MSG(("Clock::GetVideoPts %lld",*pts));
        return true;
    }
    return false;
}

bool Clock::GetAudioPts(uint64* pts)
{
    AutoLock lock(&m_MediaTimeLock);
    if (m_pActiveAudioDecoder && m_pActiveAudioDecoder->GetCurrentPTS(pts))
    {
        return true;
    }
    return false;
}


/*static*/ uint64 Clock::GetLevelFromTimeDiff( _In_ uint64 refPts, _In_ uint64 stc )
{
    if( IS_VALID_TIME(refPts) && ( refPts >= stc ) )
    {
        return( refPts - stc );
    }
    else
    {
        return( 0 );
    }
}

void Clock::GetDecoderBufferStatus( _Out_ DecoderBufferStatus* pDecoderBufferStatus )
{
    if (StateIsSet(eClockState_Set))
    {
        pDecoderBufferStatus->currentStc90kHz = CurrentStc();

        pDecoderBufferStatus->audioBufferLevel90kHz = (uint32)GetLevelFromTimeDiff( m_LastAudioPts, pDecoderBufferStatus->currentStc90kHz );
        pDecoderBufferStatus->videoBufferLevel90kHz = (uint32)GetLevelFromTimeDiff( m_LastVideoPts, pDecoderBufferStatus->currentStc90kHz );

        uint32 maxVideoBuffer;
        uint32 rebufferThreshold;
        uint32 videoBufferDelta;

        //Target buffer level
        if( m_RendererState.IsImmediateMode )
        {
            maxVideoBuffer = gClockConfiguration[eMaxEsFifoLimitInTrickPlayback_90kHz];
            videoBufferDelta = gClockConfiguration[eEsFifoDeltaToRequestFragmentInTrickPlayback_90kHz];
        }
        else
        {
            maxVideoBuffer = gClockConfiguration[eMaxEsFifoLimitInNormalPlayback_90kHz];
            videoBufferDelta = gClockConfiguration[eEsFifoDeltaToRequestFragmentInNormalPlayback_90kHz];
            rebufferThreshold = 90 * gClockConfiguration[eAVRebufferThreshold_ms];
        }

        rebufferThreshold = 90 * gClockConfiguration[eAVRebufferThreshold_ms];

        // Adjust the delta (if necessary) to ensure the following condition is true:
        //
        //      rebufferThreshold < ( maxVideoBuffer - videoBufferDelta ) < maxVideoBuffer
        //

        if( maxVideoBuffer < rebufferThreshold )
        {
            TRACE(( "Clock::GetDecoderBufferStatus@%p: BAD CONFIG, maxBuffer (%u) < rebufferThreshold (%u)", this, maxVideoBuffer, rebufferThreshold ));

            rebufferThreshold = maxVideoBuffer;
        }

        if( maxVideoBuffer < videoBufferDelta )
        {
            TRACE(( "Clock::GetDecoderBufferStatus@%p: BAD CONFIG, maxBuffer (%u) < delta (%u)", this, maxVideoBuffer, videoBufferDelta ));

            videoBufferDelta = maxVideoBuffer;
        }

        if( ( maxVideoBuffer - rebufferThreshold ) < videoBufferDelta )
        {
            TRACE(( "Clock::GetDecoderBufferStatus@%p: BAD CONFIG, maxBuffer (%u) - delta (%u) < rebufferThreshold (%u)", this, maxVideoBuffer, videoBufferDelta, rebufferThreshold ));

            videoBufferDelta = maxVideoBuffer - rebufferThreshold;
        }

        pDecoderBufferStatus->maxVideoBufferLevel90kHz = maxVideoBuffer;
        pDecoderBufferStatus->minVideoBufferLevelToTakeFragment90kHz = maxVideoBuffer - videoBufferDelta;

        pkASSERT( rebufferThreshold < pDecoderBufferStatus->minVideoBufferLevelToTakeFragment90kHz );
        pkASSERT( pDecoderBufferStatus->minVideoBufferLevelToTakeFragment90kHz < pDecoderBufferStatus->maxVideoBufferLevel90kHz );
    }
    else
    {
        memset( pDecoderBufferStatus, 0, sizeof(*pDecoderBufferStatus) );
    }

    pDecoderBufferStatus->clockIsRunning = StateIsSet( eClockState_Running );
}

uint64 Clock::GetCurrentMediaTime(bool smooth)
{
    //If this is a pip receiver, mediatime is meaningless (it's fullscreen time)
    if (!m_RendererState.IsFullScreen)
    {
        return INVALID_TIME;
    }

    //If the clock has not started then the media time is meaning less as well
    if (!m_bStartedSinceReset)
    {
        return INVALID_TIME;
    }

    uint64 time = INVALID_TIME;

    //Return as NTP time
    if (m_nClockTimingMode == eClockTimingMode_NTPUnit)
    {
        time = CurrentNtp(smooth);

        if (time == 0)
        {
            time = INVALID_TIME;
        }
    }
    else
    //Return the STC timing rolled over
    if (m_nClockTimingMode == eClockTimingMode_STCUnit)
    {
        time = CurrentStc();
    }
    return time;
}

uint64 Clock::PTS2NTP(uint64 pts) const
{
    //NTP Clock = 10 MHz, PCR clock = 90 KHz
    if (IS_INVALID_TIME(m_FirstVideoPts) && IS_INVALID_TIME(m_FirstAudioPts))
    {
        return 0;
    }

    if (IS_INVALID_TIME(m_BaseNTP) || IS_INVALID_TIME(m_BasePCR))
    {
        return 0;
    }

    if (pts < m_BasePCR)
    {
        TRACE(("Clock::PTS2NTP clamping pts_90kHz = %llu to m_BasePCR_90kHz = %llu, delta_90kHz = %llu",
            pts, m_BasePCR, m_BasePCR - pts));

        pts = m_BasePCR;
    }

    return NTP_10MHZTOUINT64(m_BaseNTP + ((1000 * (pts - m_BasePCR)) / 9));
}

uint64 Clock::CurrentNtp(bool smooth)
{
    if (!StateIsSet(eClockState_Set))
    {
        return 0;
    }

    uint64 ntp = m_RendererState.IsFullScreen ? CurrentNtpFullscreen(smooth) : CurrentNtpPip(smooth);
    return ntp;
}

uint64 Clock::CurrentNtpFullscreen(bool smooth)
{
    AutoLock lock(&m_MediaTimeLock);
    uint64 pts;

    if (m_RendererState.IsImmediateMode)
    {
        //Get the currently rendered frame pts
        if (!GetVideoPts(&pts))
        {
            return 0;
        }

        CLOCK_MSG(("Clock::CurrentNtpFullscreen pts:%lld m_SynthPTS:%lld",pts,m_SynthPTS));

        //The frame on the screen cannot have pts more than the last pts
        //we passed to it.
        if (pts > m_SynthPTS)
        {
            return 0;
        }

        //Do a lookup in the RAP table to figure out the NTP time of frame
        //on the screen
        return RapTable_Lookup(pts);
    }

    if (smooth)
    {
        //If nothing pushed into the decoder
        if (IS_INVALID_TIME(m_LastPts))
        {
            return 0;
        }

        //Get the currently rendered frame pts
        if (!GetVideoPts(&pts))
        {
            return 0;
        }

        //The frame on the screen cannot have pts more than the last pts
        //we passed to it.
        if (pts > m_LastPts)
        {
            return 0;
        }
    }
    else
    {
        //Get the currently rendered frame pts; otherwise return the STC
        pts = 0;
        if (!GetVideoPts(&pts))
        {
            pts = CurrentStc();
        }
    }
    return PTS2NTP(pts);
}

uint64 Clock::CurrentNtpPip(bool smooth) const
{
    //Pip decoder uses global ntp time
    return m_pReferenceClock->GetTimeEx();
}

// ===============================================================================================================
// Checks whether the decoding/rendering has completed in the HAL
// ===============================================================================================================

bool Clock::IsRenderingDone(void)
{
    AutoLock lock(&m_MediaTimeLock);

    //Done if clock has never been started
    if (!m_bStartedSinceReset)
    {
        return true;
    }

    //Not done when paused
    if (m_bPause)
    {
        return false;
    }

    //Check if clock was stopped from underrun/rebuffer
    if(!StateIsSet(eClockState_Running))
    {
        return true;
    }

    //Otherwise check with decoders if they are done
    uint64 stc = CurrentStc();
    return ((!m_pActiveVideoDecoder || m_pActiveVideoDecoder->IsRenderingDone(stc)) &&
            (!m_pActiveAudioDecoder || m_pActiveAudioDecoder->IsRenderingDone(stc)));
}

// ===============================================================================================================
// Let decoders know end of stream
// ===============================================================================================================

void Clock::SetEndOfStream()
{
    if (m_pActiveVideoDecoder)
    {
        m_pActiveVideoDecoder->SetEndOfStream();
    }

    if (m_pActiveAudioDecoder)
    {
        m_pActiveAudioDecoder->SetEndOfStream();
    }
}

// ===============================================================================================================
// Tracking pts/ntp correspondences in a table for trick modes in live streams
// ===============================================================================================================

//Initialize handling of pts/ntp correspondences table used for trick modes
void Clock::RapTable_Init(void)
{
    m_RapTableIndex = 0;
    memset(m_RapTable, 0, sizeof(m_RapTable));
}

//Push a new pts/ntp pair into the table
void Clock::RapTable_PushPair( uint64 ntp, uint64 pts )
{
    AutoLock lock(&m_MediaTimeLock);

    m_RapTableIndex = NEXT_RAP_TABLE_INDEX(m_RapTableIndex);

    CLOCK_MSG(( "RapTable_PushPair index %d ntp %lld pts: %lld", m_RapTableIndex, ntp, pts ));

    m_RapTable[ m_RapTableIndex ].ntp = ntp;
    m_RapTable[ m_RapTableIndex ].pts = pts;
}

//Lookup and return ntp time corresponding to a given pts
uint64 Clock::RapTable_Lookup( uint64 pts ) const
{
    int index = m_RapTableIndex;
    int end = NEXT_RAP_TABLE_INDEX(index);

    uint64 lastPTS = (uint64)-1;

    CLOCK_MSG(("pts %lld index=%d end=%d RAPntp=%lld RAPpts=%lld lastPTS=%lld",pts, index,end,m_RapTable[index].ntp,m_RapTable[index].pts, lastPTS ));

    // The RAP table is a circular list. Walk it backwards from the newest to the oldest entry,
    // or until a reset point (ntp is 0).
    while( index != end )
    {
        if( m_RapTable[index].ntp == 0 )
        {
            break;
        }

        if( m_RapTable[index].pts <= pts && pts < lastPTS )
        {
            return( m_RapTable[index].ntp );
        }

        lastPTS = m_RapTable[index].pts;
        index = PREV_RAP_TABLE_INDEX(index);
    }

    return 0;
}

// ===============================================================================================================
// ===============================================================================================================
