/*
 * Demultiplexer for MPEG2 Transport Streams.
 *
 * Written by Nico <nsabbi@libero.it>
 * Kind feedback is appreciated; 'sucks' and alike is not.
 * Originally based on demux_pva.c written by Matteo Giani and FFmpeg (libavformat) sources
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

#ifdef __LINUX__
#include <stdio.h>
#include <stdlib.h>
#else
#include "mp_func_trans.h"
#endif
#include <string.h>

#define MODULE_TAG "TS"
#include "mt_type.h"
#include "mutil.h"
#include "mlog.h"
#include "config.h"
#include "mp_msg.h"
#include "mpcommon.h"
#include "help_mp.h"
#include "file_playback_sequence.h"

#include "libmpcodecs/dec_audio.h"
#include "stream/stream.h"
#include "demuxer.h"
#include "parse_es.h"
#include "stheader.h"
#include "ms_hdr.h"
#include "mpeg_hdr.h"
#include "mtos_task.h"
#include "mtos_misc.h"
#include "demux_comm.h"
#include "demux_ts.h"

#ifdef CFG_ENABLE_FFMPEG_422
#include "libavutil/channel_layout.h"
#else
#include "libavutil/audioconvert.h"
#endif
#include "libavutil/samplefmt.h"
#include "mtos_printk.h"

#define MAX_CHECK_SIZE                65535
#define NUM_CONSECUTIVE_TS_PACKETS     32
#define NUM_CONSECUTIVE_AUDIO_PACKETS 348

#define TYPE_AUDIO 1
#define TYPE_VIDEO 2
#define TYPE_SUB   3

#define SAMPE_PTS_IN_MULTI_FRAME_OF_ONE_GOP         5//means multi frames in one gop, and only find one pts, in another world, the gop share the same pts
#define TRY_STEP (10*1024*1024)
#define SYNC_SIZE (4*204)
#define REF_BITRATE (188*256)
#define REF_MAX_CNT 100
#define REF_MAX_CNT1 100

typedef struct {
    int32_t atype, vtype, stype;    //types
    int32_t apid, vpid, spid;   //stream ids
    char alang[4];  //languages
    uint16_t prog;
    off_t probe;
} tsdemux_init_t;

static void demux_seek_ts(demuxer_t *demuxer, float rel_seek_secs, float audio_delay, int flags);
static int IS_AUDIO(es_stream_type_t type)
{
    switch (type) {
        case AUDIO_MP2_MP:
        case AUDIO_A52_MP:
        case AUDIO_AC4_MP:
        case AUDIO_LPCM_BE_MP:
        case AUDIO_PCM_BR_MP:
        case AUDIO_AV3A_MP:
        case AUDIO_AAC_MP:
        case AUDIO_AAC_LATM_MP:
        case AUDIO_DTS_MP:
        case AUDIO_TRUEHD_MP:
        case AUDIO_TRUEHD_AC3_MP:
        case AUDIO_S302M_MP:
            return 1;
    }
    return 0;
}

static int IS_VIDEO(es_stream_type_t type)
{
    switch (type) {
        case VIDEO_MPEG1_MP:
        case VIDEO_MPEG2_MP:
        case VIDEO_MPEG4_MP:
        case VIDEO_H264_MP:
        case VIDEO_AVC_MP:
        case VIDEO_DIRAC_MP:
        case VIDEO_HEVC_MP:
        case VIDEO_VC1_MP:
        case VIDEO_AVS_MP:
        case VIDEO_AVS2_MP:
            return 1;
    }
    return 0;
}

static int IS_SUB(es_stream_type_t type)
{
    switch (type) {
        case SPU_DVD_MP:
        case SPU_DVB_MP:
        case SPU_PGS_MP:
        case SPU_TELETEXT_MP:
            return 1;
    }
    return 0;
}

static int ts_parse(demuxer_t *demuxer, ES_stream_t *es, unsigned char *packet, int probe);

static uint8_t get_packet_size(const unsigned char *buf, int size)
{
    int i;

    if (size < (TS_FEC_PACKET_SIZE * NUM_CONSECUTIVE_TS_PACKETS)) {
        return 0;
    }

    for (i = 0; i < NUM_CONSECUTIVE_TS_PACKETS; i++) {
        if (buf[i * TS_PACKET_SIZE] != 0x47) {
            MLOGA("GET_PACKET_SIZE, pos %d, char: %2x\n", i, buf[i * TS_PACKET_SIZE]);
            goto try_fec;
        }
    }
    return TS_PACKET_SIZE;

try_fec:
    for (i = 0; i < NUM_CONSECUTIVE_TS_PACKETS; i++) {
        if (buf[i * TS_FEC_PACKET_SIZE] != 0x47) {
            MLOGA("GET_PACKET_SIZE, pos %d, char: %2x\n", i, buf[i * TS_PACKET_SIZE]);
            goto try_philips;
        }
    }
    return TS_FEC_PACKET_SIZE;

try_philips:
    for (i = 0; i < NUM_CONSECUTIVE_TS_PACKETS; i++) {
        if (buf[i * TS_PH_PACKET_SIZE] != 0x47) {
            return 0;
        }
    }
    return TS_PH_PACKET_SIZE;
}

static int parse_avc_sps(uint8_t *buf, int len, int *w, int *h);
static uint8_t *pid_lang_from_pmt(ts_priv_t *priv, int pid);

static void ts_add_stream(demuxer_t *demuxer, ES_stream_t *es)
{
    int i;
    ts_priv_t *priv = (ts_priv_t *) demuxer->priv;

    if (priv->ts.streams[es->pid].sh) {
        return;
    }

    if ((IS_AUDIO(es->type) || IS_AUDIO(es->subtype)) && priv->last_aid + 1 < MAX_A_STREAMS) {
        sh_audio_t *sh = new_sh_audio_aid(demuxer, priv->last_aid, es->pid, pid_lang_from_pmt(priv, es->pid));
        if (sh) {
            sh->needs_parsing = 1;
            sh->format = IS_AUDIO(es->type) ? es->type : es->subtype;
            sh->ds = demuxer->audio;

            priv->ts.streams[es->pid].id = priv->last_aid;
            priv->ts.streams[es->pid].sh = sh;
            priv->ts.streams[es->pid].type = TYPE_AUDIO;
            MLOGD("ADDED AUDIO PID %d, type: 0x%x stream n. %d\r\n", es->pid, sh->format, priv->last_aid);
            priv->last_aid++;

            if (es->extradata && es->extradata_len) {
                sh->wf = (WAVEFORMATEX *)malloc33(sizeof(*sh->wf) + es->extradata_len);
                if (sh->wf == NULL) {
                    return;
                }
                sh->wf->cbSize = es->extradata_len;
                memcpy(sh->wf + 1, es->extradata, es->extradata_len);
            }
        }
    }

    if ((IS_VIDEO(es->type) || IS_VIDEO(es->subtype)) && priv->last_vid + 1 < MAX_V_STREAMS) {
        sh_video_t *sh = new_sh_video_vid(demuxer, priv->last_vid, es->pid);
        if (sh) {
            sh->format = IS_VIDEO(es->type) ? es->type : es->subtype;
            sh->ds = demuxer->video;

            priv->ts.streams[es->pid].id = priv->last_vid;
            priv->ts.streams[es->pid].sh = sh;
            priv->ts.streams[es->pid].type = TYPE_VIDEO;
            MLOGD("ADDED VIDEO PID %d, type: 0x%x stream n. %d\r\n", es->pid, sh->format, priv->last_vid);
            priv->last_vid++;


            if (sh->format == VIDEO_AVC_MP && es->extradata && es->extradata_len) {
                int w = 0, h = 0;
                sh->bih = (BITMAPINFOHEADER *)calloc33(1, sizeof(*sh->bih) + es->extradata_len);
                sh->bih->biSize = sizeof(*sh->bih) + es->extradata_len;
                sh->bih->biCompression = sh->format;
                memcpy(sh->bih + 1, es->extradata, es->extradata_len);
                MLOGA("EXTRADATA(%d BYTES): \n", es->extradata_len);
                for (i = 0; i < es->extradata_len; i++) {
                    MLOGA("%02x ", (int) es->extradata[i]);
                }
                MLOGA("\n");
                if (parse_avc_sps(es->extradata, es->extradata_len, &w, &h)) {
                    sh->bih->biWidth = w;
                    sh->bih->biHeight = h;
                }
            }
        }
    }

    if (IS_SUB(es->type) && priv->last_sid + 1 < MAX_S_STREAMS) {
        sh_sub_t *sh = new_sh_sub_sid(demuxer, priv->last_sid, es->pid, pid_lang_from_pmt(priv, es->pid));
        if (sh) {
            switch (es->type) {
                case SPU_DVB_MP:
                    sh->type = 'b';
                    break;
                case SPU_DVD_MP:
                    sh->type = 'v';
                    break;
                case SPU_PGS_MP:
                    sh->type = 'p';
                    break;
                case SPU_TELETEXT_MP:
                    sh->type = 'd';
                    break;
            }
            priv->ts.streams[es->pid].id = priv->last_sid;
            priv->ts.streams[es->pid].sh = sh;
            priv->ts.streams[es->pid].type = TYPE_SUB;
            priv->last_sid++;
        }
    }
}

static int ts_check_file(demuxer_t *demuxer)
{
    int _read;
    const int buf_size = (TS_FEC_PACKET_SIZE * NUM_CONSECUTIVE_TS_PACKETS);
    //unsigned char buf[TS_FEC_PACKET_SIZE * NUM_CONSECUTIVE_TS_PACKETS], done = 0, *ptr;
    unsigned char *buf, done = 0, *ptr;
    uint32_t i, count = 0, is_ts;
    //int cc[NB_PID_MAX], last_cc[NB_PID_MAX], pid, cc_ok, c, good, bad;
    int *cc, *last_cc, pid, cc_ok, c, good, bad;
    uint8_t size = 0;
    off_t pos = 0;
    off_t init_pos;

    MLOGD("Checking for MPEG-TS...\n");
    buf = (unsigned char *)malloc33(TS_FEC_PACKET_SIZE * NUM_CONSECUTIVE_TS_PACKETS);
    if (buf == NULL) {
        return 0;
    }
    cc = (int *)malloc33(NB_PID_MAX * sizeof(int));
    if (cc == NULL) {
        SAFEFREE(buf);
        return 0;
    }
    last_cc = (int *)malloc33(NB_PID_MAX * sizeof(int));
    if (last_cc == NULL) {
        SAFEFREE(buf);
        SAFEFREE(cc);
        return 0;
    }

    init_pos = stream_tell(demuxer->stream);
    is_ts = 0;
    while (! done && (is_file_seq_exit() == MT_FALSE)) {
        i = 1;
        c = 0;

        while (((c = stream_read_char(demuxer->stream)) != 0x47 && (is_file_seq_exit() == MT_FALSE))
               && (c >= 0)
               && (i < MAX_CHECK_SIZE)
               && ! demuxer->stream->eof
              ) {
            i++;
        }


        if (c != 0x47) {
            MLOGD("THIS DOESN'T LOOK LIKE AN MPEG-TS FILE!\n");
            is_ts = 0;
            done = 1;
            continue;
        }

        pos = stream_tell(demuxer->stream) - 1;
        buf[0] = c;
        _read = stream_read(demuxer->stream, &buf[1], buf_size - 1);

        if (_read < buf_size - 1) {
            MLOGD("COULDN'T READ ENOUGH DATA, EXITING TS_CHECK\n");
            //stream_reset(demuxer->stream);
            // mtos_printk("\n%s %d\n",__func__,__LINE__);
            SAFEFREE(buf);
            SAFEFREE(cc);
            SAFEFREE(last_cc);
            return 0;
        }

        size = get_packet_size(buf, buf_size);
        if (size) {
            done = 1;
            is_ts = 1;
        }

        if (pos - init_pos >= MAX_CHECK_SIZE) {
            done = 1;
            is_ts = 0;
        }
    }

    MLOGD("TRIED UP TO POSITION %"PRIu64", FOUND %x, packet_size= %d, SEEMS A TS? %d\n", (uint64_t) pos, c, size, is_ts);
    stream_seek(demuxer->stream, pos);

    if (! is_ts) {
        SAFEFREE(buf);
        SAFEFREE(cc);
        SAFEFREE(last_cc);

        return 0;
    }
    //LET'S CHECK continuity counters
    good = bad = 0;
    for (count = 0; count < NB_PID_MAX; count++) {
        cc[count] = last_cc[count] = -1;
    }

    for (count = 0; count < NUM_CONSECUTIVE_TS_PACKETS; count++) {
        ptr = &(buf[size * count]);
        pid = ((ptr[1] & 0x1f) << 8) | ptr[2];
        //MLOGA("BUF: %02x %02x %02x %02x, PID %d, SIZE: %d \n",
        //ptr[0], ptr[1], ptr[2], ptr[3], pid, size);

        if ((pid == 8191) || (pid < 16)) {
            continue;
        }

        cc[pid] = (ptr[3] & 0xf);
        cc_ok = (last_cc[pid] < 0) || ((((last_cc[pid] + 1) & 0x0f) == cc[pid]));
        MLOGA("PID %d, COMPARE CC %d AND LAST_CC %d\n", pid, cc[pid], last_cc[pid]);
        if (! cc_ok)
            //return 0;
        {
            bad++;
        } else {
            good++;
        }

        last_cc[pid] = cc[pid];
    }
    SAFEFREE(buf);
    SAFEFREE(cc);
    SAFEFREE(last_cc);
    MLOGD("GOOD CC: %d, BAD CC: %d\n", good, bad);

    if (good >= bad) {
        return size;
    } else {
        return 0;
    }
}

static int32_t progid_idx_in_pmt(ts_priv_t *priv, uint16_t progid)
{
    int x;

    if (priv->pmt == NULL) {
        return -1;
    }

    for (x = 0; x < priv->pmt_cnt; x++) {
        if (priv->pmt[x].progid == progid) {
            return x;
        }
    }

    return -1;
}


static int32_t progid_for_pid(ts_priv_t *priv, int pid, int32_t req)        //finds the first program listing a pid
{
    int i, j;
    pmt_t *pmt;


    if (priv->pmt == NULL) {
        return -1;
    }


    for (i = 0; i < priv->pmt_cnt; i++) {
        pmt = &(priv->pmt[i]);

        if (pmt->es == NULL) {
            return -1;
        }

        for (j = 0; j < pmt->es_cnt; j++) {
            if (pmt->es[j].pid == pid) {
                if ((req == 0) || (req == pmt->progid)) {
                    return pmt->progid;
                }
            }
        }

    }
    return -1;
}

static int32_t prog_pcr_pid(ts_priv_t *priv, int progid)
{
    int i;

    if (priv->pmt == NULL) {
        return -1;
    }
    for (i = 0; i < priv->pmt_cnt; i++) {
        if (priv->pmt[i].progid == progid) {
            return priv->pmt[i].PCR_PID;
        }
    }
    return -1;
}


static int pid_match_lang(ts_priv_t *priv, uint16_t pid, char *lang)
{
    uint16_t i, j;
    pmt_t *pmt;

    if (priv->pmt == NULL) {
        return -1;
    }

    for (i = 0; i < priv->pmt_cnt; i++) {
        pmt = &(priv->pmt[i]);

        if (pmt->es == NULL) {
            return -1;
        }

        for (j = 0; j < pmt->es_cnt; j++) {
            if (pmt->es[j].pid != pid) {
                continue;
            }

            MLOGD("CMP LANG %s AND %s, pids: %d %d\n", pmt->es[j].lang, lang, pmt->es[j].pid, pid);
            if (strncmp(pmt->es[j].lang, lang, 3) == 0) {
                return 1;
            }
        }
    }

    return -1;
}

//second stage: returns the count of A52 syncwords found
static int a52_check(char *buf, int len)
{
    int cnt, frame_length = 0, ok, srate;

    cnt = ok = 0;
    if (len < 8) {
        return 0;
    }

    while (cnt < len - 7) {
        if (buf[cnt] == 0x0B && buf[cnt + 1] == 0x77) {
            frame_length = mp_a52_framesize(&buf[cnt], &srate);
            if (frame_length >= 7 && frame_length <= 3840) {
                cnt += frame_length;
                ok++;
            } else {
                cnt++;
            }
        } else {
            cnt++;
        }
    }

    MLOGD("A52_CHECK(%d input bytes), found %d frame syncwords of %d bytes length\n", len, ok, frame_length);
    return ok;
}


static off_t ts_detect_streams(demuxer_t *demuxer, tsdemux_init_t *param)
{
    int video_found = 0, audio_found = 0, i, num_packets = 0, req_apid, req_vpid, req_spid;
    int is_audio, is_video, is_sub, has_tables;
    int32_t p, chosen_pid = 0;
    off_t pos = 0, ret = 0, init_pos, end_pos;
    ES_stream_t es;
    unsigned char tmp[TS_FEC_PACKET_SIZE];
    ts_priv_t *priv = (ts_priv_t *) demuxer->priv;
    struct {
        char *buf;
        int pos;
    } pes_priv, *pes_priv1, *pptr;
    char *tmpbuf;
    unsigned int cur_ticks = 0, start_ticks = 0;


    priv->last_pid = 8192;      //invalid pid

    req_apid = param->apid;
    req_vpid = param->vpid;
    req_spid = param->spid;

    pes_priv1 = malloc33(8192 * sizeof(pes_priv));
    if (pes_priv1 == NULL) {
        return ret;
    }
    has_tables = 0;
    memset(pes_priv1, 0, 8192 * sizeof(pes_priv));
    init_pos = stream_tell(demuxer->stream);
    MLOGD("PROBING UP TO %"PRIu64", PROG: %d\n", (uint64_t) param->probe, param->prog);
    end_pos = init_pos + (param->probe ? param->probe : TS_MAX_PROBE_SIZE);

    start_ticks = mtos_ticks_get();
    while (is_file_seq_exit() == MT_FALSE) {
        cur_ticks = mtos_ticks_get();
        if (cur_ticks >= start_ticks) {
            cur_ticks -= start_ticks;
        } else {
            cur_ticks = start_ticks - cur_ticks;
        }
        /* 3 seconds */
        if (cur_ticks >= 300) {
            break;
        }

        pos = stream_tell(demuxer->stream);
        if (pos > end_pos || demuxer->stream->eof) {
            break;
        }
        if (ts_parse(demuxer, &es, tmp, 1) == 1) {
            //Non PES-aligned A52 audio may escape detection if PMT is not present;
            //in this case we try to find at least 3 A52 syncwords
            if ((es.type == PES_PRIVATE1_MP) && (! audio_found) && req_apid > -2) {
                pptr = &pes_priv1[es.pid];
                if (pptr->pos < 64 * 1024) {
                    tmpbuf = (char *)realloc33(pptr->buf, pptr->pos + es.size);
                    if (tmpbuf != NULL) {
                        pptr->buf = tmpbuf;
                        memcpy(&(pptr->buf[ pptr->pos ]), es.start, es.size);
                        pptr->pos += es.size;
                        if (a52_check(pptr->buf, pptr->pos) > 2) {
                            param->atype = AUDIO_A52_MP;
                            param->apid = es.pid;
                            es.type = AUDIO_A52_MP;
                        }
                    }
                }
            }

            is_audio = IS_AUDIO(es.type) || ((es.type == SL_PES_STREAM_MP) && IS_AUDIO(es.subtype));
            is_video = IS_VIDEO(es.type) || ((es.type == SL_PES_STREAM_MP) && IS_VIDEO(es.subtype));
            is_sub   = IS_SUB(es.type);


            if ((! is_audio) && (! is_video) && (! is_sub)) {
                continue;
            }
            if (is_audio && req_apid == -2) {
                continue;
            }

            if (is_video) {
                chosen_pid = (req_vpid == es.pid);
                if ((! chosen_pid) && (req_vpid > 0)) {
                    continue;
                }
            } else if (is_audio) {
                if (req_apid > 0) {
                    chosen_pid = (req_apid == es.pid);
                    if (! chosen_pid) {
                        continue;
                    }
                } else if (param->alang[0] > 0 && es.lang[0] > 0) {
                    if (pid_match_lang(priv, es.pid, param->alang) == -1) {
                        continue;
                    }

                    chosen_pid = 1;
                    param->apid = req_apid = es.pid;
                }
            } else if (is_sub) {
                chosen_pid = (req_spid == es.pid);
                if ((! chosen_pid) && (req_spid > 0)) {
                    continue;
                }
            }

            if (req_apid < 0 && (param->alang[0] == 0) && req_vpid < 0 && req_spid < 0) {
                chosen_pid = 1;
            }

            if ((ret == 0) && chosen_pid) {
                ret = stream_tell(demuxer->stream);
            }

            p = progid_for_pid(priv, es.pid, param->prog);
            if (p != -1) {
                has_tables++;
                if (!param->prog && chosen_pid) {
                    param->prog = p;
                }
            }

            if ((param->prog > 0) && (param->prog != p)) {
                if (audio_found) {
                    if (is_video && (req_vpid == es.pid)) {
                        param->vtype = IS_VIDEO(es.type) ? es.type : es.subtype;
                        param->vpid = es.pid;
                        video_found = 1;
                        break;
                    }
                }

                if (video_found) {
                    if (is_audio && (req_apid == es.pid)) {
                        param->atype = IS_AUDIO(es.type) ? es.type : es.subtype;
                        param->apid = es.pid;
                        audio_found = 1;
                        break;
                    }
                }
                continue;
            }

            MLOGA("TYPE: %x, PID: %d, PROG FOUND: %d\n", es.type, es.pid, param->prog);

            if (is_video) {
                if ((req_vpid == -1) || (req_vpid == es.pid)) {
                    param->vtype = IS_VIDEO(es.type) ? es.type : es.subtype;
                    param->vpid = es.pid;
                    video_found = 1;
                }
            }


            if (((req_vpid == -2) || (num_packets >= NUM_CONSECUTIVE_AUDIO_PACKETS)) && audio_found && !param->probe) {
                //novideo or we have at least 348 audio packets (64 KB) without video (TS with audio only)
                param->vtype = 0;
                break;
            }

            if (is_sub) {
                if ((req_spid == -1) || (req_spid == es.pid)) {
                    param->stype = es.type;
                    param->spid = es.pid;
                }
            }

            if (is_audio) {
                if ((req_apid == -1) || (req_apid == es.pid)) {
                    param->atype = IS_AUDIO(es.type) ? es.type : es.subtype;
                    param->apid = es.pid;
                    audio_found = 1;
                    req_apid = es.pid;//to make sure choose the first audio.feyang fix bug 133379
                }
            }

            if (audio_found && (param->apid == es.pid) && (! video_found)) {
                num_packets++;
            }

            if ((has_tables == 0) && (video_found && audio_found) && (pos >= 2000000)) {
                break;
            }
        }
    }

    for (i = 0; i < 8192; i++) {
        if (pes_priv1[i].buf != NULL) {
            SAFEFREE(pes_priv1[i].buf);
            pes_priv1[i].buf = NULL;
            pes_priv1[i].pos = 0;
        }
    }
    SAFEFREE(pes_priv1);

    if (video_found) {
        if (param->vtype == VIDEO_MPEG1_MP) {
            MLOGD("VIDEO MPEG1(pid=%d) ", param->vpid);
        } else if (param->vtype == VIDEO_MPEG2_MP) {
            MLOGD("VIDEO MPEG2(pid=%d) ", param->vpid);
        } else if (param->vtype == VIDEO_MPEG4_MP) {
            MLOGD("VIDEO MPEG4(pid=%d) ", param->vpid);
        } else if (param->vtype == VIDEO_H264_MP) {
            MLOGD("VIDEO H264(pid=%d) ", param->vpid);
        } else if (param->vtype == VIDEO_VC1_MP) {
            MLOGD("VIDEO VC1(pid=%d) ", param->vpid);
        } else if (param->vtype == VIDEO_AVC_MP) {
            MLOGD("VIDEO AVC(NAL-H264, pid=%d) ", param->vpid);
        } else if (param->vtype == VIDEO_HEVC_MP) {
            MLOGD("VIDEO HEVC(pid=%d) ", param->vpid);
        }
    } else {
        param->vtype = UNKNOWN_MP;
        //WE DIDN'T MATCH ANY VIDEO STREAM
        MLOGD("NO VIDEO! ");
    }

    if (param->atype == AUDIO_MP2_MP) {
        MLOGD("AUDIO MPA(pid=%d)", param->apid);
    } else if (param->atype == AUDIO_A52_MP) {
        MLOGD("AUDIO A52(pid=%d)", param->apid);
    } else if (param->atype == AUDIO_DTS_MP) {
        MLOGD("AUDIO DTS(pid=%d)", param->apid);
    } else if (param->atype == AUDIO_LPCM_BE_MP) {
        MLOGD("AUDIO LPCM(pid=%d)", param->apid);
    } else if (param->atype == AUDIO_PCM_BR_MP) {
        MLOGD("AUDIO PCMBR(pid=%d)", param->apid);
    } else if (param->atype == AUDIO_AAC_MP) {
        MLOGD("AUDIO AAC(pid=%d)", param->apid);
    } else if (param->atype == AUDIO_AAC_LATM_MP) {
        MLOGD("AUDIO AAC LATM(pid=%d)", param->apid);
    } else if (param->atype == AUDIO_TRUEHD_MP) {
        MLOGD("AUDIO TRUEHD(pid=%d)", param->apid);
    } else if (param->atype == AUDIO_S302M_MP) {
        MLOGD("AUDIO S302M(pid=%d)", param->apid);
    } else {
        audio_found = 0;
        param->atype = UNKNOWN_MP;
        //WE DIDN'T MATCH ANY AUDIO STREAM, SO WE FORCE THE DEMUXER TO IGNORE AUDIO
        MLOGW("NO AUDIO! (try increasing -tsprobe)");
    }

    if (IS_SUB(param->stype)) {
        MLOGD(" SUB %s(pid=%d) ", (param->stype == SPU_DVD_MP ? "DVD" : param->stype == SPU_DVB_MP ? "DVB" : "Teletext"), param->spid);
    } else {
        param->stype = UNKNOWN_MP;
        MLOGD(" NO SUBS (yet)! ");
    }

    if (video_found || audio_found) {
        if (!param->prog) {
            p = progid_for_pid(priv, video_found ? param->vpid : param->apid, 0);
            if (p != -1) {
                param->prog = p;
            }
        }

        if (demuxer->stream->eof && (ret == 0)) {
            ret = init_pos;
        }
        MLOGD(" PROGRAM N. %d\n", param->prog);
    } else {
        MLOGD("\n");
    }


    for (i = 0; i < NB_PID_MAX; i++) {
        if (priv->ts.pids[i] != NULL) {
            priv->ts.pids[i]->payload_size = 0;
            priv->ts.pids[i]->pts = priv->ts.pids[i]->last_pts = 0;
            priv->ts.pids[i]->last_cc = -1;
            priv->ts.pids[i]->is_synced = 0;
        }
    }
    //MLOGD("[%s] time3=[%d]\n",__func__,cur_ticks);

    if (priv->pmt_cnt > 0) {
        int k = 0;
        int isvideo = 0;
        int isaudio = 0;
        for (i = 0; i < priv->pmt_cnt; i++) {
            pmt_t *pmt = &priv->pmt[i];
            param->prog = pmt->progid;
            isvideo = 0;
            isaudio = 0;
            for (k = 0; k < pmt->es_cnt; k++) {
                if (IS_VIDEO(pmt->es[k].type) && isvideo == 0) {
                    isvideo = 1;
                    priv->selected_vpid = pmt->es[k].pid;
                    param->vtype = pmt->es[k].type;
                    param->vpid = pmt->es[k].pid;
                } else if (IS_AUDIO(pmt->es[k].type) && isaudio == 0) {
                    isaudio = 1;
                    priv->selected_apid = pmt->es[k].pid;
                    param->apid = pmt->es[k].pid;
                    param->atype = pmt->es[k].type;
                } else if (IS_SUB(pmt->es[k].type)) {
                    param->stype = pmt->es[k].type;
                    param->spid = pmt->es[k].pid;
                }
            }
            if (isvideo && isaudio) {
                MLOGI("[%s:%d] prog:%d,vpid:0x%x,apid:0x%x\n", __func__, __LINE__, param->prog, param->vpid, param->apid);
                break;
            }
        }
    }
    return ret;
}

static int parse_avc_sps(uint8_t *buf, int len, int *w, int *h)
{
    int sps, sps_len;
    unsigned char *ptr;
    mp_mpeg_header_t picture;
    if (len < 6) {
        return 0;
    }
    sps = buf[5] & 0x1f;
    if (!sps) {
        return 0;
    }
    sps_len = (buf[6] << 8) | buf[7];
    if (!sps_len || (sps_len > len - 8)) {
        return 0;
    }
    ptr = &(buf[8]);
    picture.display_picture_width = picture.display_picture_height = 0;
    h264_parse_sps(&picture, ptr, len - 8);
    if (!picture.display_picture_width || !picture.display_picture_height) {
        return 0;
    }
    *w = picture.display_picture_width;
    *h = picture.display_picture_height;
    return 1;
}
static int ts_sync(stream_t *stream)
{

    while (!stream->eof && (is_file_seq_exit() == MT_FALSE))
        if (stream_read_char(stream) == 0x47) {
            return 1;
        }

    return 0;
}

static int ts_parse_timestamp(
    const unsigned char *buf, const int buf_size, ES_stream_t *es)
{
    long long pts, dts;
    int buf_remains      = buf_size;
    unsigned int flags   = buf[0];
    unsigned int has_pts = MT_SUCCESS;
    const unsigned int PTS_DTS_START_BIT = 2;
    const unsigned int PTS_DTS_FIELD_LEN = 5;

    es->pts = 0.0;
    es->dts = MP_NOPTS_VALUE;
    pts = dts = MP_NOPTS_VALUE;
    buf_remains -= PTS_DTS_START_BIT;
    if ((flags & 0xc0) == 0x80) {
        if (buf_remains >= PTS_DTS_FIELD_LEN) {
            pts = dts = demux_parse_mpeg2_pts(&buf[buf_size - buf_remains]);
        }
    } else if ((flags & 0xc0) == 0xc0) {
        if (buf_remains >= PTS_DTS_FIELD_LEN) {
            pts = demux_parse_mpeg2_pts(&buf[buf_size - buf_remains]);
        }
        buf_remains -= PTS_DTS_FIELD_LEN;
        if (buf_remains >= PTS_DTS_FIELD_LEN) {
            dts = demux_parse_mpeg2_pts(&buf[buf_size - buf_remains]);
        }
    } else {
        has_pts = MT_FAILURE;
    }

    if (MP_NOPTS_VALUE != pts) {
        es->pts      = pts / 90000.0;
        es->pts31bit = pts >> 2;
    }

    if (MP_NOPTS_VALUE != dts) {
        es->dts = dts * 1000 / 90;
    }

    return has_pts;
}
// 0 = EOF or no stream found
// else = [-] number of bytes written to the packet
static int ts_parse_pts(demuxer_t *demuxer, ES_stream_t *es, unsigned char *packet, int backward)
{
    int afc, buf_size, is_start, pid, req_pid;
    ts_priv_t *priv = (ts_priv_t *) demuxer->priv;
    stream_t *stream = demuxer->stream;
    unsigned char *p;
    int32_t ts_error;
    int junk   = 0;
    int ts_cnt = 0;
    off_t filepos = 0;

    junk = priv->ts.packet_size - TS_PACKET_SIZE;
    req_pid = es->pid;
    if (!ts_sync(stream)) {
        return MT_FALSE;
    }

    stream_skip(stream, -1);
    memset(es, 0, sizeof(*es));
    while ((is_file_seq_exit() == MT_FALSE)) {
        filepos = stream_tell(stream);
        ts_cnt++;
        ts_error = 0;
        if (ts_cnt > 20000 ||
            (backward && filepos <= demuxer->movi_start) ||
            (!backward && filepos >= demuxer->movi_end)) {
            return MT_FALSE;
        }

        buf_size = priv->ts.packet_size - junk;
        stream_read(stream, &packet[0], priv->ts.packet_size);
        if (packet[0] != 0x47) {
            if (!ts_sync(stream)) {
                return MT_FALSE;
            }
            continue;
        }

        buf_size -= 4;
        if ((packet[1]  >> 7) & 0x01) { //transport error
            ts_error = 1;
        }

        is_start = packet[1] & 0x40;
        pid = ((packet[1] & 0x1f) << 8) | packet[2];
        if (ts_error) {
            is_start = 0;   //queued to the packet data
        }

        if (!is_start || (pid != req_pid)) {    //invalid pid
            if (backward) {
                stream_skip(stream, -2 * priv->ts.packet_size);
            }
            continue;
        }
        es->pid = pid;
        afc = (packet[3] >> 4) & 3;
        if (!(afc % 2)) { //no payload in this TS packet
            if (backward) {
                stream_skip(stream, -2 * priv->ts.packet_size);
            }
            continue;
        }//184

        if (afc > 1) {
            buf_size--;//183

            if (packet[4] > 0) {
                buf_size--;//182

                buf_size -= packet[4] - 1;
                if (buf_size == 0) { //176
                    if (backward) {
                        stream_skip(stream, -2 * priv->ts.packet_size);
                    }
                    continue;
                }
            }
        }
        if (is_start) {
            if (buf_size == 0 || buf_size > 184) {
                return MT_FALSE;
            }

            p = &packet[188 - buf_size];
            if (p[0] || p[1] || (p[2] != 1)) {
                continue;
            }

            buf_size -= 6;
            if (buf_size == 0) {
                return MT_FALSE;
            }

            es->payload_size = (p[4] << 8 | p[5]);
            if (MT_SUCCESS == ts_parse_timestamp(&p[7], buf_size, es)) {
                return MT_TRUE;
            }
        }
        if (backward) {
            stream_skip(stream, -2 * priv->ts.packet_size);
        }
        continue;
    }

    return MT_FALSE;
}

static int ts_parse_pts_1(demuxer_t *demuxer, ES_stream_t *es, unsigned char *packet, int backward)
{
    int buf_size, is_start, pid, req_pid;
    int len, afc;
    ts_priv_t *priv = (ts_priv_t *) demuxer->priv;
    stream_t *stream = demuxer->stream;
    unsigned char *p;
    int32_t ts_error;
    int junk = 0;
    int64_t filepos = 0;
    int step_len;
    int i = 0;
    unsigned char *rd_ptr = NULL;
    junk = priv->ts.packet_size - TS_PACKET_SIZE;
    req_pid = es->pid;

    if (! ts_sync(stream)) {
        return -1;
    }
    stream_skip(stream, -1);
    memset(es, 0, sizeof(*es));
    filepos = stream_tell(stream);
    if (filepos < 0) {
        return -1;
    }
    step_len = (REF_BITRATE / priv->ts.packet_size) * priv->ts.packet_size;
    if (backward) {
        if (filepos > demuxer->movi_start) {
            if ((filepos - demuxer->movi_start) > step_len) {
                stream_seek(stream, filepos - step_len + priv->ts.packet_size);
                len = stream_read(stream, &packet[0], step_len);
            } else {
                stream_seek(stream, demuxer->movi_start + priv->ts.packet_size);
                len = stream_read(stream, &packet[0], (filepos - demuxer->movi_start) / priv->ts.packet_size * priv->ts.packet_size);
            }
        } else {
            return -1;
        }
    } else {
        if (filepos < demuxer->movi_end) {
            if ((demuxer->movi_end - filepos) > step_len) {
                len = stream_read(stream, &packet[0], step_len);
            } else {
                len = stream_read(stream, &packet[0], (demuxer->movi_end - filepos) / priv->ts.packet_size * priv->ts.packet_size);
            }
        } else {
            return -1;
        }
    }
    if (backward) {
        rd_ptr = packet + len - priv->ts.packet_size;
    } else {
        rd_ptr = packet;
    }

    MLOGA("s:%p e:%p len:%d packet_size:%d max:%d junk:%d\n",
        packet, rd_ptr, len, (int) (priv->ts.packet_size), (int) REF_BITRATE, junk);
    while ((is_file_seq_exit() == MT_FALSE)) {
        buf_size = priv->ts.packet_size - junk;
        ts_error = 0;
        if (backward) {
            if (rd_ptr > packet) {
                if (i != 0) {
                    rd_ptr -= priv->ts.packet_size;
                } else {
                    i++;
                }
            } else {
                break;
            }
        } else {
            if (rd_ptr < packet + len - 1) {
                if (i != 0) {
                    rd_ptr += priv->ts.packet_size;
                } else {
                    i++;
                }
            } else {
                break;
            }
        }
        if (rd_ptr < packet) {
            break;
        }

        if (rd_ptr[0] != 0x47) {
            continue;
        }
        buf_size -= 4;

        if ((rd_ptr[1]  >> 7) & 0x01) { //transport error
            ts_error = 1;
        }

        is_start = rd_ptr[1] & 0x40;
        pid = ((rd_ptr[1] & 0x1f) << 8) | rd_ptr[2];
        if (ts_error) {
            is_start = 0;   //queued to the packet data
        }

        if (!is_start || (pid != req_pid)) {    //invalid pid
            continue;
        }

        afc = (rd_ptr[3] >> 4) & 3;
        if (!(afc % 2)) { //no payload in this TS packet
            continue;
        }//184

        if (afc > 1) {
            buf_size--;//183

            //c==0 is allowed!
            if (rd_ptr[4] > 0) {
                buf_size--;//182
                buf_size -= rd_ptr[4] - 1;
                if (buf_size == 0) { //176
                    continue;
                }
            }
        }
        if (is_start) {
            if (buf_size == 0 || buf_size > 184) {
                return -1;
            }
            p = &rd_ptr[188 - buf_size];
            if (p[0] || p[1] || (p[2] != 1)) {
                return -1;
            }

            buf_size -= 6;
            if (buf_size == 0) {
                return -1;
            }

            es->payload_size = (p[4] << 8 | p[5]);
            if (MT_SUCCESS == ts_parse_timestamp(&p[7], buf_size, es)) {
                return (rd_ptr - packet);
            }
        }
        continue;
    }

    return -1;
}


static int ts_search_sync_byte(demuxer_t *demuxer, unsigned char *pkt, ts_priv_t *priv, off_t pos)
{
    //tizhang@20181128 for 105998
    stream_t *stream = demuxer->stream;
    off_t first_pos;
    off_t file_pos;
    int offset = -1;
    int sync_num = 3;
    int i;

    if (sync_num <= 0) {
        return 0;
    }
    stream_reset(demuxer->stream);
    //demux_seek_ts(demuxer,0,0,SEEK_ABSOLUTE);
    stream_seek(demuxer->stream, pos);
    file_pos = stream_tell(demuxer->stream);
    first_pos = file_pos;
    if ((first_pos + SYNC_SIZE) >= demuxer->movi_end) {
        return -1;
    }
    stream_read(stream, pkt, SYNC_SIZE);
    file_pos = first_pos + SYNC_SIZE - 180;
    while (file_pos < demuxer->movi_end && file_pos > (demuxer->movi_start + (priv->ts.packet_size * (sync_num - 1))) && \
           file_pos > (first_pos + priv->ts.packet_size * (sync_num - 1))) {
        for (i = 0; i < sync_num; i++) {
            if (pkt[file_pos - first_pos - priv->ts.packet_size * i] != 0x47) {
                break;
            }
        }
        if (i == sync_num) {
            offset = file_pos - first_pos;
            break;
        } else {
            offset = -1;
        }
        --file_pos;
    }
    return offset;
}

static int cal_bps_with_end2(demuxer_t *demuxer, ES_stream_t *es, int pid, ts_priv_t *priv, off_t start_pos)
{
    uint8_t tmp[204];
    double first_pts = 0;
    double last_pts = 0;
    int bps = 0;
    off_t filepos = 0 ;
    int64_t pos = ((demuxer->movi_end - demuxer->movi_start) / priv->ts.packet_size - 2) * priv->ts.packet_size
                  + demuxer->movi_start;
    ts_parse_pts(demuxer, es, (unsigned char *)(&tmp), 0);
    first_pts = es->pts;
    if (MP_NOPTS_VALUE != first_pts &&
        MP_NOPTS_VALUE == priv->file_first_pts) {
        priv->file_first_pts = first_pts;
    }

    stream_seek(demuxer->stream, pos);   //IF IT'S FROM A PIPE IT WILL FAIL, BUT WHO CARES?
    es->pid = pid;
    if (priv->ts.packet_size == 188) {
        while (stream_read_char(demuxer->stream) != 0x47 && pos > demuxer->movi_start && (is_file_seq_exit() == MT_FALSE)) {
            stream_skip(demuxer->stream, -10 * priv->ts.packet_size - 1);
            pos = pos - 10 * priv->ts.packet_size;
            if (pos < 0) {
                break;
            }
        }
        stream_skip(demuxer->stream, -1);
    }
    ts_parse_pts(demuxer, es, (unsigned char *)(&tmp), 1);
    last_pts = es->pts;
    filepos = stream_tell(demuxer->stream);
    bps = (int)(1.f * (filepos - start_pos) / (last_pts - first_pts));
    if (bps < 0) {
        bps = 1;
    }
    stream_reset(demuxer->stream);
    stream_seek(demuxer->stream, start_pos);    //IF IT'S FROM A PIPE IT WILL FAIL, BUT WHO CARES?
    return bps;
}
static int cal_bps_with_end(demuxer_t *demuxer, ES_stream_t *es, int pid, ts_priv_t *priv, off_t start_pos,
                            int *cal_time_interval, off_t *end_pos)
{
    double first_pts = 0;
    double next_pts = 0;
    uint8_t *tmp = NULL;
    int ret, cnt, cnt1;
    off_t tmp_pos;
    off_t stream_size = demuxer->movi_end - demuxer->movi_start;
    off_t first_pos = demuxer->movi_start;
    off_t next_pos = stream_size + demuxer->movi_start;

    stream_reset(demuxer->stream);
    stream_seek(demuxer->stream, first_pos);

    es->pid  = pid;
    tmp = (uint8_t *)malloc33(REF_BITRATE);
    if (tmp == NULL) {
        ret = MT_FAILURE;
        goto FAIL;
    }
    ret = ts_parse_pts(demuxer, es, (unsigned char *)tmp, 0);
    if (MT_FALSE == ret) {
        es->pts = MP_NOPTS_VALUE;
    }

    first_pts = es->pts;
    if (MP_NOPTS_VALUE != first_pts &&
        MP_NOPTS_VALUE == priv->file_first_pts) {
        priv->file_first_pts = first_pts;
    }

    first_pos = stream_tell(demuxer->stream);
    cnt = 0;
    cnt1 = 0;
    next_pos -= SYNC_SIZE + 1;
find_next_pts:
    memset(tmp, 0, REF_BITRATE);
    tmp_pos = ts_search_sync_byte(demuxer, (unsigned char *)tmp, priv, next_pos);
    if (tmp_pos == -1) {
        if (cnt < 20) {
            next_pos -= ((REF_BITRATE * ++cnt) / priv->ts.packet_size) * priv->ts.packet_size;
        } else if (cnt < 30) {
            next_pos -= ((REF_BITRATE * 5 * ++cnt) / priv->ts.packet_size) * priv->ts.packet_size;
        } else {
            next_pos -= ((REF_BITRATE * 10 * ++cnt) / priv->ts.packet_size) * priv->ts.packet_size;
        }
        if (next_pos <= demuxer->movi_start || cnt1 >= REF_MAX_CNT) { //tizhang@20180827 change cnt from 5 to 50 to fix 103498
            ret = MT_FAILURE;
            goto FAIL;
        }
        goto find_next_pts;
    } else {
        next_pos += tmp_pos;
    }

    stream_reset(demuxer->stream);
    stream_seek(demuxer->stream, next_pos);
    es->pid  = pid;
    es->pts  = 0;      // xingwei @20190117 for 107215
    ret = ts_parse_pts_1(demuxer, es, (unsigned char *)tmp, 1);
    next_pts = es->pts;
    if (next_pts <= first_pts || ret < 0) {
        if (cnt1 < 20) {
            next_pos -= ((REF_BITRATE * ++cnt1) / priv->ts.packet_size) * priv->ts.packet_size;
        } else if (cnt1 < 30) {
            next_pos -= ((REF_BITRATE * 5 * ++cnt1) / priv->ts.packet_size) * priv->ts.packet_size;
        } else {
            next_pos -= ((REF_BITRATE * 10 * ++cnt1) / priv->ts.packet_size) * priv->ts.packet_size;
        }
        if (next_pos <= demuxer->movi_start || cnt1 >= REF_MAX_CNT1) { //tizhang@20180827 change cnt from 5 to 50 to fix 103498
            ret = MT_FAILURE;
            goto FAIL;
        }
        goto find_next_pts;
    }

    next_pos = stream_tell(demuxer->stream);
    ret = (int)((next_pos + ret - first_pos) / (next_pts - first_pts));
    *end_pos = next_pos;
    *cal_time_interval = (int)((next_pts - first_pts) * 1000);

FAIL:
    if (tmp) {
        SAFEFREE(tmp);
    }
    MLOGI("first:%lf,%lld; last:%lf,%lld,dur:%dms\n",
          first_pts, first_pos, next_pts, next_pos, *cal_time_interval);
    stream_seek(demuxer->stream, first_pos);
    return ret;
}

static int try_to_cal_bps(demuxer_t *demuxer, ES_stream_t *es, int pid, ts_priv_t *priv)
{
    uint8_t tmp[204];
    double first_pts = 0;
    double next_pts = 0;
    int bps_try_step =  TRY_STEP;
    off_t stream_size = demuxer->movi_end - demuxer->movi_start;
    off_t first_pos = demuxer->movi_start;
    off_t next_pos = MIN(bps_try_step, stream_size / 4) + demuxer->movi_start;
    off_t filepos = 0 ;
    es->pid  = pid;
    stream_seek(demuxer->stream, first_pos);
    ts_parse_pts(demuxer, es, tmp, 0);
    first_pts = es->pts;
    if (MP_NOPTS_VALUE != first_pts &&
        MP_NOPTS_VALUE == priv->file_first_pts) {
        priv->file_first_pts = first_pts;
    }

    first_pos = stream_tell(demuxer->stream);
find_next_pts:
    stream_seek(demuxer->stream, next_pos);
    es->pid  = pid;
    ts_parse_pts(demuxer, es, tmp, 0);
    next_pts = es->pts;
    if (next_pts - first_pts > 1.f) {
        //find bps
        filepos = stream_tell(demuxer->stream);
        return (int)((filepos - first_pos) / (next_pts - first_pts));
    } else {
        //start fix bug 99368 ,when next_pts is 0,there will be
        //mis-caculate
        if (next_pts > first_pts) {
            first_pts = next_pts;
            first_pos = stream_tell(demuxer->stream);
        }
        //end bug 99368
        next_pos += (MIN(bps_try_step, stream_size / 4));

        if (next_pos >= demuxer->movi_end) {
            stream_reset(demuxer->stream);
            return 0;
        }

        goto find_next_pts;
    }

    return 0;
}

static int ts_check_scrambling_cnt(ts_priv_t *priv, stream_t *stream)
{
    int scrambled = 1;
    int sa_cnt = -1, sv_cnt = -1, nsa_cnt = -1, nsv_cnt = -1;
    if (priv->selected_apid >= 0 && priv->selected_apid < NB_PID_MAX) {
        ES_stream_t *esa = priv->ts.pids[priv->selected_apid];
        sa_cnt = esa->scrambled_cnt;
        nsa_cnt = esa->not_scrambled_cnt;
        if (0 == sa_cnt || nsa_cnt > sa_cnt) {
            scrambled = 0;
        }
    }

    if (priv->selected_vpid >= 0 && priv->selected_vpid < NB_PID_MAX) {
        ES_stream_t *esv = priv->ts.pids[priv->selected_vpid];
        sv_cnt = esv->scrambled_cnt;
        nsv_cnt = esv->not_scrambled_cnt;
        if (0 == sv_cnt || nsv_cnt > sv_cnt) {
            scrambled = 0;
        }
    }

    MLOGI("[%d %d] scrambled:[%d %d] not_scrambled:[%d %d] ret:%d\n",
        priv->selected_apid, priv->selected_vpid, sa_cnt, sv_cnt, nsa_cnt, nsv_cnt, scrambled);
    stream->transport_scrambling_control = scrambled;
    return scrambled;
}

static int get_stream_bps(demuxer_t *demuxer, tsdemux_init_t   *params, off_t start_pos,
                          ts_priv_t *priv, sh_video_t *sh_video, sh_audio_t *sh_audio)
{
    int cal_time_interval = 0;
    off_t end_pos = 0;

    if (fp_is_timeshift_file() == 1) {
        /* do nothing*/
    } else {
        MLOGI("%s%d\n", __func__, __LINE__);
        int video_bps = 0;
        int audio_bps = 0;
        int finally_bps = 0;

        ES_stream_t *es = (ES_stream_t *)calloc33(sizeof(ES_stream_t), 1);
        if (es == NULL) {
            return 0;
        }

        if (params->vtype != UNKNOWN_MP && sh_video) {
            /*
            *calculate the bps by video
            */
            es->pid = params->vpid;
            video_bps = cal_bps_with_end(demuxer, es, params->vpid, priv, start_pos, &cal_time_interval, &end_pos);
            MLOGI("%s----%d:video_bps = %d\n", __func__, __LINE__, video_bps);
            if (video_bps == -1) {
                video_bps = cal_bps_with_end2(demuxer, es, params->vpid, priv, start_pos);
                MLOGI("%s----%d:video_bps = %d\n", __func__, __LINE__, video_bps);
            }

            if (video_bps > 0) {
                sh_video->i_bps = video_bps;
                MLOGI("%s%d   sh_video->i_bps = %d\n", __func__, __LINE__, sh_video->i_bps);
            } else {
                sh_video->i_bps = try_to_cal_bps(demuxer, es, params->vpid, priv);
                MLOGI("%s%d   sh_video->i_bps = %d\n", __func__, __LINE__, sh_video->i_bps);
            }

            if (video_bps > 0 && params->atype != UNKNOWN_MP && sh_audio) {
                //calculate the bps by audio
                int cal_time_interval2 = 0;
                off_t end_pos2 = 0;
                audio_bps = cal_bps_with_end(demuxer, es, params->apid, priv, start_pos, &cal_time_interval2, &end_pos2);
                if (audio_bps > 0) {
                    sh_audio->i_bps = audio_bps;
                } else {
                    sh_audio->i_bps = try_to_cal_bps(demuxer, es, params->apid, priv);
                }
                MLOGI("%s----%d:audio_bps = %d\n", __func__, __LINE__, audio_bps);
                if (cal_time_interval2 > cal_time_interval && end_pos2 > end_pos) {
                    //detect more data, more accurate
                    finally_bps = audio_bps;
                } else {
                    finally_bps = video_bps;
                }
            }

            /*
            *next else if is calculate the bps only via audio
            */
        } else if (params->atype != UNKNOWN_MP && sh_audio) {
            MLOGI("%s%d\n", __func__, __LINE__);
            audio_bps = cal_bps_with_end(demuxer, es, params->apid, priv, start_pos, &cal_time_interval, &end_pos);
            if (audio_bps == -1) {
                audio_bps = cal_bps_with_end2(demuxer, es, params->apid, priv, start_pos);
            }
            MLOGI("%s----%d:audio_bps\n", __func__, __LINE__, audio_bps);
            if (audio_bps > 0) {
                sh_audio->i_bps = audio_bps;
                MLOGI("%s%d   sh_audio->i_bps = %d\n", __func__, __LINE__, sh_audio->i_bps);
            } else {
                sh_audio->i_bps = try_to_cal_bps(demuxer, es, params->apid, priv);
                MLOGI("%s%d   sh_audio->i_bps = %d\n", __func__, __LINE__, sh_audio->i_bps);
            }
            finally_bps = audio_bps;
        }

        if (finally_bps > 0 && MP_NOPTS_VALUE == priv->file_last_pts) {
            priv->file_last_pts = es->pts;//related to trickplay
        }

        SAFEFREE(es);
        stream_reset(demuxer->stream);
        stream_seek(demuxer->stream, start_pos);    //IF IT'S FROM A PIPE IT WILL FAIL, BUT WHO CARES?
    }
    demux_seek_ts(demuxer, 0.0f, 0.0f, SEEK_ABSOLUTE);

    return 0;
}
static demuxer_t *demux_open_ts(demuxer_t *demuxer)
{
    int i;
    uint8_t packet_size/*,tmp[204]*/;
    sh_video_t *sh_video = NULL;
    sh_audio_t *sh_audio = NULL;
    off_t start_pos;
    tsdemux_init_t params;
    ts_priv_t *priv = demuxer->priv;

    MLOGD("DEMUX OPEN, AUDIO_ID: %d, VIDEO_ID: %d, SUBTITLE_ID: %d,\n",
          demuxer->audio->id, demuxer->video->id, demuxer->sub->id);

    demuxer->type = DEMUXER_TYPE_MPEG_TS;
    stream_reset(demuxer->stream);

    packet_size = ts_check_file(demuxer);
    if (!packet_size) {
        return NULL;
    }

    priv = (ts_priv_t *)calloc33(1, sizeof(ts_priv_t));
    if (priv == NULL) {
        MLOGF("DEMUX_OPEN_TS, couldn't allocate enough memory for ts->priv, exit\n");
        return NULL;
    }

    for (i = 0; i < NB_PID_MAX; i++) {
        priv->ts.pids[i] = NULL;
        priv->ts.streams[i].id = -3;
    }
    priv->pat.progs = NULL;
    priv->pat.progs_cnt = 0;
    priv->pat.section.buffer = NULL;
    priv->pat.section.buffer_len = 0;

    priv->pmt = NULL;
    priv->pmt_cnt = 0;
    priv->file_last_pts  = MP_NOPTS_VALUE;
    priv->file_first_pts = MP_NOPTS_VALUE;
    priv->selected_apid  = -1;
    priv->selected_vpid  = -1;

    priv->keep_broken = MT_FALSE;
    priv->ts.packet_size = packet_size;

    demuxer->priv = priv;
    demuxer->seekable = 1;

    params.atype = params.vtype = params.stype = UNKNOWN_MP;
    params.apid = demuxer->audio->id;
    params.vpid = demuxer->video->id;
    params.spid = demuxer->sub->id;
    params.prog = 0;
    params.probe = 0;

    if (audio_lang != NULL) {
        strncpy(params.alang, audio_lang, 3);
        params.alang[3] = 0;
    } else {
        memset(params.alang, 0, 4);
    }

    start_pos = ts_detect_streams(demuxer, &params);
    if (priv->pmt_cnt == 0) {
        goto end;
    }
    demuxer->sub->id = params.spid;
    priv->prog = params.prog;
    if (params.vtype != UNKNOWN_MP) {
        ts_add_stream(demuxer, priv->ts.pids[params.vpid]);
        priv->selected_vpid = params.vpid;
        sh_video            = priv->ts.streams[params.vpid].sh;
        demuxer->video->id  = priv->ts.streams[params.vpid].id;
        if (!sh_video) {
            goto end;
        }
        sh_video->ds        = demuxer->video;
        sh_video->format    = params.vtype;
        demuxer->video->sh  = sh_video;
    }
    if (params.atype != UNKNOWN_MP) {
        ES_stream_t *es = priv->ts.pids[params.apid];
        priv->selected_apid = params.apid;
        if (!IS_AUDIO(es->type) && !IS_AUDIO(es->subtype) && IS_AUDIO(params.atype)) {
            es->subtype = params.atype;
        }

        ts_add_stream(demuxer, priv->ts.pids[params.apid]);
        sh_audio           = priv->ts.streams[params.apid].sh;
        demuxer->audio->id = priv->ts.streams[params.apid].id;
        if (!sh_audio) {
            goto end;
        }
        sh_audio->ds       = demuxer->audio;
        sh_audio->format   = params.atype;
        demuxer->audio->sh = sh_audio;
    }

    MLOGI("Opened TS demuxer, audio: %x(pid %d), video: %x(pid %d)...POS=%"PRIu64"\n",
          params.atype, demuxer->audio->id, params.vtype, demuxer->video->id, (uint64_t) start_pos);
    start_pos = start_pos <= priv->ts.packet_size ?
                demuxer->stream->start_pos :
                start_pos - priv->ts.packet_size;
    demuxer->movi_start = start_pos;
    demuxer->reference_clock = MP_NOPTS_VALUE;
    stream_reset(demuxer->stream);
    stream_seek(demuxer->stream, start_pos);    //IF IT'S FROM A PIPE IT WILL FAIL, BUT WHO CARES?

    priv->last_pid = 8192;      //invalid pid
    for (i = 0; i < 3; i++) {
        priv->fifo[i].pack  = NULL;
        priv->fifo[i].offset = 0;
    }
    priv->fifo[0].ds = demuxer->audio;
    priv->fifo[1].ds = demuxer->video;
    priv->fifo[2].ds = demuxer->sub;

    priv->fifo[0].buffer_size = 12 * 1024;
    priv->fifo[1].buffer_size = 64 * 1024;
    priv->fifo[2].buffer_size = 32767;

    priv->pat.section.buffer_len = 0;
    for (i = 0; i < priv->pmt_cnt; i++) {
        priv->pmt[i].section.buffer_len = 0;
    }

    demuxer->filepos = stream_tell(demuxer->stream);
    if ((sh_video == NULL && sh_audio == NULL)
        || ts_check_scrambling_cnt(priv, demuxer->stream))  {
        return NULL;
    }

    get_stream_bps(demuxer, &params, start_pos, priv, sh_video, sh_audio);
    parse_es_open_parser_all(demuxer, &(priv->parser_handle));
    demux_seek_ts(demuxer, 0.0f, 0.0f, SEEK_ABSOLUTE);

    return demuxer;
end:
    ts_check_scrambling_cnt(priv, demuxer->stream);
    return NULL;
}

static void demux_close_ts(demuxer_t *demuxer)
{
    uint16_t i;
    ts_priv_t *priv = (ts_priv_t *) demuxer->priv;

    if (priv) {
        parse_es_close_parser_all(demuxer, &(priv->parser_handle));
        SAFEFREE(priv->pat.section.buffer);
        SAFEFREE(priv->pat.progs);
        if (priv->pmt) {
            for (i = 0; i < priv->pmt_cnt; i++) {
                SAFEFREE(priv->pmt[i].section.buffer);
                SAFEFREE(priv->pmt[i].es);
            }
            SAFEFREE(priv->pmt);
        }
        for (i = 0; i < NB_PID_MAX; i++) {
            SAFEFREE(priv->ts.pids[i]);
            priv->ts.pids[i] = NULL;
        }
        for (i = 0; i < 3; i++) {
            if (priv->fifo[i].pack) {
                free_demux_packet(priv->fifo[i].pack);
            }
            priv->fifo[i].pack = NULL;
        }
        SAFEFREE(priv);
    }
    demuxer->priv = NULL;
}


#define getbits mp_getbits

static int mp4_parse_sl_packet(pmt_t *pmt, uint8_t *buf, uint16_t packet_len, int pid, ES_stream_t *pes_es)
{
    int i, n, m, mp4_es_id = -1;
    uint64_t v = 0;
    uint32_t pl_size = 0;
    int deg_flag = 0;
    mp4_es_descr_t *es = NULL;
    mp4_sl_config_t *sl = NULL;
    uint8_t au_start = 0, au_end = 0, rap_flag = 0, ocr_flag = 0, padding = 0,  padding_bits = 0, idle = 0;

    pes_es->is_synced = 0;
    MLOGD("mp4_parse_sl_packet, pid: %d, pmt: %pm, packet_len: %d\n", pid, pmt, packet_len);
    if (! pmt || !packet_len) {
        return 0;
    }

    for (i = 0; i < pmt->es_cnt; i++) {
        if (pmt->es[i].pid == pid) {
            mp4_es_id = pmt->es[i].mp4_es_id;
        }
    }
    if (mp4_es_id < 0) {
        return -1;
    }

    for (i = 0; i < pmt->mp4es_cnt; i++) {
        if (pmt->mp4es[i].id == mp4_es_id) {
            es = &(pmt->mp4es[i]);
        }
    }
    if (! es) {
        return -1;
    }

    pes_es->subtype = es->decoder.object_type;

    sl = &(es->sl);
    if (!sl) {
        return -1;
    }

    //now es is the complete es_descriptor of out mp4 ES stream
    MLOGA("ID: %d, FLAGS: 0x%x, subtype: %x\n", es->id, sl->flags, pes_es->subtype);

    n = 0;
    if (sl->au_start) {
        pes_es->sl.au_start = au_start = getbits(buf, n++, 1);
    } else {
        pes_es->sl.au_start = (pes_es->sl.last_au_end ? 1 : 0);
    }
    if (sl->au_end) {
        pes_es->sl.au_end = au_end = getbits(buf, n++, 1);
    }

    if (!sl->au_start && !sl->au_end) {
        pes_es->sl.au_start = pes_es->sl.au_end = au_start = au_end = 1;
    }
    pes_es->sl.last_au_end = pes_es->sl.au_end;


    if (sl->ocr_len > 0) {
        ocr_flag = getbits(buf, n++, 1);
    }
    if (sl->idle) {
        idle = getbits(buf, n++, 1);
    }
    if (sl->padding) {
        padding = getbits(buf, n++, 1);
    }
    if (padding) {
        padding_bits = getbits(buf, n, 3);
        n += 3;
    }

    if (idle || (padding && !padding_bits)) {
        pes_es->payload_size = 0;
        return -1;
    }

    //(! idle && (!padding || padding_bits != 0)) is true
    n += sl->packet_seqnum_len;
    if (sl->degr_len) {
        deg_flag = getbits(buf, n++, 1);
    }
    if (deg_flag) {
        n += sl->degr_len;
    }

    if (ocr_flag) {
        n += sl->ocr_len;
        MLOGA("OCR: %d bits\n", sl->ocr_len);
    }

    if (packet_len * 8 <= n) {
        return -1;
    }

    MLOGA("AU_START: %d, AU_END: %d\n", au_start, au_end);
    if (au_start) {
        int dts_flag = 0, cts_flag = 0, ib_flag = 0;

        if (sl->random_accesspoint) {
            rap_flag = getbits(buf, n++, 1);
        }

        //check commented because it seems it's rarely used, and we need this flag set in case of au_start
        //the decoder will eventually discard the payload if it can't decode it
        //if(rap_flag || sl->random_accesspoint_only)
        pes_es->is_synced = 1;

        n += sl->au_seqnum_len;
        if (packet_len * 8 <= n + 8) {
            return -1;
        }
        if (sl->use_ts) {
            dts_flag = getbits(buf, n++, 1);
            cts_flag = getbits(buf, n++, 1);
        }
        if (sl->instant_bitrate_len) {
            ib_flag = getbits(buf, n++, 1);
        }
        if (packet_len * 8 <= n + 8) {
            return -1;
        }
        if (dts_flag && (sl->ts_len > 0)) {
            n += sl->ts_len;
            MLOGA("DTS: %d bits\n", sl->ts_len);
        }
        if (packet_len * 8 <= n + 8) {
            return -1;
        }
        if (cts_flag && (sl->ts_len > 0)) {
            i = 0;

            while (i < sl->ts_len) {
                m = FFMIN(8, sl->ts_len - i);
                v |= getbits(buf, n, m);
                if (sl->ts_len - i > 8) {
                    v <<= 8;
                }
                i += m;
                n += m;
                if (packet_len * 8 <= n + 8) {
                    return -1;
                }
            }

            pes_es->pts = (double) v / (double) sl->ts_resolution;
            MLOGA("CTS: %d bits, value: %"PRIu64"/%d = %.3f\n", sl->ts_len, v, sl->ts_resolution, pes_es->pts);
        }


        i = 0;
        pl_size = 0;
        while (i < sl->au_len) {
            m = FFMIN(8, sl->au_len - i);
            pl_size |= getbits(buf, n, m);
            if (sl->au_len - i > 8) {
                pl_size <<= 8;
            }
            i += m;
            n += m;
            if (packet_len * 8 <= n + 8) {
                return -1;
            }
        }
        MLOGA("AU_LEN: %u (%d bits)\n", pl_size, sl->au_len);
        if (ib_flag) {
            n += sl->instant_bitrate_len;
        }
    }

    m = (n + 7) / 8;
    if (0 < pl_size && pl_size < pes_es->payload_size) {
        pes_es->payload_size = pl_size;
    }

    MLOGD("mp4_parse_sl_packet, n=%d, m=%d, size from pes hdr: %u, sl hdr size: %u, RAP FLAGS: %d/%d\n",
          n, m, pes_es->payload_size, pl_size, (int) rap_flag, (int) sl->random_accesspoint_only);

    return m;
}

//this function parses the extension fields in the PES header and returns the substream_id, or -1 in case of errors
static int parse_pes_extension_fields(unsigned char *p, int pkt_len)
{
    int skip = 0;
    unsigned char flags;

    if (!(p[7] & 0x1)) { //no extension_field
        return -1;
    }
    skip = 9;
    if (p[7] & 0x80) {
        skip += 5;
        if (p[7] & 0x40) {
            skip += 5;
        }
    }
    if (p[7] & 0x20) { //escr_flag
        skip += 6;
    }
    if (p[7] & 0x10) { //es_rate_flag
        skip += 3;
    }
    if (p[7] & 0x08) { //dsm_trick_mode is unsupported, skip
        skip = 0;//don't let's parse the extension fields
    }
    if (p[7] & 0x04) { //additional_copy_info
        skip += 1;
    }
    if (p[7] & 0x02) { //pes_crc_flag
        skip += 2;
    }
    if (skip >= pkt_len) { //too few bytes
        return -1;
    }
    flags = p[skip];
    skip++;
    if (flags & 0x80) { //pes_private_data_flag
        skip += 16;
    }
    if (skip >= pkt_len) {
        return -1;
    }
    if (flags & 0x40) { //pack_header_field_flag
        unsigned char l = p[skip];
        skip += l;
    }
    if (flags & 0x20) { //program_packet_sequence_counter
        skip += 2;
    }
    if (flags & 0x10) { //p_std
        skip += 2;
    }
    if (skip >= pkt_len) {
        return -1;
    }
    if (flags & 0x01) { //finally the long desired pes_extension2
        unsigned char l = p[skip];  //ext2 flag+len
        skip++;
        if ((l == 0x81) && (skip < pkt_len)) {
            int ssid = p[skip];
            MLOGA("SUBSTREAM_ID=%d (0x%02X)\n", ssid, ssid);
            return ssid;
        }
    }

    return -1;
}

static int PCM_BR_parse_header(pcm_info_t *pcm_info, const uint8_t *header)
{
    static const uint8_t bits_per_samples[4] = { 0, 16, 20, 24 };
    static const uint32_t channel_layouts[16] = {
        0, AV_CH_LAYOUT_MONO, 0, AV_CH_LAYOUT_STEREO, AV_CH_LAYOUT_SURROUND,
        AV_CH_LAYOUT_2_1, AV_CH_LAYOUT_4POINT0, AV_CH_LAYOUT_2_2, AV_CH_LAYOUT_5POINT0,
        AV_CH_LAYOUT_5POINT1, AV_CH_LAYOUT_7POINT0, AV_CH_LAYOUT_7POINT1, 0, 0, 0, 0
    };
    static const uint8_t channels[16] = {
        0, 1, 0, 2, 3, 3, 4, 4, 5, 6, 7, 8, 0, 0, 0, 0
    };
    uint8_t channel_layout = header[2] >> 4;

    /* get the sample depth and derive the sample format from it */
    pcm_info->bits_per_coded_sample = bits_per_samples[header[3] >> 6];
    pcm_info->sample_fmt = pcm_info->bits_per_coded_sample == 16 ? AV_SAMPLE_FMT_S16 :
                           AV_SAMPLE_FMT_S32;
    pcm_info->bits_per_raw_sample = pcm_info->bits_per_coded_sample;

    /* get the sample rate. Not all values are used. */
    switch (header[2] & 0x0f) {
        case 1:
            pcm_info->sample_rate = 48000;
            break;
        case 4:
            pcm_info->sample_rate = 96000;
            break;
        case 5:
            pcm_info->sample_rate = 192000;
            break;
        default:
            pcm_info->sample_rate = 0;
    }

    /*
    * get the channel number (and mapping). Not all values are used.
    * It must be noted that the number of channels in the MPEG stream can
    * differ from the actual meaningful number, e.g. mono audio still has two
    * channels, one being empty.
    */
    pcm_info->channel_layout = channel_layouts[channel_layout];
    pcm_info->channels = channels[channel_layout];

    pcm_info->bit_rate = FFALIGN(pcm_info->channels, 2) * pcm_info->sample_rate *
                         pcm_info->bits_per_coded_sample;

    return 0;
}

static int pes_parse2(unsigned char *buf, uint16_t packet_len, ES_stream_t *es, int32_t type_from_pmt, pmt_t *pmt, int pid)
{
    unsigned char  *p = NULL;
    uint32_t       header_len;
    uint32_t       stream_id;
    uint32_t       pkt_len, pes_is_aligned;

    //Here we are always at the start of a PES packet
    MLOGA("pes_parse2(%p, %d): \n", buf, (uint32_t) packet_len);

    if (packet_len == 0 || packet_len > 184) {
        MLOGA("pes_parse2, BUFFER LEN IS TOO SMALL OR TOO BIG: %d EXIT\n", packet_len);
        return 0;
    }

    p = buf;
    pkt_len = packet_len;
    MLOGA("pes_parse2: HEADER %02x %02x %02x %02x\n", p[0], p[1], p[2], p[3]);
    if (p[0] || p[1] || (p[2] != 1)) {
        MLOGA("pes_parse2: error HEADER %02x %02x %02x (should be 0x000001) \n", p[0], p[1], p[2]);
        return 0 ;
    }

    packet_len -= 6;
    if (packet_len == 0) {
        MLOGA("pes_parse2: packet too short: %d, exit\n", packet_len);
        return 0;
    }

    es->payload_size = (p[4] << 8 | p[5]);
    pes_is_aligned = (p[6] & 4);

    stream_id = p[3];
    (void) ts_parse_timestamp(&p[7], packet_len, es);

    if (es->pts < 0) {
        es->pts = 0;
    }

    es->pts31bit = (es->pts * 90000 / 4);
    header_len = p[8];
    if (header_len + 9 > pkt_len) { //9 are the bytes read up to the header_length field
        MLOGA("demux_ts: illegal value for PES_header_data_length (0x%02x)\n", header_len);
        return 0;
    }

    if (stream_id == 0xfd) {
        int ssid = parse_pes_extension_fields(p, pkt_len);
        if (ssid == 0x72 && type_from_pmt != AUDIO_DTS_MP && type_from_pmt != SPU_PGS_MP) {
            es->type  = type_from_pmt = AUDIO_TRUEHD_MP;
        }
    }

    p += header_len + 9;
    packet_len -= header_len + 3;

    if (es->payload_size) {
        es->payload_size -= header_len + 3;
    }

    es->is_synced = 1;  //only for SL streams we have to make sure it's really true, see below
    if (stream_id == 0xbd) {
        MLOGA("pes_parse2: audio buf = %02X %02X %02X %02X %02X %02X %02X %02X, 80: %d\n",
              p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[0] & 0x80);
        /*
        * we check the descriptor tag first because some stations
        * do not include any of the A52 header info in their audio tracks
        * these "raw" streams may begin with a byte that looks like a stream type.
        */
        if (type_from_pmt == SPU_PGS_MP) {
            es->start = p;
            es->size  = packet_len;
            es->type  = SPU_PGS_MP;
            es->payload_size -= packet_len;
            return 1;
        }
        if ((type_from_pmt == AUDIO_A52_MP) ||       /* A52 - raw */
            (packet_len >= 2 && p[0] == 0x0B && p[1] == 0x77) /* A52 - syncword */) {
            MLOGA("A52 RAW OR SYNCWORD\n");
            es->start = p;
            es->size  = packet_len;
            es->type  = AUDIO_A52_MP;
            es->payload_size -= packet_len;
            return 1;
        } else if (type_from_pmt == AUDIO_AC4_MP) {
            es->start = p;
            es->size  = packet_len;
            es->type  = AUDIO_AC4_MP;
            es->payload_size -= packet_len;
            return 1;
        } else if (type_from_pmt == SPU_DVB_MP ||  /* SPU SUBS */
                 (packet_len >= 2 && (p[0] == 0x20) && pes_is_aligned)) { // && p[1] == 0x00))
            // offset/length fiddling to make decoding with lavc possible
            es->start = p + 2;
            es->size  = packet_len - 2;
            es->type  = SPU_DVB_MP;
            es->payload_size -= packet_len;

            return 1;
        } else if (pes_is_aligned && packet_len >= 1 && ((p[0] & 0xE0) == 0x20)) { //SPU_DVD_MP
            //DVD SUBS
            es->start   = p + 1;
            es->size    = packet_len - 1;
            es->type    = SPU_DVD_MP;
            es->payload_size -= packet_len;

            return 1;
        } else if (pes_is_aligned && packet_len >= 4 && (p[0] & 0xF8) == 0x80) {
            MLOGA("A52 WITH HEADER\n");
            es->start   = p + 4;
            es->size    = packet_len - 4;
            es->type    = AUDIO_A52_MP;
            es->payload_size -= packet_len;

            return 1;
        } else if (pes_is_aligned && packet_len >= 1 && ((p[0] & 0xf0) == 0xa0)) {
            int pcm_offset;

            for (pcm_offset = 0; ++pcm_offset < packet_len - 1 ;) {
                if (p[pcm_offset] == 0x01 && p[pcm_offset + 1] == 0x80) {
                    /* START */
                    pcm_offset += 2;
                    break;
                }
            }

            es->start   = p + pcm_offset;
            es->size    = packet_len - pcm_offset;
            es->type    = AUDIO_LPCM_BE_MP;
            es->payload_size -= packet_len;
            if (pmt) {
                pmt->pcm_info.audio_is_pcm = 1;
                pmt->pcm_info.is_big_endian = 1;
                pmt->pcm_info.channels = 2;
                pmt->pcm_info.bits_per_coded_sample = 16;
                pmt->pcm_info.sample_rate = 48000;
            }
            return 1;
        } else {
            MLOGA("PES_PRIVATE1_MP\n");
            /* current adec cannot recognize hdmv lpcm header, so we remove it */
            if (AUDIO_PCM_BR_MP == type_from_pmt) {
                if (packet_len >= DEFAULT_FRAME_HEADER_BYTES) {
                    PCM_BR_parse_header(&(pmt->pcm_info), p);
                }
            }

            es->start   = p;
            es->size    = packet_len;
            es->type    = (type_from_pmt == UNKNOWN_MP ? PES_PRIVATE1_MP : type_from_pmt);
            es->payload_size -= packet_len;

            return 1;
        }
    } else if (((stream_id >= 0xe0) && (stream_id <= 0xef)) || (stream_id == 0xfd && type_from_pmt != UNKNOWN_MP)) {
        es->start   = p;
        es->size    = packet_len;
        if (type_from_pmt != UNKNOWN_MP) {
            es->type    = type_from_pmt;
        } else {
            es->type    = VIDEO_MPEG2_MP;
        }
        if (es->payload_size) {
            es->payload_size -= packet_len;
        }

        MLOGA("pes_parse2: M2V size %d\n", es->size);
        return 1;
    } else if ((stream_id == 0xfa)) {
        int l;

        es->is_synced = 0;
        if (type_from_pmt != UNKNOWN_MP) { //MP4 A/V or SL
            es->start   = p;
            es->size    = packet_len;
            es->type    = type_from_pmt;

            if (type_from_pmt == SL_PES_STREAM_MP) {
                l = mp4_parse_sl_packet(pmt, p, packet_len, pid, es);
                MLOGA("L=%d, TYPE=%x\n", l, type_from_pmt);
                if (l < 0) {
                    MLOGA("pes_parse2: couldn't parse SL header, passing along full PES payload\n");
                    l = 0;
                }

                es->start   += l;
                es->size    -= l;
            }

            if (es->payload_size) {
                es->payload_size -= packet_len;
            }
            return 1;
        }
    } else if ((stream_id & 0xe0) == 0xc0) {
        es->start   = p;
        es->size    = packet_len;

        if (type_from_pmt != UNKNOWN_MP) {
            es->type = type_from_pmt;
        } else {
            es->type    = AUDIO_MP2_MP;
        }

        es->payload_size -= packet_len;

        return 1;
    } else if (type_from_pmt != -1) { //as a last resort here we trust the PMT, if present
        es->start   = p;
        es->size    = packet_len;
        es->type    = type_from_pmt;
        es->payload_size -= packet_len;

        return 1;
    } else {
        MLOGA("pes_parse2: unknown packet, id: %x\n", stream_id);
    }

    es->is_synced = 0;
    return 0;
}

static void ts_drain_reamin_fifo(
    demuxer_t *demuxer, ts_priv_t *priv)
{
    int i;

    for (i = 0; i < 3; i++) {
        if ((priv->fifo[i].pack != NULL) && (priv->fifo[i].offset != 0)) {
            int resize_ret = 0;
            resize_ret = resize_demux_packet(priv->fifo[i].pack, priv->fifo[i].offset);
            if (resize_ret == -1) {
                free_demux_packet(priv->fifo[i].pack);
                priv->fifo[i].pack = NULL;
                return ;
            }
            /* add all remain data in fifo to pkt list */
            parse_es_add_packet(demuxer,
                                priv->fifo[i].ds, &(priv->parser_handle), priv->fifo[i].pack);
            priv->fifo[i].offset = 0;
            priv->fifo[i].pack = NULL;
        }
    }
}

static int32_t prog_idx_in_pat(ts_priv_t *priv, uint16_t progid)
{
    int x;

    if (priv->pat.progs == NULL) {
        return -1;
    }

    for (x = 0; x < priv->pat.progs_cnt; x++) {
        if (priv->pat.progs[x].id == progid) {
            return x;
        }
    }

    return -1;
}


static int32_t prog_id_in_pat(ts_priv_t *priv, uint16_t pid)
{
    int x;

    if (priv->pat.progs == NULL) {
        return -1;
    }

    for (x = 0; x < priv->pat.progs_cnt; x++) {
        if (priv->pat.progs[x].pmt_pid == pid) {
            return priv->pat.progs[x].id;
        }
    }

    return -1;
}

static int collect_section(ts_section_t *section, int is_start, unsigned char *buff, int size)
{
    uint8_t *ptr;
    uint16_t tlen;
    int skip, tid;

    MLOGD("COLLECT_SECTION, start: %d, size: %d, collected: %d\n", is_start, size, section->buffer_len);
    if (! is_start && !section->buffer_len) {
        return 0;
    }

    if (is_start) {
        if (! section->buffer) {
            section->buffer = (uint8_t *)malloc33(4096 + 256);
            if (section->buffer == NULL) {
                return 0;
            }
        }
        section->buffer_len = 0;
    }

    if (size + section->buffer_len > 4096 + 256) {
        MLOGD("COLLECT_SECTION, excessive len: %d + %d\n", section->buffer_len, size);
        return 0;
    }

    memcpy(&(section->buffer[section->buffer_len]), buff, size);
    section->buffer_len += size;

    if (section->buffer_len < 3) {
        return 0;
    }

    skip = section->buffer[0];
    if (skip + 4 > section->buffer_len) {
        return 0;
    }

    ptr = &(section->buffer[skip + 1]);
    tid = ptr[0];
    tlen = ((ptr[1] & 0x0f) << 8) | ptr[2];
    MLOGD("SKIP: %d+1, TID: %d, TLEN: %d, COLLECTED: %d\n", skip, tid, tlen, section->buffer_len);
    if (section->buffer_len < (skip + 1 + 3 + tlen)) {
        MLOGA("DATA IS NOT ENOUGH, NEXT TIME\n");
        return 0;
    }

    return skip + 1;
}

static int parse_pat(ts_priv_t *priv, int is_start, unsigned char *buff, int size)
{
    int skip;
    unsigned char *ptr;
    unsigned char *base;
    int entries, i;
    uint16_t progid;
    ts_section_t *section;

    section = &(priv->pat.section);
    skip = collect_section(section, is_start, buff, size);
    if (! skip) {
        return 0;
    }

    ptr = &(section->buffer[skip]);
    //PARSING
    priv->pat.table_id = ptr[0];
    if (priv->pat.table_id != 0) {
        return 0;
    }
    priv->pat.ssi = (ptr[1] >> 7) & 0x1;
    priv->pat.curr_next = ptr[5] & 0x01;
    priv->pat.ts_id = (ptr[3]  << 8) | ptr[4];
    priv->pat.version_number = (ptr[5] >> 1) & 0x1F;
    priv->pat.section_length = ((ptr[1] & 0x03) << 8) | ptr[2];
    priv->pat.section_number = ptr[6];
    priv->pat.last_section_number = ptr[7];

    //check_crc32(0xFFFFFFFFL, ptr, priv->pat.buffer_len - 4, &ptr[priv->pat.buffer_len - 4]);
    MLOGD("PARSE_PAT: section_len: %d, section %d/%d\n", priv->pat.section_length, priv->pat.section_number, priv->pat.last_section_number);

    entries = (int)(priv->pat.section_length - 9) / 4;  //entries per section

    for (i = 0; i < entries; i++) {
        int32_t idx;
        base = &ptr[8 + i * 4];
        progid = (base[0] << 8) | base[1];

        if ((idx = prog_idx_in_pat(priv, progid)) == -1) {
            priv->pat.progs = realloc_struct(priv->pat.progs, priv->pat.progs_cnt + 1, sizeof(struct pat_progs_t));
            if (!priv->pat.progs) {
                int sz = sizeof(struct pat_progs_t) * (priv->pat.progs_cnt + 1);
                priv->pat.progs_cnt = 0;
                MLOGE("PARSE_PAT: COULDN'T REALLOC %d bytes, NEXT\n", sz);
                break;
            }
            idx = priv->pat.progs_cnt;
            priv->pat.progs_cnt++;
        }

        priv->pat.progs[idx].id = progid;
        priv->pat.progs[idx].pmt_pid = ((base[2]  & 0x1F) << 8) | base[3];
        MLOGD("PROG: %d (%d-th of %d), PMT: %d\n", priv->pat.progs[idx].id, i + 1, entries, priv->pat.progs[idx].pmt_pid);
        MLOGD("PROGRAM_ID=%d (0x%02X), PMT_PID: %d(0x%02X)\n",
              progid, progid, priv->pat.progs[idx].pmt_pid, priv->pat.progs[idx].pmt_pid);
    }

    return 1;
}


static int32_t es_pid_in_pmt(pmt_t *pmt, uint16_t pid)
{
    uint16_t i;

    if (pmt == NULL) {
        return -1;
    }

    if (pmt->es == NULL) {
        return -1;
    }

    for (i = 0; i < pmt->es_cnt; i++) {
        if (pmt->es[i].pid == pid) {
            return (int32_t) i;
        }
    }

    return -1;
}


static uint16_t get_mp4_desc_len(uint8_t *buf, int *len)
{
    //uint16_t i = 0, size = 0;
    int i = 0, j, size = 0;

    MLOGA("PARSE_MP4_DESC_LEN(%d), bytes: ", *len);
    j = FFMIN(*len, 4);
    while (i < j) {
        MLOGA(" %x ", buf[i]);
        size |= (buf[i] & 0x7f);
        if (!(buf[i] & 0x80)) {
            break;
        }
        size <<= 7;
        i++;
    }
    MLOGA(", SIZE=%d\n", size);

    *len = i + 1;
    return size;
}


static uint16_t parse_mp4_slconfig_descriptor(uint8_t *buf, int len, void *elem)
{
    int i = 0;
    mp4_es_descr_t *es;
    mp4_sl_config_t *sl;

    MLOGD("PARSE_MP4_SLCONFIG_DESCRIPTOR(%d)\n", len);
    es = (mp4_es_descr_t *) elem;
    if (!es) {
        MLOGD("argh! NULL elem passed, skip\n");
        return len;
    }
    sl = &(es->sl);

    sl->ts_len = sl->ocr_len = sl->au_len = sl->instant_bitrate_len = sl->degr_len = sl->au_seqnum_len = sl->packet_seqnum_len = 0;
    sl->ocr = sl->dts = sl->cts = 0;

    if (buf[0] == 0) {
        i++;
        sl->flags = buf[i];
        i++;
        sl->ts_resolution = (buf[i] << 24) | (buf[i + 1] << 16) | (buf[i + 2] << 8) | buf[i + 3];
        i += 4;
        sl->ocr_resolution = (buf[i] << 24) | (buf[i + 1] << 16) | (buf[i + 2] << 8) | buf[i + 3];
        i += 4;
        sl->ts_len = buf[i];
        i++;
        sl->ocr_len = buf[i];
        i++;
        sl->au_len = buf[i];
        i++;
        sl->instant_bitrate_len = buf[i];
        i++;
        sl->degr_len = (buf[i] >> 4) & 0x0f;
        sl->au_seqnum_len = ((buf[i] & 0x0f) << 1) | ((buf[i + 1] >> 7) & 0x01);
        i++;
        sl->packet_seqnum_len = ((buf[i] >> 2) & 0x1f);
        i++;

    } else if (buf[0] == 1) {
        sl->flags = 0;
        sl->ts_resolution = 1000;
        sl->ts_len = 32;
        i++;
    } else if (buf[0] == 2) {
        sl->flags = 4;
        i++;
    } else {
        sl->flags = 0;
        i++;
    }

    sl->au_start = (sl->flags >> 7) & 0x1;
    sl->au_end = (sl->flags >> 6) & 0x1;
    sl->random_accesspoint = (sl->flags >> 5) & 0x1;
    sl->random_accesspoint_only = (sl->flags >> 4) & 0x1;
    sl->padding = (sl->flags >> 3) & 0x1;
    sl->use_ts = (sl->flags >> 2) & 0x1;
    sl->idle = (sl->flags >> 1) & 0x1;
    sl->duration = sl->flags & 0x1;

    if (sl->duration) {
        sl->timescale = (buf[i] << 24) | (buf[i + 1] << 16) | (buf[i + 2] << 8) | buf[i + 3];
        i += 4;
        sl->au_duration = (buf[i] << 8) | buf[i + 1];
        i += 2;
        sl->cts_duration = (buf[i] << 8) | buf[i + 1];
        i += 2;
    } else { //no support for fixed durations atm
        sl->timescale = sl->au_duration = sl->cts_duration = 0;
    }

    MLOGD("MP4SLCONFIG(len=0x%x), predef: %d, flags: %x, use_ts: %d, tslen: %d, timescale: %d, dts: %"PRIu64", cts: %"PRIu64"\n",
          len, buf[0], sl->flags, sl->use_ts, sl->ts_len, sl->timescale, (uint64_t) sl->dts, (uint64_t) sl->cts);

    return len;
}

static int parse_mp4_descriptors(pmt_t *pmt, uint8_t *buf, int len, void *elem);

static uint16_t parse_mp4_decoder_config_descriptor(pmt_t *pmt, uint8_t *buf, int len, void *elem)
{
    int i = 0, j;
    mp4_es_descr_t *es;
    mp4_decoder_config_t *dec;

    MLOGD("PARSE_MP4_DECODER_CONFIG_DESCRIPTOR(%d)\n", len);
    es = (mp4_es_descr_t *) elem;
    if (!es) {
        MLOGD("argh! NULL elem passed, skip\n");
        return len;
    }
    dec = (mp4_decoder_config_t *) & (es->decoder);

    dec->object_type = buf[i];
    dec->stream_type = (buf[i + 1] >> 2) & 0x3f;

    if (dec->object_type == 1 && dec->stream_type == 1) {
        dec->object_type = MP4_OD_MP;
        dec->stream_type = MP4_OD_MP;
    } else if (dec->stream_type == 4) {
        if (dec->object_type == 0x6a) {
            dec->object_type = VIDEO_MPEG1_MP;
        }
        if (dec->object_type >= 0x60 && dec->object_type <= 0x65) {
            dec->object_type = VIDEO_MPEG2_MP;
        } else if (dec->object_type == 0x20) {
            dec->object_type = VIDEO_MPEG4_MP;
        } else if (dec->object_type == 0x21) {
            dec->object_type = VIDEO_AVC_MP;
        }
        /*else if(dec->object_type == 0x22)
            MLOGW("TYPE 0x22\n");*/
        else {
            dec->object_type = UNKNOWN_MP;
        }
    } else if (dec->stream_type == 5) {
        if (dec->object_type == 0x40) {
            dec->object_type = AUDIO_AAC_MP;
        } else if (dec->object_type == 0x6b) {
            dec->object_type = AUDIO_MP2_MP;
        } else if (dec->object_type >= 0x66 && dec->object_type <= 0x69) {
            dec->object_type = AUDIO_MP2_MP;
        } else {
            dec->object_type = UNKNOWN_MP;
        }
    } else {
        dec->object_type = dec->stream_type = UNKNOWN_MP;
    }

    if (dec->object_type != UNKNOWN_MP) {
        //update the type of the current stream
        for (j = 0; j < pmt->es_cnt; j++) {
            if (pmt->es[j].mp4_es_id == es->id) {
                pmt->es[j].type = SL_PES_STREAM_MP;
            }
        }
    }

    if (len > 13) {
        parse_mp4_descriptors(pmt, &buf[13], len - 13, dec);
    }

    MLOGD("MP4DECODER(0x%x), object_type: 0x%x, stream_type: 0x%x\n", len, dec->object_type, dec->stream_type);

    return len;
}

static uint16_t parse_mp4_decoder_specific_descriptor(uint8_t *buf, int len, void *elem)
{
    int i;
    mp4_decoder_config_t *dec;

    MLOGD("PARSE_MP4_DECODER_SPECIFIC_DESCRIPTOR(%d)\n", len);
    dec = (mp4_decoder_config_t *) elem;
    if (!dec) {
        MLOGD("argh! NULL elem passed, skip\n");
        return len;
    }

    MLOGA("MP4 SPECIFIC INFO BYTES: \n");
    for (i = 0; i < len; i++) {
        MLOGA("%02x ", buf[i]);
    }
    MLOGA("\n");

    if (len > MAX_EXTRADATA_SIZE) {
        MLOGE("DEMUX_TS, EXTRADATA SUSPICIOUSLY BIG: %d, REFUSED\r\n", len);
        return len;
    }
    memcpy(dec->buf, buf, len);
    dec->buf_size = len;

    return len;
}

static uint16_t parse_mp4_es_descriptor(pmt_t *pmt, uint8_t *buf, int len)
{
    int i = 0, j = 0, k, found;
    uint8_t flag;
    mp4_es_descr_t es, *target_es = NULL;

    MLOGD("PARSE_MP4ES: len=%d\n", len);
    memset(&es, 0, sizeof(mp4_es_descr_t));
    while (i < len) {
        es.id = (buf[i] << 8) | buf[i + 1];
        MLOGD("MP4ES_ID: %d\n", es.id);
        i += 2;
        flag = buf[i];
        i++;
        if (flag & 0x80) {
            i += 2;
        }
        if (flag & 0x40) {
            i += buf[i] + 1;
        }
        if (flag & 0x20) {  //OCR, maybe we need it
            i += 2;
        }

        j = parse_mp4_descriptors(pmt, &buf[i], len - i, &es);
        MLOGD("PARSE_MP4ES, types after parse_mp4_descriptors: 0x%x, 0x%x\n", es.decoder.object_type, es.decoder.stream_type);
        if (es.decoder.object_type != UNKNOWN_MP && es.decoder.stream_type != UNKNOWN_MP) {
            found = 0;
            //search this ES_ID if we already have it
            for (k = 0; k < pmt->mp4es_cnt; k++) {
                if (pmt->mp4es[k].id == es.id) {
                    target_es = &(pmt->mp4es[k]);
                    found = 1;
                }
            }

            if (! found) {
                pmt->mp4es = realloc_struct(pmt->mp4es, pmt->mp4es_cnt + 1, sizeof(mp4_es_descr_t));
                if (!pmt->mp4es) {
                    pmt->mp4es_cnt = 0;
                    MLOGW("CAN'T REALLOC MP4_ES_DESCR\n");
                    continue;
                }
                target_es = &(pmt->mp4es[pmt->mp4es_cnt]);
                pmt->mp4es_cnt++;
            }
            memcpy(target_es, &es, sizeof(mp4_es_descr_t));
            MLOGD("MP4ES_CNT: %d, ID=%d\n", pmt->mp4es_cnt, target_es->id);
        }

        i += j;
    }

    return len;
}

static void parse_mp4_object_descriptor(pmt_t *pmt, uint8_t *buf, int len, void *elem)
{
    int i, j = 0, id;

    i = 0;
    id = (buf[0] << 2) | ((buf[1] & 0xc0) >> 6);
    MLOGD("PARSE_MP4_OBJECT_DESCRIPTOR: len=%d, OD_ID=%d\n", len, id);
    if (buf[1] & 0x20) {
        i += buf[2] + 1;    //url
        MLOGD("URL\n");
    } else {
        i = 2;

        while (i < len) {
            j = parse_mp4_descriptors(pmt, &(buf[i]), len - i, elem);
            MLOGD("OBJD, NOW i = %d, j=%d, LEN=%d\n", i, j, len);
            i += j;
        }
    }
}


static void parse_mp4_iod(pmt_t *pmt, uint8_t *buf, int len, void *elem)
{
    int i, j = 0;
    mp4_od_t *iod = &(pmt->iod);

    iod->id = (buf[0] << 2) | ((buf[1] & 0xc0) >> 6);
    MLOGD("PARSE_MP4_IOD: len=%d, IOD_ID=%d\n", len, iod->id);
    i = 2;
    if (buf[1] & 0x20) {
        i += buf[2] + 1;    //url
        MLOGD("URL\n");
    } else {
        i = 7;
        while (i < len) {
            j = parse_mp4_descriptors(pmt, &(buf[i]), len - i, elem);
            MLOGD("IOD, NOW i = %d, j=%d, LEN=%d\n", i, j, len);
            i += j;
        }
    }
}

static int parse_mp4_descriptors(pmt_t *pmt, uint8_t *buf, int len, void *elem)
{
    int tag, descr_len, i = 0, j = 0;

    MLOGD("PARSE_MP4_DESCRIPTORS, len=%d\n", len);
    if (! len) {
        return len;
    }

    while (i < len) {
        tag = buf[i];
        j = len - i - 1;
        descr_len = get_mp4_desc_len(&(buf[i + 1]), &j);
        MLOGD("TAG=%d (0x%x), DESCR_len=%d, len=%d, j=%d\n", tag, tag, descr_len, len, j);
        if (descr_len > len - j + 1) {
            MLOGD("descriptor is too long, exit\n");
            return len;
        }
        i += j + 1;

        switch (tag) {
            case 0x1:
                parse_mp4_object_descriptor(pmt, &(buf[i]), descr_len, elem);
                break;
            case 0x2:
                parse_mp4_iod(pmt, &(buf[i]), descr_len, elem);
                break;
            case 0x3:
                parse_mp4_es_descriptor(pmt, &(buf[i]), descr_len);
                break;
            case 0x4:
                parse_mp4_decoder_config_descriptor(pmt, &buf[i], descr_len, elem);
                break;
            case 0x05:
                parse_mp4_decoder_specific_descriptor(&buf[i], descr_len, elem);
                break;
            case 0x6:
                parse_mp4_slconfig_descriptor(&buf[i], descr_len, elem);
                break;
            default:
                MLOGD("Unsupported mp4 descriptor 0x%x\n", tag);
        }
        i += descr_len;
    }

    return len;
}

static ES_stream_t *new_pid(ts_priv_t *priv, int pid)
{
    ES_stream_t *tss;

    tss = (ES_stream_t *)calloc33(sizeof(*tss), 1);
    if (! tss) {
        return NULL;
    }
    tss->pid = pid;
    tss->last_cc = -1;
    tss->type = UNKNOWN_MP;
    tss->subtype = UNKNOWN_MP;
    tss->is_synced = 0;
    tss->extradata = NULL;
    tss->extradata_alloc = tss->extradata_len = 0;
    priv->ts.pids[pid] = tss;

    return tss;
}


static int parse_program_descriptors(pmt_t *pmt, uint8_t *buf, uint16_t len)
{
    uint16_t i = 0, k, olen = len;

    while (len > 0) {
        MLOGD("PROG DESCR, TAG=%x, LEN=%d(%x)\n", buf[i], buf[i + 1], buf[i + 1]);
        if (buf[i + 1] > len - 2) {
            MLOGD("ERROR, descriptor len is too long, skipping\n");
            return olen;
        }

        if (buf[i] == 0x1d) {
            if (buf[i + 3] == 2) { //buggy versions of vlc muxer make this non-standard mess (missing iod_scope)
                k = 3;
            } else {
                k = 4;    //this is standard compliant
            }
            parse_mp4_descriptors(pmt, &buf[i + k], (int) buf[i + 1] - (k - 2), NULL);
        }

        len -= 2 + buf[i + 1];
    }

    return olen;
}

static int parse_descriptors(struct pmt_es_t *es, uint8_t *ptr)
{
    int j, descr_len, len;

    j = 0;
    len = es->descr_length;
    while (len > 2) {
        descr_len = ptr[j + 1];
        MLOGD("...descr id: 0x%x, len=%d\n", ptr[j], descr_len);
        if (descr_len > len) {
            MLOGE("INVALID DESCR LEN for tag %02x: %d vs %d max, EXIT LOOP\n", ptr[j], descr_len, len);
            return -1;
        }

        if (ptr[j] == 0x6a || ptr[j] == 0x7a) { //A52 Descriptor
            if (es->type == 0x6) {
                es->type = AUDIO_A52_MP;
                if (ptr[j] == 0x7a) {
                    es->audio_eac3_flag = 1;
                }
                MLOGA("DVB A52 Descriptor\n");
            }
        } else if (ptr[j] == 0x7b) { //DVB DTS Descriptor
            if (es->type == 0x6) {
                es->type = AUDIO_DTS_MP;
                MLOGA("DVB DTS Descriptor\n");
            }
        } else if (ptr[j] == 0x56) { // Teletext
            if (descr_len >= 5) {
                memcpy(es->lang, ptr + j + 2, 3);
                es->lang[3] = 0;
            }
            es->type = SPU_TELETEXT_MP;
        } else if (ptr[j] == 0x59) { //Subtitling Descriptor
            uint8_t subtype;

            MLOGA("Subtitling Descriptor\n");
            if (descr_len < 8) {
                MLOGA("Descriptor length too short for DVB Subtitle Descriptor: %d, SKIPPING\n", descr_len);
            } else {
                memcpy(es->lang, &ptr[j + 2], 3);
                es->lang[3] = 0;
                subtype = ptr[j + 5];
                if (
                    (subtype >= 0x10 && subtype <= 0x13) ||
                    (subtype >= 0x20 && subtype <= 0x23)
                ) {
                    es->type = SPU_DVB_MP;
                    //page parameters: compo page 2 bytes, ancillary page 2 bytes
                } else {
                    es->type = UNKNOWN_MP;
                }
            }
        } else if (ptr[j] == 0x50) { //Component Descriptor
            MLOGA("Component Descriptor\n");
            memcpy(es->lang, &ptr[j + 5], 3);
            es->lang[3] = 0;
        } else if (ptr[j] == 0xa) { //Language Descriptor
            memcpy(es->lang, &ptr[j + 2], 3);
            es->lang[3] = 0;
            MLOGD("Language Descriptor: %s\n", es->lang);
        } else if (ptr[j] == 0x5) { //Registration Descriptor (looks like e fourCC :) )
            MLOGA("Registration Descriptor\n");
            if (descr_len < 4) {
                MLOGA("Registration Descriptor length too short: %d, SKIPPING\n", descr_len);
            } else {
                char *d;
                memcpy(es->format_descriptor, &ptr[j + 2], 4);
                es->format_descriptor[4] = 0;

                d = &ptr[j + 2];
                if (d[0] == 'A' && d[1] == 'C' && d[2] == '-' && d[3] == '3') {
                    es->type = AUDIO_A52_MP;
                } else if (d[0] == 'D' && d[1] == 'T' && d[2] == 'S' && d[3] == '1') {
                    es->type = AUDIO_DTS_MP;
                } else if (d[0] == 'D' && d[1] == 'T' && d[2] == 'S' && d[3] == '2') {
                    es->type = AUDIO_DTS_MP;
                } else if (d[0] == 'V' && d[1] == 'C' && d[2] == '-' && d[3] == '1') {
                    es->type = VIDEO_VC1_MP;
                } else if (d[0] == 'd' && d[1] == 'r' && d[2] == 'a' && d[3] == 'c') {
                    es->type = VIDEO_DIRAC_MP;
                } else if (d[0] == 'B' && d[1] == 'S' && d[2] == 'S' && d[3] == 'D') {
                    es->type = AUDIO_S302M_MP;
                } else if (d[0] == 'H' && d[1] == 'E' && d[2] == 'V' && d[3] == 'C') {
                    es->type = VIDEO_HEVC_MP;
                } else if(d[0] == 'E' && d[1] == 'A' && d[2] == 'C' && d[3] == '3'){
                    es->type = AUDIO_A52_MP;
                } else if(d[0] == 'A' && d[1] == 'C' && d[2] == '-' && d[3] == '4'){
                    es->type = AUDIO_AC4_MP;
                }
                MLOGA("FORMAT %s\n", es->format_descriptor);
            }
        } else if (ptr[j] == 0x1e || ptr[j] == 0x1f) {
            // 0x1f is FMC, but currently it is easiest to handle them the same way
            es->mp4_es_id = (ptr[j + 2] << 8) | ptr[j + 3];
            MLOGD("SL Descriptor: ES_ID: %d(%x), pid: %d\n", es->mp4_es_id, es->mp4_es_id, es->pid);
        } else {
            MLOGA("Unknown descriptor 0x%x, SKIPPING\n", ptr[j]);
        }

        len -= 2 + descr_len;
        j += 2 + descr_len;
    }

    return 1;
}

static int parse_sl_section(pmt_t *pmt, ts_section_t *section, int is_start, unsigned char *buff, int size)
{
    int tid, len, skip;
    uint8_t *ptr;
    skip = collect_section(section, is_start, buff, size);
    if (! skip) {
        return 0;
    }

    ptr = &(section->buffer[skip]);
    tid = ptr[0];
    len = ((ptr[1] & 0x0f) << 8) | ptr[2];
    MLOGD("TABLEID: %d (av. %d), skip=%d, LEN: %d\n", tid, section->buffer_len, skip, len);
    if (len > 4093 || section->buffer_len < len || tid != 5) {
        MLOGD("SECTION TOO LARGE or wrong section type, EXIT\n");
        return 0;
    }

    if (!(ptr[5] & 1)) {
        return 0;
    }

    //8 is the current position, len - 9 is the amount of data available
    parse_mp4_descriptors(pmt, &ptr[8], len - 9, NULL);

    return 1;
}

static int parse_pmt(ts_priv_t *priv, uint16_t progid, uint16_t pid, int is_start, unsigned char *buff, int size)
{
    unsigned char *base, *es_base;
    pmt_t *pmt;
    int32_t idx, es_count, section_bytes;
    uint8_t m = 0;
    int skip;
//       uint32_t format_identifier = 0;
    ts_section_t *section;
    ES_stream_t *tss;
    int i;

    idx = progid_idx_in_pmt(priv, progid);

    if (idx == -1) {
        priv->pmt = realloc_struct(priv->pmt, priv->pmt_cnt + 1, sizeof(pmt_t));
        if (!priv->pmt) {
            int sz = (priv->pmt_cnt + 1) * sizeof(pmt_t);
            priv->pmt_cnt = 0;
            MLOGE("PARSE_PMT: COULDN'T REALLOC %d bytes, NEXT\n", sz);
            return 0;
        }
        idx = priv->pmt_cnt;
        memset(&(priv->pmt[idx]), 0, sizeof(pmt_t));
        priv->pmt_cnt++;
        priv->pmt[idx].progid = progid;
    }

    pmt = &(priv->pmt[idx]);

    section = &(pmt->section);
    skip = collect_section(section, is_start, buff, size);
    if (! skip) {
        return 0;
    }

    base = &(section->buffer[skip]);

    MLOGD("FILL_PMT(prog=%d), PMT_len: %d, IS_START: %d, TS_PID: %d, SIZE=%d, M=%d, ES_CNT=%d, IDX=%d, PMT_PTR=%p\n",
          progid, pmt->section.buffer_len, is_start, pid, size, m, pmt->es_cnt, idx, pmt);

    pmt->table_id = base[0];
    if (pmt->table_id != 2) {
        return -1;
    }
    pmt->ssi = base[1] & 0x80;
    pmt->section_length = (((base[1] & 0xf) << 8) | base[2]);
    pmt->version_number = (base[5] >> 1) & 0x1f;
    pmt->curr_next = (base[5] & 1);
    pmt->section_number = base[6];
    pmt->last_section_number = base[7];
    pmt->PCR_PID = ((base[8] & 0x1f) << 8) | base[9];
    pmt->prog_descr_length = ((base[10] & 0xf) << 8) | base[11];
    if (pmt->prog_descr_length > pmt->section_length - 9) {
        MLOGD("PARSE_PMT, INVALID PROG_DESCR LENGTH (%d vs %d)\n", pmt->prog_descr_length, pmt->section_length - 9);
        return -1;
    }

    if (pmt->prog_descr_length) {
        parse_program_descriptors(pmt, &base[12], pmt->prog_descr_length);
    }

    es_base = &base[12 + pmt->prog_descr_length];   //the beginning of th ES loop

    section_bytes = pmt->section_length - 13 - pmt->prog_descr_length;
    es_count  = 0;

    while (section_bytes >= 5) {
        int es_pid, es_type;

        es_type = es_base[0];
        es_pid = ((es_base[1] & 0x1f) << 8) | es_base[2];

        idx = es_pid_in_pmt(pmt, es_pid);
        if (idx == -1) {
            pmt->es = realloc_struct(pmt->es, pmt->es_cnt + 1, sizeof(struct pmt_es_t));
            if (!pmt->es) {
                int sz = sizeof(struct pmt_es_t) * (pmt->es_cnt + 1);
                pmt->es_cnt = 0;
                MLOGE("PARSE_PMT, COULDN'T ALLOCATE %d bytes for PMT_ES\n", sz);
                continue;
            }
            idx = pmt->es_cnt;
            memset(&(pmt->es[idx]), 0, sizeof(struct pmt_es_t));
            pmt->es_cnt++;
        }

        pmt->es[idx].descr_length = ((es_base[3] & 0xf) << 8) | es_base[4];


        if (pmt->es[idx].descr_length > section_bytes - 5) {
            MLOGD("PARSE_PMT, ES_DESCR_LENGTH TOO LARGE %d > %d, EXIT\n",
                  pmt->es[idx].descr_length, section_bytes - 5);
            return -1;
        }


        pmt->es[idx].pid = es_pid;
        if (es_type != 0x6) {
            pmt->es[idx].type = UNKNOWN_MP;
        } else {
            pmt->es[idx].type = es_type;
        }

        parse_descriptors(&pmt->es[idx], &es_base[5]);

        switch (es_type) {
            case 1:
                pmt->es[idx].type = VIDEO_MPEG1_MP;
                break;
            case 2:
                pmt->es[idx].type = VIDEO_MPEG2_MP;
                break;
            case 3:
            case 4:
                pmt->es[idx].type = AUDIO_MP2_MP;
                break;
            case 6:
                if (pmt->es[idx].type == 0x6) { //this could have been ovrwritten by parse_descriptors
                    pmt->es[idx].type = UNKNOWN_MP;
                }
                break;
            case 0x10:
                pmt->es[idx].type = VIDEO_MPEG4_MP;
                break;
            case 0x0f:
                pmt->es[idx].type = AUDIO_AAC_MP;
                break;
            case 0x11:
                pmt->es[idx].type = AUDIO_AAC_LATM_MP;
                for (i = 0; i < pmt->mp4es_cnt; i++)
                    if (pmt->mp4es[i].id == pmt->es[idx].mp4_es_id &&
                        pmt->mp4es[i].decoder.object_type == AUDIO_AAC_MP) {
                        pmt->es[idx].type = AUDIO_AAC_MP;
                    }
                break;
            case 0x1b:
                pmt->es[idx].type = VIDEO_H264_MP;
                break;
            case 0x12:
                pmt->es[idx].type = SL_PES_STREAM_MP;
                break;
            case 0x13:
                pmt->es[idx].type = SL_SECTION_MP;
                break;
            case 0x24:
                pmt->es[idx].type = VIDEO_HEVC_MP;
                break;
            case 0x80:
                pmt->es[idx].type = AUDIO_PCM_BR_MP;
                pmt->pcm_info.audio_is_pcm = 1;
                pmt->pcm_info.is_big_endian = 1;
                break;
            case 0x81:
                pmt->es[idx].type = AUDIO_A52_MP;
                break;
            case 0x8A:
            case 0x82:
            case 0x85:
            case 0x86:
                pmt->es[idx].type = AUDIO_DTS_MP;
                break;
            case 0x84:
            case 0x87:
                pmt->es[idx].type = AUDIO_A52_MP;
                pmt->es[idx].audio_eac3_flag = 1;
                break;
            case 0x90:
                pmt->es[idx].type = SPU_PGS_MP;
                break;
            case 0xD1:
                pmt->es[idx].type = VIDEO_DIRAC_MP;
                break;
            case 0xD2:
                pmt->es[idx].type = VIDEO_AVS2_MP;
                break;
            case 0xD5:
                pmt->es[idx].type = AUDIO_AV3A_MP;
                break;
            case 0xEA:
                pmt->es[idx].type = VIDEO_VC1_MP;
                break;
            case 0x42:
                pmt->es[idx].type = VIDEO_AVS_MP;
                break;
            case 0x83:
                if (AUDIO_A52_MP == pmt->es[idx].type) {
                    pmt->es[idx].type = AUDIO_TRUEHD_AC3_MP;
                } else {
                    pmt->es[idx].type = AUDIO_TRUEHD_MP;
                }
                break;
            default:
                MLOGA("UNKNOWN_MP ES TYPE=0x%x\n", es_type);
                pmt->es[idx].type = UNKNOWN_MP;
        }

        tss = priv->ts.pids[es_pid];            //an ES stream
        if (tss == NULL) {
            tss = new_pid(priv, es_pid);
            if (tss) {
                tss->type = pmt->es[idx].type;
            }
        } else {
            //if(tss->type != pmt->es[idx].type)
            //    mtos_printk("zx es_pid %d tss->type 0x%x will update\n", es_pid, tss->type);
            tss->type = pmt->es[idx].type;
        }

        section_bytes -= 5 + pmt->es[idx].descr_length;
        MLOGD("PARSE_PMT(%d INDEX %d), STREAM: %d, FOUND pid=0x%x (%d), type=0x%x, ES_DESCR_LENGTH: %d, bytes left: %d\n",
              progid, idx, es_count, pmt->es[idx].pid, pmt->es[idx].pid, pmt->es[idx].type, pmt->es[idx].descr_length, section_bytes);


        es_base += 5 + pmt->es[idx].descr_length;

        es_count++;
    }

    MLOGD("----------------------------\n");
    return 1;
}

static pmt_t *pmt_of_pid(ts_priv_t *priv, int pid, mp4_decoder_config_t **mp4_dec)
{
    int32_t i, j, k;

    if (priv->pmt) {
        for (i = 0; i < priv->pmt_cnt; i++) {
            if (priv->pmt[i].es && priv->pmt[i].es_cnt) {
                for (j = 0; j < priv->pmt[i].es_cnt; j++) {
                    if (priv->pmt[i].es[j].pid == pid) {
                        //search mp4_es_id
                        if (priv->pmt[i].es[j].mp4_es_id) {
                            for (k = 0; k < priv->pmt[i].mp4es_cnt; k++) {
                                if (priv->pmt[i].mp4es[k].id == priv->pmt[i].es[j].mp4_es_id) {
                                    *mp4_dec = &(priv->pmt[i].mp4es[k].decoder);
                                    break;
                                }
                            }
                        }

                        return &(priv->pmt[i]);
                    }
                }
            }
        }
    }

    return NULL;
}

static int32_t pid_type_from_pmt(ts_priv_t *priv, int pid)
{
    int32_t pmt_idx, pid_idx, i, j;

    pmt_idx = progid_idx_in_pmt(priv, priv->prog);

    if (pmt_idx != -1) {
        pid_idx = es_pid_in_pmt(&(priv->pmt[pmt_idx]), pid);
        if (pid_idx != -1) {
            return priv->pmt[pmt_idx].es[pid_idx].type;
        }
    }
    for (i = 0; i < priv->pmt_cnt; i++) {
        pmt_t *pmt = &(priv->pmt[i]);
        for (j = 0; j < pmt->es_cnt; j++)
            if (pmt->es[j].pid == pid) {
                return pmt->es[j].type;
            }
    }

    return UNKNOWN_MP;
}


static uint8_t *pid_lang_from_pmt(ts_priv_t *priv, int pid)
{
    int32_t pmt_idx, pid_idx, i, j;

    pmt_idx = progid_idx_in_pmt(priv, priv->prog);

    if (pmt_idx != -1) {
        pid_idx = es_pid_in_pmt(&(priv->pmt[pmt_idx]), pid);
        if (pid_idx != -1) {
            return priv->pmt[pmt_idx].es[pid_idx].lang;
        }
    } else {
        for (i = 0; i < priv->pmt_cnt; i++) {
            pmt_t *pmt = &(priv->pmt[i]);
            for (j = 0; j < pmt->es_cnt; j++)
                if (pmt->es[j].pid == pid) {
                    return pmt->es[j].lang;
                }
        }
    }

    return NULL;
}

#define FLOAT_COMPARE(a, b) ((a) - (b))
static int fill_packet(demuxer_t *demuxer, demux_stream_t *ds, demux_packet_t **dp, int *dp_offset, TS_stream_info *si)
{
    int ret = 0;

    if (*dp && *dp_offset <= 0) {
        free_demux_packet(*dp);
        *dp = NULL;
    }
    if (*dp) {
        int ret_resize = 0;
        ret = *dp_offset;
        ret_resize = resize_demux_packet(*dp, ret); //shrinked to the right size
        ts_priv_t *priv = (ts_priv_t *) demuxer->priv;
        if (ret_resize == -1) {
            free_demux_packet(*dp);
            *dp = NULL;
            *dp_offset = 0;
            return 0;
        }

        double dp_pts = (*dp)->pts;
        parse_es_add_packet(demuxer, ds, &(priv->parser_handle), *dp);
        MLOGA("ADDED %d  bytes to %s fifo, PTS=%.3f\n", ret,
              (ds == demuxer->audio ? "audio" : (ds == demuxer->video ? "video" : "sub")), dp_pts);
        if (si) {
            float dur;
            float diff = dp_pts - si->last_pts;

            if (abs(diff) > 1) { //1 second, there's a discontinuity
                si->duration += si->last_pts - si->first_pts;
                si->first_pts = si->last_pts = dp_pts;
            } else {
                si->last_pts = dp_pts;
            }

            si->size += ret;
            dur = si->duration + (si->last_pts - si->first_pts);
            if (dur > 0 && ds == demuxer->video) {
                if (dur > 1) { //otherwise it may be unreliable
                    priv->vbitrate = (uint32_t)((float) si->size / dur);
                }
            }
        }
    }

    *dp = NULL;
    *dp_offset = 0;

    return ret;
}

static int fill_extradata(mp4_decoder_config_t *mp4_dec, ES_stream_t *tss)
{
    uint8_t *tmp;

    MLOGA("MP4_dec: %p, pid: %d\n", mp4_dec, tss->pid);

    if (mp4_dec->buf_size > tss->extradata_alloc) {
        tmp = (uint8_t *)realloc33(tss->extradata, mp4_dec->buf_size);
        if (!tmp) {
            return 0;
        }
        tss->extradata = tmp;
        tss->extradata_alloc = mp4_dec->buf_size;
    }
    memcpy(tss->extradata, mp4_dec->buf, mp4_dec->buf_size);
    tss->extradata_len = mp4_dec->buf_size;
    MLOGD("EXTRADATA: %p, alloc=%d, len=%d\n", tss->extradata, tss->extradata_alloc, tss->extradata_len);

    return tss->extradata_len;
}

// 0 = EOF or no stream found
// else = [-] number of bytes written to the packet
static int ts_parse(demuxer_t *demuxer, ES_stream_t *es, unsigned char *packet, int probe)
{
    ES_stream_t *tss;
    int buf_size, is_start, pid, base;
    int len, cc, /*cc_ok,*/ afc, retv = 0, is_video, is_audio, is_sub;
    int transport_scrambling_control;
    ts_priv_t *priv = (ts_priv_t *) demuxer->priv;
    stream_t *stream = demuxer->stream;
    unsigned char *p = NULL;
    demux_stream_t *ds = NULL;
    demux_packet_t **dp = NULL;
    int *dp_offset = 0, *buffer_size = 0;
    int32_t progid, pid_type, bad, ts_error;
    int junk = 0, rap_flag = 0;
    pmt_t *pmt;
    mp4_decoder_config_t *mp4_dec;
    TS_stream_info *si;
    int v_a_bytes = 0;
    unsigned int cur_ticks = 0, start_ticks = 0;

    memset(es, 0, sizeof(*es));
    start_ticks = mtos_ticks_get();
    while ((is_file_seq_exit() == MT_FALSE)) {
        cur_ticks = mtos_ticks_get();
        if (cur_ticks >= start_ticks) {
            cur_ticks -= start_ticks;
        } else {
            cur_ticks = start_ticks - cur_ticks;
        }

        /* probe 3 seconds, no packet then quit */
        if (cur_ticks >= 300) {
            return 0;
        }

        if (is_file_seq_exit()) {
            return 0;
        }
        bad = ts_error = 0;
        ds = NULL;
        dp = NULL;
        dp_offset = buffer_size = NULL;
        rap_flag = 0;
        mp4_dec = NULL;
        es->is_synced = 0;
        es->lang[0] = 0;
        si = NULL;
        v_a_bytes = demuxer->audio->bytes + demuxer->video->bytes;
/* S6 use dynamic memory */
#ifndef CONFIG_MT_MONTAGE_PLATFORM
        if (v_a_bytes > MAX_PACK_BYTES) {
            // tizhang @20190320 to fix 108787 .memory pool smaller 0xb0000, taipei_101 loop at here loop
            MLOGW("[ts_parse]vpacks[%d],apacks[%d],vsizes[%d],asize[%d]\n",
                  demuxer->video->packs, demuxer->audio->packs, demuxer->video->bytes, demuxer->audio->bytes);
            return 0;
        }
#endif
        //NO AUDIO DATA
        if (demuxer->audio->sh == NULL && v_a_bytes > 0x200000) { //for check_stream_fps while1
            MLOGD("11[ts_parse]vpacks[%d],apacks[%d],vsizes[%d],asize[%d]\n",
                  demuxer->video->packs, demuxer->audio->packs, demuxer->video->bytes, demuxer->audio->bytes);
            return 0;
        }

        junk = priv->ts.packet_size - TS_PACKET_SIZE;
        buf_size = priv->ts.packet_size - junk;
        if (stream_eof(stream)) {
            if (! probe) {
                ts_drain_reamin_fifo(demuxer, priv);
                demuxer->filepos = stream_tell(demuxer->stream);
            }

            return 0;
        }


        if (! ts_sync(stream)) {
            MLOGA("TS_PARSE: COULDN'T SYNC\n");
            return 0;
        }

        len = stream_read(stream, &packet[1], 3);
        if (len != 3) {
            return 0;
        }

        buf_size -= 4;
        if ((packet[1]  >> 7) & 0x01) { //transport error
            ts_error = 1;
        }

        is_start = packet[1] & 0x40;
        pid = ((packet[1] & 0x1f) << 8) | packet[2];
        tss = priv->ts.pids[pid];           //an ES stream
        if (tss == NULL) {
            tss = new_pid(priv, pid);
            if (tss == NULL) {
                continue;
            }
        }

        transport_scrambling_control = (packet[3] & 0xc0);
        cc = (packet[3] & 0xf);
        // cc_ok = (tss->last_cc < 0) || ((((tss->last_cc + 1) & 0x0f) == cc));
        tss->last_cc = cc;
        bad = ts_error; // || (! cc_ok);
        if (bad) {
            if (priv->keep_broken == 0) {
                stream_skip(stream, buf_size + junk);
                continue;
            }
            is_start = 0; // queued to the packet data
        }

        if (is_start) {
            tss->is_synced = 1;
        }

        if ((!is_start && !tss->is_synced) || ((pid > 1) && (pid < 16)) || (pid == 8191)) {     // invalid pid
            int is_reserved_pmt_pid = 0;
            /* PID values 0x0005-0x000F are reserved accroiding to H.222.0 */
            if ((pid >= 5) && (pid <= 15) &&
                (-1 != prog_id_in_pat(priv, pid))) {
                is_reserved_pmt_pid = 1;
            }
            if (!is_reserved_pmt_pid) {
                stream_skip(stream, buf_size + junk);
                continue;
            }
        }

        afc = (packet[3] >> 4) & 3;
        if (!(afc % 2)) { // no payload in this TS packet
            stream_skip(stream, buf_size + junk);
            continue;
        }
        if (transport_scrambling_control) {
            tss->scrambled_cnt++;
            stream_skip(stream, buf_size + junk);
            continue;
        } else {
            tss->not_scrambled_cnt++;
        }

        if (afc > 1) {
            int c;
            c = stream_read_char(stream);
            buf_size--;
            if (c < 0 || c > 183) { //broken from the stream layer or invalid
                stream_skip(stream, buf_size + junk);
                continue;
            }

            //c==0 is allowed!
            if (c > 0) {
                uint8_t pcrbuf[188];
                int flags = stream_read_char(stream);
                int has_pcr;
                rap_flag = (flags & 0x40) >> 6;
                has_pcr = flags & 0x10;

                buf_size--;
                c--;
                stream_read(stream, pcrbuf, c);

                if (has_pcr) {
                    int pcr_pid = prog_pcr_pid(priv, priv->prog);
                    if (pcr_pid == pid) {
                        uint64_t pcr, pcr_ext;

                        pcr  = (int64_t)(pcrbuf[0]) << 25;
                        pcr |=  pcrbuf[1]         << 17 ;
                        pcr |= (pcrbuf[2]) << 9;
                        pcr |=  pcrbuf[3]  <<  1 ;
                        pcr |= (pcrbuf[4] & 0x80) >>  7;

                        pcr_ext = (uint64_t)((pcrbuf[4] & 0x01) << 8) ;
                        pcr_ext |= pcrbuf[5];

                        pcr = pcr * 300 + pcr_ext;
                        demuxer->reference_clock = (double)pcr / (double)27000000.0;
                    }
                }

                buf_size -= c;
                if (buf_size == 0) {
                    continue;
                }
            }
        }

        //find the program that the pid belongs to; if (it's the right one or -1) && pid_type==SL_SECTION_MP
        //call parse_sl_section()
        pmt = pmt_of_pid(priv, pid, &mp4_dec);
        if (mp4_dec) {
            fill_extradata(mp4_dec, tss);
            if (IS_VIDEO(mp4_dec->object_type) || IS_AUDIO(mp4_dec->object_type)) {
                tss->type = SL_PES_STREAM_MP;
                tss->subtype = mp4_dec->object_type;
            }
        }

        //TABLE PARSING
        priv->last_pid = pid;
        base = priv->ts.packet_size - buf_size;
        is_video = IS_VIDEO(tss->type) || (tss->type == SL_PES_STREAM_MP && IS_VIDEO(tss->subtype));
        is_audio = IS_AUDIO(tss->type) || (tss->type == SL_PES_STREAM_MP && IS_AUDIO(tss->subtype)) || (tss->type == PES_PRIVATE1_MP);
        is_sub   = IS_SUB(tss->type);
        pid_type = pid_type_from_pmt(priv, pid);

        // PES CONTENT STARTS HERE
        if (! probe) {
            if ((is_video || is_audio || is_sub) && is_start) {
                ts_add_stream(demuxer, tss);
            }

            if (is_video && (demuxer->video->id == priv->ts.streams[pid].id)) {
                ds = demuxer->video;

                dp = &priv->fifo[1].pack;
                dp_offset = &priv->fifo[1].offset;
                buffer_size = &priv->fifo[1].buffer_size;
                si = &priv->vstr;
            } else if (is_audio && (demuxer->audio->id == priv->ts.streams[pid].id)) {
                ds = demuxer->audio;

                dp = &priv->fifo[0].pack;
                dp_offset = &priv->fifo[0].offset;
                buffer_size = &priv->fifo[0].buffer_size;
                si = &priv->astr;
            } else if (is_sub) {
                sh_sub_t *sh_sub = demuxer->sub->sh;

                if (sh_sub && sh_sub->sid == tss->pid) {
                    ds = demuxer->sub;

                    dp = &priv->fifo[2].pack;
                    dp_offset = &priv->fifo[2].offset;
                    buffer_size = &priv->fifo[2].buffer_size;
                } else {
                    stream_skip(stream, buf_size + junk);
                    continue;
                }
            }
            //linda zhu add, to get only one frame in one packet in MPEG2
            //IS IT TIME TO QUEUE DATA to the dp_packet?
            if (is_start && (dp != NULL)) {
                retv = fill_packet(demuxer, ds, dp, dp_offset, si);
            }

            if (dp && *dp == NULL) {
                if (*buffer_size > MAX_PACK_BYTES) {
                    *buffer_size = MAX_PACK_BYTES;
                }
                *dp = new_demux_packet(*buffer_size);   //es->size
                *dp_offset = 0;
                if (! *dp) {
                    MLOGW("fill_buffer, NEW_ADD_PACKET(%d)FAILED\n", *buffer_size);
                    return 0;
                }
                MLOGA("CREATED DP(%d)\n", *buffer_size);
            }
        }

        if (probe || !dp) { //dp is NULL for tables and sections
            p = &packet[base];
        } else { //feeding
            if ((*dp) && (*dp_offset + buf_size > *buffer_size)) {
                if (ds == demuxer->video && *buffer_size < 1025 * 1024 && fp_is_timeshift_file() == 0) {
                    *buffer_size *= 2;
                } else {
                    *buffer_size = *dp_offset + buf_size + TS_FEC_PACKET_SIZE;
                }
                {
                    int ret_resize = 0;

                    ret_resize = resize_demux_packet(*dp, *buffer_size);
                    if (ret_resize == -1) {
                        free_demux_packet(*dp);
                        *dp = NULL;
                        *dp_offset = 0;

                        return 0;
                    }
                }
            }
            if ((*dp) && (*dp)->buffer == NULL) {
                return 0;
            }
            if ((*dp) && (*dp)->buffer) {
                p = &((*dp)->buffer[*dp_offset]);
            }
        }

        len = stream_read(stream, p, buf_size);
        if (len < buf_size) {
            MLOGA("ts_parse() couldn't read enough data: %d < %d\r\n", len, buf_size);
            continue;
        }
        stream_skip(stream, junk);

        if (pid == 0) {
            parse_pat(priv, is_start, p, buf_size);
            continue;
        } else if ((tss->type == SL_SECTION_MP) && pmt) {
            int k, mp4_es_id = -1;
            ts_section_t *section;
            for (k = 0; k < pmt->mp4es_cnt; k++) {
                if (pmt->mp4es[k].decoder.object_type == MP4_OD_MP && pmt->mp4es[k].decoder.stream_type == MP4_OD_MP) {
                    mp4_es_id = pmt->mp4es[k].id;
                }
            }
            MLOGA("MP4ESID: %d\n", mp4_es_id);
            for (k = 0; k < pmt->es_cnt; k++) {
                if (pmt->es[k].mp4_es_id == mp4_es_id) {
                    section = &(tss->section);
                    parse_sl_section(pmt, section, is_start, &packet[base], buf_size);
                }
            }
            continue;
        } else {
            progid = prog_id_in_pat(priv, pid);
            if (progid != -1) {
                if (pid != demuxer->video->id && pid != demuxer->audio->id && pid != demuxer->sub->id) {
                    parse_pmt(priv, progid, pid, is_start, &packet[base], buf_size);
                    continue;
                } else {
                    MLOGE("Argh! Data pid %d used in the PMT, Skipping PMT parsing!\n", pid);
                }
            }
        }

        if (!probe && !dp) {
            continue;
        }

        if (is_start) {
            uint8_t *lang = NULL;

            MLOGA("IS_START\n");

            len = pes_parse2(p, buf_size, es, pid_type, pmt, pid);
            if (! len) {
                tss->is_synced = 0;
                continue;
            }

            es->pid = tss->pid;
            tss->is_synced |= es->is_synced || rap_flag;
            tss->payload_size = es->payload_size;

            if ((is_sub || is_audio) && (lang = pid_lang_from_pmt(priv, es->pid))) {
                memcpy(es->lang, lang, 3);
                es->lang[3] = 0;
            } else {
                es->lang[0] = 0;
            }

            if (probe) {
                if (es->type == UNKNOWN_MP) {
                    return 0;
                }

                tss->type = es->type;
                tss->subtype = es->subtype;

                return 1;
            } else {
                if (FLOAT_COMPARE(es->pts, 0.0) == 0.0f) {
                    es->pts = tss->pts = tss->last_pts;
                    es->pts31bit = tss->pts31bit;
                } else {
                    tss->pts = tss->last_pts = es->pts;
                    tss->pts31bit = es->pts31bit;
                }

                MLOGA("ts_parse, NEW pid=%d, PSIZE: %u, type=%X, start=%p, len=%d\n",
                      es->pid, es->payload_size, es->type, es->start, es->size);

                demuxer->filepos = stream_tell(demuxer->stream) - es->size;

                if (es->size < 0 || es->size > buf_size) {
                    MLOGE("Broken ES packet size\n");
                    es->size = 0;
                }
                memmove(p, es->start, es->size);
                *dp_offset += es->size;
                (*dp)->flags = 0;
                (*dp)->pos = stream_tell(demuxer->stream);
                (*dp)->pts = es->pts;
                (*dp)->pts31bit = es->pts31bit;
                // subtitle packets must be returned immediately if possible
                if (is_sub && !tss->payload_size) {
                    retv = fill_packet(demuxer, ds, dp, dp_offset, si);
                }

                if (retv > 0) {
                    return retv;
                } else {
                    continue;
                }
            }
        } else {
            uint16_t sz;

            es->pid = tss->pid;
            es->type = tss->type;
            es->subtype = tss->subtype;
            es->pts = tss->pts = tss->last_pts;
            es->start = &packet[base];

            if (tss->payload_size > 0) {
                sz = FFMIN(tss->payload_size, buf_size);
                tss->payload_size -= sz;
                es->size = sz;
            } else {
                if (is_video) {
                    sz = es->size = buf_size;
                } else {
                    continue;
                }
            }

            if (! probe) {
                *dp_offset += sz;

                // subtitle packets must be returned immediately if possible
                if (*dp_offset >= MAX_PACK_BYTES || (is_sub && !tss->payload_size)) {
                    (*dp)->pts = tss->last_pts;
                    retv = fill_packet(demuxer, ds, dp, dp_offset, si);
                    return 1;
                }

                continue;
            } else {
                memmove(es->start, p, sz);

                if (es->size) {
                    return es->size;
                } else {
                    continue;
                }
            }
        }
    }

    return 0;
}

static void reset_fifos(demuxer_t *demuxer, int a, int v, int s)
{
    ts_priv_t *priv = demuxer->priv;
    if (a) {
        if (priv->fifo[0].pack != NULL) {
            free_demux_packet(priv->fifo[0].pack);
            priv->fifo[0].pack = NULL;
        }
        priv->fifo[0].offset = 0;
    }

    if (v) {
        if (priv->fifo[1].pack != NULL) {
            free_demux_packet(priv->fifo[1].pack);
            priv->fifo[1].pack = NULL;
        }
        priv->fifo[1].offset = 0;
    }

    if (s) {
        if (priv->fifo[2].pack != NULL) {
            free_demux_packet(priv->fifo[2].pack);
            priv->fifo[2].pack = NULL;
        }
        priv->fifo[2].offset = 0;
    }
    demuxer->reference_clock = MP_NOPTS_VALUE;
}

static void reset_seek_ctx(demuxer_t *demuxer)
{
    int i;
    demux_stream_t *d_audio = demuxer->audio;
    demux_stream_t *d_video = demuxer->video;
    sh_audio_t *sh_audio = d_audio->sh;
    sh_video_t *sh_video = d_video->sh;
    ts_priv_t  *priv     = (ts_priv_t *) demuxer->priv;

    ts_drain_reamin_fifo(demuxer, demuxer->priv);
    reset_fifos(demuxer, NULL != sh_audio, NULL != sh_video, demuxer->sub->id > 0);
    parse_es_flush(demuxer, &(priv->parser_handle));

    for (i = 0; i < NB_PID_MAX; i++) {
        if (priv->ts.pids[i] != NULL) {
            priv->ts.pids[i]->is_synced = 0;
            priv->ts.pids[i]->last_pts = 0;
            priv->ts.pids[i]->pts31bit = 0;
        }
    }
}

static int stream_seek_get_valid_pts(
    demuxer_t *demuxer, off_t seek_pos, double *pts)
{
    int i, ret;
    ES_stream_t es_info = {0};
    const static int MAX_TRY_CNT = 100;
    unsigned char buff[TS_FEC_PACKET_SIZE] = {0};
    demux_stream_t *ds_v = demuxer->video;
    ts_priv_t      *priv = (ts_priv_t *) demuxer->priv;

    *pts = MP_NOPTS_VALUE;
    (void) stream_seek(demuxer->stream, seek_pos);
    for (i = 0; i < MAX_TRY_CNT; i++) {
        es_info.pid = priv->selected_vpid;
        ret = ts_parse_pts(demuxer, &es_info, buff, 0);
        if (1 == ret && es_info.pts != MP_NOPTS_VALUE &&
            ds_v->id == priv->ts.streams[es_info.pid].id) {
            *pts = es_info.pts;
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}

static off_t find_wanted_seek_pos(demuxer_t *demuxer,
                                  const double request_time, const double time_start, const double time_end,
                                  const off_t pos_start, const off_t pos_end, const int try_cnt)
{
    off_t  seek_pos;
    double local_rate;
    double searched_pts;
    double t_start = time_start;
    double t_end   = time_end;
    off_t  ps_start = pos_start;
    off_t  ps_end   = pos_end;
    /* range [request_time - 1, request_time + 1] will be taken */
    const static int DRIFT       = 1;
    const double SEEK_LOW_TH  = MAX(time_start, request_time - DRIFT);
    const double SEEK_HIGH_TH = MIN(time_end, request_time + DRIFT);

    if (try_cnt <= 0               || MP_NOPTS_VALUE == time_start   ||
        MP_NOPTS_VALUE == time_end || MP_NOPTS_VALUE == request_time ||
        pos_start      == pos_end  || time_start     == time_end     || request_time <= time_start) {
        return pos_start;
    }

    if (request_time >= time_end) {
        return pos_end;
    }

    local_rate = (ps_end - ps_start) / (t_end - t_start);
    seek_pos   = ps_start + (off_t)(local_rate * (request_time - t_start));
    if (MT_FALSE ==
        stream_seek_get_valid_pts(demuxer, seek_pos, &searched_pts)) {
        return seek_pos;
    }

    if (searched_pts >= SEEK_LOW_TH && searched_pts <= SEEK_HIGH_TH) {
        MLOGD("try cnt:%d low:%lf, hi:%lf s:%lf e:%lf\n",
              try_cnt, SEEK_LOW_TH, SEEK_HIGH_TH, time_start, time_end);
        return seek_pos;
    }

    /* avergae bytes rate lower,
     * pts in the tail section, discard low section
     */
    if (searched_pts < SEEK_LOW_TH) {
        ps_start = seek_pos;
        t_start  = searched_pts;
    }

    /* avergae bytes rate higher,
     * pts in the section ahead, discard tail section
     */
    if (searched_pts > SEEK_HIGH_TH) {
        ps_end = seek_pos;
        t_end  = searched_pts;
    }

    return find_wanted_seek_pos(demuxer, request_time,
                                t_start, t_end, ps_start, ps_end, try_cnt - 1);
}

static void demux_seek_ts(demuxer_t *demuxer, float seek_secs, float audio_delay, int flags)
{
    float rel_seek_secs = seek_secs;
    demux_stream_t *d_audio = demuxer->audio;
    demux_stream_t *d_video = demuxer->video;
    const static int MAX_TRY_CNT = 60;
    int i_unfind_cnt = 0;
    double start_pts = 0.0;

    if (d_audio == NULL || d_video == NULL) {
        return;
    }
    sh_audio_t *sh_audio = d_audio->sh;
    sh_video_t *sh_video = d_video->sh;
    ts_priv_t *priv = (ts_priv_t *) demuxer->priv;
    int i, video_ibps = 0, audio_ibps = 0, part_bps = 0;
    off_t seek_pos;
    unsigned int cur_ticks = 0, start_ticks = 0;

    if (d_video) {
        ds_fill_buffer(d_video);
        start_pts = d_video->pts;
    }
    //================= seek in MPEG-TS ==========================
    reset_seek_ctx(demuxer);

    //get video_ibps
    if (sh_video != NULL) {
        video_ibps = sh_video->i_bps;
    }
    if (video_ibps && video_ibps < 10 * 1024) {
        video_ibps = 2324 * 75;
    }

    //get audio_ibps
    if (sh_audio != NULL) {
        audio_ibps = sh_audio->i_bps;
    } else {
        audio_ibps = 2324 * 75;
    }
    if(start_pts>0 &&
        demuxer->filepos > demuxer->movi_start &&
        start_pts > priv->file_first_pts){
        part_bps = (demuxer->filepos - demuxer->movi_start)*1.0/(start_pts - priv->file_first_pts);
    }
    //calculate the seek_pos
    seek_pos = (flags & SEEK_ABSOLUTE) ? demuxer->movi_start : demuxer->filepos;
    if (flags & SEEK_FACTOR) { // float seek 0..1
        seek_pos += (demuxer->movi_end - demuxer->movi_start) * rel_seek_secs;
    } else {
        if ((flags & SEEK_ABSOLUTE) && rel_seek_secs > 0.0f && NULL != sh_video             &&
            MP_NOPTS_VALUE != priv->file_first_pts && MP_NOPTS_VALUE != priv->file_last_pts &&
            priv->file_first_pts != priv->file_last_pts) {

            seek_pos = find_wanted_seek_pos(demuxer, rel_seek_secs + priv->file_first_pts,
                                            priv->file_first_pts, priv->file_last_pts,
                                            demuxer->movi_start, demuxer->movi_end, MAX_TRY_CNT);
            reset_seek_ctx(demuxer);
        } else if(part_bps >0) {
            seek_pos += (off_t)(((off_t) part_bps) * rel_seek_secs);
        } else if (video_ibps == 0 && audio_ibps != 0) { /* unspecified or VBR */
            seek_pos += (off_t)(((off_t) audio_ibps) * rel_seek_secs);
        } else {
            seek_pos += (off_t)(((off_t) video_ibps) * rel_seek_secs);
        }
    }

    //adjust the seek_pos
    if (seek_pos < demuxer->movi_start) {
        seek_pos = demuxer->movi_start;
    }
    //jqw@20190826 for bug112134
    if (seek_pos < priv->seek_start_filepos || ((start_pts > 0) && (start_pts + seek_secs) < 0)) {
        //add "start_pts > 0" judgement for bug 25650
        seek_pos = priv->seek_start_filepos;
    }

    priv->seek_stop_flag = 0;
    stream_seek(demuxer->stream, seek_pos);
    //jqw@20190107 for bug106866
    if (priv->seek_stop_filepos > 0) {
        priv->mark_filepos = stream_tell(demuxer->stream);
    }

    videobuf_code_len = 0;
    if (sh_video != NULL) {
        ds_fill_buffer(d_video);
    }

    if (sh_audio != NULL) {
        ds_fill_buffer(d_audio);
    }

    start_ticks = mtos_ticks_get();
    cur_ticks = mtos_ticks_get();

    if (sh_audio != NULL && priv->selected_apid > 0 && priv->selected_apid < NB_PID_MAX && priv->ts.pids[priv->selected_apid] != NULL) {
        sh_audio->format = priv->ts.pids[priv->selected_apid]->type;
    }
    if (sh_video != NULL && priv->selected_vpid > 0 && priv->selected_vpid < NB_PID_MAX && priv->ts.pids[priv->selected_vpid] != NULL) {
        sh_video->format = priv->ts.pids[priv->selected_vpid]->type;
    }

    while (sh_video != NULL && (is_file_seq_exit() == MT_FALSE)) {
        cur_ticks = mtos_ticks_get();
        if (cur_ticks - start_ticks > 300 && fp_is_loadmedia_state()) {
            break;
        }
        int start_len;
        i = sync_video_packet(d_video ,&start_len);
        //jqw@20190107 for bug106866
        if (priv->seek_stop_filepos > 0 && seek_secs < 0) {   // this only fb
            i_unfind_cnt ++;
            if ((i_unfind_cnt > 30 && (stream_tell(demuxer->stream) >= priv->seek_stop_filepos)) || (d_video->pts > start_pts)) { // xingwei@20230725 for bug15247
                priv->seek_stop_flag = 1;
                break;
            }
        }

        if ((sh_video->format == VIDEO_MPEG1_MP) || (sh_video->format == VIDEO_MPEG2_MP)) {
            if (i == 0x1B3 || i == 0x1B8) {
                d_video->buffer_pos -= start_len;
                break; // found it!
            }
        } else if ((sh_video->format == VIDEO_AVS_MP) && (i == 0x1B3 || i == 0x1B0)) {
            d_video->buffer_pos = d_video->buffer_pos - start_len;
            break;
        } else if ((sh_video->format == VIDEO_AVS2_MP) && (i == 0x1B3 || i == 0x1B0)) {
            d_video->buffer_pos = d_video->buffer_pos - start_len;
            break;
        } else if ((sh_video->format == VIDEO_MPEG4_MP) &&
                   (i == 0x1B0 || i == 0x1B3 || i == 0x1B5 || ((i >= 0x100) && (i <= 0x11F)))) {
            d_video->buffer_pos = d_video->buffer_pos - start_len;
            break;
        } else if (sh_video->format == VIDEO_VC1_MP && (i == VC1_SEQUENCE_HEADER)) {
            d_video->buffer_pos -= start_len;
            break;
        } else if (sh_video->format == VIDEO_HEVC_MP &&
                   (HEVC_NAL_TYPE(i) == HEVC_NAL_VPS || HEVC_NAL_TYPE(i) == HEVC_NAL_SPS || IS_HEVC_IRAP_NAL(i))) {
            d_video->buffer_pos = d_video->buffer_pos - start_len;
            if ((sh_audio != NULL) && (d_audio != NULL) &&
                (d_video->pts - d_audio->pts > 0.001) && (is_file_seq_exit() == MT_FALSE)) {
                int audio_presync_cnt = 5;
                do {
                    ds_free_packs(d_audio);
                    ds_fill_buffer(d_audio);
                } while (audio_presync_cnt-- > 0 && (d_video->pts - d_audio->pts > 0.001) && (is_file_seq_exit() == MT_FALSE));
            }

            break;
        } else { //H264
            if ((i & ~0x60) == 0x105 || (i & ~0x60) == 0x107) {
                d_video->buffer_pos = d_video->buffer_pos - start_len;
                break;
            }
        }

        if (!i || !skip_video_packet(d_video)) {
            break;    // EOF?
        }
    }
}

static int demux_ts_fill_buffer(
    demuxer_t *demuxer, demux_stream_t *ds)
{
    int ret;
    ES_stream_t es;
    ts_priv_t *priv = (ts_priv_t *)demuxer->priv;

    ret = -ts_parse(demuxer, &es, (unsigned char *) priv->packet, 0);
    if (ds->eof) {
        /* Send the last packet if exsit */
        parse_es_add_packet(demuxer, ds, &(priv->parser_handle), NULL);
    }

    return ret;
}

static int ts_check_file_dmx(demuxer_t *demuxer)
{
    int ret = ts_check_file(demuxer) ? DEMUXER_TYPE_MPEG_TS : 0;

    return ret;
}

static int is_usable_program(ts_priv_t *priv, pmt_t *pmt)
{
    int j;

    for (j = 0; j < pmt->es_cnt; j++) {
        if (priv->ts.pids[pmt->es[j].pid] == NULL || priv->ts.streams[pmt->es[j].pid].sh == NULL) {
            continue;
        }
        if (
            priv->ts.streams[pmt->es[j].pid].type == TYPE_VIDEO ||
            priv->ts.streams[pmt->es[j].pid].type == TYPE_AUDIO
        ) {
            return 1;
        }
    }

    return 0;
}

static int demux_ts_control(demuxer_t *demuxer, int cmd, void *arg)
{
    ts_priv_t *priv = (ts_priv_t *)demuxer->priv;

    switch (cmd) {
        case DEMUXER_CTRL_SWITCH_AUDIO:
        case DEMUXER_CTRL_SWITCH_VIDEO: {
            void *sh = NULL;
            int i, n;
            demux_stream_t *ds;
            unsigned short *selected_pid;
            int reftype, areset = 0, vreset = 0;

            if (cmd == DEMUXER_CTRL_SWITCH_VIDEO) {
                reftype = TYPE_VIDEO;
                ds = demuxer->video;
                vreset = 1;
                selected_pid = &priv->selected_vpid;
            } else {
                reftype = TYPE_AUDIO;
                ds = demuxer->audio;
                areset = 1;
                selected_pid = &priv->selected_apid;
            }

            n = *((int *)arg);
            if (n == -2) {
                reset_fifos(demuxer, areset, vreset, 0);
                ds->id = -2;
                ds->sh = NULL;
                ds_free_packs(ds);
                *((int *)arg) = ds->id;
                return DEMUXER_CTRL_OK;
            }

            if (n < 0) {
                for (i = 0; i < 8192; i++) {
                    if (priv->ts.streams[i].id == ds->id &&
                        priv->ts.streams[i].type == reftype) {
                        break;
                    }
                }

                while (!sh && (is_file_seq_exit() == MT_FALSE)) {
                    i = (i + 1) % 8192;
                    if (priv->ts.streams[i].type == reftype) {
                        if (priv->ts.streams[i].id == ds->id) { //we made a complete loop
                            break;
                        }
                        sh = priv->ts.streams[i].sh;
                    }
                }
            } else { //audio track <n>
                if (n >= 8192 || priv->ts.streams[n].type != reftype) {
                    return DEMUXER_CTRL_NOTIMPL;
                }
                i = n;
                sh = priv->ts.streams[i].sh;
            }

            if (sh) {
                ds->sh = sh;
                if (ds->id != priv->ts.streams[i].id) {
                    ds->id = priv->ts.streams[i].id;
                    reset_fifos(demuxer, areset, vreset, 0);
                    parse_es_reset_parser(demuxer, ds, &(priv->parser_handle));
                }
                ds_free_packs(ds);
                MLOGI("Switch to audio pid %d, id: %d, sh: %p\n", i, ds->id, sh);
            }
            *selected_pid = i;
            *((int *)arg) = ds->id;
            return DEMUXER_CTRL_OK;
        }
        case DEMUXER_CTRL_IDENTIFY_PROGRAM: {   //returns in prog->{aid,vid} the new ids that comprise a program
            int i, j, cnt = 0;
            int vid_done = 0, aid_done = 0;
            pmt_t *pmt = NULL;
            demux_program_t *prog = arg;

            if (priv->pmt_cnt < 2) {
                return DEMUXER_CTRL_NOTIMPL;
            }

            if (prog->progid == -1) {
                int cur_pmt_idx = 0;

                for (i = 0; i < priv->pmt_cnt; i++)
                    if (priv->pmt[i].progid == priv->prog) {
                        cur_pmt_idx = i;
                        break;
                    }

                i = (cur_pmt_idx + 1) % priv->pmt_cnt;
                while (i != cur_pmt_idx) {
                    pmt = &priv->pmt[i];
                    cnt = is_usable_program(priv, pmt);
                    if (cnt) {
                        break;
                    }
                    i = (i + 1) % priv->pmt_cnt;
                }
            } else {
                for (i = 0; i < priv->pmt_cnt; i++)
                    if (priv->pmt[i].progid == prog->progid) {
                        pmt = &priv->pmt[i]; //required program
                        cnt = is_usable_program(priv, pmt);
                    }
            }

            if (!cnt) {
                return DEMUXER_CTRL_NOTIMPL;
            }

            //finally some food
            prog->aid = prog->vid = -2; //no audio and no video by default
            for (j = 0; j < pmt->es_cnt; j++) {
                if (priv->ts.pids[pmt->es[j].pid] == NULL ||
                    priv->ts.streams[pmt->es[j].pid].sh == NULL) {
                    continue;
                }

                if (!vid_done && priv->ts.streams[pmt->es[j].pid].type == TYPE_VIDEO) {
                    vid_done = 1;
                    prog->vid = pmt->es[j].pid;
                } else if (!aid_done && priv->ts.streams[pmt->es[j].pid].type == TYPE_AUDIO) {
                    aid_done = 1;
                    prog->aid = pmt->es[j].pid;
                }
            }

            priv->prog = prog->progid = pmt->progid;
            return DEMUXER_CTRL_OK;
        }
        case DEMUXER_CTRL_GET_SEEK_MARKPOS: {
            *((off_t *)arg) = priv->mark_filepos;
            return DEMUXER_CTRL_OK;
        }
        case DEMUXER_CTRL_SET_SEEK_ENDPOS: {
            priv->seek_stop_filepos = *((off_t *)arg);
            return DEMUXER_CTRL_OK;
        }
        case DEMUXER_CTRL_GET_SEEK_STOPFLAG: {
            *((int *)arg) = priv->seek_stop_flag;
            return DEMUXER_CTRL_OK;
        }
        case DEMUXER_CTRL_SET_SEEK_STARTPOS: {
            priv->seek_start_filepos = *((off_t *)arg);
            return DEMUXER_CTRL_OK;
        }

        default:
            return DEMUXER_CTRL_NOTIMPL;
    }
}

/* Should be wrapped into control command */
#if MT_DES("EXTERNAL PIERCE API", 1)

void ts_get_pcm_info(demuxer_t *demuxer, int *is_big_endian, int *bits, int *channel, int *sample_rate)
{
    ts_priv_t *priv = (ts_priv_t *)demuxer->priv;
    pmt_t *pmt = (pmt_t *)(&priv->pmt[priv->selected_program_idx]);
    if (pmt) {
        *is_big_endian = pmt->pcm_info.is_big_endian;
        *bits = pmt->pcm_info.bits_per_coded_sample;
        *channel = pmt->pcm_info.channels;
        *sample_rate = pmt->pcm_info.sample_rate;
    } else {
        *is_big_endian = 1;
        *channel = 2;
        *bits = 16;
        *sample_rate = 48000;
    }
}

static int select_prefer_audio(pmt_t *pmt)
{
    /* If normal audio stream error, play may fail */
    int prefer_idx = (unsigned int)(-1);
    for (int i = 0; i < pmt->es_cnt; i++) {
        /* if mp2, mp3, aac, our board support very good, we can play directly */
        if (AUDIO_MP2_MP == pmt->es[i].type ||
            AUDIO_AAC_MP == pmt->es[i].type ||
            AUDIO_AAC_LATM_MP == pmt->es[i].type) {
            return i;
        }
        /* pcm may have too much data that cannot play smoothly, no mp2, mp3, aac, we select pcm */
        if (AUDIO_PCM_BR_MP == pmt->es[i].type || AUDIO_LPCM_BE_MP == pmt->es[i].type) {
            if ((-1 == prefer_idx) || (-1 != prefer_idx &&
                AUDIO_PCM_BR_MP != pmt->es[prefer_idx].type &&
                AUDIO_LPCM_BE_MP != pmt->es[prefer_idx].type)) {
                prefer_idx = i;
            }
        /* ac3 and dts maybe need Amplifier, normal cannot play, we select last */
        } else if (AUDIO_A52_MP == pmt->es[i].type ||
                   AUDIO_AC4_MP == pmt->es[i].type ||
                   AUDIO_DTS_MP == pmt->es[i].type) {
            if (prefer_idx == (unsigned int) (-1)) {
                prefer_idx = i;
            }
        }
    }

    return prefer_idx;
}

void set_ts_prog(demuxer_t *demuxer)
{
    ts_priv_t *priv = demuxer->priv;
    pmt_t   *pmt = (pmt_t *)(&priv->pmt[priv->selected_program_idx]);

    if (NULL == pmt || demuxer->audio->sh == NULL) {
        return;
    }
    unsigned short req_apid = 0xffff;
    int prefer_idx = select_prefer_audio(pmt);
    if (-1 != prefer_idx) {
        req_apid = pmt->es[prefer_idx].pid;
    }
    if (req_apid != 0xffff && req_apid != priv->selected_apid && priv->selected_vpid != 0xffff) {
        ES_stream_t *es = priv->ts.pids[req_apid];

        if (!IS_AUDIO(es->type) && !IS_AUDIO(es->subtype) && IS_AUDIO(pmt->es[prefer_idx].type)) {
            es->subtype = pmt->es[prefer_idx].type;
        }
        demuxer_switch_audio(demuxer, req_apid);
        demux_seek_ts(demuxer, 0.0f, 0.0f, SEEK_ABSOLUTE);
    }
    MLOGI("Select pmt:%d aid(%d %d) vid(%d %d)\n", priv->selected_program_idx,
          demuxer->audio->id, priv->selected_apid, demuxer->video->id, priv->selected_vpid);
}
#endif

const demuxer_desc_t demuxer_desc_mpeg_ts = {
    "MPEG-TS demuxer",
    "mpegts",
    "TS",
    "Nico Sabbi",
    "",
    DEMUXER_TYPE_MPEG_TS,
    0, // unsafe autodetect
    ts_check_file_dmx,
    demux_ts_fill_buffer,
    demux_open_ts,
    demux_close_ts,
    demux_seek_ts,
    demux_ts_control
};
