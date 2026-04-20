/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __IPC_H__
#define __IPC_H__

#include "mt_cache.h"

#if 1
#define AP_CPU_ID  0
#define AV_CPU_ID  1
#define CPU_ID_MASK  0x3FF
#define INVALID_CPU_ID 0xFF

#define LOCAL_CPU_ID  (read_c0_ebase() & CPU_ID_MASK)

#define CPU_IS_AP  ((AP_CPU_ID == LOCAL_CPU_ID) ? 1 : 0)
#define CPU_NOT_READY  0
#define CPU_READY      1

#define MSG_ID_CREATE_PIPE 0xABCD7777
#endif

/**
 * Fixed 64 Cache Line size align!
 * 64 is compatible both APCPU and AVCPU!
 */
#define IPC_MSG_ALIGN_ATTR			__attribute__ ((aligned (64)))


/*================== BEGIN [ MCPU ECPU ] 2016.05.12 ===================*/
/*  AP cpu w*/
#define  SYMPHONY_MCPU_SCPU_MB_BASE			0xBF128000

#define  SYMPHONY_MCPU_MB_INIT     (SYMPHONY_MCPU_SCPU_MB_BASE) 				//0xBF128000
#define  SYMPHONY_MCPU_MB_INTCLR   ((SYMPHONY_MCPU_SCPU_MB_BASE)+0x04)	//0xBF128004
#define  SYMPHONY_MCPU_MB_ENACLR   ((SYMPHONY_MCPU_SCPU_MB_BASE)+0x08) //0xBF128008
#define  SYMPHONY_SCPU_MB_INTSET   ((SYMPHONY_MCPU_SCPU_MB_BASE)+0x0C) //0xBF12800C
#define  SYMPHONY_SCPU_MB_ENASET   ((SYMPHONY_MCPU_SCPU_MB_BASE)+0x10) //0xBF128010

/*Ap share data for scpu*/
#define  SYMPHONY_SCPU_MB_ADDR_BASE ((SYMPHONY_MCPU_SCPU_MB_BASE)+0x14)//0xBF128014
#define  SYMPHONY_SCPU_MB_ADDR_REG(chan) (SYMPHONY_SCPU_MB_ADDR_BASE + (chan) * 4)

/* s cpu w */
#define  SYMPHONY_SCPU_MB_INIT     ((SYMPHONY_MCPU_SCPU_MB_BASE)+0xC0) //0xBF1280C0
#define  SYMPHONY_SCPU_MB_INTCLR   ((SYMPHONY_MCPU_SCPU_MB_BASE)+0xC4) //0xBF1280C4
#define  SYMPHONY_SCPU_MB_ENACLR   ((SYMPHONY_MCPU_SCPU_MB_BASE)+0xC8) //0xBF1280C8
#define  SYMPHONY_MCPU_MB_INTSET   ((SYMPHONY_MCPU_SCPU_MB_BASE)+0xCC) //0xBF1280CC
#define  SYMPHONY_MCPU_MB_ENASET   ((SYMPHONY_MCPU_SCPU_MB_BASE)+0xD0) //0xBF1280D0

/*scpu share data for ap cpu*/
#define  SYMPHONY_MCPU_MB_ADDR_BASE ((SYMPHONY_MCPU_SCPU_MB_BASE)+0xD4) //0xBF1280D4
#define  SYMPHONY_MCPU_MB_ADDR_REG(chan) (SYMPHONY_MCPU_MB_ADDR_BASE + (chan) * 4)

/* all cpu can w */
#define  SYMPHONY_MCPU_MB_INT   		((SYMPHONY_MCPU_SCPU_MB_BASE)+0x180) //0xBF128180
#define  SYMPHONY_MCPU_MB_INT_STATE  SYMPHONY_MCPU_MB_INT
#define  SYMPHONY_MCPU_MB_ENA   		((SYMPHONY_MCPU_SCPU_MB_BASE)+0x184)	//0xBF128184
#define  SYMPHONY_MCPU_MB_ENA_STATE  SYMPHONY_MCPU_MB_ENA

#define  SYMPHONY_SCPU_MB_INT   			((SYMPHONY_MCPU_SCPU_MB_BASE)+0x188)	//0xBF128188
#define  SYMPHONY_SCPU_MB_INT_STATE   	SYMPHONY_SCPU_MB_INT
#define  SYMPHONY_SCPU_MB_ENA   			((SYMPHONY_MCPU_SCPU_MB_BASE)+0x18C)	//0xBF12818C
#define  SYMPHONY_SCPU_MB_ENA_STATE   	SYMPHONY_SCPU_MB_ENA

#define  SYMPHONY_MB_MEMLOCK_BASE   	((SYMPHONY_MCPU_SCPU_MB_BASE)+0x200)//0xBF128200 //0xBF128040
#define  SYMPHONY_MB_MEMLOCK_REG(chan) (SYMPHONY_MB_MEMLOCK_BASE + (chan) * 4)

/*M2E cpu BASE */
#define  SYMPHONY_MCPU_ECPU_MB_BASE 		0xBF124000

/*MCPU W*/
#define  SYMPHONY_M2ECPU_MB_INIT     	(SYMPHONY_MCPU_ECPU_MB_BASE) 				//0xBF128000
#define  SYMPHONY_M2ECPU_MB_INTCLR			((SYMPHONY_MCPU_ECPU_MB_BASE)+0x04)	//0xBF128004
#define  SYMPHONY_M2ECPU_MB_ENACLR   	((SYMPHONY_MCPU_ECPU_MB_BASE)+0x08) //0xBF128008
#define  SYMPHONY_M2ECPU_MB_INTSET   	((SYMPHONY_MCPU_ECPU_MB_BASE)+0x0C) //0xBF12800C
#define  SYMPHONY_M2ECPU_MB_ENASET   	((SYMPHONY_MCPU_ECPU_MB_BASE)+0x10) //0xBF128010

/*Ap share data for ecpu*/
#define  SYMPHONY_M2ECPU_MB_ADDR_BASE 			((SYMPHONY_MCPU_ECPU_MB_BASE)+0x14)//0xBF128014
#define  SYMPHONY_M2ECPU_MB_ADDR_REG(chan) (SYMPHONY_M2ECPU_MB_ADDR_BASE + (chan) * 4)

/* E cpu w */
#define  SYMPHONY_E2MCPU_MB_INIT     	((SYMPHONY_MCPU_ECPU_MB_BASE)+0xC0) //0xBF1280C0
#define  SYMPHONY_E2MCPU_MB_INTCLR   	((SYMPHONY_MCPU_ECPU_MB_BASE)+0xC4) //0xBF1280C4
#define  SYMPHONY_E2MCPU_MB_ENACLR   	((SYMPHONY_MCPU_ECPU_MB_BASE)+0xC8) //0xBF1280C8
#define  SYMPHONY_E2MCPU_MB_INTSET   	((SYMPHONY_MCPU_ECPU_MB_BASE)+0xCC) //0xBF1280CC
#define  SYMPHONY_E2MCPU_MB_ENASET   	((SYMPHONY_MCPU_ECPU_MB_BASE)+0xD0) //0xBF1280D0

/*Ecpu share data for ap cpu*/
#define  SYMPHONY_E2MCPU_MB_ADDR_BASE 			((SYMPHONY_MCPU_ECPU_MB_BASE)+0xD4) //0xBF1280D4
#define  SYMPHONY_E2MCPU_MB_ADDR_REG(chan) (SYMPHONY_E2MCPU_MB_ADDR_BASE + (chan) * 4)

/* all cpu can w */
#define  SYMPHONY_MECPU_MB_INT_STATE  ((SYMPHONY_MCPU_ECPU_MB_BASE)+0x180)
#define  SYMPHONY_MECPU_MB_ENA_STATE  ((SYMPHONY_MCPU_ECPU_MB_BASE)+0x184)

#define  SYMPHONY_EMCPU_MB_INT_STATE   ((SYMPHONY_MCPU_ECPU_MB_BASE)+0x188)
#define  SYMPHONY_EMCPU_MB_ENA_STATE   ((SYMPHONY_MCPU_ECPU_MB_BASE)+0x18C)

#define  SYMPHONY_MECPU_MB_MEMLOCK_BASE   	 ((SYMPHONY_MCPU_ECPU_MB_BASE)+0x200)	//0xBF128200 //0xBF128040
#define  SYMPHONY_MECPU_MB_MEMLOCK_REG(chan) (SYMPHONY_MECPU_MB_MEMLOCK_BASE + (chan) * 4)
/*============== END [ MCPU ECPU ] 2016.05.12 ====================*/

typedef enum {
 	T_MEMLOCK_SPINLOCK = 0,
	T_MSGLOCK_SPINLOCK = 1,
 	T_MEMLOCK_MAX = 32,
}en_memlock;

#define MASTER_CPU_ID  0
#define SLAVE_CPU_ID  1
#define SECURE_CPU_ID  2
#define CPU_ID_MASK  0x3FF
#define INVALID_CPU_ID 0xFF

#define CPU_NOT_READY  0


#define MB_MSG_SPRAM_RD 0
#define MB_MSG_SPRAM_WR 1
#define MB_MSG_MCPU_RDY 2
#define MB_MSG_SCPU_RDY 3

#define MB_MSG_CREATE_PIPE  4

typedef enum{
	MB_T_INVALID = 0,
	MB_T_DDR_READY,
	MB_T_R_REG,
	MB_T_W_REG,
	MB_T_R_OTP,
	MB_T_W_OTP,
	MB_T_FLASH_KEY_ADDR,
	MB_T_FLASH_ENC,
	MB_T_FLASH_DEC,
	MB_T_CRYPTO_ENGINE,
	MB_T_RUN_SOS,
	MB_T_DESC_CW_SETTING,
	MB_T_DESC_KEYLADDER_SETTING,
	MB_T_DESC_ADV_CW_SETTING,
/*JUST FOR TEST CMD*/
	MB_T_TEST_START = 0X30,
	MB_T_TEST_DMA,
	MB_T_HDMI_HDCP,
	MB_T_MAX,
}en_mbox_type;

/*!
    struct mb msg
  */
struct mb_msg
{
/*!
    member 1
  */
  unsigned int ttl_len;
/*!
    member 2
  */
  unsigned int msg_type;
};

/*!
    struct for memlock
  */
struct mb_memlock
{
/*!
    memeber 1
  */
  struct mb_msg *p_pmsg;
/*!
    member 2
  */
  unsigned int valid;
};

#define MAX_PIPE_NUM 32

/*!
    struct for otp
  */
typedef struct _sym_otp_op
{
	/*!
   	 addr for otp
  	*/
	u32 addr;
	/*!
   	 addr for len
  	*/
	u32 len;
	/*!
   	 addr for data
  	*/
	u32 data;
}sym_otp_op;

/*!
  test case ipc for av
  */
#define AV_TESTCASE_IPC 0x80001001
/*!
  test case ipc for ap
  */
#define AP_TESTCASE_IPC 0x80001002
/*!
  RPC ipc for av
  */
#define AV_RPC_IPC 0x80001003
/*!
  RPC ipc for ap
  */
#define AP_RPC_IPC 0x80001004

/*!
  IPC App ID: General message communication Application Flag
  */
#define IPC_APP_MSG_COM_FLAG	0x00000000
/*!
  IPC App ID: RPC Application Flag
  */
#define IPC_APP_RPC_FLAG		0x80000000

/*!
  IPC App ID: Interrupt Application Flag
  */
#define IPC_APP_INTR_FLAG		0x40000000

/*!
  IPC App ID: Application MASK
  */
#define IPC_APP_MASK			0x0000FFFF

/*!
  IPC pipe message available event flag
  */
#define IPC_EVT_MSG_AVAILABLE_FLAG	0x00000001

/*!
  IPC pipe break event flag
  */
#define IPC_EVT_BREAK_FLAG			0x80000000

/*!
  test case msg id for video
  */
#define TEST_VIDEO_MSG_ID 0x000A
/*!
  test case msg id for audio
  */
#define TEST_AUDIO_MSG_ID 0x000B


/*!
   AP_CMD_ID
  */
#define AP_CMD_ID 0xA000A000
/*!
  AV_CMD_ID
  */
#define AV_CMD_ID 0xB000B000


/*!
  ap ipc regs
  */
#define AP_CPU_INT (*(volatile unsigned int *)0xbfe7001c)
/*!
  ap ipc regs
  */
#define AV_TO_AP_CMD0 (*(volatile unsigned int *)0xbfe70000)
/*!
  ap ipc regs
  */
#define AV_TO_AP_CMD1 (*(volatile unsigned int *)0xbfe70004)
/*!
  ap ipc regs
  */
#define AV_TO_AP_CMD2 (*(volatile unsigned int *)0xbfe70008)
/*!
  ap ipc regs
  */
#define AV_TO_AP_CMD3 (*(volatile unsigned int *)0xbfe7000C)
/*!
  ap ipc regs
  */
#define AV_TO_AP_DATA (*(volatile unsigned int *)0xbfe70010)
/*!
  ap ipc regs
  */
#define AV_TO_AP_CMD_READY (*(volatile unsigned int *)0xbfe70014)
/*!
  ap ipc regs
  */
#define AV_TO_AP_ACT_DONE (*(volatile unsigned int *)0xbfe70018)
/*!
  av ipc regs
  */
#define AP_TO_AV_CMD0 (*(volatile unsigned int *)0xbfd00000)
/*!
  av ipc regs
  */
#define AV_CPU_INT (*(volatile unsigned int *)0xbfd0001c)
/*!
  av ipc regs
  */
#define AP_TO_AV_CMD1 (*(volatile unsigned int *)0xbfd00004)
/*!
  av ipc regs
  */
#define AP_TO_AV_CMD2 (*(volatile unsigned int *)0xbfd00008)
/*!
  av ipc regs
  */
#define AP_TO_AV_CMD3 (*(volatile unsigned int *)0xbfd0000C)
/*!
  av ipc regs
  */
#define AP_TO_AV_DATA (*(volatile unsigned int *)0xbfd00010)
/*!
  av ipc regs
  */
#define AP_TO_AV_CMD_READY (*(volatile unsigned int *)0xbfd00014)
/*!
  av ipc regs
  */
#define AP_TO_AV_ACT_DONE (*(volatile unsigned int *)0xbfd00018)

/*!
  ap cpu magic word
  */
#define AP_CPU_READY_MAGIC (0x12345678)
/*!
  av cpu magic word
  */
#define AV_CPU_READY_MAGIC (0x87654321)
/*!
  av_ack_flag
  */
#define AV_ACK_FLAG (0xacacacac)
/*!
  ap_ack_flag
  */
#define AP_ACK_FLAG (0xcacacaca)

/*!
  Message transfered between device drivers even if thay are run on different CPU
  */
typedef struct
{
  /*!
      messege id, see dev_pipe_msg_id_t
    */
  u32 msg_id;
  /*!
      messege 1st parameter
    */
  /*
   * 20240314: Don't use ulong, phys_addr_t, related with ARCH!
   *		   Just use fixed u32 for AVCPU MIPS32.
   */
#if 0
  ulong param1;
#endif
  u32 param1;
  /*!
      messege 2nd parameter
    */
  u32 param2;

  /* Extended for transfer non-cacheable memory address and size */
  /*!
      non-cacheable physical memory address
    */
  /*
   * 20240314: Don't use ulong, phys_addr_t, related with ARCH!
   *		   Just use fixed u32 for AVCPU MIPS32.
   */
#if 0
  phys_addr_t phy_addr;
#endif
  u32 phy_addr;
  /*!
      non-cacheable physical memory size
    */
  u32 mem_size;
  /* Extended for transfer non-cacheable retval memory address */
  /*!
      non-cacheable physical memory address of return value
    */
  /*
   * 20240314: Don't use ulong, phys_addr_t, related with ARCH!
   *		   Just use fixed u32 for AVCPU MIPS32.
   */
#if 0
  phys_addr_t retval_phy_addr;
#endif
  u32 retval_phy_addr;

  /*!
      Extend application id for multi-application supporting
      0x00000000: general message communication application
      0x80000000: RPC application, low 16 bit: RPC Proxy Function ID
    */
  u32 app_id;

  /* Note: sem & time_out_ms should not send/receive as msg by IPC */
  /*!
     sem
  */
  u32 sem;
  /*!
      time out (ms)
  */
  u32 time_out_ms;

  /*!
      reserved, cache line size(32) byte aligned
    */
  //u32 reserved[0];

} IPC_MSG_ALIGN_ATTR ipc_msg_t;
/*!
    pipe max num
  */
#define MAX_PIPE_NUM 32
/*!
    struct for ipc_pipe_t
  */
typedef struct
{
  /*!
      messege id mask
    */
  u32 msg_id_mask;
  /*!
      the source processor id
    */
  u32 p_sid;
  /*!
      the destin processor id
    */
  u32 p_did;
}ipc_pipe_t;

/*!
    struct for pipe
  */
typedef struct pipe_info
{
  /*!
      pipe  message
    */
  ipc_msg_t *Datapipe;
  /*!
      read pointer
    */
  int ReadPt;
  /*!
      write pointer
    */
  int WritePt;
  /* 20171208 re-used for Event */
  union
  {
	   /*!
	      spinlock
	    */
	  u32 sp_lock;
	   /*!
	      event id
	    */
	  u32 event;
  };
  /*!
      this pipe infor
    */
  ipc_pipe_t pipe_info;
  /*!
      this pipe depth
    */
  int pipe_depth;
  /*!
      this pipe used or not
    */
  u32 used;
}pipe_info_t;

/*!
    struct for Mailbox Message
  */
typedef struct
{
  /*!
      pipe  message
    */
  ipc_msg_t p_msg;
  /*!
      processor id
    */
  u32 local_id;

  /*!
      reserved, cache line size(32) byte aligned
    */
  u32 reserved[6];

} IPC_MSG_ALIGN_ATTR mb_msg_t;

/*!
  check ap cpu status
  */
typedef RET_CODE (* check_ap_ready_t)(void);
/*!
  check av cpu status
  */
typedef RET_CODE (* check_av_ready_t)(void);
/*!
  ipc ap fifo init
  */
typedef void (* ap_ipc_init_t)(int max_pipe_num);
/*!
  ipc av fifo init
  */
typedef void (* av_ipc_init_t)(int max_pipe_num);
/*!
  ipc av fifo create
  */
typedef RET_CODE (* av_ipc_pipe_create_t)(ipc_pipe_t *p_pipe_t,int pipe_depth);
/*!
  ipc ap fifo create
  */
typedef RET_CODE (* ap_ipc_pipe_create_t)(ipc_pipe_t *p_pipe_t,int pipe_depth);
/*!
  ap send to av
  */
typedef RET_CODE (* ap_send_to_av_t)(u32 local_id, ipc_msg_t *p_msg, u8 ack_flag);
/*!
  av send to ap
  */
typedef RET_CODE (* av_send_to_ap_t)(u32 local_id, ipc_msg_t *p_msg, u8 ack_flag);
/*!
  ap received from av
  */
typedef RET_CODE (* ap_recv_from_av_t)(u32 local_id, ipc_msg_t *p_msg);
/*!
  av received from ap
  */
typedef RET_CODE (* av_recv_from_ap_t)(u32 local_id, ipc_msg_t *p_msg);
/*!
  ap send to ap
  */
typedef RET_CODE (* ap_send_to_ap_t)(u32 local_id, ipc_msg_t *p_msg);
/*!
  av send to av
  */
typedef RET_CODE (* av_send_to_av_t)(u32 local_id, ipc_msg_t *p_msg);
/*!
  ap received from ap
  */
typedef RET_CODE (* ap_recv_from_ap_t)(u32 local_id, ipc_msg_t *p_msg);
/*!
  av received from av
  */
typedef RET_CODE (* av_recv_from_av_t)(u32 local_id, ipc_msg_t *p_msg);


/*!
  ap received from ap
  */
typedef void (* ap_recv_down_ack_t)(void);
/*!
  av received from av
  */
typedef void (* av_recv_down_ack_t)(ipc_msg_t *p_msg);

/*!
  av poll pipe message
  */
typedef RET_CODE (* av_pipe_poll_t)(u32 local_id);

/*!
BEGIN: ECPU FUNC
*/
/*!
ECPU get cpu id
*/
typedef unsigned int (*get_local_cpu_id_t)(void);
/*!
ECPU get cpu ready
*/
typedef unsigned int (*check_cpu_ready_t)(unsigned int cpuid);
/*!
ECPU set cpu ready
*/
typedef unsigned int (*set_cpu_ready_t)(unsigned int cpuid);
/*!
ECPU get mb memlock
*/
typedef unsigned int (*mcpu_mb_get_memlock_t)(unsigned int chan);
/*!
ECPU release mb memlock
*/
typedef void (*mcpu_mb_release_memlock_t)(unsigned int chan);
/*!
ECPU mb notify
*/
typedef int (*mcpu_mb_notify_t)(unsigned int channel, ipc_msg_t *msg);
/*!
ECPU write pipe
*/
typedef  int (*mcpu_writepipe_t)(pipe_info_t *p_pipe, ipc_msg_t *p_msg);
/*!
ECPU read pipe
*/
typedef int (*mcpu_readpipe_t)(pipe_info_t *p_pipe, ipc_msg_t *p_msg);
/*!
ECPU mb msg set
*/
typedef int (*mcpu_mb_msg_set_t)(ipc_msg_t *p_msg);
/*!
ECPU mb msg recv
*/
typedef int (*mcpu_mb_msg_receive_t)(ipc_msg_t *p_msg);
/*!
ECPU mb msg pipe create
*/
typedef void (*mcpu_mb_msg_pipe_create_t)(void);
/*!
ECPU mailbox config
*/
typedef void (*mcpu_mailbox_config_t)(void);
/*!
ECPU mb msg get
*/
typedef int (*mcpu_mb_msg_work_t)(void);
/*!
ECPU reg write
*/
typedef u32 (*scpu_reg_wr_t)(u32 addr, u32 val);
/*!
ECPU reg read
*/
typedef u32 (*scpu_reg_rd_t)(u32 addr, u32 *result);
/*!
ECPU otp write
*/
typedef u32 (*scpu_otp_wr_t)(u32 addr, u8 len, u32 val);
/*!
ECPU otp read
*/
typedef u32 (*scpu_otp_rd_t)(u32 addr, u8 len, u32 *result);
/*
*	END: ECPU FUNC
*/
/*!
  av received from av
  */
typedef struct
{
/*!
  av ap ipc function
  */
  CACHE_ALIGN check_ap_ready_t     check_ap_ready_set;
  /*!
  av ap ipc function
  */
  CACHE_ALIGN check_av_ready_t     check_av_ready_set;
 /*!
  av ap ipc function
  */
  CACHE_ALIGN ap_ipc_init_t        ap_ipc_init_set;
  /*!
  av ap ipc function
  */
  CACHE_ALIGN av_ipc_init_t        av_ipc_init_set;
   /*!
  av ap ipc function
  */
  CACHE_ALIGN av_ipc_pipe_create_t av_ipc_pipe_create_set;
   /*!
  av ap ipc function
  */
  CACHE_ALIGN ap_ipc_pipe_create_t ap_ipc_pipe_create_set;
   /*!
  av ap ipc function
  */
  CACHE_ALIGN ap_send_to_av_t      ap_send_to_av_set;
   /*!
  av ap ipc function
  */
  CACHE_ALIGN av_send_to_ap_t      av_send_to_ap_set;
   /*!
  av ap ipc function
  */
  CACHE_ALIGN ap_recv_from_av_t    ap_recv_from_av_set;
   /*!
  av ap ipc function
  */
  CACHE_ALIGN av_recv_from_ap_t    av_recv_from_ap_set;
   /*!
  av ap ipc function
  */
  CACHE_ALIGN ap_send_to_ap_t      ap_send_to_ap_set;
    /*!
  av ap ipc function
  */
  CACHE_ALIGN av_send_to_av_t      av_send_to_av_set;
   /*!
  av ap ipc function
  */
  CACHE_ALIGN ap_recv_from_ap_t    ap_recv_from_ap_set;
  /*!
  av ap ipc function
  */
  CACHE_ALIGN av_recv_from_av_t    av_recv_from_av_set;
   /*!
  av ap ipc function
  */
  CACHE_ALIGN ap_recv_down_ack_t   ap_recv_down_ack_set;
   /*!
  av ap ipc function
  */
  CACHE_ALIGN av_recv_down_ack_t   av_recv_down_ack_set;

   /*!
  av ap ipc function
  */
  CACHE_ALIGN av_pipe_poll_t   av_pipe_poll_set;

    /*
   *  ap ecpu ipc func
   */
    /*!
   mb msg get
   */
   CACHE_ALIGN mcpu_mb_msg_work_t   mcpu_mb_msg_work_set;
    /*!
   mb mailbox config
   */
   CACHE_ALIGN mcpu_mailbox_config_t   mcpu_mailbox_config_set;
    /*!
   mb msg pipe create
   */
   CACHE_ALIGN mcpu_mb_msg_pipe_create_t   mcpu_mb_msg_pipe_create_set;
    /*!
   mb msg recv
   */
   CACHE_ALIGN mcpu_mb_msg_receive_t   mcpu_mb_msg_receive_set;
    /*!
   mb msg set
   */
   CACHE_ALIGN mcpu_mb_msg_set_t   mcpu_mb_msg_set_set;
    /*!
   mb readpipe
   */
   CACHE_ALIGN mcpu_readpipe_t   mcpu_readpipe_set;
    /*!
   mb write pipe
   */
   CACHE_ALIGN mcpu_writepipe_t   mcpu_writepipe_set;
    /*!
   mb notify
   */
   CACHE_ALIGN mcpu_mb_notify_t   mcpu_mb_notify_set;
    /*!
   mb release memlock
   */
   CACHE_ALIGN mcpu_mb_release_memlock_t   mcpu_mb_release_memlock_set;
    /*!
   mb memlock get
   */
   CACHE_ALIGN mcpu_mb_get_memlock_t   mcpu_mb_get_memlock_set;
    /*!
   mb cpu ready
   */
   CACHE_ALIGN set_cpu_ready_t   set_cpu_ready_set;
    /*!
   mb check cpu ready
   */
   CACHE_ALIGN check_cpu_ready_t   check_cpu_ready_set;
    /*!
   mb get cpu id
   */
   CACHE_ALIGN get_local_cpu_id_t   get_local_cpu_id_set;

   /*!
   ap scpu reg read
   */
   CACHE_ALIGN scpu_reg_rd_t   scpu_reg_rd_set;

   /*!
   ap scpu reg write
   */
   CACHE_ALIGN scpu_reg_wr_t   scpu_reg_wr_set;

   /*!
   ap scpu otp write
   */
   CACHE_ALIGN scpu_otp_wr_t   scpu_otp_wr_set;

   /*!
   ap scpu otp read
   */
   CACHE_ALIGN scpu_otp_rd_t   scpu_otp_rd_set;

}ipc_fw_fun_set_t;


/*!
  check ap cpu status
  */
RET_CODE check_ap_ready(void);
/*!
  check av cpu status
  */
RET_CODE check_av_ready(void);
/*!
  ipc ap fifo init
  */
void ap_ipc_init(int max_pipe_num);
/*!
  ipc av fifo init
  */
void av_ipc_init(int max_pipe_num);
/*!
  ipc av fifo create
  */
RET_CODE av_ipc_pipe_create(ipc_pipe_t *p_pipe_t,int pipe_depth);
/*!
  ipc ap fifo create
  */
RET_CODE ap_ipc_pipe_create(ipc_pipe_t *p_pipe_t,int pipe_depth);
/*!
  ap send to av
  */
RET_CODE ap_send_to_av(u32 local_id, ipc_msg_t *p_msg, u8 ack_flag);
/*!
  av send to ap
  */
RET_CODE av_send_to_ap(u32 local_id, ipc_msg_t *p_msg, u8 ack_flag);
/*!
  ap received from av
  */
RET_CODE ap_recv_from_av(u32 local_id, ipc_msg_t *p_msg);
/*!
  av received from ap
  */
RET_CODE av_recv_from_ap(u32 local_id, ipc_msg_t *p_msg);
/*!
  ap send to ap
  */
RET_CODE ap_send_to_ap(u32 local_id, ipc_msg_t *p_msg);
/*!
  av send to av
  */
RET_CODE av_send_to_av(u32 local_id, ipc_msg_t *p_msg);
/*!
  ap received from ap
  */
RET_CODE ap_recv_from_ap(u32 local_id, ipc_msg_t *p_msg);
/*!
  av received from av
  */
RET_CODE av_recv_from_av(u32 local_id, ipc_msg_t *p_msg);


/*!
  ap received from ap
  */
void ap_recv_down_ack(void);
/*!
  av received from av
  */
void av_recv_down_ack(ipc_msg_t *p_msg);

/*!
  av poll receiving pipe's message
  */
RET_CODE av_pipe_poll(u32 local_id);

/*!
  av received from av
  */
//void ipc_fw_attach(void);
#endif //__IPC_H__
