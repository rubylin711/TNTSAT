/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <uapi/linux/sched/types.h>
//#include "mt_hdmi20.h"
#include "hdmi20_priv.h"
#include "mt_drv_mmz.h"
#include "mt_common.h"
#include "mt_drv_dev.h"
#include "mt_drv_struct.h"
#include "drv_hdmi_ioctl.h"
#include "drv_global.h"
#include "drv_hdmi_ext.h"
#include "mt_drv_module.h"
#include "si_drv_tx_regs.h"
#include "drv_reg_proc.h"
#if (defined(CONFIG_MT_HDMI_V20_DEBUG_PROC_FS) || defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS))
	#include "mt_osal.h"
	#include "si_misc.h"
	#include "si_hdmitx.h"
	#include "mt_drv_proc.h"
	#include "drv_compatibility.h"
	#include "si_vidpath_regs.h"
#endif

//#include "mt_analog_reg.h"

#define HDMI20_MINOR   UMAP_MIN_MINOR_HDMI20
#define HDMI20_MINORS  UMAP_DEV_NUM_HDMI20 //#define HDMI20_MINORS UMAP_MAX_MINOR_HDMI20
#define HDMI20_BUFFERS_PER_THREAD 1024

#define HDMI20_USE_INTERRUPT 0        //etude2 hdmi intr always high, hang up.
#define HDMI_NAME                      "MT_HDMI"
#define HDMI20_IO_MEM_MAX		MT_RELATED_MODE_NUM
#define HDMI20_EMP_BUFF_SIZE		(2<<10)
#define HDMI20_EMP_BUFF_NUM	(2)

struct hdmi20_driver {
	struct cdev cdev;
	struct hdmi20_device *hdmi20_dev[HDMI20_MINORS];
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

struct hdmi20_sub_mod {
	unsigned char __iomem*  membase;
	resource_size_t		mapbase;
	resource_size_t		mapsize;
};

struct hdmi20_device {
	int	valid;
	struct list_head list_name;
	struct platform_device *pdev;
	dev_t devt;
	struct mutex mutex_hdmi;
	//	wait_queue_head_t waitqueue;
	struct device *p_dev;
	struct device *dev;
	int open_cnt;
	struct hdmi20_sub_mod regDig[HDMI20_IO_MEM_MAX];
	unsigned int irq;
	struct hdmi20_params params_in;
	mmz_buffer_s emp_buf[HDMI20_EMP_BUFF_NUM];
};

struct hdmi20_buffer {
	mmz_buffer_s in_sMBuf;
	mmz_buffer_s out_sMBuf;
	int buf_cnt;
	struct completion done;
	struct completion call;
	int do_what;
	int connected;
	bool in_buf_need;
	bool out_buf_need;
};

struct hdmi20_thread {
	struct list_head list;	/* join into list_thread of hdmi20_name  */
	struct hdmi20_buffer *buffers[HDMI20_BUFFERS_PER_THREAD];
	int tid_cnt;
	pid_t tid;
};

struct hdmi20_name {
	struct list_head list;	/* join into list_name of hdmi20_device  */
	struct list_head list_thread;
	int name_cnt;
	struct completion attach;
	char name[HDMI20_NAME_SIZE];
};

int g_hdmi_dbg_en = 0;
static atomic_t hdmi_open_cnt_atomic = ATOMIC_INIT(0);
extern ulong g_ProcHandle;
extern MT_U32 HDMIStandbySetupFlag;
extern unsigned int suspend_flag;
extern unsigned int start_flag;
module_param_named(g_hdmi_dbg_en, g_hdmi_dbg_en, int, 0644);
EXPORT_SYMBOL(g_hdmi_dbg_en);

static struct hdmi20_driver *hdmi20_drv;
static struct class *hdmi20_class;

extern mt_s32 hdmi_Ioctl(struct inode *inode, struct file *file, unsigned int cmd, mt_void *arg);
extern void HdmiDrvRegBaseSet(void *p, ulong vAddr, HDMI_RELATED_REG_MOD_E mod);
extern mt_s32 HDMI_DRV_Init(mt_void);
extern mt_void  HDMI_DRV_EXIT(mt_void);
extern mt_s32 MT_DRV_HDMI_Video_Cofig(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param);
extern mt_s32 MT_DRV_HDMI_Notify_register(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param);
extern mt_s32 MT_DRV_HDMI_Av_Mute(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param);
extern mt_s32 MT_DRV_HDMI_Clk_Cfg(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param);
extern mt_s32 MT_DRV_HDMI_EMP_Cfg(MT_UNF_HDMI_ID_E enHdmi, void *p_param, mt_u32 len);
extern mt_s32 MT_DRV_HDMI_Vcfg_Check(MT_UNF_HDMI_ID_E enHdmi, void *param_in, void *param_out);
extern mt_void HDMI_PinConfig(mt_void);
extern mt_s32 hdmi_Suspend(basedev_s *pdev, pm_message_t state);
extern mt_s32 hdmi_Resume(basedev_s *pdev);
static int hdmi20_resume(struct platform_device *pdev);
struct hdmi20_params *hdmi20_params_get(void);

#if defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS)
	static ssize_t hdmi20_show(struct device *dev, struct device_attribute *attr, char *buffer);
	static ssize_t hdmi20_store(struct device *dev, struct device_attribute *attr, const char *buffer, size_t count);
	static ssize_t sink_show(struct device *dev, struct device_attribute *attr, char *buffer);
#endif

#if (defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS) || defined(CONFIG_MT_HDMI_V20_DEBUG_PROC_FS))
	static MT_S32 HDMI0_Proc_Common(char *p, MT_VOID *v, MT_U32 isproc);
	static MT_S32 HDMI0_Sink_Proc_Common(char *p, MT_VOID *v, MT_U32 isproc);
#endif
static HDMI_EXPORT_FUNC_S s_stHdmiExportFuncs = {
	.pfnHdmiInit = MT_DRV_HDMI_Init,
	.pfnHdmiDeinit = MT_DRV_HDMI_Deinit,
	.pfnHdmiOpen = MT_DRV_HDMI_Open,
	.pfnHdmiClose = MT_DRV_HDMI_Close,
	.pfnHdmiGetPlayStus = MT_DRV_HDMI_PlayStus,
	.pfnHdmiGetAoAttr = MT_DRV_AO_HDMI_GetAttr,
	.pfnHdmiGetSinkCapability = MT_DRV_HDMI_GetSinkCapability,
	.pfnHdmiGetAudioCapability = MT_DRV_HDMI_GetAudioCapability,
	.pfnHdmiSetAudioMute = MT_DRV_HDMI_SetAudioMute,
	.pfnHdmiSetAudioUnMute = MT_DRV_HDMI_SetAudioUnMute,
	.pfnHdmiAudioChange = MT_DRV_HDMI_AudioChange,
	.pfnHdmiPreFormat = MT_DRV_HDMI_PreFormat,
	.pfnHdmiSetFormat = MT_DRV_HDMI_SetFormat,
	.pfnHdmiDetach = MT_DRV_HDMI_Detach,
	.pfnHdmiAttach = MT_DRV_HDMI_Attach,
	.pfnHdmiResume = NULL,//hdmi20_resume,
	.pfnHdmiSuspend = NULL,//hdmi20_suspend,
	.pfnHdmiSoftResume = NULL,//hdmi_SoftResume,
	.pfnHdmiConfigVid = MT_DRV_HDMI_Video_Cofig,
	.pfnHdmiNotifyRegister = MT_DRV_HDMI_Notify_register,
	.pfnHdmiAvMute = MT_DRV_HDMI_Av_Mute,
	.pfnHdmiClkCfg = MT_DRV_HDMI_Clk_Cfg,
	.pfnHdmiEmpCfg = MT_DRV_HDMI_EMP_Cfg,
	.pfnHdmiVcfgCheck = MT_DRV_HDMI_Vcfg_Check,
};
static void hdmi20_ana_reg_init(struct hdmi20_device *hdmi20_dev);
#if defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS)
	static DEVICE_ATTR_RW(hdmi20);
	static DEVICE_ATTR_RO(sink);
#endif
static int hdmi20_open(struct inode *inode, struct file *file)
{
	struct hdmi20_device *hdmi20_dev;
	int idx = 0;
	int ret = MT_SUCCESS;

	HDMI20_PRINT_FUNC_ENTER();
	if (atomic_inc_return(&hdmi_open_cnt_atomic) == 1) {
		g_ProcHandle = (ulong)file;
	} else {
		ret = MT_SUCCESS;
		goto OPEN_RETURN;
	}
	idx = iminor(inode) - hdmi20_drv->minor;
	if (idx >= hdmi20_drv->minors || idx < 0) {
		ret = -ENODEV;
		goto OPEN_RETURN;
	}
	hdmi20_dev = hdmi20_drv->hdmi20_dev[idx];
	HDMI20_PRINTK("\n[%s_%d] hdl:%px,idx=%d\n", __func__, __LINE__, hdmi20_dev, idx);
	file->private_data = hdmi20_dev;

	/* if the device no support llseek method, we should call nonseekable_open */
	nonseekable_open(inode, file);

	mutex_lock(&hdmi20_dev->mutex_hdmi);
	hdmi20_dev->open_cnt++;
	mutex_unlock(&hdmi20_dev->mutex_hdmi);
OPEN_RETURN:
	HDMI20_PRINT_FUNC_EXIT();
	return ret;
}

static int hdmi20_release(struct inode *inode, struct file *file)
{
	struct hdmi20_device *hdmi20_dev;
	struct hdmi20_name *name;
	struct hdmi20_name *name_safe;
	struct hdmi20_thread *thread;
	struct hdmi20_thread *thread_safe;
	struct hdmi20_buffer *buf;
	int i;
	MT_U32 u32Index;
	HDMI_PROC_EVENT_S *pEventList =  DRV_Get_EventList(MT_UNF_HDMI_ID_0);

	HDMI20_PRINT_FUNC_ENTER();
	if (atomic_dec_return(&hdmi_open_cnt_atomic) != 0)
	{
		MT_ALWAYS_PRINT("%s  atmOpenCnt  is not zero !\n", __func__);
	} else {
		hdmi20_dev = file->private_data;

		for(u32Index = 0; u32Index < MAX_PROCESS_NUM; u32Index++)
		{
			if (pEventList[u32Index].u32ProcHandle == (ulong)file)
			{
				DRV_HDMI_ReleaseProcID(MT_UNF_HDMI_ID_0, u32Index);
				break;
			}
		}

		if(DRV_Get_IsThreadStoped())
		{
			//avoid ctrl+c in setFormatting / setAttring
			DRV_Set_ThreadStop(MT_FALSE);
		}

		DRV_HDMI_DeInit(MT_TRUE);

		if(DRV_HDMI_GetInitNum(MT_UNF_HDMI_ID_0) == 0)
		{
			HDMIStandbySetupFlag = MT_FALSE;
		}

		mutex_lock(&hdmi20_dev->mutex_hdmi);
		if (--hdmi20_dev->open_cnt <= 0) {
			list_for_each_entry_safe(name, name_safe, &hdmi20_dev->list_name, list) {
				list_del(&name->list);
				list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
					list_del(&thread->list);
					for (i = 0; i < HDMI20_BUFFERS_PER_THREAD; i++) {
						if (thread->buffers[i] != NULL) {
							buf = thread->buffers[i];
							if (buf->in_buf_need) {
								mt_drv_mmz_release(&buf->in_sMBuf);
							}
							if (buf->out_buf_need) {
								mt_drv_mmz_release(&buf->out_sMBuf);
							}
							kfree(thread->buffers[i]);
							thread->buffers[i] = NULL;
						}
					}
					kfree(thread);
				}
				kfree(name);
			}
		}
		mutex_unlock(&hdmi20_dev->mutex_hdmi);
	}
	HDMI20_PRINT_FUNC_EXIT();

	return 0;
}

static long hdmi20_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	if (cmd != CMD_HDMI_POLL_EVENT && cmd != CMD_HDMI_GET_STATUS ) {
		HDMI20_PRINT_FUNC_ENTER();
		HDMI20_PRINTK("\ncmd:0x%x, init:0x%lx, 0x%lx\n", cmd, (ulong)CMD_HDMI_INIT, (ulong)CMD_HDMI_GET_PROCID);
	}
	return (long)mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, hdmi_Ioctl);
	//HDMI20_PRINT_FUNC_EXIT();
}

static int hdmi20_mmap(struct file *file, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;
	//phys_addr_t offset = (phys_addr_t)vma->vm_pgoff << PAGE_SHIFT;

	/* always cached, don't call vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot) */

	HDMI20_PRINT_FUNC_ENTER();
	if (remap_pfn_range(vma,
						vma->vm_start,
						vma->vm_pgoff,
						size,
						vma->vm_page_prot)) {
		return -EAGAIN;
	}
	HDMI20_PRINT_FUNC_EXIT();
	return 0;
}

static struct file_operations hdmi20_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.unlocked_ioctl = hdmi20_ioctl,
	.open = hdmi20_open,
	.release = hdmi20_release,
	.mmap = hdmi20_mmap,
};

#if defined(CONFIG_MT_HDMI_V20_DEBUG_PROC_FS)
static MT_S32 HDMI0_Proc(struct seq_file *p, MT_VOID *v)
{
	MT_S32 ret;
	ret = HDMI0_Proc_Common((char *)p, v, 1);
	return ret;
}

static MT_S32 HDMI0_Sink_Proc(struct seq_file *p, MT_VOID *v)
{
	MT_S32  ret;
	ret = HDMI0_Sink_Proc_Common((char *)p, v, 1);
	return ret;
}
#endif

#if defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS)
static MT_S32 HDMI0_Sysfs(char *p, MT_VOID *v)
{
	MT_S32 ret;
	ret = HDMI0_Proc_Common(p, v, 0);
	return ret;
}

static MT_S32 HDMI0_Sink_Proc_Sysfs(char *p, MT_VOID *v)
{
	MT_S32  ret;
	ret = HDMI0_Sink_Proc_Common(p, v, 0);
	return ret;
}
#endif

#if (defined(CONFIG_MT_HDMI_V20_DEBUG_PROC_FS) || defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS))

#define MT_DRV_PROC_EchoHelper(fmt...) HDMI20_PRINTK(fmt)

#define BIT_ENUM__TPI_INFO_FSEL__AVI                  0x00
#define BIT_ENUM__TPI_INFO_FSEL__GBD                  0x01
#define BIT_ENUM__TPI_INFO_FSEL__AUDIO                0x02
#define BIT_ENUM__TPI_INFO_FSEL__SPD                  0x03
#define BIT_ENUM__TPI_INFO_FSEL__MPEG                 0x04
#define BIT_ENUM__TPI_INFO_FSEL__VSIF                 0x05
#define BIT_ENUM__TPI_INFO_FSEL__GEN1                 0x06  // use this for ISRC
#define BIT_ENUM__TPI_INFO_FSEL__GEN2                 0x07  // use this for ISRC2
#define BIT_ENUM__TPI_INFO_FSEL__GEN3                 0x08  // use this for ACP
#define BIT_ENUM__TPI_INFO_FSEL__GEN4                 0x09
#define BIT_ENUM__TPI_INFO_FSEL__GEN5                 0x0A

static MT_U8 *g_pDefHDMIMode[] = {"NULL", "HDMI", "DVI", "BUTT"};

static MT_U8 *g_pAudioFmtCode[] = {
	"Reserved", "PCM",  "AC3",     "MPEG1", "MP3",   "MPEG2", "AAC",
	"DTS",     "ATRAC", "ONE_BIT", "DDP",   "DTS_HD", "MAT",  "DST",
	"WMA_PRO",  "EXT AUD--NA"
};
#if 0 /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
static MT_U8 *g_pSampleRate[] = {
	"32", "44.1", "48", "88.2", "96", "176.4", "192", "BUTT"
};
#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
static MT_U8 *g_pSpeaker[] = {
	"FL/FR", "LFE", "FC", "RL/RR", "RC", "FLC/FRC", "RLC/RRC",
	"FLW/FRW", "FLH/FRH", "TC", "FCH"
};

static MT_U8 *g_pAudInputType[] = { "I2S", "SPDIF", "HBR", "BUTT"};
static MT_U8 *g_pColorSpace[] = {"RGB444", "YCbCr422", "YCbCr444", "YCbCr420/BUTT", "Auto", "Auto"};
static MT_U8 *g_pDeepColor[] = {"24bit", "30bit", "36bit", "48bit", "Y444", "OFF", "BUTT"};

static MT_U8 *g_p3DMode[] = {
	"FPK",
	"FILED_ALTER",
	"LINE_ALTE",
	"SBS_FULL",
	"L_DEPTH",
	"L_DEPTH_G_DEPTH",
	"TAB",
	"", //0x07 unknown
	"SBS_HALF",
};

static MT_U8 *g_pScanInfo[] = {"No Data", "OverScan", "UnderScan", "Future"};
static MT_U8 *g_pPixelRep[] = {"1x(No Repeat)", "2x", "3x", "4x", "5x", "6x", "7x", "8x", "9x", "10x", "Reserved"};

static MT_U8 g_pDispFmtString[MT_DRV_DISP_FMT_BUTT + 1][DEF_FILE_NAMELENGTH];

static MT_U8 g_pUnfFmtString[MT_UNF_ENC_FMT_BUTT + 1][DEF_FILE_NAMELENGTH];

void hdmi_InitFmtArray(void)
{
	MT_U32 i = 0;
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);

	DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_0, MT_FALSE);
	memset(pSinkCap, 0, sizeof(MT_UNF_EDID_BASE_INFO_S));
	//init native fmt
	pSinkCap->enNativeFormat = MT_UNF_ENC_FMT_BUTT;

	for (i = 0; i < MT_UNF_ENC_FMT_BUTT + 1; i++) {
		memset(g_pUnfFmtString[i], 0, sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
		mt_osal_strncpy(g_pUnfFmtString[i], "unknown", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	}

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_1080P_60              ], "1080P60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_1080P_60              ], "1080P60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_1080P_50              ], "1080P50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_1080P_30              ], "1080P30", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_1080P_25              ], "1080P25", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_1080P_24              ], "1080P24", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_1080i_60              ], "1080i60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_1080i_50              ], "1080i50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_720P_60               ], "720P60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_720P_50               ], "720P50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_576P_50               ], "576P50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_480P_60               ], "480P60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_PAL                   ], "PAL", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_PAL_N                 ], "PAL_N", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_PAL_Nc                ], "PAL_Nc", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_NTSC                  ], "NTSC", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_NTSC_J                ], "NTSC_J", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_NTSC_PAL_M            ], "NTSC_PAL_M", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_SECAM_SIN             ], "SECAM_SIN", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_SECAM_COS             ], "SECAM_COS", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING], "1080P24_FP", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_720P_60_FRAME_PACKING ], "720P60_FP", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_720P_50_FRAME_PACKING ], "720P50_FP", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_861D_640X480_60       ], "640x480", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_800X600_60       ], "800x600", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1024X768_60      ], "1024x768", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1280X720_60      ], "1280x720", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1280X800_60      ], "1280x800", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1280X1024_60     ], "1280x1024", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1360X768_60      ], "1360x768", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1366X768_60      ], "1366x768", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1400X1050_60     ], "1400x1050", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1440X900_60      ], "1440x900", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1440X900_60_RB   ], "1440x900_RB", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1600X900_60_RB   ], "1600x900_RB", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1600X1200_60     ], "1600x1200", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1680X1050_60     ], "1680x1050", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1680X1050_60_RB  ], "1680x1050_RB", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1920X1080_60     ], "1920x1080", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1920X1200_60     ], "1920x1200", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_1920X1440_60     ], "1920x1440", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_2048X1152_60     ], "2048x1152 ", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_2560X1440_60_RB  ], "2560x1440 _RB", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_VESA_2560X1600_60_RB  ], "2560x1600 _RB", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_3840X2160_24          ], "3840X2160_24", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_3840X2160_25          ], "3840X2160_25", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_3840X2160_30          ], "3840X2160_30", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_3840X2160_50          ], "3840X2160_50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_3840X2160_60          ], "3840X2160_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_4096X2160_24          ], "4096X2160_24", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_4096X2160_25          ], "4096X2160_25", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_4096X2160_30          ], "4096X2160_30", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_4096X2160_50          ], "4096X2160_50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_4096X2160_60          ], "4096X2160_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pUnfFmtString[MT_UNF_ENC_FMT_BUTT                  ], "BUTT", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	for (i = 0; i < MT_DRV_DISP_FMT_BUTT; i++) {
		memset(g_pDispFmtString[i], 0, sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
		mt_osal_strncpy(g_pDispFmtString[i], "unknown", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	}

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_1080P_60             ], "1080P60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_1080P_50             ], "1080P50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_1080P_30             ], "1080P30", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_1080P_25             ], "1080P25", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_1080P_24             ], "1080P24", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_1080i_60             ], "1080i60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_1080i_50             ], "1080i50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_720P_60              ], "720P60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_720P_50              ], "720P50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_576P_50              ], "576P50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_480P_60              ], "480P60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL                  ], "PAL", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_B                ], "PAL_B", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_B1               ], "PAL_B1", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_D                ], "PAL_D", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_D1               ], "PAL_D1", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_G                ], "PAL_G", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_H                ], "PAL_H", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_K                ], "PAL_K", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_I                ], "PAL_I", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_N                ], "PAL_N", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_Nc               ], "PAL_Nc", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_M                ], "PAL_M", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_PAL_60               ], "PAL_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_NTSC                 ], "NTSC", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_NTSC_J               ], "NTSC_J", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_NTSC_443             ], "NTSC_443", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_SECAM_SIN            ], "SECAM_SIN", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_SECAM_COS            ], "SECAM_COS", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_SECAM_L              ], "SECAM_L", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_SECAM_B              ], "SECAM_B", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_SECAM_G              ], "SECAM_G", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_SECAM_D              ], "SECAM_D", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_SECAM_K              ], "SECAM_K", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_SECAM_H              ], "SECAM_H", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_1440x576i_50         ], "1440x576i_50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_1440x480i_60         ], "1440x480i_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_1080P_24_FP          ], "1080P_24_FP", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_720P_60_FP           ], "720P_60_FP", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_720P_50_FP           ], "720P_50_FP", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_861D_640X480_60      ], "640X480_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_800X600_60      ], "800X600_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1024X768_60     ], "1024X768_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1280X720_60     ], "1280X720_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1280X800_60     ], "1280X800_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1280X1024_60    ], "1280X1024_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1360X768_60     ], "1360X768_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1366X768_60     ], "1366X768_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1400X1050_60    ], "1400X1050_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1440X900_60     ], "1440X900_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1440X900_60_RB  ], "1440X900_60_RB", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1600X900_60_RB  ], "1600X900_60_RB", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1600X1200_60    ], "1600X1200_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1680X1050_60    ], "1680X1050_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1680X1050_60_RB ], "1680X1050_60_RB", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1920X1080_60    ], "1920X1080_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1920X1200_60    ], "1920X1200_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_1920X1440_60    ], "1920X1440_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_2048X1152_60    ], "2048X1152_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_2560X1440_60_RB ], "2560X1440_60_RB", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_VESA_2560X1600_60_RB ], "2560X1600_60_RB", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_3840X2160_24         ], "3840X2160_24", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_3840X2160_25         ], "3840X2160_25", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_3840X2160_30         ], "3840X2160_30", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_3840X2160_50         ], "3840X2160_50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_3840X2160_60         ], "3840X2160_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_4096X2160_24         ], "4096X2160_24", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_4096X2160_25         ], "4096X2160_25", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_4096X2160_30         ], "4096X2160_30", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_4096X2160_50         ], "4096X2160_50", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_4096X2160_60         ], "4096X2160_60", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);

	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_CUSTOM               ], "Customer Timing", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
	mt_osal_strncpy(g_pDispFmtString[MT_DRV_DISP_FMT_BUTT                 ], "BUTT", sizeof(MT_U8)*DEF_FILE_NAMELENGTH);
}

MT_S32 DRV_HDMI_ReadPhy(void)
{
	MT_U32 u32Ret;

	u32Ret = SI_TX_PHY_GetOutPutEnable();

	return u32Ret;
}

#define HDMI_PROC_PRINT(p,...) ({\
		MT_U32 cnt = 0;\
		if(isproc == 1){\
			seq_printf((struct seq_file *)p,##__VA_ARGS__);\
		}else{\
			cnt = (MT_U32)snprintf((char *)p, PAGE_SIZE - count, ##__VA_ARGS__);\
			count += cnt;\
		}\
		cnt;\
	})

static MT_S32 HDMI0_Proc_Common(char *p, MT_VOID *v, MT_U32 isproc)
{
	MT_U32 u32Reg, index, u32DefHDMIMode;
	HDMI_ATTR_S                   stHDMIAttr;
	HDMI_VIDEO_ATTR_S            *pstVideoAttr;
	HDMI_AUDIO_ATTR_S            *pstAudioAttr;
	HDMI_APP_ATTR_S     *pstAppAttr = &(DRV_Get_AppAttrMt(MT_UNF_HDMI_ID_0)->stAppAttr);
	MT_UNF_HDMI_STATUS_S          stHdmiStatus;
	MT_UNF_HDMI_CEC_STATUS_S      CECStatus;
	MT_U32                        u32PlayStatus = 0;
	MT_S32 s32Temp, Ret = -1;
	ssize_t count = 0;

	p += HDMI_PROC_PRINT(p, "--------------------------------- Mt HDMI Dev Stat --------------------------------\n");
	Ret = DRV_HDMI_GetAttr(MT_UNF_HDMI_ID_0, &stHDMIAttr);
	if (Ret != MT_SUCCESS) {
		p += HDMI_PROC_PRINT(p, "HDMI driver do not Open\n" );
		p += HDMI_PROC_PRINT(p, "----------------------------------------- END -----------------------------------------\n");
		return count;
	}
	u32Reg = 1;//DRV_ReadByte_8BA(0, TX_SLV0, 0x08);// 0x72:0x08
	if ((u32Reg & 0x01) != 0x01) {
		p += HDMI_PROC_PRINT(p, "HDMI do not Start!\n");
		p += HDMI_PROC_PRINT(p, "----------------------------------------- END -----------------------------------------\n");
		return count;
	}
	pstVideoAttr = &stHDMIAttr.stVideoAttr;
	pstAudioAttr = &stHDMIAttr.stAudioAttr;
	//pstAppAttr = &stHDMIAttr.stAppAttr;

	DRV_HDMI_GetStatus(MT_UNF_HDMI_ID_0, &stHdmiStatus);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Hotplug");
	if (stHdmiStatus.bConnected) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Disable");
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Thread");
	s32Temp = (DRV_Get_IsChnOpened(MT_UNF_HDMI_ID_0) && !DRV_Get_IsThreadStoped() && !SI_IsHDMIResetting()) ;
	if (s32Temp) {
		p += HDMI_PROC_PRINT(p, "%s\n", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "Disable");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Sink");
	if (stHdmiStatus.bSinkPowerOn) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Active");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Deactive");
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "HDCP Enable");
	if (pstAppAttr->bHDCPEnable) {
		p += HDMI_PROC_PRINT(p, "%s\n", "ON");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "OFF");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "PHY Output");
	s32Temp = DRV_HDMI_ReadPhy();
	if (s32Temp) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Disable");
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "HDCP Encryption");
	s32Temp = (MT_S32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__HDCP2X_CTL_0);
	if (s32Temp & BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_ENCRYPT_EN) {
		if ((SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_COPP_DATA2) & BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION) == 0) {
			p += HDMI_PROC_PRINT(p, "%s\n", "2X ON");
		} else {
			p += HDMI_PROC_PRINT(p, "%s\n", "2X Err");
		}
	} else if ((SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_COPP_DATA2) & BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION) == 0) {
		p += HDMI_PROC_PRINT(p, "%s\n", "1X ON");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "OFF");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Play Status");
	DRV_HDMI_GetPlayStatus(0, &u32PlayStatus);
	if (u32PlayStatus) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Start");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Stop");
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "CEC Status");
	memset(&CECStatus, 0, sizeof(MT_UNF_HDMI_CEC_STATUS_S));
	#ifdef CEC_SUPPORT
	DRV_HDMI_CECStatus(MT_UNF_HDMI_ID_0, &CECStatus);
	#endif
	if (CECStatus.bEnable) {
		p += HDMI_PROC_PRINT(p, "%s\n", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "Disable");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "EDID Status");
	if (DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Valid");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Unvalid");
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "CEC Phy Addr");

	p += HDMI_PROC_PRINT(p, "%02d.%02d.%02d.%02d\n", CECStatus.u8PhysicalAddr[0],
						 CECStatus.u8PhysicalAddr[1], CECStatus.u8PhysicalAddr[2], CECStatus.u8PhysicalAddr[3]);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Default Mode");
	u32DefHDMIMode = DRV_Get_DefaultOutputMode(MT_UNF_HDMI_ID_0);
	p += HDMI_PROC_PRINT(p, "%-20s| ", g_pDefHDMIMode[u32DefHDMIMode]);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "CEC Logical Addr");
	p += HDMI_PROC_PRINT(p, "%d\n", CECStatus.u8LogicalAddr);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Output Mode");
	s32Temp = (MT_S32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_SC);
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__SCRCTL);
	if ((u32Reg & BIT_MSK__SCRCTL__REG_HDMI2_ON) && (s32Temp & BIT_MSK__TPI_SC__REG_TPI_OUTPUT_MODE_B0)) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "HDMI2x");
	} else if (s32Temp & BIT_MSK__TPI_SC__REG_TPI_OUTPUT_MODE_B0) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "HDMI1x");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "DVI");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "AVMUTE");
	if ( BIT_MSK__TPI_SC__REG_TPI_AV_MUTE != (s32Temp & BIT_MSK__TPI_SC__REG_TPI_AV_MUTE)) {
		p += HDMI_PROC_PRINT(p, "%s ", "Disable");
	} else {
		p += HDMI_PROC_PRINT(p, "%s ", "Enable");
	}
	p += HDMI_PROC_PRINT(p, "\n");

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Force SetFmt Delay");
	if (IsForceFmtDelay()) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Force");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Default");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Force Mute Delay");
	if (IsForceMuteDelay()) {
		p += HDMI_PROC_PRINT(p, "%s ", "Force");
	} else {
		p += HDMI_PROC_PRINT(p, "%s ", "Default");
	}
	p += HDMI_PROC_PRINT(p, "\n");

	p += HDMI_PROC_PRINT(p, "---------------- Video -------------------|---------------- Audio -------------------\n");

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Video Output ");
	if (pstAppAttr->bEnableVideo) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Disable");
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "AUD Output");
	if (pstAppAttr->bEnableAudio) {
		p += HDMI_PROC_PRINT(p, "%s\n", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "Disable");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Current Fmt");
	if (pstVideoAttr->enVideoFmt < MT_DRV_DISP_FMT_BUTT) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", g_pDispFmtString[pstVideoAttr->enVideoFmt]);
	} else {
		p += HDMI_PROC_PRINT(p, "err-%-16d| ", pstVideoAttr->enVideoFmt);
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Input Type");
	p += HDMI_PROC_PRINT(p, "%s\n", g_pAudInputType[pstAudioAttr->enSoundIntf]);

	{
	uint8_t cs, pc, std;
	extern void SI_GetHdmiHalCs(uint8_t *cs, uint8_t *std, uint8_t *pc);
	SI_GetHdmiHalCs(&cs, &std, &pc);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Color Space");
	//p += HDMI_PROC_PRINT(p, "%-20s| ", g_pColorSpace[pstAppAttr->enVidOutMode]);
	p += HDMI_PROC_PRINT(p, "%-20s| ", g_pColorSpace[cs]);
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Sample Rate");
	p += HDMI_PROC_PRINT(p, "%dHZ\n", pstAudioAttr->enSampleRate);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "DeepColor");
	{
		{
			uint8_t dc;
			char d[20];
			SI_GetHdmiHalDc(&dc);
			if ( pstAppAttr->enDeepColorMode == MT_UNF_HDMI_DEEP_COLOR_OFF ) {
				sprintf(d,"%s%s%s","OFF","/",g_pDeepColor[dc<3 ? dc : 3]);
				p += HDMI_PROC_PRINT(p, "%-20s| ", d);
			} else if ( pstAppAttr->enDeepColorMode == MT_UNF_HDMI_DEEP_COLOR_BUTT ) {
				sprintf(d,"%s%s%s","BUTT","/",g_pDeepColor[dc<3 ? dc : 3]);
				p += HDMI_PROC_PRINT(p, "%-20s| ", d);
			} else {
				sprintf(d,"%s%s%s",g_pDeepColor[pstAppAttr->enDeepColorMode],"/",g_pDeepColor[dc<3 ? dc : 3]);
				p += HDMI_PROC_PRINT(p, "%-20s| ", d);
			}
		}
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Bit Depth");
	p += HDMI_PROC_PRINT(p, "%dbit\n", pstAudioAttr->enBitDepth);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "xvYCC");
	if (pstAppAttr->bxvYCCMode) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Disable");
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Trace Mode");
	if (pstAudioAttr->bIsMultiChannel) {
		p += HDMI_PROC_PRINT(p, "%s\n", "Multichannel(8)");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "Stereo");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "3D Mode");
	if (0 == pstVideoAttr->u83DParam) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "FPK");
	} else if (8 == pstVideoAttr->u83DParam) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "SBS HALF");
	} else if (6 == pstVideoAttr->u83DParam) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "TAB");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "2D");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "N Value");
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__N_SVAL3);
	u32Reg = (u32Reg << 8) | (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__N_SVAL2);
	u32Reg = (u32Reg << 8) | (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__N_SVAL1);
	p += HDMI_PROC_PRINT(p, "0x%x(%d)\n", u32Reg, u32Reg);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Global SetFmt Delay");
	p += HDMI_PROC_PRINT(p, "%-20d| ", GetGlobalFmtDelay());

	p += HDMI_PROC_PRINT(p, "%-20s: ", "CTS");
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__CTS_TXHVAL3);
	u32Reg = (u32Reg << 8) | (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__CTS_TXHVAL2);
	u32Reg = (u32Reg << 8) | (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__CTS_TXHVAL1);
	p += HDMI_PROC_PRINT(p, "0x%x(%d)\n", u32Reg, u32Reg);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Global Mute Delay");
	p += HDMI_PROC_PRINT(p, "%-20d| ", GetGlobalsMuteDelay());
	p += HDMI_PROC_PRINT(p, "\n");
	//count += HDMI_Parse_SiCMS(&p,isproc);
	p += HDMI_PROC_PRINT(p, "---------------------------------- Info Frame status --------------------------------\n");
	p += HDMI_PROC_PRINT(p, "%-25s: ", "AVI InfoFrame");
	SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__AVI);
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_EN);
	if ( 0xc0 == (u32Reg & 0xc0)) {
		p += HDMI_PROC_PRINT(p, "%-15s| ", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%-15s| ", "Disable");
	}
	p += HDMI_PROC_PRINT(p, "%-23s: ", "Gamut Metadata Packet");
	u32Reg = 0;//DRV_ReadByte_8BA(0, TX_SLV1, 0x3F);  // 0x7A:0x3F
	if (0xC0 == (u32Reg & 0xC0)) {
		p += HDMI_PROC_PRINT(p, "%s\n", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "Disable");
	}

	p += HDMI_PROC_PRINT(p, "%-25s: ", "AUD InfoFrame");
	SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__AUDIO);
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_EN);
	if ( 0xc0 == (u32Reg & 0xc0)) {
		p += HDMI_PROC_PRINT(p, "%-15s| ", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%-15s| ", "Disable");
	}
	p += HDMI_PROC_PRINT(p, "%-23s: ", "Generic Packet");
	u32Reg = 0;//DRV_ReadByte_8BA(0, TX_SLV1, 0x3F);  // 0x7A:0x3F
	if (0x03 == (u32Reg & 0x03)) {
		p += HDMI_PROC_PRINT(p, "%s\n", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "Disable");
	}

	p += HDMI_PROC_PRINT(p, "%-25s: ", "MPg/VendorSpec InfoFrame");
	SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__VSIF);
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_EN);
	if ( 0xC0 == (u32Reg & 0xC0)) {
		p += HDMI_PROC_PRINT(p, "%-15s| ", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%-15s| ", "Disable");
	}
	p += HDMI_PROC_PRINT(p, "%-23s: ", "DRM InfoFrame");
	SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__GEN3);
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_EN);
	if ( 0xC0 == (u32Reg & 0xC0)) {
		p += HDMI_PROC_PRINT(p, "%-15s| ", "Enable");
	} else {
		p += HDMI_PROC_PRINT(p, "%-15s| ", "Disable");
	}
	p += HDMI_PROC_PRINT(p, "\n");
	p += HDMI_PROC_PRINT(p, "%-25s: ", "HDR10p");
	u32Reg = (MT_U32)SiiDrvCraRdReg32((SiiInst_t)NULL, REG_ADDR__EMP_CTRL1);
	p += HDMI_PROC_PRINT(p, "6A0[0x%x]/", u32Reg);
	if (u32Reg & BIT_MSK__REG_EMP_MODE) {
		p += HDMI_PROC_PRINT(p, "CO/");
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__EMP_START_LINE_LOW);
		u32Reg |= ((MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__EMP_START_LINE_HIGH)) << 8;
		p += HDMI_PROC_PRINT(p, "start[%d]/", u32Reg);
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__EMP_END_LINE_LOW);
		u32Reg |= ((MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__EMP_END_LINE_HIGH)) << 8;
		p += HDMI_PROC_PRINT(p, "end[%d]/", u32Reg);
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__EMP_CTRL);
		p += HDMI_PROC_PRINT(p, "686[0x%x]/", u32Reg);
		if (u32Reg & BIT_MSK__REG_EMP_REPEAT) {
			p += HDMI_PROC_PRINT(p, "Repeat Y/");
		} else {
			p += HDMI_PROC_PRINT(p, "Repeat N/");
		}
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__INTR_MASK);
		p += HDMI_PROC_PRINT(p, "intr[0x%x]/", u32Reg);
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__INTR_STATUS);
		p += HDMI_PROC_PRINT(p, "sts[0x%x]/", u32Reg);
	} else {
		if (u32Reg & BIT_MSK__REG_CPU_EN) {
			p += HDMI_PROC_PRINT(p, "DC/");
		} else {
			p += HDMI_PROC_PRINT(p, "DD/");
		}
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__EMP_CTRL);
		p += HDMI_PROC_PRINT(p, "686[0x%x]/", u32Reg);
		if (u32Reg & BIT_MSK__REG_EMP_REPEAT_1) {
			p += HDMI_PROC_PRINT(p, "Repeat Y/");
		} else {
			p += HDMI_PROC_PRINT(p, "Repeat N/");
		}
		u32Reg = (MT_U32)SiiDrvCraRdReg32((SiiInst_t)NULL, REG_ADDR__EMP_CTRL1);
		if (u32Reg & BIT_MSK__REG_EMP_MTW_MODE) {
			p += HDMI_PROC_PRINT(p, "mtw inner-frame/");
		} else {
			p += HDMI_PROC_PRINT(p, "mtw inter-frame/");
		}
		p += HDMI_PROC_PRINT(p, "pkt[%d]/", (u32Reg & BIT_MSK__REG_DMA_PKT_NUM) >> 8);
		u32Reg = (MT_U32)SiiDrvCraRdReg32((SiiInst_t)NULL, REG_ADDR__EMP_MTW_START_CFG);
		p += HDMI_PROC_PRINT(p, "start[%d]/", u32Reg);
		u32Reg = (MT_U32)SiiDrvCraRdReg32((SiiInst_t)NULL, REG_ADDR__EMP_MTW_END_CFG);
		p += HDMI_PROC_PRINT(p, "end[%d]/", u32Reg);
	}
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__HDR10PLUS_CFG);
	switch (u32Reg & 0x03) {
		case 0:
			p += HDMI_PROC_PRINT(p, "D-EMP/ ");
			break;
		case 1:
			p += HDMI_PROC_PRINT(p, "D-VSIF/ ");
			break;
		case 2:
		case 3:
			p += HDMI_PROC_PRINT(p, "C-Only/ ");
			break;
		default:
			break;
	}
	p += HDMI_PROC_PRINT(p, "\n");
	p += HDMI_PROC_PRINT(p, "-------------------------------------- Raw Data -------------------------------------\n");
	p += HDMI_PROC_PRINT(p, "AVI InfoFrame :\n");
	SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__AVI);
	for (index = 0; index < 17; index ++) {
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + index);
		p += HDMI_PROC_PRINT(p, "0x%02x,", u32Reg);
	}
	p += HDMI_PROC_PRINT(p, "\n");
	p += HDMI_PROC_PRINT(p, "AUD InfoFrame :\n");
	SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__AUDIO);
	for (index = 0; index < 10; index ++) {
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + index);
		p += HDMI_PROC_PRINT(p, "0x%02x,", u32Reg);
	}
	p += HDMI_PROC_PRINT(p, "\n");
	p += HDMI_PROC_PRINT(p, "MPg/VendorSpec Inforframe :\n");
	SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__VSIF);
	for (index = 0; index < 26; index ++) {
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + index);
		p += HDMI_PROC_PRINT(p, "0x%02x,", u32Reg);
	}
	p += HDMI_PROC_PRINT(p, "\n");
	p += HDMI_PROC_PRINT(p, "DRM Inforframe :\n");
	SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__GEN3);
	for (index = 0; index < 26; index ++) {
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + index);
		p += HDMI_PROC_PRINT(p, "0x%02x,", u32Reg);
	}
	p += HDMI_PROC_PRINT(p, "\n");
	p += HDMI_PROC_PRINT(p, "------------------------------------ Parsed InfoFrame -------------------------------\n");
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Video ID Code(VIC)");
	SiiDrvCraPutBit8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__AVI);
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + 7);
	p += HDMI_PROC_PRINT(p, "0x%-18x| ", u32Reg);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Colorimetry");
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + 5);
	u32Reg = u32Reg >> 6;
	if (!u32Reg) {
		p += HDMI_PROC_PRINT(p, "%s\n", "No Data");
	} else if (0x1 == u32Reg) {
		p += HDMI_PROC_PRINT(p, "%s\n", "ITU601");
	} else if (0x2 == u32Reg) {
		p += HDMI_PROC_PRINT(p, "%s\n", "ITU709");
	} else {
		u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + 6);
		u32Reg &= 0x70;
		if (u32Reg == 0x00) {
			p += HDMI_PROC_PRINT(p, "%s\n", "xvYCC601");
		} else if (u32Reg == 0x10) {
			p += HDMI_PROC_PRINT(p, "%s\n", "xvYCC709");
		} else if (u32Reg == 0x20) {
			p += HDMI_PROC_PRINT(p, "%s\n", "sYCC601");
		} else if (u32Reg == 0x30) {
			p += HDMI_PROC_PRINT(p, "%s\n", "opYCC601");
		} else if (u32Reg == 0x40) {
			p += HDMI_PROC_PRINT(p, "%s\n", "opRGB");
		} else if (u32Reg == 0x50) {
			p += HDMI_PROC_PRINT(p, "%s\n", "BT.2020 YcCbcCrc");
		} else if (u32Reg == 0x60) {
			p += HDMI_PROC_PRINT(p, "%s\n", "BT.2020 RGB or YCbCr");
		} else {
			u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + 17);
			u32Reg &= 0xf0;
			if (u32Reg == 0x00) {
				p += HDMI_PROC_PRINT(p, "%s\n", "P3D65 RGB");
			} else if (u32Reg == 0x10) {
				p += HDMI_PROC_PRINT(p, "%s\n", "P3DCI RGB");
			} else if (u32Reg == 0x20) {
				p += HDMI_PROC_PRINT(p, "%s\n", "BT.2100 ICtCp");
			} else {
				p += HDMI_PROC_PRINT(p, "%s\n", "reserve");
			}
		}
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Pixel Repetition");
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + 8);
	u32Reg &= 0x0F;
	if (u32Reg > 10) {
		//array overflow, set to reserved
		u32Reg = 10;
	}
	p += HDMI_PROC_PRINT(p, "%-20s| ", g_pPixelRep[u32Reg]);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "ScanInfo");
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + 4);
	u32Reg &= 0x03;
	p += HDMI_PROC_PRINT(p, "%s\n", g_pScanInfo[u32Reg]);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Output Color Space");
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + 4);
	u32Reg &= 0x60;
	u32Reg >>= 5;
	p += HDMI_PROC_PRINT(p, "%-20s| ", g_pColorSpace[u32Reg]);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "AspectRatio");
	u32Reg = (MT_U32)SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__TPI_INFO_B0 + 5);
	u32Reg &= 0x30;
	if (0x00 == u32Reg) {
		p += HDMI_PROC_PRINT(p, "%s\n", "No Data");
	} else if (0x10 == u32Reg) {
		p += HDMI_PROC_PRINT(p, "%s\n", "4:3");
	} else if (0x20 == u32Reg) {
		p += HDMI_PROC_PRINT(p, "%s\n", "16:9");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "Future");
	}
	p += HDMI_PROC_PRINT(p, "--------------------------------- Debug Command -------------------------------------\n");
	p += HDMI_PROC_PRINT(p, "type 'echo help > /proc/msp/hdmi0' to get help informatin \n");
	p += HDMI_PROC_PRINT(p, "---------------------------------------- END ----------------------------------------\n");

	return count;
}

static MT_S32 HDMI0_Sink_Proc_Common(char *p, MT_VOID *v, MT_U32 isproc)
{
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
	MT_U32 i, j;
	ssize_t count = 0;
	HDMI_EDID_S pEDID = {0};

	//hdmi_InitFmtArray();

	p += HDMI_PROC_PRINT(p, "--------------------------------- Mt HDMI Sink Capability -------------------------\n");

	p += HDMI_PROC_PRINT(p, "%-20s: ", "EDID Status");
	if (DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "OK");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "Failed");
		return count;
	}
	i = DRV_HDMI_Force_GetEDID(&pEDID);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "TV Manufacture Name");
	p += HDMI_PROC_PRINT(p, "%s\n", pSinkCap->stMfrsInfo.u8MfrsName);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Source of EDID");

	if (DRV_Get_IsUserEdid(MT_UNF_HDMI_ID_0)) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "User Setting");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "From Sink");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "ProductCode");
	p += HDMI_PROC_PRINT(p, "%x\n", pSinkCap->stMfrsInfo.u32ProductCode);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Hdmi Support");
	if (pSinkCap->bSupportHdmi) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "TRUE");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "FALSE");
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "SerialNumber");
	p += HDMI_PROC_PRINT(p, "%x\n", pSinkCap->stMfrsInfo.u32SerialNumber);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "EDID Version");
	p += HDMI_PROC_PRINT(p, "%d.%-18d| ", pSinkCap->u8Version, pSinkCap->u8Revision);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Week Of Manufacture");
	p += HDMI_PROC_PRINT(p, "%d\n", pSinkCap->stMfrsInfo.u32Week);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Extend Block Num");
	p += HDMI_PROC_PRINT(p, "%-20d| ", pSinkCap->u8ExtBlockNum);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Year Of Manufacture");
	p += HDMI_PROC_PRINT(p, "%d\n", pSinkCap->stMfrsInfo.u32Year);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "DVI Dual");
	if (pSinkCap->bSupportDVIDual) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "TRUE");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "FALSE");
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "CEC PhyAddr Valid");
	if (pSinkCap->stCECAddr.bPhyAddrValid) {
		p += HDMI_PROC_PRINT(p, "%s\n", "TRUE");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "FALSE");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Supports AI");
	if (pSinkCap->bSupportsAI) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "TRUE");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "FALSE");
	}
	p += HDMI_PROC_PRINT(p, "%-20s: ", "CEC Phy Add");
	p += HDMI_PROC_PRINT(p, "%02x.%02x.%02x.%02x\n", pSinkCap->stCECAddr.u8PhyAddrA,
						 pSinkCap->stCECAddr.u8PhyAddrB, pSinkCap->stCECAddr.u8PhyAddrC, pSinkCap->stCECAddr.u8PhyAddrD);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "MaxTmdsCR");
	p += HDMI_PROC_PRINT(p, "%-20d| ", pEDID.stEdidParsed.maxclk);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "SCDC/LTE");
	p += HDMI_PROC_PRINT(p, "%2d/%2d\n", pSinkCap->bSupportScdc,pSinkCap->bSupportLTE);
	{
		extern SiiInst_t DRV_HDMI_Get_TxInst(void);
		SiiDrvDsHdcpVersion_t hdcpcap = SII_DRV_DS_HDCP_VER__NONE;
		p += HDMI_PROC_PRINT(p, "%-20s: ", "Hdcp Cap");
		SiiDrvTxHdcpCapGet(DRV_HDMI_Get_TxInst(), &hdcpcap);
		if ( hdcpcap == SII_DRV_DS_HDCP_VER__22 ) {
			p += HDMI_PROC_PRINT(p, "%-20s| ", "hdcp2x");
		} else if ( hdcpcap == SII_DRV_DS_HDCP_VER__1X ) {
			p += HDMI_PROC_PRINT(p, "%-20s| ", "hdcp1x");
		} else {
			p += HDMI_PROC_PRINT(p, "%-20s| ", "hdcp none");
		}
		p += HDMI_PROC_PRINT(p, "\n");
	}
	p += HDMI_PROC_PRINT(p, "-------------------------------------- Video ----------------------------------------\n");
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Video Timing");
	for (i = 0, j = 0; i < MT_UNF_ENC_FMT_BUTT; i++) {
		if (pSinkCap->bSupportFormat[i]) {
			if (pSinkCap->bSupportFormat[i] & 0x2) {
				p += HDMI_PROC_PRINT(p, "%s_%d / ", g_pUnfFmtString[i],pSinkCap->bSupportFormat[i]);
			} else {
				p += HDMI_PROC_PRINT(p, "%s / ", g_pUnfFmtString[i]);
			}
			j++;
			if (0 == j % 6) {
				p += HDMI_PROC_PRINT(p, "\n%-22s", "");
			}
		}
	}
	p += HDMI_PROC_PRINT(p, "\n");
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Native Format");
	p += HDMI_PROC_PRINT(p, "%s\n", g_pUnfFmtString[pSinkCap->enNativeFormat]);
	p += HDMI_PROC_PRINT(p, "%-20s: ", "Colorimetry");
	if (pEDID.stEdidParsed.supported_xvycc601) {
		p += HDMI_PROC_PRINT(p, "%s", "xvYCC601 / ");
	}
	if (pEDID.stEdidParsed.supported_xvycc709) {
		p += HDMI_PROC_PRINT(p, "%s", "xvYCC709 / ");
	}
	if (pEDID.stEdidParsed.supported_bt2020cycc) {
		p += HDMI_PROC_PRINT(p, "%s", "cycc 2020 / ");
	}
	if (pEDID.stEdidParsed.supported_bt2020ycc) {
		p += HDMI_PROC_PRINT(p, "%s", "ycc 2020 / ");
	}
	if (pEDID.stEdidParsed.supported_bt2020rgb) {
		p += HDMI_PROC_PRINT(p, "%s", "rgb 2020 / ");
	}
	p += HDMI_PROC_PRINT(p, "\n");

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Color Space");
	p += HDMI_PROC_PRINT(p, "%s", "RGB444");
	if (pSinkCap->stColorSpace.bYCbCr444) {
		p += HDMI_PROC_PRINT(p, " / %s", "YCbCr444");
	}

	if (pSinkCap->stColorSpace.bYCbCr422) {
		p += HDMI_PROC_PRINT(p, " / %s", "YCbCr422");
	}

	p += HDMI_PROC_PRINT(p, "\n");
	if ( pEDID.stEdidParsed.ycbcr420_supported ) {
		p += HDMI_PROC_PRINT(p, "%-20s: ", "YUV420");
		if ( pEDID.stEdidParsed.supported_3840x2160p_50Hz & 0x2) {
			p += HDMI_PROC_PRINT(p, " / %s", "3840x2160@50");
		}
		if ( pEDID.stEdidParsed.supported_3840x2160p_60Hz & 0x2) {
			p += HDMI_PROC_PRINT(p, " / %s", "3840x2160@60");
		}
		if ( pEDID.stEdidParsed.supported_4096x2160p_50Hz & 0x2) {
			p += HDMI_PROC_PRINT(p, " / %s", "4096x2160@50");
		}
		if ( pEDID.stEdidParsed.supported_4096x2160p_60Hz & 0x2) {
			p += HDMI_PROC_PRINT(p, " / %s", "4096x2160@60");
		}
		p += HDMI_PROC_PRINT(p, "\n");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Deep Color");
	p += HDMI_PROC_PRINT(p, "rgb %s", "24");
	if (pSinkCap->stDeepColor.bDeepColor30Bit) {
		p += HDMI_PROC_PRINT(p, " / %s", "30");
	}
	if (pSinkCap->stDeepColor.bDeepColor36Bit) {
		p += HDMI_PROC_PRINT(p, " / %s", "36");
	}
	if (pSinkCap->stDeepColor.bDeepColor48Bit) {
		p += HDMI_PROC_PRINT(p, " / %s", "48");
	}
	if (pEDID.stEdidParsed.ycbcr420_supported) {
		p += HDMI_PROC_PRINT(p, ":: y420 %s", "24");
		if (pSinkCap->stDeepColor.bDeepColor30Bit_Y420) {
			p += HDMI_PROC_PRINT(p, " / %s", "30");
		}
		if (pSinkCap->stDeepColor.bDeepColor36Bit_Y420) {
			p += HDMI_PROC_PRINT(p, " / %s", "36");
		}
		if (pSinkCap->stDeepColor.bDeepColor48Bit_Y420) {
			p += HDMI_PROC_PRINT(p, " / %s", "48");
		}
	}
	if (pSinkCap->stDeepColor.bDeepColorY444) {
		p += HDMI_PROC_PRINT(p, "bit + Y444\n");
	} else {
		p += HDMI_PROC_PRINT(p, "bit\n");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "HDR");
	{
		mt_u32 hdr = 0;
		if ( pEDID.stEdidParsed.supported_hdr10) {
			p += HDMI_PROC_PRINT(p, " / %s", "HDR10");
			hdr = 1;
		}
		if ( pEDID.stEdidParsed.supported_hlg) {
			p += HDMI_PROC_PRINT(p, " / %s", "HLG");
			hdr = 1;
		}
		if ( pEDID.stEdidParsed.supported_hdr10p_vsif) {
			p += HDMI_PROC_PRINT(p, " / %s", "HDR10+ VSIF");
			hdr = 1;
		}
		if ( pEDID.stEdidParsed.hdr10p_emp) {
			p += HDMI_PROC_PRINT(p, " / %s", "HDR10+ EMP");
			hdr = 1;
		}
		if ( pEDID.stEdidParsed.supported_hdr10p_vivid) {
			p += HDMI_PROC_PRINT(p, " / %s", "HDR10+ VIVID");
			hdr = 1;
		}
		if ( hdr == 0 ) {
			p += HDMI_PROC_PRINT(p, " / %s", "NONE");
		}
	}
	p += HDMI_PROC_PRINT(p, "\n");

	p += HDMI_PROC_PRINT(p, "%-20s: ", "3D Support");
	if (pSinkCap->st3DInfo.bSupport3D) {
		p += HDMI_PROC_PRINT(p, "%s\n", "Support");
		p += HDMI_PROC_PRINT(p, "%-20s: ", "3D Type");
		for (i = 0; i < MT_UNF_EDID_3D_BUTT; i++) {
			if (pSinkCap->st3DInfo.bSupport3DType[i]) {
				p += HDMI_PROC_PRINT(p, "%s / ", g_p3DMode[i]);
			}
		}
		p += HDMI_PROC_PRINT(p, "\n");
		p += HDMI_PROC_PRINT(p, "%-20s: \n", "3D Format List");
		if ( pEDID.stEdidParsed.supported_3d_struc_1080i_50Hz ) {
			p += HDMI_PROC_PRINT(p, "3D1080i50 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_1080i_50Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_1080i_60Hz ) {
			p += HDMI_PROC_PRINT(p, "3D1080i60 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_1080i_60Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_1080p_24Hz ) {
			p += HDMI_PROC_PRINT(p, "3D1080p24 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_1080p_24Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_1080p_25Hz ) {
			p += HDMI_PROC_PRINT(p, "3D1080p25 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_1080p_25Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_1080p_30Hz ) {
			p += HDMI_PROC_PRINT(p, "3D1080p30 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_1080p_30Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_1080p_50Hz ) {
			p += HDMI_PROC_PRINT(p, "3D1080p50 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_1080p_50Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_1080p_60Hz ) {
			p += HDMI_PROC_PRINT(p, "3D1080p60 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_1080p_60Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_480i_60Hz ) {
			p += HDMI_PROC_PRINT(p, "3D480i60 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_480i_60Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_480p_60Hz ) {
			p += HDMI_PROC_PRINT(p, "3D480p60 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_480p_60Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_576i_50Hz ) {
			p += HDMI_PROC_PRINT(p, "3D576i50 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_576i_50Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_576p_50Hz ) {
			p += HDMI_PROC_PRINT(p, "3D576p50 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_576p_50Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_720p_50Hz ) {
			p += HDMI_PROC_PRINT(p, "3D720p50 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_720p_50Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_720p_60Hz ) {
			p += HDMI_PROC_PRINT(p, "3D720p60 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_720p_60Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_3840x2160p_24Hz ) {
			p += HDMI_PROC_PRINT(p, "3D3840x2160p_24 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_3840x2160p_24Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_3840x2160p_25Hz ) {
			p += HDMI_PROC_PRINT(p, "3D3840x2160p_25 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_3840x2160p_25Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_3840x2160p_30Hz ) {
			p += HDMI_PROC_PRINT(p, "3D3840x2160p_30 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_3840x2160p_30Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_3840x2160p_50Hz ) {
			p += HDMI_PROC_PRINT(p, "3D3840x2160p_50 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_3840x2160p_50Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_3840x2160p_60Hz ) {
			p += HDMI_PROC_PRINT(p, "3D3840x2160p_60 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_3840x2160p_60Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_4096x2160p_24Hz ) {
			p += HDMI_PROC_PRINT(p, "3D4096x2160p_24 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_4096x2160p_24Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_4096x2160p_25Hz ) {
			p += HDMI_PROC_PRINT(p, "3D4096x2160p_25 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_4096x2160p_25Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_4096x2160p_30Hz ) {
			p += HDMI_PROC_PRINT(p, "3D4096x2160p_30 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_4096x2160p_30Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_4096x2160p_50Hz ) {
			p += HDMI_PROC_PRINT(p, "3D4096x2160p_50 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_4096x2160p_50Hz));
		}
		if ( pEDID.stEdidParsed.supported_3d_struc_4096x2160p_60Hz ) {
			p += HDMI_PROC_PRINT(p, "3D4096x2160p_60 : %s\n", GET_3D_STRUCT_STRING(pEDID.stEdidParsed.supported_3d_struc_4096x2160p_60Hz));
		}
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "Not Support");
		p += HDMI_PROC_PRINT(p, "%-20s: None", "3D Type");
	}

	p += HDMI_PROC_PRINT(p, "\n");

	p += HDMI_PROC_PRINT(p, "-------------------------------------- Audio ----------------------------------------\n");

	p += HDMI_PROC_PRINT(p, "%-16s| ", "Audio Fmt");
	p += HDMI_PROC_PRINT(p, "%-6s| ", "Chn");
	p += HDMI_PROC_PRINT(p, "%-6s| ", "Dep");
	p += HDMI_PROC_PRINT(p, "%-25s", "samplerate");
	//p += HDMI_PROC_PRINT(p, "%-10s","Extend");
	p += HDMI_PROC_PRINT(p, "\n");

	for (i = 0; i < MT_UNF_EDID_MAX_AUDIO_CAP_COUNT; i++) {
		if (pSinkCap->stAudioInfo[i].enAudFmtCode) {
			p += HDMI_PROC_PRINT(p, "%-3d %-12s| ", pSinkCap->stAudioInfo[i].enAudFmtCode, g_pAudioFmtCode[pSinkCap->stAudioInfo[i].enAudFmtCode]);
			p += HDMI_PROC_PRINT(p, "%-6d| ", pSinkCap->stAudioInfo[i].u8AudChannel);
			if ( pSinkCap->stAudioInfo[i].enAudFmtCode == 12 ) {
				p += HDMI_PROC_PRINT(p, "%-6d| ", pSinkCap->stAudioInfo[i].u32MaxBitRate);
			} else {
				p += HDMI_PROC_PRINT(p, "%-6s| ", "-");
			}

			for (j = 0; j < MAX_SAMPE_RATE_NUM; j++) {
				if (pSinkCap->stAudioInfo[i].enSupportSampleRate[j] != 0) {
					p += HDMI_PROC_PRINT(p, "%d ", (pSinkCap->stAudioInfo[i].enSupportSampleRate[j]));
				}
			}
			p += HDMI_PROC_PRINT(p, "Hz");

			p += HDMI_PROC_PRINT(p, "\n");
		}
	}

	if ( pEDID.stEdidParsed.DdMat48K ) {
		p += HDMI_PROC_PRINT(p, "%-3d %-12s| ", 12, g_pAudioFmtCode[12]);
		p += HDMI_PROC_PRINT(p, "%-6s| ", "-");
		p += HDMI_PROC_PRINT(p, "%-6s| ", "-");
		p += HDMI_PROC_PRINT(p, "48KHz PCM Only");
	}
	p += HDMI_PROC_PRINT(p, "\n%-10s : %d \n", "Audio Info Num", pSinkCap->u32AudioInfoNum);
	p += HDMI_PROC_PRINT(p, "\n%-10s : ", "Speaker");

	for (i = 0; i < MT_UNF_EDID_AUDIO_SPEAKER_BUTT; i++) {
		if (pSinkCap->bSupportAudioSpeaker[i]) {
			p += HDMI_PROC_PRINT(p, "%s ", g_pSpeaker[i]);
		}
	}

	p += HDMI_PROC_PRINT(p, "\n");

	p += HDMI_PROC_PRINT(p, "------------------------------ Custom Prefer Timing ---------------------------------\n");

	p += HDMI_PROC_PRINT(p, "%-20s: ", "VFB");
	p += HDMI_PROC_PRINT(p, "%-20d| ", pSinkCap->stPerferTiming.u32VFB);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "HFB");
	p += HDMI_PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32HFB);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "VBB");
	p += HDMI_PROC_PRINT(p, "%-20d| ", pSinkCap->stPerferTiming.u32VBB);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "HBB");
	p += HDMI_PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32HBB);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "VACT");
	p += HDMI_PROC_PRINT(p, "%-20d| ", pSinkCap->stPerferTiming.u32VACT);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "HACT");
	p += HDMI_PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32HACT);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "VPW");
	p += HDMI_PROC_PRINT(p, "%-20d| ", pSinkCap->stPerferTiming.u32VPW);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "HPW");
	p += HDMI_PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32HPW);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "IDV");
	if (pSinkCap->stPerferTiming.bIDV) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "TRUE");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "FALSE");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "IHS");
	if (pSinkCap->stPerferTiming.bIHS) {
		p += HDMI_PROC_PRINT(p, "%s\n", "TRUE");
	} else {
		p += HDMI_PROC_PRINT(p, "%s\n", "FALSE");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "IVS");
	if (pSinkCap->stPerferTiming.bIVS) {
		p += HDMI_PROC_PRINT(p, "%-20s| \n", "TRUE");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| \n", "FALSE");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Image Width");
	p += HDMI_PROC_PRINT(p, "%-20d| ", pSinkCap->stPerferTiming.u32ImageWidth);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Image Height");
	p += HDMI_PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32ImageHeight);

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Interlace");

	if (pSinkCap->stPerferTiming.bInterlace) {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "TRUE");
	} else {
		p += HDMI_PROC_PRINT(p, "%-20s| ", "FALSE");
	}

	p += HDMI_PROC_PRINT(p, "%-20s: ", "Pixel Clock");
	p += HDMI_PROC_PRINT(p, "%d\n",  pSinkCap->stPerferTiming.u32PixelClk);

	p += HDMI_PROC_PRINT(p, "------------------------------------ EDID Raw Data ---------------------------------- \n");

	if (!DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
		p += HDMI_PROC_PRINT(p, "!! Data unbelievably !! \n");
	}
	//no else
	{
		MT_U32 index, u32EdidLegth = 0;
		MT_U8  Data[1024];

		memset(Data, 0, 1024);
		u32EdidLegth = 128 * (pSinkCap->u8ExtBlockNum + 1);

		if (u32EdidLegth > 1024) {
			u32EdidLegth = 1024;
		}

		//SI_Proc_ReadEDIDBlock(Data, u32EdidLegth);
		DRV_HDMI_GetRawEdidInfo(MT_UNF_HDMI_ID_0, Data);
		for (index = 0; index < u32EdidLegth; index ++) {
			p += HDMI_PROC_PRINT(p, "%02x ", Data[index]);
			if (0 == ((index + 1) % 16)) {
				p += HDMI_PROC_PRINT(p, "\n");
			}
		}
	}

	p += HDMI_PROC_PRINT(p, "---------------------------------------- END ----------------------------------------\n");

	return count;
}

MT_VOID hdmi_ProcWriteEntry(MT_CHAR *chCmd);
MT_S32 hdmi_ProcWrite_Common(struct file * file,
							 const char __user * buf, size_t count, loff_t *ppos, MT_U32 isproc)
{
	MT_CHAR  chCmd[60] = {0};
	if (count > 40) {
		MT_DRV_PROC_EchoHelper("Error:Echo too long.\n");
		return MT_FAILURE;
	}
	if ( isproc ) {
		if (copy_from_user(chCmd, buf, count)) {
			MT_DRV_PROC_EchoHelper("copy from user failed\n");
			return MT_FAILURE;
		}
	} else {
		memcpy(chCmd, buf, count);
	}
	hdmi_ProcWriteEntry(chCmd);
	return count;
}

MT_CHAR *hdmi_GetProcArg(MT_CHAR *chCmd, MT_CHAR *chArg, MT_U32 u32ArgBufSize)
{
	MT_CHAR *chSrc = chCmd;
	MT_U32 i = 0;

	if (NULL == chCmd || NULL == chArg) {
		return NULL;
	}

	/* clear ' ' and '\n' */
	while (' ' == *chSrc || '\n' == *chSrc) {
		chSrc++;
	}

	/* copy char to dest */
	while ('\0' != *chSrc && ' ' != *chSrc && '\n' != *chSrc) {
		*chArg++ = *chSrc++;
		if (i++ >= u32ArgBufSize) {
			COM_ERR("Error : The arg size is larger than the buffer size.\n");
			return NULL;
		}
	}
	*chArg = '\0';

	return chSrc;
}

extern void sSetEDIDPrintEn(uint32_t en);
extern void sSetHdmi_CommonDebug(uint32_t en);
extern MT_BOOL get_current_rgb_mode(MT_UNF_HDMI_ID_E enHdmi);
extern void SI_SetBpssVidVal(uint8_t c0, uint8_t c1, uint8_t c2);
uint16_t g_test_mtx[15];
MT_VOID hdmi_ProcWriteEntry(MT_CHAR *chCmd)
{
	MT_CHAR  chArg1[DEF_FILE_NAMELENGTH] = {0};
	MT_CHAR  chArg2[DEF_FILE_NAMELENGTH] = {0};
	MT_CHAR  chArg3[DEF_FILE_NAMELENGTH] = {0};

	#ifdef HDMI_DEBUG
	//MT_CHAR  chArg3[DEF_FILE_NAMELENGTH] = {0};  //·��
	#endif

	MT_CHAR  *chPtr = NULL;

	chPtr = hdmi_GetProcArg(chCmd, chArg1, DEF_FILE_NAMELENGTH - 1);
	chPtr = hdmi_GetProcArg(chPtr, chArg2, DEF_FILE_NAMELENGTH - 1);
	chPtr = hdmi_GetProcArg(chPtr, chArg3, DEF_FILE_NAMELENGTH - 1);

	#ifdef HDMI_DEBUG
	//chPtr = hdmi_GetProcArg(chPtr, chArg3, DEF_FILE_NAMELENGTH - 1);
	#endif
	if (chPtr == NULL) {
		//for avoid TQE:unused val 'chPtr';
		MT_DRV_PROC_EchoHelper("param len over Max namelen \n");
	}

	//debug print enable
	if (!mt_osal_strncmp(chArg1, "dbgs", DEF_FILE_NAMELENGTH)) {
		MT_U32 dgb;
		dgb = (MT_U32)simple_strtol(chArg2, NULL, 10);
		sSetEDIDPrintEn(dgb);
		COM_INFO("Si print:%d\n", dgb);
	} else if (!mt_osal_strncmp(chArg1, "dbgh", DEF_FILE_NAMELENGTH)) {
		g_hdmi_dbg_en = (MT_U32)simple_strtol(chArg2, NULL, 10);
		COM_INFO("hdmi drv print:%d\n", g_hdmi_dbg_en);
	} else if (!mt_osal_strncmp(chArg1, "debug", DEF_FILE_NAMELENGTH)) {
		int dgbon;
		dgbon = (MT_U32)simple_strtol(chArg2, NULL, 10);
		sSetHdmi_CommonDebug(dgbon);
		COM_INFO("hdmi common debug:%d\n", g_hdmi_dbg_en);
	}  else if (!mt_osal_strncmp(chArg1, "clk", DEF_FILE_NAMELENGTH)) {
		struct hdmi20_params *params = NULL;
		params = hdmi20_params_get();
		params->clk_cfg = (MT_U32)simple_strtol(chArg2, NULL, 10);
		COM_INFO("hdmi drv print:%d\n", g_hdmi_dbg_en);
	}
	//sw reset
	else if (!mt_osal_strncmp(chArg1, "swrst", DEF_FILE_NAMELENGTH)) {
		MT_DRV_PROC_EchoHelper("hdmi resetting... ... ... \n");
		SI_SW_ResetHDMITX();
	} else if (!mt_osal_strncmp(chArg1, "mute", DEF_FILE_NAMELENGTH)) {
		if (chArg2[0] == '1') {
			MT_DRV_PROC_EchoHelper("mute...  \n");
			DRV_HDMI_SetAVMute(0, MT_TRUE);
		} else if (chArg2[0] == '0') {
			MT_DRV_PROC_EchoHelper("unmute... \n");
			DRV_HDMI_SetAVMute(0, MT_FALSE);
		}
	} else if (!mt_osal_strncmp(chArg1, "dc", DEF_FILE_NAMELENGTH)) {
		HDMI_ATTR_S pstAttr;
		MT_UNF_HDMI_DEEP_COLOR_E dc;
		COM_INFO("Cmd set deep color to start \n");
		DRV_HDMI_GetAttr(MT_UNF_HDMI_ID_0, &pstAttr);
		switch (chArg2[0]) {
			case '0' :
				dc = MT_UNF_HDMI_DEEP_COLOR_24BIT;
				break;
			case '1' :
				dc = MT_UNF_HDMI_DEEP_COLOR_30BIT;
				break;
			case '2' :
				dc = MT_UNF_HDMI_DEEP_COLOR_36BIT;
				break;
			default:
				dc = MT_UNF_HDMI_DEEP_COLOR_BUTT;
				break;
		}
		pstAttr.stAppAttr.enDeepColorMode = dc;
		COM_INFO("Cmd set deep color to %d \n", dc);
		DRV_HDMI_SetAttr(MT_UNF_HDMI_ID_0, &pstAttr);
	}
	 else if (!mt_osal_strncmp(chArg1, "cs", DEF_FILE_NAMELENGTH)) {
		HDMI_ATTR_S pstAttr;
		MT_UNF_HDMI_VIDEO_MODE_E cs;
		COM_INFO("Cmd set color space start\n");
		DRV_HDMI_GetAttr(MT_UNF_HDMI_ID_0, &pstAttr);
		switch (chArg2[0]) {
			case '0' :
				cs = MT_UNF_HDMI_VIDEO_MODE_RGB444;
				break;
			case '1' :
				cs = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
				break;
			case '2' :
				cs = MT_UNF_HDMI_VIDEO_MODE_YCBCR420;
				break;
			default:
				cs = MT_UNF_HDMI_VIDEO_MODE_BUTT;
				break;
		}
		pstAttr.stAppAttr.enVidOutMode = cs;
		COM_INFO("Cmd set color space to %d \n", cs);
		DRV_HDMI_SetAttr(MT_UNF_HDMI_ID_0, &pstAttr);
	}
	else if (!mt_osal_strncmp(chArg1, "mtx0", DEF_FILE_NAMELENGTH)) {
		//echo can't input so many params
		mt_u8 i = 0;
		MT_CHAR  chArgn[5][DEF_FILE_NAMELENGTH] = {0};
		memcpy(&(chArgn[0][0]),chArg2,DEF_FILE_NAMELENGTH);
		g_test_mtx[0] = (uint16_t)simple_strtol(chArgn[0], NULL, 16);
		COM_INFO("mtx[%d] %4x \n", 0,g_test_mtx[0]);
		for (i=0;i<4;i++) {
			chPtr = hdmi_GetProcArg(chPtr, chArgn[i+1], DEF_FILE_NAMELENGTH - 1);
			g_test_mtx[i+1] = (uint16_t)simple_strtol(chArgn[i+1], NULL, 16);
			COM_INFO("mtx[%d] %4x \n", i+1,g_test_mtx[i+1]);
		}
	 }
	else if (!mt_osal_strncmp(chArg1, "mtx1", DEF_FILE_NAMELENGTH)) {
		//echo can't input so many params
		mt_u8 i = 0;
		MT_CHAR  chArgn[5][DEF_FILE_NAMELENGTH] = {0};
		memcpy(&(chArgn[0][0]),chArg2,DEF_FILE_NAMELENGTH);
		g_test_mtx[5] = (uint16_t)simple_strtol(chArgn[0], NULL, 16);
		COM_INFO("mtx[%d] %4x \n", 0,g_test_mtx[0]);
		for (i=0;i<4;i++) {
			chPtr = hdmi_GetProcArg(chPtr, chArgn[i+1], DEF_FILE_NAMELENGTH - 1);
			g_test_mtx[i+6] = (uint16_t)simple_strtol(chArgn[i+1], NULL, 16);
			COM_INFO("mtx[%d] %4x \n", i+6,g_test_mtx[i+6]);
		}
	 }
	else if (!mt_osal_strncmp(chArg1, "mtx2", DEF_FILE_NAMELENGTH)) {
		//echo can't input so many params
		mt_u8 i = 0;
		MT_CHAR  chArgn[5][DEF_FILE_NAMELENGTH] = {0};
		memcpy(&(chArgn[0][0]),chArg2,DEF_FILE_NAMELENGTH);
		g_test_mtx[10] = (uint16_t)simple_strtol(chArgn[0], NULL, 16);
		for (i=0;i<4;i++) {
			chPtr = hdmi_GetProcArg(chPtr, chArgn[i+1], DEF_FILE_NAMELENGTH - 1);
			g_test_mtx[i+11] = (uint16_t)simple_strtol(chArgn[i+1], NULL, 16);
			COM_INFO("mtx[%d] %4x \n", i+11,g_test_mtx[i+11]);
		}
		if (DRV_HDMI_Set_Csc_Mtx(0, g_test_mtx) == MT_FAILURE) {
			COM_INFO("mtx set fail, hdmi is not initialized\n");
		}
	 }
	else if (!mt_osal_strncmp(chArg1, "3d", DEF_FILE_NAMELENGTH)) {
		HDMI_ATTR_S stAttr;
		DRV_HDMI_GetAttr(0, &stAttr);
		if (chArg2[0] == '0') {
			MT_DRV_PROC_EchoHelper("3d mode disable...  \n");
			//MT_DRV_HDMI_Set3DMode(0,MT_FALSE,MT_UNF_3D_MAX_BUTT);
			stAttr.stVideoAttr.b3DEnable = MT_FALSE;
			stAttr.stVideoAttr.u83DParam = MT_UNF_EDID_3D_BUTT;
		} else if (!mt_osal_strncmp(chArg2, "fp", DEF_FILE_NAMELENGTH)) {
			MT_DRV_PROC_EchoHelper("Frame Packing... \n");
			//MT_DRV_HDMI_Set3DMode(0,MT_TRUE,MT_UNF_3D_FRAME_PACKETING);
			stAttr.stVideoAttr.b3DEnable = MT_TRUE;
			stAttr.stVideoAttr.u83DParam = MT_UNF_EDID_3D_FRAME_PACKETING;
		} else if (!mt_osal_strncmp(chArg2, "sbs", DEF_FILE_NAMELENGTH)) {
			MT_DRV_PROC_EchoHelper("Side by side(half)... \n");
			//MT_DRV_HDMI_Set3DMode(0,MT_TRUE,MT_UNF_3D_SIDE_BY_SIDE_HALF);
			stAttr.stVideoAttr.b3DEnable = MT_TRUE;
			stAttr.stVideoAttr.u83DParam = MT_UNF_EDID_3D_SIDE_BY_SIDE_HALF;
		} else if (!mt_osal_strncmp(chArg2, "tab", DEF_FILE_NAMELENGTH)) {
			MT_DRV_PROC_EchoHelper("Top and bottom... \n");
			//MT_DRV_HDMI_Set3DMode(0,MT_TRUE,MT_UNF_3D_TOP_AND_BOTTOM);
			stAttr.stVideoAttr.b3DEnable = MT_TRUE;
			stAttr.stVideoAttr.u83DParam = MT_UNF_EDID_3D_TOP_AND_BOTTOM;
		}
		DRV_HDMI_SetAttr(0, &stAttr);
	} else if (!mt_osal_strncmp(chArg1, "cbar", DEF_FILE_NAMELENGTH)) {
		MT_U32 u32Reg = 0;
		ulong reg_vaddr = 0;
		reg_vaddr = (ulong)ioremap(0xbf470000, 256);
		if ( reg_vaddr ) {
			if (chArg2[0] == '0') {
				MT_DRV_PROC_EchoHelper("colorbar disable...  \n");
				u32Reg = readl((void *)(reg_vaddr + 0x40));
				u32Reg &= (~0x80000000);
				writel(u32Reg, (void *)(reg_vaddr + 0x40));
			} else if (chArg2[0] == '1') {
				MT_DRV_PROC_EchoHelper("colorbar enable.. \n");
				u32Reg = readl((void *)(reg_vaddr + 0x40));
				u32Reg |= 0x80000000;
				writel(u32Reg, (void *)(reg_vaddr + 0x40));
			}
			iounmap((void *)reg_vaddr);
		}
	} else if (!mt_osal_strncmp(chArg1, "vblank", DEF_FILE_NAMELENGTH)) {
		MT_U8 BlankValue[3];

		if (chArg2[0] == '0') {
			MT_DRV_PROC_EchoHelper("vblank disable...  \n");
			SiiDrvCraClrBit8((SiiInst_t)NULL, REG_ADDR__VP__OUTPUT_BLANK_CONFIG, BIT_MSK__VP__OUTPUT_BLANK_CONFIG__ENABLE_ACTIVE_OVERRIDE);
		} else if (!mt_osal_strncmp(chArg2, "black", DEF_FILE_NAMELENGTH)) {
			if (get_current_rgb_mode(MT_UNF_HDMI_ID_0)) {
				BlankValue[0] = 0x00;
				BlankValue[1] = 0x00;
				BlankValue[2] = 0x00;
			} else {
				BlankValue[0] = 0x10;
				BlankValue[1] = 0x80;
				BlankValue[2] = 0x80;
			}
			MT_DRV_PROC_EchoHelper("vblank black.. \n");
			SI_SetBpssVidVal((uint8_t)BlankValue[0], (uint8_t)BlankValue[1], (uint8_t)BlankValue[2]);
			SiiDrvCraSetBit8((SiiInst_t)NULL, REG_ADDR__VP__OUTPUT_BLANK_CONFIG, BIT_MSK__VP__OUTPUT_BLANK_CONFIG__ENABLE_ACTIVE_OVERRIDE | 0x40);
		} else if (!mt_osal_strncmp(chArg2, "red", DEF_FILE_NAMELENGTH)) {
			if (get_current_rgb_mode(MT_UNF_HDMI_ID_0)) {
				BlankValue[0] = 0x00;
				BlankValue[1] = 0x00;
				BlankValue[2] = 0xff;
			} else {
				BlankValue[0] = 0x00;
				BlankValue[1] = 0x00;
				BlankValue[2] = 0xff;
			}
			MT_DRV_PROC_EchoHelper("vblank red.. \n");
			SI_SetBpssVidVal((uint8_t)BlankValue[0], (uint8_t)BlankValue[1], (uint8_t)BlankValue[2]);
			SiiDrvCraSetBit8((SiiInst_t)NULL, REG_ADDR__VP__OUTPUT_BLANK_CONFIG, BIT_MSK__VP__OUTPUT_BLANK_CONFIG__ENABLE_ACTIVE_OVERRIDE | 0x40);
		} else if (!mt_osal_strncmp(chArg2, "green", DEF_FILE_NAMELENGTH)) {
			if (get_current_rgb_mode(MT_UNF_HDMI_ID_0)) {
				BlankValue[0] = 0xff;
				BlankValue[1] = 0x00;
				BlankValue[2] = 0x00;
			} else {
				BlankValue[0] = 0xff;
				BlankValue[1] = 0x00;
				BlankValue[2] = 0x00;
			}
			MT_DRV_PROC_EchoHelper("vblank green.. \n");
			SI_SetBpssVidVal((uint8_t)BlankValue[0], (uint8_t)BlankValue[1], (uint8_t)BlankValue[2]);
			SiiDrvCraSetBit8((SiiInst_t)NULL, REG_ADDR__VP__OUTPUT_BLANK_CONFIG, BIT_MSK__VP__OUTPUT_BLANK_CONFIG__ENABLE_ACTIVE_OVERRIDE | 0x40);
		} else if (!mt_osal_strncmp(chArg2, "blue", DEF_FILE_NAMELENGTH)) {
			if (get_current_rgb_mode(MT_UNF_HDMI_ID_0)) {
				BlankValue[0] = 0x00;
				BlankValue[1] = 0xff;
				BlankValue[2] = 0x00;
			} else {
				BlankValue[0] = 0x00;
				BlankValue[1] = 0xff;
				BlankValue[2] = 0x00;
			}
			MT_DRV_PROC_EchoHelper("vblank blue.. \n");
			SI_SetBpssVidVal((uint8_t)BlankValue[0], (uint8_t)BlankValue[1], (uint8_t)BlankValue[2]);
			SiiDrvCraSetBit8((SiiInst_t)NULL, REG_ADDR__VP__OUTPUT_BLANK_CONFIG, BIT_MSK__VP__OUTPUT_BLANK_CONFIG__ENABLE_ACTIVE_OVERRIDE | 0x40);
		}
	} else if (!mt_osal_strncmp(chArg1, "audio", DEF_FILE_NAMELENGTH)) {
		HDMI_AUDIO_ATTR_S stHDMIAOAttr;
		memset((void*)&stHDMIAOAttr, 0, sizeof(HDMI_AUDIO_ATTR_S));
		DRV_HDMI_GetAOAttr(0, &stHDMIAOAttr);

		if (chArg2[0] == '0') {
			MT_DRV_PROC_EchoHelper("audio I2S \n");
			stHDMIAOAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_I2S;
			DRV_HDMI_AudioChange(0, &stHDMIAOAttr);
		} else if (chArg2[0] == '1') {
			MT_DRV_PROC_EchoHelper("audio SPDIF \n");
			stHDMIAOAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_SPDIF;
			DRV_HDMI_AudioChange(0, &stHDMIAOAttr);
		} else if (chArg2[0] == '2') {
			MT_DRV_PROC_EchoHelper("audio HBR \n");
			stHDMIAOAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_HBR;
			DRV_HDMI_AudioChange(0, &stHDMIAOAttr);
		} else if (chArg2[0] == '3') {
			MT_DRV_PROC_EchoHelper("audio pcm 8ch 192Khz \n");
			stHDMIAOAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_I2S;
			stHDMIAOAttr.bIsMultiChannel = MT_TRUE;
			stHDMIAOAttr.u32Channels = 8;
			stHDMIAOAttr.enSampleRate = MT_UNF_SAMPLE_RATE_192K;
			DRV_HDMI_AudioChange(0, &stHDMIAOAttr);
		} else if (chArg2[0] == '4') {
			MT_DRV_PROC_EchoHelper("audio pcm 2ch 48Khz \n");
			stHDMIAOAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_I2S;
			stHDMIAOAttr.bIsMultiChannel = MT_FALSE;
			stHDMIAOAttr.u32Channels = 2;
			stHDMIAOAttr.enSampleRate = MT_UNF_SAMPLE_RATE_48K;
			DRV_HDMI_AudioChange(0, &stHDMIAOAttr);
		} else {
			MT_DRV_PROC_EchoHelper("not supported \n");
		}
	} else if (!mt_osal_strncmp(chArg1, "thread", DEF_FILE_NAMELENGTH)) {
		if (chArg2[0] == '0') {
			MT_DRV_PROC_EchoHelper("thread stop \n");
			DRV_Set_ThreadStop(MT_TRUE);
		} else if (chArg2[0] == '1') {
			MT_DRV_PROC_EchoHelper("thread start \n");
			DRV_Set_ThreadStop(MT_FALSE);
		}
	} else if (!mt_osal_strncmp(chArg1, "cec", DEF_FILE_NAMELENGTH)) {
		#if defined (CEC_SUPPORT)
		if (chArg2[0] == '0') {
			HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();
			//cec_enable_flag = 0;
			DRV_Set_CECEnable(MT_UNF_HDMI_ID_0, MT_FALSE);
			DRV_Set_CECStart(MT_UNF_HDMI_ID_0, MT_FALSE);
			pstChnAttr[MT_UNF_HDMI_ID_0].u8CECCheckCount = 0;
			memset(&(pstChnAttr[MT_UNF_HDMI_ID_0].stCECStatus), 0, sizeof(MT_UNF_HDMI_CEC_STATUS_S));
			(MT_VOID)SI_CEC_Close();
		} else if (chArg2[0] == '1') {
			SI_CEC_SetUp();
			DRV_Set_CECEnable(MT_UNF_HDMI_ID_0, MT_TRUE);
			(MT_VOID)SI_CEC_Open();
		} else if (chArg2[0] == '2') {
			HDMI_CEC_S pCECCmd = {0};
			memset(&pCECCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
			pCECCmd.enHdmi = MT_UNF_HDMI_ID_0;
			pCECCmd.CECCmd.u8Opcode = CEC_OPCODE_USER_CONTROL_PRESSED;
			pCECCmd.CECCmd.unOperand.stRawData.u8Data[0] = MT_UNF_CEC_UICMD_VOLUME_DOWN;
			pCECCmd.CECCmd.unOperand.stRawData.u8Length = 1;
			DRV_HDMI_SetCECCommand(pCECCmd.enHdmi, &(pCECCmd.CECCmd));
		} else if (chArg2[0] == '3') {
			HDMI_CEC_S pCECCmd = {0};
			memset(&pCECCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
			pCECCmd.enHdmi = MT_UNF_HDMI_ID_0;
			pCECCmd.CECCmd.u8Opcode = CEC_OPCODE_USER_CONTROL_PRESSED;
			pCECCmd.CECCmd.unOperand.stRawData.u8Data[0] = MT_UNF_CEC_UICMD_VOLUME_UP;
			pCECCmd.CECCmd.unOperand.stRawData.u8Length = 1;
			DRV_HDMI_SetCECCommand(pCECCmd.enHdmi, &(pCECCmd.CECCmd));
		} else if (chArg2[0] == '4') {
			HDMI_CEC_S pCECCmd = {0};
			memset(&pCECCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
			pCECCmd.enHdmi = MT_UNF_HDMI_ID_0;
			pCECCmd.CECCmd.u8Opcode = CEC_OPCODE_STANDBY;
			pCECCmd.CECCmd.enDstAdd = (MT_U32)simple_strtol(chArg3, NULL, 10);
			DRV_HDMI_SetCECCommand(pCECCmd.enHdmi, &(pCECCmd.CECCmd));
		} else if (chArg2[0] == '5') {
			HDMI_CEC_S pCECCmd = {0};
			memset(&pCECCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
			pCECCmd.enHdmi = MT_UNF_HDMI_ID_0;
			pCECCmd.CECCmd.u8Opcode = CEC_OPCODE_IMAGE_VIEW_ON;
			pCECCmd.CECCmd.enDstAdd = (MT_U32)simple_strtol(chArg3, NULL, 10);;
			DRV_HDMI_SetCECCommand(pCECCmd.enHdmi, &(pCECCmd.CECCmd));
		} else if (chArg2[0] == '6') {
			HDMI_CEC_S pCECCmd = {0};
			memset(&pCECCmd, 0, sizeof(MT_UNF_HDMI_CEC_CMD_S));
			pCECCmd.enHdmi = MT_UNF_HDMI_ID_0;
			pCECCmd.CECCmd.u8Opcode = CEC_OPCODE_USER_CONTROL_PRESSED;//release msg auto send by drv
			pCECCmd.CECCmd.unOperand.stRawData.u8Data[0] = MT_UNF_CEC_UICMD_MUTE;
			pCECCmd.CECCmd.unOperand.stRawData.u8Length = 1;
			DRV_HDMI_SetCECCommand(pCECCmd.enHdmi, &(pCECCmd.CECCmd));
		}
		#else
		MT_DRV_PROC_EchoHelper("do not support cec \n");
		#endif
	} else if (!mt_osal_strncmp(chArg1, "setAttr", DEF_FILE_NAMELENGTH)) {
		HDMI_ATTR_S *pstHDMIAttr = DRV_Get_HDMIAttr(MT_UNF_HDMI_ID_0);
		DRV_Set_ForceUpdateFlag(MT_UNF_HDMI_ID_0, MT_TRUE);
		DRV_HDMI_SetAttr(MT_UNF_HDMI_ID_0, pstHDMIAttr);
	} else if (!mt_osal_strncmp(chArg1, "output", DEF_FILE_NAMELENGTH)) {
		if (chArg2[0] == '0') {
			MT_DRV_PROC_EchoHelper("output normal \n");
			DRV_Set_ForceOutputMode(MT_FALSE);
		} else if (chArg2[0] == '1') {
			MT_DRV_PROC_EchoHelper("output force mode \n");
			DRV_Set_ForceOutputMode(MT_TRUE);
		}
	} else {
		MT_DRV_PROC_EchoHelper("--------------------------------- HDMI debug options --------------------------------\n");
		MT_DRV_PROC_EchoHelper("you can perform HDMI debug with such commond:\n");
		MT_DRV_PROC_EchoHelper("echo [arg1] [arg2] [arg3] > /proc/msp/hdmi \n\n");
		MT_DRV_PROC_EchoHelper("debug action                arg1        arg2                        arg3\n");
		MT_DRV_PROC_EchoHelper("--------------------------  --------    --------------------        ---------------\n");
		MT_DRV_PROC_EchoHelper("hdmi drv print level                    dbgh        0 disable / 1~n enable \n");
		MT_DRV_PROC_EchoHelper("hdmi drv si print level                    dbgs        0 disable / 1~n enable \n");
		MT_DRV_PROC_EchoHelper("color space                    cs        0:RGB, 1:YUV444, 2:YUV420, 3:Auto \n");
		MT_DRV_PROC_EchoHelper("mtx0~2: set matrix for RGB output, NOTE: available after set cs to RGB output\n");
		MT_DRV_PROC_EchoHelper("mtx0                    mtx0        1111 2222 3333 4444 5555 \n");
		MT_DRV_PROC_EchoHelper("mtx1                    mtx1        6666 7777 8888 9999 aaaa \n");
		MT_DRV_PROC_EchoHelper("mtx2                    mtx2        bbbb cccc dddd eeee ffff \n");
		MT_DRV_PROC_EchoHelper("deep color                    dc        0:8bit, 1:10bit, 2:12bit, 3~x:Auto \n");
		MT_DRV_PROC_EchoHelper("colorbar                    cbar        0 disable / 1 enable \n");
		MT_DRV_PROC_EchoHelper("vblank(yuv data from hdmi)  vblank      0 /red / green/ blue/ black \n");
		MT_DRV_PROC_EchoHelper("software reset              swrst       no param \n");
		MT_DRV_PROC_EchoHelper("Avmute                      mute        0 unmute/ 1 mute \n");
		MT_DRV_PROC_EchoHelper("Set 3D InfoFrame            3d          0 disable3D /fp/sbs/tab  \n");
		MT_DRV_PROC_EchoHelper("Debug audio Change          audio       0 I2S / 1 SPdif / 2 HBR / 3 PCM 8ch 192KHz / 4 PCM 2ch 48KHz  \n");
		MT_DRV_PROC_EchoHelper("Thread stop/start           thread      0 stop / 1 start  \n");
		MT_DRV_PROC_EchoHelper("cec enable                  cec         0 disable / 1 enable \n");
		MT_DRV_PROC_EchoHelper("Force set attr              setAttr     no param \n");
		MT_DRV_PROC_EchoHelper("-------------------------------------------------------------------------------------\n");
	}
}

#endif

#if defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS)
#if 0
static ssize_t hdmi20_show(struct device *dev, struct device_attribute *attr, char *buffer)
{
	ssize_t count = 0;
	struct hdmi20_device *hdmi20_dev = dev_get_drvdata(dev);
	struct hdmi20_name *name;
	struct hdmi20_name *name_safe;
	struct hdmi20_thread *thread;
	struct hdmi20_thread *thread_safe;
	struct hdmi20_buffer *buf;
	int i;

	HDMI20_PRINT_FUNC_ENTER();
	mutex_lock(&hdmi20_dev->mutex_hdmi);
	count += snprintf(buffer + count, PAGE_SIZE - count, "open_cnt: %d\n", hdmi20_dev->open_cnt);
	list_for_each_entry_safe(name, name_safe, &hdmi20_dev->list_name, list) {
		count += snprintf(buffer + count, PAGE_SIZE - count,
						  "	name_cnt: %d, name: %s\n",
						  name->name_cnt,
						  name->name);
		list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
			count += snprintf(buffer + count, PAGE_SIZE - count,
							  "		tid_cnt: %d, tid: %d\n",
							  thread->tid_cnt,
							  thread->tid);
			for (i = 0; i < HDMI20_BUFFERS_PER_THREAD; i++) {
				if (thread->buffers[i] != NULL) {
					buf = thread->buffers[i];
					#if defined(CONFIG_MT_CHIP_ETUDE2)
					count += snprintf(buffer + count, PAGE_SIZE - count,
									  "			buf_cnt: %d, in_size = %d, out_size = %d, buf_id: %d\n",
									  buf->buf_cnt,
									  buf->in_sMBuf.u32Size, buf->out_sMBuf.u32Size,
									  i);
					#else
					//sym6
					count += snprintf(buffer + count, PAGE_SIZE - count,
									  "			buf_cnt: %d, in_size = %ld, out_size = %ld, buf_id: %d\n",
									  buf->buf_cnt,
									  buf->in_sMBuf.size, buf->out_sMBuf.size,
									  i);
					#endif
				}
			}
		}
	}
	mutex_unlock(&hdmi20_dev->mutex_hdmi);
	HDMI20_PRINT_FUNC_EXIT();
	return count;
}
#else
static ssize_t hdmi20_show(struct device *dev, struct device_attribute *attr, char *buffer)
{
	ssize_t count = 0;
	struct hdmi20_device *hdmi20_dev = dev_get_drvdata(dev);

	HDMI20_PRINT_FUNC_ENTER();
	mutex_lock(&hdmi20_dev->mutex_hdmi);
	#if defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS)
	count = HDMI0_Sysfs(buffer, NULL);
	#endif
	mutex_unlock(&hdmi20_dev->mutex_hdmi);
	HDMI20_PRINT_FUNC_EXIT();
	return count;
}
#endif

static ssize_t hdmi20_store(struct device *dev, struct device_attribute *attr, const char *buffer, size_t count)
{
	struct hdmi20_device *hdmi20_dev = dev_get_drvdata(dev);
	HDMI20_PRINT_FUNC_ENTER();
	mutex_lock(&hdmi20_dev->mutex_hdmi);
	count = hdmi_ProcWrite_Common((struct file *)NULL, buffer, count, 0, 0);
	mutex_unlock(&hdmi20_dev->mutex_hdmi);
	HDMI20_PRINT_FUNC_EXIT();
	return count;
}

static ssize_t sink_show(struct device *dev, struct device_attribute *attr, char *buffer)
{
	ssize_t count = 0;
	struct hdmi20_device *hdmi20_dev = dev_get_drvdata(dev);
	HDMI20_PRINT_FUNC_ENTER();
	mutex_lock(&hdmi20_dev->mutex_hdmi);

	#if defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS)
	count = HDMI0_Sink_Proc_Sysfs(buffer, NULL);
	#endif
	mutex_unlock(&hdmi20_dev->mutex_hdmi);
	HDMI20_PRINT_FUNC_EXIT();
	return count;
}
#endif

#if defined(CONFIG_MT_HDMI_V20_DEBUG_PROC_FS)
mt_s32 hdmi_ProcWrite(struct file *file,
					  const char __user *buf, size_t count, loff_t *ppos)
{
	HDMI20_PRINT_FUNC_ENTER();
	count = hdmi_ProcWrite_Common(file, buf, count, ppos, 1);
	HDMI20_PRINT_FUNC_EXIT();
	return (mt_s32)count;
}
#endif

#if HDMI20_USE_INTERRUPT
#include "si_vidpath_regs.h"
static uint32_t g_emp_intr_send_ok_cnt = 0;
static uint32_t g_emp_intr_send_fail_cnt = 0;
static uint32_t g_emp_intr_cpu_err_cnt = 0;
static uint32_t g_emp_intr_cnt = 0;
static mt_u32 g_emp_send_cnt;
mt_u32 g_test_emp_mtw_intr_cnt = 0;
mt_u32 g_test_emp_dma_done_intr_cnt = 0;

static irqreturn_t hdmi20_irq_handler(int irq, void *dev_id)
{
	uint8_t reg_tmp;
	SiiDrvCraAddr_t reg_addr;
	int empm = 0;

	//HDMI20_PRINT_FUNC_ENTER();
	//HDMI20_PRINTK("\nirq:%d\n",irq);

	// set intr mask
	reg_addr = REG_ADDR__INTR_STATUS;
	reg_tmp = SiiDrvCraRdReg8((SiiInst_t)0, reg_addr);
	empm = SiiDrvCraRdReg8((SiiInst_t)0, REG_ADDR__EMP_CTRL1);
	//HDMI20_PRINTK("\nhdmi20_irq_handler irq:%d,0x%x\n",irq,reg_tmp);
	g_emp_intr_cnt++;
	if ( reg_tmp ) {
		if ( empm & 0x01 ){
			if ( reg_tmp & BIT_MSK_REG__EMP_ERR_CPU) {
				g_emp_intr_cpu_err_cnt++;
				HDMI20_PRINTK("\nirq emp cpu err:0x%x\n",(uint32_t)reg_tmp);
			}
			if ( reg_tmp & BIT_MSK_REG__EMP_ERR_HDMI) {
				g_emp_intr_send_fail_cnt++;
				HDMI20_PRINTK("\nirq emp send fail:0x%x\n",(uint32_t)reg_tmp);
			}
			if ( reg_tmp & BIT_MSK_REG__EMP_SUCCESS_HDMI) {
				g_emp_intr_send_ok_cnt++;
				HDMI20_PRINTK("\nirq emp send success:0x%x\n",(uint32_t)reg_tmp);
			}
		}
		if ( reg_tmp & BIT_MSK_REG__MTW_FALLING_EDGE) {
			//g_emp_intr_send_ok_cnt++;
			g_test_emp_mtw_intr_cnt++;
			if ( g_test_emp_mtw_intr_cnt % 100 == 0 ) {
				HDMI20_PRINTK("\nirq emp falling edge:0x%x\n", reg_tmp);
			}
		}
		if ( reg_tmp & BIT_MSK_REG__DMA_DONE) {
			//g_emp_intr_send_ok_cnt++;
			g_test_emp_dma_done_intr_cnt++;
			if ( g_test_emp_dma_done_intr_cnt % 100 == 0 ) {
				HDMI20_PRINTK("\nirq emp dma done:0x%x\n", reg_tmp);
			}
		}
		#if 0
		if (g_emp_send_cnt != g_emp_intr_send_ok_cnt || \
				(g_emp_intr_send_ok_cnt % 100 == 1)) {
			HDMI20_PRINTK("\nemp info %d_%d,%d,%d\n", \
						  (uint32_t)g_emp_intr_send_ok_cnt, (uint32_t)g_emp_send_cnt, \
						  (uint32_t)g_emp_intr_send_fail_cnt, \
						  (uint32_t)g_emp_intr_cpu_err_cnt);
		}
		#else
		if ( g_emp_intr_cnt % 100 == 0 ) {
			HDMI20_PRINTK("\nemp intr %d: %d_%d,%d,%d, %d\n", (uint32_t)g_emp_intr_cnt, \
						  (uint32_t)g_emp_intr_send_ok_cnt, (uint32_t)g_emp_send_cnt, \
						  (uint32_t)g_emp_intr_send_fail_cnt, \
						  (uint32_t)g_emp_intr_cpu_err_cnt, g_test_emp_dma_done_intr_cnt);
		}
		#endif
		SiiDrvCraWrReg8((SiiInst_t)0, reg_addr, (uint8_t)reg_tmp);
	}

	return IRQ_HANDLED;
}
#endif

#define MREAD(A) (*((volatile unsigned int *)(A)))
#define MWRITE(A, V) (*((volatile unsigned int *)(A)) = (V))
static struct task_struct * g_aud_cfg_task = NULL;
//static mmz_buffer_s g_aud_buff_mmz[4];//bit0:aout, bit1:pp, bit2:spd, bit3:mix
static int aud_cfg_task(void *data)
{
	mt_s32 ret = 0;
	mt_u32 reg_204H = 0;
	mt_u32 Channels = 2;
	mt_u32 BitDepth = 16;
	mt_u32 SampleRate = 0;
	HDMI_AUDIOINTERFACE_E SoundIntf = HDMI_AUDIO_INTERFACE_I2S;
	MT_UNF_EDID_AUDIO_FORMAT_CODE_E AudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM;
	MT_UNF_SAMPLE_RATE_E samp_rate_table[14] = {
		MT_UNF_SAMPLE_RATE_48K,//48000
		MT_UNF_SAMPLE_RATE_8K,//8000
		MT_UNF_SAMPLE_RATE_11K,//11025
		MT_UNF_SAMPLE_RATE_12K,//1200
		MT_UNF_SAMPLE_RATE_16K,//16000
		MT_UNF_SAMPLE_RATE_22K,//22050
		MT_UNF_SAMPLE_RATE_24K,//24000
		MT_UNF_SAMPLE_RATE_32K,//3200
		MT_UNF_SAMPLE_RATE_44K,//44100
		MT_UNF_SAMPLE_RATE_48K,//48000
		MT_UNF_SAMPLE_RATE_88K,//8820
		MT_UNF_SAMPLE_RATE_96K,//96000
		MT_UNF_SAMPLE_RATE_176K,//176400
		MT_UNF_SAMPLE_RATE_192K,//192000
	};
	#if 0
	mt_u32 buff_idx = 0;
	mt_u32 buff_len = 0;
	mt_u32 is_av_4g_mmz = 0;
	char buff_name[32] = {0};
	#endif
	void __iomem * addr_204H = ioremap(0xbf490204, 4);
	HDMI_AUDIO_ATTR_S aud_cfg_attr;

	msleep_interruptible(30000);
	pr_info("<TN>%s:L%d:running...\n", __FUNCTION__, __LINE__);
	while (1) {
		reg_204H = MREAD(addr_204H);
		if (reg_204H) {
			AudioCode = 0xf & reg_204H;
			if ((MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM == AudioCode) ||
					(MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3 == AudioCode) ||
					(MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP == AudioCode) ||
					(MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS == AudioCode) ||
					(MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS_HD == AudioCode)) {
				SoundIntf = 0xf & (reg_204H >> 4);
				SampleRate = 0xf & (reg_204H >> 8);
				Channels = 0xf & (reg_204H >> 12);
				BitDepth = 0xff & (reg_204H >> 16);
				memset(&aud_cfg_attr, 0, sizeof(HDMI_AUDIO_ATTR_S));
				aud_cfg_attr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_BUTT > AudioCode ? AudioCode : MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM;
				aud_cfg_attr.enSoundIntf = HDMI_AUDIO_INTERFACE_BUTT > SoundIntf ? SoundIntf : HDMI_AUDIO_INTERFACE_I2S;
				aud_cfg_attr.enSampleRate = 14 > SampleRate ? samp_rate_table[SampleRate] : MT_UNF_SAMPLE_RATE_48K;
				if ((MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP == AudioCode) || (MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS_HD == AudioCode)) {
					aud_cfg_attr.enSampleRate *= 4;
				}
				aud_cfg_attr.enBitDepth = BitDepth ? BitDepth : 16;
				aud_cfg_attr.u32Channels = Channels ? Channels : 2;
				if (2 < aud_cfg_attr.u32Channels) {
					aud_cfg_attr.bIsMultiChannel = MT_TRUE;
				} else {
					aud_cfg_attr.bIsMultiChannel = MT_FALSE;
				}
				aud_cfg_attr.u8DownSampleParm = 0;
				aud_cfg_attr.u8I2SCtlVbit = 0;
				ret = MT_DRV_HDMI_AudioChange(MT_UNF_HDMI_ID_0, &aud_cfg_attr);
				if (MT_SUCCESS != ret) {
					pr_info("<TN>%s:L%d:aud2hdmi cfg FAIL[%d/%d|%d-%d-%d]\n", __FUNCTION__, __LINE__,
						   aud_cfg_attr.enAudioCode, aud_cfg_attr.enSoundIntf,
						   aud_cfg_attr.enSampleRate, aud_cfg_attr.u32Channels, aud_cfg_attr.enBitDepth);
				} else {
					pr_info("<TN>%s:L%d:aud2hdmi cfg SUCCESS[%d/%d|%d-%d-%d]\n", __FUNCTION__, __LINE__,
						   aud_cfg_attr.enAudioCode, aud_cfg_attr.enSoundIntf,
						   aud_cfg_attr.enSampleRate, aud_cfg_attr.u32Channels, aud_cfg_attr.enBitDepth);
				}
			} else {
				pr_info("<TN>L%d:CMD input param error, 204H[%#x]!\n", __LINE__, reg_204H);
				pr_info("\n<Aud2Hdmi-HelpInfo>\n");
				pr_info("devmem 0xbf490204 32 [value]\n");
				pr_info("bit[3:0]: audio code<0x1-pcm, 0x2-ac3, 0x7-dts, 0xA-eac3, 0xB-dtsHD>, default as pcm\n");
				pr_info("bit[7:4]: HDMI interface<0x0-i2s, 0x1-spdif, 0x2-hbr>, default as i2s\n");
				pr_info("bit]11:8]: samplerate index<0x1-8K, 0x2-11.025K, 0x3-12K, 0x4-16K, 0x5-22.05K, 0x6-24K, 0x7-32K, 0x8-44.1, 0x9-48K, 0xA-88.2K, 0xB-96K, 0xC-176.4K, 0xD-192K>\n");
				pr_info("bit]15:12]: channel, default as 2\n");
				pr_info("bit]23:16]: bit depth, default as 16\n");
				pr_info("Example:\n");
				pr_info("PCM-48K-2ch-16bit: devmem 0xbf490204 32 0x102901\n");
				pr_info("AC3-48K-2ch-16bit: devmem 0xbf490204 32 0x102912\n");
				pr_info("EAC3-48K-2ch-16bit: devmem 0xbf490204 32 0x10291A\n");
				pr_info("HBR-dtdHD-48K-8ch-16bit: devmem 0xbf490204 32 0x10892B\n\n");
			}
			MWRITE(addr_204H, 0);
		}
		msleep_interruptible(1000);

		#if 0
		//audio buffer malloc from ZONE "av_4g"/"av"
		buff_idx = 0xf & (reg_204H >> 4);
		buff_len = 0xffffff & (reg_204H >> 8);
		if (0xA == (0xf & reg_204H)) {
			is_av_4g_mmz = 1;
		}
		pr_info("<TN>L%d:audio buffer malloc [av_4g]: idx/len[%x/%#x]av_4g[%x]\n", __LINE__, buff_idx, buff_len, is_av_4g_mmz);
		if (4 > buff_idx) {
			if (g_aud_buff_mmz[buff_idx].size && g_aud_buff_mmz[buff_idx].startVirAddr) {
				mt_drv_mmz_unmap_and_release(&g_aud_buff_mmz[buff_idx]);
				g_aud_buff_mmz[buff_idx].startVirAddr = NULL;
				pr_info("<TN>L%d:AudBuff[%x] unmap and release!\n", __LINE__, buff_idx);
			}

			memset(buff_name, 0, 32);
			snprintf(buff_name, 32, "AudBuff%d", buff_idx);
			memset(&g_aud_buff_mmz[buff_idx], 0, sizeof(mmz_buffer_s));
			g_aud_buff_mmz[buff_idx].size = buff_len;
			if (is_av_4g_mmz) {
				ret = mt_drv_mmz_alloc_and_map(buff_name, "av_4g", buff_len, 64, &g_aud_buff_mmz[buff_idx]);
			} else {
				ret = mt_drv_mmz_alloc_and_map(buff_name, "av", buff_len, 64, &g_aud_buff_mmz[buff_idx]);
			}
			if (MT_SUCCESS == ret) {
				pr_info("<TN>L%d:AudBuff[%x] malloc and map SUCCESS size/vir/phy[%#x|%#lx/%#x]\n", __LINE__,
					   buff_idx, (mt_u32)g_aud_buff_mmz[buff_idx].size, (long unsigned int)g_aud_buff_mmz[buff_idx].startVirAddr, (mt_u32)g_aud_buff_mmz[buff_idx].startPhyAddr);
			} else {
				memset(&g_aud_buff_mmz[buff_idx], 0, sizeof(mmz_buffer_s));
				pr_info("<TN>L%d:AudBuff[%x] malloc and map FAIL!\n", __LINE__, buff_idx);
			}
		}
		#endif
	}
	return 0;
}
static int hdmi_aud_cfg_task_creare(void)
{
	int err;
	struct sched_param param;

	if (NULL == g_aud_cfg_task) {
		g_aud_cfg_task = kthread_create(aud_cfg_task, NULL, "aud_cfg_task");
		if (IS_ERR(g_aud_cfg_task)) {
			printk("Unable to start aud_cfg_task thread!");
			err = PTR_ERR(g_aud_cfg_task);
			g_aud_cfg_task = NULL;
			return err;
		}
		pr_info("<TN>%s:L%d:aud_cfg_task created\n", __FUNCTION__, __LINE__);

		param.sched_priority = 99;
		sched_setscheduler(g_aud_cfg_task, SCHED_RR, &param);
		wake_up_process(g_aud_cfg_task);
	}

	return 0;
}

static void hdmi20_intr_mux_set(struct hdmi20_device *hdmi20_dev, uint32_t mode)
{
	SiiDrvCraAddr_t reg_addr;

	HDMI20_PRINT_FUNC_ENTER();
	HDMI20_PRINTK("\nset intr mode:%d\n", mode);
	reg_addr = REG_ADDR__INT_MUX;
	switch (mode) {
		case 1:  //all intr is enable for CPU
		case 2:  //only mtw and emp intr is enable for CPU
			HdmiPrivDrvPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_INT_MUX, mode);
			break;
		default:
			HdmiPrivDrvPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_INT_MUX, 1);
			break;
	}
	HDMI20_PRINT_FUNC_EXIT();
}

#if HDMI20_EMP_DMA
static void hdmi20_emp_mode_set(struct hdmi20_device *hdmi20_dev, HDMI20_EMP_MODE_T mode)
{
	//uint32_t reg32_tmp;
	SiiDrvCraAddr_t reg_addr;

	HDMI20_PRINT_FUNC_ENTER();
	HDMI20_PRINTK("\nset emp mode:%d\n", mode);

	//clear video path intr
	reg_addr = REG_ADDR__EMP_CTRL1;
	switch (mode) {
		case HDMI20_EMP_CPU_ONLY:
			HdmiPrivDrvSetBit32((ulong)NULL, reg_addr, BIT_MSK__REG_EMP_MODE);
			break;
		case HDMI20_EMP_DMA_DMA:
			HdmiPrivDrvPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_EMP_MODE | BIT_MSK__REG_CPU_EN, 0);
			break;
		case HDMI20_EMP_DMA_CPU:
			HdmiPrivDrvPutBit32((ulong)NULL, reg_addr, BIT_MSK__REG_EMP_MODE | BIT_MSK__REG_CPU_EN, BIT_MSK__REG_CPU_EN);
			break;
		default:
			HdmiPrivDrvSetBit32((ulong)NULL, reg_addr, BIT_MSK__REG_EMP_MODE);
			break;
	}
	hdmi20_dev->params_in.emp_mode = mode;
	HDMI20_PRINT_FUNC_EXIT();
}
#endif

static void hdmi20_ana_reg_init(struct hdmi20_device *hdmi20_dev)
{
	#if HDMI_ANA_REG_IN_HDMI
	struct hdmi20_params *params_in;
	params_in = &(hdmi20_drv->hdmi20_dev[0]->params_in);
	params_in->hdmi_ana_regs.ana_hdmi_ao_reg0 = ANA_AO_REG0;
	params_in->hdmi_ana_regs.ana_hdmi_tx_reg0 = HDMI_TX_REG0;
	params_in->hdmi_ana_regs.ana_hdmi_tx_reg1 = HDMI_TX_REG1;
	params_in->hdmi_ana_regs.ana_hdmi_tx_reg2 = HDMI_TX_REG2;
	params_in->hdmi_ana_regs.ana_reg_hdmi_test = REG_HDMI_TEST;
	params_in->hdmi_ana_regs.ana_clkgen_vhd_reg = REG_CLKGEN_VHDPLL;
	params_in->hdmi_ana_regs.ana_clkgen_vhdintp = REG_CLKGEN_VHDINTP;
	params_in->hdmi_ana_regs.ana_clkgen_vsdpll = REG_CLKGEN_VSDPLL;
	params_in->hdmi_ana_regs.ana_clkgen_vsdintp = REG_CLKGEN_VSDINTP;
	params_in->hdmi_ana_regs.ana_clkgen_pdsys_reg = REG_CLKGEN_PDSYS;
	params_in->hdmi_ana_regs.ana_clkgen_vhdssc_reg = REG_CLKGEN_VHDSSC;
	params_in->hdmi_ana_regs.ana_clkgen_vsdssc_reg = REG_CLKGEN_VSDSSC;
	params_in->hdmi_ana_regs.ana_disp_vout_clk = 0xbf50a600;
	params_in->hdmi_ana_regs.ana_disp_vout_clksel = 0xbf50a604;
	params_in->hdmi_ana_regs.ana_sys_block_rst = 0xbf50a60c;
	#if 1
	HDMI20_PRINTK("HDMI ANA REGS: \nao_reg0[0x%x]\nao_reg0[0x%x]\n", \
				  params_in->hdmi_ana_regs.ana_hdmi_ao_reg0, \
				  params_in->hdmi_ana_regs.ana_hdmi_tx_reg0);
	#endif
	#endif
}

static int hdmi20_probe(struct platform_device *pdev)
{
	struct hdmi20_device *hdmi20_dev = NULL;
	int ret = 0;
	int idx = 0;
	u32 devidx = -1;
	u32 i = 0;
	int irq;
	struct resource *r[HDMI20_IO_MEM_MAX];

	HDMI20_PRINT_FUNC_ENTER();
	for (i = 0; i < HDMI20_IO_MEM_MAX; i++) {
		r[i] = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (r[i]) {
			HDMI20_PRINTK("\n[%s_%d]r[%d]:0x%lx,0x%lx, 0x%lx\n", \
						  __func__, __LINE__, i, (ulong)(r[i]->start), (ulong)(r[i]->end), (ulong)(r[i]->end - r[i]->start + 1));
		} else {
			HDMI20_PRINTK("\n[%s_%d]get reg addr %d fail\n", __func__, __LINE__, i);
		}
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		HDMI20_PRINTK("\n[%s_%d]get irq number fail\n", __func__, __LINE__);
		ret = irq;
		goto fail_alloc_minor;
	} else {
		HDMI20_PRINTK("\n[%s_%d]0x%d\n", __func__, __LINE__, irq);
	}

	of_property_read_u32(pdev->dev.of_node, "devidx", &devidx);
	idx = (devidx == -1) ? 0 : devidx;
	if (idx >= hdmi20_drv->minors) {
		ret = -ENODEV;
		goto fail_alloc_minor;
	}

	hdmi20_dev = kzalloc(sizeof(struct hdmi20_device), GFP_KERNEL);
	if (!hdmi20_dev) {
		ret = -ENOMEM;
		goto fail_kzalloc_hdmi20_dev;
	}

	hdmi20_dev->valid = HDMI_DEV_MAGIC;
	hdmi20_dev->pdev = pdev;

	ret = of_property_read_u32(pdev->dev.of_node, "g-hdcp-en", &(hdmi20_dev->params_in.hdcp_en));
	if ( ret ) {
		HDMI20_PRINTK("\n[%s_%d]do not config g-hdcp-en in dts!!!\n", __func__, __LINE__);
		hdmi20_dev->params_in.hdcp_en = 0;
	}
	ret = of_property_read_u32(pdev->dev.of_node, "g-hdcp2x-en", &(hdmi20_dev->params_in.hdcp2x_en));
	if ( ret ) {
		HDMI20_PRINTK("\n[%s_%d]do not config g-hdcp2x-en in dts!!!\n", __func__, __LINE__);
		hdmi20_dev->params_in.hdcp2x_en = 0;
	}
	ret = of_property_read_u32(pdev->dev.of_node, "g-scdc-en", &(hdmi20_dev->params_in.scdc_en));
	if ( ret ) {
		HDMI20_PRINTK("\n[%s_%d]do not config g-scdc-en in dts!!!\n", __func__, __LINE__);
		hdmi20_dev->params_in.scdc_en = 0;
	}
	ret = of_property_read_u32(pdev->dev.of_node, "g-cec-en", &(hdmi20_dev->params_in.cec_en));
	if ( ret ) {
		HDMI20_PRINTK("\n[%s_%d]do not config g-cec-en in dts!!!\n", __func__, __LINE__);
		hdmi20_dev->params_in.cec_en = 0;
	}
	ret = of_property_read_u32(pdev->dev.of_node, "g-emp-mode", &(hdmi20_dev->params_in.emp_mode));
	if ( ret ) {
		HDMI20_PRINTK("\n[%s_%d]do not config g-emp-mode in dts!!!\n", __func__, __LINE__);
		hdmi20_dev->params_in.emp_mode = 0;
	}
	ret = of_property_read_u32(pdev->dev.of_node, "g-hdcp-enhance", &(hdmi20_dev->params_in.hdcp_enhance_cfg));
	if ( ret ) {
		HDMI20_PRINTK("\n[%s_%d]do not config g-hdcp-enhance in dts!!!\n", __func__, __LINE__);
		hdmi20_dev->params_in.hdcp_enhance_cfg = 0;
	}
	ret = of_property_read_u32(pdev->dev.of_node, "g-clk-cfg", &(hdmi20_dev->params_in.clk_cfg));
	if ( ret ) {
		HDMI20_PRINTK("\n[%s_%d]do not config g-clk-cfg in dts!!!\n", __func__, __LINE__);
		hdmi20_dev->params_in.clk_cfg = 0;
	}
	hdmi20_dev->params_in.hdcp2x_en = 1; //Always init hdcp2x to 1
	hdmi20_dev->params_in.scdc_en = 1;//Always init scdc to 1
	HDMI20_PRINTK("\n[%s_%d] hdcp:%u,hdcp2x:%u,scdc:%u,cec:%u,emp:%u,hdcp_enh:%u\n", __func__, __LINE__, \
				  hdmi20_dev->params_in.hdcp_en, hdmi20_dev->params_in.hdcp2x_en, hdmi20_dev->params_in.scdc_en, \
				  hdmi20_dev->params_in.cec_en, hdmi20_dev->params_in.emp_mode, hdmi20_dev->params_in.hdcp_enhance_cfg);

	hdmi20_dev->devt = MKDEV(MAJOR(hdmi20_drv->devt), hdmi20_drv->minor + idx);
	hdmi20_drv->hdmi20_dev[idx] = hdmi20_dev;
	hdmi20_dev->p_dev = &pdev->dev;

	INIT_LIST_HEAD(&hdmi20_dev->list_name);
	mutex_init(&hdmi20_dev->mutex_hdmi);
	HDMI20_PRINTK("\n[%s_%d] hdl:%px,idx=%d\n", __func__, __LINE__, hdmi20_dev, idx);

	if (devidx == -1) {
		hdmi20_dev->dev = device_create(hdmi20_class, NULL, hdmi20_dev->devt, NULL, UMAP_DEVNAME_HDMI20);
	} else {
		hdmi20_dev->dev = device_create(hdmi20_class, NULL, hdmi20_dev->devt, NULL, UMAP_DEVNAME_HDMI20"%d", idx);
	}
	if (IS_ERR(hdmi20_dev->dev)) {
		ret = PTR_ERR(hdmi20_dev->dev);
		goto fail_device_create;
	}

	#if defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS)
	if (device_create_file(&pdev->dev, &dev_attr_hdmi20)) {
		ret = -ENOENT;
		goto fail_device_create_file;
	}

	if (device_create_file(&pdev->dev, &dev_attr_sink)) {
		ret = -ENOENT;
		goto fail_device_create_sink_file;
	}
	#endif

	for (i = 0; i < HDMI20_IO_MEM_MAX; i++) {
		if ( r[i] ) {
			hdmi20_dev->regDig[i].mapbase = r[i]->start;
			hdmi20_dev->regDig[i].mapsize = r[i]->end - r[i]->start + 1;
			hdmi20_dev->regDig[i].membase = ioremap(hdmi20_dev->regDig[i].mapbase, hdmi20_dev->regDig[i].mapsize);
			//printk("reg:0x%lx, membase:0x%lx\n",(ulong)hdmi20_dev->regDig[i].mapbase,(ulong)hdmi20_dev->regDig[i].membase);
			if (IS_ERR_OR_NULL(hdmi20_dev->regDig[i].membase)) {
				goto fail_ioremap;
			}
		} else {
			break;
		}
	}

	hdmi20_ana_reg_init(hdmi20_dev);
	ret = HDMI_DRV_Init();
	if (MT_SUCCESS != ret) {
		HDMI20_PRINTK("HDMI_DRV_Init failed\n");
		goto fail_drv_init;
	}
	//HDMI20_PRINTK("\n[%s_%d]0x%llx, %px\n", __func__, __LINE__, hdmi20_dev->regDig.mapsize, hdmi20_dev->regDig.membase);

	//TEST_JY
	ret = hdmi_aud_cfg_task_creare();
	pr_info("<TN>%s:L%d:AudCfg-Init-END:ret[%x]\n", __FUNCTION__, __LINE__, ret);

	hdmi20_dev->irq = irq;
	hdmi20_intr_mux_set(hdmi20_dev, 2);
	HDMI20_PRINTK("\n[%s_%d]0x%x\n", __func__, __LINE__, hdmi20_dev->irq);
	#if HDMI20_USE_INTERRUPT
	ret = request_irq(hdmi20_dev->irq, hdmi20_irq_handler, IRQF_TRIGGER_HIGH, dev_name(hdmi20_dev->dev), hdmi20_dev);
	if (ret) {
		goto fail_request_irq;
	}
	#endif
	{
	extern void SiiModTxHdcpOtpWrite(void);
	SiiModTxHdcpOtpWrite();
	}
	#if HDMI20_EMP_DMA
	hdmi20_emp_mode_set(hdmi20_dev, (HDMI20_EMP_MODE_T)(hdmi20_dev->params_in.emp_mode));
	//hdmi20_emp_mode_set(hdmi20_dev,(HDMI20_EMP_MODE_T)HDMI20_EMP_CPU_ONLY);
	HDMI20_PRINTK("\n[%s_%d]:%d\n", __func__, __LINE__, hdmi20_dev->params_in.emp_mode);
	if ( hdmi20_dev->params_in.emp_mode == HDMI20_EMP_DMA_DMA || hdmi20_dev->params_in.emp_mode == HDMI20_EMP_DMA_CPU) {
		ret = mt_drv_mmz_alloc_and_map("HDMI_EMP_BUFF", MT_NULL, \
									   HDMI20_EMP_BUFF_SIZE * HDMI20_EMP_BUFF_NUM, 0, (mmz_buffer_s*)(&(hdmi20_dev->emp_buf[0])));
		if ( ret != MT_SUCCESS) {

			#if HDMI20_USE_INTERRUPT
			goto fail_request_irq;
			#else
			goto fail_drv_init;
			#endif
		}
		HDMI20_PRINTK("\n[%s_%d]:%lu, %lx, %llx\n", __func__, __LINE__, \
					  hdmi20_dev->emp_buf[0].size, (ulong)hdmi20_dev->emp_buf[0].startVirAddr, (uint64_t)hdmi20_dev->emp_buf[0].startPhyAddr);
		hdmi20_dev->emp_buf[1].size = HDMI20_EMP_BUFF_SIZE;
		hdmi20_dev->emp_buf[1].startVirAddr = hdmi20_dev->emp_buf[0].startVirAddr + HDMI20_EMP_BUFF_SIZE;
		hdmi20_dev->emp_buf[1].startPhyAddr = hdmi20_dev->emp_buf[0].startPhyAddr + HDMI20_EMP_BUFF_SIZE;
	}
	#endif
	platform_set_drvdata(pdev, hdmi20_dev);
	dev_set_drvdata(hdmi20_dev->dev, hdmi20_dev);

	HDMI20_PRINT_FUNC_EXIT();

	return 0;

	#if HDMI20_USE_INTERRUPT
fail_request_irq:
	HDMI_DRV_EXIT();
	#endif
fail_drv_init:
	for (i = 0; i < HDMI20_IO_MEM_MAX; i++) {
		if ( hdmi20_dev->regDig[i].membase ) {
			iounmap(hdmi20_dev->regDig[i].membase);
		}
	}
fail_ioremap:
	#if defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS)
	device_remove_file(&pdev->dev, &dev_attr_sink);
fail_device_create_sink_file:
	device_remove_file(&pdev->dev, &dev_attr_hdmi20);
fail_device_create_file:
	#endif
	device_destroy(hdmi20_class, hdmi20_dev->devt);
	hdmi20_dev->dev = NULL;
fail_device_create:
	mutex_destroy(&hdmi20_dev->mutex_hdmi);
	hdmi20_drv->hdmi20_dev[idx] = NULL;
	kfree(hdmi20_dev);
	hdmi20_dev = NULL;
fail_kzalloc_hdmi20_dev:
fail_alloc_minor:
	return ret;
}

static int hdmi20_remove(struct platform_device *pdev)
{
	int idx;
	int i;
	struct hdmi20_device *hdmi20_dev = platform_get_drvdata(pdev);

	HDMI20_PRINT_FUNC_ENTER();
	if ( hdmi20_dev == NULL ) {
		return 0;
	}
	#if HDMI20_EMP_DMA
	if ( hdmi20_dev->emp_buf[0].startVirAddr ) {
		mt_drv_mmz_unmap_and_release(&(hdmi20_dev->emp_buf[0]));
		memset(hdmi20_dev->emp_buf, 0, sizeof(mmz_buffer_s)*HDMI20_EMP_BUFF_NUM);
	}
	#endif
	#if HDMI20_USE_INTERRUPT
	free_irq(hdmi20_dev->irq, hdmi20_dev);
	#endif
	HDMI_DRV_EXIT();
	for (i = 0; i < HDMI20_IO_MEM_MAX; i++) {
		if ( hdmi20_dev->regDig[i].membase ) {
			iounmap(hdmi20_dev->regDig[i].membase);
		}
	}
	#if defined(CONFIG_MT_HDMI_V20_DEBUG_SYSFS)
	device_remove_file(&pdev->dev, &dev_attr_sink);
	device_remove_file(&pdev->dev, &dev_attr_hdmi20);
	#endif
	device_destroy(hdmi20_class, hdmi20_dev->devt);
	idx = hdmi20_dev->devt - hdmi20_drv->devt;
	hdmi20_drv->hdmi20_dev[idx] = NULL;
	hdmi20_dev->dev = NULL;
	hdmi20_dev->valid = 0x0;

	mutex_destroy(&hdmi20_dev->mutex_hdmi);

	platform_set_drvdata(pdev, NULL);

	kfree(hdmi20_dev);
	hdmi20_dev = NULL;
	HDMI20_PRINT_FUNC_EXIT();

	return 0;
}

static int hdmi20_suspend(struct platform_device *pdev, pm_message_t stState)
{
	//struct hdmi20_device *hdmi20_dev = platform_get_drvdata(pdev);
	mt_s32 ret;
	pm_message_t a;
	HDMI20_PRINT_FUNC_ENTER();
	ret = hdmi_Suspend((basedev_s *)NULL, a);
	return ret;
}

static int hdmi20_resume(struct platform_device *pdev)
{
	mt_s32 ret;
	//struct hdmi20_device *hdmi20_dev = platform_get_drvdata(pdev);
	HDMI20_PRINT_FUNC_ENTER();
	ret = hdmi_Resume(NULL);
	return ret;
}

#if defined(CONFIG_OF)
static const struct of_device_id hdmi20_of_match[] = {
	{ .compatible = "montage,hdmi20" },
	{},
};
MODULE_DEVICE_TABLE(of, hdmi20_of_match);
#endif

static struct platform_driver hdmi20_platform_driver = {
	.probe = hdmi20_probe,
	.remove = hdmi20_remove,
	.suspend = hdmi20_suspend,
	.resume = hdmi20_resume,
	.driver = {
		.name = UMAP_DEVNAME_HDMI20,
		.of_match_table = of_match_ptr(hdmi20_of_match),
	}
};

int __init HDMI_DRV_ModInit(void)
{
	int  ret;

	HDMI20_PRINT_FUNC_ENTER();
	{
		#if defined(CONFIG_MT_HDMI_V20_DEBUG_PROC_FS)
		mt_proc_entry_t *pProcItem;
		mt_drv_proc_t stFnOpt = {
			.fnRead = HDMI0_Proc,
		};

		mt_drv_proc_t stFnSinkOpt = {
			.fnRead = HDMI0_Sink_Proc,
		};
		pProcItem = mt_drv_proc_add_module("hdmi0", &stFnOpt, NULL);
		if (pProcItem != MT_NULL) {
			pProcItem->write = hdmi_ProcWrite;
		}

		mt_drv_proc_add_module("hdmi0_sink", &stFnSinkOpt, NULL);
		#endif

		#if (defined(CONFIG_MT_HDMI_V20_DEBUG_PROC_FS) || defined(CONFIG_MT_HDMI_V20_DEBUG_PROC_FS))
		//Init Array
		hdmi_InitFmtArray();
		#endif
	}
	hdmi20_drv = kzalloc(sizeof(struct hdmi20_driver), GFP_KERNEL);
	if (!hdmi20_drv) {
		ret = -ENOMEM;
		goto fail_kzalloc_drv;
	}

	hdmi20_class = class_create(UMAP_DEVNAME_HDMI20);
	if (IS_ERR(hdmi20_class)) {
		ret = PTR_ERR(hdmi20_class);
		goto fail_class_create;
	}

	hdmi20_drv->minor = HDMI20_MINOR;
	hdmi20_drv->minors = HDMI20_MINORS;
	hdmi20_drv->devt = MKDEV(MT_DEVICE_MAJOR, hdmi20_drv->minor);
	hdmi20_drv->major = MAJOR(hdmi20_drv->devt);

	cdev_init(&hdmi20_drv->cdev, &hdmi20_fops);
	hdmi20_drv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&hdmi20_drv->cdev, hdmi20_drv->devt, hdmi20_drv->minors);
	if (ret) {
		ret = -EINVAL;
		goto fail_cdev_add;
	}

	ret = platform_driver_register(&hdmi20_platform_driver);
	if (ret) {
		goto fail_platform_driver_register;
	}

	HDMI20_PRINT_FUNC_EXIT();
	return 0;

fail_platform_driver_register:
	cdev_del(&hdmi20_drv->cdev);
fail_cdev_add:
	class_destroy(hdmi20_class);
	hdmi20_class = NULL;
fail_class_create:
	kfree(hdmi20_drv);
	hdmi20_drv = NULL;
fail_kzalloc_drv:
	return ret;
}

ulong HdmiDrvRegBaseGet(void *p, HDMI_RELATED_REG_MOD_E mod)
{
	//p = p;
	if ( hdmi20_drv->hdmi20_dev[0] ) {
		if (hdmi20_drv->hdmi20_dev[0]->valid != HDMI_DEV_MAGIC) {
			return ((ulong)NULL);
		} else {
			return ((ulong)(hdmi20_drv->hdmi20_dev[0]->regDig[mod].membase));
		}
	} else {
		return ((ulong)NULL);
	}
}

ulong HdmiDrvRegPhyBaseGet(void *p, HDMI_RELATED_REG_MOD_E mod)
{
	//p = p;
	if ( hdmi20_drv->hdmi20_dev[0] ) {
		if (hdmi20_drv->hdmi20_dev[0]->valid != HDMI_DEV_MAGIC) {
			return ((ulong)NULL);
		} else {
			return ((ulong)(hdmi20_drv->hdmi20_dev[0]->regDig[mod].mapbase));
		}
	} else {
		return ((ulong)NULL);
	}
}

ulong hdmi_paddr2vaddr(void *p, ulong addr)
{
	int i;
	ulong mask28b = 0x0fffffffUL;

	//p = p;
	for (i = 0; i < MT_RELATED_MODE_NUM; i++) {
		if ( addr < 0x10000 ) {
			//Si addr
			addr = (ulong)(hdmi20_drv->hdmi20_dev[0]->regDig[i].membase) + addr;
			break;
		} else if ( ( addr & mask28b ) >= (hdmi20_drv->hdmi20_dev[0]->regDig[i].mapbase & mask28b) && \
					( addr & mask28b ) < ((hdmi20_drv->hdmi20_dev[0]->regDig[i].mapbase & mask28b) + hdmi20_drv->hdmi20_dev[0]->regDig[i].mapsize) ) {
			addr = (ulong)(hdmi20_drv->hdmi20_dev[0]->regDig[i].membase) + (( addr & mask28b ) \
					- (ulong)((ulong)hdmi20_drv->hdmi20_dev[0]->regDig[i].mapbase & mask28b));
			break;
		}
	}
	if ( i == MT_RELATED_MODE_NUM ) {
		addr = 0UL;
		HDMI20_DRV_REG_PROC_PRINTK("\n[%s_%d] Reg[0x%lx] is not in dts table \n", __func__, __LINE__, (ulong)addr);
	} else {
		HDMI20_DRV_REG_PROC_PRINTK("\n[%s_%d]find 0x%lx base on table %d: 0x%lx:g_vir_hdmi_regaddr_map[0][i] in the table\n", __func__, __LINE__, (ulong)addr, i, (ulong)(hdmi20_drv->hdmi20_dev[0]->regDig[i].membase));
	}
	return addr;
}

HDMI20_EMP_MODE_T hdmi20_emp_mode_get(void)
{
	HDMI20_EMP_MODE_T mode = HDMI20_EMP_CPU_ONLY;
	if ( hdmi20_drv->hdmi20_dev[0] == NULL ) {
		mode = HDMI20_EMP_MODE_MAX;
	} else if (hdmi20_drv->hdmi20_dev[0]->valid != HDMI_DEV_MAGIC) {
		mode = HDMI20_EMP_MODE_MAX;
	} else {
		mode = hdmi20_drv->hdmi20_dev[0]->params_in.emp_mode;
	}
	return mode;
}

mt_s32 hdmi20_emp_mode_force_set(HDMI20_EMP_MODE_T mode)
{
	mt_s32 ret = MT_SUCCESS;
	if ( hdmi20_drv->hdmi20_dev[0] == NULL ) {
		ret = MT_FAILURE;
	} else if (hdmi20_drv->hdmi20_dev[0]->valid != HDMI_DEV_MAGIC) {
		ret = MT_FAILURE;
	} else {
		if ( hdmi20_emp_mode_get() == HDMI20_EMP_DMA_DMA || \
				hdmi20_emp_mode_get() == HDMI20_EMP_DMA_CPU ) {
			hdmi20_drv->hdmi20_dev[0]->params_in.emp_mode = mode;
		} else {
			ret = MT_FAILURE;
		}
	}
	return ret;
}

mmz_buffer_s hdmi20_emp_dma_buff_get(u32 idx)
{
	mmz_buffer_s mmz_buff = {0};
	if ( idx >= HDMI20_EMP_BUFF_NUM ) {
		return mmz_buff;
	}
	return hdmi20_drv->hdmi20_dev[0]->emp_buf[idx];
}

struct hdmi20_params *hdmi20_params_get(void)
{
	return &(hdmi20_drv->hdmi20_dev[0]->params_in);
}

void __exit hdmi20_drv_modexit(void)
{
	HDMI20_PRINT_FUNC_ENTER();
	cdev_del(&hdmi20_drv->cdev);

	#if defined(CONFIG_MT_HDMI_V20_DEBUG_PROC_FS)
	mt_drv_proc_rm_module("hdmi0");
	mt_drv_proc_rm_module("hdmi0_sink");
	#endif
	platform_driver_unregister(&hdmi20_platform_driver);

	class_destroy(hdmi20_class);
	hdmi20_class = NULL;

	kfree(hdmi20_drv);
	hdmi20_drv = NULL;
	HDMI20_PRINT_FUNC_EXIT();
}

mt_s32 DRV_HDMI_Register(mt_void)
{
	mt_s32 ret;

	ret = mt_drv_module_register((mt_u32)MT_ID_HDMI, HDMI_NAME, (mt_void *)&s_stHdmiExportFuncs);
	if (MT_SUCCESS != ret) {
		MT_ERR_HDMI("mt_drv_module_register failed\n");
		return ret;
	}
	return MT_SUCCESS;
}

mt_s32 DRV_HDMI_UnRegister(mt_void)
{
	mt_s32 ret;
	ret = mt_drv_module_unregister((mt_u32)MT_ID_HDMI);
	if (MT_SUCCESS != ret) {
		COM_FATAL("mt_drv_module_unregister failed\n");
		return ret;
	}
	return MT_SUCCESS;
}

MODULE_DESCRIPTION("ETUDE2 HDMI 2.0 driver ");
MODULE_AUTHOR("Montage Lz Inc.");
MODULE_LICENSE("GPL");
