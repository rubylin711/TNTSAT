/*
 * Real parser & demuxer
 * copyright (C) 2001 Alex Beregszaszi
 * copyright (C) 2005, 2006 Roberto Togni
 * based on FFmpeg's libav/rm.c
 *
 * audio codecs: (supported by RealPlayer8 for Linux)
 *  DNET - RealAudio 3.0, really it's AC3 in swapped-byteorder
 *  SIPR - SiproLab's audio codec, ACELP decoder working with MPlayer,
 *         needs fine-tuning too :)
 *  ATRC - RealAudio 8 (ATRAC3) - www.minidisc.org/atrac3_article.pdf,
 *         ACM decoder uploaded, needs some fine-tuning to work
 *         -> RealAudio 8
 *  COOK/COKR - Real Cooker -> RealAudio G2
 *
 * video codecs: (supported by RealPlayer8 for Linux)
 *  RV10 - H.263 based, working with libavcodec's decoder
 *  RV20-RV40 - using RealPlayer's codec plugins
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
#include <unistd.h>
#include <inttypes.h>

#include "config.h"
#include "mp_msg.h"
#include "help_mp.h"
#include "mpbswap.h"
#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "stream/stream.h"
#include "aviprint.h"
#include "demuxer.h"
#include "stheader.h"
#include "file_playback_sequence.h"
#include "demux_real.h"

#define BUFFER_HEADER_EXTEND_FOR_WRITE  64
typedef struct sh_real_opaque_data
{
    unsigned int opauqe_data_size;
    unsigned int media_object_format;
    unsigned short bit_count;
    unsigned short pad_width;
    unsigned short pad_height;
    unsigned char *p_opaque_data;
}sh_real_opaque_data_t;
//#define mp_dbg(mod,lev, args... ) mp_msg_c((mod<<8)|lev, ## args )
typedef struct packet_hdr_s {
    unsigned int slice_header_len;
    unsigned int offset;

    unsigned short TR;
    unsigned short width;
    unsigned short height;
    unsigned short MBA;

    unsigned char ECC;
    unsigned char PicCodType;
    unsigned char SQUANT;
    unsigned char BitStreamVersion;
    unsigned char OSV_QUANT;
    unsigned char Deblock_Pass_Thru;
    unsigned char pixel_aspect_ratio;
    unsigned char broken_up_by_us;

    unsigned char MBA_bits;
    unsigned char start_mb_data;///because reade slice header is not align ,so we read more one byte ,including mb data,this is the valid mb data in this byte
    unsigned char start_mb_data_bitnum;///valid data in first mb data to make data align
} packet_hdr_t;

#define MAX_STREAMS 32
#define MAX_MLTIIDX 16

static unsigned char sipr_swaps[38][2]={
    {0,63},{1,22},{2,44},{3,90},{5,81},{7,31},{8,86},{9,58},{10,36},{12,68},
    {13,39},{14,73},{15,53},{16,69},{17,57},{19,88},{20,34},{21,71},{24,46},
    {25,94},{26,54},{28,75},{29,50},{32,70},{33,92},{35,74},{38,85},{40,56},
    {42,87},{43,65},{45,59},{48,79},{49,93},{51,89},{55,95},{61,76},{67,83},
    {77,80} };

typedef struct {
    unsigned int	timestamp;
    int		offset;
//    int		packetno;
//    int		len; /* only filled by our index generator */
//    int		flags; /* only filled by our index generator */
} real_index_table_t;

typedef struct {
    /* for seeking */
    int		index_chunk_offset;
    real_index_table_t *index_table[MAX_STREAMS];

//    int		*index_table[MAX_STREAMS];
    int		index_table_size[MAX_STREAMS];
    int		index_malloc_size[MAX_STREAMS];
    int		data_chunk_offset;
    int		num_of_packets;
    int		current_packet;
    int		streams_in_file;

// need for seek
    int		audio_need_keyframe;
    int		video_after_seek;

    int		current_apacket;
    int		current_vpacket;

    // timestamp correction:
    int64_t		kf_base;// timestamp of the prev. video keyframe
    unsigned int	kf_pts;	// timestamp of next video keyframe
    unsigned int	a_pts;	// previous audio timestamp
    double	v_pts;  // previous video timestamp
    unsigned long	duration;

    /* stream id table */
//    int		last_a_stream;
//    int 	a_streams[MAX_STREAMS];
//    int		last_v_stream;
//    int 	v_streams[MAX_STREAMS];

    /**
     * Used to demux multirate files
     */
    int is_multirate; ///< != 0 for multirate files
    int str_data_offset[MAX_STREAMS]; ///< Data chunk offset for every audio/video stream
    int audio_curpos; ///< Current file position for audio demuxing
    int video_curpos; ///< Current file position for video demuxing
    int a_num_of_packets; ///< Number of audio packets
    int v_num_of_packets; ///< Number of video packets
    int a_bitrate; ///< Audio bitrate
    int v_bitrate; ///< Video bitrate
    int stream_switch; ///< Flag used to switch audio/video demuxing

   /**
    * Used to demux MLTI files
    */
    int is_mlti; ///< != 0 for MLTI files
    unsigned int mp2rm_streamid[MAX_STREAMS]; ///< Convert Mplayer stream_id to rm stream id
    unsigned int rm2mp[MAX_STREAMS][MAX_MLTIIDX]; ///< Convert rm stream id and mlti index to Mplayer stream_id

   /**
    * Used to reorder audio data
    */
    unsigned int intl_id[MAX_STREAMS]; ///< interleaver id, per stream
    int sub_packet_size[MAX_STREAMS]; ///< sub packet size, per stream
    int sub_packet_h[MAX_STREAMS]; ///< number of coded frames per block
    int coded_framesize[MAX_STREAMS]; ///< coded frame size, per stream
    int audiopk_size[MAX_STREAMS]; ///< audio packet size
    unsigned char *audio_buf; ///< place to store reordered audio data
    double *audio_timestamp; ///< timestamp for each audio packet
    int sub_packet_cnt; ///< number of subpacket already received
    int audio_filepos; ///< file position of first audio packet in block

    unsigned int slice_size_addr;
    unsigned int slice_size;
    sh_real_opaque_data_t real_video_opaque;
    unsigned short prev_width;
    unsigned short prev_height;
    char video_file_start;
    unsigned char slice_size_bit_offset;
} real_priv_t;

//! use at most 200 MB of memory for index, corresponds to around 25 million entries
#define MAX_INDEX_ENTRIES (200*1024*1024 / sizeof(real_index_table_t))

static unsigned int enc_vlc_code(unsigned short info, unsigned char info_len)
{
    unsigned int v = 1;
    unsigned int shift = 1;
    unsigned char i;

    for(i=0; i<info_len; i++)
    {
        v += ((info&1)<<shift);
        shift += 2;
        info = info>>1;
    }

    return v;
}
/* originally from FFmpeg */
static void get_str(int isbyte, demuxer_t *demuxer, char *buf, int buf_size)
{
    int len;

    if (isbyte)
	len = stream_read_char(demuxer->stream);
    else
	len = stream_read_word(demuxer->stream);

    stream_read(demuxer->stream, buf, (len > buf_size) ? buf_size : len);
    if (len > buf_size)
	stream_skip(demuxer->stream, len-buf_size);

    mp_msg(MSGT_DEMUX, MSGL_V, "read_str: %d bytes read\n", len);
}

static void dump_index(demuxer_t *demuxer, int stream_id)
{
    real_priv_t *priv = demuxer->priv;
    real_index_table_t *index;
    int i, entries;

    if (!mp_msg_test(MSGT_DEMUX,MSGL_V))
	return;

    if ((unsigned)stream_id >= MAX_STREAMS)
	return;

    index = priv->index_table[stream_id];
    entries = priv->index_table_size[stream_id];

    mp_msg(MSGT_DEMUX, MSGL_V, "Index table for stream %d\n", stream_id);
    for (i = 0; i < entries; i++)
    {
#if 1
	mp_msg(MSGT_DEMUX, MSGL_V,"i: %d, pos: %d, timestamp: %u\n", i, index[i].offset, index[i].timestamp);
#else
	mp_msg(MSGT_DEMUX, MSGL_V,"packetno: %x pos: %x len: %x timestamp: %x flags: %x\n",
	    index[i].packetno, index[i].offset, index[i].len, index[i].timestamp,
	    index[i].flags);
#endif
    }
}

static int parse_index_chunk(demuxer_t *demuxer)
{
    real_priv_t *priv = demuxer->priv;
    int origpos = stream_tell(demuxer->stream);
    int next_header_pos = priv->index_chunk_offset;
    int i, entries, stream_id;

read_index:
    stream_seek(demuxer->stream, next_header_pos);

    i = stream_read_dword_le(demuxer->stream);
    if ((i == -256) || (i != MKTAG('I', 'N', 'D', 'X')))
    {
	mp_msg(MSGT_DEMUX, MSGL_WARN,"Something went wrong, no index chunk found on given address (%d)\n",
	    next_header_pos);
	index_mode = -1;
        if (i == -256)
	    stream_reset(demuxer->stream);
    	stream_seek(demuxer->stream, origpos);
	return 0;
	//goto end;
    }

    mp_msg(MSGT_DEMUX, MSGL_V,"Reading index table from index chunk (%d)\n",
	next_header_pos);

    i = stream_read_dword(demuxer->stream);
    mp_msg(MSGT_DEMUX, MSGL_V,"size: %d bytes\n", i);

    i = stream_read_word(demuxer->stream);
    if (i != 0)
	mp_msg(MSGT_DEMUX, MSGL_WARN,"Hmm, index table with unknown version (%d), please report it to MPlayer developers!\n", i);

    entries = stream_read_dword(demuxer->stream);
    mp_msg(MSGT_DEMUX, MSGL_V,"entries: %d\n", entries);

    stream_id = stream_read_word(demuxer->stream);
    mp_msg(MSGT_DEMUX, MSGL_V,"stream_id: %d\n", stream_id);

    next_header_pos = stream_read_dword(demuxer->stream);
    mp_msg(MSGT_DEMUX, MSGL_V,"next_header_pos: %d\n", next_header_pos);
    if (entries <= 0 || entries > MAX_INDEX_ENTRIES)
    {
	if (next_header_pos)
	    goto read_index;
	i = entries;
	goto end;
    }

    priv->index_table_size[stream_id] = entries;
    priv->index_table[stream_id] = (real_index_table_t *)calloc33(priv->index_table_size[stream_id], sizeof(real_index_table_t));

    for (i = 0; i < entries; i++)
    {
	stream_skip(demuxer->stream, 2); /* version */
	priv->index_table[stream_id][i].timestamp = stream_read_dword(demuxer->stream);
	priv->index_table[stream_id][i].offset = stream_read_dword(demuxer->stream);
	stream_skip(demuxer->stream, 4); /* packetno */
//	priv->index_table[stream_id][i].packetno = stream_read_dword(demuxer->stream);
//	printf("Index table: Stream#%d: entry: %d: pos: %d\n",
//	    stream_id, i, priv->index_table[stream_id][i].offset);
    }

    dump_index(demuxer, stream_id);

    if (next_header_pos > 0)
	goto read_index;

end:
    if (i == -256)
	stream_reset(demuxer->stream);
    stream_seek(demuxer->stream, origpos);
    if (i == -256)
	return 0;
    else
	return 1;
}

#if 1

static void add_index_item(demuxer_t *demuxer, int stream_id, unsigned int timestamp, int offset)
{
  if ((unsigned)stream_id < MAX_STREAMS)
  {
    real_priv_t *priv = demuxer->priv;
    real_index_table_t *index;
    if (priv->index_table_size[stream_id] >= MAX_INDEX_ENTRIES) {
      mp_msg(MSGT_DEMUXER, MSGL_WARN, "Index too large during building\n");
      return;
    }
    if (priv->index_table_size[stream_id] >= priv->index_malloc_size[stream_id])
    {
      if (priv->index_malloc_size[stream_id] == 0)
	priv->index_malloc_size[stream_id] = 2048;
      else
	priv->index_malloc_size[stream_id] += priv->index_malloc_size[stream_id] / 2;
      // in case we have a really large chunk...
      if (priv->index_table_size[stream_id] >=
            priv->index_malloc_size[stream_id])
        priv->index_malloc_size[stream_id] =
          priv->index_table_size[stream_id] + 1;
      priv->index_table[stream_id] = (real_index_table_t *)realloc33(priv->index_table[stream_id], priv->index_malloc_size[stream_id]*sizeof(priv->index_table[0][0]));
    }
    if (priv->index_table_size[stream_id] > 0)
    {
      index = &priv->index_table[stream_id][priv->index_table_size[stream_id] - 1];
      if (index->timestamp >= timestamp || index->offset >= offset)
	return;
    }
    index = &priv->index_table[stream_id][priv->index_table_size[stream_id]++];
    index->timestamp = timestamp;
    index->offset = offset;
  }
}

static void add_index_segment(demuxer_t *demuxer, int seek_stream_id, int64_t seek_timestamp)
{
  int tag, len, stream_id, flags;
  unsigned int timestamp;
  if (seek_timestamp != -1 && (unsigned)seek_stream_id >= MAX_STREAMS)
    return;
  while (1)
  {
    demuxer->filepos = stream_tell(demuxer->stream);

    tag = stream_read_dword(demuxer->stream);
    if (tag == MKTAG('A', 'T', 'A', 'D'))
    {
      stream_skip(demuxer->stream, 14);
      continue; /* skip to next loop */
    }
    len = tag & 0xffff;
    if (tag == -256 || len < 12)
      break;

    stream_id = stream_read_word(demuxer->stream);
    timestamp = stream_read_dword(demuxer->stream);

    stream_skip(demuxer->stream, 1); /* reserved */
    flags = stream_read_char(demuxer->stream);

    if (flags == -256)
      break;

    if (flags & 2)
    {
      add_index_item(demuxer, stream_id, timestamp, demuxer->filepos);
      if (stream_id == seek_stream_id && timestamp >= seek_timestamp)
      {
	stream_seek(demuxer->stream, demuxer->filepos);
	return;
      }
    }
    // printf("Index: stream=%d packet=%d timestamp=%u len=%d flags=0x%x datapos=0x%x\n", stream_id, entries, timestamp, len, flags, index->offset);
    /* skip data */
    stream_skip(demuxer->stream, len-12);
  }
}

static int generate_index(demuxer_t *demuxer)
{
  real_priv_t *priv = demuxer->priv;
  int origpos = stream_tell(demuxer->stream);
  int data_pos = priv->data_chunk_offset-10;
  int i;
  int tag;

  stream_seek(demuxer->stream, data_pos);
  tag = stream_read_dword(demuxer->stream);
  if (tag != MKTAG('A', 'T', 'A', 'D'))
  {
    mp_msg(MSGT_DEMUX, MSGL_WARN,"Something went wrong, no data chunk found on given address (%d)\n", data_pos);
  }
  else
  {
    stream_skip(demuxer->stream, 14);
    add_index_segment(demuxer, -1, -1);
  }
  for (i = 0; i < MAX_STREAMS; i++)
  {
    if (priv->index_table_size[i] > 0)
    {
      dump_index(demuxer, i);
    }
  }
  stream_reset(demuxer->stream);
  stream_seek(demuxer->stream, origpos);
  return 0;
}

#else

static int generate_index(demuxer_t *demuxer)
{
    real_priv_t *priv = demuxer->priv;
    int origpos = stream_tell(demuxer->stream);
    int data_pos = priv->data_chunk_offset-10;
    int num_of_packets = 0;
    int i, entries = 0;
    int len, stream_id = 0, flags;
    unsigned int timestamp;
    int tab_pos = 0;

read_index:
    stream_seek(demuxer->stream, data_pos);

    i = stream_read_dword_le(demuxer->stream);
    if ((i == -256) || (i != MKTAG('D', 'A', 'T', 'A')))
    {
	mp_msg(MSGT_DEMUX, MSGL_WARN,"Something went wrong, no data chunk found on given address (%d)\n",
	    data_pos);
	goto end;
    }
    stream_skip(demuxer->stream, 4); /* chunk size */
    stream_skip(demuxer->stream, 2); /* version */

    num_of_packets = stream_read_dword(demuxer->stream);
    mp_msg(MSGT_DEMUX, MSGL_V,"Generating index table from raw data (pos: 0x%x) for %d packets\n",
	data_pos, num_of_packets);

    data_pos = stream_read_dword_le(demuxer->stream)-10; /* next data chunk */

    for (i = 0; i < MAX_STREAMS; i++)
    {
    priv->index_table_size[i] = num_of_packets;
    priv->index_table[i] = calloc33(priv->index_table_size[i], sizeof(real_index_table_t));
//    priv->index_table[stream_id] = realloc33(priv->index_table[stream_id],
//	priv->index_table_size[stream_id] * sizeof(real_index_table_t));
    }

    tab_pos = 0;

//    memset(priv->index_table_size, 0, sizeof(int)*MAX_STREAMS);
//    memset(priv->index_table, 0, sizeof(real_index_table_t)*MAX_STREAMS);

    while (tab_pos < num_of_packets)
    {
    i = stream_read_char(demuxer->stream);
    if (i == -256)
	goto end;
    stream_skip(demuxer->stream, 1);
//    stream_skip(demuxer->stream, 2); /* version */

    len = stream_read_word(demuxer->stream);
    stream_id = stream_read_word(demuxer->stream);
    timestamp = stream_read_dword(demuxer->stream);

    stream_skip(demuxer->stream, 1); /* reserved */
    flags = stream_read_char(demuxer->stream);

    i = tab_pos;

//    priv->index_table_size[stream_id] = i;
//    if (priv->index_table[stream_id] == NULL)
//	priv->index_table[stream_id] = malloc33(priv->index_table_size[stream_id] * sizeof(real_index_table_t));
//    else
//	priv->index_table[stream_id] = realloc33(priv->index_table[stream_id],
//	    priv->index_table_size[stream_id] * sizeof(real_index_table_t));

    priv->index_table[stream_id][i].timestamp = timestamp;
    priv->index_table[stream_id][i].offset = stream_tell(demuxer->stream)-12;
    priv->index_table[stream_id][i].len = len;
    priv->index_table[stream_id][i].packetno = entries;
    priv->index_table[stream_id][i].flags = flags;

    tab_pos++;

    /* skip data */
    stream_skip(demuxer->stream, len-12);
    }
    dump_index(demuxer, stream_id);
    if (data_pos)
	goto read_index;

end:
    if (i == -256)
	stream_reset(demuxer->stream);
    stream_seek(demuxer->stream, origpos);
    if (i == -256)
	return 0;
    else
	return 1;
}
#endif


static int real_check_file(demuxer_t* demuxer)
{
    real_priv_t *priv;
    int c;

    mp_msg(MSGT_DEMUX,MSGL_V,"Checking for REAL\n");

    c = stream_read_dword_le(demuxer->stream);
    if (c == -256)
	return 0; /* EOF */
    if (c != MKTAG('.', 'R', 'M', 'F'))
	return 0; /* bad magic */

    priv = (real_priv_t *)calloc33(1, sizeof(real_priv_t));
    demuxer->priv = priv;

    return DEMUXER_TYPE_REAL;
}


static double real_fix_timestamp_static(packet_hdr_t *pkt_hdr, unsigned int timestamp, unsigned int format, int64_t *kf_base, int *kf_pts, double *pts){
  double v_pts;
  unsigned int kf=timestamp;
  int pict_type;
//  unsigned int orig_kf;

  if(format==mmioFOURCC('R','V','3','0') || format==mmioFOURCC('R','V','4','0')){
    pict_type = pkt_hdr->PicCodType;
//    orig_kf=kf= pkt_hdr->TR;  //    kf= 2*SHOW_BITS(12);
//    if(pict_type==0)
    if(pict_type<=1){
      // I frame, sync timestamps:
      *kf_base=(int64_t)timestamp-kf;
      mp_msg(MSGT_DEMUX, MSGL_DBG2,"\nTS: base=%08"PRIX64"\n",*kf_base);
      kf=timestamp;
    } else {
      // P/B frame, merge timestamps:
      int64_t tmp=(int64_t)timestamp-*kf_base;
      kf|=tmp&(~0x1fff);	// combine with packet timestamp
      if(kf<tmp-4096) kf+=8192; else // workaround wrap-around problems
      if(kf>tmp+4096) kf-=8192;
      kf+=*kf_base;
    }
    if(pict_type != 3){ // P || I  frame -> swap timestamps
	unsigned int tmp=kf;
	kf=*kf_pts;
	*kf_pts=tmp;
//	if(kf<=tmp) kf=0;
    }
  }
    v_pts=kf*0.001f;
//    if(pts && (v_pts<*pts || !kf)) v_pts=*pts+frametime;
    if(pts) *pts=v_pts;
    return v_pts;
}
#define SKIP_BITS(n) buffer<<=n
#define SHOW_BITS(n) ((buffer)>>(32-(n)))
double real_fix_timestamp(unsigned char *buf, unsigned int timestamp, unsigned int format, int64_t *kf_base, int *kf_pts, double *pts){
  double v_pts;
  unsigned char *s = buf + 1 + (*buf+1)*8;
  uint32_t buffer= (s[0]<<24) + (s[1]<<16) + (s[2]<<8) + s[3];
  unsigned int kf=timestamp;
  int pict_type;
  unsigned int orig_kf;

  if(format==mmioFOURCC('R','V','3','0') || format==mmioFOURCC('R','V','4','0')){
    if(format==mmioFOURCC('R','V','3','0')){
      SKIP_BITS(3);
      pict_type= SHOW_BITS(2);
      SKIP_BITS(2 + 7);
    }else{
      SKIP_BITS(1);
      pict_type= SHOW_BITS(2);
      SKIP_BITS(2 + 7 + 3);
    }
    orig_kf=
    kf= SHOW_BITS(13);  //    kf= 2*SHOW_BITS(12);
//    if(pict_type==0)
    if(pict_type<=1){
      // I frame, sync timestamps:
      *kf_base=(int64_t)timestamp-kf;
      mp_msg(MSGT_DEMUX, MSGL_DBG2,"\nTS: base=%08"PRIX64"\n",*kf_base);
      kf=timestamp;
    } else {
      // P/B frame, merge timestamps:
      int64_t tmp=(int64_t)timestamp-*kf_base;
      kf|=tmp&(~0x1fff);	// combine with packet timestamp
      if(kf<tmp-4096) kf+=8192; else // workaround wrap-around problems
      if(kf>tmp+4096) kf-=8192;
      kf+=*kf_base;
    }
    if(pict_type != 3){ // P || I  frame -> swap timestamps
	unsigned int tmp=kf;
	kf=*kf_pts;
	*kf_pts=tmp;
//	if(kf<=tmp) kf=0;
    }
    mp_msg(MSGT_DEMUX, MSGL_DBG2,"\nTS: %08X -> %08X (%04X) %d %02X %02X %02X %02X %5u\n",timestamp,kf,orig_kf,pict_type,s[0],s[1],s[2],s[3],pts?kf-(unsigned int)(*pts*1000.0):0);
  }
    v_pts=kf*0.001f;
//    if(pts && (v_pts<*pts || !kf)) v_pts=*pts+frametime;
    if(pts) *pts=v_pts;
    return v_pts;
}

#define RV_INTRAPIC  0
#define RV_FORCED_INTRAPIC 1
#define RV_INTERPIC 2
#define RV_TRUEBPIC 3

typedef struct dp_hdr_s {
    uint32_t chunks;	// number of chunks
    uint32_t timestamp; // timestamp from packet header
    uint32_t len;	// length of actual data
    uint32_t header_buffer_len;	// offset to data offset array
} dp_hdr_t;
typedef struct getbits_context
{
    unsigned char value;
    unsigned char *buffer;
    unsigned int bufferlen;
    unsigned int bitcnt;
    unsigned int index;
}mtg_getbits_context_t;
static unsigned int mtg_get_bits(mtg_getbits_context_t *s,int n){
    unsigned int x=0;
    while(n-->0){
	if(!s->bitcnt){
	    if(s->index >= s->bufferlen)
            return -1;
	    s->value=s->buffer[s->index++];
	    s->bitcnt=8;
	}
	//x=(x<<1)|(buf&1);buf>>=1;
	x=(x<<1)|(s->value>>7);s->value<<=1;
	--s->bitcnt;
    }
    return x;
}
static int mtg_write_bits(mtg_getbits_context_t *s,int v,int n){
    while(n-->0)
    {
        s->value<<=1;
        s->value += ((v>>n)&1);
        ++s->bitcnt;
        if(s->bitcnt == 8)
        {
            if(s->index >= s->bufferlen)
                return -1;
            s->buffer[s->index++] = s->value;
            s->bitcnt=0;
            s->value=0;
        }
    }
    return 0;
}
unsigned int mtg_init_op_bits8(mtg_getbits_context_t *s, const uint8_t *buffer,
                                 int byte_size)
{
    if(!buffer || byte_size <= 0)
    {
        return -1;
    }

    s->buffer = buffer;
    s->bufferlen = byte_size;
    s->bitcnt = 0;
    s->index = 0;
    s->value = 0;
	return 0;
}

static     const unsigned short rv34_mb_max_sizes[6] = { 0x2F, 0x62, 0x18B, 0x62F, 0x18BF, 0x23FF };
static    const unsigned char rv34_mb_bits_sizes[6] = { 6, 7, 9, 11, 13, 14 };
static const int rv40_standard_widths[]   = { 160, 172, 240, 320, 352, 640, 704, 0};
static const int rv40_standard_heights[]  = { 120, 132, 144, 240, 288, 480, -8, -10, 180, 360, 576, 0};
static int ff_rv34_get_start_offset(int mb_size)
{
    int i;
    for(i = 0; i < 5; i++)
        if(rv34_mb_max_sizes[i] >= mb_size - 1)
            break;
    return rv34_mb_bits_sizes[i];
}

static int get_dimension(mtg_getbits_context_t *gb, const int *dim)
{
    int t   = mtg_get_bits(gb, 3);
    int val = dim[t];

    if(val < 0)
    {
        val = dim[mtg_get_bits(gb,1) - val];
    }
    if(!val){
        do{
            t = mtg_get_bits(gb, 8);
            val += t << 2;
        }while(t == 0xFF);
    }

    return val;
}
static void rv40_parse_picture_size(mtg_getbits_context_t *gb, int *w, int *h)
{
    *w = get_dimension(gb, rv40_standard_widths);
    *h = get_dimension(gb, rv40_standard_heights);
}
static void update_slice_size(real_priv_t *priv, unsigned int slice_bit_len)
{
    mtg_getbits_context_t bitctx;
    unsigned int slice_size = priv->slice_size;
    unsigned int addr = priv->slice_size_addr;
     unsigned char bitoffset = priv->slice_size_bit_offset;
    unsigned char *buffer = (unsigned char *)addr;
    unsigned char tmp = (buffer[0]>>(8-bitoffset));

    mtg_init_op_bits8(&bitctx,buffer,6);
    mtg_write_bits(&bitctx,tmp,bitoffset);

    slice_size += slice_bit_len;
    mtg_write_bits(&bitctx,slice_size>>24,8);
    mtg_write_bits(&bitctx,3,2);
    mtg_write_bits(&bitctx,slice_size>>16,8);
    mtg_write_bits(&bitctx,3,2);
    mtg_write_bits(&bitctx,slice_size>>8,8);
    mtg_write_bits(&bitctx,3,2);
    mtg_write_bits(&bitctx,slice_size,8);
    mtg_write_bits(&bitctx,3,2);

    if(bitctx.bitcnt)
    {
        tmp = buffer[bitctx.index];
        tmp =(((unsigned char)(tmp<<(8-bitctx.bitcnt)))>>(8-bitctx.bitcnt));
        tmp |= (bitctx.value<<(8-bitctx.bitcnt));
        buffer[bitctx.index] = tmp;
    }
    priv->slice_size = slice_size;
}
static void write_slice_size(real_priv_t *priv, packet_hdr_t *pkt_hdr, mtg_getbits_context_t *bitctx, unsigned char *buffer, unsigned int slice_bit_len)
{
        priv->slice_size = slice_bit_len;
        if(pkt_hdr->broken_up_by_us)
        {
            priv->slice_size_addr = (unsigned int)(buffer+bitctx->index);
            priv->slice_size_bit_offset = bitctx->bitcnt;
        }

        mtg_write_bits(bitctx,slice_bit_len>>24,8);
        mtg_write_bits(bitctx,3,2);
        mtg_write_bits(bitctx,slice_bit_len>>16,8);
        mtg_write_bits(bitctx,3,2);
        mtg_write_bits(bitctx,slice_bit_len>>8,8);
        mtg_write_bits(bitctx,3,2);
        mtg_write_bits(bitctx,slice_bit_len,8);
        mtg_write_bits(bitctx,3,2);

}
static void write_ecc_tr_db_struffing(packet_hdr_t *pkt_hdr, mtg_getbits_context_t *bitctx)
{
        char stuffing_bits;

        mtg_write_bits(bitctx,pkt_hdr->ECC,1);
        mtg_write_bits(bitctx,pkt_hdr->TR>>8,5);
        mtg_write_bits(bitctx,pkt_hdr->Deblock_Pass_Thru,1);
        mtg_write_bits(bitctx,1,1);

        stuffing_bits = 8-bitctx->bitcnt - pkt_hdr->start_mb_data_bitnum;
        stuffing_bits = (stuffing_bits <= 0)?(stuffing_bits+8):stuffing_bits;

        mtg_write_bits(bitctx,stuffing_bits,8);
        mtg_write_bits(bitctx,0,stuffing_bits);
        if(pkt_hdr->start_mb_data_bitnum)
            mtg_write_bits(bitctx,pkt_hdr->start_mb_data,pkt_hdr->start_mb_data_bitnum);

}
static int realvideo_write_file_header(sh_video_t *sh_video, real_priv_t *priv, unsigned char *buffer)
{
    int index = 0;
    unsigned int i;
    uint32_t SubMOFTag;

    if(sh_video->format == mmioFOURCC('R', 'V', '3', '0')) ///rv8
    {
        mp_msg(MSGT_DEMUX,MSGL_INFO,"realvideo_write_file_header rv8\n");
        AV_WB32(buffer+index,0x00000100);
        index += 4;

        SubMOFTag = mmioFOURCC('R', 'V', '3', '0');
    }
    else if(sh_video->format == mmioFOURCC('R', 'V', '4', '0'))///rv9-rv10
    {
        mp_msg(MSGT_DEMUX,MSGL_INFO,"realvideo_write_file_header rv9\n");
        AV_WB32(buffer+index,0x55555555);
        index += 4;

        SubMOFTag = mmioFOURCC('R', 'V', '4', '0');
    }
    else
    {
        mp_msg(MSGT_DEMUX,MSGL_ERR,"[%s] format[0x%x] is not rv30 or rv40, we can't support!\n",__func__,sh_video->format);
        return -1;
    }

    AV_WB32(buffer+index,0x5649444F);///tag = VIDO
    index += 4;

    AV_WB32(buffer+index,priv->real_video_opaque.opauqe_data_size+26);///length
    index += 4;

    AV_WB32(buffer+index,0x5649444F);///MOFTag = VIDO
    index += 4;

    AV_WL32(buffer+index,SubMOFTag);///SubMOFTag
    index += 4;

    AV_WB16(buffer+index,sh_video->disp_w);///width
    index += 2;

    AV_WB16(buffer+index,sh_video->disp_h);///height
    index += 2;

    AV_WB16(buffer+index,priv->real_video_opaque.bit_count);///bitcount
    index += 2;

    AV_WB16(buffer+index,priv->real_video_opaque.pad_width);///pad_width
    index += 2;

    AV_WB16(buffer+index,priv->real_video_opaque.pad_height);///pad_height
    index += 2;

    AV_WB32(buffer+index,(uint32_t)sh_video->fps);///fps
    index += 4;

    AV_WB32(buffer+index,priv->real_video_opaque.opauqe_data_size);///opaque data size
    index += 4;

    for(i=0;i<priv->real_video_opaque.opauqe_data_size;i++)
    {
        buffer[index+i] = priv->real_video_opaque.p_opaque_data[i];
    }
    index += priv->real_video_opaque.opauqe_data_size;

    return index;
}

static int realvideo_write_pic_header(real_priv_t *priv, sh_video_t *sh_video, packet_hdr_t *pkt_hdr, unsigned char *buffer, unsigned int len, unsigned int slice_raw_bs_len)
{
    int index = 0;
    char is_rv8 = (sh_video->format == mmioFOURCC('R', 'V', '3', '0'));
    char is_rv9 = (sh_video->format == mmioFOURCC('R', 'V', '4', '0'));
    if(is_rv8 || is_rv9)
    {
        unsigned int slice_payload_len;

        unsigned int picture_layer_info;///31bits

        unsigned short tmp;
        unsigned short CSFMT_FWI;///9bits
        unsigned short CSFMT_FHI;///9bits

        unsigned char sequence_end_code;///1bit
        unsigned char is_QCIF;///1bit
        unsigned char PQUANT;///5bits
        unsigned char TR_low;///8bits

        unsigned char pixel_aspect_ratio;

        mtg_getbits_context_t bitctx;


        mtg_init_op_bits8(&bitctx,buffer+index,len-index);

        if(is_rv9)
            mtg_write_bits(&bitctx,0x55555555,32);
        else
            mtg_write_bits(&bitctx,1,24);

        /**********************picture_layer_info*****************/
        sequence_end_code = 0;
        is_QCIF = (pkt_hdr->width == 176 && pkt_hdr->height == 144);
        PQUANT = pkt_hdr->SQUANT;
        TR_low = pkt_hdr->TR&0xff;
        tmp = TR_low;
        tmp =  (tmp<<5)+PQUANT;
        tmp =  (tmp<<1)+(1-is_QCIF);
        tmp = (tmp<<1)+sequence_end_code;
        picture_layer_info = enc_vlc_code(tmp,15);
        mtg_write_bits(&bitctx,picture_layer_info,31);

        if(pkt_hdr->PicCodType == RV_INTRAPIC || pkt_hdr->PicCodType == RV_FORCED_INTRAPIC)
        {
            mtg_write_bits(&bitctx,3,3);///3bits
        }
        else if(pkt_hdr->PicCodType == RV_INTERPIC)
        {
            mtg_write_bits(&bitctx,1,1);///1bit
        }
        else///RV_TRUEBPIC
        {
            mtg_write_bits(&bitctx,1,5);///5bits
        }

        if(!is_QCIF)
        {
            pixel_aspect_ratio = 1;
            mtg_write_bits(&bitctx,pixel_aspect_ratio,4);///4bits

            CSFMT_FWI = (pkt_hdr->width>>2)-1;
            mtg_write_bits(&bitctx,CSFMT_FWI,9);

            mtg_write_bits(&bitctx,1,1);

            CSFMT_FHI = (pkt_hdr->height>>2);
            mtg_write_bits(&bitctx,CSFMT_FHI,9);
        }

        if(is_rv9)
            mtg_write_bits(&bitctx,pkt_hdr->OSV_QUANT,2);

        slice_payload_len = (slice_raw_bs_len - pkt_hdr->slice_header_len)*8 + pkt_hdr->start_mb_data_bitnum;
        write_slice_size(priv,pkt_hdr,&bitctx,buffer,slice_payload_len);

        write_ecc_tr_db_struffing(pkt_hdr,&bitctx);

        index += bitctx.index;
    }

    return index;
}

static int realvideo_write_slice_header(real_priv_t *priv, sh_video_t *sh_video, packet_hdr_t *pkt_hdr, unsigned char *buffer, unsigned int len, unsigned int slice_raw_bs_len)
{
    int index = 0;
    char is_rv8 = (sh_video->format == mmioFOURCC('R', 'V', '3', '0'));
    char is_rv9 = (sh_video->format == mmioFOURCC('R', 'V', '4', '0'));
    if(is_rv8 || is_rv9)
    {
        unsigned int slice_payload_len;
        unsigned char Slice_GOB_frame_ID;
        mtg_getbits_context_t bitctx;

        mtg_init_op_bits8(&bitctx,buffer+index,len-index);

        if(is_rv9)
            mtg_write_bits(&bitctx,0x1d1c10,24);
        else
            mtg_write_bits(&bitctx,1,17);

        mtg_write_bits(&bitctx,1,1);

        mtg_write_bits(&bitctx,pkt_hdr->MBA,pkt_hdr->MBA_bits);
        if(pkt_hdr->MBA_bits > 11)
            mtg_write_bits(&bitctx,1,1);

        mtg_write_bits(&bitctx,pkt_hdr->SQUANT,5);

        if(is_rv9)
            mtg_write_bits(&bitctx,pkt_hdr->OSV_QUANT,2);

        mtg_write_bits(&bitctx,1,1);

        Slice_GOB_frame_ID = 0;
        mtg_write_bits(&bitctx,Slice_GOB_frame_ID,2);

        slice_payload_len = (slice_raw_bs_len - pkt_hdr->slice_header_len)*8 + pkt_hdr->start_mb_data_bitnum;
        write_slice_size(priv,pkt_hdr,&bitctx,buffer,slice_payload_len);

        write_ecc_tr_db_struffing(pkt_hdr,&bitctx);

        index += bitctx.index;
    }
    return index;
}
static int rv8_parse_slice_header(demuxer_t *demuxer, packet_hdr_t *hdr, unsigned char *buf, unsigned int buflen)
{
    real_priv_t *priv = demuxer->priv;
    unsigned int max_rpr;
    unsigned int rpr;
    unsigned int mb_size;
    unsigned int mb_bits= 0;
    int w = priv->prev_width;
    int h = priv->prev_height;
    unsigned int extradata_size = priv->real_video_opaque.opauqe_data_size;
    unsigned char *extradata = priv->real_video_opaque.p_opaque_data;
    mtg_getbits_context_t bit_context;

    mtg_init_op_bits8(&bit_context,buf,buflen);

    ////see rv_codec_opaque_data in RealVideo_Opaque_Data_Specification.doc
    if(!extradata && extradata_size < 8)
    {
        mp_msg(MSGT_DEMUX,MSGL_ERR,"[%s] rv8 need opaque data as spoflag,but no data found!\n",__func__);
        return -1;
    }

    mtg_get_bits(&bit_context,3);

    hdr->PicCodType = mtg_get_bits(&bit_context,2);

    hdr->ECC = mtg_get_bits(&bit_context,1);

    hdr->SQUANT = mtg_get_bits(&bit_context,5);

    hdr->Deblock_Pass_Thru = mtg_get_bits(&bit_context,1);

    hdr->TR = mtg_get_bits(&bit_context,13);

    max_rpr = extradata[1]&7;
    rpr = mtg_get_bits(&bit_context, av_log2(max_rpr) + 1);
    if (extradata_size < rpr * 2 + 8)
    {
        mp_msg(MSGT_DEMUX,MSGL_ERR,"demux_real: rv8_get_slice_header :Insufficient extradata - need at least %d bytes, got %d\n",
                   8 + rpr * 2, extradata_size);
        return -1;
    }

    if(rpr)
    {
        w = extradata[6 + rpr*2] << 2;
        h = extradata[7 + rpr*2] << 2;
    }
    hdr->width = w;
    hdr->height = h;

    mb_size = ((w + 15) >> 4) * ((h + 15) >> 4);
    mb_bits = ff_rv34_get_start_offset(mb_size);
    hdr->MBA = mtg_get_bits(&bit_context,mb_bits);
    hdr->MBA_bits = mb_bits;

    ///RTYPE
    mtg_get_bits(&bit_context,1);

    hdr->slice_header_len = bit_context.index;

    if(bit_context.bitcnt)
    {
        hdr->start_mb_data_bitnum = bit_context.bitcnt;
        hdr->start_mb_data = mtg_get_bits(&bit_context,hdr->start_mb_data_bitnum);
    }
    else
        hdr->start_mb_data_bitnum = 0;

    return 0;
}
static int rv9_parse_slice_header(demuxer_t *demuxer,packet_hdr_t *hdr, unsigned char *buf, unsigned int buflen)
{
    real_priv_t *priv = demuxer->priv;
    mtg_getbits_context_t bit_context;
    int w = priv->prev_width;
    int h = priv->prev_height;
    unsigned int mb_size;
    unsigned int mb_bits= 0;

    mtg_init_op_bits8(&bit_context,buf,buflen);

    hdr->pixel_aspect_ratio = 1;//dont know the value ,so set 1

    hdr->ECC = mtg_get_bits(&bit_context,1);

    hdr->PicCodType = mtg_get_bits(&bit_context,2);

    hdr->SQUANT = mtg_get_bits(&bit_context,5);

    ///1bit bitstream version
    hdr->BitStreamVersion = mtg_get_bits(&bit_context,1);

    ///reserved
    mtg_get_bits(&bit_context,1);

    hdr->OSV_QUANT = mtg_get_bits(&bit_context,2);

    hdr->Deblock_Pass_Thru = mtg_get_bits(&bit_context,1);

    hdr->TR = mtg_get_bits(&bit_context,13);


    if(hdr->PicCodType == RV_INTERPIC || hdr->PicCodType == RV_TRUEBPIC)
    {
        if(mtg_get_bits(&bit_context,1) == 0)
        {
            rv40_parse_picture_size(&bit_context,&w,&h);
        }
    }
    else
        rv40_parse_picture_size(&bit_context,&w,&h);

    hdr->width = w;
    hdr->height = h;
    priv->prev_width = w;
    priv->prev_height = h;

    mb_size = ((w + 15) >> 4) * ((h + 15) >> 4);
    mb_bits = ff_rv34_get_start_offset(mb_size);
    hdr->MBA = mtg_get_bits(&bit_context,mb_bits);
    hdr->MBA_bits = mb_bits;

    hdr->slice_header_len = bit_context.index;

    if(bit_context.bitcnt)
    {
        hdr->start_mb_data_bitnum = bit_context.bitcnt;
        hdr->start_mb_data = mtg_get_bits(&bit_context,hdr->start_mb_data_bitnum);
    }
    else
        hdr->start_mb_data_bitnum = 0;


    return 0;
}
static int parse_slice_header(sh_video_t *sh_video, demuxer_t *demuxer, packet_hdr_t *pkt_hdr, unsigned char *buffer, int len)
{
    if(sh_video->format == mmioFOURCC('R', 'V', '3', '0'))
    {
        return rv8_parse_slice_header(demuxer,pkt_hdr,buffer,len);
    }
    else if(sh_video->format == mmioFOURCC('R', 'V', '4', '0'))
    {
        return rv9_parse_slice_header(demuxer,pkt_hdr,buffer,len);
    }
    else
    {
        mp_msg(MSGT_DEMUX,MSGL_ERR,"[%s] format[0x%x] is not rv30 or rv40, we can't support!\n",__func__,sh_video->format);
        return -1;
    }

}
static void queue_video_packet(real_priv_t *priv, demux_stream_t *ds, demux_packet_t *dp)
{
    dp_hdr_t hdr = *(dp_hdr_t*)dp->buffer;
    packet_hdr_t *pkt_hdr = (packet_hdr_t*)(dp->buffer+sizeof(dp_hdr_t));
    sh_video_t *sh_video = ds->sh;

    uint32_t i;
    int index = 0;
    unsigned int slice_raw_bs_len;
    unsigned char *tmp = (unsigned char *)malloc33(sizeof(packet_hdr_t)*(1+hdr.chunks));
    unsigned char last_broken_flag = 0;
    memcpy(tmp, dp->buffer+sizeof(dp_hdr_t), sizeof(packet_hdr_t)*(1+hdr.chunks));
    if(priv->video_file_start == 1)///file header
    {
        priv->video_file_start = 0;

        index += realvideo_write_file_header(sh_video,priv,dp->buffer);
    }

    pkt_hdr = (packet_hdr_t*)tmp;
    for(i=0;i<=hdr.chunks;i++,pkt_hdr++)
    {
        if(i < hdr.chunks)
        {
            slice_raw_bs_len = (pkt_hdr+1)->offset - pkt_hdr->offset;
        }
        else
        {
            slice_raw_bs_len = hdr.len - pkt_hdr->offset;
        }
        if(i == 0)//first slice
        {
            last_broken_flag = pkt_hdr->broken_up_by_us;
            index += realvideo_write_pic_header(priv,sh_video,pkt_hdr,dp->buffer+index, dp->len-index, slice_raw_bs_len);
            memmove(dp->buffer+index,dp->buffer+hdr.header_buffer_len+pkt_hdr->offset+pkt_hdr->slice_header_len,slice_raw_bs_len-pkt_hdr->slice_header_len);
            index += (slice_raw_bs_len-pkt_hdr->slice_header_len);
        }
        else
        {
            if(last_broken_flag && pkt_hdr->broken_up_by_us)
            {
                update_slice_size(priv,slice_raw_bs_len<<3);
                memmove(dp->buffer+index,dp->buffer+hdr.header_buffer_len+pkt_hdr->offset,slice_raw_bs_len);
                index += slice_raw_bs_len;
            }
            else
            {
                index += realvideo_write_slice_header(priv,sh_video,pkt_hdr,dp->buffer+index, dp->len-index, slice_raw_bs_len);
                memmove(dp->buffer+index,dp->buffer+hdr.header_buffer_len+pkt_hdr->offset+pkt_hdr->slice_header_len,slice_raw_bs_len-pkt_hdr->slice_header_len);
                index += (slice_raw_bs_len-pkt_hdr->slice_header_len);
            }
        }
    }
    dp->len = index;


    if(priv->video_after_seek){
        priv->kf_base = 0;
        priv->kf_pts = hdr.timestamp;
        priv->video_after_seek = 0;
    }

    if(hdr.len >= 3)  /* this check may be useless */
    {
        pkt_hdr = (packet_hdr_t*)tmp;
        dp->pts = real_fix_timestamp_static(pkt_hdr,hdr.timestamp,
                                     ((sh_video_t *)ds->sh)->format,
                                     &priv->kf_base, &priv->kf_pts,
                                     &priv->v_pts);
    }

    free33(tmp);
    ds_add_packet(ds, dp);
}

// return value:
//     0 = EOF or no stream found
//     1 = successfully read a packet
static int demux_real_fill_buffer(demuxer_t *demuxer, demux_stream_t *dsds)
{
    real_priv_t *priv = demuxer->priv;
    demux_stream_t *ds = NULL;
    int len;
    unsigned int timestamp;
    int rm_stream_id, mp_stream_id;
#ifdef CRACK_MATRIX
    int i;
#endif
    int flags;
    int version;
    int pk_group;
    demux_packet_t *dp;
    int x, sps, cfs, sph, spc, w;
    int audioreorder_getnextpk = 0;

    packet_hdr_t *pkt_hdr = NULL;
    char broken_up_by_us = 0;

  // Don't demux video if video codec init failed
  if (demuxer->video->id >= 0 && !demuxer->video->sh)
    demuxer->video->id = -2;

  while(!stream_eof(demuxer->stream)){

    /* Handle audio/video demxing switch for multirate files (non-interleaved) */
    if (priv->is_multirate && priv->stream_switch) {
        demuxer->audio->eof = priv->current_apacket >= priv->index_table_size[demuxer->audio->id];
        demuxer->video->eof = priv->current_vpacket >= priv->index_table_size[demuxer->video->id];
        if (demuxer->audio->eof && demuxer->video->eof)
            return 0;
        else if (!demuxer->audio->eof && demuxer->video->eof)
            stream_seek(demuxer->stream, priv->audio_curpos); // Get audio
        else if (demuxer->audio->eof && !demuxer->video->eof)
            stream_seek(demuxer->stream, priv->video_curpos); // Get video
        else if (priv->index_table[demuxer->audio->id][priv->current_apacket].timestamp <
            priv->index_table[demuxer->video->id][priv->current_vpacket].timestamp)
            stream_seek(demuxer->stream, priv->audio_curpos); // Get audio
        else
            stream_seek(demuxer->stream, priv->video_curpos); // Get video
        priv->stream_switch = 0;
    }

    demuxer->filepos = stream_tell(demuxer->stream);
    version = stream_read_word(demuxer->stream); /* version */
    len = stream_read_word(demuxer->stream);
    if ((version==0x4441) && (len==0x5441)) { // new data chunk
	mp_msg(MSGT_DEMUX,MSGL_INFO,"demux_real: New data chunk is coming!!!\n");
    if (priv->is_multirate)
        return 0; // EOF
	stream_skip(demuxer->stream,14);
	demuxer->filepos = stream_tell(demuxer->stream);
        version = stream_read_word(demuxer->stream); /* version */
	len = stream_read_word(demuxer->stream);
    } else if ((version == 0x494e) && (len == 0x4458)) {
        mp_msg(MSGT_DEMUX,MSGL_V,"demux_real: Found INDX chunk. EOF.\n");
        demuxer->stream->eof=1;
        return 0;
    }


    if (len == -256){ /* EOF */
//	printf("len==-256!\n");
	return 0;
    }
    if (len < 12){
    	unsigned int idx_streamid;
	mp_msg(MSGT_DEMUX, MSGL_V,"%08X: packet v%d len=%d  \n",(int)demuxer->filepos,(int)version,(int)len);
	mp_msg(MSGT_DEMUX, MSGL_WARN,"bad packet len (%d)\n", len);
	if ((unsigned)demuxer->video->id < MAX_STREAMS) {
	    idx_streamid = priv->is_mlti ? priv->mp2rm_streamid[demuxer->video->id] : demuxer->video->id;
	    if (priv->current_vpacket + 1 < priv->index_table_size[idx_streamid]) {
		stream_seek(demuxer->stream, priv->index_table[idx_streamid][++priv->current_vpacket].offset);
	    }
	} else if ((unsigned)demuxer->audio->id < MAX_STREAMS) {
	    idx_streamid = priv->is_mlti ? priv->mp2rm_streamid[demuxer->audio->id] : demuxer->audio->id;
	    if (priv->current_apacket + 1 < priv->index_table_size[idx_streamid]) {
		stream_seek(demuxer->stream, priv->index_table[idx_streamid][++priv->current_apacket].offset);
	    }
	}
	continue; //goto loop;
    }

    rm_stream_id = stream_read_word(demuxer->stream);
    timestamp = stream_read_dword(demuxer->stream);
    pk_group = stream_read_char(demuxer->stream);
    flags = stream_read_char(demuxer->stream);
    /* flags:		*/
    /*  0x1 - reliable  */
    /* 	0x2 - keyframe	*/

    if (version == 1) {
        int tmp;
        tmp = stream_read_char(demuxer->stream);
        mp_msg(MSGT_DEMUX, MSGL_DBG2,"Version: %d, skipped byte is %d\n", version, tmp);
        len--;
        if (priv->is_mlti)
            mp_msg(MSGT_DEMUX, MSGL_WARN,"MLTI file with v1 DATA, expect problems! Please contact Mplayer developers.\n");
    }

    if (flags & 2)
      add_index_item(demuxer, rm_stream_id, timestamp, demuxer->filepos);

//    printf("%08X: packet v%d len=%4d  id=%d  pts=%6d  rvd=%d  flags=%d  \n",
//	(int)demuxer->filepos,(int)version,(int)len, stream_id,
//	(int) timestamp, reserved, flags);

    mp_dbg(MSGT_DEMUX,MSGL_DBG2,  "\npacket#%d: pos: 0x%0x, len: %d, rm_id: %d, pts: %u, flags: %x grp:%d\n",
	priv->current_packet, (int)demuxer->filepos, len, rm_stream_id, timestamp, flags, pk_group);

    priv->current_packet++;
    len -= 12;

//    printf("s_id=%d  aid=%d  vid=%d  \n",stream_id,demuxer->audio->id,demuxer->video->id);

    // Map rm stream id and packet group to MPlayer stream aid or vid if file is MLTI
    if (priv->is_mlti && rm_stream_id < MAX_STREAMS && (pk_group>>1) < MAX_MLTIIDX)
        mp_stream_id = priv->rm2mp[rm_stream_id][(pk_group>>1)-1];
    else
        mp_stream_id = rm_stream_id;

    /* check stream_id: */

    if(demuxer->audio->id==mp_stream_id){
    	if (priv->audio_need_keyframe == 1&& flags != 0x2)
		goto discard;
got_audio:
	ds=demuxer->audio;
	mp_dbg(MSGT_DEMUX,MSGL_DBG2, "packet is audio (mp_id: %d)\n", mp_stream_id);

        if (flags & 2) {
    	    priv->sub_packet_cnt = 0;
    	    audioreorder_getnextpk = 0;
        }

	// parse audio chunk:
	{
#ifdef CRACK_MATRIX
	    int spos=stream_tell(demuxer->stream);
	    static int cnt=0;
	    static int cnt2=CRACK_MATRIX;
#endif
	    if (((sh_audio_t *)ds->sh)->format == mmioFOURCC('M', 'P', '4', 'A')) {
		uint16_t sub_packet_lengths[16], sub_packets, i;
		int totlen = 0;
		/* AAC in Real: several AAC frames in one Real packet. */
		/* Second byte, upper four bits: number of AAC frames */
		/* next n * 2 bytes: length of the AAC frames in bytes, BE */
		if (len < 2)
		    goto discard;
		sub_packets = (stream_read_word(demuxer->stream) & 0xf0) >> 4;
		if (len < 2 * sub_packets)
		    goto discard;
		for (i = 0; i < sub_packets; i++)
		    totlen += sub_packet_lengths[i] = stream_read_word(demuxer->stream);
		if (len < totlen )
		    goto discard;
		for (i = 0; i < sub_packets; i++) {
		    demux_packet_t *dp = new_demux_packet(sub_packet_lengths[i]);if(dp == NULL) return 0;
		    stream_read(demuxer->stream, dp->buffer, sub_packet_lengths[i]);
		    if (priv->a_pts != timestamp)
			dp->pts = timestamp / 1000.0;
		    priv->a_pts = timestamp;
		    dp->pos = demuxer->filepos;
		    ds_add_packet(ds, dp);
		}
		return 1;
	    }
        if ((priv->intl_id[demuxer->audio->id] == mmioFOURCC('I', 'n', 't', '4')) ||
            (priv->intl_id[demuxer->audio->id] == mmioFOURCC('g', 'e', 'n', 'r')) ||
            (priv->intl_id[demuxer->audio->id] == mmioFOURCC('s', 'i', 'p', 'r'))) {
            if (!priv->audio_buf) {
                priv->audio_buf = (unsigned char *)calloc33(priv->sub_packet_h[demuxer->audio->id], priv->audiopk_size[demuxer->audio->id]);
                priv->audio_timestamp = (double *)calloc33(priv->sub_packet_h[demuxer->audio->id], sizeof(double));
            }
            sps = priv->sub_packet_size[demuxer->audio->id];
            sph = priv->sub_packet_h[demuxer->audio->id];
            cfs = priv->coded_framesize[demuxer->audio->id];
            w = priv->audiopk_size[demuxer->audio->id];
            spc = priv->sub_packet_cnt;
            switch (priv->intl_id[demuxer->audio->id]) {
                case mmioFOURCC('I', 'n', 't', '4'):
                    if (len < cfs * sph/2)
                        goto discard;
                    for (x = 0; x < sph / 2; x++)
                        stream_read(demuxer->stream, priv->audio_buf + x * 2 * w + spc * cfs, cfs);
                    break;
                case mmioFOURCC('g', 'e', 'n', 'r'):
                    if (len < w)
                        goto discard;
                    for (x = 0; x < w / sps; x++)
                        stream_read(demuxer->stream, priv->audio_buf + sps * (sph * x + ((sph + 1) / 2) * (spc & 1) +
                                    (spc >> 1)), sps);
                    break;
                case mmioFOURCC('s', 'i', 'p', 'r'):
                    if (len < w)
                        goto discard;
                    stream_read(demuxer->stream, priv->audio_buf + spc * w, w);
                    if (spc == sph - 1) {
                        int n;
                        int bs = sph * w * 2 / 96;  // nibbles per subpacket
                        // Perform reordering
                        for(n=0; n < 38; n++) {
                            int j;
                            int i = bs * sipr_swaps[n][0];
                            int o = bs * sipr_swaps[n][1];
                            // swap nibbles of block 'i' with 'o'      TODO: optimize
                            for(j = 0;j < bs; j++) {
                                int x = (i & 1) ? (priv->audio_buf[i >> 1] >> 4) : (priv->audio_buf[i >> 1] & 0x0F);
                                int y = (o & 1) ? (priv->audio_buf[o >> 1] >> 4) : (priv->audio_buf[o >> 1] & 0x0F);
                                if(o & 1)
                                    priv->audio_buf[o >> 1] = (priv->audio_buf[o >> 1] & 0x0F) | (x << 4);
                                else
                                    priv->audio_buf[o >> 1] = (priv->audio_buf[o >> 1] & 0xF0) | x;
                                if(i & 1)
                                    priv->audio_buf[i >> 1] = (priv->audio_buf[i >> 1] & 0x0F) | (y << 4);
                                else
                                    priv->audio_buf[i >> 1] = (priv->audio_buf[i >> 1] & 0xF0) | y;
                                ++i; ++o;
                            }
                        }
                    }
                    break;
            }
            priv->audio_need_keyframe = 0;
            priv->audio_timestamp[priv->sub_packet_cnt] = (priv->a_pts==timestamp) ? MP_NOPTS_VALUE : (timestamp/1000.0);
            priv->a_pts = timestamp;
            if (priv->sub_packet_cnt == 0)
                priv->audio_filepos = demuxer->filepos;
            if (++(priv->sub_packet_cnt) < sph)
                audioreorder_getnextpk = 1;
            else {
                int apk_usize = ((sh_audio_t*)ds->sh)->wf->nBlockAlign;
                audioreorder_getnextpk = 0;
                priv->sub_packet_cnt = 0;
                // Release all the audio packets
                for (x = 0; x < sph*w/apk_usize; x++) {
                    dp = new_demux_packet(apk_usize);   if(dp == NULL) return 0;
                    memcpy(dp->buffer, priv->audio_buf + x * apk_usize, apk_usize);
                    /* Put timestamp only on packets that correspond to original audio packets in file */
		    if (x * apk_usize % w == 0)
			dp->pts = priv->audio_timestamp[x * apk_usize / w];
                    dp->pos = priv->audio_filepos; // all equal
                    dp->flags = x ? 0 : 0x10; // Mark first packet as keyframe
                    ds_add_packet(ds, dp);
                }
            }
        } else { // No interleaving
            dp = new_demux_packet(len);if(dp == NULL) return 0;
            stream_read(demuxer->stream, dp->buffer, len);

#ifdef CRACK_MATRIX
	    mp_msg(MSGT_DEMUX, MSGL_V,"*** audio block len=%d\n",len);
	    { // HACK - used for reverse engineering the descrambling matrix
		FILE* f=fopen("test.rm","r+"); if(f == NULL) return 0;
		fseek(f,spos,SEEK_SET);
		++cnt;
//		    for(i=0;i<len;i++) dp->buffer[i]=i/0x12;
//		    for(i=0;i<len;i++) dp->buffer[i]=i;
//		    for(i=0;i<len;i++) dp->buffer[i]=cnt;
//		    for(i=0;i<len;i++) dp->buffer[i]=cnt<<4;
		    for(i=0;i<len;i++) dp->buffer[i]=(i==cnt2) ? (cnt+16*(8+cnt)) : 0;
		if(cnt==6){ cnt=0; ++cnt2; }
		fwrite(dp->buffer, len, 1, f);
		fclose(f);
		if(cnt2>0x150) *((int*)NULL)=1; // sig11 :)
	    }
#endif
#if 0
	    if( ((sh_audio_t *)ds->sh)->format == 0x2000) {
		// if DNET, swap bytes, as DNET is byte-swapped AC3:
		char *ptr = dp->buffer;
		int i;
		for (i = 0; i < len; i += 2)
		{
		    const char tmp = ptr[0];
		    ptr[0] = ptr[1];
		    ptr[1] = tmp;
		    ptr += 2;
		}
	    }
#endif
	    if (priv->audio_need_keyframe == 1) {
		priv->audio_need_keyframe = 0;
	    } else if(priv->a_pts != timestamp)
	        dp->pts = timestamp/1000.0;
	    priv->a_pts=timestamp;
	    dp->pos = demuxer->filepos;
	    dp->flags = (flags & 0x2) ? 0x10 : 0;
	    ds_add_packet(ds, dp);

        } // codec_id check, codec default case
	}
// we will not use audio index if we use -idx and have a video
	if(((!demuxer->video->sh && index_mode == 2) || priv->is_multirate) && (unsigned)demuxer->audio->id < MAX_STREAMS) {
		while (priv->current_apacket + 1 < priv->index_table_size[rm_stream_id] &&
		       timestamp > priv->index_table[rm_stream_id][priv->current_apacket].timestamp) {
			priv->current_apacket += 1;
			priv->stream_switch = 1;
		}
		if (priv->stream_switch)
			priv->audio_curpos = stream_tell(demuxer->stream);
	}

    // If we're reordering audio packets and we need more data get it
    if (audioreorder_getnextpk)
        continue;

	return 1;
    }

    if(demuxer->video->id==mp_stream_id){
got_video:
	ds=demuxer->video;
	mp_dbg(MSGT_DEMUX,MSGL_DBG2, "packet is video (mp_id: %d)\n", mp_stream_id);

	// parse video chunk:
	{
	    // we need a more complicated, 2nd level demuxing, as the video
	    // frames are stored fragmented in the video chunks :(
	    sh_video_t *sh_video = ds->sh;
	    demux_packet_t *dp;
	    unsigned vpkg_header, vpkg_length, vpkg_offset;
	    int vpkg_seqnum=-1;
	    int vpkg_subseq=0;

	    while(len>2){
		dp_hdr_t* dp_hdr;
		unsigned char* dp_data;
//		uint8_t* extra;

//		printf("xxx len=%d  \n",len);

		// read packet header
		// bit 7: 1=last block in block chain
		// bit 6: 1=short header (only one block?)
		vpkg_header=stream_read_char(demuxer->stream); --len;
		mp_dbg(MSGT_DEMUX,MSGL_DBG2, "hdr: %02X (len=%d) ",vpkg_header,len);

		if (0x40==(vpkg_header&0xc0)) {//whole frame
		    // seems to be a very short header
	    	    // 2 bytes, purpose of the second byte yet unknown
	    	    int bummer;
		    bummer=stream_read_char(demuxer->stream); --len;
 		    mp_dbg(MSGT_DEMUX,MSGL_DBG2,  "%02X",bummer);
 	    	    vpkg_offset=0;
 		    vpkg_length=(unsigned)len;
		} else {

		    if (0==(vpkg_header&0x40)) {//partial frame  or last partial frame
			// sub-seqnum (bits 0-6: number of fragment. bit 7: ???)
		        vpkg_subseq=stream_read_char(demuxer->stream);
	                --len;
		        mp_dbg(MSGT_DEMUX,MSGL_DBG2,  "subseq: %02X ",vpkg_subseq);
			vpkg_subseq&=0x7f;
	            }

	  	    // size of the complete packet
		    // bit 14 is always one (same applies to the offset)
		    vpkg_length=stream_read_word(demuxer->stream);
                 broken_up_by_us = vpkg_length>>15;
		    len-=2;
		    mp_dbg(MSGT_DEMUX,MSGL_DBG2, "l: %02X %02X ",vpkg_length>>8,vpkg_length&0xff);
		    if (!(vpkg_length&0xC000)) {
			vpkg_length<<=16;
		        vpkg_length|=(uint16_t)stream_read_word(demuxer->stream);
		        mp_dbg(MSGT_DEMUX,MSGL_DBG2, "l+: %02X %02X ",(vpkg_length>>8)&0xff,vpkg_length&0xff);
	    	        len-=2;
		    } else
		    vpkg_length&=0x3fff;

		    // offset of the following data inside the complete packet
		    // Note: if (hdr&0xC0)==0x80 then offset is relative to the
		    // _end_ of the packet, so it's equal to fragment size!!!
		    vpkg_offset=stream_read_word(demuxer->stream);
	            len-=2;
		    mp_dbg(MSGT_DEMUX,MSGL_DBG2, "o: %02X %02X ",vpkg_offset>>8,vpkg_offset&0xff);
		    if (!(vpkg_offset&0xC000)) {
			vpkg_offset<<=16;
		        vpkg_offset|=(uint16_t)stream_read_word(demuxer->stream);
		        mp_dbg(MSGT_DEMUX,MSGL_DBG2, "o+: %02X %02X ",(vpkg_offset>>8)&0xff,vpkg_offset&0xff);
	    	        len-=2;
		    } else
		    vpkg_offset&=0x3fff;

		    vpkg_seqnum=stream_read_char(demuxer->stream); --len;
		    mp_dbg(MSGT_DEMUX,MSGL_DBG2, "seq: %02X ",vpkg_seqnum);
	        }
 		mp_dbg(MSGT_DEMUX,MSGL_DBG2, "\n");
                mp_dbg(MSGT_DEMUX,MSGL_DBG2, "blklen=%d\n", len);
		mp_msg(MSGT_DEMUX,MSGL_DBG2, "block: hdr=0x%0x, len=%d, offset=%d, seqnum=%d\n",
		    vpkg_header, vpkg_length, vpkg_offset, vpkg_seqnum);

		if(ds->asf_packet){
		    dp=ds->asf_packet;
		    dp_hdr=(dp_hdr_t*)dp->buffer;
		    dp_data=dp->buffer+dp_hdr->header_buffer_len;
		    pkt_hdr=(packet_hdr_t *)(dp->buffer+sizeof(dp_hdr_t));
		    mp_dbg(MSGT_DEMUX,MSGL_DBG2, "we have an incomplete packet (oldseq=%d new=%d)\n",ds->asf_seq,vpkg_seqnum);
		    // we have an incomplete packet:
		    if(ds->asf_seq!=vpkg_seqnum){
			// this fragment is for new packet, close the old one
			mp_msg(MSGT_DEMUX,MSGL_DBG2, "closing probably incomplete packet, len: %d  \n",dp->len);
			queue_video_packet(priv, ds, dp);
			ds->asf_packet=NULL;
		    } else {
			// append data to it!
			++dp_hdr->chunks;
			mp_msg(MSGT_DEMUX,MSGL_DBG2,"[chunks=%d  subseq=%d]\n",dp_hdr->chunks,vpkg_subseq);

                   pkt_hdr += dp_hdr->chunks;
                   pkt_hdr->offset = dp_hdr->len;
                   pkt_hdr->broken_up_by_us = broken_up_by_us;

			if(0x80==(vpkg_header&0xc0)){
			    // last fragment!
			    if(dp_hdr->len!=vpkg_length-vpkg_offset)
				mp_msg(MSGT_DEMUX,MSGL_V,"warning! assembled.len=%d  frag.len=%d  total.len=%d  \n",dp->len,vpkg_offset,vpkg_length-vpkg_offset);
			    if (vpkg_offset > dp->len - dp_hdr->header_buffer_len - dp_hdr->len) vpkg_offset = dp->len - dp_hdr->header_buffer_len - dp_hdr->len;
            		    stream_read(demuxer->stream, dp_data+dp_hdr->len, vpkg_offset);
                       parse_slice_header(sh_video,demuxer,pkt_hdr,dp_data+dp_hdr->len, vpkg_offset);
			    if((dp_data[dp_hdr->len]&0x20) && (sh_video->format==0x30335652)) --dp_hdr->chunks; else
			    dp_hdr->len+=vpkg_offset;
			    len-=vpkg_offset;
 			    mp_dbg(MSGT_DEMUX,MSGL_DBG2, "fragment (%d bytes) appended, %d bytes left\n",vpkg_offset,len);
			    // we know that this is the last fragment -> we can close the packet!
			    queue_video_packet(priv, ds, dp);
			    ds->asf_packet=NULL;
			    // continue parsing
			    continue;
			}
			// non-last fragment:
			if(dp_hdr->len!=vpkg_offset)
			    mp_msg(MSGT_DEMUX,MSGL_V,"warning! assembled.len=%d  offset=%d  frag.len=%d  total.len=%d  \n",dp->len,vpkg_offset,len,vpkg_length);
			if (len > dp->len - dp_hdr->header_buffer_len - dp_hdr->len) len = dp->len - dp_hdr->header_buffer_len - dp_hdr->len;
            		stream_read(demuxer->stream, dp_data+dp_hdr->len, len);
                   parse_slice_header(sh_video,demuxer,pkt_hdr,dp_data+dp_hdr->len, len);
			if((dp_data[dp_hdr->len]&0x20) && (sh_video->format==0x30335652)) --dp_hdr->chunks; else
			dp_hdr->len+=len;
			len=0;
			break; // no more fragments in this chunk!
		    }
		}
		// create new packet!
		dp = new_demux_packet(sizeof(dp_hdr_t)+vpkg_length+sizeof(packet_hdr_t)*(1+2*(vpkg_header&0x3F))+BUFFER_HEADER_EXTEND_FOR_WRITE);if(dp == NULL) return 0;
	    	// the timestamp seems to be in milliseconds
                dp->pos = demuxer->filepos;
                dp->flags = (flags & 0x2) ? 0x10 : 0;
		ds->asf_seq = vpkg_seqnum;
		dp_hdr=(dp_hdr_t*)dp->buffer;
		dp_hdr->chunks=0;
		dp_hdr->timestamp=timestamp;
		dp_hdr->header_buffer_len=sizeof(dp_hdr_t)+sizeof(packet_hdr_t)*(1+2*(vpkg_header&0x3F))+BUFFER_HEADER_EXTEND_FOR_WRITE;
		dp_data=dp->buffer+dp_hdr->header_buffer_len;
		pkt_hdr=(packet_hdr_t *)(dp->buffer+sizeof(dp_hdr_t));
             pkt_hdr->offset = 0;
             pkt_hdr->broken_up_by_us = broken_up_by_us;

		if(0x00==(vpkg_header&0xc0)){
		    // first fragment:
		    if (len > dp->len - dp_hdr->header_buffer_len) len = dp->len - dp_hdr->header_buffer_len;
		    dp_hdr->len=len;
		    stream_read(demuxer->stream, dp_data, len);
                 parse_slice_header(sh_video,demuxer,pkt_hdr,dp_data, len);
		    ds->asf_packet=dp;
		    len=0;
		    break;
		}
		// whole packet (not fragmented):
		if (vpkg_length > len) {
		    mp_msg(MSGT_DEMUX, MSGL_WARN,"\n******** WARNING: vpkg_length=%i > len=%i ********\n", vpkg_length, len);
		    /*
		     * To keep the video stream rolling, we need to break
		     * here. We shouldn't touch len to make sure rest of the
		     * broken packet is skipped.
		     */
		    break;
		}
		dp_hdr->len=vpkg_length; len-=vpkg_length;
		stream_read(demuxer->stream, dp_data, vpkg_length);
             parse_slice_header(sh_video,demuxer,pkt_hdr,dp_data, vpkg_length);
		queue_video_packet(priv, ds, dp);

	    } // while(len>0)

	    if(len){
		mp_msg(MSGT_DEMUX, MSGL_WARN,"\n******** !!!!!!!! BUG!! len=%d !!!!!!!!!!! ********\n",len);
		if(len>0) stream_skip(demuxer->stream, len);
	    }
	}
	if ((unsigned)demuxer->video->id < MAX_STREAMS) {
		while (priv->current_vpacket + 1 < priv->index_table_size[rm_stream_id] &&
		       timestamp > priv->index_table[rm_stream_id][priv->current_vpacket + 1].timestamp) {
			priv->current_vpacket += 1;
			priv->stream_switch = 1;
		}
		if (priv->stream_switch)
			priv->video_curpos = stream_tell(demuxer->stream);
	}

	return 1;
    }

if((unsigned)rm_stream_id<MAX_STREAMS){
    if(demuxer->audio->id==-1 && demuxer->a_streams[mp_stream_id]){
	sh_audio_t *sh = demuxer->a_streams[mp_stream_id];
	demuxer->audio->id=mp_stream_id;
	demuxer->audio->sh=sh;
        mp_msg(MSGT_DEMUX,MSGL_V,"Auto-selected RM audio ID = %d (rm id %d)\n",mp_stream_id, rm_stream_id);
	goto got_audio;
    }

    if(demuxer->video->id==-1 && demuxer->v_streams[mp_stream_id]){
	sh_video_t *sh = demuxer->v_streams[mp_stream_id];
	demuxer->video->id=mp_stream_id;
	demuxer->video->sh=sh;
        mp_msg(MSGT_DEMUX,MSGL_V,"Auto-selected RM video ID = %d (rm id %d)\n",mp_stream_id, rm_stream_id);
	goto got_video;
    }

}

    mp_msg(MSGT_DEMUX,MSGL_DBG2, "unknown stream id (%d)\n", rm_stream_id);
discard:
    stream_skip(demuxer->stream, len);
  }//    goto loop;
  return 0;
}

static demuxer_t* demux_open_real(demuxer_t* demuxer)
{
    real_priv_t* priv = demuxer->priv;
    int num_of_headers;
    int a_streams=0;
    int v_streams=0;
    int i;
    int header_size;

    priv->video_file_start = 1;

    header_size = stream_read_dword(demuxer->stream); /* header size */
    mp_msg(MSGT_DEMUX,MSGL_V, "real: Header size: %d\n", header_size);
    i = stream_read_word(demuxer->stream); /* version */
    mp_msg(MSGT_DEMUX,MSGL_V, "real: Header object version: %d\n", i);
    if (header_size == 0x10)
    	i = stream_read_word(demuxer->stream);
    else /* we should test header_size here too. */
    	i = stream_read_dword(demuxer->stream);
    mp_msg(MSGT_DEMUX,MSGL_V, "real: File version: %d\n", i);
    num_of_headers = stream_read_dword(demuxer->stream);

    /* parse chunks */
    for (i = 1; i <= num_of_headers; i++)
//    for (i = 1; ; i++)
    {
	int chunk_id, chunk_pos, chunk_size;

	chunk_pos = stream_tell(demuxer->stream);
	chunk_id = stream_read_dword_le(demuxer->stream);
	chunk_size = stream_read_dword(demuxer->stream);

	stream_skip(demuxer->stream, 2); /* version */

	mp_msg(MSGT_DEMUX,MSGL_V, "Chunk: %.4s (%x) (size: 0x%x, offset: 0x%x)\n",
	    (char *)&chunk_id, chunk_id, chunk_size, chunk_pos);

	if (chunk_id != MKTAG('D', 'A', 'T', 'A') && chunk_size < 10){
	    mp_msg(MSGT_DEMUX,MSGL_ERR,"demux_real: invalid chunksize! (%d)\n",chunk_size);
	    break; //return;
	}

	switch(chunk_id)
	{
	    case MKTAG('P', 'R', 'O', 'P'):
		/* Properties header */

		stream_skip(demuxer->stream, 4); /* max bitrate */
		stream_skip(demuxer->stream, 4); /* avg bitrate */
		stream_skip(demuxer->stream, 4); /* max packet size */
		stream_skip(demuxer->stream, 4); /* avg packet size */
		stream_skip(demuxer->stream, 4); /* nb packets */
		priv->duration = stream_read_dword(demuxer->stream)/1000; /* duration */
		stream_skip(demuxer->stream, 4); /* preroll */
		priv->index_chunk_offset = stream_read_dword(demuxer->stream);
		mp_msg(MSGT_DEMUX,MSGL_V,"First index chunk offset: 0x%x\n", priv->index_chunk_offset);
		priv->data_chunk_offset = stream_read_dword(demuxer->stream)+10;
		mp_msg(MSGT_DEMUX,MSGL_V,"First data chunk offset: 0x%x\n", priv->data_chunk_offset);
		priv->streams_in_file = stream_read_word(demuxer->stream);
		mp_msg(MSGT_DEMUX,MSGL_V,"Number of streams in file: %d\n", priv->streams_in_file);
#if 0
		stream_skip(demuxer->stream, 2); /* flags */
#else
		{
		    int flags = stream_read_word(demuxer->stream);

		    if (flags)
		    {
		    mp_msg(MSGT_DEMUX,MSGL_V,"Flags (%x): ", flags);
		    if (flags & 0x1)
			mp_msg(MSGT_DEMUX,MSGL_V,"[save allowed] ");
		    if (flags & 0x2)
			mp_msg(MSGT_DEMUX,MSGL_V,"[perfect play (more buffers)] ");
		    if (flags & 0x4)
			mp_msg(MSGT_DEMUX,MSGL_V,"[live broadcast] ");
		    mp_msg(MSGT_DEMUX,MSGL_V,"\n");
		    }
		}
#endif
		break;
	    case MKTAG('C', 'O', 'N', 'T'):
	    {
		/* Content description header */
		char *buf;
		int len;

		len = stream_read_word(demuxer->stream);
		if (len > 0)
		{
		    buf = (char *)malloc33(len+1);
		    stream_read(demuxer->stream, buf, len);
		    buf[len] = 0;
		    demux_info_add(demuxer, "title", buf);
		    free33(buf);
		}

		len = stream_read_word(demuxer->stream);
		if (len > 0)
		{
		    buf = (char *)malloc33(len+1);
		    stream_read(demuxer->stream, buf, len);
		    buf[len] = 0;
		    demux_info_add(demuxer, "author", buf);
		    free33(buf);
		}

		len = stream_read_word(demuxer->stream);
		if (len > 0)
		{
		    buf = (char *)malloc33(len+1);
		    stream_read(demuxer->stream, buf, len);
		    buf[len] = 0;
		    demux_info_add(demuxer, "copyright", buf);
		    free33(buf);
		}

		len = stream_read_word(demuxer->stream);
		if (len > 0)
		{
		    buf = (char *)malloc33(len+1);
	    	    stream_read(demuxer->stream, buf, len);
		    buf[len] = 0;
		    demux_info_add(demuxer, "comment", buf);
		    free33(buf);
		}
		break;
	    }
	    case MKTAG('M', 'D', 'P', 'R'):
	    {
		/* Media properties header */
		int stream_id;
		int bitrate;
		int codec_data_size;
		int codec_pos;
		int tmp;
		int len;
		char *descr, *mimet = NULL;
		stream_id = stream_read_word(demuxer->stream);
		mp_msg(MSGT_DEMUX,MSGL_V,"Found new stream (id: %d)\n", stream_id);

		stream_skip(demuxer->stream, 4); /* max bitrate */
		bitrate = stream_read_dword(demuxer->stream); /* avg bitrate */
		stream_skip(demuxer->stream, 4); /* max packet size */
		stream_skip(demuxer->stream, 4); /* avg packet size */
		stream_skip(demuxer->stream, 4); /* start time */
		stream_skip(demuxer->stream, 4); /* preroll */
		stream_skip(demuxer->stream, 4); /* duration */

		if ((len = stream_read_char(demuxer->stream)) > 0) {
		    descr = (char *)malloc33(len+1);
	    	stream_read(demuxer->stream, descr, len);
		    descr[len] = 0;
		    mp_msg(MSGT_DEMUX, MSGL_INFO,"Stream description: %s\n", descr);
		    free33(descr);
		}
		if ((len = stream_read_char(demuxer->stream)) > 0) {
		    mimet = (char *)malloc33(len+1);
	    	stream_read(demuxer->stream, mimet, len);
		    mimet[len] = 0;
		    mp_msg(MSGT_DEMUX, MSGL_INFO,"Stream mimetype: %s\n", mimet);
		}

		/* Type specific header */
		codec_data_size = stream_read_dword(demuxer->stream);
		codec_pos = stream_tell(demuxer->stream);

#ifdef MP_DEBUG   //tscan ignore it
//#define stream_skip(st,siz) { int i; for(i=0;i<siz;i++) mp_msg(MSGT_DEMUX,MSGL_V," %02X",stream_read_char(st)); mp_msg(MSGT_DEMUX,MSGL_V,"\n");}
#endif

	if (!strncmp(mimet,"audio/",6)) {
	  if (strstr(mimet,"x-pn-realaudio") || strstr(mimet,"x-pn-multirate-realaudio")) {
		int num_mlti, mlti_cnt, ra_size;
		tmp = stream_read_dword(demuxer->stream);
		if (tmp == MKTAG('I', 'T', 'L', 'M')) // MLTI chunk in audio
		{
		    int num_streams, stream_cnt;
		    mp_msg(MSGT_DEMUX,MSGL_V,"MLTI chunk in audio.\n");
		    num_streams = stream_read_word(demuxer->stream);
		    for (stream_cnt = 0; stream_cnt < num_streams; stream_cnt++)
		        stream_skip(demuxer->stream, 2); // MDPR index, one per stream
		    num_mlti = stream_read_word(demuxer->stream);
		    if (num_mlti != 1) {
		        mp_msg(MSGT_DEMUX,MSGL_V,"Found MLTI in audio with %d substreams.\n", num_mlti);
		        priv->is_mlti = 1;
		    } else
		        mp_msg(MSGT_DEMUX,MSGL_V,"Found MLTI in audio with 1 substream. Ignoring\n");
		    if (num_mlti > MAX_MLTIIDX) {
		        mp_msg(MSGT_DEMUX,MSGL_ERR,"Too many (%d) MLTI audio, truncating; expect problems. Please report to Mplayer developers.\n", num_mlti);
		        num_mlti = MAX_MLTIIDX - 1; // Limit to max MLTI
		    }
		    ra_size = stream_read_dword(demuxer->stream); // Size of the following .ra chunk
		    tmp = stream_read_dword(demuxer->stream);
		} else {
		    num_mlti = 1;
		    ra_size = codec_data_size;
		}
		for (mlti_cnt = 0; mlti_cnt < num_mlti; mlti_cnt++) {
		if (mlti_cnt) {
		    ra_size = stream_read_dword(demuxer->stream); // Size of the following .ra chunk
		    tmp = stream_read_dword(demuxer->stream);
		}
		if (tmp != MKTAG(0xfd, 'a', 'r', '.'))
		{
		    mp_msg(MSGT_DEMUX,MSGL_V,"Audio: can't find .ra in codec data\n");
		    stream_skip(demuxer->stream, ra_size - 4);
		} else {
		    /* audio header */
		    int aid = priv->is_mlti ? priv->streams_in_file + a_streams + v_streams : stream_id;
		    sh_audio_t *sh = new_sh_audio(demuxer, aid, NULL);if(sh == NULL) return NULL;
		    char buf[128]; /* for codec name */
		    int frame_size;
		    int sub_packet_size = 0;
		    int sub_packet_h = 0;
		    int version;
		    int coded_frame_size = 0;
		    int codecdata_length;
		    int i;
		    char *buft;
		    int hdr_size;
		    mp_msg(MSGT_DEMUX, MSGL_INFO, MSGTR_AudioID, "real", aid);
		    priv->mp2rm_streamid[aid] = stream_id;
		    priv->rm2mp[stream_id][mlti_cnt] = aid;
		    mp_msg(MSGT_DEMUX,MSGL_V,"Mplayer aid %d is rm stream %d with MDPR index %d\n", aid, stream_id, mlti_cnt);
		    mp_msg(MSGT_DEMUX,MSGL_V,"Found audio stream!\n");
		    version = stream_read_word(demuxer->stream);
		    mp_msg(MSGT_DEMUX,MSGL_V,"version: %d\n", version);
                   if (version == 3) {
                    stream_skip(demuxer->stream, 2);
                    stream_skip(demuxer->stream, 10);
                    stream_skip(demuxer->stream, 4);
                    // Name, author, (c) are also in CONT tag
                    if ((i = stream_read_char(demuxer->stream)) != 0) {
                      buft = (char *)malloc33(i+1);
                      stream_read(demuxer->stream, buft, i);
                      buft[i] = 0;
                      demux_info_add(demuxer, "Name", buft);
                      free33(buft);
                    }
                    if ((i = stream_read_char(demuxer->stream)) != 0) {
                      buft = (char *)malloc33(i+1);
                      stream_read(demuxer->stream, buft, i);
                      buft[i] = 0;
                      demux_info_add(demuxer, "Author", buft);
                      free33(buft);
                    }
                    if ((i = stream_read_char(demuxer->stream)) != 0) {
                      buft = (char *)malloc33(i+1);
                      stream_read(demuxer->stream, buft, i);
                      buft[i] = 0;
                      demux_info_add(demuxer, "Copyright", buft);
                      free33(buft);
                    }
                    if ((i = stream_read_char(demuxer->stream)) != 0)
                      mp_msg(MSGT_DEMUX,MSGL_WARN,"Last header byte is not zero!\n");
                    stream_skip(demuxer->stream, 1);
                    i = stream_read_char(demuxer->stream);
                    sh->format = stream_read_dword_le(demuxer->stream);
                    if (i != 4) {
                      mp_msg(MSGT_DEMUX,MSGL_WARN,"Audio FourCC size is not 4 (%d), please report to "
                             "MPlayer developers\n", i);
                      stream_skip(demuxer->stream, i - 4);
                    }
                    if (sh->format != mmioFOURCC('l','p','c','J')) {
                      mp_msg(MSGT_DEMUX,MSGL_WARN,"Version 3 audio with FourCC %8x, please report to "
                             "MPlayer developers\n", sh->format);
                    }
                    sh->channels = 1;
                    sh->samplesize = 16;
                    sh->samplerate = 8000;
                    frame_size = 240;
                    strcpy(buf, "14_4");
                   } else {
		    stream_skip(demuxer->stream, 2); // 00 00
		    stream_skip(demuxer->stream, 4); /* .ra4 or .ra5 */
		    stream_skip(demuxer->stream, 4); // ???
		    stream_skip(demuxer->stream, 2); /* version (4 or 5) */
		    hdr_size = stream_read_dword(demuxer->stream); // header size
		    mp_msg(MSGT_DEMUX,MSGL_V,"header size: %d\n", hdr_size);
		    stream_skip(demuxer->stream, 2);/* codec flavor id */
		    coded_frame_size = stream_read_dword(demuxer->stream);/* needed by codec */
		    mp_msg(MSGT_DEMUX,MSGL_V,"coded_frame_size: %d\n", coded_frame_size);
		    stream_skip(demuxer->stream, 4); // big number
		    stream_skip(demuxer->stream, 4); // bigger number
		    stream_skip(demuxer->stream, 4); // 2 || -''-
		    sub_packet_h = stream_read_word(demuxer->stream);
		    mp_msg(MSGT_DEMUX,MSGL_V,"sub_packet_h: %d\n", sub_packet_h);
		    frame_size = stream_read_word(demuxer->stream);
		    mp_msg(MSGT_DEMUX,MSGL_V,"frame_size: %d\n", frame_size);
		    sub_packet_size = stream_read_word(demuxer->stream);
		    mp_msg(MSGT_DEMUX,MSGL_V,"sub_packet_size: %d\n", sub_packet_size);
		    stream_skip(demuxer->stream, 2); // 0

		    if (version == 5)
			stream_skip(demuxer->stream, 6); //0,srate,0

		    sh->samplerate = stream_read_word(demuxer->stream);
		    stream_skip(demuxer->stream, 2);  // 0
		    sh->samplesize = stream_read_word(demuxer->stream)/8;
		    sh->channels = stream_read_word(demuxer->stream);
		    mp_msg(MSGT_DEMUX,MSGL_V,"samplerate: %d, channels: %d\n",
			sh->samplerate, sh->channels);

		    if (version == 5)
		    {
			stream_read(demuxer->stream, buf, 4);  // interleaver id
			priv->intl_id[aid] = MKTAG(buf[0], buf[1], buf[2], buf[3]);
			stream_read(demuxer->stream, buf, 4); // fourcc
			buf[4] = 0;
		    }
		    else
		    {
			/* Interleaver id */
			get_str(1, demuxer, buf, sizeof(buf));
			priv->intl_id[aid] = MKTAG(buf[0], buf[1], buf[2], buf[3]);
			/* Codec FourCC */
			get_str(1, demuxer, buf, sizeof(buf));
		    }
                   }

		    /* Emulate WAVEFORMATEX struct: */
		    sh->wf = (WAVEFORMATEX*)calloc33(1, sizeof(*sh->wf));
		    sh->wf->nChannels = sh->channels;
		    sh->wf->wBitsPerSample = sh->samplesize*8;
		    sh->wf->nSamplesPerSec = sh->samplerate;
		    sh->wf->nAvgBytesPerSec = bitrate/8;
		    sh->wf->nBlockAlign = frame_size;
		    sh->wf->cbSize = 0;
		    sh->format = MKTAG(buf[0], buf[1], buf[2], buf[3]);

		    switch (sh->format)
		    {
			case MKTAG('d', 'n', 'e', 't'):
			    mp_msg(MSGT_DEMUX,MSGL_V,"Audio: DNET -> AC3\n");
//			    sh->format = 0x2000;
			    break;
			case MKTAG('1', '4', '_', '4'):
                sh->wf->nBlockAlign = 0x14;
                            break;

			case MKTAG('2', '8', '_', '8'):
			    sh->wf->nBlockAlign = coded_frame_size;
			    break;

			case MKTAG('s', 'i', 'p', 'r'):
			case MKTAG('a', 't', 'r', 'c'):
			case MKTAG('c', 'o', 'o', 'k'):
			    // realaudio codec plugins - common:
			    stream_skip(demuxer->stream,3);  // Skip 3 unknown bytes
			    if (version==5)
			      stream_skip(demuxer->stream,1);  // Skip 1 additional unknown byte
			    codecdata_length=stream_read_dword(demuxer->stream);
			    // Check extradata len, we can't store bigger values in cbSize anyway
			    if ((unsigned)codecdata_length > 0xffff) {
			        mp_msg(MSGT_DEMUX,MSGL_ERR,"Extradata too big (%d)\n", codecdata_length);
				goto skip_this_chunk;
			    }
			    sh->wf->cbSize = codecdata_length;
			    sh->wf = (WAVEFORMATEX*)realloc33(sh->wf, sizeof(*sh->wf)+sh->wf->cbSize);if(sh->wf == NULL) return NULL;
			    stream_read(demuxer->stream, ((char*)(sh->wf+1)), codecdata_length); // extras
                if (priv->intl_id[aid] == MKTAG('g', 'e', 'n', 'r'))
    			    sh->wf->nBlockAlign = sub_packet_size;
    			else
    			    sh->wf->nBlockAlign = coded_frame_size;

			    break;

			case MKTAG('r', 'a', 'a', 'c'):
			case MKTAG('r', 'a', 'c', 'p'):
			    /* This is just AAC. The two or five bytes of */
			    /* config data needed for libfaad are stored */
			    /* after the audio headers. */
			    stream_skip(demuxer->stream,3);  // Skip 3 unknown bytes
			    if (version==5)
				stream_skip(demuxer->stream,1);  // Skip 1 additional unknown byte
			    codecdata_length=stream_read_dword(demuxer->stream);
			    if (codecdata_length>=1) {
				sh->codecdata_len = codecdata_length - 1;
				sh->codecdata = (unsigned char*)calloc33(sh->codecdata_len, 1);
				stream_skip(demuxer->stream, 1);
				stream_read(demuxer->stream, sh->codecdata, sh->codecdata_len);
			    }
			    sh->format = mmioFOURCC('M', 'P', '4', 'A');
			    break;
			default:
			    mp_msg(MSGT_DEMUX,MSGL_V,"Audio: Unknown (%s)\n", buf);
		    }

		    // Interleaver setup
		    priv->sub_packet_size[aid] = sub_packet_size;
		    priv->sub_packet_h[aid] = sub_packet_h;
		    priv->coded_framesize[aid] = coded_frame_size;
		    priv->audiopk_size[aid] = frame_size;

		    sh->wf->wFormatTag = sh->format;

		    mp_msg(MSGT_DEMUX,MSGL_V,"audio fourcc: %.4s (%x)\n", (char *)&sh->format, sh->format);
		    if ( mp_msg_test(MSGT_DEMUX,MSGL_V) )
		    print_wave_header(sh->wf, MSGL_V);

		    /* Select audio stream with highest bitrate if multirate file*/
		    if (priv->is_multirate && ((demuxer->audio->id == -1) ||
		                               ((demuxer->audio->id >= 0) && priv->a_bitrate && (bitrate > priv->a_bitrate)))) {
			    demuxer->audio->id = stream_id;
			    demuxer->audio->sh = sh;
			    priv->a_bitrate = bitrate;
			    mp_msg(MSGT_DEMUX,MSGL_DBG2,"Multirate autoselected audio id %d with bitrate %d\n", stream_id, bitrate);
		    }

		    ++a_streams;

#ifdef stream_skip
#undef stream_skip
#endif
		} // .ra
		} // MLTI
	  } else if (strstr(mimet,"X-MP3-draft-00")) {
		    sh_audio_t *sh = new_sh_audio(demuxer, stream_id, NULL);if(sh == NULL) return NULL;
    		    mp_msg(MSGT_DEMUX, MSGL_INFO, MSGTR_AudioID, "real", stream_id);

		    /* Emulate WAVEFORMATEX struct: */
		    sh->wf = (WAVEFORMATEX*)calloc33(1, sizeof(*sh->wf));if(sh->wf == NULL) return NULL;
		    sh->wf->nChannels = 0;//sh->channels;
		    sh->wf->wBitsPerSample = 16;
		    sh->wf->nSamplesPerSec = 0;//sh->samplerate;
		    sh->wf->nAvgBytesPerSec = 0;//bitrate;
		    sh->wf->nBlockAlign = 0;//frame_size;
		    sh->wf->cbSize = 0;
		    sh->wf->wFormatTag = sh->format = mmioFOURCC('a','d','u',0x55);

		    ++a_streams;
	  } else if (strstr(mimet,"x-ralf-mpeg4")) {
		    sh_audio_t *sh = new_sh_audio(demuxer, stream_id, NULL);if(sh == NULL) return NULL;
    		    mp_msg(MSGT_DEMUX, MSGL_INFO, MSGTR_AudioID, "real", stream_id);

		    // Check extradata len, we can't store bigger values in cbSize anyway
		    if ((unsigned)codec_data_size > 0xffff) {
		        mp_msg(MSGT_DEMUX,MSGL_ERR,"Extradata too big (%d)\n", codec_data_size);
			goto skip_this_chunk;
		    }
		    /* Emulate WAVEFORMATEX struct: */
		    sh->wf = (WAVEFORMATEX*)calloc33(1, sizeof(*sh->wf)+codec_data_size);if(sh->wf == NULL) return NULL;
		    sh->wf->nChannels = 0;
		    sh->wf->wBitsPerSample = 16;
		    sh->wf->nSamplesPerSec = 0;
		    sh->wf->nAvgBytesPerSec = 0;
		    sh->wf->nBlockAlign = 0;
		    sh->wf->wFormatTag = sh->format = mmioFOURCC('L','S','D',':');
		    sh->wf->cbSize = codec_data_size;
		    stream_read(demuxer->stream, (char*)(sh->wf+1), codec_data_size);

		    ++a_streams;
	  } else if (strstr(mimet,"x-pn-encrypted-ra")) {
		 mp_msg(MSGT_DEMUX,MSGL_ERR,"Encrypted audio is not supported\n");
	  } else {
		 mp_msg(MSGT_DEMUX,MSGL_V,"Unknown audio stream format\n");
		}
	} else if (!strncmp(mimet,"video/",6)) {
	  if (strstr(mimet,"x-pn-realvideo") || strstr(mimet,"x-pn-multirate-realvideo")) {
		int num_mlti, mlti_cnt, vido_size, vido_pos;
		tmp = stream_read_dword(demuxer->stream);
		if (tmp == MKTAG('I', 'T', 'L', 'M')) // MLTI chunk in video
		{
		    int num_streams, stream_cnt;
		    mp_msg(MSGT_DEMUX,MSGL_V,"MLTI chunk in video.\n");
		    num_streams = stream_read_word(demuxer->stream);
		    for (stream_cnt = 0; stream_cnt < num_streams; stream_cnt++)
		        stream_skip(demuxer->stream, 2); // MDPR index, one per stream
		    num_mlti = stream_read_word(demuxer->stream);
		    if (num_mlti != 1) {
		        mp_msg(MSGT_DEMUX,MSGL_V,"Found MLTI in video with %d substreams.\n", num_mlti);
		         priv->is_mlti = 1;
		    } else
		        mp_msg(MSGT_DEMUX,MSGL_V,"Found MLTI in audio with 1 substream. Ignoring\n");
		    if (num_mlti > MAX_MLTIIDX) {
		        mp_msg(MSGT_DEMUX,MSGL_ERR,"Too many (%d) MLTI video, truncating; expect problems. Please report to Mplayer developers.\n", num_mlti);
		        num_mlti = MAX_MLTIIDX - 1; // Limit to max MLTI
		    }
		    vido_size = stream_read_dword(demuxer->stream); // Size of the following .vido chunk
		    vido_pos = stream_tell(demuxer->stream);;
		    stream_skip(demuxer->stream, 4);
		    tmp = stream_read_dword(demuxer->stream);
		    priv->is_mlti = 1;
		} else {
		    num_mlti = 1;
		    vido_size = codec_data_size;
		    vido_pos = codec_pos;
		    tmp = stream_read_dword(demuxer->stream);
		}
		for (mlti_cnt = 0; mlti_cnt < num_mlti; mlti_cnt++) {
		if (mlti_cnt) {
		    vido_size = stream_read_dword(demuxer->stream); // Size of the following vido chunk
		    mp_msg(MSGT_DEMUX,MSGL_V,"VIDO size: %x\n", vido_size);
		    vido_pos = stream_tell(demuxer->stream);;
		    stream_skip(demuxer->stream, 4);
		    tmp = stream_read_dword(demuxer->stream);
		}
		if(tmp != MKTAG('O', 'D', 'I', 'V'))
		{
		    mp_msg(MSGT_DEMUX,MSGL_V,"Video: can't find VIDO in codec data\n");
		    stream_skip(demuxer->stream, vido_size - 4);
		} else {
		    /* video header */
		    int vid = priv->is_mlti ? priv->streams_in_file + a_streams + v_streams : stream_id;
		    sh_video_t *sh = new_sh_video(demuxer, vid);    if(sh == NULL) return NULL;
                 sh_real_opaque_data_t *opaque = &priv->real_video_opaque;
                 opaque->media_object_format = tmp;
                 opaque->opauqe_data_size = 0;

		    mp_msg(MSGT_DEMUX, MSGL_INFO, MSGTR_VideoID, "real", vid);
		    priv->mp2rm_streamid[vid] = stream_id;
		    priv->rm2mp[stream_id][mlti_cnt] = vid;
		    mp_msg(MSGT_DEMUX,MSGL_V,"Mplayer vid %d is rm stream %d with MDPR index %d\n", vid, stream_id, mlti_cnt);

		    sh->format = stream_read_dword_le(demuxer->stream); /* fourcc */
		    mp_msg(MSGT_DEMUX,MSGL_V,"video fourcc: %.4s (%x)\n", (char *)&sh->format, sh->format);

		    /* emulate BITMAPINFOHEADER */
		    sh->bih = (BITMAPINFOHEADER*)calloc33(1, sizeof(*sh->bih));
	    	    sh->bih->biSize = sizeof(*sh->bih);
		    priv->prev_width = sh->disp_w = sh->bih->biWidth = stream_read_word(demuxer->stream);
		    priv->prev_height = sh->disp_h = sh->bih->biHeight = stream_read_word(demuxer->stream);
		    if (sh->disp_w > 0 && sh->disp_h > 0)
			sh->original_aspect = (float)sh->disp_w / sh->disp_h;
		    sh->bih->biPlanes = 1;
		    sh->bih->biBitCount = 24;
		    sh->bih->biCompression = sh->format;
		    sh->bih->biSizeImage= sh->bih->biWidth*sh->bih->biHeight*3;

		    sh->fps = (float) stream_read_word(demuxer->stream);

                 //add by libin ------this 16 bits is bit_count in protocol ,but mplayer set it as fps ,dont know why??
                 opaque->bit_count = (unsigned short)sh->fps;

		    if (sh->fps<=0) sh->fps=24; // we probably won't even care about fps
		    sh->frametime = 1.0f/sh->fps;

#if 1
		    //stream_skip(demuxer->stream, 4);
		    opaque->pad_width = stream_read_word(demuxer->stream);
                 opaque->pad_height = stream_read_word(demuxer->stream);
#else
		    mp_msg(MSGT_DEMUX, MSGL_V,"unknown1: 0x%X  \n",stream_read_dword(demuxer->stream));
		    mp_msg(MSGT_DEMUX, MSGL_V,"unknown2: 0x%X  \n",stream_read_word(demuxer->stream));
		    mp_msg(MSGT_DEMUX, MSGL_V,"unknown3: 0x%X  \n",stream_read_word(demuxer->stream));
#endif
//		    if(sh->format==0x30335652 || sh->format==0x30325652 )
		    if(1)
		    {
			int tmp=stream_read_word(demuxer->stream);
			if(tmp>0){
			    sh->fps=tmp; sh->frametime = 1.0f/sh->fps;
			}
		    } else {
	    		int fps=stream_read_word(demuxer->stream);
			mp_msg(MSGT_DEMUX, MSGL_WARN,"realvid: ignoring FPS = %d\n",fps);
		    }
		    stream_skip(demuxer->stream, 2);

		    {
			    // read and store codec extradata
			    unsigned int cnt = vido_size - (stream_tell(demuxer->stream) - vido_pos);
			    if (cnt > 0x7fffffff - sizeof(*sh->bih)) {
			        mp_msg(MSGT_DEMUX, MSGL_ERR,"Extradata too big (%u)\n", cnt);
			    } else  {
				sh->bih = (BITMAPINFOHEADER*)realloc33(sh->bih, sizeof(*sh->bih) + cnt);if(sh->bih == NULL) return NULL;
			        sh->bih->biSize += cnt;
				stream_read(demuxer->stream, ((unsigned char*)(sh->bih+1)), cnt);
                          opaque->p_opaque_data = ((unsigned char*)(sh->bih+1));
                          opaque->opauqe_data_size = cnt;
			    }
		    }
		    if(sh->format == 0x30315652 && ((unsigned char*)(sh->bih+1))[6] == 0x30)
			    sh->bih->biCompression = sh->format = mmioFOURCC('R', 'V', '1', '3');

		    /* Select video stream with highest bitrate if multirate file*/
		    if (priv->is_multirate && ((demuxer->video->id == -1) ||
		                               ((demuxer->video->id >= 0) && priv->v_bitrate && (bitrate > priv->v_bitrate)))) {
			    demuxer->video->id = stream_id;
			    demuxer->video->sh = sh;
			    priv->v_bitrate = bitrate;
			    mp_msg(MSGT_DEMUX,MSGL_DBG2,"Multirate autoselected video id %d with bitrate %d\n", stream_id, bitrate);
		    }

		    ++v_streams;

		} // VIDO
		} // MLTI
	  } else {
		 mp_msg(MSGT_DEMUX,MSGL_V,"Unknown video stream format\n");
	  }
	} else if (strstr(mimet,"logical-")) {
		 if (strstr(mimet,"fileinfo")) {
		     mp_msg(MSGT_DEMUX,MSGL_V,"Got a logical-fileinfo chunk\n");
		 } else if (strstr(mimet,"-audio") || strstr(mimet,"-video")) {
		    int i, stream_cnt;
		    int stream_list[MAX_STREAMS];

		    priv->is_multirate = 1;
		    stream_skip(demuxer->stream, 4); // Length of codec data (repeated)
		    stream_cnt = stream_read_dword(demuxer->stream); // Get number of audio or video streams
		    if ((unsigned)stream_cnt >= MAX_STREAMS) {
		        mp_msg(MSGT_DEMUX,MSGL_ERR,"Too many streams in %s. Big troubles ahead.\n", mimet);
		        goto skip_this_chunk;
		    }
		    for (i = 0; i < stream_cnt; i++)
		        stream_list[i] = stream_read_word(demuxer->stream);
		    for (i = 0; i < stream_cnt; i++)
		        if ((unsigned)stream_list[i] >= MAX_STREAMS) {
		            mp_msg(MSGT_DEMUX,MSGL_ERR,"Stream id out of range: %d. Ignored.\n", stream_list[i]);
		            stream_skip(demuxer->stream, 4); // Skip DATA offset for broken stream
		        } else {
		            priv->str_data_offset[stream_list[i]] = stream_read_dword(demuxer->stream);
		            mp_msg(MSGT_DEMUX,MSGL_V,"Stream %d with DATA offset 0x%08x\n", stream_list[i], priv->str_data_offset[stream_list[i]]);
		        }
		    // Skip the rest of this chunk
		 } else
		     mp_msg(MSGT_DEMUX,MSGL_V,"Unknown logical stream\n");
		}
		else {
		    mp_msg(MSGT_DEMUX, MSGL_ERR, "Not audio/video stream or unsupported!\n");
		}
//		break;
//	    default:
skip_this_chunk:
		/* skip codec info */
		tmp = stream_tell(demuxer->stream) - codec_pos;
		mp_msg(MSGT_DEMUX,MSGL_V,"### skipping %d bytes of codec info\n", codec_data_size - tmp);
#if 0
		{ int i;
		  for(i=0;i<codec_data_size - tmp;i++)
		      mp_msg(MSGT_DEMUX, MSGL_V," %02X",stream_read_char(demuxer->stream));
		  mp_msg(MSGT_DEMUX, MSGL_V,"\n");
		}
#else
		stream_skip(demuxer->stream, codec_data_size - tmp);
#endif
		free33(mimet);
		break;
//	    }
	    }
	    case MKTAG('D', 'A', 'T', 'A'):
		goto header_end;
	    case MKTAG('I', 'N', 'D', 'X'):
	    default:
		mp_msg(MSGT_DEMUX,MSGL_V,"Unknown chunk: %x\n", chunk_id);
		stream_skip(demuxer->stream, chunk_size - 10);
		break;
	}
    }

header_end:
    if(priv->is_multirate && priv->is_mlti)
        mp_msg(MSGT_DEMUX,MSGL_ERR,"Multirate and MLTI in the same file is bad. Please contact Mplayer developers.\n");

    if(priv->is_multirate) {
        mp_msg(MSGT_DEMUX,MSGL_V,"Selected video id %d audio id %d\n", demuxer->video->id, demuxer->audio->id);
        /* Perform some sanity checks to avoid checking streams id all over the code*/
        if (demuxer->audio->id >= MAX_STREAMS) {
            mp_msg(MSGT_DEMUX,MSGL_ERR,"Invalid audio stream %d. No sound will be played.\n", demuxer->audio->id);
            demuxer->audio->id = -2;
        } else if ((demuxer->audio->id >= 0) && (priv->str_data_offset[demuxer->audio->id] == 0)) {
            mp_msg(MSGT_DEMUX,MSGL_ERR,"Audio stream %d not found. No sound will be played.\n", demuxer->audio->id);
            demuxer->audio->id = -2;
        }
        if (demuxer->video->id >= MAX_STREAMS) {
            mp_msg(MSGT_DEMUX,MSGL_ERR,"Invalid video stream %d. No video will be played.\n", demuxer->video->id);
            demuxer->video->id = -2;
        } else if ((demuxer->video->id >= 0) && (priv->str_data_offset[demuxer->video->id] == 0)) {
            mp_msg(MSGT_DEMUX,MSGL_ERR,"Video stream %d not found. No video will be played.\n", demuxer->video->id);
            demuxer->video->id = -2;
        }
    }

    if(priv->is_multirate && ((demuxer->video->id >= 0) || (demuxer->audio->id  >=0))) {
        /* If audio or video only, seek to right place and behave like standard file */
        if (demuxer->video->id < 0) {
            // Stream is audio only, or -novideo
            stream_seek(demuxer->stream, priv->data_chunk_offset = priv->str_data_offset[demuxer->audio->id]+10);
            priv->is_multirate = 0;
        }
        if (demuxer->audio->id < 0) {
            // Stream is video only, or -nosound
            stream_seek(demuxer->stream, priv->data_chunk_offset = priv->str_data_offset[demuxer->video->id]+10);
            priv->is_multirate = 0;
        }
    }

  if(!priv->is_multirate) {
//    printf("i=%d num_of_headers=%d   \n",i,num_of_headers);
    priv->num_of_packets = stream_read_dword(demuxer->stream);
    stream_skip(demuxer->stream, 4); /* next data header */

    mp_msg(MSGT_DEMUX,MSGL_V,"Packets in file: %d\n", priv->num_of_packets);

    if (priv->num_of_packets == 0)
	priv->num_of_packets = -10;
  } else {
        priv->audio_curpos = priv->str_data_offset[demuxer->audio->id] + 18;
        stream_seek(demuxer->stream, priv->str_data_offset[demuxer->audio->id]+10);
        priv->a_num_of_packets = stream_read_dword(demuxer->stream);
        priv->video_curpos = priv->str_data_offset[demuxer->video->id] + 18;
        stream_seek(demuxer->stream, priv->str_data_offset[demuxer->video->id]+10);
        priv->v_num_of_packets = stream_read_dword(demuxer->stream);
        priv->stream_switch = 1;
        /* Index required for multirate playback, force building if it's not there */
        /* but respect user request to force index regeneration */
        if (index_mode == -1)
            index_mode = 1;
    }


    priv->audio_need_keyframe = 0;
    priv->video_after_seek = 0;

    switch (index_mode){
	case -1: // untouched
	    if ((demuxer->stream->flags & MP_STREAM_SEEK) == MP_STREAM_SEEK &&
                priv->index_chunk_offset && parse_index_chunk(demuxer))
	    {
		demuxer->seekable = 1;
	    }
	    break;
	case 1: // use (generate index)
	    if (priv->index_chunk_offset && parse_index_chunk(demuxer))
	    {
		demuxer->seekable = 1;
	    } else {
		generate_index(demuxer);
		demuxer->seekable = 1;
	    }
	    break;
	case 2: // force generating index
	    generate_index(demuxer);
	    demuxer->seekable = 1;
	    break;
	default: // do nothing
    	    break;
    }

    // detect streams:
    if(demuxer->video->id==-1 && v_streams>0){
	// find the valid video stream:
	ds_fill_buffer(demuxer->video);
    }
    if(demuxer->audio->id==-1 && a_streams>0){
	// find the valid audio stream:
	if(!ds_fill_buffer(demuxer->audio)){
          mp_msg(MSGT_DEMUXER,MSGL_INFO,"RM: " MSGTR_MissingAudioStream);
	}
    }
    if(demuxer->video->id==-1 && v_streams>0){
	// try video once more in case there were too many audio packets first.
	demuxer->video->eof = 0;
	demuxer->video->fill_count = 0;
	if(!ds_fill_buffer(demuxer->video)){
          mp_msg(MSGT_DEMUXER,MSGL_INFO,"RM: " MSGTR_MissingVideoStream);
	}
    }
    if(demuxer->video->id==-1 && v_streams>0){
        // worst case just select the first
        int i;
        for (i = 0; i < MAX_V_STREAMS; i++)
            if (demuxer->v_streams[i]) {
                demuxer->video->id = i;
                demuxer->video->sh = demuxer->v_streams[i];
            }
    }
    if(demuxer->audio->id==-1 && a_streams>0){
        // worst case just select the first
        int i;
        for (i = 0; i < MAX_A_STREAMS; i++)
            if (demuxer->a_streams[i]) {
                demuxer->audio->id = i;
                demuxer->audio->sh = demuxer->a_streams[i];
            }
    }

    if(demuxer->video->sh){
	sh_video_t *sh=demuxer->video->sh;
	mp_msg(MSGT_DEMUX,MSGL_V,"VIDEO:  %.4s [%08X,%08X]  %dx%d  (aspect %4.2f)  %4.2f fps\n",
	    (char *)&sh->format,((unsigned int*)(sh->bih+1))[1],((unsigned int*)(sh->bih+1))[0],
	    sh->disp_w,sh->disp_h,sh->aspect,sh->fps);
    }

    if(demuxer->audio->sh){
	sh_audio_t *sh=demuxer->audio->sh;
	mp_msg(MSGT_DEMUX,MSGL_V,"AUDIO:  %.4s [%08X]\n",
	    (char *)&sh->format,sh->format);
    }

    return demuxer;
}

static void demux_close_real(demuxer_t *demuxer)
{
    int i;
    real_priv_t* priv = demuxer->priv;

    if (priv){
    	for(i=0; i<MAX_STREAMS; i++)
	    free33(priv->index_table[i]);
	free33(priv->audio_buf);
	free33(priv->audio_timestamp);
	free33(priv);
    }

    return;
}

/* please upload RV10 samples WITH INDEX CHUNK */
static void demux_seek_real(demuxer_t *demuxer, float rel_seek_secs, float audio_delay, int flags)
{
    real_priv_t *priv = demuxer->priv;
    demux_stream_t *d_audio = demuxer->audio;
    demux_stream_t *d_video = demuxer->video;
    sh_audio_t *sh_audio = d_audio->sh;
    sh_video_t *sh_video = d_video->sh;
    int vid = d_video->id, aid = d_audio->id;
    int next_offset = 0;
    int64_t target_timestamp = 0;
    int streams = 0;
    int retried = 0;


    if (priv->is_mlti && (unsigned)vid < MAX_STREAMS)
	vid = priv->mp2rm_streamid[d_video->id];
    if (priv->is_mlti && (unsigned)aid < MAX_STREAMS)
	aid = priv->mp2rm_streamid[d_audio->id];

    if (sh_video && (unsigned)vid < MAX_STREAMS && priv->index_table_size[vid])
	streams |= 1;
    if (sh_audio && (unsigned)aid < MAX_STREAMS && priv->index_table_size[aid])
	streams |= 2;

//    printf("streams: %d\n", streams);

    if (!streams)
	return;

    if (flags & SEEK_ABSOLUTE)
	priv->current_apacket = priv->current_vpacket = 0;
    if (flags & SEEK_FACTOR)
        rel_seek_secs *= priv->duration;

    if ((streams & 1) && priv->current_vpacket >= priv->index_table_size[vid])
	priv->current_vpacket = priv->index_table_size[vid] - 1;
    if ((streams & 2) && priv->current_apacket >= priv->index_table_size[aid])
	priv->current_apacket = priv->index_table_size[aid] - 1;

//    if (index_mode == 1 || index_mode == 2) {
    	if (streams & 1) {// use the video index if we have one
            target_timestamp = priv->index_table[vid][priv->current_vpacket].timestamp;
            target_timestamp += rel_seek_secs * 1000;
	    if (rel_seek_secs > 0)
	    	while (priv->index_table[vid][priv->current_vpacket].timestamp < target_timestamp){
	    		priv->current_vpacket += 1;
	    		if (priv->current_vpacket >= priv->index_table_size[vid]) {
	    			priv->current_vpacket = priv->index_table_size[vid] - 1;
				if (!retried) {
					stream_seek(demuxer->stream, priv->index_table[vid][priv->current_vpacket].offset);
					add_index_segment(demuxer, vid, target_timestamp);
					retried = 1;
				}
				else
	    				break;
	    		}
	    	}
	    else if (rel_seek_secs < 0) {
	    	while (priv->index_table[vid][priv->current_vpacket].timestamp > target_timestamp){
	    		priv->current_vpacket -= 1;
	    		if (priv->current_vpacket < 0) {
	    			priv->current_vpacket = 0;
	    			break;
	    		}
	    	}
	    }
	    priv->video_curpos = priv->index_table[vid][priv->current_vpacket].offset;
	    priv->audio_need_keyframe = !priv->is_multirate;
	    priv->video_after_seek = 1;
        } else {
            target_timestamp = priv->index_table[aid][priv->current_apacket].timestamp;
            target_timestamp += rel_seek_secs * 1000;
        }
    	if (streams & 2) {
	    if (rel_seek_secs > 0)
	    	while (priv->index_table[aid][priv->current_apacket].timestamp < target_timestamp){
	    		priv->current_apacket += 1;
	    		if (priv->current_apacket >= priv->index_table_size[aid]) {
	    			priv->current_apacket = priv->index_table_size[aid] - 1;
	    			break;
	    		}
	    	}
	    else if (rel_seek_secs < 0)
	    	while (priv->index_table[aid][priv->current_apacket].timestamp > target_timestamp){
	    		priv->current_apacket -= 1;
	    		if (priv->current_apacket < 0) {
	    			priv->current_apacket = 0;
	    			break;
	    		}
	    	}
	    priv->audio_curpos = priv->index_table[aid][priv->current_apacket].offset;
        }
//    }
    next_offset = streams & 1 ? priv->video_curpos : priv->audio_curpos;
//    printf("seek: pos: %d, current packets: a: %d, v: %d\n",
//	next_offset, priv->current_apacket, priv->current_vpacket);
    if (next_offset)
        stream_seek(demuxer->stream, next_offset);

    demux_real_fill_buffer(demuxer, NULL);
}

static int demux_real_control(demuxer_t *demuxer, int cmd, void *arg)
{
    real_priv_t *priv = demuxer->priv;
    unsigned int lastpts = priv->v_pts ? priv->v_pts : priv->a_pts;

    switch (cmd) {
        case DEMUXER_CTRL_GET_TIME_LENGTH:
	    if (priv->duration == 0)
	        return DEMUXER_CTRL_DONTKNOW;

	    *((double *)arg) = (double)priv->duration;
	    return DEMUXER_CTRL_OK;

	case DEMUXER_CTRL_GET_PERCENT_POS:
	    if (priv->duration == 0)
	        return DEMUXER_CTRL_DONTKNOW;

	    *((int *)arg) = (int)(100 * lastpts / priv->duration);
	    return DEMUXER_CTRL_OK;

	default:
	    return DEMUXER_CTRL_NOTIMPL;
    }
}


const demuxer_desc_t demuxer_desc_real = {
  "Realmedia demuxer",
  "real",
  "REAL",
  "Alex Beregszasi, Florian Schneider, A'rpi, Roberto Togni",
  "handles new .RMF files",
  DEMUXER_TYPE_REAL,
  1, // safe autodetect
  real_check_file,
  demux_real_fill_buffer,
  demux_open_real,
  demux_close_real,
  demux_seek_real,
  demux_real_control
};
