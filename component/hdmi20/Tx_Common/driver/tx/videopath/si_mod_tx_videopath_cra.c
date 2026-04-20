/*************************************************************************
* si_drv_videopath_cra.c
*************************************************************************/

//#include "si_drv_videopath_cra.h"
#include "si_vidpath_regs.h"

void SiiModTxVideoPathRegRead(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t *ptrdata, size_t rSize)
{
	uint8_t i;
	uint8_t tmp;

	tmp = (uint8_t)rSize;
	for (i = 0; i < tmp; i++) {
		*(ptrdata + i) = SiiDrvCraRdReg8(inst, addr + i);
	}
}

// This function is implemeted as per the limitation of Video path register access
// Any change in the register takes effet only after the last byte is written
void SiiModTxVideoPathRegWrite(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t *ptrdata, size_t wSize)
{
	uint8_t i;
	uint8_t tmp;

	tmp = (uint8_t)wSize;
	for (i = 0; i < tmp; i++) {
		SiiDrvCraWrReg8(inst, addr + i, *(ptrdata + i));
	}
}
