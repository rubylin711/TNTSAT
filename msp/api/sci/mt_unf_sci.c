/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "mt_unf_sci.h"
#include "./protocol/globals.h"

#include "drv_sci_ioctl.h"
#include "mt_drv_struct.h"
#include "mt_module_debug.h"

#ifndef MT_ERR_SCI
#define MT_ERR_SCI printf
#endif
//FIXME: bionic not support
#ifdef ANDROID
enum {
    PTHREAD_CANCEL_ENABLE,
#define PTHREAD_CANCEL_ENABLE   PTHREAD_CANCEL_ENABLE
    PTHREAD_CANCEL_DISABLE
#define PTHREAD_CANCEL_DISABLE  PTHREAD_CANCEL_DISABLE
};
#endif
#if 0
static const mt_u8 s_szSCIVersion[] = "SDK_VERSION:["\
                                      MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
                                      __DATE__", "__TIME__"]";
#endif
static struct s_reader reader[2];
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int fd = -1;
static int init_count = 0;

mt_s32 mt_unf_sci_init(mt_void)
{
    int i = 0;
    int state = 0;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
    pthread_mutex_lock(&mutex);

    if(init_count > 0) {
        pthread_mutex_unlock(&mutex);
        pthread_setcancelstate(state, &state);
        return MT_SUCCESS;
    }

    fd = open("/dev/" UMAP_DEVNAME_SCI, O_RDWR);
    if(fd < 0) {
        pthread_mutex_unlock(&mutex);
        pthread_setcancelstate(state, &state);
        return MT_FAILURE;
    }

    memset(reader, 0, sizeof(reader));
    for(; i < 2; i ++) {
        reader[i].handle = fd;
        reader[i].dev_id = (mt_u8)i;
        reader[i].use_default_etu = 1;
        snprintf(reader[i].device, sizeof(reader[i].device),"%s", "/dev/" UMAP_DEVNAME_SCI);
        cardreader_m88cc6000(&reader[i].crdr);
    }
    init_count ++;
    pthread_mutex_unlock(&mutex);
    pthread_setcancelstate(state, &state);

    return MT_SUCCESS;
}

mt_s32 mt_unf_sci_deinit(mt_void)
{
    int state = 0;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
    pthread_mutex_lock(&mutex);
    if(init_count == 0) {
        pthread_mutex_unlock(&mutex);
        pthread_setcancelstate(state, &state);
        return MT_SUCCESS;
    }
    init_count --;
    close(fd);
    reader[0].handle = -1;
    reader[1].handle = -1;
    pthread_mutex_unlock(&mutex);
    pthread_setcancelstate(state, &state);

    return MT_SUCCESS;
}

/*******************************************
Function:              MT_UNF_SCI_Open
Description:  open SCI device
Calls:       MT_SCI_Open
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_open(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_PROTOCOL_E enSciProtocol, mt_u32 u32Frequency)
{
    mt_s32 Ret = MT_SUCCESS;
    int state = 0;
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (enSciProtocol >= MT_UNF_SCI_PROTOCOL_BUTT) {
        MT_ERR_SCI("para enSciProtocol is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (MT_UNF_SCI_PROTOCOL_T14 == enSciProtocol) {
        if ((u32Frequency < 1000) || (u32Frequency > 6000)) {
            MT_ERR_SCI("para u32Frequency is invalid.\n");
            return MT_ERR_SCI_INVALID_PARA;
        }
    } else {
        if ((u32Frequency < 1000) || (u32Frequency > 5000)) {
            MT_ERR_SCI("para u32Frequency is invalid.\n");
            return MT_ERR_SCI_INVALID_PARA;
        }
    }

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
    pthread_mutex_lock(&mutex);
    switch(enSciProtocol) {
    case MT_UNF_SCI_PROTOCOL_T0:
        reader[enSciPort].protocol_type = ATR_PROTOCOL_TYPE_T0;
        break;
    case MT_UNF_SCI_PROTOCOL_T1:
        reader[enSciPort].protocol_type = ATR_PROTOCOL_TYPE_T1;
        break;
    case MT_UNF_SCI_PROTOCOL_T14:
        reader[enSciPort].protocol_type = ATR_PROTOCOL_TYPE_T14;
        break;
    default:
        reader[enSciPort].protocol_type = ATR_PROTOCOL_TYPE_T0;
        break;
    }
    reader[enSciPort].mhz = (mt_u32)(u32Frequency / 10);
    reader[enSciPort].read_timeout =  (mt_u32)((9600 * 372 * 100) /reader[enSciPort].mhz);
    Ret = ICC_Async_Device_Init(&reader[enSciPort]);
    if(Ret != 0) {
        pthread_mutex_unlock(&mutex);
        pthread_setcancelstate(state, &state);
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }
    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    if(reader[enSciPort].protocol_type == ATR_PROTOCOL_TYPE_T14) {
        attr.Hz = 6000000;
        attr.etu = 620;
        attr.parity_en = 0;
        attr.error_handle_en = 0;
        attr.stop_width = SCI_ONE_STOP;
        attr.read_timeout = (9600 * 620 * 100) /600;
        attr.N = 0;
        reader[enSciPort].mhz = 600;
    } else {
        attr.Hz = u32Frequency * 1000;
        attr.etu = 372;
        if(reader[enSciPort].protocol_type == ATR_PROTOCOL_TYPE_T1) {
            attr.parity_en = 1;
            attr.error_handle_en = 0;
        } else {
            attr.parity_en = 1;
            attr.error_handle_en = 1;
        }
        attr.stop_width = SCI_TWO_STOPS;
        attr.read_timeout = reader[enSciPort].read_timeout;
        attr.N = 0;
    }
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    pthread_mutex_unlock(&mutex);
    pthread_setcancelstate(state, &state);
    return MT_SUCCESS;
}
/*******************************************
Function:              MT_UNF_SCI_Reconfig
Description:  Reconfig SCI device
Calls:       MT_SCI_Config
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_reconfig(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_PROTOCOL_E enSciProtocol, mt_u32 u32Frequency)
{
	//mt_s32 Ret = MT_SUCCESS;
	//int state = 0;
	SCI_ATTR_S attr;

	if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
		MT_ERR_SCI("para enSciPort is invalid.\n");
		return MT_ERR_SCI_INVALID_PARA;
	}

	if (enSciProtocol >= MT_UNF_SCI_PROTOCOL_BUTT) {
		MT_ERR_SCI("para enSciProtocol is invalid.\n");
		return MT_ERR_SCI_INVALID_PARA;
	}

	if (MT_UNF_SCI_PROTOCOL_T14 == enSciProtocol) {
		if ((u32Frequency < 1000) || (u32Frequency > 6000)) {
			MT_ERR_SCI("para u32Frequency is invalid.\n");
			return MT_ERR_SCI_INVALID_PARA;
		}
	} else {
		if ((u32Frequency < 1000) || (u32Frequency > 5000)) {
			MT_ERR_SCI("para u32Frequency is invalid.\n");
			return MT_ERR_SCI_INVALID_PARA;
		}
	}

	//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
	pthread_mutex_lock(&mutex);
	switch(enSciProtocol) {
	case MT_UNF_SCI_PROTOCOL_T0:
		reader[enSciPort].protocol_type = ATR_PROTOCOL_TYPE_T0;
		break;
	case MT_UNF_SCI_PROTOCOL_T1:
		reader[enSciPort].protocol_type = ATR_PROTOCOL_TYPE_T1;
		break;
	case MT_UNF_SCI_PROTOCOL_T14:
		reader[enSciPort].protocol_type = ATR_PROTOCOL_TYPE_T14;
		break;
	default:
		reader[enSciPort].protocol_type = ATR_PROTOCOL_TYPE_T0;
		break;
	}
	reader[enSciPort].mhz = (mt_u32)(u32Frequency / 10);
	reader[enSciPort].read_timeout =  (mt_u32)((9600 * 372 * 100) /reader[enSciPort].mhz);
	attr.dev_id = enSciPort;
	ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
	if(reader[enSciPort].protocol_type == ATR_PROTOCOL_TYPE_T14) {
		attr.Hz = 6000000;
		attr.etu = 620;
		attr.parity_en = 0;
		attr.error_handle_en = 0;
		attr.stop_width = SCI_ONE_STOP;
		attr.read_timeout = (9600 * 620 * 100) /600;
		attr.N = 0;
		reader[enSciPort].mhz = 600;
	} else {
		attr.Hz = u32Frequency * 1000;
		attr.etu = 372;
		if(reader[enSciPort].protocol_type == ATR_PROTOCOL_TYPE_T1) {
			attr.parity_en = 1;
			attr.error_handle_en = 0;
		} else {
			attr.parity_en = 1;
			attr.error_handle_en = 1;
		}
		attr.stop_width = SCI_TWO_STOPS;
		attr.read_timeout = reader[enSciPort].read_timeout;
		attr.N = 0;
	}
	ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
	pthread_mutex_unlock(&mutex);
	//pthread_setcancelstate(state, &state);
	return MT_SUCCESS;
}


/*******************************************
Function:              MT_UNF_SCI_Close
Description:  close SCI device
Calls:        MT_SCI_Close
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_close(MT_UNF_SCI_PORT_E enSciPort)
{
    mt_s32 Ret = MT_SUCCESS;
    int state = 0;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }


    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
    pthread_mutex_lock(&mutex);
    Ret = ICC_Async_Close(&reader[enSciPort]);
    if(Ret != 0) {
        pthread_mutex_unlock(&mutex);
        pthread_setcancelstate(state, &state);
        return MT_UNF_SCI_ERR_CLOSE_FAIL;
    }
    pthread_mutex_unlock(&mutex);
    pthread_setcancelstate(state, &state);
    return MT_SUCCESS;
}

/*******************************************
Function:              MT_UNF_SCI_ResetCard
Description:  reset card
Calls:        MT_UNF_SCI_ResetCard
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_resetcard(MT_UNF_SCI_PORT_E enSciPort, MT_BOOL bWarmResetValid)
{
    mt_s32 Ret;
    ATR atr;
    SCI_STATUS_S status;
    int state = 0;
    MT_UNF_SCI_PORT_E port = 0;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if ((bWarmResetValid != MT_TRUE)
        && (bWarmResetValid != MT_FALSE)
       ) {
        MT_ERR_SCI("para bWarmResetValid is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    status.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_STATUS, &status);
    if(!status.status) {
        return MT_UNF_SCI_ERR_CARD_NOT_READY;
    }
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
    pthread_mutex_lock(&mutex);
    if(bWarmResetValid == MT_TRUE) {

        Ret = ICC_Async_Activate(&reader[enSciPort], &atr, reader[enSciPort].use_default_etu);
    } else {
        port = enSciPort;

        ioctl(reader[enSciPort].handle, SCI_IOC_DEACTIVATE, &port);
        ioctl(reader[enSciPort].handle, SCI_IOC_ACTIVATE, &port);

        Ret = ICC_Async_Activate(&reader[enSciPort], &atr, reader[enSciPort].use_default_etu);
    }

    if(Ret != 0) {
        pthread_mutex_unlock(&mutex);
        pthread_setcancelstate(state, &state);
        return MT_UNF_SCI_ERR_RESET_FAIL;
    }
    pthread_mutex_unlock(&mutex);
    pthread_setcancelstate(state, &state);

    return Ret;
}

/*******************************************
Function:              MT_UNF_SCI_DeactiveCard
Description:  deactive card
Calls:        MT_SCI_DeactiveCard
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_deactivecard(MT_UNF_SCI_PORT_E enSciPort)
{
    MT_UNF_SCI_PORT_E port = 0;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }
    port = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_DEACTIVATE, &port);
    return MT_SUCCESS;
}

/*******************************************
Function:              MT_UNF_SCI_GetATR
Description:  get ATR data
Calls:        MT_SCI_GetATR
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_getatr(MT_UNF_SCI_PORT_E enSciPort, mt_u8 *pu8AtrBuf, mt_u32 u32AtrBufSize, mt_u8 *pu8AtrRcvCount)
{

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (!pu8AtrBuf) {
        MT_ERR_SCI("para pu8AtrBuf is null.\n");
        return MT_ERR_SCI_NULL_PTR;
    }

    if (!u32AtrBufSize) {
        MT_ERR_SCI("para u32AtrBufSize is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (!pu8AtrRcvCount) {
        MT_ERR_SCI("para pu8AtrRcvCount is null.\n");
        return MT_ERR_SCI_NULL_PTR;
    }

    if(reader[enSciPort].card_atr_length > (char)u32AtrBufSize) {
        return MT_UNF_SCI_ERR_ATR_BUF_TOO_LITTLE;
    }

    pthread_mutex_lock(&mutex);
    memcpy(pu8AtrBuf, reader[enSciPort].card_atr, (mt_u32)(reader[enSciPort].card_atr_length));
    *pu8AtrRcvCount = (mt_u8)(reader[enSciPort].card_atr_length);
    pthread_mutex_unlock(&mutex);
    return MT_SUCCESS;
}

/*******************************************
Function:              MT_UNF_SCI_GetCardStatus
Description:  get the status of card
Calls:        MT_UNF_SCI_GetCardStatus
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_getcardstatus(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_STATUS_E *penSciStatus)
{
    SCI_STATUS_S status;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (!penSciStatus) {
        MT_ERR_SCI("para penSciStatus is null.\n");
        return MT_ERR_SCI_NULL_PTR;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    status.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_STATUS, &status);
    if(status.status) {
        *penSciStatus = MT_UNF_SCI_STATUS_READY;
    } else {
        *penSciStatus = MT_UNF_SCI_STATUS_NOCARD;
    }
    return MT_SUCCESS;

}

/*******************************************
Function:              MT_UNF_SCI_Send
Description:  send data to card
Calls:        MT_SCI_Send
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_send(MT_UNF_SCI_PORT_E enSciPort, mt_u8 *pSciSendBuf, mt_u32 u32SendLen, mt_u32 *pu32ActLen,
                       mt_u32 u32TimeoutUs)
{
    SCI_DATA_S data;
    SCI_ATTR_S attr;
    static mt_u32 timeout = 0;
#if 0
    SCI_ATTR_S attr;
    mt_u32 bt;
#endif
    int ret = MT_SUCCESS;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (!pSciSendBuf) {
        MT_ERR_SCI("para pSciSendBuf is null.\n");
        return MT_ERR_SCI_NULL_PTR;
    }

    if (!pu32ActLen) {
        MT_ERR_SCI("para pu32ActLen is null.\n");
        return MT_ERR_SCI_NULL_PTR;
    }

    if (!u32SendLen) {
        MT_ERR_SCI("para u32SendLen is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (u32SendLen > 512) {
        MT_ERR_SCI("para u32SendLen is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }
#if 0
    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    bt = ((attr.N + 12) * attr.etu * 100) /reader->mhz;
    MT_USLEEP(1000 + bt);
#endif

    if(timeout!=u32TimeoutUs) {
        attr.dev_id = enSciPort;
        ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
        attr.write_timeout = u32TimeoutUs;
        ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
        timeout = u32TimeoutUs;
    }

    data.dev_id = enSciPort;
    data.data_buf = (ulong)pSciSendBuf;
    data.data_len = u32SendLen;
    ret = ioctl(reader[enSciPort].handle, SCI_IOC_SEND_DATA, &data);
    *pu32ActLen = u32SendLen;

    if(ret > 0) {
        return MT_SUCCESS;
    } else {
        return MT_FAILURE;
    }
}

/*******************************************
Function:              MT_UNF_SCI_Receive
Description:  receive data from card
Calls:        MT_SCI_Receive
Data Accessed:  NA
Data Updated:   NA
Input:          u32TimeoutUs: unit microsecond (us)
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_receive (MT_UNF_SCI_PORT_E enSciPort, mt_u8 *pSciReceiveBuf, mt_u32 u32ReceiveLen, mt_u32 *pu32ActLen,
                           mt_u32 u32TimeoutUs)
{
    mt_s32 Ret = MT_SUCCESS;
    SCI_DATA_S data;
    SCI_ATTR_S attr;
    int i = 0, j = 0, k = 0;
    static mt_u32 timeout = 0;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (!pSciReceiveBuf) {
        MT_ERR_SCI("para pSciReceiveBuf is null.\n");
        return MT_ERR_SCI_NULL_PTR;
    }

    if (!pu32ActLen) {
        MT_ERR_SCI("para pu32ActLen is null.\n");
        return MT_ERR_SCI_NULL_PTR;
    }

    if ((u32ReceiveLen > 512) || (0 == u32ReceiveLen)) {
        MT_ERR_SCI("para u32ReceiveLen is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    if(timeout!=u32TimeoutUs) {
        attr.dev_id = enSciPort;
        ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
        attr.read_timeout = u32TimeoutUs;
        ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
        timeout = u32TimeoutUs;
    }

    data.dev_id = enSciPort;

    i = (int)u32ReceiveLen;
    while(1) {
        data.data_len = (unsigned long)i;
        data.data_buf = (ulong)(&pSciReceiveBuf[k]);
        j = ioctl(reader[enSciPort].handle, SCI_IOC_RECIEVE_DATA, &data);
        if(j > 0) {
            k += j;
            i -=j;
            if(i <= 0) {
                break;
            }
        } else {
            if(-ENXIO == j) {
                return MT_UNF_SCI_ERR_CARD_NOT_READY;
            }
            if(i == (int)u32ReceiveLen) {
                Ret = MT_FAILURE;
            }
            break;
        }
    }

    *pu32ActLen = (mt_u32)k;

    return Ret;
}

/*******************************************
Function:              MT_UNF_SCI_Transfer
Description:  Transfer data from card
Calls:        MT_SCI_Transfer
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_transfer (MT_UNF_SCI_PORT_E enSciPort, mt_u8 *pCmd, mt_u32 CmdLen, mt_u8 *pResponse,mt_u32 *rlen)
{
    mt_s32 Ret = MT_SUCCESS;
    unsigned short u16ActualRead = 0;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (!pCmd) {
        MT_ERR_SCI("para pCmd is null.\n");
        return MT_ERR_SCI_NULL_PTR;
    }

    if (!pResponse) {
        MT_ERR_SCI("para pResponse is null.\n");
        return MT_ERR_SCI_NULL_PTR;
    }

    if (!rlen) {
        MT_ERR_SCI("para rlen is null.\n");
        return MT_ERR_SCI_NULL_PTR;
    }

    if ((CmdLen > 512) || (0 == CmdLen)) {
        MT_ERR_SCI("para u32ReceiveLen is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    Ret = ICC_Async_CardWrite(&reader[enSciPort], pCmd, (unsigned short)CmdLen, pResponse, &u16ActualRead);
    *rlen = u16ActualRead;

    return Ret;
}


/*******************************************
Function:              MT_UNF_SCI_ConfigVccEn
Description:  set the valid level of Vcc
Calls:        MT_SCI_ConfigVccEn
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_configvccen(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_LEVEL_E enSciVcc)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (enSciVcc >= MT_UNF_SCI_LEVEL_BUTT) {
        MT_ERR_SCI("para enSciVcc is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    attr.vcc_en_level = enSciVcc;
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;

}

/*******************************************
Function:              mt_unf_sci_configdetect
Description:  set the valid level of detect
Calls:        MT_SCI_ConfigDetect
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_sci_configdetect(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_LEVEL_E enSciDetect)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (enSciDetect >= MT_UNF_SCI_LEVEL_BUTT) {
        MT_ERR_SCI("para enSciDetect is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }
    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    attr.slot_type =
        (enSciDetect == MT_UNF_SCI_LEVEL_LOW ? SCI_SLOT_ALWAYS_CLOSE: SCI_SLOT_ALWAYS_OPEN);
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;
}

/*******************************************
Function		:MT_UNF_SCI_ConfigClkMode
Description		:config clk work mode(od or cmos)
Calls			:MT_SCI_ConfigClkMode
Data Accessed	:NA
Data Updated	:NA
Input			:NA
Output			:NA
Return			:ErrorCode(reference to document)
Others			:NA
*******************************************/
mt_s32 mt_unf_sci_configclkmode(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_MODE_E enClkMode)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (enClkMode >= MT_UNF_SCI_MODE_BUTT) {
        MT_ERR_SCI("para enClkMode is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }
    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    attr.clkpin_mode = enClkMode;
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;
}

mt_s32 mt_unf_sci_configresetmode(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_MODE_E enResetMode)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (enResetMode >= MT_UNF_SCI_MODE_BUTT) {
        MT_ERR_SCI("para enResetMode is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }
    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    attr.rstpin_mode = enResetMode;
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;
}


mt_s32 mt_unf_sci_configvccenmode(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_MODE_E enVccEnMode)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (enVccEnMode >= MT_UNF_SCI_MODE_BUTT) {
        MT_ERR_SCI("para enVccEnMode is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    attr.iopin_mode = enVccEnMode;
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;

}

mt_s32 mt_unf_sci_configtype(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_TYPE_E cardtype)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (cardtype > MT_UNF_SCI_INVERSE_CARD) {
        MT_ERR_SCI("para cardtype is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    attr.type = cardtype | 0x80;
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;

}


/*******************************************
Function		:MT_UNF_SCI_SetEtuFactor
Description	:set work baudrate
Calls			:MT_UNF_SCI_SetEtuFactor
Data Accessed	:NA
Data Updated	:NA
Input		:NA
Output		:NA
Return		:ErrorCode(reference to document)
Others		:NA
*******************************************/
mt_s32 mt_unf_sci_setetufactor(MT_UNF_SCI_PORT_E enSciPort, mt_u32 u32ClkFactor, mt_u32 u32BaudFactor)

{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if ((u32ClkFactor < 372) || (u32ClkFactor > 2048)) {
        MT_ERR_SCI("para u32ClkRate is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if ((u32BaudFactor < 1) || (u32BaudFactor > 32) || ((1 != u32BaudFactor) && (u32BaudFactor % 2) != 0)) {
        MT_ERR_SCI("para u32BitRate is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }


    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    attr.etu = u32ClkFactor / u32BaudFactor;
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;
}

/*******************************************
Function		:mt_unf_sci_setguardtime
Description	:set guard delay time
Calls			:mt_unf_sci_setguardtime
Data Accessed	:NA
Data Updated	:NA
Input		:NA
Output		:NA
Return		:ErrorCode(reference to document)
Others		:NA
*******************************************/
mt_s32 mt_unf_sci_setguardtime(MT_UNF_SCI_PORT_E enSciPort, mt_u32 u32GuardTime)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (u32GuardTime > 254) {
        MT_ERR_SCI("para u32GuardTime is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);

    if(reader[enSciPort].protocol_type == ATR_PROTOCOL_TYPE_T1) {
        if(u32GuardTime <= 11) {
            u32GuardTime = 0;
        } else {
            u32GuardTime = u32GuardTime -11;
        }
    } else {
        if(u32GuardTime <= 12) {
            u32GuardTime = 0;
        } else {
            u32GuardTime = u32GuardTime -12;
        }
    }

    attr.N = (mt_u8)u32GuardTime;
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;
}


/*******************************************
Function		:mt_unf_sci_setblktimeout
Description	:set block wait time
Calls			:mt_unf_sci_setblktimeout
Data Accessed	:NA
Data Updated	:NA
Input		:NA
Output		:NA
Return		:ErrorCode(reference to document)
Others		:NA
*******************************************/
mt_s32 mt_unf_sci_setblktimeout(MT_UNF_SCI_PORT_E enSciPort, mt_u32 u32BlkTime)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);

    if(u32BlkTime == 0) {
        attr.blktime_en= 0;
    } else {
        attr.blktime_en = 1;
    }
    attr.blk_timeout= u32BlkTime*372;
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;
}

/*******************************************
Function		:mt_unf_sci_setrsttimeout
Description	:set atr wait time
Calls			:mt_unf_sci_setrsttimeout
Data Accessed	:NA
Data Updated	:NA
Input		:NA
Output		:NA
Return		:ErrorCode(reference to document)
Others		:According to 7816.u32RstTime_Clocks should be 400~40000
*******************************************/
mt_s32 mt_unf_sci_setrsttimeout(MT_UNF_SCI_PORT_E enSciPort, mt_u32 u32RstTime_Clocks)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    if(0 == u32RstTime_Clocks) {
        attr.rsttime_en = 0;
    } else {
        attr.rsttime_en = 1;
    }
    attr.rst_timeout= u32RstTime_Clocks;   //clocks
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;
}

/*******************************************
Function		:mt_unf_sci_setrecetimeout
Description	:set rece wait time
Calls			:mt_unf_sci_setrecetimeout
Data Accessed	:NA
Data Updated	:NA
Input		:NA
Output		:NA
Return		:ErrorCode(reference to document)
Others		:NA
*******************************************/
mt_s32 mt_unf_sci_setrecetimeout(MT_UNF_SCI_PORT_E enSciPort, mt_u32 u32ReceTime)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);

    if(u32ReceTime == 0) {
        attr.recetime_en = 0;
    } else {
        attr.recetime_en = 1;
    }
    attr.rece_timeout= u32ReceTime*372;
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;
}

/*******************************************
Function		:mt_unf_sci_recev_completed
Description	:discard recvtimeout int
Calls			:mt_unf_sci_recev_completed
Data Accessed	:NA
Data Updated	:NA
Input		:NA
Output		:NA
Return		:ErrorCode(reference to document)
Others		:NA
*******************************************/
mt_s32 mt_unf_sci_recev_completed(MT_UNF_SCI_PORT_E enSciPort)
{
    ulong tmp=1;  //reservd

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    ioctl(reader[enSciPort].handle, SCI_IOC_RECIEVE_COMPLETED, &tmp);

    return MT_SUCCESS;
}
/*******************************************
Function		:mt_unf_sci_config_overload
Description	:config overload
Calls			:mt_unf_sci_config_overload
Data Accessed	:NA
Data Updated	:NA
Input		:overload==0:close;overload==1:open;overload==2:release;
Output		:NA
Return		:ErrorCode(reference to document)
Others		:NA
*******************************************/
mt_s32 mt_unf_sci_config_overload(MT_UNF_SCI_PORT_E enSciPort,ulong overload)
{
    if (enSciPort >= MT_UNF_SCI_PORT_BUTT)
    {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if(reader[enSciPort].handle <= 0){
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }

    return ioctl(reader[enSciPort].handle, SCI_IOC_OVERLOAD, &overload);
}

/*******************************************
Function		:MT_UNF_SCI_NegotiatePPS
Description	:request PPS negotiation
Calls			:MT_UNF_SCI_NegotiatePPS
Data Accessed	:NA
Data Updated	:NA
Input		:NA
Output		:NA
Return		:ErrorCode(reference to document)
Others		:NA
*******************************************/
#define PPS_MAX_LENGTH  6
#define PPS_HAS_PPS1(block)       ((block[1] & 0x10) == 0x10)
#define PPS_HAS_PPS2(block)       ((block[1] & 0x20) == 0x20)
#define PPS_HAS_PPS3(block)       ((block[1] & 0x40) == 0x40)

static uint32_t PPS_GetLength(unsigned char *block)
{
    uint32_t length = 3;

    if(PPS_HAS_PPS1(block)) {
        length++;
    }

    if(PPS_HAS_PPS2(block)) {
        length++;
    }

    if(PPS_HAS_PPS3(block)) {
        length++;
    }

    return length;
}

mt_s32 mt_unf_sci_negotiatepps(MT_UNF_SCI_PORT_E enSciPort, mt_u8 *pSciSendBuf, mt_u32 Sendlen, mt_u32 RecTimeouts)
{
    mt_u8 request[PPS_MAX_LENGTH] = {0};
    mt_u8 response[PPS_MAX_LENGTH] = {0};
    mt_u32 request_len = 0;
    mt_u32 response_len = 0;
    mt_u32 confirm_len = 0;
    mt_s32 ret = 0;

    memcpy(request, pSciSendBuf, PPS_MAX_LENGTH);
    /* PCK : check byte-bitwise XOR of PPSS,PPS0,and PPS1 */
    request[3] = (request[0] ^ request[1] ^ request[2]);
    ret = mt_unf_sci_send(enSciPort, request, 4, &request_len, RecTimeouts);
    if (ret != MT_SUCCESS) {
        MT_ERR_SCI("PPS request send is fail.\n");
        return ret;
    }

    ret = mt_unf_sci_receive(enSciPort, response, 2, &response_len, RecTimeouts);
    if (ret != MT_SUCCESS) {
        MT_ERR_SCI("Recieve PPS response1#1 is fail.\n");
        return ret;
    }
    if(2 == response_len)
        ;
    else {
        MT_ERR_SCI("Recieve PPS response1#2 is fail.\n");
        return MT_FAILURE;
    }

    confirm_len = PPS_GetLength(response);

    ret = mt_unf_sci_receive(enSciPort, &response[2], confirm_len - 2, &response_len, RecTimeouts);
    mt_unf_sci_recev_completed(enSciPort);

    if (ret != MT_SUCCESS) {
        MT_ERR_SCI("Recieve PPS response2#1 is fail.\n");
        return ret;
    }
    if((confirm_len - 2) != response_len) {
        MT_ERR_SCI("Recieve PPS response2#2 is fail.\n");
        return MT_FAILURE;
    } else if(memcmp(request, response, confirm_len)) {
        return 0xdef;
    }
    return MT_SUCCESS;
}


/*******************************************
Function		:MT_UNF_SCI_SetTxRetries
Description	:set Tx retry times
Calls			:MT_UNF_SCI_SetTxRetries
Data Accessed	:NA
Data Updated	:NA
Input		:NA
Output		:NA
Return		:ErrorCode(reference to document)
Others		:NA
*******************************************/
mt_s32 mt_unf_sci_settxretries(MT_UNF_SCI_PORT_E enSciPort, mt_u32 TxRetryTimes)
{
    SCI_ATTR_S attr;

    if (enSciPort >= MT_UNF_SCI_PORT_BUTT) {
        MT_ERR_SCI("para enSciPort is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }

    if (TxRetryTimes > 7) {
        MT_ERR_SCI("para TxRetryTimes is invalid.\n");
        return MT_ERR_SCI_INVALID_PARA;
    }
    if(reader[enSciPort].handle <= 0) {
        return MT_UNF_SCI_ERR_OPEN_FAIL;
    }
    attr.dev_id = enSciPort;
    ioctl(reader[enSciPort].handle, SCI_IOC_GET_ATTR, &attr);
    attr.tx_retrys = (mt_u8)TxRetryTimes;
    ioctl(reader[enSciPort].handle, SCI_IOC_SET_ATTR, &attr);
    return MT_SUCCESS;
}

