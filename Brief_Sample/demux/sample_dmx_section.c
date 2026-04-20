/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_demux.h"
#include "mt_adp_frontend.h"

#include "pthread.h"

/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_SECTION_DEBUG
#define MT_SECTION_PRINT   printf
#else
#define MT_SECTION_PRINT
#endif

#define SAMPLE_SECTION_FUNCTION_ENTER()             MT_SECTION_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_SECTION_FUNCTION_EXIT()              MT_SECTION_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_SECTION_FATAL_PRINT(fmt...)          MT_SECTION_PRINT(" [FATAL] " fmt)
#define SAMPLE_SECTION_ERR_PRINT(fmt...)            MT_SECTION_PRINT(" [ERROR] " fmt)
#define SAMPLE_SECTION_WARN_PRINT(fmt...)           MT_SECTION_PRINT(" [WARN] "  fmt)
#define SAMPLE_SECTION_INFO_PRINT(fmt...)           MT_SECTION_PRINT(" [INFO] "  fmt)
#define SAMPLE_SECTION_DBG_PRINT(fmt...)            MT_SECTION_PRINT(" [DEBUG] " fmt)

#define DMX_ID_0             0
#define TUNER_ID_0           0
#define MAX_FILTER_NUM       64
#define MAX_SECTIONS_PER_MOD 256
/*************************** Structure Definition ****************************/
typedef struct
{
  MT_S32 is_received;
}section_info_t;
typedef struct
{
  MT_CHAR last_section_number;
  MT_S32 start;
  MT_U32 before_number;
  section_info_t section_info[MAX_SECTIONS_PER_MOD];
}mod_info_t;
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
static MT_BOOL   g_bTaskQuit = MT_FALSE;
static MT_U32    m_Table[256] =
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
  0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,  0xC5A92679,
  0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
  0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673,
  0xFDE69BC4,  0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
  0x933EB0BB, 0x97FFAD0C,  0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,0xBCB4666D,
  0xB8757BDA, 0xB5365D03, 0xB1F740B4,
};

/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_SectionMain(MT_S32 argc, MT_CHAR *argv[]);
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
        SAMPLE_SECTION_ERR_PRINT( "file %s open error!!\n", fileName);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SECTION_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        fclose(pTsFile);
        pTsFile = NULL;
        return s32Ret;
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
            SAMPLE_SECTION_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER...............!\n");

            rewind(pTsFile);
            continue;
        }

        /** Updates the write pointer of a TS buffer after the TS data is input */
        s32Ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != s32Ret)
        {
           SAMPLE_SECTION_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    /** Destroys an existing TS buffer */
    (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }

    return MT_SUCCESS;
}



/*!
@brief Check if the QAM matches
@param[in]  mod_type            QAM
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static MT_S32 MT_SectionModeCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_SECTION_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_SECTION_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_SECTION_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
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
static mt_s32 MT_SectionModeCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_SECTION_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_SECTION_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_SectionModeDmxInit(MT_INPUR_SIG_TYPE_T sig_type)
{
    MT_S32           s32Ret = MT_FAILURE;
    mt_sys_version_s stSysChipInfo;

    /** Initializes the demux module */
    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SECTION_ERR_PRINT("MT_UNF_DMX_Init failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    if(MT_INPUT_SIG_TYPE_FILE == sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SECTION_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, s32Ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
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
            SAMPLE_SECTION_ERR_PRINT("failed to mt_sys_get_version\n");
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
            SAMPLE_SECTION_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}


/*!
@brief Demux module deinitialization
@return::MT_VOID
@*/
static MT_VOID MT_SectionModeDmxDeInit(MT_VOID)
{
    /** Unbind demux from the port */
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();

    return;
}


static MT_U32 MT_SectionModeCRC32(MT_U8 *buffer, MT_U32 size)
{
    MT_U32 Result = 0xFFFFFFFF;

    while(size--)
    {
        Result = (Result << 8) ^ m_Table[(Result >> 24) ^ *buffer++];
    }

    return Result;
}


static MT_S32 MT_SectionModeRecvSection(MT_HANDLE hChannel)
{
    MT_U32            times = 0;
    MT_U32            gettimes = 0;
    MT_U32            i = 0;
    MT_U32            k = 0;
    MT_U32            u32AcquireNum = 0;
    MT_U32            pu32AcquiredNum = 0;
    MT_U32            mod_id = 0;
    MT_U32            last_section_number = 0;
    MT_U32            section_number = 0;
    MT_U32            crcSoftware = 0;
    MT_U32            crcStream = 0;
    MT_S32            ret = MT_FAILURE;
    mod_info_t        *mod_info = NULL;
    MT_UNF_DMX_DATA_S pstBuf[32];

    if(MT_INVALID_HANDLE == hChannel)
    {
        SAMPLE_SECTION_ERR_PRINT("The input handle is empty-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    mod_info = (mod_info_t*)malloc(sizeof(mod_info_t) * 0xffff);
    memset(mod_info, 0, sizeof(mod_info_t) * 0xffff);
    gettimes = 512;
    u32AcquireNum = 32;

    while(gettimes--)
    {
        ret = MT_UNF_DMX_AcquireBuf(hChannel, u32AcquireNum, &pu32AcquiredNum, pstBuf, (MT_U32)5000);
        if(MT_SUCCESS != ret)
        {
            MT_SECTION_PRINT("MT_UNF_DMX_AcquireBuf failed!\n");
            MT_USLEEP(10 * 1000);
            times++;
            if(3 == times)
            {
                MT_SECTION_PRINT("Failed to get buff, Please check channel PID\n");
                free(mod_info);
                mod_info = NULL;
                return MT_FAILURE;
            }
            continue;
        }

        for(i = 0; i < pu32AcquiredNum; i++)
        {
            mod_id = (pstBuf[i].pu8Data[3] << 8) | pstBuf[i].pu8Data[4];
            section_number = pstBuf[i].pu8Data[6];
            last_section_number = pstBuf[i].pu8Data[7];
            mod_info[mod_id].last_section_number = last_section_number;

            if((0 != section_number) && (1 != mod_info[mod_id].start))
            {
                MT_SECTION_PRINT("[%04x] data is incomplete++++++++++++++++++++++++++++++++++++\n", mod_id);
            }

            if(1 == mod_info[mod_id].start)
            {
                if((++mod_info[mod_id].before_number) == section_number)
                {
                    mod_info[mod_id].section_info[section_number].is_received = 1;
                    mod_info[mod_id].before_number = section_number;
                    if(section_number == mod_info[mod_id].last_section_number)
                    {
                        MT_SECTION_PRINT("[%04x] data is complete\n", mod_id);
                        MT_SECTION_PRINT("0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9\n");
                        for(int j = 0; j <= mod_info[mod_id].last_section_number; j++)
                        {
                            if(1 == mod_info[mod_id].section_info[j].is_received)
                            {
                                MT_SECTION_PRINT("1 ");
                            }
                            else
                            {
                                MT_SECTION_PRINT("0 ");
                            }
                        }
                        MT_SECTION_PRINT("\n");
                        memset(&mod_info[mod_id], 0, sizeof(mod_info_t) * 0xff);
                    }
                }
                else
                {
                    MT_SECTION_PRINT("[%04x] data is incomplete++++++++++++++++++++++++++++++++++++\n", mod_id);
                    memset(&mod_info[mod_id], 0, sizeof(mod_info_t) * 0xff);
                }
            }

            if((0 == section_number) && (1 != mod_info[mod_id].section_info[section_number].is_received))
            {
                if(0 != mod_info[mod_id].last_section_number)
                {
                    mod_info[mod_id].start = 1;
                    mod_info[mod_id].before_number = section_number;
                    mod_info[mod_id].section_info[section_number].is_received = 1;
                }
                else
                {
                    MT_SECTION_PRINT("[%04x] data is complete\n", mod_id);
                    MT_SECTION_PRINT("0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9\n");
                    MT_SECTION_PRINT("1\n");
                }
            }

            k = pstBuf[i].u32Size;
            crcSoftware = MT_SectionModeCRC32(&(pstBuf[i].pu8Data[0]), pstBuf[i].u32Size - 4);
            crcStream=pstBuf[i].pu8Data[k-4];
            crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-3];
            crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-2];
            crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-1];
            if(crcSoftware != crcStream)
            {
                MT_SECTION_PRINT("err.CRC,don't care TDT!!!\n");
            }
        }

        ret = MT_UNF_DMX_ReleaseBuf(hChannel, pu32AcquiredNum, pstBuf);
        if (MT_SUCCESS != ret)
        {
            MT_SECTION_PRINT("call MT_UNF_DMX_ReleaseBuf failed!\n");
        }

    }

    for(int j = 0; j < 0xffff; j++)
    {
        if(1 == mod_info[j].start)
        {
            MT_SECTION_PRINT("[%04x] completed data was not checked\n", j);
            MT_SECTION_PRINT("0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9\n");
            for(i = 0; i <= mod_info[j].last_section_number; i++)
            {
                if(1 == mod_info[j].section_info[i].is_received)
                {
                    MT_SECTION_PRINT("1 ");
                }
                else
                {
                    MT_SECTION_PRINT("0 ");
                }
            }
            MT_SECTION_PRINT("\n");
        }
    }

    free(mod_info);
    mod_info = NULL;

    MT_SECTION_PRINT("receive over!\n");

    return MT_SUCCESS;
}


/*!
@brief Request section data, set channel and filter properties
@param[out] pSdtTb               The address where the STD data is stored
@return::MT_SUCCESS              Success.
@return::MT_FAILURE              Failed.
@return::flag                    The return value of the error.
@*/
static MT_S32 MT_SectionModeAcquireSection(MT_U32 ChannelPID)
{
    MT_U8                    u8Negate[DMX_FILTER_MAX_DEPTH] = { 0 };
    MT_S32                   s32Ret = MT_FAILURE;
    MT_S32                   flag = MT_SUCCESS;
    MT_HANDLE                hChan = MT_INVALID_HANDLE;
    MT_HANDLE                hFilter[3] = { MT_INVALID_HANDLE };
    MT_UNF_DMX_CHAN_ATTR_S   stChanAttr= { 0 };
    MT_UNF_DMX_FILTER_ATTR_S stFilterAttr = { 0 };
    MT_U8 match[2][1] = { {0x3c}, {0x3b} };
    MT_U8 mask[2][1] = { {0x0}, {0x0} };
    /** Creates a PID channel based on channel attributes */
    stChanAttr.u32BufSize = 16 * 1024;
    stChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    stChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
    stChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    s32Ret = MT_UNF_DMX_CreateChannel(DMX_ID_0, &stChanAttr, &hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SECTION_ERR_PRINT("Create channel error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return s32Ret;
    }

    /** Sets the PID of a channel */
    s32Ret = MT_UNF_DMX_SetChannelPID(hChan, ChannelPID);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SECTION_ERR_PRINT("Set pid error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        flag = s32Ret;
        goto ERR0;
    }

    memset(u8Negate, 0, DMX_FILTER_MAX_DEPTH * sizeof(MT_U8));

    for(int j = 0; j < 2; j++)
    {
        stFilterAttr.u32FilterDepth = 1;
        memcpy(stFilterAttr.au8Match, match[j], DMX_FILTER_MAX_DEPTH);
        memcpy(stFilterAttr.au8Mask, mask[j], DMX_FILTER_MAX_DEPTH);
        memcpy(stFilterAttr.au8Negate, u8Negate, DMX_FILTER_MAX_DEPTH);

        /** Creates a data filter */
        s32Ret = MT_UNF_DMX_CreateFilter(DMX_ID_0, &stFilterAttr, &hFilter[j]);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SECTION_ERR_PRINT("Create filter error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            flag = s32Ret;
            if(0 != j)
            {
                (MT_VOID)MT_UNF_DMX_DetachFilter(hFilter[0], hChan);
                (MT_VOID)MT_UNF_DMX_DestroyFilter(hFilter[0]);
            }
            goto ERR0;
        }
        /** Sets the filter criteria of a filter */
        s32Ret = MT_UNF_DMX_SetFilterAttr(hFilter[j], &stFilterAttr);

        /** Attaches filters to a specific channel */
        s32Ret |= MT_UNF_DMX_AttachFilter(hFilter[j], hChan);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SECTION_ERR_PRINT("Attach filter error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            flag = s32Ret;
            if(0 == j)
            {
                (MT_VOID)MT_UNF_DMX_DestroyFilter(hFilter[0]);
            }
            else
            {
                (MT_VOID)MT_UNF_DMX_DetachFilter(hFilter[0], hChan);
                (MT_VOID)MT_UNF_DMX_DestroyFilter(hFilter[0]);
                (MT_VOID)MT_UNF_DMX_DestroyFilter(hFilter[1]);
            }
            goto ERR0;
        }
    }

    /** Enables a channel */
    s32Ret = MT_UNF_DMX_OpenChannel(hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SECTION_ERR_PRINT("Open channel error!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        flag = s32Ret;
        goto ERR1;
    }

    /** Read and print the section's data */
    s32Ret = MT_SectionModeRecvSection(hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SECTION_ERR_PRINT("MT_SectionModeRecvSection return MT_FAILURE!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        flag = s32Ret;
    }

    /** Disables a channel */
    (MT_VOID)MT_UNF_DMX_CloseChannel(hChan);
ERR1:
    (MT_VOID)MT_UNF_DMX_DetachFilter(hFilter[0], hChan);
    (MT_VOID)MT_UNF_DMX_DetachFilter(hFilter[1], hChan);
    (MT_VOID)MT_UNF_DMX_DestroyFilter(hFilter[0]);
    (MT_VOID)MT_UNF_DMX_DestroyFilter(hFilter[1]);
ERR0:
    (MT_VOID)MT_UNF_DMX_DestroyChannel(hChan);

    if(MT_SUCCESS != flag)
    {
        return flag;
    }

    return MT_SUCCESS;
}


static MT_VOID MT_SectionModePrintMenu(MT_VOID)
{
    MT_SECTION_PRINT("commond: \n");
    MT_SECTION_PRINT("     p: enter the PID \n");
    MT_SECTION_PRINT("     q: quit \n");
    MT_SECTION_PRINT("     h: help \n");
    MT_SECTION_PRINT("SECTION>> ");
}


static MT_S32 MT_SectionModeCmdTask(MT_VOID)
{
    MT_CHAR *pfgetret = NULL;
    MT_CHAR inPutCmd[32] ={ 0 };
    MT_U32  channelPid = 0;

    while(1)
    {
        (MT_VOID)MT_SectionModePrintMenu();

        pfgetret=fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);
        pfgetret=pfgetret;
        if ('q' == inPutCmd[0])
        {
            g_bTaskQuit = MT_TRUE;
            SAMPLE_SECTION_INFO_PRINT("prepare to exit!\n");
            break;
        }
        else if ('p' == inPutCmd[0])
        {
            MT_SECTION_PRINT("Input PID>> ");
            scanf("%x", &channelPid);
            getchar();
            (MT_VOID)MT_SectionModeAcquireSection(channelPid);
        }
        else if ('h' == inPutCmd[0])
        {
            SAMPLE_SECTION_INFO_PRINT("Print help info \n");
        }
    }
    return 0;
}

static MT_VOID MT_SectionModePrint_Help(MT_CHAR *name)
{
    MT_SECTION_PRINT("Lack of parameters\n");
    MT_SECTION_PRINT("\nUsage:\n");
    MT_SECTION_PRINT("%s\n", name);
    MT_SECTION_PRINT("    -f: path of the stream file\n");
    MT_SECTION_PRINT("    -c: DVBC locks frequency\n");
    MT_SECTION_PRINT("    -s: DVBS locks frequency\n");
    MT_SECTION_PRINT("example:\n");
    MT_SECTION_PRINT("    %s -f ./490.ts\n", name);
    MT_SECTION_PRINT("    %s -c 314 6875 64\n", name);
    MT_SECTION_PRINT("    %s -s 3840 27500 1 0 0\n", name);
}


/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_SectionParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

    if(argc < 2)
    {
        (MT_VOID)MT_SectionModePrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:c:s")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_SectionModePrint_Help(argv[0]);
                return MT_FAILURE;

            case 'f':
                if(argc != 3)
                {
                    (MT_VOID)MT_SectionModePrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy((mt_char*)pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
            break;

            case 's':
                if(argc != 7)
                {
                    (MT_VOID)MT_SectionModePrint_Help(argv[0]);
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
                    (MT_VOID)MT_SectionModePrint_Help(argv[0]);
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
MT_S32 MT_SectionMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32    s32Ret = MT_FAILURE;
    mt_input_para_t    sInputParam = { 0 };
    pthread_t          stInjectTSThread = 0;

    s32Ret = MT_SectionParase_args(argc, argv, &sInputParam);
    if(MT_SUCCESS != s32Ret)
    {
        return MT_SUCCESS;
    }

#ifndef MT_SAMPLE_APP
        /** System initialization */
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SECTION_ERR_PRINT("failed to mt_sys_init\n");
            return s32Ret;
        }
#endif

    if(MT_INPUT_SIG_TYPE_FILE != sInputParam.sig_type)
    {
        s32Ret = MTADP_Fe_Init(TUNER_ID_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SECTION_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            goto ERR1;
        }

        if (MT_INPUT_SIG_TYPE_CAB == sInputParam.sig_type)
        {
            s32Ret = MT_SectionModeCheckDvbcParam(&sInputParam.input_param.cab);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_SECTION_ERR_PRINT("MT_SectionModeCheckDvbcParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }

            s32Ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                        sInputParam.input_param.cab.freq,
                                        sInputParam.input_param.cab.sym_rate,
                                        sInputParam.input_param.cab.mod_type);
        }
        else if(MT_INPUT_SIG_TYPE_SAT == sInputParam.sig_type)
        {
            s32Ret = MT_SectionModeCheckDvbsParam(&sInputParam.input_param.sat);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_SECTION_ERR_PRINT("MT_SectionModeCheckDvbsParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }

            s32Ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                        sInputParam.input_param.sat.freq,
                                        sInputParam.input_param.sat.sym_rate,
                                        sInputParam.input_param.sat.onoff_22k,
                                        sInputParam.input_param.sat.polarization,
                                        sInputParam.input_param.sat.port_type);
        }

        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SECTION_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
            goto ERR2;
        }
    }

    /** DMX module initialization */
    s32Ret = MT_SectionModeDmxInit(sInputParam.sig_type);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_SECTION_ERR_PRINT("failed to StartDmx\n");
        goto ERR2;
    }

    if(MT_INPUT_SIG_TYPE_FILE == sInputParam.sig_type)
    {
        g_bTaskQuit = MT_FALSE;
        s32Ret = pthread_create(&stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))InjectTsTask, &sInputParam.input_param.file);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SECTION_ERR_PRINT("failed to pthread_create\n");
            goto ERR3;
        }
        sleep(1);
        if(g_bTaskQuit == MT_TRUE)
        {
            goto ERR3;
        }
    }

    (MT_VOID)MT_SectionModeCmdTask();

    if(MT_INPUT_SIG_TYPE_FILE == sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;
        pthread_join(stInjectTSThread, NULL);
    }
ERR3:
    /** Demux module deinitialization */
    (MT_VOID)MT_SectionModeDmxDeInit();
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
    return s32Ret;
}
