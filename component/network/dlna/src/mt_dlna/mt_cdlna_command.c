/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <mt_cdlna.h>
#include <mt_cdlna_command.h>

static int __dlnac_setmeta(struct dlna_device *ddev, CgUpnpArgument *arg)
{
	int ret = -1;

	char *md, ch; 
	char* str_s, *str_e;

	if (!ddev || !arg)
		return -1;

	md = cg_xml_unescapechars(arg->value);
	str_s = strstr(md, "<dc:title>");
	if (str_s) {
		str_e = strstr(md, "</dc:title>");
		if (!str_e) {
			printf("\nFound Error in metadata!!!\n");
			return -1;
		}
		str_s += strlen("<dc:title>");
		ch = *str_e;
		*str_e = 0;
		if (ddev->dd_setmeta)
			ret = ddev->dd_setmeta(str_s);
		*str_e = ch;
	}
	return ret;
}

static int __dlnac_seturl(struct dlna_device *ddev, CgUpnpArgument *arg)
{	
	int ret = -1;
	char* url;

	if (!ddev || !arg)
		return -1;
	
	url = cg_xml_unescapechars(arg->value);
	if (ddev->dd_seturl)
		ret = ddev->dd_seturl(url);
	return ret;
}

static int __dlnac_play(struct dlna_device *ddev)
{
	int ret = -1;
	
	if (!ddev)
		return -1;
	
	if (ddev->dd_play)
		ret = ddev->dd_play();
	
	return ret;
}

static int __dlnac_stop(struct dlna_device *ddev)
{
	int ret = -1;
	
	if (!ddev)
		return -1;

	if (ddev->dd_stop)
		ret = ddev->dd_stop();
	
	return ret;
}

static int __dlnac_pause(struct dlna_device *ddev)
{
	int ret = -1;
	
	if (!ddev)
		return -1;
	if (ddev->dd_pause)
		ret = ddev->dd_pause();
	
	return ret;
}

static int __dlnac_resume(struct dlna_device *ddev)
{
	int ret = -1;
	
	if (!ddev)
		return -1;
	if (ddev->dd_resume)
		ret = ddev->dd_resume();
	
	return ret;
}

static int __dlnac_seek(struct dlna_device *ddev, CgUpnpArgument *arg)
{
	char* pst;
	char* tmp;
	char ch;
	int s_time, ret = -1;

	if (!ddev || !arg)
		return -1;
	
	pst = cg_xml_unescapechars(arg->value);

	if (!pst)
		return -1;

	tmp = strstr(pst, ":");
	if (!tmp)
		return -1;
	ch = tmp[0];
	tmp[0] = 0;

	s_time = atoi(pst) * 3600;
	tmp[0] = ch;

	pst = tmp + 1;
	tmp = strstr(pst, ":");
	if (!tmp)
		return -1;

	ch = tmp[0];
	tmp[0] = 0;
	s_time += (atoi(pst) * 60);
	tmp[0] = ch;

	pst = tmp + 1;
	s_time += atoi(pst);

	if (ddev->dd_seek)
		ret = ddev->dd_seek(s_time);

	return ret;
}

static int __dlnac_getpostion(struct dlna_device *ddev, CgUpnpArgument *arg)
{
	char tmp[16];
	int postion = 0;

	if (!arg || !ddev || !ddev->dd_getpostion)
		return -1;

	postion = ddev->dd_getpostion();
	sprintf(tmp, "%02d:%02d:%02d", postion / 3600, (postion % 3600)/60, postion % 60);

	cg_upnp_argument_setvalue(arg, tmp);
	return 0;
}

static int __dlnac_setvolume(struct dlna_device *ddev, CgUpnpArgument *arg)
{
	char *vol;
	int ret = -1, vol_v;

	if (!ddev || !arg)
		return -1;

	vol = cg_xml_unescapechars(arg->value);
	if (!vol)
		return -1;

	vol_v = atoi(vol);
	if (ddev->dd_setvolume)
		ret = ddev->dd_setvolume(vol_v);

	return ret;
}

static int __dlnac_getvolume(struct dlna_device *ddev, CgUpnpArgument *arg)
{
	char tmp[6];
	int vol_v;

	if (!ddev || !ddev->dd_getvolume)
		return -1;

	vol_v = ddev->dd_getvolume();
	sprintf(tmp, "%d", vol_v);

	cg_upnp_argument_setvalue(arg, tmp);

	return vol_v;
}

static int __dlnac_setmute(struct dlna_device *ddev)
{
	if (!ddev || !ddev->dd_setmute)
		return -1;

	return ddev->dd_setmute();
}

static int __dlnac_gettransport_info(struct dlna_device *ddev,CgUpnpArgument *arg)
{
	int ret = 0;
	char trans[20]={0};
	if (!ddev || !ddev->dd_setmute)
		return -1;
	
	
	ret = ddev->dd_gettransport_info(trans);
	if(ret != -1)
	{
		cg_upnp_argument_setvalue(arg, trans);
	}
	return ret;
}
static int __dlnac_gettrackduration(struct dlna_device *ddev,CgUpnpArgument *arg)
{
	char tmp[16];
	int postion = 0;

	if (!arg || !ddev || !ddev->dd_gettrackduration)
		return -1;

	postion = ddev->dd_gettrackduration();
	sprintf(tmp, "%02d:%02d:%02d", postion / 3600, (postion % 3600)/60, postion % 60);
	cg_upnp_argument_setvalue(arg, tmp);
	return 0;
}



static int __dlnac_getmute(struct dlna_device *ddev)
{
	if (!ddev || !ddev->dd_getmute)
		return -1;

	return ddev->dd_getmute();
}

int dlnac_command(CgUpnpAvRenderer *dmr, CgUpnpArgument *arg, int cmd)
{
	struct dlna_device *ddev;
	int ret = -1;
	
	if (!dmr)
		return -1;

	ddev = (struct dlna_device *)dmr->private;
	if (ddev->dd_type != DDT_DMR)
		return -1;
	
	switch(cmd)
	{
		case DLNAC_SETMETA:
			ret = __dlnac_setmeta(ddev, arg);
			break;
		case DLNAC_SETURL:
			ret = __dlnac_seturl(ddev, arg);
			break;
		case DLNAC_PLAY:
			ret = __dlnac_play(ddev);
			break;
		case DLNAC_STOP:
			ret = __dlnac_stop(ddev);
			break;
		case DLNAC_PAUSE:
			ret = __dlnac_pause(ddev);
			break;
		case DLNAC_RESUME:
			ret = __dlnac_resume(ddev);
			break;
		case DLNAC_SEEK:
			ret = __dlnac_seek(ddev, arg);
			break;
		case DLNAC_GETPOSTION:
			ret = __dlnac_getpostion(ddev, arg);
			break;
		case DLNAC_SETVOLUME:
			ret = __dlnac_setvolume(ddev, arg);
			break;
		case DLNAC_GETVOLUME:
			ret = __dlnac_getvolume(ddev, arg);
			break;
		case DLNAC_SETMUTE:
			ret = __dlnac_setmute(ddev);
			break;
		case DLNAC_GETMUTE:
			ret = __dlnac_getmute(ddev);
			break;
		case DLNAC_GETTRANSPORT_INFO:
			ret = __dlnac_gettransport_info(ddev, arg);
			break;
		case DLNAC_GETTRACKDURATION:
			ret = __dlnac_gettrackduration(ddev, arg);
			break;
		default:
			break;
	}
		
	return ret;

}
