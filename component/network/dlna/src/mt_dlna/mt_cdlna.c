/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <cybergarage/util/clist.h>
#include <mt_cdlna_command.h>
#include <cybergarage/upnp/std/av/cdms_filesys.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "mt_cdlna.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cybergarage/util/clog.h"
#include "satip_player.h"
#include "mt_dlna_daemon.h"

#define CG_UPNP_DEVICE_DMR_DESCRIPTION_URI "/description_dmr.xml"
#define CG_UPNP_DEVICE_DMS_DESCRIPTION_URI "/description_dms.xml"

static char *cg_root_devdesc =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"\
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\" xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">\n"\
" <specVersion>\n"\
"  <major>1</major>\n"\
"  <minor>0</minor>\n"\
" </specVersion>\n"\
" <device>\n"\
"  <deviceType>urn:schemas-upnp-org:device:rootdevice</deviceType>\n"\
"  <friendlyName>root</friendlyName>\n"\
"  <manufacturer>root</manufacturer>\n"\
"  <manufacturerURL>root</manufacturerURL>\n"\
"  <modelDescription>root</modelDescription>\n"\
"  <modelName>root</modelName>\n"\
"  <modelNumber>1.0</modelNumber>\n"\
"  <UDN>uuid:BA2E90A0-3669-401a-B249-F85196ADFC66</UDN>\n"\
"  <modelURL>http://www.cybergarage.org</modelURL>\n"\
"  <dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMR-1.50</dlna:X_DLNADOC>\n"\
"  <dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMS-1.00</dlna:X_DLNADOC>\n"\
" </device>\n"\
"</root>\n";

extern unsigned mt_get_ipaddr(void);
extern char* dlna_get_ipaddr(CgUpnpMediaServer* dms);
extern void __dlna_free_filter(struct __mt_ffilter* hlist);
extern int dlna_filter_match(CgUpnpMediaServer* dms, char* name);

static void set_nodevalue(CgUpnpDevice* dev, char* name, char* value)
{
	cg_xml_node_setchildnode(dev->deviceNode, name, value);
}

static char* get_nodevalue(CgUpnpDevice* dev, char* name)
{
	return cg_xml_node_getchildnodevalue(dev->deviceNode, name);
}

#if 1


//#define	MTDLNA_DEBUG 1

#undef OS_PRINTF
#ifdef MTDLNA_DEBUG
#define	OS_PRINTF	printf
#else
#define OS_PRINTF(...)     do{}while(0)
#endif
#define	OS_ERROR	printf

void __MT_DLNA_Set_NodeValue(void* handle, char* name, char* value)
{
	struct dlna_device *device = handle;
	CgUpnpDevice* dev;
	CgUpnpAvRenderer *dmr;
	CgUpnpMediaServer* dms;

	if (!device)
		return;

	switch(device->dd_type)
	{
		case DDT_ROOT:
			dev = (CgUpnpDevice* )device->dd_device;
			break;
		case DDT_DMR:
			dmr = (CgUpnpAvRenderer *)device->dd_device;
			dev = dmr->dev;
			break;
		case DDT_DMS:
			dms = ((struct dlna_dmsdev*) device->dd_device)->dd_dms;
			dev = dms->dev;
			break;
		default:
			return;
	}

	set_nodevalue(dev, name, value);
}

char* MT_DLNA_Get_NodeValue(void* handle, char* name)
{
	struct dlna_device *device = handle;
	CgUpnpDevice* dev;
	CgUpnpAvRenderer *dmr;
	CgUpnpMediaServer* dms;

	if (!device)
		return NULL;

	switch(device->dd_type)
	{
		case DDT_ROOT:
			dev = (CgUpnpDevice* )device->dd_device;
			break;
		case DDT_DMR:
			dmr = (CgUpnpAvRenderer *)device->dd_device;
			dev = dmr->dev;
			break;
		case DDT_DMS:
			dms = ((struct dlna_dmsdev*) device->dd_device)->dd_dms;
			dev = dms->dev;
			break;
		default:
			return NULL;
	}

	return get_nodevalue(dev, name);
}


void* __MT_DLNA_Create_RootDev(void)
{
	struct dlna_device* ddr;
	CgUpnpDevice *root;

	ddr = malloc(sizeof(*ddr));
	if (!ddr)
		return NULL;
	root = cg_upnp_device_new();
	if (root) {
		cg_upnp_device_parsedescription(root, cg_root_devdesc, cg_strlen(cg_root_devdesc));
		ddr->dd_type = DDT_ROOT;
		ddr->dd_device = root;
		root->private = ddr;
	} else {
		free(ddr);
		ddr = NULL;
	}

	return ddr;
}

void __MT_DLNA_Delete_RootDev(void* rt)
{
	struct dlna_device *ddr = rt;

	if (!rt)
		return;

	if (ddr->dd_type == DDT_ROOT)
		cg_upnp_device_delete((CgUpnpDevice*)ddr->dd_device);

	free(ddr);
}

 void* __MT_DLNA_Create_DMR(void* rt)
{
	struct dlna_device* ddr;
	struct dlna_device* ddmr;
	CgUpnpDevice* root;
	CgUpnpAvRenderer *dmr;

	ddr = (struct dlna_device*)rt;
	if (!ddr || ddr->dd_type != DDT_ROOT)
		return NULL;

	ddmr = malloc(sizeof(*ddr));
	if (!ddmr)
		return NULL;

	root = (CgUpnpDevice*)ddr->dd_device;

	dmr = cg_upnpav_dmr_new();
	if (dmr) {
		cg_upnp_device_setdescriptionuri(dmr->dev, CG_UPNP_DEVICE_DMR_DESCRIPTION_URI);
		cg_list_add((CgList *)root->deviceList, (CgList *)dmr->dev);
		ddmr->dd_type = DDT_DMR;
		ddmr->dd_device = dmr;
		dmr->private = ddmr;
	} else {
		free(ddmr);
		ddmr = NULL;
	}

	return ddmr;
}

void __MT_DLNA_Delete_DMR(void* dmr)
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || (ddmr->dd_type != DDT_DMR))
		return;

	cg_upnpav_dmr_delete((CgUpnpAvRenderer *)ddmr->dd_device);
	free(ddmr);
}

void* __MT_DLNA_Create_DMS(void* rt, void* private)
{
	struct dlna_device* ddr;
	struct dlna_device* ddms;
	CgUpnpDevice* root;
	CgUpnpMediaServer* dms;
	struct dlna_dmsdev *d_dms;
	unsigned ip;

	ddr = (struct dlna_device*)rt;
	if (!ddr || ddr->dd_type != DDT_ROOT)
		return NULL;

	root = (CgUpnpDevice*)ddr->dd_device;
	if (!root)
		return NULL;

	ddms = malloc(sizeof(*ddms));
	if (!ddms)
		return NULL;

	d_dms = malloc(sizeof(struct dlna_dmsdev));
	if (!d_dms) {
		free(ddms);
		return NULL;
	}

	dms = cg_upnp_dms_filesys_new();
	if (!dms) {
		free(d_dms);
		free(ddms);
		return NULL;
	}

	ddms->dd_type = DDT_DMS;
	ddms->dd_device = d_dms;
	d_dms->dd_dms = dms;
	d_dms->dd_hsvr = private;
	memset(&d_dms->dd_filter, 0, sizeof(d_dms->dd_filter));
	cg_upnpav_dms_setuserdata(dms, ddms);
	cg_upnp_device_setdescriptionuri(dms->dev, CG_UPNP_DEVICE_DMS_DESCRIPTION_URI);
	cg_list_add((CgList *)root->deviceList, (CgList *)dms->dev);

	ip = mt_get_ipaddr();
	strcpy(d_dms->dd_ipaddr, inet_ntoa(*((struct in_addr*)&ip)));
	return ddms;
}

char* dlna_get_ipaddr(CgUpnpMediaServer* dms)
{
	struct dlna_device* ddms;
	struct dlna_dmsdev *d_dms;

	if (!dms)
		return NULL;

	ddms = cg_upnpav_dms_getuserdata(dms);
	if (!ddms || !ddms->dd_device)
		return NULL;

	if (ddms->dd_type != DDT_DMS)
		return NULL;

	d_dms = ddms->dd_device;
	return d_dms->dd_ipaddr;
}

void dlna_add_filter(void* dms, char* postfix)
{
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev *d_dms;
	struct __mt_ffilter *mff, **prev;

	if (!ddms || !ddms->dd_device)
		return;

	if (ddms->dd_type != DDT_DMS)
		return;

	if (!postfix)
		return;

	d_dms = ddms->dd_device;

	prev = &d_dms->dd_filter.mf_next;
	while (*prev)
		prev = &((*prev)->mf_next);

	mff = malloc(sizeof(*mff));
	if (!mff)
		return;

	mff->mf_next = NULL;
	mff->mf_postfix = malloc(strlen(postfix) + 1);
	if (!mff->mf_postfix) {
		free(mff);
		return;
	}

	strcpy(mff->mf_postfix, postfix);
	mff->mf_postfix[strlen(postfix)] = 0;
	*prev = mff;
}

void dlna_del_filter(void* dms, char* postfix)
{
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev *d_dms;
	struct __mt_ffilter *mff, *prev;

	if (!ddms || !ddms->dd_device)
		return;

	if (ddms->dd_type != DDT_DMS)
		return;

	if (!postfix)
		return;

	d_dms = ddms->dd_device;

	prev = &d_dms->dd_filter;
	mff = prev->mf_next;
	while (mff) {
		if (strcmp(mff->mf_postfix, postfix) == 0)
			break;
		prev = mff;
		mff = mff->mf_next;
	}

	if (mff) {
		prev = mff->mf_next;
		free(mff->mf_postfix);
		free(mff);
	}
}

int dlna_filter_match(CgUpnpMediaServer* dms, char* name)
{
	struct dlna_device* ddms;
	struct dlna_dmsdev *d_dms;
	struct __mt_ffilter *mff, *prev;
	char* tmp;
	return 1;
	if (!dms)
		return 1;

	ddms = cg_upnpav_dms_getuserdata(dms);
	if (!ddms || !ddms->dd_device)
		return 1;

	if (ddms->dd_type != DDT_DMS)
		return 1;

	d_dms = ddms->dd_device;
	mff = d_dms->dd_filter.mf_next;
	while (mff) {
		tmp = strstr(name, mff->mf_postfix);
		if (!tmp) {
			mff = mff->mf_next;
			continue;
		}

		if (tmp[strlen(mff->mf_postfix)] == 0)
			return 1;

		mff = mff->mf_next;
	}

	return 0;
}

void __dlna_free_filter(struct __mt_ffilter* hlist)
{
	struct __mt_ffilter* tmp, *p;

	tmp = hlist->mf_next;
	while (tmp) {
		p = tmp->mf_next;
		free(tmp->mf_postfix);
		free(tmp);
		tmp = p;
	}

}

void __MT_DLNA_Delete_DMS(void* dms)
{
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev *d_dms;

	if (!ddms || ddms->dd_type != DDT_DMS)
		return;

	d_dms = ddms->dd_device;
	if (!d_dms)
		return;

	cg_upnp_dms_filesys_delete(d_dms->dd_dms);
	__dlna_free_filter(&d_dms->dd_filter);
	free(d_dms);
	free(dms);
}

//void cg_upnp_dump_content(void *objectContent);
void dlna_dms_dump(void* dms)
{
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev *d_dms;

	if (!ddms || ddms->dd_type != DDT_DMS)
		return;

	d_dms = ddms->dd_device;
	if (!d_dms)
		return;

	cg_upnp_dump_content(d_dms->dd_dms->rootContent);
}

int __MT_DLNA_Start(void* dev)
{
	struct dlna_device* ddr = dev;
	if (!dev || ddr->dd_type != DDT_ROOT)
		return -1;

	if (cg_upnp_device_start((CgUpnpDevice*)ddr->dd_device))
		return 0;
	else
		return -1;
}

void __MT_DLNA_Stop(void* dev)
{
	struct dlna_device* ddr = dev;

	if (!dev || ddr->dd_type != DDT_ROOT)
		return;

	cg_upnp_device_stop((CgUpnpDevice*)ddr->dd_device);
}


//void cg_upnp_dms_add_content(void *dms, void* arg, char* path, char* name);

int MT_DLNA_DMS_AddContent(void* dms, char* path, char* name)
{
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev* d_dms;

	if (!ddms || ddms->dd_type != DDT_DMS)
		return -1;

	d_dms = ddms->dd_device;
	if (!d_dms)
		return -1;

	cg_upnp_dms_add_content(d_dms->dd_dms, d_dms->dd_hsvr, path, name);
	return 0;
}

int MT_DLNA_Satip_Addprogramlist(void* dms,char *pmsgbuf,int msglen)
{
	
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev* d_dms;
	CgUpnpMediaContent *dircon,*filecon;
	char basebuf[512];
	char idmd5[64];
	CgUpnpMediaResource *res;
	int i = 0;
	struct mtdlna_satip_cfg *pcfg;
	satip_pg_list plist;
	struct in_addr addr;

	if (!ddms || ddms->dd_type != DDT_DMS)
		return -1;

	d_dms = ddms->dd_device;
	if (!d_dms)
		return -1;

	dircon = cg_upnp_dms_addentry(d_dms->dd_dms, NULL, "satip_program");
	if (!dircon) {
		return -1;
	}

	cg_log_debug_l4("msglen:%d\n",msglen);
	pcfg = (struct mtdlna_satip_cfg*)d_dms->dd_hsvr;	
	addr.s_addr = pcfg->tsserver_ip;
	cg_log_debug_l4("ip:%s,port:%d\n",inet_ntoa(addr),pcfg->port);
	
	plist.pg_cnt = msglen/sizeof(satip_pg_info_t);
	plist.pglist = (satip_pg_info_t*)pmsgbuf;
	
	cg_log_debug_l4("pg_cnt:%d\n",plist.pg_cnt);
	
	for(i = 0;i < plist.pg_cnt;i++) {
		memset(basebuf,0,sizeof(basebuf));
		sprintf(basebuf, "http://%s:%d/%d_%d_%d_%d_%d_%d_%d_%d_%d_%d.ts",
				inet_ntoa(addr),
				pcfg->port,
				plist.pglist[i].pg_id,
				plist.pglist[i].freq,
				plist.pglist[i].symb_rate,
				plist.pglist[i].onoff_22k,
				plist.pglist[i].polar,
				plist.pglist[i].v_pid,
				plist.pglist[i].a_pid,
				plist.pglist[i].pmt_pid,
				plist.pglist[i].pcr_pid,
				plist.pglist[i].is_des
				);
		filecon = cg_upnpav_content_new();

		cg_str2md5(basebuf, idmd5);
		cg_log_debug_l4("program:%s,url:%s\n",plist.pglist[i].pg_name, basebuf);
		cg_upnpav_content_setid(filecon, idmd5);
		cg_upnpav_content_setparentid(filecon, cg_upnpav_content_getid(dircon));
		cg_upnpav_content_settitle(filecon, plist.pglist[i].pg_name);	
		cg_upnpav_content_settype(filecon, CG_UPNPAV_CONTENT_ITEM);
		cg_upnpav_content_setupnpclass(filecon, CG_UPNPAV_UPNPCLASS_MOVIE);

		cg_upnpav_content_addchildcontent(dircon, filecon);
		res = cg_upnpav_resource_new();
		cg_upnpav_resource_seturl(res, basebuf);
		cg_upnpav_resource_setsize(res,99999999);
		cg_upnpav_resource_setmimetype(res,"video/x-matroska");
		cg_upnpav_content_addresource(filecon, res);
        cg_upnpav_dms_condir_updatesystemupdateid(d_dms->dd_dms);
		
	}

	return 0;	
}

#else

void MT_DLNA_Set_NodeValue(void* handle, char* name, char* value)
{
	struct dlna_device *device = handle;
	CgUpnpDevice* dev;
	CgUpnpAvRenderer *dmr;
	CgUpnpMediaServer* dms;

	if (!device)
		return;

	switch(device->dd_type)
	{
		case DDT_ROOT:
			dev = (CgUpnpDevice* )device->dd_device;
			break;
		case DDT_DMR:
			dmr = (CgUpnpAvRenderer *)device->dd_device;
			dev = dmr->dev;
			break;
		case DDT_DMS:
			dms = ((struct dlna_dmsdev*) device->dd_device)->dd_dms;
			dev = dms->dev;
			break;
		default:
			return;
	}

	set_nodevalue(dev, name, value);
}

char* MT_DLNA_Get_NodeValue(void* handle, char* name)
{
	struct dlna_device *device = handle;
	CgUpnpDevice* dev;
	CgUpnpAvRenderer *dmr;
	CgUpnpMediaServer* dms;

	if (!device)
		return NULL;

	switch(device->dd_type)
	{
		case DDT_ROOT:
			dev = (CgUpnpDevice* )device->dd_device;
			break;
		case DDT_DMR:
			dmr = (CgUpnpAvRenderer *)device->dd_device;
			dev = dmr->dev;
			break;
		case DDT_DMS:
			dms = ((struct dlna_dmsdev*) device->dd_device)->dd_dms;
			dev = dms->dev;
			break;
		default:
			return NULL;
	}

	return get_nodevalue(dev, name);
}


void* MT_DLNA_Create_RootDev(void)
{
	struct dlna_device* ddr;
	CgUpnpDevice *root;

	ddr = malloc(sizeof(*ddr));
	if (!ddr)
		return NULL;
	root = cg_upnp_device_new();
	if (root) {
		cg_upnp_device_parsedescription(root, cg_root_devdesc, cg_strlen(cg_root_devdesc));
		ddr->dd_type = DDT_ROOT;
		ddr->dd_device = root;
		root->private = ddr;
	} else {
		free(ddr);
		ddr = NULL;
	}

	return ddr;
}



void MT_DLNA_Delete_RootDev(void* rt)
{
	struct dlna_device *ddr = rt;

	if (!rt)
		return;

	if (ddr->dd_type == DDT_ROOT)
		cg_upnp_device_delete((CgUpnpDevice*)ddr->dd_device);

	free(ddr);
}

void* MT_DLNA_Create_DMR(void* rt)
{
	struct dlna_device* ddr;
	struct dlna_device* ddmr;
	CgUpnpDevice* root;
	CgUpnpAvRenderer *dmr;

	ddr = (struct dlna_device*)rt;
	if (!ddr || ddr->dd_type != DDT_ROOT)
		return NULL;

	ddmr = malloc(sizeof(*ddr));
	if (!ddmr)
		return NULL;

	root = (CgUpnpDevice*)ddr->dd_device;

	dmr = cg_upnpav_dmr_new();
	if (dmr) {
		cg_upnp_device_setdescriptionuri(dmr->dev, CG_UPNP_DEVICE_DMR_DESCRIPTION_URI);
		cg_list_add((CgList *)root->deviceList, (CgList *)dmr->dev);
		ddmr->dd_type = DDT_DMR;
		ddmr->dd_device = dmr;
		dmr->private = ddmr;
	} else {
		free(ddmr);
		ddmr = NULL;
	}

	return ddmr;
}

void MT_DLNA_Delete_DMR(void* dmr)
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return;

	cg_upnpav_dmr_delete((CgUpnpAvRenderer *)ddmr->dd_device);
	free(ddmr);
}

void* MT_DLNA_Create_DMS(void* rt, void* private)
{
	struct dlna_device* ddr;
	struct dlna_device* ddms;
	CgUpnpDevice* root;
	CgUpnpMediaServer* dms;
	struct dlna_dmsdev *d_dms;
	unsigned ip;

	ddr = (struct dlna_device*)rt;
	if (!ddr || ddr->dd_type != DDT_ROOT)
		return NULL;

	root = (CgUpnpDevice*)ddr->dd_device;
	if (!root)
		return NULL;

	ddms = malloc(sizeof(*ddms));
	if (!ddms)
		return NULL;

	d_dms = malloc(sizeof(struct dlna_dmsdev));
	if (!d_dms) {
		free(ddms);
		return NULL;
	}

	dms = cg_upnp_dms_filesys_new();
	if (!dms) {
		free(d_dms);
		free(ddms);
		return NULL;
	}

	ddms->dd_type = DDT_DMS;
	ddms->dd_device = d_dms;
	d_dms->dd_dms = dms;
	d_dms->dd_hsvr = private;
	memset(&d_dms->dd_filter, 0, sizeof(d_dms->dd_filter));
	cg_upnpav_dms_setuserdata(dms, ddms);
	cg_upnp_device_setdescriptionuri(dms->dev, CG_UPNP_DEVICE_DMS_DESCRIPTION_URI);
	cg_list_add((CgList *)root->deviceList, (CgList *)dms->dev);

	ip = mt_get_ipaddr();
	strcpy(d_dms->dd_ipaddr, inet_ntoa(*((struct in_addr*)&ip)));
	return ddms;
}

char* dlna_get_ipaddr(CgUpnpMediaServer* dms)
{
	struct dlna_device* ddms;
	struct dlna_dmsdev *d_dms;

	if (!dms)
		return NULL;

	ddms = cg_upnpav_dms_getuserdata(dms);
	if (!ddms || !ddms->dd_device)
		return;

	if (ddms->dd_type != DDT_DMS)
		return;

	d_dms = ddms->dd_device;
	return d_dms->dd_ipaddr;
}

void dlna_add_filter(void* dms, char* postfix)
{
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev *d_dms;
	struct __mt_ffilter *mff, **prev;

	if (!ddms || !ddms->dd_device)
		return;

	if (ddms->dd_type != DDT_DMS)
		return;

	if (!postfix)
		return;

	d_dms = ddms->dd_device;

	prev = &d_dms->dd_filter.mf_next;
	while (*prev)
		prev = &((*prev)->mf_next);

	mff = malloc(sizeof(*mff));
	if (!mff)
		return;

	mff->mf_next = NULL;
	mff->mf_postfix = malloc(strlen(postfix) + 1);
	if (!mff->mf_postfix) {
		free(mff);
		return;
	}

	strcpy(mff->mf_postfix, postfix);
	*prev = mff;
}

void dlna_del_filter(void* dms, char* postfix)
{
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev *d_dms;
	struct __mt_ffilter *mff, *prev;

	if (!ddms || !ddms->dd_device)
		return;

	if (ddms->dd_type != DDT_DMS)
		return;

	if (!postfix)
		return;

	d_dms = ddms->dd_device;

	prev = &d_dms->dd_filter;
	mff = prev->mf_next;
	while (mff) {
		if (strcmp(mff->mf_postfix, postfix) == 0)
			break;
		prev = mff;
		mff = mff->mf_next;
	}

	if (mff) {
		prev = mff->mf_next;
		free(mff->mf_postfix);
		free(mff);
	}
}

int dlna_filter_match(CgUpnpMediaServer* dms, char* name)
{
	struct dlna_device* ddms;
	struct dlna_dmsdev *d_dms;
	struct __mt_ffilter *mff, *prev;
	char* tmp;

	if (!dms)
		return 1;

	ddms = cg_upnpav_dms_getuserdata(dms);
	if (!ddms || !ddms->dd_device)
		return 1;

	if (ddms->dd_type != DDT_DMS)
		return 1;

	d_dms = ddms->dd_device;
	mff = d_dms->dd_filter.mf_next;
	while (mff) {
		tmp = strstr(name, mff->mf_postfix);
		if (!tmp) {
			mff = mff->mf_next;
			continue;
		}

		if ((tmp[-1] == '.') &&
			tmp[strlen(mff->mf_postfix)] == 0)
			return 1;

		mff = mff->mf_next;
	}

	return 0;
}

static void __dlna_free_filter(struct __mt_ffilter* hlist)
{
	struct __mt_ffilter* tmp, *p;

	tmp = hlist->mf_next;
	while (tmp) {
		p = tmp->mf_next;
		free(tmp->mf_postfix);
		free(tmp);
		tmp = p;
	}

}

void MT_DLNA_Delete_DMS(void* dms)
{
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev *d_dms;

	if (!ddms || ddms->dd_type != DDT_DMS)
		return;

	d_dms = ddms->dd_device;
	if (!d_dms)
		return;

	cg_upnp_dms_filesys_delete(d_dms->dd_dms);
	__dlna_free_filter(&d_dms->dd_filter);
	free(d_dms);
	free(dms);
}

void cg_upnp_dump_content(void *objectContent);
void dlna_dms_dump(void* dms)
{
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev *d_dms;

	if (!ddms || ddms->dd_type != DDT_DMS)
		return;

	d_dms = ddms->dd_device;
	if (!d_dms)
		return;

	cg_upnp_dump_content(d_dms->dd_dms->rootContent);
}

int MT_DLNA_Start(void* dev)
{
	struct dlna_device* ddr = dev;
	if (!dev || ddr->dd_type != DDT_ROOT)
		return -1;

	if (cg_upnp_device_start((CgUpnpDevice*)ddr->dd_device))
		return 0;
	else
		return -1;
}

void MT_DLNA_Stop(void* dev)
{
	struct dlna_device* ddr = dev;

	if (!dev || ddr->dd_type != DDT_ROOT)
		return;

	cg_upnp_device_stop((CgUpnpDevice*)ddr->dd_device);
}

int MT_DLNA_SetCB_SetMeta(void* dmr, int(*setmeta)(char*))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_setmeta = setmeta;
	return 0;
}

int MT_DLNA_SetCB_SetURL(void* dmr, int(*seturl)(char*))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_seturl = seturl;
	return 0;
}

int MT_DLNA_SetCB_Play(void* dmr, int(*play)(void))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_play = play;
	return 0;
}

int MT_DLNA_SetCB_Stop(void* dmr, int(*stop)(void))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_stop = stop;
	return 0;
}

int MT_DLNA_SetCB_Pause(void* dmr, int(*pause)(void))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_pause = pause;
	return 0;
}

int MT_DLNA_SetCB_Resume(void* dmr, int(*resume)(void))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_resume = resume;
	return 0;
}

int MT_DLNA_SetCB_Seek(void* dmr, int(*seek)(int))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_seek = seek;
	return 0;
}

int MT_DLNA_SetCB_GetPostion(void* dmr, int(*getpostion)(void))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_getpostion = getpostion;
	return 0;
}

int MT_DLNA_SetCB_SetVolume(void* dmr, int(*setvolume)(int))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_setvolume = setvolume;
	return 0;
}

int MT_DLNA_SetCB_GetVolume(void* dmr, int(*getvolume)(void))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_getvolume = getvolume;
	return 0;
}

int MT_DLNA_SetCB_SetMute(void* dmr, int(*setmute)(void))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_setmute = setmute;
	return 0;
}

int MT_DLNA_SetCB_GetMute(void* dmr, int(*getmute)(void))
{
	struct dlna_device* ddmr = dmr;

	if (!ddmr || ddmr->dd_type != DDT_DMR)
		return -1;

	ddmr->dd_getmute = getmute;
	return 0;
}

void cg_upnp_dms_add_content(void *dms, void* arg, char* path, char* name);

int MT_DLNA_DMS_AddContent(void* dms, char* path, char* name)
{
	struct dlna_device* ddms = dms;
	struct dlna_dmsdev* d_dms;

	if (!ddms || ddms->dd_type != DDT_DMS)
		return -1;

	d_dms = ddms->dd_device;
	if (!d_dms)
		return -1;

	cg_upnp_dms_add_content(d_dms->dd_dms, d_dms->dd_hsvr, path, name);
	return 0;
}

#endif

