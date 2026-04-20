/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdlib.h>
#include <pthread.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_adp_demux.h"
#include "mt_adp_mpi.h"



/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_CAT_DEBUG

#define MT_CAT_PRINT   printf
#else

#define MT_CAT_PRINT

#endif

#define SAMPLE_CAT_FUNCTION_ENTER()     MT_CAT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_CAT_FUNCTION_EXIT()      MT_CAT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_CAT_FATAL_PRINT(fmt...)          MT_CAT_PRINT(" [FATAL] " fmt)
#define SAMPLE_CAT_ERR_PRINT(fmt...)            MT_CAT_PRINT(" [ERROR] " fmt)
#define SAMPLE_CAT_WARN_PRINT(fmt...)           MT_CAT_PRINT(" [WARN] "  fmt)
#define SAMPLE_CAT_INFO_PRINT(fmt...)           MT_CAT_PRINT(" [INFO] "  fmt)
#define SAMPLE_CAT_DBG_PRINT(fmt...)            MT_CAT_PRINT(" [DEBUG] " fmt)

#define SAMPLE_CAT_PRINT   printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0 0

#define CHANNEL_MAX_PROG    256

#define MAX_SECTION_LEN 4096


#define CAT_TABLE_ID (0x01)

#define PGPAT_TIMEOUT (10000)



/*************************** Structure Definition ****************************/
typedef struct tagSource_Param_T
{
    mt_u8 FileName[256];
}source_param_t;

typedef struct hiCAT_TBL_S
{
    mt_u16 Cattableid;
    mt_u16 CA_system_ID;
    mt_u16 Descriptor_tag;
    mt_u16 Descriptor_length;
    mt_u16 CA_PID;
} MT_CAT_TB;
typedef struct
{
    pthread_t  stInjectTSThread;
} MT_CAT_RUN_INFO;



/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;

static MT_CAT_RUN_INFO    g_stCatRunInfo;


static mt_u32 mt_crcTable[256] =
{
  0x00000000, 0x04C11DB7, 0x9823B6E, 0xD4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2,
  0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3,
  0x3C8EA00A, 0x384FBDBD, 0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
  0x5BD4B01B, 0x569796C2, 0x52568B75,  0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
  0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,  0x9823B6E0, 0x9CE2AB57, 0x91A18D8E,
  0x95609039,0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,  0xBE2B5B58, 0xBAEA46EF,
  0xB7A96036,0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,  0xD4326D90,
  0xD0F37027,0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
  0xF23A8028,0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A,
  0xEC7DD02D,0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C,
  0x2E003DC5,0x2AC12072,  0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x18AEB13,
  0x54BF6A4,0x808D07D, 0xCC9CDCA,  0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB,
  0x6F52C06C, 0x6211E6B5, 0x66D0FB02,  0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
  0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,  0xACA5C697, 0xA864DB20, 0xA527FDF9,
  0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,  0x8AAD2B2F, 0x8E6C3698,
  0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,  0xE0B41DE7,
  0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
  0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED,
  0xD8FBA05A,  0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85,
  0x738AAD5C, 0x774BB0EB,  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
  0x58C1663D, 0x558240E4, 0x51435D53,  0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
  0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,  0x315D626, 0x7D4CB91, 0xA97ED48,
  0xE56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,  0xF12F560E, 0xF5EE4BB9,
  0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,  0xD727BBB6,
  0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
  0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC,
  0xA379DD7B,  0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD,
  0x81B02D74, 0x857130C3,  0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645,
  0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,  0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
  0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,  0x119B4BE9, 0x155A565E, 0x18197087,
  0x1CD86D30, 0x29F3D35, 0x65E2082, 0xB1D065B, 0xFDC1BEC,  0x3793A651, 0x3352BBE6,
  0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 0xC5A92679,
  0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
  0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673,
  0xFDE69BC4,  0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
  0x933EB0BB, 0x97FFAD0C,  0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,0xBCB4666D,
  0xB8757BDA, 0xB5365D03, 0xB1F740B4,
};

#ifdef MT_SAMPLE_APP
    mt_s32 MT_CatMain(mt_s32 argc, mt_char *argv[]);
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static mt_u32 MT_CRC32(mt_u8 *buffer, mt_u32 size)
{
    mt_u32 Result = 0xFFFFFFFF;

    while (size--){
        Result = (Result << 8) ^ mt_crcTable[(Result >> 24) ^ *buffer ++];
    }

    return Result;
}

static void MT_CatDumpData(mt_u8 *head,mt_u32 lens)
{
    mt_u32 i;
    for(i=0;i<lens;i++){
        SAMPLE_CAT_PRINT("%02x ",head[i]);
        if((i&15) == 15){
            SAMPLE_CAT_PRINT("\n");
        }
    }
    SAMPLE_CAT_PRINT("\n");
}


/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_CatDmxInit(MT_VOID)
{
    mt_s32  ret = MT_SUCCESS;

    SAMPLE_CAT_FUNCTION_ENTER();

    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CAT_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CAT_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
        (mt_void)MT_UNF_DMX_DeInit();
        return MT_FAILURE;
    }

    SAMPLE_CAT_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_CatDmxDeInit(MT_VOID)
{
    MT_S32 ret = 0;

    ret = MT_UNF_DMX_DetachTSPort(DMX_ID_0);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CAT_ERR_PRINT(" MT_UNF_DMX_DetachTSPort failed.\n");
    }

    ret = MT_UNF_DMX_DeInit();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CAT_ERR_PRINT(" MT_UNF_DMX_DeInit failed.\n");
        return ret;
    }

    return MT_SUCCESS;
}


/*
@brief Read path file contents into g_hTsBuffer
@param[in] args, Structure of file
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_CatInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_HANDLE hTsBuffer = 0;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    source_param_t *pstParam = (source_param_t *)(args);

    SAMPLE_CAT_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->FileName);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->FileName, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_CAT_ERR_PRINT( "\nfile %s open error!!\n", pstParam->FileName);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CAT_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        (mt_void)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CAT_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*1000, &StreamBuf, 1000);
        if(MT_SUCCESS != ret)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_CAT_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_CAT_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }

    MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);

    return MT_SUCCESS;
}

/*
@brief pat stream type parsing
@param[in] buf, pmt content
@param[in] len, pmt content length
@param[out] pstruProg, Gets the handle to the pmt
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_CatParseCAT( const mt_u8  *pu8SectionData, mt_s32 s32Length, mt_u8 *pSectionStruct)
{

    MT_CAT_TB *pCatTb = (MT_CAT_TB *)pSectionStruct;

    SAMPLE_CAT_FUNCTION_ENTER();

    pCatTb->Cattableid = pu8SectionData[0];
    pCatTb->Descriptor_tag = pu8SectionData[8];
    pCatTb->Descriptor_length = pu8SectionData[9];
    pCatTb->CA_system_ID = (pu8SectionData[10] << 8) | (pu8SectionData[11]);

    pu8SectionData += 12;
    pCatTb->CA_PID = (mt_u16)(((pu8SectionData[0] & 0x1F) << 8) | pu8SectionData[1]);


    SAMPLE_CAT_FUNCTION_EXIT();
    return (MT_SUCCESS);
}

/*
@brief Fetch section packet
@param[in] hChannel, The filter channel created
@param[in] u32TimeOutms, Timeout period
@param[out] pBuf, section content
@param[out] pAcquiredNum, Quantity received
@param[out] pBuffSize, Received content size
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_CatSectionRecvFunc(mt_handle hChannel, mt_u8 *pBuf, mt_u32* pAcquiredNum , mt_u32 * pBuffSize)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u8  u8tableid = 0;
    MT_UNF_DMX_DATA_S sCatSection[32] = { 0 };
    mt_u32 num = 0;
    mt_u32 i = 0, k = 0;
    mt_u32 count = 0 ;
    mt_u32 u32Times = 10;
    mt_u32 u32SecTotalNum=0;
    mt_u32 u32SecNum = 0;
    mt_u32 RequestNum = 32;
    mt_u32 crcSoftware = 0;
    mt_u32 crcStream = 0;
    mt_u8 u8SecGotFlag[MAX_SECTION_NUM] = { 0 };

    memset(u8SecGotFlag, 0, sizeof(u8SecGotFlag[MAX_SECTION_NUM]));

    while (--u32Times)
    {
        s32Ret = MT_UNF_DMX_AcquireBuf(hChannel, RequestNum, &num, sCatSection, (mt_u32)PGPAT_TIMEOUT);
        if ((MT_SUCCESS != s32Ret) || (num == 0)){
            SAMPLE_CAT_ERR_PRINT("MT_UNF_DMX_AcquireBuf failed\n");
            continue;
        }

        for (i = 0; i < num; i++)
        {
            SAMPLE_CAT_INFO_PRINT("sCatSection[%d].enDataType = %d\n",i, sCatSection[i].enDataType);
            (MT_VOID)MT_CatDumpData(sCatSection[i].pu8Data, sCatSection[i].u32Size);

            if (sCatSection[i].enDataType == MT_UNF_DMX_DATA_TYPE_WHOLE)
            {
                u8tableid = sCatSection[i].pu8Data[0];
                u32SecNum = sCatSection[i].pu8Data[6];
                u32SecTotalNum = sCatSection[i].pu8Data[7] + 1;

                SAMPLE_CAT_INFO_PRINT("u32SecNum=%d, u32SecTotalNum=%d,u8tableid=%d u32Size=%d\n",u32SecNum,u32SecTotalNum,u8tableid, sCatSection[i].u32Size);

                k = sCatSection[i].u32Size;
                crcSoftware = MT_CRC32(&(sCatSection[i].pu8Data[0]), sCatSection[i].u32Size - 4);
                crcStream=sCatSection[i].pu8Data[k-4];
                crcStream=(crcStream<<8)|sCatSection[i].pu8Data[k-3];
                crcStream=(crcStream<<8)|sCatSection[i].pu8Data[k-2];
                crcStream=(crcStream<<8)|sCatSection[i].pu8Data[k-1];
                if(crcSoftware != crcStream){
                    SAMPLE_CAT_ERR_PRINT("err.CRC....!!!\n");
                }

                SAMPLE_CAT_INFO_PRINT("crc = 0x%x \n", crcStream);

                if(u8SecGotFlag[u32SecNum] == 0)
                {
                    memcpy((void *)(pBuf + u32SecNum * MAX_SECTION_LEN), sCatSection[i].pu8Data, sCatSection[i].u32Size);
                    u8SecGotFlag[u32SecNum] = 1;
                    pBuffSize[u32SecNum] = sCatSection[i].u32Size;
                    count++;
                }
            }
        }

        SAMPLE_CAT_INFO_PRINT("count=%d\n", count);
        MT_UNF_DMX_ReleaseBuf(hChannel, num, sCatSection);

        /* to check if all sections are received*/
        if(u32SecTotalNum == count)
        {
            break;
        }


    }

    if (u32Times == 0)
    {
        SAMPLE_CAT_ERR_PRINT("MT_UNF_DMX_AcquireBuf time out\n");
        return MT_FAILURE;
    }

    if(pAcquiredNum != MT_NULL)
    {
        *pAcquiredNum = u32SecTotalNum;
    }
    SAMPLE_CAT_INFO_PRINT("download section u8tableid = 0x%x  success !!!\n",u8tableid);

    return MT_SUCCESS;
}

/*
@brief Set the get pat filter
@param[in] pPatTb, pat handle
@param[in] u32DmxId, dmx id
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_CatAcquireSection( MT_CAT_TB *pCatTb, mt_u32 u32DmxId)
{
    mt_u8                    *u8DataBuf = NULL;
    mt_u8                    *p = NULL;
    mt_u32                   u32AquiredNum = 0;
    mt_u32                   u32BufSize[256];
    mt_s32                   s32Ret = MT_FAILURE;
    mt_u32                   i = 0;
    mt_handle                hChan = MT_NULL;
    mt_handle                hFilter = MT_NULL;
    MT_UNF_DMX_CHAN_ATTR_S   stChanAttr= { 0 };
    MT_UNF_DMX_FILTER_ATTR_S stFilterAttr = { 0 };

    u8DataBuf = malloc(MAX_SECTION_LEN * MAX_SECTION_NUM);
    if(NULL==u8DataBuf)
    {
        return MT_FAILURE;
    }
    stChanAttr.u32BufSize = 16 * 1024;
    stChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    stChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
    stChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;

    s32Ret = MT_UNF_DMX_CreateChannel(u32DmxId, &stChanAttr, &hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_CAT_ERR_PRINT("create ch err\n");
        goto FREE_BIGMEM;
    }
    s32Ret = MT_UNF_DMX_SetChannelPID(hChan, CAT_TABLE_ID);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_CAT_ERR_PRINT("set pid err\n");
        goto FREE_CHANNEL;
    }
    stFilterAttr.u32FilterDepth = 1;
    stFilterAttr.au8Match[0] = CAT_TABLE_ID;
    stFilterAttr.au8Mask[0] = 0;
    stFilterAttr.au8Negate[0] = 0;

    s32Ret = MT_UNF_DMX_CreateFilter(u32DmxId, &stFilterAttr, &hFilter);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_CAT_ERR_PRINT("create flt err\n");
        goto FREE_CHANNEL;
    }
    s32Ret = MT_UNF_DMX_SetFilterAttr(hFilter, &stFilterAttr);
    s32Ret |= MT_UNF_DMX_AttachFilter(hFilter, hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_CAT_ERR_PRINT("attach flt err\n");
        goto FREE_FILTER;
    }
    s32Ret = MT_UNF_DMX_OpenChannel(hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_CAT_ERR_PRINT("open chnl err\n");
        goto DETCH_FILTER;
    }

    memset(u8DataBuf, 0, sizeof(MAX_SECTION_LEN * MAX_SECTION_NUM));
    memset(u32BufSize, 0, sizeof(u32BufSize));
    s32Ret = MT_CatSectionRecvFunc(hChan, u8DataBuf, &u32AquiredNum , u32BufSize);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_CAT_ERR_PRINT("close ch err\n");
    }
    p = u8DataBuf;
    for(i=0; i < u32AquiredNum; i++)
    {
        MT_CatParseCAT(p, u32BufSize[i], (mt_u8 *)pCatTb);
        p = p + MAX_SECTION_LEN;
    }

    s32Ret = MT_UNF_DMX_CloseChannel(hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_CAT_ERR_PRINT("close ch err\n");
    }
    DETCH_FILTER:
    s32Ret = MT_UNF_DMX_DetachFilter(hFilter, hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_CAT_ERR_PRINT("Det flt err\n");
    }
    FREE_FILTER:
    s32Ret = MT_UNF_DMX_DestroyFilter(hFilter);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_CAT_ERR_PRINT("Del flt err\n");
    }
    FREE_CHANNEL:
    s32Ret = MT_UNF_DMX_DestroyChannel(hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_CAT_ERR_PRINT("Del ch err\n");
    }
    FREE_BIGMEM:
    free(u8DataBuf);
    return s32Ret;
}

static void MT_CatPrintInfo(MT_CAT_TB *pCatTb)
{
    MT_CAT_PRINT(" Get CAT info --------------------------------\n");
    SAMPLE_CAT_INFO_PRINT("       Cattableid = 0x%x \n", pCatTb->Cattableid);
    SAMPLE_CAT_INFO_PRINT("     CA_system_ID = 0x%x \n", pCatTb->CA_system_ID);
    SAMPLE_CAT_INFO_PRINT("   descriptor_tag = 0x%x \n", pCatTb->Descriptor_tag);
    SAMPLE_CAT_INFO_PRINT("descriptor_length = 0x%x \n", pCatTb->Descriptor_length);
    SAMPLE_CAT_INFO_PRINT("           CA_PID = 0x%x \n", pCatTb->CA_PID);
    MT_CAT_PRINT(" Get CAT info end! ---------------------------\n");
}


/*
@brief help
@return void
*/
static void MT_CatPrint_help(char *name)
{
    SAMPLE_CAT_INFO_PRINT("Lack of parameters\n");
    SAMPLE_CAT_INFO_PRINT("such as: %s -f ./1.ts\n", name);
}
static MT_VOID MT_CatPrintMenu(void)
{


    SAMPLE_CAT_PRINT("     h : help \n");
    SAMPLE_CAT_PRINT("     q : quit \n");
    SAMPLE_CAT_PRINT("CAT>> ");

}
static MT_VOID MT_CatExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stCatRunInfo.stInjectTSThread, NULL);
    (MT_VOID)MT_CatDmxDeInit();
    memset(&g_stCatRunInfo, 0xff, sizeof(g_stCatRunInfo));
}


/*
@brief quit
@return void
*/
static void MT_CatCmdTask(void)
{
    char *fgetret = NULL;
    MT_CHAR inputCmd[32] = { 0 };

    while (1)
    {
        (MT_VOID)MT_CatPrintMenu();
        fgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret = fgetret;


        if ('q' == inputCmd[0])
        {
            SAMPLE_CAT_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }

        else if('h' == inputCmd[0])
        {
            SAMPLE_CAT_INFO_PRINT("help info\n");
        }
    }
}

static MT_S32 MT_Catparase_args(MT_S32 argc, MT_CHAR *argv[], source_param_t *pInparam)
{
    MT_S32 opt = 0;

    SAMPLE_CAT_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHqf:")) != -1)
    {
         switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_CatPrint_help(argv[0]);
                return MT_FAILURE;

            case 'f':
                 MTADP_Strncpy((mt_char*)pInparam->FileName, mt_optarg, sizeof(source_param_t));
                break;
            default:
                (void)MT_CatPrint_help(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_CAT_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
    mt_s32 MT_CatMain(mt_s32 argc, mt_char *argv[])
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32 ret = 0;
    MT_CAT_TB sCatTb = { 0 };
    source_param_t stParam = { 0 };

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_CatPrint_help(argv[0]);
        return MT_FAILURE;
    }

    ret = MT_Catparase_args(argc, argv, &stParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_CAT_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }

    if(MT_TRUE == g_bTaskQuit)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_CAT_ERR_PRINT("mt_sys_init failed.\n");
            return ret;
        }
#endif

    ret = MT_CatDmxInit();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CAT_ERR_PRINT("failed to StartDmx\n");
        goto ERR0;
    }

    g_bTaskQuit = MT_FALSE;
    ret = pthread_create(&g_stCatRunInfo.stInjectTSThread, NULL, (void * (*)(void *))MT_CatInjectTsTask, &stParam);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CAT_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
        goto ERR1;
    }
    sleep(1);
    if(g_bTaskQuit == MT_TRUE)
    {
        goto ERR1;
    }


    ret = MT_CatAcquireSection(&sCatTb, DMX_ID_0);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_CAT_ERR_PRINT("MT_AcquireNITSection failed, ret = %d\n", ret);
        goto ERR1;
    }

    (MT_VOID)MT_CatPrintInfo(&sCatTb);
    }

    (MT_VOID)MT_CatCmdTask();
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }


ERR1:
    (MT_VOID)MT_CatExit();

ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    return MT_SUCCESS;
}
