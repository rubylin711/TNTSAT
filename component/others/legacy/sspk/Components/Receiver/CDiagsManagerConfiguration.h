///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

// ===============================================================================================================
// Different diagnostics levels
// ===============================================================================================================

enum eDiagsLevel
{
    kDiagsSevere = 0,
    kDiagsCritical = 1,
    kDiagsError = 2,
    kDiagsInformational = 3,
    kDiagsDebug = 4,
};

// ===============================================================================================================
// Global configuration parameters for diags manager
// ===============================================================================================================

class CDiagsManagerConfiguration
{
public:
    //Constructor
    CDiagsManagerConfiguration();

public:
    //Process commands
    bool        Command(const std::string& command, const std::vector<std::string>& args);

public:
    //Diagnostics level
    eDiagsLevel DiagsLevel;
    //Send first AV diagnostics update after so many milliseconds
    int32       DiagsFirstUpdateAfterTuneTime;
    //Periodically send diagnostics update after so many milliseconds
    int32       DiagsPeriodicUpdateAfterTime;
};

// ===============================================================================================================
// ===============================================================================================================

extern CDiagsManagerConfiguration gDiagsManagerConfiguration;

// ===============================================================================================================
// ===============================================================================================================
