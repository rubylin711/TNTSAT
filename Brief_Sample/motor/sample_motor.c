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
#ifdef MT_SAMPLE_MOTOR_DEBUG

#define MT_MOTOR_PRINT   printf
#else

#define MT_MOTOR_PRINT

#endif

#define SAMPLE_MOTOR_FUNCTION_ENTER()       MT_MOTOR_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_MOTOR_FUNCTION_EXIT()        MT_MOTOR_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_MOTOR_FATAL_PRINT(fmt...)    MT_MOTOR_PRINT(" [FATAL] " fmt)
#define SAMPLE_MOTOR_ERR_PRINT(fmt...)      MT_MOTOR_PRINT(" [ERROR] " fmt)
#define SAMPLE_MOTOR_WARN_PRINT(fmt...)     MT_MOTOR_PRINT(" [WARN] "  fmt)
#define SAMPLE_MOTOR_INFO_PRINT(fmt...)     MT_MOTOR_PRINT(" [INFO] "  fmt)
#define SAMPLE_MOTOR_DBG_PRINT(fmt...)      MT_MOTOR_PRINT(" [DEBUG] " fmt)


#define SAMPLE_MOTOR_PRINT  printf
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2


#define TUNER_ID_0          0

static MT_BOOL g_bTaskQuit = MT_TRUE;
static mt_u32 g_tuner_id = 0;


/*************************** Structure Definition ****************************/

#ifdef MT_SAMPLE_APP
MT_S32 MT_MotorMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif


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
static mt_s32 MT_MotorSetParameter(MT_U32 tuner_id, MT_U32 sig_type, MT_U32 tuner_dev_type, MT_U32 tuner_addr,
             MT_U32 demod_dev_type, MT_U32 demod_addr, MT_U32 out_put_mode, MT_U32 I2c_channel)
{
    MT_S32 ret = 0;
    mt_unf_fe_attr_t mtTunerAttr = { 0 };

    mt_sys_version_s stSysChipInfo = { 0 };


    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MOTOR_ERR_PRINT("mt_sys_get_version failed! \n");
        return ret;
    }
    SAMPLE_MOTOR_INFO_PRINT("chipVersion = 0x%x\n", stSysChipInfo.enChipVersion);

    ret = mt_unf_fe_get_default_attr(tuner_id, &mtTunerAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MOTOR_ERR_PRINT("mt_unf_fe_get_default_attr failed! \n");
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
        SAMPLE_MOTOR_INFO_PRINT("[%s %d]tuner_dev_type = %d, tuner_addr = 0x%x\n", __FUNCTION__, __LINE__, tuner_dev_type, tuner_addr);
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY6_A0)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
#endif
    else
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }

    mtTunerAttr.demod_dev_type = demod_dev_type;
    mtTunerAttr.demod_addr = demod_addr;
    mtTunerAttr.demod_i2c_id = I2c_channel;
    mtTunerAttr.output_mode = out_put_mode;
    mtTunerAttr.tuner_i2c_id[0] = 0;
    mtTunerAttr.no_need_init = 0;

    SAMPLE_MOTOR_INFO_PRINT("tuner_type[%d], tuner_addr[0x%x], demod_type[%d], demod_addr[0x%x], demod_id[%d], output_mode[%d] \n",
                            mtTunerAttr.tuner_type, mtTunerAttr.tuner_addr,
                            mtTunerAttr.demod_dev_type, mtTunerAttr.demod_addr,
                            mtTunerAttr.demod_i2c_id, mtTunerAttr.output_mode );

    ret = mt_unf_fe_set_attr(tuner_id, &mtTunerAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MOTOR_ERR_PRINT("mt_unf_fe_set_attr failed! \n");
        return ret;
    }
    return ret;

}

static void MT_MotorPrintMenu(MT_VOID)
{
    SAMPLE_MOTOR_PRINT("     w: Keep turning to the west \n");
    SAMPLE_MOTOR_PRINT("     e: Keep turning to the east \n");
    SAMPLE_MOTOR_PRINT("     a: Slowly turn to the west \n");
    SAMPLE_MOTOR_PRINT("     d: Slowly turn to the east \n");
    SAMPLE_MOTOR_PRINT("     z: Fast turn to the west \n");
    SAMPLE_MOTOR_PRINT("     x: Fast turn to the east \n");
    SAMPLE_MOTOR_PRINT("     t: Stop turning \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_MOTOR_PRINT("     b : background run \n");
#endif
    SAMPLE_MOTOR_PRINT("     h : help \n");
    SAMPLE_MOTOR_PRINT("     q : quit \n");
    SAMPLE_MOTOR_PRINT("Motor>> ");
}

/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static void MT_MotorPrint_help(char *name)
{
    SAMPLE_MOTOR_PRINT(" [ options ]...\n"
       "\n"
       "Options:\n"
       " ?/-h/-H        print this help\n"
       " -t <tuner>     set tuner id \n");
    SAMPLE_MOTOR_PRINT("example: %s -t 0 \n", name);
    SAMPLE_MOTOR_PRINT("         %s -q  <exit> \n", name);
}

static mt_s32 MT_MotorCmdTask(MT_VOID)
{
    mt_s32     ret = MT_SUCCESS;
    mt_s32     rotation_num = 0;
    MT_CHAR    inputCmd[32] = { 0 };
    MT_CHAR    *pfgetret = NULL;
    mt_unf_fe_diseqc_move_t stMove = { 0 };

    while(1)
    {
        (void)MT_MotorPrintMenu();

        pfgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        pfgetret = pfgetret;

        if('q' == inputCmd[0])
        {
            SAMPLE_MOTOR_INFO_PRINT("exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_MOTOR_INFO_PRINT("motor in back!\n");
            break;
        }
#endif
        else if ('w' == inputCmd[0])
        {
            memset(&stMove, 0, sizeof(stMove));
            stMove.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
            stMove.dir = MT_UNF_FE_DISEQC_MOVE_DIR_WEST;
            stMove.type = MT_UNF_FE_DISEQC_MOVE_CONTINUE;
            ret = mt_unf_fe_diseqc_move(g_tuner_id, &stMove);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MOTOR_ERR_PRINT("mt_unf_fe_diseqc_move failed! \n");
            }
        }
        else if ('e' == inputCmd[0])
        {
            memset(&stMove, 0, sizeof(stMove));
            stMove.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
            stMove.dir = MT_UNF_FE_DISEQC_MOVE_DIR_EAST;
            stMove.type = MT_UNF_FE_DISEQC_MOVE_CONTINUE;
            ret = mt_unf_fe_diseqc_move(g_tuner_id, &stMove);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MOTOR_ERR_PRINT("mt_unf_fe_diseqc_move failed! \n");
            }
        }
        else if ('a' == inputCmd[0])
        {
            memset(&stMove, 0, sizeof(stMove));
            stMove.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
            stMove.dir = MT_UNF_FE_DISEQC_MOVE_DIR_WEST;
            stMove.type = MT_UNF_FE_DISEQC_MOVE_STEP_SLOW;
            SAMPLE_MOTOR_INFO_PRINT("Please enter the motor rotation times \n");
            scanf("%d", &rotation_num);
            while(rotation_num--)
            {
                ret = mt_unf_fe_diseqc_move(g_tuner_id, &stMove);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_MOTOR_ERR_PRINT("mt_unf_fe_diseqc_move failed! \n");
                }
                MT_USLEEP(500 * 1000);
            }

        }
        else if ('d' == inputCmd[0])
        {
            memset(&stMove, 0, sizeof(stMove));
            stMove.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
            stMove.dir = MT_UNF_FE_DISEQC_MOVE_DIR_EAST;
            stMove.type = MT_UNF_FE_DISEQC_MOVE_STEP_SLOW;
            SAMPLE_MOTOR_INFO_PRINT("Please enter the motor rotation times \n");
            scanf("%d", &rotation_num);
            while(rotation_num--)
            {
                ret = mt_unf_fe_diseqc_move(g_tuner_id, &stMove);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_MOTOR_ERR_PRINT("mt_unf_fe_diseqc_move failed! \n");
                }
                MT_USLEEP(500 * 1000);
            }
        }
        else if ('z' == inputCmd[0])
        {
            memset(&stMove, 0, sizeof(stMove));
            stMove.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
            stMove.dir = MT_UNF_FE_DISEQC_MOVE_DIR_WEST;
            stMove.type = MT_UNF_FE_DISEQC_MOVE_STEP_FAST;
            SAMPLE_MOTOR_INFO_PRINT("Please enter the motor rotation times \n");
            scanf("%d", &rotation_num);
            while(rotation_num--)
            {
                ret = mt_unf_fe_diseqc_move(g_tuner_id, &stMove);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_MOTOR_ERR_PRINT("mt_unf_fe_diseqc_move failed! \n");
                }
                MT_USLEEP(500 * 1000);
            }
        }
        else if ('x' == inputCmd[0])
        {
            memset(&stMove, 0, sizeof(stMove));
            stMove.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
            stMove.dir = MT_UNF_FE_DISEQC_MOVE_DIR_EAST;
            stMove.type = MT_UNF_FE_DISEQC_MOVE_STEP_FAST;
            SAMPLE_MOTOR_INFO_PRINT("Please enter the motor rotation times \n");
            scanf("%d", &rotation_num);
            while(rotation_num--)
            {
                ret = mt_unf_fe_diseqc_move(g_tuner_id, &stMove);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_MOTOR_ERR_PRINT("mt_unf_fe_diseqc_move failed! \n");
                }
                MT_USLEEP(500 * 1000);
            }
        }
        else if ('t' == inputCmd[0])
        {

            ret = mt_unf_fe_diseqc_stop(g_tuner_id, MT_UNF_FE_DISEQC_LEVEL_1_X);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MOTOR_ERR_PRINT("mt_unf_fe_diseqc_stop failed! \n");
            }
        }
        else if ('h' == inputCmd[0])
        {
            SAMPLE_MOTOR_INFO_PRINT("Print help info \n");
            continue;
        }
    }

    return MT_SUCCESS;
}

static MT_VOID MT_MotorExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)mt_unf_fe_set_lnb_power(g_tuner_id, MT_UNF_FE_LNB_POWER_OFF);
    (MT_VOID)MTADP_Fe_DeInit(g_tuner_id);
}


static MT_S32 MT_MotorParase_args(int argc, char *argv[], mt_u32* tuner_id)
{
    int opt = 0;
    while((opt = MTADP_Getopt(argc, argv, "h?Hf:c:s:t:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                MT_MotorPrint_help(argv[0]);
                return MT_FAILURE;
            case 't':
                *tuner_id = strtol(mt_optarg, 0, 0);
                break;

            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_MotorExit();
                }
                return MT_TASK_EXIT;

            default:
                MT_MotorPrint_help(argv[0]);
                return MT_FAILURE;
        }
    }


    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_MotorMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32 ret = MT_SUCCESS;

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        MT_MotorPrint_help(argv[0]);
        return MT_SUCCESS;
    }

    ret = MT_MotorParase_args(argc, argv, &g_tuner_id);
    if (MT_FAILURE == ret)
    {
        SAMPLE_MOTOR_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_MOTOR_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MOTOR_ERR_PRINT("mt_sys_init failed! \n");
            return ret;
        }
#endif
        ret = MTADP_Fe_Init(g_tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MOTOR_ERR_PRINT("MTADP_Fe_Init failed! \n");
            goto ERR1;
        }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
#ifdef MT_SYM4_DSS
        ret = MT_MotorSetParameter(g_tuner_id, MT_UNF_FE_SIG_TYPE_DVBS_AUTO, MT_UNF_TUNER_TYPE_M88TS6011, 0x58, MT_UNF_DEMOD_DEV_TYPE_M88DS6113, 0xD2, 1, 1);
#else
        ret = MT_MotorSetParameter(g_tuner_id, 2048, 34, 88, 288, 24, 1, 0);
#endif
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        if (g_tuner_id == 0)
        {
            ret = MT_MotorSetParameter(g_tuner_id, 2048, 34, 88, 288, 24, 4, 0);
        }
        else
        {
            ret = MT_MotorSetParameter(g_tuner_id, 2048, 80, 90, 336, 210, 4, 1);
        }
#endif
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MOTOR_ERR_PRINT("MTADP_Fe_Init failed! \n");
            goto ERR2;
        }

        ret = mt_unf_fe_set_lnb_power(g_tuner_id, MT_UNF_FE_LNB_POWER_ON);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MOTOR_ERR_PRINT("mt_unf_fe_set_lnb_power failed! \n");
            goto ERR2;
        }

        g_bTaskQuit = MT_FALSE;
    }

    (MT_VOID)MT_MotorCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    (MT_VOID)mt_unf_fe_set_lnb_power(g_tuner_id, MT_UNF_FE_LNB_POWER_OFF);

ERR2:
    (MT_VOID)MTADP_Fe_DeInit(g_tuner_id);

ERR1:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    return MT_SUCCESS;

}

