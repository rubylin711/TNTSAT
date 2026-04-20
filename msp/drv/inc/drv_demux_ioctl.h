/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
File Name     : drv_demux_ioctl.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2009/12/14
Last Modified :
Description   : demux
Function List :
History       :
* main\1    2009-12-14   jianglei     init.
******************************************************************************/
#ifndef __DRV_DEMUX_IOCTL_H__
#define __DRV_DEMUX_IOCTL_H__

#include "mt_type.h"

#include "mt_mpi_demux.h"
#include "drv_demux_config.h"
#include "mt_drv_demux.h"
/***************************** Macro Definition ******************************/
#define DMX_CHANID(ChanHandle)      ((ChanHandle) & 0xff)

#define DMX_CHANHANDLE(ChanId)      ((ChanId) | 0x00000100 | (MT_ID_DEMUX << 16))

#define DMX_CHECK_CHANHANDLE(ChanHandle)                                \
    do                                                                  \
    {                                                                   \
        if (   (DMX_CHANID(ChanHandle) >= DMX_CHANNEL_CNT)              \
            || (((ChanHandle) & 0xffffff00) != DMX_CHANHANDLE(0)) )     \
        {                                                               \
            MT_ERR_DEMUX("Invalid Handle 0x%x\n", ChanHandle);         \
            return MT_ERR_DMX_INVALID_PARA;                             \
        }                                                               \
    } while (0)

#define DMX_RECID(RecHandle)    ((RecHandle) & 0xff)

#define DMX_RECHANDLE(RecId)    ((RecId) | 0x00000400 | (MT_ID_DEMUX << 16))

#define DMX_CHECK_RECHANDLE(RecHandle)                                  \
    do                                                                  \
    {                                                                   \
        if (   (DMX_RECID(RecHandle) >= DMX_REC_CNT)                        \
            || (((RecHandle) & 0xffffff00) != DMX_RECHANDLE(0)) )       \
        {                                                               \
            MT_ERR_DEMUX("Invalid Handle 0x%x\n", RecHandle);          \
            return MT_ERR_DMX_INVALID_PARA;                             \
        }                                                               \
    } while (0)

/*************************** Structure Definition ****************************/
typedef struct mtDMX_Port_GetAttr_S
{
    DMX_PORT_MODE_E         PortMode;
    mt_u32                  PortId;
    MT_UNF_DMX_PORT_ATTR_S  PortAttr;
} DMX_Port_GetAttr_S;

typedef DMX_Port_GetAttr_S DMX_Port_SetAttr_S ;

typedef struct mtDMX_Tag_GetAttr_S
{
    mt_u32 DmxId;
    MT_UNF_DMX_TAG_ATTR_S TagAttr;
}DMX_Tag_GetAttr_S;

typedef DMX_Tag_GetAttr_S DMX_Tag_SetAttr_S;

typedef struct mtDMX_TSO_Port_GetAttr_S
{
    mt_u32                  PortId;
    MT_UNF_DMX_TSO_PORT_ATTR_S  PortAttr;
} DMX_TSO_Port_Attr_S;

typedef struct mtDMX_Port_Attach_S
{
    DMX_PORT_MODE_E PortMode;
    mt_u32          PortId;
    mt_u32          DmxId;
} DMX_Port_Attach_S;

typedef struct mtDMX_Port_Ctrl_S
{
    DMX_PORT_MODE_E 		PortMode;
    mt_u32          		PortId;
    MT_UNF_DMX_PORT_CTRL_S 	PortValue;
} DMX_Port_Ctrl_S;

typedef struct mtDMX_Port_Clk_S
{
    DMX_PORT_MODE_E 			PortMode;
    mt_u32          			PortId;
    MT_UNF_DMX_PORT_CLK_SEL_E 	PortClk;
} DMX_Port_Clk_S;

typedef struct mtDMX_Port_Src_S
{
    DMX_PORT_MODE_E 			PortMode;
    mt_u32          			PortId;
    MT_UNF_DMX_PORT_SRC_SEL_E 	PortSrc;
} DMX_Port_Src_S;

typedef struct mtDMX_PortPacketNum_S
{
    DMX_PORT_MODE_E PortMode;
    mt_u32          PortId;
    mt_u32          TsPackCnt;
    mt_u32          ErrTsPackCnt;
} DMX_PortPacketNum_S;

typedef DMX_Port_Attach_S DMX_Port_GetId_S;

typedef struct mtDMX_TsBufInit_S
{
    mt_u32          PortId;
    mt_u32          BufSize;
    DMX_MMZ_BUF_S   TsBuf;
}DMX_TsBufInit_S;

typedef struct mtDMX_TsBufGet_S
{
    mt_u32          PortId;
    mt_u32          ReqLen;
    DMX_DATA_BUF_S  Data;
    mt_u32          TimeoutMs;
} DMX_TsBufGet_S;

typedef struct mtDMX_TsBufPut_S
{
    mt_u32  PortId;
    mt_u32  ValidDataLen;
    mt_u32  StartPos;
    dmx_ts_data_t DateType;
    mt_u32  Pts;
    mt_u16  Pid;
    phys_addr_t  SpecifiedDataAddr;
} DMX_TsBufPut_S;

typedef struct mtDMX_TsBufStaGet_S
{
    mt_u32                      PortId;
    MT_UNF_DMX_TSBUF_STATUS_S   Status;
} DMX_TsBufStaGet_S;

typedef struct mtDMX_TsBufFullCare_S
{
    mt_u32                      	PortId;
    MT_UNF_DMX_AV_CHN_FULL_CARE_S   AvFullCare;
	MT_UNF_DMX_REC_CHN_FULL_CARE_S 	RecFullCare;  
} DMX_TsBufFullCare_S;

typedef struct mtDMX_ChanNew_S
{
    mt_u32 u32DemuxId;
    MT_UNF_DMX_CHAN_ATTR_S stChAttr;
    mt_handle hChannel;
    DMX_MMZ_BUF_S stChBuf;
    DMX_MMZ_BUF_S stChDescBuf;
}DMX_ChanNew_S;

typedef struct mtDMX_GetChan_Attr_S
{
    mt_handle hChannel;
    MT_UNF_DMX_CHAN_ATTR_S stChAttr;
}DMX_GetChan_Attr_S;

typedef DMX_GetChan_Attr_S DMX_SetChan_Attr_S ;


typedef struct mtDMX_ChanPIDSet_S
{
    mt_handle hChannel;
    mt_u32 u32Pid;
}DMX_ChanPIDSet_S;

typedef struct mtDMX_ChanPIDGet_S
{
    mt_handle hChannel;
    mt_u32  u32Pid;
}DMX_ChanPIDGet_S;

typedef struct mtDMX_ChanStatusGet_S
{
    mt_handle hChannel;
    MT_UNF_DMX_CHAN_STATUS_S stStatus;
}DMX_ChanStatusGet_S;

typedef struct mtDMX_ChannelIdGet_S
{
    mt_u32 u32DmxId;
    mt_u32  u32Pid;
    MT_UNF_DMX_CHAN_TYPE_E enChannelType;
    mt_handle hChannel;
}DMX_ChannelIdGet_S;

typedef struct mtDMX_AVChannelIdGet_S
{
    mt_u32 u32DmxId;
    mt_u32  u32Pid;
    mt_handle vidChannel;
    mt_handle audChannel;
}DMX_AVChannelIdGet_S;
typedef struct mtDMX_FreeChanGet_S
{
    mt_u32 u32DmxId;
    mt_u32 u32FreeCount;
}DMX_FreeChanGet_S;

typedef struct mtDMX_ScrambledFlagGet_S
{
    mt_handle hChannel;
    MT_UNF_DMX_SCRAMBLED_FLAG_E enScrambleFlag;
}DMX_ScrambledFlagGet_S;

typedef struct
{
    mt_u32                      DmxId;
    MT_UNF_DMX_FILTER_ATTR_S    FilterAttr;
    mt_handle                   Filter;
} DMX_NewFilter_S;

typedef struct
{
    mt_handle                   Filter;
    MT_UNF_DMX_FILTER_ATTR_S    FilterAttr;
} DMX_FilterSet_S;

typedef DMX_FilterSet_S DMX_FilterGet_S;

typedef struct
{
    mt_handle   Filter;
    mt_handle   Channel;
} DMX_FilterAttach_S;

typedef DMX_FilterAttach_S DMX_FilterDetach_S;
typedef DMX_FilterAttach_S DMX_FilterChannelIDGet_S;

typedef struct
{
    mt_u32  DmxId;
    mt_u32  FreeCount;
} DMX_FreeFilterGet_S;

typedef struct mtDMX_GetDataFlag_S
{
    mt_u32 u32Flag[4];
    mt_u32 u32TimeOutMs;
}DMX_GetDataFlag_S;

typedef struct mtDMX_SelectDataFlag_S
{
    mt_handle *channel;             /*channel handles to check*/
    mt_u32 channelnum;              /*channel number to check*/
    mt_u32 u32Flag[4];              /*dataflag*/
    mt_u32 u32TimeOutMs;            /*timeout time in MS*/
}DMX_SelectDataFlag_S;

typedef struct mtDMX_CheckDataFlag_S
{
    mt_handle hChannel;             /*channel handle to check*/
    mt_u32 u32TimeOutMs;            /*timeout time in MS*/
}DMX_CheckDataFlag_S;

typedef struct mtDMX_AcqMsg_S
{
    mt_handle hChannel;
    mt_u32 u32AcquireNum;
    mt_u32 u32AcquiredNum;
    MT_UNF_DMX_DATA_S *pstBuf;
    mt_u32 u32TimeOutMs;
}DMX_AcqMsg_S;

typedef struct mtDMX_RelMsg_S
{
    mt_handle hChannel;
    mt_u32 u32ReleaseNum;
    MT_UNF_DMX_DATA_S *pstBuf;
}DMX_RelMsg_S;

typedef struct mtDMX_NewPcr_S
{
    mt_u32 u32DmxId;
    mt_u32 u32PcrId;
}DMX_NewPcr_S;

typedef struct mtDMX_PcrPidSet_S
{
    mt_u32 pu32PcrChId;
    mt_u32 u32Pid;
}DMX_PcrPidSet_S;

typedef DMX_PcrPidSet_S  DMX_PcrPidGet_S;

typedef struct mtDMX_PcrScrGet_S
{
    mt_u32 pu32PcrChId;
    mt_u32 reserve;
    mt_u64 u64PcrValue;
    mt_u64 u64ScrValue;
}DMX_PcrScrGet_S;

typedef struct mtDMX_PcrValGet_S
{
    mt_u32 pu32PcrChId;
    mt_u32 u32PcrMs;
}DMX_PcrValGet_S;

typedef struct
{
    mt_u32 u32PcrChId;
    mt_u32 u32SyncHandle;
} DMX_PCRSYNC_S;

typedef struct mtDMX_PesBufAttach_S
{
    mt_handle hChannel;
    mt_mmz_buf_s stPesBuf;
}DMX_PesBufAttach_S;

typedef struct mtDMX_PesBufStaGet_S
{
    mt_handle hChannel;
    MT_MPI_DMX_BUF_STATUS_S stBufStat;
}DMX_PesBufStaGet_S;

typedef struct mtDMX_PesBufGet_S
{
    mt_handle hChannel;
    MT_UNF_ES_BUF_S stEsBuf;
}DMX_PesBufGet_S;

typedef struct
{
    mt_handle               RecHandle;
    MT_UNF_DMX_REC_ATTR_S   RecAttr;
    phys_addr_t             RecBufPhyAddr;
    mt_u32                  RecBufSize;
    phys_addr_t             RecIdxBufPhyAddr;
    mt_u32                  RecIdxBufSize;
} DMX_Rec_CreateChan_S;

typedef struct
{
    mt_handle               RecHandle;
    MT_UNF_DMX_REC_ATTR_S   RecAttr;
} DMX_Rec_SwitchIndex_S;

typedef struct
{
    mt_handle   RecHandle;
    mt_handle   ChanHandle;
    mt_u32      Pid;
} DMX_Rec_AddPid_S;

typedef struct
{
    mt_handle   RecHandle;
    mt_handle   ChanHandle;
} DMX_Rec_DelPid_S;

typedef struct
{
    mt_handle   RecHandle;
    mt_u32      Pid;
} DMX_Rec_ExcludePid_S;

typedef struct
{
    mt_handle               RecHandle;
    MT_UNF_DMX_REC_DATA_S   RecData;
    mt_u32                  TimeoutMs;
} DMX_Rec_AcquireData_S;

typedef struct
{
    mt_handle               RecHandle;
    MT_UNF_DMX_REC_DATA_S   RecData;
    mt_u32                  TimeoutMs;
	mt_u8					node_index;
} DMX_LinkRec_AcquireData_S;

typedef struct
{
    mt_handle               RecHandle;
    MT_UNF_DMX_REC_DATA_S   RecData;
} DMX_Rec_ReleaseData_S;

#if 1
typedef struct
{
    mt_handle               RecHandle;
    MT_UNF_DMX_REC_DATA_S   RecData;
    mt_u32                  TimeoutMs;
} DMX_Rec_AcquireScd_S;

typedef struct
{
    mt_handle               RecHandle;
    MT_UNF_DMX_REC_DATA_S   RecData;
} DMX_Rec_ReleaseScd_S;
#endif

typedef struct
{
    mt_handle               RecHandle;
    MT_UNF_DMX_REC_INDEX_S  IndexData;
    mt_u32                  TimeoutMs;
} DMX_Rec_AcquireIndex_S;

typedef struct
{
    mt_handle                   RecHandle;
    MT_UNF_DMX_RECBUF_STATUS_S  BufStatus;
} DMX_Rec_BufStatus_S;

#ifdef DMX_USE_ECM
typedef struct
{
    mt_handle   hChannel;
    mt_u32      u32SwFlag;
} DMX_ChanSwGet_S;

typedef struct
{
    mt_handle       hChannel;
    DMX_MMZ_BUF_S   stChnBuf;
} DMX_ChanSwBufGet_S;
#endif

typedef struct
{
    mt_handle   hChannel;
    mt_u32      u32ChanTsCnt;
} DMX_ChanChanTsCnt_S;

typedef struct mtDMX_SetChan_ExtAttr_S
{
    MT_UNF_DMX_CHAN_CC_REPEAT_SET_S stChCCRepeatSet;
}DMX_SetChan_CC_REPEAT_S;

typedef struct
{
    mt_u32      u32RegisterOffset;
    mt_u32      u32RegisterValue;
} DMX_SetRegister_S;

typedef struct mtDMX_BufFullCare_S
{
    mt_handle hChannel;
    MT_UNF_DMX_BUF_FULL_CARE_S  stBufFullCare;
}DMX_BufFullCare_S;

typedef struct
{
	mt_u8   swtsi_num;
	mt_u8	rec_id;
	mt_u8	node_num;
} DMX_FastPlay_Param_S;

typedef struct
{
	mt_u32	enable;	//0:disable t2mi, 1:enable t2mi
    mt_u32	input;	//t2mi input source channel
	mt_u32	output; //t2mi output destination channel
	mt_u32	pid;	//t2mi pid
	mt_u32	plpid;	//t2mi plpid
} DMX_T2MI_Para_S;

typedef struct mtDMX_Set_PortId_S
{
    mt_handle 		hChannel;
	DMX_PORT_MODE_E	PortMode;
    mt_u32    		PortId;
} DMX_Set_PortId_S;

/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
typedef struct
{
    mt_handle               hChannel;
    mt_u8  isenable;
} TSI_CIPlus_Enable_S;

typedef struct
{
    mt_handle  				chan;
} TSI_CIPlus_ChCfg_S;

typedef struct
{
    mt_u8  					use_cache;
} TSI_CIPlus_BufCfg_S;

typedef struct
{
	mt_u8 					enable;
} TSI_CIPlus_EnableCfg_S;


typedef struct
{
    mt_u32  				div;
} TSI_CIPlus_CamClkCfg_S;

typedef struct
{
    mt_u32  				interval;
} TSI_CIPlus_TsPktIntervalCfg_S;

typedef struct
{
    mt_u8  				from_cam;
	mt_u8				serial;
} TSI_TSI2_SourceCfg_S;

typedef struct
{
    mt_u32  lln_num_start;
} TSI_CIPlus_LlnNumStart_S;

typedef struct
{
    mt_u32  byteorder;
} TSI_CIPlus_SwtsiByteorder_S;

typedef struct
{
    mt_handle               hChannel;
    mt_u8  care_full;
} TSI_CIPlus_SwtsiFullCfg_S;

typedef struct
{
    mt_u32  ahb_rd_delay;
} TSI_CIPlus_AhbRdDelay_S;

typedef struct
{
    mt_u8  bufstatus;
	mt_u8  cistatus;
} TSI_CIPlus_Status_S;

#endif
/*add end*/

/********************************************************
  DEMUX command code definition
 *********************************************************/
/*DEMUX base code of command code*/
#define DEMUX_CMD_MASK              (0xF0)
#define DEMUX_GLOBAL_CMD            (0x00)
#define DEMUX_PORT_CMD              (0x10)
#define DEMUX_TSBUFFER_CMD          (0x20)
#define DEMUX_CHAN_CMD              (0x30)
#define DEMUX_FILT_CMD              (0x40)
#define DEMUX_KSET_CMD              (0x50)
#define DEMUX_RECV_CMD              (0x60)
#define DEMUX_PCR_CMD               (0x70)
#define DEMUX_AV_CMD                (0x80)
#define DEMUX_REC_CMD               (0x90)
#define DEMUX_DATA_CMD              (0xA0)
#define DEMUX_T2MI_CMD              (0xB0)
#define DEMUX_LINKREC_FASTPLAY_CMD  (0xC0)
#define DEMUX_CIPLUS_CMD			(0xD0)
#define DEMUX_OTHER1_CMD            (0xE0)
#define DEMUX_OTHER2_CMD            (0xF0)

/* global */
#define CMD_DEMUX_GET_POOLBUF_ADDR      _IOR (MT_ID_DEMUX, 0x00, DMX_MMZ_BUF_S)
#define CMD_DEMUX_GET_CAPABILITY        _IOR (MT_ID_DEMUX, 0x01, MT_UNF_DMX_CAPABILITY_S)
#define CMD_DEMUX_SET_PUSI              _IOW (MT_ID_DEMUX, 0x02, MT_UNF_DMX_PUSI_SET_S)
#define CMD_DEMUX_SET_TEI               _IOW (MT_ID_DEMUX, 0x03, MT_UNF_DMX_TEI_SET_S)
#define CMD_DEMUX_TSI_ATTACH_TSO        _IOW (MT_ID_DEMUX, 0x04, MT_UNF_DMX_TSI_ATTACH_TSO_S)
#define CMD_DEMUX_GET_REGISTER         	_IOW (MT_ID_DEMUX, 0x05, mt_u32)
#define CMD_DEMUX_SET_REGISTER         	_IOW (MT_ID_DEMUX, 0x06, DMX_SetRegister_S)
#define CMD_DEMUX_DUMP_ALL_REGISTER  	_IOW (MT_ID_DEMUX, 0x07, mt_u32)
#define CMD_DEMUX_DMX_SET_PORTID        _IOW (MT_ID_DEMUX, 0x08, MT_UNF_DMX_PORT_E)
#define CMD_DEMUX_SOFT_RESET		  	_IOW (MT_ID_DEMUX, 0x09, mt_u32)
#define CMD_DEMUX_HAEDWARE_INIT		  	_IOW (MT_ID_DEMUX, 0x0A, mt_u32)
#define CMD_DEMUX_GET_CLK		  		_IOR (MT_ID_DEMUX, 0x0B, MT_UNF_DMX_TSI_CLK_E)
#define CMD_DEMUX_SET_CLK		  		_IOW (MT_ID_DEMUX, 0x0C, MT_UNF_DMX_TSI_CLK_E)

/* TS PORT */
#define CMD_DEMUX_PORT_GET_ATTR             _IOWR(MT_ID_DEMUX, 0x10, DMX_Port_GetAttr_S)        /* get port attr */
#define CMD_DEMUX_PORT_SET_ATTR             _IOW (MT_ID_DEMUX, 0x11, DMX_Port_SetAttr_S)        /* set port attr */
#define CMD_DEMUX_PORT_ATTACH               _IOW (MT_ID_DEMUX, 0x12, DMX_Port_Attach_S)         /* attach ts port to demux */
#define CMD_DEMUX_PORT_DETACH               _IOW (MT_ID_DEMUX, 0x13, mt_u32)                    /* detach ts port from demux */
#define CMD_DEMUX_PORT_GETID                _IOWR(MT_ID_DEMUX, 0x14, DMX_Port_GetId_S)          /* get ts port id of demux */
#define CMD_DEMUX_PORT_GETPACKETNUM         _IOWR(MT_ID_DEMUX, 0x15, DMX_PortPacketNum_S)       /* get ts pack counter */
#define CMD_DEMUX_TSO_PORT_GET_ATTR          _IOWR(MT_ID_DEMUX, 0x16, DMX_TSO_Port_Attr_S)        /* get TSO port attr */
#define CMD_DEMUX_TSO_PORT_SET_ATTR          _IOW(MT_ID_DEMUX, 0x17, DMX_TSO_Port_Attr_S)        /* Set TSO port attr */
#define CMD_DEMUX_DMX_GET_TAG_ATTR           _IOWR(MT_ID_DEMUX, 0x18, DMX_Tag_GetAttr_S) /* get port tag attrs */
#define CMD_DEMUX_DMX_SET_TAG_ATTR           _IOW(MT_ID_DEMUX, 0x19, DMX_Tag_SetAttr_S) /* set port tag attrs */
#define CMD_DEMUX_GET_PORT_CTRL          	_IOWR(MT_ID_DEMUX, 0x1A, DMX_Port_Ctrl_S) /* get port sample ctrl */
#define CMD_DEMUX_SET_PORT_CTRL          	_IOW(MT_ID_DEMUX, 0x1B, DMX_Port_Ctrl_S) /* set port sample ctrl */
#define CMD_DEMUX_GET_PORT_SRC          	_IOWR(MT_ID_DEMUX, 0x1C, DMX_Port_Src_S) /* get port src */
#define CMD_DEMUX_SET_PORT_SRC          	_IOW(MT_ID_DEMUX, 0x1D, DMX_Port_Src_S) /* set port src */
#define CMD_DEMUX_GET_PORT_CLK          	_IOWR(MT_ID_DEMUX, 0x1E, DMX_Port_Src_S) /* get port clk */
#define CMD_DEMUX_SET_PORT_CLK          	_IOW(MT_ID_DEMUX, 0x1F, DMX_Port_Src_S) /* set port clk */

/* Ts Buffer */
#define CMD_DEMUX_TS_BUFFER_INIT            _IOWR(MT_ID_DEMUX, 0x20, DMX_TsBufGet_S)            /* TS Buffer init */
#define CMD_DEMUX_TS_BUFFER_DEINIT          _IOW (MT_ID_DEMUX, 0x21, mt_u32)                    /* TS Buffer deinit */
#define CMD_DEMUX_TS_BUFFER_GET             _IOWR(MT_ID_DEMUX, 0x22, DMX_TsBufGet_S)            /* Get TS Buffer */
#define CMD_DEMUX_TS_BUFFER_PUT             _IOW (MT_ID_DEMUX, 0x23, DMX_TsBufPut_S)            /* Put TS Buffer */
#define CMD_DEMUX_TS_BUFFER_RESET           _IOW (MT_ID_DEMUX, 0x24, mt_u32)                    /* Reset TS Buffer */
#define CMD_DEMUX_TS_BUFFER_GET_STATUS      _IOWR(MT_ID_DEMUX, 0x25, DMX_TsBufStaGet_S)         /* Get TS Buffer status */
#define CMD_DEMUX_CHAN_BUF_FULL_CARE_SET    _IOW (MT_ID_DEMUX, 0x26, DMX_BufFullCare_S)

#define CMD_DEMUX_TS_BUFFER_GET_FULL_CARE    _IOWR (MT_ID_DEMUX, 0x27, DMX_TsBufFullCare_S)
#define CMD_DEMUX_TS_BUFFER_SET_FULL_CARE    _IOW (MT_ID_DEMUX, 0x28, DMX_TsBufFullCare_S)

/* Channal */
#define CMD_DEMUX_CHAN_NEW                  _IOWR(MT_ID_DEMUX, 0x30, DMX_ChanNew_S)             /* apply for a free channel */
#define CMD_DEMUX_CHAN_DEL                  _IOW (MT_ID_DEMUX, 0x31, mt_handle)                 /* delete an allocated channel */
#define CMD_DEMUX_CHAN_OPEN                 _IOW (MT_ID_DEMUX, 0x32, mt_handle)                 /* open channel */
#define CMD_DEMUX_CHAN_CLOSE                _IOW (MT_ID_DEMUX, 0x33, mt_handle)                 /* close channel */
#define CMD_DEMUX_CHAN_ATTR_GET             _IOWR(MT_ID_DEMUX, 0x34, DMX_GetChan_Attr_S)
#define CMD_DEMUX_CHAN_ATTR_SET             _IOW (MT_ID_DEMUX, 0x35, DMX_SetChan_Attr_S)
#define CMD_DEMUX_GET_CHAN_STATUS           _IOWR(MT_ID_DEMUX, 0x36, DMX_ChanStatusGet_S)       /* get channel open/close status */
#define CMD_DEMUX_PID_SET                   _IOW (MT_ID_DEMUX, 0x37, DMX_ChanPIDSet_S)          /* set pid of channel */
#define CMD_DEMUX_PID_GET                   _IOWR(MT_ID_DEMUX, 0x38, DMX_ChanPIDGet_S)          /* get pid of channel */
#define CMD_DEMUX_CHANID_GET                _IOWR(MT_ID_DEMUX, 0x39, DMX_ChannelIdGet_S)        /* get channel id with the designated pid */
#define CMD_DEMUX_AVCHANID_GET                _IOWR(MT_ID_DEMUX, 0x49, DMX_AVChannelIdGet_S)        /* get channel id with the designated pid */
#define CMD_DEMUX_FREECHAN_GET              _IOWR(MT_ID_DEMUX, 0x3A, DMX_FreeChanGet_S)         /* get free channel counter */
#define CMD_DEMUX_SCRAMBLEFLAG_GET          _IOWR(MT_ID_DEMUX, 0x3B, DMX_ScrambledFlagGet_S)    /* get scrambed flag of audio channel */
#define CMD_DEMUX_CHAN_SET_EOS_FLAG         _IOWR(MT_ID_DEMUX, 0x3C, mt_handle)
#define CMD_DEMUX_CHAN_CC_REPEAT_SET        _IOW (MT_ID_DEMUX, 0x3D, DMX_SetChan_CC_REPEAT_S)   /* set channel CC repeat attr*/


#ifdef DMX_USE_ECM
#define CMD_DEMUX_GET_CHAN_SWFLAG           _IOWR(MT_ID_DEMUX, 0x3D, DMX_ChanSwGet_S)
#define CMD_DEMUX_GET_CHAN_SWBUF_ADDR       _IOWR(MT_ID_DEMUX, 0x3E, DMX_ChanSwBufGet_S)        /* get sw buffer addr */
#endif

#define CMD_DEMUX_GET_CHAN_TSCNT            _IOWR(MT_ID_DEMUX, 0x3F, DMX_ChanChanTsCnt_S)       /* get channel ts count */

/* Filter */
#define CMD_DEMUX_FLT_NEW                   _IOWR(MT_ID_DEMUX, 0x40, DMX_NewFilter_S)           /* apply for a free filter */
#define CMD_DEMUX_FLT_DEL                   _IOW (MT_ID_DEMUX, 0x41, mt_handle)                 /* delete an allocated filter */
#define CMD_DEMUX_FLT_SET                   _IOW (MT_ID_DEMUX, 0x42, DMX_FilterSet_S)           /* set fiter parameter */
#define CMD_DEMUX_FLT_GET                   _IOWR(MT_ID_DEMUX, 0x43, DMX_FilterGet_S)           /* get fiter parameter */
#define CMD_DEMUX_FLT_ATTACH                _IOW (MT_ID_DEMUX, 0x44, DMX_FilterAttach_S)        /* attach a filter to a channel */
#define CMD_DEMUX_FLT_DETACH                _IOW (MT_ID_DEMUX, 0x45, DMX_FilterDetach_S)        /* detach a filter from a channel */
#define CMD_DEMUX_FREEFLT_GET               _IOWR(MT_ID_DEMUX, 0x46, DMX_FreeFilterGet_S)       /* get free filter coute */
#define CMD_DEMUX_FLT_DELALL                _IOW (MT_ID_DEMUX, 0x47, mt_handle)                 /* delete all filters on a channel */
#define CMD_DEMUX_FLT_CHANID_GET            _IOWR(MT_ID_DEMUX, 0x48, DMX_FilterChannelIDGet_S)

/* data receive */
#define CMD_DEMUX_GET_DATA_FLAG             _IOWR(MT_ID_DEMUX, 0x60, DMX_GetDataFlag_S)         /* get data flag of dma buffer */
#define CMD_DEMUX_ACQUIRE_MSG               _IOWR(MT_ID_DEMUX, 0x61, DMX_AcqMsg_S)
#define CMD_DEMUX_RELEASE_MSG               _IOW (MT_ID_DEMUX, 0x62, DMX_RelMsg_S)
#define CMD_DEMUX_SELECT_DATA_FLAG          _IOWR(MT_ID_DEMUX, 0x63, DMX_SelectDataFlag_S)
#define CMD_DEMUX_CHECK_DATA_FLAG           _IOWR(MT_ID_DEMUX, 0x64, DMX_CheckDataFlag_S)

/* PCR */
#define CMD_DEMUX_PCR_NEW                   _IOWR(MT_ID_DEMUX, 0x70, DMX_NewPcr_S)              /* set pcr pid */
#define CMD_DEMUX_PCR_DEL                   _IOW (MT_ID_DEMUX, 0x71, mt_u32)                    /* set pcr pid */
#define CMD_DEMUX_PCRPID_SET                _IOW (MT_ID_DEMUX, 0x72, DMX_PcrPidSet_S)           /* set pcr pid */
#define CMD_DEMUX_PCRPID_GET                _IOWR(MT_ID_DEMUX, 0x73, DMX_PcrPidGet_S)           /* get pcr pid */
#define CMD_DEMUX_CURPCR_GET                _IOWR(MT_ID_DEMUX, 0x74, DMX_PcrScrGet_S)           /* get pcr count */
#define CMD_DEMUX_PCRSYN_ATTACH             _IOWR(MT_ID_DEMUX, 0x75, DMX_PCRSYNC_S)             /* attach pcr channel and sync handle */
#define CMD_DEMUX_PCRSYN_DETACH             _IOWR(MT_ID_DEMUX, 0x76, DMX_PCRSYNC_S)             /* detach pcr channel and sync handle */

/* AV */
#define CMD_DEMUX_PES_BUFFER_GETSTAT        _IOWR(MT_ID_DEMUX, 0x80, DMX_PesBufStaGet_S)        /* Get PES Buffer status */
#define CMD_DEMUX_ES_BUFFER_GET             _IOWR(MT_ID_DEMUX, 0x81, DMX_PesBufGet_S)           /* Get ES Buffer */
#define CMD_DEMUX_ES_BUFFER_PUT             _IOW (MT_ID_DEMUX, 0x82, DMX_PesBufGet_S)           /* Put ES Buffer */

/* REC */
#define CMD_DEMUX_REC_CHAN_CREATE           _IOWR(MT_ID_DEMUX, 0x90, DMX_Rec_CreateChan_S)
#define CMD_DEMUX_REC_CHAN_DESTROY          _IOW (MT_ID_DEMUX, 0x91, mt_handle)
#define CMD_DEMUX_REC_CHAN_ADD_PID          _IOWR(MT_ID_DEMUX, 0x92, DMX_Rec_AddPid_S)
#define CMD_DEMUX_REC_CHAN_DEL_PID          _IOW (MT_ID_DEMUX, 0x93, DMX_Rec_DelPid_S)
#define CMD_DEMUX_REC_CHAN_DEL_ALL_PID      _IOW (MT_ID_DEMUX, 0x94, mt_handle)
#define CMD_DEMUX_REC_CHAN_ADD_EXCLUDE_PID  _IOW (MT_ID_DEMUX, 0x95, DMX_Rec_ExcludePid_S)
#define CMD_DEMUX_REC_CHAN_DEL_EXCLUDE_PID  _IOW (MT_ID_DEMUX, 0x96, DMX_Rec_ExcludePid_S)
#define CMD_DEMUX_REC_CHAN_CANCEL_EXCLUDE   _IOW (MT_ID_DEMUX, 0x97, mt_handle)
#define CMD_DEMUX_REC_CHAN_START            _IOW (MT_ID_DEMUX, 0x98, mt_handle)
#define CMD_DEMUX_REC_CHAN_STOP             _IOW (MT_ID_DEMUX, 0x99, mt_handle)
#define CMD_DEMUX_REC_CHAN_ACQUIRE_DATA     _IOWR(MT_ID_DEMUX, 0x9A, DMX_Rec_AcquireData_S)
#define CMD_DEMUX_REC_CHAN_RELEASE_DATA     _IOW (MT_ID_DEMUX, 0x9B, DMX_Rec_ReleaseData_S)
#define CMD_DEMUX_REC_CHAN_ACQUIRE_INDEX    _IOWR(MT_ID_DEMUX, 0x9C, DMX_Rec_AcquireIndex_S)
#define CMD_DEMUX_REC_CHAN_GET_BUF_STATUS   _IOWR(MT_ID_DEMUX, 0x9D, DMX_Rec_BufStatus_S)

/* ES Data Push */
#define CMD_DEMUX_DATA_PUSH_START           _IOW(MT_ID_DEMUX, 0xA0, mt_handle)
#define CMD_DEMUX_DATA_PUSH_STOP            _IOW(MT_ID_DEMUX, 0xA1, mt_handle)
#define CMD_DEMUX_CHAN_INDEX_RESUME         _IOW(MT_ID_DEMUX, 0xA2, mt_handle)   /* set channel index*/
#define CMD_DEMUX_TRICK_SEEK_IN             _IOW(MT_ID_DEMUX, 0xA3, mt_handle)
#define CMD_DEMUX_TRICK_SEEK_OUT            _IOW(MT_ID_DEMUX, 0xA4, mt_handle)

#define CMD_DEMUX_T2MI_ENABLE     		_IOW(MT_ID_DEMUX, 0xB0, mt_u32)
#define CMD_DEMUX_T2MI_SET_IN_CH    	_IOW(MT_ID_DEMUX, 0xB1, mt_u32)
#define CMD_DEMUX_T2MI_SET_OUT_CH    	_IOW(MT_ID_DEMUX, 0xB2, mt_u32)
#define CMD_DEMUX_T2MI_SET_PID    		_IOW(MT_ID_DEMUX, 0xB3, mt_u32)
#define CMD_DEMUX_T2MI_SET_PLPID  		_IOW(MT_ID_DEMUX, 0xB4, mt_u32)
#define CMD_DEMUX_T2MI_SOFTRESET  		_IOW(MT_ID_DEMUX, 0xB5, mt_u32)
#define CMD_DEMUX_T2MI_CONFIG	  		_IOW(MT_ID_DEMUX, 0xB6, DMX_T2MI_Para_S)


#define CMD_DEMUX_LINKREC_CHAN_CREATE       	_IOWR(MT_ID_DEMUX, 0xC0, DMX_LinkRec_CreateChan_S)
#define CMD_DEMUX_LINKREC_CHAN_DESTROY      	_IOW (MT_ID_DEMUX, 0xC1, mt_handle)
#define CMD_DEMUX_LINKREC_CHAN_ACQUIRE_DATA 	_IOWR(MT_ID_DEMUX, 0xC2, DMX_LinkRec_AcquireData_S)
#define CMD_DEMUX_LINKREC_CHAN_RELEASE_DATA 	_IOW (MT_ID_DEMUX, 0xC3, DMX_Rec_ReleaseData_S)
#define CMD_DEMUX_LINKREC_CHAN_ACQUIRE_INDEX    _IOWR(MT_ID_DEMUX, 0xC4, DMX_Rec_AcquireIndex_S)

#define CMD_DEMUX_FASTPLAY_START _IOWR(MT_ID_DEMUX, 0xCA, DMX_LinkRec_AcquireData_S)
#define CMD_DEMUX_FASTPLAY_STOP  _IOW (MT_ID_DEMUX, 0xCB, DMX_Rec_ReleaseData_S)

/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#define CMD_TSI_CI_CHAN_EN				_IOWR(MT_ID_DEMUX, 0xD0, TSI_CIPlus_Enable_S)
#define CMD_TSI_CI_REC_CH_SET		_IOWR(MT_ID_DEMUX, 0xD1, TSI_CIPlus_ChCfg_S)	
#define CMD_TSI_CI_SWTSI_CH_SET		_IOWR(MT_ID_DEMUX, 0xD2, TSI_CIPlus_ChCfg_S)	
#define CMD_TSI_CI_BUF_CFG			_IOWR(MT_ID_DEMUX, 0xD3, TSI_CIPlus_BufCfg_S)
#define CMD_TSI_CI_CAMCLK_CFG		_IOWR(MT_ID_DEMUX, 0xD4, TSI_CIPlus_CamClkCfg_S)
#define CMD_TSI_CI_TSINTERVAL_CFG		_IOWR(MT_ID_DEMUX, 0xD5, TSI_CIPlus_TsPktIntervalCfg_S)
#define CMD_TSI_TSI2_SRC_CFG		_IOWR(MT_ID_DEMUX, 0xD6, TSI_TSI2_SourceCfg_S)
#define CMD_TSI_CI_LLN_NUM_START_CFG							_IOWR(MT_ID_DEMUX,0xD7,TSI_CIPlus_LlnNumStart_S)
#define CMD_TSI_CI_SWTSI_BYTEORDER_CFG							_IOWR(MT_ID_DEMUX,0xD8,TSI_CIPlus_SwtsiByteorder_S)
#define CMD_TSI_CI_SWTSI_FULL_CFG	_IOWR(MT_ID_DEMUX, 0xD9, TSI_CIPlus_SwtsiFullCfg_S)
#define CMD_TSI_CI_AHBRD_DELAY_CFG	_IOWR(MT_ID_DEMUX, 0xDA,	TSI_CIPlus_AhbRdDelay_S)
#define CMD_TSI_CI_GETSTATUS		_IOR(MT_ID_DEMUX, 0xDB,	TSI_CIPlus_Status_S)
#define CMD_TSI_CI_ENABLE_CFG		_IOWR(MT_ID_DEMUX, 0xDC, TSI_CIPlus_BufCfg_S)

#endif
/*add end*/

#define CMD_DEMUX_GET_CHAN_REF	_IOWR(MT_ID_DEMUX, 0xE0, mt_handle)					/*get channel reference count, 0:success, other:failure*/
#define CMD_DEMUX_GET_CHAN_AVID	_IOWR(MT_ID_DEMUX, 0xF0, mt_handle)					/*get channel reference count, 0:success, other:failure*/

#endif  // __DRV_DEMUX_IOCTL_H__

