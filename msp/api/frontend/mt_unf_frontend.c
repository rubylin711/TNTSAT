/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#if 1
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

//#include <pthread.h>
//#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/genetlink.h>

#include "mt_type.h"
#include "mt_common.h"
#include "mt_mpi_mem.h"
#include "mt_unf_frontend.h"
#include "drv_frontend_ioctl.h"
#include "mt_drv_struct.h"
#include "mt_module_debug.h"

#include "mt_error_mpi.h"

//#include "../../mt_driver/frontend/dm6k/port_dm6k.h"

//#define MT_UNF_FE_DEV_NAME   "fe"
#define MT_UNF_FE_DEV_NAME "mt_tuner"
//#define MT_UNF_FE_DEV_NAME1 "mt_tuner"

#define QAM_RF_MIN 45000  /*kHz*/
#define QAM_RF_MAX 870000 /*kHz*/
#define TER_RF_MIN 48000  //kHz
#define TER_RF_MAX 870000 //kHz
#define TER_BW_MIN 1700   //KHz
#define TER_BW_MAX 10000  //KHz
#define PI 3.14159265
#define UNF_TUNER_NUM 5
#define MT_UNF_FE_MAX_NUM UNF_TUNER_NUM

#define MAX_BLINDSCAN_TIMES (4)

#define SAT_C_MIN (3000)
#define SAT_C_MAX (4200)
#define SAT_KU_MIN (10600)
#define SAT_KU_MAX (12750)
#define SAT_IF_MIN_KHZ (950000)
#define SAT_IF_MAX_KHZ (2150000)
#define SAT_C_MIN_KHZ (3000000)
#define SAT_C_MAX_KHZ (4200000)
#define SAT_KU_MIN_KHZ (10600000)
#define SAT_KU_MAX_KHZ (12750000)

#define SAT_SYMBOLRATE_MAX (60000000) //(45000000)

#define SAT_IF_MIN (950)
#define SAT_IF_MAX (2150)
#define SAT_IF_C_H_MIN (1550)
#define SAT_IF_C_H_MAX (2150)
#define SAT_IF_C_L_MIN (950)
#define SAT_IF_C_L_MAX (1550)
//#define SAT_IF_KU_H_MIN_1 (1100)
//#define SAT_IF_KU_H_MAX_1 (2150)
//#define SAT_IF_KU_H_MIN_2 (950)
//#define SAT_IF_KU_H_MAX_2 (2000)
//#define SAT_IF_KU_L_MIN (950)
//#define SAT_IF_KU_L_MAX (1950)

#define SAT_LO_C_L (5150)
#define SAT_LO_C_H (5750)
//#define SAT_LO_KU_L_1 (9750)
//#define SAT_LO_KU_H_1 (10600)
//#define SAT_LO_KU_L_2 (9750)
//#define SAT_LO_KU_H_2 (10750)

#define SAT_DOWNLINK_FREQ_KU_MID (11700)


#define SAT_DOWNLINK_DSS_A3_MIN (11000)
#define SAT_DOWNLINK_DSS_A3_MAX (12300)
#define SAT_LO_DSS_A3_L         (10500)
#define SAT_LO_DSS_A3_H         (13100)
#define SAT_IF_DSS_A3_L_MIN       (500)    // 11000 - 10500 =  500
#define SAT_IF_DSS_A3_L_MAX      (1800)    // 12300 - 10500 = 1800
#define SAT_IF_DSS_A3_H_MIN       (800)    // 13100 - 12300 =  800
#define SAT_IF_DSS_A3_H_MAX      (2100)    // 13100 - 11000 = 2100



#define DISEQC_DELAY_TIME_MS (50)

#define T2_GROUP_SUM (16)

#define DVBS_BS_NL_NAME "dvbs_bs_nl"
#define DVBS_BS_SEQ 0x11223344

/* cmd0 can match data0 and data1, cmd1 also can match data0 and data1 */
enum
{
    DVBS_BS_NL_OPS_CMD0,
    //DVBS_BS_NL_OPS_CMD1,
    __DVBS_BS_NL_OPS_MAX,
};
#define DVBS_BS_NL_OPS_AMOUNT (__DVBS_BS_NL_OPS_MAX)

enum
{
    DVBS_BS_NL_ATTR_UNSPEC,
    DVBS_BS_NL_ATTR_DATA0,
    //DVBS_BS_NL_ATTR_DATA1,
    __DVBS_BS_NL_ATTR_MAX,
};
#define DVBS_BS_NL_FAMILY_ATTR_MAX (__DVBS_BS_NL_ATTR_MAX - 1)
#define DVBS_BS_NL_FAMILY_ATTR_AMOUNT (__DVBS_BS_NL_ATTR_MAX)

struct dvbs_bs_netlink_data
{
    pid_t pid;
    unsigned int seq;
    unsigned int freq_khz;
    unsigned int symbol_rate;
    unsigned int port_type;
    unsigned char DataTsNumber;
    unsigned char ts_id;
    unsigned char ts_index;
    unsigned char DataTsIdArray[32];
    unsigned int data01;
    unsigned char cmd;
    char name[32];
};

/*********************************************************************/

#if 0
#define GET_REFER_SNR(ref, array, fec_rate)                          \
    {                                                                \
        mt_u32 i = 0;                                                \
        for (i = 0; i < sizeof(array) / sizeof(fe_refer_snr_s); i++) \
        {                                                            \
            if (array[i].en_fec_rate == fec_rate)                    \
            {                                                        \
                ref = array[i].refer_snr;                            \
            }                                                        \
        }                                                            \
    }
#endif

static const mt_char g_fe_version[] __attribute__((used)) = "SDK_VERSION:[" MKMARCOTOSTR(SDK_VERSION) "] Build Time:[" __DATE__ ", " __TIME__ "]";
typedef struct
{
    mt_u32 tuner_id;
    mt_unf_fe_blindscan_para_t scan_info;
} blindscan_para_t;

typedef struct
{
    mt_u32 port;
    mt_unf_fe_ter_scan_para_t s_para;
} terscan_para_t;

typedef struct
{
    mt_unf_fe_lnb_22k_t en_lnb_22k;
    mt_unf_fe_polar_t en_polar;
    mt_u16 start_freq; /* MHz */
    mt_u16 stop_freq;  /* MHz */
} blindscan_condition_t;

typedef struct
{
    mt_u32 scan_times;
    blindscan_condition_t scan_cond[MAX_BLINDSCAN_TIMES];
} blindscan_ctrl_t;

typedef struct
{
    mt_unf_fe_lnb_config_t lnb_config;  /* LNB configuration */
    mt_unf_fe_lnb_power_t en_lnb_power; /* LNB power */
    mt_unf_fe_polar_t en_polar;         /* LNB polarization */
    mt_unf_fe_lnb_22k_t en_lnb_22k;     /* LNB 22K on or off */
    pthread_t *p_bs_monitor;            /* Blind scan thread */
    //pthread_t*               pConnectMonitor; /* connect thread using while the symbol rate is low*/
    MT_BOOL blindscan_stop; /* Blind scan stop flag */
    MT_BOOL blindscan_busy; /* Blind scan working flag */
    //MT_BOOL                         bConnectStop; /* Blind scan stop flag */
    //mt_unf_fe_switch_22k_t       enSavedSwitch22K;
    mt_unf_fe_switch_22k_t en_switch_22k;
    mt_unf_fe_switch_toneburst_t en_toneburst;
} fe_status_sat_t;

typedef struct
{
    mt_u16 signal_level;
    mt_s16 signal_dbuv;
} fe_signal_level_sat_t;

typedef struct
{
    pthread_t *p_bs_monitor; /* Blind scan thread */
    MT_BOOL bs_busy;         /* Blind scan stop flag */
    MT_BOOL bs_stop;         /* Blind scan stop flag */
    mt_unf_fe_ter_antenna_power_t en_antenna_power;
} fe_status_ter_t;

typedef struct _unf_fe_info_t
{
    mt_unf_fe_attr_t attr;
    mt_s32 attr_set_cnt;
    mt_unf_fe_attr_t *p_def_attr;
} mt_unf_fe_info_t;

/*static mt_unf_fe_info_t tuner_info[MT_UNF_FE_MAX_NUM];
static mt_s32 init_cnt = 0;
static mt_s32 g_fe_fd_cnt = 0; Clean Warning [-Wunused-variable]*/
static mt_s32 g_fe_fd = 0; //[UNF_TUNER_NUM] = {0};
//static mt_u32  g_plp_mode = 0;
//frontend_acc_qam_param_t
//static fe_agc_qam_param_t s_stCurrentSignal[UNF_TUNER_NUM]; // 180601
//mt_unf_fe_cab_connect_para_t
//static fe_agc_qam_param_t s_stCurrentLockSignal[UNF_TUNER_NUM];
static MT_BOOL b_fe_init = MT_FALSE;
static MT_BOOL b_fe_open = 0; //[UNF_TUNER_NUM] = {MT_FALSE,};
static mt_unf_fe_attr_t g_default_fe_attr[UNF_TUNER_NUM];
static mt_unf_fe_attr_t g_current_fe_attr[UNF_TUNER_NUM];
static mt_unf_fe_connect_para_t g_current_fe_connect_para[UNF_TUNER_NUM];
static fe_status_sat_t g_sat_para[UNF_TUNER_NUM];
static fe_status_ter_t g_ter_para[UNF_TUNER_NUM]; // 180601

static pthread_mutex_t g_fe_mutex = PTHREAD_MUTEX_INITIALIZER;

#if 0
#define BS_TEST_MAX_TP 256

static MT_U32 locked_tp_num;
static MT_U32 unlocked_tp_num;
static MT_U32 total_tp_num;
mt_unf_fe_channel_info_t win_channel_info[BS_TEST_MAX_TP];
static mt_unf_fe_channel_info_t locked_channel_info[BS_TEST_MAX_TP];
static mt_unf_fe_channel_info_t unlocked_channel_info[BS_TEST_MAX_TP];
#endif

#define MT_FE_LOCK() (void)pthread_mutex_lock(&g_fe_mutex);
#define MT_FE_UNLOCK() (void)pthread_mutex_unlock(&g_fe_mutex);

#define CHECK_TUNER_OPEN()                             \
    do                                                 \
    {                                                  \
        MT_FE_LOCK();                                  \
        if (!b_fe_open)                                \
        {                                              \
            MT_INFO_FRONTEND("frontend not opened\n"); \
            MT_FE_UNLOCK();                            \
            return MT_ERR_FE_NOT_OPEN;                 \
        }                                              \
        MT_FE_UNLOCK();                                \
    } while (0)

#if 0
static mt_unf_fe_attr_t default_fe_attr[MT_UNF_FE_MAX_NUM] =
{
    {
        .sig_type = MT_UNF_FE_SIG_TYPE_CAB,
        .tuner_type = MT_UNF_TUNER_TYPE_M88TC6800,
        .tuner_addr = 0xC6,
        .demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CC6000,
        .demod_addr = 0x38,
        .output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL,
        .demod_i2c_id = 0,
        .tuner_i2c_id[0] = 0,
        .no_need_init = 0
    },

    {
        .sig_type = MT_UNF_FE_SIG_TYPE_CAB,
        .tuner_type = MT_UNF_TUNER_TYPE_M88TC3800,
        .tuner_addr = 0xC2,
        .demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB,
        .demod_addr = 0x38,
        .output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL,
        .demod_i2c_id = 0,
        .tuner_i2c_id[0] = 0,
        .no_need_init = 0
    },
    {
        .sig_type = MT_UNF_FE_SIG_TYPE_SAT,
        .tuner_type = MT_UNF_TUNER_TYPE_M88TS2022,
        .tuner_addr = 0xc0,
        .demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT,
        .demod_addr = 0xd0,
        .output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL,
        .demod_i2c_id = 0,
        .tuner_i2c_id[0] = 0,
        .no_need_init = 0
    },
    {
        .sig_type = MT_UNF_FE_SIG_TYPE_CTTB,
        .tuner_type = MT_UNF_TUNER_TYPE_M88TC3800,
        .tuner_addr = 0xC2,
        .demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88DM6K,
        .demod_addr = 0x18,
        .output_mode = MT_UNF_FE_OUTPUT_MODE_SERIAL,
        .demod_i2c_id = 0,
        .tuner_i2c_id[0] = 0,
        .no_need_init = 0
    },
};
#endif

#if 0
static fe_signal_level_sat_t g_signal_level_av2011[100] =
{
    {0, 18},      {0, 19},      {0, 20},      {1906,21},    {4959, 22},   {6712, 23},   {8254, 24},   {9419, 25},   {10508, 26},  {11450, 27},
    {12360, 28},  {13151, 29},  {13940, 30},  {14744, 31},  {15396, 32},  {16027, 33},  {16529, 34},  {16997, 35},  {17425, 36},  {17900, 37},
    {18248, 38},  {18658, 39},  {19012, 40},  {19324, 41},  {19707, 42},  {19937, 43},  {20246, 44},  {20478, 48},  {20789, 49},  {21018, 50},
    {21380, 51},  {21658, 52},  {21967, 53},  {22338, 54},  {22646, 55},  {23056, 56},  {23400, 57},  {23932, 58},  {24469, 59},  {25119, 60},
    {25844, 61},  {26485, 66},  {27486, 67},  {28610, 68},  {29767, 69},  {30935, 70},  {31449, 74},  {32603, 75},  {33750, 76},  {34700, 77},
    {35526, 78},  {36280, 79},  {37022, 80},  {37677, 81},  {38217, 82},  {38724, 83},  {39135, 84},  {39584, 85},  {40063, 86},  {40486, 87},
    {40863, 88},  {41399, 89},  {41931, 90},  {42498, 91},  {43132, 92},  {43701, 93},  {44282, 94},  {44761, 95},  {45353, 96},  {45795, 97},
    {46258, 98},  {46792, 99},  {47346, 100}, {47771, 101}, {48144, 102}, {48611, 103}, {49030, 104}, {49344, 105}, {49663, 106}, {50020, 107},
    {50195, 108}, {50440, 109}, {50652, 110}, {50911, 111}, {51170, 112}, {51287, 113}, {51480, 114}, {51617, 115}, {51868, 116}, {51858, 117},
    {51890, 118}, {51873, 119}, {51893, 120}, {51906, 121}, {51808, 122}, {51843, 123}, {51857, 124}, {51831, 125}, {51823, 126}, {51823, 127}
};

static fe_signal_level_sat_t g_signal_level_sharp7903[100] =
{
    {5463, 18},   {5962, 19},   {6488, 20},   {7011, 21},   {7611, 22},   {8206, 23},   {8811, 24},   {9482, 25},   {10174, 26},  {16635, 37},
    {11382, 28},  {12050, 29},  {12664, 30},  {13179, 31},  {13860, 32},  {14447, 33},  {14967, 34},  {15490, 35},  {16096, 36},  {17900, 37},
    {17168, 38},  {17645, 39},  {18122, 40},  {18610, 41},  {19053, 42},  {19657, 43},  {20523, 44},  {21279, 45},  {22003, 46},  {22683, 47},
    {23345, 48},  {23943, 49},  {24440, 50},  {24994, 51},  {25409, 52},  {25804, 53},  {26186, 54},  {26469, 55},  {26906, 56},  {27343, 57},
    {27828, 58},  {28254, 59},  {28725, 60},  {29231, 61},  {29715, 62},  {30227, 63},  {30730, 64},  {31233, 65},  {31713, 66},  {32291, 67},
    {32860, 68},  {33391, 69},  {33901, 70},  {34514, 71},  {35076, 72},  {35802, 73},  {37001, 74},  {37667, 75},  {38242, 76},  {38775, 77},
    {39240, 78},  {39642, 79},  {39996, 80},  {40356, 81},  {40684, 82},  {41033, 83},  {41339, 84},  {41568, 85},  {41950, 86},  {42288, 87},
    {42609, 88},  {42983, 89},  {43368, 90},  {43847, 91},  {44335, 92},  {44923, 93},  {45543, 94},  {46188, 95},  {46897, 96},  {47591, 97},
    {48245, 98},  {48902, 99},  {49535, 100}, {50093, 101}, {50611, 102}, {51103, 103}, {51525, 104}, {51963, 105}, {52479, 106}, {52825, 107},
    {53257, 108}, {53601, 109}, {53985, 110}, {54323, 111}, {54642, 112}, {54947, 113}, {55242, 114}, {55477, 116}, {55465, 118}, {55451, 120}
};
#endif

static mt_s32 g_tuner_freq = 0;

#if 0
typedef struct
{
    mt_s32 signal_freq;
    mt_s32 signal_level;
} fe_signal_level_t;

static fe_signal_level_t g_signal_level_tda18250[] =
{
    { 52, 5},  { 60, 5},  { 68, 5},  { 76, 5},  { 84, 5},  { 92, 5},  {100, 5},
    {108, 5},  {115, 5},  {123, 5},  {131, 4},  {139, 5},  {147, 5},  {155, 5},  {163, 5},  {171, 5},
    {179, 5},  {187, 5},  {195, 4},  {203, 4},  {211, 4},  {219, 5},  {227, 4},  {235, 4},  {243, 4},  {251, 4},
    {259, 4},  {267, 4},  {275, 4},  {283, 4},  {291, 4},  {299, 4},  {307, 4},  {315, 4},  {323, 4},  {331, 5},
    {339, 5},  {347, 5},  {355, 4},  {363, 5},  {371, 5},  {379, 5},  {387, 5},  {395, 4},  {411, 5},  {419, 5},
    {427, 5},  {435, 4},  {443, 5},  {451, 5},  {459, 5},  {467, 5},  {474, 5},  {482, 5},  {490, 5},  {498, 5},
    {506, 5},  {514, 4},  {522, 5},  {530, 4},  {538, 5},  {546, 4},  {554, 4},  {562, 4},  {570, 4},  {578, 4},
    {586, 4},  {594, 4},  {602, 4},  {610, 4},  {618, 4},  {626, 4},  {634, 4},  {642, 4},  {650, 5},  {658, 4},
    {666, 4},  {674, 4},  {682, 4},  {690, 5},  {698, 4},  {706, 4},  {714, 4},  {722, 5},  {730, 4},  {738, 4},
    {746, 4},  {754, 5},  {762, 5},  {770, 5},  {778, 5},  {786, 5},  {794, 5},  {802, 5},  {810, 5},  {818, 5},
    {826, 5},  {834, 5},  {842, 5},  {850, 6},  {858, 7}
};
#endif

/*    //tda18250 soft filter open
static fe_signal_level_t g_signal_level_tda18250[] =
{
    {52, 5},    {60, 5},    {68, 5},    {76, 5},   {84, 5},    {92, 5},   {100, 5},
    {108, 3},  {115, 2},  {123, 3},  {131, 3},  {139, 3},  {147, 3},  {155,3},  {163, 3},  {171, 2},
    {179, 2},  {187, 2},  {195, 3},  {203, 3},  {211, 3},  {219, 4},  {227, 4},  {235, 4},  {243, 4},  {251, 4},
    {259, 4},  {267, 5},  {275, 5},  {283, 4},  {291, 5},  {299, 5},  {307, 5},  {315, 5},  {323, 5},  {331, 5},
    {339, 5},  {347, 5},  {355, 4},  {363, 7},  {371, 6},  {379, 6},  {387, 6},  {395, 6},  {411, 6},  {419, 6},
    {427, 6},  {435, 6},  {443, 6},  {451, 6},  {459, 6},  {467, 6},  {474, 5},  {482, 5},  {490, 6},  {498, 5},
    {506, 7},  {514, 7},  {522, 7},  {530, 8},  {538, 8},  {546, 8},  {554, -1},  {562, -1},  {570, -1},  {578, 0},
    {586, -1},  {594, -1},  {602, -1},  {610, -1},  {618, -1},  {626, -1},  {634, -1},  {642, -1},  {650, -1},  {658, -1},
    {666, -1},  {674, -1},  {682, -1},  {690, -1},  {698, -1},  {706, -1},  {714, -1},  {722, -1},  {730, -2},  {738, -3},
    {746, -2},  {754, -3},  {762, -2},  {770, -2},  {778, -2},  {786, -3},  {794, -2},  {802, -2},  {810, -2},  {818, -2},
    {826, -2},  {834, -2},  {842, -1},  {850, -1},  {858, 0}
}; */

static mt_void fe_downlinkfreq_to_if(mt_unf_fe_lnb_config_t *p_lnb_config,
                                     mt_unf_fe_polar_t en_polar, mt_u32 downlinkfreq,
                                     mt_u32 *p_if, mt_unf_fe_lnb_22k_t *p_en_lnb_22k);
static mt_void fe_if_to_downlinkfreq(const mt_unf_fe_lnb_config_t *p_lnb_config,
                                     mt_unf_fe_polar_t en_polar, mt_unf_fe_lnb_22k_t en_lnb_22k,
                                     mt_u32 if_khz, mt_u32 *p_downlinkfreq_khz);
static mt_s32 fe_set_lnbout_and_22k(mt_u32 port, mt_unf_fe_polar_t en_polar, mt_unf_fe_lnb_22k_t en_lnb_22k);
static mt_void *fe_dvbs_blindscan_thread(mt_void *p_blindscan);
//static mt_void* unf_dvbs_blindscan_thread(mt_void *p_blindscan);
static mt_s32 unicable_chan_change(mt_u32 port, mt_u32 freq_mhz, mt_unf_fe_polar_t en_polar);

mt_s32 fe_check_bs_status(mt_u32 tuner_id, mt_u8 *status);
mt_s32 fe_get_blindscan_result(mt_u32 tuner_id, void *para);
mt_void *fe_dvbt2_scan_thread(mt_void *p_ter_scan);

mt_unf_fe_switch_toneburst_t fe_diseqc_get_toneburst_status(mt_u32 tuner_id);
mt_s32 fe_diseqc_sendrecv_message(mt_u32 tuner_id,
                                  const mt_unf_fe_diseqc_sendmsg_t *p_sendmsg,
                                  mt_unf_fe_diseqc_recvmsg_t *p_recvmsg);

mt_s32 unicable_diseqc_sendrecv_message(mt_u32 tuner_id,
                                        const mt_unf_fe_diseqc_sendmsg_t *p_sendmsg,
                                        mt_unf_fe_diseqc_recvmsg_t *p_recvmsg);
mt_s32 unicable_power_off(mt_u32 port, mt_u8 scr_no);
mt_s32 unicable_scrx_signal_on(mt_u32 tuner_id);
mt_s32 unicable_lofrq(mt_u32 tuner_id, mt_u8 scr_no, mt_u8 lofreq_no);

mt_s32 mt_unf_fe_get_current_plp_type(mt_u32 tuner_id, mt_unf_fe_t2_plp_type_t *p_plptype);

mt_void set_blindscan_ctrl_cond(mt_u32 port,
                                blindscan_ctrl_t *p_bs_ctrl,
                                mt_s32 i,
                                mt_unf_fe_lnb_22k_t lnb_22k,
                                mt_unf_fe_polar_t polar,
                                mt_s32 start_freq_mhz,
                                mt_s32 stop_freq_mhz);

mt_void set_blindscan_ctrl_cond(mt_u32 port,
                                blindscan_ctrl_t *p_bs_ctrl,
                                mt_s32 i,
                                mt_unf_fe_lnb_22k_t lnb_22k,
                                mt_unf_fe_polar_t polar,
                                mt_s32 start_freq_mhz,
                                mt_s32 stop_freq_mhz)
{
    if (i >= MAX_BLINDSCAN_TIMES)
    {
        return;
    }

    p_bs_ctrl->scan_cond[i].en_lnb_22k = (lnb_22k);
    p_bs_ctrl->scan_cond[i].en_polar = (polar);

    /* Check whether the LNB is unicable */
    if (MT_UNF_FE_LNB_UNICABLE != g_sat_para[port].lnb_config.lnb_type)
    {
        if (g_sat_para[port].lnb_config.low_lo == SAT_LO_DSS_A3_L)
        {
            if ((start_freq_mhz) < SAT_IF_DSS_A3_L_MIN)
            {
                p_bs_ctrl->scan_cond[i].start_freq = SAT_IF_DSS_A3_L_MIN;
            }
            else
            {
                p_bs_ctrl->scan_cond[i].start_freq = (mt_u16)(start_freq_mhz);
            }
            
            if ((stop_freq_mhz) > SAT_IF_DSS_A3_L_MAX)
            {
                p_bs_ctrl->scan_cond[i].stop_freq = SAT_IF_DSS_A3_L_MAX;
            }
            else
            {
                p_bs_ctrl->scan_cond[i].stop_freq = (mt_u16)(stop_freq_mhz);
            }
            
            if ((p_bs_ctrl->scan_cond[i].stop_freq) < (p_bs_ctrl->scan_cond[i].start_freq))
            {
                p_bs_ctrl->scan_cond[i].stop_freq = p_bs_ctrl->scan_cond[i].start_freq;
            }

#if 0
            printf("%s[%d] ---- LO[%d], start[%d], stop[%d]\n", __FUNCTION__, __LINE__, 
                    g_sat_para[port].lnb_config.low_lo, 
                    p_bs_ctrl->scan_cond[i].start_freq, 
                    p_bs_ctrl->scan_cond[i].stop_freq);
#endif
        }
        else if (g_sat_para[port].lnb_config.high_lo == SAT_LO_DSS_A3_H)
        {
            if ((start_freq_mhz) < SAT_IF_DSS_A3_H_MIN)
            {
                p_bs_ctrl->scan_cond[i].start_freq = SAT_IF_DSS_A3_H_MIN;
            }
            else
            {
                p_bs_ctrl->scan_cond[i].start_freq = (mt_u16)(start_freq_mhz);
            }
            
            if ((stop_freq_mhz) > SAT_IF_DSS_A3_H_MAX)
            {
                p_bs_ctrl->scan_cond[i].stop_freq = SAT_IF_DSS_A3_H_MAX;
            }
            else
            {
                p_bs_ctrl->scan_cond[i].stop_freq = (mt_u16)(stop_freq_mhz);
            }
            
            if ((p_bs_ctrl->scan_cond[i].stop_freq) < (p_bs_ctrl->scan_cond[i].start_freq))
            {
                p_bs_ctrl->scan_cond[i].stop_freq = p_bs_ctrl->scan_cond[i].start_freq;
            }

#if 0
            printf("%s[%d] ---- LO[%d], start[%d], stop[%d]\n", __FUNCTION__, __LINE__, 
                    g_sat_para[port].lnb_config.high_lo, 
                    p_bs_ctrl->scan_cond[i].start_freq, 
                    p_bs_ctrl->scan_cond[i].stop_freq);
#endif
        }
        else
        {
            if ((start_freq_mhz) < SAT_IF_MIN)
            {
                p_bs_ctrl->scan_cond[i].start_freq = SAT_IF_MIN;
            }
            else
            {
                p_bs_ctrl->scan_cond[i].start_freq = (mt_u16)(start_freq_mhz);
            }

            if ((stop_freq_mhz) > SAT_IF_MAX)
            {
                p_bs_ctrl->scan_cond[i].stop_freq = SAT_IF_MAX;
            }
            else
            {
                p_bs_ctrl->scan_cond[i].stop_freq = (mt_u16)(stop_freq_mhz);
            }

            if ((p_bs_ctrl->scan_cond[i].stop_freq) < (p_bs_ctrl->scan_cond[i].start_freq))
            {
                p_bs_ctrl->scan_cond[i].stop_freq = p_bs_ctrl->scan_cond[i].start_freq;
            }
        }
    }
    else
    {
        if ((start_freq_mhz) < SAT_KU_MIN)
        {
            p_bs_ctrl->scan_cond[i].start_freq = SAT_KU_MIN;
        }
        else
        {
            p_bs_ctrl->scan_cond[i].start_freq = (mt_u16)(start_freq_mhz);
        }

        if ((stop_freq_mhz) > SAT_KU_MAX)
        {
            p_bs_ctrl->scan_cond[i].stop_freq = SAT_KU_MAX;
        }
        else
        {
            p_bs_ctrl->scan_cond[i].stop_freq = (mt_u16)(stop_freq_mhz);
        }

        if ((p_bs_ctrl->scan_cond[i].stop_freq) < (p_bs_ctrl->scan_cond[i].start_freq))
        {
            p_bs_ctrl->scan_cond[i].stop_freq = p_bs_ctrl->scan_cond[i].start_freq;
        }
    }
}

/* Convert downlink frequency to IF, calculate LNB 22K status synchronously, for connect */
/*Downlink freq dominates the course, i.e, the freq band is decided by the downlink freq*/
static mt_void fe_downlinkfreq_to_if(mt_unf_fe_lnb_config_t *p_lnb_config,
                                     mt_unf_fe_polar_t en_polar, mt_u32 downlinkfreq,
                                     mt_u32 *p_if, mt_unf_fe_lnb_22k_t *p_en_lnb_22k)
{
    /* Default */
    if (MT_NULL != p_en_lnb_22k)
    {
        *p_en_lnb_22k = MT_UNF_FE_LNB_22K_OFF;
    }

    /*fix issue 133586 no need to modify the lnb_band there*/
    /*if ((SAT_C_MIN_KHZ <= downlinkfreq) && (SAT_C_MAX_KHZ >= downlinkfreq))
    {
        p_lnb_config->lnb_band = MT_UNF_FE_LNB_BAND_C;
    }
    else if ((SAT_KU_MIN_KHZ <= downlinkfreq) && (SAT_KU_MAX_KHZ >= downlinkfreq))
    {
        p_lnb_config->lnb_band = MT_UNF_FE_LNB_BAND_KU;
    }
    else
    {
        MT_ERR_FRONTEND("Error freq!\n");
        return;
    }*/

    switch (p_lnb_config->lnb_band)
    {
    /* C band, IF = LO - downlink frequency */
    case MT_UNF_FE_LNB_BAND_C:
        /* Single LO */
        if ((MT_UNF_FE_LNB_SINGLE_FREQUENCY == p_lnb_config->lnb_type) || (p_lnb_config->high_lo == p_lnb_config->low_lo))
        {
            *p_if = p_lnb_config->low_lo * 1000 - downlinkfreq;
        }
        /* Dual LO */
        else
        {
            /* V/R polarization, use high LO */
            if ((MT_UNF_FE_POLARIZATION_V == en_polar) || (MT_UNF_FE_POLARIZATION_R == en_polar))
            {
                *p_if = p_lnb_config->high_lo * 1000 - downlinkfreq;
            }
            /* H/L polarization, use low LO */
            else
            {
                *p_if = p_lnb_config->low_lo * 1000 - downlinkfreq;
            }
        }

        break;

    /* Ku band, IF = downlink frequency - LO */
    case MT_UNF_FE_LNB_BAND_KU:
        //printf("unf line[%d].\r\n", __LINE__);
        /* Single LO */
        if ((MT_UNF_FE_LNB_SINGLE_FREQUENCY == p_lnb_config->lnb_type) || (p_lnb_config->high_lo == p_lnb_config->low_lo))
        {
            if (SAT_LO_DSS_A3_L == p_lnb_config->low_lo)
            {
                *p_if = downlinkfreq - p_lnb_config->low_lo * 1000;
            }
            else if (SAT_LO_DSS_A3_H == p_lnb_config->high_lo)
            {
                *p_if = p_lnb_config->high_lo * 1000 - downlinkfreq;
            }
            else
            {
                *p_if = downlinkfreq - p_lnb_config->low_lo * 1000;
            }
        }
        /* Dual LO */
        else // if (MT_UNF_FE_LNB_DUAL_FREQUENCY == p_lnb_config->lnb_type)
        {
            //printf("unf line[%d] downlinkfreq[%d]\r\n", __LINE__, downlinkfreq);
            /* downlink frequency >= 11700MHz, use high LO */
            if ((downlinkfreq >= SAT_DOWNLINK_FREQ_KU_MID * 1000))
            {
                *p_if = downlinkfreq - p_lnb_config->high_lo * 1000;

                /* Ku dual LO LNB use 22K select high LO */
                if (MT_NULL != p_en_lnb_22k)
                {
                    *p_en_lnb_22k = MT_UNF_FE_LNB_22K_ON;
                }
                //printf("unf line[%d] *p_if[%d]\r\n", __LINE__, *p_if);
            }
            /* downlink frequency < 11700MHz, use low LO */
            else
            {
                *p_if = downlinkfreq - p_lnb_config->low_lo * 1000;
            }
        }
#if 0
            /*unicable LNB*/
            else if (MT_UNF_FE_LNB_UNICABLE == p_lnb_config->lnb_type)
            {
                *p_if = p_lnb_config->unicable_if_freq_mhz * 1000;
            }
#endif
        break;

    default:
        break;
    }
}

/* Convert IF to downlink frequency, the units of IF and Dlink freq are both kHz */
static mt_void fe_if_to_downlinkfreq(const mt_unf_fe_lnb_config_t *p_lnb_config,
                                     mt_unf_fe_polar_t en_polar, mt_unf_fe_lnb_22k_t en_lnb_22k,
                                     mt_u32 if_khz, mt_u32 *p_downlinkfreq_khz)
{
    switch (p_lnb_config->lnb_band)
    {
    /* C band, downlink frequency = LO - IF */
    case MT_UNF_FE_LNB_BAND_C:
        /* Single LO */
        if ((MT_UNF_FE_LNB_SINGLE_FREQUENCY == p_lnb_config->lnb_type) || (p_lnb_config->high_lo == p_lnb_config->low_lo))
        {
            *p_downlinkfreq_khz = p_lnb_config->low_lo * 1000 - if_khz;
        }
        /* Dual LO */
        else
        {
            /* V/R polarization, use high LO */
            if ((MT_UNF_FE_POLARIZATION_V == en_polar) || (MT_UNF_FE_POLARIZATION_R == en_polar))
            {
                *p_downlinkfreq_khz = p_lnb_config->high_lo * 1000 - if_khz;
            }
            /* H/L polarization, use low LO */
            else
            {
                *p_downlinkfreq_khz = p_lnb_config->low_lo * 1000 - if_khz;
            }
        }

        break;

    /* Ku band, downlink frequency = IF + LO */
    case MT_UNF_FE_LNB_BAND_KU:

        /* Single LO */
        if ((MT_UNF_FE_LNB_SINGLE_FREQUENCY == p_lnb_config->lnb_type) || (p_lnb_config->high_lo == p_lnb_config->low_lo))
        {
            if (SAT_LO_DSS_A3_L == p_lnb_config->low_lo)
            {
                *p_downlinkfreq_khz = p_lnb_config->low_lo * 1000 + if_khz;
            }
            else if (SAT_LO_DSS_A3_H == p_lnb_config->high_lo)
            {
                *p_downlinkfreq_khz = p_lnb_config->high_lo * 1000 - if_khz;
            }
            else
            {
                *p_downlinkfreq_khz = p_lnb_config->low_lo * 1000 + if_khz;
            }
        }
        /* Dual LO */
        else
        {
            /* 22K on, use high LO */
            if (MT_UNF_FE_LNB_22K_ON == en_lnb_22k)
            {
                *p_downlinkfreq_khz = p_lnb_config->high_lo * 1000 + if_khz;
            }
            /* 22K off, use low LO */
            else
            {
                *p_downlinkfreq_khz = p_lnb_config->low_lo * 1000 + if_khz;
            }
        }

        break;

    default:
        return;
    }
}

static mt_s32 fe_set_lnbout_and_22k(mt_u32 tuner_id, mt_unf_fe_polar_t en_polar, mt_unf_fe_lnb_22k_t en_lnb_22k)
{
    fe_lnb_out_t lnb_out = {0};
    fe_set_22k_onoff_t fe_onoff_22k = {0};
    mt_s32 ret = 0;
#if 0
    if (MT_UNF_FE_LNB_UNICABLE != g_sat_para[tuner_id].lnb_config.lnb_type)
    {
        switch (g_sat_para[tuner_id].en_lnb_power)
        {
        /* 0V */
        case MT_UNF_FE_LNB_POWER_OFF:
            lnb_out.voltage = TUNER_LNB_OUT_0V;
            break;

        /* 13V/18V */
        case MT_UNF_FE_LNB_POWER_ON:
            if ((MT_UNF_FE_POLARIZATION_V == en_polar) || (MT_UNF_FE_POLARIZATION_R == en_polar))
            {
                lnb_out.voltage = TUNER_LNB_OUT_13V;
            }
            else
            {
                lnb_out.voltage = TUNER_LNB_OUT_18V;
            }

            break;

        /* 14V/19V */
        case MT_UNF_FE_LNB_POWER_ENHANCED:
#if 0
                if ((MT_UNF_FE_POLARIZATION_V == en_polar) || (MT_UNF_FE_POLARIZATION_R == en_polar))
                {
                    lnb_out.voltage = TUNER_LNB_OUT_14V;
                }
                else
                {
                    lnb_out.voltage = TUNER_LNB_OUT_19V;
                }
#endif
            //break;
            return MT_ERR_FE_INVALID_PARA;

        default:
            return MT_ERR_FE_INVALID_PARA;
        }
    }
    else
    {
        switch (g_sat_para[tuner_id].en_lnb_power)
        {
        /* 0V */
        case MT_UNF_FE_LNB_POWER_OFF:
            lnb_out.voltage = TUNER_LNB_OUT_0V;
            break;

        /* 18V */
        case MT_UNF_FE_LNB_POWER_ON:
            lnb_out.voltage = TUNER_LNB_OUT_18V;
            break;

        /* 19V */
        case MT_UNF_FE_LNB_POWER_ENHANCED:
            //lnb_out.voltage = TUNER_LNB_OUT_19V;
            //lnb_out.voltage = TUNER_LNB_OUT_18V;
            //break;
            return MT_ERR_FE_INVALID_PARA;

        default:
            return MT_ERR_FE_INVALID_PARA;
        }
    }
#endif

    //printf("fe_set_lnbout_and_22k line[%d].\n", __LINE__);
    if ((MT_UNF_FE_POLARIZATION_V == en_polar) || (MT_UNF_FE_POLARIZATION_R == en_polar))
    {
        lnb_out.voltage = TUNER_LNB_OUT_13V;
    }
    else
    {
        lnb_out.voltage = TUNER_LNB_OUT_18V;
    }

    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        lnb_out.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        lnb_out.port = tuner_id;
    }

    ret = ioctl(g_fe_fd, FE_SET_LNBOUT_CMD, &lnb_out);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set LNB out fail.\n");
        return MT_ERR_FE_FAILED_LNBCTRL;
    }

    //printf("fe_set_lnbout_and_22k line[%d].\n", __LINE__);

    /* Save polarization status */
    g_sat_para[tuner_id].en_polar = en_polar;

    /* 22K signal control. If LNB power off, can't send 22K signal. */
    if ((MT_UNF_FE_LNB_POWER_OFF != g_sat_para[tuner_id].en_lnb_power) &&
        (MT_UNF_FE_LNB_UNICABLE != g_sat_para[tuner_id].lnb_config.lnb_type))
    {
        /* If has 22K switch, 22K controlled by switch.
               If hasn't 22K switch, 22K controlled by tuner lock or blind scan.
             */
        //if (MT_UNF_FE_SWITCH_22K_NONE == g_sat_para[tuner_id].en_switch_22k)//don't check, set 22k according to app
        {
            if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
            {
                fe_onoff_22k.tuner_id = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
            }
            else
            {
                fe_onoff_22k.tuner_id = tuner_id;
            }

            fe_onoff_22k.onoff = en_lnb_22k;
            ret = ioctl(g_fe_fd, FE_SEND_CONTINUOUS_22K_CMD, &fe_onoff_22k);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("Set continuous 22K fail.\n");
                return MT_ERR_FE_FAILED_LNBCTRL;
            }
        }
    }

    /* Save polarization status */
    g_sat_para[tuner_id].en_lnb_22k = en_lnb_22k;

    return MT_SUCCESS;
}

mt_s32 fe_check_bs_status(mt_u32 tuner_id, mt_u8 *status)
{
    int ret = 0;
    fe_data_t data_t = {0};

    data_t.port = tuner_id;
    ret = ioctl(g_fe_fd, FE_CHECK_BLIND_SCAN_STATUS_CMD, &data_t);
    if (ret < 0)
        return MT_FAILURE; //MT_UNF_TUNER_ERR_CHECK_BLIND_SCAN_STATUS;

    *status = (mt_u8)data_t.data;

    return MT_SUCCESS;
}

mt_s32 fe_get_blindscan_result(mt_u32 tuner_id, void *para)
{
#if 1
    fe_blindscan_info_t bs_result = {0};
    mt_unf_fe_channel_info_t *p_chanel_info = NULL;
    int ret = 0;

    //printf("fe_get_blindscan_result g_fe_fd=%d para=%p.\n", g_fe_fd, para);

    bs_result.tuner_id = tuner_id;
    bs_result.scan_info = (fe_blindscan_param_t *)para;
    p_chanel_info = (mt_unf_fe_channel_info_t *)bs_result.scan_info->p_channel_info;

    ret = ioctl(g_fe_fd, FE_GET_BLIND_SCAN_RESULT_CMD, &bs_result);
    if (ret < 0)
    {
        MT_ERR_FRONTEND("fe_get_blindscan_result ioctrl failed.\n");
        return MT_UNF_TUNER_ERR_GET_BLIND_SCAN_NOTIFY;
    }

    //printf("@@@@fe_get_blindscan_result line:%d.\n",__LINE__);
    bs_result.scan_info = (fe_blindscan_param_t *)para;
    bs_result.scan_info->p_channel_info = p_chanel_info;
    //printf("@@@@bs_result.scan_info->channel_num_total=%u\n",
    //bs_result.scan_info->channel_num_total);

#endif

    return MT_SUCCESS;
}

#if 0
/*The freq used is downlink freq while using unicable LNB, else is IF freq.*/
static mt_void* fe_dvbs_blindscan_thread(mt_void* p_blindscan)
{
    mt_unf_fe_blindscan_para_t bs_para;
//    fe_blindscan_t blindscan;
    fe_blindscan_info_t bs_info;
    fe_blindscan_param_t stActionPara;
    blindscan_ctrl_t blindscan_ctrl = {0};
    mt_unf_fe_sat_tpinfo_t stResult;
    //mt_unf_fe_sat_tpinfo_t sat_tp[BUF_TP_NUM];
    mt_unf_fe_blindscan_notify_t unNotify;
    //TUNER_DATA_S stTunerData;
    //TUNER_LNB_OUT_S stLNBOut;
    mt_u32 all_freq  = 0;
    mt_u32 tuner_id;
    mt_s32 ret = MT_FAILURE;
    mt_s32 i;//, j, k;
    mt_s32 if_start = 0;
    mt_s32 if_stop = 0;
    mt_u32 dlink_start = 0;
    mt_u32 dlink_stop = 0;
    mt_u16 last_progress_percent = 0;
    mt_u16 progress_percent = 0;
    mt_u16 last_num = 0;
    mt_unf_fe_blindscan_status_t enStatus = MT_UNF_FE_BLINDSCAN_STATUS_IDLE;
    //mt_unf_fe_sat_tpinfo_t *pstVerifyStart = MT_NULL;
    mt_u8 status = 0;
//    mt_u32 tp_index = 0;
    mt_u8 /*may_no_sig = 0, may_sfu = 0,*/ stop_quit = 0;

    //printf("\n---thread--line:%d-------g_fe_fd=%d-------\n", __LINE__, g_fe_fd);

    if (MT_NULL == p_blindscan)
    {
        MT_ERR_FRONTEND("Input parameter(p_blindscan) invalid\n");
        return MT_NULL;
    }

    tuner_id = ((blindscan_para_t *)p_blindscan)->tuner_id;
    bs_para = ((blindscan_para_t *)p_blindscan)->scan_info;

    if (MT_NULL == (bs_para.scan_para.sat.scan_notify))
    {
        MT_ERR_FRONTEND("Input parameter(scan_notify) invalid\n");
        /*if (g_sat_para[tuner_id].pBlindScanMonitor)
        {
            free(g_sat_para[tuner_id].pBlindScanMonitor);
            g_sat_para[tuner_id].pBlindScanMonitor = HI_NULL;
        }*/

        g_sat_para[tuner_id].blindscan_stop = MT_FALSE;
        return MT_NULL;
    }

    g_sat_para[tuner_id].blindscan_busy = MT_TRUE;

    memset(&stActionPara, 0, sizeof(stActionPara));
    bs_info.tuner_id = tuner_id;
    bs_info.scan_info = &stActionPara;

    /* Manual scan mode */
    if (MT_UNF_FE_BLINDSCAN_MODE_MANUAL == bs_para.mode)
    {
        /*unicable, convert IF to downlink freq first */
        if (MT_UNF_FE_LNB_UNICABLE == g_sat_para[tuner_id].lnb_config.lnb_type)
        {
            /*use 22k select low lo or high lo*/
            fe_if_to_downlinkfreq(&(g_sat_para[tuner_id].lnb_config), bs_para.scan_para.sat.polar, bs_para.scan_para.sat.lnb_22k,
                                   bs_para.scan_para.sat.start_freq, &dlink_start);
            fe_if_to_downlinkfreq(&(g_sat_para[tuner_id].lnb_config), bs_para.scan_para.sat.polar, bs_para.scan_para.sat.lnb_22k,
                                   bs_para.scan_para.sat.stop_freq, &dlink_stop);

            blindscan_ctrl.scan_times = 1;
            //printf("unf thread line[d]: dlink_start[%d] dlink_stop[%d] lnb_22k[%d].\n", dlink_start, dlink_stop, bs_para.scan_para.sat.lnb_22k);
            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, bs_para.scan_para.sat.lnb_22k, bs_para.scan_para.sat.polar,
                            (mt_s32)(dlink_start/1000), (mt_s32)(dlink_stop/1000));
        }
        else /*not unicable */
        {
            blindscan_ctrl.scan_times = 1;
            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, bs_para.scan_para.sat.lnb_22k, bs_para.scan_para.sat.polar,
                            (mt_s32)(bs_para.scan_para.sat.start_freq/1000), (mt_s32)(bs_para.scan_para.sat.stop_freq/1000));
        }
    }
    /* Auto scan mode */
    else
    {
        if (MT_UNF_FE_LNB_UNICABLE != g_sat_para[tuner_id].lnb_config.lnb_type)
        {
            /* Single LO */
            if ((MT_UNF_FE_LNB_SINGLE_FREQUENCY == g_sat_para[tuner_id].lnb_config.lnb_type)
                 || (g_sat_para[tuner_id].lnb_config.low_lo == g_sat_para[tuner_id].lnb_config.high_lo))
            {
                /* C band */
                if (MT_UNF_FE_LNB_BAND_C == g_sat_para[tuner_id].lnb_config.lnb_band)
                {
                    if_start = (mt_s32)(g_sat_para[tuner_id].lnb_config.low_lo - SAT_C_MAX);
                    if_stop = (mt_s32)(g_sat_para[tuner_id].lnb_config.low_lo - SAT_C_MIN);
                    blindscan_ctrl.scan_times = 2;
                    set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, /*MT_FALSE*/MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                    set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, /*MT_FALSE*/MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                }
                /* Ku band */
                else
                {
                    if_start = (mt_s32)(SAT_KU_MIN - g_sat_para[tuner_id].lnb_config.low_lo);
                    if_stop = (mt_s32)(SAT_KU_MAX - g_sat_para[tuner_id].lnb_config.low_lo);
                    blindscan_ctrl.scan_times = 2;
                    set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, /*MT_FALSE*/MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                    set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, /*MT_FALSE*/MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                }
            }
            /* Dual LO */
            else
            {
                /* C band */
                if (MT_UNF_FE_LNB_BAND_C == g_sat_para[tuner_id].lnb_config.lnb_band)
                {
                    if ((g_sat_para[tuner_id].lnb_config.low_lo == SAT_LO_C_L)
                                     && (g_sat_para[tuner_id].lnb_config.high_lo == SAT_LO_C_H))
                    {
                        /* 13V/18V select High/Low LO here */
                        blindscan_ctrl.scan_times = 2;
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, /*MT_FALSE*/MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, SAT_IF_C_L_MIN, SAT_IF_C_L_MAX);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, /*MT_FALSE*/MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, SAT_IF_C_H_MIN, SAT_IF_C_H_MAX);
                    }
                    /* Special C band LNB */
                    else
                    {
                        blindscan_ctrl.scan_times = 2;
                        if_start = (mt_s32)(g_sat_para[tuner_id].lnb_config.low_lo - SAT_C_MAX);
                        if_stop = (mt_s32)(g_sat_para[tuner_id].lnb_config.low_lo - SAT_C_MIN);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, /*MT_FALSE*/MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                        if_start = (mt_s32)(g_sat_para[tuner_id].lnb_config.high_lo - SAT_C_MAX);
                        if_stop = (mt_s32)(g_sat_para[tuner_id].lnb_config.high_lo - SAT_C_MIN);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, /*MT_FALSE*/MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                    }
                }
                /* Ku band */
                else
                {
                    /*no 22k switch*/
                    if (MT_UNF_FE_SWITCH_22K_NONE == g_sat_para[tuner_id].en_switch_22k)
                    {
                        blindscan_ctrl.scan_times = 4;
                        if_start = (mt_s32)(/*SAT_KU_MIN*/SAT_DOWNLINK_FREQ_KU_MID - g_sat_para[tuner_id].lnb_config.high_lo);
                        if_stop = (mt_s32)(SAT_KU_MAX - g_sat_para[tuner_id].lnb_config.high_lo);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_ON, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 2, MT_UNF_FE_LNB_22K_ON, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                        if_start = (mt_s32)(SAT_KU_MIN - g_sat_para[tuner_id].lnb_config.low_lo);
                        if_stop = (mt_s32)(/*SAT_KU_MAX*/SAT_DOWNLINK_FREQ_KU_MID - g_sat_para[tuner_id].lnb_config.low_lo);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 3, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                    }
                    /*22k switch 0Hz port*/
                    else if (MT_UNF_FE_SWITCH_22K_0 == g_sat_para[tuner_id].en_switch_22k)
                    {
                        blindscan_ctrl.scan_times = 2;
                        if_start = (mt_s32)(SAT_KU_MIN - g_sat_para[tuner_id].lnb_config.low_lo);
                        if_stop = (mt_s32)(/*SAT_KU_MAX*/SAT_DOWNLINK_FREQ_KU_MID - g_sat_para[tuner_id].lnb_config.low_lo);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                    }
                    /*22k switch 22kHz port*/
                    else if (MT_UNF_FE_SWITCH_22K_22 == g_sat_para[tuner_id].en_switch_22k)
                    {
                        blindscan_ctrl.scan_times = 2;
                        if_start = (mt_s32)(/*SAT_KU_MIN*/SAT_DOWNLINK_FREQ_KU_MID - g_sat_para[tuner_id].lnb_config.high_lo);
                        if_stop = (mt_s32)(SAT_KU_MAX - g_sat_para[tuner_id].lnb_config.high_lo);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_ON, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, MT_UNF_FE_LNB_22K_ON, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                    }
                }
            }
        }
        else /*unicable LNB */
        {
            /*Single LO, bs 2 times, start downlink freq, stop downlink freq */
            if (g_sat_para[tuner_id].lnb_config.low_lo == g_sat_para[tuner_id].lnb_config.high_lo)
            {
                blindscan_ctrl.scan_times = 2;
                dlink_start = (SAT_IF_MIN + g_sat_para[tuner_id].lnb_config.low_lo);
                dlink_stop = (SAT_IF_MAX + g_sat_para[tuner_id].lnb_config.low_lo);
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, (mt_s32)dlink_start, (mt_s32)dlink_stop);
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, (mt_s32)dlink_start, (mt_s32)dlink_stop);
            }
            /*Dual LO, bs 4 times, start downlink freq, stop downlink freq */
            else
            {
                blindscan_ctrl.scan_times = 4;
                dlink_start = SAT_DOWNLINK_FREQ_KU_MID;
                dlink_stop = (SAT_IF_MAX + g_sat_para[tuner_id].lnb_config.high_lo);
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, (mt_s32)dlink_start, (mt_s32)dlink_stop);
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 2, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, (mt_s32)dlink_start, (mt_s32)dlink_stop);
                dlink_start = (SAT_IF_MIN + g_sat_para[tuner_id].lnb_config.low_lo);
                dlink_stop = SAT_DOWNLINK_FREQ_KU_MID;
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, (mt_s32)dlink_start, (mt_s32)dlink_stop);
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 3, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, (mt_s32)dlink_start, (mt_s32)dlink_stop);
            }
        }
    }

    //printf("usr: dvbs thread line[%d]\n", __LINE__);

    for (i = 0; i < (mt_s32)(blindscan_ctrl.scan_times); i++)
    {
        /* If register diseqc_set, set diseqc */
        if (bs_para.scan_para.sat.diseqc_set)
        {
            bs_para.scan_para.sat.diseqc_set(tuner_id,
                                                 blindscan_ctrl.scan_cond[i].en_polar,
                                                 blindscan_ctrl.scan_cond[i].en_lnb_22k);
        }

        ret = fe_set_lnbout_and_22k(tuner_id, blindscan_ctrl.scan_cond[i].en_polar, blindscan_ctrl.scan_cond[i].en_lnb_22k);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("fe_set_lnbout_and_22k fail.\n");
        }

        usleep(100 * 1000);
        //printf("usr dvbs_thread line[%d] lnb_type[%d] en_polar[%d] en_lnb_22k[%d]\n", __LINE__,
            //g_sat_para[tuner_id].lnb_config.lnb_type,
            //blindscan_ctrl.scan_cond[i].en_polar,
            //blindscan_ctrl.scan_cond[i].en_lnb_22k);

        /*unicable should send ODU_ChannelChange cmd first, then calculate the if_freq.*/
        if (MT_UNF_FE_LNB_UNICABLE == g_sat_para[tuner_id].lnb_config.lnb_type)
        {
            //printf("-------unf line:%d start_freq = %d, stop_freq = %d--------\n",
                //__LINE__, blindscan_ctrl.scan_cond[i].start_freq, blindscan_ctrl.scan_cond[i].stop_freq);
            (mt_void)unicable_chan_change(tuner_id, blindscan_ctrl.scan_cond[i].stop_freq, blindscan_ctrl.scan_cond[i].en_polar);
            //bs_info.scan_info->start_freq = 950 * 1000;
            //bs_info.scan_info->stop_freq = 2150 * 1000;
            bs_info.scan_info->start_freq = bs_para.scan_para.sat.start_freq;
            bs_info.scan_info->stop_freq = bs_para.scan_para.sat.stop_freq;
            //printf("unf line[%d] start_freq[%d] stop_freq[%d]\r\n", __LINE__, bs_info.scan_info->start_freq, bs_info.scan_info->stop_freq);
            if (MT_UNF_FE_SATPOSN_A == g_sat_para[tuner_id].lnb_config.unicable_port_no)
            {
                if (MT_UNF_FE_POLARIZATION_V == blindscan_ctrl.scan_cond[i].en_polar)
                {
                    g_sat_para[tuner_id].lnb_config.unicable_bank = 1;
                }
                else
                {
                    g_sat_para[tuner_id].lnb_config.unicable_bank = 3;
                }
            }
            else
            {
                if (MT_UNF_FE_POLARIZATION_V == blindscan_ctrl.scan_cond[i].en_polar)
                {
                    g_sat_para[tuner_id].lnb_config.unicable_bank = 5;
                }
                else
                {
                    g_sat_para[tuner_id].lnb_config.unicable_bank = 7;
                }
            }

            bs_info.scan_info->uc_param.use_uc = 1;
            bs_info.scan_info->uc_param.bank = g_sat_para[tuner_id].lnb_config.unicable_bank;
            bs_info.scan_info->uc_param.user_band = g_sat_para[tuner_id].lnb_config.unicable_scr_no;
            bs_info.scan_info->uc_param.ub_freq_mhz = g_sat_para[tuner_id].lnb_config.unicable_if_freq_mhz;
            //printf("-------unf line:%d start_freq = %d, stop_freq = %d--------\n",
            //__LINE__, blindscan_ctrl.scan_cond[i].start_freq, blindscan_ctrl.scan_cond[i].stop_freq);
        }
        else
        {
            bs_info.scan_info->start_freq = blindscan_ctrl.scan_cond[i].start_freq * 1000;
            bs_info.scan_info->stop_freq = blindscan_ctrl.scan_cond[i].stop_freq * 1000;
            //bs_info.scan_info->start_freq = 950 * 1000;
            //bs_info.scan_info->stop_freq = 2150 * 1000;
        }

        //printf("usr dvbs thread i=%d--start_freq %d--stop_freq=%d\n", i,
                //blindscan_ctrl.scan_cond[i].start_freq,
                //blindscan_ctrl.scan_cond[i].stop_freq);
        ret = ioctl(g_fe_fd, FE_BLINDSCAN_ACTION_CMD, &bs_info);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("FE_BLINDSCAN_ACTION_CMD error %d\n", ret);
            enStatus = MT_UNF_FE_BLINDSCAN_STATUS_FAIL;
        }
        else
        {
            enStatus = MT_UNF_FE_BLINDSCAN_STATUS_SCANNING;
        }

        //printf("usr dvbs thread line:%d enStatus=%d-----\n", __LINE__, enStatus);
        bs_info.scan_info->p_channel_info = (mt_unf_fe_channel_info_t *)mt_malloc(MT_ID_FRONTEND, sizeof(mt_unf_fe_channel_info_t) * 128);

        /* New TP callback */
        while (1)//bs_info.scan_info->channel_num_cur > last_num)
        {
            /* Finish status */
            if (0)//(u32StepNum - 1 == (mt_u32)j) && ((mt_u32)i == blindscan_ctrl.scan_times - 1))
            {
                enStatus = MT_UNF_FE_BLINDSCAN_STATUS_FINISH;
            }

            /* Quit status */
            if (g_sat_para[tuner_id].blindscan_stop)
            {
                enStatus = MT_UNF_FE_BLINDSCAN_STATUS_QUIT;
            }

            /* If over, break */
            if (enStatus > MT_UNF_FE_BLINDSCAN_STATUS_SCANNING)
            {
                //printf("usr dvbs thread line:%d\n", __LINE__);
                stop_quit = 1;
                break;
            }

            fe_check_bs_status(tuner_id, &status);
            if (status != 0)
            {
                usleep(50);
                ret = fe_get_blindscan_result(tuner_id, (void *)bs_info.scan_info);
                //MT_ERR_FRONTEND("================================================\n");
                //printf("Scaned channel num %u\n", bs_info.scan_info->channel_num_total);
                //MT_ERR_FRONTEND("Now check lock one by one:\n\n");
                if (MT_SUCCESS != ret)
                {
                    MT_ERR_FRONTEND("fe_get_blindscan_result error %d\n", ret);
                    enStatus = MT_UNF_FE_BLINDSCAN_STATUS_FAIL;
                }
                else
                {
                    enStatus = MT_UNF_FE_BLINDSCAN_STATUS_SCANNING;
                }

                for (last_num = 0; last_num < bs_info.scan_info->channel_num_total; last_num++)
                {
                    if (bs_info.scan_info->p_channel_info[last_num].symbol_rate < 1500)
                    {
                        continue;
                    }

                    /* Convert IF(950-2150) to downlink frequency (C:3400-4200, Ku: 10600-12750) */
                    /* unicable do not need. The freq offset of downlink is the same as the if freq.
                    so we can substract the offset to the downlink freq directly. */
#if 0
                    printf("lnb_band[%d] en_polar[%d] en_lnb_22k[%d] freq[%d]\n",
                    g_sat_para[tuner_id].lnb_config.lnb_band,
                    blindscan_ctrl.scan_cond[i].en_polar,
                    blindscan_ctrl.scan_cond[i].en_lnb_22k,
                    bs_info.scan_info->p_channel_info[last_num].frequency);
#endif
                    fe_if_to_downlinkfreq(&(g_sat_para[tuner_id].lnb_config),
                        blindscan_ctrl.scan_cond[i].en_polar,
                        blindscan_ctrl.scan_cond[i].en_lnb_22k,
                        bs_info.scan_info->p_channel_info[last_num].frequency,
                        &(stResult.freq));
                    /* Symbol rate */
                    stResult.symbol_rate = bs_info.scan_info->p_channel_info[last_num].symbol_rate;
                    /* Polarization */
                    stResult.polar = blindscan_ctrl.scan_cond[i].en_polar;
                    if (MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT == g_current_fe_attr[tuner_id].demod_dev_type)
                    {
                        unNotify.result = &stResult;
                        /* Notify new TP */
                        //printf("----------unf line:%d Notify new TP-----\n", __LINE__);
                        bs_para.scan_para.sat.scan_notify(tuner_id, MT_UNF_FE_BLINDSCAN_EVT_NEWRESULT, &unNotify);
                    }

                    all_freq = bs_para.scan_para.sat.stop_freq - bs_para.scan_para.sat.start_freq;
                    //printf("----------unf line:%d all_freq=%d-----\n", __LINE__, all_freq);
                    if (all_freq)
                    {
                        //printf("----------unf line:%d if[%d] start_freq[%d]-----\n", __LINE__,
                            //bs_info.scan_info->p_channel_info[last_num].frequency,
                            //blindscan_ctrl.scan_cond[i].start_freq);
                        //progress_percent = (mt_u16)((bs_info.scan_info->p_channel_info[last_num].frequency/1000 - blindscan_ctrl.scan_cond[i].start_freq) * 100 / all_freq);
                        progress_percent = (mt_u16)((bs_info.scan_info->p_channel_info[last_num].frequency - bs_para.scan_para.sat.start_freq) * 100 / all_freq);
                        //printf("----------unf line:%d progress_percent=%d-----\n", __LINE__, progress_percent);
                    }

                    if ((progress_percent > last_progress_percent) && (progress_percent < 100))
                    {
                        /* Notify new status and percent */
                        //printf("----------unf line:%d progress_percent=%d-----\n", __LINE__, progress_percent);
                        unNotify.progress_percent = &progress_percent;
                        bs_para.scan_para.sat.scan_notify(tuner_id, MT_UNF_FE_BLINDSCAN_EVT_PROGRESS, &unNotify);
                        /* Record new percent */
                        last_progress_percent = progress_percent;
                    }
                }

                enStatus = MT_UNF_FE_BLINDSCAN_STATUS_FINISH;
                stop_quit = 1;
                break;
            }

            usleep(50);
        }
    }


    if (stop_quit)
    {
        /* Notify new status and percent */
        unNotify.status = &enStatus;
        bs_para.scan_para.sat.scan_notify(tuner_id, MT_UNF_FE_BLINDSCAN_EVT_STATUS, &unNotify);
        //break;
    }

    /*unNotify.ppstVerifyStart = &pstVerifyStart;
    bs_para.blindscan_ctrl.stSat.pfnEVTNotify(tuner_id, HI_UNF_TUNER_BLINDSCAN_EVT_VERIFY, &unNotify);
    pstVerifyStart = *(unNotify.ppstVerifyStart);*/


    /*if (g_sat_para[tuner_id].pBlindScanMonitor)
    {
        free(g_sat_para[tuner_id].pBlindScanMonitor);
        g_sat_para[tuner_id].pBlindScanMonitor = HI_NULL;
    }*/

    mt_free(MT_ID_FRONTEND, bs_info.scan_info->p_channel_info);
    bs_info.scan_info->p_channel_info = NULL;

    g_sat_para[tuner_id].blindscan_busy = MT_FALSE;
    g_sat_para[tuner_id].blindscan_stop = MT_FALSE;
    return MT_NULL;
}

#else
static int fe_get_netlink_family_id(int sock_fd)
{
    int len_family;
    struct nlmsghdr *nlh_family = NULL;
    struct nlmsghdr *nlh = NULL;
    struct genlmsghdr *gehdr = NULL;
    struct nlattr *nla = NULL;
    char *fe_nl_name = NULL;
    int fe_nl_name_len;
    struct iovec iov[1];
    struct sockaddr_nl kernel_addr;
    struct msghdr msg;
    int sendcnt;
    int recvcnt;
    int rem;
    void *pos = NULL;
    int family_id;
    
    len_family = NLMSG_SPACE(GENL_HDRLEN + NLA_HDRLEN + GENL_NAMSIZ);
    nlh_family = (struct nlmsghdr *)mt_malloc(MT_ID_FRONTEND, (mt_u32)len_family);
    nlh = nlh_family;
    memset(nlh, 0, (size_t)len_family);

    nlh->nlmsg_len = (__u32)len_family;
    nlh->nlmsg_pid = (pthread_self() << 16 | getpid()); /* self id */
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = GENL_ID_CTRL;
    nlh->nlmsg_seq = DVBS_BS_SEQ;

    gehdr = NLMSG_DATA(nlh);         //get the netlink message payload's pointer
    gehdr->cmd = CTRL_CMD_GETFAMILY; /* copy from genl_ctrl_ops in kernel */
    gehdr->version = 0x2;
    gehdr->reserved = 0;

    nla = (struct nlattr *)((void *)gehdr + GENL_HDRLEN); //the generic netlick message payload's pointer
    fe_nl_name_len = (int)strlen(DVBS_BS_NL_NAME);
    nla->nla_type = CTRL_ATTR_FAMILY_NAME;
    nla->nla_len = NLA_HDRLEN + NLA_ALIGN(fe_nl_name_len + 1);
    fe_nl_name = (char *)((void *)nla + NLA_HDRLEN);
    strcpy(fe_nl_name, DVBS_BS_NL_NAME);
    fe_nl_name[fe_nl_name_len] = '\0';

    iov[0].iov_base = (void *)nlh;
    iov[0].iov_len = nlh->nlmsg_len;

    memset(&kernel_addr, 0, sizeof(kernel_addr));
    kernel_addr.nl_family = AF_NETLINK;
    kernel_addr.nl_pid = 0; /* send to kernel */
    kernel_addr.nl_groups = 0;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&kernel_addr;
    msg.msg_namelen = sizeof(kernel_addr);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    sendcnt = sendmsg(sock_fd, &msg, 0); /* ask driver family id */
    //printf("sendcnt ctrl = %d\n", sendcnt);
    if (sendcnt < 0)
    {
        MT_ERR_FRONTEND("sendmsg failed, errno = %d, %s\n", errno, strerror(errno));
    }
    mt_free(MT_ID_FRONTEND, nlh_family);

    len_family = 4096; /* from NLMSG_DEFAULT_SIZE + NLMSG_HDRLEN */
    nlh_family = (struct nlmsghdr *)mt_malloc(MT_ID_FRONTEND, (mt_u32)len_family);
    memset(nlh_family, 0, (size_t)len_family);

    memset(&msg, 0, sizeof(msg));
    iov[0].iov_base = (void *)nlh_family;
    iov[0].iov_len = (size_t)len_family;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    recvcnt = recvmsg(sock_fd, &msg, 0);
    if (recvcnt <= 0)
    {
        MT_ERR_FRONTEND("recv family id failed\n");
        close(sock_fd);
        return MT_FAILURE;
    }
    //printf("recvcnt ctrl = %d\n", recvcnt);

    nlh = nlh_family;

    while (NLMSG_OK(nlh, recvcnt))
    {
        //printf("nlh->nlmsg_len = %d\n", nlh->nlmsg_len);
        //printf("nlh->nlmsg_seq = 0x%x\n", nlh->nlmsg_seq);

        if (nlh->nlmsg_type != GENL_ID_CTRL)
        {
            nlh = NLMSG_NEXT(nlh, recvcnt);
            continue;
        }

        if ((recvcnt - GENL_HDRLEN) >= (int)sizeof(struct nlmsghdr))
            gehdr = NLMSG_DATA(nlh);
        else
        {
            nlh = NLMSG_NEXT(nlh, recvcnt);
            continue;
        }
        nla = (struct nlattr *)((void *)gehdr + GENL_HDRLEN);

        for (pos = nla, rem = (int)(nlh->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN);
             rem >= NLA_HDRLEN && nla->nla_len >= NLA_HDRLEN && nla->nla_len <= rem;
             rem -= NLA_ALIGN(nla->nla_len), pos += NLA_ALIGN(nla->nla_len))
        {
            if (((struct nlattr *)pos)->nla_len > NLA_HDRLEN)
            {
                if (((struct nlattr *)pos)->nla_type == CTRL_ATTR_FAMILY_ID)
                {
                    family_id = *(int *)((void *)pos + NLA_HDRLEN);
                    MT_INFO_FRONTEND("family_id = 0x%x\n", family_id);
                    mt_free(MT_ID_FRONTEND, nlh_family);
                    goto return_fe_family_id;
                }
            }
        }

        nlh = NLMSG_NEXT(nlh, recvcnt);
    }

    MT_ERR_FRONTEND("not found family id\n");

    return MT_FAILURE;
return_fe_family_id:
    return family_id;
}
mt_void *fe_dvbs_blindscan_thread(mt_void *p_blindscan)
{
    mt_unf_fe_blindscan_para_t bs_para;
    //    fe_blindscan_t blindscan;
    fe_blindscan_info_t bs_info;
    fe_blindscan_param_t stActionPara;
    blindscan_ctrl_t blindscan_ctrl = {0};
    mt_unf_fe_sat_tpinfo_t stResult;
    mt_unf_fe_blindscan_notify_t unNotify;
    mt_u32 all_freq = 0;
    mt_u32 tuner_id;
    mt_s32 ret = MT_FAILURE;
    mt_s32 i; //, j, k;
    mt_s32 if_start = 0;
    mt_s32 if_stop = 0;
    mt_u32 dlink_start = 0;
    mt_u32 dlink_stop = 0;
    //    mt_u32 last_freq_khz = 0;
    mt_u16 progress_percent = 0, tpfind_percent = 0;
    mt_u16 last_progress_percent = 0, last_tpfind_percent = 0;
    mt_unf_fe_blindscan_status_t enStatus = MT_UNF_FE_BLINDSCAN_STATUS_IDLE;
    mt_u8 status = 0;
    mt_u8 stop_quit = 0;
    //mt_s32 no_meaning = 0;
    fe_data_t fe_data = {0};

    int sock_fd = -1;
    struct sockaddr_nl app_addr;
    struct sockaddr_nl kernel_addr;
    struct msghdr msg;
    struct nlmsghdr *nlh = NULL;
    struct nlmsghdr *nlh_family = NULL;
    struct nlmsghdr *nlh_snd = NULL;
    struct nlmsghdr *nlh_rcv = NULL;
    struct genlmsghdr *gehdr = NULL;
    struct nlattr *nla = NULL;
    void *pos = NULL;
    struct iovec iov[1];
    struct dvbs_bs_netlink_data *fe_nl_data = NULL;
    int len_family;
    int len_snd;
    int len_rcv;
    int sendcnt;
    int recvcnt;
    char *fe_nl_name = NULL;
    int fe_nl_name_len;
    int rem;
    int family_id;
    int seed = 0;
    fd_set fds;
    struct timeval tv;
    int sel_ret;

    //printf("[%s %d]enter\n", __FUNCTION__, __LINE__);
    mt_set_pthread_name(__FUNCTION__);

    sock_fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_GENERIC);
    if (sock_fd < 0)
    {
        printf("[%s %d]create socket failed\n", __FUNCTION__, __LINE__);
        return MT_NULL;
    }
    //printf("[%s %d]socket = %d\n", __FUNCTION__, __LINE__, sock_fd);

    memset(&app_addr, 0, sizeof(app_addr));
    app_addr.nl_family = AF_NETLINK;
    app_addr.nl_pid = getpid(); /* self pid */
    app_addr.nl_groups = 0;

    //printf("app_addr.nl_pid = %d\n", app_addr.nl_pid);

    bind(sock_fd, (struct sockaddr *)&app_addr, sizeof(app_addr));

#if 1
    len_family = NLMSG_SPACE(GENL_HDRLEN + NLA_HDRLEN + GENL_NAMSIZ);
    nlh_family = (struct nlmsghdr *)mt_malloc(MT_ID_FRONTEND, (mt_u32)len_family);
    nlh = nlh_family;
    memset(nlh, 0, (size_t)len_family);

    nlh->nlmsg_len = (__u32)len_family;
    nlh->nlmsg_pid = (pthread_self() << 16 | getpid()); /* self id */
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = GENL_ID_CTRL;
    nlh->nlmsg_seq = DVBS_BS_SEQ;

    gehdr = NLMSG_DATA(nlh);         //get the netlink message payload's pointer
    gehdr->cmd = CTRL_CMD_GETFAMILY; /* copy from genl_ctrl_ops in kernel */
    gehdr->version = 0x2;
    gehdr->reserved = 0;

    nla = (struct nlattr *)((void *)gehdr + GENL_HDRLEN); //the generic netlick message payload's pointer
    fe_nl_name_len = (int)strlen(DVBS_BS_NL_NAME);
    nla->nla_type = CTRL_ATTR_FAMILY_NAME;
    nla->nla_len = NLA_HDRLEN + NLA_ALIGN(fe_nl_name_len + 1);
    fe_nl_name = (char *)((void *)nla + NLA_HDRLEN);
    strcpy(fe_nl_name, DVBS_BS_NL_NAME);
    fe_nl_name[fe_nl_name_len] = '\0';

    iov[0].iov_base = (void *)nlh;
    iov[0].iov_len = nlh->nlmsg_len;

    memset(&kernel_addr, 0, sizeof(kernel_addr));
    kernel_addr.nl_family = AF_NETLINK;
    kernel_addr.nl_pid = 0; /* send to kernel */
    kernel_addr.nl_groups = 0;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&kernel_addr;
    msg.msg_namelen = sizeof(kernel_addr);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    sendcnt = sendmsg(sock_fd, &msg, 0); /* ask driver family id */
    //printf("sendcnt ctrl = %d\n", sendcnt);
    if (sendcnt < 0)
    {
        MT_ERR_FRONTEND("sendmsg failed, errno = %d, %s\n", errno, strerror(errno));
    }
    mt_free(MT_ID_FRONTEND, nlh_family);
#endif

#if 1
    len_family = 4096; /* from NLMSG_DEFAULT_SIZE + NLMSG_HDRLEN */
    nlh_family = (struct nlmsghdr *)mt_malloc(MT_ID_FRONTEND, (mt_u32)len_family);
    memset(nlh_family, 0, (size_t)len_family);

    memset(&msg, 0, sizeof(msg));
    iov[0].iov_base = (void *)nlh_family;
    iov[0].iov_len = (size_t)len_family;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    recvcnt = recvmsg(sock_fd, &msg, 0);
    if (recvcnt <= 0)
    {
        MT_ERR_FRONTEND("recv family id failed\n");
        close(sock_fd);
        return MT_NULL;
    }
    //printf("recvcnt ctrl = %d\n", recvcnt);

    nlh = nlh_family;

    while (NLMSG_OK(nlh, recvcnt))
    {
        //printf("nlh->nlmsg_len = %d\n", nlh->nlmsg_len);
        //printf("nlh->nlmsg_seq = 0x%x\n", nlh->nlmsg_seq);

        if (nlh->nlmsg_type != GENL_ID_CTRL)
        {
            nlh = NLMSG_NEXT(nlh, recvcnt);
            continue;
        }

        if ((recvcnt - GENL_HDRLEN) >= (int)sizeof(struct nlmsghdr))
            gehdr = NLMSG_DATA(nlh);
        else
        {
            nlh = NLMSG_NEXT(nlh, recvcnt);
            continue;
        }
        nla = (struct nlattr *)((void *)gehdr + GENL_HDRLEN);

        for (pos = nla, rem = (int)(nlh->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN);
             rem >= NLA_HDRLEN && nla->nla_len >= NLA_HDRLEN && nla->nla_len <= rem;
             rem -= NLA_ALIGN(nla->nla_len), pos += NLA_ALIGN(nla->nla_len))
        {
            if (((struct nlattr *)pos)->nla_len > NLA_HDRLEN)
            {
                if (((struct nlattr *)pos)->nla_type == CTRL_ATTR_FAMILY_ID)
                {
                    family_id = *(int *)((void *)pos + NLA_HDRLEN);
                    MT_INFO_FRONTEND("family_id = 0x%x\n", family_id);
                    mt_free(MT_ID_FRONTEND, nlh_family);
                    goto got_fe_family_id;
                }
            }
        }

        nlh = NLMSG_NEXT(nlh, recvcnt);
    }

    MT_ERR_FRONTEND("not found family id\n");

    return MT_NULL;

got_fe_family_id: /* now communicate with blindscan */

    len_snd = NLMSG_SPACE(GENL_HDRLEN + NLA_HDRLEN + sizeof(struct dvbs_bs_netlink_data));
    nlh_snd = (struct nlmsghdr *)mt_malloc(MT_ID_FRONTEND, (mt_u32)len_snd);
    if (NULL == nlh_snd)
    {
        close(sock_fd);
        sock_fd = -1;
        return MT_NULL;
    }

    memset(nlh_snd, 0, (size_t)len_snd);
    nlh = nlh_snd;

    len_rcv = 4096; /* from NLMSG_DEFAULT_SIZE + NLMSG_HDRLEN */
    nlh_rcv = (struct nlmsghdr *)mt_malloc(MT_ID_FRONTEND, (mt_u32)len_rcv);
    if (NULL == nlh_rcv)
    {
        mt_free(MT_ID_FRONTEND, nlh_snd);
        nlh_snd = NULL;

        close(sock_fd);
        sock_fd = -1;
        return MT_NULL;
    }
    memset(nlh_rcv, 0, (size_t)len_rcv);
#endif

    if (MT_NULL == p_blindscan)
    {
        MT_ERR_FRONTEND("Input parameter(p_blindscan) invalid\n");

        mt_free(MT_ID_FRONTEND, nlh_rcv);
        nlh_rcv = NULL;

        mt_free(MT_ID_FRONTEND, nlh_snd);
        nlh_snd = NULL;

        close(sock_fd);
        sock_fd = -1;
        return MT_NULL;
    }

    memset(&bs_para, 0, sizeof(mt_unf_fe_blindscan_para_t));

    tuner_id = ((blindscan_para_t *)p_blindscan)->tuner_id;
    bs_para = ((blindscan_para_t *)p_blindscan)->scan_info;

    if (MT_NULL == (bs_para.scan_para.sat.scan_notify))
    {
        MT_ERR_FRONTEND("Input parameter(scan_notify) invalid\n");
        /*if (g_sat_para[tuner_id].pBlindScanMonitor)
        {
            free(g_sat_para[tuner_id].pBlindScanMonitor);
            g_sat_para[tuner_id].pBlindScanMonitor = HI_NULL;
        }*/

        g_sat_para[tuner_id].blindscan_stop = MT_FALSE;

        mt_free(MT_ID_FRONTEND, nlh_rcv);
        nlh_rcv = NULL;

        mt_free(MT_ID_FRONTEND, nlh_snd);
        nlh_snd = NULL;

        close(sock_fd);
        sock_fd = -1;

        return MT_NULL;
    }

    g_sat_para[tuner_id].blindscan_busy = MT_TRUE;

    memset(&stActionPara, 0, sizeof(stActionPara));
    bs_info.tuner_id = tuner_id;
    bs_info.scan_info = &stActionPara;

    /* Manual scan mode */
    if (MT_UNF_FE_BLINDSCAN_MODE_MANUAL == bs_para.mode)
    {
        /*unicable, convert IF to downlink freq first */
        if (MT_UNF_FE_LNB_UNICABLE == g_sat_para[tuner_id].lnb_config.lnb_type)
        {
            /*use 22k select low lo or high lo*/
            fe_if_to_downlinkfreq(&(g_sat_para[tuner_id].lnb_config), bs_para.scan_para.sat.polar, bs_para.scan_para.sat.lnb_22k,
                                  bs_para.scan_para.sat.start_freq, &dlink_start);
            fe_if_to_downlinkfreq(&(g_sat_para[tuner_id].lnb_config), bs_para.scan_para.sat.polar, bs_para.scan_para.sat.lnb_22k,
                                  bs_para.scan_para.sat.stop_freq, &dlink_stop);

            blindscan_ctrl.scan_times = 1;
            //printf("unf thread line[d]: dlink_start[%d] dlink_stop[%d] lnb_22k[%d].\n", dlink_start, dlink_stop, bs_para.scan_para.sat.lnb_22k);
            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, bs_para.scan_para.sat.lnb_22k, bs_para.scan_para.sat.polar,
                                    (mt_s32)(dlink_start / 1000), (mt_s32)(dlink_stop / 1000));
        }
        else /*not unicable */
        {
            blindscan_ctrl.scan_times = 1;
            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, bs_para.scan_para.sat.lnb_22k, bs_para.scan_para.sat.polar,
                                    (mt_s32)(bs_para.scan_para.sat.start_freq / 1000), (mt_s32)(bs_para.scan_para.sat.stop_freq / 1000));
        }
    }
    /* Auto scan mode */
    else
    {
        if (MT_UNF_FE_LNB_UNICABLE != g_sat_para[tuner_id].lnb_config.lnb_type)
        {
            /* Single LO */
            if ((MT_UNF_FE_LNB_SINGLE_FREQUENCY == g_sat_para[tuner_id].lnb_config.lnb_type) || (g_sat_para[tuner_id].lnb_config.low_lo == g_sat_para[tuner_id].lnb_config.high_lo))
            {
                /* C band */
                if (MT_UNF_FE_LNB_BAND_C == g_sat_para[tuner_id].lnb_config.lnb_band)
                {
                  /*fix issue 133586*/
                  /* When the local oscillator is 5150, the maximum FREQ is indeed 4200MHz, 
                   * but when the local oscillator is 5750, the maximum FREQ is 4800MHz.
                   * So when the local oscillator is 5750, you should subtract 4800MHz instead of 4200MHz.
                   * In a word, in the single local oscillator mode, the range of intermediate frequency is 950MHz to 2150MHz
                   */
                    blindscan_ctrl.scan_times = 2;
                    set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, /*MT_FALSE*/ MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, SAT_IF_C_L_MIN, SAT_IF_C_H_MAX);
                    set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, /*MT_FALSE*/ MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, SAT_IF_C_L_MIN, SAT_IF_C_H_MAX);
                }
                /* Ku band */
                else
                {
                    if_start = (mt_s32)(SAT_KU_MIN - g_sat_para[tuner_id].lnb_config.low_lo);
                    if_stop = (mt_s32)(SAT_KU_MAX - g_sat_para[tuner_id].lnb_config.low_lo);
                    blindscan_ctrl.scan_times = 2;
                    set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, /*MT_FALSE*/ MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                    set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, /*MT_FALSE*/ MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                }
            }
            /* Dual LO */
            else
            {
                /* C band */
                if (MT_UNF_FE_LNB_BAND_C == g_sat_para[tuner_id].lnb_config.lnb_band)
                {
                    if ((g_sat_para[tuner_id].lnb_config.low_lo == SAT_LO_C_L) && (g_sat_para[tuner_id].lnb_config.high_lo == SAT_LO_C_H))
                    {
                        /* 13V/18V select High/Low LO here */
                        blindscan_ctrl.scan_times = 2;
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, /*MT_FALSE*/ MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, SAT_IF_C_L_MIN, SAT_IF_C_L_MAX);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, /*MT_FALSE*/ MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, SAT_IF_C_H_MIN, SAT_IF_C_H_MAX);
                    }
                    /* Special C band LNB */
                    else
                    {
                        blindscan_ctrl.scan_times = 2;
                        if_start = (mt_s32)(g_sat_para[tuner_id].lnb_config.low_lo - SAT_C_MAX);
                        if_stop = (mt_s32)(g_sat_para[tuner_id].lnb_config.low_lo - SAT_C_MIN);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, /*MT_FALSE*/ MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                        if_start = (mt_s32)(g_sat_para[tuner_id].lnb_config.high_lo - SAT_C_MAX);
                        if_stop = (mt_s32)(g_sat_para[tuner_id].lnb_config.high_lo - SAT_C_MIN);
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, /*MT_FALSE*/ MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                    }
                }
                /* Ku band */
                else
                {
                    if (g_sat_para[tuner_id].lnb_config.low_lo == SAT_LO_DSS_A3_L)
                    {
                        blindscan_ctrl.scan_times = 1;
                        if_start = (mt_s32)SAT_IF_DSS_A3_L_MIN;
                        if_stop = (mt_s32)SAT_IF_DSS_A3_L_MAX;
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                    }
                    else if (g_sat_para[tuner_id].lnb_config.high_lo == SAT_LO_DSS_A3_H)
                    {
                        blindscan_ctrl.scan_times = 1;
                        if_start = (mt_s32)SAT_IF_DSS_A3_H_MIN;
                        if_stop = (mt_s32)SAT_IF_DSS_A3_H_MAX;
                        set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                    }
                    else // Normal LNB
                    {
                        /*no 22k switch*/
                        if (MT_UNF_FE_SWITCH_22K_NONE == g_sat_para[tuner_id].en_switch_22k)
                        {
                            blindscan_ctrl.scan_times = 4;
                            if_start = (mt_s32)(/*SAT_KU_MIN*/ SAT_DOWNLINK_FREQ_KU_MID - g_sat_para[tuner_id].lnb_config.high_lo);
                            if_stop = (mt_s32)(SAT_KU_MAX - g_sat_para[tuner_id].lnb_config.high_lo);
                            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_ON, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 2, MT_UNF_FE_LNB_22K_ON, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                            if_start = (mt_s32)(SAT_KU_MIN - g_sat_para[tuner_id].lnb_config.low_lo);
                            if_stop = (mt_s32)(/*SAT_KU_MAX*/ SAT_DOWNLINK_FREQ_KU_MID - g_sat_para[tuner_id].lnb_config.low_lo);
                            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 3, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                        }
                        /*22k switch 0Hz port*/
                        else if (MT_UNF_FE_SWITCH_22K_0 == g_sat_para[tuner_id].en_switch_22k)
                        {
                            blindscan_ctrl.scan_times = 2;
                            if_start = (mt_s32)(SAT_KU_MIN - g_sat_para[tuner_id].lnb_config.low_lo);
                            if_stop = (mt_s32)(/*SAT_KU_MAX*/ SAT_DOWNLINK_FREQ_KU_MID - g_sat_para[tuner_id].lnb_config.low_lo);
                            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                        }
                        /*22k switch 22kHz port*/
                        else if (MT_UNF_FE_SWITCH_22K_22 == g_sat_para[tuner_id].en_switch_22k)
                        {
                            blindscan_ctrl.scan_times = 2;
                            if_start = (mt_s32)(/*SAT_KU_MIN*/ SAT_DOWNLINK_FREQ_KU_MID - g_sat_para[tuner_id].lnb_config.high_lo);
                            if_stop = (mt_s32)(SAT_KU_MAX - g_sat_para[tuner_id].lnb_config.high_lo);
                            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_ON, MT_UNF_FE_POLARIZATION_H, if_start, if_stop);
                            set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, MT_UNF_FE_LNB_22K_ON, MT_UNF_FE_POLARIZATION_V, if_start, if_stop);
                        }
                    }
                }
            }
        }
        else /*unicable LNB */
        {
            /*Single LO, bs 2 times, start downlink freq, stop downlink freq */
            if (g_sat_para[tuner_id].lnb_config.low_lo == g_sat_para[tuner_id].lnb_config.high_lo)
            {
                blindscan_ctrl.scan_times = 2;
                dlink_start = (SAT_IF_MIN + g_sat_para[tuner_id].lnb_config.low_lo);
                dlink_stop = (SAT_IF_MAX + g_sat_para[tuner_id].lnb_config.low_lo);
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, (mt_s32)dlink_start, (mt_s32)dlink_stop);
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, (mt_s32)dlink_start, (mt_s32)dlink_stop);
            }
            /*Dual LO, bs 4 times, start downlink freq, stop downlink freq */
            else
            {
                blindscan_ctrl.scan_times = 4;
                dlink_start = SAT_DOWNLINK_FREQ_KU_MID;
                dlink_stop = (SAT_IF_MAX + g_sat_para[tuner_id].lnb_config.high_lo);
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 0, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, (mt_s32)dlink_start, (mt_s32)dlink_stop);
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 2, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, (mt_s32)dlink_start, (mt_s32)dlink_stop);
                dlink_start = (SAT_IF_MIN + g_sat_para[tuner_id].lnb_config.low_lo);
                dlink_stop = SAT_DOWNLINK_FREQ_KU_MID;
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 1, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_H, (mt_s32)dlink_start, (mt_s32)dlink_stop);
                set_blindscan_ctrl_cond(tuner_id, &blindscan_ctrl, 3, MT_UNF_FE_LNB_22K_OFF, MT_UNF_FE_POLARIZATION_V, (mt_s32)dlink_start, (mt_s32)dlink_stop);
            }
        }
    }

    //printf("----thread---line=%d-----------------------\n", __LINE__);

    for (i = 0; i < (mt_s32)(blindscan_ctrl.scan_times); i++)
    {
        mt_u8 unicable_scan_times = 1, bs_time = 0;

        if (g_sat_para[tuner_id].blindscan_stop)
        {
            printf("[%s %d]cancel blindscan, enStatus = %d, quit loop\n", __FUNCTION__, __LINE__, enStatus);
            enStatus = MT_UNF_FE_BLINDSCAN_STATUS_QUIT;
            stop_quit = 1;
            break;
        }

        /* If register diseqc_set, set DiSEqC */
        if (bs_para.scan_para.sat.diseqc_set)
        {
            bs_para.scan_para.sat.diseqc_set(tuner_id,
                                             blindscan_ctrl.scan_cond[i].en_polar,
                                             blindscan_ctrl.scan_cond[i].en_lnb_22k);
        }

        if (0 == bs_para.scan_para.sat.lnb_mannual)
        {
            ret = fe_set_lnbout_and_22k(tuner_id, blindscan_ctrl.scan_cond[i].en_polar, blindscan_ctrl.scan_cond[i].en_lnb_22k);
            //printf("usr dvbs thread line[%d] g_fe_fd=%d-------\n", __LINE__, g_fe_fd);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("fe_set_lnbout_and_22k fail.\n");
            }
        }
        else
        {
            if (bs_para.scan_para.sat.p_lnbout_and_22k)
            {
                bs_para.scan_para.sat.p_lnbout_and_22k(tuner_id,
                                                       blindscan_ctrl.scan_cond[i].en_polar,
                                                       blindscan_ctrl.scan_cond[i].en_lnb_22k);
            }
        }

        usleep(100 * 1000);

        //maybe has call mt_unf_fe_blindscan_stop in usleep
        //so check the g_sat_para[tuner_id].blindscan_stop again after usleep
        if (g_sat_para[tuner_id].blindscan_stop)
        {
            printf("[%s %d]cancel blindscan, enStatus = %d, quit loop\n", __FUNCTION__, __LINE__, enStatus);
            enStatus = MT_UNF_FE_BLINDSCAN_STATUS_QUIT;
            stop_quit = 1;
            break;
        }

        //printf("usr dvbs thread line[%d] lnb_type[%d] en_polar[%d] en_lnb_22k[%d]\n", __LINE__,
        //g_sat_para[tuner_id].lnb_config.lnb_type,
        //blindscan_ctrl.scan_cond[i].en_polar,
        //blindscan_ctrl.scan_cond[i].en_lnb_22k);

        /*unicable should send ODU_ChannelChange cmd first, then calculate the if freq. */
        if (MT_UNF_FE_LNB_UNICABLE == g_sat_para[tuner_id].lnb_config.lnb_type)
        {
            ///printf("-------unf line:%d start_freq = %d, stop_freq = %d--------\n",
            //__LINE__, blindscan_ctrl.scan_cond[i].start_freq, blindscan_ctrl.scan_cond[i].stop_freq);
            if ((MT_UNF_DEMOD_DEV_TYPE_M88DM6K != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88RS6060 != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88DS6103 != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88DS6113 != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88CT8K != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88CS8800 != g_current_fe_attr[tuner_id].demod_dev_type))
            {
                (mt_void) unicable_chan_change(tuner_id, blindscan_ctrl.scan_cond[i].stop_freq, blindscan_ctrl.scan_cond[i].en_polar);
            }
            else // for Montage DVB-S/S2 demod Unicable blindscan
            {
                if (g_sat_para[tuner_id].lnb_config.unicable_ver_no == 2) // DCSS, Unicable 2.0
                {
                    unicable_scan_times = 1;
                }
                else // SCR, Unicable 1.0, blindscan 2 times to compatiable with different Unicable devices
                {
                    //unicable_scan_times = 2;
                    /*fix issue30100 duplicate TP and program occurs when scanning twice */
                    unicable_scan_times = 1;
                }
            }

            //bs_info.scan_info->start_freq = 950 * 1000;
            //bs_info.scan_info->stop_freq = 2150 * 1000;
            bs_info.scan_info->start_freq = bs_para.scan_para.sat.start_freq;
            bs_info.scan_info->stop_freq = bs_para.scan_para.sat.stop_freq;
            //printf("unf line[%d] start_freq[%d] stop_freq[%d]\r\n", __LINE__, bs_info.scan_info->start_freq, bs_info.scan_info->stop_freq);
#if 0
            if (MT_UNF_FE_SATPOSN_A == g_sat_para[tuner_id].lnb_config.unicable_port_no)
            {
                if (MT_UNF_FE_POLARIZATION_V == blindscan_ctrl.scan_cond[i].en_polar)
                {
                    g_sat_para[tuner_id].lnb_config.unicable_bank = 1;
                }
                else
                {
                    g_sat_para[tuner_id].lnb_config.unicable_bank = 3;
                }
            }
            else
            {
                if (MT_UNF_FE_POLARIZATION_V == blindscan_ctrl.scan_cond[i].en_polar)
                {
                    g_sat_para[tuner_id].lnb_config.unicable_bank = 5;
                }
                else
                {
                    g_sat_para[tuner_id].lnb_config.unicable_bank = 7;
                }
            }
#else
            if (MT_UNF_FE_SATPOSN_A == g_sat_para[tuner_id].lnb_config.unicable_port_no)
            {
                if (MT_UNF_FE_POLARIZATION_V == blindscan_ctrl.scan_cond[i].en_polar)
                {
                    if (MT_UNF_FE_LNB_22K_ON == blindscan_ctrl.scan_cond[i].en_lnb_22k)
                    {
                        g_sat_para[tuner_id].lnb_config.unicable_bank = 1; // 0
                    }
                    else
                    {
                        g_sat_para[tuner_id].lnb_config.unicable_bank = 0; // 1
                    }
                }
                else
                {
                    if (MT_UNF_FE_LNB_22K_ON == blindscan_ctrl.scan_cond[i].en_lnb_22k)
                    {
                        g_sat_para[tuner_id].lnb_config.unicable_bank = 3; // 2
                    }
                    else
                    {
                        g_sat_para[tuner_id].lnb_config.unicable_bank = 2; // 3
                    }
                }
            }
            else
            {
                if (MT_UNF_FE_POLARIZATION_V == blindscan_ctrl.scan_cond[i].en_polar)
                {
                    if (MT_UNF_FE_LNB_22K_ON == blindscan_ctrl.scan_cond[i].en_lnb_22k)
                    {
                        g_sat_para[tuner_id].lnb_config.unicable_bank = 5; // 4
                    }
                    else
                    {
                        g_sat_para[tuner_id].lnb_config.unicable_bank = 4; // 5
                    }
                }
                else
                {
                    if (MT_UNF_FE_LNB_22K_ON == blindscan_ctrl.scan_cond[i].en_lnb_22k)
                    {
                        g_sat_para[tuner_id].lnb_config.unicable_bank = 7; // 6
                    }
                    else
                    {
                        g_sat_para[tuner_id].lnb_config.unicable_bank = 6; // 7
                    }
                }
            }
#endif
            printf("%s bank[%d] high[%d] low[%d]\n", __FUNCTION__, g_sat_para[tuner_id].lnb_config.unicable_bank, g_sat_para[tuner_id].lnb_config.high_lo, g_sat_para[tuner_id].lnb_config.low_lo);

            bs_info.scan_info->uc_param.use_uc = 1;
            bs_info.scan_info->uc_param.bank = g_sat_para[tuner_id].lnb_config.unicable_bank;
            bs_info.scan_info->uc_param.user_band = g_sat_para[tuner_id].lnb_config.unicable_scr_no;
            bs_info.scan_info->uc_param.ub_freq_mhz = (mt_u16)g_sat_para[tuner_id].lnb_config.unicable_if_freq_mhz;
            bs_info.scan_info->uc_param.ub_ver = g_sat_para[tuner_id].lnb_config.unicable_ver_no;
            //printf("usr dvbs thread line:%d start_freq = %d, stop_freq = %d.\n",
            //__LINE__, blindscan_ctrl.scan_cond[i].start_freq, blindscan_ctrl.scan_cond[i].stop_freq);
        }
        else
        {
            bs_info.scan_info->start_freq = (mt_u32)blindscan_ctrl.scan_cond[i].start_freq * 1000;
            bs_info.scan_info->stop_freq = (mt_u32)blindscan_ctrl.scan_cond[i].stop_freq * 1000;
            //bs_info.scan_info->start_freq = 950 * 1000;
            //bs_info.scan_info->stop_freq = 2150 * 1000;
        }

        //printf("usr dvbs thread i=%d start_freq=%d stop_freq=%d\n", i,
        //blindscan_ctrl.scan_cond[i].start_freq,
        //blindscan_ctrl.scan_cond[i].stop_freq);

        for (bs_time = 0; bs_time < unicable_scan_times; bs_time++)
        {
            if (bs_time == 0)
            {
                bs_info.scan_info->invert_spectrum = 0; // normal spectrum

                printf("%s[%d] ---- Normal spectrum blindscan.\n", __FUNCTION__, __LINE__);
            }
            else
            {
                bs_info.scan_info->invert_spectrum = 1; // invert spectrum for the 2nd time blindscan

                printf("%s[%d] ---- Inverted spectrum blindscan for Unicable.\n", __FUNCTION__, __LINE__);
            }

            if (g_sat_para[tuner_id].blindscan_stop)
            {
                printf("[%s %d]cancel blindscan, enStatus = %d, quit loop\n", __FUNCTION__, __LINE__, enStatus);
                enStatus = MT_UNF_FE_BLINDSCAN_STATUS_QUIT;
                stop_quit = 1;
                break;
            }
            /* Fix issue 23875
             * Because the unregister and register actions will cause the value of the family id to change.
             * So Drv provide a new API to deregister and register.
             * Avoid inconsistent API and DRV family ids.
             * The APP needs to call the drv's registered family interface before BLSCAN and get new family id 
             */
            fe_data.port = tuner_id;
            fe_data.data = 0;//no meaning
            ret = ioctl(g_fe_fd, FE_REGISTER_NLK_FAMILY_CMD,&fe_data);
            if(MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("FE_REGISTER_NLK_FAMILY_CMD error %d\n", ret);
                enStatus = MT_UNF_FE_BLINDSCAN_STATUS_QUIT;
                stop_quit = 1;
                break;
            }
            else
            {
                family_id = fe_get_netlink_family_id(sock_fd);
                if(family_id == MT_FAILURE)
                {
                    MT_ERR_FRONTEND("not found family id\n");
                    enStatus = MT_UNF_FE_BLINDSCAN_STATUS_QUIT;
                    stop_quit = 1;
                    break;
                }
                /*MT_ERR_FRONTEND("[%s %d]family_id = %d\n", __FUNCTION__, __LINE__, family_id);*/
            }
            ret = ioctl(g_fe_fd, FE_BLINDSCAN_ACTION_CMD, &bs_info);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("TUNER_BLINDSCAN_ACTION_CMD error %d\n", ret);
                enStatus = MT_UNF_FE_BLINDSCAN_STATUS_FAIL;
            }
            else
            {
                enStatus = MT_UNF_FE_BLINDSCAN_STATUS_SCANNING;
            }

            //printf("usr dvbs thread line[%d] enStatus=%d.\n", __LINE__, enStatus);

#if 1
            /* New TP callback */
            while (1) //bs_info.scan_info->channel_num_cur > last_num)
            {
                /* Finish status */
                if (0) //(u32StepNum - 1 == (mt_u32)j) && ((mt_u32)i == blindscan_ctrl.scan_times - 1))
                {
                    enStatus = MT_UNF_FE_BLINDSCAN_STATUS_FINISH;
                }

                /* Quit status */
                if (g_sat_para[tuner_id].blindscan_stop)
                {
                    enStatus = MT_UNF_FE_BLINDSCAN_STATUS_QUIT;
#if 0
                    if (MT_NULL != g_sat_para[tuner_id].p_bs_monitor)
                    {
                        g_sat_para[tuner_id].blindscan_stop = MT_TRUE;
                        ret = ioctl(g_fe_fd, FE_BLIND_SCAN_CANCEL_CMD, &tuner_id);
                        if (ret != MT_SUCCESS)
                        {
                            enStatus = MT_UNF_FE_BLINDSCAN_STATUS_BUTT;
                        }
                    }
#endif
                }

                /* If over, break */
                if (enStatus > MT_UNF_FE_BLINDSCAN_STATUS_SCANNING)
                {
                    //printf("[%s %d]enStatus = %d, quit loop\n", __FUNCTION__, __LINE__, enStatus);
                    stop_quit = 1;
                    break;
                }

                fe_check_bs_status(tuner_id, &status); //check the status of drv's blindscan, 0:runing, 1:finish or abort
                if (status != 0)
                {
                    enStatus = MT_UNF_FE_BLINDSCAN_STATUS_FINISH;
                    //printf("[%s %d]enStatus = %d, quit loop\n", __FUNCTION__, __LINE__, enStatus);
                    stop_quit = 1;
                    break;
                }
                else
                {
                    //usleep(50);
                    //while (1)
                    {
                        nlh = nlh_snd;
                        memset(nlh, 0, (size_t)len_snd);

                        nlh->nlmsg_len = (__u32)len_snd;
                        nlh->nlmsg_pid = (pthread_self() << 16 | getpid()); /* self id */
                        nlh->nlmsg_seq = (__u32)(DVBS_BS_SEQ + seed);
                        nlh->nlmsg_flags = NLM_F_REQUEST;
                        nlh->nlmsg_type = family_id;

                        gehdr = NLMSG_DATA(nlh);
                        gehdr->version = 0x1;
                        gehdr->reserved = 0;

                        nla = (struct nlattr *)((void *)gehdr + GENL_HDRLEN);
                        nla->nla_len = NLA_HDRLEN + NLA_ALIGN(sizeof(struct dvbs_bs_netlink_data));
                        fe_nl_data = (struct dvbs_bs_netlink_data *)((void *)nla + NLA_HDRLEN);
                        snprintf(fe_nl_data->name, sizeof(fe_nl_data->name), "blindscan_%d", seed);
                        fe_nl_data->pid = getpid();
                        fe_nl_data->seq = nlh->nlmsg_seq;
                        fe_nl_data->freq_khz = 0;
                        fe_nl_data->symbol_rate = 0;
                        fe_nl_data->port_type = 0;

                        //printf("usr netlink cycle: line[%d]\n", __LINE__);

                        switch (seed % 4)
                        {
                        case 0:
                            gehdr->cmd = DVBS_BS_NL_OPS_CMD0;
                            nla->nla_type = DVBS_BS_NL_ATTR_DATA0;
                            break;
#if 0
                        case 1:
                            gehdr->cmd = DVBS_BS_NL_OPS_CMD0;
                            nla->nla_type = DVBS_BS_NL_ATTR_DATA1;
                            break;

                        case 2:
                            gehdr->cmd = DVBS_BS_NL_OPS_CMD1;
                            nla->nla_type = DVBS_BS_NL_ATTR_DATA0;
                            break;
#endif
                        default:                                   /* 3 */
                            gehdr->cmd = DVBS_BS_NL_OPS_CMD0;      //CS8K_SAT_NL_OPS_CMD1;
                            nla->nla_type = DVBS_BS_NL_ATTR_DATA0; //CS8K_SAT_NL_ATTR_DATA1;
                        }

                        fe_nl_data->cmd = gehdr->cmd;
                        fe_nl_data->data01 = nla->nla_type;

                        iov[0].iov_base = (void *)nlh;
                        iov[0].iov_len = nlh->nlmsg_len;

                        memset(&kernel_addr, 0, sizeof(kernel_addr));
                        kernel_addr.nl_family = AF_NETLINK;
                        kernel_addr.nl_pid = 0; /* send to kernel */
                        kernel_addr.nl_groups = 0;

                        memset(&msg, 0, sizeof(msg));
                        msg.msg_name = (void *)&kernel_addr;
                        msg.msg_namelen = sizeof(kernel_addr);
                        msg.msg_iov = iov;
                        msg.msg_iovlen = 1;
                        sendcnt = sendmsg(sock_fd, &msg, 0);
                        //printf("sendcnt dvbs_bs = %d\n", sendcnt);
                        if (sendcnt < 0)
                        {
                            MT_ERR_FRONTEND("sendmsg failed, errno=%d, %s\n", errno, strerror(errno));
                        }

                        seed++;

                        //MT_ERR_FRONTEND(" ---- BS_DEBUG -- : sendmsg ok, sendcnt = %d, seed = %d\n", sendcnt, seed);

                        //MT_ERR_FRONTEND(" ---- BS_DEBUG -- : process OK. ioctrl - FE_BLIND_SCAN_EVENT_CMD start, g_fe_fd[%d]==========\n", g_fe_fd);
                        ret = ioctl(g_fe_fd, FE_BLIND_SCAN_EVENT_CMD, &tuner_id);
                        //MT_ERR_FRONTEND(" ---- BS_DEBUG -- : process OK. ioctrl - FE_BLIND_SCAN_EVENT_CMD end ==========\n");
                        //printf("%s[%d] after MT_UNF_FE_BLINDSCAN_EVT_LOCKED\n", __FUNCTION__, __LINE__);

                        memset(&msg, 0, sizeof(msg));
                        iov[0].iov_base = (void *)nlh_rcv;
                        iov[0].iov_len = (size_t)len_rcv;
                        msg.msg_iov = iov;
                        msg.msg_iovlen = 1;

                    read_again:

                        //if (finish)
                        //return NULL;
                        fe_check_bs_status(tuner_id, &status);
                        if (status != 0)
                        {
                            enStatus = MT_UNF_FE_BLINDSCAN_STATUS_FINISH;
                            stop_quit = 1;
                            break;
                        }

                        FD_ZERO(&fds);
                        FD_SET(sock_fd, &fds);
                        tv.tv_sec = 0;
                        tv.tv_usec = 500 * 1000;
                        sel_ret = select(sock_fd + 1, &fds, NULL, NULL, &tv);
                        if (sel_ret < 0)
                        {
                            MT_ERR_FRONTEND("select failed, errno = %d, %s\n", errno, strerror(errno));
                        }

                        recvcnt = recvmsg(sock_fd, &msg, MSG_DONTWAIT);
                        if (recvcnt <= 0)
                        {
                            usleep(1000);
                            goto read_again;
                        }
                        //printf("recvcnt cs8k_sat = %d\n", recvcnt);

                        nlh = nlh_rcv;
                        while (NLMSG_OK(nlh, recvcnt))
                        {
                            if ((recvcnt - GENL_HDRLEN) >= (int)sizeof(struct nlmsghdr))
                                gehdr = NLMSG_DATA(nlh);
                            else
                            {
                                nlh = NLMSG_NEXT(nlh, recvcnt);
                                continue;
                            }
                            nla = (struct nlattr *)((void *)gehdr + GENL_HDRLEN);

                            for (pos = nla, rem = (int)(nlh->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN);
                                 rem >= NLA_HDRLEN && nla->nla_len >= NLA_HDRLEN && nla->nla_len <= rem;
                                 rem -= NLA_ALIGN(nla->nla_len), pos += NLA_ALIGN(nla->nla_len))
                            {
                                if (((struct nlattr *)pos)->nla_len > NLA_HDRLEN)
                                {
                                    if (((struct nlattr *)pos)->nla_type == DVBS_BS_NL_ATTR_DATA0)
                                    {
                                        int tmp_cnt = 0;

                                        fe_nl_data = (struct dvbs_bs_netlink_data *)(pos + NLA_HDRLEN);
                                        //printf("usr : pid = %d\n", fe_nl_data->pid);
                                        //printf("usr : seq = %d\n", fe_nl_data->seq);
                                        //printf("usr : cmd = %d\n", fe_nl_data->cmd);
                                        //printf("usr : data01 = %d\n", fe_nl_data->data01);
                                        //printf("usr : name : %s, x = %u, y = %u port_type = %u\n", fe_nl_data->name, fe_nl_data->freq_khz, fe_nl_data->symbol_rate, fe_nl_data->port_type);
#if 0
                                        if ((fe_nl_data->symbol_rate < 1500) || (fe_nl_data->freq_khz == 0))
                                        {
                                            unNotify.result = &stResult;

                                            bs_para.scan_para.sat.scan_notify(tuner_id, MT_UNF_FE_BLINDSCAN_EVT_NEWRESULT, &unNotify);

                                            nlh = NLMSG_NEXT(nlh, recvcnt);

                                            continue;
                                        }
#endif

                                        /* Convert IF(950-2150) to downlink frequency (C:3400-4200, Ku: 10600-12750) */
                                        /* unicable do not need. The freq offset of downlink is the same as the if freq.
                                          So we can substract the offset to the downlink freq directly. */
#if 0
                                        printf("lnb_band[%d] en_polar[%d] en_lnb_22k[%d] freq[%d]\n",
                                        g_sat_para[tuner_id].lnb_config.lnb_band,
                                        blindscan_ctrl.scan_cond[i].en_polar,
                                        blindscan_ctrl.scan_cond[i].en_lnb_22k,
                                        bs_info.scan_info->p_channel_info[last_num].frequency);
#endif
                                        fe_if_to_downlinkfreq(&(g_sat_para[tuner_id].lnb_config),
                                                              blindscan_ctrl.scan_cond[i].en_polar,
                                                              blindscan_ctrl.scan_cond[i].en_lnb_22k,
                                                              fe_nl_data->freq_khz, /*freq*/
                                                              &(stResult.freq));
                                        /* Symbol rate */
                                        stResult.symbol_rate = fe_nl_data->symbol_rate;
                                        /* Polarization */
                                        stResult.polar = blindscan_ctrl.scan_cond[i].en_polar;

                                        /* Multi-stream */
                                        stResult.DataTsNumber = fe_nl_data->DataTsNumber;
                                        stResult.ts_id = fe_nl_data->ts_id;
                                        stResult.ts_index = fe_nl_data->ts_index;
                                        for (tmp_cnt = 0; tmp_cnt < fe_nl_data->DataTsNumber; tmp_cnt++)
                                        {
                                            stResult.DataTsIdArray[tmp_cnt] = fe_nl_data->DataTsIdArray[tmp_cnt];
                                        }

                                        stResult.dvb_type = fe_nl_data->port_type;

                                        stResult.spectrum = bs_time; // 0: normal spectrum; 1: inverted spectrum

                                        if ((MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT == g_current_fe_attr[tuner_id].demod_dev_type) || 
                                            (MT_UNF_DEMOD_DEV_TYPE_M88RS6060 == g_current_fe_attr[tuner_id].demod_dev_type) || 
                                            (MT_UNF_DEMOD_DEV_TYPE_M88DS6103 == g_current_fe_attr[tuner_id].demod_dev_type) || 
                                            (MT_UNF_DEMOD_DEV_TYPE_M88DS6113 == g_current_fe_attr[tuner_id].demod_dev_type) || 
                                            (MT_UNF_DEMOD_DEV_TYPE_M88DM6K == g_current_fe_attr[tuner_id].demod_dev_type) || 
                                            (MT_UNF_DEMOD_DEV_TYPE_M88CT8K == g_current_fe_attr[tuner_id].demod_dev_type) || 
                                            (MT_UNF_DEMOD_DEV_TYPE_M88CS8800 == g_current_fe_attr[tuner_id].demod_dev_type))
                                        {
                                            if ((MT_UNF_PORT_TYPE_DVBS == stResult.dvb_type) || 
                                                (MT_UNF_PORT_TYPE_DVBS2 == stResult.dvb_type) || 
                                                (MT_UNF_PORT_TYPE_DIRECTV == stResult.dvb_type))
                                            { // Locked
                                                unNotify.result = &stResult;
                                                /* Notify new TP */
                                                //printf("usr dvbs thread line:%d Notify new TP.\n", __LINE__);
                                                bs_para.scan_para.sat.scan_notify(tuner_id, MT_UNF_FE_BLINDSCAN_EVT_NEWRESULT, &unNotify);
                                                /* Notify locked TP */
                                                //bs_para.scan_para.sat.scan_notify(tuner_id, MT_UNF_FE_BLINDSCAN_EVT_LOCKED, &unNotify);
                                                //printf("%s[%d] MT_UNF_FE_BLINDSCAN_EVT_LOCKED\n", __FUNCTION__, __LINE__);

                                                //all_freq = blindscan_ctrl.scan_cond[i].stop_freq - blindscan_ctrl.scan_cond[i].start_freq;
                                                all_freq = bs_para.scan_para.sat.stop_freq - bs_para.scan_para.sat.start_freq;
                                                //printf("usr dvbs thread line:%d all_freq=%d-----\n", __LINE__, all_freq);
                                                if (all_freq)
                                                {
                                                    //printf("usr dvbs thread line:%d if[%d] start_freq[%d]-----\n", __LINE__,
                                                    //bs_info.scan_info->p_channel_info[last_num].frequency,
                                                    //blindscan_ctrl.scan_cond[i].start_freq);
                                                    progress_percent = (mt_u16)((fe_nl_data->freq_khz - bs_para.scan_para.sat.start_freq) * 100 / all_freq);
                                                    if (progress_percent > 100)
                                                        progress_percent = 100;
                                                    if (unicable_scan_times == 2) // for 2-times Unicable blindscan
                                                    {
                                                        progress_percent /= 2;
                                                        if (bs_time > 0) // the 2nd time
                                                            progress_percent += 50;
                                                    }
                                                    //printf("usr dvbs thread line:%d progress_percent=%d-----\n", __LINE__, progress_percent);
                                                }

                                                if ((progress_percent > last_progress_percent) && (progress_percent < 100))
                                                {
                                                    /* Notify new status and percent */
                                                    //printf("usr dvbs thread line:%d progress_percent=%d-----\n", __LINE__, progress_percent);
                                                    unNotify.progress_percent = &progress_percent;
                                                    bs_para.scan_para.sat.scan_notify(tuner_id, MT_UNF_FE_BLINDSCAN_EVT_PROGRESS, &unNotify);
                                                    /* Record new percent */
                                                    last_progress_percent = progress_percent;
                                                }
                                            }
                                            else if (MT_UNF_PORT_TYPE_DVBS_BUTT == stResult.dvb_type)
                                            { // Unlock
                                                //printf("%s[%d] Unlock!\n", __FUNCTION__, __LINE__);

                                                unNotify.result = &stResult;
                                                /* Notify unlock TP */

                                                bs_para.scan_para.sat.scan_notify(tuner_id, MT_UNF_FE_BLINDSCAN_EVT_UNLOCK, &unNotify);
                                                //printf("%s[%d] MT_UNF_FE_BLINDSCAN_EVT_UNLOCK\n", __FUNCTION__, __LINE__);

                                                //ret = ioctl(g_fe_fd, FE_BLIND_SCAN_EVENT_CMD, &tuner_id);
                                                //printf("%s[%d] after MT_UNF_FE_BLINDSCAN_EVT_UNLOCK\n", __FUNCTION__, __LINE__);
                                            }
                                            else
                                            { // Find
                                                //printf("%s[%d] TpFind!\n", __FUNCTION__, __LINE__);

                                                all_freq = bs_para.scan_para.sat.stop_freq - bs_para.scan_para.sat.start_freq;
                                                if (all_freq)
                                                {
                                                    tpfind_percent = (mt_u16)((fe_nl_data->freq_khz - bs_para.scan_para.sat.start_freq) * 100 / all_freq);
                                                    if (tpfind_percent > 100)
                                                        tpfind_percent = 100;

                                                    if (unicable_scan_times == 2) // for 2-times Unicable blindscan
                                                    {
                                                        tpfind_percent /= 2;
                                                        if (bs_time > 0) // the 2nd time
                                                            tpfind_percent += 50;
                                                    }
                                                }

                                                if ((tpfind_percent > last_tpfind_percent) && (tpfind_percent < 100))
                                                {
                                                    /* Notify find new TP status and percent */
                                                    unNotify.progress_percent = &tpfind_percent;
                                                    printf("%s[%d] MT_UNF_FE_BLINDSCAN_EVT_FINDTP, %d%%\n", __FUNCTION__, __LINE__, tpfind_percent);
                                                    bs_para.scan_para.sat.scan_notify(tuner_id, MT_UNF_FE_BLINDSCAN_EVT_FINDTP, &unNotify);

                                                    /* Record find new TP percent */
                                                    last_tpfind_percent = tpfind_percent;
                                                }

                                                //ret = ioctl(g_fe_fd, FE_BLIND_SCAN_EVENT_CMD, &tuner_id);
                                                //printf("%s[%d] after MT_UNF_FE_BLINDSCAN_EVT_FINDTP\n", __FUNCTION__, __LINE__);

                                                //continue;
                                            }
                                        }
                                    }
                                }
                            }
                            nlh = NLMSG_NEXT(nlh, recvcnt);
                        }
                        /*
                        if (fe_nl_data->freq_khz > 1400 * 1000)
                        {
                            g_sat_para[tuner_id].blindscan_stop = MT_TRUE;
                        }
                        */
                    }
                }
            }
        }
#endif
    }

    if (stop_quit)
    {
        /* Notify new status and percent */
        printf("[%s %d]enStatus = %d\n", __FUNCTION__, __LINE__, enStatus);
        unNotify.status = &enStatus;
        bs_para.scan_para.sat.scan_notify(tuner_id, MT_UNF_FE_BLINDSCAN_EVT_STATUS, &unNotify);
        //break;
    }

    if (sock_fd >= 0)
    {
        printf("[%s %d]close socket\n", __FUNCTION__, __LINE__);
        close(sock_fd);
        sock_fd = -1;
    }

    if (nlh_snd != NULL)
    {
        mt_free(MT_ID_FRONTEND, nlh_snd);
        nlh_snd = NULL;
    }

    if (nlh_rcv != NULL)
    {
        mt_free(MT_ID_FRONTEND, nlh_rcv);
        nlh_rcv = NULL;
    }

    /*unNotify.ppstVerifyStart = &pstVerifyStart;
    bs_para.blindscan_ctrl.stSat.pfnEVTNotify(tuner_id, HI_UNF_TUNER_BLINDSCAN_EVT_VERIFY, &unNotify);
    pstVerifyStart = *(unNotify.ppstVerifyStart);*/
    /*if (g_sat_para[tuner_id].pBlindScanMonitor)
    {
        free(g_sat_para[tuner_id].pBlindScanMonitor);
        g_sat_para[tuner_id].pBlindScanMonitor = HI_NULL;
    }*/

    //mt_free(MT_ID_FRONTEND, bs_info.scan_info->p_channel_info);
    //bs_info.scan_info->p_channel_info = NULL;

    g_sat_para[tuner_id].blindscan_busy = MT_FALSE;
    g_sat_para[tuner_id].blindscan_stop = MT_FALSE;

    MT_INFO_FRONTEND("netlink_thread finished\n");
    //printf("[%s %d]leave\n", __FUNCTION__, __LINE__);

    return MT_NULL;
}
#endif

mt_void *fe_dvbt2_scan_thread(mt_void *p_ter_scan)
{
#if 1 // 180601
    mt_s32 ret;
    mt_u32 port = 0;

    //fe_channel_info_t *p_channel_info = (fe_channel_info_t *)p_ter_scan;

    //mt_unf_fe_ter_scan_para_t s_para;
    fe_terscan_info_t ter_scanInfo;
    //static HI_UNF_TUNER_TER_TPINFO_S terTP = {0};
    mt_unf_fe_ter_channel_attr_t stTpArray[TER_MAX_TP];
    mt_unf_fe_ter_plp_attr_t stPlpAttr;
    mt_unf_fe_ter_scan_status_t stScanStatus = MT_UNF_FE_TER_SCAN_STATUS_IDLE;
    mt_unf_fe_ter_scan_notify_t stScanNofify;
    mt_u32 u32Num, i, j, k, u32GrpNum = 0, u32Cnt = 0;
    mt_u8 u8CommExist = 0;

    typedef struct stT2MultiGroup
    {
        mt_u8 u8GrpId;
        mt_u8 u8CommId;
        mt_u8 u8Combination;
    } T2MultiGroup;
    T2MultiGroup T2PLPArray[T2_GROUP_SUM];

    if (MT_NULL == p_ter_scan)
    {
        MT_ERR_FRONTEND("Input parameter(p_ter_scan) invalid\n");
        //g_ter_para[port].blindscan_busy = MT_FALSE;
        return MT_NULL;
    }
    mt_set_pthread_name(__FUNCTION__);
    g_ter_para[port].bs_busy = MT_TRUE;
    ter_scanInfo.port = ((terscan_para_t *)p_ter_scan)->port;
    memcpy(&(ter_scanInfo.param.ter), &(((terscan_para_t *)p_ter_scan)->s_para.ter), sizeof(mt_unf_fe_ter_scan_attr_t));

    ret = ioctl(g_fe_fd, FE_TERSCAN_ACTION_CMD, &ter_scanInfo);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_FRONTEND("TUNER_TERSCAN_ACTION_CMD failed.\n");
        g_ter_para[port].bs_busy = MT_FALSE;
        return MT_NULL;
    }

    memset(stTpArray, 0, sizeof(stTpArray));
    if (ter_scanInfo.param.tpinfo.dvbt_mode) //dvb-t
    {
        stTpArray[0].freq = ter_scanInfo.param.ter.freq;
        stTpArray[0].band_width = ter_scanInfo.param.ter.band_width;
        stTpArray[0].dvbt_mode = 1;
        stTpArray[0].ts_pri = MT_UNF_FE_TS_PRIORITY_HP;
        ((terscan_para_t *)p_ter_scan)->s_para.chan_num = 1;

        if (!ter_scanInfo.param.tpinfo.dvbt_hier)
        {
            stTpArray[1].freq = ter_scanInfo.param.ter.freq;
            stTpArray[1].band_width = ter_scanInfo.param.ter.band_width;
            stTpArray[1].dvbt_mode = 1;
            stTpArray[1].ts_pri = MT_UNF_FE_TS_PRIORITY_LP;
            ((terscan_para_t *)p_ter_scan)->s_para.chan_num = 2;
        }
    }
    else //dvb-t2
    {
        u32Num = ter_scanInfo.param.tpinfo.progNum; // group id
        for (i = 0; i < u32Num - 1; i++)
        {
            for (j = 0; j < u32Num - 1 - i; j++)
            {
                if (ter_scanInfo.param.tpinfo.plp_attr[j].plp_grpid > ter_scanInfo.param.tpinfo.plp_attr[j + 1].plp_grpid)
                {
                    memcpy(&stPlpAttr, &ter_scanInfo.param.tpinfo.plp_attr[j], sizeof(mt_unf_fe_ter_plp_attr_t));
                    memcpy(&ter_scanInfo.param.tpinfo.plp_attr[j], &ter_scanInfo.param.tpinfo.plp_attr[j + 1], sizeof(mt_unf_fe_ter_plp_attr_t));
                    memcpy(&ter_scanInfo.param.tpinfo.plp_attr[j + 1], &stPlpAttr, sizeof(mt_unf_fe_ter_plp_attr_t));
                }
            }
        }

        memset(T2PLPArray, 0, sizeof(T2PLPArray));
        T2PLPArray[0].u8GrpId = ter_scanInfo.param.tpinfo.plp_attr[0].plp_grpid;
        for (i = 0, u32GrpNum = 0; i < u32Num; i++)
        {
            if (T2PLPArray[u32GrpNum].u8GrpId != ter_scanInfo.param.tpinfo.plp_attr[i].plp_grpid)
            {
                u32GrpNum++;
                if (u32GrpNum >= T2_GROUP_SUM)
                {
                    g_ter_para[port].bs_busy = MT_FALSE;
                    return MT_NULL;
                }
                T2PLPArray[u32GrpNum].u8GrpId = ter_scanInfo.param.tpinfo.plp_attr[i].plp_grpid;
                u32Cnt = 0;
                u8CommExist = 0;
            }

            if (MT_UNF_FE_T2_PLP_TYPE_COM == ter_scanInfo.param.tpinfo.plp_attr[i].plp_type)
            {
                T2PLPArray[u32GrpNum].u8CommId = ter_scanInfo.param.tpinfo.plp_attr[i].plp_id;
                u8CommExist = 1;
            }

            u32Cnt++;
            if ((u32Cnt >= 2) && u8CommExist)
            {
                T2PLPArray[u32GrpNum].u8Combination = 1;
            }
        }

        u32GrpNum = u32GrpNum + 1;

        for (i = 0, j = i, k = 0; i < u32Num; i++)
        {
            for (j = 0; j < u32GrpNum; j++)
            {
                if (ter_scanInfo.param.tpinfo.plp_attr[i].plp_grpid == T2PLPArray[j].u8GrpId)
                    break;
            }

            if (j >= T2_GROUP_SUM)
            {
                MT_ERR_FRONTEND("plp group number is more than 16.\n");
                g_ter_para[port].bs_busy = MT_FALSE;
                return MT_NULL;
            }

            if (MT_UNF_FE_T2_PLP_TYPE_COM == ter_scanInfo.param.tpinfo.plp_attr[i].plp_type)
            {
                continue;
            }

            stTpArray[k].freq = ter_scanInfo.param.ter.freq;
            stTpArray[k].band_width = ter_scanInfo.param.ter.band_width;
            stTpArray[k].dvbt_mode = 0;
            stTpArray[k].channel_mode = ter_scanInfo.param.tpinfo.channel_attr;
            stTpArray[k].plp_index = ter_scanInfo.param.tpinfo.plp_attr[i].plp_index;
            stTpArray[k].plp_id = ter_scanInfo.param.tpinfo.plp_attr[i].plp_id;
            stTpArray[k].com_id = T2PLPArray[j].u8CommId;
            stTpArray[k].combination = T2PLPArray[j].u8Combination;
            k++;
            if (k >= TER_MAX_TP)
                k = 0;
        }

        ((terscan_para_t *)p_ter_scan)->s_para.chan_num = k;
    }

    if (MT_NULL != ter_scanInfo.param.ter.scan_notify)
    {
        *(stScanNofify.progress_percent) = 100;
        if ((MT_NULL != stScanNofify.result) && (sizeof(stScanNofify.result) >= 16 * sizeof(mt_unf_fe_ter_channel_attr_t)))
        {
            memcpy(stScanNofify.result, stTpArray, 16 * sizeof(mt_unf_fe_ter_channel_attr_t));
        }
        ter_scanInfo.param.ter.scan_notify(ter_scanInfo.port, stScanStatus, &stScanNofify);
    }

    memcpy(((terscan_para_t *)p_ter_scan)->s_para.chan_array, stTpArray, sizeof(stTpArray));

    /*printf("\n-----------------------scan result-------------------------\n");
    if (ter_scanInfo.param.tpinfo.dvbt_mode)   //dvb-t
    {
        printf("signal:DVB-T\n");
        printf("program number:%d\n",ter_scanInfo.param.tpinfo.progNum);
    }
    else
    {
        printf("signal:DVB-T2\n");
        if (HI_UNF_TUNER_TER_MIXED_CHANNEL == ter_scanInfo.param.tpinfo.channel_mode)
        {
            printf("mixed channel,there are base and lite signals\n");
        }
        else
        {
            if (HI_UNF_TUNER_TER_MODE_BASE== ter_scanInfo.param.tpinfo.channel_attr)
                printf("pure channel,there is only base signal\n");
            else
                printf("pure channel,there is only lite signal\n");
        }

        printf("program number:%d\n",ter_scanInfo.param.tpinfo.progNum);

        printf("plp index		plp id		common plp id		combination\n");
        for (i = 0; i < k; i++)
        {
            printf("%d			%d			%d			%d\n",stTpArray[i].plp_index,stTpArray[i].plp_id,stTpArray[i].u8CommId,stTpArray[i].u8Combination);
        }
    }*/

    g_ter_para[port].bs_busy = MT_FALSE;
#endif

    return MT_NULL;
}

static mt_s32 fe_diseqc_send_22k(mt_u32 tuner_id, MT_BOOL b_status)
{
#if 0
    int ret = 0;
    fe_data_t onoff_22k = {0};

    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (g_fe_fd_cnt == 0)
        return MT_UNF_TUNER_ERR_UNOPEN;

    onoff_22k.port = tuner_id;
    onoff_22k.data = b_status ? 1 : 0;;
    ret = ioctl(g_fe_fd[tuner_id], FE_SET_22K_ONOFF_CMD, &onoff_22k);
    if (ret < 0)
        return MT_UNF_TUNER_ERR_SET_22KONOFF;

    return MT_SUCCESS;

#else

    fe_set_22k_onoff_t onoff_22k = {0};
    mt_s32 ret = MT_FAILURE;

    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        onoff_22k.tuner_id = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        onoff_22k.tuner_id = tuner_id;
    }

    onoff_22k.onoff = b_status ? 1 : 0;
    ret = ioctl(g_fe_fd, FE_SEND_CONTINUOUS_22K_CMD, &onoff_22k);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("set continuous 22K fail.\n");
        return MT_ERR_FE_FAILED_DISEQC;
    }

    return MT_SUCCESS;
#endif
}

static mt_s32 fe_diseqc_stop_22k(mt_u32 tuner_id)
{
    mt_s32 ret = MT_FAILURE;

    ret = fe_diseqc_send_22k(tuner_id, MT_FALSE);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Stop 22K fail.\n");
        return MT_ERR_FE_FAILED_SWITCH;
    }

    /* Save status */
    //g_sat_para[port].enSavedSwitch22K = g_sat_para[port].en_switch_22k;

    return MT_SUCCESS;
}

static mt_s32 fe_diseqc_resume_22k(mt_u32 tuner_id)
{
    /* Resume */
    if (MT_UNF_FE_SWITCH_22K_22 == g_sat_para[tuner_id].en_switch_22k /*enSavedSwitch22K*/)
    {
        return fe_diseqc_send_22k(tuner_id, MT_TRUE);
    }
    else
    {
        return MT_SUCCESS;
    }
}

mt_unf_fe_switch_toneburst_t fe_diseqc_get_toneburst_status(mt_u32 tuner_id)
{
    return g_sat_para[tuner_id].en_toneburst;
}

mt_s32 fe_diseqc_sendrecv_message(mt_u32 tuner_id,
                                  const mt_unf_fe_diseqc_sendmsg_t *p_sendmsg,
                                  mt_unf_fe_diseqc_recvmsg_t *p_recvmsg)
{
#if 1
    fe_diseqc_sendmsg_t diseqc_send_msg;
    fe_diseqc_recvmsg_t diseqc_recv_msg;
    fe_data_t fe_data = {0};
    mt_s32 ret = MT_FAILURE;
    mt_u32 repeat_times = 0;
    mt_u32 repeat_time = 0;
    MT_BOOL b_sendtone = MT_FALSE;

    CHECK_TUNER_OPEN();
    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_sendmsg)
    {
        MT_ERR_FRONTEND("Input parameter(p_sendmsg) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_sendmsg->level)
    {
        MT_ERR_FRONTEND("Input parameter(enLevel) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_SWITCH_TONEBURST_BUTT <= p_sendmsg->tone_burst)
    {
        MT_ERR_FRONTEND("Input parameter(en_toneburst) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_DISEQC_MSG_MAX_LENGTH < p_sendmsg->len)
    {
        MT_ERR_FRONTEND("Input parameter(u8Length) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_DISEQC_MAX_REPEAT_TIMES < p_sendmsg->repeat_times)
    {
        MT_ERR_FRONTEND("Input parameter(u8RepeatTimes) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    /* p_recvmsg can be NULL */

    /* Handle tone burst */
    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        fe_data.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        fe_data.port = tuner_id;
    }

    switch (p_sendmsg->tone_burst)
    {
    case MT_UNF_FE_SWITCH_TONEBURST_NONE:
        b_sendtone = MT_FALSE;
        break;

    case MT_UNF_FE_SWITCH_TONEBURST_0:
        b_sendtone = MT_TRUE;
        fe_data.data = 0;
        break;

    case MT_UNF_FE_SWITCH_TONEBURST_1:
        b_sendtone = MT_TRUE;
        fe_data.data = 1;
        break;
    default:
        MT_ERR_FRONTEND("Input parameter invalid!\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    /* Stop continuous 22K */
    (mt_void) fe_diseqc_stop_22k(tuner_id);
    usleep(DISEQC_DELAY_TIME_MS * 1000);

    /* Send command */

    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        diseqc_send_msg.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        diseqc_send_msg.port = tuner_id;
    }

    diseqc_send_msg.msg = *p_sendmsg;
    repeat_times = p_sendmsg->repeat_times;

    while (/*repeat_times >= 0*/ 1)
    //for (;;)
    {
        /* Handle repeat */
        if (repeat_time == 1)
        {
            diseqc_send_msg.msg.data[0]++;
        }

        /* Send command */
        ret = ioctl(g_fe_fd, FE_DISEQC_SEND_MSG_CMD, &diseqc_send_msg);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("Send DiSEqC message fail.\n");
            return MT_ERR_FE_FAILED_DISEQC;
        }

        /* After send command, delay 15ms */
        if (MT_UNF_FE_DISEQC_LEVEL_2_X != p_sendmsg->level)
            usleep(DISEQC_DELAY_TIME_MS * 1000);

        /* Send tone */
        if (b_sendtone)
        {
            ret = ioctl(g_fe_fd, FE_SEND_TONE_CMD, &fe_data);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("Send tone fail.\n");
                return MT_ERR_FE_FAILED_DISEQC;
            }

            /* After send tone, delay 15ms */
            usleep(DISEQC_DELAY_TIME_MS * 1000);
        }

        if (repeat_times == 0)
        {
            break;
        }

        repeat_times--;
        repeat_time++;
    }

    /* Recv message */
    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        diseqc_recv_msg.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        diseqc_recv_msg.port = tuner_id;
    }

    diseqc_recv_msg.msg = p_recvmsg;
    if (MT_NULL != p_recvmsg)
    {
        if (MT_UNF_FE_DISEQC_LEVEL_2_X == p_sendmsg->level)
        {
            ret = ioctl(g_fe_fd, FE_DISEQC_RECV_MSG_CMD, &diseqc_recv_msg);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("Recv DiSEqC message fail.\n");
                return MT_ERR_FE_FAILED_DISEQC;
            }

            usleep(DISEQC_DELAY_TIME_MS * 1000);
        }
        else
        {
            diseqc_recv_msg.msg->status = MT_UNF_FE_DISEQC_RECV_UNSUPPORT;
            diseqc_recv_msg.msg->len = 0;
        }
    }

    (mt_void) fe_diseqc_resume_22k(tuner_id);
#endif
    return MT_SUCCESS;
}

mt_s32 unicable_diseqc_sendrecv_message(mt_u32 tuner_id,
                                        const mt_unf_fe_diseqc_sendmsg_t *p_sendmsg,
                                        mt_unf_fe_diseqc_recvmsg_t *p_recvmsg)
{
    fe_diseqc_sendmsg_t diseqc_send_msg;
    fe_diseqc_recvmsg_t diseqc_recv_msg;
    fe_data_t fe_data = {0};
    fe_lnb_out_t lnb_out;
    mt_s32 ret = MT_FAILURE;
    mt_u32 repeat_times;
    mt_u32 repeat_time = 0;
    MT_BOOL b_sendtone = MT_FALSE;

    if (!b_fe_open)
    {
        MT_ERR_FRONTEND("tuner not opened, tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_NOT_OPEN;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_sendmsg)
    {
        MT_ERR_FRONTEND("Input parameter(p_sendmsg) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_DISEQC_LEVEL_BUTT <= p_sendmsg->level)
    {
        MT_ERR_FRONTEND("Input parameter(enLevel) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_SWITCH_TONEBURST_BUTT <= p_sendmsg->tone_burst)
    {
        MT_ERR_FRONTEND("Input parameter(en_toneburst) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_DISEQC_MSG_MAX_LENGTH < p_sendmsg->len)
    {
        MT_ERR_FRONTEND("Input parameter(u8Length) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_DISEQC_MAX_REPEAT_TIMES < p_sendmsg->repeat_times)
    {
        MT_ERR_FRONTEND("Input parameter(u8RepeatTimes) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    /* p_recvmsg can be NULL */

    /* Handle tone burst */
    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        fe_data.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        fe_data.port = tuner_id;
    }

    //fe_data.port = tuner_id;
    switch (p_sendmsg->tone_burst)
    {
    case MT_UNF_FE_SWITCH_TONEBURST_NONE:
        b_sendtone = MT_FALSE;
        break;

    case MT_UNF_FE_SWITCH_TONEBURST_0:
        b_sendtone = MT_TRUE;
        fe_data.data = 0;
        break;

    case MT_UNF_FE_SWITCH_TONEBURST_1:
        b_sendtone = MT_TRUE;
        fe_data.data = 1;
        break;
    default:
        MT_ERR_FRONTEND("Input parameter invalid!\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    /* Stop continuous 22K */
    (mt_void) fe_diseqc_stop_22k(tuner_id);
    usleep(DISEQC_DELAY_TIME_MS * 1000);

    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        lnb_out.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        lnb_out.port = tuner_id;
    }

    //lnb_out.port = tuner_id;
    lnb_out.voltage = TUNER_LNB_OUT_13V;
    ret = ioctl(g_fe_fd, FE_SET_LNBOUT_CMD, &lnb_out);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set LNB out 13V fail.\n");
        return MT_ERR_FE_FAILED_LNBCTRL;
    }

    //lnb_out.port = tuner_id;
    lnb_out.voltage = TUNER_LNB_OUT_18V;
    ret = ioctl(g_fe_fd, FE_SET_LNBOUT_CMD, &lnb_out);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set LNB out 18V fail.\n");
        return MT_ERR_FE_FAILED_LNBCTRL;
    }

    /* Send command */
    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        diseqc_send_msg.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        diseqc_send_msg.port = tuner_id;
    }

    //diseqc_send_msg.port = tuner_id;
    diseqc_send_msg.msg = *p_sendmsg;
    repeat_times = p_sendmsg->repeat_times;
    //while (/*repeat_times >= 0*/1)
    for (;;)
    {
        /* Handle repeat */
        if (repeat_time == 1)
        {
            diseqc_send_msg.msg.data[0]++;
        }

        /* Send command */
        ret = ioctl(g_fe_fd, FE_DISEQC_SEND_MSG_CMD, &diseqc_send_msg);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("Send DiSEqC message fail.\n");
            return MT_ERR_FE_FAILED_DISEQC;
        }

        (mt_void) fe_diseqc_stop_22k(tuner_id);
        /* After send command, delay 15ms */
        usleep(DISEQC_DELAY_TIME_MS * 1000);

        /* Send tone */
        if (b_sendtone)
        {
            ret = ioctl(g_fe_fd, FE_SEND_TONE_CMD, &fe_data);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("Send tone fail.\n");
                return MT_ERR_FE_FAILED_DISEQC;
            }

            /* After send tone, delay 15ms */
            usleep(DISEQC_DELAY_TIME_MS * 1000);
        }

        if (repeat_times == 0)
        {
            break;
        }

        repeat_times--;
        repeat_time++;
    }

    /* Recv message */
    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        diseqc_recv_msg.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        diseqc_recv_msg.port = tuner_id;
    }

    //diseqc_recv_msg.port = tuner_id;
    diseqc_recv_msg.msg = p_recvmsg;
    if (MT_NULL != p_recvmsg)
    {
        if (MT_UNF_FE_DISEQC_LEVEL_2_X == p_sendmsg->level)
        {
            ret = ioctl(g_fe_fd, FE_DISEQC_RECV_MSG_CMD, &diseqc_recv_msg);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("Recv DiSEqC message fail.\n");
                return MT_ERR_FE_FAILED_DISEQC;
            }

            usleep(DISEQC_DELAY_TIME_MS * 1000);
        }
        else
        {
            diseqc_recv_msg.msg->status = MT_UNF_FE_DISEQC_RECV_UNSUPPORT;
            diseqc_recv_msg.msg->len = 0;
        }
    }

    //lnb_out.port = tuner_id;
    lnb_out.voltage = TUNER_LNB_OUT_13V;
    ret = ioctl(g_fe_fd, FE_SET_LNBOUT_CMD, &lnb_out);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set LNB out 13V fail.\n");
        return MT_ERR_FE_FAILED_LNBCTRL;
    }

    (mt_void) fe_diseqc_resume_22k(tuner_id);

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_init(mt_void)
{
#if 0
    mt_u8 tuner_id;

    if (init_cnt > 0)
        return MT_SUCCESS;

    init_cnt ++;

    g_fe_fd_cnt = 0;
    g_fe_fd = 0;

    tuner_info[0].attr_set_cnt = 0;
    tuner_info[0].p_def_attr = &default_fe_attr[0];

    tuner_info[1].attr_set_cnt = 0;
    tuner_info[1].p_def_attr = &default_fe_attr[1];

    tuner_info[2].attr_set_cnt = 0;
    tuner_info[2].p_def_attr = &default_fe_attr[2];

    return MT_SUCCESS;

#else

    mt_u8 tuner_id;

    mt_sys_version_s stSysChipInfo;
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    mt_sys_get_version(&stSysChipInfo);

    if (b_fe_init)
    {
        return MT_SUCCESS;
    }
    MT_INFO_FRONTEND("%s[%d]=====================stSysChipInfo.enChipVersion = 0x%x\n", __FILE__, __LINE__, stSysChipInfo.enChipVersion);

    if (MT_CHIP_SYMPHONY2_A0 > stSysChipInfo.enChipVersion)
    {
        g_default_fe_attr[0].sig_type = MT_UNF_FE_SIG_TYPE_CAB;
        g_default_fe_attr[0].output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL;
        //g_default_fe_attr[0].reset_gpio_no = 0;/*need to check*/

        g_default_fe_attr[0].demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB;
        g_default_fe_attr[0].demod_addr = 0x38; //QAM_PORT0_ADDR;
        g_default_fe_attr[0].demod_i2c_id = 0;
        g_default_fe_attr[0].tuner_type = MT_UNF_TUNER_TYPE_M88TC3800;
        g_default_fe_attr[0].tuner_addr = 0xc2; //TUNER_PORT0_ADDR;
        g_default_fe_attr[0].tuner_i2c_id[0] = 0;

        g_default_fe_attr[1].sig_type = MT_UNF_FE_SIG_TYPE_SAT;
        g_default_fe_attr[1].output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL;

        g_default_fe_attr[1].demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT;
        g_default_fe_attr[1].demod_addr = 0xd0; //QAM_PORT2_ADDR;
        g_default_fe_attr[1].demod_i2c_id = 0;
        g_default_fe_attr[1].tuner_type = MT_UNF_TUNER_TYPE_M88TS2022;
        g_default_fe_attr[1].tuner_i2c_id[0] = 0;
        g_default_fe_attr[1].tuner_addr = 0xc0; //TUNER_PORT2_ADDR;
        g_default_fe_attr[0].fe_config.pin_config.diseqc_rx_mode = 0;
        g_default_fe_attr[0].fe_config.pin_config.diseqc_rx_gpio_pin = 0;
    }
    else if ((MT_CHIP_SYMPHONY4_A1 >= stSysChipInfo.enChipVersion) && (MT_CHIP_SYMPHONY4_A0 <= stSysChipInfo.enChipVersion))
    {
#if 1
        MT_INFO_FRONTEND("%s[%d] -- Sym4!\n", __FUNCTION__, __LINE__);

        g_default_fe_attr[0].sig_type = MT_UNF_FE_SIG_TYPE_CAB; // MT_UNF_FE_SIG_TYPE_J83B;
        g_default_fe_attr[0].output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL;

        g_default_fe_attr[0].demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8800;
        g_default_fe_attr[0].demod_addr = 0x38;
        g_default_fe_attr[0].demod_i2c_id = 0;
        g_default_fe_attr[0].tuner_type = MT_UNF_TUNER_TYPE_M88TS6011;
        g_default_fe_attr[0].tuner_addr = 0x58;
        g_default_fe_attr[0].tuner_i2c_id[0] = 0;
        g_default_fe_attr[0].fe_config.tun2_type = MT_UNF_TUNER_TYPE_M88TC6800;
        g_default_fe_attr[0].fe_config.tun2_addr = 0xC6;

        g_default_fe_attr[1].sig_type = MT_UNF_FE_SIG_TYPE_SAT;
        g_default_fe_attr[1].output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL;

        g_default_fe_attr[1].demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8800;
        g_default_fe_attr[1].demod_addr = 0xd0; //QAM_PORT2_ADDR;
        g_default_fe_attr[1].demod_i2c_id = 0;
        g_default_fe_attr[1].tuner_type = MT_UNF_TUNER_TYPE_M88TS6011;
        g_default_fe_attr[1].tuner_i2c_id[0] = 0;
        g_default_fe_attr[1].tuner_addr = 0x58; //TUNER_PORT2_ADDR;
        g_default_fe_attr[0].fe_config.tun2_type = MT_UNF_TUNER_TYPE_M88TC6800;
        g_default_fe_attr[0].fe_config.tun2_addr = 0xC6;
        g_default_fe_attr[1].fe_config.pin_config.diseqc_rx_mode = 0;
        g_default_fe_attr[1].fe_config.pin_config.diseqc_rx_gpio_pin = 0;
#else
        g_default_fe_attr[0].sig_type = MT_UNF_FE_SIG_TYPE_CAB;
        g_default_fe_attr[0].output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL;
        //g_default_fe_attr[0].reset_gpio_no = 0;/*need to check*/

        g_default_fe_attr[0].demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB;
        g_default_fe_attr[0].demod_addr = 0x38; //QAM_PORT0_ADDR;
        g_default_fe_attr[0].demod_i2c_id = 0;
        g_default_fe_attr[0].tuner_type = MT_UNF_TUNER_TYPE_M88TC3800;
        g_default_fe_attr[0].tuner_addr = 0xc2; //TUNER_PORT0_ADDR;
        g_default_fe_attr[0].tuner_i2c_id[0] = 0;

        g_default_fe_attr[1].sig_type = MT_UNF_FE_SIG_TYPE_SAT;
        g_default_fe_attr[1].output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL;

        g_default_fe_attr[1].demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT;
        g_default_fe_attr[1].demod_addr = 0xd0; //QAM_PORT2_ADDR;
        g_default_fe_attr[1].demod_i2c_id = 0;
        g_default_fe_attr[1].tuner_type = MT_UNF_TUNER_TYPE_M88TS2022;
        g_default_fe_attr[1].tuner_i2c_id[0] = 0;
        g_default_fe_attr[1].tuner_addr = 0xc0; //TUNER_PORT2_ADDR;
        g_default_fe_attr[1].fe_config.pin_config.diseqc_rx_mode = 0;
        g_default_fe_attr[1].fe_config.pin_config.diseqc_rx_gpio_pin = 0;
#endif
    }
    else if ((MT_CHIP_SYMPHONY6_MAX >= stSysChipInfo.enChipVersion) && (MT_CHIP_SYMPHONY6_A0 <= stSysChipInfo.enChipVersion))
    {
        //MT_INFO_FRONTEND("%s[%d] -- Sym6!\n", __FUNCTION__, __LINE__);
        MT_ERR_FRONTEND("%s[%d] -- Sym6!\n", __FUNCTION__, __LINE__);

        g_default_fe_attr[0].sig_type = MT_UNF_FE_SIG_TYPE_CAB; // MT_UNF_FE_SIG_TYPE_J83B;
        g_default_fe_attr[0].output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL;

        g_default_fe_attr[0].demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8800;
        g_default_fe_attr[0].demod_addr = 0x38;
        g_default_fe_attr[0].demod_i2c_id = 0;
        g_default_fe_attr[0].tuner_type = MT_UNF_TUNER_TYPE_M88TS6011;
        g_default_fe_attr[0].tuner_addr = 0x58;
        g_default_fe_attr[0].tuner_i2c_id[0] = 0;
        g_default_fe_attr[0].fe_config.tun2_type = MT_UNF_TUNER_TYPE_M88TC6800;
        g_default_fe_attr[0].fe_config.tun2_addr = 0xC6;

        g_default_fe_attr[1].sig_type = MT_UNF_FE_SIG_TYPE_SAT;
        g_default_fe_attr[1].output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL;

        g_default_fe_attr[1].demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8800;
        g_default_fe_attr[1].demod_addr = 0xd0; //QAM_PORT2_ADDR;
        g_default_fe_attr[1].demod_i2c_id = 0;
        g_default_fe_attr[1].tuner_type = MT_UNF_TUNER_TYPE_M88TS6011;
        g_default_fe_attr[1].tuner_i2c_id[0] = 0;
        g_default_fe_attr[1].tuner_addr = 0x58; //TUNER_PORT2_ADDR;
        g_default_fe_attr[0].fe_config.tun2_type = MT_UNF_TUNER_TYPE_M88TC6800;
        g_default_fe_attr[0].fe_config.tun2_addr = 0xC6;
        g_default_fe_attr[1].fe_config.pin_config.diseqc_rx_mode = 0;
        g_default_fe_attr[1].fe_config.pin_config.diseqc_rx_gpio_pin = 0;
    }
    else
    {
        g_default_fe_attr[0].sig_type = MT_UNF_FE_SIG_TYPE_CAB;
        g_default_fe_attr[0].output_mode = MT_UNF_FE_OUTPUT_MODE_PARALLEL;
        //g_default_fe_attr[0].reset_gpio_no = 0;/*need to check*/

        g_default_fe_attr[0].demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CT8K;
        g_default_fe_attr[0].demod_addr = 0x18; //QAM_PORT0_ADDR;
        g_default_fe_attr[0].demod_i2c_id = 0;
        g_default_fe_attr[0].tuner_type = MT_UNF_TUNER_TYPE_M88TS6011;
        g_default_fe_attr[0].tuner_addr = 0x58; //TUNER_PORT0_ADDR;
        g_default_fe_attr[0].tuner_i2c_id[0] = 0;
        g_default_fe_attr[0].fe_config.tun2_type = MT_UNF_TUNER_TYPE_M88TC6800;
        g_default_fe_attr[0].fe_config.tun2_addr = 0xc6;
        g_default_fe_attr[0].fe_config.pin_config.diseqc_rx_mode = 0;
        g_default_fe_attr[0].fe_config.pin_config.diseqc_rx_gpio_pin = 0;
    }

    for (tuner_id = 0; tuner_id < UNF_TUNER_NUM; tuner_id++)
    {
        g_current_fe_attr[tuner_id].demod_dev_type = MT_UNF_DEMOD_TYPE_BUTT;
        g_current_fe_attr[tuner_id].demod_i2c_id = 0;
        g_current_fe_attr[tuner_id].tuner_i2c_id[0] = 0;
        g_current_fe_attr[tuner_id].output_mode = MT_UNF_FE_OUTPUT_MODE_BUTT;
        g_current_fe_attr[tuner_id].sig_type = MT_UNF_FE_SIG_TYPE_BUTT;
        g_current_fe_attr[tuner_id].tuner_type = MT_UNF_TUNER_TYPE_BUTT;
    }

    /*need to check*/

    /* Default single frequency, C band SAT_LO_C_L */
    g_sat_para[0].p_bs_monitor = MT_NULL;
    //g_sat_para[0].pConnectMonitor   = MT_NULL;
    g_sat_para[0].blindscan_stop = MT_FALSE;
    g_sat_para[0].blindscan_busy = MT_FALSE;
    //g_sat_para[0].bConnectStop       = MT_FALSE;
    g_sat_para[0].lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_C;
    g_sat_para[0].lnb_config.lnb_type = MT_UNF_FE_LNB_SINGLE_FREQUENCY;
    g_sat_para[0].lnb_config.low_lo = SAT_LO_C_L;
    g_sat_para[0].lnb_config.high_lo = SAT_LO_C_L;
    g_sat_para[0].en_lnb_power = MT_UNF_FE_LNB_POWER_ON;
    g_sat_para[0].en_switch_22k = MT_UNF_FE_SWITCH_22K_NONE;
    //g_sat_para[0].enSavedSwitch22K = MT_UNF_FE_SWITCH_22K_NONE;
    g_sat_para[0].en_toneburst = MT_UNF_FE_SWITCH_TONEBURST_NONE;

    /*check end*/
    g_sat_para[1].p_bs_monitor = MT_NULL;
    //g_sat_para[1].pConnectMonitor   = MT_NULL;
    g_sat_para[1].blindscan_stop = MT_FALSE;
    //g_sat_para[1].bConnectStop       = MT_FALSE;
    g_sat_para[1].blindscan_busy = MT_FALSE;
    g_sat_para[1].lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_C;
    g_sat_para[1].lnb_config.lnb_type = MT_UNF_FE_LNB_SINGLE_FREQUENCY;
    g_sat_para[1].lnb_config.low_lo = SAT_LO_C_L;
    g_sat_para[1].lnb_config.high_lo = SAT_LO_C_L;
    g_sat_para[1].en_lnb_power = MT_UNF_FE_LNB_POWER_ON;
    g_sat_para[1].en_switch_22k = MT_UNF_FE_SWITCH_22K_NONE;
    //g_sat_para[1].enSavedSwitch22K = MT_UNF_FE_SWITCH_22K_NONE;
    g_sat_para[1].en_toneburst = MT_UNF_FE_SWITCH_TONEBURST_NONE;

    g_sat_para[2].p_bs_monitor = MT_NULL;
    //g_sat_para[2].pConnectMonitor   = MT_NULL;
    g_sat_para[2].blindscan_stop = MT_FALSE;
    //g_sat_para[2].bConnectStop      = MT_FALSE;
    g_sat_para[2].blindscan_busy = MT_FALSE;
    g_sat_para[2].lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_C;
    g_sat_para[2].lnb_config.lnb_type = MT_UNF_FE_LNB_SINGLE_FREQUENCY;
    g_sat_para[2].lnb_config.low_lo = SAT_LO_C_L;
    g_sat_para[2].lnb_config.high_lo = SAT_LO_C_L;
    g_sat_para[2].en_lnb_power = MT_UNF_FE_LNB_POWER_ON;
    g_sat_para[2].en_switch_22k = MT_UNF_FE_SWITCH_22K_NONE;
    //g_sat_para[2].enSavedSwitch22K = MT_UNF_FE_SWITCH_22K_NONE;
    g_sat_para[2].en_toneburst = MT_UNF_FE_SWITCH_TONEBURST_NONE;

#if 1 // 180601
    g_ter_para[0].p_bs_monitor = MT_NULL;
    g_ter_para[0].bs_stop = MT_FALSE;
    g_ter_para[0].bs_busy = MT_FALSE;
    g_ter_para[0].en_antenna_power = MT_UNF_FE_TER_ANTENNA_POWER_OFF;

    g_ter_para[1].p_bs_monitor = MT_NULL;
    g_ter_para[1].bs_stop = MT_FALSE;
    g_ter_para[1].bs_busy = MT_FALSE;
    g_ter_para[1].en_antenna_power = MT_UNF_FE_TER_ANTENNA_POWER_OFF;

    g_ter_para[2].p_bs_monitor = MT_NULL;
    g_ter_para[2].bs_stop = MT_FALSE;
    g_ter_para[2].bs_busy = MT_FALSE;
    g_ter_para[2].en_antenna_power = MT_UNF_FE_TER_ANTENNA_POWER_OFF;
#endif

    b_fe_init = MT_TRUE;

    return MT_SUCCESS;
#endif
}

mt_s32 mt_unf_fe_deinit(mt_void)
{
#if 0
    if (init_cnt == 0)
        return MT_SUCCESS;
    init_cnt --;

    if (g_fe_fd_cnt > 0)
    {
        mt_unf_fe_close(0);
        g_fe_fd_cnt = 0;
    }

    tuner_info[0].attr_set_cnt = 0;
    tuner_info[0].p_def_attr = NULL;

    tuner_info[1].attr_set_cnt = 0;
    tuner_info[1].p_def_attr = NULL;

    tuner_info[2].attr_set_cnt = 0;
    tuner_info[2].p_def_attr = NULL;

    return MT_SUCCESS;
#else

    b_fe_init = MT_FALSE;

    return MT_SUCCESS;
#endif
}

mt_s32 mt_unf_fe_get_default_attr(mt_u32 tuner_id, mt_unf_fe_attr_t *p_fe_attr)
{
#if 0
    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    memcpy(p_fe_attr, tuner_info[tuner_id].p_def_attr, sizeof(mt_unf_fe_attr_t));

    return MT_SUCCESS;

#else

    if (!b_fe_init)
    {
        MT_ERR_FRONTEND("FE: fe unf hasn't been Inited\n");
        return MT_ERR_FE_NOT_INIT;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_fe_attr)
    {
        MT_ERR_FRONTEND("Input parameter(p_fe_attr) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    memcpy(p_fe_attr, &g_default_fe_attr[tuner_id], sizeof(mt_unf_fe_attr_t));

    return MT_SUCCESS;
#endif
}

mt_s32 mt_unf_fe_set_attr(mt_u32 tuner_id, const mt_unf_fe_attr_t *p_fe_attr)
{
#if 0
    int ret = 0;
    fe_attr_t attr = {0};

    //printf("mt_unf_fe_set_attr line:%d.\n", __LINE__);

    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (g_fe_fd_cnt == 0)
        return MT_UNF_TUNER_ERR_UNOPEN;

    memcpy(&tuner_info[tuner_id].attr, p_fe_attr, sizeof(mt_unf_fe_attr_t));

    if (tuner_info[tuner_id].attr_set_cnt == 0)
        tuner_info[tuner_id].attr_set_cnt ++;

    //printf("mt_unf_fe_set_attr line:%d demod_dev_type %d.\n", __LINE__, p_fe_attr->demod_dev_type);

    attr.tuner_id = tuner_id;
    memcpy(&attr.attr, p_fe_attr, sizeof(mt_unf_fe_attr_t));

    ret = ioctl(g_fe_fd, FE_SET_ATTR_CMD, &attr);
    if (ret < 0)
    {
        printf("mt_unf_fe_set_attr ioctrl failed.\n");
        return ret;
    }
    else
        return MT_SUCCESS;

#else
    mt_s32 ret;
    mt_u32 i = 0;
    fe_attr_t attr = {0};
    //fe_data_t fe_data;
    //fe_databuf_t datbuf = {0};
    //fe_config_t fe_cfg = {0};

    //mt_tuner_attr_t tun_attr = {MT_UNF_TUNER_TYPE_BUTT, 0};
    //mt_demod_attr_t dmd_attr = {MT_UNF_DEMOD_TYPE_BUTT, 0};

    //printf("[%s %d]enter\n", __FUNCTION__, __LINE__);
    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id)invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_fe_attr)
    {
        MT_ERR_FRONTEND("Input parameter(p_fe_attr)invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    /* Modified begin: l00185424 2011-11-26 Support satellite */
    if (MT_UNF_FE_SIG_TYPE_BUTT <= p_fe_attr->sig_type)
    {
        MT_ERR_FRONTEND("Input parameter(p_fe_attr)invalid, sigType unsupported:%d\n", p_fe_attr->sig_type);
        return MT_ERR_FE_INVALID_PARA;
    }

    /* Check satellite tuner attribute */
    if (MT_UNF_FE_SIG_TYPE_SAT == p_fe_attr->sig_type)
    {
#if 0
        if (MT_UNF_FE_RFAGC_BUTT <= p_fe_attr->unTunerAttr.sat.enRFAGC)
        {
            MT_ERR_FRONTEND("Input parameter(enRFAGC)invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }
        if (MT_UNF_FE_IQSPECTRUM_BUTT <= p_fe_attr->unTunerAttr.sat.iq_spectrum)
        {
            MT_ERR_FRONTEND("Input parameter(iq_spectrum)invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }
        if (MT_UNF_FE_TSCLK_POLAR_BUTT <= p_fe_attr->unTunerAttr.sat.ts_clk_polar)
        {
            MT_ERR_FRONTEND("Input parameter(ts_clk_polar)invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }
        if (MT_UNF_FE_TS_FORMAT_BUTT <= p_fe_attr->unTunerAttr.sat.ts_format)
        {
            MT_ERR_FRONTEND("Input parameter(ts_format)invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }
        if (MT_UNF_FE_TS_SERIAL_PIN_BUTT <= p_fe_attr->unTunerAttr.sat.ts_serial_pin)
        {
            MT_ERR_FRONTEND("Input parameter(ts_serial_pin)invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }
        if (MT_UNF_FE_DISEQCWAVE_BUTT <= p_fe_attr->unTunerAttr.sat.diseqc_wave)
        {
            MT_ERR_FRONTEND("Input parameter(diseqc_wave)invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }
        if (MT_UNF_LNBCTRL_DEV_TYPE_BUTT <= p_fe_attr->unTunerAttr.sat.lnbctrl_dev)
        {
            MT_ERR_FRONTEND("Input parameter(lnbctrl_dev)invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }
#endif
    }

    /* Modified end: l00185424 2011-11-26 Support satellite */
    if (16 <= p_fe_attr->demod_i2c_id)
    {
        MT_ERR_FRONTEND("Input parameter(p_fe_attr->demod_i2c_id) invalid:%d\n", p_fe_attr->demod_i2c_id);
        return MT_ERR_FE_INVALID_PARA;
    }

    for (i = 0; i < MT_I2C_MAX_NUM_USER; i++)		
    {
        if (16 <= p_fe_attr->tuner_i2c_id[i])
        {
            MT_ERR_FRONTEND("Input parameter(p_fe_attr->tuner_i2c_id[%d]) invalid:%d\n", i, p_fe_attr->tuner_i2c_id[i]);
            return MT_ERR_FE_INVALID_PARA;
        }
    }

    if (MT_UNF_FE_OUTPUT_MODE_BUTT <= p_fe_attr->output_mode)
    {
        MT_ERR_FRONTEND("Input parameter(p_fe_attr->enOutputMode) invalid:%d\n", p_fe_attr->output_mode);
        return MT_ERR_FE_INVALID_PARA;
    }

    //memcpy(&tuner_info[tuner_id].attr, p_fe_attr, sizeof(mt_unf_fe_attr_t));

    //if (tuner_info[tuner_id].attr_set_cnt == 0)
    //tuner_info[tuner_id].attr_set_cnt ++;

    //printf("mt_unf_fe_set_attr line:%d demod_dev_type %d.\n", __LINE__, p_fe_attr->demod_dev_type);

    attr.tuner_id = tuner_id;
    memcpy(&attr.attr, p_fe_attr, sizeof(mt_unf_fe_attr_t));

    ret = ioctl(g_fe_fd, FE_SET_ATTR_CMD, &attr);
    if (ret < 0)
    {
        MT_ERR_FRONTEND("mt_unf_fe_set_attr ioctrl failed.\n");
        return ret;
    }
    //else
    //return MT_SUCCESS;

#if 0
    fe_data.data = (mt_u32)(p_fe_attr->demod_i2c_id);
    fe_data.port = tuner_id;
    ret = ioctl(g_fe_fd, FE_SELECT_I2C_CMD, (mt_u32)&fe_data);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner TUNER_SELECT_I2C_CMD error\n");
        return MT_ERR_FE_FAILED_SELECTI2CCHANNEL;
    }


    datbuf.port = tuner_id;
    tun_attr.tuner_type = p_fe_attr->tuner_type;
    tun_attr.tuner_addr = p_fe_attr->tuner_addr;

    dmd_attr.demod_type = p_fe_attr->demod_dev_type;
    dmd_attr.demod_addr = p_fe_attr->demod_addr;
    //dmd_attr.use_unicable = 0;//p_fe_attr->use_unicable = 0;

    fe_cfg.port = tuner_id;
    fe_cfg.lock_indicate = p_fe_attr->lock_indicate;
    fe_cfg.vsel_when_13v = p_fe_attr->vsel_when_13v;
    fe_cfg.vsel_when_lnb_off = p_fe_attr->vsel_when_lnb_off;
    fe_cfg.diseqc_out_when_lnb_off = p_fe_attr->diseqc_out_when_lnb_off;
    fe_cfg.lnb_enable = p_fe_attr->lnb_enable;
    fe_cfg.lnb_prot_level = p_fe_attr->lnb_prot_level;
    fe_cfg.lnb_enable_by_mcu = p_fe_attr->lnb_enable_by_mcu;
    fe_cfg.lnb_prot_by_mcu = p_fe_attr->lnb_prot_by_mcu;
    fe_cfg.lnb_enable_pin = p_fe_attr->lnb_enable_pin;
    fe_cfg.lnb_prot_pin = p_fe_attr->lnb_prot_pin;
    fe_cfg.lock_pin_by_mcu = p_fe_attr->lock_pin_by_mcu;
    fe_cfg.lock_pin = p_fe_attr->lock_pin;
    fe_cfg.lnb_vol_pin_by_mcu = p_fe_attr->lnb_vol_pin_by_mcu;
    fe_cfg.lnb_vol_pin = p_fe_attr->lnb_vol_pin;
    fe_cfg.demod_reset_pin = p_fe_attr->demod_reset_pin;
    fe_cfg.lnb_vol_pin_mode = p_fe_attr->lnb_vol_pin_mode;
    fe_cfg.reset_gpio_no = p_fe_attr->reset_gpio_no;
    fe_cfg.use_unicable = p_fe_attr->use_unicable;

    if (MT_UNF_TUNER_TYPE_BUTT <= tun_attr.tuner_type)
    {
        MT_ERR_FRONTEND("Input parameter(p_fe_attr->tuner_type) invalid:%d\n", tun_attr.tuner_type);
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_DEMOD_TYPE_BUTT <= dmd_attr.demod_type)
    {
        MT_ERR_FRONTEND("Input parameter(p_fe_attr->demod_dev_type) invalid:%d\n", dmd_attr.demod_type);
        return MT_ERR_FE_INVALID_PARA;
    }

    datbuf.databuf[0] = (mt_u32)&dmd_attr;
    datbuf.databuf[1] = (mt_u32)&tun_attr;

    /* Added begin: l00185424, for outer demod configure, 2012-01-19 */
    datbuf.databuf[2] = (mt_u32)&fe_cfg;//p_fe_attr->reset_gpio_no;
    /* Added end: l00185424, for outer demod configure, 2012-01-19 */
    ret = ioctl(g_fe_fd, FE_SELECT_TYPE_CMD, (mt_u32)&datbuf);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("FE: FE_SELECT_TYPE_CMD error\n");
        return ret;
    }

    fe_data.data = p_fe_attr->output_mode;
    fe_data.port = port;

    ret = ioctl(g_fe_fd, FE_SET_TSTYPE_CMD, (mt_u32)&fe_data);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("FE: mt_unf_fe_set_tstype error\n");
        return MT_FAILURE;
    }

    ret = ioctl(g_fe_fd, FE_SET_INIT_CFG_CMD, (mt_u32)&fe_cfg);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("FE: FE_SET_INIT_CFG_CMD error\n");
        return MT_FAILURE;
    }

#if 0
    fe_data.port = port;
    fe_data.data = (mt_u32)blind_scan_call_back_fun;
    ret = ioctl(g_fe_fd, FE_SET_REGISTER_NOTIFY_CMD, (mt_u32)&fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("FE: FE_SET_REGISTER_NOTIFY_CMD error\n");
        return MT_FAILURE;
    }
#endif

#endif

    memcpy(&g_current_fe_attr[tuner_id], p_fe_attr, sizeof(mt_unf_fe_attr_t));
    //printf("\n---unf--line:%d-------g_fe_fd=%d-------\n", __LINE__, g_fe_fd);

    return MT_SUCCESS;
#endif
}

mt_s32 mt_unf_fe_set_sat_attr(mt_u32 tuner_id, const mt_unf_fe_sat_attr_t *p_sat_fe_attr)
{
    //mt_s32 ret;
    //fe_data_t fe_data;

    CHECK_TUNER_OPEN();
    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port)invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_sat_fe_attr)
    {
        MT_ERR_FRONTEND("Input parameter(p_fe_attr)invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if ((MT_UNF_FE_SIG_TYPE_SAT != g_current_fe_attr[tuner_id].sig_type) ||
        (MT_UNF_FE_SIG_TYPE_SAT_2 != g_current_fe_attr[tuner_id].sig_type) ||
        ((MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2) != g_current_fe_attr[tuner_id].sig_type))
    {
        MT_ERR_FRONTEND("Current sig type is not satellite!\n");
        return MT_ERR_FE_INVALID_PARA;
    }

#if 0
    if (MT_UNF_FE_RFAGC_BUTT <= p_sat_fe_attr->en_rfagc)//enRFAGC
    {
        MT_ERR_FRONTEND("Input parameter(enRFAGC)invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }
    if (MT_UNF_FE_IQSPECTRUM_BUTT <= p_sat_fe_attr->iq_spectrum)
    {
        MT_ERR_FRONTEND("Input parameter(iq_spectrum)invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }
    if (MT_UNF_FE_TSCLK_POLAR_BUTT <= p_sat_fe_attr->ts_clk_polar)//ts_clk_polar
    {
        MT_ERR_FRONTEND("Input parameter(ts_clk_polar)invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }
    if (MT_UNF_FE_TS_FORMAT_BUTT <= p_sat_fe_attr->ts_format)//ts_format
    {
        MT_ERR_FRONTEND("Input parameter(ts_format)invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }
    if (MT_UNF_FE_TS_SERIAL_PIN_BUTT <= p_sat_fe_attr->ts_serial_pin)//ts_serial_pin
    {
        MT_ERR_FRONTEND("Input parameter(ts_serial_pin)invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }
    if (MT_UNF_FE_DISEQCWAVE_BUTT <= p_sat_fe_attr->diseqc_wave)//diseqc_wave
    {
        MT_ERR_FRONTEND("Input parameter(diseqc_wave)invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }
    if (MT_UNF_LNBCTRL_DEV_TYPE_BUTT <= p_sat_fe_attr->lnbctrl_dev)//lnbctrl_dev
    {
        MT_ERR_FRONTEND("Input parameter(lnbctrl_dev)invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }
    fe_data.port = tuner_id;
    fe_data.data = (mt_u32)p_sat_fe_attr;
    ret = ioctl(g_fe_fd, FE_SET_DEMODATTR_CMD, (mt_u32)&fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("FE: FE_SETDEMODATTR_CMD error\n");
        return MT_ERR_FE_FAILED_SETSATATTR;
    }
#endif

    //printf("\n---unf--line:%d-------g_fe_fd=%d-------\n", __LINE__, g_fe_fd);

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_attr(mt_u32 tuner_id, mt_unf_fe_attr_t *p_fe_attr)
{
#if 0
    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (tuner_info[tuner_id].attr_set_cnt == 0)
        return MT_UNF_TUNER_ERR_ATTR_UNSET;

    memcpy(p_fe_attr, &tuner_info[tuner_id].attr, sizeof(mt_unf_fe_attr_t));

    return MT_SUCCESS;

#else

    if (!b_fe_init)
    {
        MT_ERR_FRONTEND("tuner unf hasn't been inited\n");
        return MT_ERR_FE_NOT_INIT;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id)invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_fe_attr)
    {
        MT_ERR_FRONTEND("Input parameter(p_fe_attr)invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    memcpy(p_fe_attr, &g_current_fe_attr[tuner_id], sizeof(mt_unf_fe_attr_t));

    return MT_SUCCESS;
#endif
}

mt_s32 mt_unf_fe_open(mt_u32 tuner_id)
{
#if 0
    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (g_fe_fd_cnt > 0)
        return MT_SUCCESS;

    //printf("line:%d mt_unf_fe_open %d.\n", __LINE__, tuner_id);

    /*need check*/
    g_fe_fd = open("/dev/"MT_UNF_FE_DEV_NAME, O_RDWR);

    if (g_fe_fd < 0)
        return MT_UNF_TUNER_ERR_OPEN_FAIL;

    g_fe_fd_cnt ++;

    return MT_SUCCESS;

#else

    mt_s32 fe_fd = 0;

    if (!b_fe_init)
    {
        MT_ERR_FRONTEND("tuner unf hasn't been inited\n");
        return MT_ERR_FE_NOT_INIT;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (b_fe_open)
    {
        return MT_SUCCESS;
    }

    MT_FE_LOCK();

    /* frontend device node is /dev/mt_tuner */
    fe_fd = open("/dev/" MT_UNF_FE_DEV_NAME, O_RDWR | O_CLOEXEC, 0);
    if (fe_fd < 0)
    {
        MT_ERR_FRONTEND("open %s tuner failed\n", "/dev/" MT_UNF_FE_DEV_NAME);
        MT_FE_UNLOCK();
        return MT_ERR_FE_FAILED_INIT;
    }

    g_fe_fd = fe_fd;

    //printf("\n------------g_fe_fd = %d-------\n", g_fe_fd);

    b_fe_open = MT_TRUE;

    MT_FE_UNLOCK();

    return MT_SUCCESS;
#endif
}

mt_s32 mt_unf_fe_close(mt_u32 tuner_id)
{
#if 0
    int ret = 0;

    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (g_fe_fd_cnt == 0)
        return MT_SUCCESS;

    ret = close(g_fe_fd);
    if (ret < 0)
        return MT_UNF_TUNER_ERR_CLOSE_FAIL;

    g_fe_fd_cnt --;
    g_fe_fd = 0;

    return MT_SUCCESS;

#else

    if (!b_fe_init)
    {
        MT_ERR_FRONTEND("tuner unf hasn't been inited\n");
        return MT_ERR_FE_NOT_INIT;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (!b_fe_open)
    {
        return MT_SUCCESS;
    }

    MT_FE_LOCK();
    close(g_fe_fd);

    //printf("-------unf---line:%d close g_fe_fd=%d.-----\n", __LINE__, g_fe_fd);

    MT_FE_UNLOCK();
    g_fe_fd = 0;
    b_fe_open = MT_FALSE;
#endif

    return MT_SUCCESS;
}

#define FRAMING_BYTE (0)
#define ADDRESS_BYTE (1)
#define COMMAND_BYTE (2)
#define DATA_BYTE_0 (3)

#define FORMAT_DISEQC_CMD_VALUE(a, F, A, C, aD, L) \
    {                                              \
        int i;                                     \
        a[FRAMING_BYTE] = F;                       \
        a[ADDRESS_BYTE] = A;                       \
        a[COMMAND_BYTE] = C;                       \
        for (i = 0; i < L; i++)                    \
        {                                          \
            a[DATA_BYTE_0 + i] = ((mt_u8 *)aD)[i]; \
        }                                          \
    }

static mt_s32 unicable_chan_change(mt_u32 port, mt_u32 freq_mhz, mt_unf_fe_polar_t en_polar)
{
#if 1
    mt_u8 scr_no = 0;
    mt_u8 u8LNBNO = 0;
    mt_u16 u16Tune = 0;
    mt_u8 u8ChanByte[2] = {0};
    mt_unf_fe_diseqc_sendmsg_t diseqc_send_msgMsg;
    mt_s32 ret = 0;

    diseqc_send_msgMsg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    diseqc_send_msgMsg.tone_burst = fe_diseqc_get_toneburst_status(port);
    diseqc_send_msgMsg.len = 5;
    diseqc_send_msgMsg.repeat_times = 0;
    scr_no = g_sat_para[port].lnb_config.unicable_scr_no;
    MT_INFO_FRONTEND("unicable_chan_change tone_burst[%d] scr_no[%d]\r\n", diseqc_send_msgMsg.tone_burst, scr_no);

    /*low LO*/
    if (freq_mhz < SAT_DOWNLINK_FREQ_KU_MID)
    {
        u16Tune = (mt_u16)(((freq_mhz - g_sat_para[port].lnb_config.low_lo) + g_sat_para[port].lnb_config.unicable_if_freq_mhz) / 4 - 350);
        if (MT_UNF_FE_SATPOSN_A == g_sat_para[port].lnb_config.unicable_port_no)
        {
            if (MT_UNF_FE_POLARIZATION_V == en_polar)
            {
                u8LNBNO = 0;
            }
            else
            {
                u8LNBNO = 2;
            }
        }
        else
        {
            if (MT_UNF_FE_POLARIZATION_V == en_polar)
            {
                u8LNBNO = 4;
            }
            else
            {
                u8LNBNO = 6;
            }
        }
    }
    else /*high LO*/
    {
        u16Tune = (mt_u16)(((freq_mhz - g_sat_para[port].lnb_config.high_lo) + g_sat_para[port].lnb_config.unicable_if_freq_mhz) / 4 - 350);
        if (MT_UNF_FE_SATPOSN_A == g_sat_para[port].lnb_config.unicable_port_no)
        {
            if (MT_UNF_FE_POLARIZATION_V == en_polar)
            {
                u8LNBNO = 1;
            }
            else
            {
                u8LNBNO = 3;
            }
        }
        else
        {
            if (MT_UNF_FE_POLARIZATION_V == en_polar)
            {
                u8LNBNO = 5;
            }
            else
            {
                u8LNBNO = 7;
            }
        }
    }

    u8ChanByte[0] = (mt_u8)(((scr_no & 7) << 5) | ((u8LNBNO & 7) << 2) | ((u16Tune >> 8) & 3));
    u8ChanByte[1] = (mt_u8)(u16Tune & 0xFF);

    FORMAT_DISEQC_CMD_VALUE(diseqc_send_msgMsg.data, 0xE0, 0x10, 0x5A, u8ChanByte, 2);

    ret = unicable_diseqc_sendrecv_message(port, &diseqc_send_msgMsg, NULL /*&diseqc_recv_msgMsg*/);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Send WRITE N0 fail.\n");
        return ret;
    }

/* If support level 2.x, handle received message here. */
#endif
    return MT_SUCCESS;
}

mt_s32 unicable_power_off(mt_u32 port, mt_u8 scr_no)
{
    mt_unf_fe_diseqc_sendmsg_t diseqc_send_msgMsg;
    mt_s32 ret = 0;
    mt_u8 u8ChanByte[2] = {0, 0};

    if (scr_no >= 4)
    {
        MT_ERR_FRONTEND("SCR NO error.\n");
        return MT_FALSE;
    }

    diseqc_send_msgMsg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    diseqc_send_msgMsg.tone_burst = fe_diseqc_get_toneburst_status(port);
    diseqc_send_msgMsg.len = 5;
    diseqc_send_msgMsg.repeat_times = 0;

    u8ChanByte[0] = (mt_u8)((scr_no & 7) << 5);

    FORMAT_DISEQC_CMD_VALUE(diseqc_send_msgMsg.data, 0xE0, 0x10, 0x5A, u8ChanByte, 2);

    ret = unicable_diseqc_sendrecv_message(port, &diseqc_send_msgMsg, NULL /*&diseqc_recv_msgMsg*/);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Send WRITE N0 fail.\n");
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32 unicable_scrx_signal_on(mt_u32 tuner_id)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t diseqc_send_msgMsg;
    mt_s32 ret = 0;
    mt_u8 u8ChanByte[2] = {0, 0};
    mt_u8 u8SubFunc = 0;
    mt_u8 scr_no = 0;

    if (scr_no >= 4)
    {
        MT_ERR_FRONTEND("SCR NO error.\n");
        return MT_FALSE;
    }

    diseqc_send_msgMsg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    diseqc_send_msgMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
    diseqc_send_msgMsg.len = 5;
    diseqc_send_msgMsg.repeat_times = 0;

    /*u8ChanByte[0] = ((scr_no & 7) << 5) || ((u8SubFunc & 0x1f) << 0); Clean Warning [-Wint-in-bool-context]*/
    u8ChanByte[0] = ((scr_no & 7) << 5) | ((u8SubFunc & 0x1f) << 0);

    FORMAT_DISEQC_CMD_VALUE(diseqc_send_msgMsg.data, 0xE0, 0x10, 0x5B, u8ChanByte, 2);

    ret = unicable_diseqc_sendrecv_message(tuner_id, &diseqc_send_msgMsg, NULL /*&diseqc_recv_msgMsg*/);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Send SCRxSignal ON fail.\n");
        return ret;
    }
#endif
    return MT_SUCCESS;
}

#if 0
mt_s32 unicable_config(mt_u32 port,mt_u8 scr_no,mt_u8 app_no)
{
    mt_unf_fe_diseqc_sendmsg_t diseqc_send_msg;
    mt_s32 ret = 0;
    mt_u8 u8ChanByte[2] = {0,0};
    mt_u8 u8SubFunc=1;

    if (scr_no>=4)
    {
        MT_ERR_FRONTEND("SCR NO error.\n");
        return MT_FALSE;
    }

    diseqc_send_msg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    diseqc_send_msg.tone_burst = fe_diseqc_get_toneburst_status(port);
    diseqc_send_msg.len = 5;
    diseqc_send_msg.repeat_times = 0;

    u8ChanByte[0] = ((scr_no & 7) << 5) || ((u8SubFunc & 0x1f) << 0);
    u8ChanByte[1] = app_no;

    FORMAT_DISEQC_CMD_VALUE(diseqc_send_msgMsg.data, 0xE0, 0x10, 0x5B, u8ChanByte, 2);

    ret = unicable_diseqc_sendrecv_message(port, &diseqc_send_msgMsg, NULL/*&diseqc_recv_msgMsg*/);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Send unicable config fail.\n");
        return ret;
    }
    return MT_SUCCESS;
}
#endif

mt_s32 unicable_lofrq(mt_u32 tuner_id, mt_u8 scr_no, mt_u8 lofreq_no)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t diseqc_send_msg;
    mt_s32 ret = 0;
    mt_u8 u8ChanByte[2] = {0, 0};
    mt_u8 u8SubFunc = 2;

    if (scr_no >= 4)
    {
        MT_ERR_FRONTEND("SCR NO error.\n");
        return MT_FALSE;
    }

    diseqc_send_msg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    diseqc_send_msg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
    diseqc_send_msg.len = 5;
    diseqc_send_msg.repeat_times = 0;

    /*u8ChanByte[0] = ((scr_no & 7) << 5) || ((u8SubFunc & 0x1f) << 0);Clean Warning[-Wint-in-bool-context]*/
    u8ChanByte[0] = ((scr_no & 7) << 5) | ((u8SubFunc & 0x1f) << 0);
    u8ChanByte[1] = lofreq_no;

    FORMAT_DISEQC_CMD_VALUE(diseqc_send_msg.data, 0xE0, 0x10, 0x5B, u8ChanByte, 2);

    ret = unicable_diseqc_sendrecv_message(tuner_id, &diseqc_send_msg, NULL /*&diseqc_recv_msg*/);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Send unicable local oscillator number fail.\n");
        return ret;
    }
#endif
    return MT_SUCCESS;
}

#define IS_SIGNAL_SAT(sig_type) ((MT_UNF_FE_SIG_TYPE_SAT == sig_type) || (MT_UNF_FE_SIG_TYPE_SAT_2 == sig_type) || (MT_UNF_FE_SIG_TYPE_DVBS_AUTO == sig_type) || ((MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2) == sig_type))

#if 0
mt_s32 mt_unf_fe_get_default_timeout(mt_u32 tuner_id, mt_u32 *p_timeout)
{
    int ret = 0;
    fe_def_timeout_t i_timeout;

    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (g_fe_fd_cnt == 0)
        return MT_UNF_TUNER_ERR_UNOPEN;

    if (tuner_info[tuner_id].attr_set_cnt == 0)
        return MT_UNF_TUNER_ERR_ATTR_UNSET;

    i_timeout.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, TUNER_IOC_GET_DEFAULT_TIMEOUT, &i_timeout);
    if (ret < 0)
        return ret;

    *p_timeout = i_timeout.timeout;

    return MT_SUCCESS;
}
#else

mt_s32 mt_unf_fe_get_default_timeout(mt_u32 tuner_id, const mt_unf_fe_connect_para_t *p_conn_param, mt_u32 *p_timeout_ms)
{
    mt_u32 symb_rate_khz = 0;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tunerId is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_conn_param)
    {
        MT_ERR_FRONTEND("Input parameter(p_conn_param) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_NULL == p_timeout_ms)
    {
        MT_ERR_FRONTEND("Input parameter(p_conn_param) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if ((mt_s32)IS_SIGNAL_SAT(g_current_fe_attr[tuner_id].sig_type) != (mt_s32)IS_SIGNAL_SAT(p_conn_param->sig_type))
    {
        MT_ERR_FRONTEND("Current sigtype and connect type not match!\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_SIG_TYPE_CAB == p_conn_param->sig_type)
    {
        *p_timeout_ms = 1000;
    }
    /* Satellite */
    else if ((MT_UNF_FE_SIG_TYPE_SAT == p_conn_param->sig_type) ||
             (MT_UNF_FE_SIG_TYPE_SAT_2 == p_conn_param->sig_type) ||
             ((MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2) == p_conn_param->sig_type))
    {
        if (MT_UNF_FE_LNB_BAND_C == g_sat_para[tuner_id].lnb_config.lnb_band)
        {
            if ((SAT_C_MIN_KHZ > p_conn_param->connect_param.sat.freq) ||
                (SAT_C_MAX_KHZ < p_conn_param->connect_param.sat.freq))
            {
                MT_ERR_FRONTEND("Input parameter(freq) invalid: %d\n",
                                p_conn_param->connect_param.sat.freq);
                return MT_ERR_FE_INVALID_PARA;
            }
        }

        if (MT_UNF_FE_LNB_BAND_KU == g_sat_para[tuner_id].lnb_config.lnb_band)
        {
            if ((SAT_KU_MIN_KHZ > p_conn_param->connect_param.sat.freq) ||
                (SAT_KU_MAX_KHZ < p_conn_param->connect_param.sat.freq))
            {
                MT_ERR_FRONTEND("Input parameter(freq) invalid: %d\n",
                                p_conn_param->connect_param.sat.freq);
                return MT_ERR_FE_INVALID_PARA;
            }
        }

        if (SAT_SYMBOLRATE_MAX < p_conn_param->connect_param.sat.sym_rate)
        {
            MT_ERR_FRONTEND("Input parameter(sym_rate) invalid: %d\n",
                            p_conn_param->connect_param.sat.sym_rate);
            return MT_ERR_FE_INVALID_PARA;
        }

        if (MT_UNF_FE_POLARIZATION_BUTT <= p_conn_param->connect_param.sat.polarization)
        {
            MT_ERR_FRONTEND("Input parameter(en_polar) invalid: %d\n",
                            p_conn_param->connect_param.sat.polarization);
            return MT_ERR_FE_INVALID_PARA;
        }

        symb_rate_khz = p_conn_param->connect_param.sat.sym_rate / 1000;

        switch (g_current_fe_attr[tuner_id].demod_dev_type)
        {
        case MT_UNF_DEMOD_TYPE_3136:
        case MT_UNF_DEMOD_TYPE_3136I:
            if (2000 > symb_rate_khz)
            {
                *p_timeout_ms = 2000;
            }
            else if (3000 > symb_rate_khz)
            {
                *p_timeout_ms = 1500;
            }
            else if (4900 > symb_rate_khz)
            {
                *p_timeout_ms = 1400;
            }
            else if (8000 > symb_rate_khz)
            {
                *p_timeout_ms = 1200;
            }
            else if (15000 > symb_rate_khz)
            {
                *p_timeout_ms = 1150;
            }
            else if (20000 > symb_rate_khz)
            {
                *p_timeout_ms = 1900;
            }
            else
            {
                *p_timeout_ms = 600;
            }
            break;

        case MT_UNF_DEMOD_TYPE_AVL6211:
            if (5000 > symb_rate_khz)
            {
                *p_timeout_ms = 10000;
            }
            else if (10000 > symb_rate_khz)
            {
                *p_timeout_ms = 1200;
            }
            else
            {
                *p_timeout_ms = 1000;
            }
            break;

        default:
            MT_ERR_FRONTEND("Error demod type!\n");
            return MT_ERR_FE_INVALID_PARA;
        }
    }
    /* Terrestrial, Other */
    else if (MT_UNF_FE_SIG_TYPE_BUTT > p_conn_param->sig_type)
    {
        *p_timeout_ms = 1000;
    }
    else
    {
        MT_ERR_FRONTEND("Error signal type!\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    return MT_SUCCESS;
}
#endif

/*Clean Warning [-Wunused-function]
static mt_void *fe_ter_signal_detect(mt_void *p_signal)
{
#if 1 // 180601
    //fe_data_t fe_data;
    static fe_signal_t fe_signal;
    mt_s32 ret = MT_SUCCESS, timeout = 4000;
    mt_u32 timespan = 0;

    fe_connect_para_t connect_info;
    fe_status_t fe_state;

    if (MT_NULL == p_signal)
    {
        MT_ERR_FRONTEND("Input parameter(p_signal) invalid\n");
        return MT_NULL;
    }

    //memcpy(&fe_signal,p_signal,sizeof(fe_signal_t));        // 180615
    printf("+++++++++++++freq:%d\n", fe_signal.signal.freq);
    printf("+++++++++++++bandwidth:%d\n", fe_signal.signal.srbw.sym_rate);

    memcpy(&connect_info, p_signal, sizeof(fe_connect_para_t));

    while (timeout > 0)
    {
#if 0
        fe_signal.signal.dvbt_mode = 1;
        fe_signal.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;

        ret = ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&fe_signal);
#else
        //connect_info.para.sig_type = p_signal->para.sig_type;
        //connect_info.para.connect_param.ter.freq = p_signal->para.connect_param.ter.freq;
        //connect_info.para.connect_param.ter.band_width = p_signal->para.connect_param.ter.band_width;
        //connect_info.para.connect_param.ter.plp_id = p_signal->para.connect_param.ter.plp_id;
        ret = ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&connect_info);
#endif
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("dvb-t connect failed.\n");
        }
        else
        {
            //fe_data.data = 0;
            //fe_data.port = fe_signal.tuner_id;
            timespan = 0;
            timeout -= 500;
            while (timespan < 500)
            {
#if 0
                ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, (mt_u32)&fe_data);
                if (MT_SUCCESS != ret)
                {
                    MT_ERR_FRONTEND("dvb-t get lock failed.\n");
                }

                MT_ERR_FRONTEND("DVB-T get lock, fe_data.data = %d\n", fe_data.data);

                if (MT_UNF_FE_SIGNAL_LOCKED == fe_data.data)
                {
                    g_current_fe_connect_para[fe_signal.tuner_id].sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;
                    return MT_NULL;
                }
                else
                {
                    usleep(10 * 1000);
                    timespan += 10;
                }
#else
                fe_state.tuner_id = connect_info.tuner_id;
                ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, (mt_u32)&fe_state);
                if (MT_SUCCESS != ret)
                {
                    MT_ERR_FRONTEND("dvb-t get lock failed.\n");
                }

                if (MT_UNF_FE_SIGNAL_LOCKED == fe_state.status.lock_status)
                {
                    g_current_fe_connect_para[connect_info.tuner_id].sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;

                    MT_ERR_FRONTEND("----fe_ter_signal_detect(): DVB-T Locked! Lock state = %d\n", fe_state.status.lock_status);

                    return MT_NULL;
                }
                else
                {
                    usleep(10 * 1000);
                    timespan += 10;
                }
#endif
            }
        }

#if 0
        fe_signal.signal.dvbt_mode = 0;
        fe_signal.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;

        ret = ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&fe_signal);
#else
        connect_info.para.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;
        ret = ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&connect_info);
#endif

        if (MT_SUCCESS != ret)
        {
            return MT_NULL;
        }
        else
        {
            //fe_data.data = 0;
            //fe_data.port = fe_signal.tuner_id;
            timespan = 0;
            timeout -= 1000;
            while (timespan < 1000)
            {
#if 0
                ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, (mt_u32)&fe_data);
                if (MT_SUCCESS != ret)
                {
                    return MT_NULL;
                }

                MT_ERR_FRONTEND("DVB-T2 get lock, fe_data.data = %d\n", fe_data.data);

                if (MT_UNF_FE_SIGNAL_LOCKED == fe_data.data)
                {
                    g_current_fe_connect_para[fe_signal.tuner_id].sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;
                    return MT_NULL;
                }
                else
                {
                    usleep(10 * 1000);
                    timespan += 10;
                }
#else
                ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, (mt_u32)&fe_state);
                if (MT_SUCCESS != ret)
                {
                    return MT_NULL;
                }

                if (MT_UNF_FE_SIGNAL_LOCKED == fe_state.status.lock_status)
                {
                    g_current_fe_connect_para[connect_info.tuner_id].sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;

                    MT_ERR_FRONTEND("----fe_ter_signal_detect(): DVB-T2 Locked! Lock state = %d\n", fe_state.status.lock_status);

                    return MT_NULL;
                }
                else
                {
                    usleep(10 * 1000);
                    timespan += 10;
                }
#endif
            }
        }
    }
#endif

    return MT_NULL;
}
*/

mt_s32 mt_unf_fe_set_tuner_param(mt_u32 tuner_id, mt_unf_fe_tuner_param_t *p_tuner_param)
{
    mt_s32 ret = MT_SUCCESS;

    //mt_u32 enterTime;
    //mt_u32 curTime;
    //mt_u32 checkTime;

    fe_tn_param_t tuner_param;

    CHECK_TUNER_OPEN();
    //mt_sys_get_time_stamp_ms(&enterTime);

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_tuner_param)
    {
        MT_ERR_FRONTEND("Input parameter(p_tuner_param) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    tuner_param.tuner_id     = tuner_id;
    tuner_param.sig_type     = p_tuner_param->sig_type;
    tuner_param.freq_KHz     = p_tuner_param->freq_KHz;
    tuner_param.sym_rate_KSs = p_tuner_param->sym_rate_KSs;

    ret = ioctl(g_fe_fd, FE_SET_TUNER_PARAM_CMD, &tuner_param);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("mt_unf_fe_set_tuner_param[%d]: return error\n", __LINE__);
        return ret;
    }

    return 0;
}


mt_s32 mt_unf_fe_connect(mt_u32 tuner_id, mt_unf_fe_connect_para_t *p_conn_param, mt_u32 timeout)
{
    mt_s32 ret = MT_SUCCESS;
    //fe_data_t fe_data = {0}; // 180601
    fe_agc_qam_param_t tuner_attr = {0};
    //static fe_signal_t fe_signal = {0};
    //static pthread_t *pTerSignalDetect = MT_NULL; /* terr signal detect thread */ // 180601
    mt_s32 timespan = 0; // 180601
    //    fe_databuf_t stConnectTimeout = {0};
    mt_unf_fe_lnb_22k_t en_lnb_22k = {0};
    //    mt_u32 i = 0;
    mt_s32 s32TimeLeft = 0; // 180601
    //    fe_data_t stTmpTunerData = {0};
    mt_u8 u8ParamNOK = 0;
    //mt_s32 waitConnectTime = timeout;

    mt_u32 port = tuner_id;

    fe_connect_para_t connect_info = {0};
    mt_unf_fe_connect_para_t para_attr = {0};
    fe_def_timeout_t tmo;
    fe_status_t st;
    mt_u32 enterTime;
    mt_u32 curTime;
    mt_u32 checkTime;
    mt_u32 startConnectTime;

    mt_u32 snr = 0;

    //MT_ERR_FRONTEND("%s() %d: log 1\n", __FUNCTION__, __LINE__);

    CHECK_TUNER_OPEN();
    mt_sys_get_time_stamp_ms(&enterTime);

    //MT_ERR_FRONTEND("%s() %d: log 2\n", __FUNCTION__, __LINE__);

    //fe_data.data = 0;
    //fe_data.port = tuner_id;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_conn_param)
    {
        MT_ERR_FRONTEND("Input parameter(p_conn_param) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (((mt_s32)IS_SIGNAL_SAT(g_current_fe_attr[tuner_id].sig_type) != (mt_s32)IS_SIGNAL_SAT(p_conn_param->sig_type)) && 
        (MT_UNF_DEMOD_DEV_TYPE_M88DM6K != g_current_fe_attr[tuner_id].demod_dev_type) && 
        (MT_UNF_DEMOD_DEV_TYPE_M88RS6060 != g_current_fe_attr[tuner_id].demod_dev_type) && 
        (MT_UNF_DEMOD_DEV_TYPE_M88DS6103 != g_current_fe_attr[tuner_id].demod_dev_type) && 
        (MT_UNF_DEMOD_DEV_TYPE_M88DS6113 != g_current_fe_attr[tuner_id].demod_dev_type) && 
        (MT_UNF_DEMOD_DEV_TYPE_M88CT8K != g_current_fe_attr[tuner_id].demod_dev_type) && 
        (MT_UNF_DEMOD_DEV_TYPE_M88CS8800 != g_current_fe_attr[tuner_id].demod_dev_type))
    {
        MT_ERR_FRONTEND("%s[%d] -- Demod type[%d]. Current sig_type[%d] and connect_type[%d] not match!\n", __FUNCTION__, __LINE__, 
                         g_current_fe_attr[tuner_id].demod_dev_type, 
                         g_current_fe_attr[tuner_id].sig_type, p_conn_param->sig_type);

        return MT_ERR_FE_INVALID_POINT;
    }

    MT_INFO_FRONTEND("sig_type = %d\n", p_conn_param->sig_type);

    //fe_signal.sig_type = p_conn_param->sig_type;
    st.tuner_id = tuner_id;

    para_attr.connect_mode = p_conn_param->connect_mode;

    /* Cable */
    if ((MT_UNF_FE_SIG_TYPE_CAB == p_conn_param->sig_type) || (MT_UNF_FE_SIG_TYPE_J83B == p_conn_param->sig_type))
    {
        g_tuner_freq = (mt_s32)p_conn_param->connect_param.cab.freq;
#if 0
        g_current_fe_connect_para[tuner_id].connect_param.cab.freq = p_conn_param->connect_param.cab.freq;
        g_current_fe_connect_para[tuner_id].connect_param.cab.qam_type = p_conn_param->connect_param.cab.mod_type;//modulation type
        g_current_fe_connect_para[tuner_id].connect_param.cab.sym_rate = p_conn_param->connect_param.cab.sym_rate;//srbw
        g_current_fe_connect_para[tuner_id].connect_param.cab.band_width = p_conn_param->connect_param.cab.band_width;//srbw
        g_current_fe_connect_para[tuner_id].connect_param.cab.b_si = p_conn_param->connect_param.cab.b_reverse;//b_si
#endif
        para_attr.connect_param.cab.freq = p_conn_param->connect_param.cab.freq;
        para_attr.connect_param.cab.mod_type = p_conn_param->connect_param.cab.mod_type; //modulation type
        para_attr.connect_param.cab.sym_rate = p_conn_param->connect_param.cab.sym_rate;
        //tuner_attr.srbw.band_width = p_conn_param->connect_param.cab.band_width;//srbw

        para_attr.connect_param.cab.b_reverse = p_conn_param->connect_param.cab.b_reverse;

        para_attr.sig_type = p_conn_param->sig_type; // 181116

        //MT_INFO_FRONTEND("user conn freq[%d]\n", p_conn_param->connect_param.cab.freq);
        //MT_ERR_FRONTEND("user conn freq[%d], timeout = %d\n", p_conn_param->connect_param.cab.freq, timeout);
        //printf("user conn freq[%d], timeout = %d\n", p_conn_param->connect_param.cab.freq, timeout);

        if ((para_attr.connect_param.cab.freq < QAM_RF_MIN) || (para_attr.connect_param.cab.freq > QAM_RF_MAX))
        {
            MT_ERR_FRONTEND("Input parameter(pSignal.u32frequency) invalid freq = %d\n", tuner_attr.freq);
            return MT_ERR_FE_INVALID_PARA;
        }

        if ((para_attr.connect_param.cab.sym_rate < 900000) || (para_attr.connect_param.cab.sym_rate > 7200000))
        {
            if ((MT_UNF_DEMOD_DEV_TYPE_M88CT8K != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88CS8800 != g_current_fe_attr[tuner_id].demod_dev_type))
            {
                MT_ERR_FRONTEND("Input parameter(pSignal.sym_rate = %d) invalid, \n", para_attr.connect_param.cab.sym_rate);
                return MT_ERR_FE_INVALID_PARA;
            }
            else
            {
                MT_ERR_FRONTEND("Input parameter(pSignal.sym_rate = %d) invalid, activate auto QAM & symbol rate mode\n", para_attr.connect_param.cab.sym_rate);
                para_attr.connect_param.cab.sym_rate = 0;
            }
        }

#if 0
        switch (p_conn_param->connect_param.cab.mod_type)//mod_type
        {
            case MT_UNF_MOD_TYPE_QAM_16:
                para_attr.connect_param.cab.mod_type = QAM_TYPE_16;//qam_type
                break;

            case MT_UNF_MOD_TYPE_QAM_32:
                para_attr.connect_param.cab.mod_type = QAM_TYPE_32;
                break;

            case MT_UNF_MOD_TYPE_QAM_64:
            case MT_UNF_MOD_TYPE_DEFAULT:
                para_attr.connect_param.cab.mod_type = QAM_TYPE_64;
                break;

            case MT_UNF_MOD_TYPE_QAM_128:
                para_attr.connect_param.cab.mod_type = QAM_TYPE_128;
                break;

            case MT_UNF_MOD_TYPE_QAM_256:
                para_attr.connect_param.cab.mod_type = QAM_TYPE_256;
                break;
            default:
                MT_ERR_FRONTEND("Tuner mt_unf_set_eqamType error:%d\n", p_conn_param->connect_param.cab.mod_type);
                return MT_ERR_FE_INVALID_PARA;

        }
#endif
        /*
        g_current_fe_attr[tuner_id] = tuner_attr;
        fe_signal.port = tuner_id;
        fe_signal.signal = tuner_attr;
        memcpy(&g_current_fe_connect_para[port], p_conn_param, sizeof(mt_unf_fe_connect_para_t));
        stConnectTimeout.databuf[0] = timeout;
        stConnectTimeout.port = port;
        ret = ioctl(g_fe_fd, FE_CONNECT_TIMEOUT_CMD, (unsigned long)&stConnectTimeout);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("MT_UNF FE_CONNECT_TIMEOUT_CMD");
            return ret;
        }
        */

        g_current_fe_connect_para[tuner_id] = para_attr;
        connect_info.tuner_id = tuner_id;
        connect_info.para = para_attr;
        memcpy(&g_current_fe_connect_para[tuner_id], p_conn_param, sizeof(mt_unf_fe_connect_para_t));

        ret = ioctl(g_fe_fd, FE_CONNECT_CMD, &connect_info);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("mt_unf_fe_connect[%d]: return error\n", __LINE__);

            return ret;
        }

        //MT_ERR_FRONTEND("%s[%d] -- connect OK! Now get lock status, timeout = %d\n", __FUNCTION__, __LINE__, timeout);
        //printf("%s[%d] -- connect OK! Now get lock status, timeout = %d\n", __FUNCTION__, __LINE__, timeout);

        //FIXME: why memcpy to outside?
        memset((void *)p_conn_param, 0, sizeof(mt_unf_fe_connect_para_t));
        memcpy((void *)p_conn_param, &connect_info.para, sizeof(mt_unf_fe_connect_para_t));

        if (timeout != 0)
        {
            tmo.tuner_id = tuner_id;
            //ioctl(g_fe_fd, FE_GET_DEFAULT_TIMEOUT_CMD, &tmo);
            tmo.timeout = 200;
            usleep(tmo.timeout * 1000);
            st.tuner_id = tuner_id;
            ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);

            if (st.status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
            {
                return MT_SUCCESS;
            }

            if (timeout > tmo.timeout)
                timeout = timeout - tmo.timeout;
            else
                timeout = 0;

            while (timeout != 0)
            {
                usleep(10 * 1000);
                ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
                if (st.status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
                {
                    //printf("%s[%d] -- Locked, %3d\n", __FUNCTION__, __LINE__, timeout);
                    printf("%s[%d] -- %s Locked, %3d\n", __FUNCTION__, __LINE__, (MT_UNF_FE_SIG_TYPE_CAB == p_conn_param->sig_type) ? "DVB-C" : "J83B", timeout);

                    mt_unf_fe_get_snr(tuner_id, &snr);

                    return MT_SUCCESS;
                }

                if (timeout > 10)
                    timeout = timeout - 10;
                else
                    timeout = 0;

                //printf("%s[%d] -- Unlock, %3d\n", __FUNCTION__, __LINE__, timeout);
            }

            MT_ERR_FRONTEND("mt_unf_fe_connect[%d] -- Unlock or timeout\n", __LINE__);
            //printf("mt_unf_fe_connect[%d] -- Unlock or timeout\n", __LINE__);

            return MT_FAILURE;
        }

        return MT_SUCCESS;
#if 0
        if (0 == timeout)
        {
            return MT_SUCCESS;
        }

        while (timespan < timeout)
        {
            fe_data.port = port;
            ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, (mt_u32)&fe_data);
            if (MT_SUCCESS != ret)
            {
                return ret;
            }

            if (MT_UNF_FE_SIGNAL_LOCKED == fe_data.data)
            {
                return MT_SUCCESS;
            }
            else
            {
                usleep(10 * 1000);
                timespan += 10;
            }
        }
#endif
    }
    /* Satellite */
    else if ((MT_UNF_FE_SIG_TYPE_SAT == p_conn_param->sig_type) || 
             (MT_UNF_FE_SIG_TYPE_SAT_2 == p_conn_param->sig_type) || 
             (MT_UNF_FE_SIG_TYPE_DVBS_AUTO == p_conn_param->sig_type) || 
             (MT_UNF_FE_SIG_TYPE_DIRECTV == p_conn_param->sig_type) || 
             ((MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2) == p_conn_param->sig_type))
    {
        if ((MT_UNF_DEMOD_DEV_TYPE_M88DS6113 != g_current_fe_attr[tuner_id].demod_dev_type) && 
            (MT_UNF_FE_SIG_TYPE_DIRECTV == p_conn_param->sig_type))
        {
            MT_ERR_FRONTEND("Unsuppoorted signal type(DirecTV/DSS) for this demod!\n");

            return MT_ERR_FE_INVALID_PARA;
        }

        if (SAT_SYMBOLRATE_MAX < p_conn_param->connect_param.sat.sym_rate)
        {
            MT_ERR_FRONTEND("Input parameter(sym_rate) invalid: %d\n",
                            p_conn_param->connect_param.sat.sym_rate);
            u8ParamNOK = 1;
            //return MT_ERR_FE_INVALID_PARA;
        }

        if (MT_UNF_FE_POLARIZATION_BUTT <= p_conn_param->connect_param.sat.polarization)
        {
            MT_ERR_FRONTEND("Input parameter(en_polar) invalid: %d\n",
                            p_conn_param->connect_param.sat.polarization);
            u8ParamNOK = 1;
            //return MT_ERR_FE_INVALID_PARA;
        }

#if 0
        for (i = 0; i < 4; i++)
        {
            stTmpTunerData.port = port;
            stTmpTunerData.data = FunctMode_Demod;
            ret = ioctl(g_fe_fd, FE_SETFUNCMODE_CMD, &stTmpTunerData);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("TUNER_SETFUNCMODE_CMD error %d\n", ret);
            }
            else
            {
                break;
            }
        }
#endif

#if 0
        if (g_sat_para[port].pConnectMonitor)
        {
            //set the stop flag
            g_sat_para[port].bConnectStop = MT_TRUE;
            (mt_void)pthread_join(*g_sat_para[port].pConnectMonitor, MT_NULL);
            mt_free(MT_ID_FRONTEND, g_sat_para[port].pConnectMonitor);
            g_sat_para[port].pConnectMonitor = MT_NULL;
            g_sat_para[port].bConnectStop     = MT_FALSE;
        }
#endif

        /* Convert downlink frequency to IF */
        fe_downlinkfreq_to_if(&(g_sat_para[tuner_id].lnb_config), p_conn_param->connect_param.sat.polarization,
                              p_conn_param->connect_param.sat.freq, &(para_attr.connect_param.sat.freq), &en_lnb_22k);

        /*fix issue 133586 For DRV, we just need to make sure the intermediate frequency is between 950MHz and 2150MHz*/
        if ((para_attr.connect_param.sat.freq < SAT_IF_MIN_KHZ) ||(para_attr.connect_param.sat.freq > SAT_IF_MAX_KHZ))
        {
            MT_ERR_FRONTEND("Input parameter(freq) invalid: %d,if freq invalid :%d\n",
                             p_conn_param->connect_param.sat.freq,
                             para_attr.connect_param.sat.freq);
            u8ParamNOK = 1;
        }

        if (u8ParamNOK) /*when parameter is bad, use this default parameter.*/
        {
            /*
            fe_signal.port  = port;
            fe_signal.signal.freq = 900000;
            fe_signal.signal.srbw.sym_rate = 30000000;
            fe_signal.signal.polar = MT_UNF_FE_POLARIZATION_H;
            (mt_void)ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&fe_signal);
            */
            connect_info.tuner_id = tuner_id;
            connect_info.para.connect_param.sat.freq = 1400000;
            connect_info.para.connect_param.sat.sym_rate = 50000000;
            connect_info.para.connect_param.sat.onoff_22k = 0;
            connect_info.para.connect_param.sat.polarization = MT_UNF_FE_POLARIZATION_H;
            connect_info.para.connect_param.sat.lnb_status = p_conn_param->connect_param.sat.lnb_status;

            if (0 == p_conn_param->connect_param.sat.lnb_status)
            {
                /* LNB power and 22K signal switch */
                ret = fe_set_lnbout_and_22k(tuner_id, connect_info.para.connect_param.sat.polarization, 0);
                if (MT_SUCCESS != ret)
                {
                    MT_ERR_FRONTEND("fe_set_lnbout_and_22k fail.\n");
                }
            }

            ioctl(g_fe_fd, FE_CONNECT_CMD, &connect_info);
            return MT_ERR_FE_INVALID_PARA;
        }
        //printf("unf conn line[%d] freq[%d]\n",__LINE__, para_attr.connect_param.sat.freq);
        para_attr.connect_param.sat.sym_rate = p_conn_param->connect_param.sat.sym_rate;
        para_attr.connect_param.sat.polarization = p_conn_param->connect_param.sat.polarization;
        para_attr.connect_param.sat.port_type = p_conn_param->connect_param.sat.port_type;
        para_attr.connect_param.sat.onoff_22k = p_conn_param->connect_param.sat.onoff_22k;
        para_attr.connect_param.sat.ts_id = p_conn_param->connect_param.sat.ts_id;

        para_attr.connect_param.sat.AMC_index = p_conn_param->connect_param.sat.AMC_index;
        para_attr.connect_param.sat.AMC_offset = p_conn_param->connect_param.sat.AMC_offset;

        para_attr.connect_param.sat.lnb_status = p_conn_param->connect_param.sat.lnb_status;

        MT_DBG_FRONTEND("%s() %d: para_attr.sig_type = %d, p_conn_param->sig_type = %d\n", __FUNCTION__, __LINE__, para_attr.sig_type, p_conn_param->sig_type);

        para_attr.sig_type = p_conn_param->sig_type; // 181114

        if (0 == p_conn_param->connect_param.sat.lnb_status)
        {
            en_lnb_22k = p_conn_param->connect_param.sat.onoff_22k;

            //printf("[%s %d]tuner_id=%d, en_lnb_22k=%d\n", __FUNCTION__, __LINE__, tuner_id, en_lnb_22k);
            /* LNB power and 22K signal switch */
            ret = fe_set_lnbout_and_22k(tuner_id, p_conn_param->connect_param.sat.polarization, en_lnb_22k);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("fe_set_lnbout_and_22k fail.\n");
            }
        }

        if (MT_UNF_FE_LNB_UNICABLE == g_sat_para[tuner_id].lnb_config.lnb_type)
        {
            if ((MT_UNF_DEMOD_DEV_TYPE_M88DM6K != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88CT8K != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88RS6060 != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88DS6103 != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88DS6113 != g_current_fe_attr[tuner_id].demod_dev_type) && 
                (MT_UNF_DEMOD_DEV_TYPE_M88CS8800 != g_current_fe_attr[tuner_id].demod_dev_type))
            {
                ret = unicable_chan_change(tuner_id, p_conn_param->connect_param.sat.freq / 1000, p_conn_param->connect_param.sat.polarization);
                if (MT_SUCCESS != ret)
                {
                    MT_ERR_FRONTEND("Send ODU_ChannelChange cmd fail.\n");
                }
            }

            //p_conn_param->connect_param.sat.uc_param.use_uc = 1;
            //p_conn_param->connect_param.sat.uc_param.bank = g_sat_para[tuner_id].lnb_config.unicable_bank;
            //p_conn_param->connect_param.sat.uc_param.user_band = g_sat_para[tuner_id].lnb_config.unicable_scr_no;
            //p_conn_param->connect_param.sat.uc_param.ub_freq_mhz = g_sat_para[tuner_id].lnb_config.unicable_if_freq_mhz;
            para_attr.connect_param.sat.uc_param.use_uc = 1;
            para_attr.connect_param.sat.uc_param.bank = p_conn_param->connect_param.sat.uc_param.bank;
            para_attr.connect_param.sat.uc_param.user_band = p_conn_param->connect_param.sat.uc_param.user_band;
            para_attr.connect_param.sat.uc_param.ub_freq_mhz = p_conn_param->connect_param.sat.uc_param.ub_freq_mhz;
            para_attr.connect_param.sat.uc_param.ub_ver = p_conn_param->connect_param.sat.uc_param.ub_ver;
        }

#if 0
        s_stCurrentSignal[tuner_id] = tuner_attr;
        fe_signal.port  = tuner_id;
        fe_signal.signal = tuner_attr;
        memcpy(&g_current_fe_connect_para[tuner_id], p_conn_param, sizeof(mt_unf_fe_connect_para_t));

        g_sat_para[port].pConnectMonitor = (pthread_t*)mt_malloc(MT_ID_FRONTEND, sizeof(pthread_t));
        if (MT_NULL == g_sat_para[port].pConnectMonitor)
        {
            MT_ERR_FRONTEND("No memory.\n");
            return MT_ERR_FE_FAILED_CONNECT;
        }

        //fe_signal.bstopFlag = MT_FALSE;
        ret = pthread_create(g_sat_para[port].pConnectMonitor, 0, TUNER_ConnectThread, &fe_signal);

        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("Create pthread fail.\n");
            if (g_sat_para[port].pConnectMonitor)
            {
                mt_free(MT_ID_FRONTEND, g_sat_para[port].pConnectMonitor);
                g_sat_para[port].pConnectMonitor = MT_NULL;
                g_sat_para[port].bConnectStop = MT_FALSE;
            }

            return MT_ERR_FE_FAILED_CONNECT;
        }
#endif
        mt_sys_get_time_stamp_ms(&startConnectTime);
        //s_stCurrentSignal[tuner_id] = tuner_attr;
        g_current_fe_connect_para[tuner_id] = para_attr;
        connect_info.tuner_id = tuner_id;
        connect_info.para = para_attr;
        //memcpy(&connect_info.para, p_conn_param, sizeof(mt_unf_fe_connect_para_t));
        //if (MT_UNF_FE_LNB_UNICABLE == g_sat_para[tuner_id].lnb_config.lnb_type)
        //para.para.connect_param.sat.freq = tuner_attr.freq;
        memcpy(&g_current_fe_connect_para[tuner_id], p_conn_param, sizeof(mt_unf_fe_connect_para_t));

        //g_sat_para[tuner_id].lnb_config.unicable_bank = p_conn_param->connect_param.sat.uc_param.bank;

        //ret = ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&fe_signal);
        ret = ioctl(g_fe_fd, FE_CONNECT_CMD, &connect_info);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("mt_unf_fe_connect(): connect failed.\n");
            return ret;
        }

        MT_DBG_FRONTEND("%s[%d]: ret = %d, timeout = %d\n", __FUNCTION__, __LINE__, ret, timeout);

        //FIXME: why memcpy to outside?
        memset((void *)p_conn_param, 0, sizeof(mt_unf_fe_connect_para_t));
        memcpy((void *)p_conn_param, &connect_info.para, sizeof(mt_unf_fe_connect_para_t));

        fe_if_to_downlinkfreq(&(g_sat_para[tuner_id].lnb_config), p_conn_param->connect_param.sat.polarization, 
                              en_lnb_22k, p_conn_param->connect_param.sat.freq, &(p_conn_param->connect_param.sat.freq));


#if 0
        mt_sys_get_time_stamp_ms(&curTime);
        waitConnectTime -= curTime - startConnectTime;
        do
        {
            st.tuner_id = tuner_id;
            ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);

            if (st.status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
            {
                return MT_SUCCESS;
            }

            usleep(10 * 1000);
            waitConnectTime -= 10;
        } while (waitConnectTime > 0);
#endif

        if (timeout != 0)
        {
            //MT_ERR_FRONTEND("fe unf line %d.\n", __LINE__);

            //mt_sys_get_time_stamp_ms(&curTime);
            mt_sys_get_time_stamp_ms(&checkTime);
            s32TimeLeft = (mt_s32)connect_info.para.channel_set_info.lock_time; // - (curTime - startConnectTime);
            timespan = 0;
            while (timespan < s32TimeLeft)
            {
                //fe_data.port = port;
                mt_sys_get_time_stamp_ms(&curTime);
                //if (curTime - startConnectTime > connect_info.para.channel_set_info.lock_time)
                if (curTime - checkTime > connect_info.para.channel_set_info.lock_time)
                {
                    break;
                }

                ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
                if (MT_SUCCESS != ret)
                {
                    MT_ERR_FRONTEND("dvbs get lock failed. ret = %d\n", ret);
                }

                if (MT_UNF_FE_SIGNAL_LOCKED == st.status.lock_status)
                {
                    g_current_fe_connect_para[port].sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;

                    g_current_fe_connect_para[port].connect_param.sat.DataTsNumber = st.status.param.connect_param.sat.DataTsNumber;
                    g_current_fe_connect_para[port].connect_param.sat.ts_id = st.status.param.connect_param.sat.ts_id;
                    g_current_fe_connect_para[port].connect_param.sat.ts_index = st.status.param.connect_param.sat.ts_index;

                    memcpy(g_current_fe_connect_para[port].connect_param.sat.DataTsIdArray, 
                           st.status.param.connect_param.sat.DataTsIdArray, 
                           sizeof(st.status.param.connect_param.sat.DataTsIdArray) / sizeof(mt_u8));

                    p_conn_param->connect_param.sat.DataTsNumber = st.status.param.connect_param.sat.DataTsNumber;
                    p_conn_param->connect_param.sat.ts_id = st.status.param.connect_param.sat.ts_id;
                    p_conn_param->connect_param.sat.ts_index = st.status.param.connect_param.sat.ts_index;

                    memcpy(p_conn_param->connect_param.sat.DataTsIdArray, 
                           st.status.param.connect_param.sat.DataTsIdArray, 
                           sizeof(st.status.param.connect_param.sat.DataTsIdArray) / sizeof(mt_u8));

                    printf("%s[%d] ---- DVB-S or S2 Locked! Total %d TS\n", __FUNCTION__, __LINE__, 
                           st.status.param.connect_param.sat.DataTsNumber);

                    if (st.status.param.connect_param.sat.DataTsNumber > 1)
                    {
                        int i = 0;

                        for (i = 0; i < st.status.param.connect_param.sat.DataTsNumber; i++)
                            printf("\tTS[%d] ---- %d\n", i, st.status.param.connect_param.sat.DataTsIdArray[i]);
                    }

                    mt_unf_fe_get_snr(tuner_id, &snr);

                    return MT_SUCCESS;
                }
                /*else if (MT_UNF_FE_SIGNAL_DROPPED == st.status.lock_status)
                {
                    break;
                }*/
                else
                {
                    usleep(10 * 1000);
                    timespan += 10;
                }
            }

            return MT_FAILURE;
        }

        return MT_SUCCESS;
    }
    /* Terrestrial, Other */
    else if (((MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2) == p_conn_param->sig_type) || (MT_UNF_FE_SIG_TYPE_DVB_T == p_conn_param->sig_type) || (MT_UNF_FE_SIG_TYPE_DVB_T2 == p_conn_param->sig_type) || (MT_UNF_FE_SIG_TYPE_DVBT_AUTO == p_conn_param->sig_type))
    {
#if 1 // 180601
        s32TimeLeft = (mt_s32)timeout;

#if 0
        tuner_attr.freq  = p_conn_param->connect_param.ter.freq;
        tuner_attr.srbw.band_width = p_conn_param->connect_param.ter.band_width;
        tuner_attr.b_si = p_conn_param->connect_param.ter.b_reverse;
        tuner_attr.ter.dvbt2.channel_attr = p_conn_param->connect_param.ter.channel_mode;
        tuner_attr.ter.dvbt = p_conn_param->connect_param.ter.dvbt_prio;
#endif

        memcpy(&g_current_fe_connect_para[port], p_conn_param, sizeof(mt_unf_fe_connect_para_t));
        //g_current_fe_connect_para[port].sig_type = (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2);
        g_current_fe_connect_para[port].sig_type = p_conn_param->sig_type;

        para_attr.sig_type = p_conn_param->sig_type;
        para_attr.connect_param.ter.freq = p_conn_param->connect_param.ter.freq;
        para_attr.connect_param.ter.band_width = p_conn_param->connect_param.ter.band_width;
        para_attr.connect_param.ter.plp_id = p_conn_param->connect_param.ter.plp_id;
        para_attr.connect_param.ter.port_type = p_conn_param->connect_param.ter.port_type;
		/*fix issue30643*/
		para_attr.connect_param.ter.channel_mode = p_conn_param->connect_param.ter.channel_mode;
        connect_info.tuner_id = tuner_id;
        connect_info.para = para_attr;

        printf("----mt_unf_fe_connect(), tuner_id = %d, timeout = %d, sig_type = %d\n", tuner_id, timeout, g_current_fe_connect_para[port].sig_type);

        printf("----sig_type = %d, freq = %d, band_width = %d, plp_id = %d, port_type = %d,channel_mode = %d \n",
               connect_info.para.sig_type,
               connect_info.para.connect_param.ter.freq,
               connect_info.para.connect_param.ter.band_width,
               connect_info.para.connect_param.ter.plp_id,
               connect_info.para.connect_param.ter.port_type,
               connect_info.para.connect_param.ter.channel_mode);

        if (timeout == 0)
        {
#if 0
            fe_signal.tuner_id = port;
            fe_signal.signal = tuner_attr;

            if (MT_NULL != pTerSignalDetect)
            {
                (mt_void)pthread_join(*pTerSignalDetect, MT_NULL);
                mt_free(MT_ID_FRONTEND, pTerSignalDetect);
                pTerSignalDetect = MT_NULL;
            }

            pTerSignalDetect = (pthread_t*)mt_malloc(MT_ID_FRONTEND, sizeof(pthread_t));
            if (MT_NULL == pTerSignalDetect)
            {
                MT_ERR_FRONTEND("No memory.\n");
                return MT_FAILURE;
            }
            //ret = pthread_create(pTerSignalDetect, 0, fe_ter_signal_detect, &fe_signal);
            ret = pthread_create(pTerSignalDetect, 0, fe_ter_signal_detect, &connect_info);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("Create pthread fail.\n");
                if (pTerSignalDetect)
                {
                    mt_free(MT_ID_FRONTEND, pTerSignalDetect);
                    pTerSignalDetect = MT_NULL;
                }

                return MT_FAILURE;
            }
#else
            connect_info.para.sig_type = p_conn_param->sig_type;

            ret = ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&connect_info);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("dvb-t connect failed.\n");
            }
            else
            {
                //FIXME: why memcpy to outside?
                memset((void *)p_conn_param, 0, sizeof(mt_unf_fe_connect_para_t));
                memcpy((void *)p_conn_param, &connect_info.para, sizeof(mt_unf_fe_connect_para_t));

                return MT_SUCCESS;
            }
#endif
        }
        else
        {
            st.tuner_id = tuner_id;

            if (p_conn_param->sig_type == MT_UNF_FE_SIG_TYPE_DVBT_AUTO)
            {
                connect_info.para.sig_type = MT_UNF_FE_SIG_TYPE_DVBT_AUTO;
                connect_info.para.connect_param.ter.port_type = MT_UNF_PORT_TYPE_DVBT_AUTO;
                mt_sys_get_time_stamp_ms(&startConnectTime);

                ret = ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&connect_info);

                if (MT_SUCCESS != ret)
                {
                    MT_ERR_FRONTEND("dvbt_auto connect failed. ret = %d\n", ret);
                    //return MT_FAILURE;
                }
                else
                {
                    //fe_data.data = 0;
                    //fe_data.port = port;
                    mt_sys_get_time_stamp_ms(&checkTime);
                    s32TimeLeft = (mt_s32)(connect_info.para.channel_set_info.lock_time);// - (curTime - startConnectTime));
                    timespan = 0;
                    while (timespan < s32TimeLeft)
                    {
                        //fe_data.port = port;
                        mt_sys_get_time_stamp_ms(&curTime);
                        //if (curTime - startConnectTime > connect_info.para.channel_set_info.lock_time)
                        if (curTime - checkTime > connect_info.para.channel_set_info.lock_time)
                        {
                            break;
                        }

                        ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
                        if (MT_SUCCESS != ret)
                        {
                            MT_ERR_FRONTEND("dvbt_auto get lock failed. ret = %d\n", ret);
                        }

                        if (MT_UNF_FE_SIGNAL_LOCKED == st.status.lock_status)
                        {
                            g_current_fe_connect_para[port].sig_type = MT_UNF_FE_SIG_TYPE_DVBT_AUTO;
                            //printf("<<<<<<<<<<MT_SUCCESS:%s LINE = %d cost_time:%d\n", __func__, __LINE__,curTime-startConnectTime);
                            return MT_SUCCESS;
                        }
                        /*else if (HI_UNF_TUNER_SIGNAL_DROPPED == fe_data.data)
                        {
                            break;
                        }*/
                        else
                        {
                            usleep(10 * 1000);
                            timespan += 10;
                        }
                    }
                }
                //printf("<<<<<<<<<<MT_FAILURE:%s LINE = %d cost_time:%d\n", __func__, __LINE__,curTime-startConnectTime);
                return MT_FAILURE;
            }

            while (1)
            {
                mt_sys_get_time_stamp_ms(&curTime);
                if (curTime - enterTime > timeout)
                {
                    break;
                }

                if ((p_conn_param->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T) ||
                    (p_conn_param->sig_type == (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2)))
                {
                    connect_info.para.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;
                    connect_info.para.connect_param.ter.port_type = MT_UNF_PORT_TYPE_DVBT;
                    mt_sys_get_time_stamp_ms(&startConnectTime);

                    ret = ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&connect_info);

                    if (MT_SUCCESS != ret)
                    {
                        MT_ERR_FRONTEND("dvb-t connect failed.\n");
                    }
                    else
                    {
                        //fe_data.data = 0;
                        //fe_data.port = port;

                        mt_sys_get_time_stamp_ms(&curTime);
                        s32TimeLeft = (mt_s32)(connect_info.para.channel_set_info.lock_time - (curTime - startConnectTime));
                        timespan = 0;
                        while (timespan < s32TimeLeft)
                        {
                            //fe_data.port = port;
                            mt_sys_get_time_stamp_ms(&curTime);
                            if (curTime - startConnectTime > connect_info.para.channel_set_info.lock_time)
                            {
                                break;
                            }

                            ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
                            if (MT_SUCCESS != ret)
                            {
                                MT_ERR_FRONTEND("dvb-t get lock failed. ret = %d\n", ret);
                            }

                            if (MT_UNF_FE_SIGNAL_LOCKED == st.status.lock_status)
                            {
                                g_current_fe_connect_para[port].sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;
                                return MT_SUCCESS;
                            }
                            /*else if (HI_UNF_TUNER_SIGNAL_DROPPED == fe_data.data)
                            {
                                break;
                            }*/
                            else
                            {
                                usleep(10 * 1000);
                                timespan += 10;
                            }
                        }
                    }
                }

                if ((p_conn_param->sig_type == MT_UNF_FE_SIG_TYPE_DVB_T2) ||
                    (p_conn_param->sig_type == (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2)))
                {
                    connect_info.para.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;
                    connect_info.para.connect_param.ter.port_type = MT_UNF_PORT_TYPE_DVBT2;
                    mt_sys_get_time_stamp_ms(&startConnectTime);
                    ret = ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&connect_info);
                    if (MT_SUCCESS != ret)
                    {
                        mt_sys_get_time_stamp_ms(&curTime);
                        printf("<<<<<<<<<<%s LINE = %d ret = 0x%x costtime = %d\n", __func__, __LINE__, ret, curTime - enterTime);
                        return ret;
                    }
                    else
                    {
                        //fe_data.data = 0;
                        //fe_data.port = port;
                        mt_sys_get_time_stamp_ms(&curTime);
                        s32TimeLeft = (mt_s32)(connect_info.para.channel_set_info.lock_time - (curTime - startConnectTime));
                        timespan = 0;
                        while (timespan < s32TimeLeft)
                        {
                            mt_sys_get_time_stamp_ms(&curTime);
                            if (curTime - startConnectTime > connect_info.para.channel_set_info.lock_time)
                            {
                                break;
                            }

                            //fe_data.port = port;
                            ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
                            if (MT_SUCCESS != ret)
                            {
                                return ret;
                            }

                            if (MT_UNF_FE_SIGNAL_LOCKED == st.status.lock_status)
                            {
                                g_current_fe_connect_para[port].sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;
                                return MT_SUCCESS;
                            }
                            /*else if (HI_UNF_TUNER_SIGNAL_DROPPED == fe_data.data)
                            {
                                break;
                            }*/
                            else
                            {
                                usleep(10 * 1000);
                                timespan += 10;
                            }
                        }
                    }
                }
            }
        }
#endif
    }
    else if (MT_UNF_FE_SIG_TYPE_DTMB == p_conn_param->sig_type)
    {
        g_tuner_freq = p_conn_param->connect_param.ter.freq;

        //para_attr.connect_param.cab.freq = p_conn_param->connect_param.cab.freq;
        //para_attr.connect_param.cab.mod_type = p_conn_param->connect_param.cab.mod_type; //modulation type
        //para_attr.connect_param.cab.sym_rate = p_conn_param->connect_param.cab.sym_rate;

        para_attr.sig_type = p_conn_param->sig_type;
        para_attr.connect_param.ter.freq = p_conn_param->connect_param.ter.freq;
        para_attr.connect_param.ter.band_width = p_conn_param->connect_param.ter.band_width;
        para_attr.connect_param.ter.port_type = p_conn_param->connect_param.ter.port_type;

        MT_ERR_FRONTEND("user conn freq[%d]\n", p_conn_param->connect_param.ter.freq);

        if ((para_attr.connect_param.ter.freq < TER_RF_MIN) || (para_attr.connect_param.ter.freq > TER_RF_MAX))
        {
            MT_ERR_FRONTEND("Input parameter(pSignal.u32frequency) invalid freq = %d\n", tuner_attr.freq);
            return MT_ERR_FE_INVALID_PARA;
        }

        g_current_fe_connect_para[tuner_id] = para_attr;
        connect_info.tuner_id = tuner_id;
        connect_info.para = para_attr;
        memcpy(&g_current_fe_connect_para[tuner_id], p_conn_param, sizeof(mt_unf_fe_connect_para_t));

        ret = ioctl(g_fe_fd, FE_CONNECT_CMD, &connect_info);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("mt_unf_fe_connect: DTMB\n");
            return ret;
        }

        memset((void *)p_conn_param, 0, sizeof(mt_unf_fe_connect_para_t));
        memcpy((void *)p_conn_param, &connect_info.para, sizeof(mt_unf_fe_connect_para_t));

        if (timeout != 0)
        {
            tmo.tuner_id = tuner_id;
            //ioctl(g_fe_fd, FE_GET_DEFAULT_TIMEOUT_CMD, &tmo);
            tmo.timeout = 200;
            usleep(tmo.timeout * 1000);
            st.tuner_id = tuner_id;
            ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);

            if (st.status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
            {
                return MT_SUCCESS;
            }

            if (timeout > tmo.timeout)
                timeout = timeout - tmo.timeout;
            else
                timeout = 0;

            while (timeout != 0)
            {
                usleep(10 * 1000);
                ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
                if (st.status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
                {
                    return MT_SUCCESS;
                }

                if (timeout > 10)
                    timeout = timeout - 10;
                else
                    timeout = 0;
            }

            MT_ERR_FRONTEND("mt_unf_fe_connect OK.\n");

            return MT_FAILURE;
        }

        return MT_SUCCESS;
    }
    else if (MT_UNF_FE_SIG_TYPE_BUTT > p_conn_param->sig_type)
    {
#if 0
        if (MT_UNF_FE_SIG_TYPE_DVB_T == p_conn_param->sig_type)
        {
            tuner_attr.dvbt_mode = 1;
        }
        else
        {
            tuner_attr.dvbt_mode = 0;
        }

        tuner_attr.freq  = p_conn_param->connect_param.ter.freq;
        tuner_attr.srbw.band_width = p_conn_param->connect_param.ter.band_width;
        tuner_attr.b_si = p_conn_param->connect_param.ter.b_reverse;
        tuner_attr.ter.dvbt2.channel_attr = p_conn_param->connect_param.ter.channel_mode;
        tuner_attr.ter.dvbt = p_conn_param->connect_param.ter.dvbt_prio;

        switch (p_conn_param->connect_param.ter.mode_type)
        {
            case MT_UNF_MOD_TYPE_QAM_16:
                tuner_attr.qam_type = QAM_TYPE_16;
                break;
            case MT_UNF_MOD_TYPE_QAM_64:
                tuner_attr.qam_type = QAM_TYPE_64;
                break;
            default:
                tuner_attr.qam_type = QAM_TYPE_16;
                break;
        }

        if ((tuner_attr.freq < TER_RF_MIN) || (tuner_attr.freq > TER_RF_MAX))
        {
            MT_ERR_FRONTEND("Input parameter(pSignal.u32frequency) invalid freq = %d\n", tuner_attr.freq);
            return MT_ERR_FE_INVALID_PARA;
        }

        if ((tuner_attr.srbw.band_width < TER_BW_MIN) || (tuner_attr.srbw.band_width > TER_BW_MAX))
        {
            MT_ERR_FRONTEND("Input parameter(pSignal.band_width = %d) invalid, \n", tuner_attr.srbw.band_width);
            return MT_ERR_FE_INVALID_PARA;
        }

        s_stCurrentSignal[port] = tuner_attr;
        fe_signal.tuner_id = port;
        fe_signal.signal = tuner_attr;
        memcpy(&g_current_fe_connect_para[port], p_conn_param, sizeof(mt_unf_fe_connect_para_t));

        ret = ioctl(g_fe_fd, FE_CONNECT_CMD, (unsigned long)&fe_signal);
        if (MT_SUCCESS != ret)
        {
            return ret;
        }

        if (0 == timeout)
        {
            return MT_SUCCESS;
        }

        while (timespan < timeout)
        {
            fe_data.port = port;
            ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, (mt_u32)&fe_data);
            if (MT_SUCCESS != ret)
            {
                return ret;
            }

            if (MT_UNF_FE_SIGNAL_LOCKED == fe_data.data)
            {
                return MT_SUCCESS;
            }
            else
            {
                usleep(10 * 1000);
                timespan += 10;
            }
        }

        return MT_SUCCESS;
#endif
    }
    else
    {
        MT_ERR_FRONTEND("Error signal type!\n");
    }

    return MT_ERR_FE_FAILED_CONNECT;
}
mt_s32 mt_unf_fe_get_fast_lock(mt_u32 tuner_id, mt_unf_fe_status_t *p_status)
{
	mt_s32 ret = 0;
	fe_status_t st;

	CHECK_TUNER_OPEN();
	
    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_status)
    {
        MT_ERR_FRONTEND("Input parameter(p_status) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    st.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_FAST_LOCK_CMD, &st);

    if (MT_SUCCESS != ret)
    {
        return ret;
    }

	memcpy(p_status, &st.status, sizeof(mt_unf_fe_status_t));
	return MT_SUCCESS;
}
mt_s32 mt_unf_fe_get_status(mt_u32 tuner_id, mt_unf_fe_status_t *p_status)
{
#if 0
    int ret = 0;
    fe_status_t status;

    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (g_fe_fd_cnt == 0)
        return MT_UNF_TUNER_ERR_UNOPEN;

    if (tuner_info[tuner_id].attr_set_cnt == 0)
        return MT_UNF_TUNER_ERR_ATTR_UNSET;

    status.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &status);

    if (ret < 0)
        return MT_UNF_TUNER_ERR_GET_STATUS_FAIL;
    memcpy(p_status, &status.status, sizeof(mt_unf_fe_status_t));

    return MT_SUCCESS;

#else

    mt_s32 ret = 0;
    mt_u32 sig_type = MT_UNF_FE_SIG_TYPE_BUTT;/*Clean Warning [-Wswitch]*/
    fe_status_t st;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_status)
    {
        MT_ERR_FRONTEND("Input parameter(p_status) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    st.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);

    if (MT_SUCCESS != ret)
    {
        return ret;
    }

    memcpy(p_status, &st.status, sizeof(mt_unf_fe_status_t));

    if (MT_UNF_DEMOD_DEV_TYPE_M88DS6113 != g_current_fe_attr[tuner_id].demod_dev_type)
    {
        p_status->param.sig_type = g_current_fe_connect_para[tuner_id].sig_type;
    }

    sig_type = p_status->param.sig_type;

    switch (sig_type)
    {
    case MT_UNF_FE_SIG_TYPE_DVB_T:
    case MT_UNF_FE_SIG_TYPE_DVB_T2:
    case MT_UNF_FE_SIG_TYPE_ISDB_T:
    case MT_UNF_FE_SIG_TYPE_ATSC_T:
    case MT_UNF_FE_SIG_TYPE_DTMB:
    case MT_UNF_FE_SIG_TYPE_DVBT_AUTO:
    case (MT_UNF_FE_SIG_TYPE_DVB_T | MT_UNF_FE_SIG_TYPE_DVB_T2):
        p_status->param.sig_type = g_current_fe_connect_para[tuner_id].sig_type;
        p_status->param.connect_param.ter.freq = 
            g_current_fe_connect_para[tuner_id].connect_param.ter.freq;
        p_status->param.connect_param.ter.band_width = 
            g_current_fe_connect_para[tuner_id].connect_param.ter.band_width;
        break;

    case MT_UNF_FE_SIG_TYPE_SAT:
    case MT_UNF_FE_SIG_TYPE_SAT_2:
    case MT_UNF_FE_SIG_TYPE_DVBS_AUTO:
    case MT_UNF_FE_SIG_TYPE_DIRECTV:
    case MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2:
#if 1
        p_status->param.connect_param.sat.freq = 
            g_current_fe_connect_para[tuner_id].connect_param.sat.freq;
        p_status->param.connect_param.sat.sym_rate = 
            g_current_fe_connect_para[tuner_id].connect_param.sat.sym_rate;
        p_status->param.connect_param.sat.onoff_22k = 
            g_current_fe_connect_para[tuner_id].connect_param.sat.onoff_22k;
        p_status->param.connect_param.sat.polarization = 
            g_current_fe_connect_para[tuner_id].connect_param.sat.polarization;

        memcpy(&p_status->param.connect_param.sat.uc_param, 
               &g_current_fe_connect_para[tuner_id].connect_param.sat.uc_param, 
               sizeof(g_current_fe_connect_para[tuner_id].connect_param.sat.uc_param));

        p_status->param.connect_param.sat.lnb_status = 
            g_current_fe_connect_para[tuner_id].connect_param.sat.lnb_status;
#else
        memcpy(&p_status->param.connect_param, &g_current_fe_connect_para[tuner_id].connect_param, sizeof(g_current_fe_connect_para[tuner_id].connect_param));
#endif
        break;

    case MT_UNF_FE_SIG_TYPE_CAB:
    case MT_UNF_FE_SIG_TYPE_J83B:
    default:
        p_status->param.connect_param.cab.b_reverse =
            g_current_fe_connect_para[tuner_id].connect_param.cab.b_reverse;
        p_status->param.connect_param.cab.mod_type =
            g_current_fe_connect_para[tuner_id].connect_param.cab.mod_type;
        p_status->param.connect_param.cab.freq =
            g_current_fe_connect_para[tuner_id].connect_param.cab.freq;
        p_status->param.connect_param.cab.sym_rate =
            g_current_fe_connect_para[tuner_id].connect_param.cab.sym_rate;
        break;
    }

    return MT_SUCCESS;
#endif
}

mt_s32 mt_unf_fe_get_ber(mt_u32 tuner_id, mt_u32 *p_ber)
{
#if 0
    int ret = 0;
    fe_ber_t ber;

    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (g_fe_fd_cnt == 0)
        return MT_UNF_TUNER_ERR_UNOPEN;

    if (tuner_info[tuner_id].attr_set_cnt == 0)
        return MT_UNF_TUNER_ERR_ATTR_UNSET;

    ber.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_BER_CMD, &ber);
    if (ret < 0)
        return MT_UNF_TUNER_ERR_GET_BER_FAIL;

    memcpy(p_ber, ber.ber, sizeof(mt_u32) * 3);

    return MT_SUCCESS;

#else
    mt_s32 ret;
    /*mt_double d_ber = 0;Clean Warning [-Wunused-variable]*/
    //    mt_s32 i = 0;
    /*mt_u32 u32ErrSum; Clean Warning [-Wunused-variable]*/
    //fe_databuf_t fe_databuf;
    //fe_data_t fe_data;
    fe_status_t st;
    fe_ber_t ber_para;
    /*fe_signal_info_t fe_signal_info;Clean Warning [-Wunused-variable]*/
#if 0
    fe_data.port= port;
    fe_data.data = 0;

    fe_databuf.port= port;
    memset(fe_databuf.databuf, 0, sizeof(fe_databuf.databuf));
#endif

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_ber)
    {
        MT_ERR_FRONTEND("Input parameter(p_ber) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    st.tuner_id = tuner_id;
    /*p_ber points to the first address of three unsigned numbers which have BER*/
    /*CNcomment:p_ber */
    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }

    ber_para.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_BER_CMD, &ber_para);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner mt_unf_fe_get_ber error\n");
        return ret;
    }

#if 1 //20201104
    {
        mt_u32 error_packets = ber_para.ber[1], total_packets = ber_para.ber[0];
        mt_u8 j = 0;

        p_ber[0] = 0; // integer part
        p_ber[1] = 0; // decimal part, 3-bit valid digits
        p_ber[2] = 0; // exponent part, absolute value, negative value

        if ((error_packets == 0) || (total_packets == 0))
        {
            p_ber[0] = 0;
            p_ber[1] = 0;
            p_ber[2] = 0;
        }
        else
        {
            while (error_packets < total_packets)
            {
                error_packets *= 10;

                p_ber[2]++;
            }

            p_ber[0] = error_packets / total_packets;

            j = 0;

            do
            {
                p_ber[1] *= 10;
                error_packets %= total_packets;
                error_packets *= 10;
                p_ber[1] += error_packets / total_packets;
                j++;
            } while (j < 3);
        }

        //printf("%s[%d] ---- BER = %d.%03dE-%d\n", __FUNCTION__, __LINE__, p_ber[0], p_ber[1], p_ber[2]);
    }

    return ret;
#else
//*p_ber = ber_para.ber;

//printf("unf get ber line[%d] sig_type[%d]\n", __LINE__, g_current_fe_connect_para[tuner_id].sig_type);
#if 1
    /* Modified begin: l00185424 2011-11-26 Support satellite */
    /* If the signal type is cable, convert the data here. */
    if (MT_UNF_FE_SIG_TYPE_CAB == g_current_fe_connect_para[tuner_id].sig_type)
    {
        //*p_ber = ber_para.ber[0];
        if (ber_para.ber[0] != 0)
        {
            d_ber = (mt_double)ber_para.ber[1] / (mt_double)ber_para.ber[0];
            *p_ber = (mt_u32)d_ber;
            //printf("d_ber[%f]\n", d_ber);
        }
        else
            *p_ber = 0;

#if 0
        switch (g_current_fe_attr[tuner_id].demod_dev_type)
        {
         case MT_UNF_DEMOD_TYPE_NONE:
            break;
         case MT_UNF_DEMOD_TYPE_DS3103:
         case MT_UNF_DEMOD_TYPE_BUTT:
         case MT_UNF_DEMOD_TYPE_J83B:
         {
            /* conver to value */
            d_ber = ((mt_double)(((ber_para.ber[0] & 0xFF) << 16) + ((ber_para.ber[1] & 0xFF) << 8)
                    + (ber_para.ber[2] & 0xFF)) / 8388608.0);
            i = 0;
            if (d_ber != 0)
            {
                while (d_ber < 1)
                {
                    d_ber *= 10;
                    i++;
                }
            }

            /*get the integer and exponent part by Scientific notation, keep three effective digits*/
            /*CNcomment:*/
            p_ber[0] = (mt_u32)d_ber;
            p_ber[1] = ((mt_u32)(d_ber * 1000)) % 1000;
            p_ber[2] = (mt_u32)i;
            break;
        }
        case MT_UNF_DEMOD_TYPE_MXL254:
        case MT_UNF_DEMOD_TYPE_MXL214:
        {
            d_ber = (mt_double)(ber_para.ber[0]);
            i = 0;
            if (d_ber != 0)
            {
                while (d_ber >= 1)
                {
                    d_ber = d_ber /10;
                    i++;
                }
            }

            /*get the integer and exponent part by Scientific notation, keep three effective digits*/
            /*CNcomment:*/
            p_ber[0] = (mt_u32)d_ber;
            p_ber[1] = ((mt_u32)(d_ber * 1000)) % 1000;
            p_ber[2] = (mt_u32)(7 - i);

            break;
        }
        case MT_UNF_DEMOD_TYPE_TDA18280:
        {
            p_ber[0] = (mt_u32)ber_para.ber[0];
            p_ber[1] = (mt_u32)ber_para.ber[1];
            p_ber[2] = (mt_u32)ber_para.ber[2];

            break;
        }
        default:
            break;
        }
#endif
    }
    /* For satellite, convert it at lower driver. */
    else if ((MT_UNF_FE_SIG_TYPE_SAT == g_current_fe_connect_para[tuner_id].sig_type) || 
             (MT_UNF_FE_SIG_TYPE_SAT_2 == g_current_fe_connect_para[tuner_id].sig_type) || 
             (MT_UNF_FE_SIG_TYPE_DVBS_AUTO == g_current_fe_connect_para[tuner_id].sig_type) || 
             ((MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2) == g_current_fe_connect_para[tuner_id].sig_type))
    {
#if 0
        memcpy(p_ber, ber_para.ber, sizeof(ber_para.ber));

        d_ber = p_ber[0] / pow(10.0, 9.0);
        if (d_ber > 1 || d_ber < 0)
        {
            MT_ERR_FRONTEND("Error BER !\n");
            return MT_FAILURE;
        }

        if (d_ber == 0)
        {
            p_ber[0] = 0;
            p_ber[1] = 0;
            p_ber[2] = 0;
        }
        else
        {
            i = 0;

            while (d_ber < 1.0)
            {
                d_ber *= 10;
                i++;
            }

            p_ber[0] = (mt_u32)d_ber;
            p_ber[1] = (mt_u32)((d_ber - p_ber[0]) * 1000.0);
            p_ber[2] = (mt_u32)i;
        }
#else
        fe_signal_info.tuner_id = tuner_id;
        //fe_signal_info.info = 0;
        ret = ioctl(g_fe_fd, FE_GET_SIGNAL_INFO_CMD, (int)&fe_signal_info);

        if (fe_signal_info.info.sig_info.sat.sat_type == MT_UNF_FE_DVBS2)
        {
            d_ber = (mt_double)ber_para.ber[1] / (mt_double)ber_para.ber[0];
            *p_ber = (mt_u32)d_ber;
            MT_INFO_FRONTEND("d_ber[%f]\n", d_ber);
            //printf("unf ge ber line[%d]\n",__LINE__);
        }
        /* Tscancode: duplicate branches for 'if' and 'else'. */
        //else if (fe_signal_info.info.sig_info.sat.sat_type == MT_UNF_PORT_TYPE_DVBS)
        //{
        //    *p_ber = 0;
        //    MT_INFO_FRONTEND("unf ge ber \n");
        //}
        else
        {
            *p_ber = 0;
            MT_INFO_FRONTEND("unf ge ber \n");
        }
#endif
    }
    else
    {
        //memcpy(p_ber, fe_databuf.databuf, sizeof(fe_databuf.databuf));
        u32ErrSum = ber_para.ber[0];
        d_ber = u32ErrSum * 1.0 / (ber_para.ber[1] * ber_para.ber[2]);
        *p_ber = (mt_u32)d_ber;
    }
    /* Modified end: l00185424 2011-11-26 Support satellite */

    return MT_SUCCESS;
#endif
#endif

#endif
}

mt_s32 mt_unf_fe_get_pre_ber(mt_u32 tuner_id, mt_u32 *p_ber)
{
    mt_s32 ret;
    /*mt_double d_ber = 0;Clean Warning [-Wunused-variable]*/
    //    mt_s32 i = 0;
    /*mt_u32 u32ErrSum;Clean Warning [-Wunused-variable]*/
    //fe_databuf_t fe_databuf;
    //fe_data_t fe_data;
    /*fe_status_t st;Clean Warning [-Wunused-variable]*/
    fe_ber_t ber_para;
    /*fe_signal_info_t fe_signal_info; Clean Warning [-Wunused-variable]*/

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_ber)
    {
        MT_ERR_FRONTEND("Input parameter(p_ber) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    /*st.tuner_id = tuner_id;Clean Warning [-Wunused-variable]*/
    /*p_ber points to the first address of three unsigned numbers which have BER*/
    /*CNcomment: p_ber */

#if 0
    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }
#endif

    ber_para.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_PRE_BER_CMD, &ber_para);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner mt_unf_fe_get_ber error\n");
        return ret;
    }

    {
        mt_u32 error_packets = ber_para.ber[1], total_packets = ber_para.ber[0];
        mt_u8 j = 0;

        p_ber[0] = 0; // integer part
        p_ber[1] = 0; // decimal part, 3-bit valid digits
        p_ber[2] = 0; // exponent part, absolute value, negative value

        if ((error_packets == 0) || (total_packets == 0))
        {
            p_ber[0] = 0;
            p_ber[1] = 0;
            p_ber[2] = 0;
        }
        else
        {
            while (error_packets < total_packets)
            {
                error_packets *= 10;

                p_ber[2]++;
            }

            p_ber[0] = error_packets / total_packets;

            j = 0;

            do
            {
                p_ber[1] *= 10;
                error_packets %= total_packets;
                error_packets *= 10;
                p_ber[1] += error_packets / total_packets;
                j++;
            } while (j < 3);
        }

        //printf("%s[%d] ---- error = %7d, total = %7d, PRE BER = %d.%03dE-%d\n", __FUNCTION__, __LINE__, error_packets, total_packets, p_ber[0], p_ber[1], p_ber[2]);
    }

    return ret;
}
mt_s32 mt_unf_fe_get_accurate_snr(mt_u32 tuner_id, mt_s32 *p_snr) /* unit: 0.001dB  */
{
    mt_s32 ret;

    fe_accurate_snr_t snr_para;
    fe_status_t st;

    CHECK_TUNER_OPEN();	

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_snr)
    {
        MT_ERR_FRONTEND("Input parameter(p_snr) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    st.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }

    snr_para.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_ACCURATE_SNR_CMD, &snr_para);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner GET_SNR_CMD error\n");
        return MT_ERR_FE_FAILED_GETSNR;
    }
	
    *p_snr = snr_para.snr;
	
    return MT_SUCCESS;
}
mt_s32 mt_unf_fe_get_snr(mt_u32 tuner_id, mt_u32 *p_snr) /* range : 0-255  */
{
#if 0
    int ret = 0;
    fe_snr_t snr;

    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (g_fe_fd_cnt == 0)
        return MT_UNF_TUNER_ERR_UNOPEN;

    if (tuner_info[tuner_id].attr_set_cnt == 0)
        return MT_UNF_TUNER_ERR_ATTR_UNSET;

    snr.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_SNR_CMD, &snr);
    if (ret < 0)
        return MT_UNF_TUNER_ERR_GET_SNR_FAIL;

    *p_snr = snr.snr;
    return MT_SUCCESS;

#else
    //  mt_double u32Snr = 0;
    //  mt_double tmp = 0.0;
    //  mt_double dSnrEva = 0;
    mt_s32 ret;
    //fe_data_t fe_data;

    fe_snr_t snr_para;
    fe_status_t st;

    //fe_data.data = 0;
    //fe_data.port = port;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_snr)
    {
        MT_ERR_FRONTEND("Input parameter(p_snr) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    st.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }

    snr_para.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_SNR_CMD, &snr_para);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner GET_SNR_CMD error\n");
        return MT_ERR_FE_FAILED_GETSNR;
    }

    *p_snr = snr_para.snr;

    return MT_SUCCESS;

#if 0
    /* calculate the SNR regarding to the modulation */
    if (MT_UNF_FE_SIG_TYPE_DVB_T == g_current_fe_connect_para[tuner_id].sig_type
        || MT_UNF_FE_SIG_TYPE_DVB_T2 == g_current_fe_connect_para[tuner_id].sig_type)
    {
        //tmp = (mt_double)fe_data.data;
        //*p_snr = (mt_u32)(10 * log10(tmp) - 11.7);
    }
    else
    {
        tmp = (mt_double)snr_para.snr;
        tmp = tmp / (pow(2.0, 20.0));
        switch (g_current_fe_attr[tuner_id].demod_dev_type)
        {
        case MT_UNF_DEMOD_TYPE_NONE:
            break;

        case MT_UNF_DEMOD_TYPE_DS3103:
        case MT_UNF_DEMOD_TYPE_BUTT:
        case MT_UNF_DEMOD_TYPE_J83B:
        switch (s_stCurrentSignal[tuner_id].qam_type)
        {
            case QAM_TYPE_16:
                dSnrEva = 10.0 * log10((5.0 / 18.0) / tmp);
                u32Snr = dSnrEva;
                break;

            case QAM_TYPE_32:
                dSnrEva = 10.0 * log10(0.2 / tmp);
                u32Snr = dSnrEva;
                break;

            case QAM_TYPE_64:
                dSnrEva = 10.0 * log10(((42.0 / 14.0) / 14.0) / tmp);
                u32Snr = dSnrEva;
                break;

            case QAM_TYPE_128:
                dSnrEva = 10.0 * log10(((82.0 / 22.0) / 22.0) / tmp);
                u32Snr = dSnrEva;
                break;

            case QAM_TYPE_256:
                dSnrEva = 10.0 * log10(((170.0 / 32.0) / 32.0) / tmp);
                u32Snr = dSnrEva;
                break;

            default:
                return MT_FAILURE;
        }
            break;

        case MT_UNF_DEMOD_TYPE_3136:
        case MT_UNF_DEMOD_TYPE_3136I:
            if (snr_para.snr)
            {
                u32Snr = 10.0 * log10(8192.0 / (mt_double)(snr_para.snr));
            }
            else
            {
                u32Snr = 50.0;
            }
            break;

        case MT_UNF_DEMOD_TYPE_AVL6211:
            u32Snr = (mt_u32)(snr_para.snr / 100.0);
            break;

        case MT_UNF_DEMOD_TYPE_MXL101:
            u32Snr = snr_para.snr;
            break;

        case MT_UNF_DEMOD_TYPE_MN88472:
            u32Snr = snr_para.snr;
            break;

        case MT_UNF_DEMOD_TYPE_IT9170:
            u32Snr = snr_para.snr;
            break;

        case MT_UNF_DEMOD_TYPE_IT9133:
            u32Snr = snr_para.snr;
            break;

        case MT_UNF_DEMOD_TYPE_MXL254:
        case MT_UNF_DEMOD_TYPE_MXL214:
            u32Snr = (mt_double) (snr_para.snr /10);
            break;

        case MT_UNF_DEMOD_TYPE_TDA18280:
            u32Snr = (mt_double) (snr_para.snr);
            break;

        default:
            return MT_ERR_FE_INVALID_PARA;
        }
        *p_snr = (mt_u32)u32Snr;
    }

    return MT_SUCCESS;
#endif
#endif
}

mt_s32 mt_unf_fe_get_signal_strength(mt_u32 tuner_id, mt_u32 *p_signal_strength)
{
#if 0
    int ret = 0;
    fe_signal_strength_t sig_strength;

    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (g_fe_fd_cnt == 0)
        return MT_UNF_TUNER_ERR_UNOPEN;

    if (tuner_info[tuner_id].attr_set_cnt == 0)
        return MT_UNF_TUNER_ERR_ATTR_UNSET;

    sig_strength.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_SIGNAL_STRENGTH_CMD, &sig_strength);
    if (ret < 0)
        return MT_UNF_TUNER_ERR_GET_SIG_STRENGTH_FAIL;

    *p_signal_strength = sig_strength.strength;

    return MT_SUCCESS;

#else

    mt_s32 ret;

    fe_signal_strength_t strength_para;
    /*fe_status_t st;Clean Warning [-Wunused-variable]*/

    //fe_data_t fe_data;
    //fe_databuf_t fe_databuf;
    //    mt_s32 s32Agc = 0;
    //    mt_u32 i = 0;
    //    mt_s32 s32levelcorrect = 0;
    //    fe_signal_level_sat_t* p_signalLevel = MT_NULL;

    //fe_data.port = port;
    //fe_data.data = 0;
    //fe_databuf.port= port;
    //memset(fe_databuf.databuf,0,sizeof(fe_databuf.databuf));

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_signal_strength)
    {
        MT_ERR_FRONTEND("Input parameter(p_signal_strength) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

#if 0
    st.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }
#endif

    strength_para.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_SIGNAL_STRENGTH_CMD, &strength_para);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner FE_GET_SIGNAL_STRENGTH_CMD error\n");
        *p_signal_strength = 0;
        return MT_ERR_FE_FAILED_GETSIGNALSTRENGTH;
    }

    *p_signal_strength = strength_para.strength[0];

    return MT_SUCCESS;

#if 0
    /* Modefied begin: l00185424 2011-11-26 Support satellite*/
    if (MT_UNF_FE_SIG_TYPE_CAB == g_current_fe_connect_para[tuner_id].sig_type)
    {
        strength_para.tuner_id = tuner_id;
        //strength_para.data = 0;
        ret = ioctl(g_fe_fd, FE_GET_SIGNAL_STRENGTH_CMD, &strength_para);

        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("Tuner TUNER_GET_SIGNALSTRENGTH_CMD error\n");
            *p_signal_strength  = 0;
            return MT_ERR_FE_FAILED_GETSIGNALSTRENGTH;
        }

        s32Agc = (mt_s32)strength_para.strength[1];

        switch (g_current_fe_attr[tuner_id].tuner_type)//tuner_type
        {
             case MT_UNF_TUNER_TYPE_CD1616:
            {
                if (s32Agc >= 2263)
                {
                    *p_signal_strength = 30;
                }
                else if (s32Agc >= 2019)
                {
                    *p_signal_strength = (mt_u32)((-1.0 / 61) * s32Agc + 67.10);
                }
                else if (s32Agc >= 1531)
                {
                    *p_signal_strength = (mt_u32)((-5.0 / 218) * s32Agc + 80.11);
                }
                else if (s32Agc >= 1122)
                {
                    *p_signal_strength = (mt_u32)((-12.0 / 409) * s32Agc + 89.92);
                }
                else if (s32Agc >= 1088)
                {
                    *p_signal_strength = (mt_u32)(-0.5 * s32Agc + 614);
                }
                else if (s32Agc >= 1032)
                {
                    *p_signal_strength = (mt_u32)((-5.0 / 7) * s32Agc + 847);
                }
                else
                {
                    *p_signal_strength = 110;
                }
                break;
            }

            case MT_UNF_TUNER_TYPE_ALPS_TDAE://MT_UNF_TUNER_TYPE_ALPS_TDAE
            {
                if (s32Agc >= 2134)
                {
                    *p_signal_strength  = 30;
                }
                else if (s32Agc >= 1763)
                {
                    *p_signal_strength = (mt_u32)((-9.0 / 371) * s32Agc + 81.77);
                }
                else if (s32Agc >= 1366)
                {
                    *p_signal_strength = (mt_u32)((-11.0 / 405) * s32Agc + 86.88);
                }
                else if (s32Agc >= 1285)
                {
                    *p_signal_strength = 70;
                }
                else
                {
                    *p_signal_strength = 100;
                }
                break;
            }

            case MT_UNF_TUNER_TYPE_TMX7070X:
            {
                if (s32Agc >= 2747)
                {
                    *p_signal_strength = 30;
                }
                else if (s32Agc >= 2255)
                {
                    *p_signal_strength = (mt_u32)((-9.0 / 160) * s32Agc + 184.5);
                }
                else
                {
                    *p_signal_strength = 75;
                }
                break;
            }

            case MT_UNF_TUNER_TYPE_TDA18250:
            {
                if (1 == strength_para.strength[2])
                {
                    s32Agc = 0- s32Agc;
                }

                for (i = 0; i < (sizeof(g_signal_level_tda18250) / sizeof(fe_signal_level_t)); i++)
                {
                    if (g_tuner_freq < (g_signal_level_tda18250[i].signal_freq * 1000))
                    {
                        if (0 == i)
                        {
                            s32levelcorrect = g_signal_level_tda18250[i].signal_level;
                            break;
                        }
                        s32levelcorrect = (g_signal_level_tda18250[i].signal_level + g_signal_level_tda18250[i - 1].signal_level) / 2;
                        break;
                    }
                    else if ((g_signal_level_tda18250[i].signal_freq * 1000) == g_tuner_freq)
                    {
                        s32levelcorrect = g_signal_level_tda18250[i].signal_level;
                        break;
                    }
                    else
                    {
                        if (((sizeof(g_signal_level_tda18250) / sizeof(fe_signal_level_t)) - 1) == i)
                        {
                            s32levelcorrect = g_signal_level_tda18250[i].signal_level;
                            break;
                        }
                    }
                 }

                 *p_signal_strength = (mt_u32)(s32Agc / 100.00 + 60 + s32levelcorrect);
                 break;
            }

            case MT_UNF_TUNER_TYPE_M88TS2022:
            {
                //*p_signal_strength = (mt_u32)(s32Agc / 101.76 - 1.0);
                *p_signal_strength = (mt_u32)(s32Agc / 101.76);
                break;
            }

            case MT_UNF_TUNER_TYPE_MT2081:
            {
                *p_signal_strength = (mt_u32)((((109.0 - 30.0) * (s32Agc - 3102.0) / (10514.0 - 3102.0))) + 30);
                break;
            }

            /*TDCC-G131F*/
            case MT_UNF_TUNER_TYPE_TDCC:
            {
                if (s32Agc >= 1166)
                {
                    *p_signal_strength = (mt_u32)((-22.0 / 634) * s32Agc + 92.46);
                }
                else
                {
                    *p_signal_strength = 81;
                }
                break;
            }

            case MT_UNF_TUNER_TYPE_R820C:
            {
                if (MT_UNF_DEMOD_TYPE_BUTT == g_current_fe_attr[tuner_id].demod_dev_type)
                {
                    if (45 == strength_para.strength[2])
                    {
                        *p_signal_strength = (mt_u32)(60.0 - ((s32Agc * 26.0) / 613.0));
                    }
                    else
                    {
                        if (strength_para.strength[2] > 27)
                        {
                            *p_signal_strength = (mt_u32)((109 - ((50.0 / 39.0) * strength_para.strength[2])) + (50.0 / 39.0));
                        }
                        else
                        {
                            *p_signal_strength = (mt_u32)((107 - ((27.0 / 21.0) * strength_para.strength[2]))+ (9.0 / 21.0));
                        }
                    }
                }
                else
                {
                    if (MT_UNF_DEMOD_TYPE_J83B == g_current_fe_attr[tuner_id].demod_dev_type)
                    {
                        if (45 == strength_para.strength[2])
                        {
                            *p_signal_strength = (mt_u32)((s32Agc * (-21.0 / 1011.0)) + 37.0 + (21.0 / 1011.0) * 2616.0);
                        }
                        else
                        {
                            if (strength_para.strength[2] > 27)
                            {
                                *p_signal_strength = (mt_u32)((109.0 - ((50.00 / 39.00) * strength_para.strength[2])) + (50.00 / 39.00));
                            }
                            else
                            {
                                *p_signal_strength = (mt_u32)((107.0 - ((27.00 / 21.00) * strength_para.strength[2])) + (9.00 / 21.00));
                            }
                        }
                    }
                    else
                    {
                        if (45 == strength_para.strength[2])
                        {
                            *p_signal_strength = (mt_u32)((s32Agc - 4116.5) / (-50.85) + 2.0);
                        }
                        else
                        {
                            if (strength_para.strength[2] > 27)
                            {
                                *p_signal_strength = (mt_u32)((109.0 - ((50.00 / 39.00) * strength_para.strength[2])) + (50.00 / 39.00));
                            }
                            else
                            {
                                *p_signal_strength = (mt_u32)((107.0 - ((27.00 / 21.00) * strength_para.strength[2])) + (9.00 / 21.00));
                            }
                        }
                    }
                }
                break;
            }

            case MT_UNF_TUNER_TYPE_MXL203:
            {
                *p_signal_strength = (mt_u32)(110.0 - (((s32Agc - 691.0) * 80.0) / (2503.0 - 691.0)));
                break;
            }

            case MT_UNF_TUNER_TYPE_MXL603:
            {
                if (1 == strength_para.strength[2])
                {
                    s32Agc = 0 - s32Agc;
                }

                *p_signal_strength = (mt_u32)((s32Agc / 100.0) + 107);
                break;
            }

            case MT_UNF_TUNER_TYPE_MXL254:
            case MT_UNF_TUNER_TYPE_MXL214:
            {
                *p_signal_strength = (mt_u32)(s32Agc);
                break;
            }

            case MT_UNF_TUNER_TYPE_TDA18280:
            {
                *p_signal_strength = (mt_u32)(s32Agc);
                break;
            }

            default:
            {
                return MT_FAILURE;
            }
        }
    }
    else if (MT_UNF_FE_SIG_TYPE_SAT == g_current_fe_connect_para[tuner_id].sig_type)
    {
        strength_para.tuner_id = tuner_id;
        //fe_data.data = 0;
        ret = ioctl(g_fe_fd, FE_GET_SIGNAL_STRENGTH_CMD, &strength_para);

        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("Tuner TUNER_GET_SIGNALSTRENGTH_CMD error\n");
            *p_signal_strength = 0;
            return MT_ERR_FE_FAILED_GETSIGNALSTRENGTH;
        }

        if (MT_UNF_DEMOD_TYPE_AVL6211 == g_current_fe_attr[tuner_id].demod_dev_type)
        {
            switch (g_current_fe_attr[tuner_id].tuner_type)
            {
            case MT_UNF_TUNER_TYPE_AV2011:
                p_signalLevel = g_signal_level_av2011;
                break;

            case MT_UNF_TUNER_TYPE_SHARP7903:
                p_signalLevel = g_signal_level_sharp7903;
                break;

            default:
                {
                    return MT_FAILURE;
                }
            }

            //if (MT_NULL != p_signalLevel)
            {
                for (i = 0; i < 100; i++)
                {
                    if (strength_para.strength[1] <= p_signalLevel[i].signal_level)
                    {
                        if ((0 == i) && (strength_para.strength[1] < p_signalLevel[i].signal_level))
                        {
                            MT_ERR_FRONTEND("RFSignalLevel is too weak !");
                            *p_signal_strength = 18;
                        }
                        else
                        {
                            *p_signal_strength = (mt_u16)(p_signalLevel[i].signal_dbuv);
                        }

                        break;
                    }
                }
            }
            /*else
            {
                *p_signal_strength = 18;
            }*/
        }
        else if (MT_UNF_DEMOD_TYPE_3136 == g_current_fe_attr[tuner_id].demod_dev_type ||
                    MT_UNF_DEMOD_TYPE_3136I == g_current_fe_attr[tuner_id].demod_dev_type)
        {
            s32Agc = (mt_s32)strength_para.strength[1];
            switch (g_current_fe_attr[tuner_id].tuner_type)
            {
            case MT_UNF_TUNER_TYPE_AV2011:
            default:
                {
                    if (s32Agc < 1000)  //>-10dbm
                    {
                        *p_signal_strength = 97;
                    }
                    else if (s32Agc < 1600)  //-40dbm ~ -10dbm
                    {
                        *p_signal_strength = (mt_u32)(67 - 0.05 * (s32Agc - 1600));
                    }
                    else if (s32Agc < 2200)
                    {
                        *p_signal_strength = (mt_u32)(47 - 0.033 * (s32Agc - 2200));
                    }
                    else if (s32Agc < 3000)
                    {
                        *p_signal_strength = (mt_u32)(27 - 0.025 * (s32Agc - 3000));
                    }
                    else
                    {
                        *p_signal_strength = (mt_u32)(17 - 0.01 * (s32Agc - 4095));    //<-80dbm
                    }
                    break;
                }

            case MT_UNF_TUNER_TYPE_SHARP7903:
                {
                    if (s32Agc < 600)
                    {
                        *p_signal_strength = 107;      //<-80dbm
                    }
                    else if (s32Agc < 1150) //-18dbm ~ 0dbm
                    {
                        *p_signal_strength = (mt_u32)(89 - (18.0 / 500) * (s32Agc - 1150));
                    }
                    else if (s32Agc < 1300) //-30dbm ~ -18dbm
                    {
                        *p_signal_strength = (mt_u32)(77 - (12.0 / 150) * (s32Agc - 1300));
                    }
                    else if (s32Agc < 1700) //-40dbm ~ -30dbm
                    {
                        *p_signal_strength = (mt_u32)(47 - (10.0 / 400) * (s32Agc - 1700));
                    }
                    else if (s32Agc < 2340) //-60dbm ~ -40dbm
                    {
                        *p_signal_strength = (mt_u32)(47 - (20.0 / 640) * (s32Agc - 2340));
                    }
                    else if (s32Agc < 2480) //-63dbm ~ -60dbm
                    {
                        *p_signal_strength = (mt_u32)(44 - (3.0 / 140) * (s32Agc - 2480));
                    }
                    else if (s32Agc < 2650) //-66dbm ~ -63dbm
                    {
                        *p_signal_strength = (mt_u32)(41 - (3.0 / 170) * (s32Agc - 2650));
                    }
                    else if (s32Agc < 3500) //-85dbm ~ -66dbm
                    {
                        *p_signal_strength = (mt_u32)(22 - (19.0 / 850) * (s32Agc - 3500));
                    }
                    else        //<-85dbm
                    {
                        *p_signal_strength = 22;
                    }
                    break;
                }

            case MT_UNF_TUNER_TYPE_RDA5815:
                {
                    if (s32Agc < 350)  //>-10dbm
                    {
                        *p_signal_strength = 97;   //(dbuv)
                    }
                    else if (s32Agc < 800)       //-25dbm ~ -10dbm
                    {
                        *p_signal_strength = (mt_u32)(82 - 0.033 * (s32Agc - 800));
                    }
                    else if (s32Agc < 1725)     //-50dbm ~ -25dbm
                    {
                        *p_signal_strength = (mt_u32)(57 - 0.027 * (s32Agc - 1725));
                    }
                    else if (s32Agc < 2207)  //-60dbm ~ -50dbm
                    {
                        *p_signal_strength = (mt_u32)(47 - 0.021 * (s32Agc - 2207));
                    }
                    else if (s32Agc < 2507)  //-63dbm ~ -60dbm
                    {
                        *p_signal_strength = (mt_u32)(44 - 0.01 * (s32Agc - 2507));
                    }
                    else if (s32Agc < 2661)  //-66dbm ~ -63dbm
                    {
                        *p_signal_strength = (mt_u32)(41 - 0.0195 * (s32Agc - 2661));
                    }
                    else if (s32Agc < 2884)  //-69dbm ~ -66dbm
                    {
                        *p_signal_strength = (mt_u32)(38 - 0.0135 * (s32Agc - 2884));
                    }
                    else if (s32Agc < 3178)  //-72dbm ~ -69dbm
                    {
                        *p_signal_strength = (mt_u32)(35 - 0.0102 * (s32Agc - 3178));
                    }
                    else if (s32Agc < 3910)  //-81dbm ~ -72dbm
                    {
                        *p_signal_strength = (mt_u32)(26 - 0.0123 * (s32Agc - 3910));
                    }
                    else if (s32Agc < 4085)  //-84dbm ~ -81dbm
                    {
                        *p_signal_strength = (mt_u32)(23 - 0.0171 * (s32Agc - 4085));
                    }
                    else
                    {
                        *p_signal_strength = 22;   //<-85dbm
                    }
                    break;
                }

            case MT_UNF_TUNER_TYPE_M88TS2022:
                {
                    mt_u32 u32BBGainBase = 0, u32BBGain = 0, u32RFGain = 0;

                    if (s32Agc < 3920)
                    {
                        u32BBGainBase = 0;
                    }
                    else if (s32Agc < 3952)
                    {
                        u32BBGainBase = (mt_u32)((2.0 / 32) * (s32Agc - 3920));
                    }
                    else if (s32Agc < 4000)
                    {
                        u32BBGainBase = (mt_u32)(2 + (4.5 / 48) * (s32Agc - 3952));
                    }
                    else if (s32Agc < 4016)
                    {
                        u32BBGainBase = (mt_u32)(6.5 + (0.5 / 16) * (s32Agc - 4000));
                    }
                    else if (s32Agc < 4032)
                    {
                        u32BBGainBase = (mt_u32)(7 + (4.0 / 16) * (s32Agc - 4016));
                    }
                    else if (s32Agc < 4040)
                    {
                        u32BBGainBase = (mt_u32)(11 + (1.0 / 8) * (s32Agc - 4032));
                    }
                    else if (s32Agc < 4080)
                    {
                        u32BBGainBase = (mt_u32)(12 + (0.3 / 8) * (s32Agc - 4040));
                    }
                    else
                    {
                        u32BBGainBase = 13;
                    }

                    u32BBGain = (mt_u32)((p_signal_strength[0] & 0x1f) * 36.0 / 10);
                    u32RFGain = (mt_u32)(2 + (p_signal_strength[2] & 0x1f) * 24.0 / 10);
                    *p_signal_strength =(mt_u32)(107 - (u32BBGainBase + u32BBGain + u32RFGain));
                    break;
                }
            }
        }
    }
    else
    {
        strength_para.tuner_id = tuner_id;
        ret = ioctl(g_fe_fd, FE_GET_SIGNAL_STRENGTH_CMD, &strength_para);

        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("Tuner FE_GET_SIGNALSTRENGTH_CMD error\n");
            *p_signal_strength = 0;
            return MT_ERR_FE_FAILED_GETSIGNALSTRENGTH;
        }
        *p_signal_strength = strength_para.strength[1];
    }

    return MT_SUCCESS;
#endif

#endif
}

mt_s32 mt_unf_fe_get_signal_quality(mt_u32 tuner_id, mt_u32 *p_signal_quality)
{
#if 0
    int ret = 0;
    fe_signal_quality_t quality;

    if (init_cnt == 0)
        return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= MT_UNF_FE_MAX_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    if (g_fe_fd_cnt == 0)
        return MT_UNF_TUNER_ERR_UNOPEN;

    if (tuner_info[tuner_id].attr_set_cnt == 0)
        return MT_UNF_TUNER_ERR_ATTR_UNSET;

    quality.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_SIGNAL_QUALITY_CMD, &quality);
    if (ret < 0)
        return MT_UNF_TUNER_ERR_GET_SIG_QUALITY_FAIL;

    *p_signal_quality = quality.quality;

    return MT_SUCCESS;

#else

    mt_s32 ret; //,delta;
    //fe_data_t fe_data;
    //    fe_signal_info_t fe_signal_info;
    //    mt_u32 snr = 0;
    //    mt_double tmp = 0;

    fe_signal_quality_t quality_para;
    //    fe_snr_t snr_para;
    fe_status_t st;

    //fe_data.port = port;
    //fe_data.data = 0;
    //fe_signal_info.port = port;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_signal_quality)
    {
        MT_ERR_FRONTEND("Input parameter(p_signal_quality) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    st.tuner_id = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status) //MT_UNF_FE_SIGNAL_LOCKED
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        *p_signal_quality = 0;
        return MT_ERR_FE_NOT_CONNECT;
    }

    quality_para.tuner_id = tuner_id;
    quality_para.quality = 0;
    ret = ioctl(g_fe_fd, FE_GET_SIGNAL_QUALITY_CMD, &quality_para);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner get quality error\n");
        return MT_ERR_FE_FAILED_GETSIGNALQUALITY;
    }

    *p_signal_quality = quality_para.quality;

#if 0
    snr_para.tuner_id = tuner_id;
    snr_para.snr = 0;
    ret = ioctl(g_fe_fd, FE_GET_SNR_CMD, (int)&snr_para);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner get SNR error\n");
        return MT_ERR_FE_FAILED_GETSIGNALQUALITY;
    }

    switch (g_current_fe_attr[tuner_id].demod_dev_type)
    {
        case MT_UNF_DEMOD_TYPE_3136:
        case MT_UNF_DEMOD_TYPE_3136I:
            snr = (mt_u32)(1000*(3.913 - log10((mt_double)snr_para.snr)));
            break;
        case MT_UNF_DEMOD_TYPE_3137:
            tmp = (mt_double)(10*log10((mt_double)snr_para.snr) - 11.7);
            break;
        case MT_UNF_DEMOD_TYPE_AVL6211:
        default:
            snr = snr_para.snr;
    }

    /* For SAT */
    if (MT_UNF_FE_SIG_TYPE_SAT == g_current_fe_connect_para[tuner_id].sig_type)
    {
        mt_u8 u8SNRrefer = 0;
        typedef struct
        {
            mt_unf_fe_fecrate_t fec_rate;
            mt_u8 u8ReferSNR;
        } TUNER_REFER_SNR_S;

        TUNER_REFER_SNR_S astDVBS2QpskRefSNR[8] =
        {
            {MT_UNF_FE_FEC_1_2, 10}, {MT_UNF_FE_FEC_3_5,  24}, {MT_UNF_FE_FEC_2_3, 32},
            {MT_UNF_FE_FEC_3_4, 41}, {MT_UNF_FE_FEC_4_5,  47}, {MT_UNF_FE_FEC_5_6, 52},
            {MT_UNF_FE_FEC_8_9, 63}, {MT_UNF_FE_FEC_9_10, 65}
        };

        TUNER_REFER_SNR_S astDVBSRefSNR[6];
        TUNER_REFER_SNR_S astDVBS28pskRefSNR[6];

        TUNER_REFER_SNR_S astDVBSRefSNR_Avl6211[6] =
        {
            {MT_UNF_FE_FEC_1_2, 12}, {MT_UNF_FE_FEC_2_3, 32}, {MT_UNF_FE_FEC_3_4, 41},
            {MT_UNF_FE_FEC_5_6, 52}, {MT_UNF_FE_FEC_6_7, 58}, {MT_UNF_FE_FEC_7_8, 62}
        };
        TUNER_REFER_SNR_S astDVBS28pskRefSNR_Avl6211[6] =
        {
            {MT_UNF_FE_FEC_3_5, 57}, {MT_UNF_FE_FEC_2_3, 67}, {MT_UNF_FE_FEC_3_4, 80},
            {MT_UNF_FE_FEC_4_5, 95}, {MT_UNF_FE_FEC_5_6, 10}, {MT_UNF_FE_FEC_8_9, 11}
        };

        TUNER_REFER_SNR_S astDVBSRefSNR_Hi3136[6] =
        {
            {MT_UNF_FE_FEC_1_2, 23}, {MT_UNF_FE_FEC_2_3, 42}, {MT_UNF_FE_FEC_3_4, 52},
            {MT_UNF_FE_FEC_5_6, 63}, {MT_UNF_FE_FEC_6_7, 70}, {MT_UNF_FE_FEC_7_8, 70}
        };
        TUNER_REFER_SNR_S astDVBS28pskRefSNR_Hi3136[6] =
        {
            {MT_UNF_FE_FEC_3_5, 57}, {MT_UNF_FE_FEC_2_3, 67}, {MT_UNF_FE_FEC_3_4, 80},
            {MT_UNF_FE_FEC_4_5, 95}, {MT_UNF_FE_FEC_5_6, 108}, {MT_UNF_FE_FEC_8_9, 111}
        };

        switch (g_current_fe_attr[tuner_id].demod_dev_type)
        {
            case MT_UNF_DEMOD_TYPE_3136:
            case MT_UNF_DEMOD_TYPE_3136I:
                memcpy(astDVBSRefSNR, astDVBSRefSNR_Hi3136, sizeof(astDVBSRefSNR_Hi3136));
                memcpy(astDVBS28pskRefSNR, astDVBS28pskRefSNR_Hi3136, sizeof(astDVBS28pskRefSNR_Hi3136));
                break;

            case MT_UNF_DEMOD_TYPE_AVL6211:
            default:
                memcpy(astDVBSRefSNR, astDVBSRefSNR_Avl6211, sizeof(astDVBSRefSNR_Avl6211));
                memcpy(astDVBS28pskRefSNR, astDVBS28pskRefSNR_Avl6211, sizeof(astDVBS28pskRefSNR_Avl6211));
        }

        fe_signal_info.tuner_id = tuner_id;
        fe_signal_info.info = 0;
        ret = ioctl(g_fe_fd, FE_GET_SIGNAL_INFO_CMD, (int)&fe_signal_info);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("Tuner get signal info error\n");
            return MT_ERR_FE_FAILED_GETSIGNALQUALITY;
        }

        /* For DVBS */
        if (MT_UNF_FE_DVBS == fe_signal_info.info.sig_info.sat.sat_type)
        {
            ;//GET_REFER_SNR(u8SNRrefer, astDVBSRefSNR, fe_signal_info.info.sig_info.sat.fec_rate);//fec_rate
        }
        /* For DVBS2 */
        else
        {
            if (MT_UNF_MOD_TYPE_8PSK == fe_signal_info.info.sig_info.sat.mode_type)
            {
                ;//GET_REFER_SNR(u8SNRrefer, astDVBS28pskRefSNR, fe_signal_info.info.sig_info.sat.fec_rate);
            }
            else if (MT_UNF_MOD_TYPE_QPSK == fe_signal_info.info.sig_info.sat.mode_type)
            {
                ;//GET_REFER_SNR(u8SNRrefer, astDVBS2QpskRefSNR, fe_signal_info.info.sig_info.sat.fec_rate);
            }
        }

        if ((snr / 10) > u8SNRrefer)
        {
            snr = snr / 10 - u8SNRrefer;
            if (snr >= 100)
            {
                *p_signal_quality = 99;
            }
            else if (snr >= 50)  /* >5.0dB */
            {
                *p_signal_quality = 80 + (snr - 50) * 20 / 50;
            }
            else if (snr >= 25)  /* > 2.5dB */
            {
                *p_signal_quality = 50 + (snr - 25) * 30 / 25;
            }
            else if (snr >= 10)  /* > 1dB */
            {
                *p_signal_quality = 25 + (snr - 10) * 25 / 15;
            }
            else
            {
                *p_signal_quality = 5 + (snr) * 20 / 10;
            }
        }
        else
        {
            *p_signal_quality = 5;
        }
    }

#endif

#if 0
    /* For other, unsupport now */
    else if (MT_UNF_FE_SIG_TYPE_DVB_T <= g_current_fe_connect_para[tuner_id].sig_type && MT_UNF_FE_SIG_TYPE_DTMB >= g_current_fe_connect_para[tuner_id].sig_type)
    {
         typedef struct
        {
            mt_unf_modulation_type_t  enModulation;
            mt_unf_fe_fecrate_t fec_rate;
            mt_float   f16NordigP1;
        } TUNER_NORDIG_P1_S;

        TUNER_NORDIG_P1_S  stCNNordigP1[4][6] =
        {
            {{MT_UNF_MOD_TYPE_QPSK,MT_UNF_FE_FEC_1_2,3.5},{MT_UNF_MOD_TYPE_QPSK,MT_UNF_FE_FEC_3_5,4.7},{MT_UNF_MOD_TYPE_QPSK,MT_UNF_FE_FEC_2_3,5.6},{MT_UNF_MOD_TYPE_QPSK,MT_UNF_FE_FEC_3_4,6.6},{MT_UNF_MOD_TYPE_QPSK,MT_UNF_FE_FEC_4_5,7.2},{MT_UNF_MOD_TYPE_QPSK,MT_UNF_FE_FEC_5_6,7.7}},
            {{MT_UNF_MOD_TYPE_QAM_16,MT_UNF_FE_FEC_1_2,8.7},{MT_UNF_MOD_TYPE_QAM_16,MT_UNF_FE_FEC_3_5,10.1},{MT_UNF_MOD_TYPE_QAM_16,MT_UNF_FE_FEC_2_3,11.4},{MT_UNF_MOD_TYPE_QAM_16,MT_UNF_FE_FEC_3_4,12.5},{MT_UNF_MOD_TYPE_QAM_16,MT_UNF_FE_FEC_4_5,13.3},{MT_UNF_MOD_TYPE_QAM_16,MT_UNF_FE_FEC_5_6,13.8}},
            {{MT_UNF_MOD_TYPE_QAM_64,MT_UNF_FE_FEC_1_2,13.0},{MT_UNF_MOD_TYPE_QAM_64,MT_UNF_FE_FEC_3_5,14.8},{MT_UNF_MOD_TYPE_QAM_64,MT_UNF_FE_FEC_2_3,16.2},{MT_UNF_MOD_TYPE_QAM_64,MT_UNF_FE_FEC_3_4,17.7},{MT_UNF_MOD_TYPE_QAM_64,MT_UNF_FE_FEC_4_5,18.7},{MT_UNF_MOD_TYPE_QAM_64,MT_UNF_FE_FEC_5_6,19.4}},
            {{MT_UNF_MOD_TYPE_QAM_256,MT_UNF_FE_FEC_1_2,17.0},{MT_UNF_MOD_TYPE_QAM_256,MT_UNF_FE_FEC_3_5,19.4},{MT_UNF_MOD_TYPE_QAM_256,MT_UNF_FE_FEC_2_3,20.8},{MT_UNF_MOD_TYPE_QAM_256,MT_UNF_FE_FEC_3_4,22.9},{MT_UNF_MOD_TYPE_QAM_256,MT_UNF_FE_FEC_4_5,24.3},{MT_UNF_MOD_TYPE_QAM_256,MT_UNF_FE_FEC_5_6,25.1}},
        };

        mt_u8 i,j;
        double f32CNReceive = 0,f32NordigReference = 0,f32CNRelative = 0,f32SNRReceive = 0;

        switch (g_current_fe_attr[tuner_id].demod_dev_type)
        {
        case MT_UNF_DEMOD_TYPE_MXL101:
            f32SNRReceive = ((mt_double)fe_data.data) / 10000;

            ret = ioctl(g_fe_fd, FE_GET_SIGNALINFO_CMD, (int)&fe_signal_info);
            if (MT_SUCCESS == ret)
            {
                for (i = 0; i < 4; i++)
                {
                    if (stCNNordigP1[i][0].enModulation == fe_signal_info.info.sig_info.ter.mod_type)
                    {
                        for (j = 0; j < 6; j++)
                        {
                            if (stCNNordigP1[i][j].fec_rate == fe_signal_info.info.sig_info.ter.fec_rate)
                            {
                                f32NordigReference = stCNNordigP1[i][j].f16NordigP1;
                            }
                        }
                    }
                }
            }
            else
            {
                MT_ERR_FRONTEND("tuner get signalinfo fail.\n");
                return MT_ERR_FE_FAILED_GETSIGNALQUALITY;
            }

            f32CNReceive = f32SNRReceive - 1;
            f32CNRelative = f32CNReceive - f32NordigReference;
            if (f32CNRelative >= 0)
            {
                if (f32CNRelative >= 10)
                {
                    *p_signal_quality = 99;
                }
                else if (f32CNRelative >= 6)
                {
                    *p_signal_quality = 75 + (f32CNRelative - 6) * 24 / 4.0;
                }
                else if (f32CNRelative >= 3.5)  /* > 3.5dB */
                {
                    *p_signal_quality = 45 + (f32CNRelative - 3.5) * 30 / 2.5;
                }
                else if (f32CNRelative >= 2)  /* > 2dB */
                {
                    *p_signal_quality = 30 + (f32CNRelative - 2) * 15 / 1.5;
                }
                else if (f32CNRelative >= 1)  /* > 1dB */
                {
                    *p_signal_quality = 15 + (f32CNRelative - 1) * 15 / 1.0;
                }
                else
                {
                    *p_signal_quality = 5 + (f32CNRelative) * 10;
                }
            }
            else
            {
                *p_signal_quality = 5;
            }
            break;
        case MT_UNF_DEMOD_TYPE_3137:
            ret = ioctl(g_fe_fd, FE_GET_SIGANLINFO_CMD, (int)&fe_signal_info);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_FRONTEND("Tuner mt_unf_fe_get_signal_info error\n");
                return MT_ERR_FE_FAILED_GETSIGNALINFO;
            }

            if (MT_UNF_MOD_TYPE_QPSK == fe_signal_info.info.sig_info.ter.mod_type)
            {
                delta = 25;
            }
            else if (MT_UNF_MOD_TYPE_QAM_16 == fe_signal_info.info.sig_info.ter.mod_type)
            {
                delta = 15;
            }
            else if (MT_UNF_MOD_TYPE_QAM_64 == fe_signal_info.info.sig_info.ter.mod_type)
            {
                delta = 10;
            }
            else
            {
                delta = -5;
            }

            tmp = 3 * tmp + delta;

            if (tmp > 95)
                tmp = 95;
            else if (tmp < 10)
                tmp = 10;
            else
                tmp = tmp;

            *p_signal_quality = (mt_u32)tmp;
            break;
        default:
            *p_signal_quality = 0;
            return MT_ERR_FE_FAILED_GETSIGNALQUALITY;
        }
    }
    else
    {
        *p_signal_quality = 0;
        return MT_ERR_FE_FAILED_GETSIGNALQUALITY;
    }
#endif

    return MT_SUCCESS;
#endif
}

mt_s32 mt_unf_fe_get_real_freq_symb(mt_u32 tuner_id, mt_u32 *pu32Freq, mt_u32 *pu32Symb, mt_s32 *ps32FreqOffset)
{
    mt_s32 ret = MT_FAILURE;
    fe_data_t fe_data = {0};
    fe_status_t st;

    st.tuner_id = tuner_id;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == pu32Freq)
    {
        MT_ERR_FRONTEND("Input parameter(pu32Freq) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_NULL == pu32Symb)
    {
        MT_ERR_FRONTEND("Input parameter(pu32Symb) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_NULL == ps32FreqOffset)
    {
        MT_ERR_FRONTEND("Input parameter(ps32FreqOffset) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }

    fe_data.port = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_REAL_FREQ, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner FE_GET_REAL_FREQ error\n");
        return MT_ERR_FE_FAILED_REGRW;
    }

    *pu32Freq = fe_data.data;

    fe_data.port = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_FREQ_OFFSET, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner FE_GET_FREQ_OFFSET error\n");
        return MT_ERR_FE_FAILED_REGRW;
    }

    *ps32FreqOffset = (mt_s32)(fe_data.data);

    fe_data.port = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_REAL_SYMBOL, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner FE_GET_REAL_SYMBOL error\n");
        return MT_ERR_FE_FAILED_REGRW;
    }

    *pu32Symb = fe_data.data;

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_s2_multi_stream_info(mt_u32 tuner_id, mt_unf_fe_connect_para_t *p_connect_para)
{
    mt_s32 ret = MT_FAILURE;
    fe_data_t fe_data = {0};
    fe_status_t st;

    fe_datalist_t fe_datalist;

    st.tuner_id = tuner_id;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_connect_para)
    {
        MT_ERR_FRONTEND("Input parameter(p_connect_para) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }

    fe_data.port = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_S2_MULTI_STREAM_TS_CNT, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner FE_GET_S2_MULTI_STREAM_TS_CNT error\n");
        return MT_ERR_FE_FAILED_REGRW;
    }

    p_connect_para->connect_param.sat.DataTsNumber = fe_data.data;

    //fe_data.port = tuner_id;
    //ret = ioctl(g_fe_fd, FE_GET_S2_MULTI_STREAM_TS_ID, &fe_data);
    fe_datalist.port = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_S2_MULTI_STREAM_TS_ID, &fe_datalist);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner FE_GET_S2_MULTI_STREAM_TS_ID error\n");
        return MT_ERR_FE_FAILED_REGRW;
    }

    memcpy(p_connect_para->connect_param.sat.DataTsIdArray, fe_datalist.data_list, sizeof(p_connect_para->connect_param.sat.DataTsIdArray) / sizeof(mt_u8));

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_s2_multi_stream_ts_id(mt_u32 tuner_id, mt_u8 ts_id)
{
    mt_s32 ret = MT_FAILURE;
    fe_data_t fe_data = {0};
    fe_status_t st;

    st.tuner_id = tuner_id;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }

    fe_data.port = tuner_id;
    fe_data.data = ts_id;
    ret = ioctl(g_fe_fd, FE_SET_S2_MULTI_STREAM_TS_ID, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner FE_SET_S2_MULTI_STREAM_TS_ID error\n");
        return MT_ERR_FE_FAILED_REGRW;
    }

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_gs_package_mode(mt_u32 tuner_id, mt_u8 *p_gs_package_mode)
{
    mt_s32 ret = MT_FAILURE;
	//mt_s32 i = 0;
    fe_data_t fe_data = {0};
    fe_status_t st;

    st.tuner_id = tuner_id;
	
	 if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }
	if (MT_NULL == p_gs_package_mode)
    {
        MT_ERR_FRONTEND("Input parameter(p_gs_package_mode) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }
	//for(i = 0; i < 100;i++)
    {
		ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
    	if (MT_SUCCESS != ret)
    	{
        	MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        	return MT_ERR_FE_FAILED_GETSTATUS;
    	}
		//MT_ERR_FRONTEND("lock status:%d\n",st.status.lock_status);
	}

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }

    fe_data.port = tuner_id;
    fe_data.data = (mt_u32)*p_gs_package_mode;
    ret = ioctl(g_fe_fd, FE_SET_S2_GS_PACKAGE_MODE_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Get BBHeader TS/GS Mode fail.\n");
        return MT_ERR_FE_FAILED_SETGSPACKETMODE;
    }
    return MT_SUCCESS;
}


mt_s32 mt_unf_fe_get_s2_bbheader_ts_gse_mode(mt_u32 tuner_id, mt_u8 *p_ts_gse_mode)
{
    mt_s32 ret = MT_FAILURE;
    //mt_s32 i = 0;
    fe_data_t fe_data = {0};
    fe_status_t st;

    st.tuner_id = tuner_id;
	
    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_ts_gse_mode)
    {
        MT_ERR_FRONTEND("Input parameter(p_ts_gse_mode) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    //for(i = 0; i < 100;i++)
    {
        ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
            return MT_ERR_FE_FAILED_GETSTATUS;
        }
        //MT_ERR_FRONTEND("lock status:%d\n",st.status.lock_status);
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }

    fe_data.port = tuner_id;
    ret = ioctl(g_fe_fd, FE_GET_S2_BBH_TS_GSE_MODE_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Get BBHeader TS/GS Mode fail.\n");
        return MT_ERR_FE_FAILED_GETBBHTSGSEMODE;
    }

    *p_ts_gse_mode = (mt_u8)fe_data.data;

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_s2_set_gse_attr(mt_u32 tuner_id, mt_u32 gse_param)
{
    mt_s32 ret = MT_FAILURE;
    fe_data_t fe_data = {0};
    fe_status_t st;

    st.tuner_id = tuner_id;

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    /*In dump adc data mode , do not need to check whether frontend is locked*/
    if ((MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)&&(gse_param < 0xFF))
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }

    fe_data.port = tuner_id;
    fe_data.data = gse_param;
    ret = ioctl(g_fe_fd, FE_GET_S2_GSE_ID_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner FE_SET_S2_MULTI_STREAM_TS_ID error\n");
        return MT_ERR_FE_FAILED_REGRW;
    }

    return MT_SUCCESS;
}

/*Obtains current signal information of the TUNER, used in satellite and terrestrial, not necessary for cable.*/
mt_s32 mt_unf_fe_get_signal_info(mt_u32 tuner_id, mt_unf_fe_signal_info_t *p_signal_info)
{
#if 1
    mt_s32 ret;
    fe_signal_info_t fe_signal_info;
    //fe_status_t st;

    //st.tuner_id = tuner_id;

    fe_signal_info.tuner_id = tuner_id;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_signal_info)
    {
        MT_ERR_FRONTEND("Input parameter(p_signal_info) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

#if 0 // 20200902 disabled for Jiuzhou' reuqest
    ret = ioctl(g_fe_fd, FE_GET_STATUS_CMD, &st);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("GET_STATUS_CMD error\n");
        return MT_ERR_FE_FAILED_GETSTATUS;
    }

    if (MT_UNF_FE_SIGNAL_LOCKED != st.status.lock_status)
    {
        MT_ERR_FRONTEND("SIGNAL DROP\n");
        return MT_ERR_FE_NOT_CONNECT;
    }
#endif

    fe_signal_info.info.sig_type = g_current_fe_attr[tuner_id].sig_type;

    ret = ioctl(g_fe_fd, FE_GET_SIGNAL_INFO_CMD, &fe_signal_info);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner HI_UNF_TUNER_GetSignalInfo error\n");
        return MT_ERR_FE_FAILED_GETSIGNALINFO;
    }

    memcpy(p_signal_info, &fe_signal_info.info, sizeof(mt_unf_fe_signal_info_t));
    switch (p_signal_info->sig_type)
    {
    case MT_UNF_FE_SIG_TYPE_DVB_T:
    case MT_UNF_FE_SIG_TYPE_DVB_T2:
    case MT_UNF_FE_SIG_TYPE_ISDB_T:
    case MT_UNF_FE_SIG_TYPE_ATSC_T:
    case MT_UNF_FE_SIG_TYPE_DTMB:
        //p_signal_info->sig_info.ter.freq = 
        //    g_current_fe_connect_para[tuner_id].connect_param.ter.freq;
        //p_signal_info->sig_info.ter.band_width = 
        //    g_current_fe_connect_para[tuner_id].connect_param.ter.band_width;
        break;

    case MT_UNF_FE_SIG_TYPE_SAT:
    case MT_UNF_FE_SIG_TYPE_SAT_2:
        //p_signal_info->sig_info.sat.freq = 
        //    g_current_fe_connect_para[tuner_id].connect_param.sat.freq;
        //p_signal_info->sig_info.sat.symbol_rate = 
        //    g_current_fe_connect_para[tuner_id].connect_param.sat.sym_rate;
        p_signal_info->sig_info.sat.polar = 
            g_current_fe_connect_para[tuner_id].connect_param.sat.polarization;
        break;

    case MT_UNF_FE_SIG_TYPE_CAB:
    case MT_UNF_FE_SIG_TYPE_J83B:
        //p_signal_info->sig_info.cab.freq = 
        //    g_current_fe_connect_para[tuner_id].connect_param.cab.freq;
        //p_signal_info->sig_info.cab.symbol_rate = 
        //    g_current_fe_connect_para[tuner_id].connect_param.cab.sym_rate;
        //p_signal_info->sig_info.cab.mode_type = 
        //    g_current_fe_connect_para[tuner_id].connect_param.cab.mod_type;
        //p_signal_info->sig_info.cab.iq_mode = 
        //    g_current_fe_connect_para[tuner_id].connect_param.cab.b_reverse;
        break;

    case MT_UNF_FE_SIG_TYPE_DVBT_AUTO:
    case MT_UNF_FE_SIG_TYPE_DVBS_AUTO:
        break;

    /* Unsupport now */
    default:
        return MT_ERR_FE_FAILED_GETSIGNALINFO;
    }
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_lnb_config(mt_u32 tuner_id, mt_unf_fe_lnb_config_t *p_lnb)
{
#if 1
    /*mt_u32 local_freq; Clean Warning [-Wunused-variable]*/

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_lnb)
    {
        MT_ERR_FRONTEND("Input parameter(p_lnb) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    if (MT_UNF_FE_LNB_TYPE_BUTT <= p_lnb->lnb_type)
    {
        MT_ERR_FRONTEND("Input parameter(p_lnb->lnb_type) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_LNB_BAND_BUTT <= p_lnb->lnb_band)
    {
        MT_ERR_FRONTEND("Input parameter(p_lnb->lnb_band) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (MT_UNF_FE_LNB_SINGLE_FREQUENCY == p_lnb->lnb_type)
    {
        if (p_lnb->low_lo != p_lnb->high_lo)
        {
            MT_ERR_FRONTEND("Input parameter invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }
    }

    /*fix issue 133586*/
    /*if (p_lnb->low_lo > p_lnb->high_lo)
    {
        local_freq = p_lnb->low_lo;
        p_lnb->low_lo = p_lnb->high_lo;
        p_lnb->high_lo = local_freq;
    }*/

    if (MT_UNF_FE_LNB_BAND_C == p_lnb->lnb_band)
    {
        if ((p_lnb->low_lo > 7500) || (p_lnb->high_lo > 7500))
        {
            MT_ERR_FRONTEND("Invalid LO freq\n");
            return MT_ERR_FE_INVALID_PARA;
        }
    }
    else
    {
        if ((p_lnb->low_lo <= 7500) || (p_lnb->high_lo <= 7500))
        {
            MT_ERR_FRONTEND("Invalid LO freq\n");
            return MT_ERR_FE_INVALID_PARA;
        }
    }

    if (MT_UNF_FE_LNB_UNICABLE == p_lnb->lnb_type)
    {
        if ((MT_UNF_FE_LNB_BAND_KU != p_lnb->lnb_band) || 
            (7500 >= p_lnb->low_lo) ||
            (7500 >= p_lnb->high_lo))
        {
            MT_ERR_FRONTEND("Unicable only support Ku band!\n");
            return MT_ERR_FE_INVALID_PARA;
        }

        if (((7 < p_lnb->unicable_scr_no) && (p_lnb->unicable_ver_no != 2)) || (31 < p_lnb->unicable_scr_no))
        {
            MT_ERR_FRONTEND("Unicable support %d SCR max!\n", (p_lnb->unicable_ver_no == 2) ? 32 : 8);
            return MT_ERR_FE_INVALID_PARA;
        }

        if ((SAT_IF_MAX < p_lnb->unicable_if_freq_mhz) || 
            (SAT_IF_MIN > p_lnb->unicable_if_freq_mhz))
        {
            MT_ERR_FRONTEND("Unicable IF error!\n");
            return MT_ERR_FE_INVALID_PARA;
        }

        if (MT_UNF_FE_SATPOSN_BUT <= p_lnb->unicable_port_no)
        {
            MT_ERR_FRONTEND("Unicable position error!\n");
            return MT_ERR_FE_INVALID_PARA;
        }
    }

    g_sat_para[tuner_id].lnb_config = *p_lnb;

    if (p_lnb->lnb_agent_mode)
    {
        mt_u32 agent_tuner_id = p_lnb->lnb_agent_id;

        g_sat_para[agent_tuner_id].lnb_config = *p_lnb;
    }

#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_lnb_power(mt_u32 tuner_id, mt_unf_fe_lnb_power_t en_lnb_power)
{
    fe_lnb_out_t lnb_out;
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_UNF_FE_LNB_POWER_BUTT <= en_lnb_power)
    {
        MT_ERR_FRONTEND("Input parameter(p_lnb) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        lnb_out.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        lnb_out.port = tuner_id;
    }

    //lnb_out.port = tuner_id;

    switch (en_lnb_power)
    {
    case MT_UNF_FE_LNB_POWER_ON:
        lnb_out.voltage = TUNER_LNB_OUT_18V;
        break;

    case MT_UNF_FE_LNB_POWER_ENHANCED:
        lnb_out.voltage = TUNER_LNB_OUT_18V; //TUNER_LNB_OUT_19V;
        break;

    case MT_UNF_FE_LNB_POWER_OFF:
        lnb_out.voltage = TUNER_LNB_OUT_0V;
        break;

    default:
        return MT_ERR_FE_INVALID_PARA;
    }

    ret = ioctl(g_fe_fd, FE_SET_LNBOUT_CMD, &lnb_out);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set LNB power fail.\n");
        return MT_ERR_FE_FAILED_LNBCTRL;
    }

    g_sat_para[tuner_id].en_lnb_power = en_lnb_power;

#if 0
    fe_lnb_out_t lnb_out;
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", port);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_UNF_FE_LNB_POWER_BUTT <= en_lnb_power)
    {
        MT_ERR_FRONTEND("Input parameter(p_lnb) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        lnb_out.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        lnb_out.port = tuner_id;
    }

    //lnb_out.port = port;

    switch (en_lnb_power)
    {
    case MT_UNF_FE_LNB_POWER_ON:
        lnb_out.voltage = TUNER_LNB_OUT_18V;
        break;

    case MT_UNF_FE_LNB_POWER_ENHANCED:
        lnb_out.voltage = TUNER_LNB_OUT_19V;
        break;

    case MT_UNF_FE_LNB_POWER_OFF:
        lnb_out.voltage = TUNER_LNB_OUT_0V;
        break;

    default:
        return MT_ERR_FE_INVALID_PARA;
    }

    ret = ioctl(g_fe_fd, FE_SET_LNBOUT_CMD, &lnb_out);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set LNB power fail.\n");
        return MT_ERR_FE_FAILED_LNBCTRL;
    }

    g_sat_para[port].en_lnb_power = en_lnb_power;
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_lnb_onoff(mt_u32 tuner_id, mt_u8 on_off)
{
    fe_lnb_out_t lnb_out;
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        lnb_out.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        lnb_out.port = tuner_id;
    }

    //lnb_out.port = tuner_id;
    lnb_out.on_off = on_off;

    ret = ioctl(g_fe_fd, FE_SET_LNB_ONOFF_CMD, &lnb_out);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set LNB power fail.\n");
        return MT_ERR_FE_FAILED_LNBCTRL;
    }

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_polarization(mt_u32 tuner_id, mt_unf_fe_lnb_polar polar)
{
    fe_lnb_out_t lnb_out;
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        lnb_out.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        lnb_out.port = tuner_id;
    }

    //lnb_out.port = tuner_id;

    if (PORT_PORLAR_HORIZONTAL == polar)
    {
        lnb_out.voltage = TUNER_LNB_OUT_18V;
    }
    else
    {
        lnb_out.voltage = TUNER_LNB_OUT_13V;
    }

    if ((PORT_PORLAR_VERTICAL == polar) || (PORT_PORLAR_RIGHT == polar))
    {
        lnb_out.voltage = TUNER_LNB_OUT_13V;
    }
    else
    {
        lnb_out.voltage = TUNER_LNB_OUT_18V;
    }

    ret = ioctl(g_fe_fd, FE_SET_LNBOUT_CMD, &lnb_out);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set LNB power fail.\n");
        return MT_ERR_FE_FAILED_LNBCTRL;
    }

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_22k_onoff(mt_u32 tuner_id, mt_u8 on_off)
{
    fe_set_22k_onoff_t fe_onoff_22k = {0};
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        fe_onoff_22k.tuner_id = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        fe_onoff_22k.tuner_id = tuner_id;
    }

    //fe_onoff_22k.tuner_id = tuner_id;
    fe_onoff_22k.onoff = on_off;
    ret = ioctl(g_fe_fd, FE_SEND_CONTINUOUS_22K_CMD, &fe_onoff_22k);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set continuous 22K fail.\n");
        return MT_ERR_FE_FAILED_LNBCTRL;
    }

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_plpid(mt_u32 tuner_id, mt_u8 plp_id)
{
    fe_data_t fe_data = {0};
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = tuner_id;
    fe_data.data = plp_id;

    ret = ioctl(g_fe_fd, FE_SET_PLPNO_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set PLP ID fail.\n");
        return MT_ERR_FE_FAILED_SETPLPID;
    }

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_plp_mode(mt_u32 tuner_id, mt_u8 mode)
{
#if 0
    g_plp_mode = mode;
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_common_plpid(mt_u32 tuner_id, mt_u8 plp_id)
{
#if 1
    fe_data_t fe_data = {0};
    mt_s32 ret;

    if (!b_fe_open)
    {
        MT_ERR_FRONTEND("tuner not opened, tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_NOT_OPEN;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = tuner_id;
    fe_data.data = plp_id;

    ret = ioctl(g_fe_fd, FE_SET_COMMONPLP_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set Common PLP ID fail.\n");
        return MT_ERR_FE_FAILED_SETPLPID;
    }
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_common_plp_combination(mt_u32 tuner_id, mt_u8 plp_id)
{
#if 0
    fe_data_t fe_data = {0};
    mt_s32 ret;

    if (!b_fe_open)
    {
        MT_ERR_FRONTEND("tuner not opened, tuner_id is: %d\n", port);
        return MT_ERR_FE_NOT_OPEN;
    }

    if (UNF_TUNER_NUM <= port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", port);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = port;
    fe_data.data = plp_id;

    ret = ioctl(g_fe_fd, FE_SET_COMMONPLP_COMBINATION_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set Common PLP Combination fail.\n");
        return MT_ERR_FE_FAILED_SETPLPID;
    }
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_plpnum(mt_u32 tuner_id, mt_u8 *plp_num)
{
    fe_data_t fe_data = {0};
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (!b_fe_open)
    {
        MT_ERR_FRONTEND("tuner not opened, tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_NOT_OPEN;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == plp_num)
    {
        MT_ERR_FRONTEND("Input parameter(plp_num) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    fe_data.port = tuner_id;
    fe_data.data = 0;

    ret = ioctl(g_fe_fd, FE_GET_PLPNUM_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Get PLP NUM fail.\n");
        return MT_ERR_FE_FAILED_GETPLPNUM;
    }

    *plp_num = (mt_u8)fe_data.data;

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_current_plp_type(mt_u32 tuner_id, mt_unf_fe_t2_plp_type_t *p_plptype)
{
#if 0
    fe_data_t fe_data = {0};
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", port);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_plptype)
    {
        MT_ERR_FRONTEND("Input parameter(p_plp_type) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    fe_data.port = port;
    fe_data.data = 0;

    ret = ioctl(g_fe_fd, FE_GETCURPLPTYPE_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Get PLP Type fail.\n");
        return MT_ERR_FE_FAILED_GETPLPTYPE;
    }

    *p_plptype = (mt_unf_fe_t2_plp_type_t)fe_data.data;
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_plpid(mt_u32 tuner_id, mt_u8 *p_plp_id)
{
    fe_data_t fe_data = {0};
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (!b_fe_open)
    {
        MT_ERR_FRONTEND("tuner not opened, tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_NOT_OPEN;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_plp_id)
    {
        MT_ERR_FRONTEND("Input parameter(p_plp_id) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    fe_data.port = tuner_id;

    ret = ioctl(g_fe_fd, FE_GET_PLP_ID_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Get PLP id fail.\n");
        return MT_ERR_FE_FAILED_GETPLPTYPE;
    }

    *p_plp_id = (mt_u8)fe_data.data;

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_plp_grpid(mt_u32 tuner_id, mt_u8 *p_plp_grpid)
{
#if 0
    fe_data_t fe_data = {0};
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", port);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_plp_grpid)
    {
        MT_ERR_FRONTEND("Input parameter(p_plp_grpid) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    fe_data.port = port;

    ret = ioctl(g_fe_fd, FE_GET_GROUP_PLP_ID_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Get PLP group id fail.\n");
        return MT_ERR_FE_FAILED_GETPLPTYPE;
    }

    *p_plp_grpid = (mt_u8)fe_data.data;
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_hierarchy_num(mt_u32 tuner_id, mt_u8 *p_hier_num)
{
    fe_data_t fe_data = {0};
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (!b_fe_open)
    {
        MT_ERR_FRONTEND("tuner not opened, tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_NOT_OPEN;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_hier_num)
    {
        MT_ERR_FRONTEND("Input parameter(plp_num) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    fe_data.port = tuner_id;
    fe_data.data = 0;

    ret = ioctl(g_fe_fd, FE_GET_HIERARCHY_NUM_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Get HIERARCHY NUM failed.\n");
        return MT_ERR_FE_FAILED_GETPLPNUM;
    }

    *p_hier_num = (mt_u8)fe_data.data;

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_hierarchy_id(mt_u32 tuner_id, mt_u8 hier_id)
{
    fe_data_t fe_data = {0};
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = tuner_id;
    fe_data.data = hier_id;

    ret = ioctl(g_fe_fd, FE_SET_HIERARCHY_ID_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set HIERARCHY ID failed.\n");
        return MT_ERR_FE_FAILED_SETPLPID;
    }

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_hierarchy_id(mt_u32 tuner_id, mt_u8 *p_hier_id)
{
    fe_data_t fe_data = {0};
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (!b_fe_open)
    {
        MT_ERR_FRONTEND("tuner not opened, tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_NOT_OPEN;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_hier_id)
    {
        MT_ERR_FRONTEND("Input parameter(p_hier_id) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    fe_data.port = tuner_id;

    ret = ioctl(g_fe_fd, FE_GET_PLP_ID_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Get PLP id fail.\n");
        return MT_ERR_FE_FAILED_GETPLPTYPE;
    }

    *p_hier_id = (mt_u8)fe_data.data;

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_t_t2_cell_id(mt_u32 tuner_id, mt_u16 *p_cell_id)
{
    fe_data_t fe_data = {0};
    mt_s32 ret;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid, tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_cell_id)
    {
        MT_ERR_FRONTEND("Input parameter(p_cell_id) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    fe_data.port = tuner_id;

    ret = ioctl(g_fe_fd, FE_GET_T_T2_CELL_ID_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Get cell id fail.\n");
        return MT_ERR_FE_FAILED_GETPLPTYPE;
    }

    *p_cell_id = (mt_u16)(fe_data.data);

    return MT_SUCCESS;
}

/*set lnb on_off*/
mt_s32 mt_unf_fe_set_antenna_power(mt_u32 tuner_id, mt_unf_fe_ter_antenna_power_t en_power)
{
    mt_s32 ret = MT_FAILURE;
#if 1 // 180601
    fe_data_t fe_data = {0};
    mt_u32 port = tuner_id;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", port);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = port;
    fe_data.data = (mt_u32)en_power;
    g_ter_para[port].en_antenna_power = en_power;

    ret = ioctl(g_fe_fd, FE_SET_ANTENNA_POWER_CMD, &fe_data);
#endif

    return ret;
}

mt_s32 mt_unf_fe_get_antenna_power(mt_u32 tuner_id, mt_unf_fe_ter_antenna_power_t *p_en_power)
{
#if 1
    mt_u32 port = tuner_id;

    if (UNF_TUNER_NUM <= port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", port);
        return MT_ERR_FE_INVALID_PORT;
    }

    *p_en_power = g_ter_para[port].en_antenna_power;
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_blindscan_start(mt_u32 tuner_id, const mt_unf_fe_blindscan_para_t *p_blindscan)
{
#if 1
    static blindscan_para_t blindscan;
    //fe_blindscan_param_t bs_param;
    //fe_blindscan_info_t bs_info;

    mt_s32 ret = MT_FAILURE;

    //printf("---blind start--line:%d-------mode=%d-------\n", __LINE__, p_blindscan->mode);

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_blindscan)
    {
        MT_ERR_FRONTEND("Input parameter(p_terscan) invalid\n");
        return MT_ERR_FE_INVALID_POINT;
    }

    /* DVB-S/S2 Blindscan */
    if (((MT_UNF_FE_SIG_TYPE_SAT == g_current_fe_attr[tuner_id].sig_type) || 
         (MT_UNF_FE_SIG_TYPE_SAT_2 == g_current_fe_attr[tuner_id].sig_type) || 
         ((MT_UNF_FE_SIG_TYPE_SAT | MT_UNF_FE_SIG_TYPE_SAT_2) == g_current_fe_attr[tuner_id].sig_type)) || 
        (MT_UNF_DEMOD_DEV_TYPE_M88DM6K == g_current_fe_attr[tuner_id].demod_dev_type) || 
        (MT_UNF_DEMOD_DEV_TYPE_M88RS6060 == g_current_fe_attr[tuner_id].demod_dev_type) || 
        (MT_UNF_DEMOD_DEV_TYPE_M88DS6103 == g_current_fe_attr[tuner_id].demod_dev_type) || 
        (MT_UNF_DEMOD_DEV_TYPE_M88DS6113 == g_current_fe_attr[tuner_id].demod_dev_type) || 
        (MT_UNF_DEMOD_DEV_TYPE_M88CT8K == g_current_fe_attr[tuner_id].demod_dev_type) || 
        (MT_UNF_DEMOD_DEV_TYPE_M88CS8800 == g_current_fe_attr[tuner_id].demod_dev_type)) // 20200827
    {
        if (MT_UNF_FE_BLINDSCAN_MODE_BUTT <= p_blindscan->mode)
        {
            MT_ERR_FRONTEND("Input parameter(mode) invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }

        ///*
        if (MT_NULL == p_blindscan->scan_para.sat.scan_notify)
        {
            MT_ERR_FRONTEND("Input parameter(pfnEVTNotify) invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }
        //*/

        if (MT_UNF_FE_BLINDSCAN_MODE_MANUAL == p_blindscan->mode)
        {
            if (MT_UNF_FE_POLARIZATION_BUTT <= p_blindscan->scan_para.sat.polar)
            {
                MT_ERR_FRONTEND("Input parameter(en_polar) invalid\n");
                return MT_ERR_FE_INVALID_PARA;
            }

            if (MT_UNF_FE_LNB_22K_BUTT <= p_blindscan->scan_para.sat.lnb_22k)
            {
                MT_ERR_FRONTEND("Input parameter(en_lnb_22k) invalid\n");
                return MT_ERR_FE_INVALID_PARA;
            }

            if ((SAT_IF_MIN_KHZ > p_blindscan->scan_para.sat.start_freq) || (SAT_IF_MAX_KHZ < p_blindscan->scan_para.sat.start_freq))
            {
                MT_ERR_FRONTEND("Input parameter(u32StartFreq) invalid\n");
                return MT_ERR_FE_INVALID_PARA;
            }

            if ((SAT_IF_MIN_KHZ > p_blindscan->scan_para.sat.stop_freq) || (SAT_IF_MAX_KHZ < p_blindscan->scan_para.sat.stop_freq))
            {
                MT_ERR_FRONTEND("Input parameter(u32StopFreq) invalid\n");
                return MT_ERR_FE_INVALID_PARA;
            }

            if (p_blindscan->scan_para.sat.stop_freq <= p_blindscan->scan_para.sat.start_freq)
            {
                MT_ERR_FRONTEND("Input parameter(u32StopFreq) invalid\n");
                return MT_ERR_FE_INVALID_PARA;
            }

            //printf("---blind start--line:%d-------g_fe_fd=%d-------\n", __LINE__, g_fe_fd);
        }

#if 1
        if (MT_NULL != g_sat_para[tuner_id].p_bs_monitor)
        {
            if (g_sat_para[tuner_id].blindscan_busy) //is blinding scan
            {
                MT_ERR_FRONTEND("Blind scan busy.\n");
                return MT_ERR_FE_FAILED_BLINDSCAN;
            }
            else //blind scan finish or cancel
            {
                (mt_void) pthread_join(*g_sat_para[tuner_id].p_bs_monitor, MT_NULL);
                //free(g_sat_para[port].p_bs_monitor);
                mt_free(MT_ID_FRONTEND, g_sat_para[tuner_id].p_bs_monitor);
                g_sat_para[tuner_id].p_bs_monitor = MT_NULL;
            }
        }

        g_sat_para[tuner_id].p_bs_monitor = (pthread_t *)mt_malloc(MT_ID_FRONTEND, sizeof(pthread_t));
        if (MT_NULL == g_sat_para[tuner_id].p_bs_monitor)
        {
            MT_ERR_FRONTEND("No memory.\n");
            return MT_ERR_FE_FAILED_BLINDSCAN;
        }
#endif

        blindscan.tuner_id = tuner_id;
        blindscan.scan_info = *p_blindscan;

#if 1
        //printf("\n usr blind start line:%d g_fe_fd=%d\n", __LINE__, g_fe_fd);
        ret = pthread_create(g_sat_para[tuner_id].p_bs_monitor, 0, fe_dvbs_blindscan_thread, &blindscan);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("Create pthread fail.\n");
            if (g_sat_para[tuner_id].p_bs_monitor)
            {
                mt_free(MT_ID_FRONTEND, g_sat_para[tuner_id].p_bs_monitor);
                g_sat_para[tuner_id].p_bs_monitor = MT_NULL;
            }

            g_sat_para[tuner_id].blindscan_stop = MT_FALSE;
            return MT_ERR_FE_FAILED_BLINDSCAN;
        }

        g_sat_para[tuner_id].blindscan_stop = MT_FALSE;

        /*wait thread until thread finish*/
        //(mt_void) pthread_join(*g_sat_para[tuner_id].p_bs_monitor, MT_NULL);
#endif
    }
    else
    {
        MT_ERR_FRONTEND("%s[%d] -- Demod type[%d]. Error signal type[%d]!!\n", __FUNCTION__, __LINE__, 
                         g_current_fe_attr[tuner_id].demod_dev_type, 
                         g_current_fe_attr[tuner_id].sig_type);

        return MT_ERR_FE_INVALID_PARA;
    }
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_blindscan_stop(mt_u32 tuner_id)
{
#if 1
    mt_s32 ret = MT_FAILURE;
    mt_u32 timeout = 100;

    /*
    fe_data_t fe_data = { 0 };
    fe_data.port = port;
    fe_data.data = 0;
    */
    /* Reserve */

    CHECK_TUNER_OPEN();

    //printf("[%s %d]enter\n", __FUNCTION__, __LINE__);
    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL != g_sat_para[tuner_id].p_bs_monitor)
    {
        //cancel fe_dvbs_blindscan_thread
        g_sat_para[tuner_id].blindscan_stop = MT_TRUE;
        //printf("[%s %d]g_sat_para[tuner_id].blindscan_stop=%d\n", __FUNCTION__, __LINE__, g_sat_para[tuner_id].blindscan_stop);

        //in fe_dvbs_blindscan_thread will set g_sat_para[tuner_id].blindscan_stop to FALSE
        //max wait time is 1s
        while ((MT_TRUE == g_sat_para[tuner_id].blindscan_stop) && (timeout != 0))
        {
            usleep(10 * 1000);

            if (timeout > 10)
            {
                timeout = timeout - 10;
            }
            else
            {
                timeout = 0;
            }
        }

        //printf("[%s %d]timeout=%d\n", __FUNCTION__, __LINE__, timeout);

        //cancel drv's thread after fe_dvbs_blindscan_thread has been canceled
        //this will ensure FE_BLIND_SCAN_CANCEL_CMD not execute before FE_BLINDSCAN_ACTION_CMD
        ret = ioctl(g_fe_fd, FE_BLIND_SCAN_CANCEL_CMD, &tuner_id);
        if (ret != MT_SUCCESS)
        {
            return MT_FAILURE;
        }

        (mt_void) pthread_join(*g_sat_para[tuner_id].p_bs_monitor, MT_NULL);
        //free(g_sat_para[port].p_bs_monitor);
        mt_free(MT_ID_FRONTEND, g_sat_para[tuner_id].p_bs_monitor);
        g_sat_para[tuner_id].p_bs_monitor = MT_NULL;
    }
#endif
    //printf("[%s %d]leave\n", __FUNCTION__, __LINE__);

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_ter_scan_start(mt_u32 tuner_id, mt_unf_fe_ter_scan_para_t *p_terscan)
{
#if 1 // 180601
    mt_s32 ret = MT_FAILURE;
    static terscan_para_t ter_scan;
    mt_u32 port = tuner_id;

    if (MT_UNF_FE_SIG_TYPE_DVB_T == g_current_fe_attr[port].sig_type || MT_UNF_FE_SIG_TYPE_DVB_T2 == g_current_fe_attr[port].sig_type)
    {
        if ((p_terscan->ter.freq < TER_RF_MIN) || (p_terscan->ter.freq > TER_RF_MAX))
        {
            MT_ERR_FRONTEND("Input parameter(freq) invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }

        if ((p_terscan->ter.band_width < TER_BW_MIN) || (p_terscan->ter.band_width > TER_BW_MAX))
        {
            MT_ERR_FRONTEND("Input parameter(band_width) invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }

        if (p_terscan->ter.scan_mode >= MT_UNF_FE_TER_SCAN_DVB_T_T2_BUTT)
        {
            MT_ERR_FRONTEND("Input parameter(enScanMode) invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }

        if (p_terscan->ter.scan_lite > 1)
        {
            MT_ERR_FRONTEND("Input parameter(u8ScanLite) invalid\n");
            return MT_ERR_FE_INVALID_PARA;
        }

        if (MT_NULL != g_ter_para[port].p_bs_monitor)
        {
            if (g_ter_para[port].bs_busy)
            {
                MT_ERR_FRONTEND("Blind scan busy.\n");
                return MT_ERR_FE_FAILED_BLINDSCAN;
            }
            else
            {
                (mt_void) pthread_join(*g_ter_para[port].p_bs_monitor, MT_NULL);
                free(g_ter_para[port].p_bs_monitor);
                g_ter_para[port].p_bs_monitor = MT_NULL;
            }
        }

        g_ter_para[port].p_bs_monitor = (pthread_t *)malloc(sizeof(pthread_t));
        if (MT_NULL == g_ter_para[port].p_bs_monitor)
        {
            MT_ERR_FRONTEND("No memory.\n");
            return MT_ERR_FE_FAILED_BLINDSCAN;
        }

        memset(p_terscan->chan_array, 0, sizeof(p_terscan->chan_array));
        ter_scan.port = port;
        ter_scan.s_para = *p_terscan;

        ret = pthread_create(g_ter_para[port].p_bs_monitor, 0, fe_dvbt2_scan_thread, &ter_scan);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("Create pthread fail.\n");
            if (g_ter_para[port].p_bs_monitor)
            {
                free(g_ter_para[port].p_bs_monitor);
                g_ter_para[port].p_bs_monitor = MT_NULL;
            }

            g_ter_para[port].bs_stop = MT_FALSE;
            return MT_ERR_FE_FAILED_BLINDSCAN;
        }

        (mt_void) pthread_join(*g_ter_para[port].p_bs_monitor, MT_NULL);
        free(g_ter_para[port].p_bs_monitor);
        g_ter_para[port].p_bs_monitor = MT_NULL;

        g_ter_para[port].bs_stop = MT_FALSE;
    }
    else
    {
        MT_ERR_FRONTEND("Error signal type!\n");
        return MT_ERR_FE_INVALID_PARA;
    }
    return ret;
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_ter_scan_stop(mt_u32 tuner_id)
{
#if 1 // 180601
    mt_u32 port = tuner_id;

    /*fe_data_t fe_data = { 0 };

    fe_data.port = port;
    fe_data.data = 0;*/
    /* Reserve */

    if (UNF_TUNER_NUM <= port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", port);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL != g_ter_para[port].p_bs_monitor)
    {
        g_ter_para[port].bs_stop = MT_TRUE;
        (mt_void) pthread_join(*g_ter_para[port].p_bs_monitor, MT_NULL);
        mt_free(MT_ID_FRONTEND, g_ter_para[port].p_bs_monitor);
        g_ter_para[port].p_bs_monitor = MT_NULL;
    }
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_switch_22k(mt_u32 tuner_id, mt_unf_fe_switch_22k_t switch_port)
{
#if 1
    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_UNF_FE_SWITCH_22K_BUTT <= switch_port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    /* Save */
    g_sat_para[tuner_id].en_switch_22k = switch_port;

    if (MT_UNF_FE_SWITCH_22K_0 == switch_port)
    {
        return fe_diseqc_send_22k(tuner_id, MT_FALSE);
    }
    else if (MT_UNF_FE_SWITCH_22K_22 == switch_port)
    {
        return fe_diseqc_send_22k(tuner_id, MT_TRUE);
    }
    else
    {
        return MT_SUCCESS;
    }
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_switch_0_12v(mt_u32 tuner_id, mt_unf_fe_switch_0_12v_t switch_port)
{
#if 1
    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_UNF_FE_SWITCH_0_12V_BUTT <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_switch_toneburst(mt_u32 tuner_id, mt_unf_fe_switch_toneburst_t status)
{
#if 1
    fe_data_t fe_data;
    mt_s32 ret = MT_FAILURE;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(tuner_id) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_UNF_FE_SWITCH_TONEBURST_BUTT <= status)
    {
        MT_ERR_FRONTEND("Input parameter(status) invalid\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    /* Save */
    g_sat_para[tuner_id].en_toneburst = status;

    /* Send tone */
    if (g_sat_para[tuner_id].lnb_config.lnb_agent_mode)
    {
        fe_data.port = g_sat_para[tuner_id].lnb_config.lnb_agent_id;
    }
    else
    {
        fe_data.port = tuner_id;
    }

    //fe_data.port = tuner_id;
    if ((MT_UNF_FE_SWITCH_TONEBURST_0 == status) || (MT_UNF_FE_SWITCH_TONEBURST_1 == status))
    {
        fe_data.data = status - 1;
        (mt_void) fe_diseqc_stop_22k(tuner_id);
        usleep(DISEQC_DELAY_TIME_MS * 1000);

        ret = ioctl(g_fe_fd, FE_SEND_TONE_CMD, &fe_data);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FRONTEND("Set tone burst fail.\n");
            return MT_ERR_FE_FAILED_DISEQC;
        }

        usleep(DISEQC_DELAY_TIME_MS * 1000);
        (mt_void) fe_diseqc_resume_22k(tuner_id);
    }
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_standby(mt_u32 tuner_id)
{
    int ret = 0;

    //if (init_cnt == 0)
    //return MT_UNF_TUNER_ERR_UNINIT;

    if (tuner_id >= UNF_TUNER_NUM)
        return MT_UNF_TUNER_ERR_ID_OVER;

    //if (g_fe_fd_cnt == 0)
    //return MT_UNF_TUNER_ERR_UNOPEN;

    //if (tuner_info[tuner_id].attr_set_cnt == 0)
    //    return MT_UNF_TUNER_ERR_ATTR_UNSET;

    ret = ioctl(g_fe_fd, FE_STANDBY_CMD, &tuner_id);
    if (ret < 0)
        return MT_UNF_TUNER_ERR_STANDBY_FAIL;

    return MT_SUCCESS;

#if 0
    fe_data_t fe_data;
    mt_s32 ret = MT_FAILURE;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", port);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = port;
    fe_data.data = 1;

    ret = ioctl(g_fe_fd, FE_STANDBY_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner standby fail.\n");
        return MT_ERR_FE_FAILED_STANDBY;
    }

    return MT_SUCCESS;
#endif
}

mt_s32 mt_unf_fe_wakeup(mt_u32 tuner_id)
{
    int ret = 0;

    //if (init_cnt == 0)
    //    return MT_UNF_TUNER_ERR_UNINIT;

    //if (tuner_id >= MT_UNF_FE_MAX_NUM)
	if (tuner_id >= UNF_TUNER_NUM)	
        return MT_UNF_TUNER_ERR_ID_OVER;

    //if (g_fe_fd_cnt == 0)
    //    return MT_UNF_TUNER_ERR_UNOPEN;

    //if (tuner_info[tuner_id].attr_set_cnt == 0)
    //    return MT_UNF_TUNER_ERR_ATTR_UNSET;

    ret = ioctl(g_fe_fd, FE_WAKEUP_CMD, &tuner_id);
    if (ret < 0)
        return MT_UNF_TUNER_ERR_WAKEUP_FAIL;

    return MT_SUCCESS;

#if 0
    fe_data_t fe_data;
    mt_s32 ret = MT_FAILURE;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", port);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = port;
    fe_data.data = 0;

    ret = ioctl(g_fe_fd, FE_STANDBY_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner wake up fail.\n");
        return MT_ERR_FE_FAILED_WAKEUP;
    }

    return MT_SUCCESS;
#endif
}

mt_s32 mt_unf_fe_set_ts_out(mt_u32 tuner_id, mt_unf_fe_ts_out_t *p_tsout)
{
#if 0
    fe_data_t fe_data;
    mt_s32 ret = MT_FAILURE;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= port)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", port);
        return MT_ERR_FE_INVALID_PORT;
    }

    if (MT_NULL == p_tsout)
    {
        MT_ERR_FRONTEND("penTSOUT is NULL\n");
        return MT_ERR_FE_INVALID_PARA;
    }

    fe_data.port = port;
    fe_data.data = (mt_u32)p_tsout;

    ret = ioctl(g_fe_fd, FE_SETTSOUT_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner set ts out fail.\n");
        return MT_ERR_FE_FAILED_SETTSOUT;
    }
#endif

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_constellation_data(mt_u32 tuner_id, mt_unf_fe_sample_datalen_t data_len, mt_unf_fe_sample_data_t *p_data)
{
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_spectrum_data(mt_u32 tuner_id, mt_unf_fe_sample_datalen_t data_len, mt_u32 *p_data)
{
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_agc(mt_u32 tuner_id, mt_s32 center_freq, mt_s32 *p_agc)
{
#if 1
    fe_databuf_t fe_databuf = {0};
    mt_s32 ret;

    if (!b_fe_open)
    {
        MT_ERR_FRONTEND("tuner not opened, tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_NOT_OPEN;
    }

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port) invalid,invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_databuf.port = tuner_id;
    //fe_databuf.databuf[0] = (mt_u32)center_freq;

    ret = ioctl(g_fe_fd, FE_GET_AGC_CMD, &fe_databuf);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("get agc fail.\n");
        return MT_FAILURE;
    }

    *p_agc = (mt_s32)fe_databuf.databuf[0];
#endif

    //*p_agc = 0;

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_unicable_power_off(mt_u32 tuner_id, mt_u8 scr_no)
{
    mt_s32 ret = 0;
    ret = unicable_power_off(tuner_id, scr_no);
    return ret;
}

mt_s32 mt_unf_fe_unicable_scrx_on(mt_u32 tuner_id)
{
    mt_s32 ret = 0;
    ret = unicable_scrx_signal_on(tuner_id);

    return ret;
}

mt_s32 mt_unf_fe_unicable_config(mt_u32 tuner_id, mt_u8 scr_no, mt_u8 app_no)
{
    mt_s32 ret = 0;
    //ret = unicable_config(tuner_id, scr_no, app_no);
    return ret;
}

mt_s32 mt_unf_fe_unicable_lofreq(mt_u32 tuner_id, mt_u8 scr_no, mt_u8 lofreq_no)
{
    mt_s32 ret = 0;
    ret = unicable_lofrq(tuner_id, scr_no, lofreq_no);
    return ret;
}

mt_s32 mt_unf_fe_switch_super_search_mode(mt_u32 tuner_id, mt_u32 super_search_mode)
{
    mt_s32 ret = 0;
    fe_data_t fe_data;

    fe_data.port = tuner_id;
    fe_data.data = super_search_mode;

    ret = ioctl(g_fe_fd, FE_SET_SUPER_SEARCH_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner[%d] switch super search mode to %d failed.\n", tuner_id, super_search_mode);
        return MT_FAILURE;
    }

    MT_ERR_FRONTEND("Tuner[%d] switch super search mode to %d OK.\n", tuner_id, super_search_mode);

    return ret;
}

mt_s32 mt_unf_fe_unicable_retry(mt_u32 tuner_id)
{
    mt_s32 ret = 0;
    fe_data_t fe_data;

    fe_data.port = tuner_id;
    fe_data.data = 0;

    ret = ioctl(g_fe_fd, FE_SET_UNICABLE_RETRY_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner[%d] unicable retry failed.\n", tuner_id);
        return MT_FAILURE;
    }

    MT_ERR_FRONTEND("Tuner[%d] unicable retry OK.\n", tuner_id);

    return ret;
}

mt_s32 mt_unf_fe_get_tuner_dbg_info(mt_u32 tuner_id, mt_u32 data)
{
    fe_data_t fe_data;
    mt_s32 ret = MT_FAILURE;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port)invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = tuner_id;
    fe_data.data = (mt_u32)data;

    ret = ioctl(g_fe_fd, FE_GET_TN_DBG_INFO_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner get dbg info failed.\n");
        return MT_FAILURE;
    }

    return ret;
}

mt_s32 mt_unf_fe_suspend(mt_u32 tuner_id)
{
    fe_data_t fe_data;
    mt_s32 ret = MT_FAILURE;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port)invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = tuner_id;

    ret = ioctl(g_fe_fd, FE_SUSPEND_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        //MT_ERR_FRONTEND("Tuner suspend failed.\n");
        return MT_FAILURE;
    }

    MT_ERR_FRONTEND("%s[%d][tuner %d]---- OK!\n", __FUNCTION__, __LINE__, tuner_id);

    return ret;
}

mt_s32 mt_unf_fe_resume(mt_u32 tuner_id)
{
    fe_data_t fe_data;
    mt_s32 ret = MT_FAILURE;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port)invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = tuner_id;

    ret = ioctl(g_fe_fd, FE_RESUME_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        //MT_ERR_FRONTEND("Tuner resume failed.\n");
        return MT_FAILURE;
    }

    MT_ERR_FRONTEND("%s[%d][tuner %d]---- OK!\n", __FUNCTION__, __LINE__, tuner_id);

    return ret;
}

mt_s32 mt_unf_fe_suspend_all(void)
{
    mt_u8 tuner_id = 0;

    for (tuner_id = 0; tuner_id < UNF_TUNER_NUM; tuner_id++)
    {
        mt_unf_fe_suspend(tuner_id);
    }

    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_resume_all(void)
{
    mt_u8 tuner_id = 0;

    for (tuner_id = 0; tuner_id < UNF_TUNER_NUM; tuner_id++)
    {
        mt_unf_fe_resume(tuner_id);
    }

    return MT_SUCCESS;
}
mt_s32 mt_unf_fe_set_lowpower(mt_u32 tuner_id,mt_unf_fe_sig_type_t sig_type)
{
    fe_data_t fe_data;
    mt_s32 ret = MT_FAILURE;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port)invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = tuner_id;
    fe_data.data = (mt_u32)sig_type;

    ret = ioctl(g_fe_fd, FE_SET_LOWPOWER_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Tuner set_lowpower failed.\n");
        return MT_FAILURE;
    }

    MT_ERR_FRONTEND("%s[%d][tuner %d]---- OK!\n", __FUNCTION__, __LINE__, tuner_id);

    return ret;    
}

mt_s32 mt_unf_fe_dss_filter_scid(mt_unf_fe_dss_scid_filter_t  *scid_filter)
{
    int ret = 0;
	
    if(scid_filter == NULL)
    {
	  MT_ERR_FRONTEND("scid_filter is null.\n");
        return MT_FAILURE;
    }

    printf("[%s : %d][tuner %d]---- scid=0x%x, 0x%x \n", __FUNCTION__, __LINE__, 
			scid_filter->tuner_id,scid_filter->u16_scid[0], scid_filter->u16_scid[1]);


    if (UNF_TUNER_NUM <= scid_filter->tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port)invalid, invalid tuner_id is: %d\n", scid_filter->tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    ret = ioctl(g_fe_fd, FE_DSS_FILTER_CMD, scid_filter);
    if (MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }


    return ret;
}

mt_s32 mt_unf_fe_set_bbframe_padding_onoff(mt_u32 tuner_id,mt_unf_fe_bbframe_packing_type_t bbframe_packing_type)
{
    fe_data_t fe_data;
    mt_s32 ret = MT_FAILURE;

    CHECK_TUNER_OPEN();

    if (UNF_TUNER_NUM <= tuner_id)
    {
        MT_ERR_FRONTEND("Input parameter(port)invalid, invalid tuner_id is: %d\n", tuner_id);
        return MT_ERR_FE_INVALID_PORT;
    }

    fe_data.port = tuner_id;
    fe_data.data = (mt_u32)bbframe_packing_type;

    ret = ioctl(g_fe_fd, FE_SET_BBFARME_PADDING_CMD, &fe_data);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_FRONTEND("Set_bbframe_padding failed.\n");
        return MT_FAILURE;
    }

    MT_DBG_FRONTEND("[tuner %d]---- OK!\n", tuner_id);

    return ret;    
}
#endif

