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
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_DISEQC2_DEBUG

#define MT_DISEQC2_PRINT   printf
#else

#define MT_DISEQC2_PRINT

#endif

#define SAMPLE_DISEQC2_FUNCTION_ENTER()     MT_DISEQC2_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DISEQC2_FUNCTION_EXIT()      MT_DISEQC2_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DISEQC2_FATAL_PRINT(fmt...)      MT_DISEQC2_PRINT(" [FATAL] " fmt)
#define SAMPLE_DISEQC2_ERR_PRINT(fmt...)        MT_DISEQC2_PRINT(" [ERROR] " fmt)
#define SAMPLE_DISEQC2_WARN_PRINT(fmt...)       MT_DISEQC2_PRINT(" [WARN] "  fmt)
#define SAMPLE_DISEQC2_INFO_PRINT(fmt...)       MT_DISEQC2_PRINT(" [INFO] "  fmt)
#define SAMPLE_DISEQC2_DBG_PRINT(fmt...)        MT_DISEQC2_PRINT(" [DEBUG] " fmt)


#define SAMPLE_DISEQC2_PRINT  printf



#define INT_TUNER_ID_0      0
#define MAX_MESG_NUM        8

/*************************** Structure Definition ****************************/

typedef struct
{
    MT_U32 tuner_id;
    MT_U32 rx_mode;
    MT_U32 rx_gpio_pin;
    mt_u32 slen;
    mt_u32 rlen;
    mt_char mesg[MAX_MESG_NUM];
} mt_input_diseqc2_para_t;


#ifdef MT_SAMPLE_APP
MT_S32 MT_DisEqc2Main(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static MT_S32 MT_Diseqc2LNB_Test(MT_U32 tuner_id, MT_U8 type, MT_U8 value)
{
    MT_S32 ret = MT_SUCCESS;

    SAMPLE_DISEQC2_INFO_PRINT("[%s %d] -- tuner_id = %d, type = %d, value = %d\n", __FUNCTION__, __LINE__, tuner_id, type, value);
    switch (type)
    {
        case 1:
        mt_unf_fe_set_lnb_onoff(tuner_id, value);
        break;

        case 2:
        if (value)
        {
            mt_unf_fe_set_lnb_power(tuner_id, MT_UNF_FE_LNB_POWER_ON);
        }
        else
        {
            mt_unf_fe_set_lnb_power(tuner_id, MT_UNF_FE_LNB_POWER_OFF);
        }
        break;

        case 3:
        if (value)
        {
            mt_unf_fe_set_polarization(tuner_id, PORT_PORLAR_HORIZONTAL);
        }
        else
        {
            mt_unf_fe_set_polarization(tuner_id, PORT_PORLAR_VERTICAL);
        }
        break;

        case 4:
        mt_unf_fe_set_22k_onoff(tuner_id, value);
        break;

        default:
        SAMPLE_DISEQC2_ERR_PRINT("[%s %d] -- type = %d, the type is wrong\n", __FUNCTION__, __LINE__, type);
        break;
    }

    return ret;
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
static mt_s32 MT_Diseqc2SetParameter(MT_U32 tuner_id, MT_U32 sig_type, MT_U32 tuner_dev_type, MT_U32 tuner_addr,
             MT_U32 demod_dev_type, MT_U32 demod_addr, MT_U32 out_put_mode, MT_U32 I2c_channel, mt_input_diseqc2_para_t InutParam)
{
    MT_S32 ret = 0;
    mt_unf_fe_attr_t mtTunerAttr = { 0 };

    mt_sys_version_s stSysChipInfo = { 0 };


    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC2_ERR_PRINT("mt_sys_get_version failed! \n");
        return ret;
    }
    SAMPLE_DISEQC2_INFO_PRINT("chipVersion = 0x%x\n", stSysChipInfo.enChipVersion);

    ret = mt_unf_fe_get_default_attr(tuner_id, &mtTunerAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC2_ERR_PRINT("mt_unf_fe_get_default_attr failed! \n");
        return ret;
    }

    mtTunerAttr.fe_config.pin_config.diseqc_rx_mode = InutParam.rx_mode;
    mtTunerAttr.fe_config.pin_config.diseqc_rx_gpio_pin = InutParam.rx_gpio_pin;


    mtTunerAttr.sig_type = sig_type;


    if (stSysChipInfo.enChipVersion < MT_CHIP_SYMPHONY2_A0)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY2_A3)
    {
        if ((tuner_dev_type == MT_UNF_TUNER_TYPE_M88TC3800) ||
            (tuner_dev_type == MT_UNF_TUNER_TYPE_M88TC6800) ||
            (tuner_dev_type == MT_UNF_TUNER_TYPE_MXL603) ||
            (tuner_dev_type == MT_UNF_TUNER_TYPE_MXL608) ||
            (tuner_dev_type == MT_UNF_TUNER_TYPE_MXL_608))
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
        SAMPLE_DISEQC2_INFO_PRINT("[%s %d]tuner_dev_type = %d, tuner_addr = 0x%x\n", __FUNCTION__, __LINE__, tuner_dev_type, tuner_addr);
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


    SAMPLE_DISEQC2_INFO_PRINT("%s[%d] -- output_mode = %d\n", __FUNCTION__, __LINE__, out_put_mode);

    ret = mt_unf_fe_set_attr(tuner_id, &mtTunerAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC2_ERR_PRINT("mt_unf_fe_set_attr failed! \n");
        return ret;
    }
    return ret;

}

static mt_s32 MT_Diseqc2Sendrecv(mt_input_diseqc2_para_t InutParam)
{

    mt_s32 Ret = 0;
    mt_u32 i = 0;
    mt_u32 port = 0;
    mt_u32 polar = 0;
    mt_u32 onoff_22k = 0;
    mt_unf_fe_diseqc_switch4port_t st4Port = { 0 };
    mt_unf_fe_diseqc2_rsmsg_info msg_info = { 0 };

    memset(&msg_info, 0, sizeof(mt_unf_fe_diseqc2_rsmsg_info));
    port = 2;
    polar = 1;
    onoff_22k = 0;
    msg_info.slen = InutParam.slen;
    msg_info.rlen = InutParam.rlen;

    st4Port.level = MT_UNF_FE_DISEQC_LEVEL_2_X;
    st4Port.port = (mt_unf_fe_diseqc_switch_port_t)port;
    st4Port.polar = polar;
    st4Port.lnb_22k = onoff_22k;

    if (msg_info.slen > 0)
    {
        msg_info.sendmsg[0] = InutParam.mesg[0];
        SAMPLE_DISEQC2_PRINT("tx_buf[0] = 0x%02x\n", InutParam.mesg[0]);
    }
    if (msg_info.slen > 1)
    {
        msg_info.sendmsg[1] = InutParam.mesg[1];
        SAMPLE_DISEQC2_PRINT("tx_buf[1] = 0x%02x\n", InutParam.mesg[1]);
    }
    if (msg_info.slen > 2)
    {
        msg_info.sendmsg[2] = InutParam.mesg[2];
        SAMPLE_DISEQC2_PRINT("tx_buf[2] = 0x%02x\n", InutParam.mesg[2]);
    }
    if (msg_info.slen > 3)
    {
        msg_info.sendmsg[3] = InutParam.mesg[3];
        SAMPLE_DISEQC2_PRINT("tx_buf[3] = 0x%02x\n", InutParam.mesg[3]);
    }
    if (msg_info.slen > 4)
    {
        msg_info.sendmsg[4] = InutParam.mesg[4];
        SAMPLE_DISEQC2_PRINT("tx_buf[4] = 0x%02x\n", InutParam.mesg[4]);
    }
    if (msg_info.slen > 5)
    {
        msg_info.sendmsg[5] = InutParam.mesg[5];
        SAMPLE_DISEQC2_PRINT("tx_buf[5] = 0x%02x\n", InutParam.mesg[5]);
    }
    if (msg_info.slen > 6)
    {
        msg_info.sendmsg[6] = InutParam.mesg[6];
        SAMPLE_DISEQC2_PRINT("tx_buf[6] = 0x%02x\n", InutParam.mesg[6]);
    }
    if (msg_info.slen > 7)
    {
        msg_info.sendmsg[7] = InutParam.mesg[7];
        SAMPLE_DISEQC2_PRINT("tx_buf[7] = 0x%02x\n", InutParam.mesg[7]);
    }


    if (msg_info.slen > 8 || msg_info.rlen > 8)
    {
        SAMPLE_DISEQC2_ERR_PRINT("param num if bigger. \n");
        return MT_FAILURE;
    }


    Ret = mt_unf_fe_diseqc2_srmsg(InutParam.tuner_id, &st4Port, &msg_info);
    if (MT_SUCCESS != Ret)
    {
       SAMPLE_DISEQC2_ERR_PRINT("send recv msg failed \n");
       return MT_FAILURE;

    }
    for (i = 0; i < msg_info.rlen; i++)
    {
       SAMPLE_DISEQC2_PRINT("\t0x%02x", msg_info.rcvmsg[i]);
    }
    SAMPLE_DISEQC2_PRINT("\n");
    memset(&msg_info.rcvmsg, 0, sizeof(mt_u8) * MT_UNF_DISEQC_MSG_MAX_LENGTH);
    MT_USLEEP(10000);

    return MT_SUCCESS;

}

static void MT_Diseqc2Print_help(char *name)
{

    SAMPLE_DISEQC2_PRINT("Options:\n");
    SAMPLE_DISEQC2_PRINT(" %s -f tuner_id rx_mode rx_gpio_pin slen rlen smesg0 smesg1...  \n", name);
    SAMPLE_DISEQC2_PRINT(" %s -f 0 1 102 4 7 160 4 90 254 (Use the corresponding inverto device)\n", name);
    SAMPLE_DISEQC2_PRINT(" %s -f 0 1 102 3 2 226 0 17 (Replace the corresponding triax equipment)\n", name);
}


static mt_s32 MT_Diseqc2Parase_args(int argc, char *argv[], mt_input_diseqc2_para_t *pInutParam)
{
    mt_s32 opt = 0;
    mt_s32 i = 0;

    if(argc < 8)
    {
        (MT_VOID)MT_Diseqc2Print_help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_Diseqc2Print_help(argv[0]);
                return MT_FAILURE;

            case 'f':
                pInutParam->tuner_id = strtol(argv[2], 0, 0);
                pInutParam->rx_mode = strtol(argv[3], 0, 0);
                pInutParam->rx_gpio_pin = strtol(argv[4], 0, 0);
                pInutParam->slen = strtol(argv[5], 0, 0);
                if(pInutParam->slen > 8)
                {
                    (MT_VOID)MT_Diseqc2Print_help(argv[0]);
                    return MT_FAILURE;
                }
                pInutParam->rlen = strtol(argv[6], 0, 0);
                if(argc < 7 + pInutParam->slen)
                {
                    SAMPLE_DISEQC2_ERR_PRINT("Error in parameters \n");
                    return MT_FAILURE;
                }
                for(i = 0; i<pInutParam->slen; i++)
                {
                    pInutParam->mesg[i] = strtol(argv[7+i], 0, 0);
                }
                return MT_SUCCESS;


            default:
                (MT_VOID)MT_Diseqc2Print_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_DisEqc2Main(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32 ret = MT_SUCCESS;

    mt_input_diseqc2_para_t    sInputParam = {0};


    ret = MT_Diseqc2Parase_args(argc, argv, &sInputParam);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC2_ERR_PRINT("MT_Diseqc2Parase_args failed! \n");
        return ret;
    }

#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC2_ERR_PRINT("mt_sys_init failed! \n");
        return ret;
    }
#endif
    ret = MTADP_Fe_Init(sInputParam.tuner_id);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC2_ERR_PRINT("MTADP_Fe_Init failed! \n");
        goto ERR1;
    }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
#ifdef MT_SYM4_DSS
    ret = MT_Diseqc2SetParameter(sInputParam.tuner_id, MT_UNF_FE_SIG_TYPE_DVBS_AUTO, MT_UNF_TUNER_TYPE_M88TS6011, 0x58, MT_UNF_DEMOD_DEV_TYPE_M88DS6113, 0xD2, 1, 1, sInputParam);
#else
    ret = MT_Diseqc2SetParameter(sInputParam.tuner_id, 2048, 34, 88, 288, 24, 1, 0, sInputParam);
#endif

#elif defined CONFIG_MT_CHIP_SYMPHONY6
    if (sInputParam.tuner_id == INT_TUNER_ID_0)
    {
        ret = MT_Diseqc2SetParameter(sInputParam.tuner_id, MT_UNF_FE_SIG_TYPE_DVBS_AUTO, MT_UNF_TUNER_TYPE_M88TS6011,
            0x58, MT_UNF_DEMOD_DEV_TYPE_M88CS8800, 0xd0, MT_UNF_FE_OUTPUT_MODE_PARALLEL, 0, sInputParam);
    }
    else
    {
        ret = MT_Diseqc2SetParameter(sInputParam.tuner_id, MT_UNF_FE_SIG_TYPE_DVBS_AUTO, MT_UNF_TUNER_TYPE_M88RS6060,
            0x5a, MT_UNF_DEMOD_DEV_TYPE_M88RS6060, 0xd2, MT_UNF_FE_OUTPUT_MODE_SERIAL, 1, sInputParam);
    }
#endif
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC2_ERR_PRINT("MT_Diseqc2_SetParameter failed! \n");
        goto ERR2;
    }

    ret = MT_Diseqc2LNB_Test(sInputParam.tuner_id, 3, 1);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC2_ERR_PRINT("MT_Diseqc2_LNB_Test failed! \n");
        goto ERR2;
    }

    ret = MT_Diseqc2Sendrecv(sInputParam);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DISEQC2_ERR_PRINT("MT_senddrecv failed! \n");
        goto ERR2;
    }


ERR2:
    (MT_VOID)MTADP_Fe_DeInit(sInputParam.tuner_id);

ERR1:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    return MT_SUCCESS;

}

