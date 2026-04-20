/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "string.h"
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>

#include "mt_go.h"
#include "mtgo_surface.h"
#include "mt_go_config.h"
#ifdef MTGO_TEXT_SUPPORT

//#include "include/ft2build.h"
#include "ft2build.h"
#include "uni2gb.h"
//#include "mt_lld_disp.h"
//#include "mt_lld_layer.h"

#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BITMAP_H
//#include "freetype.h"
//#include "ftglyph.h"
//#include "ftbitmap.h"
#endif
#include "mt_common.h"
#include "mtgo_text.h"
#include "mt_module_debug.h"
#include "mt_tde_type.h"
#include "mt_go_text.h"
#include "mt_tde_api.h"
//#define MT_INFO_MTGO
#define DEFAULT_FONT_SIZE (22)
#define GRAY_TH (0)
#define BOLD_STRENGTH (2)
#define USE_KERNING (0)
#ifdef MTGO_TEXT_SUPPORT

typedef struct ft2_ctx_t
{
    FT_Face face;
    FT_GlyphSlot slot;
} ft2_ctx_s;

typedef struct
{
    mt_u32 scroll_times;
    mt_u32 scroll_speed;
    MT_RECT scroll_win;
    mt_handle bg_surface;
    mt_handle *text_surface_arr; //gray-8
    mt_handle *rgb_surface_arr;  //rgb
    mt_u32 bmp_num;
    mt_handle scroll_hdl;
} scroll_s;

static FT_Library library;

typedef struct
{
    ft2_ctx_s *ft2CtxtBase;
    MTGO_TEXTOUTATTR_S text_attr;
    MT_BOOL bg_transparent;
    MT_BOOL bg_color_valid;
    mt_u32 char_distance;
    mt_u32 line_distance;
    MTGO_TEXT_STYLE_E style;
    scroll_s scroll;
    MT_BOOL scroll_created;
    mt_char *pFontBuf;
} MTGO_TEXT_S;

typedef struct
{
    FT_UInt index;  /* glyph index */
    FT_Vector pos;  /* glyph origin on the baseline */
    FT_Glyph image; /* glyph image */
} TGlyph;

#define MTGO_MAX_TEXTSCROLL_NUM 5
typedef struct
{
    MT_BOOL TextScrollUsed[MTGO_MAX_TEXTSCROLL_NUM];
    mt_handle TextScrollHdl[MTGO_MAX_TEXTSCROLL_NUM];
    mt_handle TextDstSurface[MTGO_MAX_TEXTSCROLL_NUM];
} mtgo_textMemCtrl;

MTGO_TEXT_S text_hdl = { 0 };
static mtgo_textMemCtrl mtTextMem;

//#define Printf  printf
#ifdef MTGO_TEXT_SUPPORT

static mt_void MT_GO_Dump_Surface_Font(MTGO_SURFACE_S *p_surface);
static mt_void MT_GO_Dump_Surface_Font_ARGB8888(mt_handle MasksurfaceHandle, mt_handle *pArgb888SurfaceHandle);
#endif
/*
static  mt_u8 Sample_Get_key2(void)
{
    mt_u8 cc = 0;
    while(1)
    {
       cc  = getchar();
       Printf("Get Key: %c\n", cc);
       if('a' <= cc || cc >= 'z')
       {
        break;
       }
    }   
    return cc;
}
*/

static mt_s32 mt_open(const char *filename, int flag)
{
    return open(filename, flag, 0664);
}

static mt_s32 mt_close(int file)
{
    if (file < 0)
	return -1;

    return close(file);
}
static mt_s32 mt_seek(int file, int off, int fromwhere)
{
    int ret = 0;

    if (file < 0)
	return -1;

    ret = lseek(file, (off_t)off, fromwhere);
    return ret;
}

static mt_s32 mt_read(int file, void *buf, int size)
{
    int ret = 0;

    if (file < 0 || size == 0)
	return -1;

    ret = read(file, buf, (mt_u32)size);
    return ret;
}

static mt_s32 mt_tell(int file)
{
    int ret = 0;

    if (file < 0)
	return -1;

    ret = lseek(file, 0, SEEK_CUR);
    return ret;
}

static mt_s32 mt_filesize(int file)
{
    mt_s32 curpos, length;

    curpos = mt_tell(file);
    mt_seek(file, 0L, SEEK_SET);
    length = mt_seek(file, 0L, SEEK_END);
    mt_seek(file, curpos, SEEK_SET);
    return length;

    return -1;
}
#endif

mt_s32 MT_GO_GetFileSize(mt_char *pFileName, mt_u32 *pSize)
{
#ifdef MTGO_TEXT_SUPPORT

    mt_s32 file;
    mt_u32 text_size;

    if (pFileName == NULL) {
	MT_ERR_MTGO("File Name is null!\n");
	return MT_FAILURE;
    }
    file = mt_open(pFileName, O_RDONLY);
    if (file == 0) {
	MT_ERR_MTGO("fail to read file!\n");
	return MT_FAILURE;
    }
    text_size = (mt_u32)mt_filesize(file);
    *pSize = text_size;
    mt_close(file);

    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif
}

mt_s32 MT_GO_ReadFile(mt_char *pFileName, mt_char *pBuf, mt_u32 ReadOffset, mt_u32 ReadSize)
{
#ifdef MTGO_TEXT_SUPPORT

    mt_s32 file;
    mt_u32 read_size;
    mt_s32 seek_offset;

    if (pFileName == NULL) {
	MT_ERR_MTGO("File Name is null!\n");
	return MT_FAILURE;
    }
    file = mt_open(pFileName, O_RDONLY);
    if (file == 0) {
	MT_ERR_MTGO("fail to read file!\n");
	return MT_FAILURE;
    }
    seek_offset = mt_seek(file, (mt_s32)ReadOffset, SEEK_SET);
    if (seek_offset != (mt_s32)ReadOffset) {
	MT_ERR_MTGO("fail to seek offset\n");
    } else {
	read_size = (mt_u32)mt_read(file, (void *)pBuf, (mt_s32)ReadSize);
	if (read_size != ReadSize) {
	    MT_ERR_MTGO("read size error: %d!\n",read_size);
	}
    }
    mt_close(file);

    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif
}

mt_s32 MT_GO_InitText(mt_void)
{
#ifdef MTGO_TEXT_SUPPORT

    FT_Error error;
    mt_u32 cnt;

    error = FT_Init_FreeType(&library);
    if (error) {
	MT_ERR_MTGO("FT_Init_FreeType failed\n");
	return MT_FAILURE;
    }
    MT_INFO_MTGO("library:0x%lx\n", (ulong)library);
    for (cnt = 0; cnt < MTGO_MAX_TEXTSCROLL_NUM; cnt++) {
	mtTextMem.TextScrollUsed[cnt] = MT_FALSE;
	mtTextMem.TextScrollHdl[cnt] = MT_NULL_PTR;
	mtTextMem.TextDstSurface[cnt] = MT_NULL_PTR;
    }
    return MT_SUCCESS;
#else

	return MT_FAILURE;
#endif	
}

mt_s32 MT_GO_DeinitText(mt_void)
{
#ifdef MTGO_TEXT_SUPPORT

    MT_GO_DestroyAllTextScrolls();
    FT_Done_FreeType(library);
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif	
}

mt_s32 MT_GO_CreateText(mt_char *pFontFile, mt_handle *pTextOut)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_INFO_S info;
	mt_s32 ret;
    info.pFontFile = pFontFile;
    info.u32Size = DEFAULT_FONT_SIZE;
    ret = MT_GO_CreateTextEx(&info, pTextOut);
    return ret;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_DestroyText(mt_handle TextOut)
{
#ifdef MTGO_TEXT_SUPPORT

    ft2_ctx_s *p_text = ((MTGO_TEXT_S *)TextOut)->ft2CtxtBase;
    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)TextOut;
    FT_Done_Face(p_text->face);
    free(p_text);
    free(p_text_hdl->pFontBuf);
    free(p_text_hdl);
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_SetPixelSize(mt_handle TextOut, mt_u32 width, mt_u32 height)
{
#ifdef MTGO_TEXT_SUPPORT

    FT_Error error;
    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)TextOut;
    ft2_ctx_s *p_text = p_text_hdl->ft2CtxtBase;
    error = FT_Set_Pixel_Sizes(p_text->face, width, height);
    if (error) {
	MT_ERR_MTGO("Error : FT_Set_Pixel_Sizes() failed! \n");
	return MT_FAILURE;
    }
    p_text_hdl->text_attr.FontAttr.MaxWidth = (mt_u8)width;
    p_text_hdl->text_attr.FontAttr.Height = (mt_u8)height;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif	
}

mt_s32 MT_GO_GetPixelSize(mt_handle TextOut, mt_u32 *pWidth, mt_u32 *pHeight)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)TextOut;

    *pWidth = p_text_hdl->text_attr.FontAttr.MaxWidth;
    *pHeight = p_text_hdl->text_attr.FontAttr.Height;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif	
}

mt_s32 MT_GO_GetTextAttr(mt_handle TextOut, MTGO_TEXTOUTATTR_S *pTextOutAttr)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)TextOut;
    if (p_text_hdl == NULL) {
	MT_ERR_MTGO("TextOut is null!\n");
	return MT_FAILURE;
    }
    pTextOutAttr->BgColor = p_text_hdl->text_attr.BgColor;
    pTextOutAttr->FgColor = p_text_hdl->text_attr.FgColor;
    pTextOutAttr->FontAttr.MaxWidth = p_text_hdl->text_attr.FontAttr.MaxWidth;
    pTextOutAttr->FontAttr.Height = p_text_hdl->text_attr.FontAttr.Height;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif	
}

mt_s32 MT_GO_SetTextBGTransparent(mt_handle TextOut, MT_BOOL bTransparent)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)TextOut;
    if (p_text_hdl == NULL) {
	MT_ERR_MTGO("TextOut is null!\n");
	return MT_FAILURE;
    }
    p_text_hdl->bg_transparent = bTransparent;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_SetTextBGColor(mt_handle TextOut, MT_COLOR Color)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)TextOut;
    if (p_text_hdl == NULL) {
	MT_ERR_MTGO("TextOut is null!\n");
	return MT_FAILURE;
    }
    p_text_hdl->text_attr.BgColor = Color;
    p_text_hdl->bg_color_valid = MT_TRUE;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_SetTextColor(mt_handle TextOut, MT_COLOR Color)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)TextOut;
    if (p_text_hdl == NULL) {
	MT_ERR_MTGO("TextOut is null!\n");
	return MT_FAILURE;
    }
    p_text_hdl->text_attr.FgColor = Color;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

#ifdef MTGO_TEXT_SUPPORT

static void Char_to_Unicode(mt_u8 *strChar, wchar_t *codeUnic, mt_u32 *code_num)
{
    mt_u32 n;
    mt_u32 i = 0;
    wchar_t codeChar;
    mt_u32 text_length = strlen((char *)strChar);

    for (n = 0; n < text_length; n++) {
	if (strChar[n] > 0x80) {
	    codeChar = (((wchar_t)strChar[n]) << 8) | strChar[n + 1];
	    ++n;
	    codeUnic[i++] = (wchar_t)FT2_CharToUnicode((WORD)codeChar);
	} else {
	    codeUnic[i++] = (wchar_t)strChar[n];
	}
    }
    *code_num = i;
}

static mt_void Char2UniCount(const mt_u8 *strChar, mt_u32 *code_num)
{
    mt_u32 n;
    mt_u32 i = 0;
    mt_u32 text_length = strlen((char *)strChar);

    for (n = 0; n < text_length; n++) {
	if (strChar[n] > 0x80) {
	    ++n;
	    ++i;
	} else {
	    ++i;
	}
    }
    *code_num = i;
}

static int Get_Glyphs_Frm_Wstr(FT_Face face, wchar_t *wstr, mt_u32 code_num, TGlyph glyphs[], mt_u32 char_distance)
{
    mt_u32 n;
    TGlyph *glyph = glyphs;
    int pen_x = 0;
    int pen_y = 0;
    int error;
    FT_GlyphSlot slot = face->glyph;
#if USE_KERNING
    mt_u32 use_kerning = FT_HAS_KERNING(face);
    mt_u32 previous = 0;
    MT_INFO_MTGO("use_kerning:%d\n", use_kerning);
#endif

    for (n = 0; n < code_num; n++) {
	glyph->index = FT_Get_Char_Index(face, (FT_ULong)(wstr[n]));
#if USE_KERNING
	if (use_kerning && previous && glyph->index) {
	    FT_Vector delta;
	    FT_Get_Kerning(face, previous, glyph->index, FT_KERNING_DEFAULT, &delta);
	    pen_x += delta.x >> 6;
	}
#endif
	glyph->pos.x = pen_x;
	glyph->pos.y = pen_y;

	error = FT_Load_Glyph(face, glyph->index, FT_LOAD_DEFAULT);
	if (error)
	    continue;
	error = FT_Get_Glyph(face->glyph, &glyph->image);
	if (error)
	    continue;

	pen_x += slot->advance.x >> 6;
	pen_x += (int)char_distance;
#if USE_KERNING
	previous = glyph->index;
#endif
	glyph++;
    }
    /* count number of glyphs loaded */
    //printf("Get_Glyphs_Frm_Wstr end\n");
    return (glyph - glyphs);
}

static mt_s32 Get_Glyphs_Frm_Wstr_Clip(FT_Face face, wchar_t *wstr, TGlyph glyphs[], mt_u32 char_distance, mt_u32 line_width, const mt_u8 *p_text, mt_u32 *p_byte_num)
{
    int n;
    TGlyph *glyph = glyphs;
    int pen_x = 0;
    int pen_y = 0;
    int error;
    mt_u32 i = 0;
    mt_u32 text_length;
    wchar_t codeChar;
    FT_GlyphSlot slot = face->glyph;

    text_length = strlen((char *)p_text);

    for (n = 0; n < (int)text_length; n++) {
	if (p_text[n] > 0x80) {
	    codeChar = (((wchar_t)p_text[n]) << 8) | p_text[n + 1];
	    wstr[i] = FT2_CharToUnicode((WORD)codeChar);
	    ++n;
	} else if ((p_text[n] == 0xd) && (p_text[n + 1] == 0xa)) {
	    *p_byte_num = (mt_u32)(n + 2);
	    goto Exit;
	} else {
	    wstr[i] = p_text[n];
	}

	glyph->index = FT_Get_Char_Index(face, (FT_Long)(wstr[i]));
	glyph->pos.x = pen_x;
	glyph->pos.y = pen_y;
	error = FT_Load_Glyph(face, glyph->index, FT_LOAD_DEFAULT);
	if (error)
	    continue;
	error = FT_Get_Glyph(slot, &glyph->image);
	if (error)
	    continue;
	pen_x += slot->advance.x >> 6;

	if (pen_x > (int)line_width) {
	    break;
	}

	i++;
	glyph++;
    }

    if (p_text[n] > 0x80) {
	*p_byte_num = (mt_u32)(n - 1);
    } else if (n == (int)text_length) {
	*p_byte_num = (mt_u32)n;
    } else if ((p_text[n] < 0x21) || (p_text[n] == 0x80)) {
	*p_byte_num = (mt_u32)n;
    } else {
	while ((p_text[n] >= 0x21) && (p_text[n] <= 0x7E)) {
	    n--;
	    glyph--;
	}
	if (p_text[n] > 0x80) {
	    *p_byte_num = (mt_u32)(n + 1);
	    glyph++;
	} else {
	    *p_byte_num = (mt_u32)n;
	}
    }

Exit:
    return (glyph - glyphs);
}

static void Draw_Glyphs(TGlyph glyphs[], FT_UInt num_glyphs, mt_u8 *buf, mt_u32 buf_pitch, mt_u32 ascender, MTGO_TEXT_STYLE_E style, mt_u32 bold_strength)
{
    mt_u32 n;
    int error;
    int i, j;
    int x;
    int y;
    float lean = 0.3f;
    FT_Matrix matrix;
    for (n = 0; n < num_glyphs; n++) {
	if ((style & MTGO_TEXT_STYLE_ITALIC) == MTGO_TEXT_STYLE_ITALIC) {
	    matrix.xx = 0x10000L;
	    matrix.xy = (FT_Fixed)(lean * 0x10000L);
	    matrix.yx = 0;
	    matrix.yy = 0x10000L;
	    FT_Glyph_Transform(glyphs[n].image, &matrix, 0);
	}
	error = FT_Glyph_To_Bitmap(&glyphs[n].image, FT_RENDER_MODE_NORMAL, 0, 1);
	if (!error) {
	    FT_BitmapGlyph bit = (FT_BitmapGlyph)glyphs[n].image;
	    FT_Bitmap *bitmap = &bit->bitmap;
	    mt_u8 *p_bmp_buf;
	    if ((style & MTGO_TEXT_STYLE_BOLD) == MTGO_TEXT_STYLE_BOLD) {
		FT_Bitmap_Embolden(library, bitmap, (FT_Pos)bold_strength, (FT_Pos)bold_strength);
	    }
	    p_bmp_buf = bitmap->buffer;
	    x = glyphs[n].pos.x + bit->left;
	    y = (int)(glyphs[n].pos.y) + (int)ascender - (int)(bit->top);

	    if (GRAY_TH > 0) {
		for (i = 0; i < (int)bitmap->rows; i++) {
		    for (j = 0; j < (int)bitmap->width; j++) {
			if (p_bmp_buf[i * bitmap->pitch + j] < GRAY_TH)
			    buf[(i + y) * (int)buf_pitch + x + j] = 0;
			else
			    buf[(i + y) * (int)buf_pitch + x + j] = p_bmp_buf[i * bitmap->pitch + j];
		    }
		}
	    } else {
		for (i = 0; i < bitmap->rows; i++) {
		    memcpy(buf + x + (i + y) * (int)buf_pitch, p_bmp_buf + i * bitmap->pitch, bitmap->width);
		}
	    }
	    FT_Done_Glyph(glyphs[n].image);
	}
    }
}


static void Draw_Glyphs_Arr(TGlyph glyphs[], FT_UInt num_glyphs, mt_handle *p_text_arr, mt_u32 bmp_num, mt_u32 ascender, MTGO_TEXT_STYLE_E style, mt_u32 bold_strength)
{
    mt_u32 n;
    int error;
    int i, j;
    int x;
    int y;
    float lean = 0.3f;
    FT_Matrix matrix;
    mt_char *buf1 = NULL;
    mt_char *buf2 = NULL;
    mt_u32 buf_pitch1 = 0;
    mt_u32 buf_pitch2 = 0;

    MTGO_SURFACE_S *p_text_surface;
    MTGO_SURFACE_S *p_text_surface_next = NULL;

    int bmp_id = 0;
    //mt_u32 cur_pos = 0;
    //mt_u32 next_pos = 0;
    mt_u32 cur_end = 0;
    mt_u32 prev_end = 0;

    p_text_surface = (MTGO_SURFACE_S *)p_text_arr[0];
    cur_end = (mt_u32)(p_text_surface->Width);

    for (n = 0; n < num_glyphs; n++) {
	if ((style & MTGO_TEXT_STYLE_ITALIC) == MTGO_TEXT_STYLE_ITALIC) {
	    matrix.xx = 0x10000L;
	    matrix.xy = (FT_Fixed)(lean * 0x10000L);
	    matrix.yx = 0;
	    matrix.yy = 0x10000L;
	    FT_Glyph_Transform(glyphs[n].image, &matrix, 0);
	}
	error = FT_Glyph_To_Bitmap(&glyphs[n].image, FT_RENDER_MODE_NORMAL, 0, 1);
	if (!error) {
	    FT_BitmapGlyph bit = (FT_BitmapGlyph)glyphs[n].image;
	    FT_Bitmap *bitmap = &bit->bitmap;
	    mt_u8 *p_bmp_buf;
	    if ((style & MTGO_TEXT_STYLE_BOLD) == MTGO_TEXT_STYLE_BOLD) {
		FT_Bitmap_Embolden(library, bitmap, (FT_Pos)bold_strength, (FT_Pos)bold_strength);
	    }
	    p_bmp_buf = bitmap->buffer;
	    x = glyphs[n].pos.x + bit->left;
	    y = (int)(glyphs[n].pos.y) + (int)ascender - (int)(bit->top);

	    p_text_surface = (MTGO_SURFACE_S *)p_text_arr[bmp_id];
	    buf1 = p_text_surface->Data[0].pData;
	    buf_pitch1 = p_text_surface->Data[0].Pitch;

	    if (bmp_id < (int)bmp_num - 1) {
		p_text_surface_next = (MTGO_SURFACE_S *)p_text_arr[bmp_id + 1];
		buf2 = p_text_surface_next->Data[0].pData;
		buf_pitch2 = p_text_surface_next->Data[0].Pitch;
	    } else {
		buf2 = NULL;
	    }

	    for (i = 0; i < (int)bitmap->rows; i++) {
		for (j = 0; j < (int)bitmap->width; j++) {
		    if (x + j < (int)cur_end) {
			if (p_bmp_buf[i * bitmap->pitch + j] < GRAY_TH)
			    buf1[(i + y) * (int)buf_pitch1 + x + j - (int)prev_end] = 0;
			else
			    buf1[(i + y) * (int)buf_pitch1 + x + j - (int)prev_end] = p_bmp_buf[i * bitmap->pitch + j];
		    } else if (buf2 != NULL) {
			if (p_bmp_buf[i * bitmap->pitch + j] < GRAY_TH)
			    buf2[(i + y) * (int)buf_pitch2 + x + j - (int)cur_end] = 0;
			else
			    buf2[(i + y) * (int)buf_pitch2 + x + j - (int)cur_end] = p_bmp_buf[i * bitmap->pitch + j];
		    }
		}
	    }

	    if (p_text_surface_next != NULL) {
		if (x + (int)bitmap->width >= (int)cur_end) {
		    prev_end = cur_end;
		    cur_end += p_text_surface_next->Width;
		    bmp_id++;
		}
	    } else {
		MT_ASSERT(0);
	    }
	}
	FT_Done_Glyph(glyphs[n].image);
    }
}
#endif
#if 0
void Draw_Glyphs_Clip(
    TGlyph glyphs[], FT_UInt num_glyphs, mt_u8 *buf, mt_u32 buf_pitch, mt_u32 ascender,
    MTGO_TEXT_STYLE_E style, mt_u32 bold_strength)
{
    int n;
    int error;
    int i, j;
    int x;
    int y;
    float lean = 0.3f;
    FT_Matrix matrix;
    for (n = 0; n < num_glyphs; n++)
    {
        if((style & MTGO_TEXT_STYLE_ITALIC) == MTGO_TEXT_STYLE_ITALIC)
        {
            matrix.xx = 0x10000L;
            matrix.xy = lean * 0x10000L;
            matrix.yx = 0;
            matrix.yy = 0x10000L;
            FT_Glyph_Transform(glyphs[n].image, &matrix, 0); 
        }
        error = FT_Glyph_To_Bitmap(&glyphs[n].image, FT_RENDER_MODE_NORMAL, NULL, 1);
        if ( !error ) 
        {
            FT_BitmapGlyph bit = (FT_BitmapGlyph)glyphs[n].image;
            FT_Bitmap *bitmap = &bit->bitmap;
            mt_u8 *p_bmp_buf;
            if((style & MTGO_TEXT_STYLE_BOLD) == MTGO_TEXT_STYLE_BOLD)
            {
                FT_Bitmap_Embolden(library, bitmap, bold_strength, bold_strength);
            }
            p_bmp_buf = bitmap->buffer;
            x = glyphs[n].pos.x +  bit->left;
            y = glyphs[n].pos.y + ascender - bit->top;

            if(GRAY_TH > 0)
            {
                for(i = 0; i < bitmap->rows; i++)
                {
                    for(j = 0; j < bitmap->width; j++)
                    {
                        if(p_bmp_buf[i * bitmap->pitch + j] < GRAY_TH)
                            buf[(i+y)*buf_pitch+x+j] = 0;
                        else
                            buf[(i+y)*buf_pitch+x+j]  = p_bmp_buf[i * bitmap->pitch + j];
                    }
                }
            }
            else
            {
                for(i = 0; i < bitmap->rows; i++)
                {
                    memcpy(buf + x + (i + y) * buf_pitch, p_bmp_buf + i * bitmap->pitch, bitmap->width);
                }
            }
            FT_Done_Glyph(glyphs[n].image);
        }
    }
}
#endif

mt_s32 MT_GO_TextOut(mt_handle TextOut, mt_handle Surface, const mt_char *pText,
                     const MT_RECT *pRect)
{
    MT_GO_TextOutEx(TextOut, Surface, pText, pRect, MTGO_LAYOUT_LEFT | MTGO_LAYOUT_TOP);
    return MT_SUCCESS;
}

mt_s32 MT_GO_TextOut_Clip(mt_handle TextOut, mt_handle Surface, const mt_u8 *pText,
                          const MT_RECT *pRect, mt_u32 LineWidth, mt_u32 PreLoadByteNum, mt_u32 *pByteNum)
{
    MT_GO_TextOutEx_Clip(
        TextOut, Surface, (mt_u8 *)pText, pRect, MTGO_LAYOUT_LEFT | MTGO_LAYOUT_TOP, LineWidth, PreLoadByteNum, pByteNum);
    return MT_SUCCESS;
}
#ifdef MTGO_TEXT_SUPPORT

static u32 utf8_to_unicode(mt_u16 *p_dst, mt_u32 outsize, mt_u8 *p_src, mt_u32 insize)
{
  mt_u32 totalNum = 0;
  mt_u8 *p_data = p_src;
  mt_u32 resultsize = 0;
  mt_u8 *p_tmp = (mt_u8 *)p_dst;
  mt_u32 i = 0;
  mt_u8 t1 = 0, t2 = 0;
  mt_u16 t3 = 0, t4 = 0, t5 = 0;
  mt_u32 tmp_size = 0;

  if(p_dst == NULL || p_src == NULL || insize == 0)
  {
    return -1;
  }

  for(i = 0; i < insize; i++)
  {
    if (*p_data >= 0x00 && *p_data <= 0x7f)
    {
      p_data ++;
      totalNum ++;
    }
    else if ((*p_data & (0xe0))== 0xc0)
    {
      p_data += 2;
      totalNum ++;
    }
    else if ((*p_data & (0xf0))== 0xe0)
    {
      p_data += 3;
      totalNum ++;
    }
  }

  if(outsize < totalNum)
  {
    return -1;
  }
  

  p_data = p_src;
  while(*p_data)
  {
    if (*p_data >= 0x00 && *p_data <= 0x7f)
    {
      *p_tmp = *p_data;
      p_tmp ++;
      p_tmp ++;
      resultsize += 1;
    }
    else if ((*p_data & 0xe0)== 0xc0)
    {
      t1 = *p_data & (0x1f);
      p_data ++;
      t2 = *p_data & (0x3f);

      *p_tmp = t2 | ((t1 & (0x03)) << 6);
      p_tmp ++;
      *p_tmp = t1 >> 2;
      p_tmp ++;
      resultsize += 1;
    }
    else if ((*p_data & (0xf0))== 0xe0)
    {
      t3 = *p_data & (0x1f);
      p_data ++;
      t4 = *p_data & (0x3f);
      p_data ++;
      t5 = *p_data & (0x3f);

      *p_tmp = ((t4 & (0x03)) << 6) | t5;
      p_tmp ++;
      *p_tmp = (t3 << 4) | (t4 >> 2);
      p_tmp ++;
      resultsize += 1;
    }
    p_data ++;
    tmp_size ++;
    if (tmp_size >= insize)
    {
      break;
    }
  }
  
  return resultsize;

}

static void utf8_to_Unicode(mt_u8 *strChar, mt_u16 *codeUnic, mt_u32 *code_num)
{
	*code_num = utf8_to_unicode(codeUnic, strlen((char*)strChar), strChar, strlen((char*)strChar));
}

static void Unicode_to_wchar(mt_u16 *strChar, wchar_t *codeUnic, mt_u32 code_num)
{
    mt_u32 n;
    mt_u32 i = 0;
	
    for (n = 0; n < code_num; n++) {
	    codeUnic[i++] = strChar[n];
    }
}

static mt_s32 decText(mt_handle hTextOut, mt_handle *pTextSurfaceHandle, const mt_char *pText)
{
    MTGO_SURFACE_S *p_text_surface = NULL;
    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    mt_u32 num_glyphs;
    FT_BBox bbox;
    int line_width;
    int line_height;
    mt_s32 ret;
    TGlyph *glyph;
    mt_u32 ascender;
    mt_u32 descender;
    wchar_t *codeUnic;
    mt_u16 *unic_tmp;
    mt_u32 code_num;
    FT_Face face;
    
    if (p_text_hdl == NULL) {
	MT_ERR_MTGO("TextOut is null!\n");
	return MT_FAILURE;
    }
    if (pText == NULL) {
	MT_ERR_MTGO("pText is null!\n");
	return MT_FAILURE;
    }
    //convert char to unicode
	unic_tmp = (mt_u16*)malloc(sizeof(mt_u16) * strlen(pText));
	memset(unic_tmp, 0, sizeof(mt_u16) * strlen(pText));
	utf8_to_Unicode((mt_u8 *)pText, unic_tmp, &code_num);
    
    codeUnic = (wchar_t *)malloc(sizeof(wchar_t) * strlen(pText));
	memset(codeUnic, 0, sizeof(wchar_t) * strlen(pText));
    Unicode_to_wchar((mt_u16 *)unic_tmp, codeUnic, code_num);
	free(unic_tmp);
    
    glyph = malloc(code_num * sizeof(TGlyph));

    //get glyphs
    face = p_text_hdl->ft2CtxtBase->face;
    num_glyphs = (mt_u32)Get_Glyphs_Frm_Wstr(face, codeUnic, code_num, glyph, p_text_hdl->char_distance);

    //calculate string width and height
    FT_Glyph_Get_CBox(glyph[num_glyphs - 1].image, FT_GLYPH_BBOX_TRUNCATE, &bbox);
    line_width = glyph[num_glyphs - 1].pos.x + bbox.xMax * 2;
    if (p_text_hdl->style & MTGO_TEXT_STYLE_ITALIC)
	line_width += bbox.xMax / 2;
    ascender = (mt_u32)(face->size->metrics.ascender >> 6);
    descender = (mt_u32)(face->size->metrics.descender >> 6);
    line_height = ((int)ascender - (int)descender + ((int)(face->size->metrics.height) >> 6)) / 2;
	if (((int)ascender - (int)descender + ((int)(face->size->metrics.height) >> 6)) % 2 != 0)
		line_height += 1;

    //store gray-8 bmp to text_surface
    ret = MT_GO_CreateSurface(line_width, line_height, MTGO_PF_GRAY_8, pTextSurfaceHandle);
    if (ret != MT_SUCCESS) {
	MT_ERR_MTGO("create text surface failed!\n");
	free(glyph);
	free(codeUnic);
	return MT_FAILURE;
    }

    p_text_surface = (MTGO_SURFACE_S *)(*pTextSurfaceHandle);
/*
    cc = Sample_Get_key2();
    printf("Fill the text mask surface, key: f\n");
    if(cc == 'f')
    {
        MT_RECT  rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = line_width;
        rect.h = line_height;
       
        MT_GO_FillRect((mt_handle)p_text_surface, &rect, 0xcc, MTGO_COMPOPT_NONE);
    }
*/      
    MT_INFO_MTGO("decText: surf [%d][%d] [%d][%d]\n", p_text_surface->Width, p_text_surface->Height, line_width, line_height);

    MT_INFO_MTGO("decText: surf [%x][%x] [%x]\n", p_text_surface->Data[0].pData, p_text_surface->Data[0].pPhyData, p_text_surface->Data[0].Pitch);
    
    //Sample_Get_key2();
   
    Draw_Glyphs(glyph, num_glyphs, (mt_u8 *)(p_text_surface->Data[0].pData), p_text_surface->Data[0].Pitch, ascender, p_text_hdl->style, BOLD_STRENGTH * p_text_hdl->text_attr.FontAttr.Height);

    //Sample_Get_key2();
    
    free(glyph);
    free(codeUnic);

    //Sample_Get_key2();
    
    return ret;
}



static mt_s32 decTextArrary(mt_handle hTextOut, mt_handle **pTextSurfaceArr, mt_u32 *bmp_num, const mt_char *pText)
{
    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    mt_u32 num_glyphs;
    FT_BBox bbox;
    int line_width;
    int line_height;
    mt_s32 ret = MT_SUCCESS;
    TGlyph *glyph;
    mt_u32 ascender;
    mt_u32 descender;
    wchar_t *codeUnic;
    mt_u32 code_num;
    FT_Face face;
    int i;
    mt_handle *p_surface_arr;

    if (p_text_hdl == NULL) {
	MT_ERR_MTGO("TextOut is null!\n");
	return MT_FAILURE;
    }
    if (pText == NULL) {
	MT_ERR_MTGO("pText is null!\n");
	return MT_FAILURE;
    }
    //convert char to unicode
    codeUnic = (wchar_t *)malloc(sizeof(wchar_t) * strlen(pText));
    Char_to_Unicode((mt_u8 *)pText, codeUnic, &code_num);
    glyph = (TGlyph *)malloc(code_num * sizeof(TGlyph));

    //get glyphs
    face = p_text_hdl->ft2CtxtBase->face;
    num_glyphs = (mt_u32)Get_Glyphs_Frm_Wstr(face, codeUnic, code_num, glyph, p_text_hdl->char_distance);

    //calculate string width and height
    FT_Glyph_Get_CBox(glyph[num_glyphs - 1].image, FT_GLYPH_BBOX_TRUNCATE, &bbox);
    line_width = glyph[num_glyphs - 1].pos.x + bbox.xMax;
    if (p_text_hdl->style & MTGO_TEXT_STYLE_ITALIC)
	line_width += bbox.xMax / 2;
    ascender = (mt_u32)(face->size->metrics.ascender >> 6);
    descender = (mt_u32)(face->size->metrics.descender >> 6);
    line_height = ((int)ascender - (int)descender + ((int)(face->size->metrics.height) >> 6)) / 2;
	if (((int)ascender - (int)descender + ((int)(face->size->metrics.height) >> 6)) % 2 != 0)
		line_height += 1;

    //store gray-8 bmp to text_surface arrary
    *bmp_num = line_width / MAX_BMPSURFACE_WIDTH + ((line_width % MAX_BMPSURFACE_WIDTH) != 0);

    MT_INFO_MTGO("line_width:%d, bmp_num:%d, bbox.xMax:%d", line_width, *bmp_num, (mt_s32)bbox.xMax);

    p_surface_arr = (mt_handle *)malloc(*bmp_num * sizeof(mt_handle));
    for (i = 0; i < (int)(*bmp_num); i++) {
	if (i == (int)(*bmp_num) - 1) {
	    ret = MT_GO_CreateSurface(line_width - i * MAX_BMPSURFACE_WIDTH,
	                              line_height, MTGO_PF_GRAY_8, &p_surface_arr[i]);
	} else {
	    ret = MT_GO_CreateSurface(MAX_BMPSURFACE_WIDTH, line_height, MTGO_PF_GRAY_8, &p_surface_arr[i]);
	}
    }

    if (ret != MT_SUCCESS) {
	MT_ERR_MTGO("create text surface failed!\n");
	free(glyph);
	free(codeUnic);
	return MT_FAILURE;
    }

    Draw_Glyphs_Arr(glyph, num_glyphs, p_surface_arr, *bmp_num,
                    ascender, p_text_hdl->style, BOLD_STRENGTH * p_text_hdl->text_attr.FontAttr.Height);

    free(glyph);
    free(codeUnic);
    *pTextSurfaceArr = p_surface_arr;
    return ret;
}

static mt_s32 decTextClip(mt_handle hTextOut, mt_handle *pTextSurface,
                     mt_u8 *pText, mt_u32 LineWidth, mt_u32 PreLoadByteNum, mt_u32 *pByteNum)
{
    MTGO_SURFACE_S *p_text_surface;
    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    mt_u32 num_glyphs;
    FT_BBox bbox;
    int line_width;
    int line_height;
    mt_s32 ret;
    TGlyph *glyph;
    mt_u32 ascender;
    mt_u32 descender;
    wchar_t *codeUnic;
    mt_u32 code_num;
    mt_u32 byte_num;
    //mt_s32 i;
    FT_Face face;

    if (p_text_hdl == NULL) {
	MT_ERR_MTGO("TextOut is null!\n");
	return MT_FAILURE;
    }
    if (pText == NULL) {
	MT_ERR_MTGO("pText is null!\n");
	return MT_FAILURE;
    }

    Char2UniCount(pText, &code_num);
    codeUnic = (wchar_t *)malloc(sizeof(wchar_t) * code_num);
    glyph = malloc(sizeof(TGlyph) * code_num);

    //get glyphs
    face = p_text_hdl->ft2CtxtBase->face;
    num_glyphs = (mt_u32)Get_Glyphs_Frm_Wstr_Clip(face, codeUnic, glyph, p_text_hdl->char_distance, LineWidth, pText, &byte_num);
    if (byte_num < PreLoadByteNum) {
	memset(pText + byte_num, 0, PreLoadByteNum - byte_num);
    }
    *pByteNum = byte_num;

    //calculate string width and height
    FT_Glyph_Get_CBox(glyph[num_glyphs - 1].image, FT_GLYPH_BBOX_TRUNCATE, &bbox);
    line_width = glyph[num_glyphs - 1].pos.x + bbox.xMax;
    ascender = (mt_u32)(face->size->metrics.ascender) >> 6;
    descender = (mt_u32)(face->size->metrics.descender) >> 6;
    line_height = ((int)ascender - (int)descender + ((int)(face->size->metrics.height) >> 6)) / 2;
	if (((int)ascender - (int)descender + ((int)(face->size->metrics.height) >> 6)) % 2 != 0)
		line_height += 1;

    //store gray-8 bmp to text_surface
    ret = MT_GO_CreateSurface(line_width, line_height, MTGO_PF_GRAY_8, pTextSurface);
    if (ret != MT_SUCCESS) {
	MT_ERR_MTGO("create text surface failed!\n");
	free(glyph);
	free(codeUnic);
	return MT_FAILURE;
    }
    p_text_surface = (MTGO_SURFACE_S *)(*pTextSurface);

    //   Draw_Glyphs_Clip(glyph, num_glyphs, (mt_u8 *)p_text_surface->p_buf_mmap, p_text_surface->pitch,
    //          ascender, p_text_hdl->style, BOLD_STRENGTH * p_text_hdl->text_attr.FontAttr.Height);

    Draw_Glyphs(glyph, num_glyphs, (mt_u8 *)p_text_surface->Data[0].pData, p_text_surface->Data[0].Pitch,
                ascender, p_text_hdl->style, BOLD_STRENGTH * p_text_hdl->text_attr.FontAttr.Height);
    free(glyph);
    free(codeUnic);
    return ret;
}

#endif



mt_s32 MT_GO_TextOutEx(mt_handle hTextOut, mt_handle hSurface,
                       const mt_char *pText, const MT_RECT *pRect,
                       mt_u32 Style)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_SURFACE_S *p_targetSurface = (MTGO_SURFACE_S *)hSurface;
    mt_handle fgSurfaceHandle = 0;
    mt_handle text_mask_surface_handle = 0;
    //MTGO_MASKOPT_S maskOpt = { 0 };
    MTGO_SURFACE_S *text_mask_surface = NULL;
    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    MT_RECT win_rect = { 0 };
    MT_RECT dst_rect = { 0 };
    MT_RECT text_rect = { 0 };
    int line_width = 0;
    int line_height = 0;
    MT_POS pos = { 0 };
    mt_s32 ret = MT_SUCCESS;
    
    if (p_targetSurface == NULL) {
	MT_ERR_MTGO("err: Target Surface is null!\n");
	return MT_FAILURE;
    }

    if (pRect == NULL) {
	win_rect.x = 0;
	win_rect.y = 0;
	win_rect.w = p_targetSurface->Width;
	win_rect.h = p_targetSurface->Height;
    } else {
	win_rect.x = pRect->x;
	win_rect.y = pRect->y;
	win_rect.w = pRect->w;
	win_rect.h = pRect->h;
    }

    ret = decText(hTextOut, &text_mask_surface_handle, pText);
    if (ret != MT_SUCCESS) {
	MT_ERR_MTGO("dec text failed!\n");
	return ret;
    }

    text_mask_surface = (MTGO_SURFACE_S *)text_mask_surface_handle;
    MT_ASSERT(text_mask_surface_handle != 0);
    line_width = text_mask_surface->Width;
    line_height = text_mask_surface->Height;

    MT_INFO_MTGO("MT_GO_TextOutEx: [%d][%d] [%d][%d][%d][%d] [%s]\n", text_mask_surface->Width, text_mask_surface->Height, win_rect.x, win_rect.y, win_rect.w, win_rect.h, pText);
    MT_GO_Dump_Surface_Font(text_mask_surface);
      
    MT_GO_Dump_Surface_Font_ARGB8888(text_mask_surface_handle, &fgSurfaceHandle);
    
    switch (Style) {
    case MTGO_LAYOUT_LEFT:
    case MTGO_LAYOUT_TOP:
    case MTGO_LAYOUT_LEFT | MTGO_LAYOUT_TOP:
	pos.x = 0;
	pos.y = 0;
	break;
    case MTGO_LAYOUT_RIGHT:
    case MTGO_LAYOUT_RIGHT | MTGO_LAYOUT_TOP:
	pos.x = win_rect.w - line_width;
	pos.y = 0;
	break;
    case MTGO_LAYOUT_BOTTOM:
    case MTGO_LAYOUT_LEFT | MTGO_LAYOUT_BOTTOM:
	pos.x = 0;
	pos.y = win_rect.h - line_height;
	break;
    case MTGO_LAYOUT_HCENTER | MTGO_LAYOUT_BOTTOM:
	pos.x = (win_rect.w - line_width) / 2;
	pos.y = win_rect.h - line_height;
	break;
    case MTGO_LAYOUT_HCENTER:
    case MTGO_LAYOUT_HCENTER | MTGO_LAYOUT_TOP:
	pos.x = (win_rect.w - line_width) / 2;
	pos.y = 0;
	break;
    case MTGO_LAYOUT_VCENTER:
    case MTGO_LAYOUT_VCENTER | MTGO_LAYOUT_LEFT:
	pos.x = 0;
	pos.y = (win_rect.h - line_height) / 2;
	break;
    case MTGO_LAYOUT_RIGHT | MTGO_LAYOUT_BOTTOM:
	pos.x = win_rect.w - line_width;
	pos.y = win_rect.h - line_height;
	break;
    case MTGO_LAYOUT_HCENTER | MTGO_LAYOUT_VCENTER:
	pos.x = (win_rect.w - line_width) / 2;
	pos.y = (win_rect.h - line_height) / 2;
	break;
    case MTGO_LAYOUT_WRAP:
    case MTGO_LAYOUT_WORDELLIPSIS:
    case MTGO_LAYOUT_BUTT:
	MT_ERR_MTGO("layout mode not supported\n");
	return MT_FAILURE;
    }

    dst_rect.x = pos.x + win_rect.x;
    dst_rect.y = pos.y + win_rect.y;
    if (line_width + pos.x <= win_rect.w)
	dst_rect.w = line_width;
    else
	dst_rect.w = win_rect.w - pos.x;

    if (line_height + pos.y <= win_rect.h)
	dst_rect.h = line_height;
    else
	dst_rect.h = win_rect.h - pos.y;

    if ((p_text_hdl->bg_color_valid == MT_TRUE) && (p_text_hdl->bg_transparent == MT_FALSE)) {
	MT_GO_FillRect((mt_handle)p_targetSurface, &dst_rect, p_text_hdl->text_attr.BgColor, MTGO_COMPOPT_NONE);
	//printf("bgcolor:0x%08x, fgcolor:0x%08x\n", p_text_hdl->text_attr.BgColor, p_text_hdl->text_attr.FgColor);
    }


    text_rect.x = 0;
    text_rect.y = 0;
    text_rect.w = dst_rect.w;
    text_rect.h = dst_rect.h;

    MT_INFO_MTGO("------------------\n");
    MT_INFO_MTGO("PARAM: [%d][%d][%x][%x]\n", line_width, line_height, p_text_hdl->text_attr.FgColor, p_text_hdl->text_attr.BgColor);
    MT_INFO_MTGO("PARAM: [%d][%d][%d][%d]\n", win_rect.x, win_rect.y, win_rect.w, win_rect.h);

    MT_INFO_MTGO("PARAM: [%d][%d][%d][%d]\n", text_rect.x, text_rect.y, text_rect.w, text_rect.h);
    MT_INFO_MTGO("PARAM: [%d][%d][%d][%d]\n", dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);
    
    //MT_GO_SetSurfaceColorKey(text_mask_surface_handle, color);   // color = 0
    //blitOpt.ColorKeyFrom = MTGO_CKEY_NONE;
    //blitOpt.EnableRop = MT_TRUE;
    //blitOpt.Rop = MTGO_ROP_PS;
    //blitOpt.RopAlpha = MTGO_ROP_PS;
#if 0    
    blitOpt.EnableGlobalAlpha = MT_TRUE;
    blitOpt.PixelAlphaComp = MTGO_COMPOPT_SRCOVER;
    MT_INFO_MTGO("MT_GO_TEXT: MT_GO_Blit: 1\n");
    ret = MT_GO_Blit(text_mask_surface_handle, &text_rect, hSurface, &dst_rect, &blitOpt);
    MT_INFO_MTGO("MT_GO_TEXT: MT_GO_Blit: 2\n");
    MT_ASSERT(ret == MT_SUCCESS);
#else
	TDE2_OPT_S opt = {0};
	TDE2_SURFACE_S stDst;
	TDE2_SURFACE_S stEx;
	mt_handle handle;
	TDE2_RECT_S exRect;
	TDE2_RECT_S dstRect;
	
	opt.enPaint = MT_TRUE;
	opt.stPaintOpt.paint_color = p_text_hdl->text_attr.FgColor;
	opt.stPaintOpt.paint_type = TDE2_PAINT_TYPE_COLOR;
	opt.enAMapMix = MT_TRUE;
	opt.enAluCmd = TDE2_ALUCMD_BLEND;
	opt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_SRCOVER;

	opt.enColorKeyMode = TDE2_COLORKEY_MODE_EX; 
	opt.enColorKeySelect = TDE2_MASK_KEY_MATCH;

	opt.unColorKeyValue.struCkARGB.stAlpha.u8CompMin = 0;
	opt.unColorKeyValue.struCkARGB.stAlpha.u8CompMax = 0;	
	opt.unColorKeyValue.struCkARGB.stRed.u8CompMin = 0;
	opt.unColorKeyValue.struCkARGB.stRed.u8CompMax = 0;
	opt.unColorKeyValue.struCkARGB.stGreen.u8CompMin = 0;
	opt.unColorKeyValue.struCkARGB.stGreen.u8CompMax = 0;
	opt.unColorKeyValue.struCkARGB.stBlue.u8CompMin = 0;
	opt.unColorKeyValue.struCkARGB.stBlue.u8CompMax = 0;

	dstRect.s32Xpos = dst_rect.x;
	dstRect.s32Ypos = dst_rect.y;
	dstRect.u32Width = (mt_u32)(dst_rect.w);
	dstRect.u32Height = (mt_u32)(dst_rect.h);
	
	exRect.s32Xpos = 0;
	exRect.s32Ypos = 0;
	exRect.u32Width = (mt_u32)(text_mask_surface->Width);
	exRect.u32Height = (mt_u32)(text_mask_surface->Height);
	
	MT_GO_MEMSurfaceToTDESurface(hSurface, &stDst);
	MT_GO_MEMSurfaceToTDESurface(text_mask_surface_handle, &stEx);

	handle = (mt_handle)MT_TDE2_BeginJob();   

	ret = MT_TDE2_Bitblit_3src((TDE_HANDLE)handle, NULL, NULL,
				   NULL, NULL, &stEx, &exRect, &stDst, &dstRect, &opt);

	MT_TDE2_EndJob((TDE_HANDLE)handle, MT_FALSE, MT_TRUE, 500);


#endif
    //Sample_Get_key2();
    
    MT_GO_FreeSurface(fgSurfaceHandle);
    MT_GO_FreeSurface(text_mask_surface_handle);
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif
}

mt_s32 MT_GO_TextOutEx_Clip(mt_handle hTextOut, mt_handle hSurface,
                            mt_u8 *pText, const MT_RECT *pRect,
                            mt_u32 Style, mt_u32 LineWidth,
                            mt_u32 PreLoadByteNum, mt_u32 *pByteNum)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_SURFACE_S *p_surface_hdl = (MTGO_SURFACE_S *)hSurface;
    mt_handle fgSurfaceHandle = 0;
    MTGO_MASKOPT_S maskOpt = { 0 };

    mt_handle text_surface = 0;
    MTGO_SURFACE_S *p_text_surface = NULL;
    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    MT_RECT win_rect = { 0 };
    MT_RECT dst_rect = { 0 };
    MT_RECT text_rect = { 0 };
    int line_width = 0;
    int line_height = 0;
    MT_POS pos = { 0 };
    mt_s32 ret = MT_SUCCESS;

#ifdef __MT_GO_ERR__
    MTGO_PAINTBLIT3OPT_S bltopt = { 0 };
#endif

    if (p_surface_hdl == NULL) {
	MT_ERR_MTGO("Surface is null!\n");
	return MT_FAILURE;
    }
    if (pRect == NULL) {
	win_rect.x = 0;
	win_rect.y = 0;
	win_rect.w = p_surface_hdl->Width;
	win_rect.h = p_surface_hdl->Height;
    } else {
	win_rect.x = pRect->x;
	win_rect.y = pRect->y;
	win_rect.w = pRect->w;
	win_rect.h = pRect->h;
    }
    ret = decTextClip(hTextOut, &text_surface, pText, LineWidth, PreLoadByteNum, pByteNum);
    if (ret != MT_SUCCESS) {
	MT_ERR_MTGO("dec text failed!\n");
	return ret;
    }
    p_text_surface = (MTGO_SURFACE_S *)text_surface;
    line_width = p_text_surface->Width;
    line_height = p_text_surface->Height;

    switch (Style) {
    case MTGO_LAYOUT_LEFT:
    case MTGO_LAYOUT_TOP:
    case MTGO_LAYOUT_LEFT | MTGO_LAYOUT_TOP:
	pos.x = 0;
	pos.y = 0;
	break;
    case MTGO_LAYOUT_RIGHT:
    case MTGO_LAYOUT_RIGHT | MTGO_LAYOUT_TOP:
	pos.x = win_rect.w - line_width;
	pos.y = 0;
	break;
    case MTGO_LAYOUT_BOTTOM:
    case MTGO_LAYOUT_LEFT | MTGO_LAYOUT_BOTTOM:
	pos.x = 0;
	pos.y = win_rect.h - line_height;
	break;
    case MTGO_LAYOUT_HCENTER | MTGO_LAYOUT_BOTTOM:
	pos.x = (win_rect.w - line_width) / 2;
	pos.y = win_rect.h - line_height;
	break;
    case MTGO_LAYOUT_HCENTER:
    case MTGO_LAYOUT_HCENTER | MTGO_LAYOUT_TOP:
	pos.x = (win_rect.w - line_width) / 2;
	pos.y = 0;
	break;
    case MTGO_LAYOUT_VCENTER:
    case MTGO_LAYOUT_VCENTER | MTGO_LAYOUT_LEFT:
	pos.x = 0;
	pos.y = (win_rect.h - line_height) / 2;
	break;
    case MTGO_LAYOUT_RIGHT | MTGO_LAYOUT_BOTTOM:
	pos.x = win_rect.w - line_width;
	pos.y = win_rect.h - line_height;
	break;
    case MTGO_LAYOUT_HCENTER | MTGO_LAYOUT_VCENTER:
	pos.x = (win_rect.w - line_width) / 2;
	pos.y = (win_rect.h - line_height) / 2;
	break;
    case MTGO_LAYOUT_WRAP:
    case MTGO_LAYOUT_WORDELLIPSIS:
    case MTGO_LAYOUT_BUTT:
	MT_ERR_MTGO("layout mode not supported\n");
	return MT_FAILURE;
    }

    dst_rect.x = pos.x + win_rect.x;
    dst_rect.y = pos.y + win_rect.y;
    if (line_width + pos.x <= win_rect.w)
	dst_rect.w = line_width;
    else
	dst_rect.w = win_rect.w - pos.x;

    if (line_height + pos.y <= win_rect.h)
	dst_rect.h = line_height;
    else
	dst_rect.h = win_rect.h - pos.y;

    if ((p_text_hdl->bg_color_valid == MT_TRUE) && (p_text_hdl->bg_transparent == MT_FALSE)) {
	MT_GO_FillRect((mt_handle)p_surface_hdl, &dst_rect, p_text_hdl->text_attr.BgColor, MTGO_COMPOPT_NONE);
	MT_INFO_MTGO("bgcolor:0x%08x, fgcolor:0x%08x\n", p_text_hdl->text_attr.BgColor, p_text_hdl->text_attr.FgColor);
    }

#ifdef __MT_GO_ERR__
    bltopt.enableAMapMix = MT_TRUE;
    bltopt.enableBld = MT_TRUE;
    bltopt.blendCfg.src_blend_fact = MTGO_ONE;
    bltopt.blendCfg.dst_blend_fact = MTGO_ONE_MINUS_SRC_ALPHA;
    bltopt.paintCfg.paint_color = p_text_hdl->text_attr.FgColor;
    bltopt.paint_type = MTGO_PAINT_TYPE_COLOR;
    MT_GO_SetSurfaceColorKey(text_surface, 0);
    bltopt.enableExCkey = MT_TRUE;

    text_rect.x = 0;
    text_rect.y = 0;
    text_rect.w = dst_rect.w;
    text_rect.h = dst_rect.h;
    MT_GO_PaintBlit3Source(text_surface, &text_rect,
                           (mt_handle)p_surface_hdl, &dst_rect, &bltopt);
#else

    maskOpt.PixelAlphaComp = MTGO_COMPOPT_SRCOVER;

    text_rect.x = 0;
    text_rect.y = 0;
    text_rect.w = dst_rect.w;
    text_rect.h = dst_rect.h;

    MT_INFO_MTGO("PARAM: [%d][%d][%x][%x]\n", line_width, line_height, p_text_hdl->text_attr.FgColor, p_text_hdl->text_attr.BgColor);
    MT_INFO_MTGO("PARAM: [%d][%d][%d][%d]\n", win_rect.x, win_rect.y, win_rect.w, win_rect.h);

    MT_INFO_MTGO("PARAM: [%d][%d][%d][%d]\n", text_rect.x, text_rect.y, text_rect.w, text_rect.h);
    MT_INFO_MTGO("PARAM: [%d][%d][%d][%d]\n", dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);

    ret = MT_GO_CreateSurface(line_width, line_height, MTGO_PF_8888, &fgSurfaceHandle);
    MT_ASSERT(ret == MT_SUCCESS);
    ret = MT_GO_FillRect(fgSurfaceHandle, &text_rect, p_text_hdl->text_attr.FgColor, MTGO_COMPOPT_NONE);
    MT_ASSERT(ret == MT_SUCCESS);

    MT_INFO_MTGO("MT_GO_TEXT: MASKBLIT: 1\n");
    ret = MT_GO_MaskBlit(fgSurfaceHandle, &text_rect, hSurface, &dst_rect, text_surface, &text_rect, &maskOpt);
    MT_INFO_MTGO("MT_GO_TEXT: MASKBLIT: 2\n");
    MT_ASSERT(ret == MT_SUCCESS);
#endif

    MT_GO_FreeSurface(fgSurfaceHandle);
    MT_GO_FreeSurface(text_surface);
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif	
}

#ifdef MTGO_TEXT_SUPPORT

static mt_s32 textGray2Rgb(MT_COLOR Color, mt_u32 Speed, mt_handle *TextSurfaceArr, mt_u32 bmp_num, mt_handle *RgbSurfaceArr)
{
    mt_s32 ret = MT_SUCCESS;

#ifdef __MT_GO_ERR__
    MTGO_PAINTBLIT3OPT_S bltopt = { 0 };
#endif

#ifdef __MT_GO_ERR__

    MTGO_SURFACE_S *p_text_surface;
    MT_RECT rect;
    mt_u32 i;


    bltopt.enableAMapMix = MT_TRUE;
    bltopt.enableRop = MT_TRUE;
    bltopt.ropCfg.rop_a_id = MTGO_ROP_COPYPEN;
    bltopt.ropCfg.rop_c_id = MTGO_ROP_COPYPEN;
    bltopt.ropCfg.rop_pattern = 0;
    bltopt.paintCfg.paint_color = Color;
    bltopt.paint_type = MTGO_PAINT_TYPE_COLOR;
    bltopt.enableExCkey = MT_TRUE;

    for (i = 0; i < bmp_num; i++) {
	p_text_surface = (MTGO_SURFACE_S *)TextSurfaceArr[i];
	if (i == bmp_num - 1) {
	    ret = MT_GO_CreateSurface(p_text_surface->width + Speed, p_text_surface->height, MTGO_PF_ARGB8888, &RgbSurfaceArr[i]);
	} else {
	    ret = MT_GO_CreateSurface(p_text_surface->width, p_text_surface->height, MTGO_PF_ARGB8888, &RgbSurfaceArr[i]);
	}
	if (ret != MT_SUCCESS) {
	    MT_ERR_MTGO("create rgb text surface failed!\n");
	    return ret;
	}
	MT_GO_FillRect(RgbSurfaceArr[i], NULL, 0, MTGO_COMPOPT_NONE);
	MT_GO_SetSurfaceColorKey(p_text_surface, 0);

	rect.x = 0;
	rect.y = 0;
	rect.w = p_text_surface->width;
	rect.h = p_text_surface->height;
	ret = MT_GO_PaintBlit3Source(p_text_surface, NULL,
	                             RgbSurfaceArr[i], &rect, &bltopt);
	if (ret != MT_SUCCESS) {
	    MT_ERR_MTGO("MT_GO_PaintBlit3Source failed!\n");
	}
    }
#endif

    return ret;
}
#endif
mt_s32 MT_GO_SetScroll(mt_handle TextOut, mt_u32 rounds, mt_u32 speed)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)TextOut;
    if (p_text_hdl == NULL) {
	MT_ERR_MTGO("TextOut is null!\n");
	return MT_FAILURE;
    }
    p_text_hdl->scroll.scroll_speed = speed;
    p_text_hdl->scroll.scroll_times = rounds;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_CreateTextScroll(mt_handle hTextOut, mt_handle hSurface,
                              const mt_char *pText, const MT_RECT *pRect)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_SURFACE_S *p_surface_hdl = (MTGO_SURFACE_S *)hSurface;
    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    MT_RECT win_rect;
    MT_RECT mid_rect;
    mt_s32 ret;
    mt_u32 i;

    if (p_surface_hdl == NULL) {
	MT_ERR_MTGO("Surface is null!\n");
	return MT_FAILURE;
    }

    MT_ASSERT(p_text_hdl != NULL);
    if (p_text_hdl->scroll_created == MT_TRUE) {
	MT_INFO_MTGO("scroll has been created before\n");
	return MT_SUCCESS;
    }

    if (pRect == NULL) {
	win_rect.x = 0;
	win_rect.y = 0;
	win_rect.w = p_surface_hdl->Width;
	win_rect.h = p_surface_hdl->Height;
    } else {
	win_rect.x = pRect->x;
	win_rect.y = pRect->y;
	win_rect.w = pRect->w;
	win_rect.h = pRect->h;
    }

    ret = decTextArrary(hTextOut, &(p_text_hdl->scroll.text_surface_arr), &(p_text_hdl->scroll.bmp_num), pText);
    if (ret != MT_SUCCESS) {
	MT_ERR_MTGO("dec text failed!\n");
	return ret;
    }

    p_text_hdl->scroll.rgb_surface_arr = (mt_handle *)malloc(p_text_hdl->scroll.bmp_num * sizeof(mt_handle));
    textGray2Rgb(p_text_hdl->text_attr.FgColor, p_text_hdl->scroll.scroll_speed,
                 p_text_hdl->scroll.text_surface_arr, p_text_hdl->scroll.bmp_num, p_text_hdl->scroll.rgb_surface_arr);

    mid_rect.x = 0;
    mid_rect.y = 0;
    mid_rect.w = win_rect.w;
    mid_rect.h = win_rect.h;

    ret = MT_GO_CreateSurface(mid_rect.w, mid_rect.h, p_surface_hdl->PixelFormat, &p_text_hdl->scroll.bg_surface);
    if (ret != MT_SUCCESS) {
	MT_ERR_MTGO("create bg surface failed!\n");
    }

    //store original text back ground to bg_surface
    MT_GO_Blit((mt_handle)p_surface_hdl, &win_rect, p_text_hdl->scroll.bg_surface, &mid_rect, NULL);

#ifdef __MT_GO_ERR__
    lld_text_create_scroll(p_text_hdl->scroll.rgb_surface_arr, p_text_hdl->scroll.bmp_num, hSurface, p_text_hdl->scroll.bg_surface,
                           &win_rect, p_text_hdl->scroll.scroll_times, p_text_hdl->scroll.scroll_speed, &p_text_hdl->scroll.scroll_hdl);
    p_text_hdl->scroll.scroll_win.x = win_rect.x;
    p_text_hdl->scroll.scroll_win.y = win_rect.y;
    p_text_hdl->scroll.scroll_win.w = win_rect.w;
    p_text_hdl->scroll.scroll_win.h = win_rect.h;
    p_text_hdl->scroll_created = MT_TRUE;
#endif

    for (i = 0; i < MTGO_MAX_TEXTSCROLL_NUM; i++) {
	if (mtTextMem.TextScrollUsed[i] == MT_FALSE) {
	    mtTextMem.TextScrollUsed[i] = MT_TRUE;
	    mtTextMem.TextScrollHdl[i] = hTextOut;
	    mtTextMem.TextDstSurface[i] = hSurface;
	    break;
	}
    }
    if (i == MTGO_MAX_TEXTSCROLL_NUM)
	MT_INFO_MTGO("\r\n Too many textScrolls!!!!!!!");
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_DestroyTextScroll(mt_handle hTextOut, mt_handle hSurface)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    mt_handle hTextOut_bak = hTextOut;
    mt_u32 i;

    if (p_text_hdl->scroll_created == MT_TRUE) {
#ifdef __MT_GO_ERR__
	lld_text_destroy_scroll(p_text_hdl->scroll.scroll_hdl);
#endif

	if (hSurface != 0) {
	    MT_GO_Blit(p_text_hdl->scroll.bg_surface, NULL, hSurface, &p_text_hdl->scroll.scroll_win, NULL);
	}

	MT_GO_FreeSurface(p_text_hdl->scroll.bg_surface);
	p_text_hdl->scroll.bg_surface = 0;

	for (i = 0; i < p_text_hdl->scroll.bmp_num; i++) {
	    MT_GO_FreeSurface(p_text_hdl->scroll.text_surface_arr[i]);
	    MT_GO_FreeSurface(p_text_hdl->scroll.rgb_surface_arr[i]);
	    p_text_hdl->scroll.text_surface_arr[i] = (mt_handle)NULL;
	    p_text_hdl->scroll.rgb_surface_arr[i] = (mt_handle)NULL;
	}
	p_text_hdl->scroll_created = MT_FALSE;

	for (i = 0; i < MTGO_MAX_TEXTSCROLL_NUM; i++) {
	    if (mtTextMem.TextScrollHdl[i] == hTextOut_bak) {
		if (mtTextMem.TextScrollUsed[i] == MT_FALSE)
			MT_INFO_MTGO("\r\n free textscroll which not be used before !!!!!");
		mtTextMem.TextScrollUsed[i] = MT_FALSE;
		mtTextMem.TextScrollHdl[i] = MT_NULL_PTR;
		mtTextMem.TextDstSurface[i] = MT_NULL_PTR;
		break;
	    }
	}
    }
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_CreateTextEx(const MTGO_TEXT_INFO_S *pInfo, mt_handle *phText)
{
#ifdef MTGO_TEXT_SUPPORT

    FT_Error error;
    mt_u32 file_size = 0;
    ft2_ctx_s *p_text = NULL;
    MTGO_TEXT_S *p_text_hdl = NULL;

    if (pInfo == NULL) {
	MT_ERR_MTGO("pInfo is null\n");
	return MT_FAILURE;
    }
    if (pInfo->pFontFile == NULL) {
	MT_ERR_MTGO("no font file\n");
	return MT_FAILURE;
    }
    p_text_hdl = (MTGO_TEXT_S *)malloc(sizeof(MTGO_TEXT_S));
    if (p_text_hdl == NULL) {
	return MT_FAILURE;
    }
    memset(p_text_hdl, 0, sizeof(MTGO_TEXT_S));
    p_text_hdl->ft2CtxtBase = (ft2_ctx_s *)malloc(sizeof(ft2_ctx_s));
    p_text = p_text_hdl->ft2CtxtBase;
    if (p_text == NULL) {
      free(p_text_hdl);
	return MT_FAILURE;
    }

    error = MT_GO_GetFileSize(pInfo->pFontFile, &file_size);
    if ((error != MT_SUCCESS) || (!file_size)) {
      free(p_text);
      free(p_text_hdl);   
	return MT_FAILURE;
    }

    p_text_hdl->pFontBuf = malloc(file_size);
    if (NULL == p_text_hdl->pFontBuf) {
      free(p_text);
      free(p_text_hdl);   
	return MT_FAILURE;
    }

    error = MT_GO_ReadFile(pInfo->pFontFile, p_text_hdl->pFontBuf, 0, file_size);
    if (error != MT_SUCCESS) {
      free(p_text_hdl->pFontBuf);
      free(p_text);
      free(p_text_hdl);            
	return MT_FAILURE;
    }

    error = FT_New_Memory_Face(library, (FT_Byte*)p_text_hdl->pFontBuf, (FT_Long)file_size, 0, &p_text->face);
    if (error == FT_Err_Unknown_File_Format) {
      free(p_text_hdl->pFontBuf);
	free(p_text);
  free(p_text_hdl);  
	MT_ERR_MTGO("Error : unknown file format! error:%d\n", error);
	return MT_FAILURE;
    } else if (error) {
    free(p_text_hdl->pFontBuf);
	free(p_text);
  free(p_text_hdl); 
	MT_ERR_MTGO("Error : FT_New_Face() failed! error:%d\n", error);
	return MT_FAILURE;
    }
    error = FT_Set_Pixel_Sizes(p_text->face, pInfo->u32Size, 0);
    if (error) {
	MT_ERR_MTGO("Error : FT_Set_Pixel_Sizes() failed! \n");
    }
    p_text_hdl->text_attr.FontAttr.Height = (mt_u8)(pInfo->u32Size);
    p_text_hdl->text_attr.FontAttr.MaxWidth = (mt_u8)(pInfo->u32Size);
    *phText = (mt_handle)(p_text_hdl);
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_SetTextStyle(mt_handle hTextOut, MTGO_TEXT_STYLE_E eStyle)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    p_text_hdl->style = eStyle;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_GetCharExtra(mt_handle hTextOut, mt_u32 *pDistance)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    *pDistance = p_text_hdl->char_distance;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_SetCharExtra(mt_handle hTextOut, mt_u32 u32Distance)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    p_text_hdl->char_distance = u32Distance;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_GetLineExtra(mt_handle hTextOut, mt_u32 *pDistance)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    *pDistance = p_text_hdl->line_distance;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_SetLineExtra(mt_handle hTextOut, mt_u32 u32Distance)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)hTextOut;
    p_text_hdl->line_distance = u32Distance;
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_GetTextExtent(mt_handle TextOut, const mt_char *pText, mt_s32 *pWidth, mt_s32 *pHeight)
{
#ifdef MTGO_TEXT_SUPPORT

    MTGO_TEXT_S *p_text_hdl = (MTGO_TEXT_S *)TextOut;
    FT_BBox bbox;
    mt_u32 num_glyphs;
    int line_width;
    int line_height;
    TGlyph *glyph;
    mt_u32 ascender;
    mt_u32 descender;
    wchar_t *codeUnic;
    mt_u32 code_num;
    FT_Face face;

    if (p_text_hdl == NULL) {
	MT_ERR_MTGO("TextOut is null!\n");
	return MT_FAILURE;
    }
    if (pText == NULL) {
	MT_ERR_MTGO("pText is null!\n");
	return MT_FAILURE;
    }
    //convert char to unicode
    codeUnic = (wchar_t *)malloc(sizeof(wchar_t) * strlen(pText));
    Char_to_Unicode((mt_u8 *)pText, codeUnic, &code_num);
    glyph = malloc(code_num * sizeof(TGlyph));

    //get glyphs
    face = p_text_hdl->ft2CtxtBase->face;
    num_glyphs = (mt_u32)Get_Glyphs_Frm_Wstr(face, codeUnic, code_num, glyph, p_text_hdl->char_distance);

    //calculate string width and height
    FT_Glyph_Get_CBox(glyph[num_glyphs - 1].image, FT_GLYPH_BBOX_TRUNCATE, &bbox);
    line_width = glyph[num_glyphs - 1].pos.x + bbox.xMax;
    ascender = (mt_u32)(face->size->metrics.ascender) >> 6;
    descender = (mt_u32)(face->size->metrics.descender) >> 6;
    line_height = ((int)ascender - (int)descender + ((int)(face->size->metrics.height) >> 6)) / 2;
	if (((int)ascender - (int)descender + ((int)(face->size->metrics.height) >> 6)) % 2 != 0)
		line_height += 1;

    *pHeight = line_height;
    *pWidth = line_width;
    free(glyph);
    free(codeUnic);
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}

mt_s32 MT_GO_DestroyAllTextScrolls(void)
{
#ifdef MTGO_TEXT_SUPPORT

    mt_u32 i;

    for (i = 0; i < MTGO_MAX_TEXTSCROLL_NUM; i++) {
	if (mtTextMem.TextScrollUsed[i] == MT_TRUE) {
	    MT_GO_DestroyTextScroll(mtTextMem.TextScrollHdl[i], mtTextMem.TextDstSurface[i]);
	}
    }
    return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif		
}


#ifdef MTGO_TEXT_SUPPORT
#if 0
static mt_void MT_GO_Dump_Surface(MTGO_SURFACE_S *p_surface)
{
#ifdef MTGO_TEXT_SUPPORT

   //MTGO_SURFACE_S *p_surface =  NULL;
   mt_void  *p_tempBuf = NULL;
   mt_u32 fd = 0;
   ulong buf_addr = 0;
   mt_u32 pitch = 0;
   mt_u32 w = 0;
   mt_u32 h = 0;
   mt_u32 fmt = 0;
   mt_u32 bpp = 0;
   mt_u32 i = 0;
   mt_char fileName[100];

   //p_surface = (MTGO_SURFACE_S *)p_surfaceHandle;
   MT_ASSERT(p_surface != NULL);

   buf_addr = (ulong)p_surface->Data[0].pData;
   pitch= p_surface->Data[0].Pitch;
   w = (mt_u32)(p_surface->Width);
   h = (mt_u32)(p_surface->Height);
   fmt = p_surface->PixelFormat;
   bpp = p_surface->Data[0].Bpp;
   
    sprintf(fileName, "surf_%lx_%d_%d_%d_%d_%d.bin", buf_addr, pitch, bpp, fmt, w, h);

    fd = (mt_u32)open(fileName, O_WRONLY | O_CREAT, 0666);
    if(fd <= 0)
    {
      MT_ERR_MTGO("err [%d][%s]\n", fd, fileName);
      return;
    }

    MT_INFO_MTGO("[%d][%s]\n",fd, fileName);

    for(i = 0; i < h; i++)
    {
        p_tempBuf = (mt_void *)(buf_addr + i * pitch);
         write((int)fd, p_tempBuf, pitch);
    }

    close((int)fd);

    MT_INFO_MTGO("end\n");

    return;
#else
	return MT_FAILURE;
#endif		
}
#endif

static mt_void MT_GO_Dump_Surface_Font(MTGO_SURFACE_S *p_surface)
{
#ifdef MTGO_TEXT_SUPPORT

   //MTGO_SURFACE_S *p_surface =  NULL;
   mt_u8  *p_tempBuf = NULL;
   ulong buf_addr = 0;
//   mt_u32 phyAddr = 0;
   mt_u32 pitch = 0;
   mt_u32 w = 0;
   mt_u32 h = 0;
//   mt_u32 fmt = 0;
   mt_u32 bpp = 0;
   mt_u32 i = 0;
   mt_u32 j = 0;
//   mt_char fileName[100];

   //p_surface = (MTGO_SURFACE_S *)p_surfaceHandle;
   MT_ASSERT(p_surface != NULL);

   buf_addr = (ulong)p_surface->Data[0].pData;
//   phyAddr = (mt_u32)p_surface->Data[0].pPhyData;
   pitch= p_surface->Data[0].Pitch;
   w = (mt_u32)p_surface->Width;
   h = (mt_u32)p_surface->Height;
//   fmt = p_surface->PixelFormat;
   bpp = p_surface->Data[0].Bpp;
   
//   sprintf(fileName, "surf_%x_%x_%d_%d_%d_%d_%d.bin", buf_addr, phyAddr, pitch, bpp, fmt, w, h);

//   printf("%s: [%s]\n", __FUNCTION__, fileName);

   return;
   
    for(i = 0; i < h; i++)
    {
        for(j = 0; j <= w; j++)
        {
             p_tempBuf = (mt_void *)(buf_addr + i * pitch + j * bpp);
             if(*p_tempBuf >= 10)
              {
                 printf("*");
              }
             else
              {
                 printf(" ");
              }
        }
       printf("\n");
    }
    
    printf("%s: end\n", __FUNCTION__);

    return;
#else
	return MT_FAILURE;
#endif		
}


static mt_void MT_GO_Dump_Surface_Font_ARGB8888(mt_handle MasksurfaceHandle, mt_handle *pArgb888SurfaceHandle)
{
#ifdef MTGO_TEXT_SUPPORT

   MTGO_SURFACE_S *p_surface =  NULL;
   mt_handle DstSurfaceHandle =  0;
   MTGO_SURFACE_S *p_dstSurface =  NULL;
   
   mt_u8  *p_tempBuf = NULL;
   ulong buf_addr = 0;
//   mt_u32 phyAddr = 0;
   mt_u32 pitch = 0;
   mt_u32 w = 0;
   mt_u32 h = 0;
//   mt_u32 fmt = 0;
   mt_u32 bpp = 0;
   mt_u32 i = 0;
   mt_u32 j = 0;
//   mt_char fileName[100];
   mt_s32  ret = 0;

   ulong dst_buf_addr = 0;
   mt_u32  *p_tempDstBuf = NULL;
   mt_u32 dst_bpp = 0;
   mt_u32 dst_pitch = 0;
   
   p_surface = (MTGO_SURFACE_S *)MasksurfaceHandle;
   MT_ASSERT(p_surface != NULL);

   buf_addr = (ulong)p_surface->Data[0].pData;
//   phyAddr = (mt_u32)p_surface->Data[0].pPhyData;
   pitch= p_surface->Data[0].Pitch;
   w = (mt_u32)p_surface->Width;
   h = (mt_u32)p_surface->Height;
//   fmt = p_surface->PixelFormat;
   bpp = p_surface->Data[0].Bpp;
   
//   sprintf(fileName, "surf_%x_%x_%d_%d_%d_%d_%d.bin", buf_addr, phyAddr, pitch, bpp, fmt, w, h);

//   printf("%s: [%s]\n", __FUNCTION__, fileName);

    ret = MT_GO_CreateSurface((mt_s32)w, (mt_s32)h, MTGO_PF_8888, &DstSurfaceHandle);
    if (ret != MT_SUCCESS)
    {
	   MT_ERR_MTGO("create text surface failed!\n");
	   return ;
    }

   p_dstSurface = (MTGO_SURFACE_S *)DstSurfaceHandle;
   dst_buf_addr = (ulong)p_dstSurface->Data[0].pData;
   dst_pitch= p_dstSurface->Data[0].Pitch;
   dst_bpp = p_dstSurface->Data[0].Bpp;

   *pArgb888SurfaceHandle = DstSurfaceHandle;
   
    for(i = 0; i < h; i++)
    {
        for(j = 0; j < w; j++)
        {
             p_tempBuf = (mt_void *)(buf_addr + i * pitch + j * bpp);
             p_tempDstBuf = (mt_void *)(dst_buf_addr + i * dst_pitch + j * dst_bpp);
             if(*p_tempBuf >= 10)
              {
                 *p_tempDstBuf = 0xffff0000;
              }
             else
              {
                 *p_tempDstBuf = 0xffffff00;
              }
        }
    }
    
//    printf("%s: end\n", __FUNCTION__);

    return;
#else
	return ;
#endif		
}
#endif

