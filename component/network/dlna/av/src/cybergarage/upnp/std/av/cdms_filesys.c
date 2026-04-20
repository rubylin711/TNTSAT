/************************************************************
*
*	CyberLink for C
*
*	Copyright (C) Satoshi Konno 2005
*
*	File: cdms_filesys.c
*
*	Revision:
*       05/11/05
*               - first release.
*
************************************************************/

#include <cybergarage/upnp/std/av/cdms_filesys.h> 
#include <cybergarage/upnp/std/av/cmediaserver.h>
#include <cybergarage/net/cinterface.h>

#include <stdio.h>
#include <ctype.h>

#if defined(WIN32)
#include <windows.h>
#include <tchar.h>
#if !defined(WINCE)
#include <sys/stat.h>
#include <conio.h>
#endif
#else
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#endif
#include "mt_base64.h"
#include "mt_cdlna.h"

/****************************************
* cg_upnp_dms_filesys_getmimetype
****************************************/

char *cg_upnp_dms_filesys_getmimetype(char *ext)
{
	if (cg_strcaseeq(ext, "jpeg") || cg_strcaseeq(ext, "jpg"))
		return CG_UPNPAV_MIMETYPE_JPEG;
	if (cg_strcaseeq(ext, "mpeg") || cg_strcaseeq(ext, "mpg"))
		return CG_UPNPAV_MIMETYPE_MPEG;
	if (cg_strcaseeq(ext, "mp3"))
		return CG_UPNPAV_MIMETYPE_MP3;
	return NULL;
}

/****************************************
* cg_upnp_dms_filesys_getupnpclass
****************************************/

char *cg_upnp_dms_filesys_getupnpclass(char *ext)
{
	if (cg_strcaseeq(ext, "jpeg") || cg_strcaseeq(ext, "jpg"))
		return CG_UPNPAV_UPNPCLASS_PHOTO;
	if (cg_strcaseeq(ext, "mpeg") || cg_strcaseeq(ext, "mpg"))
		return CG_UPNPAV_UPNPCLASS_MOVIE;
	if (cg_strcaseeq(ext, "mp3"))
		return CG_UPNPAV_UPNPCLASS_MUSIC;
	return NULL;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

extern char* dlna_get_ipaddr(CgUpnpMediaServer* dms);
extern int dlna_filter_match(CgUpnpMediaServer* dms, char* name);

void __mt_addfilest(CgUpnpMediaServer *dms, CgUpnpMediaContent *content,
					char* path, void* svr)
{
	DIR* pdir;
	struct dirent *ent;
	char file[512];
	char basebuf[512];
	CgUpnpMediaContent *con;
	CgString *fullPathStr;
	char idmd5[64];
	CgUpnpMediaResource *res;
	char *resURL = file;
	int len, idx = 0;

	pdir = opendir(path);
	if (!pdir) 
		return ;

	while ((ent = readdir(pdir)) != NULL) {
		if (strcmp(ent->d_name, ".") == 0 ||
			strcmp(ent->d_name, "..") == 0)
			continue;
		if (ent->d_type & 4) {
			sprintf(file, "%s/%s", path, ent->d_name);
			__mt_addfilest(dms, content, file, svr);
			continue;
		}

		if (!dlna_filter_match(dms, ent->d_name))
			continue;
		
		sprintf(file, "%s/%s", path, ent->d_name);
		len = mt_base64_encode(file, strlen(file), basebuf);
		basebuf[len] = 0;

		fullPathStr = cg_string_new();
		cg_string_addvalue(fullPathStr, path);
		cg_string_addvalue(fullPathStr, "/");
        cg_string_addvalue(fullPathStr, ent->d_name);

        con = cg_upnpav_content_new();
		cg_str2md5((char*)cg_string_getvalue(fullPathStr), idmd5);
		cg_upnpav_content_setid(con, idmd5);
		cg_upnpav_content_setparentid(con, cg_upnpav_content_getid(content));
		cg_upnpav_content_settitle(con, ent->d_name);
		if (ent->d_type & 4)
			cg_upnpav_content_settype(con, CG_UPNPAV_CONTENT_CONTAINER);
		else
			cg_upnpav_content_settype(con, CG_UPNPAV_CONTENT_ITEM);
		cg_upnpav_content_setupnpclass(con, CG_UPNPAV_UPNPCLASS_MOVIE);


		cg_upnp_dms_filesys_content_setpubicdirectory(con, cg_string_getvalue(fullPathStr));

        cg_upnpav_content_addchildcontent(content, con);
		res = cg_upnpav_resource_new();
		sprintf(resURL, "http://%s:%d/%s", dlna_get_ipaddr(dms), mt_get_svrport(svr), basebuf);
		cg_upnpav_resource_seturl(res, resURL);
		cg_upnpav_content_addresource(con, res);
        cg_upnpav_dms_condir_updatesystemupdateid(dms);
        cg_string_delete(fullPathStr);
	
	}

	closedir(pdir);
}
CgUpnpMediaContent * cg_upnp_dms_addentry(CgUpnpMediaServer *dms, CgUpnpMediaContentList *parentCon, char *name)
{
	CgUpnpMediaContent *con;
	CgUpnpMediaContent *rootContent;
	char idmd5_sat[256];
	
	rootContent = dms->rootContent;

	con = cg_upnpav_content_new();

	/* id */
	cg_str2md5(name, idmd5_sat);
	cg_upnpav_content_setid(con, idmd5_sat);

	/* title */
	cg_upnpav_content_settitle(con, name);

	/* parent id */
	//cg_upnp_media_content_setparentid(con, cg_upnp_media_content_getid(parentCon));
	cg_upnpav_content_setparentid(con, cg_upnpav_content_getid(rootContent));
	/* type */
	cg_upnpav_content_settype(con, CG_UPNPAV_CONTENT_CONTAINER);

	/* upnp:class */
	cg_upnpav_content_setupnpclass(con, "object.container");

	cg_upnp_dms_filesys_content_setpubicdirectory(con, name);

	cg_upnpav_content_addchildcontent(rootContent, con);
	cg_upnpav_dms_condir_updatesystemupdateid(dms);
	return con;
}

/****************************************
* cg_upnp_dms_filesys_updatecontentlist
****************************************/

void cg_upnp_dms_filesys_updatecontentlist(CgUpnpMediaServer *dms, 
		CgUpnpMediaContentList *parentCon, char *pubdir)
{
	CgUpnpMediaContent *con;
	CgUpnpMediaResource *res;
	CgNetworkInterfaceList *netIfList;
	CgNetworkInterfaceList *netIf;
	int conType;
	char *fileName;
	CgString *fullPathStr;
	int fileExtIdx;
	char *fileExt;
	char idmd5[CG_MD5_STRING_BUF_SIZE];
	char resURL[CG_UPNPAV_FILESYS_RESURL_MAXLEN];
	char *mimeType;
	char *upnpClass;
	CgUpnpMediaFileSystemContentData *conData;
	char dlnaAttr[CG_UPNPAV_DLNAATTR_MAXLEN];
#if !defined(WINCE)
	struct stat fileStat;
	off_t fileSize;
#else
	fpos_t fileSize;
	FILE *fp;
#endif
#if defined(WIN32)
	CgString *findDirStr;
	#if defined(UNICODE)
	TCHAR wCharBuf[MAX_PATH];
	char mCharBuf[MAX_PATH];
	#endif
	WIN32_FIND_DATA fd;
	MT_HANDLE_T hFind;
#else
	struct dirent **fileList;
	int n;
#endif

	return ;
	for (con=cg_upnpav_content_getchildcontents(parentCon); con; con=cg_upnpav_content_next(con)) {
		conData =cg_upnp_dms_filesys_content_getdata(con);
		if (!conData)
			continue;
		cg_upnp_dms_filesys_content_data_delete(conData);
	}
	cg_upnpav_content_clearchildcontents(parentCon);

	if (cg_strlen(pubdir) <= 0)
		return;

	netIfList = cg_net_interfacelist_new();
	cg_net_gethostinterfaces(netIfList);
	netIf = cg_net_interfacelist_gets(netIfList);
	if (!netIf) {
		cg_net_interfacelist_delete(netIfList);
		return;
	}

	conType = CG_UPNPAV_CONTENT_NONE;

#if defined(WIN32)
	findDirStr = cg_string_new();
	cg_string_addvalue(findDirStr, pubdir);
	cg_string_addvalue(findDirStr, "\\*.*");
	#if defined(UNICODE)
	MultiByteToWideChar(CP_UTF8, 0, cg_string_getvalue(findDirStr), -1, wCharBuf, (MAX_PATH-1));
	hFind = FindFirstFile(wCharBuf, &fd);
	#else
	hFind = FindFirstFile(cg_string_getvalue(findDirStr), &fd);
	#endif
	if (hFind != INVALID_HANDLE_VALUE) {
		do{
			#if defined(UNICODE)
			WideCharToMultiByte(CP_ACP, 0, fd.cFileName, -1, mCharBuf, (MAX_PATH-1), NULL, NULL);
			fileName = cg_strdup(mCharBuf);
			#else
			fileName = cg_strdup(fd.cFileName);
			#endif
			if (!cg_streq(".", fileName) && !cg_streq("..", fileName)) {
				fullPathStr = cg_string_new();
				cg_string_addvalue(fullPathStr, pubdir);
				cg_string_addvalue(fullPathStr, "\\");
				cg_string_addvalue(fullPathStr, fileName);
				conType = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? CG_UPNPAV_CONTENT_CONTAINER : CG_UPNPAV_CONTENT_ITEM;
#else
	n = scandir(pubdir, &fileList, 0, alphasort);
	if (0 <= n) {
		while(n--) {
			if (!cg_streq(".", fileList[n]->d_name) && !cg_streq("..", fileList[n]->d_name)) {
				fileName = cg_strdup(fileList[n]->d_name);
				fullPathStr = cg_string_new();
				cg_string_addvalue(fullPathStr, pubdir);
				cg_string_addvalue(fullPathStr, "/");
				cg_string_addvalue(fullPathStr, fileList[n]->d_name);
				if(stat(cg_string_getvalue(fullPathStr), &fileStat) != -1)
					conType = ((fileStat.st_mode & S_IFMT)==S_IFDIR) ? CG_UPNPAV_CONTENT_CONTAINER : CG_UPNPAV_CONTENT_ITEM;
#endif
				if (conType == CG_UPNPAV_CONTENT_NONE)
					continue;

				mimeType = NULL;
				if ((conType == CG_UPNPAV_CONTENT_ITEM)) {
					fileExtIdx = cg_strrchr(fileName, ".", 1);
					if (0 < fileExtIdx) {
						fileExt = (fileName + fileExtIdx + 1);
						fileName[fileExtIdx] = '\0';
						mimeType = cg_upnp_dms_filesys_getmimetype(fileExt);
						upnpClass = cg_upnp_dms_filesys_getupnpclass(fileExt);
					}
					if (!mimeType || !upnpClass) {
						free(fileName);
						cg_string_delete(fullPathStr);
						continue;
					}
				}

				con = cg_upnpav_content_new();

				/* id */
				cg_str2md5((char *)cg_string_getvalue(fullPathStr), idmd5);
				cg_upnpav_content_setid(con, idmd5);

				/* title */
				cg_upnpav_content_settitle(con, fileName);

				/* parent id */
				cg_upnpav_content_setparentid(con, cg_upnpav_content_getid(parentCon));

				/* type */
				cg_upnpav_content_settype(con, conType);

				/* upnp:class */
				cg_upnpav_content_setupnpclass(con, upnpClass);

				/* url */
				if ((conType == CG_UPNPAV_CONTENT_ITEM)) {
					res = cg_upnpav_resource_new();
					sprintf(resURL, "http://%s:%d/%s/%s",
						cg_net_interface_getaddress(netIf),
						cg_upnp_device_gethttpport(cg_upnpav_dms_getdevice(dms)),
						CG_UPNPAV_FILESYS_RESURL_PATH,
						idmd5
						);
					cg_upnpav_resource_seturl(res, resURL);
					cg_upnpav_resource_setmimetype(res, mimeType);
					cg_upnpav_resource_setdlnaattribute(res, cg_upnpav_resource_getdlnaattributes(res, dlnaAttr, sizeof(dlnaAttr)));
					cg_upnpav_content_addresource(con, res);

					/* size */
#if !defined(WINCE)
					fileSize = 0;
					if (stat(cg_string_getvalue(fullPathStr), &fileStat) == 0)
						fileSize = fileStat.st_size;
#else
					fp = fopen(cg_string_getvalue(fullPathStr), "rb");
					if (fp) {
						fseek(fp, 0, SEEK_END); 
						fgetpos(fp, &fileSize); 
						fclose(fp);
					}
#endif
					cg_upnpav_resource_setsize(res, (long)fileSize);
				}

				cg_upnp_dms_filesys_content_setpubicdirectory(con, cg_string_getvalue(fullPathStr));

				cg_upnpav_content_addchildcontent(parentCon, con);
				cg_upnpav_dms_condir_updatesystemupdateid(dms);

				free(fileName);
				cg_string_delete(fullPathStr);
				
				conType = CG_UPNPAV_CONTENT_NONE;
#if defined(WIN32)
			}
		} while(FindNextFile(hFind,&fd) != FALSE);
		FindClose(hFind);
	}
	cg_string_delete(findDirStr);
#else
			}
			free(fileList[n]);
		}
		free(fileList);
	}
#endif

	cg_net_interfacelist_delete(netIfList);
}

/****************************************
* cg_upnp_dms_filesys_updaterootcontentlist
****************************************/

void cg_upnp_dms_filesys_updaterootcontentlist(CgUpnpMediaServer *dms)
{
	char *pubdir;
	CgUpnpMediaContent *rootContent;

	pubdir = cg_upnp_dms_filesys_getpublicationdirectory(dms);
	if (!pubdir)
		return;

	rootContent = cg_upnpav_dms_getrootcontent(dms);

	if (!rootContent)
		return;

	cg_upnpav_dms_lock(dms);

	cg_upnp_dms_filesys_updatecontentlist(dms, rootContent, pubdir);

	cg_upnpav_dms_unlock(dms);
}

/****************************************
* cg_upnp_dms_filesys_setpublicationdirectory
****************************************/

void cg_upnp_dms_filesys_setpublicationdirectory(CgUpnpMediaServer *dms, char *pubdir)
{
	CgUpnpMediaContent *rootContent;

	rootContent = cg_upnpav_dms_getrootcontent(dms);
	if (!rootContent)
		return;

	cg_upnp_dms_filesys_content_setpubicdirectory(rootContent, pubdir);
}

/****************************************
* cg_upnp_dms_filesys_getpublicationdirectory
****************************************/

char *cg_upnp_dms_filesys_getpublicationdirectory(CgUpnpMediaServer *dms)
{
	CgUpnpMediaContent *rootContent;

	rootContent = cg_upnpav_dms_getrootcontent(dms);
	if (!rootContent)
		return NULL;

	return cg_upnp_dms_filesys_content_getpubicdirectory(rootContent);
}

/****************************************
* cg_upnp_dms_filesys_actionlistner
****************************************/

MT_BOOL cg_upnp_dms_filesys_actionlistner(CgUpnpAction *action)
{
	CgUpnpMediaServer *dms;
	CgUpnpDevice *dev;
	char *contentID;
	CgUpnpMediaContent *content;
	char *contentDir;
	char *actionName;

	actionName = cg_upnp_action_getname(action);
	if (cg_strlen(actionName) <= 0)
		return FALSE;

	/* Browse */
	if (cg_streq(actionName, CG_UPNPAV_DMS_CONTENTDIRECTORY_BROWSE)) {
		contentID = cg_upnp_action_getargumentvaluebyname(action, CG_UPNPAV_DMS_CONTENTDIRECTORY_BROWSE_OBJECT_ID);
		if (cg_strlen(contentID) <=0)
			return FALSE;

		dev = cg_upnp_service_getdevice(cg_upnp_action_getservice(action));
		if (!dev)
			return FALSE;

		dms = (CgUpnpMediaServer *)cg_upnp_device_getuserdata(dev);
		if (!dms)
			return FALSE;

		cg_upnpav_dms_lock(dms);
		
		content = cg_upnpav_dms_findcontentbyid(dms, contentID);
		if (!content) {
			cg_upnpav_dms_unlock(dms);
			return FALSE;
		}

		contentDir = (char *)cg_upnp_dms_filesys_content_getpubicdirectory(content);
		if (cg_strlen(contentDir) <= 0) {
			cg_upnpav_dms_unlock(dms);
			return FALSE;
		}

		cg_upnp_dms_filesys_updatecontentlist(dms, content, contentDir);
		
		cg_upnpav_dms_unlock(dms);
	}

	return FALSE;
}

/****************************************
* cg_upnp_dms_filesys_new
****************************************/

CgUpnpMediaServer *cg_upnp_dms_filesys_new()
{
	CgUpnpMediaServer *dms;

	dms = cg_upnpav_dms_new();

	cg_upnpav_dms_setactionlistener(dms, cg_upnp_dms_filesys_actionlistner);
	return dms;
}

/****************************************
* cg_upnp_dms_filesys_delete
****************************************/

void cg_upnp_dms_filesys_delete(CgUpnpMediaServer *dms)
{
	cg_upnpav_dms_delete(dms);
}

void cg_upnp_dump_content(CgUpnpAvContent *objectContent)
{
	CgUpnpAvContent *content;

	if (!objectContent)
		return;

	for (content=cg_upnpav_content_getchildcontents(objectContent); content; content=cg_upnpav_content_next(content)) {
		cg_upnp_dump_content(content);
	}

}

void cg_upnp_dms_add_content(CgUpnpMediaServer *dms, void* arg,
					char* path, char* name)
{
	CgUpnpMediaContent *con;

	con = cg_upnp_dms_addentry(dms, NULL, name);
	__mt_addfilest(dms, con, path, arg);
}
