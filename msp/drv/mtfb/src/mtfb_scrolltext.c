/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>

#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/spinlock.h>

#include <linux/fb.h>
#include <linux/interrupt.h>

#include "mtfb_drv.h"
#include "mtfb.h"
#include "mtfb_p.h"
#include "mtfb_comm.h"
#include "mtfb_scrolltext.h"





#ifdef CFG_MTFB_SCROLLTEXT_SUPPORT

static spinlock_t scrolltextLock = __SPIN_LOCK_UNLOCKED(scrolltextLock);


/***************************************************************************************
* func			: mtfb_alloscrolltext_handle
* description	: 判断滚动字幕参数是否合法
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_u32 mtfb_alloscrolltext_handle(mt_u32 u32LayerId) 
{
    mt_u32 u32ScrollTextHandle;
    MTFB_SCROLLTEXT_INFO_S *pstScrollTextInfo;
    pstScrollTextInfo = &s_stTextLayer[u32LayerId];

	/** 最多只能有两个字幕 **/
    u32ScrollTextHandle = pstScrollTextInfo->u32ScrollTextId++;
	if(pstScrollTextInfo->u32ScrollTextId > 1)
	{
		pstScrollTextInfo->u32ScrollTextId = 0;
	}
    if (!pstScrollTextInfo->bAvailable)
    {
        MTFB_ERROR("the scroll text was invalid!\n");
        return MTFB_SCROLLTEXT_BUTT_HANDLE;
    }

   /** MTFB_SCROLLTEXT_HANDLE :
	**    MTFB_SCROLLTEXT_HD0_HANDLE1 0x21
	**                                  2 = MTFB_LAYER_HD0
	**                                  1 = u32textnum(2) - 1
	** u32ScrollTextHandle = 0 或 1
	** (0x0f & u32ScrollTextHandle) = 0 或 1
	** u32LayerId等于以下几个值
	**MTFB_LAYER_HD_0 = 0x0,
	**MTFB_LAYER_HD_1 = 0x1,
	**MTFB_LAYER_HD_2 = 0x2,
	**MTFB_LAYER_HD_3 = 0x3,
	**
	**MTFB_LAYER_SD_0 = 0x4, 
	**MTFB_LAYER_SD_1 = 0x5,    
	**MTFB_LAYER_SD_2 = 0x6,    
	**MTFB_LAYER_SD_3 = 0x7, 
	**
	**MTFB_LAYER_AD_0 = 0x8, 
	**MTFB_LAYER_AD_1 = 0x9,
	**MTFB_LAYER_AD_2 = 0x10,
	**MTFB_LAYER_AD_3 = 0x11
	**高4位表示图层ID，低四位表示哪个字幕
	**例如MTFB_SCROLLTEXT_HD1_HANDLE0 表示MTFB_LAYER_HD_1 = fb1字母层第一个字幕0x10
    **/
    return ((0xf0 & (u32LayerId << 4)) | (0x0f & u32ScrollTextHandle));
	
}

/***************************************************************************************
* func			: mtfb_parse_scrolltexthandle
* description	: 获取图层ID和字幕的ID
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_u32 mtfb_parse_scrolltexthandle(mt_u32 u32Handle, mt_u32 *pU32LayerId, mt_u32 *pScrollTextId) 
{


    if (u32Handle >= MTFB_SCROLLTEXT_BUTT_HANDLE)
    {
        MTFB_ERROR("invalid scrolltext handle!\n");
		return MT_FAILURE;
    }
    /** 这样算的原因是因为创建字幕的时候句柄是根据图层ID和字幕ID组合的**/
    *pU32LayerId   = (u32Handle & 0xf0) >> 4;
    *pScrollTextId = u32Handle & 0x0f;

    if(   (MTFB_LAYER_CURSOR == *pU32LayerId)
        ||(*pU32LayerId >= MTFB_LAYER_ID_BUTT))
    {
        MTFB_ERROR("invalid scrolltext handle!\n");
        *pU32LayerId = MTFB_LAYER_ID_BUTT;
        return MT_FAILURE;
    }
	/** 最多只支持两个字幕 **/
    if (*pScrollTextId >= SCROLLTEXT_NUM)
    {
        MTFB_ERROR("invalid scrolltext handle!\n");
        *pScrollTextId = SCROLLTEXT_NUM;
        return MT_FAILURE;
    }

    return MT_SUCCESS;
	
}

/***************************************************************************************
* func			: mtfb_check_scrolltext_para
* description	: 判断滚动字幕参数是否合法
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 mtfb_check_scrolltext_para(mt_u32 u32LayerId, MTFB_SCROLLTEXT_ATTR_S *stAttr)
{

	mt_u32 i;
	MTFB_RECT stScrollTextRect, stSrcRect;
    struct fb_info *info = s_stLayer[u32LayerId].pstInfo;
    MTFB_PAR_S *pstPar   = (MTFB_PAR_S *)info->par;
	
    stScrollTextRect = stAttr->stRect;
    if (0 == stAttr->u16CacheNum || SCROLLTEXT_CACHE_NUM < stAttr->u16CacheNum)
    {
        MTFB_ERROR("the cachenum u applied was invalid!\n");                
        return MT_FAILURE;
    }
	/**
	 **一个图层最多只支持上下两个字幕,要是使用一个字幕层创建两个字幕的情况下
	 **要是使用两个图层分别创建上下字幕这里就不会起作用了
	 **/
    if(s_stTextLayer[u32LayerId].u32textnum >= SCROLLTEXT_NUM)
    {
        MTFB_ERROR("the scrolltext num created by hifb%d reached the maxinum!\n", u32LayerId);                
        return MT_FAILURE;
    }

    if (  (0 > stScrollTextRect.x)
		||(0 > stScrollTextRect.y)
        ||(pstPar->stExtendInfo.stPos.s32XPos > stScrollTextRect.x )
        ||(pstPar->stExtendInfo.stPos.s32YPos > stScrollTextRect.y))
    {
        MTFB_ERROR("failed to create the scrolltext because of wrong pos info!\n");                
        return MT_FAILURE;
    }

    if (   (0 > stScrollTextRect.w)
		 ||(0 > stScrollTextRect.h)
         ||(pstPar->stExtendInfo.u32DisplayWidth  < stScrollTextRect.w)
         ||(pstPar->stExtendInfo.u32DisplayHeight < stScrollTextRect.h))
    {
        MTFB_ERROR("failed to create the scrolltext because of wrong width or height!\n");                
        return MT_FAILURE;
    }

   if (  (pstPar->stExtendInfo.stPos.s32XPos + pstPar->stExtendInfo.u32DisplayWidth) < (stScrollTextRect.w + stScrollTextRect.x)
	    ||(pstPar->stExtendInfo.stPos.s32YPos + pstPar->stExtendInfo.u32DisplayHeight) < (stScrollTextRect.h + stScrollTextRect.y)) 
    {
        MTFB_ERROR("failed to create the scrolltext because of wrong width or height!\n");                
        return MT_FAILURE;
    }

	/** 
	 ** whether the scroll text overlayed with each other
	 ** 两个字幕不能有重叠区域的情况
	 **/
	for (i = 0; i < s_stTextLayer[u32LayerId].u32textnum; i++)
	{
		if(s_stTextLayer[u32LayerId].stScrollText[i].bAvailable)
		{
			stSrcRect = s_stTextLayer[u32LayerId].stScrollText[i].stRect;
			if(mtfb_isoverlay(&stSrcRect, &stScrollTextRect))
			{
				MTFB_ERROR("failed to create the scrolltext because the scrolltext overlayed with another!\n");                
        		return MT_FAILURE;
			}
		}
	}

    return MT_SUCCESS;
    
}

/***************************************************************************************
* func			: mtfb_freescrolltext_cachebuf
* description	: 释放滚动字幕buffer
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 mtfb_freescrolltext_cachebuf(MTFB_SCROLLTEXT_S *pstScrollText)
{
    mt_u32 i;
    mt_char *pBuf;

    for (i = 0; i < pstScrollText->u32cachebufnum; i++)
    {
        pBuf = pstScrollText->stCachebuf[i].pVirAddr;
        if (MT_NULL != pBuf)
        {
            mtfb_buf_ummap(pBuf);
        }
        pstScrollText->stCachebuf[i].pVirAddr = MT_NULL;        
        
        if (pstScrollText->stCachebuf[i].u32PhyAddr != 0)
        {
            mtfb_buf_freemem(pstScrollText->stCachebuf[i].u32PhyAddr);
        }
        pstScrollText->stCachebuf[i].u32PhyAddr = 0;

		pstScrollText->stCachebuf[i].bInusing   = MT_FALSE;
    }
    
    return MT_SUCCESS;
}


/***************************************************************************************
* func			: mtfb_allocscrolltext_buf
* description	: 创建滚动字幕buffer
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 mtfb_allocscrolltext_buf(mt_u32 u32LayerId, MTFB_SCROLLTEXT_ATTR_S *stAttr)
{

	struct fb_info *info = s_stLayer[u32LayerId].pstInfo;
	mt_u32 u32StartAddr;
    mt_u32 i, u32cacheSize, u32Pitch;
    mt_char name[32];
    mt_char *pBuf;

	/**
	 ** 上下两个滚动字幕，使用的是第几个滚动字幕
	 ** 要么一个图层中有两个字幕
	 ** 要么两个图层分别使用一个字幕
	 **/
	mt_u32 u32Index  = s_stTextLayer[u32LayerId].u32ScrollTextId;
    MTFB_SCROLLTEXT_S *pstScrollText = &(s_stTextLayer[u32LayerId].stScrollText[u32Index]);
	/** 信息都保存在s_stTextLayer全局变量中 **/
	
    /** if with old cache buffer **/
    if(pstScrollText->bAvailable)
    {/** 该字幕已经用过了，清该字幕的buffer **/
        /** free old buffer*/
        MTFB_INFO("free old scrolltext cache buffer\n");        
        mtfb_freescrolltext_cachebuf(pstScrollText);
    }

    /** Modify 16 to 32, preventing out of bound. **/       
    /** 16 bytes aligmn **/
    u32Pitch = ((stAttr->stRect.w * info->var.bits_per_pixel >> 3) + 15)>>4;    
    u32Pitch = u32Pitch << 4;
        
    u32cacheSize =  u32Pitch * stAttr->stRect.h;

    /** 几buffer模式是用户态设置的 **/
    for (i = 0; i < stAttr->u16CacheNum; i++)
    {/** 双buffer模式 **/
		snprintf(name, sizeof(name), "MTFB_Layer%d_Scroll%d", u32LayerId, i);
		u32StartAddr = mtfb_buf_allocmem(name, u32cacheSize);

		if (MT_NULL == u32StartAddr)
		{   
		    MTFB_ERROR("failed to allocate cache buffer for the scrolltext!\n");
		    return MT_FAILURE;
		}
		pstScrollText->stCachebuf[i].u32PhyAddr = u32StartAddr;
        
        pBuf = (mt_char *)mtfb_buf_map(pstScrollText->stCachebuf[i].u32PhyAddr);
        if (pBuf == MT_NULL)
        {
            MTFB_ERROR("map cache buffer failed!\n");
            mtfb_buf_freemem(pstScrollText->stCachebuf[i].u32PhyAddr);
            return MT_FAILURE;
        }
        memset(pBuf, 0, u32cacheSize);
		/** 刚分配完还没有在使用 **/
        pstScrollText->stCachebuf[i].bInusing   = MT_FALSE;
        pstScrollText->stCachebuf[i].pVirAddr   = pBuf;
		
    }
    pstScrollText->bAvailable     = MT_TRUE; /** 分配完了，说明该字幕可以使用了 **/
    pstScrollText->u32Stride      = u32Pitch;
    pstScrollText->u32cachebufnum = stAttr->u16CacheNum;
    pstScrollText->bDeflicker     = stAttr->bDeflicker;
	pstScrollText->ePixelFmt      = stAttr->ePixelFmt;
    memcpy(&(pstScrollText->stRect), &(stAttr->stRect), sizeof(MTFB_RECT));

	return MT_SUCCESS;
    
}

/***************************************************************************************
* func			: mtfb_create_scrolltext
* description	: 创建滚动字幕
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 mtfb_create_scrolltext(mt_u32 u32LayerId, MTFB_SCROLLTEXT_CREATE_S *stScrollText)
{


	mt_s32 s32Ret;
	mt_u32 u32Index;
    MTFB_SCROLLTEXT_ATTR_S stAttr = stScrollText->stAttr; /** 用户传下来的信息 **/

	/** fb内部信息 **/
	MTFB_SCROLLTEXT_INFO_S *pstTextInfo = &s_stTextLayer[u32LayerId];
	MTFB_SCROLLTEXT_S *stText;

#if 0
	/** 要传给FB的信息 **/
	printk("\n==============================================================\n");
	/** /dev/fb1对应的u32LayerId = 1 **/
	printk("fb u32LayerId                   = %d\n",u32LayerId);
	printk("fb stScrollAttr.stScrollRect    = [%d,%d,%d,%d]\n",stAttr.stRect.x,stAttr.stRect.y,stAttr.stRect.w,stAttr.stRect.h);
	printk("fb stScrollAttr.ePixelFormat    = %d\n",stAttr.ePixelFmt);
	printk("fb stScrollAttr.u16CacheNum     = %d\n",stAttr.u16CacheNum);
	printk("fb stScrollAttr.u16RefreshFreq  = %d\n",stAttr.u16RefreshFreq);
	printk("fb stScrollAttr.bDeflicker      = %d\n",stAttr.bDeflicker);
	printk("==============================================================\n");
#endif

	/**
	 **u32LayerId这个在创建字幕层的时候已经赋值了。
	 **/
    /*check the parameter of scrolltext struct*/
	s32Ret = mtfb_check_scrolltext_para(u32LayerId, &stAttr);
    if (MT_SUCCESS != s32Ret)
    {
    	MTFB_ERROR("failed to create scrolltext!\n");
        return MT_FAILURE;
    }
	
    /**
     ** allocate buffer for scrolltext
     **/
	s32Ret = mtfb_allocscrolltext_buf(u32LayerId, &stAttr);
    if (MT_SUCCESS != s32Ret)
    {
    	MTFB_ERROR("failed to create scrolltext!\n");
        return MT_FAILURE;
    } 

    /**
     ** return the handle ,that usr can manipulate it
     ** to fill data to scrolltext
     **/
    pstTextInfo->u32textnum++;          /** 分配一个字幕就自加一个 **/
    pstTextInfo->bAvailable  = MT_TRUE; /** 该字幕可用 **/

	/** 这个字幕句柄是和图层ID以及每个图层有两个字幕相挂钩的 **/
	stScrollText->u32Handle = mtfb_alloscrolltext_handle(u32LayerId);  
	/** 最多只能有两个0或1 **/
	u32Index = stScrollText->u32Handle & 0x0f;	
	/** 这里也会保存到全局变量中 **/
	stText = &(pstTextInfo->stScrollText[u32Index]);
	stText->enHandle = stScrollText->u32Handle; /** 保存句柄，是哪个图形层的 **/
    stText->u32IdleFlag = 1;

	init_waitqueue_head(&(stText->wbEvent));

    return MT_SUCCESS;

}

/***************************************************************************************
* func			: mtfb_fill_scrolltext
* description	: 数据处理，这个地方是重头戏，处理比较复杂
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 mtfb_fill_scrolltext(MTFB_SCROLLTEXT_DATA_S *stScrollTextData)
{

    mt_u32 i, u32LayerId, u32TextId, u32Handle;
    mt_s32 s32Ret;
	unsigned long flags;
    MTFB_SCROLLTEXT_CACHE stCacheBuf;
    MTFB_BUFFER_S stTempBuf, stCanvasBuf;
    MTFB_BLIT_OPT_S stBlitOpt;
    struct fb_info *info;
    MTFB_PAR_S *pstPar;
    MTFB_SCROLLTEXT_S *pstScrollText;

	
    u32Handle = stScrollTextData->u32Handle;
    memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));

	/** 通过传入的句柄解析出图层的ID和字幕的ID **/
    s32Ret = mtfb_parse_scrolltexthandle(u32Handle, &u32LayerId, &u32TextId);
	if (MT_SUCCESS != s32Ret)
	{
		MTFB_ERROR("fill data to scrolltext failed because of invalid scrolltext handle!\n");
        return MT_FAILURE;
    }
    
    info = s_stLayer[u32LayerId].pstInfo;
    pstPar = (MTFB_PAR_S *)info->par;
    pstScrollText = &(s_stTextLayer[u32LayerId].stScrollText[u32TextId]);
    
    if(!pstScrollText->bAvailable)
    {/** 该字幕可用，创建的时候对这个参数已经赋值 **/
        MTFB_ERROR("the scrolltext was invalid!\n");
        return MT_FAILURE;
    }

	/**
	 ** if pause the scrolltext,no need to fill data to cache buffer,return success
	 ** 如果是暂停的话不做任何处理
	 **/
	if (pstScrollText->bPause)
	{
		return MT_SUCCESS;
	}
    /**
     **if virtual address was available, use memcpy to blit usr data
     **/
	if(MT_NULL != stScrollTextData->u32PhyAddr)
    {
        /*wait for a idle cache buffer ,then u can fill your data to it*/
		if(!pstScrollText->u32IdleFlag)
		{/** 等待空闲buffer **/
			wait_event_interruptible_hrtimeout(pstScrollText->wbEvent, pstScrollText->u32IdleFlag, ms_to_ktime(500));
		}       

        /*find the idle cache buffer ,fill user data to the idle buffer*/
        /*if all the cache buffer allocated for scrolltext was in using,block the user app*/
        stCanvasBuf.stCanvas.u32PhyAddr = stScrollTextData->u32PhyAddr;
        stCanvasBuf.stCanvas.enFmt      = pstScrollText->ePixelFmt;
        stCanvasBuf.stCanvas.u32Width   = pstScrollText->stRect.w;
        stCanvasBuf.stCanvas.u32Height  = pstScrollText->stRect.h;      
        stCanvasBuf.stCanvas.u32Pitch   = stScrollTextData->u32Stride;

        stCanvasBuf.UpdateRect.x        = 0;
        stCanvasBuf.UpdateRect.y        = 0;
        stCanvasBuf.UpdateRect.w        = pstScrollText->stRect.w;
        stCanvasBuf.UpdateRect.h        = pstScrollText->stRect.h;
    
        for (i = 0; i < pstScrollText->u32cachebufnum; i++)
        {
            stCacheBuf = pstScrollText->stCachebuf[i];
       
            if (!stCacheBuf.bInusing)
            {
                stTempBuf.stCanvas.u32PhyAddr = stCacheBuf.u32PhyAddr;
                stTempBuf.stCanvas.enFmt      = pstPar->stExtendInfo.enColFmt;
                stTempBuf.stCanvas.u32Width   = pstScrollText->stRect.w;
                stTempBuf.stCanvas.u32Height  = pstScrollText->stRect.h;                
				stTempBuf.stCanvas.u32Pitch = pstScrollText->u32Stride;
				
                stTempBuf.UpdateRect.x        = 0;
                stTempBuf.UpdateRect.y        = 0;
                stTempBuf.UpdateRect.w        = pstScrollText->stRect.w;
                stTempBuf.UpdateRect.h        = pstScrollText->stRect.h;

                if (stTempBuf.stCanvas.u32Pitch != stCanvasBuf.stCanvas.u32Pitch)
                {
                    stBlitOpt.bScale = MT_TRUE;
                }

                if (pstScrollText->bDeflicker && pstPar->stBaseInfo.enAntiflickerMode == MTFB_ANTIFLICKER_TDE)
                {
                    stBlitOpt.enAntiflickerLevel = pstPar->stBaseInfo.enAntiflickerLevel;
                }

                //stBlitOpt.bRegionDeflicker = MT_TRUE;
                stBlitOpt.bBlock           = MT_TRUE;
                stBlitOpt.bCallBack        = MT_FALSE;

                s32Ret = s_stDrvTdeOps.MTFB_DRV_Blit(&stCanvasBuf, &stTempBuf, &stBlitOpt, MT_TRUE);
                if (s32Ret <= 0)
                {
                    MTFB_ERROR("mtfb_fill_scrolltext blit err !\n");
                    return MT_FAILURE;
                }
                
               spin_lock_irqsave(&scrolltextLock, flags);
			   /** 这块buffer在使用 **/
               pstScrollText->stCachebuf[i].bInusing = MT_TRUE;
               /*clear Idle Flag default */
               pstScrollText->u32IdleFlag = 0;
               
                /*Traverse the Cache array from cache i+1,Once find inusing cache,switch the inusing cache to cache i*/
                while (i < pstScrollText->u32cachebufnum)
                {
                     mt_u32 u32LastUsingCache = i; 
                     mt_u32 j = i + 1;
                     
                     while (j < pstScrollText->u32cachebufnum)
                     {
                          if (pstScrollText->stCachebuf[j].bInusing)
                          {/** 第二块buffer是否被使用 **/               
                                MT_BOOL bInusing =  pstScrollText->stCachebuf[j].bInusing;          
                                mt_u32  u32PhyAddr = pstScrollText->stCachebuf[j].u32PhyAddr;
                                mt_u8   *pVirAddr = pstScrollText->stCachebuf[j].pVirAddr;
                                
               
                                pstScrollText->stCachebuf[j].bInusing = pstScrollText->stCachebuf[u32LastUsingCache].bInusing;
                                pstScrollText->stCachebuf[j].u32PhyAddr = pstScrollText->stCachebuf[u32LastUsingCache].u32PhyAddr;
                                pstScrollText->stCachebuf[j].pVirAddr = pstScrollText->stCachebuf[u32LastUsingCache].pVirAddr;
                                pstScrollText->stCachebuf[u32LastUsingCache].bInusing = bInusing;
                                pstScrollText->stCachebuf[u32LastUsingCache].u32PhyAddr = u32PhyAddr;
                                pstScrollText->stCachebuf[u32LastUsingCache].pVirAddr = pVirAddr;
                                u32LastUsingCache = j;
                          }
                          /*Find idle cache buffer,set the Idle Flag*/
                          else
                          {
                                pstScrollText->u32IdleFlag = 1;
                          }
                          j++;
						  
                     }
                     i++;
					 
                }
                
                
                /*Check whether the Cache 0  is Inusing, Because Cache 0 may be set  Inusing state in VO irq */
                if (!pstScrollText->stCachebuf[0].bInusing)
                {
                    mt_u32 i;

                    /*Cache 0 is not Inusing,So set the Idle Flag*/
                    pstScrollText->u32IdleFlag = 1;

                    /*Place the cache 0 to the end when cache 0 is inusing*/
                    for (i = 1; i < pstScrollText->u32cachebufnum; i++)
                    {
                            MT_BOOL bInusing =  pstScrollText->stCachebuf[i - 1].bInusing;          
                            mt_u32  u32PhyAddr = pstScrollText->stCachebuf[i - 1].u32PhyAddr;
                            mt_u8   *pVirAddr = pstScrollText->stCachebuf[i - 1].pVirAddr;

                            pstScrollText->stCachebuf[i - 1].bInusing = pstScrollText->stCachebuf[i].bInusing;
                            pstScrollText->stCachebuf[i - 1].u32PhyAddr = pstScrollText->stCachebuf[i].u32PhyAddr;
                            pstScrollText->stCachebuf[i - 1].pVirAddr = pstScrollText->stCachebuf[i].pVirAddr;
                            pstScrollText->stCachebuf[i].bInusing = bInusing;
                            pstScrollText->stCachebuf[i].u32PhyAddr = u32PhyAddr;
                            pstScrollText->stCachebuf[i].pVirAddr = pVirAddr;
                    }
                }

                spin_unlock_irqrestore(&scrolltextLock, flags);	
                
                return MT_SUCCESS;
            }
        }
    }
    else if (MT_NULL != stScrollTextData->pu8VirAddr)
    {
    	mt_char *pBuf;
        if (pstPar->stExtendInfo.enColFmt != pstScrollText->ePixelFmt)
		{
			MTFB_ERROR("invalid virtual address!\n");
			return MT_FAILURE;
		}
        /*wait for a idle cache buffer ,then u can fill your data to it*/
		if(!pstScrollText->u32IdleFlag)
		{					
			wait_event_interruptible_hrtimeout(pstScrollText->wbEvent, pstScrollText->u32IdleFlag, ms_to_ktime(500));
		}
        
        /*find the idle cache buffer ,fill user data to the idle buffer*/ 
        for (i = 0; i < pstScrollText->u32cachebufnum; i++)
        {
            mt_u32 u32LineNum;
            stCacheBuf = pstScrollText->stCachebuf[i];
            if (!stCacheBuf.bInusing)
            {
                pBuf = pstScrollText->stCachebuf[i].pVirAddr;
                for (u32LineNum = 0; u32LineNum < pstScrollText->stRect.h; u32LineNum++)
                {
                    memcpy(pBuf + u32LineNum*pstScrollText->u32Stride, 
                                 stScrollTextData->pu8VirAddr + u32LineNum*stScrollTextData->u32Stride, 
                                 pstScrollText->u32Stride);
                }
       
                spin_lock_irqsave(&scrolltextLock, flags);
                pstScrollText->stCachebuf[i].bInusing = MT_TRUE;
        
                pstScrollText->u32IdleFlag = 0;
            
                /*Traverse the Cache array from cache i+1,Once find inusing cache,switch the inusing cache to cache i*/
                while (i < pstScrollText->u32cachebufnum)
                {
                     mt_u32 u32LastUsingCache = i;
                     mt_u32 j = i + 1;
                     
                     while (j < pstScrollText->u32cachebufnum)
                     {
                          if (pstScrollText->stCachebuf[j].bInusing)
                          {                               
                                MT_BOOL bInusing =  pstScrollText->stCachebuf[j].bInusing;          
                                mt_u32  u32PhyAddr = pstScrollText->stCachebuf[j].u32PhyAddr;
                                mt_u8   *pVirAddr = pstScrollText->stCachebuf[j].pVirAddr;
                                
               
                                pstScrollText->stCachebuf[j].bInusing = pstScrollText->stCachebuf[u32LastUsingCache].bInusing;
                                pstScrollText->stCachebuf[j].u32PhyAddr = pstScrollText->stCachebuf[u32LastUsingCache].u32PhyAddr;
                                pstScrollText->stCachebuf[j].pVirAddr = pstScrollText->stCachebuf[u32LastUsingCache].pVirAddr;
                                pstScrollText->stCachebuf[u32LastUsingCache].bInusing = bInusing;
                                pstScrollText->stCachebuf[u32LastUsingCache].u32PhyAddr = u32PhyAddr;
                                pstScrollText->stCachebuf[u32LastUsingCache].pVirAddr = pVirAddr;
                                u32LastUsingCache = j;
                          }
                          /*Find idle cache buffer,set the Idle Flag*/
                          else
                          {
                                pstScrollText->u32IdleFlag = 1;
                          }
                          j++;
                     }
                      i++;
                }
                     
                /*Check whether the Cache 0  is Inusing, Because Cache 0 may be set  Inusing state in VO irq */
                if (!pstScrollText->stCachebuf[0].bInusing)
                {
                    mt_u32 i;

                    /*Cache 0 is not Inusing,So set the Idle Flag*/
                    pstScrollText->u32IdleFlag = 1;
                    
                    /*Place the cache 0 to the end when cache 0 is inusing*/
                    for (i = 1; i < pstScrollText->u32cachebufnum; i++)
                    {
                            MT_BOOL bInusing =  pstScrollText->stCachebuf[i - 1].bInusing;          
                            mt_u32  u32PhyAddr = pstScrollText->stCachebuf[i - 1].u32PhyAddr;
                            mt_u8   *pVirAddr = pstScrollText->stCachebuf[i - 1].pVirAddr;

                            pstScrollText->stCachebuf[i - 1].bInusing = pstScrollText->stCachebuf[i].bInusing;
                            pstScrollText->stCachebuf[i - 1].u32PhyAddr = pstScrollText->stCachebuf[i].u32PhyAddr;
                            pstScrollText->stCachebuf[i - 1].pVirAddr = pstScrollText->stCachebuf[i].pVirAddr;
                            pstScrollText->stCachebuf[i].bInusing = bInusing;
                            pstScrollText->stCachebuf[i].u32PhyAddr = u32PhyAddr;
                            pstScrollText->stCachebuf[i].pVirAddr = pVirAddr;
                    }
                }

                spin_unlock_irqrestore(&scrolltextLock, flags);	            
                return MT_SUCCESS;
            }
        }
    }

    return MT_FAILURE;
}


/***************************************************************************************
* func			: mtfb_destroy_scrolltext
* description	: 销毁滚动字幕
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 mtfb_destroy_scrolltext(mt_u32 u32LayerID, mt_u32 u32ScrollTextID)
{
	mt_s32 s32Ret;
	MTFB_SCROLLTEXT_S       *pstScrollText;

	if (!s_stTextLayer[u32LayerID].stScrollText[u32ScrollTextID].bAvailable)
	{
	    MTFB_ERROR("invalid scrolltext handle!\n");
    	return MT_FAILURE;
	}

	pstScrollText = &(s_stTextLayer[u32LayerID].stScrollText[u32ScrollTextID]);
	/*wait tde blit job done*/
	if (pstScrollText->s32TdeBlitHandle)
	{
		s32Ret = s_stDrvTdeOps.MTFB_DRV_WaitForDone(pstScrollText->s32TdeBlitHandle, 1000);
        if (s32Ret < 0)
        {
        	MTFB_ERROR("MTFB_DRV_WaitForDone failed!ret=%x\n", s32Ret);
        	return MT_FAILURE;
        }
	}

	mtfb_freescrolltext_cachebuf(pstScrollText);
	s_stTextLayer[u32LayerID].u32textnum--;
	s_stTextLayer[u32LayerID].u32ScrollTextId = u32ScrollTextID;
	memset(pstScrollText,0,sizeof(MTFB_SCROLLTEXT_S));
	
	return MT_SUCCESS;
	
}

/***************************************************************************************
* func			: mtfb_scrolltext_callback
* description	: 滚动字幕回调，TDE blit之后回调的
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
static mt_s32 mtfb_scrolltext_callback(MT_VOID *pParaml, MT_VOID *pParamr)
{
	mt_s32 s32Ret;
    mt_u32 u32TextLayerId, u32TextId, u32Handle;
    MTFB_SCROLLTEXT_S *pstScrollText;
    u32Handle = *(mt_u32 *)pParaml;

	s32Ret = mtfb_parse_scrolltexthandle(u32Handle, &u32TextLayerId, &u32TextId);
	if (MT_SUCCESS != s32Ret)
	{
		MTFB_ERROR("failed to parse the scrolltext handle!\n");
		return MT_FAILURE;
	}
	
    pstScrollText = &(s_stTextLayer[u32TextLayerId].stScrollText[u32TextId]);
	pstScrollText->stCachebuf[0].bInusing = MT_FALSE;
	
    if (pstScrollText->bAvailable)
    {
        pstScrollText->s32TdeBlitHandle = MT_NULL;
        pstScrollText->bBliting         = MT_FALSE;
       /** wake up 数据处理的时候会等这个空闲 **/
       pstScrollText->u32IdleFlag = 1;
       wake_up_interruptible(&(pstScrollText->wbEvent));
    }
    
    return MT_SUCCESS;
}


/***************************************************************************************
* func			: mtfb_scrolltext_blit
* description	: 滚动字幕blit
* param[in] 	:
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 mtfb_scrolltext_blit(mt_u32 u32LayerID)
{
	mt_s32 s32Ret;
    mt_u32 i, j;
    mt_u32 u32StartAddr;    
    MTFB_SCROLLTEXT_INFO_S *pstScrollTextInfo;
    MTFB_SCROLLTEXT_S      *pstScrollText;
	struct fb_info *info; 
    MTFB_PAR_S *pstPar; 
	
	info = s_stLayer[u32LayerID].pstInfo;
	pstPar = (MTFB_PAR_S *)(info->par);
    pstScrollTextInfo = &(s_stTextLayer[u32LayerID]);
    //u32StartAddr = pstPar->stBufInfo.u32DisplayAddr[0];	
    u32StartAddr = pstPar->stRunInfo.u32ScreenAddr;	
	/*blit the cache buffer of scrolltext to the display buffer*/	
    if (s_stTextLayer[u32LayerID].bAvailable)
    {        
        for (i = 0; i < SCROLLTEXT_NUM; i++)
        {
            j = 0;
            pstScrollText = &(pstScrollTextInfo->stScrollText[i]);
            
            if (pstScrollText->bAvailable
				&& !pstScrollText->bPause
				&& pstScrollText->stCachebuf[j].bInusing
				&& !pstScrollText->bBliting)
            {                    	
                MTFB_BUFFER_S stTempBuf, stCanvasBuf;
                MTFB_BLIT_OPT_S stBlitOpt;

                memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));
                
                stCanvasBuf.stCanvas.u32PhyAddr = pstScrollText->stCachebuf[j].u32PhyAddr;
                stCanvasBuf.stCanvas.enFmt      = pstPar->stExtendInfo.enColFmt;					
                stCanvasBuf.stCanvas.u32Width   = pstScrollText->stRect.w;
                stCanvasBuf.stCanvas.u32Height  = pstScrollText->stRect.h;
                stCanvasBuf.stCanvas.u32Pitch   = pstScrollText->u32Stride;
		
                stCanvasBuf.UpdateRect.x        = 0;
                stCanvasBuf.UpdateRect.y        = 0;
                stCanvasBuf.UpdateRect.w        = pstScrollText->stRect.w;
                stCanvasBuf.UpdateRect.h        = pstScrollText->stRect.h;

                stTempBuf.stCanvas.u32PhyAddr   = u32StartAddr;
                stTempBuf.stCanvas.enFmt        = pstPar->stExtendInfo.enColFmt;
                stTempBuf.stCanvas.u32Width     = pstPar->stExtendInfo.u32DisplayWidth;
                stTempBuf.stCanvas.u32Height    = pstPar->stExtendInfo.u32DisplayHeight;
				stTempBuf.stCanvas.u32Pitch   = info->fix.line_length;
				
                stTempBuf.UpdateRect.x          = pstScrollText->stRect.x - pstPar->stExtendInfo.stPos.s32XPos;
                stTempBuf.UpdateRect.y          = pstScrollText->stRect.y - pstPar->stExtendInfo.stPos.s32YPos;;
                stTempBuf.UpdateRect.w          = pstScrollText->stRect.w;
                stTempBuf.UpdateRect.h          = pstScrollText->stRect.h;

                if (stTempBuf.stCanvas.u32Width != stCanvasBuf.stCanvas.u32Width
                    || stTempBuf.stCanvas.u32Height != stCanvasBuf.stCanvas.u32Height)
                {
                    stBlitOpt.bScale = MT_TRUE;
                }

                if (pstScrollText->bDeflicker
                    && pstPar->stBaseInfo.enAntiflickerMode == MTFB_ANTIFLICKER_TDE)
                {
                    stBlitOpt.enAntiflickerLevel = pstPar->stBaseInfo.enAntiflickerLevel;
                }

                //stBlitOpt.bRegionDeflicker = MT_TRUE;
                stBlitOpt.bBlock           = MT_FALSE;
                stBlitOpt.bCallBack        = MT_TRUE;
				stBlitOpt.pfnCallBack      = mtfb_scrolltext_callback;
                stBlitOpt.pParam           = &(pstScrollText->enHandle);
				/** TDE调用完之后会回调 mtfb_scrolltext_callback 这个函数 **/
                s32Ret = s_stDrvTdeOps.MTFB_DRV_Blit(&stCanvasBuf, &stTempBuf, &stBlitOpt, MT_TRUE);
				pstScrollText->bBliting = MT_TRUE;
				if (s32Ret <= 0)
				{
					MTFB_ERROR("mtfb_scrolltext_blit blit err !\n");
                    return MT_FAILURE;
				}

				pstScrollText->s32TdeBlitHandle = s32Ret;
	 
            }
        }
    }

    return MT_SUCCESS;
}
#endif

