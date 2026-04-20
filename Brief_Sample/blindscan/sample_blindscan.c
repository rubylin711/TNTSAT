/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <linux/fs.h>
#include <stdio.h>
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
#include "mt_unf_frontend.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_demux.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_BLINDSCAN_DEBUG

#define MT_BLINDSCAN_PRINT   printf
#else

#define MT_BLINDSCAN_PRINT

#endif

#define SAMPLE_BLINDSCAN_FUNCTION_ENTER()   MT_BLINDSCAN_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_BLINDSCAN_FUNCTION_EXIT()        MT_BLINDSCAN_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_BLINDSCAN_FATAL_PRINT(fmt...)        MT_BLINDSCAN_PRINT(" [FATAL] " fmt)
#define SAMPLE_BLINDSCAN_ERR_PRINT(fmt...)          MT_BLINDSCAN_PRINT(" [ERROR] " fmt)
#define SAMPLE_BLINDSCAN_WARN_PRINT(fmt...)         MT_BLINDSCAN_PRINT(" [WARN] "  fmt)
#define SAMPLE_BLINDSCAN_INFO_PRINT(fmt...)         MT_BLINDSCAN_PRINT(" [INFO] "  fmt)
#define SAMPLE_BLINDSCAN_DBG_PRINT(fmt...)          MT_BLINDSCAN_PRINT(" [DEBUG] " fmt)


#define SAMPLE_BLINDSCAN_PRINT  printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define DMX_ID_0            0
#define TUNER_ID_0          0
#define MAX_TP_NUM          64

/*************************** Structure Definition ****************************/
typedef struct
{
    mt_u32 tuner_id; /**<tuner id*/
    mt_u32 startfreq; /**<start Frequency, in kHz*/
    mt_u32 endfreq; /**<stopfreq Frequency, in kHz*/
    mt_u32 onoff_22k; /**<22k*/
    mt_u32 polar; /**<Polarization mode>*/
    mt_u32 bs_mode; /**<blind mode>*/
} mt_input_Blindscan_para_t;

typedef enum
{
    MT_BLINDSCAN_STATUS_INIT,
    MT_BLINDSCAN_STATUS_SCANNING,
    MT_BLINDSCAN_STATUS_FINISH,

} mt_blindscan_status_t;


typedef struct
{
    mt_s32 s32TPNum;
    mt_unf_fe_sat_tpinfo_t satTPinfo[MAX_TP_NUM];
    mt_blindscan_status_t bs_status;
}mt_TPinfo_para_t;


/********************** Global Variable declaration **************************/
static mt_TPinfo_para_t g_sTPinfo;
static MT_BOOL    g_bTaskQuit = MT_TRUE;


#ifdef MT_SAMPLE_APP
MT_S32 MT_BlindscanMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static mt_s32 MT_BlindscanCheckParam(mt_input_Blindscan_para_t *p_BlindScan_in)
{
    if(p_BlindScan_in->startfreq >= p_BlindScan_in->endfreq)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("Frequency setting error\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}


/*!
@brief Demux initializes and retrieves the PMT and PAT tables in TS.
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_BlindscanDmxInit(MT_VOID)
{
    MT_S32                 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    /** Obtain the chip model */
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("MT_UNF_DMX_AttachTSPort failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);

        return ret;
    }

    /** Initializes the demux module */
    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
        return ret;
    }


    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        /** Bind Demux to tuner port 1 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);
        SAMPLE_BLINDSCAN_INFO_PRINT("Connect port 0!\n");
    }
    else
    {
        /** Bind Demux to tuner port 0 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
        SAMPLE_BLINDSCAN_INFO_PRINT("Connect port 1!\n");
    }

    if(MT_SUCCESS != ret)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        return ret;
    }

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit
 @return void
*/
static void MT_BlindscanDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    (MT_VOID)MT_UNF_DMX_DeInit();

}



/*
@brief set tuner parameters
@param[in] tuner_id,Port of the tuner
@param[in] sig_type,Type of received signal
@param[in] tuner_dev_type,tuner Device type
@param[in] tuner_addr, The address of tuner
@param[in] demod_dev_type, Type of the demod device
@param[in] demod_addr, The address of demod
@param[in] out_put_mode, Output mode
@param[in] I2c_channel, i2c Channel mode
@return MT_SUCCESS
@return MT_FAILURE
*/
static mt_s32 MT_Blindscan_Set_Parameter(MT_U32 tuner_id, MT_U32 sig_type, MT_U32 tuner_dev_type, MT_U32 tuner_addr,
             MT_U32 demod_dev_type, MT_U32 demod_addr, MT_U32 out_put_mode, MT_U32 I2c_channel)
{
    MT_S32           ret = 0;
    mt_unf_fe_attr_t mtTunerAttr = { 0 };
    mt_sys_version_s stSysChipInfo = { 0 };

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("mt_sys_get_version err!\n");
        return ret;
    }

    SAMPLE_BLINDSCAN_INFO_PRINT("MTCommand_Tuner_Operation:   chipVersion = 0x%x\n", stSysChipInfo.enChipVersion);

    ret = mt_unf_fe_get_default_attr(tuner_id,&mtTunerAttr);
    if(SUCCESS != ret)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("mt_unf_fe_get_default_attr err!\n");
        return ret;
    }


    mtTunerAttr.sig_type = sig_type;

    if (stSysChipInfo.enChipVersion < MT_CHIP_SYMPHONY2_A0)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY2_A3)
    {
        if ((tuner_dev_type == MT_UNF_TUNER_TYPE_M88TC3800)
          ||(tuner_dev_type == MT_UNF_TUNER_TYPE_M88TC6800)
          ||(tuner_dev_type == MT_UNF_TUNER_TYPE_MXL_608))
        {
            mtTunerAttr.fe_config.tun2_type = tuner_dev_type;
            mtTunerAttr.fe_config.tun2_addr = tuner_addr;
        }
        else
        {
            mtTunerAttr.tuner_type = tuner_dev_type;
            mtTunerAttr.tuner_addr = tuner_addr;
        }
    }
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY4_A1)
    {
        SAMPLE_BLINDSCAN_INFO_PRINT("[%s %d]tuner_dev_type = %d, tuner_addr = 0x%x\n", __FUNCTION__, __LINE__, tuner_dev_type, tuner_addr);
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    else if(stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY6_A0)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
#endif

    mtTunerAttr.demod_dev_type = demod_dev_type;
    mtTunerAttr.demod_addr = demod_addr;
    mtTunerAttr.demod_i2c_id = I2c_channel;
    mtTunerAttr.output_mode = out_put_mode;
    mtTunerAttr.tuner_i2c_id[0] = 0;
    mtTunerAttr.no_need_init = 0;

    SAMPLE_BLINDSCAN_INFO_PRINT("tuner_type[%d], tuner_addr[0x%x], demod_type[%d], demod_addr[0x%x], demod_id[%d], output_mode[%d] \n",
                            mtTunerAttr.tuner_type, mtTunerAttr.tuner_addr,
                            mtTunerAttr.demod_dev_type, mtTunerAttr.demod_addr,
                            mtTunerAttr.demod_i2c_id, mtTunerAttr.output_mode );

    ret = mt_unf_fe_set_attr(tuner_id, &mtTunerAttr);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("mt_unf_fe_set_attr failed.ret is %d\n",ret);
        return ret;
    }

    SAMPLE_BLINDSCAN_INFO_PRINT("output_mode = %d\n", out_put_mode);

    return MT_SUCCESS;
}


static MT_S32 MT_Blindscan_Notify(MT_U32 tuner_id, mt_unf_fe_blindscan_evt_t enEVT, mt_unf_fe_blindscan_notify_t *punNotify)
{
  mt_s32 i = 0;
  mt_s32 num = 0;
  mt_unf_fe_sat_tpinfo_t temp;



  switch (enEVT)
  {
    case MT_UNF_FE_BLINDSCAN_EVT_STATUS:
      if(MT_UNF_FE_BLINDSCAN_STATUS_FAIL == *(punNotify->status))
      {
        SAMPLE_BLINDSCAN_ERR_PRINT("Scan fail.\n");
        return MT_FAILURE;
      }
      else if ((MT_UNF_FE_BLINDSCAN_STATUS_FINISH == *(punNotify->status)) || (MT_UNF_FE_BLINDSCAN_STATUS_QUIT == *(punNotify->status)))
      {
        SAMPLE_BLINDSCAN_PRINT("100%%");
        SAMPLE_BLINDSCAN_PRINT("done\n");
        SAMPLE_BLINDSCAN_INFO_PRINT("Scan over, find %d TP.\n", g_sTPinfo.s32TPNum);
        g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_FINISH;
        return MT_SUCCESS;


      }
      break;

    case MT_UNF_FE_BLINDSCAN_EVT_PROGRESS:

      SAMPLE_BLINDSCAN_PRINT("%d%% \n", *(punNotify->progress_percent));
      break;

    case MT_UNF_FE_BLINDSCAN_EVT_NEWRESULT:
      num++;
      temp = *(punNotify->result);
      SAMPLE_BLINDSCAN_PRINT("\t%03d %7d %5d %d %d, %d \n", num, temp.freq, temp.symbol_rate, temp.polar, temp.code_rate, temp.DataTsNumber);

      for (i = 0; i < temp.DataTsNumber; i++)
      {
            SAMPLE_BLINDSCAN_PRINT("TS[%d] ---- 0x%02x\n", i, temp.DataTsIdArray[i]);
            (MT_VOID)mt_unf_fe_set_s2_multi_stream_ts_id(tuner_id, temp.DataTsIdArray[i]);

            MT_USLEEP(200000);
      }


      g_sTPinfo.satTPinfo[g_sTPinfo.s32TPNum] = *(punNotify->result);
      g_sTPinfo.s32TPNum++;
      g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_SCANNING;
      break;

    default:
      break;
  }

  return MT_SUCCESS;
}


static MT_S32 MT_Blindscan_Dvbs(mt_u32 tuner_id, MT_U8 bs_mode, MT_U32 u32StartFreq, MT_U32 u32StopFreq, MT_U8 polar, MT_U8 onoff_22k)
{
    MT_S32 ret = 0;
    mt_unf_fe_blindscan_para_t stBlindScanPara = { 0 };

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#ifdef MT_SYM4_DSS
    ret = MT_Blindscan_Set_Parameter(tuner_id, MT_UNF_FE_SIG_TYPE_DVBS_AUTO, MT_UNF_TUNER_TYPE_M88TS6011, 0x58, MT_UNF_DEMOD_DEV_TYPE_M88DS6113, 0xD2, 1, 1);
#else
    ret = MT_Blindscan_Set_Parameter(tuner_id, 2048, 34, 88, 288, 24, 1, 0);
#endif
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    if (tuner_id == 0)
    {
        ret = MT_Blindscan_Set_Parameter(tuner_id, 2048, 34, 88, 288, 24, 4, 0);
    }
    else
    {
        ret = MT_Blindscan_Set_Parameter(tuner_id, 2048, 80, 90, 336, 210, 4, 1);
    }

#endif
    if(MT_SUCCESS != ret)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("call MT_Blindscan_Set_Parameter failed.\n");
        return MT_FAILURE;
    }
    memset(&stBlindScanPara, 0, sizeof(mt_unf_fe_blindscan_para_t));

    if (0 == bs_mode)
    {
        /* Auto */
        stBlindScanPara.mode = MT_UNF_FE_BLINDSCAN_MODE_AUTO;
        /* If your diseqc device need config polarization and 22K, you need register the callback */
        stBlindScanPara.scan_para.sat.diseqc_set = MT_NULL;
        stBlindScanPara.scan_para.sat.scan_notify = (mt_void(*)(mt_u32, mt_unf_fe_blindscan_evt_t, void *))MT_Blindscan_Notify;
    }
    else
    {
        stBlindScanPara.mode = MT_UNF_FE_BLINDSCAN_MODE_MANUAL;
        stBlindScanPara.scan_para.sat.polar = polar;
        stBlindScanPara.scan_para.sat.lnb_22k = onoff_22k;
        stBlindScanPara.scan_para.sat.start_freq = u32StartFreq * 1000;
        stBlindScanPara.scan_para.sat.stop_freq = u32StopFreq * 1000;
        stBlindScanPara.scan_para.sat.diseqc_set = MT_NULL;
        stBlindScanPara.scan_para.sat.scan_notify = (mt_void(*)(mt_u32, mt_unf_fe_blindscan_evt_t, void *))MT_Blindscan_Notify;
    }


    ret = mt_unf_fe_blindscan_start(tuner_id, &stBlindScanPara);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("call mt_unf_fe_blindscan_start failed.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}



static MT_VOID MT_BlindscanPrintMenu(MT_VOID)
{

    SAMPLE_BLINDSCAN_PRINT("\n 1 - %d : select the TP number to get program info \n", g_sTPinfo.s32TPNum);
    SAMPLE_BLINDSCAN_PRINT("     p : print TP info \n");
    SAMPLE_BLINDSCAN_PRINT("     r : restart scan TP \n");
    SAMPLE_BLINDSCAN_PRINT("     w : turn on/off lnb power \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_BLINDSCAN_PRINT("     b : background run \n");
#endif
    SAMPLE_BLINDSCAN_PRINT("     h : help \n");
    SAMPLE_BLINDSCAN_PRINT("     q : quit \n");
    SAMPLE_BLINDSCAN_PRINT("Blindscan>> ");

}


/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static void MT_BlindscanPrint_help(char *name)
{

    SAMPLE_BLINDSCAN_PRINT("Options:\n");
    SAMPLE_BLINDSCAN_PRINT(" %s -t tuner_id -b mode -s start_freq -e stop_freq -k 22k -p polar \n", name);
    SAMPLE_BLINDSCAN_PRINT(" %s -t 0 -b 1 -s 1000 -e 1100 -k 1 -p 0\n", name);
    SAMPLE_BLINDSCAN_PRINT(" %s -q  <exit> \n", name);
}

static MT_VOID MT_BlindscanPrintTpList(MT_VOID)
{
    MT_S32 i = 0;
    MT_CHAR dvb_type[g_sTPinfo.s32TPNum][10];
    for(i = 0;i < g_sTPinfo.s32TPNum; i++)
    {
        switch(g_sTPinfo.satTPinfo[i].dvb_type)
        {
            case 0:
                strcpy(dvb_type[i], "DVB-S");
                break;
            case 1:
                strcpy(dvb_type[i], "DVB-S2");
                break;
            default:
                break;
        }
    }
    MT_BLINDSCAN_PRINT("<Index>          <Freq>           <Sym Rate>         <Type> \n");
    for(i = 0;i < g_sTPinfo.s32TPNum; i++)
    {
        MT_BLINDSCAN_PRINT(" %3d         %10dKHz     %8dKSs           %5s\n", i+1, g_sTPinfo.satTPinfo[i].freq, g_sTPinfo.satTPinfo[i].symbol_rate, dvb_type[i]);
    }
}

static MT_VOID MT_BlindscanPrintProgramInfo(PMT_COMPACT_TBL *progTbl, MT_U32  u32ProgNum)
{
    MT_S32  i = 0;
    MT_S32  j = 0;
    MT_CHAR dvb_type[g_sTPinfo.s32TPNum][10];

    if(g_sTPinfo.satTPinfo[u32ProgNum-1].dvb_type == 1)
    {
        strcpy(dvb_type[i], "DVB-S2");
    }
    else
    {
        strcpy(dvb_type[i], "DVB-S");
    }
    MT_BLINDSCAN_PRINT("<Index>          <Freq>           <Sym Rate>         <Type> \n");
    MT_BLINDSCAN_PRINT(" %3d         %10dKHz     %8dKSs           %5s\n", u32ProgNum, g_sTPinfo.satTPinfo[u32ProgNum-1].freq, g_sTPinfo.satTPinfo[u32ProgNum-1].symbol_rate, (mt_char*)dvb_type);

    for(i =0; i<progTbl->prog_num;i++)
    {
        MT_BLINDSCAN_PRINT("Channel Num = %d, Program ServiceID = %d PMT PID = %x\n", i+1, progTbl->proginfo[i].ProgID,progTbl->proginfo[i].PmtPid);

         for (j = 0; j < progTbl->proginfo[i].VElementNum;j++)
         {
             MT_BLINDSCAN_PRINT("\tVideo Stream PID   = 0x%x\n",progTbl->proginfo[i].VElementPid);
             switch (progTbl->proginfo[i].VideoType)
             {
                case MT_UNF_VCODEC_TYPE_H264:
                    MT_BLINDSCAN_PRINT("\tVideo Stream Type H264\n");
                    break;
                case MT_UNF_VCODEC_TYPE_MPEG2:
                    MT_BLINDSCAN_PRINT("\tVideo Stream Type MP2\n");
                    break;
                case MT_UNF_VCODEC_TYPE_MPEG4:
                    MT_BLINDSCAN_PRINT("\tVideo Stream Type MP4\n");
                    break;
                case MT_UNF_VCODEC_TYPE_HEVC:
                    MT_BLINDSCAN_PRINT("\tVideo Stream Type HEVC\n");
                    break;
                default:
                    MT_BLINDSCAN_PRINT("\tVideo Stream Type error\n");
             }
         }
         for (j = 0; j < progTbl->proginfo[i].AElementNum; j++)
         {
            MT_BLINDSCAN_PRINT("\tAudio Stream PID   = 0x%x\n", progTbl->proginfo[i].Audioinfo[j].u16AudioPid);

            switch (progTbl->proginfo[i].Audioinfo[j].u32AudioEncType)
            {
                case HA_AUDIO_ID_MP3:
                    MT_BLINDSCAN_PRINT("\tAudio Stream Type MP3\n");
                    break;
                case HA_AUDIO_ID_AAC:
                    MT_BLINDSCAN_PRINT("\tAudio Stream Type AAC\n");
                    break;
                case HA_AUDIO_ID_DOLBY_PLUS:
                    MT_BLINDSCAN_PRINT("\tAudio Stream Type AC3\n");
                    break;
                case HA_AUDIO_ID_DTSHD:
                    MT_BLINDSCAN_PRINT("\tAudio Stream Type DTS\n");
                    break;
                default:
                    MT_BLINDSCAN_PRINT("\tAudio Stream Type error\n");
             }
         }
         MT_BLINDSCAN_PRINT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }
}

static MT_S32 MT_BlindscanCmdTask(mt_input_Blindscan_para_t *pInutParam)
{
    MT_S32  ret = 0;

    MT_U32  u32ProgNum = 0;
    MT_CHAR inputCmd[32] = { 0 };
    mt_s32 lnb_power;
    PMT_COMPACT_TBL *progTbl = MT_NULL;

     while(g_sTPinfo.bs_status != MT_BLINDSCAN_STATUS_FINISH)
     {

        usleep(5000);
     }

    while(1)
    {
        (MT_VOID)MT_BlindscanPrintMenu();
        /* get inputCmd*/
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_BLINDSCAN_INFO_PRINT("prepare to exit!\n");
            g_bTaskQuit  = MT_TRUE;
            break;
        }
        #ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_BLINDSCAN_INFO_PRINT("Dvbs play in back!\n");
            break;
        }
        #endif
        else  if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {

            u32ProgNum = atoi(inputCmd);

            if(u32ProgNum > 0 && u32ProgNum <= g_sTPinfo.s32TPNum)
            {

                ret = MTADP_Fe_Connect_Dvbs(pInutParam->tuner_id,  (g_sTPinfo.satTPinfo[u32ProgNum-1].freq)/1000, (g_sTPinfo.satTPinfo[u32ProgNum-1].symbol_rate), pInutParam->onoff_22k, 0, 2);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_BLINDSCAN_ERR_PRINT("MTADP_Fe_Connect_Dvbs failed.\n");
                    return MT_FAILURE;
                }
                ret = MT_BlindscanDmxInit();
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_BLINDSCAN_ERR_PRINT("MT_BlindscanDmxInit failed.\n");
                    return MT_FAILURE;
                }


                /** The search module is initialized */
                (MT_VOID)MTADP_Search_Init();

                /** Get the PMT table */
                ret = MTADP_Search_GetAllPmt(DMX_ID_0, &progTbl);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_BLINDSCAN_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
                    (MT_VOID)MTADP_Search_DeInit();
                }
                MT_BLINDSCAN_PRINT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                (MT_VOID)MT_BlindscanPrintProgramInfo(progTbl, u32ProgNum);
                SAMPLE_BLINDSCAN_INFO_PRINT("get Program information over!!!!\n");
            }

            (MT_VOID)MTADP_Search_FreeAllPmt(progTbl);
            (MT_VOID)MT_BlindscanDmxDeInit();

        }
        else if('p' == inputCmd[0])
        {
            SAMPLE_BLINDSCAN_INFO_PRINT("Print TP info \n");
            (MT_VOID)MT_BlindscanPrintTpList();
        }
        else if('w' == inputCmd[0])
        {
            SAMPLE_BLINDSCAN_INFO_PRINT("Choice lnb power(0: power off 1: power open 2: power enhanced):\n");
            scanf("%d", &lnb_power);
            getchar();
            lnb_power %= MT_UNF_FE_LNB_POWER_BUTT;
            mt_unf_fe_set_lnb_power(pInutParam->tuner_id, lnb_power);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_BLINDSCAN_ERR_PRINT("mt_unf_fe_set_lnb_power failed.\n");
                return MT_FAILURE;
            }
        }
        else if('r' == inputCmd[0])
        {
            memset(&g_sTPinfo, 0, sizeof(g_sTPinfo));
            g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_SCANNING;
            ret = MT_Blindscan_Dvbs(pInutParam->tuner_id, pInutParam->bs_mode, pInutParam->startfreq, pInutParam->endfreq, pInutParam->polar, pInutParam->onoff_22k);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_BLINDSCAN_ERR_PRINT("MT_Blindscan_Dvbs failed.\n");
            }
            while(g_sTPinfo.bs_status != MT_BLINDSCAN_STATUS_FINISH)
            {

                usleep(5000);
            }

        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_BLINDSCAN_INFO_PRINT("Print help info \n");
        }
    }
    return MT_SUCCESS;
}
static void MT_BlindscanExit(mt_u32 tuner_id)
{

    (MT_VOID)MTADP_Fe_DeInit(tuner_id);
    memset(&g_sTPinfo, 0, sizeof(g_sTPinfo));
    g_bTaskQuit = MT_TRUE;
}

/*!
@brief gets the external input parameters.
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::void
@*/
static mt_s32 MT_BlindscanParase_args(int argc, char *argv[], mt_input_Blindscan_para_t *pInutParam)
{
    mt_s32 opt = 0;

    while((opt = MTADP_Getopt(argc, argv, ":?hHb:t:p:k:s:e:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_BlindscanPrint_help(argv[0]);
            return MT_FAILURE;
             case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_BlindscanExit(pInutParam->tuner_id);
                }
                return MT_TASK_EXIT;
            case 'b':
                pInutParam->bs_mode = strtol(mt_optarg, 0, 0);
                break;

            case 's':
                pInutParam->startfreq = strtol(mt_optarg, 0, 0);
                break;

            case 'k':
                pInutParam->onoff_22k = strtol(mt_optarg, 0, 0);
                break;

            case 'e':
                pInutParam->endfreq = strtol(mt_optarg, 0, 0);
                break;

            case 't':
                pInutParam->tuner_id = strtol(mt_optarg, 0, 0);
                break;

            case 'p':
                pInutParam->polar = strtol(mt_optarg, 0, 0);
                break;

            default:
                (void)MT_BlindscanPrint_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_BlindscanMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32  ret = 0;
    static mt_input_Blindscan_para_t    sInputParam = {0};

    if(argc != 13 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_BlindscanPrint_help(argv[0]);
        return MT_SUCCESS;
    }
    ret = MT_BlindscanParase_args(argc, argv, &sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_BLINDSCAN_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {

        ret = MT_BlindscanCheckParam(&sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_BLINDSCAN_ERR_PRINT("MT_BlindscanCheckParam failed.\n");
            return ret;
        }
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_BLINDSCAN_ERR_PRINT("MT_SYS_Init failed.\n");
            return ret;
        }
#endif
        ret = MTADP_Fe_Init(sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_BLINDSCAN_ERR_PRINT("MTADP_Fe_Init failed.\n");
            goto ERR1;
        }

        g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_INIT;

        ret = MT_Blindscan_Dvbs(sInputParam.tuner_id, sInputParam.bs_mode, sInputParam.startfreq, sInputParam.endfreq, sInputParam.polar, sInputParam.onoff_22k);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_BLINDSCAN_ERR_PRINT("MT_Blindscan_Dvbs failed.\n");
            goto ERR2;
        }

        g_sTPinfo.bs_status = MT_BLINDSCAN_STATUS_SCANNING;
        g_bTaskQuit = MT_FALSE;
    }

    (void)MT_BlindscanCmdTask(&sInputParam);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }


ERR2:
    (MT_VOID)MT_BlindscanExit(sInputParam.tuner_id);

ERR1:
#ifndef MT_SAMPLE_APP
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif
    return ret;


}

