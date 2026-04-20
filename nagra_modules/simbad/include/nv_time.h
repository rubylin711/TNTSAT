/**
  @file nv_time.h

  @brief
  This file defines the Nagra time interface. 

  @details

  It defines the function structure providing the required features as well as 
  the error messages to manage.

  This interface is realized by the device platform implementer.
  It provides to the Nagra client a time service.

  COPYRIGHT:
    2014 - 2016 Nagravision S.A.
*/

/*
   ==========================================================================
   IMPORTANT REMARK :
   ==========================================================================

   Comments in this file use special tags to allow automatic API 
   documentation generation in HTML format, using the GNU-General Public 
   Licensed Doxygen tool.
   For more information about Doxygen, please check www.doxygen.org

   Depending on the platform, the CHM file may not open properly if it is 
   stored on a network drive. So either the file should be moved on a local 
   drive or add the following registry entry on Windows platform (regedit):
   [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\HTMLHelp\1.x\\ItssRestrictions] "MaxAllowedZone"=dword:00000003

   ========================================================================== 
*/

/* ========================================================================== */
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @addtogroup g_time
  @brief Describe the Nagra time interface of Nagra clients.

  @details
  The Nagra <b>Time</b> interface introduces definition for enhancing internal 
  time enforcements. Please make sure to have read the documentation pages 
  for a complete description of the interface constraints and requirements.
*/

#ifndef NV_TIME_H
#define NV_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include "nv_defs.h"

/* ========================================================================== */
/*                                DEFINITIONS                                 */
/* ========================================================================== */

/**
  @addtogroup g_time
  @{
*/
/* -------------------------------------------------------------------------- */
/**
  @name Nagra time interface version
  @brief
  Define the version number of the Nagra time interface.

  @details
  This version has to be included in the time interface structure returned by 
  nvGetTimeInterface(). To do so, use the time macro ::TIMEAPI_VERSION_INT to 
  put it in the right format.

  @{
*/

/** @brief Nagra time interface version major number. */
#define TIMEAPI_VERSION_MAJOR       1
/** @brief Nagra time interface version medium number. */
#define TIMEAPI_VERSION_MEDIUM      2
/** @brief Nagra time interface version minor number. */
#define TIMEAPI_VERSION_MINOR       7

/**
  @brief Nagra time interface version formatted as a single integer.
  @hideinitializer
*/
#define TIMEAPI_VERSION_INT       \
    NV_INTERFACE_VERSION_INT(TIMEAPI_VERSION_MAJOR, TIMEAPI_VERSION_MEDIUM, TIMEAPI_VERSION_MINOR)
/**
  @brief Nagra time interface version formatted as a string.
  @hideinitializer
*/
#define TIMEAPI_VERSION_STRING    \
    NV_INTERFACE_VERSION_STRING(TIMEAPI_, TIMEAPI_VERSION_MAJOR, TIMEAPI_VERSION_MEDIUM, TIMEAPI_VERSION_MINOR)

/**@}*/
/* -------------------------------------------------------------------------- */
/**@}*/

/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @ingroup g_time

  @brief
  Alarm listener.

  @details

  This type defines the prototype of the function to be provided as 
  @a onAlarm argument to INvTime::alarmStart().
  Check its detailed description for complete description of the alarm 
  management.

  @param[in] xAlarm
  Handle of the alarm that just elapses.

  @param[in] xContext
  Context handle that was provided during INvTime::alarmStart().

  @see INvTime::AlarmStart().
*/

typedef void (*INvAlarmListener)
(
  TNvHandle xAlarm,
  TNvHandle xContext
);

/**
  @ingroup g_time
  @brief
  This structure defines the Nagra time interface content.

  @details
  It is a collection of function pointers composing the interface.
*/

typedef struct {

  uint32_t version;
  /**<
    @brief
    Nagra time interface version number.
    @details
    Assign it to the ::TIMEAPI_VERSION_INT result.
  */

  bool_t (*clockGetPosixTime)
  (
    uint32_t* value
  );
  /**<
    @brief
    Return a POSIX time measure.
   
    @pre
    The source of time must have been established prior to the Nagra client 
    loading and execution: Its origin must have been correctly set to provide 
    a correct consistent POSIX time measure e.g. synchronized with an 
    authenticated time server, etc ...

    @post
    The time value is filled with the current POSIX time.

    @details

    This function returns a POSIX time measure.
    The measure of POSIX time is the number of second elapsed since the Epoch 
    (00:00:00 UTC on Jan. 01, 1970).

    @param[out] value
    Reference to a time value to be filled with the POSIX time value.

    @retval TRUE
    The time source is currently correctly managed.
    The time value provided back is consistent.

    @retval FALSE
    The time source is currently experiencing management issues e.g. 
    synchronization... The time value provided back may be inconsistent.

    @see @ref p_time_posix "POSIX time measure".
  */

  void (*clockGetMonotonicTime)
  (
    uint32_t* value
  );
  /**<
    @brief 
    Return a system time measure.

    @pre
    The source of time must have been established prior to the Nagra client 
    loading and execution: Its implementation-defined origin although 
    arbitrary must have been set.

    @post
    The time value is filled with the current system time.

    @details
    This function returns a time measure which guarantees accurate time 
    difference i.e. accurate delay computation. 

    The measure of system time is a number of second elapsed since a system 
    arbitrary origin.

    @param[out] value
     Reference to a time value to be filled with the system time value.

    @see @ref p_time_monotonic "Monotonic time measure".
  */

  bool_t (*alarmStart)
  (
    TNvHandle*       pxAlarm,
    uint32_t          xDelay,
    INvAlarmListener onAlarm,
    TNvHandle        xContext
  );
  /**<
    @brief
    Program and start an alarm with the provided delay value (milliseconds).

    @pre
    An alarm resource must be free for use.

    @post
    An alarm resource has been programmed and started with the provided context.

    @details

    An alarm resource is programmed to elapse after @a xDelay value (milliseconds).
    The programmed alarm resource is immediately started.
    A handle to the allocated alarm resource is provided back in @a pxAlarm.

    @a onAlarm refers the function that must be called when the alarm elapses
    providing the original alarm handle and context handle.
    Elapsed alarm resources must be released before the @a onAlarm call.

    @note
    An alarm resource is intended to elapse only once triggering a single call 
    to @c onAlarm callback function.

    @warning
    Implementation of alarm feature within trusted execution environment must 
    preempt rich environment execution when the alarm -- programmed within 
    trusted execution environment -- elapses.
    Moreover Nagra trusted client implementations -- e.g. running in trusted 
    execution environment -- do not manage threading or protection against 
    concurrent accesses.
    Therefore a particular attention must be paid when implementing this 
    specification with such environment:
    - The implementation must guarantee that the Nagra trusted client is not 
      called concurrently by other platform services e.g. against 
      @ref g_tfl_tam "Nagra trusted client interface" calls.
    - The implementation must allows others implemented platform services to 
      be called from the @a onAlarm execution including @ref g_time itself 
      e.g. for restarting alarm resource.

    @internal
    @todo 
    Future: Move the above warning to the host platform requirements in case of
    deployment within TEE and list precise list of reentrant platform services.
    @endinternal

    @param[out] pxAlarm
    Handle of the alarm resource used.

    @param[in] xDelay
    Delay to be programmed expressed in milliseconds.

    @param[in] onAlarm
    Address of a function that must be called when the programmed delay elapsed.

    @param[in] xContext
    Context handle to be provided back with @a onAlarm() call.
    
    @retval ::TRUE
    An alarm resource has been successfully started with the provided context.

    @retval ::FALSE
    An error occurs in programming or starting an alarm resource.
  */

  void (*alarmCancel)
  (
    TNvHandle xAlarm,
    TNvHandle* pxContext
  );
  /**<
    @brief
    Cancel an alarm resource previously programmed and started.

    @pre
    The provided alarm must have been previously programmed and started.

    @post
    The provided alarm has been stopped and canceled.
    The programmed context has been deleted.

    @param[in] xAlarm
    Handle of the alarm resource to cancel.

    @param[out] pxContext
    Context handle provided during INvTime::alarmStart() call.
    This parameter may be @c NULL. In this case, it must be ignored.

    @details
    Once canceled, the alarm resource is released.
    Therefore the provided @a onAlarm when the alarm resource was programmed 
    must never be called.
  */

} INvTime;

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @ingroup g_time
  @brief
  Provide the time interface structure.

  @pre
  Interface functions must have been defined.

  @post
  The memory allocated to the Nagra time interface structure must remain 
  accessible as long as the Nagra client is running.

  @details
  This function is used by the Nagra client to retrieve the Nagra time 
  interface structure.

  This function should be called once during Nagra client initialization but 
  it cannot be definitively assumed. Therefore the address of the structure 
  and the memory allocated to it must be valid as long as the Nagra client 
  library is loaded and running.

  @return
  A constant pointer to the Nagra time interface structure.
    
  @see ::INvTime.
*/

const INvTime* nvGetTimeInterface
(
  void
);

#ifdef __cplusplus
}
#endif

#endif /* NV_TIME_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
