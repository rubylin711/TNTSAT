/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_MPI_CIPHER_V2_H__
#define __MT_MPI_CIPHER_V2_H__

#include "mt_type.h"
#include "mt_mpi_kt.h"
#include "mt_mpi_ce.h"
#include <pthread.h>
#include <semaphore.h>

struct cipher_config_priv 
{
    mt_handle cipher;
    MT_CIPHER_CTRL_S *p_ctrl;
};

struct cipher_rsa_config_priv 
{
    mt_handle cipher;
    MT_CIPHER_RSA_CTRL_S *p_ctrl;
};

struct cipher_data_priv
{ 
    mt_handle cipher;
    mt_u8 *p_src;
    mt_u8 *p_dst;
    mt_u32 length;
};

struct cipher_rng_priv
{
    mt_u8 *p_rng_data;
    mt_u32 length;
};

struct cipher_hash_data_priv
{
    mt_handle hash_handle;
    mt_u8 *p_msg;
    mt_u32 length;
    mt_u8 *p_output_hash; //pointer to user allocated output buffer at 'final' stage
    mt_u32 hash_type;
};

#define MSS_IOCTL_LOCK_DUMMY

/*For HMAC*/
#define HASH_MAX_BLOCK_SIZE  (64)
#define HASH_MAX_DIGEST_SIZE (32)

/*For Ioctl Mutex Lock*/
#ifdef MSS_IOCTL_LOCK_DUMMY
#define MSS_LOCK_INIT(p_mutex)
#define MSS_LOCK_DESTROY(p_mutex)
#define MSS_LOCK(p_mutex)
#define MSS_UNLOCK(p_mutex)
#else
#define MSS_LOCK_INIT(p_mutex)                 \
	(void)pthread_mutex_init(p_mutex, NULL)

#define MSS_LOCK_DESTROY(p_mutex)                  \
	(void)pthread_mutex_destroy(p_mutex)

#define MSS_LOCK(p_mutex)                          \
	(void)pthread_mutex_lock(p_mutex)

#define MSS_UNLOCK(p_mutex)                        \
	(void)pthread_mutex_unlock(p_mutex)
#endif

/* Semaphore Synchronize */
#define MSS_SEM_CTX_NAME(name) mss_##name##_sem_ctx
#define MSS_SEM_CTX(sem) sem_t MSS_SEM_CTX_NAME(sem)
#define MSS_SEM_INIT(sem, v) sem_init(&MSS_SEM_CTX_NAME(sem), 0, v)
#define MSS_SEM_WAIT(sem) sem_wait(&MSS_SEM_CTX_NAME(sem))
#define MSS_SEM_POST(sem) sem_post(&MSS_SEM_CTX_NAME(sem))

/******************************* API Declaration *****************************/

/*!
  Init the cipher device
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_cipher_init(void);

/*!
  Deinit the cipher device
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_cipher_deinit(void);

mt_s32 mt_mpi_crypto_open(int *ce_fd);
mt_s32 mt_mpi_crypto_close(int ce_fd);

/*!
  allocate a buffer used by cipher only.

  \param[in] (length) bytes to allocate
  
  \return SUCCESS, else fail
  */
mt_u8 *mt_mpi_cipher_malloc(mt_u32 length);

/*!
  free a buffer allocated by MT_MPI_CIPHER_malloc.

  \param[in] (p) pointer of buffer to free
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_cipher_free(mt_void *p);

/*!
  Create hash operation handle
  
  \param[in] (hash_type) hash type
  \param[in] (ce_fd) cipher file handle
  \param[out] (p_handle) created cipher hash handle
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_hash_create(MT_CIPHER_HASH_TYPE_E hash_type, int ce_fd, mt_handle *p_handle);

/*!
  Calculate the hash

  \param[in] (ce_fd) cipher file handle
  \param[in] (handle) hash handle
  \param[in] (p_data) input data 
  \param[in] (length) length of data
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_hash_update(int ce_fd, mt_handle handle, mt_u8 *p_data, mt_u32 length);

/*!
  Get the final hash value

  \param[in] (ce_fd) cipher file handle
  \param[in] (handle) hash handle
  \param[out] (p_output_hash) final output hash value
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_hash_final(int ce_fd, mt_handle handle, mt_u8 *p_output_hash);

/*!
  Create mac operation handle
  
  \param[in] (mac_type) mac type
  \param[in] (p_hmac_attr) mac attribute
  \param[in] (ce_fd) cipher file handle
  \param[out] (p_handle) created cipher mac handle
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_mac_create(MT_CIPHER_MAC_TYPE_E mac_type, MT_CIPHER_HMAC_ATTS_S *p_hmac_attr, int ce_fd, mt_handle *p_handle);

/*!
  Calculate the mac

  \param[in] (ce_fd) cipher file handle
  \param[in] (handle) mac handle
  \param[in] (p_data) input data 
  \param[in] (length) length of data
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_mac_update(int ce_fd, mt_handle handle, mt_u8 *p_data, mt_u32 length);

/*!
  Get the final mac value

  \param[in] (ce_fd) cipher file handle
  \param[in] (handle) mac handle
  \param[out] (p_output_mac) final output mac value
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_mac_final(int ce_fd, mt_handle handle, mt_u8 *p_output_mac);

/*!
  Obtain a cipher handle for encryption decryption and hash operation

  \param[in] (channel) CryptoEngine channel
  \param[out] (ph_cipher) created cipher handle
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_crypto_create(unsigned int channel, int ce_fd, mt_handle *p_cipher);

/*!
  Destroy the existing cipher handle
  
  \param[in] (h_cipher) the cipher handle will be destroied
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_crypto_destroy(int ce_fd, mt_handle cipher);

/*!
  Configures the cipher control information
  
  \param[in] (cipher) cipher handle
  \param[in] (p_ctrl) cipher attributes
  \param[in] (key_slot) key slot id
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_crypto_config(int ce_fd, mt_handle cipher, MT_CIPHER_CTRL_S *p_ctrl, mt_u32 key_slot);

/*!
  Performs encryption or decryption 
  
  \param[in] (cipher) cipher handle
  \param[in] (p_src_addr) source data address
  \param[in] (p_dest_addr) target data address
  \param[in] (length) length of data
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_crypto_process(int ce_fd, mt_handle cipher, mt_u8 *p_src_addr, mt_u8 *p_dest_addr, mt_u32 length);

/*!
  Performs encryption or decryption(Always expect physical address)

  \param[in] (cipher) cipher handle
  \param[in] (p_src_phyaddr) source data physical address
  \param[in] (p_dest_phyaddr) target data physical address
  \param[in] (clear_length) clear length of data
  \param[in] (cipher_length) cipher length of data

  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_crypto_process_phy(int ce_fd, mt_handle crypto, phys_addr_t p_src_phyaddr, phys_addr_t p_dest_phyaddr, mt_u32 clear_length, mt_u32 cipher_length);

/*!
  Request encryption or decryption asynchronously

  \param[in] (cipher) cipher handle
  \param[in] (src) source data address
  \param[in] (dst) target data address
  \param[in] (count) count of the pair of (clear_length, protected_length)
  \param[in] (clear_length) length of clear data
  \param[in] (protected_length) length of protected data

  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_cipher_crypto_async_request(int ce_fd, mt_handle cipher, const mt_u8 *src, mt_u8 *dst, mt_u32 count, mt_u32 clear_length, mt_u32 protected_length);

/*!
  Start encryption or decryption asynchronously

  \param[in] (cipher) cipher handle

  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_cipher_crypto_async_start(int ce_fd, mt_handle cipher);

/*!
  Wait encryption or decryption for completion

  \param[in] (cipher) cipher handle

  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_cipher_crypto_async_wait(int ce_fd, mt_handle cipher);

/*!
  Obtain a rsa handle for encryption decryption

  \param[out] (p_rsa_handle) create rsa handle
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_crypto_rsa_create(int ce_fd, mt_handle *p_rsa_handle);

/*!
  Destroy the existing cipher handle
  
  \param[in] (rsa_handle) the rsa handle
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_crypto_rsa_destroy(int ce_fd, mt_handle rsa_handle);

/*!
  Configures the rsa control information
  
  \param[in] (rsa_handle) rsa handle
  \param[in] (p_ctrl) rsa attributes
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_crypto_rsa_config(int ce_fd, mt_handle rsa_handle, MT_CIPHER_RSA_CTRL_S *p_ctrl);

/*!
  Performs rsa encryption or decryption 
  
  \param[in] (rsa_handle) rsa handle
  \param[in] (p_src_addr) source data address
  \param[in] (p_dest_addr) target data address
  \param[in] (length) length of data
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_crypto_rsa_process(int ce_fd, mt_handle rsa_handle, mt_u8 *p_src_addr, mt_u8 *p_dest_addr, mt_u32 length);


/**
 * @brief Create TS handle
 *
 * @param[in] (channel) CryptoEngine channel
 * @param[out] p_ts_handle: the TS handle
 *
 * @return SUCCESS, else fail
 */
mt_s32 mt_mpi_crypto_ts_create(unsigned int channel, int ce_fd, mt_handle *p_ts_handle);

/**
 * @brief Destroy TS handle
 *
 * @param[in] handle: the TS handle
 *
 * @return SUCCESS, else fail
 */
mt_s32 mt_mpi_crypto_ts_destroy(int ce_fd, mt_handle handle);

/**
 * @brief  Configures the TS control information
 *
 * @param[in] handle: the TS handle
 * @param[in] p_ctrl: contrl info.
 *
 * @return SUCCESS, else fail
 */
mt_s32 mt_mpi_crypto_ts_config(int ce_fd, mt_handle handle, MT_CE_TS_CTRL_S *p_ctrl);

/**
 * @brief Processing...
 *
 * @param[in] handle
 * @param[in] p_src_addr: input data buffer
 * @param[out] p_dst_addr: output data buffer
 * @param[in] length: data length
 *
 * @return SUCCESS, else fail
 */
mt_s32 mt_mpi_crypto_ts_process(int ce_fd, mt_handle handle, phys_addr_t p_src_addr, phys_addr_t p_dst_addr, mt_u32 length);

/*!
  Obtain a bn handle

  \param[out] (p_bn_handle) create bn handle
  
  \return SUCCESS, else fail
*/
mt_s32 mt_mpi_crypto_bn_create(int ce_fd, mt_handle *p_bn_handle);

/*!
  Destroy the existing cipher handle
  
  \param[in] (handle) the bn handle
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_crypto_bn_destroy(int ce_fd, mt_handle handle);

/**
 * @brief modular
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  d:   in data  
 * @param[in]  m:   modulo
 * @param[in]  d_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
mt_s32 mt_mpi_crypto_bn_mod_mod(int ce_fd, mt_handle handle, mt_u8 *r, mt_u8 *d, mt_u8 *m, mt_u32 d_length, mt_u32 m_length);

/**
 * @brief modular multiply
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  a:   input a
 * @param[in]  b:   input b
 * @param[in]  m:   modulo
 * @param[in]  a_length:    data length
 * @param[in]  b_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
mt_s32 mt_mpi_crypto_bn_mod_mul(int ce_fd, mt_handle handle, mt_u8 *r, mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length);

/**
 * @brief modular addition
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  a:   input a
 * @param[in]  b:   input b
 * @param[in]  m:   modulo
 * @param[in]  a_length:    data length
 * @param[in]  b_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
mt_s32 mt_mpi_crypto_bn_mod_add(int ce_fd, mt_handle handle, mt_u8 *r, mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length);

/**
 * @brief modular subtraction
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  a:   input a
 * @param[in]  b:   input b
 * @param[in]  m:   modulo
 * @param[in]  a_length:    data length
 * @param[in]  b_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
mt_s32 mt_mpi_crypto_bn_mod_sub(int ce_fd, mt_handle handle, mt_u8 *r, mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length);

/**
 * @brief modular inverse
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  d:   input data
 * @param[in]  m:   modulo
 * @param[in]  d_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
mt_s32 mt_mpi_crypto_bn_mod_inv(int ce_fd, mt_handle handle, mt_u8 *r, mt_u8 *d, mt_u8 *m, mt_u32 d_length, mt_u32 m_length);

/**
 * @brief modular exponentiation
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  a:   input data
 * @param[in]  p:   input data
 * @param[in]  m:   modulo
 * @param[in]  a_length:    data length
 * @param[in]  p_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
mt_s32 mt_mpi_crypto_bn_mod_exp(int ce_fd, mt_handle handle, mt_u8 *r, mt_u8 *a, mt_u8 *p, mt_u8 *m, mt_u32 a_length, mt_u32 p_length, mt_u32 m_length);


/**
 * @brief Create ECP handle
 *
 * @param[out] p_ecp_handle: the ECP handle
 *
 * @return SUCCESS, or failed
 */
mt_s32 mt_mpi_crypto_ec_point_create(int ce_fd, mt_handle *p_ecp_handle);

/**
 * @brief Destroy ECP handle
 *
 * @param[in] p_ecp_handle: the ECP handle
 *
 * @return SUCCESS, or failed
 */
mt_s32 mt_mpi_crypto_ec_point_destroy(int ce_fd, mt_handle handle);

/**
 * @brief ECP multiply operation
 *
 * @param[in] handle
 * @param[in] xParams
 * @param[out] r
 * @param[in] g_scalar
 * @param[in] p_point
 * @param[in] p_scalar
 *
 * @return SUCCESS, or failed
 */
mt_s32 mt_mpi_crypto_ec_point_mul(int ce_fd, mt_handle handle, MT_CIPHER_EC_PARAMS_S xParams, MT_CIPHER_EC_POINT_S *r, const mt_u8 *g_scalar,  MT_CIPHER_EC_POINT_S *p_point, const mt_u8 *p_scalar);

/**
 * @brief ECP add operation
 *
 * @param[in] handle
 * @param[in] xParams
 * @param[out] r
 * @param[in] a 
 * @param[in] b
 *
 * @return SUCCESS, or failed
 */
mt_s32 mt_mpi_crypto_ec_point_add(int ce_fd, mt_handle handle, MT_CIPHER_EC_PARAMS_S xParams, MT_CIPHER_EC_POINT_S *r, const MT_CIPHER_EC_POINT_S *a, const MT_CIPHER_EC_POINT_S *b);

/*!
  Get a random number
  
  \param[in] (bytes_to_get) bytes to get
  \param[out] (p_random_number) point to the random number
  
  \return SUCCESS, else fail
  */
mt_s32 mt_mpi_cipher_get_random_number(mt_u32 bytes_to_get, mt_u8 *p_random_number);

mt_s32 mt_mpi_kt_attr_config(MT_CIPHER_CTRL_S *p_ctrl, MT_KT_KEY_ATTR_S *p_ka);
mt_s32 mt_mpi_kt_attr_config_ext(MT_KT_CTRL_S *p_ctrl, MT_KT_KEY_ATTR_S *p_ka);

#if defined (CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 mt_mpi_cipher_bgc_request(int ce_fd, mt_handle *bgc_slot, MT_CE_BGC_SEM_REQ_S time_out);
mt_s32 mt_mpi_cipher_bgc_release(int ce_fd, mt_handle bgc_slot, MT_CE_BGC_SEM_REQ_S time_out);
mt_s32 mt_mpi_cipher_bgc_setup(int ce_fd, mt_handle bgc_slot, const mt_u8 *bgc_addr, mt_u32 size, MT_CE_BGC_DELAY delay, mt_u8 *golden_hash, MT_CE_BGC_LOCK_S lock);
#endif

#endif	/*__MT_MPI_CIPHER_V2_H__*/
