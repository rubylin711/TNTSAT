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

#include "decryption_comp.h"
#include "avio.h"
#include "libavutil/base64.h"
#include "libavutil/avstring.h"

// #define TRANSPORT_DESDEC_DEBUG
#define MAX_ENCRYPTION_INIT_INFO_NUM                      1024
/* ID from ContentProtection  */
#define CLEARKEY_CPRO_SYSTEM_ID             "1077efec-c0b2-4d02-ace3-3c1e52e2fb4b"
#define WIDEVINE_CPRO_SYSTEM_ID             "edef8ba9-79d6-4ace-a3c8-27dcd51d21ed"
#define PLAYREADY_CPRO_SYSTEM_ID            "9a04f079-9840-4286-ab92-e65be0885f95"
#define CHINADRM_CPRO_SYSTEM_ID             "3d5e6d35-9b9a-41e8-b843-dd3c6e72c42c"
#define VERIMATRIX_CPRO_SYSTEM_ID           "9a27dd82-fde2-4725-8cbc-4234aa06ec09"
#define CONTENT_PROTECTION_SYSTEM_ID_SIZE   (strlen(CLEARKEY_CPRO_SYSTEM_ID))

#define SYSTEM_ID_SIZE_DEFAULT                (16)
static const unsigned char CLEARKEY_SYSTEM_ID[SYSTEM_ID_SIZE_DEFAULT] = {
    0x10, 0x77, 0xef, 0xec, 0xc0, 0xb2, 0x4d, 0x02, 0xac, 0xe3, 0x3c, 0x1e, 0x52, 0xe2, 0xfb, 0x4b
};
static const unsigned char PLAYREADY_SYSTEM_ID[SYSTEM_ID_SIZE_DEFAULT] = {
    0x9a, 0x04, 0xf0, 0x79, 0x98, 0x40, 0x42, 0x86, 0xab, 0x92, 0xe6, 0x5b, 0xe0, 0x88, 0x5f, 0x95
};
static const unsigned char WIDEVINE_SYSTEM_ID[SYSTEM_ID_SIZE_DEFAULT] = {
    0xed, 0xef, 0x8b, 0xa9, 0x79, 0xd6, 0x4a, 0xce, 0xa3, 0xc8, 0x27, 0xdc, 0xd5, 0x1d, 0x21, 0xed
};
static const unsigned char CHINADRM_SYSTEM_ID[SYSTEM_ID_SIZE_DEFAULT] = {
    0x3d, 0x5e, 0x6d, 0x35, 0x9b, 0x9a, 0x41, 0xe8, 0xb8, 0x43, 0xdd, 0x3c, 0x6e, 0x72, 0xc4, 0x2c
};
/* Verimatrix Conditional Access System:VCAS */
static const unsigned char VCAS_SYSTEM_ID[SYSTEM_ID_SIZE_DEFAULT] = {
	0x9a, 0x27, 0xdd, 0x82, 0xfd, 0xe2, 0x47, 0x25, 0x8c, 0xbc, 0x42, 0x34, 0xaa, 0x06, 0xec, 0x09
};

static int hexchar2int(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static xmlNodePtr find_child_node_by_name(xmlNodePtr rootnode, const char *nodename)
{
    xmlNodePtr node = rootnode;
    if (!node) {
        return NULL;
    }

    node = xmlFirstElementChild(node);
    while (node) {
        if (!av_strcasecmp(node->name, nodename)) {
            return node;
        }
        node = xmlNextElementSibling(node);
    }
    return NULL;
}

static void parse_encryption_info_system_id(char *uri, AVEncryptionInitInfo *info)
{
    unsigned char *system_id =
        av_strnstr(uri, "uuid:", CONTENT_PROTECTION_SYSTEM_ID_SIZE);
    if (!system_id) {
        return;
    }
    system_id += strlen("uuid:");
    if (!av_strncasecmp(system_id, PLAYREADY_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE)) {
        memcpy(info->system_id, PLAYREADY_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE);
    } else if (!av_strncasecmp(system_id, WIDEVINE_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE)) {
        memcpy(info->system_id, WIDEVINE_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE);
    } else if (!av_strncasecmp(system_id, CHINADRM_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE)) {
        memcpy(info->system_id, CHINADRM_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE);
	} else if (!av_strncasecmp(system_id, VERIMATRIX_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE)) {
		memcpy(info->system_id, VERIMATRIX_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE);
	} else {
        av_log(NULL, AV_LOG_WARNING, "Not support system id:%s\n", uri);
    }
}

static void parse_content_protection_scheme_id_uri(
    xmlNodePtr node, AVEncryptionInitInfo *info)
{
    char *scheme_id_uri =
        xmlGetProp(node, "schemeIdUri");

    if (!scheme_id_uri) {
        return;
    }
    parse_encryption_info_system_id(scheme_id_uri, info);
    xmlFree(scheme_id_uri);
}

static void  parse_content_protection_default_kid(
    xmlNodePtr node, AVEncryptionInitInfo *info)
{
    unsigned int i = 0;
    unsigned char *default_kid =
        xmlGetProp(node, "default_KID");

    if (!default_kid) {
        return;
    }
    unsigned int default_kid_len = strlen(default_kid);
    unsigned char *ptr = default_kid;
    while ((ptr = strstr(ptr, "-"))) {
        memmove(ptr, ptr + 1, strlen(ptr));
    }

    if (strlen(default_kid) < (info->key_id_size << 1)) {
        goto finish;
    }

    ptr = default_kid;
    for (i = 0; i < info->key_id_size; i++) {
        unsigned int tmp[2] = {hexchar2int(ptr[i * 2]), hexchar2int(ptr[i * 2 + 1])};
        ptr[i] = (unsigned char) ((tmp[0] << 4 | tmp[1]));
    }

    for (i = 0; i < info->num_key_ids; i++) {
        if (!av_strncasecmp(default_kid,
            info->key_ids[i], info->key_id_size)) {
            break;
        }
    }

    if (i == info->num_key_ids) {
        unsigned int alloc_size = 0;
        uint8_t **key_ids = av_fast_realloc(info->key_ids, &alloc_size,
                  (info->num_key_ids + 1) * sizeof(*key_ids));
        if (!key_ids) {
            av_log(NULL, AV_LOG_ERROR, "No memory for dash kids\n");
            goto finish;
        }
        info->key_ids = key_ids;
        info->key_ids[i] = av_mallocz(info->key_id_size);
        if (!info->key_ids[i]) {
            av_log(NULL, AV_LOG_ERROR, "No memory for dash kid\n");
            goto finish;
        }
        memcpy(info->key_ids[i], default_kid, info->key_id_size);
        info->num_key_ids = i + 1;
    }
finish:
    xmlFree(default_kid);
}

static int parse_content_protection_header_info(
    xmlNodePtr node, AVEncryptionInitInfo *info)
{
    int ret = 0;
    xmlNodePtr child = NULL;
    if ((child = find_child_node_by_name(node, "pro"))) {
    } else if ((child = find_child_node_by_name(node, "pssh"))) {
    } else {
        return 0;
    }

    unsigned char *content = xmlNodeGetContent(child);
    unsigned int  content_len = strlen(content);
    /* select information with more data */
    if (info->header_data_size < content_len) {
        info->header_data =
            av_realloc(info->header_data, content_len);
        if (!info->header_data) {
            ret = AVERROR(ENOMEM);
            info->header_data_size = 0;
            goto finish;
        }
        //info->header_data[content_len] = '\0';
        memcpy(info->header_data, content, content_len);
        info->header_data_size = content_len;
    }

finish:
    xmlFree(content);
    return ret;
}

static int check_encryption_init_info(AVEncryptionInitInfo *info)
{
    if (!info->system_id || !info->system_id_size) {
        return 0;
    }

    if (!info->num_key_ids || !info->key_id_size || !info->key_ids || !info->key_ids[0]) {
        return 0;
    }

    if (!info->header_data || !info->header_data_size) {
        return 0;
    }
    return 1;
}

static int get_system_type(AVEncryptionInitInfo *info)
{
    if (SYSTEM_ID_SIZE_DEFAULT == info->system_id_size) {
        if (!memcmp(info->system_id, PLAYREADY_SYSTEM_ID, SYSTEM_ID_SIZE_DEFAULT)) {
            return DRM_SYSTEM_TYPE_PLAYREADY;
        } else if (!memcmp(info->system_id, WIDEVINE_SYSTEM_ID, SYSTEM_ID_SIZE_DEFAULT)) {
            return DRM_SYSTEM_TYPE_WIDEVINE;
        } else if (!memcmp(info->system_id, CHINADRM_SYSTEM_ID, SYSTEM_ID_SIZE_DEFAULT)) {
            return DRM_SYSTEM_TYPE_CHINADRM;
        } else if (!memcmp(info->system_id, VCAS_SYSTEM_ID, SYSTEM_ID_SIZE_DEFAULT)) {
            return DRM_SYSTEM_TYPE_VERIMATRIX;
        }
    }

    if ((CONTENT_PROTECTION_SYSTEM_ID_SIZE == info->system_id_size)) {
        if (!memcmp(info->system_id, PLAYREADY_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE)) {
            return DRM_SYSTEM_TYPE_PLAYREADY;
        } else if (!memcmp(info->system_id, WIDEVINE_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE)) {
            return DRM_SYSTEM_TYPE_WIDEVINE;
        } else if (!memcmp(info->system_id, CHINADRM_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE)) {
            return DRM_SYSTEM_TYPE_CHINADRM;
        } else if (!memcmp(info->system_id, VERIMATRIX_CPRO_SYSTEM_ID, CONTENT_PROTECTION_SYSTEM_ID_SIZE)) {
            return DRM_SYSTEM_TYPE_VERIMATRIX;
        }
    }
    return DRM_SYSTEM_TYPE_NONE;
}
/* return -1: a ahead, 1: b ahead, 0:not change */
static int compare_id_size(const void *a, const void *b) {
    AVEncryptionInitInfo *pa = (AVEncryptionInitInfo *)(*((intptr_t *) a));
    AVEncryptionInitInfo *pb = (AVEncryptionInitInfo *)(*((intptr_t *) b));

    /* id size bigger, info from hls/mpd list, move back */
    if (pa->system_id_size > pb->system_id_size) {
        return 1;
    } else if (pa->system_id_size < pb->system_id_size) {
        return -1;
    }

    return 0;
}

static int compare_by_system_type(const void *a, const void *b) {
    AVEncryptionInitInfo *pa = (AVEncryptionInitInfo *)(*((intptr_t *) a));
    AVEncryptionInitInfo *pb = (AVEncryptionInitInfo *)(*((intptr_t *) b));

    int typea = get_system_type(pa);
    int typeb = get_system_type(pb);

    /* move back */
    if (DRM_SYSTEM_TYPE_NONE == typea && DRM_SYSTEM_TYPE_NONE != typeb) {
        return 1;
    }
    /* keep, no change */
    if (DRM_SYSTEM_TYPE_NONE == typeb) {
        return 0;
    }

    /* typea typeb not none */
    return typea - typeb;
}

static int compare_by_init_length(const void *a, const void *b) {
    AVEncryptionInitInfo *pa = (AVEncryptionInitInfo *)(*((intptr_t *) a));
    AVEncryptionInitInfo *pb = (AVEncryptionInitInfo *)(*((intptr_t *) b));

    int hsize_a = (int) pa->header_data_size;
    int hsize_b = (int) pb->header_data_size;
    /* put longer ahead */
    return hsize_b - hsize_a;
}

static void do_select_by_type(
    AVEncryptionInitInfo **array, int array_num)
{
    if (array_num <= 1) {
        return;
    }

    int idx = 0, start = 0;
    /* put playread ahead */
    qsort(array, array_num, sizeof(AVEncryptionInitInfo *), compare_by_system_type);
    while (idx < array_num &&
        get_system_type(array[idx]) == DRM_SYSTEM_TYPE_PLAYREADY) {
        idx++;
    }

    start = idx;
    if (idx > 1) {
        /* put longer init data in playread ahead */
        qsort(array, idx, sizeof(AVEncryptionInitInfo *), compare_by_init_length);
    }

    while (idx < array_num &&
        get_system_type(array[idx]) == DRM_SYSTEM_TYPE_WIDEVINE) {
        idx++;
    }

    int num = idx - start;
    if (num > 1) {
        /* put longer init data in widevine ahead */
        qsort(&array[start], num, sizeof(AVEncryptionInitInfo *), compare_by_init_length);
    }
}

/* prefer priority :
 * 1 : system id size, smaller size from file not mpd or m3u8
 * 2 : long header size means comprehensive information
 */
static AVEncryptionInitInfo *do_select_by_field(AVEncryptionInitInfo *info)
{
    AVEncryptionInitInfo *curr, *array[MAX_ENCRYPTION_INIT_INFO_NUM] = {0};

    int inum = 0;
    for (curr = info; curr && inum < MAX_ENCRYPTION_INIT_INFO_NUM; inum++) {
        array[inum] = curr;
        curr = curr->next;
        array[inum]->next = NULL;
    }

    int idx;
    /* sort by id size */
    qsort(array, inum, sizeof(AVEncryptionInitInfo *), compare_id_size);
    /* find info from file */
    for (idx = 0; idx < inum; idx++) {
        if (array[idx]->system_id_size != SYSTEM_ID_SIZE_DEFAULT) {
            break;
        }
    }

    /* all from hls/mpd file */
    if (idx <= 1) {
        goto collect;
    }
    do_select_by_type(array, idx);
collect:
    for (idx = 0; idx < inum - 1; idx++) {
        array[idx]->next = array[idx + 1];
    }
    curr = array[0];
    return curr;
}

static void do_recorrect(AVEncryptionInitInfo *bad, AVEncryptionInitInfo *good)
{
    if (!bad || !good) {
        return;
    }

    /* cannot modify other part */
    if (bad->num_key_ids &&
        bad->key_id_size && bad->key_ids && bad->key_ids[0]) {
        return;
    }

    int type = get_system_type(bad);
    AVEncryptionInitInfo *curr = good;
    while(curr) {
        if (type == get_system_type(curr)) {
            break;
        }
        curr = curr->next;
    }

    if (!curr) {
        return;
    }

    if (curr->num_key_ids) {
        bad->key_ids = curr->key_id_size ?
            av_mallocz_array(curr->num_key_ids, sizeof(*curr->key_ids)) : NULL;
    }
    if (!bad->key_ids) {
        return;
    }

    bad->num_key_ids = curr->num_key_ids;
    bad->key_id_size = curr->key_id_size;
    for (unsigned int i = 0; i < curr->num_key_ids; i++) {
        bad->key_ids[i] = av_mallocz(curr->key_id_size);
        if (!bad->key_ids[i]) {
            return;
        }
        memcpy(bad->key_ids[i], curr->key_ids[i], curr->key_id_size);
    }
}

/* Only sort, not change content, need? */
static void recorrect_encryption_init_info(AVEncryptionInitInfo **info)
{
    AVEncryptionInitInfo *curr, *bad, *hbad, *good, *hgood;

    /* No change currently */
    curr = *info;
    hbad = hgood = good = bad = NULL;
    /*1. put good and bad in seperated link  */
    while (curr) {
        if (check_encryption_init_info(curr)) {
            if (!hgood) {
                hgood = good = curr;
            } else {
                good->next = curr;
                good = good->next;
            }
            curr = curr->next;
            good->next = NULL;
        } else {
            if (!hbad) {
                hbad = bad = curr;
            } else {
                bad->next = curr;
                bad = bad->next;
            }
            curr = curr->next;
            bad->next = NULL;
        }
    }
    /*2. correct information from file */
    curr = hbad;
    while(curr) {
        /* pssh from file, but miss some information,
         * only kid can change, other information cannot change.
         */
        if (SYSTEM_ID_SIZE_DEFAULT == curr->system_id_size) {
            (void) do_recorrect(curr, hgood);
        }
        curr = curr->next;
    }

    /*3. prefer info from file not from .mpd or .m3u8 */
    if (good) {
        good->next = hbad;
        curr = hgood;
    } else {
        curr = hbad;
    }

    *info = do_select_by_field(curr);
}

static void china_drm_parse_m3u8_pssh_kids(
    char *start_arry, char *end_arry, AVEncryptionInitInfo *info)
{
#define MAX_BASE64_ENCODE_SIZE 256
    char decode_buffer[MAX_BASE64_ENCODE_SIZE];
    start_arry++;
    while (start_arry < end_arry) {

        char *pid_start = strchr(start_arry, '"');
        if (!pid_start) {
            break;
        }

        start_arry++;
        char *pid_end = strchr(start_arry, '"');
        if (!pid_end) {
            break;
        }
        pid_start++; /* skip " */
        unsigned int encode_len = (unsigned int)(pid_end - pid_start);
        if (encode_len >= MAX_BASE64_ENCODE_SIZE) {
            av_log(NULL, AV_LOG_ERROR, "%s encode data too long!\n");
            break;
        }
        decode_buffer[encode_len] = '\0';
        memcpy(decode_buffer, pid_start, encode_len);
        uint8_t **key_ids = av_realloc_array(
            info->key_ids, info->num_key_ids + 1, sizeof(*info->key_ids));
        if (!key_ids) {
            break;
        }

        info->key_ids = key_ids;
        info->key_ids[info->num_key_ids] = av_mallocz(16);
        if (!info->key_ids[info->num_key_ids]) {
            break;
        }
        (void) av_base64_decode(info->key_ids[info->num_key_ids], decode_buffer, 16);
        info->num_key_ids += 1;
        start_arry = pid_end + 1;
    }
}

/*
 * Size          32   uimsbf PSSH
 * Type          32   uimsbf 0x70 73 73 68
 * Version        8   uimsbf 0x00 or 0x01
 * Flags         24   uimsbf 0x00 00 00
 * SystemId      128  uimsbf 0x3d5e6d359b9a41e8b843dd3c6e72c42c
 * KID_Count     32   uimsbf
 * KID KID_Count 16*8 uimsbf
 * DataSize      32   uimsbf
 * Data DataSize 8    uimsbf
*/
static int china_drm_parse_m3u8_uri(
    unsigned char *uri, AVEncryptionInitInfo *info)
{
/* size:4, type:4 ver:1, flags:3,id:16 data size:4 */
#define MIN_CHINA_DRM_PSSH_HEAD_LEN 32
    uri = uri + strlen("data:text/plain;base64,");
    if (info->header_data) {
        av_freep(&(info->header_data));
    }
    info->header_data_size = 0;
    info->header_data = av_strdup(uri);
    if (!(info->header_data)) {
        return AVERROR(ENOMEM);
    }

    info->header_data_size = strlen(info->header_data);
    unsigned char *decode_data = av_mallocz((info->header_data_size) * 2);
    if (!decode_data) {
        return 0;
    }

    int decoded_bytes = av_base64_decode(
        decode_data, info->header_data, info->header_data_size);
    if (decoded_bytes < 0) {
        return decoded_bytes;
    }

    if (decoded_bytes > (int) info->header_data_size) {
        av_log(NULL, AV_LOG_ERROR, "av_base64_decode china drm pssh error\n");
        decoded_bytes = info->header_data_size;
    }

    // decoded_bytes = FFMIN(decoded_bytes, AV_RB32(decode_data));
    /* size:4, type:4 ver:1, flags:3,id:16 data size:4 */
    if (decoded_bytes <= MIN_CHINA_DRM_PSSH_HEAD_LEN) {
        goto exit;
    }

    int version = (int) decode_data[8];
    if (1 == version) {
        av_log(NULL, AV_LOG_ERROR, "Not support hls china drm version 1\n");
        goto exit;
    }

    char *kids = strstr(&decode_data[MIN_CHINA_DRM_PSSH_HEAD_LEN], "\"kids\":");
    if (!kids) {
        goto exit;
    }

    char *start = strstr(&decode_data[MIN_CHINA_DRM_PSSH_HEAD_LEN], "[");
    char *end = strstr(&decode_data[MIN_CHINA_DRM_PSSH_HEAD_LEN], "]");
    if (!start || !end) {
        goto exit;
    }
    china_drm_parse_m3u8_pssh_kids(start, end, info);
exit:
    av_free(decode_data);
    av_log(NULL, AV_LOG_INFO, "header data is : %s\n", info->header_data);
    return 0;
}

#ifdef TRANSPORT_DESDEC_DEBUG
#include <openssl/aes.h>
#include <openssl/des.h>
enum MPDecryptionMode {
    DECRYPTION_DEFAULT,
    DECRYPTION_XOR,
    DECRYPTION_3DES,
    DECRYPTION_AES,
    DECRYPTION_NONE,
};

static void dump_to_file(char *out, int len)
{
    FILE *fp_enc = NULL;
    if (!fp_enc) {
        fp_enc =fopen("T:\\nfs\\bin\\clip\\kj_iptv_youtube\\xor\\test.bin", "ab+");
    }
    if (NULL != fp_enc) {
        fwrite(out, 1, 188, fp_enc);
        fclose(fp_enc);
        fp_enc = NULL;
    }
}

/************************************************************************
 ** Function
 **    decrypt way : DES-ECB
 **    key         : 24B,fill 0x00 if key len < 24B,get 24B header if key > 24B
 **    padding     : PKCS7Padding
 ************************************************************************/
static uint32_t bin_des3_dec(unsigned char *pucKey, unsigned char *pucBuf, uint32_t iInLen)
{
    uint32_t iIndex = 0;
    unsigned char block_key[9];
    DES_key_schedule ks, ks2, ks3;

        memset(block_key, 0, sizeof(block_key));
        memcpy(block_key, pucKey + 0, 8);
        DES_set_key_unchecked((const_DES_cblock*)block_key, &ks);
        memcpy(block_key, pucKey + 8, 8);
        DES_set_key_unchecked((const_DES_cblock*)block_key, &ks2);
        memcpy(block_key, pucKey + 16, 8);
        DES_set_key_unchecked((const_DES_cblock*)block_key, &ks3);

        for (iIndex = 0; iIndex < iInLen; iIndex += 8) {
            DES_ecb3_encrypt((const_DES_cblock*)&pucBuf[iIndex],
                (DES_cblock*)&pucBuf[iIndex], &ks, &ks2, &ks3, DES_DECRYPT);
        }

    return 0;
}

/************************************************************************
 ** Function
 **    decrypt way : XOR
 ************************************************************************/
static void xor_dec(char *packet, const char *key)
{
    for(uint32_t i = 4; i < 188; i++) {
        packet[i] ^= key[i % strlen(key)];
    }
}

/************************************************************************
 ** Function
 **    decrypt way : AES-CBC
 **    key         : 24B,fill 0x00 if key len < 24B,get 24B header if key > 24B
 **    padding     : PKCS7Padding
 ************************************************************************/
static uint32_t bin_aes_dec(unsigned char *pucKey, unsigned char *pucBuf, uint32_t iInLen)
{
    AES_KEY AESkeyDec;
    unsigned char iv[AES_BLOCK_SIZE + 1];
        memset(iv, 0, AES_BLOCK_SIZE + 1);
    unsigned char out[2048];
    memset(out, 0, 2048);

    if (AES_set_decrypt_key(pucKey, 24 * 8, &AESkeyDec) < 0) {
        av_log(NULL, AV_LOG_ERROR, "AES_set_decrypt_key fail\n");
        return -1;
    }
    memcpy(iv, pucKey, AES_BLOCK_SIZE);
    AES_cbc_encrypt(pucBuf, out, iInLen, &AESkeyDec, iv, AES_DECRYPT);
    memcpy(pucBuf, out, iInLen);

    return 0;
}

static int x_decrypt_ts_func(void *h, char *packet, char *out, int len, int type)
{
    if (((packet[3] >> 6) & 0x3) != 0x3) {
        return -1;
    }

    char key[64] = {0};
    memcpy(out, packet, 4);
    out[3] &= 0x3F;
    if (DECRYPTION_XOR == type) {
        sprintf(key, "12345abcde");
        xor_dec(packet, key);
       // dump_to_file(out, 188);
    } else if (DECRYPTION_3DES == type) {
        sprintf(key, "12345abcde");
        bin_des3_dec((unsigned char*)key, (unsigned char *)(packet + 4), 184);
        av_hex_dump_log(NULL, AV_LOG_TRACE, packet, 188);
        // dump_to_file(packet, 188);
    } else if (DECRYPTION_AES == type) {
        sprintf(key, "12345abcde");
        bin_aes_dec((unsigned char*)key, (unsigned char *)(packet + 12), 176);
        av_hex_dump_log(NULL, AV_LOG_TRACE, packet, 188);
    }
    memcpy(out, packet, len);
    return 0;
}

static int transport_desdec_callback(void *h,
    const void *info, const unsigned char *in, unsigned char *out, int len)
{
    return x_decrypt_ts_func(h, in, out, len, DECRYPTION_XOR);
   // return x_decrypt_ts_func(h, in, out, len, DECRYPTION_AES);
   // return x_decrypt_ts_func(h, in, out, len, DECRYPTION_3DES);
}

#endif

int ff_hls_parse_ext_x_key(unsigned char *uri,
    unsigned char *keyformat, AVEncryptionInitInfo **encryption_info)
{
    if (!uri || !keyformat || !encryption_info) {
        return AVERROR(EINVAL);
    }

    int32_t ret = 0;
    AVEncryptionInitInfo *info = *encryption_info;
    if (!info) {
        info = av_encryption_init_info_alloc(CONTENT_PROTECTION_SYSTEM_ID_SIZE,
            /* num_key_ids */ 0, /* key_id_size */ 16, /* data_size */ 0);
        if (!info) {
            return AVERROR(ENOMEM);
        }
    }

    parse_encryption_info_system_id(keyformat, info);
    if (DRM_SYSTEM_TYPE_CHINADRM != get_system_type(info)) {
        goto fail;
    }
    ret = china_drm_parse_m3u8_uri(uri, info);
    if (ret < 0) {
        goto fail;
    }
    *encryption_info = info;
    return 0;
fail:
    av_encryption_init_info_free(info);
    return ret;
}

int ff_dash_parse_content_protection(
    xmlNodePtr node, AVEncryptionInitInfo **encryption_info)
{
    int32_t ret = 0;

    AVEncryptionInitInfo *info = *encryption_info;
    if (!info) {
        info = av_encryption_init_info_alloc(CONTENT_PROTECTION_SYSTEM_ID_SIZE,
            /* num_key_ids */ 0, /* key_id_size */ 16, /* data_size */ 0);
        if (!info) {
            return AVERROR(ENOMEM);
        }
    }

    parse_content_protection_scheme_id_uri(node, info);
    parse_content_protection_default_kid(node, info);
    ret = parse_content_protection_header_info(node, info);
    if (ret < 0) {
        av_encryption_init_info_free(info);
        return ret;
    }

    *encryption_info = info;
    return 0;
}

int ff_dash_clone_content_protection(
    AVEncryptionInitInfo **dst, AVEncryptionInitInfo *src)
{
    uint32_t i;
    int32_t ret = 0;
    if (!src || !dst || !src->header_data_size) {
        return AVERROR_INVALIDDATA;
    }

    uint8_t *header_data =
        av_malloc(src->header_data_size);
    if (!header_data) {
        return AVERROR(ENOMEM);
    }

    AVEncryptionInitInfo *info =
        av_encryption_init_info_alloc(src->system_id_size,
            src->num_key_ids, src->key_id_size, src->data_size);
    if (!info) {
        av_free(header_data);
        return AVERROR(ENOMEM);
    }

    info->header_data = header_data;
    info->header_data_size = src->header_data_size;
    memcpy(info->data, src->data, src->data_size);
    memcpy(info->system_id, src->system_id, src->system_id_size);
    memcpy(info->header_data, src->header_data, src->header_data_size);
    for (i = 0; i < src->num_key_ids; i++) {
        info->key_ids[i] = av_mallocz(src->key_id_size);
        if (!info->key_ids[i]) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        memcpy(info->key_ids[i], src->key_ids[i], src->key_id_size);
    }
    *dst = info;
    return 0;
fail:
    av_encryption_init_info_free(info);
    return ret;
}

void ff_dash_free_content_protection(
    AVEncryptionInitInfo **encryption_info)
{
    if (!encryption_info || !*encryption_info) {
        return;
    }
    av_encryption_init_info_free(*encryption_info);
    *encryption_info = NULL;
}

int ff_decrypt_comp_get_drm_system_type(AVEncryptionInitInfo *info)
{
    return get_system_type(info);
}

int ff_decrypt_comp_merge_init_info(AVStream *st, AVEncryptionInitInfo *info)
{
    int ret = 0;
    AVEncryptionInitInfo *old_init_info = NULL;
    if (!st || !info) {
        return 0;
    }

    size_t side_data_size, old_side_data_size;
    uint8_t *side_data, *old_side_data;
    /* There is existing initialization data */
    old_side_data = av_stream_get_side_data(st,
        AV_PKT_DATA_ENCRYPTION_INIT_INFO, &old_side_data_size);
    if (!old_side_data) {
        AVDictionaryEntry *entry = NULL;
       /* only encrypted mkv/webm set base64 key id, which should be used, such as tears.mpd */
        entry = av_dict_get(st->metadata, "enc_key_id", NULL, 0);
        if (entry) {
            uint8_t out_key_id[16] = {0};
            ret = av_base64_decode(&out_key_id[0], entry->value, 16);
            if (ret >= 0 && 16 == info->key_id_size) {
                memcpy(info->key_ids[0], &out_key_id[0], 16);
                av_log(NULL, AV_LOG_WARNING, "Change default key id according file\n");
            }
        }
    } else {
        old_init_info = av_encryption_init_info_get_side_data(old_side_data, old_side_data_size);
        if (old_init_info) {
            // Append to the end of the list.
            for (AVEncryptionInitInfo *cur = old_init_info;; cur = cur->next) {
                if (!cur->next) {
                    cur->next = info;
                    break;
                }
            }
            info = old_init_info;
        } else {
            /* Assume existing side-data will be valid, so the only error we could get is OOM. */
            return AVERROR(ENOMEM);
        }
        /* No need correct when only one info */
        recorrect_encryption_init_info(&info);
    }
    side_data = av_encryption_init_info_add_side_data(info, &side_data_size);
    if (!side_data) {
        return AVERROR(ENOMEM);
    }
    ret = av_stream_add_side_data(st, AV_PKT_DATA_ENCRYPTION_INIT_INFO,
                                  side_data, side_data_size);
    if (ret < 0) {
        av_free(side_data);
    }

    st->need_parsing = AVSTREAM_PARSE_NONE;
    return ret;
}

/**
 * ChinaDrm_descriptor (0xC0) parse_china_drm_descriptor:
 */
int ff_transport_parse_china_drm_descriptor(
    AVFormatContext *s, const uint8_t *buf, unsigned int size, ChinaDrmDescr *descr)
{
    if (!s || !descr || !buf || size < 2) {
        return AVERROR(EINVAL);
    }
    av_hex_dump_log(s, AV_LOG_DEBUG, buf, size);

    descr->video_format            = (unsigned int) ((buf[0] >> 4) & 0xF);
    descr->audio_format            = (unsigned int) ((buf[1] >> 4) & 0xF);
    descr->video_encryption_method = (unsigned int) (buf[0] & 0xF);
    descr->audio_encryption_method = (unsigned int) (buf[1] & 0xF);

    descr->nb_drm_data_bytes = size - 2;
    /* max 253 bytes  */
    if (descr->nb_drm_data_bytes > 253) {
        descr->nb_drm_data_bytes = 253;
    }
    memcpy(descr->drm_data_bytes, &buf[2], descr->nb_drm_data_bytes);
    av_log(s, AV_LOG_DEBUG, "a/v format (%d %d) encryption method(0x%x 0x%x) \n",
        descr->video_format, descr->audio_format, descr->video_encryption_method, descr->audio_encryption_method);
    return 0;
}

int ff_transport_get_desdec_callback(
    AVFormatContext *s, TransportDesDecCallback *cb)
{
    if (!s || !cb || !s->control_message_cb) {
        return AVERROR(EINVAL);
    }
#ifdef TRANSPORT_DESDEC_DEBUG
    *cb = transport_desdec_callback;
    return 0;
#else
    intptr_t func = 0;
    int ret = s->control_message_cb(s,
        AV_TRANSPORT_QUERY_DESDEC_CB, &func, sizeof(func));
    *cb = (TransportDesDecCallback) func;
    return ret;
#endif
}

int ff_transport_desdec(AVFormatContext *s, TSScramEncInfo *info,
    TransportDesDecCallback cb, const unsigned char *in, unsigned char *out, int len)
{
    if (!s || !in || !out || !len || !cb) {
        return AVERROR(EINVAL);
    }

    return cb((void *) s, (void *) info, in, out, len);
}
