/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CC_RENDER_VSB_H__
#define __CC_RENDER_VSB_H__
#include <stdlib.h>
#include <stdio.h>
//#include "common.h"
//#include "lib_rect.h"

//nclude "cc_commons.h"
//screen size
//#include "display.h"

#include "mt_unf_cc.h"
#define DEBUG_LEVEL   0

#define CLOSEDCP_MAX_CHAR_NUM             100
#define CLOSEDCP_MAX_PES_NUM              16
#define CLOSEDCP_MAX_DECODED_MAX_SIZE     3 * 1024
#define CLOSEDCP_BACK_MARGIN_NUM          5

#define CC_DEBUG_LEVEL                                 0
#define CC_FONT_WIDTH                                12
#define CC_FONT_HEIGHT                               24
#define CC_FONT_INTERVEL                            2
#define DISPLAY_CC_LINE_INTERVAL            (CC_FONT_HEIGHT + 4)

#define EIA608_SCREEN_ROWS 15
#define EIA608_SCREEN_COLUMNS 32

/*!
    the status of PES comprise
*/

typedef struct
{
    mt_u32 left;
    mt_u32 right;
    mt_u32 top;
    mt_u32 bottom;
}rect_t; 

typedef struct
{
    mt_u32 x;
    mt_u32 y;
    mt_u32 w;
    mt_u32 h;
}rect_size_t;

/*!
   A point defined by x, y coordinates.
  */
typedef struct
{
  /*!
     The x coordinate of the point.
    */
  mt_s16 x;
  /*!
     The y coordinate of the point.
    */
  mt_s16 y;
}point_t;


/*****************************************************************************
 * Local prototypes
 *****************************************************************************/
typedef enum
{
    EIA608_MODE_POPON = 0,
    EIA608_MODE_ROLLUP_2 = 1,
    EIA608_MODE_ROLLUP_3 = 2,
    EIA608_MODE_ROLLUP_4 = 3,
    EIA608_MODE_PAINTON = 4,
    EIA608_MODE_TEXT = 5
} eia608_mode_t;

typedef enum
{
    CC_COLOR_TRANSPARENT,
    CC_COLOR_BLUE,
    CC_COLOR_GREEN,
    CC_COLOR_RED,
    CC_COLOR_WHITE,
    CC_COLOR_YELLOW,
    CC_COLOR_BLACK,
    CC_COLOR_CYAN,
    CC_COLOR_MAGENTA,
}CcDisplayColor_t;

enum cc_font {
    CC_FONT_REGULAR,
    CC_FONT_ITALICS,
    CC_FONT_UNDERLINED,
    CC_FONT_UNDERLINED_ITALICS,
};

enum cc_charset {
    CCSET_BASIC_AMERICAN,
    CCSET_SPECIAL_AMERICAN,
    CCSET_EXTENDED_SPANISH_FRENCH_MISC,
    CCSET_EXTENDED_PORTUGUESE_GERMAN_DANISH,
};

typedef enum
{
    EIA_608_708,
    ARIB_STD24,
    EIA_DTVCC_708,
    PROTOCOL_UNKNOW
}CC_PROTOCOL;

    typedef struct {
        mt_u32 PTS;
        mt_u8  fNotPTS;
        mt_u8  data_group_id;
        mt_u8  is_management;
        mt_u8  data_group_version;
        mt_u8  data_group_link_number;
        mt_u8  last_data_group_link_number;
        mt_u8  TMD;
        mt_u8  num_languages;
        mt_u8  language_tag;
        mt_u8  DMF;
        mt_u8  ISO_639_language_code[3];
        mt_u8  Format;
        mt_u8  TCS;
        mt_u8  rollup_mode;
        mt_u8  data_unit_parameter;
        mt_u32 data_unit_size;
    } CC_HeaderInfo;

    typedef struct {
        /* Active position */
        mt_u8 fAPB;
        mt_u8 fAPF;
        mt_u8 fAPD;
        mt_u8 fAPU;
        mt_u8 fAPR;
        mt_u8 fPAPF;
        mt_u8 fAPS;
        mt_u8 paramPAPF[1];
        mt_u8 paramAPS[2];

        /* Colour Control */
        mt_u8 fBKF;
        mt_u8 fRDF;
        mt_u8 fGRF;
        mt_u8 fYLF;
        mt_u8 fBLF;
        mt_u8 fMGF;
        mt_u8 fCNF;
        mt_u8 fWHF;
        mt_u8 fCOL;
        mt_u8 charColor[1];
        mt_u8 paramCOL[2];

        /* Character Size */
        mt_u8 fSSZ;
        mt_u8 fMSZ;
        mt_u8 fNSZ;
        mt_u8 charSize[1];
        mt_u8 fSZX;
        mt_u8 paramSZX[1];

        /* Flashing */
        mt_u8 fFLC;
        mt_u8 paramFLC[1];

        /* Pattern Polarity */
        mt_u8 fPOL;
        mt_u8 paramPOL[1];

        /* HIGHLIGHTING CHARACTER BLOCK */
        mt_u8 fHLC;
        mt_u8 paramHLC[1];

        /* Repeat Character */
        mt_u8 fRPC;
        mt_u8 paramRPC[1];

        /* Lining */
        mt_u8 fSPL;
        mt_u8 fSTL;

        /* Time */
        mt_u8 fTIME;
        mt_u8 paramTIME[2];

        /* Clear screen */
        mt_u8 fCS;

        /* Control Sequence Introducer */
        mt_u8 fCSI;
        mt_u8 fSWF;
        mt_u8 fRCS;
        mt_u8 fACPS;
        mt_u8 fSDF;
        mt_u8 fSDP;
        mt_u8 fSSM;
        mt_u8 fSHS;
        mt_u8 fSVS;
        mt_u8 fORN;
        mt_u8 fPRA;
        mt_u8 paramSWF[3];        /* Set Writing Format                             */
        mt_u8 paramRCS[3];        /* Raster Colour command                          */
        mt_u8 paramACPS_H[3];     /* Active Coordinate Position Set(Horizontal)     */
        mt_u8 paramACPS_V[3];     /* Active Coordinate Position Set(Vertical)       */
        mt_u8 paramSDF_H[3];      /* SET DISPLAY FORMAT(Horizontal)                 */
        mt_u8 paramSDF_V[3];      /* SET DISPLAY FORMAT(Vertical)                   */
        mt_u8 paramSDP_H[3];      /* Set Display Position(Horizontal)               */
        mt_u8 paramSDP_V[3];      /* Set Display Position(Vertical)                 */
        mt_u8 paramSSM_H[3];      /* Character composition dot designation(width)   */
        mt_u8 paramSSM_V[3];      /* Character composition dot designation(height)  */
        mt_u8 paramSHS[3];        /* Set Horizontal Spacing                         */
        mt_u8 paramSVS[3];        /* Set Vertical Spacing                           */
        mt_u8 paramORN[1];        /* Ornament Control                               */
        mt_u8 paramORN_C[8];      /* Ornament Colour Control                        */
        mt_u8 paramPRA[3];        /* Built-in sound replay                          */

        mt_u32 APB_num;
        mt_u32 APF_num;
        mt_u32 APD_num;
        mt_u32 APU_num;
        mt_u32 APR_num;

        mt_u32 num1_SWF;
        mt_u32 num1_RCS;
        mt_u32 num1_ACPS;
        mt_u32 num2_ACPS;
        mt_u32 num1_SDF;
        mt_u32 num2_SDF;
        mt_u32 num1_SDP;
        mt_u32 num2_SDP;
        mt_u32 num1_SSM;
        mt_u32 num2_SSM;
        mt_u32 num1_SHS;
        mt_u32 num1_SVS;
        mt_u32 num1_ORN;
        mt_u32 num2_ORN;
        mt_u32 num1_PRA;

        mt_u8 decodedData[128];  /* Max character num at one line */
        mt_u32 charcount;

    } CC_DataInfo;


typedef struct
{
    mt_u8         tag;
    mt_u32        langCode;

} CLOSEDCP_STREAMINF_S;

typedef struct {
    mt_u32                 color1;
    mt_u32                 color2;
    mt_u32                 osdWidth;
    mt_u32                 osdHeight;
    mt_u32                 xPos;
    mt_u32                 yPos;
    mt_u32                 xPosBase;
    mt_u32                 yPosBase;
    mt_u32                 drawWidth;
    mt_u32                 drawHeight;
    mt_u32                 fontSize;
    mt_u32                 interval_x;
    mt_u32                 interval_y;
    mt_u32                 block_x;
    mt_u32                 block_y;
    mt_u8                  isCS;
    CC_DataInfo              dataInfo;
} CLOSEDCP_DrawCharInfo;

typedef struct {
    CC_HeaderInfo            headerInfo;
    CLOSEDCP_DrawCharInfo    drawCharInfo[CLOSEDCP_MAX_CHAR_NUM];
    CLOSEDCP_DrawCharInfo    *curDecodCharInfo;
    CLOSEDCP_DrawCharInfo    *curDrawCharInfo;
    mt_u32                      drawCharNum;
} CLOSEDCP_PesInfo;

struct eia608_screen {
    /* +1 is used to compensate null character of string */
    mt_u8 characters[EIA608_SCREEN_ROWS][EIA608_SCREEN_COLUMNS+1];
    mt_u8 charsets[EIA608_SCREEN_ROWS][EIA608_SCREEN_COLUMNS+1];
    mt_u8 colors[EIA608_SCREEN_ROWS][EIA608_SCREEN_COLUMNS+1];
    mt_u8 fonts[EIA608_SCREEN_ROWS][EIA608_SCREEN_COLUMNS+1];
    /*
     * Bitmask of used rows; if a bit is not set, the
     * corresponding row is not used.
     * for setting row 1  use row | (1 << 0)
     * for setting row 15 use row | (1 << 14)
     */
    mt_s16 row_used;
};
typedef struct eia608_screen eia608_screen;

typedef struct
{
    int real_time;
    struct eia608_screen screen[2];
    int active_screen;
    mt_u8 cursor_row;
    mt_u8 cursor_column;
    mt_u8 cursor_color;
    mt_u8 cursor_font;
    mt_u8 cursor_charset;
    eia608_mode_t mode;
    mt_u8 prev_cmd[2];
    int rollup;
    int screen_touched;
	
	mt_u32 start_time;
	mt_u32 end_time;
    mt_u16 old_unicode[EIA608_SCREEN_COLUMNS+1];
	int paint_num;	
    MT_UNF_CC_DISPLAY_PARAM_S  cc_old_display_param;

    mt_u32 color;
    mt_u32 bgColor;
    mt_u32                 u32DisplayWidth;
    mt_u32                 u32DisplayHeight;

    MT_UNF_CC_GETPTS_CB_FN       pfnCCGetPts; 
    MT_UNF_CC_DISPLAY_CB_FN      pfnCCDisplay;
    MT_UNF_CC_GETTEXTSIZE_CB_FN  pfnCCGetTextSize;
    MT_UNF_CC_BLIT_CB_FN         pfnBlit;
    MT_UNF_CC_VBI_CB_FN          pfnVBIOutput;
    MT_UNF_CC_XDS_CB_FN          pfnXDSOutput; 

} CLOSEDCP_PARAM_S;

typedef struct {
	MT_BOOL ccTaskRunning;
	mt_u32* p_stack;

	
/////////////////////////////////////////////////
    CLOSEDCP_PARAM_S ccParam;


    CLOSEDCP_STREAMINF_S ccInfo[2];
    CLOSEDCP_STREAMINF_S* curCcInfo;

    mt_u8 ccStrNums;
    mt_s8 ccStrCurr;
    MT_BOOL ccStrStarted;

    CLOSEDCP_PesInfo pesInfo[16];
    CLOSEDCP_PesInfo *curDecodPesInfo;
    CLOSEDCP_PesInfo *curDrawPesInfo;
    mt_u32 decodedPesCount;
    mt_u32 drawPesCount;
    mt_u32 restDecodedPesCount;
    mt_u8 isSetParam;

    mt_u8 doDecode;
	
	int show_quantity; 
    MT_HANDLE cc708_handle;
    
} sCcContext;



/*
*  CC global function
*/
ulong cc_init_vsb(MT_UNF_CC_PARAM_S *pstCCParam, mt_u32 stack_size, CC_PROTOCOL cc_protocol,void *cb);
void cc_set_show_mode(int show_num);
mt_s32 cc_deinit_vsb(void);

MT_BOOL  getCcDisplayOnOff(void);
void setCcDisplayOnOff(sCcContext* pCcContext, MT_BOOL  onoff); /*Open:  onoff = TRUE,  Close: onoff = FALSE*/

void setCcReset(sCcContext* pCcContext);

#endif
