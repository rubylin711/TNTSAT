/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/

#ifndef __MMC_SDIO_H__
#define __MMC_SDIO_H__

#ifndef __KERNEL__

#define R4_18V_PRESENT (1<<24)
#define R4_MEMORY_PRESENT (1 << 27)

/*
 * Card Common Control Registers (CCCR)
 */

#define SDIO_CCCR_CCCR      0x00

#define  SDIO_CCCR_REV_1_00 0   /* CCCR/FBR Version 1.00 */
#define  SDIO_CCCR_REV_1_10 1   /* CCCR/FBR Version 1.10 */
#define  SDIO_CCCR_REV_1_20 2   /* CCCR/FBR Version 1.20 */
#define  SDIO_CCCR_REV_3_00 3   /* CCCR/FBR Version 3.00 */

#define  SDIO_SDIO_REV_1_00 0   /* SDIO Spec Version 1.00 */
#define  SDIO_SDIO_REV_1_10 1   /* SDIO Spec Version 1.10 */
#define  SDIO_SDIO_REV_1_20 2   /* SDIO Spec Version 1.20 */
#define  SDIO_SDIO_REV_2_00 3   /* SDIO Spec Version 2.00 */
#define  SDIO_SDIO_REV_3_00 4   /* SDIO Spec Version 3.00 */

#define SDIO_CCCR_SD        0x01

#define  SDIO_SD_REV_1_01   0   /* SD Physical Spec Version 1.01 */
#define  SDIO_SD_REV_1_10   1   /* SD Physical Spec Version 1.10 */
#define  SDIO_SD_REV_2_00   2   /* SD Physical Spec Version 2.00 */
#define  SDIO_SD_REV_3_00   3   /* SD Physical Spev Version 3.00 */

#define SDIO_CCCR_IOEx      0x02
#define SDIO_CCCR_IORx      0x03

#define SDIO_CCCR_IENx      0x04    /* Function/Master Interrupt Enable */
#define SDIO_CCCR_INTx      0x05    /* Function Interrupt Pending */

#define SDIO_CCCR_ABORT     0x06    /* function abort/card reset */

#define SDIO_CCCR_IF        0x07    /* bus interface controls */

#define  SDIO_BUS_WIDTH_MASK     0x03   /* data bus width setting */
#define  SDIO_BUS_WIDTH_1BIT     0x00
#define  SDIO_BUS_WIDTH_RESERVED 0x01
#define  SDIO_BUS_WIDTH_4BIT     0x02
#define  SDIO_BUS_ECSI           0x20   /* Enable continuous SPI interrupt */
#define  SDIO_BUS_SCSI           0x40   /* Support continuous SPI interrupt */

#define  SDIO_BUS_ASYNC_INT      0x20

#define  SDIO_BUS_CD_DISABLE     0x80   /* disable pull-up on DAT3 (pin 1) */

#define SDIO_CCCR_CAPS      0x08

#define  SDIO_CCCR_CAP_SDC  0x01    /* can do CMD52 while data transfer */
#define  SDIO_CCCR_CAP_SMB  0x02    /* can do multi-block xfers (CMD53) */
#define  SDIO_CCCR_CAP_SRW  0x04    /* supports read-wait protocol */
#define  SDIO_CCCR_CAP_SBS  0x08    /* supports suspend/resume */
#define  SDIO_CCCR_CAP_S4MI 0x10    /* interrupt during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_E4MI 0x20    /* enable ints during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_LSC  0x40    /* low speed card */
#define  SDIO_CCCR_CAP_4BLS 0x80    /* 4 bit low speed card */

#define SDIO_CCCR_CIS       0x09    /* common CIS pointer (3 bytes) */

/* Following 4 regs are valid only if SBS is set */
#define SDIO_CCCR_SUSPEND   0x0c
#define SDIO_CCCR_SELx      0x0d
#define SDIO_CCCR_EXECx     0x0e
#define SDIO_CCCR_READYx    0x0f

#define SDIO_CCCR_BLKSIZE   0x10

#define SDIO_CCCR_POWER     0x12

#define  SDIO_POWER_SMPC    0x01    /* Supports Master Power Control */
#define  SDIO_POWER_EMPC    0x02    /* Enable Master Power Control */

#define SDIO_CCCR_SPEED     0x13

#define  SDIO_SPEED_SHS     0x01    /* Supports High-Speed mode */
#define  SDIO_SPEED_BSS_SHIFT    1
#define  SDIO_SPEED_BSS_MASK     (7<<SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_SDR12        (0<<SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_SDR25        (1<<SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_SDR50        (2<<SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_SDR104       (3<<SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_DDR50        (4<<SDIO_SPEED_BSS_SHIFT)
#define  SDIO_SPEED_EHS          SDIO_SPEED_SDR25   /* Enable High-Speed */

#define SDIO_CCCR_UHS       0x14
#define  SDIO_UHS_SDR50     0x01
#define  SDIO_UHS_SDR104    0x02
#define  SDIO_UHS_DDR50     0x04

#define SDIO_CCCR_DRIVE_STRENGTH 0x15
#define  SDIO_SDTx_MASK          0x07
#define  SDIO_DRIVE_SDTA         (1<<0)
#define  SDIO_DRIVE_SDTC         (1<<1)
#define  SDIO_DRIVE_SDTD         (1<<2)
#define  SDIO_DRIVE_DTSx_MASK    0x03
#define  SDIO_DRIVE_DTSx_SHIFT   4
#define  SDIO_DTSx_SET_TYPE_B    (0 << SDIO_DRIVE_DTSx_SHIFT)
#define  SDIO_DTSx_SET_TYPE_A    (1 << SDIO_DRIVE_DTSx_SHIFT)
#define  SDIO_DTSx_SET_TYPE_C    (2 << SDIO_DRIVE_DTSx_SHIFT)
#define  SDIO_DTSx_SET_TYPE_D    (3 << SDIO_DRIVE_DTSx_SHIFT)

#endif /* #ifdef __KERNEL__ */

#define SDIO_CCCR_INT_EXT   0x16
#define  SDIO_INT_EXT_SAI        0x01
#define  SDIO_INT_EXT_EAI        0x02

#define SDIO_CCCR_OCR       0x20

#define SDIO_CCCR_RCA       0x44

#define SDIO_CCCR_RX_PRODUCER   0x60
#define SDIO_CCCR_TX_COMSUMER   0x61

#define SDIO_CCCR_DEVICE_REG_0   0x60
#define SDIO_CCCR_DEVICE_REG_1   0x61
#define SDIO_CCCR_DEVICE_REG_2   0x62
#define SDIO_CCCR_DEVICE_REG_3   0x63
#define SDIO_CCCR_DEVICE_REG_4   0x64
#define SDIO_CCCR_DEVICE_REG_5   0x65
#define SDIO_CCCR_DEVICE_REG_6   0x66
#define SDIO_CCCR_DEVICE_REG_7   0x67
#define SDIO_CCCR_DEVICE_REG_8   0x68
#define SDIO_CCCR_DEVICE_REG_9   0x69
#define SDIO_CCCR_DEVICE_REG_A   0x6A
#define SDIO_CCCR_DEVICE_REG_B   0x6B
#define SDIO_CCCR_DEVICE_REG_C   0x6C
#define SDIO_CCCR_DEVICE_REG_D   0x6D
#define SDIO_CCCR_DEVICE_REG_E   0x6E
#define SDIO_CCCR_DEVICE_REG_F   0x6F

#define SDIO_CCCR_RX_CONSUMER   0x70
#define SDIO_CCCR_TX_PRODUCER   0x71

#define SDIO_CCCR_HOST_REG_0     0x70
#define SDIO_CCCR_HOST_REG_1     0x71
#define SDIO_CCCR_HOST_REG_2     0x72
#define SDIO_CCCR_HOST_REG_3     0x73
#define SDIO_CCCR_HOST_REG_4     0x74
#define SDIO_CCCR_HOST_REG_5     0x75
#define SDIO_CCCR_HOST_REG_6     0x76
#define SDIO_CCCR_HOST_REG_7     0x77
#define SDIO_CCCR_HOST_REG_8     0x78
#define SDIO_CCCR_HOST_REG_9     0x79
#define SDIO_CCCR_HOST_REG_A     0x7A
#define SDIO_CCCR_HOST_REG_B     0x7B
#define SDIO_CCCR_HOST_REG_C     0x7C
#define SDIO_CCCR_HOST_REG_D     0x7D
#define SDIO_CCCR_HOST_REG_E     0x7E
#define SDIO_CCCR_HOST_REG_F     0x7F

#define SDIO_CCCR_RWSTA          0x80
#define SDIO_CCCR_CMD_STATE      0x81
#define SDIO_CCCR_SDIO_BUS_CST   0x82

#define SDIO_CCCR_CWD_CNT        0x84

#define SDIO_CCCR_DWD_CNT        0x88

#define SDIO_CCCR_CUR_TOTAL_SIZE 0xD0

#define SDIO_CCCR_CUR_FUN_ID     0xD3
#define  SDIO_CUR_FUN_ID_MASK         0x0F

#define SDIO_CCCR_IORDYCTRL 0xD8

#define SDIO_CCCR_SDCLTA    0xDE

#define SDIO_CCCR_CSRINTEN  0xE0

#define SDIO_CCCR_HINT_SET  0xE4
#define  SDIO_HINT_SW_LOOPBACK   (0x80)
#define  SDIO_HINT_SW_CLR_IRQ    (0x40)
#define  SDIO_HINT_SW_ENTER_SP   (0x20)
#define  SDIO_HINT_SW_RESERVE5   (0x10)
#define  SDIO_HINT_SW_RESERVE6   (0x08)
#define  SDIO_HINT_SW_RX_DONE    (0x04)
#define  SDIO_HINT_SW_TX_DONE    (0x02)
#define  SDIO_HINT_SW_FW_RDY     (0x01)

#define SDIO_CCCR_INTS      0xE8

#define SDIO_CCCR_INTSEN    0xEC
#define  SDIO_INT_RW_SHIFT       24
#define  SDIO_INT_SW_RESERVE1    (0x80 << SDIO_INT_RW_SHIFT)
#define  SDIO_INT_SW_RESERVE2    (0x40 << SDIO_INT_RW_SHIFT)
#define  SDIO_INT_SW_RESERVE3    (0x20 << SDIO_INT_RW_SHIFT)
#define  SDIO_INT_SW_RESERVE4    (0x10 << SDIO_INT_RW_SHIFT)
#define  SDIO_INT_CMD52_WM_DONE  (0x08 << SDIO_INT_RW_SHIFT)
#define  SDIO_INT_CMD52_RM_DONE  (0x04 << SDIO_INT_RW_SHIFT)
#define  SDIO_INT_CMD53_WM_DONE  (0x02 << SDIO_INT_RW_SHIFT)
#define  SDIO_INT_CMD53_RM_DONE  (0x01 << SDIO_INT_RW_SHIFT)

#define  SDIO_INT_SW_SHIFT       16
#define  SDIO_INT_SW_LOOPBACK    (0x80 << SDIO_INT_SW_SHIFT)
#define  SDIO_INT_SW_CLR_IRQ     (0x40 << SDIO_INT_SW_SHIFT)
#define  SDIO_INT_SW_ENTER_SP    (0x20 << SDIO_INT_SW_SHIFT)
#define  SDIO_INT_SW_RESERVE5    (0x10 << SDIO_INT_SW_SHIFT)
#define  SDIO_INT_SW_RESERVE6    (0x08 << SDIO_INT_SW_SHIFT)
#define  SDIO_INT_SW_RX_DONE     (0x04 << SDIO_INT_SW_SHIFT)
#define  SDIO_INT_SW_TX_DONE     (0x02 << SDIO_INT_SW_SHIFT)
#define  SDIO_INT_SW_FW_RDY      (0x01 << SDIO_INT_SW_SHIFT)

#define  SDIO_INT_MISC_SHIFT     8
#define  SDIO_INT_RESERVE7       (0x80 << SDIO_INT_MISC_SHIFT)
#define  SDIO_INT_18V_SWITCH_ERR (0x40 << SDIO_INT_MISC_SHIFT)
#define  SDIO_INT_CMD_DONE       (0x20 << SDIO_INT_MISC_SHIFT)
#define  SDIO_INT_DAT_WATCH_DOG  (0x10 << SDIO_INT_MISC_SHIFT)
#define  SDIO_INT_CMD_WATCH_DOG  (0x08 << SDIO_INT_MISC_SHIFT)
#define  SDIO_INT_FN_CHANGE      (0x04 << SDIO_INT_MISC_SHIFT)
#define  SDIO_INT_IO_RESET       (0x02 << SDIO_INT_MISC_SHIFT)
#define  SDIO_INT_SDCLK_RESUME   (0x01 << SDIO_INT_MISC_SHIFT)

#define  SDIO_INT_CRC_SHIFT      0
#define  SDIO_INT_FN7_CRC_ERR    (0x80 << SDIO_INT_CRC_SHIFT)
#define  SDIO_INT_FN6_CRC_ERR    (0x40 << SDIO_INT_CRC_SHIFT)
#define  SDIO_INT_FN5_CRC_ERR    (0x20 << SDIO_INT_CRC_SHIFT)
#define  SDIO_INT_FN4_CRC_ERR    (0x10 << SDIO_INT_CRC_SHIFT)
#define  SDIO_INT_FN3_CRC_ERR    (0x08 << SDIO_INT_CRC_SHIFT)
#define  SDIO_INT_FN2_CRC_ERR    (0x04 << SDIO_INT_CRC_SHIFT)
#define  SDIO_INT_FN1_CRC_ERR    (0x02 << SDIO_INT_CRC_SHIFT)
#define  SDIO_INT_FN0_CRC_ERR    (0x01 << SDIO_INT_CRC_SHIFT)

#define SDIO_CCCR_CIS_DMABASE    0xF0

#define SDIO_CCCR_DMAC      0xF4
#define  SDIO_DMAC_FWW_MASK      0x03
#define  SDIO_DMAC_FWW_SHIFT     0
#define  SDIO_DMAC_FWW_8_BIT     (0 << SDIO_DMAC_FWW_SHIFT)
#define  SDIO_DMAC_FWW_16_BIT    (1 << SDIO_DMAC_FWW_SHIFT)
#define  SDIO_DMAC_FWW_32_BIT    (2 << SDIO_DMAC_FWW_SHIFT)
#define  SDIO_DMAC_FWW_64_BIT    (3 << SDIO_DMAC_FWW_SHIFT)

#define SDIO_CCCR_CWDE      0xF5
#define  SDIO_CWDE_CWDE         0x01
#define  SDIO_CWDE_DWDE         0x02
#define  SDIO_CWDE_SPI_MODE     0x10

#define SDIO_CCCR_MISC      0xF6
#define  SDIO_MISC_BEE           0x01
#define  SDIO_MISC_USCS          0x02

#define SDIO_CCCR_SPICLK    0xF7

#define SDIO_CCCR_OCRSEL    0xF8
#define  SDIO_OCRSEL_MASK        0x00FFFFFF

/*
 * Function Basic Registers (FBR)
 */

#ifndef __KERNEL__

#define SDIO_FBR_BASE(f)    ((f) * 0x100) /* base of function f's FBRs */

#define SDIO_FBR_STD_IF     0x00

#define  SDIO_FBR_SUPPORTS_CSA   0x40   /* supports Code Storage Area */
#define  SDIO_FBR_ENABLE_CSA     0x80   /* enable Code Storage Area */

#define SDIO_FBR_STD_IF_EXT 0x01

#define SDIO_FBR_POWER      0x02

#define  SDIO_FBR_POWER_SPS 0x01    /* Supports Power Selection */
#define  SDIO_FBR_POWER_EPS 0x02    /* Enable (low) Power Selection */

#define SDIO_FBR_CIS        0x09    /* CIS pointer (3 bytes) */


#define SDIO_FBR_CSA        0x0C    /* CSA pointer (3 bytes) */

#define SDIO_FBR_CSA_DATA   0x0F

#define SDIO_FBR_BLKSIZE    0x10    /* block size (2 bytes) */

#endif /* #ifdef __KERNEL__ */

#define SDIO_FBR_DMABASE    0x40

#define SDIO_FBR_DMAC       0xF4







/*
 * Standard SDIO Function Interfaces
 */

#define SDIO_CLASS_NONE     0x00    /* Not a SDIO standard interface */
#define SDIO_CLASS_UART     0x01    /* standard UART interface */
#define SDIO_CLASS_BT_A     0x02    /* Type-A BlueTooth std interface */
#define SDIO_CLASS_BT_B     0x03    /* Type-B BlueTooth std interface */
#define SDIO_CLASS_GPS      0x04    /* GPS standard interface */
#define SDIO_CLASS_CAMERA   0x05    /* Camera standard interface */
#define SDIO_CLASS_PHS      0x06    /* PHS standard interface */
#define SDIO_CLASS_WLAN     0x07    /* WLAN interface */
#define SDIO_CLASS_ATA      0x08    /* Embedded SDIO-ATA std interface */

/*   
 * Vendors and devices.  Sort key: vendor first, device next.
 */  
#define SDIO_VENDOR_ID_MONTAGE              0x0188
#define SDIO_DEVICE_ID_LYNX_ROM3             0x6700
#define SDIO_DEVICE_ID_LYNX_ROM2             0x7000
#define SDIO_DEVICE_ID_LYNX_FPGA              0x1881

#define SDIO_TUPLE_FIRMWARE_INFO          0x80

#define SDIO_TUPLE_BOOT_ROM3            0x03
#define SDIO_TUPLE_BOOT_ROM2            0x81

#define SDIO_POWER_ON_RESET               1
#define SDIO_NATIVE_RESET                 0

#define SDIO_LOOPBACK_0_MODE              0
#define SDIO_LOOPBACK_X_MODE              1
#define SDIO_LOOPBACK_R_MODE              2
#define SDIO_LOOPBACK_T_MODE              3

struct sdio_fw_info{
    u32 datablock_start;
    u32 datablock_size;
    u32 datablock_num;

    /* tx ring buffer */
    u32 tx_ring_ptr;
    u32 tx_buf_size;
    u8  tx_full_flag;//8
    u8  tx_producer;//9
    u8  tx_consumer;//10

    /* rx ring buffer */
    u32 rx_buf_start;//11
    u32 rx_buf_size;//15
    u8 rx_full_flag;//19
    u8 rx_producer;//20
    u8 rx_consumer;//21

    u32 size_align;
    u8 sdio_cksum_enable;
    u8 fast_tx_commit_enable;
    u8 fast_rx_commit_enable;
} __attribute__((packed));

struct sdio_fw_info_tuple{
    u8 code;
    u8 size;
    struct sdio_fw_info info;
} __attribute__((packed));

typedef struct SDIO_HEADER {
    u8 id;
#define SDIO_CMD_NOP            0x01
#define SDIO_CMD_LOOPBACK   0x02
#define SDIO_CMD_DATA           0x04
#define SDIO_CMD_RX_TEST      0x08
#define SDIO_CMD_MASK           0xf
    u8 seq_no;
    u16 payload_len;
    u8 cksum;
    u8 offset;//u8 tx_consumer;
    u16 res;  //TX: skb ALIGN ; Rx : Rx_produce and Rx full flag
} sdio_header;
#define SDIO_HDR_SIZE   (sizeof(sdio_header))
#define DMA_ADDRESS_ALIGNMENT   (4)

struct sdio_desc {
    u8  head;
    u8  addr;
    u8  tail;
    u8  own;
    u16 len;
    u16 res;
} __attribute__((packed));

typedef struct BUFFER_LEN_REMAIN {
    u32 buf_remain;
    u32 remain1;
    u32 remain2;
} buf_len_remain;

typedef struct BUFFER_LIST {
#define TX_BUFFER_MAX_NUM   20
    struct list_head buffer_list;
    u8 buffer_cnt;
    spinlock_t t_buf_lock;
} buffer_list;

typedef struct TX_BUFFER_ENTRY {
/*Should be sync with FW: APP_TX_RING_SIZE*/
#define TX_BUFFER_SIZE (16*1024) /*16K*/

    u8* buf;
    u32 size;
    u32 start;
    u32 end;
    dma_addr_t dma_handle;
    struct list_head list_head;
} tx_buffer_entry;

#define CMD53_ARG_READ          0
#define CMD53_ARG_WRITE         1
#define CMD53_ARG_BLOCK_BASIS   1
#define CMD53_ARG_FIXED_ADDRESS 0
#define CMD53_ARG_INCR_ADDRESS  1

static inline void set_cmd53_arg(u32 *arg, u8 rw, u8 func,
					     u8 mode, u8 opcode, u32 addr,
					     u16 blksz)
{
	*arg = (((rw & 1) << 31) |
		((func & 0x7) << 28) |
		((mode & 1) << 27) |
		((opcode & 1) << 26) |
		((addr & 0x1FFFF) << 9) |
		(blksz & 0x1FF));
}


#endif
