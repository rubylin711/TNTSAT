///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RateControl.h"
#include "Trace.h"
#include "ReceiverDiagEvents.h"
#include "pkExecutive.h"
#include <deque>
using namespace std;

//#define RATECONTROL_SPEW
#ifdef RATECONTROL_SPEW
#define RATECONTROL_TRACE(x) TRACE(x)
#else
#define RATECONTROL_TRACE(x)
#endif

// ===============================================================================================================
// ===============================================================================================================

CChronometer::CChronometer()
    : _start(0)
    , _stop(0)
    , _bRunning(false)
{
    QueryPerformanceFrequency((LARGE_INTEGER*)&_frequency);
}

CChronometer::~CChronometer()
{
}

void CChronometer::Reset()
{
    _start = _stop = 0;
    _bRunning = false;
}

void CChronometer::Start()
{
    QueryPerformanceCounter((LARGE_INTEGER*)&_start);
    _stop = _start;
    _bRunning = true;
}

void CChronometer::Stop()
{
    QueryPerformanceCounter((LARGE_INTEGER*)&_stop);
    _bRunning = false;
}

void CChronometer::Restart()
{
    QueryPerformanceCounter((LARGE_INTEGER*)&_stop);
    _start = _stop;
}

bool CChronometer::isRunning()
{
    return _bRunning;
}

uint64 CChronometer::GetTimeElapsedFromStart()
{
    uint64 t;
    if (_bRunning)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&t);
        t = t - _start;
    }
    else
    {
        t = _stop - _start;
    }

    return (t * TIMESCALE_10MHZ) / _frequency;        // in hns
}

RateControl::RateControl()
    : _controlledRate(0)
    , _thresholdRate(0)
    , _recvBlockSize(0)
    , _bitRate(INVALID_UINT32)
    , _byteCount(0)
    , _timeToMeasure(0)
    , _sumRateInfo(0, 0)
    , _controlIsOn(false)
    , _enabled(false)
    , _sleepTime(0)
{
}

void RateControl::Init(
    uint32 controlledRate,
    uint32 thresholdRate,
    uint32 timeToMeasure,
    uint32 recvBlockSize)
{
    RATECONTROL_TRACE(("@%p: RateControl::Init( %d, %d, %d, %d )", this, controlledRate, thresholdRate, timeToMeasure, recvBlockSize ));

    if (timeToMeasure < MinTimeToMeasure)
    {
        timeToMeasure = MinTimeToMeasure;
    }

    _controlledRate = controlledRate;
    _thresholdRate = thresholdRate;


    _timeToMeasure = timeToMeasure;
    _recvBlockSize = recvBlockSize;

    _timer.Reset();
    _timer.Start();
    _byteCount = 0;
    _bitRate = INVALID_UINT32;
    _sumRateInfo.Reset();
    _rateInfoQueue.clear();

    _controlIsOn = true;
    _sleepTime = 0;
}

void RateControl::SetControlledRate(uint32 controlledRate, uint32 thresholdRate)
{
    RATECONTROL_TRACE(("@%p: RateControl::SetControlledRate( %d, %d )", this, controlledRate, thresholdRate ));

    _controlledRate = controlledRate;

    if (_thresholdRate != thresholdRate)
    {
        // stream rate changed.
        // the collected rate info is for the old stream rate, clear it.
        _rateInfoQueue.clear();
        _sumRateInfo.Reset();

        _thresholdRate = thresholdRate;
        _sleepTime = 0;
    }
}

void RateControl::StopRateControl()
{
    _controlIsOn = false;
}

void RateControl::ResumeRateControl()
{
    _controlIsOn = true;
}

void RateControl::StartNextReceive(uint32  mediaType, uint32 mbrIndex)
{
    if (_byteCount >= _recvBlockSize)
    {
        // time should've spent
        uint32 time_should_spent = (uint32)((UINT64)_byteCount * 8 * 1000 / _controlledRate); // in ms
        // time actually spent till this moment
        uint32 time_spent = (uint32)(_timer.GetTimeElapsedFromStart() / 10000);    // in ms

        // control the rate when desired
        if (_enabled && _controlIsOn)
        {
            // It can happen that the rate of reading the current data block is faster than
            // the controlled rate, but the measured bit rate (over a longer time span) is
            // slower than the required stream rate. We should not sleep when this happens.

            if ((time_should_spent > time_spent) &&
                (!(_sumRateInfo.timeSpent >= _timeToMeasure && _bitRate <= _thresholdRate)))
            {
                uint32 sleep_time = time_should_spent - time_spent;

                RATECONTROL_TRACE((
                        "@%p: RateControl byteCount %d, timeShouldSpent %d, timeSpent %d, sleep %d",
                        this,
                        _byteCount,
                        time_should_spent,
                        time_spent,
                        sleep_time ));

                // sleep so time actually spent matches with time should spent.
                Executive_Sleep(sleep_time);
                _sleepTime += sleep_time;

                // time actually spent till this moment
                time_spent = (uint32)(_timer.GetTimeElapsedFromStart() / 10000);    // in ms
            }
        }

        // calculate the avg bitrate
        RateInfo info(_byteCount, time_spent);
        _bitRate = AddRateInfo(info);
        //if mbr index is zero  we need to change the rate to fix the problem that the Quality level is always stay zero
        //the rate change range is very small when the quality level is zero
        #if 0  
        if(mbrIndex == 0 || mbrIndex == 1){
            _bitRate = _bitRate * 1.5;
        }
        #else
          if(mbrIndex == 0)
          {
            _bitRate = _bitRate * 3;
          }
          else if(mbrIndex == 1)
          {
             _bitRate = _bitRate * 1.5;
          }
        #endif

        ReportEvent_RateMeasurement( this, _byteCount, time_spent, _bitRate, _timeToMeasure );

        // restart timer
        _timer.Restart();

        // reset _byteCount
        _byteCount = 0;
    }
}

void RateControl::ReceivedCount(int nCount)
{
    _byteCount += (uint32)nCount;
}

void RateControl::ResetCount()
{
    _timer.Restart();
    _byteCount = 0;
}

// return avg bitrate in bps
uint32 RateControl::AddRateInfo(RateInfo& rateInfo)
{
    // push the latest info into Q
    _sumRateInfo.byteCount += rateInfo.byteCount;
    _sumRateInfo.timeSpent += rateInfo.timeSpent;

    _rateInfoQueue.push_back(rateInfo);

    // pop the oldest info out of Q to maintain that
    // _sumRateInfo.timeSpent is eaque to _timeToMeasure
    while (_sumRateInfo.timeSpent > _timeToMeasure && (!_rateInfoQueue.empty()))
    {
        uint32 c = _rateInfoQueue.front().byteCount;
        uint32 t = _rateInfoQueue.front().timeSpent;

        uint32 t_delta = _sumRateInfo.timeSpent - _timeToMeasure;

        if (t_delta >= t)
        {
            // pop the oldest entry of the Q
            _sumRateInfo.byteCount -= c;
            _sumRateInfo.timeSpent -= t;
            _rateInfoQueue.pop_front();
        }
        else
        {
            // proportionally substract from the oldest entry
            uint32 c_delta = t_delta * c / t;
            _sumRateInfo.byteCount -= c_delta;
            _sumRateInfo.timeSpent -= t_delta;

            _rateInfoQueue.front().byteCount -= c_delta;
            _rateInfoQueue.front().timeSpent -= t_delta;
        }
    }

    if (_sumRateInfo.timeSpent)
        return (uint32)((uint64)_sumRateInfo.byteCount * 8 * 1000 / _sumRateInfo.timeSpent);

    return 0;
}

uint32 RateControl::GetBitrate()
{
    return _bitRate;
}

uint32 RateControl::GetRecvBlockSize()
{
    return _recvBlockSize;
}

// how much time rate-controll slept since last time this function is called?
uint32 RateControl::GetSleepTimeSinceLastSampling()
{
    uint32 ret = _sleepTime;
    _sleepTime = 0;

    return ret;
}

// ===============================================================================================================
// ===============================================================================================================
