/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
#if 1 //def __LINUX__
#include "mplayer/config.h"
#ifdef __LINUX__
#include <stdio.h>
#include <stdlib.h>
#else
#include "mp_func_trans.h"
#endif
#include <string.h>

#include "mt_common.h"

#include "mt_type.h"

#include "mplayer/libmpdemux/stheader.h"
#include "demux_mp_misc.h"

double video_time_usage;
double vout_time_usage;
double force_fps;
char *audio_lang;
int sub_visibility = 1;
//ASS_Track* ass_track = 0;
//int ass_enabled = 0;
#define VERSION "SVN-r35368-snapshot-4.4.3"
const char *mplayer_version = "MPlayer " VERSION;
//ASS_Library *ass_library;
#if CONFIG_FONTCONFIG
int font_fontconfig = 1;
#else
int font_fontconfig = -1;
#endif
char *font_name = NULL;
char *sub_font_name = NULL;
//char *sub_cp=NULL;
float text_font_scale_factor = 3.5;
int subtitle_autoscale = 3;
// sub:
float font_factor = 0.75;
char **sub_name;
char **sub_paths;
float sub_delay;
float sub_fps;
int sub_auto = 1;
char *vobsub_name;
int subcc_enabled;
int suboverlap_enabled = 1;
char *dvdsub_lang;

#define FF_LOG2(X) (31 - __builtin_clz((X) | 1))
// streaming:
int audio_id = -1;
int video_id = -1;
int dvdsub_id = -1;
int sub_utf8 = 0;
//subtitle* vo_sub=NULL;

int av_log2(unsigned v)
{
    return FF_LOG2(v);
}

void resync_video_stream(sh_video_t *sh_video)
{
    (void)(sh_video);
}
void resync_audio_stream(sh_audio_t *sh_audio)
{
    (void)(sh_audio);
}
void skip_audio_frame(sh_audio_t *sh_audio)
{
    (void)(sh_audio);
}
int teletext_control(void *p, int cmd, void *arg)
{
    (void)(p);
    (void)(cmd);
    (void)(arg);
    return 0;
}
//char *get_path(const char *filename){}
#if 0 //def __LINUX__
typedef void * hfile_t;
/*!
  vfs seek mode
  */
typedef enum
{
  /*!
    seek from head.
    */
  VFS_FILE_SEEK_SET,
  /*!
    seek from tail.
    */
  VFS_FILE_SEEK_CUR,
  /*!
    seek from curn.
    */
  VFS_FILE_SEEK_END,
}vfs_file_seek_mode_t;
hfile_t vfs_open(u8 *p_path, u32 mode)
{
}
void vfs_seek(hfile_t file, s64 offset, vfs_file_seek_mode_t mode)
{
}
u32 vfs_tell(hfile_t file)
{
}
void vfs_close(hfile_t file)
{
}
u32 vfs_read(void * p_buf, u32 count, u32 size, hfile_t file)
{
}
#endif

#if 0 //ndef __LINUX__
size_t av_strlcpy(char *dst, const char *src, size_t size) {
    size_t len = 0;

    while (++len < size && *src)
        *dst++ = *src++;

    if (len <= size)
        *dst = 0;

    return len + strlen(src) - 1;
}
#endif

int usec_sleep(int usec_delay)
{
#ifdef __LINUX__
//#ifdef HAVE_NANOSLEEP
//    struct timespec ts;
//    ts.tv_sec = usec_delay / 1000000;
//    ts.tv_nsec = (usec_delay % 1000000) * 1000;
//    return nanosleep(&ts, NULL);
//#else
    return MT_USLEEP(usec_delay);
//#endif
#else
    mtos_task_sleep(usec_delay / 1000);
    return 0;
#endif
}

#define FETCH_IP_ADDR_URL "http://iframe.ip138.com/ic.asp"
#define FETCH_IP_ADDR_RSS "r:local_ip_addr.rss"
#define FETCH_IP_COUNTRY_RSS "r:local_ip_country.rss"
#define DEFAULT_DOWNLOAD_TIMEOUT (15)
#define VID_HTML_MAX_SIZE 1024
int get_local_ip(char *ip_addr)
{
#ifndef __LINUX__
    char buf[128] = { 0 };
    int ret;
    ufs_file_t *p_fp;
    u16 *p_filename = NULL;
    void *p_addr = NULL;
    u32 real_size = 0;
    char *p_start = NULL;
    char *p_end = NULL;
    char *p = NULL;
    int size = 0;

    strcat(buf, FETCH_IP_ADDR_URL);
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();
    if ((p_file_seq != NULL) && (p_file_seq->http_task_prio != 0))
	Nw_Download_Init(p_file_seq->http_task_prio, NULL, 0);
    else
	Nw_Download_Init(127, NULL, 0);
    ret = Nw_DownloadURLTimeout(buf, FETCH_IP_ADDR_RSS, DEFAULT_DOWNLOAD_TIMEOUT, NULL, NULL, NULL, NULL, 0);
    if (ret != 1) {

	return -1;
    }
    p_fp = (ufs_file_t *)mtos_malloc(sizeof(ufs_file_t));
    if (p_fp == NULL)
	return -1;
    memset(p_fp, 0, sizeof(ufs_file_t));

    unsigned short path_tmp[256] = { 0 };
    p_filename = Convert_Utf8_To_Unicode((unsigned char *)FETCH_IP_ADDR_RSS, path_tmp);
    if (ufs_open(p_fp, p_filename, (op_mode_t)(UFS_READ | UFS_OPEN)) != 0) {

	return -1;
    }
    p_addr = mtos_align_malloc(VID_HTML_MAX_SIZE, 16);
    if (p_addr == NULL) {
        ufs_close(p_fp);
        return -1;
    }
    ret = ufs_read(p_fp, p_addr, VID_HTML_MAX_SIZE, &real_size);

    if ((p_start = strstr((char *)p_addr, "["))) {
	p_end = strstr(p_start, "]");
	size = p_end - (p_start + 1);
	if (size > 0 && size < 32) {

	    memcpy(ip_addr, p_start + 1, size);
	    OS_PRINTF("\n%s %d %s\n", __func__, __LINE__, ip_addr);

	} else
	    return -1;
    }
    ufs_close(p_fp);
#endif
    return 0;
}
int get_ip_country(char *ip_addr, char *country_id)
{
#ifndef __LINUX__

    char taobao_ipinfo[128] = { "http://ip.taobao.com/service/getIpInfo.php?ip=" };
    int ret;
    ufs_file_t *p_fp;
    u16 *p_filename = NULL;
    void *p_addr = NULL;
    u32 real_size = 0;
    char *p_start = NULL;
    char *p_end = NULL;
    char *p = NULL;
    int size = 0;
    strcat(taobao_ipinfo, ip_addr);
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();
    if ((p_file_seq != NULL) && (p_file_seq->http_task_prio != 0))
    Nw_Download_Init(p_file_seq->http_task_prio, NULL, 0);
    else
	Nw_Download_Init(127, NULL, 0);
    ret = Nw_DownloadURLTimeout(taobao_ipinfo, FETCH_IP_COUNTRY_RSS, DEFAULT_DOWNLOAD_TIMEOUT, NULL, NULL, NULL, NULL, 0);
    if (ret != 1) {

	return -1;
    }
    p_fp = (ufs_file_t *)mtos_malloc(sizeof(ufs_file_t));
    if (p_fp == NULL)
	return -1;
    memset(p_fp, 0, sizeof(ufs_file_t));

    unsigned short path_tmp[256] = { 0 };
    p_filename = Convert_Utf8_To_Unicode((unsigned char *)FETCH_IP_COUNTRY_RSS, path_tmp);
    if (ufs_open(p_fp, p_filename, (op_mode_t)(UFS_READ | UFS_OPEN)) != 0) {

	return -1;
    }
    p_addr = mtos_align_malloc(VID_HTML_MAX_SIZE, 16);
    if (p_addr == NULL)
    {
        ufs_close(p_fp);
        return -1;
    }
    ret = ufs_read(p_fp, p_addr, VID_HTML_MAX_SIZE, &real_size);

    if ((p_start = strstr((char *)p_addr, "country_id"))) {
	p_start += (strlen("country_id") + 2);
	p_end = strstr(p_start, "area");
	size = p_end - 3 - (p_start + 1);
	if (size > 0 && size < 32) {

	    memcpy(country_id, p_start + 1, size);
	    OS_PRINTF("\n%s %d %s\n", __func__, __LINE__, country_id);

	} else
	    return -1;
    }
    ufs_close(p_fp);
#endif
    return 0;
}
#endif
