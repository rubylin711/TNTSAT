/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mt_type.h"
#include "mt_common.h"
#include "mt_module_debug.h"
#include "mt_unf_cipher_v2.h"
#include "mt_mpi_keyladder.h"
#include "mt_mpi_cipher_v2.h"
#include "mt_mpi_ce.h"
#include "drv_ce_ioctl.h"
#include "mt_mpi_bn.h"

#define SUPPORT_SOFTWARE_HMAC   1
#ifdef SUPPORT_SOFTWARE_HMAC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// SHA1常量定义
#define SHA1_BLOCK_SIZE 64
#define SHA1_DIGEST_SIZE 20

// SHA1上下文结构（用于增量计算）
typedef struct {    
	uint8_t data[SHA1_BLOCK_SIZE];  // 数据缓冲区    
	uint32_t datalen;               // 缓冲区中数据长度    
	uint64_t bitlen;                // 总数据位数（用于最终填充）    
	uint32_t state[5];              // SHA1状态寄存器
} SHA1_CTX;// HMAC-SHA1上下文结构（支持增量更新）

typedef struct {    
	SHA1_CTX inner_ctx;             // 内层哈希上下文（处理 key^ipad + data）    
	uint8_t k_opad[SHA1_BLOCK_SIZE];// 外层密钥（key^opad）
} HMAC_SHA1_CTX;
// 循环左移宏
#define ROTLEFT(a, b) (((a) << (b)) | ((a) >> (32 - (b))))
static  HMAC_SHA1_CTX g_sym6_ctx;
#endif //SUPPORT_SOFTWARE_HMAC


/* For symmetric crypto algorithms and hash, hmac, bdc, rnd*/
static pthread_mutex_t g_m2m_mutex = PTHREAD_MUTEX_INITIALIZER;
/* For asymmetric crypto algorithms (rsa/bn/ecc)*/
static pthread_mutex_t g_m2m_asym_mutex = PTHREAD_MUTEX_INITIALIZER;


/*---------------------SUPPORT_SOFTWARE_HMAC----------------------------*/
/*
 * Call it inside lock
 */
#ifdef SUPPORT_SOFTWARE_HMAC

// SHA1核心变换函数（内部使用）
static void sha1_transform(SHA1_CTX *ctx, const uint8_t data[]) {
    uint32_t a, b, c, d, e, i, j, t, m[80];

    // 将输入数据转换为32位字并扩展
    for (i = 0; i < 16; i++) {
        m[i] = (data[i*4] << 24) | (data[i*4+1] << 16) | 
               (data[i*4+2] << 8) | (data[i*4+3]);
    }
    for (i = 16; i < 80; i++) {
        m[i] = ROTLEFT(m[i-3] ^ m[i-8] ^ m[i-14] ^ m[i-16], 1);
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];

    // 4轮SHA1计算
    for (i = 0; i < 20; i++) {
        t = ROTLEFT(a, 5) + ((b & c) | (~b & d)) + e + 0x5a827999 + m[i];
        e = d; d = c; c = ROTLEFT(b, 30); b = a; a = t;
    }
    for (i = 20; i < 40; i++) {
        t = ROTLEFT(a, 5) + (b ^ c ^ d) + e + 0x6ed9eba1 + m[i];
        e = d; d = c; c = ROTLEFT(b, 30); b = a; a = t;
    }
    for (i = 40; i < 60; i++) {
        t = ROTLEFT(a, 5) + ((b & c) | (b & d) | (c & d)) + e + 0x8f1bbcdc + m[i];
        e = d; d = c; c = ROTLEFT(b, 30); b = a; a = t;
    }
    for (i = 60; i < 80; i++) {
        t = ROTLEFT(a, 5) + (b ^ c ^ d) + e + 0xca62c1d6 + m[i];
        e = d; d = c; c = ROTLEFT(b, 30); b = a; a = t;
    }

    // 更新状态
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
}

// 初始化SHA1上下文
static void sha1_init(SHA1_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xc3d2e1f0;
}

// 增量更新SHA1数据（支持分多次输入）
static void sha1_update(SHA1_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; i++) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == SHA1_BLOCK_SIZE) {
            sha1_transform(ctx, ctx->data);  // 满一块后进行变换
            ctx->bitlen += 512;              // 累计位数（512 = 64*8）
            ctx->datalen = 0;
        }
    }
}

// 完成SHA1计算并输出结果
static void sha1_final(SHA1_CTX *ctx, uint8_t hash[]) {
    uint32_t i = ctx->datalen;

    // 填充位（100...0）
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < SHA1_BLOCK_SIZE) ctx->data[i++] = 0x00;
        sha1_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
        i = 0;
    }

    // 填充总长度（以位为单位）
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[56] = (ctx->bitlen >> 56) & 0xff;
    ctx->data[57] = (ctx->bitlen >> 48) & 0xff;
    ctx->data[58] = (ctx->bitlen >> 40) & 0xff;
    ctx->data[59] = (ctx->bitlen >> 32) & 0xff;
    ctx->data[60] = (ctx->bitlen >> 24) & 0xff;
    ctx->data[61] = (ctx->bitlen >> 16) & 0xff;
    ctx->data[62] = (ctx->bitlen >> 8) & 0xff;
    ctx->data[63] = ctx->bitlen & 0xff;
    sha1_transform(ctx, ctx->data);

    // 转换状态为哈希结果（大端转小端）
    for (i = 0; i < 5; i++) {
        hash[i*4] = (ctx->state[i] >> 24) & 0xff;
        hash[i*4+1] = (ctx->state[i] >> 16) & 0xff;
        hash[i*4+2] = (ctx->state[i] >> 8) & 0xff;
        hash[i*4+3] = ctx->state[i] & 0xff;
    }
}

// 初始化HMAC-SHA1上下文（输入密钥）
static void hmac_sha1_init(HMAC_SHA1_CTX *ctx, const uint8_t *key, size_t key_len) {
    uint8_t k_ipad[SHA1_BLOCK_SIZE];
    uint8_t key_hash[SHA1_DIGEST_SIZE];
    size_t i;

    // 处理密钥：若密钥长度超过块大小，先哈希缩短
    if (key_len > SHA1_BLOCK_SIZE) {
        SHA1_CTX sha_ctx;
        sha1_init(&sha_ctx);
        sha1_update(&sha_ctx, key, key_len);
        sha1_final(&sha_ctx, key_hash);
        key = key_hash;
        key_len = SHA1_DIGEST_SIZE;
    }

    // 初始化ipad和opad
    memset(k_ipad, 0, SHA1_BLOCK_SIZE);
    memset(ctx->k_opad, 0, SHA1_BLOCK_SIZE);
    memcpy(k_ipad, key, key_len);
    memcpy(ctx->k_opad, key, key_len);

    // 与固定值异或（ipad=0x36, opad=0x5c）
    for (i = 0; i < SHA1_BLOCK_SIZE; i++) {
        k_ipad[i] ^= 0x36;
        ctx->k_opad[i] ^= 0x5c;
    }

    // 初始化内层哈希：先处理 k_ipad
    sha1_init(&ctx->inner_ctx);
    sha1_update(&ctx->inner_ctx, k_ipad, SHA1_BLOCK_SIZE);
}

// 增量更新HMAC数据（支持分多次输入）
static void hmac_sha1_update(HMAC_SHA1_CTX *ctx, const uint8_t *data, size_t len) {
    // 将数据累加到内层哈希中
    sha1_update(&ctx->inner_ctx, data, len);
}

// 完成HMAC计算并输出结果
static void hmac_sha1_final(HMAC_SHA1_CTX *ctx, uint8_t *digest) {
    uint8_t inner_hash[SHA1_DIGEST_SIZE];
    SHA1_CTX outer_ctx;

    // 完成内层哈希：得到 H(k_ipad + data)
    sha1_final(&ctx->inner_ctx, inner_hash);

    // 计算外层哈希：H(k_opad + 内层结果)
    sha1_init(&outer_ctx);
    sha1_update(&outer_ctx, ctx->k_opad, SHA1_BLOCK_SIZE);
    sha1_update(&outer_ctx, inner_hash, SHA1_DIGEST_SIZE);
    sha1_final(&outer_ctx, digest);
}

// 辅助函数：将字节数组转为十六进制字符串
static void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex) {
    const char *hex_chars = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        hex[i*2] = hex_chars[(bytes[i] >> 4) & 0x0f];
        hex[i*2+1] = hex_chars[bytes[i] & 0x0f];
    }
    hex[len*2] = '\0';
}

// 示例：分多次update数据计算HMAC-SHA1
/*
int hmac_sha1_test() {
    // 示例密钥和数据（分三段）
    const uint8_t key[] = "mysecretkey";
    const uint8_t data1[] = "Hello, ";
    const uint8_t data2[] = "HMAC-";
    const uint8_t data3[] = "SHA1!";
    
    // 初始化HMAC上下文
    HMAC_SHA1_CTX ctx;
    hmac_sha1_init(&ctx, key, sizeof(key)-1);  // 减去字符串结束符'\0'
    
    // 分三次更新数据
    hmac_sha1_update(&ctx, data1, sizeof(data1)-1);
    hmac_sha1_update(&ctx, data2, sizeof(data2)-1);
    hmac_sha1_update(&ctx, data3, sizeof(data3)-1);
    
    // 计算最终结果
    uint8_t digest[SHA1_DIGEST_SIZE];
    hmac_sha1_final(&ctx, digest);
    
    // 输出十六进制结果
    char hex_digest[SHA1_DIGEST_SIZE * 2 + 1];
    bytes_to_hex(digest, SHA1_DIGEST_SIZE, hex_digest);
    printf("HMAC-SHA1结果: %s\n", hex_digest);  // 应与一次性输入结果相同
    
    return 0;
}
*/

#endif  
/*---------------------SUPPORT_SOFTWARE_HMAC----------------------------*/

//#define MSS_INTERNAL_DEBUG
#ifdef MSS_INTERNAL_DEBUG
static void dump_data(unsigned char *data_src, mt_u32 len)
{
    unsigned int i = 0;

    printf("\n");
    for (i = 0; i < len; i++) {
        printf("%02x ", data_src[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}
#endif


/*!
  Cipher operation decrypt or encrypt
  */
mt_s32 mt_mpi_cipher_init(void)
{
    return MT_SUCCESS;
}

mt_s32 mt_mpi_cipher_deinit(void)
{
    MSS_LOCK_DESTROY(&g_m2m_mutex);
    MSS_LOCK_DESTROY(&g_m2m_asym_mutex);

    return MT_SUCCESS;
}

mt_s32 mt_mpi_crypto_open(int *ce_fd)
{
    int fd = -1;
    mt_s32 ret = MT_FAILURE;

    MSS_LOCK(&g_m2m_mutex);

    fd = open("/dev/mt_crypto_engine", O_RDWR, 0);
    if (fd >= 0) {
        *ce_fd = fd;
        ret = MT_SUCCESS;
    } else {
        ret = MT_FAILURE;
    }

    MSS_UNLOCK(&g_m2m_mutex);
    return ret;
}

mt_s32 mt_mpi_crypto_close(int ce_fd)
{
    if (ce_fd <= 0)
        return MT_FAILURE;

    return close(ce_fd);
}

mt_u8 *mt_mpi_cipher_malloc(mt_u32 size)
{
    mt_void *vir_addr;
    phys_addr_t phy_addr;

    if (size == 0)
        return NULL;

    phy_addr = mt_mmz_new(size, 0, "ddr", "cipher_buf");
    if (phy_addr == 0) {
        return NULL;
    }

    //printf("%s::Got phyaddr=0x%x\n", __FUNCTION__, phy_addr);

    //map,but not cached
    vir_addr = mt_mmz_map(phy_addr, 0);
    if (vir_addr == NULL) {
        mt_mmz_delete(phy_addr);
        return NULL;
    }

    /* success,return the virtual address of the buffer */
    return (mt_u8 *)vir_addr;
}

mt_s32 mt_mpi_cipher_free(mt_void *p_vir)
{
    mt_s32 ret;
    phys_addr_t phy_addr;
    ulong phy_size;

    if (p_vir == NULL) {
        printf("Can not free NULL pointer\n");
        return -1;
    }

    //printf("%s::Got viraddr=0x%x\n", __FUNCTION__, (mt_u32)p_vir);

    ret = mt_mmz_get_phyaddr(p_vir, &phy_addr, &phy_size);

    //printf("%s::Got phyaddr=0x%x, physize=%d\n", __FUNCTION__, phy_addr, phy_size);
    ret |= mt_mmz_unmap(p_vir);
    ret |= mt_mmz_delete(phy_addr);

    //printf("%s::return %d\n", __FUNCTION__, ret);
    return ret;
}

/* virutal address to physical address, no check */
static phys_addr_t vir2phy(mt_u8 *vir)
{
    mt_s32 ret = MT_SUCCESS;
    phys_addr_t phy_addr;
    ulong phy_size;

    ret = mt_mmz_get_phyaddr((mt_void *)vir, &phy_addr, &phy_size);
    if (ret == MT_SUCCESS)
        return (phys_addr_t)phy_addr;
    else
        return (phys_addr_t)0;
}

static mt_s32 mt_mpi_hash_update_internal(int ce_fd, mt_handle handle, mt_u8 *p_data, mt_u32 length)
{
    mt_s32 ret = MT_FAILURE;
    CMD_SHA_UPDATE_S cmd_update = { 0 };
    phys_addr_t p_src;
    mt_u8 *p_src_mmz = NULL;

    p_src = vir2phy(p_data);
    if (!p_src) {
        p_src_mmz = mt_mpi_cipher_malloc(length);
        if (!p_src_mmz)
            return ret;

        memcpy(p_src_mmz, p_data, length);
        p_src = vir2phy(p_src_mmz);
    }

    cmd_update.session = (mt_session)handle;
    cmd_update.p_msg = p_src;
    cmd_update.length = length;
    cmd_update.is_phy_addr = 1;

    MSS_LOCK(&g_m2m_mutex);

    ret = ioctl(ce_fd, CE_IOC_SHA_UPDATE, &cmd_update);
    if (ret != MT_SUCCESS) {
        MT_ERR_CIPHER("sha update fail, ret: 0x%x\n", cmd_update.ret);
        goto out;
    }

out:
    MSS_UNLOCK(&g_m2m_mutex);
    if (p_src_mmz)
        mt_mpi_cipher_free(p_src_mmz);

    ret = cmd_update.ret;
    return ret;
}

mt_s32 mt_mpi_hash_create(MT_CIPHER_HASH_TYPE_E hash_type, int ce_fd, mt_handle *p_handle)
{
    mt_s32 ret = MT_FAILURE;
    CMD_SHA_INIT_S cmd_init;
    CMD_SHA_CREATE_S cmd_create;
    CMD_SHA_DESTROY_S cmd_destroy;
    MT_CE_SHA_CTRL_S ctrl_s = { 0 };

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    switch (hash_type) {
    case MT_CIPHER_HASH_TYPE_SHA1:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SHA1;
        break;
    case MT_CIPHER_HASH_TYPE_SHA224:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SHA224;
        break;
    case MT_CIPHER_HASH_TYPE_SHA256:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SHA256;
        break;
    case MT_CIPHER_HASH_TYPE_SHA384:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SHA384;
        break;
    case MT_CIPHER_HASH_TYPE_SHA512:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SHA512;
        break;
    case MT_CIPHER_HASH_TYPE_SM3:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SM3;
        break;
    default:
        return MT_FAILURE;
    }

    ret = ioctl(ce_fd, CE_IOC_SHA_CREATE, &cmd_create);
    if (MT_SUCCESS == ret) {
        cmd_init.session = cmd_create.session;
    } else {
        return ret;
    }

    cmd_init.ctrl.sha_para.chan_num = MT_CE_CHANNEL_0;
    cmd_init.ctrl.sha_para.algo_mode = ctrl_s.sha_para.algo_mode;

    ret = ioctl(ce_fd, CE_IOC_SHA_INIT, &cmd_init);
    if (MT_SUCCESS != ret) {
        cmd_destroy.session = cmd_create.session;
        ioctl(ce_fd, CE_IOC_SHA_DESTROY, &cmd_destroy);
        return ret;
    }

    *p_handle = (mt_handle)cmd_init.session;

    return ret;
}


mt_s32 mt_mpi_hash_update(int ce_fd, mt_handle handle, mt_u8 *p_data, mt_u32 length)
{
    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (p_data == NULL || length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    return mt_mpi_hash_update_internal(ce_fd, handle, p_data, length);
}

static void mt_mpi_crypto_get_hash_attr(int ce_fd, mt_handle handle, MT_CE_SHA_CTRL_S *p_attr)
{
    CMD_SHA_ATTR_S cmd_attr = { 0 };

    cmd_attr.session = (mt_session)handle;
    ioctl(ce_fd, CE_IOC_SHA_ATTR, &cmd_attr);
    if (0 == cmd_attr.ret) {
        memcpy(p_attr, &cmd_attr.ctrl, sizeof(cmd_attr.ctrl));
    }
}

mt_s32 mt_mpi_hash_final(int ce_fd, mt_handle handle, mt_u8 *p_output_hash)
{
    mt_s32 ret = MT_FAILURE;
    CMD_SHA_DESTROY_S cmd_destroy;
    CMD_SHA_FINAL_S cmd_final = { 0 };
    MT_CE_SHA_CTRL_S hash_attr = { 0 };

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (!p_output_hash) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    mt_mpi_crypto_get_hash_attr(ce_fd, handle, &hash_attr);

    cmd_final.session = (mt_session)handle;
    memset(cmd_final.p_dgst, 0, sizeof(cmd_final.p_dgst));

    MSS_LOCK(&g_m2m_mutex);

    ioctl(ce_fd, CE_IOC_SHA_FINAL, &cmd_final);
    if (cmd_final.ret == 0) {
        memcpy(p_output_hash, cmd_final.p_dgst, hash_attr.sha_para.digest_size);
    } else {
        MT_ERR_CIPHER("Calculate hash value error!");
    }

    cmd_destroy.session = (mt_session)handle;
    ioctl(ce_fd, CE_IOC_SHA_DESTROY, &cmd_destroy);
    ret = cmd_destroy.ret;

    MSS_UNLOCK(&g_m2m_mutex);

    return ret;
}
static MT_CIPHER_HMAC_ATTS_S   g_hmac_attr_sha1 = {0};
mt_s32 mt_mpi_mac_create(MT_CIPHER_MAC_TYPE_E mac_type,
    MT_CIPHER_HMAC_ATTS_S *p_hmac_attr, int ce_fd, mt_handle *p_handle)
{
    mt_s32 ret = MT_FAILURE;
    CMD_SHA_INIT_S cmd_init;
    CMD_SHA_CREATE_S cmd_create;
    CMD_SHA_DESTROY_S cmd_destroy;
    MT_CE_SHA_CTRL_S ctrl_s = { 0 };
    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
#ifdef SUPPORT_SOFTWARE_HMAC
    if(mac_type == MT_CIPHER_MAC_TYPE_SHA1)
    {
	  if((p_hmac_attr != NULL)  && (p_hmac_attr->p_hmac_key != NULL))	
	  {
		 g_hmac_attr_sha1.p_hmac_key = (mt_u8 *)malloc(p_hmac_attr->key_len);
		 memcpy(g_hmac_attr_sha1.p_hmac_key,  p_hmac_attr->p_hmac_key, p_hmac_attr->key_len);
		 g_hmac_attr_sha1.key_len = p_hmac_attr->key_len;
		 hmac_sha1_init(&g_sym6_ctx, p_hmac_attr->p_hmac_key, p_hmac_attr->key_len); 
		 *p_handle = (ulong)g_hmac_attr_sha1.p_hmac_key;
		 return MT_SUCCESS;
	  }
    }
#endif	
    if ((p_hmac_attr->key_slot[0] != MT_CIPHER_KEYSLOT_INVALID)
            && (p_hmac_attr->key_slot[1] != MT_CIPHER_KEYSLOT_INVALID)) {
        if (p_hmac_attr->key_slot[1] != p_hmac_attr->key_slot[0] + 1) {
            MT_ERR_CIPHER("Two slots with consecutive serial numbers must be used for 256bit mac key!");
            return MT_FAILURE;
        }
    }
    switch (mac_type) {
    case MT_CIPHER_MAC_TYPE_SHA224:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_HMAC_SHA224;
        break;
    case MT_CIPHER_MAC_TYPE_SHA256:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_HMAC_SHA256;
        break;
    case MT_CIPHER_MAC_TYPE_SHA384:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_HMAC_SHA384;
        break;
    case MT_CIPHER_MAC_TYPE_SHA512:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_HMAC_SHA512;
        break;
    case MT_CIPHER_MAC_TYPE_SM3:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_HMAC_SM3;
        break;
    case MT_CIPHER_MAC_TYPE_CMAC_AES128:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_CMAC_AES128;
        break;
    case MT_CIPHER_MAC_TYPE_CMAC_SM4:
        ctrl_s.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_CMAC_SM4;
        break;
    default:
        return MT_FAILURE;
    }

    ret = ioctl(ce_fd, CE_IOC_MAC_CREATE, &cmd_create);
    if (MT_SUCCESS == ret) {
        cmd_init.session = cmd_create.session;
    } else {
        return ret;
    }

    cmd_init.ctrl.sha_para.chan_num = MT_CE_CHANNEL_0;
    cmd_init.ctrl.sha_para.algo_mode = ctrl_s.sha_para.algo_mode;
    cmd_init.ctrl.sha_para.key_size = p_hmac_attr->key_len;
    cmd_init.ctrl.sha_para.key_slot = p_hmac_attr->key_slot[0];
    if (p_hmac_attr->p_hmac_key)
        memcpy(cmd_init.ctrl.sha_para.hmac_key, p_hmac_attr->p_hmac_key, p_hmac_attr->key_len);

    ret = ioctl(ce_fd, CE_IOC_MAC_INIT, &cmd_init);
    if (MT_SUCCESS != ret) {
        cmd_destroy.session = cmd_create.session;
        ioctl(ce_fd, CE_IOC_MAC_DESTROY, &cmd_destroy);
        return ret;
    }

    *p_handle = (mt_handle)cmd_init.session;

    return ret;
}

mt_s32 mt_mpi_mac_update(int ce_fd, mt_handle handle, mt_u8 *p_data, mt_u32 length)
{
    mt_s32 ret = MT_FAILURE;
    phys_addr_t p_src = (phys_addr_t)0;
    mt_u8 *p_src_mmz = NULL;
    CMD_SHA_UPDATE_S cmd_update = { 0 };

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }
    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (p_data == NULL || length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }
    if((g_hmac_attr_sha1.p_hmac_key == handle) &&  (g_hmac_attr_sha1.key_len > 0))
    {
	    // soft hmac
	    hmac_sha1_update(&g_sym6_ctx, p_data, length);
	    return 0;
	 
    }
    p_src = vir2phy(p_data);
    if (!p_src) {
        p_src_mmz = mt_mpi_cipher_malloc(length);
        if (!p_src_mmz)
            return ret;

        memcpy(p_src_mmz, p_data, length);
        p_src = vir2phy(p_src_mmz);
    }

    cmd_update.session = (mt_session)handle;
    cmd_update.p_msg = p_src;
    cmd_update.length = length;
    cmd_update.is_phy_addr = 1;

    MSS_LOCK(&g_m2m_mutex);

    ret = ioctl(ce_fd, CE_IOC_MAC_UPDATE, &cmd_update);
    if (ret != MT_SUCCESS) {
        MT_ERR_CIPHER("mac update fail, ret: 0x%x\n", cmd_update.ret);
        goto out;
    }

out:
    MSS_UNLOCK(&g_m2m_mutex);
    if (p_src_mmz)
        mt_mpi_cipher_free(p_src_mmz);

    ret = cmd_update.ret;
    return ret;
}

mt_s32 mt_mpi_mac_final(int ce_fd, mt_handle handle, mt_u8 *p_output_mac)
{
    mt_s32 ret = MT_FAILURE;
    CMD_SHA_DESTROY_S cmd_destroy;
    CMD_SHA_FINAL_S cmd_final = { 0 };
    MT_CE_SHA_CTRL_S hash_attr = { 0 };

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (!p_output_mac) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }
    if((g_hmac_attr_sha1.p_hmac_key == handle) &&  (g_hmac_attr_sha1.key_len > 0))
    {
    		uint8_t digest[SHA1_DIGEST_SIZE];
    		//char hex_digest[SHA1_DIGEST_SIZE * 2 + 1];

    		if(g_hmac_attr_sha1.p_hmac_key != NULL){
			free(g_hmac_attr_sha1.p_hmac_key);
    			g_hmac_attr_sha1.p_hmac_key = NULL;
			g_hmac_attr_sha1.key_len = 0;
    		}
		// 计算最终结果
    		hmac_sha1_final(&g_sym6_ctx, digest);
    		memcpy(p_output_mac, digest, 20);
    		// 输出十六进制结果
    		//bytes_to_hex(digest, SHA1_DIGEST_SIZE, hex_digest);
    		//printf("HMAC-SHA1结果: %s\n", hex_digest); 
    		
		return  MT_SUCCESS;
    }
    mt_mpi_crypto_get_hash_attr(ce_fd, handle, &hash_attr);

    cmd_final.session = (mt_session)handle;
    memset(cmd_final.p_dgst, 0, sizeof(cmd_final.p_dgst));

    MSS_LOCK(&g_m2m_mutex);

    ioctl(ce_fd, CE_IOC_MAC_FINAL, &cmd_final);
    if (cmd_final.ret == 0) {
        memcpy(p_output_mac, cmd_final.p_dgst, hash_attr.sha_para.digest_size);
    } else {
        MT_ERR_CIPHER("Calculate mac value error!");
    }

    cmd_destroy.session = (mt_session)handle;
    ioctl(ce_fd, CE_IOC_MAC_DESTROY, &cmd_destroy);
    ret = cmd_destroy.ret;

    MSS_UNLOCK(&g_m2m_mutex);

    return ret;
}

/**
 * @brief
 *      AES/TDES processing
 *
 * @param key_ladder_handle
 * @param p_cipher
 *
 * @return
 */
mt_s32 mt_mpi_crypto_create(unsigned int channel, int ce_fd, mt_handle *p_crypto)
{
    mt_s32 ret = MT_FAILURE;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    CMD_ADES_CREATE_S cmd_create = {
        .channel = channel,
    };

    ret = ioctl(ce_fd, CE_IOC_ADES_CREATE, &cmd_create);
    if (MT_SUCCESS == ret) {
        *p_crypto = (mt_handle)cmd_create.session;
    }

    ret = cmd_create.ret;
    return ret;
}

mt_s32 mt_mpi_crypto_destroy(int ce_fd, mt_handle crypto)
{
    mt_s32 ret = MT_FAILURE;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    CMD_ADES_DESTROY_S cmd_destroy;

    cmd_destroy.session = (mt_session)crypto;

    ioctl(ce_fd, CE_IOC_ADES_DESTROY, &cmd_destroy);
    ret = cmd_destroy.ret;

    return ret;
}

static MT_CE_ADES_WORK_MODE_E to_ce_work_mode(MT_CIPHER_WORK_MODE_E mode)
{
    switch (mode) {
        case MT_CIPHER_WORK_MODE_ECB:
            return MT_CE_ADES_WORK_MODE_ECB;
        case MT_CIPHER_WORK_MODE_CBC:
            return MT_CE_ADES_WORK_MODE_CBC;
        case MT_CIPHER_WORK_MODE_CTR:
            return MT_CE_AES_WORK_MODE_CTR; //AES only
        case MT_CIPHER_WORK_MODE_CBCDVS042:
            return MT_CE_ADES_WORK_MODE_CBCDVS042;
        case MT_CIPHER_WORK_MODE_CBCCTS:
            return MT_CE_ADES_WORK_MODE_CBCCTS;
        case MT_CIPHER_WORK_MODE_CFB:
            return MT_CE_ADES_WORK_MODE_CFB;
        case MT_CIPHER_WORK_MODE_OFB:
            return MT_CE_ADES_WORK_MODE_OFB;
        case MT_CIPHER_WORK_MODE_CBCS:
            return MT_CE_ADES_WORK_MODE_CBCS;
        case MT_CIPHER_WORK_MODE_CENS:
            return MT_CE_ADES_WORK_MODE_CENS;
        default:
            return MT_CE_ADES_WORK_MODE_UNKNOWN;
    }
}

static mt_void mpi_config_convert(MT_CE_ADES_CTRL_S *p_ce_ctrl, MT_CIPHER_CTRL_S *p_ctrl, mt_u32 key_slot)
{
    if (p_ctrl->operation == MT_CIPHER_OPERATION_DECRYPT)
        p_ce_ctrl->operation = MT_CE_ADES_OPERATION_DECRYPT;
    else
        p_ce_ctrl->operation = MT_CE_ADES_OPERATION_ENCRYPT;

    switch (p_ctrl->algorithm) {
        case MT_CIPHER_ALG_DES:
            p_ce_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_DES;
            break;
        case MT_CIPHER_ALG_TDES:
            p_ce_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_TDES_ABA; /* ABA only */
            break;
        case MT_CIPHER_ALG_AES:
            p_ce_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_AES128;
            break;
        case MT_CIPHER_ALG_AES256:
            p_ce_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_AES256;
            break;
        case MT_CIPHER_ALG_SM4:
            p_ce_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_SM4;
            break;
        default:
            break;
    }

    p_ce_ctrl->ades_para.work_mode = to_ce_work_mode(p_ctrl->work_mode);
    p_ce_ctrl->ades_para.key_slot = key_slot;
    p_ce_ctrl->ades_para.crypt_blocks = p_ctrl->cbcs_params.crypt_blocks;
    p_ce_ctrl->ades_para.skip_blocks = p_ctrl->cbcs_params.skip_blocks;
}

mt_s32 mt_mpi_crypto_config(int ce_fd, mt_handle crypto, MT_CIPHER_CTRL_S *p_ctrl, mt_u32 key_slot)
{
    mt_s32 ret = MT_FAILURE;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("invalid cipher fd");
        return MT_FAILURE;
    }

    CMD_ADES_CTRL_S cmd_ctrl;

    cmd_ctrl.session = (mt_session)crypto;
    mpi_config_convert(&(cmd_ctrl.ctrl), p_ctrl, key_slot);

    ioctl(ce_fd, CE_IOC_ADES_CONFIG, &cmd_ctrl);
    ret = cmd_ctrl.ret;

    return ret;
}

mt_s32 mt_mpi_crypto_process(int ce_fd, mt_handle crypto, mt_u8 *p_src_addr, mt_u8 *p_dest_addr, mt_u32 length)
{
    mt_s32 ret = MT_FAILURE;
    CMD_ADES_PROCESS_S cmd_process;
    phys_addr_t p_src_phy_addr;
    phys_addr_t p_dst_phy_addr;
#if defined (CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    mt_u8 *p_src_mmz_addr = NULL;
    mt_u8 *p_dst_mmz_addr = NULL;
#endif

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    p_src_phy_addr = vir2phy(p_src_addr);
    p_dst_phy_addr = vir2phy(p_dest_addr);
    if (!p_src_phy_addr || !p_dst_phy_addr)
        MT_ERR_CIPHER("src[%p] & dst[%p] is not mmz address!\n", p_src_addr, p_dest_addr);

    cmd_process.session = (mt_session)crypto;
    if (p_src_phy_addr && p_dst_phy_addr) {
        cmd_process.p_src_addr = p_src_phy_addr;
        cmd_process.p_dst_addr = p_dst_phy_addr;
        cmd_process.is_phy_addr = 1;
    } else {
#if defined (CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
        if (!p_src_phy_addr) {
            p_src_mmz_addr = mt_mpi_cipher_malloc(length);
            if (!p_src_mmz_addr)
                goto out;

            memcpy(p_src_mmz_addr, p_src_addr, length);
            cmd_process.p_src_addr = vir2phy(p_src_mmz_addr);
        } else {
            cmd_process.p_src_addr = p_src_phy_addr;
        }

        if (!p_dst_phy_addr) {
            p_dst_mmz_addr = mt_mpi_cipher_malloc(length);
            if (!p_dst_mmz_addr)
                goto out;

            cmd_process.p_dst_addr = vir2phy(p_dst_mmz_addr);
        } else {
            cmd_process.p_dst_addr = p_dst_phy_addr;
        }

        cmd_process.is_phy_addr = 1;
#else
        cmd_process.p_src_addr = p_src_addr;
        cmd_process.p_dst_addr = p_dest_addr;
        cmd_process.is_phy_addr = 0;
#endif
    }
    cmd_process.length = length;
    cmd_process.clear_length = 0;

    MSS_LOCK(&g_m2m_mutex);

    ioctl(ce_fd, CE_IOC_ADES_PROCESS, &cmd_process);
    ret = cmd_process.ret;

#if defined (CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
out:
    if (p_src_mmz_addr)
        mt_mpi_cipher_free(p_src_mmz_addr);

    if (p_dst_mmz_addr) {
        memcpy(p_dest_addr, p_dst_mmz_addr, length);
        mt_mpi_cipher_free(p_dst_mmz_addr);
    }
#endif

    MSS_UNLOCK(&g_m2m_mutex);

    return ret;
}

mt_s32 mt_mpi_crypto_process_phy(int ce_fd, mt_handle crypto, phys_addr_t p_src_phyaddr, phys_addr_t p_dest_phyaddr, mt_u32 clear_length, mt_u32 cipher_length)
{
    mt_s32 ret = MT_FAILURE;
    CMD_ADES_PROCESS_S cmd_process;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

	if (!p_src_phyaddr || !p_dest_phyaddr) {
		return MT_FAILURE;
	}

    cmd_process.session = (mt_session)crypto;
	cmd_process.p_src_addr = p_src_phyaddr;
	cmd_process.p_dst_addr = p_dest_phyaddr;
	cmd_process.is_phy_addr = 1;
    cmd_process.clear_length = clear_length;
    cmd_process.length = cipher_length;

    MSS_LOCK(&g_m2m_mutex);

    ioctl(ce_fd, CE_IOC_ADES_PROCESS, &cmd_process);
    ret = cmd_process.ret;

    MSS_UNLOCK(&g_m2m_mutex);

    return ret;
}

mt_s32 mt_mpi_cipher_crypto_async_request(int ce_fd, mt_handle crypto, const mt_u8 *src, mt_u8 *dst, mt_u32 count, mt_u32 clear_length, mt_u32 protected_length)
{
    mt_s32 ret = MT_FAILURE;
    CMD_ADES_ASYNC_REQUEST_S cmd_async_request;
    phys_addr_t p_src_phy_addr;
    phys_addr_t p_dst_phy_addr;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    p_src_phy_addr = vir2phy(src);
    p_dst_phy_addr = vir2phy(dst);

    if (!p_src_phy_addr || !p_dst_phy_addr)
        return MT_FAILURE;

    cmd_async_request.session = (mt_session)crypto;
    cmd_async_request.src = p_src_phy_addr;
    cmd_async_request.dst = p_dst_phy_addr;
    cmd_async_request.is_phy_addr = 1;
    cmd_async_request.count = count;
    cmd_async_request.clear_length = clear_length;
    cmd_async_request.protected_length = protected_length;

    MSS_LOCK(&g_m2m_mutex);

    ioctl(ce_fd, CE_IOC_ADES_ASYNC_REQUEST, &cmd_async_request);
    ret = cmd_async_request.ret;

    MSS_UNLOCK(&g_m2m_mutex);

    return ret;
}

mt_s32 mt_mpi_cipher_crypto_async_start(int ce_fd, mt_handle crypto)
{
    mt_s32 ret = MT_FAILURE;
    CMD_ADES_ASYNC_START_S cmd_async_start;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    cmd_async_start.session = (mt_session)crypto;

    MSS_LOCK(&g_m2m_mutex);

    ioctl(ce_fd, CE_IOC_ADES_ASYNC_START, &cmd_async_start);
    ret = cmd_async_start.ret;

    MSS_UNLOCK(&g_m2m_mutex);

    return ret;
}

mt_s32 mt_mpi_cipher_crypto_async_wait(int ce_fd, mt_handle crypto)
{
    mt_s32 ret = MT_FAILURE;
    CMD_ADES_ASYNC_WAIT_S cmd_async_wait;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    cmd_async_wait.session = (mt_session)crypto;

    MSS_LOCK(&g_m2m_mutex);

    ioctl(ce_fd, CE_IOC_ADES_ASYNC_WAIT, &cmd_async_wait);
    ret = cmd_async_wait.ret;

    MSS_UNLOCK(&g_m2m_mutex);

    return ret;
}

mt_s32 mt_mpi_crypto_rsa_create(int ce_fd, mt_handle *p_rsa_handle)
{

    mt_s32 ret = MT_FAILURE;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    CMD_RSA_CREATE_S cmd_create;

    ret = ioctl(ce_fd, CE_IOC_RSA_CREATE, &cmd_create);
    if (MT_SUCCESS == ret) {
        *p_rsa_handle = (mt_handle)cmd_create.session;
    }
    ret = cmd_create.ret;

    return ret;
}

mt_s32 mt_mpi_crypto_rsa_destroy(int ce_fd, mt_handle handle)
{
    mt_s32 ret = MT_FAILURE;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    CMD_RSA_DESTROY_S cmd_destroy;

    cmd_destroy.session = (mt_session)handle;
    ioctl(ce_fd, CE_IOC_RSA_DESTROY, &cmd_destroy);

    ret = cmd_destroy.ret;

    return ret;
}

mt_s32 mt_mpi_crypto_rsa_config(int ce_fd, mt_handle rsa_handle, MT_CIPHER_RSA_CTRL_S *p_ctrl)
{
    mt_s32 ret = MT_FAILURE;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("invalid cipher fd");
        return MT_FAILURE;
    }

    CMD_RSA_CTRL_S rsa_ctrl;
    memset(&rsa_ctrl, 0, sizeof(CMD_RSA_CTRL_S));
    rsa_ctrl.session = (mt_session)rsa_handle;

    memcpy(rsa_ctrl.ctrl.rsa_para.p_m, p_ctrl->rsa_para.p_m, p_ctrl->rsa_para.key_length);
    rsa_ctrl.ctrl.rsa_para.key_length = p_ctrl->rsa_para.key_length;

    memcpy(rsa_ctrl.ctrl.rsa_para.p_e + p_ctrl->rsa_para.key_length - p_ctrl->rsa_para.exp_length,
        p_ctrl->rsa_para.p_e, p_ctrl->rsa_para.exp_length);
    rsa_ctrl.ctrl.rsa_para.exp_length = p_ctrl->rsa_para.key_length;

    ioctl(ce_fd, CE_IOC_RSA_CONFIG, &rsa_ctrl);
    ret = rsa_ctrl.ret;

    return ret;
}

mt_s32 mt_mpi_crypto_rsa_process(int ce_fd, mt_handle handle, mt_u8 *p_src_addr, mt_u8 *p_dest_addr, mt_u32 length)
{
    mt_s32 ret = MT_FAILURE;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    CMD_RSA_PROCESS_S cmd_process;

    memset(&cmd_process, 0, sizeof(CMD_RSA_PROCESS_S));
    cmd_process.session = (mt_session)handle;
    cmd_process.src_length = length;
    cmd_process.p_src_addr = p_src_addr;
    cmd_process.p_dst_addr = p_dest_addr;

    ioctl(ce_fd, CE_IOC_RSA_PROCESS, &cmd_process);
    ret = cmd_process.ret;

    return ret;
}

mt_s32 mt_mpi_crypto_ts_create(unsigned int channel, int ce_fd, mt_handle *p_ts_handle)
{
    mt_s32 ret = MT_FAILURE;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    CMD_TS_CREATE_S cmd_create = {
        .channel = channel,
    };

    ioctl(ce_fd, CE_IOC_TS_CREATE, &cmd_create);
    if (MT_SUCCESS == cmd_create.ret) {
        *p_ts_handle = (mt_handle)cmd_create.session;
    }

    ret = cmd_create.ret;

    return ret;
}

mt_s32 mt_mpi_crypto_ts_destroy(int ce_fd, mt_handle handle)
{
    mt_s32 ret = MT_FAILURE;
    CMD_TS_DESTROY_S cmd_destroy;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    cmd_destroy.session = (mt_session)handle;
    ioctl(ce_fd, CE_IOC_TS_DESTROY, &cmd_destroy);
    ret = cmd_destroy.ret;

    return ret;
}

mt_s32 mt_mpi_crypto_ts_config(int ce_fd, mt_handle handle, MT_CE_TS_CTRL_S *p_ctrl)
{
    mt_s32 ret = MT_FAILURE;
    CMD_TS_CTRL_S cmd_ctrl;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    cmd_ctrl.session = (mt_session)handle;
    memcpy(&(cmd_ctrl.ctrl), p_ctrl, sizeof(MT_CE_TS_CTRL_S));
    ioctl(ce_fd, CE_IOC_TS_CONFIG, &cmd_ctrl);
    ret = cmd_ctrl.ret;

    return ret;
}

mt_s32 mt_mpi_crypto_ts_process(int ce_fd, mt_handle handle, phys_addr_t p_src_addr, phys_addr_t p_dst_addr, mt_u32 length)
{
    mt_s32 ret = MT_FAILURE;
    CMD_TS_PROCESS_S cmd_process;

	if (!p_src_addr || !p_dst_addr) {
		return MT_FAILURE;
	}
    cmd_process.is_phy_addr = 1;
    cmd_process.session = (mt_session)handle;
    cmd_process.p_src_addr = p_src_addr;
    cmd_process.p_dst_addr = p_dst_addr;
    cmd_process.length = length;

    MSS_LOCK(&g_m2m_mutex);

    ioctl(ce_fd, CE_IOC_TS_PROCESS, &cmd_process);
    ret = cmd_process.ret;

    MSS_UNLOCK(&g_m2m_mutex);

    return ret;
}

/*
 * BN functions
 */
mt_s32 mt_mpi_crypto_bn_create(int ce_fd, mt_handle *p_bn_handle)
{
    mt_s32 ret = MT_FAILURE;
    CMD_BN_CREATE_S cmd_create;

    ioctl(ce_fd, CE_IOC_BN_CREATE, &cmd_create);
    if (MT_SUCCESS == cmd_create.ret) {
        *p_bn_handle = (mt_handle)cmd_create.session;
    }

    ret = cmd_create.ret;
    return ret;
}

mt_s32 mt_mpi_crypto_bn_destroy(int ce_fd, mt_handle handle)
{
    mt_s32 ret = MT_FAILURE;
    CMD_BN_DESTROY_S cmd_destroy;

    cmd_destroy.session = (mt_session)handle;

    ioctl(ce_fd, CE_IOC_BN_DESTROY, &cmd_destroy);

    ret = cmd_destroy.ret;
    return ret;
}

mt_s32 mt_mpi_crypto_bn_mod_mod(int ce_fd, mt_handle handle, mt_u8 *r, mt_u8 *d,
        mt_u8 *m, mt_u32 d_length, mt_u32 m_length)
{
    CMD_BN_MOD_MOD_S cmd_mod_mod;
    mt_u32 d_vl, m_vl;
    mt_u32 op_len;

    if (r == NULL || d == NULL || m == NULL)
        return MT_FAILURE;

    d_vl = mt_bn_len(d, d_length);
    m_vl = mt_bn_len(m, m_length);

    /* not support  len(m) > 256 */
    if (m_vl > 256)
        return MT_FAILURE;

    /* if len(d) > len(m) + 1, need divide d to several parts */
    if (d_vl > m_vl+1) {
        return mt_bn_mod(ce_fd, handle, r, d+d_length-d_vl, m+m_length-m_vl, d_vl, m_vl);
    }

    if (d_vl > m_vl)
        op_len = d_vl;
    else
        op_len = m_vl;

    op_len = (op_len + 0x3u) & ~0x3u;

    memset(&cmd_mod_mod, 0, sizeof(cmd_mod_mod));
    cmd_mod_mod.session = (mt_session)handle;

    memcpy(cmd_mod_mod.d+op_len-d_length, d, d_length);
    memcpy(cmd_mod_mod.m+op_len-m_length, m, m_length);
    cmd_mod_mod.d_length = op_len;
    cmd_mod_mod.m_length = op_len;

    ioctl(ce_fd, CE_IOC_BN_MOD_MOD, &cmd_mod_mod);

    if (MT_SUCCESS == cmd_mod_mod.ret) {
        if (m_length > op_len) {
            memset(r, 0, m_length-op_len);
            memcpy(r+m_length-op_len, cmd_mod_mod.r, op_len);
        } else {
            memcpy(r, cmd_mod_mod.r+op_len-m_length, m_length);
        }
    }

    return cmd_mod_mod.ret;
}

mt_s32 mt_mpi_crypto_bn_mod_mul(int ce_fd, mt_handle handle, mt_u8 *r,
        mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length)
{
    CMD_BN_MOD_MUL_S cmd_mod_mul;

    if (r == NULL || a == NULL || b == NULL || m == NULL)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (a_length == 0 || b_length == 0 || m_length == 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (m_length > 256)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if(mt_bn_is_odd(m, m_length) != 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    memset(&cmd_mod_mul, 0, sizeof(cmd_mod_mul));
    cmd_mod_mul.session = (mt_session)handle;

    memcpy(cmd_mod_mul.m, m, m_length);
    cmd_mod_mul.m_length = m_length;

    /* op1 = a mod m */
    if (a_length != m_length || mt_bn_ucmp(a, a_length, m, m_length) >= 0) {
        mt_mpi_crypto_bn_mod_mod(ce_fd, handle, cmd_mod_mul.a, a, m, a_length, m_length);
        cmd_mod_mul.a_length = m_length;
    } else {
        memcpy(cmd_mod_mul.a, a, a_length);
        cmd_mod_mul.a_length = a_length;
    }

    /* op2 = b mod m */
    if (b_length != m_length || mt_bn_ucmp(b, b_length, m, m_length) >= 0) {
        mt_mpi_crypto_bn_mod_mod(ce_fd, handle, cmd_mod_mul.b, b, m, b_length, m_length);
        cmd_mod_mul.b_length = m_length;
    } else {
        memcpy(cmd_mod_mul.b, b, b_length);
        cmd_mod_mul.b_length = b_length;
    }

    ioctl(ce_fd, CE_IOC_BN_MOD_MUL, &cmd_mod_mul);
    if (MT_SUCCESS == cmd_mod_mul.ret) {
        memcpy(r, cmd_mod_mul.r, m_length);
    }

    return cmd_mod_mul.ret;
}

mt_s32 mt_mpi_crypto_bn_mod_add(int ce_fd, mt_handle handle, mt_u8 *r,
        mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length)
{
    CMD_BN_MOD_ADD_S cmd_mod_add;

    if (r == NULL || a == NULL || b == NULL || m == NULL)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (a_length == 0 || b_length == 0 || m_length == 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (m_length > 256)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if(mt_bn_is_odd(m, m_length) != 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    memset(&cmd_mod_add, 0, sizeof(cmd_mod_add));
    cmd_mod_add.session = (mt_session)handle;

    memcpy(cmd_mod_add.m, m, m_length);
    cmd_mod_add.m_length = m_length;

    /* op1 = a mod m */
    if (a_length != m_length || mt_bn_ucmp(a, a_length, m, m_length) >= 0) {
        mt_mpi_crypto_bn_mod_mod(ce_fd, handle, cmd_mod_add.a, a, m, a_length, m_length);
        cmd_mod_add.a_length = m_length;
    } else {
        memcpy(cmd_mod_add.a, a, a_length);
        cmd_mod_add.a_length = a_length;
    }

    /* op2 = b mod m */
    if (b_length != m_length || mt_bn_ucmp(b, b_length, m, m_length) >= 0) {
        mt_mpi_crypto_bn_mod_mod(ce_fd, handle, cmd_mod_add.b, b, m, b_length, m_length);
        cmd_mod_add.b_length = m_length;
    } else {
        memcpy(cmd_mod_add.b, b, b_length);
        cmd_mod_add.b_length = b_length;
    }

    ioctl(ce_fd, CE_IOC_BN_MOD_ADD, &cmd_mod_add);

    if (MT_SUCCESS == cmd_mod_add.ret) {
        memcpy(r, cmd_mod_add.r, m_length);
    }

    return cmd_mod_add.ret;
}

mt_s32 mt_mpi_crypto_bn_mod_sub(int ce_fd, mt_handle handle, mt_u8 *r,
        mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length)
{
    CMD_BN_MOD_SUB_S cmd_mod_sub;

    if (r == NULL || a == NULL || b == NULL || m == NULL)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (a_length == 0 || b_length == 0 || m_length == 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (m_length > 256)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if(mt_bn_is_odd(m, m_length) != 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    memset(&cmd_mod_sub, 0, sizeof(cmd_mod_sub));
    cmd_mod_sub.session = (mt_session)handle;

    memcpy(cmd_mod_sub.m, m, m_length);
    cmd_mod_sub.m_length = m_length;

    /* op1 = a mod m */
    if (a_length != m_length || mt_bn_ucmp(a, a_length, m, m_length) >= 0) {
        mt_mpi_crypto_bn_mod_mod(ce_fd, handle, cmd_mod_sub.a, a, m, a_length, m_length);
        cmd_mod_sub.a_length = m_length;
    } else {
        memcpy(cmd_mod_sub.a, a, a_length);
        cmd_mod_sub.a_length = a_length;
    }

    /* op2 = b mod m */
    if (b_length != m_length || mt_bn_ucmp(b, b_length, m, m_length) >= 0) {
        mt_mpi_crypto_bn_mod_mod(ce_fd, handle, cmd_mod_sub.b, b, m, b_length, m_length);
        cmd_mod_sub.b_length = m_length;
    } else {
        memcpy(cmd_mod_sub.b, b, b_length);
        cmd_mod_sub.b_length = b_length;
    }

    ioctl(ce_fd, CE_IOC_BN_MOD_SUB, &cmd_mod_sub);

    if (MT_SUCCESS == cmd_mod_sub.ret) {
        memcpy(r, cmd_mod_sub.r, m_length);
    }

    return cmd_mod_sub.ret;
}

mt_s32 mt_mpi_crypto_bn_mod_inv(int ce_fd, mt_handle handle, mt_u8 *r,
        mt_u8 *d, mt_u8 *m, mt_u32 d_length, mt_u32 m_length)
{
    CMD_BN_MOD_INV_S cmd_mod_inv;

    if (r == NULL || d == NULL || m == NULL)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (d_length == 0 || m_length == 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (m_length > 256)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if(mt_bn_is_odd(m, m_length) != 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    memset(&cmd_mod_inv, 0, sizeof(cmd_mod_inv));
    cmd_mod_inv.session = (mt_session)handle;

    memcpy(cmd_mod_inv.m, m, m_length);
    cmd_mod_inv.m_length = m_length;

    /* op1 = d mod m */
    if (d_length != m_length || mt_bn_ucmp(d, d_length, m, m_length) >= 0) {
        mt_mpi_crypto_bn_mod_mod(ce_fd, handle, cmd_mod_inv.d, d, m, d_length, m_length);
        cmd_mod_inv.d_length = m_length;
    } else {
        memcpy(cmd_mod_inv.d, d, d_length);
        cmd_mod_inv.d_length = d_length;
    }

    ioctl(ce_fd, CE_IOC_BN_MOD_INV, &cmd_mod_inv);

    if (MT_SUCCESS == cmd_mod_inv.ret) {
        memcpy(r, cmd_mod_inv.r, m_length);
    }

    return cmd_mod_inv.ret;
}

mt_s32 mt_mpi_crypto_bn_mod_exp(int ce_fd, mt_handle handle, mt_u8 *r,
        mt_u8 *a, mt_u8 *p, mt_u8 *m, mt_u32 a_length, mt_u32 p_length, mt_u32 m_length)
{
    CMD_BN_MOD_EXP_S cmd_mod_exp;
    mt_u32 i;

    if (r == NULL || a == NULL || p == NULL || m == NULL)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (a_length == 0 || p_length == 0 || m_length == 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (m_length > 256)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if(mt_bn_is_odd(m, m_length) != 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    memset(&cmd_mod_exp, 0, sizeof(cmd_mod_exp));
    cmd_mod_exp.session = (mt_session)handle;

    memcpy(cmd_mod_exp.m, m, m_length);
    cmd_mod_exp.m_length = m_length;

    /* op1 = a mod m */
    if (a_length != m_length || mt_bn_ucmp(a, a_length, m, m_length) >= 0) {
        mt_mpi_crypto_bn_mod_mod(ce_fd, handle, cmd_mod_exp.a, a, m, a_length, m_length);
        cmd_mod_exp.a_length = m_length;
    } else {
        memcpy(cmd_mod_exp.a, a, a_length);
        cmd_mod_exp.a_length = a_length;
    }

    /* op2 = p */
    if (p_length == m_length) {
        memcpy(cmd_mod_exp.p, p, m_length);
        cmd_mod_exp.p_length = m_length;
    } else if (p_length < m_length) {
        memset(cmd_mod_exp.p, 0, m_length - p_length);
        memcpy(cmd_mod_exp.p + m_length - p_length, p, p_length);
        cmd_mod_exp.p_length = m_length;
    } else { // p_length > m_length
        for (i = 0; i < p_length - m_length; i++) {
            if (p[i] != 0) {
                return CE_BN_MOD_MUL_INPUT_ERROR;
            }
        }
        memcpy(cmd_mod_exp.p, p + p_length - m_length, m_length);
        cmd_mod_exp.p_length = m_length;
    }

    ioctl(ce_fd, CE_IOC_BN_MOD_EXP, &cmd_mod_exp);

    if (MT_SUCCESS == cmd_mod_exp.ret) {
        memcpy(r, cmd_mod_exp.r, m_length);
    }

    return cmd_mod_exp.ret;
}

mt_s32 mt_mpi_crypto_ec_point_create(int ce_fd, mt_handle *p_ecp_handle)
{
    CMD_EC_POINT_CREATE_S cmd_create;

    ioctl(ce_fd, CE_IOC_EC_POINT_CREATE, &cmd_create);
    if (MT_SUCCESS == cmd_create.ret) {
        *p_ecp_handle = (mt_handle)cmd_create.session;
    }

    return cmd_create.ret;
}

mt_s32 mt_mpi_crypto_ec_point_destroy(int ce_fd, mt_handle handle)
{
    CMD_EC_POINT_DESTROY_S cmd_destroy;

    cmd_destroy.session = (mt_session)handle;

    ioctl(ce_fd, CE_IOC_EC_POINT_DESTROY, &cmd_destroy);

    return cmd_destroy.ret;
}

mt_s32 mt_mpi_crypto_ec_point_mul(int ce_fd, mt_handle handle, MT_CIPHER_EC_PARAMS_S xParams, MT_CIPHER_EC_POINT_S *r, const mt_u8 *g_scalar, MT_CIPHER_EC_POINT_S *p_point, const mt_u8 *p_scalar)
{
    CMD_EC_POINT_MUL_S cmd_ec_point_mul;
    mt_u32 CaseID = 0;

    if (xParams.GX && xParams.GY && g_scalar) {
        if (xParams.keySize == 15 || xParams.keySize == 21 || xParams.keySize == 29) {
            if (g_scalar[0] != 0) {
                CaseID = 1;
            } else {
                CaseID = 3;
            }
        } else {
            CaseID = 5;
        }
    } else if (p_point && p_scalar) {
        if (xParams.keySize == 15 || xParams.keySize == 21 || xParams.keySize == 29) {
            if (p_scalar[0] != 0) {
                CaseID = 2;
            } else {
                CaseID = 4;
            }
        } else {
            CaseID = 6;
        }
    } else {
        return CE_EC_POINT_MUL_INPUT_ERROR;
    }

    if (CaseID == 1 || CaseID == 2) {
        mt_u32 DRV_LENGTH;
        if (xParams.keySize == 21) {
            DRV_LENGTH = 20;
        } else if (xParams.keySize == 29) {
            DRV_LENGTH = 28;
        } else {
            DRV_LENGTH = 14;
        }

        mt_u8 r1X[28 /*DRV_LENGTH*/] = { 0 };
        mt_u8 r1Y[28 /*DRV_LENGTH*/] = { 0 };
        mt_u8 r2X[28 /*DRV_LENGTH*/] = { 0 };
        mt_u8 r2Y[28 /*DRV_LENGTH*/] = { 0 };
        MT_CE_EC_POINT_S r1 = { r1X, r1Y };
        MT_CE_EC_POINT_S r2 = { r2X, r2Y };
        mt_u8 K1[28 /*DRV_LENGTH*/] = { 0 };
        mt_u8 K2[28 /*DRV_LENGTH*/] = { 0 };
        mt_u8 K3[28 /*DRV_LENGTH*/] = { 0 };
        //========================g_scalar=K1+K2=========================
        mt_u8 *r_bin = K1;
        mt_u32 r_bytes, i, flag;

        if (CaseID == 1) {
            r_bytes = mt_bn_rshift1(r_bin, g_scalar, xParams.keySize);
            flag = (g_scalar[xParams.keySize - 1] & 0x1) ? 1 : 0;
        }
        else { //(CaseID == 2)
            r_bytes = mt_bn_rshift1(r_bin, p_scalar, xParams.keySize);
            flag = (p_scalar[xParams.keySize - 1] & 0x1) ? 1 : 0;
        }

        if (r_bytes < DRV_LENGTH) //patch
        {
            for (i = 1; i <= r_bytes; i++) {
                r_bin[DRV_LENGTH - i] = r_bin[r_bytes - i];
            }
            memset(r_bin, 0, (DRV_LENGTH - r_bytes));
        }
        memcpy(K2, K1, DRV_LENGTH);

        if (flag) {
            for (i = DRV_LENGTH; i > 0; i--) {
                if (K2[i - 1] != 0xff) {
                    K2[i - 1] = (mt_u8)(K2[i - 1] + 1);
                    break;
                }
            }

            for (; i < DRV_LENGTH; i++) {
                K2[i] = 0;
            }
        } else {
            K3[DRV_LENGTH - 1] = 2;
        }

        //drv_ce_data_print("K1:", K1, DRV_LENGTH, 16);
        //drv_ce_data_print("K2:", K2, DRV_LENGTH, 16);
        //drv_ce_data_print("K3:", K2, DRV_LENGTH, 16);

        //=========================K1*G========================
        cmd_ec_point_mul.session = (mt_session)handle;

        memcpy(cmd_ec_point_mul.xParams_q, xParams.q + xParams.keySize - DRV_LENGTH, DRV_LENGTH);
        memcpy(cmd_ec_point_mul.xParams_a, xParams.a + xParams.keySize - DRV_LENGTH, DRV_LENGTH);
        cmd_ec_point_mul.xParams_keySize = DRV_LENGTH;

        if (CaseID == 1) {
            memcpy(cmd_ec_point_mul.xParams_GX, xParams.GX + xParams.keySize - DRV_LENGTH, DRV_LENGTH);
            memcpy(cmd_ec_point_mul.xParams_GY, xParams.GY + xParams.keySize - DRV_LENGTH, DRV_LENGTH);
            memcpy(cmd_ec_point_mul.g_scalar, K1, DRV_LENGTH);
            cmd_ec_point_mul.p_point_flag = 0;
        } else //(CaseID == 2)
        {
            memcpy(cmd_ec_point_mul.p_point_X, p_point->X + xParams.keySize - DRV_LENGTH, DRV_LENGTH);
            memcpy(cmd_ec_point_mul.p_point_Y, p_point->Y + xParams.keySize - DRV_LENGTH, DRV_LENGTH);
            memcpy(cmd_ec_point_mul.p_scalar, K1, DRV_LENGTH);
            cmd_ec_point_mul.p_point_flag = 1;
        }

        ioctl(ce_fd, CE_IOC_EC_POINT_MUL, &cmd_ec_point_mul);

        if (MT_SUCCESS == cmd_ec_point_mul.ret) {
            memcpy(r1.X, cmd_ec_point_mul.r_X, DRV_LENGTH);
            memcpy(r1.Y, cmd_ec_point_mul.r_Y, DRV_LENGTH);
        } else {
            return CE_EC_POINT_MUL_160K1G_ERROR;
        }
        //drv_ce_data_print("r1.X:", r1.X, DRV_LENGTH, 16);
        //drv_ce_data_print("r1.Y:", r1.Y, DRV_LENGTH, 16);

        //=========================k1*G+k2*G========================
        if (flag) {
            if (CaseID == 1) {
                memcpy(cmd_ec_point_mul.g_scalar, K2, DRV_LENGTH);
                cmd_ec_point_mul.p_point_flag = 0;
            } else //(CaseID == 2)
            {
                memcpy(cmd_ec_point_mul.p_scalar, K2, DRV_LENGTH);
                cmd_ec_point_mul.p_point_flag = 1;
            }

            ioctl(ce_fd, CE_IOC_EC_POINT_MUL, &cmd_ec_point_mul);

            if (MT_SUCCESS == cmd_ec_point_mul.ret) {
                memcpy(r2.X, cmd_ec_point_mul.r_X, DRV_LENGTH);
                memcpy(r2.Y, cmd_ec_point_mul.r_Y, DRV_LENGTH);
            } else {
                return CE_EC_POINT_MUL_160K2G_ERROR;
            }
            //drv_ce_data_print("r2.X:", r2.X, DRV_LENGTH, 16);
            //drv_ce_data_print("r2.Y:", r2.Y, DRV_LENGTH, 16);

            CMD_EC_POINT_ADD_S cmd_ec_point_add;
            cmd_ec_point_add.session = (mt_session)handle;

            memcpy(cmd_ec_point_add.xParams_q, xParams.q + xParams.keySize - DRV_LENGTH, DRV_LENGTH);
            memcpy(cmd_ec_point_add.xParams_a, xParams.a + xParams.keySize - DRV_LENGTH, DRV_LENGTH);
            cmd_ec_point_add.xParams_keySize = DRV_LENGTH;

            memcpy(cmd_ec_point_add.a_X, r1.X, DRV_LENGTH);
            memcpy(cmd_ec_point_add.a_Y, r1.Y, DRV_LENGTH);

            memcpy(cmd_ec_point_add.b_X, r2.X, DRV_LENGTH);
            memcpy(cmd_ec_point_add.b_Y, r2.Y, DRV_LENGTH);

            ioctl(ce_fd, CE_IOC_EC_POINT_ADD, &cmd_ec_point_add);

            if (MT_SUCCESS == cmd_ec_point_add.ret) {
                memset(r->X, 0, xParams.keySize);
                memset(r->Y, 0, xParams.keySize);
                memcpy(r->X + xParams.keySize - DRV_LENGTH, cmd_ec_point_add.r_X, DRV_LENGTH);
                memcpy(r->Y + xParams.keySize - DRV_LENGTH, cmd_ec_point_add.r_Y, DRV_LENGTH);
            } else {
                return CE_EC_POINT_MUL_160K1K2G_ERROR;
            }
        }
        //=========================K1*G*2========================
        else {
            memcpy(r2.X, r1.X, DRV_LENGTH);
            memcpy(r2.Y, r1.Y, DRV_LENGTH);

            memcpy(cmd_ec_point_mul.p_point_X, r1.X, DRV_LENGTH);
            memcpy(cmd_ec_point_mul.p_point_Y, r1.Y, DRV_LENGTH);
            memcpy(cmd_ec_point_mul.p_scalar, K3, DRV_LENGTH);
            cmd_ec_point_mul.p_point_flag = 1;

            ioctl(ce_fd, CE_IOC_EC_POINT_MUL, &cmd_ec_point_mul);

            if (MT_SUCCESS == cmd_ec_point_mul.ret) {
                memset(r->X, 0, xParams.keySize);
                memset(r->Y, 0, xParams.keySize);
                memcpy(r->X + xParams.keySize - DRV_LENGTH, cmd_ec_point_mul.r_X, DRV_LENGTH);
                memcpy(r->Y + xParams.keySize - DRV_LENGTH, cmd_ec_point_mul.r_Y, DRV_LENGTH);
            } else {
                return CE_EC_POINT_MUL_160K1K2G_ERROR;
            }
        }

        return 0;
    } else if (CaseID == 3 || CaseID == 4) {
        cmd_ec_point_mul.session = (mt_session)handle;

        memcpy(cmd_ec_point_mul.xParams_q, xParams.q + 1, xParams.keySize - 1);
        memcpy(cmd_ec_point_mul.xParams_a, xParams.a + 1, xParams.keySize - 1);
        cmd_ec_point_mul.xParams_keySize = xParams.keySize - 1;
        if (CaseID == 3) {
            memcpy(cmd_ec_point_mul.xParams_GX, xParams.GX + 1, xParams.keySize - 1);
            memcpy(cmd_ec_point_mul.xParams_GY, xParams.GY + 1, xParams.keySize - 1);
            memcpy(cmd_ec_point_mul.g_scalar, g_scalar + 1, xParams.keySize - 1);
            cmd_ec_point_mul.p_point_flag = 0;
        } else //(CaseID == 4)
        {
            memcpy(cmd_ec_point_mul.p_point_X, p_point->X + 1, xParams.keySize - 1);
            memcpy(cmd_ec_point_mul.p_point_Y, p_point->Y + 1, xParams.keySize - 1);
            memcpy(cmd_ec_point_mul.p_scalar, p_scalar + 1, xParams.keySize - 1);
            cmd_ec_point_mul.p_point_flag = 1;
        }

        ioctl(ce_fd, CE_IOC_EC_POINT_MUL, &cmd_ec_point_mul);

        if (MT_SUCCESS == cmd_ec_point_mul.ret) {
            r->X[0] = 0;
            r->Y[0] = 0;
            memcpy(r->X + 1, cmd_ec_point_mul.r_X, xParams.keySize - 1);
            memcpy(r->Y + 1, cmd_ec_point_mul.r_Y, xParams.keySize - 1);
        }

        return cmd_ec_point_mul.ret;
    } else //(CaseID == 5 || CaseID == 6)
    {
        cmd_ec_point_mul.session = (mt_session)handle;

        memcpy(cmd_ec_point_mul.xParams_q, xParams.q, xParams.keySize);
        memcpy(cmd_ec_point_mul.xParams_a, xParams.a, xParams.keySize);
        cmd_ec_point_mul.xParams_keySize = xParams.keySize;

        if (CaseID == 5) {
            memcpy(cmd_ec_point_mul.xParams_GX, xParams.GX, xParams.keySize);
            memcpy(cmd_ec_point_mul.xParams_GY, xParams.GY, xParams.keySize);
            memcpy(cmd_ec_point_mul.g_scalar, g_scalar, xParams.keySize);
            cmd_ec_point_mul.p_point_flag = 0;
        } else //(CaseID == 6)
        {
            memcpy(cmd_ec_point_mul.p_point_X, p_point->X, xParams.keySize);
            memcpy(cmd_ec_point_mul.p_point_Y, p_point->Y, xParams.keySize);
            memcpy(cmd_ec_point_mul.p_scalar, p_scalar, xParams.keySize);
            cmd_ec_point_mul.p_point_flag = 1;
        }

        ioctl(ce_fd, CE_IOC_EC_POINT_MUL, &cmd_ec_point_mul);

        if (MT_SUCCESS == cmd_ec_point_mul.ret) {
            memcpy(r->X, cmd_ec_point_mul.r_X, xParams.keySize);
            memcpy(r->Y, cmd_ec_point_mul.r_Y, xParams.keySize);
        }

        return cmd_ec_point_mul.ret;
    }
}

mt_s32 mt_mpi_crypto_ec_point_add(int ce_fd, mt_handle handle, MT_CIPHER_EC_PARAMS_S xParams, MT_CIPHER_EC_POINT_S *r, const MT_CIPHER_EC_POINT_S *a, const MT_CIPHER_EC_POINT_S *b)
{
    CMD_EC_POINT_ADD_S cmd_ec_point_add;

    cmd_ec_point_add.session = (mt_session)handle;

    memcpy(cmd_ec_point_add.xParams_q, xParams.q, xParams.keySize);
    memcpy(cmd_ec_point_add.xParams_a, xParams.a, xParams.keySize);
    cmd_ec_point_add.xParams_keySize = xParams.keySize;

    if (a) {
        memcpy(cmd_ec_point_add.a_X, a->X, xParams.keySize);
        memcpy(cmd_ec_point_add.a_Y, a->Y, xParams.keySize);
    }

    if (b) {
        memcpy(cmd_ec_point_add.b_X, b->X, xParams.keySize);
        memcpy(cmd_ec_point_add.b_Y, b->Y, xParams.keySize);
    }

    ioctl(ce_fd, CE_IOC_EC_POINT_ADD, &cmd_ec_point_add);

    if (MT_SUCCESS == cmd_ec_point_add.ret) {
        memcpy(r->X, cmd_ec_point_add.r_X, xParams.keySize);
        memcpy(r->Y, cmd_ec_point_add.r_Y, xParams.keySize);
    }

    return cmd_ec_point_add.ret;
}

/**
 * @brief
 *
 * @param p_random_number
 *
 * @return
 */
mt_s32 mt_mpi_cipher_get_random_number(mt_u32 bytes_to_get, mt_u8 *p_random_number)
{
    mt_s32 ret = MT_SUCCESS;
    ssize_t read_bytes = 0;

    MSS_LOCK(&g_m2m_mutex);

    int rng_fd = open("/dev/hwrng", O_RDONLY);
    if (rng_fd < 0) {
        MSS_UNLOCK(&g_m2m_mutex);
        return MT_FAILURE;
    }

    read_bytes = read(rng_fd, p_random_number, bytes_to_get);
    if (read_bytes < 0) {
        MSS_UNLOCK(&g_m2m_mutex);
        ret = MT_FAILURE;
    }

    close(rng_fd);
    MSS_UNLOCK(&g_m2m_mutex);
    return ret;
}

#if defined (CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 mt_mpi_cipher_bgc_request(int ce_fd, mt_handle *bgc_slot, MT_CE_BGC_SEM_REQ_S time_out)
{
    CMD_BGC_REQ_S bgc_slot_request;
    mt_s32 ret = MT_FAILURE;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    bgc_slot_request.req_time_out = time_out;

    MSS_LOCK(&g_m2m_mutex);

    ret = ioctl(ce_fd, CE_IOC_BGC_REQUEST, &bgc_slot_request);

    if (MT_SUCCESS == ret) {
        *bgc_slot = bgc_slot_request.session;
    }

    MSS_UNLOCK(&g_m2m_mutex);

    return ret;
}

mt_s32 mt_mpi_cipher_bgc_release(int ce_fd, mt_handle bgc_slot, MT_CE_BGC_SEM_REQ_S time_out)
{
    mt_s32 ret = MT_FAILURE;
    CMD_BGC_RLS_S bgc_slot_release;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    bgc_slot_release.session = bgc_slot;
    bgc_slot_release.req_time_out = time_out;

    MSS_LOCK(&g_m2m_mutex);

    ret = ioctl(ce_fd, CE_IOC_BGC_RELEASE, &bgc_slot_release);

    MSS_UNLOCK(&g_m2m_mutex);
    return ret;
}

mt_s32 mt_mpi_cipher_bgc_setup(int ce_fd, mt_handle bgc_slot, const mt_u8 *bgc_addr, mt_u32 size, MT_CE_BGC_DELAY delay, mt_u8 *golden_hash, MT_CE_BGC_LOCK_S lock)
{
    mt_s32 ret = MT_FAILURE;
    CMD_BGC_SETUP_S bgc_setup_t;

    if (ce_fd < 0) {
        MT_ERR_CIPHER("Invalid cipher fd");
        return MT_FAILURE;
    }

    bgc_setup_t.session = bgc_slot;
    bgc_setup_t.start_addr = bgc_addr;
    bgc_setup_t.size = size;
    bgc_setup_t.delay = delay;
    bgc_setup_t.golden_hash = golden_hash;
    bgc_setup_t.lock = lock;

    MSS_LOCK(&g_m2m_mutex);

    ret = ioctl(ce_fd, CE_IOC_BGC_SETUP, &bgc_setup_t);

    MSS_UNLOCK(&g_m2m_mutex);
    return ret;
}
#endif
