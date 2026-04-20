/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __CIRC_BUF_H__
#define __CIRC_BUF_H__

#include "mt_type.h"
#include "mt_debug.h"
#include <linux/string.h>

#include "hal_aoe_func.h"
#include "aud_in_aria_reg.h"

#include "mt_module_debug.h"

#define RIGHT_CH_OFFSET_SRC (AI_SAMPLE_PERFRAME_DF * 2 * AI_BUFF_FRAME_NUM_DF)
#define RIGHT_CH_OFFSET_DST (AI_SAMPLE_PERFRAME_DF * 2)
#if 1

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define SIMPLE_CB_GAP 32


typedef enum {
  AUDIO_BUF_TYPE_NONE = 0,

  AUDIO_BUF_TYPE_PP = 1,
  AUDIO_BUF_TYPE_PCM = 2,
  AUDIO_BUF_TYPE_SPDIF = 3,
  AUDIO_BUF_TYPE_CAST = 4,
  AUDIO_BUF_TYPE_IN = 5,
#ifdef CONFIG_MT_CHIP_SYMPHONY4
  AUDIO_BUF_TYPE_MIX = 6,
#endif  
  AUDIO_BUF_TYPE_MAX = 10
}AudioBufTypeAria;


typedef struct mtCIRC_BUF_S
{
    AudioBufTypeAria  CBType;
    ulong      u32RegBase;
    ulong     *pu32Write;  //存放读指针偏移量地址
    ulong      pu32Write_cnt_reg;  //存放读指针偏移量地址
    ulong      cmd_reg;
    ulong      *pu32Read;   //存放写指针偏移量地址
    mt_u8      *pu8Data;   //DDR缓存起始地址(虚拟地址)
    mt_u32     u32Lenght;   //循环buffer长度
    mt_u32     info;
} CIRC_BUF_S;

mt_void CIRC_BUF_Init(CIRC_BUF_S *pstCb,
                        //AudioBufTypeAria CBType,
                        ulong u32RegBase,
                        ulong pu8Data,
                        mt_u32  u32Len);


static inline mt_void CIRC_BUF_DeInit(CIRC_BUF_S *pstCb)
{
    return ;
}

static inline mt_u32 CIRC_BUF_QueryBusy_ProvideRptr(CIRC_BUF_S *pstCb, mt_u32 *pu32Rptr)
{
    ulong  u32ReadPos, u32WritePos, u32BusyLen=0;

    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos  = (ulong)(pstCb->pu32Read);         //取出寄存器地址里面的内容
    u32WritePos = (ulong)(pstCb->pu32Write);

    u32ReadPos = *pu32Rptr;

    if (u32WritePos >= u32ReadPos)
    {
        u32BusyLen = u32WritePos - u32ReadPos;
    }
    else
    {
        u32BusyLen = pstCb->u32Lenght - (u32ReadPos - u32WritePos);
    }

    return u32BusyLen;
}

static inline mt_u32 CIRC_BUF_QueryBusy(CIRC_BUF_S *pstCb)
{
    ulong  u32ReadPos, u32WritePos, u32BusyLen=0;

    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos  = (ulong)(pstCb->pu32Read);         //取出寄存器地址里面的内容
    u32WritePos = (ulong)(pstCb->pu32Write);


    if (u32WritePos >= u32ReadPos)
    {
        u32BusyLen = u32WritePos - u32ReadPos;
    }
    else
    {
        u32BusyLen = pstCb->u32Lenght - (u32ReadPos - u32WritePos);
    }
//MT_INFO_AIAO("minnan CIRC_BUF_QueryBusy w 0x%x, r 0x%x, data 0x%x, len 0x%x\n",
  //pstCb->pu32Write, pstCb->pu32Read, pstCb->pu8Data, pstCb->u32Lenght);

    return u32BusyLen;
}

static inline mt_u32 CIRC_BUF_QueryFree(CIRC_BUF_S *pstCb)
{
  mt_u32  u32FreeLen=0;
  #if 1  //cooper

  if(pstCb->pu32Write_cnt_reg)
  {
    u32FreeLen = pstCb->u32Lenght - ((*((volatile mt_u32 *)(pstCb->pu32Write_cnt_reg))) << 3);
  }

  #else
    mt_u32  u32ReadPos, u32WritePos;

    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos  = (pstCb->pu32Read);
    u32WritePos = (pstCb->pu32Write);

    if (u32WritePos >= u32ReadPos)
    {
        u32FreeLen = (pstCb->u32Lenght - (u32WritePos - u32ReadPos));
    }
    else
    {
        u32FreeLen = (u32ReadPos - u32WritePos);
    }

    u32FreeLen = (u32FreeLen <= SIMPLE_CB_GAP) ? 0 : u32FreeLen - SIMPLE_CB_GAP;
    /* 循环Buffer避免写满，否则读方向认为Buffer为空 */
    if(u32FreeLen>0)
        u32FreeLen -= 1;

//MT_INFO_AIAO("minnan CIRC_BUF_QueryFree w 0x%x, r 0x%x, data 0x%x, len 0x%x\n",
  //pstCb->pu32Write, pstCb->pu32Read, pstCb->pu8Data, pstCb->u32Lenght);
    #endif
    return u32FreeLen;
}

static inline mt_u32 CIRC_BUF_QueryReadPos(CIRC_BUF_S *pstCb)
{
    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);

    return *(pstCb->pu32Read);
}

/*****************************************************************************
 Prototype       : CIRC_BUF_Read_NotUpRptr
 Description     : 从共享CB中读取指定长度的数据
 Input           : pstCb     ** CB的管理结构
                   pu32DataOffset   ** 获取数据的偏移
                   u32Len  ** 希望读取的数据长度
 Output          : None
 Return Value    : 返回成功读取到的数据长度
*****************************************************************************/

static inline mt_u32 CIRC_BUF_Read_NotUpRptr(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len, ulong *pu32Rptr, ulong *pu32Wptr)
{
    mt_u8  *pVirAddr   = NULL;
    ulong  u32RdLen[2]={0,0}, u32RdPos[2]={0,0}, i;
    ulong  u32ReadPos, u32WritePos;

    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);
    //MT_ASSERT(NULL != pDest);

#if 0
    MT_INFO_AIAO("pstCb->pu32Read = 0x%x, ", *(pstCb->pu32Read));
    MT_INFO_AIAO("pstCb->pu32Write = 0x%x, ", *(pstCb->pu32Write));
    MT_INFO_AIAO("pstCb->pu8Data = 0x%x, ", *(pstCb->pu8Data));
    MT_INFO_AIAO("pstCb->u32Lenght = %d, ", pstCb->u32Lenght);
#endif

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos  = (ulong)(pu32Rptr);
    u32WritePos = (ulong)(pstCb->pu32Write);

    u32RdPos[0] = u32ReadPos;
    if (u32WritePos >= u32ReadPos)
    {
        if (u32WritePos >= u32ReadPos + u32Len)
        {
            /* 如果剩余数据较多，则只读取一部分 */
            u32RdLen[0] = u32Len;
        }
        else
        {
            /* 如果剩余数据不足，则全部读走 */
            u32RdLen[0] = u32WritePos - u32ReadPos;
        }
    }
    else
    {
        if ( u32ReadPos + u32Len <= pstCb->u32Lenght)
        {
            /* 如果不需要折返，则直接读取所需的数据 */
            u32RdLen[0] = u32Len;
        }
        else
        {
            /* 如果需要折返，则先读取尾部的数据 */
            u32RdLen[0] = pstCb->u32Lenght - u32ReadPos;

            /* 重新折返到头，读取剩余的部分数据 */
            u32RdPos[1] = 0;
            u32RdLen[1] = u32Len - u32RdLen[0];
            if(u32WritePos < u32RdLen[1])
            {
                /* 如果剩余数据不足，则读取全部数据 */
                u32RdLen[1] = u32WritePos;
            }
        }
    }

    for (i=0; ( i < 2 ) && (u32RdLen[i] != 0); i++)
    {
        pVirAddr = (mt_u8*)(pstCb->pu8Data + u32RdPos[i]);

        memcpy(pDest, pVirAddr, u32RdLen[i]);

        pDest += u32RdLen[i];

        u32ReadPos = u32RdPos[i] + u32RdLen[i];
    }

    /* 将读指针写回Buffer头 */
    if (u32ReadPos == pstCb->u32Lenght) u32ReadPos = 0;

    //not update Rptr
    //*(pstCb->pu32Read) = u32ReadPos;

    pu32Wptr= (ulong*)u32WritePos;
    pu32Rptr = (ulong*)u32ReadPos;

    /* 返回实际读取的长度 */
    return u32RdLen[0] + u32RdLen[1];
}

/*****************************************************************************
 Prototype       : CIRC_BUF_Read
 Description     : 从共享CB中读取指定长度的数据
 Input           : pstCb     ** CB的管理结构
                   pDest   ** 用户空间申请内存后虚拟地址(文件)
                   u32Len  ** 希望读取的数据长度
 Output          : None
 Return Value    : 返回成功读取到的数据长度
*****************************************************************************/
static inline mt_u32 CIRC_BUF_Read_Vsb_interlace(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len)
{
     mt_u8 *pVirAddr = NULL;
    ulong u32RdLen[2] = { 0, 0 }, u32RdPos[2] = { 0, 0 }, i;
    ulong u32ReadPos, u32WritePos;
    //mt_u32 rightchoffsetdst = u32Len;	//unused
    MT_INFO_AIAO("CIRC_BUF_Read_Vsb_interlace \n ");


    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos = (ulong)(pstCb->pu32Read);
    u32WritePos = (ulong)(pstCb->pu32Write);

    u32RdPos[0] = u32ReadPos;
    if (u32WritePos >= u32ReadPos) {
	if (u32WritePos >= u32ReadPos + u32Len) {
	    /* 如果剩余数据较多，则只读取一部分 */
	    u32RdLen[0] = u32Len;
	} else {
	    /* 如果剩余数据不足，则全部读走 */
	    MT_INFO_AIAO("111111111111 data not enough! \n");
	    u32RdLen[0] = u32WritePos - u32ReadPos;
	}
    } else {
	if (u32ReadPos + u32Len <= pstCb->u32Lenght) {
	    /* 如果不需要折返，则直接读取所需的数据 */
	    u32RdLen[0] = u32Len;
	} else {
	    /* 如果需要折返，则先读取尾部的数据 */
	    u32RdLen[0] = pstCb->u32Lenght - u32ReadPos;

	    /* 重新折返到头，读取剩余的部分数据 */
	    u32RdPos[1] = 0;
	    u32RdLen[1] = u32Len - u32RdLen[0];
	    if (u32WritePos < u32RdLen[1]) {
		/* 如果剩余数据不足，则读取全部数据 */
		MT_INFO_AIAO("22222222222222 data not enough! \n");
		u32RdLen[1] = u32WritePos;
	    }
	}
    }

    for (i = 0; (i < 2) && (u32RdLen[i] != 0); i++) {
	pVirAddr = (mt_u8 *)(pstCb->pu8Data + u32RdPos[i]);
	memcpy(pDest, pVirAddr, u32RdLen[i]);

	pDest += u32RdLen[i];
	u32ReadPos = u32RdPos[i] + u32RdLen[i];
    }

    /* 将读指针写回Buffer头 */
    if (u32ReadPos == pstCb->u32Lenght)
	u32ReadPos = 0;

    (pstCb->pu32Read) = (ulong*)u32ReadPos;

    //MT_INFO_AIAO("CIRC_BUF_Read w 0x%x, r 0x%x, data 0x%x, len 0x%x  dstaddr 0x%x  value 0x%x\n",
    // pstCb->pu32Write, pstCb->pu32Read, pstCb->pu8Data, pstCb->u32Lenght, test_temp,*test_temp);

    /* 返回实际读取的长度 */
    return u32RdLen[0] + u32RdLen[1];
}
static inline mt_u32 CIRC_BUF_Read_Vsb(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len)
{
    mt_u8 *pVirAddr = NULL;
    ulong u32RdLen[2] = { 0, 0 }, u32RdPos[2] = { 0, 0 }, i;
    ulong u32ReadPos, u32WritePos;
    ulong rightchoffsetsrc = pstCb->u32Lenght;
    ulong rightchoffsetdst = u32Len;
//TODO++++++++++++++
//MT_ASSERT(NULL != pstCb);
//MT_ASSERT(NULL != pDest);

#if 0
    MT_INFO_AIAO("pstCb->pu32Read = 0x%x, ", *(pstCb->pu32Read));
    MT_INFO_AIAO("pstCb->pu32Write = 0x%x, ", *(pstCb->pu32Write));
    MT_INFO_AIAO("pstCb->pu8Data = 0x%x, ", *(pstCb->pu8Data));
    MT_INFO_AIAO("pstCb->u32Lenght = %d, ", pstCb->u32Lenght);
#endif

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos = (ulong)(pstCb->pu32Read);
    u32WritePos = (ulong)(pstCb->pu32Write);

    u32RdPos[0] = u32ReadPos;
    if (u32WritePos >= u32ReadPos) {
	if (u32WritePos >= u32ReadPos + u32Len) {
	    /* 如果剩余数据较多，则只读取一部分 */
	    u32RdLen[0] = u32Len;
	} else {
	    /* 如果剩余数据不足，则全部读走 */
	    MT_INFO_AIAO("111111111111 data not enough! \n");
	    u32RdLen[0] = u32WritePos - u32ReadPos;
	}
    } else {
	if (u32ReadPos + u32Len <= pstCb->u32Lenght) {
	    /* 如果不需要折返，则直接读取所需的数据 */
	    u32RdLen[0] = u32Len;
	} else {
	    /* 如果需要折返，则先读取尾部的数据 */
	    u32RdLen[0] = pstCb->u32Lenght - u32ReadPos;

	    /* 重新折返到头，读取剩余的部分数据 */
	    u32RdPos[1] = 0;
	    u32RdLen[1] = u32Len - u32RdLen[0];
	    if (u32WritePos < u32RdLen[1]) {
		/* 如果剩余数据不足，则读取全部数据 */
		MT_INFO_AIAO("22222222222222 data not enough! \n");
		u32RdLen[1] = u32WritePos;
	    }
	}
    }

    for (i = 0; (i < 2) && (u32RdLen[i] != 0); i++) {
	pVirAddr = (mt_u8 *)(pstCb->pu8Data + u32RdPos[i]);
	memcpy(pDest, pVirAddr, u32RdLen[i]);

	pVirAddr = (mt_u8 *)(pstCb->pu8Data + u32RdPos[i] + rightchoffsetsrc);
	memcpy(pDest + rightchoffsetdst, pVirAddr, u32RdLen[i]);

	pDest += u32RdLen[i];
	u32ReadPos = u32RdPos[i] + u32RdLen[i];
    }

    /* 将读指针写回Buffer头 */
    if (u32ReadPos == pstCb->u32Lenght)
	u32ReadPos = 0;

    (pstCb->pu32Read) = (ulong*)u32ReadPos;

    //MT_INFO_AIAO("CIRC_BUF_Read w 0x%x, r 0x%x, data 0x%x, len 0x%x  dstaddr 0x%x  value 0x%x\n",
    // pstCb->pu32Write, pstCb->pu32Read, pstCb->pu8Data, pstCb->u32Lenght, test_temp,*test_temp);

    /* 返回实际读取的长度 */
    return u32RdLen[0] + u32RdLen[1];
}

static inline mt_u32 CIRC_BUF_Read(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len)
{
    mt_u8 *pVirAddr = NULL;
    ulong u32RdLen[2] = { 0, 0 }, u32RdPos[2] = { 0, 0 }, i;
    ulong u32ReadPos, u32WritePos;
   // ulong *test_temp = (ulong*)(pDest);
//TODO++++++++++++++
//MT_ASSERT(NULL != pstCb);
//MT_ASSERT(NULL != pDest);

#if 0
    MT_INFO_AIAO("pstCb->pu32Read = 0x%x, ", *(pstCb->pu32Read));
    MT_INFO_AIAO("pstCb->pu32Write = 0x%x, ", *(pstCb->pu32Write));
    MT_INFO_AIAO("pstCb->pu8Data = 0x%x, ", *(pstCb->pu8Data));
    MT_INFO_AIAO("pstCb->u32Lenght = %d, ", pstCb->u32Lenght);
#endif

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos = (ulong)(pstCb->pu32Read);
    u32WritePos = (ulong)(pstCb->pu32Write);

    u32RdPos[0] = u32ReadPos;
    if (u32WritePos >= u32ReadPos) {
	if (u32WritePos >= u32ReadPos + u32Len) {
	    /* 如果剩余数据较多，则只读取一部分 */
	    u32RdLen[0] = u32Len;
	} else {
	    /* 如果剩余数据不足，则全部读走 */
	    u32RdLen[0] = u32WritePos - u32ReadPos;
	}
    } else {
	if (u32ReadPos + u32Len <= pstCb->u32Lenght) {
	    /* 如果不需要折返，则直接读取所需的数据 */
	    u32RdLen[0] = u32Len;
	} else {
	    /* 如果需要折返，则先读取尾部的数据 */
	    u32RdLen[0] = pstCb->u32Lenght - u32ReadPos;

	    /* 重新折返到头，读取剩余的部分数据 */
	    u32RdPos[1] = 0;
	    u32RdLen[1] = u32Len - u32RdLen[0];
	    if (u32WritePos < u32RdLen[1]) {
		/* 如果剩余数据不足，则读取全部数据 */
		u32RdLen[1] = u32WritePos;
	    }
	}
    }

    for (i = 0; (i < 2) && (u32RdLen[i] != 0); i++) {
	pVirAddr = (mt_u8 *)(pstCb->pu8Data + u32RdPos[i]);

	memcpy(pDest, pVirAddr, u32RdLen[i]);

	pDest += u32RdLen[i];

	u32ReadPos = u32RdPos[i] + u32RdLen[i];
    }

    /* 将读指针写回Buffer头 */
    if (u32ReadPos == pstCb->u32Lenght)
	u32ReadPos = 0;

    (pstCb->pu32Read) = (ulong*)u32ReadPos;
 //   MT_INFO_AIAO("minnan CIRC_BUF_Read w 0x%x, r 0x%x, data 0x%x, len 0x%x value 0x%x read addr 0x%x\n",
 //          pstCb->pu32Write, pstCb->pu32Read, pstCb->pu8Data, pstCb->u32Lenght, *test_temp, (mt_u32)test_temp);

    /* 返回实际读取的长度 */
    return u32RdLen[0] + u32RdLen[1];
}

#ifdef MT_SND_CAST_SUPPORT
/*****************************************************************************
 Prototype       : CIRC_BUF_CastRead
 Description     : 从共享CB中读取指定长度的数据
 Input           : pstCb     ** CB的管理结构
                   pu32DataOffset   ** 获取数据的偏移
                   u32Len  ** 希望读取的数据长度
 Output          : None
 Return Value    : 返回成功读取到的数据长度
*****************************************************************************/
static inline mt_u32 CIRC_BUF_CastRead(CIRC_BUF_S *pstCb, mt_u32 *pu32DataOffset, mt_u32 u32Len)
{
    ulong  u32RdLen[2]={0,0}, u32RdPos[2]={0,0};
    ulong  u32ReadPos, u32WritePos;

#if 0
    MT_INFO_AIAO("pstCb->pu32Read = 0x%x, ", *(pstCb->pu32Read));
    MT_INFO_AIAO("pstCb->pu32Write = 0x%x, ", *(pstCb->pu32Write));
    MT_INFO_AIAO("pstCb->pu8Data = 0x%x, ", *(pstCb->pu8Data));
    MT_INFO_AIAO("pstCb->u32Lenght = %d, ", pstCb->u32Lenght);
#endif

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos  = (ulong)(pstCb->pu32Read);
    u32WritePos = (ulong)(pstCb->pu32Write);

    u32RdPos[0] = u32ReadPos;
    if (u32WritePos >= u32ReadPos)
    {
        if (u32WritePos >= u32ReadPos + u32Len)
        {
            /* 如果剩余数据较多，则只读取一部分 */
            u32RdLen[0] = u32Len;
        }
        else
        {
			*pu32DataOffset = 0;
			return  0;
        }
    }
    else
    {
        if ( u32ReadPos + u32Len <= pstCb->u32Lenght)
        {
            /* 如果不需要折返，则直接读取所需的数据 */
            u32RdLen[0] = u32Len;
        }
        else
        {
            /* 如果需要折返，则先读取尾部的数据 */
            u32RdLen[0] = pstCb->u32Lenght - u32ReadPos;

            /* 重新折返到头，读取剩余的部分数据 */
            u32RdPos[1] = 0;
            u32RdLen[1] = u32Len - u32RdLen[0];
            if(u32WritePos < u32RdLen[1])
            {
                /* 如果剩余数据不足，则读取全部数据 */
                *pu32DataOffset = 0;
                return 0;
            }
        }
    }

    *pu32DataOffset = u32RdPos[0];
    //MT_INFO_AIAO("\npu32DataOffset  0x%x  \n", u32RdPos[0]);
    return u32RdLen[0] + u32RdLen[1];
}

/*****************************************************************************
 Prototype       : CIRC_BUF_CastRelese
 Description     : 从共享CB中释放指定长度的数据
 Input           : pstCb     ** CB的管理结构
                   u32Len  ** 希望释放的数据长度
 Output          : None
 Return Value    : 返回成功释放到的数据长度
*****************************************************************************/
static inline mt_u32 CIRC_BUF_CastRelese(CIRC_BUF_S *pstCb, mt_u32 u32Len)
{
    ulong  u32RdLen[2]={0,0}, u32RdPos[2]={0,0}, i;
    ulong  u32ReadPos, u32WritePos;

    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);
    //MT_ASSERT(NULL != pDest);

#if 0
    MT_INFO_AIAO("pstCb->pu32Read = 0x%x, ", *(pstCb->pu32Read));
    MT_INFO_AIAO("pstCb->pu32Write = 0x%x, ", *(pstCb->pu32Write));
    MT_INFO_AIAO("pstCb->pu8Data = 0x%x, ", *(pstCb->pu8Data));
    MT_INFO_AIAO("pstCb->u32Lenght = %d, ", pstCb->u32Lenght);
#endif

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos  = (ulong)(pstCb->pu32Read);
    u32WritePos = (ulong)(pstCb->pu32Write);
    u32RdPos[0] = u32ReadPos;
    if (u32WritePos >= u32ReadPos)
    {
        if (u32WritePos >= u32ReadPos + u32Len)
        {
            /* 如果剩余数据较多，则只读取一部分 */
            u32RdLen[0] = u32Len;
        }
        else
        {
            /* 如果剩余数据不足，则全部读走 */
            u32RdLen[0] = u32WritePos - u32ReadPos;
        }
    }
    else
    {

        if ( u32ReadPos + u32Len <= pstCb->u32Lenght)
        {
            /* 如果不需要折返，则直接读取所需的数据 */
            u32RdLen[0] = u32Len;
        }
        else
        {
            /* 如果需要折返，则先读取尾部的数据 */
            u32RdLen[0] = pstCb->u32Lenght - u32ReadPos;
            //MT_INFO_AIAO("\nCIRC_BUF_CastRelese    7 \n");
            /* 重新折返到头，读取剩余的部分数据 */
            u32RdPos[1] = 0;
            u32RdLen[1] = u32Len - u32RdLen[0];
            if(u32WritePos < u32RdLen[1])
            {
                /* 如果剩余数据不足，则读取全部数据 */
                u32RdLen[1] = u32WritePos;
            }
        }
    }

    for (i=0; ( i < 2 ) && (u32RdLen[i] != 0); i++)
    {
        //pVirAddr = (mt_u8*)(pstCb->pu8Data + u32RdPos[i]);
        //memcpy(pDest, pVirAddr, u32RdLen[i]);
        //pDest += u32RdLen[i];
        u32ReadPos = u32RdPos[i] + u32RdLen[i];
    }

    /* 将读指针写回Buffer头 */
    if (u32ReadPos == pstCb->u32Lenght) u32ReadPos = 0;

    (pstCb->pu32Read) = (ulong*)u32ReadPos;
    //MT_INFO_AIAO(" release : 0x%x\n", (u32RdLen[0] + u32RdLen[1]));
    /* 返回实际读取的长度 */
    return u32RdLen[0] + u32RdLen[1];
}
#endif
extern ulong  g_audio_reg_base ;

static inline mt_u32 get_chan_by_chan_exist(mt_u32 chan_exist)
{
  mt_u32 chan = 0;
  int i = 0;
  for(i = 0;i < 8;i++)
  {
    if((1 << i) & chan_exist)
      chan += 1;
  }
  return chan;
}

static inline mt_u32 CIRC_BUF_Write_Vsb(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len)
{
    ulong *pVirAddr = NULL;

    ulong u32WtLen[2] = { 0, 0 };
    ulong u32WtPos[2] = { 0, 0 };
    mt_u32 i;
    ulong u32ReadPos, u32WritePos;

    // mt_u32 *test_temp = (mt_u32)(pDest); //not need

    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);
    //MT_ASSERT(NULL != pDest);

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos = (ulong)(pstCb->pu32Read);
    u32WritePos = (ulong)(pstCb->pu32Write);

    u32WtPos[0] = u32WritePos;

    if (u32WritePos == u32ReadPos) {
	MT_INFO_AIAO("!!!!!!!!!!!!!!!read maybe slower !\n");
    }

    if (u32WritePos >= u32ReadPos) {
	if (pstCb->u32Lenght >= (u32WritePos + u32Len)) {
	    u32WtLen[0] = u32Len;
	} else {
	    u32WtLen[0] = pstCb->u32Lenght - u32WritePos;
	    u32WtLen[1] = u32Len - u32WtLen[0];

	    u32WtPos[1] = 0;
	}

    } else {
	u32WtLen[0] = u32Len;
    }

    for (i = 0; (i < 2) && (u32WtLen[i] != 0); i++) {
	//hw write data,just need update writepos
	pVirAddr = (ulong *)((ulong)pstCb->pu8Data + u32WtPos[i]);

	u32WritePos = u32WtPos[i] + u32WtLen[i];
    }

    /* 将读指针写回Buffer头 */
    if (u32WritePos == pstCb->u32Lenght)
	u32WritePos = 0;

    (pstCb->pu32Write) = (ulong*)u32WritePos;

    //MT_INFO_AIAO("minnan CIRC_BUF_Write w 0x%x, r 0x%x, data 0x%x, len 0x%x \n",
    //pstCb->pu32Write, pstCb->pu32Read, pstCb->pu8Data, pstCb->u32Lenght);

    /* 返回实际读取的长度 */
    return u32WtLen[0] + u32WtLen[1];
}

//extern void __cpuc_flush_dcache_area(void *, size_t);
mt_u32 CIRC_BUF_Write(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len, mt_u32 ChanExist);
#if 0
static inline mt_u32 CIRC_BUF_Write(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len)
//  mt_u32 CIRC_BUF_Write(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len)
{
  #if 1

  #if 1 //pp way
  int i;
  mt_u32 cpy_all = pstCb->u32Lenght - ((*(pstCb->pu32Write_cnt_reg)) << 3);
  mt_u32 write_pnt = pstCb->pu32Write;
  mt_u32 cpy1,cpy2;
  mt_u32 chan_exist = 0;
  mt_u32 chan = 0;

  reg_aud_snt_ch_srt_t *p_chan_reg = g_audio_reg_base;
  chan_exist = p_chan_reg->bitc.ch_input_mode;
  chan = get_chan_by_chan_exist(chan_exist);

  u32Len /= chan;
  if(cpy_all > u32Len)
    cpy_all = u32Len;

  //(*(mt_u32 *)pstCb->cmd_reg) = (cpy_all >> 3) | 0x80000000 ;

  return cpy_all;

  if((write_pnt + cpy_all) > pstCb->u32Lenght)
  {
    cpy1 = pstCb->u32Lenght - write_pnt;
    cpy2 = cpy_all - cpy1;
  }
  else
  {
    cpy1 = cpy_all;
    cpy2 = 0;
  }

  for(i = 0;i < 8;i ++)
  {
    if((1 << i) & chan_exist)
    {
      if(cpy2)
      {
        mt_u32 dest = pstCb->pu8Data + write_pnt + i * pstCb->u32Lenght;
        mt_u32 src = pDest + i * u32Len;

        memcpy(dest, src, cpy1);
       // __cpuc_flush_dcache_area((void *)(dest), cpy1);

        dest = pstCb->pu8Data + i * pstCb->u32Lenght;
        src = pDest + i * u32Len + cpy1;
        memcpy(dest, src, cpy2);
       // __cpuc_flush_dcache_area((void *)(dest), cpy2);
      }
      else
      {
        mt_u32 dest = pstCb->pu8Data + write_pnt + i * pstCb->u32Lenght;
        mt_u32 src = pDest + i * u32Len;
        memcpy(dest, src, cpy_all);
       // __cpuc_flush_dcache_area((void *)(dest), cpy_all);
      }

       (*(mt_u32 *)pstCb->cmd_reg) = (cpy_all >> 3) | 0x80000000 | (i << 24);
    }
  }

  if(cpy2)
  {
    write_pnt = cpy2;
  }
  else
  {
    write_pnt += cpy_all;
    write_pnt %= pstCb->u32Lenght;
  }

  pstCb->pu32Write = write_pnt;

  return cpy_all;
  #else //pcm way
  mt_u32 cpy_all = pstCb->u32Lenght - ((*(pstCb->pu32Write_cnt_reg)) << 3);
  mt_u32 write_pnt = pstCb->pu32Write;

  if(cpy_all > u32Len)
    cpy_all = u32Len;

  if((write_pnt + cpy_all) > pstCb->u32Lenght)
  {
    mt_u32 cpy1,cpy2;
    cpy1 = pstCb->u32Lenght - write_pnt;
    cpy2 = cpy_all - cpy1;
    memcpy((pstCb->pu8Data + write_pnt), pDest, cpy1);
    memcpy((pstCb->pu8Data), (pDest + cpy1), cpy2);
    write_pnt = cpy2;
  }
  else
  {
    memcpy((pstCb->pu8Data + write_pnt), pDest, cpy_all);
    write_pnt += cpy_all;
    write_pnt %= pstCb->u32Lenght;
  }
  pstCb->pu32Write = write_pnt;

  (*(u32 *)pstCb->cmd_reg) = (cpy_all >> 3) | 0x80000000;

  return cpy_all;

  #endif

  #else
    mt_u32  *pVirAddr   = NULL;

    mt_u32  u32WtLen[2]={0,0};
    mt_u32  u32WtPos[2]={0,0};
    mt_u32  i;
    mt_u32  u32ReadPos, u32WritePos;
    mt_u32 *test_temp = (mt_u32)(pDest);

    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);
    //MT_ASSERT(NULL != pDest);
    return  u32Len;

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos  = (pstCb->pu32Read);
    u32WritePos = (pstCb->pu32Write);

    u32WtPos[0] = u32WritePos;
    if (u32WritePos >= u32ReadPos)
    {
        if (pstCb->u32Lenght >= (u32WritePos + u32Len))
        {
            u32WtLen[0] = u32Len;
        }
        else
        {
            u32WtLen[0] = pstCb->u32Lenght - u32WritePos;
            u32WtLen[1] = u32Len - u32WtLen[0];

            u32WtPos[1] = 0;
        }
    }
    else
    {
        u32WtLen[0] = u32Len;
    }

    for (i=0; ( i < 2 ) && (u32WtLen[i] != 0); i++)
    {
        pVirAddr = (mt_u32 *)((mt_u32)pstCb->pu8Data + u32WtPos[i]);
        if(pDest)
        {
            memcpy(pVirAddr, pDest, u32WtLen[i]);
            pDest = pDest+ u32WtLen[i];
        }
        else //only passthrouth send 0
        {
            memset(pVirAddr, 0, u32WtLen[i]);
        }
        u32WritePos = u32WtPos[i] + u32WtLen[i];
    }

    /* 将读指针写回Buffer头 */
    if (u32WritePos == pstCb->u32Lenght) u32WritePos = 0;
    (pstCb->pu32Write) = u32WritePos;

MT_INFO_AIAO("minnan CIRC_BUF_Write w 0x%x, r 0x%x, data 0x%x, len 0x%x value 0x%x start value 0x%x\n",
  pstCb->pu32Write, pstCb->pu32Read, pstCb->pu8Data, pstCb->u32Lenght, *test_temp, *(pstCb->pu8Data));

    /* 返回实际读取的长度 */
    return u32WtLen[0] + u32WtLen[1];
    #endif
}
#endif


static inline mt_void CIRC_BUF_Flush(CIRC_BUF_S *pstCb)
{
    //*pstCb->pu32Write     = 0;
    //*pstCb->pu32Read      = 0;
}


static inline mt_u32 CIRC_BUF_UpdateRptr(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len)
{
    ulong  u32RdLen[2]={0,0}, u32RdPos[2]={0,0}, i;
    ulong  u32ReadPos, u32WritePos;


    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos  = (ulong)(pstCb->pu32Read);
    u32WritePos = (ulong)(pstCb->pu32Write);

    u32RdPos[0] = u32ReadPos;
    if (u32WritePos >= u32ReadPos)
    {
        if (u32WritePos >= u32ReadPos + u32Len)
        {
            /* 如果剩余数据较多，则只读取一部分 */
            u32RdLen[0] = u32Len;
        }
        else
        {
            /* 如果剩余数据不足，则全部读走 */
            u32RdLen[0] = u32WritePos - u32ReadPos;
        }
    }
    else
    {
        if ( u32ReadPos + u32Len <= pstCb->u32Lenght)
        {
            /* 如果不需要折返，则直接读取所需的数据 */
            u32RdLen[0] = u32Len;
        }
        else
        {
            /* 如果需要折返，则先读取尾部的数据 */
            u32RdLen[0] = pstCb->u32Lenght - u32ReadPos;

            /* 重新折返到头，读取剩余的部分数据 */
            u32RdPos[1] = 0;
            u32RdLen[1] = u32Len - u32RdLen[0];
            if(u32WritePos < u32RdLen[1])
            {
                /* 如果剩余数据不足，则读取全部数据 */
                u32RdLen[1] = u32WritePos;
            }
        }
    }

    for (i=0; ( i < 2 ) && (u32RdLen[i] != 0); i++)
    {


        u32ReadPos = u32RdPos[i] + u32RdLen[i];
    }

    /* 将读指针写回Buffer头 */
    if (u32ReadPos == pstCb->u32Lenght) u32ReadPos = 0;

    (pstCb->pu32Read) = (ulong*)u32ReadPos;

    /* 返回实际读取的长度 */
    return u32RdLen[0] + u32RdLen[1];
}

static inline mt_u32 CIRC_BUF_UpdateWptr(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len)
{

    mt_u32  u32WtLen[2]={0,0};
    mt_u32  u32WtPos[2]={0,0};
    mt_u32  i;
    ulong  u32ReadPos, u32WritePos;

    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos  = (ulong)(pstCb->pu32Read);
    u32WritePos = (ulong)(pstCb->pu32Write);

    u32WtPos[0] = u32WritePos;
    if (u32WritePos >= u32ReadPos)
    {
        if (pstCb->u32Lenght >= (u32WritePos + u32Len))
        {
            u32WtLen[0] = u32Len;
        }
        else
        {
            u32WtLen[0] = pstCb->u32Lenght - u32WritePos;
            u32WtLen[1] = u32Len - u32WtLen[0];

            u32WtPos[1] = 0;
        }
    }
    else
    {
        u32WtLen[0] = u32Len;
    }

    for (i=0; ( i < 2 ) && (u32WtLen[i] != 0); i++)
    {


        u32WritePos = u32WtPos[i] + u32WtLen[i];
    }

    /* 将读指针写回Buffer头 */
    if (u32WritePos == pstCb->u32Lenght) u32WritePos = 0;
    (pstCb->pu32Write) = (ulong*)u32WritePos;

    /* 返回实际读取的长度 */
    return u32WtLen[0] + u32WtLen[1];
}


static inline mt_u32 CIRC_BUF_ALSA_UpdateWptr(CIRC_BUF_S *pstCb, mt_u32 u32Len)
{
    mt_u32  u32WtLen[2]={0,0};
#if 0
    mt_u32  u32WtPos[2]={0,0};
    mt_u32  i;
#endif
    ulong  u32ReadPos, u32WritePos;

    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);

    /* 先从共享内存中取出头信息，避免使用过程中发生变化 */
    u32ReadPos  = (ulong)(pstCb->pu32Read);
    u32WritePos = (ulong)(pstCb->pu32Write);
 #if 1
    u32WritePos += u32Len;
    //MT_INFO_AIAO("\n u32WritePos=0x%x, pstCb->u32Lenght= 0x%x\n",u32WritePos,pstCb->u32Lenght);
    if (u32WritePos > pstCb->u32Lenght)
            u32WritePos -= pstCb->u32Lenght;
#else

    u32WtPos[0] = u32WritePos;
    if (u32WritePos >= u32ReadPos)
    {
        if (pstCb->u32Lenght >= (u32WritePos + u32Len))
        {
            u32WtLen[0] = u32Len;
        }
        else
        {
            u32WtLen[0] = pstCb->u32Lenght - u32WritePos;
            u32WtLen[1] = u32Len - u32WtLen[0];

            u32WtPos[1] = 0;
        }
    }
    else
    {
        u32WtLen[0] = u32Len;
    }

    for (i=0; ( i < 2 ) && (u32WtLen[i] != 0); i++)
    {


        u32WritePos = u32WtPos[i] + u32WtLen[i];
    }

    /* 将读指针写回Buffer头 */
    if (u32WritePos == pstCb->u32Lenght) u32WritePos = 0;
#endif
    *(pstCb->pu32Write) = u32WritePos;	//???

    //MT_INFO_AIAO("\n\n  u32WritePos=0x%x   \n\n", u32WritePos);

    /* 返回实际读取的长度 */
    return u32WtLen[0] + u32WtLen[1];
}


static inline mt_u32 CIRC_BUF_ALSA_UpdateRptr(CIRC_BUF_S *pstCb, mt_u32 u32Len)
{
    ulong  u32ReadPos, u32WritePos;
    u32ReadPos  = (ulong)(pstCb->pu32Read);
    u32WritePos = (ulong)(pstCb->pu32Write);
#if 1
  u32ReadPos += u32Len;
  if (u32ReadPos > pstCb->u32Lenght)
	  u32ReadPos -= pstCb->u32Lenght;
#else
    u32RdPos[0] = u32ReadPos;
    if (u32WritePos >= u32ReadPos)
    {
        if (u32WritePos >= u32ReadPos + u32Len)
        {
            u32RdLen[0] = u32Len;
        }
        else
        {
            u32RdLen[0] = u32WritePos - u32ReadPos;
        }
    }
    else
    {
        if ( u32ReadPos + u32Len <= pstCb->u32Lenght)
        {
            u32RdLen[0] = u32Len;
        }
        else
        {
            u32RdLen[0] = pstCb->u32Lenght - u32ReadPos;
            u32RdPos[1] = 0;
            u32RdLen[1] = u32Len - u32RdLen[0];
            if(u32WritePos < u32RdLen[1])
            {
                u32RdLen[1] = u32WritePos;
            }
        }
    }
    for (i=0; ( i < 2 ) && (u32RdLen[i] != 0); i++)
    {
        u32ReadPos = u32RdPos[i] + u32RdLen[i];
    }
    if (u32ReadPos == pstCb->u32Lenght) u32ReadPos = 0;
   #endif
    (pstCb->pu32Read) = (ulong*)u32ReadPos;
    return MT_TRUE;
}
#ifdef MT_ALSA_AI_SUPPORT
static inline mt_void CIRC_BUF_ALSA_Flush(CIRC_BUF_S *pstCb)
{
    pstCb->pu32Write     = 0;
    pstCb->pu32Read      = 0;
}
static inline mt_u32 CIRC_BUF_ALSA_QueryWritePos(CIRC_BUF_S *pstCb)
{
    return (pstCb->pu32Write);
}
static inline mt_u32 CIRC_BUF_ALSA_QueryReadPos(CIRC_BUF_S *pstCb)
{
    return (pstCb->pu32Read);
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif  /*end if 0*/
#endif /* End of #ifndef __SIMPLE_CB_H__*/
