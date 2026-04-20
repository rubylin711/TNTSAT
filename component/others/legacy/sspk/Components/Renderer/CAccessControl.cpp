///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CAccessControl.h"
#include "IReceiverControl.h"
#include "CReceiverDiagnostics.h"
#include "IDecoder.h"
#include "StringUtils.h"
#include "Trace.h"
#include <string>
#include <map>
using namespace std;

//#define ACCESSCONTROL_SPEW
#if defined(ACCESSCONTROL_SPEW)
#define ACCESSCONTROL_MSG(x) TRACE(x)
#else
#define ACCESSCONTROL_MSG(x)
#endif

// ===============================================================================================================
// ===============================================================================================================

bool ACCESS_TIME_BOUNDARY::SetValue(bool block, uint64 startTime, uint64 endTime)
{
    nBlock = block ? BLOCK_BLOCKED : BLOCK_UNBLOCKED;
    nTime1 = startTime;
    nTime2 = endTime;
    return nTime2 >= nTime1;
}

ACCESS_TIME_BOUNDARY& ACCESS_TIME_BOUNDARY::operator=(const ACCESS_TIME_BOUNDARY& boundaryInfo)
{
    nBlock = boundaryInfo.nBlock;
    nTime1 = boundaryInfo.nTime1;
    nTime2 = boundaryInfo.nTime2;
    return *this;
}

bool ACCESS_TIME_BOUNDARY::operator==(const ACCESS_TIME_BOUNDARY& boundaryInfo) const
{
    return ((nBlock == boundaryInfo.nBlock) &&
            (nTime1 == boundaryInfo.nTime1) &&
            (nTime2 == boundaryInfo.nTime2));
}

bool ACCESS_TIME_BOUNDARY::operator!=(const ACCESS_TIME_BOUNDARY& boundaryInfo) const
{
    return !operator==(boundaryInfo);
}

int ACCESS_TIME_BOUNDARY::FlipBlockState(int block)
{
    if (block == BLOCK_BLOCKED)
        return BLOCK_UNBLOCKED;

    if (block == BLOCK_UNBLOCKED)
        return BLOCK_BLOCKED;

    return BLOCK_UNKNOWN;
}

const char* ACCESS_TIME_BOUNDARY::Enum2String(int block)
{
    if (block == BLOCK_BLOCKED)
        return "blocked";

    if (block == BLOCK_UNBLOCKED)
        return "unblocked";

    return "unknown";
}

// access control info is stored in DecoderAV object.
// but between switch of channel or play mode (normal vs. trick),
// there is a short moment that no video DecoderAV object is in the map.
// i.e. the old decoderAV is removed and the new one is yet to be added.
// this static variable is used in those short peroids. it remembers the
// access control state of the last DecoderAV object.
AccessControlMap CAccessControl::AccessControlCounters;
Lockable CAccessControl::AccessControlCountersLock;

int CAccessControl::CheckAccessControl(uint32 pipeIdN, IDecoder* decoder)
{
    AutoLock lock(&AccessControlCountersLock);
    int blocked = ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN;
    AccessControlMap::iterator I = AccessControlCounters.find(pipeIdN);
    if (I != AccessControlCounters.end())
    {
        // check access control on the video decoder
        if (decoder && (decoder->IsAcquired() == 1))
        {
            blocked = decoder->CheckAccessControl();
        }

        ACCESSCONTROL_COUNTER& access_control_counter = (*I).second;
        if (blocked == ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN)
        {
            // return last known state if current state is unknown
            blocked = access_control_counter.last_block;
            ResetSkipCounter();
        }
        else
        {
            // otherwise save the last known state
            access_control_counter.last_block = blocked;
        }
    }
    return blocked;
}

bool CAccessControl::SkipAccessControlCheck(uint32 pipeIdN, int* p_last_block)
{
    AutoLock lock(&AccessControlCountersLock);

    // check if one exists for the pipe under question
    AccessControlMap::iterator I = AccessControlCounters.find(pipeIdN);
    if (I == AccessControlCounters.end())
    {
        *p_last_block = ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN;
        return true;
    }

    // check skip counters to determine if we should double check the blockage again
    ACCESSCONTROL_COUNTER& access_control_counter = (*I).second;

    if (access_control_counter.counter < CHECK_ACCESSCONTROL_CALL_2_SKIP)
    {
        ++access_control_counter.counter;
        *p_last_block = access_control_counter.last_block;
        return true;
    }

    access_control_counter.counter = 0;
    return false;
}

// skip counter used to be reset when no decoder is found (since channel switch rebuild the pipeline)
// but for pip channel switch, it happens too fast that between two contiguous compositor's GetNextFrame
// call, the decoder switch is done so 'no decoder is found' does not happen.
// reset skip counter now is done in constructor of DecoderAV object.
void CAccessControl::ResetSkipCounter()
{
    AutoLock lock(&AccessControlCountersLock);
    for (AccessControlMap::iterator I = AccessControlCounters.begin(); I != AccessControlCounters.end(); ++I)
    {
        (*I).second.counter = CHECK_ACCESSCONTROL_CALL_2_SKIP;
    }
}

// create a new entry in our static lookup table
void CAccessControl::InitAccessControl(uint32 pipeIdN)
{
    AutoLock lock(&AccessControlCountersLock);
    AccessControlMap::iterator I = AccessControlCounters.find(pipeIdN);
    if (I == AccessControlCounters.end())
    {
        ACCESSCONTROL_COUNTER& access_control_counter = AccessControlCounters[pipeIdN];
        access_control_counter.counter = CHECK_ACCESSCONTROL_CALL_2_SKIP;
        access_control_counter.last_block = ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN;
    }
}

bool CAccessControl::SetAccessControl(const ACCESS_TIME_BOUNDARY& boundary)
{
    AutoLock lock(&BoundaryInfoLock);
    BoundaryInfo = boundary;
    return BoundaryInfo.nTime2 >= BoundaryInfo.nTime1;
}

// returns BLOCK_UNBLOCKED if access is allowed
// returns BLOCK_BLOCKED if access is disallowed
// returns BLOCK_UNKNOWN if access control info is not yet set.
int CAccessControl::CheckTimeBoundary(uint64 currentNtp, int& last_block, IReceiverControl* receiverControl, CDecoderDiagnostics& decoderDiagnostics)
{
    ACCESS_TIME_BOUNDARY boundary_info;
    {
        AutoLock lock(&BoundaryInfoLock);
        boundary_info = BoundaryInfo;
    }

    // treat zero currentNtp as inboundary.
    // this happens when we are building the PCRPTS delay buffer following a tune
    bool bIsInBoundary = currentNtp ? (currentNtp >= boundary_info.nTime1 && currentNtp < boundary_info.nTime2) : true;
    // are we in between t1 and t2?
    int block =  bIsInBoundary ? boundary_info.nBlock : ACCESS_TIME_BOUNDARY::FlipBlockState(boundary_info.nBlock);

    // check if block state has changed
    if (last_block != block)
    {
        TRACE
            ((
            "======================> AccessControl (%s/%08x) : %s ---> %s, %s, %llx <============================",
            IsVideo ? "video" : IsAudioDescription ? "audio description" : "audio",
            mPipeIdN,
            ACCESS_TIME_BOUNDARY::Enum2String(last_block),
            ACCESS_TIME_BOUNDARY::Enum2String(block),
            bIsInBoundary ? "In Boundary" : "Out of Boundary",
            currentNtp
            ));

        if (block == ACCESS_TIME_BOUNDARY::BLOCK_BLOCKED)
        {
            decoderDiagnostics.OnBlocked();
        }
        else
        if (block == ACCESS_TIME_BOUNDARY::BLOCK_UNBLOCKED)
        {
            decoderDiagnostics.OnUnBlocked();
        }

        // notify managed app when we cross time boundary except on startup when we
        // transition from unknow to block/unblock state.
        if (IsVideo && currentNtp && (last_block != ACCESS_TIME_BOUNDARY::BLOCK_UNKNOWN))
        {
            string str =
                "status=accesscontrol&block=" + string(boundary_info.nBlock == ACCESS_TIME_BOUNDARY::BLOCK_BLOCKED?"true":"false") +
                "&t1=" + toString64(boundary_info.nTime1, true) +
                "&t2=" + toString64(boundary_info.nTime2, true) +
                "&type=" + string(IsVideo?"video":IsAudioDescription?"audiodesc":"audio") +
                "&curstate=" + ACCESS_TIME_BOUNDARY::Enum2String(block) +
                "&curtime=" + toString64(currentNtp, true);
            receiverControl->NotifyStatus(str);
        }

        // save the state
        last_block = block;
    }
    return block;
}

// ===============================================================================================================
// ===============================================================================================================
