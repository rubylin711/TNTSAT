/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/******************************************************************************************
  File Name     : suplayer_intf.c
  Version       : Initial Draft
  Author        : Montage tech multimedia software group
  Created       : 2016/01/06
  Description   :
  History       :
  1.Date        : 2016/01/06
    Author      :
    Modification: Created file

*******************************************************************************/
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/cpufreq.h>

#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_mmz.h"
#include "mt_drv_suplayer.h"
#include "mt_error_mpi.h"
#include "mt_drv_module.h"
#include "mt_module.h"
#include "mt_kernel_adapt.h"
#include "drv_suplayer_ioctl.h"
#include "mt_osal.h"
#include "mt_module_debug.h"
#include "mt_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#define SUPLAYER_NAME         "MT_SUPLAYER"

static mt_device_s          g_SuplayerRegisterData;
static atomic_t             g_SuplayerCount = ATOMIC_INIT(0);
SUPLAYER_GLOBAL_STATE_S       g_SuplayerGlobalState;

MT_DECLARE_MUTEX(g_SuplayerMutex);

static mt_s32 SUPLAYER_ProcParsePara(mt_char *pProcPara,mt_char **ppItem,mt_char **ppValue)
{
    mt_char *pChar = MT_NULL;
    mt_char *pItem,*pValue;

    pChar = strchr(pProcPara,'=');
    if (MT_NULL == pChar)
    {
        return MT_FAILURE; /* Not Found '=' */
    }

    pItem = pProcPara;
    pValue = pChar + 1;
    *pChar = '\0';

    /* remove blank bytes from item tail */
    pChar = pItem;
    while(*pChar != ' ' && *pChar != '\0')
    {
        pChar++;
    }
    *pChar = '\0';

    /* remove blank bytes from value head */
    while(*pValue == ' ')
    {
        pValue++;
    }

    *ppItem = pItem;
    *ppValue = pValue;
    return MT_SUCCESS;
}

static mt_void SUPLAYER_ProcPrintHelp(mt_void)
{
    mt_drv_proc_echohelp("echo SendAud=1|0 > /proc/msp/suplayerxx_control, enable or disable send audio es to avplay\n");
    mt_drv_proc_echohelp("echo SendVid=1|0 > /proc/msp/suplayerxx_control, enable or disable send video es to avplay\n");
    mt_drv_proc_echohelp("echo SendSubt=1|0 > /proc/msp/suplayerxx_control, enable or disable send subtitle es out\n");
    mt_drv_proc_echohelp("echo DumpAes=2|1|0 > /proc/msp/suplayerxx_control, enable or disable dump audio es to file, 2:dump aes with header. 1:dump aes without header. 0:disable dump aes\n");
    mt_drv_proc_echohelp("echo DumpVes=1|0 > /proc/msp/suplayerxx_control, enable or disable dump video es to file\n");
    mt_drv_proc_echohelp("echo DumpSes=1|0 > /proc/msp/suplayerxx_control, enable or disable dump subtitle es to file, not support now\n");
    mt_drv_proc_echohelp("echo LogLv=0~x > /proc/msp/suplayerxx_control, 0:disable proc and print, >0:enable proc info, >1/2/3/4 enable prints\n");
    mt_drv_proc_echohelp("cat /proc/msp/suplayerxx_fileinfo, print fileinfo(xx=00~n instance num)\n");
    mt_drv_proc_echohelp("cat /proc/msp/suplayerxx_format, print demuxer info\n");
    mt_drv_proc_echohelp("cat /proc/msp/suplayerxx_adapter, print adapter info\n");
    mt_drv_proc_echohelp("cat /proc/msp/suplayerxx_control, print control info\n");
    return;
}

static mt_s32 SUPLAYER_ProcRead_Control(struct seq_file *p, mt_void *v)
{
    SUPLAYER_S          *pSuplayer = MT_NULL;
    mt_u32            SuplayerId;
    mt_proc_entry_t  *pProcItem;

    v = v;
    pProcItem = p->private;

    SuplayerId = (pProcItem->entry_name[8] - '0')*10 + (pProcItem->entry_name[9] - '0');
    
    pSuplayer = g_SuplayerGlobalState.SuplayerInfo[SuplayerId].pSuplayer;
    if (MT_NULL == pSuplayer)
    {
        SUPLAYER_ProcPrintHelp();
        return -EFAULT;
    }
    PROC_PRINT(p, "\n%s\n",__func__);
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Info Start-------------------\n", SuplayerId);
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Control  Info Start-------------------\n", SuplayerId);
#if 1
    PROC_PRINT(p,
                    "cur status    :%s\n"
                    "goto status    :%u\n"
                    "play speed    :%d(x)\n"
                    "cur video id    :%u\n"
                    "cur audio id            :%u\n"
                    "cur subtitle id            :%u\n"
                    "empty count            :%u\n"
                    "play progress            :%u(0-100)\n"
                    "download progress            :%u(0-100)\n"
                    "last play duration            :%u(ms)\n"
                    "first pts            :%u(ms)\n"
                    "last pts            :%u(ms)\n"
                    "last I frame pts            :%u(ms)\n"
                    "last seek pts            :%u(ms)\n"
                    "call seek pts            :%u(ms)\n"
                    "first pts after seek            :%u(ms)\n"
                    "call seek pos            :%u(byte)\n"
                    "last vid pos            :%u(byte)\n"
                    "chip type    :%u\n"
                    "fill state            :%u\n"
                    "aud_out    :%u\n"
                    "vdec_policy            :%u\n"
                    "video            :\n"
                    "   es push cnt            :%u\n"
                    "   es handle cnt            :%u\n"
                    "   es buf full cnt            :%u\n"
                    "   es eof                   :%u\n"
                    "   es buf size            :%u KB\n"
                    "   es buf free size            :%u KB\n"
                    "   es get fail            :%u\n"
                    "   es hungry            :%u\n"
                    "   ds bytes            :%u Bytes\n"
                    "   opts            :%ums\n"
                    "   spts            :%ums\n"
                    "   ppts            :%ums\n"
                    "   codec support            :%d\n"
                    "audio            :\n"
                    "   es push cnt            :%u\n"
                    "   es handle cnt            :%u\n"
                    "   es buf full cnt            :%u\n"
                    "   es eof                   :%u\n"
                    "   es buf size            :%u Bytes\n"
                    "   es buf free size            :%u Bytes\n"
                    "   es get fail            :%u\n"
                    "   es hungry            :%u\n"
                    "   ds bytes            :%u\n"
                    "   opts            :%ums\n"
                    "   spts            :%ums\n"
                    "   ppts            :%ums\n"
                    "   codec support            :%d\n"
                    "subt            :\n"
                    "   es push cnt            :%u\n"
                    "   es handle cnt            :%u\n"
                    "   es eof               :%u\n"
                    "   ds bytes            :%u Bytes\n"
                    "   opts            :%ums\n"
                    "   spts            :%ums\n"
                    "   codec support            :%d\n",
                    pSuplayer->ctrl.cur_status,
                    pSuplayer->ctrl.goto_status,
                    pSuplayer->ctrl.speed,
                    pSuplayer->ctrl.vid.id,
                    pSuplayer->ctrl.aud.id,
                    pSuplayer->ctrl.subt.id,
                    pSuplayer->ctrl.empty_cnt,
                    pSuplayer->ctrl.play_progress,
                    pSuplayer->ctrl.download_progress,
                    pSuplayer->ctrl.last_play_duration,
                    pSuplayer->ctrl.first_pts,
                    pSuplayer->ctrl.last_pts,
                    pSuplayer->ctrl.last_iframe_pts,
                    pSuplayer->ctrl.last_seek_pts,
                    pSuplayer->ctrl.call_seek_pts,
                    pSuplayer->ctrl.first_pts_after_seek,
                    pSuplayer->ctrl.call_seek_pos,
                    pSuplayer->ctrl.last_vid_pos,
                    pSuplayer->ctrl.chip_type,
                    pSuplayer->ctrl.fill_es_states,
                    pSuplayer->ctrl.aud_out_mode,
                    pSuplayer->ctrl.vdec_policy,
                    pSuplayer->ctrl.vid.es_push_cnt,
                    pSuplayer->ctrl.vid.es_hdl_true_cnt,
                    pSuplayer->ctrl.vid.es_buf_full_cnt,
                    pSuplayer->ctrl.vid.es_eof,
                    pSuplayer->ctrl.vid.es_buf_size,
                    pSuplayer->ctrl.vid.es_buf_free_size,
                    pSuplayer->ctrl.vid.es_get_fail,
                    pSuplayer->ctrl.vid.es_hungry,
                    pSuplayer->ctrl.vid.ds_bytes,
                    pSuplayer->ctrl.vid.orig_pts,
                    pSuplayer->ctrl.vid.sys_pts/1000,
                    pSuplayer->ctrl.vid.play_pts/1000,
                    pSuplayer->ctrl.vid.codec_support,
                    pSuplayer->ctrl.aud.es_push_cnt,
                    pSuplayer->ctrl.aud.es_hdl_true_cnt,
                    pSuplayer->ctrl.aud.es_buf_full_cnt,
                    pSuplayer->ctrl.aud.es_eof,
                    pSuplayer->ctrl.aud.es_buf_size,
                    pSuplayer->ctrl.aud.es_buf_free_size,
                    pSuplayer->ctrl.aud.es_get_fail,
                    pSuplayer->ctrl.aud.es_hungry,
                    pSuplayer->ctrl.aud.ds_bytes,
                    pSuplayer->ctrl.aud.orig_pts,
                    pSuplayer->ctrl.aud.sys_pts/1000,
                    pSuplayer->ctrl.aud.play_pts/1000,
                    pSuplayer->ctrl.aud.codec_support,
                    pSuplayer->ctrl.subt.es_push_cnt,
                    pSuplayer->ctrl.subt.es_hdl_true_cnt,
                    pSuplayer->ctrl.subt.es_eof,
                    pSuplayer->ctrl.subt.ds_bytes,
                    pSuplayer->ctrl.subt.orig_pts,
                    pSuplayer->ctrl.subt.sys_pts/1000,
                    pSuplayer->ctrl.subt.codec_support
                    );
#endif
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Control  Info End-------------------\n", SuplayerId);
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Tplay Control  Info Start-------------------\n", SuplayerId);
#if 1
    PROC_PRINT(p,
                    "total send frame            :%u\n"
                    "sys time of send last i frame            :%u(ms)\n"
                    "total i frame scale            :%u(ms)\n"
                    "base sys time of tplay            :%u(ms)\n"
                    "base i frame pts of tplay            :%u(ms)\n"
                    "num of discard i frame            :%u\n"
                    "num of send p&b frame            :%u\n",
                    pSuplayer->ctrl.tplay.totol_send_frame,
                    pSuplayer->ctrl.tplay.sys_time_of_send_last_iframe,
                    pSuplayer->ctrl.tplay.total_iframe_scale,
                    pSuplayer->ctrl.tplay.base_systime_of_tplay,
                    pSuplayer->ctrl.tplay.base_iframe_pts_of_tplay,
                    pSuplayer->ctrl.tplay.num_of_discard_iframe,
                    pSuplayer->ctrl.tplay.num_of_send_pbframe
                    );
#endif
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Tplay Control  Info End-------------------\n", SuplayerId);
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Netbuffer Control  Info Start-------------------\n", SuplayerId);
#if 1
    PROC_PRINT(p,
                    "status of buffer            :%u\n"
                    "duration of format buffer            :%u(ms)\n"
                    "size of format buffer            :%u(bytes)\n"
                    "duration of player buffer            :%u(ms)\n"
                    "size of player buffer            :%u(bytes)\n"
                    "download finish            :%u\n"
                    "reset player            :%u\n"
                    "report first frame time            :%u\n"
                    "set buffer under run            :%u\n",
                    pSuplayer->ctrl.netbuf.buf_status,
                    pSuplayer->ctrl.netbuf.duration_of_format_buf,
                    pSuplayer->ctrl.netbuf.size_of_format_buffer,
                    pSuplayer->ctrl.netbuf.duration_of_player_buffer,
                    pSuplayer->ctrl.netbuf.size_of_player_buffer,
                    pSuplayer->ctrl.netbuf.download_finish,
                    pSuplayer->ctrl.netbuf.reset_player,
                    pSuplayer->ctrl.netbuf.report_first_frame_time,
                    pSuplayer->ctrl.netbuf.set_buffer_under_run
                    );
#endif
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Netbuffer Control  Info End-------------------\n", SuplayerId);

    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Proc Control  Info Start-------------------\n", SuplayerId);
#if 1
    PROC_PRINT(p,
                    "send audio       :%u\n"
                    "send video       :%u\n"
                    "send subtite     :%u\n"
                    "dump audio       :%u\n"
                    "dump video       :%u\n"
                    "dump subtitle    :%u\n"
                    "Log level      :%u\n",
                    pSuplayer->ctrl.send_aud,
                    pSuplayer->ctrl.send_vid,
                    pSuplayer->ctrl.send_subt,
                    pSuplayer->ctrl.dump_aes,
                    pSuplayer->ctrl.dump_ves,
                    pSuplayer->ctrl.dump_ses,
                    pSuplayer->ctrl.log_level
                    );
#endif
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Proc Control  Info End-------------------\n", SuplayerId);
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Control  Info End-------------------\n", SuplayerId);

    return MT_SUCCESS;
}

static mt_s32 SUPLAYER_ProcWrite_Control(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    struct seq_file   *s = file->private_data;
    mt_proc_entry_t  *pProcItem = s->private;
    mt_u32            SuplayerId;
    mt_char           ProcPara[64]={0};
    mt_s32            Ret;
    mt_char           *pItemName = MT_NULL;
    mt_char           *pItemValue = MT_NULL;
    SUPLAYER_S          *pSuplayer = MT_NULL;

#if 1
    if (copy_from_user(ProcPara, buf, count))
    {
        return -EFAULT;
    }

    ProcPara[sizeof(ProcPara) - 1] = 0;
    Ret = SUPLAYER_ProcParsePara(ProcPara, &pItemName, &pItemValue);
    MT_FATAL_SUPLAYER("\n[%s_%d] name=%s,val=%s\n",__func__,__LINE__,pItemName,pItemValue);
    if (MT_SUCCESS != Ret)
    {
        SUPLAYER_ProcPrintHelp();
        return -EFAULT;
    }

    SuplayerId = (pProcItem->entry_name[8] - '0')*10 + (pProcItem->entry_name[9] - '0');

    if (SuplayerId >= SUPLAYER_MAX_NUM)
    {
        return -EFAULT;
    }

    pSuplayer = g_SuplayerGlobalState.SuplayerInfo[SuplayerId].pSuplayer;
    if (MT_NULL == pSuplayer)
    {
        return -EFAULT;
    }

    if (!mt_osal_strncmp(pItemName, "SendAud", strlen("SendAud")))
    {
        MT_FATAL_SUPLAYER("\n[%s_%d] send_aud=%d\n",__func__,__LINE__,pSuplayer->ctrl.send_aud);
        pSuplayer->ctrl.send_aud = simple_strtol(pItemValue, NULL, 10);
        MT_FATAL_SUPLAYER("\n[%s_%d] send_aud=%d\n",__func__,__LINE__,pSuplayer->ctrl.send_aud);
    }
    else if (0 == mt_osal_strncmp(pItemName, "SendVid", strlen("SendVid")))
    {
        pSuplayer->ctrl.send_vid = simple_strtol(pItemValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItemName, "SendSubt", strlen("SendSubt")))
    {
        pSuplayer->ctrl.send_subt = simple_strtol(pItemValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItemName, "DumpAes", strlen("DumpAes")))
    {
        pSuplayer->ctrl.dump_aes = simple_strtol(pItemValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItemName, "DumpVes", strlen("DumpVes")))
    {
        pSuplayer->ctrl.dump_ves = simple_strtol(pItemValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItemName, "DumpSes", strlen("DumpSes")))
    {
        pSuplayer->ctrl.dump_ses = simple_strtol(pItemValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItemName, "LogLv", strlen("LogLv")))
    {
        pSuplayer->ctrl.log_level = simple_strtol(pItemValue, NULL, 10);
    }
    else
    {
        SUPLAYER_ProcPrintHelp();
        return -EFAULT;
    }
#endif

    return count;
}

static mt_s32 SUPLAYER_ProcRead_Fileinfo(struct seq_file *p, mt_void *v)
{
    SUPLAYER_S          *pSuplayer = MT_NULL;
    mt_u32            SuplayerId;
    mt_proc_entry_t  *pProcItem;

    v = v;
    pProcItem = p->private;

    SuplayerId = (pProcItem->entry_name[8] - '0')*10 + (pProcItem->entry_name[9] - '0');
    
    pSuplayer = g_SuplayerGlobalState.SuplayerInfo[SuplayerId].pSuplayer;
    if (MT_NULL == pSuplayer)
    {
        SUPLAYER_ProcPrintHelp();
        return -EFAULT;
    }

    PROC_PRINT(p, "\n%s\n",__func__);
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Media  Info Start-------------------\n", SuplayerId);
	pSuplayer->file.vid_info.fourcc[4] = 0;
	pSuplayer->file.aud_info.fourcc[4] = 0;
	pSuplayer->file.aud_info.lang[4] = 0;
#if 1
    PROC_PRINT(p,
                    "Source Type       :%s\n"
                    "File Size             :%llu\n"
                    "Start Time          :%u:%u:%u\n"
                    "Duration             :%u:%u:%u\n"
                    "bps                    :%u KB/s\n"
                    "Program 0:\n"
                    "    Video 0 info:\n"
                    "        stream idx:  %u\n"
                    "        format:        %s\n"
                    "        codec:         %u\n"
                    "        w * h:          %u * %u\n"
                    "        fps:             %u\n"
                    "        bps:            %u KB/s\n"
                    "        duration:     %u:%u:%u\n"
                    "    Audio 0 info:\n"
                    "        stream idx:   %u\n"
                    "        stream cnt:   %u\n"
                    "        format:        %s\n"
                    "        codec:         0x%x\n"
                    "        samplerate:  %u Hz\n"
                    "        bitpersample:  %uBytes/sample\n"
                    "        channels:        %u\n"
                    "        bps:             %u bits/s\n"
                    "        lang:            %s\n"
                    "        subID:          %d\n"
                    "        duration:      %u:%u:%u\n",
                    (pSuplayer->file.src_type == SOURCE_TYPE_LOCAL) ? "LOCAL" : "NETWORK",
                    pSuplayer->file.filesize,
                    pSuplayer->file.starttime/3600,(pSuplayer->file.starttime/60)%60,pSuplayer->file.starttime%60,
                    pSuplayer->file.duration/3600,(pSuplayer->file.duration/60)%60,pSuplayer->file.duration%60,
                    pSuplayer->file.bps/1000,
                    pSuplayer->file.vid_info.stream_idx,
                    pSuplayer->file.vid_info.fourcc,
                    pSuplayer->file.vid_info.codec_id,
                    pSuplayer->file.vid_info.width,pSuplayer->file.vid_info.height,
                    pSuplayer->file.vid_info.fps,
                    pSuplayer->file.vid_info.bps/1024,
                    pSuplayer->file.vid_info.duration/3600,(pSuplayer->file.vid_info.duration/60)%60,pSuplayer->file.vid_info.duration%60,
                    pSuplayer->file.aud_info.stream_idx,
                    pSuplayer->file.aud_info.track_cnt,
                    pSuplayer->file.aud_info.fourcc,
                    pSuplayer->file.aud_info.codec_id,
                    pSuplayer->file.aud_info.samplerate,
                    pSuplayer->file.aud_info.bitpersample,
                    pSuplayer->file.aud_info.channels,
                    pSuplayer->file.aud_info.bps,
                    pSuplayer->file.aud_info.lang,
                    pSuplayer->file.aud_info.subID,
                    pSuplayer->file.aud_info.duration/3600,(pSuplayer->file.aud_info.duration/60)%60,pSuplayer->file.aud_info.duration%60
                    );
#endif
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Media  Info End-------------------\n", SuplayerId);

    return MT_SUCCESS;
}

static mt_s32 SUPLAYER_ProcWrite_Fileinfo(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    file =  file;
    buf = buf;
    count = count;
    ppos = ppos;
    MT_FATAL_SUPLAYER("\nDo not support write for this proc\n");
    return count;
}

static mt_s32 SUPLAYER_ProcRead_Format(struct seq_file *p, mt_void *v)
{
    SUPLAYER_S          *pSuplayer = MT_NULL;
    mt_u32            SuplayerId;
    mt_proc_entry_t  *pProcItem;

    v = v;
    pProcItem = p->private;

    SuplayerId = (pProcItem->entry_name[8] - '0')*10 + (pProcItem->entry_name[9] - '0');
    
    pSuplayer = g_SuplayerGlobalState.SuplayerInfo[SuplayerId].pSuplayer;
    if (MT_NULL == pSuplayer)
    {
        SUPLAYER_ProcPrintHelp();
        return -EFAULT;
    }

    PROC_PRINT(p, "\n%s\n",__func__);
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Format  Info Start-------------------\n", SuplayerId);
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Demux  Info Start-------------------\n", SuplayerId);
#if 1
    PROC_PRINT(p,
                    "FilePath       :%s\n"
                    "DemuName             :%s\n"
                    "SupportFormatName          :%s\n"
                    "SupportProtocolName             :%s\n",
                    pSuplayer->format.path,
                    pSuplayer->format.demuxer,
                    pSuplayer->format.support_format_name,
                    pSuplayer->format.support_protocol_name
                    );
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Demux  Info End-------------------\n", SuplayerId);
#endif
        PROC_PRINT(p,"----------------------Montage SUPLAYER%d Param  Info Start-------------------\n", SuplayerId);
#if 1
        PROC_PRINT(p,
                        "BufferConfigType       :%u\n"
                        "MaxBufferSize             :%u\n"
                        "TimeOutOfNetwork          :%u\n"
                        "BufferStart             :%u\n"
                        "BufferEnough          :%u\n"
                        "BufferFull             :%u\n"
                        "HlsStartMode          :%u\n"
                        "DstSubCodeType             :%u\n"
                        "Cookie          :%u\n"
                        "UserAgent             :%s\n",
                        pSuplayer->format.buf_config_type,
                        pSuplayer->format.max_buf_size,
                        pSuplayer->format.time_out_of_network,
                        pSuplayer->format.buf_start,
                        pSuplayer->format.buf_enough,
                        pSuplayer->format.buf_full,
                        pSuplayer->format.hls_start_mode,
                        pSuplayer->format.dst_subcode_type,
                        pSuplayer->format.cookie,
                        pSuplayer->format.user_agent
                        );
        PROC_PRINT(p,"----------------------Montage SUPLAYER%d Param  Info End-------------------\n", SuplayerId);
#endif
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Format  Info End-------------------\n", SuplayerId);

    return MT_SUCCESS;
}

static mt_s32 SUPLAYER_ProcWrite_Format(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    file =  file;
    buf = buf;
    count = count;
    ppos = ppos;
    MT_FATAL_SUPLAYER("\nDo not support write for this proc\n");
    return count;
}

static mt_s32 SUPLAYER_ProcRead_Adapter(struct seq_file *p, mt_void *v)
{
    SUPLAYER_S          *pSuplayer = MT_NULL;
    mt_u32            SuplayerId;
    mt_proc_entry_t  *pProcItem;

    v = v;
    pProcItem = p->private;

    SuplayerId = (pProcItem->entry_name[8] - '0')*10 + (pProcItem->entry_name[9] - '0');
    
    pSuplayer = g_SuplayerGlobalState.SuplayerInfo[SuplayerId].pSuplayer;
    if (MT_NULL == pSuplayer)
    {
        SUPLAYER_ProcPrintHelp();
        return -EFAULT;
    }

    PROC_PRINT(p, "\n%s\n",__func__);
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Adapter  Info Start-------------------\n", SuplayerId);
#if 0
    PROC_PRINT(p,
                    "AdapterStatus       :%s\n"
                    "bExtraAVPLAY             :%u\n"
                    "avplay handle          :0x%x\n"
                    "aud track handle             :0x%x\n"
                    "window handle          :0x%x\n"
                    "subtitle handle             :0x%x\n"
                    "aud mix height          :%u\n"
                    "aud port             :%u\n"
                    "display id          :%u\n"
                    "win-x             :%u\n"
                    "win-y          :%u\n"
                    "win-w             :%u\n"
                    "win-h          :%u\n"
                    "vdec error num             :%u\n"
                    "vid pts offset          :%u(ms)\n"
                    "aud pts offset             :%u(ms)\n"
                    "subt pts offset             :%u(ms)\n"
                    "Use ffmpeg aud codec          :%u\n"
                    "play speed             :%d\n"
                    "bvid begin          :%u\n"
                    "baud begin             :%u\n"
                    "cur pg id          :%u\n"
                    "cur vid id             :%u\n"
                    "cur aud id          :%u\n"
                    "cur subt id             :%u\n"
                    "aud frame send len          :%u\n"
                    "eos             :%u\n",
                    pSuplayer->adapter.status,
                    pSuplayer->adapter.bextra_avplay,
                    pSuplayer->adapter.avplay,
                    pSuplayer->adapter.aud_track_hdl,
                    pSuplayer->adapter.window_hdl,
                    pSuplayer->adapter.subo_hdl,
                    pSuplayer->adapter.aud_mix_heigh,
                    pSuplayer->adapter.aud_port,
                    pSuplayer->adapter.display_id,
                    pSuplayer->adapter.win_x,
                    pSuplayer->adapter.win_y,
                    pSuplayer->adapter.win_w,
                    pSuplayer->adapter.win_h,
                    pSuplayer->adapter.vdec_err_cover,
                    pSuplayer->adapter.vid_pts_offset,
                    pSuplayer->adapter.aud_pts_offset,
                    pSuplayer->adapter.subt_pts_offset,
                    pSuplayer->adapter.buse_ffmpeg_aud_codec,
                    pSuplayer->adapter.speed,
                    pSuplayer->adapter.bvid_start,
                    pSuplayer->adapter.baud_start,
                    pSuplayer->adapter.cur_pg_id,
                    pSuplayer->adapter.cur_vid_id,
                    pSuplayer->adapter.cur_aud_id,
                    pSuplayer->adapter.cur_subt_id,
                    pSuplayer->adapter.aud_frame_send_len,
                    pSuplayer->adapter.eos
                    );
#endif
    PROC_PRINT(p,"----------------------Montage SUPLAYER%d Adapter  Info End-------------------\n", SuplayerId);

    return MT_SUCCESS;
}


static mt_s32 SUPLAYER_ProcWrite_Adapter(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    file =  file;
    buf = buf;
    count = count;
    ppos = ppos;
    MT_FATAL_SUPLAYER("\nDo not support write for this proc\n");
    return count;
}

#define             PROC_CHAR_MAX (30)
mt_s32 SUPLAYER_Create(SUPLAYER_CREATE_S *pSuplayerCreate, struct file *file)
{
    mt_proc_entry_t  *pProcItem_control;
    mt_proc_entry_t  *pProcItem_fileinfo;
    mt_proc_entry_t  *pProcItem_format;
    mt_proc_entry_t  *pProcItem_adapter;
    mt_char           ProcNam_control[PROC_CHAR_MAX];
    mt_char           ProcName_fileinfo[PROC_CHAR_MAX];
    mt_char           ProcName_format[PROC_CHAR_MAX];
    mt_char           ProcName_adapter[PROC_CHAR_MAX];
    mt_s32            Ret;
    mt_u32            i = 0;
    mt_char           BufName[32];
    mmz_buffer_s      MemBuf;

    mt_osal_snprintf(BufName, sizeof(BufName), "SUPLAYER_Inst%02d", i);

	//Bug: 0x2000(8K) < sizeof(SUPLAYER_S)
    //Ret = mt_drv_mmz_alloc_and_map(BufName, MMZ_OTHERS, 0x2000, 0, &MemBuf);
    Ret = mt_drv_mmz_alloc_and_map(BufName, MMZ_OTHERS, sizeof(SUPLAYER_S), 0, &MemBuf);
    if (Ret != MT_SUCCESS)
    {
        MT_FATAL_SUPLAYER("malloc %s mmz failed.\n", BufName);

        return Ret;
    }
    memset((void*)MemBuf.startVirAddr, 0, sizeof(SUPLAYER_S));

    mt_osal_snprintf(ProcNam_control, sizeof(ProcNam_control), "%s%02d%s", MT_MOD_SUPLAYER, i,"_control");

    pProcItem_control = mt_drv_proc_add_module(ProcNam_control, MT_NULL, MT_NULL);
    if (!pProcItem_control)
    {
        MT_FATAL_SUPLAYER("add %s proc failed.\n", ProcNam_control);
        mt_drv_mmz_unmap_and_release(&MemBuf);
        return MT_FAILURE;
    }
    pProcItem_control->read = SUPLAYER_ProcRead_Control;
    pProcItem_control->write = SUPLAYER_ProcWrite_Control;

    mt_osal_snprintf(ProcName_fileinfo, sizeof(ProcName_fileinfo), "%s%02d%s", MT_MOD_SUPLAYER, i,"_fileinfo");

    pProcItem_fileinfo = mt_drv_proc_add_module(ProcName_fileinfo, MT_NULL, MT_NULL);
    if (!pProcItem_fileinfo)
    {
        MT_FATAL_SUPLAYER("add %s proc failed.\n", ProcName_fileinfo);
        mt_drv_mmz_unmap_and_release(&MemBuf);
        return MT_FAILURE;
    }
	pProcItem_fileinfo->read = SUPLAYER_ProcRead_Fileinfo;
	pProcItem_fileinfo->write = SUPLAYER_ProcWrite_Fileinfo;

    mt_osal_snprintf(ProcName_format, sizeof(ProcName_format), "%s%02d%s", MT_MOD_SUPLAYER, i,"_format");

    pProcItem_format = mt_drv_proc_add_module(ProcName_format, MT_NULL, MT_NULL);
    if (!pProcItem_format)
    {
        MT_FATAL_SUPLAYER("add %s proc failed.\n", ProcName_format);
        mt_drv_mmz_unmap_and_release(&MemBuf);
        return MT_FAILURE;
    }
    pProcItem_format->read = SUPLAYER_ProcRead_Format;
    pProcItem_format->write = SUPLAYER_ProcWrite_Format;

    mt_osal_snprintf(ProcName_adapter, sizeof(ProcName_adapter), "%s%02d%s", MT_MOD_SUPLAYER, i,"_adapter");

    pProcItem_adapter = mt_drv_proc_add_module(ProcName_adapter, MT_NULL, MT_NULL);
    if (!pProcItem_adapter)
    {
        MT_FATAL_SUPLAYER("add %s proc failed.\n", ProcName_adapter);
        mt_drv_mmz_unmap_and_release(&MemBuf);
        return MT_FAILURE;
    }
    pProcItem_adapter->read = SUPLAYER_ProcRead_Adapter;
    pProcItem_adapter->write = SUPLAYER_ProcWrite_Adapter;

    g_SuplayerGlobalState.SuplayerInfo[i].pSuplayer = (SUPLAYER_S *)MemBuf.startVirAddr;
    g_SuplayerGlobalState.SuplayerInfo[i].SuplayerPhyAddr = MemBuf.startPhyAddr;
    g_SuplayerGlobalState.SuplayerInfo[i].File = (ulong)file;

    pSuplayerCreate->SuplayerId = i;
    pSuplayerCreate->SuplayerPhyAddr = MemBuf.startPhyAddr;
    MT_FATAL_SUPLAYER("sup_id=%x.\n", pSuplayerCreate->SuplayerId);
    g_SuplayerGlobalState.SuplayerCount++;

    return MT_SUCCESS;
}

mt_s32 SUPLAYER_Destroy(mt_u32 SuplayerId)
{
    mt_char           ProcNam_control[PROC_CHAR_MAX];
    mt_char           ProcName_fileinfo[PROC_CHAR_MAX];
    mt_char           ProcName_format[PROC_CHAR_MAX];
    mt_char           ProcName_adapter[PROC_CHAR_MAX];
    mmz_buffer_s      MemBuf;

    if (MT_NULL == g_SuplayerGlobalState.SuplayerInfo[SuplayerId].pSuplayer)
    {
        MT_ERR_SUPLAYER("this is invalid handle.\n");
        return MT_ERR_SUPLAYER_DESTROY_ERR;
    }
    memset(ProcNam_control, 0, sizeof(ProcNam_control));

    mt_osal_snprintf(ProcNam_control, sizeof(ProcNam_control), "%s%02d%s", MT_MOD_SUPLAYER, SuplayerId,"_control");
    mt_drv_proc_rm_module(ProcNam_control);//MT_DRV_PROC_RemoveModule


    memset(ProcName_fileinfo, 0, sizeof(ProcName_fileinfo));
    mt_osal_snprintf(ProcName_fileinfo, sizeof(ProcName_fileinfo), "%s%02d%s", MT_MOD_SUPLAYER, SuplayerId,"_fileinfo");
    mt_drv_proc_rm_module(ProcName_fileinfo);//MT_DRV_PROC_RemoveModule

    memset(ProcName_format, 0, sizeof(ProcName_format));
    mt_osal_snprintf(ProcName_format, sizeof(ProcName_format), "%s%02d%s", MT_MOD_SUPLAYER, SuplayerId,"_format");
    mt_drv_proc_rm_module(ProcName_format);//MT_DRV_PROC_RemoveModule

    memset(ProcName_adapter, 0, sizeof(ProcName_adapter));
    mt_osal_snprintf(ProcName_adapter, sizeof(ProcName_adapter), "%s%02d%s", MT_MOD_SUPLAYER, SuplayerId,"_adapter");
    mt_drv_proc_rm_module(ProcName_adapter);//MT_DRV_PROC_RemoveModule

    MemBuf.startVirAddr = (void *)g_SuplayerGlobalState.SuplayerInfo[SuplayerId].pSuplayer;
    MemBuf.startPhyAddr = g_SuplayerGlobalState.SuplayerInfo[SuplayerId].SuplayerPhyAddr;
    MemBuf.size = sizeof(SUPLAYER_S);

    mt_drv_mmz_unmap_and_release(&MemBuf);
    
    g_SuplayerGlobalState.SuplayerInfo[SuplayerId].pSuplayer = MT_NULL;
    g_SuplayerGlobalState.SuplayerInfo[SuplayerId].SuplayerPhyAddr = MT_NULL;
    g_SuplayerGlobalState.SuplayerInfo[SuplayerId].File = MT_NULL;
    g_SuplayerGlobalState.SuplayerInfo[SuplayerId].SuplayerUsrAddr = MT_NULL;

    g_SuplayerGlobalState.SuplayerCount--;
    return MT_SUCCESS;
}

mt_s32 SUPLAYER_SetUsrAddr(SUPLAYER_USR_ADDR_S *pSuplayerUsrAddr)
{
    g_SuplayerGlobalState.SuplayerInfo[pSuplayerUsrAddr->SuplayerId].SuplayerUsrAddr = (ulong)(pSuplayerUsrAddr->SuplayerUsrAddr);
    return MT_SUCCESS;
}

mt_s32 SUPLAYER_CheckId(SUPLAYER_USR_ADDR_S *pSuplayerUsrAddr, struct file *file)
{
    if (g_SuplayerGlobalState.SuplayerInfo[pSuplayerUsrAddr->SuplayerId].File != ((ulong)file))
    {
        MT_ERR_SUPLAYER("this is invalid handle:%d.\n",__LINE__);
        //return MT_ERR_SUPLAYER_INVALID_PARA;
    }

    pSuplayerUsrAddr->SuplayerUsrAddr = g_SuplayerGlobalState.SuplayerInfo[pSuplayerUsrAddr->SuplayerId].SuplayerUsrAddr;
	

    return MT_SUCCESS;
}

mt_s32 SUPLAYER_Ioctl(struct inode *inode, struct file *file, unsigned int cmd, mt_void *arg)
{
    mt_s32           Ret;

    Ret = down_interruptible(&g_SuplayerMutex);

    switch (cmd)
    {
        case CMD_SUPLAYER_CREATE:
        {
            SUPLAYER_CREATE_S  *pSuplayerCreate;

            pSuplayerCreate = (SUPLAYER_CREATE_S *)arg;

            Ret = SUPLAYER_Create(pSuplayerCreate, file);

            break;
        }

        case CMD_SUPLAYER_DESTROY:
        {
            Ret = SUPLAYER_Destroy(*((mt_u32 *)arg));

            break;
        }
        
        case CMD_SUPLAYER_SET_USRADDR:
        {
            SUPLAYER_USR_ADDR_S *pSuplayerUsrAddr;

            pSuplayerUsrAddr = (SUPLAYER_USR_ADDR_S *)arg;

            Ret = SUPLAYER_SetUsrAddr(pSuplayerUsrAddr);
            //Ret = SUPLAYER_SetUsrAddr(pSuplayerUsrAddr);

            break;
        }

        case CMD_SUPLAYER_CHECK_ID:
        {
            SUPLAYER_USR_ADDR_S *pSuplayerUsrAddr;
            
            pSuplayerUsrAddr = (SUPLAYER_USR_ADDR_S *)arg;
            
            Ret = SUPLAYER_CheckId(pSuplayerUsrAddr, file);
            
            break;
        }

        default:
            up(&g_SuplayerMutex);
            return -ENOIOCTLCMD;
    }

    up(&g_SuplayerMutex);
    return Ret;
}

static mt_s32 SUPLAYER_DRV_Open(struct inode *finode, struct file  *ffile)
{
    return 0;
}

static mt_s32 SUPLAYER_DRV_Close(struct inode *finode, struct file  *ffile)
{
    mt_s32           i;
    mt_s32           Ret;

    Ret = down_interruptible(&g_SuplayerMutex);

    for (i=0; i<SUPLAYER_MAX_NUM; i++)
    {
        if (g_SuplayerGlobalState.SuplayerInfo[i].File == ((ulong)ffile))
        {
            Ret = SUPLAYER_Destroy(i);
            if (Ret != MT_SUCCESS)
            {
                up(&g_SuplayerMutex);
                return -1;
            }
        }
    }

    if (atomic_dec_and_test(&g_SuplayerCount))
    {
    }

    up(&g_SuplayerMutex);

    return 0;
}

static long SUPLAYER_DRV_Ioctl(struct file *ffile, unsigned int cmd, unsigned long arg)
{
    mt_s32 Ret;

    Ret = mt_drv_usercopy(ffile->f_path.dentry->d_inode, ffile, cmd, arg, SUPLAYER_Ioctl);

    return Ret;
}

static struct file_operations SUPLAYER_FOPS =
{
    .owner          =  THIS_MODULE,
    .open           =  SUPLAYER_DRV_Open,
    .unlocked_ioctl =  SUPLAYER_DRV_Ioctl,
    .release        =  SUPLAYER_DRV_Close,
};

static baseops_s SUPLAYER_DRVOPS = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = NULL,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = NULL,
};

mt_s32 __init SUPLAYER_DRV_ModInit(mt_void)
{
    mt_u32      i;
    mt_s32      Ret;

    MT_INFO_SUPLAYER("SUPLAYER_DRV_ModInit int\n");

    Ret = mt_drv_module_register(MT_ID_SUPLAYER, SUPLAYER_NAME, MT_NULL);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_SUPLAYER("ERR: MT_DRV_MODULE_Register, Ret = %#x!\n", Ret);
    }

    g_SuplayerGlobalState.SuplayerCount = 0;
    for (i=0; i<SUPLAYER_MAX_NUM; i++)
    {
        g_SuplayerGlobalState.SuplayerInfo[i].pSuplayer = MT_NULL;
        g_SuplayerGlobalState.SuplayerInfo[i].SuplayerPhyAddr = MT_NULL;
        g_SuplayerGlobalState.SuplayerInfo[i].File = MT_NULL;
        g_SuplayerGlobalState.SuplayerInfo[i].SuplayerUsrAddr = MT_NULL;
    }

    mt_osal_snprintf(g_SuplayerRegisterData.devfs_name, sizeof(g_SuplayerRegisterData.devfs_name), UMAP_DEVNAME_SUPLAYER);
    g_SuplayerRegisterData.fops = &SUPLAYER_FOPS;
    g_SuplayerRegisterData.minor = UMAP_MIN_MINOR_SUPLAYER;
    g_SuplayerRegisterData.owner  = THIS_MODULE;
    g_SuplayerRegisterData.drvops = &SUPLAYER_DRVOPS;

    if (mt_drv_dev_register(&g_SuplayerRegisterData) < 0)
    {
        MT_ERR_SUPLAYER("register SUPLAYER failed.\n");
        return MT_FAILURE;
    }

    MT_INFO_SUPLAYER("SUPLAYER_DRV_ModInit exit \n");

    return  0;
}

mt_void __exit SUPLAYER_DRV_ModExit(mt_void)
{
    mt_drv_dev_unregister(&g_SuplayerRegisterData);

    mt_drv_module_unregister(MT_ID_SUPLAYER);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
