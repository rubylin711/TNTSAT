/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_DLNA_DAEMON_H
#define __MT_DLNA_DAEMON_H

extern void* __MT_DLNA_Create_DMS(void* rt, void* private);
extern void __MT_DLNA_Set_NodeValue(void* handle, char* name, 
			char* value);
extern void* __MT_DLNA_Create_RootDev(void);
extern void __MT_DLNA_Delete_RootDev(void* rt);
extern void* __MT_DLNA_Create_DMR(void* rt);
extern void __MT_DLNA_Delete_DMR(void* dmr);
extern void __MT_DLNA_Delete_DMS(void* dms);
extern int __MT_DLNA_Start(void* dev);
extern void __MT_DLNA_Stop(void* dev);
extern int MT_DLNA_Satip_Addprogramlist(void* dms,char *pmsgbuf,int msglen);

#endif
