/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MTLZ_AVFILTER_H_H__
#define __MTLZ_AVFILTER_H_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "mtlz_avfilter_stream.h"
#define INVALID_ENCRYPTED_FLAG          (0xFFFFffff)

typedef struct MTAVSubsampleEncryptionInfo {
    /** The number of bytes that are clear. */
    unsigned short *bytes_of_clear_data;

    /**
     * The number of bytes that are protected.  If using pattern encryption,
     * the pattern applies to only the protected bytes; if not using pattern
     * encryption, all these bytes are encrypted.
     */
    unsigned int *bytes_of_protected_data;
} MTAVSubsampleEncryptionInfo;

/**
 * This describes encryption info for a packet.  This contains frame-specific
 * info for how to decrypt the packet before passing it to the decoder.
 *
 * The size of this struct is not part of the public ABI.
 */
typedef struct MTAVEncryptionInfo {
    /** The fourcc encryption scheme, in big-endian byte order. */
    unsigned int scheme;

    /**
     * Only used for pattern encryption.  This is the number of 16-byte blocks
     * that are encrypted.
     */
    unsigned int crypt_byte_block;

    /**
     * Only used for pattern encryption.  This is the number of 16-byte blocks
     * that are clear.
     */
    unsigned int skip_byte_block;

    /**
     * The ID of the key used to encrypt the packet.  This should always be
     * 16 bytes long, but may be changed in the future.
     */
    unsigned char *key_id;
    unsigned int key_id_size;

    /**
     * The initialization vector.  This may hMTAVe been zero-filled to be the
     * correct block size.  This should always be 16 bytes long, but may be
     * changed in the future.
     */
    unsigned char *iv;
    unsigned int iv_size;

    /**
     * An array of subsample encryption info specifying how parts of the sample
     * are encrypted.  If there are no subsamples, then the whole sample is
     * encrypted.
     */
    MTAVSubsampleEncryptionInfo subsamples;
    unsigned int subsample_count;
} MTAVEncryptionInfo;

/**
 * This describes info used to initialize an encryption key system.
 *
 * The size of this struct is not part of the public ABI.
 */
typedef struct MTAVEncryptionInitInfo {
    /**
     * A unique identifier for the key system this is for, can be NULL if it
     * is not known.  This should be 16 hex bytes or 36bytes ascii data
     */
    unsigned char *system_id;
    unsigned int system_id_size;

    /**
     * An array of key IDs this initialization data is for.  All IDs are the
     * same length.  Can be NULL if there are no known key IDs.
     */
    unsigned char **key_ids;
    /** The number of key IDs. */
    unsigned int num_key_ids;
    /**
     * The number of bytes in each key ID.  This should always be 16, but may
     * change in the future.
     */
    unsigned int key_id_size;

    /**
     * Our system specific initialization data.  This data is copied directly
     * from the pssh or ContentProtection
     */
    unsigned char *header_data;
    unsigned int header_data_size;
} MTAVEncryptionInitInfo;

typedef struct _MTAVFilterInitInfo {
    union {
        MTAVSAudioInfo audio;
        MTAVSVideoInfo video;
    } codec;
    MTAVEncryptionInitInfo    encryption;
} MTAVFilterInitInfo;

typedef struct _MTAVFilter {
    unsigned int codec_id;
    unsigned int encrypted;

    void *opaque;
    MTAVSF_MSG_FUNC   msg_cb;
    MTAVStreamFilter *bsf;
    MTAVFilterInitInfo init_info;
} MTAVFilter;

/* Create stream filter MTAVStreamFilter
 * For clear stream:
 *     1. Create stream filter when no filter exist.
 *     2. Recreate stream filter when codec id change.
 * For ecnrypted stream: *
 *     1. Create stream filter when no filter exist.
 *     2. No need recreate stream filter if stream has Clear Content
 *     3. Not support codec id change for ecnrypted stream
 *        Reason:
 *            A. Once init decryption system, all DDR memory will be controlled under scramble mode.
 *               memory can be accessed by the DMA only.
 *            B. All Clear data pushed before init decryption system will be scrambled, so these data
 *               will be changed and will not availabe any more.
 *               Thus, we shoud init decryption system for the first push and use DMA for memory copy.
 *            C. Deinit decryption system for the last push.
 */
int mtav_filter_create(MTAVFilter *mtav,
    unsigned int codec_id, unsigned int encrypted);
/**
 * Init the filter use the init_info,
 * which maybe differ from each other due to codec difference,
 * maybe MTAVStreamFilterVideoInfo, or MTAVStreamFilterAudioInfo, or MTAVEncryptionInitInfo
 *
 * @param MTAV       The instance of MTAVFilter.
 * @param init_info  The initialization info for current stream.
 *
 * @return 0 when successful, or -1 on error.
 */
int mtav_filter_init(MTAVFilter *mtav, void *init_info);
/**
 * Get the pre-filtered stream info from the filter
 *
 * if we are clear stream(enc NULL) :
 *     parse part of the data and then output data bytes after filter.
 * if we are encrypted stream(enc Not NULL):
 *     decrypted the stream and the parse the part of the data
 * Caller use the output info to decide what to do next, such as allocating a/vdec buffer etc.
 *
 * @param mtav       The instance of MTAVFilter.
 * @param para       The encryption info with type MTAVEncryptionInfo of current data and
 *                   keyframe info with type unsigned int for vc1 and dump extra, 1: keyframe, 0: not keyframe
 * @param data       The data to be filtered, it can be set NULL to generate data_size bytes 0 output.
 * @param pts        The pts of data stream to be filtered
 * @param data_size  The bytes of data to be filtered
 * @param info       The outuput info for filter
 *
 * @return 0 when successful, or -1 on error.
 */
int mtav_filter_get_info(MTAVFilter *mtav, MTAVStreamPara *para,
    unsigned char *data, long long pts, unsigned int data_size, void *info);
/**
 * Filter data stream kept in filter buffer.
 * if the out_buf_size smaller than filtered data size, you can
 * call several times until all data output
 *
 * @param mtav            The instance of MTAVFilter.
 * @param out_buf         The data buffer for keeping filtered data.
 * @param out_buf_size    The bytes of out_buf.
 *
 * @return 0 when successful, or -1 on error.
 */
int mtav_filter_filter(MTAVFilter *mtav,
    unsigned char *out_buf, unsigned int out_buf_size);

/**
 * Flush filter buffer of reset filter state
 *
 * Some codec need more header info such as vp9 after seek
 *
 * @param mtav          The instance of MTAVFilter.
 *
 * @return 0 when successful, or -1 on error.
 */
int mtav_filter_flush(MTAVFilter *mtav);

void mtav_filter_release(MTAVFilter *mtav);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTLZ_AVFILTER_H_H__ */