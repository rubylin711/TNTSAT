/*
 * This file the implementation of the Nagra time interface on the GP TEE standard.
 * It uses the time functions defined in tee_internal_api.h GP TEE internal header.
 *
 * Copyright 2015 Nagravision S.A.
 */

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#ifdef _NV_REMAP_DEFS_
# error ISO C99 definitions must be used.
#endif
#include "nv_time.h"


/* ========================================================================== */
/*                             TIME ADAPTERS                                  */
/* ========================================================================== */

/*
 * Local wrappers.
 * */
static bool_t nv_getPosixTime(uint32_t* value);
static void nv_getMonotonicTime(uint32_t* value);
static bool_t nv_alarmStart(TNvHandle *pxAlarm,
			    uint32_t xDelay,
			    INvAlarmListener onAlarm,
			    TNvHandle xContext);
static void nv_alarmCancel(TNvHandle xAlarm, TNvHandle* pxContext);

/* ========================================================================== */
/*                       NAGRA TIME INTERFACE STRUCTURE                       */
/* ========================================================================== */
const INvTime *nvGetTimeInterface(void)
{
	//printf("test REE TIME interface %s, %d \n", __FUNCTION__, __LINE__);
	static const INvTime iTime = {
		TIMEAPI_VERSION_INT,
		nv_getPosixTime,
		nv_getMonotonicTime,
		NULL, //nv_alarmStart,
		NULL, //nv_alarmCancel

	};

	return &iTime;
}

/* -------------------------------------------------------------------------- */
/*                             nv_getPosixTime                                */
/* -------------------------------------------------------------------------- */
static bool_t nv_getPosixTime(uint32_t *value)
{
	bool_t rtn = FALSE;



	return rtn;
}

/* -------------------------------------------------------------------------- */
/*                          nv_getMonotonicTime                               */
/* -------------------------------------------------------------------------- */
static void nv_getMonotonicTime(uint32_t *value)
{

}

/* -------------------------------------------------------------------------- */
/*                               nv_alarmStart                                */
/* -------------------------------------------------------------------------- */
static bool_t nv_alarmStart(TNvHandle *pxAlarm, uint32_t xDelay,
		            INvAlarmListener onAlarm, TNvHandle xContext)
{
	bool_t rtn = FALSE;

	/* Check parameters */
	if ( NULL == pxAlarm ) return FALSE;
	if ( 0 == xDelay ) return FALSE;
	if ( NULL == onAlarm ) return FALSE;

	/*
	 * GP TEE standard definition does not provide timer support.
	 * Therefore this implementation is platform-specific.
	 * Check other provided examples.
	 *
	 * IMPORTANT:
	 * GP assumes that a TA can only be invoked through the TA interface.
	 * GP assumes also that there is no concurrent access to a TA within TEE.
	 * Nagra TA follows GP assumption and is therefore not protected against
	 * reentrant calls.
	 * Therefore call to onAlarm when delay elapses must be protected from
	 * other Nagra trusted client calls like TFL calls.
	 * Remember also that Nagra trusted client may call any platform services the
	 * onAlarm callback.
	 */
	(void)xContext;

	return rtn;
}

/* -------------------------------------------------------------------------- */
/*                              nv_alarmCancel                                */
/* -------------------------------------------------------------------------- */
static void nv_alarmCancel(TNvHandle xAlarm, TNvHandle *pxContext)
{
	if (NULL == xAlarm)
		return;
	/*
	 * GP TEE standard definition does not provide timer support.
	 * Therefore this implementation is platform-specific.
	 * Check other provided examples.
	 */
	(void)pxContext;
}

/* -------------------------------------------------------------------------- */
/*                              END OF FILE                                   */
/* -------------------------------------------------------------------------- */
