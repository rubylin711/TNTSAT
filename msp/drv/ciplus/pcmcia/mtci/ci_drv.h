/******************************************
montage ciplus register define
*******************************************/
typedef struct CI_REG {
	unsigned int reg_ci_ctrl;		//0x00
	unsigned int reg_ci_intsts;		//0x04
 	unsigned int reg_ci_intctrl;	//0x08
	unsigned int reg_ci_fsmcnt1;	//0x0c
 	unsigned int reg_ci_fsmcnt2;	//0x10
  	unsigned int reg_ci_cdcfg;		//0x14
  	unsigned int reg_ci_ts1ctrl;	//0x18
}ci_reg_struct;

/******************************************
ts source select register struct
******************************************/
typedef union REG_TS_SRC_SEL {
	unsigned int BYTE;
	struct BIT{
		unsigned int ts0_sel:1;
		unsigned int reserved_1_3:3;
		unsigned int ts1_sel:1;
		unsigned int reserved_5_11:3;
		unsigned int ts2_sel:1;
		unsigned int reserved_13_15:3;
		unsigned int ts3_sel:1;
		unsigned int reserved_17_19:3;
		unsigned int cam_s_sel:1;
		unsigned int reserved_21_23:3;
		unsigned int s_sel:1;
		unsigned int reserved_25_31:7;
	}bit;
}ts_src_sel;

/******************************************
ci controller status define
******************************************/
enum CI_HOST_STATUS
{
	CI_STATUS_HOST_OPEN 	= 0X01,
	CI_STATUS_HOST_CLOSE	= 0x02,
};

/******************************************
ci frequence enum define
******************************************/
enum CI_FREQ	{
	CI_FREQ_80M,
	CI_FREQ_120M ,
};

/******************************************
cam status define
******************************************/
enum CAM_STATUS
{
	CAM_OUT,
	CAM_IN,
};

/***************************************
interrupt status mask define
****************************************/
#define CI_INTST_CD1RMVD		0x01
#define CI_INTST_CD1INSTD		0x02
#define CI_INTST_CD2RMVD		0x04
#define CI_INTST_CD2INSTD		0x08

#define CI_CAMIN	(CI_INTST_CD1INSTD|CI_INTST_CD2INSTD)
#define CI_CAMOUT	(CI_INTST_CD1RMVD|CI_INTST_CD2RMVD)
#define CI_INTST_MASK					(0x1fff)

#define CI_INTCTRL_CAMDEC		(CI_CAMIN|CI_CAMOUT)
#define CI_INTCTRL_CAMIRQ		(0x20)

struct mt_ci_dev
{
	u32 flags;			/*ci host status*/
	ci_reg_struct *iobase;
	u32 cam_status;			/*cam status*/
	struct device *dev;
	struct cdev cdev;
	struct mutex iomutex;     /* mutual exclusion semaphore     */
	struct work_struct ci_wq;
	spinlock_t spinlock;
	struct clk *pclk;
	struct reset_control *reset;
	struct gpio_desc *gd_power;
	struct class *ci_class;
	ts_src_sel reg_ts_src_sel;
	int irq;
	int major;
	int minor;
	u32 resume_regs[8];
	int flag_res;
	unsigned long *cam_reg_base;
	struct device_node *dev_nd;
};

