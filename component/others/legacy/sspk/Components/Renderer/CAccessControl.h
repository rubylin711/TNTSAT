///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AutoLock.h"
#include <string>
#include <map>

// ===============================================================================================================
// ===============================================================================================================

// compositor (in its best effort) tries to call us 30 times a second.
// we don't have to check access control that frequently.
// for now, we do once for every 15 call.
#define CHECK_ACCESSCONTROL_CALL_2_SKIP 15

typedef struct ACCESSCONTROL_COUNTER_tag
{
    int counter;
    int last_block;
} ACCESSCONTROL_COUNTER;

typedef std::map<uint32, ACCESSCONTROL_COUNTER> AccessControlMap;

// CAccessControl provides basic blocking/unblocking services on a given stream. The service is used by managed
// app to implement features such as PCon, EAS, etc.

// When providing the blocking/unblocking service, CAccessControl object is not aware of the differences of those
// implemented features such as PCon or EAS, or any concurrency of these events. All it keeps track of is a
// time boundary of a blocking operation plus a flag indicates the need of flushing decoder when the blocking
// starts. The rest is taken care of by the managed side, i.e. the concurrency of PCon/EAS, or the fact that EAS
// might go across channel tune, etc.

class ACCESS_TIME_BOUNDARY
{
public:
    enum
    {
        BLOCK_UNKNOWN = -1,
        BLOCK_UNBLOCKED = 0,
        BLOCK_BLOCKED = 1
    };

    int nBlock;
    uint64 nTime1;
    uint64 nTime2;

    ACCESS_TIME_BOUNDARY() : nBlock(BLOCK_UNKNOWN), nTime1(0), nTime2(0xffffffffffffffffULL) {}

    bool SetValue(bool block, uint64 startTime, uint64 endTime);

    ACCESS_TIME_BOUNDARY& operator=(const ACCESS_TIME_BOUNDARY& boundaryInfo);
    bool operator==(const ACCESS_TIME_BOUNDARY& boundaryInfo) const;
    bool operator!=(const ACCESS_TIME_BOUNDARY& boundaryInfo) const;

    static int FlipBlockState(int block);
    static const char* Enum2String(int block);
};

class IReceiverControl;
class CDecoderDiagnostics;
class IDecoder;

class CAccessControl
{
private:
    static Lockable         AccessControlCountersLock;
    static AccessControlMap AccessControlCounters;
public:
    static int  CheckAccessControl(uint32 pipeIdN, IDecoder* vidDecoder);
    static bool SkipAccessControlCheck(uint32 pipeIdN, int* pBlocked);
    static void ResetSkipCounter(void);
    static void InitAccessControl(uint32 pipeIdN);

public:
    CAccessControl(uint32 pipeIdN, bool isVideo, bool isAudioDescription) : mPipeIdN(pipeIdN), IsVideo(isVideo), IsAudioDescription(isAudioDescription) {}
    bool SetAccessControl(const ACCESS_TIME_BOUNDARY& boundary);
    int  CheckTimeBoundary(uint64 currentNtp, int& last_block, IReceiverControl* receiverControl, CDecoderDiagnostics& decoderDiagnostics);

private:
    //Pipe id
    uint32                 mPipeIdN;

    bool                   IsVideo;
    bool                   IsAudioDescription;
    Lockable               BoundaryInfoLock;
    ACCESS_TIME_BOUNDARY   BoundaryInfo;
};

// ===============================================================================================================
// ===============================================================================================================
