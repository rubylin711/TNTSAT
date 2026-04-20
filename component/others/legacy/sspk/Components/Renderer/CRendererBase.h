///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/// <summary>
///     CRendererBase.h
///     Provides base implementation of IRenderer interface
/// </summary>

#pragma once

#include "IRenderer.h"
#include "IReceiverControl.h"
#include "CRendererState.h"
#include "CReceiverDiagnostics.h"
#include "Clock.h"
#include "IHalDecoder.h"
#include "IDecoder.h"
#include "Buffer.h"

// ===============================================================================================================
// ===============================================================================================================

class CRendererBase : public IRenderer
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    CRendererBase(IReceiverControl* receiverControl)
        : mReceiverControl(receiverControl)
        , mRendererState(mReceiverControl->GetRendererState())
        , mClock(mReceiverControl->GetClock())
        , mPipeIdN(mRendererState.mPipeIdN)
    {}
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~CRendererBase() {}
    /// <summary>
    /// Implements <see cref="IRenderer.Initialize">IRenderer::Initialize</see>
    /// Initializes the renderer based on tune request
    /// </summary>
    /// <param name="tuneRequest"></param>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override void        Initialize(CTuneRequest& tuneRequest) {}
    /// <summary>
    /// Implements <see cref="IRenderer.OnSync">IRenderer::OnSync</see>
    /// Resynchronise downstream components. Also forces a microcode reload if so requested.
    /// </summary>
    /// <param name="sync"></param>
    /// <param name="cleanStall"></param>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override void        OnSync(bool sync, bool cleanStall) {}
    /// <summary>
    /// Implements <see cref="IRenderer.SetSecondaryAudioPreference">IRenderer.SetSecondaryAudioPreference</see>
    /// Set SAP selection - sets appropriate parameters for sap selection
    /// </summary>
    /// <param name="void"></param>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override void        SetSecondaryAudioPreference(void) {}
    /// <summary>
    /// Implements <see cref="IRenderer.UpdateSubtitleLanguage">IRenderer.UpdateSubtitleLanguage</see>
    /// Update the subtitle language downstream
    /// </summary>
    /// <param name="void"></param>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override void        UpdateSubtitleLanguage(void) {}
    /// <summary>
    /// Implements <see cref="IRenderer.Command">IRenderer::Command</see>
    /// Generic interface to send custom commands
    /// </summary>
    /// <param name="args">[IN] </param>
    /// <return>
    /// <para>Returns true if the command was processed</para>
    /// </return>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override bool        Command(const std::string& command, const std::vector<std::string>& args) { return false; }
    /// <summary>
    /// Implements <see cref="IRenderer.OnFirstPacket">IRenderer::OnFirstPacket</see>
    /// Pre-processing for the first packet
    /// </summary>
    /// <param name="packet">[IN] </param>
    /// <return>
    /// <para>Returns true is successful</para>
    /// </return>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override void        OnFirstPacket(IPacket& packet) {}
    /// <summary>
    /// Implements <see cref="IRenderer.ReceivePacket">IRenderer::ReceivePacket</see>
    /// Pre-process the packet
    /// </summary>
    /// <param name="packet">[IN] </param>
    /// <return>
    /// <para>Returns true is successful</para>
    /// </return>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override bool        ReceivePacket(IPacket& packet) { return true; }
    /// <summary>
    /// Implements <see cref="IRenderer.ProcessPacket">IRenderer::ProcessPacket</see>
    /// Process the packet downstream
    /// </summary>
    /// <param name="packet">[IN] </param>
    /// <return>
    /// <para>Returns false when packet needs to be retried</para>
    /// </return>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override bool        ProcessPacket(IPacket& packet) { return true; }
    /// <summary>
    /// Implements <see cref="IRenderer.PauseLiveParameters">IRenderer::PauseLiveParameters</see>
    /// Returns url parameters to be used for pausing live streams
    /// </summary>
    /// <return>
    /// <para>Returns url parameters to be used for pausing live streams</para>
    /// </return>
    /// <seealso cref="IRenderer">IRenderer</seealso>
    __override std::string PauseLiveParameters(void) { return ""; }

protected:
    //Receiver control APIs
    IReceiverControl*      mReceiverControl;
    //Current rendering state
    CRendererState&        mRendererState;
    //Clock to use
    Clock&                 mClock;
    //Pipe id
    uint32                 mPipeIdN;
};

// ===============================================================================================================
// ===============================================================================================================
