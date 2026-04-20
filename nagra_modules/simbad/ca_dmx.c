/*
 * @file ca_dmx.c
 * @brief Nagravision Stream Processing APIs implemetation on Montage Symphony4 platform
 *
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 */
#include "ca_dmx.h"

/**
 *  @brief
 *    This function opens a demux filter.
 *
 *  @param[out] pxFilterId
 *                Identifier assigned to the demux filter opened.
 *
 *  @param[in]  xTransportSessionId
 *                Transport session identifier associated to the stream to be
 *                filtered.
 *
 *  @param[in]  xFilterDepth
 *                Maximum size in bytes of the matching section filter pattern, 
 *                starting from the table ID. If the filter requires more than 
 *                the table ID, \c xFilterDepth will include the two bytes of 
 *                \c section_length.
 *
 *  @param[in]  pxPrivateData
 *                Private data used by the CA software for internal processing. 
 *                This pointer shall be passed back as parameter of the two 
 *                callback functions \c TDmxFilterQueryBufferCallback and \c 
 *                TDmxFilterReceivedSectionCallback
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_NO_MORE_RESOURCES
 *             No more demux filter resources are available
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
*/
TDmxStatus dmxFilterOpen
(
        TDmxFilterId*	         pxFilterId,
        TTransportSessionId	    xTransportSessionId,
        TSize	                  xFilterDepth,
  const void*	                 pxPrivateData
)
{
	//TODO:
	return DMX_NO_ERROR;
}


/**
 *  @brief
 *    This function closes the given demux filter
 *
 *  @param[in]  xFilterId
 *                Identifier of the demux filter to be closed
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             This filter resource was not opened
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
*/
TDmxStatus dmxFilterClose
(
  TDmxFilterId   xFilterId
)
{
	//TODO:
	return DMX_NO_ERROR;
}


/**
 *  @brief
 *    This function defines the filtering patterns of the demux filter.
 *
 *    Let's illustrate these parameters by an example. If we would like to catch 
 *    a section with the first byte set to 0x42 and the third one different than 
 *    0xC4, then the filtering patterns shall be defined as follows: 
 * 
 *      \c pxValue=0x4200C4          \n
 *      \c pxEqualMask=0xFF0000      \n
 *      \c pxNotEqualMask=0x0000FF   \n
 *
 *  @param[in]  xFilterId
 *                Identifier of the demux filter
 *
 *  @param[in]  pxValue       
 *                Section filter value pattern. This pattern is the array of 
 *                bytes that are compared to the incoming section. This  
 *                pattern is used in combination with the "equal" and 
 *                "not-equal" masks. These two masks indicates precisely which 
 *                bit of the value pattern are taken into account ( i.e. all 
 *                bits not covered by a mask are ignored).
 *
 *  @param[in]  pxEqualMask
 *                Bitmap indicating which bits of the value pattern must match 
 *                the incoming section.
 *                
 *  @param[in]  pxNotEqualMask
 *                Bitmap indicating which bits of the value pattern must not match 
 *                the incoming section.
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR_FILTER_RUNNING
 *             The filter is still running
 *
 *  @retval  DMX_ERROR_BAD_PARAMETER
 *             The given parameters are inconsistent
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# The size of the patterns is equal to the filter depth given in
 *       \c dmxFilterOpen().
 *    .
 *    -# This function shall only be called when the filter is stopped.
 *    .
 *    -# The \c pxEqualMask and \c pxNotEqualMask masks must not overlap
 *       (the bit-and operation of these two masks shall be equal to 0).
 *       \c DMX_ERROR_BAD_PARAMETER shall be returned when masks overlap.
*/
TDmxStatus dmxFilterSetPatterns
(
        TDmxFilterId   xFilterId,
  const TUnsignedInt8* pxValue,
  const TUnsignedInt8* pxEqualMask,
  const TUnsignedInt8* pxNotEqualMask
)
{
	//TODO:
	return DMX_NO_ERROR;
}


/**
 *  @brief
 *    This function set the PID of packet to be filtered
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @param[in]  xPid
 *                PID of the packet to be filtered
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR_BAD_PID
 *             The PID is out of range
 *
 *  @retval  DMX_ERROR_FILTER_RUNNING
 *             The filter is still running
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# This function can called only if the filter is stopped.
*/
TDmxStatus dmxFilterSetPid
(
  TDmxFilterId    xFilterId,
  TPid            xPid
)
{
	//TODO:
	return DMX_NO_ERROR;
}



/**
 *  @brief
 *    This function starts filtering on a specified filter.
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @param[in]  xTimeout
 *                Timeout on the filter in [ms]. If 0, the timeout is considered 
 *                to be infinite. If the timeout expires and no section has been 
 *                catched, the notification callbacks must be called anyway with 
 *                \c pxSection=NULL and \c xSectionSize=0.
 *
 *  @param[in]  xLoopMode
 *                This mode defines the behavior of the filter right after 
 *                catching a matching section:
 *                - \c DMX_LOOP_CONTINUOUS: the filter remains active as long as 
 *                  not explicitely stopped by \c calling \c dmxFilterStop().
 *                - \c DMX_LOOP_ONE_SHOT: the filter automatically stops after 
 *                  catching a section.
 *                - \c DMX_LOOP_TOGGLE: this mode is used to acquire sections 
 *                  broadcast alternately on two different tables. The filter 
 *                  remains active as long as not explicitely stopped by 
 *                  calling \c dmxFilterStop().
 *
 *  @param[in]  xCrcMode
 *                Indicate whether the CRC has to be checked or not:
 *                - \c DMX_CRC_CHECK
 *                - \c DMX_CRC_IGNORE
 *
 *  @param[in]  xQueryBufferCallback
 *                Callback used by the DMX driver to get a buffer in order to 
 *                store a matching section.
 *
 *  @param[in]  xReceivedSectionCallback
 *                Callback used by the DMX driver to notify the CA software of 
 *                that a section matching the filter pattern has been acquired.
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR_FILTER_RUNNING
 *             The filter is already running
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# Both callbacks are mandatory
 *    .
 *    -# After a one shot filtering, the filter is simply stopped and can be
 *       restarted at any time with \c dmxFilterStart().
 *    .
 *    -# This function can only be called when the filter is stopped.
*/
TDmxStatus dmxFilterStart
(
  TDmxFilterId                        xFilterId,
  TOsTime                             xTimeout,
  TDmxLoopMode                        xLoopMode,
  TDmxCrcMode                         xCrcMode,
  TDmxFilterQueryBufferCallback       xQueryBufferCallback,
  TDmxFilterReceivedSectionCallback   xReceivedSectionCallback
)
{
	//TODO:
	return DMX_NO_ERROR;
}


/**
 *  @brief
 *    This function stops the filtering on a specified filter and freezes the
 *    buffer in its current state.
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# This function does flush the reception buffer.
*/
TDmxStatus dmxFilterStop
(
  TDmxFilterId   xFilterId
)
{
	//TODO:
	return DMX_NO_ERROR;
}


/**
 *  @brief
 *    This function resets the filtering on a specified filter by flushing the
 *    buffer and resetting the timeout.
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
*/
TDmxStatus dmxFilterReset
(
  TDmxFilterId   xFilterId
)
{
	//TODO:
	return DMX_NO_ERROR;
}


