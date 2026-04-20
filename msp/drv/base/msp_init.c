#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>

#if defined(CONFIG_TEE)
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#endif

#include "mt_type.h"
#include "mt_drv_log.h"
#include "mt_debug.h"
#include "mt_drv_init.h"

static mt_s32 __init msp_drv_modules_init(mt_void)
{
	MT_INFO_LOG(" msp modules drv inits \n");

	system_verify_init();
	HDMI_DRV_ModInit(); //gavin remove tmp, need check compile issues
	sci_drv_modinit();
	keytable_setup();
	DMX_DRV_ModInit();
	//TODO:

	VDP_DRV_ModInit();
	PQ_DRV_ModInit();
	i2c_drv_modinit();
	otp_modinit();
	AVPLAY_DRV_ModInit();
	SYNC_DRV_ModInit();
	VDEC_DRV_ModInit();
	AIAO_DRV_ModInit();
	//ADEC_DRV_ModInit();
	MTFB_DRV_ModInit();
	drv_osd_layer_ModInit();
	dmac_modinit();
	spdma_drv_modinit();
	JPEG_DRV_ModInit();
	PNG_DRV_ModInit();
	//IR_DRV_ModInit();		//no need, use linux standard ir now
	fe_drv_modinit();
	gpio_modinit();
	STANDBY_DRV_ModInit();  //gavin remove tmp, need check compile issues
	KEYLED_DRV_ModInit();
#if !defined(CONFIG_MT_FPGA)
#if !defined(CONFIG_TEE)
#if defined(CONFIG_MT_AVPLAY) && defined(CONFIG_MT_ADEC)	/*!loader*/
	ipc_modinit();
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	rpc_modinit();
#endif
#endif
#endif
#endif
	adec_drv_init();
	PVR_DRV_ModInit();
	ipcs_setup();
	vss_setup();
	cipher_scpu_init();
	keyladder_init();
	crypto_engine_setup();
	cert_drv_init();
	
#if !defined(CONFIG_MT_FPGA)
#if !defined(CONFIG_TEE)
#if defined(CONFIG_MT_AVPLAY) && defined(CONFIG_MT_ADEC)	/*!loader*/
	mt_ampshm_init();
#endif
#endif
#endif
	mtimer_drv_modinit();
	TDE_DRV_ModInit();
	wdg_drv_modinit();
	mtest_drv_init();
	VPSS_DRV_ModInit();
	misc_drv_modinit();
	always_timer_drv_modinit();
	lxc_ipc_drv_modinit();
	SUPLAYER_DRV_ModInit();
	mt_analog_mod_init();
	mt_crm_mod_init();
	mt_pinctrl_mod_init();
	drv_snd_ao_module_init();

	chipverify_mss_drv_init();

	mt_opc_setup();

	return MT_SUCCESS;
}

static void __exit msp_drv_modules_exit(void)
{
	//TODO, call modules exit api
	//mt_rng_drv_modexit();
	return;
}

//For TEE solution, some modules could only
//init by user after TEE(and/or AVCPU) startup.
#if defined(CONFIG_TEE)
static atomic_t mod_user_init_atomic = ATOMIC_INIT(-1);
static struct work_struct mod_user_init_work;

static atomic_t mod_user_init_work_atomic = ATOMIC_INIT(0);
static wait_queue_head_t mod_user_init_waitque;

static void modules_user_init_work(struct work_struct *work_arg)
{
	MT_INFO_LOG(" msp modules drv user init work\n");

#if defined(CONFIG_MT_AVPLAY) && defined(CONFIG_MT_ADEC)	/*!loader*/
	ipc_modinit();
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	rpc_modinit();
#endif
	mt_ampshm_init();
#endif

	atomic_dec(&mod_user_init_work_atomic);
	wake_up_interruptible(&mod_user_init_waitque);

	return;
}

void msp_drv_modules_user_init(void)
{
	if (atomic_inc_and_test(&mod_user_init_atomic))
	{
		MT_INFO_LOG(" msp modules drv user init \n");

		init_waitqueue_head(&mod_user_init_waitque);

		//NOTE: it will block mt_sys_init if call modules_user_init_work directly!
		INIT_WORK(&mod_user_init_work, modules_user_init_work);
		schedule_work(&mod_user_init_work);

		//wait modules_user_init_work done
		atomic_inc(&mod_user_init_work_atomic);
		(void)wait_event_interruptible_hrtimeout(mod_user_init_waitque,
												(atomic_read(&mod_user_init_work_atomic) == 0),
												ms_to_ktime(1000));
	}

	return;
}

EXPORT_SYMBOL(msp_drv_modules_user_init);
#endif

module_init(msp_drv_modules_init);
module_exit(msp_drv_modules_exit);

MODULE_AUTHOR("MONTAGE");
MODULE_LICENSE("GPL");

