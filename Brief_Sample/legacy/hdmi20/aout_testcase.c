/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "mt_type.h"
#include <time.h>

#include <dlfcn.h>

#include <sys/select.h>
#include <signal.h>
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

#include "hdmi20.h"
#include "aout_testcase.h"
#include "misc_reg_op.h"
#include "si_drv_tx_regs.h"

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
mt_u32 do_case_iis_audio_default(void);

/***** local functions *******************************************************/
static mt_u32 get_file_name(char* filename, AUD_INPUT_T* aud_params)
{
	char key;
	mt_u32 mode;
	mt_u32 fs;
	mt_u32 channel;
	mt_u32 downmix;
	//char filename[30] = "hdmi_pcm_048k_2ch.raw";//

	if (filename == NULL) {
		return BB_FAILURE;
	}
	//SiiDrvCraWrReg32();
	SSHH_PRINTF("\n%s_%d: Input audio mode, '0': I2S, '1':SPDIF(DD..),  '2':SPDIF(pcm) '3':SPDIF(DD+)\n", __func__, __LINE__);
	key = (char)_getch();
	mode = (mt_u32)key - '0';
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
		default:
			break;
	}

	SSHH_PRINTF("\n%s_%d: Input audio sample rate, '0': 32K, '1':48K, '2':96K, '3':192K, '4':44.1K,'5':88.2K,'6':176.4K \n", __func__, __LINE__);
	key = (char)_getch();
	fs = (mt_u32)key - '0';
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

	SSHH_PRINTF("\n%s_%d: Input audio channel number, '1': 1 memo, '2':2 channels, '6':5.1channels, '8':7.1channels\n", __func__, __LINE__);
	key = (char)_getch();
	channel = (mt_u32)key - '0';
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

	SSHH_PRINTF("\n%s_%d: Input audio downmix, '0': normal, '1':downmix enable\n", __func__, __LINE__);
	key = (char)_getch();
	downmix = (mt_u32)key - '0';

	SSHH_PRINTF("\n%s_%d: mode:%d, fs:%d,ch:%d,mix:%d\n", __func__, __LINE__, mode, fs, channel, downmix);
	SSHH_PRINTF("\n%s_%d: open audio file:%s\n", __func__, __LINE__, filename);
	aud_params->mode = mode;
	aud_params->fs = fs;
	aud_params->ch = channel;
	aud_params->downmix = downmix;

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
			SSHH_PRINTF( "\n free buf ptr fail: 0x%lx!!!\n", (ulong)g_aout_aud_buff.user_viraddr);
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

static mt_u32 aout_hdmi_infoframe_set(mt_u32 channel)
{
	mt_u32 ret = 0;
	ulong reg_addr = REG_ADDR__TPI_INFO_FSEL;
	mt_u8 reg_val = 0;
	mt_u8 aud_infoframe[31];
	mt_u8 ck_sum = 0;
	mt_u32 i;

	if (channel < 1) {
		SSHH_PRINTF( "\r\n channel number:%d error!!!", channel);
		return BB_FAILURE;
	}
	memset(aud_infoframe, 0, sizeof(aud_infoframe) / sizeof(aud_infoframe[0]));
	reg_addr = REG_ADDR__TPI_INFO_FSEL;
	reg_val = 0x2; //infoframe select [3:0] = 0x2
	ret = misc_reg_write8(reg_addr, &reg_val, 1);

	aud_infoframe[0] = 0x84; //type
	aud_infoframe[1] = 0x01; //version
	aud_infoframe[2] = 0x0a; //length
	aud_infoframe[3] = 0; //checksum
	aud_infoframe[4] = channel - 1; //[2:0] channel number - 1

	for (i = 0; i < 31; i++) {
		ck_sum += aud_infoframe[i];
	}
	aud_infoframe[3] = 256 - ck_sum; //checksum

	for (i = 0; i < 31; i++) {
		reg_addr++;
		reg_val = aud_infoframe[i];
		ret = misc_reg_write8(reg_addr, &reg_val, 1);
	}

	reg_addr = REG_ADDR__TPI_INFO_EN;//0x1f5706df
	reg_val = 0xc0; //[7:6]=0x3
	ret = misc_reg_write8(reg_addr, &reg_val, 1);

	return ret;
}

static mt_u32 aud_ch_srt_cfg(AUD_INPUT_T params, mt_u32 pcm_32b_flag)
{
	mt_u32 sr = 0;
	mt_u32 ch_input_mode = 0;
	mt_u32 ret = 0;
	ulong reg_addr = 0x1f490000;
	mt_u32 reg_val = 0;

	switch (params.fs) {
		case 0:
			sr = 6;//32K
			break;
		case 1:
			sr = 5;//48K
			break;
		case 2:
			sr = 0xd;//96K
			break;
		case 3:
			sr = 7;//192K
			break;
		case 4:
			sr = 4;//44.1K
			break;
		case 5:
			sr = 0xc;//88.2K
			break;
		case 6:
			sr = 3;//176.4K
			break;
		default:
			sr = 5; //48K
			break;
	}

	switch (params.ch) { //[7:0] rrs,rls,rs,ls,center,lfe,r,l
		case 2:
			ch_input_mode = 3;
			break;
		case 6:
			ch_input_mode = 0x3f;
			break;
		case 8:
			ch_input_mode = 0xff;
			break;
		default:
			ch_input_mode = 1;
			break;
	}

	reg_val = sr | (pcm_32b_flag << 4) | (ch_input_mode << 8);
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	return ret;
}

static mt_u32 aud_i2s_spdif_cfg(AUD_INPUT_T params)
{
	mt_u32 ret = 0;
	ulong reg_addr = 0x1f490004;
	mt_u32 reg_val = 0x305004;
	//mt_u32 spdif_path_sel  = 0;
	//mt_u32 spdif_spd_buf = 0;
	mt_u32 spdif_audout_buf = 0;
	mt_u32 i2s_ch_sel = 0;

	if (params.mode == 1) {
		spdif_audout_buf = 1;
	}
	if (params.mode == 1) {
		i2s_ch_sel = 0;
	} else {
		i2s_ch_sel = 5;
	}
	reg_val |= (spdif_audout_buf << 8) | (i2s_ch_sel << 4);
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	return ret;
}

static mt_u32 aud_hdmi_params_set(AUD_INPUT_T params)
{
	mt_u32 ret = 0;
	ulong reg_addr = 0x1f490084;
	mt_u8 reg_val = 0;
	ret = aout_hdmi_infoframe_set(params.ch);

	reg_addr = REG_ADDR__TPI_AUD_FS;
	switch (params.fs) {
		case 0://SII_AUDIO_FS__32KHZ:
			reg_val = 0x03;
			break;

		case 4://SII_AUDIO_FS__44_1KHZ:
			reg_val = 0x00;
			break;

		case 1://SII_AUDIO_FS__48KHZ:
			reg_val = 0x02;
			break;

		case 5://SII_AUDIO_FS__88_2KHZ:
			reg_val = 0x08;
			break;

		case 2://SII_AUDIO_FS__96KHZ:
			reg_val = 0x0A;
			break;

		case 6://SII_AUDIO_FS__176_4KHZ:
			reg_val = 0x0C;
			break;

		case 3://SII_AUDIO_FS__192KHZ:
			reg_val = 0x0E;
			break;
		default :
			reg_val = 0x02;
			break;
	}
	reg_val |= 0x80;//[3:0]=0x2 //0x4:22.05k, 0x6:24k, 0x3:32k, 0x0:44.1k, 0x2:48k, 0x8:88.2k, 0xa:96k, 0xc:176.4k, 0xe:192k, 0x9:768k
	ret = misc_reg_write8(reg_addr, &reg_val, 1);

	reg_addr = REG_ADDR__AUDP_TXCTRL;
	if (params.ch > 2 && params.mode == 0) {
		reg_val = 0x02;    //[1] =0:2ch,=1:>2ch
	} else {
		reg_val = 0x00;    //[1] =0:2ch,=1:>2ch
	}
	ret = misc_reg_write8(reg_addr, &reg_val, 1);

	reg_addr = REG_ADDR__AUD_MODE;
	if (params.mode == 1 || params.mode == 3) {
		reg_val = 0x02;    //[1]=1 enable spdif
	} else if (params.ch == 8 || params.ch == 7) {
		reg_val = 0xf0;    //[1]=0 disable spdif, [7:4] =f 8ch
	} else if (params.ch == 6 || params.ch == 5) {
		reg_val = 0x70;    //[1]=0 disable spdif, [7:4] =7 6ch
	} else if (params.ch == 4 || params.ch == 3) {
		reg_val = 0x30;    //[1]=0 disable spdif, [7:4] =3 4ch
	} else {
		reg_val = 0x10;    //[1]=0 disable spdif, [7:4] =1 2ch
	}
	ret = misc_reg_write8(reg_addr, &reg_val, 1);

	//downsample
	reg_addr = REG_ADDR__TPI_DOWN_SMPL_CTRL;
	if (params.downmix) {
		reg_val = 0x2;    //[1:0] 0:only 32/44.1/48 passed, 1/3:pass all, 2:donw to 32/44.1/48
	} else {
		reg_val = 0x3;    //[1:0] 0:only 32/44.1/48 passed, 1/3:pass all, 2:donw to 32/44.1/48
	}
	ret = misc_reg_write8(reg_addr, &reg_val, 1);

	//channel state set
	reg_addr = REG_ADDR__I2S_CHST0;
	reg_val = 0x0;
	ret = misc_reg_write8(reg_addr, &reg_val, 3); //0x3a1e/0x3a1f/0x3a20
	reg_addr = REG_ADDR__I2S_CHST3;//????
	reg_val = 0x2;
	ret = misc_reg_write8(reg_addr, &reg_val, 1);
	reg_addr = REG_ADDR__I2S_CHST4;//???
	reg_val = 0x2;
	ret = misc_reg_write8(reg_addr, &reg_val, 1);
	reg_addr = REG_ADDR__I2S_CHST6;
	reg_val = 0x0;
	ret = misc_reg_write8(reg_addr, &reg_val, 2);
	return ret;
}

//#define AUD_SAMPLE_RATE_EF  32809  //32809 = 2^33 * 1000 / 261818181
#define AUD_SAMPLE_RATE_2_33  	(8589934592ULL)  //32809 = 2^33
#define SYM6_FPGA_AOUT_108M (1)

#if (defined(CONFIG_MT_CHIP_ETUDE2))

	#if  (!defined(CONFIG_MT_FPGA))
		#define AUD_SAMPLE_RATE_EF  32809  //32809 = 2^33 * 1000 / 261818181
		#define AUD_SAMPLE_RATE_MCLK  	(261818181ULL)  //261818181
	#else
		#define AUD_SAMPLE_RATE_EF  159072  //32809 = 2^33 * 1000 / 54000000
		#define AUD_SAMPLE_RATE_MCLK  	(54000000ULL)  //261818181
	#endif

#elif (defined(CONFIG_MT_CHIP_SYMPHONY6))

	#if  (!defined(CONFIG_MT_FPGA))
		#define AUD_SAMPLE_RATE_EF  32809  //32809 = 2^33 * 1000 / 261818181
		#define AUD_SAMPLE_RATE_MCLK  	(261818181ULL)  //261818181
	#else
		#if SYM6_FPGA_AOUT_108M
			#define AUD_SAMPLE_RATE_EF  79536  //79536 = 2^33 * 1000 / 108000000
			#define AUD_SAMPLE_RATE_MCLK  	(108000000ULL)  //261818181
		#else
			#define AUD_SAMPLE_RATE_EF  171798  //32809 = 2^33 * 1000 / 50000000
			#define AUD_SAMPLE_RATE_MCLK  	(50000000ULL)  //261818181
		#endif
	#endif

#else

	#if  (!defined(CONFIG_MT_FPGA))
		#define AUD_SAMPLE_RATE_EF  32809  //32809 = 2^33 * 1000 / 261818181
		#define AUD_SAMPLE_RATE_MCLK  	(261818181ULL)  //261818181
	#else
		#define AUD_SAMPLE_RATE_EF  159072  //32809 = 2^33 * 1000 / 54000000
		#define AUD_SAMPLE_RATE_MCLK  	(54000000ULL)  //261818181
	#endif

#endif

static mt_u32 aout_play(AUD_INPUT_T params, mt_u32 BufAddr, mt_u32 BufLen)
{
	mt_u32 ret = 0;
	ulong reg_addr = 0x1f490084;
	mt_u32 reg_val = 0;
	mt_u32 cur_sr;
	mt_u64 clk_div64 = (0ULL);

	reg_addr = 0x1f490084;
	reg_val = 0;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	reg_addr = 0x1f50a504;
	reg_val = misc_reg_read32_single(reg_addr);
	reg_val = (reg_val & (~0x3)) | 0x1;
	ret = misc_reg_write32_single(reg_addr, reg_val);
	reg_addr = 0x1f50a500;
	reg_val = misc_reg_read32_single(reg_addr);
	reg_val = (reg_val & (~0x3)) | 0x3;
	ret = misc_reg_write32_single(reg_addr, reg_val);

	ret = aud_ch_srt_cfg(params, 0);
	ret = aud_i2s_spdif_cfg(params);
	reg_addr = 0x1f49000c;
	if (params.ch == 8) {
		reg_val = 0x0;    //[3]: data writen to audio out buffer '0': 8ch mode, '1':2ch mode
	} else {
		reg_val = 0x8;    //[3]: data writen to audio out buffer '0': 8ch mode, '1':2ch mode
	}
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	reg_addr = 0x1f490010;

	switch (params.fs) {
		case 0:
			cur_sr = 32000;
			break;
		case 1:
			cur_sr = 48000;
			break;
		case 2:
			cur_sr = 96000;
			break;
		case 3:
			cur_sr = 192000;
			break;
		case 4:
			cur_sr = 44100;//44.1k, use 44 avoiding float point to test
			break;
		case 5:
			cur_sr = 88200;//88.2k, use 88 avoiding float point to test
			break;
		case 6:
			cur_sr = 176000;//176.4k, use 176 avoiding float point to test
			break;
		default:
			cur_sr = 48000;
			break;
	}

	clk_div64 = AUD_SAMPLE_RATE_2_33 * cur_sr / AUD_SAMPLE_RATE_MCLK;
	//reg_val = reg_val * cur_sr / base_sr;
	reg_val = (mt_u32)clk_div64;

	if (1 == params.mode_ex) {
		reg_val <<= 2;//clk*4 when plays DD+
	}

	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	reg_addr = 0x1f490018;
	reg_val = (mt_u32)BufAddr / 8; //audio out buffer base  Unit 8Bytes
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	reg_addr = 0x1f49001c;
	reg_val = BufLen >> 3; //audio out buffer length  Unit 8Bytes, 128Bytes align
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	reg_addr = 0x1f490020;
	reg_val = (BufLen >> 3) + 16; //aout buff full threshold, never full for ic verify
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	reg_addr = 0x1f490008;
	reg_val = (mt_u32)0x8000;//volume
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	reg_addr = 0x1f490084;
	reg_val = 0x8;//audio out buffer length  Unit 8Bytes, 128Bytes align
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	ret = aud_hdmi_params_set(params);
	return ret;
}

mt_u32 do_case_iis_audio(void)
{
	mt_u32 u32Ret = 0;
	AUD_INPUT_T aud_param = {0};
	const char filename[30] = "hdmi_pcm_048k_2ch.raw";//
	FILE *fin;
	mt_u32 aud_buf = 0x3200000;
	mt_u8 *data_in_ptr = (mt_u8 *)NULL;
	mt_u32 data_in_size = 0;
	mt_u32 file_len = 0;
	mt_u32 len = 1 << 18;

	u32Ret = get_file_name(filename, &aud_param);

	SSHH_PRINTF( "\r\n open file:");
	SSHH_PRINTF( "%s\n", filename);
	fopen_s(&fin, filename, "rb");
	if (!fin) {
		SSHH_PRINTF( "\r\n fail to open file:%s!!!\n", filename);
		return BB_FAILURE;
	} else {
		SSHH_PRINTF( "\r\n success to open file:%s!!!\n", filename);
	}

	fseek(fin, 0, SEEK_END);
	file_len = (mt_u32)ftell(fin);
	fseek(fin, 0, SEEK_SET);
	u32Ret = get_aud_mem(&data_in_ptr, AUD_DATA_PLAY_SIZE);
	if (data_in_ptr == NULL) {
		SSHH_PRINTF( "\r\n alloc aud data buffer fail:0x%x!!!", AUD_DATA_PLAY_SIZE);
	}
	if ((aud_param.ch == 6 || aud_param.ch == 8) && aud_param.mode == 0) { //pcm, source file 2ch, convert to 6/8 ch
		uint8_t data_tmp[16];
		uint8_t mul = 3;
		uint8_t i;
		len = 4;//L/R 16bit*2
		while ((data_in_size + len * mul)  < file_len && (data_in_size + len * mul) < AUD_DATA_PLAY_SIZE) {
			fread(data_tmp, len, 1, fin);
			for (i = 0; i < ((aud_param.ch + 1) >> 1); i++) {
				memcpy(data_in_ptr + data_in_size, data_tmp, len);
				data_in_size += len;
			}
		}
	} else { /*if(aud_param.ch == 2)*/
		len = 1 << 18;
		while ((data_in_size + len)  < file_len && (data_in_size + len) < AUD_DATA_PLAY_SIZE) {
			fread(data_in_ptr + data_in_size, len, 1, fin);
			data_in_size += len;
		}
	}
	fclose(fin);
	u32Ret = enable_aout_clk(1);
	u32Ret = config_src_coef();
	u32Ret = preload_aud_data(&aud_buf, data_in_ptr, data_in_size);

	SSHH_PRINTF( "\r\n aud raw buf:0x%x,size:%d,flen=%d!!!", aud_buf, data_in_size, file_len);
	for (len = 0; len < 8; len++) {
		SSHH_PRINTF( "\r\n aud raw buf:data_in_size[%d]=0x%x!!!", len, (mt_u32)data_in_ptr[len]);
	}
	if (aud_param.mode == 1 || aud_param.mode == 3) {
		aud_param.ch = 2;    //Treat compressed audio as 2 channels
	}
	u32Ret = aout_play(aud_param, aud_buf, data_in_size);
	SiiLibTimeMilliDelay(1000 * 10); //played for 10s
	u32Ret = release_aud_mem(data_in_ptr);
	return u32Ret;
}

mt_u32 do_case_iis_audio_default(void)
{
	mt_u32 u32Ret = 0;
	AUD_INPUT_T aud_param = {0};
	const char filename[30] = "hdmi_pcm_048k_2ch.raw";//
	FILE *fin;
	mt_u32 aud_buf = 0x3200000;
	mt_u8 *data_in_ptr = (mt_u8 *)NULL;
	mt_u32 data_in_size = 0;
	mt_u32 file_len = 0;
	mt_u32 len = 1 << 18;
	AVINFOSTORAGE_T param;

	u32Ret = get_hdmi_storage_params(&param);
	aud_param.ch = 2;
	aud_param.downmix = 0;
	aud_param.fs = 1;
	aud_param.mode = 0;
	aud_param.mode_ex = 0;

	param.aud_param.audio_ch = aud_param.ch;
	param.aud_param.audio_mode = aud_param.mode;
	param.aud_param.audio_sr = aud_param.fs;
	param.aud_param.audio_downmix = aud_param.downmix;
	param.aud_param.audio_mode_ex = aud_param.mode_ex;
	param.aud_param.audio_ofile = 0x0;

	SSHH_PRINTF( "\r\n open file:");
	SSHH_PRINTF( "%s\n", filename);
	//fopen_s(&fin, filename, "rb");
	fopen_aud(&fin, filename, "rb");
	if (!fin) {
		SSHH_PRINTF( "\r\n fail to open file:%s!!!\n", filename);
		param.aud_param.audio_ch = aud_param.ch;
		param.aud_param.audio_mode = aud_param.mode;
		param.aud_param.audio_sr = aud_param.fs;
		param.aud_param.audio_downmix = aud_param.downmix;
		param.aud_param.audio_mode_ex = aud_param.mode_ex;
		param.aud_param.audio_ofile = 0xff;
		set_hdmi_storage_params(param);
		u32Ret = aud_hdmi_params_set(aud_param);
		return BB_FAILURE;
	} else {
		SSHH_PRINTF( "\r\n success to open file:%s!!!\n", filename);
	}

	fseek(fin, 0, SEEK_END);
	file_len = (mt_u32)ftell(fin);
	fseek(fin, 0, SEEK_SET);
	u32Ret = get_aud_mem(&data_in_ptr, AUD_DATA_PLAY_SIZE);
	if (data_in_ptr == NULL) {
		SSHH_PRINTF( "\r\n alloc aud data buffer fail:0x%x!!!", AUD_DATA_PLAY_SIZE);
	}
	if ((aud_param.ch == 6 || aud_param.ch == 8) && aud_param.mode == 0) { //pcm, source file 2ch, convert to 6/8 ch
		uint8_t data_tmp[16];
		uint8_t mul = 3;
		uint8_t i;
		len = 4;//L/R 16bit*2
		while ((data_in_size + len * mul)  <= file_len && (data_in_size + len * mul) <= AUD_DATA_PLAY_SIZE) {
			fread(data_tmp, len, 1, fin);
			for (i = 0; i < ((aud_param.ch + 1) >> 1); i++) {
				memcpy(data_in_ptr + data_in_size, data_tmp, len);
				data_in_size += len;
			}
		}
	} else { /*if(aud_param.ch == 2)*/
		len = 1 << 18;
		while ((data_in_size + len)  <= file_len && (data_in_size + len) <= AUD_DATA_PLAY_SIZE) {
			fread(data_in_ptr + data_in_size, len, 1, fin);
			data_in_size += len;
		}
	}
	fclose(fin);
	u32Ret = enable_aout_clk(1);
	u32Ret = config_src_coef();
	u32Ret = preload_aud_data(&aud_buf, data_in_ptr, data_in_size);

	SSHH_PRINTF( "\r\n aud raw buf:0x%x,size:%d,flen=%d!!!", aud_buf, data_in_size, file_len);
	for (len = 0; len < 8; len++) {
		SSHH_PRINTF( "\r\n aud raw buf:data_in_size[%d]=0x%x!!!", len, (mt_u32)data_in_ptr[len]);
	}
	if (aud_param.mode == 1) {
		aud_param.ch = 2;    //Treat compressed audio as 2 channels
	}
	u32Ret = aout_play(aud_param, aud_buf, data_in_size);
	SiiLibTimeMilliDelay(1000 * 10); //played for 10s
	u32Ret = release_aud_mem(data_in_ptr);
	set_hdmi_storage_params(param);
	return u32Ret;
}

/***** end of file ***********************************************************/

