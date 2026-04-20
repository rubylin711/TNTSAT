///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IHalClock.h"
#include "DecoderBufferStatus.h"
#include "CClockConfiguration.h"
#include "AutoLock.h"

// ===============================================================================================================
// Clock state
// ===============================================================================================================

enum eClockState
{
    eClockState_Set        = 0x01,
    eClockState_Running    = 0x02,
    eClockState_Correcting = 0x04,
    eClockState_Validating = 0x08,
};

// ===============================================================================================================
// NTP/PCR pairs for correspondences
// ===============================================================================================================

struct NTP_PCR_PAIR
{
    uint64 ntp;     // NTP (Network Time Protocol) time
    uint64 pcr;     // PCR (Program Clock Reference) time
};

// ===============================================================================================================
// Tracking NTP/PTS pairs for DVR trick modes
// ===============================================================================================================

struct RAP_FRAME_PAIR
{
    uint64 ntp;     // NTP (Network Time Protocol) time of the RAP (Random Access Point)
    uint64 pts;     // PTS (Presentation Time Stamp) time of the RAP (Random Access Point)
};

#define RAP_TABLE_LEN           1024
#define NEXT_RAP_TABLE_INDEX(i) ((i + 1) % RAP_TABLE_LEN)
#define PREV_RAP_TABLE_INDEX(i) ((i - 1 + RAP_TABLE_LEN) % RAP_TABLE_LEN)

// ===============================================================================================================
// Clock
// ===============================================================================================================

class  IReceiverControl;
class  IHalDecoderFactory;
class  CRendererState;
class  CReceiverDiagnostics;
class  IDecoder;
class  IReferenceClock;
class  CTuneRequest;
struct CReceiverNotificationData;

class Clock
{
public:

    Clock( _In_ IReceiverControl* pReceiverControl );

    ~Clock();

    //Currently called at the time of receiver instantiation/destruction
    bool                   Acquire(IHalDecoderFactory* halDecoderFactory, bool bMustbeHardwareClock);
    void                   Release(void);

    //Only expected to be used at the time of HAL decoder instantiation
    LPVOID                 GetContext(void) { return m_pHalClock->GetContext(); }

    //Returns current STC value
    uint64                 CurrentStc(void);

    //Current video decoder in action
    void                   SetVideoContext(IDecoder* decoder);

    //Current audio decoder in action
    void                   SetAudioContext(IDecoder* decoder);

    //Called by receiver when tune starts
    void                   OnTune(CTuneRequest& tuneRequest);

    //Called by receiver when receiving start signal from socket
    void                   OnStart(bool allowStart);

    //Called by receiver when receiving stop signal from socket
    void                   OnStop(void);

    //Called by receiver when detuning
    void                   OnDetune(void);

    //Called when resetting
    void                   OnReset(CReceiverNotificationData* notificationData);

    //Called when a sync happens
    void                   OnSync(void);

    //Called when stream changes (i.e. audio language switch, ad splicing, etc.)
    void                   OnStreamChange(bool changed);

    //Called on timeslice for clock to do some homework
    void                   OnTimeslice(void);

    //Called by video decoders to set clock start frame delay
    void                   SetClockStartFrameDelay(int streamType) { /* Not supported */ }

    //Called by sockets to set the buffer delay
    void                   SetStreamBufferDelay(int bufferDelay);

    //Called by sockets to set default wall clock buffer delay to use when no other signaling present
    void                   SetStreamBufferDefaultDelay(int defaultBufferDelay);

    //A bit map of different clock modes
    //Called by sockets to set the clock mode
    //Customize clock behavior based stream source
    void                   SetClockMode(uint32 clockMode);

    //Called by media transport to set the clock timing mode
    //Customizes clock timing behavior as expected by app
    void                   SetClockTimingMode(eClockTimingMode clockTimingMode);

    //Called set set the frames per second to be rendered when in immediate mode
    void                   SetTrickFrameRate(uint32 trickFrameRate);

    //Called to set if there is a video stream expected in the stream
    void                   VideoIsPresent(bool isVideoPresent) { m_bVideoIsPrimaryStream = m_bVideoIsPresent = isVideoPresent; }

    //Used to set video or audio as primary clock driver
    void                   VideoIsPrimaryStream(bool isVideoPrimaryStream) { m_bVideoIsPrimaryStream = isVideoPrimaryStream; }

    //Has the clock gone past given pts
    bool                   HasPtsExpired(uint64& pts);

    //Forces a clock start in case ES buffers have filled up
    bool                   ForceBufferDelayStart(void);
    bool                   IsSet(void) const { return StateIsSet(eClockState_Set); }
    bool                   IsRunning(void) const { return StateIsSet(eClockState_Running); }
    bool                   IsStarted(void) const { return m_bStartedSinceReset; }
    bool                   IsPaused(void) const { return m_bPause; }
    bool                   IsStopped(void) const { return !m_bStartedSinceReset || m_bPause; }


    //Pause/play a stream
    void                   Pause(bool bPause);

    //Stop the current stream and get the clock into ready state
    void                   Stop(void);

    //Push latest correspondences received in the stream - used for clock correction
    void                   PushCorrespondence(NTP_PCR_PAIR* correspondence);

    //Called by receiver to set if the PTS Offset needs to be recalculated
    void                   RecalculatePtsOffset(bool recalc) { /* Not supported */ }

    //Called by decoders to adjust the pts for every PES frame that is received
    //and has timing association for rendering purposes...
    bool                   AdjustPts(bool isVideo, uint64& pts) { /* Not supported */ return false; }


    //Called before any audio/video samples are pushed into the HAL decoders
    //These code paths are used to start the clock depending upon the incoming
    //stream behavior and track the underflow/overflow scenarios...

    enum SampleAction
    {
        eDeliverSample,
        eDiscardSample,
        eDelaySample,
    };

    SampleAction           AddVideoSample( _Inout_ uint64* pts );
    SampleAction           AddAudioSample( _In_ uint64 pts, _In_ uint64 lastPts, _In_ bool isAudioDesc );


    //Extract currently rendered video PTS, if any
    bool                   GetVideoPts(uint64* pts);

    //Extract currently rendered audio PTS, if any
    bool                   GetAudioPts(uint64* pts);

    //Last video pts sent to the video decoder
    uint64                 LastDecoderVideoPts(void) const { return m_LastVideoPts; }

    //Last audio pts sent to the audio decoder
    uint64                 LastDecoderAudioPts(void) const { return m_LastAudioPts; }

    //Determines buffer status
    void                   GetDecoderBufferStatus( _Out_ DecoderBufferStatus* pDecoderBufferStatus );

    //return current media time
    uint64                 GetCurrentMediaTime(bool smooth);

    //Convert given pts into NTP value
    uint64                 PTS2NTP(uint64 pts) const;

    //Extract NTP time of currently renderered video frame
    uint64                 CurrentNtp(bool smooth);

    //Checks whether the decoding/rendering has completed in the HAL
    bool                   IsRenderingDone(void);

    void                   SetEndOfStream();

private:

    // Resets the state of the object
    void                   Reset(void);

    //Initializes STC to given value
    void                   SetStc(uint64 val);

    //Get the latest STC
    uint64                 GetStc(void);

    //Check if pts is falling behind clock
    //Also logs the current buffer status in diagnostics
    bool                   CheckForUnderrun(uint64 pts, uint64 stc, bool bVideo);

    //Check if clock should be put into rebuffering mode
    void                   CheckForRebuffer(uint64 pts, uint64 stc, bool bVideo);

    //Check if we are done rebuffering and if so resume clock
    void                   CheckForRebufferClockStart(uint64 pts);

    //Track initial frame count and detect if audio is primary clock driver
    void                   OnInitialFrame(bool bVideo);

    //Used internally to start the clock
    void                   Start(bool bStart);

    //Set the base pcr/ntp time pair
    void                   UpdateBaseTime(uint64 ntp, uint64 pcr);
    bool                   AddVideoSample_Internal(uint64 pts);
    void                   SetVideoClock(uint64 val);
    bool                   AddAudioSample_Internal(uint64 pts);
    void                   SetAudioClock(uint64 val);
    void                   StreamDefined_StartClock(bool video, uint64 pts);

    //Extract current timings for fullscreen stream
    uint64                 CurrentNtpFullscreen(bool smooth);

    //Extract current timings for pip stream
    uint64                 CurrentNtpPip(bool smooth)const;

    //Initialize handling of pts/ntp correspondences table used for trick modes
    void                   RapTable_Init(void);

    //Push a new pts/ntp pair into the table
    void                   RapTable_PushPair( uint64 ntp, uint64 pts );

    //Lookup and return ntp time corresponding to a given pts
    uint64                 RapTable_Lookup(uint64 pts) const;

    bool ClockModeEnabled( eClockMode mode ) const { return( ( m_nClockMode & mode ) == mode ); }

    bool StateIsSet( eClockState s ) const { return( ( m_clockStateBitmask & s ) == s ); }
    void SetState( eClockState s )         { m_clockStateBitmask |= (uint8)s; }
    void ResetState( eClockState s )       { m_clockStateBitmask &= (uint8)~s; }

    static uint64 GetLevelFromTimeDiff( _In_ uint64 refPts, _In_ uint64 stc );

private:

    //
    // Attributes
    //

    IReceiverControl*      m_pReceiverControl;
    CRendererState&        m_RendererState;
    CReceiverDiagnostics&  m_Diagnostics;
    uint32                 m_PipeIdN;
    IReferenceClock*       m_pReferenceClock;
    IHalClock*             m_pHalClock;
    IDecoder*              m_pActiveVideoDecoder;
    IDecoder*              m_pActiveAudioDecoder;

    int                    m_nBufferDelay;          //Stream Buffer Delay
    int                    m_nDefaultBufferDelay;   //Wall clock (default) buffer delay when no other explicit signaling present

    //A bit map of different clock modes
    //Called by sockets to set the clock mode
    //Customize clock behavior based stream source
    //(see eClockMode enum)
    int                    m_nClockMode;

    //Called by media transports to set the clock timing mode
    //Customizes clock timing behavior as expected by app
    uint32                 m_nClockTimingMode;

    //Determines frame rate while in trick mode
    uint32                 m_frameDurationInTrickModePTS;

    // Tells if video is present
    bool                   m_bVideoIsPresent;

    //Whether the tune was just done and waiting clock to start the first time
    bool                   m_bJustTuned;

    //Whether we are paused
    bool                   m_bPause;

    //Lock used for accessing/updating parameters used to track timings
    mutable Lockable       m_MediaTimeLock;


    //Whether we are rebuffering
    bool                   m_bRebuffering;

    //Rebuffer threshold in 90kHz
    int                    m_iRebufferThreshold;

    //If stream has just changed (i.e. audio language switch, ad splice, etc.)
    bool                   m_bJustChangedStream;
    uint64                 m_nStreamChangeTickCount;

    //Internal clock state (see eClockState enum)
    uint8                  m_clockStateBitmask;

    //Whether clock has ever been started since Reset was called
    bool                   m_bStartedSinceReset;

    //Whether video or audio is the primary stream
    bool                   m_bVideoIsPrimaryStream;

    //How many video frames that we have received so far
    int32                  m_nInitialVideoFrameCount;

    //How many audio frames that we have received so far
    int32                  m_nInitialAudioFrameCount;

    //Default Clock Start
    uint64                 m_nBaseTickCount;
    uint64                 m_nPcrPtsExpire;

    //Base NTP and PCR used for tracking time
    uint64                 m_BaseNTP;
    uint64                 m_BasePCR;

    //First video pts
    uint64                 m_FirstVideoPts;

    //First audio pts
    uint64                 m_FirstAudioPts;

    //Last video pts sent to decoder
    uint64                 m_LastVideoPts;

    //Last audio pts sent to decoder
    uint64                 m_LastAudioPts;

    //Last pts sent to decoder
    uint64                 m_LastPts;

    //Synthesized pts
    uint64                 m_SynthPTS;

    //Last time timeslice checked for underrun/rebuffer
    uint32                 m_LastCheckForUnderrunTickCount;

    //Handling pts/ntp time pairs used for trick in Live streams
    int32                  m_RapTableIndex;
    RAP_FRAME_PAIR         m_RapTable[RAP_TABLE_LEN];
};

// ===============================================================================================================
// ===============================================================================================================
