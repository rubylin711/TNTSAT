/*
 * Copyright (c) 2011 Justin Ruggles
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVFORMAT_DECRYPTION_COMP_H
#define AVFORMAT_DECRYPTION_COMP_H

#include "avformat.h"
#include <libxml/parser.h>
#include "libavutil/encryption_info.h"

/* Don't change below enum order */
enum DRM_SYSTEM_TYPE {
    DRM_SYSTEM_TYPE_NONE = 0,
    DRM_SYSTEM_TYPE_PLAYREADY,
    DRM_SYSTEM_TYPE_WIDEVINE,
    DRM_SYSTEM_TYPE_CHINADRM,
    DRM_SYSTEM_TYPE_VERIMATRIX,
};

typedef struct {
    void *key;
    void *iv;
    int type;
} TSScramEncInfo;

typedef struct ChinaDrmDescr {
    int video_format;
    unsigned int  video_encryption_method;
    int audio_format;
    unsigned int  audio_encryption_method;
    unsigned int  nb_drm_data_bytes;
    unsigned char drm_data_bytes[256]; /* max 253 bytes */
} ChinaDrmDescr;

typedef int (*TransportDesDecCallback) (void *ctx,
    const void *info, const unsigned char *in, unsigned char *out, int len);

int ff_hls_parse_ext_x_key(unsigned char *uri,
    unsigned char *keyformat, AVEncryptionInitInfo **encryption_info);
int ff_dash_parse_content_protection(
    xmlNodePtr node, AVEncryptionInitInfo **encryption_info);

int ff_dash_clone_content_protection(
    AVEncryptionInitInfo **dst, AVEncryptionInitInfo *src);
void ff_dash_free_content_protection(
    AVEncryptionInitInfo **encryption_info);

int ff_decrypt_comp_get_drm_system_type(AVEncryptionInitInfo *info);
int ff_decrypt_comp_merge_init_info(
    AVStream *st, AVEncryptionInitInfo *info);
/* append init info for the first packet */
int ff_decrypt_comp_append_init_info(AVStream *st, AVPacket *pkt);

int ff_transport_parse_china_drm_descriptor(
    AVFormatContext *s, const uint8_t *buf, unsigned int size, ChinaDrmDescr *descr);

int ff_transport_get_desdec_callback(
    AVFormatContext *s, TransportDesDecCallback *cb);
int ff_transport_desdec(AVFormatContext *s, TSScramEncInfo *info,
    TransportDesDecCallback cb, const unsigned char *in, unsigned char *out, int len);

#endif /* AVFORMAT_DECRYPTION_COMP_H */
