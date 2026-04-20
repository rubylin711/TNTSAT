/*
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "ca_cert_impl.h"
#include "ca_cert.h"
#include "mt_unf_common.h"
#include "mt_unf_cert.h"
#include "nocs_sec_impl.h"
#include "mt_sec_ext.h"

#define MAGIC_CERT 0xdeadbeaf

#define OTP_PRIVILEGED_MODE_OFFSET                  (0x3E78)
#define OTP_PRIVILEGED_MODE_SIZE                    (4)
#define OTP_PRIVILEGED_MODE_SHIFT                   (10)
#define OTP_PRIVILEGED_MODE_MASK                    (0xF)
#define OTP_REEISPRIVILEGE_VALUE                    (0xF)
#if 0
#define CERT_LOG( fmt, ...) \
    ({ \
     printf("[%s:%u] Cert "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
     })

static void data_debug_dump(const unsigned char *data, unsigned int len)
{
	unsigned int i = 0, j = 0;

	for (i=0; i<len; i+=j) {
		for (j=0; ((j<8) && (i+j<len)); j++) {
			printf("0x%02x, ", data[i + j]);
		}

		printf("\n");
	}
}
#else
#define CERT_LOG( fmt, ...)
#endif
#define CERT_DBG(fmt,  ...) CERT_LOG( fmt, ##__VA_ARGS__)


/******************************************************************************/
/*                                                                            */
/*                           FUNCTION TABLE DEFINITION                        */
/*                                                                            */
/******************************************************************************/

#define sec_get_bit_val(reg, shift, vmask)  (((unsigned int)(reg) >> (shift)) & (vmask))

static mt_handle cert_handle = MT_INVALID_HANDLE;

//#define CERT_REE 0xaa
//#define CERT_TEE 0x55
//static int cert_cpu = CERT_REE;

static TBoolean checkTeePrivilegedMode(void)
{
#if 1
    mt_u8 otp_firewall[4];
    //CERT_DBG("\n");

    mt_otp_read_byte(OTP_PRIVILEGED_MODE_OFFSET, OTP_PRIVILEGED_MODE_SIZE, (MT_U8 *)otp_firewall);

    if (sec_get_bit_val(otp_firewall[1], 2, OTP_PRIVILEGED_MODE_MASK) != OTP_REEISPRIVILEGE_VALUE) {
        CERT_DBG("UNPRIVILEGED_MODE\n");
        return FALSE;
    } else {
        CERT_DBG("PRIVILEGED_MODE\n");
        return TRUE;
    }
#else
        return FALSE;
#endif
}

#define MT_REE_CERTLOCK_BIT    (0)
#define MT_TEE_CERTLOCK_BIT    (1)
#define MT_SW_REG3_REE_CERTLOCK_ADDR    (0xbf30f03c)
static int cert_open(void)
{
    int ret = 0;
    CERT_DBG("\n");
    if (0== cert_handle || cert_handle == MT_INVALID_HANDLE)
        ret = mt_unf_cert_open(&cert_handle);
    CERT_DBG("ret = 0x%x \n, ret");
    return ret;
}

static int cert_close(void)
{
    int ret = 0;
    CERT_DBG("\n");
    ret = mt_unf_cert_close(cert_handle);
    cert_handle = MT_INVALID_HANDLE;
    return ret;
}

static int cert_lock(void)
{
    int ret = 0;
    mt_u32 data = 0;
        CERT_DBG("\n");

    ret = mt_unf_cert_lock(cert_handle);
    if (!ret) {
        CERT_DBG("\n");
        while(1) //wait TEE unlock CERT
        {
            mt_sys_read_register(MT_SW_REG3_REE_CERTLOCK_ADDR, (mt_u32 *)&data);
            if ( (data & (1 << MT_TEE_CERTLOCK_BIT)) || (data & (1 << MT_REE_CERTLOCK_BIT)) ) {
                usleep(50);
		  CERT_DBG("\n");
                continue;
            }
            break;
        }
        CERT_DBG("\n");
        data = data | (1 << MT_REE_CERTLOCK_BIT);
        mt_sys_write_register(MT_SW_REG3_REE_CERTLOCK_ADDR, data);
    }
    return ret;
}

static int cert_unlock(void)
{
    int ret = 0;
    mt_u32 data = 0;

    mt_sys_read_register(MT_SW_REG3_REE_CERTLOCK_ADDR, (mt_u32 *)&data);
    data = data & (~(1 << MT_REE_CERTLOCK_BIT));
    mt_sys_write_register(MT_SW_REG3_REE_CERTLOCK_ADDR, data);

    ret = mt_unf_cert_unlock(cert_handle);
    return ret;
}

static int cert_reset(void)
{
    int ret = 0;
       CERT_DBG("\n");

    ret = mt_unf_cert_reset(cert_handle);
    return ret;
}

static int cert_exchange(uint32_t cmds_num,
			CERT_COMMAND_S *p_cmds, uint32_t *p_processed_num)
{
    int ret = 0;
/*
    if (cert_cpu == CERT_REE)
        ret = mt_unf_cert_exchange(cert_handle, cmds_num, p_cmds, p_processed_num);
    else
        ret = tee_cert_exchange(cmds_num, p_cmds, p_processed_num);
*/
    CERT_DBG("\n");
    if (checkTeePrivilegedMode()) {
        return CERT_ERROR;
    }
    ret = mt_unf_cert_exchange(cert_handle, cmds_num, p_cmds, p_processed_num);
    return ret;
}

static int cert_export_key(uint32_t slot_id, uint32_t ext_attr)
{
    int ret = 0;
    CERT_DBG("\n");
    ret = mt_unf_cert_export_key(cert_handle, slot_id, ext_attr);
    return ret;
}

static int cert_key_ack(void)
{
    int ret = 0;
    CERT_DBG("\n");
    ret = mt_unf_cert_key_ack(cert_handle);
    return ret;
}

static TCertStatus certLock(TCertResourceHandle* pxResourceHandle)
{
    TCertResourceHandle xResHandle = NULL;
    CERT_DBG("\n");

    if (pxResourceHandle == NULL) {
        return CERT_ERROR;
    }
    CERT_DBG("\n");
    if(0 != cert_open()) {
        return CERT_ERROR;
    }
    CERT_DBG("\n");
    if(0 != cert_lock()) {
        cert_close();
        return CERT_ERROR;
    }
    CERT_DBG("\n");
    xResHandle = (TCertResourceHandle)malloc(sizeof(struct SCertResourceHandle));
    if (xResHandle == NULL) {
        cert_unlock();
        cert_close();
        return CERT_ERROR;
    }

    //xResHandle->cert_handle = (unsigned int)handle;
    xResHandle->magic_cert = MAGIC_CERT;

    *pxResourceHandle = xResHandle;

    return CERT_NO_ERROR;
}

static TCertStatus certUnlock(TCertResourceHandle xResourceHandle)
{

    CERT_DBG("\n");
    if ((xResourceHandle == NULL || xResourceHandle->magic_cert != MAGIC_CERT)) {
	    return CERT_ERROR_BAD_HANDLE;
    }

    if (cert_unlock() != 0) {
        cert_close();
        //xResourceHandle->cert_handle = MT_INVALID_HANDLE;
        xResourceHandle->magic_cert = 0xffffffff;
        free(xResourceHandle);
	    return CERT_ERROR;
    }

    cert_close();

    //xResourceHandle->cert_handle = MT_INVALID_HANDLE;
    xResourceHandle->magic_cert = 0xffffffff;
    free(xResourceHandle);

    return CERT_NO_ERROR;
}

static TCertStatus certExchange(
	TCertResourceHandle  xResourceHandle,
	size_t               xNumOfCommands,
	const TCertCommand*       pxCommands,
	size_t*             pxNumOfProcessedCommands)
{
    int ret;
    CERT_DBG("[REE]\n");

    if ((xResourceHandle == NULL || xResourceHandle->magic_cert != MAGIC_CERT)) {
	    return CERT_ERROR_BAD_HANDLE;
    }

    if (pxNumOfProcessedCommands == NULL
        || pxCommands == NULL || xNumOfCommands == 0) {
        return CERT_ERROR;
    }


    ret = cert_exchange(xNumOfCommands,
        (const CERT_COMMAND_S *)pxCommands, (mt_u32 *)pxNumOfProcessedCommands);


    if (CERT_NO_ERROR == ret) {
        return CERT_NO_ERROR;
    } else if (CERT_ERROR_TIMEOUT == ret) {
        return CERT_ERROR_TIMEOUT;
    } else
        return CERT_ERROR;
}

static TCertStatus certAklReset(void)
{
    CERT_DBG("\n");

    cert_reset();

    return CERT_NO_ERROR;
}


//Function table functions
static TCertFunctionTable    gCertFunctionTable =
{
    CERTAPI_VERSION_INT,
    certLock,
    certUnlock,
    certExchange,
    (TCertUseEncryptionKey)NULL,
    (TCertUseDecryptionKey)NULL,
    certAklReset,
};

TCertFunctionTable* certGetFunctionTable(void)
{
	/* TODO */
	return &gCertFunctionTable;
}

TSignedInt32 certExportKey(TUnsignedInt32 keyslot)
{
    int ret = CERT_NO_ERROR;

    CERT_DBG("\n");

    ret = cert_export_key(keyslot, 0);
    if(ret != 0) {
        return CERT_ERROR;
    }

    ret = cert_key_ack();
    if(ret != 0) {
        return CERT_ERROR;
    }

    return ret;
}

