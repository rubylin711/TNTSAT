/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MSS_CMD_UTILS_H__
#define __MSS_CMD_UTILS_H__


#define PLAT_LINUX

//Only for secure cpu enabled
//#define SCPU_EN


#ifndef MT_SUCCESS
#define MT_SUCCESS      (0)
#endif
#ifndef MT_FAILURE
#define MT_FAILURE      (-1)
#endif

/* platform definitions */
#ifdef PLAT_LINUX

#define mss_print   printf
//typedef clock_t mss_clock_t;

//typedef pthread_mutex_t mss_mutex_t;

//void mss_mutex_create(mss_mutex_t *mutex);
//void mss_mutex_destroy(mss_mutex_t *mutex);

#else   //PLAT_UCOS

#include "mtos_printk.h"
#define mss_print   OS_PRINTF
typedef double mss_clock_t;
typedef void (*mss_thread_func_t)(void *varg);
double mss_clock(void);

typedef os_sem_t mss_mutex_t;

inline void mss_mutex_create(mss_mutex_t *mutex)
{
    mtos_sem_create(mutex, TRUE);
}

inline void mss_mutex_destroy(mss_mutex_t *mutex)
{
    mtos_sem_destroy(mutex, TRUE);
}

#define mss_mutex_lock(x)     mtos_sem_take(x, 0)    
#define mss_mutex_unlock(x)   mtos_sem_give(x)    


#endif

#define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x[0]))

typedef enum _MSS_CPU_MODE_E {
    MSS_APCPU = 0,
    MSS_SECCPU = 1,
} MSS_CPU_MODE_E;

void mss_dump(const char *tag, unsigned char *buffer, unsigned int len);

//void dump(const char *tag, unsigned char *buffer, unsigned int len);
//void print_time(const char *func_name, const char *tag, mss_clock_t time);

void test_count_init(void);
//void check_result(const char *tag, unsigned char *expect, unsigned char *result, unsigned int size, mss_clock_t elapsed);
void test_count_show(void);

void HexStrToByte(const char* source, unsigned char* dest, unsigned int *len);

#define ALGO_AES        (0)
#define ALGO_TDES       (1)
#define ALGO_MAX        (2)

#define OP_DEC          (0)
#define OP_ENC          (1)
#define OP_MAX          (2)

#ifdef SCPU_EN  //only CH2, or CH3

//CH_2 used by hash ?
#define WORK_CHANNEL_ID    MT_CIPHER_CRYPTO_CH_2 

#else //CH1,or CH0

#define WORK_CHANNEL_ID    MT_CIPHER_CRYPTO_CH_1

#endif

/* per-thread processing count */
#define PROCESSING_COUNT    (100)
#define THR_ADES_ON         (1)
#define THR_HASH_ON         (1)
#define THR_HMAC_ON         (1)
#define THR_RSA_ON          (1)

#ifdef SCPU_EN
enum {
    SEE_MB_TYPE_INVALID = 0,
    SEE_MB_TYPE_TLV_HEAD,
    SEE_MB_TYPE_TLV_CUR,
    SEE_MB_TYPE_KEYSLOT,
    SEE_MB_TYPE_INDEX,
    SEE_MB_TYPE_SCKNUM,
    SEE_MB_TYPE_KDATA,
    SEE_MB_TYPE_IVDATA,
    SEE_MB_TYPE_KATTR,
    SEE_MB_TYPE_INPUTDATA,
    SEE_MB_TYPE_OUTPUTDDATA,
    SEE_MB_TYPE_CHECK,

    SEE_MB_TYPE_MAX = 255,
};

#define SEE_MB_MSGLEN_MAX   (1024)

/*Command type:DO NOT modify the values unless you know what it is!*/
enum {
    MT_MB_COMMAND_INVALID = 0,
    MT_MB_COMMAND_SEND_DATA = 0x1,
    MT_MB_COMMAND_SET_CLEARKEY = 0x2,
    MT_MB_COMMAND_SET_KEYLADDER = 0x3,
    MT_MB_COMMAND_LOAD_HDCP_KEY = 0x10,
};

/*keyslot definition: 0~47 for APCPU*/
#define AP_KEYSLOT_ID_INVALID       (48)
#define AP_KEYSLOT_ID_MAX           (48)

typedef enum AP_Keyslot_Status {
    AP_KEYSLOT_FREE = 0,
    AP_KEYSLOT_BUSY = 1,
} AP_Keyslot_Status_e;

#endif
#endif
