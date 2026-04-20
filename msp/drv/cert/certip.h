/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CERT_IP_H__
#define __CERT_IP_H__

typedef enum {
    CERTIP_TIMEOUT_DEFAULT,
    CERTIP_TIMEOUT_OTP,
    LAST_CERTIP_TIMEOUT
} CERTIP_TIMEOUT_TYPE_E;

typedef struct _certip_command_struct
{
    unsigned char input[32];
    unsigned char output[32];
    unsigned char status[4];
    unsigned char opcodes[4];
    CERTIP_TIMEOUT_TYPE_E timeout;
} certip_command_s;

int certip_reset(void);
int certip_exchange(unsigned int cmds_num,
                    certip_command_s *p_cmds,
                    unsigned int *p_processed_num,
                    int *cert_status);
int certip_output_key(unsigned int slot_id, unsigned int ext_attr);
int certip_key_ack(void);

#endif //__CERT_IP_H__
