///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IDiagsEvent.h"

// ===============================================================================================================
// ===============================================================================================================

/// <summary>
/// IDiagsProvider interface used by most objects to extract diagnostics information periodically.
/// Any object wishing to provide periodic information should inherit from this interface.
/// </summary>
class IDiagsProvider
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    IDiagsProvider() {}
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~IDiagsProvider() {}
    /// <summary>
    /// Called by the diags manager to initialize the diags provider when attached
    /// </summary>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    virtual void DiagsInit(void);
    /// <summary>
    /// Implements <see cref="IDiags.DiagsReset">IDiags.DiagsReset</see>
    /// </summary>
    /// <param name="void"></param>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    virtual void DiagsReset(void);
    /// <summary>
    /// Implements <see cref="IDiags.DiagsRetrieve">IDiags.DiagsRetrieve</see>
    /// </summary>
    /// <param name="diagsEvent"></param>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    virtual void DiagsRetrieve(IDiagsEvent* diagsEvent) = 0;
    /// <summary>
    /// Implements <see cref="IDiags.DiagsRetrieve">IDiags.DiagsRetrieve</see>
    /// </summary>
    /// <param name="void"></param>
    /// <return>
    /// <para>Returns IDiagsEvent* type</para>
    /// </return>
    virtual IDiagsEvent* DiagsRetrieve(void) = 0;

    /// <summary>
    /// Called periodically by the diagnostics manager to retrieve diagnostic event from this provider.
    /// It can return NULL if it is not yet time to send the periodic update from this provider.
    /// </summary>
    /// <param name="void"></param>
    /// <return>
    /// <para>Returns <see cref="IDiagsEvent">IDiagsEvent*</see> type</para>
    /// </return>
    IDiagsEvent* DiagsGeneratePeriodicEvent(int32 ticks);

private:
    //Used internally to manage the periodic nature of retrieving the diagnostic events
    int _diagsNextSendTime;
};

// ===============================================================================================================
// ===============================================================================================================
