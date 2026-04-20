///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

// ===============================================================================================================
// ISpliceControl APIs
//
// CReceiver is expected to expose APIs to be used by CSpliceEngine to control stream splicing
// ===============================================================================================================

class ISpliceControl
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~ISpliceControl() {}
    /// <summary>
    /// Stop receiver for a stream splice
    /// </summary>
    virtual void StopForSplice(void) = 0;
    /// <summary>
    /// Preflight next Ad
    /// </summary>
    virtual bool PreflightNextAd(void) = 0;
    /// <summary>
    /// Detunes from primary stream
    /// </summary>
    virtual void DetuneFromPrimary(void) = 0;
    /// <summary>
    /// Instaniates secondary socket
    /// </summary>
    virtual bool TuneToSecondary(std::string& secondaryUrl) = 0;
    /// <summary>
    /// Attaches receiver to secondary socket
    /// </summary>
    virtual void JoinSecondary(void) = 0;
    /// <summary>
    /// Detunes from a secondary stream
    /// </summary>
    virtual void DetuneFromSecondary(void) = 0;
    /// <summary>
    /// Tune back to primary stream with additional parameters supplied
    /// </summary>
    virtual bool TuneBackToPrimary(std::string& urlParameters) = 0;
    /// <summary>
    /// Resets the renderer in preparation for next stream being spliced
    /// </summary>
    virtual void ResetForSplice(void) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
