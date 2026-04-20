/*=========================================================================
| INCLUDES
========================================================================*/
/* This #include can be customized to conform to the user's build paths. */
#include "BlackBox.h"
#include "si_drv_tx_regs.h"
#include "mt_common.h"

#if defined(CONFIG_MT_CHIP_ETUDE2)
	#define DEBUG_BB (0)
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
	#define DEBUG_BB (0)
#else
	#define DEBUG_BB (0)
#endif

#if DEBUG_BB
static union {
	char c[4];
	unsigned long mylong;
} endian_test = {{ 'l', '#', '#', 'b' } };
#define ENDIANNESS ((char)endian_test.mylong)
#define BB_PRINTF printf
#else
#define BB_PRINTF(fmt,args...)
#endif

ulong hdmi20_sys_reg_virt_addr = 0;
extern mt_s32 mpi_memdev_init(mt_void);
extern mt_s32 mpi_memdev_deinit(mt_void);
extern mt_s32 mpi_memdev_map_register(phys_addr_t RegAddr, mt_u32 u32Length, mt_void **pVirAddr);
extern mt_s32 mpi_memdev_unmap_register(mt_void *pVirAddr);
byte I2C_ReadBlock_4dump (BB_HANDLE bbHandle, byte deviceID, word regAddr, byte* pData, word length);

byte BlackBox_GetVersion (dword *pVersion)
{
	return BB_SUCCESS;
}

byte BlackBox_Number (byte *number)
{
	*number = 1;
	return BB_SUCCESS;
}

byte BlackBox_Open (byte number, byte * bbHandle)
{
	mt_s32 s32Ret = MT_SUCCESS;
	#if DEBUG_BB
	char *stringt = (ENDIANNESS == 'l') ? "Little Endian" : "Big Endian";
	#endif
	mpi_memdev_init();
	s32Ret = mpi_memdev_map_register(SYMPHONY_IO_PA(HDMI20_REG_BASE_ADDR), HDMI_REGISTER_FILE_SIZE, (mt_void*)&hdmi20_sys_reg_virt_addr);
	BB_PRINTF("\n%s_%d:%lx,%d, endia:%s\n", __func__, __LINE__, hdmi20_sys_reg_virt_addr, s32Ret, stringt);
	return s32Ret;
}

byte BlackBox_Close (BB_HANDLE bbHandle)
{
	mpi_memdev_unmap_register((mt_void*)hdmi20_sys_reg_virt_addr);
	BB_PRINTF("\n%s_%d:%lx\n", __func__, __LINE__, hdmi20_sys_reg_virt_addr);
	mpi_memdev_deinit();
	return BB_SUCCESS;
}

byte BlackBox_Serial (BB_HANDLE bbHandle, char ** serial)
{
	return BB_SUCCESS;
}

byte BlackBox_Description (BB_HANDLE bbHandle, char ** description)
{
	return BB_SUCCESS;
}

byte BlackBox_GetLastStatus (char **pStatus)
{
	return BB_SUCCESS;
}

byte I2C_ReadByte( BB_HANDLE bbHandle, byte deviceID, word regAddr)
{
	return BB_SUCCESS;
}

word I2C_ReadWord (BB_HANDLE bbHandle, byte deviceID, word regAddr)
{
	return BB_SUCCESS;
}

byte I2C_ReadBlock (BB_HANDLE bbHandle, byte deviceID, word regAddr, byte* pData, word length)
{
	int i = 0;
	//int hdmi_reg = 0;
	int ddprint = 1;

	if (regAddr == (BASE_ADDRESS + 0x7e) || \
			regAddr == (BASE_ADDRESS + 0x7f) || \
			regAddr == (BASE_ADDRESS + 0x925)  || \
			regAddr == (BASE_ADDRESS + 0xfa6)  || regAddr == (BASE_ADDRESS + 0xfa7) || \
			((regAddr >= (BASE_ADDRESS + 0x1010)) && (regAddr <= (BASE_ADDRESS + 0x1017)))) { // timer read intr registers
		ddprint = 0;
	}

	if (length > 1 && (regAddr != (BASE_ADDRESS + 0xfa6) && regAddr != (BASE_ADDRESS + 0xfa7))) {
		printf("\n%s_%d:d[%x, %d]\n", __func__, __LINE__, (int)regAddr, (int)length);
	}
	if (regAddr >= BASE_ADDRESS && regAddr < (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr -= BASE_ADDRESS;
		//hdmi_reg = 1;
		#if 0
		if (ddprint)
			BB_PRINTF("\n%s_%d:%x,regAddr=%x,length=%d,%x\n", __func__, __LINE__, hdmi20_sys_reg_virt_addr, \
					  (int)regAddr, (int)length, SYMPHONY_IO_PA(HDMI20_REG_BASE_ADDR + regAddr + i));
		#endif
	} else {
		//hdmi_reg = 0;
		if (ddprint)
			printf("\n%s_%d:%lx,regAddr=%x,length=%d,%lx\n", __func__, __LINE__, hdmi20_sys_reg_virt_addr, \
				   (int)regAddr, (int)length, SYMPHONY_IO_PA(HDMI20_REG_BASE_ADDR + regAddr + i));
		return BB_SUCCESS;
	}
	if (deviceID == 0x11) {
		//virtual dev id for mt register op
		for (i = 0; i < length; i++) {
			((mt_u32 *)pData)[i] = hdmi20_read_reg32(hdmi20_sys_reg_virt_addr + regAddr + (i << 2));
			if (ddprint) {
				BB_PRINTF("\n%s_%d:reg[%lx]=0x%x\n", __func__, __LINE__, SYMPHONY_IO_PA(HDMI20_REG_BASE_ADDR + regAddr + (i << 2)), ((mt_u32 *)pData)[i]);
			}
		}
	} else {
		for (i = 0; i < length; i++) {
			pData[i] = hdmi20_read_reg8((hdmi20_sys_reg_virt_addr + regAddr + i));
			if (ddprint) {
				BB_PRINTF("\n%s_%d:reg[%lx]=0x%x\n", __func__, __LINE__, SYMPHONY_IO_PA(HDMI20_REG_BASE_ADDR + regAddr + i), (int)pData[i]);
			}
		}
	}

	return BB_SUCCESS;
}

byte I2C_ReadSegmentBlock (BB_HANDLE bbHandle, byte deviceID, byte segAddr, word regAddr, byte *pData, word length)
{
	return BB_SUCCESS;
}

void I2C_WriteByte (BB_HANDLE bbHandle, byte deviceID, word regAddr, byte value)
{
	//return BB_SUCCESS;
}

void I2C_WriteWord (BB_HANDLE bbHandle, byte deviceID, word regAddr, word value)
{
	//return BB_SUCCESS;
}

byte I2C_WriteBlock (BB_HANDLE bbHandle, byte deviceID, word regAddr, byte* Data, word length)
{
	int i = 0;
	int ddprint = 1;

	#if 0
	if (length > 1) {
		printf("\n%s_%d:d[%x, %d]\n", __func__, __LINE__, (int)regAddr, (int)length);
	}
	#endif
	if (regAddr >= BASE_ADDRESS && regAddr < (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr -= BASE_ADDRESS;
	} else {
		#if 0
		printf("\n%s_%d:%x,regAddr=%x,length=%d,%d,%x\n", __func__, __LINE__, hdmi20_sys_reg_virt_addr, \
			   (int)regAddr, (int)length, (int)Data[0], SYMPHONY_IO_PA(HDMI20_REG_BASE_ADDR + regAddr + i));
		#endif
		return BB_SUCCESS;
	}

	if ((regAddr >= 0x1010 && regAddr <= 0x1017) || regAddr == 0x8cf\
			|| regAddr == 0xe50 || regAddr == 0xe4c || regAddr == 0xe4d || regAddr == 0xe4e\
			|| regAddr == 0xfa6  || regAddr == 0xfa7 \
	   ) {
		ddprint = 0;
	} else if (regAddr == 0xf4 || regAddr == 0xd2 || regAddr == 0x642 || regAddr == 0xf8f || regAddr == 0xf90) {
		/* fifo register, read will change the ptr, so can't read again*/
		ddprint = 2;
	}

	if (deviceID == 0x11) {
		//virtual dev id for mt register op
		for (i = 0; i < length; i++) {
			hdmi20_write_reg32((hdmi20_sys_reg_virt_addr + regAddr + (i << 2)), ((mt_u32 *)Data)[i]);
			if (ddprint) {
				BB_PRINTF("\n%s_%d:after write:reg[%lx]=%x\n", __func__, __LINE__, SYMPHONY_IO_PA(HDMI20_REG_BASE_ADDR + regAddr + (i << 2)), \
						  (unsigned int)hdmi20_read_reg32((hdmi20_sys_reg_virt_addr + regAddr + (i << 2))));
			}
		}
	} else {
		for (i = 0; i < length; i++) {
			hdmi20_write_reg8((hdmi20_sys_reg_virt_addr + regAddr + i), Data[i]);
			if (ddprint) {
				if (ddprint == 1) {
					BB_PRINTF("\n%s_%d:write 0x%x, after write:reg[%lx]=0x%x\n", __func__, __LINE__, (uint32_t)Data[i], SYMPHONY_IO_PA(HDMI20_REG_BASE_ADDR + regAddr + i), (unsigned int)hdmi20_read_reg8((hdmi20_sys_reg_virt_addr + regAddr + i)));
				} else {
					BB_PRINTF("\n%s_%d:write 0x%x\n", __func__, __LINE__, (uint32_t)Data[i]);
				}
			}
		}
	}
	return BB_SUCCESS;
}

byte I2C_ReadBlock_4dump (BB_HANDLE bbHandle, byte deviceID, word regAddr, byte* pData, word length)
{
	int i = 0;

	if (regAddr >= BASE_ADDRESS && regAddr < (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr -= BASE_ADDRESS;
	} else {
		return BB_SUCCESS;
	}
	if (deviceID == 0x11) {
		//virtual dev id for mt register op
		for (i = 0; i < length; i++) {
			((mt_u32 *)pData)[i] = hdmi20_read_reg32(hdmi20_sys_reg_virt_addr + regAddr + (i << 2));
		}
	} else {
		for (i = 0; i < length; i++) {
			pData[i] = hdmi20_read_reg8((hdmi20_sys_reg_virt_addr + regAddr + i));
		}
	}

	return BB_SUCCESS;
}

byte GetI2CRepeatedStartMode (BB_HANDLE bbHandle, byte* startMode)
{
	return BB_SUCCESS;
}

byte SetI2CRepeatedStartMode (BB_HANDLE bbHandle, byte startMode)
{
	return BB_SUCCESS;
}

byte GetI2CBigEndianMode (BB_HANDLE bbHandle, byte* endianMode)
{
	return BB_SUCCESS;
}

byte SetI2CBigEndianMode (BB_HANDLE bbHandle, byte endianMode)
{
	return BB_SUCCESS;
}

byte GetI2C16bitAddress (BB_HANDLE bbHandle, byte* is16bitMode)
{
	return BB_SUCCESS;
}

byte SetI2C16bitAddress (BB_HANDLE bbHandle, byte is16bitMode)
{
	return BB_SUCCESS;
}

byte GetI2CIncAddress (BB_HANDLE bbHandle, byte* incAddrMode)
{
	return BB_SUCCESS;
}

byte SetI2CIncAddress (BB_HANDLE bbHandle, byte incAddrMode)
{
	return BB_SUCCESS;
}

byte I2C_GetOptions (BB_HANDLE bbHandle, I2C_OPTIONS *pOptions)
{
	return BB_SUCCESS;
}

byte I2C_SetOptions (BB_HANDLE bbHandle, I2C_OPTIONS pOptions)
{
	return BB_SUCCESS;
}

byte GPIO_GetPins (BB_HANDLE bbHandle, byte pinMask)
{
	return BB_SUCCESS;
}

byte GPIO_SetPins (BB_HANDLE bbHandle, byte pinMask)
{
	return BB_SUCCESS;
}

byte GPIO_ClearPins (BB_HANDLE bbHandle, byte pinMask)
{
	return BB_SUCCESS;
}

byte GPIO_ModifyPins( BB_HANDLE bbHandle, byte PinMask, byte PinState)
{
	return BB_SUCCESS;
}

byte GPIO_GetOptions (BB_HANDLE bbHandle, GPIO_OPTIONS *pOptions)
{
	return BB_SUCCESS;
}

byte GPIO_SetOptions (BB_HANDLE bbHandle, GPIO_OPTIONS pOptions)
{
	return BB_SUCCESS;
}
