/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __SYMPHONY_USBCAM_ROUTE_TS_H_
#define __SYMPHONY_USBCAM_ROUTE_TS_H_

//out
#define CI_MAX_CHANNEL 32

#define MT_ERR_USBCAM_ROUTE_MEMORY (s32)(0x80800001)
#define MT_ERR_USBCAM_ROUTE_TASK_ERR (s32)(0x80800002)
#define MT_ERR_USBCAM_ROUTE_PID_OVERFLOW (s32)(0x80800003)
#define MT_ERR_USBCAM_ROUTE_ALREADY_START (s32)(0x80800004)
#define MT_ERR_USBCAM_ROUTE_ALREADY_STOP (s32)(0x80800005)
#define MT_ERR_USBCAM_ROUTE_PARAM_ERR (s32)(0x80800006)
#define MT_ERR_USBCAM_ROUTE_OPEN_DEV_ERR (s32)(0x80800007)
#define MT_ERR_USBCAM_ROUTE_GET_PROGRAM_ERR (s32)(0x80800008)
#define MT_ERR_USBCAM_ROUTE_ATTACH_ERR (s32)(0x80800009)


#define PAT_PID 0


#define DMX_0 0
#define DMX_1 1
#define BOOL int

#define USBCAM_GET_REC_PRINT(fmt,arg...)        //printf(fmt,##arg)
#define USBCAM_PUSH_DATA_PRINT(fmt,arg...)      //printf(fmt,##arg)
#define USBCAM_ERROR_PRINT(fmt,arg...)          printf(fmt,##arg)
#define USBCAM_INFO_PRINT(fmt,arg...)           //printf(fmt,##arg)
#define USBCAM_WARN_PRINT(fmt,arg...)           //printf(fmt,##arg)
#define USBCAM_DEBUG_PRINT(fmt,arg...)           //printf(fmt,##arg)

#define MAX_TX_SIZE (188*1024)
#define MAX_RX_SIZE (188*1024)
#define INVALID_PID 0x1FFF

#define DEV_FILE_COMMAND	"/dev/ciplus0"
#define DEV_FILE_MEDIA 		"/dev/ciplus1"

typedef struct{
    //route path config
    unsigned int rec_dmxid;
    unsigned int ci_dmxid;
    MT_UNF_DMX_PORT_E tsbuf_port;
    MT_UNF_DMX_PORT_E tuner_port;

    //record info
    unsigned int all_prog_record;
    unsigned int recbuf_size;
}usbcam_init_parm_t;



typedef struct {
	mt_handle rec_handle;
	unsigned int source;
	unsigned int all_prog_record;
    unsigned int recbuf_size;
    unsigned int vpid;
    unsigned int vtype;
    unsigned int apid;
    unsigned int atype;
    unsigned int pcrpid;
    unsigned int pmtpid;
    int link_mode;
    MT_HANDLE dmxchan[5];
    unsigned int dmx_chan_cnt;
} record_info_t;

typedef struct {
	pthread_t id;
	unsigned int exit;
	unsigned int run;
} thread_info;

typedef struct {
	int flag;
	char rec_file[128];
	char usbcam_file[128];
} debug_info;


typedef struct {
	record_info_t record_info;
	thread_info get_data_task;
	thread_info push_data_task;
	debug_info debug;
	MT_HANDLE ram_handle;//
	int usbcam_dev;
	int usbcam_state;
	int start_fragment;
    void *program_tab;
    unsigned int cur_prog_index;
} usbcam_play_context_s;

s32 usbcam_route_ts_init(void);
s32 usbcam_route_ts_uninit(void);

s32 usbcam_route_ts_start(void);

s32 usbcam_route_ts_stop(void);

//s32 usbcam_route_ts_adddel_pid(u16 pid, BOOL add_del);

s32 usbcam_route_ts_set_usbcam_state(int state);

void usbcam_route_set_record_info(void);

//void usbcam_route_fragment(u32 fragment);

usbcam_play_context_s *get_usbcam_context(void);

s32 usbcam_route_param_init(usbcam_init_parm_t *param);

void usbcam_route_start_fragment(u32 fragment);

#endif

