#ifndef _C_DLNA_H__
#define _C_DLNA_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>	 

#define	MTDLNA_CMDSOCK	"/tmp/dlna_cmd_sock"
#define	MTDLNA_MSGSOCK	"/tmp/dlna_msg_sock"

struct mtdlna_device {
	int dd_type;
	int dd_state;
};

struct mtdlna_cmdhdr {
	unsigned short	d_cmd;
	unsigned short	d_len;
};

typedef int (*http_req_cb)(int sockfd,char * reqpath);


#define	MTDLNAC_START			0x01
#define	MTDLNAC_STOP			0x02
#define	MTDLNAC_SETNODE_ROOT	0x03
#define	MTDLNAC_SETNODE_DMR		0x04
#define	MTDLNAC_SETNODE_DMS		0x05
#define	MTDLNAC_CREATE_ROOT		0x06
#define	MTDLNAC_CREATE_DMR		0x07
#define	MTDLNAC_CREATE_DMS		0x08
#define	MTDLNAC_SETCB_META		0x09
#define	MTDLNAC_SETCB_URL		0x0A
#define	MTDLNAC_SETCB_PLAY		0x0B
#define NTDLNAC_SATIP_INIT		0x0C
#define NTDLNAC_SATIP_SETPROGLIST	0x0D	
#define	MTDLNAC_NAK				0xFD
#define	MTDLNAC_ACK				0xFE
#define	MTDLNAC_EXIT			0xFF

#define	MTDLNAM_META			0x01
#define	MTDLNAM_URL				0x02
#define	MTDLNAM_PLAY			0x03
#define	MTDLNAM_STOP			0x04
#define	MTDLNAM_PAUSE			0x05
#define	MTDLNAM_RESUME			0x06
#define	MTDLNAM_SEEK_POS		0x07
#define	MTDLNAM_GET_POS			0x08
#define	MTDLNAM_SET_VOL			0x09
#define	MTDLNAM_GET_VOL			0x0A
#define	MTDLNAM_SET_MUTE		0x0B
#define	MTDLNAM_GET_MUTE		0x0C
#define	MTDLNAM_GET_TRAN		0x0D
#define	MTDLNAM_GET_TRAN_ACK	0x0E
#define MTDLNAM_GET_TRADK_DUR	0X0F

#define	MTDLNAM_NAK				0xFD
#define	MTDLNAM_ACK				0xFE
#define MTDLNAM_GET_TRADK_ACK	0xFF

/****************************************
* MT_DLNA_Set_NodeValue 
* input: handle- rootdev or dmr ...
* name:  nodename (e.g, "friendlyName")
* value: node value you want set (e.g "My_DLNA_DMR")
****************************************/
void MT_DLNA_Set_NodeValue(void* handle, char* name, char* value);
/****************************************
* MT_DLNA_Get_NodeValue 
* input: handle- rootdev or dmr ...
* name:  nodename (e.g, "friendlyName")
* return: the value of node or NULL
****************************************/
char* MT_DLNA_Get_NodeValue(void* handle, char* name);
/****************************************
* MT_DLNA_Create_RootDev 
* input: none
* return: the rootdev(OK) or NULL (Fail)
****************************************/
void* MT_DLNA_Create_RootDev(void);
/****************************************
* MT_DLNA_Delete_RootDev 
* input: rootdev
****************************************/
void MT_DLNA_Delete_RootDev(void* rtdev);
/****************************************
* MT_DLNA_Create_DMR 
* input: rootdev
* return: the dmr device(OK) or NULL (Fail)
****************************************/
void* MT_DLNA_Create_DMR(void* rtdev);
/****************************************
* MT_DLNA_Delete_DMR 
* input: dmrdev
****************************************/
void MT_DLNA_Delete_DMR(void* dmr);
/****************************************
* MT_DLNA_Create_DMS 
* input: rootdev
* return: the dms device(OK) or NULL (Fail)
****************************************/
void* MT_DLNA_Create_DMS(void* rt);

/****************************************
* MT_DLNA_Delete_DMR 
* input: dmrdev
****************************************/
void MT_DLNA_Delete_DMS(void* dms);

/****************************************
* MT_DLNA_Start 
* input: root device
* return: 0(OK) or -1 (Fail)
****************************************/
int MT_DLNA_Start(void* rtdev);
/****************************************
* MT_DLNA_Stop 
* input: root device
****************************************/
void MT_DLNA_Stop(void* rtdev);

/****************************************
* MT_DLNA_Init 
* input: 
****************************************/
int MT_DLNA_Init(void);

/****************************************
* MT_DLNA_DeInit 
* input: 
****************************************/
void MT_DLNA_DeInit(void);


/****************************************
* dlna_setcb_xxx 
* input: 1. dmr device 2. funtion pointer
****************************************/
extern int MT_DLNA_SetCB_SetMeta(void* dmr, int(*setmeta)(char*));
extern int MT_DLNA_SetCB_SetURL(void* dmr, int(*seturl)(char*));
extern int MT_DLNA_SetCB_Play(void* dmr, int(*play)(void));
extern int MT_DLNA_SetCB_Stop(void* dmr, int(*stop)(void));
extern int MT_DLNA_SetCB_Pause(void* dmr, int(*pause)(void));
extern int MT_DLNA_SetCB_Resume(void* dmr, int(*resume)(void));
extern int MT_DLNA_SetCB_Seek(void* dmr, int(*seek)(int));
extern int MT_DLNA_SetCB_GetPostion(void* dmr, int(*getpostion)(void));
extern int MT_DLNA_SetCB_SetVolume(void* dmr, int(*setvolume)(int));
extern int MT_DLNA_SetCB_GetVolume(void* dmr, int(*getvolume)(void));
extern int MT_DLNA_SetCB_SetMute(void* dmr, int(*setmute)(void));
extern int MT_DLNA_SetCB_GetMute(void* dmr, int(*getmute)(void));
extern int MT_DLNA_SetCB_GetTrackDuration(void* dmr, int(*gettrackduration)(void));
extern int MT_DLNA_SetCB_GetTransportInfo(void* dmr, int(*trans)(void));


// DMS apis
extern int MT_DLNA_DMS_AddContent(void* dms, char* path, char* name);

extern 	void mt_httpsvr_set_callback(http_req_cb callback);
extern 	void* mt_start_httpsvr(char* host,int port);
extern void mt_stop_httpsvr(void* handle);
extern 	int  MT_DLNA_Send_Cmd(int cmd,void *para,int len);
extern int mt_get_svrport(void* handle);
extern void dlna_dms_dump(void* dms);
extern void dlna_del_filter(void* dms, char* postfix);
extern unsigned mt_get_ipaddr(void);
extern void dlna_add_filter(void* dms, char* postfix);

#endif
