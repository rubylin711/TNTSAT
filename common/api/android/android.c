//
// Android stubs
//
#include "emmc_raw.h"

//to let android happy
//see: mt_flash.c
emmc_flash_s g_emmcflash = {
	.raw_areastart = 0,
	.raw_areasize  = 4294967296,	/*4G*/
	.erasesize     = 512
};

