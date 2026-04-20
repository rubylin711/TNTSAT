/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2020 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/printk.h>

#include <linux/pci.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

//#include <net/netlink.h>
//#include <linux/security.h>
//#include <net/net_namespace.h>
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

//#include <net/sock.h>
//#include <net/genetlink.h>

#include "mt_type.h" //flax

//#include "mt_drv_mmz.h"

//#include <drv_frontend.h>
#include "mt_unf_frontend.h"

//#include "mt_fe_common.h"
#include "port_hd2502.h"
#include "drv_frontend_ioctl.h"

#include "mt_module_debug.h"

#include "mt_mach/chipinfo.h"

#include "drv_gpio_ioctl.h"


//#define FOR_PORT_HD2502_CONNECT_SYCHRONOUS



#define MT_FE_HD2502_PIN_LEVEL_LOW 0
#define MT_FE_HD2502_PIN_LEVEL_HIGH 1
//#define MT_FE_HD2502_DISEQC_COMMAND_START_DELAY 30
#define MT_FE_HD2502_DISEQC_COMMAND_START_DELAY 50
#define MT_FE_HD2502_DISEQC_COMMAND_END_DELAY 50


//static MT_FE_LOCK_STATE ss2_status = MtFeLockState_Undef;

//mt_fe_hd2502_priv_handle p_priv = NULL;
static mt_u8 g_dev_init_flag = 0;
static mt_u8 g_priv_count = 0;


mt_fe_hd2502_priv_handle g_hd2502_priv = NULL;
int g_i2c_hd2502 = 0;

mt_unf_fe_channel_info_t *hd2502_pg_channel_info = NULL;

extern UINT8 hd2502_tuner_type;

static int _mt_fe_i2c_write_hd2502(U8 dev_addr, U8 *w_buf, U16 w_byte)
{
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int ret = 0;

	i2c = i2c_get_adapter(g_i2c_hd2502);
	msg->addr = dev_addr;
	msg->flags = I2C_M_TEN | I2C_M_SALVE_TYPE;
	msg->buf = w_buf;
	msg->len = w_byte;
	mt_msg.rlen = 0;
	mt_msg.wlen = w_byte;
	mt_msg.slave_type = I2C_SLAVE_DEV_SOC_EXTER;
	ret = i2c_transfer(i2c, msg, 1);
	if (ret < 0)
	{
		printk("_mt_fe_i2c_write_hd2502() failed, dev_addr = 0x%02x, ret %d\n", dev_addr, ret);
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

static int _mt_fe_i2c_read_hd2502(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
{
	// 8 bit Register Read Protocol:
	// +------+-+-----+-+-+----------+-+
	// |MASTER|S|SADDR|W|  |RegAddr   |
	// +------+-+-----+-+-+-----------+-+
	// |SLAVE |                          |A|               |A| |
	// +------+-+-----+-+-+-----------+-+
	// +------+-+-----+-+-+-----+--+-+
	// |MASTER|S|SADDR|R| |     |MN|P|
	// +------+-+-----+-+-+-----+--+-+
	// |SLAVE |         |A|Data |  | |
	// +------+---------+-+-----+--+-+
	// Legends: SADDR(I2c slave address), S(Start condition), MA(Master Ack), MN(Master NACK),
	// P(Stop condition)
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int ret = 0;
	U8 buffer[256];

	//*r_buf = *w_buf;
	memcpy(buffer, w_buf, w_byte);

	i2c = i2c_get_adapter(g_i2c_hd2502);
	//printk("[%s ] line:%d i2c_bus 0x%08x\n", __func__, __LINE__, i2c);
	msg->addr = dev_addr;
	msg->flags = I2C_M_STD_RD | I2C_M_SALVE_TYPE;
	//msg->buf = r_buf;
	msg->buf = buffer;
	msg->len = r_byte;
	mt_msg.rlen = r_byte;
	mt_msg.wlen = w_byte;
	mt_msg.slave_type = I2C_SLAVE_DEV_SOC_EXTER;
	ret = i2c_transfer(i2c, msg, 1);
	if (ret < 0)
	{
		printk("_mt_fe_i2c_read_hd2502() failed, dev_addr = 0x%02x, ret %d\n", dev_addr, ret);
		return MT_FAILURE;
	}

	memcpy(r_buf, buffer, r_byte);

	return MT_SUCCESS;
}


void _mt_sleep_hd2502(U32 ms)
{
	/*
		TODO:
			Delay ms.
	*/

	if (ms < 10)
	{
		usleep_range(ms * 1000, ms * 1000 + 500);
	}
	else
	{
		msleep(ms);
	}
}

void _mt_delayus_hd2502(U32 us)
{
	usleep_range(us / 100 * 100 + 50, us / 100 * 100 + 100);
}

UINT8 _mt_fe_write32_hd2502(U32 reg_addr, U32 reg_data)
{
	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(reg_addr), reg_data);

	return MT_SUCCESS;
}

int _mt_fe_read32_hd2502(U32 reg_addr, U32 *p_data)
{
	*p_data = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(reg_addr));

	return MT_SUCCESS;
}

void HD_Delay( UINT32 uiMS)
{
	if (uiMS < 10)
	{
		usleep_range(uiMS * 1000, uiMS * 1000 + 500);
	}
	else
	{
		msleep(uiMS);
	}
}

UINT8 Write_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer)
{
	//UINT32 i = 0;
	UINT8 ret = 0;

	if (length > 256)
		return HDIC_I2C_TRANSFER_ERROR;

	ret = _mt_fe_i2c_write_hd2502(device_address, value_buffer, length);

	if (ret != MT_SUCCESS)
		printk("%s[%d] ---- I2C error! dev_addr[0x%02x], length[%d]\n", __FUNCTION__, __LINE__, device_address, length);

	return (ret == MT_SUCCESS) ? HDIC_NO_ERROR : HDIC_I2C_TRANSFER_ERROR;
}

UINT8 Read_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer)
{
	//UINT32 i = 0;
	s32 ret = 0;

	U8 data[2];

	ret = _mt_fe_i2c_read_hd2502(device_address, data, 0, value_buffer, length);
       //printk("[%s : %d] ---- ret=%d, value_buffer[0x%02x], length[%d]\n", __FUNCTION__, __LINE__, ret, value_buffer[0], length);
	if (ret != MT_SUCCESS)
		printk("%s[%d] ---- I2C error! value_buffer[0x%02x], length[%d]\n", __FUNCTION__, __LINE__, value_buffer[0], length);

	return (ret == MT_SUCCESS) ? HDIC_NO_ERROR : HDIC_I2C_TRANSFER_ERROR;
}


// ----------------------------------------------------------------------------------------

#define SAT_C_MIN_KHZ (3000000)
#define SAT_C_MAX_KHZ (4200000)
#define SAT_KU_MIN_KHZ (10600000)
#define SAT_KU_MAX_KHZ (12750000)
#define SAT_DOWNLINK_FREQ_KU_MID (11700)
#define SAT_SYMBOLRATE_MAX (60000000) //(45000000)

static int port_hd2502_channel_set(void *handle, mt_unf_fe_connect_para_t *para)
{
	//MT_U32 for_scan = 0;
	UINT8 ret = 0;

	mt_unf_fe_channel_info_t *p_channel_info = NULL;
	mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;

	U32 freq_MHz = 1000;
	U32 symbol_rate_KSs = 27500;

	p_channel_info = &(para->channel_info);
	para->channel_info.lock = 0;

	//printk("%s[%d]: para->sig_type = %d\n", __FUNCTION__, __LINE__, para->sig_type);

	HD2502_DEBUG_PRINTF("hd2502_channel set abs: freq = %d, sym = %d, bs = %d, type = %d, use_uc = %d, freq_MHz=%d\n",
			para->connect_param.sat.freq,
			para->connect_param.sat.sym_rate,
			p_priv->for_scan,
			para->connect_param.sat.port_type,
			para->connect_param.sat.uc_param.use_uc,
			freq_MHz);
	HD2502_DEBUG_PRINTF("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
	memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
	//priv->for_scan = p_channel_set_info->for_scan;
	HD2502_DEBUG_PRINTF("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
	//para->connect_param.sat.freq / 1000,
	//para->connect_param.sat.sym_rate,
	//dvb_type,
	//p_priv->for_scan
	freq_MHz = (para->connect_param.sat.freq + 500) / 1000;
	symbol_rate_KSs = para->connect_param.sat.sym_rate;
	HD2502_DEBUG_PRINTF("[ %s : %d ]====== freq_MHz=%d \n",__FUNCTION__,__LINE__,freq_MHz);
	HDIC2501_SetTuner(freq_MHz);
	HD2502_DEBUG_PRINTF("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
	HDIC2501_SetParameters(SYMBOLRATEAUTODETECT_MANUAL, symbol_rate_KSs * 1000);
	HD2502_DEBUG_PRINTF("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
	HDIC2501_SoftReset();

	HD2502_DEBUG_PRINTF("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
	if (1)//(ret == 0)
	{
		/* set check lock delay time */
		if (para->connect_param.sat.sym_rate >= 2000)
		{
			para->channel_set_info.lock_time = 2000; //2s
		}
		else
		{
			para->channel_set_info.lock_time = 4000; //4s
		}
		HD2502_DEBUG_PRINTF("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
		HD2502_DEBUG_PRINTF("[%s] line:%d lock_time %d\n", __func__, __LINE__, para->channel_set_info.lock_time);

		return MT_SUCCESS;
	}
	else
	{
		printk("TP set failed %d\n", ret);

		para->channel_set_info.lock_time = 2000; //2s

		return MT_FAILURE;
	}
	HD2502_DEBUG_PRINTF("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
	return MT_FAILURE;
}

#if 0
static void port_hd2502_set_22k_onoff(void *handle, MT_U8 onoff_22k, MT_U8 diseqc_out_when_lnb_off)
{
	//U8 val_0xa1, val_0xa2;
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;


	printk("port_hd2502_set_22k_onoff %d - %s\n", onoff_22k, (onoff_22k == 1) ? "22K On" : "22K Off");
}
#endif

// voltage: 0 - 13V; 1 - 18V
static void port_hd2502_set_lnb_voltage(void *handle, MT_U8 voltage)
{
	//MT_U32 temp=0;
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;

	HD2502_DEBUG_PRINTF("port_hd2502_set_lnb_voltage %d - %dV\n", voltage, (voltage == 0) ? 13 : 18);
/*
	BF50F818[25]=0，BF13C0F8[3:0]=1
	GPIO_Mask:0xBF0A001C[30]=1，Write_EN:0xBF0A0014[30]=0，Set data 0xBF0A0010[30]=0 or 1
*/
	/*_mt_fe_read32_hd2502(0xBF50F818, &temp);
	temp &= 0xfdffffff;
	_mt_fe_write32_hd2502(0xBF50F818, temp);

	_mt_fe_read32_hd2502(0xBF13C0F8, &temp);
	temp |= 0x01;
	_mt_fe_write32_hd2502(0xBF13C0F8, temp);

	_mt_fe_read32_hd2502(0xBF0A001C, &temp);
	temp |= 0x40000000;
	_mt_fe_write32_hd2502(0xBF0A001C, temp);


	_mt_fe_read32_hd2502(0xBF0A0014, &temp);
	temp &= 0xbfffffff;
	_mt_fe_write32_hd2502(0xBF0A0014, temp);

	_mt_fe_read32_hd2502(0xBF0A0010, &temp);
	temp |= 0x40000000;
	_mt_fe_write32_hd2502(0xBF0A0010, temp);*/	

	HDIC2501_SetPolar(voltage);

	return;
}

static void port_hd2502_set_lnb_onoff(void *handle, MT_U8 lnb_enable, mt_unf_fe_pin_config_para_t *pin_config)
{
	//U8 val_0xa1, val_0xa2; //, pin, level, value;
	mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;

	//pinmux_configure();

	HD2502_DEBUG_PRINTF("port_hd2502_set_lnb_onoff %d - %s\n", lnb_enable, (lnb_enable == 1) ? "On" : "Off");
	if(lnb_enable == 1)
		drv_gpio_set_value(p_priv->lnb_onoff_gpio,GPIO_VALUE_HIGH_LEVEL);
	else if(lnb_enable == 0)
		drv_gpio_set_value(p_priv->lnb_onoff_gpio,GPIO_VALUE_LOW_LEVEL);

	return;
}

static int port_hd2502_diseqc_sendmsg(void *handle, mt_unf_fe_diseqc_sendmsg_t *p_diseqc_sendmsg)
{
	UINT8 ret = HDIC_NO_ERROR;
	//UINT8 rx_buf[10];
	//UINT8 rx_len;

	mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;

	//return MT_SUCCESS;

	if (p_diseqc_sendmsg->len == 0) // || p_diseqc_sendmsg->p_tx_buf == NULL)
	{
		return MT_FAILURE;
	}

	if (p_diseqc_sendmsg->len > 8)
	{
		printk("ERROR:p_diseqc_sendmsg->len > 8\n");
		return MT_FAILURE;
	}

	memcpy(p_priv->cur_diseqc.p_tx_buf, p_diseqc_sendmsg->data, p_diseqc_sendmsg->len);

	p_priv->cur_diseqc.mode = PORT_DISEQC_BYTES; //p_diseqc_cmd->mode;
	p_priv->cur_diseqc.tx_len = p_diseqc_sendmsg->len;

	if (p_priv->lnb_onoff == 0)
	{
		return MT_SUCCESS;
	}

	HD2502_DEBUG_PRINTF("DiSEqC send msg[0x%x, 0x%x, 0x%x, 0x%x], send size[%d]\n",
			p_diseqc_sendmsg->data[0],
			p_diseqc_sendmsg->data[1],
			p_diseqc_sendmsg->data[2],
			p_diseqc_sendmsg->data[3],
			p_diseqc_sendmsg->len);


#if 0
	if (rx_len <= 10)
		memcpy(p_priv->cur_diseqc.p_rx_buf, rx_buf, rx_len);
#endif

	return (ret == HDIC_NO_ERROR) ? MT_SUCCESS : MT_FAILURE;
}

static int port_hd2502_diseqc_recvmsg(void *handle, mt_unf_fe_diseqc_recvmsg_t *p_diseqc_recvmsg)
{
	UINT8 ret = HDIC_NO_ERROR;
	UINT8 rx_buf[10];
	UINT8 rx_len;

	mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;

	//return MT_SUCCESS;

	if (p_diseqc_recvmsg->len == 0) // || p_diseqc_sendmsg->p_tx_buf == NULL)
	{
		return MT_FAILURE;
	}

	if (p_diseqc_recvmsg->len > 8)
	{
		printk("ERROR:p_diseqc_sendmsg->len > 8\n");
		return MT_FAILURE;
	}

	rx_len = p_diseqc_recvmsg->len;

	memcpy(p_priv->cur_diseqc.p_tx_buf, p_diseqc_recvmsg->msg, p_diseqc_recvmsg->len);

	p_priv->cur_diseqc.mode = PORT_DISEQC_BYTES; //p_diseqc_cmd->mode;
	p_priv->cur_diseqc.tx_len = p_diseqc_recvmsg->len;

	if (p_priv->lnb_onoff == 0)
	{
		return MT_SUCCESS;
	}

	HD2502_DEBUG_PRINTF("DiSEqC send msg[0x%x, 0x%x, 0x%x, 0x%x], send size[%d]\n",
			p_diseqc_recvmsg->msg[0],
			p_diseqc_recvmsg->msg[1],
			p_diseqc_recvmsg->msg[2],
			p_diseqc_recvmsg->msg[3],
			p_diseqc_recvmsg->len);



	if (rx_len <= 10)
		memcpy(p_priv->cur_diseqc.p_rx_buf, rx_buf, rx_len);

	HD2502_DEBUG_PRINTF("DiSEqC recv msg[0x%x, 0x%x, 0x%x, 0x%x], send received[%d]\n",
			p_diseqc_recvmsg->msg[0],
			p_diseqc_recvmsg->msg[1],
			p_diseqc_recvmsg->msg[2],
			p_diseqc_recvmsg->msg[3],
			p_diseqc_recvmsg->len);

	return (ret == HDIC_NO_ERROR) ? MT_SUCCESS : MT_FAILURE;
}

static int port_hd2502_get_status(void *handle, mt_unf_fe_status_t *p_status)
{
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;
	//UINT8 stat = 0;
	UINT8 ret = 0;

	UINT8 locked, Status;

	//UINT16 LDPC_BER = 0;

	ret = HDIC2501_IsDemodLocked(&locked, &Status);

	if (locked == 1)
	{
		p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;
	}
	else
	{
		p_status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
	}

	//HDIC2501_GetLdpcBER(&LDPC_BER);

	//printk(KERN_ERR "\tABS-S %s, LDPC BER = %d / 4096\n", locked ? "locked" : "unlock", LDPC_BER);

	return MT_SUCCESS;
}

static int port_hd2502_get_signal_quality(void *handle, MT_U32 *p_quality)
{
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;
#if 0
	UINT8 ret = 0;
	UINT8 percent = 0;

	UINT32 _snr = 0;

	ret = HDIC2501_GetSignalSNR(&_snr);

	percent = _snr * 5 / 1000;

	if (percent < 0)
		percent = 0;

	if (percent > 100)
		percent = 100;

	if (ret != HDIC_NO_ERROR)
	{
		return MT_FAILURE;
	}

	*p_quality = percent;
 #else
    /*fix RedMine issue 26822*/
    UINT8 locked = 0,status = 0,i = 0;
    UINT32 snr = 0,snr_total = 0,percent = 0;
    UINT8 ret = 0;
     
    HDIC2501_IsDemodLocked(&locked, &status);
    if(locked == 0x01)
    {/*locked*/
        for(i=0;i<3;i++)
        {
            ret = HDIC2501_GetSignalSNR(&snr);

    	    if (ret != HDIC_NO_ERROR)
    	    {
    		    return MT_FAILURE;
    	    }
            snr_total += snr;
            _mt_sleep_hd2502(10);
        }
        percent = snr_total/1000 +30;
        if(percent >= 100)
            percent = 98;
        *p_quality = percent;
    }
    else
    {
    	ret = HDIC2501_GetSignalSNR(&snr);

    	if (ret != HDIC_NO_ERROR)
    	{
    		return MT_FAILURE;
    	}
        *p_quality = snr / 1000;    
    }
    
 #endif

	return MT_SUCCESS;
}

static int port_hd2502_get_ber(void *handle, MT_U32 *p_ber)
{
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;
	UINT8 ret = 0;
	UINT32 ber = 0;

	ret = HDIC2501_GetSignalBER(&ber);

	p_ber[0] = 1000000;
	p_ber[1] = ber;
	p_ber[2] = 0;

	return MT_SUCCESS;
}

static int port_hd2502_get_snr(void *handle, MT_U32 *p_snr)
{
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;
	UINT32 snr = 0;
	UINT8 ret = 0;

	ret = HDIC2501_GetSignalSNR(&snr);

	if (ret != HDIC_NO_ERROR)
	{
		return MT_FAILURE;
	}

	*p_snr = snr / 1000;

	return MT_SUCCESS;
}

static int port_hd2502_get_signal_strength(void *handle, MT_U32 *p_strength)
{
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;
#if 0
	UINT16 _strength = 0;
	UINT8 ret = 0;

	ret = HDIC2501_GetFieldStrength(&_strength);
	if (ret != HDIC_NO_ERROR)
	{
		return MT_FAILURE;
	}

	*p_strength = _strength / 10;
#else
    /*fix RedMine issue 26822*/
    UINT8 locked = 0,status = 0,i = 0;
    UINT32 snr = 0,snr_total = 0,percent = 0;
    UINT8 ret = 0;
   
    
    HDIC2501_IsDemodLocked(&locked, &status);
    if(locked == 0x01)
    {/*locked*/
        for(i=0;i<4;i++)
        {
            ret = HDIC2501_GetSignalSNR(&snr);

    	    if (ret != HDIC_NO_ERROR)
    	    {
    		    return MT_FAILURE;
    	    }
            snr_total += snr;
            _mt_sleep_hd2502(10);
        }
        percent = snr_total/1000 +30;
        if(percent >= 100)
            percent = 98;
        *p_strength = percent;
    }
    else
    {
    	ret = HDIC2501_GetSignalSNR(&snr);

    	if (ret != HDIC_NO_ERROR)
    	{
    		return MT_FAILURE;
    	}
        *p_strength = snr / 1000;    
    }
#endif
	return MT_SUCCESS;
}

static void port_hd2502_get_signal_info(void *handle, mt_unf_fe_signal_info_t *p_sig_info)
{
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;

	return;
}

static int port_hd2502_get_signal_agc(void *handle, mt_u32 *p_agc)
{
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;
	//UINT8 ret = 0;
	//mt_u32 tuner_gain = 0;

	*p_agc = 0;

	return MT_SUCCESS;
}

static int port_hd2502_standby(void *handle)
{
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;
	UINT8 ret = 0;


	return (ret == HDIC_NO_ERROR) ? MT_SUCCESS : MT_FAILURE;
}

static int port_hd2502_wakeup(void *handle)
{
	//mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;
	UINT8 ret = 0;


	return (ret == HDIC_NO_ERROR) ? MT_SUCCESS : MT_FAILURE;
}

static int port_hd2502_get_default_timeout(void *handle, mt_u32 *timeout)
{
	*timeout = 120;

	return MT_SUCCESS;
}

static int port_hd2502_set_io(void *handle, MT_BOOL onoff)
{
  return MT_SUCCESS;
}

static int port_hd2502_channel_connect(void *handle, mt_unf_fe_connect_para_t *para)
{
	mt_unf_fe_channel_info_t *p_channel_info = NULL;
	mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;

	//UINT8 ret = MT_SUCCESS;
	/*MT_FE_LOCK_STATE status = 0; clean warning*/
	/*MT_U32 cnt = 0; clean warning*/

	/*U8 tmp; clean warning*/
	MT_U8 voltage = 0;
	//MT_U8 count;

	p_priv->sig_type = para->sig_type;
	p_channel_info = &(para->channel_info);
	//pinmux_configure();

	//printk("----port_hd2502_channel_connect() log1, sig_type = %d\n", para->sig_type);


	HD2502_DEBUG_PRINTF("hd2502 connect: freq = %d, sym = %d, type = %d, use_uc = %d, lnb_polar=%d, polarization=%d\n",
			para->connect_param.sat.freq,
			para->connect_param.sat.sym_rate,
			para->connect_param.sat.port_type,
			para->connect_param.sat.uc_param.use_uc,
			p_priv->lnb_polar,
			para->connect_param.sat.polarization);

	if (p_priv->lnb_onoff == 0)
	{
		return MT_SUCCESS;
	}
	HD2502_DEBUG_PRINTF("----port_hd2502_channel_connect() log1, sig_type = %d\n", para->sig_type);
	//if (p_priv->lnb_polar != para->connect_param.sat.polarization)
	{
		if ((para->connect_param.sat.polarization == PORT_PORLAR_HORIZONTAL) || 
			(para->connect_param.sat.polarization == PORT_PORLAR_LEFT))
			voltage = 1;
		else
			voltage = 0;
		
		port_hd2502_set_lnb_voltage(handle, voltage);
		p_priv->lnb_polar = (MT_U8)para->connect_param.sat.polarization;
		p_priv->lnb_voltage = voltage;
	}

	memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
	para->channel_info.lock = 0;

	p_priv->for_scan = 0;
	para->channel_set_info.for_scan = 0;
	port_hd2502_channel_set(p_priv, para);

#ifdef DUMP_REG	
	if(1)   //for test
	{
		u32 value = 0;
		u8 status = 0;
		u8 locked=0;
		int i=0;
		for(i=0; i<50; i++)
		{
			value = 0;
			port_hd2502_get_signal_strength(p_priv, &value);
			printk("[%s : %d] >>>> strength=%d\n",__FUNCTION__,__LINE__,value);
			value = 0;
			port_hd2502_get_signal_quality(p_priv, &value);
			printk("[%s : %d] >>>> quality=%d\n",__FUNCTION__,__LINE__,value);
			value = 0;
			HDIC2501_IsDemodLocked(&locked, &status);			
			printk("[%s : %d] >>>> locked=%d,value=%d\n",__FUNCTION__,__LINE__,locked,status);
			_mt_sleep_hd2502(100);

		}
	}

	if(1)   //demod reg
	{
		int i=0;
		u8 ReadData1=0;
		HD2502_DEBUG_PRINTF("\n==========demod reg : 0x00 -0xff ============\n");
		for(i=0; i<0xff; i++)
		{
			HDIC2501_ReadRegister(i, &ReadData1);
			//HD2502_DEBUG_PRINTF("%02x,",ReadData1);
			printk(KERN_CONT "%02x,",ReadData1);
			if((i+1)%16 == 0)  HD2502_DEBUG_PRINTF("\n");
		}
		HD2502_DEBUG_PRINTF("\n==========demod reg : ============\n");
	}
	mt_fe_tn_ts6011_dump_reg(); 
#endif	
	return MT_SUCCESS;
}

static int port_hd2502_ioctl(void *handle, mt_u32 cmd, ulong param)
{
  //UINT8 status = 0;
  //mt_unf_fe_diseqc_cmd_t diseqc_cmd = {0};
  //mt_unf_fe_diseqc_cmd_t *p_diseqc_cmd = NULL;
  //mt_unf_fe_scan_info_t *p_scan_info = NULL;
  mt_unf_fe_diseqc_sendmsg_t *p_sendmsg;
  mt_unf_fe_diseqc_recvmsg_t *p_recvmsg;
  //fe_blindscan_param_t *p_scan_info = NULL;
  //U8 tx_buf[8] = {0};
  U8 voltage = 0;
  //U8 pin = 0;
  //U8 level = 0;
  //U8 value = 0;
  //int rc = 0;
  INT32 carrieroffset_KHz = 0;
  //U32 sym_rate_KSs = 27500;

  //INT8 tn_version[20];

  mt_fe_hd2502_priv_handle p_priv = (mt_fe_hd2502_priv_handle)handle;

  //printk("%s[%d]: cmd %d - param 0x%08x\n", __FUNCTION__, __LINE__, cmd, param);

  switch (cmd)
  {
  case NIM_IOCTRL_CHANNEL_CHECK_LOCK:
    {
      UINT8 ret = 0;

      UINT8 locked, Status;

      ret = HDIC2501_IsDemodLocked(&locked, &Status);

      if (param != 0)
        ((mt_unf_fe_channel_info_t *)param)->lock = (locked == 1) ? 1 : 0;
  	}
    break;

  case NIM_IOCTRL_DISEQC1X:
    p_priv->diseqc_2x = 0;
    break;

  case NIM_IOCTRL_DISEQC2X:
    p_priv->diseqc_2x = 1;
    break;

  case NIM_IOCTRL_SET_PORLAR:
    if (p_priv->lnb_onoff == 0)
    {
      break;
    }

    if (p_priv->lnb_polar == param)
    {
      break;
    }

    voltage = ((param == PORT_PORLAR_HORIZONTAL) || (param == PORT_PORLAR_LEFT)) ? 1 : 0;
    port_hd2502_set_lnb_voltage(handle, voltage);
    p_priv->lnb_polar = (MT_U8)param;
    p_priv->lnb_voltage = voltage;
    break;

  case NIM_IOCTRL_SET_LNB_ONOFF:
    //printk("ker hd2502 line:%d param=0x%08x\n", __LINE__, param);

#if 0
    if (p_priv->lnb_onoff == param)
    {
      break;
    }
#endif

    //printk("NIM_IOCTRL_SET_LNB_ONOFF set %d\n", param);
    port_hd2502_set_lnb_onoff(handle, (MT_U8)param, &p_priv->cfg.pin_config);
    p_priv->lnb_onoff = param;
    if (param == 0)
    {
      break;
    }

    /* restore voltage */
    port_hd2502_set_lnb_voltage(handle, p_priv->lnb_voltage);

    break;

  case NIM_IOCTRL_SET_22K_ONOFF:
    break;

  case NIM_IOCTRL_GET_PORLAR:
    if (param != 0)
      *((MT_U8 *)param) = p_priv->lnb_polar;
    break;

  case NIM_IOCTRL_GET_22K_ONOFF:
    if (param != 0)
      *((MT_U8 *)param) = p_priv->onoff_22k;
    break;

  case NIM_IOCTRL_GET_TN_VERSION:
    //*((MT_U8 *)param) = p_priv->cfg.tun_support;

    break;

  case NIM_IOCTRL_GET_SIGNAL_INFO:
    port_hd2502_get_signal_info(handle, (mt_unf_fe_signal_info_t *)param);
    break;

  case NIM_IOCTRL_DISEQC_SENDMSG:
    p_sendmsg = (mt_unf_fe_diseqc_sendmsg_t *)param;
    port_hd2502_diseqc_sendmsg(handle, p_sendmsg);
    break;

  case NIM_IOCTRL_DISEQC_RECVMSG:
    p_recvmsg = (mt_unf_fe_diseqc_recvmsg_t *)param;
    port_hd2502_diseqc_recvmsg(handle, p_recvmsg);
    break;
    //#endif

  case NIM_IOCTRL_SAT_BS_EVENT_PROCESSED:
    break;

  case NIM_IOCTRL_GET_SAT_REAL_FREQ:
    break;

  case NIM_IOCTRL_GET_SAT_FREQ_OFFSET:
    if (param != 0)
      *((MT_S32 *)param) = carrieroffset_KHz;
    break;

  case NIM_IOCTRL_SUSPEND:
    hd2502_suspend();
    break;

  case NIM_IOCTRL_RESUME:
    hd2502_resume();
    break;

  default:
    break;
  }

  return MT_SUCCESS;
}


int hd2502_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr)
{
	mt_u8 reg_val = 0;
	//int ret = 0;
	mt_u32 temp = 0;
	mt_fe_hd2502_priv_handle p_priv = NULL;

	//printk(KERN_ERR "[%s %d]g_dev_init_flag=%d\n", __FUNCTION__, __LINE__, g_dev_init_flag);

	if (info->is_attach)
	{
		printk(KERN_ERR "[%s %d]has attach\n", __FUNCTION__, __LINE__);
		return MT_SUCCESS;
	}

	info->ops.connect = port_hd2502_channel_connect;
	info->ops.get_status = port_hd2502_get_status;
	info->ops.get_ber = port_hd2502_get_ber;
	info->ops.get_snr = port_hd2502_get_snr;
	info->ops.get_signal_strength = port_hd2502_get_signal_strength;
	info->ops.get_signal_quality = port_hd2502_get_signal_quality;
	info->ops.get_signal_agc = port_hd2502_get_signal_agc;

	info->ops.standby = port_hd2502_standby;
	info->ops.wakeup = port_hd2502_wakeup;
	info->ops.get_default_timeout = port_hd2502_get_default_timeout;
	info->ops.set_io = port_hd2502_set_io;
	info->ops.port_ioctl = port_hd2502_ioctl;
#if 1 // disable blindscan
	info->ops.blind_scan = NULL;
#else
	info->ops.blind_scan = port_hd2502_blind_scan;
#endif
	switch (attr->tuner_type)
	{
		case MT_UNF_TUNER_TYPE_RDA5812:
			hd2502_tuner_type = 1; // RDA_5812_TUNER;
			printk("%s[%d] ---- Select tuner[%d] RDA 5812!\n", __FUNCTION__, __LINE__, hd2502_tuner_type);
			break;

		case MT_UNF_TUNER_TYPE_SHARP6306:
		case MT_UNF_TUNER_TYPE_SHARP7306:
		case MT_UNF_TUNER_TYPE_SHARP6903:
		case MT_UNF_TUNER_TYPE_SHARP7903:
			hd2502_tuner_type = 2; // ISHARP TUNER;
			printk("%s[%d] ---- Select tuner[%d] Sharp!\n", __FUNCTION__, __LINE__, hd2502_tuner_type);
			break;

		case MT_UNF_TUNER_TYPE_M88TS6011:
			hd2502_tuner_type = 3; // Montage TUNER;
			break;
			
		case MT_UNF_TUNER_TYPE_RDA5815M:
		default:
			hd2502_tuner_type = 0; // RDA_5815M_TUNER;
			printk("%s[%d] ---- Select tuner[%d] RDA 5815M!\n", __FUNCTION__, __LINE__, hd2502_tuner_type);
			break;
	}
	if (!g_dev_init_flag)
	{
		switch (symphony_get_chip_rev())
		{
			case CHIP_SYMPHONY_A0:
			case CHIP_SYMPHONY_A1:
			case CHIP_SYMPHONY_A2:
			case CHIP_SYMPHONY3_A0:
				printk("%s[%d] -- Symphony1 or Symphony3 + HD2502\n", __FUNCTION__, __LINE__);
				break;

			case CHIP_SYMPHONY2_A0:
			case CHIP_SYMPHONY2_A1:
			case CHIP_SYMPHONY2_A2:
			case CHIP_SYMPHONY2_A3:
				printk("%s[%d] -- Symphony2 + HD2502\n", __FUNCTION__, __LINE__);

				/*read 0xbf5b0c00 before demod start working*/
				_mt_fe_read32_hd2502(0xbf5b0c00, &temp);

				/*apb address*/
				_mt_fe_read32_hd2502(0xbf138010, &temp);
				temp &= ~(0x3 << 0);
				_mt_fe_write32_hd2502(0xbf138010, temp);

				/*config demod mux*/
				_mt_fe_read32_hd2502(0xbf138020, &temp);
				temp &= ~(1 << 0);
				_mt_fe_write32_hd2502(0xbf138020, temp);

				/*config tuner i2c master pinmux*/
				_mt_fe_read32_hd2502(0xbf13c010, &temp);
				temp &= ~(0x0f << 16);
				temp &= ~(0x0f << 20);
				_mt_fe_write32_hd2502(0xbf13c010, temp);


				// 1. S2x_ts1_clk输入SW_PIN2_SEL BIT[19:16]=4,BIT[23:20]=4;对应寄存器：BF13C008.
				// 2. S2x_ts0_clk输入SW_PIN2_SEL BIT[15:12]=0,BIT[27:24]=0;对应寄存器：BF13C008.

				_mt_fe_read32_hd2502(0xbf13c008, &temp);
				temp &= ~(0xffff << 12);
				temp |= (0x04 << 16);
				temp |= (0x04 << 20);
				_mt_fe_write32_hd2502(0xbf13c008, temp);

				// enable ext TS0, 0xbf138008[12] = 0
				// enable ext TS3, 0xbf138008[20] = 0
				_mt_fe_read32_hd2502(0xbf138008, &temp);
				temp &= ~(0x01 << 20);
				temp &= ~(0x01 << 12);
				_mt_fe_write32_hd2502(0xbf138008, temp);

				// for 88 pin config
				_mt_fe_read32_hd2502(0xbf5d009c, &temp);
				temp &= ~0x3000000; // bit[25:24] = 0
				_mt_fe_write32_hd2502(0xbf5d009c, temp);

				_mt_fe_read32_hd2502(0xbf157000, &temp);
				temp &= ~0x0300000; // bit[21:20] = 0
				_mt_fe_write32_hd2502(0xbf157000, temp);

				break;

			case CHIP_SYMPHONY4_A0:
			case CHIP_SYMPHONY4_A1:
				printk("%s[%d] -- Symphony4 + HD2502\n", __FUNCTION__, __LINE__);
#if 0	// TS2 3线 SYNC
				//BF13C048[3:0]=5//TS2_CLK
				_mt_fe_read32_hd2502(0xbf13c048, &temp);
				temp &= ~0x0F;
				temp |= 0x05;
				_mt_fe_write32_hd2502(0xbf13c048, temp);

				//BF5D0094[3]=1  //IF PAD设置为digital input
				_mt_fe_read32_hd2502(0xbf5d0094, &temp);
				temp |= 0x08;
				_mt_fe_write32_hd2502(0xbf5d0094, temp);

				//BF13C1A0[2:0]=3 //TS2_SYNC pinmux
				_mt_fe_read32_hd2502(0xbf13c1a0, &temp);
				temp &= ~0x07;
				temp |= 0x03;
				_mt_fe_write32_hd2502(0xbf13c1a0, temp);

				//BF13C1A4[2:0]=2 //TS2_DATA
				_mt_fe_read32_hd2502(0xbf13c1a4, &temp);
				temp &= ~0x07;
				temp |= 0x02;
				_mt_fe_write32_hd2502(0xbf13c1a4, temp);

#if 0
				//BF50B000[0]=1 //open clk
				_mt_fe_read32_hd2502(0xbf50b000, &temp);
				temp |= 0x01;
				_mt_fe_write32_hd2502(0xbf50b000, temp);
#endif

				//BF50B004[6:5]=00 //clk正沿
				_mt_fe_read32_hd2502(0xbf50b004, &temp);
				temp &= ~0x60;
				_mt_fe_write32_hd2502(0xbf50b004, temp);

#if 0
				//BF50B00C[5]=1  //rest off
				_mt_fe_read32_hd2502(0xbf50b00c, &temp);
				temp |= 0x20;
				_mt_fe_write32_hd2502(0xbf50b00c, temp);
#endif

				//BF138008[0]=0 //TS2 input
				_mt_fe_read32_hd2502(0xbf138008, &temp);
				temp &= ~0x01;
				_mt_fe_write32_hd2502(0xbf138008, temp);

				//BF200020[31:0]=0xC7D801FF//TC2 CTR
				_mt_fe_write32_hd2502(0xbf200020, 0x0);
				_mt_fe_write32_hd2502(0xbf200020, 0xc7f101ff);
				//_mt_fe_write32_hd2502(0xbf200020, 0xc7d801ff);
#elif 1 // TS2 2线
				if(hd2502_tuner_type == 0)   //RDA_5815M_TUNER  demod - tsi2
				{
					//BF13C048[3:0]=5//TS2_CLK
					_mt_fe_read32_hd2502(0xbf13c048, &temp);
					temp &= ~0x0F;
					temp |= 0x05;
					_mt_fe_write32_hd2502(0xbf13c048, temp);

					//BF5D0094[3]=1  //IF PAD设置为digital input
					_mt_fe_read32_hd2502(0xbf5d0094, &temp);
					temp |= 0x08;
					_mt_fe_write32_hd2502(0xbf5d0094, temp);

					//BF13C1A4[2:0]=2 //TS2_DATA
					_mt_fe_read32_hd2502(0xbf13c1a4, &temp);
					temp &= ~0x07;
					temp |= 0x02;
					_mt_fe_write32_hd2502(0xbf13c1a4, temp);

					//BF50B000[0]=1 //open clk
					_mt_fe_read32_hd2502(0xbf50b000, &temp);
					temp |= 0x01;
					_mt_fe_write32_hd2502(0xbf50b000, temp);

					//BF50B004[6:5]=00 //clk正沿
					_mt_fe_read32_hd2502(0xbf50b004, &temp);
					temp &= ~0x60;
					_mt_fe_write32_hd2502(0xbf50b004, temp);

					//BF50B00C[5]=1  //rest off
					_mt_fe_read32_hd2502(0xbf50b00c, &temp);
					temp |= 0x20;
					_mt_fe_write32_hd2502(0xbf50b00c, temp);

					//BF138008[0]=0 //TS2 input
					_mt_fe_read32_hd2502(0xbf138008, &temp);
					temp &= ~0x01;
					_mt_fe_write32_hd2502(0xbf138008, temp);

					//BF200020[31:0]=0xC7D801FF//TC2 CTR
					_mt_fe_write32_hd2502(0xbf200020, 0x0);
					_mt_fe_write32_hd2502(0xbf200020, 0xc7d801ff);
				}
				if(hd2502_tuner_type == 3)   //TS6011  demod - tsi1    
				{
					//"0xBF5D0094[0]=1  //IQ PAD设置为digital input
					// 0xBF13C190[2:0]=2 // TS1_CLK pinmux
					//0xBF13C198[2:0]=2 // TS1_VALID pinmux
					//0xBF13C19C[2:0]=2 // TS1_DATA pinmux
					//0xBF138008[4]=0 //TS1 input
					//0xBF50B000[0]=1 //打开时钟
					//0xBF50B004[4:3]= 00 //ts1 clk 正沿
					//0xBF50B00C[4]=1  //复位释放//
					//0xBF200010[31:0]=0xC7EB11FF//TC1 CTR读取
					//0xBF200014 TS状态// bit31=1,bit30=1标示TS lock"		
					_mt_fe_read32_hd2502(0xbf5d0094, &temp);
					temp |= 0x01;
					_mt_fe_write32_hd2502(0xbf5d0094, temp);

					_mt_fe_read32_hd2502(0xBF13C190, &temp);
					temp &= ~0x07;
					temp |= 0x02;
					_mt_fe_write32_hd2502(0xBF13C190, temp);
					
					_mt_fe_read32_hd2502(0xBF13C198, &temp);
					temp &= ~0x07;
					temp |= 0x02;
					_mt_fe_write32_hd2502(0xBF13C198, temp);

					_mt_fe_read32_hd2502(0xBF13C19C, &temp);
					temp &= ~0x07;
					temp |= 0x02;
					_mt_fe_write32_hd2502(0xBF13C19C, temp);

					_mt_fe_read32_hd2502(0xBF138008, &temp);
					temp &= ~(0x01 << 4);
					_mt_fe_write32_hd2502(0xBF138008, temp);
					
					_mt_fe_read32_hd2502(0xBF50B000, &temp);
					temp |= 0x01;
					_mt_fe_write32_hd2502(0xBF50B000, temp);

					_mt_fe_read32_hd2502(0xBF50B004, &temp);
					temp &= ~(0x18);
					_mt_fe_write32_hd2502(0xBF50B004, temp);
										
					_mt_fe_read32_hd2502(0xBF50B00C, &temp);
					temp |= 0x10;
					_mt_fe_write32_hd2502(0xBF50B00C, temp);
					
					//BF200010[31:0]=0xC7D801FF//TC1 CTR
					_mt_fe_write32_hd2502(0xBF200010, 0x0);
					_mt_fe_write32_hd2502(0xBF200010, 0xc7c101ff);
				}
#endif
				break;
			case CHIP_SYMPHONY6_A1:
				printk("%s[%d] -- Symphony6 A1 + HD2502\n", __FUNCTION__, __LINE__);
				if(hd2502_tuner_type == 3)   //TS6011  demod - tsi1  
				{
					/*TS1 4line mode CLK Data0 SYNC VAILD*/
					//Pinmux Setting  
					/*Set TSI1_CLK bit[15:0] to 0x0002*/  
					_mt_fe_write32_hd2502(0xbf13c20c,0x0002);
					/*Set TSI1_SYNC bit[15:0] to 0x0002*/   
					_mt_fe_write32_hd2502(0xbf13c208,0x0002);
					/*Set TSI1_VAILD bit[15:0] to 0x0002*/  
					_mt_fe_write32_hd2502(0xbf13c204,0x0002);
					/*Set TSI1_DO bit[15:0] to 0x0002*/  
					_mt_fe_write32_hd2502(0xbf13c200,0x0002);
 
 					/*choose PAD TSI1 CLK*/
					_mt_fe_read32_hd2502(0xBF50B004, &temp);
					temp &=~ 0x300;
					_mt_fe_write32_hd2502(0xBF50B004, temp);

					//Analog Setting  
					_mt_fe_read32_hd2502(0xBF5D0094, &temp);
					temp |= 0x0001;
					_mt_fe_write32_hd2502(0xBF5D0094, temp);

					//TSI Setting 				
					_mt_fe_read32_hd2502(0xBF138008, &temp);
					temp |= 0x10;
					_mt_fe_write32_hd2502(0xBF138008, temp);

					/*Disable TS1 Bit[31] set to 0*/
					_mt_fe_write32_hd2502(0xBF200010, 0x0000);

					_mt_fe_write32_hd2502(0xbf200010, 0xc7cb01ff);

					/* data bitwidth slecte 1:bit 0:2bit  for sym6-a1*/ 
					_mt_fe_read32_hd2502(0xbf200080, &temp);
					temp |= 0x2490;
					_mt_fe_write32_hd2502(0xbf200080, temp);

					/*GPIO30 use for LNB-EN*/
					_mt_fe_write32_hd2502(0xBF13C078, 0x0001);
					drv_gpio_io_enable(GPIO_30, TRUE);
					drv_gpio_set_dir(GPIO_30, GPIO_DIR_OUTPUT);
				}
				break;
			default:
				printk("%s[%d] -- Unknown chip\n", __FUNCTION__, __LINE__);
				break;
		}

		/*read 0xbf5b0c00 before demod start working*/

		//_mt_fe_read32_hd2502(0xbf5b0c00, &temp);
		reg_val = HAL_GET_U8((volatile u8 *)(mt_get_demod_base()+ 0x00));
	}

	p_priv = kzalloc(sizeof(mt_fe_hd2502_priv_t), GFP_KERNEL);
	if (NULL == p_priv)
	{
		printk(KERN_ERR "%s[%d]kzalloc priv error\n", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	info->handle = (void *)p_priv;
	g_hd2502_priv = p_priv;

	g_i2c_hd2502 = attr->demod_i2c_id;

	printk(KERN_ERR "%s[%d] ---- g_i2c_hd2502 & demod_i2c_id[%d]\n", __FUNCTION__, __LINE__, g_i2c_hd2502);

	memcpy(&(p_priv->cfg), &(attr->fe_config), sizeof(mt_unf_fe_config_para_t));

	p_priv->sig_type = attr->sig_type;
	p_priv->onoff_22k = 0;
	p_priv->bs_stop = FALSE;
	p_priv->diseqc_2x = 0;
	p_priv->lnb_polar = PORT_PORLAR_HORIZONTAL;
	p_priv->lnb_voltage = 0;
	p_priv->lnb_onoff = 0;
	p_priv->cur_diseqc.tx_len = 0;
	//p_priv->cur_diseqc.p_tx_buf = priv->diseqc_tx_buf;

	p_priv->cfg.pin_config.lnb_enable = 1;              //MT_FE_PIN_LEVEL_HIGH;
	p_priv->cfg.pin_config.vsel_when_13v = 0;           //MT_FE_PIN_LEVEL_LOW;
	p_priv->cfg.pin_config.vsel_when_lnb_off = 1;       //MT_FE_PIN_LEVEL_HIGH;
	p_priv->cfg.pin_config.diseqc_out_when_lnb_off = 0; //MT_FE_PIN_LEVEL_LOW;
	p_priv->cfg.pin_config.lnb_enable_by_mcu = 0;
	p_priv->cfg.pin_config.lnb_prot_by_mcu = 0;

	if (p_priv->cfg.freq_offset_limit == 0)
	{
		p_priv->cfg.freq_offset_limit = 4000;
	}

	if (NULL == hd2502_pg_channel_info)
	{
		hd2502_pg_channel_info = kzalloc(sizeof(mt_unf_fe_channel_info_t) * MAX_BS_TP_NUM_PER_SAT, GFP_KERNEL);
		if (hd2502_pg_channel_info == NULL)
		{
			kfree((void *)p_priv);
			g_hd2502_priv = NULL;
			return -ENOMEM;
		}
	}
	p_priv->scan_info.p_channel_info = hd2502_pg_channel_info;


	if (!g_dev_init_flag)
	{
		if(hd2502_tuner_type == 0)   Pio_Init(GPIO_95);     //RDA_5815M_TUNER;
		if(hd2502_tuner_type == 3)
		{
			switch (symphony_get_chip_rev())
			{
				case CHIP_SYMPHONY4_A0:
				case CHIP_SYMPHONY4_A1:
					Pio_Init(GPIO_70);     //TS6011  demod - tsi1    
					break;
				case CHIP_SYMPHONY6_A1:
					printk("%s[%d] -- Symphony6 A1 HW rc reset,no need gpio\n", __FUNCTION__, __LINE__);
					p_priv->lnb_onoff_gpio = GPIO_30;
					break;
				default:
					break;
			}
			
		}
		HDIC2501_Init(hd2502_tuner_type);

		printk("%s[%d] ---- demod_addr[%02x], tuner_addr[%02x]\n", __FUNCTION__, __LINE__, attr->demod_addr, attr->tuner_addr);
	}

	info->is_attach = 1;
	info->pre_attr.demod_dev_type = attr->demod_dev_type;
	g_dev_init_flag = 1;
	g_priv_count++;
	//printk(KERN_ERR "[%s %d]success\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}

int hd2502_detach(frontend_info_s *info)
{
	mt_fe_hd2502_priv_handle p_priv = NULL;

	if (NULL == info)
	{
		printk(KERN_ERR "[%s %d]error, info is NULL\n", __FUNCTION__, __LINE__);
		return -EINVAL;
	}

	p_priv = (mt_fe_hd2502_priv_handle)(info->handle);
	if (NULL == p_priv)
	{
		printk(KERN_ERR "[%s %d]error, info is NULL\n", __FUNCTION__, __LINE__);
		return -EINVAL;
	}

	return MT_SUCCESS;
}

int hd2502_resume(void)
{
	mt_u8 reg_val = 0;
	//int ret = 0;
	mt_u32 temp = 0;

	//printk(KERN_ERR "[%s %d]g_dev_init_flag=%d\n", __FUNCTION__, __LINE__, g_dev_init_flag);

	if (1)//(!g_dev_init_flag)
	{
		switch (symphony_get_chip_rev())
		{
			case CHIP_SYMPHONY_A0:
			case CHIP_SYMPHONY_A1:
			case CHIP_SYMPHONY_A2:
			case CHIP_SYMPHONY3_A0:
				printk("%s[%d] -- Symphony1 or Symphony3 + hd2502\n", __FUNCTION__, __LINE__);
				break;

			case CHIP_SYMPHONY2_A0:
			case CHIP_SYMPHONY2_A1:
			case CHIP_SYMPHONY2_A2:
			case CHIP_SYMPHONY2_A3:
				printk("%s[%d] -- Symphony2 + hd2502\n", __FUNCTION__, __LINE__);

				/*read 0xbf5b0c00 before demod start working*/
				_mt_fe_read32_hd2502(0xbf5b0c00, &temp);

				/*apb address*/
				_mt_fe_read32_hd2502(0xbf138010, &temp);
				temp &= ~(0x3 << 0);
				_mt_fe_write32_hd2502(0xbf138010, temp);

				/*config demod mux*/
				_mt_fe_read32_hd2502(0xbf138020, &temp);
				temp &= ~(1 << 0);
				_mt_fe_write32_hd2502(0xbf138020, temp);

				/*config tuner i2c master pinmux*/
				_mt_fe_read32_hd2502(0xbf13c010, &temp);
				temp &= ~(0x0f << 16);
				temp &= ~(0x0f << 20);
				_mt_fe_write32_hd2502(0xbf13c010, temp);


				// 1. S2x_ts1_clk输入SW_PIN2_SEL BIT[19:16]=4,BIT[23:20]=4;对应寄存器：BF13C008.
				// 2. S2x_ts0_clk输入SW_PIN2_SEL BIT[15:12]=0,BIT[27:24]=0;对应寄存器：BF13C008.

				_mt_fe_read32_hd2502(0xbf13c008, &temp);
				temp &= ~(0xffff << 12);
				temp |= (0x04 << 16);
				temp |= (0x04 << 20);
				_mt_fe_write32_hd2502(0xbf13c008, temp);

				// enable ext TS0, 0xbf138008[12] = 0
				// enable ext TS3, 0xbf138008[20] = 0
				_mt_fe_read32_hd2502(0xbf138008, &temp);
				temp &= ~(0x01 << 20);
				temp &= ~(0x01 << 12);
				_mt_fe_write32_hd2502(0xbf138008, temp);

				// for 88 pin config
				_mt_fe_read32_hd2502(0xbf5d009c, &temp);
				temp &= ~0x3000000; // bit[25:24] = 0
				_mt_fe_write32_hd2502(0xbf5d009c, temp);

				_mt_fe_read32_hd2502(0xbf157000, &temp);
				temp &= ~0x0300000; // bit[21:20] = 0
				_mt_fe_write32_hd2502(0xbf157000, temp);

				break;

			case CHIP_SYMPHONY4_A0:
			case CHIP_SYMPHONY4_A1:
				/* fix issue 13008/13003 STB can not play after Standby
				* There are four sections that need to be set up before frontend works,
				* include PINMUX CLK ANALOG TS.
				* Two of the modules(PINMUX CLK) are backed up and restored in their own internal drivers, 
				* but the other two(ANALOG TS) are not. So it leads to problems
				* Frontend has to reset their Settings until the ANALOG/TS can fix the problem
				*/
				printk("%s[%d] -- Symphony4 + hd2502\n", __FUNCTION__, __LINE__);

				//BF5D0094[3]=1  //IF PAD设置为digital input
				_mt_fe_read32_hd2502(0xbf5d0094, &temp);
				temp |= 0x08;
				_mt_fe_write32_hd2502(0xbf5d0094, temp);

				//BF138008[0]=0 //TS2 input
				_mt_fe_read32_hd2502(0xbf138008, &temp);
				temp &= ~0x01;
				_mt_fe_write32_hd2502(0xbf138008, temp);

				//BF200020[31:0]=0xC7D801FF//TC2 CTR
				_mt_fe_write32_hd2502(0xbf200020, 0x0);
				_mt_fe_write32_hd2502(0xbf200020, 0xc7d801ff);
#if 0	// TS2 pinmux reference
				//BF13C048[3:0]=5//TS2_CLK
				_mt_fe_read32_hd2502(0xbf13c048, &temp);
				temp &= ~0x0F;
				temp |= 0x05;
				_mt_fe_write32_hd2502(0xbf13c048, temp);

				//BF5D0094[3]=1  //IF PAD设置为digital input
				_mt_fe_read32_hd2502(0xbf5d0094, &temp);
				temp |= 0x08;
				_mt_fe_write32_hd2502(0xbf5d0094, temp);

				//BF13C1A0[2:0]=1 //TS2_SYNC pin配置为GPIO
				_mt_fe_read32_hd2502(0xbf13c1a0, &temp);
				temp &= ~0x07;
				temp |= 0x01;
				_mt_fe_write32_hd2502(0xbf13c1a0, temp);

				//BF13C1A4[2:0]=2 //TS2_DATA
				_mt_fe_read32_hd2502(0xbf13c1a4, &temp);
				temp &= ~0x07;
				temp |= 0x02;
				_mt_fe_write32_hd2502(0xbf13c1a4, temp);

				//BF50B000[0]=1 //open clk
				_mt_fe_read32_hd2502(0xbf50b000, &temp);
				temp |= 0x01;
				_mt_fe_write32_hd2502(0xbf50b000, temp);

				//BF50B004[6:5]=00 //clk正沿
				_mt_fe_read32_hd2502(0xbf50b004, &temp);
				temp &= ~0x60;
				_mt_fe_write32_hd2502(0xbf50b004, temp);

				//BF50B00C[5]=1  //rest off
				_mt_fe_read32_hd2502(0xbf50b00c, &temp);
				temp |= 0x20;
				_mt_fe_write32_hd2502(0xbf50b00c, temp);

				//BF138008[0]=0 //TS2 input
				_mt_fe_read32_hd2502(0xbf138008, &temp);
				temp &= ~0x01;
				_mt_fe_write32_hd2502(0xbf138008, temp);

				//BF200020[31:0]=0xC7D801FF//TC2 CTR
				_mt_fe_write32_hd2502(0xbf200020, 0x0);
				_mt_fe_write32_hd2502(0xbf200020, 0xc7d801ff);

#elif 0	// TS3 Parallel
				// DEBUGB 0xBF13c124 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c124, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c124, temp);

				// DEBUGB 0xBF13c128 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c128, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c128, temp);

				// DEBUGB 0xBF13c12c 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c12c, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c12c, temp);

				// DEBUGB 0xBF13c130 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c130, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c130, temp);

				// DEBUGB 0xBF13c134 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c134, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c134, temp);

				// DEBUGB 0xBF13c164 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c164, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c164, temp);

				// DEBUGB 0xBF13c174 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c174, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c174, temp);

				// DEBUGB 0xBF13c178 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c178, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c178, temp);

				// DEBUGB 0xBF13c17c 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c17c, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c17c, temp);

				// DEBUGB 0xBF13c180 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c180, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c180, temp);

				// DEBUGB 0xBF13c184 3 0 0000
				_mt_fe_read32_hd2502(0xbf13c184, &temp);
				temp &= 0xfffffff0;
				_mt_fe_write32_hd2502(0xbf13c184, temp);

				// DEBUGB 0xBF138008 12 12 1
				_mt_fe_read32_hd2502(0xbf138008, &temp);
				temp |= 0x1000;
				_mt_fe_write32_hd2502(0xbf138008, temp);

				// DEBUGB 0xBF50b000 0 0 1
				_mt_fe_read32_hd2502(0xbf50b000, &temp);
				temp |= 0x01;
				_mt_fe_write32_hd2502(0xbf50b000, temp);

				// DEBUGB 0xBF50b00c 6 6 1
				_mt_fe_read32_hd2502(0xbf50b00c, &temp);
				temp |= 0x40;
				_mt_fe_write32_hd2502(0xbf50b00c, temp);

				// DEBUGB 0xBF50b004 8 7 01
				_mt_fe_read32_hd2502(0xbf50b004, &temp);
				temp &= 0xfffffe7f;
				temp |= 0x00000080;
				_mt_fe_write32_hd2502(0xbf50b004, temp);

				// DEBUG 0xBF200030 0x87c001ff

				_mt_fe_write32_hd2502(0xbf200030, 0x0);
				_mt_fe_write32_hd2502(0xbf200030, 0x87c001ff);

#endif
				break;

			default:
				printk("%s[%d] -- Unknown chip\n", __FUNCTION__, __LINE__);
				break;
		}

		/*read 0xbf5b0c00 before demod start working*/

		//_mt_fe_read32_hd2502(0xbf5b0c00, &temp);
		reg_val = HAL_GET_U8((volatile u8 *)(mt_get_demod_base()+ 0x00));
	}

	if (1) //(!g_dev_init_flag)
	{
		HDIC2501_Init(hd2502_tuner_type);

		//printk("%s[%d] ---- demod_addr[%02x], tuner_addr[%02x]\n", __FUNCTION__, __LINE__, dev_handle->demod_dev_addr, dev_handle->tuner_cfg.tuner_dev_addr);
	}

	//printk(KERN_ERR "[%s %d]success\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}

int hd2502_suspend(void)
{
	return MT_SUCCESS;
}


