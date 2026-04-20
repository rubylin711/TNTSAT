///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IAVManager.h"
#include "IDecoder.h"
#include "CHalDecoder.h"
#include "IReceiverControl.h"
#include "CRendererState.h"
#include "CReceiverDiagnostics.h"
#include "CDecoderConfiguration.h"
#include "CStreamInfo.h"
#include "Decoder_Audio.h"
//#include "Decoder_DFXP.h"
#include "Decoder_Video.h"
#include "IPTVDecoderHal.h"
#include "Trace.h"
#include "IXDrm.h"

// ===============================================================================================================
// IDecoderFactory - manages instantiation/destruction and other interactions compositor
// ===============================================================================================================

class CDecoderFactory : public IDecoderFactory
{
public:
    //Constructor
    CDecoderFactory(IAVManager* avManager)
        : mAVManager(avManager)
        , mHDCPAction(XDRM_OPL_UNKNOWN)
        , mDisableComponentVideo(false)
    {
    }

    //Destructor
    virtual ~CDecoderFactory() {}
    //Allocate a decoder based on the given elementary stream information
    __override IDecoder* AcquireDecoder(IReceiverControl* receiverControl, const CStreamInfo& si)
    {
        CRendererState& rendererState = receiverControl->GetRendererState();

        (void)rendererState; // prevent unused warning in release build

        CReceiverDiagnostics& diagnostics = receiverControl->GetDiagnostics();
        IDecoder* decoder = NULL;
        bool supported = true;

        switch (si.Format)
        {
        case StreamType_Video:
        case StreamType_Video_Constrained:
        case StreamType_Video_AVC:
        case StreamType_Video_VC1:
        case StreamType_Video_WMV9:
            supported = gDecoderConfiguration.IsEnabled(si.Format);
            diagnostics.OnUnsupportedCodec(si, supported);
            if (!supported)
            {
                TRACE_ERROR(("[%08x] Unsupported video codec with stream type %d", rendererState.mPipeIdN, si.Format));
                break;
            }

            decoder = new DecoderVideo(receiverControl, si);
            CHECK_ALLOC(decoder);
            break;

        case StreamType_Audio_11172:
        case StreamType_Audio_13818_3:
        case StreamType_Audio_AC3:
        case StreamType_Audio_EnhancedAC3:
        case StreamType_Audio_AAC:
        case StreamType_Audio_HEAAC:
        case StreamType_Audio_WMA:
        case StreamType_Audio_WMAPRO:
        case StreamType_Audio_WMA_WITH_HEADER:
            supported = gDecoderConfiguration.IsEnabled(si.Format);
            diagnostics.OnUnsupportedCodec(si, supported);
            if (!supported)
            {
                TRACE_ERROR(("[%08x] Unsupported audio codec with stream type %d", rendererState.mPipeIdN, si.Format));
                break;
            }

            decoder = new DecoderAudio(receiverControl, si);
            CHECK_ALLOC(decoder);
            break;

        /* TODO: Support DFXP decoder
        case StreamType_DFXP:
            try
            {
                decoder = new DecoderDFXP(receiverControl, si);
            }
            catch (const dfxp::decoder::DFXPDecoderException&)
            {
                throw DecoderException("Error instantiating DFXP decoder");
            }
            CHECK_ALLOC(decoder);
            break;
        */
        }

        return decoder;
    }

    //Dispose off the given decoder
    __override void DisposeDecoder(IDecoder* decoder)
    {
        if (decoder != NULL)
        {
            //Do not forget to call Release on the decoder
            decoder->Release();
            //Free decoder
            delete decoder;
        }
    }

    //Save given decoder into decoder map
    __override void RegisterDecoder(uint32 pipeIdN, IDecoder* decoder, LPVOID pDecoderContext = NULL) {}
    //Remove given decoder from decoder map
    __override void UnRegisterDecoder(uint32 pipeIdN, const IDecoder* decoder, LPVOID pDecoderContext = NULL) {}
    //Notifies the compositor of fullscreen block state at video decoder instantiation time
    __override void NotifyFullscreenBlockStatus(bool bBlock) {}

    //Enable HDCP (this is required by playready DRM)
    __override void HandleHDCPOutput(uint32 pipeIdN, XDRM_OPL_ACTION action)
    {
        IPTV_HAL_DECODER_DRM_SETOPL oplValue;
        oplValue.pipeIdN = pipeIdN;

        if (mHDCPAction != action)
        {
            mHDCPAction = action;
            TRACE((action != XDRM_OPL_DISABLE ? "Enabling HDCP" : "Disabling HDCP"));

            if ( action == XDRM_OPL_ENABLE_DOWN_RES )
            {
                oplValue.oplType = IPTV_HAL_DECODER_DRM_SETOPL_TYPE_HDCP_ENABLE_DOWN_RES;
            }
            else if ( action == XDRM_OPL_ENABLE_ALWAYS )
            {
                oplValue.oplType = IPTV_HAL_DECODER_DRM_SETOPL_TYPE_HDCP_ENABLE_ALWAYS;;
            }
            else
            {
               oplValue.oplType = IPTV_HAL_DECODER_DRM_SETOPL_TYPE_HDCP_DISABLE; 
            }

            IPTV_HAL_Decoder_SetValue(NULL, IPTV_HAL_DECODER_VALUETYPE_DRM_SETOPL, &oplValue, sizeof(oplValue));
        }
    }

    //Disable component video (this is required by playready DRM)
    __override void HandleComponentVideoOutput(uint32 pipeIdN, bool disable)
    {
        IPTV_HAL_DECODER_DRM_SETOPL oplValue;
        oplValue.pipeIdN = pipeIdN;

        if (mDisableComponentVideo != disable)
        {
            mDisableComponentVideo = disable;
            TRACE((disable ? "Disabling Component Video Output" : "Enabling Component Video Output"));

            oplValue.oplType = disable ? IPTV_HAL_DECODER_DRM_SETOPL_TYPE_COMPONENT_DISABLE :
                                         IPTV_HAL_DECODER_DRM_SETOPL_TYPE_COMPONENT_ENABLE;

            IPTV_HAL_Decoder_SetValue(NULL, IPTV_HAL_DECODER_VALUETYPE_DRM_SETOPL, &oplValue, sizeof(oplValue));
        }
    }

    __override DECODER_ERR IoControl(uint32 pipeIdN, DECODER_CONTROL control, void* inBuf=NULL, int inSize=0, void** outBuf=NULL, int outSize=0)
    {
        return DECODER_ERR_NOT_IMPLEMENTED;
    }

private:
    //Factory of factories
    IAVManager*            mAVManager;

    XDRM_OPL_ACTION        mHDCPAction;
    bool                   mDisableComponentVideo;
};
