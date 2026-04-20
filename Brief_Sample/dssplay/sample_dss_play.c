/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_unf_hdmi.h"
#include "mt_unf_frontend.h"

//#include "mt_unf_advca.h"
#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"

#include "common/mt_adp.h"
#include "common/mt_adp_audio.h"
#include "common/mt_adp_hdmi.h"
#include "common/mt_adp_boardcfg.h"
#include "common/mt_adp_mpi.h"
#include "common/mt_adp_frontend.h"

#include "mt_unf_misc.h"
#include "mt_unf_cipher_v2.h"
//#include "pka/pka.h"
//#include "mss_cmd_utils.h"


#define DMX_ID              0

#define DUMP_DATA
#ifdef DUMP_DATA
#define DUMP_PRINTF printf
#else
#define DUMP_PRINTF(fmt, args...)   do{ } while (0)
#endif

//#define DSS_DEBUG
#ifdef DSS_DEBUG
#define DSS_DEBUG_PRINT printf
#else
#define DSS_DEBUG_PRINT(fmt, args...)   do{ } while (0)
#endif

#define DSS_INFO_PRINT printf
#define DSS_ERR_PRINT printf

#define SUCCESS 0
#define ERR_FAILURE -1
typedef enum
{
	DSS_NULL_PACKET,
	DSS_RANGING_PACKET,
	DSS_AUXILIARY_DATA_PACKET,
	DSS_VIDEO_SERVICE_PACKET,
	DSS_AUDIO_SERVICE_PACKET,
	DSS_PROGRAM_GUIDE_PACKET,
	DSS_CONDITIONAL_ACCESS_PACKET,
	DSS_DATA_APPLICATION_PACKET
}dss_packet_type_t;

typedef enum
{
	DSS_CF_SCRAMBLED = 0,
	DSS_CF_NOT_SCRAMBLED = 1
}dss_prefix_control_flag_t;

typedef enum
{
	DSS_ADG_RTS = 0x00,
	DSS_ADG_CWP = 0x01,
	DSS_ADG_RTS_CWP = 0x03,
	DSS_ADG_GOP_MAP_DATA = 0x04,
	DSS_ADG_SVP = 0x0B,/*001000b to 001011b SVP control data*/
	DSS_ADG_BROADBAND_VIDEO_DATA = 0x0C,
	DSS_ADG_RESERVED = 0x0D
}dss_auxiliary_data_group_type_t;

static u8 null_ranging_packet_payload[128] = 
{
	4,9,180,6,149,240,167,88,169,6,
	78,175,172,129,134,185,162,181,137,118,
	8,149,57,198,147,97,2,83,64,38,
	41,20,48,124,121,26,179,128,88,113,
	223,82,75,112,18,242,249,172,112,199,
	/*214,50,93,159,218,180,223,65,141,123,*/ /*In spec payload[55]=180,but in fact is 189*/
	214,50,93,159,218,189,223,65,141,123,
	64,184,0,54,38,137,99,57,113,146,
	191,245,71,194,159,212,55,154,235,227,
	129,200,197,13,230,112,19,246,86,128,
	182,122,127,197,176,233,125,137,212,61,
	/*187,96,192,141,69,15,108,80,184,106,*/ /*In spec payload[107]=80,but in fact is 89*/
	187,96,192,141,69,15,108,89,184,106, 
	159,231,224,157,197,198,57,60,134,61,
	11,218,100,50,214,95,53,184
};
struct dss_prefix_data
{
	u8 pf;/*Packet Framing 1bit This bit toggles between 0 and 1 with each packet*/
	u8 bb;/*Bundle Boundary 1bit 1:a sequence header or a picture header*/
	u8 cf;/*Control Flag 0: scrambled 1: not scrambled*/
	u8 cs;/*Control sync*/
	u16 scid;/*Service Channel ID 0x000 NULL packet,0xFEF to 0xFFF reserved*/
	u8 cc;/*Continuity Counter*/
	u8 hd;/*Header Designator*/
	dss_packet_type_t packet_type;
};

struct dss_auxiliary_data_prefix
{
	u8 mf;/*Modifiable Flag 1bit this bit is always set to 1*/
	u8 cff;/*Current field flag 1bit 0: not a vaild ADG,1: a vaild ADG*/
	u8 afid;/*Aux Field ID*/
	u8 afs;/*Auxiliary Field Size*/
};

struct dss_decoder
{
	struct dss_prefix_data prefix_data;
	struct dss_auxiliary_data_prefix aux_prefix_data;
	u16 packet_type[8];
	u8 payload[127];
	u16 hd[16];
	u16 video_scid_filter;
	u16 audio_scid_filter;
	u8 need_copy_length;
	u8 last_cc;
	u8 data_source;//0:demod+dmx 1:file
	mt_handle handle_win;
    mt_handle handle_track;
	mt_handle handle_avplay;
	u32 had_copy_video_data_length;
	u32 had_copy_auido_data_length;
};

#define V_ES_BUFFER_SIZE			0x100000
#define AUDIO_ES_1_READ_LEN         0x10000
#define READ_DSS_DATA_LEN           130*5

static pthread_t g_EsThd1;
struct dss_decoder g_decoder = {0};
FILE  *g_p_datasource = MT_NULL;

static u8 *video_buff=NULL;//[V_ES_BUFFER_SIZE] = {0};
static u8 *audio_buff=NULL;//[AUDIO_ES_1_READ_LEN] = {0};
static MT_BOOL g_bAudPlay = MT_FALSE;
static MT_BOOL g_bVidPlay = MT_FALSE;
static MT_BOOL g_bStopEsThread = MT_FALSE;
static int     g_debug_enable = 0;
static pthread_mutex_t g_ESLock = PTHREAD_MUTEX_INITIALIZER;
static mt_handle dmxhandle;
static int  g_scid_filter_enable  = 0;

#define MUTEX_LOCK()			do{pthread_mutex_lock(&g_ESLock);}while(0)
#define MUTEX_UNLOCK()			do{pthread_mutex_unlock(&g_ESLock);}while(0)


#ifdef DATA_Crypto_Process
#define  DATA_Crypto_Process    1

typedef double mss_clock_t;
static u8 *dss_mss_input;
static u32 ts_packet_count = 0;

static mt_u8 aes_tdes_iv[16] = {
    0xd1,0xa0,0x85,0x1a,0x9c,0xf4,0x58,0x4a,  /* this 8 bytes is tdes-iv */
    0x15,0x40,0x0b,0xb9,0x8b,0xee,0x78,0x54
};

static mt_u8 aes_tdes_key[16] = {
    0x06, 0xf1, 0x7c, 0x62, 0x09, 0xb7, 0xb5, 0xd8, 
    0xee, 0xfc, 0x1f, 0xdd, 0x1d, 0xc3, 0x3e, 0x7c,
};


/* AES-TDES test vectors start */
mt_u8 aes_tdes_test_data[32] = {
    0x65, 0xd0, 0xef, 0x70, 0x84, 0xaa, 0x98, 0x28, 
    0xfa, 0x62, 0x87, 0x85, 0x75, 0x9c, 0xbd, 0x2e,
    0x65, 0xd0, 0xef, 0x70, 0x84, 0xaa, 0x98, 0x28, 
    0xfa, 0x62, 0x87, 0x85, 0x75, 0x9c, 0xbd, 0x2e,
};


static unsigned char *mss_malloc(unsigned int size, unsigned char is_user_space_buf)
{
    void *p_vir_addr;

    if (size == 0)
        return NULL;

    if (is_user_space_buf) {
        return (unsigned char *)malloc(size);
    } else {
        p_vir_addr = mt_unf_cipher_malloc(size);

        //printf("%s::Got viraddr=0x%x\n", __FUNCTION__, vir_addr);

        /* success,return the virtual address of the buffer */
        return (unsigned char *)p_vir_addr;
    }
}

static int mss_free(void *p_vir, unsigned char is_user_space_buf)
{
    mt_s32 ret = MT_SUCCESS;

    if (p_vir == NULL) {
        printf("Can not free NULL pointer\n");
        return -1;
    }

    if (is_user_space_buf) {
        free(p_vir);
    } else {
        //printf("%s::Got viraddr=0x%x\n", __FUNCTION__, (mt_u32)p_vir);
        mt_unf_cipher_free(p_vir);
    }

    return ret;

}

/* virutal address to physical address, no check */
static mt_u8 *vir2phy(mt_u8 *vir)
{
    mt_u32 phy_addr;
    mt_u32 phy_size;

    mt_mmz_get_phyaddr((mt_void *)vir, &phy_addr, &phy_size);

    return (mt_u8 *)phy_addr;
}


static int keyslot_set(unsigned int slot_id, MT_CIPHER_CTRL_S *p_info, unsigned char *key, unsigned char *iv)
{
#ifdef SCPU_EN
    return mt_unf_scpu_cipher_keyslot_set(slot_id, p_info, key, iv);
#else
    return mt_unf_cipher_keyslot_set(slot_id, p_info, key, iv);
#endif
}

static int keyslot_request(unsigned int *p_slot)
{
#ifdef SCPU_EN
    return mt_unf_scpu_cipher_keyslot_request(p_slot);
#else
    return mt_unf_cipher_keyslot_request(p_slot);
#endif
}

static int keyslot_release(unsigned int slot_id)
{
#ifdef SCPU_EN
    return mt_unf_scpu_cipher_keyslot_release(slot_id);
#else
    return mt_unf_cipher_keyslot_release(slot_id);
#endif
}

static int crypto_process(unsigned int slot_id, MT_CIPHER_CTRL_S *p_info, unsigned char *input, unsigned char *output, unsigned int input_size)
{
    int ret = 0;
    mt_handle p_cipher;

    ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_1, &p_cipher);
    //printf("==ret=%d\n", ret);
    ret |= mt_unf_cipher_crypto_config(p_cipher, p_info, slot_id);

    //printf("==ret=%d\n", ret);
#ifdef SCPU_EN
    cryptolock_take();
#endif
	ret |= mt_unf_cipher_crypto_process(p_cipher, input, output, input_size);
#ifdef SCPU_EN
    cryptolock_give();
#endif
	//printf("==ret=%d\n", ret);
    mt_unf_cipher_crypto_destroy(p_cipher);

    return ret;
}

static int cipher_aes_tdes_start(unsigned int algo, unsigned int op, unsigned int mode, mt_u8 *key, mt_u8 *iv, 
        mt_u8 *input, mt_u8 *output, mt_u32 input_size)
{
    int ret = 0;
    unsigned int slot_id;
    MT_CIPHER_CTRL_S info;
    mss_clock_t t;

    keyslot_request(&slot_id);
    //printf("==slot_id=%d\n", slot_id);

    memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));

    info.core = MT_CIPHER_CORE_M2M_RAW;
    info.work_mode = mode;

    info.algorithm = MT_CIPHER_ALG_AES;
    //info.algorithm = MT_CIPHER_ALG_TDES;
   
    //info.operation = MT_CIPHER_OPERATION_DECRYPT;
    info.operation = MT_CIPHER_OPERATION_ENCRYPT;
    
    t = clock();
    //print_time(__FUNCTION__, "start time:", t);

    // if (g_kl_test_cb.kl_cb) {
     //   if (g_kl_test_cb.kl_cb(g_kl_test_cb.arg, slot_id)) {
     //       goto aes_tdes_err;
     //   }
     //     keyslot_set(slot_id, &info, NULL, iv);
     // } else {
        /*clear-text key */
        keyslot_set(slot_id, &info, key, iv);
    //}

    ret = crypto_process(slot_id, &info, input, output, input_size);
    if (ret != MT_SUCCESS) {
        printf("%s failed\n", __FUNCTION__);
        goto aes_tdes_err;
    }

aes_tdes_err:
    keyslot_release(slot_id);

    t = clock() - t;
    //print_time(__FUNCTION__, "time taken:", t);

    return ret;
}

static int cipher_aes_tdes_test(unsigned int algo, unsigned int op, unsigned int mode, char *key_iv_file, char *data_file)
{
    int ret = 0;
    mt_u8 *output = NULL;
    mt_u8 *input = NULL;
    mt_u8 *expect_data = NULL;
    mt_u32 input_size = 32;
    mt_u8 is_user_data = 0;
    mt_u8 key[16] = {0};
    mt_u8 iv[16] = {0};
    mss_clock_t time_elapsed;


    if (key_iv_file == NULL || data_file == NULL) {
        printf("Use internal demo.\n");
        input = mss_malloc(input_size, 0);
        if (input == NULL)
            return -1;

        output = input;

        memset(input, 0, input_size);
        memcpy(input, aes_tdes_test_data, input_size);

        memcpy(key, aes_tdes_key, 16);
        memcpy(iv, aes_tdes_iv, 16);
    } else {
        printf("Use user data.\n");
        input = read_file(data_file, &input_size, 0);
        if (input == NULL) {
            printf("read user data failed!\n");
            return -1;
        }
        output = input; //in-place operation

        //dump 64 bytes of input data
        dump("Input", input, 64);

        //TODO: alignment requirement ??
        if ((input_size & 0x07) != 0) {
            printf("Input data must be at least 8-byte aligned!\n");
            mss_free(input, 0);
            return -1;
        }

        ret = read_key_iv(key_iv_file, key, iv);
        if (ret)
            return -1;

        dump("Key", key, 16);
        dump("IV", iv, 16);

        is_user_data = 1;
    } 

    printf("%s-%s-%s start...\n", to_algo_str(algo), to_op_str(op), to_work_mode_str(mode));
    ret = cipher_aes_tdes_start(0, 0, 0, key, iv, input, output, input_size);
    if (ret)
        goto __aes_tdes_start_failed;


    if (is_user_data) {
        printf("It's user data, please check the output");
        dump("Output", output, (input_size > 32 ? 32 : input_size));
    } else {
        expect_data = get_ades_internal_expect_data(algo, op, mode);
        check_result("Compare", expect_data, output, input_size, time_elapsed);
    }

__aes_tdes_start_failed:
    if (((mt_u32)output != (mt_u32)input) && output != NULL)
        mss_free(output, 0);
    output = NULL;

    if (input)
        mss_free(input, 0);
    input = NULL;

    return ret;
}


#endif   //DATA_Crypto_Process


static void dss_dump_data(char *p_name,const unsigned char *p_data, int length)
{
	unsigned int i;
	DUMP_PRINTF("\n");
	DUMP_PRINTF("begin dump [%s] data, length = [%d]: \n", p_name, length);
	for (i = 0; i < length; i++)
	{	
		DUMP_PRINTF("%02x ", p_data[i]);
		if((i+1)%16 == 0)
			DUMP_PRINTF("\n");
	}
	DUMP_PRINTF("\n\n");
}
static void dss_dump_data_u16 (char *p_name,const u16 *p_data, int length)
{
	unsigned int i;
	DUMP_PRINTF("\n");
	DUMP_PRINTF("begin dump [%s] data, length = [%d]: \n", p_name, length);
	for (i = 0; i < length; i++)
	{	
		DUMP_PRINTF("%04x ", p_data[i]);
		if((i+1)%16 == 0)
			DUMP_PRINTF("\n");
	}
	DUMP_PRINTF("\n\n");
}

static int is_null_or_ranging_packet(unsigned char *p_in)
{
	int i = 0;
	for(i = 0;i<128;i++)
	{
		if(null_ranging_packet_payload[i] == p_in[i])
		{
			continue;
		}
		else
		{
			DSS_DEBUG_PRINT("i:%d,%d != %d \n",i,null_ranging_packet_payload[i],p_in[i]);
			return 0;
		}
	}
	return 1;
}
static void fill_array(u16 *p_in,u16 value,u16 *p_length,char *p_name)
{
	u16 length = 0;
	u16 i = 0;
	u8 new_data_flag = 0;

	length = *p_length;

	if(length == 0)
	{
		new_data_flag = 1;
	}
	else
	{
		for(i = 0;i < length;i++)
		{
			if(p_in[i] == 0xFFFF)
			{
				DSS_ERR_PRINT("i = %d length = %d no data in array?\n",i,length);
				break;
			}
			if(p_in[i] == value)
			{
				new_data_flag = 0;
				break;
			}
			else if(p_in[i] != value)
			{
				new_data_flag = 1;
			}
		}
	}
	if(new_data_flag == 1)
	{
		p_in[length] = value;
		length++;
		dss_dump_data_u16(p_name,p_in,length);
	}
	*p_length = length;
}
static int find_value_in_array_u16(u16 *p_in,u16 array_size,u16 value)
{
	u16 i = 0;
	int ret = 0;//0: no match value 1:find match value
	for(i = 0;i<array_size;i++)
	{
		if(p_in[i] == value)
		{
			ret = 1;
			break;
		}
	}
	return ret;
}
static int find_the_last_pes_header_start(const unsigned char *p_in)
{
	u8 i = 0;
	u8 data_start_index = 0;
	do{
		if(i > 127)
			break;
		if((p_in[i]== 0x00)&&(p_in[i+1]== 0x00)&&(p_in[i+2]== 0x01))
		{
			i+=3;
			data_start_index = i;
		}
		else
		{
			i++;
		}
	}while(1);
	return data_start_index;
}

static int find_start_code_index(const unsigned char *p_in)
{
	u8 i = 0;
	u8 data_start_index = 0;
	u8 stream_id = 0;
	do{
		if(i > 127)
			break;
		if((p_in[i]== 0x00)&&(p_in[i+1]== 0x00)&&(p_in[i+2]== 0x01))
		{
			stream_id = p_in[i+3];
			if(stream_id < 0xBC) //This maybe video or audio data start code
			{
				data_start_index = i;
				break;
			}
			else
			{
				i+=3;
				data_start_index = i;
			}
		}
		else
		{
			i++;
		}
	}while(1);
	return data_start_index;
}
static void parse_dss_prefix(const unsigned char *p_in,struct dss_prefix_data *p_prefix_data)
{
	memset(p_prefix_data, 0, sizeof(struct dss_prefix_data));

	p_prefix_data->pf = (p_in[0]&0x80)>>7;
	p_prefix_data->bb = (p_in[0]&0x40)>>6;
	p_prefix_data->cf = (p_in[0]&0x20)>>5;
	p_prefix_data->cs = (p_in[0]&0x10)>>4;
	p_prefix_data->scid = ((p_in[0]&0x0F)<<8)|p_in[1];
	p_prefix_data->cc = ((p_in[2]&0xF0)>>4);
	p_prefix_data->hd = p_in[2]&0x0F;

	//dss_dump_data("dss_prefix",p_in,3);
	DSS_DEBUG_PRINT("prefix_data:\n");
	DSS_DEBUG_PRINT("pf:%d bb:%d(1:video data) cf:%d(0:scrambled)cs:%d scid:0x%x,cc:%d,hd:0x%x\n",p_prefix_data->pf,
					p_prefix_data->bb,p_prefix_data->cf,p_prefix_data->cs,p_prefix_data->scid,p_prefix_data->cc,p_prefix_data->hd);
}
static int parse_dss_auxiliary_packet(const unsigned char *p_in,struct dss_decoder *p_decoder)
{
	struct dss_auxiliary_data_prefix *p_aux_prefix_data = NULL;
	u8 tmp = 0;

	p_aux_prefix_data = (struct dss_auxiliary_data_prefix *)&p_decoder->aux_prefix_data;
	if(p_aux_prefix_data == NULL)
	{
		DSS_ERR_PRINT("[%s %d]p_aux_prefix_data is NULL,Please check \n",__FUNCTION__,__LINE__);
		return ERR_FAILURE;
	}

	p_aux_prefix_data->mf = (p_in[0]&0x80)>>7;
	
	p_aux_prefix_data->cff = (p_in[0]&0x40)>>6;
	if(p_aux_prefix_data->cff == 0)
	{
		DSS_ERR_PRINT("[%s %d]Not a vaild ADG cff is %d,Please check \n",__FUNCTION__,__LINE__,p_aux_prefix_data->cff);
		return ERR_FAILURE;
	}

	p_aux_prefix_data->cff = p_in[1];
	tmp = p_in[0]&0x3F;
	switch(tmp){
		case 0x00:
			p_aux_prefix_data->afid = DSS_ADG_RTS;
			break;
		case 0x01:
			p_aux_prefix_data->afid = DSS_ADG_CWP;
			break;
		case 0x02:
			p_aux_prefix_data->afid = DSS_ADG_RESERVED;
			break;
		case 0x03:
			p_aux_prefix_data->afid = DSS_ADG_RTS_CWP;
			break;
		case 0x04:
			p_aux_prefix_data->afid = DSS_ADG_GOP_MAP_DATA;
			break;
		case 0x05:
		case 0x06:
		case 0x07:
			p_aux_prefix_data->afid = DSS_ADG_RESERVED;
			break;
		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
			p_aux_prefix_data->afid = DSS_ADG_SVP;
			break;
		case 0x0C:
			p_aux_prefix_data->afid = DSS_ADG_BROADBAND_VIDEO_DATA;
			break;
		default:
			p_aux_prefix_data->afid = DSS_ADG_RESERVED;
			break;
	}
	
}
static int extract_vaild_data(const unsigned char *p_in,struct dss_decoder *p_decoder)
{
	struct dss_prefix_data *p_prefix_data = NULL;
	u8 start_copy_index = 0;
	memset(p_decoder->payload, 0, sizeof(u8)*127);
	p_decoder->need_copy_length = 0;
	
	p_prefix_data = (struct dss_prefix_data *)&p_decoder->prefix_data;
	if(p_prefix_data == NULL)
	{
		DSS_ERR_PRINT("p_prefix_data is NULL,PLS check \n");
		return ERR_FAILURE;
	}

	if(p_prefix_data->hd == 0x00)
	{
		DSS_DEBUG_PRINT("This packet is auxiliary packet!!\n");
		p_prefix_data->packet_type = DSS_AUXILIARY_DATA_PACKET;
		//Todo AUX parse
		parse_dss_auxiliary_packet(p_in,p_decoder);
	}
	else
	{
	    /*if (p_prefix_data->cc !=(p_decoder->last_cc+1))
		{
			if((p_decoder->last_cc == 0x0F)&&(p_prefix_data->cc == 0))
			{
				
			}
			else
			{
				DSS_ERR_PRINT("[%s %d]Data not Continuity last_cc:%d,cc:%d !!\n",__FUNCTION__,__LINE__,p_decoder->last_cc,p_prefix_data->cc);
			}
		}
		p_decoder->last_cc = p_prefix_data->cc;*/
		
		if(p_prefix_data->bb == 0)
		{//This maybe video data not containing PSE start code or audio data
			p_decoder->need_copy_length = 127;
			memcpy(p_decoder->payload, p_in, p_decoder->need_copy_length);
#ifdef DATA_Crypto_Process
			memcpy(dss_mss_input, &p_in[0], 128*4);
			ts_packet_count ++;
			if(ts_packet_count >= 1024)
			{
				ts_packet_count = 0;
				printf("cipher_aes_tdes_start ... 1\n");
				mt_mmz_flush(dss_mss_input, 0, 0);
				cipher_aes_tdes_start(0, 1, 1, aes_tdes_key, aes_tdes_iv, dss_mss_input, dss_mss_input, 512*1024);
			}
#endif //DATA_Crypto_Process			
			if(p_prefix_data->hd == 0x04)
			{
				if(find_value_in_array_u16(p_decoder->hd, 16, 0x06)== 0)
					p_prefix_data->packet_type = DSS_AUDIO_SERVICE_PACKET;
				else
					p_prefix_data->packet_type = DSS_VIDEO_SERVICE_PACKET;
			}
		}
		else
		{//BB = 1 PES header include
			start_copy_index = find_start_code_index(p_in);
			if(start_copy_index > 127)
				p_decoder->need_copy_length = 0;
			else
				p_decoder->need_copy_length = 127 - start_copy_index;
			
			DSS_DEBUG_PRINT("This packet is PES Header packet,start_copy_index is 0x%x,need_copy_length is %d!!\n",start_copy_index,p_decoder->need_copy_length);

			if(p_decoder->need_copy_length > 0){
				memcpy(p_decoder->payload, &p_in[start_copy_index], p_decoder->need_copy_length);
#ifdef DATA_Crypto_Process				
				memcpy(dss_mss_input, &p_in[start_copy_index], 128*4);
				ts_packet_count ++;
				if(ts_packet_count >= 1024){
					ts_packet_count = 0;
					printf("cipher_aes_tdes_start ... 2\n");
					mt_mmz_flush(dss_mss_input, 0, 0);
					cipher_aes_tdes_start(0, 1, 1, aes_tdes_key, aes_tdes_iv, dss_mss_input, dss_mss_input, 512*1024);
					
				}
#endif //DATA_Crypto_Process		
			}
			p_prefix_data->packet_type = DSS_VIDEO_SERVICE_PACKET;
		}
	}
}
static mt_void tuner_init(mt_s32 argc, mt_char *argv[])
{
	mt_unf_fe_status_t fe_status;
	int count = 0;
	mt_u32 freq = 0;
	mt_u32 sym = 0;

    freq  = strtol(argv[1],NULL,0);
    sym = strtol(argv[2],NULL,0);
    mtadp_fe_init();
    mtadp_fe_connect_dvbs(0,freq, sym,0, 0, 4);

	do{
		mt_unf_fe_get_status(0, &fe_status);
		if(count > 500)
		{	
			printf("[%s :  %s] Lock Timeout\n",__FILE__, __FUNCTION__);
			break;
		}
		count++;
		MT_USLEEP(10 * 1000);
	}while(fe_status.lock_status != MT_UNF_FE_SIGNAL_LOCKED);
	printf("[%s] Lock status %d\n", __FUNCTION__,fe_status.lock_status);
	
}

//static mt_s32 avplay_event(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent, long u32Para)
static long avplay_event(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent, long u32Para)
{
	static unsigned long n = 0;	//video frame counter
	MT_UNF_VIDEO_FRAME_INFO_S *pVdecFrm;
	MT_UNF_AO_FRAMEINFO_S *pAudFrm;

	//printf("\n%s: handle=%x, event=%d, param=%x\n",__FUNCTION__,
	//		hAvplay,enEvent,u32Para);

	if (hAvplay == 0)
	{
		printf("[ERROR] %s: invalid avplay handle\n",__FUNCTION__);
		return MT_SUCCESS;
	}

	switch (enEvent)
	{
		case MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME:

			pVdecFrm = (MT_UNF_VIDEO_FRAME_INFO_S*)u32Para;

			if (pVdecFrm != NULL
				/*&& (pVdecFrm->u32FrameIndex % 100) == 0*/
				&& (n++ % 100) == 0)
			{
				DSS_DEBUG_PRINT("============================================================\n");
				DSS_DEBUG_PRINT("%s: hAvplay %x - EVENT_NEW_VID_FRAME\n",__FUNCTION__,hAvplay);

				DSS_DEBUG_PRINT("           u32FrameIndex: %u\n",pVdecFrm->u32FrameIndex);
				DSS_DEBUG_PRINT("                u32Width: %u\n",pVdecFrm->u32Width);
				DSS_DEBUG_PRINT("               u32Height: %u\n",pVdecFrm->u32Height);
				DSS_DEBUG_PRINT("         u32DisplayWidth: %u\n",pVdecFrm->u32DisplayWidth);
				DSS_DEBUG_PRINT("        u32DisplayHeight: %u\n",pVdecFrm->u32DisplayHeight);
				DSS_DEBUG_PRINT("       u32DisplayCenterX: %u\n",pVdecFrm->u32DisplayCenterX);
				DSS_DEBUG_PRINT("       u32DisplayCenterY: %u\n",pVdecFrm->u32DisplayCenterY);
				DSS_DEBUG_PRINT("                  u64Pts: %llu\n",pVdecFrm->u64Pts);
				DSS_DEBUG_PRINT("          u32AspectWidth: %u\n",pVdecFrm->u32AspectWidth);
				DSS_DEBUG_PRINT("         u32AspectHeight: %u\n",pVdecFrm->u32AspectHeight);
				DSS_DEBUG_PRINT("             stFrameRate: %d.%03d\n",pVdecFrm->stFrameRate.u32fpsInteger,
															pVdecFrm->stFrameRate.u32fpsDecimal);
				DSS_DEBUG_PRINT("           enVideoFormat: %d\n",pVdecFrm->enVideoFormat);
				DSS_DEBUG_PRINT("            bProgressive: %d\n",pVdecFrm->bProgressive);
				DSS_DEBUG_PRINT("             enFieldMode: %d\n",pVdecFrm->enFieldMode);
				DSS_DEBUG_PRINT("          bTopFieldFirst: %d\n",pVdecFrm->bTopFieldFirst);
				/* not used */
				//printf("      field_storage_mode: %u\n",pVdecFrm->filed_storage_mode);
				//printf("              is_3D_flag: %u\n",pVdecFrm->is_3D_flag);
				DSS_DEBUG_PRINT("      enFramePackingType: %d\n",pVdecFrm->enFramePackingType);
				/* not used */
				//printf("            packing_type: %d\n",pVdecFrm->packing_type);
				DSS_DEBUG_PRINT("           u32ErrorLevel: %u\n",pVdecFrm->u32ErrorLevel);
				/* not used */
				//printf("display_order_mode_valid: %u\n",pVdecFrm->display_order_mode_valid);
				//printf("      display_order_mode: %d\n",pVdecFrm->display_order_mode);
				DSS_DEBUG_PRINT("     picture_coding_type: %u\n",pVdecFrm->picture_coding_type);
				DSS_DEBUG_PRINT("------------------------------------------------------------\n");
			}
			break;

		case MT_UNF_AVPLAY_EVENT_NEW_AUD_FRAME:

			pAudFrm = (MT_UNF_AO_FRAMEINFO_S*)u32Para;

			if (pAudFrm != NULL
				&& (pAudFrm->u32FrameCounter % 100) == 1)
			{
				DSS_DEBUG_PRINT("============================================================\n");
				DSS_DEBUG_PRINT("%s: hAvplay %x - EVENT_NEW_AUD_FRAME\n",__FUNCTION__,hAvplay);

				DSS_DEBUG_PRINT("        u32FrameIndex: %u\n",pAudFrm->u32FrameIndex);
				DSS_DEBUG_PRINT("      u32FrameCounter: %u\n",pAudFrm->u32FrameCounter);
				DSS_DEBUG_PRINT("             u64PtsMs: %llu\n",pAudFrm->u64PtsMs);
				DSS_DEBUG_PRINT("      s32BitPerSample: %d\n",pAudFrm->s32BitPerSample);
				DSS_DEBUG_PRINT("         bInterleaved: %d\n",pAudFrm->bInterleaved);
				DSS_DEBUG_PRINT("        u32SampleRate: %u\n",pAudFrm->u32SampleRate);
				DSS_DEBUG_PRINT("          u32Channels: %u\n",pAudFrm->u32Channels);
				DSS_DEBUG_PRINT("     u32ChannelsExist: 0x%x\n",pAudFrm->u32ChannelsExist);
				DSS_DEBUG_PRINT("u32PcmSamplesPerFrame: %u\n",pAudFrm->u32PcmSamplesPerFrame);
				DSS_DEBUG_PRINT(" u32BitsBytesPerFrame: %u\n",pAudFrm->u32BitsBytesPerFrame);
				DSS_DEBUG_PRINT("                b_eos: %d\n",pAudFrm->b_eos);
				DSS_DEBUG_PRINT("  bEac4TimeSampleRate: %d\n",pAudFrm->bEac4TimeSampleRate);
				DSS_DEBUG_PRINT("------------------------------------------------------------\n");
			}
			break;

		case MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT:
			DSS_DEBUG_PRINT("\n%s: hAvplay %x - EVENT_VID_UNSUPPORT\n",__FUNCTION__,hAvplay);
			break;

		case MT_UNF_AVPLAY_EVENT_EOS:
			DSS_DEBUG_PRINT("\non_avplay_event: EOS Reached!!!\n");
			break;

		default:
			DSS_DEBUG_PRINT("\n[WARNING]%s: hAvplay %x - Event(%d) not support!\n",__FUNCTION__,
						hAvplay,enEvent);
			break;
	}

	return MT_SUCCESS;
}
static mt_void PushEsDataTthread(mt_void *args)
{
    mt_handle hAvplay;
    MT_UNF_STREAM_BUF_S StreamBuf;
    mt_s32 Readlen;	//read() may return < 0
    mt_u32 Readlen8;
    mt_s32 Ret;
    //MT_BOOL bVidEOF = MT_FALSE;
    //MT_BOOL bAudEOF = MT_FALSE;
	MT_BOOL b_start_video_play = MT_FALSE;
	MT_BOOL b_start_audio_play = MT_FALSE;
	struct dss_prefix_data *p_prefix_data = NULL;
	mt_u64 aud_pts_fake = 0;
	unsigned char *dss_one_pkt=NULL;//[1300] = {0};
	unsigned char *dss_pkt = NULL;
	mt_u32 data_len = 0;
	mt_u32 video_buff_copy_index = 0;
	mt_u32 packet_count = 0; 
	int index = 0;
	MT_UNF_ES_BUF_S esinfo={0};
    FILE *EsFile=NULL;
    mt_u32 faulty_data_len = 0;
	
	memset(g_decoder.hd, 0xFFFF, sizeof(u16)*16);
	memset(g_decoder.packet_type, 0xFFFF, sizeof(u16)*8);

	dss_one_pkt = (u8 *)malloc(READ_DSS_DATA_LEN);
	if(dss_one_pkt){
		memset(dss_one_pkt, 0, READ_DSS_DATA_LEN);
	}else{
		return ERR_FAILURE;
	}


	video_buff=(u8 *)malloc(V_ES_BUFFER_SIZE);
	if(video_buff){
		memset(video_buff, 0, V_ES_BUFFER_SIZE);
	}else{
		return ERR_FAILURE;
	}

	audio_buff=(u8 *)malloc(AUDIO_ES_1_READ_LEN);
	if(audio_buff){
		memset(audio_buff, 0, AUDIO_ES_1_READ_LEN);
	}else{
		return ERR_FAILURE;
	}

    hAvplay = g_decoder.handle_avplay;
	
    p_prefix_data = &g_decoder.prefix_data;
	if(p_prefix_data == NULL)
	{
		DSS_ERR_PRINT("p_prefix_data is NULL,PLS check \n");
		return ERR_FAILURE;
	}
	
    if((g_debug_enable == 2) && (g_decoder.data_source == 0))  EsFile = fopen("dss_es.ts", "wb");
	
    while (!g_bStopEsThread)
    {
    	packet_count = 0;
		
		if(g_decoder.data_source == 0)
		{
			memset(&esinfo, 0, sizeof(MT_UNF_ES_BUF_S));	
			//if(MT_SUCCESS!=MT_UNF_DMX_CheckDataHandle(dmxhandle, 1000)){
	         ////   printf("call MT_UNF_DMX_SelectDataHandle failed!\n");
	          //  MT_USLEEP(10 * 1000);
	           // continue;
			//}
			Ret=MT_UNF_DMX_AcquireEs(dmxhandle,&esinfo);
           		 if(MT_SUCCESS==Ret){
                	//dumpdata(esinfo.pu8Buf,esinfo.u32BufLen);
				data_len = esinfo.u32BufLen;
				if(g_debug_enable == 1) {
					printf("esinfo.u32BufLen = %d, packet_count=%d, esinfo.pu8Buf=0x%x\n",
						esinfo.u32BufLen, esinfo.u32BufLen / 130, esinfo.pu8Buf);
				}
				if(esinfo.u32BufLen > 0)
				{
					packet_count = esinfo.u32BufLen / 130;
					dss_pkt = esinfo.pu8Buf;
					if(g_debug_enable == 2)	
					{
						Ret = (mt_s32)fwrite(esinfo.pu8Buf, 1, esinfo.u32BufLen, EsFile);
						if (Ret != (mt_s32)esinfo.u32BufLen) {
							printf("ret=%x\n", Ret);
							printf("[SaveRecDataThread] fwrite error");
							break;
						}
					}
					if(faulty_data_len > 0)
					{
						memcpy(&dss_one_pkt[faulty_data_len], dss_pkt, (130 - faulty_data_len));
						dss_pkt = &dss_one_pkt[0];
						packet_count = 1;
						esinfo.u32BufLen = 130 - faulty_data_len;
						if(g_debug_enable == 1) {
							printf("esinfo.u32BufLen = %d\n", esinfo.u32BufLen);
						}
						MT_UNF_DMX_ReleaseEs(dmxhandle,&esinfo);
						faulty_data_len = 0;	
						memset(&esinfo, 0, sizeof(MT_UNF_ES_BUF_S));
					}
					
					if((packet_count == 0) && (esinfo.u32BufLen > 0))    // data len < 130
					{
						memset(dss_one_pkt, 0, READ_DSS_DATA_LEN);
						memcpy(dss_one_pkt, dss_pkt, esinfo.u32BufLen);
						faulty_data_len = esinfo.u32BufLen;
						MT_USLEEP(10 * 1000);
						MT_UNF_DMX_ReleaseEs(dmxhandle,&esinfo);
						memset(&esinfo, 0, sizeof(MT_UNF_ES_BUF_S));
	            		continue;
					}
				}	
                
	             }
		      else
		      {
	                   printf("AcquireEs err Ret=0x%x\n",Ret);
			    if(Ret == MT_ERR_DMX_NOAVAILABLE_DATA)
					{
						MT_USLEEP(10 * 1000);
		            			continue;
					}
	             }
		}
		else if(g_decoder.data_source == 1)
		{
			memset(dss_one_pkt, 0, READ_DSS_DATA_LEN);
			data_len = fread(dss_one_pkt,1,READ_DSS_DATA_LEN,g_p_datasource);
			packet_count = data_len / 130;
			if(data_len < READ_DSS_DATA_LEN)
			{
				DSS_INFO_PRINT("read  file end and rewind!\n");
                		rewind(g_p_datasource);
				memset(g_decoder.hd, 0xFFFF, sizeof(u16)*16);
				memset(g_decoder.packet_type, 0xFFFF, sizeof(u16)*8);
				continue;
			}
			dss_pkt = &dss_one_pkt[0];
		}
		if(g_debug_enable == 1) printf("packet_count = %d, read data len =%d\n",packet_count,data_len);
		index = 0;
		
		while((packet_count > index)  &&  (g_debug_enable != 2))
		{
			index ++;
			//if(g_debug_enable == 1) printf("index = %d, dss_pkt=0x%x\n",index,dss_pkt);
			if(data_len >= 130)
			{
				parse_dss_prefix(&dss_pkt[0],p_prefix_data);
				if(p_prefix_data->scid == 0x00)
				{
					p_prefix_data->packet_type = DSS_NULL_PACKET;
					if(is_null_or_ranging_packet(&dss_pkt[2]))
					{
						DSS_DEBUG_PRINT("It is Null packet.\n");
						//fwrite(&dss_pkt[0], 1, 130, p_video_file);
						dss_pkt += 130; 
						continue;
					}
					else
					{
						DSS_ERR_PRINT("ERR: scid is 0,but payload no match,pls check!!\n");
						dss_dump_data("payload",&dss_pkt[2],128);
					}

				}
				else
				{
					if(p_prefix_data->cf == 1)
					{
						if(g_scid_filter_enable == 1)
						{
							//printf("scid:0x%x=0x%x\n",p_prefix_data->scid, g_decoder.video_scid_filter);
						}
						if((p_prefix_data->scid == g_decoder.audio_scid_filter)||(p_prefix_data->scid == g_decoder.video_scid_filter))
						{
							extract_vaild_data(&dss_pkt[3], &g_decoder);
							if(g_decoder.need_copy_length > 0)
							{
								if(p_prefix_data->scid == g_decoder.audio_scid_filter)
								{
									if(g_debug_enable == 1)  printf("1.had_copy_auido_data_length is %d %d\n",g_decoder.had_copy_auido_data_length,g_decoder.need_copy_length);
									memcpy(&audio_buff[g_decoder.had_copy_auido_data_length], &g_decoder.payload, g_decoder.need_copy_length);
									g_decoder.had_copy_auido_data_length += g_decoder.need_copy_length;
									//DSS_ERR_PRINT("2.had_copy_auido_data_length is %d\n",g_decoder.had_copy_auido_data_length);
									if((AUDIO_ES_1_READ_LEN - g_decoder.had_copy_auido_data_length) < 127)
									{
										b_start_audio_play = MT_TRUE;
										//DSS_ERR_PRINT("[%s %d]had_copy_auido_data_length is %d\n",__FUNCTION__,__LINE__,g_decoder.had_copy_auido_data_length);
									}
								}
								else if(p_prefix_data->scid == g_decoder.video_scid_filter)
								{
									if(g_debug_enable == 1)  printf("1.had_copy_video_data_length is %d, %d\n",g_decoder.had_copy_video_data_length,g_decoder.need_copy_length);
									memcpy(&video_buff[g_decoder.had_copy_video_data_length], &g_decoder.payload, g_decoder.need_copy_length);
									g_decoder.had_copy_video_data_length += g_decoder.need_copy_length;
									//DSS_ERR_PRINT("2.had_copy_video_data_length is %d\n",g_decoder.had_copy_video_data_length);
									if((V_ES_BUFFER_SIZE - g_decoder.had_copy_video_data_length) < 127)
									{
										//DSS_ERR_PRINT("[%s %d] had_copy_video_data_length is %d \n",__FUNCTION__,__LINE__,g_decoder.had_copy_video_data_length);
										b_start_video_play = MT_TRUE;
									}
								}
							}
							
						}
					}
					else
					{
						//The packet is scrambled, we can do nothing about it.
						//Or the scid is no the data we needed
						dss_pkt += 130; 
						continue;
					}
				}
				
			}
			dss_pkt += 130; 
		}
		
		if((g_decoder.data_source == 0) && (esinfo.u32BufLen > 0))
		{
			esinfo.u32BufLen = packet_count * 130;
			MT_UNF_DMX_ReleaseEs(dmxhandle,&esinfo);
			memset(&esinfo, 0, sizeof(MT_UNF_ES_BUF_S));
		}

		if((g_debug_enable == 2) && (g_decoder.data_source == 0))
		{
			continue;  //for write file
		}
        if (b_start_video_play & g_bVidPlay)
        {
			video_buff_copy_index = 0;
			do{
	        	MUTEX_LOCK();
	            Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, V_ES_BUFFER_SIZE, &StreamBuf, 0);
				//Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, g_decoder.had_copy_video_data_length, &StreamBuf, 0);
	            if (MT_SUCCESS == Ret
	            	&& StreamBuf.u32Size > 0
	            	&& StreamBuf.pu8Data != NULL)
	            {
					if(g_debug_enable == 1){
	                	printf("1.EsTthread: GetVidBuf %p, 0x%x video_data_length:0x%x \n",StreamBuf.pu8Data,StreamBuf.u32Size,g_decoder.had_copy_video_data_length);
					}
		        	memset(StreamBuf.pu8Data, 0, StreamBuf.u32Size);
					if(g_decoder.had_copy_video_data_length > StreamBuf.u32Size)
					{
						g_decoder.had_copy_video_data_length -= StreamBuf.u32Size;
						Readlen = StreamBuf.u32Size;
					}
					else
					{
						Readlen = g_decoder.had_copy_video_data_length;
						g_decoder.had_copy_video_data_length = 0;
					}
					memcpy(StreamBuf.pu8Data,&video_buff[video_buff_copy_index], Readlen);
					video_buff_copy_index += Readlen;
					//memset(video_buff, 0, V_ES_BUFFER_SIZE);
					
	                //Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, g_pVidEsFile);
					if(g_debug_enable == 1)  printf("putBuf >>> Readlen %d\n",Readlen);
	                if (Readlen > 0)
	                {
						//FIXME: fread segment fault!
						//buffer must aligned with 16k and stuff with 0xff?
						//must put 16k buffer each time! NOT Readlen!
	                    //Ret = MT_UNF_AVPLAY_PutBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, Readlen, 0);

						//it's better to align 8 byte for hw vdec
						//推荐8字节对齐
						//如果不是完整的一帧数据,添加填充数据反而会出马赛克
						//Readlen8 = StreamBuf.u32Size;
						Readlen8 = Readlen;
						//stuff zero
						if (Readlen < Readlen8)
						{
							memset(StreamBuf.pu8Data+Readlen, 0, Readlen8-Readlen);
						}

	                   	Ret = MT_UNF_AVPLAY_PutBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, Readlen8, 0);
	                    if (Ret != MT_SUCCESS)
	                    {
	                        DSS_ERR_PRINT("call MT_UNF_AVPLAY_PutBuf failed.\n");
	                    }
	                }
	            }
	            else
	            {
					//printf("2.RET %d EsTthread: GetVidBuf %p, 0x%x video_data_length:0x%x \n",Ret,StreamBuf.pu8Data,StreamBuf.u32Size,g_decoder.had_copy_video_data_length);
					//MT_USLEEP(30000);
	            }
				MT_USLEEP(30000);
	            MUTEX_UNLOCK();
			}while(g_decoder.had_copy_video_data_length != 0);
			b_start_video_play = MT_FALSE;
			memset(video_buff, 0, V_ES_BUFFER_SIZE);
        }

        if (b_start_audio_play & g_bAudPlay)
        {
			do{
        	MUTEX_LOCK();

            Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, AUDIO_ES_1_READ_LEN, &StreamBuf, 0);
            if (MT_SUCCESS == Ret
            	&& StreamBuf.pu8Data != NULL
            	&& StreamBuf.u32Size == AUDIO_ES_1_READ_LEN)
            {
            	if(g_decoder.had_copy_auido_data_length > StreamBuf.u32Size)
					{
						g_decoder.had_copy_auido_data_length -= StreamBuf.u32Size;
						Readlen = StreamBuf.u32Size;
					}
					else
					{
						Readlen = g_decoder.had_copy_auido_data_length;
						g_decoder.had_copy_auido_data_length = 0;
					}
				memcpy(StreamBuf.pu8Data,audio_buff, Readlen);
                //Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), AUDIO_ES_1_READ_LEN, g_pAudEsFile);
				b_start_audio_play = MT_FALSE;
				memset(audio_buff, 0, AUDIO_ES_1_READ_LEN);
                if (Readlen > 0)
                {
                    Ret = MT_UNF_AVPLAY_PutBuf64(hAvplay,  MT_UNF_AVPLAY_BUF_ID_ES_AUD, Readlen, (171 * aud_pts_fake++), NULL);
                    if (Ret != MT_SUCCESS)
                    {
                        DSS_ERR_PRINT("call MT_UNF_AVPLAY_PutBuf failed.\n");
                    }
                }
				MUTEX_UNLOCK();
				break;
            }
            else if (Ret != MT_SUCCESS)
            {
                MT_USLEEP(30000);
            }
           	MUTEX_UNLOCK();
			}while(1);
        }

        //MT_USLEEP(1000 * 10);
    }

    if((g_debug_enable == 2) && (g_decoder.data_source == 0))   fclose(EsFile);

    /*
	    所有A/V的ES数据送完之后:
	    需要先调用MT_UNF_AVPLAY_PutBuf64,EOS Flag参数置1(Data Length不能等于0);
	    然后调用MT_UNF_AVPLAY_FlushStream;
	    最后可以通过以下几个API获取EOS状态:
	      (1)MT_UNF_AVPLAY_RegisterEvent -> 主动通知：EOS后将会触发注册的callback
	      (2)MT_UNF_AVPLAY_IsBuffEmpty   -> 查询方式：EOS后将返回True
	      (3)MT_UNF_AVPLAY_GetStatusInfo -> 查询方式：EOS后，返回的Status中对应的enRunStatus为MT_UNF_AVPLAY_STATUS_EOS
	 */
    if(g_bVidPlay) {
    	//send EOS, 32 bytes of zero at least!
    	do {
            MT_UNF_AVPLAY_PUTBUFEX_OPT_S PutOpt;
            
            printf("send video eos flag\n");
        	Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, 32, &StreamBuf, 0);
    		if (Ret == MT_SUCCESS && StreamBuf.u32Size >= 32)
    		{
                memset(StreamBuf.pu8Data, 0, StreamBuf.u32Size);
                
				PutOpt.bEndOfFrm		= MT_TRUE;
				PutOpt.bContinue		= MT_TRUE;
				PutOpt.u32PtsValide 	= 1;
				PutOpt.u32FrameFinsh	= 1;
				PutOpt.u32EosFlag		= 1;

                Ret = MT_UNF_AVPLAY_PutBuf64(hAvplay,
    						MT_UNF_AVPLAY_BUF_ID_ES_VID,
    						StreamBuf.u32Size,
    						0,
    						&PutOpt);
    			if (Ret != MT_SUCCESS)
    			{
    				printf("error: MT_UNF_AVPLAY_PutBuf64 {VID} failed, return %d!\n",Ret);
    			}
                g_bVidPlay = MT_FALSE;
    			break;
    		}
            MT_USLEEP(30000);
        } while (1);
    }

    if(g_bAudPlay) {
    	//send EOS, 32 bytes of zero at least!
    	do {
            MT_UNF_AVPLAY_PUTBUFEX_OPT_S PutOpt;
            
            printf("send audio eos flag\n");
        	Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, 32, &StreamBuf, 0);
    		if (Ret == MT_SUCCESS && StreamBuf.u32Size >= 32)
    		{
                memset(StreamBuf.pu8Data, 0, StreamBuf.u32Size);
                
				PutOpt.bEndOfFrm		= MT_TRUE;
				PutOpt.bContinue		= MT_TRUE;
				PutOpt.u32PtsValide 	= 1;
				PutOpt.u32FrameFinsh	= 1;
				PutOpt.u32EosFlag		= 1;

                Ret = MT_UNF_AVPLAY_PutBuf64(hAvplay,
    						MT_UNF_AVPLAY_BUF_ID_ES_AUD,
    						StreamBuf.u32Size,
    						0,
    						&PutOpt);
    			if (Ret != MT_SUCCESS)
    			{
    				printf("error: MT_UNF_AVPLAY_PutBuf64 {Aud} failed, return %d!\n",Ret);
    			}
                g_bAudPlay = MT_FALSE;
    			break;
    		}
            MT_USLEEP(30000);
        } while (1);
    }
    
    if(!g_bVidPlay && !g_bAudPlay) {
        MT_UNF_AVPLAY_FLUSH_STREAM_OPT_S FlushOpt;
        MT_UNF_AVPLAY_STATUS_INFO_S stStatusInfo;
                    
        printf("do flush stream to wait EOS\n");
        memset(&FlushOpt, 0, sizeof(MT_UNF_AVPLAY_FLUSH_STREAM_OPT_S));
    	MT_UNF_AVPLAY_FlushStream(hAvplay, &FlushOpt);

#if 1
        do {
            //(3)通过MT_UNF_AVPLAY_GetStatusInfo()获取Buffer状态
            Ret = MT_UNF_AVPLAY_GetStatusInfo(hAvplay, &stStatusInfo);
            if (Ret != MT_SUCCESS)
            {
                printf("call MT_UNF_AVPLAY_GetStatusInfo failed.\n");
                break;
            }
            MT_USLEEP(30000);
        } while (stStatusInfo.enRunStatus != MT_UNF_AVPLAY_STATUS_EOS && stStatusInfo.enRunStatus != MT_UNF_AVPLAY_STATUS_STOP);
#else
        MT_BOOL bIsEmpty = MT_FALSE;
        do {
            MT_USLEEP(30000);
            MT_UNF_AVPLAY_IsBuffEmpty(hAvplay, &bIsEmpty);
            printf("Is buffer empty %d\n", bIsEmpty);
        } while(!bIsEmpty);
#endif
        MT_UNF_AVPLAY_STOP_OPT_S Stop;
        Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
        Stop.u32TimeoutMs = 0;
        MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD|MT_UNF_AVPLAY_MEDIA_CHAN_VID, &Stop);

        sleep(3);
        DSS_INFO_PRINT("\nSample DSS ES Play Finished, Esplay Exit!\n");
        //exit(0);
    }
    DSS_INFO_PRINT("exit ts_pthread  while\n");
    g_bStopEsThread = MT_TRUE;
    DSS_INFO_PRINT("exit ts_pthread .............\n");
    	
    return;
}
mt_s32  audio_video_init()
{
    mt_s32 Ret = 0, index;

    MT_UNF_AUDIOTRACK_ATTR_S  stTrackAttr;

    MT_UNF_VCODEC_TYPE_E VdecType = MT_UNF_VCODEC_TYPE_MPEG2;
    mt_u32 AdecType = HA_AUDIO_ID_MP3;
    
    MT_UNF_AVPLAY_ATTR_S AvplayAttr;
    MT_UNF_SYNC_ATTR_S AvSyncAttr;
    MT_UNF_AVPLAY_STOP_OPT_S Stop;
    MT_HA_DECODEMODE_E enAudioDecMode = HD_DEC_MODE_RAWPCM;
    mt_s32 s32DtsDtsCoreOnly = 0;
    MT_UNF_ENC_FMT_E g_enDefaultFmt = MT_UNF_ENC_FMT_1080i_50;
    MT_BOOL bAdvancedProfil = 1;
    mt_u32  u32CodecVersion = 8;
    MT_UNF_AVPLAY_OPEN_OPT_S stMaxCapbility;


	if(g_decoder.video_scid_filter != 0)
	{
		g_bVidPlay = MT_TRUE;
	}

	if(g_decoder.audio_scid_filter != 0)
	{
		g_bAudPlay = MT_TRUE;
	}

	g_enDefaultFmt = MT_UNF_ENC_FMT_1080i_50;

    MTADP_MCE_Exit();

    Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, g_enDefaultFmt);

    Ret = MT_UNF_AVPLAY_Init();

    if (Ret != MT_SUCCESS)
    {
        DSS_ERR_PRINT("call MT_UNF_AVPLAY_Init failed.\n");
        goto SND_DEINIT;
    }

    if (g_bAudPlay)
    {
        Ret = MTADP_Snd_Init();
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call SndInit failed.\n");
            goto VO_DEINIT;
        }

        Ret = MTADP_AVPlay_RegADecLib();
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_AVPLAY_RegisterAcodecLib failed.\n");
            goto SND_DEINIT;
        }
    }

    Ret  = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);

	//AvplayAttr.stStreamAttr.u32VidBufSize = (4*1024*1024);
    Ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &g_decoder.handle_avplay);
    if (Ret != MT_SUCCESS)
    {
        DSS_ERR_PRINT("call MT_UNF_AVPLAY_Create failed.\n");
        goto AVPLAY_DEINIT;
    }

    Ret = MT_UNF_AVPLAY_GetAttr(g_decoder.handle_avplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
    Ret |= MT_UNF_AVPLAY_SetAttr(g_decoder.handle_avplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    if (MT_SUCCESS != Ret)
    {
        DSS_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");
        goto AVPLAY_DEINIT;
    }

    if (g_bAudPlay)
    {
        Ret = MT_UNF_AVPLAY_ChnOpen(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
            goto AVPLAY_VSTOP;
        }

        //set to 0
        memset(&stTrackAttr,0,sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
        Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
            goto ACHN_CLOSE;
        }
        Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr,&g_decoder.handle_track);
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
            goto ACHN_CLOSE;
        }

        #ifndef SEND_TRACK_DATA_BY_USER

        Ret = MT_UNF_SND_Attach(g_decoder.handle_track, g_decoder.handle_avplay);
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_SND_Attach failed.\n");
            goto TRACK_DESTROY;
        }
        #endif

        Ret = MTADP_AVPlay_SetAdecAttr(g_decoder.handle_avplay, AdecType, enAudioDecMode, s32DtsDtsCoreOnly);
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");
            goto SND_DETACH;
        }

        Ret = MT_UNF_AVPLAY_Start(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_AVPLAY_Start failed.\n");
        }
    }

    Ret = MTADP_Disp_Init(g_enDefaultFmt);

    if (Ret != MT_SUCCESS)
    {
        DSS_ERR_PRINT("call DispInit failed.\n");
        return Ret;
    }

    if (g_bVidPlay)
    {
        Ret  = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        DSS_INFO_PRINT("[%s] line%d\n",__FUNCTION__,__LINE__);
        Ret |= MTADP_VO_CreatWin(MT_NULL, &g_decoder.handle_win);
        DSS_INFO_PRINT("[%s] line%d\n",__FUNCTION__,__LINE__);
       if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call VoInit failed.\n");
            MTADP_VO_DeInit();
            goto DISP_DEINIT;
        }

    }

    if (g_bVidPlay)
    {

        if (MT_UNF_VCODEC_TYPE_MVC == VdecType)
        {
            stMaxCapbility.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
            stMaxCapbility.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
            stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MVC;
        }
        else
        {
            stMaxCapbility.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_4096x2160;
            stMaxCapbility.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
            stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_BUTT;
        }

        Ret = MT_UNF_AVPLAY_ChnOpen(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stMaxCapbility);
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
            goto AVPLAY_DESTROY;
        }

        /*set compress attr*/
        MT_UNF_VCODEC_ATTR_S VcodecAttr;
        Ret = MT_UNF_AVPLAY_GetAttr(g_decoder.handle_avplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);

        if (MT_UNF_VCODEC_TYPE_VC1 == VdecType)
        {
            VcodecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = bAdvancedProfil;
            VcodecAttr.unExtAttr.stVC1Attr.u32CodecVersion = u32CodecVersion;
        }

        if (MT_UNF_VCODEC_TYPE_VP6 == VdecType)
        {
            VcodecAttr.unExtAttr.stVP6Attr.bReversed = 0;
        }

        VcodecAttr.enType = VdecType;
        VcodecAttr.u32UseDescInfoFlag = 0;
        Ret |= MT_UNF_AVPLAY_SetAttr(g_decoder.handle_avplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if (MT_SUCCESS != Ret)
        {
            DSS_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");
            goto AVPLAY_DEINIT;
        }


        Ret = MT_UNF_VO_AttachWindow(g_decoder.handle_win, g_decoder.handle_avplay);

        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_VO_AttachWindow failed.\n");
            goto VCHN_CLOSE;
        }

        Ret = MT_UNF_VO_SetWindowEnable(g_decoder.handle_win, MT_TRUE);
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_VO_SetWindowEnable failed.\n");
            goto WIN_DETATCH;
        }

        Ret = MTADP_AVPlay_SetVdecAttr(g_decoder.handle_avplay, VdecType, MT_UNF_VCODEC_MODE_NORMAL);

        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MTADP_AVPlay_SetVdecAttr failed.\n");
            goto WIN_DETATCH;
        }

        Ret = MT_UNF_AVPLAY_Start(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
        if (Ret != MT_SUCCESS)
        {
            DSS_ERR_PRINT("call MT_UNF_AVPLAY_Start failed.\n");
            goto WIN_DETATCH;
        }
    }

    if (g_decoder.video_scid_filter == 0)
    {
        MT_UNF_DISP_BG_COLOR_S BgColor;

        BgColor.u8Red   = 0;
        BgColor.u8Green = 200;
        BgColor.u8Blue  = 200;
        MT_UNF_DISP_SetBgColor(MT_UNF_DISPLAY1, &BgColor);
    }
    
    //MT_UNF_AVPLAY_RegisterEvent(g_decoder.handle_avplay, MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME, avplay_event);
    //MT_UNF_AVPLAY_RegisterEvent(g_decoder.handle_avplay, MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT, avplay_event);
    //MT_UNF_AVPLAY_RegisterEvent(g_decoder.handle_avplay, MT_UNF_AVPLAY_EVENT_EOS, avplay_event);

	return Ret; 
	



SND_DETACH:
    if (g_bAudPlay)
    {
        MT_UNF_SND_Detach(g_decoder.handle_track, g_decoder.handle_avplay);
        DSS_INFO_PRINT("minnan sample MT_UNF_SND_Detach!\n");
    }

TRACK_DESTROY:
    if (g_bAudPlay)
    {
        MT_UNF_SND_DestroyTrack(g_decoder.handle_track);
        DSS_INFO_PRINT("minnan sample MT_UNF_SND_DestroyTrack!\n");
    }

ACHN_CLOSE:
    if (g_bAudPlay)
    {
        MT_UNF_AVPLAY_ChnClose(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
        DSS_INFO_PRINT("minnan sample MT_UNF_AVPLAY_ChnClose!\n");
    }

AVPLAY_VSTOP:
    if (g_bVidPlay)
    {
        Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
        Stop.u32TimeoutMs = 0;
        MT_UNF_AVPLAY_Stop(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &Stop);
    }

WIN_DETATCH:
    if (g_bVidPlay)
    {
        MT_UNF_VO_SetWindowEnable(g_decoder.handle_win, MT_FALSE);
        MT_UNF_VO_DetachWindow(g_decoder.handle_win, g_decoder.handle_avplay);
    }

VCHN_CLOSE:
    if (g_bVidPlay)
    {
        MT_UNF_AVPLAY_ChnClose(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    }

AVPLAY_DESTROY:
    MT_UNF_AVPLAY_Destroy(g_decoder.handle_avplay);

AVPLAY_DEINIT:
    MT_UNF_AVPLAY_DeInit();

SND_DEINIT:
    if (g_bAudPlay)
    {
        MTADP_Snd_DeInit();
    }

VO_DEINIT:
    if (g_bVidPlay)
    {
        MT_UNF_VO_DestroyWindow(g_decoder.handle_win);
        MTADP_VO_DeInit();
    }

DISP_DEINIT:
    MTADP_Disp_DeInit();

    return Ret;
}
void  audio_video_deinit()
{
	MT_UNF_AVPLAY_STOP_OPT_S Stop;
	
	if (g_bAudPlay)
    {
        Stop.u32TimeoutMs = 0;
        Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
        MT_UNF_AVPLAY_Stop(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &Stop);

		MT_UNF_SND_Detach(g_decoder.handle_track, g_decoder.handle_avplay);
        DSS_INFO_PRINT("minnan sample MT_UNF_SND_Detach!\n");

		MT_UNF_SND_DestroyTrack(g_decoder.handle_track);
        DSS_INFO_PRINT("minnan sample MT_UNF_SND_DestroyTrack!\n");

        MT_UNF_AVPLAY_ChnClose(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
        DSS_INFO_PRINT("minnan sample MT_UNF_AVPLAY_ChnClose!\n");

    }

    if (g_bVidPlay)
    {
        Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
        Stop.u32TimeoutMs = 0;
        MT_UNF_AVPLAY_Stop(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &Stop);

		MT_UNF_VO_SetWindowEnable(g_decoder.handle_win, MT_FALSE);
        MT_UNF_VO_DetachWindow(g_decoder.handle_win, g_decoder.handle_avplay);

        MT_UNF_AVPLAY_ChnClose(g_decoder.handle_avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

    }


    MT_UNF_AVPLAY_Destroy(g_decoder.handle_avplay);

    MT_UNF_AVPLAY_DeInit();

    if (g_bAudPlay)
    {
        MTADP_Snd_DeInit();
    }

    if (g_bVidPlay)
    {
        MT_UNF_VO_DestroyWindow(g_decoder.handle_win);
        MTADP_VO_DeInit();
    }

    MTADP_Disp_DeInit();
}


int demux_init(void)
{
	int Ret = 0;
	 MT_UNF_DMX_CHAN_ATTR_S      stChnAttr;
	 
	u32 tspes_pid = 0xa0;
	u32 is_tstype = 0;	

	Ret = MT_UNF_DMX_Init();
	if (MT_SUCCESS != Ret){
	   printf("call MT_UNF_DMX_Init failed.\n");
	   return -1;
	}
	
	Ret = MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_0);

	Ret |= MT_UNF_DMX_GetChannelDefaultAttr(&stChnAttr);
	if (MT_SUCCESS != Ret){
		printf("call MT_UNF_DMX_GetChannelDefaultAttr failed!\n");
		return -1;
	}
	if(0==is_tstype){
		stChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_DSS;
	}else{
		stChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_POST;
	}
	stChnAttr.u32BufSize = (4*1024*1024);
	stChnAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
	Ret = MT_UNF_DMX_CreateChannel(DMX_ID,&stChnAttr,&dmxhandle);
	if (MT_SUCCESS != Ret){
		printf("call MT_UNF_DMX_CreateChannel failed! Ret=0x%x\n",Ret);
		return -2;
	}
	Ret = MT_UNF_DMX_SetChannelPID(dmxhandle,tspes_pid);
	Ret |= MT_UNF_DMX_OpenChannel(dmxhandle);
	if (MT_SUCCESS != Ret){
		printf("call MT_UNF_DMX_OpenChannel failed!\n");
		return -3;
	} 
	printf("createChannel >>>> config pid=0xa0\n");
	return Ret;
}

void demux_deinit()
{
	    
    MT_UNF_DMX_CloseChannel(dmxhandle);
    MT_UNF_DMX_DestroyChannel(dmxhandle);

    MT_UNF_DMX_DetachTSPort(DMX_ID);
    MT_UNF_DMX_DeInit();
}


void demod_pinmux_config(void)
{
    
	//input configuration in Serial_port 
	/*	
	devmem 0xbf13c048 32 0x3205
	devmem 0xbf5d0094 32 0x8
	devmem 0xbf13c1a0 32 0x1
	devmem 0xbf13c1a4 32 0x2
	devmem 0xbf50B000 32 0x1
	devmem 0xbf50B004 32 0xa
	devmem 0xbf138008 32 0x10
	devmem 0xbf200020 32 0
	devmem 0xbf200020 32 0xc7fb01ff
	devmem 0xbf200024 32
	*/
}

// ./sample_dss_play 3700 10000 0x10 0x11 1 Tx31_DSS_fixed_key_scid_16_64_96.trp 0
mt_s32 main(mt_s32 argc, mt_char *argv[])

{//./dss_parse_es_play freq sym Vscid Ascid Data_Sources(0:demod+dmx 1:file) file_path
       mt_s32 Ret = 0;
	char *filename_datasource=NULL;
	mt_unf_fe_dss_scid_filter_t scid_filter;
	
	/*Init DSS Decoder*/
 	memset(&g_decoder, 0, sizeof(struct dss_decoder));
	
	g_decoder.video_scid_filter = strtol(argv[3], NULL, 0);
	g_decoder.audio_scid_filter = strtol(argv[4], NULL, 0);
	g_decoder.data_source = strtol(argv[5], NULL, 0);
       g_debug_enable = strtol(argv[7], NULL, 0);

	DSS_INFO_PRINT("[%s %d] argc = %d,video_scid:0x%x,audio_scid:0x%x,data_source:%d(0:dmx,1:file)\n",__FUNCTION__,__LINE__,
																							argc,g_decoder.video_scid_filter,
																						g_decoder.audio_scid_filter,g_decoder.data_source);
   	if((g_decoder.audio_scid_filter == g_decoder.video_scid_filter)&&((g_decoder.audio_scid_filter != 0)||(g_decoder.video_scid_filter != 0)))
	{
		DSS_ERR_PRINT("[%s %d] Err Vscid == Ascid ,Exit App!!!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	
	demod_pinmux_config();
	g_scid_filter_enable = 0;
	mt_sys_init();
#ifdef DATA_Crypto_Process
	    Ret = mt_unf_misc_init();
	    Ret = mt_unf_misc_module_set(HAL_KT, 1);
	    Ret = mt_unf_misc_module_set(HAL_CRYPTO, 1);
	    Ret = mt_unf_misc_module_set(HAL_CRYPTO_DES, 1);
	    Ret = mt_unf_misc_module_set(HAL_CRYPTO_TDES, 1);
	    Ret = mt_unf_misc_module_set(HAL_CRYPTO_AES, 1);
	    Ret = mt_unf_misc_module_set(HAL_CRYPTO_SHA, 1);
	    Ret = mt_unf_misc_module_set(HAL_CRYPTO_RSA, 1);
	    Ret = mt_unf_misc_module_set(HAL_KL_CW, 1);
	    Ret = mt_unf_misc_module_set(HAL_RNG, 1);
	    Ret = mt_unf_misc_module_set(HAL_RNG2, 1);
	    //#ifdef CONFIG_MT_CIPHER_SUPPORT
	    Ret = mt_unf_cipher_init();
	    ts_packet_count = 0;
    //#endif
	dss_mss_input = mss_malloc(1024*1024, 0);
#endif //DATA_Crypto_Process		
	if(g_decoder.data_source == 0)
       {//Todo
		tuner_init(argc, argv);
		printf("BTC TO DVBS ,   nim mode inject date >>>> \n");
		if(argc >= 9){
			int i=0;
			int ret=0;
			if(strtol(argv[8], NULL, 0) == 1) // filter scid
			{
				scid_filter.tuner_id = 0;
				scid_filter.b_filter_mode = 0;
				for(i=0; i<DIRECTTV_SCID_FILTER_MAX_COUNT; i++)  scid_filter.u16_scid[i] = 0x0;
				for(i=0; i<DIRECTTV_SCID_FILTER_MAX_COUNT; i++)  scid_filter.u16_mask[i] = 0xffff;
				scid_filter.u16_scid[0] = g_decoder.video_scid_filter;
				scid_filter.u16_mask[0] = 0x0;
				scid_filter.u16_scid[1] = g_decoder.audio_scid_filter;
				scid_filter.u16_mask[1] = 0x0;
				ret = mt_unf_fe_dss_filter_scid(&scid_filter);
				g_scid_filter_enable = 1;
				printf("mt_unf_fe_dss_filter_scid >>>> ret=0x%x\n",ret);
			}
		}
		Ret = demux_init();
		if(Ret != 0)
		{
			DSS_ERR_PRINT("[%s %d] pthread_create PushEsDataTthread error",__FUNCTION__,__LINE__);
			goto END_IN;
		}
	}
	else if(g_decoder.data_source == 1)
	{
		printf("file mode inject date >>>> \n");
		if(argc < 6)
		{
			DSS_ERR_PRINT("[%s %d] Miss Input File Path,APP will use default file\n",__FUNCTION__,__LINE__);
			g_p_datasource = fopen("dss130_16_64_96.trp","rb");
			if(!g_p_datasource)
			{
				DSS_ERR_PRINT("Open dss130_16_64_96.trp file failed!\n");
				goto END_IN;
			}
		}
		else
		{
			filename_datasource = (char *)argv[6];
			g_p_datasource = fopen(filename_datasource,"rb");
			printf("fopen >>> filename_data source ret=%x, file = %s\n",g_p_datasource,filename_datasource);
			if(!g_p_datasource)
			{
				DSS_ERR_PRINT("Open %s file failed!\n",filename_datasource);
				goto END_IN;
			}
		}
	}

    Ret = audio_video_init();
	if(Ret != 0)
	{
		DSS_ERR_PRINT("[%s %d] pthread_create PushEsDataTthread error",__FUNCTION__,__LINE__);
		goto END_IN;
	}
  	printf("pthread_create >>> PushEsDataTthread\n");
    Ret =pthread_create(&g_EsThd1, MT_NULL, (mt_void *)PushEsDataTthread, MT_NULL);
    if (0 != Ret) {
        DSS_ERR_PRINT("[%s %d] pthread_create PushEsDataTthread error",__FUNCTION__,__LINE__);
        goto  END_AV;
    }
    MT_USLEEP(1000 * 1000);


       
    while (1) {
        mt_char InputCmd[32];
        DSS_INFO_PRINT("please input the q to quit!\n");
        fgets(InputCmd, 30, stdin);
        if ('q' == InputCmd[0]) {
            g_bStopEsThread = MT_TRUE;
            DSS_INFO_PRINT("prepare to quit!\n");
			pthread_join(g_EsThd1, MT_NULL);
            MT_USLEEP(10 * 1000);
            
            break;
        }
        MT_USLEEP(10 * 1000);
    }
    
	
END_AV:
	audio_video_deinit();
	if(g_decoder.data_source == 0){
		demux_deinit();
	}
END_IN:
	if (g_p_datasource)
	{
		fclose(g_p_datasource);
	}
CA_DEINIT:
    mt_sys_deinit();
 
    return Ret;
}
