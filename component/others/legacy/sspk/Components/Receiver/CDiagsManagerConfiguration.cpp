///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CDiagsManagerConfiguration.h"
#include "Trace.h"
#include <string>
#include <vector>
using namespace std;

// ===============================================================================================================
// Global configuration parameters common to all components and paths
// ===============================================================================================================

CDiagsManagerConfiguration::CDiagsManagerConfiguration()
{
    //Diagnostics level
    DiagsLevel = kDiagsSevere;
    //Send first AV diagnostics update after so many milliseconds (default is 1 minute)
    DiagsFirstUpdateAfterTuneTime = 60000;
    //Periodically send diagnostics update after so many milliseconds (default is 2 minutes)
    DiagsPeriodicUpdateAfterTime = 120000;
}

//Process commands
bool CDiagsManagerConfiguration::Command(const string& command, const vector<string>& args)
{
    //Number of arguments
    uint32 numargs = (uint32)args.size();

    //Diagnostics level
    if (command == "avdiagslevel")
    {
        if (numargs == 1)
        {
            DiagsLevel = (eDiagsLevel)atoi(args[0].c_str());

            TRACE(("DiagsLevel=%u", DiagsLevel));
        }
        return true;
    }

    //Send first AV diagnostics update after so many milliseconds
    if (command == "diagnosticsupdatefirsttime")
    {
        if (numargs == 1)
        {
            DiagsFirstUpdateAfterTuneTime = atoi(args[0].c_str());

            TRACE(("DiagsFirstUpdateAfterTuneTime=%u", DiagsFirstUpdateAfterTuneTime));
        }
        return true;
    }

    //Periodically send diagnostics update after so many milliseconds
    if (command == "diagnosticsupdateperiodic")
    {
        if (numargs == 1)
        {
            DiagsPeriodicUpdateAfterTime = atoi(args[0].c_str());

            TRACE(("DiagsPeriodicUpdateAfterTime=%u", DiagsPeriodicUpdateAfterTime));
        }
        return true;
    }

    return false;
}

// ===============================================================================================================
// ===============================================================================================================

CDiagsManagerConfiguration gDiagsManagerConfiguration;

// ===============================================================================================================
// ===============================================================================================================
