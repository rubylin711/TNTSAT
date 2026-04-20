/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
//#include <linux/ioport.h>
//#include <linux/notifier.h>
//#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/fs.h>
//#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/atomic.h>
#include <asm/io.h>
//#include <linux/mutex.h>
#include <linux/clk.h>

#include <linux/kthread.h>
#include <linux/delay.h>

#include <net/sock.h>
#include <net/genetlink.h>

#include "mt_type.h"
#include "mt_module_debug.h"

#include "mt_drv_stat.h"
#include "mt_drv_dev.h"
#include "mt_drv_reg.h"
#include "mt_kernel_adapt.h"
#include "mt_drv_proc.h"
#include "mt_drv_module.h"

#include "drv_frontend_ext.h"
#include "drv_frontend_ioctl.h"

//#include "mt_fe_def_dvbc.h"
//#include "mt_fe_i2c.h"
#include "mt_error_mpi.h"

//#include "drv_frontend_priv.h"
//#include "./dm6k/port_dm6k.h"

#if 0
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <mt_unf_frontend.h>
#include <drv_frontend.h>
#include "drv_frontend_priv.h"
#include "./dm6k/port_dm6k.h"
#endif

#define DVBS_NL_NAME "dvbs_bs_nl"

/* cmd0 can match data0 and data1, cmd1 also can match data0 and data1 */
enum
{
  DVBS_NL_OPS_CMD0,
  //DVBS_NL_OPS_CMD1,
  __DVBS_NL_OPS_MAX,
};

#define DVBS_NL_OPS_AMOUNT (__DVBS_NL_OPS_MAX)

enum
{
  DVBS_NL_ATTR_UNSPEC,
  DVBS_NL_ATTR_DATA0,
  //DVBS_NL_ATTR_DATA1,
  __DVBS_NL_ATTR_MAX,
};

#define DVBS_NL_FAMILY_ATTR_MAX (__DVBS_NL_ATTR_MAX - 1)
#define DVBS_NL_FAMILY_ATTR_AMOUNT (__DVBS_NL_ATTR_MAX)


static struct genl_ops dvbs_nl_ops[DVBS_NL_OPS_AMOUNT];


struct genl_family dvbs_nl_family = {
    .name = DVBS_NL_NAME,
    .version = 0x1,
    .maxattr = DVBS_NL_FAMILY_ATTR_MAX,
    .netnsok = true,
    .module = THIS_MODULE,
    .ops = dvbs_nl_ops,
    .n_ops = ARRAY_SIZE(dvbs_nl_ops),
};


MT_DECLARE_MUTEX(g_fe_mutex);

//#define MAX_FRONTEND_NUM 3
/*fix issue26063 kernel panic*/
#define MAX_FRONTEND_NUM 6

static frontend_info_s feinfo[MAX_FRONTEND_NUM];
static struct mutex ioctl_lock;
extern int i2c_gpio_seq_read(
    u8 slv_addr,
    u8 *p_buf,
    u32 wlen,
    u32 rlen,
    u32 param);
extern int i2c_gpio_open(u32 bus_clk_khz);


fe_blindscan_param_t g_scan_info;

#ifdef CONFIG_NET
long mt_fe_ioctl_common(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    mt_u32 tid = 0;
    int ret = 0;
    fe_attr_t fe_attr;
    //fe_channel_info_t channel_info;
    fe_connect_para_t connect_para;
    //fe_status_t st;
    fe_status_t fe_stat;
    fe_ber_t fe_ber;
    fe_snr_t fe_snr;
    fe_signal_strength_t fe_strength;
    fe_signal_quality_t fe_quality;
    fe_data_t tuner_agc;
    
    if (MT_NULL == arg)
	{
		MT_ERR_FRONTEND("copy data from user error: cmd=%d\n", _IOC_NR(cmd));
		return MT_FAILURE;
    }

    switch (cmd)
    {
        case FE_SET_ATTR_CMD:
			if(copy_from_user(&fe_attr, argp, sizeof(fe_attr_t)))
			{
	                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
	                return MT_FAILURE;
	              }
				//printk("fe_ioctl line:%d.\n", __LINE__);
				//fe_attr = (fe_attr_t *)arg;
				tid = fe_attr.tuner_id;
	             if(tid >= MAX_FRONTEND_NUM)
	             {
	                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
	                return MT_FAILURE;
	              }
			mutex_lock(&ioctl_lock);
#if defined(CONFIG_MT_CHIP_ARIA)
			if (feinfo[tid].is_attach)
			{
			    if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_TYPE_M88DVBC)
				{
					ret = m88ca8k_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
			}
			//printk("fe_ioctl line:%d demod_dev_type %d.\n", __LINE__, p_attr->attr.demod_dev_type);

			if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_TYPE_M88DVBC)
			{
			    //printk("fe_ioctl line:%d.\n", __LINE__);
			    ret = m88ca8k_attach(&feinfo[tid], &(fe_attr.attr));
			    if (ret < 0)
					goto ioctl_error;
			}

			mutex_unlock(&ioctl_lock);

#else
			if (feinfo[tid].is_attach)
			{
			    if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CC6000 || 
			        feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DC2800)
			    {
					//ret = m88dc2800_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#ifdef CONFIG_MT_FRONTEND_DMD_TENOR
				else if ((feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6920) || 
						 (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6930) || 
						 (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6960) || 
						 (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RC6800) || 
						 (feinfo[tid].pre_attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6920) || 
						 (feinfo[tid].pre_attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6930) || 
						 (feinfo[tid].pre_attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6960) || 
						 (feinfo[tid].pre_attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88RC6800))
				{
					ret = m88tc6930_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD2856
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856)
				{
					//printk("fe_ioctl line:%d. sony cxd2856 detach\n", __LINE__);

					ret = sony_cxd2856_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY
				else if ((feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD_FAMILY)
						||(feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856))
				{
					//printk("fe_ioctl line:%d. sony cxd detach\n", __LINE__);

					ret = sony_cxd_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_DD3K
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DD3K)
				{
					ret = m88dd3k_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_DM6K
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DM6K)
				{
					ret = m88dm6k_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_CT8K
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CT8K)
				{
					ret = m88ct8k_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_CS8800
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8800)
				{
					ret = m88cs8800_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			       }
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_CS8K_CABLE
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB)
				{
					ret = m88cs8k_cab_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT)
				{
					ret = m88cs8k_sat_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_RS6060
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RS6060)
				{
					ret = m88rs6060_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_DS6103
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DS6103)
				{
					ret = m88ds6103_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_DS6113
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DS6113)
				{
					ret = m88ds6113_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_TP5001
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_TP5001)
				{
					ret = tp5001_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_HD2502
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_HD2502)
				{
					ret = hd2502_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
			}

			if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CC6000 ||
			    fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DC2800)
			{
			    //ret = m88dc2800_attach(&feinfo[tid], &attr.attr);
			    if (ret < 0)
					goto ioctl_error;
			}
			#ifdef CONFIG_MT_FRONTEND_DMD_TENOR
			else if ((fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6920) || 
					 (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6930) || 
					 (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6960) || 
					 (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RC6800) || 
					 (fe_attr.attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6920) || 
					 (fe_attr.attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6930) || 
					 (fe_attr.attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6960) || 
					 (fe_attr.attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88RC6800))
			{
				ret = m88tc6930_attach(&feinfo[tid], &fe_attr.attr);
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD2856
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856)
			{
				//printk("fe_ioctl line:%d. sony cxd2856 attach\n", __LINE__);

				ret = sony_cxd2856_attach(&feinfo[tid], &fe_attr.attr);
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY
			else if ((fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD_FAMILY)
					||(fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856))
			{
				//printk("fe_ioctl line:%d. sony cxd2856 attach\n", __LINE__);

				ret = sony_cxd_attach(&feinfo[tid], &fe_attr.attr);
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_DD3K
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DD3K)
			{
				ret = m88dd3k_attach(&feinfo[tid], &fe_attr.attr);
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_DM6K
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DM6K)
			{
			    ret = m88dm6k_attach(&feinfo[tid], &fe_attr.attr);
			    if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_CT8K
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CT8K)
			{
			    ret = m88ct8k_attach(&feinfo[tid], &fe_attr.attr);
			    if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_CS8800
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8800)
			{
			    ret = m88cs8800_attach(&feinfo[tid], &fe_attr.attr);
			    if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_CS8K_CABLE
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB)
			{
			    ret = m88cs8k_cab_attach(&feinfo[tid], &(fe_attr.attr));
			    if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT)
			{
			    ret = m88cs8k_sat_attach(&feinfo[tid], &(fe_attr.attr));
			    if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_RS6060
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RS6060)
			{
				ret = m88rs6060_attach(&feinfo[tid], &(fe_attr.attr));
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_DS6103
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DS6103)
			{
				ret = m88ds6103_attach(&feinfo[tid], &(fe_attr.attr));
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_DS6113
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DS6113)
			{
				ret = m88ds6113_attach(&feinfo[tid], &(fe_attr.attr));
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_HD2502
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_HD2502)
			{
				ret = hd2502_attach(&feinfo[tid], &(fe_attr.attr));
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			else
			{
				printk("%s[%d] ---- Unknown demod [%d]\n", __FUNCTION__, __LINE__, fe_attr.attr.demod_dev_type);
			}
			mutex_unlock(&ioctl_lock);
#endif
			break;

	    case FE_CONNECT_CMD:
	            mt_drv_stat_event(STAT_EVENT_CONNECT, 0);
				if(copy_from_user(&connect_para, argp, sizeof(fe_connect_para_t)))
	            {
	                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
	                return MT_FAILURE;
	            }   
				//connect_para = (fe_connect_para_t *)arg;
				tid = connect_para.tuner_id;
	            if(tid >= MAX_FRONTEND_NUM)
	            {
	                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
	                return MT_FAILURE;
	            }
		     mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.connect(feinfo[tid].handle, &(connect_para.para));
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &connect_para, sizeof(fe_connect_para_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			break;

	    case FE_GET_STATUS_CMD:
			if(copy_from_user(&fe_stat, argp, sizeof(fe_status_t)))
  	            {
  	                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
  	                return MT_FAILURE;
  	            }
  				tid = fe_stat.tuner_id;
  	            if(tid >= MAX_FRONTEND_NUM)
  	            {
  	                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
  	                return MT_FAILURE;
  	            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.get_status(feinfo[tid].handle, &(fe_stat.status));
			if (ret < 0)
			    goto ioctl_error;
            if(fe_stat.status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
                mt_drv_stat_event(STAT_EVENT_LOCKED, 0);
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_stat, sizeof(fe_status_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			break;

	    case FE_GET_BER_CMD:
			if(copy_from_user(&fe_ber, argp, sizeof(fe_ber_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = fe_ber.tuner_id;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.get_ber(feinfo[tid].handle, fe_ber.ber);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_ber, sizeof(fe_ber_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			break;

		case FE_GET_PRE_BER_CMD:
			if(copy_from_user(&fe_ber, argp, sizeof(fe_ber_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = fe_ber.tuner_id;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if (feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			//printk("%s[%d] ---- Get pre-BER, fe_ber.ber = 0x%08x\n", __FUNCTION__, __LINE__, fe_ber.ber);
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_PRE_BER, (ulong)fe_ber.ber);
			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_ber, sizeof(fe_ber_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			break;

	    case FE_GET_SNR_CMD:
			if(copy_from_user(&fe_snr, argp, sizeof(fe_snr_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = fe_snr.tuner_id;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.get_snr(feinfo[tid].handle, &(fe_snr.snr));
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_snr, sizeof(fe_snr_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			break;

	    case FE_GET_SIGNAL_STRENGTH_CMD:
			if(copy_from_user(&fe_strength, argp, sizeof(fe_signal_strength_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = fe_strength.tuner_id;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.get_signal_strength(feinfo[tid].handle, fe_strength.strength);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_strength, sizeof(fe_signal_strength_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			break;

	    case FE_GET_SIGNAL_QUALITY_CMD:
			if(copy_from_user(&fe_quality, argp, sizeof(fe_signal_quality_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = fe_quality.tuner_id;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.get_signal_quality(feinfo[tid].handle, &(fe_quality.quality));
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_quality, sizeof(fe_signal_quality_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			break;

	    case FE_GET_AGC_CMD:
			if(copy_from_user(&tuner_agc, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = tuner_agc.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			if (feinfo[tid].ops.get_signal_agc)
			{
			    ret = feinfo[tid].ops.get_signal_agc(feinfo[tid].handle, &(tuner_agc.data));
			    if (ret < 0)
					goto ioctl_error;
			}

			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &tuner_agc, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			break;

	    case FE_STANDBY_CMD:
			if(copy_from_user(&tid, argp, sizeof(mt_u32)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.standby(feinfo[tid].handle);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;

	    case FE_WAKEUP_CMD:
			if(copy_from_user(&tid, argp, sizeof(mt_u32)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.wakeup(feinfo[tid].handle);
			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;

		default:
			return -ENOTTY;
	}

	return 0;
ioctl_error:
	mutex_unlock(&ioctl_lock);

	return ret; 
}

long mt_fe_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    int ret = 0;
    fe_channel_info_t channel_info;
    //fe_attr_t fe_attr;
    mt_unf_fe_dss_scid_filter_t scid_filter = {0,};
 #if 0
    fe_attr_t fe_attr;

    fe_connect_para_t connect_para;
    fe_status_t fe_stat;
    fe_ber_t fe_ber;
    fe_snr_t fe_snr;
    fe_signal_strength_t fe_strength;
    fe_signal_quality_t fe_quality;
    fe_data_t tuner_agc;
 #endif

    fe_lnb_out_t lnb_onoff;
    fe_data_t onoff_22k;

    fe_data_t plp_data;
    fe_data_t cell_id;
    fe_data_t fe_data;

    fe_signal_info_t sig_info;
    /*
    fe_set_io_t set_io;
    fe_set_lnb_onoff_t lnb_onoff;
    fe_set_22k_onoff_t onoff_22k;
    fe_set_polar_t lnb_polar;
    fe_signal_info_t sig_info;
    */

    fe_blindscan_info_t bs_info;
    //blindscan_para_t bs_info;
    fe_blindscan_info_t *p_bs_info;

    //mt_unf_fe_blindscan_para_t *p_scan_info;
    //fe_blind_scan_t bs_info;
    //blindscan_para_t bs_info;
    fe_data_t bs_stat;
    //    fe_bs_result_t bs_result;
    //    fe_bs_result_t *p_bs_result;
    /*fe_common_mt_u32_t common_u32_para;
    fe_bs_result_t *p_bs_result;
    mt_u32 u32param;
    */

    fe_tn_param_t tn_param;

    //fe_diseqc_control_t diseqc_control;
    //fe_diseqc_control_t *p_diseqc_control = NULL;

    fe_diseqc_sendmsg_t diseqc_sendmsg;
    fe_diseqc_recvmsg_t diseqc_recvmsg;
    mt_unf_fe_diseqc_recvmsg_t rcvmsg;

    //mt_u8 u8param;

    //    mt_u8 i2c_addr;
    mt_u32 tid = 0;
    //    mt_u8 data;
    //    mt_u32 *p_tuner_id = 0;
    fe_data_t tone_data;

    fe_data_t reserved_data;

    fe_datalist_t multi_ts_list;
    fe_accurate_snr_t fe_accurate_snr;
	fe_status_t fe_stat;
    MT_DBG_FRONTEND("fe_ioctl line:%d, cmd = 0x%x\n", __LINE__,cmd);

    memset(&rcvmsg, 0, sizeof(mt_unf_fe_diseqc_recvmsg_t));
    if (MT_NULL == arg)
    {
        MT_ERR_FRONTEND("copy data from user error: cmd=%d\n", _IOC_NR(cmd));
        return MT_FAILURE;
    }


    switch (cmd)
	{
#if 0
	    case FE_SET_ATTR_CMD:
			copy_from_user(&fe_attr, argp, sizeof(fe_attr_t));
			//printk("fe_ioctl line:%d.\n", __LINE__);
			//fe_attr = (fe_attr_t *)arg;
			tid = fe_attr.tuner_id;
			mutex_lock(&ioctl_lock);
#if defined(CONFIG_MT_CHIP_ARIA)
			if (feinfo[tid].is_attach)
			{
			    if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_TYPE_M88DVBC)
				{
					ret = m88ca8k_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
			}
			//printk("fe_ioctl line:%d demod_dev_type %d.\n", __LINE__, p_attr->attr.demod_dev_type);

			if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_TYPE_M88DVBC)
			{
			    //printk("fe_ioctl line:%d.\n", __LINE__);
			    ret = m88ca8k_attach(&feinfo[tid], &(fe_attr.attr));
			    if (ret < 0)
					goto ioctl_error;
			}

			mutex_unlock(&ioctl_lock);

#else
			if (feinfo[tid].is_attach)
			{
			    if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CC6000 || 
			        feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DC2800)
			    {
					//ret = m88dc2800_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#ifdef CONFIG_MT_FRONTEND_DMD_TENOR
				else if ((feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6920) || 
						 (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6930) || 
						 (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6960) || 
						 (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RC6800) || 
						 (feinfo[tid].pre_attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6920) || 
						 (feinfo[tid].pre_attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6930) || 
						 (feinfo[tid].pre_attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6960) || 
						 (feinfo[tid].pre_attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88RC6800))
				{
					ret = m88tc6930_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD2856
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856)
				{
					//printk("fe_ioctl line:%d. sony cxd2856 detach\n", __LINE__);

					ret = sony_cxd2856_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY
				else if ((feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD_FAMILY)
						||(feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856))
				{
					//printk("fe_ioctl line:%d. sony cxd detach\n", __LINE__);

					ret = sony_cxd_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_DD3K
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DD3K)
				{
					ret = m88dd3k_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_DM6K
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DM6K)
				{
					ret = m88dm6k_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_CT8K
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CT8K)
				{
					ret = m88ct8k_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_CS8800
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8800)
				{
					ret = m88cs8800_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_CS8K_CABLE
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB)
				{
					ret = m88cs8k_cab_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT)
				{
					ret = m88cs8k_sat_detach(&feinfo[tid]);
					if (ret < 0)
					    goto ioctl_error;
			    }
				#endif
				#ifdef CONFIG_MT_FRONTEND_DMD_RS6060
				else if (feinfo[tid].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RS6060)
				{
					ret = m88rs6060_detach(&feinfo[tid]);
					if (ret < 0)
						goto ioctl_error;
				}
				#endif
			}

			if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CC6000 ||
			    fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DC2800)
			{
			    //ret = m88dc2800_attach(&feinfo[tid], &attr.attr);
			    if (ret < 0)
					goto ioctl_error;
			}
			#ifdef CONFIG_MT_FRONTEND_DMD_TENOR
			else if ((fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6920) || 
					 (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6930) || 
					 (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6960) || 
					 (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RC6800) || 
					 (fe_attr.attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6920) || 
					 (fe_attr.attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6930) || 
					 (fe_attr.attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88TC6960) || 
					 (fe_attr.attr.tuner_type == MT_UNF_TUNER_DEV_TYPE_M88RC6800))
			{
				ret = m88tc6930_attach(&feinfo[tid], &fe_attr.attr);
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD2856
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856)
			{
				//printk("fe_ioctl line:%d. sony cxd2856 attach\n", __LINE__);

				ret = sony_cxd2856_attach(&feinfo[tid], &fe_attr.attr);
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY
			else if ((fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD_FAMILY)
				  ||(fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856))
			{
				//printk("fe_ioctl line:%d. sony cxd attach\n", __LINE__);

				ret = sony_cxd_attach(&feinfo[tid], &fe_attr.attr);
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_DD3K
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DD3K)
			{
				ret = m88dd3k_attach(&feinfo[tid], &fe_attr.attr);
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_DM6K
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DM6K)
			{
			    ret = m88dm6k_attach(&feinfo[tid], &fe_attr.attr);
			    if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_CT8K
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CT8K)
			{
			    ret = m88ct8k_attach(&feinfo[tid], &fe_attr.attr);
			    if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_CS8800
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8800)
			{
			    ret = m88cs8800_attach(&feinfo[tid], &fe_attr.attr);
			    if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_CS8K_CABLE
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB)
			{
			    ret = m88cs8k_cab_attach(&feinfo[tid], &(fe_attr.attr));
			    if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8000_SAT)
			{
			    ret = m88cs8k_sat_attach(&feinfo[tid], &(fe_attr.attr));
			    if (ret < 0)
					goto ioctl_error;
			}
			#endif
			#ifdef CONFIG_MT_FRONTEND_DMD_RS6060
			else if (fe_attr.attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RS6060)
			{
				ret = m88rs6060_attach(&feinfo[tid], &(fe_attr.attr));
				if (ret < 0)
					goto ioctl_error;
			}
			#endif
			else
			{
				printk("%s[%d] ---- Unknown demod [%d]\n", __FUNCTION__, __LINE__, fe_attr.attr.demod_dev_type);
			}
			mutex_unlock(&ioctl_lock);
#endif
			break;

	    case FE_CONNECT_CMD:
            mt_drv_stat_event(STAT_EVENT_CONNECT, 0);
			copy_from_user(&connect_para, argp, sizeof(fe_connect_para_t));
			//connect_para = (fe_connect_para_t *)arg;
			tid = connect_para.tuner_id;
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.connect(feinfo[tid].handle, &(connect_para.para));
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			copy_to_user(argp, &connect_para, sizeof(fe_connect_para_t));
			break;

	    case FE_GET_STATUS_CMD:
			copy_from_user(&fe_stat, argp, sizeof(fe_status_t));
			tid = fe_stat.tuner_id;
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.get_status(feinfo[tid].handle, &(fe_stat.status));
			if (ret < 0)
			    goto ioctl_error;
            if(fe_stat.status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
                mt_drv_stat_event(STAT_EVENT_LOCKED, 0);
			mutex_unlock(&ioctl_lock);
			copy_to_user(argp, &fe_stat, sizeof(fe_status_t));
			break;

	    case FE_GET_BER_CMD:
			copy_from_user(&fe_ber, argp, sizeof(fe_ber_t));
			tid = fe_ber.tuner_id;
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.get_ber(feinfo[tid].handle, fe_ber.ber);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			copy_to_user(argp, &fe_ber, sizeof(fe_ber_t));
			break;

		case FE_GET_PRE_BER_CMD:
			copy_from_user(&fe_ber, argp, sizeof(fe_ber_t));
			tid = fe_ber.tuner_id;
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if (feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			//printk("%s[%d] ---- Get pre-BER, fe_ber.ber = 0x%08x\n", __FUNCTION__, __LINE__, fe_ber.ber);
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_PRE_BER, (mt_u32)fe_ber.ber);
			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			copy_to_user(argp, &fe_ber, sizeof(fe_ber_t));
			break;

	    case FE_GET_SNR_CMD:
			copy_from_user(&fe_snr, argp, sizeof(fe_snr_t));
			tid = fe_snr.tuner_id;
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.get_snr(feinfo[tid].handle, &(fe_snr.snr));
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			copy_to_user(argp, &fe_snr, sizeof(fe_snr_t));
			break;

	    case FE_GET_SIGNAL_STRENGTH_CMD:
			copy_from_user(&fe_strength, argp, sizeof(fe_signal_strength_t));
			tid = fe_strength.tuner_id;
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.get_signal_strength(feinfo[tid].handle, fe_strength.strength);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			copy_to_user(argp, &fe_strength, sizeof(fe_signal_strength_t));
			break;

	    case FE_GET_SIGNAL_QUALITY_CMD:
			copy_from_user(&fe_quality, argp, sizeof(fe_signal_quality_t));
			tid = fe_quality.tuner_id;
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.get_signal_quality(feinfo[tid].handle, &(fe_quality.quality));
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			copy_to_user(argp, &fe_quality, sizeof(fe_signal_quality_t));
			break;

	    case FE_GET_AGC_CMD:
			copy_from_user(&tuner_agc, argp, sizeof(fe_data_t));
			tid = tuner_agc.port;
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			if (feinfo[tid].ops.get_signal_agc)
			{
			    ret = feinfo[tid].ops.get_signal_agc(feinfo[tid].handle, &(tuner_agc.data));
			    if (ret < 0)
					goto ioctl_error;
			}

			mutex_unlock(&ioctl_lock);
			copy_to_user(argp, &tuner_agc, sizeof(fe_data_t));
			break;

	    case FE_STANDBY_CMD:
			copy_from_user(&tid, argp, sizeof(mt_u32));
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.standby(feinfo[tid].handle);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;

	    case FE_WAKEUP_CMD:
			copy_from_user(&tid, argp, sizeof(mt_u32));
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.wakeup(feinfo[tid].handle);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;
#else
        case FE_SET_ATTR_CMD:
        case FE_CONNECT_CMD:
        case FE_GET_STATUS_CMD:
        case FE_GET_BER_CMD:
        case FE_GET_PRE_BER_CMD:
        case FE_GET_SNR_CMD:
        case FE_GET_SIGNAL_STRENGTH_CMD:
        case FE_GET_SIGNAL_QUALITY_CMD:
        case FE_GET_AGC_CMD:
        case FE_STANDBY_CMD:
        case FE_WAKEUP_CMD:
            /*Clean warning larger than 1024 bytes [-Wframe-larger-than=]*/
            ret = mt_fe_ioctl_common(filp, cmd, arg);
            if(ret < 0)
               goto ioctl_common_error;
            break;
#endif
		case FE_GET_PLP_ID_CMD:
			if(copy_from_user(&plp_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			tid = plp_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}
			/*
			if(MT_UNF_FE_SIG_TYPE_DVB_T == feinfo[tid].pre_attr.sig_type)
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T_GET_HIERARCHY_NUM, (mt_u32) & plp_data.info);
			}
			else if(MT_UNF_FE_SIG_TYPE_DVB_T2 == feinfo[tid].pre_attr.sig_type)
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T2_GET_PLP_NUM, (mt_u32) & plp_data.info);
			}
			*/
			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T2_GET_PLP_NO, (ulong)&(plp_data.data));//(mt_u32) & plp_data.info);
			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &plp_data, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			break;


		case FE_GET_PLPNUM_CMD:
			if(copy_from_user(&plp_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = plp_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}
			/*
			if(MT_UNF_FE_SIG_TYPE_DVB_T == feinfo[tid].pre_attr.sig_type)
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T_GET_HIERARCHY_NUM, (mt_u32) & plp_data.info);
			}
			else if(MT_UNF_FE_SIG_TYPE_DVB_T2 == feinfo[tid].pre_attr.sig_type)
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T2_GET_PLP_NUM, (mt_u32) & plp_data.info);
			}
			*/
			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T2_GET_PLP_NUM, (ulong)&(plp_data.data));//(mt_u32) & plp_data.info);
			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &plp_data, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			break;

		case FE_SET_COMMONPLP_CMD:
			if(copy_from_user(&plp_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = plp_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}
			/*
			if(MT_UNF_FE_SIG_TYPE_DVB_T == feinfo[tid].pre_attr.sig_type)
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T_SET_HIERARCHY_NO, plp_data.data);
			}
			else if(MT_UNF_FE_SIG_TYPE_DVB_T2 == feinfo[tid].pre_attr.sig_type)
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T2_SET_PLP_NO, plp_data.data);
			}
			*/
			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T2_SET_COMMON_PLP_ID, plp_data.data);
			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;

		case FE_SET_PLPNO_CMD:
			if(copy_from_user(&plp_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = plp_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}
			/*
			if(MT_UNF_FE_SIG_TYPE_DVB_T == feinfo[tid].pre_attr.sig_type)
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T_SET_HIERARCHY_NO, plp_data.data);
			}
			else if(MT_UNF_FE_SIG_TYPE_DVB_T2 == feinfo[tid].pre_attr.sig_type)
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T2_SET_PLP_NO, plp_data.data);
			}
			*/
			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T2_SET_PLP_NO, plp_data.data);
			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;

		case FE_GET_HIERARCHY_ID_CMD:
			if(copy_from_user(&plp_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = plp_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T_GET_HIERARCHY_NO, (ulong)&(plp_data.data));//(mt_u32) & plp_data.info);
			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &plp_data, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			break;


		case FE_GET_HIERARCHY_NUM_CMD:
			if(copy_from_user(&plp_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = plp_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T_GET_HIERARCHY_NUM, (ulong)&(plp_data.data));//(mt_u32) & plp_data.info);
			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &plp_data, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			break;

		case FE_SET_HIERARCHY_ID_CMD:
			if(copy_from_user(&plp_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = plp_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T_SET_HIERARCHY_NO, plp_data.data);
			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;

		case FE_GET_T_T2_CELL_ID_CMD:
			if(copy_from_user(&cell_id, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = cell_id.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_T_T2_GET_CELL_ID, (ulong)&(cell_id.data));

			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &cell_id, sizeof(fe_data_t)))
             {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }  
			break;


		case FE_CHANNEL_CHECK_LOCK_CMD:
			if(copy_from_user(&channel_info, argp, sizeof(fe_channel_info_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			//p_channel_info = (fe_channel_info_t *)arg;
			tid = channel_info.tuner_id;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_CHANNEL_CHECK_LOCK, (ulong) & channel_info.info);
			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &channel_info, sizeof(fe_channel_info_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			break;
#if 0
	    case FE_IOC_CHECK_LOCK:
	        //copy_from_user(&channel_info, argp, sizeof(fe_channel_info_t));
	        p_lock_stat = (fe_data_t *)arg;
	        tid = p_lock_stat->port;
	        //mutex_lock(&ioctl_lock);
	        if (feinfo[tid].is_attach == 0)
	        {
	            ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	            goto ioctl_error;
	        }
	        ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_CHANNEL_CHECK_LOCK, (mt_u32)&(p_lock_stat->data));
	        if (ret < 0)
	            goto ioctl_error;
	        //mutex_unlock(&ioctl_lock);
	        //copy_to_user(argp, &channel_info, sizeof(fe_channel_info_t));
	        break;
#endif

	    case FE_SET_LNBOUT_CMD:
			if(copy_from_user(&lnb_onoff, argp, sizeof(fe_lnb_out_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			//p_lnb_onoff = (fe_lnb_out_t *)arg;
			tid = lnb_onoff.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			if (lnb_onoff.voltage > 0)
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_LNB_ONOFF, 1);

				if (lnb_onoff.voltage == TUNER_LNB_OUT_18V)
				{
				    ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_PORLAR, 0);
				}
				else if (lnb_onoff.voltage == TUNER_LNB_OUT_13V)
				{
				    ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_PORLAR, 1);
				}

				if (ret < 0)
				{
					goto ioctl_error;
				}
			}
			else
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_LNB_ONOFF, 0);
			}

			if (ret < 0)
			{
			    goto ioctl_error;
			}

			mutex_unlock(&ioctl_lock);
			break;

		case FE_SET_LNB_ONOFF_CMD:
			if(copy_from_user(&lnb_onoff, argp, sizeof(fe_lnb_out_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = lnb_onoff.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if (feinfo[tid].ops.port_ioctl)
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			}

			printk("[%s %d]lnb_onoff.on_off=%d\n", __FUNCTION__, __LINE__, lnb_onoff.on_off);
			if (lnb_onoff.on_off != 0)
			{
			    ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_LNB_ONOFF, 1);
			}
			else
			{
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_LNB_ONOFF, 0);
			}

			if (ret < 0)
			{
				goto ioctl_error;
			}

			mutex_unlock(&ioctl_lock);
			break;

#if 0
	    case FE_IOC_SET_PORLAR:
	        copy_from_user(&lnb_polar, argp, sizeof(fe_set_polar_t));
	        tid = lnb_polar.tuner_id;
	        mutex_lock(&ioctl_lock);
	        if (feinfo[tid].is_attach == 0)
	        {
	            ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	            goto ioctl_error;
	        }
	        ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                        NIM_IOCTRL_SET_PORLAR, lnb_polar.polar);
	        if (ret < 0)
	            goto ioctl_error;
	        mutex_unlock(&ioctl_lock);
	        break;
#endif

	    case FE_SEND_CONTINUOUS_22K_CMD: //FE_SET_22K_ONOFF_CMD:
			if(copy_from_user(&onoff_22k, argp, sizeof(fe_set_22k_onoff_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			//p_22k_onoff = (fe_data_t *)arg;
			tid = onoff_22k.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_22K_ONOFF, onoff_22k.data);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;

	    case FE_DISEQC1X_CMD:
			if(copy_from_user(&tid, argp, sizeof(mt_u8)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_DISEQC1X, 0);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;

	    case FE_DISEQC2X_CMD:
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_DISEQC2X, 0);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;
#if 0
	    case FE_GET_PORLAR_CMD:
	        copy_from_user(&lnb_polar, argp, sizeof(fe_set_polar_t));
	        tid = lnb_polar.tuner_id;
	        mutex_lock(&ioctl_lock);
	        if (feinfo[tid].is_attach == 0)
	        {
	            ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	            goto ioctl_error;
	        }
	        ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                        NIM_IOCTRL_GET_PORLAR, (mt_u32)&lnb_polar.polar);
	        if (ret < 0)
	            goto ioctl_error;
	        mutex_unlock(&ioctl_lock);
	        copy_to_user(argp, &lnb_polar, sizeof(fe_set_polar_t));
	        break;

	    case FE_GET_22K_ONOFF_CMD:
	        //copy_from_user(&onoff_22k, argp, sizeof(fe_set_22k_onoff_t));
	        tid = onoff_22k.tuner_id;
	        mutex_lock(&ioctl_lock);
	        if (feinfo[tid].is_attach == 0)
	        {
	            ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	            goto ioctl_error;
	        }
	        ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                        NIM_IOCTRL_GET_22K_ONOFF, (mt_u32)&onoff_22k.onoff);
	        if (ret < 0)
	            goto ioctl_error;
	        mutex_unlock(&ioctl_lock);
	        //copy_to_user(argp, &onoff_22k, sizeof(fe_set_22k_onoff_t));

	        case TUNER_IOC_GET_DEFAULT_TIMEOUT:
	           copy_from_user(&timeout, argp, sizeof(fe_def_timeout_t));
	           tid = timeout.tuner_id;
	           mutex_lock(&ioctl_lock);
	           if(feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.get_default_timeout(feinfo[tid].handle, &timeout.timeout);
	            if(ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            copy_to_user(argp, &timeout,  sizeof(fe_def_timeout_t));
	            break;
	        case TUNER_IOC_SET_IO:
	           copy_from_user(&set_io, argp, sizeof(fe_set_io_t));
	           tid = set_io.tuner_id;
	           mutex_lock(&ioctl_lock);
	           if(feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.set_io(feinfo[tid].handle, set_io.onoff);
	            if(ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            copy_to_user(argp, &set_io,  sizeof(fe_set_io_t));
	            break;
	        case TUNER_IOC_QUERY_TYPE:
	            copy_from_user(&i2c_addr, argp, sizeof(mt_u8));
	            mutex_lock(&ioctl_lock);
	            i2c_gpio_open(300);
	            data = 0;
	            ret = i2c_gpio_seq_read(i2c_addr, &data, 1, 1,0);
	            if(ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            break;
	        case TUNER_IOC_CHANNEL_CHECK_LOCK:
#if 0
	            copy_from_user(&channel_info, argp, sizeof(fe_channel_info_t));
	            tid = channel_info.tuner_id;
	            mutex_lock(&ioctl_lock);
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_CHANNEL_CHECK_LOCK, (mt_u32)&channel_info.channelInfo);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            copy_to_user(argp, &channel_info, sizeof(fe_channel_info_t));
#endif
	            break;
	        case TUNER_IOC_CHANGE_TN_MODE:
	            copy_from_user(&common_u32_para, argp, sizeof(fe_common_mt_u32_t));
	            tid = common_u32_para.tuner_id;
	            mutex_lock(&ioctl_lock);
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_CHANGE_TN_MODE, common_u32_para.para);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            break;
#endif

#if 0
#if 0
	        case FE_SET_ANTENNA_POWER_CMD:
	            //copy_from_user(&onoff_22k, argp, sizeof(fe_set_lnb_onoff_t));
	            tid = onoff_22k.tuner_id;
	            //mutex_lock(&ioctl_lock);
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_SET_LNB_ONOFF, onoff_22k.onoff);
	            if (ret < 0)
	                goto ioctl_error;
	            //mutex_unlock(&ioctl_lock);
	            break;
#endif

	        case TUNER_IOC_CHECK_LNB_SC_PROT:
	            //copy_from_user(&common_u8_para, argp, sizeof(fe_common_mt_u8_t));
	            tid = common_u8_para.tuner_id;
	            mutex_lock(&ioctl_lock);
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }

                if(feinfo[tid].ops.port_ioctl)
                    ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_CHECK_LNB_SC_PROT, (mt_u32)&common_u8_para.para);
	            if (ret < 0)
	                goto ioctl_error;
	            //mutex_unlock(&ioctl_lock);
	            //copy_to_user(argp, &common_u8_para, sizeof(fe_common_mt_u8_t));
	            break;

	        case TUNER_IOC_LNB_SC_PROT_RESTORE:
	            copy_from_user(&u8param, argp, sizeof(mt_u8));
	            tid = u8param;
	            mutex_lock(&ioctl_lock);
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }

                if(feinfo[tid].ops.port_ioctl)
                    ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_LNB_SC_PROT_RESTORE, 0);

	            if (ret < 0)
	                goto ioctl_error;

	            mutex_unlock(&ioctl_lock);
	            break;

	        case TUNER_IOC_REMOVE_PROTECT:
	            copy_from_user(&u8param, argp, sizeof(mt_u8));
	            tid = u8param;
	            mutex_lock(&ioctl_lock);
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }

                if(feinfo[tid].ops.port_ioctl)
                    ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_REMOVE_PROTECT, 0);

	            if (ret < 0)
	                goto ioctl_error;

	            mutex_unlock(&ioctl_lock);
	            break;

	        case TUNER_IOC_ENABLE_CHECK_PROTECT:
	            copy_from_user(&u8param, argp, sizeof(mt_u8));
	            tid = u8param;
	            mutex_lock(&ioctl_lock);
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }

                if(feinfo[tid].ops.port_ioctl)
                    ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_ENABLE_CHECK_PROTECT, 0);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            break;

	        case TUNER_IOC_GET_TN_VERSION:
	            copy_from_user(&common_u32_para, argp, sizeof(fe_common_mt_u32_t));
	            tid = common_u32_para.tuner_id;
	            mutex_lock(&ioctl_lock);
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }

                if(feinfo[tid].ops.port_ioctl)
                    ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_GET_TN_VERSION, (mt_u32)&common_u32_para.para);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            copy_to_user(argp, &common_u32_para, sizeof(mt_u8));
	            break;

	        case TUNER_IOC_RECOVER:
	            copy_from_user(&u8param, argp, sizeof(mt_u8));
	            tid = u8param;
	            mutex_lock(&ioctl_lock);
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_RECOVER, 0);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            break;

	        case TUNER_IOC_SET_CHANNEL_INFO:
	            copy_from_user(&common_u32_para, argp, sizeof(fe_common_mt_u32_t));
	            tid = common_u32_para.tuner_id;
	            mutex_lock(&ioctl_lock);
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_SET_CHANNEL_INFO, common_u32_para.para);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            break;
#endif
	    case FE_GET_SIGNAL_INFO_CMD:
#if 1
			if(copy_from_user(&sig_info, argp, sizeof(fe_signal_info_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
            tid = sig_info.tuner_id;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
			                                 NIM_IOCTRL_GET_SIGNAL_INFO, (ulong) & sig_info.info);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &sig_info, sizeof(fe_signal_info_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
#endif
		break;

		case FE_GET_TN_DBG_INFO_CMD:
			if(copy_from_user(&reserved_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = reserved_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_TN_DBG_INFO, (ulong)&reserved_data.data);

			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &reserved_data, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			break;

#if 0
	        case TUNER_IOC_GET_CHANNEL_INFO:
#if 0
	            copy_from_user(&channel_info, argp, sizeof(fe_channel_info_t));
	            mutex_lock(&ioctl_lock);
	            tid = channel_info.tuner_id;
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_GET_CHANNEL_INFO, (mt_u32)&channel_info.channelInfo);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            copy_to_user(argp, &channel_info, sizeof(fe_channel_info_t));
#endif
				break;
	        case TUNER_IOC_SET_DM_GPIO0_OUTPUT:
	            copy_from_user(&common_u8_para, argp, sizeof(fe_common_mt_u8_t));
	            mutex_lock(&ioctl_lock);
	            tid = common_u8_para.tuner_id;
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_SET_DM_GPIO0_OUTPUT, (mt_u32)&common_u8_para.para);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            break;
	        case TUNER_IOC_GET_DM_GPIO0_INPUT:
	            copy_from_user(&common_u8_para, argp, sizeof(fe_common_mt_u8_t));
	            mutex_lock(&ioctl_lock);
	            tid = common_u8_para.tuner_id;
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_GET_DM_GPIO0_INPUT, (mt_u32)&common_u8_para.para);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            copy_to_user(argp, &common_u8_para, sizeof(fe_common_mt_u8_t));
	            break;
	        case TUNER_IOC_SET_DM_GPIO1_OUTPUT:
	            copy_from_user(&common_u8_para, argp, sizeof(fe_common_mt_u8_t));
	            mutex_lock(&ioctl_lock);
	            tid = common_u8_para.tuner_id;
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_SET_DM_GPIO1_OUTPUT, (mt_u32)&common_u8_para.para);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            break;
	        case TUNER_IOC_GET_DM_GPIO1_INPUT:
	            copy_from_user(&common_u8_para, argp, sizeof(fe_common_mt_u8_t));
	            mutex_lock(&ioctl_lock);
	            tid = common_u8_para.tuner_id;
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle,
	                            NIM_IOCTRL_GET_DM_GPIO1_INPUT, (mt_u32)&common_u8_para.para);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            copy_to_user(argp, &common_u8_para, sizeof(fe_common_mt_u8_t));
	            break;

	        case FE_IOC_CONNECT_ASHCHRONOUS:
	            copy_from_user(&connect_para, argp, sizeof(fe_connect_para_t));
	            mutex_lock(&ioctl_lock);
	            tid = connect_para.tuner_id;
	            if (feinfo[tid].is_attach == 0)
	            {
	                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
	                goto ioctl_error;
	            }
	            ret = feinfo[tid].ops.connect_asychronous(feinfo[tid].handle, &connect_para.para);
	            if (ret < 0)
	                goto ioctl_error;
	            mutex_unlock(&ioctl_lock);
	            break;
#endif

	    case FE_BLINDSCAN_ACTION_CMD:
			if(copy_from_user(&bs_info, argp, sizeof(fe_blindscan_info_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
            tid = bs_info.tuner_id;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			//printk("mt_fe_ioctl line:%d tuner_id=%d\n", __LINE__, tid);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}
			//printk("mt_fe_ioctl line:%d tuner_id=%d\n", __LINE__, tid);
			//ret = feinfo[tid].ops.blind_scan(feinfo[tid].handle, (fe_blindscan_param_t *)bs_info.scan_info);
			if(copy_from_user(&g_scan_info, bs_info.scan_info, sizeof(fe_blindscan_param_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                ret = MT_FAILURE;
                goto ioctl_error;
            }
			ret = feinfo[tid].ops.blind_scan(feinfo[tid].handle, &g_scan_info);

			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			//printk("mt_fe_ioctl line:%d tuner_id=%d\n", __LINE__, tid);
			if(copy_to_user(argp, &bs_info, sizeof(fe_blindscan_info_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			break;

	    case FE_CHECK_BLIND_SCAN_STATUS_CMD:
			if(copy_from_user(&bs_stat, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			tid = bs_stat.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_SCAN_STATUS, (ulong) &bs_stat.data);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &bs_stat, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			break;

	    case FE_BLIND_SCAN_CANCEL_CMD:
			if(copy_from_user(&tid, argp, sizeof(mt_u32)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SCAN_CANCEL, (ulong)NULL);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;

	    case FE_GET_BLIND_SCAN_RESULT_CMD:
			if(copy_from_user(&bs_info, argp, sizeof(fe_blindscan_info_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = bs_info.tuner_id;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            bs_info.scan_info = kzalloc(sizeof(fe_blindscan_param_t), GFP_KERNEL);
			if (bs_info.scan_info == NULL)
			{
			    return -ENOMEM;
			}
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_SCAN_RESULT, (ulong)bs_info.scan_info);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);

			//p_bs_info = (fe_blindscan_info_t *)argp;
			//p_bs_result = (TUNER_BS_RESULT_S *)argp;
			//printk("###p_blind_scan_info->channel_num=%u\n", p_blind_scan_info->channel_num);
			if (bs_info.scan_info->channel_num_total)
			{
			    p_bs_info = (fe_blindscan_info_t *)argp;
			    if(copy_to_user(p_bs_info->scan_info->p_channel_info, bs_info.scan_info->p_channel_info,
			                 bs_info.scan_info->channel_num_total * sizeof(mt_unf_fe_channel_info_t)))
			    {
                    MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                    kfree(bs_info.scan_info);        
                    return MT_FAILURE;
                }
			    if(copy_to_user(p_bs_info->scan_info, bs_info.scan_info, sizeof(fe_blindscan_param_t)))
                {
                    MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                    kfree(bs_info.scan_info);        
                    return MT_FAILURE;
                }   
			    if(copy_to_user(argp, &bs_info, sizeof(fe_blindscan_info_t)))
                {
                    MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                    kfree(bs_info.scan_info);        
                    return MT_FAILURE;
                }   
			}
			kfree(bs_info.scan_info);
#if 0
			copy_from_user(&bs_result, argp, sizeof(fe_bs_result_t));
            tid = bs_result.tuner_id;
            bs_result.p_blind_scan_info = kzalloc(sizeof(mt_unf_fe_sat_blindscan_para_t), GFP_KERNEL);
            if (bs_result.p_blind_scan_info == NULL)
                return -ENOMEM;
            mutex_lock(&ioctl_lock);
            if (feinfo[tid].is_attach == 0)
            {
                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
                goto ioctl_error;
            }
            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_SCAN_RESULT, (mt_u32)bs_result.p_blind_scan_info);
            if (ret < 0)
                goto ioctl_error;
            mutex_unlock(&ioctl_lock);

			p_bs_result = (fe_bs_result_t *)argp;
            //p_bs_result = (TUNER_BS_RESULT_S *)argp;
			//printk("###p_blind_scan_info->channel_num=%u\n", p_blind_scan_info->channel_num);
#if 0
			copy_to_user(p_bs_result->p_channel_info, bs_result->p_blind_scan_info->p_channel_info,
                    bs_result.p_blind_scan_info->channel_num_total * sizeof(mt_unf_fe_channel_info_t));
            copy_to_user(bs_result.p_blind_scan_info, bs_result.p_blind_scan_info, sizeof(mt_unf_fe_scan_info_t));
            copy_to_user(argp, &bs_result, sizeof(fe_bs_result_t));
            kfree(bs_result.p_blind_scan_info);

#endif
            //printk("###bs_result.p_blind_scan_info->channel_num=%u\n", bs_result.p_blind_scan_info->channel_num);
            copy_to_user(p_bs_result->p_blind_scan_info->scan_para.sat->p_channel_info, bs_result.p_blind_scan_info->p_channel_info,
                    bs_result.p_blind_scan_info->channel_num_total * sizeof(mt_unf_fe_channel_info_t));
            copy_to_user(p_bs_result->p_blind_scan_info, bs_result.p_blind_scan_info, sizeof(mt_unf_fe_blindscan_para_t));
            copy_to_user(argp, &bs_result, sizeof(fe_bs_result_t));
            kfree(bs_result.p_blind_scan_info);
#endif

			break;
#if 0
        case FE_GET_BLIND_SCAN_RESULT_CMD:
            copy_from_user(&bs_result, argp, sizeof(fe_bs_result_t));
            tid = bs_result.tuner_id;
            bs_result.p_blind_scan_info = kzalloc(sizeof(mt_unf_fe_scan_info_t), GFP_KERNEL);
            if (bs_result.p_blind_scan_info == NULL)
                return -ENOMEM;
            mutex_lock(&ioctl_lock);
            if (feinfo[tid].is_attach == 0)
            {
                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
                goto ioctl_error;
            }
            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_SCAN_RESULT, (MT_U32)(bs_result.p_blind_scan_info));
            if (ret < 0)
                goto ioctl_error;
            mutex_unlock(&ioctl_lock);
            p_bs_result = (fe_bs_result_t *)argp;
            //printk("###bs_result.p_blind_scan_info->channel_num=%u\n", bs_result.p_blind_scan_info->channel_num);
            copy_to_user(p_bs_result->p_blind_scan_info->p_channel_info, bs_result.p_blind_scan_info->p_channel_info,
                    bs_result.p_blind_scan_info->channel_num_total * sizeof(mt_unf_fe_channel_info_t));
            copy_to_user(p_bs_result->p_blind_scan_info, bs_result.p_blind_scan_info, sizeof(mt_unf_fe_scan_info_t));
            kfree(bs_result.p_blind_scan_info);
            //copy_to_user(argp, &bs_result, sizeof(TUNER_BS_RESULT_S));
            break;
#endif

		case FE_BLIND_SCAN_EVENT_CMD:
			if(copy_from_user(&tid, argp, sizeof(mt_u8)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SAT_BS_EVENT_PROCESSED, (ulong)NULL);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			break;

	    case FE_DISEQC_SEND_MSG_CMD:
			if(copy_from_user(&diseqc_sendmsg, argp, sizeof(fe_diseqc_sendmsg_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = diseqc_sendmsg.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_DISEQC_SENDMSG,
			                                 (ulong)(fe_diseqc_sendmsg_t *)&diseqc_sendmsg.msg);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &diseqc_sendmsg, sizeof(fe_diseqc_sendmsg_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
#if 0
            //p_diseqc_control = (fe_diseqc_control_t *)argp;
            copy_from_user(&diseqc_control, argp, sizeof(fe_diseqc_control_t));
            copy_from_user(&diseqc_control, argp, sizeof(fe_diseqc_control_t));
			fe_diseqc_sendmsg_t
            tid = diseqc_control.tuner_id;
            mutex_lock(&ioctl_lock);
            if (feinfo[tid].is_attach == 0)
            {
                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
                goto ioctl_error;
            }
            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_DISEQC_CTL,
                        (mt_u32)(fe_diseqc_control_t *)&diseqc_control.diseqc_cmd);
            if (ret < 0)
                goto ioctl_error;
            mutex_unlock(&ioctl_lock);
            copy_to_user(argp, &diseqc_control, sizeof(fe_diseqc_control_t));
#endif
			break;

	    case FE_SEND_TONE_CMD:
			memset(&tone_data, 0, sizeof(fe_data_t));
			if(copy_from_user(&tone_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = tone_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_DISEQC_SEND_TONEBURST, tone_data.data);
			if (ret < 0)
			    goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &tone_data, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));        
                return MT_FAILURE;
            }
			break;

	    case FE_DISEQC_RECV_MSG_CMD:
			if(copy_from_user(&diseqc_recvmsg, argp, sizeof(fe_diseqc_recvmsg_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = diseqc_recvmsg.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			if(copy_from_user(&rcvmsg, diseqc_recvmsg.msg, sizeof(mt_unf_fe_diseqc_recvmsg_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                ret = MT_FAILURE;
                goto ioctl_error;
            }
			//printk("msglen:%d\n",rcvmsg.len);

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_DISEQC_RECVMSG,
											 (ulong)&rcvmsg);

			mutex_unlock(&ioctl_lock);
			if(copy_to_user(diseqc_recvmsg.msg, &rcvmsg, sizeof(mt_unf_fe_diseqc_recvmsg_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));        
                ret = MT_FAILURE;
            }
			if(copy_to_user(argp, &diseqc_recvmsg, sizeof(fe_diseqc_recvmsg_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));        
                ret = MT_FAILURE;
            }   
			if (ret < 0)
			{
				printk("receive msg status:%d\n",rcvmsg.status);
				return ret;
			}
			break;

		case FE_GET_REAL_FREQ:
			if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			tid = fe_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_SAT_REAL_FREQ, (ulong)&(fe_data.data));

			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_data, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));        
                return MT_FAILURE;
            }   
			break;

		case FE_GET_FREQ_OFFSET:
			if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			tid = fe_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_SAT_FREQ_OFFSET, (ulong)&(fe_data.data));

			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_data, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));        
                return MT_FAILURE;
            }   
			break;

		case FE_GET_REAL_SYMBOL:
			if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			tid = fe_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_SAT_REAL_SYM, (ulong)&(fe_data.data));

			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_data, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));        
                return MT_FAILURE;
            }   
			break;

		case FE_GET_S2_MULTI_STREAM_TS_CNT:
			if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			tid = fe_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SAT_GET_MULTI_STREAM_TS_CNT, (ulong)&(fe_data.data));

			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_data, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));        
                return MT_FAILURE;
            }   
			break;

		case FE_GET_S2_MULTI_STREAM_TS_ID:
			//copy_from_user(&fe_data, argp, sizeof(fe_data_t));
			if(copy_from_user(&multi_ts_list, argp, sizeof(fe_datalist_t)))
	            {
	                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
	                return MT_FAILURE;
	            }   
				tid = multi_ts_list.port;
	            if(tid >= MAX_FRONTEND_NUM)
	            {
	                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
	                return MT_FAILURE;
	            }
	            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			//ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SAT_GET_MULTI_STREAM_TS_LIST, &(multi_ts_list.data_list));
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SAT_GET_MULTI_STREAM_TS_LIST, (ulong)multi_ts_list.data_list);

			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &multi_ts_list, sizeof(fe_datalist_t)))
	            {
	                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));        
	                return MT_FAILURE;
	            }   
			break;

		case FE_SET_S2_MULTI_STREAM_TS_ID:
				if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
	            {
	                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
	                return MT_FAILURE;
	            }   
				tid = fe_data.port;
	            if(tid >= MAX_FRONTEND_NUM)
	            {
	                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
	                return MT_FAILURE;
	            }
	            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			//ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SAT_SET_MULTI_STREAM_TS_ID, &(fe_data.data));
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SAT_SET_MULTI_STREAM_TS_ID, fe_data.data);

			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			//copy_to_user(argp, &fe_data, sizeof(fe_data_t));
			break;

		case FE_GET_S2_GSE_ID_CMD:
			if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
	            {
	                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
	                return MT_FAILURE;
	            }
				tid = fe_data.port;
	            if(tid >= MAX_FRONTEND_NUM)
	            {
	                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
	                return MT_FAILURE;
	            }
			mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_TEST_FOR_GSE, fe_data.data);

			if (ret < 0)
				goto ioctl_error;

			mutex_unlock(&ioctl_lock);
			//copy_to_user(argp, &fe_data, sizeof(fe_data_t));
			break;

		case FE_SET_TUNER_PARAM_CMD:
			if(copy_from_user(&tn_param, argp, sizeof(fe_tn_param_t)))
	             {
	                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
	                 return MT_FAILURE;
	            }   
				tid = tn_param.tuner_id;
	            if(tid >= MAX_FRONTEND_NUM)
	            {
	                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
	                return MT_FAILURE;
	            }
			mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_TN_PARAM, (ulong)&tn_param);

			if (ret < 0)
				goto ioctl_error;

			mutex_unlock(&ioctl_lock);
			//copy_to_user(argp, &tn_param, sizeof(fe_tn_param_t));
			break;

		case FE_SET_SUPER_SEARCH_CMD:
			if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
	                {
	                	MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
	                 	return MT_FAILURE;
	                }
			tid = fe_data.port;

	                if(tid >= MAX_FRONTEND_NUM)
	                {
	                	MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
	                	return MT_FAILURE;
	                }
			mutex_lock(&ioctl_lock);


			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_SUPER_SEARCH, fe_data.data);

			if (ret < 0)
				goto ioctl_error;

			mutex_unlock(&ioctl_lock);
			//copy_to_user(argp, &fe_data, sizeof(fe_data_t));
			break;

		case FE_SET_UNICABLE_RETRY_CMD:
			if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = fe_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_UNICABLE_RETRY, fe_data.data);

			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			//copy_to_user(argp, &fe_data, sizeof(fe_data_t));
			break;

		case FE_SUSPEND_CMD:
			if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = fe_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SUSPEND, fe_data.data);

			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			//copy_to_user(argp, &fe_data, sizeof(fe_data_t));
			break;

		case FE_RESUME_CMD:
			if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = fe_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_RESUME, fe_data.data);

			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			//copy_to_user(argp, &fe_data, sizeof(fe_data_t));
			break;

			
		case FE_DSS_FILTER_CMD:
			copy_from_user(&scid_filter, argp, sizeof(mt_unf_fe_dss_scid_filter_t));
			tid = scid_filter.tuner_id;
			mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			
			printk("scid_filter = %x, %x\n",scid_filter.u16_scid[0], scid_filter.u16_scid[1]);
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_DSS_SCID_FILTER,  (ulong)(mt_unf_fe_dss_scid_filter_t *)&scid_filter);

			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			//copy_to_user(argp, &fe_data, sizeof(fe_data_t));
			break;
        case FE_GET_S2_BBH_TS_GSE_MODE_CMD:
			copy_from_user(&fe_data, argp, sizeof(fe_data_t));
			tid = fe_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_TS_GSE_MODE, (ulong)&(fe_data.data));

			if (ret < 0)
			  goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			copy_to_user(argp, &fe_data, sizeof(fe_data_t));
			break;
         case FE_REGISTER_NLK_FAMILY_CMD:
            copy_from_user(&fe_data, argp, sizeof(fe_data_t));
            tid = fe_data.port;
            mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}
            if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_REGISTER_NLK_FAMILY, 0);
            //pr_err("fe_ioctl line:%d. ret %d\n", __LINE__,ret);
			if (ret < 0)
			  goto ioctl_error;            
            mutex_unlock(&ioctl_lock);
            break;
         case FE_SET_LOWPOWER_CMD:
			if(copy_from_user(&fe_data, argp, sizeof(fe_data_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }
			tid = fe_data.port;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);

			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);

			ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_LOWPOWER, fe_data.data);

			if (ret < 0)
				goto ioctl_error;
			mutex_unlock(&ioctl_lock);
			//copy_to_user(argp, &fe_data, sizeof(fe_data_t));
			break;
         case FE_GET_ACCURATE_SNR_CMD:
            if(copy_from_user(&fe_accurate_snr, argp, sizeof(fe_accurate_snr_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }

            tid = fe_accurate_snr.tuner_id;
            if(tid >= MAX_FRONTEND_NUM)
            {
                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
                return MT_FAILURE;
            }
            mutex_lock(&ioctl_lock);
            if (feinfo[tid].is_attach == 0)
            {
                ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
                goto ioctl_error;
            }
	
            if(feinfo[tid].ops.port_ioctl)
                ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
                //ret = feinfo[tid].ops.get_snr(feinfo[tid].handle, &(fe_accurate_snr.snr));

            ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_ACCURATE_SNR, (ulong)&(fe_accurate_snr.snr));
            if (ret < 0)
                goto ioctl_error;
            mutex_unlock(&ioctl_lock);
            copy_to_user(argp, &fe_accurate_snr, sizeof(fe_accurate_snr_t));
            break;
		case FE_SET_S2_GS_PACKAGE_MODE_CMD:
            copy_from_user(&fe_data, argp, sizeof(fe_data_t));
			tid = fe_data.port;

			mutex_lock(&ioctl_lock);

			printk("%s====%d\n", __FUNCTION__, __LINE__);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_GS_PACKAGE_MODE, fe_data.data);

			if (ret < 0)
				goto ioctl_error;

			mutex_unlock(&ioctl_lock);
            break;
		case FE_SET_BBFARME_PADDING_CMD:
	        copy_from_user(&fe_data, argp, sizeof(fe_data_t));
			tid = fe_data.port;

			mutex_lock(&ioctl_lock);

			printk("%s====%d\n", __FUNCTION__, __LINE__);
			if (feinfo[tid].is_attach == 0)
			{
				ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
				goto ioctl_error;
			}

			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_BBFRAME_PADDING_ONOFF, fe_data.data);

			if (ret < 0)
				goto ioctl_error;

			mutex_unlock(&ioctl_lock);
            break;
		case FE_GET_FAST_LOCK_CMD:
			if(copy_from_user(&fe_stat, argp, sizeof(fe_status_t)))
  	            {
  	                MT_ERR_FRONTEND("[%s %d]copy data from user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
  	                return MT_FAILURE;
  	            }
  				tid = fe_stat.tuner_id;
  	            if(tid >= MAX_FRONTEND_NUM)
  	            {
  	                MT_ERR_FRONTEND("[%s %d]tid is %d,Cannot be greater than %d(MAX_NUM),pls check\n",__FUNCTION__,__LINE__,tid,MAX_FRONTEND_NUM);
  	                return MT_FAILURE;
  	            }
			mutex_lock(&ioctl_lock);
			if (feinfo[tid].is_attach == 0)
			{
			    ret = MT_UNF_TUNER_ERR_ATTR_UNSET;
			    goto ioctl_error;
			}

			//if(feinfo[tid].ops.port_ioctl)
				//ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_SET_MODULE_INDEX, tid);
			
			if(feinfo[tid].ops.port_ioctl)
				ret = feinfo[tid].ops.port_ioctl(feinfo[tid].handle, NIM_IOCTRL_GET_FAST_LOCK, (ulong)&(fe_stat.status));
			if (ret < 0)
			    goto ioctl_error;
            //if(fe_stat.status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
               //mt_drv_stat_event(STAT_EVENT_LOCKED, 0);
			mutex_unlock(&ioctl_lock);
			if(copy_to_user(argp, &fe_stat, sizeof(fe_status_t)))
            {
                MT_ERR_FRONTEND("[%s %d]copy data to user error: cmd=%d\n",__FUNCTION__,__LINE__,_IOC_NR(cmd));
                return MT_FAILURE;
            }   
			break;
		default:
			return -ENOTTY;
	}

	return 0;

ioctl_error:
	mutex_unlock(&ioctl_lock);
ioctl_common_error:
	return ret;
}

#endif


mt_s32 mt_fe_open(struct inode *inode, struct file *filp)
{
    mt_s32 ret = 0;
    MT_S32 mt_idx = iminor(inode);
    fe_priv_data_s *fe_priv_data = get_mt_priv(mt_idx);

    if (atomic_inc_return(&fe_priv_data->atmOpenCnt) == 1)
    {
        if (!(IS_ERR_OR_NULL(fe_priv_data->mclk)))
          clk_prepare_enable(fe_priv_data->mclk);
        if (!(IS_ERR_OR_NULL(fe_priv_data->xtalclk)))
          clk_prepare_enable(fe_priv_data->xtalclk);
        if (!(IS_ERR_OR_NULL(fe_priv_data->drvclk)))
          clk_prepare_enable(fe_priv_data->drvclk);
    }
#if 0
    ret = down_interruptible(&g_fe_mutex);
	if (atomic_inc_return(&fe_port_availble) == 1)
    {
        tuner_enable_crg();
        tuner_enable_adc();
        ret = fe_get_i2c_func();
        if (MT_SUCCESS != ret)
        {
            MT_ERR_FE("fe_get_i2c_func err\n");
            //return MT_FAILURE;
        }
    }
    up(&g_fe_mutex);
#else
/*
    ret = down_interruptible(&g_fe_mutex);

	if (atomic_inc_return(&fe_port_availble) == 1)
    {
		memset(fe_sat_resume_info, 0, sizeof(fe_sat_resume_info)*5);
    }
    up(&g_fe_mutex);
	*/
#endif

    return ret;
}

mt_s32 mt_fe_release(struct inode *inode, struct file *filp)
{
    //    mt_s32 ret = 0;
    MT_S32 mt_idx = iminor(inode);
    fe_priv_data_s *fe_priv_data = get_mt_priv(mt_idx);

#if 0
	ret =down_interruptible(&g_fe_mutex);

	if (atomic_dec_return(&fe_port_availble) == 0)
	{
	   tuner_disable_crg();
	   tuner_disable_adc();
	}
	up(&g_fe_mutex);
#endif

    if (atomic_dec_return(&fe_priv_data->atmOpenCnt) != 0) 
    {
        MT_ALWAYS_PRINT("%s  atmOpenCnt  is not zero !\n", __func__);
    }
    else
    {
        if (!(IS_ERR_OR_NULL(fe_priv_data->mclk)))
            clk_disable_unprepare(fe_priv_data->mclk);
        if (!(IS_ERR_OR_NULL(fe_priv_data->xtalclk)))
            clk_disable_unprepare(fe_priv_data->xtalclk);
        if (!(IS_ERR_OR_NULL(fe_priv_data->drvclk)))
            clk_disable_unprepare(fe_priv_data->drvclk);
    }

    return MT_SUCCESS;
}

#if 0
long mt_fe_ioctl(struct file *filp, mt_u32 cmd, unsigned long arg)
{
    mt_s32 ret;

    ret = down_interruptible(&g_fe_mutex);

    if (MT_FE_IOC_MAGIC != _IOC_TYPE(cmd)) {
	up(&g_fe_mutex);
	return -ENOTTY;
    }

    ret = mt_drv_usercopy(filp->f_path.dentry->d_inode, filp, cmd, arg, fe_ioctl);

    up(&g_fe_mutex);
    return ret;
}
#endif

mt_s32 fe_proc_read_reg(struct seq_file *p, mt_void *v)
{
    mt_u32 i;

    PROC_PRINT(p, "---------Montage fe info---------\n");

    for (i = 0; i < MAX_FRONTEND_NUM; i++)
    {
#if 0
        if ( MT_NULL == feinfo[port].ops.fe_connect)
        {
            continue;
        }

        if (feinfo[port].ops.fe_get_registers)
        {
            feinfo[port].ops.fe_get_registers(port, p);
        }
#endif

        if (feinfo[i].is_attach == 0)
			continue;
    }

    return MT_SUCCESS;
}

mt_s32 fe_proc_read(struct seq_file *p, mt_void *v)
{
    return MT_SUCCESS;
}

mt_s32 drv_frontend_suspend(void)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 i = 0;

    for (i = 0; i < MAX_FRONTEND_NUM; i++)
    {
        if (feinfo[i].is_attach)
        {
#ifdef CONFIG_MT_FRONTEND_DMD_CS8800
            if (feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8800)
            {
                ret = m88cs8800_suspend();
                if (ret < 0)
                    return ret;
            }
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD2856
            if (feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856)
            {
                /*fix issue28152*/
                ret = sony_cxd2856_suspend(feinfo[i].handle);
                if (ret < 0)
                    return ret;
            }
#endif
#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY
            if ((feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD_FAMILY)
				||(feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856))
			{
                ret = sony_cxd_suspend(feinfo[i].handle);
                if (ret < 0)
                    return ret;
            }
#endif
#ifdef CONFIG_MT_FRONTEND_DMD_RS6060
            if (feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RS6060)
            {
                ret = m88rs6060_suspend();
                if (ret < 0)
                    return ret;
            }
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_DS6103
            if (feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DS6103)
            {
                ret = m88ds6103_suspend();
                if (ret < 0)
                    return ret;
            }
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_DS6113
            if (feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DS6113)
            {
                ret = m88ds6113_suspend();
                if (ret < 0)
                    return ret;
            }
#endif
        }
    }

    return ret;
}

mt_s32 drv_frontend_resume(void)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 i = 0;

    for (i = 0; i < MAX_FRONTEND_NUM; i++)
    {
        if (feinfo[i].is_attach)
        {
#ifdef CONFIG_MT_FRONTEND_DMD_CS8800
            if (feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8800)
            {
                ret = m88cs8800_resume();
                if (ret < 0)
                    return ret;
            }
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD2856
            if (feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856)
            {
                /*fix issue28152*/
                ret = sony_cxd2856_resume(feinfo[i].handle);
                if (ret < 0)
                    return ret;
            }
#endif
#ifdef CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY
            if ((feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD_FAMILY)
				||(feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856))
			{

                ret = sony_cxd_resume(feinfo[i].handle);
                if (ret < 0)
                    return ret;
            }
#endif
#ifdef CONFIG_MT_FRONTEND_DMD_RS6060
            if (feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RS6060)
            {
                ret = m88rs6060_resume();
                if (ret < 0)
                    return ret;
            }
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_DS6103
            if (feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DS6103)
            {
                ret = m88ds6103_resume();
                if (ret < 0)
                    return ret;
            }
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_DS6113
            if (feinfo[i].pre_attr.demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DS6113)
            {
                ret = m88ds6113_resume();
                if (ret < 0)
                    return ret;
            }
#endif
        }
    }

    return ret;
}

mt_s32 fe_suspend(basedev_s *pdev, pm_message_t state)
{
    mt_s32 ret = MT_SUCCESS;
    fe_priv_data_s *fe_priv_data = dev_get_platdata(&pdev->dev);

    ret = drv_frontend_suspend();
    if (ret != MT_SUCCESS)
    {
        MT_PRINT("drv_frontend_suspend() failed\n");
    }

    MT_PRINT("fe_suspend OK\n");

    if (!(IS_ERR_OR_NULL(fe_priv_data->mclk)))
        clk_disable_unprepare(fe_priv_data->mclk);
    if (!(IS_ERR_OR_NULL(fe_priv_data->xtalclk)))
        clk_disable_unprepare(fe_priv_data->xtalclk);
    if (!(IS_ERR_OR_NULL(fe_priv_data->drvclk)))
        clk_disable_unprepare(fe_priv_data->drvclk);

    return MT_SUCCESS;
}

mt_s32 fe_resume(basedev_s *pdev)
{
    mt_s32 ret = MT_SUCCESS;
    fe_priv_data_s *fe_priv_data = dev_get_platdata(&pdev->dev);

    if (!(IS_ERR_OR_NULL(fe_priv_data->mclk)))
        clk_prepare_enable(fe_priv_data->mclk);
    if (!(IS_ERR_OR_NULL(fe_priv_data->xtalclk)))
        clk_prepare_enable(fe_priv_data->xtalclk);
    if (!(IS_ERR_OR_NULL(fe_priv_data->drvclk)))
        clk_prepare_enable(fe_priv_data->drvclk);


    ret = drv_frontend_resume();
    if (ret != MT_SUCCESS)
    {
        MT_PRINT("drv_frontend_resume() failed\n");
    }

    MT_PRINT("fe_resume OK\n");

    return MT_SUCCESS;
}

#define FRONTEND_NAME "mt_tuner"

static fe_export_func_s g_fe_ext_funcs =
    {
        .fe_suspend = fe_suspend,
        .fe_resume = fe_resume
    };

mt_s32 mt_drv_fe_init(mt_void)
{
    mt_s32 ret;
    mt_u8 i = 0;

    for (i = 0; i < MAX_FRONTEND_NUM; i++)
    {
        feinfo[i].is_attach = 0;
    }

    mutex_init(&ioctl_lock);

    ret = mt_drv_module_register(MT_ID_FRONTEND, FRONTEND_NAME, (mt_void *)&g_fe_ext_funcs);
    if (MT_SUCCESS != ret)
    {
        MT_FATAL_FE("mt_drv_module_register failed\n");

        return ret;
    }

    return ret; //MT_SUCCESS;
}

mt_void mt_drv_fe_deinit(mt_void)
{
    return;
}

mt_void mt_unf_fe_calc_PLS_gold_code(mt_u8 *pNormalCode, mt_u32 PLSGoldCode)
{
	typedef struct _PLS_Table_t
	{
		mt_u32	iIndex;
		mt_u8	PLSCode[3];
	}PLS_Table_t;

	static PLS_Table_t PLS_List[] = 
	{
		{     0, 	{0x01, 	0x00, 	0x00}},
		{  5000, 	{0x0d, 	0xe0, 	0x00}},
		{ 10000, 	{0x51, 	0x15, 	0x00}},
		{ 15000, 	{0xcf, 	0xc9, 	0x00}},
		{ 20000, 	{0x67, 	0x33, 	0x03}},
		{ 25000, 	{0x02, 	0xc9, 	0x02}},
		{ 30000, 	{0xe5, 	0xc6, 	0x01}},
		{ 35000, 	{0xdb, 	0xc0, 	0x03}},
		{ 40000, 	{0x7c, 	0x5f, 	0x02}},
		{ 45000, 	{0x8d, 	0x65, 	0x00}},
		{ 50000, 	{0x14, 	0x96, 	0x00}},
		{ 55000, 	{0xf7, 	0x61, 	0x03}},
		{ 60000, 	{0xbc, 	0x28, 	0x00}},
		{ 65000, 	{0x77, 	0xa9, 	0x01}},
		{ 70000, 	{0xe7, 	0x05, 	0x01}},
		{ 75000, 	{0x88, 	0x85, 	0x01}},
		{ 80000, 	{0x2f, 	0xbb, 	0x02}},
		{ 85000, 	{0xe1, 	0x07, 	0x00}},
		{ 90000, 	{0xd5, 	0x67, 	0x01}},
		{ 95000, 	{0x94, 	0x37, 	0x03}},
		{100000, 	{0x57, 	0x39, 	0x02}},
		{105000, 	{0xc7, 	0x03, 	0x00}},
		{110000, 	{0xbf, 	0x12, 	0x00}},
		{115000, 	{0x50, 	0x0e, 	0x00}},
		{120000, 	{0xca, 	0xc4, 	0x00}},
		{125000, 	{0x46, 	0xc3, 	0x00}},
		{130000, 	{0x2f, 	0xc6, 	0x01}},
		{135000, 	{0x7c, 	0xe5, 	0x01}},
		{140000, 	{0xb9, 	0x36, 	0x01}},
		{145000, 	{0x9d, 	0xe5, 	0x01}},
		{150000, 	{0xc4, 	0x32, 	0x01}},
		{155000, 	{0x13, 	0xb3, 	0x00}},
		{160000, 	{0x0c, 	0x9f, 	0x02}},
		{165000, 	{0xb2, 	0xb5, 	0x03}},
		{170000, 	{0xac, 	0x7e, 	0x01}},
		{175000, 	{0xb6, 	0xa2, 	0x01}},
		{180000, 	{0xb6, 	0x3e, 	0x01}},
		{185000, 	{0x17, 	0x2c, 	0x02}},
		{190000, 	{0xd7, 	0x2a, 	0x02}},
		{195000, 	{0x93, 	0x61, 	0x02}},
		{200000, 	{0x67, 	0x92, 	0x02}},
		{205000, 	{0x38, 	0x07, 	0x01}},
		{210000, 	{0xb4, 	0x5a, 	0x01}},
		{215000, 	{0xed, 	0x31, 	0x02}},
		{220000, 	{0x9e, 	0x4d, 	0x02}},
		{225000, 	{0x17, 	0x08, 	0x02}},
		{230000, 	{0x37, 	0xb9, 	0x00}},
		{235000, 	{0x2c, 	0xed, 	0x00}},
		{240000, 	{0xe0, 	0x64, 	0x00}},
		{245000, 	{0x90, 	0x39, 	0x01}},
		{250000, 	{0x35, 	0x0e, 	0x01}},
		{255000, 	{0x1c, 	0x9e, 	0x02}},
		{260000, 	{0x58, 	0x78, 	0x00}}
	};

	unsigned char x0;
	unsigned char x1;
	unsigned char x2;
	unsigned char x3;
	unsigned char x4;
	unsigned char x5;
	unsigned char x6;
	unsigned char x7;
	unsigned char x8;
	unsigned char x9;
	unsigned char x10;
	unsigned char x11;
	unsigned char x12;
	unsigned char x13;
	unsigned char x14;
	unsigned char x15;
	unsigned char x16;
	unsigned char x17;
	int i;
	unsigned char tmp;

	mt_u32 ulPLSCode = 0, ulPLSIndex = 0;
	int iPLSCnt = sizeof(PLS_List) / sizeof(PLS_Table_t), iPLSListIndex;
	mt_u8 iPLSCode[3];

	ulPLSCode = PLSGoldCode;

	iPLSListIndex = ulPLSCode / 5000;

	if(iPLSListIndex > iPLSCnt - 1)
		iPLSListIndex = iPLSCnt - 1;

	ulPLSIndex = PLS_List[iPLSListIndex].iIndex;
	iPLSCode[0] = PLS_List[iPLSListIndex].PLSCode[0];
	iPLSCode[1] = PLS_List[iPLSListIndex].PLSCode[1];
	iPLSCode[2] = PLS_List[iPLSListIndex].PLSCode[2];

	x0  = (iPLSCode[0] >> 0) & 0x01;
	x1  = (iPLSCode[0] >> 1) & 0x01;
	x2  = (iPLSCode[0] >> 2) & 0x01;
	x3  = (iPLSCode[0] >> 3) & 0x01;
	x4  = (iPLSCode[0] >> 4) & 0x01;
	x5  = (iPLSCode[0] >> 5) & 0x01;
	x6  = (iPLSCode[0] >> 6) & 0x01;
	x7  = (iPLSCode[0] >> 7) & 0x01;
	x8  = (iPLSCode[1] >> 0) & 0x01;
	x9  = (iPLSCode[1] >> 1) & 0x01;
	x10 = (iPLSCode[1] >> 2) & 0x01;
	x11 = (iPLSCode[1] >> 3) & 0x01;
	x12 = (iPLSCode[1] >> 4) & 0x01;
	x13 = (iPLSCode[1] >> 5) & 0x01;
	x14 = (iPLSCode[1] >> 6) & 0x01;
	x15 = (iPLSCode[1] >> 7) & 0x01;
	x16 = (iPLSCode[2] >> 0) & 0x01;
	x17 = (iPLSCode[2] >> 1) & 0x01;

	for(i = ulPLSIndex; i <= ulPLSCode; i ++)
	{
		iPLSCode[0] = (x7  << 7) + ( x6 << 6) + (x5  << 5) + (x4  << 4) + (x3  << 3) + (x2  << 2) + (x1 << 1) + (x0 << 0);
		iPLSCode[1] = (x15 << 7) + (x14 << 6) + (x13 << 5) + (x12 << 4) + (x11 << 3) + (x10 << 2) + (x9 << 1) + (x8 << 0);
		iPLSCode[2] = (x17 << 1) +  x16;

		tmp	 = (x0 ^ x7) & 0x01;
		x0	 = x1;
		x1	 = x2;
		x2	 = x3;
		x3	 = x4;
		x4	 = x5;
		x5	 = x6;
		x6	 = x7;
		x7	 = x8;
		x8	 = x9;
		x9	 = x10;
		x10	 = x11;
		x11	 = x12;
		x12	 = x13;
		x13	 = x14;
		x14	 = x15;
		x15	 = x16;
		x16	 = x17;
		x17	 = tmp;
	}

	pNormalCode[0] = iPLSCode[0];
	pNormalCode[1] = iPLSCode[1];
	pNormalCode[2] = iPLSCode[2];

	return;
}


#if 0
static const struct file_operations fops = {
	.owner			 = 	THIS_MODULE,
	.unlocked_ioctl	 = 	fe_ioctl,
};

static struct miscdevice miscdev = {
	.minor	 = 	MISC_DYNAMIC_MINOR,
	.name	 = 	"tuner",
	.fops	 = 	&fops,
};

static int __init fe_init(void)
{
    int ret = 0;
    feinfo[0].is_attach = 0;
    feinfo[1].is_attach = 0;
    feinfo[2].is_attach = 0;
    mutex_init(&ioctl_lock);
    ret = misc_register(&miscdev);
    if (ret != 0)
    {
        printk(KERN_ERR
            "cannot register miscdev (err=%d)\n", ret);
        return ret;
    }
    return 0;
}
#endif
