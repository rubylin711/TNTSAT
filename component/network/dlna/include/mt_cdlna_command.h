#ifndef _CDLNA_COMMAND_H__
#define _CDLNA_COMMAND_H__

#include <cybergarage/upnp/cdevice.h>
#include <cybergarage/upnp/std/av/cmediarenderer.h>


enum {
	DLNAC_SETMETA = 1,
	DLNAC_SETURL,
	DLNAC_PLAY,
	DLNAC_STOP,
	DLNAC_PAUSE,
	DLNAC_RESUME,
	DLNAC_SEEK,
	DLNAC_GETPOSTION,
	DLNAC_SETVOLUME,
	DLNAC_GETVOLUME,
	DLNAC_SETMUTE,
	DLNAC_GETMUTE,
	DLNAC_GETTRANSPORT_INFO,
	DLNAC_GETTRACKDURATION
};

struct dlna_device {
	int		dd_type;	
	void*	dd_private;
	void*	dd_device;
	int (*dd_setmeta)(char*);
	int (*dd_seturl)(char*);
	int (*dd_play)(void);
	int (*dd_stop)(void);
	int (*dd_pause)(void);
	int (*dd_resume)(void);
	int (*dd_seek)(int);
	int (*dd_getpostion)(void);
	int (*dd_setvolume)(int);
	int (*dd_getvolume)(void);
	int (*dd_setmute)(void);
	int (*dd_getmute)(void);
	int (*dd_gettransport_info)(char *);
	int (*dd_gettrackduration)(void);
};

#define DDT_ROOT	1
#define DDT_DMR		2
#define DDT_DMS		4

struct dlna_dmrdev {
	CgUpnpAvRenderer* dd_dmr;
	int (*dd_setmeta)(char*);
	int (*dd_seturl)(char*);
	int (*dd_play)(void);
	int (*dd_stop)(void);
	int (*dd_pause)(void);
	int (*dd_resume)(void);
	int (*dd_seek)(int);
	int (*dd_getpostion)(void);
	int (*dd_setvolume)(int);
	int (*dd_getvolume)(void);
	int (*dd_setmute)(void);
	int (*dd_getmute)(void);	
};

struct __mt_ffilter {
	char*	mf_postfix;
	struct __mt_ffilter * mf_next;
};

struct dlna_dmsdev {
	CgUpnpMediaServer* dd_dms;
	struct __mt_ffilter dd_filter;
	void* dd_hsvr;
	char  dd_ipaddr[20];
};
extern int dlnac_command(CgUpnpAvRenderer *dmr, CgUpnpArgument *arg, int cmd);

#endif

