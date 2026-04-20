/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "dirent.h"
#define MODULE_TAG "FPM"
#include "mutil.h"
#include "mlog.h"
#include "drv_adp.h"
#include "file_playback_sequence.h"
#include "file_seq_internal.h"
#include "file_seq_misc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#if MT_DES("Internal API Definition", 1)

static int is_folder_exist(const char *folder_name)
{
	DIR *dir = opendir(folder_name);
	if(NULL != dir){
		closedir(dir);
		return MT_TRUE;
	}
	return MT_FALSE;
}

#endif

#if MT_DES("External API Definition", 1)
int run_memory(MT_BOOL flush_cache)
{
    sys_mem_debug_t curr_mem_info = {0};

    show_sys_memory_info(&(curr_mem_info), &(curr_mem_info), MT_FALSE);

    return MT_SUCCESS;
}

void open_es_dump_file(PLAYBACK_INTERNAL_T *pbi, STREAM_TYPE_E type)
{
#define MAX_FILE_NMAE_LEN  128
    const unsigned char *disk_path = NULL;
    unsigned char fname[MAX_FILE_NMAE_LEN] = {0};
    struct {
        unsigned char *type_str;
        RET_CODE (*dump_on_func)(void);
        FILE **fp;
    } info[STREAM_TYPE_BUTT] = {
        {""   /* STREAM_TYPE_NULL */, NULL, NULL},
        {"aes"/* STREAM_TYPE_AUD  */, suplayer_get_dump_aud_flag , &(pbi->audio.dump_aes_fp)},
        {"ves"/* STREAM_TYPE_VID  */, suplayer_get_dump_vid_flag , &(pbi->video.dump_ves_fp)},
        {"ses"/* STREAM_TYPE_SUB  */, suplayer_get_dump_subt_flag, &(pbi->subtile.dump_ses_fp)},
        {"raw"/* STREAM_TYPE_RAW  */, NULL},
    };

    if (type <= STREAM_TYPE_NULL || type >= STREAM_TYPE_RAW) {
        MLOGE("Open dump file type error\n");
        return;
    }
    if (NULL != *(info[type].fp)) {
        fclose(*(info[type].fp));
        *(info[type].fp) = NULL;
    }
    if(0 == info[type].dump_on_func()) {
        return;
    }

    if (MT_TRUE == is_folder_exist("/media/casetest")) {
        disk_path = "/media/casetest";
    } else if (MT_TRUE == is_folder_exist("/media/sda1")) {
        disk_path = "/media/sda1";
    } else {
        MLOGE("Not allowed dump disk path\n");
        return;
    }

    unsigned short ms, us;
    unsigned char  hour, minute, second;
    mclock_get_clock(&hour, &minute, &second, &ms, &us);
    sprintf(fname, "%s/dump_%s_%2d%2d%2d", disk_path,
        info[type].type_str, (int) hour, (int) minute, (int) second);

    *(info[type].fp) = fopen(fname, "wb+");

    if (NULL == *(info[type].fp)) {
        MLOGW("Open video dump file error\n");
        return;
    }

    MLOGI("Open video dump file %s success\n", fname);
}

void write_es_dump_file(PLAYBACK_INTERNAL_T *pbi,
    STREAM_TYPE_E type, const void *start_addr, unsigned int size)
{
    FILE *fp[STREAM_TYPE_BUTT] = {
        /* STREAM_TYPE_NULL */NULL,
        /* STREAM_TYPE_AUD  */pbi->audio.dump_aes_fp,
        /* STREAM_TYPE_VID  */pbi->video.dump_ves_fp,
        /* STREAM_TYPE_SUB  */pbi->subtile.dump_ses_fp,
        /* STREAM_TYPE_RAW  */NULL,
    };

    if (type <= STREAM_TYPE_NULL || type >= STREAM_TYPE_RAW) {
        MLOGE("Write dump file type error\n");
        return;
    }

    if (NULL == fp[type]) {
        return;
    }
    fwrite(start_addr, size, 1, fp[type]);
}

void close_es_dump_file(PLAYBACK_INTERNAL_T *pbi, STREAM_TYPE_E type)
{
    FILE **fp[STREAM_TYPE_BUTT] = {
        /* STREAM_TYPE_NULL */NULL,
        /* STREAM_TYPE_AUD  */&(pbi->audio.dump_aes_fp),
        /* STREAM_TYPE_VID  */&(pbi->video.dump_ves_fp),
        /* STREAM_TYPE_SUB  */&(pbi->subtile.dump_ses_fp),
        /* STREAM_TYPE_RAW  */NULL,
    };

    if (type <= STREAM_TYPE_NULL || type >= STREAM_TYPE_RAW) {
        MLOGE("Close dump file type error\n");
        return;
    }
    if (NULL != *(fp[type])) {
        fclose(*(fp[type]));
        *(fp[type]) = NULL;
    }
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
