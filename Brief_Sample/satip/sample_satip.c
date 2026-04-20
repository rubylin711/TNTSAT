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
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
#include "pthread.h"
#include "mt_unf_sound.h"
#include "satip_player.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mt_cdlna.h>
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_SATIP_DEBUG
#define MT_SATIP_PRINT   printf
#else
#define MT_SATIP_PRINT
#endif

#define SAMPLE_SATIP_FUNCTION_ENTER()             MT_SATIP_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_SATIP_FUNCTION_EXIT()              MT_SATIP_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_SATIP_FATAL_PRINT(fmt...)          MT_SATIP_PRINT(" [FATAL] " fmt)
#define SAMPLE_SATIP_ERR_PRINT(fmt...)            MT_SATIP_PRINT(" [ERROR] " fmt)
#define SAMPLE_SATIP_WARN_PRINT(fmt...)           MT_SATIP_PRINT(" [WARN] "  fmt)
#define SAMPLE_SATIP_INFO_PRINT(fmt...)           MT_SATIP_PRINT(" [INFO] "  fmt)
#define SAMPLE_SATIP_DBG_PRINT(fmt...)            MT_SATIP_PRINT(" [DEBUG] " fmt)


#define SAMPLE_SATIP_PRINT   printf

#define RECOVER              4
#define DMX_ID_0             0
#define TUNER_ID_0           0
#define MT_TASK_RUN          1
#define MT_TASK_EXIT         2
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
typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
    pthread_t          stInjectTSThread;
    PMT_COMPACT_TBL    *pProgTbl;
    mt_input_para_t    sInputParam;
} MT_SATIP_RUN_INFO;
typedef struct __rec_info
{
    MT_HANDLE rechandle;
    MT_HANDLE chanhandle[8];
    MT_S32 chancnt;
    MT_UNF_DMX_REC_DATA_S recdata;
}rec_info;
/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_SATIP_RUN_INFO satip_run_info;
static MT_VOID *g_satipdev;
/******************************* API declaration *****************************/
/*!
@brief Check if the QAM matches
@param[in]  mod_type            QAM
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Failure.
@*/
static MT_S32 MT_SatipCheckDvbcParam(mt_input_cab_para_t *p_cab_in)
{
    if(p_cab_in->freq < 45 || p_cab_in->freq > 862)
    {
        SAMPLE_SATIP_ERR_PRINT("The frequency is not in range\n");
        return MT_FAILURE;
    }

    if(p_cab_in->sym_rate < 900 || p_cab_in->sym_rate > 7200)
    {
        SAMPLE_SATIP_ERR_PRINT("The symbol rate is not in range\n");
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
            SAMPLE_SATIP_ERR_PRINT("QAM mismatch(16, 32, 64, 128, 256)\n");
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
static mt_s32 MT_SatipCheckDvbsParam(mt_input_sat_para_t *p_sat_in)
{
    if((p_sat_in->freq) > 4200 || (p_sat_in->freq) < 3000)
    {
        SAMPLE_SATIP_ERR_PRINT("freq error. freq = %d \n", p_sat_in->freq);
        SAMPLE_SATIP_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*!
@brief Demux initializes
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 MT_SatipDmxInit(MT_INPUR_SIG_TYPE_T sig_type)
{
    MT_S32           s32Ret = MT_FAILURE;
    mt_sys_version_s stSysChipInfo;

    /** Initializes the demux module */
    s32Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SATIP_ERR_PRINT("MT_UNF_DMX_Init failed, s32Ret = 0x%x\n", s32Ret);
        return s32Ret;
    }

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    s32Ret = mt_sys_get_version(&stSysChipInfo);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SATIP_ERR_PRINT("failed to mt_sys_get_version\n");
        return MT_FAILURE;
    }

    if (MT_INPUT_SIG_TYPE_CAB == sig_type)
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
    else if (MT_INPUT_SIG_TYPE_SAT == sig_type)
    {
        if (MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
        {
            s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_0);
        }
        else
        {
            s32Ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
        }
    }

    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_SATIP_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*!
@brief Demux module deinitialization
@return::MT_VOID
@*/
static MT_VOID MT_SatipDmxDeinit(MT_VOID)
{
    /** Unbind demux from the port */
    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    /** Deinitializes the DEMUX module */
    (MT_VOID)MT_UNF_DMX_DeInit();
}

static MT_VOID dvb_nim_lock(MT_VOID * param)
{
    satip_pg_info_t *prog = (satip_pg_info_t*)param;
    if(!prog)
    {
        return;
    }
    SAMPLE_SATIP_PRINT("lock freq: %d, symb: %d\n", prog->freq, prog->symb_rate);
}

static unsigned long dvb_start_rec(satip_pg_info_t *prog)
{
    MT_S32 ret = 0;
    MT_UNF_DMX_REC_ATTR_S RecAttr;
    MT_S32 i = 0;
    rec_info *prec = (rec_info*)malloc(sizeof(rec_info));
    if(!prec)
    {
        return 0xffffffff;
    }
    memset(prec, 0, sizeof(rec_info));
    memset(&RecAttr, 0 , sizeof(MT_UNF_DMX_REC_ATTR_S));
    RecAttr.u32DmxId = 0;
    RecAttr.u32RecBufSize = 4 * 1024 * 1024;

    RecAttr.enRecType = MT_UNF_DMX_REC_TYPE_SELECT_PID;
    RecAttr.bDescramed = MT_TRUE;
    if(prog->v_pid < 0x1fff)
    {
        RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_VIDEO;
        RecAttr.u32IndexSrcPid = prog->v_pid;
        RecAttr.enVCodecType = MT_UNF_VCODEC_TYPE_MPEG2;
        RecAttr.type_mode = DMX_PARTIAL_TS_PACKET;
    }
    else if(prog->a_pid < 0x1fff)
    {
        RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_AUDIO;
        RecAttr.u32IndexSrcPid = prog->a_pid;
        RecAttr.type_mode = DMX_PARTIAL_TS_PACKET;
    }
    else
    {
        RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
    }

    ret = MT_UNF_DMX_CreateRecChn(&RecAttr, &prec->rechandle);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SATIP_ERR_PRINT("[%s - %u] MT_UNF_DMX_CreateRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
        free(prec);
        return 0xffffffff;
    }

    //pat
    ret = MT_UNF_DMX_AddRecPid(prec->rechandle, 0, &prec->chanhandle[prec->chancnt]);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SATIP_ERR_PRINT("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
        goto exit;
    }
    ++prec->chancnt;

    if(prog->pmt_pid < 0x1fff)
    {
        ret = MT_UNF_DMX_AddRecPid(prec->rechandle, prog->pmt_pid, &prec->chanhandle[prec->chancnt]);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SATIP_ERR_PRINT("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
            goto  exit;
        }
        ++prec->chancnt;
    }

    if(prog->v_pid < 0x1fff)
    {
        ret = MT_UNF_DMX_AddRecPid(prec->rechandle, prog->v_pid, &prec->chanhandle[prec->chancnt]);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SATIP_ERR_PRINT("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
            goto exit;
        }
        ++prec->chancnt;
    }

    if(prog->a_pid < 0x1fff)
    {
        ret = MT_UNF_DMX_AddRecPid(prec->rechandle, prog->a_pid, &prec->chanhandle[prec->chancnt]);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SATIP_ERR_PRINT("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
            goto exit;
        }
        ++prec->chancnt;
    }

    if(prog->pcr_pid < 0x1fff)
    {
        ret = MT_UNF_DMX_AddRecPid(prec->rechandle, prog->pcr_pid, &prec->chanhandle[prec->chancnt]);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SATIP_ERR_PRINT("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
            goto exit;
        }
        ++prec->chancnt;
    }

    ret = MT_UNF_DMX_StartRecChn(prec->rechandle);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SATIP_ERR_PRINT("[%s - %u] MT_UNF_DMX_StartRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
        goto exit;
    }

    return (unsigned long)prec;
exit:
    if(prec->rechandle != (MT_HANDLE) -1)
    {
        ret = MT_UNF_DMX_StopRecChn(prec->rechandle);
    }

    for(i = 0; i < prec->chancnt; i++)
    {
        ret = MT_UNF_DMX_DelRecPid(prec->rechandle, prec->chanhandle[i]);
    }

    ret = MT_UNF_DMX_DestroyRecChn(prec->rechandle);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SATIP_ERR_PRINT("[%s - %u] MT_UNF_DMX_DestroyRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
    }

    free(prec);
    return 0xffffffff;
}


static MT_S32 dvb_stop_rec(unsigned long rechandle)
{
    MT_S32 ret = 0;
    MT_S32 i = 0;

    rec_info *prec = (rec_info*)rechandle;

    if(prec->rechandle == (MT_HANDLE) -1)
    {
        return 0;
    }

    ret = MT_UNF_DMX_StopRecChn(prec->rechandle);
    for(i = 0; i < prec->chancnt; i++)
    {
        ret = MT_UNF_DMX_DelRecPid(prec->rechandle, prec->chanhandle[i]);
    }

    ret = MT_UNF_DMX_DestroyRecChn(prec->rechandle);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SATIP_ERR_PRINT("[%s - %u] MT_UNF_DMX_DestroyRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
    }

    free(prec);
    SAMPLE_SATIP_PRINT("dvb_stop_rec done\n");
    return ret;
}


static MT_U32 dvb_rec_acquire_buf(unsigned long rechandle, MT_U8 **pbufaddr, MT_U32 *len)
{
    MT_S32 ret = 0;

    rec_info *prec = (rec_info*)rechandle;
    ret = MT_UNF_DMX_AcquireRecData(prec->rechandle, &prec->recdata, 100);
    if(ret != MT_SUCCESS)
    {
        if(MT_ERR_DMX_TIMEOUT == ret || ret == MT_ERR_DMX_NOAVAILABLE_DATA)
        {
            *len = 0;
            return 0;
        }
        *len = 0;
        return -1;
    }

    *len = prec->recdata.u32Len;
    *pbufaddr = prec->recdata.pDataAddr;
    return *len;
}


static MT_S32 dvb_rec_release_buf(unsigned long rechandle, MT_U8 *pbufaddr)
{
    MT_S32 ret;
    rec_info *prec = (rec_info*)rechandle;
    ret = MT_UNF_DMX_ReleaseRecData(prec->rechandle, &prec->recdata);
    return ret;
}


static MT_S32 MT_SatipInit(MT_VOID)
{
    MT_U32 ip = 0;
    struct in_addr in;
    satip_dvb_op_t dvbop;

    dvbop.nim_lock = dvb_nim_lock;
    dvbop.start_rec = dvb_start_rec;
    dvbop.stop_rec = dvb_stop_rec;
    dvbop.rec_acquire_buf = dvb_rec_acquire_buf;
    dvbop.rec_release_buf = dvb_rec_release_buf;
    ip = mt_get_ipaddr();
    in.s_addr = ip;
    g_satipdev = mt_satip_init(inet_ntoa(in),8080,&dvbop);
    if(!g_satipdev)
    {
        return -1;
    }

    return 0;
}


static MT_S32 MT_SatipDeInit(MT_VOID)
{
    mt_satip_deinit(g_satipdev);
    return 0;
}


static MT_S32 MT_SatipStart(MT_VOID)
{
    MT_CHAR *server_name = "satip_server";
    mt_satip_set_friendlyname(g_satipdev, server_name, strlen(server_name));
    mt_satip_start(g_satipdev);
    return 0;
}


static MT_S32 MT_SatipStop(MT_VOID)
{
    mt_satip_stop(g_satipdev);
    return 0;
}


static int MT_SatipSetProgramList(MT_VOID)
{
    int ret = 0;
    static satip_pg_info_t pg_info[8];
    satip_pg_list plist;
    int i = 0, j = 0;
    PMT_COMPACT_TBL *pmt_tab = MT_NULL;
    SDT_TB sdt_tb;
    int pgcnt = 0;

    MTADP_Search_Init();
    ret = MTADP_Search_GetAllPmt(0, &pmt_tab);
    if (ret != MT_SUCCESS) {
        return -1;
    }

    memset(pg_info, 0, sizeof(pg_info));
    for (i = 0; i < pmt_tab->prog_num && i < 8; i++) {
        if(MT_INPUT_SIG_TYPE_CAB == satip_run_info.sInputParam.sig_type)
        {
            pg_info[i].freq = satip_run_info.sInputParam.input_param.cab.freq;
            pg_info[i].symb_rate = satip_run_info.sInputParam.input_param.cab.sym_rate;
        }
        else if(MT_INPUT_SIG_TYPE_SAT == satip_run_info.sInputParam.sig_type)
        {
            pg_info[i].freq = satip_run_info.sInputParam.input_param.sat.freq;
            pg_info[i].symb_rate = satip_run_info.sInputParam.input_param.sat.sym_rate;
            pg_info[i].onoff_22k = satip_run_info.sInputParam.input_param.sat.onoff_22k;
            pg_info[i].polar = satip_run_info.sInputParam.input_param.sat.port_type;
        }

        pg_info[i].v_pid = pmt_tab->proginfo[i].VElementPid;
        pg_info[i].a_pid = pmt_tab->proginfo[i].AElementPid;
        pg_info[i].pcr_pid = pmt_tab->proginfo[i].PcrPid;
        pg_info[i].pg_id = pmt_tab->proginfo[i].ProgID;
        pg_info[i].pmt_pid = pmt_tab->proginfo[i].PmtPid;
        pgcnt++;
    }

    memset(&sdt_tb, 0, sizeof(sdt_tb));
    ret = SRH_SDTRequest(0, &sdt_tb);
    if (ret != MT_SUCCESS) {
        MTADP_Search_FreeAllPmt(pmt_tab);
        MTADP_Search_DeInit();
    }

    for (i = 0; i < pgcnt; i++){
        for (j = 0; j < sdt_tb.u32ProgNum; j++) {
            if (pg_info[i].pg_id == sdt_tb.SdtInfo[j].u16ServiceID) {
                strncpy((char*)pg_info[i].pg_name, (char*)sdt_tb.SdtInfo[j].s8ProgName,
                    strlen((char*)sdt_tb.SdtInfo[j].s8ProgName) > 32 ? 32 : strlen((char*)sdt_tb.SdtInfo[j].s8ProgName));
                break;
            }
        }

        if (j == sdt_tb.u32ProgNum) {
            sprintf((char*)pg_info[i].pg_name,"prog_%d", pg_info[i].pg_id);
        }
        SAMPLE_SATIP_INFO_PRINT("program name:%s\n", pg_info[i].pg_name);
    }

    plist.pg_cnt = pgcnt;
    plist.pglist = pg_info;
    mt_satip_setproglist(g_satipdev, &plist);

    (MT_VOID)MTADP_Search_FreeAllPmt(pmt_tab);
    (MT_VOID)MTADP_Search_DeInit();

    return 0;
}


static MT_VOID MT_SatipPrintMenu(MT_VOID)
{
    SAMPLE_SATIP_PRINT("commond: \n");
    SAMPLE_SATIP_PRINT("     q: quit \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_SATIP_PRINT("     b : background run \n");
#endif
    SAMPLE_SATIP_PRINT("     h: help \n");
    SAMPLE_SATIP_PRINT("SATIP>> ");
}


static MT_VOID MT_SatipExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;

    (MT_VOID)MT_SatipStop();

    (MT_VOID)MT_SatipDeInit();

    (MT_VOID)MTADP_Search_FreeAllPmt(satip_run_info.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    (MT_VOID)MT_SatipDmxDeinit();

    if(MT_INPUT_SIG_TYPE_FILE != satip_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
    memset(&satip_run_info, 0xff, sizeof(satip_run_info));
}


/*!
@brief The thread on which the command was entered
@param[in]  hAvplay             Handle to AV player
@param[in]  hWin                The input window handler
@param[in]  pProgTbl            The data structure of the PMT
@return::MT_VOID
@*/
static MT_VOID MT_SatipCmdTask(MT_VOID)
{
    MT_CHAR                inputCmd[32] = { 0 };

    while(1)
    {
        (MT_VOID)MT_SatipPrintMenu();

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_SATIP_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_SATIP_INFO_PRINT("Ratio play in back!\n");
            break;
        }
#endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_SATIP_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}


static MT_VOID MT_SatipPrint_Help(MT_CHAR *name)
{
    MT_SATIP_PRINT("Lack of parameters\n");
    MT_SATIP_PRINT("\nUsage:\n");
    MT_SATIP_PRINT("%s\n", name);
    MT_SATIP_PRINT("    -c: DVBC locks frequency\n");
    MT_SATIP_PRINT("    -s: DVBS locks frequency\n");
#ifdef MT_SAMPLE_APP
    MT_SATIP_PRINT("    -q: Exit the background\n");
#endif
    MT_SATIP_PRINT("example:\n");
    MT_SATIP_PRINT("    %s -c 314 6875 64\n",name);
    MT_SATIP_PRINT("    %s -s 3840 27500 1 0 0\n",name);
}


/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_SatipParase_args(MT_S32 argc, MT_CHAR *argv[], mt_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

    if(argc < 2 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_SatipPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hH:c:s:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_SatipPrint_Help(argv[0]);
                return MT_FAILURE;
            case 's':
                if(argc != 7)
                {
                    (MT_VOID)MT_SatipPrint_Help(argv[0]);
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
                    (MT_VOID)MT_SatipPrint_Help(argv[0]);
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
                    (MT_VOID)MT_SatipExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_SatipPrint_Help(argv[0]);
                return MT_FAILURE;

        }
    }


    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_SatipMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32                 s32Ret = MT_SUCCESS;

    s32Ret = MT_SatipParase_args(argc, argv, &satip_run_info.sInputParam);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_SATIP_ERR_PRINT("Parase args err. stop window.\n");
        return MT_FAILURE;
    }
    else if (MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_SATIP_ERR_PRINT("Recv stop command. stop window.\n");
        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
    #ifndef MT_SAMPLE_APP
        /** System initialization */
        s32Ret = mt_sys_init();
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_SATIP_ERR_PRINT("failed to mt_sys_init\n");
            return s32Ret;
        }
    #endif

        if (MT_INPUT_SIG_TYPE_FILE != satip_run_info.sInputParam.sig_type)
        {
            s32Ret = MTADP_Fe_Init(TUNER_ID_0);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_SATIP_ERR_PRINT("MTADP_Fe_Init failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR1;
            }

            if (MT_INPUT_SIG_TYPE_CAB == satip_run_info.sInputParam.sig_type)
            {
                s32Ret = MT_SatipCheckDvbcParam(&satip_run_info.sInputParam.input_param.cab);
                if (MT_SUCCESS != s32Ret)
                {
                    SAMPLE_SATIP_ERR_PRINT("MT_SatipCheckDvbcParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                    goto ERR2;
                }

                s32Ret = MTADP_Fe_Connect_Dvbc(TUNER_ID_0,
                                            satip_run_info.sInputParam.input_param.cab.freq,
                                            satip_run_info.sInputParam.input_param.cab.sym_rate,
                                            satip_run_info.sInputParam.input_param.cab.mod_type);
            }
            else if (MT_INPUT_SIG_TYPE_SAT == satip_run_info.sInputParam.sig_type)
            {
                s32Ret = MT_SatipCheckDvbsParam(&satip_run_info.sInputParam.input_param.sat);
                if (MT_SUCCESS != s32Ret)
                {
                    SAMPLE_SATIP_ERR_PRINT("MT_SatipCheckDvbsParam failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                    goto ERR2;
                }

                s32Ret = MTADP_Fe_Connect_Dvbs(TUNER_ID_0,
                                            satip_run_info.sInputParam.input_param.sat.freq,
                                            satip_run_info.sInputParam.input_param.sat.sym_rate,
                                            satip_run_info.sInputParam.input_param.sat.onoff_22k,
                                            satip_run_info.sInputParam.input_param.sat.polarization,
                                            satip_run_info.sInputParam.input_param.sat.port_type);
            }

            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_SATIP_ERR_PRINT("MTADP_Fe_Connect failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);
                goto ERR2;
            }
        }

        /** DMX module initialization */
        s32Ret = MT_SatipDmxInit(satip_run_info.sInputParam.sig_type);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SATIP_ERR_PRINT("failed to StartDmx\n");
            goto ERR2;
        }

        s32Ret = MT_SatipInit();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SATIP_ERR_PRINT("failed to MT_SatipInit\n");
            goto ERR3;
        }

        s32Ret = MT_SatipSetProgramList();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SATIP_ERR_PRINT("failed to MT_SatipSetProgramList\n");
            goto ERR4;
        }

        s32Ret = MT_SatipStart();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_SATIP_ERR_PRINT("failed to MT_SatipStart\n");
            goto ERR5;
        }
        g_bTaskQuit = MT_FALSE;
    }

    (MT_VOID)MT_SatipCmdTask();

    if (g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

ERR5:
    (MT_VOID)MT_SatipStop();
ERR4:
    (MT_VOID)MT_SatipDeInit();
ERR3:
    (MT_VOID)MT_SatipDmxDeinit();
ERR2:
    if (MT_INPUT_SIG_TYPE_FILE != satip_run_info.sInputParam.sig_type)
    {
        (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);
    }
ERR1:
#ifndef MT_SAMPLE_APP
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&satip_run_info, 0xff, sizeof(satip_run_info));

    return s32Ret;
}
