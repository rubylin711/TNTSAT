/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_hdmi20_cfg.h"
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <errno.h>
#include <string.h>
//#include <sys/types.h>
#include <string.h>
//#include <pthread.h>
//#include <assert.h>
#include <linux/fb.h>
//#include <sys/mman.h>
//#include <sys/ioctl.h>
//#include <fcntl.h>
//#include <unistd.h>
#if (!__HDMI_UBOOT__)
	#include "mt_type.h"
#endif
#include <time.h>

//#include <dlfcn.h>

//#include <sys/select.h>
//#include <signal.h>
// Copyright ?2007, Deco, Inc.  All rights reserved.
//
// No part of this work may be reproduced, modified, distributed, transmitted,
// transcribed, or translated into any language or computer format, in any form
// or by any means without written permission of: Deco, Inc.,
// 1060 East Arques Avenue, Sunnyvale, California 94085
//------------------------------------------------------------------------------
//#define SII_DEBUG 1

/***** #include statements ***************************************************/
#include "conio.h"
#include "si_datatypes.h"
#include "si_lib_malloc_api.h"
#include "si_lib_seq_api.h"
#include "platform_api.h"
#include "si_lib_obj_api.h"
#include "si_lib_log_api.h"
#include "si_drv_cra_api.h"
#include "si_app_tx_api.h"

//#include "hdmi20.h"
#if (!__HDMI_UBOOT__)
	#include "mt_common.h"
#endif
#include "mt_aout_testcase.h"
#include "mt_hdmi20_testcase.h"
#include "misc_reg_op.h"
#include "si_drv_tx_api.h"
#include "si_drv_tx_regs.h"

#define SII_INFOFRAME_MAX_LEN 31

#define SII_INFOFRAME_AVI_MAX_LEN 17
#define SII_INFOFRAME_AUDIO_MAX_LEN 14
#define SII_INFOFRAME_VS_MAX_LEN 31
#define SYM6_FPGA_AOUT_108M (1)

static const mt_u32 srcflt_coef[256] = {
	//table_2A
	0xffffff0a, 0xfffff548, 0xffffffb4, 0x0000029e, 0xfffffade, 0x000008ba,
	0xfffff26a, 0x000013e7, 0xffffe43c, 0x00002566, 0xffffcf0e,
	0x00003ea9, 0xffffb12d, 0x000061df, 0xffff878b, 0x000093ae, 0xffff4a8a,
	0x0000e17e, 0xfffee073, 0x00018315, 0xfffdb39f, 0x000513f2,
	0x000e18a1, 0xfffd74f2, 0x00013cf0, 0xffff4647, 0x0000731b, 0xffffb8cc,
	0x000029d2, 0xffffea92, 0x00000726, 0x000002bd, 0xfffff6ab,
	0x00000d6a, 0xfffff071, 0x00001040, 0xfffff02b, 0x00000eae, 0xfffff2fb,
	0x00000b30, 0xfffff67f, 0x000008aa, 0xfffff588, 0xfffffafa,
	//table_2B
	0x00000562, 0x0000550b, 0xfffe58eb, 0x00058622, 0x000c9794, 0xffff5694,
	0xffffb935, 0x00002af1,
	//table_3A
	0xffffff78, 0xfffff6cc, 0xfffffede, 0x000005fb, 0xfffff9d5, 0x00000c53,
	0xfffff0cf, 0x0000179c, 0xffffe2a8, 0x0000285d, 0xffffcec4,
	0x00003f42, 0xffffb44f, 0x00005d63, 0xffff91a8, 0x00008567, 0xffff6230,
	0x0000bfc8, 0xffff1548, 0x000130c3, 0xfffe4b1e, 0x0003680a,
	0x000ee8da, 0xfffe3617, 0x0000c598, 0xffff9d44, 0x0000317f, 0xffffee1c,
	0x00000025, 0x00000da8, 0xffffec21, 0x000019db, 0xffffe5bf,
	0x00001c27, 0xffffe6ad, 0x00001924, 0xffffeb60, 0x000013b2, 0xfffff171,
	0x00000de8, 0xfffff6ec, 0x00000a2a, 0xfffff7c9, 0xfffffa7b,
	0xfffffdb7, 0xfffff51a, 0x000006cb, 0xfffffd10, 0x00000489, 0xffffff32,
	0x0000007b, 0x0000055b, 0xfffff727, 0x00001219, 0xffffe5e7,
	0x0000286a, 0xffffc924, 0x00004cd2, 0xffff9ac6, 0x00008824, 0xffff4d00,
	0x0000f1f6, 0xfffeb2dd, 0x0001ee02, 0xfffcac9b, 0x000a2a83,
	0x000a2a83, 0xfffcac9b, 0x0001ee02, 0xfffeb2dd, 0x0000f1f6, 0xffff4d00,
	0x00008824, 0xffff9ac6, 0x00004cd2, 0xffffc924, 0x0000286a,
	0xffffe5e7, 0x00001219, 0xfffff727, 0x0000055b, 0x0000007b, 0xffffff32,
	0x00000489, 0xfffffd10, 0x000006cb, 0xfffff51a, 0xfffffdb7,
	//table_3B
	0xffffff12, 0xffffec99, 0x00002a60, 0xffffcbb0, 0x000019a0, 0x00006ba9,
	0xfffe07d3, 0x000c3e47, 0x000721e5, 0xfffd5c3f,
	0x00013893, 0xffff8531, 0x00001fc2, 0x000006a1, 0xfffff3c7, 0xfffffb4a,
	0xffffefe7, 0x00003e56, 0xffff7a0b, 0x0000e902,
	0xfffeb6ec, 0x00019578, 0x000e5137, 0x00019578, 0xfffeb6ec, 0x0000e902,
	0xffff7a0b, 0x00003e56, 0xffffefe7, 0xfffffb4a,
	0x0000017e, 0x000050ce, 0xfffeae4e, 0x00037fef, 0x000d900b, 0x000052b8,
	0xffff6fce, 0x000035d6,//table_4A
	0x00000855, 0x00004d4c, 0xfffe3116, 0x000785da, 0x000b48b4, 0xfffe92aa,
	0x00000759, 0x00001994,
	0xfffffb77, 0xffffb68c, 0x000a3582, 0x00069ff0, 0xffff8a6e, //table_4B
	0xffffe685, 0x0000c89d, 0x000c82d3, 0x0003277b, 0xffffb86b,
	0x00000310, 0x00036ad3, 0x000abbe4, 0x0001d7a4, //table_5A
	0x00001473, 0x0005792d, 0x00099e4f, 0x0000d590,
	0x00004d6d, 0x0007b34d, 0x0007b34d, 0x00004d6d,
	0xfffff68f, 0x00000000, 0x0000071b, 0x00000000, 0xfffff64a, 0x00000000,
	//table_2D
	0x00000cea, 0x00000000, 0xffffef35, 0x00000000, 0x0000157a, 0x00000000,
	0xffffe4e5, 0x00000000, 0x000021e2, 0x00000000, 0xffffd5ef, 0x00000000,
	0x00003413, 0x00000000, 0xffffbf83, 0x00000000, 0x0000504d, 0x00000000,
	0xffff9ace, 0x00000000, 0x00008267, 0x00000000, 0xffff512e, 0x00000000,
	0x0000fc7e, 0x00000000, 0xfffe5267, 0x00000000, 0x00051620, 0x00080000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

static mt_u32 release_aud_mem(mt_u8 *ptr);
mt_u8 ic_verify_samplerate2index_tt(mt_u32 sample);
mt_u8 ic_verify_chan_mode_tt(mt_u32 chan);

/***** local functions *******************************************************/
static mt_u32 get_file_name(char* filename, AUD_INPUT_T* aud_params)
{
	char key;
	mt_u32 mode;
	mt_u32 fs;
	mt_u32 channel;
	mt_u32 downmix;
	mt_u32 is_hbr;

	if (filename == NULL) {
		return BB_FAILURE;
	}
INPUT_AUDIO_MODE:
	SSHH_PRINTF("\n%s_%d: Input audio mode, '0': I2S, '1':SPDIF(DD..),  '2':SPDIF(pcm) Not support '3':SPDIF(DD+) '4':DTS\n", __func__, __LINE__);
	#if HDMI20_AUD_HBR
	SSHH_PRINTF("\n%s_%d: Input audio mode, '5': I2S HBR(NOT Support), '6':SPDIF(DD..) HBR(NOT Support),	'7':SPDIF(pcm) Not support '8':SPDIF(DD+) HBR(NOT Support) '9':DTS HBR\n", __func__, __LINE__);
	#endif
	key = (char)_getch();
	mode = (mt_u32)key - '0';
	#if HDMI20_AUD_HBR
	if (mode > 9 || mode == 2 || mode == 7) {
		goto INPUT_AUDIO_MODE;
	}
	is_hbr = (mode > 4);
	if (is_hbr) {
		//strcpy(filename, "hdmi_eac_048k_8ch.raw");
		filename[18] = 'h';
		filename[19] = 'b';
		filename[20] = 'r';
		mode -= 5;
	}
	#else
	if (mode > 4 || mode == 2) {
		goto INPUT_AUDIO_MODE;
	}
	#endif
	aud_params->mode_ex = 0;
	switch (mode) {
		case 0:
			filename[5] = 'p';
			filename[6] = 'c';
			filename[7] = 'm';
			break;
		case 1:
			filename[5] = 's';
			filename[6] = 'p';
			filename[7] = 'd';
			break;
		case 3:
			if (3 == mode) {
				aud_params->mode_ex = 1;
			}
			filename[5] = 'e';
			filename[6] = 'a';
			filename[7] = 'c';
			break;
		case 2:
			filename[5] = 'p';
			filename[6] = 'c';
			filename[7] = 'm';
			break;
		case 4:
			filename[5] = 'd';
			filename[6] = 't';
			filename[7] = 's';
			break;
		default:
			break;
	}

INPUT_AUDIO_FS:
	SSHH_PRINTF("\n%s_%d: Input audio sample rate, '0': 32K, '1':48K, '2':96K, '3':192K, '4':44.1K,'5':88.2K,'6':176.4K \n", __func__, __LINE__);
	key = (char)_getch();
	fs = (mt_u32)key - '0';
	if (fs > 6) {
		goto INPUT_AUDIO_FS;
	}
	switch (fs) {
		case 0:
			filename[9] = '0';
			filename[10] = '3';
			filename[11] = '2';
			break;
		case 1:
			filename[9] = '0';
			filename[10] = '4';
			filename[11] = '8';
			break;
		case 2:
			filename[9] = '0';
			filename[10] = '9';
			filename[11] = '6';
			break;
		case 3:
			filename[9] = '1';
			filename[10] = '9';
			filename[11] = '2';
			break;
		case 4:
			filename[9] = '4';
			filename[10] = '4';
			filename[11] = '1';
			break;
		case 5:
			filename[9] = '8';
			filename[10] = '8';
			filename[11] = '2';
			break;
		case 6:
			filename[9] = '1';
			filename[10] = '7';
			filename[11] = '6';
			break;
		default:
			filename[9] = '0';
			filename[10] = '4';
			filename[11] = '8';
			break;
	}

INPUT_AUDIO_CH:
	SSHH_PRINTF("\n%s_%d: Input audio channel number, '1': 1 memo, '2':2 channels, '6':5.1channels, '8':7.1channels\n", __func__, __LINE__);
	key = (char)_getch();
	channel = (mt_u32)key - '0';

	if (channel != 2 && channel != 6 && channel != 8) {
		goto INPUT_AUDIO_CH;
	}
	switch (channel) {
		case 1:
			filename[14] = '1';
			break;
		case 2:
			filename[14] = '2';
			break;
		case 6:
			filename[14] = '6';
			break;
		case 8:
			filename[14] = '8';
			break;
		default:
			filename[14] = '2';
			break;
	}

	#if HDMI20_RELEASE_2_QA
	downmix = 0;
	#else
	SSHH_PRINTF("\n%s_%d: Input audio downmix, '0': normal, '1':downmix enable\n", __func__, __LINE__);
	key = (char)_getch();
	downmix = (mt_u32)key - '0';
	#endif

	SSHH_PRINTF("\n%s_%d: mode:%d, fs:%d,ch:%d,mix:%d,hbr:%d\n", __func__, __LINE__, mode, fs, channel, downmix, is_hbr);
	SSHH_PRINTF("\n%s_%d: open audio file:%s\n", __func__, __LINE__, filename);
	aud_params->mode = mode;
	aud_params->fs = fs;
	aud_params->ch = channel;
	aud_params->downmix = downmix;
	aud_params->is_hbr = is_hbr;

	return BB_SUCCESS;
}

static mt_mmz_buf_s  g_aout_aud_buff = {0};
static mt_u32 get_aud_mem(mt_u8 **ptr, mt_u32 size)
{
	mt_s32 ret;
	ret = (mt_s32)release_aud_mem(NULL);
	g_aout_aud_buff.bufsize = size + 16;//make sure 8 align is safe
	ret = mt_mmz_malloc(&g_aout_aud_buff);
	if ( ret ) {
		SSHH_PRINTF( "\n Malloc Audbuf fail: 0x%x\n", (mt_u32)g_aout_aud_buff.bufsize);
		return BB_FAILURE;
	} else {
		if (g_aout_aud_buff.phyaddr & 0x7) {
			*ptr = g_aout_aud_buff.user_viraddr + (8 - (g_aout_aud_buff.phyaddr & 0x7));
		} else {
			*ptr = g_aout_aud_buff.user_viraddr;
		}
		SSHH_PRINTF( "\n aud data buffer ptr: 0x%lu, 0x%x\n", (ulong)*ptr, (mt_u32)g_aout_aud_buff.bufsize);
	}
	if (*ptr) {
		return BB_SUCCESS;
	} else {
		*ptr = NULL;
		return BB_FAILURE;
	}
}

static mt_u32 release_aud_mem(mt_u8 *ptr)
{
	mt_s32 ret;
	if (g_aout_aud_buff.user_viraddr) {
		ret = mt_mmz_free(&g_aout_aud_buff);
		if (ret != BB_SUCCESS) {
			SSHH_PRINTF( "\n free buf ptr fail: 0x%lx!!!\n", (ulong) g_aout_aud_buff.user_viraddr);
		}
		memset(&g_aout_aud_buff, 0, sizeof(mt_mmz_buf_s));
	} else {
		SSHH_PRINTF( "\n free error buf ptr: 0x%lx, %lx\n", (ulong)ptr, (ulong)g_aout_aud_buff.user_viraddr);
	}
	return BB_SUCCESS;
}

static mt_u32 preload_aud_data(mt_u32 *BufAddr, mt_u8 *data,  mt_u32 dsize)
{
	//data = data;
	//dsize = dsize;
	*BufAddr = (mt_u32)g_aout_aud_buff.phyaddr;//

	return BB_SUCCESS;
}

static mt_u32 enable_aout_clk(mt_u32 en)
{
	mt_u32 ret = 0;
	ulong reg_addr = 0x1f50a500;
	mt_u32 reg_val = 0;

	if (en) {
		reg_val = 0x03;    //enable aout clk
	} else {
		reg_val = 0x00;    //disable aout clk, need to confirm what should be set
	}
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	return ret;
}

static mt_u32 config_src_coef(void)
{
	mt_u32 i;
	mt_u32 ret = 0;
	ulong reg_addr = 0x1f49004c;
	mt_u32 reg_val = 0;

	//reg_val = 0x03;  //enable aout clk
	//reg_addr = 0x1f50a500;
	//ret = misc_reg_write32(reg_addr, &reg_val, 1);

	reg_addr = 0x1f49004c;
	for (i = 0; i < 256; i++) {
		reg_val = srcflt_coef[i] & 0x001fffff; //low 21bits
		reg_val |= i << 21;		 //wr_coef_addr
		reg_val |= 0x80000000;  //wr_coef_en
		ret = misc_reg_write32(reg_addr, &reg_val, 1);
	}
	return ret;
}

static mt_u32 aud_hdmi_get_fs( AUD_INPUT_T params )
{
	mt_u32 ret = 0;
	if ( params.mode == 3 || params.mode_ex == 1 ) {//EAC3 || HBR
		switch ( params.fs) { //fs * 4
			case 0://32K
				ret = 8;//128K
				break;
			case 1://48K
				ret = 3;//192K
				break;
			case 4://44.1K
				ret = 6;//176.4K
				break;
			default:
				ret = params.fs;
				break;
		}
	} else {
		ret = params.fs;
	}

	//SSHH_PRINTF("\nacr: infs:0x%x,outfs:0x%x,mode:%d,mode_ex:%d\n",params.fs,ret,params.mode,params.mode);
	return ret;
}

static mt_void aud_hdmi_acr_cfg(AUD_INPUT_T params)
{
	AVINFOSTORAGE_T param;
	mt_u32 ret = BB_SUCCESS;
	mt_u32 acr_n = 6144;
	mt_u32 tmds_cr = 0;

	ret = get_hdmi_storage_params(&param);
	switch (param.tvsys) {
		case 0:
		case 2:
		case 4:
		case 5:
		case 7:
		case 9:
		case 27:
		case 29:
		case 31:
			tmds_cr = 297000;
			break;
		case 1:
		case 3:
		case 6:
		case 8:
		case 28:
		case 30:
			tmds_cr = 594000;
			break;
		case 10:
		case 11:
		case 36:
		case 37:
			tmds_cr = 148500;
			break;
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 32:
		case 33:
		case 34:
		case 35:
			tmds_cr = 74250;
			break;
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
			tmds_cr = 27000;
			break;
		default:
			tmds_cr = 74250;
			break;
	}
	params.fs = aud_hdmi_get_fs(params);
	switch ( params.fs ) {
		case 0://32K
			switch (tmds_cr) {
				case 27000:
					acr_n = 4096;//cts=27000
					break;
				case 74250:
					acr_n = 4096;//cts=74250
					break;
				case 148500:
					acr_n = 4096;//cts=148500
					break;
				case 297000:
					acr_n = 3072;//cts=222750
					break;
				case 594000:
					acr_n = 3072;//cts=445500
					break;
				default:
					acr_n = 4096;//cts=74250
					break;
			}
			break;
		case 1://48K
			switch (tmds_cr) {
				case 27000:
					acr_n = 6144;//cts=27000
					break;
				case 74250:
					acr_n = 6144;//cts=74250
					break;
				case 148500:
					acr_n = 6144;//cts=148500
					break;
				case 297000:
					acr_n = 5120;//cts=247500
					break;
				case 594000:
					acr_n = 6144;//cts=594000
					break;
				default:
					acr_n = 6144;//cts=74250
					break;
			}
			break;
		case 2://96K
			switch (tmds_cr) {
				case 27000:
					acr_n = 12288;//cts=27000
					break;
				case 74250:
					acr_n = 12288;//cts=74250
					break;
				case 148500:
					acr_n = 12288;//cts=148500
					break;
				case 297000:
					acr_n = 10240;//cts=247500
					break;
				case 594000:
					acr_n = 12288;//cts=594000
					break;
				default:
					acr_n = 12288;//cts=74250
					break;
			}
			break;
		case 3://192K
			switch (tmds_cr) {
				case 27000:
					acr_n = 24576;//cts=27000
					break;
				case 74250:
					acr_n = 24576;//cts=74250
					break;
				case 148500:
					acr_n = 24576;//cts=148500
					break;
				case 297000:
					acr_n = 20480;//cts=247500
					break;
				case 594000:
					acr_n = 24576;//cts=594000
					break;
				default:
					acr_n = 24576;//cts=74250
					break;
			}
			break;
		case 4://44.1K
			switch (tmds_cr) {
				case 27000:
					acr_n = 6272;//cts=27000
					break;
				case 74250:
					acr_n = 6272;//cts=74250
					break;
				case 148500:
					acr_n = 6272;//cts=148500
					break;
				case 297000:
					acr_n = 4704;//cts=247500
					break;
				case 594000:
					acr_n = 9408;//cts=990000
					break;
				default:
					acr_n = 6272;//cts=74250
					break;
			}
			break;
		case 5://176.4K
			switch (tmds_cr) {
				case 27000:
					acr_n = 12544;//cts=27000
					break;
				case 74250:
					acr_n = 12544;//cts=74250
					break;
				case 148500:
					acr_n = 12544;//cts=148500
					break;
				case 297000:
					acr_n = 9408;//cts=247500
					break;
				case 594000:
					acr_n = 18816;//cts=990000
					break;
				default:
					acr_n = 12544;//cts=74250
					break;
			}
			break;
		case 6://192K
			switch (tmds_cr) {
				case 27000:
					acr_n = 25088;//cts=27000
					break;
				case 74250:
					acr_n = 25088;//cts=74250
					break;
				case 148500:
					acr_n = 25088;//cts=148500
					break;
				case 297000:
					acr_n = 18816;//cts=247500
					break;
				case 594000:
					acr_n = 37632;//cts=990000
					break;
				default:
					acr_n = 25088;//cts=74250
					break;
			}
			break;
		case 7://64K
			switch (tmds_cr) {
				case 27000:
					acr_n = 8192;//cts=27000
					break;
				case 74250:
					acr_n = 8192;//cts=74250
					break;
				case 148500:
					acr_n = 8192;//cts=148500
					break;
				case 297000:
					acr_n = 8192;//cts=297000
					break;
				case 594000:
					acr_n = 8192;//cts=594000
					break;
				default:
					acr_n = 8192;//cts=74250
					break;
			}
			break;
		case 8://128K
			switch (tmds_cr) {
				case 27000:
					acr_n = 16384;//cts=27000
					break;
				case 74250:
					acr_n = 16384;//cts=74250
					break;
				case 148500:
					acr_n = 16384;//cts=148500
					break;
				case 297000:
					acr_n = 16384;//cts=297000
					break;
				case 594000:
					acr_n = 16384;//cts=594000
					break;
				default:
					acr_n = 16384;//cts=74250
					break;
			}
			break;
		default:
			break;
	}
	SSHH_PRINTF("\nacr: tvsys=%d,fs=%d,N=0x%x\n", param.tvsys, params.fs, acr_n);
	ret = misc_reg_put8(REG_ADDR__N_SVAL1, 0xff, (mt_u8)(acr_n & 0xff));
	ret = misc_reg_put8(REG_ADDR__N_SVAL2, 0xff, (mt_u8)((acr_n >> 8) & 0xff));
	ret = misc_reg_put8(REG_ADDR__N_SVAL3, 0xff, (mt_u8)((acr_n >> 16) & 0x0f));
	if ( ret ) {
		return ;
	}
}

static void hdmiAudioMuteEnable(uint8_t en)
{
	mt_u32 ret = 0;
	ulong reg_addr = REG_ADDR__AUDP_TXCTRL;
	mt_u8 reg_val = 0x0;

	ret = misc_reg_read8(reg_addr, &reg_val, 1);
	reg_val &= ~(1 << 7);
	reg_val |= (en << 7) ;
	ret = misc_reg_put8(reg_addr, 0xff, reg_val);  //mute/unmute audio fifo data

	reg_addr = REG_ADDR__TPI_AUD_CONFIG;
	ret = misc_reg_read8(reg_addr, &reg_val, 1);
	reg_val &= ~(1 << 4);
	reg_val |= (en << 4) ;
	ret = misc_reg_put8(reg_addr, 0xff, reg_val);  //mute/unmute audio fifo data
	if ( ret ) {
		return ;
	}
}

#define IC_VERIFY_PRT_ENABLE
#ifdef IC_VERIFY_PRT_ENABLE
	#define IC_VERIFY_PRT(fmt...) printf(fmt)
#else
	#define IC_VERIFY_PRT(fmt...) do{}while(0)
#endif

#define AUD_SAMPLE_RATE_2_33  	(8589934592ULL)  //32809 = 2^33

#if (defined(CONFIG_MT_CHIP_ETUDE2))

	#if  (!defined(CONFIG_MT_FPGA))
		#define AUD_SAMPLE_RATE_MCLK  	(261818181ULL)  //261818181
	#else
		#define AUD_SAMPLE_RATE_MCLK  	(54000000ULL)  //261818181
	#endif

#elif (defined(CONFIG_MT_CHIP_SYMPHONY6))

	#if  (!defined(CONFIG_MT_FPGA))
		#define AUD_SAMPLE_RATE_MCLK  	(261818181ULL)  //261818181
	#else
		#if SYM6_FPGA_AOUT_108M
			#define AUD_SAMPLE_RATE_MCLK  	(108000000ULL)  //261818181
		#else
			#define AUD_SAMPLE_RATE_MCLK  	(50000000ULL)  //261818181
		#endif
	#endif

#else

	#if  (!defined(CONFIG_MT_FPGA))
		#define AUD_SAMPLE_RATE_MCLK  	(261818181ULL)  //261818181
	#else
		#define AUD_SAMPLE_RATE_MCLK  	(54000000ULL)  //261818181
	#endif

#endif

#define REG_BASE_ADDR_AUDIO 0xbf490000UL
#define REG_PA_READ(addr,val) mt_sys_read_register(SYMPHONY_IO_PA(addr), val)
#define REG_PA_WRITE(addr,val) mt_sys_write_register(SYMPHONY_IO_PA(addr), val)

enum {
	AUDIO_TYPE_OTHER = 0,
	AUDIO_TYPE_AC3,
	AUDIO_TYPE_EAC3,
	AUDIO_TYPE_DTS,
	AUDIO_TYPE_DTS_HBR,
	AUDIO_TYPE_INVALID = 0xff
};

mt_u8 ic_verify_samplerate2index_tt(mt_u32 sample)
{
	mt_u8 index = 0;

	if (sample <= 8000) {
		index = 0xa;
	} else if (sample <= 11025) {
		index = 0x8;
	} else if (sample <= 12000) {
		index = 0x9;
	} else if (sample <= 16000) {
		index = 0x2;
	} else if (sample <= 22050) {
		index = 0x0;
	} else if (sample <= 24000) {
		index = 0x1;
	} else if (sample <= 32000) {
		index = 0x6;
	} else if (sample <= 44100) {
		index = 0x4;
	} else if (sample <= 48000) {
		index = 0x5;
	} else if (sample <= 64000) {
		index = 0xe;
	} else if (sample <= 88200) {
		index = 0xc;
	} else if (sample <= 96000) {
		index = 0xd;
	} else if (sample <= 128000) {
		index = 0xf;
	} else if (sample <= 176400) {
		index = 0x3;
	} else if (sample <= 192000) {
		index = 0x7;
	} else {
		index = 0x5;
	}

	return index;
}

mt_u8 ic_verify_chan_mode_tt(mt_u32 chan)
{
	mt_u8 mode = 0;
	mt_u8 idx = chan;

	if (8 < idx) {
		idx = 8;
	}

	while (idx) {
		mode = (mode << 1) | 1;
		idx--;
	}

	return mode;
}

static void ic_verify_aout_cfg_tt(mt_u32 buffAddr, mt_u32 buffSize, mt_u32 sampleRate, mt_u32 chan, mt_u8 type)
{
	mt_s32 ret = 0;
	mt_u32 reg_val = 0;
	mt_u8 chan_mode = 0;
	mt_u8 samp_rate_idx = 0;
	mt_u32 clk_div = 0;
	mt_u64 clk_div64 = 0LL;

	if (AUDIO_TYPE_EAC3 == type || AUDIO_TYPE_DTS_HBR == type) {
		sampleRate <<= 2;
	}
	samp_rate_idx = ic_verify_samplerate2index_tt(sampleRate);
	chan_mode = ic_verify_chan_mode_tt(chan);
	clk_div64 = AUD_SAMPLE_RATE_2_33 * (mt_u64)sampleRate / AUD_SAMPLE_RATE_MCLK;
	clk_div = (mt_u32)clk_div64;
	IC_VERIFY_PRT("AOUT-Cfg[%d/%x/%x]\n", samp_rate_idx, chan_mode, clk_div);

	//data preload stop
	ret |= REG_PA_READ(REG_BASE_ADDR_AUDIO + 0x84, &reg_val);
	ret |= REG_PA_WRITE(REG_BASE_ADDR_AUDIO + 0x84, reg_val & (~0x39)); //bit:[0]pp,[3]pcm,[4]spd,[5]mix

	//aout clock
	ret |= REG_PA_READ(0xbf50a504, &reg_val); //aout sys clock 288MHz
	ret |= REG_PA_WRITE(0xbf50a504, (reg_val & (~0x3)) | 0x1);
	ret |= REG_PA_READ(0xbf50a500, &reg_val); //enable aout clock
	ret |= REG_PA_WRITE(0xbf50a500, (reg_val & (~0x3)) | 0x3);

	//aout buffer
	ret |= REG_PA_WRITE(REG_BASE_ADDR_AUDIO + 0x18, buffAddr / 8); //buffer base
	ret |= REG_PA_WRITE(REG_BASE_ADDR_AUDIO + 0x1c, buffSize / 8); //buffer length
	ret |= REG_PA_WRITE(REG_BASE_ADDR_AUDIO + 0x20, buffSize / 8 + 16); //aout buff full threshold, never full for ic verify

	//clk div
	ret |= REG_PA_WRITE(REG_BASE_ADDR_AUDIO + 0x10, clk_div); //div1 for pcm and mix buffer

	//volume
	ret |= REG_PA_WRITE(REG_BASE_ADDR_AUDIO + 0x8, 0x8000);

	//i2s-spdif-configs
	ret |= REG_PA_READ(REG_BASE_ADDR_AUDIO + 0x4, &reg_val);
	if (AUDIO_TYPE_EAC3 == type || AUDIO_TYPE_AC3 == type || AUDIO_TYPE_DTS == type) {
		reg_val = (reg_val & (~(0xf << 8))) | (0x1 << 8);
	} else {
		reg_val &= (~(0xf << 8));
	}

	reg_val &= (~(0x3 << 1)); //I2S, always aout pcm buffer
	reg_val |= (0x2 << 1); //I2S, always aout pcm buffer
	ret |= REG_PA_WRITE(REG_BASE_ADDR_AUDIO + 0x4, reg_val);

	//ch-srt-configs
	ret |= REG_PA_READ(REG_BASE_ADDR_AUDIO + 0x0, &reg_val);
	reg_val = (reg_val & (~0xf)) | (mt_u32)samp_rate_idx; //sample rate
	reg_val = (reg_val & (~(0x1 << 4))); //pcm 32bit flag
	reg_val = (reg_val & (~(0xff << 8))) | ((mt_u32)chan_mode << 8); //ch mode
	ret |= REG_PA_WRITE(REG_BASE_ADDR_AUDIO + 0x0, reg_val);

	//pp-en-configs
	ret |= REG_PA_READ(REG_BASE_ADDR_AUDIO + 0xc, &reg_val);
	reg_val &= (~(0x1 << 3));
	if (2 == chan) {
		reg_val |= (0x1 << 3);
	}
	ret |= REG_PA_WRITE(REG_BASE_ADDR_AUDIO + 0xc, reg_val);

	//data preload start
	ret |= REG_PA_READ(REG_BASE_ADDR_AUDIO + 0x84, &reg_val);
	ret |= REG_PA_WRITE(REG_BASE_ADDR_AUDIO + 0x84, (reg_val & (~0x8)) | 0x8); //bit:[0]pp,[3]pcm,[4]spd,[5]mix

	if (ret) {
		IC_VERIFY_PRT("%s:L%d:implement error!\n", __func__, __LINE__);
	}
}

static mt_u32 aout_play(AUD_INPUT_T params, mt_u32 BufAddr, mt_u32 BufLen)
{
	mt_u32 sample_rate = 0;
	mt_u8 type;
	SiiAudioFormat_t AudioFormat;

	memset(&AudioFormat, 0, sizeof(SiiAudioFormat_t));
	switch (params.fs) {
		case 0:
			sample_rate = 32000;
			break;
		case 1:
			sample_rate = 48000;
			break;
		case 2:
			sample_rate = 96000;
			break;
		case 3:
			sample_rate = 192000;
			break;
		case 4:
			sample_rate = 44100;
			break;
		case 5:
			sample_rate = 88200;
			break;
		case 6:
			sample_rate = 176400;
			break;
		default:
			sample_rate = 48000;
			break;
	}

	switch (params.mode) {
		case 0:
			type = (mt_u8)AUDIO_TYPE_OTHER;
			break;
		case 1:
			type = (mt_u8)AUDIO_TYPE_AC3;
			break;
		case 2:
			type = (mt_u8)AUDIO_TYPE_OTHER;
			break;
		case 3:
			type = (mt_u8)AUDIO_TYPE_EAC3;
			break;
		case 4:
			type = (mt_u8)AUDIO_TYPE_DTS;
			break;
		default:
			type = (mt_u8)AUDIO_TYPE_OTHER;
			break;
	}
	if ( params.is_hbr ) {
		type = AUDIO_TYPE_DTS_HBR;
	}
	ic_verify_aout_cfg_tt((mt_u32)BufAddr, BufLen, sample_rate, params.ch, type);

	switch (params.fs) {
		case 0:
			sample_rate = SII_AUDIO_FS__32KHZ;
			if ( type == (mt_u8)AUDIO_TYPE_EAC3 && params.is_hbr == 0 ) {
				sample_rate = SII_AUDIO_FS__128KHZ;
			}
			break;
		case 1:
			sample_rate = SII_AUDIO_FS__48KHZ;
			if ( type == (mt_u8)AUDIO_TYPE_EAC3 && params.is_hbr == 0 ) {
				sample_rate = SII_AUDIO_FS__192KHZ;
			}
			break;
		case 2:
			sample_rate = SII_AUDIO_FS__96KHZ;
			break;
		case 3:
			sample_rate = SII_AUDIO_FS__192KHZ;
			break;
		case 4:
			sample_rate = SII_AUDIO_FS__44_1KHZ;
			if ( type == (mt_u8)AUDIO_TYPE_EAC3 && params.is_hbr == 0 ) {
				sample_rate = SII_AUDIO_FS__176_4KHZ;
			}
			break;
		case 5:
			sample_rate = SII_AUDIO_FS__88_2KHZ;
			break;
		case 6:
			sample_rate = SII_AUDIO_FS__176_4KHZ;
			break;
		case 7:
			sample_rate = SII_AUDIO_FS__64KHZ;
			break;
		case 8:
			sample_rate = SII_AUDIO_FS__128KHZ;
			break;
		default:
			sample_rate = SII_AUDIO_FS__48KHZ;
			break;
	}
	AudioFormat.audioFs = (SiiAudioFs_t)sample_rate;
	AudioFormat.layout1 = (uint8_t)params.ch;
	AudioFormat.dsd = (uint8_t)0;
	AudioFormat.hbrA = (uint8_t)params.is_hbr;
	if (AudioFormat.hbrA) {
		AudioFormat.i2s = 1;
		AudioFormat.spdif = 0;
	} else {
		AudioFormat.i2s = (uint8_t)(params.mode == 0);
		AudioFormat.spdif = (uint8_t)(params.mode == 1 || params.mode == 3 || params.mode == 4);
	}
	aud_hdmi_acr_cfg(params);
	SiiDrvTxAudioFormatSet(getTxInstance(), &AudioFormat);
	return BB_SUCCESS;
}

static mt_void hdmi_set_aud_infoframe(mt_u8 ch, info_struct_t *info)
{
	mt_u8 cksum = 0;
	mt_u8 i;

	memset(info, 0, sizeof(info_struct_t));
	info->type = 0x84;
	info->version = 0x01;
	info->length = 0x0A;

	if (ch > 0) {
		info->pb_byte[1] = ch - 1;
	} else {
		info->pb_byte[1] = 0;
	}

	if (ch == 6) {
		info->pb_byte[4] = 0x0b;
	} else if (ch == 8) {
		info->pb_byte[4] = 0x13;
	} else {
		info->pb_byte[4] = 0;
	}

	cksum += info->type;
	cksum += info->version;
	cksum += info->length;
	for (i = 1; i < info->length; i++) {
		cksum += info->pb_byte[i];
	}
	cksum = 256 - cksum;
	info->pb_byte[0] = cksum;
}

mt_void play_test_audio(void)
{
	mt_u32 u32Ret = 0;
	AUD_INPUT_T aud_param = {0};
	const char filename[30] = "hdmi_pcm_048k_2ch.raw";//
	FILE *fin;
	mt_u32 aud_buf = 0x3200000;
	mt_u8 *data_in_ptr = NULL;
	mt_u32 data_in_size = 0;
	mt_u32 file_len = 0;
	mt_u32 len = 1 << 18;
	AVINFOSTORAGE_T param;
	info_struct_t info;
	SiiInfoFrame_t info_st = {0};

	u32Ret = get_hdmi_storage_params(&param);
	u32Ret = get_file_name(filename, &aud_param);

	SSHH_PRINTF( "\r\n open file:");
	SSHH_PRINTF( "%s\n", filename);
	fopen_aud(&fin, filename, "rb");
	if (!fin) {
		SSHH_PRINTF( "\r\n xx fail to open file:%s!!!\n", filename);
		param.aud_param.audio_ch = aud_param.ch;
		param.aud_param.audio_mode = aud_param.mode;
		param.aud_param.audio_sr = aud_param.fs;
		param.aud_param.audio_downmix = aud_param.downmix;
		param.aud_param.audio_mode_ex = aud_param.mode_ex;
		param.aud_param.audio_ofile = 0xff;
		param.aud_param.is_hbr = aud_param.is_hbr;
		set_hdmi_storage_params(param);
		u32Ret = release_aud_mem(NULL);
		return;
	} else {
		SSHH_PRINTF( "\r\n success to open file:%s!!!\n", filename);
	}

	hdmiAudioMuteEnable(1); //mute audio first and unmute it in tx_hdmi
	fseek(fin, 0, SEEK_END);
	file_len = (mt_u32)ftell(fin);
	fseek(fin, 0, SEEK_SET);
	u32Ret = get_aud_mem(&data_in_ptr, AUD_DATA_PLAY_SIZE);
	if (data_in_ptr == NULL) {
		SSHH_PRINTF( "\r\n alloc aud data buffer fail:0x%x!!!", AUD_DATA_PLAY_SIZE);
	}
	{
		len = 0x3c000;//1<<18;
		while (data_in_size < file_len && data_in_size < AUD_DATA_PLAY_SIZE) {
			if ((data_in_size + len) > file_len) {
				fread(data_in_ptr + data_in_size, file_len - data_in_size, 1, fin);
				data_in_size += file_len - data_in_size;
			} else {
				fread(data_in_ptr + data_in_size, len, 1, fin);
				data_in_size += len;
			}
		}
	}
	fclose(fin);
	u32Ret = enable_aout_clk(1);
	u32Ret = config_src_coef();
	u32Ret = preload_aud_data(&aud_buf, data_in_ptr, data_in_size);

	SSHH_PRINTF( "\r\n aud raw buf:%x,size:%d,flen=%d!!!", aud_buf, data_in_size, file_len);
	for (len = 0; len < 8; len++) {
		SSHH_PRINTF( "\r\n aud raw buf:data_in_size[%d]=0x%x!!!", len, (mt_u32)data_in_ptr[len]);
	}

	param.aud_param.audio_ch = aud_param.ch;
	param.aud_param.audio_mode = aud_param.mode;
	param.aud_param.audio_sr = aud_param.fs;
	param.aud_param.audio_downmix = aud_param.downmix;
	param.aud_param.audio_mode_ex = aud_param.mode_ex;
	param.aud_param.audio_ofile = 0x0;
	param.aud_param.is_hbr = aud_param.is_hbr;
	set_hdmi_storage_params(param);

	if (aud_param.mode == 1 || aud_param.mode == 3 || aud_param.mode == 4) {
		aud_param.ch = 2;    //Treat compressed audio as 2 channels
	}
	if ( aud_param.is_hbr ) {
		aud_param.ch = 8;
		aud_param.mode = 0;
		aud_param.mode_ex = 1;
	}
	u32Ret = aout_play(aud_param, aud_buf, data_in_size);

	hdmi_set_aud_infoframe((mt_u8)aud_param.ch, &info);
	info_st.ifId = SII_INFO_FRAME_ID__AUDIO;
	SII_MEMCPY((uint8_t*)&info_st.b, (uint8_t*)(&info), SII_INFOFRAME_AVI_MAX_LEN);
	SiiDrvTxInfoframeSet(getTxInstance(), &info_st);
	SiiLibTimeMilliDelay(300);//played for 300ms
	hdmiAudioMuteEnable(0);
	if ( u32Ret ) {
		return ;
	}
}

mt_u32 hdmi_get_test_aud_ch(mt_void)
{
	AVINFOSTORAGE_T param;
	mt_u32 ret = BB_SUCCESS;

	ret = get_hdmi_storage_params(&param);
	if ( ret == BB_SUCCESS ) {
		return (param.aud_param.audio_ch);
	} else {
		return 0xff;
	}
}

mt_u32 hdmi_get_test_aud_sr(mt_void)
{
	AVINFOSTORAGE_T param;
	mt_u32 ret = BB_SUCCESS;

	ret = get_hdmi_storage_params(&param);
	if ( ret == BB_SUCCESS ) {
		return (param.aud_param.audio_sr);
	} else {
		return 0xff;
	}
}

mt_u32 hdmi_get_test_aud_mode(mt_void)
{
	AVINFOSTORAGE_T param;
	mt_u32 ret = BB_SUCCESS;

	ret = get_hdmi_storage_params(&param);
	if ( ret == BB_SUCCESS ) {
		return (param.aud_param.audio_mode);
	} else {
		return 0xff;
	}
}

mt_u32 hdmi_get_test_hbr_mode(mt_void)
{
	AVINFOSTORAGE_T param;
	mt_u32 ret = BB_SUCCESS;

	ret = get_hdmi_storage_params(&param);
	if ( ret == BB_SUCCESS ) {
		return (param.aud_param.is_hbr);
	} else {
		return 0xff;
	}
}

mt_u32 hdmi_get_test_aud_open_file_status(mt_void)
{
	AVINFOSTORAGE_T param;
	mt_u32 ret = BB_SUCCESS;

	ret = get_hdmi_storage_params(&param);
	if ( ret == BB_SUCCESS ) {
		return (param.aud_param.audio_ofile);
	} else {
		return 0xff;
	}
}

/***** end of file ***********************************************************/

