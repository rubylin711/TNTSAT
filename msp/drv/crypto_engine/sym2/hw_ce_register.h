/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_CE_REG_H__
#define __HW_CE_REG_H__



#define CE_BASE			0xBF300000

#define CRYPTO_DMA_CHn_WSLV(n)		(CE_BASE+0x0000+n*0x100)
#define CRYPTO_DMA_CHn_RSLV(n)		(CE_BASE+0x0400+n*0x100)

//31:8		RSV
//7:0		DMA_CHn_IBUF_SPACE		R		cpu方式对应DBUF剩余空间，单位byte
#define CRYPTO_DMA_CHn_IBUF(n)		(CE_BASE+0x800+n*0x10)

//31:8		RSV
//7:0		DMA_CHn_OBUF_CNT				cpu方式处理有效数据，单位byte
#define CRYPTO_DMA_CHn_OBUF(n)		(CE_BASE+0x0880+n*0x10)

//31:12		RSV
//11:8		SW_SRC_ENDIAN_CFG		RW		cpu模式通道读大小端配置
//											1:小端模式
//											0:大端模式
//											BIT[8]	对应通道0
//											BIT[9]	对应通道1
//											BIT[10]	对应通道2
//											BIT[11]	对应通道3
//7:4		SW_DST_ENDIAN_CFG		RW		cpu模式通道写大小端配置
//											1:小端模式
//											0:大端模式
//											BIT[4]	对应通道0
//											BIT[5]	对应通道1
//											BIT[6]	对应通道2
//											BIT[7]	对应通道3
// 3:1		RSV
//0			DMA_BUS_MODE_CFG		RW		DMA通道总线写处理模式
//											0:wr_done
//											1:wr_last
//Default: 0x000000f0
#define CRYPTO_DMA_BUS_MODE		(CE_BASE+0x0900)
typedef union _HW_CE_BUS_MODE_REG
{
  mt_u32 all;
  struct {
  mt_u32 DMA_BUS_MODE_CFG:1;
  mt_u32 :3;
  mt_u32 SW_DST_ENDIAN_CFG:4;
  mt_u32 SW_SRC_ENDIAN_CFG:4;
//  mt_u32 SW_DST_ENDIAN_CFG_CH0:1;
//  mt_u32 SW_DST_ENDIAN_CFG_CH1:1;
//  mt_u32 SW_DST_ENDIAN_CFG_CH2:1;
//  mt_u32 SW_DST_ENDIAN_CFG_CH3:1;
//  mt_u32 SW_SRC_ENDIAN_CFG_CH0:1;
//  mt_u32 SW_SRC_ENDIAN_CFG_CH1:1;
//  mt_u32 SW_SRC_ENDIAN_CFG_CH2:1;
//  mt_u32 SW_SRC_ENDIAN_CFG_CH3:1;
  mt_u32 :20;
  } bitc;
} HW_CE_BUS_MODE_REG;

//31:29		RSV
//28:0		DMA_CHn_SRC			channelN 数据源起始地址，单位8byte(配置内存地址需除以8)
#define CRYPTO_DMA_CHn_SRC(n)			(CE_BASE+0x1000+n*0x100)

//31:0		DMA_CHn_LEN			channelN 数据块长度单位byte
#define CRYPTO_DMA_CHn_LEN(n)			(CE_BASE+0x1004+n*0x100)

//31:17		RSV
//28:0		DMA_CHn_DST			channelN 数据源目的地址，单位8byte(配置内存地址需除以8)
#define CRYPTO_DMA_CHn_DST(n)			(CE_BASE+0x1008+n*0x100)


//31:21		RSV
//20		DMA_CHn_OUT_EN			RW		channelN数据处理结果输出使能，高有效
//19:17		RSV
//16		DMA_CHn_MODE_CFG		RW		channelN模式配置0:DMA模式1:CPU模式
//15:14		DMA_CHn_DST_ENDIAN_CFG	RW		channelN目的数据大小端配置
//											bit0:32位数据大小端设置0:小端1:大端
//											bit1:64位数据32单位数据大小端设置0:小端1:大端
//13:12		DMA_CHn_SRC_ENDIAN_CFG	RW		channelN源数据大小端配置
//											bit0:32位数据大小端设置0:小端1:大端
//											bit1:64位数据32单位数据大小端设置0:小端1:大端
//11:9		RSV
//8			DMA_CHn_HOLD			RW		channelN暂停使能	高有效
//7:5		RSV
// 4			DMA_CHn_CANCEL			RW		channelN取消使能	高有效
// 3:1		RSV
//0			DMA_CHn_ENABLE			RW		channelN开启使能	高有效
#define CRYPTO_DMA_CHn_CFG(n)			(CE_BASE+0x100C+n*0x100)
typedef union _HW_CE_CHANNEL_CFG_REG
{
  mt_u32 all;
  struct {
  mt_u32 DMA_CHn_ENABLE:1;
  mt_u32 :3;
  mt_u32 DMA_CHn_CANCEL:1;
  mt_u32 :3;
  mt_u32 DMA_CHn_HOLD:1;
  mt_u32 :3;
  mt_u32 DMA_CHn_SRC_ENDIAN_CFG:2;
  mt_u32 DMA_CHn_DST_ENDIAN_CFG:2;
  mt_u32 DMA_CHn_MODE_CFG:1;
  mt_u32 :3;
  mt_u32 DMA_CHn_OUT_EN:1;
  mt_u32 :11;
  } bitc;
} HW_CE_CHANNEL_CFG_REG;

//31:12		RSV
//11:8		DBUF_IDLE				R		DBUF通道N空闲有效状态，高有效
//7:5		RV
// 4			DMA_CHn_HOLD_VLD		R		channelN暂停有效状态，高有效
// 3:1		RSV	
//0			DMA_CHn_BUSY			R		channelN空闲状态0:IDLE	1:BUSY
#define CRYPTO_DMA_CHn_STATE(n)		(CE_BASE+0x1018+n*0x100)

//AES/DES 分组标志
// 31:2		RSV
// 1			LAST_GROUP			RW		AES/DES最后一组操作
// 0			FIRST_GROUP			RW		AES/DES第一组操作
#define CRYPTO_DMA_CHn_GRP(n)			(CE_BASE+0x101C+n*0x100)
typedef union _HW_CE_CHANNEL_GRP_REG
{
  mt_u32 all;
  struct {
  mt_u32 GRP_MODE:2;
  mt_u32 :30;
  } bitc;
} HW_CE_CHANNEL_GRP_REG;

// 31:24	RSV
// 23:22	TSPARSE_SMALL_MODE	RW		小于一个block的小包处理模式
//									00:清流	01:DVS042的尾包处理方式		10:异或IVE
// 21	TSPARSE_SHORT_MODE	RW		不是block倍数的小包处理模式(非DVS042或者CTS模式)
//									00:头包清流		01:尾包清流
// 20	TSPARSE_CTS_MODE	RW		0:解扰模式的CBC-CTS模式，TSPARSE_EN为1时配置为0
//									1:普通CBC-CTS模式，TSPARSE_EN为0时配置为1
// 19:17	RSV
// 16:15	TSPARSE_IVE_MODE	RW		00	MDI mode
//									01	MDD mode
//									10	MSC override mode
// 14	TSPARSE_IVE_CAL_EN	RW		IVE计算使能
// 13	TSPARSE_EN			RW		0:不使能TS信号1:使能TS信号
// 12	TSPARSE_LEN			RW		0:TS包长188byte	1:TS包长192byte
// 11	TSPARSE_IND			RW		0:清除加扰指示1:保留加扰指示
//                                                		>>B0 版本:
//                                                		>>TSPARSE_IND为1时加扰/解扰case无论是否做了加解扰操作加扰标识都保持不变
//                                                		>>TSPARSE_IND为0时做了加解扰操作时加扰标识会被修改
// 10	TSPARSE_KEY_SEL		RW		0:奇密钥加密1:偶密钥加密(仅当bit9为0时有效)
// 9		TSPARSE_DEC			RW		0:加密	1:解密
// 8:7	ALGORITHM_SEL		RW		00:AES
//									01:DES
//									10:SHA
// 6:4	ALGORITHM_MODE		RW		SHA模式:
//									//001:sha-1
//									//010:sha-224
//									011:sha-256
//									100:HMAC-sha256
//									AES模式:
//									[6]:		0加密1解密
//									[5:4]:	00	128bit key
//											//01	192bit key
//											//10	256bit key
//									DES模式:
//									[6]:		0加密1解密
//									[5]:		0 DES	1 TDES
//									[4]:		0 ABA	//1 ABC
// 3:0	ADES_MODE	RW		0000:ECB normal
//							0001:CBC normal
//							0010:CTR(仅当ALGORITHM_SEL为0时有效)
//							0011:CBC DVS042
//							0100:CBC CTS
//							0101:RCBC(仅当TSPARSE_EN为1时有效)
//							0110:ECB CTS(仅当TSPARSE_EN为1时有效)
//							1000:1-CFB(仅当TSPARSE_EN为0时有效)
//							1001:8-CFB(仅当TSPARSE_EN为0时有效)
//							1010:128-CFB/64-CFB(仅当TSPARSE_EN为0时有效)
//							1100:1-OFB(仅当TSPARSE_EN为0时有效)
//							1101:8-OFB(仅当TSPARSE_EN为0时有效)
//注意:
// 1.当CBCCTS/ECBCTS模式时,TSPARSE_DEC(bit9)必须与ALGORITHM_MODE中的加解密模式(bit6)配置为一致
// 2.当测试非TS包时，即TSPARSE_EN为0时，TSPARSE_IVE_CAL_EN必须配置为0
#define ALGORITHMn_MODE(n)		(CE_BASE+0x2400+n*0x08)
typedef union _HW_CE_ALGO_MODE_REG
{
  mt_u32 all;
  struct {
  mt_u32 ADES_MODE:4;
  mt_u32 ALGORITHM_MODE:3;
  mt_u32 ALGORITHM_SEL:2;
  mt_u32 TSPARSE_DEC:1;
  mt_u32 TSPARSE_KEY_SEL:1;
  mt_u32 TSPARSE_IND:1;
  mt_u32 TSPARSE_LEN:1;
  mt_u32 TSPARSE_EN:1;
  mt_u32 TSPARSE_IVE_CAL_EN:1;
  mt_u32 TSPARSE_IVE_MODE:2;
  mt_u32 :3;
  mt_u32 TSPARSE_CTS_MODE:1;
  mt_u32 TSPARSE_SHORT_MODE:1;
  mt_u32 TSPARSE_SMALL_MODE:2;
  mt_u32 :8;
  } bitc;
} HW_CE_ALGO_MODE_REG;

//31:0	TOTAL_LENGTH		RW		数据块总长度
#define ALGORITHMn_LEN(n)		(CE_BASE+0x2404+n*0x08)

//31:5	RSV
// 4		SHA_TOTAL_BUSY		R	SHA算法busy信号,以TOTAL_LENGTH为单位
// 3		SHA_BUSY	R		SHA算法busy信号
// 2		DES_BUSY	R		DES算法busy信号
// 1		AES_BUSY	R		AES算法busy信号
//0		TS_BUSY		R		TS解包算法busy信号	
#define ALGORITHM_STATUS		(CE_BASE+0x2420)

//SHA分组信息
//31:14	RSV
//13:12	SHA_CONTINUE		RW		00:代表第不分组
//									01:代表第一个分组
//									10:代表中间分组
//									11:代表最后一个分组
//11:9	RSV
//8		RSV
//7:6	RSV
//5:0	RSV
#define SHA_GRP_CTRL		(CE_BASE+0x2430)
typedef union _HW_CE_SHA_GRP_REG
{
  mt_u32 all;
  struct {
  mt_u32 :12;
  mt_u32 GRP_MODE:2;
  mt_u32 :18;
  } bitc;
} HW_CE_SHA_GRP_REG;

//31:6	RSV
//5:0	EVEN_KEY_ADDR		RW		偶数key的index
#define ALGORITHM_EVEN_KEY_ADDR(n)	(CE_BASE+0x2490+n*0x10)

//31:6	RSV
//5:0	ODD_KEY_ADDR	RW				奇数key的index
#define ALGORITHM_ODD_KEY_ADDR(n)	(CE_BASE+0x2494+n*0x10)


//31:1	RSV
// 0		KEY_SMALLEND	RW				channel key(包括初始向量及SHA 计算结果)小端模式
//Default: 0x00000001
//12月9日以后的bitfile版本，这个寄存器将取消对key的大小端控制，只作为sha摘要结果的大小端控制
#define ALGORITHM_KEY_MODE(n)			(CE_BASE+0x2498+n*0x10)

//31:0		SHA_REG			RW		SHA结果
#define SHA_REG(n)			(CE_BASE+0x2700+4*n)

//31:0		SHA_LEN_SUM		RW		SHA总长度
#define SHA_LEN_SUM		(CE_BASE+0x2720)






//31:29		RSV
//28:16		PID1_FILT		RW		过滤加解密的PID1
//15:13		RSV
//12:0		PID0_FILT		RW		过滤加解密的PID0
#define PID01_FILT			(CE_BASE+0x2600)
typedef union _HW_CE_TS_PID01_FILT_REG
{
  mt_u32 all;
  struct {
  mt_u32 PID0_FILT:13;
  mt_u32 :3;
  mt_u32 PID1_FILT:13;
  mt_u32 :3;
  } bitc;
} HW_CE_TS_PID01_FILT_REG;

//31:29		RSV
//28:16		PID3_FILT		RW		过滤加解密的PID3
//15:13		RSV
//12:0		PID2_FILT		RW		过滤加解密的PID2
#define PID23_FILT			(CE_BASE+0x2604)
typedef union _HW_CE_TS_PID23_FILT_REG
{
  mt_u32 all;
  struct {
  mt_u32 PID2_FILT:13;
  mt_u32 :3;
  mt_u32 PID3_FILT:13;
  mt_u32 :3;
  } bitc;
} HW_CE_TS_PID23_FILT_REG;

//31:29		RSV
//28:16		PID5_FILT		RW		过滤加解密的PID5
//15:13		RSV
//12:0		PID4_FILT		RW		过滤加解密的PID4
#define PID45_FILT			(CE_BASE+0x2608)
typedef union _HW_CE_TS_PID45_FILT_REG
{
  mt_u32 all;
  struct {
  mt_u32 PID4_FILT:13;
  mt_u32 :3;
  mt_u32 PID5_FILT:13;
  mt_u32 :3;
  } bitc;
} HW_CE_TS_PID45_FILT_REG;

//31:29		RSV
//28:16		PID7_FILT		RW		过滤加解密的PID7
//15:13		RSV
//12:0		PID6_FILT		RW		过滤加解密的PID6
#define PID67_FILT			(CE_BASE+0x260C)
typedef union _HW_CE_TS_PID67_FILT_REG
{
  mt_u32 all;
  struct {
  mt_u32 PID6_FILT:13;
  mt_u32 :3;
  mt_u32 PID7_FILT:13;
  mt_u32 :3;
  } bitc;
} HW_CE_TS_PID67_FILT_REG;

// 31:10		RSV
// 9:8		PID_FILT_CH		RW		PID过滤的通道
//									00:crypto ch0
//									01:crypto ch1
//									10:crypto ch2
//									11:crypto ch3
// 7			PID7_FILT		RW		PID7过滤使能
// 6			PID6_FILT		RW		PID6过滤使能
// 5			PID5_FILT		RW		PID5过滤使能
// 4			PID4_FILT		RW		PID4过滤使能
// 3			PID3_FILT		RW		PID3过滤使能
// 2			PID2_FILT		RW		PID2过滤使能
// 1			PID1_FILT		RW		PID1过滤使能
// 0			PID0_FILT		RW		PID0过滤使能
#define PID_FILT_CFG		(CE_BASE+0x2610)
typedef union _HW_CE_TS_PID_FILT_CFG_REG
{
  mt_u32 all;
  struct {
  mt_u32 PID0_FILT_EN:1;
  mt_u32 PID1_FILT_EN:1;
  mt_u32 PID2_FILT_EN:1;
  mt_u32 PID3_FILT_EN:1;
  mt_u32 PID4_FILT_EN:1;
  mt_u32 PID5_FILT_EN:1;
  mt_u32 PID6_FILT_EN:1;
  mt_u32 PID7_FILT_EN:1;
  mt_u32 PID_FILT_CH:2;
  mt_u32 :22;
  } bitc;
} HW_CE_TS_PID_FILT_CFG_REG;

//31:9		RSV
//8			RSA_CRT_SEL			RW		1表示开启CRT模式
//7:4		RSV
// 3:0		FORCE_ENC_MODE		RW           [0]: 1表示CH0 对非清流的TS包也进行强制加扰
//                                                                  [1]: 1表示CH1 对非清流的TS包也进行强制加扰
//                                                                  [2]: 1表示CH2 对非清流的TS包也进行强制加扰
//                                                                  [3]: 1表示CH3 对非清流的TS包也进行强制加扰
#define CRYPTO_MISC_CFG	(CE_BASE+0x2614)
typedef union _HW_CE_MISC_CFG_REG
{
  mt_u32 all;
  struct {
  mt_u32 FORCE_ENC_MODE:4;
  mt_u32 :4;
  mt_u32 RSA_CRT_SEL:1;
  mt_u32 :23;
  } bitc;
} HW_CE_MISC_CFG_REG;


#define RSA_DAT_IN		(CE_BASE+0x2800)
#define RSA_DAT_OUT		(CE_BASE+0x2804)
//======== RSA_CMD ========
//				31:9		//保留
//RSA_BUSY		8		//RSA算法busy信号
//				7:2		//保留
//RSA_TYPE		1:0		//RSA数据类型
//						// 00:写数据M
//						// 01:写数据E
//						// 1x:写数据X
#define RSA_CMD				(CE_BASE+0x2808)
//======== ECC_CMD ========
//31:1		RSV
//0			ECC_ENDIAN			ECC数据大小端
//								0:小端
//								1:大端
//								默认值为0
#define ECC_ENDIAN		(CE_BASE+0x280C)

#define ECC_DAT_IN		(CE_BASE+0x2810)
#define ECC_DAT_OUT		(CE_BASE+0x2814)
//======== ECC_DAT_CFG ========
// 31:16		RSV
// 15:8		ECC_LEN				写入数据长度(数据位宽/32)
// 7:4		ECC_ADDR			写入ram的起始地址
// 3:0		ECC_RAM_NUM		写入的ram编号，范围为0~7
#define ECC_DAT_CFG		(CE_BASE+0x2818)

//======== ECC_CMD ========
//31		ECC_BUSY		R	ECC算法的busy信号
//30		MODINV_FAIL		R	1:模逆失败，该点为无穷远点
//29		RSV
//28		ECC_START			ECC算法开始信号
//27:24		ECC_CMD				0x01	点乘
//								0x02	点加
//								0x03	模乘
//								0x04	模加
//								0x05	模减
//								0x06	模逆
//23:20		DST_ADDR			输出数据所在ram的起始地址
//19:16		DST_RAM_NUM		输出数据所在ram编号，范围0~7
//15:12		SRC1_ADDR			SRC1写入ram的起始地址
//11:8		SRC1_RAM_NUM		SRC1写入的ram编号，范围为0~7
//7:4		SRC0_ADDR			SRC0写入ram的起始地址
// 3:0		SRC0_RAM_NUM		SRC0写入的ram编号，范围为0~7
#define ECC_CMD			(CE_BASE+0x281C)


//普通cpu中断使能
// 31:5		RESERVED
// 4			RSA_INT_MASK		RSA计算结束中断使能
// 3			DMACH3_INT_MASK	DMA处理通道3结束中断使能
// 2			DMACH2_INT_MASK	DMA处理通道2结束中断使能
// 1			DMACH1_INT_MASK	DMA处理通道1结束中断使能
// 0			DMACH0_INT_MASK	DMA处理通道0结束中断使能
#define COMINT_MASK		(CE_BASE+0x3000)

// 31:5		RESERVED
// 4			RSA_INT_STA		RSA计算结束中断状态，读状态清
// 3			DMACH3_INT_STA		DMA处理通道3结束中断状态，读状态清
// 2			DMACH2_INT_STA		DMA处理通道2结束中断状态，读状态清
// 1			DMACH1_INT_STA		DMA处理通道1结束中断状态，读状态清
// 0			DMACH0_INT_STA		DMA处理通道0结束中断状态，读状态清
#define COMINT_STA		(CE_BASE+0x300C)
//安全cpu中断使能
// 31:5		RESERVED
// 4			RSA_INT_MASK		RSA计算结束中断使能
// 3			DMACH3_INT_MASK	DMA处理通道3结束中断使能
// 2			DMACH2_INT_MASK	DMA处理通道2结束中断使能
// 1			DMACH1_INT_MASK	DMA处理通道1结束中断使能
// 0			DMACH0_INT_MASK	DMA处理通道0结束中断使能
#define SECINT_MASK		(CE_BASE+0x3010)
// 31:5		RESERVED
// 4			RSA_INT_STA			RSA计算结束中断状态，读状态清
// 3			DMACH3_INT_STA		DMA处理通道3结束中断状态，读状态清
// 2			DMACH2_INT_STA		DMA处理通道2结束中断状态，读状态清
// 1			DMACH1_INT_STA		DMA处理通道1结束中断状态，读状态清
// 0			DMACH0_INT_STA		DMA处理通道0结束中断状态，读状态清
#define SECINT_STA		(CE_BASE+0x301C)

// 2		清0开始AXI软复位，delay 1us以上，置1结束AXI软复位(用于crypto engine整体软复位)
// 1		清0开始模块软复位，delay 1us以上，置1结束模块软复位(用于crypto engine整体软复位)
// 0		清0开始RST软复位，delay 1us以上，置1结束RST软复位(用于rsa软复位)
#define CRYPTO_SW_RST	(0xBF510030)


#endif	/*__HW_CE_REG_H__*/

