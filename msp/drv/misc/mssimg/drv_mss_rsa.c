/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_ce_if.h"
#include "drv_mss_rsa.h"
#include "mt_cache.h"
#include "mt_drv_mmz.h"

__maybe_unused static int hexdump(const char *name, const void *addr, int len)
{
    int i = 0;

    if (name != 0) {
        printk("\n==================%s==================\n", name);
    }
    printk("Dump %p(%d):", addr, len);
    for (i = 0; i < len; ++i) {
        if ((i % 16) == 0)
            printk("\n");

        printk("%02x ", *(unsigned char *)((unsigned long)addr + i));
    }
    printk("\n");

    return 0;
}

__maybe_unused static inline void flush_dcache_range(ulong addr, ulong size)
{
    ulong residue = addr & (CACHE_LINE_SIZE - 1);
    addr &= ~residue;
    size = (size + residue + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
    mt_dcache_flush((void*)addr, size);
}

static int rsa_crypto_public(struct rsa_public_key *key,
        const unsigned char *src, unsigned int src_len, unsigned char *dst)
{
    int ret = 0;
    mt_session p_cipher;
    MT_CE_RSA_CTRL_S info;
    void *p_priv = drv_ce_get_handle();

    memset(&info, 0, sizeof(MT_CE_RSA_CTRL_S));

    memcpy(info.rsa_para.p_m, key->n, key->n_length);
    info.rsa_para.key_length = key->n_length;

    memcpy(info.rsa_para.p_e + key->n_length - key->e_length, key->e, key->e_length);
    info.rsa_para.exp_length = key->n_length;

    ret = drv_ce_rsa_create(&p_cipher, p_priv);
    if (ret != MT_SUCCESS)
        goto EXIT;

    ret = drv_ce_rsa_config(p_cipher, &info);
    if (ret != MT_SUCCESS)
    	goto EXIT;

    ret = drv_ce_rsa_process(p_cipher, (mt_u8 *)src, dst, src_len);

EXIT:
    drv_ce_rsa_destroy(p_cipher);

    return ret;
}

static int drv_mss_hash(mt_u32 hashtype, mt_u8 *p_msg, mt_u32 length, mt_u8 *p_out)
{
    int ret = MT_FAILURE;
    mt_session hash_handle;
    MT_CE_SHA_CTRL_S ctrl_s = { 0 };
    mmz_buffer_s mmz_buf;

    if (p_out == NULL)
        return MT_FAILURE;

    if (mt_drv_mmz_alloc_and_map("PSS_MGF_BUF", "ddr", length, 64, &mmz_buf)) {
        printk("%s: %d mmz malloc fail!\n", __func__, __LINE__);
        return MT_FAILURE;
    }
    memcpy(mmz_buf.startVirAddr, p_msg, length);

    if (hashtype == MT_CIPHER_HASH_TYPE_SHA1) {
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SHA1;
    } else if (hashtype == MT_CIPHER_HASH_TYPE_SHA256) {
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SHA256;
    } else {
        printk("%s: %d bad algo mode!\n", __func__, __LINE__);
        goto exit;
    }

    ret = drv_ce_sha_create(&hash_handle, NULL);
    if (ret != MT_SUCCESS) {
        goto exit;
    }

    ret = drv_ce_sha_init(hash_handle, &ctrl_s);
    if (ret != MT_SUCCESS) {
        goto exit;
    }

    ret = drv_ce_sha_update(hash_handle, 1, (phys_addr_t)mmz_buf.startPhyAddr, length);
    if (ret != MT_SUCCESS) {
        goto exit;
    }
    ret = drv_ce_sha_final(hash_handle, p_out);
    if (ret != MT_SUCCESS) {
        goto exit;
    }

    ret = drv_ce_sha_destroy(hash_handle);
    if (ret != MT_SUCCESS) {
        goto exit;
    }

    ret = MT_SUCCESS;
exit:
    mt_drv_mmz_unmap_and_release(&mmz_buf);

    return ret;
}

static int MGF1(mt_u8 *mask, mt_u32 len, mt_u8 *seed, mt_u32 seedlen, mt_u32 hashtype)
{
    int ret = 0;
    mt_u32 i, outlen = 0;
    mt_u8 cnt[4];
    mt_u8 md[64];
    mt_u8 update_buffer[128]; /* H(32B):digest(20B or 32B) + cnt(4B) = 36B */
    mt_u32 mdlen;

    mdlen = (MT_CIPHER_HASH_TYPE_SHA1 == hashtype) ? 20 : 32;

    for (i = 0; outlen < len; i++) {
        cnt[0] = (unsigned char)((i >> 24) & 255);
        cnt[1] = (unsigned char)((i >> 16) & 255);
        cnt[2] = (unsigned char)((i >> 8) & 255);
        cnt[3] = (unsigned char)(i & 255);

        memcpy(update_buffer, seed, (mt_u32)seedlen);
        memcpy(update_buffer + seedlen, cnt, 4);

        if (outlen + mdlen <= len) {
            ret = drv_mss_hash(hashtype, update_buffer, seedlen + 4, mask + outlen);
            outlen += mdlen;
        } else {
            ret = drv_mss_hash(hashtype, update_buffer, seedlen + 4, md);
            memcpy(mask + outlen, md, (mt_u32)(len - outlen));
            outlen = len;
        }

        if (ret) {
            printk("%s: %d calc hash fail!\n", __func__, __LINE__);
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

static int PKCS1_PSS_verify(const mt_u8 *msg, mt_u32 mlen, mt_u32 emBits,
        mt_u8 *sign, mt_u32 slen, mt_u32 hashtype)
{
    int ret, i;
    mt_u32 hlen, saltlen;
    mt_u32 emlen, dblen;
    mt_u8 M1[32+32+8];
    mt_u8 mask[256];
    mt_u8 H1[32];
    mt_u8 *mHash, *salt;
    mt_u8 *DB;
    mt_u8 *em, *maskedDB, *H;
    mt_u8 *p;
    mt_u8 lmask;

    hlen = (MT_CIPHER_HASH_TYPE_SHA1 == hashtype) ? 20 : 32;

    em = sign;
    emlen = (emBits+7)/8;

    lmask = 0;
    for (i=0; i<emlen*8-emBits; i++)
        lmask = (lmask>>1) | 0x80;

    /* printk("%s: %d, mlen: 0x%x, slen: 0x%x, hashtype: %d, lmask: %d\n", __func__, __LINE__, mlen, slen, hashtype, lmask); */

    /* hexdump("msg", msg, 32); */
    /* hexdump("sign", sign, slen); */

    /* BC */
    if (em[emlen - 1] != 0xBC)
        return MT_RSA_ERR_BAD_PADDING;

    /* maskedDB */
    maskedDB = em;
    dblen = emlen - hlen -1;

    /* H */
    H = em + dblen;

    /* DB = maskedDB ^ MGF(H) */
    DB = maskedDB;

    ret = MGF1(mask, dblen, H, hlen, hashtype);
    if (ret) {
        printk("%s: %d error!\n", __func__, __LINE__);
        return MT_RSA_ERR_BAD_PADDING;
    }

    /* hexdump("MGF_H", H, hlen); */
    /* hexdump("MGF_mask", mask, sizeof(mask)); */

    for (i = 0; i < dblen; i++)
    	DB[i] ^= mask[i];

    DB[0] &= (mt_u8)~lmask;

    /* DB = PS + 01 + salt ==> salt */
    p = DB;
    while( p < H - 1 && *p == 0 )
        p++;

    if( *p++ != 0x01 ) {
        printk("%s: %d error!\n", __func__, __LINE__);
        return MT_RSA_ERR_BAD_PADDING;
    }

    salt = p;
    saltlen = (mt_u32)(H - salt);

    /* M' = pad + mHash + salt */

    /* pad with zero-padded */
    memset(M1, 0, 8);

    mHash = M1 + 8;

    /* mHash */
    ret = drv_mss_hash(hashtype, (mt_u8 *)msg, mlen, mHash);

    /* salt */
    memcpy(mHash + hlen, salt, saltlen);

    /* H' = hash(M') */
    H = em + emlen - hlen - 1;

    ret = drv_mss_hash(hashtype, M1, 8 + hlen + saltlen, H1);

    if(memcmp(H, H1, hlen) != 0)
        return MT_RSA_ERR_BAD_PADDING;

    return MT_SUCCESS;
}

#define MT_RSA_SIGN      1
#define MT_RSA_CRYPT     2

static mt_u8 sha1_digestinfo[15] = {0x30,0x21,0x30,0x09,0x06,0x05,0x2b,0x0e,0x03,0x02,0x1a,0x05,0x00,0x04,0x14};
static mt_u8 sha256_digestinfo[19] = {0x30,0x31,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0x04,0x20};

static int PKCS1_V1_5_decode(mt_u8 *to, mt_u32 *tlen,
    mt_u8 *from, mt_u32 flen, mt_u32 mode)
{
    mt_u32 i;
    mt_u8 *p = from;
    mt_u8 bad = 0;

    /* 0 */
    bad |= p[0];

    if (mode == MT_RSA_CRYPT)
    {
        /* BT */
        bad |= p[1] ^ MT_RSA_CRYPT;

        /* PS: pad out with non-zero random data */
        for (i=2; i<flen; i++)
        {
            if (p[i] == 0)
                break;
        }
    }
    else
    {
        /* BT */
        bad |= p[1] ^ MT_RSA_SIGN;

        /* PS: pad out with 0xFF */
        for (i=2; i<flen; i++)
        {
            if (p[i] != 0xFF)
            {
                if (p[i] == 0)
                    break;

                bad = 1;
                break;
            }
        }
    }

    if (i == flen)
        bad = 1;
    if (i < 10)
        bad = 1;

    if (bad == 0)
    {
        /* 0 */
        i++;

        /* D */
        *tlen = flen - i;
        memcpy(to, &p[i], *tlen);
    }

    if (bad)
        return MT_RSA_ERR_BAD_PADDING;
    else
        return MT_SUCCESS;
}

static int PKCS1_V1_5_verify(const mt_u8 *msg, mt_u32 mlen,
        mt_u8 *sign, mt_u32 slen, mt_u32 hashtype)
{
    int ret;
    mt_u8 digestinfo[64];
    mt_u32 digestinfoLen;
    mt_u8 *digest;
    mt_u8 hash[32];
    mt_u32 hlen;

    ret = PKCS1_V1_5_decode(digestinfo, &digestinfoLen, sign, slen, MT_RSA_SIGN);
    if (ret != MT_SUCCESS)
        return ret;

    if (MT_CIPHER_HASH_TYPE_SHA1 == hashtype)
    {
        if (digestinfoLen != 35)
            return MT_RSA_ERR_BAD_PADDING;

        if (memcmp(digestinfo, sha1_digestinfo, 15))
            return MT_RSA_ERR_BAD_PADDING;

        digest = digestinfo + 15;
        hlen = 20;
    }
    else if (MT_CIPHER_HASH_TYPE_SHA256 == hashtype)
    {
        if (digestinfoLen != 51)
            return MT_RSA_ERR_BAD_PADDING;

        if (memcmp(digestinfo, sha256_digestinfo, 19))
            return MT_RSA_ERR_BAD_PADDING;

        digest = digestinfo + 19;
        hlen = 32;
    }
    else
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    /* digest = HASH(msg) */
    ret = drv_mss_hash(hashtype, (mt_u8 *)msg, mlen, hash);

    if(memcmp(digest, hash, hlen) != 0)
        return MT_RSA_ERR_BAD_PADDING;

    return MT_SUCCESS;
}

int mt_rsa_verify(struct rsa_public_key *key, MT_RSA_ALG algo,
	const unsigned char *msg, unsigned int mlen, const unsigned char *sign, unsigned int slen)
{
    int ret;
    mt_u32 buf[64];
    mt_u8 *signaure;

    signaure = (mt_u8 *)buf;

    if (NULL == key || NULL == msg || NULL == sign)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (NULL == key->e || NULL == key->n)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (slen != key->n_length)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    ret = rsa_crypto_public(key, sign, slen, signaure);
    if (ret != MT_SUCCESS)
        return ret;

    if (algo == MT_RSASSA_PKCS1_PSS_MGF1_SHA1)
        ret = PKCS1_PSS_verify(msg, mlen, key->n_length*8-1, signaure, slen,
                        MT_CIPHER_HASH_TYPE_SHA1);
    else if (algo == MT_RSASSA_PKCS1_PSS_MGF1_SHA256)
        ret = PKCS1_PSS_verify(msg, mlen, key->n_length*8-1, signaure, slen,
                        MT_CIPHER_HASH_TYPE_SHA256);
    else if (algo == MT_RSASSA_PKCS1_V1_5_SHA1)
        ret = PKCS1_V1_5_verify(msg, mlen, signaure, slen,
                        MT_CIPHER_HASH_TYPE_SHA1);
    else if (algo == MT_RSASSA_PKCS1_V1_5_SHA256)
        ret = PKCS1_V1_5_verify(msg, mlen, signaure, slen,
                        MT_CIPHER_HASH_TYPE_SHA256);
    else
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    return ret;
}

