/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _PURIN_SYS_SND_H_
#define _PURIN_SYS_SND_H_

enum {
    LASTEST_CAPTURE_INVALID_SIZE = -1,
    LASTEST_CAPTURE_REQUEST = 0,
    LASTEST_CAPTURE_WAITING,
    LASTEST_CAPTURE_DONE
};

typedef struct {
    u8 * output_buf;
    u32 output_size;
    u32 status;
    ktime_t req_ktime;
    unsigned long wakelock_jiffies;
} lastest_capture_info;

#define AT_LEAST_WAITING_US 8500

#endif
