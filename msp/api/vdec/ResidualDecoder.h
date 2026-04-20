/* Copyright (c) V-Nova International Limited 2022-2024. All rights reserved. */
#ifndef VN_LCEVC_DVP_H_
#define VN_LCEVC_DVP_H_

#define VN_LCEVC_DVP_Concat_Do(a, version) a##version
#define VN_LCEVC_DVP_Concat(a, version) VN_LCEVC_DVP_Concat_Do(a, version)
#define VN_DVP_API_Version 2
#define LCEVC_CreateDecoder VN_LCEVC_DVP_Concat(LCEVC_CreateDecoder_, VN_DVP_API_Version)

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \brief Opaque handle for the LCEVC Decoder instance.
 */
typedef struct LCEVC_Decoder* LCEVC_DecoderHandle;

/*!
 * \brief This enum represents the possible API return codes.
 */
typedef enum LCEVC_ReturnCode
{
    LCEVC_Success = 0, /**< The API call completed successfully */
    LCEVC_Again = 1, /**< Not an error, the requested operation can't be performed, try again later. Only returned by LCEVC_Send... and LCEVC_Receive... functions */
    LCEVC_Drain = 2, /**< EOS has been signalled but there is still data left in the LCEVC Queue, which needs to be retrieved with LCEVC_ReceiveNextCommandBuffers */
    LCEVC_Empty = 3, /**< After EOS has been signalled and there is no more data available, Only returned by LCEVC_ReceiveNextCommandBuffers */
    LCEVC_Error = -1,        /**< A generic catch all error */
    LCEVC_InvalidParam = -2, /**< The user supplied an invalid parameter to the function call */
    LCEVC_NotFound = -3, /**< The query call failed to find the item by name, but didn't not generate an error. */
    LCEVC_NotSupported = -4, /**< The functionality requested is not supported on the running system. */
    LCEVC_Flushed = -5, /**< The requested operation failed because it was flushed via LCEVC_Synchronize. */
    LCEVC_Timeout = -6,     /**< The requested operation failed because it timed out. */
    LCEVC_NoLcevcData = -7, /**< Indicates that there is no LCEVC data available. Only returned by LCEVC_StartGenerateEnhancementLayer and LCEVC_FinishEnhancementLayer indicating that the output buffer contains no residual information */
    LCEVC_Busy = -8,        /**< The function is busy with another job */
    LCEVC_InvalidDecoder = -9 /**< The decoder is in an invalid state and is unusable */
} LCEVC_ReturnCode;

/*!
 * \brief This enum represents the output packing format of an LCEVC residual plane buffer
 *
 * NOTE: New values will be added at the end.
 *
 * NOTE: Not all packing formats are currently supported please refer to the release notes for
 * information about currently supported formats.
 */
typedef enum LCEVC_ResidualPacking
{
    LCEVC_ResidualY8bitS = 0,       /**< Residual plane is signed 8bit, i.e. one pixel per byte. */
    LCEVC_ResidualY10bitPacked = 1, /**< Residual plane is Y packed 10 bit, 4 pixels in 5 bytes. */
    LCEVC_ResidualY10bitUnpacked = 2, /**< Residual plane is Y unpacked, 10 bits in 2 bytes - i.e. YUV */
    LCEVC_ResidualUYVY8bitS = 3, /**< Residual plane is UYVY signed 8bit, 2 pixels in 4 bytes. */
    LCEVC_ResidualYVYU8bitS = 4, /**< Residual plane is YVYU signed 8bit, 2 pixels in 4 bytes. */
    LCEVC_ResidualUYVY10bitPacked = 5, /**< Residual plane is UYVY packed 10 bit, 2 pixels in 5 bytes. */
    LCEVC_ResidualUYVY10bitUnpacked = 6, /**< Residual plane is UYVY 10 bit, 2 pixels in 8 bytes. */
    LCEVC_ResidualVYUY10bitPacked = 7, /**< Residual plane is VYUY packed 10 bit, 2 pixels in 5 bytes. */
    LCEVC_ResidualVYUY10bitUnpacked = 8, /**< Residual plane is VYUY 10 bit, 2 pixels in 8 bytes. */
    LCEVC_ResidualYVYU10bitPacked = 9, /**< Residual plane is YVYU packed 10 bit, 2 pixels in 5 bytes. */
    LCEVC_ResidualYVYU10bitUnpacked = 10, /**< Residual plane is YVYU 10 bit, 2 pixels in 8 bytes. */
    LCEVC_ResidualYUYV10bitPacked = 11, /**< Residual plane is YUYV packed 10 bit, 2 pixels in 5 bytes. */
    LCEVC_ResidualYUYV10bitUnpacked = 12, /**< Residual plane is YUYV 10 bit, 2 pixels in 8 bytes. */
    LCEVC_ResidualYUYV16bit = 13, /**< Residual plane is YUYV 16 bit, 2 pixels in 8 bytes. */
    LCEVC_ResidualY8bitU = 14,    /**< Residual plane is unsigned 8bit, i.e. one pixel per byte. */
    LCEVC_ResidualUYVY8bitU = 15, /**< Residual plane is UYVY unsigned 8 bit, 2 pixels in 4 bytes. */
    LCEVC_ResidualYVYU8bitU = 16, /**< Residual plane is YVYU unsigned 8 bit, 2 pixels in 4 bytes. */
} LCEVC_ResidualPacking;

/*!
 * \brief This enum represents the output packing mode of an LCEVC residual plane buffer.
 */
typedef enum LCEVC_ResidualPackingMode
{
    LCEVC_PackingModeLinear = 0,       /**< Residual plane is linearly packed. */
    LCEVC_PackingModeLittleEndian = 1, /**< Residual plane is LE packed. */
} LCEVC_ResidualPackingMode;

/*!
 * \brief Information about a residual plane buffer - the buffer that the LCEVC enhancement plane will be written to.
 */
typedef struct LCEVC_ResidualPlaneBuffer
{
    void* data;          /**< Pointer to the actual buffer. */
    uint64_t dataHandle; /**< External memory handle, for use with the DVP Memory API. If non-zero, will be used in preference to data */
    size_t size;         /**< Size of the buffer, in bytes */
    uint32_t stride; /**< Stride of the plane, in bytes. How many bytes for a single line of pixels. */
} LCEVC_ResidualPlaneBuffer;

/*!
 * \brief Struct containing information about the format of a residual plane */
typedef struct LCEVC_ResidualPlaneInfo
{
    uint32_t width;                /**< Width of the plane in pixels. */
    uint32_t height;               /**< Height of the plane in pixels. */
    uint32_t stride;               /**< Line stride of the plane in pixels */
    LCEVC_ResidualPacking packing; /**< How the plane is packed. */
    size_t size;                   /**< size of the plane, in bytes */
    uint32_t cropTop; /**< Vertical top offset of the crop area, aka active area, where the Image samples are to be found. NOTE: crop size might differ from sample size */
    uint32_t cropBottom; /**< Vertical bottom offset of the crop area, aka active area, where the Image samples are to be found. NOTE: crop size might differ from sample size */
    uint32_t cropLeft; /**< Horizontal left offset of the crop area, aka active area, where the Image samples are to be found. NOTE: crop size might differ from sample size */
    uint32_t cropRight; /**< Horizontal right offset of the crop area, aka active area, where the Image samples are to be found. NOTE: crop size might differ from sample size */
} LCEVC_ResidualPlaneInfo;

/*!
 * \brief Enum defining LCEVC Log levels, with increasing levels of verbosity. */
typedef enum LCEVC_LogLevel
{
    LCEVC_LogLevel_None = 0,
    LCEVC_LogLevel_Error = 1,
    LCEVC_LogLevel_Warning = 2,
    LCEVC_LogLevel_Info = 3,
    LCEVC_LogLevel_Debug = 4,
    LCEVC_LogLevel_Verbose = 5,
} LCEVC_LogLevel;

typedef void (*LCEVC_LogCallback)(void* userData, const char* msg, size_t len, LCEVC_LogLevel logLevel);

/*!
 * \brief Public facing UpsampleKernel structure.
 *
 * Struct containing the upsample kernel. two arrays of length `len` for the kernel and 180-degree kernel. */
typedef struct LCEVC_UpsampleKernel
{
    int16_t k[2][8]; /**< upsample kernels of length 'len', phase kernel and 180-degree phase kernel */
    size_t len;      /**< the used length of the upsample kernel. */
    bool needPredictedResiduals; /**< If this is false, the kernel can be used in a standard upscale, if it is true the predicted residual operation is also needed.*/
} LCEVC_UpsampleKernel;

/*!
 * \brief Public facing CommandBuffer data structure.
 *
 * Contains LCEVC state, and an opaque commandbuffer pointer. */
typedef struct LCEVC_CommandBuffer
{
    void* commandBuffers; /**< Opaque commandbuffer pointer */
    void* userData;       /**< userData pointer, populated from the associated LCEVC data */
    int64_t timestamp;    /**< timestamp of this command buffer state object. */
    int64_t produceStartTimeNs; /**< Starting time, in nano-seconds, for producing this command buffer set */
    int64_t produceEndTimeNs; /**< End time, in nano-seconds, for producing this command buffer set */
} LCEVC_CommandBuffer;

/*!
 * \brief Public facing Residual output data structure.
 *
 * Contains all the information set via a call to LCEVC_StartGenerateEnhancementLayer/LCEVC_StartSkipEnhancementLayer */
typedef struct LCEVC_ResidualExtract
{
    int64_t timestamp; /**< timestamp of this command buffer state object. */
    bool skipped; /**< indicated this finish is associated with a call to LCEVC_StartSkipEnhancementLayer */
    LCEVC_ResidualPlaneBuffer outputInformation; /**< data passed to the LCEVC_StartGenerateEnhancementLayer call, will be set to 0 for a LCEVC_StartSkipEnhancementLayer */
    void* userData; /**< userData pointer, populated from the associated LCEVC data */
} LCEVC_ResidualExtract;

/*!
 * \brief Create an instance of an LCEVC Decoder
 *
 * @param[in]   logCallback    Function pointer to a logging callback function
 * @param[in]   logLevel       The desired log level
 * @param[in]   logUserData    Some userData associated with logging
 *
 * @param[out]  decoder        Pointer to be populated with the LCEVC decoder instance created.
 * @return an LCEVC status code. Pointer only valid if LCEVC_Success
 *
 * NOTE: This function `LCEVC_CreateDecoder` is actually a define (see LCEVC_DVP_API_Version.h) which
 *       includes an API binary compatiblity version number at the end, e.g LCEVC_CreateDecoder_1
 *       This is done to ensure that the DVP shared library is binary compatible with the software using it.
 *       The version number (and new function name) will ONLY change when any structures or function signatures change.
 *       Only software compiled with the new DVP release will be compatible with the new API version of software.
 */
LCEVC_ReturnCode LCEVC_CreateDecoder(LCEVC_LogCallback logCallback, LCEVC_LogLevel logLevel,
                                     void* logUserData, LCEVC_DecoderHandle* decoder);

/*!
 * \brief Destroy an instance of a Decoder.
 *
 * @param[in]    decoder            instance to be destroyed
 */
void LCEVC_DestroyDecoder(LCEVC_DecoderHandle decoder);

/*!
 * @defgroup LCEVC_ConfigureDecoder LCEVC_ConfigureDecoder[type]
 *
 * Set a configuration variable of type [type]. [type] may be: Bool, Int, Float, String or Pointer.
 *
 */
LCEVC_ReturnCode LCEVC_ConfigureDecoderBool(LCEVC_DecoderHandle decoder, const char* name, bool val);
LCEVC_ReturnCode LCEVC_ConfigureDecoderInt(LCEVC_DecoderHandle decoder, const char* name, int32_t val);
LCEVC_ReturnCode LCEVC_ConfigureDecoderUint(LCEVC_DecoderHandle decoder, const char* name, uint32_t val);
LCEVC_ReturnCode LCEVC_ConfigureDecoderInt64(LCEVC_DecoderHandle decoder, const char* name, int64_t val);
LCEVC_ReturnCode LCEVC_ConfigureDecoderUint64(LCEVC_DecoderHandle decoder, const char* name, uint64_t val);
LCEVC_ReturnCode LCEVC_ConfigureDecoderFloat(LCEVC_DecoderHandle decoder, const char* name, float val);
LCEVC_ReturnCode LCEVC_ConfigureDecoderString(LCEVC_DecoderHandle decoder, const char* name,
                                              const char* val);
LCEVC_ReturnCode LCEVC_ConfigureDecoderPointer(LCEVC_DecoderHandle decoder, const char* name,
                                               const void* val);

/*!
 * @defgroup LCEVC_GetDecoderProperty LCEVC_GetDecoderProperty[type]
 *
 * Get a decoder property of type [type]. [type] may be: Bool, Int or String.
 *
 */
LCEVC_ReturnCode LCEVC_GetDecoderPropertyBool(LCEVC_DecoderHandle decoder, const char* name, bool* val);
LCEVC_ReturnCode LCEVC_GetDecoderPropertyInt(LCEVC_DecoderHandle decoder, const char* name, int32_t* val);
LCEVC_ReturnCode LCEVC_GetDecoderPropertyString(LCEVC_DecoderHandle decoder, const char* name,
                                                const char** val);

/*!
 * \brief Initialize a configured Decoder. Using the configuration set by using the LCEVC_ConfigureDecoder functions
 *
 * @param[in]   decoder             instance returned by CreateDecoder
 *
 * @return an LCEVC Status code, LCEVC_Success if successfully initialized.
 */
LCEVC_ReturnCode LCEVC_InitializeDecoder(LCEVC_DecoderHandle decoder);

/*!
 *
 * \brief set the desired log level.
 *
 * @param[in] decoder decoder instance
 * @param[in] logLevel desired log level. All logs of this level and below will be sent via the callback.
 */
void LCEVC_SetLogLevel(LCEVC_DecoderHandle decoder, LCEVC_LogLevel logLevel);

/*!
 * \brief Get the upsample kernel for the given timestamp.
 *        It is valid to call this function with the timestamp obtained from the LCEVC_ReceiveNextCommandBuffers call
 *        If in `event` mode see `LCEVC_DVPCallback` below for more details when it's valid to call this function
 *
 * @param[in]   decoder   decoder instance
 * @param[in]   timestamp  the timestamp requested
 *
 * @param[out]  kernel  upsample kernel
 *
 * @return LCEVC_Success if the kernel is found and populated.
 *         LCEVC_NoLcevcData if there has been no valid LCEVC data
 *         LCEVC_InvalidParam if we do not have a kernel for that timestamp.
 */
LCEVC_ReturnCode LCEVC_GetUpsampleKernel(LCEVC_DecoderHandle decoder, int64_t timestamp,
                                         LCEVC_UpsampleKernel* kernel);

/*!
 * \brief Get the residual plane info for the given timestamp.
 *        It is valid to call this function with the timestamp obtained from the LCEVC_ReceiveNextCommandBuffers call
 *        If in `event` mode see `LCEVC_DVPCallback` below for more details when it's valid to call this function
 *
 * @param[in]   decoder   decoder instance
 * @param[in]   timestamp  the timestamp requested
 *
 * @param[out]  residualPlaneInfo   Pointer to an LCEVC_ResidualPlaneInfo struct, to be populated.
 *
 * @return LCEVC_Success if the output info is found and populated.
 *         LCEVC_NoLcevcData if there has been no valid LCEVC data
 *         LCEVC_InvalidParam if we do not have stream info for that timestamp.
 */
LCEVC_ReturnCode LCEVC_GetResidualPlaneInfo(LCEVC_DecoderHandle decoder, int64_t timestamp,
                                            LCEVC_ResidualPlaneInfo* residualPlaneInfo);

/*!
 * \brief Set the reorder count. This must match, or exceed, the max_num_reorder_pics value in the video SPS
 *
 * @param[in]   decoder         LCEVC decoder instance
 * @param[in]   reorderCount    the new reorder count size.
 *
 * @return LCEVC_Success
 */
LCEVC_ReturnCode LCEVC_ConfigureReorderCount(LCEVC_DecoderHandle decoder, uint8_t reorderCount);

/*!
 * \brief Receive a buffer to write compressed LCEVC data into.
 *
 * The buffer returned by this function should then be passed to a subsequent call to LCEVC_SendDecoderEnhancementData.
 * The buffer can be released without using it by calling LCEVC_ReleaseLCEVCInputBuffer. Calling EITHER LCEVC_SendDecoderEnhancementData or LCEVC_ReleaseLCEVCInputBuffer
 * transfers ownership of the buffer to the DVP.
 *
 * @param[in]     decoder           LCEVC Decoder instance
 * @param[in]     size              Size of the requested buffer
 *
 * @param[out]    requestedBuffer  Double pointer to a buffer, populated by the call.
 * @return LCEVC_Success on a successful allocation. Buffer should be considered invalid on any other return code.
 */
LCEVC_ReturnCode LCEVC_RequestLCEVCInputBuffer(LCEVC_DecoderHandle decoder, size_t size,
                                               uint8_t** requestedBuffer);

/*!
 * \brief Release a buffer allocated for compressed LCEVC data WITHOUT using it.
 *
 * This API should be called when releasing an unused buffer allocated by LCEVC_RequestLCEVCInputBuffer.
 * If the buffer has been accepted by a call to LCEVC_SendDecoderEnhancementData, this API should not be used on that buffer, as
 * ownership of it has transferred to the DVP.
 *
 * @param[in]   decoder             LCEVC Decoder instance
 * @param[in]   inputBuffer         Buffer to be released.
 * @return LCEVC_Success if buffer was released. LCEVC_InvalidParam if buffer was not allocated by the LCEVC decoder.
 */
LCEVC_ReturnCode LCEVC_ReleaseLCEVCInputBuffer(LCEVC_DecoderHandle decoder, uint8_t* inputBuffer);

/*!
 * \brief Send enhancement data to the LCEVC Decoder.
 *
 * Feed a buffer of pre parsed LCEVC payload data for the Access Unit identified by timestamp. If
 * the buffer was allocated by LCEVC_RequestLCEVCInputBuffer, a return value of LCEVC_Success
 * indicates that ownership of this buffer has transferred to the DVP. On an unsuccessful return, the
 * client is responsible for releasing the buffer with LCEVC_ReleaseLCEVCInputBuffer.
 *
 * @param[in]     decoder           LCEVC Decoder instance
 * @param[in]     timestamp         Timestamp (timestamp) for the passed LCEVC data
 * @param[in]     data              pointer to the NAL units buffer
 * @param[in]     size              size of the NAL units buffer
 * @param[in]     userData          Optional user data pointer, will be carried by the LCEVC_CommandBuffer from LCEVC_ReceiveNextCommandBuffers()
 * @return
 *
 * Returns LCEVC_Again if the decoder cannot consume the enhancement data in it's current state (and
 * does not change decoder state) - Typically, The output side will need consuming to release
 * resources.
 */
LCEVC_ReturnCode LCEVC_SendDecoderEnhancementData(LCEVC_DecoderHandle decoder, int64_t timestamp,
                                                  const uint8_t* data, size_t size, void* userData);

/*!
 * \brief Receive command buffers for the next decoded LCEVC enhancement.
 *
 * Receive a set of LCEVC command buffers, which contain instructions to produce residuals for an LCEVC enhanced frame.
 *
 * @param[in]    decoder            LCEVC decoder instance
 *
 * @param[out]    commandBuffers    LCVECCommandBufferState pointer, which will be populated with commandbuffer metadata.
 * @return
 *
 * Returns:
 * LCEVC_Success when the commandBuffers contain valid information
 * LCEVC_Again when no data is available
 * LCEVC_Drain when in EOS mode and no data is available
 * LCEVC_Empty when in EOS mode only returned once signaling the end of EOS.
 * any other LCEVC LCEVC error return code, indicating there is a problem with the decoder.
 */
LCEVC_ReturnCode LCEVC_ReceiveNextCommandBuffers(LCEVC_DecoderHandle decoder,
                                                 LCEVC_CommandBuffer* commandBuffers);

/*!
 * \brief Extract an LCEVC Residual plane from a set of LCEVC command buffers.
 *
 * Begins a multi-threaded extraction of an LCEVC Residual frame from a supplied set of command
 * buffers. Starting is a non-blocking multi-threaded operation, and should always be matched with a
 * corresponding call to LCEVC_FinishEnhancementLayer, to finish the processing. NOTE: each
 * LCEVC_StartGenerateEnhancementLayer MUST be completed with a LCEVC_FinishEnhancementLayer
 *
 * @param[in]    decoder            LCEVC decoder instance
 * @param[in]    commandBuffers     Command buffers to be processed
 *
 * @param[out]    outputPlane       Pointer to a residual plane buffer structure.
 * @return
 *
 * Returns LCEVC_Success - If the call was successful.
 *         LCEVC_Busy    - If there is already an extract running, the caller must first call
 * LCEVC_FinishEnhancementLayer to complete the outstanding request. else  some other error.
 */
LCEVC_ReturnCode LCEVC_StartGenerateEnhancementLayer(LCEVC_DecoderHandle decoder,
                                                     LCEVC_CommandBuffer commandBuffers,
                                                     LCEVC_ResidualPlaneBuffer* outputPlane);

/*!
 * \brief Skip extracting LCEVC Residual plane from a set of LCEVC command buffers, whilst still
 * maintaining the temporal residual cache.
 *
 * This method acts similarly to LCEVC_StartGenerateEnhancementLayer, but will not write out the internal
 * temporal residual cache to a plane. This is required to maintain the temporal cache state, but
 * allows the caller to skip receiving an output plane for this frame. NOTE: each
 * LCEVC_StartSkipEnhancementLayer MUST be completed with a LCEVC_FinishEnhancementLayer
 *
 * @param[in]    decoder            LCEVC decoder instance
 * @param[in]    commandBuffers     CommandBuffer state structure
 *
 * @return
 *
 * Returns LCEVC_Success - If the call was successful.
 *         LCEVC_Busy    - If there is already an extract running, the caller must first call
 * LCEVC_FinishEnhancementLayer to complete the outstanding request. else  some other error.
 */
LCEVC_ReturnCode LCEVC_StartSkipEnhancementLayer(LCEVC_DecoderHandle decoder,
                                                 LCEVC_CommandBuffer commandBuffers);

/*!
 * \brief Finish a call to LCEVC_StartGenerateEnhancementLayer/LCEVC_StartSkipEnhancementLayer ,
 *        Triggering a copy of the enhancement plane to the output buffer.
 *        The outputState struct will be populated with the information provided in the outputPlane value given to the LCEVC_StartGenerateEnhancementLayer call.
 *        The decoder takes no ownership of the contents in LCEVC_ResidualPlaneBuffer.
 *
 * This is a blocking method that will block until the the result of the start is completed
 * NOTE: each LCEVC_FinishEnhancementLayer MUST be called after a LCEVC_StartGenerateEnhancementLayer/LCEVC_StartSkipEnhancementLayer
 *
 * @param[in]   decoder         LCEVC decoder instance
 * @param[out]  outputState     struct holding the result of the last LCEVC_StartGenerateEnhancementLayer/LCEVC_StartSkipEnhancementLayer call
 *
 * @return
 *
 * Returns LCEVC_Success when the task has finished. Blocking method.
 */
LCEVC_ReturnCode LCEVC_FinishEnhancementLayer(LCEVC_DecoderHandle decoder,
                                              LCEVC_ResidualExtract* outputState);

/*!
 * \brief Flush the LCEVC Decoder.
 *
 * This is a blocking call that will flush the LCEVC decoder. If there is currently a frame in progress in the decoder or
 * the residual frame generator, it will be finished before flushing. Once flushed, the LCEVC decoder will contain no
 * previous LCEVC data or decoded frames, and expects to receive new frames.
 *
 * @param[in]   decoder     LCEVC decoder instance
 *
 * @return
 *
 * LCEVC_Success when flush is complete.
 *
 * */
LCEVC_ReturnCode LCEVC_Flush(LCEVC_DecoderHandle decoder);

/*!
 * \brief Signal the end of a stream.
 *
 * Used for triggering draining of the LCEVC Reorder Queue.
 * NOTE: Once the EOS has been started it is expected that no more calls to LCEVC_SendDecoderEnhancementData
 *       will be made. When the software is in EOS mode it can only be completed by processing all of the data
 *       or calling LCEVC_Flush. Calling LCEVC_SendDecoderEnhancementData after EOS is started BUT before it is completed may result in indefined behaviour.
 *
 * @param[in]   decoder     LCEVC decoder instance
 * @param[in]   timestamp   Timestamp for the end of stream frame if known
 *
 */
void LCEVC_SignalEOS(LCEVC_DecoderHandle decoder, int64_t timestamp);

/**
 * @brief Function pointer definition for a DVP release userData callback, to be implemented by a
 * DVP client.
 * NOTE: This must be implemented if the userData provided to the LCEVC_SendDecoderEnhancementData call requires some sort of resource management.
 *       This maybe the only way that the userData will be returned to the client on a flush or shutdown.
 *
 * @param decoder The decoder object making the callback
 * @param timestamp The timestamp this callback is for
 * @param userData The userData provided on the LCEVC_SendDecoderEnhancementData call
 * @param clientContext a client context for this callback
 *
 */
typedef void (*LCEVC_DVPReleaseUserdataCallback)(LCEVC_DecoderHandle decoder, int64_t timestamp,
                                                 void* userData, void* clientContext);

/**
 * \brief The DVP callback types that may be triggered.
 *
 * The following events may be fired upto 100ms before the command buffer is available via the
 * LCEVC_ReceiveNextCommandBuffers call There may be any number of command buffers received after
 * the event before the one (indicated by the timestamp) that triggered the change becomes
 * available. The change is only valid from the command buffer matching the timestamp onwards.
 * LCEVC_DVP_OUTPUT_FORMAT_CHANGED, LCEVC_DVP_UPSAMPLE_KERNEL_CHANGED
 */
typedef enum LCEVC_DVPCallbackType
{
    LCEVC_DVP_OUTPUT_FORMAT_CHANGED =
        0, /**< LCEVC Resolution or Format has changed. Once received it is valid to call LCEVC_GetResidualPlaneInfo. */
    LCEVC_DVP_UPSAMPLE_KERNEL_CHANGED =
        1, /**< The upsample kernel has changed. Once received it is valid to call LCEVC_GetUpsampleKernel.*/
    LCEVC_DVP_COMMAND_BUFFER_AVAILABLE =
        2, /**< Indicates that a new command buffer is available to be collected using the LCEVC_ReceiveNextCommandBuffers call */
    LCEVC_DVP_ENHANCEMENT_LAYER_COMPLETE =
        3, /**< Indicates that an enhancement layer started with LCEVC_StartGenerateEnhancementLayer/LCEVC_StartSkipEnhancementLayer may be completed with the LCEVC_FinishEnhancementLayer call */
    LCEVC_DVP_ERROR = 9999 /**< A generic, unrecoverable error code. Decoder state after receiveing this error is unpredictable. */
} LCEVC_DVPCallbackType;

/**
 * @brief Function pointer definition for a DVP callback, to be implemented by a DVP client.
 *
 * @param decoder The decoder object making the callback
 * @param callbackType The LCEVC_DVPCallbackType signalled
 * @param timestamp The timestamp this callback is for
 * @param userData a userdata pointer, passed back to the client on every callback
 * @param extraData an optional extra data pointer. The contents of this pointer are determined by the value of callbackType.
 *
 */
typedef void (*LCEVC_DVPCallback)(LCEVC_DecoderHandle decoder, LCEVC_DVPCallbackType callbackType,
                                  int64_t timestamp, void* userData, void* extraData);

/**
 * \brief The changes of state that the thread callbacks support
 */
typedef enum LCEVC_ThreadState
{
    LCEVC_DVP_THREAD_STARTING = 0, /**< Thread is starting */
    LCEVC_DVP_THREAD_STOPPING = 1, /**< Thread is stopping */
    LCEVC_DVP_THREAD_UNKNOWN = -1  /**< Thread is in unknown state */
} LCEVC_ThreadState;

/**
 * @brief Function pointer definition for a DVP thread state change callback, to be implemented by a DVP client.
 *
 * @param decoder The decoder object making the callback
 * @param state The LCEVC_ThreadState signalled
 * @param threadName The name of the thread
 * @param userData a userdata pointer, passed back to the client on every callback
 *
 * NOTE: this callback will be called on the thread that is changing state.
 *        LCEVC_DVP_THREAD_STARTING - As the thread starts before it begins any processing.
 *        LCEVC_DVP_THREAD_STOPPING - Just before the thread exits.
 *
 */
typedef void (*LCEVC_ThreadStateChangeCallback)(LCEVC_DecoderHandle decoder, LCEVC_ThreadState state,
                                                const char* threadName, void* userData);

#ifdef __cplusplus
}
#endif

#endif /* VN_LCEVC_DVP_H_ */
