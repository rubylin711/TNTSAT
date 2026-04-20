/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <mt_cdlna.h>
#include <cybergarage/upnp/cdevice.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if 0
#define CG_UPNP_DEVICE_UDN "UDN"
#define CG_UPNP_DEVICE_DEVICE_TYPE "deviceType"
#define CG_UPNP_DEVICE_FRIENDLY_NAME "friendlyName"
#endif


#define SKIP_TO_NEXT(p)	do{\
	if (*(p) != ' ')\
	while(*(p) && *(p) != ' ') (p)++;\
	while(*(p) && *(p) == ' ') (p)++;\
} while(0)		
	

int main(int argc, char* argv[])
{
	void *root;
	void *dms;
	void *httpsvr;
	char cmdbuf[128];
	char *ptr;
	unsigned int ip = 0;
	struct in_addr in;

	root = MT_DLNA_Create_RootDev();
	if (!root) 
	{
		printf("\nCreate root Faild!!!\n");
		return -1;
	}
	ip = mt_get_ipaddr();
	in.s_addr = ip;
	httpsvr = mt_start_httpsvr(inet_ntoa(in),8080);
	if (!httpsvr) {
		MT_DLNA_Delete_RootDev(root);
		printf("\nRunning httpSvr Faild!!!\n");
		return -1;
	}

	dms = MT_DLNA_Create_DMS(root);
	if(!dms)
	{
		printf("\nCreate dms device error!!!!\n\n");
		return -1;
	}
	

	MT_DLNA_Set_NodeValue(dms, CG_UPNP_DEVICE_FRIENDLY_NAME, "DLNA_QY_DMS"); //set dmr's device name

	MT_DLNA_Start(root);
	printf("\nStart DLNA...\n");
	while(1) {
		if (!fgets(cmdbuf, 127, stdin))
			break;

		if (strncmp(cmdbuf, "name", 4) == 0) {
			printf("\nDMS Name:\t%s\n", MT_DLNA_Get_NodeValue(dms, CG_UPNP_DEVICE_FRIENDLY_NAME));
		}

		if (strncmp(cmdbuf, "setattr", 7) == 0) {
			char* type;
			type = cmdbuf + 7;
			SKIP_TO_NEXT(type);
			ptr = type;
			SKIP_TO_NEXT(ptr);
			ptr[-1] = 0;			
			MT_DLNA_Set_NodeValue(dms, type, ptr);
		}

		if (strncmp(cmdbuf, "addcontent", 10) == 0) {
			char *path, *name;

			path = cmdbuf + 10;
			SKIP_TO_NEXT(path);

			name = path;
			SKIP_TO_NEXT(name);
			name[-1] = 0;
			printf("\nPath:\t%s\nName:\t%s\n", path, name);
			MT_DLNA_DMS_AddContent(dms, path, name);
		}

		if (strncmp(cmdbuf, "addfilter", 9) == 0) {
			char* postfix;
			cmdbuf[strlen(cmdbuf) - 1] = 0; //skip '\n'
			postfix = cmdbuf + 9;
			SKIP_TO_NEXT(postfix);

			dlna_add_filter(dms, postfix);
		}

		if (strncmp(cmdbuf, "dump", 4) == 0) {
			void dlna_dms_dump(void* dms);
			dlna_dms_dump(dms);
		}
		if (strncmp(cmdbuf, "exit", 4) == 0)
			break;

	}

	MT_DLNA_Delete_DMS(dms);
	MT_DLNA_Delete_RootDev(root);

	printf("\nDone!!!\n");
	return 0;
}


