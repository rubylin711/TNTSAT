#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mt_type.h"
#include "mt_type.h"
#include "teletext_debug.h"
#include "teletext_data.h"
#include "vbi_api.h"

#define TTX_USED_PES_BUFFER (1)
#define TTX_PES_PACKET_MIN (26)
#define TTX_PES_PACKET_LEN (24*1024)
#define TTX_MAX_PES_PACKET_LEN (64*1024)
#define CACHE_NODE_NUM_MAX  (3)

typedef struct tagCache_Buffer_Node_S
{
    struct tagCache_Buffer_Node_S *next;
    mt_u8  au8Data[TTX_PES_PACKET_LEN];
    mt_u32 u32DataSize;
} Cache_Buffer_Node_S;


typedef struct tagTTX_DATA_RECV_S
{

#if TTX_USED_PES_BUFFER
    mt_u32    u32PESWritenLen; /*The data length of recving PES packket, in byte */
    //mt_u8     *pu8WriteDataAddr; /*The data address of includeing start code PES packet */
    mt_u8     au8PesBuffer[TTX_MAX_PES_PACKET_LEN];
#endif

    Cache_Buffer_Node_S *pstCacheNode;
    Cache_Buffer_Node_S astCacheNode[CACHE_NODE_NUM_MAX];

} TTX_DATA_RECV_S;

MT_BOOL ttx_check_pes(mt_u8 *pu8Data);


static mt_void Cache_Init(TTX_DATA_RECV_S *pstDataRecv)
{
    mt_u8 i = 0;

    for (i = 0; i < (CACHE_NODE_NUM_MAX - 1); i++)
    {
        pstDataRecv->astCacheNode[i].next = &pstDataRecv->astCacheNode[i+1];
    }
    pstDataRecv->astCacheNode[CACHE_NODE_NUM_MAX - 1].next = &pstDataRecv->astCacheNode[0];

    pstDataRecv->pstCacheNode = &pstDataRecv->astCacheNode[0];
}

#if 0
static mt_s32 Cache_Put(TTX_DATA_RECV_S *pstDataRecv, mt_u8 *pu8Data, mt_u32 u32DataSize)
{
    if (u32DataSize < TTX_PES_PACKET_MIN || u32DataSize > TTX_PES_PACKET_LEN)
    {
        return MT_FAILURE;
    }

    memset(pstDataRecv->pstCacheNode->au8Data, 0, TTX_PES_PACKET_LEN);
    memcpy(pstDataRecv->pstCacheNode->au8Data, pu8Data, u32DataSize);
    pstDataRecv->pstCacheNode->u32DataSize = u32DataSize;

    pstDataRecv->pstCacheNode = pstDataRecv->pstCacheNode->next;

    return MT_SUCCESS;
}

static mt_s32 Cache_Get(TTX_DATA_RECV_S *pstDataRecv, mt_u8 **ppu8Data, mt_u32 *pu32DataSize)
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

static mt_void Cache_Reset(TTX_DATA_RECV_S *pstDataRecv)
{
    mt_u8 i = 0;

    for (i = 0; i < CACHE_NODE_NUM_MAX; i++)
    {
        pstDataRecv->astCacheNode[i].u32DataSize = 0;
    }

    pstDataRecv->pstCacheNode = &pstDataRecv->astCacheNode[0];
}
#endif

mt_s32 TTX_DataRecv_Create(MT_HANDLE *phDataRecv)
{
    mt_s32 s32Ret = MT_SUCCESS;

    TTX_DATA_RECV_S *pstDataRecv = NULL;
    if (NULL == phDataRecv)
    {
        MT_ERR_TTX("param is NULL...\n");
        return MT_FAILURE;
    }

    pstDataRecv = (TTX_DATA_RECV_S*)malloc(sizeof(TTX_DATA_RECV_S));
    if (NULL == pstDataRecv)
    {
        MT_ERR_TTX("malloc data struct failure...\n");
        return MT_FAILURE;
    }

    memset(pstDataRecv, 0, sizeof(TTX_DATA_RECV_S));

#if TTX_USED_PES_BUFFER
    //pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
    pstDataRecv->u32PESWritenLen = 0;
#endif

    Cache_Init(pstDataRecv);
    *phDataRecv = (MT_HANDLE)pstDataRecv;

    MT_INFO_TTX("success, with handle:0x%08x!\n", *phDataRecv);
    return s32Ret;
}

mt_s32 TTX_DataRecv_Destroy(MT_HANDLE hDataRecv)
{
    TTX_DATA_RECV_S *pstDataRecv = (TTX_DATA_RECV_S *)hDataRecv;

    if (NULL == pstDataRecv)
    {
        MT_ERR_TTX("param is NULL...\n");

        return MT_FAILURE;
    }

    MT_INFO_TTX("begin to use handle:0x%08x!\n", pstDataRecv);

    free((void*)pstDataRecv);

    MT_INFO_TTX("success!\n");

    return MT_SUCCESS;
}


MT_BOOL ttx_check_pes(mt_u8 *pu8Data)
{
  /*
              * 1. Ref[13818-1] 2.4.3.7 PES packet
              * PES packet_start_code_prefix:00 00 01(24bits), following stream_id, which constitutes the start code marking the start of packet
              */
  if (   (pu8Data[0] == 0x00) && (pu8Data[1] == 0x00) && (pu8Data[2] == 0x01) && (pu8Data[3] == 0xbd))
    return MT_TRUE;

  return MT_FALSE;
}

mt_s32 TTX_DataRecv_Inject(MT_HANDLE hDataRecv, mt_u8 *pu8Data, mt_u32 u32DataSize)
{
    mt_s32 s32Ret = MT_SUCCESS;
#if TTX_USED_PES_BUFFER
    mt_u32 u32RemainSpaceByte = 0;
#endif
    mt_u32 u32PESPayloadLength = 0;
    TTX_DATA_RECV_S *pstDataRecv = (TTX_DATA_RECV_S *)hDataRecv;

    if (NULL == pstDataRecv || NULL == pu8Data || 0 == u32DataSize)
    {
        MT_ERR_TTX("parameter is invalid...\n");
        return MT_FAILURE;
    }
//printf("linda debug %s %d\n", __func__, __LINE__);
#if TTX_USED_PES_BUFFER

    if((pstDataRecv->u32PESWritenLen + u32DataSize) > TTX_MAX_PES_PACKET_LEN)
    {
      printf("   TTX PES PACKET IS TOO LARGE1  pstDataRecv->u32PESWritenLen %d  u32DataSize%d \n", pstDataRecv->u32PESWritenLen, u32DataSize);
      u32DataSize = TTX_MAX_PES_PACKET_LEN - pstDataRecv->u32PESWritenLen;
    }  

    memcpy(&pstDataRecv->au8PesBuffer[pstDataRecv->u32PESWritenLen], pu8Data, u32DataSize);
    pstDataRecv->u32PESWritenLen += u32DataSize;

    // parse from the head
    mt_u32 data_pos = 0;
    while(1)
    {
        //less than 6, cpy data to head and quit
        u32RemainSpaceByte = (pstDataRecv->u32PESWritenLen - data_pos);
        if(u32RemainSpaceByte < 6)
        {
          memcpy(pstDataRecv->au8PesBuffer, &pstDataRecv->au8PesBuffer[data_pos], u32RemainSpaceByte);
          pstDataRecv->u32PESWritenLen = u32RemainSpaceByte;

          break;
        }

        // pes head found
        if(ttx_check_pes(&pstDataRecv->au8PesBuffer[data_pos]))
        {
            u32PESPayloadLength = (pstDataRecv->au8PesBuffer[4+data_pos]<<8)|pstDataRecv->au8PesBuffer[5+data_pos];

            //can not be so large
            if(u32PESPayloadLength > (TTX_MAX_PES_PACKET_LEN >> 2))
            {
                //drop all
                pstDataRecv->u32PESWritenLen = 0;
                break;
            }

            //data not enough ,save it
            if( u32PESPayloadLength > u32RemainSpaceByte)
            {
                //cpy to head
                memcpy(pstDataRecv->au8PesBuffer, &pstDataRecv->au8PesBuffer[data_pos], u32RemainSpaceByte);
                pstDataRecv->u32PESWritenLen = u32RemainSpaceByte;
                break;
            }
            //abnormal val,drop it.
            if(u32PESPayloadLength == 0)
            {
                //drop all
                pstDataRecv->u32PESWritenLen = 0;
                break;
            }

            //data enough, parse it
            s32Ret = vbi_pes_parse_vsb( &pstDataRecv->au8PesBuffer[data_pos]);
            
            if(MT_SUCCESS != s32Ret)
            {
                MT_ERR_TTX("vbi_pes_parse_vsb ret abnormal...\n");
            }
            data_pos += u32PESPayloadLength;
        }
        //pes head not found, drop all
        else
        {
            //drop all
            pstDataRecv->u32PESWritenLen = 0;
            break;
        }

    }
#endif
//printf("linda debug %s %d\n", __func__, __LINE__);	
    return MT_SUCCESS;
}
//not used
#if 0
mt_s32 TTX_DataRecv_Inject2(MT_HANDLE hDataRecv, mt_u8 *pu8Data, mt_u32 u32DataSize)
{
    mt_s32 s32Ret = MT_SUCCESS;
#if TTX_USED_PES_BUFFER
    mt_u32 u32RemainSpaceByte = 0;
#endif
    mt_u32 u32PESPayloadLength = 0;
    TTX_DATA_RECV_S *pstDataRecv = (TTX_DATA_RECV_S *)hDataRecv;

    if (NULL == pstDataRecv || NULL == pu8Data || 0 == u32DataSize)
    {
        MT_ERR_TTX("parameter is invalid...\n");
        return MT_FAILURE;
    }
//printf("linda debug %s %d\n", __func__, __LINE__);
#if TTX_USED_PES_BUFFER
    if (pstDataRecv->u32PESWritenLen == 0)
    {
        /*
              * 1. Ref[13818-1] 2.4.3.7 PES packet
              * PES packet_start_code_prefix:00 00 01(24bits), following stream_id, which constitutes the start code marking the start of packet
              */

        if (   (pu8Data[0] == 0x00) && (pu8Data[1] == 0x00)
                && (pu8Data[2] == 0x01) && (pu8Data[3] == 0xbd))
        {
            if(u32DataSize <= TTX_MAX_PES_PACKET_LEN)
            {
                pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
                memcpy(pstDataRecv->pu8WriteDataAddr, pu8Data, u32DataSize);

                pstDataRecv->pu8WriteDataAddr += u32DataSize;
                pstDataRecv->u32PESWritenLen = u32DataSize;
            }
            else
            {
                MT_WARN_TTX("TTX PES PACKET IS TOO LARGE!!\n");
            }
        }
        else
        {
            MT_INFO_TTX("NOT TTX PES START CODE!!!!!\n");
        }
    }
    else
    {
        u32RemainSpaceByte = &pstDataRecv->au8PesBuffer[TTX_MAX_PES_PACKET_LEN] - pstDataRecv->pu8WriteDataAddr;

        if (u32DataSize <= u32RemainSpaceByte)
        {
            memcpy(pstDataRecv->pu8WriteDataAddr, pu8Data, u32DataSize);

            pstDataRecv->pu8WriteDataAddr += u32DataSize;
            pstDataRecv->u32PESWritenLen += u32DataSize;
        }
        else
        {
            MT_WARN_TTX("\nTTX SECTION PACKET IS TOO LARGE!!\n");

            /* discard the received data, reset pes buffer */
            pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
            pstDataRecv->u32PESWritenLen = 0;
        }
    }
#if 0		
//	printf("linda debug %s %d\n", __func__, __LINE__);	
    if(pstDataRecv->u32PESWritenLen >= 6)
    {
        /* PES packet_length(payload size): A 16-bit field after start code(24-bit) and stream_id(8-bit), that is, the fifth and sixth byte */
        u32PESPayloadLength = (pstDataRecv->au8PesBuffer[4]<<8)|pstDataRecv->au8PesBuffer[5];

        /* The whole pes data received ok */
        if(pstDataRecv->u32PESWritenLen >= (u32PESPayloadLength+6))
        {
            if (u32PESPayloadLength+6 <= TTX_PES_PACKET_LEN )
            {
                s32Ret = Cache_Put(pstDataRecv, pu8Data, u32PESPayloadLength+6);
            }

            /* started  parsing pes packet data  */
//		printf("linda debug %s %d\n", __func__, __LINE__);
            s32Ret = vbi_pes_parse_vsb(pstDataRecv->au8PesBuffer);


            memset(pstDataRecv->au8PesBuffer, 0, sizeof(pstDataRecv->au8PesBuffer));
            pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
            pstDataRecv->u32PESWritenLen = 0;		
            return s32Ret;
        }
    }
#else
    if(pstDataRecv->u32PESWritenLen >= 6)
    {
        /* PES packet_length(payload size): A 16-bit field after start code(24-bit) and stream_id(8-bit), that is, the fifth and sixth byte */
        u32PESPayloadLength = (pstDataRecv->au8PesBuffer[4+start_pos]<<8)|pstDataRecv->au8PesBuffer[5+start_pos];

//printf("\r\n --------u32PESPayloadLength:%d, pstDataRecv->u32PESWritenLen:%d", u32PESPayloadLength, pstDataRecv->u32PESWritenLen);
        /* The whole pes data received ok */
        while(1)
        {
			if(pstDataRecv->u32PESWritenLen >= (u32PESPayloadLength+6))
			{
	            if (u32PESPayloadLength+6 <= TTX_PES_PACKET_LEN )
	            {
	                s32Ret = Cache_Put(pstDataRecv,  &pstDataRecv->au8PesBuffer[start_pos], u32PESPayloadLength+6);
	            }

	            /* started  parsing pes packet data  */
	//		printf("linda debug %s %d\n", __func__, __LINE__);
	            s32Ret = vbi_pes_parse_vsb( &pstDataRecv->au8PesBuffer[start_pos]);

				if(pstDataRecv->u32PESWritenLen == (u32PESPayloadLength+6))
				{
		            memset(pstDataRecv->au8PesBuffer, 0, sizeof(pstDataRecv->au8PesBuffer));
		            pstDataRecv->pu8WriteDataAddr = pstDataRecv->au8PesBuffer;
		            pstDataRecv->u32PESWritenLen = 0;		
					start_pos = 0;
					break;
				}
				else
				{
					start_pos = start_pos + (u32PESPayloadLength+6);
					pstDataRecv->u32PESWritenLen -= (u32PESPayloadLength+6);
				}
			}
			else
			{					
//				printf("\r\n ~~~~~u32PESPayloadLength:%d,pstDataRecv->u32PESWritenLen:%d", u32PESPayloadLength,pstDataRecv->u32PESWritenLen);
				break;			
			}
			
			u32PESPayloadLength = (pstDataRecv->au8PesBuffer[4+start_pos]<<8)|pstDataRecv->au8PesBuffer[5+start_pos];
//			printf("\r\n ~~~~~u32PESPayloadLength:%d,pstDataRecv->u32PESWritenLen:%d", u32PESPayloadLength,pstDataRecv->u32PESWritenLen);
        }
    }

#endif
#else
    if (u32DataSize < 6)
    {
        MT_ERR_TTX("u32DataSize < 6...\n");
        return MT_FAILURE;
    }
    if ((pu8Data[0] == 0x00) && (pu8Data[1] == 0x00)
            && (pu8Data[2] == 0x01) && (pu8Data[3] == 0xbd) )
    {
        u32PESPayloadLength = (pu8Data[4]<<8)| pu8Data[5];
        if (u32DataSize >= (u32PESPayloadLength+6))
        {

            if (u32PESPayloadLength+6 <= TTX_PES_PACKET_LEN )
            {
                s32Ret = Cache_Put(pstDataRecv, pu8Data, u32PESPayloadLength+6);
            }

            /* started  parsing pes packet data  */
//		printf("linda debug %s %d\n", __func__, __LINE__);
            vbi_pes_parse_vsb(pu8Data);
        }
    }
#endif
//printf("linda debug %s %d\n", __func__, __LINE__);	
    return MT_SUCCESS;
}
#endif
