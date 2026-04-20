/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*
 * Mailbox module header file 
 */
#ifndef __MAILBOX_H_
#define __MAILBOX_H_

/*  AP cpu w*/
#define  MCPU_SCPU_MB_BASE		SYMPHONY_MAILBOX_VIRT_BASE
//	0xBF128000

#define  MCPU_MB_INIT     (MCPU_SCPU_MB_BASE) 				//0xBF128000
#define  MCPU_MB_INTCLR   ((MCPU_SCPU_MB_BASE)+0x04)	//0xBF128004
#define  MCPU_MB_ENACLR   ((MCPU_SCPU_MB_BASE)+0x08) //0xBF128008
#define  SCPU_MB_INTSET   ((MCPU_SCPU_MB_BASE)+0x0C) //0xBF12800C
#define  SCPU_MB_ENASET   ((MCPU_SCPU_MB_BASE)+0x10) //0xBF128010

/*Ap share data for scpu*/
#define  SCPU_MB_ADDR_BASE ((MCPU_SCPU_MB_BASE)+0x14)//0xBF128014
#define  SCPU_MB_ADDR_REG(chan) (SCPU_MB_ADDR_BASE + (chan) * 4)


/* s cpu w */
#define  SCPU_MB_INIT     ((MCPU_SCPU_MB_BASE)+0xC0) //0xBF1280C0
#define  SCPU_MB_INTCLR   ((MCPU_SCPU_MB_BASE)+0xC4) //0xBF1280C4
#define  SCPU_MB_ENACLR   ((MCPU_SCPU_MB_BASE)+0xC8) //0xBF1280C8
#define  MCPU_MB_INTSET   ((MCPU_SCPU_MB_BASE)+0xCC) //0xBF1280CC
#define  MCPU_MB_ENASET   ((MCPU_SCPU_MB_BASE)+0xD0) //0xBF1280D0

/*scpu share data for ap cpu*/
#define  MCPU_MB_ADDR_BASE ((MCPU_SCPU_MB_BASE)+0xD4) //0xBF1280D4
#define  MCPU_MB_ADDR_REG(chan) (MCPU_MB_ADDR_BASE + (chan) * 4)

/* all cpu can w */
#define  MCPU_MB_INT   		((MCPU_SCPU_MB_BASE)+0x180) //0xBF128180
#define  MCPU_MB_INT_STATE  MCPU_MB_INT   
#define  MCPU_MB_ENA   		((MCPU_SCPU_MB_BASE)+0x184)	//0xBF128184
#define  MCPU_MB_ENA_STATE  MCPU_MB_ENA

#define  SCPU_MB_INT   			((MCPU_SCPU_MB_BASE)+0x188)	//0xBF128188
#define  SCPU_MB_INT_STATE   	SCPU_MB_INT
#define  SCPU_MB_ENA   			((MCPU_SCPU_MB_BASE)+0x18C)	//0xBF12818C
#define  SCPU_MB_ENA_STATE   	SCPU_MB_ENA

#define  MB_MEMLOCK_BASE   	((MCPU_SCPU_MB_BASE)+0x200)//0xBF128200 //0xBF128040
#define  MB_MEMLOCK_REG(chan) (MB_MEMLOCK_BASE + (chan) * 4)

/*======================================[ MCPU ECPU ]=============================================*/
/*M2E cpu BASE */
#define  MCPU_ECPU_MB_BASE 		0xBF124000

/*MCPU W*/
#define  M2ECPU_MB_INIT     	(MCPU_ECPU_MB_BASE) 				//0xBF128000
#define  M2ECPU_MB_INTCLR			((MCPU_ECPU_MB_BASE)+0x04)	//0xBF128004
#define  M2ECPU_MB_ENACLR   	((MCPU_ECPU_MB_BASE)+0x08) //0xBF128008
#define  M2ECPU_MB_INTSET   	((MCPU_ECPU_MB_BASE)+0x0C) //0xBF12800C
#define  M2ECPU_MB_ENASET   	((MCPU_ECPU_MB_BASE)+0x10) //0xBF128010

/*Ap share data for ecpu*/
#define  M2ECPU_MB_ADDR_BASE 			((MCPU_ECPU_MB_BASE)+0x14)//0xBF128014
#define  M2ECPU_MB_ADDR_REG(chan) (M2ECPU_MB_ADDR_BASE + (chan) * 4)


/* E cpu w */
#define  E2MCPU_MB_INIT     	((MCPU_ECPU_MB_BASE)+0xC0) //0xBF1280C0
#define  E2MCPU_MB_INTCLR   	((MCPU_ECPU_MB_BASE)+0xC4) //0xBF1280C4
#define  E2MCPU_MB_ENACLR   	((MCPU_ECPU_MB_BASE)+0xC8) //0xBF1280C8
#define  E2MCPU_MB_INTSET   	((MCPU_ECPU_MB_BASE)+0xCC) //0xBF1280CC
#define  E2MCPU_MB_ENASET   	((MCPU_ECPU_MB_BASE)+0xD0) //0xBF1280D0

/*Ecpu share data for ap cpu*/
#define  E2MCPU_MB_ADDR_BASE 			((MCPU_ECPU_MB_BASE)+0xD4) //0xBF1280D4
#define  E2MCPU_MB_ADDR_REG(chan) (E2MCPU_MB_ADDR_BASE + (chan) * 4)

/* all cpu can w */
#define  MECPU_MB_INT_STATE  ((MCPU_ECPU_MB_BASE)+0x180)   
#define  MECPU_MB_ENA_STATE  ((MCPU_ECPU_MB_BASE)+0x184)

#define  EMCPU_MB_INT_STATE   ((MCPU_ECPU_MB_BASE)+0x188)
#define  EMCPU_MB_ENA_STATE   ((MCPU_ECPU_MB_BASE)+0x18C)

#define  MECPU_MB_MEMLOCK_BASE   	 ((MCPU_ECPU_MB_BASE)+0x200)	//0xBF128200 //0xBF128040
#define  MECPU_MB_MEMLOCK_REG(chan) (MECPU_MB_MEMLOCK_BASE + (chan) * 4)

#define MAX_CHANNEL_INDEX	 8

typedef enum {
 	T_MEMLOCK_SPINLOCK = 0,
	T_MSGLOCK_SPINLOCK = 1,	
	T_CRYPTOLOCK_SPINLOCK = 2,
 	T_MEMLOCK_MAX = 32,
}en_memlock;

/* cpu id to identify which cpu this local cpu is */
#define MASTER_CPU_ID  0
#define SLAVE_CPU_ID  1
#define SECURE_CPU_ID  2
#define CPU_ID_MASK  0x3FF  /* bit 0:9 in ebase */
#define INVALID_CPU_ID 0xFF

#define CPU_NOT_READY  0
#define CPU_READY      1

#define MB_MSG_SPRAM_RD 0
#define MB_MSG_SPRAM_WR 1
#define MB_MSG_MCPU_RDY 2
#define MB_MSG_SCPU_RDY 3

#define MB_MSG_CREATE_PIPE  4


struct mb_msg
{
  unsigned int ttl_len;  /* total length of this message */
  unsigned int msg_type; /* what the intention of the msg is for */  
};

struct mb_memlock 
{
  struct mb_msg *p_pmsg;
  unsigned int valid;
};

/*!
  Message transfered between device drivers even if thay are run on different CPU 
  */
typedef struct
{
  /*!
      messege id, see dev_pipe_msg_id_t
    */ 
  unsigned int msg_id;
  /*!
      messege 1st parameter
    */   
  ulong param1;
  /*!
      messege 2nd parameter
    */    
  unsigned int param2;
  /*!
     sem
  */   
  unsigned int sem;
  /*!
      time out (ms)
  */  
  unsigned int time_out_ms;
}ipc_msg_t;
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
  unsigned int msg_id_mask;
  /*!
      the source processor id
    */   
  unsigned int p_sid;
  /*!
      the destin processor id
    */   
  unsigned int p_did;
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
   /*!
      spinlock
    */   
  unsigned int sp_lock;
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
  unsigned int used;
}pipe_info_t;

/*!
    struct for Mailbox Message
  */
typedef struct
{
  /*!
    * pipe  message
    */   
  ipc_msg_t p_msg;
  /*!
    *  processor id
    */     
  unsigned int local_id;
}mb_msg_t;

/**
 * @brief Mailbox_Sec init.
 */
void ipcs_mbx_init(void);

/**
 * @brief Mailbox_Sec de-init. 
 */
void ipcs_mbx_deinit(void);

/**
 * @brief Send message to VSCPU
 *
 * @param channel
 * @param msg
 *
 * @return 
 */
int ipcs_send_mbx_msg(unsigned int channel, ipc_msg_t *msg);

/**
 * @brief Receive message from VSCPU
 *
 * @param channel
 * @param p_msg
 *
 * @return 
 */
int ipcs_recv_mbx_msg(unsigned char channel, ipc_msg_t *p_msg);

int ipcs_check_seccpu_status(void);
void ipcs_set_local_status(int status);

#endif	/* __MAILBOX_H_ */
