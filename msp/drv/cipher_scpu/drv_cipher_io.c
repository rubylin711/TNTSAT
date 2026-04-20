/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
  system
  */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
//
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/proc_fs.h>

#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/compiler.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/version.h>

#include <asm/io.h>
#include <asm/cache.h>
#include <asm/cacheflush.h>
#include "mt_cache.h"
/*!
 * ko
 */
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "drv_cipher_scpu_ioctl.h"
#include "crypto_secure.h"
#include "hal_cipher.h"
#include "../ipcs/ipcs_symphony.h"
#include "mt_module_debug.h"
#include "mt_drv_mmz.h" /* hash result */
/*!
  security
  */
static void mt_flush_data_cache_range(mt_u8 *addr, unsigned int size)
{
    phys_addr_t addr_aligned = (phys_addr_t)addr;
    size_t size_aligned = size;

    if (addr_aligned % CACHE_LINE_SIZE)
    {
        addr_aligned = (addr_aligned / CACHE_LINE_SIZE) * CACHE_LINE_SIZE;
    }

    if (size_aligned % CACHE_LINE_SIZE)
    {
        size_aligned = (size_aligned / CACHE_LINE_SIZE) * CACHE_LINE_SIZE + CACHE_LINE_SIZE;
    }
    mt_dcache_flush((phys_addr_t)(addr_aligned), (size_t)size_aligned);
}

static void mt_invaild_data_cache_range(mt_u8 *addr, unsigned int size)
{
    phys_addr_t addr_aligned = (phys_addr_t)addr;
    size_t size_aligned = size;

    if (addr_aligned % CACHE_LINE_SIZE)
    {
        addr_aligned = (addr_aligned / CACHE_LINE_SIZE) * CACHE_LINE_SIZE;
    }

    if (size_aligned % CACHE_LINE_SIZE)
    {
        size_aligned = (size_aligned / CACHE_LINE_SIZE) * CACHE_LINE_SIZE + CACHE_LINE_SIZE;
    }
    mt_dcache_invalid((phys_addr_t)(addr_aligned), (size_t)size_aligned);
}


/*
 * global parameters
 */
typedef struct _keyladder_info_struct
{
    MT_CIPHER_KEY_LADDER_SOURCE_E keyladder_source;
    mt_u32 keyladder_level;
    mt_u8 *keyladder_input[ADV_MAXKELADDERLIMIT];
    EN_ADV_KEYCONFIGINFO ladder_info[ADV_MAXKELADDERLIMIT];
    EN_ADV_M2MCONFIGINFO m2m_info;
}keyladder_info_struct;

typedef struct _cipher_info_struct
{
    keyladder_info_struct *key_ladder_handle;
    EN_ADV_KEYCONFIGINFO info;
}cipher_handle;

typedef struct _hash_info_struct
{
    EN_ADV_KEYCONFIGINFO info ;
    mmz_buffer_s hash_result;
} hash_handle;

static  int cipher_major        = 241;

//#define HAL_CIPHER_DEBUG
void dump_data(char *s, mt_u8 *buf, mt_u32 length)
{
#ifdef HAL_CIPHER_DEBUG
    mt_u32 i = 0;
    MT_INFO_CIPHER("%s\n", s);
    for(i = 0;i < length; i++)
    {
        if((i%16) == 0 && i != 0)
            MT_INFO_CIPHER("\n");
        MT_INFO_CIPHER("0x%02x ", buf[i]);
    }
    MT_INFO_CIPHER("\n");
#else
    return;
#endif
}

static void cipher_transfer_workmode(mt_u32 work_mode , EN_ADV_KEYCONFIGINFO *p_info)
{
    switch(work_mode)
    {
        case MT_CIPHER_WORK_MODE_ECB:
            p_info->enBlockMode = ADV_BLOCKMODE_ECB;
            break;
        case MT_CIPHER_WORK_MODE_CBC:
            p_info->enBlockMode = ADV_BLOCKMODE_CBC;
            break;
        case MT_CIPHER_WORK_MODE_CTR:
            p_info->enBlockMode = ADV_BLOCKMODE_CTR;
            break;
        case MT_CIPHER_WORK_MODE_CBCDVS042:
            p_info->enBlockMode = ADV_BLOCKMODE_CBCDVS042;
            break;
        case MT_CIPHER_WORK_MODE_CBCCTS:
            p_info->enBlockMode = ADV_BLOCKMODE_CBCCTS;
            break;
        case MT_CIPHER_WORK_MODE_RCBC:
            p_info->enBlockMode = ADV_BLOCKMODE_RCBC;
            break;
        case MT_CIPHER_WORK_MODE_ECBCTS:
            p_info->enBlockMode = ADV_BLOCKMODE_ECBCTS;
            break;
        case MT_CIPHER_WORK_MODE_CFB:
            p_info->enBlockMode = ADV_BLOCKMODE_CFB;
            break;
        case MT_CIPHER_WORK_MODE_CFB8:
            p_info->enBlockMode = ADV_BLOCKMODE_CFB8;
            break;
        case MT_CIPHER_WORK_MODE_CFB64_128:
            p_info->enBlockMode = ADV_BLOCKMODE_CFB64_128;
            break;
        case MT_CIPHER_WORK_MODE_OFB:
            p_info->enBlockMode = ADV_BLOCKMODE_OFB;
            break;
        case MT_CIPHER_WORK_MODE_OFB8:
            p_info->enBlockMode = ADV_BLOCKMODE_OFB8;
            break;
        case MT_CIPHER_WORK_MODE_OFB64_128:
            p_info->enBlockMode = ADV_BLOCKMODE_OFB64_128;
            break;
        default:
            MT_ERR_CIPHER( "unknown work_mode type!\n");
            break;
    }
}

static mt_s32 cipher_info_transfer(EN_ADV_KEYCONFIGINFO *p_info, MT_CIPHER_CTRL_S *p_ctrl, MT_BOOL use_keyladder)
{
    if(p_ctrl->operation == MT_CIPHER_OPERATION_DECRYPT)
    {
        p_info->enOpration = EN_ADV_CIPHERENGINE_DECRYPT;
    }
    else if(p_ctrl->operation == MT_CIPHER_OPERATION_ENCRYPT)
    {
        p_info->enOpration = EN_ADV_CIPHERENGINE_ENCRYPT;
    }
    else
    {
        MT_ERR_CIPHER( "unknown operation type!\n");
        return MT_FAILURE;
    }

    if(p_ctrl->algorithm == MT_CIPHER_ALG_DES)
    {
        p_info->enAlgorithm = ADV_ALGORITHM_DES;
        cipher_transfer_workmode(p_ctrl->parameter.des_para.work_mode, p_info);
        p_info->enKeySize = p_ctrl->parameter.des_para.key_length;
        if(!use_keyladder)
        {
            if(p_ctrl->parameter.des_para.p_key != NULL)
                copy_from_user(p_info->u8Key, (void __user*)p_ctrl->parameter.des_para.p_key, p_info->enKeySize);
            else
                MT_WARN_LOG( "Warning, input key is NULL!\n");

        }
        copy_from_user(p_info->u8IV, (void __user *)p_ctrl->parameter.des_para.iv, 16);
    }
    else if(p_ctrl->algorithm == MT_CIPHER_ALG_TDES)
    {
        p_info->enAlgorithm = ADV_ALGORITHM_TDES;
        cipher_transfer_workmode(p_ctrl->parameter.tdes_para.work_mode, p_info);
        p_info->enKeySize = p_ctrl->parameter.tdes_para.key_length;
        if(!use_keyladder)
        {
            if(p_ctrl->parameter.tdes_para.p_key != NULL)
                copy_from_user(p_info->u8Key, (void __user*)p_ctrl->parameter.tdes_para.p_key,  p_info->enKeySize);
            else
                MT_WARN_LOG( "Warning, input key is NULL!\n");
        }
        copy_from_user(p_info->u8IV, (void __user*)p_ctrl->parameter.tdes_para.iv, 16);
    }
    else if(p_ctrl->algorithm == MT_CIPHER_ALG_AES)
    {
        p_info->enAlgorithm = ADV_ALGORITHM_AES;
        cipher_transfer_workmode(p_ctrl->parameter.aes_para.work_mode, p_info);
        p_info->enKeySize = p_ctrl->parameter.aes_para.key_length;
        if(!use_keyladder)
        {
            if(p_ctrl->parameter.aes_para.p_key != NULL)
                copy_from_user(p_info->u8Key, (void __user*)p_ctrl->parameter.aes_para.p_key,  p_info->enKeySize);
            else
                MT_WARN_LOG( "Warning, input key is NULL!\n");
        }
        copy_from_user(p_info->u8IV,(void __user *) p_ctrl->parameter.aes_para.iv, 16);
    }
    else if(p_ctrl->algorithm ==  MT_CIPHER_ALG_RSA)
    {
        p_info->enAlgorithm = ADV_ALGORITHM_RSA;
        p_info->info_RSAkey.enKeySize = p_ctrl->parameter.rsa_para.key_length;
        p_info->info_RSAkey.p_addr_e = (mt_u32 *)((mt_u32)p_ctrl->parameter.rsa_para.p_e | 0xa0000000);
        p_info->info_RSAkey.p_addr_m = (mt_u32 *)((mt_u32)p_ctrl->parameter.rsa_para.p_m | 0xa0000000);
    }
    else
    {
        MT_ERR_CIPHER( "unknown algorithm type!\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

/*!
 ***********************************************************************
 **  description : copy data from or to user space according to _IOC_DIR
 **  return      : int
 ***********************************************************************
 */
static int cipher_usercopy(struct inode *inode, struct file *file,
        unsigned int cmd, unsigned long arg,
        int (*func)(struct inode *inode, struct file *file,
            unsigned int cmd, void *arg))
{
    char    sbuf[128];
    void    *mbuf = NULL;
    void    *parg = NULL;
    int     err  = -EINVAL;

    /*  Copy arguments into temp kernel buffer  */
    switch (_IOC_DIR(cmd)) {
        case _IOC_NONE:
            /*
             * For this command, the pointer is actually an integer
             * argument.
             */
            parg = (void *) arg;
            break;
        case _IOC_READ: /* some v4l ioctls are marked wrong ... */
        case _IOC_WRITE:
        case (_IOC_WRITE | _IOC_READ):
            if (_IOC_SIZE(cmd) <= sizeof(sbuf)) {
                parg = sbuf;
            } else {
                /* too big to allocate from stack */
                mbuf = kmalloc(_IOC_SIZE(cmd),GFP_DMA);
                if (NULL == mbuf)
                    return -ENOMEM;
                parg = mbuf;
            }

            err = -EFAULT;
            if (copy_from_user(parg, (void __user *)arg, _IOC_SIZE(cmd)))
                goto out;
            break;
    }

    /* call driver */
    if ((err = func(inode, file, cmd, parg)) == -ENOIOCTLCMD)
        err = -EINVAL;

    if (err < 0)
        goto out;

    /*  Copy results into user buffer  */
    switch (_IOC_DIR(cmd))
    {
        case _IOC_READ:
        case (_IOC_WRITE | _IOC_READ):
            if (copy_to_user((void __user *)arg, parg, _IOC_SIZE(cmd)))
            err = -EFAULT;
            break;
    }

out:
    kfree(mbuf);
    return err;
}

/*!
 ***********************************************************************
 **  description : Open the device; in fact, there's nothing to do here.
 **  return      : NULL
 ***********************************************************************
 */
static int cipher_open (struct inode *inode, struct file *filp)////
{
    MT_INFO_CIPHER("[cipher_open]: ... \n");
    return MT_SUCCESS;
}


/*!
 ***********************************************************************
 **  description : Closing is just as simpler.
 **  return      : NULL
 ***********************************************************************
 */
static int cipher_release(struct inode *inode, struct file *filp)////
{
    MT_INFO_CIPHER("[cipher_release]: ... \n");
    return MT_SUCCESS;
}


/*
 * IO CONCTROL for secure
 */
static  int cipher_ioctl(struct inode *inode, struct file *file,unsigned int cmd, void *arg)
{
    int ret = MT_FAILURE;
    cipher_handle *p_handle = NULL;

    if (arg == NULL) {
        MT_ERR_CIPHER("Bad cipher parametes\n");
        return ret;
    }

    switch(cmd)
    {
        case CMD_SEC_CIPHER_INIT:
            //do nothing until now..
            ret = MT_SUCCESS;
            break;
        case CMD_SEC_CIPHER_HANDLE_CREATE:
            {
                struct cipher_handle_priv *ci = (struct cipher_handle_priv *)arg;
                p_handle = kmalloc(sizeof(cipher_handle), GFP_DMA);
                if (p_handle == NULL) {
                    MT_ERR_CIPHER("malloc failure!\n");
                    return  MT_FAILURE;
                }
                memset(p_handle, 0, sizeof(cipher_handle));
                //key_ladder_handle can be NULL
                p_handle->key_ladder_handle = (keyladder_info_struct *)ci->key_ladder_handle;
                ci->p_cipher = (mt_handle)p_handle;
                ret = MT_SUCCESS;
            };
            break;
        case CMD_SEC_CIPHER_HANDLE_DESTROY:
            {
                p_handle = (cipher_handle *)arg;
                if (p_handle == NULL)
                    return  MT_FAILURE;

                kfree(p_handle);
                p_handle = NULL;
                ret = MT_SUCCESS;
            };
            break;
        case CMD_SEC_CIPHER_INFO_CONFIG:
            {
                struct cipher_config_priv *ci = (struct cipher_config_priv *)arg;
                MT_CIPHER_CTRL_S *p_ctrl = ci->p_ctrl;
                p_handle = (cipher_handle *)ci->cipher;
                if (p_handle == NULL) {
                    MT_ERR_CIPHER("Invalid cipher handle\n");
                    return MT_FAILURE;
                }

                if (p_ctrl == NULL) {
                    MT_ERR_CIPHER("Bad cipher parameters\n");
                    return MT_FAILURE;
                }

                if (p_handle->key_ladder_handle != NULL) {
                    ret = cipher_info_transfer(&p_handle->key_ladder_handle->m2m_info.data2data_infro, p_ctrl, MT_TRUE);
                } else {
                    ret = cipher_info_transfer(&p_handle->info, p_ctrl, MT_FALSE);
                }

                ret = MT_SUCCESS;
            };
            break;
        case CMD_SEC_CIPHER_KEYLADDER_CREATE:
            {
                struct cipher_keyladder_priv *ci = (struct cipher_keyladder_priv *)arg;
                MT_CIPHER_KEY_LADDER_SOURCE_E keyladder_source = ci->keyladder_source;
                mt_u32 key_index = ci->index;
                keyladder_info_struct *p_keyladder_info = NULL;

                if (keyladder_source >= MT_CIPHER_KEY_LADDER_RESERVED) {
                    MT_ERR_CIPHER("Bad cipher parameters\n");
                    return MT_FAILURE;
                }

                p_keyladder_info = kmalloc(sizeof(keyladder_info_struct), GFP_DMA);
                if(p_keyladder_info == NULL)
                {
                    MT_ERR_CIPHER("keyladder info create failure!\n");
                    return  MT_FAILURE;
                }

                memset(p_keyladder_info, 0, sizeof(keyladder_info_struct));
                p_keyladder_info->keyladder_source = keyladder_source;
                p_keyladder_info->m2m_info.data2data_infro.key_index = key_index;
                switch (keyladder_source)
                {
                    case  MT_CIPHER_KEY_LADDER_FLASH:
                        MT_INFO_CIPHER("ladder flash\n");
                        p_keyladder_info->m2m_info.data2data_infro.enKeyOption0 = ADV_KEYOPTION_FLASH;
                        break;
                    case  MT_CIPHER_KEY_LADDER_UNIQUE:
                        p_keyladder_info->m2m_info.data2data_infro.enKeyOption0 = ADV_KEYOPTION_UNIQUE;
                        break;
                    case  MT_CIPHER_KEY_LADDER_COMMON:
                        p_keyladder_info->m2m_info.data2data_infro.enKeyOption0 =  ADV_KEYOPTION_COMMON;
                        break;
                    case  MT_CIPHER_KEY_LADDER_PVR:
                        p_keyladder_info->m2m_info.data2data_infro.enKeyOption0 =  ADV_KEYOPTION_PVR;
                        break;
                    case  MT_CIPHER_KEY_LADDER_DATA:
                        p_keyladder_info->m2m_info.data2data_infro.enKeyOption0 =  ADV_KEYOPTION_DATA;
                        break;
                    case  MT_CIPHER_KEY_LADDER_HDCP:
                        p_keyladder_info->m2m_info.data2data_infro.enKeyOption0 = ADV_KEYOPTION_HDCP;
                        break;
                    case  MT_CIPHER_KEY_LADDER_CW:
                        p_keyladder_info->m2m_info.data2data_infro.enKeyOption0 = ADV_KEYOPTION_CW;
                        break;
                    case  MT_CIPHER_KEY_LADDER_ESWCK:
                        p_keyladder_info->m2m_info.data2data_infro.enKeyOption0 = ADV_KEYOPTION_ESWCK;
                        break;
                    default:
                        MT_ERR_CIPHER("unknown keyladder source!\n");
                        kfree(p_keyladder_info);
                        return  MT_FAILURE;
                }
                ci->p_keyladder = (mt_handle)p_keyladder_info;
                ret = MT_SUCCESS;
            };
            break;
        case CMD_SEC_CIPHER_KEYLADDER_DESTROY:
            {
                mt_u32 i = 0;
                keyladder_info_struct *p_keyladder_info = (keyladder_info_struct *)arg;

                if (p_keyladder_info->keyladder_level > 0) {
                    for(i = 0; i < p_keyladder_info->keyladder_level; i ++) {
                        kfree(p_keyladder_info->keyladder_input[i]);
                    }
                }

                kfree(p_keyladder_info);
                MT_INFO_CIPHER("keyladder destroy done\n");
                ret = MT_SUCCESS;
            };
            break;
        case CMD_SEC_CIPHER_KEYLADDER_LINK:
            {
                //parse arg
                struct cipher_keyladder_priv *ci = (struct cipher_keyladder_priv*)arg;
                keyladder_info_struct *p_klad_info = (keyladder_info_struct *)ci->p_keyladder;
                MT_CIPHER_CTRL_S *p_ctrl = ci->p_ctrl;
                mt_u8 *input = ci->input;
                mt_u32 length = ci->length;
                mt_u32 cur_level = 0;

                if (p_klad_info == NULL || p_ctrl == NULL) {
                    MT_ERR_CIPHER("Bad cipher parameters\n");
                    return MT_FAILURE;
                }

                if (input == NULL || length == 0) {
                    MT_ERR_CIPHER("Invalid cipher input data\n");
                    return MT_FAILURE;
                }

                if(p_klad_info->keyladder_level >= ADV_MAXKELADDERLIMIT)
                {
                    MT_ERR_CIPHER("keyladder level too much\n");
                    return  MT_FAILURE;
                }

                cur_level = p_klad_info->keyladder_level;
                //NOTE: must be freed after destroy called
                p_klad_info->keyladder_input[cur_level] = (mt_u8 *)kmalloc(length, GFP_DMA);
                if (p_klad_info->keyladder_input[cur_level] == NULL) {
                    MT_ERR_CIPHER("Malloc for keyladder input failed\n");
                    return MT_FAILURE;
                }

                copy_from_user(p_klad_info->keyladder_input[cur_level], (void __user*)input, length);
                cipher_info_transfer(&p_klad_info->ladder_info[cur_level], p_ctrl, MT_TRUE);
                p_klad_info->m2m_info.enKeyInfor.key_infor[cur_level] =
                    (EN_ADV_KEYCONFIGINFO *)((mt_u32)&p_klad_info->ladder_info[cur_level] | 0xa0000000);
                //flush all data for firmware usage
                mt_flush_data_cache_range((mt_u8 *)(&p_klad_info->ladder_info[cur_level]), sizeof(EN_ADV_KEYCONFIGINFO));
                mt_flush_data_cache_range((mt_u8 *)(p_klad_info->keyladder_input[cur_level]), length);
                //update keyladder info
                p_klad_info->ladder_info[cur_level].pu8Input = (mt_u8 *)((mt_u32)p_klad_info->keyladder_input[cur_level] | 0xa0000000);
                p_klad_info->ladder_info[cur_level].mt_u32Length = length;
                p_klad_info->keyladder_level++;
                p_klad_info->m2m_info.enKeyInfor.u8_process_count = p_klad_info->keyladder_level;
                MT_INFO_CIPHER("keyladder link:level=%d\n", p_klad_info->keyladder_level);
                ret = MT_SUCCESS;
            };
            break;
        case CMD_SEC_CIPHER_HASH_CREATE:
            {
                struct cipher_hash_data_priv *ci = (struct cipher_hash_data_priv *)arg;
                MT_CIPHER_HASH_TYPE_E hash_type = ci->hash_type;

                hash_handle *p_hash_info = kmalloc(sizeof(hash_handle), GFP_DMA); //must be freed after final called
                if(p_hash_info == NULL) {
                    MT_ERR_CIPHER("Hash create failure!\n");
                    return MT_FAILURE;
                }

                memset(p_hash_info, 0, sizeof(hash_handle));

                ret = mt_drv_mmz_alloc_and_map("hash", "ddr", 64, 0, &p_hash_info->hash_result);
                if (ret != MT_SUCCESS) {
                    MT_ERR_CIPHER("Malloc for hash result failed\n");
                    kfree(p_hash_info);
                    p_hash_info = NULL;
                    return MT_FAILURE;
                }

                switch(hash_type)
                {
                    case MT_CIPHER_HASH_TYPE_SHA1:
                        p_hash_info->info.enAlgorithm = ADV_ALGORITHM_SHA;
                        break;
                    case MT_CIPHER_HASH_TYPE_SHA224:
                        p_hash_info->info.enAlgorithm = ADV_ALGORITHM_SHA224;
                        break;
                    case MT_CIPHER_HASH_TYPE_SHA256:
                        p_hash_info->info.enAlgorithm = ADV_ALGORITHM_SHA256;
                        break;
                    case MT_CIPHER_HASH_TYPE_SHA384:
                        p_hash_info->info.enAlgorithm = ADV_ALGORITHM_SHA384;
                        break;
                    case MT_CIPHER_HASH_TYPE_SHA512:
                        p_hash_info->info.enAlgorithm = ADV_ALGORITHM_SHA512;
                        break;
                    default:
                        MT_ERR_CIPHER("Invalid hash type\n");
                        mt_drv_mmz_unmap_and_release(&p_hash_info->hash_result);
                        kfree(p_hash_info);
                        return MT_FAILURE;
                }
                //success
                ci->hash_handle = (mt_handle)p_hash_info;
                ret = MT_SUCCESS;
            };
            break;
        case CMD_SEC_CIPHER_HASH_UPDATE:
            {
                struct cipher_hash_data_priv *ci = (struct cipher_hash_data_priv*)arg;
                hash_handle *p_handle = (hash_handle *)ci->hash_handle;
                /* phy addr from user */
                mt_u8 *p_data = ci->p_data;
                mt_u32 length = ci->length;
                ipc_msg_t msg = {0};
                ipc_msg_t recv_msg = {0};
                EN_ADV_KEYCONFIGINFO *p_info = NULL;

                if (p_handle == NULL) {
                    MT_ERR_CIPHER("Invalid hash handle\n");
                    return MT_FAILURE;
                }

                if (p_data == NULL || length == 0) {
                    MT_ERR_CIPHER("Bad hash parameters\n");
                    return MT_FAILURE;
                }

                p_info = &p_handle->info;
                p_info->pu8Input = (mt_u8 *)((mt_u32 )p_data | 0xa0000000);
                p_info->pu8Output = (mt_u8 *)(p_handle->hash_result.u32StartPhyAddr | 0xa0000000);
                p_info->mt_u32Length = length;

                msg.msg_id = MB_T_CRYPTO_ENGINE;
                msg.param1 = (unsigned int)((mt_u32)p_info | 0xa0000000);
                msg.param2 = 0;

                //flush all data fro firmware usage
                mt_flush_data_cache_range((mt_u8 *)p_info, sizeof(EN_ADV_KEYCONFIGINFO));

                ipcs_send_mbx_msg(0 , &msg);
                ret = ipcs_recv_mbx_msg(0, &recv_msg);

                if(msg.msg_id == recv_msg.msg_id && ret == 0)
                {
                    ret = MT_SUCCESS;
                }
                else
                {
                    MT_ERR_CIPHER("MCPU: FAIL Crypto enc,msg_id = 0x%x,ret = %x\n", msg.msg_id, ret);
                    ret = MT_FAILURE;
                }
            };
            break;
        case CMD_SEC_CIPHER_HASH_FINAL:
            {
                struct cipher_hash_data_priv *ci = (struct cipher_hash_data_priv *)arg;
                hash_handle *p_handle = (hash_handle *)ci->hash_handle;
                mt_u8 *p_output_hash = ci->p_output_hash;
                mt_u32 copy_length = 0;

                if (p_handle == NULL) {
                    MT_ERR_CIPHER("hash_handle is NULL!\n");
                    return MT_FAILURE;
                }

                if (p_output_hash == NULL) {
                    MT_ERR_CIPHER("Invalid hash output buffer\n");
                    return MT_FAILURE;
                }

                switch(p_handle->info.enAlgorithm)
                {
                    case ADV_ALGORITHM_SHA:
                        copy_length = 160/8;
                        break;
                    case ADV_ALGORITHM_SHA224:
                        copy_length = 224/8;
                        break;
                    case ADV_ALGORITHM_SHA256:
                        copy_length = 256/8;
                        break;
                    case ADV_ALGORITHM_SHA384:
                        copy_length = 384/8;
                        break;
                    case ADV_ALGORITHM_SHA512:
                        copy_length = 512/8;
                        break;
                    default:
                        MT_ERR_CIPHER("unknown hash type!\n");
                        return MT_FAILURE;
                        break;
                }

                copy_to_user((void __user*)p_output_hash, (const void *)(p_handle->hash_result.u32StartVirAddr), copy_length);
                mt_drv_mmz_unmap_and_release(&p_handle->hash_result);
                kfree(p_handle);
                ret = MT_SUCCESS;
            };
            break;
        case CMD_SEC_CIPHER_DATA_PROCESS:
            {
                struct cipher_data_priv *ci = (struct cipher_data_priv *)arg;
                //phy address from user mmz
                mt_u8 *p_src_addr = ci->p_src_addr;
                mt_u8 *p_dest_addr = ci->p_dest_addr;
                mt_u32 length = ci->length;

                EN_ADV_KEYCONFIGINFO *p_info = NULL;
                EN_ADV_M2MCONFIGINFO *p_m2m_info = NULL;

                ipc_msg_t msg = {0};
                ipc_msg_t recv_msg = {0};
                
                p_handle = (cipher_handle *)ci->cipher;
                if (p_handle == NULL) {
                    MT_ERR_CIPHER("Invalid cipher handle\n");
                    return MT_FAILURE;
                }

                if (length == 0 || p_src_addr == NULL) {
                    MT_ERR_CIPHER("Bad cipher parameters\n");
                    return MT_FAILURE;
                }

                if(p_dest_addr == NULL) {
                    MT_INFO_CIPHER("this processing is load HDCP!\n");
                }

                if (p_handle->key_ladder_handle != NULL) {
                    p_info = &p_handle->key_ladder_handle->m2m_info.data2data_infro;
                    p_m2m_info = &p_handle->key_ladder_handle->m2m_info;
                    if (p_info == NULL || p_m2m_info == NULL) {
                        MT_ERR_CIPHER("Bad key ladder handle when processing data\n");
                        ret = MT_FAILURE;
                        break;//to end of this case
                    }

                    if (p_handle->key_ladder_handle->m2m_info.data2data_infro.enKeyOption0 == ADV_KEYOPTION_PVR) {
                        msg.msg_id = MB_T_PVR_SETTING;
                    } else if(p_handle->key_ladder_handle->m2m_info.data2data_infro.enKeyOption0 == ADV_KEYOPTION_FLASH) {
                        msg.msg_id = MB_T_FLASH_OP;
                    } else {
                        msg.msg_id = MB_T_M2M_SETTING;
                    }

                    msg.param1 = (unsigned int)((mt_u32)p_m2m_info | 0xa0000000);
                    p_info->pu8Input = (mt_u8 *)((mt_u32)p_src_addr | 0xa0000000);

                    if (p_info->enAlgorithm == ADV_ALGORITHM_RSA) {
                        p_info->info_RSAkey.p_addr_d = (mt_u32 *)p_info->pu8Input;
                    }

                    if(p_dest_addr) {
                        p_info->pu8Output = (mt_u8 *)((mt_u32)p_dest_addr | 0xa0000000);
                    }
                    p_info->mt_u32Length = length;

                    mt_flush_data_cache_range((mt_u8 *)p_m2m_info, sizeof(EN_ADV_M2MCONFIGINFO));
                    if (p_handle->key_ladder_handle->keyladder_level > 0) {
                        mt_flush_data_cache_range((mt_u8 *)p_handle->key_ladder_handle->ladder_info,
                                sizeof(EN_ADV_KEYCONFIGINFO) * p_handle->key_ladder_handle->keyladder_level);
                    }
                    MT_INFO_CIPHER("processing with keyladder\n");
                }
                else //no keyladder data processing
                {
                    p_info = &p_handle->info;
                    if (p_info == NULL) {
                        break; //goto end of this case
                    }

                    msg.msg_id = MB_T_CRYPTO_ENGINE;
                    if(p_dest_addr) {
                        p_info->pu8Output = (mt_u8*)((mt_u32)p_dest_addr | 0xa0000000);
                    }

                    p_info->pu8Input = (mt_u8*)((mt_u32)p_src_addr | 0xa0000000);

                    if (p_info->enAlgorithm == ADV_ALGORITHM_RSA) {
                        p_info->info_RSAkey.p_addr_d = (mt_u32 *)p_info->pu8Input;
                    }
                    p_info->mt_u32Length = length;
                    msg.param1 = (unsigned int)((mt_u32)p_info | 0xa0000000);
                    mt_flush_data_cache_range((mt_u8 *)p_info, sizeof(EN_ADV_KEYCONFIGINFO));
                }

                msg.param2 = 0;

                ipcs_send_mbx_msg(0 , &msg);
                ret = ipcs_recv_mbx_msg(0, &recv_msg);

                if (msg.msg_id == recv_msg.msg_id && ret == 0) {
                    MT_INFO_CIPHER("processing success\n");
                    ret = MT_SUCCESS;
                }
                else
                {
                    MT_ERR_CIPHER("MCPU: FAIL Crypto enc,send msg_id = %x,recv_msg.msg_id = %x  ret = %x\n", msg.msg_id, recv_msg.msg_id, ret);
                    ret = MT_FAILURE;
                }
            };
            break;
        case CMD_SEC_CIPHER_SET_CW:
            {
                struct cipher_cw_priv *ci = (struct cipher_cw_priv *)arg;
                MT_CIPHER_CW_INFO_S *p_cw_info = ci->p_cw_info;
                mt_u8 *p_src_addr = ci->p_src_addr;
                mt_u32 length = ci->length;
                mt_u8 *in_addr = NULL;

                ipc_msg_t msg = {0};
                ipc_msg_t recv_msg = {0};
                EN_ADV_KEYCONFIGINFO *p_info = NULL;
                EN_ADV_M2MCONFIGINFO *p_m2m_info = NULL;

                p_handle = (cipher_handle*)ci->cipher_handle;
                if (p_handle == NULL) {
                    MT_ERR_CIPHER("Invalid cipher handle\n");
                    return MT_FAILURE;
                }

                if (p_handle->key_ladder_handle == NULL) {
                    MT_ERR_CIPHER("please set keyladder handle for CW!\n");
                    return MT_FAILURE;
                }

                if (p_cw_info == NULL || p_src_addr == NULL || length == 0) {
                    MT_ERR_CIPHER("Bad cipher parameters\n");
                    return MT_FAILURE;
                }

                p_info = &p_handle->key_ladder_handle->m2m_info.data2data_infro;
                p_m2m_info = &p_handle->key_ladder_handle->m2m_info;

                if(p_cw_info->type == MT_CIPHER_CW_EVEN_KEY)
                {
                    p_m2m_info->enCWType = ADV_CWTYPE_EVEN;
                }
                else if(p_cw_info->type == MT_CIPHER_CW_ODD_KEY)
                {
                    p_m2m_info->enCWType = ADV_CWTYPE_ODD;
                }
                else
                {
                    MT_ERR_CIPHER("unknown CW type!\n");
                    return MT_FAILURE;
                }

                p_m2m_info->u8CWIndex = p_cw_info->descrambler_id;
                p_m2m_info->u8CWAddr = p_cw_info->offset;
                p_m2m_info->u8CWLen = p_cw_info->length;

                in_addr = (mt_u8*)kmalloc(length, GFP_DMA);
                if (in_addr == NULL) {
                    MT_ERR_CIPHER("Malloc for input data failed\n");
                    return MT_FAILURE;
                }

                copy_from_user(in_addr, (void __user*)p_src_addr, length);
                p_info->pu8Input = (mt_u8 *)((mt_u32 )in_addr | 0xa0000000);
                p_info->mt_u32Length = length;
                mt_flush_data_cache_range((mt_u8 *)p_m2m_info, sizeof(EN_ADV_M2MCONFIGINFO));
                mt_flush_data_cache_range((mt_u8 *)p_handle->key_ladder_handle->ladder_info,
                        sizeof(EN_ADV_KEYCONFIGINFO) * p_handle->key_ladder_handle->keyladder_level);
                msg.msg_id = MB_T_DESC_ADV_CW_SETTING;
                msg.param1 = (unsigned int)((mt_u32)p_m2m_info | 0xa0000000);
                msg.param2 = 0;

                mt_flush_data_cache_range(in_addr, length);

                ipcs_send_mbx_msg(0 , &msg);
                ret = ipcs_recv_mbx_msg(0, &recv_msg);

                if (msg.msg_id == recv_msg.msg_id && ret == 0) {
                    ret = MT_SUCCESS;
                } else {
                    MT_ERR_CIPHER("MCPU: FAIL Crypto enc,msg_id = %x,ret = %x\n", msg.msg_id, ret);
                    ret = MT_FAILURE;
                }

                kfree(in_addr);
            };
            break;
        case CMD_SEC_CIPHER_GET_PVR_KEY:
            {
                struct cipher_pvr_priv *ci = (struct cipher_pvr_priv *)arg;
                mt_u32 channel_id = ci->channel_id;
                mt_u32 key_length = ci->key_length;
                mt_u8 *p_pvr_key = ci->p_pvr_key;
                EN_ADV_M2MCONFIGINFO *p_m2m_info = NULL;

                ipc_msg_t msg = {0};
                ipc_msg_t recv_msg = {0};

                if (p_pvr_key == NULL) {
                    MT_ERR_CIPHER("Invalid pvr key\n");
                    return MT_FAILURE;
                }

                p_m2m_info = kmalloc(sizeof(EN_ADV_M2MCONFIGINFO), GFP_DMA);
                if (p_m2m_info == NULL) {
                    MT_ERR_CIPHER("Malloc for m2m info failed\n");
                    return MT_FAILURE;
                } else {
                    mt_u8 *p_output = kmalloc(key_length, GFP_DMA);
                    if (p_output == NULL) {
                        ret = MT_FAILURE;
                        kfree(p_m2m_info);
                        p_m2m_info = NULL;
                        return MT_FAILURE;
                    } else {
                        msg.msg_id = MB_T_PVRKEY_SETTING;
                        mt_flush_data_cache_range(p_output, key_length);
                        mt_invaild_data_cache_range(p_output, key_length);
                        msg.param1 = (unsigned int)((mt_u32)p_m2m_info | 0xa0000000);
                        p_m2m_info->data2data_infro.pu8Output = (mt_u8*)((mt_u32)p_output | 0xa0000000);
                        p_m2m_info->data2data_infro.key_index = channel_id;
                        p_m2m_info->data2data_infro.enKeySize = key_length;
                        mt_flush_data_cache_range((mt_u8 *)p_m2m_info, sizeof(EN_ADV_M2MCONFIGINFO));

                        ipcs_send_mbx_msg(0 , &msg);
                        ret = ipcs_recv_mbx_msg(0, &recv_msg);

                        if (msg.msg_id == recv_msg.msg_id && ret == 0) {
                            copy_to_user((void __user *)p_pvr_key, p_output, key_length);
                            ret = MT_SUCCESS;
                        } else {
                            MT_ERR_CIPHER("MCPU: FAIL get pvr key,msg_id = %x,ret = %x\n", msg.msg_id, ret);
                            ret = MT_FAILURE;
                        }
                        kfree(p_m2m_info);
                        kfree(p_output);
                    }
                }
            };
            break;
        case CMD_SEC_CIPHER_GET_RND_NUM:
            {
                mt_u32 *p_random_number = (mt_u32 *)arg;

                ipc_msg_t msg = {0};
                ipc_msg_t recv_msg = {0};

                msg.msg_id = MB_T_RT_GET_RANDOM_DATA;
                ipcs_send_mbx_msg(0 , &msg);
                ret = ipcs_recv_mbx_msg(0, &recv_msg);

                if (ret == MT_SUCCESS) {
                    if(msg.msg_id == recv_msg.msg_id && ret == 0) {
                        //*p_random_number = recv_msg.param1;
                        copy_to_user((void __user*)p_random_number, &recv_msg.param1, 4);
                        ret = MT_SUCCESS;
                    } else {
                        MT_ERR_CIPHER( "MCPU: FAIL specail command,msg_id = %x,ret = %x\n", msg.msg_id, ret);
                        ret = MT_FAILURE;
                    }
                }
            };
            break;
        case CMD_SEC_CIPHER_SPECIAL_CMD:
            {
                struct cipher_cmd_priv *ci = (struct cipher_cmd_priv *)arg;
                MT_CIPHER_SPECIAL_COMMAND_E cmd_id = ci->cmd_id;
                mt_u32 in_len = ci->in_len;
                mt_u32 *p_input = NULL;
                mt_u32 out_len = 0;
                mt_u32 *p_output = NULL;

                ipc_msg_t msg = {0};
                ipc_msg_t recv_msg = {0};
                msg.msg_id = cmd_id;

                if(ci->p_input == NULL && ci->in_len == 0
                        && ci->p_output == NULL && ci->out_len == 0)
                {
                    ipcs_send_mbx_msg(0 , &msg);
                    return MT_SUCCESS;
                }

                if (ci->p_input == NULL || ci->in_len == 0) {
                    MT_ERR_CIPHER("Bad command parameters\n");
                    return MT_FAILURE;
                }

                p_input = kmalloc(in_len, GFP_DMA);
                if (p_input == NULL) {
                    MT_ERR_CIPHER("Malloc for command input data failed\n");
                    return MT_FAILURE;
                }

                copy_from_user(p_input, (void __user*)ci->p_input, in_len);
                mt_flush_data_cache_range((mt_u8 *)p_input, in_len);
                msg.param1 = (unsigned int)(virt_to_phys((volatile const void *)p_input) | 0xa0000000);

                if (ci->p_output != NULL) {
                    out_len = ci->out_len;
                    p_output = kmalloc(out_len, GFP_DMA);
                    if (p_output == NULL) {
                        kfree(p_input);
                        return MT_FAILURE;
                    }

                    mt_flush_data_cache_range((mt_u8 *)p_output, out_len);
                    mt_invaild_data_cache_range((mt_u8 *)p_output, out_len);
                    msg.param2 = (unsigned int)(virt_to_phys((volatile const void *)p_output) | 0xa0000000);
                }
                ipcs_send_mbx_msg(0 , &msg);
                ret = ipcs_recv_mbx_msg(0, &recv_msg);

                if (msg.msg_id == recv_msg.msg_id && ret == 0) {
                    if (ci->p_output != NULL)
                        copy_to_user((void __user*)ci->p_output, p_output, out_len);
                    ret = MT_SUCCESS;
                } else {
                    MT_ERR_CIPHER( "MCPU: Special CMD FAIL,msg_id = %x,ret = %x\n", msg.msg_id, ret);
                    ret = MT_FAILURE;
                }

                kfree(p_input);
                if (p_output != NULL) {
                    kfree(p_output);
                    p_output = NULL;
                }
            };
            break;
        case CMD_SEC_CIPHER_SEND_MSG:
            {
                struct cipher_send_msg_priv *ci = (struct cipher_send_msg_priv *)arg;
                mt_u32 channel_id = ci->channel_id;
                mt_u32 wait_ack = ci->ack;
                mt_u32 msg_id = ci->msg_id;
                mt_u32 param1 = ci->param1;
                mt_u32 param2 = ci->param2;

                ipc_msg_t msg = {0};
                ipc_msg_t recv_msg = {0};

                if (channel_id >= (MAX_CHANNEL_INDEX-1)/2) {
                    ret = MT_FAILURE;
                }

                msg.msg_id = msg_id;
                msg.param1 = param1;
                msg.param2 = param2;

                ipcs_send_mbx_msg((channel_id*2+1), &msg);
                if (wait_ack) {
                    ret = ipcs_recv_mbx_msg((channel_id*2+1), &recv_msg);
                    if (msg.msg_id == recv_msg.msg_id && ret == 0) {
                        ret = MT_SUCCESS;
                    } else {
                        MT_ERR_CIPHER("MCPU: FAIL specail command,msg_id = %x,ret = %x\n", msg.msg_id, ret);
                        ret = MT_FAILURE;
                    }
                } else {
                    //I do not want response
                    ret = MT_SUCCESS;
                }
            }
            break;
        case CMD_SEC_CIPHER_RECV_MSG:
            {
                struct cipher_recv_msg_priv *ci = (struct cipher_recv_msg_priv *)arg;
                mt_u32 channel_id = ci->channel_id;
                mt_u32 send_ack = ci->ack;
                mt_u32 *p_msg_id = ci->p_msg_id;
                mt_u32 *p_param1 = ci->p_param1;
                mt_u32 *p_param2 = ci->p_param2;

                ipc_msg_t msg = {0};

                if (channel_id >= (MAX_CHANNEL_INDEX-1)/2) {
                    return MT_FAILURE;
                }

                if (p_msg_id == NULL || p_param1 == NULL || p_param2 == NULL) {
                    MT_ERR_CIPHER("Bad parameters\n");
                    return MT_FAILURE;
                }

                ret = ipcs_recv_mbx_msg((channel_id*2+2), &msg);
                if (ret == 0) {
                    *p_msg_id = msg.msg_id;
                    *p_param1 = msg.param1;
                    *p_param2 = msg.param2;
                    if (send_ack) {
                        ipcs_send_mbx_msg((channel_id*2+2), &msg);
                        ret = MT_SUCCESS;
                    }
                }

            };
            break;
    }
    return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
static  long symphony_cipher_ioctl( struct file *filp,
        unsigned int cmd, unsigned long arg)
{
    struct inode *inode;
    inode = filp->f_path.dentry->d_inode;
    return cipher_usercopy(inode, filp, cmd, arg, cipher_ioctl);
}
#else
static  int symphony_cipher_ioctl(struct inode *inode, struct file *filp,
        unsigned int cmd, unsigned long arg)
{
    return cipher_usercopy(inode, filp, cmd, arg, cipher_ioctl);
}
#endif

static struct file_operations cipher_fops = {
    .owner   = THIS_MODULE,
    .open    = cipher_open,
    .release = cipher_release,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
    .unlocked_ioctl   = symphony_cipher_ioctl,
#else
    .ioctl   = symphony_cipher_ioctl,
#endif
};

static mt_device_s    CipherDev;
int cipher_scpu_init(void)
{
    strncpy(CipherDev.devfs_name, UMAP_DEVNAME_CIPHER, sizeof(CipherDev.devfs_name) - 1);
    CipherDev.fops   = &cipher_fops;
    CipherDev.minor  = UMAP_MIN_MINOR_CIPHER;
    CipherDev.owner  = THIS_MODULE;
    CipherDev.drvops = NULL;

    if (mt_drv_dev_register(&CipherDev) < 0)
    {
        MT_ERR_CIPHER("Reg cipher dev failed\n");
        return MT_FAILURE;
    }

    MT_INFO_CIPHER("Cipher init done\n");
    return MT_SUCCESS;
}


void cipher_scpu_cleanup(void)
{
    MT_INFO_CIPHER("cipher_cleanup\n");
    unregister_chrdev_region(MKDEV(cipher_major,0), 1);
    MT_INFO_CIPHER("cipher_cleanup finished\n");
}

module_param(cipher_major, int, 0);

