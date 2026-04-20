///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/// <summary>
///     IRenderer.h
///     Provides interface for managing downstream components for rendering
///     different types of stream containers and controlling behavior of those
///     components...
/// </summary>

#pragma once

#include <string>
#include <vector>

// ===============================================================================================================
// ===============================================================================================================

class IReceiverControl;
class CTuneRequest;
class IPacket;

// ===============================================================================================================
// ===============================================================================================================

class IRenderer
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~IRenderer() {}
    /// <summary>
    /// Initializes the renderer based on tune request
    /// </summary>
    /// <param name="tuneRequest"></param>
    virtual void           Initialize(CTuneRequest& tuneRequest) = 0;
    /// <summary>
    /// Resynchronise downstream components.
    /// Also forces a microcode reload if so requested.
    /// </summary>
    /// <param name="sync"></param>
    /// <param name="cleanStall"></param>
    virtual void           OnSync(bool sync, bool cleanStall) = 0;
    /// <summary>
    /// Set SAP selection - sets appropriate parameters for sap selection
    /// </summary>
    /// <param name="void"></param>
    virtual void           SetSecondaryAudioPreference(void) = 0;
    /// <summary>
    /// Update the subtitle language downstream
    /// </summary>
    /// <param name="void"></param>
    virtual void           UpdateSubtitleLanguage(void) = 0;
    /// <summary>
    /// Generic interface to send custom commands
    /// </summary>
    /// <param name="args">[IN] </param>
    /// <return>
    /// <para>Returns true if the command was processed</para>
    /// </return>
    virtual bool           Command(const std::string& command, const std::vector<std::string>& args) = 0;
    /// <summary>
    /// Pre-processing for the first packet
    /// </summary>
    /// <param name="packet">[IN] </param>
    virtual void           OnFirstPacket(IPacket& packet) = 0;
    /// <summary>
    /// Pre-process the packet
    /// </summary>
    /// <param name="packet">[IN] </param>
    /// <return>
    /// <para>Returns true is successful</para>
    /// </return>
    virtual bool           ReceivePacket(IPacket& packet) = 0;
    /// <summary>
    /// Process the packet downstream
    /// </summary>
    /// <param name="packet">[IN] </param>
    /// <return>
    /// <para>Returns false when packet needs to be retried</para>
    /// </return>
    virtual bool           ProcessPacket(IPacket& packet) = 0;
    /// <summary>
    /// Returns url parameters to be used for pausing live streams
    /// </summary>
    /// <return>
    /// <para>Returns url parameters to be used for pausing live streams</para>
    /// </return>
    virtual std::string    PauseLiveParameters(void) = 0;
};

// ===============================================================================================================
// ===============================================================================================================

class IRendererFactory
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~IRendererFactory() {}
    /// <summary>
    /// Acquires a renderer which processes incoming streams
    /// </summary>
    /// <param name="packet">[IN] </param>
    /// <param name="renderer">[IN] </param>
    /// <return>
    /// <para>Returns a renderer instance</para>
    /// </return>
    virtual IRenderer* AcquireRenderer(IPacket& packet, IReceiverControl* receiverControl) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
