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
#include "mt_adp_demux.h"
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
#include "pthread.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_EIT_DEBUG
#define MT_EIT_PRINT   printf
#else
#define MT_EIT_PRINT
#endif

#define SAMPLE_EIT_FUNCTION_ENTER()               MT_EIT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_EIT_FUNCTION_EXIT()                MT_EIT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_EIT_FATAL_PRINT(fmt...)            MT_EIT_PRINT(" [FATAL] " fmt)
#define SAMPLE_EIT_ERR_PRINT(fmt...)              MT_EIT_PRINT(" [ERROR] " fmt)
#define SAMPLE_EIT_WARN_PRINT(fmt...)             MT_EIT_PRINT(" [WARN] "  fmt)
#define SAMPLE_EIT_INFO_PRINT(fmt...)             MT_EIT_PRINT(" [INFO] "  fmt)
#define SAMPLE_EIT_DBG_PRINT(fmt...)              MT_EIT_PRINT(" [DEBUG] " fmt)

#define SAMPLE_EIT_PRINT   printf


#define DMX_ID_0            0
#define TUNER_ID_0          0
#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2


#define MAX_SECTIONS_PER_MOD 256
#define SECITON_BUF_SIZE 512*1024
#define MT_EIT_TIME_ZONE_EAST8 8


/*!
  Short event descriptor id
  */
#define DVB_DESC_SHORT_EVENT               0x4d
/*!
  Extend event
  */
#define DVB_DESC_EXT_EVENT                 0x4e
/*!
  Time shifted event descriptor id
  */
#define DVB_DESC_TIME_SHIFTED_EVT          0x4f
/*!
  Component descriptor id
  */
#define DVB_DESC_COMPONENT_DESC            0x50

/*!
  Content descriptor id
  */
#define DVB_DESC_CONTENT                   0x54
/*!
  Parental rating descriptor id
  */
#define DVB_DESC_PARENTAL_RATING           0x55

/*!
  MAX service number in one EIT section
  */
#define MAX_EVT_PER_SEC  (64)
/*!
  Max service name length
  */
#define MAX_EVT_NAME_LEN    (64)
/*!
  Max short text length in EIT table
  */
#define MAX_SHT_TEXT_LEN    (256)

/*!
  Language code length
  */
#define LANGUAGE_CODE_LEN (3)

/*!
  MAX length to item description
  */
#define MAX_ITEM_DESC_LEN (15)

/*!
  MAX length to item content
  */
#define MAX_ITEM_CONT_LEN (15)

/*!
  MAX item number
  */
#define MAX_ITEM_NUM      (3)

/*!
  MAX txt length
  */
#define MAX_EXT_TXT_LEN   (256)

/*!
  Max number of extend event descriptors
  */
#define MAX_EXT_DESC_NUM  (16)

/*!
  Max extend event txt number
  */
#define MAX_EXT_TXT_NUM  (16)

/*!
  Max Short event txt number
  */
#define MAX_SHT_TXT_NUM  (32)
/*!
  Length of EIT head
  */
#define EIT_SEC_HEAD_LEN  12
/*!
  Min length of EIT
  */
#define EIT_MIN_LEN 15
/*!
  CRC length
  */
#define CRC_SIZE  (4)


/*!
   Todo: fix me
  */
#define SYS_GET_LOW_HALF_BYTE(X)          ((X) & (0x0f))

/*!
   Todo: fix me
  */
#define SYS_GET_HIGH_HALF_BYTE(X)         ((X) & (0xf0))


/*!
   Todo: fix me
  */
#define MASK_FIRST_BIT_OF_BYTE(X)         ((X) & (0x01))

/*!
  Combination of two byte(8bit) as a word(16bit)
  */
#define MAKE_WORD2(high, low) ((u16)(((u8)(low)) | \
                                       ((( u16)((u8)(high))) << 8)))



/*************************** Structure Definition ****************************/
#ifndef utc_time_t
typedef struct
{
  /*!
    Year
    */
  u16 year;
  /*!
    Month
    */
  u8 month;
  /*!
    Day
    */
  u8 day;
  /*!
    Hour
    */
  u8 hour;
  /*!
    Minute
    */
  u8 minute;
  /*!
    Sec
    */
  u8 second;
  /*!
    researved.
    */
  u8 reserved;
} utc_time_t;
#endif

typedef struct
{
  MT_S32 is_received;
}section_info_t;

typedef struct
{
  MT_CHAR max_section_number;
  MT_BOOL start;
  MT_U32 recv_number;
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

/* EIT table */



/*!
  time shifted event descriptor
  */
typedef struct tag_time_shifted_evt_descr
{
    /*!
      reference service id
      */
    u16 svc_id;
    /*!
      reference event id
      */
    u16 evt_id;
}time_shifted_evt_descr_t;


/*!
  Item info in extend event descriptor
  */
typedef struct
{
  /*!
    Item description, such as cast list etc.
    */
  u8 item_name[MAX_ITEM_DESC_LEN];
  /*!
    Item content, such as cast name etc.
    */
  u8 item_cont[MAX_ITEM_CONT_LEN];
}item_info_t;


/*!
  EIT extend event descriptor
  */
typedef struct
{
  /*!
    Extend ext event index in extend event description series
    */
  u8 index;
  /*!
    Language code
    */
  u8 lang_code[LANGUAGE_CODE_LEN];
  /*!
    MAX item information
    */
  item_info_t item[MAX_ITEM_NUM];
  /*!
    item number
    */
  u8  item_num;
  /*!
    Total desc number
    */
  u8  tot_ext_desc;
  /*!
    Event id
    */
  u16 evt_id;
}ext_evt_desc_t;

/*!
  Content descriptor
  */
typedef struct
{
  /*!
    content nibble
    High 4 bits are level 1
    Low 4 bits are level 2
    */
  u8 cont_level;
  /*!
    User nibble
    High 4 bits are nibble 1
    Low 4 bits are nibble 2
    */
  u8 usr_nib;
  /*!
    Event id
    */
  u16 evt_id;
}cont_desc_t;

/*!
  Short event text info
  */
typedef struct
{
  /*!
    Text of short event
    */
  u8 txt[MAX_SHT_TEXT_LEN];
  /*!
    Event id
    */
  u16 evt_id;
  /*!
    Length of short event text
    */
  u16 txt_len;
  /*!
     Length of event name
     */
   u8 name_len;
   /*!
     Event name of cerain event
     */
   u8 evt_nm[MAX_EVT_NAME_LEN];
    /*!
    Language code
    */
  u8 lang_code[LANGUAGE_CODE_LEN];


}sht_evt_txt_t;



/*!
  Extend event text information
  */
typedef struct
{
  /*!
    TXT info in extend event description
    */
  u8  ext_txt[MAX_EXT_TXT_LEN];
  /*!
    Extend text length
    */
  u8 txt_len;
  /*!
    language code
     */
  u8 lang_code[LANGUAGE_CODE_LEN];
  /*!
    Event id
    */
  u16 evt_id;
}ext_evt_txt_t;

/*!
  parental rating descriptor
  */
typedef struct parental_rating_desc
{
    /*!
      country_code
      */
    u8 country_code[3];
    /*!
      event rating
      */
    u8 rating;
}parental_rating_desc_t;


/*!
  EIT event descriptor for one event
  */
typedef struct
{
  /*!
    Event id
    */
  u16 evt_id;
  /*!
    evt running status
    */
  u8 running_status : 3;
  /*!
    Duration of event time
    */
  utc_time_t drt_time;
  /*!
    Start time of event time
    */
  utc_time_t st_tm ;
  /*!
    time shifted event descriptor found
    */
  u8 time_shifted_evt_des_found;
  /*!
    time shifted event descriptor
    */
  time_shifted_evt_descr_t time_shifted_evt_des;
  /*!
     parental rating
        */
  parental_rating_desc_t  parental_rating;
}sht_evt_desc_t;



/*!
  EIT data for processing EIT information
  */
typedef struct
{
  /*!
    Ts stream id,concern with different TP
    */
  u16 stream_id;
  /*!
    Table id
    */
  u8  table_id;
  /*!
    Version number
    */
  u8  version;
  /*!
    Section length
    */
  u16 sec_length;
  /*!
    Service id
    */
  u16 svc_id;
  /*!
    If the nit info is avalaible in current TS or other TS
    */
  MT_BOOL pf_flag;
  /*!
    The counter of this section in max section length
    */
  u8 section_number;
  /*!
    Max section length.
    */
  u8 last_section_number;
  /*!
    Last section number in each section
    */
  u8 seg_last_sec_number;
  /*!
    Last table id of EIT with the same service id
    */
  u8 last_table_id;
  /*!
    Descriptor struct for des LOOP content
    */
  sht_evt_desc_t sht_evt_info[MAX_EVT_PER_SEC] ;
  /*!
    Extend event info
    */
  ext_evt_desc_t ext_evt_info[MAX_EXT_DESC_NUM];
  /*!
    Content description array
    */
  cont_desc_t cont_desc[MAX_EVT_PER_SEC];
  /*!
    Short event txt array
    */
  sht_evt_txt_t sht_txt[MAX_SHT_TXT_NUM];
  /*!
    Extend event txt array
    */
  ext_evt_txt_t ext_txt[MAX_EXT_TXT_NUM];
  /*!
    Short event description number
    */
  u8 tot_evt_num;
  /*!
    Total extend event descriptor number
    */
  u8 tot_ext_info_num;
  /*!
    Content desc number
    */
  u8 tot_cont_num;
  /*!
    Short event txt number
    */
  u8 tot_sht_txt_num;
  /*!
    Extend event text number
    */
  u8 tot_ext_txt_num;
  /*!
    Id for rebroadcast other network's info
    */
  u16 org_nw_id;
}TS_Eit_T;

/* end */


typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
    pthread_t          stInjectTSThread;
    PMT_COMPACT_TBL    *pProgTbl;
    mt_input_para_t    sInputParam;
} MT_EIT_RUN_INFO;





/********************** Global Variable declaration **************************/
static MT_BOOL   g_bTaskQuit = MT_TRUE;
static MT_EIT_RUN_INFO eit_run_info;

/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_EitMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*!
@brief The thread that receives the file stream data
@param[in] args                 TS file name
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_EitModeInjectTsTask(MT_VOID *args)
{
    MT_S32              s32Ret = MT_SUCCESS;
    MT_U32              Readlen = 0;
    MT_HANDLE           hTsBuffer = MT_INVALID_HANDLE;
    MT_CHAR             *fileName = NULL;
    FILE                *pTsFile = NULL;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };

    fileName = (MT_CHAR*)args;
    pTsFile = fopen(fileName, "rb");
    if(NULL == pTsFile)
    {
        SAMPLE_EIT_ERR_PRINT( "file %s open error!!\n", fileName);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    s32Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        fclose(pTsFile);
        return s32Ret;
    }

    s32Ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);
        fclose(pTsFile);
        return MT_FAILURE;
    }

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        s32Ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*1000, &StreamBuf, 1000);
        if(MT_SUCCESS != s32Ret)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_EIT_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }

        s32Ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != s32Ret)
        {
           SAMPLE_EIT_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }
    (MT_VOID)MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);

    return MT_SUCCESS;
}


/*!
@brief Check if the QAM matches
@param[in]  mod_type            QAM
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static MT_S32 MT_EitModeCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_EIT_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_EIT_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_EIT_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
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
static mt_s32 MT_EitModeCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_EIT_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_EIT_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}



/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_EitModeDmxInit(MT_INPUR_SIG_TYPE_T sig_type)
{
    MT_S32           s32Ret = MT_FAILURE;
    mt_sys_version_s stSysChipInfo;

    /** Initializes the demux module */
    s32Ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("MT_UNF_DMX_Init failed, s32Ret = 0x%x\n", s32Ret);
        return s32Ret;
    }

    if(MT_INPUT_SIG_TYPE_FILE == sig_type)
    {
        s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_EIT_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed, s32Ret = 0x%x\n", s32Ret);
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
            SAMPLE_EIT_ERR_PRINT("failed to mt_sys_get_version\n");
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
            SAMPLE_EIT_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

/*!
@brief Demux module deinitialization
@return::MT_VOID
@*/
static MT_VOID MT_EitModeDmxDeInit(MT_VOID)
{

    /** Unbind demux from the port */
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();
    return;
}


static MT_VOID MT_EitExit(void)
{
    g_bTaskQuit = MT_TRUE;

#ifndef MT_SAMPLE_APP
    /** Stop playing the show */

    (MT_VOID)MTADP_Search_FreeAllPmt(eit_run_info.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(MT_INPUT_SIG_TYPE_FILE == eit_run_info.sInputParam.sig_type)
    {
        /** Wait for the thread to end */
        pthread_join(eit_run_info.stInjectTSThread, NULL);
    }

    (MT_VOID)MT_EitModeDmxDeInit();

    if(MT_INPUT_SIG_TYPE_FILE != eit_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
#endif

    memset(&eit_run_info, 0xff, sizeof(eit_run_info));
}

void MT_EiTimeTrans(mt_u8 *p_in, utc_time_t *p_out)
{
  mt_u8 s = 0;
  mt_u8 i = 0;

  //out->y = 0;
  //out->mon = 0;
  //out->d = 0;
  //p_out->value = 0;
  memset(p_out, 0, sizeof(utc_time_t));

  s = (p_in[i] & 0xF0);
  s = (s >> 4) * 10;
  p_out->hour += s;
  s = (p_in[i++] & 0xF);
  p_out->hour += s;

  //Min
  s = (p_in[i] & 0xF0);
  s = (s >> 4) * 10;
  p_out->minute += s;
  s = (p_in[i] & 0xF);
  p_out->minute += s;

  //Second
  i++;
  s = (p_in[i] & 0xF0);
  s = (s >> 4) * 10;
  p_out->second += s;
  s = (p_in[i] & 0xF);
  p_out->second += s;
}


void MT_EitAdjusTimeZone(mt_s8 time_zone, utc_time_t *utc_time)
{

    utc_time_t *p_tm = utc_time;

    if((p_tm->hour + time_zone) >= 24)
      {

        p_tm->hour = p_tm->hour + time_zone - 24; //调整时区
          p_tm->day += 1;
      }
    else if((p_tm->hour + time_zone) <= 0)
      {
          p_tm->hour = p_tm->hour + time_zone + 24; //调整时区
          p_tm->day -= 1;
      }
    else
      {
          p_tm->hour += time_zone;
      }


}


static mt_s32 MT_EiTDateTrans(mt_u8 *p_in, utc_time_t *p_out)
{
  mt_u16 mjd = 0;
  mt_u16 m = 0;
  mt_u16 d = 0;
  mt_u16 y = 0;
  mt_u16 k = 0;
  mt_u8 i = 0;
  utc_time_t *p_tm = NULL;
  mt_u8 p = 0;
  mt_u32 tmp = 0;
  mt_u32 tmp2 = 0;
  MT_S32 ret = SUCCESS;

  p_tm = p_out;
  memset(p_tm, 0, sizeof(utc_time_t));

  /*!
     Change MJD format year, month and day into Hex format
    */
  mjd = (p_in[i++] << 8);
  mjd += p_in[i++];
  mjd &= 0xFFFF;

  y = (mjd * 100 - 1507820) / 36525;
  tmp = y * 36525/100;

  //m = mjd - 14956.1 - tmp;
  m = (mjd - 14956.1 - tmp) * 10000/306001;
  //m = ((mjd * 100 - 1495610) - y * 36525) * 100 / 306001;

  tmp = y * 36525 / 100; //float to u32
  tmp2 = (m * 306001) / 10000;
  d = mjd - 14956 - tmp - tmp2;

  if(m == 14 || m == 15)
  {
    k = 1;
  }
  else
  {
    k = 0;
  }

  /*year*/
  y += k;
  m -= (k * 12 + 1);

  if(y >= 200)//To spec
  {
    return ERR_FAILURE;
  }
  p_tm->year = (u16)((0xFF & y) + 1900);


  //Month
  p_tm->month = (u8)(0xFF & m);

  //Day
  p_tm->day = (u8)(0xFF & d);

  //Change BCD format EPG into Hex

  //Hour
  p = (p_in[i] & 0xF0);
  p = (p >> 4) * 10;
  p_tm->hour += p;

  p = (p_in[i++] & 0xF);
  p_tm->hour += p;

  //Min
  p = (p_in[i] & 0xF0);
  p = (p >> 4) * 10;
  p_tm->minute += p;
  p = (p_in[i] & 0xF);
  p_tm->minute += p;

  //Second
  i++;

  p = (p_in[i] & 0xF0);
  p = (p >> 4) * 10;
  p_tm->second += p;

  //(tm->t).m+= p ;
  p = (p_in[i] & 0xF);
  p_tm->second += p;

  return ret;
}


static RET_CODE MT_EiTParseContDesc(cont_desc_t *p_eit_cont, mt_u16 event_id, mt_u8 *p_buf)
{
  //Content level
  p_eit_cont->cont_level = p_buf[0];
  p_buf ++;

  //User nibble
  p_eit_cont->usr_nib    = p_buf[0];
  p_eit_cont->evt_id     = event_id;
  p_buf ++;

  return SUCCESS;
}

static RET_CODE MT_EiTParseShtEvt(sht_evt_desc_t *p_sht_desc, sht_evt_txt_t *p_txt, mt_u16 event_id, mt_u8 *p_buf)
{
  mt_u16 event_name_len = 0;
  mt_u16 txt_len = 0;

  //Event name length
  event_name_len = (u16)p_buf[0];
  p_buf ++;

  //Event name information
  if(event_name_len > MAX_EVT_NAME_LEN)
  {
    p_txt->name_len = MAX_EVT_NAME_LEN;
  }
  else
  {
    p_txt->name_len = event_name_len;
  }

  p_txt->evt_id = p_sht_desc->evt_id;
  memcpy((u8 *)p_txt->evt_nm, p_buf, p_txt->name_len);
  p_buf += event_name_len;

  //Event text length
  txt_len = p_buf[0];
  if(txt_len == 0)
  {
    return SUCCESS;
  }

  p_buf ++;

  if(txt_len > MAX_SHT_TEXT_LEN)
  {
    p_txt->txt_len = MAX_SHT_TEXT_LEN;
  }
  else
  {
    p_txt->txt_len = txt_len;
  }

  //Event txt
  memcpy(p_txt->txt, p_buf, p_txt->txt_len);
  p_buf += txt_len;

  //Set event id to text information
  p_txt->evt_id = event_id;

  //Add \0 to string end
  if(p_txt->name_len  > MAX_EVT_NAME_LEN)
  {
    p_txt->name_len = MAX_EVT_NAME_LEN;
  }

  return SUCCESS;
}

static RET_CODE MT_EiTParseExtDesc(ext_evt_desc_t *p_ext_desc, ext_evt_txt_t *p_ext_txt, u16 event_id, mt_u8 *p_buf)
{
  mt_u8 item_desc_len = 0;
  mt_u8 item_len = 0;
  mt_u8  item_num  = 0;
//  u32 item_cont_len = 0;
  mt_u32 txt_len = 0;
  mt_u8 len_of_items = 0;

  //Set total event description number
  p_ext_desc->tot_ext_desc = SYS_GET_LOW_HALF_BYTE(p_buf[0]);
  p_ext_desc->index = SYS_GET_HIGH_HALF_BYTE(p_buf[0]) >> 4;
  p_ext_desc->evt_id = event_id;
  p_buf ++;

  if(p_ext_desc->index > p_ext_desc->tot_ext_desc)
  {
    return ERR_FAILURE;
  }
  //Get language code
  memcpy(p_ext_desc->lang_code, p_buf, LANGUAGE_CODE_LEN * sizeof(u8));
  memcpy(p_ext_txt->lang_code, p_buf, LANGUAGE_CODE_LEN * sizeof(u8));
  p_buf += LANGUAGE_CODE_LEN * sizeof(u8);

  len_of_items = p_buf[0];
  p_buf ++;


 //Parse item content loop
  while(len_of_items > 0)
  {
    //Item name length
    item_desc_len = p_buf[0];
    len_of_items -- ;
    p_buf ++;

    if(item_desc_len > len_of_items)
    {
      return ERR_FAILURE;
    }

    //Item name information
    if(item_desc_len > 0)
    {
      if(item_num < MAX_ITEM_NUM)
      {
        if(item_desc_len < MAX_ITEM_DESC_LEN)
        {
          memcpy(p_ext_desc->item[item_num].item_name, p_buf, item_desc_len);
        }
        else
        {
          memcpy(p_ext_desc->item[item_num].item_name, p_buf, MAX_ITEM_DESC_LEN);
        }
      }
      p_buf += item_desc_len;

//      CHECK_FAIL_RET_CODE(len_of_items >= item_desc_len);
      len_of_items -= item_desc_len;
    }

    if(len_of_items == 0)
    {
      return ERR_FAILURE;
    }
    //Item cont len
    item_len = p_buf[0];
    len_of_items --;
    p_buf ++;

    if(item_len > len_of_items)
    {
      return ERR_FAILURE;
    }

    if(item_len > 0)
    {
      //Item cont information
      if(item_num < MAX_ITEM_NUM)
      {
        if(item_len < MAX_ITEM_CONT_LEN)
        {
          memcpy(p_ext_desc->item[item_num].item_cont, p_buf, item_len);
        }
        else
        {
          memcpy(p_ext_desc->item[item_num].item_cont, p_buf, MAX_ITEM_CONT_LEN);
        }
        item_num ++;
      }

      p_buf += item_len;
//      CHECK_FAIL_RET_CODE(len_of_items >= item_len);
      len_of_items -= item_len;
    }
  }

//  CHECK_FAIL_RET_CODE(len_of_items == 0);

  //Set total item number in current extend event desc
  p_ext_desc->item_num = item_num;

  //Set text information
  txt_len = p_buf[0];
  if(txt_len == 0)
  {
    return ERR_FAILURE;
  }

  p_buf ++;

  if(txt_len > MAX_EXT_TXT_LEN)
  {
    txt_len = MAX_EXT_TXT_LEN;
  }

  //if(txt_len > MAX_EXT_TXT_LEN)
  {
    p_ext_txt->txt_len = txt_len;
  }

  //Set 0 to the end of txt string
  memcpy(p_ext_txt->ext_txt, p_buf, txt_len * sizeof(u8));
  p_ext_txt->evt_id =  event_id;
  return SUCCESS;
}

static void MT_EiTParseTimeShiftedEvtDes(mt_u8 *p_buf, time_shifted_evt_descr_t *p_des)
{
  p_des->svc_id = MAKE_WORD2(p_buf[1], p_buf[2]);
  p_des->evt_id = MAKE_WORD2(p_buf[3], p_buf[4]);
}


/*! This API is added by Chandler */
static mt_s32 MT_EitParseEIT(mt_u8 *p_buf_addr, TS_Eit_T *p_eit_info)
{
  mt_u16    cnt     = 0 ;
  mt_u16    lenth   = 0 ;
  mt_u16    desloop = 0 ;
  mt_u16    size    = 0 ;
  mt_u8     *p_inbuf  = NULL;

  mt_u8     tot_evt_num = 0;
  mt_u8     tot_ext_info_num = 0;
  mt_u8     tot_cont_num = 0;
  mt_u8     tot_sht_txt_num = 0;
  mt_u8     tot_ext_txt_num = 0;

  u16    event_id = 0;
  sht_evt_desc_t *p_eit_desc = NULL;
  time_shifted_evt_descr_t *p_time_shifted_evt_des = NULL;
  MT_BOOL is_err = FALSE;

//  CHECK_FAIL_RET_ZERO(p_buf_addr != NULL);
  p_inbuf = p_buf_addr;

  /*!
    Begin to parse eit schedule section
    */
//  CHECK_FAIL_RET_ZERO(p_eit_info != NULL);
  p_eit_info->tot_evt_num = 0;

  //Table id
  p_eit_info->table_id = p_inbuf[cnt];
  cnt++;
  //Section length
  p_eit_info->sec_length = SYS_GET_LOW_HALF_BYTE(p_inbuf[cnt++])<<8;
  p_eit_info->sec_length += p_inbuf[cnt++] ;
  p_eit_info->sec_length &= 0xFFF ;

  p_eit_info->tot_evt_num = 0;
  p_eit_info->tot_ext_txt_num = 0;
  p_eit_info->tot_sht_txt_num = 0;
  p_eit_info->tot_ext_info_num = 0;


  p_eit_info->svc_id  = p_inbuf[cnt++]<<8 ;
  p_eit_info->svc_id += p_inbuf[cnt++];

  p_eit_info->version = (p_inbuf[cnt] >> 1) & 0x1f;
  p_eit_info->pf_flag = MASK_FIRST_BIT_OF_BYTE(p_inbuf[cnt++]) ;

  p_eit_info->section_number = p_inbuf[cnt++] ;
  p_eit_info->last_section_number = p_inbuf[cnt++] ;

  p_eit_info->stream_id = p_inbuf[cnt++]<<8 ;
  p_eit_info->stream_id += p_inbuf[cnt++];

  p_eit_info->org_nw_id = p_inbuf[cnt++] << 8 ;
  p_eit_info->org_nw_id += p_inbuf[cnt++];

  //Segment last section number
  p_eit_info->seg_last_sec_number = p_inbuf[cnt];
  cnt++;

  p_eit_info->last_table_id       = p_inbuf[cnt];
  cnt++;

 if(p_eit_info->sec_length < EIT_MIN_LEN)
  {
    /*!
     Section length is too short
     */
    //mdl_dbg(("Invalid section\n"));
    return 0;
  }

  lenth = (p_eit_info->sec_length - 11 - CRC_SIZE);
//  MT_EIT_PRINT("ServiceID: 0x%x\n", p_eit_info->svc_id);

  while(lenth > 0)
  {
    is_err = FALSE;
    if(tot_evt_num >= MAX_EVT_PER_SEC)
    {
      break;
    }
    //Get current short event desc
    p_eit_desc = &p_eit_info->sht_evt_info[tot_evt_num];

    if(lenth < EIT_SEC_HEAD_LEN)
    {
      break;
    }

    //Get event id
    p_eit_desc->evt_id  = (p_inbuf[cnt++] << 8);
    p_eit_desc->evt_id +=  p_inbuf[cnt++];
    event_id = p_eit_desc->evt_id;

    //Start time
    MT_EiTDateTrans(&p_inbuf[cnt], &p_eit_desc->st_tm);
    cnt += 5;

    //Drt time
    MT_EiTimeTrans(&p_inbuf[cnt], &p_eit_desc->drt_time);
    cnt += 3;

    //running status
    p_eit_desc->running_status = ((p_inbuf[cnt] & 0xE0) >> 5);

    desloop = SYS_GET_LOW_HALF_BYTE(p_inbuf[cnt++]) << 8;
    desloop += p_inbuf[cnt++];
    desloop &= 0xFFF;
    size = desloop ;

//  MT_EIT_PRINT("EvtID: 0x%4x\n", p_eit_desc->evt_id);
//      pst_time = &p_eit_desc->st_tm;
//  MT_EIT_PRINT("StartTime: %d-%d-%d %d:%d:%d \n", pst_time->year,pst_time->month, pst_time->day, pst_time->hour, pst_time->minute,pst_time->second);
//  pdrt_time = &p_eit_desc->drt_time;
//    MT_EIT_PRINT("Duration: %d:%d:%d \n", pdrt_time->hour, pdrt_time->minute,pdrt_time->second);

//    MT_EIT_PRINT("RunSata: 0x%x\n",p_eit_desc->running_status);
    if(size > (lenth - EIT_SEC_HEAD_LEN))
    {
      break;
    }

    while(desloop > 0)
    {
      switch(p_inbuf[cnt])
      {
        case DVB_DESC_CONTENT:
          {
            cnt++;

            if(p_inbuf[cnt] > (desloop - 2))
            {
              desloop = 0;
              memset((u8 *)p_eit_desc, 0, sizeof(sht_evt_desc_t));
              //p_eit_info->tot_evt_num  --;
              break;
            }

            //CHECK_FAIL_RET_ZERO(tot_cont_num < MAX_EVT_PER_SEC);
            if(tot_cont_num < MAX_EVT_PER_SEC)
            {
              MT_EiTParseContDesc(&p_eit_info->cont_desc[tot_cont_num],
                      event_id, &p_inbuf[cnt + 1]);
            }
            else
            {
              break;
            }
            tot_cont_num ++;
          }
          break;
        case DVB_DESC_SHORT_EVENT: //0x4d
          {
            sht_evt_txt_t *p_cur_txt = NULL;
            cnt++;

            //CHECK_FAIL_RET_ZERO(tot_sht_txt_num < MAX_SHT_TXT_NUM);

            if(tot_sht_txt_num >= MAX_SHT_TXT_NUM)
            {
              break;
            }
            p_cur_txt = &p_eit_info->sht_txt[tot_sht_txt_num];

            if(p_inbuf[cnt] > (desloop - 2))
            {
              desloop = 0;
              //p_eit_info->evt_cnt--;
              break;
            }

            //Set language code
            memcpy(p_cur_txt->lang_code, &p_inbuf[cnt + 1],
                                             LANGUAGE_CODE_LEN * sizeof(u8));

            if(MT_EiTParseShtEvt(p_eit_desc, p_cur_txt, event_id, &p_inbuf[cnt + 4]) == SUCCESS)
            {
              tot_sht_txt_num ++;
            }
//          MT_EIT_PRINT("EvtName: %s(%d) \n", p_cur_txt->evt_nm, p_cur_txt->name_len);
          }
          break;
        case DVB_DESC_COMPONENT_DESC:
          {
            cnt++;

            if(p_inbuf[cnt] > (desloop - 2))
            {
              desloop = 0;
              memset((u8 *)p_eit_desc, 0, sizeof(sht_evt_desc_t));
              //p_eit_info->evt_cnt --;
              break;
            }

            //Copy language code out
            //memcpy(p_eit_desc->lang_code, &p_inbuf[cnt + 4],
            //LANGUAGE_CODE_LEN * sizeof(u8));
          }
          break;
        case DVB_DESC_EXT_EVENT:
          {
            ext_evt_desc_t *p_cur_desc = NULL;
            ext_evt_txt_t *p_cur_txt = NULL;

            /*!
              Skip tag
              */
            cnt++;

            //CHECK_FAIL_RET_ZERO(tot_ext_txt_num < MAX_EXT_TXT_NUM);
            if(tot_ext_txt_num >= MAX_EXT_TXT_NUM)
            {
              break;
            }
            p_cur_txt = &p_eit_info->ext_txt[tot_ext_txt_num];

            //CHECK_FAIL_RET_ZERO(tot_ext_info_num < MAX_EXT_DESC_NUM);
            if(tot_ext_info_num >= MAX_EXT_DESC_NUM)
            {
              break;
            }
            p_cur_desc  = &p_eit_info->ext_evt_info[tot_ext_info_num];

            if(p_inbuf[cnt] > (desloop - 2))
            {
              desloop = 0;
              memset((u8 *)p_eit_desc, 0, sizeof(sht_evt_desc_t));
              //p_eit_info->evt_cnt --;
              break;
            }

            MT_EiTParseExtDesc(p_cur_desc, p_cur_txt, event_id, &p_inbuf[cnt + 1]);
            if(p_cur_desc->item_num > 0)
            {
              tot_ext_info_num ++;
            }

            if(p_cur_txt->txt_len > 1)
            {
              tot_ext_txt_num ++;
            }
          }
          break;
        case DVB_DESC_TIME_SHIFTED_EVT:
          {
            cnt ++;
            p_eit_desc->time_shifted_evt_des_found = 1;
            p_time_shifted_evt_des = &p_eit_desc->time_shifted_evt_des;
            MT_EiTParseTimeShiftedEvtDes(&p_inbuf[cnt], p_time_shifted_evt_des);
          }
          break;
#if 1
        case DVB_DESC_PARENTAL_RATING:
          {
            cnt++;
             if(p_inbuf[cnt] > (desloop - 2))
              {
                desloop = 0;
                //memset((u8*)p_eit_desc, 0, sizeof(sht_evt_desc_t));
                break;
              }
            memcpy(p_eit_desc->parental_rating.country_code, &p_inbuf[cnt + 1],
                                             LANGUAGE_CODE_LEN * sizeof(u8));
            p_eit_desc->parental_rating.rating = p_inbuf[cnt + 4];
          }
          break;
#endif
        default:
          {
            //Skip tag
            cnt++;
            if(p_inbuf[cnt] > (desloop - 2))
            {
              desloop = 0;
              //p_eit_info->evt_cnt--;
              break;
            }
          }
          break;
      }
      if(0 == desloop)
     {
        is_err = TRUE;
        break;
    }
      desloop -= (p_inbuf[cnt] + 2);
      cnt += (p_inbuf[cnt]+ 1);
    }

    desloop = 0;
    lenth -= (12 + size);
    if(TRUE != is_err)
    {
        tot_evt_num++;
    }
  }
  p_eit_info->tot_cont_num = tot_cont_num;
  p_eit_info->tot_evt_num = tot_evt_num;
  p_eit_info->tot_ext_txt_num = tot_ext_txt_num;
  p_eit_info->tot_sht_txt_num = tot_sht_txt_num;
  p_eit_info->tot_ext_info_num = tot_ext_info_num;

  return p_eit_info->sec_length;
}

static MT_VOID MT_EITPrintTable(TS_Eit_T *pEiTb, MT_U8 size)

{
    MT_U32 i = 0, j = 0;
    utc_time_t *pst_time = NULL;
    utc_time_t *pdrt_time = NULL;

    SAMPLE_EIT_FUNCTION_ENTER();

    MT_EIT_PRINT("ServiceID: 0x%x\n", pEiTb[0].svc_id);
    for(i = 0; i < size; i++)
    {
        SAMPLE_EIT_INFO_PRINT("-------------------------------\n");
        for(j =0; j<pEiTb[i].tot_evt_num; j++)
        {
            pst_time = &(pEiTb[i].sht_evt_info[j].st_tm);
            MT_EitAdjusTimeZone(MT_EIT_TIME_ZONE_EAST8, pst_time);

            MT_EIT_PRINT("EvtID: 0x%4x\n", pEiTb[i].sht_evt_info[j].evt_id);
            MT_EIT_PRINT("StartTime: %d-%d-%d %d:%d:%d \n", pst_time->year,pst_time->month, pst_time->day, pst_time->hour, pst_time->minute,pst_time->second);
            pdrt_time = &(pEiTb[i].sht_evt_info[j].drt_time);
            MT_EIT_PRINT("Duration: %d:%d:%d \n", pdrt_time->hour, pdrt_time->minute,pdrt_time->second);

            MT_EIT_PRINT("RunSata: 0x%x\n",pEiTb[i].sht_evt_info[j].running_status);
            MT_EIT_PRINT("EvtName: %s\n\n", pEiTb[i].sht_txt[j].evt_nm);
        }
    }

    SAMPLE_EIT_INFO_PRINT("======================================\n");

    SAMPLE_EIT_FUNCTION_EXIT();

}

static MT_S32 MT_EitScheduleSectionRead(MT_HANDLE hChannel, MT_U32 u32TimeOutms, MT_U8 *pBuf, MT_U32 *pAcquiredNum)
{
    MT_U8             u8tableid = 0;
    MT_S32            ret = MT_FAILURE;
    MT_U32            num = 0;
    MT_U32            u32Times = 0;
    MT_U32            u32SecTotalNum = 0;
    MT_U32            u32SecNum = 0;
    MT_U32            RequestNum = 32;
    mod_info_t        mod_info = {0};
    MT_UNF_DMX_DATA_S sSection[32];

    SAMPLE_EIT_FUNCTION_ENTER();

    if(MT_INVALID_HANDLE == hChannel)
    {
        SAMPLE_EIT_ERR_PRINT("The input handle is NULL. \n");
        return MT_FAILURE;
    }

    memset(&mod_info, 0, sizeof(mod_info_t));
    u32Times = 1000;

    while(u32Times--)
    {
        ret = MT_UNF_DMX_CheckDataHandle(hChannel, (MT_U32)100);
        if (MT_SUCCESS != ret)
        {
            if(0 == u32Times % 50)
            {
//              SAMPLE_EIT_ERR_PRINT("The channel no data. \n");
                MT_USLEEP(3 * 1000);
            }
            continue;
        }
        /** Obtains the received data packets from a specific channel */
        ret = MT_UNF_DMX_AcquireBuf(hChannel, RequestNum, &num, sSection, u32TimeOutms);
        if(MT_SUCCESS != ret)
        {
            continue;
        }

        for(MT_U32 i = 0; i < num; i++)
        {
            if(sSection[i].enDataType == MT_UNF_DMX_DATA_TYPE_WHOLE)
            {
                u8tableid = sSection[i].pu8Data[0];
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
                }
            }
            else
            {
                SAMPLE_EIT_ERR_PRINT("Section data type is not MT_UNF_DMX_DATA_TYPE_WHOLE\n");

                continue;
            }

//          SAMPLE_EIT_INFO_PRINT("filt: 0x%x u32Size: %d Tid: 0x%x  s: %d l: %d\n", sSection[i].filthandle, sSection[i].u32Size, u8tableid, u32SecNum, u32SecTotalNum);

            if(mod_info.start == MT_FALSE)
            {
                mod_info.start = MT_TRUE;
                mod_info.max_section_number = u32SecTotalNum;
            }

            if(mod_info.section_info[u32SecNum].is_received != 1)
            {
                memcpy((MT_VOID *)(pBuf + u32SecNum * 4096), sSection[i].pu8Data, sSection[i].u32Size);
                mod_info.section_info[u32SecNum].is_received = 1;
                mod_info.recv_number++;
            }


            if(mod_info.recv_number == mod_info.max_section_number)
            {
                *pAcquiredNum = mod_info.recv_number;
                /** Releases the buffers for storing data packets after data packets are processed */
                ret = MT_UNF_DMX_ReleaseBuf(hChannel, num, sSection);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_EIT_ERR_PRINT("MT_UNF_DMX_ReleaseBuf failed, ret= 0x%x \n", ret);

                    return ret;
                }

                SAMPLE_EIT_FUNCTION_EXIT();

                return MT_SUCCESS;
            }


        }

        /** Releases the buffers for storing data packets after data packets are processed */
        ret = MT_UNF_DMX_ReleaseBuf(hChannel, num, sSection);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_EIT_ERR_PRINT("MT_UNF_DMX_ReleaseBuf failed, ret= 0x%x \n", ret);

            return ret;
        }

    }

    SAMPLE_EIT_ERR_PRINT("MT_UNF_DMX_AcquireBuf u32Times out.\n");

    return MT_FAILURE;
}

static mt_s32 MT_EitAcquireEitScheduleSection(mt_u16 ServiceID, mt_u8 TableID)
{
    mt_u8 u8Match[DMX_FILTER_MAX_DEPTH] = { 0 };
    mt_u8 u8Mask[DMX_FILTER_MAX_DEPTH] = { 0 };
    mt_u8 u8Negate[DMX_FILTER_MAX_DEPTH] = { 0 };
    mt_s32 s32Ret = MT_FAILURE;
    mt_handle hChan = MT_NULL;
    mt_handle hFilter = MT_NULL;
    MT_UNF_DMX_CHAN_ATTR_S stChanAttr= { 0 };
    MT_UNF_DMX_FILTER_ATTR_S stFilterAttr = { 0 };
    mt_u8 *u8DataBuf = NULL;
    MT_U32 u32AquiredNum = 0;
    MT_U8 *p = NULL;
    TS_Eit_T *peittb = { 0 };
    MT_U32 i = 0;

    /** Creates a PID channel based on channel attributes */
    stChanAttr.u32BufSize = SECITON_BUF_SIZE;
    stChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    stChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
    stChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    s32Ret = MT_UNF_DMX_CreateChannel(DMX_ID_0, &stChanAttr, &hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Create channel error!\n");
        return s32Ret;
    }

    /** Sets the PID of a channel */
    s32Ret = MT_UNF_DMX_SetChannelPID(hChan, EIT_TSPID);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Set pid error!\n");
        goto FREE_CHANNEL;
    }

    memset(u8Match, 0, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(u8Mask, 0xff, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(u8Negate, 0, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));


    u8Match[0] = TableID;
    u8Mask[0] = 0x00;

    u8Match[1] = ServiceID >> 8;
    u8Mask[1]   = 0x00;

    u8Match[2] = ServiceID & 0x000000ff;
    u8Mask[2]   = 0x00;

    stFilterAttr.u32FilterDepth = 3;
    memcpy(stFilterAttr.au8Match, u8Match, DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Mask, u8Mask, DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Negate, u8Negate, DMX_FILTER_MAX_DEPTH);

    /** Creates a data filter */
    s32Ret = MT_UNF_DMX_CreateFilter(DMX_ID_0, &stFilterAttr, &hFilter);
    if(MT_SUCCESS != s32Ret)
    {
        goto FREE_CHANNEL;
    }
    /** Sets the filter criteria of a filter */
    s32Ret = MT_UNF_DMX_SetFilterAttr(hFilter, &stFilterAttr);

    /** Attaches filters to a specific channel */
    s32Ret |= MT_UNF_DMX_AttachFilter(hFilter, hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Attach filter error!\n");
        goto FREE_FILTER;
    }

    /** Enables a channel */
    s32Ret = MT_UNF_DMX_OpenChannel(hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Open channel error!\n");
        goto DETCH_FILTER;
    }

    SAMPLE_EIT_INFO_PRINT("Table[0x%x], ServiceID[0x%x], Open filer[0x%lx] on channel[0x%lx].\n",TableID, ServiceID, hFilter, hChan);

    //system("cat /proc/msp/demux_filter");

    u8DataBuf = malloc(MAX_SECTION_LEN * MAX_SECTION_NUM);
    if(NULL == u8DataBuf)
    {
        goto DETCH_FILTER;

    }

    memset(u8DataBuf, 0, sizeof(MAX_SECTION_LEN * MAX_SECTION_NUM));

    /** Read and print the section's data */
    s32Ret = MT_EitScheduleSectionRead(hChan, 100, u8DataBuf, &u32AquiredNum);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("No data from 0x%x was obtained\n", ServiceID);
        goto DETCH_FILTER;
    }

    peittb = malloc(sizeof(TS_Eit_T) * u32AquiredNum);
    memset(peittb, 0, sizeof(TS_Eit_T) * u32AquiredNum);

    p = u8DataBuf;
    for(i = 0; i < u32AquiredNum; i++)
    {
        MT_EitParseEIT(p, &peittb[i]);
        p = p + 4096;
    }

    /* show EPG */
    (MT_VOID)MT_EITPrintTable(peittb, u32AquiredNum);

    /** Disables a channel */
    s32Ret = MT_UNF_DMX_CloseChannel(hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Close channel error!\n");
    }

DETCH_FILTER:
    (MT_VOID)MT_UNF_DMX_DetachFilter(hFilter, hChan);
FREE_FILTER:
    (MT_VOID)MT_UNF_DMX_DestroyFilter(hFilter);
FREE_CHANNEL:
    free(u8DataBuf);
    (MT_VOID)MT_UNF_DMX_DestroyChannel(hChan);
    return s32Ret;
}


static mt_s32 MT_EitAcquireEitSection(mt_u16 ServiceID)
{
    mt_u8 u8Match[DMX_FILTER_MAX_DEPTH] = { 0 };
    mt_u8 u8Mask[DMX_FILTER_MAX_DEPTH] = { 0 };
    mt_u8 u8Negate[DMX_FILTER_MAX_DEPTH] = { 0 };
    mt_s32 s32Ret = MT_FAILURE;
    mt_handle hChan = MT_NULL;
    mt_handle hFilter = MT_NULL;
    MT_UNF_DMX_CHAN_ATTR_S stChanAttr= { 0 };
    MT_UNF_DMX_FILTER_ATTR_S stFilterAttr = { 0 };
    mt_u8 *u8DataBuf = NULL;
    MT_U32 u32AquiredNum = 0;
    MT_U8 *p = NULL;
    TS_Eit_T eittb[2] = { 0 };

    /** Creates a PID channel based on channel attributes */
    stChanAttr.u32BufSize = SECITON_BUF_SIZE;
    stChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    stChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
    stChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    s32Ret = MT_UNF_DMX_CreateChannel(DMX_ID_0, &stChanAttr, &hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Create channel error!\n");
        return s32Ret;
    }

    /** Sets the PID of a channel */
    s32Ret = MT_UNF_DMX_SetChannelPID(hChan, EIT_TSPID);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Set pid error!\n");
        goto FREE_CHANNEL;
    }

    memset(u8Match, 0, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(u8Mask, 0xff, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(u8Negate, 0, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));


    u8Match[0] = EIT_TABLE_ID_PF_ACTUAL;
    u8Mask[0] = 0;

    u8Match[1] = ServiceID >> 8;
    u8Mask[1]   = 0x00;

    u8Match[2] = ServiceID & 0x000000ff;
    u8Mask[2]   = 0x00;

    stFilterAttr.u32FilterDepth = 3;
    memcpy(stFilterAttr.au8Match, u8Match, DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Mask, u8Mask, DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Negate, u8Negate, DMX_FILTER_MAX_DEPTH);

    /** Creates a data filter */
    s32Ret = MT_UNF_DMX_CreateFilter(DMX_ID_0, &stFilterAttr, &hFilter);
    if(MT_SUCCESS != s32Ret)
    {
        goto FREE_CHANNEL;
    }
    /** Sets the filter criteria of a filter */
    s32Ret = MT_UNF_DMX_SetFilterAttr(hFilter, &stFilterAttr);

    /** Attaches filters to a specific channel */
    s32Ret |= MT_UNF_DMX_AttachFilter(hFilter, hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Attach filter error!\n");
        goto FREE_FILTER;
    }

    /** Enables a channel */
    s32Ret = MT_UNF_DMX_OpenChannel(hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Open channel error!\n");
        goto DETCH_FILTER;
    }

    u8DataBuf = malloc(MAX_SECTION_LEN * MAX_SECTION_NUM);
    if(NULL == u8DataBuf)
    {
        goto DETCH_FILTER;
    }

    memset(u8DataBuf, 0, sizeof(MAX_SECTION_LEN * MAX_SECTION_NUM));

    /** Read and print the section's data */
    s32Ret = MT_EitScheduleSectionRead(hChan, 2500, u8DataBuf, &u32AquiredNum);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("No data from 0x%x was obtained\n", ServiceID);
        goto DETCH_FILTER;
    }

    SAMPLE_EIT_INFO_PRINT("u32AquiredNum: %d \n", u32AquiredNum);

    p = u8DataBuf;
    for(MT_S32 i = 0; i < u32AquiredNum; i++)
    {
        MT_EitParseEIT(p, &eittb[i]);
        p = p + 4096;
    }

    (MT_VOID)MT_EITPrintTable(eittb, u32AquiredNum);

    /** Disables a channel */
    s32Ret = MT_UNF_DMX_CloseChannel(hChan);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Close channel error!\n");
    }

DETCH_FILTER:
    (MT_VOID)MT_UNF_DMX_DetachFilter(hFilter, hChan);
FREE_FILTER:
    (MT_VOID)MT_UNF_DMX_DestroyFilter(hFilter);
FREE_CHANNEL:
    free(u8DataBuf);
    (MT_VOID)MT_UNF_DMX_DestroyChannel(hChan);
    return s32Ret;
}



static MT_VOID MT_EitModePrintMenu(MT_U32 prog_num)
{
    SAMPLE_EIT_PRINT("\n 1 - %d : select the program \n", prog_num);

    SAMPLE_EIT_PRINT("     h : help \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_EIT_PRINT("     b : background run \n");
#endif
    SAMPLE_EIT_PRINT("     y : current program schedule \n");
    SAMPLE_EIT_PRINT("     a : all program schedules \n");
    SAMPLE_EIT_PRINT("     i : the following two moments of the program \n");
    SAMPLE_EIT_PRINT("     q : quit \n");
    SAMPLE_EIT_PRINT("EIT>> ");
}


/*!
@brief The thread on which the command was entered
@param[in]  hAvplay             Handle to AV player
@param[in]  pProgTbl            The data structure of the PMT
@return::MT_VOID
@*/
static MT_VOID MT_EitCmdTask(PMT_COMPACT_TBL *pProgTbl)
{
    MT_S32             ret = MT_FAILURE;
    MT_U32             u32ProgNum = 1;
    MT_U8              tableID = 0;
    MT_CHAR            inputCmd[32] = { 0 };
    PMT_COMPACT_PROG   *pstCurrentProgInfo = NULL;

    pstCurrentProgInfo = pProgTbl->proginfo;
    while(1)
    {
        (MT_VOID)MT_EitModePrintMenu(pProgTbl->prog_num);

        fgets((MT_CHAR *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_EIT_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_EIT_PRINT("Dvbc play in back!\n");
            break;
        }
#endif
        else  if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            u32ProgNum = atoi(inputCmd);

            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                pstCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum - 1) % pProgTbl->prog_num);

                SAMPLE_EIT_PRINT("The current program is: %d serviceID : 0x%x \n", u32ProgNum, pstCurrentProgInfo->ProgID);
                continue;
            }
            SAMPLE_EIT_ERR_PRINT("Out of the program, please enter the program: 1-%d\n", pProgTbl->prog_num);
        }
        else if('y' == inputCmd[0])
        {
            tableID = 0x50;

            SAMPLE_EIT_PRINT("------------Current program schedule------------\n");
            SAMPLE_EIT_PRINT("Searching...\n");
            for(; tableID <= 0x51; tableID++)
            {
                ret = MT_EitAcquireEitScheduleSection(pstCurrentProgInfo->ProgID, tableID);
                if (MT_SUCCESS != ret)
                {
                    break;
                }
            }
        }
        else if('a' == inputCmd[0])
        {
            SAMPLE_EIT_PRINT("------------All program schedules------------\n");
            for(int j = 0; j < pProgTbl->prog_num; j++)
            {
                tableID = 0x50;
                SAMPLE_EIT_PRINT("Timeline of Program %d, ServiceID: 0x%x \n", j + 1, pProgTbl->proginfo[j].ProgID);
                SAMPLE_EIT_PRINT("Searching...\n");
                for(; tableID <= 0x51; tableID++)
                {
                    ret = MT_EitAcquireEitScheduleSection(pProgTbl->proginfo[j].ProgID, tableID);
                    if (MT_SUCCESS != ret)
                    {
                        break;
                    }
                }
                SAMPLE_EIT_PRINT("--------------------------------------\n");
            }
        }
        else if('i' == inputCmd[0])
        {
            SAMPLE_EIT_PRINT("------------The following two moments of the program------------\n");
            SAMPLE_EIT_PRINT("Searching...\n");
            ret = MT_EitAcquireEitSection(pstCurrentProgInfo->ProgID);
            if(MT_SUCCESS != ret)
            {
                continue;
            }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_EIT_INFO_PRINT("Print help info \n");
            continue;
        }
    }

}


static MT_VOID MT_EitPrint_Help(MT_CHAR *name)
{
    MT_EIT_PRINT("Lack of parameters\n");
    MT_EIT_PRINT("\nUsage:\n");
    MT_EIT_PRINT("%s\n", name);
    MT_EIT_PRINT("    -f: path of the stream file\n");
    MT_EIT_PRINT("    -c: EIT locks frequency\n");
    MT_EIT_PRINT("    -s: DVBS locks frequency\n");
    MT_EIT_PRINT("example:\n");
    MT_EIT_PRINT("    %s -f ./6ch_voices_id_7_dd_DVB_h264_25fps.trp\n",name);
    MT_EIT_PRINT("    %s -c 314 6875 64\n",name);
    MT_EIT_PRINT("    %s -s 3840 27500 1 0 0\n",name);
#ifdef MT_SAMPLE_APP
    MT_EIT_PRINT("    -q: Exit the background\n");
#endif
}


/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_EitParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

    if(argc < 2 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_EitPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:c:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_EitPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'f':
                if(argc != 3)
                {
                    (MT_VOID)MT_EitPrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_FILE;
                MTADP_Strncpy((mt_char*)pInputParam->input_param.file.file_name, mt_optarg, sizeof(mt_input_file_para_t));
                break;

            case 's':
                if(argc != 7)
                {
                    (MT_VOID)MT_EitPrint_Help(argv[0]);
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
                    (MT_VOID)MT_EitPrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                pInputParam->sig_type = MT_INPUT_SIG_TYPE_CAB;

                pInputParam->input_param.cab.freq = strtol(argv[2], 0, 0);
                pInputParam->input_param.cab.sym_rate= strtol(argv[3], 0, 0);
                pInputParam->input_param.cab.mod_type= strtol(argv[4], 0, 0);
                return MT_SUCCESS;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_EitExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_EitPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}



#ifdef MT_SAMPLE_APP
MT_S32 MT_EitMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32                 s32Ret = MT_SUCCESS;

    s32Ret = MT_EitParase_args(argc, argv, &eit_run_info.sInputParam);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_EIT_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        /** System initialization */
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_EIT_ERR_PRINT("failed to mt_sys_init\n");
            return s32Ret;
        }

        /** HDMI initialization */
        s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_EIT_ERR_PRINT("failed to StartDmx\n");
            goto ERR0;
        }

        sleep(1);

        /** Display initialization */
        s32Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_EIT_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR1;
        }
#endif

        g_bTaskQuit = MT_FALSE;
        if(MT_INPUT_SIG_TYPE_FILE != eit_run_info.sInputParam.sig_type)
        {
            s32Ret = MTADP_Fe_Init(TUNER_ID_0);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_EIT_ERR_PRINT("MTADP_Fe_Init failed. s32Ret = 0x%x\n", s32Ret);
                goto ERR2;
            }

            if (MT_INPUT_SIG_TYPE_CAB == eit_run_info.sInputParam.sig_type)
            {
                s32Ret = MT_EitModeCheckDvbcParam(&eit_run_info.sInputParam.input_param.cab);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_EIT_ERR_PRINT("MT_EitModeCheckDvbcParam failed. s32Ret = 0x%x\n", s32Ret);
                    goto ERR3;
                }

                s32Ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                            eit_run_info.sInputParam.input_param.cab.freq,
                                            eit_run_info.sInputParam.input_param.cab.sym_rate,
                                            eit_run_info.sInputParam.input_param.cab.mod_type);
            }
            else if(MT_INPUT_SIG_TYPE_SAT == eit_run_info.sInputParam.sig_type)
            {
                s32Ret = MT_EitModeCheckDvbsParam(&eit_run_info.sInputParam.input_param.sat);
                if(MT_SUCCESS != s32Ret)
                {
                    SAMPLE_EIT_ERR_PRINT("MT_CCModeCheckDvbsParam failed. s32Ret = 0x%x\n", s32Ret);
                    goto ERR3;
                }

                s32Ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                            eit_run_info.sInputParam.input_param.sat.freq,
                                            eit_run_info.sInputParam.input_param.sat.sym_rate,
                                            eit_run_info.sInputParam.input_param.sat.onoff_22k,
                                            eit_run_info.sInputParam.input_param.sat.polarization,
                                            eit_run_info.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_EIT_ERR_PRINT("MTADP_Fe_Connect failed. s32Ret = 0x%x\n", s32Ret);
                goto ERR3;
            }
        }

        /** DMX module initialization */
        s32Ret = MT_EitModeDmxInit(eit_run_info.sInputParam.sig_type);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_EIT_ERR_PRINT("failed to StartDmx\n");
            goto ERR3;
        }

        if(MT_INPUT_SIG_TYPE_FILE == eit_run_info.sInputParam.sig_type)
        {
            s32Ret = pthread_create(&eit_run_info.stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_EitModeInjectTsTask, &eit_run_info.sInputParam.input_param.file);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_EIT_ERR_PRINT("failed to pthread_create thread.\n");
                goto ERR4;
            }
            sleep(1);
            if(g_bTaskQuit == MT_TRUE)
            {
                goto ERR5;
            }
        }

        (MT_VOID)MTADP_Search_Init();

        /** Get the PMT table */
        s32Ret = MTADP_Search_GetAllPmt(DMX_ID_0, &eit_run_info.pProgTbl);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_EIT_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");

            goto ERR6;
        }

    }

    (MT_VOID)MT_EitCmdTask(eit_run_info.pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }


    (MT_VOID)MTADP_Search_FreeAllPmt(eit_run_info.pProgTbl);
ERR6:
    (MT_VOID)MTADP_Search_DeInit();
ERR5:
    if(MT_INPUT_SIG_TYPE_FILE == eit_run_info.sInputParam.sig_type)
    {
        g_bTaskQuit = MT_TRUE;

        /** Wait for the thread to end */
        (MT_VOID)pthread_join(eit_run_info.stInjectTSThread, NULL);
    }
ERR4:
    /** Demux module deinitialization */
    (MT_VOID)MT_EitModeDmxDeInit();
ERR3:
    if(MT_INPUT_SIG_TYPE_FILE != eit_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR2:
#ifndef MT_SAMPLE_APP
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&eit_run_info, 0xff, sizeof(eit_run_info));

    return s32Ret;
}
