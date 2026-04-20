/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_KT_REG_H__
#define __HW_KT_REG_H__


//===========================================key table===========================================

#define REG_KEYTABLE_BASE				0xBF304000
#define REG_KEYTABLE_OPERATION      	REG_KEYTABLE_BASE
#define REG_KEYTABLE_START          		(REG_KEYTABLE_BASE + 0x4)
//31:1	RSV
//0		big_endian	RW		0:little endian
//							1:big endian
//							suggest:
//							for writing:	1,for SlotValid and KeyAttribute, this bit should be 1
//										2,for Key and IV, this bit should be 0
//							for reading:	this bit should be 0
#define REG_KEYTABLE_ENDIAN_CONFIG	(REG_KEYTABLE_BASE + 0x8)
#define REG_KEYTABLE_WRITE_DATA0		(REG_KEYTABLE_BASE + 0x10)
#define REG_KEYTABLE_WRITE_DATA1		(REG_KEYTABLE_BASE + 0x14)
#define REG_KEYTABLE_WRITE_DATA2		(REG_KEYTABLE_BASE + 0x18)
#define REG_KEYTABLE_WRITE_DATA3		(REG_KEYTABLE_BASE + 0x1c)

#define REG_KEYTABLE_READ_DATA0		(REG_KEYTABLE_BASE + 0x20)
#define REG_KEYTABLE_READ_DATA1		(REG_KEYTABLE_BASE + 0x24)
#define REG_KEYTABLE_READ_DATA2		(REG_KEYTABLE_BASE + 0x28)
#define REG_KEYTABLE_READ_DATA3		(REG_KEYTABLE_BASE + 0x2c)

//16:0		Key Usage		[0]:	can be used on AES
//							[1]:	can be used on DES
//							[2]:	can be used on TDES
//							[3]:	can be used on CSAv2
//							[4]:	can be used on CSAv3
//							[5]:	can be used on SM2/3/4
//							[6]:	can be used on HMAC
//							[7]:	can be used on M2M(PVR)
//							[8]:	can be used on ASA
//							[9]:	can be used on Multi2
//							[10]:reserved
//							[11]:reserved
//							[12]:reserved
//							[13]:reserved
//							[14]:reserved
//							[15]:reserved
//							[16]:reserved
//17		M2M Key User		M2M perspective
//							0: can be used in REE
//							1: forbid using in REE
//19:18		Enc/Dec			[18]	0:decryption operation is not allowed
//								1:decryption operation is allowed
//							[19]	0:encryption operation is not allowed
//								1:encryption operation is allowed
//21:20		Key Size			size of the associated key:
//							00: 64-bit
//							01: 128-bit
//							10: 192-bit
//							11: 256-bit
//24:22		KeySource		000:HostCPU
//							001:SCPU
//							010:AKL
//							011:CW KL
//							100:PVR KL
//							Others:reserved
//26:25		TDES key check	00:no check
//							01:check Akey=Bkey with parity bits
//							10:check Akey=Bkey without parity bits
//							11:reserved
//31:27		Reserved
#define REG_KEYTABLE_KEYATTRIBUTE	(REG_KEYTABLE_BASE + 0x30)
typedef union _HW_KT_KEY_ATTR_REG
{
	mt_u32 all;
	struct {
	mt_u32 AES_ONOFF:1;
	mt_u32 DES_ONOFF:1;
	mt_u32 TDES_ONOFF:1;
	mt_u32 CSAv2_ONOFF:1;
	mt_u32 CSAv3_ONOFF:1;
	mt_u32 SM2_3_4_ONOFF:1;
	mt_u32 HMAC_ONOFF:1;
	mt_u32 M2M_ONOFF:1;
	mt_u32 ASA_ONOFF:1;
	mt_u32 Multi2_ONOFF:1;
	mt_u32 :7;
	mt_u32 REE_ONOFF:1;
	mt_u32 DEC_ONOFF:1;
	mt_u32 ENC_ONOFF:1;
	mt_u32 KEY_SIZE:2;
	mt_u32 KEY_SOURCE:3;
	mt_u32 TDES_KEYCHK:2;
	mt_u32 :5;
	} bitc;
}HW_KT_KEY_ATTR_REG;

#define REG_KEYTABLE_KEYSLOVALID    	(REG_KEYTABLE_BASE + 0x34)

#define REG_KEYTABLE_KEY_DATA0		(REG_KEYTABLE_BASE + 0x40)
#define REG_KEYTABLE_KEY_DATA1		(REG_KEYTABLE_BASE + 0x44)
#define REG_KEYTABLE_KEY_DATA2		(REG_KEYTABLE_BASE + 0x48)
#define REG_KEYTABLE_KEY_DATA3		(REG_KEYTABLE_BASE + 0x4c)

#define REG_KEYTABLE_KEY_DEBUG		(REG_KEYTABLE_BASE + 0x50)

#define REG_KEYTABLE_KEYDELIVERY_START	(REG_KEYTABLE_BASE + 0x60)
#define REG_KEYTABLE_KEYDELIVERY_STATUS	(REG_KEYTABLE_BASE + 0x64)
#define REG_KEYTABLE_KEYDELIVERY_INFO	(REG_KEYTABLE_BASE + 0x68)


#endif	/*__HW_KT_REG_H__*/

