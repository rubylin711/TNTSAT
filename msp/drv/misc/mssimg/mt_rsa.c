/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/string.h>
#include <linux/printk.h>
#include "mt_common.h"
#include "mt_rsa.h"

#include "drv_mss_rsa.h"


/* NOTE: only support single local RSA instance */
static MT_RSA l_RSA;

//#define RSA_PRINTK printk
#define RSA_PRINTK(...) do{}while(0)

MT_RSA *MT_RSA_new(void)
{
	return &l_RSA;
}

void MT_RSA_delete(MT_RSA *rsa)
{
	//do nothing
	return;
}

int MT_RSA_set_public_key(MT_RSA *r,
                          const unsigned char *key, unsigned int length)
{
	if (r == NULL || key == NULL || length <= 0)
	{
		RSA_PRINTK("%s: err invalid parameters(%p %p %u)!\n",__FUNCTION__,r,key,length);
		return (-1);
	}

	r->pub_key = (unsigned char *)key;
	r->pub_key_length = length;

	return 0;
}

int MT_RSA_verify(int type, const unsigned char *m, unsigned int m_length,
                  const unsigned char *sigbuf, unsigned int siglen, MT_RSA *rsa)
{
	int ret =0;	
    struct rsa_public_key pubkey;
    unsigned char e[256];
    int algo = type;
    RSA_PRINTK("MT_RSA_verify\n");
	if (m == NULL || sigbuf == NULL || rsa == NULL || m_length <= 0 || siglen <= 0)
	{
		RSA_PRINTK("%s: err invalid parameters(%p %u %p %u %p)!\n",__FUNCTION__,m,m_length,sigbuf,siglen,rsa);
		return (-1);
	}

	if (rsa->pub_key == NULL || rsa->pub_key_length <= 0)
	{
		RSA_PRINTK("%s: err invalid rsa(%p %u)!\n",__FUNCTION__,rsa->pub_key,rsa->pub_key_length);
		return (-1);
	}

	/* 65537 */
	memset(e, 0, 256);
	e[255] = 0x01;
	e[253] = 0x01;

	memset(&pubkey, 0, sizeof(struct rsa_public_key));
	pubkey.e_length = 256;
	pubkey.e = e;
	pubkey.n_length = rsa->pub_key_length;
	pubkey.n = rsa->pub_key;

	ret = mt_rsa_verify(&pubkey, algo, m, m_length, sigbuf, siglen);
	if (0 != ret)
	{
		RSA_PRINTK("%s: err, rsa verify failed(%d)!\n",__FUNCTION__,ret);
		return ret;
	}

	return ret;
}

