#ifndef __TDE_PROC_H__
#define __TDE_PROC_H__

#ifdef __KERNEL__
#include "tde_hal.h"
#include "tde_hal_aria_vsb.h"
#endif

#include "tde_config.h"

#ifndef CONFIG_TDE_PROC_DISABLE


typedef struct mtTDE_REG_PROC_S
{
  mt_u32 u32RegAddr;
  mt_u32 u32RegVal;
}MT_TDE_REG_PROC_S;

typedef struct mtTDE_IMG_PROC_S
{
    /*!
      comments
      */
    mt_u32 buf;
    /*!
      unit:byte
      */
    mt_u32 pitch;
    /*!
      unit:pixel
      */
    mt_u32 width;
    /*!
      unit:pixel
      */
    mt_u32 height;
    /*!
      comments
      */
    rect_vsb_t rect;
    /*!
      comments
      */
    pix_fmt_t pix_format;
    /*!
      color key
      */
    MT_BOOL ck_en;
    /*!
      comments
      */
    //mt_u32 key_color;
    mt_u32 ck_min;
    mt_u32 ck_max;

    /*!
      added in symphony, new colorkey rule
      */       
    mt_u32               key_color_mod;
    /*!
      added in symphony, new colorkey rule
      */       
    mt_u32 key_color_select;  
}MT_TDE_IMG_PROC_S;

typedef struct mtTDE_PROC_PARAM_S
{
   MT_BOOL clip_en;   
   /*!
     comments
     */
   MT_BOOL no_dithering;
   /*!
     comments
     */
   MT_BOOL src_img_en;
   /*!
     comments
     */
   MT_BOOL ex_img_en;
   /*!
     comments
     */
   MT_BOOL bg_img_en;
   /*!
     comments
     */
   MT_TDE_IMG_PROC_S src_img;
   /*!
     src2(background) image
     */
   MT_TDE_IMG_PROC_S dst_img;
   /*!
     comments
     */
   MT_TDE_IMG_PROC_S ex_img;
   /*!
     comments
     */
   MT_TDE_IMG_PROC_S bg_img;    
  
  /*!
     comments
  */
   gpe_ops_t gpe_op;
   /*!
     
    VG_PAINT_TYPE_COLOR,
    VG_PAINT_TYPE_LINEAR_GRADIENT,
    VG_PAINT_TYPE_RADIAL_GRADIENT,
    VG_PAINT_TYPE_ELLIPSE_GRADIENT,
    VG_PAINT_TYPE_PATTERN,
     */
   mt_u32 paint_type;
   /*!
     comments
     */
   alpha_map_mod_t alpha_map_mod;
   /*!
     blend cfg
     */
   blend_cfg_t blend;
   /*!
    rop cfg
     */
   rop_cfg_t rop;
   /*!
     comments
     */
   rotator_op_t rotator_op;
   /*!
     blend cfg for alpha
     */
   blend_cfg_t blend_alpha;   

}MT_TDE_PROC_PARAM_S;

#define TDE_REG_MAX (0x400)

typedef struct mtTDE_PROC_INFO_S
{
  MT_TDE_PROC_PARAM_S tde_param;
  MT_TDE_REG_PROC_S tde_reg[TDE_REG_MAX / 4];
  MT_BOOL timeout;
  mt_u32 axi_state;
  mt_u32 tde_state;
}MT_TDE_PROC_INFO_S;


#ifdef __KERNEL__

#define TDE_MAX_PROC_NUM 8
/*****************************************************************************
* Function:      TDEProcRecordNode
* Description:   Record TDE Node configure information
* Input:         null
* Output:        none
* Return:        none
* Others:        none
*****************************************************************************/
mt_void TDEProcRecordNode(TDE_HWNode_S* pHWNode);

int tde_read_proc(struct seq_file *p, mt_void *v);

int tde_write_proc(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos);  
mt_void TDEProcSetInfo(MT_TDE_PROC_INFO_S *pProc);
mt_void TDE_Proc_init(mt_void);
mt_void TDE_Proc_Cleanup(mt_void);


#endif
#endif




#endif
