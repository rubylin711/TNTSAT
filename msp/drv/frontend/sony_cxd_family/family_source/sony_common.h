/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage-LZ Group                                                          */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2021 Montage-LZ Group Limited. All Rights Reserved.         */
/*****************************************************************************/

/*------------------------------------------------------------------------------
  Copyright 2016-2021 Sony Semiconductor Solutions Corporation

  Last Updated    : 2021/06/21
  Modification ID : e235d15d1807a3ccb021191f62ae61f20440b7ae
------------------------------------------------------------------------------*/
/**
 @file  sony_common.h

 @brief Common definitions and functions used in all reference code.

        The user should modify several points depend on their software system.
        Please check "<PORTING>" comments.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_COMMON_H
#define SONY_COMMON_H

/* Type definitions. */
/* <PORTING> Please comment out if conflicted */
#if 0//def __linux__
#include <stdint.h>
#else
typedef unsigned char uint8_t;      /**< Unsigned  8 bit integer. */
typedef unsigned short uint16_t;    /**< Unsigned 16 bit integer. */
typedef unsigned int uint32_t;      /**< Unsigned 32 bit integer. */
typedef signed char int8_t;         /**< Signed    8 bit integer. */
typedef signed short int16_t;       /**< Signed   16 bit integer. */
typedef signed int int32_t;         /**< Signed   32 bit integer. */
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

/* <PORTING> Sleep function define (Please modify as you like) */
#if defined(_WINDOWS)
#include "sony_windows_sleep.h"
#define SONY_SLEEP(n) sony_windows_Sleep(n)
#elif defined(__linux__)
//#include <unistd.h>
#include <linux/delay.h>
//#define SONY_SLEEP(n) usleep(n * 1000)
//#define SONY_SLEEP(n) mdelay(n)

extern void sony_sleep_delay_ms(uint32_t ms);

#define SONY_SLEEP(n) sony_sleep_delay_ms(n)
#endif

#ifndef SONY_SLEEP
#error SONY_SLEEP(n) is not defined. This macro must be ported to your platform.
#endif

/* <PORTING> Trace function enable */
/* #define SONY_TRACE_ENABLE */
/* <PORTING> Enable if I2C related function trace is necessary */
/* #define SONY_TRACE_I2C_ENABLE */

/**
 @brief Macro to specify unused argument and suppress compiler warnings.
*/
#define SONY_ARG_UNUSED(arg) ((void)(arg))

/*------------------------------------------------------------------------------
  Enumerations
------------------------------------------------------------------------------*/
/**
 @brief Return codes

 @note  Function trace function will be useful to know errored line in the reference code.
        To enable function trace, please define SONY_TRACE_ENABLE macro.
*/
typedef enum {
    /**
     @brief The function was successfully finished.
    */
    SONY_RESULT_OK,

    /**
     @brief Invalid argument.
    */
    SONY_RESULT_ERROR_ARG,

    /**
     @brief I2C communication error.
    */
    SONY_RESULT_ERROR_I2C,

    /**
     @brief Current software state is not valid.

            e.g. Calls monitor API but the demodulator is in sleep state.
    */
    SONY_RESULT_ERROR_SW_STATE,

    /**
     @brief Current device internal state is not valid.

            Many monitor APIs can return this error code in low signal quality situation
            because such monitor values can be obtained only if the demodulator is locked.
    */
    SONY_RESULT_ERROR_HW_STATE,

    /**
     @brief Timeout occurred.

            Tune API may return this error code if the demodulator cannot lock
            the signal within defined time period.
    */
    SONY_RESULT_ERROR_TIMEOUT,

    /**
     @brief Failed to lock.

            Tune API may return this error code if the demodulator unlock detection flag is raised.
            This means the valid signal does not exist on current frequency.
    */
    SONY_RESULT_ERROR_UNLOCK,

    /**
     @brief Out of range error.
    */
    SONY_RESULT_ERROR_RANGE,

    /**
     @brief Not supported for current device.
    */
    SONY_RESULT_ERROR_NOSUPPORT,

    /**
     @brief The operation is cancelled.
    */
    SONY_RESULT_ERROR_CANCEL,

    /**
     @brief Other error.
    */
    SONY_RESULT_ERROR_OTHER,

    /**
     @brief Memory overflow.

            This error means the API ran out of memory region allocated by user.
    */
    SONY_RESULT_ERROR_OVERFLOW,

    /**
     @brief Tune was successful, but please confirm parameters.
    */
    SONY_RESULT_OK_CONFIRM
} sony_result_t;

/*------------------------------------------------------------------------------
  Common functions
------------------------------------------------------------------------------*/
/**
 @brief Convert unsigned integer to signed integer.

 @param value Unsigned integer value.
 @param bitlen The bit width of "value".

 @return The signed integer value.
*/
int32_t sony_Convert2SComplement(uint32_t value, uint32_t bitlen);

/**
 @brief Split bit from byte array data.

 @param pArray The byte array data.
 @param startBit The start bit position.
 @param bitNum The bit length.

 @return The data split from byte array data.
*/
uint32_t sony_BitSplitFromByteArray(uint8_t *pArray, uint32_t startBit, uint32_t bitNum);
/*------------------------------------------------------------------------------
  Trace
------------------------------------------------------------------------------*/
/*
 Disables MS compiler warning (__pragma(warning(disable:4127))
 with do { } while (0);
*/
/** Macro for beginning multiline macro. */
#define SONY_MACRO_MULTILINE_BEGIN  do {
#if ((defined _MSC_VER) && (_MSC_VER >= 1300))
#define SONY_MACRO_MULTILINE_END \
        __pragma(warning(push)) \
        __pragma(warning(disable:4127)) \
        } while(0) \
        __pragma(warning(pop))
#else
/** Macro for ending multiline macro. */
#define SONY_MACRO_MULTILINE_END } while(0)
#endif


#ifdef SONY_TRACE_ENABLE
/* <PORTING> This is only a sample of trace macro. Please modify is necessary. */
void sony_trace_log_enter(const char* funcname, const char* filename, unsigned int linenum);
void sony_trace_log_return(sony_result_t result, const char* filename, unsigned int linenum);
#define SONY_TRACE_ENTER(func) sony_trace_log_enter((func), __FILE__, __LINE__)
#define SONY_TRACE_RETURN(result) \
    SONY_MACRO_MULTILINE_BEGIN \
        sony_result_t result_tmp = (result); \
        sony_trace_log_return(result_tmp, __FILE__, __LINE__); \
        return result_tmp; \
    SONY_MACRO_MULTILINE_END
#else /* SONY_TRACE_ENABLE */
/** Trace enter */
#define SONY_TRACE_ENTER(func)
/** Trace return */
#define SONY_TRACE_RETURN(result) return(result)
/** I/O trace enter */
#define SONY_TRACE_I2C_ENTER(func)
/** I/O trace return */
#define SONY_TRACE_I2C_RETURN(result) return(result)
#endif /* SONY_TRACE_ENABLE */


#ifdef SONY_TRACE_I2C_ENABLE
/* <PORTING> This is only a sample of trace macro. Please modify is necessary. */
void sony_trace_i2c_log_enter(const char* funcname, const char* filename, unsigned int linenum);
void sony_trace_i2c_log_return(sony_result_t result, const char* filename, unsigned int linenum);
#define SONY_TRACE_I2C_ENTER(func) sony_trace_i2c_log_enter((func), __FILE__, __LINE__)
#define SONY_TRACE_I2C_RETURN(result) \
    SONY_MACRO_MULTILINE_BEGIN \
        sony_result_t result_tmp = (result); \
        sony_trace_i2c_log_return(result_tmp, __FILE__, __LINE__); \
        return result_tmp; \
    SONY_MACRO_MULTILINE_END
#else /* SONY_TRACE_I2C_ENABLE */
#define SONY_TRACE_I2C_ENTER(func)
#define SONY_TRACE_I2C_RETURN(result) return(result)
#endif /* SONY_TRACE_I2C_ENABLE */


/*------------------------------------------------------------------------------
  Multi-threaded defines
 ------------------------------------------------------------------------------*/
/**
 @brief "<PORTING>" Defines for basic atomic operations for cancellation.
*/
typedef struct sony_atomic_t {
    /**
     @brief Underlying counter.
    */
    volatile int counter;
} sony_atomic_t;
#define sony_atomic_set(a,i) ((a)->counter = i)                 /**< Set counter atomically. */
#define sony_atomic_read(a) ((a)->counter)                      /**< Get counter atomically. */


/*------------------------------------------------------------------------------
  Stopwatch struct and functions definitions
------------------------------------------------------------------------------*/
/**
 @brief "<PORTING>" Stopwatch structure to measure accurate time.
*/
typedef struct sony_stopwatch_t {
    /**
     @brief Underlying start time.
    */
    uint32_t startTime;

} sony_stopwatch_t;

/**
 @brief Start the stopwatch.

 @param pStopwatch The stopwatch instance.

 @return SONY_RESULT_OK is successful.
*/
sony_result_t sony_stopwatch_start (sony_stopwatch_t * pStopwatch);

/**
 @brief Pause for a specified period of time.

 @param pStopwatch The stopwatch instance.

 @param ms  The time in milliseconds to sleep.

 @return SONY_RESULT_OK is successful.
*/
sony_result_t sony_stopwatch_sleep (sony_stopwatch_t * pStopwatch, uint32_t ms);

/**
 @brief Returns the elapsed time (ms) since the stopwatch was started.

 @param pStopwatch The stopwatch instance.

 @param pElapsed The elapsed time in milliseconds.

 @return SONY_RESULT_OK is successful.

*/
sony_result_t sony_stopwatch_elapsed (sony_stopwatch_t * pStopwatch, uint32_t* pElapsed);

#endif /* SONY_COMMON_H */
