/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VBI_INSERTER_ARIA_H__
#define __VBI_INSERTER_ARIA_H__
/*!
  comments
  */
typedef struct
{
  mt_u8 *p_vbibuffer_head;
  mt_u8 *p_vbibuffer_tail;
  mt_u8 *p_vbibuffer_rdp;
  mt_u8 *p_vbibuffer_wrp;
  mt_u32 vbibuffer_size;
}vbi_buf_t;

mt_u32 vbi_parse_ESpacket(mt_u8 *pBuffer,mt_u32 Len);
mt_u32 vbi_parse_PEStoES(mt_u8 *pesBuffer, mt_u32 pesLen, mt_u8 *esBuffer, mt_u8 *esLen);
mt_u8 vbi_parse_TStoPES(mt_u8 * one_ts);
void PTI_CallBackTs2PES(
    mt_u8 *buff,
    mt_u16 len);
mt_u32 PTI_VBI_TS_CallBack(
  mt_u8 *pBUffer,
  mt_u16 Len);
void vbi_rp_reset(void);

void vbi_isr(void);
void vbi_sw_ttx_isr(void);
void vbi_sw_cc_isr(void);
void vbi_vps_isr(void);
void vbi_wss_isr(void);
mt_s32 aria_vbi_inserter_attach(char *p_name);
#endif

