/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HAL_SPDMA_H__
#define __HAL_SPDMA_H__

/*!!
  maximal dma channel's number
  */
#define HAL_SPDMA_MAX_CHAN_NUM 8


/*!
  DMA command
*/
#define SPDMA_IOC_MAGIC     's'

#define SPDMA_IOC_START					_IOWR(SPDMA_IOC_MAGIC, 0x1, hal_spdma_io_param_t)
#define SPDMA_IOC_STOP					_IOW(SPDMA_IOC_MAGIC, 0x2)
#define SPDMA_IOC_CLOSE					_IO(SPDMA_IOC_MAGIC, 0x3)
#define SPDMA_IOC_CHECK					_IOW(SPDMA_IOC_MAGIC, 0x5)
//#define SPDMA_IOC_START_WITH_CH			_IOWR(SPDMA_IOC_MAGIC, 0x11, hal_spdma_io_param_t)
//#define SPDMA_IOC_ACTIVE_CH				_IOWR(SPDMA_IOC_MAGIC, 0x12, int)
//#define SPDMA_IOC_DEACTIVE_CH			_IOWR(SPDMA_IOC_MAGIC, 0x13, int)

#define SPDMA_IOC_TESTCASE   			_IOWR(SPDMA_IOC_MAGIC, 0xC,hal_spdma_io_param_t)


/*
#define SPDMA_IOC_RESUME               _IO(SPDMA_IOC_MAGIC, 0x4)
#define SPDMA_IOC_CHECK                _IOW(SPDMA_IOC_MAGIC, 0x5, int)
#define SPDMA_IOC_PAUSE                _IO(SPDMA_IOC_MAGIC, 0x6)
#define SPDMA_IOC_RESET                _IO(SPDMA_IOC_MAGIC, 0x7)
#define SPDMA_IOC_SOFT_RESET           _IO(SPDMA_IOC_MAGIC, 0x8)
#define SPDMA_IOC_PUSH                 _IO(SPDMA_IOC_MAGIC, 0x9)
#define SPDMA_IOC_CAPACITY_GET         _IO(SPDMA_IOC_MAGIC, 0xA)
#define SPDMA_IOC_MEMCPY_TESTCASE      _IO(SPDMA_IOC_MAGIC, 0xB)

#define SPDMA_IOC_MEMSET_LLN_TESTCASE   _IOWR(SPDMA_IOC_MAGIC, 0xC,hal_dma_io_param_t)
#define SPDMA_IOC_MUTI_CHAIN_TESTCASE  _IO(SPDMA_IOC_MAGIC, 0xD)
#define SPDMA_IOC_PERFORMANCE_TESTCASE _IO(SPDMA_IOC_MAGIC, 0xE)
#define SPDMA_IOC_OVER_MEM_TESTCASE    _IO(SPDMA_IOC_MAGIC, 0xF)
#define SPDMA_IOC_MEMSET_TESTCASE   _IOWR(SPDMA_IOC_MAGIC, 0x10,hal_dma_capacity_t)

#define SPDMA_IOC_START_WITH_CH     _IOWR(SPDMA_IOC_MAGIC, 0x11, hal_dma_io_param_t)
#define SPDMA_IOC_ACTIVE_CH     _IOWR(SPDMA_IOC_MAGIC, 0x12, int)
#define SPDMA_IOC_DEACTIVE_CH     _IOWR(SPDMA_IOC_MAGIC, 0x13, int)
*/

#define SPDMA_IOC_MAXNR 32

/*!
  Success return
  */
#define SPDMA_SUCCESS ((mt_s32)0)
/*!
  Fail for common reason
  */
#define SPDMA_ERR_FAILURE ((mt_s32)-1)
/*!
  Fail for waiting timeout
  */
#define SPDMA_ERR_TIMEOUT ((mt_s32)-2)
/*!
  Fail for function param invalid
  */
#define SPDMA_ERR_PARAM ((mt_s32)-3)
/*!
  Fail for module status invalid
  */
#define SPDMA_ERR_STATUS ((mt_s32)-4)
/*!
  Fail for module busy
  */
#define SPDMA_ERR_BUSY ((mt_s32)-5)
/*!
  Fail for no enough memory
  */
#define SPDMA_ERR_NO_MEM ((mt_s32)-6)
/*!
  Fail for no enough resource
  */
#define SPDMA_ERR_NO_RSRC ((mt_s32)-7)
/*!
  Fail for hardware error
  */
#define SPDMA_ERR_HARDWARE ((mt_s32)-8)
/*!
  Fail for feature not support
  */
#define SPDMA_ERR_NOFEATURE ((mt_s32)-9)



typedef enum
{
    SPDMA_CMD_NULL,
	SPDMA_CMD_DDR_2_SPRAM,
	SPDMA_CMD_SPRAM_2_DDR,
	SPDMA_CMD_DDR_2_DDR,
	SPDMA_CMD_SPRAM_2_SPRAM,
	SPDMA_CMD_DDR_2_AUD,
	SPDMA_CMD_SPRAM_2_AUD,
	SPDMA_CMD_7,
	SPDMA_CMD_8,
	SPDMA_CMD_IRQ,
	SPDMA_CMD_CID_CHECK,
	SPDMA_CMD_CID_INIT,
	SPDMA_CMD_CID_START,
	SPDMA_CMD_BRAM_2_SPRAM,
	SPDMA_CMD_SPRAM_2_BRAM,
	//DMA_SUBCMD_MULTIPLE,
	SPDMA_CMD_HELP,
	SPDMA_CMD_MAX,
}spdma_cmd_type_t;

/*!!
  SPDMA transfer test type
  */
typedef enum
{
  SPDMA_TYPE_DDR2SPRAM = 0x01,

  SPDMA_TYPE_SPRAM2DDR = 0x02,

  SPDMA_TYPE_DDR2DDR = 0x03,

  SPDMA_TYPE_SPRAM2SPRAM = 0x04,

  SPDMA_TYPE_DDR2AUD = 0x05,

  SPDMA_TYPE_SPRAM2AUD = 0x06,
}hal_spdma_type_t;

/*!!
  DMA transfer unit
  */
typedef enum
{
  /*!!
    DMA transfer in unit of byte
    */
  SPDMA_TRANS_UNIT_BYTE = 0x01,
  /*!!
    DMA transfer in unit of half word
    */
  SPDMA_TRANS_UNIT_HALF = 0x02,
  /*!!
    DMA transfer in unit of word
    */
  SPDMA_TRANS_UNIT_WORD = 0x04,
  /*!!
    DMA transfer in unit of double-word
    */
  SPDMA_TRANS_UNIT_DWORD = 0x08
}hal_trans_unit_t;

/*!
  AVSPDMA status
  */
typedef enum
{
  /*!
    The AVSPDMA is in stop status
    */
  AVSPDMA_STATUS_STOP = 0,
  /*!
    The AVSPDMA is in running status
    */
  AVSPDMA_STATUS_RUNNING,
  /*!
    driver has not been inited
    */
  AVSPDMA_STATUS_LAST

}avspdma_status_t;

/*!!
  SPDMA status used by HAL_SPDMA_XXXX_XX_CHECK() as return value
  */
typedef enum
{
  /*!!
    The DMA is in stop status
    */
  SPDMA_STATUS_STOP = 0,
  /*!!
    The DMA is in running status
    */
  SPDMA_STATUS_RUNNING,
  /*!!
    The DMA is in pause status
    */
  SPDMA_STATUS_PAUSE,
  /*!!
    The DMA is finish a transmit status
    */
  SPDMA_STATUS_DONE,
  /*!!
    driver has not been inited
    */
  SPDMA_STATUS_LAST = SPDMA_STATUS_DONE
}spdma_status_t;

/*!!
  SPDMA event
  */
typedef enum
{
  /*!!
    driver has not been inited
    */
  SPDMA_EVENT_NOT_INIT = 0x01,
  /*!!
    a timeout occurred
    */
  SPDMA_EVENT_TIMEOUT = 0x02,
  /*!!
    DMA transfer error happend
    */
  SPDMA_EVENT_ERROR = 0x04,
  /*!!
    source interval buffer transfer complete
    */
  SPDMA_EVENT_SRC_INTERVAL_INTERRUPT = 0x80,
  /*!!
    destination interval buffer transfer complete
    */
  SPDMA_EVENT_DST_INTERVAL_INTERRUPT = 0x10,
  /*!!
    DMA transfer has been completed
    */
  SPDMA_EVENT_COMPLETE = 0x20,
  /*!!
    driver has not been inited
    */
  SPDMA_EVENT_LAST = SPDMA_EVENT_COMPLETE,
  /*!!
    DMA event mask
    */
  SPDMA_EVENT_MASK = 0x3F
}spdma_event_t;

/*!!
  DMA mode used by hal_dma_1d_start() and struct hal_dma_2d_param
  */
typedef enum
{
  /*!!
    The dma 2d mode, else is 1d
    */
  SPDMA_MODE_2D = 0x0400,
  /*!!
    ALU AND operation:   src & alu_data --> dst
    */
  SPDMA_MODE_ALU_AND = 0x010000,
  /*!!
    ALU OR operation:    src | alu_data --> dst
    */
  SPDMA_MODE_ALU_OR = 0x020000,
  /*!!
    ALU XOR operation:   src ^ alu_data --> dst
    */
  SPDMA_MODE_ALU_XOR = 0x040000,
  /*!!
    ALU NOT operation:   ~src --> dst
    */
  SPDMA_MODE_ALU_NOT = 0x080000,
  /*!!
    ALU FILL the user dword, used for the operation like memset
    */
  SPDMA_MODE_ALU_FILL = 0x100000,
  /*!!
    ALU mask
    */
  SPDMA_MODE_ALU_MASK = 0x1F0000,
}spdma_mode_t;

/*!!
  SPDMA channel configuration param
  */
typedef struct
{
/*!!
  Destination peripheral
  */
  mt_u8 dst_peripheral;
/*!!
  Source peripheral
  */
  mt_u8 src_peripheral;
/*!!
  Destination endian pattern
  */
  mt_u8 dst_endian;
/*!!
  Source endian pattern
  */
  mt_u8 src_endian;
/*!!
  Destination clock
  */
  mt_u8 dst_clk;
/*!!
  Source clock
  */
  mt_u8 src_clk;
/*!!
  DMA source address increase
  */
  mt_u8 src_i;
/*!!
  DMA destination address increase
  */
  mt_u8 dst_i;
/*!!
  DMA source transfer unit size
  */
  mt_u8 src_usize;
/*!!
  DMA destination transfer unit size
  */
  mt_u8 dst_usize;
/*!!
  DMA source burst size
  */
  mt_u8 src_bsize;
/*!!
  DMA destination burst size
  */
  mt_u8 dst_bsize;
} spdma_chn_config_t;


/*!!
  DMA  channel control param
  */
typedef struct
{
/*!!
  Interrupt enable: linked list operation completed
  */
  mt_u8 int_link_en;
/*!!
  Interrupt enable: linked list node operation completed
  */
  mt_u8 int_node_en;
/*!!
  Load param from registers
  */
  mt_u8 chn_param_reg_en;
} spdma_chn_control_t;


/*!
  DMA plus transfer's parameters
  */
typedef struct hal_spdma_param
{
   /*!!
    param count
    */
   mt_u32 count;
   mt_u32 type;//cmd
   mt_u32 shift_flag;
   mt_u32 saturated_flag;
   mt_u32 round_flag;
   mt_u32 cut_flag;
   mt_u32 burst_len;
   mt_u32 transit_len;
   mt_u32 data[10];
   /*!!
    Virtual Source address
    */
  unsigned long vir_src_addr;
  /*!!
    Virtual Destin address
    */
  unsigned long vir_dst_addr;
  /*!!
    Source address
    */
  phys_addr_t phy_src_addr;
  /*!!
    Destin address
    */
  phys_addr_t phy_dst_addr;

  /*!!
    DMA channel configuration param
  */
  //spdma_chn_config_t config;

}hal_spdma_param_t;


/*!
  DMA linked-list-node
  */
typedef struct
{
  /*!
    Next node address
    */
  mt_u32 next_node;
  /*!
    Configuration
    */
  mt_u32 config;
  /*!
    Source address
    */
  mt_u32 src_addr;
  /*!
    Destination address
    */
  mt_u32 dst_addr;
  /*!
    Source block size
    */
  mt_u32 src_block;
  /*!
    Destination block size
    */
  mt_u32 dst_block;
  /*!
    Transfer data count
    */
  mt_u32 trans_cnt;
  /*!
    ALU word
    */
  mt_u32 alu_word;
}spdma_chn_lln_t;

/*!!
  DMA notify callback function
  */
typedef mt_void (*HAL_SPDMA_NOTIFY) (
    /*!!
    The channel's id
    */
    mt_s32 id,
    /*!!
    The event mask
    */
    spdma_event_t event,
    /*!!
    The 1st parameter
    */
    mt_u32 param1,
    /*!!
    The 2nd parameter
    */
    mt_u32 param2);

/*!!
  DMA notify callback structure
  */
typedef struct
{
  /*!!
    The pointer of callback function
    */
  HAL_SPDMA_NOTIFY fn_cb;
  /*!!
    The mask of event fot this notify, see dma_event_t
    */
  mt_u32 event_mask;
  /*!!
    The 1st parameter of callback function
    */
  mt_u32 param1;
  /*!!
    The 2nd parameter of callback function
    */
  mt_u32 param2;
}hal_spdma_notify_t;





typedef struct
{
  hal_spdma_param_t param;
  hal_spdma_notify_t *p_notify;
  //unsigned int flags;
}hal_spdma_io_param_t;


/*!
  AVDMA transfer's parameters
  */
typedef struct hal_avspdma_param
{
  /*!
    Source address
    */
  mt_u32 src_addr;
  /*!
    Destination address
    */
  mt_u32 dst_addr;
  /*!
    Transfer region height
    */
  mt_u16 height;
  /*!
    Transfer region width
    */
  mt_u16 width;

}hal_avspdma_param_t;

/*!
  comment
  */
typedef enum
{
  /*!
    comment
    */
  SPDMA_ADDR_FIX = 0,
  /*!
    comment
    */
  SPDMA_ADDR_INC
} spdma_addr_i_t;

/*!
  SPDMA channel ID
  */
typedef enum
{
  /*!
    comment
    */
  SPDMA_CHN_ID0 = 0,//PCM PP BUF0
  /*!
    comment
    */
  SPDMA_CHN_ID1,
  /*!
    comment
    */
  SPDMA_CHN_ID2,
  /*!
    comment
    */
  SPDMA_CHN_ID3,
  /*!
    comment
    */
  SPDMA_CHN_ID4,
  /*!
    comment
    */
  SPDMA_CHN_ID5,
  /*!
    comment
    */
  SPDMA_CHN_ID6,
  /*!
    comment
    */
  SPDMA_CHN_ID7,//PCM PP BUF7
  /*!
    comment
    */
  SPDMA_CHN_ID_MAX
} spdma_chn_id_t;


/*!
  DMA burst size
  */
typedef enum
{
  /*!
    comment
    */
  SPDMA_BURST_NUM1 = 0,
  /*!
    comment
    */
  SPDMA_BURST_NUM2,
  /*!
    comment
    */
  SPDMA_BURST_NUM4,
  /*!
    comment
    */
  SPDMA_BURST_NUM8,
  /*!
    comment
    */
  SPDMA_BURST_NUM16
} spdma_burst_num_t;

/*!
  DMA transfer unit size
*/
typedef enum
{
  /*!
    comment
    */
  SPDMA_USIZE_8BIT = 0,
  /*!
    comment
    */
  SPDMA_USIZE_16BIT,
  /*!
    comment
    */
  SPDMA_USIZE_32BIT,
  /*!
    comment
    */
  SPDMA_USIZE_64BIT
} spdma_usize_t;



/*!
  This structure defines the option of dma.
  */
typedef struct hal_spdma_op
{
/*!
  Comment
  */
  mt_s32 (*start)(mt_u32 *, mt_u32 *, mt_u32 *);
/*!
  Comment
  */
  mt_s32 (*check)(mt_void);
/*!
  Comment
  */
  mt_s32 (*pause)(mt_void);
/*!
  Comment
  */
  mt_s32 (*resume)(mt_void);
/*!
  Comment
  */
  mt_s32 (*stop)(mt_void);
/*!
  Comment
  */
  mt_s32 (*reset)(mt_void);
/*!
  Comment
  */
  //mt_s32 (*soft_reset)(mt_void);
/*!
  Comment
  */
  //mt_s32 (*push)(mt_s32, mt_u32);
 /*!
  Comment
  */
  //spdma_priv_t *p_priv;
  hal_spdma_notify_t dma_notify_param;
}hal_spdma_op_t;


/*!!
  secure audio/video dma channel
  */
#define SPDMA_CHANNEL_SECURE_VIDEO_0			SPDMA_CHN_ID5
#define SPDMA_CHANNEL_SECURE_AUDIO_0			SPDMA_CHN_ID6
#define SPDMA_CHANNEL_SECURE_AUDIO_1			SPDMA_CHN_ID7

/*!!
  dma channel flags
  */
#define FLAG_SPDMA_CHANNEL_SECURE_VIDEO_0		(0x01 << SPDMA_CHANNEL_SECURE_VIDEO_0)
#define FLAG_SPDMA_CHANNEL_SECURE_AUDIO_0		(0x01 << SPDMA_CHANNEL_SECURE_AUDIO_0)
#define FLAG_SPDMA_CHANNEL_SECURE_AUDIO_1		(0x01 << SPDMA_CHANNEL_SECURE_AUDIO_1)

mt_s32 mt_spdma_open(void);
void mt_spdma_close(void);
mt_s32 mt_spdma_start(hal_spdma_io_param_t *dma_param);
mt_s32 mt_spdma_stop(mt_s32 chn_id);
spdma_status_t mt_spdma_status(mt_s32 chn_id);
mt_s32 mt_spdma_request_chn_id(mt_s32 chn_id);
mt_s32 mt_spdma_release_chn_id(mt_s32 chn_id);
void *mt_spdma_test(void *to, const void *from, size_t n, spdma_cmd_type_t *p_cmd);


/*!!
  initiate the DMA

  \param[in] None

  \return SUCCESS if OK, else fail
  */
extern mt_s32 hal_spdma_init(mt_void);


/*!!
  Interface used to start a SPDMA operation

  \param[in] p_param this thansfer's operation
  \param[in] p_notify this thansfer's notify

  \return SUCCESS if OK, else fail
*/
extern mt_s32 hal_spdma_start(mt_void *param, mt_void *p_notify);


/*!!
  Interface used to check a DMA channel's status.

  \param[in] id channel id

  \return SUCCESS if OK, else fail
*/
extern mt_s32 hal_spdma_check(mt_void);

/*!!
  Interface used to pause a DMA channel's operation

  \param[in] id channel id

  \return see dma_status_t
*/
extern mt_s32 hal_spdma_pause(mt_void);

/*!!
  Interface used to resume a DMA channel's operation

  \param[in] id channel id

  \return see dma_status_t
*/
extern mt_s32 hal_spdma_resume(mt_void);

/*!!
  Interface used to stop a DMA channel's operation

  \param[in] id channel id

  \return see dma_status_t
*/
extern mt_s32 hal_spdma_stop(mt_void);

/*!!
  Interface used to conduct global reset for DMA

  \return see dma_status_t
*/
extern mt_s32 hal_spdma_reset(mt_void);

/*!!
  Interface used to conduct soft reset for specific DMA channel

  \param[in] id channel id

  \return see dma_status_t
*/
extern mt_s32 hal_spdma_soft_reset(mt_void);

/*!!
  Interface used to transfer data by DMA
  original data size was set as unknown, confirmed value is set midway

  \param[in] id channel id
  \param[in] length confimed data size

  \return see dma_status_t
*/
extern mt_s32 hal_spdma_push(mt_s32 id, mt_u32 length);

#endif //__HAL_DMA_H__
