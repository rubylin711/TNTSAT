/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_KT_REG_H__
#define __HW_KT_REG_H__

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#define KT_BASE    SYMPHONY_IO_VA(0xBF330000)

// 31:14    RSV
// 13:8     target_key_slot  RW  target key slot index
//              the index of the target key slot to be written or read
// 7:3      RSV
// 2:0      operation    RW  operation mode
//              000: valid/invalid target key slot, this operation is to set the SlotValid flag in target key slot with WriteData[0]
//              001: write KEY into target key slot; it can be forbidden by leaving OTP_HCPUKey2KT[3:0] all zero
//              010: write IV into target key slot
//              011: write KeyAttribute into target key slot
//              100: Reserved for REECPU; write TeeCfgData into TeeData  Slot for TEECPU and VSCPU(SecureAHBHost)
//              101: RSV
//              110: read TeeData
//              111: read target key slot, the read data will be placed in ReadData register
//Default: 0x0
#define KT_OPERATION (KT_BASE + 0x0)

// 31:1     RSV
// 0        start     RW  bit to trigger operations
//              1: trigger an operation; when an operation is done, this bit will return to 0 by HW when this bit is 1,
//              next operation can't be executed and input data can't be changed
//Default: 0x0
#define KT_START  (KT_BASE + 0x4)

// 31:1     RSV
// 0        big_endian   RW  0: little endian
//              1: big endian
//              Write:
//              for SlotValid and KeyAttribute, this bit should be 1;
//              for Key and IV, this bit should be 0;
//              Read:
//              this bit should be 0
//Default: 0x0
#define KT_ENDIAN  (KT_BASE + 0x8)

// 31:5     RSV
// 4          tee_cfgdata_chken RW one-way register, only set by TEECPU or VSCPU(SecureAHBHost)
//              0: don't check teecfg data; 1: check teecfg data
// 3:1       RSV
// 0          ivclr_mode RW IV clear mode
//              0: don't clear IV when invalidating slot; 1: clear IV when invalidating slot
//Default: 0x0
#define KT_IVCLR_MODE (KT_BASE + 0xC)

// 31:0     write_data_127t96  WO  used for KEY[127:96] or IV[127:96]
//              big endian:  KEY/IV[127:96]
//              little endian: KEY/IV[103:96]
//                             KEY/IV[111:104]
//                             KEY/IV[119:112]
//                             KEY/IV[127:120]
//Default: 0x0
#define KT_WR_DATA_127T96  (KT_BASE + 0x10)

// 31:0     write_data_95t64  WO  used for KEY[95:64] or IV[95:64]
//              big endian:  KEY/IV[95:64]
//              little endian: KEY/IV[71:64]
//                             KEY/IV[47:40]
//                             KEY/IV[79:72]
//                             KEY/IV[95:88]
//Default: 0x0
#define KT_WR_DATA_95T64  (KT_BASE + 0x14)

// 31:0     write_data_63t32  WO  used for KEY[63:32] or IV[63:32]
//              big endian:  KEY/IV[63:32]
//              little endian: KEY/IV[39:32]
//                             KEY/IV[47:40]
//                             KEY/IV[55:48]
//                             KEY/IV[63:56]
//Default: 0x0
#define KT_WR_DATA_63T32  (KT_BASE + 0x18)

// 31:0     write_data_31t0  WO  used for KEY[31:0] or IV[31:0]
//              big endian:  KEY/IV[31:0]
//              little endian: KEY/IV[7:0]
//                             KEY/IV[15:8]
//                             KEY/IV[23:16]
//                             KEY/IV[31:24]
//              for KeyAttribute: 32 bits used, and should big endian
//              for SlotValid: only LSB 1 used, and should big endian
//              the following just used for SCPU and VSCPU:
//              TP/SC/SCID/WCID:  bit[3:0] is wcid
//                                bit[7:4] is scid
//                                bit[8] is sc
//                                bit[9] is tp
//              permission:    bit[3:0] is permission
//Default: 0x0
#define KT_WR_DATA_31T0  (KT_BASE + 0x1C)

// 31:0     read_data_127t96  RO  read data bit 127:96
//              big endian: IV[127:96]
//              little endian: IV[103:96]
//                             IV[111:104]
//                             IV[119:112]
//                             IV[127:120]
//Default: 0x00061728
#define KT_RD_DATA_127T96  (KT_BASE + 0x20)

// 31:0     read_data_95t64  RO  read data bit 95:64
//              big endian: IV[95:64]
//              little endian: IV[71:64]
//                             IV[79:72]
//                             IV[87:80]
//                             IV[95:88]
//Default: 0x00061728
#define KT_RD_DATA_95T64  (KT_BASE + 0x24)

// 31:0     read_data_63t32  RO  read data bit 63:32
//              big endian: IV[63:32]
//              little endian: IV[39:32]
//                             IV[47:40]
//                             IV[55:48]
//                             IV[63:56]
//Default: 0x0
#define KT_RD_DATA_63T32  (KT_BASE + 0x28)

// 31:0     read_data_31t0  RO  read data bit 31:0
//              big endian: IV[31:0]
//              little endian: IV[7:0]
//                             IV[15:8]
//                             IV[23:16]
//                             IV[31:24]
//Default: 0x0
#define KT_RD_DATA_31T0  (KT_BASE + 0x2C)

// 31:0     key_attribute   RO  read key attribute[31:0], it's readable when OTP_KTAttrReadDis[1:0] is 2'b00;
//              when OTP_KTAttrReadDis[1:0] is not 2'b00, read key_attribute[31:0] is 0xbbaa9988
//Default: 0x0
#define KT_RD_DATA_ATTR  (KT_BASE + 0x30)

// 31:0     read_key_127t96  RO  read key bit 127:96
//              big endian: KEY[127:96]
//              little endian: KEY[103:96]
//                             KEY[111:104]
//                             KEY[119:112]
//                             KEY[127:120]
//Default: 0x0
#define KT_RD_KEY_127T96  (KT_BASE + 0x40)

// 31:0     read_key_95t64  RO  read key bit 95:64
//              big endian: KEY[95:64]
//              little endian: KEY[71:64]
//                             KEY[79:72]
//                             KEY[87:80]
//                             KEY[95:88]
//Default: 0x0
#define KT_RD_KEY_95T64  (KT_BASE + 0x44)

// 31:0     read_key_63t32  RO  read key bit 63:32
//              big endian: KEY[63:32]
//              little endian: KEY[39:32]
//                             KEY[47:40]
//                             KEY[55:48]
//                             KEY[63:56]
//Default: 0x0
#define KT_RD_KEY_63T32  (KT_BASE + 0x48)

// 31:0     read_key_31t0  RO  read key bit 31:0
//              big endian: KEY[31:0]
//              little endian: KEY[7:0]
//                             KEY[15:8]
//                             KEY[23:16]
//                             KEY[31:24]
//Default: 0x0
#define KT_RD_KEY_31T0  (KT_BASE + 0x4C)

// 31:1     RSV
// 0:       kt_key_rden_debug RW  0: key can't be read
//                                      1: key can be read
//Default: 0x0
#define KT_RD_KEY_DEBUG  (KT_BASE + 0x50)

// 31:1     RSV
// 0:         kt_tee_cfg_data_chk_lock RW lock signal for tee_cfg_data_chk
//                1: lock, it's one-way register and only can be configed by SecureAHBHost
//Default: 0x0
#define KT_WR_TEE_CFG_DATA_CHK_LOCK  (KT_BASE + 0x54)

// 31:0     kt_tee_cfg_data_chk_31t0 RW tee_cfg_data_chk bit[31:0]
//                can be locked by tee_cfg_data_chk_lock and only can be configed by SecureAHBHost
//Default: 0x0
#define KT_WR_TEE_CFG_DATA_CHK_31t0  (KT_BASE + 0x58)

// 31:4     RSV
// 3:0       kt_tee_cfg_data_chk_35t32 RW  tee_cfg_data_chk bit[35:32]
//                can be locked by tee_cfg_data_chk_lock and only can be configed by SecureAHBHost
//Default: 0x0
#define KT_WR_TEE_CFG_DATA_CHK_35t32  (KT_BASE + 0x5C)

// 31:     kt_read_data_teecfg_chk RO
//             0: tee cfg data check fail; 1: tee cfg data check pass
// 30:1     RSV
// 0:       kt_read_data_slotvalid RO  read data of slotvalid
//Default: 0x0
#define KT_RD_DATA_SLOTVALID (KT_BASE + 0x34)

// 31:14    RSV
// 13       tp     RO  0: can be used in REE
//              1: forbit using in REE
// 12       sc     RW  0: RDCID shall be matched with SCID
//              1: don't case SCID
// 11:8     scid     RO  protected channel id for input
// 7:4      wcid     RO  protected channel id for write transaction
// 3:0      permission   RO  0000: any known key source can write this entry.
//                           others: key source from the secure domain can write this entry(TEE).
//                              i.e, PortSCPUKey, PortCAVnKey semaphored by SCPU,
//                              PortVSCPU, PortCAV1Key semaphored by VSCPU
//Default: 0x0
#define KT_RD_DATA_TEE   (KT_BASE + 0x38)

// 31:1     RSV
// 0:       kt_rng_clr   WO  clear Nonce SlotValid and Key before CPU want to update Nonce
//Default: 0x0
#define KT_RNG_CLR    (KT_BASE + 0x78)

// 31:9     RSV
// 8        kt_rng_nonce_teecfg_chkpass RW indicate tee data cfg for nonce check pass or not
//             0: fail;  1: pass
// 7:5     RSV
// 4        kt_rng_nonce_valid RW indicate that curent nonce is valid or not
//             1 means valid
// 3:1     RSV
// 0:       kt_rng_not_empty  RO  random data has arrived after reset
//              0: has no valid random data, random data is 0
//              1: has valid random data
//Default: 0x0
#define KT_RNG_STATUS   (KT_BASE + 0x7C)

#endif /*__HW_KT_REG_H__*/

