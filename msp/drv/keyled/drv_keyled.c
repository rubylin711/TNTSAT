/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <asm/unistd.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
//#include <mach/hardware.h>
#include <asm/signal.h>
#include <linux/time.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/of_platform.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <dt-bindings/clock/mt_clock.h>
#include <linux/input.h>

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include "mt_common.h"
#include "mt_type.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_module.h"
#include "mt_error_mpi.h"
#include "mt_module_debug.h"
#include "mt_osal.h"

#include "mt_unf_keyled.h"
#include "mt_drv_keyled.h"
#include "drv_keyled_ioctl.h"
#include "drv_keyled_priv.h"

#define MAX_MAP_NUM 8
static int flag_keyled_debug = 0;
static u64 g_keymap[MAX_MAP_NUM]={0};

keyled_info_s klinfo;
static struct mutex ioctl_lock;
static struct task_struct *kptask = NULL;
static wait_queue_head_t kpwq;
static keyled_keyfifo_s kvfifo;

static MT_BOOL is_keyled_opened = MT_FALSE;
static atomic_t g_KeyledCount = ATOMIC_INIT(0);
static struct input_dev *virtual_input_dev=NULL;

static struct mt_keyled_device *g_keyled_drv = NULL;

#define keyled_dbg_msg(type, fmt, ...) \
	pr_debug("KEYLED DBG " type " (pid %d): " fmt "\n", current->pid,       \
		 ##__VA_ARGS__)
#define dbg_gen(fmt, ...)   keyled_dbg_msg("keyled", fmt, ##__VA_ARGS__)

static void keyled_keyfifo_init(void)
{
    memset(kvfifo.kv_ringbuf, 0, sizeof(kvfifo.kv_ringbuf));
    kvfifo.count = 0;
    kvfifo.rp = 0;
    kvfifo.wp = 0;
    spin_lock_init(&kvfifo.lock);
}

static int keyled_keyfifo_put(MT_U32 keyval, MT_U32 pressStatus)
{
    spin_lock(&kvfifo.lock);
    if(kvfifo.count < KEYLED_KEYVAL_RINGBUF_SIZE)
    {
        kvfifo.kv_ringbuf[kvfifo.wp].keyVal = keyval;
        kvfifo.kv_ringbuf[kvfifo.wp].pressStatus = pressStatus;
        kvfifo.wp ++;
        kvfifo.wp = kvfifo.wp % KEYLED_KEYVAL_RINGBUF_SIZE;
        kvfifo.count ++;
        spin_unlock(&kvfifo.lock);
        return 0;
    }
    spin_unlock(&kvfifo.lock);
    return -1;
}

static int keyled_keyfifo_get(MT_U32 *keyval, MT_U32 *pressStatus)
{
	if((NULL == keyval) || (NULL== pressStatus))
		return -1;

    spin_lock(&kvfifo.lock);
    if(kvfifo.count > 0 && kvfifo.count <= KEYLED_KEYVAL_RINGBUF_SIZE)
    {
        *keyval = kvfifo.kv_ringbuf[kvfifo.rp].keyVal;
        *pressStatus = kvfifo.kv_ringbuf[kvfifo.rp].pressStatus;
        kvfifo.rp ++;
        kvfifo.rp = kvfifo.rp % KEYLED_KEYVAL_RINGBUF_SIZE;
        kvfifo.count --;
        spin_unlock(&kvfifo.lock);
        return 0;
    }
    spin_unlock(&kvfifo.lock);
    return -1;
}

static int keyprocess(void *data)
{

    MT_U32 kv = 0;
    MT_INFO_KEYLED("keyprocess kthread.\n");
    while(!kthread_should_stop())
    {
        wait_event_interruptible(kpwq,klinfo.is_attach != 0);
        mutex_lock(&ioctl_lock);
        if(klinfo.kl_ops.get_keyval != NULL)
          kv = klinfo.kl_ops.get_keyval();
        mutex_unlock(&ioctl_lock);
        if(0 != kv) {
            dbg_gen("getkey:%x,%x",kv,klinfo.idle_keyval);
        }
        if(kv != klinfo.idle_keyval  &&  kv != 0xFF)
        {
            if(klinfo.key_info.key_status == KEYLED_KEYSTATUS_IDLE)
            {
                klinfo.key_info.key_status = KEYLED_KEYSTATUS_PRESS;
                klinfo.key_info.jiffies = jiffies;
                klinfo.key_info.keyval = kv;
                //put key to fifo, status=press
               // printk("###%s, line[%d],kv=[%d]##\n",__FUNCTION__,__LINE__,kv);

                (void)keyled_keyfifo_put(klinfo.key_info.keyval, MT_UNF_KEY_STATUS_DOWN);
                //if (klinfo.type & MT_UNF_KEYLED_TYPE_KEYADC) 
				{
					int i=0;
					u32 tmp_key,v_key;
                    for(i=0; i<MAX_MAP_NUM; i++) {
						tmp_key = ((g_keymap[i]>>32) &0xffffffff);
						if(0 == tmp_key){
							break;
						}
						if (tmp_key == klinfo.key_info.keyval) {
							v_key = (g_keymap[i]&0xffffffff);
							//input_event(virtual_input_dev, EV_MSC, MSC_SCAN, klinfo.key_info.keyval);
							input_report_key(virtual_input_dev, v_key, 1);//press
							input_report_key(virtual_input_dev, v_key, 0);//relse
							input_sync(virtual_input_dev);
							if (flag_keyled_debug) {
								printk(KERN_EMERG "push %x %x\n",klinfo.key_info.keyval,v_key);
							}
						}
					}
					if (flag_keyled_debug) {
                    	printk(KERN_EMERG "key:%x\n",klinfo.key_info.keyval);
                    }
                }
            }
            else
            {
                if(kv != klinfo.key_info.keyval)
                {
                    klinfo.key_info.jiffies = jiffies;
                    klinfo.key_info.keyval = kv;
                }
                else
                {
                    if(klinfo.repkey_en)
                    {
                        if(time_before((unsigned long)klinfo.key_info.jiffies + (unsigned long)klinfo.repkey_timeout, jiffies))
                        {
                            //put key to fifo, status=hold
                           (void)keyled_keyfifo_put(klinfo.key_info.keyval, MT_UNF_KEY_STATUS_HOLD);
                           klinfo.key_info.jiffies = jiffies;
                        }
                    }
                }
            }
        }
        else
        {
            if(klinfo.key_info.key_status != KEYLED_KEYSTATUS_IDLE)
            {
                if(klinfo.keyup_en)
                {
                    //put key to fifo, status=up
                    (void)keyled_keyfifo_put(klinfo.key_info.keyval, MT_UNF_KEY_STATUS_UP);
                }
                klinfo.key_info.key_status = KEYLED_KEYSTATUS_IDLE;
            }

        }
        set_current_state(TASK_UNINTERRUPTIBLE);
        schedule_timeout(HZ / 10);
    }

    return 0;
}


static long KEYLED_Ioctl(struct file * file, mt_u32 cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	MT_U8 keyled_type = 0;

#ifdef CONFIG_MT_FPGA
	MT_U8 vfd_reset = 0;
#endif

#if defined(CFG_MT_KEYLED_TT1629B_SUPPOR) || defined(CFG_MT_KEYLED_FD650_SUPPORT)
	keyled_display_led_bright_t bright_level = 0;
#endif
#ifdef CFG_MT_KEYLED_PT6393_SUPPORT
	led_special_dis_cfg_t	 vfd_special_char = {0,0};
	vfd_display_char display_char_vfd = {{0},0};
#endif
	keyled_keyval_s keyval = {0, 0};
	//keyled_keyval_char_s keyval_ch;
	MT_U32 ledval = 0;
	keyled_display_char_s ledbuf;
	keyled_display_lbd_s lbdbuf;
	MT_U32 kadctype = 0;
	MT_U32 repkey_timeout = 0;
	MT_U32 repkey_en = 0;
	MT_U32 keyup_en = 0;
	//MT_UNF_GPIOKB_CONFIG_S gpiokb_config;
	keyled_select_type_v2 keyled_type_v2;
	int ret = 0;
	keyled_param_s pos_map_buf;
    int flag_kadc = 0;

    switch (cmd)
	{
        case KEYLED_IOC_SELECT_TYPE:
			if (copy_from_user(&keyled_type, argp, sizeof(keyled_type)) != 0)
			{
				return -EIO;
			}
			mutex_lock(&ioctl_lock);
			memset(&klinfo.kl_ops, 0, sizeof(keyled_ops_s));
            dbg_gen("select type:%x\n",flag_kadc);
			klinfo.is_init = 0;
            flag_kadc = (keyled_type & MT_UNF_KEYLED_TYPE_KEYADC);

			if((keyled_type&0xf) == MT_UNF_KEYLED_TYPE_FD650)
			{
				#ifdef CFG_MT_KEYLED_FD650_SUPPORT
				keyled_fd650_attach(&klinfo, flag_kadc);
				#endif
			}
            else if((keyled_type&0xf) == MT_UNF_KEYLED_TYPE_CT1642)
			{
				#ifdef CFG_MT_KEYLED_CT1642_SUPPORT
				keyled_ct1642_attach(&klinfo, flag_kadc);
				#endif
			}
            else if((keyled_type&0xf) == MT_UNF_KEYLED_TYPE_TT1629B)
			{
				#ifdef CFG_MT_KEYLED_TT1629B_SUPPORT
				keyled_tt1629b_attach(&klinfo, flag_kadc);
				#endif
			}
            else if((keyled_type&0xf) == MT_UNF_KEYLED_TYPE_PT6393)
			{
				#ifdef CFG_MT_KEYLED_PT6393_SUPPORT
				keyled_pt6393_attach(&klinfo, flag_kadc);
				#endif
			}
            else if((keyled_type&0xf) == MT_UNF_KEYLED_TYPE_FD612)
			{
				#ifdef CFG_MT_KEYLED_FD612_SUPPORT
				keyled_fd612_attach(&klinfo, flag_kadc);
				#endif
			}
            else if(keyled_type & MT_UNF_KEYLED_TYPE_KEYADC)
			{
				#ifdef CFG_MT_KEYLED_ADC_SUPPORT
				keyled_keyadc_attach(&klinfo);
				#endif
			}
			else
			{
			    klinfo.is_attach = 0;
			    ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
			    wake_up(&kpwq);
			    goto ioctl_error;
			}
			if(klinfo.kl_ops.keyled_init)
			{
				klinfo.kl_ops.keyled_init(g_keyled_drv);
			}
			else
			{
				ret = -EPERM;
                goto ioctl_error;
			}
			klinfo.is_init = 1;
			wake_up(&kpwq);
			mutex_unlock(&ioctl_lock);
            break;

		case KEYLED_IOC_SELECT_TYPE_V2://also set keyadc type in this interface
			if (copy_from_user(&keyled_type_v2, argp, sizeof(keyled_select_type_v2)) != 0)
			{
				return -EIO;
			}
			mutex_lock(&ioctl_lock);
			memset(&klinfo.kl_ops, 0, sizeof(keyled_ops_s));
			//printk("[%s %d]keyled_type_v2.keyled_type=%d, keyled_type_v2.kadc_type=%d\n", __FUNCTION__, __LINE__, keyled_type_v2.keyled_type, keyled_type_v2.kadc_type);
			klinfo.is_init = 0;
			flag_kadc = (keyled_type_v2.keyled_type & MT_UNF_KEYLED_TYPE_KEYADC);
            if (flag_kadc) {
                klinfo.kadc_type = keyled_type_v2.kadc_type;
            }
            dbg_gen("select type2:%x %x\n",keyled_type_v2.keyled_type,keyled_type_v2.kadc_type);
            if((keyled_type_v2.keyled_type&0xf) == MT_UNF_KEYLED_TYPE_FD650)
			{
				#ifdef CFG_MT_KEYLED_FD650_SUPPORT
				keyled_fd650_attach(&klinfo, flag_kadc);
				#endif
			}
            else if((keyled_type_v2.keyled_type&0xf) == MT_UNF_KEYLED_TYPE_CT1642)
			{
				#ifdef CFG_MT_KEYLED_CT1642_SUPPORT
				keyled_ct1642_attach(&klinfo, flag_kadc);
				#endif
			}
            else if((keyled_type_v2.keyled_type&0xf) == MT_UNF_KEYLED_TYPE_TT1629B)
			{
				#ifdef CFG_MT_KEYLED_TT1629B_SUPPORT
				keyled_tt1629b_attach(&klinfo, flag_kadc);
				#endif
			}
            else if((keyled_type_v2.keyled_type&0xf) == MT_UNF_KEYLED_TYPE_PT6393)
			{
				#ifdef CFG_MT_KEYLED_PT6393_SUPPORT
				keyled_pt6393_attach(&klinfo, flag_kadc);
				#endif
			}
            else if((keyled_type_v2.keyled_type&0xf) == MT_UNF_KEYLED_TYPE_FD612)
			{
				#ifdef CFG_MT_KEYLED_FD612_SUPPORT
				keyled_fd612_attach(&klinfo, flag_kadc);
				#endif
			}
            else if(keyled_type_v2.keyled_type & MT_UNF_KEYLED_TYPE_KEYADC)
			{
				#ifdef CFG_MT_KEYLED_ADC_SUPPORT
				keyled_keyadc_attach(&klinfo);
				#endif
			}
			else
			{
			    klinfo.is_attach = 0;
			    ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
			    wake_up(&kpwq);
			    goto ioctl_error;
			}
			if(klinfo.kl_ops.keyled_init)
			{
				klinfo.kl_ops.keyled_init(g_keyled_drv);
			}
			else
			{
				ret = -EPERM;
                goto ioctl_error;
			}
			klinfo.is_init = 1;
			wake_up(&kpwq);
			mutex_unlock(&ioctl_lock);
            break;

		case KEYLED_IOC_HW_INIT:
			mutex_lock(&ioctl_lock);
			//printk("[%s %d]KEYLED_IOC_INIT\n", __FUNCTION__, __LINE__);
			klinfo.is_init = 0;
			if (NULL != klinfo.kl_ops.keyled_init)
			{
				klinfo.kl_ops.keyled_init(g_keyled_drv);
			}
			else
			{
				ret = -EPERM;
                goto ioctl_error;
			}
			klinfo.is_init = 1;
			wake_up(&kpwq);
			mutex_unlock(&ioctl_lock);
            break;

        case KEYLED_IOC_GET_VALUE:
            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }

            if(keyled_keyfifo_get(&keyval.keyVal, &keyval.pressStatus) < 0)
            {
                ret = MT_UNF_KEYLED_ERR_NO_KEYVAL;
                goto ioctl_error;
            }
            mutex_unlock(&ioctl_lock);

            if (copy_to_user(argp, &keyval, sizeof(keyval)) != 0)
            {
            	return -EIO;
            }

            break;

        case KEYLED_IOC_SET_REPKEY_TIMEOUT:
            if (copy_from_user(&repkey_timeout, argp, sizeof(repkey_timeout)) != 0)
            {
            	return -EIO;
            }

            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }

            klinfo.repkey_timeout = (repkey_timeout * HZ) / 1000;
            if((repkey_timeout * HZ) % 1000 > 0)
            {
                klinfo.repkey_timeout ++;
            }
            mutex_unlock(&ioctl_lock);
			break;

        case KEYLED_IOC_ENABLE_REPKEY:
            if (copy_from_user(&repkey_en, argp, sizeof(repkey_en)) != 0)
            {
            	return -EIO;
            }

            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }
            klinfo.repkey_en = repkey_en;
            mutex_unlock(&ioctl_lock);
            break;

        case KEYLED_IOC_ENABLE_KEYUP:
            if (copy_from_user(&keyup_en, argp, sizeof(keyup_en)) != 0)
            {
            	return -EIO;
            }

            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }
            klinfo.keyup_en = keyup_en;
            mutex_unlock(&ioctl_lock);
            break;

        case KEYLED_IOC_DISPLAY:
            if (copy_from_user(&ledval, argp, sizeof(ledval)) != 0)
            {
            	return -EIO;
            }

            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }
			if(klinfo.kl_ops.display)
	   		{
	   			klinfo.kl_ops.display((ulong)ledval);
			}
			else
			{
				ret = -EPERM;
                goto ioctl_error;
			}
            mutex_unlock(&ioctl_lock);
            break;

		case KEYLED_IOC_DISPLAY_ASC:
			memset(&ledbuf, 0x00, sizeof(keyled_display_char_s));
            if (copy_from_user(&ledbuf, argp, sizeof(keyled_display_char_s)) != 0)
            {
            	return -EIO;
            }

            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }
			if(klinfo.kl_ops.display_asc)
	   		{
	   			klinfo.kl_ops.display_asc((ulong)(&ledbuf));
			}
			else
			{
				ret = -EPERM;
                goto ioctl_error;
			}
            mutex_unlock(&ioctl_lock);
            break;

		case KEYLED_IOC_DISPLAY_LBD:
			memset(&lbdbuf, 0x00, sizeof(keyled_display_lbd_s));
            if (copy_from_user(&lbdbuf, argp, sizeof(keyled_display_lbd_s)) != 0)
            {
            	return -EIO;
            }

            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }
			if(klinfo.kl_ops.display_lbd)
	   		{
	   			klinfo.kl_ops.display_lbd((ulong)(&lbdbuf));
			}
			else
			{
				ret = -EPERM;
                goto ioctl_error;
			}
            mutex_unlock(&ioctl_lock);
            break;

        case KEYLED_IOC_SET_KADC_TYPE:
            if (copy_from_user(&kadctype, argp, sizeof(kadctype)) != 0)
            {
            	return -EIO;
            }
            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }
			klinfo.kadc_type = kadctype;
            mutex_unlock(&ioctl_lock);
            break;

		case KEYLED_IOC_SET_BRIGHT_LEVEL:
#if defined(CFG_MT_KEYLED_TT1629B_SUPPOR) || defined(CFG_MT_KEYLED_FD650_SUPPORT)
            if (copy_from_user(&bright_level, argp, sizeof(bright_level)) != 0)
            {
            	return -EIO;
            }
            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }
            if(klinfo.kl_ops.set_brightness)
            {
            	klinfo.kl_ops.set_brightness(bright_level);
            }
			else
			{
				ret = -EPERM;
                goto ioctl_error;
			}
            mutex_unlock(&ioctl_lock);
#endif
            break;

		case KEYLED_IOC_SET_LED_POS:
			memset(&pos_map_buf, 0x00, sizeof(keyled_param_s));
            if (copy_from_user(&pos_map_buf, argp, sizeof(keyled_param_s)) != 0)
            {
            	return -EIO;
            }

            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }
			if(klinfo.kl_ops.set_led_pos)
	   		{
	   			klinfo.kl_ops.set_led_pos(pos_map_buf);
			}
			else
			{
				ret = -EPERM;
                goto ioctl_error;
			}
            mutex_unlock(&ioctl_lock);
            break;

		case KEYLED_IOC_SET_LED_MAP:
			memset(&pos_map_buf, 0x00, sizeof(keyled_param_s));
            if (copy_from_user(&pos_map_buf, argp, sizeof(keyled_param_s)) != 0)
            {
            	return -EIO;
            }

            mutex_lock(&ioctl_lock);
            if(klinfo.is_attach == 0)
            {
                ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
                goto ioctl_error;
            }
			if(klinfo.kl_ops.set_led_map)
	   		{
	   			klinfo.kl_ops.set_led_map(pos_map_buf);
			}
			else
			{
				ret = -EPERM;
                goto ioctl_error;
			}
            mutex_unlock(&ioctl_lock);
			break;
#ifdef CFG_MT_KEYLED_PT6393_SUPPORT
		case KEYLED_IOC_VFD_SPECIAL_CHAR_DISPLAY:
			if (copy_from_user(&vfd_special_char, argp, sizeof(led_special_dis_cfg_t)) != 0)
			{
				return -EIO;
			}
			mutex_lock(&ioctl_lock);
			if(klinfo.is_attach == 0)
			{
				ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
				goto ioctl_error;
			}
			if(klinfo.kl_ops.display_special_ch)
			{
				klinfo.kl_ops.display_special_ch((ulong)&vfd_special_char);
			}
			else
			{
				ret = -EPERM;
				goto ioctl_error;
			}
			mutex_unlock(&ioctl_lock);
			break;
		case KEYLED_IOC_VFD_DISPLAY:
			if (copy_from_user(&display_char_vfd, argp, sizeof(vfd_display_char)) != 0)
			{
				return -EIO;
			}
			mutex_lock(&ioctl_lock);
			if(klinfo.is_attach == 0)
			{
				ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
				goto ioctl_error;
			}
			if(klinfo.kl_ops.vfd_display_ch)
			{
				klinfo.kl_ops.vfd_display_ch((ulong)&display_char_vfd);
			}
			else
			{
				ret = -EPERM;
				goto ioctl_error;
			}
			mutex_unlock(&ioctl_lock);
			break;
#endif

#ifdef CONFIG_MT_FPGA
		case KEYLED_IOC_VFD_RESET:
			if (copy_from_user(&vfd_reset, argp, sizeof(vfd_reset)) != 0)
			{
				return -EIO;
			}
			mutex_lock(&ioctl_lock);
			if(klinfo.is_attach == 0)
			{
				ret = MT_UNF_KEYLED_ERR_TYPE_UNSUPPORTED;
				goto ioctl_error;
			}
			if(klinfo.kl_ops.vfd_reset)
			{
				klinfo.kl_ops.vfd_reset();
			}
			else
			{
				ret = -EPERM;
				goto ioctl_error;
			}
			mutex_unlock(&ioctl_lock);

			break;
#endif

        default:
            return -ENOTTY;
	}

    return 0;
 ioctl_error:
    mutex_unlock(&ioctl_lock);
    return ret;
}

static mt_s32 KEYLED_Open(struct inode *inode, struct file *filp)
{
	if (1 == atomic_inc_return(&g_KeyledCount))
	{
		if(MT_FALSE == is_keyled_opened)
		{
			klinfo.keyup_en = 0;
			klinfo.repkey_en = 0;
			klinfo.repkey_timeout = 0;
			klinfo.key_info.jiffies = jiffies;
			klinfo.key_info.key_status = KEYLED_KEYSTATUS_IDLE;
			klinfo.key_info.keyval = 0;

			mutex_init(&ioctl_lock);
			init_waitqueue_head(&kpwq);
			keyled_keyfifo_init();
			kptask = kthread_run(keyprocess, NULL, "keyled keyprocess");
			if (IS_ERR(kptask))
				return PTR_ERR(kptask);

			is_keyled_opened = MT_TRUE;
		}
	}
	return MT_SUCCESS;
}

static mt_s32 KEYLED_Release(struct inode *inode, struct file *filp)
{
	#if 0
	struct irq_desc *desc = NULL;

	desc = irq_to_desc(IRQ_LEDKB_ID);
	if(desc)
	{
		free_irq(IRQ_LEDKB_ID, MT_NULL);
	}
	#endif

	if (atomic_dec_and_test(&g_KeyledCount))
	{
		if(NULL == kptask)
			return MT_SUCCESS;

		if (!IS_ERR(kptask))
		{
			kthread_stop(kptask);
			kptask = NULL;
		}

		if(klinfo.kl_ops.release != NULL)
		{
			klinfo.kl_ops.release();
		}
		klinfo.is_attach = 0;
		klinfo.is_init = 0;
		is_keyled_opened = MT_FALSE;
	}
	return MT_SUCCESS;
}

static struct file_operations KEYLED_FOPS =
{
    open :KEYLED_Open,
    release:KEYLED_Release,
    unlocked_ioctl:KEYLED_Ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = KEYLED_Ioctl,
#endif
};


static int symphony_keyled_suspend (struct platform_device *pdev, pm_message_t stState)
{
    //printk(KERN_EMERG "keyled suspend.................\n");
	return 0;
}

static int symphony_keyled_resume(struct platform_device *pdev)
{
    //printk("keyled resume in.................\n");
    if(klinfo.kl_ops.keyled_init)
    {
		klinfo.kl_ops.keyled_init(g_keyled_drv);
        printk("keyled resume ok\n");
    }
	return 0;
}

static int symphony_keyled_remove(struct platform_device *pdev)
{
	struct mt_keyled_device *keyleddev = NULL;

	keyleddev = platform_get_drvdata(pdev);
	if (keyleddev) {

		cdev_del(&keyleddev->cdev);
		kfree(keyleddev);
	}

	return 0;
}

static ssize_t symphony_keyled_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	// Code to read the attribute value and store it in the buf
	return 0;
}

static ssize_t symphony_keyled_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	// Code to write the attribute value from buf
	return 0;
}

static DEVICE_ATTR(keyled, 0664, symphony_keyled_show, symphony_keyled_store);

static int symphony_keyled_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct mt_keyled_device *keyleddev;
	mt_proc_entry_t *pProcItem;
	struct resource *res;
	struct clk	*clk_devm = NULL;
	struct clk	*clk_kadc = NULL;
	struct device_node *of_node = NULL;
    u32 clk_khz = 0;

	keyleddev = kzalloc(sizeof(struct mt_keyled_device), GFP_KERNEL);
	if (NULL == keyleddev) {
		ret = -ENOMEM;
		goto fail_keyled_kzalloc;
	}

	keyleddev->keyled_class = class_create(UMAP_DEVNAME_KEYLED);
	if (IS_ERR(keyleddev->keyled_class)) {
		ret = PTR_ERR(keyleddev->keyled_class);
		goto fail_keyled_class;
	}

	keyleddev->minor = UMAP_MIN_MINOR_KEYLED;
	keyleddev->minors = UMAP_DEV_NUM_KEYLED;
	keyleddev->devt = MKDEV(MT_DEVICE_MAJOR, keyleddev->minor);
	keyleddev->major = MAJOR(keyleddev->devt);
	keyleddev->dev = &pdev->dev;

	cdev_init(&keyleddev->cdev, &KEYLED_FOPS);
	keyleddev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&keyleddev->cdev, keyleddev->devt, keyleddev->minors);
	if (ret) {
		ret = -EINVAL;
		goto fail_keyled_cdev;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {
		keyleddev->ledkb_dev.base = (void *)SYMPHONY_IO_VA(res->start);
		//printk("[%s_%d]start = 0x%lx\n", __func__, __LINE__, (ulong)res->start);
		//printk("[%s_%d]end = 0x%lx\n", __func__, __LINE__, (ulong)res->end);
	}
	else
	{
		keyleddev->ledkb_dev.base = (void *)SYMPHONY_IO_VA(0xbf154000);
	}

	//printk("\nledkb, %s.%d remapped=%p\n",__FUNCTION__,__LINE__,keyleddev->ledkb_dev.base);

	keyleddev->ledkb_dev.clk_khz = 200;
	clk_devm = devm_clk_get(keyleddev->dev, "ledkb_clk");
    if (!IS_ERR(clk_devm)) {
        clk_prepare_enable(clk_devm);       //sure enable
		clk_khz = clk_get_rate(clk_devm);
    }

    clk_kadc = devm_clk_get(keyleddev->dev, "ao_kadc_clk");
    if (!IS_ERR(clk_kadc)) {
        clk_prepare_enable(clk_kadc);       //sure enable
    }

	if (0 == clk_khz)
	{
		of_node = pdev->dev.of_node;
		if(of_node)
		{
			ret = of_property_read_u32(of_node,"freq",&clk_khz);
			if (0 == ret)
			{
				printk("\nledkb, %s.%d clk_khz:%d\n",__FUNCTION__,__LINE__,clk_khz);
				keyleddev->ledkb_dev.clk_khz = clk_khz;
			}
		}
	}

	keyleddev->dev = device_create(keyleddev->keyled_class, NULL, keyleddev->devt, NULL, UMAP_DEVNAME_KEYLED);
	if (IS_ERR(keyleddev->dev)) {
		ret = PTR_ERR(keyleddev->dev);
		printk("keyled device_create failed. ret = %d\n",ret);
		goto fail_keyled_device;
	}

	if (device_create_file(&pdev->dev, &dev_attr_keyled)) {
		ret = -ENOENT;
		printk("keyled device_create_file failed.\n");
		goto fail_keyled_create_file;
	}

	g_keyled_drv = keyleddev;

	pProcItem = mt_drv_proc_add_module(MT_MOD_KEYLED, NULL, NULL);
	if (!pProcItem)
	{
		printk("keyled add proc failed.\n");
		ret = -1;
		g_keyled_drv = NULL;
		goto fail_keyled_create_file;
	}

	pProcItem->read = NULL;
	pProcItem->write = NULL;

	platform_set_drvdata(pdev, keyleddev);
	dev_set_drvdata(keyleddev->dev, keyleddev);

	return ret;

fail_keyled_create_file:
	device_destroy(keyleddev->keyled_class, keyleddev->devt);
fail_keyled_device:
	cdev_del(&keyleddev->cdev);
fail_keyled_cdev:
	class_destroy(keyleddev->keyled_class);
fail_keyled_class:
	kfree(keyleddev);
fail_keyled_kzalloc:
	return ret;
}

#if defined(CONFIG_OF)
	 static const struct of_device_id symphony_keyled_of_match[] = {
		 { .compatible = "montage,keyled" },
		 {},
	 };
MODULE_DEVICE_TABLE(of, symphony_keyled_of_match);
#endif

static struct platform_driver symphony_keyled_driver = {
	.probe 		= symphony_keyled_probe,
	.remove		= symphony_keyled_remove,
	.suspend	= symphony_keyled_suspend,
	.resume		= symphony_keyled_resume,
	.driver	= {
		.name = UMAP_DEVNAME_KEYLED,
		.of_match_table = of_match_ptr(symphony_keyled_of_match),
	},
};

static char * simple_seektonextchar(char *s)
{
    if(*s != 0) {
        while(' ' != *s) {
            s++;
        }
    }
    if(*s != 0) {
        while(' ' == *s) {
            s++;
        }
    }
    return s;
}

static int keyled_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "keyled: vkey.m%d.u%d.d%d.l%d.r%d.o%d.p%d\n",
					KEY_MENU,KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_OK,KEY_POWER);
	seq_printf(m, "echo \"debugon 1\" >/proc/keyled,  to open print\n");
	seq_printf(m, "echo \"map key vkey\" >/proc/keyled, add map\n");

    return 0;
}

static ssize_t keyled_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos)
{
    char cmd[256]= {0};
    char *g=cmd;

    if (copy_from_user(&cmd,buffer,count)) {
        return -EFAULT;
    }
    printk(KERN_EMERG "cmd=%s\n",cmd);

	switch (cmd[0]){
		case 'd':
			if('e'==cmd[1] && 'b'==cmd[2] && 'u'==cmd[3]){
				ulong onoff = 0;
				if(g){
					g=simple_seektonextchar(g);
					onoff=simple_strtoul(g, &g, 0);
				}
				if(onoff){
					flag_keyled_debug = 1;
				}else{
					flag_keyled_debug = 0;
				}
			}
		break;
		case 'm':
			if('a'==cmd[1] && 'p'==cmd[2]){
				int i;
				u32 tmp_key;
				ulong key = 0,vkey = 0;
				if(g){
					g=simple_seektonextchar(g);
					key=simple_strtoul(g, &g, 0);
				}
				if(g){
					g=simple_seektonextchar(g);
					vkey=simple_strtoul(g, &g, 0);
				}
				if(0 == key){
					break;
				}
				for(i=0; i<MAX_MAP_NUM; i++) {
					tmp_key = ((g_keymap[i]>>32)&0xffffffff);
					if((tmp_key == key) || (0==tmp_key)){
						g_keymap[i] = key;
						g_keymap[i] = (g_keymap[i]<<32);
						g_keymap[i] |= vkey;
						//printk("modify [%d] = %llx\n",i, g_keymap[i]);
						break;
					}
				}
			}
		break;

		default:
			break;
	}

    return count;
}

static int keyled_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, keyled_proc_show, NULL);
}

static const struct proc_ops keyled_proc_ops = {
    .proc_open		= keyled_proc_open,
    .proc_read		= seq_read,
    .proc_lseek		= seq_lseek,
    .proc_write     = keyled_proc_write,
    .proc_release	= single_release,
};
static struct proc_dir_entry *proc_keyled=NULL;

mt_s32 __init KEYLED_DRV_ModInit(mt_void)
{
	int ret = platform_driver_register(&symphony_keyled_driver);
	if (ret) {
		return ret;
	}
	{
		int err = 0;
		memset(g_keymap, 0, sizeof(g_keymap));
		virtual_input_dev = input_allocate_device();
		if (!virtual_input_dev) {
			pr_err("Failed to allocate input device\n");
			return -ENOMEM;
		}

		virtual_input_dev->name = "Front Panel Input Device";
		virtual_input_dev->phys = "frontpl/input0";
		virtual_input_dev->id.bustype = BUS_HOST;
		virtual_input_dev->id.vendor  = 0x1001;
		virtual_input_dev->id.product = 0x1001;
		virtual_input_dev->id.version = 0x0001;
		virtual_input_dev->evbit[0] = BIT_MASK(EV_KEY);
        __set_bit(EV_KEY, virtual_input_dev->evbit);
        __set_bit(KEY_MENU, virtual_input_dev->keybit);
        __set_bit(KEY_UP, virtual_input_dev->keybit);
        __set_bit(KEY_DOWN, virtual_input_dev->keybit);
        __set_bit(KEY_LEFT, virtual_input_dev->keybit);
        __set_bit(KEY_RIGHT, virtual_input_dev->keybit);
        __set_bit(KEY_OK, virtual_input_dev->keybit);
        __set_bit(KEY_POWER, virtual_input_dev->keybit);
		err = input_register_device(virtual_input_dev);
		if (err) {
			pr_err("Failed to register input device\n");
			input_free_device(virtual_input_dev);
			return err;
		}
	}
	proc_keyled = proc_create("mtkeyled", 0644, NULL, &keyled_proc_ops);

	return 0;
}

mt_void __exit KEYLED_DRV_ModExit(mt_void)
{
	remove_proc_entry("mtkeyled", NULL);

    if (NULL != virtual_input_dev) {
        pr_err("unregister input device\n");
        input_unregister_device(virtual_input_dev);
        virtual_input_dev = NULL;
    }

	platform_driver_unregister(&symphony_keyled_driver);
}

