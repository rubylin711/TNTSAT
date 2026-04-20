#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "mt_type.h"
#include "mt_unf_avplay.h"
#include "mt_unf_common.h"


#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DOLBYPLUS.decode.h"
#include "HA.AUDIO.DTSHD.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"

#include "mt_adp_debug.h"
#include "mt_adp_data.h"
#include "mt_adp_demux.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"


#ifdef MTADP_SEARCH_DEBUG

#define MTADP_SEARCH_PRINT   MTADP_PRINT

#else

#define MTADP_SEARCH_PRINT

#endif


#define MTADP_SEARCH_FUNCTION_ENTER()  MTADP_SEARCH_PRINT(" [MTADP_SEARCH][%s]: Enter ==>> \n", __FUNCTION__)
#define MTADP_SEARCH_FUNCTION_EXIT()   MTADP_SEARCH_PRINT(" [MTADP_SEARCH][%s]: Exit ==<< \n", __FUNCTION__)

#define MTADP_SEARCH_FATAL_PRINT(fmt...)       MTADP_SEARCH_PRINT(" [MTADP_SEARCH][FATAL] " fmt)
#define MTADP_SEARCH_ERR_PRINT(fmt...)         MTADP_SEARCH_PRINT(" [MTADP_SEARCH][ERROR] " fmt)
#define MTADP_SEARCH_WARN_PRINT(fmt...)        MTADP_SEARCH_PRINT(" [MTADP_SEARCH][WARN] "  fmt)
#define MTADP_SEARCH_INFO_PRINT(fmt...)        MTADP_SEARCH_PRINT(" [MTADP_SEARCH][INFO] "  fmt)
#define MTADP_SEARCH_MT_DBG_PRINT(fmt...)      MTADP_SEARCH_PRINT(" [MTADP_SEARCH][DEBUG] " fmt)



#define MT_SEARCH_PRINT  printf

#define PGPAT_TIMEOUT (10000)
#define PGPMT_TIMEOUT (10000)
#define PGSDT_TIMEOUT (10000)

#define STREAM_TYPE_SCTE_82H 1

#define DMX_DEFAULT_BUF_NUM 32

static MT_BOOL g_bSrchInit = MT_FALSE;

PMT_COMPACT_TBL g_ProgTable;

static mt_void ParseSDTDesc( const mt_u8* pu8DesData, mt_s32 s32DescLen, SDT_INFO *pstruProg)
{
    mt_u32 u32NameLength = 0;
    mt_u8 u8Tag = 0;
    mt_u8 u8Length = 0;
    const mt_u8* pu8Data = MT_NULL;

    if ((MT_NULL == pu8DesData) || (MT_NULL == pstruProg))
    {
        MTADP_SEARCH_ERR_PRINT("pu8DesData or pstruProg is NULL!\n");
        return;
    }

    while (s32DescLen > 0)
    {
        u8Tag = *pu8DesData++;
        u8Length = *pu8DesData++;

        if (u8Length == 0)
        {
            return;
        }

        pu8Data = pu8DesData;
        pu8DesData += u8Length;

        s32DescLen -= (u8Length + 2);

        switch (u8Tag)
        {
            case SERVICE_DESCRIPTOR:
        {
            pstruProg->u32ServiceType = *pu8Data++;

            u32NameLength = *pu8Data++;
            pu8Data += u32NameLength;

            u32NameLength = *pu8Data++;

            if (0 == u32NameLength)
            {
                break;
            }


            /*Parse service name*/
            if (u32NameLength > MAX_PROGNAME_LENGTH - 1)
            {
                u32NameLength = (mt_u32)(MAX_PROGNAME_LENGTH - 1);
            }

            strncpy((mt_char*)pstruProg->s8ProgName, (mt_char*)pu8Data, u32NameLength);

            break;
        }
        case NVOD_REFERENCE_DESCRIPTOR:
        {
            while (u8Length > 0)
            {

                pstruProg->u32ServiceType = 0x04;

                u8Length = u8Length - 6;

                if (u8Length <= 0)
                {
                    break;
                }

                pu8Data += 6;
            }

            break;
        }
        case COUNTRY_AVAILABILITY_DESCRIPTOR:
        case LINKAGE_DESCRIPTOR:
            //printf("\n +++ SDT LINKAGE linktype =0x%02x +++\n", linktype);

        case MOSAIC_DESCRIPTOR:

            //printf("\n +++ SDT MOSAIC_DESCRIPTOR +++\n");
             break;
        case CA_IDENTIFIER_DESCRIPTOR:
        case TELEPHONE_DESCRIPTOR:
        case MULTILINGUAL_SERVICE_NAME_DESCRIPTOR:
        case PRIVATE_DATA_SPECIFIER_DESCRIPTOR:
        case DATA_BROADCAST_DESCRIPTOR:
        case STUFFING_DESCRIPTOR:
        case BOUQUET_NAME_DESCRIPTOR:
            break;

        case EXTENSION_DESCRIPTOR:
        {
            mt_s32 offset = 0;
            mt_s32 len = 0;
            mt_u8 descriptor_tag_extension = pu8Data[offset++];

            if (u8Length < 4 || descriptor_tag_extension != 8)
            {
                break;
            }

            pstruProg->message_descriptor_flag = MT_TRUE;
            pstruProg->message_descriptor[pstruProg->num_message_descriptor].message_id = pu8Data[offset++];

            memset(pstruProg->message_descriptor[pstruProg->num_message_descriptor].iso639LanguageCode, 0, 4);
            memcpy(pstruProg->message_descriptor[pstruProg->num_message_descriptor].iso639LanguageCode, &pu8Data[offset], 3);
            pstruProg->message_descriptor[pstruProg->num_message_descriptor].iso639LanguageCode[3] = '\0';
            offset += 3;

            len = u8Length - offset;
            if (len > 32)
            {
                len  = 32;
            }

            memset(pstruProg->message_descriptor[pstruProg->num_message_descriptor].message, 0, 32);
            memcpy(pstruProg->message_descriptor[pstruProg->num_message_descriptor].message, &pu8Data[offset], len);
            pstruProg->message_descriptor[pstruProg->num_message_descriptor].message[len] = '\0';

            if (pstruProg->num_message_descriptor < MESSAGE_DESCRIPTOR_MAX)
            {
                pstruProg->num_message_descriptor++;
            }

            break;
        }
        default:
            break;
        }
    }
}

static mt_void ParsePMTDesc( const mt_u8* pu8DesData, mt_s32 s32DescLength, PMT_TB* pstruProg)
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
                pstruProg->Audioinfo[pstruProg->u16AudoNum].u32audiotype = pu8DesData[3];
                //MTADP_SEARCH_INFO_PRINT("+++2audio = %d \n", pstruProg->Audioinfo[pstruProg->u16AudoNum].u32audiotype);
                break;
            }

            case CA_DESCRIPTOR:
            {
                if (PROG_MAX_CA > pstruProg->u16CANum){
                    pstruProg->CASystem[pstruProg->u16CANum].u16CASystemID = pu8DesData[0] << 8 | pu8DesData[1];
                    pstruProg->CASystem[pstruProg->u16CANum].u16CAPID = (pu8DesData[2] << 8 | pu8DesData[3]) & 0x1FFF;
                    pstruProg->u16CANum++;
                }
                else
                {
                    MTADP_SEARCH_ERR_PRINT("CA number is over than the max number:%d\n", PROG_MAX_CA);
                }
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
                   // printf("Subtitle tag s32DescLength:%u.............pstruProg->u16SubtitlingNum = %d, u8CntIndex = %d\n",s32DescLength, pstruProg->u16SubtitlingNum, u8CntIndex);
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
                #if 1
                    MTADP_SEARCH_INFO_PRINT("[%d]lang code:%#x\n", pstruProg->u16SubtitlingNum, pstruProg->SubtitingInfo[pstruProg->u16SubtitlingNum].DesInfo[u8CntIndex].u32LangCode);
                    MTADP_SEARCH_INFO_PRINT("[%d]page id :%u\n", pstruProg->u16SubtitlingNum, pstruProg->SubtitingInfo[pstruProg->u16SubtitlingNum].DesInfo[u8CntIndex].u16PageID);
                    MTADP_SEARCH_INFO_PRINT("[%d]ancilary page id : %u\n", pstruProg->u16SubtitlingNum, pstruProg->SubtitingInfo[pstruProg->u16SubtitlingNum].DesInfo[u8CntIndex].u16AncillaryPageID);
                    MTADP_SEARCH_INFO_PRINT("[%d]count is %u\n\n", pstruProg->u16SubtitlingNum, pstruProg->SubtitingInfo[pstruProg->u16SubtitlingNum].u8DesInfoCnt);
                    MTADP_SEARCH_INFO_PRINT("[%d]pid is 0x%x\n\n", pstruProg->u16SubtitlingNum,pstruProg->SubtitingInfo[pstruProg->u16SubtitlingNum].u16SubtitlingPID);
                #endif
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
                if(pstruProg->u16ClosedCaptionNum > CAPTION_SERVICE_MAX)
                {
                    pstruProg->u16ClosedCaptionNum = CAPTION_SERVICE_MAX;
                }
                j ++;
                while(j < u8Length)
                {
                    /* Ref : 6.9.2 Caption Service Descriptor in ATSC a/65 */
                    for(u8Index = 0; u8Index < pstruProg->u16ClosedCaptionNum; u8Index++)
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

            case EXTENSION_DESCRIPTOR:
            {
                mt_u8 descriptor_tag_extension = 0;

                descriptor_tag_extension = pu8DesData[0];
                if (0x6 == descriptor_tag_extension)
                {
                    pstruProg->Audioinfo[pstruProg->u16AudoNum].u32audiotype = (pu8DesData[1] >> 1) & 0x1f;
                }
            }
                break;

            default:
                break;
        }

        pu8DesData    += u8Length;
        s32DescLength -= (u8Length + 2);
    }
}

mt_s32 SRH_ParsePAT( const mt_u8  *pu8SectionData, mt_s32 s32Length, mt_u8 *pSectionStruct)
{
    mt_u16 u16ProgrameNumber = 0;
    mt_u16 u16Pid  = 0x1FFF;
    mt_u16 u16TsId = 0;
    PAT_TB *pPatTb = (PAT_TB *)pSectionStruct;
    PAT_INFO *pPatInfo = { 0 };

    if ((MT_NULL == pu8SectionData) || (MT_NULL == pSectionStruct))
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

    while(s32Length >= 4)
    {
        pPatInfo = &(pPatTb->PatInfo[pPatTb->u16ProgNum]);

        u16ProgrameNumber = (mt_u16)((pu8SectionData[0] << 8) | pu8SectionData[1]);

        u16Pid = (mt_u16)(((pu8SectionData[2] & 0x1F) << 8) | pu8SectionData[3]);

        if(u16ProgrameNumber != 0x0000)
        {
            pPatInfo->u16ServiceID = u16ProgrameNumber;
            pPatInfo->u16PmtPid = u16Pid;
            pPatTb->u16ProgNum++;
#ifdef SRCH_DEBUG
            MTADP_SEARCH_INFO_PRINT(" parser PAT get PmtPid %d(0x%x) ServiceID %d(0x%x)  index %d\n",
                   pPatInfo->u16PmtPid, pPatInfo->u16PmtPid, pPatInfo->u16ServiceID, pPatInfo->u16ServiceID, index);
#endif
        }

        pu8SectionData += 4;
        s32Length -= 4;
    }

    return MT_SUCCESS;
}

static MT_BOOL IsSubtitleStream(const mt_u8 *pu8Data, mt_s32 s32Length)
{
    MT_BOOL bRet = MT_FALSE;

    int   nLen = s32Length;
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

static MT_BOOL IsAudioStream(const mt_u8 *pu8Data, mt_s32 s32Length, PMT_TB *pmtTb)
{

    MT_BOOL bRet = MT_FALSE;

    int   nLen = s32Length;
    mt_u8 *pData = (mt_u8*)pu8Data;
    mt_u8 u8Tag = 0;
    mt_u8 u8Length = 0;



    while (nLen > 0)
    {
        u8Tag = *pData++;

        u8Length = *pData++;

        if (u8Tag == 0x0A)
        {
            memcpy(pmtTb->Audioinfo[pmtTb->u16AudoNum].language_code, pData, 3);
            pmtTb->Audioinfo[pmtTb->u16AudoNum].u32audiotype = pData[3];
            MTADP_SEARCH_INFO_PRINT("+++1audio = %d language=[%s]\n", pmtTb->Audioinfo[pmtTb->u16AudoNum].u32audiotype, pmtTb->Audioinfo[pmtTb->u16AudoNum].language_code);
            bRet = MT_TRUE;
            break;
        }

        pData += u8Length;

        nLen -= (u8Length+2);
    }

    return bRet;
}


static MT_BOOL IsARIBCCStream(const mt_u8 *pu8Data, mt_s32 s32Length)
{
    MT_BOOL bRet = MT_FALSE;

    int   nLen = s32Length;
    mt_u8 *pData = (mt_u8*)pu8Data;
    mt_u8 u8Tag = 0;
    mt_u8 u8Length = 0;
    mt_u8 u8ComponentTag = 0;

    while (nLen > 0)
    {
        u8Tag = *pData++;
        u8Length = *pData++;
        /*Ref ARIB-TR-B14v2_8-vol3-p3-2-E2.pdf  P3-107*/
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

static MT_BOOL IsTtxStream(const mt_u8 *pu8Data, mt_s32 s32Length)
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

static MT_BOOL IsAC3Stream(const mt_u8 *buf, mt_u32 len)
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

static MT_BOOL IsAC4Stream(const mt_u8 *buf, mt_u32 len)
{
    while (len >= 2)
    {
        mt_u32 tag      = buf[0];
        mt_u32 tag_len  = buf[1];

        buf += 2;
        len -= 2;

        switch (tag)
        {
            case AC4_REGISTRATION_DESCRIPTOR:
                if (tag_len == 0x4)
                {
                    if (buf[0] == 0x41 && buf[1] == 0x43 && buf[2] == 0x2d && buf[3] == 0x34)
                    {
                        return MT_TRUE;
                    }
                }
            case EXTENSION_DESCRIPTOR:
                if (buf[0] == 0x15)
                {
                    return MT_TRUE;
                }

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


// 解析音频标志位
static audio_flags_t parse_audio_flags(mt_u8 flags_byte)
{
    audio_flags_t flags;

    flags.audio_description = (flags_byte >> 7) & 0x01;
    flags.spoken_subtitles = (flags_byte >> 6) & 0x01;
    flags.dialogue_enhancement = (flags_byte >> 5) & 0x01;
    flags.interactivity_enabled = (flags_byte >> 4) & 0x01;
    flags.language_code_present = (flags_byte >> 3) & 0x01;
    flags.text_label_present = (flags_byte >> 2) & 0x01;
    flags.multi_stream_info_present = (flags_byte >> 1) & 0x01;
    flags.future_extension = flags_byte & 0x01;

    return flags;
}

// 解析辅助组件信息
static mt_s32 parse_aux_component_info(const mt_u8* data, mt_s32 offset, mt_s32 data_len, aux_component_info_t* aux_info)
{
    if (offset >= data_len)
    {
        return -1;
    }

    mt_u8 aux_header = data[offset++];
    aux_info->num_aux_components = (aux_header >> 5) & 0x07;
    aux_info->reserved_zero_future_use = aux_header & 0x1F;
    for (mt_s32 i = 0; i < aux_info->num_aux_components; i++)
    {
        if (i < AC4_AUX_COMPONENT_MAX && offset < data_len)
        {
            aux_info->component_tags[i] = data[offset++];
        }
        else
        {
            break;
        }
    }

    return offset;
}

// 解析单个音频预选条目
static mt_s32 parse_audio_preselection(const mt_u8* data, mt_s32 offset, mt_s32 data_len, audio_preselection_t* preselection)
{
    if (offset + 2 > data_len)
    {
        return -1;
    }

    // 解析preselection ID和音频渲染指示
    mt_u8 byte1 = data[offset++];
    preselection->reserved = (byte1 >> 7) & 0x01;
    preselection->preselection_id = (byte1 >> 3) & 0x0F;
    preselection->audio_rendering_indication = byte1 & 0x07;

    // 标志位
    mt_u8 byte2 = data[offset++];
    preselection->flags = parse_audio_flags(byte2);

    // 解析语言码
    if (preselection->flags.language_code_present)
    {
        if (offset + 3 > data_len)
        {
            return -1;
        }

        memcpy(preselection->language_code, &data[offset], 3);
        preselection->language_code[3] = '\0';
        offset += 3;
    }
    else
    {
        preselection->language_code[0] = '\0';
    }

    // 文本标签和消息ID的处理
    preselection->has_message_id = 0;
    if (preselection->flags.text_label_present)
    {
        // 在这个数据流中，text_label_present=1但实际上没有4字节文本标签
        // 只有message_id直接跟在语言码后面
        if (offset < data_len)
        {
            preselection->message_id = data[offset++];
            preselection->has_message_id = 1;
        }
    }

    // 解析多流信息
    preselection->has_aux_info = 0;
    if (preselection->flags.multi_stream_info_present)
    {
        mt_s32 new_offset = parse_aux_component_info(data, offset, data_len, &preselection->aux_info);
        if (new_offset > 0)
        {
            offset = new_offset;
            preselection->has_aux_info = 1;
        }
    }

    // 解析未来扩展
    if (preselection->flags.future_extension)
    {
        if (offset >= data_len)
        {
            return offset;
        }

        mt_u8 future_extension_length = data[offset++];
        if (offset + future_extension_length <= data_len)
        {
            offset += future_extension_length;
        }
    }

    return offset;
}

// 主解析函数
static mt_s32 parse_audio_preselection_descriptor(const mt_u8* data, mt_s32 data_len,
                                       audio_preselection_descriptor_t* descriptor)
{
    mt_s32 offset = 0;


    if (data_len < 4)
    {
        return -1;
    }

    descriptor->descriptor_tag = data[offset++];
    descriptor->descriptor_length = data[offset++];
    descriptor->descriptor_tag_extension = data[offset++];
    if (descriptor->descriptor_tag != EXTENSION_DESCRIPTOR || descriptor->descriptor_tag_extension != AUDIO_PRESELECTION_DESCRIPTOR)
    {
        return -2;
    }

    if (offset >= data_len)
    {
        return -1;
    }

    mt_u8 global_header = data[offset++];
    descriptor->num_preselections = (global_header >> 3) & 0x1F;
    descriptor->has_aux_components = (global_header >> 2) & 0x01;
    descriptor->reserved_global = global_header & 0x03;
    descriptor->num_parsed_preselections = 0;
    for (int i = 0; i < descriptor->num_preselections && offset < data_len; i++)
    {
        if (descriptor->num_parsed_preselections < AC4_PRESELECTION_MAX)
        {
            int new_offset = parse_audio_preselection(data, offset, data_len,
                                                    &descriptor->preselections[descriptor->num_parsed_preselections]);
            if (new_offset < 0)
            {
                break;
            }

            offset = new_offset;
            descriptor->num_parsed_preselections++;
        }
        else
        {
            break;
        }
    }

    return offset;
}


mt_s32 SRH_ParseAudioPreselectionInfo(PMT_AUDIO *Audioinfo, const mt_u8 *buf, mt_u32 len)
{
    audio_preselection_descriptor_t descriptor;
    mt_s32 parsed_bytes = 0;

    while (len >= 2)
    {
        mt_u32 descriptor_tag = buf[0];
        mt_u32 descriptor_length  = buf[1];

        parsed_bytes = parse_audio_preselection_descriptor(buf, len, &descriptor);
        if (parsed_bytes > 0) {
            Audioinfo->audio_preselection_descriptor = descriptor;
            Audioinfo->has_audio_preselection = 1;
            break;
        }

        buf += (descriptor_length + 2);
        if (descriptor_length >= len)
        {
            len = 0;
        }
        else
        {
            len -= (descriptor_length + 2);
        }
    }

}


mt_s32 SRH_ParseDDPAudiodescriptionInfo(PMT_AUDIO *Audioinfo, const mt_u8 *buf, mt_u32 len)
{
    mt_s32 offset;

    while (len >= 2)
    {
        offset = 0;
        mt_u32 descriptor_tag = buf[offset++];
        mt_u32 descriptor_length  = buf[offset++];

        if (AC3_PLUS_DESCRIPTOR == descriptor_tag  && descriptor_length >= 2)
        {
            offset++;
            mt_u32 component_type = buf[offset++]; //componet type info[visually impared]
            if (DDP_VISUALLY_IMPAIRED_DESCRIPTOR == component_type)
            {
                Audioinfo->u32audiotype = component_type;
            }
            else
            {
                Audioinfo->u32audiotype = 0;
            }
        }

        buf += (descriptor_length + 2);
        if (descriptor_length >= len)
        {
            len = 0;
        }
        else
        {
            len -= (descriptor_length + 2);
        }
    }

}

mt_s32 SRH_ParsePMT ( const mt_u8 *pu8SectionData, mt_s32 s32Length, mt_u8 *pSectionStruct)
{
    mt_u8 u8StreamType = 0;

    mt_u16 u16DesLen = 0;
    mt_u16 u16Pid  = 0x1FFF;
    PMT_TB *pPmtTb = (PMT_TB *)pSectionStruct;

    if ((MT_NULL == pu8SectionData) || (MT_NULL == pSectionStruct))
    {
        return MT_FAILURE;
    }

    if (PMT_TABLE_ID != pu8SectionData[0])
    {
        return MT_FAILURE;
    }

    memcpy(pPmtTb->u8PmtData, pu8SectionData, s32Length);
    pPmtTb->u32PmtLen = s32Length;
   // printf("enter length : %d\n", s32Length);

    s32Length = (mt_s32)((pu8SectionData[1] << 8) | pu8SectionData[2]) & 0x0fff;

   // printf("s32Length = %d, %#x:%#x\n", s32Length, pu8SectionData[1], pu8SectionData[2]);

    pPmtTb->u16ServiceID = (mt_u16)((pu8SectionData[3] << 8) | pu8SectionData[4]);

    pu8SectionData += 8; /* skip 8-byte to PCR_PID*/

    u16Pid = (mt_u16)(((pu8SectionData[0] & 0x1F) << 8) | pu8SectionData[1]);
    u16DesLen = (mt_u16)(((pu8SectionData[2] & 0x0F) << 8) | pu8SectionData[3]);

    pPmtTb->u16PcrPid = u16Pid;

    pu8SectionData += 4;

    s32Length -= 9;

   // printf("pcr pid = %#x, deslength:%d, s32Length = %d\n", u16Pid, u16DesLen, s32Length);

    if (u16DesLen > 0)
    {
        ParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
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

        //printf("stream type : %#x, pid:%#x, length:%d, u16DesLen = %d\n", u8StreamType, u16Pid, s32Length, u16DesLen);

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
        case STREAM_TYPE_AVS2_VIDEO:
        {
           // printf("video stream type is %#x, pid=%#x\n", u8StreamType, u16Pid);
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
                else if (u8StreamType == STREAM_TYPE_AVS2_VIDEO)
                {
                    pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u32VideoEncType = MT_UNF_VCODEC_TYPE_AVS2;
                }
                else
                {
                    pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u32VideoEncType = MT_UNF_VCODEC_TYPE_MPEG2;
                }

                if (u16DesLen > 0)
                {
                    ParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
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
        case STREAM_TYPE_PCM_AUDIO:
        case STREAM_TYPE_AC3_AUDIO:
        case STREAM_TYPE_EAC3_AUDIO:
    #ifndef STREAM_TYPE_SCTE_82H
        case STREAM_TYPE_DTS_AUDIO:
    #endif
        case STREAM_TYPE_DTS_ES_XLL_AUDIO:
        case STREAM_TYPE_DOLBY_TRUEHD_AUDIO:
        case STREAM_TYPE_VIVID_AUDIO:
        {

            if(pPmtTb->u16AudoNum < PROG_MAX_AUDIO)
            {
                pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u16AudioPid = u16Pid;
#ifdef SRCH_DEBUG
                MTADP_SEARCH_INFO_PRINT(" audio type = 0x%x \n", u8StreamType);
#endif
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
                else if (u8StreamType == STREAM_TYPE_EAC3_AUDIO)
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DOLBY_PLUS;
                }
                else if (u8StreamType == STREAM_TYPE_DTS_ES_XLL_AUDIO)
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DTSHD;
                }
                else if (u8StreamType == STREAM_TYPE_VIVID_AUDIO)
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_VVID;
                }
                else if (u8StreamType == STREAM_TYPE_PCM_AUDIO)
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_PCM;
                }
                else
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_MP3;
                }

                if (u16DesLen > 0)
                {
                    ParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
                    pu8SectionData += u16DesLen;
                    s32Length -= u16DesLen;
                }
                else
                {
                    pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32audiotype = 0;
                    //MTADP_SEARCH_INFO_PRINT("+++3audio = %d \n", pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32audiotype);

                }

                pPmtTb->u16AudoNum++;
            }

            break;
        }
#if 1
        case STREAM_TYPE_PRIVATE:
        {
           // printf("private stream...........pid=%#x\n", u16Pid);
            if (u16DesLen > 0 )/* subtitling stream info */
            {
                if  (IsSubtitleStream(pu8SectionData, u16DesLen))
                {
                    if (SUBTITLING_MAX > pPmtTb->u16SubtitlingNum)
                    {
                        pPmtTb->SubtitingInfo[pPmtTb->u16SubtitlingNum].u16SubtitlingPID = u16Pid;

                        //printf("using pPmtTb->u16SubtitlingNum = %d to parse description\n", pPmtTb->u16SubtitlingNum);

                        ParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);

                        pPmtTb->u16SubtitlingNum++;
                    }
                    else
                    {
                        MTADP_SEARCH_ERR_PRINT("Subtitle language is over than the max number:%d\n", SUBTITLING_MAX);
                    }
                }
                else if (IsARIBCCStream(pu8SectionData, u16DesLen))
                {
                    pPmtTb->u16ARIBCCPid = u16Pid;
                }
                else if (IsTtxStream(pu8SectionData, u16DesLen))
                {
                    if (TTX_MAX > pPmtTb->u16TtxNum)
                    {
                        pPmtTb->stTtxInfo[pPmtTb->u16TtxNum].u16TtxPID = u16Pid;

                        ParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);

                        pPmtTb->u16TtxNum++;
                    }
                }

                else if(IsAudioStream(pu8SectionData, u16DesLen, pPmtTb))
                {
                    if (pPmtTb->u16AudoNum < PROG_MAX_AUDIO)
                    {
                        pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u16AudioPid = u16Pid;
                        if (IsAC4Stream(pu8SectionData, u16DesLen))
                        {
                            pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DOLBY_AC4;
                        }
                        // dolby parse
                        else if (IsAC3Stream(pu8SectionData, u16DesLen))
                        {
                            pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DOLBY_PLUS;

                            //SRH_ParseDDPAudiodescriptionInfo(&pPmtTb->Audioinfo[pPmtTb->u16AudoNum], pu8SectionData, u16DesLen);
                            ParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
                        }
                        else
                        {
                            pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_MP3;
                        }

                        if(pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32audiotype != 0 && pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32audiotype != 3 &&
                           pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32audiotype != DDP_VISUALLY_IMPAIRED_DESCRIPTOR)
                        {
                            pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32audiotype = 0;
                        }

                        pPmtTb->u16AudoNum++;
                    }
                }
                else
                {
                    if (IsAC4Stream(pu8SectionData, u16DesLen))
                    {
                        pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u16AudioPid = u16Pid;
                        pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DOLBY_AC4;
                        pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32audiotype = 0;

                        SRH_ParseAudioPreselectionInfo(&pPmtTb->Audioinfo[pPmtTb->u16AudoNum], pu8SectionData, u16DesLen);
                        pPmtTb->u16AudoNum++;
                    }
                    else if (IsAC3Stream(pu8SectionData, u16DesLen))
                    {
                        pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u16AudioPid = u16Pid;
                        pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DOLBY_PLUS;

                        //SRH_ParseDDPAudiodescriptionInfo(&pPmtTb->Audioinfo[pPmtTb->u16AudoNum], pu8SectionData, u16DesLen);
                        ParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);

                        pPmtTb->u16AudoNum++;
                    }
                    else
                    {
                        if(pPmtTb->u16VideoNum == 1)
                        {
                            pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u16AudioPid = u16Pid;
                            pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32AudioEncType = HA_AUDIO_ID_DTSPASSTHROUGH;
                            pPmtTb->Audioinfo[pPmtTb->u16AudoNum].u32audiotype = 0;
                             pPmtTb->u16AudoNum++;
                        }
                        else
                        {
                            pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u16VideoPid = u16Pid;

                            pPmtTb->Videoinfo[pPmtTb->u16VideoNum].u32VideoEncType = MT_UNF_VCODEC_TYPE_HEVC;

                            pPmtTb->u16VideoNum++;
                        }

                    }


                }

                pu8SectionData += u16DesLen;
                s32Length -= u16DesLen;
            }

        }
        break;
#endif

    #ifdef STREAM_TYPE_SCTE_82H
        case STREAM_TYPE_SCTE:
        {
            if (u16DesLen > 0)
            {
                ParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
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
                MTADP_SEARCH_ERR_PRINT("SCTE Subtitle language is over than the max number:%d\n", SCTE_SUBTITLE_MAX);
            }

            break;
        }
    #endif

        default:
        {
            if (u16DesLen > 0)
            {
                ParsePMTDesc(pu8SectionData, u16DesLen, pPmtTb);
                pu8SectionData += u16DesLen;
                s32Length -= u16DesLen;
            }
        }
        break;
        }
    }

    return MT_SUCCESS;
}

mt_s32 SRH_ParseNIT(const mt_u8 *pu8SectionData, mt_s32 s32Length, mt_u8 *pSectionStruct)
{
    mt_u16 u16OrgNetworkId = 0x1FFF;
    mt_u16 u16TsId = 0x1FFF;
    u16 network_desc_len = 0;
    u16 tsport_desc_len = 0;
    u16 i = 0;

    MTADP_SEARCH_FUNCTION_ENTER();

    NIT_TB *pNitTb = (NIT_TB *)pSectionStruct;
    NIT_INFO *pNitInfo;

    if ((MT_NULL == pu8SectionData) || (MT_NULL == pSectionStruct))
    {
        return MT_FAILURE;
    }

    if (NIT_TABLE_ID_ACTUAL != pu8SectionData[0])
    {
        return MT_FAILURE;
    }

    s32Length = (mt_s32)((pu8SectionData[1] << 8) | pu8SectionData[2]) & 0x0fff;

    u16TsId = (mt_u16)((pu8SectionData[3] << 8) | pu8SectionData[4]);

    pNitTb->u16TsId = u16TsId;

    network_desc_len = (mt_u16)((pu8SectionData[8] << 8) | pu8SectionData[9]) & 0x0fff;
    pu8SectionData += 10;
    s32Length -= 10;

    //skip parser network descriptor
    pu8SectionData += network_desc_len;
    s32Length -= network_desc_len;

    pu8SectionData += 2;
    s32Length -= 2;
    i = 0;
    while (s32Length > 4)
    {
        pNitInfo = &(pNitTb->NitInfo[i]);
    //Only paser first
        u16TsId = (mt_u16)((pu8SectionData[0] << 8) | pu8SectionData[1]);
        u16OrgNetworkId = (mt_u16)((pu8SectionData[2] << 8) | pu8SectionData[3]);

        MTADP_SEARCH_INFO_PRINT("SRH_ParseNIT g_ts_id:0x%x,g_orignal_network_id:0x%x\n",u16TsId, u16OrgNetworkId);

        pNitInfo->u16TsId = u16TsId;
        pNitInfo->u16OrgNetID = u16OrgNetworkId;

        tsport_desc_len = (mt_u16)((pu8SectionData[4]  << 8) | pu8SectionData[5]) & 0x0fff;
        pu8SectionData += 6;
        s32Length -= 6;

        //skip parser transport descriptor

        pu8SectionData += tsport_desc_len;
        s32Length -= tsport_desc_len;

        i++;
    }

    MTADP_SEARCH_FUNCTION_EXIT();


    return MT_SUCCESS;
}

mt_s32 SRH_ParseCAT(const mt_u8 *pu8SectionData, mt_s32 s32Length, mt_u8 *pSectionStruct)
{

    CAT_TB *pCatTb = (CAT_TB *)pSectionStruct;
    CAT_INFO *pCatInfo;

    if ((MT_NULL == pu8SectionData) || (MT_NULL == pSectionStruct))
    {
        return MT_FAILURE;
    }

    if (CAT_TABLE_ID != pu8SectionData[0])
    {
        return MT_FAILURE;
    }

    MTADP_SEARCH_INFO_PRINT("[CAT] tid : 0x%x \n", pu8SectionData[0]);

    return MT_SUCCESS;
}

mt_s32 SRH_ParseSDT(const mt_u8 *pu8SectionData, mt_s32 s32Length, mt_u8 *pSectionStruct)
{
    mt_u8 u8EitFlag = 0;
    mt_u8 u8EitFlag_PF = 0;
    mt_u8 u8RunStatus = 0;
    mt_u8 u8FreeCA = 0;
    mt_u16 u16NetworkId = 0x1FFF;
    mt_u16 u16ProgramNumber = 0;
    mt_u16 u16TsId   = 0;
    mt_s32 s32DesLen = 0;

    SDT_TB *pSdtTb = (SDT_TB *)pSectionStruct;
    SDT_INFO *pSdtInfo;

    if ((MT_NULL == pu8SectionData) || (MT_NULL == pSectionStruct))
    {
        return MT_FAILURE;
    }

    if (SDT_TABLE_ID_ACTUAL != pu8SectionData[0])
    {
        return MT_FAILURE;
    }

    s32Length = (mt_s32)((pu8SectionData[1] << 8) | pu8SectionData[2]) & 0x0fff;

    u16TsId = (mt_u16)((pu8SectionData[3] << 8) | pu8SectionData[4]);
    u16NetworkId = (mt_u16)((pu8SectionData[8] << 8) | pu8SectionData[9]);

    pSdtTb->u16TsId  = u16TsId;
    pSdtTb->u16NetID = u16NetworkId;

    pu8SectionData += 11;
    s32Length -= 11;

    while (s32Length > 4)
    {
        pSdtInfo = &(pSdtTb->SdtInfo[pSdtTb->u32ProgNum]);

        u16ProgramNumber = (mt_u16)((pu8SectionData[0] << 8) | pu8SectionData[1]);

        u8EitFlag = (mt_u8)((pu8SectionData[2] & 0x02) >> 1);
        u8EitFlag_PF = (mt_u8)(pu8SectionData[2] & 0x01);

        u8RunStatus = (mt_u8)((pu8SectionData[3] & 0xE0) >> 5);

        u8FreeCA = (mt_u8)((pu8SectionData[3] & 0x10) >> 4);

        pSdtInfo->u16ServiceID = u16ProgramNumber;
        pSdtInfo->u8EitFlag = u8EitFlag;
        pSdtInfo->u8EitFlag_PF = u8EitFlag_PF;
        pSdtInfo->RunState = u8RunStatus;
        pSdtInfo->CAMode = u8FreeCA;

        MTADP_SEARCH_INFO_PRINT("[SDT] ProgramNumber : %d CAMode: 0x%x \n", u16ProgramNumber, u8FreeCA);

        s32DesLen = (mt_s32)(((pu8SectionData[3] & 0x0F) << 8) | pu8SectionData[4]);

        pu8SectionData += 5;
        s32Length -= 5;

        ParseSDTDesc( pu8SectionData, s32DesLen, pSdtInfo);

        pu8SectionData += s32DesLen;
        s32Length -= s32DesLen;
        pSdtTb->u32ProgNum++;
    }

    return MT_SUCCESS;
}

/*
@biref PATRequest
@param[in] u32DmxID, ID of decoder
@param[in] pat_tb, Handle to the pat table
@return SUCCESS
*/
mt_s32 SRH_PATRequest(mt_u32 u32DmxID, PAT_TB *pat_tb)
{
    mt_s32 s32Ret = 0;

    DMX_DATA_FILTER_S stDataFilter = { 0 };

    MTADP_SEARCH_FUNCTION_ENTER();

    if(pat_tb == NULL)
    {
        MTADP_SEARCH_ERR_PRINT("pat_tb is NULL!\n");
        return MT_FAILURE;
    }

    memset(stDataFilter.u8Match, 0, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Mask, 0xff, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Negate, 0, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));

    stDataFilter.u32TSPID   = PAT_TSPID;
    stDataFilter.u32TimeOut = PGPAT_TIMEOUT;
    stDataFilter.u16FilterDepth = 1;
    stDataFilter.u8Crcflag = 0;

    stDataFilter.u8Match[0] = PAT_TABLE_ID;
    stDataFilter.u8Mask[0] = 0;

    /* set call back func*/
    stDataFilter.funSectionFunCallback = &SRH_ParsePAT;
    stDataFilter.pSectionStruct = (mt_u8 *)pat_tb;

    MTADP_SEARCH_INFO_PRINT("start data filter\n");

    s32Ret = DMX_SectionStartDataFilter(u32DmxID, &stDataFilter);
    if (s32Ret != MT_SUCCESS)
    {
        MTADP_SEARCH_ERR_PRINT("\n No PAT received \n");
    }
    MTADP_SEARCH_FUNCTION_EXIT();

    return s32Ret;
}

/*
@biref PMTRequest
@param[in] u32DmxID, ID of decoder
@param[in] pmt_tb, Handle to the pmt table
@param[in] u16PmtPid, Progam 's PMT ID
@param[in] u16ServiceId, Progam 's SERVICE ID
@return SUCCESS
*/
mt_s32 SRH_PMTRequest(mt_u32 u32DmxID, PMT_TB *pmt_tb, mt_u16 u16PmtPid, mt_u16 u16ServiceId)
{
    mt_s32 s32Ret = 0;
    DMX_DATA_FILTER_S stDataFilter = { 0 };


    memset(stDataFilter.u8Match, 0x00, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Mask, 0xff, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Negate, 0x00, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));

    stDataFilter.u32TSPID   = u16PmtPid;
    stDataFilter.u32TimeOut = PGPMT_TIMEOUT;
    stDataFilter.u16FilterDepth = 3;
    stDataFilter.u8Crcflag = 0;

    stDataFilter.u8Match[0] = PMT_TABLE_ID;
    stDataFilter.u8Mask[0] = 0x0;

    /* set service id*/
    stDataFilter.u8Match[1] = (mt_u8)((u16ServiceId >> 8) & 0x00ff);
    stDataFilter.u8Mask[1] = 0;

    stDataFilter.u8Match[2] = (mt_u8)(u16ServiceId & 0x00ff);
    stDataFilter.u8Mask[2] = 0;

    /* set call back func*/
    stDataFilter.funSectionFunCallback = &SRH_ParsePMT;
    stDataFilter.pSectionStruct = (mt_u8 *)pmt_tb;

    /* start data filter*/
    s32Ret = DMX_SectionStartDataFilter(u32DmxID, &stDataFilter);
    if (s32Ret == MT_FAILURE)
    {
        MTADP_SEARCH_ERR_PRINT("DMX_SectionStartDataFilter failed to receive PMT\n");
    }

    return s32Ret;
}


mt_s32 SRH_NITRequest(mt_u32 u32DmxID, NIT_TB *nit_tb)
{
    mt_s32 s32Ret;
    DMX_DATA_FILTER_S stDataFilter;

    MTADP_SEARCH_FUNCTION_ENTER();

    memset(stDataFilter.u8Match, 0x00, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Mask, 0xff, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Negate, 0x00, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));

    stDataFilter.u32TSPID   = NIT_TSPID;                      //set ts pid
    stDataFilter.u32TimeOut = PGSDT_TIMEOUT;    // timeout
    stDataFilter.u16FilterDepth = 4;
    stDataFilter.u8Crcflag = 0;

    stDataFilter.u8Match[0] = NIT_TABLE_ID_ACTUAL;
    stDataFilter.u8Mask[0] = 0x0;

    /* set call back func*/
    stDataFilter.funSectionFunCallback = &SRH_ParseNIT;
    stDataFilter.pSectionStruct = (mt_u8 *)nit_tb;

     /* start data filter*/
    s32Ret = DMX_SectionStartDataFilter(0, &stDataFilter);
    if (s32Ret == MT_FAILURE)
    {
        MTADP_SEARCH_ERR_PRINT("\n No SDT received\n");
    }

    MTADP_SEARCH_FUNCTION_EXIT();

    return (s32Ret);
}


mt_s32 SRH_CATRequest(mt_u32 u32DmxID, CAT_TB *cat_tb)
{
    mt_s32 s32Ret;
    DMX_DATA_FILTER_S stDataFilter;

#ifdef SRCH_DEBUG
    MTADP_SEARCH_INFO_PRINT("\n ++++ SDT Request");
#endif
    memset(stDataFilter.u8Match, 0x00, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Mask, 0xff, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Negate, 0x00, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));

    stDataFilter.u32TSPID   = CAT_TSPID;                      //set ts pid
    stDataFilter.u32TimeOut = PGSDT_TIMEOUT;    // timeout
    stDataFilter.u16FilterDepth = 1;
    stDataFilter.u8Crcflag = 0;

    stDataFilter.u8Match[0] = CAT_TABLE_ID;
    stDataFilter.u8Mask[0] = 0x0;

    /* set call back func*/
    stDataFilter.funSectionFunCallback = &SRH_ParseCAT;
    stDataFilter.pSectionStruct = (mt_u8 *)cat_tb;

     /* start data filter*/
    s32Ret = DMX_SectionStartDataFilter(0, &stDataFilter);
    if (s32Ret == MT_FAILURE)
    {
        MTADP_SEARCH_INFO_PRINT("\n No SDT received\n");
    }

    return (s32Ret);
}


static mt_s32 ParsePcmBRHeader(pcm_info_t *pcm, mt_u8* header)
{
    static const mt_u8 bits_per_samples[4] = {0, 16, 20, 24};
    static const mt_u8 channels[16] = {0, 1, 0, 2, 3, 3, 4, 4, 5, 6, 7, 8, 0, 0, 0, 0};
    mt_u8 channel_layout = header[2] >> 4;

    /* get the sample depth and derive the sample format from it */
    pcm->bits_per_coded_sample = bits_per_samples[header[3] >> 6];

    /* get the sample rate. Not all values are used. */
    switch (header[2] & 0x0f) {
        case 1:
            pcm->sample_rate = 48000;
            break;
        case 4:
            pcm->sample_rate = 96000;
            break;
        case 5:
            pcm->sample_rate = 192000;
            break;
        default:
            pcm->sample_rate = 0;
    }

    /*
    * get the channel number (and mapping). Not all values are used.
    * It must be noted that the number of channels in the MPEG stream can
    * differ from the actual meaningful number, e.g. mono audio still has two
    * channels, one being empty.
    */
    pcm->channels = channels[channel_layout];
    pcm->audio_is_pcm = 1;
    pcm->is_big_endian = 1;

    return MT_SUCCESS;
}

static mt_s32 pes_parse_pcm(mt_u8* data, pcm_info_t *pcm)
{
    mt_u32  stream_id;
    mt_u8  *p = NULL;
    mt_u32  pes_is_aligned;
    uint32_t header_len;

    if (data == NULL) {
        MTADP_SEARCH_ERR_PRINT("invalid data \n");
        return -1;
    }

    p = data;
    stream_id = p[3];

    pes_is_aligned = (p[6] & 4);
    header_len = p[8];

    p += header_len + 9;
    if (stream_id == 0xbd) {
        if (pes_is_aligned && ((p[0] & 0xf0) == 0xa0)) {
        pcm->audio_is_pcm = 1;
        pcm->is_big_endian = 1;
        pcm->channels = 2;
        pcm->bits_per_coded_sample = 16;
        pcm->sample_rate = 48000;
        } else {
            ParsePcmBRHeader(pcm, p);
        }
    } else {
        pcm->audio_is_pcm = 1;
        pcm->is_big_endian = 1;
        pcm->channels = 2;
        pcm->bits_per_coded_sample = 16;
        pcm->sample_rate = 48000;
    }

    return MT_SUCCESS;
}

/*
@biref DMX_DataRead_GetPcmInfo
@param[in] hChannel, dmx channel to get pes data
@param[in] u32TimeOutms, time out
@param[out] pcm, pcm info
@return SUCCESS
*/
static mt_s32 DMX_DataRead_GetPcmInfo(mt_handle hChannel, mt_u32 u32TimeOutms, pcm_info_t *pcm)
{
    mt_s32 s32Ret = 0;
    MT_UNF_DMX_DATA_S dmx_data[DMX_DEFAULT_BUF_NUM];
    mt_u32 num, i;
    mt_u32 u32Times = 0;
    mt_u32 RequestNum = DMX_DEFAULT_BUF_NUM;
    mt_u8* pbuf = NULL;
    mt_u32 pes_head_flag = MT_FALSE;

    u32Times = u32TimeOutms / 10;
    while (--u32Times)
    {
        num = 0;
        s32Ret = MT_UNF_DMX_AcquireBuf(hChannel, RequestNum, &num, dmx_data, u32TimeOutms);
        if (s32Ret != MT_SUCCESS)
        {
            usleep(5 * 1000);
            continue;
        }

        for (i = 0; i < num; i++)
        {
            pbuf = dmx_data[i].pu8Data;
            /*check if pes header location*/
            if (pbuf[0]==0 && pbuf[1]==0 && pbuf[2] == 1)
            {
                pes_head_flag = MT_TRUE;
                pes_parse_pcm(pbuf, pcm);
                break;
            }
        }

        s32Ret = MT_UNF_DMX_ReleaseBuf(hChannel, num, dmx_data);
        if (s32Ret != MT_SUCCESS)
        {
            MTADP_SEARCH_ERR_PRINT("call MT_UNF_DMX_ReleaseBuf fail, ret=0x%x\n", s32Ret);
            return s32Ret;
        }

        if (pes_head_flag) {
            pes_head_flag = MT_FALSE;
            break;
        }
    }

    return MT_SUCCESS;
}


/*
@biref MTADP_GetPcmInfo
@param[in] u32DmxID, ID of decoder
@param[in] audPid, audio pid
@param[out] pcm, pcm info
@return SUCCESS
*/
static mt_s32 MTADP_GetPcmInfo(mt_u32 u32DmxId, mt_u16 audPid, pcm_info_t *pcm)
{
    mt_s32 s32Ret;
    mt_handle hChan, hFilter;
    MT_UNF_DMX_CHAN_ATTR_S stChanAttr;
    MT_UNF_DMX_FILTER_ATTR_S stFilterAttr;


    memset(&stChanAttr, 0, sizeof(MT_UNF_DMX_CHAN_ATTR_S));
    s32Ret = MT_UNF_DMX_GetChannelDefaultAttr(&stChanAttr);
    if (MT_SUCCESS != s32Ret) {
        printf("call MT_UNF_DMX_GetChannelDefaultAttr fail, ret=0x%x\n", s32Ret);
        return s32Ret;
    }

    stChanAttr.u32BufSize = 188 * 1024;
    stChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_PES;
    stChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
    stChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    s32Ret = MT_UNF_DMX_CreateChannel(u32DmxId, &stChanAttr, &hChan);
    if (MT_SUCCESS != s32Ret) {
        printf("call MT_UNF_DMX_CreateChannel fail, ret=0x%x\n", s32Ret);
        return s32Ret;
    }

    s32Ret=MT_UNF_DMX_SetChannelPID(hChan, audPid);
    if (MT_SUCCESS != s32Ret) {
        printf("call MT_UNF_DMX_SetChannelPID fail, ret=0x%x\n", s32Ret);
        goto FREE_CHANNEL;
    }

    memset(stFilterAttr.au8Mask, 0xFF, DMX_FILTER_MAX_DEPTH); //not compare if mask is 0xff
    memset(stFilterAttr.au8Match, 0x0, DMX_FILTER_MAX_DEPTH);
    memset(stFilterAttr.au8Negate, 0x0, DMX_FILTER_MAX_DEPTH);
    stFilterAttr.u32FilterDepth = 0;
    s32Ret=MT_UNF_DMX_CreateFilter(u32DmxId, &stFilterAttr, &hFilter);
    if(MT_SUCCESS!=s32Ret) {
        printf("call MT_UNF_DMX_CreateFilter fail, ret=0x%x\n", s32Ret);
        goto FREE_CHANNEL;
    }

    s32Ret = MT_UNF_DMX_AttachFilter(hFilter, hChan);
    if (MT_SUCCESS != s32Ret) {
        printf("call MT_UNF_DMX_AttachFilter fail, ret=0x%x\n", s32Ret);
        goto FREE_FILTER;
    }

    s32Ret = MT_UNF_DMX_OpenChannel(hChan);
    if (MT_SUCCESS != s32Ret) {
        printf("call MT_UNF_DMX_OpenChannel fail, ret=0x%x\n", s32Ret);
        goto DETCH_FILTER;
    }

    s32Ret = DMX_DataRead_GetPcmInfo(hChan, 2000, pcm);
    if (MT_SUCCESS != s32Ret) {
        printf("call DMX_DataRead_GetPcmInfo fail, ret=0x%x\n", s32Ret);
    }

    MT_UNF_DMX_CloseChannel(hChan);

DETCH_FILTER:
    MT_UNF_DMX_DetachFilter(hFilter, hChan);

FREE_FILTER:
    MT_UNF_DMX_DestroyFilter(hFilter);

FREE_CHANNEL:
    MT_UNF_DMX_DestroyChannel(hChan);

    return s32Ret;
}


mt_s32 SRH_SDTRequest(mt_u32 u32DmxID, SDT_TB *sdt_tb)
{
    mt_s32 s32Ret;
    DMX_DATA_FILTER_S stDataFilter;

    MTADP_SEARCH_FUNCTION_ENTER();

#ifdef SRCH_DEBUG
    MTADP_SEARCH_INFO_PRINT("\n ++++ SDT Request");
#endif
    memset(stDataFilter.u8Match, 0x00, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Mask, 0xff, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Negate, 0x00, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));

    stDataFilter.u32TSPID   = SDT_TSPID;                      //set ts pid
    stDataFilter.u32TimeOut = PGSDT_TIMEOUT;    // timeout
    stDataFilter.u16FilterDepth = 4;
    stDataFilter.u8Crcflag = 0;

    stDataFilter.u8Match[0] = SDT_TABLE_ID_ACTUAL;
    stDataFilter.u8Mask[0] = 0x0;

    /* set call back func*/
    stDataFilter.funSectionFunCallback = &SRH_ParseSDT;
    stDataFilter.pSectionStruct = (mt_u8 *)sdt_tb;

     /* start data filter*/
    s32Ret = DMX_SectionStartDataFilter(0, &stDataFilter);
    if (s32Ret == MT_FAILURE)
    {
        MTADP_SEARCH_ERR_PRINT("\n No SDT received\n");
    }

    MTADP_SEARCH_FUNCTION_EXIT();

    return (s32Ret);
}

mt_s32 DVB_SearchStart(mt_u32 u32DmxID)
{
    DB_PROGRAM_S stProgInfo;
    PAT_TB pat_tb;
    PMT_TB pmt_tb;
    SDT_TB sdt_tb;

    mt_u32 i, j;
    mt_s32 s32Ret;
    mt_s32 ProgNum = -1;

    memset(&pat_tb, 0, sizeof(pat_tb));
    memset(&pmt_tb, 0, sizeof(pmt_tb));
    memset(&sdt_tb, 0, sizeof(sdt_tb));

    s32Ret = SRH_PATRequest(u32DmxID, &pat_tb);
    if (s32Ret == MT_FAILURE)
    {
        return MT_FAILURE;
    }

    for (i = 0; i < pat_tb.u16ProgNum; i++)
    {
        memset(&stProgInfo, 0, sizeof(DB_PROGRAM_S));
        stProgInfo.u16FrontendID = SEARCHING_FRONTEND_ID;
        stProgInfo.u16TsID   = pat_tb.u16TsID;
        stProgInfo.u16PmtPid = pat_tb.PatInfo[i].u16PmtPid;
        stProgInfo.u16ServiceID = pat_tb.PatInfo[i].u16ServiceID;
        MTADP_SEARCH_INFO_PRINT("stProgInfo.u16TsID=0x%x,stProgInfo.u16PmtPid=0x%x,stProgInfo.u16ServiceID=0x%x\n",
        stProgInfo.u16TsID,stProgInfo.u16PmtPid,stProgInfo.u16ServiceID);
        memset(&pmt_tb, 0, sizeof(PMT_TB));
        if (SRH_PMTRequest(u32DmxID, &pmt_tb, stProgInfo.u16PmtPid, stProgInfo.u16ServiceID) == MT_SUCCESS)
        {
            stProgInfo.u16PcrPid = pmt_tb.u16PcrPid;
            stProgInfo.u8AudioChannel = pmt_tb.u16AudoNum;
            for (j = 0; j <= pmt_tb.u16AudoNum; j++)
            {
                stProgInfo.AudioEX[j].u16audiopid = pmt_tb.Audioinfo[j].u16AudioPid;
                stProgInfo.AudioEX[j].u32AudioEncType = pmt_tb.Audioinfo[j].u32AudioEncType;
            }

            stProgInfo.u8VideoChannel = pmt_tb.u16VideoNum;
            stProgInfo.VideoEX.u16VideoPid = pmt_tb.Videoinfo[0].u16VideoPid;
            stProgInfo.VideoEX.u32VideoEncType = pmt_tb.Videoinfo[0].u32VideoEncType;
        }

        DB_AddDVBProg(&stProgInfo);
    }

    s32Ret = SRH_SDTRequest(u32DmxID, &sdt_tb);
    if (s32Ret != MT_SUCCESS)
    {
        return MT_FAILURE;
    }

    for (i = 0; i < sdt_tb.u32ProgNum; i++)
    {
        memset(&stProgInfo, 0, sizeof(DB_PROGRAM_S));

        ProgNum = DB_GetDVBProgInfoByServiceID(sdt_tb.SdtInfo[i].u16ServiceID, &stProgInfo);
        if (ProgNum == MT_ERR_PARAM)
        {
            continue;
        }

        stProgInfo.u16NetworkID  = sdt_tb.u16NetID;
        stProgInfo.enServiceType = sdt_tb.SdtInfo[i].u32ServiceType;
        memcpy(stProgInfo.ProgramName, sdt_tb.SdtInfo[i].s8ProgName, MAX_PROGNAME_LENGTH);
        DB_SetDVBProgInfo(ProgNum, &stProgInfo);
    }

    return MT_SUCCESS;
}

mt_void DVB_SaveSearch(mt_u32 u32FrontendID)
{
    DB_PROGRAM_S stProgInfo;

    mt_u32 i;

    for (i = 0; i < DB_GetProgTotalCount(); i++)
    {
        if (MT_FAILURE == DB_GetDVBProgInfo(i, &stProgInfo))
        {
            break;
        }

        if (stProgInfo.u16FrontendID == SEARCHING_FRONTEND_ID)
        {
            stProgInfo.u16FrontendID = u32FrontendID;
            DB_SetDVBProgInfo(i, &stProgInfo);
        }
    }
}

mt_void DVB_ListProg()
{
    mt_s32 i, j;
    DB_PROGRAM_S stProgInfo;

    MTADP_SEARCH_INFO_PRINT("\tList TV program\n");

    for (i = 0; i < DB_GetProgTotalCount(); i++)
    {
        if (MT_FAILURE != DB_GetDVBProgInfo(i, &stProgInfo))
        {
            MTADP_SEARCH_INFO_PRINT("\n%-10d %-20s \n", i, stProgInfo.ProgramName);
            if (stProgInfo.u8VideoChannel > 0)
            {
                MTADP_SEARCH_INFO_PRINT("\tVideo Stream PID = %d(0x%04x) ", stProgInfo.VideoEX.u16VideoPid,
                       stProgInfo.VideoEX.u16VideoPid);
                switch (stProgInfo.VideoEX.u32VideoEncType)
                {
                case MT_UNF_VCODEC_TYPE_H264:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type H264\n");
                    break;
                case MT_UNF_VCODEC_TYPE_MPEG4:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type MPEG4\n");
                    break;
                case MT_UNF_VCODEC_TYPE_MPEG2:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type MPEG2\n");
                    break;
                case MT_UNF_VCODEC_TYPE_HEVC:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type HEVC\n");
                    break;
                default:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type error\n");
                }
            }

            for (j = 0; j < stProgInfo.u8AudioChannel; j++)
            {
                MTADP_SEARCH_INFO_PRINT("\tAudio Stream PID = %d(0x%04x)", stProgInfo.AudioEX[j].u16audiopid,
                       stProgInfo.AudioEX[j].u16audiopid);
                switch (stProgInfo.AudioEX[j].u32AudioEncType)
                {
                case HA_AUDIO_ID_AAC:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type AAC\n");
                    break;
                case HA_AUDIO_ID_MP3:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type MP3\n");
                    break;
                case HA_AUDIO_ID_DOLBY_PLUS:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type AC3\n");
                    break;
                case HA_AUDIO_ID_DTSHD:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type DTSHD\n");
                    break;
                case HA_AUDIO_ID_DRA:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type DRA\n");
                    break;
                default:
                    MTADP_SEARCH_INFO_PRINT("\tStream Type error\n");
                }
            }
        }
    }

    MTADP_SEARCH_INFO_PRINT("\n\n");
}


mt_void MTADP_Search_Init()
{
    if (MT_FALSE == g_bSrchInit)
    {
        g_bSrchInit = MT_TRUE;
    }

    return;
}
mt_s32  MTADP_Search_get_proglist(PMT_COMPACT_TBL **pProgTable)
{
    if(g_ProgTable.proginfo != NULL)
    {
        *pProgTable = &g_ProgTable;
        return MT_SUCCESS;
    }
    return MT_FAILURE;

}


mt_s32  MTADP_Search_destory_proglist(void)
{
    if(g_ProgTable.proginfo != NULL)
    {
        free(g_ProgTable.proginfo);
        g_ProgTable.proginfo = MT_NULL;

        g_ProgTable.prog_num = 0;
        g_ProgTable.currentprog = MT_NULL;
        return MT_SUCCESS;
    }
    return MT_FAILURE;

}


static mt_s32  MTADP_Search_add_prog(PMT_COMPACT_TBL *pProgTable)
{
    PMT_COMPACT_PROG *prog_info = NULL;

    if(g_ProgTable.prog_num == 0)
    {
        g_ProgTable.proginfo = (PMT_COMPACT_PROG*)malloc(pProgTable->prog_num * sizeof(PMT_COMPACT_PROG));
        if(g_ProgTable.proginfo == NULL)
        {
            printf("g_ProgTable.proginfo is NULL \n");
            return MT_FAILURE;
        }
        g_ProgTable.prog_num = pProgTable->prog_num;
        memcpy(g_ProgTable.proginfo, pProgTable->proginfo, sizeof(PMT_COMPACT_PROG) * pProgTable->prog_num);
        g_ProgTable.currentprog = g_ProgTable.proginfo;

    }
    else
    {
        prog_info = (PMT_COMPACT_PROG*)malloc((g_ProgTable.prog_num + pProgTable->prog_num)* sizeof(PMT_COMPACT_PROG));

        memcpy(prog_info, g_ProgTable.proginfo, sizeof(PMT_COMPACT_PROG) * g_ProgTable.prog_num);
        memcpy(prog_info + g_ProgTable.prog_num, pProgTable->proginfo, sizeof(PMT_COMPACT_PROG) * pProgTable->prog_num);

        free(g_ProgTable.proginfo);
        g_ProgTable.proginfo = prog_info;
        g_ProgTable.prog_num = g_ProgTable.prog_num + pProgTable->prog_num;
        g_ProgTable.currentprog = g_ProgTable.proginfo;
    }


    return MT_SUCCESS ;
}

/*
@brief get PMT table
@param[in] u32DmxId, ID of decoder
@param[out] ppProgTable, Pass out the pmt data obtained
@return MT_SUCCESS
*/
mt_s32 MTADP_Search_GetAllPmt(mt_u32 u32DmxId, PMT_COMPACT_TBL **ppProgTable)
{
    mt_s32          i = 0;
    mt_s32          j = 0;
    mt_s32          s32Ret = 0;
    PAT_TB          pat_tb = { 0 };
    PMT_TB          pmt_tb = { 0 };
    SDT_TB          sdt_tb = { 0 };
    PMT_COMPACT_TBL *pProgTable = MT_NULL;
    mt_u32          u32Promnum = 0;
    mt_u32          u32Cnt = 0;
    mt_u32          u32SubtitleIndex = 0;
    mt_u32          u32SubtDespIndex = 0;

    if(!ppProgTable)
    {
        MTADP_SEARCH_ERR_PRINT("para is null pointer!\n");
        return MT_FAILURE;
    }

    MTADP_SEARCH_FUNCTION_ENTER();

    memset(&pat_tb, 0, sizeof(pat_tb));
    memset(&pmt_tb, 0, sizeof(pmt_tb));
    memset(&sdt_tb, 0, sizeof(sdt_tb));

    do
    {
        MTADP_SEARCH_INFO_PRINT("do .............  SRH_PATRequest \n");
        s32Ret = SRH_PATRequest(u32DmxId, &pat_tb);
        if(MT_SUCCESS != s32Ret)
        {
            MTADP_SEARCH_ERR_PRINT("failed to search PAT\n");
            return MT_FAILURE;
        }

        if(0 != pat_tb.u16ProgNum)
        {
            break;
        }
        MTADP_SEARCH_INFO_PRINT(" SRH_PATRequest fail. retry times: %d \n", u32Cnt);
        MTADP_SEARCH_INFO_PRINT(" If want to stop please enter \"Ctrl + c\" \n\n");
        MT_USLEEP(5000);
        u32Cnt += 1;
    }while(u32Cnt < 3);

    pProgTable = (PMT_COMPACT_TBL*)malloc(sizeof(PMT_COMPACT_TBL));
    if(MT_NULL == pProgTable)
    {
        MTADP_SEARCH_ERR_PRINT("have no memory for pat\n");
        return MT_FAILURE;
    }

    pProgTable->prog_num = pat_tb.u16ProgNum;
    pProgTable->proginfo = (PMT_COMPACT_PROG*)malloc(pat_tb.u16ProgNum * sizeof(PMT_COMPACT_PROG));
    if(MT_NULL == pProgTable->proginfo)
    {
        MTADP_SEARCH_ERR_PRINT("have no memory for pat\n");
        free(pProgTable);
        pProgTable = MT_NULL;
        return MT_FAILURE;
    }

    memset(pProgTable->proginfo, 0, pat_tb.u16ProgNum * sizeof(PMT_COMPACT_PROG));

    MT_SEARCH_PRINT("ALL Program Infomation[%d]:\n",pat_tb.u16ProgNum);

    for(i = 0; i < pat_tb.u16ProgNum; i++)
    {
        if ((pat_tb.PatInfo[i].u16ServiceID == 0) || (pat_tb.PatInfo[i].u16PmtPid == 0x1fff))
        {
            continue;
        }

        memset(&pmt_tb, 0, sizeof(PMT_TB));
        s32Ret = SRH_PMTRequest(u32DmxId, &pmt_tb, pat_tb.PatInfo[i].u16PmtPid, pat_tb.PatInfo[i].u16ServiceID);
        if(MT_SUCCESS != s32Ret)
        {
            MTADP_SEARCH_ERR_PRINT("failed to search PMT[%d]\n", i);
            continue;
        }
        pProgTable->proginfo[u32Promnum].ProgID = pat_tb.PatInfo[i].u16ServiceID;
        pProgTable->proginfo[u32Promnum].PmtPid = pat_tb.PatInfo[i].u16PmtPid;
        pProgTable->proginfo[u32Promnum].PcrPid = pmt_tb.u16PcrPid;
        pProgTable->proginfo[u32Promnum].VideoType   = pmt_tb.Videoinfo[0].u32VideoEncType;
        pProgTable->proginfo[u32Promnum].VElementNum = pmt_tb.u16VideoNum;
        pProgTable->proginfo[u32Promnum].VElementPid = pmt_tb.Videoinfo[0].u16VideoPid;
        pProgTable->proginfo[u32Promnum].AElementNum = pmt_tb.u16AudoNum;

        for(mt_s32 n = 0;n < pmt_tb.u16AudoNum; n++)
        {
            if(pmt_tb.Audioinfo[n].u32audiotype != 0x03)
            {
                pProgTable->proginfo[u32Promnum].AElementPid = pmt_tb.Audioinfo[n].u16AudioPid;
                pProgTable->proginfo[u32Promnum].AudioType   = pmt_tb.Audioinfo[n].u32AudioEncType;
                break;
            }
        }

        pProgTable->proginfo[u32Promnum].u16SubtitlingNum = pmt_tb.u16SubtitlingNum;
        pProgTable->proginfo[u32Promnum].u16SCTESubtNum = pmt_tb.u16SCTESubtNum;
        pProgTable->proginfo[u32Promnum].u16ClosedCaptionNum= pmt_tb.u16ClosedCaptionNum;
        memcpy(pProgTable->proginfo[u32Promnum].stClosedCaption, pmt_tb.stClosedCaption, sizeof(pmt_tb.stClosedCaption));
        pProgTable->proginfo[u32Promnum].u16ARIBCCPid = pmt_tb.u16ARIBCCPid;
        pProgTable->proginfo[u32Promnum].u16TtxNum = pmt_tb.u16TtxNum;
        memcpy(pProgTable->proginfo[u32Promnum].stTtxInfo, pmt_tb.stTtxInfo, sizeof(PMT_TTX_S)*TTX_MAX);
        memcpy(pProgTable->proginfo[u32Promnum].u8PmtData, pmt_tb.u8PmtData, pmt_tb.u32PmtLen);
        pProgTable->proginfo[u32Promnum].u32PmtLen = pmt_tb.u32PmtLen;

        for(j=0; j<pmt_tb.u16AudoNum; j++)
        {
            /* added by gaoyanfeng 00182102 for multi-audio begin */
            if (j < PROG_MAX_AUDIO)
            {
                pProgTable->proginfo[u32Promnum].Audioinfo[j].u16AudioPid = pmt_tb.Audioinfo[j].u16AudioPid;
                pProgTable->proginfo[u32Promnum].Audioinfo[j].u32AudioEncType = pmt_tb.Audioinfo[j].u32AudioEncType;
                pProgTable->proginfo[u32Promnum].Audioinfo[j].u32audiotype = pmt_tb.Audioinfo[j].u32audiotype;
                pProgTable->proginfo[u32Promnum].Audioinfo[j].has_audio_preselection = pmt_tb.Audioinfo[j].has_audio_preselection;
                memcpy(pProgTable->proginfo[u32Promnum].Audioinfo[j].language_code, pmt_tb.Audioinfo[j].language_code, 4);

                if (pmt_tb.Audioinfo[j].u32AudioEncType == HA_AUDIO_ID_PCM) {
                    MTADP_GetPcmInfo(u32DmxId, pmt_tb.Audioinfo[j].u16AudioPid, &pProgTable->proginfo[u32Promnum].Audioinfo[j].pcm);
                }

                if (pmt_tb.Audioinfo[j].has_audio_preselection) {
                    memcpy(&pProgTable->proginfo[u32Promnum].Audioinfo[j].audio_preselection_descriptor,
                        &pmt_tb.Audioinfo[j].audio_preselection_descriptor, sizeof(audio_preselection_descriptor_t));
                }


            }
            /* added by gaoyanfeng 00182102 for multi-audio end */
        }

       pProgTable->proginfo[u32Promnum].u16CANum = pmt_tb.u16CANum;
       for(j=0; j<pmt_tb.u16CANum; j++)
       {
           if(j < PROG_MAX_CA)
           {
                pProgTable->proginfo[u32Promnum].CASystem[j].u16CASystemID = pmt_tb.CASystem[j].u16CASystemID;
                pProgTable->proginfo[u32Promnum].CASystem[j].u16CAPID = pmt_tb.CASystem[j].u16CAPID;
           }
        }

        /* parse and deal with subtitling info */
        for(u32SubtitleIndex=0; u32SubtitleIndex < pProgTable->proginfo[u32Promnum].u16SubtitlingNum; u32SubtitleIndex++)
        {
            pProgTable->proginfo[u32Promnum].SubtitingInfo[u32SubtitleIndex].u16SubtitlingPID =
                pmt_tb.SubtitingInfo[u32SubtitleIndex].u16SubtitlingPID;

            pProgTable->proginfo[u32Promnum].SubtitingInfo[u32SubtitleIndex].u8DesInfoCnt = pmt_tb.SubtitingInfo[u32SubtitleIndex].u8DesInfoCnt;


            for(u32SubtDespIndex=0; u32SubtDespIndex < pProgTable->proginfo[u32Promnum].SubtitingInfo[u32SubtitleIndex].u8DesInfoCnt; u32SubtDespIndex++)
            {
                pProgTable->proginfo[u32Promnum].SubtitingInfo[u32SubtitleIndex].DesInfo[u32SubtDespIndex].u32LangCode =
                    pmt_tb.SubtitingInfo[u32SubtitleIndex].DesInfo[u32SubtDespIndex].u32LangCode;

                pProgTable->proginfo[u32Promnum].SubtitingInfo[u32SubtitleIndex].DesInfo[u32SubtDespIndex].u16PageID =
                    pmt_tb.SubtitingInfo[u32SubtitleIndex].DesInfo[u32SubtDespIndex].u16PageID;

                pProgTable->proginfo[u32Promnum].SubtitingInfo[u32SubtitleIndex].DesInfo[u32SubtDespIndex].u16AncillaryPageID =
                    pmt_tb.SubtitingInfo[u32SubtitleIndex].DesInfo[u32SubtDespIndex].u16AncillaryPageID;
            }
        }

        /* parse and deal with scte subtitle info */
        for(u32SubtitleIndex=0; u32SubtitleIndex < pProgTable->proginfo[u32Promnum].u16SCTESubtNum; u32SubtitleIndex++)
        {
            pProgTable->proginfo[u32Promnum].stSCTESubtInfo[u32SubtitleIndex].u16SCTESubtPID = pmt_tb.stSCTESubtInfo[u32SubtitleIndex].u16SCTESubtPID;
        }

        MT_SEARCH_PRINT("Channel Num = %d, Program ServiceID = %d PMT PID = 0x%x,\n", u32Promnum + 1, pat_tb.PatInfo[i].u16ServiceID,
               pat_tb.PatInfo[i].u16PmtPid);
        for (j = 0; j < pmt_tb.u16VideoNum; j++)
        {
            MT_SEARCH_PRINT("\tVideo Stream PID = %d\n",pmt_tb.Videoinfo[j].u16VideoPid);
            switch (pmt_tb.Videoinfo[j].u32VideoEncType)
            {
            case MT_UNF_VCODEC_TYPE_AVS:
                MT_SEARCH_PRINT("\tVideo Stream Type AVS\n");
                break;
            case MT_UNF_VCODEC_TYPE_H264:
                MT_SEARCH_PRINT("\tVideo Stream Type H264\n");
                break;
            case MT_UNF_VCODEC_TYPE_MPEG2:
                MT_SEARCH_PRINT("\tVideo Stream Type MPEG2\n");
                break;
            case MT_UNF_VCODEC_TYPE_MPEG4:
                MT_SEARCH_PRINT("\tVideo Stream Type MPEG4\n");
                break;
            case MT_UNF_VCODEC_TYPE_HEVC:
                MT_SEARCH_PRINT("\tVideo Stream Type HEVC\n");
                break;
            case MT_UNF_VCODEC_TYPE_VC1:
                MT_SEARCH_PRINT("\tVideo Stream Type VC1\n");
                break;
            case MT_UNF_VCODEC_TYPE_VP8:
                MT_SEARCH_PRINT("\tVideo Stream Type VP8\n");
                break;
            case MT_UNF_VCODEC_TYPE_VP9:
                MT_SEARCH_PRINT("\tVideo Stream Type VP9\n");
                break;
            case MT_UNF_VCODEC_TYPE_AVS2:
                MT_SEARCH_PRINT("\tVideo Stream Type AVS2\n");
                break;
            default:
                MT_SEARCH_PRINT("\tVideo Stream Type error\n");
            }
        }

        for (j = 0; j < pmt_tb.u16AudoNum; j++)
        {
            MT_SEARCH_PRINT("\tAudio Stream PID = %d\n", pmt_tb.Audioinfo[j].u16AudioPid);
            switch (pmt_tb.Audioinfo[j].u32AudioEncType)
            {
            case HA_AUDIO_ID_PCM:
                MT_SEARCH_PRINT("\tAudio Stream Type PCM\n");
                break;
            case HA_AUDIO_ID_MP2:
                MT_SEARCH_PRINT("\tAudio Stream Type MP2\n");
                break;
            case HA_AUDIO_ID_MP3:
                MT_SEARCH_PRINT("\tAudio Stream Type MP3\n");
                break;
            case HA_AUDIO_ID_AAC:
                MT_SEARCH_PRINT("\tAudio Stream Type AAC\n");
                break;
            case HA_AUDIO_ID_DRA:
                MT_SEARCH_PRINT("\tAudio Stream Type DRA\n");
                break;
            case HA_AUDIO_ID_WMA9STD:
                MT_SEARCH_PRINT("\tAudio Stream Type WMA9STD\n");
                break;
            case HA_AUDIO_ID_DOLBY_PLUS:
                MT_SEARCH_PRINT("\tAudio Stream Type DOLBY_PLUS\n");
                break;
            case HA_AUDIO_ID_DOLBY_TRUEHD:
                MT_SEARCH_PRINT("\tAudio Stream Type DOLBY_TRUEHD\n");
                break;
            case HA_AUDIO_ID_DOLBY_CONVERT:
                MT_SEARCH_PRINT("\tAudio Stream Type DOLBY_CONVERT\n");
                break;
            case HA_AUDIO_ID_DTSHD:
                MT_SEARCH_PRINT("\tAudio Stream Type DTSHD\n");
                break;
            case HA_AUDIO_ID_AC3PASSTHROUGH:
                MT_SEARCH_PRINT("\tAudio Stream Type AC3PASSTHROUGH\n");
                break;
            case HA_AUDIO_ID_EAC3PASSTHROUGH:
                MT_SEARCH_PRINT("\tAudio Stream Type EAC3PASSTHROUGH\n");
                break;
            case HA_AUDIO_ID_DTSPASSTHROUGH:
                MT_SEARCH_PRINT("\tAudio Stream Type DTSPASSTHROUGH\n");
            case HA_AUDIO_ID_DOLBY_AC4:
                MT_SEARCH_PRINT("\tAudio Stream Type AC4\n");
                break;
            case HA_AUDIO_ID_VVID:
                MT_SEARCH_PRINT("\tAudio Stream Type VIVID\n");
                break;
            default:
                MT_SEARCH_PRINT("\tAudio Stream Type error\n");
            }

            MTADP_SEARCH_INFO_PRINT("audio = %d language=[%s]\n", pmt_tb.Audioinfo[j].u32audiotype, pmt_tb.Audioinfo[j].language_code);
        }

        if(pmt_tb.u16SubtitlingNum > 0)
        {
            pProgTable->proginfo[u32Promnum].SubtType |= SUBT_TYPE_DVB;
            for(j = 0; j < pmt_tb.u16SubtitlingNum; j++)
            {
                MT_SEARCH_PRINT("\tDVB subtitle PID  = 0x%x\n",pmt_tb.SubtitingInfo[j].u16SubtitlingPID);
            }
        }

        if(pmt_tb.u16SCTESubtNum > 0)
        {
            pProgTable->proginfo[u32Promnum].SubtType |= SUBT_TYPE_SCTE;
            for(j = 0; j < pmt_tb.u16SCTESubtNum; j++)
            {
                MT_SEARCH_PRINT("\tSCTE subtitle PID  = 0x%x\n",pmt_tb.stSCTESubtInfo[j].u16SCTESubtPID);
            }
        }

        if(pProgTable->proginfo[u32Promnum].u16ClosedCaptionNum > 0)
        {

            for(j = 0; j < pProgTable->proginfo[u32Promnum].u16ClosedCaptionNum; j++)
            {
                if(pProgTable->proginfo[u32Promnum].stClosedCaption[j].u8IsDigitalCC)
                {
                    MT_SEARCH_PRINT("\tClosed Captioning 708, language : %#x, service num : %d\n",
                    pProgTable->proginfo[u32Promnum].stClosedCaption[j].u32LangCode,
                    pProgTable->proginfo[u32Promnum].stClosedCaption[j].u8ServiceNumber);
                }
                else
                {
                    MTADP_SEARCH_ERR_PRINT("\tClosed Captioning 608\n");
                }
            }
        }

        if(pProgTable->proginfo[u32Promnum].u16ARIBCCPid)
        {
            MT_SEARCH_PRINT("\tARIB CC PID            = 0x%x\n",pProgTable->proginfo[u32Promnum].u16ARIBCCPid);
        }

        if(pProgTable->proginfo[u32Promnum].u16TtxNum > 0)
        {
            MT_SEARCH_PRINT("\tTeletext NUM           = %d \n",pmt_tb.u16TtxNum);
            for(j = 0; j < pProgTable->proginfo[u32Promnum].u16TtxNum; j++)
            {
                MT_SEARCH_PRINT("\tTeletext %d PID            = 0x%x\n",j, pmt_tb.stTtxInfo[j].u16TtxPID);
            }
        }
        if(pProgTable->proginfo[u32Promnum].u16CANum > 0)
        {
            for(j = 0; j < pProgTable->proginfo[u32Promnum].u16CANum; j++)
            {
                MT_SEARCH_PRINT("\tCA %d PID            = 0x%x\n", j, pProgTable->proginfo[u32Promnum].CASystem[j].u16CAPID);
            }

        }

        if ((pmt_tb.u16VideoNum > 0) || (pmt_tb.u16AudoNum > 0))
        {
            u32Promnum++;
        }
    }

    pProgTable->prog_num = u32Promnum;

    *ppProgTable = pProgTable;


    MTADP_Search_add_prog(pProgTable);

    return u32Promnum ? MT_SUCCESS : MT_FAILURE;
}

/*
@brief Release the obtained pmt data
@param[in] pProgTable, pmt data pointer
@return MT_SUCCESS
*/
mt_s32  MTADP_Search_FreeAllPmt(PMT_COMPACT_TBL *pProgTable)
{



    if (MT_NULL != pProgTable)
    {
        if (MT_NULL != pProgTable->proginfo)
        {
            free(pProgTable->proginfo);
            pProgTable->proginfo = MT_NULL;
        }

        free(pProgTable);
        pProgTable = MT_NULL;
    }

    return MT_SUCCESS;
}


mt_s32 MTADP_Get_Current_Info(PMT_COMPACT_PROG **stCurrentProgInfo)
{
    if (g_ProgTable.currentprog != NULL)
    {
        *stCurrentProgInfo = g_ProgTable.currentprog;
        return MT_SUCCESS;
    }
    return MT_FAILURE;
}

mt_s32 MTADP_Set_Current_Info(PMT_COMPACT_PROG *stCurrentProgInfo)
{

    g_ProgTable.currentprog = stCurrentProgInfo;

    return MT_SUCCESS;


}



mt_void MTADP_Search_DeInit()
{

    if (MT_TRUE == g_bSrchInit)
    {
        g_bSrchInit = MT_FALSE;
    }

    MTADP_Search_destory_proglist();

    return;
}
