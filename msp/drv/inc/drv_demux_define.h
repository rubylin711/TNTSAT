/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_DEMUX_DEFINE_H__
#define __DRV_DEMUX_DEFINE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <linux/list.h>
#include <linux/wait.h>

#include "mt_type.h"

#include "drv_demux_config.h"
#include "drv_demux_ext.h"

#include "hal_demux_regs.h"

#include "mt_drv_mmz.h"
#include "mt_drv_proc.h"

#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_mpi_demux.h"

#define MT_DEMUX_PROC_SUPPORT    1


#define DMX_FQ_COM_BLKSIZE              (8 * 1024)
#define DMX_PES_MAX_SIZE                  ((64 * 1024) / DMX_FQ_COM_BLKSIZE)

#define DMX_MAX_FILTER_NUM_PER_CHANNEL  32      //the FILTER number in per channel

#define DMX_REGION_FILTER_COUNT         32
#define DMX_REGION_CHANNEL_COUNT      32

#define DMX_PER_DESC_LEN                       0x10
#define DMX_MAX_ERRLIST_NUM                0x100

#define DMX_CHANNEL_REGION_NUM              3
#define DMX_CHANNEL_NUM_PER_REGION      32
#define DMX_CHANNEL_NUM_PER_RECORD      32

#define DMX_FQ_REGION_NUM                        2
#define DMX_FQ_NUM_PER_REGION                32

#define DMX_OQ_REGION_NUM                        4
#define DMX_OQ_NUM_PER_REGION                32

#define DMX_OQ_RSV_OFFSET   0
#define DMX_OQ_CTRL_OFFSET  1
#define DMX_OQ_EOPWR_OFFSET 2
#define DMX_OQ_SZUS_OFFSET  3
#define DMX_OQ_SADDR_OFFSET 4
#define DMX_OQ_RDWR_OFFSET  5
#define DMX_OQ_CFG_OFFSET   6
#define DMX_OQ_START_OFFSET 7

#define DMX_FQ_CTRL_OFFSET  0
#define DMX_FQ_RDVD_OFFSET  1
#define DMX_FQ_SZWR_OFFSET  2
#define DMX_FQ_START_OFFSET 3
#define DMX_FQ_ALOVF_CNT    4

#define DMX_FQ_COMMOM           0x0
#define DMX_FQ_AV_BASE          8
#define DMX_FQ_REC_BASE         2
#define DMX_FQ_SCD_BASE         5
#define DMX_OQ_DEPTH                0x3ff
#define DMX_OQ_ALOVF_CNT    0x4//almost overflow level
#define DMX_OQ_OUTINT_CNT   0x1//OQ queue continues output buf block to this number will cuase break
#define DMX_OQ_START_CNT    0x30//OQ queue continues output minimal free block count
#define DMX_OQ_STOP_PERSENT  80 //channel pid disable persent,if oq use dscs more the 80% driver will stop it
#define DMX_OQ_START_PERSENT 60//channel pid enable persent,if oq use dscs less the 60% driver will restart it
#define DMX_OQ_SOFT_ALOVF   (DMX_OQ_ALOVF_CNT*2)//software almost overflow level
#define DMX_MAX_AVFQ_DESC   0x3f0
#define DMX_MAX_BLOCK_SIZE  0xffff


#define DMX_SHIFT_16BIT     16
#define DMX_SHIFT_8BIT      8
#define DMX_BUF_INTERVAL    0x0

#define DMX_INVALID_PORT_ID             0xffffffff

#define DMX_DEFAULT_TAG_LENGTH      (4)

#define DMX_INVALID_DEMUX_ID            0xffffffff
#define DMX_INVALID_SYNC_HANDLE         0xffffffff
#define DMX_INVALID_CHAN_ID             0xffff
#define DMX_INVALID_FILTER_ID           0xffff
#define DMX_INVALID_KEY_ID              0xffff
#define DMX_INVALID_PID                 0x1fff
#define DMX_INVALID_BUF_ID              0xffff
#define DMX_INVALID_FQ_ID               0xffff

#define DMX_MIN_TS_BUFFER_SIZE          0x1000
#define DMX_MAX_TS_BUFFER_SIZE          0x1000000
#define DMX_TS_BUFFER_GAP               0x100

#define DMX_DISABLE                     0x0
#define DMX_ENABLE                      0x1

#define DMX_MAX_SEC_LEN                 0x1000

#define SECTION_LENGTH_FIELD_SIZE       2

#define DMX_MAX_IP_DESC_DEPTH           0xffff
#define DMX_MIN_IP_DESC_DEPTH           0x3ff
#define DMX_MAX_IP_BLOCK_SIZE           0xffff

#define DMX_MAX_LOST_TH                 0x3
#define DMX_MAX_LOCK_TH                 0x7
#define DMX_MAX_FLUSH_WAIT              0x1000

#define DMX_RAM_PORT_MIN_LEN            188
#define DMX_RAM_PORT_MAX_LEN            255

#define DMX_TS_PACKET_LEN               188
#define DMX_TS_PACKET_LEN_204           204
#define DMX_SCD_PACKET_LEN              28
#define DMX_TOTAL_SCD_FLTNUM            10
#define DMX_TOTAL_RANGE_FLTNUM          7
#define DMX_TOTAL_RANGE_FLTNUM_V200     16

#define DMX_PORT_OFFSET                 1
#define DMX_DEFAULT_INT_CNT             1


#define DMX_TAG_PORT_OFFSET 0x20

#define DMX_PES_HEADER_LENGTH           9
#define DMX_PES_HEADER_AND_PTS_LENGTH   14

#define DMX_DEFAULT_POST_TH             0

/* speed is 15 ,it means the Max speed of a IP port equal :
100 Mbps@MV300
126 Mbps@CV200
We consider this speed is enough for IP port,
user can change this value small to get more speed of IP port,
but that may cause the total speed of demux be stress
*/
#define DMX_DEFAULT_IP_SPEED              15


#define DMX_KEY_HARDONLY_FLAG           0xffffffff

#define DMX_CHECKCHN_TIMEOUT        1000    /* 1s */
#define DMX_RESETCHN_TIME1          2000    /* 2s */
#define DMX_RESETCHN_TIME2          5000    /* 2s */
#define DMX_CHECK_START_PERSENT     80      /* 80% */
#define DMX_CHN_RESET_PERSENT       20      /* 20% */

#define INVALID_PTS                 0xFFFFFFFFL

#define DMX_CSA2_TABLE_ID           0
#define DMX_SPE_TABLE_ID            1
#define DMX_OTHER_TABLE_ID          2

#define DMX_KEY_MIN_LEN             8
#define DMX_KEY_MAX_LEN            32//24   //16

#define DMX_PES_NODE_NUM           (200)

#define CHECKTAGPORTID(TagPortId) ({                      \
    MT_BOOL bRet = MT_FALSE;                                    \
    if (TagPortId != DMX_INVALID_PORT_ID                  \
        && TagPortId < DMX_TAG_MAX_TS_WAY)    \
    {                                                                        \
        bRet = MT_TRUE;                                               \
    }                                                                        \
    bRet;                                                                  \
})

#define CHECKPOINTER(ptr)                                   \
    do                                                      \
    {                                                       \
        if (!(ptr))                                         \
        {                                                   \
            MT_ERR_DEMUX("pointer is null\n");             \
            return MT_ERR_DMX_NULL_PTR;                     \
        }                                                   \
    } while (0)

#define CHECKDMXID(DmxId)                                   \
    do                                                      \
    {                                                       \
        if ((DmxId) >= DMX_CNT)                             \
        {                                                   \
            MT_ERR_DEMUX("invalid demux %d\n", DmxId);     \
            return MT_ERR_DMX_INVALID_PARA;                 \
        }                                                   \
    } while (0)
    
#define CHECKTSOPORTID(Id)                                \
    do                                                      \
    {                                                       \
        if ((Id) >= DMX_TSOPORT_CNT)                      \
        {                                                   \
            MT_ERR_DEMUX("invalid TSO port %u\n", Id);   \
            return MT_ERR_DMX_INVALID_PARA;                 \
        }                                                   \
    } while (0)

#define CHECKTUNERPORTID(Id)                                \
    do                                                      \
    {                                                       \
        if ((Id) >= DMX_TUNERPORT_CNT)                      \
        {                                                   \
            MT_ERR_DEMUX("invalid tuner port %u\n", Id);   \
            return MT_ERR_DMX_INVALID_PARA;                 \
        }                                                   \
    } while (0)

#define CHECKRAMPORTID(Id)                                  \
    do                                                      \
    {                                                       \
        if ((Id) >= DMX_RAMPORT_CNT)                        \
        {                                                   \
            MT_ERR_DEMUX("invalid ram port %u\n", Id);     \
            return MT_ERR_DMX_INVALID_PARA;                 \
        }                                                   \
    } while (0)

#define DMXINC(a, size) \
    if ((++a) >= (size)) { \
        (a) = 0;\
    } \

#define CHECKVALIDEPID(pid) ({                      \
    MT_BOOL bRet = MT_FALSE;                                    \
    if (((pid) > 0) && ((pid) <  DMX_INVALID_PID))                 \
    {                                                                        \
        bRet = MT_TRUE;                                               \
    }                                                                        \
    bRet;                                                                  \
})
    

typedef enum mtDMX_Ch_AFMode_E
{
    DMX_AF_SEND                   = 0,                         /*send AF data*/
    DMX_AF_DISCARD                = 1,                         /*discard AF data*/
    DMX_UNMEANING_AF_DISCARD      = 2                          /*send meaningful AF data only*/
}DMX_Ch_AFMode_E;

typedef enum mtDMX_Ch_ATTR_E
{
    DMX_CH_GENERAL = 0,
    DMX_CH_GENERAL_NOPL,
    DMX_CH_PES_NOCHECK = 2,
    DMX_CH_PES = 3,
    DMX_CH_AUDIO = 6,
    DMX_CH_VIDEO,
    DMX_CH_UNDEF
}DMX_Ch_ATTR_E;

typedef enum
{
    DMX_FLUSH_TYPE_REC_PLAY = 0x0,
    DMX_FLUSH_TYPE_PLAY     = 0x1,
    DMX_FLUSH_TYPE_REC      = 0x2,
    DMX_FLUSH_TYPE_UNDEF    = 0x3
} DMX_FLUSH_TYPE_E;

typedef enum
{
    DMX_OQ_CLEAR_TYPE_PLAY  = 0,
    DMX_OQ_CLEAR_TYPE_REC   = 1,
    DMX_OQ_CLEAR_TYPE_SCD   = 2,
    DMX_OQ_CLEAR_TYPE_UNDEF
} DMX_OQ_CLEAR_TYPE_E;

typedef enum
{
    DMX_REC_STATUS_STOP,
    DMX_REC_STATUS_START
} DMX_REC_STATUS_E;

typedef enum
{
    DMX_OQ_MODE_UNUSED,
    DMX_OQ_MODE_PLAY,
    DMX_OQ_MODE_REC,
    DMX_OQ_MODE_SCD,
    DMX_OQ_MODE_UNDEF
} DMX_OQ_MODE_E;

typedef enum
{
    DMX_ISUSED_FREE,
    DMX_ISUSED_BUSY
}DMX_BUFF_STATUS_E;


/****************************************************************************/

typedef struct
{
    MT_UNF_DMX_PORT_TYPE_E  PortType;
    mt_u32                  SyncLockTh;
    mt_u32                  SyncLostTh;

    /*
    ** whether Tuner input clock inverting or not.
    ** 0: in-phase(default)
    ** 1: inverting
    */
    mt_u32  TunerInClk;
    /* port-line sequence select:
    ** parallel:
    **    0: mean cdata[7] is the significant bit(default)
    **    1: mean cdata[0] is the significant bit
    ** serial:
    **    0: mean cdata[0] is the data line (default)
    **    1: mean cdata[7] is the data line
    */
    mt_u32  BitSelector;
    MT_BOOL bAttachWithTSO;
    MT_UNF_DMX_TSO_PORT_E AttachedTSO;
    /*this tsi should back presure some ram port ,default is 0,means invalid (ram port >=128),
    in situation as : ram128------>tso---->tsi---->demux*/
    MT_UNF_DMX_PORT_E BPRamPort; 
    
} DMX_TunerPort_Info_S;

typedef struct
{
    mt_u32 u32UsedFlag;
    DMX_UserMsg_S psMsg;
} DMX_ERRMSG_S;

typedef struct
{
#ifdef DMX_REGION_SUPPORT
    mt_u32      DmxId;
#endif
    mt_u32      ChanId;
    mt_u32      FilterId;
    mt_u32      FilterBuffId[4];
    mt_u32      Depth;
    mt_u32      AttachFlag;
    mt_u8       Match[DMX_FILTER_MAX_DEPTH];
    mt_u8       Mask[DMX_FILTER_MAX_DEPTH];
    mt_u8       Negate[DMX_FILTER_MAX_DEPTH];
    reg_filtern_config_t config_reg;
} DMX_FilterInfo_S;

typedef struct
{
    mt_u32 entry_valid;
    mt_u32 odd_key_slot_index;
    mt_u32 even_key_slot_index;
} DMX_Keyslot_S;

typedef struct {
    /*!
     ds_odd_push enable
     */
    mt_u8 ds_odd_push_en;
    /*!
     ds_even_push enable
     */
    mt_u8 ds_even_push_en;
    /*!
     ds_clear enable
     */
    mt_u8 ds_clr_en;
    /*!
     srctag_clr 
     */
    mt_u8 ds_srctag_clr;	
    /*!
     ds_cw channel number
     */	
    mt_u8 ds_cw_ch;
    /*!
     ds_mode:ts pes auto package
     */
     mt_u8 ds_mode;
    /*!
     ds_core: 0:csa 1:aes or des
     */
     mt_u8 ds_core;	
    /*!
     ivecal_mode:
     */
     mt_u8 ivecal_mode;	
    /*!
     disc_mode:
     */
     mt_u8 disc_mode;
    /*!
     pkt_mode
     */
     mt_u8 pkt_mode;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    reg_tsi_ds_chn_tscfg_t_sym6 ds_chn_tscfg;
    reg_tsi_ades_pktmode_t_sym6 ades_pktmode;
    reg_tsi_ades_disc_mode_t_sym6  ades_disc_mode;
    DMX_Keyslot_S keyslot_tab;
    reg_dmx_key_attribute_t key_atribute;
#else //	CONFIG_MT_CHIP_SYMPHONY6
#if defined(CONFIG_MT_CHIP_SYMPHONY1)
    reg_tsi_ds_chn_tscfg_t ds_chn_tscfg;
#else //CONFIG_MT_CHIP_SYMPHONY1
    reg_tsi_ds_chn_tscfg_t_sym2 ds_chn_tscfg;
#endif //CONFIG_MT_CHIP_SYMPHONY1
    reg_tsi_ades_pktmode_t ades_pktmode;
    reg_tsi_ades_disc_mode_t_sym2  ades_disc_mode;
    DMX_Keyslot_S keyslot_tab;
    reg_dmx_key_attribute_t key_atribute;
#endif	//CONFIG_MT_CHIP_SYMPHONY6
} DescModeSetting_t;

typedef struct
{
    mt_u32  CaType;
    mt_u32  CaEntropy;
    mt_u32  DescType;
    mt_u32  KeyLen;
    DescModeSetting_t  DescramblerMode;
    mt_u32  DmxId;
    mt_u32  Pid[DMX_AV_CHANNEL_CNT];

#ifdef MT_DEMUX_PROC_SUPPORT
    mt_u32  ChanCount;
    mt_u32  EvenKey[DMX_KEY_MAX_LEN / sizeof(mt_u32)];
    mt_u32  OddKey[DMX_KEY_MAX_LEN / sizeof(mt_u32)];
#endif
} DMX_KeyInfo_S;

typedef struct
{
    DMX_OQ_MODE_E enOQBufMode;
    mt_u32          u32OQId;
    mt_u32          u32AttachId;
    mt_u32          u32OQVirAddr;
    mt_u32          u32OQPhyAddr;
    mt_u32          u32OQDepth;
    mt_u32          u32FQId;
    mt_u32          u32ProcsBlk; /*the current OQ Desc now be processing ,in anther words: OQ read pointer keeped in software.(software is reading the data of the correlative buffer block)*/
    mt_u32          u32ProcsOffset;/*offset of the current OQ BB(buffer block: u32ProcsBlk) now be processing */
    mt_u32          u32ReleaseBlk;
    mt_u32          u32ReleaseOffset;
    mt_u32              OqWakeUp;
    wait_queue_head_t   OqWaitQueue;
    wait_queue_head_t *pWatchWaitQueue;
} DMX_OQ_Info_S;

/*
Passing decode parameters
*/
typedef struct mt_Disp_Control_t
{
    mt_u32          u32DispTime;
    mt_u32          u32DispEnableFlag;       
    mt_u32          u32DispFrameDistance;   
    mt_u32          u32DistanceBeforeFirstFrame;
    mt_u32          u32GopNum;
} Disp_Control_t;

typedef struct 
{
    ulong pes_node_addr; 	//virtual data to store a pes, it is part of the pes buffer which is malloced in create channel
    mt_u32 pes_node_len;	//the length of the pes
    mt_u32 pes_node_merged;	//whether the pes have been parsed completely. 
    struct list_head pes_node;

}DMX_SOFT_PES_NODE_S;

typedef struct
{
   mt_u32 nextpktid;
   mt_u32 ts_rp;
   mt_u32 pes_len;
   ulong pes_buf_rd;	//how many data have been read by user
   ulong pes_buf_wr;	//how many data have been write by driver
   struct list_head pes_free_node_list;
   struct list_head pes_used_node_list;
   struct list_head pes_read_node_list;
   DMX_SOFT_PES_NODE_S pes_arry[DMX_PES_NODE_NUM];
 
   mt_u32 pesphyadr;        //mmz phy addr
   mt_u32 pesphysiz;        //mmz phy size
   mt_u8 *pesdata;          //mmz vir addr
}DMX_SOFT_PESHEADER_S;

typedef struct
{
    mt_u32                          DmxId;
    mt_u32                          ChanId;
    mt_u32                          avChanId;
    mt_u32                          ChanPid;
    mt_u32                          FilterCount;
    mt_u32                          secBuffId;
    mt_u32                          ChanBufSize;
    DMX_SOFT_PESHEADER_S            *softpeshead;
    MT_UNF_DMX_CHAN_TYPE_E          APPChanType;
    MT_UNF_DMX_CHAN_TYPE_E          ChanType;
    MT_UNF_DMX_CHAN_CRC_MODE_E      ChanCrcMode;
    MT_UNF_DMX_CHAN_OUTPUT_MODE_E   ChanOutMode;
    MT_UNF_DMX_CHAN_STATUS_E        ChanStatus;
    mt_u32                          KeyId;
    mt_u32                          ChanOqId;
    mt_u32                          u32TotolAcq;
    mt_u32                          u32HitAcq;
    mt_u32                          u32Release;
    mt_u32                          u32PesBlkCnt;			/*Count of PES block alreadly be acquired by user,after finishing acquire a whole PES packet,it will be set 0*/
    mt_u32                          u32PesLength;			/*Current PES packet's len,The value been set when each time a new PES header come and parser it get the length. The value been set zeor when initialize or after inishing acquire a whole PES packet*/
    mt_u32                          u32ProcsOffset;         /* only for pes, The offset of Current PES packet alreadly been acquired, each time after DMXOsiFindPes return ,it will recorded the added datalen . it will be set 0 after finishing acquire a whole PES packet*/
    mt_u32                          LastPts;                /* the last parsed PTS*/
    mt_u32                          u32AcqTime;             /* the last get time */
    mt_u32                          u32AcqTimeInterval;     /* the interval from this time to previous time, in millisecond */
    mt_u32                          u32RelTime;             /* the last release time */
    mt_u32                          u32RelTimeInterval;     /* the interval from this time to previous time in milliseond */
    mt_u32                          u32ChnResetLock;
#ifdef DMX_USE_ECM
    mt_u32                          u32SwFlag;
#endif
    MT_BOOL                         enDiscardErrorPUSIPacket;
    MT_BOOL                         ChanEosFlag;
    Disp_Control_t                  stLastControl;
    reg_demux_slotn_cfg0_t slot_reg0;
    reg_demux_slotn_cfg1_t slot_reg1;
    MT_BOOL  avsync_flag;
    MT_UNF_VCODEC_TYPE_E   vCodecType;
    mt_u32                          u32BuffWriteP;
    struct mutex                    chan_mutex;
    mt_u32                          u32countRef;
    struct list_head                chan_node;
    wait_queue_head_t               hw_pes_wait;
	wait_queue_head_t               pes_wait;
    mt_u8							pes_wait_flag; //used for wait queue
    MT_BOOL                         aes_to_user;
    mt_s64                          pts_check_tick;
	mt_u32 							rec_id;
} DMX_ChanInfo_S;

typedef struct
{
    DMX_PORT_MODE_E             PortMode;
    mt_u32                      PortId;

#ifdef DMX_REGION_SUPPORT
    mt_u32                      DmxChanCount;
    mt_u32                      DmxFilterCount;
#endif
} DMX_Sub_DevInfo_S;

#ifdef DMX_TAG_DEAL_SUPPORT
typedef struct
{
    MT_BOOL bEnabled;
    MT_UNF_DMX_TAG_SYNC_MODE_E enSyncMod;
    mt_u32  u32TagLen;
    mt_u32  u32TSPortId;
    mt_u32  DmxAttachedPortID[DMX_CNT];             /*use to recorded each demux uses tag deal port id*/
    MT_UNF_DMX_TAG_ATTR_S  TagPortAttrs[DMX_TAG_MAX_TS_WAY];
}DMX_TagDeal_Info_S;
#else
typedef struct
{
}DMX_TagDeal_Info_S;
#endif

typedef struct dmx_symphony_swtsi_s
{
  reg_swtsi_chn_dbuf_staddr_t ch_dbuf_staddr;
  reg_swtsi_chn_next_lln_t    ch_lld_next;
  reg_swtsi_chn_dbuf_pid_t    ch_dbuf_pid;
  reg_swtsi_chn_dbuf_cfg_t    ch_dbuf_cfg;
  mt_u32        ch_dts;
  mt_u32        ch_pts;
}dmx_symphony_swtsi_t;

#if 1
typedef struct
{
    MT_UNF_DMX_PORT_TYPE_E  PortType;
    mt_u32                  SyncLockTh;
    mt_u32                  SyncLostTh;
    mt_u32                  MinLen;
    mt_u32                  MaxLen;

    phys_addr_t             DescPhyAddr;
    ulong                   DescKerAddr;
    mt_u32                  DescDepth;
    mt_u32                  DescWrite;
    mt_u32                 *DescCurrAddr;

    phys_addr_t             PhyAddr;
    ulong                   KerAddr;
    mt_u32                  BufSize;
    mt_u32                  Read;       /*Ts buffer read (offset )pointer, update in DMXOsiIsr, whtn get IP desc out int*/
    mt_u32                  Write;      /*Ts buffer write pointer,update in DMX_OsiTsBufferPut*/

    phys_addr_t             ReqAddr;    // physical address
    mt_u32                  ReqLen;

    mt_u32                  WaitLen;
    MT_BOOL                 WakeUp;
    wait_queue_head_t       WaitQueue;

    spinlock_t              LockRamPort;

    mt_u32                  GetCount;
    mt_u32                  GetValidCount;
    mt_u32                  PutCount;
    dmx_symphony_swtsi_t    *DmxRamPortInfo;
    mt_u32                  DmxRamPortInfoPhyAddr;
    mmz_buffer_s            MmzBuf;
    mt_u32                  UnblockMode;
    mt_size_t 				LockFlag;
	mt_u32 					valid_size;
	mt_u32 					reserved;
	mt_u32 					RequsetLen;
	mt_u32					LastVdecRp;
	mt_u32					VdecRpNotMoveCount;
	mt_u32					Lasttime;
	struct tasklet_struct  RamPorttasklet;
	mt_u32					full_care_reg;
    //dmx_symphony_swtsi_t __attribute__((aligned(8))) DmxRamPortInfo[2];
} DMX_RamPort_Info_S;

#else
typedef enum
{
    DMX_TS_188,
    DMX_TS_192,
    DMX_TS_ES,
    DMX_TS_PES
} DMX_RAM_data_type;

typedef struct
{
    mt_u8                   SwIndex;
    DMX_RAM_data_type DataFormat; 
    mt_u32                  DataLength; 
    mt_u32                  MemAddress;
    mt_u32                  Pid;
    mt_u8                   StreamId;  
    mt_u32                  Pts;
    mt_u32                  Dts;    

    mt_u32                  PhyAddr;
    mt_u32                  KerAddr;
    mt_u32                  BufSize;
}DMX_RamPort_Info_S;
#endif

typedef struct
{
    mt_u32 u32Info;
    mt_u32 u32Addr;
    mt_u32 u32Dts;
    mt_u32 u32Pts;
} dmx_es_desc_data_s;

typedef struct 
{
    mt_u64 origPTS;	/* 33bit PTS */
    mt_u64 u64PTS;	/* 64bit PTS in unit of microsecond */
    mt_u32 u32EsPhyAddr;
    mt_u32 pts_valid;
} dmx_es_pkt_info;

typedef struct
{
    mt_u8     sw_index;
    dmx_ts_data_t ts_format;
    mt_u32      data_length;
    mt_u32      ts_clk;
    mt_u32      mem_address;
    mt_u32      pid;
    mt_u8   stream_id;
    mt_u32      pts;
    mt_u32      dts;    
}dmx_dma_config_t;

typedef struct
{
    mt_u32              u32IsUsed;
    ulong                u32BufVirAddr;
    phys_addr_t          u32BufPhyAddr;
    mt_u32              u32BufSize;
    mt_u32              u32BlockSize;
    ulong                u32FQVirAddr;
    phys_addr_t          u32FQPhyAddr;
    mt_u32              u32FQDepth;
#ifdef MT_DEMUX_PROC_SUPPORT
    mt_u32              FqOverflowCount;
#endif
    //struct semaphore    LockFq;
    spinlock_t          LockFq; 
} DMX_FQ_Info_S;


typedef struct
{
  mt_u32 BuffId;//**对应硬件的id**//
  DMX_BUFF_STATUS_E u32IsUsed;
  reg_bufn_staddr_t buf_staddr_reg;
  reg_bufn_size_t buf_size_reg;
  reg_bufn_disc_wptr_t buf_disc_reg;
  reg_bufn_ts_int_cfg_t buf_ts_int_reg;
  reg_bufn_data_wptr_t buf_data_reg;
  reg_bufn_cursec_len_t buf_sec_len_reg;
  mmz_buffer_s  sec_data_buff;

  mt_u32 received_sec;
  mt_u32 desc_num;//ts:ts num
  mt_u32 data_buff_write_p;  
  mt_u32 desc_buff_write_p;  

  mt_u32              OqWakeUp;
  wait_queue_head_t   OqWaitQueue;
  wait_queue_head_t   *pWatchWaitQueue;
}DMX_ChanSecBuff_S;


typedef struct
{
    mt_u32 esBuffId;//**对应硬件的id**//
    mt_u32 u32IsUsed;

    reg_trpp_ch_property_t  ch_property_reg;
    reg_trpp_ch_parse_set_t  ch_parse_set_reg;
    reg_trpp_ch_start_code1_t reg_startcode1;
    reg_trpp_ch_start_code2_t reg_startcode2;
    reg_trpp_ch_frm_start_code_m1_t reg_startcode_mask1;
    reg_trpp_ch_frm_start_code_m2_t reg_startcode_mask2;

    mmz_buffer_s  data_buff;
    mmz_buffer_s  desc_buff;

    mt_u32 es_buff_write_p;  
    mt_u32 desc_buff_write_p;  
    MT_BOOL   user_buf_wpage_down;   
    MT_BOOL   user_buf_rpage_down;
    dmx_es_pkt_info cur_desc;
    dmx_es_pkt_info next_desc;
    dmx_es_pkt_info *p_cur_desc;
    dmx_es_pkt_info *p_next_desc;
    mt_u32 ch_id;
    
}DMX_ChanEsBuff_S;

typedef struct
{
    mt_u32  DmxId;
    mt_u32  PcrPid;
    mt_u32  SyncHandle;
    mt_u64  PcrValue;
    mt_u64  ScrValue;
} DMX_PCR_Info_S;

/**recorded Ts time stamp mode*/
/**CNcomment: 录制TS包时间戳添加模式*/
typedef enum mtDMX_REC_TIMESTAMP_MODE_E
{
    DMX_REC_TIMESTAMP_NONE,               /**<No time stamp added before each recoreded  ts packet*/  /**<CNcomment: 不在每个录制的TS 包前加时间戳 */
	DMX_REC_TIMESTAMP_ZERO, 				 /**<Use 4 byte 0  added before each recoreded  ts packet*/  /**<CNcomment: 在每个录制的TS 包前加4字节时间戳，内容为0 */
    DMX_REC_TIMESTAMP_HIGH32BIT_SCR,      /**<Use high 32 bit of SCR_base (4 byte)  added before each recoreded  ts packet*/  /**<CNcomment: 在每个录制的TS 包前加4字节时间戳，内容为SCR_BASE 的高32bit */ 
    DMX_REC_TIMESTAMP_LOW32BIT_SCR, 		 /**<Use low 32 bit of SCR_base (4 byte)  added before each recoreded  ts packet*/  /**<CNcomment: 在每个录制的TS 包前加4字节时间戳，内容为SCR_BASE 的低32bit */ 
} DMX_REC_TIMESTAMP_MODE_E;

typedef struct
{
    mt_u32                      DmxId;
	mt_u32                      RecId;
    MT_UNF_DMX_REC_TYPE_E       RecType;
    MT_BOOL                     Descramed;
    MT_UNF_DMX_REC_INDEX_TYPE_E IndexType;
    MT_UNF_VCODEC_TYPE_E        VCodecType;
    mt_u32                      IndexPid;
    mt_u32                      IndexChanID;
    mt_u32                      ChannelId[DMX_CHANNEL_NUM_PER_RECORD];
    DMX_REC_STATUS_E            RecStatus;
    mt_u32                      RecFqId;
    mt_u32                      ScdFqId;
    mt_u32                      RecOqId;
    mt_u32                      ScdOqId;
    mt_u32                      ScdId;
    mt_u32                      PicParser;
    mt_u32                      FirstFrameMs;
    MT_UNF_DMX_REC_INDEX_S      LastFrameInfo;
    mt_u32                      AddUpMs;
    DMX_REC_TIMESTAMP_MODE_E    enRecTimeStamp;
    mt_u32                      ScrambleDetectTime;     /*time of detecting the stream change clear----------->scramble*/
    mt_u32                      ScrambleDetectCnt;     /*conut of detecting the stream change clear----------->scramble, after we consider the scramble flag valid, we will set it 0*/
    mt_u32                      ScrambleDetectOffset;   /*TS offset of detecting the stream change clear----------->scramble*/
    mt_u32                      ClearDetectTime;        /*time of detecting the stream change scramble----------->clear*/
    mt_u32                      ClearDetectCnt;        /*count of detecting the stream change scramble----------->clear*,after we consider the clear flag valid, we will set it 0*/
    MT_BOOL                   bSCDBufIsEmpty;
    mmz_buffer_s            RecBuffer;
    mmz_buffer_s            RecIdxBuffer; 
    struct semaphore            LockRec;
    reg_demux_slotn_cfg0_t slot_reg0;
    reg_demux_slotn_cfg1_t slot_reg1;
    mt_u64  interrupt_count;
    mt_u32  pvrRecBuffZoneId;

    mt_u64  rp_globe;
    mt_u32  overflow_cnt;
    mt_u32  recbuf_size;
    DMX_RecInfo_Overflow_S  overflowinfo;
    MT_BOOL			link_mode;
    mt_u8			link_node_num; //max is 8
    mmz_buffer_s    link_rec_buf[8];
    mt_u32			read_num[8];
    mt_u8			node_index;

    mt_u8	multi_chan_flag;

    mt_u32 rec_data_overflow_cnt;
    mt_u32 rec_index_overflow_cnt;
    mt_u32 rec_init_thrd;
    mt_u8  sync_with_index; //0: use the record buf's Read and Write Pointer, 1: record buf's Read pointer sync with index's ts number.
    mt_u32  recbuf_align_size;
	
} DMX_RecInfo_S;

typedef struct 
{
    mt_u32 FilterBuffId;
    DMX_BUFF_STATUS_E u32IsUsed;
}DMX_FilterBuff_S;


typedef struct mtDMX_DEV_OSI_S
{
    DMX_TunerPort_Info_S        TunerPortInfo[DMX_TUNERPORT_CNT + 1]; /*DMX_TUNERPORT_CNT may be 0 ,so add 1*/
    DMX_RamPort_Info_S          RamPortInfo[DMX_RAMPORT_CNT];
    MT_UNF_DMX_TSO_PORT_ATTR_S  TSOPortInfo[DMX_TSOPORT_CNT + 1];   /*DMX_TSOPORT_CNT may be 0 ,so add 1*/
    DMX_Sub_DevInfo_S           SubDevInfo[DMX_CNT];
    DMX_TagDeal_Info_S          TagDealInfo;
    DMX_ChanInfo_S              DmxChanInfo[DMX_CHANNEL_CNT];
    DMX_FilterInfo_S            DmxFilterInfo[DMX_FILTER_CNT];
    DMX_KeyInfo_S               DmxKeyInfo[DMX_KEY_CNT];
    DMX_PCR_Info_S              DmxPcrInfo[DMX_PCR_CHANNEL_CNT];
    DMX_FQ_Info_S               DmxFqInfo[DMX_FQ_CNT];
    DMX_OQ_Info_S               DmxOqInfo[DMX_OQ_CNT];
    DMX_RecInfo_S               DmxRecInfo[DMX_REC_CNT];
    DMX_ChanSecBuff_S           DmxChanSecBuff[DMX_CHANNEL_CNT];
    DMX_ChanEsBuff_S            DmxChanEsBuff[DMX_AV_CHANNEL_CNT];
    DMX_FilterBuff_S            DmxFilterBuff[DMX_FILTER_BUFF_CNT];
    mt_u32                      KeyCsa2HardFlag;
    mt_u32                      KeyCsa3HardFlag;
    mt_u32                      KeySpeHardFlag;
    mt_u32                      KeyOtherHardFlag;
    mt_u32                      AVChanCount;
    mt_s32                      Reference;       /*Reference count ,for multi-process*/
    wait_queue_head_t           DmxWaitQueue;
    struct semaphore            lock_Channel;
    struct semaphore            lock_AVChan;
    struct semaphore            lock_Filter;
    struct semaphore            lock_Key;
    struct semaphore            lock_OqBuf;
	struct semaphore            lock_RecChan;
    spinlock_t                  	splock_OqBuf;
    MT_BOOL				bPvrRecBuffZoneIsUsed[DMX_REC_CNT];
    struct list_head            pes_chan_list;
    struct mutex                pes_chan_list_mutex;
    struct list_head            sec_chan_list;
    struct mutex                sec_chan_list_mutex;
    struct list_head            ts_chan_list;
    struct mutex                ts_chan_list_mutex;
    struct list_head            hw_pes_chan_list;
    struct mutex                hw_pes_chan_list_mutex;
    mt_bool                        chip_fused_fta;
} DMX_DEV_OSI_S;

/*
[127:96]:FQStartAddr(32bit);
[96:64]:{FQSize(16bit),FQWPtr(16bit)};
[63:32]:{FQVal(16bit),FQRPtr(16bit)}
[31:0]:{FQAlovfl_TH(8bit),FQIntCfg(4bit),FQIntCnt(4bit),FQUse(16bit)}
*/
//not used now
#if 0
typedef struct mt_FQ_Header_t
{
    mt_size_t FQStartAddr:32;
    mt_size_t FQSize:16;
    mt_size_t FQWPtr:16;
    mt_size_t FQVal:16;
    mt_size_t FQRPtr:16;
    mt_size_t FQAlovfl_TH:16;
    mt_size_t FQIntCfg:16;
    mt_size_t FQIntCnt:16;
    mt_size_t FQUse:16;
} FQ_HeaderInfor_t;
#else
typedef struct mt_FQ_Header_t
{
    mt_u32 FQUse;
	mt_u32 FQRPtr;
	mt_u32 FQVal;
	mt_u32 FQWPtr;
	mt_u32 FQSize;
	mt_u32 FQStartAddr;
    MT_BOOL Valid;
} FQ_HeaderInfor_t;
#endif
typedef struct
{
    mt_u32 start_addr;
    mt_u32 buflen;/*rsv(16)+buflen(16)*/
} FQ_DescInfo_S;

/*
[255:224]:BQSAddr(32bit)
[223:192]:{Rsv(2bit),BQIntCfg(4bit),BQSize(10bit),BQAlovfl_TH(8bit),BQCfg(8bit)};
[191:160]:{Rsv(6bit),BQRPtr(10bit),BQIntCnt(4bit),Rsv(2bit),BQWPtr(10bit)}
[159:128]:BBSAddr(32bit)
[127:96]:{BBSize(16bit),BQUse(16bit)}
[96:64]:{BBEopAddr(16bit),BBWaddr(16bit)}
[63:32]:{WResByte(24bit),PVRCtrl(8bit)}
[31:0]:{EopResByte(24bit),Rsv(8bit)}

*/
//not used now
#if 0
typedef struct mt_OQ_Header_t
{
    mt_size_t OQSAddr:32;
    mt_size_t Rsv1:2;
    mt_size_t OQIntCfg:4;
    mt_size_t OQSize:10;
    mt_size_t OQAlovfl_TH:8;
    mt_size_t FQCfg:8;
    mt_size_t Rsv2:6;
    mt_size_t OQRPtr:10;
    mt_size_t OQIntCnt:4;
    mt_size_t Rsv3:2;
    mt_size_t OQWPtr:10;
    mt_size_t BBSAddr:32;
    mt_size_t BBSize:16;
    mt_size_t OQUse:16;
    mt_size_t BBEopAddr:16;
    mt_size_t BBWaddr:16;
    mt_size_t WResByte:24;
    mt_size_t PVRCtrl:8;
    mt_size_t EopResByte:24;
    mt_size_t Rsv4:8;
} OQ_HeaderInfor_t;
#else
typedef struct mt_OQ_Header_t
{
    mt_u32 OQID;
	mt_u32 BBWAddr;
	mt_u32 BBEopAddr;
	mt_u32 OQUse;
	mt_u32 BBSize;
	mt_u32 BBSAddr;
	mt_u32 OQWPtr;
	mt_u32 OQRPtr;
    mt_u32 OQWAddr;
    mt_u32 OQRAddr;
	mt_u32 FQCfg;
	mt_u32 OQSize;
	mt_u32 OQSAddr;
    MT_BOOL Valid;
} OQ_HeaderInfor_t;
#endif

typedef struct mt_ChannelDataFlow_info_t
{
    mt_u32 PIDTsPacket;             /*0xda00 + 4*n*/
    mt_u32 BufferTsPacket;          /*0xd800 + 4*n */
    mt_u32 OQEn;
    mt_u32 FQEn;
} ChannelDataFlow_info_t;


/*
OQ QUEUE DESCRIBTOR
*/
typedef struct mt_OQ_Desc_t
{
    mt_u32 start_addr;
    mt_u32 cactrl_buflen;/*cactrl(16)+buflen(16)*/
    mt_u32 pvrctrl_datalen;/*rsv(8)+pvrctrl(8)+datalen(16)*/
    mt_u32 carsv;
} OQ_DescInfo_S;
typedef struct
{
    mt_u32  DescPhyAddr;
    mt_u32  DescDepth;
    mt_u32  DescWPtr ;
    mt_u32  DescRPtr ;
    mt_u32  ValidDescNum;       /*The number of desc logic can read */
    mt_u32  AddDescNum;         /**/
    mt_u32  DescWAddr;
    mt_u32  DescRAddr;
    mt_u32  DescSize;             /*the desc->buffer block size */
} DMX_Proc_RamPort_DescInfo_S;

typedef struct
{
    
    mt_u32   FQLow32BPEn;           /*IP_BP_FQ_CFG ,default is 0x0, ,each bit means a FQ (0~31),for example: 0x6 means FQ1,FQ2 BR enable */ 
    mt_u32   FQHigh8BPEn;           /*IP_BP_FQ_CFG ,default is 0x0, ,each bit means a FQ (32~39),for example: 0x6 means FQ33,FQ34 BR enable */ 
    MT_BOOL  FQBP;                  /*0xc340 + t*0x8 ,FQ BackPressure*/
    //MT_BOOL  OQBP;                /*0xc300 + t*0x10, OQ BackPressure, As driver default disable the OQ BP ,so we do not need watch here*/
    MT_BOOL  SwitchBufferBp;        /*0xc204*/
    MT_BOOL  OverflowBp;            /*0xc204, IP is in BP status (caused by channel Desc queue overflow (FQ/OQ))*/
    mt_u32   Rate;                  /*0xc830 + t*0x100 ,IP rate*/
    //MT_BOOL  SwitchBufferOverflow;  /*(0x0140)*/   
    //MT_BOOL  PortLogicOverflow;     /*0x3a28*/
    MT_BOOL  DebugEn;               /*0xC890+0x100*t , defalut is enable ,the following filds is controled by this fild*/
    mt_u32   OutByte;               /*0xC894+0x100*t ,the number of IP port ouput bytes*/  
    mt_u32   OutTSNum;        /*0xC898+0x100*t ,the number of IP port ouput TS packets*/  
    mt_u32   BPCount;               /*0xC89c+0x100*t ,the BackPressure count*/   
 
} DMX_Proc_RamPort_BPStatus_S;

/********************************************************
the common bit mask definition
*********************************************************/
#define DMX_MASK_BIT_0      0x00000001UL        /* bit0 */
#define DMX_MASK_BIT_1      0x00000002UL        /* bit1 */
#define DMX_MASK_BIT_2      0x00000004UL        /* bit2 */
#define DMX_MASK_BIT_3      0x00000008UL        /* bit3 */
#define DMX_MASK_BIT_4      0x00000010UL        /* bit4 */
#define DMX_MASK_BIT_5      0x00000020UL        /* bit5 */
#define DMX_MASK_BIT_6      0x00000040UL        /* bit6 */
#define DMX_MASK_BIT_7      0x00000080UL        /* bit7 */
#define DMX_MASK_BIT_8      0x00000100UL        /* bit8 */
#define DMX_MASK_BIT_9      0x00000200UL        /* bit9 */
#define DMX_MASK_BIT_10     0x00000400UL        /* bit10 */
#define DMX_MASK_BIT_11     0x00000800UL        /* bit11 */
#define DMX_MASK_BIT_12     0x00001000UL        /* bit12 */
#define DMX_MASK_BIT_13     0x00002000UL        /* bit13 */
#define DMX_MASK_BIT_14     0x00004000UL        /* bit14 */
#define DMX_MASK_BIT_15     0x00008000UL        /* bit15 */
#define DMX_MASK_BIT_16     0x00010000UL        /* bit16 */
#define DMX_MASK_BIT_17     0x00020000UL        /* bit17 */
#define DMX_MASK_BIT_18     0x00040000UL        /* bit18 */
#define DMX_MASK_BIT_19     0x00080000UL        /* bit19 */
#define DMX_MASK_BIT_20     0x00100000UL        /* bit20 */
#define DMX_MASK_BIT_21     0x00200000UL        /* bit21 */
#define DMX_MASK_BIT_22     0x00400000UL        /* bit22 */
#define DMX_MASK_BIT_23     0x00800000UL        /* bit23 */
#define DMX_MASK_BIT_24     0x01000000UL        /* bit24 */
#define DMX_MASK_BIT_25     0x02000000UL        /* bit25 */
#define DMX_MASK_BIT_26     0x04000000UL        /* bit26 */
#define DMX_MASK_BIT_27     0x08000000UL        /* bit27 */
#define DMX_MASK_BIT_28     0x10000000UL        /* bit28 */
#define DMX_MASK_BIT_29     0x20000000UL        /* bit29 */
#define DMX_MASK_BIT_30     0x40000000UL        /* bit30 */
#define DMX_MASK_BIT_31     0x80000000UL        /* bit31 */

#define DMX_MASK_INT_ALL               0xffffffff

#define DMX_DEFAULT_BUF_NUM     16


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // __DRV_DEMUX_DEFINE_H__

