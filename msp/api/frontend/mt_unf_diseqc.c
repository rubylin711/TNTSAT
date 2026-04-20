/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "mt_type.h"
#include "mt_common.h"
#include "mt_unf_frontend.h"
//#include "drv_frontend_ext.h"
#include "drv_frontend_ioctl.h"
#include "mt_error_mpi.h"

/*--------------------------- MACRO DECLARATIONS ---------------------------------------*/
#define UNF_TUNER_NUM 5

#define LO_FREQUENCY_L (0x0)
#define LO_FREQUENCY_H (0x1)
#define POLARISATION_V (0x0)
#define POLARISATION_H (0x2)
/*#define SAT_POSITION_A (0x0)
#define SAT_POSITION_B (0x4)
#define SWITCH_OPTION_A (0x0)
#define SWITCH_OPTION_B (0x8)*/
#define PORT_GROUP_CLR_BITS (0xF0)

//#define DISEQC_CMD_LEN (6)

#define FRAMING_BYTE (0)
#define ADDRESS_BYTE (1)
#define COMMAND_BYTE (2)
#define DATA_BYTE_0 (3)
#define DATA_BYTE_1 (4)
#define DATA_BYTE_2 (5)

#define MAX_LONGITUDE (1800)
#define MAX_LATITUDE (900)
#define ANGLE_MULTIPLE (10.0)

#define PI (3.14159265)
#define USALS_CONST_1 ((double)3561680000.0)
#define USALS_CONST_2 ((double)180.0)
#define USALS_CONST_3 ((double)6370.0)
#define USALS_CONST_4 ((double)40576900.0)
#define USALS_CONST_5 ((double)1821416900.0)
#define USALS_CONST_6 ((double)537628000.0)

#define DISEQC_DELAY_BETWEEN_CMD_MS (25)

#ifndef ABS
#define ABS(arg) ((arg) < 0 ? -(arg) : (arg))
#endif

#define FORMAT_DISEQC_CMD(a, F, A, C) \
    {                                 \
        a[FRAMING_BYTE] = F;          \
        a[ADDRESS_BYTE] = A;          \
        a[COMMAND_BYTE] = C;          \
    }

#define FORMAT_DISEQC_CMD_VALUE(a, F, A, C, aD, L) \
    {                                              \
        int cnt;                                     \
        a[FRAMING_BYTE] = F;                       \
        a[ADDRESS_BYTE] = A;                       \
        a[COMMAND_BYTE] = C;                       \
        for (cnt = 0; cnt < L; cnt++)                    \
        {                                          \
            a[DATA_BYTE_0 + cnt] = ((mt_u8 *)aD)[cnt]; \
        }                                          \
    }

/*-------------------- STATIC STRUCTURE DECLARATIONS -----------------------------------*/

/* DiSEqC Framing code */
//typedef enum
//{
#define MASTER_NOREPLY_FIRST 0xE0
//#define MASTER_NOREPLY_REPEAT      0xE1
#define MASTER_REPLY_FIRST 0xE2
/*#define MASTER_REPLY_REPEAT        0xE3
#define SLAVE_OK                   0xE4
#define SLAVE_NOT_SUPPORT          0xE5
#define SLAVE_PARITY_ERROR         0xE6
#define SLAVE_BAD_COMMAND          0xE7*/
//} DISEQC_FRAMING_E;

/* DiSEqC Device address */
//typedef enum
//{
#define DEVICE_ANY 0x00
#define DEVICE_ANY_LNB_SW_SMATV 0x10
/*#define DEVICE_LNB                      0x11
#define DEVICE_LNB_LOOPTHROUGH          0x12
#define DEVICE_SW                       0x14
#define DEVICE_SW_LOOPTHROUGH           0x15
#define DEVICE_SMATV                    0x18
#define DEVICE_ANY_POLARISER            0x20
#define DEVICE_LINEAR_POLARISER         0x21
#define DEVICE_ANY_POSITIONER           0x30*/
#define DEVICE_AZIMUTH_POSITIONER 0x31
/*#define DEVICE_ELEVATION_POSITIONER     0x32
#define DEVICE_ANY_INSTALLER_AID        0x40
#define DEVICE_SIGNAL_STRENGTH          0x41
#define DEVICE_ANY_INTELLIGENT          0x70
#define DEVICE_SUBSCRIBER_CONTROL       0x71
#define DEVICE_BUTT                     0x72*/
//} DISEQC_DEVICE_E;

/* DiSEqC Command */
//typedef enum
//{
/* M*R/1.0 */
#define CMD_RESET 0x00 /**<Reset DiSEqC microcontroller*/
                       /* R/2.0 */
//#define CMD_CLR_RESET         0x01   /**<Clear the "Reset" flag*/
/* R/1.0 */
#define CMD_STANDBY 0x02  /**<Switch peripheral power supply off*/
#define CMD_POWER_ON 0x03 /**<Switch peripheral power supply on*/
                          /* S/2.0 */
//#define CMD_SET_CONTEND       0x04 /**<Set Contention flag*/
//#define CMD_CONTEND           0x05     /**<Return address only if Contention flag is set*/
/* R/2.0 */
//#define CMD_CLR_CONTEND       0x06 /**<Clear Contention flag*/
/* S/2.0 */
//#define CMD_ADDRESS           0x07     /**<Return address unless Contention flag is set*/
//#define CMD_MOVE_C            0x08      /**<Change address only if Contention flag is set*/
/* R/2.0 */
#if 0
#define CMD_MOVE 0x90     /**<Change address unless Contention flag is set*/
#define CMD_STATUS 0x10   /**<Read Status register flags*/
#define CMD_CONFIG 0x11   /**<Read Configuration flags (peripheral hardware)*/
#define CMD_SWITCH_0 0x14 /**<Read Switching state flags (Committed port)*/
#define CMD_SWITCH_1 0x15 /**<Read Switching state flags (Uncommitted port)*/
#define CMD_SWITCH_2 0x16
#define CMD_SWITCH_3 0x17

        /* R/1.0 */
#define CMD_SET_LO 0x20    /**<Select the Low Local Oscillator frequency*/
#define CMD_SET_VR 0x21    /**<Select Vertical Polarisation (or Right circular)*/
#define CMD_SET_POS_A 0x22 /**<Select Satellite position A (or position C)*/
#define CMD_SET_S0_A 0x23  /**<Select Switch Option A (e.g. positions A/B)*/
#define CMD_SET_HI 0x24    /**<Select the High Local Oscillator frequency*/
#define CMD_SET_HL 0x25    /**<Select Horizontal Polarisation (or Left circular)*/
#define CMD_SET_POS_B 0x26 /**<Select Satellite position B (or position D)*/
#define CMD_SET_S0_B 0x27  /**<Select Switch Option B (e.g. positions C/D)*/
        /* R/1.1 */
#define CMD_SET_S1_A 0x28  /**<Select switch S1 input A (deselect input B)*/
#define CMD_SET_S2_A 0x29  /**<Select switch S2 input A (deselect input B)*/
#define CMD_SET_S3_A 0x2A  /**<Select switch S3 input A (deselect input B)*/
#define CMD_SET_S4_A 0x2B  /**<Select switch S4 input A (deselect input B)*/
#define CMD_SET_S1_B 0x2C  /**<Select switch S1 input B (deselect input A)*/
#define CMD_SET_S2_B 0x2D  /**<Select switch S2 input B (deselect input A)*/
#define CMD_SET_S3_B 0x2E  /**<Select switch S3 input B (deselect input A)*/
#define CMD_SET_S4_B 0x2F  /**<Select switch S4 input B (deselect input A)*/
#define CMD_SLEEP 0x30     /**<Ignore all bus commands except "Awake"*/
#define CMD_AWAKE 0x31     /**<Respond to future bus commands normally*/
#endif
/* M/1.0 */
#define CMD_WRITE_N0 0x38 /**<Write to Port group 0 (Committed switches)*/
                          /* M/1.1 */
#define CMD_WRITE_N1 0x39 /**<Write to Port group 1 (Uncommitted switches)*/
//#define CMD_WRITE_N2          0x3A
//#define CMD_WRITE_N3          0x3B

/* S/2.0 */
#if 0
#define CMD_READ_A0 0x40   /**<Read Analogue value A0*/
#define CMD_READ_A1 0x41   /**<Read Analogue value A1*/
        /* R/1.2 */
#define CMD_WRITE_A0 0x48  /**<Write Analogue value A0 (e.g. Skew)*/
        /* S/1.2 */
#define CMD_WRITE_A1 0x49  /**<Write Analogue value A1*/
        /* S/2.0 */
#define CMD_LO_STRING 0x50 /**<Read current frequency [Reply = BCD string]*/
        /* R/2.0 */
#define CMD_LO_NOW 0x51    /**<Read current frequency table entry number*/
        /* S/2.0 */
#define CMD_LO_LO 0x52     /**<Read Lo frequency table entry number*/
#define CMD_LO_HI 0x53     /**<Read Hi frequency table entry number*/
#endif
/* M/1.1 */
//#define CMD_WRITE_RREQ        0x58  /**<Write channel frequency (BCD string)*/
//#define CMD_CH_NO             0x59       /**<Write (receiver's) selected channel number*/
/* M/1.2 */
#define CMD_HALT 0x60       /**<Stop Positioner movement*/
#define CMD_LIMITS_OFF 0x63 /**<Disable Limits*/
                            /* R/2.2 */
//#define CMD_POS_STAT          0x64    /**<Read Positioner Status Register*/
/* M/1.2 */
#define CMD_LIMIT_E 0x66    /**<Set East Limit (& Enable recommended)*/
#define CMD_LIMIT_W 0x67    /**<Set West Limit (& Enable recommended)*/
#define CMD_DRIVE_EAST 0x68 /**<Drive Motor East (with optional timeout/steps)*/
#define CMD_DRIVE_WEST 0x69 /**<Drive Motor West (with optional timeout/steps)*/
#define CMD_STORE_NN 0x6A   /**<Store Satellite Position & Enable Limits*/
#define CMD_GOTO_NN 0x6B    /**<Drive Motor to Satellite Position nn*/
#define CMD_GOTO_XX 0x6E    /**<Drive Motor to Angular Position*/
                            /* R/1.2 */
#define CMD_SET_POSNS 0x6F  /**<(Re-)Calculate Satellite Positions*/
//#define CMD_BUTT
//} DISEQC_CMD_E;

typedef struct
{
    mt_unf_fe_diseqc_switch4port_t stPort1_0;
    mt_unf_fe_diseqc_switch16port_t stPort1_1;
} DISEQC_STATUS_S;

/*------------------------- GLOBAL DECLARATIONS ----------------------------------------*/

extern mt_unf_fe_switch_toneburst_t fe_diseqc_get_toneburst_status(mt_u32 tuner_id);

/*------------------------- STATIC DECLARATIONS ----------------------------------------*/

static DISEQC_STATUS_S s_stDiSEqCStatus[UNF_TUNER_NUM] =
    {
        {.stPort1_0 =
             {
                 MT_UNF_FE_DISEQC_LEVEL_1_X,
                 MT_UNF_FE_DISEQC_SWITCH_NONE,
                 MT_UNF_FE_POLARIZATION_H,
                 MT_UNF_FE_LNB_22K_OFF},
         .stPort1_1 =
             {
                 MT_UNF_FE_DISEQC_LEVEL_1_X,
                 MT_UNF_FE_DISEQC_SWITCH_NONE}},
        {.stPort1_0 =
             {
                 MT_UNF_FE_DISEQC_LEVEL_1_X,
                 MT_UNF_FE_DISEQC_SWITCH_NONE,
                 MT_UNF_FE_POLARIZATION_H,
                 MT_UNF_FE_LNB_22K_OFF},
         .stPort1_1 =
             {
                 MT_UNF_FE_DISEQC_LEVEL_1_X,
                 MT_UNF_FE_DISEQC_SWITCH_NONE}},
        {.stPort1_0 =
             {
                 MT_UNF_FE_DISEQC_LEVEL_1_X,
                 MT_UNF_FE_DISEQC_SWITCH_NONE,
                 MT_UNF_FE_POLARIZATION_H,
                 MT_UNF_FE_LNB_22K_OFF},
         .stPort1_1 =
             {
                 MT_UNF_FE_DISEQC_LEVEL_1_X,
                 MT_UNF_FE_DISEQC_SWITCH_NONE}}};

/*------------------------------------ CODE --------------------------------------------*/
extern mt_s32 fe_diseqc_sendrecv_message(mt_u32 tuner_id,
                                         const mt_unf_fe_diseqc_sendmsg_t *pstSendMsg,
                                         mt_unf_fe_diseqc_recvmsg_t *pstRecvMsg);

static mt_s32 DISEQC_SendCmd2_0(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch4port_t *pstPara, mt_unf_fe_diseqc2_rsmsg_info *msg) //diseqc2.0
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8PortGroupBits;
    mt_u8 u8CMD;
    mt_u8 u8Framing;
    mt_u8 u8Polar;
    mt_u8 u8LOFreq;
    /*mt_u8 i = 0; Clean Warning*/
    //MT_ERR_FE("DISEQC_SendCmd2_0 start2 sendlen:%d,recvlen:%d\n",msg->slen,msg->rlen);

    if (pstPara->level == MT_UNF_FE_DISEQC_LEVEL_2_X)
    {
        if ((msg == NULL) || (msg->rcvmsg == NULL))
        {
            printf("receive msg buff is null\n");
            return MT_FAILURE;
        }

        /* Polarization */
        if ((MT_UNF_FE_POLARIZATION_V == pstPara->polar) || (MT_UNF_FE_POLARIZATION_R == pstPara->polar))
        {
            u8Polar = POLARISATION_V;
        }
        else
        {
            u8Polar = POLARISATION_H;
        }

        /* Init parameter */
        memset(&stSendMsg, 0, sizeof(stSendMsg));
        memset(&stRecvMsg, 0, sizeof(stRecvMsg));

        stSendMsg.level = pstPara->level;
        stSendMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
        stSendMsg.len = msg->slen;
        stSendMsg.repeat_times = 0;

        stRecvMsg.len = msg->rlen;

        memcpy(&stSendMsg.data, msg->sendmsg, (sizeof(mt_u8) * (msg->slen)));
        //for(i=0;i<msg->slen;i++)
        //    MT_ERR_FE("stSendMsg.data[%d]=%d\n",i,stSendMsg.data[i]);

        //MT_ERR_FE("len:%d,msg:0x%x,rcvmsg:0x%x\n",stRecvMsg.len,stRecvMsg.msg,rcvmsg);
        s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_FE("Send Recv mssage fail,status:%d\n", stRecvMsg.status);
            return s32Ret;
        }

        //for(i = 0; i < msg->rlen; i++)
        //    MT_ERR_FE("stRecvMsg.data[%d]=%d,status:%d,len:%d\n",i,stRecvMsg.msg[i],stRecvMsg.status,stRecvMsg.len);

        memcpy(msg->rcvmsg, &(stRecvMsg.msg), stRecvMsg.len);
    }
    else
    {
        /* Polarization */
        if ((MT_UNF_FE_POLARIZATION_V == pstPara->polar) || (MT_UNF_FE_POLARIZATION_R == pstPara->polar))
        {
            u8Polar = POLARISATION_V;
        }
        else
        {
            u8Polar = POLARISATION_H;
        }

        /* LO, 22K */
        u8LOFreq = (MT_UNF_FE_LNB_22K_ON == pstPara->lnb_22k) ? LO_FREQUENCY_H : LO_FREQUENCY_L;

        /* Init parameter */
        memset(&stSendMsg, 0, sizeof(stSendMsg));
        //memset(&stRecvMsg, 0, sizeof(stRecvMsg));
        stSendMsg.level = pstPara->level;
        stSendMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
        stSendMsg.len = 4;
        stSendMsg.repeat_times = 0;

        u8Framing = (pstPara->level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;

        /* Send WRITE N0(0x38) command */
        u8PortGroupBits = (mt_u8)(PORT_GROUP_CLR_BITS | (((mt_u8)(pstPara->port - 1)) << 2) | u8Polar | u8LOFreq);
        u8CMD = CMD_WRITE_N0;
        FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, u8Framing, DEVICE_ANY_LNB_SW_SMATV, u8CMD, &u8PortGroupBits, 1);

        s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, NULL /*&stRecvMsg*/);
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_FE("Send WRITE N0 fail.\n");
            return s32Ret;
        }
    }

/* If support level 2.x, handle received message here. */
#endif
    return MT_SUCCESS;
}

static mt_s32 DISEQC_SendCmd1_0(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch4port_t *pstPara)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    //mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8PortGroupBits;
    mt_u8 u8CMD;
    mt_u8 u8Framing;
    mt_u8 u8Polar;
    mt_u8 u8LOFreq;

    /* Polarization */
    if ((MT_UNF_FE_POLARIZATION_V == pstPara->polar) || (MT_UNF_FE_POLARIZATION_R == pstPara->polar))
    {
        u8Polar = POLARISATION_V;
    }
    else
    {
        u8Polar = POLARISATION_H;
    }

    /* LO, 22K */
    u8LOFreq = (MT_UNF_FE_LNB_22K_ON == pstPara->lnb_22k) ? LO_FREQUENCY_H : LO_FREQUENCY_L;

    /* Init parameter */
    memset(&stSendMsg, 0, sizeof(stSendMsg));
    //memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    stSendMsg.level = pstPara->level;
    stSendMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
    stSendMsg.len = 4;
    stSendMsg.repeat_times = 0;

    u8Framing = (pstPara->level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;

    /* Send WRITE N0(0x38) command */
    u8PortGroupBits = (mt_u8)(PORT_GROUP_CLR_BITS | (((mt_u8)(pstPara->port - 1)) << 2) | u8Polar | u8LOFreq);
    u8CMD = CMD_WRITE_N0;
    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, u8Framing, DEVICE_ANY_LNB_SW_SMATV, u8CMD, &u8PortGroupBits, 1);

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, NULL /*&stRecvMsg*/);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send WRITE N0 fail.\n");
        return s32Ret;
    }

/* If support level 2.x, handle received message here. */
#endif
    return MT_SUCCESS;
}

static mt_s32 DISEQC_SendCmd1_1(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch16port_t *pstPara)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    //mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8PortGroupBits;
    mt_u8 u8CMD;
    mt_u8 u8Framing;

    /* Init parameter */
    memset(&stSendMsg, 0, sizeof(stSendMsg));
    //memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    stSendMsg.level = pstPara->level;
    stSendMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
    stSendMsg.len = 4;
    stSendMsg.repeat_times = 0;

    u8Framing = (pstPara->level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;

    /* Send WRITE N1(0x39) command */
    u8PortGroupBits = (mt_u8)(PORT_GROUP_CLR_BITS | (pstPara->port - 1));
    u8CMD = CMD_WRITE_N1;
    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, u8Framing, DEVICE_ANY_LNB_SW_SMATV, u8CMD, &u8PortGroupBits, 1);

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, NULL /*&stRecvMsg*/);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send WRITE N1 fail.\n");
        return s32Ret;
    }

/* If support level 2.x, handle received message here. */
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_switch4port(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch4port_t *p_prm)
{
#if 1
    mt_s32 s32Ret;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_FAILURE; //MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_prm)
    {
        MT_ERR_FE("Input parameter(pstPara) invalid\n");
        return MT_FAILURE; //MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_prm->level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", p_prm->level);
        return MT_FAILURE; //MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_DISEQC_SWITCH_PORT_5 <= p_prm->port)
    {
        MT_ERR_FE("Switch port invalid: %d\n", p_prm->port);
        return MT_FAILURE; //MT_ERR_FE_INVALID_PARA;
    }

    /* Save port parameter */
    s_stDiSEqCStatus[tuner_id].stPort1_0 = *p_prm;

    /* If NONE, only save. */
    if (MT_UNF_FE_DISEQC_SWITCH_NONE == p_prm->port)
    {
        return MT_SUCCESS;
    }

    /* If use 4port device, other parameter must be valid. */
    if (MT_UNF_FE_POLARIZATION_BUTT <= p_prm->polar)
    {
        MT_ERR_FE("Polarization invalid: %d\n", p_prm->polar);
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_LNB_22K_BUTT <= p_prm->lnb_22k)
    {
        MT_ERR_FE("LNB 22K invalid: %d\n", p_prm->lnb_22k);
        return MT_ERR_FE_INVALID_PARA;
    }

    /* Switch 1.0 */
    s32Ret = DISEQC_SendCmd1_0(tuner_id, p_prm);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send cmd 1.0 fail.\n");
        return s32Ret;
    }

    /*
     * If has switch 1.1, set it here.
     * Support Tuner - DiSEqC1.0 - DiSEqC1.1 - Other switch - LNB cascaded.
     */
    if (MT_UNF_FE_DISEQC_SWITCH_NONE != s_stDiSEqCStatus[tuner_id].stPort1_1.port)
    {
        MT_USLEEP(DISEQC_DELAY_BETWEEN_CMD_MS * 1000);
        s32Ret = DISEQC_SendCmd1_1(tuner_id, &(s_stDiSEqCStatus[tuner_id].stPort1_1));
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_FE("Send cmd 1.1 fail.\n");
            return s32Ret;
        }
    }
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_switch16port(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch16port_t *p_prm)
{
#if 1
    mt_s32 s32Ret;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_prm)
    {
        MT_ERR_FE("Input parameter(pstPara) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_prm->level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", p_prm->level);
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_DISEQC_SWITCH_PORT_BUTT <= p_prm->port)
    {
        MT_ERR_FE("Switch port invalid: %d\n", p_prm->port);
        return MT_ERR_FE_INVALID_PARA;
    }

    /* Save port parameter */
    s_stDiSEqCStatus[tuner_id].stPort1_1 = *p_prm;

    /* If NONE, return. */
    if (MT_UNF_FE_DISEQC_SWITCH_NONE == p_prm->port)
    {
        return MT_SUCCESS;
    }

    /* If have 1.0 switch, set if first */
    if (MT_UNF_FE_DISEQC_SWITCH_NONE != s_stDiSEqCStatus[tuner_id].stPort1_0.port)
    {
        s32Ret = DISEQC_SendCmd1_0(tuner_id, &(s_stDiSEqCStatus[tuner_id].stPort1_0));
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_FE("Send cmd 1.0 fail.\n");
            return s32Ret;
        }

        MT_USLEEP(DISEQC_DELAY_BETWEEN_CMD_MS * 1000);
    }

    /* Switch 1.1 */
    s32Ret = DISEQC_SendCmd1_1(tuner_id, p_prm);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send cmd 1.1 fail.\n");
        return s32Ret;
    }

    /*
     * If have 1.0 switch, repeat.
     * Support Tuner - DiSEqC1.1 - DiSEqC1.0 - Other switch - LNB cascaded.
     */
    if (MT_UNF_FE_DISEQC_SWITCH_NONE != s_stDiSEqCStatus[tuner_id].stPort1_0.port)
    {
        MT_USLEEP(DISEQC_DELAY_BETWEEN_CMD_MS * 1000);
        s32Ret = DISEQC_SendCmd1_0(tuner_id, &(s_stDiSEqCStatus[tuner_id].stPort1_0));
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_FE("Send cmd 1.0 fail.\n");
            return s32Ret;
        }
    }
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_storepos(mt_u32 tuner_id, const mt_unf_fe_diseqc_position_t *p_prm)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8Framing;
    mt_u8 u8Pos;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_prm)
    {
        MT_ERR_FE("Input parameter(pstPara) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_prm->level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", p_prm->level);
        return MT_ERR_FE_INVALID_PARA;
    }

    if (DISEQC_MAX_MOTOR_PISITION < p_prm->pos)
    {
        MT_ERR_FE("Input parameter(pstPara->u32Pos) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    memset(&stSendMsg, 0, sizeof(stSendMsg));
    memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    u8Framing = (p_prm->level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;
    u8Pos = (mt_u8)p_prm->pos;

    stSendMsg.level = p_prm->level;
    stSendMsg.tone_burst = MT_UNF_FE_SWITCH_TONEBURST_NONE;
    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, u8Framing, DEVICE_AZIMUTH_POSITIONER, CMD_STORE_NN, &u8Pos, 1);
    stSendMsg.len = 4;
    stSendMsg.repeat_times = 0;

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send Store N fail.\n");
        return s32Ret;
    }

    /* TODO: If level 2.x, handle received message here. */

    return s32Ret;
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_goto_pos(mt_u32 tuner_id, const mt_unf_fe_diseqc_position_t *p_prm)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8Framing;
    mt_u8 u8Pos;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_prm)
    {
        MT_ERR_FE("Input parameter(pstPara) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_prm->level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", p_prm->level);
        return MT_ERR_FE_INVALID_PARA;
    }

    if (p_prm->pos > DISEQC_MAX_MOTOR_PISITION)
    {
        MT_ERR_FE("Input parameter(pstPara->u32Pos) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    memset(&stSendMsg, 0, sizeof(stSendMsg));
    memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    u8Framing = (p_prm->level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;
    u8Pos = (mt_u8)p_prm->pos;

    stSendMsg.level = p_prm->level;
    stSendMsg.tone_burst = MT_UNF_FE_SWITCH_TONEBURST_NONE;
    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, u8Framing, DEVICE_AZIMUTH_POSITIONER, CMD_GOTO_NN, &u8Pos, 1);
    stSendMsg.len = 4;
    stSendMsg.repeat_times = 0;

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send Goto N fail.\n");
        return s32Ret;
    }

    /* TODO: If level 2.x, handle received message here. */

    return s32Ret;
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_set_limit(mt_u32 tuner_id, const mt_unf_fe_diseqc_limit_t *p_prm)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8Framing;
    mt_u8 u8CMD;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_prm)
    {
        MT_ERR_FE("Input parameter(pstPara) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_prm->level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", p_prm->level);
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_DISEQC_LIMIT_BUTT <= p_prm->limit)
    {
        MT_ERR_FE("Input parameter(pstPara->enLimit) invalid: %d\n", p_prm->limit);
        return MT_ERR_FE_INVALID_PARA;
    }

    memset(&stSendMsg, 0, sizeof(stSendMsg));
    memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    u8Framing = (p_prm->level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;
    switch (p_prm->limit)
    {
    case MT_UNF_FE_DISEQC_LIMIT_OFF:
    default:
        u8CMD = CMD_LIMITS_OFF;
        break;

    case MT_UNF_FE_DISEQC_LIMIT_EAST:
        u8CMD = CMD_LIMIT_E;
        break;

    case MT_UNF_FE_DISEQC_LIMIT_WEST:
        u8CMD = CMD_LIMIT_W;
        break;
    }

    stSendMsg.level = p_prm->level;
    stSendMsg.tone_burst = MT_UNF_FE_SWITCH_TONEBURST_NONE;
    FORMAT_DISEQC_CMD(stSendMsg.data, u8Framing, DEVICE_AZIMUTH_POSITIONER, u8CMD);
    stSendMsg.len = 3;
    stSendMsg.repeat_times = 0;

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send limit cmd fail.\n");
        return s32Ret;
    }

    /* TODO: If level 2.x, handle received message here. */

    return s32Ret;
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_move(mt_u32 tuner_id, const mt_unf_fe_diseqc_move_t *p_prm)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8Framing;
    mt_u8 u8CMD;
    mt_u8 u8Value;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_prm)
    {
        MT_ERR_FE("Input parameter(pstPara) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_prm->level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", p_prm->level);
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_DISEQC_MOVE_DIR_BUTT <= p_prm->dir)
    {
        MT_ERR_FE("Input parameter(pstPara->enDir) invalid, %d\n", p_prm->dir);
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_DISEQC_MOVE_TYPE_BUTT <= p_prm->type)
    {
        MT_ERR_FE("Input parameter(pstPara->enType) invalid, %d\n", p_prm->type);
        return MT_ERR_FE_INVALID_PARA;
    }

    memset(&stSendMsg, 0, sizeof(stSendMsg));
    memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    u8Framing = (p_prm->level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;

    /* Direction */
    switch (p_prm->dir)
    {
    case MT_UNF_FE_DISEQC_MOVE_DIR_EAST:
    case MT_UNF_FE_DISEQC_MOVE_DIR_BUTT:
    default:
        u8CMD = CMD_DRIVE_EAST;
        break;

    case MT_UNF_FE_DISEQC_MOVE_DIR_WEST:
        u8CMD = CMD_DRIVE_WEST;
        break;
    }

    /* Value */
    switch (p_prm->type)
    {
    case MT_UNF_FE_DISEQC_MOVE_STEP_SLOW:
    case MT_UNF_FE_DISEQC_MOVE_TYPE_BUTT:
    default:
        u8Value = 0xff;
        break;

    case MT_UNF_FE_DISEQC_MOVE_STEP_FAST:
        u8Value = 0xfb;
        break;

    case MT_UNF_FE_DISEQC_MOVE_CONTINUE:
        u8Value = 0x00;
        break;
    }

    stSendMsg.level = p_prm->level;
    stSendMsg.tone_burst = MT_UNF_FE_SWITCH_TONEBURST_NONE;
    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, u8Framing, DEVICE_AZIMUTH_POSITIONER, u8CMD, &u8Value, 1);
    stSendMsg.len = 4;
    stSendMsg.repeat_times = 0;

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send drive cmd fail.\n");
        return s32Ret;
    }

    /* TODO: If level 2.x, handle received message here. */

    return s32Ret;
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_stop(mt_u32 tuner_id, const mt_unf_fe_diseqc_level_t enable_level)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8Framing;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= enable_level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", enable_level);
        return MT_ERR_FE_INVALID_PARA;
    }

    memset(&stSendMsg, 0, sizeof(stSendMsg));
    memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    u8Framing = (enable_level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;

    stSendMsg.level = enable_level;
    stSendMsg.tone_burst = MT_UNF_FE_SWITCH_TONEBURST_NONE;
    FORMAT_DISEQC_CMD(stSendMsg.data, u8Framing, DEVICE_AZIMUTH_POSITIONER, CMD_HALT);
    stSendMsg.len = 3;
    stSendMsg.repeat_times = 0;

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send halt fail.\n");
        return s32Ret;
    }

    /* TODO: If level 2.x, handle received message here. */

    return s32Ret;
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_recalculate(mt_u32 tuner_id, const mt_unf_fe_diseqc_recalculate_t *p_prm)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8Framing;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_prm)
    {
        MT_ERR_FE("Input parameter(pstPara) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_prm->level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", p_prm->level);
        return MT_ERR_FE_INVALID_PARA;
    }

    memset(&stSendMsg, 0, sizeof(stSendMsg));
    memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    u8Framing = (p_prm->level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;
    stSendMsg.level = p_prm->level;
    stSendMsg.tone_burst = MT_UNF_FE_SWITCH_TONEBURST_NONE;
    stSendMsg.data[FRAMING_BYTE] = u8Framing;
    stSendMsg.data[ADDRESS_BYTE] = DEVICE_AZIMUTH_POSITIONER;
    stSendMsg.data[COMMAND_BYTE] = CMD_SET_POSNS;
    stSendMsg.data[DATA_BYTE_0] = p_prm->para1;
    stSendMsg.data[DATA_BYTE_1] = p_prm->para2;
    stSendMsg.data[DATA_BYTE_2] = p_prm->para3;

    if ((0 == p_prm->para2) && (0 == p_prm->para3))
    {
        stSendMsg.len = 4;
    }
    else
    {
        stSendMsg.len = 6;
    }
    stSendMsg.repeat_times = 0;

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send Set Posns fail.\n");
        return s32Ret;
    }

    /* TODO: If level 2.x, handle received message here. */

    return s32Ret;
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_calc_angular(mt_u32 tuner_id, mt_unf_fe_diseqc_usals_para_t *p_prm)
{
#if 0
    mt_u8 u8Byte1 = 0;
    mt_u8 u8Byte2 = 0;
    mt_u8 u8RotateAngle;
    mt_u8 u8Fractional;
    mt_u16 u16Long;
    mt_u16 u16Lat;
    mt_u16 u16SatLong;
    mt_double dSatAngle;
    mt_double dLongitude;
    mt_double dLatitude;
    mt_double dT1;
    mt_double dT2;
    mt_double dT3;
    mt_double dT4;
    mt_double dTemp;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_prm)
    {
        MT_ERR_FE("Input parameter(pstPara) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (p_prm->local_longitude > 2*MAX_LONGITUDE)
    {
        MT_ERR_FE("Input parameter(pstPara->u16LocalLongitude) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (p_prm->local_latitude > 2*MAX_LATITUDE)
    {
        MT_ERR_FE("Input parameter(pstPara->u16LocalLatitude) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (p_prm->sat_longitude > 2*MAX_LONGITUDE)
    {
        MT_ERR_FE("Input parameter(pstPara->u16SatLongitude) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    /* Convert angle */
    u16Long = p_prm->local_longitude;
    u16Lat = p_prm->local_latitude;
    u16SatLong = p_prm->sat_longitude;
    dLongitude = ((u16Long < MAX_LONGITUDE) ? u16Long : -(2*MAX_LONGITUDE - u16Long)) / ANGLE_MULTIPLE;
    dLatitude = ((u16Lat < MAX_LATITUDE) ? u16Lat : -(2*MAX_LATITUDE - u16Lat)) / ANGLE_MULTIPLE;
    dSatAngle = ((u16SatLong < MAX_LONGITUDE) ? u16SatLong : -(2*MAX_LONGITUDE - u16SatLong)) / ANGLE_MULTIPLE;

    MT_ERR_FE("dLongitude=%f, dLatitude=%f, dSatAngle=%f\n", dLongitude, dLatitude, dSatAngle);

    dTemp = dSatAngle - dLongitude;
    dT1 = (mt_double)(USALS_CONST_1 * cosf((mt_float)(PI * ABS(dTemp) / USALS_CONST_2)));
    dT1 = (mt_double) sqrtf((mt_float)(USALS_CONST_1 - dT1));
    dT1 = dT1 * dT1;
    dT2 = USALS_CONST_3 * sinf((mt_float)(PI * dLatitude / USALS_CONST_2));
    dT2 = dT2 * dT2;
    dT3 = (mt_double) sqrtf((mt_float)(USALS_CONST_4 - dT2));
    dT3 = (mt_double) dT3 * sinf((mt_float)(PI * ABS (dTemp) / USALS_CONST_2));
    dT3 = dT3 * dT3;
    dT3 = (mt_double) sqrtf((mt_float)(dT2 + dT3));
    dT3 = (asinf((mt_float)(dT3/USALS_CONST_3)) * USALS_CONST_2) / PI;
    dT3 = (mt_double) sqrtf((mt_float)(USALS_CONST_5 - (USALS_CONST_6 * cosf((mt_float)(PI * dT3 / USALS_CONST_2)))));
    dT4 = (mt_double) sqrtf((mt_float)(USALS_CONST_5 - (USALS_CONST_6 * cosf((mt_float)(PI * dLatitude / USALS_CONST_2)))));
    dT4 = (acosf((mt_float)(((dT3 * dT3) + (dT4 * dT4) - dT1) / (2.0 * dT3 * dT4))) * USALS_CONST_2) / PI;

    /* Handle negative case */
    if (((dLatitude > 0) && 
           (((dSatAngle<dLongitude) && (ABS(dLongitude-dSatAngle)<180.0)) || 
             ((dSatAngle>dLongitude) && (ABS(dLongitude-dSatAngle)>180.0)))) || 
         ((dLatitude <= 0) && 
           (((dSatAngle>dLongitude) && (ABS(dLongitude-dSatAngle)<180.0)) || 
             ((dSatAngle<dLongitude) && (ABS(dLongitude-dSatAngle)>180.0)))))
    {
        dT4 = -dT4;
    }

    u8RotateAngle = (mt_u8) ABS(dT4);
    u8Fractional = (mt_u8)((ABS(dT4) - (double) u8RotateAngle) * ANGLE_MULTIPLE);

    /*
     * Generally, mator can't support rotation angle > 85;
     * and, if the difference between local longitude and satellite longitude is too large, 
     * the calculate result may be 0.
     */
    if ((u8RotateAngle > 85) || 
        ((p_prm->sat_longitude != p_prm->local_longitude) && (u8RotateAngle==0) && (u8Fractional==0)))
    {
        MT_ERR_FE("Rotation angle too large!\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    /* Handle the first nibble */
    u8Byte1 = ((dT4 < 0) ? 0xD0 : 0xE0) | (u8RotateAngle >> 4);
    u8Byte2 = (u8RotateAngle & 0xF) << 4;

    /* According to section 3.10 of <positioner_appli_notice.pdf> */
    switch (u8Fractional)
    {
    case 0:
        u8Byte2 |= 0x0;
        break;
    case 1:
        u8Byte2 |= 0x2;
        break;
    case 2:
        u8Byte2 |= 0x3;
        break;
    case 3:
        u8Byte2 |= 0x5;
        break;
    case 4:
        u8Byte2 |= 0x6;
        break;
    case 5:
        u8Byte2 |= 0x8;
        break;
    case 6:
        u8Byte2 |= 0xA;
        break;
    case 7:
        u8Byte2 |= 0xB;
        break;
    case 8:
        u8Byte2 |= 0xD;
        break;
    case 9:
        u8Byte2 |= 0xE;
        break;
    default:
        u8Byte2 |= 0x0;
        break;
    }

    p_prm->angular  = 0;
    p_prm->angular |= (mt_u16)u8Byte1 << 8;
    p_prm->angular |= u8Byte2;
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_goto_angular(mt_u32 tuner_id, const mt_unf_fe_diseqc_usals_angular_t *p_prm)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8Framing;
    mt_u8 u8Value[2];

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_prm)
    {
        MT_ERR_FE("Input parameter(pstPara) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_prm->level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", p_prm->level);
        return MT_ERR_FE_INVALID_PARA;
    }

    memset(&stSendMsg, 0, sizeof(stSendMsg));
    memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    u8Framing = (p_prm->level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;

    stSendMsg.level = p_prm->level;
    stSendMsg.tone_burst = MT_UNF_FE_SWITCH_TONEBURST_NONE;
    u8Value[0] = (mt_u8)(p_prm->angular >> 8);
    u8Value[1] = (mt_u8)(p_prm->angular);
    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, u8Framing, DEVICE_AZIMUTH_POSITIONER, CMD_GOTO_XX, u8Value, 2);
    stSendMsg.len = 5;
    stSendMsg.repeat_times = 0;

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send Goto XX fail.\n");
        return s32Ret;
    }

    /* TODO: If level 2.x, handle received message here. */

    return s32Ret;
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_reset(mt_u32 tuner_id, mt_unf_fe_diseqc_level_t enable_level)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8Framing;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= enable_level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", enable_level);
        return MT_ERR_FE_INVALID_PARA;
    }

    memset(&stSendMsg, 0, sizeof(stSendMsg));
    memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    u8Framing = (enable_level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;
    stSendMsg.level = enable_level;
    stSendMsg.tone_burst = MT_UNF_FE_SWITCH_TONEBURST_NONE;
    FORMAT_DISEQC_CMD(stSendMsg.data, u8Framing, DEVICE_ANY, CMD_RESET);
    stSendMsg.len = 3;
    stSendMsg.repeat_times = 2;

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("DiSEqC reset fail.\n");
        return s32Ret;
    }

    /* TODO: If level 2.x, handle received message here. */

    return s32Ret;
#endif
}

mt_s32 mt_unf_fe_diseqc_standby(mt_u32 tuner_id, mt_unf_fe_diseqc_level_t level)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8Framing;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", level);
        return MT_ERR_FE_INVALID_PARA;
    }

    memset(&stSendMsg, 0, sizeof(stSendMsg));
    memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    u8Framing = (level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;
    stSendMsg.level = level;
    stSendMsg.tone_burst = MT_UNF_FE_SWITCH_TONEBURST_NONE;
    FORMAT_DISEQC_CMD(stSendMsg.data, u8Framing, DEVICE_ANY, CMD_STANDBY);
    stSendMsg.len = 3;
    stSendMsg.repeat_times = 2;

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send standby fail.\n");
        return s32Ret;
    }

    /* TODO: If level 2.x, handle received message here. */

    return s32Ret;
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc_wakeup(mt_u32 tuner_id, mt_unf_fe_diseqc_level_t level)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_unf_fe_diseqc_recvmsg_t stRecvMsg;
    mt_s32 s32Ret;
    mt_u8 u8Framing;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", level);
        return MT_ERR_FE_INVALID_PARA;
    }

    memset(&stSendMsg, 0, sizeof(stSendMsg));
    memset(&stRecvMsg, 0, sizeof(stRecvMsg));
    u8Framing = (level == MT_UNF_FE_DISEQC_LEVEL_2_X) ? MASTER_REPLY_FIRST : MASTER_NOREPLY_FIRST;
    stSendMsg.level = level;
    stSendMsg.tone_burst = MT_UNF_FE_SWITCH_TONEBURST_NONE;
    FORMAT_DISEQC_CMD(stSendMsg.data, u8Framing, DEVICE_ANY, CMD_POWER_ON);
    stSendMsg.len = 3;
    stSendMsg.repeat_times = 2;

    s32Ret = fe_diseqc_sendrecv_message(tuner_id, &stSendMsg, &stRecvMsg);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FE("Send power on fail.\n");
        return s32Ret;
    }

    /* TODO: If level 2.x, handle received message here. */

    return s32Ret;
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_diseqc2_srmsg(mt_u32 tuner_id, const mt_unf_fe_diseqc_switch4port_t *p_prm, mt_unf_fe_diseqc2_rsmsg_info *msg) //diseqc2
{
#if 1
    mt_s32 s32Ret;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FE("Input parameter(tuner_id) invalid: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_prm)
    {
        MT_ERR_FE("Input parameter(pstPara) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_prm->level)
    {
        MT_ERR_FE("DiSEqC level invalid: %d\n", p_prm->level);
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_DISEQC_SWITCH_PORT_BUTT <= p_prm->port)
    {
        MT_ERR_FE("Switch port invalid: %d\n", p_prm->port);
        return MT_ERR_FE_INVALID_PARA;
    }

    /* Save port parameter */
    s_stDiSEqCStatus[tuner_id].stPort1_0 = *p_prm;

    /* If NONE, return. */
    if (MT_UNF_FE_DISEQC_SWITCH_NONE == p_prm->port)
    {
        return MT_SUCCESS;
    }

    /* If have 1.0 switch, set if first */
    if (MT_UNF_FE_DISEQC_SWITCH_NONE != s_stDiSEqCStatus[tuner_id].stPort1_0.port)
    {
        s32Ret = DISEQC_SendCmd2_0(tuner_id, &(s_stDiSEqCStatus[tuner_id].stPort1_0), msg);
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_FE("Send cmd 2.0 fail.\n");
            return s32Ret;
        }

        MT_USLEEP(DISEQC_DELAY_BETWEEN_CMD_MS * 1000);
    }

#endif

    return MT_SUCCESS;
}
