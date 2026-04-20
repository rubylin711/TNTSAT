/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/string.h>
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include "mt_type.h"
#include "mt_common.h"
#include "mt_aes.h"
#include "drv_ce_if.h"
#include "drv_kt_if.h"

#include "drv_mss_rsa.h"

#include "mt_cache.h"


#define mt_goto_fail(func)        \
    {                          \
	goto _##func##_failed; \
    }

//#define AES_PRINTK printk
#define AES_PRINTK(...) do{}while(0)


static inline void mt_aes_flush_dcache_range(ulong addr, ulong size)
{
    ulong residue = addr & (CACHE_LINE_SIZE - 1);
    addr &= ~residue;
    size = (size + residue + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
    mt_dcache_flush((void*)addr, size);
}


static int mt_cipher_keyslot_set_iv(unsigned int keyslot, unsigned char *p_iv, unsigned int ivsize)
{

	 int ret = MT_SUCCESS;
     char iv[16];

    if (keyslot == MT_AES_CIPHER_KEYSLOT_INVALID)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (p_iv == NULL)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (ivsize != 8 && ivsize != 16)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    memcpy(iv, p_iv, ivsize);
    drv_kt_write_iv(drv_ce_get_handle(), keyslot, iv, ivsize);

    return ret;
}



static int mt_cipher_crypto_create(u8 index, unsigned long *p_crypto)
{

	int ret = MT_SUCCESS;
    //int ce_fd = -1;
	mt_session session =0;

    if ( p_crypto == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    ret = drv_ce_ades_create(&session, index, drv_ce_get_handle());

    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_CREATE;
        mt_goto_fail(crypto_create);
    }

    *p_crypto = (unsigned long)session;

_crypto_create_failed:
    return ret;
}

static int mt_cipher_crypto_config(unsigned long crypto, unsigned int key_slot)
{
 	int ret = MT_SUCCESS;
    //int ce_fd = -1;
    mt_session session  = (mt_session)crypto;
	MT_CE_ADES_CTRL_S ctrl;

    if (session == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        mt_goto_fail(crypto_config);
    }

    if ( key_slot >= MT_AES_CIPHER_KEYSLOT_INVALID) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        mt_goto_fail(crypto_config);
    }

//default AES_cbc_decrypt for avcpu
    ctrl.operation = MT_CE_ADES_OPERATION_DECRYPT;
	ctrl.ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_AES128;
	ctrl.ades_para.work_mode = MT_CE_ADES_WORK_MODE_CBC;
    ctrl.ades_para.key_slot = key_slot;

	ret = drv_ce_ades_config(session, &ctrl);

    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_CONFIG;
        mt_goto_fail(crypto_config);
    }

_crypto_config_failed:
    return ret;

}

static int mt_cipher_crypto_process(unsigned long crypto, unsigned char *p_src_addr, unsigned char *p_dest_addr, unsigned int length)
{
    int ret = MT_SUCCESS;
    //int ce_fd = -1;
    mt_session session  = (mt_session)crypto;
	mt_u8 *p_src_addr_fin;
  	mt_u8 *p_dst_addr_fin;
	mt_u8 is_phy_addr;
	mt_u32 length_fin;


    if (session == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        mt_goto_fail(crypto_process);
    }

    if (p_src_addr == NULL || p_dest_addr == NULL || length == 0) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        mt_goto_fail(crypto_process);
    }

    mt_aes_flush_dcache_range((ulong)p_src_addr, length);

    p_src_addr_fin = (mt_u8 *)(ulong)virt_to_phys((void *)p_src_addr);
	p_dst_addr_fin = (mt_u8 *)(ulong)virt_to_phys((void *)p_dest_addr);
	AES_PRINTK("in src:0x%x,dst:0x%x,out src:0x%x,dst:0x%x\n",p_src_addr,p_dest_addr,p_src_addr_fin,p_dst_addr_fin);
	is_phy_addr=1;


    length_fin = length;


	 ret = drv_ce_ades_process(session, is_phy_addr,
		    (phys_addr_t)(ulong)p_src_addr_fin, (phys_addr_t)(ulong)p_dst_addr_fin, 0, length_fin);

    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_PROCESS;
        mt_goto_fail(crypto_process);
    }

_crypto_process_failed:
    return ret;

}

static int mt_cipher_crypto_destroy(unsigned long crypto)
{
    int ret = MT_SUCCESS;
    //int ce_fd = -1;
    mt_session session  = (mt_session)crypto;

    if (session == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        mt_goto_fail(crypto_destroy);
    }

    ret = drv_ce_ades_destroy(session);

_crypto_destroy_failed:
    return ret;
}


int MT_AES_set_decrypt_keyslot(const unsigned int keyslot,
                               MT_AES_KEY *key)
{
	if (key == NULL || keyslot >= MT_AES_CIPHER_KEYSLOT_INVALID)
	{
		AES_PRINTK("%s: err invalid parameters(%p %x)!\n",__FUNCTION__,key,keyslot);
		return (-1);
	}

	key->keyslot = keyslot;

	return 0;
}

int MT_AES_cbc_encrypt(const unsigned char *in, unsigned char *out,
                       size_t length, const MT_AES_KEY *key,
                       unsigned char *ivec, const int enc)
{
	int ret =0;
    unsigned int iv_size = 16;
    unsigned long p_cipher;
	//void *p_priv = drv_kt_get_handle();
	//MT_KT_KEY_ATTR_S attr ={0};

	AES_PRINTK("priv:0x%x,attr:0x%x,keyslot:%d\n",p_priv,&attr,key->keyslot);

   	//drv_kt_read_attribute(p_priv, key->keyslot, &attr);
	//AES_PRINTK("kt attr AES_ONOFF:0x%x,DES_ONOFF:0x%x,TDES_ONOFF:0x%x,CSAv2_ONOFF:0x%x,CSAv3_ONOFF:0x%x,SM2_3_4_ONOFF:0x%x,ASA_ONOFF:0x%x,TS_ONOFF:0x%x,M2M_ONOFF:0x%x\n",
	 //attr.AES_ONOFF,attr.DES_ONOFF,attr.TDES_ONOFF,attr.CSAv2_ONOFF,attr.CSAv3_ONOFF,attr.SM2_3_4_ONOFF,attr.ASA_ONOFF,attr.TS_ONOFF,attr.M2M_ONOFF);
	//AES_PRINTK("kt attr MAC_ONOFF:0x%x,Multi2_ONOFF:0x%x,REE_ONOFF:0x%x,DEC_ONOFF:0x%x,ENC_ONOFF:0x%x,KEY_SIZE:0x%x,IV_SIZE:0x%x,KEY_SOURCE:0x%x,TDES_KEYCHK:0x%x\n",
	 //attr.MAC_ONOFF,attr.Multi2_ONOFF,attr.REE_ONOFF,attr.DEC_ONOFF,attr.ENC_ONOFF,attr.KEY_SIZE,attr.IV_SIZE,attr.KEY_SOURCE,attr.TDES_KEYCHK);

	if (in == NULL || out == NULL || length <= 0)
	{
		AES_PRINTK("%s: err invalid parameters(%p %p %u)!\n",__FUNCTION__,in,out,length);
		return (-1);
	}

	if (key == NULL || key->keyslot >= MT_AES_CIPHER_KEYSLOT_INVALID)
	{
		AES_PRINTK("%s: err invalid parameters(%p %x)!\n",__FUNCTION__,key,key->keyslot);
		return (-1);
	}

    if (key->keyslot < MT_AES_CIPHER_KEYSLOT_INVALID)
    {
        /* use preset keyslot */
        if (ivec)
        {
            ret = mt_cipher_keyslot_set_iv(key->keyslot, ivec, iv_size);
            if (ret != MT_SUCCESS)
            {
				AES_PRINTK("%s: err, cipher keyslot set iv failed(%d)!\n",__FUNCTION__,ret);
                goto Err1;
            }
        }
    }

	ret = mt_cipher_crypto_create(MT_CE_CHANNEL_0, &p_cipher);
	if (ret != MT_SUCCESS)
	{
		AES_PRINTK("%s: err, cipher crypto create failed(%d)!\n",__FUNCTION__,ret);
		goto Err1;
	}

	ret = mt_cipher_crypto_config(p_cipher, key->keyslot);
	if (ret != MT_SUCCESS)
	{
		AES_PRINTK("%s: err, cipher crypto config failed(%d)!\n",__FUNCTION__,ret);
		goto Err2;
	}

	ret = mt_cipher_crypto_process(p_cipher, (unsigned char*)in, out, length);
	if (ret != MT_SUCCESS)
	{
		AES_PRINTK("%s: err, cipher crypto process failed(%d)!\n",__FUNCTION__,ret);
		goto Err2;
	}

Err2:
	(void)mt_cipher_crypto_destroy(p_cipher);

Err1:
	return ret;
}


