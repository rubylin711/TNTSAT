/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_demux.h"

#include "pthread.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_SDT_DEBUG

#define MT_SDT_PRINT   printf
#else

#define MT_SDT_PRINT

#endif

#define SAMPLE_SDT_FUNCTION_ENTER()            MT_SDT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_SDT_FUNCTION_EXIT()             MT_SDT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_SDT_FATAL_PRINT(fmt...)          MT_SDT_PRINT(" [FATAL] " fmt)
#define SAMPLE_SDT_ERR_PRINT(fmt...)            MT_SDT_PRINT(" [ERROR] " fmt)
#define SAMPLE_SDT_WARN_PRINT(fmt...)           MT_SDT_PRINT(" [WARN] "  fmt)
#define SAMPLE_SDT_INFO_PRINT(fmt...)           MT_SDT_PRINT(" [INFO] "  fmt)
#define SAMPLE_SDT_DBG_PRINT(fmt...)            MT_SDT_PRINT(" [DEBUG] " fmt)

#define DMX_ID_0            0
#define TUNER_ID_0          0
/*************************** Structure Definition ****************************/
typedef enum input_sig_type_t {
    MT_INPUT_SIG_TYPE_CAB = 1,
    /**<Cable signal*/
    MT_INPUT_SIG_TYPE_SAT = 2,
    /**<Satellite signal*/
    MT_INPUT_SIG_TYPE_DVB_T = 3,
    /**<Terrestrial signal*/
    MT_INPUT_SIG_TYPE_FILE = 4,
    /**<local file */
}MT_INPUR_SIG_TYPE_T;

typedef struct
{
    MT_U32 freq; /**<Frequency, in kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/
    MT_U32 mod_type; /**<QAM mode*/
} mt_input_cab_para_t;

typedef struct
{
    MT_U32 freq; /* frequency kHz */
    MT_U32 sym_rate;
    MT_U8 port_type;     //!<differ DVBS/DVBS2/AUTO from eatchother
    MT_U8 onoff_22k;                     //!< 22K on/off
    MT_U8 polarization;                  //!< Polarization
} mt_input_sat_para_t;

typedef struct
{
    MT_U32 freq; /**<Frequency, in kHz*/
    MT_U32 sym_rate; /**<Symbol rate, in bit/s*/
    MT_U32 mod_type; /**<QAM mode*/
    MT_U8 port_type;
} mt_input_ter_para_t;


typedef struct tagInput_Param_T
{
    MT_U8 file_name[256];
}mt_input_file_para_t;

typedef struct
{
    MT_INPUR_SIG_TYPE_T sig_type;
    union
    {
        mt_input_cab_para_t cab;
        mt_input_ter_para_t ter;
        mt_input_sat_para_t sat;
        mt_input_file_para_t file;
    } input_param;

} mt_input_para_t;
typedef struct
{
    MT_U8 file_name[256];
}source_file_param_t;
/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_SdtMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*!
@brief The thread that receives the file stream data
@param[in] args                 TS file name
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 InjectTsTask(MT_VOID *args)
{
    MT_S32                 s32Ret = MT_SUCCESS;
    MT_U32                 Readlen = 0;
    MT_CHAR                *fileName = NULL;
    MT_HANDLE              hTsBuffer = MT_INVALID_HANDLE;
    FILE                   *pTsFile = NULL;
    MT_UNF_STREAM_BUF_S    StreamBuf = { 0 };

    fileName = (MT_CHAR*)args;
    pTsFile = fopen(fileName, "rb");
    if(NULL == pTsFile)
    {
        SAMPLE_SDT_ERR_PRINT( "file %s open error!!\n", fileName);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        (MT_VOID)MT_UNF_DMX_DeInit();
        return s32Ret;
    }

    s32Ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    /** loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        /** Obtains a TS buffer to input data */
        s32Ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*200, &StreamBuf, 1000);
        if(MT_SUCCESS != s32Ret)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_SDT_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER...............!\n");

            rewind(pTsFile);
            continue;
        }

        /** Updates the write pointer of a TS buffer after the TS data is input */
        s32Ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != s32Ret)
        {
           SAMPLE_SDT_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }

    /** Destroys an existing TS buffer */
    (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);

    return MT_SUCCESS;
}


/*!
@brief Check if the QAM matches
@param[in]  mod_type            QAM
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static MT_S32 MT_SdtModeCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_SDT_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_SDT_ERR_PRINT("The symbol rate is not in range\n");
        return MT_FAILURE;
    }

    switch(p_cab_in->mod_type)
    {
        case 16:
            break;
        case 32:
            break;
        case 64:
            break;
        case 128:
            break;
        case 256:
            break;
        default:
            SAMPLE_SDT_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*!
@brief Check if the DVBS param matches
@param[in]  p_sat_in            DVBS param
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static mt_s32 MT_SdtModeCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_SDT_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_SDT_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


static MT_VOID MT_SdtModeParseDesc( const MT_U8* pu8DesData, MT_S32 s32DescLen, SDT_INFO *pstruProg)
{
    MT_U32 u32NameLength = 0;
    MT_U8  u8Tag = 0;
    MT_U8  u8Length = 0;

    const MT_U8* pu8Data = MT_NULL;

    if ((MT_NULL == pu8DesData) || (MT_NULL == pstruProg))
    {
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
                if (u32NameLength > 32 - 1)
                {
                    u32NameLength = (MT_U32)(32 - 1);
                }

                strncpy((MT_CHAR*)pstruProg->s8ProgName, (MT_CHAR*)pu8Data, u32NameLength);

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

            case MOSAIC_DESCRIPTOR:
                break;
            case CA_IDENTIFIER_DESCRIPTOR:
            case TELEPHONE_DESCRIPTOR:
            case MULTILINGUAL_SERVICE_NAME_DESCRIPTOR:
            case PRIVATE_DATA_SPECIFIER_DESCRIPTOR:
            case DATA_BROADCAST_DESCRIPTOR:
            case STUFFING_DESCRIPTOR:
            case BOUQUET_NAME_DESCRIPTOR:
                break;
            default:
                break;
        }
    }
}


/*!
@brief Parse the data
@param[in]  pu8SectionData             Channel handle
@param[in]  s32Length         The timeout period to set
@param[out] pSdtTb                 The address where the obtained data is stored
@param[out] pAcquiredNum         The total number of sections retrieved
@param[out] pBuffSize            The obtained section size
@return::MT_SUCCESS              Success.
@return::MT_FAILURE              Failed.
@return::ret                     The return value of the error.
@*/
static MT_S32 MT_SdtModeParseSDT(const MT_U8 *pu8SectionData, SDT_TB *pSdtTb)
{
    MT_U8 u8EitFlag = 0;
    MT_U8 u8EitFlag_PF = 0;
    MT_U8 u8RunStatus = 0;
    MT_U8 u8FreeCA = 0;
    MT_U16 u16NetworkId = 0x1FFF;
    MT_U16 u16ProgramNumber = 0;
    MT_U16 u16TsId   = 0;
    MT_S32 s32DesLen = 0;
    MT_S32 s32Length = 0;
    SDT_INFO *pSdtInfo;

    if ((MT_NULL == pu8SectionData) || (MT_NULL == pSdtTb))
    {
        return MT_FAILURE;
    }

    if (SDT_TABLE_ID_ACTUAL != pu8SectionData[0])
    {
        return MT_FAILURE;
    }

    s32Length = (MT_S32)((pu8SectionData[1] << 8) | pu8SectionData[2]) & 0x0fff;
    u16TsId = (MT_U16)((pu8SectionData[3] << 8) | pu8SectionData[4]);
    u16NetworkId = (MT_U16)((pu8SectionData[8] << 8) | pu8SectionData[9]);

    pSdtTb->u16TsId  = u16TsId;
    pSdtTb->u16NetID = u16NetworkId;

    pu8SectionData += 11;
    s32Length -= 11;

    while (s32Length > 4)
    {
        pSdtInfo = &(pSdtTb->SdtInfo[pSdtTb->u32ProgNum]);

        u16ProgramNumber = (MT_U16)((pu8SectionData[0] << 8) | pu8SectionData[1]);

        u8EitFlag = (MT_U8)((pu8SectionData[2] & 0x02) >> 1);
        u8EitFlag_PF = (MT_U8)(pu8SectionData[2] & 0x01);

        u8RunStatus = (MT_U8)((pu8SectionData[3] & 0xE0) >> 5);

        u8FreeCA = (MT_U8)((pu8SectionData[3] & 0x10) >> 4);

        pSdtInfo->u16ServiceID = u16ProgramNumber;
        pSdtInfo->u8EitFlag = u8EitFlag;
        pSdtInfo->u8EitFlag_PF = u8EitFlag_PF;
        pSdtInfo->RunState = u8RunStatus;
        pSdtInfo->CAMode = u8FreeCA;

        s32DesLen = (MT_S32)(((pu8SectionData[3] & 0x0F) << 8) | pu8SectionData[4]);


        pu8SectionData += 5;
        s32Length -= 5;

        MT_SdtModeParseDesc(pu8SectionData, s32DesLen, pSdtInfo);

        pu8SectionData += s32DesLen;
        s32Length -= s32DesLen;
        pSdtTb->u32ProgNum++;
    }

    return MT_SUCCESS;
}

/*!
@brief Read and print the section's data
@param[in]  hChannel             Channel handle
@param[in]  u32TimeOutms         The timeout period to set
@param[out] pBuf                 The address where the obtained data is stored
@param[out] pAcquiredNum         The total number of sections retrieved
@param[out] pBuffSize            The obtained section size
@return::MT_SUCCESS              Success.
@return::MT_FAILURE              Failed.
@return::ret                     The return value of the error.
@*/
static MT_S32 MT_SdtModeSectionRead(MT_HANDLE hChannel, MT_U32 u32TimeOutms, MT_U8 *pBuf, MT_U32 *pAcquiredNum)
{
    MT_U8             u8tableid = 0;
    MT_S32            ret = MT_FAILURE;
    MT_S32            startPrint = MT_FAILURE;
    MT_U32            num = 0;
    MT_U32            count = 0 ;
    MT_U32            u32Times = 0;
    MT_U32            u32SecTotalNum = 0;
    MT_U32            u32SecNum = 0;
    MT_U32            u32BeforSecNum = 0;
    MT_U32            RequestNum = 32;
    MT_UNF_DMX_DATA_S sSection[32];

    if(MT_INVALID_HANDLE == hChannel)
    {
        SAMPLE_SDT_ERR_PRINT("The input handle is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    u32Times = u32TimeOutms / 10;

    while(--u32Times)
    {
        /** Obtains the received data packets from a specific channel */
        if((MT_SUCCESS == MT_UNF_DMX_AcquireBuf(hChannel, RequestNum, &num, sSection, u32TimeOutms)) && (num > 0))
        {
            SAMPLE_SDT_INFO_PRINT("Acquire num: %d\n", num);
            for(MT_U32 i = 0; i < num; i++)
            {
                if(sSection[i].enDataType == MT_UNF_DMX_DATA_TYPE_WHOLE)
                {
                    u8tableid = sSection[i].pu8Data[0];
                    count++;
                    if(((EIT_TABLE_ID_SCHEDULE_ACTUAL_LOW <= u8tableid) && ( u8tableid <= EIT_TABLE_ID_SCHEDULE_ACTUAL_HIGH)) ||
                        ((EIT_TABLE_ID_SCHEDULE_OTHER_LOW <= u8tableid) && ( u8tableid <= EIT_TABLE_ID_SCHEDULE_OTHER_HIGH)))
                    {
                        u32SecNum = sSection[i].pu8Data[6] >> 3;
                        u32SecTotalNum = (sSection[i].pu8Data[7] >> 3) + 1;
                    }
                    else if(TDT_TABLE_ID == u8tableid || TOT_TABLE_ID == u8tableid)
                    {
                        u32SecNum = 0;
                        u32SecTotalNum = 1;
                    }
                    else
                    {
                        u32SecNum = sSection[i].pu8Data[6];
                        u32SecTotalNum = sSection[i].pu8Data[7] + 1;
                        SAMPLE_SDT_INFO_PRINT("SecNum: %d\n", u32SecNum);
                        SAMPLE_SDT_INFO_PRINT("SecTotalNum: %d\n", u32SecTotalNum);
                    }

                    *pAcquiredNum = u32SecTotalNum;

                    if(0 == u32SecNum)
                    {
                        startPrint = MT_SUCCESS;
                        u32BeforSecNum = u32SecNum;
                    }

                    if(MT_SUCCESS == startPrint)
                    {
                        /** Transfer data into the buffer */
                        memcpy((MT_VOID *)(pBuf + u32SecNum * 4096), sSection[i].pu8Data, sSection[i].u32Size);

                        if(((++u32BeforSecNum) != u32SecNum) && (u32SecNum != 0))
                        {
                            SAMPLE_SDT_ERR_PRINT("Data is missing!\n");
                            SAMPLE_SDT_ERR_PRINT("BeforSecNum: %d, currentSecNum: %d\n", --u32BeforSecNum, u32SecNum);

                            startPrint = MT_FAILURE;

                            /** Clear the buffer */
                            memset(pBuf, 0, sizeof((u32SecNum + 1) * 4096));
                        }
                        u32BeforSecNum = u32SecNum;
                    }
                }
                else
                {
                    SAMPLE_SDT_ERR_PRINT("Section data type is not MT_UNF_DMX_DATA_TYPE_WHOLE\n");

                    /** Releases the buffers for storing data packets after data packets are processed */
                    ret = MT_UNF_DMX_ReleaseBuf(hChannel, num, sSection);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_SDT_ERR_PRINT("MT_UNF_DMX_ReleaseBuf failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                        return ret;
                    }
                    return MT_FAILURE;
                }

                if(((u32SecTotalNum - 1) == u32SecNum) && (MT_SUCCESS == startPrint))
                {
                    SAMPLE_SDT_INFO_PRINT("Data is integrity!\n");

                    /** Releases the buffers for storing data packets after data packets are processed */
                    ret = MT_UNF_DMX_ReleaseBuf(hChannel, num, sSection);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_SDT_ERR_PRINT("MT_UNF_DMX_ReleaseBuf failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                        return ret;
                    }
                    return MT_SUCCESS;
                }

                if((5*u32SecTotalNum) == count)
                {
                    SAMPLE_SDT_ERR_PRINT("Data is missing!\n");

                    /** Releases the buffers for storing data packets after data packets are processed */
                    ret = MT_UNF_DMX_ReleaseBuf(hChannel, num, sSection);
                    if(MT_SUCCESS != ret)
                    {
                        SAMPLE_SDT_ERR_PRINT("MT_UNF_DMX_ReleaseBuf failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                        return ret;
                    }
                    return MT_FAILURE;
                }
            }

            /** Releases the buffers for storing data packets after data packets are processed */
            ret = MT_UNF_DMX_ReleaseBuf(hChannel, num, sSection);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SDT_ERR_PRINT("MT_UNF_DMX_ReleaseBuf failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                return ret;
            }
        }
        else
        {
            SAMPLE_SDT_ERR_PRINT("MT_UNF_DMX_AcquireBuf failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            return MT_FAILURE;
        }
    }

    SAMPLE_SDT_ERR_PRINT("MT_UNF_DMX_AcquireBuf time out-------<%s> line: %d\n", __FUNCTION__, __LINE__);
    return MT_FAILURE;
}


/*!
@brief Request section data, set channel and filter properties
@param[out] pSdtTb               The address where the STD data is stored
@return::MT_SUCCESS              Success.
@return::MT_FAILURE              Failed.
@return::flag                    The return value of the error.
@*/
static MT_S32 MT_SdtModeAcquireSection(SDT_TB *pSdtTb)
{
    MT_U8                    u8Match[DMX_FILTER_MAX_DEPTH] = { 0 };
    MT_U8                    u8Mask[DMX_FILTER_MAX_DEPTH] = { 0 };
    MT_U8                    u8Negate[DMX_FILTER_MAX_DEPTH] = { 0 };
    MT_U8                    *u8DataBuf = NULL;
    MT_U8                    *p = NULL;
    MT_U32                   u32AquiredNum = 0;
    MT_S32                   s32Ret = MT_FAILURE;
    MT_S32                   flag = MT_SUCCESS;
    MT_HANDLE                hChan = MT_INVALID_HANDLE;
    MT_HANDLE                hFilter = MT_INVALID_HANDLE;
    MT_UNF_DMX_CHAN_ATTR_S   stChanAttr= { 0 };
    MT_UNF_DMX_FILTER_ATTR_S stFilterAttr = { 0 };

    /** Creates a PID channel based on channel attributes */
    stChanAttr.u32BufSize = 16 * 1024;
    stChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    stChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
    stChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    s32Ret = MT_UNF_DMX_CreateChannel(DMX_ID_0, &stChanAttr, &hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("Create channel error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Sets the PID of a channel */
    s32Ret = MT_UNF_DMX_SetChannelPID(hChan, SDT_TSPID);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("Set pid error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        flag = s32Ret;
        goto FREE_CHANNEL;
    }

    memset(u8Match, 0, DMX_FILTER_MAX_DEPTH * sizeof(MT_U8));
    memset(u8Mask, 0xff, DMX_FILTER_MAX_DEPTH * sizeof(MT_U8));
    memset(u8Negate, 0, DMX_FILTER_MAX_DEPTH * sizeof(MT_U8));

    u8Match[0] = SDT_TABLE_ID_ACTUAL;
    u8Mask[0] = 0;

    stFilterAttr.u32FilterDepth = 4;
    memcpy(stFilterAttr.au8Match, u8Match, DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Mask, u8Mask, DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Negate, u8Negate, DMX_FILTER_MAX_DEPTH);

    /** Creates a data filter */
    s32Ret = MT_UNF_DMX_CreateFilter(DMX_ID_0, &stFilterAttr, &hFilter);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("Create filter error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        flag = s32Ret;
        goto FREE_CHANNEL;
    }
    /** Sets the filter criteria of a filter */
    s32Ret = MT_UNF_DMX_SetFilterAttr(hFilter, &stFilterAttr);

    /** Attaches filters to a specific channel */
    s32Ret |= MT_UNF_DMX_AttachFilter(hFilter, hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("Attach filter error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        flag = s32Ret;
        goto FREE_FILTER;
    }

    /** Enables a channel */
    s32Ret = MT_UNF_DMX_OpenChannel(hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("Open channel error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        flag = s32Ret;
        goto DETCH_FILTER;
    }

    u8DataBuf = malloc(4096 * 256);
    if(NULL == u8DataBuf)
    {
        goto CLOSE_CHANNEL;
    }

    memset(u8DataBuf, 0, 4096 * 256);
    /** Read and print the section's data */
    s32Ret = MT_SdtModeSectionRead(hChan, 10000, u8DataBuf, &u32AquiredNum);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("DMX_DataRead return MT_FAILURE!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        flag = s32Ret;
        goto FREE_BUFFER;
    }
    p = u8DataBuf;
    for(MT_S32 i = 0; i < u32AquiredNum; i++)
    {
        MT_SdtModeParseSDT(p, pSdtTb);
        p = p + 4096;
    }

FREE_BUFFER:
    free(u8DataBuf);
CLOSE_CHANNEL:
    /** Disables a channel */
    s32Ret = MT_UNF_DMX_CloseChannel(hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("Close channel error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
    }
DETCH_FILTER:
    s32Ret = MT_UNF_DMX_DetachFilter(hFilter, hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("Detach filter error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
    }
FREE_FILTER:
    s32Ret = MT_UNF_DMX_DestroyFilter(hFilter);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("Detach filter error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
    }
FREE_CHANNEL:
    s32Ret = MT_UNF_DMX_DestroyChannel(hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("Destroy channel error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
    }

    if(MT_SUCCESS != flag)
    {
        return flag;
    }

    return MT_SUCCESS;
}

/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_SdtModeDmxInit(MT_INPUR_SIG_TYPE_T sig_type)
{
    MT_S32           s32Ret = MT_FAILURE;
    mt_sys_version_s stSysChipInfo;

    /** Initializes the demux module */
    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SDT_ERR_PRINT("MT_UNF_DMX_Init failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    if(MT_INPUT_SIG_TYPE_FILE == sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SDT_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
            (MT_VOID)MT_UNF_DMX_DeInit();
            return s32Ret;
        }

    }
    else
    {
        memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
        s32Ret = mt_sys_get_version(&stSysChipInfo);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SDT_ERR_PRINT("failed to mt_sys_get_version\n");
            return MT_FAILURE;
        }

        if(MT_INPUT_SIG_TYPE_CAB == sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            }
            else
            {
                s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_0);
            }
        }
        else if(MT_INPUT_SIG_TYPE_SAT == sig_type)
        {
            if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
            {
                s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
            }
            else
            {
                s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            }
        }
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SDT_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}


/*!
@brief Demux module deinitialization
@return::MT_VOID
@*/
static MT_VOID MT_SdtModeDmxDeinit(MT_VOID)
{
    /** Unbind demux from the port */
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();

    return;
}


static MT_VOID MT_SdtModePrint_Help(MT_CHAR *name)
{
    MT_SDT_PRINT("Lack of parameters\n");
    MT_SDT_PRINT("\nUsage:\n");
    MT_SDT_PRINT("%s\n", name);
    MT_SDT_PRINT("    -f: path of the stream file\n");
    MT_SDT_PRINT("    -c: DVBC locks frequency\n");
    MT_SDT_PRINT("    -s: DVBS locks frequency\n");
    MT_SDT_PRINT("example:\n");
    MT_SDT_PRINT("    %s -f ./677-CC-12.ts\n", name);
    MT_SDT_PRINT("    %s -c 314 6875 64\n", name);
    MT_SDT_PRINT("    %s -s 3840 27500 1 0 0\n", name);
}


/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_SdtModeParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

    if(argc < 2)
    {
        (MT_VOID)MT_SdtModePrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:c:s")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_SdtModePrint_Help(argv[0]);
                return MT_FAILURE;

            case 'f':
                if(argc != 3)
                {
                    (MT_VOID)MT_SdtModePrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy((mt_char*)pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
            break;

            case 's':
                if(argc != 7)
                {
                    (MT_VOID)MT_SdtModePrint_Help(argv[0]);
                    return MT_FAILURE;
                }

                pInputParam->sig_type = MT_INPUT_SIG_TYPE_SAT;

                pInputParam->input_param.sat.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.sat.sym_rate = strtol(argv[3], 0, 0);
                pInputParam->input_param.sat.onoff_22k = strtol(argv[4], 0, 0);
                pInputParam->input_param.sat.polarization = strtol(argv[5], 0, 0);
                pInputParam->input_param.sat.port_type = strtol(argv[6], 0, 0);

                return MT_SUCCESS;

            case 'c':
                if(argc != 5)
                {
                    (MT_VOID)MT_SdtModePrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                return MT_SUCCESS;

        }
    }


    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_SdtMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32             ret = MT_FAILURE;
    SDT_TB             sdt_tb[32] = { 0 };
    mt_input_para_t    sInputParam = { 0 };
    pthread_t          stInjectTSThread = 0;

    ret = MT_SdtModeParase_args(argc, argv, &sInputParam);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
#ifndef MT_SAMPLE_APP
    /** System initialization */
    ret = mt_sys_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SDT_ERR_PRINT("MT_SYS_Init failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        return ret;
    }
#endif

    if(MT_INPUT_SIG_TYPE_FILE != sInputParam.sig_type)
    {
        ret = MTADP_Fe_Init(TUNER_ID_0);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SDT_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            goto ERR1;
        }

        if (MT_INPUT_SIG_TYPE_CAB == sInputParam.sig_type)
        {
            ret = MT_SdtModeCheckDvbcParam(&sInputParam.input_param.cab);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SDT_ERR_PRINT("MT_SdtModeCheckDvbcParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }

            ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                        sInputParam.input_param.cab.freq,
                                        sInputParam.input_param.cab.sym_rate,
                                        sInputParam.input_param.cab.mod_type);
        }
        else if(MT_INPUT_SIG_TYPE_SAT == sInputParam.sig_type)
        {
            ret = MT_SdtModeCheckDvbsParam(&sInputParam.input_param.sat);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SDT_ERR_PRINT("MT_SdtModeCheckDvbsParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR1;
            }

            ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                        sInputParam.input_param.sat.freq,
                                        sInputParam.input_param.sat.sym_rate,
                                        sInputParam.input_param.sat.onoff_22k,
                                        sInputParam.input_param.sat.polarization,
                                        sInputParam.input_param.sat.port_type);
        }

        if(MT_SUCCESS != ret)
        {
            SAMPLE_SDT_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            goto ERR2;
        }
    }

    /** Demux initializes and retrieves the PMT and PAT tables in TS */
    ret = MT_SdtModeDmxInit(sInputParam.sig_type);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SDT_ERR_PRINT("DVB_DmxInit failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        goto ERR2;
    }

    if(MT_INPUT_SIG_TYPE_FILE == sInputParam.sig_type)
    {
        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))InjectTsTask, &sInputParam.input_param.file);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SDT_ERR_PRINT("failed to pthread_create\n");
            goto ERR3;
        }
        sleep(1);
        if(g_bTaskQuit == MT_TRUE)
        {
            goto ERR3;
        }
    }

    ret = MT_SdtModeAcquireSection(&sdt_tb[0]);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_SDT_ERR_PRINT("MT_SdtModeAcquireSection failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        goto ERR4;
    }

    for(MT_S32 j = 0; j < 32; j++)
    {
        if(sdt_tb[j].u32ProgNum == 0)
        {
            continue;
        }
        MT_SDT_PRINT("\n");
        MT_SDT_PRINT("SDT data info:\n");
        MT_SDT_PRINT(" ProgNum:     %d\n", sdt_tb[j].u32ProgNum);
        MT_SDT_PRINT(" TsId:        %d\n", sdt_tb[j].u16TsId);
        MT_SDT_PRINT(" NetID:       %d\n", sdt_tb[j].u16NetID);
        for(MT_S32 i = 0; i < sdt_tb[j].u32ProgNum; i++)
        {
            MT_SDT_PRINT("=======================\n");
            MT_SDT_PRINT(" ServiceID:   %d\n", sdt_tb[j].SdtInfo[i].u16ServiceID);
            MT_SDT_PRINT(" EitFlag:     %d\n", sdt_tb[j].SdtInfo[i].u8EitFlag);
            MT_SDT_PRINT(" EitFlag_PF:  %d\n", sdt_tb[j].SdtInfo[i].u8EitFlag_PF);
            MT_SDT_PRINT(" RunState:    %d\n", sdt_tb[j].SdtInfo[i].RunState);
            MT_SDT_PRINT(" CAMode:      %d\n", sdt_tb[j].SdtInfo[i].CAMode);
            MT_SDT_PRINT(" ServiceType: %d\n", sdt_tb[j].SdtInfo[i].u32ServiceType);
            MT_SDT_PRINT(" ProgName:    %s\n", sdt_tb[j].SdtInfo[i].s8ProgName);
        }
        MT_SDT_PRINT("=======================\n");
    }

ERR4:
    if(MT_INPUT_SIG_TYPE_FILE == sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(stInjectTSThread, NULL);
    }
ERR3:
    /** Demux module deinitialization */
    (MT_VOID)MT_SdtModeDmxDeinit();
ERR2:
    if(MT_INPUT_SIG_TYPE_FILE != sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR1:
#ifndef MT_SAMPLE_APP
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif

    return ret;
}
