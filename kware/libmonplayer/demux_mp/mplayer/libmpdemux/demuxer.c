/*
 * DEMUXER v2.5
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#define MODULE_TAG "DX MP"
#ifdef __LINUX__
#include <stdio.h>
#include <stdlib.h>
#else
#include "mp_func_trans.h"
#endif

#include <unistd.h>
#ifndef __ANDRIOD__
#include <sys/types.h>
#include <sys/stat.h>
#include "mt_type.h"
#include "sys_define.h"
#else
#include "mt_type.h"
#endif
#include "config.h"
#include "mp_msg.h"
#include "help_mp.h"
#include "m_config.h"
#include "mpcommon.h"

#include "libvo/fastmemcpy.h"

#include "stream/stream.h"
#include "demuxer.h"
#include "stheader.h"
#include "mf.h"
#include "demux_audio.h"

#include "libaf/af_format.h"
#include "libmpcodecs/dec_audio.h"
#include "libmpcodecs/dec_video.h"
#include "libmpcodecs/dec_teletext.h"
#include "sub/ass_mp.h"
#include "mutil.h"
#include "mlog.h"

#ifdef CONFIG_FFMPEG_MP
#include "libavcodec/avcodec.h"
#if MP_INPUT_BUFFER_PADDING_SIZE < FF_INPUT_BUFFER_PADDING_SIZE
#error MP_INPUT_BUFFER_PADDING_SIZE is too small!
#endif
#include "av_helpers.h"
#endif
#include "libavutil/avstring.h"
#ifndef __ANDRIOD__
#include "sys_define.h"
#endif
#include "libavformat/avformat.h"
#include "file_playback_sequence.h"
#ifdef CFG_ENABLE_FFMPEG_422
#include "ffmpeg2mp_adapter.h"
#endif
#include "mtos_printk.h"
#include "file_seq_internal.h"

#ifdef DRM_SMP_ENABLE
#include "MTDrmApi.h"
#endif
// This is quite experimental, in particular it will mess up the pts values
// in the queue - on the other hand it might fix some issues like generating
// broken files with mencoder and stream copy.
// Better leave it disabled for now, if we find no use for it this code should
// just be removed again.
#define PARSE_ON_ADD 0
//#define OLD_CODE 1
#if 0 // defined(DEBUG_FILE_PLAYBACK)   //peacer add
extern int flag_debug;
#endif
extern int while_cnt;
extern int io_isnetworkstream;
extern int g_is_live_broadcast;
//for debug avi_desc
#ifdef CFG_ENABLE_FFMPEG_422
static inline void ff_av_pkt_free(AVPacket **pkt)
{
    av_packet_free(pkt);
}

void dp_ff_av_pkt_alloc(demux_packet_t *dp)
{
    if (dp->ff_av_pkt) {
        ff_av_pkt_free((AVPacket **) &dp->ff_av_pkt);
    }
    dp->ff_av_pkt = av_packet_alloc();
}
#else
static inline void ff_av_pkt_free(AVPacket **pkt)
{
    av_free_packet(*pkt);
    av_free(*pkt);
    *pkt = NULL;
}

void dp_ff_av_pkt_alloc(demux_packet_t *dp)
{
    if (dp->ff_av_pkt) {
        ff_av_pkt_free((AVPacket **) &dp->ff_av_pkt);
    }
    dp->ff_av_pkt = av_mallocz(sizeof(AVPacket));
    if (dp->ff_av_pkt) {
        av_init_packet(dp->ff_av_pkt);
    }
}
#endif
/* Buffer including drm buffer will be release safely */
void dp_ff_av_pkt_free(demux_packet_t *dp)
{
    if (!dp || !dp->ff_av_pkt) {
        return;
    }
    ff_av_pkt_free((AVPacket **) &dp->ff_av_pkt);
    /* dp buffer is AVPacket->data, alread free */
    dp->buffer = NULL;
}

void free_demux_packet(demux_packet_t *dp)
{
    if (dp == NULL) {
        return;
    }
    dp_ff_av_pkt_free(dp);
    if (dp->buffer) {
#if defined(DRM_SMP_ENABLE)
        DMTRM_BUFFER_OUT tmpbuf;
        /* Check and free secure memory */
        if (dp->is_secure) {
            tmpbuf.data = dp->buffer;
            tmpbuf.is_secure = 1;
            MTDrm_FreeSecureMemory(&tmpbuf);
        } else {
            free33(dp->buffer);
        }

#elif defined(VMX_OTT_SVP)
        extern int MT_VMX_OTT_DASH_Flag();
        extern int MT_VMX_OTT_Free(void *free);

        if (MT_VMX_OTT_DASH_Flag()) {
            MT_VMX_OTT_Free(dp->buffer);
        } else {
            free33(dp->buffer);
        }
#else
        free33(dp->buffer);
#endif
        dp->buffer = NULL;
    }
    free33(dp);
    return;
}

int resize_demux_packet(demux_packet_t *dp, int len)
{
    if (len > 0) {
        unsigned char* p_buffer = realloc33(dp->buffer, len + MP_INPUT_BUFFER_PADDING_SIZE);
        if (p_buffer == NULL) {
            return -1;
        }
        dp->buffer = (unsigned char *)p_buffer;
    } else {
        free33(dp->buffer);
        dp->buffer = NULL;
    }
    dp->len = len;
    if (dp->buffer) {
        memset(dp->buffer + len, 0, MP_INPUT_BUFFER_PADDING_SIZE);
    } else {
        dp->len = 0;
    }
    return 0;
}

demux_packet_t *new_demux_packet(int len)
{
    demux_packet_t *dp = (demux_packet_t *)malloc33(sizeof(demux_packet_t));

    if (dp == NULL) {
        return NULL;
    }

    dp->len    = len;
    dp->next   = NULL;
    dp->pts    = MP_NOPTS_VALUE;
    dp->dts    = MP_NOPTS_VALUE;
    dp->endpts = MP_NOPTS_VALUE;
    dp->stream_pts = MP_NOPTS_VALUE;
    dp->pos = 0;
    dp->flags = 0;
    //dp->refcount = 1;
    //dp->master = NULL;
    dp->buffer = NULL;
    dp->is_secure = 0;
    dp->ff_av_pkt = NULL;
    dp->sample_size = 0;
    if (len > 0 && (dp->buffer = (unsigned char *)malloc33(len + MP_INPUT_BUFFER_PADDING_SIZE))) {
        memset(dp->buffer + len, 0, MP_INPUT_BUFFER_PADDING_SIZE);
    } else if (len) {
        // do not even return a valid packet if allocation failed
        free33(dp);
        return NULL;
    }
    return dp;
}

void *realloc_struct(void *ptr, size_t nmemb, size_t size)
{
    if (nmemb > SIZE_MAX / size) {
        free33(ptr);
        return NULL;
    }
    return realloc33(ptr, nmemb * size);
}

static void clear_parser(sh_common_t *sh);

// Demuxer list
#ifdef NEW_AVI
extern const demuxer_desc_t demuxer_desc_avi;
#endif
extern const demuxer_desc_t demuxer_desc_asf;
extern const demuxer_desc_t demuxer_desc_matroska;
extern const demuxer_desc_t demuxer_desc_mov;
extern const demuxer_desc_t demuxer_desc_mpeg_ts;
extern const demuxer_desc_t demuxer_desc_mpeg_ps;
extern const demuxer_desc_t demuxer_desc_mpeg4_es;
extern const demuxer_desc_t demuxer_desc_audio;
#if defined(ENABLE_DEMUX_RTSP)
extern const demuxer_desc_t demuxer_desc_rtp;
#endif
extern const demuxer_desc_t demuxer_desc_lavf;

/* Please do not add any new demuxers here. If you want to implement a new
 * demuxer, add it to libavformat, except for wrappers around external
 * libraries and demuxers requiring binary support. */

const demuxer_desc_t *const demuxer_list[] = {
    &demuxer_desc_mpeg_ts,
    &demuxer_desc_lavf,
    &demuxer_desc_mpeg_ps,
    &demuxer_desc_mpeg4_es,
    &demuxer_desc_audio,
#ifdef CONFIG_LIVE555

#if defined(ENABLE_DEMUX_RTSP)
    &demuxer_desc_rtp,
#endif

#endif
    /* Please do not add any new demuxers here. If you want to implement a new
     * demuxer, add it to libavformat, except for wrappers around external
     * libraries and demuxers requiring binary support. */
    NULL
};

void free_demuxer_stream(demux_stream_t *ds)
{
    if (NULL == ds) {
        return;
    }
    ds_free_packs(ds);
    free33(ds->trickbuf);
    free33(ds);
}

demux_stream_t *new_demuxer_stream(struct demuxer *demuxer, int id)
{
    demux_stream_t *ds = calloc33(1, sizeof(demux_stream_t));
    *ds = (demux_stream_t) {
        .id = id,
        .demuxer = demuxer,
        .asf_seq = -1,
        .discard = 0,
    };
    return ds;
}

/**
 * Get demuxer description structure for a given demuxer type
 *
 * @param file_format    type of the demuxer
 * @return               structure for the demuxer, NULL if not found
 */
static const demuxer_desc_t *get_demuxer_desc_from_type(int file_format)
{
    int i;

    for (i = 0; demuxer_list[i]; i++)
        if (file_format == demuxer_list[i]->type) {
            return demuxer_list[i];
        }

    return NULL;
}

demuxer_t *alloc_demuxer(stream_t *stream, int type, const char *filename)
{
    demuxer_t *d = calloc33(1, sizeof(*d));
    d->stream = stream;
    d->stream_pts = MP_NOPTS_VALUE;
    d->reference_clock = MP_NOPTS_VALUE;
    d->movi_start = stream->start_pos;
    d->movi_end = stream->end_pos;
    d->seekable = 1;
    d->synced = 0;
    d->filepos = -1;
    d->type = type;
    if (type)
        if (!(d->desc = get_demuxer_desc_from_type(type)))
            mp_msg(MSGT_DEMUXER, MSGL_ERR,
                   "BUG! Invalid demuxer type in new_demuxer(), "
                   "big troubles ahead.\n");
    if (filename) {
        d->filename = strdup33(filename);
    }
    d->callback_ctx = NULL;
    d->interrupt_callback = NULL;
    d->app_message_callback = NULL;
    d->adaptive_playlist = NULL;
    d->idx_adaptive_playlist = 0;
    d->num_adaptive_playlist = 0;
    return d;
}

demuxer_t *new_demuxer(stream_t *stream, int type, int a_id, int v_id,
                       int s_id, char *filename)
{
    MLOGD("[%s]---start, type = %d\n", __func__, type);
    demuxer_t *d = alloc_demuxer(stream, type, filename);
    d->audio = new_demuxer_stream(d, a_id);
    d->video = new_demuxer_stream(d, v_id);
    d->sub = new_demuxer_stream(d, s_id);
    stream->eof = 0;
    stream_seek(stream, stream->start_pos);
    d->type_adaptive_stream = TYPE_ADAPTIVE_STREAM_NONE;
    d->callback_ctx         = stream->callback_ctx;
    d->app_message_callback = stream->app_message_callback;
    d->interrupt_callback   = stream->interrupt_callback;
    MLOGD("[%s]---end\n", __func__);
    return d;
}

const char *sh_sub_type2str(int type)
{
    switch (type) {
        case 't':
            return "text";
        case 'm':
            return "movtext";
        case 'a':
            return "ass";
        case 'v':
            return "vobsub";
        case 'x':
            return "xsub";
        case 'b':
            return "dvb";
        case 'd':
            return "dvb-teletext";
        case 'p':
            return "hdmv pgs";
    }
    return "unknown";
}

static void init_sh_comm(sh_common_t *sh)
{
    sh->codec_extradata = NULL;
    sh->codec_extradata_size = 0;
}

static void deinit_sh_comm(sh_common_t *sh)
{
    SAFEFREE(sh->codec_extradata);
    sh->codec_extradata_size = 0;
    clear_parser((sh_common_t *)sh);
}

sh_sub_t *new_sh_sub_sid(demuxer_t *demuxer, int id, int sid, const char *lang)
{
    if (id > MAX_S_STREAMS - 1 || id < 0) {
        mp_msg(MSGT_DEMUXER, MSGL_WARN,
               "Requested sub stream id overflow (%d > %d)\n", id,
               MAX_S_STREAMS);
        return NULL;
    }
    if (demuxer->s_streams[id]) {
        mp_msg(MSGT_DEMUXER, MSGL_WARN, "Sub stream %i redefined\n", id);
    } else {
        sh_sub_t *sh = calloc33(1, sizeof(sh_sub_t));
        demuxer->s_streams[id] = sh;
        sh->sid = sid;
        mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_SUBTITLE_ID=%d\n", sid);
        if (lang && lang[0] && strcmp(lang, "und")) {
            sh->lang = strdup33(lang);
            mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_SID_%d_LANG=%s\n", sid, lang);
        }
        if (sid == dvdsub_id) {
            demuxer->sub->id = id;
            demuxer->sub->sh = demuxer->s_streams[id];
        }
        init_sh_comm((sh_common_t *)sh);
    }
    return demuxer->s_streams[id];
}

static void free_sh_sub(sh_sub_t *sh)
{
    mp_msg(MSGT_DEMUXER, MSGL_DBG2, "DEMUXER: freeing sh_sub at %p\n", sh);
    deinit_sh_comm((sh_common_t *)sh);
#ifdef CONFIG_ASS
    if (sh->ass_track) {
        ass_free_track(sh->ass_track);
    }
#endif
    free33(sh->lang);
    free33(sh);
}

sh_audio_t *new_sh_audio_aid(demuxer_t *demuxer, int id, int aid, const char *lang)
{
    if (id > MAX_A_STREAMS - 1 || id < 0) {
        mp_msg(MSGT_DEMUXER, MSGL_WARN,
               "Requested audio stream id overflow (%d > %d)\n", id,
               MAX_A_STREAMS);
        return NULL;
    }
    if (demuxer->a_streams[id]) {
        mp_msg(MSGT_DEMUXER, MSGL_WARN, MSGTR_AudioStreamRedefined, id);
    } else {
        sh_audio_t *sh = calloc33(1, sizeof(sh_audio_t));
        mp_msg(MSGT_DEMUXER, MSGL_V, "==> Found audio stream: %d\n", id);
        demuxer->a_streams[id] = sh;
        sh->aid = aid;
        sh->ds = demuxer->audio;
        // set some defaults
        sh->samplesize = 2;
        sh->sample_format = AF_FORMAT_S16_NE;
        sh->audio_out_minsize = 8192; /* default size, maybe not enough for Win32/ACM */
        sh->pts = MP_NOPTS_VALUE;
        sh->hAvCtx = NULL;
        mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_AUDIO_ID=%d\n", aid);
        if (lang && lang[0] && strcmp(lang, "und")) {
            sh->lang = strdup33(lang);
            mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_AID_%d_LANG=%s\n", aid, lang);
        }
        if (aid == audio_id) {
            demuxer->audio->id = id;
            demuxer->audio->sh = demuxer->a_streams[id];
        }
        init_sh_comm((sh_common_t *)sh);
    }
    return demuxer->a_streams[id];
}

void free_sh_audio(demuxer_t *demuxer, int id)
{
    sh_audio_t *sh = demuxer->a_streams[id];
    demuxer->a_streams[id] = NULL;
    mp_msg(MSGT_DEMUXER, MSGL_DBG2, "DEMUXER: freeing sh_audio at %p\n", sh);

    deinit_sh_comm((sh_common_t *)sh);
    free33(sh->wf);
    free33(sh->lang);
    if (sh->title) {
        free33(sh->title);
    }
    free33(sh);
}

sh_video_t *new_sh_video_vid(demuxer_t *demuxer, int id, int vid)
{
    if (id > MAX_V_STREAMS - 1 || id < 0) {
        mp_msg(MSGT_DEMUXER, MSGL_WARN,
               "Requested video stream id overflow (%d > %d)\n", id,
               MAX_V_STREAMS);
        return NULL;
    }
    if (demuxer->v_streams[id]) {
        mp_msg(MSGT_DEMUXER, MSGL_WARN, MSGTR_VideoStreamRedefined, id);
    } else {
        sh_video_t *sh = calloc33(1, sizeof(sh_video_t));
        mp_msg(MSGT_DEMUXER, MSGL_V, "==> Found video stream: %d\n", id);
        demuxer->v_streams[id] = sh;
        sh->vid = vid;
        sh->ds = demuxer->video;
        mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_VIDEO_ID=%d\n", vid);
        if (vid == video_id) {
            demuxer->video->id = id;
            demuxer->video->sh = demuxer->v_streams[id];
        }
        init_sh_comm((sh_common_t *)sh);
    }
    return demuxer->v_streams[id];
}

void free_sh_video(sh_video_t *sh)
{
    mp_msg(MSGT_DEMUXER, MSGL_DBG2, "DEMUXER: freeing sh_video at %p\n", sh);

    deinit_sh_comm((sh_common_t *)sh);
    free33(sh->bih);

    free33(sh);
}

/* open sequence: demuxer_desc -> demuxer_t -> a/v/s demux stream */
void free_demuxer(demuxer_t *demuxer)
{
    int i;

    if (!demuxer || !(demuxer->desc)) {
        return;
    }

    MLOGD("DEMUXER: free demuxer\n");
    if (demuxer->desc->close) {
        demuxer->desc->close(demuxer);
    }

    // Very ugly hack to make it behave like old implementation
    if (demuxer->desc->type == DEMUXER_TYPE_DEMUXERS) {
        goto skip_streamfree;
    }
    // free streams:
    for (i = 0; i < MAX_A_STREAMS; i++) {
        if (demuxer->a_streams[i]) {
            free_sh_audio(demuxer, i);
        }
    }
    for (i = 0; i < MAX_V_STREAMS; i++) {
        if (demuxer->v_streams[i]) {
            free_sh_video(demuxer->v_streams[i]);
        }
    }
    for (i = 0; i < MAX_S_STREAMS; i++) {
        if (demuxer->s_streams[i]) {
            free_sh_sub(demuxer->s_streams[i]);
        }
    }

    // free demuxers:
    free_demuxer_stream(demuxer->audio);
    free_demuxer_stream(demuxer->video);
    free_demuxer_stream(demuxer->sub);
skip_streamfree:
    if (demuxer->info) {
        for (i = 0; demuxer->info[i] != NULL; i++) {
            free33(demuxer->info[i]);
        }
        free33(demuxer->info);
    }
    free33(demuxer->filename);
    if (demuxer->chapters) {
        for (i = 0; i < demuxer->num_chapters; i++) {
            free33(demuxer->chapters[i].name);
        }
        free33(demuxer->chapters);
    }
    if (demuxer->attachments) {
        for (i = 0; i < demuxer->num_attachments; i++) {
            free33(demuxer->attachments[i].name);
            free33(demuxer->attachments[i].type);
            free33(demuxer->attachments[i].data);
        }
        free33(demuxer->attachments);
    }
    if (demuxer->teletext) {
        teletext_control(demuxer->teletext, TV_VBI_CONTROL_STOP, NULL);
    }
    if (demuxer->adaptive_playlist) {
        free33(demuxer->adaptive_playlist);
        demuxer->adaptive_playlist = NULL;
    }
    free33(demuxer);
}

#if OLD_CODE
static int x_check_get_bits(unsigned char buffer[], int totbitoffset, int *info, int bytecount, int numbits)
{
    int inf = 0;
    int byteoffset = 0; // byte from start of buffer
    int bitoffset = 0;  // bit from start of byte

    int bitcounter = numbits;

    byteoffset = totbitoffset / 8;
    bitoffset = 7 - (totbitoffset % 8);

    while (numbits) {
        inf <<= 1;
        inf |= (buffer[byteoffset] & (0x01 << bitoffset)) >> bitoffset;
        numbits--;
        bitoffset--;
        if (bitoffset < 0) {
            byteoffset++;
            bitoffset += 8;
            if (byteoffset > bytecount) {
                return -1;
            }
        }
    }

    *info = inf;
    return bitcounter; // return absolute offset in bit from start of frame
}
#endif
static int x_check_get_ue(unsigned char buffer[], int totbitoffset, int *info, int bytecount)
{
    int inf = 0;
    long byteoffset = 0; // byte from start of buffer
    int bitoffset = 0;   // bit from start of byte
    int ctr_bit = 0;     // control bit for current bit posision
    int bitcounter = 1;
    int len = 1;
    int info_bit = 0;

    byteoffset = totbitoffset >> 3;
    bitoffset = 7 - (totbitoffset & 7);
    ctr_bit = (buffer[byteoffset] & (0x01 << bitoffset)); // set up control bit

    while (ctr_bit == 0) {
        // find leading 1 bit
        len++;
        bitoffset -= 1;
        bitcounter++;
        if (bitoffset < 0) {
            // finish with current byte ?
            bitoffset = bitoffset + 8;
            byteoffset++;
        }
        ctr_bit = buffer[byteoffset] & (0x01 << (bitoffset));
    }
    // make infoword
    inf = 0; // shortest possible code is 1, then info is always 0
    for (info_bit = 0; info_bit < (len - 1); info_bit++) {
        bitcounter++;
        bitoffset -= 1;
        if (bitoffset < 0) {
            // finished with current byte ?
            bitoffset = bitoffset + 8;
            byteoffset++;
        }
        if (byteoffset > bytecount) {
            return -1;
        }
        inf = (inf << 1);
        if (buffer[byteoffset] & (0x01 << bitoffset)) {
            inf |= 1;
        }
    }
    *info = (int)(1 << (bitcounter >> 1)) + inf - 1;

    return bitcounter;
}
#if OLD_CODE
static int x_check_get_se(unsigned char buffer[], int totbitoffset, int *info, int bytecount)
{
    int inf = 0;
    long byteoffset = 0; // byte from start of buffer
    int bitoffset = 0;   // bit from start of byte
    int ctr_bit = 0;     // control bit for current bit posision
    int bitcounter = 1;
    int len = 1;
    int info_bit = 0;

    byteoffset = totbitoffset >> 3;
    bitoffset = 7 - (totbitoffset & 7);
    ctr_bit = (buffer[byteoffset] & (0x01 << bitoffset)); // set up control bit

    while (ctr_bit == 0) { // find leading 1 bit
        len++;
        bitoffset -= 1;
        bitcounter++;
        if (bitoffset < 0) { // finish with current byte ?
            bitoffset = bitoffset + 8;
            byteoffset++;
        }
        ctr_bit = buffer[byteoffset] & (0x01 << (bitoffset));
    }
    // make infoword
    inf = 0; // shortest possible code is 1, then info is always 0
    for (info_bit = 0; (info_bit < (len - 1)); info_bit++) {
        bitcounter++;
        bitoffset -= 1;
        if (bitoffset < 0) {
            // finished with current byte ?
            bitoffset = bitoffset + 8;
            byteoffset++;
        }
        if (byteoffset > bytecount) {
            return -1;
        }
        inf = (inf << 1);
        if (buffer[byteoffset] & (0x01 << bitoffset)) {
            inf |= 1;
        }
    }
    inf = (int)(1 << (bitcounter >> 1)) + inf - 1;
    *info = (inf + 1) / 2;
    if ((inf & 0x01) == 0) { // lsb is signed bit
        *info = -*info;
    }
    return bitcounter;
}
static char x_check_ZZ_SCAN[16] =
    { 0, 1, 4, 8, 5, 2, 3, 6, 9, 12, 13, 10, 7, 11, 14, 15 };

static char x_check_ZZ_SCAN8[64] = {
    0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};
static int x_check_ScalingList4x4[6][16];
static int x_check_ScalingList8x8[2][64];
static char x_check_UseDefaultScalingMatrix4x4Flag[6];
static char x_check_UseDefaultScalingMatrix8x8Flag[2];
static int x_bitoffset = 0;
static int x_check_log2maxframe = 0;

static int EBSPtoRBSP(unsigned char *streamBuffer, int end_bytepos, int begin_bytepos)
{
    int i = 0;
    int j = 0;
    int count = 0;

    if (end_bytepos < begin_bytepos) {
        return end_bytepos;
    }

    j = begin_bytepos;

    for (i = begin_bytepos; i < end_bytepos; i++) { //starting from begin_bytepos to avoid header information
        //in NAL unit, 0x000000, 0x000001 or 0x000002 shall not occur at any byte-aligned position
        if (count == 2 && streamBuffer[i] < 0x03) {
            return -1;
        }

        if (count == 2 && streamBuffer[i] == 0x03) {
            if ((i < end_bytepos - 1) && (streamBuffer[i + 1] > 0x03)) {
                return -1;
            }

            if (i == end_bytepos - 1) {
                return j;
            }

            i++;
            count = 0;
        }

        streamBuffer[j] = streamBuffer[i];

        if (streamBuffer[i] == 0x00) {
            count++;
        } else {
            count = 0;
        }

        j++;
    }

    return j;
}
static void x_check_Scaling_List(int *scalingList, int sizeOfScalingList,
                                 char *UseDefaultScalingMatrix, unsigned char *p_tmp_buf, int end_pos)
{
    int j = 0;
    int scanj = 0;
    int delta_scale = 0;
    int lastScale = 8;
    int nextScale = 8;

    for (j = 0; j < sizeOfScalingList; j++) {
        scanj = (sizeOfScalingList == 16) ? x_check_ZZ_SCAN[j] : x_check_ZZ_SCAN8[j];

        if (nextScale != 0) {
            x_bitoffset += x_check_get_se(p_tmp_buf, x_bitoffset, &delta_scale, end_pos);
            nextScale = (lastScale + delta_scale + 256) % 256;
            *UseDefaultScalingMatrix = (scanj == 0 && nextScale == 0);
        }

        scalingList[scanj] = (nextScale == 0) ? lastScale : nextScale;
        lastScale = scalingList[scanj];
    }
}

static int x_check_h264_slice(char *p_ebsp, int start_pos, int end_pos,
                              int *frame_mbs_only, int *filed_flag, int *bottom_field_flag)
{
    int mbNum = 0;
    int type = -1;
    int value = 0;
    unsigned char *p_tmp = (unsigned char *)p_ebsp;
    EBSPtoRBSP((unsigned char *)p_ebsp, start_pos, end_pos);
    x_bitoffset = 0;
    *filed_flag = 0;
    mbNum = end_pos - start_pos;
    while (mbNum--) {
        if ((p_tmp[0] == 0) && (p_tmp[1] == 0) && (p_tmp[2] == 1) && (((p_tmp[3] & 0x1f) == 1) || ((p_tmp[3] & 0x1f) == 5))) {
            break;
        }
        p_tmp++;
    }
    p_tmp += 4;
    x_bitoffset += x_check_get_ue(p_tmp, x_bitoffset, &mbNum, end_pos - start_pos);
    if (!mbNum) {
        x_bitoffset += x_check_get_ue(p_tmp, x_bitoffset, &type, end_pos - start_pos);
    } else {
        type = -1;
        return type;
    }
    x_bitoffset += x_check_get_ue(p_tmp, x_bitoffset, &mbNum, end_pos - start_pos);
    x_bitoffset += x_check_get_bits(p_tmp, x_bitoffset, &value, end_pos, x_check_log2maxframe);
    if (!*frame_mbs_only) {
        x_bitoffset += x_check_get_bits(p_tmp, x_bitoffset, filed_flag, end_pos, 1);
    }
    if (*filed_flag) {
        x_bitoffset += x_check_get_bits(p_tmp, x_bitoffset, bottom_field_flag, end_pos, 1);
    }

    return type;
}
#endif

int ds_get_one_pic(demux_stream_t *ds, unsigned char **start, int *size)
{
    unsigned char *buf;
    unsigned char *realloc_buf = NULL;
    int state = -1;
    int find_count = 0;
    int i = 0, ret = 0;
    int start_pos = 0;

    if (ds->trickbuf == NULL) {
        ds->trickbuf = malloc33(384 * 1024);
        if (ds->trickbuf == NULL) {
            return -1;
        }
        ds->trickbuf_pos = ds->trickbuf_size = 0;
        ds->trickbuf_alloc_len = 384 * 1024;
    }
    if (ds->trickbuf_pos >= ds->trickbuf_alloc_len / 2) {
        memcpy(&ds->trickbuf[0], &ds->trickbuf[ds->trickbuf_pos], ds->trickbuf_size - ds->trickbuf_pos);
        ds->trickbuf_size = ds->trickbuf_size - ds->trickbuf_pos;
        ds->trickbuf_pos = 0;
    }
    buf = &ds->trickbuf[ds->trickbuf_size];
    start_pos = ds->trickbuf_pos;
    if (ds->trickbuf_size + *size <= ds->trickbuf_alloc_len) {
        memcpy(buf, *start, *size);
        ds->trickbuf_size += *size;
    } else if (ds->trickbuf_size + *size <= 1024 * 1024) {
        realloc_buf = malloc33(ds->trickbuf_size + *size);
        if (realloc_buf != NULL) {
            memcpy(realloc_buf, &ds->trickbuf[0], ds->trickbuf_size);
            memcpy(realloc_buf + ds->trickbuf_size, *start, *size);
            free33(ds->trickbuf);
            ds->trickbuf = realloc_buf;
            ds->trickbuf_alloc_len = ds->trickbuf_size + *size;
            ds->trickbuf_size += *size;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
    while (find_count <= 1) {
        buf = &ds->trickbuf[0];
        for (i = ds->trickbuf_pos; i < ds->trickbuf_size; i++) {

            state = (state << 8) | buf[i];
            if (state == 0x100) {
                find_count++;
            }
            if (state == 0x100 && find_count > 1) {
                ds->trickbuf_pos = i - 3;
                break;
            }
        }
        if (i == ds->trickbuf_size && find_count <= 1) {
            ds->trickbuf_pos = ds->trickbuf_size;
            if (ds->eof) {
                return -1;
            } else {
                ret = ds_get_packet(ds, start);

                if (ds->trickbuf_size + ret <= ds->trickbuf_alloc_len && ret > 0) {
                    memcpy(&ds->trickbuf[ds->trickbuf_size], *start, ret);
                    ds->trickbuf_size += ret;
                } else if (ds->trickbuf_size + ret <= 1024 * 1024 && ret > 0) {
                    realloc_buf = malloc33(ds->trickbuf_size + ret);
                    if (realloc_buf != NULL) {
                        memcpy(realloc_buf, &ds->trickbuf[0], ds->trickbuf_size);
                        memcpy(realloc_buf + ds->trickbuf_size, *start, ret);
                        free33(ds->trickbuf);
                        ds->trickbuf = realloc_buf;
                        ds->trickbuf_alloc_len = ds->trickbuf_size + ret;
                        ds->trickbuf_size += ret;
                    } else {
                        return -1;
                    }
                } else {
                    return -1;
                }
            }
        }
    }
    if (find_count > 1) {
        *start = &ds->trickbuf[start_pos];
        *size = ds->trickbuf_pos - start_pos;
    }

    return 0;
}

#if OLD_CODE
extern int h264_frm_only_flag;
extern int mp_field_pic_flag;
extern int ds_is_codec_h264(unsigned int id);
#endif
static void ds_add_packet_internal(demux_stream_t *ds, demux_packet_t *dp)
{
#ifdef OLD_CODE
    demuxer_t *demux = (demuxer_t *)ds->demuxer;
    if (io_isnetworkstream) {
        int ds_v_packs = (demux->video == NULL) ? 0 : demux->video->packs;
        int ds_a_packs = (demux->audio == NULL) ? 0 : demux->audio->packs;
        int ds_v_bytes = (demux->video == NULL) ? 0 : demux->video->bytes;
        int ds_a_bytes = (demux->audio == NULL) ? 0 : demux->audio->bytes;
        int default_size = 3000000;
        FILE_SEQ_T *p_file_seq = x_get_cur_instance();

        if (p_file_seq->video_disp_w >= 1920) {
            default_size = 10000000;
        }

        if (ds_a_bytes > 3000000 && g_is_live_broadcast) {
            mtos_printk("[%s]apks[%d],amem[%d],vpks[%d],vmem[%d]\n", __func__, ds_a_packs, ds_a_bytes, ds_v_packs, ds_v_bytes);
            ds_free_packs(demux->audio);
        }
        if (ds_v_bytes > default_size && g_is_live_broadcast) {
            mtos_printk("[%s]apks[%d],amem[%d],vpks[%d],vmem[%d]\n", __func__, ds_a_packs, ds_a_bytes, ds_v_packs, ds_v_bytes);
            ds_free_packs(demux->video);
        }
    }
#endif

    if (dp->len > BIG_PACK_LEN) {
        ds->big_packs++;
    }
    if (ds == ds->demuxer->video && ds->sh) {
#ifdef OLD_CODE
        sh_video_t *sh = ds->sh;
        if ((type == DEMUXER_TYPE_MPEG_TS) && (ds_is_codec_h264(sh->codec_id))) {

            if (!sh->sh.extradata_parsed) {
                int i = 0;
                unsigned int state = -1;
                unsigned char *buf = dp->buffer;
                int vol_len = 0;
                int has_vol = 0;
                for (i = 0; i < dp->len; i++) {
                    state = (state << 8) | (buf[i] & 0xbf);
                    if (state == 0x127) {
                        has_vol = 1;
                    }
                    if ((state == 0x125 || state == 0x121) && has_vol) {
                        vol_len = i - 3;
                        break;
                    }
                }
#if OLD_CODE
                if (vol_len > 0) {
                    unsigned char *p_buf = (unsigned char *)malloc33(vol_len);
                    sh->sh.extradata_size = vol_len;
                    sh->sh.extra_data = (unsigned char *)malloc33(vol_len);
                    memcpy(sh->sh.extra_data, dp->buffer, vol_len);
                    memcpy(p_buf, dp->buffer, vol_len);
                    x_check_spsframe((char *)p_buf, 0, vol_len, &h264_frm_only_flag);
                    MLOGD("[%s]vol_len[%d],h264frm[%d],x_check_log2maxframe[%d]\n", __func__, vol_len, h264_frm_only_flag, x_check_log2maxframe);
                    sh->sh.extradata_parsed = 1;
                    free33(p_buf);
                    if (h264_frm_only_flag) {
                        mp_field_pic_flag = 0;
                    } else {
                        int mbNum = 0;
                        int slicetype = -1;
                        //int startBit = 0;
                        int len = dp->len - vol_len - 4;
                        p_buf = &buf[i + 1];
                        x_bitoffset = 0;

                        x_bitoffset += x_check_get_ue(p_buf, x_bitoffset, &mbNum, len);
                        x_bitoffset += x_check_get_ue(p_buf, x_bitoffset, &slicetype, len);
                        x_bitoffset += x_check_get_ue(p_buf, x_bitoffset, &mbNum, len);
                        x_bitoffset += x_check_get_bits(p_buf, x_bitoffset, &mbNum, len, x_check_log2maxframe);
                        if (!h264_frm_only_flag) {
                            x_bitoffset += x_check_get_bits(p_buf, x_bitoffset, &mp_field_pic_flag, len, 1);
                        }
                        if (mp_field_pic_flag) {
                            x_bitoffset += x_check_get_bits(p_buf, x_bitoffset, &mbNum, len, 1);
                        }
                        x_bitoffset = 0;
                        MLOGD("[%s]mp_field_pic_flag[%d]\n", __func__, mp_field_pic_flag);
                    }
                }
#endif
            }
            if (sh->sh.extradata_size && (sh->sh.needfilter == 4)) {
                demux_packet_t *new_pkt = new_demux_packet(sh->sh.extradata_size + dp->len);
                unsigned char *p_tmp = dp->buffer;
                if (NULL != new_pkt) {
                    memcpy(new_pkt->buffer, sh->sh.extra_data, sh->sh.extradata_size);
                    memcpy(new_pkt->buffer + sh->sh.extradata_size, p_tmp, dp->len);
                    //len = sh->sh.extradata_size + dp->len;
                    if (dp->buffer != new_pkt->buffer) {
                        dp->buffer = new_pkt->buffer;
                        new_pkt->buffer = p_tmp;
                        free_demux_packet(new_pkt);
                    }
                }
                sh->sh.needfilter = 0;
            }
        }
#endif
    }
    // append packet to DS stream:
    ++ds->packs;
    ds->bytes += dp->len;
    if (ds->last) {
        // next packet in stream
        ds->last->next = dp;
        ds->last = dp;
    } else {
        // first packet in stream
        ds->first = ds->last = dp;
    }
    MLOGD("DEMUX: Append packet to %s, len=%d  pts=%5.3f  pos=%u  [packs: A=%d V=%d]\n",
           (ds == ds->demuxer->audio) ? "d_audio" : "d_video", dp->len,
           dp->pts, (unsigned int)dp->pos, ds->demuxer->audio->packs,
           ds->demuxer->video->packs);
}

#ifdef CONFIG_FFMPEG_MP
static void allocate_parser(AVCodecContext **avctx, AVCodecParserContext **parser, unsigned format)
{
    enum CodecID codec_id = CODEC_ID_NONE;

    init_avcodec();

    switch (format) {
        case 0x1600:
        case MKTAG('M', 'P', '4', 'A')
                :
            codec_id = CODEC_ID_AAC;
            break;
        case 0x1602:
        case MKTAG('M', 'P', '4', 'L')
                :
            codec_id = CODEC_ID_AAC_LATM;
            break;
        case 0x2000:
        case 0x332D6361:
        case 0x332D4341:
        case 0x20736D:
        case MKTAG('s', 'a', 'c', '3')
                :
            codec_id = CODEC_ID_AC3;
            break;
        case MKTAG('d', 'n', 'e', 't')
                :
            // DNET/byte-swapped AC-3 - there is no parser for that yet
            //codec_id = CODEC_ID_DNET;
            break;
        case MKTAG('E', 'A', 'C', '3')
                :
        case MKTAG('e', 'c', '-', '3')
                :
            codec_id = CODEC_ID_EAC3;
            break;
        case 0x2001:
        case 0x86:
        case MKTAG('D', 'T', 'S', ' ')
                :
        case MKTAG('d', 't', 's', ' ')
                :
        case MKTAG('d', 't', 's', 'b')
                :
        case MKTAG('d', 't', 's', 'c')
                :
            codec_id = CODEC_ID_DTS;
            break;
        case MKTAG('f', 'L', 'a', 'C')
                :
            codec_id = CODEC_ID_FLAC;
            break;
        case MKTAG('M', 'L', 'P', ' ')
                :
            codec_id = CODEC_ID_MLP;
            break;
        case 0x55:
        case 0x5500736d:
        case 0x55005354:
        case MKTAG('.', 'm', 'p', '3')
                :
        case MKTAG('M', 'P', '3', ' ')
                :
        case MKTAG('L', 'A', 'M', 'E')
                :
            codec_id = CODEC_ID_MP3;
            break;
        case 0x50:
        case 0x5000736d:
        case MKTAG('.', 'm', 'p', '2')
                :
        case MKTAG('.', 'm', 'p', '1')
                :
            codec_id = CODEC_ID_MP2;
            break;
        case MKTAG('T', 'R', 'H', 'D')
                :
            codec_id = CODEC_ID_TRUEHD;
            break;
    }
    if (codec_id != CODEC_ID_NONE) {
        *avctx = avcodec_alloc_context3(NULL);
        if (!*avctx) {
            return;
        }
        *parser = av_parser_init(codec_id);
        if (!*parser) {
            av_freep(avctx);
        }
    }
}

static void get_parser(sh_common_t *sh, AVCodecContext **avctx, AVCodecParserContext **parser)
{
    *avctx = NULL;
    *parser = NULL;

    if (!sh || !sh->needs_parsing) {
        return;
    }

    *avctx = sh->avctx;
    *parser = sh->parser;
    if (*parser) {
        return;
    }

    allocate_parser(avctx, parser, sh->format);
    sh->avctx = *avctx;
    sh->parser = *parser;
}

int ds_parse(demux_stream_t *ds, uint8_t **buffer, int *len, double pts, off_t pos)
{
    AVCodecContext *avctx;
    AVCodecParserContext *parser;
    get_parser(ds->sh, &avctx, &parser);
    if (!parser) {
        return *len;
    }
    return av_parser_parse2(parser, avctx, buffer, len, *buffer, *len, pts, pts, pos);
}

void ds_clear_parser(demux_stream_t *ds)
{
    if (!ds->sh) {
        return;
    }
    clear_parser(ds->sh);
}
#endif
static void clear_parser(sh_common_t *sh)
{
    av_parser_close(sh->parser);
    sh->parser = NULL;
    av_freep(&sh->avctx);
}
void ds_add_packet(demux_stream_t *ds, demux_packet_t *dp)
{
#if PARSE_ON_ADD &&defined(CONFIG_FFMPEG_MP)
    int len = dp->len;
    int pos = 0;
    while (len > 0) {
        uint8_t *parsed_start = dp->buffer + pos;
        int parsed_len = len;
        int consumed = ds_parse(ds->sh, &parsed_start, &parsed_len, dp->pts, dp->pos);
        pos += consumed;
        len -= consumed;
        if (parsed_start == dp->buffer && parsed_len == dp->len) {
            ds_add_packet_internal(ds, dp);
        } else if (parsed_len) {
            demux_packet_t *dp2 = new_demux_packet(parsed_len);
            if (!dp2) {
                return;
            }
            dp2->pos = dp->pos;
            dp2->pts = dp->pts; // should be parser->pts but that works badly
            memcpy(dp2->buffer, parsed_start, parsed_len);
            ds_add_packet_internal(ds, dp2);
        }
    }
#else
    /* eof maybe set by memory alloc error */
    if (ds->eof || ds->discard) {
        while (dp) {
            demux_packet_t *dn = dp->next;
            free_demux_packet(dp);
            dp = dn;
        }
        return;
    }
    ds_add_packet_internal(ds, dp);
#endif
}

int ds_add_packet2(demux_stream_t *ds, demux_packet_t **dpi)
{
    if (!ds || !dpi || !(*dpi)) {
        return MT_FALSE;
    }

    demux_packet_t *dp = *dpi;
    /* eof maybe set by memory alloc error */
    if (ds->eof || ds->discard) {
        while (dp) {
            demux_packet_t *dn = dp->next;
            free_demux_packet(dp);
            dp = dn;
        }
        *dpi = NULL;
        return MT_FALSE;
    }

    ds_add_packet_internal(ds, dp);
    return MT_TRUE;
}

#define NAL_MAX_LEN     (5*1024*1024)
#ifndef AV_RB16
#   define AV_RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])
#endif
#ifndef AV_WB32
#define AV_WB32(p, d)                    \
    do {                                 \
    ((uint8_t *)(p))[3] = (d);       \
    ((uint8_t *)(p))[2] = (d) >> 8;  \
    ((uint8_t *)(p))[1] = (d) >> 16; \
    ((uint8_t *)(p))[0] = (d) >> 24; \
    } while (0)
#endif

static int alloc_and_copy(uint8_t **poutbuf, int *poutbuf_size,
                          const uint8_t *sps_pps, uint32_t sps_pps_size,
                          const uint8_t *in, uint32_t in_size)
{
    uint32_t offset         = *poutbuf_size;
    uint8_t nal_header_size = offset ? 3 : 4;
    void *tmp;

    *poutbuf_size += sps_pps_size + in_size + nal_header_size;
    tmp = av_realloc(*poutbuf, *poutbuf_size + FF_INPUT_BUFFER_PADDING_SIZE);
    if (!tmp) {
        return AVERROR(ENOMEM);
    }
    *poutbuf = tmp;
    if (sps_pps) {
        memcpy(*poutbuf + offset, sps_pps, sps_pps_size);
    }
    memcpy(*poutbuf + sps_pps_size + nal_header_size + offset, in, in_size);
    if (!offset) {
        AV_WB32(*poutbuf + sps_pps_size, 1);
    } else {
        (*poutbuf + offset + sps_pps_size)[0] =
            (*poutbuf + offset + sps_pps_size)[1] = 0;
        (*poutbuf + offset + sps_pps_size)[2] = 1;
    }

    return 0;
}

static int add_video_header_h264annexb(sh_video_t *sh_video, uint8_t **poutbuf,
    int *poutbuf_size, const uint8_t *buf, int buf_size, int keyframe)
{
    int i;
    uint8_t unit_type;
    int32_t nal_size;
    uint32_t cumul_size = 0;
    const uint8_t *buf_end = buf + buf_size;
    int ret = AVERROR(EINVAL);

    struct sh_h264header *sh = &sh_video->sh;
    if (!sh_video->codec_extradata || sh_video->codec_extradata_size < 6) {
        *poutbuf = (uint8_t *) buf;
        *poutbuf_size = buf_size;
        sh->needfilter = 0;
        return 0;
    }

    /* retrieve sps and pps NAL units from codec_extradata */
    if (!sh->extradata_parsed) {
        uint16_t unit_size;
        uint64_t total_size = 0;
        uint8_t *out = NULL, unit_nb, sps_done = 0, sps_seen = 0, pps_seen = 0;
        const uint8_t *codec_extradata = sh_video->codec_extradata + 4;
        static const uint8_t nalu_header[4] = {0, 0, 0, 1};
        /* retrieve length coded size */
        sh->length_size = (*codec_extradata++ & 0x3) + 1;
        /* retrieve sps and pps unit(s) */
        unit_nb = *codec_extradata++ & 0x1f; /* number of sps unit(s) */
        if (!unit_nb) {
            goto pps;
        } else {
            sps_seen = 1;
        }

        while (unit_nb--) {
            void *tmp;

            unit_size = AV_RB16(codec_extradata);
            total_size += unit_size + 4;
            if (total_size > INT_MAX - 16 ||
                codec_extradata + 2 + unit_size > sh_video->codec_extradata + sh_video->codec_extradata_size) {
                av_free(out);
                return AVERROR(EINVAL);
            }
            tmp = av_realloc(out, total_size + 16);
            if (!tmp) {
                av_free(out);
                return AVERROR(ENOMEM);
            }
            out = tmp;
            memcpy(out + total_size - unit_size - 4, nalu_header, 4);
            memcpy(out + total_size - unit_size,   codec_extradata + 2, unit_size);
            codec_extradata += 2 + unit_size;
pps:
            if (!unit_nb && !sps_done++) {
                unit_nb = *codec_extradata++; /* number of pps unit(s) */
                if (unit_nb) {
                    pps_seen = 1;
                }
            }
        }

        if (out) {
            memset(out + total_size, 0, 16);
        }

        if (!sps_seen || !pps_seen) {
            av_log(sh, AV_LOG_WARNING, "Warning: SPS / PPSNALU missing or invalid. The resulting stream may not play.\n");
        }

        av_free(sh_video->codec_extradata);
        sh_video->codec_extradata      = out;
        sh_video->codec_extradata_size  = total_size;
        sh->extradata_parsed = 1;
        sh->first_idr = 1;
        if (sh_video->codec_extradata_size < 6) {
            *poutbuf = (uint8_t *) buf;
            *poutbuf_size = buf_size;
            sh->needfilter = 0;
            return 0;
        }
    }

    *poutbuf_size = 0;
    *poutbuf = NULL;
    do {
        ret = AVERROR(EINVAL);
        if (buf + sh->length_size > buf_end) {
            goto fail;
        }
        for (nal_size = 0, i = 0; i < sh->length_size; i++) {
            nal_size = (nal_size << 8) | buf[i];
        }

        buf += sh->length_size;
        unit_type = *buf & 0x1f;

        // nal_size > NAL_MAX_LEN add by libin, sometimes buf + nal_size will overflow
        if (buf + nal_size > buf_end || nal_size < 0 || nal_size > NAL_MAX_LEN) {
            goto fail;
        }

        /* prepend only to the first type 5 NAL unit of an IDR picture */
        if (sh->first_idr && unit_type == 5) {
            if ((ret = alloc_and_copy(poutbuf, poutbuf_size,
                                      sh_video->codec_extradata, sh_video->codec_extradata_size,
                                      buf, nal_size)) < 0) {
                goto fail;
            }
            sh->first_idr = 0;
        } else {
            if ((ret = alloc_and_copy(poutbuf, poutbuf_size,
                                      NULL, 0,
                                      buf, nal_size)) < 0) {
                goto fail;
            }
            if (!sh->first_idr && unit_type == 1) {
                sh->first_idr = 1;
            }
        }

        buf += nal_size;
        cumul_size += nal_size + sh->length_size;
    } while (cumul_size < buf_size);

    return 1;

fail:
    av_freep(poutbuf);
    *poutbuf_size = 0;
    return ret;
}

void ds_read_packet(demux_stream_t *ds, stream_t *stream, int len,
                    double pts, off_t pos, int flags)
{
    demux_packet_t *dp = new_demux_packet(len);
    if (!dp) {
        return;
    }

    len = stream_read(stream, (char *)dp->buffer, len);
    if (ds == ds->demuxer->video && ds->sh) {
        sh_video_t *sh = ds->sh;
        int type = ds->demuxer->type;
        unsigned long long biComp = 0;
        if (sh->bih) {
            biComp = le2me_32(sh->bih->biCompression);
        }

        if ((type == DEMUXER_TYPE_MPEG4_ES) || (sh->sh.needfilter && (strstr(((char *)&biComp), "avc1")))) {
            demux_packet_t *new_pkt = new_demux_packet(0);
            unsigned char *p_tmp = dp->buffer;
            if (NULL != new_pkt) {
                //use outside ffmpeg disable temperary
                add_video_header_h264annexb(sh, (uint8_t **) & (new_pkt->buffer), &(new_pkt->len), (const uint8_t *)dp->buffer, dp->len, 1);
                len = new_pkt->len;
                if (dp->buffer != new_pkt->buffer) {
                    dp->buffer = new_pkt->buffer;
                    new_pkt->buffer = p_tmp;
                    free_demux_packet(new_pkt);
                } else {
                    new_pkt->buffer = NULL;
                    free_demux_packet(new_pkt);
                }
            }
        } else if (sh->sh.needfilter == 2) {
            if (!sh->sh.extradata_parsed) {
                int i;
                unsigned int state = -1;
                unsigned char *buf = dp->buffer;
                int vol_len = 0;
                for (i = 0; i < dp->len; i++) {
                    state = (state << 8) | buf[i];
                    if (state == 0x1B3 || state == 0x1B6) {
                        vol_len = i - 3;
                        break;
                    }
                }
                if (vol_len > 0) {
                    sh->codec_extradata_size = vol_len;
                    sh->codec_extradata = (unsigned char *)malloc33(vol_len);
                    if (NULL != sh->codec_extradata) {
                        memcpy(sh->codec_extradata, dp->buffer, vol_len);
                    }
                }
                sh->sh.extradata_parsed = 1;
            }
            if (sh->codec_extradata_size && flags) {
                demux_packet_t *new_pkt = new_demux_packet(sh->codec_extradata_size + dp->len);
                unsigned char *p_tmp = dp->buffer;
                if (NULL != new_pkt) {
                    memcpy(new_pkt->buffer, sh->codec_extradata, sh->codec_extradata_size);
                    memcpy(new_pkt->buffer + sh->codec_extradata_size, p_tmp, dp->len);
                    len = sh->codec_extradata_size + dp->len;
                    if (dp->buffer != new_pkt->buffer) {
                        dp->buffer = new_pkt->buffer;
                        new_pkt->buffer = p_tmp;
                        free_demux_packet(new_pkt);
                    }
                }
            }
        }
    }
    resize_demux_packet(dp, len);
    dp->pts = pts;
    dp->pos = pos;
    dp->flags = flags;
    // append packet to DS stream:
    ds_add_packet(ds, dp);
}

int hls_fps_first = 0;
int hls_update_fps = 0;
int hls_detect_fps = 0;
int has_hls_fps = 0;
int detect_pks_num;
int hls_fps_done = 0;
int hls_protocol = 0;
int revise_fps = 0;
double detect_last_pts;
double detect_start_pts;

void ds_detect_hls_reset(void)
{
    hls_update_fps = 0;
    hls_detect_fps = 0;
    has_hls_fps = 0;
    detect_pks_num = 0;
    hls_fps_done = 0;
    detect_last_pts = 0;
    detect_start_pts = 0;
    hls_fps_first = 0;
    hls_protocol = 0;
    revise_fps = 0;
}
void ds_detect_hls_fps(demux_packet_t *p)
{
    if (p->len && !hls_fps_done) {
        hls_update_fps = 1;
        if (!has_hls_fps) {
            detect_start_pts = p->pts;
            detect_last_pts = p->pts;
            has_hls_fps = 1;
            detect_pks_num = 0;
            hls_detect_fps = 0;
        }
        if (p->pts - detect_last_pts > 1.0 || p->pts - detect_last_pts < -1.0) {
            has_hls_fps = 0;
            detect_pks_num = 0;
        } else {
            detect_pks_num++;
            detect_last_pts = p->pts;
        }
        if (detect_pks_num >= 50) {
            hls_detect_fps = detect_pks_num / (p->pts - detect_start_pts) + 0.5;
        }
        if (detect_pks_num >= 600) {
            hls_fps_done = 1;
        }
    }
}
// return value:
//     0 = EOF or no stream found or invalid type
//     1 = successfully read a packet

int demux_fill_buffer(demuxer_t *demux, demux_stream_t *ds)
{
    return demux->desc->fill_buffer(demux, ds);
}

// return value:
//     0 = EOF
//     1 = successful
#define MAX_ACCUMULATED_PACKETS 64
int ds_prefill_buffer(demux_stream_t *ds)
{
    demuxer_t *demux = ds->demuxer;
    int v_a_bytes = demux->audio->bytes + demux->video->bytes;
    if (v_a_bytes < 1000 * 1024) {
        demux_fill_buffer(demux, ds);
    }
    return v_a_bytes;
}

int is_full_drop(demux_stream_t *ds)
{
    FILE_SEQ_T *p_file_seq = file_seq_get_instance();
    if (TS_SEQ_NORMAL_PLAY != p_file_seq->cur_speed || MT_TRUE == p_file_seq->is_audio_deecoder_error) {
        goto DROPD;
    }

    return 0;

DROPD:
    return 1;
}

/* func: for trick drop */
/* args: */
/* num: drop pack number */
/* opt: [0]: drop audio, [1] drop subtitle, [2] drop video */
void ds_drop_data(demux_stream_t *ds, int num, int opt)
{
    int k;

    if (NULL == ds || NULL == ds->demuxer) {
        MLOGD("\n[%s_%d]ds=%p,%p\n", __func__, __LINE__, ds, ds ? ds->demuxer : NULL);
        return;
    }

    demuxer_t *demux = ds->demuxer;
    unsigned char *pkt_buf;


    switch (opt) {
        case 0:
            for (k = 0; k < MIN(demux->audio->packs, num); k++) {
                ds_get_packet(demux->audio, &pkt_buf);
            }
            break;
        case 1:
            MLOGD("\n[%s_%d]drop sub:pack=%d,num=%d\n", __func__, __LINE__, demux->sub->packs, MIN(demux->sub->packs, num));
            for (k = 0; k < MIN(demux->sub->packs, num); k++) {
                ds_get_packet(demux->sub, &pkt_buf);
            }
            break;
        case 2:
            MLOGD("\n[%s_%d]drop vid:pack=%d,num=%d\n", __func__, __LINE__, demux->video->packs, MIN(demux->video->packs, num));
            for (k = 0; k < MIN(demux->video->packs, num); k++) {
                ds_get_packet(demux->video, &pkt_buf);
            }
            break;
        default:
            break;
    }
}

void ds_full_drop_data(demux_stream_t *ds, int opt)
{
    if (is_full_drop(ds)) {
        ds_drop_data(ds, 50, opt);
    }
}

//extern int ds_get_packet(demux_stream_t *ds, unsigned char **start);
int ds_fill_buffer(demux_stream_t *ds)
{
    int break_flag = 0;
    demuxer_t *demux = ds->demuxer;

    if (ds->current) {
        free_demux_packet(ds->current);
    }

    ds->current = NULL;
    while (is_file_seq_exit() == FALSE) {

        if (ds->packs) {
            demux_packet_t *p = ds->first;
            // obviously not yet EOF after all
            ds->eof = 0;

            // copy useful data:
            ds->buffer = p->buffer;
            ds->buffer_pos  = 0;
            ds->buffer_size = p->len;
            ds->pos   = p->pos;
            ds->dpos += p->len;
            ds->sample_size = p->sample_size;
            ds->pts31bit    = p->pts31bit;
            ds->gop_seg_num = p->gop_seg_num;
            ++ds->pack_no;
            ds->pts_valide = MT_FALSE;
            ds->is_secure  = p->is_secure;
            if (p->pts != MP_NOPTS_VALUE) {
                ds->pts        = p->pts;
                ds->pts_bytes  = 0;
                ds->pts_valide = MT_TRUE;
            }
            ds->pts_bytes += p->len;
            if (p->stream_pts != MP_NOPTS_VALUE) {
                demux->stream_pts = p->stream_pts;
            }
            ds->flags = p->flags;
            // unlink packet:
            ds->bytes -= p->len;
            ds->current = p;
            ds->first   = p->next;
            if (!ds->first) {
                ds->last = NULL;
            }
            --ds->packs;
            if (revise_fps && !hls_fps_done && hls_protocol && ds == demux->video) {
                ds_detect_hls_fps(p);
            }
            return 1;
        }

        if (ds->eof || ds->discard) {
            break_flag = 1;
            break;
        }

        if (DEMUXER_TYPE_MPEG_TS == (ds->demuxer)->type) {
            int max_bytes = MAX_PACK_BYTES;
            if (demux->audio->packs >= MAX_PACKS ||
                demux->audio->bytes >= max_bytes) {
                MLOGD(MSGTR_TooManyAudioInBuffer, demux->audio->packs, demux->audio->bytes);

                return 0;
            }
            max_bytes = 10 * MAX_PACK_BYTES;
            if (demux->video->packs >= MAX_PACKS ||
                demux->video->bytes >= max_bytes) {

                MLOGD(MSGTR_TooManyVideoInBuffer, demux->video->packs, demux->video->bytes);
                return 0;
            }
        } else {
            int v_a_bytes = 0;
            int max_bytes = (0x300000);
            if (demux->audio->packs >= MAX_PACKS ||
                demux->audio->bytes >= max_bytes) {
                MLOGD(MSGTR_TooManyAudioInBuffer,  demux->audio->packs, demux->audio->bytes);
                ds_full_drop_data(ds, 0);
                return 0;
            }
            v_a_bytes = demux->audio->bytes + demux->video->bytes;
            /* if no a/v in buffer, we should continued to push */
            if ((v_a_bytes > (10 * MAX_PACK_BYTES)) ||
                (v_a_bytes > MAX_PACK_BYTES && demux->audio->bytes && demux->video->bytes)) {
                MLOGD("Too many bytes in the buffer:(0x%x 0x%x bytes) limit:0x%x\n",
                      demux->audio->bytes, demux->video->bytes, MAX_PACK_BYTES);
                return 0;
            }

            int v_a_big_packs = demux->audio->big_packs + demux->video->big_packs;
            if (v_a_big_packs > MAX_BIG_PACK_NUM  && demux->audio->bytes && demux->video->bytes) {
                MLOGD("Too many big packs(%d %d),limit:%d\n",
                      demux->audio->big_packs, demux->video->big_packs, MAX_BIG_PACK_NUM);
                return 0;
            }
        }

        if (ds == ds->demuxer->sub) {
            return 0;
        }

        if (!demux_fill_buffer(demux, ds)) {
            break_flag = 4;
            MLOGI("[%s] demux_fill_buffer() failed  \n", __func__);
            break;
        }
    }
    ds->buffer_pos = ds->buffer_size = 0;
    ds->buffer = NULL;
    MLOGI("ds_fill_buffer: EOF reached (stream: %s)\n", ds == demux->audio ? "audio" : "video");

    ds->eof = 1;
    MLOGI("[%s] %s EOF reached, break_flag = %d, audio/video packs=[%d,%d]\n",
          __func__, ds == demux->audio ? "Audio" : (ds == demux->video ? "Video" : "Subtitle"),
          break_flag, demux->audio->packs, demux->video->packs);

    return 0;
}

int demux_read_data(demux_stream_t *ds, unsigned char *mem, int len)
{
    int x;
    int bytes = 0;
    while (len > 0) {
        x = ds->buffer_size - ds->buffer_pos;
        if (x == 0) {
            if (!ds_fill_buffer(ds)) {
                return bytes;
            }
        } else {
            if (x > len) {
                x = len;
            }
            if (mem) {
                fast_memcpy(mem + bytes, &ds->buffer[ds->buffer_pos], x);
            }
            bytes += x;
            len -= x;
            ds->buffer_pos += x;
        }
    }
    return bytes;
}

/**
 * \brief read data until the given 3-byte pattern is encountered, up to maxlen
 * \param mem memory to read data into, may be NULL to discard data
 * \param maxlen maximum number of bytes to read
 * \param read number of bytes actually read
 * \param pattern pattern to search for (lowest 8 bits are ignored)
 * \return whether pattern was found
 */
int demux_pattern_3(demux_stream_t *ds, unsigned char *mem, int maxlen,
                    int *read, uint32_t pattern)
{
    register uint32_t head = 0xffffff00;
    register uint32_t pat  = pattern & 0xffffff00;
    int total_len = 0;
    do {
        register unsigned char *ds_buf = &ds->buffer[ds->buffer_size];
        int len = ds->buffer_size - ds->buffer_pos;
        register long pos = -len;
        if (unlikely(pos >= 0)) { // buffer is empty
            ds_fill_buffer(ds);
            continue;
        }
        do {
            head |= ds_buf[pos];
            head <<= 8;
        } while (++pos && head != pat);
        len += pos;
        if (total_len + len > maxlen) {
            len = maxlen - total_len;
        }
        len = demux_read_data(ds, mem ? &mem[total_len] : NULL, len);
        total_len += len;
    } while ((head != pat || total_len < 3) && total_len < maxlen && !ds->eof);
    if (read) {
        *read = total_len;
    }
    return total_len >= 3 && head == pat;
}

void ds_free_packs(demux_stream_t *ds)
{
    demux_packet_t *dp = ds->first;
    while (dp) {
        demux_packet_t *dn = dp->next;
        free_demux_packet(dp);
        dp = dn;
    }
    if (ds->asf_packet) {
        // free unfinished .asf fragments:
        free33(ds->asf_packet->buffer);
        free33(ds->asf_packet);
        ds->asf_packet = NULL;
    }
    ds->first = ds->last = NULL;
    ds->packs = 0;
    ds->bytes = 0;
    if (ds->current) {
        free_demux_packet(ds->current);
    }
    ds->current = NULL;
    ds->buffer  = NULL;
    ds->buffer_pos = ds->buffer_size;
    ds->pts       = 0;
    ds->pts_bytes = 0;
    ds->big_packs = 0;
}

int ds_get_packet(demux_stream_t *ds, unsigned char **start)
{
    int len;

    if(ds->buffer_pos < 0) {
        MLOGE("buffer pos %d < 0, ds buffer:%p size:%d not security\n", ds->buffer_pos, ds->buffer, ds->buffer_size);
    }
    if (ds->buffer_pos >= ds->buffer_size) {
        if (!ds_fill_buffer(ds)) {
            *start = NULL;
            return -1;
        }
    }
    len = ds->buffer_size - ds->buffer_pos;
    *start = &ds->buffer[ds->buffer_pos];
    ds->buffer_pos += len;
    if (len > BIG_PACK_LEN) {
        ds->big_packs--;
    }

    return len;
}

int ds_get_packet_pts(demux_stream_t *ds, unsigned char **start, double *pts)
{
    int len;
    *pts = MP_NOPTS_VALUE;
    len = ds_get_packet(ds, start);
    if (len < 0) {
        return len;
    }
    // Return pts unless this read starts from the middle of a packet
    if (len == ds->buffer_pos) {
        *pts = ds->current->pts;
    }
    return len;
}

/**
 * Get a subtitle packet. In particular avoid reading the stream.
 * \param pts input: maximum pts value of subtitle packet. NOPTS or NULL for any.
 *            output: start/reference pts of subtitle
 *            May be NULL.
 * \param endpts output: pts for end of display time. May be NULL.
 * \return -1 if no packet is available
 */
int ds_get_packet_sub(demux_stream_t *ds, unsigned char **start,
                      double *pts, double *endpts)
{
    double max_pts = MP_NOPTS_VALUE;
    int len;
    *start = NULL;
    // initialize pts
    if (pts) {
        max_pts = *pts;
        *pts = MP_NOPTS_VALUE;
    }
    if (endpts) {
        *endpts = MP_NOPTS_VALUE;
    }
    if (ds->buffer_pos >= ds->buffer_size) {
        if (!ds->packs) {
            return -1;    // no sub
        }
        if (!ds_fill_buffer(ds)) {
            return -1;    // EOF
        }
    }
    // only start of buffer has valid pts
    if (ds->buffer_pos == 0) {
        if (endpts) {
            *endpts = ds->current->endpts;
        }
        if (pts) {
            *pts = ds->current->pts;
            // check if we are too early
            if (*pts != MP_NOPTS_VALUE && max_pts != MP_NOPTS_VALUE &&
                *pts > max_pts) {
                return -1;
            }
        }
    }
    len = ds->buffer_size - ds->buffer_pos;
    *start = &ds->buffer[ds->buffer_pos];
    ds->buffer_pos += len;
    return len;
}

double ds_get_next_pts(demux_stream_t *ds)
{
    demuxer_t *demux = ds->demuxer;
    int max_bytes = MAX_PACK_BYTES;
    if (DEMUXER_TYPE_MPEG_TS == (ds->demuxer)->type) {
        max_bytes = (0x300000);
    }
    // if we have not read from the "current" packet, consider it
    // as the next, otherwise we never get the pts for the first packet.
    while (!ds->first && (!ds->current || ds->buffer_pos)) {
        if (demux->audio->packs >= MAX_PACKS || demux->audio->bytes >= max_bytes) {
            mp_msg(MSGT_DEMUXER, MSGL_ERR, MSGTR_TooManyAudioInBuffer,
                   demux->audio->packs, demux->audio->bytes);
            mp_msg(MSGT_DEMUXER, MSGL_HINT, MSGTR_MaybeNI);
            return MP_NOPTS_VALUE;
        }
        if (demux->video->packs >= MAX_PACKS || demux->video->bytes >= max_bytes) {
            mp_msg(MSGT_DEMUXER, MSGL_ERR, MSGTR_TooManyVideoInBuffer,
                   demux->video->packs, demux->video->bytes);
            mp_msg(MSGT_DEMUXER, MSGL_HINT, MSGTR_MaybeNI);
            return MP_NOPTS_VALUE;
        }
        if (!demux_fill_buffer(demux, ds)) {
            return MP_NOPTS_VALUE;
        }
    }
    // take pts from "current" if we never read from it.
    if (ds->current && !ds->buffer_pos) {
        return ds->current->pts;
    }
    if (ds->first) {    // for tsscan
        return ds->first->pts;
    }
    return MP_NOPTS_VALUE;
}

// ====================================================================

static int is_special_http_music(char *filename)
{
    char *p;

    p = strstr(filename,"http://");
    if(p == NULL)
    {
        p = strstr(filename,"https://");
        if(p == NULL)
        {
            return FALSE;
        }
    }

    p = strstr(p,"/mp3");
    if(p == NULL)
    {
        return FALSE;
    }

    MLOGD("This is special http server music [%s_%d]\n",__FUNCTION__, __LINE__);

    return TRUE;
}

static int demux_music_check(char *filename)
{
    //is mp3 or wav ?  ,fix by ybc
    int filename_len;
    char *p_type = NULL;

    if (filename == NULL) {
	return FALSE;
    }

    filename_len = strlen(filename);

    if (filename_len > 4) {
	p_type = filename + filename_len - 4;

	if ((strcmp(p_type, ".mp3") == 0) || (strcmp(p_type, ".MP3") == 0) || (strcmp(p_type, ".wav") == 0) ||
        (strcmp(p_type, ".WAV") == 0) || is_special_http_music(filename)) {
	    return TRUE;
	}
    }

    return FALSE;
}

int correct_pts = 0;
int user_correct_pts = -1;

/*
NOTE : Several demuxers may be opened at the same time so
demuxers should NEVER rely on an external var to enable them
self. If a demuxer can't do any auto-detection it should only use
file_format. The user can explicitly set file_format with the -demuxer
option so there is really no need for another extra var.
For convenience an option can be added to set file_format directly
to the right type (ex: rawaudio,rawvideo).
Also the stream can override the file_format so a demuxer which rely
on a special stream type can set file_format at the stream level
(ex: tv,mf).
 */

static demuxer_t *demux_open_stream(stream_t *stream, int file_format,
    int force, int audio_id, int video_id, int dvdsub_id, char *filename)
{
    int i;
    int fformat = 0;
    demuxer_t *demuxer = NULL;
    sh_video_t *sh_video = NULL;
    const demuxer_desc_t *demuxer_desc;

    MLOGI("[%s] stream=%p format=%d force=%d audio_id=%d video_id=%d dvdsub_id=%d type:%d\n",
        __func__, stream, file_format, force, audio_id, video_id, dvdsub_id, stream->type);
    // for identifying net stream
    //sometime in file stream, this var maybe (STREAMTYPE_STREAM),but not found now
    if (file_format == DEMUXER_TYPE_UNKNOWN) {
        if ((filename && strstr(filename, "udp://")) || (filename && demux_music_check(filename))) {
            file_format = DEMUXER_TYPE_AUDIO; //DEMUXER_TYPE_LAVF;//
        } else if (stream->type == STREAMTYPE_STREAM) {
            file_format = DEMUXER_TYPE_LAVF;
        }
    }

    // If somebody requested a demuxer check it
    if (file_format) {
        if ((demuxer_desc = get_demuxer_desc_from_type(file_format))) {
            demuxer = new_demuxer(stream, demuxer_desc->type, audio_id,
                                  video_id, dvdsub_id, filename);
            if (demuxer_desc->check_file) {
                fformat = demuxer_desc->check_file(demuxer);
            }
            if (force || !demuxer_desc->check_file) {
                fformat = demuxer_desc->type;
            }

            MLOGD("[%s] ----- demuxer_desc->type=%d\n", __func__, fformat);
            demuxer_t *demux2 = demuxer;
            if (fformat != 0) {
                if (fformat == demuxer_desc->type) {
                    // Move messages to demuxer detection code?
                    MLOGI(MSGTR_Detected_XXX_FileFormat, demuxer_desc->shortdesc);
                    file_format = fformat;

                    /* ts open return NULL, current demuxer,
                     * lavf, return DEMUXER_LAVF_INVALID_VALUE, null or current demuxer,
                     * mpeg_ps, mpeg4_es open return current demuxer and audio has no open function
                     */
                    if (!demuxer->desc->open || (((demux2 = demuxer->desc->open(demuxer)) != NULL) && ((unsigned long)demux2 != DEMUXER_LAVF_INVALID_VALUE))) {
                        if (demuxer != demux2) {
                            MLOGW("[%s] %d demuxer:%s demux2:%s\n",
                                __func__, __LINE__, demuxer->desc->shortdesc, demux2->desc->shortdesc);
                        }
                        demuxer = demux2;
                        if (demuxer) {
                            MLOGD("[%s] ok\n", __func__);
                            goto dmx_open;
                        }
                    }
                } else {
                    // Format changed after check, recurse
                    free_demuxer(demuxer);
                    MLOGI("[%s] Format changed after check, recurs\n", __func__);
                    return demux_open_stream(stream, fformat, force, audio_id, video_id, dvdsub_id, filename);
                }
            }
            // Check failed for forced demuxer, quit
            if ((unsigned long) demux2 == DEMUXER_LAVF_INVALID_VALUE) {
                stream->end_pos = demuxer->movi_end;
                free_demuxer(demuxer);
                return demux_open_stream(stream, DEMUXER_TYPE_MPEG_TS, force, audio_id, video_id, dvdsub_id, filename);
            }
            free_demuxer(demuxer);
            MLOGE("[%s] ----- Check failed for forced demuxer, quit\n", __func__);
            if (file_format == fformat) {
                return NULL;
            }
        }
    }

    if (is_file_seq_exit()) {
        return NULL;
    }

    MLOGD("[%s] ----- out of for: file_format=%d\n", __func__, file_format);
    // If no forced demuxer perform file extension based detection
    // Ok. We're over the stable detectable fileformats, the next ones are
    // a bit fuzzy. So by default (extension_parsing==1) try extension-based
    // detection first:
    if (file_format == DEMUXER_TYPE_UNKNOWN && filename) {
        file_format = demuxer_type_by_filename(filename);
        MLOGD("[%s] ----- after demuxer_type_by_filename: file_format=%d\n", __func__, file_format);
        if (file_format != DEMUXER_TYPE_UNKNOWN) {
            // we like recursion :)
            demuxer = demux_open_stream(stream, file_format, force, audio_id, video_id, dvdsub_id, filename);
            if (demuxer) {
                return demuxer;    // done!
            }
            file_format = DEMUXER_TYPE_UNKNOWN; // continue fuzzy guessing...
            MLOGI("continue fuzzy content-based format guessing...\n");
        }
    }

    // Try detection for all other demuxers
    for (i = 0; (demuxer_desc = demuxer_list[i]); i++) {
        if (demuxer_desc->check_file) {
            demuxer = new_demuxer(stream, demuxer_desc->type, audio_id, video_id, dvdsub_id, filename);
            if ((fformat = demuxer_desc->check_file(demuxer)) != DEMUXER_TYPE_UNKNOWN) {
                if (fformat == demuxer_desc->type) {
                    demuxer_t *demux2 = demuxer;
                    file_format = fformat;
                    if (!demuxer->desc->open || (demux2 = demuxer->desc->open(demuxer))) {
                        if (demux2 == NULL) {
                            free_demuxer(demuxer);
                            return NULL;
                        }
                        demuxer = demux2;
                        goto dmx_open;
                    }
                } else {
                    // Format changed after check, recurse
                    free_demuxer(demuxer);
                    demuxer = demux_open_stream(stream, fformat, force, audio_id, video_id, dvdsub_id, filename);
                    if (demuxer) {
                        return demuxer;    // done!
                    }
                    file_format = DEMUXER_TYPE_UNKNOWN;
                }
            }
            free_demuxer(demuxer);
            demuxer = NULL;
        }
    }
    MLOGE("[%s] %d open fail\n", __func__, __LINE__);
    return NULL;

//====== File format recognized, set up these for compatibility: =========
dmx_open:
    MLOGI("[%s] ok:file_format = %d\n", __func__, file_format);
    demuxer->file_format = file_format;

    if ((sh_video = demuxer->video->sh) && sh_video->bih) {
        int biComp = le2me_32(sh_video->bih->biCompression);
        MLOGD("VIDEO:  [%.4s]  %dx%d  %dbpp  %5.3f fps  %5.1f kbps (%4.1f kbyte/s)\n",
               (char *)&biComp, sh_video->bih->biWidth,
               sh_video->bih->biHeight, sh_video->bih->biBitCount,
               sh_video->fps, sh_video->i_bps * 0.008f,
               sh_video->i_bps / 1024.0f);
    }
#ifdef CONFIG_ASS
    if (ass_enabled && ass_library) {
        for (i = 0; i < MAX_S_STREAMS; ++i) {
            sh_sub_t *sh = demuxer->s_streams[i];
            if (sh && sh->type == 'a') {
                sh->ass_track = ass_new_track(ass_library);
                if (sh->ass_track && sh->codec_extradata)
                    ass_process_codec_private(sh->ass_track, sh->codec_extradata,
                                              sh->codec_extradata_size);
            } else if (sh && sh->type != 'v') {
                sh->ass_track = ass_default_track(ass_library);
            }
        }
    }
#endif

    MLOGD("[%s] -%d----!\n", __func__, __LINE__);
    demuxer->enbale_hr_mp3_seek = FALSE;
    return demuxer;
}

demuxer_t *demux_open(stream_t *vs, int file_format, int audio_id,
                      int video_id, int dvdsub_id, char *filename) {
    demuxer_t *vd;

    MLOGD("[%s] ---start!file_format:%d\n", __func__, file_format);
    vd = demux_open_stream(vs, file_format, 0, audio_id, video_id, dvdsub_id, filename);
    if (!vd) {
        MLOGD("[%s] ----- vd = %p, retrun NULL\n", __func__, vd);
        return NULL;
    }

    correct_pts = user_correct_pts;
    if (correct_pts < 0) {
        correct_pts = !force_fps && demux_control(vd, DEMUXER_CTRL_CORRECT_PTS, NULL) == DEMUXER_CTRL_OK;
    }

    MLOGD("[%s] is OK --vd = %p, retrun \n", __func__, vd);
    return vd;
}

/**
 * Do necessary reinitialization after e.g. a seek.
 * Do _not_ call ds_fill_buffer between the seek and this, it breaks at least
 * seeking with ASF demuxer.
 */
static void demux_resync(demuxer_t *demuxer)
{
    sh_video_t *sh_video = demuxer->video->sh;
    sh_audio_t *sh_audio = demuxer->audio->sh;
    demux_control(demuxer, DEMUXER_CTRL_RESYNC, NULL);
    if (sh_video) {
        resync_video_stream(sh_video);
    }
    if (sh_audio) {
        resync_audio_stream(sh_audio);
    }
}

void demux_flush(demuxer_t *demuxer)
{
#if PARSE_ON_ADD
    ds_clear_parser(demuxer->video);
    ds_clear_parser(demuxer->audio);
    ds_clear_parser(demuxer->sub);
#endif
    ds_free_packs(demuxer->video);
    ds_free_packs(demuxer->audio);
    ds_free_packs(demuxer->sub);
}

int demux_seek(demuxer_t *demuxer, float rel_seek_secs, float audio_delay,
               int flags)
{
    double tmp = 0;
    double pts;

    if (!demuxer) {
        return 0;
    }

    if (!demuxer->seekable) {
        if (demuxer->file_format == DEMUXER_TYPE_AVI) {
            mp_msg(MSGT_SEEK, MSGL_WARN, MSGTR_CantSeekRawAVI);
        }
#ifdef CONFIG_TV
        else if (demuxer->file_format == DEMUXER_TYPE_TV) {
            mp_msg(MSGT_SEEK, MSGL_WARN, MSGTR_TVInputNotSeekable);
        }
#endif
        else {
            mp_msg(MSGT_SEEK, MSGL_WARN, MSGTR_CantSeekFile);
        }
        return 0;
    }

    demux_flush(demuxer);

    demuxer->stream->eof = 0;
    demuxer->video->eof = 0;
    demuxer->audio->eof = 0;
    demuxer->sub->eof = 0;

    if (flags & SEEK_ABSOLUTE) {
        pts = 0.0f;
    } else {
        if (demuxer->stream_pts == MP_NOPTS_VALUE) {
            goto dmx_seek;
        }
        pts = demuxer->stream_pts;
    }

    if (flags & SEEK_FACTOR) {
        if (STREAM_UNSUPPORTED == stream_control(
                demuxer->stream, STREAM_CTRL_GET_TIME_LENGTH, &tmp)) {
            goto dmx_seek;
        }
        pts += tmp * rel_seek_secs;
    } else {
        pts += rel_seek_secs;
    }

    if (STREAM_UNSUPPORTED != stream_control(
            demuxer->stream, STREAM_CTRL_SEEK_TO_TIME, &pts)) {
        demux_resync(demuxer);
        return 1;
    }

dmx_seek:
    if (demuxer->desc->seek) {
        demuxer->desc->seek(demuxer, rel_seek_secs, audio_delay, flags);
    }

    demux_resync(demuxer);

    return 1;
}

int demux_info_add(demuxer_t *demuxer, const char *opt, const char *param)
{
    char **info = demuxer->info;
    int n = 0;

    for (n = 0; info && info[2 * n] != NULL; n++) {
        if (!strcasecmp(opt, info[2 * n])) {
            if (!strcmp(param, info[2 * n + 1])) {
                mp_msg(MSGT_DEMUX, MSGL_V, "Demuxer info %s set to unchanged value %s\n", opt, param);
                return 0;
            }
            mp_msg(MSGT_DEMUX, MSGL_INFO, MSGTR_DemuxerInfoChanged, opt,
                   param);
            free33(info[2 * n + 1]);
            info[2 * n + 1] = strdup33(param);
            return 0;
        }
    }

    info = demuxer->info = realloc33(info, (2 * (n + 2)) * sizeof(char *));
    if (NULL != info) {
        info[2 * n] = strdup33(opt);
        info[2 * n + 1] = strdup33(param);
        memset(&info[2 * (n + 1)], 0, 2 * sizeof(char *));
    }
    return 1;
}

int demux_info_print(demuxer_t *demuxer)
{
    char **info = demuxer->info;
    int n;

    if (!info) {
        return 0;
    }

    mp_msg(MSGT_DEMUX, MSGL_INFO, MSGTR_ClipInfo);
    for (n = 0; info[2 * n] != NULL; n++) {
        mp_msg(MSGT_DEMUX, MSGL_INFO, " %s: %s\n", info[2 * n],
               info[2 * n + 1]);
        mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_CLIP_INFO_NAME%d=%s\n", n,
               info[2 * n]);
        mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_CLIP_INFO_VALUE%d=%s\n", n,
               info[2 * n + 1]);
    }
    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_CLIP_INFO_N=%d\n", n);

    return 0;
}

char *demux_info_get(demuxer_t *demuxer, const char *opt)
{
    int i;
    char **info = demuxer->info;

    for (i = 0; info && info[2 * i] != NULL; i++) {
        if (!strcasecmp(opt, info[2 * i])) {
            return info[2 * i + 1];
        }
    }

    return NULL;
}

int demux_control(demuxer_t *demuxer, int cmd, void *arg)
{
    if (demuxer->desc->control) {
        return demuxer->desc->control(demuxer, cmd, arg);
    }

    return DEMUXER_CTRL_NOTIMPL;
}

double demuxer_get_time_length(demuxer_t *demuxer)
{
	double get_time_ans;
	double dur1;
	double dur2;
	sh_video_t *sh_video = demuxer->video->sh;
	sh_audio_t *sh_audio = demuxer->audio->sh;
	// <= 0 means DEMUXER_CTRL_NOTIMPL or DEMUXER_CTRL_DONTKNOW
	MLOGD("%s%d:get the duration start\n",__func__,__LINE__);
	if (demux_control(demuxer, DEMUXER_CTRL_GET_TIME_LENGTH, (void *)&get_time_ans) <= 0) {
		if (sh_video && sh_video->i_bps && sh_audio && sh_audio->i_bps) {
			MLOGD("start:%f end:%f video_bps:%d audio_bps:%d\n",
				(double)(demuxer->movi_start),(double)(demuxer->movi_end),sh_video->i_bps,sh_audio->i_bps);
			dur1 = (double)(demuxer->movi_end -demuxer->movi_start) /
				(sh_video->i_bps);
			MLOGD("dur1= %lfms\n",dur1);
			dur2 = (double)(demuxer->movi_end -demuxer->movi_start) /
				(sh_audio->i_bps);
			MLOGD("dur2= %lfms\n",dur2);
			get_time_ans = max(dur2,dur1);
		} else if (sh_video && sh_video->i_bps) {
	    	get_time_ans = (double)(demuxer->movi_end -
	                            	demuxer->movi_start) /
	                  	 sh_video->i_bps;
		} else if (sh_audio && sh_audio->i_bps) {
	    	get_time_ans = (double)(demuxer->movi_end -
	                            	demuxer->movi_start) /
	                   	sh_audio->i_bps;
		} else {
	    	get_time_ans = 0;
    	}
	}
	MLOGD("%s %d get the duration %lf\n",__func__,__LINE__,get_time_ans);
    return get_time_ans;
}

/**
 * \brief demuxer_get_current_time() returns the time of the current play in three possible ways:
 *        either when the stream reader satisfies STREAM_CTRL_GET_CURRENT_TIME (e.g. dvd)
 *        or using sh_video->pts when the former method fails
 *        0 otherwise
 * \return the current play time
 */
double demuxer_get_current_time(demuxer_t *demuxer)
{
    double get_time_ans = 0;
    sh_video_t *sh_video = demuxer->video->sh;
    if (demuxer->stream_pts != MP_NOPTS_VALUE) {
        get_time_ans = demuxer->stream_pts;
    } else if (sh_video) {
        get_time_ans = sh_video->pts;
    }
    return get_time_ans;
}

int demuxer_get_percent_pos(demuxer_t *demuxer)
{
    int ans = 0;
    int res = demux_control(demuxer, DEMUXER_CTRL_GET_PERCENT_POS, &ans);
    int len = (demuxer->movi_end - demuxer->movi_start) / 100;
    if (res <= 0) {
        off_t pos = demuxer->filepos > 0 ? demuxer->filepos : stream_tell(demuxer->stream);
        if (len > 0) {
            ans = (pos - demuxer->movi_start) / len;
        } else {
            ans = 0;
        }
    }
    if (ans < 0) {
        ans = 0;
    }
    if (ans > 100) {
        ans = 100;
    }
    return ans;
}

int demuxer_switch_audio(demuxer_t *demuxer, int index)
{
    MLOGD("[%s]---------start!\n", __func__);
    MLOGD("[%s]---------audio_id=%d\n", __func__, index);

    int res = demux_control(demuxer, DEMUXER_CTRL_SWITCH_AUDIO, &index);
    MLOGD("[%s]---------demux_control() ret: %d\n", __func__, res);

    if (res == DEMUXER_CTRL_NOTIMPL) {
        index = demuxer->audio->id;
    }
    if (demuxer->audio->id >= 0) {
        demuxer->audio->sh = demuxer->a_streams[demuxer->audio->id];
    } else {
        demuxer->audio->sh = NULL;
    }

    MLOGD("[%s]---------demuxer->audio->id=%d\n", __func__, demuxer->audio->id);
    MLOGD("[%s]---------audio_id=%d\n", __func__, index);
    MLOGD("[%s]---------end!\n", __func__);
    return index;
}

int demuxer_switch_video(demuxer_t *demuxer, int index)
{
    int res = demux_control(demuxer, DEMUXER_CTRL_SWITCH_VIDEO, &index);
    if (res == DEMUXER_CTRL_NOTIMPL) {
        index = demuxer->video->id;
    }
    if (demuxer->video->id >= 0) {
        demuxer->video->sh = demuxer->v_streams[demuxer->video->id];
    } else {
        demuxer->video->sh = NULL;
    }
    return index;
}

int demuxer_get_playlist(demuxer_t *demuxer)
{
    int ret = demux_control(demuxer,
                            DEMUXER_CTRL_GET_PLAYLIST, (void *) NULL);
    if (ret < DEMUXER_CTRL_OK) {
        MLOGD("Demux get playlist fail\n");
        return MT_FALSE;
    }

    return MT_TRUE;
}

int demuxer_switch_playlist(demuxer_t *demuxer, int playlist_index)
{
    int ret = demux_control(demuxer,
                            DEMUXER_CTRL_SWITCH_PLAYLIST, (void *) &playlist_index);
    if (ret < DEMUXER_CTRL_OK) {
        MLOGE("Demux switch playlist fail\n");
        return MT_FALSE;
    }

    return MT_TRUE;
}

int demuxer_add_attachment(demuxer_t *demuxer, const char *name,
                           const char *type, const void *data, size_t size)
{
    if (!(demuxer->num_attachments & 31))
        demuxer->attachments = realloc33(demuxer->attachments,
                                         (demuxer->num_attachments + 32) * sizeof(demux_attachment_t));
    if (NULL == demuxer->attachments) {
        return 0;
    }
    demuxer->attachments[demuxer->num_attachments].name = name ? strdup33(name) : NULL;
    demuxer->attachments[demuxer->num_attachments].type = strdup33(type);
    demuxer->attachments[demuxer->num_attachments].data = malloc33(size);
    if (NULL != demuxer->attachments[demuxer->num_attachments].data) {
        memcpy(demuxer->attachments[demuxer->num_attachments].data, data, size);
        demuxer->attachments[demuxer->num_attachments].data_size = size;
    }

    return demuxer->num_attachments++;
}

int demuxer_add_chapter(demuxer_t *demuxer, const char *name, uint64_t start,
                        uint64_t end)
{
    if (demuxer->chapters == NULL) {
        demuxer->chapters = malloc33(32 * sizeof(*demuxer->chapters));
    } else if (!(demuxer->num_chapters % 32))
        demuxer->chapters = realloc33(demuxer->chapters,
                                      (demuxer->num_chapters + 32) * sizeof(*demuxer->chapters));
    if (NULL == demuxer->num_chapters) {
        return 0;
    }

    demuxer->chapters[demuxer->num_chapters].start = start;
    demuxer->chapters[demuxer->num_chapters].end = end;
    demuxer->chapters[demuxer->num_chapters].name = strdup33(name ? name : MSGTR_Unknown);

    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_CHAPTER_ID=%d\n", demuxer->num_chapters);
    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_CHAPTER_%d_START=%" PRIu64 "\n", demuxer->num_chapters, start);
    if (end) {
        mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_CHAPTER_%d_END=%" PRIu64 "\n", demuxer->num_chapters, end);
    }
    if (name) {
        mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_CHAPTER_%d_NAME=%s\n", demuxer->num_chapters, name);
    }

    return demuxer->num_chapters++;
}

/**
 * \brief demuxer_seek_chapter() seeks to a chapter in two possible ways:
 *        either using the demuxer->chapters structure set by the demuxer
 *        or asking help to the stream layer (e.g. dvd)
 * \param chapter - chapter number wished - 0-based
 * \param mode 0: relative to current main pts, 1: absolute
 * \param seek_pts set by the function to the pts to seek to (if demuxer->chapters is set)
 * \param num_chapters number of chapters present (set by this function is param is not null)
 * \param chapter_name name of chapter found (set by this function is param is not null)
 * \return -1 on error, current chapter if successful
 */

int demuxer_seek_chapter(demuxer_t *demuxer, int chapter, int mode,
                         float *seek_pts, int *num_chapters,
                         char **chapter_name)
{
    int ris;
    int current, total;

    if (!demuxer->num_chapters || !demuxer->chapters) {
        if (!mode) {
            ris = stream_control(demuxer->stream,
                                 STREAM_CTRL_GET_CURRENT_CHAPTER, &current);
            if (ris == STREAM_UNSUPPORTED) {
                return -1;
            }
            chapter += current;
        }

        demux_flush(demuxer);

        ris = stream_control(demuxer->stream, STREAM_CTRL_SEEK_TO_CHAPTER,
                             &chapter);

        demux_resync(demuxer);

        // exit status may be ok, but main() doesn't have to seek itself
        // (because e.g. dvds depend on sectors, not on pts)
        *seek_pts = -1.0;

        if (num_chapters) {
            if (stream_control(demuxer->stream, STREAM_CTRL_GET_NUM_CHAPTERS,
                               num_chapters) == STREAM_UNSUPPORTED) {
                *num_chapters = 0;
            }
        }

        if (chapter_name) {
            *chapter_name = NULL;
            if (num_chapters && *num_chapters) {
                char *tmp = malloc33(16);
                if (tmp) {
                    sprintf(tmp, " of %3d", *num_chapters);
                    *chapter_name = tmp;
                }
            }
        }

        return ris != STREAM_UNSUPPORTED ? chapter : -1;
    } else { // chapters structure is set in the demuxer
        sh_video_t *sh_video = demuxer->video->sh;
        sh_audio_t *sh_audio = demuxer->audio->sh;

        total = demuxer->num_chapters;

        if (mode == 1) { //absolute seeking
            current = chapter;
        } else { //relative seeking
            uint64_t now;
            now = (sh_video ? sh_video->pts : (sh_audio ? sh_audio->pts : 0.)) * 1000 + .5;

            for (current = total - 1; current >= 0; --current) {
                demux_chapter_t *chapter = demuxer->chapters + current;
                if (chapter->start <= now) {
                    break;
                }
            }
            current += chapter;
        }

        if (current >= total) {
            return -1;
        }
        if (current < 0) {
            current = 0;
        }

        *seek_pts = demuxer->chapters[current].start / 1000.0;

        if (num_chapters) {
            *num_chapters = demuxer->num_chapters;
        }

        if (chapter_name) {
            if (demuxer->chapters[current].name) {
                *chapter_name = strdup33(demuxer->chapters[current].name);
            } else {
                *chapter_name = NULL;
            }
        }

        return current;
    }
}

int demuxer_get_current_chapter(demuxer_t *demuxer)
{
    int chapter = -1;
    if (!demuxer->num_chapters || !demuxer->chapters) {
        if (stream_control(demuxer->stream, STREAM_CTRL_GET_CURRENT_CHAPTER,
                           &chapter) == STREAM_UNSUPPORTED) {
            chapter = -1;
        }
    } else {
        sh_video_t *sh_video = demuxer->video->sh;
        sh_audio_t *sh_audio = demuxer->audio->sh;
        uint64_t now;
        now = (sh_video ? sh_video->pts : (sh_audio ? sh_audio->pts : 0)) * 1000 + 0.5;
        for (chapter = demuxer->num_chapters - 1; chapter >= 0; --chapter) {
            if (demuxer->chapters[chapter].start <= now) {
                break;
            }
        }
    }
    return chapter;
}

char *demuxer_chapter_name(demuxer_t *demuxer, int chapter)
{
    if (demuxer->num_chapters && demuxer->chapters) {
        if (chapter >= 0 && chapter < demuxer->num_chapters && demuxer->chapters[chapter].name) {
            return strdup33(demuxer->chapters[chapter].name);
        }
    }
    return NULL;
}

char *demuxer_chapter_display_name(demuxer_t *demuxer, int chapter)
{
    char *chapter_name = demuxer_chapter_name(demuxer, chapter);
    if (chapter_name) {
        char *tmp = malloc33(strlen(chapter_name) + 14);
        if (NULL == tmp) {
            return NULL;
        }
        snprintf(tmp, 63, "(%d) %s", chapter + 1, chapter_name);
        free33(chapter_name);
        return tmp;
    } else {
        int chapter_num = demuxer_chapter_count(demuxer);
        char tmp[30];
        if (chapter_num <= 0) {
            sprintf(tmp, "(%d)", chapter + 1);
        } else {
            sprintf(tmp, "(%d) of %d", chapter + 1, chapter_num);
        }
        return strdup33(tmp);
    }
}

float demuxer_chapter_time(demuxer_t *demuxer, int chapter, float *end)
{
    if (demuxer->num_chapters && demuxer->chapters && chapter >= 0 && chapter < demuxer->num_chapters) {
        if (end) {
            *end = demuxer->chapters[chapter].end / 1000.0;
        }
        return demuxer->chapters[chapter].start / 1000.0;
    }
    return -1.0;
}

int demuxer_chapter_count(demuxer_t *demuxer)
{
    if (!demuxer->num_chapters || !demuxer->chapters) {
        int num_chapters = 0;
        if (stream_control(demuxer->stream, STREAM_CTRL_GET_NUM_CHAPTERS,
                           &num_chapters) == STREAM_UNSUPPORTED) {
            num_chapters = 0;
        }
        return num_chapters;
    } else {
        return demuxer->num_chapters;
    }
}

int demuxer_angles_count(demuxer_t *demuxer)
{
    int ris, angles = -1;

    ris = stream_control(demuxer->stream, STREAM_CTRL_GET_NUM_ANGLES, &angles);
    if (ris == STREAM_UNSUPPORTED) {
        return -1;
    }
    return angles;
}

int demuxer_get_current_angle(demuxer_t *demuxer)
{
    int ris, curr_angle = -1;
    ris = stream_control(demuxer->stream, STREAM_CTRL_GET_ANGLE, &curr_angle);
    if (ris == STREAM_UNSUPPORTED) {
        return -1;
    }
    return curr_angle;
}

int demuxer_set_angle(demuxer_t *demuxer, int angle)
{
    int ris, angles = -1;

    angles = demuxer_angles_count(demuxer);
    if ((angles < 1) || (angle > angles)) {
        return -1;
    }

    demux_flush(demuxer);

    ris = stream_control(demuxer->stream, STREAM_CTRL_SET_ANGLE, &angle);
    if (ris == STREAM_UNSUPPORTED) {
        return -1;
    }

    demux_resync(demuxer);

    return angle;
}

int demuxer_audio_lang(demuxer_t *d, int id, char *buf, int buf_len)
{
    struct stream_lang_req req;
    sh_audio_t *sh;
    if (id < 0 || id >= MAX_A_STREAMS) {
        return -1;
    }
    sh = d->a_streams[id];
    if (!sh) {
        return -1;
    }
    if (sh->lang) {
        av_strlcpy(buf, sh->lang, buf_len);
        return 0;
    }
    req.type = stream_ctrl_audio;
    req.id = sh->aid;
    if (stream_control(d->stream, STREAM_CTRL_GET_LANG, &req) == STREAM_OK) {
        av_strlcpy(buf, req.buf, buf_len);
        return 0;
    }
    return -1;
}

int demuxer_sub_lang(demuxer_t *d, int id, char *buf, int buf_len)
{
    struct stream_lang_req req;
    sh_sub_t *sh;
    if (id < 0 || id >= MAX_S_STREAMS) {
        return -1;
    }
    sh = d->s_streams[id];
    if (sh && sh->lang) {
        av_strlcpy(buf, sh->lang, buf_len);
        return 0;
    }
    req.type = stream_ctrl_sub;
    // assume 1:1 mapping so we can show the language of
    // DVD subs even when we have not yet created the stream.
    req.id = sh ? sh->sid : id;
    if (stream_control(d->stream, STREAM_CTRL_GET_LANG, &req) == STREAM_OK) {
        av_strlcpy(buf, req.buf, buf_len);
        return 0;
    }
    return -1;
}

int demuxer_audio_track_by_lang(demuxer_t *d, char *lang)
{
    int i, len;
    lang += strspn(lang, ",");
    while ((len = strcspn(lang, ",")) > 0) {
        for (i = 0; i < MAX_A_STREAMS; ++i) {
            sh_audio_t *sh = d->a_streams[i];
            if (sh && sh->lang && strncmp(sh->lang, lang, len) == 0) {
                return sh->aid;
            }
        }
        lang += len;
        lang += strspn(lang, ",");
    }
    return -1;
}

int demuxer_sub_track_by_lang(demuxer_t *d, char *lang)
{
    int i, len;
    lang += strspn(lang, ",");
    while ((len = strcspn(lang, ",")) > 0) {
        for (i = 0; i < MAX_S_STREAMS; ++i) {
            sh_sub_t *sh = d->s_streams[i];
            if (sh && sh->lang && strncmp(sh->lang, lang, len) == 0) {
                return sh->sid;
            }
        }
        lang += len;
        lang += strspn(lang, ",");
    }
    return -1;
}

int demuxer_default_audio_track(demuxer_t *d)
{
    int i;
    for (i = 0; i < MAX_A_STREAMS; ++i) {
        sh_audio_t *sh = d->a_streams[i];
        if (sh && sh->default_track) {
            return sh->aid;
        }
    }
    for (i = 0; i < MAX_A_STREAMS; ++i) {
        sh_audio_t *sh = d->a_streams[i];
        if (sh) {
            return sh->aid;
        }
    }
    return -1;
}

int demuxer_default_sub_track(demuxer_t *d)
{
    int i;
    for (i = 0; i < MAX_S_STREAMS; ++i) {
        sh_sub_t *sh = d->s_streams[i];
        if (sh && sh->default_track) {
            return sh->sid;
        }
    }
    return -1;
}

int demuxer_disable_track(demuxer_t *demuxer, int stream_id)
{
    int ret = demux_control(demuxer,
                            DEMUXER_CTRL_DISABLE_TRACK, (void *) &stream_id);

    if (ret <= DEMUXER_CTRL_NOTIMPL) {
        MLOGD("Demux not support disable track\n");
        return MT_FALSE;
    }

    return MT_TRUE;
}
