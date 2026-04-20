/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Acrospeed Inc. All right reserved.                                           |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief
*   \author Acrospeed
*/

#ifndef __VFC_H__
#define __VFC_H__

//#include <net/mac80211.h>
//#include "wci.h"

// this is for implementation later
#define NEED_IMP_FUNC

/* WMM stream classes */
#define WMM_NUM_AC  4
#define WMM_AC_BK   0       /* background */
#define WMM_AC_BE   1       /* best effort */
#define WMM_AC_VI   2       /* video */
#define WMM_AC_VO   3       /* voice */

enum lynx_vfc_id {
    VFC_TX_AC0 = WMM_AC_BK,
    VFC_TX_AC1 = WMM_AC_BE,
    VFC_TX_AC2 = WMM_AC_VI,
    VFC_TX_AC3 = WMM_AC_VO,
    VFC_TX_CMD,
    VFC_RX_MCU,
    VFC_RX_INT,
};

#define FRAME_TYPE_BEACON 0
#define FRAME_TYPE_AMPDU  1
#define FRAME_TYPE_DATA   2
#define FRAME_TYPE_MGMT   3
#define FRAME_TYPE_WCICMD 4 

/* LYN frame header
 *
 * NOTE: do not remove or re-arrange the fields, these are minimally
 * required to take advantage of 4-byte lookaheads in some hardware
 * implementations.
 */

struct wrb {
    union {
        u32 word[8];
        u8 byte[32];
    } cb;
   
	/* software used */

	/* (struct wbuf *) is 4 byte at FW and 32bit Host OS .*/
	/*.But 8 byte at 64 bit Host OS .So change to u32 */

	u32	p_rb_next;	/* reorder buffer link list */	/* (struct wbuf *) */
	u32	timestamp;
	u16	tail;
	u8	type;
	u8	channel;
	u32	p_wb_next;/* (struct wbuf *) */
} __attribute__((__packed__));

/* word[0,1,2,7] are used */
#define rcb_w0						cb.word[0]
#define rcb_w1						cb.word[1]
#define rcb_w2						cb.word[2]
#define rcb_w3						cb.word[7]

#define rcb_b0						cb.byte[0]
#define rcb_b1						cb.byte[1]
#define rcb_b2						cb.byte[2]
#define rcb_b3						cb.byte[3]
#define rcb_b4						cb.byte[4]
#define rcb_b5						cb.byte[5]
#define rcb_b6						cb.byte[6]
#define rcb_b7						cb.byte[7]
#define rcb_b8						cb.byte[8]
#define rcb_b9						cb.byte[9]
#define rcb_b10						cb.byte[10]
#define rcb_b11						cb.byte[11]
#define rcb_b28						cb.byte[28]
#define rcb_b29						cb.byte[29]
#define rcb_b30						cb.byte[30]
#define rcb_b31						cb.byte[31]

#define WRB_B0_HOST					0x80
#define WRB_B0_WH					0x40
#define WRB_B0_LLC					0x20
#define WRB_B0_IPCSOK				0x10
#define WRB_B0_TCPCSOK				0x08
#define WRB_B0_BA					0x04
#define WRB_B0_FMDS					0x02
#define WRB_B0_TODS					0x01
#define WRB_B1_AMSDU				0x80
#define WRB_B1_BC					0x40
#define WRB_B1_MC					0x20
#define WRB_B1_QOS					0x10
#define WRB_B1_RETRY				0x08
#define WRB_B1_PS					0x04
#define WRB_B1_EOSP					0x02
#define WRB_B1_MDAT					0x01
#define WRB_B2_RAIDX				0xE0
#define WRB_B2_MFRG					0x10
#define WRB_B2_TID					0x0F
#define WRB_B3_SAIDX				0xFF

#define WRB_B0_SHIFT_HOST			7
#define WRB_B0_SHIFT_WH				6
#define WRB_B0_SHIFT_LLC			5
#define WRB_B0_SHIFT_IPCSOK			4
#define WRB_B0_SHIFT_TCPCSOK		3
#define WRB_B0_SHIFT_BA				2
#define WRB_B0_SHIFT_FMDS			1
#define WRB_B0_SHIFT_TODS			0
#define WRB_B1_SHIFT_AMSDU			7
#define WRB_B1_SHIFT_BC				6
#define WRB_B1_SHIFT_MC				5
#define WRB_B1_SHIFT_QOS			4
#define WRB_B1_SHIFT_RETRY			3
#define WRB_B1_SHIFT_PS				2
#define WRB_B1_SHIFT_EOSP			1
#define WRB_B1_SHIFT_MDAT			0
#define WRB_B2_SHIFT_RAIDX			5
#define WRB_B2_SHIFT_MFRG			4
#define WRB_B2_SHIFT_TID			0
#define WRB_B3_SHIFT_SAIDX			0
	
#define WRB_B4_RES_3130				0xC0
#define WRB_B4_PKTLEN_HI			0x3F
#define WRB_B5_PKTLEN_LO			0xFF
#define WRB_B6_SN_HI				0xFF
#define WRB_B7_SN_LO				0xF0
#define WRB_B7_FRG_NUM				0x0F

#define WRB_B4_SHIFT_RES_3130		6
#define WRB_B4_SHIFT_PKTLEN_HI		0
#define WRB_B5_SHIFT_PKTLEN_LO		0
#define WRB_B6_SHIFT_SN_HI			0
#define WRB_B7_SHIFT_SN_LO			4
#define WRB_B7_SHIFT_FRG_NUM		0

#define WRB_B8_SECST				0xF0
#define WRB_B8_HIT					0x0F
	#define WRB_B8_DA_HIT_ADDR		0x01
	#define WRB_B8_DA_HIT_BSSID		0x02
	#define WRB_B8_SA_HIT_ADDR		0x04
	#define WRB_B8_TA_HIT_DS		0x08
#define WRB_B9_DATAOFF				0xFF
#define WRB_B10_RSSI				0xFF
#define WRB_B11_RATE				0xFF

#define WRB_B8_SHIFT_SECST			4
#define WRB_B8_SHIFT_HIT			0
#define WRB_B9_SHIFT_DATAOFF		0
#define WRB_B10_SHIFT_RSSI			0
#define WRB_B11_SHIFT_RATE			0

#define WRB_B28_SNR					0xFF
#define WRB_B29_SGI					0x10
#define WRB_B29_CBW					0x08
#define WRB_B29_B_SP				0x04
#define WRB_B29_FORMAT				0x03

#define WRB_B28_SHIFT_SNR	 		0
#define WRB_B29_SHIFT_SGI	 		4
#define WRB_B29_SHIFT_CBW	 		3
#define WRB_B29_SHIFT_B_SP	 		2
#define WRB_B29_SHIFT_FORMAT		0

#define WRB_BIT_HOST(_wrb) 		!!(_wrb->cb.byte[0] & WRB_B0_HOST)
#define WRB_BIT_WH(_wrb) 		!!(_wrb->cb.byte[0] & WRB_B0_WH)

#define WRB_RAIDX(_wrb) 		((_wrb->cb.byte[2] & WRB_B2_RAIDX) >> WRB_B2_SHIFT_RAIDX)
#define WRB_SAIDX(_wrb) 		(_wrb->cb.byte[3])
#define WRB_PKTLEN(_wrb) 		(((_wrb->cb.byte[4] & WRB_B4_PKTLEN_HI) << 8) | (_wrb->cb.byte[5] & WRB_B5_PKTLEN_LO))
#define WRB_DATAOFF(_wrb) 		(_wrb->cb.byte[9] & WRB_B9_DATAOFF)
#define WRB_RSSI(_wrb) 			(_wrb->cb.byte[10] & WRB_B10_RSSI)

typedef struct {
#ifdef __BIG_ENDIAN_BITFIELD
    u32 flag:1;             /* flag, 0:MCU own, 1:To Device */
    u32 wh:1;               /* 0: Ethernet header packet, 1: WiFi header packet */
    u32 ap:1;               /* 0: Client mode, 1:AP mode */
    u32 np:1;               /* Next page */
    u32 tocpu:1;            /* Force to MCU */
    u32 ra_is_ap:1;         /* RA is AP */
    u32 wifitxdesc:1;       /* WiFi desc is carried with packet */
    u32 dis_agg:1;          /* Disable aggregation */
    u32 enterpsq:1;         /* Enter PSQ even */
    u32 secoff:1;           /* Disable encrypt */
    u32 insgsn:1;           /* Insert global sequence number */
    u32 amsd:1;             /* AMSDU packet */
    u32 ps:1;               /* PM bit */
    u32 mdat:1;             /* More Data */
    u32 eosp:1;             /* End of sevice period */
    u32 :1;
    u32 ipcsok:1;           /* IP check-sum is OK */
    u32 tcpcsok:1;          /* TCP check-sum is OK */
    u32 :2;
    u32 tid:4;              /* Wifi packet's TID */
    u32 np_index:8;         /* Rate table index, valid when NP = 1 */

    u32 pos:2;              /* 4-byte alignment address */
    u32 pktlength:14;       /* Packet length */
    u32 res:16;             /* Reserved */
#else   /*  __LITTLE_ENDIAN */
    u32 dis_agg:1;          /* Disable aggregation */
    u32 wifitxdesc:1;       /* WiFi desc is carried with packet */
    u32 ra_is_ap:1;         /* RA is AP */
    u32 tocpu:1;            /* Force to MCU */
    u32 np:1;               /* Next page */
    u32 ap:1;               /* 0: Client mode, 1:AP mode */
    u32 wh:1;               /* 0: Ethernet header packet, 1: WiFi header packet */
    u32 flag:1;             /* flag, 0:MCU own, 1:To Device */

    u32 :1;
    u32 eosp:1;             /* End of sevice period */
    u32 mdat:1;             /* More Data */
    u32 ps:1;               /* PM bit */
    u32 amsd:1;             /* AMSDU packet */
    u32 insgsn:1;           /* Insert global sequence number */
    u32 secoff:1;           /* Disable encrypt */
    u32 enterpsq:1;         /* Enter PSQ even */

    u32 tid:4;              /* Wifi packet's TID */
    u32 :2;
    u32 tcpcsok:1;          /* TCP check-sum is OK */
    u32 ipcsok:1;           /* IP check-sum is OK */

    u32 np_index:8;         /* Rate table index, valid when NP = 1 */

    u32 pktlength:14;       /* Packet length */
    u32 pos:2;              /* 4-byte alignment address */

    u32 res:16;             /* Reserved */
#endif
} __attribute__((__packed__)) txb_desc;

#define endian_16_swap(x) (*(u16 *)(x) = (*(u16 *)(x)>>8) | (*(u16 *)(x)<<8))

struct lynx_rx_hdr {
    struct wrb rx;
} __packed;
/* be careful, sizeof (lynx_tx_hdr) should be 8 bytes for hw expected */
struct lynx_tx_hdr {
    txb_desc tx;
} __packed;

    
#endif
