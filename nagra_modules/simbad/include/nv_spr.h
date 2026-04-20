/**
 * @file nv_spr.h
 * @brief Nagravision Stream Processing API - definition
 *
 * This file defines a set of API to abstract stream processing out of
 * the platform specific implementation, as a generic integration 
 * interface shared by all required platforms. In order to deploy Nagra
 * integration test app on a specific platform, one need to implement 
 * this Stream Processing API, along with required DAL and TAL drivers.
 *
 * Copyright (c) 2019, Nagravision SA
 * All rights reserved.
 *
 * This source code is property of Nagravision SA and provided for your 
 * reference only.
 * 
 * Strictly Confidential.
 * Redistribution is not permitted.
 *
 * THIS SOFTWARE IS PROVIDED BY NAGRAVISION SA ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NAGRAVISION SA BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NV_SPR_H
#define NV_SPR_H

#ifdef __cplusplus
extern "C" {
#endif

// ==========================================================================
// Include Files
// ==========================================================================

#include "ca_defsx.h"

// ==========================================================================
// API Version
// ==========================================================================

#define NV_SPR_VERSION "1.3.0"

// ==========================================================================
// Type Definitions
// ==========================================================================

/**
 * @brief Status code returned by functions of the Stream Processing API
 */
typedef enum
{
  NV_SPR_NO_ERROR,
  NV_SPR_ERROR_BAD_PARAM,
  NV_SPR_ERROR_TIMEOUT,
  NV_SPR_ERROR,
  NV_SPR_ERROR_LAST
} TNvSprStatus;

/**
 * @brief Structure defining PID
 *
 * This structure identifies an elementary stream pid and associated 
 * stream type (similar to a PMT) to be managed in a DVB stream session.
 * Stream types are specified in ISO/IEC 13818-1, table 2-29
 */
typedef struct
{
  TUnsignedInt16 pid;
  TUnsignedInt16 streamType;
} TNvSprPidInfo;

/**
 * @brief Structure defining PID List
 *
 * This structure defines a list of elementary stream pids and types
 * associated to a DVB stream session.
 */
typedef struct
{
  TUnsignedInt16 count;
  TNvSprPidInfo* elems;
} TNvSprPidList;

/**
 * @brief Video Decoder Configuration
 */
typedef struct SNvSprVideoDecoderConfig
{
  TUnsignedInt8  codec[32]; /**< Video CODEC to be used, such as "avc1" "hev1"*/
  TUnsignedInt32 cfgSize;   /**< not used. */
  TUnsignedInt8* cfgData;   /**< not used. */
} TNvSprVideoDecoderConfig;

/**
 * @brief Audio Decoder Configuration
 */
typedef struct SNvSprAudioDecoderConfig
{
  TUnsignedInt8  codec[32];    /**< Audio CODEC to be used, such as "mp4a" */
  TUnsignedInt32 channelCount; /**< not used. */
  TUnsignedInt32 sampleSize;   /**< not used. */
  TUnsignedInt32 sampleRate;   /**< not used. */
  TUnsignedInt32 cfgSize;      /**< not used. */
  TUnsignedInt8* cfgData;      /**< not used. */
} TNvSprAudioDecoderConfig;

/**
 * @brief OTT Stream Type for provision of descrambled content
 */
typedef enum
{
  RAW_SAMPLES,
  TS_CHUNKS
} TNvSprOttStreamType;

/**
 * @brief Preferred SEC driver version to use when opening sessions
 */
typedef enum
{
  SEC_AUTO, /**< Auto-select best fitting SEC API version */
  SECv5,    /**< Force use legacy SEC API version 5 */
  SECv6     /**< Force use newer  SEC API version 6 */
} TNvSprSecVersion;

/**
 * @brief HDMI Display Mode (in case of concurrent sessions)
 */
typedef enum
{
  DISPLAY_AUTO, /**< First session in full screen. Additional session(s) in PiP */
  PIP_ONLY,     /**< First session in PiP.         Other sessions not displayed */
  FULL_ONLY     /**< First session in full screen. Other sessions not displayed */
} TNvSprDisplayMode;

// ==========================================================================
// Initialization/Termination/Debug
// ==========================================================================

/**
 * @brief Platform specific initialization
 *
 * This function is called at startup of the integration test app.
 * Platform specific implementation shall do all necessary initialization
 * work in this function.
 */
void nvSprInitialize();

/**
 * @brief Platform specific termination
 *
 * This function is called at termination of the integration test app. 
 * Platform specific implementation shall do all necessary finalization
 * work in this function.
 */
void nvSprTerminate();

/**
 * @brief Platform specific reboot
 *
 * This function is called to force a reboot of the integration platform.
 * After reboot, the integration test app shall be automatically restarted
 * and resume full functional state.
 */
void nvSprReboot();

/**
 * @brief Platform specific logging level
 *
 * This function allows changing the log level of the Stream Processing API.
 * By default, the level should be minimum (=0) and only log errors.
 */
void nvSprSetDebug(TUnsignedInt16 level);

// ==========================================================================
// DVB/OTT Transport Session Common
// ==========================================================================

/**
 * @brief Get video smp capability
 *
 * This function is just to get the information that video smp is supported
 * or not.
 *
 * @retval TBoolean
 *  Result status of the video smp
 *  Boolean value, <em>TRUE</em> to support SMP, <em>FALSE</em> not support.
 */
TBoolean nvSprGetVideoSmp();

/**
 * @brief Get audio smp capability
 *
 * This function is just to get the information that audio smp is supported
 * or not.
 *
 * @retval TBoolean
 *  Result status of the audio smp
 *  Boolean value, <em>TRUE</em> to support SMP, <em>FALSE</em> not support.
 */
TBoolean nvSprGetAudioSmp();

/**
 * @brief Obtain stream descrambling status of the transport session
 *
 * Stream descrambling status can be requested after the specific transport
 * session has been opened and enough packets injected. It should reflect
 * the real descrambling status, meaning that the A/V stream can be
 * properly decoded and rendered on HDMI output.
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[out] status
 *  Pointer to obtain the stream descrambling status boolean value.
 *  <em>TRUE</em> indicates stream is being descrambled
 *  <em>FALSE</em> indicates stream is not being descrambled
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprGetDescrambling
(
  TTransportSessionId tsid,
  TBoolean*           status
);

/**
 * @brief Close transport session
 *
 * This function stops stream processing in the given transport session
 * and releases any resources allocated to that session.
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprCloseSession
(
  TTransportSessionId tsid
);

TNvSprStatus nvSprBlackScreen
(
  TTransportSessionId tsid
);

// ==========================================================================
// DVB Live Descrambling
// ==========================================================================

/**
 * @brief Open DVB descrambling transport session
 *
 * The platform specific implementation shall do all necessary works related
 * to the transport session in order to make the stream descrambling start (e.g.
 * add pids in the pipeline demux if applicable, start stream descrambler, 
 * allocate stream output buffer, configure HDMI output, etc.).
 *
 * By default, the initial SMP status shall be disabled.
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] pids
 *  List of elementary stream PIDs and types to be descrambled and decoded
 *  on HDMI output.
 * @param[in] smp
 *  Boolean value, <em>TRUE</em> to activate SMP, <em>FALSE</em> to deactivate.
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprOpenDvbSession
(
  TTransportSessionId tsid,
  TNvSprPidList*      pids,
  TBoolean            smp
);

/**
 * @brief Inject stream data in the transport session
 *
 * After transport session is opened, this function is called to inject 
 * stream data (MPEG packets) into the processing chain. 
 * Processed descrambled packets shall either be sent out for decoding
 * and displaying on the HDMI output, or stored after rescrambling in PVR 
 * use case.
 *
 * @attention
 * 
 * <b>An adequate buffering and flow control mechanism must be implemented
 * under this function. The client may invoke it at high rate with only
 * few - possibly one - MPEG packet(s) at a time. It is thus advisable to 
 * buffer these packets for further processing, depending on the capabilities
 * of the descrambling/decoding pipeline.
 * In addition, if the caller is filling up the buffer too fast compared
 * to the consumption rate of the decoder, the function should block the 
 * caller until some space is available again in the buffer. The goal
 * is to have an injection flow control properly driven by the decoding 
 * rate.</b>
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] size
 *  Size in bytes of the stream data
 * @param[in] data
 *  The stream data (e.g. a buffer of MPEG packets)
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprInjectData
(
  TTransportSessionId tsid,
  TUnsignedInt32      size,
  TUnsignedInt8*      data
);

// ==========================================================================
// PVR Recording / Replaying
// ==========================================================================

/**
 * @brief Open DVB recording transport session
 *
 * The platform specific implementation shall do all necessary works related
 * to the transport session in order to make the stream recording start (e.g.
 * add pids in the pipeline demux if applicable, start stream descrambler, 
 * start stream rescrambler, open recording file, etc.).
 *
 * By default, the initial SMP status shall be disabled.
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] pids
 *  List of elementary stream PIDs and types to be descrambled, rescrambled
 *  and stored on disk.
 * @param[in] emi
 *  EMI to use for stream rescrambling. It will determine if we perform
 *  raw scrambling or M2TS scrambling on the recorded stream.
 * @param[in] smp
 *  Boolean value, <em>TRUE</em> to activate SMP, <em>FALSE</em> to deactivate.
 * @param[in] filename
 *  Name of file (including path) to store the rescrambled recorded stream.
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprOpenRecording
(
  TTransportSessionId tsid,
  TNvSprPidList*      pids,
  TUnsignedInt16      emi,
  TBoolean            smp,
  const char*         filename
);

/**
 * @brief Open DVB replaying transport session
 *
 * The platform specific implementation shall do all necessary works related
 * to the transport session in order to make the stream replaying start (e.g.
 * add pids in the pipeline demux if applicable, start stream descrambler, 
 * prepare A/V decoder and HDMI output, etc.).
 *
 * By default, the initial SMP status shall be disabled.
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] pids
 *  List of recorded elementary stream PIDs to be played-back.
 * @param[in] emi
 *  EMI to use for stream descrambling. It will determine if we perform
 *  raw descrambling or M2TS descrambling on the recorded stream.
 * @param[in] smp
 *  Boolean value, <em>TRUE</em> to activate SMP, <em>FALSE</em> to deactivate.
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprOpenReplaying
(
  TTransportSessionId tsid,
  TNvSprPidList*      pids,
  TUnsignedInt16      emi,
  TBoolean            smp
);

// ==========================================================================
// OTT protected Buffer Allocate/Release
// ==========================================================================

/**
 * @brief Allocate memory from a general-purpose memory area.
 *
 * This is used for OTT stream processing. It is only used to allocate non-protected
 * buffer when smp is activated.
 *
 * Buffer allocated by this function will be used in SEC <em>processOpaqueData</em>
 * as the input buffer
 * 
 * @param[in] size
 *  Size in bytes of the buffer to be allocated
 * @param[in] smp
 *  always as false, i.e. non-protected buffer.
 * @return
 *  Pointer to the buffer allocated; or NULL if allocation failed.
 */
void* nvSprAllocateMemory(
  TUnsignedInt32 size, 
  TBoolean       smp
);

/**
 * @brief Allocate memory from video-decoder memory area which is distinct from
 * system or OS-controlled memory.
 *
 * This is used for OTT stream processing (video part only). Buffer allocated by
 * this function will be used in SEC <em>processOpaqueData</em> and as the input
 * buffer to video-decoder as well. Platform should free the buffer allocated by
 * this function when it is consumed by the video-decoder.
 * 
 * This function is used for the platform who supports memory access to the
 * video-decoder area directly.
 * 
 * When SMP is not activated, an in-place operation will be used when decrpting.
 * In this case, Buffer allocated by this function will be used as both in and out
 * for SEC <em>processOpaqueData</em>
 *
 * When SMP is activated, no in-place operation will be used. Instead, a protected
 * buffer shall be used. Content in a protected buffer is protected by hardware in
 * a platform specific way. For a protected buffer, the pointer returned by this
 * function is in fact only a handle; real content of the protected buffer cannot be
 * accessed by dereferencing the pointer. In this case, Buffer allocated by this
 * function will be used as out for SEC <em>processOpaqueData</em>
 *
 * @param[in] size
 *  Size in bytes of the buffer to be allocated
 * @param[in] smp
 *  Whether the allocated buffer shall be used with SMP, i.e. protected buffer.
 * @return
 *  Pointer to the buffer allocated; or NULL if allocation failed.
 */
void* nvSprAllocateVideoMemory(
  TUnsignedInt32 size, 
  TBoolean       smp
);

/**
 * @brief Allocate memory from Audio-decoder memory area which is distinct from
 * system or OS-controlled memory.
 *
 * This is used for OTT stream processing (Audio part only). Buffer allocated by
 * this function will be used in SEC <em>processOpaqueData</em> and as the input
 * buffer to Audio-decoder as well. Platform should free the buffer allocated by
 * this function when it is consumed by the Audio-decoder.
 * 
 * This function is used for the platform who supports memory access to the
 * Audio-decoder area directly.
 *
 * When SMP is not activated, an in-place operation will be used when decrpting.
 * In this case, Buffer allocated by this function will be used as both in and out
 * for SEC <em>processOpaqueData</em>
 * 
 * When SMP is activated, no in-place operation will be used. Instead, a protected
 * buffer shall be used. Content in a protected buffer is protected by hardware in
 * a platform specific way. For a protected buffer, the pointer returned by this
 * function is in fact only a handle; real content of the protected buffer cannot be
 * accessed by dereferencing the pointer. In this case, Buffer allocated by this
 * function will be used as out for SEC <em>processOpaqueData</em>
 *
 * @param[in] size
 *  Size in bytes of the buffer to be allocated
 * @param[in] smp
 *  Whether the allocated buffer shall be used with SMP, i.e. protected buffer.
 * @return
 *  Pointer to the buffer allocated; or NULL if allocation failed.
 */
void* nvSprAllocateAudioMemory(
  TUnsignedInt32 size, 
  TBoolean       smp
);

/**
 * @brief Free the memory allocated by @ref nvSprAllocateMemory
 *
 * @param[in] pointer
 *  Pointer to the buffer to be freed; if it is NULL, do nothing.
 */
void nvSprFreeMemory(
  void* pointer
);

// ==========================================================================
// OTT Audio/Video HDMI Rendering
// ==========================================================================

/**
 * @brief Open OTT HDMI rendering session
 *
 * The platform specific implementation shall do all necessary works related
 * to the transport session in order to prepare and allocate resources needed
 * to perform the subsequent audio/video decoding and HDMI output.
 *
 * By default, the initial SMP status shall be disabled.
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] type
 *  Specify the OTT media data type (raw A/V samples or M2TS chunks).
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprOpenOttSession
(
  TTransportSessionId tsid,
  TNvSprOttStreamType type
);

/**
 * @brief Initialize the hardware video decoder for OTT stream
 *
 * This function is used to initialize the hardware video decoder.
 *
 * This is used in the scenario of watching OTT streams on the HDMI display
 * connected to the board. In this scenario, decrypted stream data will
 * be fed into the hardware decoder; but before feeding the stream data, the
 * hardware decoder needs to be initialized first.
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] cfg
 *  Pointer to a structure of video decoder configuration
 * @param[in] smp
 *  Boolean value, <em>TRUE</em> to activate SMP, <em>FALSE</em> to deactivate.
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprInitVideoDecode
(
  TTransportSessionId       tsid,
  TNvSprVideoDecoderConfig* cfg,
  TBoolean                  smp
);

/**
 * @brief Initialize the hardware audio decoder for OTT stream
 *
 * This function is used to initialize the hardware audio decoder.
 *
 * This is used in the scenario of watching OTT streams on the HDMI display
 * connected to the board. In this scenario, decrypted stream data will
 * be fed into the hardware decoder; but before feeding the stream data, the
 * hardware decoder needs to be initialized first.
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] cfg
 *  Pointer to a structure of audio decoder configuration
 * @param[in] smp
 *  Boolean value, <em>TRUE</em> to activate SMP, <em>FALSE</em> to deactivate.
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprInitAudioDecode
(
  TTransportSessionId       tsid,
  TNvSprAudioDecoderConfig* cfg,
  TBoolean                  smp
);

/**
 * @brief Feed descrambled video stream data to the hardware decoder
 *
 * After video stream data is descrambled by SEC <em>processOpaqueData</em>
 * function, this function is invoked to feed the video stream data to the
 * hardware decoder. This is used in the scenario of watching OTT streams
 * on the HDMI display connected to the board.
 *
 * When SMP is activated, the media data is in a protected buffer.
 *
 * @attention
 * 
 * <b>An adequate buffering and flow control mechanism must be implemented
 * under this function. The client may invoke it at high rate with 
 * typically one video frame per call. It is thus advisable to buffer
 * these frames for further processing, depending on the capabilities
 * of the decoding pipeline.
 * In addition, if the caller is filling up the buffer too fast compared
 * to the consumption rate of the decoder, the function should block the 
 * caller until some space is available again in the buffer. The goal
 * is to have an injection flow control properly driven by the decoding 
 * rate.</b>
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] size
 *  Size in bytes of the stream video data
 * @param[in] data
 *  The stream video data buffer
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprVideoDecode
(
  TTransportSessionId tsid,
  TUnsignedInt32      size,
  TUnsignedInt8*      data
);

/**
 * @brief Feed descrambled audio stream data to the hardware decoder
 *
 * After audio stream data is descrambled by SEC <em>processOpaqueData</em>
 * function, this function is invoked to feed the audio stream data to the
 * hardware decoder. This is used in the scenario of watching OTT streams
 * on the HDMI display connected to the board.
 *
 * When SMP is activated, the media data is in a protected buffer.
 *
 * @attention
 * 
 * <b>Please, refer to @ref nvSprVideoDecode for buffering and flow
 * control policy. In principle, the decoding pipeline shall ensure proper
 * audio/video synchronization for a given tsid. However, if audio is
 * somehow stalling, that should not block the video decoding.</b>
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] size
 *  Size in bytes of the stream audio data
 * @param[in] data
 *  The stream audio data buffer
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprAudioDecode
(
  TTransportSessionId tsid,
  TUnsignedInt32      size,
  TUnsignedInt8*      data
);

// ==========================================================================
// HDMI Overlay Messages
// ==========================================================================

/**
 * @brief Display a given bitmap image on the HDMI output.
 *
 * This is used to display on-screen user messages, such as access status.
 *
 * The bitmap image shall be displayed as an overlay on the current
 * video if any. If the bitmap size is smaller than current resolution,
 * the bitmap image shall be centered.
 *
 * @param[in] filename
 *  Filename (with path) containing the bitmap image to overlay
 */
void nvSprShowOverlay(
  const char* filename
);

/**
 * @brief Display a given bitmap image at given position on HDMI output.
 *
 * This is used to display on-screen user messages, such as fingerprints.
 *
 * The bitmap image shall be displayed as an overlay on the current
 * video if any, at the given position. The position is given by a
 * point in the bitmap (px, py) expressed as percentage of display
 * resolution, and some alignment indication (ax, ay) to specify the
 * location of the given point in the bitmap (e.g. top-left, top-right,
 * ...)
 *
 * @param[in] px
 *  Horizontal position as percentage of resolution width (0 to 100)
 *
 * @param[in] py
 *  Vertical position as percentage of resolution height (0 to 100)
 *
 * @param[in] ax
 *  Horizontal location of px in bitmap (0=left, 1=center, 2=right)
 *
 * @param[in] ay
 *  Vertical location of py in bitmap (0=top, 1=middle, 2=bottom)
 *
 * @param[in] filename
 *  Filename (with path) containing the bitmap image to overlay
 */
void nvSprShowOverlayAt(
  int         px,
  int         py,
  int         ax,
  int         ay,
  const char* filename
);

/**
 * @brief Remove any displayed overlay bitmap image on the HDMI output.
 *
 * This is used to hide on-screen user messages, such as access status.
 *
 * In case no image is currently displayed (e.g. no @ref nvSprShowOverlay
 * was called priorly), the call shall simply be ignored.
 */
void nvSprHideOverlay();

// ==========================================================================
// Configuration & Preferences
// ==========================================================================

/**
 * @brief Force the usage of a specific legacy SEC driver version.
 *
 * By default, selection of the appropriate SEC driver version when
 * opening sessions is left up to the underlying implementation.
 *
 * However, for some particular test scenarios, we want to be able to
 * force usage of a specific version (if available). The chosen version
 * shall then be used when opening new sessions until set otherwise by a
 * new call to that same function.
 * 
 * If the selected version is not available, the call can be ignored and
 * default SEC driver used.
 * 
 * @param[in] version
 *  SEC version, as Auto (default), SECv5 or SECv6
 */
void nvSprUseSecVersion(
  TNvSprSecVersion version
);

/**
 * @brief Specify the preferred HDMI display mode for a given process.
 *
 * By default, when multiple video sessions are opened concurrently,
 * the first session should be opened in HDMI full screen mode, and
 * additional session(s) in picture-in-picture mode (PiP).
 *
 * However, in some multi-process use cases, we want to be able to have
 * one process always using full screen mode and another process always
 * using picture-in-picture mode, to be able to differentiate them.
 *
 * @param[in] mode
 *  Display mode, as Auto (default), PiP only or Full Screen only
 */
void nvSprSetDisplayMode(
  TNvSprDisplayMode mode
);

/**
 * @brief Specify HDCP session (1.4 or 2.2) for HDMI output
 *
 * The platform specific implementation shall check whether the required HDCP
 * session is already established. If not, establish the required HDCP session or
 * if not available, return an error value.
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] hdcp
 *  HDCP session: 
 *              0 => No HDCP
 *              1 => HDCP 1.x
 *              2 => HDCP 2.2
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprSetHdcp
(
  TTransportSessionId   tsid,
  TUnsignedInt8         hdcp
);

/**
 * @brief Specify video capping resolution (video canvas size)
 *
 * The platform specific implementation shall apply the video capping
 * resolution if the required HDCP session (1.4 or 2.2) can't be established.
 *
 * @param[in] tsid
 *  Identifier of the transport session
 * @param[in] capping
 *  video capping resolution: 
 *              0   => No output
 *              1   => QCIF (176x144 pixels or an equivalent number of pixels)
 *              2   => CIF  (352x288 pixels or an equivalent number of pixels)
 *              3   => SD   (720x576 pixels or an equivalent number of pixels)
 *              4   => 720  p/i
 *              5   => 1080 p/i
 *              0xF => No restrictions
 * @retval TNvSprStatus
 *  Result status of the function
 */
TNvSprStatus nvSprSetCappingResolution
(
  TTransportSessionId   tsid,
  TUnsignedInt8         capping
);

#ifdef __cplusplus
}
#endif

#endif
