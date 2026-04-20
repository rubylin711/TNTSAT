/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/poll.h>
//#include <mach/hardware.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/timekeeping.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <media/rc-core.h>

//#include <../../arch/arm/mach-aria/aria_reg_base_addr.h>
//#include "himedia.h"
//#include "common_dev.h"
//#include "common_proc.h"
//#include "common_stat.h"

//#include "priv_ir.h"
//#include "drv_ir_codedef.h"
//#include "mt_mach/irq.h"

#include "mt_mach/irq.h"

#include "mt_drv_ir.h"
#include "drv_ir_ioctl.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_module.h"
#include "mt_module_debug.h"
#include "mt_kernel_adapt.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

//if want use rc of kernel to reprot key, pls open this macro
//and the 'CONFIG_INPUT_EVDEV' and 'CONFIG_RC_CORE' should be also open.
//#define IR_RC_SUPPORT

typedef enum
{
  /*!
  NEC protocol, available now
  */
  IRDA_NEC = 0,
  /*!
  SW NEC protocol
  */
  IRDA_SW_NEC,
   /*!
  SW RC5 protocol
  */
  IRDA_SW_RC5,
  /*!
  SW RCMM protocol
  */
  IRDA_SW_RCMM,
  /*!
  SW PZOCN protocol
  */
  IRDA_SW_PZOCN,
   /*!
  SW RC6 protocol
  */
  IRDA_SW_RC6,
  /*!
   SW Panasonic 7051 protocol
   */
  IRDA_SW_PANASONIC_7051,
   /*!
  NECX protocol
  */
  IRDA_NECX,
   /*!
  MAX protocol
  */
  IRDA_SW_MAX
}irda_protocol_t_;  //old types

#if ((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE))
//static struct rc_dev *mt_ir_rc_dev = NULL;
#define MT_IR_NAME		"mt-ir-rc"

struct mt_ir_priv
{
	struct rc_dev		*rc_dev;
	enum rc_proto 		rc_proto;
};
#endif

///TODO:should modify here
#if defined(CONFIG_MT_CHIP_ARIA)
#define MIPS_CPU_NUM_IRQ					8
#define SYMPHONY_PIC_IRQ_BASE		MIPS_CPU_NUM_IRQ
#define IRQ_IRDA_ID            (40+SYMPHONY_PIC_IRQ_BASE)
#endif

#define MT_ERR_PARAM        (-3)
#define DEFAULT_SAMPLE_FREQ  1

#define R_IR_BASE_ADDR   	SYMPHONY_IO_VA(0xBF151000UL)
#define BOOT_CFG 			SYMPHONY_IO_VA(0xBF140020UL)

#define R_IR_NEC_DATA      R_IR_BASE_ADDR
#define R_IR_COM_DATA     (R_IR_BASE_ADDR + 0x04)
#define R_IR_GLOBAL_CTRL  (R_IR_BASE_ADDR + 0x08)
#define R_IR_GLOBAL_STA   (R_IR_BASE_ADDR + 0x0C)
#define R_IR_INT_CFG      (R_IR_BASE_ADDR + 0x10)
#define R_IR_INT_RAWSTA   (R_IR_BASE_ADDR + 0x14)
#define R_IR_NEC_FILT     (R_IR_BASE_ADDR + 0x18)
#define R_IR_COMBUF_POP   (R_IR_BASE_ADDR + 0x1C)
#define R_IR_NEC_START_ONTSET   (R_IR_BASE_ADDR + 0x40)
#define R_IR_NEC_START_PRDSET   (R_IR_BASE_ADDR + 0x44)
#define R_IR_NEC_REPEAT_ONTSET  (R_IR_BASE_ADDR + 0x48)
#define R_IR_NEC_REPEAT_PRDSET  (R_IR_BASE_ADDR + 0x4C)
#define R_IR_NEC_BIT1_ONTSET    (R_IR_BASE_ADDR + 0x50)
#define R_IR_NEC_BIT1_PRDSET    (R_IR_BASE_ADDR + 0x54)
#define R_IR_NEC_BIT0_ONTSET    (R_IR_BASE_ADDR + 0x58)
#define R_IR_NEC_BIT0_PRDSET    (R_IR_BASE_ADDR + 0x5C)
#define R_IR_WAVEFILT_CFG0  (R_IR_BASE_ADDR + 0x80)
#define R_IR_WAVEFILT_CFG1  (R_IR_BASE_ADDR + 0x84)
#define R_IR_WAVEFILT_CFG2  (R_IR_BASE_ADDR + 0x88)
#define R_IR_WAVEFILT_CFG3  (R_IR_BASE_ADDR + 0x8C)
#define R_IR_WAVEFILT_MASK  (R_IR_BASE_ADDR + 0x90)
#define R_IR_WFN_ONTSET  (R_IR_BASE_ADDR + 0x400)
#define R_IR_WFN_PRDSET  (R_IR_BASE_ADDR + 0x404)
#define GET_SYMPHONY_IR_WAVEFILT_MASK(n) (R_IR_BASE_ADDR + 0x90 + (4 * n))
#define GET_SYMPHONY_IR_WFN_ONTSET(n) (R_IR_BASE_ADDR + 0x400 + (8 * n))
#define GET_SYMPHONY_IR_WFN_PRDSET(n) (R_IR_BASE_ADDR + 0x404 + (8 * n))


#define MODE_SHIFT  1
#define IDLE_STA_SHIFT  2
#define SAMPLE_FREQ_SHIFT   3
#define OT_SET_SHIFT    20
#define REPEAT_CTRL_SHIFT   16
#define NOISE_NFILT_SHIFT   10
#define CHK_EN_SHIFT    9
#define RPTMODE_SHIFT   8
#define IE_NEC_DN_SHIFT 0
#define IE_NEC_UP_SHIFT 1
#define INT_FRQ_SHIFT   8
#define IE_COM_SHIFT    2
#define IE_WF_SHIFT 4
#define UFILTER_EN_SHIFT    31
#define KFILTER_EN_SHIFT    30
#define UFILTER_SET_SHIFT   8
#define KFILTER_SET_SHIFT   0
#define REPEAT_STA_SHIFT    0
#define KEY_SHIFT   16
#define COMDATA_SHIFT   2
#define RECEIVE_NUM_SHIFT   8
#define WF_LEN_SHIFT    8
#define WF_STADDR_SHIFT 0
#define WF_ONTH_SHIFT   16
#define WF_ONTL_SHIFT   0
#define WF_PRDH_SHIFT   16
#define WF_PRDL_SHIFT   0
#define IE_WF_SHIFT     4
#define OVERFLOW_SHIFT  1

#define IRDA_MAX_USER                16

#define  IR_DELAY_TIME 200
#define  IR_MAX_BUF 100

#define IR_PLUSE_INVAILD	(0)
#define IR_PLUSE_VAILD		(1)


#define  IR_BUF_HEAD g_IrAttr.IrKeyBuf[g_IrAttr.Head]
#define  IR_BUF_TAIL g_IrAttr.IrKeyBuf[g_IrAttr.Tail]
#define  IR_BUF_LAST g_IrAttr.IrKeyBuf[(g_IrAttr.Head == 0) ? (g_IrAttr.IrKeyBufLen - 1) : (g_IrAttr.Head - 1)]
//#define  INC_BUF(x, len) ((++(x)) % (len))


#define RC5_WF_IGNORE_T_BIT 1 // ignore T bit for RC5 wave-filter

#if DEFAULT_SAMPLE_FREQ //  Sample freq 187.5 KHz
static u16 rc5_sigtbl[8] = {133, 266, 266, 433, 433, 600, 533, 800};
#else //  Sample freq 46.875 KHz
static u16 rc5_sigtbl[8] = {34, 67, 67, 109, 109, 150, 134, 200};
#endif
static MT_BOOL is_ir_intialized = MT_FALSE;

/*!
  IRDA RC5 Decoder State
*/
typedef enum
{
    E_RC5_STA_IDLE = 0,
    E_RC5_STA_S1,
    E_RC5_STA_S2,
    E_RC5_STA_T,
    E_RC5_STA_USR,
    E_RC5_STA_KEY,
    E_RC5_STA_FINISH,
    E_RC5_STA_INVALID
} irda_pro_rc5_status_t;

/*!
  IRDA ECMM Decoder State
*/
#if DEFAULT_SAMPLE_FREQ //  Sample freq 187.5 KHz
//static u16 rcmm_sigtbl[11] = {10,40,  80, 110,   120, 145,   150, 180,   185, 210,4095};//ontime:10~40 period 00,01,10,11
static u16 rcmm_sigtbl[11] = {10, 60, 67, 100, 101, 131, 132, 163, 164, 350, 4095};//ontime:10~40 period 00,01,10,11
#else //  Sample freq 46.875 KHz
static u16 rcmm_sigtbl[11] = {2,10,20, 27, 30, 36, 37, 45, 46, 53,1023};
#endif
static u16 rcmm_sigtbl_20M[11] = {6,30,60, 83, 85, 107, 108, 135, 139, 263,4095};

typedef enum
{
   E_RCMM_STA_IDLE = 0,
   E_RCMM_STA_handle,
   E_RCMM_STA_KEY,
   E_RCMM_STA_FINISH,

   E_RCMM_STA_INVALID
} irda_pro_E_rcmm_status_t;

/*!
  IRDA NEC Decoder State
*/
typedef enum
{
  E_NEC_STA_IDLE = 0,
  E_NEC_STA_START,
  E_NEC_STA_USRL,
  E_NEC_STA_USRH,
  E_NEC_STA_KEY,
  E_NEC_STA_YEK,
  E_NEC_STA_REPEAT,
  E_NEC_STA_FINISH
} irda_pro_nec_status_t;

/*!
  IRDA NEC Decoder Signal
*/
typedef enum
{
  E_NEC_SIG_START = 0,
  E_NEC_SIG_0,
  E_NEC_SIG_1,
  E_NEC_SIG_REPEAT,
  E_NEC_SIG_INVALID
} irda_pro_nec_sig_t;

#if DEFAULT_SAMPLE_FREQ //  Sample freq 187.5 KHz
	static u16 nec_sigtbl[16] = {1180, 2192, 2583, 3288, 70, 180, 160, 270, 70, 180, 300, 540, 1180,
	        2192, 1476, 2583};//2280
	static u16 nec_sigtbl_24m[16] = {1048, 1948, 2297, 2922, 62, 160, 142, 240, 62, 160, 266, 480, 1048,
	        1948, 1312, 2297};
#else //  Sample freq 46.875 KHz
	static u16 nec_sigtbl[16] = {295, 548, 571, 822, 17, 50, 20, 68, 17, 50, 75, 135, 295,
		548, 369, 570};
	static u16 nec_sigtbl_24m[16] = {262, 487, 507, 730, 15, 44, 15, 60, 15, 44, 66, 120, 262,
		487, 328, 506};
#endif
typedef struct
{
	union
	{
		struct
		{
			MT_U16 usercode;
			MT_U16 code;
		} nec;

		struct
		{
			MT_U8 ucode;
			MT_U8 kcode;
		} rc5;

		struct
		{
			MT_U8 ucode;
			MT_U8 kcode;
		} rcmm;
	} r;

	MT_U32 state;			//represent which bit is parse
	MT_U16 *p_sigtbl;
	MT_U8 counter;			//how many bit has been parsed
	MT_U8 repeat_interval;
} irda_dec_result_t;

typedef struct
{
    u16 ontime;
    u16 period;
    u8 protocol;
} irda_dec_para_t;

static void irda_symphony_wave_wfn_set(mt_u8 n, mt_u16 ontime_l, mt_u16 ontime_h, mt_u16 period_l, mt_u16 period_h);

static void irda_symphony_wave_mask_bit_set(mt_u32 bit, mt_u32 value);

static struct ir_wavefilter_config g_ir_filter = {0};

#define  IR_INT_TIME 107
static unsigned char g_repeat_flag = 0;
static ktime_t g_ktcur, g_ktpre;
static s64 g_deviation  = 0;
static unsigned int g_rekey = 0;
static unsigned int irclk_base_time_ns = 0;
mt_u32 INC_BUF(mt_u32 x, mt_u32 len)
{
    x++;
    return x%len;
}

typedef struct
{
    mt_u32		user_code_set;
    mt_u32 		sw_user_code[IRDA_MAX_USER];

    mt_u32		Head;
    mt_u32		Tail;
    IR_KEY_S	IrKeyBuf[IR_MAX_BUF];
    mt_u32		IrKeyBufLen;

    ////MT_BOOL bEnable;
    ////MT_BOOL bKeyUp;
    ////MT_BOOL bRepkey;
    mt_u32	RepkeyDelayTime;
    mt_u32	IrBlockTime;

	spinlock_t Plusedata_lock;
    wait_queue_head_t IrKeyWaitQueue;
}IR_ATTR_S;

static mt_device_s g_IrRegisterData;
static atomic_t g_IrCount = ATOMIC_INIT(0);
MT_DECLARE_MUTEX(g_IrMutex);

static IR_ATTR_S g_IrAttr;
static struct ir_config ir_settings;
static ir_pluse_data_s g_IrPluseData;

#define ir_writel(A,V)        HAL_PUT_U32((volatile u32 *)(A), (u32)(V))
#define ir_readl(A)           HAL_GET_U32((volatile u32 *)(A))
#define ir_readb(A)           HAL_GET_U8((volatile u8 *)(A))

#if ((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE))
static void mt_ir_set_rc_protocol(irda_protocol_t proto)
{
	struct mt_ir_priv *priv = (struct mt_ir_priv *)g_IrRegisterData.priv;

	switch(proto)
	{
		case IRDA_NEC:
		case IRDA_SW_NEC:
			priv->rc_proto = RC_PROTO_NEC;
			break;

		case IRDA_SW_RC5:
			priv->rc_proto = RC_PROTO_RC5;
			break;

		default:
			priv->rc_proto = RC_PROTO_UNKNOWN;
			break;
	}
}
#endif

#if defined(CONFIG_IR_LIRC_CODEC)
static int store_lirc_events(struct rc_dev *dev, struct ir_raw_event *ev,u16 ontime,u16 period)
{
		ev->duration = ontime*irclk_base_time_ns;
		ev->pulse = true;
		ir_raw_event_store(dev, ev);
		ir_raw_event_handle(dev);
		ev->pulse = false;
		ev->duration = (period-ontime)*irclk_base_time_ns;
		if(ev->duration>18000000)//The duration over 18000000 ns,that means transmittion has finished
		{
			ev->reset = true;
			ir_raw_event_store(dev, ev);
			ir_raw_event_handle(dev);
			ev->reset = false;
		}
		else
		{
			ir_raw_event_store(dev, ev);
			ir_raw_event_handle(dev);
		}
		return 0;
}

static void handle_lirc_events(struct rc_dev *dev)
{
		ir_raw_event_handle(dev);
}

static void reset_lirc_events(struct rc_dev *dev)
{
	ir_raw_event_overflow(dev);
}
#endif
static MT_BOOL is_27M_clk(void)
{
	if( (ir_readl(BOOT_CFG)&(1<<30)) == 0)
	{
		//printk("###%s, this is 27M clk####\n",__func__);
		return MT_TRUE;
	}

	//printk("###%s, this is 24M clk####\n",__func__);
	return MT_FALSE;
}

static unsigned char irda_symphony_get_recv_num(void)
{
  return ((ir_readl(R_IR_GLOBAL_STA) >> 8) & 0xFF);
}

/*****************************************************************
* IRDA internal functions
*****************************************************************/
static void irda_symphony_get_ontime_and_period(unsigned short * p_ontime, unsigned short * p_period)
{
  unsigned long ptmp = 0;
  unsigned long dtmp = 0;

  unsigned short m1dat_ont;
  unsigned short m1dat_prd;
#if 0
  // Data FIFO Pop
  ptmp = R_IR_COMBUF_POP;
  dtmp = ir_readl(ptmp);
  dtmp |= 0x1;
  ir_writel(ptmp, dtmp);

  // Data FIFO fetch
  ptmp = R_IR_COM_DATA;
  dtmp = ir_readl(ptmp);

  m1dat_ont = dtmp & 0xFFFF;
  m1dat_prd = (dtmp & 0xFFFF0000) >> 16;

  *p_ontime = m1dat_ont;
  *p_period = m1dat_prd;
 #endif
  // Data FIFO Pop
  ptmp = R_IR_COMBUF_POP;
  dtmp = ir_readl(ptmp);
  dtmp |= 0x1;
  ir_writel(ptmp, dtmp);

  // pop 2nd to confirm pop success
  ptmp = R_IR_COMBUF_POP;
  dtmp = ir_readl(ptmp);
  dtmp |= 0x1;


  // Data FIFO fetch
  ptmp = R_IR_COM_DATA;
  dtmp = ir_readl(ptmp);

  m1dat_ont = dtmp & 0xFFFF;
  m1dat_prd = (dtmp & 0xFFFF0000) >> 16;

  *p_ontime = m1dat_ont;
  *p_period = m1dat_prd;
}

static void ir_softreset(void)
{
       //unsigned long reg32 = 0;
	unsigned short ontime=0;
	unsigned short period=0;
	unsigned short recv_num = 0;

	recv_num = irda_symphony_get_recv_num();

	while(recv_num--)
	{
		irda_symphony_get_ontime_and_period(&ontime, &period);
	  	MT_INFO_IR("(%d)#ontime:%d, period:%d\n", recv_num, ontime, period);
	}
	MT_INFO_IR("symphony irda clr buffer\n");
}

static void nec_time_cfg(void)
{
	//unsigned int t_smp_ms,clk_div,cfg_mult,cfg_mult_min,cfg_mult_max;

	//187.5k~~0.0053ms
	//90/0.053/4 = 9/0.0053/4

	//(187.5k/24M)*100M=781k~~~~~~~~0.00128ms
	//90/0.128/4 = 0.9/0.00128/4

	//////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~cfg_mult~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~				//-------------SIM
	////if((irdacfg.clk_sample == 0)) {clk_div = 8;		}
	////if((irdacfg.clk_sample == 1)) {clk_div = 32;    }
	////if((irdacfg.clk_sample == 2)) {clk_div = 128;   }
	////if((irdacfg.clk_sample == 3)) {clk_div = 512;   }
	////
	////t_smp_ms = clk_div/IRDA_CLK/1000;//0.00128ms
	////cfg_mult = 1/SPEED_MULT/t_smp_ms/4;
	////cfg_mult = 19.53;
	//////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~cfg_mult~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	//unsigned int start_on_h = 190;//9 * cfg_mult;			//175.2
	//unsigned int start_on_l = 160;//(9-0.42) * cfg_mult;	//167.5
	//unsigned int start_pd_h = 275;//13.5 * cfg_mult;		//261.6
	//unsigned int start_pd_l = 250;//13.5 * cfg_mult;
	//unsigned int repeat_on_h = 190;//9 * cfg_mult;			//175.2
	//unsigned int repeat_on_l = 160;//(9-0.42) * cfg_mult;	//167.5
	//unsigned int repeat_pd_h = 235;//11.25 * cfg_mult;		//219.7
	//unsigned int repeat_pd_l = 205;//11.25 * cfg_mult;
	//unsigned int data1_on_h = 18;//0.56 * cfg_mult;			//10.92
	//unsigned int data1_on_l = 2;//(0.56-0.42) * cfg_mult;	//2.73
	//unsigned int data1_pd_h = 58;//2.25 * cfg_mult;			//43.92
	//unsigned int data1_pd_l = 32;//2.25 * cfg_mult;
	//unsigned int data0_on_h = 20;//0.56 * cfg_mult;			//10.92
	//unsigned int data0_on_l = 2;//(0.56-0.42) * cfg_mult;	//2.73
	//unsigned int data0_pd_h = 32;//1.125 * cfg_mult;		//21.6
	//unsigned int data0_pd_l = 10;//1.125 * cfg_mult;


	////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~cfg_mult~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~				//-------------FPGA
	//if((irdacfg.clk_sample == 0)) {clk_div = 8;		}
	//if((irdacfg.clk_sample == 1)) {clk_div = 32;    }
	//if((irdacfg.clk_sample == 2)) {clk_div = 128;   }
	//if((irdacfg.clk_sample == 3)) {clk_div = 512;   }
	//
	//t_smp_ms = clk_div/IRDA_CLK/1000;//0.0053ms
	//cfg_mult = 1/SPEED_MULT/t_smp_ms/4;
	//cfg_mult = 47.1;
	////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~cfg_mult~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	/*
	*	default 27M param
	*/
	unsigned int start_on_h = 548;//9 * cfg_mult;			//175.2
	unsigned int start_on_l = 295;//(9-0.42) * cfg_mult;	//167.5
	unsigned int start_pd_h = 822;//13.5 * cfg_mult;		//261.6
	unsigned int start_pd_l = 571;//13.5 * cfg_mult;
	unsigned int repeat_on_h = 548;//9 * cfg_mult;			//175.2
	unsigned int repeat_on_l = 295;//(9-0.42) * cfg_mult;	//167.5
	unsigned int repeat_pd_h = 822;//11.25 * cfg_mult;		//219.7
	unsigned int repeat_pd_l = 512;//11.25 * cfg_mult;
	unsigned int data1_on_h = 40;//0.56 * cfg_mult;			//10.92
	unsigned int data1_on_l = 5;//(0.56-0.42) * cfg_mult;	//2.73
	unsigned int data1_pd_h = 130;//2.25 * cfg_mult;			//43.92
	unsigned int data1_pd_l = 75;//2.25 * cfg_mult;
	unsigned int data0_on_h = 40;//0.56 * cfg_mult;			//10.92
	unsigned int data0_on_l = 5;//(0.56-0.42) * cfg_mult;	//2.73
	unsigned int data0_pd_h = 65;//1.125 * cfg_mult;		//21.6
	unsigned int data0_pd_l = 40;//1.125 * cfg_mult;

	/*
	*	24M param
	*/
	unsigned int start_on_h_24m = 487;//9 * cfg_mult;			//175.2
	unsigned int start_on_l_24m = 262;//(9-0.42) * cfg_mult;	//167.5
	unsigned int start_pd_h_24m = 730;//13.5 * cfg_mult;		//261.6
	unsigned int start_pd_l_24m = 507;//13.5 * cfg_mult;
	unsigned int repeat_on_h_24m = 548;//9 * cfg_mult;			//175.2
	unsigned int repeat_on_l_24m = 262;//(9-0.42) * cfg_mult;	//167.5
	unsigned int repeat_pd_h_24m = 730;//11.25 * cfg_mult;		//219.7
	unsigned int repeat_pd_l_24m = 455;//11.25 * cfg_mult;
	unsigned int data1_on_h_24m = 35;//0.56 * cfg_mult;			//10.92
	unsigned int data1_on_l_24m = 4;//(0.56-0.42) * cfg_mult;	//2.73
	unsigned int data1_pd_h_24m = 115;//2.25 * cfg_mult;			//43.92
	unsigned int data1_pd_l_24m = 66;//2.25 * cfg_mult;
	unsigned int data0_on_h_24m = 35;//0.56 * cfg_mult;			//10.92
	unsigned int data0_on_l_24m = 4;//(0.56-0.42) * cfg_mult;	//2.73
	unsigned int data0_pd_h_24m = 57;//1.125 * cfg_mult;		//21.6
	unsigned int data0_pd_l_24m = 35;//1.125 * cfg_mult;

	if(is_27M_clk())
	{
		ir_writel(R_IR_NEC_START_ONTSET,(start_on_h<<16) | start_on_l);
		ir_writel(R_IR_NEC_START_PRDSET,(start_pd_h<<16) | start_pd_l);
		ir_writel(R_IR_NEC_REPEAT_ONTSET,(repeat_on_h<<16) | repeat_on_l);
		ir_writel(R_IR_NEC_REPEAT_PRDSET,(repeat_pd_h<<16) | repeat_pd_l);
		ir_writel(R_IR_NEC_BIT1_ONTSET,(data1_on_h<<16) | data1_on_l);
		ir_writel(R_IR_NEC_BIT1_PRDSET,(data1_pd_h<<16) | data1_pd_l);
		ir_writel(R_IR_NEC_BIT0_ONTSET,(data0_on_h<<16) | data0_on_l);
		ir_writel(R_IR_NEC_BIT0_PRDSET,(data0_pd_h<<16) | data0_pd_l);
	}
	else
	{
		ir_writel(R_IR_NEC_START_ONTSET,(start_on_h_24m<<16) | start_on_l_24m);//	start on time
		ir_writel(R_IR_NEC_START_PRDSET, (start_pd_h_24m<<16) | start_pd_l_24m);//	start period
		ir_writel(R_IR_NEC_REPEAT_ONTSET, (repeat_on_h_24m<<16) | repeat_on_l_24m);//	repeat on time
		ir_writel(R_IR_NEC_REPEAT_PRDSET, (repeat_pd_h_24m<<16) | repeat_pd_l_24m);//	repeat period
		ir_writel(R_IR_NEC_BIT1_ONTSET, (data1_on_h_24m<<16) | data1_on_l_24m);//	data1 on time
		ir_writel(R_IR_NEC_BIT1_PRDSET, (data1_pd_h_24m<<16) | data1_pd_l_24m);//	data1 on time
		ir_writel(R_IR_NEC_BIT0_ONTSET, (data0_on_h_24m<<16) | data0_on_l_24m);//	data0 on time
		ir_writel(R_IR_NEC_BIT0_PRDSET, (data0_pd_h_24m<<16) | data0_pd_l_24m);//	data0 on time
	}
}

static void ir_hw_decoder_init(struct ir_config *settings)
{
    unsigned long reg32 = 0;
    unsigned long reg32_addr = 0;

#if 1//symphony
	reg32_addr = R_IR_NEC_FILT;
	reg32 = ir_readl(reg32_addr);
	reg32 &= ~(1 << 31); // usercode filter enable
	reg32 &= ~(1 << 30); // keycode filter enable
	ir_writel(reg32_addr, reg32);

	reg32_addr = R_IR_GLOBAL_CTRL;
	reg32 = ir_readl(reg32_addr);
	reg32 &= 0x0;
	reg32 |= 0x5 << 24;//period noise set as 0x14
	reg32 |= 0x7 << 20;//set idle overtime set as 7
	reg32 |= 0x4 << 16; // set repeat overtime set as 0xF
	reg32 &= ~(0xF << 12);//Wave filter channel disabled
	 if(settings->is_repeat)
	 {
	        reg32 |= ((settings->repkey_interval & 0xf) << REPEAT_CTRL_SHIFT);
	 }
	reg32 |= 0x1 << 8;//set repeat ctrl num
	reg32 &= ~(0x1 << 6);//set check enable
	reg32 &= ~(0x1 << 5);
	//dtmp |= (0x1 << 5);
	//dtmp &= ~(0x1 << 5);//set RPTMODE 0¼òÂë³¬Ê±¶ªÆú
	reg32 |= 0x2 << 3;
	reg32 |= 0x1<<2 ;//idle sta
	reg32 &= ~(0x1 << 1);//hw nec en
	reg32 |= 0x1 ;//sample en
	ir_writel(reg32_addr, reg32);
	//printk("reg(SYMPHONY_IR_GLOBAL_CTRL) = 0x%x\n",ir_readl(ptmp));
	reg32_addr = R_IR_INT_CFG;
	reg32 = ir_readl(reg32_addr);
	reg32 &= ~0xF; // IC decoding key down interrupt enabled;
	reg32 |= 0x1; // Wave filter interrupt disabled
	       // Soft decoding key interrupt disabled
	       // IC decoding key up interrupt disabled
   	 if(settings->is_keyup)
    	{
        		reg32 |= (1 << IE_NEC_UP_SHIFT);
    	}
	ir_writel(reg32_addr, reg32);
	nec_time_cfg();
#endif
}

static void ir_soft_decoder_init(struct ir_config *settings)
{
    //unsigned long reg32 = 0;
    unsigned long ptmp = 0;
    unsigned long dtmp = 0;
    //unsigned char sw_int_freq = 0;

   //MT_INFO_IR("[ir_symphony.c]: ir_soft_decoder_init enter\n");

	 /*
  *	PERIOD_NOISE	31:24		   0x14
  *	IDLE_OT			22:20		   0x7
  *   REPEAT_OT		19:16		   0xF
  *	WF_EN			15:12		   0x0  disable
  *   REPEAT_CTRL		11:8			   0x2
  *   CHK_EN			6			   1
  *   RPTMODE			5			   0   //not accept
  *   SAMPLE_FRQ		4:3			   2   //750K
  *   IDLE_STA			2			   1
  *   MODE			1			   0    //hw nec en
  *   SMP_EN			0			   1	  //
  */
  ptmp = R_IR_GLOBAL_CTRL;
  dtmp = ir_readl(ptmp);
  dtmp &= 0x0;
  #if DEFAULT_SAMPLE_FREQ
  dtmp &= (~(0x7 << 24));
  dtmp |= (0x5 << 24);//period noise set as 0x14
  //printk("###%s,irda_protocol=[%d]###\n",__FUNCTION__,g_ir_filter.irda_protocol);
  if ((g_ir_filter.irda_protocol == IRDA_SW_RC5))
  {
  	dtmp |= (0x19 << 24);//period noise set as 0x14
  	dtmp |= (0x5 << 20);//set idle overtime set as 7
  }
  else
  {
  	dtmp |= (0x7 << 20);//set idle overtime set as 7
  }
  dtmp |= (0xF << 16); // set repeat overtime set as 0xF
  #else
  dtmp |= (0x1 << 24);
  dtmp |= (0x1 << 20);
  dtmp |= (0x4 << 16); // set repeat overtime set as 0xF
  #endif
  //dtmp |= (0x3 << 20);//set idle overtime set as 7
  dtmp &= ~(0xF << 12);//Wave filter channel disabled
  dtmp &= ~(0x1 << 6);//set check enable
  dtmp &= ~(1 << 5); // Discard timeout repeated code (~108ms fixed);
  #if DEFAULT_SAMPLE_FREQ
  dtmp |= (0x2 << 3);//sample_clk 187.5
  #else
  dtmp |= (0x3 << 3);//sample_clk 46.875
  #endif
  dtmp |= (0x1 << 2);//idle sta
  dtmp |= (0x1 << 1);//soft protocol mode
  dtmp |= 0x1 ;//sample en

  ir_writel(ptmp, dtmp);
  //printk("reg(0xBF0A0008) = 0x%x\n",ir_readl(0xBF0A0008));

  /*
  --IC decoding key down interrupt disabled;
  --IC decoding key up interrupt disabled;
  --Soft decoding Key interrupt enabled;
  --Wave filter interrupt disabled;
  --Soft decoding interrupt every 1 valid sample
  */
  ptmp = R_IR_INT_CFG;
  dtmp = ir_readl(ptmp);
  dtmp &= (~0xF); // Wave filter interrupt disabled
               // hw nec decoding key up interrupt disabled
               // hw nec decoding key down interrupt disabled
   //if (p_ir->irda_protocol == IRDA_SW_RC5)
  {
  	dtmp |= 0x1 << 2; //soft protocol key down int enable
  }
  dtmp |= 0x1 << 3; //soft protocol overtime enable
  /* Repeat sample interrupt frequency
     1. For IRDA protocol with repeated code (i.e. NEC),
        leave INT_FRQ as zero, generate int for each valid sample
	 2. For IRDA protocol without repeated code,
	    configure INT_FRQ as 1 tipically, generate int every 2 valid samples
  */
  dtmp &= ~(0x3F << 8);

	if (0 == settings->int_interval)//to avoid negative number when minus 1.
	{
		settings->int_interval = 1;
	}
	dtmp |= (((settings->int_interval - 1) & 0x3f) << INT_FRQ_SHIFT);
	ir_writel(ptmp, dtmp);

#if 0
	MT_INFO_IR("reg(SYMPHONY_IR_INT_CFG) = 0x%x\n",readl((volatile unsigned long *)ptmp));
	MT_INFO_IR("reg(0xBF151000) = 0x%x\n",readl((volatile unsigned long *)0xBF151000));
	MT_INFO_IR("reg(0xBF151004) = 0x%x\n",readl((volatile unsigned long *)0xBF151004));
	MT_INFO_IR("reg(0xBF151008) = 0x%x\n",readl((volatile unsigned long *)0xBF151008));
	MT_INFO_IR("reg(0xBF15100c) = 0x%x\n",readl((volatile unsigned long *)0xBF15100c));
	MT_INFO_IR("reg(0xBF151010) = 0x%x\n",readl((volatile unsigned long *)0xBF151010));
	MT_INFO_IR("reg(0xBF151014) = 0x%x\n",readl((volatile unsigned long *)0xBF151014));
	MT_INFO_IR("reg(0xBF151018) = 0x%x\n",readl((volatile unsigned long *)0xBF151018));
#endif
}


static void irda_symphony_rc5_wfilt_set(u8 *bit_val, u8 wfilt_add_len ,u8 * p_channel_len)
{
	u8 j = 0, k = 0;

	while (j <= 13)
	{
		if (j == 13)
		{
			irda_symphony_wave_wfn_set(wfilt_add_len + k, rc5_sigtbl[0], rc5_sigtbl[1], 0, 4095);
			MT_INFO_IR("%d, bit_val[%d]: %d\n", __LINE__, j, bit_val[j]);
			j ++;
		}
		else if ((bit_val[j] && bit_val[j+1]) || (!bit_val[j] && !bit_val[j+1])) // 11b or 00b
		{
			if (j < 13)
			{
				irda_symphony_wave_wfn_set(wfilt_add_len + k, rc5_sigtbl[0], rc5_sigtbl[1], rc5_sigtbl[2], rc5_sigtbl[3]);
				MT_INFO_IR("%d, bit_val[%d]: %d\n", __LINE__, j, bit_val[j]);
				j ++;
			}
		}
		else if (bit_val[j] && !bit_val[j+1]) // 10b
		{
			if (j < 12)
			{
				if (bit_val[j+2]) // 101b
				{
					irda_symphony_wave_wfn_set(wfilt_add_len + k, rc5_sigtbl[2], rc5_sigtbl[3], rc5_sigtbl[6], rc5_sigtbl[7]);
				}
				else if (!bit_val[j+2]) // 100b
				{
					irda_symphony_wave_wfn_set(wfilt_add_len + k, rc5_sigtbl[2], rc5_sigtbl[3], rc5_sigtbl[4], rc5_sigtbl[5]);
				}

				MT_INFO_IR("%d, bit_val[%d]: %d\n", __LINE__, j, bit_val[j]);
				MT_INFO_IR("%d, bit_val[%d]: %d\n", __LINE__, j + 1, bit_val[j+1]);
				j += 2;
			}
			else if (j == 12) // 10b
			{
				irda_symphony_wave_wfn_set(wfilt_add_len + k, rc5_sigtbl[2], rc5_sigtbl[3], 0, 4095);

				MT_INFO_IR("%d, bit_val[%d]: %d\n", __LINE__, j, bit_val[j]);
				MT_INFO_IR("%d, bit_val[%d]: %d\n", __LINE__, j + 1, bit_val[j+1]);
				j += 2;
			}
		}
		else if (!bit_val[j] && bit_val[j+1]) // 01b
		{
			irda_symphony_wave_wfn_set(wfilt_add_len + k, rc5_sigtbl[0], rc5_sigtbl[1], rc5_sigtbl[4], rc5_sigtbl[5]);
			MT_INFO_IR("%d, bit_val[%d]: %d\n", __LINE__, j, bit_val[j]);
			j ++;
		}

		if(k == 1)
		{
			irda_symphony_wave_mask_bit_set(wfilt_add_len +k, 0);
		}
		else
		{
			irda_symphony_wave_mask_bit_set(wfilt_add_len +k, 1);
		}

		k++;
	}

	*p_channel_len = k;
}

/*!
  IRDA RCMM decoder
*/
static void irda_symphony_rcmm_decoder(irda_dec_para_t *p_para, irda_dec_result_t *p_result)
{
	u16 recv_num = 0;
	static u8 bit_v[32] = { 0 };
	static u8 bit_cnt = 0;
	static u8 bit_t = -1;
	static u32 start_ticks = 0;
	//u32 key = 0;
	u8 key_status = 0;

	recv_num = irda_symphony_get_recv_num();
	spin_lock(&(g_IrAttr.Plusedata_lock));
	printk(KERN_ERR "\n %s[%d]: recv_num=%d bit_cnt=%d p_result->state=%d\n",__FUNCTION__,__LINE__,recv_num,bit_cnt,p_result->state);
	//MT_INFO_IR("Fun[%s] Line[%u]  recv_num = %u\n", __FUNCTION__,__LINE__,recv_num);
	while(recv_num)
	{
		irda_symphony_get_ontime_and_period(&p_para->ontime, &p_para->period);
		if((g_IrPluseData.cur < IR_MAX_PLUSE_NUM) && (IR_PLUSE_VAILD != g_IrPluseData.vaild))
		{
			g_IrPluseData.ontime[g_IrPluseData.cur] = p_para->ontime;
			g_IrPluseData.period[g_IrPluseData.cur] = p_para->period;
			g_IrPluseData.cur++;
		}
		recv_num--;
		//  MT_INFO_IR("  (%d)#ontime:%d, period:%d bit_cnt= %d  \n", recv_num, p_para->ontime, p_para->period,bit_cnt);
		if(p_para->ontime >= p_result->p_sigtbl[0] && p_para->ontime <= p_result->p_sigtbl[1])
		{
			if(p_para->period >= p_result->p_sigtbl[2] && p_para->period <= p_result->p_sigtbl[3])
			{//00
				bit_v[bit_cnt++] = 0;
				bit_v[bit_cnt++] = 0;
			}
			else if(p_para->period >= p_result->p_sigtbl[4] && p_para->period <= p_result->p_sigtbl[5])
			{//01
				bit_v[bit_cnt++] = 0;
				bit_v[bit_cnt++] = 1;
			}
			else if(p_para->period >= p_result->p_sigtbl[6] && p_para->period <= p_result->p_sigtbl[7])
			{//10
				bit_v[bit_cnt++] = 1;
				bit_v[bit_cnt++] = 0;
			}
			else if(p_para->period >= p_result->p_sigtbl[8] && p_para->period <= p_result->p_sigtbl[9])
			{//11
				bit_v[bit_cnt++] = 1;
				bit_v[bit_cnt++] = 1;
			}
		}

		if (p_para->period >= 0xFFF)
		{
			MT_INFO_IR("\n # %s %d it_cnt=%d recv_num=%d p_para->period=%d\n",__FUNCTION__,__LINE__,bit_cnt,recv_num,p_para->period);
			bit_cnt = 0;
			memset(bit_v, 0x0,sizeof(bit_v));
		}

		if( bit_cnt == 32)
		{
			u8 i = 0;
			for(i = 0 ;i < 32 ;i ++)
			{
				//key |= (bit_v[i] << i);
			}

			// printk("key = 0x%x ###\n",key);
			p_result->r.rcmm.ucode = 0;

			p_result->r.rcmm.ucode |= (bit_v[4] & 0x1) << 7;
			p_result->r.rcmm.ucode |= (bit_v[5] & 0x1) << 6;
			p_result->r.rcmm.ucode |= (bit_v[6] & 0x1) << 5;
			p_result->r.rcmm.ucode |= (bit_v[7] & 0x1) << 4;
			p_result->r.rcmm.ucode |= (bit_v[8] & 0x1) << 3;
			p_result->r.rcmm.ucode |= (bit_v[9] & 0x1) << 2;
			p_result->r.rcmm.ucode |= (bit_v[10] & 0x1) << 1;
			p_result->r.rcmm.ucode |= (bit_v[11] & 0x1) ;

			p_result->r.rcmm.kcode = 0;
			p_result->r.rcmm.kcode |= (bit_v[24] & 0x1) << 7;
			p_result->r.rcmm.kcode |= (bit_v[25] & 0x1) << 6;
			p_result->r.rcmm.kcode |= (bit_v[26] & 0x1) << 5;
			p_result->r.rcmm.kcode |= (bit_v[27] & 0x1) << 4;
			p_result->r.rcmm.kcode |= (bit_v[28] & 0x1) << 3;
			p_result->r.rcmm.kcode |= (bit_v[29] & 0x1) << 2;
			p_result->r.rcmm.kcode |= (bit_v[30] & 0x1) << 1;
			p_result->r.rcmm.kcode |= bit_v[31] & 0x1;

			key_status = MT_UNF_KEY_STATUS_DOWN;
			if(bit_t == bit_v[16])
			{
				key_status = MT_UNF_KEY_STATUS_HOLD;
			}
			bit_t = bit_v[16];

			MT_INFO_IR("   kcode = 0x%x  \n", p_result->r.rcmm.kcode);
			bit_cnt = 0;
			memset(bit_v, 0x0,sizeof(bit_v));
			p_result->state = E_RCMM_STA_FINISH;

			// printk(KERN_ERR "%s[%d]: start+300 = %ld, jiffies = %ld\n",
			//	__FUNCTION__, __LINE__, start_ticks + msecs_to_jiffies(300), jiffies);

			if (!(time_after(start_ticks + msecs_to_jiffies(300), jiffies)))
			{
				printk(KERN_ERR "%s[%d]: wake up, kcode = 0x%02x, ucode = 0x%02x, kstatus = 0x%x\n",
					__FUNCTION__, __LINE__, p_result->r.rcmm.kcode, p_result->r.rcmm.ucode, key_status);
				IR_BUF_HEAD.IrKeyDataH = 0;
				IR_BUF_HEAD.IrKeyDataL =  (p_result->r.rcmm.kcode << 24) | (p_result->r.rcmm.ucode&0xffff);
				IR_BUF_HEAD.IrKeyState = key_status;
				IR_BUF_HEAD.IrProtocol = IRDA_SW_RCMM;
				g_IrAttr.Head = INC_BUF(g_IrAttr.Head, g_IrAttr.IrKeyBufLen);
				g_IrPluseData.vaild = IR_PLUSE_VAILD;
				wake_up_interruptible(&(g_IrAttr.IrKeyWaitQueue));
				// printk("\n ###  200 %s %d  kcode=0x%x ##\n",__FUNCTION__,__LINE__,p_result->r.rcmm.kcode);
				start_ticks = jiffies;
			}
			else
			{
				printk(KERN_ERR "%s[%d]: time_after error\n", __FUNCTION__, __LINE__);
			}
		}
	}
	spin_unlock(&(g_IrAttr.Plusedata_lock));
}

/*!
  IRDA RC5 decoder
*/
static void irda_symphony_rc5_decoder(irda_dec_para_t *p_para, irda_dec_result_t *p_result)
{
	u16 recv_num = 0;
	u16 irda_code = 0;
	static u8 bit_val[14] = { 0 };
	static u8 half_bit1 = 0;//1:still have waveform and the begin of remain level is high
							//0:(has two means)don't have waveform or the begin of remain level is low
	static s8 bit_T = -1;

	MT_INFO_IR("irda_symphony_rc5_decoder###\n");
	recv_num = irda_symphony_get_recv_num();
	spin_lock(&(g_IrAttr.Plusedata_lock));
	printk(KERN_ERR "%s[%d]: recv_num=%d\n", __FUNCTION__, __LINE__, recv_num);
	while(recv_num)
	{
		irda_symphony_get_ontime_and_period(&p_para->ontime, &p_para->period);
		//printk("(%d)#ontime:%d, period:%d\n", recv_num, p_para->ontime, p_para->period);
		//printk("[%s %d]p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
		if((g_IrPluseData.cur < IR_MAX_PLUSE_NUM) && (IR_PLUSE_VAILD != g_IrPluseData.vaild))
		{
			g_IrPluseData.ontime[g_IrPluseData.cur] = p_para->ontime;
			g_IrPluseData.period[g_IrPluseData.cur] = p_para->period;
			g_IrPluseData.cur++;
		}
		if ((p_para->ontime >= p_result->p_sigtbl[0] && p_para->ontime <= p_result->p_sigtbl[1]) &&
			(p_para->period >= p_result->p_sigtbl[2] && p_para->period <= p_result->p_sigtbl[3]))
		{
			if (p_result->state <= E_RC5_STA_S2)
			{
				if (p_result->state > E_RC5_STA_IDLE)
				{
					p_result->counter++;
				}

				if (p_result->counter <= 13)//to avoid count is out of array's range
				{
					bit_val[p_result->counter] = 1;//combine of the before high level, a falling edge represnt 1
													//opposite of protocol because of hardware
					half_bit1 = 1;//still have waveform and the begin of remain level is high

					p_result->state++;
				}
				else
				{
					//printk("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
					p_result->state = E_RC5_STA_INVALID;
				}
			}
			else
			{
				p_result->counter++;
				if (p_result->counter <= 13)//to avoid count is out of array's range
				{
					bit_val[p_result->counter] = half_bit1;
					if ((p_result->state == E_RC5_STA_T) ||
						((p_result->state == E_RC5_STA_USR) && (p_result->counter >= 8)))//the ninth bit is begin of E_RC5_STA_KEY
					{
						p_result->state++;
					}
					else if ((p_result->state == E_RC5_STA_KEY) && (p_result->counter == 13))
					{
						MT_INFO_IR("line %d set state INVALID\n", __LINE__);
						//printk("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
						p_result->state = E_RC5_STA_INVALID;
					}
				}
				else
				{
					//printk("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
					p_result->state = E_RC5_STA_INVALID;
				}
			}
		}
		else if ((p_para->ontime >= p_result->p_sigtbl[2] && p_para->ontime <= p_result->p_sigtbl[3]) &&
				(p_para->period >= p_result->p_sigtbl[4] && p_para->period <= p_result->p_sigtbl[5]))
		{
			if (p_result->state == E_RC5_STA_IDLE)
			{
				if (p_result->counter <= 13)//to avoid count is out of array's range
				{
					bit_val[p_result->counter] = 1;//combine of the before high level, a falling edge represnt 1
													//opposite of protocol because of hardware
					p_result->counter++;
					bit_val[p_result->counter] = 0;
					half_bit1 = 0;//still have waveform and the begin of remain level is high
					p_result->state+=3;
				}
				else
				{
					MT_INFO_IR("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
					p_result->state = E_RC5_STA_INVALID;
				}
			}
			else
			{
				if (!half_bit1)//if there isn't a remain high level, the waveform is uncorrect.
				{
					MT_INFO_IR("line %d set state INVALID\n", __LINE__);
				//MT_INFO_IR("[%s %d]error, half_bit1=%d\n", __FUNCTION__, __LINE__, half_bit1);
					p_result->state = E_RC5_STA_INVALID;
				}
				else
				{
					p_result->counter++;
					if (p_result->counter <= 12)//to avoid count is out of array's range, it will add twice, so the value is 12.
					{
						bit_val[p_result->counter] = 1;//combine of the before high level, a falling edge represnt 1
						half_bit1 = 0;//still have waveform and the begin of remain level is low
						MT_INFO_IR("line: %d, state: %d, bit_val[%d]: %d, half_bit1: %d\n", __LINE__,
									p_result->state, p_result->counter, bit_val[p_result->counter], half_bit1);

						if (((p_result->state >= E_RC5_STA_S1) && (p_result->state <= E_RC5_STA_T)) ||
							((p_result->state == E_RC5_STA_USR) && (p_result->counter >= 8)))
						{
							p_result->state++;
						}

						p_result->counter++;
						bit_val[p_result->counter] = 0;//a rising edge represent 0, the waveform was parsed finished, don't have any waveform.
						MT_INFO_IR("line: %d, state: %d, bit_val[%d]: %d, half_bit1: %d\n", __LINE__,
									p_result->state, p_result->counter, bit_val[p_result->counter], half_bit1);

						if (((p_result->state >= E_RC5_STA_S1) && (p_result->state <= E_RC5_STA_T)) ||
							((p_result->state == E_RC5_STA_USR) && (p_result->counter >= 8)))
						{
							p_result->state++;
						}
						else if ((p_result->state == E_RC5_STA_KEY) && (p_result->counter == 13))
						{
							MT_INFO_IR("line %d set state INVALID\n",__LINE__);
						//MT_INFO_IR("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
							p_result->state = E_RC5_STA_INVALID;
						}
					}
					else
					{
					//MT_INFO_IR("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
						p_result->state = E_RC5_STA_INVALID;
					}
				}
			}
		}
		else if ((p_para->ontime >= p_result->p_sigtbl[0] && p_para->ontime <= p_result->p_sigtbl[1]) &&
				(p_para->period >= p_result->p_sigtbl[4] && p_para->period <= p_result->p_sigtbl[5]))
		{
			if (half_bit1 && (p_result->state > E_RC5_STA_T))//If there is a remain waveform, it is uncorrect
			{
				MT_INFO_IR("line %d set state INVALID\n",__LINE__);
				//printk("[%s %d]error, half_bit1=%d\n", __FUNCTION__, __LINE__, half_bit1);
				p_result->state = E_RC5_STA_INVALID;
			}
			else
			{
				p_result->counter++;
				if (p_result->counter <= 13)//to avoid count is out of array's range
				{
					bit_val[p_result->counter] = 0;
					half_bit1= 1;//still have waveform and the begin of remain level is high
					MT_INFO_IR("line: %d, state: %d, bit_val[%d]: %d, half_bit1: %d\n", __LINE__,
								p_result->state, p_result->counter, bit_val[p_result->counter], half_bit1);

					if ((p_result->state == E_RC5_STA_T) ||
						((p_result->state == E_RC5_STA_USR) && (p_result->counter >= 8)))
					{
						p_result->state ++;
					}
					else if ((p_result->state == E_RC5_STA_KEY) && (p_result->counter == 13))
					{
						MT_INFO_IR("line %d set state INVALID\n",__LINE__);
						//printk("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
						p_result->state = E_RC5_STA_INVALID;
					}
				}
				else
				{
					//printk("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
					p_result->state = E_RC5_STA_INVALID;
				}
			}
		}
		else if ((p_para->ontime >= p_result->p_sigtbl[2] && p_para->ontime <= p_result->p_sigtbl[3]) &&
				(p_para->period >= p_result->p_sigtbl[6] && p_para->period <= p_result->p_sigtbl[7]))
		{
			if (p_result->state == E_RC5_STA_IDLE)
			{
				if (p_result->counter <= 13)//to avoid count is out of array's range
				{
					bit_val[p_result->counter] = 1;//combine of the before high level, a falling edge represnt 1
													//opposite of protocol because of hardware
					p_result->counter++;
					bit_val[p_result->counter] = 0;
					half_bit1 = 1;//still have waveform and the begin of remain level is high
					p_result->state+=3;
				}
				else
				{
					MT_INFO_IR("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
					p_result->state = E_RC5_STA_INVALID;
				}
			}
			else
			{
				if (!half_bit1)
				{
					MT_INFO_IR("line %d set state INVALID\n",__LINE__);
				//MT_INFO_IR("[%s %d]error, half_bit1=%d\n", __FUNCTION__, __LINE__, half_bit1);
					p_result->state = E_RC5_STA_INVALID;
				}
				else
				{
					p_result->counter++;
					if (p_result->counter <= 12)//to avoid count is out of array's range, it will add twice, so the value is 12.
					{
						bit_val[p_result->counter] = 1;
						half_bit1 = 0;//still have waveform and the begin of remain level is low
						MT_INFO_IR("line: %d, state: %d, bit_val[%d]: %d, half_bit1: %d\n", __LINE__,
									p_result->state, p_result->counter, bit_val[p_result->counter], half_bit1);

						if (((p_result->state >= E_RC5_STA_S1) && (p_result->state <= E_RC5_STA_T)) ||
							((p_result->state == E_RC5_STA_USR) && (p_result->counter >= 8)))
						{
							p_result->state++;
						}

						p_result->counter++;
						bit_val[p_result->counter] = 0;
						half_bit1= 1;//still have waveform and the begin of remain level is high
						MT_INFO_IR("line: %d, state: %d, bit_val[%d]: %d, half_bit1: %d\n", __LINE__,
									p_result->state, p_result->counter, bit_val[p_result->counter], half_bit1);
						if ((p_result->state == E_RC5_STA_S2) ||
							(p_result->state == E_RC5_STA_T) ||
							((p_result->state == E_RC5_STA_USR) && (p_result->counter >= 8)))
						{
							p_result->state++;
						}
						else if ((p_result->state == E_RC5_STA_KEY) && (p_result->counter == 13))
						{
							MT_INFO_IR("line %d set state INVALID\n",__LINE__);
						//MT_INFO_IR("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
							p_result->state = E_RC5_STA_INVALID;
						}
					}
					else
					{
						//MT_INFO_IR("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
						p_result->state = E_RC5_STA_INVALID;
					}
				}
			}
		}
		else if ((p_para->ontime >= p_result->p_sigtbl[0] && p_para->ontime <= p_result->p_sigtbl[1]) &&
				(p_para->period == 0xFFF))
		{
			p_result->counter++;
			if (p_result->counter <= 13)
			{
				bit_val[p_result->counter] = half_bit1;
				if ((p_result->state == E_RC5_STA_KEY) && (p_result->counter == 13))
				{
					MT_INFO_IR("line: %d, state: %d, bit_val[%d]: %d, half_bit1: %d\n", __LINE__,
								p_result->state, p_result->counter, bit_val[p_result->counter], half_bit1);
					p_result->state = E_RC5_STA_FINISH;
					g_IrPluseData.vaild = IR_PLUSE_VAILD;
				}
				else//to avoid some mistakes
				{
					//printk("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
					p_result->state = E_RC5_STA_INVALID;
				}
			}
			else
			{
				//printk("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
				p_result->state = E_RC5_STA_INVALID;
			}
		}
		else if ((p_para->ontime >= p_result->p_sigtbl[2] && p_para->ontime <= p_result->p_sigtbl[3]) &&
				(p_para->period == 0xFFF))
		{
			p_result->counter++;
			if (p_result->counter <= 12)
			{
				bit_val[p_result->counter] = half_bit1;
				half_bit1 = 0;//still have waveform and the begin of remain level is low

				MT_INFO_IR("line: %d, state: %d, bit_val[%d]: %d, half_bit1: %d\n", __LINE__,
							p_result->state, p_result->counter, bit_val[p_result->counter], half_bit1);

				p_result->counter++;
				bit_val[p_result->counter] = 0;
				if ((p_result->state == E_RC5_STA_KEY) && (p_result->counter == 13))
				{
					MT_INFO_IR("line: %d, state: %d, bit_val[%d]: %d, half_bit1: %d\n", __LINE__,
								p_result->state, p_result->counter, bit_val[p_result->counter], half_bit1);
					p_result->state = E_RC5_STA_FINISH;
					g_IrPluseData.vaild = IR_PLUSE_VAILD;
				}
				else//to avoid some mistakes
				{
					//printk("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
					p_result->state = E_RC5_STA_INVALID;
				}
			}
			else
			{
				//printk("[%s %d]error, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
				p_result->state = E_RC5_STA_INVALID;
			}
		}
		else
		{
			MT_INFO_IR("line %d set state INVALID\n",__LINE__);
			//printk("[%s %d]error, ontime:%d, period:%d, p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__,
			//		p_para->ontime, p_para->period, p_result->state, p_result->counter);

			p_result->state = E_RC5_STA_INVALID;
		}

		recv_num --;

		if (p_result->state == E_RC5_STA_FINISH)
		{
			p_result->r.rc5.ucode = 0;
			p_result->r.rc5.ucode |= (bit_val[3] & 0x1) << 4;
			p_result->r.rc5.ucode |= (bit_val[4] & 0x1) << 3;
			p_result->r.rc5.ucode |= (bit_val[5] & 0x1) << 2;
			p_result->r.rc5.ucode |= (bit_val[6] & 0x1) << 1;
			p_result->r.rc5.ucode |= bit_val[7] & 0x1;

			p_result->r.rc5.kcode = 0;
			p_result->r.rc5.kcode |= (bit_val[8] & 0x1) << 5;
			p_result->r.rc5.kcode |= (bit_val[9] & 0x1) << 4;
			p_result->r.rc5.kcode |= (bit_val[10] & 0x1) << 3;
			p_result->r.rc5.kcode |= (bit_val[11] & 0x1) << 2;
			p_result->r.rc5.kcode |= (bit_val[12] & 0x1) << 1;
			p_result->r.rc5.kcode |= bit_val[13] & 0x1;

			MT_INFO_IR("%s %d bit_val[1] = %d bit_val[2] = %d\n",__FUNCTION__,__LINE__,bit_val[1],bit_val[2]);
			if(bit_val[1] == 0)
			{
				p_result->r.rc5.kcode |= 0x40;
			}
			//  p_irda->user_code = p_result->r.rc5.ucode;
			irda_code = p_result->r.rc5.kcode;// | (UIO_IRDA << 8);
			IR_BUF_HEAD.IrKeyState = MT_UNF_KEY_STATUS_UP;
			if (bit_T == bit_val[2])
			{
				irda_code |= 1 << 15;
				//        MT_INFO_IR("###%s, MT_UNF_KEY_STATUS_HOLD##\n",__FUNCTION__);
				IR_BUF_HEAD.IrKeyState = MT_UNF_KEY_STATUS_HOLD;
			}
			else
			{
				bit_T = bit_val[2];
				//      MT_INFO_IR("###%s, MT_UNF_KEY_STATUS_DOWN##\n",__FUNCTION__);
				IR_BUF_HEAD.IrKeyState = MT_UNF_KEY_STATUS_DOWN;
			}

			MT_INFO_IR("[%s]   user code: %08x\n", __FUNCTION__,p_result->r.rc5.ucode);
			MT_INFO_IR("[%s]   kcode: %08x, irda_code=0x%x\n", __FUNCTION__,p_result->r.rc5.kcode, irda_code);
			printk(KERN_ERR "%s[%d]: wake up, uode: 0x%02x, kcode: 0x%02x, irda_code=0x%x\n",
				__FUNCTION__, __LINE__, p_result->r.rc5.ucode, p_result->r.rc5.kcode, irda_code);

			irda_code = 0;
			irda_code = p_result->r.rc5.ucode << 8;
			irda_code |= p_result->r.rc5.kcode;
			IR_BUF_HEAD.IrKeyDataH = 0;
			IR_BUF_HEAD.IrKeyDataL = irda_code;
			IR_BUF_HEAD.IrProtocol = IRDA_SW_RC5;
			g_IrAttr.Head = INC_BUF(g_IrAttr.Head, g_IrAttr.IrKeyBufLen);
			wake_up_interruptible(&(g_IrAttr.IrKeyWaitQueue));
			half_bit1 = 0;
			p_result->counter = 0;
			memset(bit_val, 0x0, 14);
			p_result->state = E_RC5_STA_IDLE;
		}
		else if (p_result->state == E_RC5_STA_INVALID)
		{
			MT_INFO_IR("STATE E_RC5_STA_INVALID\n");
			printk(KERN_ERR "%s[%d]: STATE E_RC5_STA_INVALID\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	if (p_result->state == E_RC5_STA_INVALID)
	{
		half_bit1 = 0;
		p_result->counter = 0;
		memset(bit_val, 0, 14);
		p_result->state = E_RC5_STA_IDLE;
		g_IrPluseData.cur = 0;
		g_IrPluseData.vaild = IR_PLUSE_INVAILD;

		while(recv_num)
		{
			irda_symphony_get_ontime_and_period(&p_para->ontime, &p_para->period);
			recv_num --;
		}

		//printk("[%s %d]new begin: p_result->state=%d, p_result->counter=%d\n", __FUNCTION__, __LINE__, p_result->state, p_result->counter);
	}
	spin_unlock(&(g_IrAttr.Plusedata_lock));
}

static u8 irda_symphony_nec_decoder_parse_sig(u16 ontime, u16 period, u16 * sigtbl)
{
  if((ontime >= sigtbl[0]) && (ontime <= sigtbl[1]) &&
      (period >= sigtbl[2]) && (period <= sigtbl[3]))
  {
    return E_NEC_SIG_START;
  }
  else if((ontime >= sigtbl[4]) && (ontime <= sigtbl[5]) &&
          (period >= sigtbl[6]) && (period <= sigtbl[7]))
  {
    return E_NEC_SIG_0;
  }
  else if((ontime >= sigtbl[8]) && (ontime <= sigtbl[9]) &&
          (period >= sigtbl[10]) && (period <= sigtbl[11]))
  {
    return E_NEC_SIG_1;
  }
  else if((ontime >= sigtbl[12]) && (ontime <= sigtbl[13]) &&
          (period >= sigtbl[14]) && (period <= sigtbl[15]))
  {
    return E_NEC_SIG_REPEAT;
  }
  else
  {
    return E_NEC_SIG_INVALID;
  }
}

inline static MT_BOOL irda_symphony_nec_decode_check_keycode(u16 code)
{
  u8 ch = (code >> 8) & 0xff;
  u8 cl = code & 0xff;
  if((ch + cl) == 0xff)
    return TRUE;
  else
    return FALSE;
}

inline static void irda_symphony_nec_reset_decoder(irda_dec_result_t *p_result)
{
  p_result->state = E_NEC_STA_IDLE;
  p_result->counter = 0;
  p_result->r.nec.code = 0;
  p_result->r.nec.usercode = 0;
}
static int nec_step = 0;
/*!
  IRDA NEC Decoder
*/
static void irda_symphony_nec_decoder(irda_dec_para_t *p_para, irda_dec_result_t *p_result)
{
  //MT_INFO_IR(KERN_EMERG "irda_symphony_nec_decoder #ontime:%d, period:%d\n", p_para->ontime, p_para->period);
  u8 sig = irda_symphony_nec_decoder_parse_sig(p_para->ontime, p_para->period, p_result->p_sigtbl);
  //IRDA_PRINT("\nnec irda_decoder sig=%s, state=%s\n", nec_sig_strs[sig],nec_sta_strs[p_result->state]);
 // printk(KERN_EMERG "[%d]: p_result->state is %d, sig is %d\n", __LINE__, p_result->state, sig);

  if(p_result->state == E_NEC_STA_IDLE)
  {
    if(sig == E_NEC_SIG_START)
    {
      p_result->counter = 0;
      p_result->state = E_NEC_STA_START;
//      MT_INFO_IR(KERN_EMERG "first start :line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step++;
    }
    else if(sig == E_NEC_SIG_REPEAT)
    {
	 p_result->state = E_NEC_STA_REPEAT;
//	  MT_INFO_IR(KERN_EMERG "idle to repeat:line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	 //MT_INFO_IR(KERN_EMERG "repeat--->%s %d\n",__FUNCTION__,__LINE__);
    }
    else
    {
      p_result->counter = 0;
    }
  }
  else if(p_result->state == E_NEC_STA_START)
  {
    if(sig == E_NEC_SIG_0 || sig == E_NEC_SIG_1)
    {
      p_result->state = E_NEC_STA_USRL;
      if(sig == E_NEC_SIG_0)
      {
        p_result->r.nec.usercode &= ~(1 << p_result->counter);
      }
      else
      {
        p_result->r.nec.usercode |= (1 << p_result->counter);
      }
//	MT_INFO_IR(KERN_EMERG "line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step++;
    }
    else
    {
      irda_symphony_nec_reset_decoder(p_result);
    }
  }
  else if(p_result->state == E_NEC_STA_USRL)
  {
    if(sig == E_NEC_SIG_0 || sig == E_NEC_SIG_1)
    {
      p_result->counter ++;
      if(p_result->counter < 8)
      {
        if(sig == E_NEC_SIG_0)
        {
          p_result->r.nec.usercode &= ~(1 << p_result->counter);
        }
        else
        {
          p_result->r.nec.usercode |= (1 << p_result->counter);
        }
//			MT_INFO_IR(KERN_EMERG "line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step++;
      }
      else
      {
        p_result->counter = 0;
        p_result->state = E_NEC_STA_USRH;
        if(sig == E_NEC_SIG_0)
        {
          p_result->r.nec.usercode &= ~(1 << (p_result->counter + 8));
        }
        else
        {
          p_result->r.nec.usercode |= (1 << (p_result->counter + 8));
        }
//			MT_INFO_IR(KERN_EMERG "line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step++;
      }
    }
    else
    {
      irda_symphony_nec_reset_decoder(p_result);
    }
  }
  else if(p_result->state == E_NEC_STA_USRH)
  {
    if(sig == E_NEC_SIG_0 || sig == E_NEC_SIG_1)
    {
      p_result->counter ++;
      if(p_result->counter < 8)
      {
        if(sig == E_NEC_SIG_0)
        {
          p_result->r.nec.usercode &= ~(1 << (p_result->counter + 8));
        }
        else
        {
          p_result->r.nec.usercode |= (1 << (p_result->counter + 8));
        }
//	MT_INFO_IR(KERN_EMERG "line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step++;
      }
      else
      {
        p_result->counter = 0;
        p_result->state = E_NEC_STA_KEY;
        if(sig == E_NEC_SIG_0)
        {
          p_result->r.nec.code &= ~(1 << p_result->counter);
        }
        else
        {
          p_result->r.nec.code |= (1 << p_result->counter);
        }
//		MT_INFO_IR(KERN_EMERG "line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step++;
      }
    }
    else
    {
      irda_symphony_nec_reset_decoder(p_result);
    }
  }
  else if(p_result->state == E_NEC_STA_KEY)
  {
    if(sig == E_NEC_SIG_0 || sig == E_NEC_SIG_1)
    {
      p_result->counter ++;
      if(p_result->counter < 8)
      {
        if(sig == E_NEC_SIG_0)
        {
          p_result->r.nec.code &= ~(1 << p_result->counter);
        }
        else
        {
          p_result->r.nec.code |= (1 << p_result->counter);
        }
	//	MT_INFO_IR(KERN_EMERG "line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step++;
      }
      else
      {
        p_result->counter = 0;
        p_result->state = E_NEC_STA_YEK;
        if(sig == E_NEC_SIG_0)
        {
          p_result->r.nec.code &= ~(1 << (p_result->counter + 8));
        }
        else
        {
          p_result->r.nec.code |= (1 << (p_result->counter + 8));
        }
	//	MT_INFO_IR(KERN_EMERG "line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step++;
      }
    }
    else
    {
      irda_symphony_nec_reset_decoder(p_result);
    }
  }
  else if(p_result->state == E_NEC_STA_YEK)
  {
    if(sig == E_NEC_SIG_0 || sig == E_NEC_SIG_1)
    {
      p_result->counter ++;
      if(p_result->counter < 8)
      {
        if(sig == E_NEC_SIG_0)
        {
          p_result->r.nec.code &= ~(1 << (p_result->counter + 8));
        }
        else
        {
          p_result->r.nec.code |= (1 << (p_result->counter + 8));
        }
        if(p_result->counter == 7)
        {
          p_result->state = E_NEC_STA_FINISH;
        }
//	MT_INFO_IR(KERN_EMERG "finish line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step++;
      }
    }
    else
    {
      irda_symphony_nec_reset_decoder(p_result);
    }
  }
  else if(p_result->state == E_NEC_STA_FINISH)
  {
    if(sig == E_NEC_SIG_START)
    {
      p_result->state = E_NEC_STA_START;
      p_result->counter = 0;
      p_result->r.nec.code = 0;
      p_result->r.nec.usercode = 0;
//	  MT_INFO_IR(KERN_EMERG "finish to start line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step=0;
    }
    else if(sig == E_NEC_SIG_REPEAT)
    {
      p_result->counter = 0;
//      MT_INFO_IR(KERN_EMERG "finish to repeat line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step=0;
      if(p_result->repeat_interval == 0)
      {
        p_result->state = E_NEC_STA_FINISH;
      }
      else
      {
        p_result->state = E_NEC_STA_REPEAT;
      }
    }
    else
    {
      irda_symphony_nec_reset_decoder(p_result);
    }
  }
  else if(p_result->state == E_NEC_STA_REPEAT)
  {
 //   MT_INFO_IR(KERN_EMERG "repeat--->%s %d\n",__FUNCTION__,__LINE__);
    if(sig == E_NEC_SIG_START)
    {
      p_result->state = E_NEC_STA_START;
      p_result->counter = 0;
      p_result->r.nec.code = 0;
      p_result->r.nec.usercode = 0;
	//  MT_INFO_IR(KERN_EMERG "repeat to restart line %d:p_para->ontime is %d, p_para->period is %d,nec_step is %d\n",__LINE__,p_para->ontime,p_para->period,nec_step);
	nec_step=0;
    }
    else if(sig == E_NEC_SIG_REPEAT)
    {
   //   	 MT_INFO_IR(KERN_EMERG "repeat--->%s %d\n",__FUNCTION__,__LINE__);
      p_result->counter ++;
      if(p_result->counter >= p_result->repeat_interval)
      {
   //   	 MT_INFO_IR(KERN_EMERG "repeat--->%s %d,repeat_interval is %d,counter is %d\n",__FUNCTION__,__LINE__,p_result->repeat_interval,p_result->counter);
        p_result->state = E_NEC_STA_FINISH;
        p_result->counter = 0;
      }
    }
    else
    {
      irda_symphony_nec_reset_decoder(p_result);
    }
  }

  if(p_result->state == E_NEC_STA_FINISH)
  {
    if(!irda_symphony_nec_decode_check_keycode(p_result->r.nec.code))
    {
      irda_symphony_nec_reset_decoder(p_result);
    }
  }
}/*IRDA NEC Decoder END*/
mt_s32 ir_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned long global_sta = 0;
	unsigned long int_rawsta = 0;
	unsigned int ir_nec_filt = 0;
	char repeat_sta = 0;
	char down_sta = 0;
	char up_sta = 0;
	u8 wfilter = 0;
	char i = 0;
	char time_val = 0;
	//char comdata_sta = 0;
	unsigned int data = 0;
	//unsigned long key = 0;
	//int receive_num = 0;
	s64 timeout = 0;
	static unsigned int nec_code = 0;

#if ((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE))
	struct mt_ir_priv *priv = (struct mt_ir_priv *)g_IrRegisterData.priv;
#endif

	g_ktcur = ktime_get_boottime();

	MT_INFO_IR("do_Ir_Isr!\n");
	printk(KERN_ERR "%s[%d]IR interrupt, proto = 0x%x\n", __FUNCTION__, __LINE__, g_ir_filter.irda_protocol);

	ir_nec_filt = ir_readl(R_IR_NEC_FILT);
	printk(KERN_ERR "%s[%d] ufilter = 0x%04x, kfilter = 0x%02x\n",
		__FUNCTION__, __LINE__, ((ir_nec_filt >> 8) & 0xffff), (ir_nec_filt & 0xff));

#if ((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE))
	printk(KERN_ERR "%s[%d]: rc support\n", __FUNCTION__, __LINE__);
#endif

	udelay(10);//the interval between press and filter interrupt is a few of clock
			   //if press a key which is set to filter very frequently,
			   //it will cause the interrupt can't reponsed.
			   //so delay some time and handle the two interrupts at same time
	global_sta = ir_readl(R_IR_GLOBAL_STA);
	int_rawsta = ir_readl(R_IR_INT_RAWSTA);
	printk(KERN_ERR "%s[%d]: int_rawsta = 0x%08lx\n", __FUNCTION__, __LINE__, int_rawsta);
	if(g_ir_filter.irda_protocol == IRDA_NEC)
	{
		MT_INFO_IR("IRDA_NEC!\n");
		repeat_sta = global_sta & 0x1;
		down_sta = int_rawsta & 0x1;
		up_sta = (int_rawsta >> 1) & 0x1;
		printk(KERN_ERR "repeat_sta = %u, down_sta = %u, up_sta = %u\n", repeat_sta, down_sta, up_sta);

		if(1 == repeat_sta)
		{
			if(g_repeat_flag)
			{
				g_repeat_flag = 0;
				for(i = 0;i<32;i++)
				{
					if((g_rekey >>i) & 0x01)
					{
						time_val+=2;
					}
					else
					{
						time_val+=1;
					}
				}
				time_val+=2;
				timeout = ktime_ms_delta(g_ktcur, g_ktpre);
				MT_INFO_IR("%s %d timeout = %lld\n",__FUNCTION__,__LINE__,timeout);
				if(timeout > (IR_INT_TIME - time_val))
				{
					g_deviation = timeout -(IR_INT_TIME - time_val);
				}
				else
				{
					g_deviation = 3;
				}

				g_ktpre = g_ktcur;

				printk(KERN_ERR "%s[%d]: repeat, returned\n", __FUNCTION__, __LINE__);
				return IRQ_HANDLED;
			}

			timeout = ktime_ms_delta(g_ktcur, g_ktpre);
			MT_INFO_IR("%s %d timeout = %lld\n",__FUNCTION__,__LINE__,timeout);
			if(timeout > IR_INT_TIME)
			{
				g_deviation += (timeout - IR_INT_TIME);
			}
			else if(timeout <  IR_INT_TIME-1)
			{
				if(g_deviation && (g_deviation >= ((IR_INT_TIME-1) - timeout)))
				{
					g_deviation -= ((IR_INT_TIME-1) - timeout);
				}
				else if(0 == g_deviation)
				{
					if(timeout <102)
					{
						repeat_sta = 0;
						g_repeat_flag = 1;
						MT_INFO_IR("%s %d timeout = %lld \n",__FUNCTION__,__LINE__,timeout);
					}
				}
				else
				{
					if((timeout + g_deviation) < 95)
					{
						repeat_sta = 0;
						g_repeat_flag = 1;
						MT_INFO_IR("%s %d timeout = %lld g_deviation = %lld\n",__FUNCTION__,__LINE__,timeout,g_deviation);
					}
					g_deviation = 0;
				}
			}
		}
		else
		{
			g_repeat_flag = 1;
			g_deviation = 0;
		}

		g_ktpre = g_ktcur;
		if(down_sta || repeat_sta || up_sta)
		{
			data = ir_readl(R_IR_NEC_DATA);
			g_rekey = data;
			//KEY = USERCODE | KEYCODE | REPEAT_STA
			//              16bits           8bits           8bits
			printk(KERN_ERR "%s[%d]: key = 0x%x, userdata = 0x%04x, keydata = 0x%02x\n",
				__FUNCTION__, __LINE__, data, (data & 0xFFFF), ((data >> 16) & 0xFF));

		#if ((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE))
			rc_keydown(priv->rc_dev, priv->rc_proto, data, 0);
		#endif

			if(repeat_sta)
			{
				IR_BUF_HEAD.IrKeyState = MT_UNF_KEY_STATUS_HOLD;
			}
			else if(down_sta)
			{
				IR_BUF_HEAD.IrKeyState = MT_UNF_KEY_STATUS_DOWN;
			}
			else if(up_sta)
			{
				IR_BUF_HEAD.IrKeyState = MT_UNF_KEY_STATUS_UP;
			}

			IR_BUF_HEAD.IrKeyDataH = 0;
			IR_BUF_HEAD.IrKeyDataL = data;
			IR_BUF_HEAD.IrProtocol = IRDA_NEC;
			g_IrAttr.Head = INC_BUF(g_IrAttr.Head, g_IrAttr.IrKeyBufLen);
			//wake_up_interruptible(&(g_IrAttr.IrKeyWaitQueue));
			wake_up(&(g_IrAttr.IrKeyWaitQueue));//this can wake up TASK_INTERRUPTIBLE and TASK_UNINTERRUPTIBLE
		}
	}
	else
	{
		static irda_dec_para_t dec_para = { 0 };
		static irda_dec_result_t dec_result = {{{0}}};

		printk(KERN_ERR "%s[%d]: DECODE_MODE_SW!\n", __FUNCTION__, __LINE__);
		if((global_sta >> OVERFLOW_SHIFT) & 0x1)
		{
			MT_INFO_IR(KERN_ERR "IR buffer overflow!\n");
			ir_softreset();
			#if defined(CONFIG_IR_LIRC_CODEC)
				reset_lirc_events(priv->rc_dev);
			#endif
			return IRQ_HANDLED;
		}
		down_sta = (int_rawsta>>2) & 0x1;
		up_sta = (int_rawsta >> 3) & 0x1;
		wfilter = (int_rawsta >> 4) & 0xf;

		printk(KERN_ERR "[%d]: down_sta = %u, up_sta = %u, wfilter = %u\n", __LINE__, down_sta, up_sta, wfilter);
		// MT_INFO_IR("irda_protocol###\n");

		if(!is_ir_intialized)
		{
			is_ir_intialized = MT_TRUE;
			if ((g_ir_filter.irda_protocol == IRDA_SW_RC5))
			{
				dec_result.state = E_RC5_STA_IDLE;
				dec_result.p_sigtbl = rc5_sigtbl;
				irclk_base_time_ns = 128000/27;//unit:ns
			}
			else if ((g_ir_filter.irda_protocol == IRDA_SW_RCMM))
			{
				printk(KERN_ERR "%s[%d]: rcmm decode!\n", __FUNCTION__, __LINE__);
				dec_result.state = E_RCMM_STA_IDLE;
				dec_result.p_sigtbl = rcmm_sigtbl;
			}
			else if(g_ir_filter.irda_protocol ==IRDA_SW_NEC )
			{
				dec_result.state = E_NEC_STA_IDLE;
				if(is_27M_clk())//boot reg bit30~bit31
				{
					dec_result.p_sigtbl = nec_sigtbl;
					irclk_base_time_ns = 128000/27;//unit:ns
				}
				else
				{
					dec_result.p_sigtbl = nec_sigtbl_24m;
					irclk_base_time_ns = 128000/24;
				}
			}
			//else if other sw protocol
		}

		if ((g_ir_filter.irda_protocol == IRDA_SW_RC5))
		{
			printk(KERN_ERR "%s[%d]: rc5 decode!\n", __FUNCTION__, __LINE__);
			irda_symphony_rc5_decoder(&dec_para, &dec_result);
			return IRQ_HANDLED;
		}
		else if ((g_ir_filter.irda_protocol == IRDA_SW_RCMM))
		{
			// printk(KERN_ERR "%s[%d]: rcmm decode!\n", __FUNCTION__, __LINE__);
			irda_symphony_rcmm_decoder(&dec_para, &dec_result);
			return IRQ_HANDLED;
		}
		else if ((g_ir_filter.irda_protocol == IRDA_SW_NEC))
		{
			#if defined(CONFIG_IR_LIRC_CODEC)
				DEFINE_IR_RAW_EVENT(ev);
			#endif
			u16 recv_num = 0;
			u16 ucode=0;
			u8 kcode=0;

			recv_num = irda_symphony_get_recv_num();
			printk(KERN_ERR "%s[%d]: sw_nec recv_num = %d\n", __FUNCTION__, __LINE__, recv_num);

			spin_lock(&(g_IrAttr.Plusedata_lock));
			while(recv_num)
    			{
      				irda_symphony_get_ontime_and_period(&dec_para.ontime, &dec_para.period);
				if((g_IrPluseData.cur < IR_MAX_PLUSE_NUM) && (IR_PLUSE_VAILD != g_IrPluseData.vaild))
				{
					g_IrPluseData.ontime[g_IrPluseData.cur] = dec_para.ontime;
					g_IrPluseData.period[g_IrPluseData.cur] = dec_para.period;
					g_IrPluseData.cur++;
				}
				#if defined(CONFIG_IR_LIRC_CODEC)
					if((dec_para.ontime >= dec_result.p_sigtbl[0]) && (dec_para.ontime <= dec_result.p_sigtbl[1]) &&
     						 (dec_para.period >= dec_result.p_sigtbl[2]) && (dec_para.period <= dec_result.p_sigtbl[3]))
					  {
    								ev.reset= true;
								ir_raw_event_store(priv->rc_dev,&ev);
								ir_raw_event_handle(priv->rc_dev);
								ev.reset= false;
 					   }
					store_lirc_events(priv->rc_dev,&ev,dec_para.ontime,dec_para.period);
				#endif

      				irda_symphony_nec_decoder(&dec_para, &dec_result);
      				if(dec_result.state == E_NEC_STA_FINISH)
      				{
        				kcode = dec_result.r.nec.code & 0xff;
       				ucode = dec_result.r.nec.usercode;
        				data = ((~kcode)<<24)|(kcode<<16)|ucode;
					nec_code = data;

					printk("%s[%d]: sw_nec dec, data = 0x%08x, kcode = 0x%02x, ucode = 0x%04x\n",
						__FUNCTION__, __LINE__, data, kcode, ucode);

					g_IrPluseData.vaild = IR_PLUSE_VAILD;
        				break;
      				}
				else if(dec_result.state == E_NEC_STA_REPEAT)
				{
					g_IrPluseData.vaild = IR_PLUSE_VAILD;
					dec_result.state = E_NEC_STA_FINISH;
					repeat_sta =1;
					break;
				}
      				recv_num --;
    			}

			while(recv_num)
           		{
      				irda_symphony_get_ontime_and_period(&dec_para.ontime, &dec_para.period);
      				recv_num --;
    			}
			spin_unlock(&(g_IrAttr.Plusedata_lock));
			if (dec_result.state != E_NEC_STA_FINISH)
      			{
       				 return IRQ_HANDLED;
      			}

			if(repeat_sta)
			{
				IR_BUF_HEAD.IrKeyState = MT_UNF_KEY_STATUS_HOLD;
			}
			else if(down_sta)
			{
				IR_BUF_HEAD.IrKeyState = MT_UNF_KEY_STATUS_DOWN;
			}
			else if(up_sta)
			{
				IR_BUF_HEAD.IrKeyState = MT_UNF_KEY_STATUS_UP;
			}

			IR_BUF_HEAD.IrKeyDataH = 0;
			IR_BUF_HEAD.IrKeyDataL = nec_code;
			IR_BUF_HEAD.IrProtocol = IRDA_SW_NEC;
			g_IrAttr.Head = INC_BUF(g_IrAttr.Head, g_IrAttr.IrKeyBufLen);
			#if ((defined IR_RC_SUPPORT) && (defined CONFIG_RC_CORE)&&(!defined(CONFIG_IR_LIRC_CODEC)))
				MT_INFO_IR(KERN_EMERG "SW_NEC rc_keydown\n");
				rc_keydown(priv->rc_dev, priv->rc_proto, nec_code, 0);
			#endif
			wake_up_interruptible(&(g_IrAttr.IrKeyWaitQueue));
			printk(KERN_ERR "[%d]: wake up, repeat_sta = %u, down_sta = %u, up_sta = %u, code = 0x%08x\n\n",
				__LINE__, repeat_sta, down_sta, up_sta, nec_code);

			return IRQ_HANDLED;
		}

#if 0
		if((global_sta >> OVERFLOW_SHIFT) & 0x1)
		{
		MT_INFO_IR(KERN_INFO "IR buffer overflow!\n");
		ir_softreset();
		return IRQ_HANDLED;
		}
		comdata_sta = (int_rawsta >> COMDATA_SHIFT) & 0x1;
		//if(comdata_sta)
		if(1)
		{
		receive_num = (global_sta >> RECEIVE_NUM_SHIFT) & 0xff;
		while(receive_num --)
		{
		ir_writel(1, (volatile unsigned long *)R_IR_COMBUF_POP);
		data = ir_readl(R_IR_COM_DATA);
		put_rxfifo(data);
		}
		keyin_status = 1;
		wake_up(&keyin_wq);
		}
#endif
	}

	return IRQ_HANDLED;
}


static void irda_symphony_wave_len_set(mt_u8 channel, mt_u8 len)
{
	ulong ptmp = 0;
	mt_u32 dtmp = 0;

	switch (channel)
	{
		case 0:
			ptmp = R_IR_WAVEFILT_CFG0;
			break;

		case 1:
			ptmp = R_IR_WAVEFILT_CFG1;
			break;

		case 2:
			ptmp = R_IR_WAVEFILT_CFG2;
			break;

		case 3:
			ptmp = R_IR_WAVEFILT_CFG3;
			break;

		default:
			break;
	}

	dtmp = ir_readl((volatile u32*)ptmp);
	dtmp &= 0xFFFF80FF;
	dtmp |= len << 8;
	ir_writel(ptmp, dtmp);
}

static void irda_symphony_wave_wfn_set(mt_u8 n, mt_u16 ontime_l, mt_u16 ontime_h, mt_u16 period_l, mt_u16 period_h)
{
	ulong ptmp = 0;
	mt_u32 dtmp = 0;

	//printk("%s, %d, n: %d, ontime_l: %d, ontime_h: %d, period_l: %d, period_h: %d\n", n, ontime_l, ontime_h, period_l, period_h);

	ptmp = GET_SYMPHONY_IR_WFN_ONTSET(n);
	dtmp = (ontime_h << 16) | ontime_l;
	ir_writel((volatile u32 *)ptmp, dtmp);
	// printk("GET_SYMPHONY_IR_WFN_ONTSET = 0x%x \n",ir_readl(ptmp));

	ptmp = GET_SYMPHONY_IR_WFN_PRDSET(n);
	dtmp = (period_h << 16) | period_l;
	ir_writel((volatile u32 *)ptmp,dtmp);
	//  printk("GET_SYMPHONY_IR_WFN_PRDSET = 0x%x \n",ir_readl(ptmp));
}

static void irda_symphony_wave_mask_bit_set(mt_u32 bit, mt_u32 value)
{
	mt_u8 position = 0;
	mt_u8 field = 0;
	mt_u32 dtmp = 0;
	ulong ptmp = 0;

	position = bit / 32;
	field = bit % 32;

	ptmp = GET_SYMPHONY_IR_WAVEFILT_MASK(position);

	dtmp = ir_readl((volatile u32 *)ptmp);
	dtmp &= ~(1 << field);
	dtmp |=(value << field);
	ir_writel((volatile u32 *)ptmp,dtmp);
}

static void irda_symphony_wave_add_set(mt_u8 channel, mt_u8 addr)
{
	ulong ptmp = 0;
	mt_u32 dtmp = 0;

	switch (channel)
	{
		case 0:
			ptmp = R_IR_WAVEFILT_CFG0;
			break;

		case 1:
			ptmp = R_IR_WAVEFILT_CFG1;
			break;

		case 2:
			ptmp = R_IR_WAVEFILT_CFG2;
			break;

		case 3:
			ptmp = R_IR_WAVEFILT_CFG3;
			break;

		default:
			break;
	}

	dtmp = ir_readl((volatile u32*)ptmp);

	dtmp &= 0xFFFFFF80;
	dtmp |= addr;

	ir_writel((volatile u32*)ptmp,dtmp);
}

static void irda_symphony_wave_channel_int_set(mt_u8 channel_int, mt_u8 is_en)
{
	ulong ptmp = 0;
	mt_u32 dtmp = 0;

	ptmp = R_IR_INT_CFG;
	dtmp = ir_readl((volatile u32*)ptmp);

	if (is_en)
	{
		dtmp |= channel_int << 4;
	}
	else
	{
		dtmp &= ~(channel_int << 4);
	}

	ir_writel((volatile u32*)ptmp, dtmp);
}

static void irda_symphony_wave_channel_set(mt_u8 channel, mt_u8 is_en)
{
	ulong ptmp = 0;
	mt_u32 dtmp = 0;

	ptmp = R_IR_GLOBAL_CTRL;
	dtmp = ir_readl((volatile u32*)ptmp);

	if (is_en)
	{
		dtmp |= channel << 12;
	}
	else
	{
		dtmp &= ~(channel << 12);
	}

	ir_writel((volatile u32*)ptmp, dtmp);
}

static mt_s32 irda_symphony_set_wfilt(ir_wavefilter_config_s ir_filter)
{
	mt_u8 i = 0;
	mt_u8 j = 0;
	mt_u8 channel_num = 0;
	mt_u8 wfilt_add_len = 0;
	mt_u8 channel_len = 0;
	mt_u8 irda_protocol = 0;
	mt_u32 code = 0;
	mt_u32 mask_code = 0;
	mt_u32 dtmp =0;
	ulong ptmp = 0;

	ptmp = R_IR_INT_CFG;
	dtmp = ir_readl((volatile u32*)ptmp);
	dtmp |= (0xf<<4);
	ir_writel((volatile u32*)ptmp, dtmp);

	irda_symphony_wave_channel_set(0xf, MT_FALSE);
	irda_symphony_wave_channel_int_set(0xf , MT_FALSE);
	channel_num = ir_filter.irda_wfilt_channel;
	g_ir_filter.irda_protocol = ir_filter.irda_protocol;
	// MT_INFO_IR("wflit channel_num = %d\n",channel_num);
	printk(KERN_ERR "%s[%d]: protocol = %d, channel_num = %d\n", __FUNCTION__, __LINE__, g_ir_filter.irda_protocol, channel_num);
	switch(channel_num)
	{
		case 4:
			irda_symphony_wave_channel_set(0xf, MT_TRUE);
			irda_symphony_wave_channel_int_set(0xf , MT_TRUE);
			break;

		case 3:
			irda_symphony_wave_channel_set(0x7, MT_TRUE);
			irda_symphony_wave_channel_int_set(0x7, MT_TRUE);
			break;

		case 2:
			irda_symphony_wave_channel_set(0x3, MT_TRUE);
			irda_symphony_wave_channel_int_set(0x3, MT_TRUE);
			break;

		case 1:
			irda_symphony_wave_channel_set(0x1, MT_TRUE);
			irda_symphony_wave_channel_int_set(0x1, MT_TRUE);
			break;

		default :
			return MT_ERR_PARAM;
	}

	for(i = 0; i < channel_num; i++)
	{
		irda_symphony_wave_add_set(i, wfilt_add_len);
		irda_protocol = ir_filter.irda_wfilt_channel_cfg[i].protocol;

		if ((irda_protocol == IRDA_NEC) || (irda_protocol == IRDA_SW_NEC))
		{
			/* IrDA NEC protocol */
			channel_len = ir_filter.irda_wfilt_channel_cfg[i].addr_len;
			mask_code = ir_filter.irda_wfilt_channel_cfg[i].wfilt_mask;
			code = ir_filter.irda_wfilt_channel_cfg[i].wfilt_code;

			MT_INFO_IR(KERN_INFO"ir_filter.irda_wfilt_channel_cfg[i].addr_len = 0x%x\n", ir_filter.irda_wfilt_channel_cfg[i].addr_len);
			MT_INFO_IR(KERN_INFO"ir_filter.irda_wfilt_channel_cfg[i].wfilt_mask = 0x%x\n", ir_filter.irda_wfilt_channel_cfg[i].wfilt_mask);
			MT_INFO_IR(KERN_INFO"ir_filter.irda_wfilt_channel_cfg[i].wfilt_code = 0x%x\n", ir_filter.irda_wfilt_channel_cfg[i].wfilt_code);

			// 16 bit usercode | 8 bit keycode | 8 bit reversed keycode
			// --->
			// 8 bit reversed keycode | 8 bit keycode | 16 bit usercode
			code = ((code & 0xFFFF0000) >> 16) |
					((code & 0xFF00) << 8) |
					((code & 0xFF) << 24);

			for(j = 0; j < channel_len; j++)
			{
				if(mask_code == 0)
				{
					irda_symphony_wave_mask_bit_set(j + wfilt_add_len, 1);//all need to compare
				}
				else//set mask bit, 1:compare, 0:not compare
				{
					if((mask_code >> j) & 0x1)
					{
						irda_symphony_wave_mask_bit_set(j + wfilt_add_len, 1);
					}
					else
					{
						irda_symphony_wave_mask_bit_set(j + wfilt_add_len, 0);
					}
				}

				if((code >> j) & 0x1) // Timing of NEC bit 1 code
				{
					if(is_27M_clk())
					{
						irda_symphony_wave_wfn_set(j + wfilt_add_len ,
						nec_sigtbl[8], nec_sigtbl[9], nec_sigtbl[10], nec_sigtbl[11]);
					}
					else
					{
						irda_symphony_wave_wfn_set(j + wfilt_add_len ,
						nec_sigtbl_24m[8], nec_sigtbl_24m[9], nec_sigtbl_24m[10], nec_sigtbl_24m[11]);
					}
				}
				else // Timing of NEC bit 0 code
				{
					if(is_27M_clk())
					{
						irda_symphony_wave_wfn_set(j + wfilt_add_len ,
						nec_sigtbl[4], nec_sigtbl[5], nec_sigtbl[6], nec_sigtbl[7]);
					}
					else
					{
						irda_symphony_wave_wfn_set(j + wfilt_add_len ,
						nec_sigtbl_24m[4], nec_sigtbl_24m[5], nec_sigtbl_24m[6], nec_sigtbl_24m[7]);
					}
				}
			}
			wfilt_add_len += channel_len;
		}
		else if (irda_protocol == IRDA_SW_RC5)
		{
			/* IrDA RC5 protocol */
			MT_U8 ucode = 0;
			MT_U8 bit_val[14] = { 0 };

			code = ir_filter.irda_wfilt_channel_cfg[i].wfilt_code;
			ucode = (code >> 8) & 0x1F;
			code &= 0x3F;
			code |= (ucode << 6) | (0x3 << 12);
			for (j = 13; j > 0; j --)
			{
				bit_val[13 - j] = (code >> j) & 0x1;
				//MT_INFO_IR("bit%d: %x\n",(13 - j),bit_val[13 - j]);
			}
			bit_val[13] = code & 0x1;
			//MT_INFO_IR("bit13: %x\n",bit_val[13]);

#if !RC5_WF_IGNORE_T_BIT
			bit_val[2] = i & 0x1; // the 3rd bit T
#endif

			irda_symphony_rc5_wfilt_set(bit_val,  wfilt_add_len, &channel_len);

			wfilt_add_len += channel_len;
		}
		else if (irda_protocol == IRDA_SW_RCMM)
		    {
			u8 bit_v[14] = {0};
			u8 k = 0;

			channel_len = ir_filter.irda_wfilt_channel_cfg[i].addr_len;
			mask_code = ir_filter.irda_wfilt_channel_cfg[i].wfilt_mask;
			code = ir_filter.irda_wfilt_channel_cfg[i].wfilt_code;

			// OS_PRINTF("%s[%d]: i = %d, code = 0x%04x, channel_len = %d\n", __FUNCTION__, __LINE__, i, code, channel_len);

			/*
			 * Toggle[15] + System[14:8] + CommandCode[7:0]
			 *
			 * Toggle represents key press repeat status, so skip it
			 *
			 * System[14:8] = 0x26
			 */
			channel_len = 8 + 6; // 14 bits, CommandCode[7:0] + System[13:8]
			// code &= 0x3fff;
			code = (code & 0xff) | (0x26 << 8);

			// convert the bit order
			for (j = 0; j < channel_len; j ++)
			{
				bit_v[channel_len -1 - j] = (code >> j) & 0x1;
			}

			for (j = 0; j < channel_len; j += 2)
			{
				if (mask_code == 0)
				{
					irda_symphony_wave_mask_bit_set(k + wfilt_add_len, 1);
				}
				else
				{
					if ((mask_code >> j) & 0x1)
					{
						irda_symphony_wave_mask_bit_set(k + wfilt_add_len, 1);
					}
					else
					{
						irda_symphony_wave_mask_bit_set(k + wfilt_add_len, 0);
					}
				}

				printk(KERN_ERR "%s[%d]: i = %d, j = %d, k = %d, channel_len = %d, wfilt_add_len = %d\n",
					__FUNCTION__, __LINE__, i, j, k, channel_len, wfilt_add_len);
				if (bit_v[j] == 0 && bit_v[j+1] == 0)
				{
					irda_symphony_wave_wfn_set(k + wfilt_add_len,
						rcmm_sigtbl[0], rcmm_sigtbl[1], rcmm_sigtbl[2], rcmm_sigtbl[3]);
				}
				else  if(bit_v[j] == 0 && bit_v[j+1] == 1)
				{
					irda_symphony_wave_wfn_set(k + wfilt_add_len,
						rcmm_sigtbl[0], rcmm_sigtbl[1], rcmm_sigtbl[4], rcmm_sigtbl[5]);
				}
				else  if(bit_v[j] == 1 && bit_v[j+1] == 0)
				{
					irda_symphony_wave_wfn_set(k + wfilt_add_len,
						rcmm_sigtbl[0], rcmm_sigtbl[1], rcmm_sigtbl[6], rcmm_sigtbl[7]);
				}
				else  if(bit_v[j] == 1 && bit_v[j+1] == 1)
				{
					irda_symphony_wave_wfn_set(k + wfilt_add_len,
						rcmm_sigtbl[0], rcmm_sigtbl[1], rcmm_sigtbl[8], rcmm_sigtbl[9]);
				}
				k++;
			}

			wfilt_add_len += (channel_len) / 2;
			channel_len = (channel_len) / 2;
		    }

		irda_symphony_wave_len_set(i, channel_len - 1);

		//MT_INFO_IR("channel len: %d, wflit_add_len: %d\n", channel_len, wfilt_add_len);
		//MT_INFO_IR("wfilt code: 0x%x\n", code);
	}

	return MT_SUCCESS;
}

static u16 nec_sigtbl_20M[16] = {1139, 1709, 1710, 2563, 71, 107, 142, 214, 71, 107, 285, 427, 1424, 2136}; //20.25 /128*1000 = 158.2Khz

void nec_time_recfg_osc(void)
{
    /*
    *	20.25M (+%5 : 21.26M,  -%5 :19.24M)param
    */

    unsigned int percent = 20;

    unsigned int start_on_h = 375 * (100+percent)/100;    // 9/4*21.26/128*1000 = 375
    unsigned int start_on_l = 337 * (100-percent)/100;     //9/4*19.24/128*1000 = 337
    unsigned int start_pd_h = 561* (100+percent)/100;
    unsigned int start_pd_l = 506 * (100-percent)/100;
    unsigned int repeat_on_h = 375 * (100+percent)/100;
    unsigned int repeat_on_l = 337 * (100-percent)/100;
    unsigned int repeat_pd_h = 467 * (100+percent)/100;
    unsigned int repeat_pd_l = 421 * (100-percent)/100;
    unsigned int data1_on_h = 24 * (100+percent)/100;
    unsigned int data1_on_l = 21 * (100-percent)/100;
    unsigned int data1_pd_h = 94 * (100+percent)/100;
    unsigned int data1_pd_l = 84 * (100-percent)/100;
    unsigned int data0_on_h = 24 * (100+percent)/100;
    unsigned int data0_on_l = 21 * (100-percent)/100;
    unsigned int data0_pd_h = 47 * (100+percent)/100;
    unsigned int data0_pd_l = 42 * (100-percent)/100;

    *((volatile unsigned int *)( R_IR_BASE_ADDR + 0x40 ))= (start_on_h<<16) | start_on_l;//	start on time
    *((volatile unsigned int *)( R_IR_BASE_ADDR + 0x44 ))= (start_pd_h<<16) | start_pd_l;//	start period
    *((volatile unsigned int *)( R_IR_BASE_ADDR + 0x48 ))= (repeat_on_h<<16) | repeat_on_l;//	repeat on time
    *((volatile unsigned int *)( R_IR_BASE_ADDR + 0x4c ))= (repeat_pd_h<<16) | repeat_pd_l;//	repeat period
    *((volatile unsigned int *)( R_IR_BASE_ADDR + 0x50 ))= (data1_on_h<<16) | data1_on_l;//	data1 on time
    *((volatile unsigned int *)( R_IR_BASE_ADDR + 0x54 ))= (data1_pd_h<<16) | data1_pd_l;//	data1 on time
    *((volatile unsigned int *)( R_IR_BASE_ADDR + 0x58 ))= (data0_on_h<<16) | data0_on_l;//	data0 on time
    *((volatile unsigned int *)( R_IR_BASE_ADDR + 0x5c ))= (data0_pd_h<<16) | data0_pd_l;//	data0 on time
}

mt_s32 irda_symphony_reset_wfilt_osc(void)
{
	u8 i = 0;
	u8 j = 0;
	u8 channel_num = 0;
	u8 wfilt_add_len = 0;
	u8 channel_len = 0;
	u8 irda_protocol = 0;
	u32 code = 0;
	u32 mask_code = 0;
	u32 dtmp =0;
	ulong ptmp = 0;

	printk("enter reset wflit\n");

	ptmp = R_IR_INT_CFG;
	dtmp = ir_readl((volatile u32*)ptmp);
	dtmp |= (0xf<<4);
	ir_writel((volatile u32*)ptmp, dtmp);

	irda_symphony_wave_channel_set(0xf, MT_FALSE);
	irda_symphony_wave_channel_int_set(0xf , MT_FALSE);
	channel_num = g_ir_filter.irda_wfilt_channel;

	printk(KERN_ERR "%s[%d]: irda_protocol = %d, wflit channel_num = %d\n", __FUNCTION__, __LINE__, irda_protocol, channel_num);
	switch(channel_num)
	{
		case 4:
			irda_symphony_wave_channel_set(0xf, MT_TRUE);
			irda_symphony_wave_channel_int_set(0xf , MT_TRUE);
			break;

		case 3:
			irda_symphony_wave_channel_set(0x7, MT_TRUE);
			irda_symphony_wave_channel_int_set(0x7, MT_TRUE);
			break;

		case 2:
			irda_symphony_wave_channel_set(0x3, MT_TRUE);
			irda_symphony_wave_channel_int_set(0x3, MT_TRUE);
			break;

		case 1:
			irda_symphony_wave_channel_set(0x1, MT_TRUE);
			irda_symphony_wave_channel_int_set(0x1, MT_TRUE);
			break;

		default :
			return MT_ERR_PARAM;
	}

	for(i = 0; i < channel_num; i++)
	{
		irda_symphony_wave_add_set(i, wfilt_add_len);
		irda_protocol = g_ir_filter.irda_wfilt_channel_cfg[i].protocol;

		if ((irda_protocol == IRDA_NEC) || (irda_protocol == IRDA_SW_NEC))
		{
			/* IrDA NEC protocol */
			channel_len = g_ir_filter.irda_wfilt_channel_cfg[i].addr_len;
			mask_code = g_ir_filter.irda_wfilt_channel_cfg[i].wfilt_mask;
			code = g_ir_filter.irda_wfilt_channel_cfg[i].wfilt_code;

			// 16 bit usercode | 8 bit keycode | 8 bit reversed keycode
			// --->
			// 8 bit reversed keycode | 8 bit keycode | 16 bit usercode
			code = ((code & 0xFFFF0000) >> 16) |
					((code & 0xFF00) << 8) |
					((code & 0xFF) << 24);

			for(j = 0; j < channel_len; j++)
			{
				if(mask_code == 0)
				{
					irda_symphony_wave_mask_bit_set(j + wfilt_add_len, 1);
				}
				else
				{
					if((mask_code >> j) & 0x1)
					{
						irda_symphony_wave_mask_bit_set(j + wfilt_add_len, 1);
					}
					else
					{
						irda_symphony_wave_mask_bit_set(j + wfilt_add_len, 0);
					}
				}

				if((code >> j) & 0x1) // Timing of NEC bit 1 code
				{
					irda_symphony_wave_wfn_set(j + wfilt_add_len ,
					nec_sigtbl_20M[8], nec_sigtbl_20M[9], nec_sigtbl_20M[10], nec_sigtbl_20M[11]);
				}
				else // Timing of NEC bit 0 code
				{
					irda_symphony_wave_wfn_set(j + wfilt_add_len ,
					nec_sigtbl_20M[4], nec_sigtbl_20M[5], nec_sigtbl_20M[6], nec_sigtbl_20M[7]);
				}
			}
			wfilt_add_len += channel_len;
		}
		else if(irda_protocol == IRDA_SW_RCMM)
		{
			/* IrDA  protocol */
			//u8 bit_v[11] = {1,0,1,0,1,1,1,1,0};
			u8 bit_v[14] = {0};
			u32 code = 0;
			u8 i = 0;
			channel_len = g_ir_filter.irda_wfilt_channel_cfg[i].addr_len;
			mask_code =g_ir_filter.irda_wfilt_channel_cfg[i].wfilt_mask;
			code =g_ir_filter.irda_wfilt_channel_cfg[i].wfilt_code;
			code = code & 0x3fff;

			for (j = 0; j < 14; j ++)
			{
				bit_v[13 - j] = (code >> j) & 0x1;
				MT_INFO_IR("bit[%d] = %d \n",(13 - j),bit_v[13 - j]);
			}

			for(j = 0;j < channel_len;j+= 2)
			{
				if(mask_code == 0)
				{
					irda_symphony_wave_mask_bit_set(i + wfilt_add_len, 1);
				}
				else
				{
					if((mask_code >> j) & 0x1)
					{
						irda_symphony_wave_mask_bit_set(i + wfilt_add_len, 1);
					}
					else
					{
						irda_symphony_wave_mask_bit_set(i + wfilt_add_len, 0);
					}
				}

				if(bit_v[j] == 0 && bit_v[j+1] == 0)
				{
					irda_symphony_wave_wfn_set(i + wfilt_add_len ,
					rcmm_sigtbl_20M[0], rcmm_sigtbl_20M[1], rcmm_sigtbl_20M[2], rcmm_sigtbl_20M[3]);
				}
				else  if(bit_v[j] == 0 && bit_v[j+1] == 1)
				{
					irda_symphony_wave_wfn_set(i + wfilt_add_len ,
					rcmm_sigtbl_20M[0], rcmm_sigtbl_20M[1], rcmm_sigtbl_20M[4], rcmm_sigtbl_20M[5]);
				}
				else  if(bit_v[j] == 1 && bit_v[j+1] == 0)
				{
					irda_symphony_wave_wfn_set(i + wfilt_add_len ,
					rcmm_sigtbl_20M[0], rcmm_sigtbl_20M[1], rcmm_sigtbl_20M[6], rcmm_sigtbl_20M[7]);
				}
				else  if(bit_v[j] == 1 && bit_v[j+1] == 1)
				{
					irda_symphony_wave_wfn_set(i + wfilt_add_len ,
					rcmm_sigtbl_20M[0], rcmm_sigtbl_20M[1], rcmm_sigtbl_20M[8], rcmm_sigtbl_20M[9]);
				}
				i++;
			}
			channel_len = (channel_len - 2)/2;
			wfilt_add_len +=channel_len;
		}

    irda_symphony_wave_len_set(i, channel_len - 1);

    printk("channel len: %d, wflit_add_len: %d\n", channel_len, wfilt_add_len);
    printk("wfilt code: 0x%x\n", code);
  }

  return MT_SUCCESS;
}
static mt_s32 irda_symphony_set_hw_usercode(MT_U8 en, MT_U16 usercode)
{
   unsigned long ptmp = 0;
   unsigned long dtmp = 0;

  ptmp = R_IR_NEC_FILT;
  dtmp = ir_readl((volatile u32*)ptmp);
  if (en)
  {
    //enable usercode filtering
    //only specific usercode acceptable
    dtmp |= 1 << 31;
    dtmp &= ~(0xFFFF << 8);
    dtmp |= (usercode & 0xFFFF)<<8;
    ir_writel((volatile u32*)ptmp, dtmp);

  }
  else
  {
    //disable usercode filtering
    //all usercode acceptable
    dtmp &= ~ (1 << 31);
    dtmp &= ~(0xFFFF << 8);
    ir_writel((volatile u32*)ptmp, dtmp);
  }

  return MT_SUCCESS;
}

static mt_s32 irda_symphony_set_hw_keycode(MT_U8  en, MT_U16 keycode)
{
   unsigned long ptmp = 0;
   unsigned long dtmp = 0;

  ptmp = R_IR_NEC_FILT;
  dtmp = ir_readl((volatile u32*)ptmp);
  if (en)
  {
    //enable keycode filtering
    //only specific keycode acceptable
    dtmp |= 1 << 30;
    ir_writel((volatile u32 *)ptmp, dtmp);

    dtmp &= ~0xFF;
    dtmp |= keycode & 0xFF;
    ir_writel((volatile u32 *)ptmp, dtmp);
  }
  else
  {
    //disable keycode filtering
    //all keycode acceptable
    dtmp &= ~(1 << 30);
    dtmp &= ~0xFF;
    ir_writel((volatile u32 *)ptmp, dtmp);
  }

  return MT_SUCCESS;
}

static long Ir_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	mt_s32 Ret = 0;
	//struct ir_wavefilter_config ir_filter = {0};
	mt_u8 i = 0;
	mt_u32 param = 0;
	unsigned long flags = 0;

	Ret = down_interruptible(&g_IrMutex);
	if (Ret)
	{
		MT_FATAL_IR("Semaphore lock is  error. \n");
		return MT_FAILURE;
	}

	switch (cmd)
	{
		case CMD_IR_SET_CONFIG:
			MT_INFO_IR(KERN_ERR"CMD_IR_SET_CONFIG\n");
			if (copy_from_user((void *)&ir_settings, (void *)arg, sizeof(struct ir_config)) != 0)
			{
				up(&g_IrMutex);
				return -EIO;
			}
			g_ir_filter.irda_protocol = ir_settings.protocol;

		#if ((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE))
			mt_ir_set_rc_protocol(ir_settings.protocol);
		#endif

			ir_softreset();

			if(g_ir_filter.irda_protocol == IRDA_NEC)
			{
				ir_hw_decoder_init(&ir_settings);
			}
			else
			{
				ir_soft_decoder_init(&ir_settings);
			}
			is_ir_intialized = false;
			ir_readl(R_IR_BASE_ADDR+0x14);//fix 125959, but don't find rootcause at present.
			break;

		case CMD_IR_SET_HW_USERCODE:
			if (copy_from_user(&param,(void *)arg,sizeof(param)) != 0)
			{
				up(&g_IrMutex);
				return -EIO;
			}
			ir_settings.ucfilter_en = param >> 16;
			ir_settings.usercode = param&0xffff;
			printk(KERN_ERR "ir_settings.ucfilter_en =%d  usercode=%x \n",ir_settings.ucfilter_en,ir_settings.usercode);
			irda_symphony_set_hw_usercode(ir_settings.ucfilter_en, ir_settings.usercode);
			break;

		case CMD_IR_SET_HW_KEYCODE:
			if (copy_from_user(&param,(void *)arg,sizeof(param)) != 0)
			{
				up(&g_IrMutex);
				return -EIO;
			}
			ir_settings.kcfilter_en = param >> 16;
			ir_settings.keycode = param&0xffff;
			printk(KERN_ERR "ir_settings.kcfilter_en =%d  keycode=%x \n",ir_settings.kcfilter_en,ir_settings.keycode);
			irda_symphony_set_hw_keycode(ir_settings.kcfilter_en, ir_settings.keycode);
			break;

		case CMD_IR_SET_BLOCKTIME:
			g_IrAttr.IrBlockTime = arg;
			break;

		case CMD_IR_SET_WAVE_FILTER:
			MT_INFO_IR(KERN_INFO"CMD_IR_SET_WAVE_FILTER\n");
			if (copy_from_user((void *)&g_ir_filter, (void *)arg, sizeof(struct ir_wavefilter_config)) != 0)
			{
				up(&g_IrMutex);
				return -EIO;
			}

			//MT_INFO_IR(KERN_INFO"reg[0xBF140020] = 0x%x\n", *(volatile unsigned int *)0xBF140020);
			//MT_INFO_IR(KERN_INFO"ir_filter.irda_wfilt_channel = %u\n", ir_filter.irda_wfilt_channel);
			//MT_INFO_IR(KERN_INFO"ir_filter.irda_wfilt_channel_cfg[0].protocol = %u\n", ir_filter.irda_wfilt_channel_cfg[0].protocol);
			//MT_INFO_IR(KERN_INFO"ir_filter.irda_wfilt_channel_cfg[0].addr_len = 0x%x\n", ir_filter.irda_wfilt_channel_cfg[0].addr_len);
			//MT_INFO_IR(KERN_INFO"ir_filter.irda_wfilt_channel_cfg[0].wfilt_code = 0x%x\n", ir_filter.irda_wfilt_channel_cfg[0].wfilt_code);

			irda_symphony_set_wfilt(g_ir_filter);

			ir_readl(R_IR_BASE_ADDR+0x14);//fix 125959, but don't find rootcause at present.
			break;

		case CMD_IR_CLEAR_KEY:
			// printk("\n ###  %s %d Head=%d Tail=%d  ###\n",__FUNCTION__,__LINE__,g_IrAttr.Head, g_IrAttr.Tail);
			spin_lock_irqsave(&(g_IrAttr.Plusedata_lock), flags);
			for(i = 0; i < g_IrAttr.Head ;i++)
			{
				memset(g_IrAttr.IrKeyBuf ,0x0,sizeof(g_IrAttr.IrKeyBuf));
			}
			g_IrAttr.Head = 0;
			g_IrAttr.Tail = 0;
			g_IrPluseData.cur = 0;
			g_IrPluseData.vaild = IR_PLUSE_INVAILD;
			spin_unlock_irqrestore(&(g_IrAttr.Plusedata_lock), flags);
			break;
		case CMD_IR_GET_PLUSE_DATA:
			spin_lock_irqsave(&(g_IrAttr.Plusedata_lock), flags);
			if(IR_PLUSE_VAILD != g_IrPluseData.vaild)
			{
				spin_unlock_irqrestore(&(g_IrAttr.Plusedata_lock), flags);
				up(&g_IrMutex);
            	return -EIO;
			}
			if (copy_to_user(argp, &g_IrPluseData, sizeof(ir_pluse_data_s)) != 0)
            {
            	spin_unlock_irqrestore(&(g_IrAttr.Plusedata_lock), flags);
				up(&g_IrMutex);
            	return -EIO;
            }
			g_IrPluseData.cur = 0;
			g_IrPluseData.vaild = IR_PLUSE_INVAILD;
			spin_unlock_irqrestore(&(g_IrAttr.Plusedata_lock), flags);
			break;
		default:
			MT_ERR_IR("Error: Inappropriate ioctl for device. cmd=%d\n", cmd);
			up(&g_IrMutex);
			return -ENOTTY;
	}

	up(&g_IrMutex);
	return MT_SUCCESS;
}

static ssize_t IR_Read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	IR_KEY_S ReadIrKey;
	mt_u32 ReadLen = 0;
	mt_s32 Ret = 0;

	Ret = down_interruptible(&g_IrMutex);
	if (Ret)
	{
		MT_FATAL_IR("Semaphore lock is  error. \n");
		return MT_FAILURE;
	}

	while ((g_IrAttr.Head) == (g_IrAttr.Tail))
	{
		if (((filp->f_flags & O_NONBLOCK) == O_NONBLOCK) || (0 == g_IrAttr.IrBlockTime))
		{
			up(&g_IrMutex);
			MT_WARN_IR("the data buf is null.\n");
			return -EAGAIN;
		}

		if (0xffffffff == g_IrAttr.IrBlockTime)
		{
			Ret = wait_event_interruptible(g_IrAttr.IrKeyWaitQueue, (g_IrAttr.Head != g_IrAttr.Tail));//TASK_INTERRUPTIBLE
			if (Ret < 0)
			{
				up(&g_IrMutex);
				MT_ERR_IR("wait data err.\n");
				return -ERESTARTSYS;
			}
		}
		else
		{
			Ret = wait_event_hrtimeout(g_IrAttr.IrKeyWaitQueue,
										(g_IrAttr.Head != g_IrAttr.Tail),
										ms_to_ktime(g_IrAttr.IrBlockTime));//TASK_UNINTERRUPTIBLE

			if (Ret == -ETIME)
			{
				up(&g_IrMutex);
				MT_WARN_IR("wait data timeout.\n");
				return MT_ERR_IR_READ_FAILED;
			}
			else if (Ret < 0)
			{
				up(&g_IrMutex);
				MT_ERR_IR("wait data err.\n");
				return -ERESTARTSYS;
			}
		}
	}

	while (((g_IrAttr.Head) != (g_IrAttr.Tail)) && ((ReadLen + sizeof(IR_KEY_S)) <= count))
	{
		ReadIrKey = IR_BUF_TAIL;
		g_IrAttr.Tail = INC_BUF(g_IrAttr.Tail, g_IrAttr.IrKeyBufLen);

		if (copy_to_user((buf + ReadLen), &ReadIrKey, sizeof(IR_KEY_S)))
		{
			MT_FATAL_IR("copy data to user failed.\n");
			up(&g_IrMutex);
			return MT_FAILURE;
		}

		ReadLen += sizeof(IR_KEY_S);

#if 0
		if (MT_UNF_KEY_STATUS_DOWN == ReadIrKey.IrKeyState)
		{
		mt_drv_stat_event(STAT_EVENT_KEYOUT, ReadIrKey.IrKeyDataL);
		}
#endif
	}

	up(&g_IrMutex);
	return ReadLen;
}

#if 0
mt_u32 IR_Select(struct file *filp, struct poll_table_struct *wait)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_IrMutex);
    if (Ret)
    {
        MT_FATAL_IR("Semaphore lock is  error. \n");
        return MT_FAILURE;
    }

    if ((g_IrAttr.Head) != (g_IrAttr.Tail))
    {
        up(&g_IrMutex);
        return 1;
    }

    poll_wait(filp, &(g_IrAttr.IrKeyWaitQueue), wait);

    up(&g_IrMutex);
    return 0;
}
#endif

mt_s32 IR_Open(struct inode *inode, struct file *filp)
{
	mt_s32 ret;

	ret = down_interruptible(&g_IrMutex);
	if (ret)
	{
		MT_FATAL_IR("Semaphore lock is  error. \n");
		return MT_FAILURE;
	}

	//MT_INFO_IR("IR_Open!\n");
	if (1 == atomic_inc_return(&g_IrCount))
	{
	#if (!((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE)))
		g_IrAttr.user_code_set = MT_FALSE;
		memset(g_IrAttr.sw_user_code, 0x0, sizeof(g_IrAttr.sw_user_code));
		g_IrAttr.Head = 0;
		g_IrAttr.Tail = 0;
		g_IrAttr.IrKeyBufLen = IR_MAX_BUF;
		g_IrAttr.RepkeyDelayTime = IR_DELAY_TIME;
		g_IrAttr.IrBlockTime = 0xffffffff;
		spin_lock_init(&g_IrAttr.Plusedata_lock);
	    is_ir_intialized = MT_FALSE;

		init_waitqueue_head(&g_IrAttr.IrKeyWaitQueue);

		ir_softreset();
	 	ret =  request_irq(IRQ_IRDA_ID, (irq_handler_t)ir_interrupt, IRQF_TRIGGER_RISING, "IR_SYMPHONY", NULL);
		if (ret != MT_SUCCESS)
		{
			MT_ERR_IR("register IR INT failed 0x%x.\n", ret);
			atomic_dec(&g_IrCount);
			up(&g_IrMutex);
			return MT_FAILURE;
		}
	#endif
    }

    up(&g_IrMutex);
    return 0;
}

mt_s32 IR_Close(struct inode *inode, struct file *filp)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_IrMutex);
    if (Ret)
    {
        MT_FATAL_IR("Semaphore lock is  error. \n");
        return MT_FAILURE;
    }

    if (atomic_dec_and_test(&g_IrCount))
    {
        MT_INFO_IR("free_ir_irq!\n");
        free_irq(IRQ_IRDA_ID, MT_NULL);
    }

    is_ir_intialized = MT_FALSE;

    up(&g_IrMutex);

    return 0;
}

static ssize_t IR_Write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    return 0;
}

static struct file_operations IR_FOPS =
{
    owner   : THIS_MODULE,
    open    : IR_Open,
    unlocked_ioctl   : Ir_Ioctl,
    /*poll    : IR_Select,*/
    read    : IR_Read,
    write   : IR_Write,
    release : IR_Close,
};


static int ir_suspend (basedev_s *pdev, pm_message_t state)
{
    MT_PRINT("IR suspend OK\n");
    return 0;
}

static int ir_resume(basedev_s *pdev)
{
	ir_softreset();
	if(g_ir_filter.irda_protocol == IRDA_NEC)
	{
		ir_hw_decoder_init(&ir_settings);
	}
	else
	{
		ir_soft_decoder_init(&ir_settings);
	}
	ir_readl(R_IR_BASE_ADDR+0x14);//fix 125959, but don't find rootcause at present.
	irda_symphony_set_hw_usercode(ir_settings.ucfilter_en, ir_settings.usercode);
	irda_symphony_set_hw_keycode(ir_settings.kcfilter_en, ir_settings.keycode);
	irda_symphony_set_wfilt(g_ir_filter);
	ir_readl(R_IR_BASE_ADDR+0x14);//fix 125959, but don't find rootcause at present.
    MT_PRINT("IR resume OK\n");
    return 0;
}

static baseops_s ir_baseOps = {
    .probe	= NULL,
    .remove = NULL,
    .shutdown = NULL,
    .prepare  = NULL,
    .complete = NULL,
    .suspend  = ir_suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume		  = ir_resume
};

static mt_s32 irda_Proc(struct seq_file *p, mt_void *v)
{
	p += PROC_PRINT(p, "this is irda\n");
	switch(ir_settings.protocol)
	{
		case IRDA_NEC:
			p += PROC_PRINT(p, "irda_protocol = IRDA_NEC \n");
			break;

		case IRDA_SW_NEC:
			p += PROC_PRINT(p, "irda_protocol = IRDA_SW_NEC \n");
			break;

		case IRDA_SW_RC5:
			p += PROC_PRINT(p, "irda_protocol = IRDA_SW_RC5 \n");
			break;

		case IRDA_SW_RCMM:
			p += PROC_PRINT(p, "irda_protocol = IRDA_SW_RCMM \n");
			break;

		default:
			p += PROC_PRINT(p, "irda_protocol = IRDA_SW_MAX \n");
			break;
	}
	//p += PROC_PRINT(p, "irda  wfilt_code = %0x\n",g_ir_filter.irda_wfilt_channel_cfg[0].wfilt_code);

	return MT_SUCCESS;
}

mt_s32 __init IR_DRV_ModInit(mt_void)
{
#if ((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE))
	mt_s32 ret = 0;
	struct mt_ir_priv *priv = NULL;
#endif

    mt_proc_entry_t *pProcItem;
    mt_drv_proc_t irdaFnOpt =
    {
       .fnRead = irda_Proc,
    };
    return 0;

#if ((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE))
	priv = kzalloc(sizeof(struct mt_ir_priv), GFP_KERNEL);
	if (!priv)
	{
		pr_err("[%s %d]no memory\n", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}
	memset(priv, 0, sizeof(struct mt_ir_priv));
#endif

    (mt_void)mt_drv_module_register(MT_ID_IR, "MT_IR", MT_NULL);

    sprintf(g_IrRegisterData.devfs_name, UMAP_DEVNAME_IR);
    g_IrRegisterData.minor	= UMAP_MIN_MINOR_IR;
    g_IrRegisterData.owner	= THIS_MODULE;
    g_IrRegisterData.fops	= &IR_FOPS;
    g_IrRegisterData.drvops = &ir_baseOps;

    if (mt_drv_dev_register(&g_IrRegisterData) < 0)
    {
        MT_FATAL_IR("register IR failed.\n");
        return MT_FAILURE;
    }

    //mt_proc_entry_t *pProcItem;
    //mt_drv_proc_t irdaFnOpt =
    //{
    //   .fnRead = irda_Proc,
    //};

    pProcItem = mt_drv_proc_add_module("ir", &irdaFnOpt, NULL);
    if (!pProcItem)
    {
        MT_FATAL_IR("add IR proc failed.\n");
        mt_drv_dev_unregister(&g_IrRegisterData);
        return MT_FAILURE;
    }

    //pProcItem->read  = NULL;
   // pProcItem->write = NULL;

#if ((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE))
	#if (defined CONFIG_RC_DECODERS)
		priv->rc_dev = rc_allocate_device(RC_DRIVER_IR_RAW);
	#else
		priv->rc_dev = rc_allocate_device(RC_DRIVER_SCANCODE);
	#endif
	if (!priv->rc_dev)
	{
		printk(KERN_ERR "[%s %d]rc_allocate_device fail\n", __FUNCTION__, __LINE__);
		mt_drv_dev_unregister(&g_IrRegisterData);
		return -ENOMEM;
	}

	priv->rc_dev->allowed_protocols = RC_PROTO_BIT_NEC | RC_PROTO_BIT_RC5;
	priv->rc_dev->driver_name = MT_IR_NAME;
	priv->rc_dev->map_name = RC_MAP_MT_IRDEC;
	priv->rc_dev->device_name = MT_IR_NAME;
	priv->rc_dev->input_phys = MT_IR_NAME "/input0";
	priv->rc_dev->input_id.bustype = BUS_HOST;
	priv->rc_dev->input_id.vendor = 0x0001;
	priv->rc_dev->input_id.product = 0x0001;
	priv->rc_dev->input_id.version = 0x0100;
	//priv->rc_dev->rx_resolution = US_TO_NS(10);
	//priv->rc_dev->timeout = US_TO_NS(110000);

	ret = rc_register_device(priv->rc_dev);
	if (ret < 0)
	{
		printk(KERN_ERR "[%s %d]ret=%d, rc_register_device failed\n", __FUNCTION__, __LINE__, ret);
		rc_free_device(priv->rc_dev);
		mt_drv_dev_unregister(&g_IrRegisterData);
		return MT_FAILURE;
	}

	g_IrRegisterData.priv = (void *)priv;

	ir_softreset();
	#if (defined CONFIG_RC_DECODERS)
		ir_settings.protocol = IRDA_SW_NEC;
		ir_soft_decoder_init(&ir_settings);
	#else
		ir_settings.protocol = IRDA_NEC;
		ir_hw_decoder_init(&ir_settings);
	#endif
	g_ir_filter.irda_protocol = ir_settings.protocol;
	mt_ir_set_rc_protocol(ir_settings.protocol);
	g_IrAttr.user_code_set = MT_FALSE;
	memset(g_IrAttr.sw_user_code, 0x0, sizeof(g_IrAttr.sw_user_code));
	g_IrAttr.Head = 0;
	g_IrAttr.Tail = 0;
	g_IrAttr.IrKeyBufLen = IR_MAX_BUF;
	g_IrAttr.RepkeyDelayTime = IR_DELAY_TIME;
	g_IrAttr.IrBlockTime = 0xffffffff;
    is_ir_intialized = MT_FALSE;
	spin_lock_init(&g_IrAttr.Plusedata_lock);
	init_waitqueue_head(&g_IrAttr.IrKeyWaitQueue);
 	ret = request_irq(IRQ_IRDA_ID, (irq_handler_t)ir_interrupt, IRQF_TRIGGER_RISING, "IR_SYMPHONY", NULL);
	if (ret != MT_SUCCESS)
	{
		printk(KERN_ERR "[%s %d]request_irq failed\n", __FUNCTION__, __LINE__);
		rc_unregister_device(priv->rc_dev);
		rc_free_device(priv->rc_dev);
		mt_drv_dev_unregister(&g_IrRegisterData);
		return MT_FAILURE;
	}

	printk(KERN_ERR "[%s %d]IR init success\n", __FUNCTION__, __LINE__);
#endif

    return 0;
}

mt_void __exit IR_DRV_ModExit(mt_void)
{
    return;
    //mt_drv_proc_rm_module(MT_MOD_IR);
#if ((defined IR_RC_SUPPORT)&&(defined CONFIG_RC_CORE))
    struct mt_ir_priv *priv = (struct mt_ir_priv *)g_IrRegisterData.priv;

    rc_unregister_device(priv->rc_dev);
	rc_free_device(priv->rc_dev);
#endif
    mt_drv_dev_unregister(&g_IrRegisterData);
    mt_drv_module_unregister(MT_ID_IR);

    return;
}

