/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _HAL_DMA_REGS_H
#define _HAL_DMA_REGS_H

 /************************************************************************
 * Defination of Reset register
 ************************************************************************/
/*!
  comments
  */
#define R_RST_REQ(n)                              (0xBF510000 + (n) * (0x4))
/*!
  comments
  */
#define R_RST_REQ_AO                            0xBF150040
/*!
  comments
  */
#define R_RST_CTRL(n)                             (0xBF510010 + (n) * (0x4))
/*!
  comments
  */
#define R_RST_CTRL_AO                            0xBF150044
/*!
  comments
  */
#define R_RST_ALLOW(n)                          (0xBF510020 + (n) * (0x4))
/*!
  comments
  */
#define R_RST_ALLOW_AO                        0xBF150048

/************************************************************************
 * DMA Plus Controller
 ************************************************************************/
/*!
  comments
  */
#define R_DMA_PLUS_BASE_ADDR    SYMPHONY_IO_VA(0xBF400000)
/*!
  comments
  */
#define R_DMA_PLUS_INT_STATUS_M    		(0)
/*!
  comments
  */
#define R_DMA_PLUS_INT_NODE_STATUS_M    (0x4)
/*!
  comments
  */
#define R_DMA_PLUS_INT_LINK_STATUS_M    (0x8)
/*!
  comments
  */
#define R_DMA_PLUS_INT_STATUS_S    		(0xC)
/*!
  comments
  */
#define R_DMA_PLUS_INT_NODE_STATUS_S    (0x10)
/*!
  comments
  */
#define R_DMA_PLUS_INT_LINK_STATUS_S    (0x14)
/*!
  comments
  */
#define R_DMA_PLUS_RAW_INT_STATUS_M    	(0x18)
/*!
  comments
  */
#define R_DMA_PLUS_RAW_NODE_STATUS_M    (0x1C)
/*!
  comments
  */
#define R_DMA_PLUS_RAW_LINK_STATUS_M    (0x20)
/*!
  comments
  */
#define R_DMA_PLUS_RAW_INT_STATUS_S    	(0x24)
/*!
  comments
  */
#define R_DMA_PLUS_RAW_NODE_STATUS_S    (0x28)
/*!
  comments
  */
#define R_DMA_PLUS_RAW_LINK_STATUS_S    (0x2C)
/*!
  comments
  */
#define R_DMA_PLUS_CG_MODE    			(0x38)
/*!
  comments
  */
#define R_DMA_PLUS_SOFT_RST_EN    		(0x50)
/*!
  comments
  */
#define R_DMA_PLUS_PRI_SET    			(0x54)
/*!
  comments
  */
#define R_DMA_PLUS_CHN_LLN    			(0x80)
/*!
  comments
  */
#define R_DMA_PLUS_CHN_SRC_ADDR    		(0x84)
/*!
  comments
  */
#define R_DMA_PLUS_CHN_DST_ADDR    		(0x88)
/*!
  comments
  */
#define R_DMA_PLUS_CHN_SRC_BLOCK    	(0x8C)
/*!
  comments
  */
#define R_DMA_PLUS_CHN_DST_BLOCK    	(0x90)
/*!
  comments
  */
#define R_DMA_PLUS_CHN_CONFIG    		(0x94)
/*!
  comments
  */
#define R_DMA_PLUS_CHN_TRANSED_CNT    	(0x98)
/*!
  comments
  */
#define R_DMA_PLUS_CHN_CONTROL    		(0x9C)
/*!
  comments
  */
#define R_DMA_PLUS_CHN_ALU_WORD    		(0xA0)
/*!
  comments
  */
#define R_DMA_PLUS_CHN_STATUS    		(0xA4)

/*!
  comments
  */
#define R_DMA_PLUS_INT_MASK_M    		(0x3B8)

#ifdef __cplusplus
}
#endif

#endif /* _DMX_REGS_SYMPHONY_H */
