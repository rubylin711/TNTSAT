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
// ===============================================================================================================

/// <summary>
/// Do some work on timeslice
///
/// Note that the work is usually done on a singleton thread and is time sensitive
/// so the providers are expected to consume very light weight work when called.
/// </summary>
class ITimeslice
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~ITimeslice() {}
    /// <summary>
    /// Do some work
    /// </summary>
    virtual void OnTimeslice(void) = 0;
};

// ===============================================================================================================
// ===============================================================================================================

/// <summary>
/// Provides timeslicing capabilities to subscribers
/// </summary>
class ITimesliceManager
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~ITimesliceManager() {}
    /// <summary>
    /// Process commands
    /// </summary>
    virtual bool    Command(const std::string& command, const std::vector<std::string>& args) = 0;
    /// <summary>
    // Register for timeslice
    /// </summary>
    virtual void    Register(ITimeslice* decoder) = 0;
    /// <summary>
    /// Unregister from timeslice
    /// </summary>
    virtual void    UnRegister(ITimeslice* decoder) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
