///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CReceiverDiags.h"
#include "DrmDefinitions.h"
#include <string>
using namespace std;

// ===============================================================================================================
// Tune timing event
// ===============================================================================================================

const WCHAR* CDiagsReceiverTuneTimingEvent::ActionStrings[] =
{
    L"AV.Timing.Unknown",
    L"AV.Timing.Pause",
    L"AV.Timing.Play",
    L"AV.Timing.Step",
    L"AV.Timing.FastForward",
    L"AV.Timing.Rewind",
    L"AV.Timing.SkipForward",
    L"AV.Timing.SkipBackward",
    L"AV.Timing.Scan",
    L"AV.Timing.SkipPlaylistItem",
    L"AV.Timing.PlayFromBeginning",
    L"AV.Timing.PlayLive",
    L"AV.Timing.PlayAt",
    L"AV.Timing.Stop",
    L"AV.Timing.FrontTruncation",
    L"AV.Timing.TuneToLive",
    L"AV.Timing.TuneToDvb",
    L"AV.Timing.TuneToDvr",
    L"AV.Timing.TuneToVod",
    L"AV.Timing.TuneToWMS",
    L"AV.Timing.TuneToMP3",
    L"AV.Timing.TuneToMP4",
    L"AV.Timing.TuneToTimeShiftLive",
    L"AV.Timing.TuneToTimeShiftRange",
    L"AV.Timing.TuneToMpegTS",
    L"AV.Timing.TuneToTS",
    L"AV.Timing.TuneToSmoothStreaming",
};

void CDiagsReceiverTuneTimingEvent::DiagsGetEventData(void)
{
    // Do base class implementation first
    CDiagsReceiverEvent::DiagsGetEventData();

    if (Action == TimingAction_TuneToDvb ||
        Action == TimingAction_TuneToMP3 ||
        Action == TimingAction_TuneToMP4 ||
        Action == TimingAction_TuneToMpegTS ||
        Action == TimingAction_TuneToTS)
    {
        DiagsLogValue(L"Total"        , Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_Instantiate        ]);
        DiagsLogValue(L"App"          , Timings[TimingSnapshotAt_TuneFromApp        ] - Timings[TimingSnapshotAt_Instantiate        ]);

        DiagsLogValue(L"Prepare"      , Timings[TimingSnapshotAt_BeforePreflight    ] - Timings[TimingSnapshotAt_TuneFromApp        ]);

        TRACE(("TuneTime[%08X][%ls] [Total:%d][App:%d] [Prepare:%d]",
            PipeIdN, ActionStrings[Action],

            Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_Instantiate        ], //Transport instantiation to clock start
            Timings[TimingSnapshotAt_TuneFromApp        ] - Timings[TimingSnapshotAt_Instantiate        ], //Transport instantiation to tune by app

            Timings[TimingSnapshotAt_BeforePreflight    ] - Timings[TimingSnapshotAt_TuneFromApp        ]  //Preparation
            ));
    }
    else
    if (Action == TimingAction_TuneToLive ||
        Action == TimingAction_TuneToDvr ||
        Action == TimingAction_TuneToTimeShiftLive ||
        Action == TimingAction_TuneToTimeShiftRange)
    {
        DiagsLogValue(L"Total"        , Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_Instantiate        ]);
        DiagsLogValue(L"App"          , Timings[TimingSnapshotAt_TuneFromApp        ] - Timings[TimingSnapshotAt_Instantiate        ]);

        DiagsLogValue(L"LoadKeys"     , Timings[TimingSnapshotAt_TuneStep1          ] - Timings[TimingSnapshotAt_TuneFromApp        ]);
        DiagsLogValue(L"KeysLoaded"   , Timings[TimingSnapshotAt_BeforePreflight    ] - Timings[TimingSnapshotAt_TuneStep1          ]);

        TRACE(("TuneTime[%08X][%ls] [Total:%d][App:%d] [LoadKeys:%d][KeysLoaded:%d]",
            PipeIdN, ActionStrings[Action],

            Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_Instantiate        ], //Transport instantiation to clock start
            Timings[TimingSnapshotAt_TuneFromApp        ] - Timings[TimingSnapshotAt_Instantiate        ], //Transport instantiation to tune by app

            Timings[TimingSnapshotAt_TuneStep1          ] - Timings[TimingSnapshotAt_TuneFromApp        ], //Initiate loading of DRM keys
            Timings[TimingSnapshotAt_BeforePreflight    ] - Timings[TimingSnapshotAt_TuneStep1          ]  //Wait for DRMkeys to be loaded
            ));
    }
    else
    if (Action == TimingAction_TuneToVod)
    {
        DiagsLogValue(L"Total"        , Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_Instantiate        ]);
        DiagsLogValue(L"App"          , Timings[TimingSnapshotAt_TuneFromApp        ] - Timings[TimingSnapshotAt_Instantiate        ]);

        DiagsLogValue(L"LoadKeys"     , Timings[TimingSnapshotAt_TuneStep1          ] - Timings[TimingSnapshotAt_TuneFromApp        ]);
        DiagsLogValue(L"MapServer"    , Timings[TimingSnapshotAt_TuneStep3          ] - Timings[TimingSnapshotAt_TuneStep2          ]);
        DiagsLogValue(L"Descriptor"   , Timings[TimingSnapshotAt_TuneStep4          ] - Timings[TimingSnapshotAt_TuneStep3          ]);
        DiagsLogValue(L"IndexFile"    , Timings[TimingSnapshotAt_TuneStep5          ] - Timings[TimingSnapshotAt_TuneStep4          ]);
        DiagsLogValue(L"KeysLoaded"   , Timings[TimingSnapshotAt_BeforePreflight    ] - Timings[TimingSnapshotAt_TuneStep5          ]);

        TRACE(("TuneTime[%08X][%ls] [Total:%d][App:%d] [LoadKeys:%d][MapServer:%d][Descriptor:%d][IndexFile:%d][KeysLoaded:%d]",
            PipeIdN, ActionStrings[Action],

            Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_Instantiate        ], //Transport instantiation to clock start
            Timings[TimingSnapshotAt_TuneFromApp        ] - Timings[TimingSnapshotAt_Instantiate        ], //Transport instantiation to tune by app

            Timings[TimingSnapshotAt_TuneStep1          ] - Timings[TimingSnapshotAt_TuneFromApp        ], //Initiate loading of DRM keys
            Timings[TimingSnapshotAt_TuneStep3          ] - Timings[TimingSnapshotAt_TuneStep2          ], //Contact VOD map server
            Timings[TimingSnapshotAt_TuneStep4          ] - Timings[TimingSnapshotAt_TuneStep3          ], //Download descriptor file
            Timings[TimingSnapshotAt_TuneStep5          ] - Timings[TimingSnapshotAt_TuneStep4          ], //Index file
            Timings[TimingSnapshotAt_BeforePreflight    ] - Timings[TimingSnapshotAt_TuneStep5          ]  //Wait for DRM keys to be loaded
            ));
    }
    else
    if (Action == TimingAction_TuneToSmoothStreaming ||
        Action == TimingAction_TuneToWMS)
    {
        bool bSmoothStreaming = (Action == TimingAction_TuneToSmoothStreaming);
        uint32 ticksEndPrepare = bSmoothStreaming ? Timings[TimingSnapshotAt_TuneStep1] : Timings[TimingSnapshotAt_BeforePreflight];

        uint32 timeTotal    = Timings[TimingSnapshotAt_ClockStarted] - Timings[TimingSnapshotAt_Instantiate];
        uint32 timeApp      = Timings[TimingSnapshotAt_TuneFromApp ] - Timings[TimingSnapshotAt_Instantiate];
        uint32 timePrepare  = ticksEndPrepare                        - Timings[TimingSnapshotAt_TuneFromApp];

        DiagsLogValue(L"Total", timeTotal);
        DiagsLogValue(L"App", timeApp);
        DiagsLogValue(L"Prepare", timePrepare);

        uint32 timeManifest = 0;
        uint32 timeDrm = 0;
        uint32 timeLicense = 0;
        uint32 timeProcessResponse = 0;

        const int DRM_BASE = TimingSnapshotAt_TuneStep3;
        
        if (Timings[DRM_BASE + DrmLicenseTimes_Start] && Timings[DRM_BASE + DrmLicenseTimes_End])
        {
            timeManifest        = Timings[DRM_BASE + DrmLicenseTimes_Start          ] - ticksEndPrepare;
            timeDrm             = Timings[DRM_BASE + DrmLicenseTimes_End            ] - Timings[DRM_BASE + DrmLicenseTimes_Start            ];
            timeLicense         = Timings[DRM_BASE + DrmLicenseTimes_GetLicense     ] - Timings[DRM_BASE + DrmLicenseTimes_GenerateChallenge];
            timeProcessResponse = Timings[DRM_BASE + DrmLicenseTimes_ProcessResponse] - Timings[DRM_BASE + DrmLicenseTimes_GetLicense       ];

            DiagsLogValue(L"License", timeLicense);
            DiagsLogValue(L"ProcessResponse", timeProcessResponse);
        }
        else if (bSmoothStreaming)
        {
            timeManifest = Timings[TimingSnapshotAt_TuneStep2      ] - ticksEndPrepare;
            timeDrm      = Timings[TimingSnapshotAt_BeforePreflight] - Timings[TimingSnapshotAt_TuneStep2];
        }

        DiagsLogValue(L"Drm", timeDrm);
        if (bSmoothStreaming)
        {
            //First packet isn't reported for smooth streaming, so just use IFrame time
            if (Timings[TimingSnapshotAt_ReceivedFirstPacket] == 0)
                Timings[TimingSnapshotAt_ReceivedFirstPacket] = Timings[TimingSnapshotAt_ReceivedFirstIFrame];

            DiagsLogValue(L"Manifest", timeManifest);
        }

        TRACE(("TuneTime[%08X][%ls] [Total:%d][App:%d] [Prepare:%d][Manifest:%d][Drm:%d][License:%d][PR:%d]",
            PipeIdN, ActionStrings[Action],

            Timings[TimingSnapshotAt_ClockStarted] - Timings[TimingSnapshotAt_Instantiate], //Transport instantiation to clock start
            Timings[TimingSnapshotAt_TuneFromApp ] - Timings[TimingSnapshotAt_Instantiate], //Transport instantiation to tune by app

            timePrepare,  //Preparation
            timeManifest, //Downloading manifest
            timeDrm,      //DRM keys loaded

            timeLicense,        //Time to fetch license from server
            timeProcessResponse //Time to process license response
            ));
    }
    else
    {
        DiagsLogValue(L"Total", Timings[TimingSnapshotAt_ClockStarted] - Timings[TimingSnapshotAt_TuneFromApp]);

        TRACE(("TuneTime[%08X][%ls] [Total:%d]",
            PipeIdN, ActionStrings[Action],

            Timings[TimingSnapshotAt_ClockStarted] - Timings[TimingSnapshotAt_TuneFromApp]  //Trick action to start of clock
            ));
    }
    //Common stuff
    {
        DiagsLogValue(L"Preflight"    , Timings[TimingSnapshotAt_AfterPreflight     ] - Timings[TimingSnapshotAt_BeforePreflight    ]);
        DiagsLogValue(L"TuneFinish"   , Timings[TimingSnapshotAt_DetuneTunerSession ] - Timings[TimingSnapshotAt_AfterPreflight     ]);
        DiagsLogValue(L"Tune"         , Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_DetuneTunerSession ]);

        DiagsLogValue(L"Detune"       , Timings[TimingSnapshotAt_SetupTunerSession  ] - Timings[TimingSnapshotAt_DetuneTunerSession ]);

        DiagsLogValue(L"NativeTune"   , Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_TuneTunerSession   ]);
        DiagsLogValue(L"ToConnect"    , Timings[TimingSnapshotAt_Connecting         ] - Timings[TimingSnapshotAt_TuneTunerSession   ]);
        DiagsLogValue(L"ToConnected"  , Timings[TimingSnapshotAt_Connected          ] - Timings[TimingSnapshotAt_Connecting         ]);
        DiagsLogValue(L"ToFirstPacket", Timings[TimingSnapshotAt_ReceivedFirstPacket] - Timings[TimingSnapshotAt_Connected          ]);
        DiagsLogValue(L"ToFirstIFrame", Timings[TimingSnapshotAt_ReceivedFirstIFrame] - Timings[TimingSnapshotAt_ReceivedFirstPacket]);
        DiagsLogValue(L"ToClockStart" , Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_ReceivedFirstPacket]);

        TRACE(("TuneTime[%08X][%ls] [Preflight:%d][TuneFinish:%d][Tune:%d] [Detune:%d] [NativeTune:%d (%d %d %d %d %d)]",
            PipeIdN, ActionStrings[Action],

            Timings[TimingSnapshotAt_AfterPreflight     ] - Timings[TimingSnapshotAt_BeforePreflight    ], //Preflight
            Timings[TimingSnapshotAt_DetuneTunerSession ] - Timings[TimingSnapshotAt_AfterPreflight     ], //Final tune process
            Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_DetuneTunerSession ], //Start of transport tune process to clock start

            Timings[TimingSnapshotAt_SetupTunerSession  ] - Timings[TimingSnapshotAt_DetuneTunerSession ], //Time taken to Detune

            Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_TuneTunerSession   ], //Tune to start of clock
            Timings[TimingSnapshotAt_Connecting         ] - Timings[TimingSnapshotAt_TuneTunerSession   ], //Tune to connect
            Timings[TimingSnapshotAt_Connected          ] - Timings[TimingSnapshotAt_Connecting         ], //Connect to connected
            Timings[TimingSnapshotAt_ReceivedFirstPacket] - Timings[TimingSnapshotAt_Connected          ], //Connected to first packet
            Timings[TimingSnapshotAt_ReceivedFirstIFrame] - Timings[TimingSnapshotAt_ReceivedFirstPacket], //First packet to first I-frame
            Timings[TimingSnapshotAt_ClockStarted       ] - Timings[TimingSnapshotAt_ReceivedFirstPacket]  //First packet to start of clock
            ));
    }
}

const WCHAR* CDiagsReceiverTuneTimingEvent::DiagsGetEventMessage(void)
{
    ASSERT(Action >= TimingAction_Unknown && Action < TimingAction_Max);
    return ActionStrings[Action >= TimingAction_Unknown && Action < TimingAction_Max ? Action : TimingAction_Unknown];
}

// ===============================================================================================================
// ===============================================================================================================
