/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#include "mtfb_comm.h"
#include "mt_drv_mmz.h"
#include "mt_gfx_comm_k.h"
#include "mt_module_debug.h"

static mt_void MT_GFX_ShowVersionK(MTGFX_MODE_ID_E ModID)
{
	#if !defined(CONFIG_GFX_COMM_VERSION_DISABLE) && !defined(CONFIG_GFX_COMM_DEBUG_DISABLE)
	
    	mt_char MouleName[7][10] = {"tde","jpegdec","jpegenc","fb","png", "mtgo", "gfx2d"};
        mt_char Version[160] ="SDK_VERSION:["MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
		__DATE__", "__TIME__"]";

    	if (ModID >= MTGFX_BUTT_ID)
    		return;
	
	if ((MTGFX_JPGDEC_ID == ModID) || (MTGFX_JPGENC_ID == ModID))	
		GFX_Printk("Load mt_%s.ko success.\t(%s)\n", MouleName[ModID],Version);
	else
		GFX_Printk("Load mt_%s.ko success.\t\t(%s)\n", MouleName[ModID],Version);		

	return;
		
	#endif
}

/***************************************************************************************
* func          : mtfb_version
* description   : CNcomment: 打印版本号 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_void mtfb_version(mt_void) 
{
	MT_GFX_ShowVersionK(MTGFX_FB_ID);
}
/***************************************************************************************
* func          : mtfb_buf_map
* description   : CNcomment: 内存映射 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_void *mtfb_buf_map(phys_addr_t u32PhyAddr)
{
	return MT_GFX_Map(u32PhyAddr);
}

/***************************************************************************************
* func          : mtfb_buf_ummap
* description   : CNcomment: 内存逆映射 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 mtfb_buf_ummap(mt_void *pViraddr)
{
	return MT_GFX_Unmap(pViraddr);
}
/***************************************************************************************
* func          : mtfb_buf_freemem
* description   : CNcomment: 释放内存 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_void mtfb_buf_freemem(phys_addr_t u32Phyaddr)
{
	MT_GFX_FreeMem(u32Phyaddr);
}
/***************************************************************************************
* func          : mtfb_buf_allocmem
* description   : CNcomment: 分配内存 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
phys_addr_t mtfb_buf_allocmem(mt_char *pName, mt_u32 u32LayerSize)
{
	return MT_GFX_AllocMem(pName, "mtfb", u32LayerSize);
}


inline mt_s32 MTFB_RectToRegion(const MTFB_RECT* pRect, MTFB_REGION* pRegion)
{
    pRegion->l = pRect->x;
    pRegion->t = pRect->y;
    pRegion->r = pRect->x + pRect->w;
    pRegion->b = pRect->y + pRect->h;
    if (pRegion->r < 0)
    {
        pRegion->r = 0;
    }

    if (pRegion->b < 0)
    {
        pRegion->b = 0;
    }

    return MT_SUCCESS;
}

inline MT_BOOL MTFB_IntersectRgn (const MTFB_REGION* psrc1, const MTFB_REGION* psrc2)
{
    mt_s32 left, top, right, bottom;

    top    = (psrc1->t > psrc2->t) ? psrc1->t : psrc2->t;
    left = (psrc1->l > psrc2->l) ? psrc1->l : psrc2->l;
    right  = (psrc1->r < psrc2->r) ? psrc1->r : psrc2->r;
    bottom = (psrc1->b < psrc2->b)
             ? psrc1->b : psrc2->b;

    if ((left >= right) || (top >= bottom))
    {
        return MT_FALSE;
    }

    return MT_TRUE;
}

inline mt_s32 MTFB_RegionToRect(const MTFB_REGION* pRegion, MTFB_RECT* pRect)
{
    pRect->x = pRegion->l;
    pRect->y = pRegion->t;
    pRect->w = pRegion->r - pRegion->l;
    pRect->h = pRegion->b - pRegion->t;
    return MT_SUCCESS;
}



inline MT_BOOL MTFB_IsIntersectRect(const MTFB_RECT* pRect1, const MTFB_RECT* pRect2)
{
    MTFB_REGION rc1;
    MTFB_REGION rc2;

    (mt_void)MTFB_RectToRegion(pRect1, &rc1);
    (mt_void)MTFB_RectToRegion(pRect2, &rc2);
    return MTFB_IntersectRgn(&rc1, &rc2);
}




/** Add rectangles, merge them if they can,  if the total counts of the rectangles more than max count and also can't merge,
we can merge them to the last rectangle*/
mt_void mtfb_addrect(MTFB_RECT *pRectHead, mt_u32 TotalNum, mt_u32 *pValidNum, MTFB_RECT *pRect)
{
	mt_s32 Index;

    /** return, if the rectangle 's mtgh or width is 0 */
	if ((0 == pRect->h) || (0 == pRect->w))
	{
		return;
	}

	/** scan all the rectangle and check if they can merge, if they can, just merge them and return */	
    for (Index = 0; Index < *pValidNum; Index++)
	{
        /** check if they cover each other */
		if (((pRect->x <= pRectHead[Index].x) 
			 && (pRect->x + pRect->w >= pRectHead[Index].x + pRectHead[Index].w)
			 && (pRect->y <= pRectHead[Index].y) 
			 && (pRect->y + pRect->h >= pRectHead[Index].y + pRectHead[Index].h))
			|| ((pRect->x >= pRectHead[Index].x) 
			 && (pRect->x + pRect->w <= pRectHead[Index].x + pRectHead[Index].w)
			 && (pRect->y >= pRectHead[Index].y) 
			 && (pRect->y + pRect->h <= pRectHead[Index].y + pRectHead[Index].h)))
		{
            MTFB_UNITE_RECT(pRectHead[Index], (*pRect)); 
			return;
		}


		/** check if they can merge by left and right */
        if ((pRect->y == pRectHead[Index].y)
			&& (((pRect->x <= pRectHead[Index].x) &&  (pRect->x + pRect->w >= pRectHead[Index].x))
			    || ((pRectHead[Index].x <= pRect->x) && (pRectHead[Index].x + pRectHead[Index].w >= pRect->x))))
		{
            MTFB_UNITE_RECT(pRectHead[Index], (*pRect)); 
			return;
		}


		/**  check if they can merge by up and down * */
		if ((pRect->x == pRectHead[Index].x)
			&& (((pRect->y <= pRectHead[Index].y) && (pRect->y + pRect->h >= pRectHead[Index].y)) 
			    || ((pRectHead[Index].y <= pRect->y) && (pRectHead[Index].y + pRectHead[Index].h >= pRect->y))))
		{
            MTFB_UNITE_RECT(pRectHead[Index], (*pRect)); 
			return;
		}
	}

	/** check if there is emputy rectangle to use, if yes, add , if no, merge it to the last rectangle */	
	if (*pValidNum < TotalNum)
	{
        pRectHead[*pValidNum] = *pRect;
		(*pValidNum)++;
	}
	else
	{
        MTFB_UNITE_RECT(pRectHead[*pValidNum - 1], (*pRect)); 
	}

	return;
}

/*check these two rectangle cover each other*/
MT_BOOL mtfb_iscontain(MTFB_RECT *pstParentRect, MTFB_RECT *pstCmtldRect)
{
    MTFB_POINT_S stPoint;
    stPoint.s32XPos = pstCmtldRect->x;
    stPoint.s32YPos = pstCmtldRect->y;
    if ((stPoint.s32XPos < pstParentRect->x) || (stPoint.s32XPos > (pstParentRect->x + pstParentRect->w))
        || (stPoint.s32YPos < pstParentRect->y) || (stPoint.s32YPos > (pstParentRect->y + pstParentRect->h)))
    {
        return MT_FALSE;
    }
    stPoint.s32XPos = pstCmtldRect->x + pstCmtldRect->w;
    stPoint.s32YPos = pstCmtldRect->y + pstCmtldRect->h;
    if ((stPoint.s32XPos < pstParentRect->x) || (stPoint.s32XPos > (pstParentRect->x + pstParentRect->w))
        || (stPoint.s32YPos < pstParentRect->y) || (stPoint.s32YPos > (pstParentRect->y + pstParentRect->h)))
    {
        return MT_FALSE;
    }
    return MT_TRUE;
}

/***************************************************************************************
* func			: mtfb_isoverlay
* description	: check these two rectangle overlay each other
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
MT_BOOL mtfb_isoverlay(MTFB_RECT *pstSrcRect, MTFB_RECT *pstDstRect)
{
    if (   pstSrcRect->x >= (pstDstRect->x + pstDstRect->w)
		|| pstDstRect->x >= (pstSrcRect->x + pstSrcRect->w))
    {
        return MT_FALSE;
    }

	if(    pstSrcRect->y >= (pstDstRect->y+pstDstRect->h)
		|| pstDstRect->y >= (pstSrcRect->y+pstSrcRect->h))
    {
        return MT_FALSE;
    }

	return MT_TRUE;	
}


/***************************************************************************************
* func          : mtfb_getbppbyfmt
* description   : CNcomment:  CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_u32 mtfb_getbppbyfmt(MTFB_COLOR_FMT_E enColorFmt)
{
    switch(enColorFmt)
    {
        case MTFB_FMT_RGB565:
        case MTFB_FMT_KRGB444: 
        case MTFB_FMT_KRGB555:
        case MTFB_FMT_ARGB4444:
        case MTFB_FMT_ARGB1555:
        case MTFB_FMT_RGBA4444:
        case MTFB_FMT_RGBA5551:
        case MTFB_FMT_ACLUT88:
        case MTFB_FMT_BGR565:
        case MTFB_FMT_ABGR1555:
        case MTFB_FMT_ABGR4444:
        case MTFB_FMT_KBGR444:
        case MTFB_FMT_KBGR555:
        {
            return 16;
        }
        case MTFB_FMT_RGB888:
        case MTFB_FMT_ARGB8565: 
        case MTFB_FMT_RGBA5658:
        case MTFB_FMT_ABGR8565:
        case MTFB_FMT_BGR888:              
        {
            return 24;
        }
        case MTFB_FMT_KRGB888:
        case MTFB_FMT_ARGB8888:
        case MTFB_FMT_RGBA8888:
        case MTFB_FMT_ABGR8888:
        case MTFB_FMT_KBGR888:            
        {
            return 32;
        }
        case MTFB_FMT_1BPP:
        {
            return 1;
        }
        case MTFB_FMT_2BPP:
        {
            return 2;
        }
        case MTFB_FMT_4BPP:
        {
            return 4;
        }
        case MTFB_FMT_8BPP:
        case MTFB_FMT_ACLUT44:
        case MTFB_FMT_RGB233:
        {
            return 8;
        }
        default:
            return 0;   
    }
}

static MT_BOOL mtfb_getBytesByFmt(mt_u32 pitch, MTFB_COLOR_FMT_E enColorFmt, mt_u32 *p_bytes)
{
   MT_BOOL bigFlg = 1;
   mt_u32   bytessss = 0;
   
   switch(enColorFmt)
    {
        case MTFB_FMT_RGB565:
        case MTFB_FMT_KRGB444: 
        case MTFB_FMT_KRGB555:
        case MTFB_FMT_ARGB4444:
        case MTFB_FMT_ARGB1555:
        case MTFB_FMT_RGBA4444:
        case MTFB_FMT_RGBA5551:
        case MTFB_FMT_ACLUT88:
        case MTFB_FMT_BGR565:
        case MTFB_FMT_ABGR1555:
        case MTFB_FMT_ABGR4444:
        case MTFB_FMT_KBGR444:
        case MTFB_FMT_KBGR555:
        {
            bytessss = 2;
            break;
        }
        case MTFB_FMT_RGB888:
        case MTFB_FMT_ARGB8565: 
        case MTFB_FMT_RGBA5658:
        case MTFB_FMT_ABGR8565:
        case MTFB_FMT_BGR888:              
        {
            bytessss =  3;
            break;
        }
        case MTFB_FMT_KRGB888:
        case MTFB_FMT_ARGB8888:
        case MTFB_FMT_RGBA8888:
        case MTFB_FMT_ABGR8888:
        case MTFB_FMT_KBGR888:            
        {
            bytessss =  4;
            break;
        }
        case MTFB_FMT_1BPP:
        { 
            bytessss = 8;
            bigFlg = 0;
            break;
        }
        case MTFB_FMT_2BPP:
        {
            bytessss = 4;
            bigFlg = 0;
            break;
        }
        case MTFB_FMT_4BPP:
        {
            bytessss = 2;
            bigFlg = 0;
            break;
        }
        case MTFB_FMT_8BPP:
        case MTFB_FMT_ACLUT44:
        case MTFB_FMT_RGB233:
        {
            bytessss = 1;
            break;
        }
        default:
            //MT_ASSERT(0);
            MT_ERR_MTFB("mtfb_getBytesByFmt: err [%x] [%x]\n", enColorFmt, pitch);
            bytessss = 1;
            bigFlg = 0;
            return 0;   
    }

    *p_bytes = bytessss;
    return bigFlg;
}


mt_u32 calc_pixel_stride_by_pitch_fmt(mt_u32 pitch, MTFB_COLOR_FMT_E enColorFmt, mt_u32 line)
{
      mt_u32 pixel_stride = 0;
      mt_u32 bytes = 0;
      mt_u32 bigFlg = 1;

      MT_ASSERT(pitch % 16 == 0);  // 16 byte aligned
      
       bigFlg = mtfb_getBytesByFmt(pitch, enColorFmt, &bytes);
       if(bigFlg)
       {
          pixel_stride = pitch / bytes;
       }
       else
       {
          pixel_stride = pitch * bytes;
       }

      //printk("calc_pixel_stride_by_pitch_fmt[%d]: [%d] [%d] [%d] [%d]\n",  line, pitch, enColorFmt, bytes, pixel_stride);
      return pixel_stride;
}


/***************************************************************************************
* func          : mtfb_bitfieldcmp
* description   : CNcomment: 判断两个像素格式是否相等 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
 mt_s32 mtfb_bitfieldcmp(struct fb_bitfield x, struct fb_bitfield y)
{
    if (   (x.offset == y.offset)
        && (x.length == y.length)
        && (x.msb_right == y.msb_right))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}


