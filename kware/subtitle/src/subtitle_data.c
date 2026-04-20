#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mt_type.h"
#include "subtitle_debug.h"
#include "subtitle_data.h"
#include "subtitle_parse.h"

/* if used sdk pes buffer directly,don't copy pes data,
*  data in pes buffer may be modified by new pes data when
*  subtitle parsing
*/
#define SUBT_USED_PES_BUFFER (1)
#define SUBT_PES_PACKET_MIN (26)
#define SUBT_PES_PACKET_LEN (24*1024)
#define SUBT_MAX_PES_PACKET_LEN (64*1024)
#define CACHE_NODE_NUM_MAX  (3)

typedef struct tagCache_Buffer_Node_S
{
    struct tagCache_Buffer_Node_S *next;
    mt_u8  au8Data[SUBT_PES_PACKET_LEN];
    mt_u32 u32DataSize;
}Cache_Buffer_Node_S;


typedef struct tagSUBT_DATA_RECV_S
{
    MT_HANDLE hDataParse;

#if SUBT_USED_PES_BUFFER
    mt_u32    u32PESWritenLen; /*The data length of recving PES packket, in byte */
    mt_u8     *pu8WriteDataAddr; /*The data address of includeing start code PES packet */
    mt_u8     au8PesBuffer[SUBT_MAX_PES_PACKET_LEN];
#endif

    mt_u16    u16PageID;
    mt_u16    u16AncillaryID;

    Cache_Buffer_Node_S *pstCacheNode;
    Cache_Buffer_Node_S astCacheNode[CACHE_NODE_NUM_MAX];

}SUBT_DATA_RECV_S;

static mt_void Cache_Init(SUBT_DATA_RECV_S *pstDataRecv)
{
    mt_u8 i = 0;

    for (i = 0; i < (CACHE_NODE_NUM_MAX - 1); i++)
    {
        pstDataRecv->astCacheNode[i].next = &pstDataRecv->astCacheNode[i+1];
    }
    pstDataRecv->astCacheNode[CACHE_NODE_NUM_MAX - 1].next = &pstDataRecv->astCacheNode[0];

    pstDataRecv->pstCacheNode = &pstDataRecv->astCacheNode[0];
}

static mt_s32 Cache_Put(SUBT_DATA_RECV_S *pstDataRecv, mt_u8 *pu8Data, mt_u32 u32DataSize)
{
    if (u32DataSize < SUBT_PES_PACKET_MIN || u32DataSize > SUBT_PES_PACKET_LEN)
    {
        return MT_FAILURE;
    }

    memset(pstDataRecv->pstCacheNode->au8Data, 0, SUBT_PES_PACKET_LEN);
    memcpy(pstDataRecv->pstCacheNode->au8Data, pu8Data, u32DataSize);
    pstDataRecv->pstCacheNode->u32DataSize = u32DataSize;

    pstDataRecv->pstCacheNode = pstDataRecv->pstCacheNode->next;

    return MT_SUCCESS;
}

static mt_s32 Cache_Get(SUBT_DATA_RECV_S *pstDataRecv, mt_u8 **ppu8Data, mt_u32 *pu32DataSize)
{
    if (pstDataRecv->pstCacheNode->u32DataSize)
    {
        *ppu8Data = pstDataRecv->pstCacheNode->au8Data;
        *pu32DataSize = pstDataRecv->pstCacheNode->u32DataSize;

        pstDataRecv->pstCacheNode->u32DataSize = 0;
    }
    else
    {
        *ppu8Data = NULL;
        *pu32DataSize = 0;
    }
    pstDataRecv->pstCacheNode = pstDataRecv->pstCacheNode->next;

    return MT_SUCCESS;
}

static mt_void Cache_Reset(SUBT_DATA_RECV_S *pstDataRecv)
{
    mt_u8 i = 0;

    for (i = 0; i < CACHE_NODE_NUM_MAX; i++)
    {
        pstDataRecv->astCacheNode[i].u32DataSize = 0;
    }

    pstDataRecv->pstCacheNode = &pstDataRecv->astCacheNode[0];
}


mt_s32 SUBT_DataRecv_Init(mt_void)
{
    return MT_SUCCESS;
}

mt_s32 SUBT_DataRecv_DeInit(mt_void)
{
    return MT_SUCCESS;
}

mt_s32 SUBT_DataRecv_Create(mt_u16 u16PageID, mt_u16 u16AncillaryID, MT_HANDLE *phDataRecv)
{
    mt_s32 s32Ret = MT_SUCCESS;

    SUBT_DATA_RECV_S *pstDataRecv = NULL;

    if (NULL == phDataRecv)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }

    pstDataRecv = (SUBT_DATA_RECV_S*)malloc(sizeof(SUBT_DATA_RECV_S));
    if (NULL == pstDataRecv)
    {
        MT_ERR_SUBT("malloc data struct failure...\n");

        return MT_FAILURE;
    }

    memset(pstDataRecv, 0, sizeof(SUBT_DATA_RECV_S));

    pstDataRecv->u16PageID = u16PageID;
    pstDataRecv->u16AncillaryID = u16AncillaryID;
    pstDataRecv->hDataParse = 0;

#if SUBT_USED_PES_BUFFER
    pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
    pstDataRecv->u32PESWritenLen = 0;
#endif

    Cache_Init(pstDataRecv);
    *phDataRecv = (MT_HANDLE)pstDataRecv;

    MT_INFO_SUBT("success, with handle:0x%08x!\n", *phDataRecv);

    return s32Ret;
}

mt_s32 SUBT_DataRecv_Destroy(MT_HANDLE hDataRecv)
{
    SUBT_DATA_RECV_S *pstDataRecv = (SUBT_DATA_RECV_S *)hDataRecv;

    if (NULL == pstDataRecv)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }

    MT_INFO_SUBT("begin to use handle:0x%08x!\n", pstDataRecv);

    free((void*)pstDataRecv);

    MT_INFO_SUBT("success!\n");

    return MT_SUCCESS;
}

mt_s32 SUBT_DataRecv_Reset(MT_HANDLE hDataRecv, MT_BOOL bRecvFlag)
{
    SUBT_DATA_RECV_S *pstDataRecv = (SUBT_DATA_RECV_S *)hDataRecv;

    if (NULL == pstDataRecv)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }

#if SUBT_USED_PES_BUFFER
    memset(pstDataRecv->au8PesBuffer, 0, sizeof(pstDataRecv->au8PesBuffer));
    pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
    pstDataRecv->u32PESWritenLen = 0;
#endif

    Cache_Reset(pstDataRecv);

    return MT_SUCCESS;
}

mt_s32 SUBT_DataRecv_Updata(MT_HANDLE hDataRecv, mt_u16 u16PageID, mt_u16 u16AncillaryID)
{
    SUBT_DATA_RECV_S *pstDataRecv = (SUBT_DATA_RECV_S *)hDataRecv;

    if (NULL == pstDataRecv)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }

    pstDataRecv->u16PageID = u16PageID;
    pstDataRecv->u16AncillaryID = u16AncillaryID;

    MT_INFO_SUBT("successfully update page id to %d and ancillary id to %d\n", u16PageID, u16AncillaryID);

    return MT_SUCCESS;
}

mt_s32 SUBT_DataRecv_BindParsing(MT_HANDLE hDataRecv, MT_HANDLE hDataParse)
{
    SUBT_DATA_RECV_S *pstDataRecv = (SUBT_DATA_RECV_S *)hDataRecv;

    if (NULL == pstDataRecv)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }

    pstDataRecv->hDataParse = hDataParse;

    return MT_SUCCESS;
}

mt_s32 SUBT_DataRecv_UnbindParsing(MT_HANDLE hDataRecv)
{
    SUBT_DATA_RECV_S *pstDataRecv = (SUBT_DATA_RECV_S *)hDataRecv;

    if (NULL == pstDataRecv)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }

    pstDataRecv->hDataParse = 0;

    return MT_SUCCESS;
}

mt_s32 SUBT_DataRecv_Redo(MT_HANDLE hDataRecv)
{
    mt_s32 s32Ret = MT_SUCCESS;
    SUBT_DATA_RECV_S *pstDataRecv = (SUBT_DATA_RECV_S *)hDataRecv;
    mt_u8 *pu8Data = NULL;
    mt_u32 u32DataSize = 0;
    if (NULL == pstDataRecv)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }
     

    /* redo parse cache pes data */
    do
    {
        s32Ret = Cache_Get(pstDataRecv, &pu8Data, &u32DataSize);
        if ((MT_SUCCESS == s32Ret) && u32DataSize && pu8Data)
        {
            if (pstDataRecv->hDataParse)
            {
                s32Ret |= SUBT_DataParse_ParsePESPacket(pstDataRecv->hDataParse, pu8Data, u32DataSize,
                                            pstDataRecv->u16PageID, pstDataRecv->u16AncillaryID);
                if (s32Ret != MT_SUCCESS)
                {
                    MT_ERR_SUBT("failed to SUBT_DataParse_ParsePESPacket...\n");
                }
            }
        }
    } while(u32DataSize);

    return MT_SUCCESS;
}


mt_s32 SUBT_DataRecv_Inject(MT_HANDLE hDataRecv, mt_u8 *pu8Data, mt_u32 u32DataSize)
{
    mt_s32 s32Ret = MT_SUCCESS;
#if SUBT_USED_PES_BUFFER
    mt_u32 u32RemainSpaceByte = 0;
#endif
    mt_u32 u32PESPayloadLength = 0;
    SUBT_DATA_RECV_S *pstDataRecv = (SUBT_DATA_RECV_S *)hDataRecv;
    if (NULL == pstDataRecv || NULL == pu8Data || 0 == u32DataSize)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

#if SUBT_USED_PES_BUFFER
    if (pstDataRecv->u32PESWritenLen == 0)
    {
        /*
              * 1. Ref[13818-1] 2.4.3.7 PES packet
              * PES packet_start_code_prefix:00 00 01(24bits), following stream_id, which constitutes the start code marking the start of packet
              * 2. Ref[Subtitling system.pdf] section 6. PES packet format
              * stream_id : 1011 1101[0xbd] standard for subtitle stream.
              */
        if (   (pu8Data[0] == 0x00) && (pu8Data[1] == 0x00)
            && (pu8Data[2] == 0x01) && (pu8Data[3] == 0xbd))
        {
            if(u32DataSize <= SUBT_MAX_PES_PACKET_LEN)
            {
                pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
                memcpy(pstDataRecv->pu8WriteDataAddr, pu8Data, u32DataSize);

                pstDataRecv->pu8WriteDataAddr += u32DataSize;
                pstDataRecv->u32PESWritenLen = u32DataSize;
            }
            else
            {
                MT_WARN_SUBT("SUBT PES PACKET IS TOO LARGE!!\n");
            }
        }
        else
        {
            MT_INFO_SUBT("NOT SUBTITLE PES START CODE!!!!!\n");
        }
    }
    else
    {
        /*
        should not be here
        according to dmx codes(water fo), a whole PES packet will not be separated into several packet
    */
        printf("\n  should not be here sdfse !!!!!!!\n\n ================  pstDataRecv->u32PESWritenLen %d     u32DataSize %d \n\n\n",  pstDataRecv->u32PESWritenLen, u32DataSize);
        
        u32RemainSpaceByte = (mt_u32)(&pstDataRecv->au8PesBuffer[SUBT_MAX_PES_PACKET_LEN] - pstDataRecv->pu8WriteDataAddr);

        if (u32DataSize <= u32RemainSpaceByte)
        {
            memcpy(pstDataRecv->pu8WriteDataAddr, pu8Data, u32DataSize);

            pstDataRecv->pu8WriteDataAddr += u32DataSize;
            pstDataRecv->u32PESWritenLen += u32DataSize;
        }
        else
        {
            MT_WARN_SUBT("\nSUBTITLE SECTION PACKET IS TOO LARGE!!\n");

            /* discard the received data, reset pes buffer */
            pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
            pstDataRecv->u32PESWritenLen = 0;
        }

		//drop
        pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
    	pstDataRecv->u32PESWritenLen = 0;
    }
    if(pstDataRecv->u32PESWritenLen >= 6)
    {
        /* PES packet_length(payload size): A 16-bit field after start code(24-bit) and stream_id(8-bit), that is, the fifth and sixth byte */
        u32PESPayloadLength = (mt_u32)((pstDataRecv->au8PesBuffer[4]<<8)|pstDataRecv->au8PesBuffer[5]);

        /* The whole pes data received ok */
        if(pstDataRecv->u32PESWritenLen >= (u32PESPayloadLength+6))
        {
            MT_INFO_SUBT("PayloadLen=0x%x,PageID=%d,AncillaryID=%d\n",
                            u32PESPayloadLength,pstDataRecv->u16PageID,pstDataRecv->u16AncillaryID);

            if (u32PESPayloadLength+6 <= SUBT_PES_PACKET_LEN )
            {
                s32Ret = Cache_Put(pstDataRecv, pu8Data, u32PESPayloadLength+6);
            }

            /* started  parsing pes packet data  */
            if (pstDataRecv->hDataParse)
            {
                s32Ret = SUBT_DataParse_ParsePESPacket(pstDataRecv->hDataParse, pstDataRecv->au8PesBuffer,
                                    u32PESPayloadLength, pstDataRecv->u16PageID, pstDataRecv->u16AncillaryID);
                if (s32Ret != MT_SUCCESS)
                {
                    MT_ERR_SUBT("failed to SUBT_DataParse_ParsePESPacket...\n");
                }
            }

            //memset(pstDataRecv->au8PesBuffer, 0, sizeof(pstDataRecv->au8PesBuffer));
            pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
            pstDataRecv->u32PESWritenLen = 0;

            return s32Ret;
        }
    }

    //according to dmx codes(water fo), a whole PES packet will not be separated into several packet
    // error packet, drop 
    pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
 	pstDataRecv->u32PESWritenLen = 0;

#else
    if (u32DataSize < 6)
    {
        MT_ERR_SUBT("u32DataSize < 6...\n");

        return MT_FAILURE;
    }

    if ((pu8Data[0] == 0x00) && (pu8Data[1] == 0x00)
            && (pu8Data[2] == 0x01) && (pu8Data[3] == 0xbd) )
    {
        u32PESPayloadLength = (pu8Data[4]<<8)| pu8Data[5];
        if (u32DataSize >= (u32PESPayloadLength+6))
        {
            MT_INFO_SUBT("PayloadLen=0x%x,PageID=%d,AncillaryID=%d\n",
                            u32PESPayloadLength,pstDataRecv->u16PageID,pstDataRecv->u16AncillaryID);

            if (u32PESPayloadLength+6 <= SUBT_PES_PACKET_LEN )
            {
                s32Ret = Cache_Put(pstDataRecv, pu8Data, u32PESPayloadLength+6);
            }

            /* started  parsing pes packet data  */
            if (pstDataRecv->hDataParse)
            {
                s32Ret = SUBT_DataParse_ParsePESPacket(pstDataRecv->hDataParse, pu8Data,
                                    u32PESPayloadLength, pstDataRecv->u16PageID, pstDataRecv->u16AncillaryID);
                if (s32Ret != MT_SUCCESS)
                {
                    MT_ERR_SUBT("failed to SUBT_DataParse_ParsePESPacket...\n");

                    return s32Ret;
                }
            }
        }
    }
#endif
    return MT_SUCCESS;
}


