/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "string.h"
#include <sys/time.h>
#include "mt_type.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "mt_common.h"
#include "mtos_mem.h"
#include "mtos_task.h"
#include "mt_unf_cc.h"
#include "mt_cc_fifo.h"
#include "mt_cc_parse.h"
#include "com_cc708.h"

#ifdef __LINUX__
#define DMX_INTERFACE
#endif

//#define DEBUG_CC
#define DEBUG_LEVEL  0

#ifdef DEBUG_CC
//#define CC_PRINTF(level, format, args...) if(level <= DEBUG_LEVEL) {char msg[100]; sprintf(msg,format,##args);printf("[CC: %dms] %s\n", time_ms(), msg); fflush(stdout);}
#define CC_PRINTF(level, format, args...) if(level < DEBUG_LEVEL) {char msg[100]; snprintf(msg,100,format,##args); printf("\r\n [CC: ] %s\n", msg);}
#else
#define CC_PRINTF(args...)
#endif

#define SHOW_TB_OFFSET      (20)
#define SHOW_LR_OFFSET      (50)
#define SHOW_LINE_HEIGHT    (50)

static sCcContext *ccContext=NULL;
static CC_PROTOCOL ccProtocol = ARIB_STD24;
static CC_PROTOCOL ccProtocolTransfor = ARIB_STD24;
static MT_BOOL ccDisplayOnOff = MT_FALSE;
static mt_u8 new_channel = 1;
static MT_BOOL g_cc_paint = FALSE;

mt_u8 *CcMalloc(mt_u32 size, mt_u32 align_v);
void CcFree(void* addr);
void timeout_write_char(MT_UNF_CC_DISPLAY_PARAM_S cc_display_param);
MT_BOOL CcProcess(CLOSEDCP_PARAM_S * ccParam, mt_u8* cc_data_buff, mt_u32 cc_data_len);

extern MT_S32 CC708_ProcessData(MT_VOID);
extern U32 SYS_GetMS(void);


// Return the value with the bit order reversed.
static mt_u8 reverse8(mt_u8 data)
{
	mt_u8 res = 0;
	int k = 0;

	for (k=0;k<8;k++)
	{
		res <<= 1;
		res |= (data & (0x01 << k) ? 1 : 0);
	}

	return res;
}

mt_u8 *CcMalloc(mt_u32 size, mt_u32 align_v)
{
    mt_u8 *addr = NULL;
    addr = mtos_align_malloc_alias(size, align_v);

    return addr;
}

void CcFree(void* addr)
{
    mtos_align_free_alias((void*)addr);
}

static void getCcColor(mt_u32 color, MT_UNF_CC_COLOR_S *p_color )
{

    switch(color)
    {
        case MT_UNF_CC_COLOR_DEFAULT:
            p_color->u8Alpha = 0x00;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0x00;
            break;

        case MT_UNF_CC_COLOR_BLUE:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0xff;
            break;

        case MT_UNF_CC_COLOR_GREEN:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0xff;
            p_color->u8Blue = 0x00;
            break;

        case MT_UNF_CC_COLOR_RED:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0xff;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0x00;
            break;

        case MT_UNF_CC_COLOR_WHITE:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0xff;
            p_color->u8Green = 0xff;
            p_color->u8Blue = 0xff;
            break;

        case MT_UNF_CC_COLOR_YELLOW:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0xff;
            p_color->u8Green = 0xff;
            p_color->u8Blue = 0x00;
            break;

        case MT_UNF_CC_COLOR_BLACK:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0x00;
            break;

        case MT_UNF_CC_COLOR_CYAN:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0xff;
            p_color->u8Blue = 0xff;
            break;

        case MT_UNF_CC_COLOR_MAGENTA:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0xff;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0xff;
            break;

        default:
            p_color->u8Alpha = 0x00;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0x00;
            break;
    }
}

static void getAribCcColor(mt_u32 color, MT_UNF_CC_COLOR_S *p_color )
{

    switch(color)
    {
        case CC_COLOR_TRANSPARENT:
            p_color->u8Alpha = 0x00;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0x00;
            break;

        case CC_COLOR_BLUE:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0xff;
            break;

        case CC_COLOR_GREEN:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0xff;
            p_color->u8Blue = 0x00;
            break;

        case CC_COLOR_RED:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0xff;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0x00;
            break;

        case CC_COLOR_WHITE:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0xff;
            p_color->u8Green = 0xff;
            p_color->u8Blue = 0xff;
            break;

        case CC_COLOR_YELLOW:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0xff;
            p_color->u8Green = 0xff;
            p_color->u8Blue = 0x00;
            break;

        case CC_COLOR_BLACK:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0x00;
            break;

        case CC_COLOR_CYAN:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0xff;
            p_color->u8Blue = 0xff;
            break;

        case CC_COLOR_MAGENTA:
            p_color->u8Alpha = 0xff;
            p_color->u8Red = 0xff;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0xff;
            break;

        default:
            p_color->u8Alpha = 0x00;
            p_color->u8Red = 0x00;
            p_color->u8Green = 0x00;
            p_color->u8Blue = 0x00;
            break;
    }
}

static void CcClearScreen(CLOSEDCP_PARAM_S * ccParam)
{
    MT_UNF_CC_DISPLAY_PARAM_S  cc_display_param;

    if(ccParam)
    {
        CC_PRINTF(2, "CcProcessor::CcClearScreen - [Screen cleared]");
        cc_display_param.enOpt = MT_UNF_CC_OPT_FILLRECT;
        cc_display_param.u32DisplayHeight = ccParam->u32DisplayHeight;
        cc_display_param.u32DisplayWidth = ccParam->u32DisplayWidth;
        cc_display_param.stRect.x = 0;
        cc_display_param.stRect.y = 0;
        cc_display_param.stRect.width = ccParam->u32DisplayWidth-1;
        cc_display_param.stRect.height = ccParam->u32DisplayHeight-1;
        cc_display_param.unDispParam.stFillRect.stColor.u8Alpha = 0x00;
        cc_display_param.unDispParam.stFillRect.stColor.u8Red = 0x00;
        cc_display_param.unDispParam.stFillRect.stColor.u8Green = 0x00;
        cc_display_param.unDispParam.stFillRect.stColor.u8Blue = 0x00;
        ccParam->pfnCCDisplay(MT_NULL, &cc_display_param);
    }
}


MT_BOOL  getCcDisplayOnOff(void)
{
	CC_PRINTF(1, "getCcDisplayOnOff .........ccDisplayOnOff=%d\n", ccDisplayOnOff);
	return ccDisplayOnOff;
}

void setCcDisplayOnOff(sCcContext* pCcContext, MT_BOOL  onoff)
{
    CC_PRINTF(1, "getCcDisplayOnOff  return  onoff = %d,  ccDisplayOnOff= %d\n", onoff,ccDisplayOnOff);
    if(ccDisplayOnOff == onoff)  return;
    ccDisplayOnOff = onoff;

    if(ccProtocol == EIA_608_708)
    {
        cc_fifo_clear();
    }
    if(onoff == MT_FALSE)
    {
        CcClearScreen(&pCcContext->ccParam);
    }
}

void setCcReset(sCcContext* pCcContext)
{
    if(ccProtocol == EIA_608_708)
    {
        cc_fifo_clear();
    }
}

#define SET_FLAG(var, val)   ( (var) |=   ( 1 << (val)) )
#define UNSET_FLAG(var, val) ( (var) &=  ~( 1 << (val)) )
#define CHECK_FLAG(var, val) ( (var) &    ( 1 << (val)) )

#define FFMIN(a,b) ((a) > (b) ? (b) : (a))

static const mt_u16 charset_overrides[4][128] =
{
    [CCSET_BASIC_AMERICAN] = {
        [0x27] = 0x2019,
        [0x2a] = 0x00e1,
        [0x5c] = 0x00e9,
        [0x5e] = 0x00ed,
        [0x5f] = 0x00f3,
        [0x60] = 0x00fa,
        [0x7b] = 0x00e7,
        [0x7c] = 0x00f7,
        [0x7d] = 0x00d1,
        [0x7e] = 0x00f1,
        [0x7f] = 0x2588
    },
    [CCSET_SPECIAL_AMERICAN] = {
        [0x30] = 0x00ae,
        [0x31] = 0x00b0,
        [0x32] = 0x00bd,
        [0x33] = 0x00bf,
        [0x34] = 0x2122,
        [0x35] = 0x00a2,
        [0x36] = 0x00a3,
        [0x37] = 0x266a,
        [0x38] = 0x00e0,
        [0x39] = 0x00A0,
        [0x3a] = 0x00e8,
        [0x3b] = 0x00e2,
        [0x3c] = 0x00ea,
        [0x3d] = 0x00ee,
        [0x3e] = 0x00f4,
        [0x3f] = 0x00fb,
    },
    [CCSET_EXTENDED_SPANISH_FRENCH_MISC] = {
        [0x20] = 0x00c1,
        [0x21] = 0x00c9,
        [0x22] = 0x00d3,
        [0x23] = 0x00da,
        [0x24] = 0x00dc,
        [0x25] = 0x00fc,
        [0x26] = 0x00b4,
        [0x27] = 0x00a1,
        [0x28] = 0xff0a,
        [0x29] = 0x2018,
        [0x2a] = 0x2013,
        [0x2b] = 0x00a9,
        [0x2c] = 0x2120,
        [0x2d] = 0x00b7,
        [0x2e] = 0x201c,
        [0x2f] = 0x201d,
        [0x30] = 0x00c0,
        [0x31] = 0x00c2,
        [0x32] = 0x00c7,
        [0x33] = 0x00c8,
        [0x34] = 0x00ca,
        [0x35] = 0x00cb,
        [0x36] = 0x00eb,
        [0x37] = 0x00ce,
        [0x38] = 0x00cf,
        [0x39] = 0x00ef,
        [0x3a] = 0x00d4,
        [0x3b] = 0x00d9,
        [0x3c] = 0x00f9,
        [0x3d] = 0x00db,
        [0x3e] = 0x00ab,
        [0x3f] = 0x00bb,
    },
    [CCSET_EXTENDED_PORTUGUESE_GERMAN_DANISH] = {
        [0x20] = 0x00c3,
        [0x21] = 0x00e3,
        [0x22] = 0x00cd,
        [0x23] = 0x00cc,
        [0x24] = 0x00ec,
        [0x25] = 0x00d2,
        [0x26] = 0x00f2,
        [0x27] = 0x00d5,
        [0x28] = 0x00f5,
        [0x29] = 0x007b,
        [0x2a] = 0x007d,
        [0x2b] = 0x005c,
        [0x2c] = 0x005e,
        [0x2d] = 0x005f,
        [0x2e] = 0xff5c,
        [0x2f] = 0xff5e,
        [0x30] = 0x00c4,
        [0x31] = 0x00e4,
        [0x32] = 0x00d6,
        [0x33] = 0x00f6,
        [0x34] = 0x00df,
        [0x35] = 0x00a5,
        [0x36] = 0x00a4,
        [0x37] = 0x00a6,
        [0x38] = 0x00c5,
        [0x39] = 0x00e5,
        [0x3a] = 0x00d8,
        [0x3b] = 0x00f8,
        [0x3c] = 0x250c,
        [0x3d] = 0x2510,
        [0x3e] = 0x2514,
        [0x3f] = 0x2518,
    },
};

static const mt_u8 pac2_attribs[32][3] = // Color, font, ident
{
    { CC_COLOR_WHITE,   CC_FONT_REGULAR,            0 },  // 0x40 || 0x60
    { CC_COLOR_WHITE,   CC_FONT_UNDERLINED,         0 },  // 0x41 || 0x61
    { CC_COLOR_GREEN,   CC_FONT_REGULAR,            0 },  // 0x42 || 0x62
    { CC_COLOR_GREEN,   CC_FONT_UNDERLINED,         0 },  // 0x43 || 0x63
    { CC_COLOR_BLUE,    CC_FONT_REGULAR,            0 },  // 0x44 || 0x64
    { CC_COLOR_BLUE,    CC_FONT_UNDERLINED,         0 },  // 0x45 || 0x65
    { CC_COLOR_CYAN,    CC_FONT_REGULAR,            0 },  // 0x46 || 0x66
    { CC_COLOR_CYAN,    CC_FONT_UNDERLINED,         0 },  // 0x47 || 0x67
    { CC_COLOR_RED,     CC_FONT_REGULAR,            0 },  // 0x48 || 0x68
    { CC_COLOR_RED,     CC_FONT_UNDERLINED,         0 },  // 0x49 || 0x69
    { CC_COLOR_YELLOW,  CC_FONT_REGULAR,            0 },  // 0x4a || 0x6a
    { CC_COLOR_YELLOW,  CC_FONT_UNDERLINED,         0 },  // 0x4b || 0x6b
    { CC_COLOR_MAGENTA, CC_FONT_REGULAR,            0 },  // 0x4c || 0x6c
    { CC_COLOR_MAGENTA, CC_FONT_UNDERLINED,         0 },  // 0x4d || 0x6d
    { CC_COLOR_WHITE,   CC_FONT_ITALICS,            0 },  // 0x4e || 0x6e
    { CC_COLOR_WHITE,   CC_FONT_UNDERLINED_ITALICS, 0 },  // 0x4f || 0x6f
    { CC_COLOR_WHITE,   CC_FONT_REGULAR,            0 },  // 0x50 || 0x70
    { CC_COLOR_WHITE,   CC_FONT_UNDERLINED,         0 },  // 0x51 || 0x71
    { CC_COLOR_WHITE,   CC_FONT_REGULAR,            4 },  // 0x52 || 0x72
    { CC_COLOR_WHITE,   CC_FONT_UNDERLINED,         4 },  // 0x53 || 0x73
    { CC_COLOR_WHITE,   CC_FONT_REGULAR,            8 },  // 0x54 || 0x74
    { CC_COLOR_WHITE,   CC_FONT_UNDERLINED,         8 },  // 0x55 || 0x75
    { CC_COLOR_WHITE,   CC_FONT_REGULAR,           12 },  // 0x56 || 0x76
    { CC_COLOR_WHITE,   CC_FONT_UNDERLINED,        12 },  // 0x57 || 0x77
    { CC_COLOR_WHITE,   CC_FONT_REGULAR,           16 },  // 0x58 || 0x78
    { CC_COLOR_WHITE,   CC_FONT_UNDERLINED,        16 },  // 0x59 || 0x79
    { CC_COLOR_WHITE,   CC_FONT_REGULAR,           20 },  // 0x5a || 0x7a
    { CC_COLOR_WHITE,   CC_FONT_UNDERLINED,        20 },  // 0x5b || 0x7b
    { CC_COLOR_WHITE,   CC_FONT_REGULAR,           24 },  // 0x5c || 0x7c
    { CC_COLOR_WHITE,   CC_FONT_UNDERLINED,        24 },  // 0x5d || 0x7d
    { CC_COLOR_WHITE,   CC_FONT_REGULAR,           28 },  // 0x5e || 0x7e
    { CC_COLOR_WHITE,   CC_FONT_UNDERLINED,        28 }   // 0x5f || 0x7f
    /* total 32 entries */
};

static void write_char(CLOSEDCP_PARAM_S *ctx, struct eia608_screen *screen, char ch)
{
    mt_u8 col = ctx->cursor_column;
    mt_u8 *row = screen->characters[ctx->cursor_row];
    mt_u8 *font = screen->fonts[ctx->cursor_row];
    mt_u8 *charset = screen->charsets[ctx->cursor_row];
    mt_u16 unicode_characters[EIA608_SCREEN_COLUMNS+1] = {0};
    MT_UNF_CC_DISPLAY_PARAM_S  cc_display_param;
    mt_u16 override;
    int j, j_u;

    if( ctx->mode == EIA608_MODE_TEXT )
        return;

    //if (col < EIA608_SCREEN_COLUMNS)
    {
        row[col] = ch;
		row[col + 1] = 0;
        font[col] = ctx->cursor_font;
        charset[col] = ctx->cursor_charset;
        ctx->cursor_charset = CCSET_BASIC_AMERICAN;
        if (ch)
        {
            if(col < EIA608_SCREEN_COLUMNS -1)
                ctx->cursor_column++;

            if (ctx->mode != EIA608_MODE_POPON)
            {
                j = 0;

                /* skip leading space */
                while (row[j] == ' ')
                    j++;

                j_u = j;
                for (; j_u < EIA608_SCREEN_COLUMNS; j_u++)
                {
                    if (row[j_u] == 0)
                        break;

                    override = charset_overrides[(int)charset[j_u]][(int)row[j_u]];
                    if(override)
                    {
                        unicode_characters[j_u] = override;
                    }
                    else
                    {
                        unicode_characters[j_u] = (mt_u16)row[j_u];
                    }
                }

                if((strlen((const char *)row)-j) != 0)
                {
                    cc_display_param.enOpt = MT_UNF_CC_OPT_DRAWTEXT;
                    cc_display_param.u32DisplayHeight = ctx->u32DisplayHeight;
                    cc_display_param.u32DisplayWidth = ctx->u32DisplayWidth;
#if 0                    
                    cc_display_param.stRect.x = j*(ctx->u32DisplayWidth/EIA608_SCREEN_COLUMNS) + 100;   //100 is offset
                    cc_display_param.stRect.y = ctx->cursor_row*(ctx->u32DisplayHeight/EIA608_SCREEN_ROWS);
                    cc_display_param.stRect.width = ctx->u32DisplayWidth - cc_display_param.stRect.x;
                    cc_display_param.stRect.height = 50;  //DISPLAY_CC_LINE_INTERVAL+2;
#else                    
                    cc_display_param.stRect.x = j*(ctx->u32DisplayWidth/EIA608_SCREEN_COLUMNS) + SHOW_LR_OFFSET;   //100 is offset
                    cc_display_param.stRect.y = SHOW_TB_OFFSET + ctx->cursor_row*((ctx->u32DisplayHeight - 2*SHOW_TB_OFFSET)/EIA608_SCREEN_ROWS);
                    cc_display_param.stRect.width = ctx->u32DisplayWidth - cc_display_param.stRect.x;
                    cc_display_param.stRect.height = (((ctx->u32DisplayHeight - 2*SHOW_TB_OFFSET)/EIA608_SCREEN_ROWS) > SHOW_LINE_HEIGHT)
                                                    ?SHOW_LINE_HEIGHT:((ctx->u32DisplayHeight - 2*SHOW_TB_OFFSET)/EIA608_SCREEN_ROWS);  //DISPLAY_CC_LINE_INTERVAL+2;
#endif
                    cc_display_param.unDispParam.stText.pu16Text = (mt_u16*)&unicode_characters[j];
                    cc_display_param.unDispParam.stText.u8TextLen = strlen((const char *)row)-j;
                    getCcColor(ctx->color, &cc_display_param.unDispParam.stText.stFgColor);
                    getCcColor(ctx->bgColor, &cc_display_param.unDispParam.stText.stBgColor);
                    getCcColor(ctx->bgColor, &cc_display_param.unDispParam.stText.stEdgeColor);
                    cc_display_param.unDispParam.stText.u8Justify = MT_UNF_CC_JUSTIFY_LEFT;
                    cc_display_param.unDispParam.stText.u8WordWrap = MT_UNF_CC_WW_DISABLE;
                    cc_display_param.unDispParam.stText.enFontStyle = MT_UNF_CC_FONTSTYLE_NORMAL;
                    cc_display_param.unDispParam.stText.enFontSize = MT_UNF_CC_FONTSIZE_DEFAULT;
                    cc_display_param.unDispParam.stText.enEdgetype = MT_UNF_CC_EDGETYPE_DEFAULT;
					ctx->paint_num ++;
                    ctx->end_time = SYS_GetMS();
					if (ctx->paint_num >= ccContext->show_quantity)
					{
						ctx->paint_num = 0;
						ccContext->ccParam.pfnCCDisplay(MT_NULL, &cc_display_param);
					}

					memcpy(&ctx->cc_old_display_param, &cc_display_param, sizeof(MT_UNF_CC_DISPLAY_PARAM_S));					
					memcpy(ctx->old_unicode, cc_display_param.unDispParam.stText.pu16Text, sizeof(mt_u16) * (EIA608_SCREEN_COLUMNS+1));
					ctx->cc_old_display_param.unDispParam.stText.pu16Text = ctx->old_unicode;
					g_cc_paint = TRUE;
                }
            }
        }
        return;
    }

}

void timeout_write_char(MT_UNF_CC_DISPLAY_PARAM_S  cc_display_param)
{
	ccContext->ccParam.pfnCCDisplay(MT_NULL, &cc_display_param);
}


static struct eia608_screen *get_writing_screen(CLOSEDCP_PARAM_S *ctx)
{
    switch (ctx->mode)
    {
        case EIA608_MODE_POPON:
            // use Inactive screen
            return ctx->screen + !ctx->active_screen;
        case EIA608_MODE_PAINTON:
        case EIA608_MODE_ROLLUP_2:
        case EIA608_MODE_ROLLUP_3:
        case EIA608_MODE_ROLLUP_4:
        case EIA608_MODE_TEXT:
            // use active screen
            return ctx->screen + ctx->active_screen;
    }

    return MT_NULL;
}

static void roll_up(CLOSEDCP_PARAM_S *ctx)
{
    struct eia608_screen *screen;
    int i, keep_lines;

    if (ctx->mode == EIA608_MODE_TEXT)
        return;

    //CcClearScreen(ctx);
    screen = get_writing_screen(ctx);

    /* +1 signify cursor_row starts from 0
     * Can't keep lines less then row cursor pos
     */
    keep_lines = FFMIN(ctx->cursor_row + 1, ctx->rollup);

    for (i = 0; i < EIA608_SCREEN_ROWS; i++) {
        if (i > ctx->cursor_row - keep_lines && i <= ctx->cursor_row)
            continue;
        UNSET_FLAG(screen->row_used, i);
    }

    for (i = 0; i < keep_lines && screen->row_used; i++) {
        const int i_row = ctx->cursor_row - keep_lines + i + 1 + EIA608_SCREEN_ROWS;

        if((i_row %EIA608_SCREEN_ROWS) < 0 || (((i_row + 1)%EIA608_SCREEN_ROWS) < 0))
        {
            CC_PRINTF(0,"row index invalid!!! \n");
            continue;
        }        
        memcpy(screen->characters[(i_row %EIA608_SCREEN_ROWS)], screen->characters[((i_row + 1)%EIA608_SCREEN_ROWS)], EIA608_SCREEN_COLUMNS);
        memcpy(screen->colors[(i_row %EIA608_SCREEN_ROWS)], screen->colors[((i_row + 1)%EIA608_SCREEN_ROWS)], EIA608_SCREEN_COLUMNS);
        memcpy(screen->fonts[(i_row %EIA608_SCREEN_ROWS)], screen->fonts[((i_row + 1)%EIA608_SCREEN_ROWS)], EIA608_SCREEN_COLUMNS);
        memcpy(screen->charsets[(i_row %EIA608_SCREEN_ROWS)], screen->charsets[((i_row + 1)%EIA608_SCREEN_ROWS)], EIA608_SCREEN_COLUMNS);
        if (CHECK_FLAG(screen->row_used, ((i_row + 1)%EIA608_SCREEN_ROWS)))
            SET_FLAG(screen->row_used, (i_row %EIA608_SCREEN_ROWS));
    }

    UNSET_FLAG(screen->row_used, ctx->cursor_row);

    ctx->screen_touched = 1;
}

static int capture_screen(CLOSEDCP_PARAM_S *ctx)
{
    int i, j, j_u;
    MT_UNF_CC_DISPLAY_PARAM_S  cc_display_param;
    mt_u16 unicode_characters[EIA608_SCREEN_COLUMNS+1];

    struct eia608_screen *screen = ctx->screen + ctx->active_screen;
    mt_u16 override;

    if(screen->row_used == 0)
    {
        CcClearScreen(ctx);
    }

    for (i = 0; screen->row_used && i < EIA608_SCREEN_ROWS; i++)
    {
        if (CHECK_FLAG(screen->row_used, i))
        {
            const mt_u8 *row = screen->characters[i];
            const mt_u8 *charset = screen->charsets[i];

            j = 0;

            /* skip leading space */
            while (row[j] == ' ')
                j++;

            j_u = j;
            for (; j_u < EIA608_SCREEN_COLUMNS; j_u++)
            {
                if (row[j_u] == 0)
                    break;

                override = charset_overrides[(int)charset[j_u]][(int)row[j_u]];
                if(override)
                {
                    unicode_characters[j_u] = override;
                }
                else
                {
                    unicode_characters[j_u] = (mt_u16)row[j_u];
                }
            }

            if((strlen((const char *)row)-j) != 0)
            {
                cc_display_param.enOpt = MT_UNF_CC_OPT_DRAWTEXT;
                cc_display_param.u32DisplayHeight = ctx->u32DisplayHeight;
                cc_display_param.u32DisplayWidth = ctx->u32DisplayWidth;
#if 0                
                cc_display_param.stRect.x = j*(ctx->u32DisplayWidth/EIA608_SCREEN_COLUMNS) + 100;   //100 is offset
                cc_display_param.stRect.y = i*(ctx->u32DisplayHeight/EIA608_SCREEN_ROWS);
                cc_display_param.stRect.width = ctx->u32DisplayWidth - cc_display_param.stRect.x;
                cc_display_param.stRect.height = 50;//DISPLAY_CC_LINE_INTERVAL+2;
#else
                cc_display_param.stRect.x = j*(ctx->u32DisplayWidth/EIA608_SCREEN_COLUMNS) + SHOW_LR_OFFSET;   //100 is offset
                cc_display_param.stRect.y = SHOW_TB_OFFSET + i*((ctx->u32DisplayHeight - 2*SHOW_TB_OFFSET)/EIA608_SCREEN_ROWS);
                cc_display_param.stRect.width = ctx->u32DisplayWidth - cc_display_param.stRect.x;
                cc_display_param.stRect.height = (((ctx->u32DisplayHeight - 2*SHOW_TB_OFFSET)/EIA608_SCREEN_ROWS) > SHOW_LINE_HEIGHT)
                                                    ?SHOW_LINE_HEIGHT:((ctx->u32DisplayHeight - 2*SHOW_TB_OFFSET)/EIA608_SCREEN_ROWS);
                
#endif
                cc_display_param.unDispParam.stText.pu16Text = (mt_u16*)&unicode_characters[j];
                cc_display_param.unDispParam.stText.u8TextLen = strlen((const char *)row)-j;
                getCcColor(ctx->color, &cc_display_param.unDispParam.stText.stFgColor);
                getCcColor(ctx->bgColor, &cc_display_param.unDispParam.stText.stBgColor);
                getCcColor(ctx->bgColor, &cc_display_param.unDispParam.stText.stEdgeColor);
                cc_display_param.unDispParam.stText.u8Justify = MT_UNF_CC_JUSTIFY_LEFT;
                cc_display_param.unDispParam.stText.u8WordWrap = MT_UNF_CC_WW_DISABLE;
                cc_display_param.unDispParam.stText.enFontStyle = MT_UNF_CC_FONTSTYLE_NORMAL;
                cc_display_param.unDispParam.stText.enFontSize = MT_UNF_CC_FONTSIZE_DEFAULT;
                cc_display_param.unDispParam.stText.enEdgetype = MT_UNF_CC_EDGETYPE_DEFAULT;
				ctx->paint_num = 0;
                ccContext->ccParam.pfnCCDisplay(MT_NULL, &cc_display_param);
            }
        }
    }

    return 0;

}

static void handle_textattr(CLOSEDCP_PARAM_S *ctx, mt_u8 hi, mt_u8 lo)
{
    int i = lo - 0x20;
    struct eia608_screen *screen = get_writing_screen(ctx);

    if (i >= 32)
        return;

    ctx->cursor_color = pac2_attribs[i][0];
    ctx->cursor_font = pac2_attribs[i][1];

    SET_FLAG(screen->row_used, ctx->cursor_row);
    write_char(ctx, screen, ' ');
}

static void handle_pac(CLOSEDCP_PARAM_S *ctx, mt_u8 hi, mt_u8 lo)
{
    static const mt_s8 row_map[] = {
        11, -1, 1, 2, 3, 4, 12, 13, 14, 15, 5, 6, 7, 8, 9, 10
    };
    int index = ( (hi<<1) & 0x0e) | ( (lo>>5) & 0x01 );
    struct eia608_screen *screen = get_writing_screen(ctx);
    int indent, i;

    if(!(index < (sizeof(row_map)/sizeof(row_map[0]))))
    {
        CC_PRINTF(2, "row_map invalid index\n");
        index = 9;//force to 15 
        //return;
    }

    if (row_map[index] <= 0)
    {
        CC_PRINTF(2, "Invalid pac index encountered\n");
        return;
    }

    lo &= 0x1f;

    ctx->cursor_row = row_map[index] - 1;
    ctx->cursor_color =  pac2_attribs[lo][0];
    ctx->cursor_font = pac2_attribs[lo][1];
    ctx->cursor_charset = CCSET_BASIC_AMERICAN;
    ctx->cursor_column = 0;
    indent = pac2_attribs[lo][2];
    for (i = 0; i < indent; i++)
    {
        write_char(ctx, screen, ' ');
    }
}

static void handle_edm(CLOSEDCP_PARAM_S *ctx)
{
    int i = 0;
    struct eia608_screen *screen = ctx->screen + ctx->active_screen;

    memset(screen, 0, sizeof(eia608_screen));
    for(i = 0; i < EIA608_SCREEN_ROWS; i++)
    {
        memset(screen->characters[i], ' ', EIA608_SCREEN_COLUMNS);
        screen->characters[i][EIA608_SCREEN_COLUMNS] = 0;
    }
    // In buffered mode, keep writing to screen until it is wiped.
    // Before wiping the display, capture contents to emit subtitle.
//    if (!ctx->real_time)
//        reap_screen(ctx, pts);

    screen->row_used = 0;

    // In realtime mode, emit an empty caption so the last one doesn't
    // stay on the screen.
//    if (ctx->real_time)
        capture_screen(ctx);
}

static void handle_eoc(CLOSEDCP_PARAM_S *ctx)
{
    // In buffered mode, we wait til the *next* EOC and
    // reap what was already on the screen since the last EOC.
//    if (!ctx->real_time)
        handle_edm(ctx);

    ctx->active_screen = !ctx->active_screen;
    ctx->cursor_column = 0;

    // In realtime mode, we display the buffered contents (after
    // flipping the buffer to active above) as soon as EOC arrives.
//    if (ctx->real_time)
        capture_screen(ctx);
}

static void handle_delete_end_of_row(CLOSEDCP_PARAM_S *ctx, mt_u8 hi, mt_u8 lo)
{
    struct eia608_screen *screen = get_writing_screen(ctx);
    write_char(ctx, screen, 0);
}

static void handle_char(CLOSEDCP_PARAM_S *ctx, char hi, char lo)
{
    struct eia608_screen *screen = get_writing_screen(ctx);

    SET_FLAG(screen->row_used, ctx->cursor_row);

    switch (hi) {
      case 0x11:
        ctx->cursor_charset = CCSET_SPECIAL_AMERICAN;
        break;
      case 0x12:
        if (ctx->cursor_column > 0)
            ctx->cursor_column -= 1;
        ctx->cursor_charset = CCSET_EXTENDED_SPANISH_FRENCH_MISC;
        break;
      case 0x13:
        if (ctx->cursor_column > 0)
            ctx->cursor_column -= 1;
        ctx->cursor_charset = CCSET_EXTENDED_PORTUGUESE_GERMAN_DANISH;
        break;
      default:
        ctx->cursor_charset = CCSET_BASIC_AMERICAN;
        write_char(ctx, screen, hi);
        break;
    }
    if(lo < 0x20)
        return;
    if (lo) 
    {
        write_char(ctx, screen, lo);
    }
    //write_char(ctx, screen, 0);

    if (lo)
    {
       CC_PRINTF(2, "(%c,%c)\n", hi, lo);
    }
    else
    {
       CC_PRINTF(2, "(%c)\n", hi);
    }
}

static void handle_erase(CLOSEDCP_PARAM_S *ctx)
{
    struct eia608_screen *screen = get_writing_screen(ctx);
    mt_u8 *row = screen->characters[ctx->cursor_row];
    const mt_u8 i_column = ctx->cursor_column - 1;

    if( ctx->mode == EIA608_MODE_TEXT )
        return;

    if( i_column < 0 )
        return;

    row[i_column] = ' ';
    ctx->cursor_column--;

}

static void process_cc608(CLOSEDCP_PARAM_S * ccParam, mt_u8 hi, mt_u8 lo)
{
    if(hi==0 && lo==0)
        return;

    if (hi == ccParam->prev_cmd[0] && lo == ccParam->prev_cmd[1])
    {
        /* ignore redundant command */
        return;
    }

    /* set prev command */
    ccParam->prev_cmd[0] = hi;
    ccParam->prev_cmd[1] = lo;

    if ( (hi == 0x10 && (lo >= 0x40 && lo <= 0x5f)) ||
       ( (hi >= 0x11 && hi <= 0x17) && (lo >= 0x40 && lo <= 0x7f) ) )
    {
        handle_pac(ccParam, hi, lo);
    }
    else if ( ( hi == 0x11 && lo >= 0x20 && lo <= 0x2f ) ||
                ( hi == 0x17 && lo >= 0x2e && lo <= 0x2f) )
    {
        handle_textattr(ccParam, hi, lo);
    }
    else if (hi == 0x14 || hi == 0x15 || hi == 0x1c)
    {
        if(new_channel != 1)
        {
            return;
        }
        switch (lo)
        {
        case 0x20:
            /* resume caption loading */
            ccParam->mode = EIA608_MODE_POPON;
            break;
        case 0x21:
            /* Backspace */
            handle_erase(ccParam);
            break;
        case 0x24:
            handle_delete_end_of_row(ccParam, hi, lo);
            break;
        case 0x25:
            ccParam->mode = EIA608_MODE_ROLLUP_2;
            ccParam->rollup = lo - 0x23;
            break;
        case 0x26:
            ccParam->mode = EIA608_MODE_ROLLUP_3;
            ccParam->rollup = lo - 0x23;
            break;
        case 0x27:
            ccParam->mode = EIA608_MODE_ROLLUP_4;
            ccParam->rollup = lo - 0x23;
            break;
        case 0x29:
            /* resume direct captioning */
            ccParam->mode = EIA608_MODE_PAINTON;
            break;
        case 0x2b:
            /* resume text display */
            ccParam->mode = EIA608_MODE_TEXT;
            break;
        case 0x2c:
            /* erase display memory */
            handle_edm(ccParam);
            break;
        case 0x2d:
            /* carriage return */
            CC_PRINTF(2, "carriage return\n");
            if (!ccParam->real_time)
//                reap_screen(ctx, pts);
            roll_up(ccParam);
            ccParam->cursor_column = 0;
			CcClearScreen(ccParam);
			capture_screen(ccParam);
			memset(ccParam->old_unicode, 0 ,sizeof(mt_u16) * (EIA608_SCREEN_COLUMNS+1));
            break;
        case 0x2e:
            /* erase buffered (non displayed) memory */
            // Only in realtime mode. In buffered mode, we re-use the inactive screen
            // for our own buffering.
//            if (ccParam->real_time)
            {
                struct eia608_screen *screen = ccParam->screen + !ccParam->active_screen;
                screen->row_used = 0;
            }
            break;
        case 0x2f:
            /* end of caption */
            CC_PRINTF(2, "handle_eoc\n");
            handle_eoc(ccParam);
            break;
        default:
            CC_PRINTF(2, "Unknown command 0x%hhx 0x%hhx\n", hi, lo);
            break;
        }
    }
    else if (hi >= 0x11 && hi <= 0x13)
    {
        /* Special characters */
        handle_char(ccParam, hi, lo);
    }
    else if (hi >= 0x20)
    {
        if(new_channel != 1)
        {
            return;
        }
        /* Standard characters (always in pairs) */
        handle_char(ccParam, hi, lo);
        ccParam->prev_cmd[0] = ccParam->prev_cmd[1] = 0;
    }
    else if (hi == 0x17 && lo >= 0x21 && lo <= 0x23)
    {
        int i;
        /* Tab offsets (spacing) */
        for (i = 0; i < lo - 0x20; i++)
        {
            handle_char(ccParam, ' ', 0);
        }
    }
    else
    {
        /* Ignoring all other non data code */
        CC_PRINTF(2, "Unknown command 0x%hhx 0x%hhx\n", hi, lo);
    }

}

MT_BOOL CcProcess(CLOSEDCP_PARAM_S * ccParam, mt_u8* cc_data_buff, mt_u32 cc_data_len)
{
    MT_BOOL bOK = MT_FALSE;
    static mt_u8 top_field_first = 0;
    mt_u8 field_number;
	
    if(cc_data_buff == NULL)
    {
        CC_PRINTF(2, "CC:No CC data...");
        return bOK;
    }

    if(cc_data_len > 0)
    {
        int x=0;
        //static u8 ch=0;

        CC_PRINTF(0, "\r\n cc_data_len=%d\n",cc_data_len);
        for(x=0; x<cc_data_len/3; x++)
        {
            CC_PRINTF(0, "[%x,%x,%x]\n",cc_data_buff[3*x + 0],cc_data_buff[3*x + 1] ,cc_data_buff[3*x + 2]);
            if((cc_data_buff[3*x + 0] & 0xff) == 0xfc )
            {
                char hi = cc_data_buff[3*x + 1] & 0x7f;
                char lo = cc_data_buff[3*x + 2] & 0x7f;


                if(hi >= 0x10 && hi <= 0x1f)
                {
                    if(hi >= 0x10 && hi <=0x17)
                    {
                        new_channel = 1;
                    }
                    else if(hi >= 0x18 && hi <= 0x1e)
                    {
                        new_channel = 2;
                    }
                }
                process_cc608(ccParam, hi, lo);

      			 /*
                if(ccParam->screen_touched)
                {
                    ccParam->screen_touched = 0;
                    capture_screen(ccParam);
                }
				*/
            }
	     else if((cc_data_buff[3*x + 0] & 0xf0) == 0x80 || 
                          (cc_data_buff[3*x + 0] & 0xf0) == 0x40 ||
                          (cc_data_buff[3*x + 0] & 0xf0) == 0x00)    //SCTE 20  
             {

                if((cc_data_buff[3*x + 0] & 0xc0) == 0x40)
                {
                    top_field_first = 1;
                }
                else if((cc_data_buff[3*x + 0] & 0xc0) == 0x80)
                {
                    top_field_first = 0;
                }
                // Treat field_number 3 as 1
                field_number = ((cc_data_buff[3*x + 0] & 0x3) - 1) & 0x01;	
                // top_field_first also affects to which field the caption belongs.
                if(!top_field_first)
                {
                    field_number ^= 0x01;
                }

                if(field_number == 0)    //top field
                {
                    cc_data_buff[3*x + 1] = reverse8(cc_data_buff[3*x + 1]);
                    cc_data_buff[3*x + 2] = reverse8(cc_data_buff[3*x + 2]);
                    char hi = cc_data_buff[3*x + 1] & 0x7f;
                    char lo = cc_data_buff[3*x + 2] & 0x7f;

                    process_cc608(ccParam, hi, lo);
/*
                    if(ccParam->screen_touched)
                    {
                        ccParam->screen_touched = 0;
                        capture_screen(ccParam);
                    } */
                }
            }
        }
    }
    return bOK;

}


static int cc_init_count = 0;
static void cc_task_vsb(void *p_data)
{
    mt_u32 cc_data_len = 0;
    mt_u8 *cc_data_buff = NULL;
    int i=0;
    mt_u32      k = 0;

    CLOSEDCP_DrawCharInfo *curDrawCharInfo;
    mt_u16      drawXpos;
    //mt_u32      fontSize;
    mt_u32      block_x;
    //mt_u32      block_y;
    mt_u16      xPos;
    mt_u16      yPos;
    MT_UNF_CC_DISPLAY_PARAM_S  cc_display_param;



	mt_set_pthread_name(__FUNCTION__);
    if(cc_data_buff == NULL)    
      cc_data_buff = (mt_u8*) CcMalloc(CC_FIFO_TOTAL_LENGTH, 32);

    ccContext->curDecodPesInfo = ccContext->pesInfo;
    ccContext->curDrawPesInfo = ccContext->pesInfo;

    while(1)
    {
        if(ccProtocol == ARIB_STD24)
        {
            if(ccContext->ccTaskRunning == MT_TRUE)
            {
                if(ccDisplayOnOff == MT_TRUE)
                {

                    /***********************/
                    /*         Draw        */
                    /***********************/
                    while (ccContext->restDecodedPesCount)
                    {
                        //stcValue = CLOSEDCP_GetStcTime();

                        //if (ccDat->curDrawPesInfo->headerInfo.PTS <= stcValue || ccDat->curDrawPesInfo->headerInfo.fNotPTS)
                        {

                            for (i = 0; i < ccContext->curDrawPesInfo->drawCharNum; i ++)
                            {
                                curDrawCharInfo = ccContext->curDrawPesInfo->drawCharInfo + k;

                                if (curDrawCharInfo->isCS)
                                {
                                    CcClearScreen(&ccContext->ccParam);
                                    curDrawCharInfo->isCS = 0;
                                }

                                if (curDrawCharInfo->dataInfo.decodedData[0])
                                {
                                    //fontSize = curDrawCharInfo->fontSize;
                                    block_x  = curDrawCharInfo->block_x;
                                    //block_y  = curDrawCharInfo->block_y;
                                    xPos     = curDrawCharInfo->xPos;
                                    yPos     = curDrawCharInfo->yPos;

                                    if (xPos) {
                                        drawXpos = xPos - block_x;
                                    }
                                    else {
                                        drawXpos = xPos;
                                    }


                                    /* NOTE: Unicode format (ISO 10646) is expected
                                     from CC subsystem. */
                                    //CC_PRINTF(1, "draw_charcount=%d, xPos=%d, yPos=%d, fontSize=%d, color1=%d, color2=%d\n", curDrawCharInfo->dataInfo.charcount, xPos, yPos, fontSize, curDrawCharInfo->color1, curDrawCharInfo->color2);

                                    cc_display_param.enOpt = MT_UNF_CC_OPT_FILLRECT;
                                    cc_display_param.u32DisplayHeight = ccContext->ccParam.u32DisplayHeight;
                                    cc_display_param.u32DisplayWidth = ccContext->ccParam.u32DisplayWidth;
                                    cc_display_param.stRect.x = drawXpos;
                                    cc_display_param.stRect.y = yPos;
                                    cc_display_param.stRect.width = ccContext->ccParam.u32DisplayWidth-drawXpos;
                                    cc_display_param.stRect.height = 50;//DISPLAY_CC_LINE_INTERVAL+2;
                                    getAribCcColor(CC_COLOR_TRANSPARENT, &cc_display_param.unDispParam.stFillRect.stColor);
                                    ccContext->ccParam.pfnCCDisplay(MT_NULL, &cc_display_param);

                                    cc_display_param.enOpt = MT_UNF_CC_OPT_DRAWTEXT;
                                    cc_display_param.u32DisplayHeight = ccContext->ccParam.u32DisplayHeight;
                                    cc_display_param.u32DisplayWidth = ccContext->ccParam.u32DisplayWidth;
                                    cc_display_param.stRect.x = xPos;
                                    cc_display_param.stRect.y = yPos;
                                    cc_display_param.stRect.width = ccContext->ccParam.u32DisplayWidth - xPos;
                                    cc_display_param.stRect.height = 50;//DISPLAY_CC_LINE_INTERVAL+2;
                                    cc_display_param.unDispParam.stText.pu16Text = (mt_u16 *)curDrawCharInfo->dataInfo.decodedData;
                                    cc_display_param.unDispParam.stText.u8TextLen = curDrawCharInfo->dataInfo.charcount;
                                    getAribCcColor(curDrawCharInfo->color1, &cc_display_param.unDispParam.stText.stFgColor);
                                    getAribCcColor(curDrawCharInfo->color2, &cc_display_param.unDispParam.stText.stBgColor);
                                    getAribCcColor(curDrawCharInfo->color2, &cc_display_param.unDispParam.stText.stEdgeColor);
                                    cc_display_param.unDispParam.stText.u8Justify = MT_UNF_CC_JUSTIFY_LEFT;
                                    cc_display_param.unDispParam.stText.u8WordWrap = MT_UNF_CC_WW_DISABLE;
                                    cc_display_param.unDispParam.stText.enFontStyle = MT_UNF_CC_FONTSTYLE_NORMAL;
                                    cc_display_param.unDispParam.stText.enFontSize = MT_UNF_CC_FONTSIZE_DEFAULT;
                                    cc_display_param.unDispParam.stText.enEdgetype = MT_UNF_CC_EDGETYPE_DEFAULT;
                                   ccContext->ccParam.pfnCCDisplay(MT_NULL, &cc_display_param);

                                }
                                k ++;
                            }
                            k = 0;

                            memset(ccContext->curDrawPesInfo, 0, sizeof(CLOSEDCP_PesInfo));

                            if (ccContext->drawPesCount % CLOSEDCP_MAX_PES_NUM == 0) {
                                ccContext->drawPesCount = 0;
                                ccContext->curDrawPesInfo = ccContext->pesInfo;
                            }
                            else {
                                ccContext->curDrawPesInfo = ccContext->curDrawPesInfo + 1;
                            }

                            ccContext->drawPesCount = ccContext->drawPesCount + 1;
                            ccContext->restDecodedPesCount = ccContext->restDecodedPesCount - 1;

                        }
                        //else
                        //{
                            //break;
                        //}
                    }
                }
            }
        }
        else if(ccProtocol == EIA_608_708 || ccProtocol == EIA_DTVCC_708)
        {
            if(ccContext->ccTaskRunning == MT_TRUE)
            {
                if(ccDisplayOnOff == MT_TRUE)
                {
                    //memset(cc_data_buff, 0, CC_DATA_BUFFER);

					//if (ccContext->ccParam.start_time != ccContext->ccParam.end_time)
                    // ccContext->ccParam.start_time = ccContext->ccParam.end_time;
					//else
					//	ccContext->ccParam.start_time = SYS_GetMS();
                    if(ccProtocol == EIA_608_708)
                    {
                        ccContext->ccParam.start_time = SYS_GetMS();
                        cc_fifo_get(cc_data_buff,  &cc_data_len);
                        if(cc_data_len > 0)
                        {
                            CcProcess(&ccContext->ccParam, cc_data_buff, cc_data_len);                        
                        }
    					else
    					{
    						if (g_cc_paint == TRUE)
    						{
    							if(SYS_GetMS() - ccContext->ccParam.end_time > 16*1000)//timeout : 608 standard request minimum value is 16 seconds
    							{
    								g_cc_paint = FALSE;
    								CcClearScreen(&ccContext->ccParam);
    							}
    						}            
    					}
                        
    					if ((ccContext->ccParam.paint_num < ccContext->show_quantity)
    						&& (ccContext->ccParam.paint_num > 0)
    						&& (ccContext->ccParam.end_time != 0)
    						&& (ccContext->ccParam.end_time - ccContext->ccParam.start_time > 200)
    						&& (g_cc_paint == TRUE))
    					{
    						timeout_write_char(ccContext->ccParam.cc_old_display_param);
    						ccContext->ccParam.paint_num = 0;
    					}
                        
                        if(cc_data_len)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        cc_fifo_get(cc_data_buff,  &cc_data_len);
                        if(cc_data_len > 0)
                        {
                            if(Com_CC708_DtvCC_ParsePicUsrData(cc_data_buff, cc_data_len,MT_FALSE) == MT_SUCCESS)
                            {
                                if(MT_FAILURE == CC708_ProcessData())
                                {
                                    CC_PRINTF(0, "CC708_ProcessData error!\n");
                                }
                            }
                            
                        }
                        if(MT_FAILURE == CC708_ProcessData())
                        {
                            CC_PRINTF(0, "2 CC708_ProcessData error!\n");
                        }                        
                        if(cc_data_len)
                        {
                            continue;
                        }
                        
                    }
                }
            }
        }
        mtos_task_sleep(10);
	}
	return;
}


ulong cc_init_vsb(MT_UNF_CC_PARAM_S *pstCCParam, mt_u32 stack_size, CC_PROTOCOL cc_protocol, void *cb)
{
    static sCcContext default_ccContext;
    rect_size_t *rect  __attribute__((unused)) = (rect_size_t *)cb;
    rect_size_t default_rect = {0,0,1280,720};

    pthread_t   g_TsThd2;
    mt_u32 Ret=0;

    if(cb == NULL)
    {
        rect = &default_rect;
    }

    new_channel = 1;

    ccContext = &default_ccContext;
    memset(ccContext, 0, sizeof(sCcContext));

    if(ccContext->ccTaskRunning == MT_FALSE)
    {
        //int i=0;
        ccContext->ccTaskRunning = MT_TRUE;
        ccContext->ccParam.pfnCCGetPts = pstCCParam->pfnCCGetPts;
        ccContext->ccParam.pfnCCDisplay = pstCCParam->pfnCCDisplay;
        ccContext->ccParam.pfnCCGetTextSize = pstCCParam->pfnCCGetTextSize;
        ccContext->ccParam.pfnBlit = pstCCParam->pfnBlit;
        ccContext->ccParam.pfnVBIOutput = pstCCParam->pfnVBIOutput;
        ccContext->ccParam.pfnXDSOutput = pstCCParam->pfnXDSOutput;

        if(pstCCParam->stCCAttr.enCCDataType == MT_UNF_CC_DATA_TYPE_608)
        {
            ccContext->ccParam.color = 	pstCCParam->stCCAttr.unCCConfig.stCC608ConfigParam.u32CC608TextColor;
            ccContext->ccParam.bgColor	 = 	pstCCParam->stCCAttr.unCCConfig.stCC608ConfigParam.u32CC608BgColor;
            switch(pstCCParam->stCCAttr.unCCConfig.stCC608ConfigParam.enCC608DispFormat)
            {
                case MT_UNF_CC_DF_720X480:
                    ccContext->ccParam.u32DisplayWidth = 720;
                    ccContext->ccParam.u32DisplayHeight = 480;
                    break;

                case MT_UNF_CC_DF_720X576:
                    ccContext->ccParam.u32DisplayWidth = 720;
                    ccContext->ccParam.u32DisplayHeight = 576;
                    break;

                case MT_UNF_CC_DF_960X540:
                    ccContext->ccParam.u32DisplayWidth = 960;
                    ccContext->ccParam.u32DisplayHeight = 540;
                    break;

                case MT_UNF_CC_DF_1280X720:
                    ccContext->ccParam.u32DisplayWidth = 1280;
                    ccContext->ccParam.u32DisplayHeight = 720;
                    break;

                case MT_UNF_CC_DF_1920X1080:
                    ccContext->ccParam.u32DisplayWidth = 1920;
                    ccContext->ccParam.u32DisplayHeight = 1080;
                    break;

                default:
                    ccContext->ccParam.u32DisplayWidth = 1280;
                    ccContext->ccParam.u32DisplayHeight = 720;
                    break;
            }
        }
        else if(pstCCParam->stCCAttr.enCCDataType == MT_UNF_CC_DATA_TYPE_708)
        {
            ccContext->ccParam.color = pstCCParam->stCCAttr.unCCConfig.stCC708ConfigParam.u32CC708TextColor;
            ccContext->ccParam.bgColor	 = 	pstCCParam->stCCAttr.unCCConfig.stCC708ConfigParam.u32CC708BgColor;;

            switch(pstCCParam->stCCAttr.unCCConfig.stCC708ConfigParam.enCC708DispFormat)
            {
                case MT_UNF_CC_DF_720X480:
                    ccContext->ccParam.u32DisplayWidth = 720;
                    ccContext->ccParam.u32DisplayHeight = 480;
                    break;

                case MT_UNF_CC_DF_720X576:
                    ccContext->ccParam.u32DisplayWidth = 720;
                    ccContext->ccParam.u32DisplayHeight = 576;
                    break;

                case MT_UNF_CC_DF_960X540:
                    ccContext->ccParam.u32DisplayWidth = 960;
                    ccContext->ccParam.u32DisplayHeight = 540;
                    break;

                case MT_UNF_CC_DF_1280X720:
                    ccContext->ccParam.u32DisplayWidth = 1280;
                    ccContext->ccParam.u32DisplayHeight = 720;
                    break;

                case MT_UNF_CC_DF_1920X1080:
                    ccContext->ccParam.u32DisplayWidth = 1920;
                    ccContext->ccParam.u32DisplayHeight = 1080;
                    break;

                default:
                    ccContext->ccParam.u32DisplayWidth = 1280;
                    ccContext->ccParam.u32DisplayHeight = 720;
                    break;
            }
        }
        else
        {
            ccContext->ccParam.u32DisplayWidth = 1280;
            ccContext->ccParam.u32DisplayHeight = 720;
        }

        ccProtocolTransfor = ccProtocol = cc_protocol;

        if(cc_protocol == EIA_608_708 || cc_protocol== EIA_DTVCC_708 || cc_protocol == PROTOCOL_UNKNOW)
        {
          cc_fifo_init();
        }

/*
        if(ccContext->p_stack == NULL)
        {
          ccContext->p_stack = (mt_u32 *)CcMalloc(stack_size, 4);
          memset(ccContext->p_stack, 0, stack_size);
        }
*/

        ccDisplayOnOff = MT_FALSE;


       if(cc_protocol == PROTOCOL_UNKNOW)
       {
          ccProtocolTransfor = ccProtocol = ARIB_STD24;
       }


        if(cc_init_count == 0)
        {
            Ret = pthread_create(&g_TsThd2, MT_NULL, (mt_void *)cc_task_vsb, MT_NULL);
            if (0 != Ret)
            {
                perror("[DmxStartRecord] pthread_create record error");
                return Ret;
            }
        }
        else
        {
            if(ccContext)
            {
                ccContext->curDecodPesInfo = ccContext->pesInfo;
                ccContext->curDrawPesInfo = ccContext->pesInfo; 
            }
        }
        cc_init_count ++;
    }

    return (ulong)ccContext;
}

void cc_set_show_mode(int show_num)
{
	if (ccContext)
		ccContext->show_quantity = show_num;
}

mt_s32 cc_deinit_vsb(void)
{
    if(ccDisplayOnOff == MT_TRUE)
    {
        ccDisplayOnOff = MT_FALSE;
    }
    cc_fifo_deinit();
    return MT_SUCCESS;
}
