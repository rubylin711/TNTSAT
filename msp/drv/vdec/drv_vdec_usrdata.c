/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
  File Name     : drv_vdec_usrdata.c
  Version       : Initial Draft
  Author        : Montage MA-SW
  Created       : 2016/01/06
  Description   :
  History       :
  1.Date        : 2016/01/06
    Author      :
    Modification: Created file

******************************************************************************/

/******************************* Include Files *******************************/

/* Sys headers */
#include <linux/sizes.h>	/*SZ_1K*/

/* Unf headers */
#include "mt_unf_common.h"

/* Drv headers */
#include "mt_kernel_adapt.h"

/* Local headers */
#include "vfmw.h"
#include "drv_vdec_private.h"
#include "drv_vdec_buf_mng.h"
#include "drv_vdec_usrdata.h"
#include "mt_module_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

#define I_FRAME 1
#define P_FRAME 2
#define B_FRAME 3

#define MAX_USERDATA_IN_ONE_FRAME (4)
#define VDEC_USERDATA_ONLY_SUPPORT_CC (1)
/*************************** Structure Definition ****************************/

typedef struct
{
    mt_handle hBuf;
    MT_BOOL bUsed;
    MT_VDEC_USRDAT_S astRefFrame[MAX_USERDATA_IN_ONE_FRAME];
    mt_u8 u8DataCnt;
}USRDATA_PARAM_S;

/***************************** Global Definition *****************************/


/***************************** Static Definition *****************************/

static USRDATA_PARAM_S g_stUsrData[MT_VDEC_MAX_INSTANCE_NEW];

/*********************************** Code ************************************/

//debug
static void dump_cc(unsigned char *data, int size)
{
	int i;

	printk("\r\n[CC]");
	for (i=0;i<size;i++)
	{
		printk("%02X",data[i]);
	}
	printk("\r\n");
}

mt_s32 USRDATA_Init(mt_void)
{
    mt_s32 i;

    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++)
    {
        g_stUsrData[i].u8DataCnt = 0;
        memset(g_stUsrData[i].astRefFrame, 0x0, sizeof(g_stUsrData[i].astRefFrame));
        g_stUsrData[i].hBuf = MT_INVALID_HANDLE;
        g_stUsrData[i].bUsed = MT_FALSE;
    }

    return MT_SUCCESS;
}

mt_s32 USRDATA_DeInit(mt_void)
{
    mt_s32 i;

    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++)
    {
        if (g_stUsrData[i].bUsed)
        {
            USRDATA_Free(i);
        }
        else
        {
            g_stUsrData[i].u8DataCnt = 0;
            g_stUsrData[i].hBuf = MT_INVALID_HANDLE;
            g_stUsrData[i].bUsed = MT_FALSE;
        }

        memset(g_stUsrData[i].astRefFrame, 0x0, sizeof(g_stUsrData[i].astRefFrame));
    }
    return MT_SUCCESS;
}

mt_s32 USRDATA_Alloc(mt_handle hHandle, MT_DRV_VDEC_USERDATABUF_S* pstBuf)
{
    mt_s32 s32Ret;
    mt_handle hBuf;
    BUFMNG_INST_CONFIG_S stBufInstCfg;

    if ((hHandle >= MT_VDEC_MAX_INSTANCE_NEW) || (MT_NULL == pstBuf) || (g_stUsrData[hHandle].bUsed))
    {
        MT_ERR_VDEC("bad param!\n");
        return MT_FAILURE;
    }

    /* Create buffer manager instance */
    stBufInstCfg.enAllocType = BUFMNG_ALLOC_INNER;
    stBufInstCfg.u32PhyAddr = 0;
    stBufInstCfg.pu8UsrVirAddr = MT_NULL;
    stBufInstCfg.pu8KnlVirAddr = MT_NULL;
    stBufInstCfg.u32Size = pstBuf->u32Size;
    snprintf(stBufInstCfg.aszName, sizeof(stBufInstCfg.aszName),"VDEC_UsrData%02d", (mt_u8)hHandle);
    s32Ret = BUFMNG_Create_forUsrData(&hBuf, &stBufInstCfg);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_VDEC("BUFMNG_Create_forUsrData err:%#x!\n", s32Ret);
        return MT_FAILURE;
    }

    /* Output phy addr */
    pstBuf->u32PhyAddr = stBufInstCfg.u32PhyAddr;
    MT_INFO_VDEC("USRDATA_Alloc: %#x %dB\n", pstBuf->u32PhyAddr, pstBuf->u32Size);

    g_stUsrData[hHandle].bUsed = MT_TRUE;
    g_stUsrData[hHandle].hBuf = hBuf;
    g_stUsrData[hHandle].u8DataCnt = 0;
    memset(g_stUsrData[hHandle].astRefFrame, 0x0, sizeof(g_stUsrData[hHandle].astRefFrame));
    return MT_SUCCESS;
}

mt_s32 USRDATA_SetUserAddr(mt_handle hHandle, ulong u32Addr)
{
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW)
    {
        return MT_FAILURE;
    }

    MT_INFO_VDEC("USRDATA_SetUserAddr: %#lx\n", u32Addr);
    return BUFMNG_SetUserAddr(g_stUsrData[hHandle].hBuf, u32Addr);
}

mt_s32 USRDATA_Free(mt_handle hHandle)
{
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW)
    {
        return MT_FAILURE;
    }

    if (MT_INVALID_HANDLE != g_stUsrData[hHandle].hBuf)
    {
        BUFMNG_Destroy_forUsrData(g_stUsrData[hHandle].hBuf);
    }

    g_stUsrData[hHandle].bUsed = MT_FALSE;
    g_stUsrData[hHandle].hBuf = MT_INVALID_HANDLE;
    g_stUsrData[hHandle].u8DataCnt = 0;
    return MT_SUCCESS;
}

mt_s32 USRDATA_Start(mt_handle hHandle)
{
    return USRDATA_Reset(hHandle);
}

mt_s32 USRDATA_Stop(mt_handle hHandle)
{
    return USRDATA_Reset(hHandle);
}

mt_s32 USRDATA_Reset(mt_handle hHandle)
{
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW)
    {
        return MT_FAILURE;
    }

    g_stUsrData[hHandle].u8DataCnt = 0;
    memset(g_stUsrData[hHandle].astRefFrame, 0x0, sizeof(g_stUsrData[hHandle].astRefFrame));
    BUFMNG_Reset(g_stUsrData[hHandle].hBuf);
    return MT_SUCCESS;
}

mt_s32 USRDATA_Acq(mt_handle hHandle, MT_UNF_VIDEO_USERDATA_S* pstUsrData, MT_UNF_VIDEO_USERDATA_TYPE_E* penType)
{
    mt_s32 s32Ret;
    BUFMNG_BUF_S stBuf;

    if ((hHandle >= MT_VDEC_MAX_INSTANCE_NEW) || (MT_NULL == pstUsrData) || (MT_NULL == penType))
    {
        return MT_FAILURE;
    }

    s32Ret = BUFMNG_AcqReadBuffer(g_stUsrData[hHandle].hBuf, &stBuf);
    if (MT_SUCCESS != s32Ret)
    {
        return s32Ret;
    }
    else
    {
        *penType = stBuf.u32Marker;
        memcpy(pstUsrData, stBuf.pu8KnlVirAddr, sizeof(MT_UNF_VIDEO_USERDATA_S));
        return MT_SUCCESS;
    }
}

mt_s32 USRDATA_Rls(mt_handle hHandle, MT_UNF_VIDEO_USERDATA_S* pstUsrData)
{
    mt_s32 s32Ret;
    BUFMNG_BUF_S stBuf;

    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW)
    {
        return MT_FAILURE;
    }

    stBuf.pu8KnlVirAddr = MT_NULL;
    stBuf.pu8UsrVirAddr = pstUsrData->pu8Buffer - sizeof(MT_UNF_VIDEO_USERDATA_S);
    stBuf.u32Size = sizeof(MT_UNF_VIDEO_USERDATA_S) + pstUsrData->u32Length;

    s32Ret = BUFMNG_RlsReadBuffer(g_stUsrData[hHandle].hBuf, &stBuf);
    return s32Ret;
}

mt_s32 USRDATA_Put(mt_handle hHandle, MT_VDEC_USRDAT_S* pstUsrData, MT_UNF_VIDEO_USERDATA_TYPE_E enType)
{
    mt_s32 s32Ret;
    MT_BOOL bOverFlow = MT_FALSE;
    BUFMNG_BUF_S stBuf;
    MT_UNF_VIDEO_USERDATA_S *pstPutData;

    stBuf.u32Size = sizeof(MT_UNF_VIDEO_USERDATA_S) + pstUsrData->data_size;
    s32Ret = BUFMNG_GetWriteBuffer_forUsrData(g_stUsrData[hHandle].hBuf, &stBuf);
	if(pstUsrData->data_size > stBuf.u32Size)
	{
		return MT_FALSE;
	}

    if (MT_ERR_BM_BUFFER_FULL == s32Ret)
    {
		MT_ERR_VDEC("UsrData BUFFER_FULL!\n");
        BUFMNG_Reset(g_stUsrData[hHandle].hBuf);
        bOverFlow = MT_TRUE;
        s32Ret = BUFMNG_GetWriteBuffer_forUsrData(g_stUsrData[hHandle].hBuf, &stBuf);
    }

    if (MT_SUCCESS != s32Ret)
    {
		MT_ERR_VDEC("Get UsrData Buffer Fail!\n");
        return s32Ret;
    }

    pstPutData = (MT_UNF_VIDEO_USERDATA_S *)stBuf.pu8KnlVirAddr;
    pstPutData->enBroadcastProfile = MT_UNF_VIDEO_BROADCAST_DVB;
    pstPutData->enPositionInStream = MT_UNF_VIDEO_USER_DATA_POSITION_UNKNOWN;
    pstPutData->u32Pts = (mt_u32)pstUsrData->PTS;
    pstPutData->u32SeqCnt = 0;
    pstPutData->u32SeqFrameCnt = 0;
    pstPutData->u32Length = pstUsrData->data_size;
    pstPutData->bBufferOverflow = bOverFlow;
	pstPutData->bTopFieldFirst = pstUsrData->top_field_first;


	
    memcpy(stBuf.pu8KnlVirAddr+sizeof(MT_UNF_VIDEO_USERDATA_S), pstUsrData->data, pstUsrData->data_size);
	//debug
	if (0)
	{
		dump_cc(pstUsrData->data, pstUsrData->data_size);
    }
    pstPutData->pu8Buffer = stBuf.pu8UsrVirAddr + sizeof(MT_UNF_VIDEO_USERDATA_S);
    stBuf.u32Marker = enType;

    s32Ret =  BUFMNG_PutWriteBuffer_forUsrData(g_stUsrData[hHandle].hBuf, &stBuf);
    return s32Ret;
}

mt_s32 USRDATA_SetEosFlag(mt_handle hHandle)
{
    mt_s32 i;
    mt_u8 u8DataCnt;

    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW)
    {
        return MT_FAILURE;
    }

    u8DataCnt = g_stUsrData[hHandle].u8DataCnt;

    /* Put reference frame */
    for (i=0; i<u8DataCnt; i++)
    {
        USRDATA_Put(hHandle, &(g_stUsrData[hHandle].astRefFrame[i]), MT_UNF_VIDEO_USERDATA_DVB1_CC);
    }
    g_stUsrData[hHandle].u8DataCnt = 0;

    return MT_SUCCESS;
}

static mt_s32 USRDATA_CC(mt_handle hHandle, MT_VDEC_USRDAT_S* pstUsrData)
{
    mt_s32 i;
    mt_u8 u8DataCnt;
#if (1==VDEC_USERDATA_NEED_ARRANGE)
    /* I_FRAME or P_FRAME, sort */
    if ((I_FRAME == pstUsrData->pic_coding_type) || (P_FRAME == pstUsrData->pic_coding_type))
    {
        u8DataCnt = g_stUsrData[hHandle].u8DataCnt;
        /* Belongs to the same frame with last data */
        if (pstUsrData->pic_num_count == g_stUsrData[hHandle].astRefFrame[0].pic_num_count)
        {
            if (u8DataCnt >= MAX_USERDATA_IN_ONE_FRAME)
            {
                MT_WARN_VDEC("Too many data in a frame.\n");
            }
            else
            {
                /* Only save new data */
                g_stUsrData[hHandle].astRefFrame[u8DataCnt] = *pstUsrData;
                g_stUsrData[hHandle].u8DataCnt++;
            }
        }
        /* New frame data */
        else
        {
            /* Put reference frame */
            for (i=0; i<u8DataCnt; i++)
            {
                USRDATA_Put(hHandle, &(g_stUsrData[hHandle].astRefFrame[i]), MT_UNF_VIDEO_USERDATA_DVB1_CC);
            }

            /* Save new data */
            g_stUsrData[hHandle].astRefFrame[0] = *pstUsrData;
            g_stUsrData[hHandle].u8DataCnt = 1;
        }

    }
    /* B_FRAME or other error frame, put directly */
    else
#endif
    {
        return USRDATA_Put(hHandle, pstUsrData, MT_UNF_VIDEO_USERDATA_DVB1_CC);
    }

    return MT_SUCCESS;
}

mt_s32 USRDATA_Arrange(mt_handle hHandle, MT_VDEC_USRDAT_S* pstUsrData)
{
#if (VDEC_USERDATA_ONLY_SUPPORT_CC == 0)
    mt_u32 u32ID;
    mt_u8 u8Type;
#endif

    if ((hHandle >= MT_VDEC_MAX_INSTANCE_NEW) || (MT_NULL == pstUsrData) ||
       (0 == pstUsrData->data_size))
    {
        return MT_FAILURE;
    }

#if (VDEC_USERDATA_ONLY_SUPPORT_CC == 0)
    if (pstUsrData->data_size > 5)
    {
        u32ID = *((mt_u32*)pstUsrData->data);
        u8Type = pstUsrData->data[4];
        if (VDEC_USERDATA_IDENTIFIER_DVB1 == u32ID)
        {
            if (VDEC_USERDATA_TYPE_DVB1_CC == u8Type)
            {
                return USRDATA_CC(hHandle, pstUsrData);
            }
            else if (VDEC_USERDATA_TYPE_DVB1_BAR == u8Type)
            {
                return USRDATA_Put(hHandle, pstUsrData, MT_UNF_VIDEO_USERDATA_DVB1_BAR);
            }
        }
        else if (VDEC_USERDATA_IDENTIFIER_AFD == u32ID)
        {
            return USRDATA_Put(hHandle, pstUsrData, MT_UNF_VIDEO_USERDATA_AFD);
        }
        else
        {
            return USRDATA_Put(hHandle, pstUsrData, MT_UNF_VIDEO_USERDATA_UNKNOWN);
        }
    }
    else
    {
        return USRDATA_Put(hHandle, pstUsrData, MT_UNF_VIDEO_USERDATA_UNKNOWN);
    }

    return MT_FAILURE;
#else
    return USRDATA_CC(hHandle, pstUsrData);
#endif
}

EXPORT_SYMBOL(USRDATA_Rls);
EXPORT_SYMBOL(USRDATA_Acq);
EXPORT_SYMBOL(USRDATA_Alloc);
EXPORT_SYMBOL(USRDATA_SetUserAddr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

