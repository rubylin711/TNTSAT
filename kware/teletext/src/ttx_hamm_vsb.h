/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __TTX_HAMM_VSB_H__
#define __TTX_HAMM_VSB_H__

/*!
   ABC

   \param[in] c
  */
void vbi_par_vsb(u8 *p_p, u32 n);
/*!
   ABC

   \param[in] c
  */
s32 vbi_unpar_vsb(u8 *p_p, u32 n);

/*!
   ABC

   \param[in] c
  */
void vbi_ham24p_vsb(u8 *p_p, u32 c);

/*!
   ABC

   \param[in] c
  */
s32 vbi_unham24p_vsb(const u8 *p_p);
#endif
