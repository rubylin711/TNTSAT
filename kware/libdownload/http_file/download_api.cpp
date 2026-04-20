/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include  <stdio.h>
#include <ctype.h>
#include <sys/time.h>
#include "mt_type.h"
//#include "osal_mtos.h"
#include "mtos_misc.h"
#include "mtos_task.h"

//#include  "http_download.h"
#include  "http_download_mini.h"
#include  "httpc.h"
//#include "zlib.h"
//#include "eventSink.h"
#include  "download_api.h"

#include "download_manager.h"
#include <openssl/sha.h>

#ifdef TEST_SPEED_OPEN
#include "commonData.h"
#endif
#include "zlib.h"

static void parse_url_component(parse_youtube_vedio_info_t * p_video_info, char * keyword, int index);

///for wanda ,
#define DISABLE_ZLIB
#undef DISABLE_ZLIB

#define   DBG_DOWNLOAD_API
#undef DBG_DOWNLOAD_API
#ifdef     DBG_DOWNLOAD_API
#define DOWNLOAD_API_LOG(format, args...)              printf(format, ##__VA_ARGS__)
#define DOWNLOAD_API_WARNING(format, args...)     printf(format, ##__VA_ARGS__)
#define DOWNLOAD_API_DEBUG(format, args...)          printf(format, ##__VA_ARGS__)
#define DOWNLOAD_API_ERROR(format, args...)          printf(format, ##__VA_ARGS__)
#else
#define DOWNLOAD_API_LOG(format, ...)               printf(format, ##__VA_ARGS__)
#define DOWNLOAD_API_WARNING(format, ...)
#define DOWNLOAD_API_DEBUG(format, ...)
#define DOWNLOAD_API_ERROR(format, ...)           printf(format, ##__VA_ARGS__)
#endif

#define  MAX_TMP_URL_LEN       (4096)
#define  MAX_FILE_NAME_LEN     (2048)
#define  LIVE_INDEX_URL_LEN    (4096)

#define    MAX_POST_COUNT  	(4)
#define    DEFAULT_DL_HTML_TIMEOUT  15
#define     MAX_VIDEOID_LEN  (32)
static char  youtubeServerUrl[256] = "https://47.241.5.241/";
static const char* HASH_SALT_YTS = "IjLJxcN3qK4=";
static int  server_ret_val = -2;

dash_youtube_playurl_info_t *g_dash_youtube = NULL;
int g_only_use_youtube_dash = 0;// 0 default, 360p 720p mp4 +  dash, 1 only dash.
char const *youtube_new_key_str1 = "adaptiveFormats";
char const *youtube_new_key_str2 = "streamingData";
int delete_file(char * path)
{
     remove(path);
	return 0;
}
#ifndef TEST_SPEED_OPEN
/* Converts a hex character to its integer value */
static char from_hex(char ch) {
	return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
static char to_hex(char code) {
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}
static int url_encode(const char *src, char *dest)
{
	const char *pstr = src;
	char *pbuf = dest;

	if (src == NULL || dest == NULL)
	{
		return -1;
	}
	while (*pstr)
	{
		if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
			*pbuf++ = *pstr;
		else if (*pstr == ' ')
			*pbuf++ = '+';
		else
			*pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
		pstr++;
	}
	*pbuf = '\0';
	return pbuf - dest;
}
static int url_decode(const char *src, char *dest)
{
	const char *pstr = src;
	char *pbuf = dest;

	if (src == NULL || dest == NULL)
	{
		return -1;
	}
	while (*pstr)
	{
		if (*pstr == '%')
		{
			if (pstr[1] && pstr[2])
			{
				*pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
				pstr += 2;
			}
		}
		else if (*pstr == '+')
		{
			*pbuf++ = ' ';
		}
		else
		{
			*pbuf++ = *pstr;
		}
		pstr++;
	}
	*pbuf = '\0';
	return pbuf - dest;
}

#endif


#define    DECOMPRESSED_BUFFER_LEN   (512*1024)
#ifndef  DISABLE_ZLIB
static int decompress_to_file(const char * srcFile, const char * dstFile)
{
    DOWNLOAD_API_DEBUG("[%s:%s:%d] start !\n", __FILE__,__func__,__LINE__);
    DOWNLOAD_API_DEBUG("srcFile[%s],dstFile[%s]\n", srcFile, srcFile);
    int ret = 0;

    int uncomprLen =  DECOMPRESSED_BUFFER_LEN;
    unsigned char *uncompr = (unsigned char*)malloc(uncomprLen);
    memset(uncompr, 0x00, DECOMPRESSED_BUFFER_LEN);

    gzFile file;

    file = gzopen(srcFile, "rb");
    if (file == NULL) {
        DOWNLOAD_API_ERROR("gzopen error \n");
        ret  = 0;
    }
    else
    {
        strcpy((char*)uncompr, "garbage");
        int len  = gzread(file, uncompr, (unsigned)uncomprLen) ;
        gzclose(file);

        DOWNLOAD_API_DEBUG("gzread(): len = %d \n", len);
        if(1)
        {
            FILE *fp_out = fopen(dstFile,"wb");
            if(!fp_out)
            {
                DOWNLOAD_API_ERROR("[%s][ERROR] fail to open local file[r:yiyuan.txt] !!!\n", __func__);
                free(uncompr);
                return 0;
            }

            fwrite(uncompr,len,1,fp_out);
            fclose(fp_out);
        }
        ret  = 1;
    }
    free(uncompr);
    uncompr = NULL;
    return ret;
}

static int rename_file(const char * srcFile, const char * dstFile)
{
    rename(srcFile,dstFile);
    return 1;
}
#endif
static int DownloadAndGetHeader(const char * url,
                          const char * tempFile,
                          unsigned int timeoutSec,
                          void * response,
                          void * arg,
                          const char * extraHeaders,
                          const char * body,
                          unsigned int bodyLen,
                          HTTP_rsp_header_t *rsp_header)
{
    DOWNLOAD_API_DEBUG("[%s] start start ...\n", __func__);
    DOWNLOAD_API_DEBUG("[%s] timeout %d seconds\n", __func__, timeoutSec);

    if (url == NULL) {
        DOWNLOAD_API_ERROR("[%s] url is NULL, please check it\n", __func__);
        return 0;
    }

    DOWNLOAD_API_DEBUG("[%s] url [%s]...\n", __func__, url);
    char tmpurl[MAX_TMP_URL_LEN];
    char file[MAX_FILE_NAME_LEN];
    memset(tmpurl, 0, MAX_TMP_URL_LEN);
    sprintf(tmpurl, "%s", url);
    memset(file, 0, MAX_FILE_NAME_LEN);
    sprintf(file, "%s", tempFile);
    http_download_mini instance;
    http_download_mini * dl = &instance;

    if (dl == NULL) {
        DOWNLOAD_API_ERROR("[%s] [ERROR] invalid dl (NULL) ...!!!\n", __func__);
        return 0;
    }

    HttpDownloadMiniSpeed loadresult = dl->download(tmpurl, timeoutSec*1000, file, NULL, false, extraHeaders, body, bodyLen,rsp_header);

    DOWNLOAD_API_DEBUG("[%s] download result state: [%d] (0:COMPLETE; 1:ERR; 2:TIMEOUT; 3:ABORT; 4:ABORT by user\n", __func__, loadresult.state);
    DOWNLOAD_API_DEBUG("[%s] download size: [%d]\n", __func__, loadresult.size);

    if(loadresult.state == HTTP_MINI_ABORT_BY_USER)
    {
        delete_file(file);
        return -1;
    }

    if(loadresult.state != HTTP_MINI_COMPLETE || loadresult.size< 0)
    {
        delete_file(file);
        return 0;
    }

    DOWNLOAD_API_DEBUG("[%s] end end...\n", __func__);
    return 1;
}

static       MT_BOOL  bAbortFlag = FALSE;

void   Nw_Download_Init_Download_Manager()
{
    http_download_mamager_init();
}

void   Nw_Download_Init(int prio, unsigned char * p_mem, int size)
{
    //http_download * dl = http_download::get_http_manager(prio, p_mem, size);
}

void   Nw_Download_Deinit(void)
{
    //http_download *dl = http_download::get_http_manager(prio,p_mem);
}

void  Nw_Download_SetAbortFlag(MT_BOOL flag)
{
    //bAbortFlag = flag;
}

void  Nw_Download_Abort(http_download_mini * instance)
{
    //http_download_mini * dl = &instance;
    DOWNLOAD_API_DEBUG("[Nw_Download_Abort] start start... ...\n");
    if(!instance)
        return;

    instance->abort();
    DOWNLOAD_API_DEBUG("[Nw_Download_Abort] done done... ...\n");
}
/*
*
*
*   RETURN VALUE:
*         0 means fail and  1 means success -1 means aborted by user
*
*
*
*/
int Nw_DownloadURLTimeout(const char * url,
                          const char * tempFile,
                          unsigned int timeoutSec,
                          void * response,
                          void * arg,
                          const char * extraHeaders,
                          const char * body,
                          unsigned int bodyLen)
{
    DOWNLOAD_API_DEBUG("[%s] start start ...\n", __func__);
    DOWNLOAD_API_DEBUG("[%s] timeout %d seconds\n", __func__, timeoutSec);

    if (url == NULL) {
        DOWNLOAD_API_ERROR("[%s] url is NULL, please check it\n", __func__);
        return 0;
    }

    DOWNLOAD_API_DEBUG("[%s] url [%s]...\n", __func__, url);
    char tmpurl[MAX_TMP_URL_LEN];
    char file[MAX_FILE_NAME_LEN];
    memset(tmpurl, 0, MAX_TMP_URL_LEN);
    sprintf(tmpurl, "%s", url);
    memset(file, 0, MAX_FILE_NAME_LEN);
    sprintf(file, "%s", tempFile);
    http_download_mini instance;
    http_download_mini * dl = &instance;

    if (dl == NULL) {
        DOWNLOAD_API_ERROR("[%s] [ERROR] invalid dl (NULL) ...!!!\n", __func__);
        return 0;
    }

    HttpDownloadMiniSpeed loadresult = dl->download(tmpurl, timeoutSec*1000, file, NULL, false, extraHeaders, body, bodyLen,NULL);

    DOWNLOAD_API_DEBUG("[%s] download result state: [%d] (0:COMPLETE; 1:ERR; 2:TIMEOUT; 3:ABORT; 4:ABORT by user\n", __func__, loadresult.state);
    DOWNLOAD_API_DEBUG("[%s] download size: [%d]\n", __func__, loadresult.size);

    if(loadresult.state == HTTP_MINI_ABORT_BY_USER)
    {
    	 delete_file(file);
        return -1;
    }

    if(loadresult.state != HTTP_MINI_COMPLETE || loadresult.size< 0)
    {
    	 delete_file(file);
        return 0;
    }

    DOWNLOAD_API_DEBUG("[%s] end end...\n", __func__);
    return 1;
}
#ifndef DISABLE_ZLIB
int Nw_DownloadURLTimeout_Gzip(const char * url,
                          const char * tempFile,
                          unsigned int timeoutSec,
                          void * response,
                          void * arg,
                          const char * extraHeaders,
                          const char * body,
                          unsigned int bodyLen)
{
    DOWNLOAD_API_DEBUG("[%s] start start ...\n", __func__);
    DOWNLOAD_API_DEBUG("[%s] timeout %d seconds\n", __func__, timeoutSec);

    if (url == NULL) {
        DOWNLOAD_API_ERROR("[%s] url is NULL, please check it\n", __func__);
        return 0;
    }

    DOWNLOAD_API_DEBUG("[%s] url [%s]...\n", __func__, url);

    char extraHeaders_t[512] = {0};
    memset(extraHeaders_t, 0, 512);
    DOWNLOAD_API_DEBUG("[%s] extraHeaders [%s]...\n", __func__, extraHeaders);
    if (extraHeaders && strlen(extraHeaders) > 0)
    {
     DOWNLOAD_API_DEBUG("[%s] extraHeaders !=NULL...\n", __func__);
        strcpy(extraHeaders_t, extraHeaders);

        if(extraHeaders[strlen(extraHeaders) - 2] == '\r' && extraHeaders[strlen(extraHeaders) - 1] == '\n') {
            strcat(extraHeaders_t, "Accept-Encoding: gzip\r\n");
        } else {
            strcat(extraHeaders_t, "\r\nAccept-Encoding: gzip\r\n");
        }
    } else {
        strcpy(extraHeaders_t, "Accept-Encoding: gzip\r\n");
    }
    DOWNLOAD_API_DEBUG("[%s] extraHeaders_t [%s]...\n", __func__, extraHeaders_t);

    char temp_gzip_file[64] = {0};
    //mtos_task_info_t info = {0};
    //mtos_task_get_info(&info);
    sprintf((char*)temp_gzip_file,"%s_zlibfile",tempFile);
    DOWNLOAD_API_DEBUG("[%s] temp_gzip_file =%s...\n", __func__, temp_gzip_file);

    char responsekey0[32] = "Content-Encoding";
    char responsevalue0[8];
    memset(responsevalue0, 0, sizeof(responsevalue0));
    char responsekey1[32] = "content-encoding";
    char responsevalue1[8];
    memset(responsevalue1, 0, sizeof(responsevalue1));

    HTTP_rsp_header_t * rsp_header = (HTTP_rsp_header_t *)malloc(sizeof(HTTP_rsp_header_t));
    memset(rsp_header, 0, sizeof(HTTP_rsp_header_t));

    rsp_header->key[0] = responsekey0;
    rsp_header->value_buf[0] = responsevalue0;
    rsp_header->value_buflen[0] = 8;

    rsp_header->key[1] = responsekey1;
    rsp_header->value_buf[1] = responsevalue1;
    rsp_header->value_buflen[1] = 8;

    int req_ret = DownloadAndGetHeader(url, temp_gzip_file, 10,NULL,NULL,extraHeaders_t,NULL,0, rsp_header);
    DOWNLOAD_API_DEBUG("[%s] responsevalue0=[%s], responsevalue1=[%s]...!!!\n", __func__, responsevalue0, responsevalue1);
    DOWNLOAD_API_DEBUG("[%s] req_ret [%d]...\n", __func__, req_ret);
    if(req_ret == 1 && ((strcmp(responsevalue0, "gzip") == 0) ||(strcmp(responsevalue1, "gzip") == 0) || strstr(url, ".gz")))
    {   
        int ret = decompress_to_file(temp_gzip_file, tempFile);
	delete_file(temp_gzip_file);

        return ret;
    }
    else
    {
        if(req_ret == 1)
        {
            rename_file(temp_gzip_file, tempFile);
        }
        delete_file(temp_gzip_file);

        return req_ret;
    }
}
#endif

static void local_strlwr(char *s)
{
	char *str;
	str = s;
	while(*str != '\0')
	{
		if(*str > 'A' && *str < 'Z'){
			*str += 'a'-'A';
		}
		str++;
	}
//	return s;
}
#ifdef TEST_SPEED_OPEN
static DownloadResult downloadurl_testspeed(const char * url,
                             unsigned int timeoutMS, TestSpeedCtrl * para)
{
    DownloadResult result;
    result.speed= -1;
    result.connectMS= -1;
    result.downloadMS= -1;
    result.errcode= TEST_SPEED_OK;

    DOWNLOAD_API_LOG("[%s] start start ...\n", __func__);
    DOWNLOAD_API_LOG("[%s] timeout %d ms\n", __func__, timeoutMS);

    TEST_SPEED_WHITE_BLACK_LIST*p_hdl = get_live_speed_list_xml_handle();
    //When run this function the first time, we should parse the blacklist which was defined in webset.
    if(!(p_hdl && p_hdl->black_list_count > 0 && p_hdl->black_list[0].value))
    {
        parse_url_white_black_list();
    }
    DOWNLOAD_API_DEBUG("[%s]  black_list_count [%d]  ! \n", __func__, p_hdl->black_list_count);


    if (url == NULL || para == NULL) {
        DOWNLOAD_API_ERROR("[%s][ERROR] url or user prarameters is NULL, please check it\n", __func__);
        result.errcode = ERR_INVALID_PARAM1;
        return result;
    }

    if(timeoutMS < 1000){
        DOWNLOAD_API_ERROR("[%s][ERROR] no time to do test speed, left time %d ms\n", __func__, timeoutMS);
        result.errcode = ERR_INVALID_TIMEOUT;
        return result;
    }

    DOWNLOAD_API_DEBUG("[%s] url [%s]...\n", __func__, url);

    char cmpurl[MAX_TMP_URL_LEN];
    memset(cmpurl, 0, MAX_TMP_URL_LEN);
    sprintf(cmpurl, "%s", url);
    local_strlwr(cmpurl) ;

    //Skip the blacklist
    for(int i=0; i < p_hdl->black_list_count; i++)
    {
        if (strstr(cmpurl, p_hdl->black_list[i].value)) {
            DOWNLOAD_API_ERROR("we find %s in this url ,so skip it\n", p_hdl->black_list[i].value);
            result.speed = -1;
            result.errcode = (TEST_SPEED_ERR_T)(p_hdl->black_list[i].errorcode);
            return result;
        }
    }

    char tmpurl[MAX_TMP_URL_LEN];
    HttpDownloadMiniSpeed loadresult;
    memset(tmpurl, 0, MAX_TMP_URL_LEN);
    sprintf(tmpurl, "%s", url);
    http_download_mini instance;
    http_download_mini * dl = &instance;

    if (dl == NULL) {
        DOWNLOAD_API_ERROR("[%s] [ERROR] invalid dl (NULL) ...!!!\n", __func__);
        result.errcode = ERR_INVALID_DL_HDL;
        return result;
    }

    mtos_task_info_t info = {0};
    mtos_task_get_info(&info);
    if(check_task_download_running((int)info.prio))
    {
        abort_download_task((int)info.prio, TRUE);
        remove_task_from_manager((int)info.prio);
    }
    add_task_to_manager((int)info.prio, TRUE, dl);

    u32 speedstart = mtos_ticks_get();
    loadresult = dl->download(tmpurl, timeoutMS, para->writeFileName, &(para->abort_flag), false, NULL, NULL, 0,NULL);
    u32 speedend = mtos_ticks_get();

    remove_task_from_manager((int)info.prio);

    if(loadresult.state == HTTP_MINI_ABORT_BY_USER)
    {
        DOWNLOAD_API_ERROR("[%s] [ERROR] Abort by user ...!!!\n", __func__);
        result.errcode = ERR_ABORT_BY_USER;  //exception
        return result;
    }

    u32 cost = (speedend - speedstart) * 10;

    if(loadresult.downloadMS > 0 && loadresult.size > 0) {

        result.speed = loadresult.size/loadresult.downloadMS;
    }
    else if (loadresult.size < 0 && loadresult.downloadMS < 0) {

       result.speed = -1;
     }
    else {
        result.speed = 0;
    }


    result.connectMS= loadresult.connectMS;
    result.downloadMS= loadresult.downloadMS;

    DOWNLOAD_API_DEBUG("[%s] result.speed = %d\n", __func__, result.speed);
    DOWNLOAD_API_DEBUG("[%s] result.connectMS = %ld\n", __func__, result.connectMS);
    DOWNLOAD_API_DEBUG("[%s] result.downloadMS = %ld\n", __func__, result.downloadMS);

    if(result.speed < 0 && result.connectMS < 0 && result.downloadMS < 0)
    {
        result.errcode = ERR_DL_EXCEPTION;  //exception
        return result;
    }

    bool isLive = true;
    char * pNew = (char *)dl->get_direct_ts_url_from_m3u8(&isLive);
    //we check whetherit is a live stream in m3u8
    if (para->skipNonLiveStream && !isLive) {
        DOWNLOAD_API_ERROR("[%s] [WARNNING] this url is not a live stream, we skip it!\n", __func__);
        result.speed = -1;
        result.errcode = ERR_INVALID_LIVE_URL;
        return result;
    }

    if (pNew) {
        if (timeoutMS <= cost) {
           DOWNLOAD_API_DEBUG("[%s] timeoutMS <= downloadMS...,   result.downloadMS = %d\n", __func__, result.downloadMS);
           result.connectMS= timeoutMS;
           result.downloadMS= 0;
           result.speed = 0;
           result.errcode = ERR_TEST_SPEED_TIMEOUT;
           return result;
        }
        DOWNLOAD_API_DEBUG("[%s] we should use direct ts url from m3u8 to redo download... timeout = %d ms\n", __func__, timeoutMS - cost,NULL);
        return Nw_DownloadURL_TestSpeed(pNew, timeoutMS - cost, para);
    }

    char * pFinal = (char *)dl->get_final_url();

    if (pFinal == NULL) {
        DOWNLOAD_API_ERROR("we can't get final url!!!\n");
        result.speed = -1;
        result.errcode = ERR_NOT_FIND_VALID_URL;
        return result;
    }

    memset(cmpurl, 0, MAX_TMP_URL_LEN);
    sprintf(cmpurl, "%s", pFinal);
    local_strlwr(cmpurl) ;

    if (para->skipNonLiveStream || para->skipNonMediaUrl) {
        ////should check the tmpurl is a valid url !
        DOWNLOAD_API_DEBUG("[%s]  the  final url is %s  ! \n", __func__, pFinal);
        //Skip the blacklist
        for(int i=0; i < p_hdl->black_list_count; i++)
        {
            if (strstr(cmpurl, p_hdl->black_list[i].value)) {
                DOWNLOAD_API_ERROR("we find %s in this url ,so skip it\n", p_hdl->black_list[i].value);
                result.speed = -1;
                result.errcode = (TEST_SPEED_ERR_T)(p_hdl->black_list[i].errorcode);
                return result;
            }
        }

        if(para->skipNonMediaUrl)
        {
            /////fix to remove the url which like http://www.qq.com ,
            ////maybe we hava a url , it is like xxx.TS xxx.Ts
            if (strstr(cmpurl, "ts")
                || strstr(cmpurl, "flv")
                || strstr(cmpurl, "mp4")
                || strstr(cmpurl, "/live") /// like http://125.39.29.120:8629/live/ningxia
               ) {
                DOWNLOAD_API_DEBUG("this url is a  a ts/flv/mp4/live url ,return the speed  ! \n");
            } else {
                DOWNLOAD_API_ERROR("this url is a not a ts/flv/mp4/live url ,so skip it\n");
                result.speed = -1;
                result.errcode = ERR_NO_NORMAL_LIVE_URL;
                return result;
            }
        }
    }

    DOWNLOAD_API_DEBUG("[%s] speed[%d] result.downloadMS [%d]..\n", __func__, result.speed, result.downloadMS);
    DOWNLOAD_API_DEBUG("[%s] end end...\n", __func__);

    return result;


}

DownloadResult Nw_DownloadURL_TestSpeed(const char * url,
                             unsigned int timeoutMS, TestSpeedCtrl * para)
{

    int connecttimes = 0;
    DownloadResult result;

    do {
        result = downloadurl_testspeed(url, timeoutMS, para);
        connecttimes ++;
        if(result.errcode == ERR_ABORT_BY_USER)
        {
            return result;
        }

        if(result.speed < 0 && result.downloadMS <= 0) {
            DOWNLOAD_API_DEBUG("[%s] Reconnect ... ...\n", __func__);
            mtos_task_sleep(3000);
            continue;
        }
        else
        {
            return result;
        }
    } while(connecttimes < 3);

    return result;
}
#endif

#ifdef DOWNLOAD_HTTP_NEW
#define DEFAULT_TIME_OUT   30
static int getExtraHeaders(char *pHeader, unsigned int header_buffer_size, HTTP_spec_info_t *p_http_spec_info)
{
    unsigned int used_len = 0;
    memset(pHeader,0,header_buffer_size);
    if(p_http_spec_info != NULL)
    {
        if(p_http_spec_info->cookie_len >0)
        {
            if((strlen(p_http_spec_info->p_cookie)+strlen("Cookie: %s\r\n")+strlen(pHeader)) > header_buffer_size)
                return -1;

            snprintf (pHeader+used_len, header_buffer_size-used_len, "Cookie: %s\r\n", p_http_spec_info->p_cookie);
            used_len = strlen(pHeader);
        }

        if(p_http_spec_info->content_type_len)
        {
            if((strlen(p_http_spec_info->p_content_type)+strlen("Content-Type: %s\r\n")+strlen(pHeader)) > header_buffer_size)
                return -1;

            snprintf (pHeader+used_len, header_buffer_size-used_len, "Content-Type: %s\r\n", p_http_spec_info->p_content_type);
            used_len = strlen(pHeader);
        }
        if(p_http_spec_info->x_request_len)
        {
            if((strlen(p_http_spec_info->p_x_request)+strlen("X-Requested-With: %s\r\n")+strlen(pHeader)) > header_buffer_size)
                return -1;

            snprintf (pHeader+used_len, header_buffer_size-used_len, "X-Requested-With: %s\r\n", p_http_spec_info->p_x_request);
            used_len = strlen(pHeader);
        }

        if(p_http_spec_info->user_agent_len != 0)
        {
            if((strlen("User-Agent: %s\r\n")+8+strlen(pHeader)) > header_buffer_size)
                return -1;

            snprintf (pHeader+used_len, header_buffer_size-used_len, "User-Agent: %s\r\n", p_http_spec_info->p_user_agent);
            used_len = strlen(pHeader);
        }
    }

    return 0;
}
static int getContentType(HTTP_header_t *http_hdr, http_download_mini* dl)
{
    if(!dl || !http_hdr)
        return -1;

    const char *prefix = "Content-Type: ";
    const char *pContentype = dl->get_ContentType_from_response();
    int payload_len = 0;
    if(!pContentype){
        DOWNLOAD_API_ERROR("[%s] can not get pContentype from download\n", __func__);
    }else
        payload_len = strlen(pContentype);

    int contentypeLen = strlen(prefix)+payload_len;

    HTTP_field_t *new_field = (HTTP_field_t*)malloc(sizeof(HTTP_field_t));
    if(!new_field){
        DOWNLOAD_API_ERROR("[%s] not enough memory to malloc HTTP_field_t\n", __func__);
        return -2;
    }

    new_field->next = NULL;
    new_field->field_name = (char *)malloc(contentypeLen+2);
    if( new_field->field_name==NULL )
    {
        free(new_field);
        return -3;
    }

    memset(new_field->field_name,0,contentypeLen+2);
    strcpy(new_field->field_name, prefix);
    if(payload_len)
        strcat(new_field->field_name,pContentype);

    if(http_hdr->last_field==NULL )
    {
        http_hdr->first_field = new_field;
    }
    else
    {
        http_hdr->last_field->next = new_field;
    }
    http_hdr->last_field = new_field;
    http_hdr->field_nb++;

    return 0;
}
int chunkhttp_download_common_start(char *p_url, HTTP_header_t **p_outbuf,  HTTP_spec_info_t *p_http_spec_info)
{
    HTTP_header_t *p1 = (HTTP_header_t*)malloc(sizeof(HTTP_header_t));

    if(!p1){
        DOWNLOAD_API_ERROR("[%s] not enough memory to malloc HTTP_header_t\n", __func__);
        return -7;
    }

    memset(p1,0,sizeof(HTTP_header_t));

     if (p_url == NULL) {
        DOWNLOAD_API_ERROR("[%s] url is NULL, please check it\n", __func__);
        free(p1);
        return MTHTTP_UNKNOWN_ERROR;
    }

    char tmpurl[MAX_TMP_URL_LEN];
    memset(tmpurl, 0, MAX_TMP_URL_LEN);
    sprintf(tmpurl, "%s", p_url);

    http_download_mini* dl = new http_download_mini();
    if(!dl){
        DOWNLOAD_API_ERROR("[%s] malloc download instance failed\n", __func__);
        free(p1);
        return MTHTTP_UNKNOWN_ERROR;
    }

    MT_BOOL isGetOrPost = FALSE;
    unsigned int extraheader_len = 0;
    unsigned int timeout = DEFAULT_TIME_OUT;//sec
    char *pPost = NULL;
    unsigned int post_len = 0;
    if(p_http_spec_info){
        isGetOrPost = (p_http_spec_info->p_post_value != NULL);
        extraheader_len = p_http_spec_info->content_type_len
            + p_http_spec_info->cookie_len
            + p_http_spec_info->x_request_len
            + p_http_spec_info->user_agent_len
            + 256;
        timeout = p_http_spec_info->timeout;

        if(isGetOrPost){
            pPost = p_http_spec_info->p_post_value;
            post_len = p_http_spec_info->post_value_len;
        }
    }

    char *extraheader = NULL;

    if(extraheader_len > 256){
        extraheader = (char*)malloc(extraheader_len);
        if(!extraheader){
            DOWNLOAD_API_ERROR("[%s] not enough memory to malloc extraheader\n", __func__);
            free(p1);
            delete dl;
            return -7;
        }
        memset(extraheader,0,extraheader_len);
        getExtraHeaders(extraheader,extraheader_len,p_http_spec_info);
    }


    int content_len  = dl->http_connect(tmpurl, timeout*1000, isGetOrPost, extraheader, pPost, post_len);
    if(content_len < 0){
        DOWNLOAD_API_ERROR("[%s] connect error\n", __func__);
        delete dl;
        free(p1);
        if(extraheader)
            free(extraheader);
        return MTHTTP_UNKNOWN_ERROR;
    }else if(content_len == 0){
        DOWNLOAD_API_ERROR("[%s] can not get Content-length after connect\n", __func__);
    }

    if(extraheader)
        free(extraheader);

    p1->content_length = content_len;

    getContentType(p1,dl);

    *p_outbuf = p1;
    return ((int)dl&0x7fffffff);//because up-layer will use this return value to check >=0 or <0 ,so address need a little change
}
int chunkhttp_download_start(char *p_url, HTTP_header_t **p_outbuf)
{
    return chunkhttp_download_common_start(p_url,p_outbuf,NULL);
}
int  chunkhttp_recv(int fd, char *response, unsigned int size, unsigned int isChunked)
{
    if(fd == 0)
        return -1;

    http_download_mini* dl = (http_download_mini*)(fd|0x80000000);///refer to chunkhttp_download_common_start return value
    int written_len = MTHTTP_UNKNOWN_ERROR;

    int status = dl->http_receive((unsigned char*)response, size, &written_len);
    if(status == -2)
        written_len = MTHTTP_UNKNOWN_ERROR;

    return written_len;
}
void chunkhttp_close(int fd)
{
    if(fd == 0)
        return;

    http_download_mini* dl = (http_download_mini*)(fd|0x80000000);///refer to chunkhttp_download_common_start return value

    dl->http_close();
    delete dl;

    return;
}
int mthttp_download_common_start(char *p_url, HTTP_header_t **p_outbuf, HTTP_spec_info_t *p_http_spec_info)
{
#define HTTP_BUF_INCREASE_SIZE (1024*128)
    int fd = chunkhttp_download_common_start(p_url,p_outbuf,p_http_spec_info);
    if(fd <= 0)
        return fd;

    HTTP_header_t *p1 = *p_outbuf;
    if(!p1)
        return MTHTTP_UNKNOWN_ERROR;

    char *buf_ptr = NULL;
    int buf_size;
    int buf_used = 0;
    buf_ptr = (char*)malloc(HTTP_BUF_INCREASE_SIZE);
    if(!buf_ptr)
        return -1;
    buf_size = HTTP_BUF_INCREASE_SIZE;

    unsigned int expect_size = 0;
    int ret;
    expect_size = p1->content_length;
    if(!expect_size)
        expect_size = 0xffffffff;
    while(buf_used < expect_size)
    {
        ret = chunkhttp_recv(fd,buf_ptr+buf_used,buf_size-buf_used,0);
        if(ret <= 0){
            break;
        }
        else{
            buf_used += ret;
            if(buf_used >= buf_size-16){
                char *tmp = (char*)malloc(HTTP_BUF_INCREASE_SIZE+buf_size);
                if(!tmp)
                    break;

                memcpy(tmp,buf_ptr,buf_used);
                free(buf_ptr);

                buf_ptr = tmp;
                buf_size += HTTP_BUF_INCREASE_SIZE;
            }
        }
    }

    p1->buffer = buf_ptr;
    p1->buffer_size = buf_size;
    p1->body = p1->buffer;
    p1->body_size = buf_used;

    return 0;
}
#endif

void* Nw_Http_Download_Start(const char * url, MT_BOOL isGetOrPost, const char * postbuffer, unsigned int bufferLen,
    unsigned int timeoutSec, HttpDownloadHeader *header, char *extraheader)
{
    if (url == NULL) {
        DOWNLOAD_API_ERROR("[%s] url is NULL, please check it\n", __func__);
        return NULL;
    }
    DOWNLOAD_API_DEBUG("[%s] url is %s, time = %d \n", __func__,url,timeoutSec);
    if (header == NULL) {
        DOWNLOAD_API_ERROR("[%s] header response buffer is NULL, please check it\n", __func__);
        return NULL;
    }

    char tmpurl[MAX_TMP_URL_LEN];
    memset(tmpurl, 0, MAX_TMP_URL_LEN);
    sprintf(tmpurl, "%s", url);

    http_download_mini* dl = new http_download_mini();
    if(!dl){
        DOWNLOAD_API_ERROR("[%s] malloc download instance failed\n", __func__);
        return NULL;
    }


    char responsekey0[32] = "Content-Encoding";
    char responsevalue0[32];
    memset(responsevalue0, 0, sizeof(responsevalue0));
    char responsekey1[32] = "content-encoding";
    char responsevalue1[32];
    memset(responsevalue1, 0, sizeof(responsevalue1));
    char responsekey2[32] = "Accept-Ranges";
    char responsevalue2[32];
    char responsekey3[32] = "Content-Range";
    char responsevalue3[32];
    memset(responsevalue2, 0, sizeof(responsevalue2));
    memset(responsevalue3, 0, sizeof(responsevalue3));

    HTTP_rsp_header_t * rsp_header = (HTTP_rsp_header_t *)malloc(sizeof(HTTP_rsp_header_t));
    memset(rsp_header, 0, sizeof(HTTP_rsp_header_t));

    rsp_header->key[0] = responsekey0;
    rsp_header->value_buf[0] = responsevalue0;
    rsp_header->value_buflen[0] = 32;

    rsp_header->key[1] = responsekey1;
    rsp_header->value_buf[1] = responsevalue1;
    rsp_header->value_buflen[1] = 32;

    rsp_header->key[2] = responsekey2;
    rsp_header->value_buf[2] = responsevalue2;
    rsp_header->value_buflen[2] = 32;

    rsp_header->key[3] = responsekey3;
    rsp_header->value_buf[3] = responsevalue3;
    rsp_header->value_buflen[3] = 32;

    dl->set_rsp_header(rsp_header);


    int content_len  = dl->http_connect(tmpurl, timeoutSec*1000, isGetOrPost, extraheader, postbuffer, bufferLen);
    if(content_len < 0){
        DOWNLOAD_API_ERROR("[%s] connect error\n", __func__);


        header->resp_code = dl->http_resp_code;
        delete dl;
        return NULL;
    }else if(content_len == 0){
        DOWNLOAD_API_ERROR("[%s] can not get Content-length after connect\n", __func__);
    }

   if((strcmp(responsevalue0, "gzip") == 0) ||(strcmp(responsevalue1, "gzip") == 0))
   {
   	header->gzip_File = 1 ;
   }
   else
   {
   	header->gzip_File = 0 ;
   }

   if(strlen(responsevalue2))
   {
        DOWNLOAD_API_DEBUG("...................Accept-Ranges: %s\n",responsevalue2);
   	header->accept_range = TRUE;
   }
   else if(strlen(responsevalue3))
   {
        DOWNLOAD_API_DEBUG("...................Content-Range: %s\n",responsevalue3);
        if(strstr(responsevalue3, "bytes"))
        {
            header->accept_range = TRUE;
        }
        else
            header->accept_range = FALSE;
   }
   else
   {
        DOWNLOAD_API_DEBUG("...................not Accept-Ranges\n");
        header->accept_range = FALSE;
   }

    header->Content_length = content_len;
    header->redirect_url = dl->get_redirect_url();
    header->resp_code = dl->http_resp_code;
    return dl;
}

HttpDownloadResult Nw_Http_Download_Recv(void *handle, unsigned char *response, unsigned int buffersize)
{
    HttpDownloadResult result;
    http_download_mini* dl = (http_download_mini*)handle;

    result.readLen = 0;

    result.status = dl->http_receive(response, buffersize, &(result.readLen));

    return result;
}

void Nw_Http_Download_Stop(void *handle)
{
    if(!handle)
        return;

    http_download_mini* dl = (http_download_mini*)handle;

    dl->http_close();
    delete dl;

    return;
}

void Nw_Http_Download_Add_Task_Prio(void *handle, int add_task_prio)
{

}
void Nw_Http_Download_Remove_Curr_Task_Prio()
{

}
/*
*
*
*
*
*
*
*
*
*
*
*/
int  Nw_DownloadURL_POST(const char * url,
                          const char * postbuffer,
                          unsigned int bufferLen,
                          const char * tempFile,
                          const char * extraHeaders,
                          const char * checkHeader,
                          const char * serviceProvider)
{
    if (checkHeader != NULL) {
        // TODO:
    }
    if (url == NULL) {
        DOWNLOAD_API_ERROR("[%s] url is NULL, please check it\n", __func__);
        return 0;
    }

    char tmpurl[MAX_TMP_URL_LEN];
    memset(tmpurl, 0, MAX_TMP_URL_LEN);
    sprintf(tmpurl, "%s", url);
    http_download_mini instance;
    http_download_mini * dl = &instance;
    unsigned char abort = 0;


    if (dl == NULL) {
        DOWNLOAD_API_ERROR("[%s] [ERROR] invalid dl (NULL) ...!!!\n", __func__);
        return 0;
    }

    HttpDownloadMiniSpeed loadresult = dl->download((char *)tmpurl, 15 * 1000, (char *)tempFile, &abort, true, extraHeaders, postbuffer, bufferLen,NULL);

    DOWNLOAD_API_DEBUG("[%s] download result state: [%d] (0:COMPLETE; 1:ERR; 2:TIMEOUT; 3:ABORT; 4:ABORT by user\n", __func__, loadresult.state);
    DOWNLOAD_API_DEBUG("[%s] download size: [%d]\n", __func__, loadresult.size);

    if(loadresult.state == HTTP_MINI_ABORT_BY_USER)
    {
       delete_file((char *)tempFile);
        return -1;
    }

    if(loadresult.state != HTTP_MINI_COMPLETE || loadresult.size< 0)
    {
    	 delete_file((char *)tempFile);
        return 0;
    }

    return 1;
}


/*
*
*
*
*/
#ifndef DISABLE_ZLIB
int  Nw_DownloadURL_POST_Gzip(const char * url,
                          const char * postbuffer,
                          unsigned int bufferLen,
                          const char * tempFile,
                          const char * extraHeaders,
                          const char * checkHeader,
                             const char * serviceProvider,
                             unsigned int timeoutSec)///seconds
{
    if (checkHeader != NULL) {
        // TODO:
    }
    if (url == NULL) {
        DOWNLOAD_API_ERROR("[%s] url is NULL, please check it\n", __func__);
        return 0;
    }

    char extraHeaders_t[512] = {0};
    memset(extraHeaders_t, 0, 512);
    DOWNLOAD_API_DEBUG("[%s] extraHeaders [%s]...\n", __func__, extraHeaders);
    if (extraHeaders && strlen(extraHeaders) > 0)
    {
	    DOWNLOAD_API_DEBUG("[%s] extraHeaders !=NULL...\n", __func__);
        strcpy(extraHeaders_t, extraHeaders);

        if(extraHeaders[strlen(extraHeaders) - 2] == '\r' && extraHeaders[strlen(extraHeaders) - 1] == '\n') {
            strcat(extraHeaders_t, "Accept-Encoding: gzip\r\n");
        } else {
            strcat(extraHeaders_t, "\r\nAccept-Encoding: gzip\r\n");
        }
    } else {
        strcpy(extraHeaders_t, "Accept-Encoding: gzip\r\n");
    }
    DOWNLOAD_API_DEBUG("[%s] extraHeaders_t [%s]...\n", __func__, extraHeaders_t);

    char temp_gzip_file[64] = {0};
    //mtos_task_info_t info = {0};
    //mtos_task_get_info(&info);
    sprintf((char*)temp_gzip_file,"%s_zlibfile",tempFile);
    DOWNLOAD_API_DEBUG("[%s] temp_gzip_file =%s...\n", __func__, temp_gzip_file);

    char responsekey0[32] = "Content-Encoding";
    char responsevalue0[8];
    memset(responsevalue0, 0, sizeof(responsevalue0));
    char responsekey1[32] = "content-encoding";
    char responsevalue1[8];
    memset(responsevalue1, 0, sizeof(responsevalue1));

    HTTP_rsp_header_t * rsp_header = (HTTP_rsp_header_t *)malloc(sizeof(HTTP_rsp_header_t));
    memset(rsp_header, 0, sizeof(HTTP_rsp_header_t));

    rsp_header->key[0] = responsekey0;
    rsp_header->value_buf[0] = responsevalue0;
    rsp_header->value_buflen[0] = 8;

    rsp_header->key[1] = responsekey1;
    rsp_header->value_buf[1] = responsevalue1;
    rsp_header->value_buflen[1] = 8;

    int req_ret = Nw_DownloadURL_POST_ex2(url, postbuffer, bufferLen,temp_gzip_file,extraHeaders_t,rsp_header,timeoutSec);
    DOWNLOAD_API_DEBUG("[%s] req_ret [%d]...\n", __func__, req_ret);
    DOWNLOAD_API_DEBUG("[%s] responsevalue0=[%s], responsevalue1=[%s]...\n", __func__, responsevalue0, responsevalue1);
    if(req_ret == 1 && ((strcmp(responsevalue0, "gzip") == 0) ||(strcmp(responsevalue1, "gzip") == 0) || strstr(url, ".gz")))
    {
        int ret = decompress_to_file(temp_gzip_file, tempFile);
        //free(rsp_header); free by clean_arg dld
        rsp_header = NULL;
        delete_file(temp_gzip_file);

        return ret;
    }
    else
    {
        if(req_ret == 1)
        {
            rename_file(temp_gzip_file, tempFile);
        }
        //free(rsp_header); free by clean_arg dld
        rsp_header = NULL;
        delete_file(temp_gzip_file);

        return req_ret;
    }
}
#endif

int  Nw_DownloadURL_POST_ex(const char * url,
                             const char * postbuffer,
                             unsigned int bufferLen,
                             const char * tempFile,
                             const char * extraHeaders,
                             const char * checkHeader,
                             const char * serviceProvider,
                             unsigned int timeoutSec)///seconds
{
    if (checkHeader != NULL) {
        // TODO:
    }
    if (url == NULL) {
        DOWNLOAD_API_ERROR("[%s] url is NULL, please check it\n", __func__);
        return 0;
    }

    char tmpurl[MAX_TMP_URL_LEN];
    memset(tmpurl, 0, MAX_TMP_URL_LEN);
    sprintf(tmpurl, "%s", url);
    http_download_mini instance;
    http_download_mini * dl = &instance;
    unsigned char abort = 0;


    if (dl == NULL) {
        DOWNLOAD_API_ERROR("[%s] [ERROR] invalid dl (NULL) ...!!!\n", __func__);
        return 0;
    }

    HttpDownloadMiniSpeed loadresult = dl->download((char *)tmpurl, timeoutSec * 1000, (char *)tempFile, &abort, true, extraHeaders, postbuffer, bufferLen,NULL);

    DOWNLOAD_API_DEBUG("[%s] download result state: [%d] (0:COMPLETE; 1:ERR; 2:TIMEOUT; 3:ABORT; 4:ABORT by user\n", __func__, loadresult.state);
    DOWNLOAD_API_DEBUG("[%s] download size: [%d]\n", __func__, loadresult.size);

    if(loadresult.state == HTTP_MINI_ABORT_BY_USER)
    {
    	delete_file((char *)tempFile);
        return -1;
    }

    if(loadresult.state != HTTP_MINI_COMPLETE || loadresult.size< 0)
    {
    	 delete_file((char *)tempFile);
        return 0;
    }

    return 1;
}
int  Nw_DownloadURL_POST_ex2(const char * url,
                             const char * postbuffer,
                             unsigned int bufferLen,
                             const char * tempFile,
                             const char * extraHeaders,
                             HTTP_rsp_header_t *rsp_header,
                             unsigned int timeoutSec)///seconds
{
    if (url == NULL) {
        DOWNLOAD_API_ERROR("[%s] url is NULL, please check it\n", __func__);
        return 0;
    }

    char tmpurl[MAX_TMP_URL_LEN];
    memset(tmpurl, 0, MAX_TMP_URL_LEN);
    sprintf(tmpurl, "%s", url);
    http_download_mini instance;
    http_download_mini * dl = &instance;
    unsigned char abort = 0;


    if (dl == NULL) {
        DOWNLOAD_API_ERROR("[%s] [ERROR] invalid dl (NULL) ...!!!\n", __func__);
        return 0;
    }

    HttpDownloadMiniSpeed loadresult = dl->download((char *)tmpurl, timeoutSec * 1000, (char *)tempFile, &abort, true, extraHeaders, postbuffer, bufferLen,rsp_header);

    DOWNLOAD_API_DEBUG("[%s] download result state: [%d] (0:COMPLETE; 1:ERR; 2:TIMEOUT; 3:ABORT; 4:ABORT by user\n", __func__, loadresult.state);
    DOWNLOAD_API_DEBUG("[%s] download size: [%d]\n", __func__, loadresult.size);

    if(loadresult.state == HTTP_MINI_ABORT_BY_USER)
    {
        return -1;
    }

    if(loadresult.state != HTTP_MINI_COMPLETE || loadresult.size< 0)
    {
        return 0;
    }

    return 1;
}
void Abort_Download_Task(int task_prio, MT_BOOL abort_flag)
{
    abort_download_task(task_prio,abort_flag);
}
#define    RAM_FS_USE  1  ////yiyuan add this , use the ramfs for prj

#ifdef RAM_FS_USE
#define   YOUTUBE_WATCH_PAGE                   (char*)"/mnt/youtube_watch_page"
#define   YOUTUBE_HTML5PLAYER_JS               (char*)"/mnt/vevo_html5_js"
#define   YOUTUBE_LIVE_FILE                    (char*)"/mnt/youtube_live_page"
#else
#define   YOUTUBE_WATCH_PAGE                   (char*)"youtube_watch_page"
#define   YOUTUBE_HTML5PLAYER_JS               (char*)"vevo_html5_js"
#define   YOUTUBE_LIVE_FILE                    (char*)"/mnt/youtube_live_page"
#endif

int get_request_size(u8 size)
{
    int request_size = 0;
    if((size&0x1) == QEQUEST_VIDEO_LD)
    {
        request_size += QEQUEST_VIDEO_QCIF + QEQUEST_VIDEO_CIF;
    }
    if((size&0x2) == QEQUEST_VIDEO_SD)
    {
        request_size += QEQUEST_VIDEO_D1 + QEQUEST_VIDEO_720P;
    }
    if((size&0x4) == QEQUEST_VIDEO_HD)
    {
        if((size&0x2) == QEQUEST_VIDEO_SD)
            request_size += QEQUEST_VIDEO_1080P;
        else
            request_size += QEQUEST_VIDEO_720P + QEQUEST_VIDEO_1080P;
    }
    return request_size;
}

int video_size_to_pos(int size)
{
    int pos = 5;
    switch (size)
    {
		case QEQUEST_VIDEO_QCIF:
            pos = 0;
            break;
        case QEQUEST_VIDEO_CIF:
            pos = 1;
            break;
        case QEQUEST_VIDEO_D1:
            pos = 2;
            break;
        case QEQUEST_VIDEO_720P:
            pos = 3;
            break;
        case QEQUEST_VIDEO_1080P:
            pos = 4;
            break;
        default:
            break;
    }
    return pos;
}

int video_width_to_pos(int width)
{
    if(width < 177)
        return 0;
    else if(width < 361)
        return 1;
    else if(width < 721)
        return 2;
    else if(width < 1281)
        return 3;
    else
        return 4;
    return 1;
}


int json_url_decode(char *str, int len) {
	char *dest = str;
	char *data = str;

	while (len--) {
        if (*data == '\\' && len >= 1 ) {
			data += 1;
            continue;
		} else {
			*dest = *data;
		}

		data++;
		dest++;
	}

	*dest = '\0';
	return dest - str;
}


int  parse_url_list(request_url_t * p_request_url, u8 video_size)
{
    //qcif cif->ld, d1 720->sd, 720 1080->hd
    if((video_size&0x1) == QEQUEST_VIDEO_LD)
    {
        //preferentially choose cif
        if(strlen(p_request_url->playUrlArray[1]) > 0)
        {
            memset(p_request_url->playUrlArray[0], 0, DOWNLOAD_URL_MAX_LEN);
            memcpy(p_request_url->playUrlArray[0], p_request_url->playUrlArray[1], strlen(p_request_url->playUrlArray[1]));
        }
    }
    if((video_size&0x2) == QEQUEST_VIDEO_SD)
    {
        //preferentially choose d1
        if(strlen(p_request_url->playUrlArray[2]) > 0)
        {
            memset(p_request_url->playUrlArray[1], 0, DOWNLOAD_URL_MAX_LEN);
            memcpy(p_request_url->playUrlArray[1], p_request_url->playUrlArray[2], strlen(p_request_url->playUrlArray[2]));
        }
        else if(strlen(p_request_url->playUrlArray[1]) > 0)
        {
            //do nothing, select the cif.
        }
    }
    if((video_size&0x4) == QEQUEST_VIDEO_HD)
    {
        //preferentially choose 720P
        memset(p_request_url->playUrlArray[2], 0, DOWNLOAD_URL_MAX_LEN);
        if(strlen(p_request_url->playUrlArray[3]) > 0)
        {
            memcpy(p_request_url->playUrlArray[2], p_request_url->playUrlArray[3], strlen(p_request_url->playUrlArray[3]));
        }
        else if(strlen(p_request_url->playUrlArray[4]) > 0)
        {
            memcpy(p_request_url->playUrlArray[2], p_request_url->playUrlArray[4], strlen(p_request_url->playUrlArray[4]));
        }
    }
    memset(p_request_url->playUrlArray[3], 0, DOWNLOAD_URL_MAX_LEN);
    memset(p_request_url->playUrlArray[4], 0, DOWNLOAD_URL_MAX_LEN);

    char * decrypt_url = NULL;
    if(strlen(p_request_url->playUrlArray[1]) > 0)
    {
        decrypt_url = p_request_url->playUrlArray[1];
    }
    else if(strlen(p_request_url->playUrlArray[2]) > 0)
    {
        decrypt_url = p_request_url->playUrlArray[2];
    }
    else if(strlen(p_request_url->playUrlArray[0]) > 0)
    {
        decrypt_url = p_request_url->playUrlArray[0];
    }


    if(decrypt_url == NULL)
       return -4;

    return 0;
}

static int php_htoi(char *s) {
	int value;
	int c;
	c = ((unsigned char *)s)[0];

	if (isupper(c))
		c = tolower(c);

	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;
	c = ((unsigned char *)s)[1];

	if (isupper(c))
		c = tolower(c);

	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;
	return (value);
}


int php_url_decode(char *str, int len) {
	char *dest = str;
	char *data = str;

	while (len--) {
		if (*data == '+') {
			*dest = ' ';

		} else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) {
			*dest = (char) php_htoi(data + 1);
			data += 2;
			len -= 2;

		} else {
			*dest = *data;
		}

		data++;
		dest++;
	}

	*dest = '\0';
	return dest - str;
}

static void parse_s_in_start(parse_youtube_vedio_info_t * p_video_info, char * keyword, int index) {
	char * p_component_start = strstr(p_video_info->buf[index], keyword);

	if (p_component_start) {
		DOWNLOAD_API_DEBUG("[%s] find %s lalalala  !\n",__func__,keyword);
		char * p_component_end = strstr(p_component_start + strlen(keyword), "\\u0026");
        if (p_component_end) {
            int cpy_cnt = p_component_end - (p_component_start + strlen(keyword));
            if(cpy_cnt < 70 || cpy_cnt >= SIG_STR_MAX_LEN)
                return;
            memset(p_video_info->sigArray[index], 0, SIG_STR_MAX_LEN);
			memcpy(p_video_info->sigArray[index], p_component_start + strlen(keyword), cpy_cnt);
        }
	}
}

#define SHA_DIGEST_LENGTH 20
static int get_decrypt_data(char * p_js_url, char * p_encrypt_sig)
{
    //first download watch page, then html5player js.
    int  par_ret = 0;
    int ret = 0, len = 0;
    int download_retry = 0;
    char tmp_buf[1280] = {0};
    char * p_tmp_js_buf = NULL;
    int i = 0;

    //unsigned char digest[SHA_DIGEST_LENGTH];
    //char string[] = "hello world";
    //SHA1((unsigned char*)&string, strlen(string), (unsigned char*)&digest);
    unsigned char tmp_hash[SHA_DIGEST_LENGTH] = {0};
    unsigned char tmp_input_buf[1280] = {0};
    snprintf((char*)tmp_input_buf, 1200, "%s%s%s", p_encrypt_sig, p_js_url,HASH_SALT_YTS);
#if (defined WOLFSSL_REPLACE || defined WOLFSSL_REPLACE400)
    SHA_CTX c;
    SHA1_Init(&c);
    SHA1_Update(&c, tmp_input_buf, strlen((char*)tmp_input_buf));
    SHA1_Final(tmp_hash, &c);
#else
    SHA1(tmp_input_buf, strlen((char*)tmp_input_buf), tmp_hash);
#endif
    char mdString[SHA_DIGEST_LENGTH*2+1]= {0};
    for (i = 0; i < SHA_DIGEST_LENGTH; i++)
        sprintf(&mdString[i*2], "%02x", (unsigned int)tmp_hash[i]);
    memset(tmp_buf, 0, 1280);
    if(strstr(youtubeServerUrl, "47.241.5.241"))
        snprintf(tmp_buf, 1200, "%syts?s=%s&js=%s&h=%s", youtubeServerUrl,p_encrypt_sig, p_js_url,mdString);
    else
        snprintf(tmp_buf, 1200, "%s?s=%s&js=%s", youtubeServerUrl,p_encrypt_sig, p_js_url);
    //DOWNLOAD_API_DEBUG("[%s] url %s\n ", __func__, tmp_buf);

    for(download_retry = 0; download_retry < MAX_POST_COUNT; download_retry++)
    {
        ret = Nw_DownloadURLTimeout(tmp_buf, YOUTUBE_HTML5PLAYER_JS, DEFAULT_DL_HTML_TIMEOUT,NULL,NULL,NULL,NULL,0);

        if (ret == 1 || ret == -1) {
            break;
        }
    }

    if(download_retry == 4 || ret == -1)
    {
        delete_file(YOUTUBE_HTML5PLAYER_JS);
        DOWNLOAD_API_ERROR("[%s][ERROR] do nothing, fail to download vevo_html5_js ...\n ", __func__);
        return -2;//request server timeout
    }

    p_tmp_js_buf = (char *)malloc(SIG_STR_MAX_LEN);
    memset(p_tmp_js_buf, 0, SIG_STR_MAX_LEN);
    FILE * fp = NULL;
    fp = fopen(YOUTUBE_HTML5PLAYER_JS, "rb");

    if (fp) {
        DOWNLOAD_API_LOG("[%s]  open vevo_html5_js ok !!!!\n", __func__);


        if (p_tmp_js_buf) {
            int rdNum = fread(p_tmp_js_buf, 1, SIG_STR_MAX_LEN-1, fp);

            if (rdNum > 0) {
                DOWNLOAD_API_LOG("[%s] read %d bytes from vevo_html5_js!!!\n", __func__, rdNum);
                len = rdNum;
            }

            fclose(fp);
            fp = NULL;
        }
    }

    memset(p_encrypt_sig, 0, SIG_STR_MAX_LEN);
    if(p_tmp_js_buf != NULL)
    {
      DOWNLOAD_API_LOG("[%s] len[%d] SIG_STR_MAX_LEN[%d]\n", __func__, len, SIG_STR_MAX_LEN);
      if(len < SIG_STR_MAX_LEN)
      {
        memcpy(p_encrypt_sig, p_tmp_js_buf, len);
      }
      free(p_tmp_js_buf);
    }

    return par_ret;
}

//<script src="/yts/jsbin/player-vflBjp0_H/en_US/base.js"  name="player/base" ></script>
static int rss_get_html5player_name(char *buf, char *html5player)
{
    //maybe no PLAYER_JS_URL!!!
    int  par_ret = -1;
    char *p_start = NULL;
    char *p_end = NULL;
    //YOUTUBEURL_PARSER_DEBUG("[%s] enter enter\n", __func__);

    if(buf == NULL)
    {
      DOWNLOAD_API_ERROR("[%s] input buf is NULL\n", __FUNCTION__);
      return par_ret;
    }

    p_start = strstr(buf, "player_ias.vflset/");

    if(p_start == NULL)
    {
      DOWNLOAD_API_ERROR("[%s] cannot find script start = 0x%lx\n", __FUNCTION__, (ulong)p_start);
      return par_ret;
    }

    p_start -= 25;

    p_start = strstr(buf, "/s/player");
    if(p_start == NULL)
    {
      DOWNLOAD_API_ERROR("[%s] cannot find script start = 0x%lx\n", __FUNCTION__, (ulong)p_start);
      return par_ret;
    }

    p_end = strstr(p_start, ".js");

    if(p_end == NULL)
    {
      DOWNLOAD_API_ERROR("[%s] cannot find script end = 0x%lx\n", __FUNCTION__, (ulong)p_end);
      return par_ret;
    }

    p_end = p_end + strlen(".js");

    if((ulong)p_end - (ulong)p_start > 127)
    {
      DOWNLOAD_API_ERROR("[%s] html tag too long start = 0x%lx, end = 0x%lx\n", __FUNCTION__, (ulong)p_start, (ulong)p_end);
      return par_ret;
    }

    memcpy(html5player, p_start, (p_end-p_start));

    par_ret = 0;

    DOWNLOAD_API_DEBUG("[%s] html5player[%s]\n", __func__, html5player);

    return par_ret;
}

static int is_right_itag(char *str)
{
    char *p_start = NULL, *p_end = NULL;
    p_start = strstr(str, "itag=");
    if(p_start)
    {
        //DOWNLOAD_API_DEBUG("itag [%.12s]\n", p_start);
        p_start += strlen("itag=");
        p_end = strstr(p_start, "\\");
        if(p_end == NULL)
            p_end = strstr(p_start, "&");

        int cpy_len = 100;
        if(p_end)
        {
            cpy_len = p_end - p_start;

        }
        else if(*(p_start+2) == '\0')
        {
            //itag=18 is end
            cpy_len = 2;
        }
        else if(*(p_start+2) == '"')
        {
            //itag=18" is end
            cpy_len = 2;
        }

        if(cpy_len <= 4)
        {
            char tmp[5] = {0};
            memcpy(tmp, p_start, cpy_len);
            int itag = atoi(tmp);
            if(itag == 22 || itag == 18 || itag == 82 || itag == 83
                || itag == 84 || itag == 85 || itag == 34 || itag == 35
                || itag == 36 || itag == 37 )
            {
                DOWNLOAD_API_DEBUG("zx itag: %d return 1\n", itag);
                return 1;
            }
        }

        //133 240p, 134 360p, 135 480p, 136 720p, 137 1080p, 140 audio
        if(strstr(str, "itag=136") || strstr(str, "itag=140") ||
            strstr(str, "itag=133") || strstr(str, "itag=134") ||
            strstr(str, "itag=135") || strstr(str, "itag=137") ||
            strstr(str, "itag=22") || strstr(str, "itag=18"))
        {
            DOWNLOAD_API_DEBUG("zx itag 2222 return 1\n");
            return 1;
        }
    }

    return 0;
}

static int is_right_itag2(char *str, int *itag_value)
{
    char *p_start = NULL, *p_end = NULL;
    p_start = strstr(str, "itag");
    if(p_start)
    {
        //DOWNLOAD_API_DEBUG("itag [%.12s]\n", p_start);
        //itag\":18,
        p_start += strlen("itag");
        p_start = strstr(p_start, ":");
        p_start += 1;
        p_end = strstr(p_start, ",");

        int cpy_len = 100;
        if(p_end)
        {
            cpy_len = p_end - p_start;
        }

        if(cpy_len <= 4)
        {
            char tmp[5] = {0};
            memcpy(tmp, p_start, cpy_len);
            int itag = atoi(tmp);
            //DOWNLOAD_API_DEBUG("tmp [%s], itag %d\n", tmp, itag);
            //133 240p, 134 360p, 135 480p, 136 720p, 137 1080p, 140 audio
            if(itag == 22 || itag == 18 || itag == 136 || itag == 140
                || itag == 133 || itag == 134 || itag == 135 || itag == 137 )
            {
                DOWNLOAD_API_DEBUG("zx itag: %d return 1\n", itag);
                *itag_value = itag;
                return 1;
            }
        }
    }

    return 0;
}

int  parse_youtube_live_list(const char * p_videoid, parse_youtube_vedio_info_t * p_video_info, char *p_indexname, request_url_t * p_request_url, u8 video_size)
{
	DOWNLOAD_API_DEBUG("[%s] start start\n", __func__);
	int buf_len = 0;
	char * indexbuf = NULL;
	char *   p_url_start = NULL;
	char  *  p_url_end = NULL;
	char * p_tmp=NULL;
	int i;
	char *liveurl = NULL;
	int index = 0;
	int download_retry = 0;
	int ret=0;
    FILE * fp = NULL;

    if( !p_indexname)
		return FALSE;
    DOWNLOAD_API_DEBUG("[%s] p_indexname %s\n", __func__, p_indexname);


    for(download_retry = 0; download_retry < MAX_POST_COUNT; download_retry++)
    {
        ret = Nw_DownloadURLTimeout(p_indexname, YOUTUBE_LIVE_FILE, DEFAULT_DL_HTML_TIMEOUT,NULL,NULL,NULL,NULL,0);

        if (ret == 1 || ret == -1)
        {
            break;
        }
    }

    if(download_retry == 4 || ret == -1)
    {
        DOWNLOAD_API_ERROR("[%s][ERROR] do nothing, fail to download youtube_watch_page ...\n ", __func__);
        return -1;
    }

    fp = fopen(YOUTUBE_LIVE_FILE, "rb");

    if (fp) {
        DOWNLOAD_API_LOG("[%s]  open %s ok !!!!\n", __func__, YOUTUBE_LIVE_FILE);
        fseek( fp, 0, SEEK_END );
	    buf_len = ftell( fp ) + 1;
        fseek( fp, 0, SEEK_SET );
        indexbuf = (char *)malloc(buf_len);

        if (indexbuf) {
            memset(indexbuf, 0, buf_len);
            int rdNum = fread(indexbuf, 1, buf_len, fp);

            if (rdNum > 0) {
                DOWNLOAD_API_LOG("[%s] read %d bytes from youtube_watch_page!!!\n", __func__, rdNum);
            }

            fclose(fp);
			fp = NULL;
        }
    }

	if(!indexbuf ||buf_len<=0)
	{
		DOWNLOAD_API_ERROR("[%s] Can not get the content to parse\n", __func__);
		return FALSE;
	}

	p_url_start=indexbuf;
	p_url_end=indexbuf+buf_len;

     DOWNLOAD_API_DEBUG("[%s] buf_len %d\n", __func__, buf_len);
	   int nLowRes = -1;
 	   int nHiRes  = -1;
  	   int nLowTag = -1;
  	   int nHiTag  = -1;
          int cpy_cnt=-1;
     //      p_video_info->index=0;

	while(p_url_start<p_url_end)
	{

		p_url_start=strstr(p_url_start ,"https:");
		if(!p_url_start)
			break;
		p_tmp=p_url_start;
		p_tmp=strstr(p_url_start ,"index.m3u8");
		if(!p_tmp)
			break;
		p_tmp+= strlen("index.m3u8");
        if(!p_tmp)
			break;
        liveurl = (char *)malloc(LIVE_INDEX_URL_LEN);
        if(liveurl)
        {
    		memset(liveurl,0, LIVE_INDEX_URL_LEN);
    		strncpy(liveurl,p_url_start,p_tmp-p_url_start);
    		p_url_start=p_tmp;
    		index = p_video_info->index;
			p_video_info->buf[index] = liveurl;//zx
			memset(p_video_info->playUrlArray[index], 0, DOWNLOAD_URL_MAX_LEN);
		    strcpy(p_video_info->playUrlArray[index], p_video_info->buf[index]);
            //DOWNLOAD_API_DEBUG("youtube live %d %s\n", index, p_video_info->playUrlArray[index]);
		    p_video_info->index++;
		}

		if(p_video_info->index >= MAX_FILM_LIST_LEN)
                break;

	}

	for (i = 0; i < p_video_info->index; i++) {
		char * p_component_start = strstr(p_video_info->playUrlArray[i], "itag/");
		if (p_component_start)
		{

			char * p_component_end=strstr(p_component_start+strlen("itag/"), "/");
			if(p_component_end)
				cpy_cnt = p_component_end - (p_component_start + strlen("itag/"));

			if(cpy_cnt<0)
			{
				DOWNLOAD_API_ERROR("[ERROR]  fail to parse the component !!!\n");
				break;
			}
			char tmp[ITAG_STR_MAX_LEN] = {0};
			memset(tmp, 0, ITAG_STR_MAX_LEN);
			memcpy(tmp, p_component_start + strlen("itag/"),cpy_cnt);
			p_video_info->itagArray[i] = atoi(tmp);
			//DOWNLOAD_API_DEBUG("[%s] 666666 tmp string %s  digit %d index %d\n", __func__,tmp,atoi(tmp),i);
			switch(p_video_info->itagArray[i])
        	{
			    case 93:  //640x360
		 	        nLowRes = i;
           	  	    nLowTag = p_video_info->itagArray[i];
			        break;

			    case 95: //1280x720
		  	        nHiRes = i;
              	    nHiTag = p_video_info->itagArray[i];
			        break;
	 		}

		}
	}

	if (nLowRes >= 0)
	{
 	 	memset(p_request_url->playUrlArray[0], 0, DOWNLOAD_URL_MAX_LEN);
	 	memcpy(p_request_url->playUrlArray[0], p_video_info->playUrlArray[nLowRes], strlen(p_video_info->playUrlArray[nLowRes]));
		DOWNLOAD_API_DEBUG("[%s] playUrlArray[0] %s\n", __func__,p_request_url->playUrlArray[0]);
	}

	 if(nHiRes >=0)
    {
 		memset(p_request_url->playUrlArray[1], 0, DOWNLOAD_URL_MAX_LEN);
		memcpy(p_request_url->playUrlArray[1], p_video_info->playUrlArray[nHiRes], strlen(p_video_info->playUrlArray[nHiRes]));
		DOWNLOAD_API_DEBUG("[%s] playUrlArray[1]  %s\n", __func__,p_request_url->playUrlArray[1]);
	}

	if( (nLowRes <0)&&(nHiRes <0))
	{

			DOWNLOAD_API_ERROR("[ERROR]  fail to parse the component !!!\n");
            DOWNLOAD_API_ERROR("[ERROR]  failed buf[%d] :%s !!!\n", index, p_video_info->buf[i]);
			return FALSE;
    }
    if(indexbuf)
    {
        free(indexbuf);
        indexbuf = NULL;
    }
    delete_file(YOUTUBE_LIVE_FILE);
    DOWNLOAD_API_DEBUG("[%s] end end,video_size%d\n", __func__,video_size);
 return 0;
}

MT_BOOL g_sd = FALSE;
MT_BOOL g_hd = FALSE;

extern "C" void Nw_youtube_get_sd_hd(MT_BOOL *p_sd, MT_BOOL *p_hd)
{
    if(NULL != p_sd)
    {
        *p_sd = g_sd;
    }

    if(NULL != p_hd)
    {
        *p_hd = g_hd;
    }
}

extern "C" char *comm_parse_get_inner_string(char *string, char *front, char *end, char **new_start);
#define YOUTUBE_FAKE_URL  "http://youtube.file.mpd"

int set_dash_youtube_url_memory(dash_youtube_playurl_info_t *ptr)
{
    if(ptr == NULL)
        return -1;
    g_dash_youtube = ptr;
    memset(g_dash_youtube, 0, sizeof(dash_youtube_playurl_info_t));
    return 0;
}

static int comm_add_amp_for_urls(char **url)
{
    char *tmp_url = *url;
    char *result = (char *)malloc(DOWNLOAD_URL_MAX_LEN);
    int i = 0, j = 0, urllen = strlen(tmp_url);

    //	add amp;
    memset(result, 0x00, DOWNLOAD_URL_MAX_LEN);

    for (i = 0; i < urllen;) {
    	if (tmp_url[i] == '&') {
    	    result[j++] = tmp_url[i];
            result[j++] = 'a';
            result[j++] = 'm';
            result[j++] = 'p';
            result[j++] = ';';
    	    i++;
    	} else {
    	    result[j] = tmp_url[i];
    	    i++;
    	    j++;
    	}
    }
    result[j] = 0;

    //mtos_printk("real config url is %s\n", result);

    memset(*url, 0x00, DOWNLOAD_URL_MAX_LEN);
    strcpy(*url, result);

    free(result);

    return 0;
}

static char * get_dash_lengthSeconds(char * urlenc_data)
{
    char *p_start = NULL;
    char *new_start = NULL;

    p_start = strstr(urlenc_data, "lengthSeconds");
    if(p_start)
    {
        p_start += strlen("lengthSeconds");
        char *p_duration = comm_parse_get_inner_string(p_start, ":\\\"", "\\", &new_start);
        if(p_duration)
        {
            return p_duration;
        }
    }
    return NULL;
}

static int write_dash_mpd(parse_youtube_vedio_info_t * p_video_info, char * urlenc_data, int audio_url, int video_url)
{
    char *p_duration = get_dash_lengthSeconds(urlenc_data);
    if(p_duration)
    {
        char *tmp_video = p_video_info->playUrlArray[video_url];
        char *tmp_audio = p_video_info->playUrlArray[audio_url];
        comm_add_amp_for_urls(&tmp_audio);
        comm_add_amp_for_urls(&tmp_video);
        char *pBuf = (char *)malloc(20480);//20k
        if(pBuf == NULL)
        {
            DOWNLOAD_API_ERROR("[%s][ERROR] fail to malloc 20480!!!\n", __func__);
            return -1;
        }
        memset(pBuf,0, 20480);
        sprintf(pBuf,
        	    "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        	    "<MPD mediaPresentationDuration=\"PT%sS\" profiles=\"urn:mpeg:dash:profile:isoff-on-demand:2011\" type=\"static\" xmlns=\"urn:mpeg:dash:schema:mpd:2011\">"
        	    "<Period>\n"
        	    "<AdaptationSet mimeType=\"audio/mp4\" segmentAlignment=\"true\" startWithSAP=\"1\">\n"
        	    "<Representation audioSamplingRate=\"44100\" bandwidth=\"141962\" codecs=\"mp4a.40.2\" id=\"audio-und\">\n"
        	    "<BaseURL>%s</BaseURL>\n"
        	    "</Representation>\n"
        	    "</AdaptationSet>\n"
        	    "<AdaptationSet mimeType=\"video/mp4\" segmentAlignment=\"true\" startWithSAP=\"1\">\n"
        	    "<Representation bandwidth=\"8000000\" codecs=\"avc1.42C01F\" height=\"1080\" width=\"1920\">\n"
        	    "<BaseURL>%s</BaseURL>\n"
        	    "</Representation>\n"
        	    "</AdaptationSet>\n"
        	    "</Period>\n"
        	    "</MPD>\n"
        	    , p_duration, p_video_info->playUrlArray[audio_url], p_video_info->playUrlArray[video_url]);

        //mtos_printk("zx: %s\n", p_video_info->playUrlArray[audio_url]);
        //mtos_printk("zx: %s\n", p_video_info->playUrlArray[video_url]);
        //RssUfsWriteBufferFile(YOUTUBE_WRITE_DASH_FLLE, pBuf, strlen(pBuf));
        free(p_duration);
        free(pBuf);
        return 0;
    }
    else
        DOWNLOAD_API_ERROR("[%s][ERROR] fail to find dash lengthSeconds !!!\n", __func__);

    return -1;
}

char *select_dash_youtube_url(int index)
{
    if(g_dash_youtube == NULL || index >= g_dash_youtube->total_urls)
    {
        DOWNLOAD_API_ERROR("[%s][ERROR] wrong index %d !!!\n", __func__, index);
        return NULL;
    }
    DOWNLOAD_API_ERROR("[%s]index %d , total_urls %d!!!\n", __func__, index, g_dash_youtube->total_urls);

    char *pBuf = (char *)malloc(20480);//20k
    if(pBuf == NULL)
    {
        DOWNLOAD_API_ERROR("[%s][ERROR] malloc 20480 byte failed!!!\n", __func__);
        return NULL;
    }
    memset(pBuf,0, 20480);
    sprintf(pBuf,
    	    "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
    	    "<MPD mediaPresentationDuration=\"PT%sS\" profiles=\"urn:mpeg:dash:profile:isoff-on-demand:2011\" type=\"static\" xmlns=\"urn:mpeg:dash:schema:mpd:2011\">"
    	    "<Period>\n"
    	    "<AdaptationSet mimeType=\"audio/mp4\" segmentAlignment=\"true\" startWithSAP=\"1\">\n"
    	    "<Representation audioSamplingRate=\"44100\" bandwidth=\"141962\" codecs=\"mp4a.40.2\" id=\"audio-und\">\n"
    	    "<BaseURL>%s</BaseURL>\n"
    	    "</Representation>\n"
    	    "</AdaptationSet>\n"
    	    "<AdaptationSet mimeType=\"video/mp4\" segmentAlignment=\"true\" startWithSAP=\"1\">\n"
    	    "<Representation bandwidth=\"8000000\" codecs=\"avc1.42C01F\" height=\"1080\" width=\"1920\">\n"
    	    "<BaseURL>%s</BaseURL>\n"
    	    "</Representation>\n"
    	    "</AdaptationSet>\n"
    	    "</Period>\n"
    	    "</MPD>\n"
    	    , g_dash_youtube->duration, g_dash_youtube->audio_playUrl, g_dash_youtube->playUrlArray[index]);

    //RssUfsWriteBufferFile(YOUTUBE_WRITE_DASH_FLLE, pBuf, strlen(pBuf));
    free(pBuf);

    return YOUTUBE_FAKE_URL;

}

int set_youtube_only_dash_url(int value)
{
    g_only_use_youtube_dash = value;
}
extern "C" int youtube_get_server_ret_val(void)
{
	int val = 0;
	val = server_ret_val;

	server_ret_val = -2;

	return val;
}

static void decrypt_youtube_vevo(parse_youtube_vedio_info_t * p_video_info, char * urlenc_data, int nRes, int sig_type)
{
    char html5player[128];
    memset(html5player, 0 , 128);
    int par_ret = rss_get_html5player_name(urlenc_data, html5player);
    if(par_ret == 0)
    {
        if(sig_type == 0)
            strcat(p_video_info->playUrlArray[nRes], "&signature=");
        else if(sig_type == 1)
            strcat(p_video_info->playUrlArray[nRes], "&sig=");

        DOWNLOAD_API_DEBUG("Need decrypt_sig[%s] js[%s]!!\n",p_video_info->sigArray[nRes], html5player);
        par_ret = get_decrypt_data(html5player, p_video_info->sigArray[nRes]);
        //DOWNLOAD_API_DEBUG("Decrypt_sig[%s]ret:%d!!\n",p_video_info->sigArray[nRes], par_ret);

		DOWNLOAD_API_DEBUG("Decrypt_sig[%s]ret:%d!!\n",p_video_info->sigArray[nRes], par_ret);

		if(-2 == par_ret) //request server timeout
		{
			server_ret_val = -1;
		}
		else
		{
        	server_ret_val = 0;
		}

		strcat(p_video_info->playUrlArray[nRes], p_video_info->sigArray[nRes]);
    }
}

#ifdef ENABLE_DASH
static int save_dash_playurls(parse_youtube_vedio_info_t * p_video_info, char * urlenc_data, int sig_type)
{
    int i = 0, j = 0;
    //133 240p, 134 360p, 135 480p, 136 720p, 137 1080p, 140 audio
    char *p_duration = get_dash_lengthSeconds(urlenc_data);
    if(p_duration)
    {
        if(strlen(p_duration) >= 16)
        {
            DOWNLOAD_API_ERROR("[%s][ERROR] fail to find dash lengthSeconds !!!\n", __func__);
            return -1;
        }
        strcpy(g_dash_youtube->duration, p_duration);
        free(p_duration);
    }
    else
    {
        DOWNLOAD_API_ERROR("[%s][ERROR] fail to find dash lengthSeconds !!!\n", __func__);
        return -1;
    }

    int dash_itag[DASH_YOUTUBE_VIDEO_MAX] = {133,134,135,136,137};
    int video_num = 0;
    for (i = 0; i < p_video_info->index; i++)
    {
        if(p_video_info->itagArray[i] == 140)
        {
            if(strlen(p_video_info->sigArray[i]) > 79)
            {
                decrypt_youtube_vevo(p_video_info, urlenc_data, i, sig_type);
            }
            char *tmp_audio = p_video_info->playUrlArray[i];
            comm_add_amp_for_urls(&tmp_audio);
            memcpy(g_dash_youtube->audio_playUrl, tmp_audio, strlen(tmp_audio));
            break;
        }
    }

    for(i = 0; i < DASH_YOUTUBE_VIDEO_MAX; i++)
    {
        for (j = 0; j < p_video_info->index; j++)
        {
            g_dash_youtube->resolution[g_dash_youtube->total_urls] = DashYoutubeVideo(i);
            if(p_video_info->itagArray[j] == dash_itag[i])
            {
                if(strlen(p_video_info->sigArray[j]) > 79)
                {
                    decrypt_youtube_vevo(p_video_info, urlenc_data, j, sig_type);
                }
                char *tmp_video = p_video_info->playUrlArray[j];
                comm_add_amp_for_urls(&tmp_video);
                memcpy(g_dash_youtube->playUrlArray[g_dash_youtube->total_urls], tmp_video, strlen(tmp_video));
                g_dash_youtube->total_urls++;
                break;
            }
        }
    }

}
#endif

/*
    string = \"\/\/i1.ytimg.com\/vi\/9HK-iODwQzk\/mqdefault.jpg\"

    it will return  //i1.yting.com/vi/9HK-iODwQzk/mqdefault.jpg.
*/
char *downlaod_parse_html_to_get_inner_string(char *string, char *front, char *end, char **new_start)
{
    char *start, *tail, *tmp;
    int front_len, end_len, len;
    int i = 0;
    int j = 0;
    char *url_tmp = NULL;

    if (string == NULL) {
	return NULL;
    }

    front_len = strlen(front);
    end_len = strlen(end);
    start = strstr(string, front);

    if (start == NULL) {
	return NULL;
    }

    start += front_len;
    tail = strstr(start, end);

    if (tail == NULL) {
	return NULL;
    }

    len = tail - start;
    url_tmp = (char *)malloc(len + 1);
    memset(url_tmp, 0x00, len + 1);
    tmp = (char *)malloc(len + 1);
    memset(tmp, 0x00, len + 1);
    memcpy(url_tmp, start, len);

    for (i = 0; i < len; i++) {
	if (url_tmp[i] != '\\') {
	    tmp[j] = url_tmp[i];
	    j++;
	}
    }

    *new_start = tail + end_len;
    free(url_tmp);
    return tmp;
}

static int transfer_0026_from_urls(char **url)
{
    char *tmp_url = *url;
    int urllen = strlen(tmp_url);
    char *result = (char *)malloc(urllen+1);
    int i = 0, j = 0;

    //	 \\u0026 to &;
    memset(result, 0x00, urllen + 1);
    //DOWNLOAD_API_DEBUG("tmp_url %s\n", tmp_url);

    for (i = 0; i < urllen;) {
	if (tmp_url[i + 0] == '\\' && tmp_url[i + 1] == '\\' && tmp_url[i + 2] == 'u' && tmp_url[i + 3] == '0'
        && tmp_url[i + 4] == '0' && tmp_url[i + 5] == '2' && tmp_url[i + 6] == '6') {
        result[j] = '&';
        j++;
	    i += 7;
    } else if (tmp_url[i + 0] == '\\' && tmp_url[i + 1] == 'u' && tmp_url[i + 2] == '0'
        && tmp_url[i + 3] == '0' && tmp_url[i + 4] == '2' && tmp_url[i + 5] == '6') {
        result[j] = '&';
        j++;
	    i += 6;
	} else {
	    result[j] = tmp_url[i];
	    i++;
	    j++;
	}
    }
    result[j] = 0;

    //DOWNLOAD_API_DEBUG("real config url is %s\n", result);

    memset(*url, 0x00, urllen + 1);
    memcpy(*url, result, strlen(result));

    free(result);

    return 0;
}


static int youtube_extract_buf(char * urlenc_data, parse_youtube_vedio_info_t * p_video_info)
{
    char *   p_stream_map = NULL;
	char *   p_url_start = NULL;
	char  *  p_url_end = NULL;
    int      data_len  = 0;
	char *   p_stream_map_tail = NULL;
    char *   tmp_buf = NULL;
    int  par_ret1 = 0;
    int  par_ret2 = 0;
    int  ret = 0;
    {
		DOWNLOAD_API_DEBUG("[%s] try %s!!!\n", __func__, youtube_new_key_str1);//adaptive_fmts
        if (p_stream_map = strstr(urlenc_data, youtube_new_key_str1) )
    	{
    		//DOWNLOAD_API_DEBUG("[%s]>>>> find 'stream_map='  lalala !!!<<<<\n", __func__);
    		p_url_start = p_stream_map + strlen(youtube_new_key_str1);
    		p_stream_map_tail = strstr(p_url_start, "}]},");
    		DOWNLOAD_API_DEBUG("[%s] len of stream_map_list  is:%d\n", __func__, p_stream_map_tail - p_url_start);
    		char parse_ok = 0;
    		int index = 0;
    		// the comma ',' means new url start
    		do {
    			p_url_end = strstr(p_url_start, "},{");
                if(p_url_end == NULL)
                {
                    DOWNLOAD_API_DEBUG("%s %d return\n", __func__, __LINE__);
                    break;
                }
                if(p_url_end >= p_stream_map_tail)
                {
                    //mtos_printk("zx %d p_url_end > p_stream_map_tail\n", __LINE__);
                    p_url_end = strstr(p_url_start, "}]");
                    if(p_url_end == NULL)
                    {
                        //mtos_printk("zx %d p_url_end > p_stream_map_tail and no \" \n", __LINE__);
                        break;
                    }
                }

    			data_len = p_url_end - p_url_start;
    			index = p_video_info->index;
    			tmp_buf = (char *)malloc(data_len + 1);

    			if (tmp_buf) {
    				memset(tmp_buf, 0, data_len + 1);
    				memcpy(tmp_buf, p_url_start, data_len);
                    //DOWNLOAD_API_DEBUG("p_video_info->buf[%d]:%s \n", index, p_video_info->buf[index]);
    				p_url_start = p_url_end + 1;
                    int tmp_itag = 0;
    				if(is_right_itag2(tmp_buf, &tmp_itag))
                    {
                        p_video_info->buf[index] = tmp_buf;
                        p_video_info->bufTypeArray[index] = 1;
                        p_video_info->itagArray[index] = tmp_itag;
        				p_video_info->index++;
                    }
                    else
                        free(tmp_buf);
    			}

                if(p_video_info->index >= MAX_FILM_LIST_LEN)
                    break;
    		} while (p_url_start < p_stream_map_tail && parse_ok == 0);
        }
        else
        {
            par_ret1 = 1;
            DOWNLOAD_API_ERROR("[%s][ERROR] fail to find %s keyword!!!\n", __func__, youtube_new_key_str1);
        }
	}

    {
		DOWNLOAD_API_DEBUG("[%s]try %s !!!\n", __func__, youtube_new_key_str2);//url_encoded_fmt_stream_map
        if (p_stream_map = strstr(urlenc_data, youtube_new_key_str2) )
    	{
    		//DOWNLOAD_API_DEBUG("[%s]>>>> find 'stream_map='  lalala !!!<<<<\n", __func__);
    		//streamingData\":{\"expiresInSeconds\":\"21540\",\"formats\":[{
    		p_stream_map += strlen(youtube_new_key_str2);
    		p_url_start = strstr(p_stream_map, "formats");
            p_url_start += strlen("formats");
    		p_stream_map_tail = strstr(p_url_start, "}],");
    		//DOWNLOAD_API_DEBUG("[%s] len of stream_map_list  is:%d\n", __func__, p_stream_map_tail - p_url_start);
    		char parse_ok = 0;
    		int index = 0;
    		// the comma ',' means new url start
    		do {
    			p_url_end = strstr(p_url_start, "},{");
                if(p_url_end == NULL)
                {
                    DOWNLOAD_API_DEBUG("%s %d return\n", __func__, __LINE__);
                    break;
                }
                if(p_url_end >= p_stream_map_tail)
                {
                    p_url_end = strstr(p_url_start, "}]");
                    if(p_url_end == NULL)
                    {
                        break;
                    }
                }

    			data_len = p_url_end - p_url_start;
    			index = p_video_info->index;
    			tmp_buf = (char *)malloc(data_len + 1);

    			if (tmp_buf) {
    				memset(tmp_buf, 0, data_len + 1);
    				memcpy(tmp_buf, p_url_start, data_len);
                    //DOWNLOAD_API_DEBUG("p_video_info->buf[%d]:%s \n", index, p_video_info->buf[index]);
    				p_url_start = p_url_end + 1;
                    int tmp_itag = 0;
    				if(is_right_itag2(tmp_buf, &tmp_itag))
                    {
                        p_video_info->buf[index] = tmp_buf;
                        p_video_info->bufTypeArray[index] = 1;
                        p_video_info->itagArray[index] = tmp_itag;
        				p_video_info->index++;
                    }
                    else
                        free(tmp_buf);
    			}

                if(p_video_info->index >= MAX_FILM_LIST_LEN)
                    break;
    		} while (p_url_start < p_stream_map_tail && parse_ok == 0);
        }
        else
        {
            par_ret2 = 1;
            DOWNLOAD_API_ERROR("[%s][ERROR] fail to find %s keyword!!!\n", __func__, youtube_new_key_str2);//adaptive_fmts
        }
	}

    if(par_ret1 && par_ret2)
    {
        DOWNLOAD_API_ERROR("[%s][ERROR] youtube url info extract nothing!!! Web changed!!!!\n", __func__);
        ret = -5;
    }
    return ret;
}

int  youtube_parse_stream_map_list(const char * p_videoid, parse_youtube_vedio_info_t * p_video_info, char * urlenc_data, request_url_t * p_request_url, u8 size, youtube_search_key_t youtube_key) {
    DOWNLOAD_API_DEBUG("[%s] start start ...\n", __func__);
    int      i = 0 ;
    int      ret = 0;
    char  *  p_url_start = NULL;
    g_sd = FALSE;
    g_hd = FALSE;

    ret = youtube_extract_buf(urlenc_data, p_video_info);
    if(ret!= 0)
        return ret;

    int nLowRes = -1;
    int nHiRes  = -1;
    int nLowTag = -1;
    int nHiTag  = -1;
    int sig_type = -1;
    for (i = 0; i < p_video_info->index; i++) {
        int input_len = 0;
        int output_len = 0;

        if (p_video_info->buf[i]) {
            sig_type = -1;
            input_len = strlen(p_video_info->buf[i]);
            output_len = php_url_decode(p_video_info->buf[i], input_len);
            //DOWNLOAD_API_DEBUG("zx p_video_info->buf[%d]:%.900s", i, p_video_info->buf[i]);
            //DOWNLOAD_API_DEBUG("%s\n", (p_video_info->buf[i]+900));
            if(p_video_info->bufTypeArray[i] == 1)
            {
                //new json, add by zhouxiang
                char *tmp_str = NULL;
                int cipher_write_style = -1;
                p_url_start = strstr(p_video_info->buf[i], "cipher");
                if(p_url_start == NULL)
                {
                    DOWNLOAD_API_DEBUG("music find cipher fail, try signatureCipher\n");
                    p_url_start = strstr(p_video_info->buf[i], "signatureCipher");
                    if(p_url_start)
                       cipher_write_style = 1;
                }
                else
                    cipher_write_style = 0;

                if(p_url_start)//\\u0026s=
                {
                    char *new_start = NULL;
                    if(strstr(p_url_start, "url="))
                    {
                        if(strstr(p_url_start, "\\\""))
                            tmp_str = downlaod_parse_html_to_get_inner_string(p_url_start, "url=", "\\\"", &new_start);
                        else
                            tmp_str = downlaod_parse_html_to_get_inner_string(p_url_start, "url=", "\"", &new_start);
                        //DOWNLOAD_API_DEBUG("zx tmp_str:%s\n", tmp_str);
                        // \u0026sp=sig\u0026s=AALgxI2wwRgIhAPrPteiZL4hF_tOQU_FN9-U2n0KV2O7AxrJTAnZ19luYAiEA7J0KiUx966opyyjn3SXrRso1h-rM2SmrH6QwLwd4tjY%3DY%3D\"
                        char *p_sp_type = strstr(p_url_start, "sp=");
                        if(p_sp_type)
                        {
                            p_sp_type += strlen("sp=");
                            if(strncmp(p_sp_type, "signature", strlen("signature"))== 0 )
                                sig_type = 0;
                            else
                                sig_type = 1;
                            DOWNLOAD_API_DEBUG("zx p_sp_type %.150s, sig_type %d\n",
                                p_sp_type, sig_type);
                            char *tmp_str2 = downlaod_parse_html_to_get_inner_string(p_sp_type, "\\u0026s=", "\\", &new_start);
                            memset(p_video_info->sigArray[i], 0, SIG_STR_MAX_LEN);
                            if(tmp_str2)
                            {
        				        memcpy(p_video_info->sigArray[i], tmp_str2, strlen(tmp_str2));
                                free(tmp_str2);
                            }
                            else if(cipher_write_style == 1)
                            {
                                p_url_start = strstr(p_url_start, "s=");
                                DOWNLOAD_API_DEBUG("zx 222 p_url_start %.150s, sig_type %d\n",
                                    p_url_start, sig_type);
                                char *tmp_str2 = downlaod_parse_html_to_get_inner_string(p_url_start, "s=", "\\", &new_start);
                                if(tmp_str2)
                                {
            				        memcpy(p_video_info->sigArray[i], tmp_str2, strlen(tmp_str2));
                                    free(tmp_str2);
                                }
                            }
                            else//for cipher_write_style == 0
                            {
                                //s= in start!  cipher\":\"s=AALgxI2wwRgIhALJ0OLfIYcG1xl4At1kufXKVQu34J4RyzAT82ue3YkLwAiEA-Cgurw-xn4DEj5aIpr2mXn9nBlf8B0jrQwcDdEd0Pzk=k=\\u0026url=
                                p_url_start = strstr(p_video_info->buf[i], "cipher");
                                if(p_url_start)
                                {
                                    char *tmp_str2 = downlaod_parse_html_to_get_inner_string(p_url_start, "\"s=", "\\", &new_start);
                                    if(tmp_str2)
                                    {
                				        memcpy(p_video_info->sigArray[i], tmp_str2, strlen(tmp_str2));
                                        free(tmp_str2);
                                    }
                                    else
                                    {
                                        p_url_start = strstr(p_url_start, "u0026s=");
                                        DOWNLOAD_API_DEBUG("zx 333 p_url_start %.150s, sig_type %d\n",
                                            p_url_start, sig_type);
                                        if(p_url_start)
                                        {
                                            char *tmp_str2 = downlaod_parse_html_to_get_inner_string(p_url_start, "s=", "\\", &new_start);
                                            if(tmp_str2)
                                            {
                        				        memcpy(p_video_info->sigArray[i], tmp_str2, strlen(tmp_str2));
                                                free(tmp_str2);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        DOWNLOAD_API_ERROR("%s ERROR, youtube music vevo unsupport find url=\n", __func__);
                        DOWNLOAD_API_ERROR("zx p_video_info->buf[%d]:%.900s", i, p_video_info->buf[i]);
                        DOWNLOAD_API_ERROR("%s\n", (p_video_info->buf[i]+900));
                    }
                }
                else
                {
                    //normal url
                    char *new_start = NULL;
                    transfer_0026_from_urls(&(p_video_info->buf[i]));
                    if(strstr(p_video_info->buf[i], "url\":\""))
                    {
                        DOWNLOAD_API_DEBUG("url\":\" type111\n");
                        tmp_str = downlaod_parse_html_to_get_inner_string(p_video_info->buf[i], "url\":\"", "\"", &new_start);
                    }
                    else if(strstr(p_video_info->buf[i], "url\\"))
                    {
                        DOWNLOAD_API_DEBUG("url type222\n");
                        p_url_start = strstr(p_video_info->buf[i], "url");
                        p_url_start = strstr(p_url_start, ":");
                        tmp_str = downlaod_parse_html_to_get_inner_string(p_url_start, "\"", "\\\"", &new_start);
                    }
                    else
                    {
                        DOWNLOAD_API_ERROR("%s ERROR, unsupport youtube html!\n", __func__);
                        DOWNLOAD_API_ERROR("zx p_video_info->buf[%d]:%.900s", i, p_video_info->buf[i]);
                        DOWNLOAD_API_ERROR("%s\n", (p_video_info->buf[i]+900));
                    }
                }

                memset(p_video_info->playUrlArray[i], 0, DOWNLOAD_URL_MAX_LEN);
                if(tmp_str)
                {
    				memcpy(p_video_info->playUrlArray[i], tmp_str, strlen(tmp_str));
                    DOWNLOAD_API_DEBUG("zx itag %d, sig_type %d url:%s\n",
                        p_video_info->itagArray[i], sig_type, p_video_info->playUrlArray[i]);
                    free(tmp_str);
                }
            }
#if 0
            //discard by youtbue
            else
            {
                //old json, no change
    			parse_url_component(p_video_info, youtube_key.keyArray[4], i);//KW_URL
                if(strstr(p_video_info->playUrlArray[i], "signature=") == NULL)
                {
                    char *p_sp_type = strstr(p_video_info->buf[i], "sp=");
                    if(p_sp_type)
                    {
                        p_sp_type += strlen("sp=");
                        if(strncmp(p_sp_type, "signature", strlen("signature"))== 0 )
                            sig_type = 0;
                        else
                            sig_type = 1;
                        DOWNLOAD_API_DEBUG("zx youtube_key.keyArray[5]:%s, p_sp_type %.10s, sig_type %d\n",
                            youtube_key.keyArray[5], p_sp_type, sig_type);
        				parse_url_component(p_video_info, youtube_key.keyArray[5], i);//KW_SIG
        				if(strlen(p_video_info->sigArray[i]) == 0)
                            parse_s_in_start(p_video_info, "s=", i);
                        DOWNLOAD_API_DEBUG("p_video_info->sigArray[i]:%s\n", p_video_info->sigArray[i]);
                    }
                }
    			parse_url_component(p_video_info, youtube_key.keyArray[6], i);//KW_ITAG
    			//DOWNLOAD_API_DEBUG("playUrlArray[%d]:%s \n", i, p_video_info->playUrlArray[i]);
    			input_len = strlen(p_video_info->playUrlArray[i]);
    			output_len = php_url_decode(p_video_info->playUrlArray[i], input_len);
            }
#endif
			//collect h264 encode stream
            /*	{18, "MP4", "270p/360p", "H.264", "AAC"},  0
            	{22, "MP4", "720p", "H.264 High", "AAC"},  3
            	{34, "FLV", "360p", "H.264 Main", "AAC"},  1
            	{35, "FLV", "480p", "H.264 Main", "AAC"},  2
            	{37, "MP4", "1080P", "H.264 High", "AAC"}, 4
            	{38, "MP4", "3072P", "H.264 High", "AAC"}, 4
            	{82, "MP4", "360p", "H.264 3D", "AAC"},    1
            	{83, "MP4", "480p", "H.264 3D", "AAC"},    0
            	{84, "MP4", "720p", "H.264 3D", "AAC"},    3
            	{85, "MP4", "1080p", "H.264", "AAC"},       2
              */
			  DOWNLOAD_API_DEBUG("cur tag:  %d!!!!\n", p_video_info->itagArray[i]);
        switch(p_video_info->itagArray[i])
        {
        case 22:
          nHiRes = i;
          nHiTag = p_video_info->itagArray[i];
          break;

        case 84:
          if (nHiTag != 22)
          {
            nHiRes = i;
            nHiTag = p_video_info->itagArray[i];
          }
          break;
        case 37:
          if (nHiTag != 84 && nHiTag != 22)
          {
            nHiRes = i;
            nHiTag = p_video_info->itagArray[i];
          }
          break;

        case 85:
          if (nHiTag != 84 && nHiTag != 22 && nHiTag != 37)
          {
            nHiRes = i;
            nHiTag = p_video_info->itagArray[i];
          }
          break;


        case 18:
          nLowRes = i;
          nLowTag = p_video_info->itagArray[i];
          break;

        case 36:
          if (nLowTag != 18)
          {
            nLowRes = i;
            nLowTag = p_video_info->itagArray[i];
          }
          break;

        case 35:
          if (nLowTag != 18)
          {
            nLowRes = i;
            nLowTag = p_video_info->itagArray[i];
          }
          break;
        case 34:
          if (nLowTag != 18 && nLowTag != 35)
          {
            nLowRes = i;
            nLowTag = p_video_info->itagArray[i];
          }
          break;

        case 83:
          if (nLowTag != 18 && nLowTag != 35 && nLowTag != 34)
          {
            nLowRes = i;
            nLowTag = p_video_info->itagArray[i];
          }
          break;

        case 82:
          if (nLowTag != 18 && nLowTag != 35 && nLowTag != 34 && nLowTag != 83)
          {
            nLowRes = i;
            nLowTag = p_video_info->itagArray[i];
          }
          break;

        }

			}


	}
  DOWNLOAD_API_DEBUG("low res tag:  %d,high res tag:  %d!!!!\n", nLowTag,nHiTag);
  //print_itag_detail(p_video_info->itagArray[i]);

  if (nLowRes >= 0)
  {
    if(strlen(p_video_info->sigArray[nLowRes]) > 79)
    {
      if(sig_type >= 0)
      {
         decrypt_youtube_vevo(p_video_info, urlenc_data, nLowRes, sig_type);
      }
    }

    memset(p_request_url->playUrlArray[0], 0, DOWNLOAD_URL_MAX_LEN);
    memcpy(p_request_url->playUrlArray[0], p_video_info->playUrlArray[nLowRes], strlen(p_video_info->playUrlArray[nLowRes]));
    g_sd = TRUE;
  }

#ifdef ENABLE_DASH
  if(g_dash_youtube )
     save_dash_playurls(p_video_info, urlenc_data, sig_type);
#endif

  if (nHiRes >= 0)
  {

    if(strlen(p_video_info->sigArray[nHiRes]) > 79)
    {
      if(sig_type >= 0)
      {
         decrypt_youtube_vevo(p_video_info, urlenc_data, nHiRes, sig_type);
      }
    }

    memset(p_request_url->playUrlArray[1], 0, DOWNLOAD_URL_MAX_LEN);
    memcpy(p_request_url->playUrlArray[1], p_video_info->playUrlArray[nHiRes], strlen(p_video_info->playUrlArray[nHiRes]));
    g_hd = TRUE;
  }
  else
  {
#ifdef ENABLE_DASH
    nHiRes = -1;
    int is_has_hiRes = 0;
    for (i = 0; i < p_video_info->index; i++) {
        if(p_video_info->itagArray[i] == 136 || p_video_info->itagArray[i] == 140)
        {
            is_has_hiRes++;
        }
    }

    int audio_url = -1;
    int video_url = -1;
    if(is_has_hiRes == 2)
    {
        for (i = 0; i < p_video_info->index; i++)
        {
            if(p_video_info->itagArray[i] == 136 || p_video_info->itagArray[i] == 140)
            {
                //DOWNLOAD_API_DEBUG("zx p_video_info->playUrlArray :%s\n", p_video_info->playUrlArray[i]);
                if(strlen(p_video_info->sigArray[i]) > 79 && g_dash_youtube == NULL)
                {
                    decrypt_youtube_vevo(p_video_info, urlenc_data, i, sig_type);
                }
                //DOWNLOAD_API_DEBUG("zx dash p_video_info->playUrlArray :%s\n", p_video_info->playUrlArray[i]);
                if(p_video_info->itagArray[i] == 136)
                    audio_url = i;
                else if (p_video_info->itagArray[i] == 140)
                    video_url = i;
            }
        }

        if(audio_url != -1 && video_url!= -1)
        {
            if(write_dash_mpd(p_video_info, urlenc_data, audio_url, video_url) == 0)
            {
                memset(p_request_url->playUrlArray[1], 0, DOWNLOAD_URL_MAX_LEN);
                memcpy(p_request_url->playUrlArray[1], YOUTUBE_FAKE_URL, strlen(YOUTUBE_FAKE_URL));
            }
        }
    }
#endif
  }


  return parse_url_list(p_request_url, size);
}

void parse_url_component(parse_youtube_vedio_info_t * p_video_info, char * keyword, int index) {
	char * p_component_start = strstr(p_video_info->buf[index], keyword);

	if (p_component_start) {
		DOWNLOAD_API_DEBUG("[%s] find %s lalalala  !\n",__func__,keyword);
		char * p_component_end = strstr(p_component_start + strlen(keyword), "\\u0026");

        if (strcmp(keyword, "itag=") == 0)
        {
            if (p_component_end == NULL)
            {
                p_component_end = strstr(p_component_start + strlen(keyword), "&");
            }
            else
            {
                //contains \\u0026, but itag=136&key=yt5&gcr=tw&
                int tmp_cnt = p_component_end - (p_component_start + strlen(keyword));
                if(tmp_cnt > 5)
                    p_component_end = strstr(p_component_start + strlen(keyword), "&");
            }
        }

		if (p_component_end) {
			int cpy_cnt = p_component_end - (p_component_start + strlen(keyword));

			if (strcmp(keyword, "url=") == 0) {
				memset(p_video_info->playUrlArray[index], 0, DOWNLOAD_URL_MAX_LEN);
				memcpy(p_video_info->playUrlArray[index], p_component_start + strlen(keyword), cpy_cnt);
				//DOWNLOAD_API_DEBUG("[%s]fetch %s ok!\n",__func__,keyword);
				//DOWNLOAD_API_DEBUG("[%s]\n",p_video_info->playUrlArray[index]);

			} else if (strcmp(keyword, "\\u0026s=") == 0) {
				memset(p_video_info->sigArray[index], 0, SIG_STR_MAX_LEN);
				memcpy(p_video_info->sigArray[index], p_component_start + strlen(keyword), cpy_cnt);
				//DOWNLOAD_API_DEBUG("[%s]fetch %s ok!\n",__func__,keyword);
				//DOWNLOAD_API_DEBUG("[%s]\n",p_video_info->sigArray[index]);
			} else if (strcmp(keyword, "itag=") == 0) {
				char tmp[ITAG_STR_MAX_LEN] = {0};
				memset(tmp, 0, ITAG_STR_MAX_LEN);
				memcpy(tmp, p_component_start + strlen(keyword), cpy_cnt);
				p_video_info->itagArray[index] = atoi(tmp);
				//DOWNLOAD_API_DEBUG("[%s]fetch %s ok!\n",__func__,keyword);
			}

		} else {
			if (strcmp(keyword, "url=") == 0) {
				memset(p_video_info->playUrlArray[index], 0, DOWNLOAD_URL_MAX_LEN);
				strcpy(p_video_info->playUrlArray[index], p_component_start + strlen(keyword));
				//DOWNLOAD_API_DEBUG("[%s]222  fetch %s ok!\n",__func__,keyword);

			} else if (strcmp(keyword, "\\u0026s=") == 0) {
				memset(p_video_info->sigArray[index], 0, SIG_STR_MAX_LEN);
				strcpy(p_video_info->sigArray[index], p_component_start + strlen(keyword));
				//DOWNLOAD_API_DEBUG("[%s]fetch %s ok!\n",__func__,keyword);
			} else if (strcmp(keyword, "itag=") == 0) {
				char tmp[ITAG_STR_MAX_LEN] = {0};
				memset(tmp, 0, ITAG_STR_MAX_LEN);
				strcpy(tmp, p_component_start + strlen(keyword));
				p_video_info->itagArray[index] = atoi(tmp);
				//DOWNLOAD_API_DEBUG("[%s]fetch %s ok!\n",__func__,keyword);
			}
		}

	} else {
        if (strcmp(keyword, "\\u0026s=") == 0)
        {
            //maybe s=21021041222B3D22A63D7AD6A61AAD01E75B52C643C.4A9985BC199300F53F274BC394CF7B9F68CCA614\u0026type=video/3gpp;....
            if(*(p_video_info->buf[index]) == 's' && *(p_video_info->buf[index]+1) == '=')
            {
                p_component_start = p_video_info->buf[index];
                char * p_component_end = strstr(p_component_start + 2, "\\u0026");
                memset(p_video_info->sigArray[index], 0, SIG_STR_MAX_LEN);
				memcpy(p_video_info->sigArray[index], p_component_start + 2, (p_component_end - p_component_start - 2));
                DOWNLOAD_API_DEBUG("222 s= sig[%s]\n",p_video_info->sigArray[index]);
            }
        }
        else
        {
            DOWNLOAD_API_ERROR("[ERROR]  fail to parse the component :%s !!!\n", keyword);
            DOWNLOAD_API_ERROR("[ERROR]  failed buf[%d] :%s !!!\n", index, p_video_info->buf[index]);
        }
	}

	//DOWNLOAD_API_DEBUG("[%s] end end ...\n",__func__);
}

void  resetParseYoutubeVideoInfoData(parse_youtube_vedio_info_t * p_info) {
	if (p_info == NULL) {
		DOWNLOAD_API_ERROR("[%s] ERROR p_info == NULL do nothing !!!\n", __func__);
		return;
	}

	if (p_info->index > 0) {
		int i = 0;
        DOWNLOAD_API_ERROR("[%s] p_info->index[%d]!!!\n", __func__, p_info->index);
		for (i = 0; i < p_info->index ; i++) {
			if (p_info->buf[i]) {
				free(p_info->buf[i]);
				p_info->buf[i] = NULL;
			}
		}

		memset(p_info, 0, sizeof(parse_youtube_vedio_info_t));
	}

	return;
}
// add by libin
static char* strncpy_skip_value(char *destaddr, char *srcaddr, int len, char value)
{
    char* dest=destaddr;
    char const* src=srcaddr;
    char v;
    while(len-->0)
    {
        v = *src++;
        if(v != value)
            *dest++ = v;
    }
    *dest='\0';
    return destaddr;
}
/*e.g.  add by libin
you can see info like follows in webinfo:

"hlsManifestUrl":"https:\/\/manifest.googlevideo.com\/api\/manifest\/hls_variant\/expire\/1522325022\/requiressl\/yes\/go\/1\/key\/\
yt6\/id\/X7Ktabhd8a4.1\/sparams\/ei%2Cgcr%2Cgo%2Chfr%2Cid%2Cip%2Cipbits%2Citag%2Cmaudio%2Cplaylist_type%2Cratebypass%2C\
requiressl%2Csource%2Ctx%2Ctxs%2Cexpire\/gcr\/tw\/itag\/0\/source\/yt_live_broadcast\/ratebypass\/yes\/maudio\/1\/ip\/118.163.116.68\
/ei\/voG8WtTCF4XAgQPPj4-4Cw\/tx\/23727951\/hfr\/1\/ipbits\/0\/signature\/D108CB94ECA871C6E40F75B0EC84D946A65ABC97.A734AF388B\
1D61BBDA3D2F9410CD1D775CD5F3DC\/dover\/10\/playlist_type\/DVR\/txs\/23727950%2C23727951\/keepalive\/yes\/file\/index.m3u8",

we will get the url from checking this infomation
*/
MT_BOOL youtube_check_live_stream_url(char *webinfo, char *p_indexname)
{

   // #define LIVE_STREAM_KEY     "\"hlsvp\""
    #define LIVE_STREAM_KEY     "hlsManifestUrl"

    DOWNLOAD_API_DEBUG("[%s] start start\n", __func__);
    char *p_start = NULL;
    char *p_end = NULL;

    if(webinfo == NULL)
        return FALSE;
    if(p_indexname == NULL)
        return FALSE;

    p_start = strstr(webinfo,LIVE_STREAM_KEY);//hlsvp the key from chenzuping.
    if(p_start == NULL)
        return FALSE;

    //p_start += strlen(LIVE_STREAM_KEY)+5;//"hlsManifestUrl":"https:
    p_start += strlen(LIVE_STREAM_KEY);
    p_start = strstr(p_start,"http");//"hlsManifestUrl":"https:
    if(p_start == NULL)
        return FALSE;
    p_end = strstr(p_start,"m3u8");////search ", to get the tail of url

    if(p_end == NULL)
        return FALSE;

    p_end +=strlen("m3u8");

    strncpy_skip_value(p_indexname, p_start, p_end-p_start, 0x5c);///0x5c means "\"

    DOWNLOAD_API_DEBUG("[%s] end end\n", __func__);
    return TRUE;

}

int youtube_get_video_play_url(const char * p_videoid, request_url_t * p_request_url, u8 size, u8 is_live)
{
    int  par_ret = 0;
    int ret;
	int download_retry = 0;
	DOWNLOAD_API_DEBUG("video id: [%s]\n", p_videoid);
	char tmp_buf[1280] = {0};
	memset(tmp_buf, 0, 1280);
    if(g_dash_youtube)
    {
        memset(g_dash_youtube, 0, sizeof(dash_youtube_playurl_info_t));
    }
    if(is_live)
    {
	    snprintf(tmp_buf, 1200, "https://www.youtube.com/get_video_info?el=player_embedded&video_id=%s", p_videoid);
    }
    else
    {
	    snprintf(tmp_buf, 1200, "https://www.youtube.com/watch?v=%s&gl=US&hl=en&has_verified=1", p_videoid);///changed by xizhou
    }
	DOWNLOAD_API_DEBUG("[url]:%s\n", tmp_buf);

	for(download_retry = 0; download_retry < MAX_POST_COUNT; download_retry++)
	{
		ret = Nw_DownloadURLTimeout(tmp_buf, YOUTUBE_WATCH_PAGE, DEFAULT_DL_HTML_TIMEOUT,NULL,NULL,NULL,NULL,0);

		if (ret == 1 || ret == -1) {
			break;
		}
	}
	if(download_retry == 4 || ret == -1)
	{
		DOWNLOAD_API_ERROR("[%s][ERROR] do nothing, fail to download youtube_watch_page ...\n ", __func__);
		return -1;
	}

    parse_youtube_vedio_info_t *p_video_info;
    p_video_info = (parse_youtube_vedio_info_t *)malloc(sizeof(parse_youtube_vedio_info_t));

	if (p_video_info) {
		memset(p_video_info, 0, sizeof(parse_youtube_vedio_info_t));
	}
    else
    {
        DOWNLOAD_API_ERROR("[%s][ERROR] alloc parse_youtube_vedio_info_t failed...\n ", __func__);
		return -5;
    }

    youtube_search_key_t youtube_key;
    memset(&youtube_key, 0, sizeof(youtube_search_key_t));
    //set default vale.
    strcpy(youtube_key.keyArray[0], "adaptive_fmts\":");
    strcpy(youtube_key.keyArray[1], "\",");
    strcpy(youtube_key.keyArray[2], "url_encoded_fmt_stream_map\":");
    strcpy(youtube_key.keyArray[3], "\",");
    strcpy(youtube_key.keyArray[4], KW_URL);
    strcpy(youtube_key.keyArray[5], KW_SIG);
    strcpy(youtube_key.keyArray[6], KW_ITAG);

	FILE * fp = NULL;
	fp = fopen(YOUTUBE_WATCH_PAGE, "rb");
	char * tmp_buf2 = NULL;

	if (fp) {
		DOWNLOAD_API_LOG("[%s]  open youtube_watch_page ok !!!!\n", __func__);
		tmp_buf2 = (char *)malloc(JS_INFO_BUF_LEN);

		if (tmp_buf2) {
			memset(tmp_buf2, 0, JS_INFO_BUF_LEN);
			int rdNum = fread(tmp_buf2, 1, JS_INFO_BUF_LEN, fp);

			if (rdNum > 0) {
				DOWNLOAD_API_LOG("[%s] read %d bytes from youtube_watch_page!!!\n", __func__, rdNum);
			}

			resetParseYoutubeVideoInfoData(p_video_info);
			p_video_info->index = 0;
			fclose(fp);
			fp = NULL;

            char * p_indexname =NULL;  //LIVE_INDEX_URL_LEN
            p_indexname=(char *)malloc(LIVE_INDEX_URL_LEN);
        	memset(p_indexname, 0, LIVE_INDEX_URL_LEN);
            if(youtube_check_live_stream_url(tmp_buf2, p_indexname) == FALSE)//add by libin ,becauese some url like https://www.youtube.com/watch?v=h5CHkwqogXU is live stream ,and we can get url from the web info.
           	{
			    par_ret = youtube_parse_stream_map_list(p_videoid, p_video_info, tmp_buf2, p_request_url, size, youtube_key);
    		}
    		else
    		{
    			is_live=true; //@todo
    			par_ret = parse_youtube_live_list(p_videoid, p_video_info, p_indexname, p_request_url, size);
    		}
            free(p_indexname);
			free(tmp_buf2);

			if (par_ret != 0)
			{
                DOWNLOAD_API_ERROR("[%s] youtube_parse_stream_map_list failed!!!\n", __func__);
			}
		}
	}


    delete_file(YOUTUBE_WATCH_PAGE);
    if(p_video_info)
	{
        resetParseYoutubeVideoInfoData(p_video_info);
		free(p_video_info);
		p_video_info = NULL;
	}
    return par_ret;


}


//p_request_url[0] is ld stream, p_request_url[1] is sd, p_request_url[2] is hd
int Nw_Request_Website_DownloadURL(u8 website, const char * url, u8 size, request_url_t * p_request_url, u8 timeout)
{
    int ret = -1;
    u8  is_live = 0;
    int request_size = get_request_size(size);
    if(request_size == 0)
    {
        DOWNLOAD_API_ERROR("[ERROR] Input usless size:%d\n ", __func__, size);
        return -3;
    }
    DOWNLOAD_API_DEBUG("[%s] website[%d] video size[%d]\n ", __func__, website, size);

    memset(p_request_url, 0, sizeof(request_url_t));

    switch (website)
    {
        case WEBSITE_YOUTUBE:
        {
            DOWNLOAD_API_DEBUG("[%s]youtube url[%s] ...\n ", __func__, url);
            ///youtube extract the p_videoid
            const char *   p_url_start = NULL;
            const char  *  p_url_end = NULL;;
            char p_videoid[MAX_VIDEOID_LEN];

            int urlLen = strlen(url);
            if (url[urlLen + 1] != NULL)
            {
              strcpy((char*)youtubeServerUrl,&url[urlLen + 1]);
            }
            DOWNLOAD_API_ERROR("youtubeServerUrl :%s\n",youtubeServerUrl);
            memset(p_videoid, 0, MAX_VIDEOID_LEN);
            p_url_start = strstr((char*)url, "watch?v=");
            if(p_url_start != NULL)
            {
                p_url_start = p_url_start + strlen("watch?v=");
                p_url_end = strstr((char*)url, "_vevo");
                if(p_url_end == NULL)
                {
                    p_url_end = strstr((char*)url, "_live");
                    if(p_url_end == NULL)
                    {
                  p_url_end = strstr((char*)url, "&");
                  if(p_url_end)
                  {
                     memcpy(p_videoid, p_url_start, (p_url_end-p_url_start));
                     DOWNLOAD_API_DEBUG("[%s] %s, p_videoid [%s]  ...\n ", __func__, p_url_start, p_videoid);
                  }
                  else
                  {
                      p_url_end = (char*)(url+strlen(url));
                      memcpy(p_videoid, p_url_start, (p_url_end-p_url_start+1));
                      DOWNLOAD_API_DEBUG("[%s]zx1 [%s] ...\n ", __func__, p_videoid);
                  }
                    }
                    else
                    {
                        is_live = 1;
                        memcpy(p_videoid, p_url_start, (p_url_end-p_url_start));
                        DOWNLOAD_API_DEBUG("[%s]zx2 [%s] ...\n ", __func__, p_videoid);
                    }
                }
                else
                {
                    memcpy(p_videoid, p_url_start, (p_url_end-p_url_start));
                }
                ret = youtube_get_video_play_url(p_videoid, p_request_url, size, is_live);
            }
            else
            {
                //youtubeDataprovider gives p_videoid
                ret = youtube_get_video_play_url(url, p_request_url, size, is_live);
            }

            DOWNLOAD_API_DEBUG("[%s] zx g_only_use_youtube_dash[%d] g_dash_youtube[0x%x] \n ",
                __func__, g_only_use_youtube_dash, g_dash_youtube);
            if(g_only_use_youtube_dash == 1 && g_dash_youtube)
            {
                if(g_dash_youtube->total_urls > 0)
                {
                    char *p_fake_url = select_dash_youtube_url(g_dash_youtube->total_urls - 1);
                    memset(p_request_url->playUrlArray[0], 0, DOWNLOAD_URL_MAX_LEN);
                    strcpy(p_request_url->playUrlArray[0], p_fake_url);
                    memset(p_request_url->playUrlArray[1], 0, DOWNLOAD_URL_MAX_LEN);
                    strcpy(p_request_url->playUrlArray[1], p_fake_url);
                }
            }
            break;
        }

        default:
        {
            DOWNLOAD_API_ERROR("[ERROR] unsupport url[%s]\n ", __func__, url);
            break;
        }
    }

    return ret;
}


