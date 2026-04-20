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

#ifndef AVFORMAT_MATROSKA_COMP_H
#define AVFORMAT_MATROSKA_COMP_H

#include "avformat.h"
#include "libavutil/encryption_info.h"

#define WEBM_ENCRYPT_ALGO_AES         (5)
#define KEY_ID_SIZE_DEFAULT           (16)
#define IV_SIZE_WEBM_DEFAULT          (8)

typedef struct _MatroskaProtection {
    /* reserve for future */
    AVEncryptionInitInfo *init_info;
    AVEncryptionInfo *info;
} MatroskaProtection;

int ff_matroska_parse_protection_meta(
    MatroskaProtection *protection, uint8_t **pkt_data, int *pkt_size, int *encrypted);
MatroskaProtection *ff_matroska_create_protection(
    uint32_t subsample_count, uint32_t key_id_size, uint32_t iv_size);
int ff_matroska_release_protection(MatroskaProtection **protection);

#endif /* AVFORMAT_MATROSKA_COMP_H */
