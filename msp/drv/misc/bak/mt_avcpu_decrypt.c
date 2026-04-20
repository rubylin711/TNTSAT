/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include "mt_common.h"
#include "drv_sys_misc.h"
#include "mt_mach/clock.h"
#include "drv_sym4_reset.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/chipinfo.h"
#include "mt_cache.h"
#include <linux/slab.h>
#include "drv_mss_rsa.h"

#include "mssimg/mt_rsa.h"
#include "mssimg/mt_aes.h"


#define UBO_HEADER_LEN			0x400
#define OFFSET_PAYLOAD_LEN		0x0c
#define VERSION_LEN				0x10
#define SIGNATURE_LEN			0x100
#define STUFF_LEN				0x1000
#define AVCPU_SIZE 				2*1024*1024

#define MAX_DECRYPT_PARAM_LEN 20
#define MAX_DECRYPT_PARAM_NUM 4
#define RSA_PUB_KEY_COUNT		(sizeof(g_rsa_pub_key_tab) / sizeof(g_rsa_pub_key_tab[0]))


//#define DECRYPT_PRINTK printk
#define DECRYPT_PRINTK(...) do{}while(0)

extern struct mtd_info *get_mtd_device_nm(const char *name);
extern int mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen,u_char *buf);


enum command_ret_t {
  RET_SUCCESS,  /* 0 = Success */
  RET_FAILURE,  /* 1 = Failure */
  RET_USAGE = -1,  /* Failure, please report 'usage' error */
};

struct ubo_header_t
{
	unsigned char *payload;				/* payload = Stuff(4K) + AVCPU.img + Version */
	unsigned int payload_len;

	unsigned char *encrypt_data;		/* Stuff(4K) + AVCPU.img */
	unsigned int encrypt_len;

	unsigned char *version;
	unsigned int ver_len;

	unsigned char *sig;					/* signature address */
	unsigned int sig_len;
};

struct rsa_pub_key_table_t
{
	const char *name;
	unsigned char *key;
	unsigned int len;
};

typedef struct _avcpu_decrypt_param_t
{
	char rsapss[MAX_DECRYPT_PARAM_LEN];
	char rsakey[MAX_DECRYPT_PARAM_LEN];
    char keyslot_type[MAX_DECRYPT_PARAM_LEN];
    unsigned int keyslot_num;
}avcpu_decrypt_param_t;

static avcpu_decrypt_param_t g_avcpu_decrypt_param;
static unsigned char __attribute__((aligned(16))) AVCPU_RSA_Key[] = {
#include "mt_avcpu_rsa_pub_key.h"
};

static struct rsa_pub_key_table_t g_rsa_pub_key_tab[] =
{
	{ .name = "AVCPU_RSA_Key",
	  .key = AVCPU_RSA_Key,
	  .len = sizeof(AVCPU_RSA_Key) }

	//add your RSA Public Key here:
};

static MT_BOOL is_encrypt = FALSE;


MT_BOOL mt_avcpu_get_encry_status(void)
{
	return is_encrypt;
}

static void mt_avcpu_set_encry_status(MT_BOOL status)
{
	is_encrypt = status;
}

static int mt_avcpu_decrypt_gatparam(char *p)
{
	char tmp[MAX_DECRYPT_PARAM_NUM][MAX_DECRYPT_PARAM_LEN];
	int ret,num;	
	int i=0;
	int j=0;

    DECRYPT_PRINTK("decry start1\n");
	memset(tmp,0,sizeof(tmp));
	memset(&g_avcpu_decrypt_param,0,sizeof(avcpu_decrypt_param_t));
	if (!p)
		return -EINVAL;

	while (*p != '\0') {
    
		if(i>=MAX_DECRYPT_PARAM_NUM &&j>=MAX_DECRYPT_PARAM_LEN)
			break;
		tmp[i][j++] = *p++;
		
		if (*p == ',')
		{
			tmp[i][j]='\0';
			j=0;
			i++;
			p++;
		}	
	}
    tmp[i][j]='\0';
	
	//for(i=0; i<MAX_DECRYPT_PARAM_NUM;i++)
		//DECRYPT_PRINTK("avcpu_decrypt_gatparam i:%d,tmp:%s\n",i,tmp[i]);

	strcpy(g_avcpu_decrypt_param.rsapss,tmp[0]);
	strcpy(g_avcpu_decrypt_param.rsakey,tmp[1]);
	strcpy(g_avcpu_decrypt_param.keyslot_type,tmp[2]);
	ret=kstrtoint(tmp[3],0,&num);
	if (ret < 0)
	{
		DECRYPT_PRINTK("keyslot num convert fail\n");
	}
	g_avcpu_decrypt_param.keyslot_num = num;
    DECRYPT_PRINTK("avcpu_decrypt_gatparamc1:%s,c2:%s,c3:%s,c4:%d\n",g_avcpu_decrypt_param.rsapss,g_avcpu_decrypt_param.rsakey,g_avcpu_decrypt_param.keyslot_type,g_avcpu_decrypt_param.keyslot_num);
	if(g_avcpu_decrypt_param.rsapss !=NULL && g_avcpu_decrypt_param.rsakey !=NULL && g_avcpu_decrypt_param.keyslot_type != NULL)
		mt_avcpu_set_encry_status(TRUE);
	
}

early_param("avcpu_decrypt", mt_avcpu_decrypt_gatparam);

static int get_rsa_public_key(const char *name, unsigned char **key, unsigned int *len)
{
	int i;

	for (i=0; i<RSA_PUB_KEY_COUNT; i++)
	{
		if (strcmp(g_rsa_pub_key_tab[i].name, name) == 0)
		{
			*key = g_rsa_pub_key_tab[i].key;
			*len = g_rsa_pub_key_tab[i].len;
			return 0;
		}
	}

	DECRYPT_PRINTK("%s: err, %s not found!\n", __FUNCTION__, name);
	return (-1);
}

static unsigned int cpu_msb32(unsigned char *d)
{
	return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3];
}

static int parse_ubo_header(unsigned char *data, unsigned int len, struct ubo_header_t *header)
{
	DECRYPT_PRINTK("%s: data %p, len %u\n",__FUNCTION__,data,len);

	if (data == NULL)
	{
		DECRYPT_PRINTK("%s: err, bad data(%p)!\n", __FUNCTION__, data);
		return (-1);
	}

	if (len < UBO_HEADER_LEN)
	{
		DECRYPT_PRINTK("%s: err, bad data length(%u)!\n", __FUNCTION__, len);
		return (-1);
	}

	header->payload_len = cpu_msb32(data + OFFSET_PAYLOAD_LEN);

	if (header->payload_len + UBO_HEADER_LEN + SIGNATURE_LEN > len)
	{
		DECRYPT_PRINTK("%s: err, bad data length(%u)!\n", __FUNCTION__, len);
		return (-1);
	}

	header->payload = data + UBO_HEADER_LEN;
	header->encrypt_data = data + UBO_HEADER_LEN;
	header->encrypt_len = header->payload_len - VERSION_LEN;
	header->version = data + UBO_HEADER_LEN + header->payload_len - VERSION_LEN;
	header->ver_len = VERSION_LEN;
	header->sig = data + UBO_HEADER_LEN + header->payload_len;
	header->sig_len = SIGNATURE_LEN;
	DECRYPT_PRINTK("%s: payload %p, len %u\n",__FUNCTION__,header->payload,header->payload_len);
	DECRYPT_PRINTK("%s: encrypt %p, len %u\n",__FUNCTION__,header->encrypt_data,header->encrypt_len);
	DECRYPT_PRINTK("%s: version %p, len %u\n",__FUNCTION__,header->version,header->ver_len);
	DECRYPT_PRINTK("%s: signature %p, len %u\n",__FUNCTION__,header->sig,header->sig_len);

	return 0;
}

static int on_cmd_aes_cbc_decrypt(u32 store_addr,u32 run_addr,const char *key_type, u32 key_slot)
{
    int ret = RET_SUCCESS;
    unsigned char *img_addr;
    unsigned char *dst_addr;
    unsigned int img_len;
    unsigned int keyslot;
    MT_AES_KEY aes_key;
	struct ubo_header_t header;


	const char *kltype;
	kltype=key_type;
 	DECRYPT_PRINTK("on_cmd_aes_cbc_decrypt start,kltype:%s,key_slot:%d\n",key_type,key_slot);

    img_addr = store_addr+SIGNATURE_LEN;
    dst_addr = run_addr-STUFF_LEN;
    img_len  = AVCPU_SIZE-SIGNATURE_LEN;

    if (!strcmp(kltype, "-keyslot"))
        keyslot = key_slot;
	else
        return RET_USAGE;

	ret = parse_ubo_header(img_addr, img_len, &header);
	if (ret != 0)
		return RET_FAILURE;

	ret = MT_AES_set_decrypt_keyslot(keyslot, &aes_key);
	if (ret != 0)
		return RET_FAILURE;

	ret = MT_AES_cbc_encrypt(header.encrypt_data,
							dst_addr,
							(size_t)header.encrypt_len,
							&aes_key,
							NULL,
							MT_AES_DECRYPT);

	DECRYPT_PRINTK("%s: AES_cbc_encrypt %p %p %u %u, ret %d.\n",__FUNCTION__,
	     header.encrypt_data, dst_addr, header.encrypt_len, keyslot,
	     ret);

	if (ret != 0)
		return RET_FAILURE;
	
    return ret;
}

static int on_cmd_rsa_verify(u32 store_addr,u32 run_addr,const char *rsa_pss, const char *rsa_key)
{
    int ret = RET_SUCCESS;
    int algo;
    const char *param;
    unsigned char *pub_key;
	unsigned int key_len;
    unsigned char *img_addr;
	unsigned int img_len;
	MT_RSA *rsa;
	struct ubo_header_t header;

    DECRYPT_PRINTK("on_cmd_rsa_verify start\n");
    param = rsa_pss;
    if (strcmp(param, "pss_sha256") == 0)
        algo = MT_RSASSA_PKCS1_PSS_MGF1_SHA256;
    else if (strcmp(param, "pss_sha1") == 0)
        algo = MT_RSASSA_PKCS1_PSS_MGF1_SHA1;
    else if (strcmp(param, "sha256") == 0)
        algo = MT_RSASSA_PKCS1_V1_5_SHA256;
    else if (strcmp(param, "sha1") == 0)
        algo = MT_RSASSA_PKCS1_V1_5_SHA1;
    else {
        return RET_USAGE;
    }

	ret = get_rsa_public_key(rsa_key, &pub_key, &key_len);
	if (ret != 0)
		return RET_FAILURE;

    img_addr = store_addr+SIGNATURE_LEN;
    img_len  = AVCPU_SIZE-SIGNATURE_LEN;

	ret = parse_ubo_header(img_addr, img_len, &header);
	if (ret != 0)
		return RET_FAILURE;

	rsa = MT_RSA_new();
	if (rsa == NULL)
		return RET_FAILURE;

	ret = MT_RSA_set_public_key(rsa, pub_key, key_len);
	if (ret != 0)
		goto Err;

	ret = MT_RSA_verify(algo, header.payload, header.payload_len,
						header.sig, header.sig_len, rsa);

	DECRYPT_PRINTK("%s: RSA_verify %d (%p %u) (%p %u) (%p %u), ret %d.\n",__FUNCTION__,
	     algo, header.payload, header.payload_len, header.sig, header.sig_len,
	     pub_key, key_len,
	     ret);

Err:
	MT_RSA_delete(rsa);

	if (ret != 0)
		return RET_FAILURE;

	return ret;
}

static int on_cmd_rsa_verify_raw(u32 store_addr,u32 run_addr,const char *rsa_pss, const char *rsa_key)
{
    int ret = RET_SUCCESS;
    int algo;
    const char *param;
    unsigned char *pub_key;
	unsigned int key_len;
    unsigned char *msg_addr;
	unsigned int msg_len;
    unsigned char *sig_addr;
	unsigned int sig_len;
	MT_RSA *rsa;
	
    param = rsa_pss;
	DECRYPT_PRINTK("algo=%d,saddr=0x%x,raddr=0x%x,pss:%s,key:%s,p:%s\n",algo,store_addr,run_addr,rsa_pss,rsa_key,param);
	
    if (strcmp(param, "pss_sha256") == 0)
        algo = MT_RSASSA_PKCS1_PSS_MGF1_SHA256;
    else if (strcmp(param, "pss_sha1") == 0)
        algo = MT_RSASSA_PKCS1_PSS_MGF1_SHA1;
    else if (strcmp(param, "sha256") == 0)
        algo = MT_RSASSA_PKCS1_V1_5_SHA256;
    else if (strcmp(param, "sha1") == 0)
        algo = MT_RSASSA_PKCS1_V1_5_SHA1;
    else {
        return RET_USAGE;
    }
   ret = get_rsa_public_key(rsa_key, &pub_key, &key_len);
	if (ret != 0)
	{
		 DECRYPT_PRINTK("get_rsa_public_key fail\n");
		return RET_FAILURE;
	}	

    msg_addr = store_addr+SIGNATURE_LEN;
    msg_len  = UBO_HEADER_LEN;
    sig_addr = store_addr;
    sig_len  = SIGNATURE_LEN;

	rsa = MT_RSA_new();
	if (rsa == NULL)
	{
		 DECRYPT_PRINTK("MT_RSA_new fail\n");
		return RET_FAILURE;
	}	

	ret = MT_RSA_set_public_key(rsa, pub_key, key_len);
	if (ret != 0)
	{
		DECRYPT_PRINTK("MT_RSA_set_public_key fail\n");
		goto Err;
	}	

	ret = MT_RSA_verify(algo, msg_addr, msg_len,
						sig_addr, sig_len, rsa);

	DECRYPT_PRINTK("%s: RSA_verify %d (%p %u) (%p %u) (%p %u), ret %d.\n",__FUNCTION__,
	     algo, msg_addr, msg_len, sig_addr, sig_len,
	     pub_key, key_len,
	     ret);

Err:
	MT_RSA_delete(rsa);

	if (ret != 0)
		return RET_FAILURE;

	return ret;
}

#if 0
void dump_avcpuimg(u32 addr,u32 len)
{
	u32 code_addr = 0;
	u8 tmp[16]={0};
	u32 i=0;
	code_addr=addr;
	DECRYPT_PRINTK("------start-------\n");

	while(i<len)
	{
		tmp[i%16]=(u8)*(u32 *)(code_addr+i);
		
		 i++;
		if((i%16)==0)
		{
		    DECRYPT_PRINTK("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
				tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],tmp[9],tmp[10],tmp[11],tmp[12],tmp[13],tmp[14],tmp[15]);
			DECRYPT_PRINTK("\n");	
		}
	} 
	 DECRYPT_PRINTK("\n");
	 
	DECRYPT_PRINTK("------end-------\n");
}
#endif

u32 mt_avcpu_do_decrypt(u32 ddr_addr,avcpu_decrypt_param_t decry_param,struct mtd_info *mtd)
{
	u32 ret=RET_SUCCESS;
    u32 ktmp_size = 0;
	void *tmp_addr;
	u32 ktmp_addr=0;
	u32 retlen=0;
	 
	ktmp_size= ALIGN(AVCPU_SIZE, CACHE_LINE_SIZE);
	tmp_addr = kmalloc(ktmp_size , GFP_KERNEL);	
	DECRYPT_PRINTK("avcpu_decrypt addr:0x%x,ktmp_size:0x%x,mtd:0x%x\n",(u32)tmp_addr,ktmp_size,mtd);
	ktmp_addr=(u32)tmp_addr; 
	ret=mtd_read(mtd, 0, ktmp_size,&retlen, ktmp_addr);
				
	DECRYPT_PRINTK("retlen:0x%x\n",retlen);
	if(ret ||(retlen != ktmp_size))
	return RET_FAILURE;

	DECRYPT_PRINTK("param rsapss:%s,rsakey:%s,type:%s,num:%d\n",decry_param.rsapss,decry_param.rsakey,decry_param.keyslot_type,decry_param.keyslot_num);
	
	ret=on_cmd_rsa_verify_raw(ktmp_addr,ddr_addr,decry_param.rsapss, decry_param.rsakey);
    if(ret!=RET_SUCCESS)
    {
    	DECRYPT_PRINTK("RSA_verify_raw fail\n");
		goto  exit;
    }
	ret=on_cmd_rsa_verify(ktmp_addr,ddr_addr,decry_param.rsapss, decry_param.rsakey);
    if(ret!=RET_SUCCESS)
    {
    	DECRYPT_PRINTK("RSA_verify fail\n");
		goto  exit;
    }
	ret=on_cmd_aes_cbc_decrypt(ktmp_addr,ddr_addr,decry_param.keyslot_type,decry_param.keyslot_num);
    if(ret!=RET_SUCCESS)
    {
    	DECRYPT_PRINTK("AES_cbc_decrypt fail\n");
		goto  exit;
    }
    
 exit:	 
 	kfree(ktmp_addr);
 	return ret;
	
}



u32 mt_avcpu_decrypt(u32 ddr_addr)
{
    struct mtd_info *mtd =NULL;
	
    DECRYPT_PRINTK("mt_avcpu_decrypt enter,addr:0x%x\n",ddr_addr);
    mtd = get_mtd_device_nm("av_cpu.img");
    if(IS_ERR(mtd))
	  return 0;
	
    //DECRYPT_PRINTK(" rsapss:%s,rsakey:%s,type:%s,num:%d\n",g_avcpu_decrypt_param.rsapss,g_avcpu_decrypt_param.rsakey,g_avcpu_decrypt_param.keyslot_type,g_avcpu_decrypt_param.keyslot_num);
	
	mt_avcpu_do_decrypt(ddr_addr,g_avcpu_decrypt_param,mtd);
	DECRYPT_PRINTK("avcpu_decrypt end\n");

	return 1;
}

