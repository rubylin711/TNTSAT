/******************************************************************************
 *
 * Copyright 2014, Deco, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of
 * Deco, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
 *
 *****************************************************************************/
/**
 * @file platform_api.h
 *
 * @brief Platform API functions to initialize a hardware interface and access registers using it
 *
 *****************************************************************************/
#ifndef __PLATFORM_API_H__
#define __PLATFORM_API_H__

#include "si_datatypes.h"
#if (__HDMI_OS_LINUX__)
	#include "aardvark.h"
	#include "BlackBox.h"
	#include "cheetah.h"
#elif (__HDMI_UBOOT__)
	#include "mt_platform.h"
#endif

/***** public macro definitions ***********************************************/

#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__)
	#define SII_ENV_BUILD_ASSERT 1 //!< Macro to enable/Disable assering on error
#else
	#define SII_ENV_BUILD_ASSERT 0 //!< Macro to enable/Disable assering on error
#endif

/***** public type definitions ***********************************************/

/**
* @brief Platfrom operation (read/write/platform initialization) satus
*/
typedef enum {
	SII_PLATFORM_STATUS__SUCCESS, //!< Platform operation successful
	SII_PLATFORM_STATUS__FAILED //!< platoform opeartion failed
} SiiPlatformStatus_t;

/**
* @brief Platfrom Interface type being used
*/
typedef enum {
	SII_PLATFORM_TYPE__GPIO, //!< All pins for GPIOs only (only in Aardvark and Cheetah)
	SII_PLATFORM_TYPE__I2C, //!< I2C interface
	SII_PLATFORM_TYPE__PRIF_OVER_I2C, //!< Parallel interface over I2C interface
	SII_PLATFORM_TYPE__SPI //!< SPI interface
} SiiPlatformInterfaceType_t;

/**
* @brief Hardware being used for the interface
*/
typedef enum {
	SII_PLATFORM_HARDWARE__BB, //!< BlackBox (Only I2C interface is allowed)
	SII_PLATFORM_HARDWARE__AARDVARK, //!< Aardvark
	SII_PLATFORM_HARDWARE__CHEETAH //!< Cheetah
} SiiPlatformInterfaceHardware_t;

/**
* @brief Details of interface being used
* @see @ref SiiPlatformInterfaceType_t, @ref SiiPlatformInterfaceHardware_t
*/
typedef struct {
	SiiPlatformInterfaceType_t interfaceType; //!< Interface type being used
	SiiPlatformInterfaceHardware_t interfaceHardware; //!< Hardware being used for interface
	uint32_t handle; //!< Handle to the hardware interface device opened
} SiiPlatformInterface_t;

/**
* @brief GPIO pins enumeration
*/
typedef enum {
	SII_PLATFORM_GPIO__0, //!< GPIO pin 0
	SII_PLATFORM_GPIO__1, //!< GPIO pin 1
	SII_PLATFORM_GPIO__2, //!< GPIO pin 2
	SII_PLATFORM_GPIO__3, //!< GPIO pin 3
	SII_PLATFORM_GPIO__4, //!< GPIO pin 4
	SII_PLATFORM_GPIO__5, //!< GPIO pin 5
	SII_PLATFORM_GPIO__6, //!< GPIO pin 6
	SII_PLATFORM_GPIO__7, //!< GPIO pin 7
} SiiPlatformGPIO_t;

/**
* @brief GPIO direction
*/
typedef enum {
	SII_PLATFORM_GPIO_DIRECTION__INPUT, //!< GPIO pin as Input
	SII_PLATFORM_GPIO_DIRECTION__OUTPUT, //!< GPIO pin as output
	SII_PLATFORM_GPIO_DIRECTION__INOUT, //!< GPIO pin as bi-directional
} SiiPlatformGPIODirection_t;

/**
* brief GPIO signal level
*/
typedef enum {
	SII_PLATFORM_GPIO_LEVEL__LOW, //!< GPIO pin level is low
	SII_PLATFORM_GPIO_LEVEL__HIGH  //!< GPIO pin level is high
} SiiPlatformGPIOLevel_t;

/***** public functions ******************************************************/

/*****************************************************************************/
/**
* @brief Initialize the platform of interest. Multiple flatform initialization is allowed.
*        Registers can be accessed using multiple hardware by providing the @ref SiiPlatformInterface_t
*        to each API.
*
* @param[in] pInterfaceInfo - pointer to the datastructure containing interface type and hardware being used
*
* @retval Returns the Success/Failure status of initialization
* @see @ref SiiPlatformInterface_t
*/
bool_t SiiPlatformInit(SiiPlatformInterface_t *pInterfaceInfo);

SiiPlatformStatus_t SiiPlatformClose(SiiPlatformInterface_t *pInterfaceInfo);

/**
* @brief API to read registers.
*
* @param[in] pInterfaceInfo - pointer to the datastructure containing interface type and hardware being used
* @param[in] devId - Device Id of the register page.  This parameter has no significance is SPI interface.
* @param[in] address - address of the register in the page.
* @param[out] data_in - pointer to the data.
* @param[in] length - number of registers to read.
*
* @retval Returns the Success/Failure status of read operartion
* @see @ref SiiPlatformInterface_t
*/
SiiPlatformStatus_t SiiPlatformRead(SiiPlatformInterface_t *pInterfaceInfo, uint8_t devId, uint16_t address, uint8_t *data_in, uint16_t length);

/**
* @brief API to write into registers.
*
* @param[in] pInterfaceInfo - pointer to the datastructure containing interface type and hardware being used
* @param[in] devId - Device Id of the register page. This parameter has no significance is SPI interface.
* @param[in] address - address of the register in the page.
* @param[in] data_out - pointer to the data.
* @param[in] length - number of registers to read.
*
* @retval Returns the Success/Failure status of write operartion
* @see @ref SiiPlatformInterface_t
*/
SiiPlatformStatus_t SiiPlatformWrite(SiiPlatformInterface_t *pInterfaceInfo, uint8_t devId, uint16_t address, uint8_t *data_out, uint16_t length);

/**
* @brief API to configure the GPIO pin as input or output are bidirectional.
*
* @param[in] pInterfaceInfo - pointer to the datastructure containing interface type and hardware being used
* @param[in] gpioPin - GPIO pin
*                      In Aardvark and Cheetah hardware interface - SPI pins shall act as GPIOs in I2C configuration
*                                                                 - I2C pins shall act as GPIOs in SPI configuration
* @param[in] direction - direction of GPIO pin operation
* @retval returns the status of operation
* @see @ref SiiPlatformInterface_t, SiiPlatfromGPIO_t
*/
SiiPlatformStatus_t SiiPlatformGPIOConfig(SiiPlatformInterface_t *pInterfaceInfo, SiiPlatformGPIO_t gpioPin, SiiPlatformGPIODirection_t direction);

/**
* @brief API to drive the GPIO pin.
*
* @param[in] pInterfaceInfo - pointer to the datastructure containing interface type and hardware being used
* @param[in] gpioPin - Output GPIO pin
* @param[in] level - Output level GPIO pin
*
* @retval returns the status of operation
* @see @ref SiiPlatformInterface_t, SiiPlatfromGPIO_t
*/
SiiPlatformStatus_t SiiPlatformGPIOSet(SiiPlatformInterface_t *pInterfaceInfo, SiiPlatformGPIO_t gpioPin, SiiPlatformGPIOLevel_t level);

/**
* @brief API to read the status of GPIO
*
* @param[in] pInterfaceInfo - pointer to the datastructure containing interface type and hardware being used
* @param[in] gpioPin - input GPIO pin
*
* @retval level level on the GPIO pin

* @see @ref SiiPlatformInterface_t, SiiPlatfromGPIO_t
*/
SiiPlatformGPIOLevel_t SiiPlatformGPIOStatusGet(SiiPlatformInterface_t *pInterfaceInfo, SiiPlatformGPIO_t gpioPin);

//-------------------------------------------------------------------------------------------------
//! @brief      Returns number of passed milli seconds since TimeInit().
//!
//! @return     Number of milli seconds.
//-------------------------------------------------------------------------------------------------
uint32_t SiiPlatformTimeMilliGet( void );

//-------------------------------------------------------------------------------------------------
//! @brief      Blocks execution for x number of milli seconds.
//!
//! @param[in]  milliDelay - Number of milli seconds.
//-------------------------------------------------------------------------------------------------
void SiiPlatformTimeMilliDelay( uint32_t milliDelay );

#if(SII_ENV_BUILD_ASSERT)
/**
* @brief Assertion macro to check for internal error conditions
*/
#  define SII_PLATFORM_DEBUG_ASSERT( expr ) \
	( (void)( (/*lint -e{506}*/(ulong)(expr)) ? SiiPlatformDebugAssert(__FILE__, __LINE__, (uint32_t)(((ulong)(expr)) != 0UL), NULL) : ((void)NULL) ) )
#else // SII_ENV_BUILD_ASSERT
/**
* @brief Dummy assertion macro
*/
#  define SII_PLATFORM_DEBUG_ASSERT( expr ) ( (void)0 )
#endif // SII_ENV_BUILD_ASSERT
#if (SII_ENV_BUILD_ASSERT != 0)
	// Assertion handler
	// Not needed if SII_ENV_BUILD_ASSERT is set to 0
	/*****************************************************************************/
	/**
	* @brief Debug assertion handler
	*
	* This function is called when the @ref SII_PLATFORM_DEBUG_ASSERT() macro is called
	* and the expression in the parameter is \c 0.
	*
	* @param[in] pFileName    String with the file name where the assertion happened
	* @param[in] lineNumber   String with the line number where the assertion happened
	* @param[in] expressionEvaluation  Reserved for the future; do not use
	* @param[in] pConditionText        Reserved for the future; do not use
	*
	* @see SII_PLATFORM_DEBUG_ASSERT()
	* @see SII_ENV_BUILD_ASSERT
	*
	*****************************************************************************/
	void SiiPlatformDebugAssert (const char *pFileName, uint32_t lineNumber, uint32_t expressionEvaluation, const char *pConditionText);
#endif // SII_ENV_BUILD_ASSERT

//-------------------------------------------------------------------------------------------------
//! @brief      Blocks execution for x number of micro seconds.
//!
//! @param[in]  microDelay - Number of micro seconds.
//-------------------------------------------------------------------------------------------------
void SiiPlatformTimeMicroDelay( uint32_t microDelay );

#endif //__PLATFROM_API_H__
