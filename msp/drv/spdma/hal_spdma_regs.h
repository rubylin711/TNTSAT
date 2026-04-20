/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _HAL_SPDMA_REGS_H
#define _HAL_SPDMA_REGS_H

 /************************************************************************
 * Defination of Reset register
 ************************************************************************/
/*!
  comments
  */
#define R_RST_REQ(n)                              (0xBF510000 + (n) * (0x4))
/*!
  comments
  */
#define R_RST_REQ_AO                            0xBF150040
/*!
  comments
  */
#define R_RST_CTRL(n)                             (0xBF510010 + (n) * (0x4))
/*!
  comments
  */
#define R_RST_CTRL_AO                            0xBF150044
/*!
  comments
  */
#define R_RST_ALLOW(n)                          (0xBF510020 + (n) * (0x4))
/*!
  comments
  */
#define R_RST_ALLOW_AO                        0xBF150048



/*!
  DMA once copy limit
  */
#define SPDMA_LIMIT (1023 * 8)

/*!
  DMA once copy limit for sdram to sdram
  */
#define SPDMA_LIMIT_D2D (8*1024*1024 - 8)

/*!
  SP_RAM_SIZE
  */
#define SP_RAM_SIZE (1024 * 8)


/************************************************************************
 * SPDMA Controller
 ************************************************************************/
#define SPDMA_BASE_ADDR						0xBE900000
#define SPDMA_SRC_ADDR						(SPDMA_BASE_ADDR + 0x40)
#define SPDMA_DST_ADDR						(SPDMA_BASE_ADDR + 0x44)
#define SPDMA_DATA_LEN						(SPDMA_BASE_ADDR + 0x74)
#define SPDMA_BUF_CONFIG_REG				(SPDMA_BASE_ADDR + 0x68)
#define SPDMA_DATA_PROCESS_CFG_REG			(SPDMA_BASE_ADDR + 0x6C)
#define SPDMA_BUF0_BASE_ADDR				(SPDMA_BASE_ADDR + 0x48)
#define SPDMA_BUF1_BASE_ADDR				(SPDMA_BASE_ADDR + 0x4C)
#define SPDMA_BUF2_BASE_ADDR				(SPDMA_BASE_ADDR + 0x50)
#define SPDMA_BUF3_BASE_ADDR				(SPDMA_BASE_ADDR + 0x54)
#define SPDMA_BUF4_BASE_ADDR				(SPDMA_BASE_ADDR + 0x58)
#define SPDMA_BUF5_BASE_ADDR				(SPDMA_BASE_ADDR + 0x5C)
#define SPDMA_BUF6_BASE_ADDR				(SPDMA_BASE_ADDR + 0x60)
#define SPDMA_BUF7_BASE_ADDR				(SPDMA_BASE_ADDR + 0x64)
#define SPDMA_CONTROL_REG					(SPDMA_BASE_ADDR + 0x70)
#define SPDMA_BUSY_STATUS					(SPDMA_BASE_ADDR)
#define SPDMA_INT_CLEAR						(SPDMA_BASE_ADDR + 0x84)
#define SPDMA_MOVED_CNT						(SPDMA_BASE_ADDR + 0x4)










#define R_SPDMA_BASE_ADDR    SYMPHONY_IO_VA(0xBE900000)


/*!
  bit[0] RW,0-复位 axi 1-复位释放
  bit[1] RW,0-复位 ahb 1-复位释放
  bit[2] RW,0-复位   core 1-复位释放
  bit[3] RW,0-复位 spdma axi 1-复位释放
  bit[4] RW,0-复位 spdma ahb 1-复位释放
  bit[5] RW,0-复位 spdma core 1-复位释放
  31:6 save
*/
#define R_AVCPU_CLK				SYMPHONY_IO_VA(0xBF508200)
#define R_AVCPU_SRSTN			SYMPHONY_IO_VA(0xBF50820C)
#define R_AVCPU_SLOCK			SYMPHONY_IO_VA(0xBF508218)


/*!
  bit[0]=1,RO,busy
  31:1 save
*/
#define R_SPDMA_BUSY_STATUS				R_SPDMA_BASE_ADDR
/*!
  bit[10:0],RO,当前数据传输量,单位:64bit
  31:11 save
*/
#define R_SPDMA_MOVED_CNT				(R_SPDMA_BASE_ADDR + 0x4)
/*!
  bit[23:0],RO,PCM PP BUF0当前写指针,单位:64bit
  31:24 save
*/
#define R_SPDMA_BUF0_POINT				(R_SPDMA_BASE_ADDR + 0x8)

/*!
  bit[31:0],RW,src addr,64bit对齐
*/
#define R_SPDMA_SRC_ADDR				(R_SPDMA_BASE_ADDR + 0x40)
/*!
  bit[31:0],RW,dst addr,64bit对齐
*/
#define R_SPDMA_DST_ADDR				(R_SPDMA_BASE_ADDR + 0x44)
/*!
  bit[31:0],RW,PCM PP BUF0 起始地址,64bit对齐.
  SEL_BUF_CH=0&&SPDMA_SEL_BASE_ADDR=1时，在初始化或复位该寄存器用于SPDMA目的地址
*/
#define R_SPDMA_BUF0_BASE_ADDR			(R_SPDMA_BASE_ADDR + 0x48)
#define R_SPDMA_BUF1_BASE_ADDR			(R_SPDMA_BASE_ADDR + 0x4c)
#define R_SPDMA_BUF2_BASE_ADDR			(R_SPDMA_BASE_ADDR + 0x50)
#define R_SPDMA_BUF3_BASE_ADDR			(R_SPDMA_BASE_ADDR + 0x54)
#define R_SPDMA_BUF4_BASE_ADDR			(R_SPDMA_BASE_ADDR + 0x58)
#define R_SPDMA_BUF5_BASE_ADDR			(R_SPDMA_BASE_ADDR + 0x5c)
#define R_SPDMA_BUF6_BASE_ADDR			(R_SPDMA_BASE_ADDR + 0x60)
#define R_SPDMA_BUF7_BASE_ADDR			(R_SPDMA_BASE_ADDR + 0x64)


/*!
  bit[23:0],RW,buf size,单位64bit
  bit[26:24],RW,SPDMA PCM PP BUF选择
  000:PCM PP BUF0
  001:PCM PP BUF1
  010:PCM PP BUF2
  011:PCM PP BUF3
  100:PCM PP BUF4
  101:PCM PP BUF5
  110:PCM PP BUF6
  111:PCM PP BUF7
  31:27 save
*/
#define R_SPDMA_BUF_CONFIG				(R_SPDMA_BASE_ADDR + 0x68)
/*!
  31:17 save
  bit[16:8],RW,SPDMA传输数据长度高位配置,单位:64bit
  7:5 save
  bit[4],RW,DATA_CUT_MODE,0非截位32位输出/1截位16位输出
  bit[3],RW,饱和模式,高有效
  bit[2],RW,移位时四舍五入，高有效
  bit[1:0],RW，移位处理模式:(00右移16位/01右移15位/10右移3位/11右移0位)
*/
#define R_SPDMA_DATA_PROCESS_CFG		(R_SPDMA_BASE_ADDR + 0x6C)
/*!
  bit[31],-,save
  bit[30:20],RW,SPDMA_DATA_LEN,SPDMA传输数据长度低位配置,单位:64bit
  bit[19:17],-,save
  bit[16],RW,SPDMA_SEL_BASE_ADDR,SPDMA目的地址选择(0选择DST_ADDR为起始地址/1根据SEL_BUF_CH选择当前BUF写指针为起始地址)
  bit[15:13],-,save
  bit[12],RW,SPDMA数据处理模式(0直接搬运/1数据处理模式)
  bit[11],-,save
  bit[10],RW,SPDMA_BUS_MODE,SPDMA总线写操作模式(0:WRLAST/1:WR_DONE)
  bit[9:8],RW,SPDMA_BURST_NUM,burst(00:4/01:8/10:16/11:32)
  bit[7:5],-,save
  bit[4],RW,SPDMA_CANCLE,自清零
  bit[3:1],-,save
  bit[0],RW,SPDMA_ENABLE,自清零
*/
#define R_SPDMA_CONTROL					(R_SPDMA_BASE_ADDR + 0x70)
/*!
  bit[31:20],-,save
  bit[19:0],RW,SPDMA_DATA_LEN_NEW,单位:64bit
*/
#define R_SPDMA_DATA_LEN				(R_SPDMA_BASE_ADDR + 0x74)

/*!
  bit[31:1],-,save
  bit[0],RW,SPDMA传输中断Clear,自清零
*/
#define R_SPDMA_INT_CLEAR				(R_SPDMA_BASE_ADDR + 0x84)

/*!
  bit[31:9],-,save
  bit[8],RW,one-way register,0:unlock,1:lock
  bit[7:4],RW,4'b1111
  bit[3:0],RW,4'b0(AHB Master读写SPRAM通路使能);others(AHB Master读写SPRAM通路关闭)
*/
#define R_SPDMA_MODE_SET				(R_SPDMA_BASE_ADDR + 0xC4)



#define R_SPDMA_RCID_CFG				(R_SPDMA_BASE_ADDR + 0xD0)
#define R_SPDMA_WCID_CFG				(R_SPDMA_BASE_ADDR + 0xD4)
#define R_SPDMA_CID_LOCK				(R_SPDMA_BASE_ADDR + 0xD8)
#define R_SPDMA_WCID_LOCK				(R_SPDMA_BASE_ADDR + 0xDC)

#define R_ARM_CID_ERR_IMASK				(R_SPDMA_BASE_ADDR + 0xE0)
#define R_ARM_CID_ERR_ICLR				(R_SPDMA_BASE_ADDR + 0xE4)
#define R_ARM_CID_ERR_ISTA				(R_SPDMA_BASE_ADDR + 0xE8)

#define R_VSCPU_CID_ERR_IMASK			(R_SPDMA_BASE_ADDR + 0xEC)
#define R_VSCPU_CID_ERR_ICLR			(R_SPDMA_BASE_ADDR + 0xF0)
#define R_VSCPU_CID_ERR_ISTA			(R_SPDMA_BASE_ADDR + 0xF4)


#ifdef __cplusplus
}
#endif

#endif /* _DMX_REGS_SYMPHONY_H */
