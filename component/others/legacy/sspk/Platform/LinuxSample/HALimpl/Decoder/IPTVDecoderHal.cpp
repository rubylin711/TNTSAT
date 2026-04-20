///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// Implementation of IPTVDecoderHal

#include "pkPAL.h"
#include "IPTVDecoderHal.h"

#include "pkExecutive.h"
//#include "CAVRendererGst.h"
#include <pkPAL.h>
#include "CAVRendererDefs.h"

#include "platPrivate.h"

#include <stdlib.h>  //srand; getenv
#include <sys/time.h>

extern "C" {
#include "mt_type.h"
#include "mt_audio_codec.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "drv_adp.h"
#include "HA.AUDIO.G711.codec.h"
#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"
#include "HA.AUDIO.AMRWB.codec.h"
#include "HA.AUDIO.TRUEHDPASSTHROUGH.decode.h"
#include "HA.AUDIO.DOLBYTRUEHD.decode.h"
#include "HA.AUDIO.DTSHD.decode.h"
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
#include "HA.AUDIO.DOLBYPLUS.decode.h"
#endif
#include "HA.AUDIO.AC3PASSTHROUGH.decode.h"
#include "HA.AUDIO.DTSM6.decode.h"

#include "HA.AUDIO.DTSPASSTHROUGH.decode.h"
#include "HA.AUDIO.FFMPEG_DECODE.decode.h"
#include "HA.AUDIO.AAC.encode.h"

}

enum AV_MediaType {
    AV_MEDIA_TYPE_UNKNOWN = -1,  ///< Usually treated as AVMEDIA_TYPE_DATA
    AV_MEDIA_TYPE_VIDEO,
    AV_MEDIA_TYPE_AUDIO,
    AV_MEDIA_TYPE_DATA,          ///< Opaque data information usually continuous
    AV_MEDIA_TYPE_SUBTITLE,
    AV_MEDIA_TYPE_ATTACHMENT,    ///< Opaque data information usually sparse
    AV_MEDIA_TYPE_NB
};


typedef struct
{
    int  streamID;//pSDecoderContext->uiInstance is uint32_t
    int  codec_id;//audio HA_CODEC_ID_E, video mtUNF_VCODEC_TYPE_E
    AV_MediaType codec_type;
	bool isEsEnd;
}  montAV_STREAM_DESCRIPTOR;

montAV_STREAM_DESCRIPTOR g_audio_desc;
montAV_STREAM_DESCRIPTOR g_video_desc;

extern "C" {

extern RET_CODE aud_set_dec_param_vsb(void *p_dev, const void *p_param,int aType);
extern RET_CODE aud_start_vsb(void *p_dev, int type);
extern RET_CODE vdec_start(void *p_dev, int format, int mode);
extern RET_CODE vdec_stop(void *p_dev);
extern RET_CODE vdec_flush();
extern RET_CODE vdec_reset();
extern RET_CODE aud_stop_vsb(void *p_dev);
extern RET_CODE vdec_set_trick_mode_2(mt_s32 playrate);
//extern RET_CODE aud_file_pushesbuffer_vsb(void *p_dev, u32 src_addr, u32 size, u64 apts);
extern RET_CODE aud_file_pushesbuffer_vsb(void *p_dev, u32 src_addr, u32 size, u64 apts, u32 eos);

extern RET_CODE vdec_dec_push_es(void *p_dev, u32 src_addr, u32 size,u64 vpts, int eos_flag);
extern void vdec_get_pts_2(s64 *out_pts);
extern RET_CODE dev_open(void *p_dev, void *p_param);
extern RET_CODE vdec_set_avsync_mode_2(void *p_dev,u32 mode);
extern RET_CODE vdec_set_dec_frm_type(void *p_dev, MT_UNF_DEC_FRM_TYPE_E dec_frm_type);
extern RET_CODE vdec_check_exit();
}

#define ABS(x)     ((x) < 0 ? -(x) : (x))

typedef struct
{
    int last_time;
    IPTV_HAL_DECODER_TIME last_getpts;
    int last_playrate;
    int flush_state; //0 is normal, 1 is flushed, 2 is flush and new pts, 10 set playrate
}  mont_avplay_info;

mont_avplay_info g_avplay_info;


extern MT_HANDLE g_avplay_es_handle;
//100ms
#define MIN_GET_PTS_INTERVAL 100

//#define DUMP_ES_TO_FILE 1

#ifdef DUMP_ES_TO_FILE
FILE *g_file[3] = {NULL};
#endif
// NOTE: Setting the enviornment variable MSPK_DECODER_INHIBIT allows running in terminal session
static const char* c_szEnvVarDecoderInhibit = "MSPK_DECODER_INHIBIT";

static const double c_dblTimescale90KHz = 90000.0;

static const IPTV_HAL_DECODER_TIME c_LogPeriod_RenderClockDiff90kHz = 5 * 60 * 90000; // 5 minutes

// WMA WAVEFORMATEX extensions

// WMAUDIO2 fields:
//  DWORD dwSamplesPerBlock; // only counting "new" samples; half of what will be used due to overlapping
//        [unused]
//  WORD  wEncodeOptions;
          static const size_t wma2ExOffset_wEncodeOptions = sizeof(uint32_t);
//  DWORD dwSuperBlockAlign; // the big size...  should be multiples of wfx.nBlockAlign.
//        [unused]
    static const size_t wma2ExSize = 1 * sizeof(uint16_t) + 2 * sizeof(uint32_t);

// WMAUDIO3 fields:
//  WORD  wValidBitsPerSample; // bits of precision
          static const size_t wma3ExOffset_wValidBitsPerSample = 0;
//  DWORD dwChannelMask;       // which channels are present in stream
          static const size_t wma3ExOffset_dwChannelMask = sizeof(uint16_t);
//  DWORD dwReserved1;
//  DWORD dwReserved2;
//  WORD  wEncodeOptions;
          static const size_t wma3ExOffset_wEncodeOptions = sizeof(uint16_t) + 3 * sizeof(uint32_t);
//  WORD  wReserved3;
    static const size_t wma3ExSize = 3 * sizeof(uint16_t) + 3 * sizeof(uint32_t);


// Platform internal C API for Init/Exit

int IPTV_HAL_Decoder_HALInit_priv()
{
    iptv_hal_error halError = IPTV_HAL_Decoder_HALInit();

    return (IPTV_HAL_ERROR_SUCCESS == halError) ? 0 : (int)halError;
}

int IPTV_HAL_Decoder_HALExit_priv()
{
    iptv_hal_error halError = IPTV_HAL_Decoder_HALExit();

    return (IPTV_HAL_ERROR_SUCCESS == halError) ? 0 : (int)halError;
}

#define GST_MAKE_FOURCC(a,b,c,d)        ((uint32_t)((a)|(b)<<8|(c)<<16|(d)<<24))


#define k_LOG_OUTPUT_PERIOD_MS 500

// Only for running without a decoder:
static const double k_ES_FIFO_SIZE_SECONDS = 8.0;

//#define HAL_TRACE_ENABLE_DECODER

#if defined(HAL_TRACE_ENABLE_DECODER)

#define HAL_DECODER_MSG( printf_exp ) ( Executive_DebugPrintf printf_exp )

#else

#define HAL_DECODER_MSG( printf_exp )   ( void ) 0

#endif

#define HAL_DECODER_ERRMSG( printf_exp ) ( Executive_DebugPrintf printf_exp )

#define HAL_DECODER_OUTPUT( printf_exp ) ( Executive_DebugPrintf printf_exp )



// #define HAL_TRACE_ENABLE_CLOCK

#if defined(HAL_TRACE_ENABLE_CLOCK)

#define HAL_CLOCK_MSG( printf_exp ) ( Executive_DebugPrintf printf_exp )

#else

#define HAL_CLOCK_MSG( printf_exp )   ( void ) 0

#endif

#define HAL_CLOCK_ERRMSG( printf_exp ) ( Executive_DebugPrintf printf_exp )

#define HAL_CLOCK_OUTPUT( printf_exp ) ( Executive_DebugPrintf printf_exp )


// #define HAL_TRACE_ENABLE_CLOCK_GETTIME


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IPTV_HAL_Decoder context structures
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SDecoderClockContext
{
    SDecoderClockContext()
        : dtStart(0)
        , dtEpoch(0)
        , dtStoppedTime(0)
        , isClockRunning(false)
        , uiInstance(0)
        , hAVRendererCtx(NULL)
    {
    }

    // Renderer context for decoders using this clock
    pkHANDLE hAVRendererCtx;

    IPTV_HAL_DECODER_TIME dtStart;
    IPTV_HAL_DECODER_TIME dtEpoch;
    IPTV_HAL_DECODER_TIME dtStoppedTime;
    bool isClockRunning;

    unsigned int uiInstance;
};

struct SDecoderContext
{
    SDecoderContext()
        : pSDecoderClockContext(NULL)
        , hAVRDecoder(NULL)
        , latestPreparePTS(0)
        , latestDTS(0)
        , nextLogOutputDecoderTime(0)
        , uiInstance(0)
        , pInputBufferChain(NULL)
        , pOutputBufferChain(NULL)
        , dtOutputTimestamp(0)
        , uiHeaderLength(0)
        , pvHeaderData(NULL)
        , bIFrameOnly(false)
    {
        memset(&acquireParams, 0, sizeof(acquireParams));
        memset(&pictureInfoValue, 0, sizeof(pictureInfoValue));
    }

    SDecoderClockContext* pSDecoderClockContext;

    pkHANDLE hAVRDecoder;

    IPTV_HAL_DECODER_ACQUIRE acquireParams;

    IPTV_HAL_DECODER_TIME latestPreparePTS;
    IPTV_HAL_DECODER_TIME latestDTS;
    IPTV_HAL_DECODER_TIME nextLogOutputDecoderTime;

    unsigned int uiInstance;

    PIPTV_HAL_BUFFER pInputBufferChain;
    PIPTV_HAL_BUFFER pOutputBufferChain;
    IPTV_HAL_DECODER_TIME dtOutputTimestamp;

    UINT32 uiHeaderLength;
    LPVOID pvHeaderData;

    IPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO_EX  pictureInfoValue;

    UINT32 hDecrypter;
    bool bIFrameOnly;
};

// helper to get system time in 90 khz
static IPTV_HAL_DECODER_TIME s_Get90kHzTime()
{
    struct timeval tvNow;
    uint64_t nowMHz = 0;

    if (0 == gettimeofday(&tvNow, NULL))
    {
        nowMHz = (1000000 * (uint64_t)tvNow.tv_sec) + (uint32_t)tvNow.tv_usec;
    }

    return (IPTV_HAL_DECODER_TIME)(9 * nowMHz / 100);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IPTV_HAL_Decoder
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Static context handle for the AVRenderer
static MT_HANDLE s_hAVRendererCtx = 0;

// Flag to allow actual decoding to be inhibited for ssh session or stub DRM testing
static bool s_IsDecodingInhibited = false;
// a place-holder context for use when s_IsDecodingInhibited is true:
static bool s_noDecoderFakeContext = true;



/// <topic name="Decoder" displayname="Audio/Video Decoder Control APIs"> </topic>

/// <summary>
/// This function is called once when the IPTV application starts up on the box.
/// </summary>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Hal Initialize failed</para>
/// </returns>
/// <remarks>
/// The implementation is expected to initialize the decoder subsystem and get the system into a ready state such that subsequent DecoderHAL APIs can be called.
/// </remarks>

#define DEFAULT_SD_BPS    (100) //Kbytes/sec
#define DEFAULT_HD_BPS    (200)//KBytes/sec
#define DEF_SUPER_HD_BPS    (1800)//KBytes/sec
#define DEFAULT_BUFFERING_SECOND   (3)//default buffer 3 seconds es data

#if 0
typedef enum {
    /*!
    *    player is buffering data
    **/
    MSS_BUFFERING_END,
    /*!
    *    player is buffering data
    **/
    MSS_BUFFERING,


} MSS_BUFFER_STATUS;

static MSS_BUFFER_STATUS buffer_status = MSS_BUFFERING_END;
static  void check_ves_water_level(unsigned int width, unsigned int hight)
{
	u32 cur_ves_wl = 0;
	u32 video_free_size = 0;
	u32 audio_free_size = 0;
	u32 video_total_size = 0;
	u32 audio_total_size = 0;
	u32 start_buffering_wl;
	u32 end_buffering_wl;

	if(width * hight <= (720 * 576)){
		start_buffering_wl = 100;
		end_buffering_wl = start_buffering_wl + DEFAULT_SD_BPS * DEFAULT_BUFFERING_SECOND;
	}else{
		 /*case  1080P*/
		if(width * hight > (1280 * 720)){
			start_buffering_wl = 300;
			end_buffering_wl = start_buffering_wl + DEF_SUPER_HD_BPS;
		}else{/*case  720P*/
			start_buffering_wl = 200;
			end_buffering_wl = start_buffering_wl + DEFAULT_HD_BPS * DEFAULT_BUFFERING_SECOND;
		}
	}

	video_decoder_get_es_buf_space(&video_free_size,&video_total_size,&audio_free_size,&audio_total_size);
	cur_ves_wl = video_total_size - video_free_size;
	//HAL_DECODER_MSG(("cur_ves_wl[%dKB], end_buffering_wl[%dKB], video_free_size[%d] audio_free_size[%d]\n", cur_ves_wl, end_buffering_wl,video_free_size, audio_free_size));
	//Executive_DebugPrintf("cur_ves_wl[%dKB], end_buffering_wl[%dKB], video_free_size[%d] audio_free_size[%d],width:%d,hight:%d\n", cur_ves_wl, end_buffering_wl,video_free_size, audio_free_size,width,hight);
	if(buffer_status == MSS_BUFFERING)
	{
		/*resume decoder*/

		if(cur_ves_wl > end_buffering_wl/* || audio_free_size <= 32/*32 Kbytes*/)
		{
			//HAL_DECODER_MSG(("@@@@current ves count [%ld  KBytes]!!!!\n", cur_ves_wl));
			//HAL_DECODER_MSG((">>>>>>>>>@@@ END BUFFERING <<<<<<<<<\n"));
			Executive_DebugPrintf("@@@@current ves count [%ld  KBytes]!!!!\n", cur_ves_wl);
			Executive_DebugPrintf(">>>>>>>>>@@@ END BUFFERING <<<<<<<<<\\n", cur_ves_wl);
			video_decoder_resume();
			if(g_audio_desc.g_a_dec != NULL){
				audio_decoder_resume(g_audio_desc.g_a_dec);
			}
			buffer_status = MSS_BUFFERING_END;
		}
		return;
	}

	if(buffer_status == MSS_BUFFERING_END && cur_ves_wl < start_buffering_wl/*100 Kbytes*/)
	{
		/*pause decoder*/
		video_decoder_pause();
		if(g_audio_desc.g_a_dec != NULL){
			audio_decoder_pause(g_audio_desc.g_a_dec);

		}
		Executive_DebugPrintf(">>>>>>>>>START   BUFFERING <<<<<<<<<<<\n");
		Executive_DebugPrintf("@@@@current ves count [%ld  KBytes]!!!!\n", cur_ves_wl);
		//HAL_DECODER_MSG((">>>>>>>>>START   BUFFERING <<<<<<<<<<<\n"));
		//HAL_DECODER_MSG(("@@@@current ves count [%ld  KBytes]!!!!\n", cur_ves_wl));
		buffer_status = MSS_BUFFERING;

	}
	return;
}
#endif

iptv_hal_error IPTV_HAL_Decoder_HALInit()
{
    pkRESULT hr;

    const char* szEnvVar = getenv(c_szEnvVarDecoderInhibit);

    //buffer_status = MSS_BUFFERING_END;


    s_IsDecodingInhibited = (NULL != szEnvVar) && (szEnvVar[0] == 't');

    if (s_IsDecodingInhibited)
    {
        //s_hAVRendererCtx = &s_noDecoderFakeContext;

        HAL_DECODER_MSG(("IPTV_HAL_Decoder_HALInit() - MSPK_DECODER_INHIBIT is set!\n", hr));
    }
    else
    {
        //hr = CAVRendererGst::Startup();
        //if (pkFAILED(hr))
        //{
        //    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_HALInit() CAVRendererGst::Startup FAILED [0x%x]\n", hr));
        //    s_IsDecodingInhibited = true;
        //    return IPTV_HAL_ERROR_DEVICEERROR;
        //}

        //hr = CAVRendererGst::Open(&s_hAVRendererCtx);
        //if (pkFAILED(hr))
        //{
        //    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_HALInit() CAVRendererGst::Open FAILED [0x%x]\n", hr));
        //    s_hAVRendererCtx = NULL;
        //    return IPTV_HAL_ERROR_DEVICEERROR;
        //}

        g_audio_desc.streamID = g_video_desc.streamID = -1;
        g_audio_desc.isEsEnd = g_video_desc.isEsEnd = FALSE;
        if(g_avplay_es_handle == MT_NULL)
        {
            HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_HALInit() g_avplay_es_handle dont init!!!!!\n"));
            return IPTV_HAL_ERROR_DEVICEERROR;
        }
        s_hAVRendererCtx = g_avplay_es_handle;
        dev_open(NULL, NULL);
        vdec_set_avsync_mode_2(NULL, 1);//VDEC_AVSYNC_FILEPLAY

        //printf("IPTV_HAL_Decoder_HALInit() zx g_avplay_es_handle 0x%x\n", g_avplay_es_handle);
        //video_decoder_init(NULL);
        memset(&g_avplay_info, 0, sizeof(mont_avplay_info));
        HAL_DECODER_MSG(("IPTV_HAL_Decoder_HALInit() succeeded\n", hr));
    }

    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function is called once when the IPTV application is about to exit
/// </summary>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Hal Exit failed</para>
/// </returns>
/// <remarks>
/// The implementation is expected to turn off the decoders and release all software/hardware resources associated with the decoder subsystem.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_HALExit()
{
    pkRESULT hr;

    HAL_DECODER_MSG(("IPTV_HAL_Decoder_HALExit()\n"));
    //printf("IPTV_HAL_Decoder_HALExit()\n");

#ifdef DUMP_ES_TO_FILE

     for(int i =0; i< 3; i++)
     {
        if(g_file[i])
        {
            fclose(g_file[i]);
            g_file[i] = NULL;
        }
     }
#endif

    if (s_IsDecodingInhibited)
    {
        s_hAVRendererCtx = 0;
    }
    else
    {
            //pkRESULT hr = CAVRendererGst::Close(s_hAVRendererCtx);
            //if (pkFAILED(hr))
            //{
            //    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_HALInit() CAVRendererGst::Close FAILED [0x%x]\n", hr));
            ///}
            g_audio_desc.streamID = g_video_desc.streamID = -1;

        //hr = CAVRendererGst::Shutdown();
        //if (pkFAILED(hr))
        //{
        //    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_HALInit() CAVRendererGst::Shutdown FAILED [0x%x]\n", hr));
        //}
    }
    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function acquires an available decoder and returns a decoder context for a given codec type if the codec type is supported and a decoder is available
/// </summary>
/// <param name="pAcqParams">[IN] Contains codec type and resoution (if a video codec is requested) </param>
/// <param name="ppDecoderContext">[OUT] Pointer to receive decoder context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: No decoders available for eCodecType</para>
/// </returns>
/// <remarks>
/// The implementation is expected to arbitrate among the available decoders considering the memory and decoding capability of the decoding engine (DSP).
/// For example if the implementation can support 1 HD and 1 PIP or 1SD and 6 PIPs, this decision logic has to be in the acquire implementation.
/// </remarks>
/// <see cref="PIPTV_HAL_DECODER_ACQUIRE"/>
iptv_hal_error IPTV_HAL_Decoder_Acquire(PIPTV_HAL_DECODER_ACQUIRE pAcqParams, LPVOID *ppvDecoderContext)
{
    static unsigned int s_AcquireInstanceCount = 0;

    if (NULL == ppvDecoderContext)
        return IPTV_HAL_ERROR_INVALID_PARAMETER;

    SDecoderContext* pSDecoderContext = new SDecoderContext;

    if (NULL == pSDecoderContext)
        return IPTV_HAL_ERROR_OUT_OF_MEMORY;

    pSDecoderContext->acquireParams = *pAcqParams;

    pSDecoderContext->uiInstance = ++s_AcquireInstanceCount;

    *ppvDecoderContext = pSDecoderContext;

    HAL_DECODER_MSG(("IPTV_HAL_Decoder_Acquire(eCodecType 0x%x, eResolution %d) context: %u\n",
        pAcqParams->eCodecType,
        pAcqParams->eResolution,
        pSDecoderContext->uiInstance));

    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function initializes the decoder, synchronized with the clock context passed in, and sets up the fifo and reference buffers.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="pInitParams">[IN] Pointer to a IPTV_HAL_DECODER_INIT structure that contains an STC context to synchronize with  </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decoder was not successfully initialized</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_Init(LPVOID pvDecoderContext, PIPTV_HAL_DECODER_INIT pInitParams)
{
    pkRESULT hr;

    SDecoderContext* pSDecoderContext = (SDecoderContext*) pvDecoderContext;

    if (NULL == pSDecoderContext)
    {
        HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init() NULL decoder context!\n"));
        return IPTV_HAL_ERROR_INVALID_PARAMETER;
    }

    SDecoderClockContext* pInitClockContext = (SDecoderClockContext*)(pInitParams->pClockContext);

    if (NULL == pInitClockContext)
    {
        HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init() NULL clock context!\n"));
        return IPTV_HAL_ERROR_INVALID_PARAMETER;
    }

    if (NULL != pInitClockContext->hAVRendererCtx)
    {
        HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init() Wrong clock renderer context [%p] \n",
            pInitClockContext->hAVRendererCtx));
        return IPTV_HAL_ERROR_INVALID_PARAMETER;
    }

    HAL_DECODER_MSG(("IPTV_HAL_Decoder_Init(decoder %u, clock %u)\n",
        pSDecoderContext->uiInstance,
        pInitClockContext->uiInstance));

    // Set up stream descriptor and then open a decoder
    {
#if 0
        pkAV_STREAM_DESCRIPTOR streamDesc;

        memset(&streamDesc, 0, sizeof(streamDesc));

        streamDesc.streamID = pSDecoderContext->uiInstance;
        streamDesc.avgBitRate = 0; // let AVRenderer default
        streamDesc.jitterBufferDuration = 0; // NOTE: not used by gstreamer

        // formatType
        streamDesc.formatType.eAudioType = pkAUDIO_FORMAT_TYPE_NONE;
        streamDesc.formatType.eVideoType = pkVIDEO_FORMAT_TYPE_NONE;
        streamDesc.formatType.eVbiType = pkVBI_FORMAT_TYPE_NONE;
        streamDesc.formatType.eSystemType = pkSYSTEM_FORMAT_TYPE_ES;

        switch (pSDecoderContext->acquireParams.eCodecType)
        {
            // video
            case IPTV_HAL_DECODER_CODECTYPE_VIDEO_MPEG2:
                streamDesc.formatType.eVideoType = pkVIDEO_FORMAT_TYPE_MPEG2;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_VIDEO_H264:
                streamDesc.formatType.eVideoType = pkVIDEO_FORMAT_TYPE_H264;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_VIDEO_VC1:
                streamDesc.formatType.eVideoType = pkVIDEO_FORMAT_TYPE_WVC1;
                streamDesc.bitmapInfo.biCompression = GST_MAKE_FOURCC('W', 'V', 'C', '1');
                break;
            case IPTV_HAL_DECODER_CODECTYPE_VIDEO_WMV9:
                streamDesc.formatType.eVideoType = pkVIDEO_FORMAT_TYPE_WMV9;
                streamDesc.bitmapInfo.biCompression = GST_MAKE_FOURCC('W', 'M', 'V', '9');
                break;

            // audio
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_AC3:
                streamDesc.formatType.eAudioType = pkAUDIO_FORMAT_TYPE_AC3;
                streamDesc.waveFormat.wFormatTag = WAVE_FORMAT_DOLBY_AC3;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_PCM:
                streamDesc.formatType.eAudioType = pkAUDIO_FORMAT_TYPE_PCM;
                streamDesc.waveFormat.wFormatTag = WAVE_FORMAT_SDP_PCM;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_MPEG:
                streamDesc.formatType.eAudioType = pkAUDIO_FORMAT_TYPE_MPEG;
                streamDesc.waveFormat.wFormatTag = WAVE_FORMAT_MPEG;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMASTD:
                streamDesc.formatType.eAudioType = pkAUDIO_FORMAT_TYPE_WMA8;
                streamDesc.waveFormat.wFormatTag = WAVE_FORMAT_WMAUDIO2;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMATS:
                streamDesc.formatType.eAudioType = pkAUDIO_FORMAT_TYPE_WMA9;
                streamDesc.waveFormat.wFormatTag = WAVE_FORMAT_WMAUDIO2;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_AAC:
                streamDesc.formatType.eAudioType = pkAUDIO_FORMAT_TYPE_AAC;
                streamDesc.waveFormat.wFormatTag = WAVE_FORMAT_AAC;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMAPRO:
                streamDesc.formatType.eAudioType = pkAUDIO_FORMAT_TYPE_WMA9Pro;
                streamDesc.waveFormat.wFormatTag = WAVE_FORMAT_WMAUDIO3;
                break;

            case IPTV_HAL_DECODER_CODECTYPE_VIDEO_AVS:
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_AVS:
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_EAC3:
            default:
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init() FAILED unknown codec type: %d\n",
                    pSDecoderContext->acquireParams.eCodecType));
                return IPTV_HAL_ERROR_DEVICEERROR;
        }

        // bitmapInfo or waveFormat

        if (pkVIDEO_FORMAT_TYPE_NONE != streamDesc.formatType.eVideoType)
        {
            switch (pSDecoderContext->acquireParams.eResolution)
            {
                case IPTV_HAL_DECODER_RESOLUTION_HD:
                    streamDesc.bitmapInfo.biHeight = 1080;
                    streamDesc.bitmapInfo.biWidth = 1920;
                    break;
                case IPTV_HAL_DECODER_RESOLUTION_PIP:
                    streamDesc.bitmapInfo.biHeight = 120;
                    streamDesc.bitmapInfo.biWidth = 180;
                    break;
                case IPTV_HAL_DECODER_RESOLUTION_SD:
                default:
                    streamDesc.bitmapInfo.biHeight = 480;
                    streamDesc.bitmapInfo.biWidth = 720;
                    break;
            }
            HAL_DECODER_MSG(("IPTV_HAL_Decoder_Init() decoder resolution: %d\n",
                (int)pSDecoderContext->acquireParams.eResolution));

            // default to no extra data off the end of the infoheader
            streamDesc.bitmapInfo.biSize = sizeof(pkAV_BITMAPINFOHEADER);

            if (0 != pSDecoderContext->uiHeaderLength && NULL != pSDecoderContext->pvHeaderData)
            {
                // TODO: use extra header data
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init(video) header data ignored: %u %p\n",
                    pSDecoderContext->uiHeaderLength, pSDecoderContext->pvHeaderData));
            }
        }
        else if (pkAUDIO_FORMAT_TYPE_NONE != streamDesc.formatType.eAudioType)
        {
            if (0 != pSDecoderContext->uiHeaderLength && NULL != pSDecoderContext->pvHeaderData)
            {
                IPTV_HAL_DECODER_AUDIO_HEADER* pAudioHeader = (IPTV_HAL_DECODER_AUDIO_HEADER*)(pSDecoderContext->pvHeaderData);

                if (pSDecoderContext->uiHeaderLength < sizeof(IPTV_HAL_DECODER_AUDIO_HEADER))
                {
                    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init(audio) short header data ignored: %u %p\n",
                        pSDecoderContext->uiHeaderLength, pSDecoderContext->pvHeaderData));
                }
                else
                {
                HAL_DECODER_MSG(("DECODER_AUDIO_HEADER: VN %u SB %u SR %u CH %u BPS %u PL %u EO %u Rs %u bps %u vbps %u CM %u \nisDRC %u %u %u %u %u %u \n",
                    (unsigned int) pAudioHeader->iVersionNumber,
                    (unsigned int) pAudioHeader->cSubband,
                    (unsigned int) pAudioHeader->iSamplingRate,
                    (unsigned int) pAudioHeader->cChannel,
                    (unsigned int) pAudioHeader->cBytePerSec,
                    (unsigned int) pAudioHeader->cbPacketLength,
                    (unsigned int) pAudioHeader->wEncodeOpt,
                    (unsigned int) pAudioHeader->Reserved1,
                    (unsigned int) pAudioHeader->cBitsPerSample,
                    (unsigned int) pAudioHeader->cValidBitsPerSample,
                    (unsigned int) pAudioHeader->u32ChannelMask,
                    (unsigned int) pAudioHeader->bDRCDataIsValid,
                    (unsigned int) pAudioHeader->u32DRCSetting,
                    (unsigned int) pAudioHeader->u32DRCAverageReference,
                    (unsigned int) pAudioHeader->u32DRCAverageTarget,
                    (unsigned int) pAudioHeader->u32DRCPeakReference,
                    (unsigned int) pAudioHeader->u32DRCPeakTarget ));

                    streamDesc.waveFormat.nChannels = pAudioHeader->cChannel;
                    streamDesc.waveFormat.nSamplesPerSec = pAudioHeader->iSamplingRate;
                    streamDesc.waveFormat.nAvgBytesPerSec = pAudioHeader->cBytePerSec;
                    streamDesc.waveFormat.nBlockAlign = (uint16_t)(pAudioHeader->cbPacketLength);
                    streamDesc.waveFormat.wBitsPerSample = pAudioHeader->cBitsPerSample;

                    if (WAVE_FORMAT_WMAUDIO2 == streamDesc.waveFormat.wFormatTag)
                    {
                        streamDesc.waveFormat.cbSize = wma2ExSize;

                        // unused: dwSamplesPerBlock

                        streamDesc.waveFormat.reserved[wma2ExOffset_wEncodeOptions + 0] = (uint8_t)(pAudioHeader->wEncodeOpt);
                        streamDesc.waveFormat.reserved[wma2ExOffset_wEncodeOptions + 1] = (uint8_t)(pAudioHeader->wEncodeOpt >> 8);

                        // unused: dwSuperBlockAlign
                    }
                    else if (WAVE_FORMAT_WMAUDIO3 == streamDesc.waveFormat.wFormatTag)
                    {
                        streamDesc.waveFormat.cbSize = wma3ExSize;

                        streamDesc.waveFormat.reserved[wma3ExOffset_wValidBitsPerSample + 0] = (uint8_t)(pAudioHeader->cValidBitsPerSample);
                        streamDesc.waveFormat.reserved[wma3ExOffset_wValidBitsPerSample + 1] = (uint8_t)(pAudioHeader->cValidBitsPerSample >> 8);

                        streamDesc.waveFormat.reserved[wma3ExOffset_wEncodeOptions + 0] = (uint8_t)(pAudioHeader->wEncodeOpt);
                        streamDesc.waveFormat.reserved[wma3ExOffset_wEncodeOptions + 1] = (uint8_t)(pAudioHeader->wEncodeOpt >> 8);

                        streamDesc.waveFormat.reserved[wma3ExOffset_dwChannelMask + 0] = (uint8_t)(pAudioHeader->u32ChannelMask);
                        streamDesc.waveFormat.reserved[wma3ExOffset_dwChannelMask + 1] = (uint8_t)(pAudioHeader->u32ChannelMask >> 8);
                        streamDesc.waveFormat.reserved[wma3ExOffset_dwChannelMask + 2] = (uint8_t)(pAudioHeader->u32ChannelMask >> 16);
                        streamDesc.waveFormat.reserved[wma3ExOffset_dwChannelMask + 3] = (uint8_t)(pAudioHeader->u32ChannelMask >> 24);
                    }

                }
            }
            else // no audio codec parameters provided
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init(audio) ERROR: missing audio parameters [%u %p]\n",
                    pSDecoderContext->uiHeaderLength, pSDecoderContext->pvHeaderData));

                return IPTV_HAL_ERROR_DEVICEERROR;
            }
        }


        if (s_IsDecodingInhibited)
        {
            pSDecoderContext->hAVRDecoder = &s_noDecoderFakeContext;
        }
        else
        {
            pkBUFFER_POOL_DESCRIPTOR bufferPoolDesc;
            printf("eAudioType %d, eVideoType %d\n", streamDesc.formatType.eAudioType, streamDesc.formatType.eVideoType);


#if 0
            hr = CAVRendererGst::OpenDecoder
                (
                    s_hAVRendererCtx,
                    &streamDesc,
                    &bufferPoolDesc,
                    &pSDecoderContext->hAVRDecoder
                );
            if (pkFAILED(hr))
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init() CAVRendererGst::Open FAILED [0x%x]\n", hr));
                pSDecoderContext->hAVRDecoder = NULL;
                return IPTV_HAL_ERROR_DEVICEERROR;
            }
#endif

        }
#endif
        montAV_STREAM_DESCRIPTOR streamDesc;
        switch (pSDecoderContext->acquireParams.eCodecType)
        {
            // video
            case IPTV_HAL_DECODER_CODECTYPE_VIDEO_MPEG2:
                streamDesc.codec_id = MT_UNF_VCODEC_TYPE_MPEG2;
                streamDesc.codec_type = AV_MEDIA_TYPE_VIDEO;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_VIDEO_H264:
                streamDesc.codec_id = MT_UNF_VCODEC_TYPE_H264;
                streamDesc.codec_type = AV_MEDIA_TYPE_VIDEO;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_VIDEO_VC1:
                streamDesc.codec_id = MT_UNF_VCODEC_TYPE_VC1;
                streamDesc.codec_type = AV_MEDIA_TYPE_VIDEO;
                break;

            // audio
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_AC3:
                streamDesc.codec_id = HA_AUDIO_ID_DOLBY_PLUS;
                streamDesc.codec_type = AV_MEDIA_TYPE_AUDIO;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_PCM:
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init() IPTV_HAL_DECODER_CODECTYPE_AUDIO_PCM too many type!!!\n"));
                streamDesc.codec_id = HA_AUDIO_ID_PCM;
                streamDesc.codec_type = AV_MEDIA_TYPE_AUDIO;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_MPEG:
                streamDesc.codec_id = HA_AUDIO_ID_MP3;//??? pkAUDIO_FORMAT_TYPE_MPEG;
                streamDesc.codec_type = AV_MEDIA_TYPE_AUDIO;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_AAC:
                streamDesc.codec_id = HA_AUDIO_ID_AAC;
                streamDesc.codec_type = AV_MEDIA_TYPE_AUDIO;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMAPRO:
                streamDesc.codec_id = HA_AUDIO_ID_WMA9STD;
                streamDesc.codec_type = AV_MEDIA_TYPE_AUDIO;
                break;
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_EAC3:
                streamDesc.codec_id = HA_AUDIO_ID_DOLBY_PLUS;
                streamDesc.codec_type = AV_MEDIA_TYPE_AUDIO;
                break;

            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMASTD:
                streamDesc.codec_id = HA_AUDIO_ID_WMA9STD;
                streamDesc.codec_type = AV_MEDIA_TYPE_AUDIO;
                break;

            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_WMATS:
            case IPTV_HAL_DECODER_CODECTYPE_VIDEO_AVS:
            case IPTV_HAL_DECODER_CODECTYPE_AUDIO_AVS:
            case IPTV_HAL_DECODER_CODECTYPE_VIDEO_WMV9:
            default:
                streamDesc.codec_id = HA_AUDIO_ID_CUSTOM_0;
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init() FAILED unknown codec type: %d\n",
                    pSDecoderContext->acquireParams.eCodecType));
                return IPTV_HAL_ERROR_DEVICEERROR;
        }

        if(streamDesc.codec_id != HA_AUDIO_ID_CUSTOM_0)
            streamDesc.streamID = pSDecoderContext->uiInstance;
        else
            streamDesc.codec_type = AV_MEDIA_TYPE_UNKNOWN;


        if(streamDesc.codec_type == AV_MEDIA_TYPE_AUDIO)
        {
            g_audio_desc.streamID = streamDesc.streamID;
            g_audio_desc.codec_id = streamDesc.codec_id;
            g_audio_desc.codec_type = streamDesc.codec_type;

            if (0 != pSDecoderContext->uiHeaderLength && NULL != pSDecoderContext->pvHeaderData)
            {
                IPTV_HAL_DECODER_AUDIO_HEADER* pAudioHeader = (IPTV_HAL_DECODER_AUDIO_HEADER*)(pSDecoderContext->pvHeaderData);

                if (pSDecoderContext->uiHeaderLength < sizeof(IPTV_HAL_DECODER_AUDIO_HEADER))
                {
                    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Init(audio) short header data ignored: %u %p\n",
                        pSDecoderContext->uiHeaderLength, pSDecoderContext->pvHeaderData));
                }
                else
                {
 #if 0
                 printf("DECODER_AUDIO_HEADER: VN %u SB %u SR %u CH %u BPS %u PL %u EO %u Rs %u bps %u vbps %u CM %u \nisDRC %u %u %u %u %u %u \n",
                    (unsigned int) pAudioHeader->iVersionNumber,
                    (unsigned int) pAudioHeader->cSubband,
                    (unsigned int) pAudioHeader->iSamplingRate,
                    (unsigned int) pAudioHeader->cChannel,
                    (unsigned int) pAudioHeader->cBytePerSec,
                    (unsigned int) pAudioHeader->cbPacketLength,
                    (unsigned int) pAudioHeader->wEncodeOpt,
                    (unsigned int) pAudioHeader->Reserved1,
                    (unsigned int) pAudioHeader->cBitsPerSample,
                    (unsigned int) pAudioHeader->cValidBitsPerSample,
                    (unsigned int) pAudioHeader->u32ChannelMask,
                    (unsigned int) pAudioHeader->bDRCDataIsValid,
                    (unsigned int) pAudioHeader->u32DRCSetting,
                    (unsigned int) pAudioHeader->u32DRCAverageReference,
                    (unsigned int) pAudioHeader->u32DRCAverageTarget,
                    (unsigned int) pAudioHeader->u32DRCPeakReference,
                    (unsigned int) pAudioHeader->u32DRCPeakTarget );
#endif
                    if (HA_AUDIO_ID_PCM == g_audio_desc.codec_id) {
                		WAV_FORMAT_S audio_param;
                		int is_big_endian = 0;
                		int bits = 16;
                		int channels = 2;
                		int sample_rate = 44100;

            			sample_rate = pAudioHeader->cBitsPerSample;
            			channels = pAudioHeader->cChannel;

                        if(MT_TRUE == is_big_endian)
                        {
                            audio_param.cbSize = 4;
                            audio_param.cbExtWord[0] = 1;//1 //1 is NORMAL_PCM_EXTWORD choose normal pcm decoder
                            //stWavFormat.cbExtWord[0] = WIFIDSP_LPCM_EXTWORD; //choose wifi_dsp_lpcm decoder
                        }

                		audio_param.nSamplesPerSec = sample_rate;
                		audio_param.nChannels = channels;
                		audio_param.wBitsPerSample = bits;
                		audio_param.cbExtWord[0] = is_big_endian;
                		audio_param.cbExtWord[1] = channels >> 1;

                        audio_param.wFormatTag = 0xffee;
                		//printf("pcm_header:is_big %d channels %d bits %d, samplerate %d \n", is_big_endian,channels, bits, sample_rate);
                        aud_set_dec_param_vsb(NULL, &audio_param,g_audio_desc.codec_id);
                        aud_stop_vsb(NULL);
                        aud_start_vsb(NULL, HA_AUDIO_ID_PCM);
                        g_audio_desc.isEsEnd = FALSE;//add to fix bug when program finished to start another program the first es data will case vdec eos
						suplayer_set_push_aud_flag(TRUE);
					}
                    else
                    {
                        aud_stop_vsb(NULL);
                        aud_start_vsb(NULL, g_audio_desc.codec_id);						
                        g_audio_desc.isEsEnd = FALSE;//add to fix bug when program finished to start another program the first es data will case vdec eos
						suplayer_set_push_aud_flag(TRUE);
					}

                }

            }

        }
        else if(streamDesc.codec_type == AV_MEDIA_TYPE_VIDEO)
        {
            g_video_desc.streamID = streamDesc.streamID;
            g_video_desc.codec_id = streamDesc.codec_id;
            g_video_desc.codec_type = streamDesc.codec_type;
            g_video_desc.isEsEnd = FALSE;//add to fix bug when program finished to start another program the first es data will case vdec eos
			suplayer_set_push_vid_flag(TRUE);			
			vdec_stop(NULL);
            vdec_start(NULL, streamDesc.codec_id, 2);
            vdec_set_dec_frm_type(NULL, MT_UNF_DEC_FRM_ALL);
        }

        if (pInitClockContext->isClockRunning)
        {
            // kick the clock to get new decoder in on the game
            IPTV_HAL_Decoder_Clock_Stop(pInitClockContext);
            IPTV_HAL_Decoder_Clock_Start(pInitClockContext);
        }
    } // !s_IsDecodingInhibited

    pSDecoderContext->pSDecoderClockContext = pInitClockContext;
    pSDecoderContext->latestPreparePTS = 0;
    pSDecoderContext->nextLogOutputDecoderTime = 0;

    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function flushes the decoder.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="bClearPicture">[IN] If true, picture buffers are also cleared for the decoder and a black screen is expected on the display. Not applicable for an audio decodercontext </param>
/// <param name="bClearDecoderStall">[IN] If true, this is a notification that an attempt to write to the decoder has failed and decoder recovery should be attempted</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decoder was not successfully flushed</para>
/// </returns>
/// <remarks>
/// The Decoder implementation should clear the reference frames and the input FIFO.
/// The FlushDecoder API is used during a channel change that does not involve changing codecs.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Flush(LPVOID pvDecoderContext, bool bClearPicture, bool bClearDecoderStall)
{
    SDecoderContext* pSDecoderContext = (SDecoderContext*) pvDecoderContext;

    HAL_DECODER_MSG(("IPTV_HAL_Decoder_Flush(%04x, bClearPicture %d, bClearDecoderStall %d\n",
        pSDecoderContext->uiInstance, bClearPicture, bClearDecoderStall));

    pSDecoderContext->latestPreparePTS = 0;
    pSDecoderContext->nextLogOutputDecoderTime = 0;

    //printf("11 IPTV_HAL_Decoder_Flush(%04x, bClearPicture %d, bClearDecoderStall %d\n",
    //        pSDecoderContext->uiInstance, bClearPicture, bClearDecoderStall);
    if (bClearPicture)// || bClearDecoderStall)//Ignore bClearDecoderStall to fix the fast speed can't resume normal speed issue.
    {
        /*
        printf("22 IPTV_HAL_Decoder_Flush(%04x, bClearPicture %d, bClearDecoderStall %d\n",
            pSDecoderContext->uiInstance, bClearPicture, bClearDecoderStall);
        */
        bool is_audio_stream = false;
        bool is_video_stream = false;
        if(g_audio_desc.streamID != -1 && g_video_desc.streamID != -1)
        {
            //pkRESULT hr = CAVRendererGst::FlushDecoder(s_hAVRendererCtx, pSDecoderContext->hAVRDecoder);
            //if (pkFAILED(hr))
            //{
            //    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Flush(%u) CAVRendererGst::FlushDecoder FAILED [0x%x]\n",
            //        pSDecoderContext->uiInstance,
            //        hr));
            //    return IPTV_HAL_ERROR_DEVICEERROR;
            //}
            if(g_audio_desc.streamID == pSDecoderContext->uiInstance)
                is_audio_stream = true;
            if(g_video_desc.streamID == pSDecoderContext->uiInstance)
                is_video_stream = true;
            if(is_audio_stream || is_video_stream)
            {
                vdec_reset();//change vdec_flush to vdec_reset because the vdec will hangup
                g_avplay_info.flush_state = 1;
            }
        }
     }

    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function is used by the application to release all resources associated with this decoder and put the decoder back to the available state.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decoder was not successfully released.</para>
/// </returns>
/// <remarks>
/// The decoder release method is used during a channel change that also involves a codec change.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_ReleaseDecoder(LPVOID pvDecoderContext)
{
    iptv_hal_error result = IPTV_HAL_ERROR_SUCCESS;

    SDecoderContext* pSDecoderContext = (SDecoderContext*) pvDecoderContext;

    HAL_DECODER_MSG(("IPTV_HAL_Decoder_ReleaseDecoder(%u)\n", pSDecoderContext->uiInstance));

    //if (NULL != pSDecoderContext->hAVRDecoder)
    //{
        if (!s_IsDecodingInhibited)
        {
            // Ensure the clock is shut down completely
            if(g_audio_desc.streamID == pSDecoderContext->uiInstance)
            {
                //printf("zx IPTV_HAL_Decoder_ReleaseDecoder aud_stop_vsb\n");
                aud_stop_vsb(NULL);
                g_audio_desc.streamID = -1;//stop need reset streamID                
				suplayer_set_push_aud_flag(FALSE);
            }
            else if(g_video_desc.streamID == pSDecoderContext->uiInstance)
            {
                memset(&g_avplay_info, 0, sizeof(mont_avplay_info));
                //printf("zx IPTV_HAL_Decoder_ReleaseDecoder vdec_stop\n");
                vdec_stop(NULL);
                g_video_desc.streamID = -1;//stop need reset streamID
				suplayer_set_push_vid_flag(FALSE);
                IPTV_HAL_Decoder_Clock_Stop(pSDecoderContext->pSDecoderClockContext);
                IPTV_HAL_Decoder_Clock_Init(pSDecoderContext->pSDecoderClockContext);
            }
            //pkRESULT hr = CAVRendererGst::CloseDecoder(s_hAVRendererCtx, pSDecoderContext->hAVRDecoder);
            //if (pkFAILED(hr))
            //{
            //    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_ReleaseDecoder(%u) CAVRendererGst::CloseDecoder FAILED [0x%x]\n",
            //        pSDecoderContext->uiInstance,
            //        hr));
            //    result = IPTV_HAL_ERROR_DEVICEERROR;
            //}
        }
        pSDecoderContext->hAVRDecoder = NULL;
    //}

    delete pSDecoderContext;

    return result;
}

/// <summary>
/// This API allows the application to gain direct access to the decoder's FIFO.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="pInBufList">[IN] Pointer to A/V data for which space is needed in the decoder FIFO (the data may be encrypted ES)</param>
/// <param name="bPTSValid">[IN] If true, the PTSTime90khz is valid</param>
/// <param name="PTSTime90khz">[IN] A presentation timestamp value that represents the PTS in 90KHz units corresponding to this ES data</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decoder rejected this sample</para>
/// <para>IPTV_HAL_ERROR_NOT_ENOUGH_MEMORY: There was not enough room in the FIFO. The application should wait for a period of time to elapse for the decoder FIFO to drain and retry the operation</para>
/// </returns>
/// <remarks>
/// The crypto block will decrypt the incoming encrypted ES data from the AV Subsystem into the decoder FIFO directly.
/// Ideally the FIFO for a decoder should be in the secure memory area accessible to the video processor but not to the CPU.
/// IPTV_HAL_Decoder_PrepareForDecode reserves space in the decoder FIFO for the PES header with any padding bytes (in the case where the decoder consumes only a PES stream) and the ES.
/// It also records various pieces of information in the decoder context such that they need not be supplied again in subsequent functions.
/// The PrepareForDecode, Decrypt (if implemented), Decode sequence will never be interleaved with another similar sequence for the same decodercontext.
/// If possible a single buffer will be allocated, but if the current write point in the FIFO is near the end, then two buffers will need to be allocated.
/// This function will also allocate enough space for the insertion of PES header and padding bytes for alignment purposes (if required as in the case of decoders consuming PES streams)
/// ahead of the ES buffer(s) being returned. This parameter is provided solely for the benefit of test applications that don't use the decrypt function that follows and yet need to
/// know where the payload should be placed.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_PrepareForDecode(
    LPVOID                     pvDecoderContext,
    PIPTV_HAL_BUFFER         pInBufList,
    bool                     bPTSValid,
    IPTV_HAL_DECODER_TIME     PTSTime90khz)
{
    iptv_hal_error retval = IPTV_HAL_ERROR_SUCCESS;

    SDecoderContext* pSDecoderContext = (SDecoderContext*)pvDecoderContext;

    HAL_DECODER_MSG(("IPTV_HAL_Decoder_PrepareForDecode[%u] PTS %.3lfs\n",
        pSDecoderContext->uiInstance,
        bPTSValid ? (double) (PTSTime90khz / c_dblTimescale90KHz) : (double)0.0));


    bPTSValid = bPTSValid  && (PTSTime90khz != -1LL);

    if (bPTSValid && (pSDecoderContext->latestPreparePTS < PTSTime90khz))
    {
        HAL_DECODER_MSG(("Updating latestPreparePTS=%lld",PTSTime90khz));
        pSDecoderContext->latestPreparePTS = PTSTime90khz;
    }

    PIPTV_HAL_BUFFER pBufListItem = pInBufList;

    //if (NULL != pSDecoderContext->hAVRDecoder)
    if (1)
    {
        // Get enough decoder buffers to hold the input buffer chain data
#if 0
        uint32_t uiDecoderBufferSize = 0; // cause first time through loop to get the first decoder buffer
        UINT32 uiDecoderBufferEnd = 0;
        PIPTV_HAL_BUFFER* ppNextOutputBuffer = &pSDecoderContext->pOutputBufferChain;
        PIPTV_HAL_BUFFER pNextOutputBuffer = NULL;

        pSDecoderContext->dtOutputTimestamp = bPTSValid ? PTSTime90khz : -1LL;

        while (NULL != pBufListItem)
        {
            HAL_DECODER_MSG(("- Buffer size/start/end (%u, %u, %u) flags 0x%x\n",
                pBufListItem->u32Size,
                pBufListItem->u32DataStart,
                pBufListItem->u32DataEnd,
                pBufListItem->u32Flags));

            uiDecoderBufferEnd += (pBufListItem->u32DataEnd - pBufListItem->u32DataStart);

            if (uiDecoderBufferEnd > uiDecoderBufferSize)
            {
                // Get another decoder buffer and create a IPTV_HAL_BUFFER to house it

                uiDecoderBufferEnd -= uiDecoderBufferSize; // adjust to only the overflow

                uint8_t* pDecoderBuffer;

                pkRESULT hr = pkS_OK;

                if (s_IsDecodingInhibited)
                {
                    uiDecoderBufferSize = 64 * 1024; // use 64K buffers for simulation
                    pDecoderBuffer = (uint8_t*) Executive_Alloc(uiDecoderBufferSize, TRUE);
                    if (NULL == pDecoderBuffer)
                    {
                        hr = pkE_INSUFFICIENT_BUFFER;
                    }
                }
                else
                {
                #if 0
                    hr = CAVRendererGst::GetDecoderBuffer
                            (
                                s_hAVRendererCtx, // pkHANDLE hRenderer,
                                pSDecoderContext->hAVRDecoder, // pkHANDLE hDecoder,
                                &pDecoderBuffer, // uint8_t **ppBuffer,
                                &uiDecoderBufferSize, // uint32_t *pBufSize,
                                100 // uint32_t timeoutMillisecs
                            );
				#endif
                }
                if (pkFAILED(hr))
                {
                    retval = IPTV_HAL_ERROR_DECODER_ESFIFO_FULL;

                    HAL_DECODER_ERRMSG(("Decoder[%u] CAVRendererGst::GetDecoderBuffer FAILED[0x%x]\n",
                        pSDecoderContext->uiInstance, hr));
                    // TODO: clean up any allocations so far
                    break;
                }
                else // set up output buffer chain
                {
                    pNextOutputBuffer = new IPTV_HAL_BUFFER;
                    if (NULL == pNextOutputBuffer)
                    {
                        retval = IPTV_HAL_ERROR_OUT_OF_MEMORY;

                        HAL_DECODER_ERRMSG(("Decoder[%u] new IPTV_HAL_BUFFER FAILED\n",
                            pSDecoderContext->uiInstance));
                        // TODO: clean up any allocations so far - this is game over
                        break;
                    }

                    // set up the chain link
                    *ppNextOutputBuffer = pNextOutputBuffer;
                    ppNextOutputBuffer = &pNextOutputBuffer->pNext;

                    pNextOutputBuffer->pBuf = (PUCHAR)pDecoderBuffer;
                    pNextOutputBuffer->pNext = NULL;
                    pNextOutputBuffer->u32DataEnd = 0;
                    pNextOutputBuffer->u32DataStart = 0;
                    pNextOutputBuffer->u32Size = uiDecoderBufferSize;
                    pNextOutputBuffer->u32Flags = (IPTV_HAL_BUFFER_FLAGS)0;
                    pNextOutputBuffer->reserved1 = 0;
                }
            }

            pNextOutputBuffer->u32DataEnd = uiDecoderBufferEnd;
            pNextOutputBuffer->u32Flags = (IPTV_HAL_BUFFER_FLAGS)( pNextOutputBuffer->u32Flags | pBufListItem->u32Flags );

            pBufListItem = pBufListItem->pNext;
        }
#endif
        pSDecoderContext->pInputBufferChain = pInBufList;
        pSDecoderContext->pOutputBufferChain = pInBufList;

    }
    else // no hAVRDecoder
    {
        while (pBufListItem)
        {
            HAL_DECODER_MSG(("- Buffer (no decoder) size/start/end (%u, %u, %u) flags 0x%x\n",
                pBufListItem->u32Size,
                pBufListItem->u32DataStart,
                pBufListItem->u32DataEnd,
                pBufListItem->u32Flags));

            pBufListItem = pBufListItem->pNext;
        }

        // Return IPTV_HAL_ERROR_DECODER_ESFIFO_FULL if ES FIFO full to throttle the pace.
        //    ESFIFO_FULL is really a status rather than an error.
        //    The upper layer is expected to try again a short time later.

        if (NULL != pSDecoderContext->pSDecoderClockContext)
        {
            IPTV_HAL_DECODER_TIME dtNow = 0;
            IPTV_HAL_Decoder_Clock_GetTime(pSDecoderContext->pSDecoderClockContext, &dtNow);

            if (pSDecoderContext->latestPreparePTS > dtNow)
            {
                double secsDecoderAhead = ((double)(pSDecoderContext->latestPreparePTS - dtNow) / c_dblTimescale90KHz);
                if (secsDecoderAhead > k_ES_FIFO_SIZE_SECONDS)
                {
                    retval = IPTV_HAL_ERROR_DECODER_ESFIFO_FULL;

                    HAL_DECODER_MSG(("Decoder[%u] ES FIFO FULL (%.3lfs)\n",
                        pSDecoderContext->uiInstance,
                        secsDecoderAhead));
                }
            }
        }
    }

    return retval;
}

/// <summary>
/// IPTV_HAL_Decoder_Decrypt calls the cryptocore's DecryptAVPayload function (see below) on behalf of the AV system.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="pfnDecryptCallback">[IN] The pfnDecryptCallback points to the function that is to be called when the buffers have been prepared. The caller can supply different callbacks to distinguish the case where decryption is required from the passthrough case where it isn't</param>
/// <param name="pDecryptContext">[IN] The pDecryptContext points to the data required by the callback function </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decrypt operation failed</para>
/// </returns>
/// <remarks>
/// IPTV_HAL_Decoder_Decrypt prepares suitable input and output buffer structures and then calls the supplied decryption callback, passing the buffers
/// and the supplied decryption context pointer. The callback will do the actual decryption, typically employing the crypto HAL for that purpose.
/// The function IPTV_HAL_Decoder_PrepareToDecode, referring to the same decoder context, must have been called previously with no
/// intervening call of IPTV_HAL_Decoder_Decode. Typically, each input buffer supplied to the decryption callback will either be an input
/// buffer placed in the decoder context by the prior call of IPTV_HAL_Decoder_PrepareToDecode, or an additional buffer
/// containing headers that may be required by the decoder on that platform. Any buffer of the latter type must have its flags
/// field set to IPTV_HAL_CRYPTO_BUFFER_FLAG_PASSTHRU so that decryption is not applied to such headers
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Decrypt(
    LPVOID             pvDecoderContext,
    LPFN_DECRYPTCB     pfnDecryptCallback,
    PVOID             pDecryptContext)
{
    iptv_hal_error retval = IPTV_HAL_ERROR_SUCCESS;

    SDecoderContext* pSDecoderContext = (SDecoderContext*)pvDecoderContext;

    HAL_DECODER_MSG(("IPTV_HAL_Decoder_Decrypt enter!!!!\n"));
#if 1
    //if (NULL != pSDecoderContext->hAVRDecoder)
    {
        if (NULL == pSDecoderContext->pInputBufferChain
            || NULL == pSDecoderContext->pOutputBufferChain)
        {
            retval = IPTV_HAL_ERROR_NOT_INITIALIZED;
            HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Decrypt[%u] buffer chains not initialized\n",
                pSDecoderContext->uiInstance));
            goto exit;
        }
        retval = pfnDecryptCallback
                (
                    pSDecoderContext->pInputBufferChain,
                    pSDecoderContext->pOutputBufferChain,
                    pDecryptContext
                );

        HAL_DECODER_MSG(("IPTV_HAL_Decoder_Decrypt[%u] pfnDecryptCallback returned %u\n",
            pSDecoderContext->uiInstance, retval));
    }
#endif

exit:
    return retval;
}

/// <summary>
/// The IPTV_HAL_Decoder_Decode function makes data that was decrypted using a prior call of IPTV_HAL_Decoder_Decrypt available to the decoder.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The decoder did not successfully set up the decode operation</para>
/// </returns>
/// <remarks>
/// Failure (a catastrophic case) can only happen if the fifo area is trashed between the PrepareForDecode and Decode operation.
/// The application should not attempt to retry the sample. The expectation here is an internal monitor thread in the implementation will detect this condition and recover the
/// decoder out of this state.
/// </remarks>

//gst 1s is 10*9
//#define GST_PTS_SECOND 1000000000
#define MONT_PTS_SECOND 1000000
iptv_hal_error IPTV_HAL_Decoder_Decode(LPVOID pvDecoderContext)
{
    iptv_hal_error retval = IPTV_HAL_ERROR_SUCCESS;

    SDecoderContext* pSDecoderContext = (SDecoderContext*)pvDecoderContext;


    if (pSDecoderContext->pictureInfoValue.hdtDTS != -1LL)
    {
        if ((pSDecoderContext->pictureInfoValue.hdtDTS - pSDecoderContext->latestDTS) < 0)
        {
            HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Decode[%u] ERROR: DTS jumped back %.3lfs; PTS:%.3lfs DTS:%0.3lfs\n",
                pSDecoderContext->uiInstance,
                (double) (pSDecoderContext->latestDTS - pSDecoderContext->pictureInfoValue.hdtDTS) / c_dblTimescale90KHz,
                (double) pSDecoderContext->latestPreparePTS / c_dblTimescale90KHz,
                (double) pSDecoderContext->pictureInfoValue.hdtDTS / c_dblTimescale90KHz ));
        }
        else
        {
            pSDecoderContext->latestDTS = pSDecoderContext->pictureInfoValue.hdtDTS;
        }
    }

#ifdef DUMP_ES_TO_FILE
    FILE* pFile;
    if(g_file[pSDecoderContext->uiInstance] == NULL)
    {
        char szFilename[64];
        FILE* pFile;
        snprintf(szFilename, sizeof(szFilename), "dump_%d.es", pSDecoderContext->uiInstance);
        fopen_s(&pFile, (const char *)&szFilename, "wb");
        g_file[pSDecoderContext->uiInstance] = pFile;
    }
    pFile = g_file[pSDecoderContext->uiInstance];
#endif

    HAL_DECODER_MSG(("IPTV_HAL_Decoder_Decode PTS:%.3lfs DTS:%0.3lfs\n",
       (double) pSDecoderContext->latestPreparePTS / c_dblTimescale90KHz,
       (double) pSDecoderContext->pictureInfoValue.hdtDTS / c_dblTimescale90KHz ));

    bool is_audio_stream = false;
    bool is_video_stream = false;

    if(g_audio_desc.streamID != -1 && g_video_desc.streamID != -1)
    {
        // Submit all the buffers in the output chain
        PIPTV_HAL_BUFFER pNextOutBuffer = pSDecoderContext->pOutputBufferChain;

        if(g_audio_desc.streamID == pSDecoderContext->uiInstance)
            is_audio_stream = true;
        if(g_video_desc.streamID == pSDecoderContext->uiInstance)
            is_video_stream = true;

        //int64_t pts = pSDecoderContext->latestPreparePTS * MONT_PTS_SECOND / c_dblTimescale90KHz;//zx22
        int64_t pts = pSDecoderContext->latestPreparePTS /c_dblTimescale90KHz * MONT_PTS_SECOND;//zx22 //modify for live play pts is so large to overflow
        //printf("zx22 %lld\n", pts);
#if 0
        // TODO: fix AVRenderer to take 90KHz time stamps instead of 1KHz
        int64_t kHzTimeStamp;

        if (-1LL == pSDecoderContext->dtOutputTimestamp)
        {
            kHzTimeStamp = pkINVALID_TIME_STAMP;
        }
        else
        {
            // convert 90KHz to 1KHz
            kHzTimeStamp = (int64_t)((pSDecoderContext->dtOutputTimestamp + 45) / 90);
        }
#endif

        // Base fragment type on sequence of output buffers
        pkSAMPLE_FRAGMENT_TYPE fragmentType;

        fragmentType = (NULL == pNextOutBuffer->pNext)
            ? pkSAMPLE_FRAGMENT_TYPE_NONE
            : pkSAMPLE_FRAGMENT_TYPE_FRAGMENT_BEGIN;

        while (NULL != pNextOutBuffer)
        {
            //PIPTV_HAL_BUFFER pOldNextOutBuffer;
            uint32_t sampleFlags = 0;
            if (0 != (pNextOutBuffer->u32Flags & IPTV_HAL_BUFFER_FLAG_ENDFRAME))
            {
                sampleFlags |= pkSAMPLEFLAG_KEY_FRAME;
            }
            if (0 != (pNextOutBuffer->u32Flags & IPTV_HAL_BUFFER_FLAG_DISCONTINUITY))
            {
                sampleFlags |= pkSAMPLEFLAG_DISCONTINUITY;
            }

            UINT32 numbytes = pNextOutBuffer->u32DataEnd - pNextOutBuffer->u32DataStart;
            //printf("SubmitDecoderBuffer len:%u , pts:%lld\n",
            //        (unsigned int) numbytes, pts);
#ifdef DUMP_ES_TO_FILE
            if ( numbytes > 0)
            {
                fwrite(pNextOutBuffer->pBuf + pNextOutBuffer->u32DataStart, 1, numbytes, pFile);
                fflush(pFile);
            }
#endif

            if(g_avplay_info.flush_state == 1)
            {
                g_avplay_info.last_getpts = pSDecoderContext->latestPreparePTS;
                g_avplay_info.flush_state = 2;
            }
            if(is_audio_stream)
            {
                //printf("zx22 a %llx isEsEnd:%d\n", pts, g_audio_desc.isEsEnd);
                aud_file_pushesbuffer_vsb(NULL, (u32)(pNextOutBuffer->pBuf + pNextOutBuffer->u32DataStart),
                    numbytes, pts, g_audio_desc.isEsEnd);
            }

            if(is_video_stream)
            {
                //printf("zx22 v %llx isEsEnd:%d\n", pts,g_video_desc.isEsEnd);
                vdec_dec_push_es(NULL, (u32)(pNextOutBuffer->pBuf + pNextOutBuffer->u32DataStart),
                    numbytes, pts, g_video_desc.isEsEnd);
            }

            //pOldNextOutBuffer = pNextOutBuffer;
            pNextOutBuffer = pNextOutBuffer->pNext;//WriteHALDecoder  Buffer* chain, delete by it
            //delete pOldNextOutBuffer;

            fragmentType = (NULL == pNextOutBuffer)
                ? pkSAMPLE_FRAGMENT_TYPE_FRAGMENT_END
                : pkSAMPLE_FRAGMENT_TYPE_FRAGMENT_CONTINUE;

        }
        /*if(is_video_stream == true){
            unsigned int video_disp_w = pSDecoderContext->pictureInfoValue.u16Width;
            unsigned int video_disp_h = pSDecoderContext->pictureInfoValue.u16Height;
            check_ves_water_level(video_disp_w,video_disp_h);
        }*/
	}
    else
        HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_Decode audio or video decoder dont init!!!!\n"));

    //if (s_IsDecodingInhibited || NULL == pSDecoderContext->hAVRDecoder)
    if(0)
    {
        double secsDecoderAhead = 0.0;
        IPTV_HAL_DECODER_TIME dtNow = 0;

        if (NULL != pSDecoderContext->pSDecoderClockContext)
        {
            IPTV_HAL_Decoder_Clock_GetTime(pSDecoderContext->pSDecoderClockContext, &dtNow);

            secsDecoderAhead = ((double)(pSDecoderContext->latestPreparePTS - dtNow) / c_dblTimescale90KHz);
        }

        // Always output here since this is all the "no decoder" does; moderate log rate when gst fails.

        if (s_IsDecodingInhibited || pSDecoderContext->nextLogOutputDecoderTime < dtNow)
        {
            pSDecoderContext->nextLogOutputDecoderTime += 90 * k_LOG_OUTPUT_PERIOD_MS;

            if (pSDecoderContext->nextLogOutputDecoderTime < dtNow)
            {
                pSDecoderContext->nextLogOutputDecoderTime = dtNow + 90 * k_LOG_OUTPUT_PERIOD_MS;
            }
            HAL_DECODER_OUTPUT(("IPTV_HAL_Decoder_Decode[%u] PTS:%.3lfs DTS:%0.3lfs dur:%0.3lfs Flags:0x%x (%.3lfs buffered) size(%d,%d,%d,%d)\n",
                pSDecoderContext->uiInstance,
                (double) pSDecoderContext->latestPreparePTS / c_dblTimescale90KHz,
                (double) pSDecoderContext->pictureInfoValue.hdtDTS / c_dblTimescale90KHz,
                (double) pSDecoderContext->pictureInfoValue.hdtDuration / c_dblTimescale90KHz,
                (unsigned int)pSDecoderContext->pictureInfoValue.u32Flags,
                secsDecoderAhead,
                (unsigned int)pSDecoderContext->pictureInfoValue.u16Width,
                (unsigned int)pSDecoderContext->pictureInfoValue.u16Height,
                (unsigned int)pSDecoderContext->pictureInfoValue.u16Aspx,
                (unsigned int)pSDecoderContext->pictureInfoValue.u16Aspy ));
        }
    }
    return retval;
}

/// <summary>
/// This function is used by the application to "get" a specified Value from the decoder.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="eValueType">[IN] Indicates the decoder value to "Get"</param>
/// <param name="pValueData">[OUT] Pointer to receive the value of the desired decoder parameter</param>
/// <param name="pValueDataLength">[OUT] Pointer to receive the length of pValueData</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid parameter</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_GetValue(
    LPVOID                         pvDecoderContext,
    IPTV_HAL_DECODER_VALUETYPE     eValueType,
    const LPVOID                 pValueData,
    UINT32*                     pValueDataLength)
{
    iptv_hal_error retVal = IPTV_HAL_ERROR_FAILED;

    SDecoderContext* pSDecoderContext = (SDecoderContext*)pvDecoderContext;

    if (NULL == pValueData || NULL == pValueDataLength)
    {
        retVal = IPTV_HAL_ERROR_INVALID_PARAMETER;
        goto bail;
    }

    if ((eValueType == IPTV_HAL_DECODER_VALUETYPE_VIDEO_CURRENTPTS)
        || (eValueType == IPTV_HAL_DECODER_VALUETYPE_AUDIO_CURRENTPTS))
    {
        if (*pValueDataLength == sizeof(IPTV_HAL_DECODER_TIME))
        {
            IPTV_HAL_DECODER_TIME dtNow = 0;

            retVal = IPTV_HAL_Decoder_Clock_GetTime(pSDecoderContext->pSDecoderClockContext, &dtNow);
            if (IPTV_HAL_ERROR_RET_FAILED(retVal))
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_GetValue[%u] IPTV_HAL_Decoder_Clock_GetTime FAILED[0x%x]\n",
                    pSDecoderContext->uiInstance, retVal));
                goto bail;
            }

            // Check for underrun (unless decoder has just been flushed)
            if (pSDecoderContext->latestPreparePTS && (pSDecoderContext->latestPreparePTS < dtNow))
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_GetValue[%u] underrun by %u; returning latest Prepare PTS, %llu %llu \n",
                    pSDecoderContext->uiInstance,
                    (unsigned int)(dtNow - pSDecoderContext->latestPreparePTS),
                    dtNow , pSDecoderContext->latestPreparePTS));

                dtNow = pSDecoderContext->latestPreparePTS;
            }

            *((IPTV_HAL_DECODER_TIME*)pValueData) = dtNow;

            HAL_DECODER_MSG(("IPTV_HAL_Decoder_GetValue(%u, %s PTS: %llu / %.3lfs)\n",
                pSDecoderContext->uiInstance,
                eValueType == IPTV_HAL_DECODER_VALUETYPE_VIDEO_CURRENTPTS ? "video" : "audio",
                *((IPTV_HAL_DECODER_TIME*)pValueData),
                (double) dtNow / c_dblTimescale90KHz));

            retVal = IPTV_HAL_ERROR_SUCCESS;
        }
    }
    else if ( eValueType == IPTV_HAL_DECODER_VALUETYPE_STREAM_GET_RENDERDONE )
    {
        if (!s_IsDecodingInhibited)
        {
            if (*pValueDataLength == sizeof(bool))
            {
                s32 isDone = vdec_check_exit();
#if 0
				if (pkFAILED( CAVRendererGst::GetRenderingDone(s_hAVRendererCtx, pSDecoderContext->hAVRDecoder, &isDone) ) )
                {
                    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_GetValue(%u, rendering done failed)\n",
                        pSDecoderContext->uiInstance));

                    return IPTV_HAL_ERROR_FAILED;
                }
#endif
                bool isStalled;
                uint32_t isStalledSize = sizeof(isStalled);

                if (IPTV_HAL_ERROR_RET_FAILED(IPTV_HAL_Decoder_GetValue(pvDecoderContext,
                                                                       IPTV_HAL_DECODER_VALUETYPE_STREAM_GET_DECODERSTALL,
                                                                       &isStalled,
                                                                       &isStalledSize)))
                {
                    isStalled = FALSE;
                }

                *((bool *)pValueData) = (isDone || isStalled);

                HAL_DECODER_MSG(("IPTV_HAL_Decoder_GetValue(%u, rendering done: %s)\n",
                    pSDecoderContext->uiInstance,
                    *((bool *)pValueData) ? "TRUE" : "FALSE"));

                retVal = IPTV_HAL_ERROR_SUCCESS;
            }
        }
    }
    else if ( eValueType == IPTV_HAL_DECODER_VALUETYPE_STREAM_GET_DECODERSTALL )
    {
        if (!s_IsDecodingInhibited)
        {
            if (*pValueDataLength == sizeof(bool))
            {
                bool isStalled = FALSE;//Set false as default to fix the skip frame issue.
                //bool isStalled = TRUE;
                //if( pSDecoderContext->pSDecoderClockContext->isClockRunning )
                {
                    //if( pkFAILED( CAVRendererGst::GetDecoderStalled(s_hAVRendererCtx, pSDecoderContext->hAVRDecoder, s_Get90kHzTime(), &isStalled) ) )
                    //{
                    //    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_GetValue(%u, rendering stalled failed)\n",
                    //        pSDecoderContext->uiInstance));
                    //    return IPTV_HAL_ERROR_FAILED;
                    //}
                }

                *((bool *)pValueData)  = isStalled;

                HAL_DECODER_MSG(("IPTV_HAL_Decoder_GetValue(%u, decoder stalled: %s)\n",
                    pSDecoderContext->uiInstance,
                    *((bool *)pValueData) ? "TRUE" : "FALSE"));

                retVal = IPTV_HAL_ERROR_SUCCESS;
            }
        }
    }
bail:
    return retVal;
}

/// <summary>
/// This function is used by the application to "set" a specified Value from the decoder.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="eValueType">[IN] Indicates the decoder value to "Set"</param>
/// <param name="pValueData">[IN] Pointer to the decoder parameter data</param>
/// <param name="nValueDataLength">[IN] Length of pValueData</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid parameter</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_SetValue(
    LPVOID pvDecoderContext,
    IPTV_HAL_DECODER_VALUETYPE eValueType,
    const LPVOID pValueData,
    UINT32 nValueDataLength)
{
    SDecoderContext* pSDecoderContext = (SDecoderContext*) pvDecoderContext;
    if (NULL == pSDecoderContext )
    {
        HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue no decoder context eValueType=%d", eValueType));
        return IPTV_HAL_ERROR_NOT_INITIALIZED;
    }

    switch (eValueType)
    {
        case IPTV_HAL_DECODER_VALUETYPE_VIDEO_WMV9:
            HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue(%u, WMV9, len %d)\n",
                pSDecoderContext->uiInstance, nValueDataLength));

            pSDecoderContext->uiHeaderLength = nValueDataLength;
            pSDecoderContext->pvHeaderData = pValueData;
            break;

        case IPTV_HAL_DECODER_VALUETYPE_AUDIO_WMA:
            HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue(%u, WMA, len %d)\n",
                pSDecoderContext->uiInstance, nValueDataLength));

            pSDecoderContext->uiHeaderLength = nValueDataLength;
            pSDecoderContext->pvHeaderData = pValueData;
            break;

        case IPTV_HAL_DECODER_VALUETYPE_VIDEO_PICTUREINFO_EX:
            if (nValueDataLength != sizeof(pSDecoderContext->pictureInfoValue))
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] invalid picture info ex length %d\n",
                    pSDecoderContext->uiInstance, nValueDataLength));
                return IPTV_HAL_ERROR_INVALID_PARAMETER;
            }
            pSDecoderContext->pictureInfoValue = *((const PIPTV_HAL_DECODER_VALUE_VIDEO_PICTUREINFO_EX) pValueData);

            HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue picInfoEx size(%d,%d,%d,%d) DTS:%0.3lfs dur:%0.3lfs Flags:0x%x\n ",
                (unsigned int)pSDecoderContext->pictureInfoValue.u16Width,
                (unsigned int)pSDecoderContext->pictureInfoValue.u16Height,
                (unsigned int)pSDecoderContext->pictureInfoValue.u16Aspx,
                (unsigned int)pSDecoderContext->pictureInfoValue.u16Aspy,
                (double)(pSDecoderContext->pictureInfoValue.hdtDTS) / c_dblTimescale90KHz,
                (double)(pSDecoderContext->pictureInfoValue.hdtDuration) / c_dblTimescale90KHz,
                (unsigned int)pSDecoderContext->pictureInfoValue.u32Flags));
            break;

        case IPTV_HAL_DECODER_VALUETYPE_VIDEO_ORIGPTS:
            if (nValueDataLength != sizeof(IPTV_HAL_DECODER_TIME))
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] invalid original pts length %d\n",
                    pSDecoderContext->uiInstance, nValueDataLength));
                return IPTV_HAL_ERROR_INVALID_PARAMETER;
            }

            HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue [%u] original pts [%.3lfs]\n",
                pSDecoderContext->uiInstance, (double) (*(IPTV_HAL_DECODER_TIME *)pValueData / c_dblTimescale90KHz)));
            break;

        case IPTV_HAL_DECODER_VALUETYPE_DRM_SETHANDLE:

            if (nValueDataLength != sizeof(pSDecoderContext->hDecrypter))
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] invalid DRM handle length %d\n",
                    pSDecoderContext->uiInstance, nValueDataLength));
                return IPTV_HAL_ERROR_INVALID_PARAMETER;
            }
            pSDecoderContext->hDecrypter = *((UINT32 *) pValueData);

            HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue [%u] DRM handle [%u]\n",
                pSDecoderContext->uiInstance, pSDecoderContext->hDecrypter ));
            break;

        case IPTV_HAL_DECODER_VALUETYPE_DRM_SETKEYID:

            if (0 == pSDecoderContext->hDecrypter)
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] DRM key ID; no sample decrypter context",
                    pSDecoderContext->uiInstance));
                return IPTV_HAL_ERROR_INVALID_PARAMETER;
            }
            else
            {
                HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue [%u] DRM key ID length %d",
                    pSDecoderContext->uiInstance, nValueDataLength));
            }
            break;

        case IPTV_HAL_DECODER_VALUETYPE_DRM_SETSAMPLEID:

            if (0 == pSDecoderContext->hDecrypter)
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] DRM sample ID; no sample decrypter context",
                    pSDecoderContext->uiInstance));
                return IPTV_HAL_ERROR_INVALID_PARAMETER;
            }
            else
            {
                UINT64 sampleID;

                if (nValueDataLength != sizeof(sampleID))
                {
                    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] invalid DRM sample ID length %d\n",
                        pSDecoderContext->uiInstance, nValueDataLength));
                    return IPTV_HAL_ERROR_INVALID_PARAMETER;
                }
                sampleID = *((UINT64 *) pValueData);

                HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue [%u] Sample ID [%llu]\n",
                    pSDecoderContext->uiInstance, sampleID));
            }
            break;

        case IPTV_HAL_DECODER_VALUETYPE_DRM_SETOPL:
            IPTV_HAL_DECODER_DRM_SETOPL   oplValue;

            if (nValueDataLength != sizeof(oplValue))
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue invalid OPL value length %d\n",
                    nValueDataLength));
                return IPTV_HAL_ERROR_INVALID_PARAMETER;
            }
            oplValue = *((const PIPTV_HAL_DECODER_DRM_SETOPL) pValueData);

            switch (oplValue.oplType)
            {
            case IPTV_HAL_DECODER_DRM_SETOPL_TYPE_HDCP_DISABLE:
                break;
            case IPTV_HAL_DECODER_DRM_SETOPL_TYPE_HDCP_ENABLE_ALWAYS:
                break;
            case IPTV_HAL_DECODER_DRM_SETOPL_TYPE_HDCP_ENABLE_DOWN_RES:
                break;
            case IPTV_HAL_DECODER_DRM_SETOPL_TYPE_COMPONENT_DISABLE:
                break;
            case IPTV_HAL_DECODER_DRM_SETOPL_TYPE_COMPONENT_ENABLE:
                break;
            default:
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] invalid OPL type value %d\n",
                    (unsigned int)oplValue.pipeIdN,
                    (int)oplValue.oplType));
                return IPTV_HAL_ERROR_INVALID_PARAMETER;
            }
            HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue [%u] Set OPL %d\n",
                (unsigned int)oplValue.pipeIdN,
                (int)oplValue.oplType));
            break;

        case IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_EOS:
            if (!s_IsDecodingInhibited)
            {
                bool endOfStream;
                if (nValueDataLength != sizeof(endOfStream))
                {
                    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] invalid stream end length %d\n",
                        pSDecoderContext->uiInstance, nValueDataLength));
                    return IPTV_HAL_ERROR_INVALID_PARAMETER;
                }
                endOfStream = *((bool *) pValueData);
                //if( pkFAILED( CAVRendererGst::SetEndOfStream(s_hAVRendererCtx, pSDecoderContext->hAVRDecoder, endOfStream)) )
                //{
                //    HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] failed to set EOS\n",
                //        pSDecoderContext->uiInstance));
                //    return IPTV_HAL_ERROR_FAILED;
                //}
                if(endOfStream == TRUE){
                    if(g_audio_desc.streamID == pSDecoderContext->uiInstance){
                        g_audio_desc.isEsEnd = TRUE;
                        Executive_DebugPrintf("audio eos!!!\n");
                    }else if(g_video_desc.streamID == pSDecoderContext->uiInstance){
                        g_video_desc.isEsEnd = TRUE;
		                Executive_DebugPrintf("video eos, push 1024 eos!!!\n");

                        char tmp_data[1024] = {0};
            			memset(tmp_data, 0, 1024);
            			/*+++++++++++Start+++++++++++*/
            			/* To push the original es out in wb_case_9902, and can't get the EOS any more by MT_UNF_AVPLAY_IsBuffEmpty */
            			/* and MT_UNF_AVPLAY_GetStatusInfo with this way*/
            			vdec_dec_push_es(NULL, (u32)(tmp_data), 1024, (u64)0, 0);
            			vdec_dec_push_es(NULL, (u32)(tmp_data), 1024, (u64)0, 0);
            			vdec_dec_push_es(NULL, (u32)(tmp_data), 1024, (u64)0, 0);
            			/*--------------End---------------*/
            			vdec_dec_push_es(NULL, (u32)(tmp_data), 1024, (u64)0, 1);//zhouxiang, push eos flag
                    }
                }
                if(g_audio_desc.isEsEnd  ||  g_video_desc.isEsEnd) {
                    /*u32 cur_ves_wl = 0;
                    u32 video_free_size = 0;
                    u32 audio_free_size = 0;
                    u32 video_total_size = 0;
                    u32 audio_total_size = 0;
                    video_decoder_get_es_buf_space(&video_free_size,&video_total_size,&audio_free_size,&audio_total_size);
                    cur_ves_wl = video_total_size - video_free_size;
                    Executive_DebugPrintf("EOS cur_ves_wl[%dKB], video_free_size[%d] audio_free_size[%d]\n", cur_ves_wl,video_free_size, audio_free_size);
                    if(buffer_status == MSS_BUFFERING){
                        Executive_DebugPrintf("resume decoder!!!\n");
                        buffer_status = MSS_BUFFERING_END;
                        video_decoder_resume();
                        if(g_audio_desc.g_a_dec != NULL){
                            audio_decoder_resume(g_audio_desc.g_a_dec);
                        }
                     }*/
                }
                HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue [%u] Set EoS %s\n",
                    pSDecoderContext->uiInstance,
                    *((bool *) pValueData) ? "TRUE" : "FALSE" ));
            }
            break;

        case IPTV_HAL_DECODER_VALUETYPE_STREAM_SET_PLAYRATE:
            float playrate;
            if (nValueDataLength != sizeof(playrate))
            {
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] invalid playrate length %d\n",
                    pSDecoderContext->uiInstance, nValueDataLength));
                return IPTV_HAL_ERROR_INVALID_PARAMETER;
            }
            playrate = *((float *) pValueData);

            HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue [%u] Set playrate %.2f\n",
                pSDecoderContext->uiInstance,
                playrate));
            /*
            printf("zx IPTV_HAL_Decoder_SetValue [%u] Set playrate %.2f\n",
                pSDecoderContext->uiInstance,
                playrate);
            */

            if(g_avplay_info.last_playrate != (int)playrate)
            {
                if(g_avplay_info.last_playrate != 0)
                {
                    g_avplay_info.last_getpts = 0;
                    g_avplay_info.flush_state = 10;
                    vdec_set_trick_mode_2((mt_s32)playrate);

                    if((int)playrate == 0 || (int)playrate == 1)
                    {
                        //printf("zx change to normal speed!!\n");
                        vdec_reset();//fix bug change to normal speed can not resume to play
                        g_avplay_info.flush_state = 1;
                        //g_avplay_info.flush_state = 0;
                    }
                }
                g_avplay_info.last_playrate = (int)playrate;
            }
            else
                HAL_DECODER_ERRMSG(("IPTV_HAL_Decoder_SetValue [%u] Set same playrate %.2f, donothing\n",
                    pSDecoderContext->uiInstance,
                    playrate));
            break;

        default:
            HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetValue(%u, type 0x%x, len %d) IGNORED\n",
                pSDecoderContext->uiInstance, eValueType, nValueDataLength));
            break;
    }


    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function is used by the application to set the decoder to I-Frame-Only mode.
/// </summary>
/// <param name="pDecoderContext">[IN] Pointer to the decoder context </param>
/// <param name="bIFrameOnly">[IN] If true, decoder is set to decode only I-Frames. If false, the decoder is set to decode all frames</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Unable to set/reset the decoder into/from I-Frame mode</para>
/// </returns>
/// <remarks>
/// This API may not be necessary for decoders that do not require I-Frame-Only as a separate mode.
/// For field encoded streams, only the top field or the bottom field will be provided to the decoder.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_SetPlayMode (LPVOID pvDecoderContext, bool bIFrameOnly)
{
    SDecoderContext* pSDecoderContext = (SDecoderContext*) pvDecoderContext;
    pSDecoderContext->bIFrameOnly = bIFrameOnly;

    HAL_DECODER_MSG(("IPTV_HAL_Decoder_SetPlayMode(%u, %s)\n",
        pSDecoderContext->uiInstance, bIFrameOnly ? "IFrameOnly" : "AnyFrame" ));
    /*
    printf("zx IPTV_HAL_Decoder_SetPlayMode(%u, %s)\n",
        pSDecoderContext->uiInstance, bIFrameOnly ? "IFrameOnly" : "AnyFrame" );
    */

    //CAVRendererGst::SetPlayRate(s_hAVRendererCtx, (int32_t) !bIFrameOnly);
    return IPTV_HAL_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IPTV_HAL_Decoder_Clock
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <topic name="STC" displayname="System Time Clock (STC) Control APIs"> </topic>

/// <summary>
/// This function is used by the application to acquire an available clock.
/// </summary>
/// <param name="bAVSync">[IN] If true, the clock will be used to synchronize audio and video</param>
/// <param name="ppClockContext">[OUT] Pointer to receive the acquired HAL clock context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: No available clock contexts</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_Clock_Acquire(bool bAVSync, LPVOID *ppvClockContext)
{
    static unsigned int s_uiClockInstance = 0;

    if (NULL == ppvClockContext)
        return IPTV_HAL_ERROR_INVALID_PARAMETER;

    SDecoderClockContext* pSDecoderClockContext = new SDecoderClockContext;

    if (NULL == pSDecoderClockContext)
        return IPTV_HAL_ERROR_OUT_OF_MEMORY;

    pSDecoderClockContext->uiInstance = ++s_uiClockInstance;

    *ppvClockContext = pSDecoderClockContext;

    HAL_CLOCK_MSG(("IPTV_HAL_Decoder_Clock_Acquire(bAVSync %d) context: %u\n",
        (int)bAVSync,
        pSDecoderClockContext->uiInstance));

    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function is used by the application to initialize an STC.
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
/// <remarks>
/// The decoder at this point should complete all initialization necessary and have the
/// clock driver APIs ready to start.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Clock_Init(LPVOID pvClockContext)
{
    SDecoderClockContext* pDCC = (SDecoderClockContext*) pvClockContext;

    struct timeval tvNow;
    gettimeofday(&tvNow, NULL);
    g_avplay_info.last_time = tvNow.tv_sec*1000 + (uint32_t)tvNow.tv_usec/1000;
 
    if(pvClockContext == NULL)
    {
        return IPTV_HAL_ERROR_SUCCESS;
    }
    HAL_CLOCK_MSG(("IPTV_HAL_Decoder_Clock_Init(context %u)\n", pDCC->uiInstance));

    pDCC->dtStart= 0;
    pDCC->dtEpoch= 0;
    pDCC->dtStoppedTime= 0;
    pDCC->isClockRunning = false;

    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function is used by the application to set the clock start time
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context </param>
/// <param name="u64Time90khz">[IN] contains the time in 90KHz units to which the STC is being set </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
/// <remarks>
/// Note that the clock should not be started at the time of calling this API.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Clock_SetTime(LPVOID pvClockContext, IPTV_HAL_DECODER_TIME u64Time90khz)
{
    SDecoderClockContext* pDCC = (SDecoderClockContext*) pvClockContext;

    HAL_CLOCK_MSG(("IPTV_HAL_Decoder_Clock_SetTime(context %u) %.3lfs\n",
        pDCC->uiInstance,
        (double)u64Time90khz / c_dblTimescale90KHz));

    pDCC->dtStart = u64Time90khz;
    pDCC->dtEpoch = 0;
    pDCC->dtStoppedTime = 0;
    pDCC->isClockRunning = false;

    if (!s_IsDecodingInhibited )
    {
		#if 0
		pkRESULT hr =
            CAVRendererGst::SetPCR(
                pDCC->hAVRendererCtx,
                (uint64_t)((u64Time90khz + 45)/90)); // convert 90KHz to 1KHz

        if (pkFAILED(hr))
        {
            HAL_CLOCK_ERRMSG(("CAVRendererGst::SetPCR failed [0x%x]\n", hr));
            return IPTV_HAL_ERROR_FAILED;
        }
		#endif
    }

    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function is used by the application to get the current time from the clock
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context </param>
/// <param name="pu64Time90khz">[OUT] Contains the time in 90KHz units obtained from the STC </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
/// <remarks>
/// The STC value returned by the decoder is used by the Microsoft application to validate the
/// clock prior to starting. It is also used to validate PTS of data pushed into the decoder.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Clock_GetTime(LPVOID pvClockContext, IPTV_HAL_DECODER_TIME *pDecoderTimeResult)
{
    if (NULL == pDecoderTimeResult)
        return IPTV_HAL_ERROR_INVALID_PARAMETER;

    SDecoderClockContext* pDCC = (SDecoderClockContext*) pvClockContext;

    //if (!s_IsDecodingInhibited
    //    && (pDCC->isClockRunning || 0 != pDCC->dtStoppedTime)) // running or paused
    if (g_audio_desc.streamID != -1 && g_video_desc.streamID != -1)
    {

        IPTV_HAL_DECODER_TIME decoderTime90kHz;
        uint64_t hnsCurrentPTS = 0;
        #if 0
        pkRESULT hr = CAVRendererGst::GetCurrentPTS(pDCC->hAVRendererCtx, &hnsCurrentPTS);
        if (pkFAILED(hr))
        {
            HAL_CLOCK_ERRMSG(("CAVRendererGst::GetCurrentPTS FAILED [0x%x]\n", hr));
            return IPTV_HAL_ERROR_FAILED;
        }
		#endif
        struct timeval tvNow;
        gettimeofday(&tvNow, NULL);
        int cur_time = tvNow.tv_sec*1000 + (uint32_t)tvNow.tv_usec/1000;
        if((cur_time - g_avplay_info.last_time) > MIN_GET_PTS_INTERVAL)
        {
          g_avplay_info.last_time = cur_time;
    	    vdec_get_pts_2((s64 *)&hnsCurrentPTS);//pts is 10*9  zx22
    	    //printf("get pts %llx\n", hnsCurrentPTS);

            //decoderTime90kHz = hnsCurrentPTS / 1000; // convert to 1MHz first to prevent overflow
            decoderTime90kHz = hnsCurrentPTS * 9 / 100; // then convert 1MHz -> 90KHz

            if(g_avplay_info.flush_state == 2)
            {
                //printf("zx flush_state 2 decoderTime90kHz %llu, g_avplay_info.last_getpts %llu\n",
                //        decoderTime90kHz, g_avplay_info.last_getpts);
                if(ABS(decoderTime90kHz - g_avplay_info.last_getpts) < 50000 && decoderTime90kHz >= g_avplay_info.last_getpts)
                {
                    //printf("zx after flush, avsync pts udpate!!! decoderTime90kHz %llu, g_last_getpts %llu\n",
                    //    decoderTime90kHz, g_avplay_info.last_getpts);
                    g_avplay_info.flush_state = 0;
                }
            }

            if(decoderTime90kHz >= g_avplay_info.last_getpts && g_avplay_info.flush_state != 2)
            {
                *pDecoderTimeResult = decoderTime90kHz;
                g_avplay_info.last_getpts = decoderTime90kHz;
                //printf("zx hnsCurrentPTS:%lld \n", hnsCurrentPTS);
                //if(g_avplay_info.flush_state == 10)
                //    printf("zx decoderTime90kHz %llu \n", decoderTime90kHz);
            }
            else if(g_avplay_info.flush_state == 10 && g_avplay_info.last_playrate < 0)
            {
                *pDecoderTimeResult = decoderTime90kHz;
                g_avplay_info.last_getpts = decoderTime90kHz;
                //printf("zx hnsCurrentPTS:%lld \n", hnsCurrentPTS);
                //if(g_avplay_info.flush_state == 10)
                //    printf("zx decoderTime90kHz %llu \n", decoderTime90kHz);
            }
            else
            {
                //printf("zx error decoderTime90kHz %llu, g_avplay_info.last_getpts %llu\n", decoderTime90kHz, g_avplay_info.last_getpts);
                *pDecoderTimeResult = g_avplay_info.last_getpts;
            }

            // Periodically log the difference between renderer and system time
            {
                static IPTV_HAL_DECODER_TIME latestLogTime90KHz = 0;

                if (decoderTime90kHz < latestLogTime90KHz)
                {
                    latestLogTime90KHz = decoderTime90kHz;
                }
                else if ( (decoderTime90kHz - latestLogTime90KHz) >= c_LogPeriod_RenderClockDiff90kHz)
                {
                    latestLogTime90KHz = decoderTime90kHz;

                    double dblDecoderTimeSecs = decoderTime90kHz / c_dblTimescale90KHz;

                    IPTV_HAL_DECODER_TIME sysTime90kHz =
                        (pDCC->isClockRunning ? s_Get90kHzTime() : pDCC->dtStoppedTime)
                        - pDCC->dtEpoch
                        + pDCC->dtStart;

                    HAL_CLOCK_OUTPUT(("IPTV_HAL_Decoder_Clock_GetTime(%u) decoder time (%.3lf) - systime = %.3lf seconds\n",
                        pDCC->uiInstance,
                        dblDecoderTimeSecs,
                        dblDecoderTimeSecs - (sysTime90kHz / c_dblTimescale90KHz)));
                }
            }
        }
        else
            *pDecoderTimeResult = g_avplay_info.last_getpts;
    }
    else // decoder time is not available or never been started
    {
        // use clock based on system time
        /**pDecoderTimeResult =
            (pDCC->isClockRunning ? s_Get90kHzTime() : pDCC->dtStoppedTime)
            - pDCC->dtEpoch
            + pDCC->dtStart;*/
            *pDecoderTimeResult = 0; //if not 0, maybe wrong
    }

#ifdef HAL_TRACE_ENABLE_CLOCK_GETTIME

    HAL_CLOCK_MSG(("IPTV_HAL_Decoder_Clock_GetTime(context %u) %.3lfs\n",
        pDCC->uiInstance,
        (double)(*pDecoderTimeResult) / c_dblTimescale90KHz));

#endif // HAL_TRACE_ENABLE_CLOCK_GETTIME

    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function is used by the application to start the clock
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
/// <remarks>
/// Video and audio should be independently synchronized with the hardware STC.
/// The decoder should start the decoder as soon as possible after this Clock_Start is called.
/// </remarks>
iptv_hal_error IPTV_HAL_Decoder_Clock_Start(LPVOID pvClockContext)
{
    SDecoderClockContext* pDCC = (SDecoderClockContext*) pvClockContext;

    if (pDCC->isClockRunning)
    {
        HAL_CLOCK_MSG(("IPTV_HAL_Decoder_Clock_Start(context %u) **ALREADY RUNNING**\n", pDCC->uiInstance));
    }
    else // clock needs to be started
    {
        HAL_CLOCK_MSG(("IPTV_HAL_Decoder_Clock_Start(context %u) at %.3lfs\n",
            pDCC->uiInstance,
            (double)(pDCC->dtEpoch) / c_dblTimescale90KHz));

        if (!s_IsDecodingInhibited )
        {
            pkRESULT hr;
            if (0 == pDCC->dtStoppedTime)
            {
                //hr = CAVRendererGst::Start(
                //        pDCC->hAVRendererCtx,
                //        pkSTREAM_SELECTION_FLAG_AUDIO_AND_VIDEO,
                //        -1); // i.e. GST_CLOCK_TIME_NONE: use the CAVRendererGst::SetPCR time
            }
            else
            {
                //hr = CAVRendererGst::Resume(pDCC->hAVRendererCtx);
            }

            if (pkFAILED(hr))
            {
                HAL_CLOCK_ERRMSG(("CAVRendererGst::%s failed [0x%x]\n",
                    (0 == pDCC->dtStoppedTime) ? "Start" : "Resume",
                    hr));
                return IPTV_HAL_ERROR_FAILED;
            }
        }

        pDCC->dtEpoch += s_Get90kHzTime() - pDCC->dtStoppedTime;
        pDCC->isClockRunning = true;
    }

    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function is used by the application to stop the clock
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_Clock_Stop(LPVOID pvClockContext)
{
    SDecoderClockContext* pDCC = (SDecoderClockContext*) pvClockContext;
    if(pvClockContext == NULL)
    {
        return IPTV_HAL_ERROR_SUCCESS;
    }

    if ( !pDCC->isClockRunning)
    {
        HAL_CLOCK_MSG(("IPTV_HAL_Decoder_Clock_Stop(context %u) **ALREADY STOPPED**\n", pDCC->uiInstance));
    }
    else // clock needs to be stopped
    {
        HAL_CLOCK_MSG(("IPTV_HAL_Decoder_Clock_Stop(context %u) at %.3lfs\n",
            pDCC->uiInstance,
            (double)(pDCC->dtStoppedTime - pDCC->dtEpoch) / c_dblTimescale90KHz));

        if (!s_IsDecodingInhibited )
        {
        	#if 0
            pkRESULT hr = CAVRendererGst::Pause(pDCC->hAVRendererCtx);

            if (pkFAILED(hr))
            {
                HAL_CLOCK_ERRMSG(("CAVRendererGst::Pause failed [0x%x]\n", hr));
                return IPTV_HAL_ERROR_FAILED;
            }
			#endif
        }

        pDCC->dtStoppedTime = s_Get90kHzTime();
        pDCC->isClockRunning = false;
    }

    return IPTV_HAL_ERROR_SUCCESS;
}

/// <summary>
/// This function is used by the application to release a previously acquired system clock context
/// </summary>
/// <param name="pClockContext">[IN] Pointer to the system clock context that is to be released</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: Invalid system clock context</para>
/// </returns>
iptv_hal_error IPTV_HAL_Decoder_Clock_Release(LPVOID pvClockContext)
{
    SDecoderClockContext* pDCC = (SDecoderClockContext*) pvClockContext;

    HAL_CLOCK_MSG(("IPTV_HAL_Decoder_Clock_Release(context %u)\n", pDCC->uiInstance));

    if (!s_IsDecodingInhibited
        && (pDCC->isClockRunning || 0 != pDCC->dtStoppedTime)) // clock has been started
    {
    	#if 0
        pkRESULT hr = CAVRendererGst::Stop(pDCC->hAVRendererCtx, pkSTREAM_SELECTION_FLAG_AUDIO_AND_VIDEO);

        if (pkFAILED(hr))
        {
            HAL_CLOCK_ERRMSG(("CAVRendererGst::Stop failed [0x%x]\n", hr));
            // return IPTV_HAL_ERROR_FAILED;
        }
		#endif
    }

    delete pDCC;

    return IPTV_HAL_ERROR_SUCCESS;
}

/*start add for seek optimize function*/

static bool _isSeekFlag = true;//add for hal flush

iptv_hal_error IPTV_HAL_Decoder_SetSeekFlag(bool flag)
{
	_isSeekFlag = flag;
    return IPTV_HAL_ERROR_SUCCESS;
}

bool IPTV_HAL_Decoder_GetSeekFlag()
{
	return _isSeekFlag;
}

/*end add for seek optimize function*/

