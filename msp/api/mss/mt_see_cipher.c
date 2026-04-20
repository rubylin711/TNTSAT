/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "mt_common.h"

#include "mt_unf_cipher_v2.h"

#include "mt_see_misc.h"
#include "mt_see_cipher.h"
#include "mt_mpi_ipcs.h"

typedef struct _MT_KT_SEE_KEY_ATTR_S {
    unsigned int AES_ONOFF;
    unsigned int DES_ONOFF;
    unsigned int TDES_ONOFF;
    unsigned int CSAv2_ONOFF;
    unsigned int CSAv3_ONOFF;
    unsigned int SM2_3_4_ONOFF;
    unsigned int HMAC_ONOFF;
    unsigned int M2M_ONOFF;
    unsigned int ASA_ONOFF;
    unsigned int Multi2_ONOFF;
    unsigned int REE_ONOFF;
    unsigned int DEC_ONOFF;
    unsigned int ENC_ONOFF;
    unsigned int KEY_SIZE;
    unsigned int KEY_SOURCE;
    unsigned int TDES_KEYCHK;
} MT_KT_SEE_KEY_ATTR_S;

int mt_unf_see_cipher_init(unsigned int input_addr, unsigned int input_max, unsigned int output_addr, unsigned int output_max)
{
    //unused
    (void)input_addr;
    (void)input_max;
    (void)output_addr;
    (void)output_max;

    return SEE_CIPHER_SUCCESS;
}
/*!
    struct for maibox set data
  */
typedef struct _see_cipher_config_t
{
    /*!
    *  [input] input data start address
     */     
    unsigned int input_s;
    /*!
    *  [input] the length of input data
     */   
    unsigned int input_len;
     /*!
    *  [input] output data start address
     */     
    unsigned int output_s;
    /*!
    *  [input/output] the length of output data
     */   
    unsigned int output_len;
}mt_see_cipher_config_t;

int mt_unf_see_key_attr_create(MT_CIPHER_CTRL_S *p_ctrl, unsigned int *p_key_attr, unsigned int * p_kattr_size)
{
    MT_KT_SEE_KEY_ATTR_S *p_ka = NULL;

    if((NULL == p_ctrl) || (NULL == p_key_attr) || (NULL == p_kattr_size))
        return SEE_CIPHER_ERROR;

    p_ka = (MT_KT_SEE_KEY_ATTR_S *)malloc(sizeof(MT_KT_SEE_KEY_ATTR_S));
    if(NULL == p_ka)
        return SEE_CIPHER_ERROR;

    memset(p_ka, 0x00, sizeof(MT_KT_SEE_KEY_ATTR_S));
    p_ka->KEY_SIZE = 16;
    switch (p_ctrl->algorithm) {
        case MT_CIPHER_ALG_DES:
            p_ka->DES_ONOFF = 1;
            p_ka->KEY_SIZE = 8;
            break;
        case MT_CIPHER_ALG_TDES:
            p_ka->TDES_ONOFF = 1;
            p_ka->KEY_SIZE = 16;
            break;
        case MT_CIPHER_ALG_AES:
            p_ka->AES_ONOFF = 1;
            break;
        case MT_CIPHER_ALG_HMAC:
            p_ka->HMAC_ONOFF = 1;
            break;
        case MT_CIPHER_ALG_CSA2:
            p_ka->CSAv2_ONOFF = 1;
            p_ka->KEY_SIZE = 8;
            break;
        case MT_CIPHER_ALG_CSA3:
            p_ka->CSAv3_ONOFF = 1;
            p_ka->KEY_SIZE = 16;
            break;
        default:
            break;
    }

    if ((p_ctrl->core == MT_CIPHER_CORE_M2M_RAW) || p_ctrl->core == MT_CIPHER_CORE_M2M_TS)
        p_ka->M2M_ONOFF = 1;

    if (p_ctrl->operation == MT_CIPHER_OPERATION_DECRYPT)
        p_ka->DEC_ONOFF = 1;
    else
        p_ka->ENC_ONOFF = 1;


    *p_key_attr = (unsigned int)(ulong)p_ka;
    *p_kattr_size = sizeof(MT_KT_SEE_KEY_ATTR_S);
    return SEE_CIPHER_SUCCESS;
}

void mt_unf_see_key_attr_destroy(unsigned int *p_key_attr)
{
    if(*p_key_attr)
    {
        free((void *)(ulong)(*p_key_attr));
    }
}

static int _mt_unf_scipher_transfer(mt_u32 msg_cmd, mt_u32 addr, mt_see_cipher_config_t *p_see_cipher_config)
{
    unsigned int msg_len = 0;
    int ret = SEE_CIPHER_ERROR;
    mpi_ipcs_msg_t msg = {0};
    mpi_ipcs_msg_t recv_msg = {0};
    mt_u32 output_len;
    mt_u32 cnt = 0;

    if (NULL == p_see_cipher_config)
    {
        return SEE_CIPHER_ERROR;
    }

    msg_len = p_see_cipher_config->input_len;
    msg.channel_id = MT_IPCS_CHAN0;
    msg.msg_id = msg_cmd;
    msg.ack = 0;
    msg.param1 = ((unsigned int)addr) | 0xA0000000;
    msg.param2 = msg_len;//message length

    ret = mt_mpi_ipcs_send_scpu_msg(&msg);

    recv_msg.channel_id = MT_IPCS_CHAN0;
    recv_msg.ack = 0;

    while(cnt < 50)
    {
        cnt++;
        MT_USLEEP(1000);
        ret |= mt_mpi_ipcs_get_scpu_response(&recv_msg);
        if(recv_msg.msg_id == msg_cmd && ret == 0)
            break;
    }

    if(recv_msg.msg_id == msg_cmd && ret == 0)
    {
        output_len = recv_msg.param2 & 0xFFFFFF;

        p_see_cipher_config->output_len = output_len;
        return SEE_CIPHER_SUCCESS;
    }
    else
        ret = SEE_CIPHER_ERROR;

    return ret;
}

int mt_unf_see_cipher_transfer(mt_u32 msg_cmd, mt_u8 *p_input, mt_u32 input_len, mt_u8 *p_output, mt_u32 *p_output_len)
{
    int ret = 0;
    mt_u32 outbuf_size;
    mt_see_cipher_config_t *p_see_cipher_config;
    mt_ipcs_mmz_buf_s  mmz_input_buf = {0};
    mt_ipcs_mmz_buf_s  mmz_output_buf = {0};
    mt_ipcs_mmz_buf_s mmz_see_cipher_config;
    phys_addr_t input_phy;
    phys_addr_t output_phy;
    phys_addr_t tmp_addr;
    ulong pu32Size;

    if (p_input == NULL || input_len == 0)
        return SEE_CIPHER_ERROR;

    if (mt_mmz_get_phyaddr((mt_void *)p_input, &tmp_addr, &pu32Size) == 0)
    {
        input_phy = tmp_addr;
    }
    else
    {
        ret = mt_mpi_ipcs_buf_malloc(&mmz_input_buf, input_len);
        if (MT_SUCCESS != ret)
        {
            ret = SEE_CIPHER_ERROR;
            goto EXIT;
        }

        memcpy(mmz_input_buf.user_viraddr, p_input, input_len);
        input_phy = mmz_input_buf.phyaddr;
    }

    ret = mt_mpi_ipcs_buf_malloc(&mmz_see_cipher_config, sizeof(mt_see_cipher_config_t));
    if (MT_SUCCESS != ret)
    {
        ret = SEE_CIPHER_ERROR;
        goto EXIT;
    }

    p_see_cipher_config = (mt_see_cipher_config_t *)mmz_see_cipher_config.user_viraddr;
    memset(p_see_cipher_config, 0, sizeof(mt_see_cipher_config_t));

    p_see_cipher_config->input_s = input_phy | 0xA0000000;
    p_see_cipher_config->input_len = input_len;

    outbuf_size = 0;
    if (p_output && p_output_len)
    {
        if (*p_output_len > 0)
            outbuf_size = *p_output_len;
        else
        outbuf_size = 1024;
    }

    output_phy = 0;
    if (outbuf_size > 0)
    {
        if (mt_mmz_get_phyaddr((mt_void *)p_output, &tmp_addr, &pu32Size) == 0)
        {
            output_phy = tmp_addr;
        }
        else
        {
            ret = mt_mpi_ipcs_buf_malloc(&mmz_output_buf, outbuf_size);
            if (MT_SUCCESS != ret)
            {
                ret = SEE_CIPHER_ERROR;
                goto EXIT;
            }
            output_phy = mmz_output_buf.phyaddr;
        }
    }

    p_see_cipher_config->output_s = output_phy | 0xA0000000;
    p_see_cipher_config->output_len = outbuf_size;

    ret = _mt_unf_scipher_transfer(msg_cmd, mmz_see_cipher_config.phyaddr, p_see_cipher_config);
    if (SEE_CIPHER_SUCCESS != ret)
    {
        ret = SEE_CIPHER_ERROR;
        goto EXIT;
    }

    if (p_output && p_output_len)
    {
        if (p_see_cipher_config->output_len < outbuf_size)
        {
            if (mmz_output_buf.user_viraddr != NULL)
            {
                memcpy(p_output, (unsigned char*)mmz_output_buf.user_viraddr, p_see_cipher_config->output_len);
            }
            *p_output_len = p_see_cipher_config->output_len;
        }
        else
        {
            ret = SEE_CIPHER_ERROR;
            goto EXIT;
        }
    }

EXIT:
    mt_mpi_ipcs_buf_free(&mmz_see_cipher_config);
    mt_mpi_ipcs_buf_free(&mmz_input_buf);
    mt_mpi_ipcs_buf_free(&mmz_output_buf);

    return ret;
}

