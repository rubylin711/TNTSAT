/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_DEMUX_CONFIG_H__
#define __DRV_DEMUX_CONFIG_H__

#define IO_ADDRESS(addr) (addr)

//#define DMX_INT_NUM                    (82 + 32)/*A9一共有128个中断，A9自己占用32个*/
#if defined(CONFIG_MT_CHIP_ARIA)
#define DMX_INT_PVR                     (54 + 32 - 8)
#define DMX_INT_GLB                     (55 + 32 - 8)
#define DMX_INT_SEC                     (56 + 32 - 8)
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) 
#define DMX_INT_PVR                     IRQ_PVR_GINT_ID
#define DMX_INT_GLB                     IRQ_TSIG0_GIN_ID
#define DMX_INT_SEC                     IRQ_PTI_SF_ID
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
#define DMX_INT_PVR                     IRQ_PVR_GINT_ID
#define DMX_INT_GLB                     IRQ_TSIG0_GIN_ID
#define DMX_INT_SEC                     IRQ_PTI_SF_ID
#endif

//#define MT_PVR_BASE                     IO_ADDRESS(reg_dmx_regs_base)

#if defined (CONFIG_MT_CHIP_SYMPHONY4)
#define DMX_CNT             6	//this means how many input(4+2), TS0-TS3, swtsi0-swtsi1
#define DMX_RAMPORT_CNT     2
#define DMX_REC_CNT         4
#define DMX_REC_INDEX_CNT	8
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
#define DMX_CNT             8	//this means how many input(4+4), TS0-TS3, swtsi0-swtsi3
#define DMX_RAMPORT_CNT		4
#define DMX_REC_CNT			7
#define DMX_REC_INDEX_CNT	12
#define DMX_LINKREC_NODE_NUM	8
#endif

#define DMX_IFPORT_CNT              1
#define DMX_TSIPORT_CNT             3

#define DMX_TSOPORT_CNT                 0
#define DMX_TUNERPORT_CNT              (DMX_IFPORT_CNT + DMX_TSIPORT_CNT)

#define DMX_CHANNEL_CNT                 128

//the hardware has 16 channels(0x0~0xF), but we use 0xf as the invalid channel in 0xbf26002c
//so there are only 15 channels can be used in driver
#define DMX_AV_CHANNEL_CNT          15	
#define DMX_PCR_CHANNEL_CNT         8

#define DMX_FILTER_CNT                    64 //64*4 = 256
#define DMX_FILTER_BUFF_CNT               256 //256*4 = 1024
#define DMX_FILTER_BUFF_LEN               1024
#if defined (CONFIG_MT_CHIP_SYMPHONY4)
#define DMX_KEY_CNT                         32
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
#define DMX_KEY_CNT                         32
#else
#define DMX_KEY_CNT                         16
#endif
#define DMX_FQ_CNT                           40
#define DMX_OQ_CNT                           128

#define DMX_RAM_PORT_OFFSET        0//15

#define DMX_SCD_NEW_FLT_SUPPORT

#define DMX_DESCRAMBLER_SUPPORT
#define DMX_DESCRAMBLER_VERSION_1

#define DMX_DESCRAMBLER_TYPE_SPE_SUPPORT
#define DMX_DESCRAMBLER_TYPE_DES_SUPPORT
#define DMX_DESCRAMBLER_TYPE_AES_NS_SUPPORT
#define DMX_DESCRAMBLER_TYPE_AES_CBC_SUPPORT
#define DMX_DESCRAMBLER_TYPE_DES_IPTV_SUPPORT
#define DMX_DESCRAMBLER_TYPE_TDES_SUPPORT

#define DMX_TUNER_PORT_SERIAL_2BIT_AND_SERIAL_NOSYNC_SUPPORT

#define DMX_RAM_PORT_SET_LENGTH_SUPPORT
#define DMX_RAM_PORT_AUTO_SCAN_SUPPORT

#define DMX_SECTION_PARSE_NOPUSI_SUPPORT

#define DMX_FILTER_DEPTH_SUPPORT

#define DMX_REC_EXCLUDE_PID_SUPPORT
#define DMX_REC_EXCLUDE_PID_NUM         (30)
#define DMX_SUPPORT_RAM_CLK_AUTO_CTL

#define DMX_SYMPHONY_TS_LEN                     (188)
#define DMX_SYMPHONY_SEC_DESC_BUF_SIZE          (1024 * 8)
/*
the size of the buf for store the section data which one part
in the end of the section buf and the other in the start of
the section buf, when handle this seciton, we need copy this
section to a continous buf, it behind of the description buf
*/
#define DMX_SYMPHONY_SEC_ROLLBACK_BUF_SIZE      (1024 * 4)
#define DMX_SYMPHONY_MAX_SEC_DESC_BUF_SIZE      (1024 * 1023)
#define DMX_SYMPHONY_ES_DESC_BUF_SIZE           (1024 * 16)
#define DMX_SYMPHONY_PES_DESC_BUF_SIZE          (1024 * 16)

#define DMX_SYMPHONY_PES_ROLLBACK_BUF_SIZE      (1024 * 64)
#define DMX_SYMPHONY_PES_PARSE_NODE_BUF_SIZE    (1024 * 128)

//video: 16k might not enough!
//20180815: 32k(1024 frames=40s) is enough for Live DVB.
#define DMX_SYMPHONY_VES_DESC_BUF_SIZE          (1024 * 32)


/*!
  TS input config
  */
typedef struct
{
  /*!
      1:parrel  0 :serial
    */
   MT_BOOL input_way;
  /*!
    configure the TS data is active at the falling edge or rising edge!
    1: rising edge ; 0: falling edge
    */
   MT_BOOL local_sel_edge;
  /*!
      0: do not check packet error indicator in TS
      1: check packet error indicator in TS, drop the packet if it is set.
    */
   MT_BOOL error_indicator;
  /*!
      1 :mask 0x47
      0 : no
    */
   MT_BOOL start_byte_mask;
}dmx_ts_input_t;
#endif  // __DRV_DEMUX_CONFIG_H__

