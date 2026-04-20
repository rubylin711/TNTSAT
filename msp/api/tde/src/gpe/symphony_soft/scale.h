/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifdef CONFIG_MT_FPGA_GPE
#define COEF coeff_0
#define FLI_FRA 8
#define FAST_2D_SCALE
#define SYMPHONY6_TEST

void    scale_vb(u8 *pRGB_src,int src_op_x,int src_op_y, int src_op_w,int src_op_h, int src_width, int src_height,
                 u8 *pRGB_dst, int dst_width,   int dst_height, scale_cfg_t *p_scale, 
                 u8 *key_src,  int key_stride, u8 *key_dst, u8 *msk_dst, int edge_option);
void    scale_hb(u8 *pRGB_src,int src_op_x,int src_op_y,int src_op_w,int src_op_h,int src_width, int src_height,
                 u8 *pRGB_dst, int dst_width, int dst_height, scale_cfg_t *p_scale,
                 u8 *key_src, int key_stride, u8 *key_dst, u8 *msk_dst, int edge_option);
void    scale_hl(u8 *pRGB_src,int src_op_x,int src_op_y,int src_op_w,int src_op_h,int src_width, int src_height,    
                 u8 *pRGB_dst, int dst_width,   int dst_height, scale_cfg_t *p_scale, 
                 u8 *key_src, int key_stride, u8 *key_dst, u8 *msk_dst, int edge_option);
#ifdef FAST_2D_SCALE
void	scale_fast(u8	*pRGB_src,int src_op_x,int src_op_y,int src_op_w,int src_op_h, int src_width, int src_height,	
				 u8	*pRGB_dst, int dst_width,	int	dst_height, scale_cfg_t *p_scale,
				 u8	*key_src, u8	*key_dst,int key_stride, int edge_option);
#endif
#endif
