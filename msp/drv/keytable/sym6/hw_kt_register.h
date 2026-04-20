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
#define hw_kt_vbase KT_BASE

/*
 * 31:15    RSV
 * 14:8     target_key_slot  RW  target key slot index
 *                               the index of the target key slot to be written or read
 * 7:3      RSV
 * 2:0      operation    RW  operation mode
 *                           000: valid/invalid target key slot, this operation is to set the SlotValid flag in target key slot with WriteData[0]
 *                           001: write KEY into target key slot; it can be forbidden by leaving OTP_HCPUKey2KT[3:0] all zero
 *                           010: write IV into target key slot
 *                           011: write KeyAttribute into target key slot
 *                           100: Reserved for REECPU; write TeeCfgData into TeeData  Slot for TEECPU and VSCPU(SecureAHBHost)
 *                           101: RSV
 *                           110: read TeeData
 *                           111: read target key slot, the read data will be placed in ReadData register
 * Default: 0x0
 */
#define KT_OPERATION (hw_kt_vbase + 0x0)

/*
 * 31:1     RSV
 * 0        start     RW  bit to trigger operations
 *                        1: trigger an operation; when an operation is done, this bit will return to 0 by HW when this bit is 1,
 *                        next operation can't be executed and input data can't be changed
 * Default: 0x0
 */
#define KT_START  (hw_kt_vbase + 0x4)

/*
 *  31:1     RSV
 *  0        order mode   RW
 *                            0: little endian
 *                            1: big endian
 *                            Write:
 *                              for SlotValid/KeyAttribute/Tee permission/Tee info, this bit should be 1;
 *                              for Key and IV, this bit should be 0;
 *                            Read:
 *                              this bit should be 0
 * Default: 0x0
 */
#define KT_ENDIAN  (hw_kt_vbase + 0x8)

/*
 *  31:5     RSV
 *  4        tee_cfgdata_chken RW    one-way register, only set by TEECPU or VSCPU(SecureAHBHost)
 *                                   0: don't check teecfg data; 1: check teecfg data
 *  3:1      RSV
 *  0        ivclr_mode        RW    IV clear mode
 *                                   0: don't clear IV when invalidating slot;
 *                                   1: clear IV when invalidating slot
 * Default: 0x0
 */
#define KT_MODE (hw_kt_vbase + 0xC)

/*
 * 31:0     write_data_127t96  WO  used for KEY[127:96] or IV[127:96]
 *                                 big endian:  KEY/IV[127:96]
 *                                 little endian: KEY/IV[103:96]
 *				                  KEY/IV[111:104]
 *                                                KEY/IV[119:112]
 *                                                KEY/IV[127:120]
 * Default: 0x0
 */
#define KT_WR_DATA_127T96  (hw_kt_vbase + 0x10)

/*
 * 31:0     write_data_95t64  WO  used for KEY[95:64] or IV[95:64]
 *                                big endian:  KEY/IV[95:64]
 *                                little endian: KEY/IV[71:64]
 *                                               KEY/IV[47:40]
 *			                         KEY/IV[79:72]
 *                                               KEY/IV[95:88]
 * Default: 0x0
 */
#define KT_WR_DATA_95T64  (hw_kt_vbase + 0x14)

/*
 * 31:0     write_data_63t32  WO  used for KEY[63:32] or IV[63:32]
 *                                big endian:  KEY/IV[63:32]
 *                                little endian: KEY/IV[39:32]
 *                                               KEY/IV[47:40]
 *                                               KEY/IV[55:48]
 *                                               KEY/IV[63:56]
 * Default: 0x0
 */
#define KT_WR_DATA_63T32  (hw_kt_vbase + 0x18)

/*
 * 31:0     write_data_31t0  WO  used for KEY[31:0] or IV[31:0]
 *                               big endian:  KEY/IV[31:0]
 *                               little endian: KEY/IV[7:0]
 *                                              KEY/IV[15:8]
 *                                              KEY/IV[23:16]
 *                                              KEY/IV[31:24]
 *                               for KeyAttribute: 32 bits used, and should big endian
 *                               for SlotValid: only LSB 1 used, and should big endian
 *                               the following just used for SCPU and VSCPU:
 *                                                bit[3:0] is permission
 *                                                bit[8:4] is wcid
 *                                                bit[13:9] is scid
 *                                                bit[14] is sc
 *                                                bit[15] is tp
 *                                                bit[16] is enc
 *                                                bit[17] is dec
 *                                                bit[18] is audio
 * Default: 0x0
 */
#define KT_WR_DATA_31T0  (hw_kt_vbase + 0x1C)

/*
 * 31:0     read_data_127t96  RO  read data bit 127:96
 *                                big endian: IV[127:96]
 *                                little endian: IV[103:96]
 *                                               IV[111:104]
 *                                               IV[119:112]
 *                                               IV[127:120]
 * Default: 0x00061728
 */
#define KT_RD_DATA_127T96  (hw_kt_vbase + 0x20)

/*
 * 31:0     read_data_95t64  RO  read data bit 95:64
 *                               big endian: IV[95:64]
 *                               little endian: IV[71:64]
 *                                              IV[79:72]
 *                                              IV[87:80]
 *                                              IV[95:88]
 * Default: 0x00061728
 */
#define KT_RD_DATA_95T64  (hw_kt_vbase + 0x24)

/*
 * 31:0     read_data_63t32  RO  read data bit 63:32
 *                               big endian: IV[63:32]
 *                               little endian: IV[39:32]
 *                                              IV[47:40]
 *                                              IV[55:48]
 *                                              IV[63:56]
 * Default: 0x0
 */
#define KT_RD_DATA_63T32  (hw_kt_vbase + 0x28)

/*
 * 31:0     read_data_31t0  RO  read data bit 31:0
 *                              big endian: IV[31:0]
 *                              little endian: IV[7:0]
 *                                             IV[15:8]
 *                                             IV[23:16]
 *                                             IV[31:24]
 * Default: 0x0
 */
#define KT_RD_DATA_31T0  (hw_kt_vbase + 0x2C)

/*
 * 31:0     key_attribute   RO  read key attribute[31:0], it's readable when OTP_KTAttrReadDis[1:0] is 2'b00;
 *                              when OTP_KTAttrReadDis[1:0] is not 2'b00, read key_attribute[31:0] is 0xbbaa9988
 * Default: 0x0
 */
#define KT_RD_DATA_ATTR  (hw_kt_vbase + 0x30)

/*
 * 31:      kt_read_data_teecfg_chk      RO      0: tee cfg data check fail; 1: tee cfg data check pass
 * 30:1     RSV
 * 0:                                            kt_read_data_slotvalid RO  read data of slotvalid
 * Default: 0x0
 */
#define KT_RD_DATA_SLOTVALID (hw_kt_vbase + 0x34)

/*
 * 31:19    RSV
 * 18       audio RO  audio_flag
 * 17       dec   RO  0: allow decryption
 *                           1: forbid decryption
 * 16       enc   RO  0: allow encryption
 *                           1: forbid encryption
 * 15       tp     RO  0: can be used in REE
 *                     1: forbit using in REE
 * 14       sc     RO  0: RDCID shall be matched with SCID
 *                     1: don't case SCID
 * 13:9    scid     RO  protected channel id for input
 * 8:4      wcid     RO  protected channel id for write transaction
 * 3:0      permission   RO  0000: any known key source can write this entry.
 *                           others: key source from the secure domain can write this entry(TEE).
 *                           i.e, PortSCPUKey, PortCAVnKey semaphored by SCPU, PortVSCPU,
 *                           PortCAV1Key semaphored by VSCPU
 * Default: 0x0
 */
#define KT_RD_DATA_TEE   (hw_kt_vbase + 0x38)

/*
 * 31:0     kt_read_data_metadata  RO  read data bit 31:0
 *                               metadata[31:0]
 * Default: 0x0
 */
#define KT_RD_DATA_METADATA  (hw_kt_vbase + 0x3C)

/*
 * 31:0     read_key_127t96  RO  read key bit 127:96
 *                               big endian: KEY[127:96]
 *                               little endian: KEY[103:96]
 *                                              KEY[111:104]
 *                                              KEY[119:112]
 *                                              KEY[127:120]
 * Default: 0x0
 */
#define KT_RD_KEY_127T96  (hw_kt_vbase + 0x40)

/*
 * 31:0     read_key_95t64  RO  read key bit 95:64
 *                              big endian: KEY[95:64]
 *                              little endian: KEY[71:64]
 *                                             KEY[79:72]
 *                                             KEY[87:80]
 *                                             KEY[95:88]
 * Default: 0x0
 */
#define KT_RD_KEY_95T64  (hw_kt_vbase + 0x44)

/*
 * 31:0     read_key_63t32  RO  read key bit 63:32
 *                              big endian: KEY[63:32]
 *                              little endian: KEY[39:32]
 *                                             KEY[47:40]
 *                                             KEY[55:48]
 *                                             KEY[63:56]
 * Default: 0x0
 */
#define KT_RD_KEY_63T32  (hw_kt_vbase + 0x48)

/*
 * 31:0     read_key_31t0  RO  read key bit 31:0
 *                             big endian: KEY[31:0]
 *                             little endian: KEY[7:0]
 *                                            KEY[15:8]
 *                                            KEY[23:16]
 *                                            KEY[31:24]
 * Default: 0x0
 */
#define KT_RD_KEY_31T0  (hw_kt_vbase + 0x4C)

/*
 * 31:1     RSV
 * 0:       kt_key_rden_debug RW  0: key can't be read
 *                                1: key can be read
 * Default: 0x0
 */
#define KT_RD_KEY_DEBUG  (hw_kt_vbase + 0x50)

/*
 * 31:1     RSV
 * 0:       kt_tee_cfg_data_chk_lock    RW      lock signal for tee_cfg_data_chk
 * 1:                                           lock, it's one-way register and only can be configed by SecureAHBHost
 * Default: 0x0
 */
#define KT_WR_TEE_CFG_DATA_CHK_LOCK  (hw_kt_vbase + 0x54)

/*
 * 31:0     kt_tee_cfg_data_chk_31t1    RW      tee_cfg_data_chk bit[31:1]
 *                                              can be locked by tee_cfg_data_chk_lock and only can be configed by SecureAHBHost
 * 0:         kt_tee_cfg_data_chk_0         RW      tee_cfg_data_chk bit[0]
 *                                              1: invalidate keyslot while updating teecfgdata
 *                                              0: not invalidate key slot
 * Default: 0x0
 */
#define KT_WR_TEE_CFG_DATA_CHK_31t0  (hw_kt_vbase + 0x58)

/*
 * 31:4     RSV
 * 3:0      kt_tee_cfg_data_chk_35t32    RW      tee_cfg_data_chk bit[35:32]
 *                                               can be locked by tee_cfg_data_chk_lock and only can be configed by SecureAHBHost
 * Default: 0x0
 */
#define KT_WR_TEE_CFG_DATA_CHK_35t32  (hw_kt_vbase + 0x5C)

/*
 * 31:0     write_data_255t224  WO  used for KEY[255:224] or IV[255:224]
 *                                 big endian:  KEY/IV[255:224]
 *                                 little endian: KEY/IV[231:224]
 *				                  KEY/IV[239:232]
 *                                                KEY/IV[247:240]
 *                                                KEY/IV[255:248]
 * Default: 0x0
 */
#define KT_WR_DATA_255T224  (hw_kt_vbase + 0x60)

/*
 * 31:0     write_data_223t192  WO  used for KEY[223:192] or IV[223:192]
 *                                 big endian:  KEY/IV[223:192]
 *                                 little endian: KEY/IV[199:192]
 *				                  KEY/IV[207:200]
 *                                                KEY/IV[215:208]
 *                                                KEY/IV[222:216]
 * Default: 0x0
 */
#define KT_WR_DATA_223T192  (hw_kt_vbase + 0x64)

/*
 * 31:0     write_data_191t160  WO  used for KEY[191:160] or IV[191:160]
 *                                 big endian:  KEY/IV[191:160]
 *                                 little endian: KEY/IV[167:160]
 *				                  KEY/IV[175:168]
 *                                                KEY/IV[183:176]
 *                                                KEY/IV[191:184]
 * Default: 0x0
 */
#define KT_WR_DATA_191T160  (hw_kt_vbase + 0x68)

/*
 * 31:0     write_data_159t128  WO  used for KEY[159:128] or IV[159:128]
 *                                 big endian:  KEY/IV[159:128]
 *                                 little endian: KEY/IV[135:128]
 *				                  KEY/IV[143:136]
 *                                                KEY/IV[151:144]
 *                                                KEY/IV[159:152]
 * Default: 0x0
 */
#define KT_WR_DATA_159T128  (hw_kt_vbase + 0x6C)

/*
 * 31:1     RSV
 * 0:       kt_rng_clr   WO  clear Nonce SlotValid and Key before CPU want to update Nonce
 * Default: 0x0
 */
#define KT_RNG_CLR    (hw_kt_vbase + 0x78)

/*
 * 31:9     RSV
 * 8        kt_rng_nonce_teecfg_chkpass    RW    indicate tee data cfg for nonce check pass or not
 *                                               0: fail;  1: pass
 * 7:5     RSV
 * 4       kt_rng_nonce_valid              RW    indicate that curent nonce is valid or not
 *                                               1 means valid
 * 3:1     RSV
 * 0:      kt_rng_not_empty                RO    random data has arrived after reset
 *                                               0: has no valid random data, random data is 0
 *                                               1: has valid random data
 * Default: 0x0
 */
#define KT_RNG_STATUS   (hw_kt_vbase + 0x7C)

/*
 * 31:17   RSV
 * 16        k256_endian     RW    CAV2PORT 256bit key endian
 *                                               0: LSB key stored in SLOT[2n], MSB key stored in SLOT[2n+1]
 *                                               1: MSB key stored in SLOT[2n], LSB key stored in SLOT[2n+1]
 * 15:13   RSV
 * 12        k256_err_clr     RW    write '1' for clear k256_err_sta
 * 11:9     RSV
 * 8          k256_err_sta    RO
 *                                               1: indicates that index accessing 256bit key slot is invalid
 *                                               it can only be cleared by k256_err_clr
 * 7          RSV
 * 6:0      k256_err_index  RO    invalid idex for accessing 256bit key slot
 *                                               it is automatically updated every time an error occurs
 *
 * Default: 0x0
 */
#define KT_K256_INFO   (hw_kt_vbase + 0x100)

/*
 * 0x00000104 - 0x00000120
 * 128 slots, every slot 2bit reg for sw use
 * Default: 0x0
 */
#define KT_SLOT_SW_15T0   (hw_kt_vbase + 0x104)
#define KT_SLOT_SW_31T16   (hw_kt_vbase + 0x108)
#define KT_SLOT_SW_47T32   (hw_kt_vbase + 0x10C)
#define KT_SLOT_SW_63T48   (hw_kt_vbase + 0x110)
#define KT_SLOT_SW_79T64   (hw_kt_vbase + 0x114)
#define KT_SLOT_SW_95T80   (hw_kt_vbase + 0x118)
#define KT_SLOT_SW_111T96   (hw_kt_vbase + 0x11C)
#define KT_SLOT_SW_127T112   (hw_kt_vbase + 0x120)


#endif /*__HW_KT_REG_H__*/

