/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2018, Montage Technology Co., Ltd.
 *
 * File Name      : drv_vdec_debug.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2018/1/4
 * Description    : MT VDEC DRV Debug & Dump functions.
 * History        :
 * 1.Date         : 2018/1/4
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>

//#include "sys_define.h"
//#include "drv_vdec_sys_define.h"

#include "vfmw.h"
#include "vfmw_reg.h"
#include "drv_vdec_ext.h"
#include "mt_drv_video.h"
#include "mt_unf_video.h"
#include "drv_vdec_private.h"
#include "vconfig.h"
#include "drv_vdec_debug.h"
#include "mt_module_debug.h"

#undef LOG_TAG
#define LOG_TAG				"VDEC_DRV"
#include "Log.h"

#undef PRINTF
//#define PRINTF			printk
#define PRINTF			MT_INFO_VDEC
#define PRINTF_ALWAYS	MT_ERR_VDEC

#if (CFG_VFMW_ON_AVCPU == 1)
/* Shared Video ES Descriptor queue */
extern VES_INST_S *ves_buffer_inst;
#else
extern VES_INST_S ves_buffer_inst[];
#endif

//spinlock for debug log
DEFINE_SPINLOCK(kVDebugLock);
//log file for debug
struct file *gp_VDebugLogFile = NULL;
DEFINE_SEMAPHORE(kVDebugMutex, 1);
//global video driver debug switcher
int g_VDebugEnable = 0;

//global video driver log level
int g_vlog_level = 1;

static int vdec_dump_es_stop(void);

//----------------------------------------------------------------------------//
//File
struct file *open_file(char *path,int flag,int mode)
{
	struct file *fp;
	fp=filp_open(path, flag, mode);
	if (!IS_ERR_OR_NULL(fp)) return fp;
	else return NULL;
}

int read_file(struct file *fp,char *buf,int readlen)
{
	int ret = 0;
#if 0
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(0);

#if 0
	if (fp->f_op && fp->f_op->read)
		ret = fp->f_op->read(fp,buf,readlen, &fp->f_pos);
	else
		ret = -1;
#else
	ret = vfs_read(fp, buf, readlen, &fp->f_pos);
#endif
	set_fs(old_fs);
#endif
	return ret;
}

int write_file(struct file *fp,char *buf,int len)
{
#if 0
	int ret;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

#if 0
	if (fp->f_op && fp->f_op->write)
		ret = fp->f_op->write(fp, buf, len, &fp->f_pos);
	else
		ret = -1;
#else
	ret = vfs_write(fp, buf, len, &fp->f_pos);
#endif

	set_fs(old_fs);

	return ret;
#endif
	return 0;
}

int close_file(struct file *fp)
{
	filp_close(fp,NULL);
	return 0;
}

int sync_file(struct file *fp)
{
	vfs_fsync(fp,0);
	return 0;
}

//----------------------------------------------------------------------------//
//Dump
void dump_int_array(const char *title, int *ar, int n)
{
	int i;
	MLOGD("%s\n",title);
	for (i=0;i<n;i++)
	{
		MLOGD("%d\n", ar[i]);
	}
	MLOGD("\n");
}

/* dump Field */
static void dump_field(MT_DIS_FIELD_INFO_T *pField)
{
	PRINTF("Dump Field %p Begin::\n",pField);
	PRINTF("  slot_idx: 0x%x\n",pField->slot_idx);
	PRINTF("  pts: 0x%x\n",pField->pts);
	PRINTF("  addrLuma: 0x%x\n",pField->addrLuma);
	PRINTF("  addrChroma: 0x%x\n",pField->addrChroma);
	PRINTF("  addrLuma_lut: 0x%x\n",pField->addrLuma_lut);
	PRINTF("  addrChroma_lut: 0x%x\n",pField->addrChroma_lut);
	PRINTF("  stride_data_luma: 0x%x\n",pField->stride_data_luma);
	PRINTF("  stride_data_chroma: 0x%x\n",pField->stride_data_chroma);
	PRINTF("  bit_depth_luma: 0x%x\n",pField->bit_depth_luma);
	PRINTF("  bit_depth_chroma: 0x%x\n",pField->bit_depth_chroma);
	PRINTF("Dump Field %p End.\n",pField);
}

/* dump SLOT Info */
void vdec_dump_slot(MT_DIS_FRAME_SLOT_INFO_T *pSlot)
{
	int i;

	PRINTF("Dump Slot Info (%p, %d) Begin::\n",pSlot,sizeof(MT_DIS_FRAME_SLOT_INFO_T));
	PRINTF("  frm_cnt: %u\n",pSlot->frm_cnt);
	PRINTF("  gop_id: 0x%x\n",pSlot->gop_id);
	PRINTF("  pts: 0x%llx\n",pSlot->pts);
	PRINTF("  pic_width: %u\n",pSlot->pic_width);
	PRINTF("  pic_height: %u\n",pSlot->pic_height);
	PRINTF("  row_jump_value: 0x%x\n",pSlot->row_jump_value);
	PRINTF("  row_jump_offset: 0x%x\n",pSlot->row_jump_offset);
	PRINTF("  display_order_mode_valid: %u\n",pSlot->display_order_mode_valid);
	PRINTF("  display_order_mode: %d\n",pSlot->display_order_mode);
	PRINTF("  repeat_frm_num: %u\n",pSlot->repeat_frm_num);
	PRINTF("  frame_pic_flag: %u\n",pSlot->frame_pic_flag);
	PRINTF("  progressive_frame: %u\n",pSlot->progressive_frame);
	PRINTF("  dirty_flag: %u\n",pSlot->dirty_flag);
	PRINTF("  progressive_sequence: %u\n",pSlot->progressive_sequence);
	PRINTF("  top_field_first: %u\n",pSlot->top_field_first);
	PRINTF("  picture_coding_type: %u\n",pSlot->picture_coding_type);
	PRINTF("  aspect_ratio: %u\n",pSlot->aspect_ratio);
	PRINTF("  fw_disable_di: %u\n",pSlot->fw_disable_di);
	PRINTF("  field_storage_mode: %u\n",pSlot->filed_storage_mode);
	PRINTF("  isHalf: %u\n",pSlot->isHalf);
	PRINTF("  is_3D_flag: %u\n",pSlot->is_3D_flag);
	PRINTF("  packing_type: %d\n",pSlot->packing_type);
	PRINTF("  tile_cfg: %u\n",pSlot->tile_cfg);
	PRINTF("  col_size: %u\n",pSlot->col_size);
	PRINTF("  frame_size: 0x%x\n",pSlot->frame_size);
	PRINTF("  input_rate: %u\n",pSlot->input_rate);
	PRINTF("  dcedat: %p\n    ",pSlot->dcedat);
	for (i=0;i<DCE_NUM;i++)
	{
		PRINTF("%08x ",pSlot->dcedat[i]);
	}
	PRINTF("\n");
	PRINTF("  p_usrdat[0]: %p\n",pSlot->p_usrdat[0]);
	PRINTF("  p_usrdat[1]: %p\n",pSlot->p_usrdat[1]);
	PRINTF("  p_usrdat[2]: %p\n",pSlot->p_usrdat[2]);
	PRINTF("  p_usrdat[3]: %p\n",pSlot->p_usrdat[3]);
	PRINTF("  sar_width: %u\n",pSlot->sar_width);
	PRINTF("  sar_height: %u\n",pSlot->sar_height);
	PRINTF("  active_format_flag: %u\n",pSlot->active_format_flag);
	PRINTF("  active_format: %u\n",pSlot->active_format);
	PRINTF("  end_of_stream_flag: %u\n",pSlot->end_of_stream_flag);
	PRINTF("  vdec_mfbc_enable: %u\n",pSlot->vdec_mfbc_enable);
	PRINTF("  dec_type: %d\n",pSlot->dec_type);
	PRINTF("  uCurBank: %d\n",pSlot->uCurBank);
	PRINTF("  crop_info: %u, (%u, %u, %u, %u)\n",
			pSlot->crop_info.bEnable,
			pSlot->crop_info.left,
			pSlot->crop_info.top,
			pSlot->crop_info.width,
			pSlot->crop_info.height);

	//HDR
	PRINTF("  HDR - transfer_characteristics: %x\n",pSlot->transfer_characteristics);
	PRINTF("  HDR - colour_primaries: %x\n",pSlot->colour_primaries);
	PRINTF("  HDR - colour_volume: \n");
	PRINTF("      colour_volume_enable: %u\n",
			pSlot->colour_volume.colour_volume_enable);
	PRINTF("      display_primaries_x: %x, %x, %x\n",
			pSlot->colour_volume.display_primaries_x[0],
			pSlot->colour_volume.display_primaries_x[1],
			pSlot->colour_volume.display_primaries_x[2]);
	PRINTF("      display_primaries_y: %x, %x, %x\n",
			pSlot->colour_volume.display_primaries_y[0],
			pSlot->colour_volume.display_primaries_y[1],
			pSlot->colour_volume.display_primaries_y[2]);
	PRINTF("      white_point_x: %x\n",
			pSlot->colour_volume.white_point_x);
	PRINTF("      white_point_y: %x\n",
			pSlot->colour_volume.white_point_y);
	PRINTF("      max_luminance: %x\n",
			pSlot->colour_volume.max_luminance);
	PRINTF("      min_luminance: %x\n",
			pSlot->colour_volume.min_luminance);

	PRINTF("  HDR - light_level: \n");
	PRINTF("      light_level_info_enable: %u\n",
			pSlot->light_level.light_level_info_enable);
	PRINTF("      max_light_level: %x\n",
			pSlot->light_level.max_light_level);
	PRINTF("      max_pic_ave_light_level: %x\n",
			pSlot->light_level.max_pic_ave_light_level);

	PRINTF("  HDR - sl_hdr_metadata_flag: %u\n",pSlot->hdr10p_dynamic_metadata_valid_flag);
	PRINTF("  HDR - sl_hdr_yuv_range: %d\n",pSlot->video_range);
	PRINTF("  HDR - sl_hdr_metadata_addr: 0x%x\n",pSlot->hdr10p_common_info_addr);

	PRINTF("  filedInfoTop: %p\n",&pSlot->filedInfoTop);
	dump_field(&pSlot->filedInfoTop);
	PRINTF("  filedInfoBot: %p\n",&pSlot->filedInfoBot);
	dump_field(&pSlot->filedInfoBot);
	PRINTF("  filedInfoTopRight: %p\n",&pSlot->filedInfoTopRight);
	dump_field(&pSlot->filedInfoTopRight);
	PRINTF("  filedInfoBotRight: %p\n",&pSlot->filedInfoBotRight);
	dump_field(&pSlot->filedInfoBotRight);

	PRINTF("Dump Slot Info %p End.\n",pSlot);
}

/* dump IMAGE BTL Info */
static void dump_image_btl(IMAGE_BTL_S *pBtl)
{
	PRINTF("Dump Image BTL %p Begin::\n",pBtl);
	PRINTF("  Is1D: %u\n",pBtl->u32Is1D);
	PRINTF("  btl_imageid: %d\n",pBtl->btl_imageid);
	PRINTF("  IsCompress: %u\n",pBtl->u32IsCompress);
	PRINTF("  HeadStride: %u\n",pBtl->u32HeadStride);
	PRINTF("  HeadOffset: %u\n",pBtl->u32HeadOffset);
	PRINTF("  YHeadAddr: 0x%x\n",pBtl->u32YHeadAddr);
	PRINTF("  CHeadAddr: 0x%x\n",pBtl->u32CHeadAddr);
	PRINTF("  CrStride: %u\n",pBtl->u32CrStride);
	PRINTF("  CStride: %u\n",pBtl->u32CStride);
	PRINTF("  CrAddr: 0x%x\n",pBtl->u32CrAddr);
	PRINTF("  DNROpen: %u\n",pBtl->u32DNROpen);
	PRINTF("  DNRInfoAddr: 0x%x\n",pBtl->u32DNRInfoAddr);
	PRINTF("  DNRInfoStride: %u\n",pBtl->u32DNRInfoStride);
	PRINTF("  YUVFormat: %d\n",pBtl->YUVFormat);

	PRINTF("Dump Image BTL %p End.\n",pBtl);
}

/* dump IMAGE DNR */
static void dump_image_dnr(IMAGE_DNR_S *pDnr)
{
	PRINTF("Dump Image DNR %p Begin::\n",pDnr);
	PRINTF("  bottom_Range_mapy_flag: %u\n",pDnr->bottom_Range_mapy_flag);
	PRINTF("  bottom_Range_mapy: %u\n",pDnr->bottom_Range_mapy);
	PRINTF("  bottom_Range_mapuv_flag: %u\n",pDnr->bottom_Range_mapuv_flag);
	PRINTF("  bottom_Range_mapuv: %u\n",pDnr->bottom_Range_mapuv);
	PRINTF("  pic_structure: %u\n",pDnr->pic_structure);
	PRINTF("  Range_mapy_flag: %u\n",pDnr->Range_mapy_flag);
	PRINTF("  Range_mapy: %u\n",pDnr->Range_mapy);
	PRINTF("  Range_mapuv_flag: %u\n",pDnr->Range_mapuv_flag);
	PRINTF("  Range_mapuv: %u\n",pDnr->Range_mapuv);
	PRINTF("  chroma_format_idc: %u\n",pDnr->chroma_format_idc);
	PRINTF("  vc1_profile: %u\n",pDnr->vc1_profile);
	PRINTF("  use_pic_qp_en: %d\n",pDnr->use_pic_qp_en);
	PRINTF("  frame compress enable: %d\n",pDnr->s32VcmpEn);
	PRINTF("  water marker enable: %d\n",pDnr->s32WmEn);
	PRINTF("  video_standard: %d\n",pDnr->video_standard);
	PRINTF("  QP_Y: %d\n",pDnr->QP_Y);
	PRINTF("  QP_U: %d\n",pDnr->QP_U);
	PRINTF("  QP_V: %d\n",pDnr->QP_V);
	PRINTF("  Rangedfrm: %d\n",pDnr->Rangedfrm);
	PRINTF("  water marker start line number: %d\n",pDnr->s32VcmpWmStartLine);
	PRINTF("  water marker end line number: %d\n",pDnr->s32VcmpWmEndLine);
	PRINTF("  DNR output frame lines width(align 16): %d\n",pDnr->s32VcmpFrameWidth);
	PRINTF("  DNR output frame lines height(align 16): %d\n",pDnr->s32VcmpFrameHeight);

	PRINTF("Dump Image DNR %p End.\n",pDnr);
}

/* dump IMAGE OPTMALG Info */
static void dump_image_optm(VDEC_OPTMALG_INFO_S *pOptm)
{
	PRINTF("Dump Image OPTMALG Info %p Begin::\n",pOptm);
	PRINTF("  IsProgressiveSeq: %d\n",pOptm->IsProgressiveSeq);
	PRINTF("  IsProgressiveFrm: %d\n",pOptm->IsProgressiveFrm);
	PRINTF("  RealFrmRate: %d\n",pOptm->RealFrmRate);
	PRINTF("  MatrixCoef: %d\n",pOptm->MatrixCoef);
	PRINTF("  Rwzb: 0x%x\n",pOptm->Rwzb);

	PRINTF("Dump Image OPTMALG Info %p End.\n",pOptm);
}

/* dump IMAGE */
void vdec_dump_image(IMAGE *pImage)
{
	PRINTF("Dump Image (%p, %d) Begin::\n",pImage,sizeof(IMAGE));
	PRINTF("  SrcPts: %llu\n",pImage->SrcPts);
	PRINTF("  PTS: %llu\n",pImage->PTS);
	PRINTF("  Usertag: 0x%llx\n",pImage->Usertag);
	PRINTF("  DispTime: %llu\n",pImage->DispTime);
	PRINTF("  luma_vir_addr: %p\n",pImage->luma_vir_addr);
	PRINTF("  chrom_vir_addr: %p\n",pImage->chrom_vir_addr);
	PRINTF("  luma_tf_vir_addr: %p\n",pImage->luma_tf_vir_addr);
	PRINTF("  chrom_tf_vir_addr: %p\n",pImage->chrom_tf_vir_addr);
	PRINTF("  luma_2d_vir_addr: %p\n",pImage->luma_2d_vir_addr);
	PRINTF("  chrom_2d_vir_addr: %p\n",pImage->chrom_2d_vir_addr);
	PRINTF("  line_num_vir_addr: %p\n",pImage->line_num_vir_addr);
	PRINTF("  AspectWidth: %u\n",pImage->u32AspectWidth);
	PRINTF("  AspectHeight: %u\n",pImage->u32AspectHeight);
	PRINTF("  DispEnableFlag: 0x%x\n",pImage->DispEnableFlag);
	PRINTF("  DispFrameDistance: %u\n",pImage->DispFrameDistance);
	PRINTF("  DistanceBeforeFirstFrame: %u\n",pImage->DistanceBeforeFirstFrame);
	PRINTF("  GopNum: %u\n",pImage->GopNum);
	PRINTF("  RepeatCnt: %u\n",pImage->u32RepeatCnt);
	PRINTF("  top_luma_phy_addr: 0x%x\n",pImage->top_luma_phy_addr);
	PRINTF("  top_chrom_phy_addr: 0x%x\n",pImage->top_chrom_phy_addr);
	PRINTF("  btm_luma_phy_addr: 0x%x\n",pImage->btm_luma_phy_addr);
	PRINTF("  btm_chrom_phy_addr: 0x%x\n",pImage->btm_chrom_phy_addr);
	PRINTF("  luma_phy_addr: 0x%x\n",pImage->luma_phy_addr);
	PRINTF("  chrom_phy_addr: 0x%x\n",pImage->chrom_phy_addr);
	PRINTF("  luma_tf_phy_addr: 0x%x\n",pImage->luma_tf_phy_addr);
	PRINTF("  chrom_tf_phy_addr: 0x%x\n",pImage->chrom_tf_phy_addr);
	PRINTF("  luma_2d_phy_addr: 0x%x\n",pImage->luma_2d_phy_addr);
	PRINTF("  chrom_2d_phy_addr: 0x%x\n",pImage->chrom_2d_phy_addr);
	PRINTF("  is_fld_save: %u\n",pImage->is_fld_save);
	PRINTF("  top_fld_type: %u\n",pImage->top_fld_type);
	PRINTF("  bottom_fld_type: %u\n",pImage->bottom_fld_type);
	PRINTF("  format: 0x%08x\n",pImage->format);
	PRINTF("  image_width: %u\n",pImage->image_width);
	PRINTF("  image_height: %u\n",pImage->image_height);
	PRINTF("  disp_width: %u\n",pImage->disp_width);
	PRINTF("  disp_height: %u\n",pImage->disp_height);
	PRINTF("  disp_center_x: %u\n",pImage->disp_center_x);
	PRINTF("  disp_center_y: %u\n",pImage->disp_center_y);
	PRINTF("  frame_rate: %u\n",pImage->frame_rate);
	PRINTF("  image_stride: %u\n",pImage->image_stride);
	PRINTF("  image_id: 0x%x\n",pImage->image_id);
	PRINTF("  error_level: %u\n",pImage->error_level);
	PRINTF("  seq_cnt: %u\n",pImage->seq_cnt);
	PRINTF("  seq_img_cnt: %u\n",pImage->seq_img_cnt);
	PRINTF("  p_usrdat[0]: %p\n",pImage->p_usrdat[0]);
	PRINTF("  p_usrdat[1]: %p\n",pImage->p_usrdat[1]);
	PRINTF("  p_usrdat[2]: %p\n",pImage->p_usrdat[2]);
	PRINTF("  p_usrdat[3]: %p\n",pImage->p_usrdat[3]);
	PRINTF("  chroma_idc: 0x%x\n",pImage->chroma_idc);
	PRINTF("  bit_depth_luma: %u\n",pImage->bit_depth_luma);
	PRINTF("  bit_depth_chroma: %u\n",pImage->bit_depth_chroma);
	PRINTF("  frame_num: %u\n",pImage->frame_idx);
	PRINTF("  last_frame: %u\n",pImage->last_frame);
	PRINTF("  view_id: 0x%x\n",pImage->view_id);
	PRINTF("  image_id_1: 0x%x\n",pImage->image_id_1);
	PRINTF("  is_3D: %u\n",pImage->is_3D);
	PRINTF("  top_luma_phy_addr_1: 0x%x\n",pImage->top_luma_phy_addr_1);
	PRINTF("  top_chrom_phy_addr_1: 0x%x\n",pImage->top_chrom_phy_addr_1);
	PRINTF("  btm_luma_phy_addr_1: 0x%x\n",pImage->btm_luma_phy_addr_1);
	PRINTF("  btm_chrom_phy_addr_1: 0x%x\n",pImage->btm_chrom_phy_addr_1);
	PRINTF("  line_num_phy_addr: 0x%x\n",pImage->line_num_phy_addr);
#ifdef VFMW_BVT_SUPPORT
	int i;

	PRINTF("  luma_sum_h: 0x%x\n",pImage->luma_sum_h);
	PRINTF("  luma_sum_l: 0x%x\n",pImage->luma_sum_l);
	PRINTF("  luma_historgam: %p\n",pImage->luma_historgam);
	for (i=0;i<32;i++)
	{
		PRINTF("%08x ",pImage->luma_historgam[i]);
	}
	PRINTF("\n");
#endif
	PRINTF("  is_1Dcompress: %u\n",pImage->is_1Dcompress);
	PRINTF("  FramePackingType: %d\n",pImage->eFramePackingType);

	PRINTF("  slotInfo: %p\n",&pImage->slotInfo);
	vdec_dump_slot(&pImage->slotInfo);

	PRINTF("  BTLInfo_1: %p\n",&pImage->BTLInfo_1);
	dump_image_btl(&pImage->BTLInfo_1);

	PRINTF("  ImageDnr: %p\n",&pImage->ImageDnr);
	dump_image_dnr(&pImage->ImageDnr);
	PRINTF("  BTLInfo: %p\n",&pImage->BTLInfo);
	dump_image_btl(&pImage->BTLInfo);
	PRINTF("  optm_inf: %p\n",&pImage->optm_inf);
	dump_image_optm(&pImage->optm_inf);

	PRINTF("Dump Image %p End.\n",pImage);
}

/* dump create channel option */
void vdec_dump_chopt(ChannelInfo_t *pChOpt)
{
//	MLOGD("Dump Channel Option (%lx, %d) Begin::\n",pChOpt,sizeof(ChannelInfo_t));
//	MLOGD("           VDH Mem StartVirAddr: 0x%lx\n",pChOpt->mem_buffer.u32StartVirAddr);
	MLOGI("           VDH Mem StartPhyAddr: 0x%llx\n",pChOpt->mem_buffer.u32StartPhyAddr);
	MLOGI("                   VDH Mem Size: 0x%lx\n",pChOpt->mem_buffer.u32Size);

	MLOGD("                  prescale_addr: 0x%llx\n",pChOpt->prescale_addr);
	MLOGD("                  prescale_size: 0x%x\n",pChOpt->prescale_size);

	MLOGI("                   ES BUFF hBuf: 0x%x\n",pChOpt->es_buffer.hBuf);
	MLOGI("                ES BUFF chan_id: 0x%x\n",pChOpt->es_buffer.chan_id);
	MLOGI("                ES BUFF PhyAddr: 0x%x\n",pChOpt->es_buffer.u32PhyAddr);
	MLOGD("             ES BUFF UsrVirAddr: %p\n",pChOpt->es_buffer.pu8UsrVirAddr);
	MLOGD("             ES BUFF KnlVirAddr: %p\n",pChOpt->es_buffer.pu8KnlVirAddr);
	MLOGI("                   ES BUFF Size: 0x%x\n",pChOpt->es_buffer.u32Size);
	MLOGI("         ES BUFF KnlVirDescAddr: %p\n",pChOpt->es_buffer.pu8KnlVirDescAddr);
	MLOGI("      ES BUFF KnlVirDescBufSize: 0x%x\n",pChOpt->es_buffer.u32KnlVirDescBufSize);
	MLOGI("        ES BUFF UseDescInfoFlag: 0x%x\n",pChOpt->es_buffer.u32UseDescInfoFlag);

	MLOGD("               2nd ES BUFF hBuf: 0x%x\n",pChOpt->es_buffer_2nd.hBuf);
	MLOGD("            2nd ES BUFF chan_id: 0x%x\n",pChOpt->es_buffer_2nd.chan_id);
	MLOGI("            2nd ES BUFF PhyAddr: 0x%x\n",pChOpt->es_buffer_2nd.u32PhyAddr);
	MLOGD("         2nd ES BUFF UsrVirAddr: %p\n",pChOpt->es_buffer_2nd.pu8UsrVirAddr);
	MLOGD("         2nd ES BUFF KnlVirAddr: %p\n",pChOpt->es_buffer_2nd.pu8KnlVirAddr);
	MLOGI("               2nd ES BUFF Size: 0x%x\n",pChOpt->es_buffer_2nd.u32Size);
	MLOGD("     2nd ES BUFF KnlVirDescAddr: %p\n",pChOpt->es_buffer_2nd.pu8KnlVirDescAddr);
	MLOGD("  2nd ES BUFF KnlVirDescBufSize: 0x%x\n",pChOpt->es_buffer_2nd.u32KnlVirDescBufSize);
	MLOGD("    2nd ES BUFF UseDescInfoFlag: 0x%x\n",pChOpt->es_buffer_2nd.u32UseDescInfoFlag);

	MLOGI("                       dec_type: %d\n",pChOpt->dec_type);
	MLOGD("                         Is_RV8: %u\n",pChOpt->Is_rv8);
	MLOGD("                    vc1_sp_main: %u\n",pChOpt->vc1_sp_main);
	MLOGI("                insert_pts_mode: %d\n",pChOpt->insert_pts_mode);
	MLOGD("                hd_open_di_flag: %u\n",pChOpt->hd_open_di_flag);
	MLOGI("                    es ptr mode: 0x%x\n",pChOpt->es_mode);
	MLOGI("                       dec_mode: %d\n",pChOpt->dec_mode);
	MLOGI("                    usrdat_addr: 0x%lx\n",pChOpt->usrdat_addr);

	MLOGI("             res_dynamic_enable: %d\n",pChOpt->res_dynamic_enable);
	MLOGI("            h264_low_delay_mode: %u\n",pChOpt->h264_low_delay_mode);
	MLOGI("           user_disable_timeout: %u\n",pChOpt->user_disable_timeout);

	MLOGI("          sl_hdr_info_base_addr: 0x%lx\n",pChOpt->hdr_common_info_base_address);
//	MLOGI("            vp9_counts_buf_addr: 0x%lx\n",pChOpt->vp9_counts_buf_addr);
/*
	MLOGI("                           HEVC: %d\n",pChOpt->support_hevc);
	MLOGI("                     HEVC 10BIT: %d\n",pChOpt->support_hevc_10b);
	MLOGI("                 Lossy Compress: %d\n",pChOpt->support_lossy);
	MLOGI("   Lossy Compress Quality Level: %d\n",pChOpt->lossy_quality_level);
	MLOGI("                            VP9: %d\n",pChOpt->support_vp9);
//	MLOGI("                   crc_buf_addr: 0x%x\n",pChOpt->crc_buf_addr);
	MLOGI("                   crc_buf_size: %u\n",pChOpt->crc_buf_size);*/
	MLOGD("Dump Channel Option %p End.\n",pChOpt);
}

/* dump channel configuration */
void vdec_dump_chcfg(VDEC_CHAN_CFG_S *pCfg)
{
	PRINTF("Dump Channel Config (%p, %d) Begin::\n",pCfg,sizeof(VDEC_CHAN_CFG_S));
	MLOGI("  VidStd: %d\n",pCfg->eVidStd);
	PRINTF("  StdExt: VC1(%d, %d) / VP6(%d)\n",pCfg->StdExt.Vc1Ext.IsAdvProfile,
				pCfg->StdExt.Vc1Ext.IsAdvProfile,
				pCfg->StdExt.Vp6Ext.bReversed);
	PRINTF("  ChanPriority: %d\n",pCfg->s32ChanPriority);
	MLOGI("  ChanErrThreshold: %d\n",pCfg->s32ChanErrThr);
	PRINTF("  stream overflow control threshold: %d\n",pCfg->s32ChanStrmOFThr);
	PRINTF("  DecMode: %d\n",pCfg->s32DecMode);
	PRINTF("  DecOrderOutput: %d\n",pCfg->s32DecOrderOutput);
	PRINTF("  DnrTfEnable: %d\n",pCfg->s32DnrTfEnable);
	PRINTF("  DnrDispOutEnable: %d\n",pCfg->s32DnrDispOutEnable);
	PRINTF("  BtlDbdrEnable: %d\n",pCfg->s32BtlDbdrEnable);
	PRINTF("  Btl1Dt2DEnable: %d\n",pCfg->s32Btl1Dt2DEnable);
	PRINTF("  LowdlyEnable: %d\n",pCfg->s32LowdlyEnable);
	PRINTF("  frame compress enable: %d\n",pCfg->s32VcmpEn);
	PRINTF("  water marker enable: %d\n",pCfg->s32WmEn);
	PRINTF("  water marker start line: %d\n",pCfg->s32VcmpWmStartLine);
	PRINTF("  water marker end line: %d\n",pCfg->s32VcmpWmEndLine);
	PRINTF("  Support All P frames: %d\n",pCfg->s32SupportAllP);
	PRINTF("  ModuleLowlyEnable: %d\n",pCfg->s32ModuleLowlyEnable);
	PRINTF("  LowdBufEnable: %d\n",pCfg->s32LowdBufEnable);
	PRINTF("  IsOmxPath: %d\n",pCfg->s32IsOmxPath);
	PRINTF("  MaxRawPacketNum: %d\n",pCfg->s32MaxRawPacketNum);
	PRINTF("  MaxRawPacketSize: %d\n",pCfg->s32MaxRawPacketSize);
	PRINTF("  ExtraFrameStoreNum: %d\n",pCfg->s32ExtraFrameStoreNum);
	PRINTF("  ES Mode: %d\n",pCfg->es_mode);
	MLOGI("  UnBlank: %d\n",pCfg->s32UnBlank);

	PRINTF("Dump Channel Config %p End.\n",pCfg);
}

/* dump VPSS frame */
void vdec_dump_vpss_frame(MT_DRV_VIDEO_FRAME_PACKAGE_S *pstFrm)
{
	mt_u32 i;
	MT_DRV_VIDEO_FRAME_S *pVFrm;

	PRINTF("Dump VPSS Frame (%p, %d) Begin::\n",pstFrm,sizeof(MT_DRV_VIDEO_FRAME_PACKAGE_S));
	PRINTF("  FrmNum: %u\n",pstFrm->u32FrmNum);
	if (pstFrm->u32FrmNum > 0)
	{
		for (i=0;i<pstFrm->u32FrmNum;i++)
		{
			pVFrm = &pstFrm->stFrame[i].stFrameVideo;
			PRINTF("== Frame: %u ==\n",i);
			PRINTF("  hPort: 0x%x\n",pstFrm->stFrame[i].hport);

			PRINTF("  FrameIndex: %u\n",pVFrm->u32FrameIndex);

			PRINTF("  LINEAR_FRAME_ADDR: %p\n",pVFrm->stLBufAddr);
			PRINTF("  Left Eye: Addr[0x%x 0x%x 0x%x], Stride[0x%x 0x%x 0x%x], BufSize 0x%x\n",
					pVFrm->stLBufAddr[0].u32YAddr,
					pVFrm->stLBufAddr[0].u32CAddr,
					pVFrm->stLBufAddr[0].u32CrAddr,
					pVFrm->stLBufAddr[0].u32YStride,
					pVFrm->stLBufAddr[0].u32CStride,
					pVFrm->stLBufAddr[0].u32CrStride,
					pVFrm->stLBufAddr[0].u32BufSize);
			PRINTF("  Right Eye: Addr[0x%x 0x%x 0x%x], Stride[0x%x 0x%x 0x%x], BufSize 0x%x\n",
					pVFrm->stLBufAddr[1].u32YAddr,
					pVFrm->stLBufAddr[1].u32CAddr,
					pVFrm->stLBufAddr[1].u32CrAddr,
					pVFrm->stLBufAddr[1].u32YStride,
					pVFrm->stLBufAddr[1].u32CStride,
					pVFrm->stLBufAddr[1].u32CrStride,
					pVFrm->stLBufAddr[1].u32BufSize);

			PRINTF("  VID_FRAME_ADDR: %p\n",pVFrm->stBufAddr);
			PRINTF("  Left Eye - Y: 0x%x 0x%x 0x%x\n",
					pVFrm->stBufAddr[0].u32PhyAddr_YHead,
					pVFrm->stBufAddr[0].u32PhyAddr_Y,
					pVFrm->stBufAddr[0].u32Stride_Y);
			PRINTF("  Left Eye - C/Cb: 0x%x 0x%x 0x%x\n",
					pVFrm->stBufAddr[0].u32PhyAddr_CHead,
					pVFrm->stBufAddr[0].u32PhyAddr_C,
					pVFrm->stBufAddr[0].u32Stride_C);
			PRINTF("  Left Eye - Cr: 0x%x 0x%x 0x%x\n",
					pVFrm->stBufAddr[0].u32PhyAddr_CrHead,
					pVFrm->stBufAddr[0].u32PhyAddr_Cr,
					pVFrm->stBufAddr[0].u32Stride_Cr);

			PRINTF("  Right Eye - Y: 0x%x 0x%x 0x%x\n",
					pVFrm->stBufAddr[1].u32PhyAddr_YHead,
					pVFrm->stBufAddr[1].u32PhyAddr_Y,
					pVFrm->stBufAddr[1].u32Stride_Y);
			PRINTF("  Right Eye - C/Cb: 0x%x 0x%x 0x%x\n",
					pVFrm->stBufAddr[1].u32PhyAddr_CHead,
					pVFrm->stBufAddr[1].u32PhyAddr_C,
					pVFrm->stBufAddr[1].u32Stride_C);
			PRINTF("  Right Eye - Cr: 0x%x 0x%x 0x%x\n",
					pVFrm->stBufAddr[1].u32PhyAddr_CrHead,
					pVFrm->stBufAddr[1].u32PhyAddr_Cr,
					pVFrm->stBufAddr[1].u32Stride_Cr);

			PRINTF("  TunnelPhyAddr: 0x%x\n",pVFrm->u32TunnelPhyAddr);
			PRINTF("  TunnelSrc: 0x%x\n",pVFrm->hTunnelSrc);
			PRINTF("  Width: %u\n",pVFrm->u32Width);
			PRINTF("  Height: %u\n",pVFrm->u32Height);
			PRINTF("  SrcPts: %u\n",pVFrm->u32SrcPts);
			PRINTF("  Pts32: %u\n",pVFrm->u32Pts);
			PRINTF("  OmxPts: %lld\n",pVFrm->s64OmxPts);
			PRINTF("  Pts64: %llu\n",pVFrm->u64Pts);
			PRINTF("  AspectWidth: %u\n",pVFrm->u32AspectWidth);
			PRINTF("  AspectHeight: %u\n",pVFrm->u32AspectHeight);
			PRINTF("  FrameRate: %u\n",pVFrm->u32FrameRate);
			PRINTF("  EOS_flag: %d\n",pVFrm->end_of_stream_flag);
			PRINTF("  PixFormat: %d\n",pVFrm->ePixFormat);
			PRINTF("  Progressive: %d\n",pVFrm->bProgressive);
			PRINTF("  FieldMode: %d\n",pVFrm->enFieldMode);
			PRINTF("  TopFieldFirst: %d\n",pVFrm->bTopFieldFirst);
			PRINTF("  Compressed: %d\n",pVFrm->bCompressd);
			PRINTF("  BitWidth: %d\n",pVFrm->enBitWidth);
			PRINTF("  DispRect: %d,%d,%d,%d\n",
					pVFrm->stDispRect.s32X,pVFrm->stDispRect.s32Y,
					pVFrm->stDispRect.s32Width,pVFrm->stDispRect.s32Height);
			PRINTF("  FrmType: %d\n",pVFrm->eFrmType);
			PRINTF("  Circumrotate: %u\n",pVFrm->u32Circumrotate);
			PRINTF("  ToFlip_H: %d\n",pVFrm->bToFlip_H);
			PRINTF("  ToFlip_V: %d\n",pVFrm->bToFlip_V);
			PRINTF("  ErrorLevel: %u\n",pVFrm->u32ErrorLevel);
			PRINTF("  Priv: 0x%08x %08x %08x %08x ...\n",
						pVFrm->u32Priv[0],pVFrm->u32Priv[1],pVFrm->u32Priv[2],pVFrm->u32Priv[3]);
			PRINTF("  IsFirstIFrame: %d\n",pVFrm->bIsFirstIFrame);
			PRINTF("  StillFrame: %d\n",pVFrm->bStillFrame);
			PRINTF("  VOBufClearFlag: %d\n",pVFrm->bVOBufClearFlag);
			PRINTF("  TBAdjust: %d\n",pVFrm->enTBAdjust);
			PRINTF("  LbxInfo: %d,%d,%d,%d\n",
					pVFrm->stLbxInfo.s32X,pVFrm->stLbxInfo.s32Y,
					pVFrm->stLbxInfo.s32Width,pVFrm->stLbxInfo.s32Height);
			PRINTF("  hVdecHandle: 0x%x\n",pVFrm->hVdecHandle);

			PRINTF("  slotInfo: %p\n",&pVFrm->slotInfo);
			vdec_dump_slot((MT_DIS_FRAME_SLOT_INFO_T*)&pVFrm->slotInfo);
		}
	}

	PRINTF("Dump VPSS Frame %p End.\n",pstFrm);
}

/* dump channel attribute */
void vdec_dump_chattr(MT_UNF_VCODEC_ATTR_S *pAttr)
{
//	MLOGD("Dump Channel Attribute (%p, %d) Begin::\n",pAttr,sizeof(MT_UNF_VCODEC_ATTR_S));
	MLOGD("        Video encoding type: %d\n",pAttr->enType);
	MLOGD("             VCODEC_EXTATTR: VC1(%d, 0x%x), VP6(%d)\n",
			pAttr->unExtAttr.stVC1Attr.bAdvancedProfile,
			pAttr->unExtAttr.stVC1Attr.u32CodecVersion,
			pAttr->unExtAttr.stVP6Attr.bReversed);
	MLOGD("                VCODEC_MODE: %d\n",pAttr->enMode);
	MLOGD("                   ErrCover: %u\n",pAttr->u32ErrCover);
	MLOGD("                   Priority: %u\n",pAttr->u32Priority);
	MLOGD("                OrderOutput: %d\n",pAttr->bOrderOutput);
	MLOGD("                CtrlOptions: %d\n",pAttr->s32CtrlOptions);
	MLOGD("                 Using Desc: %u\n",pAttr->u32UseDescInfoFlag);
	MLOGD("                  FrameRate: (%u, %u.%03u)\n",
			pAttr->u32ForceFrameRateFlag,
			pAttr->u32FrameRateInt,
			pAttr->u32FrameRateDec);
	MLOGD("               CodecContext: %p\n",pAttr->pCodecContext);
	MLOGD("               UnBlank Mode: %d\n",pAttr->enUnBlank);
	MLOGD("    User Disable FW Timeout: %d\n",pAttr->bForceDisableTimeout);

	MLOGD("Dump Channel Attribute %p End.\n",pAttr);
}

//#define CONFIG_MT_DEBUG_V_BUF_STAT
/* dump video DMX&DEC's ES buffer stat */
void vdec_dump_es_stat(mt_handle hVdec, VDEC_CHANNEL_S *pstChan)
{
#define WR_MASK		0x7FFFFFFF

	static unsigned int counter=0;
	static unsigned int pre_rd=0, pre_wr=0, pre_used_sz=0;
	static unsigned int total_rd_sz=0, total_wr_sz=0;
	unsigned int rd=0, wr=0;
	unsigned int desc_rd=0, desc_wr=0;
	unsigned int used_sz=0, es_sz=0;
	unsigned int rd_sz=0, wr_sz=0;
	bool uint32_overflow = false;
	int ves_ch = 0;
	//reset
	if (hVdec == MT_INVALID_HANDLE || pstChan == NULL)
	{
		counter=0;
		pre_rd=0;
		pre_wr=0;
		pre_used_sz=0;
		total_rd_sz=0;
		total_wr_sz=0;
		return;
	}

	//Only support DMX
	if (pstChan->hDmxVidChn == MT_INVALID_HANDLE)
	{
		return;
	}

	//---TODO
	//ves_ch = (int)(pstChan->ves_buffer_channel_id & 0x0F);

#ifdef CONFIG_MT_DEBUG_V_BUF_STAT
	VDEBUG_LOG("Dump VDEC CH(%d) ES Buffer Stat(%u) Begin::\n",hVdec,counter);
#endif

	//HW DMX
	if (pstChan->hDmxVidChn != MT_INVALID_HANDLE)
	{
		//ES Buffer Stat
		rd = VFMW_REG_READ(REG_VDEC_CH_0_RD_PTR);
		wr = VFMW_REG_READ(REG_VDEC_CH_0_WR_PTR);
		wr &= WR_MASK;
		es_sz = VFMW_REG_READ(REG_VDEC_CH_0_SZ_8);
		used_sz = VFMW_REG_READ(REG_VDEC_CH_0_USED_SZ);
		desc_rd = VFMW_REG_READ(REG_DMX_CH_0_DESC_RD_PTR);
		desc_wr = VFMW_REG_READ(REG_DMX_CH_0_DESC_WR_PTR);

#ifdef CONFIG_MT_DEBUG_V_BUF_STAT
		VDEBUG_LOG("  [DMX ES]   Start ADDR: 0x%08x\n",VFMW_REG_READ(REG_DMX_CH_0_START_ADDR));
		VDEBUG_LOG("  [DMX ES]     End ADDR: 0x%08x\n",VFMW_REG_READ(REG_DMX_CH_0_END_ADDR));
		VDEBUG_LOG("  [DMX ES]           RD: 0x%08x\n",VFMW_REG_READ(REG_DMX_CH_0_RD_PTR));
		VDEBUG_LOG("  [DMX ES]           WR: 0x%08x\n",VFMW_REG_READ(REG_DMX_CH_0_WR_PTR));

		VDEBUG_LOG("  [VDEC ES]        ADDR: 0x%08x(Reg*8K)\n",
				VFMW_REG_READ(REG_VDEC_CH_0_ADDR_8K)>>16);
		VDEBUG_LOG("  [VDEC ES]          SZ: 0x%08x(Reg*8+8), 0x%08x\n",
				es_sz,
				pstChan->u32DmxBufSize);
		VDEBUG_LOG("  [VDEC ES]        USED: 0x%08x(0x%08x)\n",used_sz,used_sz<<3);
		VDEBUG_LOG("  [VDEC ES]          RD: 0x%08x(0x%08x)\n",rd,rd<<3);
		VDEBUG_LOG("  [VDEC ES]          WR: 0x%08x(0x%08x)\n",wr,wr<<3);

		//ES Descriptor Stat
		VDEBUG_LOG("  [DMX DESC] Start ADDR: 0x%08x\n",
				VFMW_REG_READ(REG_DMX_CH_0_DESC_START_ADDR));
		VDEBUG_LOG("  [DMX DESC]   End ADDR: 0x%08x\n",VFMW_REG_READ(REG_DMX_CH_0_DESC_END_ADDR));
		//VDEBUG_LOG("  [DMX DESC]         SZ: 0x%08x\n",pstChan->u32DescBuffSize);
		VDEBUG_LOG("  [DMX DESC]         RD: 0x%08x\n",desc_rd);
		VDEBUG_LOG("  [DMX DESC]         WR: 0x%08x\n",desc_wr);
#endif
	}
	//SW FILE
#if (CFG_VFMW_ON_AVCPU == 1)
	else if (ves_buffer_inst != NULL)
#else
	else if (1)
#endif
	{
		//ES Buffer Stat
		rd = VFMW_REG_READ(REG_VDEC_CH_0_RD_PTR);
		wr = VFMW_REG_READ(REG_VDEC_CH_0_WR_PTR);
		wr &= WR_MASK;
		es_sz = VFMW_REG_READ(REG_VDEC_CH_0_SZ_8);
		used_sz = VFMW_REG_READ(REG_VDEC_CH_0_USED_SZ);
		desc_rd = ves_buffer_inst[ves_ch].u32KnDescReadOff;
		desc_wr = ves_buffer_inst[ves_ch].u32KnlDescWriterOff;

#ifdef CONFIG_MT_DEBUG_V_BUF_STAT
		VDEBUG_LOG("  [VDEC ES] ADDR: 0x%08x(Reg*8K), 0x%08x(Phy), 0x%08x(kVirt), 0x%08x(uVirt)\n",
				VFMW_REG_READ(REG_VDEC_CH_0_ADDR_8K)>>16,
				ves_buffer_inst[ves_ch].u32PhyAddr,
				(unsigned int)ves_buffer_inst[ves_ch].pu8KnlVirAddr,
				(unsigned int)ves_buffer_inst[ves_ch].pu8UsrVirAddr);
		VDEBUG_LOG("  [VDEC ES]   SZ: 0x%08x(Reg*8+8), 0x%08x\n",
				es_sz,
				ves_buffer_inst[ves_ch].u32Size);
		VDEBUG_LOG("  [VDEC ES] USED: 0x%08x(0x%08x)\n",used_sz,used_sz<<3);
		VDEBUG_LOG("  [VDEC ES]   RD: 0x%08x(0x%08x)\n",rd,rd<<3);
		VDEBUG_LOG("  [VDEC ES]   WR: 0x%08x(0x%08x)\n",wr,wr<<3);

		//ES Descriptor Stat
		VDEBUG_LOG("  [SW DESC] ADDR: 0x%08x(Phy), 0x%08x(kVirt)\n",
				ves_buffer_inst[ves_ch].u32DescPhyAddr,
				(unsigned int)ves_buffer_inst[ves_ch].pu8KnlVirDescAddr);
		VDEBUG_LOG("  [SW DESC]   SZ: 0x%08x\n",ves_buffer_inst[ves_ch].u32KnlVirDescBufSize);
		VDEBUG_LOG("  [SW DESC]   RD: 0x%08x\n",desc_rd);
		VDEBUG_LOG("  [SW DESC]   WR: 0x%08x\n",desc_wr);
#endif

		//Bug: FW's start is async operation, register REG_VDEC_CH_0_SZ_8
		//     maybe not set correctly yet.
		es_sz = ves_buffer_inst[ves_ch].u32Size>>3;
	}
	else
	{
		return;
	}

	//over write case:
#if 1
	if (rd >= pre_rd)
	{
		rd_sz = rd - pre_rd;
	}
	else
	{
		rd_sz = rd + es_sz - pre_rd;
	}
	if (total_rd_sz + rd_sz < total_rd_sz)
	{
		//32 bit unsigned int overflowed
		uint32_overflow = true;
	}
	total_rd_sz += rd_sz;

	if (wr >= pre_wr)
	{
		wr_sz = wr - pre_wr;
	}
	else
	{
		wr_sz = wr + es_sz - pre_wr;
	}
	if (total_wr_sz + wr_sz < total_wr_sz)
	{
		//32 bit unsigned int overflowed
		uint32_overflow = true;
	}
	total_wr_sz += wr_sz;

	if (total_wr_sz > total_rd_sz + es_sz
		&& !uint32_overflow)
	{
		PRINTF_ALWAYS("\n    !!! Warning - Video ES may be Over Written !!!\n\n");
		PRINTF_ALWAYS("pre_rd: 0x%x, rd: 0x%x, pre_wr: 0x%x, wr: 0x%x, pre_used_sz: 0x%x, used_sz: 0x%x, desc_rd: 0x%x, desc_wr: 0x%x\n",
				pre_rd,rd,pre_wr,wr,pre_used_sz,used_sz,
				desc_rd,desc_wr);
		PRINTF_ALWAYS("total rd: 0x%x, total wr: 0x%x, es sz: 0x%x\n",
				total_rd_sz,total_wr_sz,es_sz);
		PRINTF_ALWAYS("If data rate >= %u(Mbps), it may be a fake WARNING!\n",
				(es_sz+1)*8*8/1024/1024);

		//WARN(1, "Video ES may be Over Written!\nIf data rate >= %u(Mbps), it may be a fake WARNING!\n", (es_sz+1)*8*8/1024/1024);

		//reset, avoid too much print log
		{
			total_rd_sz=0;
			total_wr_sz=0;
		}
	}
#endif

	pre_rd = rd;
	pre_wr = wr;
	pre_used_sz = used_sz;

#ifdef CONFIG_MT_DEBUG_V_BUF_STAT
	VDEBUG_LOG("Dump VDEC CH(%u) ES Buffer Stat(%u) End.\n\n",hVdec,counter);
#endif
	counter ++;
}

//----------------------------------------------------------------------------//
//Dump Video ES Data
//for both local/network File play & DVB/TS play

static struct
{
	int enable;					//enable/disable dump VES
	char fname[256];		//dump VES file name
	struct file *fhandle;	//dump VES file handle

//dump ES thread for DVB/TS play mode
#ifdef CONFIG_MT_DUMP_V_ES_BUF

	struct task_struct *tsk;	//dump VES task handle
	unsigned int rd_ptr;		//dump VES read pointer
#endif

} g_v_dump_es_mgr =
{
	.enable = 0,
	.fname = {0},
	.fhandle = NULL,
#ifdef CONFIG_MT_DUMP_V_ES_BUF
	.tsk = NULL,
	.rd_ptr = 0,
#endif
};

int vdec_dump_es_get_enable(void)
{
	return g_v_dump_es_mgr.enable;
}

char *vdec_dump_es_get_file_name(void)
{
	return g_v_dump_es_mgr.fname;
}

void vdec_dump_es_set_enable(char *file_name)
{
	int ret;

	if (file_name != NULL && file_name[0] != 0)
	{
		strncpy(g_v_dump_es_mgr.fname, file_name, 256);
		g_v_dump_es_mgr.fname[255] = '\0';
	}
	else
	{
		strncpy(g_v_dump_es_mgr.fname, CFG_DUMP_V_ES_BUF_FILE, 256);
		g_v_dump_es_mgr.fname[255] = '\0';
	}

	//open file
	ret = down_killable(&kVDebugMutex);
	if (ret != 0)
	{
		PRINTF("[VDEC-DRV]Error, down_killable failed! return %d.\n",ret);
		return;
	}

	g_v_dump_es_mgr.enable = 1;

	if (g_v_dump_es_mgr.fhandle == NULL)
	{
		g_v_dump_es_mgr.fhandle = open_file(g_v_dump_es_mgr.fname, O_RDWR|O_CREAT|O_TRUNC, 0);
	}
	up(&kVDebugMutex);
}

void vdec_dump_es_set_disable(void)
{
	g_v_dump_es_mgr.enable = 0;

	vdec_dump_es_stop();
}

//----------------------------------------------------------------------------//
//Dump ES Data Thread
//Only for DVB/TS play mode.

//#define CONFIG_MT_DUMP_V_ES_BUF
#ifdef CONFIG_MT_DUMP_V_ES_BUF

//read VES buffer
static unsigned int ves_buf_read(VDEC_CHANNEL_S *pstChan, unsigned char **pData, unsigned int uLen)
{
#define WR_MASK		0x7FFFFFFF
	unsigned int wr_ptr = 0;
	unsigned int rd_size = 0;
//File ES Play
#if 0
	int ves_ch = (int)(pstChan->ves_buffer_channel_id & 0x0F);
	unsigned int es_addr = (unsigned int)ves_buffer_inst[ves_ch].pu8KnlVirAddr; //kernel virtual address!
	unsigned int es_size = ves_buffer_inst[ves_ch].u32Size;
#else
//DMX TS Play
	//unsigned int es_addr = pstChan->u32VidBufKerVirAddr;
	unsigned int es_addr = VFMW_REG_READ(REG_DMX_CH_0_START_ADDR) & 0xFFFFFFF8;
	unsigned int es_size = pstChan->u32DmxBufSize;
#endif

	if (es_addr == 0 || es_size == 0)
		return 0;

#if 1
	es_addr = (unsigned int)phys_to_virt((phys_addr_t)es_addr);
#endif

	//FIXME: support ch0 only!
	wr_ptr = VFMW_REG_READ(REG_VDEC_CH_0_WR_PTR);
	wr_ptr &= WR_MASK;
	wr_ptr = wr_ptr << 3;

	if (g_v_dump_es_mgr.rd_ptr < wr_ptr)
	{
		rd_size = wr_ptr - g_v_dump_es_mgr.rd_ptr;
	}
	else if (g_v_dump_es_mgr.rd_ptr > wr_ptr)
	{
		rd_size = es_size - g_v_dump_es_mgr.rd_ptr;
	}
	else
	{
		rd_size = 0;
	}

	if (rd_size > 0 && uLen > 0)
	{
		//if (rd_size > 0x4000)
		//	rd_size = 0x4000;
		if (rd_size > uLen)
			rd_size = uLen;

		*pData = (unsigned char*)(es_addr + g_v_dump_es_mgr.rd_ptr);
		PRINTF("%s: data=%p, size=%u\n", *pData, rd_size);
	}

	return rd_size;
}

//update read VES buffer(read uLen bytes of ES data complete)
static int ves_buf_read_update(VDEC_CHANNEL_S *pstChan, unsigned char *pData, unsigned int uLen)
{
//File ES Play
#if 0
	int ves_ch = (int)(pstChan->ves_buffer_channel_id & 0x0F);
	unsigned int es_size = ves_buffer_inst[ves_ch].u32Size;
#else
//DMX TS Play
	unsigned int es_size = pstChan->u32DmxBufSize;
#endif

	g_v_dump_es_mgr.rd_ptr += uLen;
	if (g_v_dump_es_mgr.rd_ptr >= es_size)
		g_v_dump_es_mgr.rd_ptr = 0;

	PRINTF("%s: len=%u, rd=%u\n", uLen, g_v_dump_es_mgr.rd_ptr);

	return 0;
}

//dump VES thread
static int dump_es_thread(void *data)
{
#define WR_MASK		0x7FFFFFFF
	//int ch = (int)data;
	VDEC_CHANNEL_S *pstChan = (VDEC_CHANNEL_S*)data;
	unsigned char *pData = NULL;
	unsigned int rd_size = 0;
	int ret = 0;

	PRINTF("[VDEC-DRV]dump video es trhead run!\n");
	do
	{
		rd_size = 0x4000;
		rd_size = ves_buf_read(pstChan, &pData, rd_size);

		if (rd_size > 0)
		{
			ret = vdec_dump_es_write((char*)pData, (int)rd_size);
			if (ret < 0)
			{
				PRINTF("[VDEC-DRV]Error, write video es(%u, %u) failed, return %d!\n",g_v_dump_es_mgr.rd_ptr,rd_size,ret);
			}

			ret = ves_buf_read_update(pstChan, pData, rd_size);
		}

		msleep(4);

	} while (!kthread_should_stop());

	PRINTF("[VDEC-DRV]dump video es trhead exit!\n");
	return 0;
}

//start dump VES(create&run the dump thread)
static int vdec_dump_es_start(mt_handle hVdec, VDEC_CHANNEL_S *pstChan)
{
	if (g_v_dump_es_mgr.enable)
	{
		g_v_dump_es_mgr.rd_ptr = 0;
		g_v_dump_es_mgr.tsk = kthread_run(dump_es_thread, (void*)pstChan, "DUMP_VES");
		if (IS_ERR(g_v_dump_es_mgr.tsk))
		{
			PRINTF("[VDEC-DRV]Error, create DUMP_V_CH_ES(%d) kthread failed!\n",hVdec);
			return (-1);
		}
		else
		{
			PRINTF("[VDEC-DRV]create DUMP_V_CH_ES(%d) ktrhead ok!\n",hVdec);
		}
	}
	return 0;
}
#endif

//stop dump VES(stop the dump thread, and close the dump file)
static int vdec_dump_es_stop(void)
{
	int ret = 0;

#ifdef CONFIG_MT_DUMP_V_ES_BUF
	PRINTF("[VDEC-DRV]stop dump video es trhead!\n");
	if (g_v_dump_es_mgr.tsk != NULL
		&& !IS_ERR(g_v_dump_es_mgr.tsk))
	{
		ret = kthread_stop(g_v_dump_es_mgr.tsk);
		g_v_dump_es_mgr.tsk = NULL;
	}
#endif

	ret = down_killable(&kVDebugMutex);
	if (ret != 0)
	{
		PRINTF("[VDEC-DRV]Error, down_killable failed! return %d.\n",ret);
		return (-1);
	}

	if (g_v_dump_es_mgr.fhandle != NULL)
	{
		ret |= sync_file(g_v_dump_es_mgr.fhandle);
		ret |= close_file(g_v_dump_es_mgr.fhandle);
		g_v_dump_es_mgr.fhandle = NULL;
	}
	up(&kVDebugMutex);

	return ret;
}

//write VES data to dump file
int vdec_dump_es_write(char* pbuf, int len)
{
	int ret = 0;

	if (g_v_dump_es_mgr.enable == 0)
		return 0;

	ret = down_killable(&kVDebugMutex);
	if (ret != 0)
	{
		PRINTF("[VDEC-DRV]Error, down_killable failed! return %d.\n",ret);
		return 0;
	}

	if (g_v_dump_es_mgr.fhandle == NULL)
	{
#if 0
		g_v_dump_es_mgr.fhandle = open_file(g_v_dump_es_mgr.fname, O_RDWR|O_CREAT|O_TRUNC, 0);
		if (g_v_dump_es_mgr.fhandle == NULL)
		{
			PRINTF("[VDEC-DRV]Error, open file(%s) failed!\n",g_v_dump_es_mgr.fname);
			return (-1);
		}
		else
		{
			PRINTF("[VDEC-DRV]open dump file(%s) ok!\n",g_v_dump_es_mgr.fname);
		}
#endif
	}
	else
	{
		ret = write_file(g_v_dump_es_mgr.fhandle, pbuf, len);
	}
	up(&kVDebugMutex);

	return ret;
}

/* Video ES Bit-Error-Ratio Disturb */
int vdec_ber_disturb(char* pbuf, int len)
{
#define MAX_DISTURB_DATA_LEN	128					//max random data length
	unsigned int r_data[MAX_DISTURB_DATA_LEN/4];	//random data
	unsigned int r_offset;							//random offset
	unsigned int r_len;								//random data length

	if (pbuf != NULL && len >= MAX_DISTURB_DATA_LEN)
	{
		get_random_bytes(r_data, sizeof(r_data));
		get_random_bytes(&r_offset, sizeof(unsigned int));
		get_random_bytes(&r_len, sizeof(unsigned int));

		r_offset = r_offset % (len-4);
		r_len = (r_len % MAX_DISTURB_DATA_LEN) + 1;

		r_len = (r_len <= (len-r_offset))?r_len:(len-r_offset);

		PRINTF("off %u, len %u\n",r_offset,r_len);
		memcpy(pbuf + r_offset, r_data, r_len);
	}

	return 0;
}

//hVdec: VDEC handle
int vdec_debug_start(mt_handle hVdec, VDEC_CHANNEL_S *pstChan)
{
	//open debug log file
/*	if (g_VDebugEnable)
	{
		spin_lock(&kVDebugLock);
		gp_VDebugLogFile = open_file(CFG_DEBUG_V_LOG_FILE, O_RDWR|O_CREAT|O_TRUNC, 0);
		spin_unlock(&kVDebugLock);
	}
*/
    /* Reset & dump ES Stat */
	
    vdec_dump_es_stat(MT_INVALID_HANDLE, NULL);
    vdec_dump_es_stat(hVdec, pstChan);

#ifdef CONFIG_MT_DUMP_V_ES_BUF
	//DMX
	if (pstChan->hDmxVidChn != MT_INVALID_HANDLE)
	{
		vdec_dump_es_start(hVdec, pstChan);
	}
	else	//FILE
	{
	}
#endif

	return 0;
}

int vdec_debug_stop(void)
{	
    /* Reset & dump ES Stat */
    vdec_dump_es_stat(MT_INVALID_HANDLE, NULL);

	//vdec_dump_es_stop();

	//close debug log file
	spin_lock(&kVDebugLock);
	if (gp_VDebugLogFile != NULL)
	{
		sync_file(gp_VDebugLogFile);
		close_file(gp_VDebugLogFile);
		gp_VDebugLogFile = NULL;
	}
	spin_unlock(&kVDebugLock);

	return 0;
}

