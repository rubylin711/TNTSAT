/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "string.h"
#include <sys/time.h>
#include "mt_type.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "mt_cc_fifo.h"
#include "mt_cc_parse.h"
#include "mt_cc_arib.h"

/*---------------------------------------------------------------------------*/
/* local defines                                                             */
/*---------------------------------------------------------------------------*/

//#define DEBUG_CC  1
#define  CC_DEBUG_LEVEL  0
#ifdef DEBUG_CC
#define CC_PRINTF(level, format, args...) if(level < CC_DEBUG_LEVEL) {char msg[100]; snprintf(msg,100,format,##args); printf("\r\n [CC: ] %s\n", msg);}
#else
#define CC_PRINTF(args...)
#endif

#define CC_MAX_PES_SIZE             (32 * 1024)
#define CC_MAX_DECODED_MAX_SIZE     (3 * 1024)
#define CC_MAX_TMP_BUF_SIZE         255
#define CC_MAX_CSI_BUF_SIZE         15

/*---------------------------------------------------------------------------*/
/* local datatypes                                                           */
/*---------------------------------------------------------------------------*/

typedef struct {

    mt_u8  isPAPF;   /* Parameterized active position forward */
    mt_u8  isAPS;    /* Active position set                   */
    mt_u8  isCOL;    /* Colour Controls                       */
    mt_u8  isSZX;    /* Character Size Controls               */
    mt_u8  isFLC;    /* Flashing control                      */
    mt_u8  isPOL;    /* Pattern Polarity Controls             */
    mt_u8  isHLC;    /* HIGHLIGHTING CHARACTER BLOCK          */
    mt_u8  isRPC;    /* Repeat Character                      */
    mt_u8  isTIME;   /* Time Controls                         */
    mt_u8  isCSI;    /* Control Sequence Introducer           */

    mt_u8  isSpecial;

    mt_u8  isGetChar;
    mt_u8  fNewControlCode;

    mt_u8  paramCSI[CC_MAX_CSI_BUF_SIZE];
    mt_u32 byteNumCSI;

    CC_DataInfo *tmpDataInfo;

    mt_u8  *pWrite;

} CC_TmpInfo;

/*---------------------------------------------------------------------------*/
/* local variables                                                           */
/*---------------------------------------------------------------------------*/

static mt_u8 TCS = 0;
static mt_u8 UTF = 0;
static CC_TmpInfo ccTmpInfo;
static CC_TmpInfo *tmpInfo;

static mt_u8 pes_buf[CC_MAX_PES_SIZE];
static mt_u8 read_buf[CC_MAX_TMP_BUF_SIZE];

/*---------------------------------------------------------------------------*/
/* extern functions                                                          */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* prototype functions declaration                                           */
mt_s32 CC_Init (void);
mt_s32 CC_AnalyzePes(mt_u8 * pesPacket,mt_u32 pesPacketSize,CC_HeaderInfo* headerInfo,
    mt_u8* dataUnit);
mt_s32 CC_DecodeDataUnit(mt_u8* dataUnit,    CC_DataInfo* dataInfo,mt_u32 *leftDataUnitSize);
/*---------------------------------------------------------------------------*/
mt_s32 CC_Init (void)
{

    memset(&ccTmpInfo,0,sizeof(CC_TmpInfo));

    return MT_SUCCESS;
}

/*!
*******************************************************************************
** \brief  Set draw character.
**
** \return Nothing
**
** \sa
******************************************************************************/
static void CC_SetDrawChar(mt_u8 charData)
{
    *(tmpInfo->pWrite) = charData;
    tmpInfo->pWrite = tmpInfo->pWrite + 1;

    memset(tmpInfo->pWrite, 0x00, 1);
    memset(tmpInfo->pWrite + 1, 0x00, 1);

    tmpInfo->tmpDataInfo->charcount ++;
    tmpInfo->isSpecial = 0;
    tmpInfo->isGetChar = 1;
}

/*!
*******************************************************************************
** \brief Get Control Sequence Introducer parameter.
**        When intermediate character is 0x20 only.
**
** \return Nothing
**
** \sa
******************************************************************************/
static void CC_GetCSI1(mt_u8 *CSI_param, mt_u8 *param1, mt_u32 *num1)
{
    mt_u32 i;

    for (i = 0; i < tmpInfo->byteNumCSI; i++)
    {
        if (CSI_param[i] == 0x20)
        {
            *num1 = i;
        }
    }
    for (i = 0; i < *num1; i++)
    {
        param1[i] = CSI_param[i];
    }

    tmpInfo->byteNumCSI = 0;
    tmpInfo->isCSI = 0;
}

/*!
*******************************************************************************
** \brief Get Control Sequence Introducer parameter.
**        When intermediate character is 0x3B and 0x20.
**
** \return Nothing
**
** \sa
******************************************************************************/
static void CC_GetCSI2(mt_u8 *CSI_param, mt_u8 *param1, mt_u8 *param2, mt_u32 *num1, mt_u32 *num2)
{
    mt_u32 i;

    for (i = 0; i < tmpInfo->byteNumCSI; i++)
    {
        if (CSI_param[i] == 0x3B)
        {
            *num1 = i;
        }
        else if (CSI_param[i] == 0x20)
        {
            *num2 = i;
        }
    }

    memcpy(param1, CSI_param, *num1);
    memcpy(param2, CSI_param+(*num1)+1, ((*num2) - (*num1)) - 1);
    *num2 = ((*num2) - (*num1)) - 1;

    tmpInfo->byteNumCSI = 0;
    tmpInfo->isCSI = 0;

}

/*!
*******************************************************************************
** \brief  Detect control code.
**
** \return Nothing
**
** \sa
******************************************************************************/
static void CC_ControlCodeChange(mt_u8 *flag, mt_u8 *isFlag, mt_u8 *mem, mt_u8 value)
{
    if (!tmpInfo->isGetChar)
    {
        if (flag != NULL)
        {
            *flag = 1;
        }
        if (isFlag != NULL)
        {
            *isFlag = 1;
        }
        if ((mem != NULL) && (value != 0))
        {
            *mem = value;
        }
    }
    else
    {
        tmpInfo->isGetChar = 0;
        tmpInfo->fNewControlCode = 1;
    }

}


mt_s32 CC_AnalyzePes(mt_u8 * pesPacket,
                      mt_u32 pesPacketSize,
                      CC_HeaderInfo* headerInfo,
                      mt_u8* dataUnit)
{
    mt_u32 i;
    mt_u8* pRead;
    mt_u32 readSize = 0;
    mt_u8 data_identifier;
    mt_u8 private_stream_id;
    mt_u8 PES_data_packet_header_length;
    mt_u32 data_unit_loop_length;
    mt_u8 unit_separator;

    /* argument NULL check */
    if ( (pesPacket == NULL) || (headerInfo == NULL) )
    {
        CC_PRINTF(2,"NULL ERROR\n");
        return MT_FAILURE;
    }

    if(CC_DEBUG_LEVEL > 0)
    {
        CC_PRINTF(2,"\npesPacketSize=%d\n", pesPacketSize);
        int j, count;
        count = 0;
        for(j = 0; j<pesPacketSize; j++)
        {
            CC_PRINTF(2,"%02x ",pesPacket[j]);
            count++;
            if(count % 16 == 0) CC_PRINTF(2,"\n");
        }
        CC_PRINTF(2,"\n");
    }

    memcpy(pes_buf, pesPacket, pesPacketSize);
    pRead = pes_buf;

    /***************************************/
    /* CC DATA check                       */
    /***************************************/
    /* data_identifier check */
    memcpy(read_buf, pRead, 1);
    pRead += 1;
    readSize += 1;
    data_identifier = *read_buf;
    CC_PRINTF(1,"data_identifier=0x%x\n", data_identifier);
    if (data_identifier != 0x80 && data_identifier != 0x81)
    {
        CC_PRINTF(2,"data_identifier = %x\n", data_identifier);
        return MT_FAILURE;
    }

    /* private_stream_id check */
    memcpy(read_buf, pRead, 1);
    pRead += 1;
    readSize += 1;
    private_stream_id = *read_buf;
    CC_PRINTF(1,"private_stream_id=0x%x\n", private_stream_id);
    if (private_stream_id != 0xFF)
    {
        CC_PRINTF(2,"private_stream_id = %x\n", private_stream_id);
    }

    memcpy(read_buf, pRead, 1);
    pRead += 1;
    readSize += 1;

    /* get PES_data_packet_header_length */
    PES_data_packet_header_length = *read_buf;// & 0x0f;
    CC_PRINTF(1,"PES_data_packet_header_length=0x%x\n", PES_data_packet_header_length);

    /* get PES_data_private_data_byte */
    if (PES_data_packet_header_length != 0)
    {
//        if (PES_data_packet_header_length > CC_MAX_TMP_BUF_SIZE) {
//            CC_PRINTF(2,"PES_data_packet_header_length ERROR\n");
//            return ERR_PARAM;
//        }
        memcpy(read_buf, pRead, PES_data_packet_header_length);
        pRead += PES_data_packet_header_length + 3;
        readSize += PES_data_packet_header_length + 3;
    }

    memcpy(read_buf, pRead, 1);
    pRead += 1;

    /* get data_group_id */
    headerInfo->data_group_id = (*read_buf & 0xfc) >> 2;
    CC_PRINTF(1,"data_group_id=0x%x\n", headerInfo->data_group_id);

    /* get data_group_version */
    headerInfo->data_group_version = *read_buf & 0x03;

    /* get data_group_link_number */
    memcpy(read_buf, pRead, 1);
    pRead += 1;
    headerInfo->data_group_link_number = *read_buf;
    if (headerInfo->data_group_link_number != 0x00)
    {
        CC_PRINTF(2,"data_group_link_number = %x\n", headerInfo->data_group_link_number);
    }

    /* last_data_group_link_number */
    memcpy(read_buf, pRead, 1);
    pRead += 1;
    headerInfo->last_data_group_link_number = *read_buf;
    if (headerInfo->last_data_group_link_number != 0x00)
    {
        CC_PRINTF(2,"last_data_group_link_number = %x\n", headerInfo->last_data_group_link_number);
    }

    // NOTE: data_group_size currently unused.
    //memcpy(read_buf, pRead, 2);
    pRead += 2;
    //data_group_size = (*read_buf)*256+*(read_buf+1);

    ///////////////////////
    /// management data ///
    ///////////////////////
    if ( (headerInfo->data_group_id == 0x0)
      || (headerInfo->data_group_id == 0x20) )
    {

        headerInfo->is_management = 1;

        /* get TMD */
        memcpy(read_buf, pRead, 1);
        pRead += 1;
        headerInfo->TMD = (*read_buf & 0xc0) >> 6;
        if (headerInfo->TMD != 0x00)
        {
            CC_PRINTF(2,"management TMD = %x\n", headerInfo->TMD);
        }
        
        if(headerInfo->TMD == 0x02)
        {
            pRead += 5;   
        }
        /* get num_languages */
        memcpy(read_buf, pRead, 1);
        pRead += 1;
        headerInfo->num_languages = *read_buf;
        if (headerInfo->num_languages > 2)
        {
            CC_PRINTF(1,"management num_languages = %x\n", headerInfo->num_languages);
        }

        for (i = 0; i < headerInfo->num_languages; i++)
        {
            memcpy(read_buf, pRead, 1);
            pRead += 1;

            /* get language_tag */
            headerInfo->language_tag = (*read_buf & 0xe0) >> 5;

            /* get DMF */
            headerInfo->DMF = (*read_buf & 0x0f);
            if ( (headerInfo->DMF != 0x02) && (headerInfo->DMF == 0x0a) ) {
                CC_PRINTF(2,"management DMF = %x\n", headerInfo->DMF);
            }

            /* get ISO_639_language_code */
            memcpy(read_buf, pRead, 3);
            pRead += 3;
            memcpy(headerInfo->ISO_639_language_code, read_buf, 3);

            memcpy(read_buf, pRead, 1);
            pRead += 1;

            /* Format */
            headerInfo->Format = (*read_buf & 0xf0) >> 4;
            if ( (headerInfo->Format != 0x08)
              && (headerInfo->Format != 0x09)
              && (headerInfo->Format != 0x0A)
              && (headerInfo->Format != 0x0B) )
            {
                CC_PRINTF(2,"management Format = %x\n", headerInfo->Format);
            }

            /* TCS */
            headerInfo->TCS = (*read_buf & 0x0f) >> 2;
            TCS = headerInfo->TCS;
            if (headerInfo->TCS != 0x00)
            {
                CC_PRINTF(2,"management TCS = %x\n", headerInfo->TCS);
            }

            /* rollup_mode */
            headerInfo->rollup_mode = (*read_buf & 0x03);
        }
    }
    else
    {
        /////////////////////
        ///  caption data ///
        /////////////////////
        headerInfo->is_management = 0;

        /* get TMD */
        memcpy(read_buf, pRead, 1);
        pRead += 1;
        headerInfo->TMD = (*read_buf & 0xc0) >> 6;
        if (headerInfo->TMD != 0x00)
        {
            CC_PRINTF(2,"TMD = %x\n", headerInfo->TMD);
        }
    }

    /* get data_unit_loop_length */
    memcpy(read_buf, pRead, 3);
    pRead += 3;
    data_unit_loop_length = ((*read_buf)*65536)
                          + (*(read_buf+1)*256)
                          + (*(read_buf+2));

    CC_PRINTF(1,"data_unit_loop_length=0x%x\n", data_unit_loop_length);
    if (data_unit_loop_length)
    {
        /* unit_separator check */
        memcpy(read_buf, pRead, 1);
        pRead += 1;
        unit_separator = *read_buf;
        if (unit_separator != 0x1f)
        {
            CC_PRINTF(2,"unit_separator = %x\n", unit_separator);
        }

        /* get data_unit_parameter */
        memcpy(read_buf, pRead, 1);
        pRead += 1;
        headerInfo->data_unit_parameter = *read_buf;
        if ( (headerInfo->data_unit_parameter != 0x20)
          && (headerInfo->data_unit_parameter != 0x35)
          && (headerInfo->data_unit_parameter != 0x30)
          && (headerInfo->data_unit_parameter != 0x31) )
        {
            CC_PRINTF(2,"data_unit_parameter = %x\n", headerInfo->data_unit_parameter);
        }

        /* get data_unit_size */
        memcpy(read_buf, pRead, 3);
        pRead += 3;
        headerInfo->data_unit_size = ((*read_buf)*65536)
                                   + (*(read_buf+1)*256)
                                   + (*(read_buf+2));

        /***************************/
        /* get data_unit_data_byte */
        /***************************/

        if (headerInfo->data_unit_size > CC_MAX_DECODED_MAX_SIZE)
        {
            CC_PRINTF(1,"data_unit_size ERROR\n");
            return MT_FAILURE;
        }

        CC_PRINTF(1,"data_unit_size=0x%x\n", headerInfo->data_unit_size);
        memcpy(dataUnit, pRead, headerInfo->data_unit_size);
    }

    return MT_SUCCESS;

}

mt_s32 CC_DecodeDataUnit(mt_u8* dataUnit,
                          CC_DataInfo* dataInfo,
                          mt_u32 *leftDataUnitSize)
{
    mt_u32 i;
    mt_u32 leftSize;

    tmpInfo = &ccTmpInfo;
    tmpInfo->tmpDataInfo = dataInfo;

    tmpInfo->tmpDataInfo->charcount = 0;
    tmpInfo->fNewControlCode = 0;
    tmpInfo->tmpDataInfo->APB_num = 0;
    tmpInfo->tmpDataInfo->APF_num = 0;
    tmpInfo->tmpDataInfo->APD_num = 0;
    tmpInfo->tmpDataInfo->APU_num = 0;
    tmpInfo->tmpDataInfo->APR_num = 0;

    leftSize = *leftDataUnitSize;

    tmpInfo->pWrite = tmpInfo->tmpDataInfo->decodedData;

    /***************************/
    /* decode character code   */
    /***************************/
    for (i = 0; i < leftSize; i++)
    {
        if (tmpInfo->fNewControlCode)
        {
            return MT_SUCCESS;
        }

        /* Get Active position parameter */
        if (tmpInfo->isPAPF)
        {
            tmpInfo->tmpDataInfo->paramPAPF[0] = dataUnit[i];
            CC_PRINTF(1,"PAPF=0x%x\n", tmpInfo->tmpDataInfo->paramPAPF[0]);
            tmpInfo->isPAPF = 0;
            *leftDataUnitSize = *leftDataUnitSize - 1;
            continue;
        }

        if (tmpInfo->isAPS)
        {
            tmpInfo->tmpDataInfo->paramAPS[0] = dataUnit[i];
            tmpInfo->tmpDataInfo->paramAPS[1] = dataUnit[i+1];
            CC_PRINTF(1,"APS=0x%x, 0x%x\n", tmpInfo->tmpDataInfo->paramAPS[0] , tmpInfo->tmpDataInfo->paramAPS[1]);
            tmpInfo->isAPS = 0;
            i++;
            *leftDataUnitSize = *leftDataUnitSize - 2;

            continue;
        }

        /* Get Colour Control parameter */
        if (tmpInfo->isCOL)
        {
            tmpInfo->tmpDataInfo->paramCOL[0] = dataUnit[i];
            CC_PRINTF(1,"COL_0=0x%x\n", tmpInfo->tmpDataInfo->paramCOL[0]);
            if ((tmpInfo->tmpDataInfo->paramCOL[0] & 0xf0) == 0x20)
            {
                tmpInfo->tmpDataInfo->paramCOL[1] = dataUnit[i+1];
                CC_PRINTF(1,"COL_1=0x%x\n", tmpInfo->tmpDataInfo->paramCOL[1]);
                i++;
            }
            tmpInfo->isCOL = 0;
            *leftDataUnitSize = *leftDataUnitSize - 1;
            continue;
        }

        /* Get Character Size Control parameter */
        if (tmpInfo->isSZX)
        {
            tmpInfo->tmpDataInfo->paramSZX[0] = dataUnit[i];
            CC_PRINTF(1,"SZX=0x%x\n", tmpInfo->tmpDataInfo->paramSZX[0]);
            tmpInfo->isSZX = 0;
            *leftDataUnitSize = *leftDataUnitSize - 1;
            continue;
        }

        /* Get Flashing control parameter */
        if (tmpInfo->isFLC)
        {
            tmpInfo->tmpDataInfo->paramFLC[0] = dataUnit[i];
            CC_PRINTF(1,"FLC=0x%x\n", tmpInfo->tmpDataInfo->paramFLC[0]);

            *leftDataUnitSize = *leftDataUnitSize - 1;
            tmpInfo->isFLC = 0;

            if ( (tmpInfo->tmpDataInfo->paramFLC[0] != 0x40)
              && (tmpInfo->tmpDataInfo->paramFLC[0] != 0x4F))
            {
                CC_PRINTF(2,"paramFLC[0] = %x\n", tmpInfo->tmpDataInfo->paramFLC[0]);
                tmpInfo->tmpDataInfo->fFLC = 0;
            }

            continue;
        }

        /* Get Pattern Polarity Control parameter */
        if (tmpInfo->isPOL)
        {
            tmpInfo->tmpDataInfo->paramPOL[0] = dataUnit[i];
            CC_PRINTF(1,"POL=0x%x\n", tmpInfo->tmpDataInfo->paramPOL[0]);

            *leftDataUnitSize = *leftDataUnitSize - 1;
            tmpInfo->isPOL = 0;

            if ( (tmpInfo->tmpDataInfo->paramPOL[0] != 0x40)
              && (tmpInfo->tmpDataInfo->paramPOL[0] != 0x41))
            {
                CC_PRINTF(2,"paramPOL[0] = %x\n", tmpInfo->tmpDataInfo->paramPOL[0]);
                tmpInfo->tmpDataInfo->fPOL = 0;
            }

            continue;
        }

        /* Get HIGHLIGHTING CHARACTER BLOCK parameter */
        if (tmpInfo->isHLC)
        {
            tmpInfo->tmpDataInfo->paramHLC[0] = dataUnit[i];
            CC_PRINTF(1,"HLC=0x%x\n", tmpInfo->tmpDataInfo->paramHLC[0]);
            tmpInfo->isHLC = 0;
            *leftDataUnitSize = *leftDataUnitSize - 1;
            continue;
        }

        /* Get Repeat Character parameter */
        if (tmpInfo->isRPC)
        {
            tmpInfo->tmpDataInfo->paramRPC[0] = dataUnit[i];
            CC_PRINTF(1,"RPC=0x%x\n", tmpInfo->tmpDataInfo->paramRPC[0]);
            tmpInfo->isRPC = 0;
            *leftDataUnitSize = *leftDataUnitSize - 1;
            continue;
        }

        /* Get Time Control parameter */
        if (tmpInfo->isTIME)
        {
            tmpInfo->tmpDataInfo->paramTIME[0] = dataUnit[i];

            tmpInfo->tmpDataInfo->paramTIME[1] = dataUnit[i+1];
            CC_PRINTF(1,"TIME=0x%x, 0x%x\n", tmpInfo->tmpDataInfo->paramTIME[0], tmpInfo->tmpDataInfo->paramTIME[1]);
            tmpInfo->isTIME = 0;
            i++;
            *leftDataUnitSize = *leftDataUnitSize - 2;

            if (tmpInfo->tmpDataInfo->paramTIME[0] != 0x20)
            {
                CC_PRINTF(2,"paramTIME[0] = %x\n", tmpInfo->tmpDataInfo->paramTIME[0]);
                tmpInfo->tmpDataInfo->fTIME = 0;
            }
            continue;
        }

        /* Get Control Sequence Introducer parameter */
        if (tmpInfo->isCSI)
        {
            tmpInfo->paramCSI[tmpInfo->byteNumCSI] = dataUnit[i];
            switch (tmpInfo->paramCSI[tmpInfo->byteNumCSI])
            {
                case 0x53: /* SWF  */
                    CC_GetCSI1(tmpInfo->paramCSI, tmpInfo->tmpDataInfo->paramSWF, &tmpInfo->tmpDataInfo->num1_SWF);
                    CC_PRINTF(1,"SWF: num=%d, 0x%x, 0x%x, 0x%x\n", tmpInfo->tmpDataInfo->num1_SWF, 
                                       tmpInfo->tmpDataInfo->paramSWF[0], tmpInfo->tmpDataInfo->paramSWF[1], tmpInfo->tmpDataInfo->paramSWF[2]);
                    if ( (tmpInfo->tmpDataInfo->paramSWF[0] != 0x37)
                      && (tmpInfo->tmpDataInfo->paramSWF[0] != 0x38)
                      && (tmpInfo->tmpDataInfo->paramSWF[0] != 0x39)
                      && (tmpInfo->tmpDataInfo->paramSWF[0] != 0x3A) )
                    {
                        CC_PRINTF(2,"paramSWF[0] = %x\n", tmpInfo->tmpDataInfo->paramSWF[0]);
                        tmpInfo->tmpDataInfo->fSWF = 0;
                    }
                    tmpInfo->tmpDataInfo->fSWF = 1;
                    break;
                case 0x6E: /* RCS  */
                    CC_GetCSI1(tmpInfo->paramCSI, tmpInfo->tmpDataInfo->paramRCS, &tmpInfo->tmpDataInfo->num1_RCS);
                    CC_PRINTF(1,"RCS: num=%d, 0x%x, 0x%x, 0x%x\n", tmpInfo->tmpDataInfo->num1_RCS, 
                                       tmpInfo->tmpDataInfo->paramRCS[0], tmpInfo->tmpDataInfo->paramRCS[1], tmpInfo->tmpDataInfo->paramRCS[2]);
                    tmpInfo->tmpDataInfo->fRCS = 1;
                    break;
                case 0x61: /* ACPS */
                    CC_GetCSI2(tmpInfo->paramCSI, tmpInfo->tmpDataInfo->paramACPS_H, tmpInfo->tmpDataInfo->paramACPS_V,
                    &tmpInfo->tmpDataInfo->num1_ACPS, &tmpInfo->tmpDataInfo->num2_ACPS);
                    CC_PRINTF(1,"ACPS: num1=%d, num2=%d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", 
                                     tmpInfo->tmpDataInfo->num1_ACPS, tmpInfo->tmpDataInfo->num2_ACPS, 
                                     tmpInfo->tmpDataInfo->paramACPS_H[0], tmpInfo->tmpDataInfo->paramACPS_H[1], tmpInfo->tmpDataInfo->paramACPS_H[2], 
                                    tmpInfo->tmpDataInfo->paramACPS_V[0], tmpInfo->tmpDataInfo->paramACPS_V[1], tmpInfo->tmpDataInfo->paramACPS_V[2]);
                    tmpInfo->tmpDataInfo->fACPS = 1;
                    break;
                case 0x56: /* SDF  */
                    CC_GetCSI2(tmpInfo->paramCSI,  tmpInfo->tmpDataInfo->paramSDF_H,  tmpInfo->tmpDataInfo->paramSDF_V,
                    &tmpInfo->tmpDataInfo->num1_SDF, &tmpInfo->tmpDataInfo->num2_SDF);
                    CC_PRINTF(1,"SDF: num1=%d, num2=%d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", 
                                      tmpInfo->tmpDataInfo->num1_SDF, tmpInfo->tmpDataInfo->num2_SDF, 
                                      tmpInfo->tmpDataInfo->paramSDF_H[0], tmpInfo->tmpDataInfo->paramSDF_H[1], tmpInfo->tmpDataInfo->paramSDF_H[2], 
                                      tmpInfo->tmpDataInfo->paramSDF_V[0], tmpInfo->tmpDataInfo->paramSDF_V[1], tmpInfo->tmpDataInfo->paramSDF_V[2]);
                    tmpInfo->tmpDataInfo->fSDF = 1;
                    break;
                case 0x5F: /* SDP  */
                    CC_GetCSI2(tmpInfo->paramCSI,  tmpInfo->tmpDataInfo->paramSDP_H,  tmpInfo->tmpDataInfo->paramSDP_V,
                    &tmpInfo->tmpDataInfo->num1_SDP, &tmpInfo->tmpDataInfo->num2_SDP);
                    CC_PRINTF(1,"SDP: num1=%d, num2=%d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", 
                                       tmpInfo->tmpDataInfo->num1_SDP, tmpInfo->tmpDataInfo->num2_SDP, 
                                       tmpInfo->tmpDataInfo->paramSDP_H[0], tmpInfo->tmpDataInfo->paramSDP_H[1], tmpInfo->tmpDataInfo->paramSDP_H[2], 
                                       tmpInfo->tmpDataInfo->paramSDP_V[0], tmpInfo->tmpDataInfo->paramSDP_V[1], tmpInfo->tmpDataInfo->paramSDP_V[2]);
                    tmpInfo->tmpDataInfo->fSDP = 1;
                    break;
                case 0x57: /* SSM  */
                    CC_GetCSI2(tmpInfo->paramCSI,  tmpInfo->tmpDataInfo->paramSSM_H,  tmpInfo->tmpDataInfo->paramSSM_V,
                    &tmpInfo->tmpDataInfo->num1_SSM, &tmpInfo->tmpDataInfo->num2_SSM);
                    CC_PRINTF(1,"SSM: num1=%d, num2=%d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", 
                                      tmpInfo->tmpDataInfo->num1_SSM, tmpInfo->tmpDataInfo->num2_SSM, 
                                      tmpInfo->tmpDataInfo->paramSSM_H[0], tmpInfo->tmpDataInfo->paramSSM_H[1], tmpInfo->tmpDataInfo->paramSSM_H[2], 
                                      tmpInfo->tmpDataInfo->paramSSM_V[0], tmpInfo->tmpDataInfo->paramSSM_V[1], tmpInfo->tmpDataInfo->paramSSM_V[2]);
                    tmpInfo->tmpDataInfo->fSSM = 1;
                    break;
                case 0x58: /* SHS  */
                    CC_GetCSI1(tmpInfo->paramCSI,  tmpInfo->tmpDataInfo->paramSHS, &tmpInfo->tmpDataInfo->num1_SHS);
                    CC_PRINTF(1,"SHS: num=%d, 0x%x, 0x%x, 0x%x\n", tmpInfo->tmpDataInfo->num1_SHS, 
                                     tmpInfo->tmpDataInfo->paramSHS[0], tmpInfo->tmpDataInfo->paramSHS[1], tmpInfo->tmpDataInfo->paramSHS[2]);
                    tmpInfo->tmpDataInfo->fSHS = 1;
                    break;
                case 0x59: /* SVS  */
                    CC_GetCSI1(tmpInfo->paramCSI,  tmpInfo->tmpDataInfo->paramSVS, &tmpInfo->tmpDataInfo->num1_SVS);
                    CC_PRINTF(1,"SVS: num=%d, 0x%x, 0x%x, 0x%x\n", tmpInfo->tmpDataInfo->num1_SVS, 
                                     tmpInfo->tmpDataInfo->paramSVS[0], tmpInfo->tmpDataInfo->paramSVS[1], tmpInfo->tmpDataInfo->paramSVS[2]);
                    tmpInfo->tmpDataInfo->fSVS = 1;
                    break;
                case 0x63: /* ORN  */
                    CC_GetCSI2(tmpInfo->paramCSI,  tmpInfo->tmpDataInfo->paramORN,  tmpInfo->tmpDataInfo->paramORN_C,
                    &tmpInfo->tmpDataInfo->num1_ORN, &tmpInfo->tmpDataInfo->num2_ORN);
                    CC_PRINTF(1,"ORN: num1=%d, num2=%d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", 
                                      tmpInfo->tmpDataInfo->num1_ORN, tmpInfo->tmpDataInfo->num2_ORN, 
                                      tmpInfo->tmpDataInfo->paramORN[0], 
                                      tmpInfo->tmpDataInfo->paramORN_C[0], tmpInfo->tmpDataInfo->paramORN_C[1], tmpInfo->tmpDataInfo->paramORN_C[2], 
                                      tmpInfo->tmpDataInfo->paramORN_C[3], tmpInfo->tmpDataInfo->paramORN_C[4]);
                    tmpInfo->tmpDataInfo->fORN = 1;
                    break;
                case 0x68: /* PRA  */
                    CC_GetCSI1(tmpInfo->paramCSI,  tmpInfo->tmpDataInfo->paramPRA, &tmpInfo->tmpDataInfo->num1_PRA);
                    CC_PRINTF(1,"PRA: num=%d, 0x%x, 0x%x, 0x%x\n", tmpInfo->tmpDataInfo->num1_PRA, 
                                     tmpInfo->tmpDataInfo->paramPRA[0], tmpInfo->tmpDataInfo->paramPRA[1], tmpInfo->tmpDataInfo->paramPRA[2]);
                    tmpInfo->tmpDataInfo->fPRA = 1;
                    break;
                case 0x6F: /* SCR?  */
                    break;
                default:
                    tmpInfo->byteNumCSI ++;
                    break;
            }

            *leftDataUnitSize = *leftDataUnitSize - 1;
            continue;
        }

        switch ( (mt_u8)(dataUnit[i]))
        {

            /******************/
            /*   Not Accept   */
            /******************/
            case 0x00:                                                                                                        break;   /* NUL     */
            case 0x01:                                                                                                        break;   /* NOT USE */
            case 0x02:                                                                                                        break;   /* NOT USE */
            case 0x03:                                                                                                        break;   /* NOT USE */
            case 0x04:                                                                                                        break;   /* NOT USE */
            case 0x05:                                                                                                        break;   /* NOT USE */
            case 0x06:                                                                                                        break;   /* NOT USE */
            case 0x10:                                                                                                        break;   /* NOT USE */
            case 0x11:                                                                                                        break;   /* NOT USE */
            case 0x12:                                                                                                        break;   /* NOT USE */
            case 0x13:                                                                                                        break;   /* NOT USE */
            case 0x14:                                                                                                        break;   /* NOT USE */
            case 0x15:                                                                                                        break;   /* NOT USE */
            case 0x17:                                                                                                        break;   /* NOT USE */
            case 0x1a:                                                                                                        break;   /* NOT USE */


            case 0x07:                                                                                                        break;   /*   BEL   */
            case 0x18:                                                                                                        break;   /*   CAN   */
            case 0x1e:                                                                                                        break;   /*   RS    */


            case 0x7f:                                                                                                        break;   /*   DEL   */
            case 0x0e:                                                                                                        break;   /*   LS1   */
            case 0x0f:                                                                                                        break;   /*   LS0   */
            case 0x19:                                                                                                        break;   /*   SS2   */
            case 0x1f:                                                                                                        break;   /*   US    */
            case 0x1b:                                                                                                        break;   /*   ESC   */

            /******************/
            /*  Control Code  */
            /******************/
            /*  Active position  */
            case 0x08: CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fAPB, NULL, NULL, 0);
                       if (!tmpInfo->fNewControlCode) {
                           tmpInfo->tmpDataInfo->APB_num ++;
                       }                                                                                                      break;    /* APB */
            case 0x09: CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fAPF, NULL, NULL, 0);
                       if (!tmpInfo->fNewControlCode) {
                           tmpInfo->tmpDataInfo->APF_num ++;
                       }                                                                                                      break;    /* APF */
            case 0x0a: CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fAPD, NULL, NULL, 0);
                       if (!tmpInfo->fNewControlCode) {
                           tmpInfo->tmpDataInfo->APD_num ++;
                       }                                                                                                      break;    /* APD */
            case 0x0b: CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fAPU, NULL, NULL, 0);
                       if (!tmpInfo->fNewControlCode) {
                           tmpInfo->tmpDataInfo->APU_num ++;
                       }                                                                                                      break;    /* APU */
            case 0x0d: CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fAPR, NULL, NULL, 0);
                       if (!tmpInfo->fNewControlCode) {
                           tmpInfo->tmpDataInfo->APR_num ++;
                       }                                                                                                      break;    /* APR */
            case 0x16: CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fPAPF, &tmpInfo->isPAPF, NULL, 0);                         break;    /* PAPF */
            case 0x1c: CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fAPS, &tmpInfo->isAPS, NULL, 0);                           break;    /* APS */

            /* Colour */
            case 0x80: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fBKF, NULL, tmpInfo->tmpDataInfo->charColor, 0x80);           /* BKF */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xc0);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fBKF, NULL, tmpInfo->tmpDataInfo->charColor, 0x80);           /* BKF */
                                }
				}
				break; 
            case 0x81: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fRDF, NULL, tmpInfo->tmpDataInfo->charColor, 0x81);           /* RDF */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xc1);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fRDF, NULL, tmpInfo->tmpDataInfo->charColor, 0x81);           /* RDF */
                                }
				}
				break;
            case 0x82: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fGRF, NULL, tmpInfo->tmpDataInfo->charColor, 0x82);           /* GRF */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xc2);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fGRF, NULL, tmpInfo->tmpDataInfo->charColor, 0x82);           /* GRF */
                                }
				}
				break; 
            case 0x83: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fYLF, NULL, tmpInfo->tmpDataInfo->charColor, 0x83);            /* YLF */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xc3);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fYLF, NULL, tmpInfo->tmpDataInfo->charColor, 0x83);            /* YLF */
                                }
				}
				break; 
            case 0x84: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fBLF, NULL, tmpInfo->tmpDataInfo->charColor, 0x84);            /* BLF */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xc4);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fBLF, NULL, tmpInfo->tmpDataInfo->charColor, 0x84);            /* BLF */
                                }
				}
				break; 
            case 0x85: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fMGF, NULL, tmpInfo->tmpDataInfo->charColor, 0x85);            /* MGF */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xc5);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fMGF, NULL, tmpInfo->tmpDataInfo->charColor, 0x85);            /* MGF */
                                }
				}
				break; 
            case 0x86: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fCNF, NULL, tmpInfo->tmpDataInfo->charColor, 0x86);            /* CNF */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xc6);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fCNF, NULL, tmpInfo->tmpDataInfo->charColor, 0x86);            /* CNF */
                                }
				}
				break; 
            case 0x87: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fWHF, NULL, tmpInfo->tmpDataInfo->charColor, 0x87);            /* WHF */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xc7);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fWHF, NULL, tmpInfo->tmpDataInfo->charColor, 0x87);            /* WHF */
                                }
				}
				break; 
            case 0x90: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fCOL, &tmpInfo->isCOL, NULL, 0);                             /* COL */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xd0);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fCOL, &tmpInfo->isCOL, NULL, 0);                             /* COL */
                                }
				}
				break; 

            /* Character Size */
            case 0x88: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fSSZ, NULL, tmpInfo->tmpDataInfo->charSize, 0x88);            /* SSZ */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xc8);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fSSZ, NULL, tmpInfo->tmpDataInfo->charSize, 0x88);            /* SSZ */
                                }
				}
				break; 
            case 0x89: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fMSZ, NULL, tmpInfo->tmpDataInfo->charSize, 0x89);            /* MSZ */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xc9);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fMSZ, NULL, tmpInfo->tmpDataInfo->charSize, 0x89);            /* MSZ */
                                }
				}
				break; 
            case 0x8a: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fNSZ, NULL, tmpInfo->tmpDataInfo->charSize, 0x8a);             /* NSZ */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xca);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fNSZ, NULL, tmpInfo->tmpDataInfo->charSize, 0x8a);             /* NSZ */
                                }
				}
				break; 
            case 0x8b: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fSZX, &tmpInfo->isSZX, NULL, 0);                              /* SZX */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xcb);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fSZX, &tmpInfo->isSZX, NULL, 0);                              /* SZX */
                                }
				}
				break; 

            case 0x8c:                                                                                                        
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xcc);
                                    UTF = 0;              					
				    }
				}
				break; 

            case 0x8d:                                                                                                        
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xcd);
                                    UTF = 0;              					
				    }
				}
				break; 

            case 0x8e:                                                                                                        
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xce);
                                    UTF = 0;              					
				    }
				}
				break; 

            case 0x8f:                                                                                                        
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xcf);
                                    UTF = 0;              					
				    }
				}
				break; 

            /* Flashing */
            case 0x91: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fFLC, &tmpInfo->isFLC, NULL, 0);                               /* FLC */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xd1);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fFLC, &tmpInfo->isFLC, NULL, 0);                               /* FLC */
                                }
				}
				break; 

            case 0x92:                                                                                                        /*   CDC   */
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xd2);
                                    UTF = 0;              					
				    }
				}
				break; 

            /* Pattern Polarity */
            case 0x93: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fPOL, &tmpInfo->isPOL, NULL, 0);                               /* POL */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xd3);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fPOL, &tmpInfo->isPOL, NULL, 0);                               /* POL */
                                }
				}
				break; 

            case 0x94:                                                                                                        /*   WMM   */
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xd4);
                                    UTF = 0;              					
				    }
				}
				break; 

            case 0x95:                                                                                                        /*  MACRO  */
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xd5);
                                    UTF = 0;              					
				    }
				}
				break; 

            case 0x96:                                                                                                        
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xd6);
                                    UTF = 0;              					
				    }
				}
				break; 

            /* HIGHLIGHTING CHARACTER BLOCK */
            case 0x97: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fHLC, &tmpInfo->isHLC, NULL, 0);                               /* HLC */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xd7);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fHLC, &tmpInfo->isHLC, NULL, 0);                               /* HLC */
                                }
				}
				break; 

            /* Repeat Character */
            case 0x98: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fRPC, &tmpInfo->isRPC, NULL, 0);                               /* RPC */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xd8);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fRPC, &tmpInfo->isRPC, NULL, 0);                               /* RPC */
                                }
				}
				break; 

            /* Stop Lining */
            case 0x99: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fSPL, NULL, NULL, 0);                                         /* SPL */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xd9);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fSPL, NULL, NULL, 0);                                         /* SPL */
                                }
				}
				break; 

            /* Start Lining */
            case 0x9a: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fSTL, NULL, NULL, 0);                                          /* STL */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xda);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fSTL, NULL, NULL, 0);                                          /* STL */
                                }
				}
				break; 

            /* Control Sequence Introducer */
            case 0x9b: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fCSI, &tmpInfo->isCSI, NULL, 0);                               /* CSI */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xdb);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fCSI, &tmpInfo->isCSI, NULL, 0);                               /* CSI */
                                }
				}
				break; 

            case 0x9c:                                                                                                        
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xdc);
                                    UTF = 0;              					
				    }
				}
				break; 

            /* Time */
            case 0x9d: 
				if(TCS == 0)
				{
                                CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fTIME, &tmpInfo->isTIME, NULL, 0);                            /* TIME */
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xdd);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fTIME, &tmpInfo->isTIME, NULL, 0);                            /* TIME */
                                }
				}
				break; 

            case 0x9e:                                                                                                        
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xde);
                                    UTF = 0;              					
				    }
				}
				break; 

            case 0x9f:                                                                                                       
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xdf);
                                    UTF = 0;              					
				    }
				}
				break; 

            /* Clear screen */
            case 0x0c: CC_ControlCodeChange(&tmpInfo->tmpDataInfo->fCS, NULL, NULL, 0);                                       break;    /* CS  */

            /* Single shift 3 */
            case 0x1d: tmpInfo->isSpecial = 1;                                                                                break;    /* SS3 */

           /******************/
            /* Character Code */
            /******************/
            case 0x20: CC_SetDrawChar(0x20);                                                                               break;    /* SP  */
            case 0x21: CC_SetDrawChar(0x21);                                                                               break;
            case 0x22: CC_SetDrawChar(0x22);                                                                               break;
            case 0x23: CC_SetDrawChar(0x23);                                                                               break;
            case 0x24: CC_SetDrawChar(0x24);                                                                               break;
            case 0x25: CC_SetDrawChar(0x25);                                                                               break;
            case 0x26: CC_SetDrawChar(0x26);                                                                               break;
            case 0x27: CC_SetDrawChar(0x27);                                                                               break;
            case 0x28: CC_SetDrawChar(0x28);                                                                               break;
            case 0x29: CC_SetDrawChar(0x29);                                                                               break;
            case 0x2a: CC_SetDrawChar(0x2a);                                                                               break;
            case 0x2b: CC_SetDrawChar(0x2b);                                                                               break;
            case 0x2c: CC_SetDrawChar(0x2c);                                                                               break;
            case 0x2d: CC_SetDrawChar(0x2d);                                                                               break;
            case 0x2e: CC_SetDrawChar(0x2e);                                                                               break;
            case 0x2f: CC_SetDrawChar(0x2f);                                                                               break;
            case 0x30: CC_SetDrawChar(0x30);                                                                               break;
            case 0x31: CC_SetDrawChar(0x31);                                                                               break;
            case 0x32: CC_SetDrawChar(0x32);                                                                               break;
            case 0x33: CC_SetDrawChar(0x33);                                                                               break;
            case 0x34: CC_SetDrawChar(0x34);                                                                               break;
            case 0x35: CC_SetDrawChar(0x35);                                                                               break;
            case 0x36: CC_SetDrawChar(0x36);                                                                               break;
            case 0x37: CC_SetDrawChar(0x37);                                                                               break;
            case 0x38: CC_SetDrawChar(0x38);                                                                               break;
            case 0x39: CC_SetDrawChar(0x39);                                                                               break;
            case 0x3a: CC_SetDrawChar(0x3a);                                                                               break;
            case 0x3b: CC_SetDrawChar(0x3b);                                                                               break;
            case 0x3c: CC_SetDrawChar(0x3c);                                                                               break;
            case 0x3d: CC_SetDrawChar(0x3d);                                                                               break;
            case 0x3e: CC_SetDrawChar(0x3e);                                                                               break;
            case 0x3f: CC_SetDrawChar(0x3f);                                                                               break;
            case 0x40: CC_SetDrawChar(0x40);                                                                               break;
            case 0x41: CC_SetDrawChar(0x41);                                                                               break;
            case 0x42: CC_SetDrawChar(0x42);                                                                               break;
            case 0x43: CC_SetDrawChar(0x43);                                                                               break;
            case 0x44: CC_SetDrawChar(0x44);                                                                               break;
            case 0x45: CC_SetDrawChar(0x45);                                                                               break;
            case 0x46: CC_SetDrawChar(0x46);                                                                               break;
            case 0x47: CC_SetDrawChar(0x47);                                                                               break;
            case 0x48: CC_SetDrawChar(0x48);                                                                               break;
            case 0x49: CC_SetDrawChar(0x49);                                                                               break;
            case 0x4a: CC_SetDrawChar(0x4a);                                                                               break;
            case 0x4b: CC_SetDrawChar(0x4b);                                                                               break;
            case 0x4c: CC_SetDrawChar(0x4c);                                                                               break;
            case 0x4d: CC_SetDrawChar(0x4d);                                                                               break;
            case 0x4e: CC_SetDrawChar(0x4e);                                                                               break;
            case 0x4f: CC_SetDrawChar(0x4f);                                                                               break;
            case 0x50: CC_SetDrawChar(0x50);                                                                               break;
            case 0x51: CC_SetDrawChar(0x51);                                                                               break;
            case 0x52: CC_SetDrawChar(0x52);                                                                               break;
            case 0x53: CC_SetDrawChar(0x53);                                                                               break;
            case 0x54: CC_SetDrawChar(0x54);                                                                               break;
            case 0x55: CC_SetDrawChar(0x55);                                                                               break;
            case 0x56: CC_SetDrawChar(0x56);                                                                               break;
            case 0x57: CC_SetDrawChar(0x57);                                                                               break;
            case 0x58: CC_SetDrawChar(0x58);                                                                               break;
            case 0x59: CC_SetDrawChar(0x59);                                                                               break;
            case 0x5a: CC_SetDrawChar(0x5a);                                                                               break;
            case 0x5b: CC_SetDrawChar(0x5b);                                                                               break;
            case 0x5c: CC_SetDrawChar(0x5c);                                                                               break;
            case 0x5d: CC_SetDrawChar(0x5d);                                                                               break;
            case 0x5e: CC_SetDrawChar(0x5e);                                                                               break;
            case 0x5f: CC_SetDrawChar(0x5f);                                                                               break;
            case 0x60: CC_SetDrawChar(0x60);                                                                               break;
            case 0x61: CC_SetDrawChar(0x61);                                                                               break;
            case 0x62: CC_SetDrawChar(0x62);                                                                               break;
            case 0x63: CC_SetDrawChar(0x63);                                                                               break;
            case 0x64: CC_SetDrawChar(0x64);                                                                               break;
            case 0x65: CC_SetDrawChar(0x65);                                                                               break;
            case 0x66: CC_SetDrawChar(0x66);                                                                               break;
            case 0x67: CC_SetDrawChar(0x67);                                                                               break;
            case 0x68: CC_SetDrawChar(0x68);                                                                               break;
            case 0x69: CC_SetDrawChar(0x69);                                                                               break;
            case 0x6a: CC_SetDrawChar(0x6a);                                                                               break;
            case 0x6b: CC_SetDrawChar(0x6b);                                                                               break;
            case 0x6c: CC_SetDrawChar(0x6c);                                                                               break;
            case 0x6d: CC_SetDrawChar(0x6d);                                                                               break;
            case 0x6e: CC_SetDrawChar(0x6e);                                                                               break;
            case 0x6f: CC_SetDrawChar(0x6f);                                                                               break;
            case 0x70: CC_SetDrawChar(0x70);                                                                               break;
            case 0x71: CC_SetDrawChar(0x71);                                                                               break;
            case 0x72: CC_SetDrawChar(0x72);                                                                               break;
            case 0x73: CC_SetDrawChar(0x73);                                                                               break;
            case 0x74: CC_SetDrawChar(0x74);                                                                               break;
            case 0x75: CC_SetDrawChar(0x75);                                                                               break;
            case 0x76: CC_SetDrawChar(0x76);                                                                               break;
            case 0x77: CC_SetDrawChar(0x77);                                                                               break;
            case 0x78: CC_SetDrawChar(0x78);                                                                               break;
            case 0x79: CC_SetDrawChar(0x79);                                                                               break;
            case 0x7a: CC_SetDrawChar(0x7a);                                                                               break;
            case 0x7b: CC_SetDrawChar(0x7b);                                                                               break;
            case 0x7c: CC_SetDrawChar(0x7c);                                                                               break;
            case 0x7d: CC_SetDrawChar(0x7d);                                                                               break;
            case 0x7e: CC_SetDrawChar(0x7e);                                                                               break;
            case 0xa0:                                                                                                        
				if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xe0);
                                    UTF = 0;              					
				    }
				}
				break; 

            case 0xa1: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xa1);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xe1);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xa1);   
                                }
				}
				break; 

            case 0xa2: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xa2);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xe2);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xa2);   
                                }
				}
				break; 
            case 0xa3: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xa3);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xe3);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xa3);   
                                }
				}
				break; 
            case 0xa4: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xa4);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xe4);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xa4);   
                                }
				}
				break; 
            case 0xa5: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xa5);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xe5);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xa5);   
                                }
				}
				break; 
            case 0xa6: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xa6);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xe6);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xa6);   
                                }
				}
				break; 
            case 0xa7: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xa7);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xe7);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xa7);   
                                }
				}
				break; 
            case 0xa8: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xa8);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xe8);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xa8);   
                                }
				}
				break; 
            case 0xa9: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xa9);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xe9);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xa9);   
                                }
				}
				break; 
            case 0xaa: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xaa);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xea);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xaa);   
                                }
				}
				break; 
            case 0xab: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xab);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xeb);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xab);   
                                }
				}
				break; 
            case 0xac: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xac);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xec);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xac);   
                                }
				}
				break; 
            case 0xad: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xad);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xed);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xad);   
                                }
				}
				break; 
            case 0xae: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xae);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xee);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xae);   
                                }
				}
				break; 
            case 0xaf: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xaf);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xef);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xaf);   
                                }
				}
				break; 
            case 0xb0: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xb0);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xf0);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xb0);   
                                }
				}
				break; 
            case 0xb1: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xb1);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xf1);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xb1);   
                                }
				}
				break; 
            case 0xb2: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xb2);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xf2);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xb2);   
                                }
				}
				break; 
            case 0xb3: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xb3);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xf3);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xb3);   
                                }
				}
				break; 
            case 0xb4: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xb4);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xf4);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xb4);   
                                }
				}
				break; 
            case 0xb5: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xb5);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xf5);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xb5);   
                                }
				}
				break; 
            case 0xb6: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xb6);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xf6);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xb6);   
                                }
				}
				break; 
            case 0xb7: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xb7);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xf7);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xb7);   
                                }
				}
				break; 
            case 0xb8: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xb8);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xf8);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xb8);   
                                }
				}
				break; 
            case 0xb9: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xb9);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xf9);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xb9);   
                                }
				}
				break; 
            case 0xba: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xba);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xfa);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xba);   
                                }
				}
				break; 
            case 0xbb: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xbb);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xfb);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xbb);   
                                }
				}
				break; 
            case 0xbc:
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xbc);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xfc);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xbc);   
                                }
				}
				break; 
            case 0xbd: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xbd);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xfd);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xbd);   
                                }
				}
				break; 
            case 0xbe: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xbe);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xfe);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xbe);   
                                }
				}
				break; 
            case 0xbf: 
				if(TCS == 0)
				{
                                CC_SetDrawChar(0xbf);                            
				}
				else if(TCS == 1)
				{
				    if(UTF == 2) 
				    {
                                    CC_SetDrawChar(0xff);
                                    UTF = 0;              					
				    }
                                else
                                {
                                    CC_SetDrawChar(0xbf);   
                                }
				}
				break; 

            case 0xc0: CC_SetDrawChar(0xc0);                                                                               break;
            case 0xc1: CC_SetDrawChar(0xc1);                                                                               break;
            case 0xc2:                                                                                
                if(TCS == 0)
                {
                    CC_SetDrawChar(0xc2);
                }
                else if(TCS == 1)
                {
                    UTF = 1;
                }
                break;
				
            case 0xc3:
                if(TCS == 0)
                {
                    CC_SetDrawChar(0xc3);
                }
                else if(TCS == 1)
                {
                    UTF = 2;
                }
                break;

            case 0xc4: CC_SetDrawChar(0xc4);                                                                               break;
            case 0xc5: CC_SetDrawChar(0xc5);                                                                               break;
            case 0xc6: CC_SetDrawChar(0xc6);                                                                               break;
            case 0xc7: CC_SetDrawChar(0xc7);                                                                               break;
            case 0xc8: CC_SetDrawChar(0xc8);                                                                               break;
            case 0xc9: CC_SetDrawChar(0xc9);                                                                               break;
            case 0xca: CC_SetDrawChar(0xca);                                                                               break;
            case 0xcb: CC_SetDrawChar(0xcb);                                                                               break;
            case 0xcc: CC_SetDrawChar(0xcc);                                                                               break;
            case 0xcd: CC_SetDrawChar(0xcd);                                                                               break;
            case 0xce: CC_SetDrawChar(0xce);                                                                               break;
            case 0xcf: CC_SetDrawChar(0xcf);                                                                               break;
            case 0xd0: CC_SetDrawChar(0xd0);                                                                               break;
            case 0xd1: CC_SetDrawChar(0xd1);                                                                               break;
            case 0xd2: CC_SetDrawChar(0xd2);                                                                               break;
            case 0xd3: CC_SetDrawChar(0xd3);                                                                               break;
            case 0xd4: CC_SetDrawChar(0xd4);                                                                               break;
            case 0xd5: CC_SetDrawChar(0xd5);                                                                               break;
            case 0xd6: CC_SetDrawChar(0xd6);                                                                               break;
            case 0xd7: CC_SetDrawChar(0xd7);                                                                               break;
            case 0xd8: CC_SetDrawChar(0xd8);                                                                               break;
            case 0xd9: CC_SetDrawChar(0xd9);                                                                               break;
            case 0xda: CC_SetDrawChar(0xda);                                                                               break;
            case 0xdb: CC_SetDrawChar(0xdb);                                                                               break;
            case 0xdc: CC_SetDrawChar(0xdc);                                                                               break;
            case 0xdd: CC_SetDrawChar(0xdd);                                                                               break;
            case 0xde: CC_SetDrawChar(0xde);                                                                               break;
            case 0xdf: CC_SetDrawChar(0xdf);                                                                               break;
            case 0xe0: CC_SetDrawChar(0xe0);                                                                               break;
            case 0xe1: CC_SetDrawChar(0xe1);                                                                               break;
            case 0xe2: CC_SetDrawChar(0xe2);                                                                               break;
            case 0xe3: CC_SetDrawChar(0xe3);                                                                               break;
            case 0xe4: CC_SetDrawChar(0xe4);                                                                               break;
            case 0xe5: CC_SetDrawChar(0xe5);                                                                               break;
            case 0xe6: CC_SetDrawChar(0xe6);                                                                               break;
            case 0xe7: CC_SetDrawChar(0xe7);                                                                               break;
            case 0xe8: CC_SetDrawChar(0xe8);                                                                               break;
            case 0xe9: CC_SetDrawChar(0xe9);                                                                               break;
            case 0xea: CC_SetDrawChar(0xea);                                                                               break;
            case 0xeb: CC_SetDrawChar(0xeb);                                                                               break;
            case 0xec: CC_SetDrawChar(0xec);                                                                               break;
            case 0xed: CC_SetDrawChar(0xed);                                                                               break;
            case 0xee: CC_SetDrawChar(0xee);                                                                               break;
            case 0xef: CC_SetDrawChar(0xef);                                                                               break;
            case 0xf0: CC_SetDrawChar(0xf0);                                                                               break;
            case 0xf1: CC_SetDrawChar(0xf1);                                                                               break;
            case 0xf2: CC_SetDrawChar(0xf2);                                                                               break;
            case 0xf3: CC_SetDrawChar(0xf3);                                                                               break;
            case 0xf4: CC_SetDrawChar(0xf4);                                                                               break;
            case 0xf5: CC_SetDrawChar(0xf5);                                                                               break;
            case 0xf6: CC_SetDrawChar(0xf6);                                                                               break;
            case 0xf7: CC_SetDrawChar(0xf7);                                                                               break;
            case 0xf8: CC_SetDrawChar(0xf8);                                                                               break;
            case 0xf9: CC_SetDrawChar(0xf9);                                                                               break;
            case 0xfa: CC_SetDrawChar(0xfa);                                                                               break;
            case 0xfb: CC_SetDrawChar(0xfb);                                                                               break;
            case 0xfc: CC_SetDrawChar(0xfc);                                                                               break;
            case 0xfd: CC_SetDrawChar(0xfe);                                                                               break;
            case 0xfe: CC_SetDrawChar(0xfe);                                                                               break;
            case 0xff: CC_SetDrawChar(0xff);                                                                               break;

            default :                                                                                                      break;
        }

        if (!tmpInfo->fNewControlCode)
        {
            *leftDataUnitSize = *leftDataUnitSize - 1;
        }

    }

    return MT_SUCCESS;
}

static void CLOSEDCP_DrawPrepare (sCcContext *ccContext, CLOSEDCP_DrawCharInfo *drawCharInfo)
{

    if (!ccContext->isSetParam) {
        drawCharInfo->color1     = CC_COLOR_BLACK;
        drawCharInfo->color2     = CC_COLOR_BLACK;
//        drawCharInfo->osdWidth   = ccContext->osdLayer.params.outputWidth;
//        drawCharInfo->osdHeight  = ccContext->osdLayer.params.outputHeight;
//        drawCharInfo->drawWidth  = ccContext->osdLayer.params.outputWidth;
//        drawCharInfo->drawHeight = ccContext->osdLayer.params.outputHeight;
        drawCharInfo->fontSize   = 36;
        drawCharInfo->xPos       = 0;
        drawCharInfo->yPos       = drawCharInfo->fontSize;
        drawCharInfo->interval_x = 2;
        drawCharInfo->interval_y = 16;
        drawCharInfo->block_x = (((drawCharInfo->interval_x*2)/2) + drawCharInfo->fontSize)/2;
        drawCharInfo->block_y = ((drawCharInfo->interval_y*2)/2) + drawCharInfo->fontSize;
        drawCharInfo->xPosBase = drawCharInfo->xPos;
        drawCharInfo->yPosBase = drawCharInfo->yPos;
    }

    /* Colour */
    if (drawCharInfo->dataInfo.fBKF) {
        drawCharInfo->color1 = CC_COLOR_BLACK;
    }
    if (drawCharInfo->dataInfo.fRDF) {
        drawCharInfo->color1 = CC_COLOR_RED;
    }
    if (drawCharInfo->dataInfo.fGRF) {
        drawCharInfo->color1 = CC_COLOR_GREEN;
    }
    if (drawCharInfo->dataInfo.fYLF) {
        drawCharInfo->color1 = CC_COLOR_YELLOW;
    }
    if (drawCharInfo->dataInfo.fBLF) {
        drawCharInfo->color1 = CC_COLOR_BLUE;
    }
    if (drawCharInfo->dataInfo.fMGF) {
        drawCharInfo->color1 = CC_COLOR_MAGENTA;
    }
    if (drawCharInfo->dataInfo.fCNF) {
        drawCharInfo->color1 = CC_COLOR_CYAN;
    }
    if (drawCharInfo->dataInfo.fWHF) {
        drawCharInfo->color1 = CC_COLOR_WHITE;
    }
    if (drawCharInfo->dataInfo.fCOL) {
        if ((drawCharInfo->dataInfo.paramCOL[0] & 0xF0) == 0x50) {
            switch (drawCharInfo->dataInfo.paramCOL[0] & 0x0F) {
                case 0x00:
                    drawCharInfo->color2 = CC_COLOR_BLACK;
                    break;
                case 0x01:
                    drawCharInfo->color2 = CC_COLOR_RED;
                    break;
                case 0x02:
                    drawCharInfo->color2 = CC_COLOR_GREEN;
                    break;
                case 0x03:
                    drawCharInfo->color2 = CC_COLOR_YELLOW;
                    break;
                case 0x04:
                    drawCharInfo->color2 = CC_COLOR_BLUE;
                    break;
                case 0x05:
                    drawCharInfo->color2 = CC_COLOR_MAGENTA;
                    break;
                case 0x06:
                    drawCharInfo->color2 = CC_COLOR_CYAN;
                    break;
                case 0x07:
                    drawCharInfo->color2 = CC_COLOR_WHITE;
                    break;
                case 0x08:
                    drawCharInfo->color2 = CC_COLOR_TRANSPARENT;
                    break;
                default:
                    break;
            }
        }
        else if ((drawCharInfo->dataInfo.paramCOL[0] & 0xF0) == 0x20) {
            /* TODO */
        }
        else {
            /* TODO */
        }
    }

    /* Flashing */
    if (drawCharInfo->dataInfo.fFLC)
    {
        /* TODO */
    }

    /* Pattern Polarity */
    if (drawCharInfo->dataInfo.fPOL)
    {
        /* TODO */
    }

    /* HIGHLIGHTING CHARACTER BLOCK */
    if (drawCharInfo->dataInfo.fHLC)
    {
        /* TODO */
    }

    /* Repeat Character */
    if (drawCharInfo->dataInfo.fRPC)
    {
        /* TODO */
    }


    /* Lining */
    if (drawCharInfo->dataInfo.fSPL)
    {
        /* TODO */
    }
    if (drawCharInfo->dataInfo.fSTL)
    {
        /* TODO */
    }


    /* Time */
    if (drawCharInfo->dataInfo.fRPC)
    {
        /* TODO */
    }


    /* Clear screen */
    if (drawCharInfo->dataInfo.fCS)
    {
        drawCharInfo->isCS = 1;
        /* TODO */
    }

    /* Control Sequence Introducer */
    if (drawCharInfo->dataInfo.fCSI)
    {
        if (drawCharInfo->dataInfo.fSWF)
        {
            switch (drawCharInfo->dataInfo.paramSWF[0] & 0x0F)
            {
                case 0x07:
                    drawCharInfo->osdWidth  = 960;
                    drawCharInfo->osdHeight = 540;
                    break;
                case 0x08:
                    /* TODO */
                    break;
                case 0x09:
                    drawCharInfo->osdWidth  = 720;
                    drawCharInfo->osdHeight = 480;
                    break;
                case 0x0A:
                    /* TODO */
                    break;
                default :
                    break;
            }
        }
        if (drawCharInfo->dataInfo.fSDP)
        {
            if (drawCharInfo->dataInfo.num1_SDP == 3)
            {
                drawCharInfo->xPos =
                 (((drawCharInfo->dataInfo.paramSDP_H[0] & 0x0F)*100)
                + ((drawCharInfo->dataInfo.paramSDP_H[1] & 0x0F)*10))
                | (drawCharInfo->dataInfo.paramSDP_H[2] & 0x0F);
            }
            else if (drawCharInfo->dataInfo.num1_SDP == 2)
            {
                drawCharInfo->xPos =
                 ((drawCharInfo->dataInfo.paramSDP_H[0] & 0x0F)*10)
                + (drawCharInfo->dataInfo.paramSDP_H[1] & 0x0F);
            }
            else
            {
                drawCharInfo->xPos = drawCharInfo->dataInfo.paramSDP_H[0] & 0x0F;
            }

            if (drawCharInfo->dataInfo.num2_SDP == 3)
            {
                drawCharInfo->yPos =
                 (((drawCharInfo->dataInfo.paramSDP_V[0] & 0x0F)*100)
                + ((drawCharInfo->dataInfo.paramSDP_V[1] & 0x0F)*10))
                | (drawCharInfo->dataInfo.paramSDP_V[2] & 0x0F);
            }
            else if (drawCharInfo->dataInfo.num2_SDP == 2)
            {
                drawCharInfo->yPos =
                 ((drawCharInfo->dataInfo.paramSDP_V[0] & 0x0F)*10)
                + (drawCharInfo->dataInfo.paramSDP_V[1] & 0x0F);
            }
            else
            {
                drawCharInfo->yPos = drawCharInfo->dataInfo.paramSDP_V[0] & 0x0F;
            }

            drawCharInfo->xPosBase = drawCharInfo->xPos;
            drawCharInfo->yPosBase = drawCharInfo->yPos;

        }
        if (drawCharInfo->dataInfo.fSDF)
        {
            if (drawCharInfo->dataInfo.num1_SDF == 3)
            {
                drawCharInfo->drawWidth =
                 (((drawCharInfo->dataInfo.paramSDF_H[0] & 0x0F)*100)
                + ((drawCharInfo->dataInfo.paramSDF_H[1] & 0x0F)*10))
                | (drawCharInfo->dataInfo.paramSDF_H[2] & 0x0F);
            }
            else if (drawCharInfo->dataInfo.num1_SDF == 2)
            {
                drawCharInfo->drawWidth =
                 ((drawCharInfo->dataInfo.paramSDF_H[0] & 0x0F)*10)
                + (drawCharInfo->dataInfo.paramSDF_H[1] & 0x0F);
            }
            else
            {
                drawCharInfo->drawWidth = drawCharInfo->dataInfo.paramSDF_H[0] & 0x0F;
            }
            if (drawCharInfo->dataInfo.num2_SDF == 3)
            {
                drawCharInfo->drawHeight =
                 (((drawCharInfo->dataInfo.paramSDF_V[0] & 0x0F)*100)
                + ((drawCharInfo->dataInfo.paramSDF_V[1] & 0x0F)*10))
                | (drawCharInfo->dataInfo.paramSDF_V[2] & 0x0F);
            }
            else if (drawCharInfo->dataInfo.num2_SDF == 2)
            {
                drawCharInfo->drawHeight =
                 ((drawCharInfo->dataInfo.paramSDF_V[0] & 0x0F)*10)
                + (drawCharInfo->dataInfo.paramSDF_V[1] & 0x0F);
            }
            else
            {
                drawCharInfo->drawHeight = drawCharInfo->dataInfo.paramSDF_V[0] & 0x0F;
            }
        }
        if (drawCharInfo->dataInfo.fSSM)
        {
            if (drawCharInfo->dataInfo.num1_SSM == 3)
            {
                drawCharInfo->fontSize =
                 (((drawCharInfo->dataInfo.paramSSM_H[0] & 0x0F)*100)
                + ((drawCharInfo->dataInfo.paramSSM_H[1] & 0x0F)*10))
                | (drawCharInfo->dataInfo.paramSSM_H[2] & 0x0F);
            }
            else if (drawCharInfo->dataInfo.num1_SSM == 2)
            {
                drawCharInfo->fontSize =
                 ((drawCharInfo->dataInfo.paramSSM_H[0] & 0x0F)*10)
                + (drawCharInfo->dataInfo.paramSSM_H[1] & 0x0F);
            }
            else
            {
                drawCharInfo->fontSize = drawCharInfo->dataInfo.paramSSM_H[0] & 0x0F;
            }

        }
        if (drawCharInfo->dataInfo.fSHS)
        {
            if (drawCharInfo->dataInfo.num1_SHS == 3)
            {
                drawCharInfo->interval_x =
                 (((drawCharInfo->dataInfo.paramSHS[0] & 0x0F)*100)
                + ((drawCharInfo->dataInfo.paramSHS[1] & 0x0F)*10))
                 | (drawCharInfo->dataInfo.paramSHS[2] & 0x0F);
            }
            else if (drawCharInfo->dataInfo.num1_SHS == 2)
            {
                drawCharInfo->interval_x =
                 ((drawCharInfo->dataInfo.paramSHS[0] & 0x0F)*10)
                + (drawCharInfo->dataInfo.paramSHS[1] & 0x0F);
            }
            else
            {
                drawCharInfo->interval_x = drawCharInfo->dataInfo.paramSHS[0] & 0x0F;
            }

        }
        if (drawCharInfo->dataInfo.fSVS)
        {
            if (drawCharInfo->dataInfo.num1_SVS == 3)
            {
                drawCharInfo->interval_y =
                 (((drawCharInfo->dataInfo.paramSVS[0] & 0x0F)*100)
                + ((drawCharInfo->dataInfo.paramSVS[1] & 0x0F)*10))
                | (drawCharInfo->dataInfo.paramSVS[2] & 0x0F);
            }
            else if (drawCharInfo->dataInfo.num1_SVS == 2)
            {
                drawCharInfo->interval_y =
                 ((drawCharInfo->dataInfo.paramSVS[0] & 0x0F)*10)
                + (drawCharInfo->dataInfo.paramSVS[1] & 0x0F);
            }
            else
            {
                drawCharInfo->interval_y = drawCharInfo->dataInfo.paramSVS[0] & 0x0F;
            }

        }
        if (drawCharInfo->dataInfo.fRCS)
        {
            /* TODO */
        }
        if (drawCharInfo->dataInfo.fACPS)
        {
            if (drawCharInfo->dataInfo.num1_ACPS == 3)
            {
                drawCharInfo->xPos =
                 (((drawCharInfo->dataInfo.paramACPS_H[0] & 0x0F)*100)
                + ((drawCharInfo->dataInfo.paramACPS_H[1] & 0x0F)*10))
                | (drawCharInfo->dataInfo.paramACPS_H[2] & 0x0F);
            }
            else if (drawCharInfo->dataInfo.num1_ACPS == 2)
            {
                drawCharInfo->xPos =
                 ((drawCharInfo->dataInfo.paramACPS_H[0] & 0x0F)*10)
                + (drawCharInfo->dataInfo.paramACPS_H[1] & 0x0F);
            }
            else
            {
                drawCharInfo->xPos = drawCharInfo->dataInfo.paramACPS_H[0] & 0x0F;
            }
            if (drawCharInfo->dataInfo.num2_ACPS == 3)
            {
                drawCharInfo->yPos =
                 (((drawCharInfo->dataInfo.paramACPS_V[0] & 0x0F)*100)
                + ((drawCharInfo->dataInfo.paramACPS_V[1] & 0x0F)*10))
                | (drawCharInfo->dataInfo.paramACPS_V[2] & 0x0F);
            }
            else if (drawCharInfo->dataInfo.num2_ACPS == 2)
            {
                drawCharInfo->yPos =
                 ((drawCharInfo->dataInfo.paramACPS_V[0] & 0x0F)*10)
                + (drawCharInfo->dataInfo.paramACPS_V[1] & 0x0F);
            }
            else
            {
                drawCharInfo->yPos = drawCharInfo->dataInfo.paramACPS_V[0] & 0x0F;
            }

        }

        if (drawCharInfo->dataInfo.fORN)
        {
            /* TODO */
        }

        if (drawCharInfo->dataInfo.fPRA)
        {
            /* TODO */
        }
    }

    /* Character Size */
    if (drawCharInfo->dataInfo.fSSZ)
    {
        drawCharInfo->fontSize = drawCharInfo->fontSize/2;
        //drawCharInfo->block_x = (((drawCharInfo->interval_x*2)/4) + drawCharInfo->fontSize)/2;
        drawCharInfo->block_x = ((drawCharInfo->interval_x*2)/4) + drawCharInfo->fontSize;
        drawCharInfo->block_y = ((drawCharInfo->interval_y*2)/4) + drawCharInfo->fontSize;
    }
    else if (drawCharInfo->dataInfo.fMSZ)
    {
        drawCharInfo->block_x = (((drawCharInfo->interval_x*2)/2) + drawCharInfo->fontSize)/2;
        drawCharInfo->block_y = ((drawCharInfo->interval_y*2)/2) + drawCharInfo->fontSize;
    }

    /* Active position */
    if (drawCharInfo->dataInfo.fAPB)
    {
        if (drawCharInfo->xPos - drawCharInfo->block_x < drawCharInfo->xPosBase)
        {
            drawCharInfo->xPos = drawCharInfo->xPosBase
                               + drawCharInfo->drawWidth
                               - (drawCharInfo->block_x * drawCharInfo->dataInfo.APB_num);
        }
        else
        {
            drawCharInfo->xPos -= drawCharInfo->block_x*drawCharInfo->dataInfo.APB_num;
        }
    }
    if (drawCharInfo->dataInfo.fAPF)
    {
        if (drawCharInfo->xPos + drawCharInfo->block_x > drawCharInfo->xPosBase + drawCharInfo->drawWidth)
        {
            drawCharInfo->xPos = drawCharInfo->xPosBase
                               + drawCharInfo->block_x*drawCharInfo->dataInfo.APF_num;
        }
        else {
            drawCharInfo->xPos += drawCharInfo->block_x*drawCharInfo->dataInfo.APF_num;
        }
    }
    if (drawCharInfo->dataInfo.fAPD)
    {
        if (drawCharInfo->yPos + drawCharInfo->block_y > drawCharInfo->yPosBase + drawCharInfo->drawHeight)
        {
            drawCharInfo->yPos = drawCharInfo->yPosBase
                               + drawCharInfo->block_y*drawCharInfo->dataInfo.APD_num;
        }
        else {
            drawCharInfo->yPos += drawCharInfo->block_y*drawCharInfo->dataInfo.APD_num;
        }
    }
    if (drawCharInfo->dataInfo.fAPU)
    {
        if (drawCharInfo->yPos - drawCharInfo->block_y < drawCharInfo->yPosBase)
        {
            drawCharInfo->yPos = drawCharInfo->yPosBase
                               + drawCharInfo->drawHeight
                               - (drawCharInfo->block_y*drawCharInfo->dataInfo.APU_num);
        }
        else
        {
            drawCharInfo->yPos -= drawCharInfo->block_y*drawCharInfo->dataInfo.APU_num;
        }
    }
    if (drawCharInfo->dataInfo.fAPR)
    {
        drawCharInfo->xPos = drawCharInfo->xPosBase;
        drawCharInfo->yPos += drawCharInfo->block_y*drawCharInfo->dataInfo.APR_num;
    }

    if (drawCharInfo->dataInfo.fPAPF)
    {
        drawCharInfo->xPos += drawCharInfo->dataInfo.paramPAPF[0]*drawCharInfo->block_x;
    }

    if (drawCharInfo->dataInfo.fAPS)
    {
        drawCharInfo->xPos = (drawCharInfo->dataInfo.paramAPS[1] & 0x0F)*drawCharInfo->block_x
                           + drawCharInfo->xPosBase;
        drawCharInfo->yPos = (drawCharInfo->dataInfo.paramAPS[0] & 0x0F)*drawCharInfo->block_y
                           + drawCharInfo->yPosBase;
    }
}


void ccParsePesData(sCcContext *ccContext, mt_u8 *pes_data, int len)
{
    mt_u8        tmpBuf[CLOSEDCP_MAX_DECODED_MAX_SIZE];
    mt_u8*       dataPtr         = NULL;
    mt_u32      leftSize;
    mt_u32      k = 0;
    mt_s32      ret = MT_SUCCESS;

    CLOSEDCP_DrawCharInfo *prevDrawCharInfo = NULL;
    CLOSEDCP_DrawCharInfo *curDrawCharInfo;

    dataPtr = tmpBuf;

    CC_Init();
    ret = CC_AnalyzePes(pes_data,  len, &ccContext->curDecodPesInfo->headerInfo, dataPtr);

    if(ret != MT_SUCCESS)
    {
        return;
    }

    ccContext->doDecode = 1;

    if (ccContext->curDecodPesInfo->headerInfo.is_management) 	
                    { /* management data */
//                    if (ccContext->curCcInfo->tag == ccContext->curDecodPesInfo->headerInfo.language_tag) {
//                        ccContext->curCcInfo->langCode = 0;//PSISI_GetLanguageCode(ccContext->curDecodPesInfo->headerInfo.ISO_639_language_code);
//                    }
//                    else {
//                        ccContext->doDecode = 0;
//                        ccContext->ccInfo[ccContext->ccStrCurr + 1].tag =  ccContext->curDecodPesInfo->headerInfo.language_tag;
//                        ccContext->ccInfo[ccContext->ccStrCurr + 1].langCode = 0;//PSISI_GetLanguageCode(ccContext->curDecodPesInfo->headerInfo.ISO_639_language_code);
//                    }
                    }
     else
    {
                         /* Analyze CC Data */
                        if (ccContext->curDecodPesInfo->headerInfo.data_unit_size && ccContext->doDecode)
                        {
                            leftSize = ccContext->curDecodPesInfo->headerInfo.data_unit_size;

                            ccContext->isSetParam = 0;

                            while (leftSize)
                            {
                                curDrawCharInfo  = ccContext->curDecodPesInfo->drawCharInfo + k;

                                if (prevDrawCharInfo != NULL)
                                {
                                    /* copy previous parameter */
                                    curDrawCharInfo->color1     = prevDrawCharInfo->color1;
                                    curDrawCharInfo->color2     = prevDrawCharInfo->color2;
                                    curDrawCharInfo->osdWidth   = prevDrawCharInfo->osdWidth;
                                    curDrawCharInfo->osdHeight  = prevDrawCharInfo->osdHeight;
                                    curDrawCharInfo->xPos       = prevDrawCharInfo->xPos
                                                                  + (prevDrawCharInfo->dataInfo.charcount*prevDrawCharInfo->block_x);
                                    curDrawCharInfo->yPos       = prevDrawCharInfo->yPos;
                                    curDrawCharInfo->xPosBase   = prevDrawCharInfo->xPosBase;
                                    curDrawCharInfo->yPosBase   = prevDrawCharInfo->yPosBase;
                                    curDrawCharInfo->drawWidth  = prevDrawCharInfo->drawWidth;
                                    curDrawCharInfo->drawHeight = prevDrawCharInfo->drawHeight;
                                    curDrawCharInfo->fontSize   = prevDrawCharInfo->fontSize;
                                    curDrawCharInfo->interval_x = prevDrawCharInfo->interval_x;
                                    curDrawCharInfo->interval_y = prevDrawCharInfo->interval_y;
                                    curDrawCharInfo->block_x    = prevDrawCharInfo->block_x;
                                    curDrawCharInfo->block_y    = prevDrawCharInfo->block_y;
                                }

                                 CC_PRINTF(1, "data_unit_size=%d, leftSize=%d\n", ccContext->curDecodPesInfo->headerInfo.data_unit_size, leftSize);
                                 CC_DecodeDataUnit(dataPtr + ccContext->curDecodPesInfo->headerInfo.data_unit_size - leftSize,
                                                   &curDrawCharInfo->dataInfo,
                                                   &leftSize);

                                 CLOSEDCP_DrawPrepare(ccContext, curDrawCharInfo);

                                 ccContext->curDecodPesInfo->drawCharNum = ccContext->curDecodPesInfo->drawCharNum + 1;
                                 prevDrawCharInfo = curDrawCharInfo;
                                 k ++;
                                 ccContext->isSetParam = 1;
                             }
                             k = 0;
                             prevDrawCharInfo = NULL;

                             if (ccContext->decodedPesCount % CLOSEDCP_MAX_PES_NUM == 0)
                             {
                                 ccContext->curDecodPesInfo = ccContext->pesInfo;
                                 ccContext->decodedPesCount = 0;
                             }
                             else
                             {
                                 ccContext->curDecodPesInfo = ccContext->curDecodPesInfo + 1;
                             }

                             ccContext->decodedPesCount = ccContext->decodedPesCount + 1;
                             ccContext->restDecodedPesCount = ccContext->restDecodedPesCount + 1;
                         }
    }


}



