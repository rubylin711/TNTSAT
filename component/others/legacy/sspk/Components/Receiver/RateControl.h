///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/*
 * RateControl.h
 *
 * This file describes the functionality of the RateControl
 */
#pragma once

#include <deque>

// for timing
class CChronometer
{
public:
    CChronometer();
    virtual ~CChronometer();

    void Reset();
    void Start();
    void Stop();
    void Restart();
    bool isRunning();

    uint64 GetTimeElapsedFromStart();

private:
    uint64     _frequency;
    uint64     _start;
    uint64     _stop;
    bool       _bRunning;
};

class RateInfo
{
public:
    RateInfo(uint32 nByteCount, uint32 nTimeSpent)
        : byteCount(nByteCount)
        , timeSpent(nTimeSpent)
    {
    }

    void Reset()
    {
        byteCount = 0;
        timeSpent = 0;
    }

    uint32 byteCount;
    uint32 timeSpent; // ms
};

//
class RateControl
{
public:
    // time over which bit rate will be measured. minimal 1000ms.
    static const uint32 MinTimeToMeasure = 1000;

    RateControl();

    void Init( uint32 controlledRate, uint32 thresholdRate, uint32 timeToMeasure, uint32 recvBlockSize );
    void SetControlledRate(uint32 controlledRate, uint32 thresholdRate);

    void Enable(bool enabled) { _enabled = enabled; }

    void StopRateControl();
    void ResumeRateControl();

    void StartNextReceive(uint32 mediaType, uint32 mbrIndex);
    void ReceivedCount(int nCount);
    void ResetCount();

    uint32 GetBitrate();
    uint32 GetRecvBlockSize();

    // how much time rate-controll slept since last time this function is called?
    uint32 GetSleepTimeSinceLastSampling();

private:
    uint32 AddRateInfo(RateInfo& rateInfo);

    CChronometer            _timer;
    uint32                  _controlledRate;    // bps
    uint32                  _thresholdRate;     // bps, stop rate-control when measured rate goes below threshold
    uint32                  _recvBlockSize;
    uint32                  _bitRate;           // bps
    uint32                  _byteCount;
    uint32                  _timeToMeasure;     // ms
    std::deque<RateInfo>    _rateInfoQueue;
    RateInfo                _sumRateInfo;
    bool                    _controlIsOn;   // used to turn on/off rate-control on the fly.
    bool                    _enabled;       // when true, rate-control's on/off is determined by _controlIsOn.
                                            // when false, rate-control is off.
    uint32                  _sleepTime;     // ms, sleep time since the last sampling.
};

