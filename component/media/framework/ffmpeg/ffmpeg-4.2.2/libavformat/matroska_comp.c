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

#include "matroska_comp.h"
#include "avio.h"
#include "internal.h"
#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"

/* WebM spec */
#define MATROSKA_BLOCK_ENCRYPTED      (0x01)
#define MATROSKA_BLOCK_PARTITIONED    (0x02)
#define MIN_ENCRYPTED_BLOCK_BYTES     (IV_SIZE_WEBM_DEFAULT + 1)

/* return used bytes */
static int parse_matroska_subsample(
    AVEncryptionInfo *enc, const uint8_t * const pkt_data, const int pkt_size)
{
    uint32_t offset          = 0;
    uint32_t offset_prev     = 0;
    uint32_t encrypted_bytes = 0;
    uint16_t clear_bytes     = 0;
    uint32_t subsample_count = 0;
    const uint8_t *buffer = pkt_data;
	const uint8_t *buffer_end = pkt_data + pkt_size;

    if (buffer >= buffer_end) {
        av_log(NULL, AV_LOG_ERROR, "Error reading the partition number\n");
        return AVERROR_INVALIDDATA;
    }
    /* Read the number of partitions (1 byte) */
    uint8_t nb_part = *buffer++;
    if (0 == nb_part) {
        av_log(NULL, AV_LOG_ERROR, "Partitioned, but the subsample number equal to zero\n");
        return AVERROR_INVALIDDATA;
    }

    subsample_count = (nb_part + 2) >> 1;
    /* WebM Spec:
     *
     * 4.6 Subsample Encrypted Block Format
     *
     * The Subsample Encrypted Block format extends the Full-sample format by setting a "partitioned" (P) bit in the Signal Byte.
     * If this bit is set, the EncryptedBlock header shall include an
     * 8-bit integer indicating the number of sample partitions (dividers between clear/encrypted sections),
     * and a series of 32-bit integers in big-endian encoding indicating the byte offsets of such partitions.
     *
     *  0                   1                   2                   3
     *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |  Signal Byte  |                                               |
     * +-+-+-+-+-+-+-+-+             IV                                |
     * |                                                               |
     * |               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |               | num_partition |     Partition 0 offset ->     |
     * |-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
     * |     -> Partition 0 offset     |              ...              |
     * |-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
     * |             ...               |     Partition n-1 offset ->   |
     * |-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
     * |     -> Partition n-1 offset   |                               |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
     * |                    Clear/encrypted sample data                |
     * |                                                               |
     * |                                                               |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     * 4.6.1 SAMPLE PARTITIONS
     *
     * The samples shall be partitioned into alternating clear and encrypted sections,
     * always starting with a clear section.
     * Generally for n clear/encrypted sections there shall be n-1 partition offsets.
     * However, if it is required that the first section be encrypted, then the first partition shall be at byte offset 0
     * (indicating a zero-size clear section), and there shall be n partition offsets.
     * Please refer to the "Sample Encryption" description of the "Common Encryption"
     * section of the VP Codec ISO Media File Format Binding Specification for more
     * detail on how subsample encryption is implemented.
     */
     if (enc->subsample_count != subsample_count) {
        av_freep(enc->subsamples);
        enc->subsamples =
        av_mallocz_array(subsample_count, sizeof(AVSubsampleEncryptionInfo));
        if (!enc->subsamples) {
            av_log(NULL, AV_LOG_ERROR, "Error alloc matroska subsamples\n");
            enc->subsample_count = 0;
            return AVERROR_INVALIDDATA;
        }
        enc->subsample_count = subsample_count;
    }
    subsample_count = 0;
    for (int i = 0; i <= nb_part; i++) {
        offset_prev = offset;
        if (i == nb_part) {
            offset = buffer_end - buffer;
        } else {
            if (buffer + sizeof(uint32_t) > buffer_end) {
                av_log(NULL, AV_LOG_ERROR, "Error reading the partition offset\n");
                return AVERROR_INVALIDDATA;
            }
            offset = AV_RB32(buffer);
            buffer += sizeof(uint32_t);
        }

        if (offset < offset_prev) {
            av_log(NULL, AV_LOG_ERROR, "Partition offsets should not decrease");
            return AVERROR_INVALIDDATA;
        }

        if (i % 2 == 0) {
            if ((offset - offset_prev) & 0xFFFF0000) {
               av_log(NULL, AV_LOG_ERROR, "The Clear Partition exceed 64KB in encrypted subsample format");
               return AVERROR_INVALIDDATA;
            }
            /* We set the Clear partition size in 16 bits, in order to
             * follow the same format of the box PSSH in CENC spec */
            clear_bytes = offset - offset_prev;
            if (i == nb_part) {
                encrypted_bytes = 0;
            }
        } else {
            encrypted_bytes = offset - offset_prev;
        }

        if ((i % 2 == 1) || (i == nb_part)) {
            if (clear_bytes == 0 && encrypted_bytes == 0) {
               av_log(NULL, AV_LOG_ERROR, "Found 2 partitions with the same offsets.");
               return AVERROR_INVALIDDATA;
            }
            enc->subsamples[subsample_count].bytes_of_clear_data = clear_bytes;
            enc->subsamples[subsample_count].bytes_of_protected_data = encrypted_bytes;
            subsample_count++;
        }
    }
    return (int)(buffer - pkt_data);
}
/* This function parses the protection info of Block/SimpleBlock and extracts the
 * IV and partitioning format (subsample) information.
 * Set those parsed information into protection info structure @info_protect which
 * will be added in protection metadata of the Gstbuffer.
 * The subsamples format follows the same pssh box format in Common Encryption spec:
 * subsample number + clear subsample size (16bit bigendian) | encrypted subsample size (32bit bigendian) | ...
 * @encrypted is an output argument: TRUE if the current Block/SimpleBlock is encrypted else FALSE
 */
static int parse_matroska_protection_meta(AVEncryptionInfo *enc,
    const uint8_t * const pkt_data, int pkt_size, int *encrypted)
{
    int     used_bytes  = 1;
    uint8_t signal_byte = 0;
	const uint8_t *buffer = pkt_data;

    /* WebM spec:
     * 4.7 Signal Byte Format
     *  0 1 2 3 4 5 6 7
     * +-+-+-+-+-+-+-+-+
     * |X|   RSV   |P|E|
     * +-+-+-+-+-+-+-+-+
     *
     * Extension bit (X)
     * If set, another signal byte will follow this byte. Reserved for future expansion (currently MUST be set to 0).
     * RSV bits (RSV)
     * Bits reserved for future use. MUST be set to 0 and MUST be ignored.
     * Encrypted bit (E)
     * If set, the Block MUST contain an IV immediately followed by an encrypted frame. If not set, the Block MUST NOT include an IV and the frame MUST be unencrypted. The unencrypted frame MUST immediately follow the Signal Byte.
     * Partitioned bit (P)
     * Used to indicate that the sample has subsample partitions. If set, the IV will be followed by a num_partitions byte, and num_partitions * 32-bit partition offsets. This bit can only be set if the E bit is also set.
     */
    pkt_size--;
    signal_byte = (uint8_t) (*buffer++);
    /* Unencrypted buffer */
    if (!(signal_byte & MATROSKA_BLOCK_ENCRYPTED)) {
        goto done;
    }

    if (pkt_size < MIN_ENCRYPTED_BLOCK_BYTES) {
        av_log(NULL, AV_LOG_ERROR, "Error reading the IV data\n");
        return AVERROR_INVALIDDATA;
    }

    if (IV_SIZE_WEBM_DEFAULT != enc->iv_size) {
        enc->iv = av_realloc(enc->iv, enc->iv_size);
        if (!enc->iv) {
            enc->iv_size = 0;
            av_log(NULL, AV_LOG_ERROR, "Error alloc matroska protection info\n");
            return AVERROR(ENOMEM);
        }
    }
    /* Encrypted buffer */
    *encrypted = 1;
    memcpy(enc->iv, buffer, enc->iv_size);
	used_bytes += enc->iv_size;
    buffer     += enc->iv_size;
    pkt_size   -= enc->iv_size;
    /* Partitioned in subsample */
    if (signal_byte & MATROSKA_BLOCK_PARTITIONED) {
        int partion_bytes = parse_matroska_subsample(enc, buffer, pkt_size);
        if (partion_bytes < 0) {
            goto release_err;
        }
        used_bytes += partion_bytes;
    }

done:
    return used_bytes;
release_err:
    return AVERROR_INVALIDDATA;
}

int ff_matroska_parse_protection_meta(
    MatroskaProtection *protection, uint8_t **pkt_data, int *pkt_size, int *encrypted)
{
	av_assert1((!protection));
	av_assert1((!pkt_data || !*pkt_data || !pkt_size || !*pkt_size));
	av_assert1((!encrypted));

    *encrypted = 0;
    AVEncryptionInfo *info = protection->info;
    int ret = parse_matroska_protection_meta(info, *pkt_data, *pkt_size, encrypted);
    if (ret < 0) {
        return ret;
    }

    int new_size = *pkt_size - ret;
    uint8_t *new_data =
        av_malloc(new_size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!new_data) {
        av_encryption_info_free(info);
        return AVERROR(ENOMEM);
    }

    memcpy(new_data, *pkt_data + ret, new_size);
    *pkt_data = new_data;
    *pkt_size = new_size;
    return 0;
}

static void inline release_matroska_protection(MatroskaProtection *protection)
{
    av_encryption_info_free(protection->info);
    av_encryption_init_info_free(protection->init_info);
}

MatroskaProtection *ff_matroska_create_protection(
    uint32_t subsample_count, uint32_t key_id_size, uint32_t iv_size)
{
    MatroskaProtection *protection =
        av_mallocz(sizeof(MatroskaProtection));
    if (!protection) {
        return NULL;
    }
    protection->info = av_encryption_info_alloc(subsample_count, key_id_size, iv_size);
    if (!protection->info) {
        av_freep(protection);
        return NULL;
    }
    protection->info->scheme = ENCRYPTION_SCHEME_WEBM_ENC;
    return protection;
}

int ff_matroska_release_protection(MatroskaProtection **protection)
{
    if (!protection || !*protection) {
        return 0;
    }
    release_matroska_protection(*protection);
    av_freep(protection);

    return 0;
}
