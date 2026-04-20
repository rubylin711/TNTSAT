#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mt_type.h"
#include "subtitle_debug.h"
#include "scte_subt_data.h"
#include "scte_subt_parse.h"
#include "scte_subt_display.h"

#define SUBT_COLOR_RGB888_SIZE (32)
#define SCTE_SUBT_BPP 4
#define PTS_UNITS 90   /*see Ref.P15*/
#define PTS_OFFSET (0x80000000/45) /*(0x100000000/PTS_UNITS)*//*see Ref.P15*/

#define CheckIndex(index) if(index >= 14) {\
        MT_ERR_SUBT("Index too large\n"); \
        return MT_FAILURE; \
    }

typedef struct tagSCTE_SUBT_PARSE_INFO_S
{
    MT_HANDLE          hDisplay;
    MT_UNF_SUBT_GETPTS_FN pfnGetPts;
    ulong u32UserData;
    SCTE_SUBT_OUTPUT_S stOutputData;
} SCTE_SUBT_PARSE_INFO_S;

typedef struct
{
    mt_u8  *p_start;
    mt_u8  *p_p;
    mt_u8  *p_end;
    mt_s32 left;
} bs_sub_t;

/*!
  subtitle bitstream init
  */
static inline void bs_sub_init(bs_sub_t *p_s, void *p_data, mt_u32 size)
{
    p_s->p_start = p_data;
    p_s->p_p     = p_data;
    p_s->p_end   = p_s->p_p + size;
    p_s->left    = 8;
}

/*!
  subtitle bitstream end flag
  */
static inline MT_BOOL bs_sub_eof(bs_sub_t *p_s)
{
    return p_s->p_p > p_s->p_end ? MT_TRUE : MT_FALSE;
}

/*!
  subtitle bitstream read
  */
static mt_u32 bs_sub_read(bs_sub_t *p_s, mt_s32 count)
{
    static const mt_u32 mask[33] =
    {
        0x00,
        0x01, 0x03, 0x07, 0x0f,
        0x1f, 0x3f, 0x7f, 0xff,
        0x1ff, 0x3ff, 0x7ff, 0xfff,
        0x1fff, 0x3fff, 0x7fff, 0xffff,
        0x1ffff, 0x3ffff, 0x7ffff, 0xfffff,
        0x1fffff, 0x3fffff, 0x7fffff, 0xffffff,
        0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
        0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
    };
    mt_s32 shr    = 0;
    mt_u32 result = 0;

    while(count > 0)
    {
        if(p_s->p_p > p_s->p_end)
        {
            MT_ERR_SUBT("SUBT!:bs read error \n");  
            break;
        }

        if((shr = p_s->left - count) >= 0)
        {
            /* more in the buffer than requested */
            result  |= ((mt_u32)(*p_s->p_p >> shr)) & mask[count];
            p_s->left -= count;
            if(p_s->left == 0)
            {
                p_s->p_p ++;
                p_s->left = 8;
            }
            return result;
        }
        else
        {
            /* less in the buffer than requested */
            result |= (*p_s->p_p & mask[p_s->left]) << (- shr);
            count -= p_s->left;
            p_s->p_p ++;
            p_s->left = 8;
        }
    }

    return result;
}

/*!
  subtitle bitstream skip
  */
static void bs_sub_skip(bs_sub_t *p_s, int i_count)
{
    p_s->left -= i_count;

    while(p_s->left <= 0)
    {
        p_s->p_p ++;
        p_s->left += 8;
    }
}

/*!
  subtitle bitstream align
  */
static inline void bs_sub_align(bs_sub_t *p_s)
{
    if(p_s->left != 8)
    {
        p_s->left = 8;
        p_s->p_p ++;
    }
}

#define CLIP(x) ((x < 0) ? (0) : ((x > 255) ? 255 : x))

static mt_u32 YUV2RGB(mt_u8  *pu8Data)
{
    mt_u8 Y   = 0, Cr = 0, Cb = 0, Alpha = 0;
    mt_u32 Clr=0;
    int r = 0, g = 0, b = 0;

    Y  = (mt_u8)(((pu8Data[0] >> 3) & 0x1f) << 3);
    Cr = (mt_u8)(((pu8Data[0] << 3 | pu8Data[1] >> 5) & 0x1f) << 3);
    Cb = (mt_u8)((pu8Data[1] & 0x1f) << 3);
    Alpha = (pu8Data[0] >> 2) & 0x01;

    r = (((298 * Y >> 4) + (0 * Cb >> 4) + (459 * Cr >> 4) +  ((-63522) >> 4)))>> 4;
    g = (((298 * Y >> 4) + ((-55) * Cb >> 4) + ((-136) * Cr >> 4) +  (19659 >> 4)))>> 4;
    b = (((298 * Y >> 4) + (541 * Cb >> 4) + (0 * Cr >> 4) +  ((-74002) >> 4)))>> 4; 
    r = CLIP(r);
    g = CLIP(g);
    b = CLIP(b);

    Clr = (mt_u32)(((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff));
    if (Alpha == 1)
    {
        Clr |= 0xFF000000;   //opacity
    }
    else
    {
        Clr |= 0x7F000000;   //half transparent
    }
    return Clr;
}

#if 0
static mt_void FillSubtClr(mt_u32 u32LineNumber, mt_u32 u32RowNumber, mt_u32 u32Width, mt_u8 u8PixOn, SCTE_SUBT_PARSE_INFO_S *pstParseInfo )
{
    mt_u32 i=0, u32CLr = 0;

    u32CLr = pstParseInfo->stOutputData.u32SubtColor;
    for (i = SCTE_SUBT_BPP * (u32LineNumber * u32Width + u32RowNumber);
         i < SCTE_SUBT_BPP * (u32LineNumber * u32Width + u32RowNumber + u8PixOn); i++)
    {
        if (i >= pstParseInfo->stOutputData.u32BitmapDataLen)
        {
            MT_INFO_SUBT("Subtitle data too long...\n");
            break;
        }

        if (0 == i % SCTE_SUBT_BPP)
        {
            pstParseInfo->stOutputData.pu8SCTESubtData[i] = (mt_u8)(u32CLr>> 24);
        }
        else if (1 == i % SCTE_SUBT_BPP)
        {
            pstParseInfo->stOutputData.pu8SCTESubtData[i] = (mt_u8)(u32CLr>> 16);
        }
        else if (2 == i % SCTE_SUBT_BPP)
        {
            pstParseInfo->stOutputData.pu8SCTESubtData[i] = (mt_u8)(u32CLr>> 8);
        }
        else
        {
            pstParseInfo->stOutputData.pu8SCTESubtData[i] = (mt_u8)u32CLr;
        }
     }
   return;
}

static MT_BOOL GetNextUChar(mt_u8 * pu8Data, mt_u16 * u16CurData, mt_u32 *u32Index, mt_u8 *u8Curlen, mt_u32 u32DataLen)
{
    (*u32Index)++;
    if (*u32Index == u32DataLen)
    {
        return MT_TRUE;
    }

    (*u8Curlen)    += 8;
    (*u16CurData) <<= 8;
    (*u16CurData)  += pu8Data[*u32Index];
    return MT_FALSE;
}

static mt_s32 DecompressBitmap(SCTE_SUBT_PARSE_INFO_S *pstParseInfo, mt_u8 *pu8DataSrc, mt_u32 u32DataLen)
{
    mt_s32 s32Ret   = MT_SUCCESS;
    mt_u8 * pu8Data = pu8DataSrc;

    mt_u8 u8PixOn     = 0, u8PixOff = 0, u8Curlen = 8;
    mt_u16 u16Temp    = 0, u16Result = 0, u16CurData = 0;
    mt_u32 u32Index   = 0;
    mt_u16 pow[] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191};

    mt_u32 u32LineNumber = 0, u32RowNumber = 0, u32Width = 0;

    if ((MT_NULL == pstParseInfo) || (MT_NULL == pu8DataSrc) || (0 == u32DataLen))
    {
        MT_ERR_SUBT("param invalid...\n");
        return MT_FAILURE;
    }

    u32Width   = pstParseInfo->stOutputData.u32ButtomXPos - pstParseInfo->stOutputData.u32TopXPos + 1;
    u16CurData = (mt_u8)(*pu8Data);

    if (SCTE_SUBT_BACKGROUD_FRAMED == pstParseInfo->stOutputData.enBackgroundStyle)
    {
        u32LineNumber = pstParseInfo->stOutputData.u32TopYPos - pstParseInfo->stOutputData.stFramed.u32TopYPos;
        u32RowNumber = pstParseInfo->stOutputData.u32TopXPos - pstParseInfo->stOutputData.stFramed.u32TopXPos;
        u32Width = pstParseInfo->stOutputData.stFramed.u32ButtomXPos - pstParseInfo->stOutputData.stFramed.u32TopXPos;
    }

    pstParseInfo->stOutputData.u32BitWidth = SUBT_COLOR_RGB888_SIZE;

    while (u32Index < u32DataLen)
    {
        /*首先就是要可以判断出四种情况，根据一个token的前三位来进行分类，000/001/01X/1XX*/
        if (u8Curlen < 3)
        {
            if (MT_TRUE == GetNextUChar(pu8Data, &u16CurData, &u32Index, &u8Curlen, u32DataLen))
            {
                break;
            }
        }

        u16Temp   = 7;
        u16Temp <<= (u8Curlen - 3);
        u16Temp &= u16CurData;
        u16Temp >>= (u8Curlen - 3);
        switch (u16Temp)
        {
        case 0:         //      token size is 5
            if (u8Curlen < 5)
            {
                if (MT_TRUE == GetNextUChar(pu8Data, &u16CurData, &u32Index, &u8Curlen, u32DataLen))
                {
                    break;
                }
            }

            u8Curlen -= 5;
            u16Result = u16CurData >> (u8Curlen);
            switch (u16Result)
            {
            case 0:
                break;
            case 1:                //00001
                u32LineNumber++;
                if (u32RowNumber > u32Width)
                {
                    MT_ERR_SUBT("Write subt data error!\n");
                }

                u32RowNumber = (pstParseInfo->stOutputData.enBackgroundStyle
                                == SCTE_SUBT_BACKGROUD_FRAMED) ? (pstParseInfo->stOutputData.u32TopXPos
                                                                  - pstParseInfo->stOutputData.stFramed.u32TopXPos) : 0;
                break;
            default:
                break;
            }
            CheckIndex(u8Curlen);
            u16CurData &= pow[u8Curlen];
            break;

        case 1:         //      token size is 7(001XXXX)
            if (u8Curlen < 7)
            {
                if (MT_TRUE == GetNextUChar(pu8Data, &u16CurData, &u32Index, &u8Curlen, u32DataLen))
                {
                    break;
                }
            }

            u8Curlen -= 7;
            u16Result = u16CurData >> (u8Curlen);
            u8PixOn = u16Result & 15;
            if (0 == u8PixOn)
            {
                u8PixOn = 16;
            }

            CheckIndex(u8Curlen);
            u16CurData &= pow[u8Curlen];
            FillSubtClr(u32LineNumber, u32RowNumber, u32Width, u8PixOn, pstParseInfo );
            u32RowNumber += (mt_u32) u8PixOn;
            break;
        case 2:
        case 3:         //      token size is 8(01XXXXXX)
            if (u8Curlen < 8)
            {
                if (MT_TRUE == GetNextUChar(pu8Data, &u16CurData, &u32Index, &u8Curlen, u32DataLen))
                {
                    break;
                }
            }

            u8Curlen -= 8;
            u16Result = u16CurData >> (u8Curlen);
            u8PixOff = u16Result & 63;
            if (0 == u8PixOff)
            {
                u8PixOff = 64;
            }

            CheckIndex(u8Curlen);
            u16CurData   &= pow[u8Curlen];
            u32RowNumber += (mt_u32) u8PixOff;
            break;
        default:        //      token size is 9(1XXXYYYYY )
            if (u8Curlen < 9)
            {
                if (MT_TRUE == GetNextUChar(pu8Data, &u16CurData, &u32Index, &u8Curlen, u32DataLen))
                {
                    break;
                }
            }

            u8Curlen -= 9;
            u16Result = u16CurData >> (u8Curlen);
            u8PixOff = (mt_u8) (u16Result & 31);
            u8PixOn = (mt_u8)((u16Result & 224) >> 5);

            if (0 == u8PixOff)
            {
                u8PixOff = 32;
            }
            if (0 == u8PixOn)
            {
                u8PixOn = 8;
            }

            CheckIndex(u8Curlen);
            u16CurData &= pow[u8Curlen];
            FillSubtClr(u32LineNumber, u32RowNumber, u32Width, u8PixOn, pstParseInfo );
            u32RowNumber += (mt_u32) u8PixOn;
            u32RowNumber += (mt_u32) u8PixOff;
            break;
        }
    }

    return s32Ret;
}

static mt_s32 ParseBitmapHead(SCTE_SUBT_PARSE_INFO_S *pstParseInfo, mt_u8 *pu8DataSrc, mt_u32 u32DataSize)
{
    mt_s32 s32Ret  = MT_SUCCESS;
    mt_u8 *pu8Data = pu8DataSrc;
    mt_u16 u16BitmapHeadLen = 0, u16BitmapLen   = 0;
    mt_u32 u32SubtDataLen = 0, u32SubtArea = 0, i  = 0;

    if((MT_NULL == pstParseInfo) || (MT_NULL == pu8DataSrc) || (0 == u32DataSize))
    {
        MT_ERR_SUBT("param invalid...\n");
        return MT_FAILURE;
    }

    pstParseInfo->stOutputData.enBackgroundStyle = SCTE_SUBT_BACKGROUD_TRANSPARENT;
    pstParseInfo->stOutputData.enOutlineStyle = SCTE_SUBT_OUTLINE_RESERVED;

    if (pu8Data[0] & 0x04) //framed
    {
        pstParseInfo->stOutputData.enBackgroundStyle = SCTE_SUBT_BACKGROUD_FRAMED;
    }

    if (pu8Data[0] & 0x01) //outlined
    {
        pstParseInfo->stOutputData.enOutlineStyle = SCTE_SUBT_OUTLINE_OUTLINED;
    }
    else if(pu8Data[0] & 0x02) //dropshadow
    {
        pstParseInfo->stOutputData.enOutlineStyle = SCTE_SUBT_OUTLINE_DROPSHADOW;
    }

    pu8Data += 1;
    u16BitmapHeadLen += 1;
    pstParseInfo->stOutputData.u32SubtColor = YUV2RGB(pu8Data); //subtitle color

    pu8Data += 2;
    u16BitmapHeadLen += 2;
    pstParseInfo->stOutputData.u32TopXPos = ((pu8Data[0] << 8) | pu8Data[1]) >> 4; //BTH
    pstParseInfo->stOutputData.u32TopYPos = ((pu8Data[1] << 8) | pu8Data[2]) & 0x0fff; //BTV
    pstParseInfo->stOutputData.u32ButtomXPos = ((pu8Data[3] << 8) | pu8Data[4]) >> 4; //BBH
    pstParseInfo->stOutputData.u32ButtomYPos = ((pu8Data[4] << 8) | pu8Data[5]) & 0x0fff; //BBV

    pu8Data += 6;
    u16BitmapHeadLen += 6;

    /*before malloc pu8SCTESubtData,free it*/
    if(pstParseInfo->stOutputData.pu8SCTESubtData)
    {
        free((void*)pstParseInfo->stOutputData.pu8SCTESubtData);
        pstParseInfo->stOutputData.pu8SCTESubtData = NULL;
    }

    if (pstParseInfo->stOutputData.enBackgroundStyle == SCTE_SUBT_BACKGROUD_FRAMED)
    {
        pstParseInfo->stOutputData.stFramed.u32TopXPos = ((pu8Data[0] << 8) | pu8Data[1]) >> 4; //FTH
        pstParseInfo->stOutputData.stFramed.u32TopYPos = ((pu8Data[1] << 8) | pu8Data[2]) & 0x0fff; //FTV
        pstParseInfo->stOutputData.stFramed.u32ButtomXPos = ((pu8Data[3] << 8) | pu8Data[4]) >> 4; //FBH
        pstParseInfo->stOutputData.stFramed.u32ButtomYPos = ((pu8Data[4] << 8) | pu8Data[5]) & 0x0fff; //FBV
        pu8Data += 6;
        u16BitmapHeadLen += 6;
        pstParseInfo->stOutputData.stFramed.u32FrameColor = YUV2RGB(pu8Data);

        u32SubtArea = (pstParseInfo->stOutputData.stFramed.u32ButtomXPos
                       - pstParseInfo->stOutputData.stFramed.u32TopXPos + 1)
                      * (pstParseInfo->stOutputData.stFramed.u32ButtomYPos
                         - pstParseInfo->stOutputData.stFramed.u32TopYPos + 1);
        u32SubtDataLen = SCTE_SUBT_BPP * u32SubtArea;

        pstParseInfo->stOutputData.u32BitmapDataLen = u32SubtDataLen;
        pstParseInfo->stOutputData.pu8SCTESubtData = (mt_u8 *)malloc(u32SubtDataLen);
        if (MT_NULL == pstParseInfo->stOutputData.pu8SCTESubtData)
        {
            MT_ERR_SUBT("malloc SCTESubtData datastructure error...\n");
            return MT_FAILURE;
        }

        for (i = 0; i < u32SubtDataLen; i++)
        {
            if (0 == i % SCTE_SUBT_BPP)
            {
                pstParseInfo->stOutputData.pu8SCTESubtData[i] =
                    (mt_u8)(pstParseInfo->stOutputData.stFramed.u32FrameColor >> 24);
            }
            else if (1 == i % SCTE_SUBT_BPP)
            {
                pstParseInfo->stOutputData.pu8SCTESubtData[i] =
                    (mt_u8)(pstParseInfo->stOutputData.stFramed.u32FrameColor >> 16);
            }
            else if (2 == i % SCTE_SUBT_BPP)
            {
                pstParseInfo->stOutputData.pu8SCTESubtData[i] =
                    (mt_u8)(pstParseInfo->stOutputData.stFramed.u32FrameColor >> 8);
            }
            else
            {
                pstParseInfo->stOutputData.pu8SCTESubtData[i] =
                    (mt_u8)pstParseInfo->stOutputData.stFramed.u32FrameColor;
            }
        }

        /*end*/
        pu8Data += 2;
        u16BitmapHeadLen += 2;
    }
    else
    {
         /*Allocate the memery of subtData*/
        u32SubtArea = (pstParseInfo->stOutputData.u32ButtomXPos
                        - pstParseInfo->stOutputData.u32TopXPos + 1)
                      * (pstParseInfo->stOutputData.u32ButtomYPos
                        - pstParseInfo->stOutputData.u32TopYPos + 1);

        u32SubtDataLen = SCTE_SUBT_BPP * u32SubtArea;
        pstParseInfo->stOutputData.pu8SCTESubtData = (mt_u8 *)malloc(u32SubtDataLen);
        if (MT_NULL == pstParseInfo->stOutputData.pu8SCTESubtData)
        {
            MT_ERR_SUBT("malloc SCTESubtData datastructure error...\n");
            return MT_FAILURE;
        }

        pstParseInfo->stOutputData.u32BitmapDataLen = u32SubtDataLen;
        memset(pstParseInfo->stOutputData.pu8SCTESubtData, 0, u32SubtDataLen);
        /*end*/
    }

    if (pstParseInfo->stOutputData.enOutlineStyle == SCTE_SUBT_OUTLINE_OUTLINED)
    {
        pstParseInfo->stOutputData.unOutlineStyle.stOutline.u16OutlineThickness = pu8Data[0] & 0x0f;
        pu8Data += 1;
        u16BitmapHeadLen += 1;
        pstParseInfo->stOutputData.unOutlineStyle.stOutline.u32OutlineColor = YUV2RGB(pu8Data);

        pu8Data += 2;
        u16BitmapHeadLen += 2;
    }
    else if (pstParseInfo->stOutputData.enOutlineStyle == SCTE_SUBT_OUTLINE_DROPSHADOW)
    {
        pstParseInfo->stOutputData.unOutlineStyle.stDropshadow.u32ShadowRight  = (pu8Data[0] & 0xf0) >> 4;
        pstParseInfo->stOutputData.unOutlineStyle.stDropshadow.u32ShadowBottom = pu8Data[0] & 0x0f;
        pu8Data += 1;
        u16BitmapHeadLen += 1;
        pstParseInfo->stOutputData.unOutlineStyle.stDropshadow.u32ShadowColor = YUV2RGB( pu8Data);

        pu8Data += 2;
        u16BitmapHeadLen += 2;
    }
    else if (pstParseInfo->stOutputData.enOutlineStyle == SCTE_SUBT_OUTLINE_RESERVED)
    {
        pu8Data += 3;
        u16BitmapHeadLen += 3;
    }

    u16BitmapLen = pu8Data[0] << 8 | pu8Data[1];
    pu8Data += 2;
    u16BitmapHeadLen += 2;
    if ((pu8Data != (&pu8DataSrc[u16BitmapHeadLen])))
    {
        MT_ERR_SUBT("\nerror, pu8Data addr:%p, 0x%02x, &pDataSrc[u8PesHeadLength] addr:%p, 0x%02x\n",
                 pu8Data, pu8Data[0], &pu8DataSrc[u16BitmapHeadLen], pu8DataSrc[u16BitmapHeadLen]);

        return MT_FAILURE;
    }

    s32Ret = DecompressBitmap( pstParseInfo, &pu8DataSrc[u16BitmapHeadLen], (mt_u32)u16BitmapLen);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_SUBT("failed to DecompressBitmap  ...\n");
        return MT_FAILURE;
    }

    MT_INFO_SUBT(" SCTE_SUBT_Parse_ParseSection success  ...\n");

    return s32Ret;
}
#endif

static void SetPixel(mt_u8 *picture, mt_u32 pitch, mt_u32 x, mt_u32 y, mt_u32 value)
{
    picture[y * pitch + 4*x+3] = (mt_u8)value;
    picture[y * pitch + 4*x+2] = (mt_u8)(value>> 8);
    picture[y * pitch + 4*x+1] = (mt_u8)(value>> 16);
    picture[y * pitch + 4*x+0] = (mt_u8)(value>> 24);

}

#ifndef __MIN
#   define __MIN(a, b)   ( ((a) < (b)) ? (a) : (b) )
#endif

static mt_s32 DecodeSimpleBitmap(SCTE_SUBT_PARSE_INFO_S *pstParseInfo, mt_u8 *pu8DataSrc, mt_u32 u32DataSize)
{
    mt_u32 position, by, bx, is_framed, outline_style, top_h, top_v, bottom_h, bottom_v;
    mt_u32 frame_top_h, frame_top_v, frame_bottom_h, frame_bottom_v;
    mt_u8 colorData[2];
    mt_u32 dx, dy;
    mt_u32 outline_thickness, shadow_right, shadow_bottom, bitmap_h, bitmap_v;
    mt_u32 bitmap_size;
    mt_u8* bitmap	= NULL;
    mt_u32 run_on_length, run_off_length, next, run, margin_h, margin_v;	
    mt_u32 frame_h, frame_v, bitmap_oh, bitmap_ov;
    MT_BOOL circle[16][16];
    bs_sub_t bs;

    if((MT_NULL == pstParseInfo) || (MT_NULL == pu8DataSrc) || (0 == u32DataSize))
    {
        MT_ERR_SUBT("param invalid...\n");
        return MT_FAILURE;
    }

    pstParseInfo->stOutputData.u32BitWidth = SUBT_COLOR_RGB888_SIZE;
    /* Parse the bitmap and its properties */
    bs_sub_init(&bs, pu8DataSrc, u32DataSize);

    bs_sub_skip(&bs, 5);
    pstParseInfo->stOutputData.enBackgroundStyle = SCTE_SUBT_BACKGROUD_TRANSPARENT;
    pstParseInfo->stOutputData.enOutlineStyle = SCTE_SUBT_OUTLINE_RESERVED;

    is_framed = bs_sub_read(&bs, 1);
    outline_style = bs_sub_read(&bs, 2);

    if (is_framed) //framed
    {
        pstParseInfo->stOutputData.enBackgroundStyle = SCTE_SUBT_BACKGROUD_FRAMED;
    }

    if (outline_style & 0x01) //outlined
    {
        pstParseInfo->stOutputData.enOutlineStyle = SCTE_SUBT_OUTLINE_OUTLINED;
    }
    else if(outline_style & 0x02) //dropshadow
    {
        pstParseInfo->stOutputData.enOutlineStyle = SCTE_SUBT_OUTLINE_DROPSHADOW;
    }

    colorData[0] = (mt_u8)bs_sub_read(&bs, 8);
    colorData[1] = (mt_u8)bs_sub_read(&bs, 8);	
    pstParseInfo->stOutputData.u32SubtColor = YUV2RGB((mt_u8*)&colorData);
    top_h = bs_sub_read(&bs, 12);
    top_v = bs_sub_read(&bs, 12);
    bottom_h = bs_sub_read(&bs, 12);
    bottom_v = bs_sub_read(&bs, 12);
    pstParseInfo->stOutputData.u32TopXPos = top_h; //BTH
    pstParseInfo->stOutputData.u32TopYPos = top_v; //BTV
    pstParseInfo->stOutputData.u32ButtomXPos = bottom_h; //BBH
    pstParseInfo->stOutputData.u32ButtomYPos = bottom_v; //BBV

    if (top_h >= bottom_h || top_v >= bottom_v)
    {
        MT_ERR_SUBT("pos error\n");
        return MT_FAILURE;
    }
    frame_top_h = top_h;
    frame_top_v = top_v;
    frame_bottom_h = bottom_h;
    frame_bottom_v = bottom_v;
    if (is_framed)
    {
        frame_top_h = bs_sub_read(&bs, 12);
        frame_top_v = bs_sub_read(&bs, 12);
        frame_bottom_h = bs_sub_read(&bs, 12);
        frame_bottom_v = bs_sub_read(&bs, 12);

        pstParseInfo->stOutputData.stFramed.u32TopXPos = frame_top_h; //FTH
        pstParseInfo->stOutputData.stFramed.u32TopYPos = frame_top_v; //FTV
        pstParseInfo->stOutputData.stFramed.u32ButtomXPos = frame_bottom_h; //FBH
        pstParseInfo->stOutputData.stFramed.u32ButtomYPos = frame_bottom_v; //FBV
        colorData[0] = (mt_u8)bs_sub_read(&bs, 8);
        colorData[1] = (mt_u8)bs_sub_read(&bs, 8);	
        pstParseInfo->stOutputData.stFramed.u32FrameColor = YUV2RGB((mt_u8*)&colorData);
        if (frame_top_h > top_h ||
            frame_top_v > top_v ||
            frame_bottom_h < bottom_h ||
            frame_bottom_v < bottom_v)
        {
            MT_ERR_SUBT("frame pos error\n");
            return MT_FAILURE;
        }
    }
    outline_thickness = 0;
    shadow_right = 0;
    shadow_bottom = 0;
    if (outline_style == 1)
    {
        bs_sub_skip(&bs, 4);
        outline_thickness = bs_sub_read(&bs, 4);
        colorData[0] = (mt_u8)bs_sub_read(&bs, 8);
        colorData[1] = (mt_u8)bs_sub_read(&bs, 8);	
        pstParseInfo->stOutputData.unOutlineStyle.stOutline.u32OutlineColor = YUV2RGB((mt_u8*)&colorData);
    }
    else if (outline_style == 2)
    {
        shadow_right = bs_sub_read(&bs, 4);
        shadow_bottom = bs_sub_read(&bs, 4);
        colorData[0] = (mt_u8)bs_sub_read(&bs, 8);
        colorData[1] = (mt_u8)bs_sub_read(&bs, 8);	
        pstParseInfo->stOutputData.unOutlineStyle.stDropshadow.u32ShadowColor = YUV2RGB((mt_u8*)&colorData);
    }
    else if (outline_style == 3)
    {
        bs_sub_skip(&bs, 24);
    }
    bs_sub_skip(&bs, 16); // bitmap_compressed_length
    bitmap_h = bottom_h - top_h;
    bitmap_v = bottom_v - top_v;
    bitmap_size = bitmap_h * bitmap_v;
    bitmap = (mt_u8 *)malloc(bitmap_size);
    memset(bitmap, 0, bitmap_size);
    if (!bitmap)
    {
        MT_ERR_SUBT("malloc bitmap error\n");
        return MT_FAILURE;
    }
    else
    {
        memset(bitmap, 0, bitmap_size);
    }
    for (position = 0; position < bitmap_size;)
    {
        if (bs_sub_eof(&bs))
        {
            for (; position < bitmap_size; position++)
                bitmap[position] = MT_FALSE;
            break;
        }

        run_on_length = 0;
        run_off_length = 0;
        if (bs_sub_read(&bs, 1) == 0)
        {
            if (bs_sub_read(&bs, 1) == 0)
            {
                if (bs_sub_read(&bs, 1) == 0)
                {
                    if (bs_sub_read(&bs, 2) == 1)
                    {
                        next = __MIN(((position / bitmap_h) + 1) * bitmap_h,
                                         bitmap_size);
                        for (; position < next; position++)
                            bitmap[position] = MT_FALSE;
                    }
                }
                else
                {
                    run_on_length = 4;
                }
            }
            else
            {
                run_off_length = 6;
            }
        }
        else
        {
            run_on_length = 3;
            run_off_length = 5;
        }

        if (run_on_length > 0)
        {
            run = bs_sub_read(&bs, (mt_s32)run_on_length);
            if (!run)
                run = (mt_u32)(1 << run_on_length);
            for (; position < bitmap_size && run > 0; position++, run--)
                bitmap[position] = MT_TRUE;
        }
        if (run_off_length > 0)
        {
            run = bs_sub_read(&bs, (mt_s32)run_off_length);
            if (!run)
                run = (mt_u32)(1 << run_off_length);
            for (; position < bitmap_size && run > 0; position++, run--)
                bitmap[position] = MT_FALSE;
        }
    }

    /* Render the bitmap */
    margin_h = 0;
    margin_v = 0;
    if (outline_style == 1)
    {
        margin_h =
        margin_v = outline_thickness;
    }
    else if (outline_style == 2)
    {
        margin_h = shadow_right;
        margin_v = shadow_bottom;
    }
    frame_top_h -= margin_h;
    frame_top_v -= margin_v;
    frame_bottom_h += margin_h;
    frame_bottom_v += margin_v;

    frame_h = frame_bottom_h - frame_top_h;
    frame_v = frame_bottom_v - frame_top_v;
    bitmap_oh = top_h - frame_top_h;
    bitmap_ov = top_v - frame_top_v;
    pstParseInfo->stOutputData.u32FrameTopXPos = frame_top_h;
    pstParseInfo->stOutputData.u32FrameTopYPos = frame_top_v;
    pstParseInfo->stOutputData.u32FrameButtomXPos = frame_bottom_h;
    pstParseInfo->stOutputData.u32FrameButtomYPos = frame_bottom_v;

    /*before malloc pu8SCTESubtData,free it*/
    if(pstParseInfo->stOutputData.pu8SCTESubtData)
    {
        free((void*)pstParseInfo->stOutputData.pu8SCTESubtData);
        pstParseInfo->stOutputData.pu8SCTESubtData = NULL;
    }
    pstParseInfo->stOutputData.u32BitmapDataLen = frame_h*frame_v*SCTE_SUBT_BPP;
    pstParseInfo->stOutputData.pu8SCTESubtData = (mt_u8 *)malloc(frame_h*frame_v*SCTE_SUBT_BPP);
    if (NULL == pstParseInfo->stOutputData.pu8SCTESubtData)
    {
        MT_ERR_SUBT("malloc SCTESubtData datastructure error...\n");
        free((void*)bitmap);
        return MT_FAILURE;
    }
    else
    {
        memset(pstParseInfo->stOutputData.pu8SCTESubtData, 0, frame_h*frame_v*SCTE_SUBT_BPP);
    }

    /* Fill up with frame (background) color */
    if (is_framed)
    {
        for (by = 0; by < frame_v; by++)
        {
            for (bx = 0; bx < frame_h; bx++)
            {
                SetPixel(pstParseInfo->stOutputData.pu8SCTESubtData, frame_h*SCTE_SUBT_BPP,
                                 bx, by, pstParseInfo->stOutputData.stFramed.u32FrameColor);
            }
        }
    }

    /* Draw the outline/shadow if requested */
    if (outline_style == 1)
    {
        /* Draw an outline
         * XXX simple but slow and of low quality (no anti-aliasing) */
        //MT_BOOL circle[16][16];
        for (dy = 0; dy <= 15; dy++)
        {
            for (dx = 0; dx <= 15; dx++)
                circle[dy][dx] = (dx > 0 || dy > 0) &&
                                 dx * dx + dy * dy <= outline_thickness * outline_thickness;
        }
        for (by = 0; by < bitmap_v; by++)
        {
            for (bx = 0; bx < bitmap_h; bx++)
            {
                if (!bitmap[by * bitmap_h + bx])
                    continue;
                for (dy = 0; dy <= outline_thickness; dy++)
                {
                    for (dx = 0; dx <= outline_thickness; dx++)
                    {
                        if (circle[dy][dx])
                        {
                            SetPixel(pstParseInfo->stOutputData.pu8SCTESubtData, frame_h*SCTE_SUBT_BPP,
                                         bx + bitmap_oh + dx, by + bitmap_ov + dy, pstParseInfo->stOutputData.unOutlineStyle.stOutline.u32OutlineColor);
                            SetPixel(pstParseInfo->stOutputData.pu8SCTESubtData, frame_h*SCTE_SUBT_BPP,
                                         bx + bitmap_oh - dx, by + bitmap_ov + dy, pstParseInfo->stOutputData.unOutlineStyle.stOutline.u32OutlineColor);
                            SetPixel(pstParseInfo->stOutputData.pu8SCTESubtData, frame_h*SCTE_SUBT_BPP,
                                         bx + bitmap_oh + dx, by + bitmap_ov - dy, pstParseInfo->stOutputData.unOutlineStyle.stOutline.u32OutlineColor);
                            SetPixel(pstParseInfo->stOutputData.pu8SCTESubtData, frame_h*SCTE_SUBT_BPP,
                                         bx + bitmap_oh - dx, by + bitmap_ov - dy, pstParseInfo->stOutputData.unOutlineStyle.stOutline.u32OutlineColor);
                        }
                    }
                }
            }
        }
    }
    else if (outline_style == 2)
    {
        /* Draw a shadow by drawing the character shifted by shaddow right/bottom */
        for (by = 0; by < bitmap_v; by++)
        {
            for (bx = 0; bx < bitmap_h; bx++)
            {
                if (bitmap[by * bitmap_h + bx])
                    SetPixel(pstParseInfo->stOutputData.pu8SCTESubtData, frame_h*SCTE_SUBT_BPP,
                                 bx + bitmap_oh + shadow_right,
                                 by + bitmap_ov + shadow_bottom,
                                 pstParseInfo->stOutputData.unOutlineStyle.stDropshadow.u32ShadowColor);
            }
        }
    }

    /* Draw the character */
    for (by = 0; by < bitmap_v; by++)
    {
        for (bx = 0; bx < bitmap_h; bx++)
        {
            if (bitmap[by * bitmap_h + bx])
                SetPixel(pstParseInfo->stOutputData.pu8SCTESubtData, frame_h*SCTE_SUBT_BPP,
                             bx + bitmap_oh, by + bitmap_ov, pstParseInfo->stOutputData.u32SubtColor);
        }
    }

    free((void*)bitmap);
    return MT_SUCCESS;	

}

mt_s32 SCTE_SUBT_Parse_Init(mt_void)
{
    return MT_SUCCESS;
}

mt_s32 SCTE_SUBT_Parse_DeInit(mt_void)
{
    return MT_SUCCESS;
}

mt_s32 SCTE_SUBT_Parse_Create(MT_HANDLE hDisplay, MT_HANDLE *phParse)
{
    mt_s32 s32Ret = MT_SUCCESS;
    SCTE_SUBT_PARSE_INFO_S *pstParseInfo = MT_NULL;

    pstParseInfo = (SCTE_SUBT_PARSE_INFO_S *)malloc(sizeof(SCTE_SUBT_PARSE_INFO_S));
    if (MT_NULL == pstParseInfo)
    {
        MT_ERR_SUBT("malloc parse datastructure error...\n");
        return MT_FAILURE;
    }

    memset(pstParseInfo, 0, sizeof(SCTE_SUBT_PARSE_INFO_S));
    pstParseInfo->hDisplay = hDisplay;
    *phParse = (MT_HANDLE)pstParseInfo;

    MT_INFO_SUBT("SCTE_SUBT_Parse_Create success, with handle:0x%08x!\n", *phParse);

    return s32Ret;
}

mt_s32 SCTE_SUBT_Parse_RegGetPtsCb(MT_HANDLE hParse, MT_UNF_SUBT_GETPTS_FN pfnGetPts, ulong u32UserData)
{
    SCTE_SUBT_PARSE_INFO_S *pstParseInfo = (SCTE_SUBT_PARSE_INFO_S *)hParse;
    if (MT_NULL == pstParseInfo)
    {
        MT_ERR_SUBT("param is MT_NULL...\n");
        return MT_FAILURE;
    }
    pstParseInfo->pfnGetPts = pfnGetPts;
    pstParseInfo->u32UserData = u32UserData;

    return MT_SUCCESS;

}

mt_s32 SCTE_SUBT_Parse_Destroy(MT_HANDLE hParse)
{
    SCTE_SUBT_PARSE_INFO_S *pstParseInfo = (SCTE_SUBT_PARSE_INFO_S *)hParse;

    if (MT_NULL == pstParseInfo)
    {
        MT_ERR_SUBT("param is MT_NULL...\n");
        return MT_FAILURE;
    }

    MT_INFO_SUBT("begin to use handle:0x%08x!\n", pstParseInfo);

    if (pstParseInfo->stOutputData.pu8SCTESubtData)
    {
        free((void*)pstParseInfo->stOutputData.pu8SCTESubtData);
        pstParseInfo->stOutputData.pu8SCTESubtData = MT_NULL;
    }

    free((void*)pstParseInfo);
    pstParseInfo = MT_NULL;
    MT_INFO_SUBT("success!\n");

    return MT_SUCCESS;
}

mt_s32 SCTE_SUBT_Parse_ParseSection(MT_HANDLE hParse, mt_u8 *pu8DataSrc, mt_u32 u32DataLen)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u8  *pu8Data = pu8DataSrc;
    mt_s64 s64CurPts = 0;
    mt_u32 u32PTSValue = 0;
    mt_u32 u32Duration = 0;
    mt_u8  u8ImmediateFlag = 0, u8PreClearFlag = 0, u8HeadLength = 0, u8Dis_standard = 0;
    mt_u8  u8FrameRate = 0;

    SCTE_SUBT_PARSE_INFO_S *pstParseInfo = (SCTE_SUBT_PARSE_INFO_S *)hParse;

    if (MT_NULL == pstParseInfo)
    {
        MT_ERR_SUBT("param is MT_NULL!!!\n");

        return MT_FAILURE;
    }

    /*skip section first seven bytes:table_id: 8bits
                            zero: 2bits
                            reserved: 2bits
                            section length: 12bits
                            zero: 1bit
                            segmention overlay includes: 1bit
                            protocal version: 6bis
                            ISO_639_language: 24bits
     *********************************************/

    //printf("section length is %#x\n",(pu8Data[1] <<8|pu8Data[2])  & 0x0fff);
    pu8Data += 7;
    u8HeadLength = (mt_u8)(u8HeadLength + 7);

    u8PreClearFlag  = pu8Data[0] & 0x80;
    u8ImmediateFlag = pu8Data[0] & 0x40;
    u8Dis_standard = pu8Data[0] & 0x1f;

    switch(u8Dis_standard)
    {
        case 0x00:
            u8FrameRate = 30;
            pstParseInfo->stOutputData.enDispStandard = STANDARD_720_480_30;
            break;
        case 0x01:
            u8FrameRate = 25;
            pstParseInfo->stOutputData.enDispStandard = STANDARD_720_576_25;
            break;
        case 0x02:
            u8FrameRate = 60;
            pstParseInfo->stOutputData.enDispStandard = STANDARD_1280_720_60;
            break;
        case 0x03:
            u8FrameRate = 60;
            pstParseInfo->stOutputData.enDispStandard = STANDARD_1920_1080_60;
            break;
        default:
            u8FrameRate = 25;
            pstParseInfo->stOutputData.enDispStandard = STANDARD_720_576_25;
            break;
    }

    pu8Data += 1;
    u8HeadLength = (mt_u8)(u8HeadLength+ 1);
    u32PTSValue = (mt_u32)((pu8Data[0] << 24) | (pu8Data[1] << 16) | (pu8Data[2] << 8) | pu8Data[3]);
    u32PTSValue /= PTS_UNITS;/*in units of 90Khz*/
    if(pstParseInfo->pfnGetPts)
    {
        (mt_void)pstParseInfo->pfnGetPts(pstParseInfo->u32UserData,&s64CurPts);
        if(u32PTSValue < (mt_u32)s64CurPts )
        {
            u32PTSValue += (mt_u32)PTS_OFFSET;
        }
    }

    /*operation*/
    if (u8PreClearFlag)
    {
        pstParseInfo->stOutputData.enDISP = SCTE_SUBT_DISP_PRECLEAR;
    }
    else
    {
        pstParseInfo->stOutputData.enDISP = SCTE_SUBT_DISP_NORMAL;
    }

    if (u8ImmediateFlag)
    {
        pstParseInfo->stOutputData.u32PTS = (mt_u32)s64CurPts;
    }
    else
    {
        pstParseInfo->stOutputData.u32PTS = u32PTSValue;
    }

    /*skip display_in_PTS:32bits*/
    pu8Data += 4;
    u8HeadLength = (mt_u8)(u8HeadLength + 4);

    u32Duration = (pu8Data[0]| pu8Data[1]) & 0x07ff;
    /*unit:ms*/
    u32Duration = (u32Duration * 1000) / u8FrameRate;
    pstParseInfo->stOutputData.u32Duration = u32Duration;

    pu8Data += 2;
    u8HeadLength = (mt_u8)(u8HeadLength + 2);

    pu8Data += 2;
    u8HeadLength = (mt_u8)(u8HeadLength + 2);

    if ((pu8Data != (&pu8DataSrc[u8HeadLength])))
    {
        MT_ERR_SUBT("\nerror, pu8Data addr:%p, 0x%02x, &pDataSrc[u8PesHeadLength] addr:%p, 0x%02x\n",
                 pu8Data, pu8Data[0], &pu8DataSrc[u8HeadLength], pu8DataSrc[u8HeadLength]);

        return MT_FAILURE;
    }

    //s32Ret = ParseBitmapHead(pstParseInfo, &pu8DataSrc[u8HeadLength], u32DataLen - (mt_u32)u8HeadLength);
    s32Ret = DecodeSimpleBitmap(pstParseInfo, &pu8DataSrc[u8HeadLength], u32DataLen - (mt_u32)u8HeadLength);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_SUBT("failed in SCTE_SUBT_Parse_ParseBitmap !!!\n");

        return MT_FAILURE;
    }

    /*Display*/
    s32Ret = SCTE_SUBT_Display_DisplaySubt(pstParseInfo->hDisplay, &pstParseInfo->stOutputData);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_SUBT("failed in SCTE_SUBT_Display_DisplaySubt !!!\n");

        return MT_FAILURE;
    }

    MT_INFO_SUBT(" SCTE_SUBT_Parse_ParseSection success  ...\n");

    return s32Ret;
}

