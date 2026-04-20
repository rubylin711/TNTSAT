/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*
        ifd_phoenix.c
        This module provides IFD handling functions for Smartmouse/Phoenix reader.
*/

#include "globals.h"
#include "icc_async.h"
#include "atr.h"
#include "drv_sci_ioctl.h"
#define ERROR 1
#define OK 0

static int32_t _m88cc6000_read(struct s_reader * reader, u_char *buf, uint32_t len)
{
    int i = 0,j = 0, k = 0;
    SCI_DATA_S data;

    data.dev_id = reader->dev_id;
    i = (int)len;
    while(1) {
        data.data_len =(unsigned long) i;
        data.data_buf = (ulong)(&buf[k]);
        j = ioctl(reader->handle, SCI_IOC_RECIEVE_DATA, &data);
        if(j > 0) {
            k += j;
            i -=j;
            if(i <= 0) {
                break;
            }
        } else {
            break;
        }
    }

    return k;
}
static int32_t m88cc6000_reader_init(struct s_reader * reader)
{
    MT_UNF_SCI_PORT_E port = 0;

    if(reader->handle < 0) {
        return ERROR;
    }

    port = reader->dev_id;
    /* Init smc first */
    ioctl(reader->handle, SCI_IOC_INIT, &port);
    return OK;
}

static int32_t m88cc6000_get_status(struct s_reader * reader, int *cardin)
{
    SCI_STATUS_S status;

    if(reader->handle < 0) {
        return ERROR;
    }
    status.dev_id = reader->dev_id;
    ioctl(reader->handle, SCI_IOC_GET_STATUS, &status);
    *cardin = status.status;

    return OK;
}

#define SMC_MAX_ATR 32

static int32_t m88cc6000_activate(struct s_reader *reader, struct s_ATR *atr)
{
    SCI_STATUS_S status;
    MT_UNF_SCI_PORT_E port = 0;
    u_char atrarr[64]= {0};
    u_char tmpatrarr[64]= {0};
    int atrlen = 0;
    int i = 0,readret = 0;
    SCI_ATTR_S attr;

    if(reader->handle < 0) {
        return ERROR;
    }
    status.dev_id = reader->dev_id;
    port = reader->dev_id;
    ioctl(reader->handle, SCI_IOC_GET_STATUS, &status);

    if(!status.status) {
        return ERROR;
    }
    //ATR occurs after 400 to 40000 clock when RST signal emits.

    attr.dev_id = reader->dev_id;

    ioctl(reader->handle, SCI_IOC_GET_ATTR, &attr);
    attr.N = 0;
    if(reader->protocol_type == ATR_PROTOCOL_TYPE_T14) {
        attr.etu = 620;
        attr.parity_en = 0;
    } else {
        attr.etu = 372;
    }
    attr.read_timeout = (2 * 40001 * 100) / reader->mhz;
    attr.rsttime_en = 1;
    //printf("++rst..timeout=%x\n",(u32)attr.rst_timeout);   //use timeout from unf or default
    attr.recetime_en = 1;
    attr.rece_timeout = (9601 * attr.etu);  //9600*etu
    ioctl(reader->handle, SCI_IOC_SET_ATTR, &attr);

    ioctl(reader->handle, SCI_IOC_RESET, &port);
    atrlen = _m88cc6000_read(reader, &atrarr[0], 1);

    attr.read_timeout = (2 * 9601 * attr.etu * 100) /reader->mhz;
    //attr.rece_timeout = (9601 * attr.etu);  //9600*etu
    ioctl(reader->handle, SCI_IOC_SET_ATTR, &attr);
    if ((atrlen > 0) && ((0x3B == atrarr[0]) ||(0x3F == atrarr[0]))) {
        for(i = 1; i<SMC_MAX_ATR; i++) {
            readret = _m88cc6000_read(reader, &atrarr[i], 1);
            if(0 == readret) {
                break;
            }
            atrlen+=1;
        }
    }
    attr.rsttime_en = 0;        //a flag for pair
    //attr.rst_timeout = 0;
    attr.recetime_en = 0;
    attr.rece_timeout = 0;
    ioctl(reader->handle, SCI_IOC_SET_ATTR, &attr);

    if(1 >= atrlen) {
        ioctl(reader->handle, SCI_IOC_DEACTIVATE, &port);
        return ERROR;
    }
#if 0
    ioctl(reader->handle, SCI_IOC_GETREG_STATUS, &reg);
    if((reg >> 5) & 0x03) {
        printf("%s %d parity error!reg = 0x%x\n",__FUNCTION__,__LINE__,reg);
        ioctl(reader->handle, SCI_IOC_DEACTIVATE, &port);
        return ERROR;
    }
#endif
    if((unsigned int)atrlen > 64) {
        atrlen = 64;
    }
    memcpy(reader->card_atr, atrarr, (unsigned int)atrlen);
    reader->card_atr_length = (char)atrlen;
    memcpy(tmpatrarr,atrarr, (unsigned int)atrlen);
    if(0 != ATR_InitFromArray(atr, tmpatrarr, atrlen)) {
        return ERROR;
    }
    return OK;
}

static int32_t m88cc6000_transmit(struct s_reader *reader, unsigned char *sent, uint32_t size,
                                  uint32_t expectedlen, uint32_t delay, uint32_t timeout)
{
    SCI_STATUS_S status;
    SCI_DATA_S data;

    if(reader->handle < 0) {
        return ERROR;
    }
    status.dev_id = reader->dev_id;
    ioctl(reader->handle, SCI_IOC_GET_STATUS, &status);
    if(!status.status) {
        return ERROR;
    }
    data.dev_id = reader->dev_id;
    data.data_buf = (ulong)sent;
    data.data_len = size;
    ioctl(reader->handle, SCI_IOC_SEND_DATA, &data);

    return OK;
}

static int32_t m88cc6000_receive(struct s_reader *reader, unsigned char *data, uint32_t size,
                                 uint32_t delay, uint32_t timeout)
{
    SCI_STATUS_S status;
    SCI_ATTR_S attr;
    SCI_DATA_S scidata;

    int i = 0, j = 0, k = 0;

    if(reader->handle < 0) {
        return ERROR;
    }
    status.dev_id = reader->dev_id;
    ioctl(reader->handle, SCI_IOC_GET_STATUS, &status);
    if(!status.status) {
        return ERROR;
    }
    attr.dev_id = reader->dev_id;
    ioctl(reader->handle, SCI_IOC_GET_ATTR, &attr);
    attr.read_timeout = timeout;
    ioctl(reader->handle, SCI_IOC_SET_ATTR, &attr);

    scidata.dev_id = reader->dev_id;
    i = (int)size;
    while(1) {
        scidata.data_len = (unsigned long)i;
        scidata.data_buf = (ulong)(&data[k]);
        j = ioctl(reader->handle, SCI_IOC_RECIEVE_DATA, &scidata);
        if(j > 0) {
            k += j;
            i -=j;
            if(i <= 0) {
                break;
            }
        } else {
            break;
        }
    }

    if(k != (int)size) {
        return ERROR;
    } else {
        return OK;
    }
}

static int32_t m88cc6000_close(struct s_reader *reader)
{
    SCI_STATUS_S status;
    MT_UNF_SCI_PORT_E port = 0;

    if(reader->handle < 0) {
        return ERROR;
    }
    status.dev_id = reader->dev_id;
    port = reader->dev_id;
    ioctl(reader->handle, SCI_IOC_GET_STATUS, &status);
    //printf("%s %d deactive card!\n",__FUNCTION__,__LINE__);
    if(status.status) {
        ioctl(reader->handle, SCI_IOC_DEACTIVATE, &port);
    }
    return OK;

}

static int32_t m88cc6000_write_settings(struct s_reader *reader, uint32_t ETU, unsigned char N)
{
    SCI_ATTR_S attr;

    if(reader->handle < 0) {
        return ERROR;
    }
    attr.dev_id = reader->dev_id;
    ioctl(reader->handle, SCI_IOC_GET_ATTR, &attr);
    if(ETU != 0) {
        attr.etu = ETU;
    }
    attr.N = N;
    ioctl(reader->handle, SCI_IOC_SET_ATTR, &attr);

    return OK;
}

void cardreader_m88cc6000(struct s_cardreader *crdr)
{
    crdr->reader_init = m88cc6000_reader_init;
    crdr->get_status = m88cc6000_get_status;
    crdr->activate = m88cc6000_activate;
    crdr->transmit = m88cc6000_transmit;
    crdr->receive = m88cc6000_receive;
    crdr->close = m88cc6000_close;
    crdr->write_settings = m88cc6000_write_settings;
}

