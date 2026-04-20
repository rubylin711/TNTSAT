/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __satip_player_h
#define __satip_player_h

struct mtdlna_satip_cfg {
	unsigned int tsserver_ip;
	int port;
};

typedef struct _satip_pg_info_t
{
    unsigned char  pg_name[68];     // Program name
    unsigned int  freq;            // Input freq in MHz(950~2150)
    unsigned int  symb_rate;       // Input symb_rate in KSs
    unsigned int  onoff_22k;       // onoff_22k : Input 22k on/off(1/0)
    unsigned int  polar;           // polarization : Input polarization, 0:H; 1:V; 2:L; 3:R
    unsigned int v_pid;           //  video pid
    unsigned int a_pid;           // audio pid
    unsigned int pcr_pid;         // pcr pid
    unsigned int pmt_pid;
    unsigned int pg_id;
    unsigned int is_des;          // [7:4] dmx record mode
} satip_pg_info_t;

typedef struct _satip_pg_list
{
	int pg_cnt;
	satip_pg_info_t *pglist;
}satip_pg_list;

typedef struct __satip_dvb_opration_t
{	
	void (*nim_lock)(void * param);
	unsigned long (*start_rec)(satip_pg_info_t *prog);
	int (*stop_rec)(unsigned  long rechandle);	
	unsigned int (*rec_acquire_buf)(unsigned long rechandle,unsigned char **pbufaddr, unsigned int *len);	
	int (*rec_release_buf)(unsigned long rechandle,unsigned char *pbufaddr);
}satip_dvb_op_t;


/**
\brief init satip. CNcomment:初始化Satip	CNend
\attention \n
N/A
\param[in] ip:http server ip    CNcomment:http server ip	CNend
\param[in] port:   http server port  CNcomment:http server port	CNend
\param[in] psatip_callback: DVB operate funcitons  CNcomment:DVB操作函数CNend
\retval :: NULL	init failed. CNcomment:初始化失败	CNend
 \retval :: Not NULL handle of satip. CNcomment: satip句柄	 CNend
 \see \n
 N/A
*/
void* mt_satip_init(char *ip,int port,satip_dvb_op_t *psatip_callback);

/**
\brief config the programs will be shared. CNcomment:配置要分享的节目列表CNend
\attention \n
if the 'port' is using,it will use the next port number until socket bind successful
如果port正在使用， 会选择下一个port，直到能够成功绑定为止
N/A
\param[in] psatipdev: satip handle    CNcomment: satip句柄	CNend
\param[in] plist: prorgram list  CNcomment:直播节目列表	CNend
\retval \n
N/A
see \n
N/A
*/
void  mt_satip_setproglist(void *psatipdev,satip_pg_list *plist);

/**
\brief deinit satip. CNcomment:反初始化satip		CNend
\attention \n
N/A
\param[in] psatipdev: satip handle    CNcomment: satip句柄	CNend
\retval  \n
N/A
see \n
N/A
*/
void mt_satip_deinit(void *pdevsatip);

/**
\brief start satip. CNcomment:启动satip		CNend
\attention \n
N/A
\param[in] psatipdev: satip handle    CNcomment: satip句柄	CNend
\retval:: 0: successful  \n			CNcomment: 0:启动成功	CNend
\retval:: -1: successful  \n			CNcomment: -1:启动失败	CNend
\see \n
N/A
*/
int mt_satip_start(void *pdevsatip);

/**
\brief stop satip. CNcomment:停止satip		CNend
\attention \n
N/A
\param[in] psatipdev: satip handle    CNcomment: satip句柄	CNend
\retval:: 0: successful  \n			CNcomment: 0:停止成功	CNend
\retval:: -1: successful  \n			CNcomment: -1:停止失败	CNend
\see \n
N/A
*/
int mt_satip_stop(void *pdevsatip);

/**
\brief set dms device's name. CNcomment:设置dms设备名字		CNend
\attention \n
N/A
\param[in] psatipdev: satip handle    CNcomment: satip句柄	CNend
\param[in] name: pointor to name    CNcomment: 设备名指针	CNend
\param[in] len: len of name    CNcomment: 设备名长度	CNend
\retval \n
N/A 
\see \n
N/A
*/
void  mt_satip_set_friendlyname(void *pdevsatip,char *name ,int len);

#endif
