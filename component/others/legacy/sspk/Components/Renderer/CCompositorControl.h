///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IDecoder.h"
#include "AutoLock.h"
#include <map>

// ===============================================================================================================
// CCompositorControl - works with MediaRoom compositor for various AV texture composition
// ===============================================================================================================

class CCompositorControl
{
private:
    //Managing list of active decoders which have useful information to provide
    //to compositor
    //
    //For example, all decoders which could provide a video frame or texture are
    //expected to register themselves.  The currently active audio decoder registers
    //itself because it could provide other interesting information like the current
    //audio video synchronization or other diagnostics information.
    //
    //Decoders are maintained in the list by means of a unique pipe id.
    //For example, the video decoders retain the pipe ids of the originating rendering
    //pipe to which they belong so that the compositor can discover them (these include
    //FULLSCREEN as well as PIPs).  The pipe ids for closed captioning, DVB subtitle
    //and teletext decoders are same as the managed application uses. Currently active
    //audio decoder uses "AUDIO" as pipe id.  Note that there is one and only one active
    //audio decoder which is expected to be rendering audio at any time.
    //
    //List of active decoders which have useful information to provide to conpositor
    static std::map<uint32,IDecoder*>  ActiveDecoders;
    //Protecting the active decoders map
    static Lockable                    ActiveDecodersLock;
public:
    //Save given decoder into decoder map
    static void                        RegisterDecoder(uint32 pipeIdN, IDecoder* decoder, LPVOID pDecoderContext = NULL);
    //Remove given decoder from decoder map
    static void                        UnRegisterDecoder(uint32 pipeIdN, const IDecoder* decoder, LPVOID pDecoderContext = NULL);

    //Notifies the compositor of fullscreen block state at video decoder instantiation time
    static void                        NotifyFullscreenBlockStatus(bool bBlock);

    //Direct communication with decoders
    static DECODER_ERR                 IoControl(uint32 pipeIdN, DECODER_CONTROL control, void* inBuf=NULL, int inSize=0, void** outBuf=NULL, int outSize=0);

    //Check access control for given pipe
    //Return true if access is blocked
    static bool                        IsBlocked(uint32 pipeIdN);

    //Is this a hardware rendererd PIP
    static bool                        IsPipHardwareRendered(uint32 pipeIdN);

    //Has PIP decoder rendererd first frame
    static bool                        HasRenderedFirstPipFrame(uint32 pipeIdN);

    //Hardware rendered PIP/FPIP
    //
    //For PC/Carbon
    // Never comes here
    //On SOC
    // FPIP and PIP come here.
    // SUBTITLE, CC, TELETEXT go to GetNextFrame
    static void                        ShowHardwarePip(uint32 pipeIdN, RECT* pRect);

    //Software rendered PIP/FPIP
    //
    //For PC/Carbon
    // Never comes here
    //On SOC
    // FPIP and PIP come here.
    // SUBTITLE, CC, TELETEXT go to GetNextFrame
    static void                        DisplayNextPipFrame(uint32 pipeIdN, RECT* pRect, int changeflags, void** ppContext);

    //Textures
    //
    //For PC/Carbon
    //  FULLSCREEN, FPIP, PIP, SUBTITLE, CC, TELETEXT all come here
    //For SOC
    //  Only SUBTITLE, CC, TELETEXT come here
    //  FULLSCREEN, FPIP, PIP go to DisplayNextPipFrame
    static void*                       GetNextFrame(uint32 pipeIdN);

    //Notify teletext decoder when screen aspect ratio changes
    static DECODER_ERR                 NotifyDisplayChange(uint width, uint height, bool is16x9);
};

// ===============================================================================================================
// Decoder access by pipe name
// ===============================================================================================================

extern DECODER_ERR Decoder_IoControl(uint32 uiPipeIdN, int control, void* inBuf=NULL, int inSize=0, void** outBuf=NULL, int outSize=0);

// ===============================================================================================================
// ===============================================================================================================
