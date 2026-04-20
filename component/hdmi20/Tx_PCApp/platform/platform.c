#include "platform_api.h"
#include "sii_time.h"
#if (__HDMI_OS_LINUX__)
#include <stdio.h>
#include <stdlib.h>

#define I2C_BIT_RATE 400
#define AARDVARK_SPI_BIT_RATE 125
#define CHEETAH_SPI_BIT_RATE 100

uint16_t ports[16] = {0xFF};
uint16_t selectedPort;

int16_t getAvailableDevice(SiiPlatformInterfaceHardware_t hardware);
void BB_Version( void );
int BB_SetI2C100kHz( BB_HANDLE handle );

int16_t getAvailableDevice(SiiPlatformInterfaceHardware_t hardware)
{
	uint32_t unique_ids[16];
	int nelem = 16;

	if (hardware == SII_PLATFORM_HARDWARE__AARDVARK) {
		// Find all the attached devices
		int count = aa_find_devices_ext(nelem, (u16 *)ports, nelem, (u32 *)unique_ids);
		u16 i;

		// Print the information on each device
		if (count > nelem) {
			count = nelem;
		}
		for (i = 0; i < (u16)count; ++i) {
			// Determine if the device is in-use
			if (ports[i] & AA_PORT_NOT_FREE) {
				ports[i] &= ~AA_PORT_NOT_FREE;
			} else {
				return ports[i];
			}
		}
		printf("No Free Aardvark Device\n");
	} else if (hardware == SII_PLATFORM_HARDWARE__CHEETAH) {
		int count = ch_find_devices_ext(nelem, (u16 *)ports, nelem, (u32 *)unique_ids);
		int i;

		printf("%d device(s) found:\n", count);

		// Print the information on each device
		if (count > nelem) {
			count = nelem;
		}
		for (i = 0; i < count; ++i) {
			// Determine if the device is in-use
			if (ports[i] & CH_PORT_NOT_FREE) {
				ports[i] &= ~CH_PORT_NOT_FREE;
			} else {
				return ports[i];
			}
		}
		printf("No Free Cheetah Device\n");

	}
	return -1;
}

void BB_Version( void )
{
	dword dwVersion;

	BlackBox_GetVersion( &dwVersion );
	printf( "BlackBox version %lx.%02lx.%02lx.%02lx\n",
			dwVersion / 0x1000000,
			dwVersion / 0x10000 & 0xff,
			dwVersion / 0x100 & 0xff,
			dwVersion & 0xff);

}

int BB_SetI2C100kHz( BB_HANDLE handle )
{
	I2C_OPTIONS i2cOptions;

	if ( I2C_GetOptions( handle, &i2cOptions ) != BB_SUCCESS) {
		printf( "I2C_GetOptions error\n" );
		return SII_PLATFORM_STATUS__FAILED;
	} else {
		i2cOptions.i2cBusMode = I2C_MODE_FAST; //I2C_MODE_STD or I2C_MODE_FAST
		i2cOptions.i2cTimeout = 100;
		i2cOptions.readTimeout = 100;
		i2cOptions.writeTimeout = 100;
		if ( I2C_SetOptions( handle, i2cOptions ) != BB_SUCCESS) {
			printf( "I2C_SetOptions error\n" );
			return SII_PLATFORM_STATUS__FAILED;
		}
	}
	return SII_PLATFORM_STATUS__SUCCESS;
}

bool_t SiiPlatformInit(SiiPlatformInterface_t *pInterfaceInfo)
{
	Sii_Init_ProcessTime();
	switch (pInterfaceInfo->interfaceType) {
		case SII_PLATFORM_TYPE__I2C:
		case SII_PLATFORM_TYPE__PRIF_OVER_I2C:
			switch (pInterfaceInfo->interfaceHardware) {
				case SII_PLATFORM_HARDWARE__BB: {
					BB_HANDLE bbHandle;
					int8_t num;
					uint32_t num_t;
					int8_t i = 0, j;
					char *bbSerial = (char *)malloc(16);
					BlackBox_Number( (byte *)&num );
					if ( num == 0 ) {
						printf( "No BlackBox connected\n" );
						if (bbSerial) {
							free(bbSerial);
						}
						return SII_PLATFORM_STATUS__FAILED;
					}
					if (num > 1) {
						printf("Detecting BlackBoxes :\n");
					}
					for (j = 0; j < num; j++) {
						if ( BlackBox_Open( num - (j + 1), &bbHandle ) == BB_SUCCESS ) {
							BlackBox_Serial(bbHandle, &bbSerial);
							printf("%d. %s\n", i + 1, bbSerial);
							BlackBox_Close(bbHandle);
							i++;
						}
					}
					if (i == 0) {
						printf("None of the Blackbox could be opened\n");
						if (bbSerial) {
							free(bbSerial);
						}
						return SII_PLATFORM_STATUS__FAILED;
					}
					if (i > 1) {
						printf("Select the Device you want to continue with: \n");
						scanf("%d", &num_t);
						num = (int8_t)num_t;
					} else {
						num = 1;
					}
					if ( BlackBox_Open( num - 1, &bbHandle ) != BB_SUCCESS ) {
						printf("Selected Blackbox could not be opened\n");
						return SII_PLATFORM_STATUS__FAILED;
					}
					pInterfaceInfo->handle = bbHandle;
					BlackBox_Serial(bbHandle, &bbSerial);
					printf("Opened BB with Serial -%s\n", bbSerial);
					if (bbSerial) {
						free(bbSerial);
					}
					BB_Version();
					SetI2C16bitAddress( (BB_HANDLE)(pInterfaceInfo->handle), (pInterfaceInfo->interfaceType == SII_PLATFORM_TYPE__PRIF_OVER_I2C) ? 1 : 0 );
					return BB_SetI2C100kHz((BB_HANDLE)(pInterfaceInfo->handle));
				}
				break;
				case SII_PLATFORM_HARDWARE__AARDVARK: {
					AardvarkExt aaext;
					int bitrate;
					int16_t port  = getAvailableDevice(SII_PLATFORM_HARDWARE__AARDVARK);
					if (port >= 0) {
						pInterfaceInfo->handle  = aa_open_ext(port, &aaext);

						if (pInterfaceInfo->handle == 0) {
							printf("Unable to open Aardvark device on port %d\n", getAvailableDevice(SII_PLATFORM_HARDWARE__AARDVARK));
							printf("Error code = %d\n", pInterfaceInfo->handle);
							return SII_PLATFORM_STATUS__FAILED;
						}
						printf("Opened Aardvark; features = 0x%02x\n", aaext.features);

						// Ensure that the I2C subsystem is enabled
						aa_configure(pInterfaceInfo->handle, AA_CONFIG_GPIO_I2C);

						// Enable the I2C bus pullup resistors (2.2k resistors).
						// This command is only effective on v2.0 hardware or greater.
						// The pullup resistors on the v1.02 hardware are enabled by default.
						aa_i2c_pullup(pInterfaceInfo->handle, AA_I2C_PULLUP_BOTH);

						// Enable the Aardvark adapter's power pins.
						// This command is only effective on v2.0 hardware or greater.
						// The power pins on the v1.02 hardware are not enabled by default.
						aa_target_power(pInterfaceInfo->handle, AA_TARGET_POWER_BOTH);
						bitrate = aa_i2c_bitrate(pInterfaceInfo->handle, I2C_BIT_RATE);
						if ( bitrate < 0 ) {
							return SII_PLATFORM_STATUS__FAILED;
						}
					} else {
						return SII_PLATFORM_STATUS__FAILED;
					}
				}
				return SII_PLATFORM_STATUS__SUCCESS;
				case SII_PLATFORM_HARDWARE__CHEETAH:
					return SII_PLATFORM_STATUS__FAILED;
				default:
					return SII_PLATFORM_STATUS__FAILED;
			}
			break;
		case SII_PLATFORM_TYPE__SPI:
			switch (pInterfaceInfo->interfaceHardware) {
				case SII_PLATFORM_HARDWARE__AARDVARK: {
					AardvarkExt aaext;
					int bitrate;
					pInterfaceInfo->handle  = aa_open_ext(getAvailableDevice(pInterfaceInfo->interfaceHardware), &aaext);

					if (pInterfaceInfo->handle == 0) {
						printf("Unable to open Aardvark device on port %d\n", getAvailableDevice(pInterfaceInfo->interfaceHardware));
						printf("Error code = %d\n", pInterfaceInfo->handle);
						return SII_PLATFORM_STATUS__FAILED;
					}

					printf("Opened Aardvark; features = 0x%02x\n", aaext.features);

					// Ensure that the SPI subsystem is enabled
					aa_configure(pInterfaceInfo->handle, AA_CONFIG_SPI_GPIO);

					// Enable the Aardvark adapter's power pins.
					// This command is only effective on v2.0 hardware or greater.
					// The power pins on the v1.02 hardware are not enabled by default.
					aa_target_power(pInterfaceInfo->handle, AA_TARGET_POWER_BOTH);
					// Setup the clock phase
					aa_spi_configure(pInterfaceInfo->handle, AA_SPI_POL_RISING_FALLING, AA_SPI_PHASE_SAMPLE_SETUP, AA_SPI_BITORDER_MSB);

					// Setup the bitrate
					bitrate = aa_spi_bitrate(pInterfaceInfo->handle, CHEETAH_SPI_BIT_RATE);
					if ( bitrate < 0 ) {
						return SII_PLATFORM_STATUS__FAILED;
					}
					aa_spi_master_ss_polarity(pInterfaceInfo->handle, AA_SPI_SS_ACTIVE_LOW);
					aa_spi_slave_disable(pInterfaceInfo->handle);
				}
				return SII_PLATFORM_STATUS__SUCCESS;
				case SII_PLATFORM_HARDWARE__CHEETAH: {
					int bitrate;
					int16_t port  = getAvailableDevice(SII_PLATFORM_HARDWARE__CHEETAH);
					if (port >= 0) {
						// Open the device
						pInterfaceInfo->handle  = ch_open(port);
						ch_spi_configure(pInterfaceInfo->handle, CH_SPI_POL_RISING_FALLING, CH_SPI_PHASE_SAMPLE_SETUP, CH_SPI_BITORDER_MSB, 0x0);
						if (pInterfaceInfo->handle == 0) {
							printf("Unable to open Cheetah device on port %d\n", port);
							printf("Error code = %d (%s)\n", pInterfaceInfo->handle, ch_status_string(pInterfaceInfo->handle));
							return SII_PLATFORM_STATUS__FAILED;
						}
						ch_target_power(pInterfaceInfo->handle, CH_TARGET_POWER_ON);
						ch_sleep_ms(100);
						// Set the bitrate.
						bitrate = ch_spi_bitrate(pInterfaceInfo->handle, CHEETAH_SPI_BIT_RATE);
						if (bitrate < 0) {
							printf("Could not set bitrate.\n");
							printf("Error code = %d\n", bitrate);
							return SII_PLATFORM_STATUS__FAILED;
						}
					} else {
						return SII_PLATFORM_STATUS__FAILED;
					}
				}
				return SII_PLATFORM_STATUS__SUCCESS;
				default:
					return SII_PLATFORM_STATUS__FAILED;
			}
			break;
		default:
			return SII_PLATFORM_STATUS__FAILED;
	}
}

SiiPlatformStatus_t SiiPlatformClose(SiiPlatformInterface_t *pInterfaceInfo)
{
	switch (pInterfaceInfo->interfaceHardware) {
		case SII_PLATFORM_HARDWARE__BB:
			if (0 != BlackBox_Close((BB_HANDLE)pInterfaceInfo->handle)) {
				return SII_PLATFORM_STATUS__FAILED;
			}
			return SII_PLATFORM_STATUS__SUCCESS;
		case SII_PLATFORM_HARDWARE__AARDVARK:
			if (AA_I2C_STATUS_OK != aa_close(pInterfaceInfo->handle)) {
				return SII_PLATFORM_STATUS__FAILED;
			}
			return SII_PLATFORM_STATUS__SUCCESS;

		case SII_PLATFORM_HARDWARE__CHEETAH:
			if (CH_OK == ch_close(pInterfaceInfo->handle)) {
				return SII_PLATFORM_STATUS__SUCCESS;
			}
			return SII_PLATFORM_STATUS__FAILED;
	}
	return SII_PLATFORM_STATUS__FAILED;
}

SiiPlatformStatus_t SiiPlatformRead(SiiPlatformInterface_t *pInterfaceInfo, uint8_t devId, uint16_t address, uint8_t *data_in, uint16_t length)
{
	switch (pInterfaceInfo->interfaceHardware) {
		case SII_PLATFORM_HARDWARE__BB:
			switch (pInterfaceInfo->interfaceType) {
				case SII_PLATFORM_TYPE__I2C:
				case SII_PLATFORM_TYPE__PRIF_OVER_I2C:
					SetI2C16bitAddress( (BB_HANDLE)(pInterfaceInfo->handle), (pInterfaceInfo->interfaceType == SII_PLATFORM_TYPE__PRIF_OVER_I2C) ? 1 : 0 );
					if (0 != I2C_ReadBlock((BB_HANDLE)pInterfaceInfo->handle, devId, (word)address, (byte *)data_in, (word)length)) {
						printf("failed to read 0x%x - 0x%x\n", devId, address);
						return SII_PLATFORM_STATUS__FAILED;
					} else {
						return SII_PLATFORM_STATUS__SUCCESS;
					}
				default:
					return SII_PLATFORM_STATUS__FAILED;
			}
			break;
		case SII_PLATFORM_HARDWARE__AARDVARK:
			switch (pInterfaceInfo->interfaceType) {
				case SII_PLATFORM_TYPE__I2C: {
					u16 num_written;
					u16 num_read;
					if (AA_I2C_STATUS_OK != aa_i2c_write_read(pInterfaceInfo->handle, devId >> 1, AA_I2C_NO_FLAGS, 1, (u08*)&address,
							&num_written, (u16)length, (u08*)data_in, &num_read)) {
						printf("failed to read 0x%x - 0x%x\n", devId, address);
						return SII_PLATFORM_STATUS__FAILED;
					}
				}
				return SII_PLATFORM_STATUS__SUCCESS;
				case SII_PLATFORM_TYPE__PRIF_OVER_I2C: {
					u16 num_written;
					u16 num_read;
					uint8_t prifAddr[2] = {0x00, 0x00};
					prifAddr[0] = (uint8_t)((address & 0xFF00) >> 8);
					prifAddr[1] = (uint8_t)(address & 0x00FF);
					if (AA_I2C_STATUS_OK != aa_i2c_write_read(pInterfaceInfo->handle, devId >> 1, AA_I2C_NO_FLAGS, 2, (u08*)prifAddr,
							&num_written, (u16)length, (u08*)data_in, &num_read)) {
						printf("failed to read 0x%x - 0x%x\n", devId, address);
						return SII_PLATFORM_STATUS__FAILED;
					}
				}
				return SII_PLATFORM_STATUS__SUCCESS;
				case SII_PLATFORM_TYPE__SPI: {
					uint8_t spiAddr[258] = {0x00};
					uint8_t inData[259];
					address <<= 1;
					address |= 0x01; //read command
					spiAddr[0] = (uint8_t)((address >> 8) & 0x00FF);
					spiAddr[1] = (uint8_t)(address & 0x00FF);
					//in above see if a loop is required
					for (;;) {
						int total_bytes_read = 0;
						int num_bytes_read = 0;

						// Read the SPI message.
						// This function has an internal timeout (see datasheet).
						// To use a variable timeout the function aa_async_poll could
						// be used for subsequent messages.
						num_bytes_read = aa_spi_write(pInterfaceInfo->handle, length + 2, spiAddr, length + 2, inData);
						//num_bytes_read = aa_spi_slave_read(pInterfaceInfo->handle, (length - num_bytes_read) & 0xFFFF, data_in+num_bytes_read);

						if ((num_bytes_read & 0xFFFF) < 0 && num_bytes_read != AA_SPI_SLAVE_TIMEOUT) {
							printf("error: %s\n", aa_status_string(num_bytes_read));
							return SII_PLATFORM_STATUS__FAILED;
						} else {
							total_bytes_read = total_bytes_read + (num_bytes_read & 0xFFFF);
						}
						if ((total_bytes_read & 0xFFFF) == length + 2) {
							memcpy(data_in, inData + 2, length);
							return SII_PLATFORM_STATUS__SUCCESS;
						}
					}
				}
				break;
				default:
					return SII_PLATFORM_STATUS__FAILED;
			}
			break;
		case SII_PLATFORM_HARDWARE__CHEETAH:
			switch (pInterfaceInfo->interfaceType) {
				case SII_PLATFORM_TYPE__SPI: {
					int count;
					uint8_t spiAddr[259] = {0x00};
					uint8_t inData[259];
					uint16_t length_to_queue = 0;
					memset(&spiAddr, 0x00, 259);
					memset(&inData, 0x00, 259);
					//need to have some work around for >400
					length_to_queue = length + 2;
					address <<= 1;
					address |= 0x01; //read command
					spiAddr[0] = (uint8_t)((address >> 8) & 0x00FF);
					spiAddr[1] = (uint8_t)(address & 0x00FF);
					// Reset the state of the bus.
					ch_spi_queue_clear(pInterfaceInfo->handle);
					ch_spi_queue_oe(pInterfaceInfo->handle, 1);

					// Set slave select to deasserted state, in case it was left
					// low by a previously interrupted transaction (ctrl-c).  This
					// will reset the state machine inside the flash.
					ch_spi_queue_ss(pInterfaceInfo->handle, 0);

					ch_spi_queue_ss(pInterfaceInfo->handle, 1);
					ch_spi_queue_array(pInterfaceInfo->handle, length_to_queue, spiAddr);
					ch_spi_queue_ss(pInterfaceInfo->handle, 0);
					count = ch_spi_batch_shift(pInterfaceInfo->handle, length_to_queue, inData);
					if (count != length_to_queue) {
						printf("Expected %d bytes but only received %d bytes\n",
							   length_to_queue, count);
						return SII_PLATFORM_STATUS__FAILED;
					}
					memcpy(data_in, inData + length_to_queue - length, length);
					// Reset the state of the bus.
					ch_spi_queue_clear(pInterfaceInfo->handle);
					ch_spi_queue_oe(pInterfaceInfo->handle, 0);
					ch_spi_batch_shift(pInterfaceInfo->handle, 0, 0);
					return SII_PLATFORM_STATUS__SUCCESS;
				}
				break;
				default:
					return SII_PLATFORM_STATUS__FAILED;
			}
			break;
	}
	return SII_PLATFORM_STATUS__FAILED;
}

SiiPlatformStatus_t SiiPlatformWrite(SiiPlatformInterface_t *pInterfaceInfo, uint8_t devId, uint16_t address, uint8_t *data_out, uint16_t length)
{
	switch (pInterfaceInfo->interfaceHardware) {
		case SII_PLATFORM_HARDWARE__BB:
			switch (pInterfaceInfo->interfaceType) {
				case SII_PLATFORM_TYPE__I2C:
				case SII_PLATFORM_TYPE__PRIF_OVER_I2C:
					SetI2C16bitAddress( (BB_HANDLE)(pInterfaceInfo->handle), (pInterfaceInfo->interfaceType == SII_PLATFORM_TYPE__PRIF_OVER_I2C) ? 1 : 0 );
					if (0 != I2C_WriteBlock((BB_HANDLE)pInterfaceInfo->handle, devId, (word)address, (byte *)data_out, (word)length)) {
						printf("failed to write 0x%x - 0x%x\n", devId, address);
						return SII_PLATFORM_STATUS__FAILED;
					} else {
						return SII_PLATFORM_STATUS__SUCCESS;
					}
				default:
					return SII_PLATFORM_STATUS__FAILED;
			}
			break;
		case SII_PLATFORM_HARDWARE__AARDVARK:
			switch (pInterfaceInfo->interfaceType) {
				case SII_PLATFORM_TYPE__I2C: {
					u16 num_written;
					u08 data_final[256 + 1]; // +1 bytes for 1 byte of register offset
					data_final[0] = address & 0xFF;
					memcpy((uint8_t *)data_final + 1, (uint8_t *)data_out, length);
					if (AA_I2C_STATUS_OK != aa_i2c_write_ext(pInterfaceInfo->handle, devId >> 1, AA_I2C_NO_FLAGS, (u16)length + 1, data_final, &num_written)) {
						printf("failed to write 0x%x - 0x%x\n", devId, address);
						return SII_PLATFORM_STATUS__FAILED;
					}
					return SII_PLATFORM_STATUS__SUCCESS;
				}
				case SII_PLATFORM_TYPE__PRIF_OVER_I2C: {
					u16 num_written;
					u08 data_final[256 + 2]; // +2 bytes for 2 byte of register offset
					data_final[1] = address & 0xFF;
					data_final[0] = (address >> 8) & 0xFF;
					memcpy((uint8_t *)data_final + 2, (uint8_t *)data_out, length);
					if (AA_I2C_STATUS_OK != aa_i2c_write_ext(pInterfaceInfo->handle, devId >> 1, AA_I2C_NO_FLAGS, (u16)length + 2, data_final, &num_written)) {
						printf("failed to write 0x%x - 0x%x\n", devId, address);
						return SII_PLATFORM_STATUS__FAILED;
					}
					return SII_PLATFORM_STATUS__SUCCESS;
				}
				case SII_PLATFORM_TYPE__SPI: {
					int total_num_bytes = 0;
					int num_bytes_written = 0;
					uint8_t spiAddr[2 + 256] = {0x00, 0x00};
					uint8_t readData[2 + 256]; //junk data received while writing
					address <<= 1;
					address |= 0x00; //write command
					spiAddr[0] = (uint8_t)((address >> 8) & 0x00FF);
					spiAddr[1] = (uint8_t)(address & 0x00FF);
					memcpy(spiAddr + 2, data_out, length);
					for (;;) {
						// first write the address. ignore the data received while writing address
						num_bytes_written = aa_spi_write(pInterfaceInfo->handle, (u16)2 + length, spiAddr,
														 (u16)2 + length, readData);

						if (num_bytes_written < 0) {
							printf("error: %s\n", aa_status_string(num_bytes_written));
							return SII_PLATFORM_STATUS__FAILED;
						} else {
							total_num_bytes += num_bytes_written;
						}
						if (total_num_bytes == length + 2) {
							return SII_PLATFORM_STATUS__SUCCESS;
						}
					}
				}
				break;
				default:
					return SII_PLATFORM_STATUS__FAILED;
			}
			break;
		case SII_PLATFORM_HARDWARE__CHEETAH:
			switch (pInterfaceInfo->interfaceType) {
				case SII_PLATFORM_TYPE__SPI: {
					int batch;
					int count;
					uint8_t spiAddr[259] = {0x00};
					uint8_t inData[259];
					uint16_t length_to_queue = 0;
					memset(&spiAddr, 0x00, 259);
					memset(&inData, 0x00, 259);
					//need to have some work around for >400
					length_to_queue = length + 2;
					memcpy(spiAddr + length_to_queue - length, data_out, length);
					address <<= 1;
					address |= 0x00; //write command
					spiAddr[0] = (uint8_t)((address >> 8) & 0x00FF);
					spiAddr[1] = (uint8_t)(address & 0x00FF);
					// Reset the state of the bus.
					ch_spi_queue_clear(pInterfaceInfo->handle);
					ch_spi_queue_ss(pInterfaceInfo->handle, 0);
					ch_spi_queue_oe(pInterfaceInfo->handle, 1);
					ch_spi_batch_shift(pInterfaceInfo->handle, 0, 0);

					ch_spi_queue_ss(pInterfaceInfo->handle, 1);
					ch_spi_queue_array(pInterfaceInfo->handle, length_to_queue, spiAddr);
					ch_spi_queue_ss(pInterfaceInfo->handle, 0);
					batch = ch_spi_batch_length(pInterfaceInfo->handle);
					count = ch_spi_batch_shift(pInterfaceInfo->handle, length_to_queue, inData);
					if (count != batch) {
						printf("Expected %d bytes but only received %d bytes\n",
							   batch, count);
						return SII_PLATFORM_STATUS__FAILED;
					}
					return SII_PLATFORM_STATUS__SUCCESS;
				}
				break;
				default:
					return SII_PLATFORM_STATUS__FAILED;
			}
			break;
		default:
			return SII_PLATFORM_STATUS__FAILED;
	}
}

SiiPlatformStatus_t SiiPlatformGPIOConfig(SiiPlatformInterface_t *pInterfaceInfo, SiiPlatformGPIO_t gpioPin, SiiPlatformGPIODirection_t direction)
{
	switch (pInterfaceInfo->interfaceHardware) {
		case SII_PLATFORM_HARDWARE__BB:
			if ((gpioPin <= SII_PLATFORM_GPIO__7) && (gpioPin >= SII_PLATFORM_GPIO__0)) {
				GPIO_OPTIONS gpioOptions;
				uint8_t gpioCfgVal;
				if ( GPIO_GetOptions( pInterfaceInfo->handle & 0xFF, &gpioOptions ) ) {
					return SII_PLATFORM_STATUS__FAILED; // failed
				}
				switch (direction) {
					case SII_PLATFORM_GPIO_DIRECTION__INPUT:
						gpioCfgVal = GPIO_INPUT;
						break;
					case SII_PLATFORM_GPIO_DIRECTION__OUTPUT:
						gpioCfgVal = GPIO_PUSH_PULL;
						break;
					case SII_PLATFORM_GPIO_DIRECTION__INOUT:
						gpioCfgVal = GPIO_BIDI;
						break;
					default:
						return SII_PLATFORM_STATUS__FAILED;
				}
				switch (gpioPin) {
					case SII_PLATFORM_GPIO__0:
						gpioOptions.gpioCfg0 = gpioCfgVal;
						break;
					case SII_PLATFORM_GPIO__1:
						gpioOptions.gpioCfg1 = gpioCfgVal;
						break;
					case SII_PLATFORM_GPIO__2:
						gpioOptions.gpioCfg2 = gpioCfgVal;
						break;
					case SII_PLATFORM_GPIO__3:
						gpioOptions.gpioCfg3 = gpioCfgVal;
						break;
					case SII_PLATFORM_GPIO__4:
						gpioOptions.gpioCfg4 = gpioCfgVal;
						break;
					case SII_PLATFORM_GPIO__5:
						gpioOptions.gpioCfg5 = gpioCfgVal;
						break;
					case SII_PLATFORM_GPIO__6:
						gpioOptions.gpioCfg6 = gpioCfgVal;
						break;
					case SII_PLATFORM_GPIO__7:
						gpioOptions.gpioCfg7 = gpioCfgVal;
						break;
				}
				if ( GPIO_SetOptions( pInterfaceInfo->handle & 0xFF, gpioOptions ) ) {
					return SII_PLATFORM_STATUS__FAILED; // failed
				}
				return SII_PLATFORM_STATUS__SUCCESS;

			}
			return SII_PLATFORM_STATUS__FAILED;
			break;
		case SII_PLATFORM_HARDWARE__AARDVARK: {
			uint8_t gpioMask = 0x00;
			switch (pInterfaceInfo->interfaceType) {
				case SII_PLATFORM_TYPE__I2C:
				case SII_PLATFORM_TYPE__PRIF_OVER_I2C:
					gpioMask = 0x01;
					if ((gpioPin > SII_PLATFORM_GPIO__5) || (gpioPin <= SII_PLATFORM_GPIO__1)) {
						return SII_PLATFORM_STATUS__FAILED;
					}
					break;
				case SII_PLATFORM_TYPE__SPI:
					gpioMask = 0x01;
					if ((gpioPin > SII_PLATFORM_GPIO__1) || (gpioPin < SII_PLATFORM_GPIO__0)) {
						return SII_PLATFORM_STATUS__FAILED;
					}
					break;
				case SII_PLATFORM_TYPE__GPIO:
					gpioMask = 0x01;
					if ((gpioPin > SII_PLATFORM_GPIO__5) || (gpioPin < SII_PLATFORM_GPIO__0)) {
						return SII_PLATFORM_STATUS__FAILED;
					}
					break;
			}
			{
				int gpioCfgVal;
				gpioCfgVal = aa_gpio_get(pInterfaceInfo->handle);
				switch (direction) {
					case SII_PLATFORM_GPIO_DIRECTION__INPUT:
						gpioCfgVal &= ~(gpioMask << (gpioPin));
						aa_gpio_direction(pInterfaceInfo->handle, gpioCfgVal & 0xFF);
						break;
					case SII_PLATFORM_GPIO_DIRECTION__OUTPUT:
						gpioCfgVal |= (gpioMask << (gpioPin));
						aa_gpio_direction(pInterfaceInfo->handle, gpioCfgVal & 0xFF);
						break;
					case SII_PLATFORM_GPIO_DIRECTION__INOUT:
						return SII_PLATFORM_STATUS__FAILED;
					default:
						return SII_PLATFORM_STATUS__FAILED;
				}
			}
			return SII_PLATFORM_STATUS__SUCCESS;
		}
		break;
		case SII_PLATFORM_HARDWARE__CHEETAH:
			break;
		default:
			return SII_PLATFORM_STATUS__FAILED;
	}
	return SII_PLATFORM_STATUS__FAILED;
}

SiiPlatformStatus_t SiiPlatformGPIOSet(SiiPlatformInterface_t *pInterfaceInfo, SiiPlatformGPIO_t gpioPin, SiiPlatformGPIOLevel_t level)
{
	switch (pInterfaceInfo->interfaceHardware) {
		case SII_PLATFORM_HARDWARE__BB:
			if ((gpioPin <= SII_PLATFORM_GPIO__7) && (gpioPin >= SII_PLATFORM_GPIO__0)) {
				if (level == SII_PLATFORM_GPIO_LEVEL__HIGH) {
					GPIO_SetPins( pInterfaceInfo->handle & 0xFF, 1 << gpioPin );
				} else {
					GPIO_ClearPins( pInterfaceInfo->handle & 0xFF, 1 << gpioPin );
				}
				return SII_PLATFORM_STATUS__SUCCESS;
			}
			return SII_PLATFORM_STATUS__FAILED;
		case SII_PLATFORM_HARDWARE__AARDVARK: {
			uint8_t value = 0;
			uint8_t gpioMask = 0x01;
			int gpiostat = 0;
			gpiostat = aa_gpio_get(pInterfaceInfo->handle);
			switch (pInterfaceInfo->interfaceType) {
				case SII_PLATFORM_TYPE__I2C:
				case SII_PLATFORM_TYPE__PRIF_OVER_I2C:
					if ((gpioPin > SII_PLATFORM_GPIO__5) || (gpioPin <= SII_PLATFORM_GPIO__1)) {
						return SII_PLATFORM_STATUS__FAILED;
					}
					break;
				case SII_PLATFORM_TYPE__SPI:
					if ((gpioPin > SII_PLATFORM_GPIO__1) || (gpioPin < SII_PLATFORM_GPIO__0)) {
						return SII_PLATFORM_STATUS__FAILED;
					}
					break;
				case SII_PLATFORM_TYPE__GPIO:
					if ((gpioPin > SII_PLATFORM_GPIO__5) || (gpioPin < SII_PLATFORM_GPIO__0)) {
						return SII_PLATFORM_STATUS__FAILED;
					}
					break;
				default:
					return SII_PLATFORM_STATUS__FAILED;
			}
			value = ((level == SII_PLATFORM_GPIO_LEVEL__HIGH) ? (gpiostat | (gpioMask << gpioPin)) : (gpiostat & ~(gpioMask << gpioPin))) & 0xFF;
			aa_gpio_set(pInterfaceInfo->handle, value);
			return SII_PLATFORM_STATUS__SUCCESS;
		}
		case SII_PLATFORM_HARDWARE__CHEETAH:
			break;
		default:
			return SII_PLATFORM_STATUS__FAILED;
	}
	return SII_PLATFORM_STATUS__FAILED;
}

SiiPlatformGPIOLevel_t SiiPlatformGPIOStatusGet(SiiPlatformInterface_t *pInterfaceInfo, SiiPlatformGPIO_t gpioPin)
{
	switch (pInterfaceInfo->interfaceHardware) {
		case SII_PLATFORM_HARDWARE__BB:
			return  GPIO_GetPins(pInterfaceInfo->handle & 0xFF, 1 << gpioPin) ? SII_PLATFORM_GPIO_LEVEL__HIGH : SII_PLATFORM_GPIO_LEVEL__LOW;
		case SII_PLATFORM_HARDWARE__AARDVARK: {
			int gpiostat = 0;
			gpiostat = aa_gpio_get(pInterfaceInfo->handle);
			return (gpiostat & (0x01 << gpioPin)) ? SII_PLATFORM_GPIO_LEVEL__HIGH : SII_PLATFORM_GPIO_LEVEL__LOW;
		}
		break;
		case SII_PLATFORM_HARDWARE__CHEETAH:
			return SII_PLATFORM_GPIO_LEVEL__LOW;
		default:
			return SII_PLATFORM_GPIO_LEVEL__LOW;
	}
}
#endif

void SiiPlatformTimeMilliDelay( uint32_t millDelay )
{
	SiI_DelayMS((unsigned long)millDelay);
}

uint32_t SiiPlatformTimeMilliGet(void)
{
	return Sii_Get_Process_Time();
}

#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__)
#if (SII_ENV_BUILD_ASSERT != 0)
void SiiPlatformDebugAssert( const char *pFileName, uint32_t lineNumber,
							 uint32_t expressionEvaluation, const char *pConditionText )
{
	// The following two parameters are currently not used
	//expressionEvaluation = expressionEvaluation; //to avoid compiler warnings about unused variable
	//pConditionText       = pConditionText; // to avoid compiler warnings about unused variable

	#if 1
	//pFileName = pFileName; // to avoid compiler warnings about unused variable
	//lineNumber = lineNumber; // to avoid compiler warnings about unused variable
	#else
	// Example how to print the place where error happened:
	// Print file name and line number caused an assertion
	printf("\n\nAssertion failure: File %s, Line %d\n\n", pFileName, (int) lineNumber);
	#endif
	if ( expressionEvaluation == 0 ) {
		printf("\n\nAsserting with failure in %s Line %d\n\n", pFileName, (int) lineNumber);
		// Stop SW execution to make the issue noticeable
		for (;;) {}; // Note: just an example
	}
}
#endif // SII_ENV_BUILD_ASSERT
#endif

void SiiPlatformTimeMicroDelay( uint32_t microDelay )
{
	SiI_DelayUS((unsigned long)microDelay);
}

