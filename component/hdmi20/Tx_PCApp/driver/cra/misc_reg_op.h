/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MISC_REG_OP_H__
#define __MISC_REG_OP_H__

/*=========================================================================
| TYPEDEFS
 ========================================================================*/
#define SD_RESET_REG_CNT (2)
#define HDMI20_RELEASE_2_QA (1)
#define HDMI20_RELEASE_2_QA_SYM6 (1)
#define HDMI20_AUD_HBR		(1)
#define HDMI20_DISP_3D		(0)
#define HDMI20_DEEP_COLOR		(1)

typedef struct {
	mt_u8 scanInfo;
	mt_u8 barInfo;
	mt_u8 activeFormatInfo;
	mt_u8 clrSpc;
	mt_u8 activeAR;
	mt_u8 pictureAR;
	mt_u8 colorimetry;
	mt_u8 scalingInfo;
	mt_u8 rgbQR;
	mt_u8 extColorimetry;
	mt_u8 itContent;
	mt_u8 vic;
	mt_u8 repetition;
	mt_u8 itContentType;
	mt_u8 yccQR;
} MtAviInfo_t;

typedef struct AUDIO_PARAM_S {
	mt_u32 audio_mode;
	mt_u32 audio_sr;
	mt_u32 audio_ch;
	mt_u32 audio_downmix;
	mt_u32 audio_mode_ex;
	mt_u32 audio_ofile;
	mt_u32 is_hbr; //'0': normal, '1' hbr
} AUDIO_PARAM_T;

typedef struct AVINFOSTORAGE_S {
	mt_u32 init;
	mt_u32 tvsys;
	MtAviInfo_t vinfo;
	AUDIO_PARAM_T aud_param;
	mt_u32 usr;
	mt_u32 ar; //'0':16:9 '1':4:3
} AVINFOSTORAGE_T;

/***** local functions *******************************************************/

mt_u32 regfile_mem_init (void);
mt_u32 regfile_mem_open (ulong base, mt_u32 size, void* sys_reg_virt_addr);
mt_u32 regfile_mem_open_all (void);
mt_u32 regfile_mem_close (void* sys_reg_virt_addr);
mt_u32 regfile_mem_close_all (void);
mt_u32 regfile_mem_deinit (void);

mt_u32 misc_reg_read32 (ulong regAddr, mt_u32* pData, mt_u32 length);
mt_u32 misc_reg_write32 (ulong regAddr, mt_u32* Data, mt_u32 length);
mt_u32 misc_reg_put32 (ulong regAddr, mt_u32 mask, mt_u32 val);
mt_u32 misc_reg_read8 (ulong regAddr, mt_u8* pData, mt_u32 length);
mt_u32 misc_reg_write8 (ulong regAddr, mt_u8* Data, mt_u32 length);
mt_u32 misc_reg_put8 (ulong regAddr, mt_u8 mask, mt_u8 val);
mt_u32 get_hdmi_storage_params(AVINFOSTORAGE_T *param);
mt_void set_hdmi_storage_params(AVINFOSTORAGE_T param);
mt_u32 misc_reg_write8_single (ulong regAddr, mt_u8 val);
mt_u8 misc_reg_read8_single (ulong regAddr);
mt_u32 misc_reg_read32_single (ulong regAddr);
mt_u32 misc_reg_write32_single (ulong regAddr, mt_u32 val);

/***** end of file ***********************************************************/

#endif /* __MISC_REG_OP_H__ */
