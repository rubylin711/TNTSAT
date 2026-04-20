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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_adp_search.h"
#include "mt_adp_demux.h"
#include "mt_adp_mpi.h"


/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_PMT_DEBUG

#define MT_PMT_PRINT   printf
#else

#define MT_PMT_PRINT

#endif

#define SAMPLE_PMT_FUNCTION_ENTER()     MT_PMT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_PMT_FUNCTION_EXIT()      MT_PMT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_PMT_FATAL_PRINT(fmt...)          MT_PMT_PRINT(" [FATAL] " fmt)
#define SAMPLE_PMT_ERR_PRINT(fmt...)            MT_PMT_PRINT(" [ERROR] " fmt)
#define SAMPLE_PMT_WARN_PRINT(fmt...)           MT_PMT_PRINT(" [WARN] "  fmt)
#define SAMPLE_PMT_INFO_PRINT(fmt...)           MT_PMT_PRINT(" [INFO] "  fmt)
#define SAMPLE_PMT_DBG_PRINT(fmt...)            MT_PMT_PRINT(" [DEBUG] " fmt)

#define SAMPLE_PMT_PRINT printf
#define DMX_ID_0 0

#define MAX_SECTION_LEN 4096


#define PGPAT_TIMEOUT (10000)
#define PGPMT_TIMEOUT (2000)
#define PGSDT_TIMEOUT (10000)



/*************************** Structure Definition ****************************/
typedef struct tagSource_Param_T
{
    mt_u8 FileName[256];
}source_param_t;
typedef struct
{
    pthread_t  stInjectTSThread;
} MT_PMT_RUN_INFO;


#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2


/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
 static MT_PMT_RUN_INFO    g_stPmtRunInfo;


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
    mt_s32 MT_PmtMain(mt_s32 argc, mt_char *argv[]);
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


static void MT_PMTSampleDumpData(mt_u8 *head,mt_u32 lens)
{
    mt_u32 i;
    for(i=0;i<lens;i++){
        printf("%02x ",head[i]);
        if((i&15) == 15){
            printf("\n");
        }
    }
    printf("\n");
}

/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_PmtDmxInit(void)
{
    mt_s32  ret = MT_SUCCESS;

    SAMPLE_PMT_FUNCTION_ENTER();

    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PMT_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PMT_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
        (mt_void)MT_UNF_DMX_DeInit();
        return MT_FAILURE;
    }

    SAMPLE_PMT_FUNCTION_EXIT();

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return void
*/
static void MT_PmtDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    (MT_VOID)MT_UNF_DMX_DeInit();

}


/*
@brief Read path file contents into g_hTsBuffer
@param[in] args, Structure of file
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_PmtInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_HANDLE hTsBuffer = 0;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    source_param_t *pstParam = (source_param_t *)(args);

    SAMPLE_PMT_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->FileName);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->FileName, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_PMT_ERR_PRINT( "\nfile %s open error!!\n", pstParam->FileName);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_PMT_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        (mt_void)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PMT_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
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
            SAMPLE_PMT_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_PMT_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
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
@brief AC3 audio stream or not
@param[in] buf, pmt content
@param[in] len, pmt content length
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_BOOL MT_PmtIsAC3Stream(const mt_u8 *buf, mt_u32 len)
{
    while (len >= 2)
    {
        mt_u32 tag      = buf[0];
        mt_u32 tag_len  = buf[1];

        buf += 2;
        len -= 2;

        switch (tag)
        {
            case AC3_DESCRIPTOR:
            case AC3_PLUS_DESCRIPTOR:
            case AC3_EXT_DESCRIPTOR:
                return MT_TRUE;

            default:
                buf += tag_len;
                if (tag_len >= len)
                {
                    len = 0;
                }
                else
                {
                    len -= tag_len;
                }
        }
    }

    return MT_FALSE;
}

/*
@brief Whether it is a ttx stream
@param[in] buf, pmt content
@param[in] len, pmt content length
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_BOOL MT_PmtIsTtxStream(const mt_u8 *pu8Data, mt_s32 s32Length)
{
    MT_BOOL bRet = MT_FALSE;

    int nLen = s32Length;
    mt_u8 *pData = (mt_u8*)pu8Data;
    mt_u8 u8Tag = 0;
    mt_u8 u8Length = 0;

    while (nLen > 0)
    {
        u8Tag = *pData++;
        u8Length = *pData++;

        if (u8Tag == TELETEXT_DESCRIPTOR)
        {
            bRet = MT_TRUE;
            break;
        }

        pData += u8Length;

        nLen -= (u8Length+2);
    }

    return bRet;
}

/*
@brief Whether it is an ARIBCC stream
@param[in] buf, pmt content
@param[in] len, pmt content length
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_BOOL MT_PmtIsARIBCCStream(const mt_u8 *pu8Data, mt_s32 s32Length)
{
    MT_BOOL bRet = MT_FALSE;

    int nLen = s32Length;
    mt_u8 *pData = (mt_u8*)pu8Data;
    mt_u8 u8Tag = 0;
    mt_u8 u8Length = 0;
    mt_u8 u8ComponentTag = 0;

    while (nLen > 0)
    {
        u8Tag = *pData++;
        u8Length = *pData++;
        if (u8Tag == STREAM_IDENTIFIER_DESCRIPTOR)
        {
            u8ComponentTag = *pData;

            if((0x30 == u8ComponentTag) || (0x87 == u8ComponentTag))
            {
                bRet = MT_TRUE;
            }
            break;
        }

        pData += u8Length;

        nLen -= (u8Length+2);
    }

    return bRet;
}

/*
@brief Whether it is a Subtitle stream
@param[in] buf, pmt content
@param[in] len, pmt content length
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static MT_BOOL MT_PmtIsSubtitleStream(const mt_u8 *pu8Data, mt_s32 s32Length)
{
    MT_BOOL bRet = MT_FALSE;

    int nLen = s32Length;
    mt_u8 *pData = (mt_u8*)pu8Data;
    mt_u8 u8Tag = 0;
    mt_u8 u8Length = 0;

    while (nLen > 0)
    {
        u8Tag = *pData++;
        u8Length = *pData++;

        if (u8Tag == SUBTITLING_DESCRIPTOR)
        {
            bRet = MT_TRUE;
            break;
        }

        pData += u8Length;

        nLen -= (u8Length+2);
    }

    return bRet;
}

/*
@brief pmt stream type parsing
@param[in] buf, pmt content
@param[in] len, pmt content length
@param[out] pstruProg, Gets the handle to the pmt
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_void MT_PmtParsePMTDesc( const mt_u8* pu8DesData, mt_s32 s32DescLength, PMT_TB* pstruProg)
{
    mt_u8 u8Tag = 0;
    mt_u8 u8Length = 0;
    mt_u8 u8CntIndex = 0;

    if ((MT_NULL == pu8DesData) || (MT_NULL == pstruProg))
    {
        return;
    }

    while (s32DescLength > 0)
    {
        u8Tag = *pu8DesData++;
        u8Length = *pu8DesData++;
        if (u8Length == 0)
        {
            return;
        }

        switch (u8Tag)
        {
            case LANGUAGE_DESCRIPTOR:
            {
                break;
            }

            case CA_DESCRIPTOR:
            {
                pstruProg->CASystem[pstruProg->u16CANum].u16CASystemID = pu8DesData[0] << 8 | pu8DesData[1];
                pstruProg->CASystem[pstruProg->u16CANum].u16CAPID = pu8DesData[2] << 8 | pu8DesData[3];
                pstruProg->u16CANum++;
                break;
            }

            case TELETEXT_DESCRIPTOR:
            {
                mt_u16 ii = 0;
                mt_u16 nCount = u8Length / 5;
                mt_u16 u16TtxIndex = pstruProg->u16TtxNum;

                for(ii = 0; ii < nCount; ii++)
                {
                    u8CntIndex = pstruProg->stTtxInfo[u16TtxIndex].u8DesInfoCnt;
                    if ((s32DescLength >= 5) && u8CntIndex < TTX_DES_MAX)
                    {
                        /* 8859-1 language code */
                        pstruProg->stTtxInfo[u16TtxIndex].stTtxDes[u8CntIndex].u32ISO639LanguageCode =
                                           (pu8DesData[ii*5+0] << 16) | (pu8DesData[ii*5 + 1] << 8) | (pu8DesData[ii*5 + 2]);
                        /* teletext_type */
                        pstruProg->stTtxInfo[u16TtxIndex].stTtxDes[u8CntIndex].u8TtxType = (pu8DesData[ii*5 + 3] >> 3) & 0x1f;
                        /* teletext_magazine_number */
                        pstruProg->stTtxInfo[u16TtxIndex].stTtxDes[u8CntIndex].u8TtxMagazineNumber = pu8DesData[ii*5 + 3] & 0x7;
                        /* teletext_page_number */
                        pstruProg->stTtxInfo[u16TtxIndex].stTtxDes[u8CntIndex].u8TtxPageNumber = pu8DesData[ii*5 + 4];
                        pstruProg->stTtxInfo[u16TtxIndex].u8DesInfoCnt++;
                    }
                }
                break;
            }

            case SUBTITLING_DESCRIPTOR:
            {
                mt_u16 ii = 0;
                mt_u16 nCount = u8Length / 8; /*8 =  3 + 1 + 2 + 2*/
                mt_u16 u16SubtIndex = pstruProg->u16SubtitlingNum;

                for(ii=0; ii<nCount; ii++)
                {
                    u8CntIndex = pstruProg->SubtitingInfo[u16SubtIndex].u8DesInfoCnt;
                    if ((s32DescLength >= 8) && u8CntIndex < SUBTDES_INFO_MAX)
                    {
                        /* 8859-1 language code */
                        pstruProg->SubtitingInfo[u16SubtIndex].DesInfo[u8CntIndex].u32LangCode =
                                           (pu8DesData[ii*8+0] << 16) | (pu8DesData[ii*8 + 1] << 8) | (pu8DesData[ii*8 + 2]);

                        /* page id */
                        pstruProg->SubtitingInfo[u16SubtIndex].DesInfo[u8CntIndex].u16PageID = (pu8DesData[ii*8 + 4] << 8) | (pu8DesData[ii*8 + 5]);

                        /* ancilary page id */
                        pstruProg->SubtitingInfo[u16SubtIndex].DesInfo[u8CntIndex].u16AncillaryPageID = (pu8DesData[ii*8 + 6] << 8) | (pu8DesData[ii*8 + 7]);

                        pstruProg->SubtitingInfo[u16SubtIndex].u8DesInfoCnt++;
                    }
                }

                break;
            }

            case AC3_DESCRIPTOR:
            case AC3_PLUS_DESCRIPTOR:
                break;

            case COPYRIGHT_DESCRIPTOR:
            case MOSAIC_DESCRIPTOR:
                break;

            case STREAM_IDENTIFIER_DESCRIPTOR:
            case PRIVATE_DATA_SPECIFIER_DESCRIPTOR:
            case SERVICE_MOVE_DESCRIPTOR:
            case CA_SYSTEM_DESCRIPTOR:
            case DATA_BROADCAST_ID_DESCRIPTOR:
                break;

            case CAPTION_SERVICE_DESCRIPTOR:
            {
                mt_u8 ServiceNum = 0;
                mt_u8 u8Index = 0;
                mt_u32 u32LangCode = 0;
                mt_u8 j = 0;

                ServiceNum = pu8DesData[0] & 0x1f;
                pstruProg->u16ClosedCaptionNum = ServiceNum;
                j ++;
                while(j < u8Length)
                {
                    /* Ref : 6.9.2 Caption Service Descriptor in ATSC a/65 */
                    for(u8Index = 0; u8Index < ServiceNum; u8Index++)
                    {
                        u32LangCode = (pu8DesData[j] << 16 | pu8DesData[j+1] << 8 | pu8DesData[j+2]);
                        j += 3;

                        pstruProg->stClosedCaption[u8Index].u8IsDigitalCC = pu8DesData[j] & 0x80;
                        if(pstruProg->stClosedCaption[u8Index].u8IsDigitalCC)
                        {
                            pstruProg->stClosedCaption[u8Index].u32LangCode = u32LangCode;
                            pstruProg->stClosedCaption[u8Index].u8ServiceNumber = pu8DesData[j] & 0x3f;
                            j += 1;
                            pstruProg->stClosedCaption[u8Index].u8IsEasyReader = pu8DesData[j] & 0x80;
                            pstruProg->stClosedCaption[u8Index].u8IsWideAspectRatio = pu8DesData[j] & 0x40;
                            j += 2;
                        }
                        else
                        {
                            j += 3;
                        }
                    }
                }
                break;
            }

            default:
                break;
        }

        pu8DesData    += u8Length;
        s32DescLength -= (u8Length + 2);
    }
}

/*
@brief pat stream type parsing
@param[in] buf, pmt content
@param[in] len, pmt content length
@param[out] pstruProg, Gets the handle to the pmt
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_PmtParsePAT( const mt_u8  *pu8SectionData, mt_s32 s32Length, mt_u8 *pSectionStruct)
{
    mt_u16 u16ProgrameNumber = 0;
    mt_u16 u16Pid  = 0x1FFF;
    mt_u16 u16TsId = 0;

    PAT_INFO *pPatInfo;
    PAT_TB *pPatTb = (PAT_TB *)pSectionStruct;
    if ((MT_NULL == pu8SectionData))
    {
        return MT_FAILURE;
    }

    if (PAT_TABLE_ID != pu8SectionData[0])
    {
        return MT_FAILURE;
    }

    s32Length = (mt_s32)((pu8SectionData[1] << 8) | pu8SectionData[2]) & 0x0fff;

    u16TsId = (mt_u16)((pu8SectionData[3] << 8) | pu8SectionData[4]);

    pPatTb->u16TsID = u16TsId;

    pu8SectionData += 8;
    s32Length -= 9;

    while (s32Length >= 4)
    {
        pPatInfo = &(pPatTb->PatInfo[pPatTb->u16ProgNum]);

        u16ProgrameNumber = (mt_u16)((pu8SectionData[0] << 8) | pu8SectionData[1]);

        u16Pid = (mt_u16)(((pu8SectionData[2] & 0x1F) << 8) | pu8SectionData[3]);

        if (u16ProgrameNumber != 0x0000)
        {
            pPatInfo->u16ServiceID = u16ProgrameNumber;
            pPatInfo->u16PmtPid = u16Pid;
            pPatTb->u16ProgNum++;


        }

        pu8SectionData += 4;
        s32Length -= 4;
    }

    return (MT_SUCCESS);
}


/*
@brief pmt stream type parsing
@param[in] buf, pmt content
@param[in] len, pmt content length
@param[out] pstruProg, Gets the handle to the pmt
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_PmtParsePMT( const mt_u8 *pu8SectionData, mt_s32 s32Length, mt_u8 *pSectionStruct)
{
    mt_u8 u8StreamType = 0;
    mt_u16 u16DesLen = 0;
    mt_u16 u16Pid  = 0x1FFF;

    PMT_TB *pPmtTb = (PMT_TB *)pSectionStruct;
    if (MT_NULL == pu8SectionData)
    {
        return MT_FAILURE;
    }

    if (PMT_TABLE_ID != pu8SectionData[0])
    {
        return MT_FAILURE;
    }

    memcpy(pPmtTb->u8PmtData, pu8SectionData, s32Length);
    pPmtTb->u32PmtLen = s32Length;

    s32Length = (mt_s32)((pu8SectionData[1] << 8) | pu8SectionData[2]) & 0x0fff;


    pPmtTb->u16ServiceID = (mt_u16)((pu8SectionData[3] << 8) | pu8SectionData[4]);

    pu8SectionData += 8; /* skip 8-byte to PCR_PID*/

    u16Pid = (mt_u16)(((pu8SectionData[0] & 0x1F) << 8) | pu8SectionData[1]);
    u16DesLen = (mt_u16)(((pu8SectionData[2] & 0x0F) << 8) | pu8SectionData[3]);

    pPmtTb->u16PcrPid = u16Pid;

    pu8SectionData += 4;

    s32Length -= 9;


    if (u16DesLen > 0)
    {
        MT_PmtParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
        pu8SectionData += u16DesLen;
        s32Length -= u16DesLen;
    }

    while (s32Length > 4)
    {
        u8StreamType = pu8SectionData[0];

        u16Pid = (((pu8SectionData[1] & 0x1F) << 8) | pu8SectionData[2]);
        u16DesLen = (((pu8SectionData[3] & 0x0F) << 8) | pu8SectionData[4]);

        pu8SectionData += 5;
        s32Length -= 5;

        SAMPLE_PMT_INFO_PRINT("stream type : %#x, pid:%#x, length:%d, u16DesLen = %d\n", u8StreamType, u16Pid, s32Length, u16DesLen);
        switch (u8StreamType)
        {
            /*video stream*/
        case STREAM_TYPE_14496_10_VIDEO:
        case STREAM_TYPE_14496_2_VIDEO:
        case STREAM_TYPE_11172_VIDEO:
        case STREAM_TYPE_13818_VIDEO:
        case STREAM_TYPE_AVS_VIDEO:
        case STREAM_TYPE_HEVC_VIDEO:
        case STREAM_TYPE_VC1_VIDEO:
        {
            if (pPmtTb->u16VideoNum < PROG_MAX_VIDEO)
            {
                pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u16VideoPid = u16Pid;

                if (u8StreamType == STREAM_TYPE_14496_10_VIDEO)
                {
                    pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u32VideoEncType = MT_UNF_VCODEC_TYPE_H264;
                }
                else if (u8StreamType == STREAM_TYPE_14496_2_VIDEO)
                {
                    pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u32VideoEncType = MT_UNF_VCODEC_TYPE_MPEG4;
                }
                else if (u8StreamType == STREAM_TYPE_AVS_VIDEO)
                {
                    pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u32VideoEncType = MT_UNF_VCODEC_TYPE_AVS;
                }
                else if (u8StreamType == STREAM_TYPE_HEVC_VIDEO)
                {
                    pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u32VideoEncType = MT_UNF_VCODEC_TYPE_HEVC;
                }
                else if (u8StreamType == STREAM_TYPE_VC1_VIDEO)
                {
                    pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u32VideoEncType = MT_UNF_VCODEC_TYPE_VC1;
                }
                else
                {
                    pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u32VideoEncType = MT_UNF_VCODEC_TYPE_MPEG2;
                }

                if (u16DesLen > 0)
                {
                    MT_PmtParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
                    pu8SectionData += u16DesLen;
                    s32Length -= u16DesLen;
                }

                pPmtTb->u16VideoNum++;
            }

            break;
        }

        /*audio stream*/
        case STREAM_TYPE_11172_AUDIO:
        case STREAM_TYPE_13818_AUDIO:
        case STREAM_TYPE_14496_3_AUDIO:
        case STREAM_TYPE_13818_7_AUDIO:
        case STREAM_TYPE_AC3_AUDIO:
        case STREAM_TYPE_DOLBY_TRUEHD_AUDIO:
        {
            if(pPmtTb->u16AudoNum < PROG_MAX_AUDIO)
            {
                pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u16AudioPid = u16Pid;
                if ((u8StreamType == STREAM_TYPE_13818_7_AUDIO) || (u8StreamType == STREAM_TYPE_14496_3_AUDIO))
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_AAC;
                }
                else if (u8StreamType == STREAM_TYPE_AC3_AUDIO)
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DOLBY_PLUS;
                }
                else if (u8StreamType == STREAM_TYPE_DOLBY_TRUEHD_AUDIO)
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DOLBY_TRUEHD;
                }
                else if (u8StreamType == STREAM_TYPE_DTS_AUDIO)
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DTSHD;
                }
                else
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_MP3;
                }

                if (u16DesLen > 0)
                {
                    MT_PmtParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
                    pu8SectionData += u16DesLen;
                    s32Length -= u16DesLen;
                }

                pPmtTb->u16AudoNum++;
            }

            break;
        }
        case STREAM_TYPE_PRIVATE:
        {
            if (u16DesLen > 0 )/* subtitling stream info */
            {
                if  (MT_PmtIsSubtitleStream(pu8SectionData, u16DesLen))
                {
                    if (SUBTITLING_MAX > pPmtTb->u16SubtitlingNum)
                    {
                        pPmtTb->SubtitingInfo[pPmtTb->u16SubtitlingNum].u16SubtitlingPID = u16Pid;


                        MT_PmtParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);

                        pPmtTb->u16SubtitlingNum++;
                    }
                    else
                    {
                        SAMPLE_PMT_ERR_PRINT("Subtitle language is over than the max number:%d\n", SUBTITLING_MAX);
                    }
                }
                else if (MT_PmtIsARIBCCStream(pu8SectionData, u16DesLen))
                {
                    pPmtTb->u16ARIBCCPid = u16Pid;
                }
                else if (MT_PmtIsTtxStream(pu8SectionData, u16DesLen))
                {
                    if (TTX_MAX > pPmtTb->u16TtxNum)
                    {
                        pPmtTb->stTtxInfo[pPmtTb->u16TtxNum].u16TtxPID = u16Pid;

                        MT_PmtParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);

                        pPmtTb->u16TtxNum++;
                    }
                }
                else
                {
                    if (pPmtTb->u16AudoNum < PROG_MAX_AUDIO)
                    {
                        pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u16AudioPid = u16Pid;

                        // dolby parse
                        if (MT_PmtIsAC3Stream(pu8SectionData, u16DesLen))
                        {
                            pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DOLBY_PLUS;
                        }
                        else
                        {
                            pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_MP3;
                        }

                        pPmtTb->u16AudoNum++;
                    }
                }

                pu8SectionData += u16DesLen;
                s32Length -= u16DesLen;
            }

        }
        break;


        case STREAM_TYPE_SCTE:
        {
            if (u16DesLen > 0)
            {
                MT_PmtParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
                pu8SectionData += u16DesLen;
                s32Length -= u16DesLen;
            }
            if (SCTE_SUBTITLE_MAX > pPmtTb->u16SCTESubtNum)
            {
                pPmtTb->stSCTESubtInfo[pPmtTb->u16SCTESubtNum].u16SCTESubtPID = u16Pid;
            
                pPmtTb->u16SCTESubtNum++;
            }
            else
            {
                SAMPLE_PMT_ERR_PRINT("SCTE Subtitle language is over than the max number:%d\n", SCTE_SUBTITLE_MAX);
            }
        }
        break;

        default:
        {
            if (u16DesLen > 0)
            {
                MT_PmtParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
                pu8SectionData += u16DesLen;
                s32Length -= u16DesLen;
            }
        }
        break;
      }
   }

    return MT_SUCCESS;
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
static mt_s32 MT_PmtDataRead(mt_handle hChannel, mt_u32 u32TimeOutms, mt_u8 *pBuf, mt_u32* pAcquiredNum , mt_u32 * pBuffSize)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u8  u8tableid = 0;
    MT_UNF_DMX_DATA_S sSection[32] = { 0 };
    mt_u32 num = 0;
    mt_u32 i = 0;
    mt_u32 k = 0;
    mt_u32 count = 0 ;
    mt_u32 u32Times = 0;
    mt_u32 u32SecTotalNum=0;
    mt_u32 u32SecNum = 0;
    mt_u32 RequestNum = 32;
    mt_u32 crcSoftware = 0;
    mt_u32 crcStream = 0;
    mt_u8 u8SecGotFlag[MAX_SECTION_NUM] = { 0 };

    u32Times = u32TimeOutms / 10;

    memset(u8SecGotFlag, 0, sizeof(u8SecGotFlag[MAX_SECTION_NUM]));
    while (--u32Times)
    {
        s32Ret = MT_UNF_DMX_AcquireBuf(hChannel, RequestNum, &num, sSection, (mt_u32)PGPAT_TIMEOUT);
        if ((MT_SUCCESS != s32Ret) || (num == 0)){
            SAMPLE_PMT_ERR_PRINT("MT_UNF_DMX_AcquireBuf failed\n");
            continue;
        }

        for (i = 0; i < num; i++)
        {
            SAMPLE_PMT_INFO_PRINT("sSection[%d].enDataType = %d\n",i, sSection[i].enDataType);
            MT_PMTSampleDumpData(sSection[i].pu8Data, sSection[i].u32Size);

            if (sSection[i].enDataType == MT_UNF_DMX_DATA_TYPE_WHOLE)
            {
                u8tableid = sSection[i].pu8Data[0];
                u32SecNum = sSection[i].pu8Data[6];
                u32SecTotalNum = sSection[i].pu8Data[7] + 1;

                SAMPLE_PMT_INFO_PRINT("u32SecNum=%d, u32SecTotalNum=%d,u8tableid=%d u32Size=%d\n",u32SecNum,u32SecTotalNum,u8tableid, sSection[i].u32Size);

                k = sSection[i].u32Size;
                crcSoftware = MT_CRC32(&(sSection[i].pu8Data[0]), sSection[i].u32Size - 4);
                crcStream=sSection[i].pu8Data[k-4];
                crcStream=(crcStream<<8)|sSection[i].pu8Data[k-3];
                crcStream=(crcStream<<8)|sSection[i].pu8Data[k-2];
                crcStream=(crcStream<<8)|sSection[i].pu8Data[k-1];
                if(crcSoftware != crcStream){
                    printf("err.CRC....!!!\n");
                }

                SAMPLE_PMT_INFO_PRINT("crc = 0x%x \n", crcStream);

                if(u8SecGotFlag[u32SecNum] == 0)
                {
                    memcpy((void *)(pBuf + u32SecNum * MAX_SECTION_LEN), sSection[i].pu8Data, sSection[i].u32Size);
                    u8SecGotFlag[u32SecNum] = 1;
                    pBuffSize[u32SecNum] = sSection[i].u32Size;
                    count++;
                }
            }
        }

        SAMPLE_PMT_INFO_PRINT("count=%d\n", count);
        MT_UNF_DMX_ReleaseBuf(hChannel, num, sSection);

        /* to check if all sections are received*/
        if(u32SecTotalNum == count)
        {
            break;
        }


    }

    if (u32Times == 0)
    {
        printf("MT_UNF_DMX_AcquireBuf time out\n");
        return MT_FAILURE;
    }

    if(pAcquiredNum != MT_NULL)
    {
        *pAcquiredNum = u32SecTotalNum;
    }

    SAMPLE_PMT_INFO_PRINT("download section u8tableid = 0x%x  success !!!\n",u8tableid);

    return MT_SUCCESS;
}

/*
@brief Set the get pat filter
@param[in] pPatTb, pat handle
@param[in] u32DmxId, dmx id
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_PmtAcquirePATSection( PAT_TB *pPatTb, mt_u32 u32DmxId)
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
        SAMPLE_PMT_ERR_PRINT("create ch err\n");
        goto FREE_BIGMEM;
    }
    s32Ret = MT_UNF_DMX_SetChannelPID(hChan, PAT_TSPID);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("set pid err\n");
        goto FREE_CHANNEL;
    }

    stFilterAttr.u32FilterDepth = 1;
    stFilterAttr.au8Match[0] = PAT_TABLE_ID;
    stFilterAttr.au8Mask[0] = 0;
    stFilterAttr.au8Negate[0] = 0;

    s32Ret = MT_UNF_DMX_CreateFilter(u32DmxId, &stFilterAttr, &hFilter);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("create flt err\n");
        goto FREE_CHANNEL;
    }
    s32Ret = MT_UNF_DMX_SetFilterAttr(hFilter, &stFilterAttr);
    s32Ret |= MT_UNF_DMX_AttachFilter(hFilter, hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("attach flt err\n");
        goto FREE_FILTER;
    }
    s32Ret = MT_UNF_DMX_OpenChannel(hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("open chnl err\n");
        goto DETCH_FILTER;
    }

    memset(u8DataBuf, 0, sizeof(MAX_SECTION_LEN * MAX_SECTION_NUM));
    memset(u32BufSize, 0, sizeof(MAX_SECTION_LEN * MAX_SECTION_NUM));
    s32Ret = MT_PmtDataRead(hChan, 10000, u8DataBuf, &u32AquiredNum , u32BufSize);
    if (MT_SUCCESS != s32Ret)
    {
       SAMPLE_PMT_ERR_PRINT("MT_PmtDataRead  err\n");
    }
     p = u8DataBuf;
     for(i=0; i < u32AquiredNum; i++)
     {
         MT_PmtParsePAT(p, u32BufSize[i], (mt_u8 *)pPatTb);
         p = p + MAX_SECTION_LEN;
     }
    s32Ret = MT_UNF_DMX_CloseChannel(hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("close ch err\n");
    }
    DETCH_FILTER:
    s32Ret = MT_UNF_DMX_DetachFilter(hFilter, hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("Det flt err\n");
    }
    FREE_FILTER:
    s32Ret = MT_UNF_DMX_DestroyFilter(hFilter);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("Del flt err\n");
    }
    FREE_CHANNEL:
    s32Ret = MT_UNF_DMX_DestroyChannel(hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("Del ch err\n");
    }
    FREE_BIGMEM:
    free(u8DataBuf);
    return s32Ret;
}



/*
@brief Set the get pat filter
@param[in] pPatTb, pat handle
@param[in] u32DmxId, dmx id
@param[in] u16PmtPid, pmt pid
@param[in] u16ServiceId, pat ServiceId
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_PmtAcquirePMTSection( PMT_TB *pPmt_tb, mt_u32 u32DmxId, mt_u16 u16PmtPid, mt_u16 u16ServiceId)
{
    mt_u8                    *u8DataBuf = NULL;
    mt_u8                    *p = NULL;
    mt_u32                   i = 0;
    mt_u32                   u32AquiredNum = 0;
    mt_u32                   u32BufSize[256];
    mt_s32                   s32Ret = MT_FAILURE;
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
        SAMPLE_PMT_ERR_PRINT("create ch err\n");
        goto FREE_BIGMEM;
    }
    s32Ret = MT_UNF_DMX_SetChannelPID(hChan, u16PmtPid);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("set pid err\n");
        goto FREE_CHANNEL;
    }
    stFilterAttr.u32FilterDepth = 3;
    stFilterAttr.au8Match[0] = PMT_TABLE_ID;
    stFilterAttr.au8Mask[0] = 0x0;
    stFilterAttr.au8Negate[0] = 0;
    /* set service id*/
    stFilterAttr.au8Match[1] = (mt_u8)((u16ServiceId >> 8) & 0x00ff);
    stFilterAttr.au8Mask[1] = 0;
    stFilterAttr.au8Negate[1] = 0;
    stFilterAttr.au8Match[2] = (mt_u8)(u16ServiceId & 0x00ff);
    stFilterAttr.au8Mask[2] = 0;
    stFilterAttr.au8Negate[2] = 0;

    s32Ret = MT_UNF_DMX_CreateFilter(u32DmxId, &stFilterAttr, &hFilter);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("create flt err\n");
        goto FREE_CHANNEL;
    }
    s32Ret = MT_UNF_DMX_SetFilterAttr(hFilter, &stFilterAttr);
    s32Ret |= MT_UNF_DMX_AttachFilter(hFilter, hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("attach flt err\n");
        goto FREE_FILTER;
    }
    s32Ret = MT_UNF_DMX_OpenChannel(hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("open chnl err\n");
        goto DETCH_FILTER;
    }

    memset(u8DataBuf, 0, sizeof(MAX_SECTION_LEN * MAX_SECTION_NUM));
    memset(u32BufSize, 0, sizeof(u32BufSize));
    s32Ret = MT_PmtDataRead(hChan, 10000, u8DataBuf, &u32AquiredNum , u32BufSize);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("MT_PmtDataRead  err\n");
    }
    p = u8DataBuf;
    for(i=0; i < u32AquiredNum; i++)
    {
        MT_PmtParsePMT(p, u32BufSize[i], (mt_u8 *)pPmt_tb);
        p = p + MAX_SECTION_LEN;
    }
    s32Ret = MT_UNF_DMX_CloseChannel(hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("close ch err\n");
    }
DETCH_FILTER:
    s32Ret = MT_UNF_DMX_DetachFilter(hFilter, hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("Det flt err\n");
    }
FREE_FILTER:
    s32Ret = MT_UNF_DMX_DestroyFilter(hFilter);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("Del flt err\n");
    }
FREE_CHANNEL:
    s32Ret = MT_UNF_DMX_DestroyChannel(hChan);
    if(MT_SUCCESS!=s32Ret)
    {
        SAMPLE_PMT_ERR_PRINT("Del ch err\n");
    }
FREE_BIGMEM:
    free(u8DataBuf);
    return s32Ret;
}

/*
@brief help
@return void
*/
static void MT_PmtPrint_help(char *name)
{
    SAMPLE_PMT_ERR_PRINT("Lack of parameters\n");
    SAMPLE_PMT_INFO_PRINT("such as: %s -f ./1.ts\n", name);
}

static MT_VOID MT_PmtPrintMenu(void)
{

    SAMPLE_PMT_PRINT("     h : help \n");
    SAMPLE_PMT_PRINT("     q : quit \n");
    SAMPLE_PMT_PRINT("PMT>> ");

}

#if 0
static MT_VOID MT_PmtExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stPmtRunInfo.stInjectTSThread, NULL);
    (MT_VOID)MT_PmtDmxDeInit();
    memset(&g_stPmtRunInfo, 0xff, sizeof(g_stPmtRunInfo));
 }
#endif

/*
@brief quit
@return void
*/
static void MT_PmtCmdTask(void)
{
    char *fgetret = NULL;
    MT_CHAR inputCmd[32] = { 0 };

    while (1)
    {
        MT_PmtPrintMenu();
        fgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret = fgetret;


        if ('q' == inputCmd[0])
        {
            SAMPLE_PMT_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_PMT_INFO_PRINT("help info\n");
        }
    }
}



static MT_S32 MT_Pmtparase_args(MT_S32 argc, MT_CHAR *argv[], source_param_t *pInparam)
{
    MT_S32 opt = 0;

    SAMPLE_PMT_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHqf:")) != -1)
    {
         switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_PmtPrint_help(argv[0]);
                return MT_FAILURE;

            case 'f':
                 MTADP_Strncpy((mt_char*)pInparam->FileName, mt_optarg, sizeof(source_param_t));
                break;
            default:
                (void)MT_PmtPrint_help(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_PMT_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
    mt_s32 MT_PmtMain(mt_s32 argc, mt_char *argv[])
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32 ret = 0;
    mt_s32 i = 0;
    mt_s32 j = 0;
    PAT_TB PatTb = { 0 };
    PMT_TB PmtTb = { 0 };

    source_param_t stParam = { 0 };

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_PmtPrint_help(argv[0]);
        return MT_FAILURE;
    }

    ret = MT_Pmtparase_args(argc, argv, &stParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_PMT_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }

    if(MT_TRUE == g_bTaskQuit)
    {
#ifndef MT_SAMPLE_APP

        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PMT_ERR_PRINT("mt_sys_init failed.\n");
            return ret;
        }
#endif

        ret = MT_PmtDmxInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PMT_ERR_PRINT( "failed to StartDmx\n");
            goto ERR0;
        }

        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&g_stPmtRunInfo.stInjectTSThread, NULL, (void * (*)(void *))MT_PmtInjectTsTask, &stParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PMT_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR1;
        }
        sleep(1);
        if(g_bTaskQuit == MT_TRUE)
        {
            goto ERR1;
        }

        ret = MT_PmtAcquirePATSection(&PatTb, DMX_ID_0);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_PMT_ERR_PRINT("MT_AcquireNITSection failed, ret = %d\n", ret);
            goto ERR2;
        }


        SAMPLE_PMT_INFO_PRINT("ALL Program Infomation[%d]:\n",PatTb.u16ProgNum);

        for(i = 0; i < PatTb.u16ProgNum; i++)
        {
            memset(&PmtTb, 0, sizeof(PMT_TB));
            ret = MT_PmtAcquirePMTSection(&PmtTb, DMX_ID_0, PatTb.PatInfo[i].u16PmtPid, PatTb.PatInfo[i].u16ServiceID);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_PMT_ERR_PRINT("MT_AcquireNITSection failed, ret = %d\n", ret);
                goto ERR2;
            }
            SAMPLE_PMT_INFO_PRINT("Program ServiceID = %d PMT PID = 0x%x,\n", PatTb.PatInfo[i].u16ServiceID, PatTb.PatInfo[i].u16PmtPid);
            SAMPLE_PMT_INFO_PRINT("Audio Num=%d\n",PmtTb.u16AudoNum);

            for(j = 0; j < PmtTb.u16AudoNum; j++)
            {
                SAMPLE_PMT_INFO_PRINT("\tAudio Stream PID  = 0x%x\n", PmtTb.Audioinfo[j].u16AudioPid);

                switch(PmtTb.Audioinfo[j].u32AudioEncType)
                {
                    case HA_AUDIO_ID_MP3:
                        SAMPLE_PMT_INFO_PRINT("\tAudio Stream Type MP3\n");
                        break;
                    case HA_AUDIO_ID_AAC:
                        SAMPLE_PMT_INFO_PRINT("\tAudio Stream Type AAC\n");
                        break;
                    case HA_AUDIO_ID_DOLBY_PLUS:
                        SAMPLE_PMT_INFO_PRINT("\tAudio Stream Type AC3\n");
                        break;
                    case HA_AUDIO_ID_DTSHD:
                        SAMPLE_PMT_INFO_PRINT("\tAudio Stream Type DTS\n");
                        break;
                    default:
                        SAMPLE_PMT_INFO_PRINT("\tAudio Stream Type error\n");
                }
            }

            SAMPLE_PMT_INFO_PRINT("Video Num=%d\n",PmtTb.u16VideoNum);

            for(j = 0; j < PmtTb.u16VideoNum; j++)
            {
                SAMPLE_PMT_INFO_PRINT("\tVideo Stream PID  = 0x%x\n",PmtTb.Videoinfo[j].u16VideoPid);
                switch(PmtTb.Videoinfo[j].u32VideoEncType)
                {
                    case MT_UNF_VCODEC_TYPE_H264:
                        SAMPLE_PMT_INFO_PRINT("\tVideo Stream Type H264\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_MPEG2:
                        SAMPLE_PMT_INFO_PRINT("\tVideo Stream Type MP2\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_MPEG4:
                        SAMPLE_PMT_INFO_PRINT("\tVideo Stream Type MP4\n");
                        break;
                    case MT_UNF_VCODEC_TYPE_HEVC:
                        SAMPLE_PMT_INFO_PRINT("\tVideo Stream Type HEVC\n");
                        break;
                    default:
                        SAMPLE_PMT_INFO_PRINT("\tVideo Stream Type error\n");
                }
            }

            if(PmtTb.u16SubtitlingNum > 0)
            {

                SAMPLE_PMT_INFO_PRINT("\tDVB subtitle NUM   = 0x%x\n",PmtTb.u16SubtitlingNum);
            }

            if(PmtTb.u16SCTESubtNum > 0)
            {
                for(j = 0; j < PmtTb.u16SCTESubtNum; j++)
                {
                    SAMPLE_PMT_INFO_PRINT("\tSCTE subtitle PID  = 0x%x\n",PmtTb.stSCTESubtInfo[j].u16SCTESubtPID);
                }
            }

            SAMPLE_PMT_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

        }

        SAMPLE_PMT_INFO_PRINT("get over!!!\n");
    }

    (MT_VOID)MT_PmtCmdTask();
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
ERR2:
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stPmtRunInfo.stInjectTSThread, NULL);
ERR1:
    (MT_VOID)MT_PmtDmxDeInit();

ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif

    return MT_SUCCESS;
}
