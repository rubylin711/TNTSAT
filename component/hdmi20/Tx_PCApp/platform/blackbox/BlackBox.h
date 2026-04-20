#ifndef __BLACK_BOX_H__
#define __BLACK_BOX_H__

#include "mt_hdmi20_cfg.h"
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	//for ulong
	#include "mt_common.h"
#endif
#include "mpi_memdev.h"
/*=========================================================================
| TYPEDEFS
 ========================================================================*/
#define SYM6_COMPILE_ALL_PASS (0)
#ifndef TOTALPHASE_DATA_TYPES
	#define TOTALPHASE_DATA_TYPES

	#ifndef _MSC_VER
		/* C99-compliant compilers (GCC) */
		#include <stdint.h>
		typedef uint8_t   u08;
		typedef uint16_t  u16;
		typedef uint32_t  u32;
		//typedef uint64_t  u64;
		typedef int8_t    s08;
		typedef int16_t   s16;
		typedef int32_t   s32;
		//typedef int64_t   s64;

	#else
		/* Microsoft compilers (Visual C++) */
		typedef unsigned __int8   u08;
		typedef unsigned __int16  u16;
		typedef unsigned __int32  u32;
		typedef unsigned __int64  u64;
		typedef signed   __int8   s08;
		typedef signed   __int16  s16;
		typedef signed   __int32  s32;
		typedef signed   __int64  s64;

	#endif /* __MSC_VER */

#endif /* TOTALPHASE_DATA_TYPES */

typedef unsigned long       dword;
typedef unsigned char       byte;
typedef unsigned short      word;

typedef unsigned char BB_HANDLE;

#define BB_SUCCESS             (0)         // Successful function call
#define BB_FAILURE             (1)         // Failure in function call

#define I2C_MODE_FAST       (0)         // 400 kHz Fast Mode
#define I2C_MODE_STD        (1)         // 100 kHz Standard Mode
#define I2C_MODE_SLOW       (2)         // 10  kHz Standard Mode

#define I2C_ADDRMODE_8      (0)         // 8  Bit Address Mode
#define I2C_ADDRMODE_16     (1)         // 16 Bit Address Mode

#define I2C_MODE_FAST       (0)         // 400 kHz Fast Mode
#define I2C_MODE_STD        (1)         // 100 kHz Standard Mode
#define I2C_MODE_SLOW       (2)         // 10  kHz Standard Mode

#define I2C_ADDRMODE_8      (0)         // 8  Bit Address Mode
#define I2C_ADDRMODE_16     (1)         // 16 Bit Address Mode

#define I2C_TIMEOUT_DISABLE (0)         // Disable clock timeouts for I2C bus
#define I2C_TIMEOUT_MIN     (1)         // 4.4ms timeout (minimum possible)
#define I2C_TIMEOUT_MAX     (127)       // 564ms timeout (maximum possible)

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	#define HDMI20_REG_BASE_ADDR (0x1f480000UL)
	#define HDMI_REGISTER_FILE_SIZE (1<<10)
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	#define HDMI20_REG_BASE_ADDR (0x1f480000UL)
	#define HDMI_REGISTER_FILE_SIZE (1<<10)
#elif defined (CONFIG_MT_CHIP_ETUDE2)
	#define HDMI20_REG_BASE_ADDR (0x1f570000UL)
	#define HDMI20_IIP_REG_SIZE (4<<10)
	#define HDMI20_MTIP_REG_SIZE (1<<8)
	#define HDMI_REGISTER_FILE_SIZE (HDMI20_IIP_REG_SIZE + HDMI20_MTIP_REG_SIZE)
#elif defined (CONFIG_MT_CHIP_SYMPHONY6)
	#define HDMI20_REG_BASE_ADDR (0x1f480000UL)
	#define HDMI20_IIP_REG_SIZE (4<<10)
	#define HDMI20_MTIP_REG_SIZE (8<<10)
	#define HDMI_REGISTER_FILE_SIZE (HDMI20_IIP_REG_SIZE + HDMI20_MTIP_REG_SIZE)
#endif

#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__)
#endif

/**
 * @brief read register
 *
 * @param[in] u32RegUsrVirtAddr mapped register address(not physical address)
 */
static inline mt_u32 hdmi20_read_reg32(ulong RegUsrVirtAddr)
{
	mt_u32 __v = *(volatile mt_u32 *)(RegUsrVirtAddr);
	__iormb(__v);
	return __v;
}

/**
 * @brief write register
 *
 * @param[in] RegUsrVirtAddr mapped register address(not physical address)
 */
static inline mt_void hdmi20_write_reg32(ulong RegUsrVirtAddr, mt_u32 u32Value)
{
	__iowmb();
	*(volatile mt_u32 *)(RegUsrVirtAddr) = u32Value;
}

/**
 * @brief read register
 *
 * @param[in] RegUsrVirtAddr mapped register address(not physical address)
 */
static inline mt_u16 hdmi20_read_reg16(ulong RegUsrVirtAddr)
{
	mt_u16 __v = *(volatile mt_u16 *)(RegUsrVirtAddr);
	__iormb(__v);
	return __v;
}

/**
 * @brief write register
 *
 * @param[in] RegUsrVirtAddr mapped register address(not physical address)
 */
static inline mt_void hdmi20_write_reg16(ulong RegUsrVirtAddr, mt_u16 u16Value)
{
	__iowmb();
	*(volatile mt_u16 *)(RegUsrVirtAddr) = u16Value;
}

/**
 * @brief read register
 *
 * @param[in] RegUsrVirtAddr mapped register address(not physical address)
 */
static inline mt_u8 hdmi20_read_reg8(ulong RegUsrVirtAddr)
{
	mt_u8 __v = *(volatile mt_u8 *)(RegUsrVirtAddr);
	__iormb(__v);
	return __v;
}

/**
 * @brief write register
 *
 * @param[in] u32RegUsrVirtAddr mapped register address(not physical address)
 */
static inline mt_void hdmi20_write_reg8(ulong RegUsrVirtAddr, mt_u8 u8Value)
{
	__iowmb();
	*(volatile mt_u8 *)(RegUsrVirtAddr) = u8Value;
}

typedef struct {
	dword   readTimeout;                // FTDI read timeout in milliseconds
	dword   writeTimeout;               // FTDI write tiemout in milliseconds

	MT_BOOL    repeatedStart;              // TRUE = use repeated start for read operations
	MT_BOOL    isBigEndian;                // TRUE = WORDS and DWORDS sent MSB first (big-endian) on I2C bus
	MT_BOOL    regAddr16;                  // TRUE = reg address is 16 bits, FALSE = 8 bits
	MT_BOOL    noIncAddr;                  // FALSE = Increment reg address every packet (default)
	// TRUE  = Do not increment (usefull for FIFO writes)
	int     i2cBusMode;                 // I2C bus clock rate code
	int     i2cTimeout;                 // Length of time SCL can be held low before the state machine is reset.
	// Range: 0 - disable timeout
	//        1 - 127 (~4.4mS to ~564mS in 4.44mS steps)
} I2C_OPTIONS;

enum {
	GPIO_BIDI       = 0x00,             // Pin is bi-directional - driven both ways
	GPIO_INPUT      = 0x01,             // Pin is input only
	GPIO_PUSH_PULL  = 0x02,             // Pin is output only - Push-pull type
	GPIO_OPEN_DRAIN = 0x03,             // Pin is output only - Open drain type
};

typedef struct {
	dword   readTimeout;                // FTDI read timeout in milliseconds
	dword   writeTimeout;               // FTDI write tiemout in milliseconds

	byte    gpioCfg0;                   // GPIO Pin 0 configuration
	byte    gpioCfg1;                   // GPIO Pin 1 configuration
	byte    gpioCfg2;                   // GPIO Pin 2 configuration
	byte    gpioCfg3;                   // GPIO Pin 3 configuration
	byte    gpioCfg4;                   // GPIO Pin 4 configuration
	byte    gpioCfg5;                   // GPIO Pin 5 configuration
	byte    gpioCfg6;                   // GPIO Pin 6 configuration
	byte    gpioCfg7;                   // GPIO Pin 7 configuration
} GPIO_OPTIONS;

byte BlackBox_GetVersion (dword *pVersion);
byte BlackBox_Number (byte *number);
byte BlackBox_Open (byte number, byte * bbHandle);
byte BlackBox_Close (BB_HANDLE bbHandle);
byte BlackBox_Serial (BB_HANDLE bbHandle, char ** serial);
byte BlackBox_Description (BB_HANDLE bbHandle, char ** description);
byte BlackBox_GetLastStatus (char **pStatus);

byte I2C_ReadByte( BB_HANDLE bbHandle, byte deviceID, word regAddr);
word I2C_ReadWord (BB_HANDLE bbHandle, byte deviceID, word regAddr);
byte I2C_ReadBlock (BB_HANDLE bbHandle, byte deviceID, word regAddr, byte* pData, word length);
byte I2C_ReadSegmentBlock (BB_HANDLE bbHandle, byte deviceID, byte segAddr, word regAddr, byte *pData, word length);

void I2C_WriteByte (BB_HANDLE bbHandle, byte deviceID, word regAddr, byte value);
void I2C_WriteWord (BB_HANDLE bbHandle, byte deviceID, word regAddr, word value);
byte I2C_WriteBlock (BB_HANDLE bbHandle, byte deviceID, word regAddr, byte* Data, word length);

byte GetI2CRepeatedStartMode (BB_HANDLE bbHandle, byte* startMode);
byte SetI2CRepeatedStartMode (BB_HANDLE bbHandle, byte startMode);
byte GetI2CBigEndianMode (BB_HANDLE bbHandle, byte* endianMode);
byte SetI2CBigEndianMode (BB_HANDLE bbHandle, byte endianMode);
byte GetI2C16bitAddress (BB_HANDLE bbHandle, byte* is16bitMode);
byte SetI2C16bitAddress (BB_HANDLE bbHandle, byte is16bitMode);
byte GetI2CIncAddress (BB_HANDLE bbHandle, byte* incAddrMode);
byte SetI2CIncAddress (BB_HANDLE bbHandle, byte incAddrMode);

byte I2C_GetOptions (BB_HANDLE bbHandle, I2C_OPTIONS *pOptions);
byte I2C_SetOptions (BB_HANDLE bbHandle, I2C_OPTIONS pOptions);

byte GPIO_GetPins (BB_HANDLE bbHandle, byte pinMask);
byte GPIO_SetPins (BB_HANDLE bbHandle, byte pinMask);
byte GPIO_ClearPins (BB_HANDLE bbHandle, byte pinMask);
byte GPIO_ModifyPins( BB_HANDLE bbHandle, byte PinMask, byte PinState);

byte GPIO_GetOptions (BB_HANDLE bbHandle, GPIO_OPTIONS *pOptions);
byte GPIO_SetOptions (BB_HANDLE bbHandle, GPIO_OPTIONS pOptions);

#endif /* __BLACK_BOX_H__ */
