/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HAL_DMA_H__
#define __HAL_DMA_H__

/*!!
  maximal dma channel's number
  */
#define HAL_DMA_MAX_CHAN_NUM 8


/*!
  DMA command
*/
#define DMA_IOC_MAGIC     'd'

#define DMA_IOC_START                _IOWR(DMA_IOC_MAGIC, 0x1, hal_dma_io_param_t)
#define DMA_IOC_STOP                 _IOW(DMA_IOC_MAGIC, 0x2, int)
#define DMA_IOC_CLOSE                _IO(DMA_IOC_MAGIC, 0x3)
#define DMA_IOC_RESUME               _IO(DMA_IOC_MAGIC, 0x4)
#define DMA_IOC_CHECK                _IOW(DMA_IOC_MAGIC, 0x5, int)
#define DMA_IOC_PAUSE                _IO(DMA_IOC_MAGIC, 0x6)
#define DMA_IOC_RESET                _IO(DMA_IOC_MAGIC, 0x7)
#define DMA_IOC_SOFT_RESET           _IO(DMA_IOC_MAGIC, 0x8)
#define DMA_IOC_PUSH                 _IO(DMA_IOC_MAGIC, 0x9)
#define DMA_IOC_CAPACITY_GET         _IO(DMA_IOC_MAGIC, 0xA)
#define DMA_IOC_MEMCPY_TESTCASE      _IO(DMA_IOC_MAGIC, 0xB)
#define DMA_IOC_MEMSET_LLN_TESTCASE   _IOWR(DMA_IOC_MAGIC, 0xC,hal_dma_io_param_t)
#define DMA_IOC_MUTI_CHAIN_TESTCASE  _IO(DMA_IOC_MAGIC, 0xD)
#define DMA_IOC_PERFORMANCE_TESTCASE _IO(DMA_IOC_MAGIC, 0xE)
#define DMA_IOC_OVER_MEM_TESTCASE    _IO(DMA_IOC_MAGIC, 0xF)
#define DMA_IOC_MEMSET_TESTCASE   _IOWR(DMA_IOC_MAGIC, 0x10,hal_dma_capacity_t)

#define DMA_IOC_START_WITH_CH     _IOWR(DMA_IOC_MAGIC, 0x11, hal_dma_io_param_t)
#define DMA_IOC_ACTIVE_CH     _IOWR(DMA_IOC_MAGIC, 0x12, int)
#define DMA_IOC_DEACTIVE_CH     _IOWR(DMA_IOC_MAGIC, 0x13, int)
#define DMA_IOC_ADDR_TRANSLATION	_IOWR(DMA_IOC_MAGIC, 0x15,dma_addr_priv_t)

#define DMA_IOC_MAXNR 32

/*!
  Success return
  */
#define DMA_SUCCESS ((mt_s32)0)
/*!
  Fail for common reason
  */
#define DMA_ERR_FAILURE ((mt_s32)-1)
/*!
  Fail for waiting timeout
  */
#define DMA_ERR_TIMEOUT ((mt_s32)-2)
/*!
  Fail for function param invalid
  */
#define DMA_ERR_PARAM ((mt_s32)-3)
/*!
  Fail for module status invalid
  */
#define DMA_ERR_STATUS ((mt_s32)-4)
/*!
  Fail for module busy
  */
#define DMA_ERR_BUSY ((mt_s32)-5)
/*!
  Fail for no enough memory
  */
#define DMA_ERR_NO_MEM ((mt_s32)-6)
/*!
  Fail for no enough resource
  */
#define DMA_ERR_NO_RSRC ((mt_s32)-7)
/*!
  Fail for hardware error
  */
#define DMA_ERR_HARDWARE ((mt_s32)-8)
/*!
  Fail for feature not support
  */
#define DMA_ERR_NOFEATURE ((mt_s32)-9)

/*!!
  DMA transfer type
  */
typedef enum
{
  /*!!
    The type of memory to memory
    */
  DMA_TYPE_MEM2MEM = 0x01,
  /*!!
    The type of memory to port
    */
  DMA_TYPE_MEM2PORT = 0x02,
  /*!!
    The type of port to memory
    */
  DMA_TYPE_PORT2MEM = 0x04,
  /*!!
    The type of memory to memory
    */
  DMA_TYPE_PORT2PORT = 0x08
}hal_dma_type_t;

/*!!
  DMA transfer unit
  */
typedef enum
{
  /*!!
    DMA transfer in unit of byte
    */
  DMA_TRANS_UNIT_BYTE = 0x01,
  /*!!
    DMA transfer in unit of half word
    */
  DMA_TRANS_UNIT_HALF = 0x02,
  /*!!
    DMA transfer in unit of word
    */
  DMA_TRANS_UNIT_WORD = 0x04,
  /*!!
    DMA transfer in unit of double-word
    */
  DMA_TRANS_UNIT_DWORD = 0x08
}hal_trans_unit_t;

/*!
  AVDMA status
  */
typedef enum
{
  /*!
    The AVDMA is in stop status
    */
  AVDMA_STATUS_STOP = 0,
  /*!
    The AVDMA is in running status
    */
  AVDMA_STATUS_RUNNING,
  /*!
    driver has not been inited
    */
  AVDMA_STATUS_LAST

}avdma_status_t;

/*!!
  DMA status used by HAL_DMA_XXXX_XX_CHECK() as return value
  */
typedef enum
{
  /*!!
    The DMA is in stop status
    */
  DMA_STATUS_STOP = 0,
  /*!!
    The DMA is in running status
    */
  DMA_STATUS_RUNNING,
  /*!!
    The DMA is in pause status
    */
  DMA_STATUS_PAUSE,
  /*!!
    The DMA is finish a transmit status
    */
  DMA_STATUS_DONE,
  /*!!
    driver has not been inited
    */
  DMA_STATUS_LAST = DMA_STATUS_DONE
}dma_status_t;

/*!!
  DMA event
  */
typedef enum
{
  /*!!
    driver has not been inited
    */
  DMA_EVENT_NOT_INIT = 0x01,
  /*!!
    a timeout occurred
    */
  DMA_EVENT_TIMEOUT = 0x02,
  /*!!
    DMA transfer error happend
    */
  DMA_EVENT_ERROR = 0x04,
  /*!!
    source interval buffer transfer complete
    */
  DMA_EVENT_SRC_INTERVAL_INTERRUPT = 0x80,
  /*!!
    destination interval buffer transfer complete
    */
  DMA_EVENT_DST_INTERVAL_INTERRUPT = 0x10,
  /*!!
    DMA transfer has been completed
    */
  DMA_EVENT_COMPLETE = 0x20,
  /*!!
    driver has not been inited
    */
  DMA_EVENT_LAST = DMA_EVENT_COMPLETE,
  /*!!
    DMA event mask
    */
  DMA_EVENT_MASK = 0x3F
}dma_event_t;

/*!!
  DMA mode used by hal_dma_1d_start() and struct hal_dma_2d_param
  */
typedef enum
{
  /*!!
    The dma 2d mode, else is 1d
    */
  DMA_MODE_2D = 0x0400,
  /*!!
    ALU AND operation:   src & alu_data --> dst
    */
  DMA_MODE_ALU_AND = 0x010000,
  /*!!
    ALU OR operation:    src | alu_data --> dst
    */
  DMA_MODE_ALU_OR = 0x020000,
  /*!!
    ALU XOR operation:   src ^ alu_data --> dst
    */
  DMA_MODE_ALU_XOR = 0x040000,
  /*!!
    ALU NOT operation:   ~src --> dst
    */
  DMA_MODE_ALU_NOT = 0x080000,
  /*!!
    ALU FILL the user dword, used for the operation like memset
    */
  DMA_MODE_ALU_FILL = 0x100000,
  /*!!
    ALU mask
    */
  DMA_MODE_ALU_MASK = 0x1F0000,
}dma_mode_t;

/*!!
  DMA channel configuration param
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
} dma_chn_config_t;


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
} dma_chn_control_t;


/*!
  DMA plus transfer's parameters
  */
typedef struct hal_dma_param
{
 /*!!
    DMA mode, see dma_mode_t
    */
  mt_u32 mode;
  /*!!
    Channel read / write priority
    */
  mt_u32 prio;
  /*!!
    Total data length in byte
    */
  mt_u32 len;
  /*!!
    filled data if use alu fill mode
    */
  mt_u32 alu_fill_data;
  /*!!
    filled data width in byte, see hal_trans_unit_t
    */
  mt_u32 alu_fill_width;
   /*!!
    Virtual Source address
    */
  void *vir_src_addr;
  /*!!
    Virtual Destin address
    */
  void *vir_dst_addr;
  /*!!
    Source address
    */
  phys_addr_t phy_src_addr;
  /*!!
    Destin address
    */
  phys_addr_t phy_dst_addr;
  /*!!
    Source region width in byte, used only in 2d mode
    */
  mt_u32 src_width;
  /*!!
    Destin region width in byte, used only in 2d mode
    */
  mt_u32 dst_width;
  /*!!
    Source region leap size in byte, used only in 2d mode
    */
  mt_u32 src_leap;
  /*!!
    Destin region leap size in byte, used only in 2d mode
    */
  mt_u32 dst_leap;
  /*!!
    DMA channel configuration param
  */
  dma_chn_config_t config;
  /*!!
    DMA channel control param
  */
  dma_chn_control_t control;
  /*!
      next node
    */
  struct hal_dma_param *p_next;
}hal_dma_param_t;



/*!!
  DMA channel's capacity
  */
typedef struct
{
  /*!
    DMA transfer unit size, see hal_trans_unit_t
    */
  mt_u8 trans_unit;
  /*!
    DMA transfer type, see hal_dma_type_t
    */
  mt_u8 type;
  /*!
    DMA support 2d if 1, else not support 2d
    */
  mt_u8 support_2d : 1;
  /*!
    DMA support link transfer if 1, else not support
    */
  mt_u8 support_link : 1;
  /*!
    DMA support alu transfer if 1, else not support
    */
  mt_u8 support_alu : 1;
  /*!
    DMA support big endian mode if 1, else not support
    */
  mt_u8 support_be : 1;
  /*!
    DMA address align double-word mode
    */
  mt_u8 addr_align_dword : 1;
  /*!
    DMA address align word mode
    */
  mt_u8 addr_align_word : 1;
  /*!
    DMA address align half-word mode
    */
  mt_u8 addr_align_half : 1;
  /*!
    DMA address align byte mode
    */
  mt_u8 addr_align_byte : 1;
  /*!
    DMA max transfer size in one request
    */
  mt_u32 max_size_one_req;
  /*!
    DMA max transfer size in width of one line, only used for 2D
    */
  mt_u32 max_size_one_width;
  /*!
    DMA max line number in height, only used for 2D
    */
  mt_u32 max_size_one_height;
  /*!
    DMA max transfer size of leap, only used for 2D
    */
  mt_u32 max_size_one_leap;
}hal_dma_chan_cap_t;

/*!!
  DMA's capacity
  */
typedef struct
{
  /*!!
    DMA channel's number
    */
  mt_u8 chan_num;
  /*!!
    DMA channel's capacity
    */
  hal_dma_chan_cap_t chan_cap[HAL_DMA_MAX_CHAN_NUM];
}hal_dma_capacity_t;

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
}dma_chn_lln_t;

/*!
  DMA linked-list-node use 64bit
  */
typedef struct dma_64bit_addr
{
  /*!
    Next node address
    */
  ulong next_64bit_addr;

  dma_chn_lln_t *dma_chn_lln;

}dma_64bit_addr_t;


/*!
  DMA linked-list-node use 64bit
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
}dma_chn_lln_64bit_t;


/*!!
  DMA notify callback function
  */
typedef mt_void (*HAL_DMA_NOTIFY) (
    /*!!
    The channel's id
    */
    mt_s32 id,
    /*!!
    The event mask
    */
    dma_event_t event,
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
  HAL_DMA_NOTIFY fn_cb;
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
}hal_dma_notify_t;
/*!!
  aes mode
  */
typedef enum aes_mode_sel
{
  /*!!
    ecb
    */
  AES_MODE_ECB = 0,
  /*!!
    cbc : not support
    */
  AES_MODE_CBC
}aes_mode_sel_t;
/*!!
  aes key select
  */
typedef enum aes_key_sel
{
  /*!!
    default use
    */
  AES_KEY_SEL_128 = 0,
  /*!!
    not support
    */
  AES_KEY_SEL_192,
  /*!!
    not support
    */
  AES_KEY_SEL_256,
  /*!!
    not support
    */
  AES_KEY_SEL_FORBID
}aes_key_sel_t;
/*!!
  enable
  */
typedef enum enable_sel
{
  /*!!
   disable aes or des
    */
  DISABLE = 0,
  /*!!
    enable aes or des
    */
  ENABLE
}enable_sel_t;
/*!!
  decrpt select
  */
typedef enum decrpt_enable
{
  /*!!
    disbale decrpt
    */
  DECRPT_ENABLE = 0,
  /*!!
    use decrpt
    */
  ENCRPT_ENABLE
}decrpt_enable_t;
/*!!
  des core select
  */
typedef enum des_core_sel
{
  /*!!
    des
    */
  ONE_DES_64 = 0,
  /*!!
    3 des : not support
    */
  THREE_DES_64
}des_core_sel_t;



typedef struct
{
  mt_s32 chn_id;
  hal_dma_param_t param;
  hal_dma_notify_t *p_notify;
  unsigned int flags;
}hal_dma_io_param_t;

/*!!
  aes des config
  */
typedef struct dma_aes_des_cfg
{
  /*!!
   enable
    */
  mt_u8 aes_enable : 1;
  /*!!
    mode
    */
  mt_u8 aes_mode : 1;
  /*!!
    XX
    */
  mt_u8 aes_key : 2;
  /*!!
     XX
     */
  mt_u8 aes_key_decrypt : 1;
  /*!!
     XX
     */
  mt_u8 des_enable : 1;
  /*!!
     XX
     */
  mt_u8 des_core : 1;
  /*!!
     XX
     */
  mt_u8 des_decrpt : 1;
  /*!!
     XX
     */
  mt_u8 aes_status ; //reserved
  /*!!
     XX
     */
  mt_u32 aes_des_key[8];
  /*!!
     XX
     */
  mt_u32 aes_vector[4];

}dma_aes_des_cfg_t;

/*!
  AVDMA transfer's parameters
  */
typedef struct hal_avdma_param
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

}hal_avdma_param_t;

/*!
  comment
  */
typedef enum
{
  /*!
    comment
    */
  DMA_ADDR_FIX = 0,
  /*!
    comment
    */
  DMA_ADDR_INC
} dma_addr_i_t;

/*!
  DMA channel ID
  */
typedef enum
{
  /*!
    comment
    */
  DMA_CHN_ID0 = 0,
  /*!
    comment
    */
  DMA_CHN_ID1,
  /*!
    comment
    */
  DMA_CHN_ID2,
  /*!
    comment
    */
  DMA_CHN_ID3,
  /*!
    comment
    */
  DMA_CHN_ID4,
  /*!
    comment
    */
  DMA_CHN_ID5,
  /*!
    comment
    */
  DMA_CHN_ID6,
  /*!
    comment
    */
  DMA_CHN_ID7,
  /*!
    comment
    */
  DMA_CHN_ID_MAX
} dma_chn_id_t;

/*!
  comment
  */
typedef enum
{
  /*!
    comment
    */
  ALU_MODE_NIL = 0,
  /*!
    comment
    */
  ALU_MODE_AND,
  /*!
    comment
    */
  ALU_MODE_OR,
  /*!
    comment
    */
  ALU_MODE_XOR,
  /*!
    comment
    */
  ALU_MODE_NOT,
  /*!
    comment
    */
  ALU_MODE_FILL
}alumode_t;

/*!
  DMA burst size
  */
typedef enum
{
  /*!
    comment
    */
  DMA_BURST_NUM1 = 0,
  /*!
    comment
    */
  DMA_BURST_NUM2,
  /*!
    comment
    */
  DMA_BURST_NUM4,
  /*!
    comment
    */
  DMA_BURST_NUM8,
  /*!
    comment
    */
  DMA_BURST_NUM16
} dma_burst_num_t;

/*!
  DMA transfer unit size
*/
typedef enum
{
  /*!
    comment
    */
  DMA_USIZE_8BIT = 0,
  /*!
    comment
    */
  DMA_USIZE_16BIT,
  /*!
    comment
    */
  DMA_USIZE_32BIT,
  /*!
    comment
    */
  DMA_USIZE_64BIT
} dma_usize_t;

/*!
  DMA  addr struct
  */
typedef struct
{
  /*!
	dma src phys_addr in cpu
	 */
  phys_addr_t cpu_src;
  /*!
	dma dst phys_addr in cpu
	 */
  phys_addr_t cpu_dst;
  /*!
	dma src phys_addr
	 */
  phys_addr_t dma_src_addr;
  /*!
	dma dst phys_addr
	 */
  phys_addr_t dma_dst_addr;
  /*!
	dma src change flag
	 */
  int src_flag;
  /*!
	dma dst change flag
	 */
  int dst_flag;
}dma_addr_priv_t;

/*!
  DMA  data
  */
typedef struct
{
  /*!
    DMA's capacity
    */
  hal_dma_capacity_t capacity;
  /*!
    DMA's notify_param
    */
  hal_dma_notify_t dma_notify_param[DMA_CHN_ID_MAX];
  /*!
    DMA's virtual p_lln_head addr
    */
  dma_chn_lln_t *p_lln_head_vir[DMA_CHN_ID_MAX];
  /*!
    DMA's phy p_lln_head addr
    */
  phys_addr_t p_lln_head_phy[DMA_CHN_ID_MAX];
  /*!
    DMA's lln_len
    */
  mt_u8 lln_len[DMA_CHN_ID_MAX];
  /*!
    DMA's virtual p_alu_src addr
    */
  void *p_alu_src_vir;
  /*!
    DMA's phy p_alu_src addr
    */
  phys_addr_t p_alu_src_phy;
}dma_priv_t;
/*!
  This structure defines the option of dma.
  */
typedef struct hal_dma_op
{
/*!
  Comment
  */
  mt_s32 (*capacity_get)(mt_u32 *, mt_u32 *);
/*!
  Comment
  */
  mt_s32 (*start)(mt_s32, mt_u32 *, mt_u32 *, mt_u32 *);
/*!
  Comment
  */
  mt_s32 (*check)(mt_s32);
/*!
  Comment
  */
  mt_s32 (*pause)(mt_s32);
/*!
  Comment
  */
  mt_s32 (*resume)(mt_s32);
/*!
  Comment
  */
  mt_s32 (*stop)(mt_s32);
/*!
  Comment
  */
  mt_s32 (*reset)(mt_void);
/*!
  Comment
  */
  mt_s32 (*soft_reset)(mt_s32);
/*!
  Comment
  */
  mt_s32 (*push)(mt_s32, mt_u32);
 /*!
  Comment
  */
  dma_priv_t *p_priv;
}hal_dma_op_t;


/*!!
  secure audio/video dma channel
  */
#define DMA_CHANNEL_SECURE_VIDEO_VC1		DMA_CHN_ID4
#define DMA_CHANNEL_SECURE_VIDEO_0			DMA_CHN_ID5
#define DMA_CHANNEL_SECURE_AUDIO_0			DMA_CHN_ID6
#define DMA_CHANNEL_SECURE_AUDIO_1			DMA_CHN_ID7

/*!!
  dma channel flags
  */
#define FLAG_DMA_CHANNEL_SECURE_VIDEO_0		(0x01 << DMA_CHANNEL_SECURE_VIDEO_0)
#define FLAG_DMA_CHANNEL_SECURE_AUDIO_0		(0x01 << DMA_CHANNEL_SECURE_AUDIO_0)
#define FLAG_DMA_CHANNEL_SECURE_AUDIO_1		(0x01 << DMA_CHANNEL_SECURE_AUDIO_1)

mt_s32 mt_dma_open(void);
void mt_dma_close(void);
mt_s32 mt_dma_start(hal_dma_io_param_t *dma_param);
mt_s32 mt_dma_stop(mt_s32 chn_id);
dma_status_t mt_dma_status(mt_s32 chn_id);
mt_s32 mt_dma_request_chn_id(mt_s32 chn_id);
mt_s32 mt_dma_release_chn_id(mt_s32 chn_id);
phys_addr_t mt_dma_memcpy(phys_addr_t to, phys_addr_t from, size_t n, unsigned int flags);
phys_addr_t mt_dma_memset(phys_addr_t to, unsigned int val, size_t n,unsigned char width);
phys_addr_t mt_dma_memcpy_ext(mt_u8 ch, phys_addr_t to, phys_addr_t from, size_t n, unsigned int flags);

/*!!
  initiate the DMA

  \param[in] None

  \return SUCCESS if OK, else fail
  */
extern mt_s32 hal_dma_init(mt_void);

/*!!
  Get DMA's capacity

  \param[in] p_capacity the capacity of DMA

  \return SUCCESS if OK, else fail
  */
extern mt_s32 hal_dma_capacity_get(hal_dma_capacity_t *p_capacity);

/*!!
  Get Free DMA Channel

  \param[in] ch_flags channel flags

  \return valid channel num if OK, else return DMA_CHN_ID_MAX
  */
mt_s32 hal_dma_get_free_channel(unsigned int ch_flags);

/*!!
  Interface used to start a DMA channel's operation

  \param[in] p_param this thansfer's operation
  \param[in] p_notify this thansfer's notify

  \return SUCCESS if OK, else fail
*/
extern mt_s32 hal_dma_start(mt_s32 id, mt_void *param, mt_void *p_notify);



/*!!
  Interface used to check a DMA channel's status.

  \param[in] id channel id

  \return SUCCESS if OK, else fail
*/
extern mt_s32 hal_dma_check(mt_s32 id);

/*!!
  Interface used to pause a DMA channel's operation

  \param[in] id channel id

  \return see dma_status_t
*/
extern mt_s32 hal_dma_pause(mt_s32 id);

/*!!
  Interface used to resume a DMA channel's operation

  \param[in] id channel id

  \return see dma_status_t
*/
extern mt_s32 hal_dma_resume(mt_s32 id);

/*!!
  Interface used to stop a DMA channel's operation

  \param[in] id channel id

  \return see dma_status_t
*/
extern mt_s32 hal_dma_stop(mt_s32 id);

/*!!
  Interface used to conduct global reset for DMA

  \return see dma_status_t
*/
extern mt_s32 hal_dma_reset(mt_void);

/*!!
  Interface used to conduct soft reset for specific DMA channel

  \param[in] id channel id

  \return see dma_status_t
*/
extern mt_s32 hal_dma_soft_reset(mt_s32 id);

/*!!
  Interface used to transfer data by DMA
  original data size was set as unknown, confirmed value is set midway

  \param[in] id channel id
  \param[in] length confimed data size

  \return see dma_status_t
*/
extern mt_s32 hal_dma_push(mt_s32 id, mt_u32 length);
#if defined(CONFIG_MT_CHIP_ARIA)
/*!!
  Interface used to start a link list DMA operation

  \param[in] id channel id
  \param[in] mode transfer mode
  \param[in] link link descriptor

  \return SUCCESS if OK, else fail
*/
extern mt_s32 hal_dma_link_start(mt_s32 id, mt_u32 mode, mt_void *p_link);

// added by Bob
extern mt_void *hal_dma_memcpy(mt_void *dest, mt_void *src, mt_u32 n);

// added by Bob
extern mt_void *hal_dma_memcpy_cbc(mt_void *dest, mt_void *src, mt_u32 n, mt_u8 mode);

/*!!
  Interface used to start a link list DMA operation

  \param[in] id channel id
  \param[in] en 1: pause, 0: resume

  \return SUCCESS if OK, else fail
*/
extern mt_s32 hal_dma_link_pause(mt_s32 id, mt_s32 en);

/*!!
  Interface used to stop a link list DMA operation

  \param[in] id channel id

  \return SUCCESS if OK, else fail
*/
extern mt_s32 hal_dma_link_stop (mt_s32 id);

/*!!
  Interface used to check a link list DMA status

  \param[in] id channel id
  \param[in] link_item one item of a link list

  \return SUCCESS if OK, else fail
*/
extern mt_s32 hal_dma_link_check(mt_s32 id, mt_void *p_link_item);


/*!!
  Interface used to check a link list DMA status

  \param[in] id channel id
  \param[in] link_item one item of a link list

  \return SUCCESS if OK, else fail
*/
mt_s32 hal_dma_aes_des_config(mt_u32 *param);

/*!!
  Interface used to check a link list DMA status

  \param[in] id channel id
  \param[in] link_item one item of a link list

  \return SUCCESS if OK, else fail
*/
mt_void hal_dma_aes_enable(mt_s32 set);

/*!!
  Interface used to check a link list DMA status

  \param[in] id channel id
  \param[in] link_item one item of a link list

  \return SUCCESS if OK, else fail
*/
mt_void hal_dma_des_enable(mt_s32 set);

/*!!
  Coments
*/
mt_s32 hal_avdma_start(hal_avdma_param_t *param);

/*!!
  Coments
*/
mt_s32 hal_avdma_check(mt_void);
#endif
#endif //__HAL_DMA_H__
