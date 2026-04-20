/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VBI_INSERTER_REG_H__
#define __VBI_INSERTER_REG_H__




/*!
  ABC
  */
#define VENC_VBI_REG_BASE                               (0x35000000)
/*!
  ABC
  */
#define SDENCODE_REG_BASE                               (0xBFD50000)
/*!
   Register: venc_reg18
   Initialization Value: 0x00000011
   Initialization Mask: 0x00000011
  */

/*
   |---------------------------------------------------------------|
   |3|3|2|2|2|2|2|2|2|2|2|2|1|1|1|1|1|1|1|1|1|1|0|0|0|0|0|0|0|0|0|0|
   |1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|
   |---------------------------------------------------------------|
   |                                                     |r|     |r|
   |                                                     |e|     |e|
   |                                                     |n|     |n|
   |                                                     |e|     |e|
   |                                                     |w|     |w|
   |                                                     |_|     |_|
   |                                                     |f|     |f|
   |                                                     |l|     |l|
   |                                                     |a|     |a|
   |                                                     |g|     |g|
   |                                                     |_|     |_|
   |                                                     |1|     |0|
   |---------------------------------------------------------------|
 */

/*!
  ABC
  */
#define VENC_VBI_REG18_REG                    ((VENC_VBI_REG_BASE) + 0x00000048)
/*!
  ABC
  */
#define VENC_VBI_REG18_INITVALUE                0x00000011
/*!
  ABC
  */
#define VENC_VBI_REG18_INITMASK                 0x00000011

/*!
  ABC
  */
#define VENC_VBI_REG18_RENEW_FLAG_1_MASK        0x00000010
/*!
  ABC
  */
#define VENC_VBI_REG18_RENEW_FLAG_1_SHIFT       4

/*!
  ABC
  */
#define VENC_VBI_REG18_RENEW_FLAG_0_MASK        0x00000001
/*!
  ABC
  */
#define VENC_VBI_REG18_RENEW_FLAG_0_SHIFT       0

/*!
   Register: venc_reg19
   Initialization Value: 0x00000000
   Initialization Mask: 0xffffffff
  */

/*
   |---------------------------------------------------------------|
   |3|3|2|2|2|2|2|2|2|2|2|2|1|1|1|1|1|1|1|1|1|1|0|0|0|0|0|0|0|0|0|0|
   |1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|
   |---------------------------------------------------------------|
   |                           vbi_data0                           |
   |---------------------------------------------------------------|
 */

/*!
  ABC
  */
#define VENC_VBI_REG19_REG                    ((VENC_VBI_REG_BASE) + 0x0000004c)
/*!
  ABC
  */
#define VENC_VBI_REG19_INITVALUE                0x00000000
/*!
  ABC
  */
#define VENC_VBI_REG19_INITMASK                 0xffffffff

/*!
  ABC
  */
#define VENC_VBI_REG19_VBI_DATA0_MASK           0xffffffff
/*!
  ABC
  */
#define VENC_VBI_REG19_VBI_DATA0_SHIFT          0

/*!
   Register: venc_reg20
   Initialization Value: 0x00000000
   Initialization Mask: 0xffffffff
  */

/*
   |---------------------------------------------------------------|
   |3|3|2|2|2|2|2|2|2|2|2|2|1|1|1|1|1|1|1|1|1|1|0|0|0|0|0|0|0|0|0|0|
   |1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|
   |---------------------------------------------------------------|
   |                           vbi_data1                           |
   |---------------------------------------------------------------|
 */

/*!
  ABC
  */
#define VENC_VBI_REG20_REG                    ((VENC_VBI_REG_BASE) + 0x00000050)
/*!
  ABC
  */
#define VENC_VBI_REG20_INITVALUE                0x00000000
/*!
  ABC
  */
#define VENC_VBI_REG20_INITMASK                 0xffffffff

/*!
  ABC
  */
#define VENC_VBI_REG20_VBI_DATA1_MASK           0xffffffff
/*!
  ABC
  */
#define VENC_VBI_REG20_VBI_DATA1_SHIFT          0

/*!
   Register: venc_reg21
   Initialization Value: 0x1796411d
   Initialization Mask: 0x3fffffff
  */

/*!
   |---------------------------------------------------------------|
   |3|3|2|2|2|2|2|2|2|2|2|2|1|1|1|1|1|1|1|1|1|1|0|0|0|0|0|0|0|0|0|0|
   |1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|
   |---------------------------------------------------------------|
   |   |     tt_level      |     wss_level     |     cc_level      |
   |---------------------------------------------------------------|
  */

/*!
  ABC
  */
#define VENC_VBI_REG21_REG                    ((VENC_VBI_REG_BASE) + 0x00000054)
/*!
  ABC
  */
#define VENC_VBI_REG21_INITVALUE                0x1796411d
/*!
  ABC
  */
#define VENC_VBI_REG21_INITMASK                 0x3fffffff

/*!
  ABC
  */
#define VENC_VBI_REG21_TT_LEVEL_MASK            0x3ff00000
/*!
  ABC
  */
#define VENC_VBI_REG21_TT_LEVEL_SHIFT           20

/*!
  ABC
  */
#define VENC_VBI_REG21_WSS_LEVEL_MASK           0x000ffc00
/*!
  ABC
  */
#define VENC_VBI_REG21_WSS_LEVEL_SHIFT          10

/*!
  ABC
  */
#define VENC_VBI_REG21_CC_LEVEL_MASK            0x000003ff
/*!
  ABC
  */
#define VENC_VBI_REG21_CC_LEVEL_SHIFT           0

/*!
   Register: venc_reg22
   Initialization Value: 0x008faaaa
   Initialization Mask: 0x71ffffff
  */

/*
   |---------------------------------------------------------------|
   |3|3|2|2|2|2|2|2|2|2|2|2|1|1|1|1|1|1|1|1|1|1|0|0|0|0|0|0|0|0|0|0|
   |1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|
   |---------------------------------------------------------------|
   | |  v  |     |c|               |                               |
   | |  b  |     |c|               |                               |
   | |  i  |     |_|               |                               |
   | |  _  |     |p|               |                               |
   | |  b  |     |a|               |                               |
   | |  i  |     |r|               |                               |
   | |  t  |     |i|  cc_clk_coef  |         tt_clk_in_reg         |
   | |  _  |     |t|               |                               |
   | |  o  |     |y|               |                               |
   | |  r  |     | |               |                               |
   | |  d  |     | |               |                               |
   | |  e  |     | |               |                               |
   | |  r  |     | |               |                               |
   |---------------------------------------------------------------|
 */

/*!
  ABC
  */
#define VENC_VBI_REG22_REG                    ((VENC_VBI_REG_BASE) + 0x00000058)
/*!
  ABC
  */
#define VENC_VBI_REG22_INITVALUE                0x008faaaa
/*!
  ABC
  */
#define VENC_VBI_REG22_INITMASK                 0x71ffffff

/*!
  ABC
  */
#define VENC_VBI_REG22_VBI_BIT_ORDER_MASK       0x70000000
/*!
  ABC
  */
#define VENC_VBI_REG22_VBI_BIT_ORDER_SHIFT      28

/*!
  ABC
  */
#define VENC_VBI_REG22_CC_PARITY_MASK           0x01000000
/*!
  ABC
  */
#define VENC_VBI_REG22_CC_PARITY_SHIFT          24

/*!
  ABC
  */
#define VENC_VBI_REG22_CC_CLK_COEF_MASK         0x00ff0000
/*!
  ABC
  */
#define VENC_VBI_REG22_CC_CLK_COEF_SHIFT        16

/*!
  ABC
  */
#define VENC_VBI_REG22_TT_CLK_IN_REG_MASK       0x0000ffff
/*!
  ABC
  */
#define VENC_VBI_REG22_TT_CLK_IN_REG_SHIFT      0




/*!
   Register: venc_reg23
   Initialization Value: 0x04c62002
   Initialization Mask: 0xffffffff
  */

/*!
   |---------------------------------------------------------------|
   |3|3|2|2|2|2|2|2|2|2|2|2|1|1|1|1|1|1|1|1|1|1|0|0|0|0|0|0|0|0|0|0|
   |1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|
   |---------------------------------------------------------------|
   |                        vbi_subcarrier                         |
   |---------------------------------------------------------------|
  */

/*!
  ABC
  */
#define VENC_VBI_REG23_REG                    ((VENC_VBI_REG_BASE) + 0x0000005c)
/*!
  ABC
  */
#define VENC_VBI_REG23_INITVALUE                0x04c62002
/*!
  ABC
  */
#define VENC_VBI_REG23_INITMASK                 0xffffffff

/*!
  ABC
  */
#define VENC_VBI_REG23_VBI_SUBCARRIER_MASK      0xffffffff
/*!
  ABC
  */
#define VENC_VBI_REG23_VBI_SUBCARRIER_SHIFT     0








/*!
  ABC
  */
#define VBI_REG_RENEW_FLAG          VENC_VBI_REG18_REG
/*!
  ABC
  */
#define VBI_RENEW_FLAG_0            VENC_VBI_REG18_RENEW_FLAG_0_MASK
/*!
  ABC
  */
#define VBI_RENEW_FLAG_1            VENC_VBI_REG18_RENEW_FLAG_1_MASK




/*!
  ABC
  */
#define VBI_REG_FIFO_0              VENC_VBI_REG19_REG
/*!
  ABC
  */
#define VBI_REG_FIFO_1              VENC_VBI_REG20_REG




/*!
  ABC
  */
#define VBI_REG_VBI_LEVEL           VENC_VBI_REG21_REG
/*!
  ABC
  */
#define VBI_CC_LEVEL_SHIFT          VENC_VBI_REG21_CC_LEVEL_SHIFT
/*!
  ABC
  */
#define VBI_CC_LEVEL_MASK           VENC_VBI_REG21_CC_LEVEL_MASK

/*!
  ABC
  */
#define VBI_WSS_LEVEL_SHIFT         VENC_VBI_REG21_WSS_LEVEL_SHIFT
/*!
  ABC
  */
#define VBI_WSS_LEVEL_MASK          VENC_VBI_REG21_WSS_LEVEL_MASK

/*!
  ABC
  */
#define VBI_TTX_LEVEL_SHIFT         VENC_VBI_REG21_TT_LEVEL_SHIFT
/*!
  ABC
  */
#define VBI_TTX_LEVEL_MASK          VENC_VBI_REG21_TT_LEVEL_MASK




/*!
  ABC
  */
#define VBI_REG_SEND_CONFIG         VENC_VBI_REG22_REG
/*!
  ABC
  */
#define VBI_SEND_TTX_CLK_SHIFT      VENC_VBI_REG22_TT_CLK_IN_REG_SHIFT
/*!
  ABC
  */
#define VBI_SEND_TTX_CLK_MASK       VENC_VBI_REG22_TT_CLK_IN_REG_MASK

/*!
  ABC
  */
#define VBI_SEND_CC_CLK_SHIFT       VENC_VBI_REG22_CC_CLK_COEF_SHIFT
/*!
  ABC
  */
#define VBI_SEND_CC_CLK_MASK        VENC_VBI_REG22_CC_CLK_COEF_MASK

/*!
  ABC
  */
#define VBI_SEND_CC_PARITY_CODING   VENC_VBI_REG22_CC_PARITY_MASK

/*!
  ABC
  */
#define VBI_SEND_CC_BIT_ORDER_SWAP      \
    (VENC_VBI_REG22_VBI_BIT_ORDER_MASK  \
        & (1 << (VENC_VBI_REG22_VBI_BIT_ORDER_SHIFT + 0)))
/*!
  ABC
  */
#define VBI_SEND_WSS_BIT_ORDER_SWAP     \
    (VENC_VBI_REG22_VBI_BIT_ORDER_MASK  \
        & (1 << (VENC_VBI_REG22_VBI_BIT_ORDER_SHIFT + 1)))
/*!
  ABC
  */
#define VBI_SEND_TTX_BIT_ORDER_SWAP     \
    (VENC_VBI_REG22_VBI_BIT_ORDER_MASK  \
        & (1 << (VENC_VBI_REG22_VBI_BIT_ORDER_SHIFT + 2)))



/*!
  ABC
  */
#define VBI_REG_CC_SUBCARRIER       VENC_VBI_REG23_REG

#endif

