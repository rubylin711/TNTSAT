/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_M2M_REG_H__
#define __HW_M2M_REG_H__

#ifdef LINUX
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#define M2M_BASE SYMPHONY_IO_VA(0xBF300000)
#else
#define M2M_BASE (0xBF300000)
#endif

#define M2M_DES_MAGIC_NUM (0x4D324D44)
#define M2M_MAX_CHANNEL_NUM  (2)
#define M2M_MAX_RING_BUF_NUM (5)

/*======================================== Channel 0/1 ========================================*/

/* 31:0  CMD_QUEUE  WO  Command queue. CMD type must be 4b'0000/4b'0001;
 *                                  otherwise hardware will drop this command and raise
 *                                  error in M2M_CHn_STATE.CMD_VALID_ERROR.
 * Default: 0x xxxxxxxx
 */
#define M2M_CHn_CMD_QUEUE_REG(n) (M2M_BASE+0x0000+n*0x1000)

/* 31:0  CMD_CNT   RO  cmd queue fifo data counter, max value is 52, before write, make sure fifo not overflow
 * Default: 0x00000000
 */
#define M2M_CHn_CMD_QUEUE_CNT_REG(n) (M2M_BASE+0x0004+n*0x1000)

#define M2M_CHn_CMD_QUEUE_CNT_MAX (52)

/* 31:3  RSVD
 * 2  FORBID_VSCPU_ACCESS   RW  0:VSCPU can access
 *            1:VSCPU can not access
 *            one way register
 * 1  FORBID_TEECPU_ACCESS  RW  0: TEECPU can access
 *            1: TEECPU can not access
 *            one way register
 * 0  FORBID_REECPU_ACCESS  RW  1: REECPU can access
 *            0: REECPU can not access
 *            one way register
 * Default: 0x00000000
 */
#define M2M_CHn_FORBID_ACCESS_REG(n) (M2M_BASE+0x0008+n*0x1000)

/* 31  RSVD
 * 30:16 RCID_PERMIT  RW  0: forbidden descriptor RCID = N transaction
 *                                    1: allow descriptor RCID = N transaction
 *                                    bit[16] permission for RCID = 0
 *                                    bit[17] permission for RCID = 1
 *                                    bit[18] permission for RCID = 2
 *                                    ...
 *                                    bit[30] permission for RCID = 14
 *                                    RCID = 15 always allowed
 *                                    these field can be locked by RCID_PERMIT_LOCK, only can be config in secure mode(hprot=1)
 * 15:1  RSVD
 * 0  RCID_CFG  RW  0: accept descriptor only in one of permit rcid
 *                                    1: accept descriptor with any rcid
 *                                    this field can be one-way locked by RCID_CFG_LOCK, only can be config in secure mode(hprot=1)
 * Default: 0x00000001
 */
#define M2M_CHn_RCID_PERMIT_REG(n) (M2M_BASE+0x0010+n*0x1000)

/* 31:2  RSVD
 * 1  RCID_CFG_LOCK  RW Lock RCID_CFG
 *         1: lock
 *         0: non-lock
 *         one-way field, only can be config in secure mode(hprot=1)
 * 0  RCID_PERMIT_LOCK RW Lock RCID_PERMIT
 *         1: lock
 *         0: non-lock
 *         one-way field, only can be config in secure mode(hprot=1)
 * Default: 0x00000000
 */
#define M2M_CHn_RCID_LOCK_REG(n) (M2M_BASE+0x0014+n*0x1000)

/* 31:16 WCID_PERMIT   RW 0: forbidden WCID = N transaction
 *         1: allow descriptor RCID = N transaction
 *         bit[16] - RCID = 0
 *         bit[17] - RCID = 1
 *         ...
 *         bit[30] - RCID = 14
 *         bit[31] - RCID = 15
 *         these field can be locked by WCID_PERMIT_LOCK, only can be config in secure mode(hprot=1)
 * 15:1  RSVD
 * 0  WCID_CFG   RW 0: only WCID_PERMIT valid
 *         1: any wcid is valid
 *         this field can be one-way locked by WCID_CFG_LOCK,
 *         only can be config in secure mode(hprot=1)
 * Default: 0x00000001
 */
#define M2M_CHn_WCID_PERMIT_REG(n) (M2M_BASE+0x0018+n*0x1000)

/* 31:2  RSVD
 * 1  WCID_CFG_LOCK  RW lock WCID_CFG
 *         1: lock
 *         0: non-lock
 *         one-way field, only can be config in secure mode(hprot=1)
 * 0  WCID_PERMIT_LOCK RW lock WCID_PERMIT
 *         1: lock
 *         0: non-lock
 *         one-way field, only can be config in secure mode(hprot=1)
 * Default: 0x00000000
 */
#define M2M_CHn_WCID_LOCK_REG(n) (M2M_BASE+0x001C+n*0x1000)

/* 31  AXI_PROT_LOCK  RW  AXI Master Prot Lock
 *          1:lock
 *          one-way
 * 30:6  RSVD
 * 5:3  AXI_PROT_DESC  RW  Channel AXI Descriptor Prot only can configure in AXI_PROT_LOCK unlock state
 * 0  AXI_PROT_DATA  RW  Channel AXI Descriptor Prot only can configure in AXI_PROT_LOCK unlock state
 * Default: 0x00000000
 */
#define M2M_CHn_AXI_PROT_REG(n) (M2M_BASE+0x0020+n*0x1000)

/* 31       M2MKEY_SRCOPTION_LOCK   RW  Lock for M2MKEY_SRCOPTION.
 *                                      Default is 0x0.
 * 30:10    RSVD
 * 9:0      M2MKEY_SRCOPTION        RW  KEYSource restriction register.
 *                                      Default is 0x033.
 *                                      [0] 0: mask ACPUKEY (keyattribute[31:28]=4'b0000)
 *                                      [1] 0: mask SCPUKEY (keyattribute[31:28]=4'b0001)
 *                                      [2] 0: mask VSCPUKE (keyattribute[31:28]=4'b0010)
 *                                      [3] 0: mask CAV1KEY (keyattribute[31:28]=4'b0011)
 *                                      [4] 0: mask CAV1KEY (keyattribute[31:28]=4'b0100)
 *                                      [5] 0: mask CAV1KEY (keyattribute[31:28]=4'b0101)
 *                                      [6] 0: mask CAV2KEY (keyattribute[31:28]=4'b0110)
 *                                      [7] 0: mask CAV2KEY (keyattribute[31:28]=4'b0111)
 *                                      [8] 0: mask CAV3KEY (keyattribute[31:28]=4'b1000)
 *                                      [9] 0: mask CAV3KEY (keyattribute[31:28]=4'b1001)
 */
#define M2M_CHn_KEYSRC_OPT(n) (M2M_BASE+0x0024+n*0x1000)

/* 31:24 TASK_ID    RO TASK ID of command done
 * 23  CMD_VALID_ERR  RO for channel 0/1, CMD must be 4b0000/4b0001
 *         1:ERR
 *         0:NO ERR
 * 22  DBUF_WBACK_WCID_ERR RO Dbuf write back to memory, WCID is forbidden.
 * 21  DESC_ADDR_ERR  RO descriptor address is not allowed
 * 20  DESC_RCID_ERR  RO descriptor RCID is not allowed
 * 19  DBUF_W_LEN_ERR  RO data destination len is 0
 * 18  DBUF_W_ADDR_ERR  RO data destination address is not allowed
 * 17  DBUF_R_LEN_ERR  RO data source length is 0
 * 16  DBUF_R_ADDR_ERR  RO data source address is not allowed
 * 15       REE_CTR_DIS         RO  REE CTR/CTR64/GCM is forbidden
 * 14       TS_nopayload_data0  RO  TS error type: TS PID filter fail; TS head error without "0x47"
 * 13       CLRDATA_RAM_REE_ERR RO  Indicate that output wcid/data of Clear Data/Head Clear/ Tail Clear/LOS is forced to 0, according to M2M REE CHs & RCID!=0 & RCID!=WCID
 * 12  DBUF_Source_RCID_ERR1 RO data source RCID is forbidden by M2M_CHn_RCID_PERMIT_REG set
 * 11  DBUF_Source_RCID_ERR0 RO data source RCID check err
 * 10  KT_ACPU_FORB  RO key is not for REE-CPU, according to KTKeySlot.KeyAttr[17] and KTKeySlot.TP
 * 9  KT_MAC_DIS   RO key is not for MAC use
 * 8  KT_M2M_DIS   RO key is not for M2M use
 * 7  KT_DEC_DIS   RO key is not for decryption
 * 6  KT_ENC_DIS   RO key is not for encryption
 * 5  KEYSIZE_FAIL  RO keysize is not matched
 * 4  KEYUSAGE_FAIL  RO keyusage is not matched
 * 3  KEY_UNVLD   RO key is not valid
 * 2  TDES_KEY_CHKFAIL RO TDES key check is failed
 * 1  PRF_UNDEF   RO Profile is unknow
 * 0  ALGO_DIS   RO algoriyhm is not supported according to OTP
 * Default: 0xFFFFFFFF
 * if command state fifo is empty, return 0xFFFFFFFF
 */
#define M2M_CHn_STATE_RING_BUF(n, RING_NUM) (M2M_BASE+0x0030+n*0x1000+RING_NUM*0x4)

/* 31:16    RSVD
 * 15:8     TASK_ID             RO  Task ID of executed command.
 *  7:0     DESCRIPTOR_ID       RO  The consequence number of descriptor has done in Task ID command. Only valid for CMD1. The number increase by "1" and can be turned around to 0 if overflow. If current command is down and new command has come, these bits will be cleared.
 * Default: 0x00000000
 */
#define M2M_CHn_STATE_DESC(n) (M2M_BASE+0x0044+n*0x1000)

/* 31:4     RSVD
 * 3        CTR_GCM_DEC_REE_DIS RW  Disable REE CH CTR decrypt algorithm. Only can be written by FWHost.
 * 2        CTR_GCM_ENC_REE_DIS RW  Disable REE CH CTR encrypt algorithm. Only can be written by FWHost.
 * 1        DEC_FORCE           RW  1: Force TS packet decryption, even if SCB=0. Only can be written by FWHost.
 * 0        ENC_FORCE           RW  1: Force TS packet encryption, even if SCB=2/3. Only can be written by FWHost.
 * Default: 0x00000000
 */
#define M2M_CHn_ENC_DEC_FORCE(n) (M2M_BASE+0x0048+n*0x1000)

/* 31       RD_ALLOW_LOCK       RW  One-way register. Once write 1 to this bit, RD_ALLOW can not be modified any more. Only can be configured by FWHost.
 * 30:1     RSVD
 * 0        RD_ALLOW            RW  CMD/DESC EXTR register access permission.
 *                                  1: allow
 *                                  0: forbidden
 *                                  Only can be configured by FWHost.
 * Default: 0x00000001
 */
#define M2M_CHn_EXTR_RD_ALLOW(n) (M2M_BASE+0x004c+n*0x1000)

/* 31:24 RSVD
 * 23       REE_CTR_DIS         RO  REE CTR/CTR64/GCM is forbidden
 * 22       CLRDATA_RAW_REE_ERR RO  Indicate that output wcid/data of Clear Data/Head Clear/ Tail Clear/LOS is forced to 0, according to M2M REE CHs & RCID!=0 & RCID!=WCID
 * 21  DATA_RCID_ERR  RO DATA RCID FORBIDDEN
 * 20  SCID_CHK_ERR  RO KTKeySlot.SCID/M2MCmd.SCID is not equal with RDCID
 * 19  KEYSRC_OPT_DIS      RO  Keysource is disabled by REG_KEYSRC_OPT
 * 18  KT_DES_KSIZE_ERR RO DES KEY SIZE mismatch
 * 17  KT_DES_DIS   RO M2M DES algorithm is disabled refer to kt_attribute
 * 16  ALGO_OTPDIS   RO Algorithm disabled by OTP
 * 15  TP_ERR    RO TP ERR
 * 14  TDESKEY_CHK_ERR  RO TDES KEY CHECK FAIL
 * 13  DK_PRF_UNDEF  RO M2MCmd.Profile is undefined while dkey is used
 * 12  KT_PRF_UNDEF  RO Profile is undefined in kt
 * 11  KT_ACPU_FORB  RO ACPU is forbidden
 * 10  KT_ENC_DIS   RO Field0000 Abstract. ENC is disabled refer to kt_attribute
 * 9  KT_DEC_DIS   RO DEC is disabled refer to kt_attribute
 * 8  KT_MAC_DIS   RO MAC algorithm is disabled refer to kt_attribute
 * 7  KT_M2M_DIS   RO M2M algorithm is disabled refer to kt_attribute
 * 6  KT_TDES_KSIZE_ERR RO TDES KEY SIZE mismatch
 * 5  KT_AES_KSIZE_ERR RO AES KEY SIZE mismatch
 * 4  KT_TDES_DIS   RO M2M TDES algorithm is disabled refer to kt_attribute
 * 3  KT_AES_DIS   RO M2M AES algorithm is disabled refer to kt_attribute
 * 2  KT_AES256_ERR  RO KTKey of M2M ALGO is AES256
 * 1  KT_AES192_ERR  RO KTKey of M2M ALGO is AES192
 * 0  KT_SLOT_UNVLD  RO KTKeySlot.SlotValid is not equal with 1
 * Default: 0x00000000
 */
#define M2M_CHn_TS_ERR0_REG(n) (M2M_BASE+0x0060+n*0x1000)

/* 31:5  RSVD
 * 6  TS_NOPLAYLOAD_TYPE2 RO TS head error without 0x47
 * 5  TS_NOPLAYLOAD_TYPE1 RO TS PID filter failed
 * 4  TS_NOPLAYLOAD_TYPE0 RO No payload
 * 3  RSVD
 * 2:0  STATE_ALGO   RO Algorithm statemachine
 *         0: IDLE
 *         1: HASH_REQ
 *         2: AD_REQ
 *         3: GHASH_REQ
 *         4: AD_GHASH_REQ
 * Default: 0x00000000
 */
#define M2M_CHn_TS_ERR1_REG(n) (M2M_BASE+0x0064+n*0x1000)

/*  31:0 the total clock count one command takes.
 *   for cmd0: whole command
 *   for cmd1: only first descriptor
 */
#define M2M_CHn_PERF_REG(n) (M2M_BASE+0x0068+n*0x1000)

/*  31:5  RSVD
 *  4  GOST_BIT_INV ordering type for storage access in gost28147
 *  2:0  GOST SBOX selection
 */
#define M2M_CHn_GOST_MODE_REG(n) (M2M_BASE+0x006c+n*0x1000)

/* CMD0 IV 输入
 * 127:0 DIV     WO Address 90H:DIV[31:0]
 *         ...
 *         Address 9CH:DIV[127:96]
 * Default: 0x00000000
 * Function: DirectIV register for CMD0, hex address from 0x0090~0x009c(n=0~1)
 */
#define M2M_CHn_CMD0_REG_DIV_REG(n) (M2M_BASE+0x0090+n*0x1000)

/* HASH分组，上一组的结果作为下一组的输入
 * 511:0 TEMP_IN    WO Address A0H:TEMP_IN[31:0]
 *         ...
 *         Address DCH:TEMP_IN[511:480]
 * Default: 0x00000000
 * Function: Temporary result register for CMD0, hex address from 0x00a0~0x00dc(n=0~1)
 */
#define M2M_CHn_CMD0_REG_TEMPIN_REG(n) (M2M_BASE+0x00A0+n*0x1000)

/* 31:0  CMD_EXTRACT   RO CMD extract from(copy) in FIFO, currently CMD now in processing state or previous cmd have been done
 * Default: 0x xxxxxxxx
 * hex address from 0x0100~0x012c(n=0~1)
 * for debug usage
 */
#define M2M_CHn_CMD_EXTR_REG(n) (M2M_BASE+0x0100+n*0x1000)

/* 31:0  DESC_EXTRACT  RO current descriptor now in processing state or previous descriptor have been done
 * Default: 0x xxxxxxxx
 * hex address from 0x0134~0x01CC(n=0~1)
 * for debug usage
 */
#define M2M_CHn_DESC_EXTR_REG(n) (M2M_BASE + 0x0134 + (mt_u32)(n) * 0x1000)

/* IV中间结果
 * 31:0  IV_CONT    RO IV content, if CMD done or Descriptor done, this register data is the result of AES/TDES IV
 * Default: 0x xxxxxxxx
 * hex address from 0x01D0~0x01DC(n=0~1)
 */
#define M2M_CHn_IV_CONT_REG(n) (M2M_BASE + 0x01D0 + (mt_u32)(n) * 0x1000)

/* HASH中间结果
 * 31:0  GHASH_CONT   RO GHASH content, if CMD done or descriptor done, this register data is the result of HASH/GHASH/CMAC
 * Default: 0x xxxxxxxx
 * hex address from 0x01E0~0x021C(n=0~1)
 */
#define M2M_CHn_GHASH_CONT_REG(n) (M2M_BASE + 0x01E0 + (mt_u64)(n) * 0x1000)

/* 最终IV结果
 * 31:0  TEMPOUT    RO IV CONT, if CMD done, this register data is the result of AES/TDES IV
 * Default: 0x xxxxxxxx
 * hex address from 0x0300~0x030c(n=0~1, RING_NUM=0~4)
 */
#define M2M_CHn_TEMPOUT0_REG(n, RING_NUM) (M2M_BASE+0x0300+n*0x1000+RING_NUM*0x58)

/* 最终HASH结果
 * 31:0  TEMPOUT    RO HASH/HMAC:output
 *         Others:GhashCount
 * Default: 0x xxxxxxxx
 * hex address from 0x0310~0x034c(n=0~1, RING_NUM=0~4)
 */
#define M2M_CHn_TEMPOUT1_REG(n, RING_NUM) (M2M_BASE+0x0310+n*0x1000+RING_NUM*0x58)

/* 最终HASHSUM结果
 * 31:0  TEMPOUT    RO HASHSUM:output
 * Default: 0x xxxxxxxx
 * hex address from 0x0350(n=0~1, RING_NUM=0~4)
 */
#define M2M_CHn_TEMPOUT2_REG(n, RING_NUM) (M2M_BASE+0x0350+n*0x1000+RING_NUM*0x58)

/* 最终LOS/LCRYPTOB/LSKIPB结果
 * 31:0  TEMPOUT    RO LOS/LCRYPTOB/LSKIPB:output
 * Default: 0x xxxxxxxx
 * hex address from 0x0354(n=0~1, RING_NUM=0~4)
 */
#define M2M_CHn_TEMPOUT3_REG(n, RING_NUM) (M2M_BASE+0x0354+n*0x1000+RING_NUM*0x58)

/* 31:3 RSVD
 * 2    REE_LEN_EN              RW      0: not use length limited by REE_LEN_SEL
 *                                      1: use length limited by REE_LEN_SEL
 *                                      Only used in ch0/1 for REE.
 *                                      ONly can configured by FWHost.
 * 1:0  REE_LEN_SEL             RW      00: protect/clear size limited to 256MB (not included)
 *                                      01: protect/clear size limited to 64MB (not included)
 *                                      10: protect/clear size limited to 8MB (not included)
 *                                      11: protect/clear size limited to 1MB (not included)
 *                                      Only take affect in REE_LEN_EN is 1.
 *                                      If exceed selected length, the both size clap to "0".
 *                                      Only used in ch0/1 for REE.
 *                                      Only can configured by FWHost. If channel accessed forbidden, it remain can be configured.
 * Default: 0x00000000
 *
 */
#define M2M_CHn_REE_LEN_LIMIT(n) (M2M_BASE+0x0800+n*0x1000)


/*======================================== BGC Channel ========================================*/
/* 31:4  RSVD
 * 3:0  SEMAPHORE   RW  before config BGC, CPU must request semaphore firstly
 *          before request, CPU need to read this register to check if BGC is IDLE state(value=4'b0000).
 *          If it is, then write these bits{1'b1, CPUID}, where CPUID list below:
 *          3'b000:APCPU(REE)
 *          3'b101:SECCPU(TEE)
 *          3'b100:VSCPU
 *          after write this register then read again to check if it is the value CPU just has written.
 *          If right, the CPU get the access authority.
 *          After all configuration done, CPU should release semaphore by writing 4'b0000.
 * Default: 0x00000000
 */
#define M2M_BGC_SEMAPHORE_REG (M2M_BASE+0x2000)

/* 31:2  RSVD
 * 1:0  SET_SEL    RW  which channel will be updated by writing this register
 *          00:Command SET0
 *          01:Command SET1
 *          10:Command SET2
 *          11:Command SET3
 *          Only can be written after got semaphore.
 *          If read, access directly.
 * Default: 0x00000000
 */
#define M2M_BGC_SET_SEL_REG (M2M_BASE+0x2004)

/* 31:3  RSVD
 * 2  VSCPU_FORBID  RW  0: access allow
 *          1: access forbidden
 *          one-way
 * 1  TEE_FORBID   RW  0: access allow
 *          1: access forbidden
 *          one-way
 * 0  REE_FORBID   RW  0: access allow
 *          1: access forbidden
 *          one-way
 * Default: 0x00000000
 */
#define M2M_BGC_ACCESS_FORBID_REG (M2M_BASE+0x2008)

/* 31  AXI_PROT_LOCK  RW  AXI Master Prot Lock
 * 30:1  RSVD
 * 0  AXI_PROT   RW  AXI bus protect signal,  only can configure in AXI_PROT_LOCK unlock state
 * Default: 0x00000000
 */
#define M2M_BGC_AXI_PROT_REG (M2M_BASE+0x200C)

/* 31:4  RSVD
 * 3  CMD_SET3_VALID  RW  CMD_SET3 valid
 *          1: valid
 *          0: invalid
 *          only can be written after got semaphore & SET_SEL is 11 & SET3_LOCK is deactive
 *          If read, access directly.
 * 2  CMD_SET2_VALID  RW  CMD_SET2 valid
 *          1: valid
 *          0: invalid
 *          only can be written after got semaphore & SET_SEL is 10 & SET2_LOCK is deactive.
 *          If read, access directly.
 * 1  CMD_SET1_VALID  RW  CMD_SET1 valid
 *          1: valid
 *          0: invalid
 *          only can be written after got semaphore & SET_SEL is 01 & SET1_LOCK is deactive.
 *          If read, access directly.
 * 0  CMD_SET0_VALID  RW  CMD_SET0 valid
 *          1: valid
 *          0: invalid
 *          only can be written after got semaphore & SET_SEL is 00 & SET0_LOCK is deactive.
 *          If read, access directly.
 * Default: 0x00000000
 */
#define M2M_BGC_VALID_SET_REG (M2M_BASE+0x2014)

/* 31:1  RSVD
 * 0:  UPDATE_ALLOW  RO  1: Allow update
 *          0: Not allow
 *          after got semaphore, before update command set, cpu need to read this register.
 * Default: 0x00000001
 */
#define M2M_BGC_UPDATE_ALLOW_REG (M2M_BASE+0x2018)

/* 31:4  RSVD
 * 3  STATE_CLR_3   WO  clear command set3 checking state
 *          1: clear
 *          0: no effect
 * 2  STATE_CLR_2   WO  clear command set2 checking state
 *          1: clear
 *          0: no effect
 * 1  STATE_CLR_1   WO  clear command set1 checking state
 *          1: clear
 *          0: no effect
 * 0  STATE_CLR_0   WO  clear command set0 checking state
 *          1: clear
 *          0: no effect
 * Default: 0x00000000
 */
#define M2M_BGC_STATE_CLEAR_REG (M2M_BASE+0x2020)

/* 31  FAIL_STATE_3  RO  1:BGC command set3 checking failed
 *               0:successful
 * 30:24 CNT_STATE_3   RO  BGC command set3 checking done times
 * 23  FAIL_STATE_2  RO  1:BGC command set2 checking failed
 *             0:successful
 * 22:16 CNT_STATE_2   RO  BGC command set2 checking done times
 * 15  FAIL_STATE_1  RO  1:BGC command set1 checking failed
 *             0:successful
 * 14:8  CNT_STATE_1   RO  BGC command set1 checking done times
 * 7  FAIL_STATE_0  RO  1:BGC command set0 checking failed
 *             0:successful
 * 6:0  CNT_STATE_0   RO  BGC command set0 checking done times
 * Default: 0x00000000
 */
#define M2M_BGC_STATE_REG (M2M_BASE+0x2024)

/* 31:16 RSVD
 * 15:12 SET3_LOCK   RW  SET3 Locked
 * 11:8  SET2_LOCK   RW  SET2 Locked
 * 7:4  SET1_LOCK   RW  SET1 Locked
 * 3:0  SET0_LOCK   RW  SET0 Locked
 *          when locked,all register corresponding command set0 can not be updated
 *          (refer to corresponding register description)
 *          any bit is 1 locked.
 *          one-way, only can be configured in got semaphore state.
 *          SET1/2/3 the same
 * Default: 0x00000000
 */
#define M2M_BGC_LOCK_REG (M2M_BASE+0x2030)

/* 31:0  BGC_CMD   RW  command set0
 *         only can be configured in having got semaphore state
 *         & M2M_BGC_SET_SEL is 0
 *         & M2M_BGC_UPDATE_ALLOW is active
 *         & command set0 is unlock
 *         only can be read in having got semaphored state.
 * Default: 0x xxxxxxxx
 * Function: BGC Command Set 0, hex address from 0x2100~0x212c
 */
#define M2M_BGC_SET0_REG (M2M_BASE+0x2100)

/* 31:0  BGC_CMD   RW  command set1
 *         only can be configured in having got semaphore state
 *         & M2M_BGC_SET_SEL is 1
 *         & M2M_BGC_UPDATE_ALLOW is active
 *         & command set1 is unlock
 *         only can be read in having got semaphored state.
 * Default: 0x xxxxxxxx
 * Function: BGC Command Set 1, hex address from 0x2130~0x215c
 */
#define M2M_BGC_SET1_REG (M2M_BASE+0x2130)

/* 31:0  BGC_CMD   RW  command set2
 *         only can be configured in having got semaphore state
 *         & M2M_BGC_SET_SEL is 2
 *         & M2M_BGC_UPDATE_ALLOW is active
 *         & command set2 is unlock
 *         only can be read in having got semaphored state.
 * Default: 0x xxxxxxxx
 * Function: BGC Command Set 2, hex address from 0x2160~0x218c
 */
#define M2M_BGC_SET2_REG (M2M_BASE+0x2160)

/* 31:0  BGC_CMD   RW  command set3
 *         only can be configured in having got semaphore state
 *         & M2M_BGC_SET_SEL is 3
 *         & M2M_BGC_UPDATE_ALLOW is active
 *         & command set3 is unlock
 *         only can be read in having got semaphored state.
 * Default: 0x xxxxxxxx
 * Function: BGC Command Set 3, hex address from 0x2190~0x21bc
 */
#define M2M_BGC_SET3_REG (M2M_BASE+0x2190)

#define M2M_BGC_SETn_REG(n) (M2M_BGC_SET0_REG + (ulong)(n) * 0x30)


/* 31:11 RSVD
 * 10:0  M2M_CLK_HWCG  RW
 *       clock gating switch. 1" means close clock gating funtion; "0" means
 *       open clock gating function. 
 *
 *       bit0: AES algorithm clock gating switch
 *       bit1: TDES/DES algorithm clock gating switch
 *       bit2: GHASH algorithm clock gating switch
 *       bit3: HASH algorithm clock gating switch
 *       bit5: DBUF clock gating switch
 *       other bits: other logic clock gating switch
 *
 * Default: 0x000007ff
 */
#define M2M_HWCG_MODE_REG (M2M_BASE+0x3010)

/*======================================== Interrupt ========================================*/
/* 31:1  RSVD
 * 0  CID_INT_CLR   WO 0:no affect
 *         1:clear CID error interrupt
 *         no need to write back to 0, hardware will write back to 0 automatically
 *         only can be configured by FWHost
 * Default: 0x00000000
 */
#define M2M_CID_INT_CLEAR_REG (M2M_BASE+0x3034)

/* 31:6  RSVD
 * 5:4  CID_ERR_CHL   RO CID error occurs in which channel
 *         00:ch0
 *         01:ch1
 *         10:bgc
 *         others:rsv
 * 3  CID_ERR_3   RO Dbuf write back to memory, WCID is forbidden.
 * 2  CID_ERR_2   RO descriptor RCID is not allowed.
 * 1  CID_ERR_1   RO data source RCID is forbidden by M2M_CHn_RCID_PERMIT_REG set.
 * 0  CID_ERR_0   RO data source RCID check err.
 * Default: 0x00000000
 */
#define M2M_CID_INT_FLAG_REG (M2M_BASE+0x3038)

/* 31:8  RSVD
 * 7:  BGC_SET3_INT_CLR  WO BGC Command Set3 error interrupt clear.
 * 6:  BGC_SET2_INT_CLR  WO BGC Command Set2 error interrupt clear.
 * 5:  BGC_SET1_INT_CLR  WO BGC Command Set1 error interrupt clear.
 * 4:  BGC_SET0_INT_CLR  WO BGC Command Set0 error interrupt clear.
 * 3:  DESC_DONE_INT_CLR_CH1 WO CH1 descriptor done interrupt clear.
 * 2:  CMD_DONE_INT_CLR_CH1 WO CH1 Command done interrupt clear.
 * 1:  DESC_DONE_INT_CLR_CH0 WO CH0 descriptor done interrupt clear.
 * 0:  CMD_DONE_INT_CLR_CH0 WO CH0 Command done interrupt clear.
 *          0:no affect
 *          1:clear, no need to write back to 0, hardware will write back to 0 automatically.
 *          REE/TEE/VSCPU share a common address space,
 *          only cpu which send the command can receive the interrupt and can clear it.
 *          the same to bit[7:1]
 * Default: 0x00000000
 */
#define M2M_INT_CLEAR_REG (M2M_BASE+0x303C)

/* 31:8  RSVD
 * 7:  BGC_SET3_INT   RO BGC Command Set3 error interrupt.
 * 6:  BGC_SET2_INT   RO BGC Command Set2 error interrupt.
 * 5:  BGC_SET1_INT   RO BGC Command Set1 error interrupt.
 * 4:  BGC_SET0_INT   RO BGC Command Set0 error interrupt.
 * 3:  DESC_DONE_INT_CH1  RO CH1 descriptor done interrupt.
 * 2:  CMD_DONE_INT_CH1  RO CH1 Command done interrupt.
 * 1:  DESC_DONE_INT_CH0  RO CH0 descriptor done interrupt.
 * 0:  CMD_DONE_INT_CH0  RO CH0 Command done interrupt.
 *          1:interrupt occurs
 *          0:no interrupt
 *          REE/TEE/VSCPU share a common address space,
 *          only cpu which send the command can receive the interrupt.
 *          the same to bit[7:1]
 * Default: 0x00000000
 */
#define M2M_INT_FLAG_REG (M2M_BASE+0x3040)

#endif /*__HW_M2M_REG_H__*/
