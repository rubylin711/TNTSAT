#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mt_type.h"
#include "subtitle_debug.h"
#include "scte_subt_data.h"
#include "scte_subt_parse.h"

#define MT_SECTION_MAX_SIZE (0x10000) /* 64K */
#define SCTE_SECTION_MAX_SIZE (0x400)  /*1K*/
#define MT_SECTION_MAX_NUM (64)

typedef struct tagSCTE_SUBT_SECTION_S
{
    mt_u8   au8Data[SCTE_SECTION_MAX_SIZE];
    mt_u32  u32DataSize;
    mt_u16  u16TableExtention;
    MT_BOOL bUsed;
} SCTE_SUBT_SECTION_S;

typedef struct tagSCTE_SUBT_DATA_RECV_S
{
    MT_HANDLE           hParse;
    SCTE_SUBT_SECTION_S astSCTESection[MT_SECTION_MAX_NUM];
    mt_u8               au8SectionAddr[MT_SECTION_MAX_SIZE];
    mt_u8               *pu8SectionWriteAddr;
    mt_u32              u32SectionSize;
} SCTE_SUBT_DATA_RECV_S;

static mt_void BufferReset(SCTE_SUBT_DATA_RECV_S *pstDataRecv)
{
    mt_u32 i=0;

    for (i = 0; i < MT_SECTION_MAX_NUM; i++)
    {
        if (pstDataRecv->astSCTESection[i].bUsed)
        {
            pstDataRecv->astSCTESection[i].bUsed = MT_FALSE;
        }
    }
}

mt_s32 SCTE_SUBT_Data_Init(mt_void)
{
    return MT_SUCCESS;
}

mt_s32 SCTE_SUBT_Data_DeInit(mt_void)
{
    return MT_SUCCESS;
}

mt_s32 SCTE_SUBT_Data_Create(MT_HANDLE hParse, MT_HANDLE *phData)
{
    mt_s32 s32Ret = MT_SUCCESS;
    SCTE_SUBT_DATA_RECV_S *pstDataRecv = MT_NULL;

    pstDataRecv = (SCTE_SUBT_DATA_RECV_S*)malloc(sizeof(SCTE_SUBT_DATA_RECV_S));
    if (MT_NULL == pstDataRecv)
    {
        MT_ERR_SUBT("malloc data struct failure!!\n");
        return MT_FAILURE;
    }

    memset(pstDataRecv, 0, sizeof(SCTE_SUBT_DATA_RECV_S));
    pstDataRecv->hParse = hParse;
    *phData = (MT_HANDLE)pstDataRecv;

    MT_INFO_SUBT("SCTE_SUBT_Data_Create success, with handle:0x%08x!...\n", *phData);

    return s32Ret;
}

mt_s32 SCTE_SUBT_Data_Destroy(MT_HANDLE hData)
{
    SCTE_SUBT_DATA_RECV_S *pstDataRecv = (SCTE_SUBT_DATA_RECV_S *)hData;

    if (MT_NULL == pstDataRecv)
    {
        MT_ERR_SUBT("SCTE_SUBT_Data_Destroy  param is MT_NULL!!\n");
        return MT_FAILURE;
    }

    MT_INFO_SUBT("begin to use handle:0x%08x!...\n", pstDataRecv);

    free((void*)pstDataRecv);

    MT_INFO_SUBT("SCTE_SUBT_Data_Destroy  success...\n");

    return MT_SUCCESS;
}

mt_s32 SCTE_SUBT_Data_Reset(MT_HANDLE hData)
{
    SCTE_SUBT_DATA_RECV_S *pstDataRecv = (SCTE_SUBT_DATA_RECV_S *)hData;

    if (MT_NULL == pstDataRecv)
    {
        MT_ERR_SUBT("param is MT_NULL!!\n");
        return MT_FAILURE;
    }

    BufferReset(pstDataRecv);

    return MT_SUCCESS;
}

mt_s32 SCTE_SUBT_Data_Inject(MT_HANDLE hData, const mt_u8 *pu8Data, mt_u32 u32DataSize)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u16 u16TableExtension = 0;
    mt_u16 u16LastSegment   = 0;
    mt_u16 u16SegmentNumber = 0;
    mt_u8  i = 0;
    SCTE_SUBT_DATA_RECV_S *pstDataRecv = (SCTE_SUBT_DATA_RECV_S *)hData;

    if ((MT_NULL == pstDataRecv) || (MT_NULL == pu8Data) || (0 == u32DataSize))
    {
        MT_ERR_SUBT("parameter is invalid!!\n");
        return MT_FAILURE;
    }

    if (u32DataSize > SCTE_SECTION_MAX_SIZE)
    {
        MT_ERR_SUBT("Data Size too long!!\n");
        return MT_FAILURE;
    }

    pstDataRecv->pu8SectionWriteAddr = pstDataRecv->au8SectionAddr;

    /*scte subtitle table_id:0xC6*/
    if (pu8Data[0] == 0xC6)
    {
        /*if the section is segmented*/
        if (pu8Data[3] & 0x40)
        {
            u16TableExtension = (mt_u16)((pu8Data[4] << 8) | pu8Data[5]);
            u16LastSegment   = (mt_u16)((pu8Data[6] << 8) | pu8Data[7]) >> 4;
            u16SegmentNumber = ((pu8Data[7] << 8) | pu8Data[8]) & 0x0fff;

            for (i = 0; i < MT_SECTION_MAX_NUM; i++)
            {
                if (pstDataRecv->astSCTESection[i].bUsed == MT_FALSE)
                {
                    if (u16SegmentNumber == 0)      // first segment
                    {
                        /*skip 5bytes indicates if is segmented*/

                        memcpy(pstDataRecv->astSCTESection[i].au8Data, pu8Data, 4);
                        memcpy(pstDataRecv->astSCTESection[i].au8Data + 4, pu8Data + 9, u32DataSize - 9 - 4);

                        pstDataRecv->astSCTESection[i].u32DataSize = u32DataSize - 9;
                        pstDataRecv->astSCTESection[i].u16TableExtention = u16TableExtension;

                    }
                    else if (u16SegmentNumber == u16LastSegment)
                    {
                        /*skip 5bytes indicates if is segmented and 4bytes header*/
                        memcpy(pstDataRecv->astSCTESection[i].au8Data, pu8Data + 9, u32DataSize - 9);
                        pstDataRecv->astSCTESection[i].u32DataSize = u32DataSize - 9;
                        pstDataRecv->astSCTESection[i].u16TableExtention = u16TableExtension;

                    }
                    else
                    {
                        memcpy(pstDataRecv->astSCTESection[i].au8Data, pu8Data + 9, u32DataSize - 9 - 4);
                        pstDataRecv->astSCTESection[i].u32DataSize = u32DataSize - 9 - 4;
                        pstDataRecv->astSCTESection[i].u16TableExtention = u16TableExtension;
                    }

                    pstDataRecv->astSCTESection[i].bUsed = MT_TRUE;
                    break;
                }
            }

            if (u16LastSegment == u16SegmentNumber)
            {
                pstDataRecv->pu8SectionWriteAddr = pstDataRecv->au8SectionAddr;
                for (i = 0; i < MT_SECTION_MAX_NUM; i++)
                {
                    if (pstDataRecv->astSCTESection[i].bUsed
                        && (pstDataRecv->astSCTESection[i].u16TableExtention == u16TableExtension))
                    {
                        memcpy(pstDataRecv->pu8SectionWriteAddr, pstDataRecv->astSCTESection[i].au8Data,
                               pstDataRecv->astSCTESection[i].u32DataSize);
                        pstDataRecv->pu8SectionWriteAddr += pstDataRecv->astSCTESection[i].u32DataSize;
                        pstDataRecv->astSCTESection[i].bUsed = MT_FALSE;
                    }
                }

                pstDataRecv->u32SectionSize = (mt_u32)(pstDataRecv->pu8SectionWriteAddr - pstDataRecv->au8SectionAddr /* - 1 TODO*/);

                /* update section size in head */
                pstDataRecv->au8SectionAddr[2] = (mt_u8)(pstDataRecv->u32SectionSize - 3);
                pstDataRecv->au8SectionAddr[1] = (mt_u8)((pstDataRecv->u32SectionSize - 3) >> 8);
                if (pstDataRecv->u32SectionSize > MT_SECTION_MAX_SIZE)
                {
                    MT_ERR_SUBT("Section Size too long!!\n");
                    return MT_FAILURE;
                }

                //buffer over,reset
                //BufferReset(pstDataRecv);

                if (pstDataRecv->hParse)
                {
                    s32Ret = SCTE_SUBT_Parse_ParseSection(pstDataRecv->hParse, pstDataRecv->au8SectionAddr,
                                                          pstDataRecv->u32SectionSize);
                    if (s32Ret != MT_SUCCESS)
                    {
                        MT_ERR_SUBT("Failed to SCTE_SUBT_Parse_ParseSection...\n");

                        return s32Ret;
                    }
                }
            }
        }
        else  //not segmented
        {
            memcpy(pstDataRecv->au8SectionAddr, pu8Data, u32DataSize);
            pstDataRecv->u32SectionSize = u32DataSize;
            if (pstDataRecv->hParse)
            {
                s32Ret = SCTE_SUBT_Parse_ParseSection(pstDataRecv->hParse, pstDataRecv->au8SectionAddr,
                                                      pstDataRecv->u32SectionSize);
                if (s32Ret != MT_SUCCESS)
                {
                    MT_ERR_SUBT("Failed to SCTE_SUBT_Parse_ParseSection...\n");

                    return s32Ret;
                }
            }
        }
    }
    else  /* pu8Data[0] is not 0xC6 */
    {
        MT_ERR_SUBT("No scte subtitle section data!!\n");
        return MT_FAILURE;
    }

    if (pstDataRecv->u32SectionSize < MT_SECTION_MAX_SIZE)
    {
        MT_INFO_SUBT("Success to desegmentation...\n");
    }
    else
    {
        MT_ERR_SUBT("Section is too large!!\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}
