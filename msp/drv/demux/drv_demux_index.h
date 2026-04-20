/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __FIDX_EXT_HEADER__
#define __FIDX_EXT_HEADER__

#include "mt_type.h"

#include "mt_unf_demux.h"

/************************************************************************/
/* constant                                                             */
/************************************************************************/
#define  FIDX_VERSION  20090804

#define  FIDX_OK       0
#define  FIDX_ERR      -1


/************************************************************************/
/* struct & enum                                                        */
/************************************************************************/
/* video standard type */

typedef enum mt_VIDSTD_E
{
	VIDSTD_MPEG2,
	VIDSTD_MPEG4,
	VIDSTD_AVS,
    VIDSTD_H264,
    VIDSTD_VC1,
    VIDSTD_H263,
    VIDSTD_DIVX3,
	VIDSTD_AUDIO_PES,
    VIDSTD_BUTT
}VIDSTD_E;

/* stream data type */
typedef enum mt_STRM_TYPE_E
{
    STRM_TYPE_ES = 0,
	STRM_TYPE_PES,
	STRM_TYPE_BUTT
}STRM_TYPE_E;

/* frame type */
typedef enum mt_FIDX_FRAME_TYPE_E
{
    FIDX_FRAME_TYPE_UNKNOWN = 0,
    FIDX_FRAME_TYPE_I,
	FIDX_FRAME_TYPE_P,
	FIDX_FRAME_TYPE_B,
	FIDX_FRAME_TYPE_PESH,
	FIDX_FRAME_TYPE_BUTT
} FIDX_FRAME_TYPE_E;

/* descriptor for frame information */
typedef struct hiFRAME_POS_S
{
	FIDX_FRAME_TYPE_E  eFrameType;
	mt_u32        u32PTS;
	mt_s64        s64GlobalOffset;
	mt_s32        s32OffsetInPacket;
	mt_s32        s32PacketCount;
	mt_s32        s32FrameSize;
	mt_u32        u32Reservd;
}FRAME_POS_S;

/* pvr index's SCD descriptor                                               */
typedef struct hiFINDEX_SCD_S
{
    mt_u8   u8IndexType;             /* type of index(pts,sc,pause,ts) */
    mt_u8   u8StartCode;             /* type of start code, 1Byte after 000001 */
    mt_u16  u16Reservd;    
    mt_u32  u32PtsMs;
	
    mt_s64  s64GlobalOffset;        /* start code offset in global buffer */
    mt_u8   au8DataAfterSC[8];      /* 1~8 Byte next to SC */
    
} FINDEX_SCD_S;

struct Trpp_global_info_t
{
  u8  bus_urgent_mode;  
  u8  sc_index_flt[16];
};
typedef struct Trpp_global_info_t s_trpp_global_info;

/*!
  This structure defines the parameter of a trpp_rec_info_t.
  */
struct trpp_rec_info_t
{
    /*!
      trpp_rec chnum
      */
    u8  rec_chnum;  
    /*!
      trpp_es_type video audio vbi
      */
    u8  es_type; 	
    /*!
      trpp_rec mode 192 or 188
      */
    MT_BOOL  rec_mode; 
    /*!
      trpp_rec sel clear or original
      */
    MT_BOOL  rec_sel;  
    /*!
      trpp_rec int mask
      */
    u8   rec_int_mak;
    /*!
      trpp_rec cnt 
      */
    u16   rec_cnt_th;   
    /*!
      trpp_rec start buffer addr
      */
    u32 data_buf_staddr;
    /*!
      trpp_rec end buffer addr
      */
    u32 data_buf_endaddr;
};
typedef struct trpp_rec_info_t s_trpp_rec_info;

/*!
  This structure defines the parameter of a trpp_index_info_t.
  */
struct trpp_index_info_t
{
    /*!
      trpp_index index ch
      */
    u8  index_chnum;  
    /*!
      trpp_index stream id mode
      */
    MT_BOOL  stream_id_mode; 
    /*!
      trpp_index stream id mask
      */
    u8  stream_id_mask;
    /*!
      trpp_index stream id
      */
    u8  stream_id_value;
    /*!
      trpp_index start code mode
      */
    MT_BOOL  sc_3byte_en;  
    /*!
      trpp_index length mode
      */
    MT_BOOL  pes_len_mode; 
    /*!
      trpp_index afld_en
      */
    MT_BOOL  afld_en;		
    /*!
      trpp_index start code enable
      */
    MT_BOOL  sc_en;
    /*!
      trpp_index pts enable
      */
    MT_BOOL  pts_en; 
    /*!
      trpp_index ts head enable
      */
    MT_BOOL  pes_head_en;
    /*!
      trpp_index pusi enable
      */
    MT_BOOL  pusi_en;  
    /*!
      trpp_index pause enable
      */
    MT_BOOL  pause_en;   
    /*!
      trpp_index filt enable
      */
    u16   filt_en;      
    /*!
      trpp_index end buffer addr
      */
    u32 data_buf_staddr;
    /*!
      trpp_index end buffer addr
      */
    u32 data_buf_endaddr;
    /*!
      trpp_index filter
      */
    u8   start_code_filter[16];
};
typedef struct trpp_index_info_t s_trpp_index_info;

typedef enum TRPP_INDEX_TYPE
{
    TRPP_INDEX_DTS = 0x00,
    TRPP_INDEX_ES_SC,
    TRPP_INDEX_PTS,
    TRPP_INDEX_TS_HEAD,
}TRPP_INDEX_TYPE_E;

typedef union { /* dmx_trpp_ch_idx_rd_addr */
    mt_u64 all;
    struct {
        mt_u32 pause                    : 1;
        mt_u32                     : 2;
        mt_u32 afld                    : 1;
        mt_u32 pusi                     :1;
        mt_u32                          :3;
        mt_u32 stream_id            :8;
        mt_u32 stream_offset            :8;
        mt_u32 rec_ch_num            :32;
        mt_u32                             : 5;
        mt_u32 stream_id_err        :1;
        TRPP_INDEX_TYPE_E idx_num        :2;
    }__attribute__((packed)) ts_head;
    struct {
        mt_u32 dts_23_0                    : 24;
        mt_u32 rec_ch_num            :32;
        mt_u32                             : 5;
        mt_u32 stream_id_err        :1;
        TRPP_INDEX_TYPE_E idx_num        :2;
    }__attribute__((packed)) pts ;
    struct {
        mt_u64 pts                    : 33;
        mt_u32             :7;
        mt_u32 dts_32_24                    : 9;
        mt_u32             :7;
        mt_u32 pts_vld                     : 1;
        mt_u32         :3;
        mt_u32 dts_vld                     : 1;
        mt_u32         :1;
        TRPP_INDEX_TYPE_E idx_num        :2;
    }__attribute__((packed)) dts ;
    struct {
        mt_u32 sc_3b                    : 8;
        mt_u32 sc_4b            :8;
        mt_u32 sc_offset            :8;
        mt_u32 rec_ch_num            :32;
        mt_u32                             : 5;
        mt_u32 stream_id_err        :1;
        TRPP_INDEX_TYPE_E idx_num        :2;
    }__attribute__((packed)) es_sc ;
} reg_trpp_ch_idx_data_t;

/*!***********************************************************************/
/*! interface functions definition                                       */
/*!***********************************************************************/

/*!***********************************************************************
@brief
	global init, clear residual information, and register call back.
 ************************************************************************/
mt_void FIDX_Init(mt_void (*OutputFramePosition)(mt_u32 *Param, FRAME_POS_S *pstScInfo));

/*!***********************************************************************
@brief
    open an instance
@param	
	VidStandard:  video standard type
@return
    if success return the instance ID, between 0 and (FIDX_MAX_CTX_NUM-1)
	otherwise return -1
 ************************************************************************/
mt_s32  FIDX_OpenInstance(VIDSTD_E VidStandard, STRM_TYPE_E StrmType, mt_u32 *Param);

/************************************************************************
@brief
    close specified instance
@param
	InstIdx:  the ID of the instance to be closed
 ************************************************************************/
mt_s32  FIDX_CloseInstance( mt_s32 InstIdx );


/*!***********************************************************************
@brief
    feed start code to FIDX.
    there are 2 method to feed necessary information to FIDX:
	1. feed stream directly. Call FIDX_MakeFrameIndex()
	2. feed start code. In this method, the start code must be scanned outside,
	   This call this function to create index.
 ************************************************************************/
mt_void  FIDX_FeedStartCode(
    mt_s32 InstIdx,                    /*! instance ID */
    const FINDEX_SCD_S *pstSC );      /* SCD descriptor */

/*!***********************************************************************
@brief
    to see if the SC is usful for index making
@return
    if the SC is usful return FIDX_OK, otherwise return FIDX_ERR
 ************************************************************************/
mt_s32  FIDX_IsSCUseful(mt_s32 InstIdx, mt_u8 u8StartCode);
#endif




