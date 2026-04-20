//***************************************************************************
//! @file     si_vidpath_regs.h
//! @brief    Deco register definition header.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Deco, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2014, Deco, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_VIDPATH_REGS_H__
#define __SI_VIDPATH_REGS_H__

#include "si_datatypes.h"
#include "si_drv_cra_api.h"

void SiiModTxVideoPathRegRead(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t *ptrdata, size_t rSize);
void SiiModTxVideoPathRegWrite(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t *ptrdata, size_t rSize);

#if __HDMI_OS_LINUX__
	#define BASE_ADDRESS 0x3000
#else
	#define BASE_ADDRESS 0x0000
#endif

#if (MT_SDK_COMPILE_HDMI20 == 0)
typedef enum {
	REGTX_VP1 = (BASE_ADDRESS | 0xB00),
	REGTX_VP2 = (BASE_ADDRESS | 0xC00),
	REGTX_VP3 = (BASE_ADDRESS | 0xD00),
	REGTX_VIDGEN = (BASE_ADDRESS | 0xE00),
} Page_t;
#else
typedef enum {
	REGTX_VP1 = (BASE_ADDRESS | 0xB00),
	REGTX_VP2 = (BASE_ADDRESS | 0xC00),
	REGTX_VP3 = (BASE_ADDRESS | 0xD00),
	REGTX_VIDGEN = (BASE_ADDRESS | 0xE00),
} Page_Vp_t;
#endif

//***************************************************************************
// REGTX_VP1. Address: 40
// Module features
#define REG_ADDR__VP__FEATURES                                           (REGTX_VP1 | 0x0000)
// (Undefined, Bits 27)
// Output pin muxing
#define BIT_MSK__VP__FEATURES__VO_MUXING                                             0x8000000
// (Undefined, Bits 26)
// Output embedded sync encoder
#define BIT_MSK__VP__FEATURES__VO_EMBD_SYNC_ENC                                      0x4000000
// (Undefined, Bits 25)
// Output blanking control
#define BIT_MSK__VP__FEATURES__VO_BLANK                                              0x2000000
// (Undefined, Bits 24)
// Output rate conversion
#define BIT_MSK__VP__FEATURES__VO_RATECONV                                           0x1000000
// (Undefined, Bits 23)
// Pixel capture at the output
#define BIT_MSK__VP__FEATURES__PIXCAP_OUT                                            0x800000
// (Undefined, Bits 22)
// Range Clip
#define BIT_MSK__VP__FEATURES__RANGE_CLIP                                            0x400000
// (Undefined, Bits 21)
// Dither/Round
#define BIT_MSK__VP__FEATURES__DITHER_RND                                            0x200000
// (Undefined, Bits 20)
// Chroma vertical subsampler
#define BIT_MSK__VP__FEATURES__C422_C420                                             0x100000
// (Undefined, Bits 19)
// Chroma horizontal subsampler
#define BIT_MSK__VP__FEATURES__C444_C422                                             0x80000
// (Undefined, Bits 18)
// Multi-Colorspace Converter #1
#define BIT_MSK__VP__FEATURES__MULTI_CSC1                                            0x40000
// (Undefined, Bits 17)
// Pixel capture after CMS
#define BIT_MSK__VP__FEATURES__PIXCAP_POST                                           0x20000
// (Undefined, Bits 16)
// PWLI #2
#define BIT_MSK__VP__FEATURES__PWLI2                                                 0x10000
// (Undefined, Bits 15)
// PWLI #1
#define BIT_MSK__VP__FEATURES__PWLI1                                                 0x8000
// (Undefined, Bits 14)
// CMS Matrix
#define BIT_MSK__VP__FEATURES__CMS_MATRIX                                            0x4000
// (Undefined, Bits 13)
// PWLI #0
#define BIT_MSK__VP__FEATURES__PWLI0                                                 0x2000
// (Undefined, Bits 12)
// Pixel capture before CMS
#define BIT_MSK__VP__FEATURES__PIXCAP_PRE                                            0x1000
// (Undefined, Bits 11)
// Multi-Colorspace Converter #0
#define BIT_MSK__VP__FEATURES__MULTI_CSC0                                            0x800
// (Undefined, Bits 10)
// Chroma horizontal upsampler
#define BIT_MSK__VP__FEATURES__C422_C444                                             0x400
// (Undefined, Bits 9)
// Chroma vertical upsampler
#define BIT_MSK__VP__FEATURES__C420_C422                                             0x200
// (Undefined, Bits 8)
// Pixel capture at the input
#define BIT_MSK__VP__FEATURES__PIXCAP_IN                                             0x100
// (Undefined, Bits 7)
// DE Generator
#define BIT_MSK__VP__FEATURES__DEGEN                                                 0x80
// (Undefined, Bits 6)
// 656 Decoder
#define BIT_MSK__VP__FEATURES__DEC656                                                0x40
// (Undefined, Bits 5)
// Retiming VTG/FIFO
#define BIT_MSK__VP__FEATURES__VI_RETIMING                                           0x20
// (Undefined, Bits 4)
// Input rate conversion
#define BIT_MSK__VP__FEATURES__VI_RATECONV                                           0x10
// (Undefined, Bits 3)
// Input sync polarity adjustment
#define BIT_MSK__VP__FEATURES__VI_SYNC_ADJUST                                        0x08
// (Undefined, Bits 2)
// Input format detector
#define BIT_MSK__VP__FEATURES__VI_FDET                                               0x04
// (Undefined, Bits 1)
// Input pin muxing
#define BIT_MSK__VP__FEATURES__VI_MUXING                                             0x02
// (Undefined, Bits 0)
// Video Path Core module
#define BIT_MSK__VP__FEATURES__VIDEO_PATH_CORE                                       0x01

// Device build time stamp
#define REG_ADDR__VP__BUILD_TIME                                         (REGTX_VP1 | 0x0008)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__BUILD_TIME__                                                      0xFFFFFFFF

// Software reset
#define REG_ADDR__VP__SOFT_RESET                                         (REGTX_VP1 | 0x000C)
// (Undefined, Bits 2)
// Video Input software reset for backend only (clk_out domain)
#define BIT_MSK__VP__SOFT_RESET__RESET_CLK_OUT                                         0x04
// (Undefined, Bits 1)
// Video Input software reset for core only (clk_core domain)
#define BIT_MSK__VP__SOFT_RESET__RESET_CLK_CORE                                        0x02
// (Undefined, Bits 0)
// Video Input software reset for frontend only (clk_in domain)
#define BIT_MSK__VP__SOFT_RESET__RESET_CLK_IN                                          0x01

// Number of data bits for the datapath
#define REG_ADDR__VP__DATA_BITS_VALUE                                    (REGTX_VP1 | 0x000E)
// (Undefined, Bits 7:0)
//
#define BIT_MSK__VP__DATA_BITS_VALUE__                                                      0xFF

// Input muting
#define REG_ADDR__VP__INPUT_MUTE                                         (REGTX_VP1 | 0x0010)
// (Undefined, Bits 5)
// Disable Cr input pins
#define BIT_MSK__VP__INPUT_MUTE__CR_DISABLE                                            0x20
// (Undefined, Bits 4)
// Disable Cb input pins
#define BIT_MSK__VP__INPUT_MUTE__CB_DISABLE                                            0x10
// (Undefined, Bits 3)
// Disable Y input pins
#define BIT_MSK__VP__INPUT_MUTE__Y_DISABLE                                             0x08
// (Undefined, Bits 2)
// Disable DE input
#define BIT_MSK__VP__INPUT_MUTE__DE_DISABLE                                            0x04
// (Undefined, Bits 1)
// Disable hsync input
#define BIT_MSK__VP__INPUT_MUTE__HSYNC_DISABLE                                         0x02
// (Undefined, Bits 0)
// Disable vsync input
#define BIT_MSK__VP__INPUT_MUTE__VSYNC_DISABLE                                         0x01

// Input sync configuration
#define REG_ADDR__VP__INPUT_SYNC_CONFIG                                  (REGTX_VP1 | 0x0012)
// (Undefined, Bits 3)
// DE input is FIELD signal (requires DE generator to be enabled!)
#define BIT_MSK__VP__INPUT_SYNC_CONFIG__DE_IS_FIELD                                           0x08
// (Undefined, Bits 2)
// Select DE polarity
#define BIT_MSK__VP__INPUT_SYNC_CONFIG__DE_POLARITY                                           0x04
// (Undefined, Bits 1)
// Select HSYNC polarity
#define BIT_MSK__VP__INPUT_SYNC_CONFIG__HSYNC_POLARITY                                        0x02
// (Undefined, Bits 0)
// Select VSYNC polarity
#define BIT_MSK__VP__INPUT_SYNC_CONFIG__VSYNC_POLARITY                                        0x01

// Input Format Selection
#define REG_ADDR__VP__INPUT_FORMAT                                       (REGTX_VP1 | 0x0014)
// (Undefined, Bits 11)
// Select whether Cb or Cr bus are used as chroma input
#define BIT_MSK__VP__INPUT_FORMAT__MUX_CB_OR_CR                                          0x800
// (Undefined, Bits 10)
// Enable 4:2:0 Y mux
#define BIT_MSK__VP__INPUT_FORMAT__MUX_420_ENABLE                                        0x400
// (Undefined, Bits 9:8)
// Remove replicated pixels
#define BIT_MSK__VP__INPUT_FORMAT__PIXEL_RATE                                            0x300
// (Undefined, Bits 7)
// Select Cb/Cr order for muxed signals
#define BIT_MSK__VP__INPUT_FORMAT__CBCR_ORDER                                            0x80
// (Undefined, Bits 6)
// Select polarity of Y/C demux logic
#define BIT_MSK__VP__INPUT_FORMAT__YC_DEMUX_POLARITY                                     0x40
// (Undefined, Bits 5)
// Enable Y/C demux logic
#define BIT_MSK__VP__INPUT_FORMAT__YC_DEMUX_ENABLE                                       0x20
// (Undefined, Bits 4:2)
// Select DDR mode
#define BIT_MSK__VP__INPUT_FORMAT__DDR_MODE                                              0x1C
// (Undefined, Bits 1)
// Select polarity of DDR decoder
#define BIT_MSK__VP__INPUT_FORMAT__DDR_POLARITY                                          0x02
// (Undefined, Bits 0)
// Strobe data on both posedge and negedge of input clock
#define BIT_MSK__VP__INPUT_FORMAT__DDR_ENABLE                                            0x01

// Input pin mapping configuration
#define REG_ADDR__VP__INPUT_MAPPING                                      (REGTX_VP1 | 0x0016)
// (Undefined, Bits 11:9)
// Select input pins for internal Cr datapath
#define BIT_MSK__VP__INPUT_MAPPING__SELECT_CR                                             0xE00
// (Undefined, Bits 8:6)
// Select input pins for internal Cb datapath
#define BIT_MSK__VP__INPUT_MAPPING__SELECT_CB                                             0x1C0
// (Undefined, Bits 5:3)
// Select input pins for internal Y datapath
#define BIT_MSK__VP__INPUT_MAPPING__SELECT_Y                                              0x38
// (Undefined, Bits 2)
// Reverse input pins (11:0 <-> 0:11)
#define BIT_MSK__VP__INPUT_MAPPING__REVERSE_CR                                            0x04
// (Undefined, Bits 1)
// Reverse input pins (11:0 <-> 0:11)
#define BIT_MSK__VP__INPUT_MAPPING__REVERSE_CB                                            0x02
// (Undefined, Bits 0)
// Reverse input pins (11:0 <-> 0:11)
#define BIT_MSK__VP__INPUT_MAPPING__REVERSE_Y                                             0x01

// Input mask
#define REG_ADDR__VP__INPUT_MASK                                         (REGTX_VP1 | 0x0018)
// (Undefined, Bits 5:4)
// Disable LSBs on Cr datapath
#define BIT_MSK__VP__INPUT_MASK__DISABLE_LSBS_CR                                       0x30
// (Undefined, Bits 3:2)
// Disable LSBs on Cb datapath
#define BIT_MSK__VP__INPUT_MASK__DISABLE_LSBS_CB                                       0x0C
// (Undefined, Bits 1:0)
// Disable LSBs on Y datapath
#define BIT_MSK__VP__INPUT_MASK__DISABLE_LSBS_Y                                        0x03

// Configure sync polarity adjustment for datapath #0
#define REG_ADDR__VP__INPUT_SYNC_ADJUST_CONFIG                           (REGTX_VP1 | 0x001C)
// (Undefined, Bits 0)
// Configure sync polarity adjustment for datapath #0
#define BIT_MSK__VP__INPUT_SYNC_ADJUST_CONFIG__AUTO_DISABLE                                          0x01

// DE Generator configuration
#define REG_ADDR__VP__DEGEN_CONFIG                                       (REGTX_VP1 | 0x0020)
// (Undefined, Bits 0)
// Enable DE generator
#define BIT_MSK__VP__DEGEN_CONFIG__ENABLE                                                0x01

// Number of clocks from start of HSYNC before generated DE
#define REG_ADDR__VP__DEGEN_PIXEL_DELAY                                  (REGTX_VP1 | 0x0022)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__DEGEN_PIXEL_DELAY__                                                      0xFFFF

// Count (per line) for DE to be active (0=1 clk period, 1=2 clk periods, etc.)
#define REG_ADDR__VP__DEGEN_PIXEL_COUNT_MINUS_ONE                        (REGTX_VP1 | 0x0024)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__DEGEN_PIXEL_COUNT_MINUS_ONE__                                                      0xFFFF

// Number of lines from start of VSYNC before generated DE minus one (0=1 line, 1=2 lines, etc.)
#define REG_ADDR__VP__DEGEN_LINE_DELAY                                   (REGTX_VP1 | 0x0026)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__DEGEN_LINE_DELAY__                                                      0xFFFF

// Number of lines of generated DE
#define REG_ADDR__VP__DEGEN_LINE_COUNT                                   (REGTX_VP1 | 0x0028)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__DEGEN_LINE_COUNT__                                                      0xFFFF

// Output muting
#define REG_ADDR__VP__OUTPUT_MUTE                                        (REGTX_VP1 | 0x0040)
// (Undefined, Bits 7)
// Tristate video output pins
#define BIT_MSK__VP__OUTPUT_MUTE__TRISTATE                                              0x80
// (Undefined, Bits 6)
// Disable Cr/R output pins
#define BIT_MSK__VP__OUTPUT_MUTE__CR_DISABLE                                            0x40
// (Undefined, Bits 5)
// Disable Cb/B output pins
#define BIT_MSK__VP__OUTPUT_MUTE__CB_DISABLE                                            0x20
// (Undefined, Bits 4)
// Disable Y/G output pins
#define BIT_MSK__VP__OUTPUT_MUTE__Y_DISABLE                                             0x10
// (Undefined, Bits 3)
// Disable DE output
#define BIT_MSK__VP__OUTPUT_MUTE__DE_DISABLE                                            0x08
// (Undefined, Bits 2)
// Disable csync output
#define BIT_MSK__VP__OUTPUT_MUTE__CSYNC_DISABLE                                         0x04
// (Undefined, Bits 1)
// Disable hsync output
#define BIT_MSK__VP__OUTPUT_MUTE__HSYNC_DISABLE                                         0x02
// (Undefined, Bits 0)
// Disable vsync output
#define BIT_MSK__VP__OUTPUT_MUTE__VSYNC_DISABLE                                         0x01

// Output port configuration
#define REG_ADDR__VP__OUTPUT_SYNC_CONFIG                                 (REGTX_VP1 | 0x0042)
// (Undefined, Bits 4)
// Swap HSYNC and CSYNC output controls
#define BIT_MSK__VP__OUTPUT_SYNC_CONFIG__SWAP_HS_CS                                            0x10
// (Undefined, Bits 3)
// Select DE polarity
#define BIT_MSK__VP__OUTPUT_SYNC_CONFIG__DE_POLARITY                                           0x08
// (Undefined, Bits 2)
// Select CSYNC polarity
#define BIT_MSK__VP__OUTPUT_SYNC_CONFIG__CSYNC_POLARITY                                        0x04
// (Undefined, Bits 1)
// Select HSYNC polarity
#define BIT_MSK__VP__OUTPUT_SYNC_CONFIG__HSYNC_POLARITY                                        0x02
// (Undefined, Bits 0)
// Select VSYNC polarity
#define BIT_MSK__VP__OUTPUT_SYNC_CONFIG__VSYNC_POLARITY                                        0x01

// Output pin mapping configuration
#define REG_ADDR__VP__OUTPUT_MAPPING                                     (REGTX_VP1 | 0x0044)
// (Undefined, Bits 11:9)
// Select internal datapath for Cr output pins
#define BIT_MSK__VP__OUTPUT_MAPPING__SELECT_CR                                             0xE00
// (Undefined, Bits 8:6)
// Select internal datapath for Cb output pins
#define BIT_MSK__VP__OUTPUT_MAPPING__SELECT_CB                                             0x1C0
// (Undefined, Bits 5:3)
// Select internal datapath for Y output pins
#define BIT_MSK__VP__OUTPUT_MAPPING__SELECT_Y                                              0x38
// (Undefined, Bits 2)
// Reverse output pins (11:0 <-> 0:11)
#define BIT_MSK__VP__OUTPUT_MAPPING__REVERSE_CR                                            0x04
// (Undefined, Bits 1)
// Reverse output pins (11:0 <-> 0:11)
#define BIT_MSK__VP__OUTPUT_MAPPING__REVERSE_CB                                            0x02
// (Undefined, Bits 0)
// Reverse output pins (11:0 <-> 0:11)
#define BIT_MSK__VP__OUTPUT_MAPPING__REVERSE_Y                                             0x01

// Output masking
#define REG_ADDR__VP__OUTPUT_MASK                                        (REGTX_VP1 | 0x0046)
// (Undefined, Bits 5:4)
// Disable LSBs on Cr output pins
#define BIT_MSK__VP__OUTPUT_MASK__DISABLE_LSBS_CR                                       0x30
// (Undefined, Bits 3:2)
// Disable LSBs on Cb output pins
#define BIT_MSK__VP__OUTPUT_MASK__DISABLE_LSBS_CB                                       0x0C
// (Undefined, Bits 1:0)
// Disable LSBs on Y output pins
#define BIT_MSK__VP__OUTPUT_MASK__DISABLE_LSBS_Y                                        0x03

// Output format
#define REG_ADDR__VP__OUTPUT_FORMAT                                      (REGTX_VP1 | 0x0048)
// (Undefined, Bits 11)
// Select whether Cb or Cr bus are used as chroma output
#define BIT_MSK__VP__OUTPUT_FORMAT__DEMUX_CB_OR_CR                                        0x800
// (Undefined, Bits 10)
// Enable 4:2:0 Y demux
#define BIT_MSK__VP__OUTPUT_FORMAT__DEMUX_420_ENABLE                                      0x400
// (Undefined, Bits 9:8)
// Remove replicated pixels
#define BIT_MSK__VP__OUTPUT_FORMAT__PIXEL_RATE                                            0x300
// (Undefined, Bits 7)
// Select Cb/Cr order for muxed signals
#define BIT_MSK__VP__OUTPUT_FORMAT__CBCR_ORDER                                            0x80
// (Undefined, Bits 6)
// Select polarity of Y/C mux logic
#define BIT_MSK__VP__OUTPUT_FORMAT__YC_MUX_POLARITY                                       0x40
// (Undefined, Bits 5)
// Enable Y/C mux logic
#define BIT_MSK__VP__OUTPUT_FORMAT__YC_MUX_ENABLE                                         0x20
// (Undefined, Bits 4:2)
// Select DDR mode
#define BIT_MSK__VP__OUTPUT_FORMAT__DDR_MODE                                              0x1C
// (Undefined, Bits 1)
// Select polarity of DDR encoder
#define BIT_MSK__VP__OUTPUT_FORMAT__DDR_POLARITY                                          0x02
// (Undefined, Bits 0)
// Send data on both posedge and negedge of output clock
#define BIT_MSK__VP__OUTPUT_FORMAT__DDR_ENABLE                                            0x01

// First line of blanking in the active area, if enabled
#define REG_ADDR__VP__OUTPUT_BLANK_START_LINE                            (REGTX_VP1 | 0x004C)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__OUTPUT_BLANK_START_LINE__                                                      0xFFFF

// Last line of blanking in the active area, if enabled
#define REG_ADDR__VP__OUTPUT_BLANK_END_LINE                              (REGTX_VP1 | 0x004E)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__OUTPUT_BLANK_END_LINE__                                                      0xFFFF

// Configure blanking data
#define REG_ADDR__VP__OUTPUT_BLANK_CONFIG                                (REGTX_VP1 | 0x0050)
// (Undefined, Bits 5)
// Polarity of alternating Cb/Cr (in 4:2:0 mode only)
#define BIT_MSK__VP__OUTPUT_BLANK_CONFIG__CB_CR_POLARITY                                        0x20
// (Undefined, Bits 4)
// Select whether Cb or Cr bus are used as chroma data output (in 4:2:0 mode only)
#define BIT_MSK__VP__OUTPUT_BLANK_CONFIG__CB_OR_CR                                              0x10
// (Undefined, Bits 3:2)
// Select chroma mode
#define BIT_MSK__VP__OUTPUT_BLANK_CONFIG__CHROMA_MODE                                           0x0C
// (Undefined, Bits 1)
// Enable active data override
#define BIT_MSK__VP__OUTPUT_BLANK_CONFIG__ENABLE_ACTIVE_OVERRIDE                                0x02
// (Undefined, Bits 0)
// Enable blanking data override
#define BIT_MSK__VP__OUTPUT_BLANK_CONFIG__ENABLE_BLANKING_OVERRIDE                              0x01

// Use this value for Y during blanking when enabled
#define REG_ADDR__VP__OUTPUT_BLANK_Y                                     (REGTX_VP1 | 0x0052)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__OUTPUT_BLANK_Y__                                                      0xFFF

// Use this value for Cb during blanking when enabled
#define REG_ADDR__VP__OUTPUT_BLANK_CB                                    (REGTX_VP1 | 0x0054)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__OUTPUT_BLANK_CB__                                                      0xFFF

// Use this value for Cr during blanking when enabled
#define REG_ADDR__VP__OUTPUT_BLANK_CR                                    (REGTX_VP1 | 0x0056)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__OUTPUT_BLANK_CR__                                                      0xFFF

// Use this value for Y during active DE when enabled
#define REG_ADDR__VP__OUTPUT_ACTIVE_Y                                    (REGTX_VP1 | 0x0058)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__OUTPUT_ACTIVE_Y__                                                      0xFFF

// Use this value for Cb during active DE when enabled
#define REG_ADDR__VP__OUTPUT_ACTIVE_CB                                   (REGTX_VP1 | 0x005A)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__OUTPUT_ACTIVE_CB__                                                      0xFFF

// Use this value for Cr during active DE when enabled
#define REG_ADDR__VP__OUTPUT_ACTIVE_CR                                   (REGTX_VP1 | 0x005C)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__OUTPUT_ACTIVE_CR__                                                      0xFFF

// End of horizontal sync pulse -1
#define REG_ADDR__VP__VTG_HORIZONTAL_SYNC_END                            (REGTX_VP1 | 0x0060)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_HORIZONTAL_SYNC_END__                                                      0xFFFF

// Start of horizontal active video line -1
#define REG_ADDR__VP__VTG_HORIZONTAL_ACTIVE_VIDEO_START                  (REGTX_VP1 | 0x0062)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_HORIZONTAL_ACTIVE_VIDEO_START__                                                      0xFFFF

// Midpoint of horizontal line -2
#define REG_ADDR__VP__VTG_HALFLINE                                       (REGTX_VP1 | 0x0064)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_HALFLINE__                                                      0xFFFF

// End of horizontal active video line -1
#define REG_ADDR__VP__VTG_HORIZONTAL_ACTIVE_VIDEO_END                    (REGTX_VP1 | 0x0066)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_HORIZONTAL_ACTIVE_VIDEO_END__                                                      0xFFFF

// End of horizontal line -2
#define REG_ADDR__VP__VTG_END_OF_LINE                                    (REGTX_VP1 | 0x0068)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_END_OF_LINE__                                                      0xFFFF

// End of vertical sync pulse (in half-lines) -1
#define REG_ADDR__VP__VTG_VERTICAL_SYNC_END                              (REGTX_VP1 | 0x0070)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_VERTICAL_SYNC_END__                                                      0xFFFF

// Trigger point for processing to start (in half-lines) -1
#define REG_ADDR__VP__VTG_TRIGGER_START                                  (REGTX_VP1 | 0x0072)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_TRIGGER_START__                                                      0xFFFF

// Start of vertical active video (in half-lines) -1
#define REG_ADDR__VP__VTG_VERTICAL_ACTIVE_VIDEO_START                    (REGTX_VP1 | 0x0074)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_VERTICAL_ACTIVE_VIDEO_START__                                                      0xFFFF

// End of vertical active video (in half-lines) -1
#define REG_ADDR__VP__VTG_VERTICAL_ACTIVE_VIDEO_END                      (REGTX_VP1 | 0x0076)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_VERTICAL_ACTIVE_VIDEO_END__                                                      0xFFFF

// End of vertical frame (in half-lines) -1
#define REG_ADDR__VP__VTG_VERTICAL_END_OF_FRAME                          (REGTX_VP1 | 0x0078)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_VERTICAL_END_OF_FRAME__                                                      0xFFFF

// Configure VTG
#define REG_ADDR__VP__VTG_CONFIG                                         (REGTX_VP1 | 0x007A)
// (Undefined, Bits 3)
// Blank video data
#define BIT_MSK__VP__VTG_CONFIG__BLANK                                                 0x08
// (Undefined, Bits 2)
// Unlock VTG
#define BIT_MSK__VP__VTG_CONFIG__UNLOCK                                                0x04
// (Undefined, Bits 1)
// Force resync every frame
#define BIT_MSK__VP__VTG_CONFIG__RESYNC                                                0x02
// (Undefined, Bits 0)
// Enable VTG
#define BIT_MSK__VP__VTG_CONFIG__ENABLE                                                0x01

// Sync threshold
#define REG_ADDR__VP__VTG_THRESHOLD                                      (REGTX_VP1 | 0x007B)
// (Undefined, Bits 7:0)
//
#define BIT_MSK__VP__VTG_THRESHOLD__                                                      0xFF

// Number of cycles of delay between reference signal and VTG signal
#define REG_ADDR__VP__VTG_CYCLE_DELAY                                    (REGTX_VP1 | 0x007C)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__VTG_CYCLE_DELAY__                                                      0xFFFF

// Bank update request for Video Generator parameters. (write any data value - this is a strobe only)
#define REG_ADDR__VP__VTG_UPDATE_REQUEST                                 (REGTX_VP1 | 0x007E)
// (Undefined, Bits 0)
//
#define BIT_MSK__VP__VTG_UPDATE_REQUEST__                                                      0x01

// Read/write bank configuration for video generator parameters
#define REG_ADDR__VP__VTG_BANK_CONFIG                                    (REGTX_VP1 | 0x007F)
// (Undefined, Bits 2)
// Select when to update double-buffer
#define BIT_MSK__VP__VTG_BANK_CONFIG__UPDATE_MODE                                           0x04
// (Undefined, Bits 1)
// Select which bank to write
#define BIT_MSK__VP__VTG_BANK_CONFIG__WRITE_BANK                                            0x02
// (Undefined, Bits 0)
// Select which bank to read
#define BIT_MSK__VP__VTG_BANK_CONFIG__READ_BANK                                             0x01

// Format Detector #0 configuration
#define REG_ADDR__VP__FDET_CONFIG                                        (REGTX_VP1 | 0x0080)
// (Undefined, Bits 3)
// Enable 656 mode
#define BIT_MSK__VP__FDET_CONFIG__ENABLE_656                                            0x08
// (Undefined, Bits 2)
// Select forced VSYNC polarity
#define BIT_MSK__VP__FDET_CONFIG__VSYNC_POLARITY                                        0x04
// (Undefined, Bits 1)
// Select forced HSYNC polarity
#define BIT_MSK__VP__FDET_CONFIG__HSYNC_POLARITY                                        0x02
// (Undefined, Bits 0)
// Select which sync polarity to use
#define BIT_MSK__VP__FDET_CONFIG__SYNC_POLARITY_FORCE                                   0x01

// Format Detector status. Write any value to clear the status
#define REG_ADDR__VP__FDET_STATUS                                        (REGTX_VP1 | 0x0081)
// (Undefined, Bits 3)
// 656 signal detected
#define BIT_MSK__VP__FDET_STATUS__VIDEO656                                              0x08
// (Undefined, Bits 2)
// Interlaced signal detected
#define BIT_MSK__VP__FDET_STATUS__INTERLACED                                            0x04
// (Undefined, Bits 1)
// Detected VSYNC polarity (requires VSYNC/DE input active)
#define BIT_MSK__VP__FDET_STATUS__VSYNC_POLARITY                                        0x02
// (Undefined, Bits 0)
// Detected HSYNC polarity (requires HSYNC/DE input active)
#define BIT_MSK__VP__FDET_STATUS__HSYNC_POLARITY                                        0x01

// Delta Threshold. If frame rate counter differs by more than this amount between frames an IRQ is issued.
#define REG_ADDR__VP__FDET_FRAME_RATE_DELTA_THRESHOLD                    (REGTX_VP1 | 0x0084)
// (Undefined, Bits 23:0)
//
#define BIT_MSK__VP__FDET_FRAME_RATE_DELTA_THRESHOLD__                                                      0xFFFFFF

// Frame rate = (1 / fdet_frame_rate) * f_clk_reg. Write any value to clear the counter
#define REG_ADDR__VP__FDET_FRAME_RATE                                    (REGTX_VP1 | 0x0088)
// (Undefined, Bits 23:0)
//
#define BIT_MSK__VP__FDET_FRAME_RATE__                                                      0xFFFFFF

// Pixels per Line (requires DE input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_PIXEL_COUNT                                   (REGTX_VP1 | 0x008C)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_PIXEL_COUNT__                                                      0xFFFF

// Active video lines per frame (requires DE input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_LINE_COUNT                                    (REGTX_VP1 | 0x008E)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_LINE_COUNT__                                                      0xFFFF

// Number of HSYNC low cycles (requires HSYNC input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_HSYNC_LOW_COUNT                               (REGTX_VP1 | 0x0090)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_HSYNC_LOW_COUNT__                                                      0xFFFF

// Number of HSYNC high cycles (requires HSYNC input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_HSYNC_HIGH_COUNT                              (REGTX_VP1 | 0x0092)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_HSYNC_HIGH_COUNT__                                                      0xFFFF

// Number of HFRONT cycles (requires HSYNC/DE inputs active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_HFRONT_COUNT                                  (REGTX_VP1 | 0x0094)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_HFRONT_COUNT__                                                      0xFFFF

// Number of HBACK cycles (requires HSYNC/DE inputs active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_HBACK_COUNT                                   (REGTX_VP1 | 0x0096)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_HBACK_COUNT__                                                      0xFFFF

// Number of VSYNC low cycles in the EVEN field (or progressive frame) (requires VSYNC input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_EVEN                          (REGTX_VP1 | 0x0098)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_VSYNC_LOW_COUNT_EVEN__                                                      0xFFFF

// Number of VSYNC high cycles in the EVEN field (or progressive frame) (requires VSYNC input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_EVEN                         (REGTX_VP1 | 0x009A)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_VSYNC_HIGH_COUNT_EVEN__                                                      0xFFFF

// Number of VFRONT cycles in the EVEN field (or progressive frame) (requires VSYNC/DE inputs active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_VFRONT_COUNT_EVEN                             (REGTX_VP1 | 0x009C)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_VFRONT_COUNT_EVEN__                                                      0xFFFF

// Number of VBACK cycles in the EVEN field (or progressive frame) (requires VSYNC/DE input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_VBACK_COUNT_EVEN                              (REGTX_VP1 | 0x009E)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_VBACK_COUNT_EVEN__                                                      0xFFFF

// Number of VSYNC low cycles in the ODD field (requires VSYNC input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_VSYNC_LOW_COUNT_ODD                           (REGTX_VP1 | 0x00A0)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_VSYNC_LOW_COUNT_ODD__                                                      0xFFFF

// Number of VSYNC high cycles in the ODD field (requires VSYNC input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_VSYNC_HIGH_COUNT_ODD                          (REGTX_VP1 | 0x00A2)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_VSYNC_HIGH_COUNT_ODD__                                                      0xFFFF

// Number of VFRONT cycles in the ODD field (requires VSYNC/DE input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_VFRONT_COUNT_ODD                              (REGTX_VP1 | 0x00A4)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_VFRONT_COUNT_ODD__                                                      0xFFFF

// Number of VBACK cycles in the ODD field (requires VSYNC/DE input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_VBACK_COUNT_ODD                               (REGTX_VP1 | 0x00A6)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_VBACK_COUNT_ODD__                                                      0xFFFF

// Running frame count (requires VSYNC input active). Write any value to clear the counter
#define REG_ADDR__VP__FDET_FRAME_COUNT                                   (REGTX_VP1 | 0x00A8)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__FDET_FRAME_COUNT__                                                      0xFFFF

// Configure which format changes cause IRQ - each bit enables the corresponding bit in the IRQ status register to cause an interrupt
#define REG_ADDR__VP__FDET_IRQ_MASK                                      (REGTX_VP1 | 0x00B0)
// (Undefined, Bits 18)
// VBACK count changes (odd fields)
#define BIT_MSK__VP__FDET_IRQ_MASK__VBACK_COUNT_ODD                                       0x40000
// (Undefined, Bits 17)
// VFRONT count changes (odd fields)
#define BIT_MSK__VP__FDET_IRQ_MASK__VFRONT_COUNT_ODD                                      0x20000
// (Undefined, Bits 16)
// VSYNC hight count changes (odd fields)
#define BIT_MSK__VP__FDET_IRQ_MASK__VSYNC_HIGH_COUNT_ODD                                  0x10000
// (Undefined, Bits 15)
// VSYNC low count changes (odd fields)
#define BIT_MSK__VP__FDET_IRQ_MASK__VSYNC_LOW_COUNT_ODD                                   0x8000
// (Undefined, Bits 14)
// VBACK count changes (even fields/progressive)
#define BIT_MSK__VP__FDET_IRQ_MASK__VBACK_COUNT_EVEN                                      0x4000
// (Undefined, Bits 13)
// VFRONT count changes (even fields/progressive)
#define BIT_MSK__VP__FDET_IRQ_MASK__VFRONT_COUNT_EVEN                                     0x2000
// (Undefined, Bits 12)
// VSYNC hight count changes (even fields/progressive)
#define BIT_MSK__VP__FDET_IRQ_MASK__VSYNC_HIGH_COUNT_EVEN                                 0x1000
// (Undefined, Bits 11)
// VSYNC low count changes (even fields/progressive)
#define BIT_MSK__VP__FDET_IRQ_MASK__VSYNC_LOW_COUNT_EVEN                                  0x800
// (Undefined, Bits 10)
// HBACK count changes
#define BIT_MSK__VP__FDET_IRQ_MASK__HBACK_COUNT                                           0x400
// (Undefined, Bits 9)
// HFRONT count changes
#define BIT_MSK__VP__FDET_IRQ_MASK__HFRONT_COUNT                                          0x200
// (Undefined, Bits 8)
// HSYNC high count changes
#define BIT_MSK__VP__FDET_IRQ_MASK__HSYNC_HIGH_COUNT                                      0x100
// (Undefined, Bits 7)
// HSYNC low count changes
#define BIT_MSK__VP__FDET_IRQ_MASK__HSYNC_LOW_COUNT                                       0x80
// (Undefined, Bits 6)
// Line count changes
#define BIT_MSK__VP__FDET_IRQ_MASK__LINE_COUNT                                            0x40
// (Undefined, Bits 5)
// Pixel count changes
#define BIT_MSK__VP__FDET_IRQ_MASK__PIXEL_COUNT                                           0x20
// (Undefined, Bits 4)
// Frame rate changes above threshold
#define BIT_MSK__VP__FDET_IRQ_MASK__FRAME_RATE                                            0x10
// (Undefined, Bits 3)
// BT.656 status changes
#define BIT_MSK__VP__FDET_IRQ_MASK__VIDEO656                                              0x08
// (Undefined, Bits 2)
// Interlaced status changes
#define BIT_MSK__VP__FDET_IRQ_MASK__INTERLACED                                            0x04
// (Undefined, Bits 1)
// VSYNC polarity changes
#define BIT_MSK__VP__FDET_IRQ_MASK__VSYNC_POLARITY                                        0x02
// (Undefined, Bits 0)
// HSYNC polarity changes
#define BIT_MSK__VP__FDET_IRQ_MASK__HSYNC_POLARITY                                        0x01

// IRQ status register
#define REG_ADDR__VP__FDET_IRQ_STATUS                                    (REGTX_VP1 | 0x00B4)
// (Undefined, Bits 18)
// VBACK count changes (odd fields)
#define BIT_MSK__VP__FDET_IRQ_STATUS__VBACK_COUNT_ODD                                       0x40000
// (Undefined, Bits 17)
// VFRONT count changes (odd fields)
#define BIT_MSK__VP__FDET_IRQ_STATUS__VFRONT_COUNT_ODD                                      0x20000
// (Undefined, Bits 16)
// VSYNC hight count changes (odd fields)
#define BIT_MSK__VP__FDET_IRQ_STATUS__VSYNC_HIGH_COUNT_ODD                                  0x10000
// (Undefined, Bits 15)
// VSYNC low count changes (odd fields)
#define BIT_MSK__VP__FDET_IRQ_STATUS__VSYNC_LOW_COUNT_ODD                                   0x8000
// (Undefined, Bits 14)
// VBACK count changes (even fields/progressive)
#define BIT_MSK__VP__FDET_IRQ_STATUS__VBACK_COUNT_EVEN                                      0x4000
// (Undefined, Bits 13)
// VFRONT count changes (even fields/progressive)
#define BIT_MSK__VP__FDET_IRQ_STATUS__VFRONT_COUNT_EVEN                                     0x2000
// (Undefined, Bits 12)
// VSYNC hight count changes (even fields/progressive)
#define BIT_MSK__VP__FDET_IRQ_STATUS__VSYNC_HIGH_COUNT_EVEN                                 0x1000
// (Undefined, Bits 11)
// VSYNC low count changes (even fields/progressive)
#define BIT_MSK__VP__FDET_IRQ_STATUS__VSYNC_LOW_COUNT_EVEN                                  0x800
// (Undefined, Bits 10)
// HBACK count changes
#define BIT_MSK__VP__FDET_IRQ_STATUS__HBACK_COUNT                                           0x400
// (Undefined, Bits 9)
// HFRONT count changes
#define BIT_MSK__VP__FDET_IRQ_STATUS__HFRONT_COUNT                                          0x200
// (Undefined, Bits 8)
// HSYNC high count changes
#define BIT_MSK__VP__FDET_IRQ_STATUS__HSYNC_HIGH_COUNT                                      0x100
// (Undefined, Bits 7)
// HSYNC low count changes
#define BIT_MSK__VP__FDET_IRQ_STATUS__HSYNC_LOW_COUNT                                       0x80
// (Undefined, Bits 6)
// Line count changes
#define BIT_MSK__VP__FDET_IRQ_STATUS__LINE_COUNT                                            0x40
// (Undefined, Bits 5)
// Pixel count changes
#define BIT_MSK__VP__FDET_IRQ_STATUS__PIXEL_COUNT                                           0x20
// (Undefined, Bits 4)
// Frame rate changes above threshold
#define BIT_MSK__VP__FDET_IRQ_STATUS__FRAME_RATE                                            0x10
// (Undefined, Bits 3)
// BT.656 status changes
#define BIT_MSK__VP__FDET_IRQ_STATUS__VIDEO656                                              0x08
// (Undefined, Bits 2)
// Interlaced status changes
#define BIT_MSK__VP__FDET_IRQ_STATUS__INTERLACED                                            0x04
// (Undefined, Bits 1)
// VSYNC polarity changes
#define BIT_MSK__VP__FDET_IRQ_STATUS__VSYNC_POLARITY                                        0x02
// (Undefined, Bits 0)
// HSYNC polarity changes
#define BIT_MSK__VP__FDET_IRQ_STATUS__HSYNC_POLARITY                                        0x01

//***************************************************************************
// REGTX_VP2. Address: 40
// Module features
#define REG_ADDR__VP__CMS__FEATURES                                      (REGTX_VP2 | 0x0000)
// (Undefined, Bits 17)
// Retiming VTG/FIFO
#define BIT_MSK__VP__CMS__FEATURES__RETIMING                                              0x20000
// (Undefined, Bits 16)
// Pixel capture at the output
#define BIT_MSK__VP__CMS__FEATURES__PIXCAP_OUT                                            0x10000
// (Undefined, Bits 15)
// Range Clip
#define BIT_MSK__VP__CMS__FEATURES__RANGE_CLIP                                            0x8000
// (Undefined, Bits 14)
// Dither/Round
#define BIT_MSK__VP__CMS__FEATURES__DITHER_RND                                            0x4000
// (Undefined, Bits 13)
// Chroma vertical subsampler
#define BIT_MSK__VP__CMS__FEATURES__C422_C420                                             0x2000
// (Undefined, Bits 12)
// Chroma horizontal subsampler
#define BIT_MSK__VP__CMS__FEATURES__C444_C422                                             0x1000
// (Undefined, Bits 11)
// Multi-Colorspace Converter #1
#define BIT_MSK__VP__CMS__FEATURES__MULTI_CSC1                                            0x800
// (Undefined, Bits 10)
// Pixel capture after CMS
#define BIT_MSK__VP__CMS__FEATURES__PIXCAP_POST                                           0x400
// (Undefined, Bits 9)
// PWLI #2
#define BIT_MSK__VP__CMS__FEATURES__PWLI2                                                 0x200
// (Undefined, Bits 8)
// PWLI #1
#define BIT_MSK__VP__CMS__FEATURES__PWLI1                                                 0x100
// (Undefined, Bits 7)
// CMS Matrix
#define BIT_MSK__VP__CMS__FEATURES__CMS_MATRIX                                            0x80
// (Undefined, Bits 6)
// PWLI #0
#define BIT_MSK__VP__CMS__FEATURES__PWLI0                                                 0x40
// (Undefined, Bits 5)
// Pixel capture before CMS
#define BIT_MSK__VP__CMS__FEATURES__PIXCAP_PRE                                            0x20
// (Undefined, Bits 4)
// Multi-Colorspace Converter #0
#define BIT_MSK__VP__CMS__FEATURES__MULTI_CSC0                                            0x10
// (Undefined, Bits 3)
// Chroma horizontal upsampler
#define BIT_MSK__VP__CMS__FEATURES__C422_C444                                             0x08
// (Undefined, Bits 2)
// Chroma vertical upsampler
#define BIT_MSK__VP__CMS__FEATURES__C420_C422                                             0x04
// (Undefined, Bits 1)
// Pixel capture at the input
#define BIT_MSK__VP__CMS__FEATURES__PIXCAP_IN                                             0x02
// (Undefined, Bits 0)
// CMS module enabled
#define BIT_MSK__VP__CMS__FEATURES__CMS_CORE                                              0x01

// Device build time stamp
#define REG_ADDR__VP__CMS__BUILD_TIME                                    (REGTX_VP2 | 0x0008)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__BUILD_TIME__                                                      0xFFFFFFFF

// Number of data bits for the datapath
#define REG_ADDR__VP__CMS__DATA_BITS_VALUE                               (REGTX_VP2 | 0x000E)
// (Undefined, Bits 7:0)
//
#define BIT_MSK__VP__CMS__DATA_BITS_VALUE__                                                      0xFF

// Configure capture module
#define REG_ADDR__VP__CMS__PIXCAP_PRE_CONFIG                             (REGTX_VP2 | 0x0010)
// (Undefined, Bits 1)
// Enable auto-capture mode
#define BIT_MSK__VP__CMS__PIXCAP_PRE_CONFIG__AUTO_TRIGGER                                          0x02
// (Undefined, Bits 0)
// Show capture point
#define BIT_MSK__VP__CMS__PIXCAP_PRE_CONFIG__SHOW_POINT                                            0x01

// Configure capture module
#define REG_ADDR__VP__CMS__PIXCAP_PRE_CONTROL                            (REGTX_VP2 | 0x0011)
// (Undefined, Bits 0)
// Trigger pixel value capture
#define BIT_MSK__VP__CMS__PIXCAP_PRE_CONTROL__TRIGGER                                               0x01

// Capture module status
#define REG_ADDR__VP__CMS__PIXCAP_PRE_STATUS                             (REGTX_VP2 | 0x0012)
// (Undefined, Bits 1)
// Error occurred, capture location not found
#define BIT_MSK__VP__CMS__PIXCAP_PRE_STATUS__ERROR                                                 0x02
// (Undefined, Bits 0)
// Capture module is busy, value has not been captured yet
#define BIT_MSK__VP__CMS__PIXCAP_PRE_STATUS__BUSY                                                  0x01

// Specify captured pixel number
#define REG_ADDR__VP__CMS__PIXCAP_PRE_PIXEL                              (REGTX_VP2 | 0x0014)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__PIXCAP_PRE_PIXEL__                                                      0xFFFF

// Specify captured line number
#define REG_ADDR__VP__CMS__PIXCAP_PRE_LINE                               (REGTX_VP2 | 0x0016)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__PIXCAP_PRE_LINE__                                                      0xFFFF

// Captured Y value of selected pixel
#define REG_ADDR__VP__CMS__PIXCAP_PRE_Y                                  (REGTX_VP2 | 0x0018)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__PIXCAP_PRE_Y__                                                      0xFFF

// Captured Cb value of selected pixel
#define REG_ADDR__VP__CMS__PIXCAP_PRE_CB                                 (REGTX_VP2 | 0x001A)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__PIXCAP_PRE_CB__                                                      0xFFF

// Captured Cr value of selected pixel
#define REG_ADDR__VP__CMS__PIXCAP_PRE_CR                                 (REGTX_VP2 | 0x001C)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__PIXCAP_PRE_CR__                                                      0xFFF

// PWLI0 Red Configuration
#define REG_ADDR__VP__CMS__PWLI0_R_CONFIG                                (REGTX_VP2 | 0x0020)
// (Undefined, Bits 11)
// Bank select for PWLI0 Red coefficient table
#define BIT_MSK__VP__CMS__PWLI0_R_CONFIG__BANK_SEL                                              0x800
// (Undefined, Bits 10:8)
// Operating mode
#define BIT_MSK__VP__CMS__PWLI0_R_CONFIG__MODE                                                  0x700
// (Undefined, Bits 7)
// Mute output
#define BIT_MSK__VP__CMS__PWLI0_R_CONFIG__MUTE                                                  0x80
// (Undefined, Bits 6:4)
// Select rounding value for up to 7-bit word size reduction at the output
#define BIT_MSK__VP__CMS__PWLI0_R_CONFIG__ROUND                                                 0x70
// (Undefined, Bits 3)
// Enable extended y_offset range
#define BIT_MSK__VP__CMS__PWLI0_R_CONFIG__ERROR_RANGE                                           0x08
// (Undefined, Bits 2)
// Enable error adjustment
#define BIT_MSK__VP__CMS__PWLI0_R_CONFIG__ERROR_ADJUST                                          0x04
// (Undefined, Bits 1)
// Inverse function
#define BIT_MSK__VP__CMS__PWLI0_R_CONFIG__INVERSE                                               0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__PWLI0_R_CONFIG__ENABLE                                                0x01

// PWLI0 Green Configuration
#define REG_ADDR__VP__CMS__PWLI0_G_CONFIG                                (REGTX_VP2 | 0x0022)
// (Undefined, Bits 11)
// Bank select for PWLI0 Green coefficient table
#define BIT_MSK__VP__CMS__PWLI0_G_CONFIG__BANK_SEL                                              0x800
// (Undefined, Bits 10:8)
// Operating mode
#define BIT_MSK__VP__CMS__PWLI0_G_CONFIG__MODE                                                  0x700
// (Undefined, Bits 7)
// Mute output
#define BIT_MSK__VP__CMS__PWLI0_G_CONFIG__MUTE                                                  0x80
// (Undefined, Bits 6:4)
// Select rounding value for up to 7-bit word size reduction at the output
#define BIT_MSK__VP__CMS__PWLI0_G_CONFIG__ROUND                                                 0x70
// (Undefined, Bits 3)
// Enable extended y_offset range
#define BIT_MSK__VP__CMS__PWLI0_G_CONFIG__ERROR_RANGE                                           0x08
// (Undefined, Bits 2)
// Enable error adjustment
#define BIT_MSK__VP__CMS__PWLI0_G_CONFIG__ERROR_ADJUST                                          0x04
// (Undefined, Bits 1)
// Inverse function
#define BIT_MSK__VP__CMS__PWLI0_G_CONFIG__INVERSE                                               0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__PWLI0_G_CONFIG__ENABLE                                                0x01

// PWLI0 Blue Configuration
#define REG_ADDR__VP__CMS__PWLI0_B_CONFIG                                (REGTX_VP2 | 0x0024)
// (Undefined, Bits 11)
// Bank select for PWLI0 Blue coefficient table
#define BIT_MSK__VP__CMS__PWLI0_B_CONFIG__BANK_SEL                                              0x800
// (Undefined, Bits 10:8)
// Operating mode
#define BIT_MSK__VP__CMS__PWLI0_B_CONFIG__MODE                                                  0x700
// (Undefined, Bits 7)
// Mute output
#define BIT_MSK__VP__CMS__PWLI0_B_CONFIG__MUTE                                                  0x80
// (Undefined, Bits 6:4)
// Select rounding value for up to 7-bit word size reduction at the output
#define BIT_MSK__VP__CMS__PWLI0_B_CONFIG__ROUND                                                 0x70
// (Undefined, Bits 3)
// Enable extended y_offset range
#define BIT_MSK__VP__CMS__PWLI0_B_CONFIG__ERROR_RANGE                                           0x08
// (Undefined, Bits 2)
// Enable error adjustment
#define BIT_MSK__VP__CMS__PWLI0_B_CONFIG__ERROR_ADJUST                                          0x04
// (Undefined, Bits 1)
// Inverse function
#define BIT_MSK__VP__CMS__PWLI0_B_CONFIG__INVERSE                                               0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__PWLI0_B_CONFIG__ENABLE                                                0x01

// Bank update request for PWLI0 parameters (write any data value - this is a strobe only)
#define REG_ADDR__VP__CMS__PWLI0_UPDATE_REQUEST                          (REGTX_VP2 | 0x0026)
// (Undefined, Bits 0)
//
#define BIT_MSK__VP__CMS__PWLI0_UPDATE_REQUEST__                                                      0x01

// Read/write bank configuration for PWLI0 parameters
#define REG_ADDR__VP__CMS__PWLI0_BANK_CONFIG                             (REGTX_VP2 | 0x0027)
// (Undefined, Bits 3)
// When 1, this double buffer is bypassed for all_update request
#define BIT_MSK__VP__CMS__PWLI0_BANK_CONFIG__ALL_UPDATE_BYPASS                                     0x08
// (Undefined, Bits 2)
// Select when to update double-buffer
#define BIT_MSK__VP__CMS__PWLI0_BANK_CONFIG__UPDATE_MODE                                           0x04
// (Undefined, Bits 1)
// Select which bank to write
#define BIT_MSK__VP__CMS__PWLI0_BANK_CONFIG__WRITE_BANK                                            0x02
// (Undefined, Bits 0)
// Select which bank to read
#define BIT_MSK__VP__CMS__PWLI0_BANK_CONFIG__READ_BANK                                             0x01

// Start address of PWLI0 coefficient table. This register auto-increments when any of the pwli0_r_data/pwli0_g_data/pwli0_b_data registers is read or written. 2 MSBs select the coefficient bank
#define REG_ADDR__VP__CMS__PWLI0_ADDR                                    (REGTX_VP2 | 0x002C)
// (Undefined, Bits 6:0)
//
#define BIT_MSK__VP__CMS__PWLI0_ADDR__                                                      0x7F

// PWLI0 Red coefficient table data. Reading or writing this register increments the pwli0_addr value
#define REG_ADDR__VP__CMS__PWLI0_R_DATA                                  (REGTX_VP2 | 0x0030)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI0_R_DATA__                                                      0xFFFFFFFF

// PWLI0 Green coefficient table data. Reading or writing this register increments the pwli0_addr value
#define REG_ADDR__VP__CMS__PWLI0_G_DATA                                  (REGTX_VP2 | 0x0034)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI0_G_DATA__                                                      0xFFFFFFFF

// PWLI0 Blue coefficient table data. Reading or writing this register increments the pwli0_addr value
#define REG_ADDR__VP__CMS__PWLI0_B_DATA                                  (REGTX_VP2 | 0x0038)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI0_B_DATA__                                                      0xFFFFFFFF

// PWLI0 combined coefficient table data. Writing this registers increments the pwli0_addr value. No read access!
#define REG_ADDR__VP__CMS__PWLI0_DATA                                    (REGTX_VP2 | 0x003C)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI0_DATA__                                                      0xFFFFFFFF

// 3x3 Matrix Configuration
#define REG_ADDR__VP__CMS__MATRIX3X3_CONFIG                              (REGTX_VP2 | 0x0040)
// (Undefined, Bits 6)
// Force selected coefficient set
#define BIT_MSK__VP__CMS__MATRIX3X3_CONFIG__FORCE_ENABLE                                          0x40
// (Undefined, Bits 5:3)
// Coefficient set for forced mode
#define BIT_MSK__VP__CMS__MATRIX3X3_CONFIG__FORCE_SET                                             0x38
// (Undefined, Bits 2)
// Enable CMS mode
#define BIT_MSK__VP__CMS__MATRIX3X3_CONFIG__CMS_MODE                                              0x04
// (Undefined, Bits 1)
// Select dither/round operation
#define BIT_MSK__VP__CMS__MATRIX3X3_CONFIG__DITHER_ENABLE                                         0x02
// (Undefined, Bits 0)
// Enable 3x3 Matrix
#define BIT_MSK__VP__CMS__MATRIX3X3_CONFIG__ENABLE                                                0x01

// 3x3 Matrix Set Configuration
#define REG_ADDR__VP__CMS__MATRIX3X3_CMS_CONFIG                          (REGTX_VP2 | 0x0041)
// (Undefined, Bits 6:3)
// Coefficient read address
#define BIT_MSK__VP__CMS__MATRIX3X3_CMS_CONFIG__COEFF_ADDR                                            0x78
// (Undefined, Bits 2:0)
// Coefficient set address
#define BIT_MSK__VP__CMS__MATRIX3X3_CMS_CONFIG__SET_ADDR                                              0x07

// 3x3 Matrix Set Control
#define REG_ADDR__VP__CMS__MATRIX3X3_CMS_CONTROL                         (REGTX_VP2 | 0x0042)
// (Undefined, Bits 1)
// Start read operation
#define BIT_MSK__VP__CMS__MATRIX3X3_CMS_CONTROL__READ                                                  0x02
// (Undefined, Bits 0)
// Start write operation
#define BIT_MSK__VP__CMS__MATRIX3X3_CMS_CONTROL__WRITE                                                 0x01

// 3x3 Matrix Set Status
#define REG_ADDR__VP__CMS__MATRIX3X3_CMS_STATUS                          (REGTX_VP2 | 0x0043)
// (Undefined, Bits 0)
// Transaction in progress
#define BIT_MSK__VP__CMS__MATRIX3X3_CMS_STATUS__BUSY                                                  0x01

// R1C1 matrix multiplier coefficient. Assumed to be 18-bit signed with 4 integer bits and 14 fractional bits.
#define REG_ADDR__VP__CMS__MATRIX3X3_MULTCOEFFR1C1                       (REGTX_VP2 | 0x0044)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_MULTCOEFFR1C1__                                                      0x3FFFF

// R1C2 matrix multiplier coefficient. Assumed to be 18-bit signed with 4 integer bits and 14 fractional bits.
#define REG_ADDR__VP__CMS__MATRIX3X3_MULTCOEFFR1C2                       (REGTX_VP2 | 0x0048)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_MULTCOEFFR1C2__                                                      0x3FFFF

// R1C3 matrix multiplier coefficient. Assumed to be 18-bit signed with 4 integer bits and 14 fractional bits.
#define REG_ADDR__VP__CMS__MATRIX3X3_MULTCOEFFR1C3                       (REGTX_VP2 | 0x004C)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_MULTCOEFFR1C3__                                                      0x3FFFF

// R2C1 matrix multiplier coefficient. Assumed to be 18-bit signed with 4 integer bits and 14 fractional bits.
#define REG_ADDR__VP__CMS__MATRIX3X3_MULTCOEFFR2C1                       (REGTX_VP2 | 0x0050)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_MULTCOEFFR2C1__                                                      0x3FFFF

// R2C2 matrix multiplier coefficient. Assumed to be 18-bit signed with 4 integer bits and 14 fractional bits.
#define REG_ADDR__VP__CMS__MATRIX3X3_MULTCOEFFR2C2                       (REGTX_VP2 | 0x0054)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_MULTCOEFFR2C2__                                                      0x3FFFF

// R2C3 matrix multiplier coefficient. Assumed to be 18-bit signed with 4 integer bits and 14 fractional bits.
#define REG_ADDR__VP__CMS__MATRIX3X3_MULTCOEFFR2C3                       (REGTX_VP2 | 0x0058)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_MULTCOEFFR2C3__                                                      0x3FFFF

// R3C1 matrix multiplier coefficient. Assumed to be 18-bit signed with 4 integer bits and 14 fractional bits.
#define REG_ADDR__VP__CMS__MATRIX3X3_MULTCOEFFR3C1                       (REGTX_VP2 | 0x005C)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_MULTCOEFFR3C1__                                                      0x3FFFF

// R3C2 matrix multiplier coefficient. Assumed to be 18-bit signed with 4 integer bits and 14 fractional bits.
#define REG_ADDR__VP__CMS__MATRIX3X3_MULTCOEFFR3C2                       (REGTX_VP2 | 0x0060)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_MULTCOEFFR3C2__                                                      0x3FFFF

// R3C3 matrix multiplier coefficient. Assumed to be 18-bit signed with 4 integer bits and 14 fractional bits.
#define REG_ADDR__VP__CMS__MATRIX3X3_MULTCOEFFR3C3                       (REGTX_VP2 | 0x0064)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_MULTCOEFFR3C3__                                                      0x3FFFF

// CMS matrix coefficient read data
#define REG_ADDR__VP__CMS__MATRIX3X3_CMS_COEFF                           (REGTX_VP2 | 0x0068)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_CMS_COEFF__                                                      0x3FFFF

// Signed input offset to be added to the corresponding video data prior to matrix multiplication
#define REG_ADDR__VP__CMS__MATRIX3X3_IN_GY_OFFSET                        (REGTX_VP2 | 0x0070)
// (Undefined, Bits 14:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_IN_GY_OFFSET__                                                      0x7FFF

// Signed input offset to be added to the corresponding video data prior to matrix multiplication
#define REG_ADDR__VP__CMS__MATRIX3X3_IN_BCB_OFFSET                       (REGTX_VP2 | 0x0072)
// (Undefined, Bits 14:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_IN_BCB_OFFSET__                                                      0x7FFF

// Signed input offset to be added to the corresponding video data prior to matrix multiplication
#define REG_ADDR__VP__CMS__MATRIX3X3_IN_RCR_OFFSET                       (REGTX_VP2 | 0x0074)
// (Undefined, Bits 14:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_IN_RCR_OFFSET__                                                      0x7FFF

// Signed output offset to be added to the corresponding video data after matrix multiplication
#define REG_ADDR__VP__CMS__MATRIX3X3_OUT_GY_OFFSET                       (REGTX_VP2 | 0x0076)
// (Undefined, Bits 14:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_OUT_GY_OFFSET__                                                      0x7FFF

// Signed output offset to be added to the corresponding video data after matrix multiplication
#define REG_ADDR__VP__CMS__MATRIX3X3_OUT_BCB_OFFSET                      (REGTX_VP2 | 0x0078)
// (Undefined, Bits 14:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_OUT_BCB_OFFSET__                                                      0x7FFF

// Signed output offset to be added to the corresponding video data after matrix multiplication
#define REG_ADDR__VP__CMS__MATRIX3X3_OUT_RCR_OFFSET                      (REGTX_VP2 | 0x007A)
// (Undefined, Bits 14:0)
//
#define BIT_MSK__VP__CMS__MATRIX3X3_OUT_RCR_OFFSET__                                                      0x7FFF

// PWLI1 Red Configuration
#define REG_ADDR__VP__CMS__PWLI1_R_CONFIG                                (REGTX_VP2 | 0x0080)
// (Undefined, Bits 11)
// Bank select for PWLI1 Red coefficient table
#define BIT_MSK__VP__CMS__PWLI1_R_CONFIG__BANK_SEL                                              0x800
// (Undefined, Bits 10:8)
// Operating mode
#define BIT_MSK__VP__CMS__PWLI1_R_CONFIG__MODE                                                  0x700
// (Undefined, Bits 7)
// Mute output
#define BIT_MSK__VP__CMS__PWLI1_R_CONFIG__MUTE                                                  0x80
// (Undefined, Bits 6:4)
// Select rounding value for up to 7-bit word size reduction at the output
#define BIT_MSK__VP__CMS__PWLI1_R_CONFIG__ROUND                                                 0x70
// (Undefined, Bits 3)
// Enable extended y_offset range
#define BIT_MSK__VP__CMS__PWLI1_R_CONFIG__ERROR_RANGE                                           0x08
// (Undefined, Bits 2)
// Enable error adjustment
#define BIT_MSK__VP__CMS__PWLI1_R_CONFIG__ERROR_ADJUST                                          0x04
// (Undefined, Bits 1)
// Inverse function
#define BIT_MSK__VP__CMS__PWLI1_R_CONFIG__INVERSE                                               0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__PWLI1_R_CONFIG__ENABLE                                                0x01

// PWLI1 Green Configuration
#define REG_ADDR__VP__CMS__PWLI1_G_CONFIG                                (REGTX_VP2 | 0x0082)
// (Undefined, Bits 11)
// Bank select for PWLI1 Green coefficient table
#define BIT_MSK__VP__CMS__PWLI1_G_CONFIG__BANK_SEL                                              0x800
// (Undefined, Bits 10:8)
// Operating mode
#define BIT_MSK__VP__CMS__PWLI1_G_CONFIG__MODE                                                  0x700
// (Undefined, Bits 7)
// Mute output
#define BIT_MSK__VP__CMS__PWLI1_G_CONFIG__MUTE                                                  0x80
// (Undefined, Bits 6:4)
// Select rounding value for up to 7-bit word size reduction at the output
#define BIT_MSK__VP__CMS__PWLI1_G_CONFIG__ROUND                                                 0x70
// (Undefined, Bits 3)
// Enable extended y_offset range
#define BIT_MSK__VP__CMS__PWLI1_G_CONFIG__ERROR_RANGE                                           0x08
// (Undefined, Bits 2)
// Enable error adjustment
#define BIT_MSK__VP__CMS__PWLI1_G_CONFIG__ERROR_ADJUST                                          0x04
// (Undefined, Bits 1)
// Inverse function
#define BIT_MSK__VP__CMS__PWLI1_G_CONFIG__INVERSE                                               0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__PWLI1_G_CONFIG__ENABLE                                                0x01

// PWLI1 Blue Configuration
#define REG_ADDR__VP__CMS__PWLI1_B_CONFIG                                (REGTX_VP2 | 0x0084)
// (Undefined, Bits 11)
// Bank select for PWLI1 Blue coefficient table
#define BIT_MSK__VP__CMS__PWLI1_B_CONFIG__BANK_SEL                                              0x800
// (Undefined, Bits 10:8)
// Operating mode
#define BIT_MSK__VP__CMS__PWLI1_B_CONFIG__MODE                                                  0x700
// (Undefined, Bits 7)
// Mute output
#define BIT_MSK__VP__CMS__PWLI1_B_CONFIG__MUTE                                                  0x80
// (Undefined, Bits 6:4)
// Select rounding value for up to 7-bit word size reduction at the output
#define BIT_MSK__VP__CMS__PWLI1_B_CONFIG__ROUND                                                 0x70
// (Undefined, Bits 3)
// Enable extended y_offset range
#define BIT_MSK__VP__CMS__PWLI1_B_CONFIG__ERROR_RANGE                                           0x08
// (Undefined, Bits 2)
// Enable error adjustment
#define BIT_MSK__VP__CMS__PWLI1_B_CONFIG__ERROR_ADJUST                                          0x04
// (Undefined, Bits 1)
// Inverse function
#define BIT_MSK__VP__CMS__PWLI1_B_CONFIG__INVERSE                                               0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__PWLI1_B_CONFIG__ENABLE                                                0x01

// Bank update request for PWLI1 parameters (write any data value - this is a strobe only)
#define REG_ADDR__VP__CMS__PWLI1_UPDATE_REQUEST                          (REGTX_VP2 | 0x0086)
// (Undefined, Bits 0)
//
#define BIT_MSK__VP__CMS__PWLI1_UPDATE_REQUEST__                                                      0x01

// Read/write bank configuration for PWLI1 parameters
#define REG_ADDR__VP__CMS__PWLI1_BANK_CONFIG                             (REGTX_VP2 | 0x0087)
// (Undefined, Bits 3)
// When 1, this double buffer is bypassed for all_update request
#define BIT_MSK__VP__CMS__PWLI1_BANK_CONFIG__ALL_UPDATE_BYPASS                                     0x08
// (Undefined, Bits 2)
// Select when to update double-buffer
#define BIT_MSK__VP__CMS__PWLI1_BANK_CONFIG__UPDATE_MODE                                           0x04
// (Undefined, Bits 1)
// Select which bank to write
#define BIT_MSK__VP__CMS__PWLI1_BANK_CONFIG__WRITE_BANK                                            0x02
// (Undefined, Bits 0)
// Select which bank to read
#define BIT_MSK__VP__CMS__PWLI1_BANK_CONFIG__READ_BANK                                             0x01

// Start address of PWLI1 coefficient table. This register auto-increments when any of the pwli1_r_data/pwli1_g_data/pwli1_b_data registers is read or written. 2 MSBs select the coefficient bank
#define REG_ADDR__VP__CMS__PWLI1_ADDR                                    (REGTX_VP2 | 0x008C)
// (Undefined, Bits 6:0)
//
#define BIT_MSK__VP__CMS__PWLI1_ADDR__                                                      0x7F

// PWLI1 Red coefficient table data. Reading or writing this register increments the pwli1_addr value
#define REG_ADDR__VP__CMS__PWLI1_R_DATA                                  (REGTX_VP2 | 0x0090)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI1_R_DATA__                                                      0xFFFFFFFF

// PWLI1 Green coefficient table data. Reading or writing this register increments the pwli1_addr value
#define REG_ADDR__VP__CMS__PWLI1_G_DATA                                  (REGTX_VP2 | 0x0094)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI1_G_DATA__                                                      0xFFFFFFFF

// PWLI1 Blue coefficient table data. Reading or writing this register increments the pwli1_addr value
#define REG_ADDR__VP__CMS__PWLI1_B_DATA                                  (REGTX_VP2 | 0x0098)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI1_B_DATA__                                                      0xFFFFFFFF

// PWLI1 combined coefficient table data. Writing this registers increments the pwli1_addr value. No read access!
#define REG_ADDR__VP__CMS__PWLI1_DATA                                    (REGTX_VP2 | 0x009C)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI1_DATA__                                                      0xFFFFFFFF

// pwli2 Red Configuration
#define REG_ADDR__VP__CMS__PWLI2_R_CONFIG                                (REGTX_VP2 | 0x00A0)
// (Undefined, Bits 11)
// Bank select for pwli2 Red coefficient table
#define BIT_MSK__VP__CMS__PWLI2_R_CONFIG__BANK_SEL                                              0x800
// (Undefined, Bits 10:8)
// Operating mode
#define BIT_MSK__VP__CMS__PWLI2_R_CONFIG__MODE                                                  0x700
// (Undefined, Bits 7)
// Mute output
#define BIT_MSK__VP__CMS__PWLI2_R_CONFIG__MUTE                                                  0x80
// (Undefined, Bits 6:4)
// Select rounding value for up to 7-bit word size reduction at the output
#define BIT_MSK__VP__CMS__PWLI2_R_CONFIG__ROUND                                                 0x70
// (Undefined, Bits 3)
// Enable extended y_offset range
#define BIT_MSK__VP__CMS__PWLI2_R_CONFIG__ERROR_RANGE                                           0x08
// (Undefined, Bits 2)
// Enable error adjustment
#define BIT_MSK__VP__CMS__PWLI2_R_CONFIG__ERROR_ADJUST                                          0x04
// (Undefined, Bits 1)
// Inverse function
#define BIT_MSK__VP__CMS__PWLI2_R_CONFIG__INVERSE                                               0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__PWLI2_R_CONFIG__ENABLE                                                0x01

// pwli2 Green Configuration
#define REG_ADDR__VP__CMS__PWLI2_G_CONFIG                                (REGTX_VP2 | 0x00A2)
// (Undefined, Bits 11)
// Bank select for pwli2 Green coefficient table
#define BIT_MSK__VP__CMS__PWLI2_G_CONFIG__BANK_SEL                                              0x800
// (Undefined, Bits 10:8)
// Operating mode
#define BIT_MSK__VP__CMS__PWLI2_G_CONFIG__MODE                                                  0x700
// (Undefined, Bits 7)
// Mute output
#define BIT_MSK__VP__CMS__PWLI2_G_CONFIG__MUTE                                                  0x80
// (Undefined, Bits 6:4)
// Select rounding value for up to 7-bit word size reduction at the output
#define BIT_MSK__VP__CMS__PWLI2_G_CONFIG__ROUND                                                 0x70
// (Undefined, Bits 3)
// Enable extended y_offset range
#define BIT_MSK__VP__CMS__PWLI2_G_CONFIG__ERROR_RANGE                                           0x08
// (Undefined, Bits 2)
// Enable error adjustment
#define BIT_MSK__VP__CMS__PWLI2_G_CONFIG__ERROR_ADJUST                                          0x04
// (Undefined, Bits 1)
// Inverse function
#define BIT_MSK__VP__CMS__PWLI2_G_CONFIG__INVERSE                                               0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__PWLI2_G_CONFIG__ENABLE                                                0x01

// pwli2 Blue Configuration
#define REG_ADDR__VP__CMS__PWLI2_B_CONFIG                                (REGTX_VP2 | 0x00A4)
// (Undefined, Bits 11)
// Bank select for pwli2 Blue coefficient table
#define BIT_MSK__VP__CMS__PWLI2_B_CONFIG__BANK_SEL                                              0x800
// (Undefined, Bits 10:8)
// Operating mode
#define BIT_MSK__VP__CMS__PWLI2_B_CONFIG__MODE                                                  0x700
// (Undefined, Bits 7)
// Mute output
#define BIT_MSK__VP__CMS__PWLI2_B_CONFIG__MUTE                                                  0x80
// (Undefined, Bits 6:4)
// Select rounding value for up to 7-bit word size reduction at the output
#define BIT_MSK__VP__CMS__PWLI2_B_CONFIG__ROUND                                                 0x70
// (Undefined, Bits 3)
// Enable extended y_offset range
#define BIT_MSK__VP__CMS__PWLI2_B_CONFIG__ERROR_RANGE                                           0x08
// (Undefined, Bits 2)
// Enable error adjustment
#define BIT_MSK__VP__CMS__PWLI2_B_CONFIG__ERROR_ADJUST                                          0x04
// (Undefined, Bits 1)
// Inverse function
#define BIT_MSK__VP__CMS__PWLI2_B_CONFIG__INVERSE                                               0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__PWLI2_B_CONFIG__ENABLE                                                0x01

// Bank update request for pwli2 parameters (write any data value - this is a strobe only)
#define REG_ADDR__VP__CMS__PWLI2_UPDATE_REQUEST                          (REGTX_VP2 | 0x00A6)
// (Undefined, Bits 0)
//
#define BIT_MSK__VP__CMS__PWLI2_UPDATE_REQUEST__                                                      0x01

// Read/write bank configuration for pwli2 parameters
#define REG_ADDR__VP__CMS__PWLI2_BANK_CONFIG                             (REGTX_VP2 | 0x00A7)
// (Undefined, Bits 3)
// When 1, this double buffer is bypassed for all_update request
#define BIT_MSK__VP__CMS__PWLI2_BANK_CONFIG__ALL_UPDATE_BYPASS                                     0x08
// (Undefined, Bits 2)
// Select when to update double-buffer
#define BIT_MSK__VP__CMS__PWLI2_BANK_CONFIG__UPDATE_MODE                                           0x04
// (Undefined, Bits 1)
// Select which bank to write
#define BIT_MSK__VP__CMS__PWLI2_BANK_CONFIG__WRITE_BANK                                            0x02
// (Undefined, Bits 0)
// Select which bank to read
#define BIT_MSK__VP__CMS__PWLI2_BANK_CONFIG__READ_BANK                                             0x01

// Start address of pwli2 coefficient table. This register auto-increments when any of the pwli2_r_data/pwli2_g_data/pwli2_b_data registers is read or written. 2 MSBs select the coefficient bank
#define REG_ADDR__VP__CMS__PWLI2_ADDR                                    (REGTX_VP2 | 0x00AC)
// (Undefined, Bits 6:0)
//
#define BIT_MSK__VP__CMS__PWLI2_ADDR__                                                      0x7F

// pwli2 Red coefficient table data. Reading or writing this register increments the pwli2_addr value
#define REG_ADDR__VP__CMS__PWLI2_R_DATA                                  (REGTX_VP2 | 0x00B0)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI2_R_DATA__                                                      0xFFFFFFFF

// pwli2 Green coefficient table data. Reading or writing this register increments the pwli2_addr value
#define REG_ADDR__VP__CMS__PWLI2_G_DATA                                  (REGTX_VP2 | 0x00B4)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI2_G_DATA__                                                      0xFFFFFFFF

// pwli2 Blue coefficient table data. Reading or writing this register increments the pwli2_addr value
#define REG_ADDR__VP__CMS__PWLI2_B_DATA                                  (REGTX_VP2 | 0x00B8)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI2_B_DATA__                                                      0xFFFFFFFF

// pwli2 combined coefficient table data. Writing this registers increments the pwli2_addr value. No read access!
#define REG_ADDR__VP__CMS__PWLI2_DATA                                    (REGTX_VP2 | 0x00BC)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__PWLI2_DATA__                                                      0xFFFFFFFF

// Configure capture module
#define REG_ADDR__VP__CMS__PIXCAP_POST_CONFIG                            (REGTX_VP2 | 0x00C0)
// (Undefined, Bits 1)
// Enable auto-capture mode
#define BIT_MSK__VP__CMS__PIXCAP_POST_CONFIG__AUTO_TRIGGER                                          0x02
// (Undefined, Bits 0)
// Show capture point
#define BIT_MSK__VP__CMS__PIXCAP_POST_CONFIG__SHOW_POINT                                            0x01

// Configure capture module
#define REG_ADDR__VP__CMS__PIXCAP_POST_CONTROL                           (REGTX_VP2 | 0x00C1)
// (Undefined, Bits 0)
// Trigger pixel value capture
#define BIT_MSK__VP__CMS__PIXCAP_POST_CONTROL__TRIGGER                                               0x01

// Capture module status
#define REG_ADDR__VP__CMS__PIXCAP_POST_STATUS                            (REGTX_VP2 | 0x00C2)
// (Undefined, Bits 1)
// Error occurred, capture location not found
#define BIT_MSK__VP__CMS__PIXCAP_POST_STATUS__ERROR                                                 0x02
// (Undefined, Bits 0)
// Capture module is busy, value has not been captured yet
#define BIT_MSK__VP__CMS__PIXCAP_POST_STATUS__BUSY                                                  0x01

// Specify captured pixel number
#define REG_ADDR__VP__CMS__PIXCAP_POST_PIXEL                             (REGTX_VP2 | 0x00C4)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__PIXCAP_POST_PIXEL__                                                      0xFFFF

// Specify captured line number
#define REG_ADDR__VP__CMS__PIXCAP_POST_LINE                              (REGTX_VP2 | 0x00C6)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__PIXCAP_POST_LINE__                                                      0xFFFF

// Captured Y value of selected pixel
#define REG_ADDR__VP__CMS__PIXCAP_POST_Y                                 (REGTX_VP2 | 0x00C8)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__PIXCAP_POST_Y__                                                      0xFFF

// Captured Cb value of selected pixel
#define REG_ADDR__VP__CMS__PIXCAP_POST_CB                                (REGTX_VP2 | 0x00CA)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__PIXCAP_POST_CB__                                                      0xFFF

// Captured Cr value of selected pixel
#define REG_ADDR__VP__CMS__PIXCAP_POST_CR                                (REGTX_VP2 | 0x00CC)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__PIXCAP_POST_CR__                                                      0xFFF

// CMS demo mode configuration
#define REG_ADDR__VP__CMS__CMS_DEMO_SPLIT_CONFIG                         (REGTX_VP2 | 0x00D0)
// (Undefined, Bits 2)
// Interlace mode
#define BIT_MSK__VP__CMS__CMS_DEMO_SPLIT_CONFIG__INTERLACE                                             0x04
// (Undefined, Bits 1:0)
// Select CMS demo display
#define BIT_MSK__VP__CMS__CMS_DEMO_SPLIT_CONFIG__SELECT                                                0x03

// Demo mode horizontal split point
#define REG_ADDR__VP__CMS__CMS_DEMO_SPLIT_X                              (REGTX_VP2 | 0x00D2)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CMS_DEMO_SPLIT_X__                                                      0xFFFF

// Demo mode vertical split point
#define REG_ADDR__VP__CMS__CMS_DEMO_SPLIT_Y                              (REGTX_VP2 | 0x00D4)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CMS_DEMO_SPLIT_Y__                                                      0xFFFF

// Demo mode split bar width
#define REG_ADDR__VP__CMS__CMS_DEMO_BAR_WIDTH                            (REGTX_VP2 | 0x00D6)
// (Undefined, Bits 7:0)
//
#define BIT_MSK__VP__CMS__CMS_DEMO_BAR_WIDTH__                                                      0xFF

// Demo mode split bar color (G/Y)
#define REG_ADDR__VP__CMS__CMS_DEMO_BAR_DATA_Y                           (REGTX_VP2 | 0x00D8)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CMS_DEMO_BAR_DATA_Y__                                                      0xFFF

// Demo mode split bar color (B/Cb)
#define REG_ADDR__VP__CMS__CMS_DEMO_BAR_DATA_CB                          (REGTX_VP2 | 0x00DA)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CMS_DEMO_BAR_DATA_CB__                                                      0xFFF

// Demo mode split bar color (R/Cr)
#define REG_ADDR__VP__CMS__CMS_DEMO_BAR_DATA_CR                          (REGTX_VP2 | 0x00DC)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CMS_DEMO_BAR_DATA_CR__                                                      0xFFF

//***************************************************************************
// REGTX_VP3. Address: 40
// Module features
#define REG_ADDR__VP__CMS__CSC0__FEATURES                                (REGTX_VP3 | 0x0000)
// (Undefined, Bits 10)
// Retiming VTG/FIFO
#define BIT_MSK__VP__CMS__CSC0__FEATURES__RETIMING                                              0x400
// (Undefined, Bits 9)
// Pixel capture at the output
#define BIT_MSK__VP__CMS__CSC0__FEATURES__PIXCAP_OUT                                            0x200
// (Undefined, Bits 8)
// Range Clip
#define BIT_MSK__VP__CMS__CSC0__FEATURES__RANGE_CLIP                                            0x100
// (Undefined, Bits 7)
// Dither/Round
#define BIT_MSK__VP__CMS__CSC0__FEATURES__DITHER_RND                                            0x80
// (Undefined, Bits 6)
// Chroma vertical subsampler
#define BIT_MSK__VP__CMS__CSC0__FEATURES__C422_C420                                             0x40
// (Undefined, Bits 5)
// Chroma horizontal subsampler
#define BIT_MSK__VP__CMS__CSC0__FEATURES__C444_C422                                             0x20
// (Undefined, Bits 4)
// Multi-Colorspace Converter
#define BIT_MSK__VP__CMS__CSC0__FEATURES__MULTI_CSC                                             0x10
// (Undefined, Bits 3)
// Chroma horizontal upsampler
#define BIT_MSK__VP__CMS__CSC0__FEATURES__C422_C444                                             0x08
// (Undefined, Bits 2)
// Chroma vertical upsampler
#define BIT_MSK__VP__CMS__CSC0__FEATURES__C420_C422                                             0x04
// (Undefined, Bits 1)
// Pixel capture at the input
#define BIT_MSK__VP__CMS__CSC0__FEATURES__PIXCAP_IN                                             0x02
// (Undefined, Bits 0)
// CSC module enabled
#define BIT_MSK__VP__CMS__CSC0__FEATURES__CSC_CORE                                              0x01

// Number of data bits for the datapath
#define REG_ADDR__VP__CMS__CSC0__DATA_BITS_VALUE                         (REGTX_VP3 | 0x0002)
// (Undefined, Bits 7:0)
//
#define BIT_MSK__VP__CMS__CSC0__DATA_BITS_VALUE__                                                      0xFF

// Device build time stamp
#define REG_ADDR__VP__CMS__CSC0__BUILD_TIME                              (REGTX_VP3 | 0x0004)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__CSC0__BUILD_TIME__                                                      0xFFFFFFFF

// Configure chroma vertical upsampler
#define REG_ADDR__VP__CMS__CSC0__C420_C422_CONFIG                        (REGTX_VP3 | 0x0008)
// (Undefined, Bits 3)
// Polarity of alternating Cb/Cr
#define BIT_MSK__VP__CMS__CSC0__C420_C422_CONFIG__IN_CB_CR_POLARITY                                     0x08
// (Undefined, Bits 2)
// Select whether Cb or Cr bus are used as chroma input
#define BIT_MSK__VP__CMS__CSC0__C420_C422_CONFIG__IN_CB_OR_CR                                           0x04
// (Undefined, Bits 1)
// Bypass module to reduce latency
#define BIT_MSK__VP__CMS__CSC0__C420_C422_CONFIG__BYPASS                                                0x02
// (Undefined, Bits 0)
// Enable chroma vertical upsampler
#define BIT_MSK__VP__CMS__CSC0__C420_C422_CONFIG__ENABLE                                                0x01

// Configure chroma upsampler
#define REG_ADDR__VP__CMS__CSC0__C422_C444_CONFIG                        (REGTX_VP3 | 0x000C)
// (Undefined, Bits 2)
// Disable Lowpass filter
#define BIT_MSK__VP__CMS__CSC0__C422_C444_CONFIG__DISABLE_FILTER                                        0x04
// (Undefined, Bits 1)
// Select whether Cb or Cr bus are used as muxed chroma input
#define BIT_MSK__VP__CMS__CSC0__C422_C444_CONFIG__USE_CB_OR_CR                                          0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__CSC0__C422_C444_CONFIG__ENABLE                                                0x01

// Configure capture module
#define REG_ADDR__VP__CMS__CSC0__PIXCAP_IN_CONFIG                        (REGTX_VP3 | 0x0010)
// (Undefined, Bits 1)
// Enable auto-capture mode
#define BIT_MSK__VP__CMS__CSC0__PIXCAP_IN_CONFIG__AUTO_TRIGGER                                          0x02
// (Undefined, Bits 0)
// Show capture point
#define BIT_MSK__VP__CMS__CSC0__PIXCAP_IN_CONFIG__SHOW_POINT                                            0x01

// Configure capture module
#define REG_ADDR__VP__CMS__CSC0__PIXCAP_IN_CONTROL                       (REGTX_VP3 | 0x0011)
// (Undefined, Bits 0)
// Trigger pixel value capture
#define BIT_MSK__VP__CMS__CSC0__PIXCAP_IN_CONTROL__TRIGGER                                               0x01

// Capture module status
#define REG_ADDR__VP__CMS__CSC0__PIXCAP_IN_STATUS                        (REGTX_VP3 | 0x0012)
// (Undefined, Bits 1)
// Error occurred, capture location not found
#define BIT_MSK__VP__CMS__CSC0__PIXCAP_IN_STATUS__ERROR                                                 0x02
// (Undefined, Bits 0)
// Capture module is busy, value has not been captured yet
#define BIT_MSK__VP__CMS__CSC0__PIXCAP_IN_STATUS__BUSY                                                  0x01

// Specify captured pixel number
#define REG_ADDR__VP__CMS__CSC0__PIXCAP_IN_PIXEL                         (REGTX_VP3 | 0x0014)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__PIXCAP_IN_PIXEL__                                                      0xFFFF

// Specify captured line number
#define REG_ADDR__VP__CMS__CSC0__PIXCAP_IN_LINE                          (REGTX_VP3 | 0x0016)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__PIXCAP_IN_LINE__                                                      0xFFFF

// Captured Y value of selected pixel
#define REG_ADDR__VP__CMS__CSC0__PIXCAP_IN_Y                             (REGTX_VP3 | 0x0018)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CSC0__PIXCAP_IN_Y__                                                      0xFFF

// Captured Cb value of selected pixel
#define REG_ADDR__VP__CMS__CSC0__PIXCAP_IN_CB                            (REGTX_VP3 | 0x001A)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CSC0__PIXCAP_IN_CB__                                                      0xFFF

// Captured Cr value of selected pixel
#define REG_ADDR__VP__CMS__CSC0__PIXCAP_IN_CR                            (REGTX_VP3 | 0x001C)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CSC0__PIXCAP_IN_CR__                                                      0xFFF

// Colorspace converter configuration
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG                        (REGTX_VP3 | 0x0020)
// (Undefined, Bits 11)
// Disable saturation
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__DISABLE_SATURATION                                    0x800
// (Undefined, Bits 10)
// Select dither/round operation
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__DITHER_ENABLE                                         0x400
// (Undefined, Bits 9)
// Select input color space
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__IN_RGB                                                0x200
// (Undefined, Bits 8)
// Select input levels
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__IN_PC                                                 0x100
// (Undefined, Bits 7:6)
// Select input color standard
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__IN_STD                                                0xC0
// (Undefined, Bits 5)
// Select output color space
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__OUT_RGB                                               0x20
// (Undefined, Bits 4)
// Select output levels
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__OUT_PC                                                0x10
// (Undefined, Bits 3:2)
// Select output color standard
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__OUT_STD                                               0x0C
// (Undefined, Bits 1:0)
// Enable conversion
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE                                                0x03

// R1C1 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR1C1                 (REGTX_VP3 | 0x0022)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR1C1__                                                      0xFFFF

// R1C2 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR1C2                 (REGTX_VP3 | 0x0024)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR1C2__                                                      0xFFFF

// R1C3 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR1C3                 (REGTX_VP3 | 0x0026)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR1C3__                                                      0xFFFF

// R2C1 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR2C1                 (REGTX_VP3 | 0x0028)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR2C1__                                                      0xFFFF

// R2C2 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR2C2                 (REGTX_VP3 | 0x002A)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR2C2__                                                      0xFFFF

// R2C3 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR2C3                 (REGTX_VP3 | 0x002C)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR2C3__                                                      0xFFFF

// R3C1 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR3C1                 (REGTX_VP3 | 0x002E)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR3C1__                                                      0xFFFF

// R3C2 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR3C2                 (REGTX_VP3 | 0x0030)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR3C2__                                                      0xFFFF

// R3C3 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR3C3                 (REGTX_VP3 | 0x0032)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR3C3__                                                      0xFFFF

// Signed input offset to be added to the corresponding video data prior to matrix multiplication
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_IN_GY_OFFSET                  (REGTX_VP3 | 0x0034)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_IN_GY_OFFSET__                                                      0x1FFF

// Signed input offset to be added to the corresponding video data prior to matrix multiplication
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_IN_BCB_OFFSET                 (REGTX_VP3 | 0x0036)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_IN_BCB_OFFSET__                                                      0x1FFF

// Signed input offset to be added to the corresponding video data prior to matrix multiplication
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_IN_RCR_OFFSET                 (REGTX_VP3 | 0x0038)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_IN_RCR_OFFSET__                                                      0x1FFF

// Signed output offset to be added to the corresponding video data after matrix multiplication
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_OUT_GY_OFFSET                 (REGTX_VP3 | 0x003A)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_OUT_GY_OFFSET__                                                      0x1FFF

// Signed output offset to be added to the corresponding video data after matrix multiplication
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_OUT_BCB_OFFSET                (REGTX_VP3 | 0x003C)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_OUT_BCB_OFFSET__                                                      0x1FFF

// Signed output offset to be added to the corresponding video data after matrix multiplication
#define REG_ADDR__VP__CMS__CSC0__MULTI_CSC_OUT_RCR_OFFSET                (REGTX_VP3 | 0x003E)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC0__MULTI_CSC_OUT_RCR_OFFSET__                                                      0x1FFF

// Module features
#define REG_ADDR__VP__CMS__CSC1__FEATURES                                (REGTX_VP3 | 0x0080)
// (Undefined, Bits 10)
// Retiming VTG/FIFO
#define BIT_MSK__VP__CMS__CSC1__FEATURES__RETIMING                                              0x400
// (Undefined, Bits 9)
// Pixel capture at the output
#define BIT_MSK__VP__CMS__CSC1__FEATURES__PIXCAP_OUT                                            0x200
// (Undefined, Bits 8)
// Range Clip
#define BIT_MSK__VP__CMS__CSC1__FEATURES__RANGE_CLIP                                            0x100
// (Undefined, Bits 7)
// Dither/Round
#define BIT_MSK__VP__CMS__CSC1__FEATURES__DITHER_RND                                            0x80
// (Undefined, Bits 6)
// Chroma vertical subsampler
#define BIT_MSK__VP__CMS__CSC1__FEATURES__C422_C420                                             0x40
// (Undefined, Bits 5)
// Chroma horizontal subsampler
#define BIT_MSK__VP__CMS__CSC1__FEATURES__C444_C422                                             0x20
// (Undefined, Bits 4)
// Multi-Colorspace Converter
#define BIT_MSK__VP__CMS__CSC1__FEATURES__MULTI_CSC                                             0x10
// (Undefined, Bits 3)
// Chroma horizontal upsampler
#define BIT_MSK__VP__CMS__CSC1__FEATURES__C422_C444                                             0x08
// (Undefined, Bits 2)
// Chroma vertical upsampler
#define BIT_MSK__VP__CMS__CSC1__FEATURES__C420_C422                                             0x04
// (Undefined, Bits 1)
// Pixel capture at the input
#define BIT_MSK__VP__CMS__CSC1__FEATURES__PIXCAP_IN                                             0x02
// (Undefined, Bits 0)
// CSC module enabled
#define BIT_MSK__VP__CMS__CSC1__FEATURES__CSC_CORE                                              0x01

// Number of data bits for the datapath
#define REG_ADDR__VP__CMS__CSC1__DATA_BITS_VALUE                         (REGTX_VP3 | 0x0082)
// (Undefined, Bits 7:0)
//
#define BIT_MSK__VP__CMS__CSC1__DATA_BITS_VALUE__                                                      0xFF

// Device build time stamp
#define REG_ADDR__VP__CMS__CSC1__BUILD_TIME                              (REGTX_VP3 | 0x0084)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VP__CMS__CSC1__BUILD_TIME__                                                      0xFFFFFFFF

// Colorspace converter configuration
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG                        (REGTX_VP3 | 0x00A0)
// (Undefined, Bits 11)
// Disable saturation
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__DISABLE_SATURATION                                    0x800
// (Undefined, Bits 10)
// Select dither/round operation
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__DITHER_ENABLE                                         0x400
// (Undefined, Bits 9)
// Select input color space
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__IN_RGB                                                0x200
// (Undefined, Bits 8)
// Select input levels
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__IN_PC                                                 0x100
// (Undefined, Bits 7:6)
// Select input color standard
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__IN_STD                                                0xC0
// (Undefined, Bits 5)
// Select output color space
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__OUT_RGB                                               0x20
// (Undefined, Bits 4)
// Select output levels
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__OUT_PC                                                0x10
// (Undefined, Bits 3:2)
// Select output color standard
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__OUT_STD                                               0x0C
// (Undefined, Bits 1:0)
// Enable conversion
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__ENABLE                                                0x03

// R1C1 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR1C1                 (REGTX_VP3 | 0x00A2)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR1C1__                                                      0xFFFF

// R1C2 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR1C2                 (REGTX_VP3 | 0x00A4)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR1C2__                                                      0xFFFF

// R1C3 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR1C3                 (REGTX_VP3 | 0x00A6)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR1C3__                                                      0xFFFF

// R2C1 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR2C1                 (REGTX_VP3 | 0x00A8)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR2C1__                                                      0xFFFF

// R2C2 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR2C2                 (REGTX_VP3 | 0x00AA)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR2C2__                                                      0xFFFF

// R2C3 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR2C3                 (REGTX_VP3 | 0x00AC)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR2C3__                                                      0xFFFF

// R3C1 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR3C1                 (REGTX_VP3 | 0x00AE)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR3C1__                                                      0xFFFF

// R3C2 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR3C2                 (REGTX_VP3 | 0x00B0)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR3C2__                                                      0xFFFF

// R3C3 matrix multiplier coefficient. Assumed to be 16-bit signed with 4 integer bits and 12 fractional bits.
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR3C3                 (REGTX_VP3 | 0x00B2)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR3C3__                                                      0xFFFF

// Signed input offset to be added to the corresponding video data prior to matrix multiplication
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_IN_GY_OFFSET                  (REGTX_VP3 | 0x00B4)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_IN_GY_OFFSET__                                                      0x1FFF

// Signed input offset to be added to the corresponding video data prior to matrix multiplication
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_IN_BCB_OFFSET                 (REGTX_VP3 | 0x00B6)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_IN_BCB_OFFSET__                                                      0x1FFF

// Signed input offset to be added to the corresponding video data prior to matrix multiplication
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_IN_RCR_OFFSET                 (REGTX_VP3 | 0x00B8)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_IN_RCR_OFFSET__                                                      0x1FFF

// Signed output offset to be added to the corresponding video data after matrix multiplication
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_OUT_GY_OFFSET                 (REGTX_VP3 | 0x00BA)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_OUT_GY_OFFSET__                                                      0x1FFF

// Signed output offset to be added to the corresponding video data after matrix multiplication
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_OUT_BCB_OFFSET                (REGTX_VP3 | 0x00BC)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_OUT_BCB_OFFSET__                                                      0x1FFF

// Signed output offset to be added to the corresponding video data after matrix multiplication
#define REG_ADDR__VP__CMS__CSC1__MULTI_CSC_OUT_RCR_OFFSET                (REGTX_VP3 | 0x00BE)
// (Undefined, Bits 12:0)
//
#define BIT_MSK__VP__CMS__CSC1__MULTI_CSC_OUT_RCR_OFFSET__                                                      0x1FFF

// Configure chroma downsampler
#define REG_ADDR__VP__CMS__CSC1__C444_C422_CONFIG                        (REGTX_VP3 | 0x00C0)
// (Undefined, Bits 2)
// Disable Lowpass filter
#define BIT_MSK__VP__CMS__CSC1__C444_C422_CONFIG__DISABLE_FILTER                                        0x04
// (Undefined, Bits 1)
// Reserved
#define BIT_MSK__VP__CMS__CSC1__C444_C422_CONFIG__RESERVED_1                                            0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__CSC1__C444_C422_CONFIG__ENABLE                                                0x01

// Configure chroma vertical downsampler
#define REG_ADDR__VP__CMS__CSC1__C422_C420_CONFIG                        (REGTX_VP3 | 0x00C2)
// (Undefined, Bits 3)
// Polarity of alternating Cb/Cr
#define BIT_MSK__VP__CMS__CSC1__C422_C420_CONFIG__OUT_CB_CR_POLARITY                                    0x08
// (Undefined, Bits 2)
// Select whether Cb or Cr bus are used as chroma output
#define BIT_MSK__VP__CMS__CSC1__C422_C420_CONFIG__OUT_CB_OR_CR                                          0x04
// (Undefined, Bits 1)
// Bypass module to reduce latency
#define BIT_MSK__VP__CMS__CSC1__C422_C420_CONFIG__BYPASS                                                0x02
// (Undefined, Bits 0)
// Enable
#define BIT_MSK__VP__CMS__CSC1__C422_C420_CONFIG__ENABLE                                                0x01

// Dither configuration
#define REG_ADDR__VP__CMS__CSC1__DITHER_CONFIG                           (REGTX_VP3 | 0x00C4)
// (Undefined, Bits 4)
// LFSR spatial dual mode
#define BIT_MSK__VP__CMS__CSC1__DITHER_CONFIG__SPATIAL_DUAL                                          0x10
// (Undefined, Bits 3)
// LFSR spatial mode
#define BIT_MSK__VP__CMS__CSC1__DITHER_CONFIG__SPATIAL_ENABLE                                        0x08
// (Undefined, Bits 2)
// Select dither/round operation
#define BIT_MSK__VP__CMS__CSC1__DITHER_CONFIG__RND_ENABLE                                            0x04
// (Undefined, Bits 1:0)
// Select bits
#define BIT_MSK__VP__CMS__CSC1__DITHER_CONFIG__MODE                                                  0x03

// Enables dynamic range clipping/saturation to programmable levels. In RGB mode, the Y parameters are used for all 3 color components. There is no bypass or enable.
#define REG_ADDR__VP__CMS__CSC1__RANGE_CLIP_CONFIG                       (REGTX_VP3 | 0x00C6)
// (Undefined, Bits 0)
// Select color space
#define BIT_MSK__VP__CMS__CSC1__RANGE_CLIP_CONFIG__INPUT_IS_RGB                                          0x01

// Y/RGB minimum clip level
#define REG_ADDR__VP__CMS__CSC1__RANGE_CLIP_Y_MIN                        (REGTX_VP3 | 0x00C8)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CSC1__RANGE_CLIP_Y_MIN__                                                      0xFFF

// Y/RGB maximum clip level
#define REG_ADDR__VP__CMS__CSC1__RANGE_CLIP_Y_MAX                        (REGTX_VP3 | 0x00CA)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CSC1__RANGE_CLIP_Y_MAX__                                                      0xFFF

// Cb/Cr minimum clip level
#define REG_ADDR__VP__CMS__CSC1__RANGE_CLIP_C_MIN                        (REGTX_VP3 | 0x00CC)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CSC1__RANGE_CLIP_C_MIN__                                                      0xFFF

// Cb/Cr maximum clip level
#define REG_ADDR__VP__CMS__CSC1__RANGE_CLIP_C_MAX                        (REGTX_VP3 | 0x00CE)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CSC1__RANGE_CLIP_C_MAX__                                                      0xFFF

// Configure capture module
#define REG_ADDR__VP__CMS__CSC1__PIXCAP_OUT_CONFIG                       (REGTX_VP3 | 0x00D0)
// (Undefined, Bits 1)
// Enable auto-capture mode
#define BIT_MSK__VP__CMS__CSC1__PIXCAP_OUT_CONFIG__AUTO_TRIGGER                                          0x02
// (Undefined, Bits 0)
// Show capture point
#define BIT_MSK__VP__CMS__CSC1__PIXCAP_OUT_CONFIG__SHOW_POINT                                            0x01

// Configure capture module
#define REG_ADDR__VP__CMS__CSC1__PIXCAP_OUT_CONTROL                      (REGTX_VP3 | 0x00D1)
// (Undefined, Bits 0)
// Trigger pixel value capture
#define BIT_MSK__VP__CMS__CSC1__PIXCAP_OUT_CONTROL__TRIGGER                                               0x01

// Capture module status
#define REG_ADDR__VP__CMS__CSC1__PIXCAP_OUT_STATUS                       (REGTX_VP3 | 0x00D2)
// (Undefined, Bits 1)
// Error occurred, capture location not found
#define BIT_MSK__VP__CMS__CSC1__PIXCAP_OUT_STATUS__ERROR                                                 0x02
// (Undefined, Bits 0)
// Capture module is busy, value has not been captured yet
#define BIT_MSK__VP__CMS__CSC1__PIXCAP_OUT_STATUS__BUSY                                                  0x01

// Specify captured pixel number
#define REG_ADDR__VP__CMS__CSC1__PIXCAP_OUT_PIXEL                        (REGTX_VP3 | 0x00D4)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__PIXCAP_OUT_PIXEL__                                                      0xFFFF

// Specify captured line number
#define REG_ADDR__VP__CMS__CSC1__PIXCAP_OUT_LINE                         (REGTX_VP3 | 0x00D6)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VP__CMS__CSC1__PIXCAP_OUT_LINE__                                                      0xFFFF

// Captured Y value of selected pixel
#define REG_ADDR__VP__CMS__CSC1__PIXCAP_OUT_Y                            (REGTX_VP3 | 0x00D8)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CSC1__PIXCAP_OUT_Y__                                                      0xFFF

// Captured Cb value of selected pixel
#define REG_ADDR__VP__CMS__CSC1__PIXCAP_OUT_CB                           (REGTX_VP3 | 0x00DA)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CSC1__PIXCAP_OUT_CB__                                                      0xFFF

// Captured Cr value of selected pixel
#define REG_ADDR__VP__CMS__CSC1__PIXCAP_OUT_CR                           (REGTX_VP3 | 0x00DC)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VP__CMS__CSC1__PIXCAP_OUT_CR__                                                      0xFFF

//***************************************************************************
// REGTX_VIDGEN. Address: 40
// Module features
#define REG_ADDR__VG__FEATURES                                           (REGTX_VIDGEN | 0x0000)
// (Undefined, Bits 0)
// Video Generator Module
#define BIT_MSK__VG__FEATURES__VIDGEN                                                0x01

// Device build time stamp
#define REG_ADDR__VG__BUILD_TIME                                         (REGTX_VIDGEN | 0x0008)
// (Undefined, Bits 31:0)
//
#define BIT_MSK__VG__BUILD_TIME__                                                      0xFFFFFFFF

// Software reset
#define REG_ADDR__VG__SOFT_RESET                                         (REGTX_VIDGEN | 0x000C)
// (Undefined, Bits 0)
//
#define BIT_MSK__VG__SOFT_RESET__                                                      0x01

// Number of data bits for the datapath
#define REG_ADDR__VG__DATA_BITS_VALUE                                    (REGTX_VIDGEN | 0x000E)
// (Undefined, Bits 7:0)
//
#define BIT_MSK__VG__DATA_BITS_VALUE__                                                      0xFF

// Video Generator Configuration
#define REG_ADDR__VG__VIDGEN_CONFIG                                      (REGTX_VIDGEN | 0x0010)
// (Undefined, Bits 5)
// Replace Cr data with Cr color value when input timing is used
#define BIT_MSK__VG__VIDGEN_CONFIG__REPLACE_CR                                            0x20
// (Undefined, Bits 4)
// Replace Cb data with Cb color value when input timing is used
#define BIT_MSK__VG__VIDGEN_CONFIG__REPLACE_CB                                            0x10
// (Undefined, Bits 3)
// Replace Y data with Y color value when input timing is used
#define BIT_MSK__VG__VIDGEN_CONFIG__REPLACE_Y                                             0x08
// (Undefined, Bits 2)
// Select chroma mode
#define BIT_MSK__VG__VIDGEN_CONFIG__CHROMA_MODE                                           0x04
// (Undefined, Bits 1)
// Select timing for video generator
#define BIT_MSK__VG__VIDGEN_CONFIG__USE_INPUT_TIMING                                      0x02
// (Undefined, Bits 0)
// Enable Video Generator
#define BIT_MSK__VG__VIDGEN_CONFIG__ENABLE                                                0x01

// Y color value
#define REG_ADDR__VG__VIDGEN_COLOR_Y                                     (REGTX_VIDGEN | 0x0012)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VG__VIDGEN_COLOR_Y__                                                      0xFFF

// Cb color value
#define REG_ADDR__VG__VIDGEN_COLOR_CB                                    (REGTX_VIDGEN | 0x0014)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VG__VIDGEN_COLOR_CB__                                                      0xFFF

// Cr color value
#define REG_ADDR__VG__VIDGEN_COLOR_CR                                    (REGTX_VIDGEN | 0x0016)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VG__VIDGEN_COLOR_CR__                                                      0xFFF

// End of horizontal sync pulse -1
#define REG_ADDR__VG__VIDGEN_HORIZONTAL_SYNC_END                         (REGTX_VIDGEN | 0x0020)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__VIDGEN_HORIZONTAL_SYNC_END__                                                      0xFFFF

// Start of horizontal active video line -1
#define REG_ADDR__VG__VIDGEN_HORIZONTAL_ACTIVE_VIDEO_START               (REGTX_VIDGEN | 0x0022)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__VIDGEN_HORIZONTAL_ACTIVE_VIDEO_START__                                                      0xFFFF

// Midpoint of horizontal line -2
#define REG_ADDR__VG__VIDGEN_HALFLINE                                    (REGTX_VIDGEN | 0x0024)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__VIDGEN_HALFLINE__                                                      0xFFFF

// End of horizontal active video line -1
#define REG_ADDR__VG__VIDGEN_HORIZONTAL_ACTIVE_VIDEO_END                 (REGTX_VIDGEN | 0x0026)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__VIDGEN_HORIZONTAL_ACTIVE_VIDEO_END__                                                      0xFFFF

// End of horizontal line -2
#define REG_ADDR__VG__VIDGEN_END_OF_LINE                                 (REGTX_VIDGEN | 0x0028)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__VIDGEN_END_OF_LINE__                                                      0xFFFF

// End of vertical sync pulse (in half-lines) -1
#define REG_ADDR__VG__VIDGEN_VERTICAL_SYNC_END                           (REGTX_VIDGEN | 0x0030)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__VIDGEN_VERTICAL_SYNC_END__                                                      0xFFFF

// Trigger point for processing to start (in half-lines) -1
#define REG_ADDR__VG__VIDGEN_TRIGGER_START                               (REGTX_VIDGEN | 0x0032)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__VIDGEN_TRIGGER_START__                                                      0xFFFF

// Start of vertical active video (in half-lines) -1
#define REG_ADDR__VG__VIDGEN_VERTICAL_ACTIVE_VIDEO_START                 (REGTX_VIDGEN | 0x0034)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__VIDGEN_VERTICAL_ACTIVE_VIDEO_START__                                                      0xFFFF

// End of vertical active video (in half-lines) -1
#define REG_ADDR__VG__VIDGEN_VERTICAL_ACTIVE_VIDEO_END                   (REGTX_VIDGEN | 0x0036)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__VIDGEN_VERTICAL_ACTIVE_VIDEO_END__                                                      0xFFFF

// End of vertical frame (in half-lines) -1
#define REG_ADDR__VG__VIDGEN_VERTICAL_END_OF_FRAME                       (REGTX_VIDGEN | 0x0038)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__VIDGEN_VERTICAL_END_OF_FRAME__                                                      0xFFFF

// Bank update request for Video Generator parameters. (write any data value - this is a strobe only)
#define REG_ADDR__VG__VIDGEN_UPDATE_REQUEST                              (REGTX_VIDGEN | 0x003E)
// (Undefined, Bits 0)
//
#define BIT_MSK__VG__VIDGEN_UPDATE_REQUEST__                                                      0x01

// Read/write bank configuration for video generator parameters
#define REG_ADDR__VG__VIDGEN_BANK_CONFIG                                 (REGTX_VIDGEN | 0x003F)
// (Undefined, Bits 2)
// Select when to update double-buffer
#define BIT_MSK__VG__VIDGEN_BANK_CONFIG__UPDATE_MODE                                           0x04
// (Undefined, Bits 1)
// Select which bank to write
#define BIT_MSK__VG__VIDGEN_BANK_CONFIG__WRITE_BANK                                            0x02
// (Undefined, Bits 0)
// Select which bank to read
#define BIT_MSK__VG__VIDGEN_BANK_CONFIG__READ_BANK                                             0x01

// Test Pattern Generator Configuration
#define REG_ADDR__VG__TPG_CONFIG                                         (REGTX_VIDGEN | 0x0040)
// (Undefined, Bits 4)
// Enable judder bar
#define BIT_MSK__VG__TPG_CONFIG__JUDDER_BAR_ENABLE                                     0x10
// (Undefined, Bits 3)
// Enable solid color output
#define BIT_MSK__VG__TPG_CONFIG__SOLID_COLOR_ENABLE                                    0x08
// (Undefined, Bits 2:1)
// Select output oversample rate
#define BIT_MSK__VG__TPG_CONFIG__OVERSAMPLE_RATE                                       0x06
// (Undefined, Bits 0)
// Enable Test Pattern Generator
#define BIT_MSK__VG__TPG_CONFIG__ENABLE                                                0x01

// Judder bar configuration
#define REG_ADDR__VG__TPG_JUDDER_CONFIG                                  (REGTX_VIDGEN | 0x0041)
// (Undefined, Bits 7:4)
// Judder bar decrement value
#define BIT_MSK__VG__TPG_JUDDER_CONFIG__DECREMENT                                             0xF0
// (Undefined, Bits 3:0)
// Judder bar increment value
#define BIT_MSK__VG__TPG_JUDDER_CONFIG__INCREMENT                                             0x0F

// Judder bar start position
#define REG_ADDR__VG__TPG_JUDDER_START_COUNT                             (REGTX_VIDGEN | 0x0042)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__TPG_JUDDER_START_COUNT__                                                      0xFFFF

// Judder bar stop/reverse position
#define REG_ADDR__VG__TPG_JUDDER_REVERSE_COUNT                           (REGTX_VIDGEN | 0x0044)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__TPG_JUDDER_REVERSE_COUNT__                                                      0xFFFF

// Judder bar width
#define REG_ADDR__VG__TPG_JUDDER_BAR_WIDTH                               (REGTX_VIDGEN | 0x0046)
// (Undefined, Bits 7:0)
//
#define BIT_MSK__VG__TPG_JUDDER_BAR_WIDTH__                                                      0xFF

// Trigger only
#define REG_ADDR__VG__TPG_JUDDER_RESTART                                 (REGTX_VIDGEN | 0x0047)
// (Undefined, Bits 0)
//
#define BIT_MSK__VG__TPG_JUDDER_RESTART__                                                      0x01

// Start address for TPG RAMs - Write address here first, each access of tpg_linegen, tpg_linemap, tpg_lut_* will increment this register
#define REG_ADDR__VG__TPG_ADDR                                           (REGTX_VIDGEN | 0x0048)
// (Undefined, Bits 15:0)
//
#define BIT_MSK__VG__TPG_ADDR__                                                      0xFFFF

// TPG LineGen value - Write start address in tpg_addr first. Each read/write access of tpg_linegen will increment the address
#define REG_ADDR__VG__TPG_LINEGEN                                        (REGTX_VIDGEN | 0x004C)
// (Undefined, Bits 17:0)
//
#define BIT_MSK__VG__TPG_LINEGEN__                                                      0x3FFFF

// TPG LineMap value - Write start address in tpg_addr first. Each read/write access of tpg_linemap will increment the address
#define REG_ADDR__VG__TPG_LINEMAP                                        (REGTX_VIDGEN | 0x0050)
// (Undefined, Bits 6:0)
//
#define BIT_MSK__VG__TPG_LINEMAP__                                                      0x7F

// TPG LUT Y value - Write start address in tpg_addr first. Each read/write access of tpg_lut_y will increment the address
#define REG_ADDR__VG__TPG_LUT_Y                                          (REGTX_VIDGEN | 0x0052)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VG__TPG_LUT_Y__                                                      0xFFF

// TPG LUT Cb value - Write start address in tpg_addr first. Each read/write access of tpg_lut_cb will increment the address
#define REG_ADDR__VG__TPG_LUT_CB                                         (REGTX_VIDGEN | 0x0054)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VG__TPG_LUT_CB__                                                      0xFFF

// TPG LUT Cr value - Write start address in tpg_addr first. Each read/write access of tpg_lut_cr will increment the address
#define REG_ADDR__VG__TPG_LUT_CR                                         (REGTX_VIDGEN | 0x0056)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VG__TPG_LUT_CR__                                                      0xFFF

// Solid Color - Y
#define REG_ADDR__VG__TPG_SOLID_COLOR_Y                                  (REGTX_VIDGEN | 0x0058)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VG__TPG_SOLID_COLOR_Y__                                                      0xFFF

// Solid Color - Cb
#define REG_ADDR__VG__TPG_SOLID_COLOR_CB                                 (REGTX_VIDGEN | 0x005A)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VG__TPG_SOLID_COLOR_CB__                                                      0xFFF

// Solid Color - Cr
#define REG_ADDR__VG__TPG_SOLID_COLOR_CR                                 (REGTX_VIDGEN | 0x005C)
// (Undefined, Bits 11:0)
//
#define BIT_MSK__VG__TPG_SOLID_COLOR_CR__                                                      0xFFF

// Select fixed patterns
#define REG_ADDR__VG__TPG_FIXED_PATTERN                                  (REGTX_VIDGEN | 0x005E)
// (Undefined, Bits 3:2)
// Pattern #1 (between last user pattern and max pattern index)
#define BIT_MSK__VG__TPG_FIXED_PATTERN__PATTERN1                                              0x0C
// (Undefined, Bits 1:0)
// Pattern #0 (max pattern index)
#define BIT_MSK__VG__TPG_FIXED_PATTERN__PATTERN0                                              0x03

#endif // __SI_VIDPATH_REGS_H__
