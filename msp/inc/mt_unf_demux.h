/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_UNF_DEMUX_H__
#define __MT_UNF_DEMUX_H__

#include "mt_unf_common.h"
#include "mt_error_mpi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*************************** Structure Definition ****************************/
/** \addtogroup      DEMUX */
/** @{ */  /** <!-- [DEMUX] */

/**Maximum depth of a filter. */
/**CNcomment:filter最大深度*/
#define DMX_FILTER_MAX_DEPTH            16

#define DMX_OVERFLOW_MAX          (16)

/**Defines the capability of the DEMUX module*/
/**CNcomment:定义DEMUX模块业务功能结构体*/
typedef struct mtUNF_DMX_CAPABILITY_S
{
    mt_u32 u32IFPortNum;     /**<Number of IF ports. */ /**< CNcomment: IF 端口数量  (与TS In 端口都是通过Tuner 连接,但是IF 端口是使用内置 QAM)    */
    mt_u32 u32TSIPortNum;     /**<Number of TS In ports. */ /**< CNcomment: TS In 端口数量 (与IF端口都是通过Tuner 连接,但是IF 端口是使用内置 QAM)    */
    mt_u32 u32TSOPortNum;     /**<Number of TS Out ports. */ /**< CNcomment: TS Out  端口数量*/
    mt_u32 u32RamPortNum;       /**<Number of Ram ports. */ /**< CNcomment: Ram端口数量     */
    mt_u32 u32DmxNum;           /**<Number of DEMUX devices*/ /**< CNcomment:设备数量     */
    mt_u32 u32ChannelNum;       /**<Number of channels, containing the audio and video channels */ /**< CNcomment:通道数量，含音视频通道的数量 */
    mt_u32 u32AVChannelNum;     /**<Number of av channels*/ /**< CNcomment:音视频通道数量 */
    mt_u32 u32FilterNum;        /**<Number of filters*/ /**< CNcomment:过滤器数量   */
    mt_u32 u32KeyNum;           /**<Number of keys*/ /**< CNcomment:密钥区数量   */
    mt_u32 u32RecChnNum;        /**<Number of record channels */ /**< CNcomment:录制通道的数量 */
} MT_UNF_DMX_CAPABILITY_S;

/**TSO port ID*/
/**CNcomment:TS输出端口ID */
typedef enum mtUNF_DMX_TSO_PORT_E
{
    MT_UNF_DMX_PORT_TSO_0,      /**<The first TS OUT port . */ /**< CNcomment:TS Out 端口0 */
    MT_UNF_DMX_PORT_TSO_1,
    MT_UNF_DMX_PORT_TSO_2,
    MT_UNF_DMX_PORT_TSO_3
}MT_UNF_DMX_TSO_PORT_E;

/**TS port ID*/
/**CNcomment:TS端口ID */
typedef enum mtUNF_DMX_PORT_E
{
    MT_UNF_DMX_PORT_IF_0 = 0x0,   /*This function is not supported*/
    MT_UNF_DMX_PORT_IF_1,   /*This function is not supported*/
    MT_UNF_DMX_PORT_IF_2,   /*This function is not supported*/
    MT_UNF_DMX_PORT_IF_3,   /*This function is not supported*/
    MT_UNF_DMX_PORT_IF_4,   /*This function is not supported*/
    MT_UNF_DMX_PORT_IF_5,   /*This function is not supported*/
    MT_UNF_DMX_PORT_IF_6,   /*This function is not supported*/
    MT_UNF_DMX_PORT_IF_7,   /*This function is not supported*/

    MT_UNF_DMX_PORT_TSI_0 = 0x20,/**<The first TS IN  port . */ /**< CNcomment:TSIN 端口0 , (TSIN 端口即Tuner 端口) */
    MT_UNF_DMX_PORT_TSI_1,
    MT_UNF_DMX_PORT_TSI_2,
    MT_UNF_DMX_PORT_TSI_3,
    MT_UNF_DMX_PORT_TSI_4,
    MT_UNF_DMX_PORT_TSI_5,
    MT_UNF_DMX_PORT_TSI_6,
    MT_UNF_DMX_PORT_TSI_7,

    MT_UNF_DMX_PORT_RAM_0 = 0x80,/**<The first RAM port . */ /**< CNcomment:RAM 端口0 */
    MT_UNF_DMX_PORT_RAM_1,
    MT_UNF_DMX_PORT_RAM_2,
    MT_UNF_DMX_PORT_RAM_3,
    MT_UNF_DMX_PORT_RAM_4,
    MT_UNF_DMX_PORT_RAM_5,
    MT_UNF_DMX_PORT_RAM_6,
    MT_UNF_DMX_PORT_RAM_7,

    MT_UNF_DMX_PORT_BUTT
} MT_UNF_DMX_PORT_E;


/**TS port mode*/
/**CNcomment:TS端口模式 */
typedef enum mtUNF_DMX_PORT_MODE_E
{
    MT_UNF_DMX_PORT_MODE_EXTERNAL,   /**<External TS input mode*/ /**< CNcomment:外部TS输入*/
    MT_UNF_DMX_PORT_MODE_INTERNAL,   /**<Internal TS input mode*/ /**< CNcomment:内部TS输入*/
    MT_UNF_DMX_PORT_MODE_RAM,        /**<Memory input mode*/ /**< CNcomment:从内存输入*/

    MT_UNF_DMX_PORT_MODE_BUTT
} MT_UNF_DMX_PORT_MODE_E;

/**TS port type*/
/**CNcomment:TS端口类型 */
typedef enum mtUNF_DMX_PORT_TYPE_E
{
    MT_UNF_DMX_PORT_TYPE_PARALLEL_BURST,          /**<Parallel burst mode*/ /**< CNcomment:并行BURST模式             */
    MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID,          /**<Parallel valid mode*/ /**< CNcomment:并行VALID模式             */
    MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188,     /**<Self-sync 188 mode*/ /**< CNcomment:自同步188模式             */
    MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_204,     /**<Self-sync 204 mode*/ /**< CNcomment:自同步204模式             */
    MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188_204, /**<self-sync 188/204 auto-identification mode */ /**< CNcomment:自同步188/204自动识别模式 */
    MT_UNF_DMX_PORT_TYPE_SERIAL,                  /**<Serial sync mode, 1bit*/ /**< CNcomment:串行sync模式, 1bit串行 */
    MT_UNF_DMX_PORT_TYPE_USER_DEFINED,            /**<User defined mode,  not supported*//**< CNcomment:用户自定义模式 */
    MT_UNF_DMX_PORT_TYPE_SERIAL2BIT,              /**<Serial sync mode, 2bit,  not supported*/ /**< CNcomment:串行sync模式, 2bit串行 */
    MT_UNF_DMX_PORT_TYPE_SERIAL_NOSYNC,           /**<Serial nosync mode, 1bit,  not supported */ /**< CNcomment:串行nosync模式, 1bit串行 */
    MT_UNF_DMX_PORT_TYPE_SERIAL2BIT_NOSYNC,       /**<Serial nosync mode, 2bit,  not supported */ /**< CNcomment:串行nosync模式, 2bit串行 */
    MT_UNF_DMX_PORT_TYPE_AUTO,                    /**<Auto mode*/ /**< CNcomment: 自动模式 */

    MT_UNF_DMX_PORT_TYPE_BUTT
} MT_UNF_DMX_PORT_TYPE_E;

/**TSO clock mode*/
/**CNcomment:TS输出端口时钟模式 */
typedef enum mtUNF_DMX_TSO_CLK_MODE_E
{
    MT_UNF_DMX_TSO_CLK_MODE_NORMAL,                  /**<Normal clock: clock always active*/ /**< CNcomment:普通模式的时钟,时钟平率均匀*/
    MT_UNF_DMX_TSO_CLK_MODE_JITTER ,                  /**<Jittered clock: clock active only when outputing data,  not supported*/ /**< CNcomment:Jittered 模式时钟,时钟频率不均匀,此时只有在有数据输出时才会出时钟 (valid信号为高的情况下)*/
    MT_UNF_DMX_TSO_CLK_MODE_BUTT
}MT_UNF_DMX_TSO_CLK_MODE_E;

/**TSO valid mode*/
/**CNcomment:TS输出端口valid信号模式 */
typedef enum mtUNF_DMX_TSO_VALID_MODE_E
{
    MT_UNF_DMX_TSO_VALID_ACTIVE_OUTPUT,          /**<Valid signal high when outputing datas ,  not supported*/ /**< CNcomment:valid信号在输出数据时为高*/
    MT_UNF_DMX_TSO_VALID_ACTIVE_HIGH ,          /**<Valid signal always high,  not supported*/ /**< CNcomment:valid信号总是为高*/
    MT_UNF_DMX_TSO_VALID_ACTIVE_BUTT
}MT_UNF_DMX_TSO_VALID_MODE_E;

/**TSO port signal line selector*/
/**CNcomment:TS输出端口线序选择 */
typedef enum mtUNF_DMX_TSO_SERIAL_BIT_E
{
    MT_UNF_DMX_TSO_SERIAL_BIT_0 = 0x0,          /**< Serial output data using data[0] as signal line*/ /**< CNcomment:使用data[0]作为串行信号线输出*/
    MT_UNF_DMX_TSO_SERIAL_BIT_7 = 0x7,          /**< Serial output data using data[7] as signal line*/ /**< CNcomment:使用data[7]作为串行信号线输出*/
    MT_UNF_DMX_TSO_SERIAL_BIT_BUTT
}MT_UNF_DMX_TSO_SERIAL_BIT_E;

/**TS out mode clock frequency   ,  not supported*/
/**CNcomment:TS输出模块时钟频率 ,注意,实际的TS out 端口输出频率 = TSO 模块时钟/分频因子*/
typedef enum mtUNF_DMX_TSO_CLK_E
{
    MT_UNF_DMX_TSO_CLK_100M ,           /**< TS out mode clock frequency 100M*/ /**< CNcomment:TS输出模块时钟频率选择为100M*/
    MT_UNF_DMX_TSO_CLK_150M ,           /**< TS out mode clock frequency 150M*/ /**< CNcomment:TS输出模块时钟频率选择为150M*/
    MT_UNF_DMX_TSO_CLK_1200M ,          /**< TS out mode clock frequency 1200M*/ /**< CNcomment:TS输出模块时钟频率选择为1200M*/
    MT_UNF_DMX_TSO_CLK_1500M ,          /**< TS out mode clock frequency 1500M*/ /**< CNcomment:TS输出模块时钟频率选择为1500M*/
    MT_UNF_DMX_TSO_CLK_BUTT
}MT_UNF_DMX_TSO_CLK_E;

/** Tag sync mode */
/** CNcomment: Tag 同步模式*/
typedef enum mtUNF_DMX_TAG_SYNC_MODE_E
{
    MT_UNF_DMX_TAG_HEAD_SYNC = 0x0,          /** < tag sync signal at tag head*/ /**< CNcomment:sync信号在tag头*/
    MT_UNF_DMX_NORMAL_HEAD_SYNC = 0x1,    /** < tag sync signal at 47 heade*/ /** <CNcomment:sync信号在47头*/
}MT_UNF_DMX_TAG_SYNC_MODE_E;

//the packet length
typedef enum mtUNF_DMX_PORT_PACKET_LENGTH_E
{
    PORT_PACKET_204,	//packet length is 204bytes
    PORT_PACKET_188		//packet length is 188bytes
}MT_UNF_DMX_PORT_PACKET_LENGTH_E;

//which check the packet length
typedef enum mtUNF_DMX_PORT_SET_PACKET_LENGTH_E
{
    PORT_HARDWARE_AUTO_CHECK,	//hardware autocheck the packet length
    PORT_SOFTWARE_SET			//software set the packet length
}MT_UNF_DMX_PORT_SET_PACKET_LENGTH_E;

//which packet to recevie
typedef enum mtUNF_DMX_PORT_PACKET_MODE_E
{
    PORT_MPEG2_MODE,	//MPEG2 packet
    PORT_DSS_MODE		//DSS packet
}MT_UNF_DMX_PORT_PACKET_MODE_E;

//whether modify the first byte
typedef enum mtUNF_DMX_PORT_FIRST_BYTE_MODE_E
{
    PORT_FIRST_BYTE_TO_47,		//the first byte will be modify to 0x47
    PORT_FIRST_BYTE_NOT_MODIFY	//not modify the first byte
}MT_UNF_DMX_PORT_FIRST_BYTE_MODE_E;

//the 3line config
typedef enum mtUNF_DMX_PORT_3LINE_MODE_E
{
    PORT_3LINE_ZERO,	//other case
    PORT_3LINE_NO_SYNC	//3line but no sync
}MT_UNF_DMX_PORT_3LINE_MODE_E;

//transfer sequence for serial mode
typedef enum mtUNF_DMX_PORT_ENDIAN_E
{
    PORT_MSB,	//bit7 first
    PORT_LSB	//bit0 first
}MT_UNF_DMX_PORT_ENDIAN_E;

//check stream lost
typedef enum mtUNF_DMX_PORT_CHECK_STREAM_LOST_E
{
    PORT_STREAM_CHECK_START_FLAG,	//check start flag
    PORT_STREAM_CHECK_TS_SYNC,		//check ts sync
    PORT_STREAM_CHECK_TS_CLK,		//check ts clk
    PORT_STREAM_CHECK_NO			//don't check
}MT_UNF_DMX_PORT_CHECK_STREAM_LOST_E;

//ts start flag
typedef enum mtUNF_DMX_PORT_TS_START_FLAG_E
{
    PORT_TS_START_47,	//0x47
    PORT_TS_START_B8	//0xB8
}MT_UNF_DMX_PORT_TS_START_FLAG_E;

//whether use the line
typedef enum mtUNF_DMX_PORT_BYPASS_E
{
    PORT_NOT_BYPASS,	//use the line
    PORT_BYPASS			//not use the line
}MT_UNF_DMX_PORT_BYPASS_E;

//which polar is valid for the line
typedef enum mtUNF_DMX_PORT_POLAR_E
{
    PORT_LOW_LEVEL,	//low level is valid
    PORT_HIGH_LEVEL	//high level is valid
}MT_UNF_DMX_PORT_POLAR_E;

//which data is used to output for serial
typedef enum mtUNF_DMX_PORT_SERIAL_DATA_E
{
    PORT_SERIAL_DATA_0, 
    PORT_SERIAL_DATA_1,
    PORT_SERIAL_DATA_2,
    PORT_SERIAL_DATA_3,
    PORT_SERIAL_DATA_4,
    PORT_SERIAL_DATA_5,
    PORT_SERIAL_DATA_6,
    PORT_SERIAL_DATA_7
}MT_UNF_DMX_PORT_SERIAL_DATA_E;

//select the port mode
typedef enum mtUNF_DMX_PORT_SERIAL_EN_E
{
    PORT_PARALLEL, 	//parallel
    PORT_SERIAL		//serial
}MT_UNF_DMX_PORT_SERIAL_EN_E;

//enable/disable the port
typedef enum mtUNF_DMX_PORT_EN_E
{
    PORT_DISABLE,	//disalbe
    PORT_ENABLE		//enable
}MT_UNF_DMX_PORT_EN_E;

typedef union {
    mt_u32 all;
    struct {
        mt_u32 syncon_th            : 4;//sync lock threshold, suggest use:0xf
        mt_u32 syncoff_th           : 4;//sync lost threshold, suggest use:0xf
        mt_u32 ts_188_reg           : 1;//if software set ts packet length, 0:204bytes, 1:188bytes, suggest use:1
        mt_u32 ts_188_reg_en        : 1;//0:hardware check ts packet length, 1:software set ts packet length, suggest use:0
        mt_u32 ts_pkt_set           : 1;//0:mpeg packet, 1:dss packet
		mt_u32 ts_first_byte_mode   : 1;//0:if first byte is not 0x47, it will be modified to 0x47. 1:don't modify, suggest use:0
		mt_u32 ts_3line_mode        : 1;//if use 3 line mode and not use sync, it should set to 1, other case set to 0
		mt_u32 ts_endian			: 1;//transfer sequence for serial mode, 0:msb first, 1:lsb first
        mt_u32                      : 2;
        mt_u32 brk_sel              : 2;//check stream lost, 0x3:not check, 0x2:check i_ts_clk, 0x1:check i_ts_sync, 0x0:check start flag
        mt_u32 sop_token            : 1;//start flag, 0:0x47, 1:0xB8, suggest use:0
        mt_u32 sync_bypass          : 1;//0:use sync, 1:don't use sync
        mt_u32 val_bypass           : 1;//0:use valid, 1:don't use valid
        mt_u32 sync_err_bypass      : 1;//check whether the sync byte is same with start flag, 0:check, 1:don't check
        mt_u32 tei_bypass           : 1;//0:use tei, 1:don't use tei
        mt_u32 err_bypass           : 1;//0:use err, 1:don't use err
        mt_u32 err_pol              : 1;//0:low level is valid, 1:high level is valid
        mt_u32 val_pol              : 1;//0:low level is valid, 1:high level is valid
        mt_u32 sync_pol             : 1;//0:low level is valid, 1:high level is valid
        mt_u32 serial_sel           : 3;//which bit output for serial mode, 0x0:data[0], ..., 0x7:data[7]
        mt_u32 serial_en            : 1;//0:parallel mode, 1:serial mode
        mt_u32 ts_en                : 1;//0:disable port, 1:enable port
    } bitc;
} MT_UNF_DMX_PORT_CTRL_S;

//tsi clk
typedef enum mtUNF_DMX_TSI_CLK_E
{
    CLK_400M,	//400M
    CLK_360M,	//360M
    CLK_262M,	//262M
    CLK_131M,	//131M
}MT_UNF_DMX_TSI_CLK_E;

//ts3 clk select
typedef enum mtUNF_DMX_PORT_CLK_SEL_E
{
    TS0_PAD_CLK,				//0
    TS0_PAD_CLK_INVERT,			//1
    TS0_DVBC_CLK,				//2
    TS0_DVBC_CLK_INVERT,		//3
    TS0_DVBS2_1_CLK,			//4
    TS0_DVBS2_1_CLK_INVERT,		//5

	TS1_PAD_CLK,				//6
	TS1_DVBS_CLK,				//7
    TS1_PAD_CLK_INVERT,    		//8
    TS1_DVBS_CLK_INVERT,		//9

	TS2_PAD_CLK,				//10
    TS2_PAD_CLK_INVERT,			//11
    TS2_DVBC_CLK,				//12
    TS2_DVBC_CLK_INVERT,		//13
    TS2_CICAM_OUT_CLK,			//14
    TS2_CICAM_OUT_CLK_INVERT,	//15

	TS3_PAD_CLK,				//16
	TS3_DVBS2_1_CLK,			//17
    TS3_PAD_CLK_INVERT,			//18
    TS3_DVBS2_1_CLK_INVERT,		//19
}MT_UNF_DMX_PORT_CLK_SEL_E;

//ts src select
typedef enum mtUNF_DMX_PORT_SRC_SEL_E
{
    TS0_EXTERNAL_DEMOD,				//0		
    TS0_INTERNAL_DEMOD_S2_1,		//1
    TS0_INTERNAL_DEMOD_C_J83B_S2_2,	//2
    
    TS1_EXTERNAL_DEMOD,				//3
    TS1_INTERNAL_DEMOD_S2_0,		//4
    
	TS2_EXTERNAL_DEMOD,				//5
	TS2_INTERNAL_DEMOD_C_J83B_S2_2,	//6
    TS2_INTERNAL_CICAM,				//7
    
    TS3_EXTERNAL_DEMOD,				//8
    TS3_INTERNAL_DEMOD_S2_1,		//9
}MT_UNF_DMX_PORT_SRC_SEL_E;

//enable/disable the port
typedef enum mtUNF_DMX_TSBUF_FULL_CARE_EN_E
{
    FULL_CARE_DISABLE,	//disable
    FULL_CARE_ENABLE	//enalbe
}MT_UNF_DMX_TSBUF_FULL_CARE_EN_E;

typedef union {
    mt_u32 all;
    struct {
        mt_u32 ch0_es            : 1;//av channel 0, es buf full care, 0:don't care, 1:care
        mt_u32 ch1_es            : 1;//av channel 1, es buf full care, 0:don't care, 1:care
        mt_u32 ch2_es            : 1;//av channel 2, es buf full care, 0:don't care, 1:care
        mt_u32 ch3_es            : 1;//av channel 3, es buf full care, 0:don't care, 1:care
        mt_u32 ch4_es            : 1;//av channel 4, es buf full care, 0:don't care, 1:care
        mt_u32 ch5_es            : 1;//av channel 5, es buf full care, 0:don't care, 1:care
        mt_u32 ch6_es            : 1;//av channel 6, es buf full care, 0:don't care, 1:care
        mt_u32 ch7_es            : 1;//av channel 7, es buf full care, 0:don't care, 1:care
        mt_u32 ch8_es            : 1;//av channel 8, es buf full care, 0:don't care, 1:care
        mt_u32 ch9_es            : 1;//av channel 9, es buf full care, 0:don't care, 1:care
        mt_u32 ch10_es           : 1;//av channel 10, es buf full care, 0:don't care, 1:care
        mt_u32 ch11_es           : 1;//av channel 11, es buf full care, 0:don't care, 1:care
        mt_u32 ch12_es           : 1;//av channel 12, es buf full care, 0:don't care, 1:care
        mt_u32 ch13_es           : 1;//av channel 13, es buf full care, 0:don't care, 1:care
        mt_u32 ch14_es           : 1;//av channel 14, es buf full care, 0:don't care, 1:care
        mt_u32 ch15_es           : 1;//av channel 15, es buf full care, 0:don't care, 1:care
        
        mt_u32 ch0_desc          : 1;//av channel 0, desc buf full care, 0:don't care, 1:care
        mt_u32 ch1_desc          : 1;//av channel 1, desc buf full care, 0:don't care, 1:care
        mt_u32 ch2_desc          : 1;//av channel 2, desc buf full care, 0:don't care, 1:care
        mt_u32 ch3_desc          : 1;//av channel 3, desc buf full care, 0:don't care, 1:care
        mt_u32 ch4_desc          : 1;//av channel 4, desc buf full care, 0:don't care, 1:care
        mt_u32 ch5_desc          : 1;//av channel 5, desc buf full care, 0:don't care, 1:care
        mt_u32 ch6_desc          : 1;//av channel 6, desc buf full care, 0:don't care, 1:care
        mt_u32 ch7_desc          : 1;//av channel 7, desc buf full care, 0:don't care, 1:care
        mt_u32 ch8_desc          : 1;//av channel 8, desc buf full care, 0:don't care, 1:care
        mt_u32 ch9_desc          : 1;//av channel 9, desc buf full care, 0:don't care, 1:care
        mt_u32 ch10_desc         : 1;//av channel 10, desc buf full care, 0:don't care, 1:care
        mt_u32 ch11_desc         : 1;//av channel 11, desc buf full care, 0:don't care, 1:care
        mt_u32 ch12_desc         : 1;//av channel 12, desc buf full care, 0:don't care, 1:care
        mt_u32 ch13_desc         : 1;//av channel 13, desc buf full care, 0:don't care, 1:care
        mt_u32 ch14_desc         : 1;//av channel 14, desc buf full care, 0:don't care, 1:care
        mt_u32 ch15_desc         : 1;//av channel 15, desc buf full care, 0:don't care, 1:care
    } bitc;
} MT_UNF_DMX_AV_CHN_FULL_CARE_S;

typedef union {
    mt_u32 all;
    struct {
        mt_u32 rec0_buf            : 1;//record 0 buf full care, 0:don't care, 1:care
        mt_u32 rec1_buf            : 1;//record 1 buf full care, 0:don't care, 1:care
        mt_u32 rec2_buf            : 1;//record 2 buf full care, 0:don't care, 1:care
        mt_u32 rec3_buf            : 1;//record 3 buf full care, 0:don't care, 1:care

		mt_u32 index0_buf          : 1;//index 0 buf full care, 0:don't care, 1:care
		mt_u32 index1_buf          : 1;//index 1 buf full care, 0:don't care, 1:care
		mt_u32 index2_buf          : 1;//index 2 buf full care, 0:don't care, 1:care
		mt_u32 index3_buf          : 1;//index 3 buf full care, 0:don't care, 1:care
		mt_u32 index4_buf          : 1;//index 4 buf full care, 0:don't care, 1:care
		mt_u32 index5_buf          : 1;//index 5 buf full care, 0:don't care, 1:care
		mt_u32 index6_buf          : 1;//index 6 buf full care, 0:don't care, 1:care
		mt_u32 index7_buf          : 1;//index 7 buf full care, 0:don't care, 1:care

		mt_u32 rec4_buf            : 1;//record 4 buf full care, 0:don't care, 1:care
        mt_u32 rec5_buf            : 1;//record 5 buf full care, 0:don't care, 1:care
        mt_u32 rec6_buf            : 1;//record 6 buf full care, 0:don't care, 1:care

		mt_u32 index8_buf          : 1;//index 8 buf full care, 0:don't care, 1:care
		mt_u32 index9_buf          : 1;//index 9 buf full care, 0:don't care, 1:care
		mt_u32 index10_buf         : 1;//index 10 buf full care, 0:don't care, 1:care
		mt_u32 index11_buf         : 1;//index 11 buf full care, 0:don't care, 1:care

		mt_u32 reserve0            : 8;

		mt_u32 ci_buf			   : 1;

		mt_u32 reserve1            : 4;		
    } bitc;
} MT_UNF_DMX_REC_CHN_FULL_CARE_S;

typedef struct
{
  mt_u32 overflow_allcnt;
  mt_u32 wp;
  mt_u32 overflow_times[DMX_OVERFLOW_MAX];
  mt_u64 overflow_e[DMX_OVERFLOW_MAX];
  mt_u64 overflow_s[DMX_OVERFLOW_MAX];
} DMX_RecInfo_Overflow_S;

/** TS Tag attributes */
/** CNcomment: TS Tag 配置属性*/
#define MAX_TAG_LENGTH (12)
typedef struct mtUNF_DMX_TAG_ATTR_S
{
    mt_u8     au8Tag[MAX_TAG_LENGTH];       /** <[IN]tag index value*/ /**< CNcomment:端口的tag值 */
    mt_u32   u32TagLen;   /** <[IN & OUT]Valid tag length(1 ~ 12bytes), which is an input para when setTagAttr, otherwise out para when getTagAttr*/ /** <CNcomment:有效tag长度(1~12字节),setTagAttr时作为输入参数,getTagAttr时作为输出参数*/
    MT_BOOL bEnabled;       /**<[IN & OUT]Port state(default disabled), which is an input para when setTagAttr, otherwise out para when getTagAttr*/ /**< CNcomment:端口使能(默认不使能),setTagAttr时作为输入参数,getTagAttr时作为输出参数*/
    MT_UNF_DMX_TAG_SYNC_MODE_E enSyncMod; /** <[IN & OUT]Sync mode(default MT_UNF_DMX_TAG_HEAD_SYNC), which is an input para when setTagAttr, otherwise out para when getTagAttr*/ /**<CNcomment:同步模式(默认值为MT_UNF_DMX_PORT_TAG_HEAD_SYNC ),setTagAttr时作为输入参数,getTagAttr时作为输出参数*/
}MT_UNF_DMX_TAG_ATTR_S;


/**TS out port attributes*/
/**CNcomment:TS Out 端口属性*/
typedef struct mtUNF_DMX_TSO_PORT_ATTR_S
{
    MT_BOOL                     bEnable;       /**<Port enable, default value MT_TRUE means enable*/ /**< CNcomment:端口使能,默认使能*/
    MT_BOOL                     bClkReverse;   /**<Clock phase reverse, default value MT_FALSE means do not reverse the phase of clock*/ /**< CNcomment:端口时钟反向,默认不反向*/
    MT_UNF_DMX_PORT_E           enTSSource;    /**<Source of this TS Out port ,can choose from MT_UNF_DMX_PORT_IF_0 to MT_UNF_DMX_PORT_TSI_9*/  /**< CNcomment:TS Out 端口数据源,可以从MT_UNF_DMX_PORT_IF_0 到MT_UNF_DMX_PORT_TSI_9 之间选择*/
    MT_UNF_DMX_TSO_CLK_MODE_E   enClkMode;     /**<Clock mode: MT_UNF_DMX_TSO_CLK_MODE_NORMAL is the default value*/ /**< CNcomment:时钟模式: 默认值为 MT_UNF_DMX_TSO_CLK_MODE_NORMAL*/
    MT_UNF_DMX_TSO_VALID_MODE_E enValidMode;   /**<Wether valid signal always enable : MT_UNF_DMX_TSO_VALID_ACTIVE_OUTPUT is the default value */ /**< CNcomment:Valid 模式: 默认值为:MT_UNF_DMX_TSO_VALID_ACTIVE_OUTPUT  */
    MT_BOOL                     bBitSync;       /**<The sync signal duration : MT_TRUE: only valid when output the first bit(default). MT_FALSE: keep when outputing the whole byte*/
                                                /**< CNcomment:sync 信号有效期 : MT_TRUE: 只在输出第一个bit时有效 (默认) . MT_FALSE: 在输出整个字节时都有效  */
    MT_BOOL                     bSerial;        /**<Wether out put mode is serial: MT_FALSE: parallel mode. MT_TRUE: serial mode (default)*/ /**< CNcomment:端口输出模式: MT_FALSE: 并行 . MT_TRUE:  串行(默认) */
    MT_UNF_DMX_TSO_SERIAL_BIT_E enBitSelector;  /**<Port line sequence select In serial mode.only valid when using serial out put mode,MT_UNF_DMX_TSO_SERIAL_BIT_7 is the default value.*/
                                                /**< CNcomment:端口串行模式线序选择,只有使用串行输出模式的时候有效,默认值为 MT_UNF_DMX_TSO_SERIAL_BIT_7*/
    MT_BOOL                     bLSB;           /**<Out put byte endian .only valid when using serial out put mode: MT_FALSE: first output MSB (default). MT_TRUE:  first output LSB*/
    						                    /**< CNcomment:端口输出大小端模式,只有使用串行输出模式的时候有效: MT_FALSE: 首先输出最高位 (默认) . MT_TRUE: 首先输出最低位  */
    MT_UNF_DMX_TSO_CLK_E        enClk;          /**<TS out mode clock frequency,default is MT_UNF_DMX_TSO_CLK_150M*/ /**< CNcomment:TS输出模块时钟频率,默认是 MT_UNF_DMX_TSO_CLK_150M*/
    mt_u32                      u32ClkDiv;      /**<TS out mode clock frequency divider,must be times of 2 ,and must meet (2 <= u32ClkDiv <= 32) .default is 2*/ /**< CNcomment:TSO模块分频因子,必须是2的整数倍,取值范围在2与32之间*/
} MT_UNF_DMX_TSO_PORT_ATTR_S;


/**TS port attributes*/
/**CNcomment:TS端口属性*/
typedef struct mtUNF_DMX_PORT_ATTR_S
{
    MT_UNF_DMX_PORT_MODE_E enPortMod;  /**<Port mode.Readonly*/ /**< CNcomment:端口模式 只读*/
    MT_UNF_DMX_PORT_TYPE_E enPortType; /**<Port type*/ /**< CNcomment:端口类型 */
    mt_u32 u32SyncLostTh;              /**<Sync loss threshold.The default value is recommended.*/ /**< CNcomment:同步丢失门限，推荐使用默认值 */
    mt_u32 u32SyncLockTh;              /**<Sync lock threshold.The default value is recommended.*/ /**< CNcomment:同步锁定门限，推荐使用默认值 */
    mt_u32 u32TunerInClk;              /**<Whether to reverse the phase of the clock input from the tuner*/ /**< CNcomment:Tuner输入时钟是否反相.0: 同相(默认); 1: 反相*/
    mt_u32 u32SerialBitSelector;       /**<Port line sequence select In parallel mode: 0: cdata[7] is the most significant bit (MSB) (default). 1: cdata[0] is the MSB.
                                           In serial mode: 1: cdata[0] is the data line (default). 0: cdata[7] is the data line.*/
                                        /**< CNcomment:端口线序选择。并行时:0: 表示cdata[7]最高位(默认);1: 表示cdata[0]最高位。
                                           串行时:1: 表示cdata[0]为数据线(默认); 0: 表示cdata[7]为数据线*/

    mt_u32 u32TunerErrMod;             /**<Level mode of the cerr_n line from the tuner to a DEMUX.
                                           0: A data error occurs when the cerr_n line is high.
                                           1: A data error occurs when the cerr_n line is low (default).*/
                                            /**< CNcomment:tuner到DEMUX的cerr_n线电平模式:
                                           0: 表示cerr_n线为高时表示数据错误;
                                           1: 表示cerr_n线为低时表示数据错误 (默认) */
    mt_u32 u32UserDefLen1;            /**<User defined length1,valid when enPortType is MT_UNF_DMX_PORT_TYPE_USER_DEFINED,188~255 *//**< CNcomment:用户自定义长度检测包长1，只有端口类型为用户自定义时有效，有效范围188~255 */
    mt_u32 u32UserDefLen2;            /**<User defined length2,valid when enPortType is MT_UNF_DMX_PORT_TYPE_USER_DEFINED,188~255 *//**< CNcomment:用户自定义长度检测包长2，只有端口类型为用户自定义时有效，有效范围188~255*/
} MT_UNF_DMX_PORT_ATTR_S;

/**Status of the TS port*/
/**CNcomment:TS端口的状态*/
typedef struct mtUNF_DMX_PORT_PACKETNUM_S
{
    mt_u32  u32TsPackCnt;    /**<Number of TS packets received from the TS port*/ /**< CNcomment:端口收到的TS包计数*/
    mt_u32  u32ErrTsPackCnt; /**<Number of error TS packets received from the TS port*/ /**< CNcomment:端口上错误TS包的计数*/
} MT_UNF_DMX_PORT_PACKETNUM_S;

/**Status of a TS buffer of a DEMUX*/
/**CNcomment:DEMUX的TS Buffer状态 */
typedef struct mtUNF_DMX_TSBUF_STATUS_S
{
    mt_u32 u32BufSize;    /**<Buffer size*/ /**< CNcomment:缓冲区大小       */
    mt_u32 u32UsedSize;   /**<Used buffer size*/ /**< CNcomment:缓冲区已使用大小,该值比实际使用值大0x100字节 */
    mt_u32 u32BufAddr;
} MT_UNF_DMX_TSBUF_STATUS_S;

/**Channel type*/
/**CNcomment:通道类型 */
typedef enum mtUNF_DMX_CHAN_TYPE_E
{
    MT_UNF_DMX_CHAN_TYPE_SEC = 0, /**<Channel that receives sections data such as program specific information (PSI) or service information (SI) data*/ /**< CNcomment:用于接收 PSI/SI等secitons数据的通道*/
    MT_UNF_DMX_CHAN_TYPE_PES,     /**<Channel that receives packetized elementary stream (PES) data*/ /**< CNcomment:用于接收PES包数据的通道*/
    MT_UNF_DMX_CHAN_TYPE_AUD,     /**<Channel that receives audio data*/ /**< CNcomment:用于接收音频数据的通道*/
    MT_UNF_DMX_CHAN_TYPE_VID,     /**<Channel that receives video data*/ /**< CNcomment:用于接收视频数据的通道*/
    MT_UNF_DMX_CHAN_TYPE_POST,    /**<Entire-packet posting channel that receives an entire TS packet with a specific packet identifier (PID). */ /**< CNcomment:整包上送通道，用于接收某PID的完整TS包*/
    MT_UNF_DMX_CHAN_TYPE_ECM_EMM, /**<Channel that receives entitlement control message (ECM) or entitlement management message (EMM) data*/ /**< CNcomment:用于接收ECM或EMM数据的通道*/
    MT_UNF_DMX_CHAN_TYPE_HW_PES,  /*hardware pes*/
    MT_UNF_DMX_CHAN_TYPE_REC,     /**<Recording mode*/ /**< CNcomment:录制 */
    MT_UNF_DMX_CHAN_TYPE_AUD_AD,  /**<Channel that receives audio ad data*/ /**< CNcomment:用于接收AD数据的通道 */
    MT_UNF_DMX_CHAN_TYPE_DSS,     /*for direct tv dss_data */
    MT_UNF_DMX_CHAN_TYPE_BUTT
} MT_UNF_DMX_CHAN_TYPE_E;

/**Cyclic redundancy check (CRC) mode of a channel*/
/**CNcomment:通道的CRC模式 */
typedef enum mtUNF_DMX_CHAN_CRC_MODE_E
{
    MT_UNF_DMX_CHAN_CRC_MODE_FORBID = 0,                /**<The CRC check is disabled*/ /**< CNcomment:CRC校验禁止 */
    MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_DISCARD = 1,     /**<The CRC check is enabled, and the error Section data is discarded*/ /**< CNcomment:CRC校验使能，并丢弃错误Section */
    MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_SEND = 2,        /**<The CRC check is enabled, and the error Section data is received*/ /**< CNcomment:CRC校验使能，并上送错误Section */
    MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD = 3, /**<Whether the CRC check is performed depends on the syntax, and the error Section data is discarded*/ /**< CNcomment:根据语法判断是否进行CRC校验，并丢弃错误Section */
    MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_SEND = 4,    /**<Whether the CRC check is performed depends on the syntax, and the error Section data is received*/ /**< CNcomment:根据语法判断是否进行CRC校验，并上送错误Section */

    MT_UNF_DMX_CHAN_CRC_MODE_BUTT
} MT_UNF_DMX_CHAN_CRC_MODE_E;

/**Output mode of a channel*/
/**CNcomment:通道的输出模式 */
typedef enum mtUNF_DMX_CHAN_OUTPUT_MODE_E
{
    MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY = 0x1,     /**<Mode of playing audios/videos or receiving data*/ /**< CNcomment:音视频播放或数据接收 */
    MT_UNF_DMX_CHAN_OUTPUT_MODE_REC  = 0x2,     /**<Recording mode*/ /**< CNcomment:录制 */
    MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY_REC = 0x3, /**<Mode of recording and playing data or receiving data*/ /**< CNcomment:同时录制和播放或数据接收 */
    MT_UNF_DMX_CHAN_OUTPUT_MODE_BUTT = -1
} MT_UNF_DMX_CHAN_OUTPUT_MODE_E;

typedef struct mt_UNF_DMX_BUF_FULL_CARE_S
{
    mt_u32    pid;
    mt_u32    es_buf_full;
    mt_u32    dsc_buf_full;
    mt_u32    sec_buf_full;
} MT_UNF_DMX_BUF_FULL_CARE_S;

/**Channel attribute*/
/**CNcomment:通道属性*/
typedef struct mtUNF_DMX_CHAN_ATTR_S
{
    mt_u32 u32BufSize;                            /**<Buffer size used by channels*/ /**< CNcomment:通道硬件使用buffer大小*/
    MT_UNF_DMX_CHAN_TYPE_E enChannelType;         /**<Channel type*/ /**< CNcomment:通道类型 */
    MT_UNF_DMX_CHAN_CRC_MODE_E enCRCMode;         /**<CRC mode.It is valid for the DEMUX_CHAN_SEC channel.*/ /**< CNcomment:CRC模式，通道类型为DMX_CHAN_SEC 时有效 */
    MT_UNF_DMX_CHAN_OUTPUT_MODE_E enOutputMode;   /**<Output mode of the channel data*/ /**< CNcomment:通道数据输出模式 */
    MT_BOOL  AVSyncFlag;
    MT_UNF_VCODEC_TYPE_E   vCodecType;
    mt_u8  *esBuffId1;
    MT_BOOL enDiscardErrorPUSIPacket;             /**<Dicard the packet with PUSI error or not*/ /**< CNcomment:丢弃还是保留有PUSI错误的包*/
    MT_BOOL bEsToUser;                            /**<MT_FALSE: default, MT_TRUE: special feature which updates aes to user*/
	mt_u32  pip_en;
} MT_UNF_DMX_CHAN_ATTR_S;

/**Scrambled flag of the channel data*/
/**CNcomment:通道数据的加扰标志 */
typedef enum mtUNF_DMX_SCRAMBLED_FLAG_E
{
    MT_UNF_DMX_SCRAMBLED_FLAG_TS,  /**<TS data is scrambled*/ /**< CNcomment:数据被TS级加扰*/
    MT_UNF_DMX_SCRAMBLED_FLAG_PES, /**<PES data is scrambled*/ /**< CNcomment:数据被PES级加扰*/
    MT_UNF_DMX_SCRAMBLED_FLAG_NO,  /**<Data is not scrambled*/ /**< CNcomment:数据未被加扰*/

    MT_UNF_DMX_SCRAMBLED_FLAG_BUTT
} MT_UNF_DMX_SCRAMBLED_FLAG_E;

/**Channel status*/
/**CNcomment:通道状态 */
typedef enum mtUNF_DMX_CHAN_STATUS_E
{
    MT_UNF_DMX_CHAN_CLOSE = 0x0,        /**<The channel is stopped.*/ /**< CNcomment:通道处于关闭状态*/
    MT_UNF_DMX_CHAN_PLAY_EN = 0x1,      /**<The channel is playing audios/videos or receiving data.*/ /**< CNcomment:通道处于音视频播放或数据接收状态*/
    MT_UNF_DMX_CHAN_REC_EN = 0X2,       /**<The channel is recording data.*/ /**< CNcomment:通道处于数据录制状态*/
    MT_UNF_DMX_CHAN_PLAY_REC_EN = 0X3,  /**<The channel is recording and receiving data.*/ /**< CNcomment:通道处于同时录制和数据接收状态*/
} MT_UNF_DMX_CHAN_STATUS_E;

/**Defines the channel status*/
/** CNcomment:定义通道状态结构体*/
typedef struct mtUNF_DMX_CHAN_STATUS_S
{
    MT_UNF_DMX_CHAN_STATUS_E enChanStatus;   /**<Channel status*/ /**< CNcomment:通道状态*/
} MT_UNF_DMX_CHAN_STATUS_S;

/**Filter attribute*/
/**CNcomment:过滤器属性*/
typedef struct mtUNF_DMX_FILTER_ATTR_S
{
    mt_u32  u32FilterDepth;                     /**<Depth of a filter.*/ /**< CNcomment:过滤器深度*/
    mt_u8   au8Match[DMX_FILTER_MAX_DEPTH];     /**<Matched bytes of a filter.The data is compared by bit.*/ /**< CNcomment:过滤器匹配字节,按bit比较*/
    mt_u8   au8Mask[DMX_FILTER_MAX_DEPTH];      /**<Masked bytes of a filter. The conditions are set by bit. 0: no mask. Comparison is required. 1: mask. Comparison is not required.*/ /**< CNcomment:过滤器屏蔽字节,按bit设置, 0:没有mask，要进行比较, 1:mask起作用，不进行比较*/
    mt_u8   au8Negate[DMX_FILTER_MAX_DEPTH];    /**<Negated bytes of a filter. 0: not negated; 1: negated*/ /**< CNcomment:过滤器取反字节,按byte设置, 0:不取反,1:取反*/
} MT_UNF_DMX_FILTER_ATTR_S;

/**Type of the DEMUX data packet.*/
/**CNcomment:DEMUX数据包的类型*/
typedef enum mtUNF_DMX_DATA_TYPE_E
{
    MT_UNF_DMX_DATA_TYPE_WHOLE = 0,  /**<The data segment contains a complete data packet*/ /**< CNcomment:此段数据包含完整的数据包, 对于SECTION每个包都是完整的*/
    MT_UNF_DMX_DATA_TYPE_HEAD,       /**<The data segment contains the head of a data packet, but the data packet may not be complete*/ /**< CNcomment:此段数据包含数据包的起始，但是不一定是完整的包, 只用于PES数据*/
    MT_UNF_DMX_DATA_TYPE_BODY,       /**<This type is valid only for the PES data.The data segment contains the body of a data packet. */ /**< CNcomment:此段数据包含数据包的内容，不包含起始，可能有结尾, 只用于PES数据*/
    MT_UNF_DMX_DATA_TYPE_TAIL,       /**<This type is valid only for the PES data.The data segment contains the tail of a data packet, and is used to identify the end of a data packet.*/ /**< CNcomment:此段数据包含数据包的结尾，用于指示可识别的包结束, 只用于PES数据*/

    MT_UNF_DMX_DATA_TYPE_BUTT
} MT_UNF_DMX_DATA_TYPE_E;

/**DEMUX data packet*/
/**CNcomment:DEMUX数据包结构*/
typedef struct mtUNF_DMX_DATA_S
{
    mt_u8   *pu8Data;                   /**<Data pointer*/ /**< CNcomment:数据指针 */
    mt_u32  u32Size;                    /**<Data length*/ /**< CNcomment:数据长度 */
    MT_UNF_DMX_DATA_TYPE_E enDataType;  /**<Data packet type*/ /**< CNcomment:数据包的类型 */
    mt_u32 filthandle;                /*filter handle*/
	phys_addr_t data_phy_addr;
} MT_UNF_DMX_DATA_S;

/**type of record*/
/**CNcomment: 录制类型*/
typedef enum mtUNF_DMX_REC_TYPE_E
{
    MT_UNF_DMX_REC_TYPE_SELECT_PID,
    MT_UNF_DMX_REC_TYPE_ALL_PID,
    MT_UNF_DMX_REC_TYPE_BUTT
} MT_UNF_DMX_REC_TYPE_E;

/**type of index*/
/**CNcomment: 索引类型 */
typedef enum mtUNF_DMX_REC_INDEX_TYPE_E
{
    MT_UNF_DMX_REC_INDEX_TYPE_NONE,     /**<No index is created */  /**<CNcomment: 不建索引 */
    MT_UNF_DMX_REC_INDEX_TYPE_VIDEO,    /**<Video index */          /**<CNcomment: 视频索引 */
    MT_UNF_DMX_REC_INDEX_TYPE_AUDIO,    /**<Audio index */          /**<CNcomment: 音频索引 */
    MT_UNF_DMX_REC_INDEX_TYPE_BUTT
} MT_UNF_DMX_REC_INDEX_TYPE_E;

/**recorded Ts time stamp mode*/
/**CNcomment: 录锟斤拷TS锟斤拷时锟斤拷锟斤拷锟斤拷模式*/
typedef enum mtUNF_DMX_REC_TIMESTAMP_MODE_E
{
    MT_UNF_DMX_REC_TIMESTAMP_NONE,       /**<No time stamp added before each recoreded  ts packet*/
	MT_UNF_DMX_REC_TIMESTAMP_INSERT, 	/**<time stamp  added before each recoreded  ts packet*/
} MT_UNF_DMX_REC_TIMESTAMP_MODE_E;

/*!
   TS Record modes.
  */
typedef enum
{
  /*!
    Full TS Record with Null Packet
    */
  DMX_FULL_TS_WITH_NULL_PACKET,
  /*!
    Full TS Record without Null Packet
    */
  DMX_FULL_TS_WITHOUT_NULL_PACKET,
  /*!
    Partial TS record
    */
  DMX_PARTIAL_TS_PACKET,
  /*!
    Partial TS record before scramble
    */
  DMX_PARTIAL_TS_PACKET_SCRAMBLE,  
  /*!
    Partial TS record after descramble
    */
  DMX_PARTIAL_TS_PACKET_DESCRAMBLE,
   /*!
    Partial TS record after descramble
    */
  DMX_PARTIAL_TS_PACKET_PES_DESCRAMBLE
} dmx_rec_mode_t;

/**record attribute*/
/**CNcomment: 录制属性*/
typedef struct mtUNF_DMX_REC_ATTR_S
{
    mt_u32                      u32DmxId;
    mt_u32                      u32RecBufSize;  /**<Buffer size used by record */ /**< CNcomment: 录制缓存大小 */
    MT_UNF_DMX_REC_TYPE_E       enRecType;      /**<Record type */ /**< CNcomment: 录制类型 */
    MT_BOOL                     bDescramed;     /**<MT_TRUE is the descrambled TS. MT_FALSE is the original TS. */ /**<CNcomment: MT_TRUE表示录制解扰后的TS流，MT_FALSE表示录制原始流 */
    MT_UNF_DMX_REC_INDEX_TYPE_E enIndexType;    /**<Index type */ /**< CNcomment: 索引类型 */
    mt_u32                      u32IndexSrcPid; /**<The index information is formed according to the PID. when indexing video, it has to be set to the video of PID. when indexing audio, it has to be set to the audio of PID. */ /**< CNcomment: 根据此PID生成索引数据，视频索引时设置为视频的PID，音频索引时设置为音频的PID */
    MT_UNF_VCODEC_TYPE_E        enVCodecType;   /**<Video encoding protocol. The protocol needs to be set only when the index type is MT_UNF_DMX_REC_INDEX_TYPE_VIDEO.*/ /**< CNcomment: 视频索引时设置视频的编码类型 */
    dmx_rec_mode_t                      type_mode;
    MT_BOOL                     AlignToPageBlock; /**if tsbuffer for DIO. set true.**/ /*如果tsbuffer被用于DIO,设置ture*/
    mt_u8   start_code_filter[16];
    mt_u16   filt_en;  
	MT_UNF_DMX_REC_TIMESTAMP_MODE_E rec_mode;
	mt_u8	multi_chan_flag;//whether use a pid record to multi channel, 1:use, 0:not use
	mt_u8	first_chan_flag;
	mt_u8	idx_filter_mode;//0:shared mode(default), all the filter use a shared start code filter, 1:every filter use its own start code filter
	mt_u8	link_node_num;//how many link node will be used if use link mode. the range is from 1 to 8. if exceed this range, driver will set to 8.
	
	mt_u8 	sync_with_index;	//0: use the record buf's Read and Write Pointer, 1: record buf's Read pointer sync with index's ts number.

} MT_UNF_DMX_REC_ATTR_S;

/**record data*/
/**CNcomment: 录制数据*/
typedef struct mtUNF_DMX_REC_DATA_S
{
    mt_u8  *pDataAddr;      /**<Data address*/          /**<CNcomment: 数据地址 */
    mt_u32  u32Offset;      /**<Offset in buffer*/      /**<CNcomment: 缓冲内偏移 */
    phys_addr_t  u32DataPhyAddr; /**<Data physical address*/ /**<CNcomment: 数据的物理地址 */
    mt_u32  u32Len;         /**<Data length*/           /**<CNcomment: 数据长度 */
    mt_u32  u32Size;
} MT_UNF_DMX_REC_DATA_S;

/**indx parameters*/
/**CNcomment: index参数*/
typedef struct mtPVRINDEXCFG_S
{
    mt_u32 idxid;         //index id
    mt_u32 dmxid;         //demux id
    mt_u32 vdectype;      //vdec type
}PVRINDEXCFG_S;

/**index data*/
/**CNcomment: 索引数据*/
typedef struct mtUNF_DMX_REC_INDEX_S
{
    MT_UNF_VIDEO_FRAME_TYPE_E   enFrameType;        /**< it is meaningless when indexing audio. */ /**<CNcomment: 音频索引时无意义 */
    mt_u32                      u32PtsMs;
    mt_u64                      u64GlobalOffset;
    mt_u32                      u32FrameSize;       /**< it is meaningless when indexing audio. */ /**<CNcomment: 音频索引时无意义 */
    mt_u32                      u32DataTimeMs;
    mt_u32                      u32HdrData[2];
    mt_u32                      u32BufOwner;
    mt_u32                      CodecType;
    mt_u32                      u32PrivatePara;     /* avs:profile_id; */
} MT_UNF_DMX_REC_INDEX_S;

/**record buffer status*/
/**CNcomment: 录制缓冲区状态结构*/
typedef struct mtUNF_DMX_RECBUF_STATUS_S
{
    mt_u32  u32BufSize;     /**<Buffer size*/ /**< CNcomment:缓冲区大小 */
    mt_u32  u32UsedSize;    /**<Used buffer*/ /**< CNcomment:缓冲区已使用大小 */
    ulong  u32BufUsrAddr;      /**buffer addr **/
    phys_addr_t  u32BufPhyAddr; 
    mt_u64  u64BufDataLen;  /*total data length*/
    mt_u64  u64RpGlobe;     /*rp globe*/
    DMX_RecInfo_Overflow_S overflowinfo;
    mt_u32  u32BufSizeAlign;     /**Buffer align size**/
} MT_UNF_DMX_RECBUF_STATUS_S;

/**Repeat CC mode of channel*//**CNcomment:通道的CC重复模式 */
typedef enum mtUNF_DMX_CHAN_CC_REPEAT_MODE
{
    MT_UNF_DMX_CHAN_CC_REPEAT_MODE_RSV   = 0x0,     /**<Receive CC repeat ts packet*/ /**< CNcomment:接收cc重复的ts包*/
    MT_UNF_DMX_CHAN_CC_REPEAT_MODE_DROP  = 0x1,     /**<Drop CC repeat ts packet*/ /**< CNcomment:丢弃cc重复的ts包 */
    MT_UNF_DMX_CHAN_CC_REPEAT_MODE_BUTT = -1
}MT_UNF_DMX_CHAN_CC_REPEAT_MODE_E;

typedef struct mtUNF_DMX_CHAN_CC_REPEAT_SET_S
{
    mt_handle hChannel;                               /**<The channel handle*/ /**< CNcomment:通道句柄*/
    MT_UNF_DMX_CHAN_CC_REPEAT_MODE_E enCCRepeatMode;   /**<Repeat CC mode of channel*/ /**< CNcomment:通道的CC模式*/
}MT_UNF_DMX_CHAN_CC_REPEAT_SET_S;

/** PUSI (Payload Unit Start Index) config structure*/
/**CNcomment: PUSI 配置结构体 */
typedef struct mtUNF_DMX_PUSI_SET_S
{
    MT_BOOL bPusi;                               /**< Value of Pusi , Default is MT_FALSE means receive ts packet without checking PUSI*/ /**< CNcomment:NoPusiEn 的值，默认为 MT_FALSE 表示接收TS包的时候不检测PUSI标志*/
}MT_UNF_DMX_PUSI_SET_S;

/** TEI (Transport Error Index) config structure*/
/**CNcomment: TEI 配置结构体 */
typedef struct mtUNF_DMX_TEI_SET_S
{
    mt_u32   u32DemuxID;					/**<The Subdiviece ID*/ /**< CNcomment:Demux 子设备ID*/
    MT_BOOL bTei;                                   /**< Value of bTei , Default is MT_FALSE means receive ts packet even TEI equal 1*/ /**< CNcomment:TEICfg 的值，默认为 MT_FALSE 表示接收TEI为1 的 TS包仍然接收*/
}MT_UNF_DMX_TEI_SET_S;

/** Define of how TSI and TSO to be attached */
/**CNcomment: TSI/TSO绑定关系结构体 */
typedef struct mtUNF_DMX_TSI_ATTACH_TSO_S
{
    MT_UNF_DMX_PORT_E       enTSI;              /**<The TSI ID*/ /**< CNcomment:TSI 端口ID*/
    MT_UNF_DMX_TSO_PORT_E   enTSO;              /**<The TSO ID*/ /**< CNcomment:TSO 端口ID*/
}MT_UNF_DMX_TSI_ATTACH_TSO_S;

typedef enum mtUNF_DMX_INVOKE_TYPE
{
    MT_UNF_DMX_INVOKE_TYPE_CHAN_CC_REPEAT_SET  = 0,  /**<dmx set channel extra attr,param:MT_UNF_DMX_CHAN_CC_REPEAT_SET_S*//**<CNcomment: 设置通道附加属性，使用参数MT_UNF_DMX_CHAN_CC_REPEAT_SET_S */
    MT_UNF_DMX_INVOKE_TYPE_PUSI_SET,                    /**<dmx set PUSI flag,param:MT_UNF_DMX_PUSI_SET_S*//**<CNcomment: 设置 NoPusiEn 标志，使用参数MT_UNF_DMX_PUSI_SET_S */
    MT_UNF_DMX_INVOKE_TYPE_TEI_SET,				/**<dmx set TEI flag,param:MT_UNF_DMX_TEI_SET_S*//**<CNcomment: 设置Demux 子设备TEICfg 标志，使用参数MT_UNF_DMX_TEI_SET_S */
    MT_UNF_DMX_INVOKE_TYPE_TSI_ATTACH_TSO,      /**<Attach TSI with TSO ,param:MT_UNF_DMX_TSI_ATTACH_TSO_S*//**<CNcomment: 将TSI 与 某个 TSO 绑定,即: TSI 的码流来自 TSO ，使用参数MT_UNF_DMX_TEI_SET_S */
    MT_UNF_DMX_INVOKE_TYPE_BUTT
} MT_UNF_DMX_INVOKE_TYPE_E;

/** @} */  /** <!-- ==== Structure Definition end ==== */


typedef struct mtUNF_DMX_DUMP_ES2USB_DEBUG_S
{
    mt_handle hAvplay;
    char *fileName;
}MT_UNF_DMX_DUMP_ES2USB_DEBUG_S;

typedef struct mt_UNF_DMX_T2MI_CONFIG_S
{
    mt_u32				enable;	//0:disable t2mi, 1:enable t2mi
    MT_UNF_DMX_PORT_E	input;	//t2mi input source channel
	MT_UNF_DMX_PORT_E	output;	//t2mi output destination channel
	mt_u32				pid;	//t2mi pid
	mt_u32				plpid;	//t2mi plpid
} MT_UNF_DMX_T2MI_CONFIG_S;

/******************************* API Declaration *****************************/
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
/*reset check buff*/
mt_s32 MT_UNF_DMX_ResetCheckChanBuf(mt_handle *hdbuff,mt_u32 maxcnt);
/*to check whether this vhd be dealed
MT_TRUE: dealed
MT_FALSE:not dealed
*/
mt_s32 MT_UNF_DMX_CheckChanHandle(mt_handle *hdbuff,mt_u32 maxcnt,mt_handle vhd);
#endif
/** \addtogroup      DEMUX */
/** @{ */  /** <!-- [DEMUX] */

/*
** Get/Set TSI's register,  add for testcase 3.2 by yuwu
*/
mt_u32 MT_UNF_DMX_GetRegister(mt_u32 registerOffset);
mt_void MT_UNF_DMX_SetRegister(mt_u32 registerOffset, mt_u32 value);
/*
*  FOR Debug
*/
mt_void MT_UNF_DMX_DumpAllRegister(void);


/**
\brief Initializes the DEMUX module.CNcomment:初始化DEMUX模块。CNend
\attention \n
Before using DEMUXs, you must call this application programming interface (API).\n
Before using the personal video recorder (PVR) or audio/video player (AVPLAY), you need to initialize the DEMUX module.\n
The error code MT_SUCCESS is returned if this API is called repeatedly.
CNcomment:在进行DEMUX相关操作前应该首先调用本接口\n
在使用PVR和AVPLAY前请提前进行DEMUX模块的初始化\n
重复调用本接口返回成功。CNend
\param  N/A.CNcomment:无。CNend
\retval ::MT_SUCCESS Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\see \n
N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_Init(mt_void);


/**
\brief Deinitializes the DEMUX module. CNcomment:去初始化DEMUX模块。CNend
\attention \n
After this API is called, the DEMUX module is stopped, and the DEMUX resources used by the process are released.\n
This API is valid when it is called for the first time. If this API is called repeatedly, the error code MT_SUCCESS is returned.
CNcomment:调用本接口停止使用DEMUX模块，并释放本进程所占用的DEMUX资源\n
本接口第一次调用起作用，重复调用返回成功。CNend
\param  N/A.CNcomment:无。CNend
\retval ::MT_SUCCESS Success CNcomment:成功。CNend
\retval ::MT_FAILURE Calling this API fails. CNcomment:API系统调用失败。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_DeInit(mt_void);


/**
\brief Obtains the information about the service capabilities of the DEMUX module.The information includes the numbers of channels, filters, keys, ports, DEMUXs, and audio/video channels.
CNcomment:查询DEMUX模块业务能力。业务功能包括通用通道数目、过滤器数目、密钥数目、端口数目、DEMUX设备数目和音视频通道数目等。CNend
\attention \n
N/A.CNcomment:无。CNend
\param[out] pstCap   Pointer to the capability of the DEMUX module (output).CNcomment:指针类型，输出DEMUX模块能力。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null.CNcomment:指针参数为空。CNend
\see \n
::MT_UNF_DMX_CAPABILITY_S
*/
mt_s32 MT_UNF_DMX_GetCapability(MT_UNF_DMX_CAPABILITY_S *pstCap);


/**
\brief Obtains the attributes of a DEMUX port.CNcomment:获取DEMUX端口属性。CNend
\attention \n
 N/A.CNcomment:无。CNend
\param[in] enPortId Port ID.CNcomment:端口号。CNend
\param[out] pstAttr Pointer to the port attributes (output).CNcomment:指针类型，输出端口属性。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid.CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null.CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetTSPortAttr(MT_UNF_DMX_PORT_E enPortId, MT_UNF_DMX_PORT_ATTR_S *pstAttr);


/**
\brief Sets the attributes of a DEMUX port.CNcomment:设置DEMUX端口属性。CNend
\attention \n
It is recommended to query the default attributes of a DEMUX port by calling MT_UNF_DMX_GetTSPortAttr before modifying the attributes of the port.\n
The port mode can be queried only, but other attributes can be modified dynamically.\n
If the sync loss threshold and sync lock threshold are used, their default values are recommended.\n
For details on how to set the input clock of the tuner, select the line sequence of the port, and set the level of the tuner error line, contact hardware engineers of Montage\n
and refer to tuner user manuals.
CNcomment:建议先通过MT_UNF_DMX_GetTSPortAttr获取默认属性，然后对要改变的属性进行设置\n
端口模式只支持查询，不支持修改，其他属性均支持随时动态修改\n
同步丢失和同步锁定门限，推荐使用默认值\n
tuner输入时钟反向、端口线序选择和tuner error线电平模式的设置请咨询硬件工程师\n
并参考tuner的用户手册，然后设置为合适的值。CNend
\param[in] enPortId Port ID.CNcomment:端口号。CNend
\param[in] pstAttr  Pointer to port attributes.CNcomment:指针类型，端口属性。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetTSPortAttr(MT_UNF_DMX_PORT_E enPortId, const MT_UNF_DMX_PORT_ATTR_S *pstAttr);


/**   This function is not supported.
\brief Obtains the attributes of a DEMUX TSO port.CNcomment:获取DEMUX TS输出端口属性。CNend
\attention \n
 N/A.CNcomment:无。CNend
\param[in] enPortId Port ID.CNcomment:端口号。CNend
\param[out] pstAttr Pointer to the port attributes (output).CNcomment:指针类型，输出端口属性。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid.CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null.CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetTSOPortAttr(MT_UNF_DMX_TSO_PORT_E enPortId, MT_UNF_DMX_TSO_PORT_ATTR_S *pstAttr);


/**    This function is not supported.
\brief Sets the attributes of a DEMUX TSO port.CNcomment:设置DEMUX TS 输出端口属性。CNend
\attention \n
It is recommended to query the default attributes of a DEMUX TSO port by calling MT_UNF_DMX_GetTSOPortAttr before modifying the attributes of the TSO port.
CNcomment:建议先通过MT_UNF_DMX_GetTSPortAttr获取默认属性，然后对要改变的属性进行设置。CNend
\param[in] enPortId Port ID.CNcomment:端口号。CNend
\param[in] pstAttr  Pointer to port attributes.CNcomment:指针类型，端口属性。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetTSOPortAttr(MT_UNF_DMX_TSO_PORT_E enPortId, MT_UNF_DMX_TSO_PORT_ATTR_S *pstAttr);

/**   This function is not supported.
\brief Get the tag attributes.CNcomment:获取tag 的相关属性。CNend
\attention \n
pstAttr.au8Tag is a key index, this interface will return corresponding tag attrs if this key value has registered, otherwise return default tag attrs.
 N/A.CNcomment:pstAttr.au8Tag参数是一个索引，这个接口会返回对应的tag参数如果这个索引已经被注册，否则的话返回默认的tag属性配置。CNend
\param[in] u32DmxId  DEMUX ID.CNcomment:DEMUX号。CNend
\param[in & out] pstAttr Pointer to the tag port attributes (output).CNcomment:指针类型，tag端口属性。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid.CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetDmxTagAttr(mt_u32 u32DmxId, MT_UNF_DMX_TAG_ATTR_S *pstAttr);

/**   This function is not supported.
\brief Set the tag attributes.CNcomment:设置tag的相关属性。CNend
\attention \n
pstAttr.au8Tag is a key index, it will register this new tag if this key value has not registered. otherwise increase tag reference number.
 N/A.CNcomment:pstAttr.au8Tag参数是一个索引，这个接口会注册新的tag值如果tag未被注册的话，否则会增加tag的应用计数。CNend
\param[in] u32DmxId  DEMUX ID.CNcomment:DEMUX号。CNend
\param[in] pstAttr Pointer to the tag port attributes (output).CNcomment:指针类型，tag端口属性。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid.CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
 */
mt_s32 MT_UNF_DMX_SetDmxTagAttr(mt_u32 u32DmxId, const MT_UNF_DMX_TAG_ATTR_S *pstAttr);


/**
\brief Attaches a DEMUX to a port.CNcomment:绑定DEMUX到端口上。CNend
\attention \n
Each DEMUX can be attached to only one port, but the data from a port can be delivered to multiple DEMUXs.\n
After a DEMUX is attached to a port, the DEMUX starts to receive the data input from this port.\n
If the DEMUX is not used, you can detach it by calling MT_UNF_DMX_DetachTSPort.\n
The DEMUXs can be attached to different ports dynamically. That is, you can attach the DEMUXs without detaching them.\n
If you call this API to attach a DEMUX to the same channel, the error code MT_SUCCESS is returned.
CNcomment:每路DEMUX只能绑定到一个PORT，但是一个PORT的数据可以分发到多个DEMUX\n
DEMUX绑定到端口上之后就开始接收这个端口上输入的数据\n
不再使用这路DEMUX接收数据后，使用MT_UNF_DMX_DetachTSPort接口解绑定DEMUX\n
可以动态的绑定到不同的端口上，不用先解绑定\n
重复调用此接口绑定到相同通道上返回成功。CNend
\param[in] u32DmxId  DEMUX ID.CNcomment:DEMUX号。CNend
\param[in] enPortId  Port ID.CNcomment:端口号。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
::MT_UNF_DMX_DetachTSPort
*/
mt_s32 MT_UNF_DMX_AttachTSPort(mt_u32 u32DmxId, MT_UNF_DMX_PORT_E enPortId);


/**
\brief Detaches a DEMUX from a port.CNcomment:从端口上解绑定DEMUX。CNend
\attention \n
If a DEMUX is not used, you can detach it from the corresponding port by calling this API.\n
This API can be called dynamically, and can be used to disable the input sources of a DEMUX.\n
If this API is called repeatedly, the error code MT_SUCCESS is returned.
CNcomment:不再使用DEMUX接收任何数据时使用此接口将DEMUX从端口上解绑定\n
可以动态使用解绑定接口，可以通过解绑定接口切断DEMUX的输入源\n
重复解绑定返回成功。CNend
\param[in] u32DmxId  DEMUX ID. CNcomment: DEMUX号。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
::MT_UNF_DMX_AttachTSPort
*/
mt_s32 MT_UNF_DMX_DetachTSPort(mt_u32 u32DmxId);


/**
\brief Obtains the ID of the port that is attached to a DEMUX. CNcomment:获取DEMUX所绑定的端口的端口号。CNend
\attention \n
If a DEMUX is not attached to any port, an error code is returned.CNcomment:如果DEMUX没有绑定到任何端口上会返回错误码。CNend
\param[in] u32DmxId  DEMUX ID. CNcomment: DEMUX号。CNend
\param[out] penPortId  Pointer to the ID of the port that is attached to a DEMUX(output).CNcomment:指针类型，输出DEMUX绑定的端口号。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOATTACH_PORT  The DEMUX is not attached to any port.CNcomment:此路DEMUX没有绑定到任何端口上。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetTSPortId(mt_u32 u32DmxId, MT_UNF_DMX_PORT_E *penPortId);


/**
\brief Creates a TS buffer for the RAM port to receive the TS data from Internet or local memory. CNcomment:为RAM端口创建TS Buffer用于接收从网络或本地存储中输入的TS数据。CNend
\attention \n
A TS buffer can be created for the RAM port only.\n
The buffer size must range from 4 KB to 16 MB.\n
It is recommended to set the buffer size to an integral multiple of 4 KB. Otherwise, bottom alignment is performed inside.\n
The TS buffer cannot be created for the same port for multiple times; otherwise, an error code is returned.
CNcomment:只有RAM端口可以创建TS Buffer\n
buffer大小必须在4K和16M之间，单位为字节\n
buffer大小最好为4K的整数倍，如果不是整数倍，内部会进行向下对齐操作\n
同一个端口不能重复创建TS Buffer,否则返回错误码。CNend
\param[in] enPortId  [in]  Port ID.CNcomment:端口号。CNend
\param[in] u32TsBufSize Size of a TS buffer.CNcomment:TS Buffer大小。CNend
\param[out] phTsBuffer  Pointer to the handle of a created TS buffer (output).CNcomment:指针类型，输出创建的TS Buffer 句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_RECREAT_TSBUFFER  The TS buffer corresponding to the port is created.CNcomment:重复创建TS Buffer。CNend
\see \n
N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_E enPortId, mt_u32 u32TsBufSize, mt_handle *phTsBuffer);


/**
\brief Destroys an existing TS buffer. CNcomment:销毁创建的TS Buffer。CNend
\attention \n
A TS buffer cannot be destroyed repeatedly. If a TS buffer is destroyed, its handle is also destroyed.\n
In addition, if a buffer is destroyed, its data is cleared. In this case, the get and put operations are not available.\n
However, the data in channels is retained when a TS buffer is destroyed. To switch streams, you need disable the channel, and then enable the channel again.
CNcomment:不能重复销毁一个TS Buffer,销毁之后handle就不存在了\n
销毁之后buffer中的数据就会被清空，不能再进行Get和Put操作\n
销毁TS buffer并不能清空通道中数据，要切换码流，要注意关闭通道，重新打开。CNend
\param[in] hTsBuffer Handle of a TS buffer.CNcomment:TS Buffer句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_DestroyTSBuffer(mt_handle hTsBuffer);


/**
\brief Obtains a TS buffer to input data.CNcomment:获取TS Buffer空间，用于数据输入。CNend
\attention \n
If you call this API repeatedly, the address of the same buffer is returned.\n
The input data length must be appropriate. If the data length is too large, data cannot be input in time.\n
If the data length is too small, the buffer is scheduled frequently, which causes the system performance to deteriorate.\n
If the size of the available space is smaller than the requested data length, an error code is returned.\n
To be specific, the requested data length must be smaller than the size of the available space, that is, the buffer cannot be full.\n
In addition, the requested data length cannot be 0; otherwise, the error code MT_ERR_DMX_INVALID_PARA is returned.\n
If u32TimeOutMs is set to 0, it indicates that the waiting time is 0; if u32TimeOutMs is set to 0XFFFFFFFF, it indicates infinite wait; if u32TimeOutMs is set to other values, it indicates that the waiting time is u32TimeOutMs ms.\n
If no buffer can be applied for during the block period, the error code MT_ERR_DMX_TIMEOUT is returned.\n
If u32TimeOutMs is set to 0, and no buffer can be applied for, it indicates that the internal TS buffer is full. In this case, you need to call the usleep(10000) function to release the CPU. Therefore, other threads can be scheduled.
CNcomment:重复get返回同一个buffer地址\n
输入数据时注意每次输入的数据长度，太长可能会导致输入数据不及时\n
太短可能会导致调度太频繁，性能下降\n
当剩余空间小于等于请求的长度时，返回错误码\n
每次申请的长度要小于剩余剩余长度，即buffer不可能被填满\n
请求长度不允许为0，否则返回参数非法错误码\n
u32TimeOutMs设置为0表示不等待，设置为0xffffffff表示一直等待，设置为其他值表示等待u32TimeOutMs毫秒。\n
若超过阻塞时间，还无法申请到Buffer，则返回MT_ERR_DMX_TIMEOUT错误码\n
u32TimeOutMs配置为0时，如果申请不到Buffer，说明此时内部TS Buffer空间已满，需要通过usleep(10000)释放cpu
以使其它线程能够得到调度。CNend
\param[in] hTsBuffer Handle of a TS buffer.CNcomment:TS Buffer句柄。CNend
\param[in] u32ReqLen Requested data length.CNcomment:请求数据的长度。CNend
\param[out] pstData  Data buffer.CNcomment:数据buffer结构。CNend
\param[in] u32TimeOutMs      Wait timeout, in ms.CNcomment:等待超时时间，单位ms。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_TIMEOUT  The wait times out. CNcomment:等待超时。CNend
\retval ::MT_ERR_DMX_NOAVAILABLE_BUF  The size of the available buffer is smaller than the requested data length.CNcomment:剩余buffer空间小于请求长度。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetTSBuffer(mt_handle hTsBuffer, mt_u32 u32ReqLen,
            MT_UNF_STREAM_BUF_S *pstData, mt_u32 u32TimeOutMs);

/**
\brief Obtains a TS buffer to input data.CNcomment:获取TS Buffer空间，用于数据输入。CNend
\attention \n
If you call this API repeatedly, the address of the same buffer is returned.\n
The input data length must be appropriate. If the data length is too large, data cannot be input in time.\n
If the data length is too small, the buffer is scheduled frequently, which causes the system performance to deteriorate.\n
If the size of the available space is smaller than the requested data length, an error code is returned.\n
To be specific, the requested data length must be smaller than the size of the available space, that is, the buffer cannot be full.\n
In addition, the requested data length cannot be 0; otherwise, the error code MT_ERR_DMX_INVALID_PARA is returned.\n
If u32TimeOutMs is set to 0, it indicates that the waiting time is 0; if u32TimeOutMs is set to 0XFFFFFFFF, it indicates infinite wait; if u32TimeOutMs is set to other values, it indicates that the waiting time is u32TimeOutMs ms.\n
If no buffer can be applied for during the block period, the error code MT_ERR_DMX_TIMEOUT is returned.\n
If u32TimeOutMs is set to 0, and no buffer can be applied for, it indicates that the internal TS buffer is full. In this case, you need to call the usleep(10000) function to release the CPU. Therefore, other threads can be scheduled.
CNcomment:重复get返回同一个buffer地址\n
输入数据时注意每次输入的数据长度，太长可能会导致输入数据不及时\n
太短可能会导致调度太频繁，性能下降\n
当剩余空间小于等于请求的长度时，返回错误码\n
每次申请的长度要小于剩余剩余长度，即buffer不可能被填满\n
请求长度不允许为0，否则返回参数非法错误码\n
u32TimeOutMs设置为0表示不等待，设置为0xffffffff表示一直等待，设置为其他值表示等待u32TimeOutMs毫秒。\n
若超过阻塞时间，还无法申请到Buffer，则返回MT_ERR_DMX_TIMEOUT错误码\n
u32TimeOutMs配置为0时，如果申请不到Buffer，说明此时内部TS Buffer空间已满，需要通过usleep(10000)释放cpu
以使其它线程能够得到调度。CNend
\param[in] hTsBuffer Handle of a TS buffer.CNcomment:TS Buffer句柄。CNend
\param[in] u32ReqLen Requested data length.CNcomment:请求数据的长度。CNend
\param[out] pstData  Data buffer.CNcomment:数据buffer结构。CNend
\param[out] pu32PhyAddr buffer physical address.CNcomment:buffer的物理地址。CNend
\param[in] u32TimeOutMs      Wait timeout, in ms.CNcomment:等待超时时间，单位ms。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_TIMEOUT  The wait times out. CNcomment:等待超时。CNend
\retval ::MT_ERR_DMX_NOAVAILABLE_BUF  The size of the available buffer is smaller than the requested data length.CNcomment:剩余buffer空间小于请求长度。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetTSBufferEx(mt_handle hTsBuffer, mt_u32 u32ReqLen,
            MT_UNF_STREAM_BUF_S *pstData, phys_addr_t *pu32PhyAddr, mt_u32 u32TimeOutMs);

/**
\brief Updates the write pointer of a TS buffer after the TS data is input.CNcomment:TS数据输入完毕，用于更新TS Buffer写指针。CNend
\attention \n
This API must work with MT_UNF_DMX_GetTSBuffer. That is, if you call MT_UNF_DMX_PutTSBuffer without calling MT_UNF_DMX_GetTSBuffer, no operation takes effect, but the error code MT_SUCCESS is returned.\n
If the valid data length is 0, no operation takes effect, and the error code MT_SUCCESS.\n
The valid data length cannot be greater than data length queried by calling MT_UNF_DMX_GetTSBuffer. Otherwise, the error code MT_ERR_DMX_INVALID_PARA is returned.\n
If the valid data length is smaller than the data length queried by calling MT_UNF_DMX_GetTSBuffer, it indicates that a part of data is stored.
CNcomment:此接口必须和Get接口配对使用，如果没有Get则进行Put操作，则不会进行任何操作，但返回成功\n
Put的长度如果为0，也不会进行任何操作直接返回成功\n
不允许Put长度大于实际Get到的长度，否则返回参数非法错误码\n
允许Put长度小于Get长度，表明只输入了一部分数据\n
为提高效率，Put的长度尽量不要太短，尽量保持在10个ts包以上Put一次，数据太短会导致性能下降\n。CNend
\param[in] hTsBuffer Handle of a TS buffer.CNcomment:TS Buffer句柄。CNend
\param[in] u32ValidDataLen Valid data length.CNcomment:有效数据的长度。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_PutTSBuffer(mt_handle hTsBuffer, mt_u32 u32ValidDataLen);

mt_s32 MT_UNF_DMX_PutTSBuffer_V1(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u8 ts_type, mt_u32 pid);

/**
\brief Updates the write pointer of a TS buffer after the TS data is input.CNcomment:TS数据输入完毕，用于更新TS Buffer写指针。CNend
\attention \n
This API must work with MT_UNF_DMX_GetTSBuffer. That is, if you call MT_UNF_DMX_PutTSBuffer without calling MT_UNF_DMX_GetTSBuffer, no operation takes effect, but the error code MT_SUCCESS is returned.\n
If the valid data length is 0, no operation takes effect, and the error code MT_SUCCESS.\n
The valid data length cannot be greater than data length queried by calling MT_UNF_DMX_GetTSBuffer. Otherwise, the error code MT_ERR_DMX_INVALID_PARA is returned.\n
If the valid data length is smaller than the data length queried by calling MT_UNF_DMX_GetTSBuffer, it indicates that a part of data is stored.
CNcomment:此接口必须和Get接口配对使用，如果没有Get则进行Put操作，则不会进行任何操作，但返回成功\n
Put的长度如果为0，也不会进行任何操作直接返回成功\n
不允许Put长度大于实际Get到的长度，否则返回参数非法错误码\n
允许Put长度小于Get长度，表明只输入了一部分数据\n
为提高效率，Put的长度尽量不要太短，尽量保持在10个ts包以上Put一次，数据太短会导致性能下降\n。CNend
\param[in] hTsBuffer Handle of a TS buffer.CNcomment:TS Buffer句柄。CNend
\param[in] u32ValidDataLen Valid data length.CNcomment:有效数据的长度。CNend
\param[in] u32StartPos start position of the valid data. CNcomment:有效数据的起始位置。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_PutTSBufferEx(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u32 u32StartPos);

/**
\brief Resets a TS buffer to clear its data.CNcomment:复位TS Buffer，清空TS Buffer中的数据。CNend
\attention \n
If a TS buffer is reset, you can call MT_UNF_DMX_PutTSBuffer only after calling MT_UNF_DMX_GetTSBuffer.CNcomment:TS Buffer复位后需要重新get才能执行put操作。CNend
\param[in] hTsBuffer Handle of a TS buffer.CNcomment:TS Buffer句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_ResetTSBuffer(mt_handle hTsBuffer);


/**
\brief Obtains the status of a TS buffer. CNcomment:获取TS Buffer状态。CNend
\attention \n
You can query the size of a TS buffer and the used buffer size by calling this API. CNcomment:可以查询TS Buffer大小和已用的空间大小。CNend
\param[in] hTsBuffer Handle of a TS buffer.CNcomment:TS Buffer句柄。CNend
\param[out] pStatus  TS buffer status.CNcomment:TS Buffer状态结构。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetTSBufferStatus(mt_handle hTsBuffer, MT_UNF_DMX_TSBUF_STATUS_S *pStatus);

/**
\brief Obtains the ID of the port corresponding to a TS buffer.CNcomment:获取TS Buffer对应的Port ID。CNend
\attention \n
If you query the ID of the port corresponding to a TS buffer that is not applied for successfully, an error code is returned.
CNcomment:对于没有申请的TS buffer,查询会返回错误。CNend
\param[in] hTsBuffer Handle of a TS buffer.CNcomment:TS Buffer句柄。CNend
\param[out] penPortId  ID of the port.corresponding to a TS buffer  CNcomment:TS Buffer对应的Port ID。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetTSBufferPortId(mt_handle hTsBuffer, MT_UNF_DMX_PORT_E *penPortId);

/**
\brief Obtains the handle of the TS buffer corresponding to a port ID.CNcomment:获取Port ID对应的TS Buffer handle。CNend
\attention \n
If you obtain the handle of a TS buffer that is not created, an error code is returned.CNcomment:对于没有申请的TS buffer,查询会返回错误。CNend
\param[in] enPortId   Port ID. CNcomment:端口号。CNend
\param[out] phTsBuffer Handle of a TS buffer.CNcomment:TS Buffer句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetTSBufferHandle(MT_UNF_DMX_PORT_E enPortId, mt_handle *phTsBuffer);

/**
\brief get swtsi full care
\attention \n
\param[in]  hTsBuffer    Handle of a TS buffer.
\param[out]  p_av_care    the value of av channel's full care.
\param[out]  p_rec_care   the value of av channel's full care.

\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  fails.
*/
mt_s32 MT_UNF_DMX_GetTSBufferFullCare(mt_handle hTsBuffer, MT_UNF_DMX_AV_CHN_FULL_CARE_S *p_av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S *p_rec_care);

/**
\brief config swtsi full care
\attention \n
\param[in]  hTsBuffer  Handle of a TS buffer.
\param[in]  av_care    the value of av channel's full care.
\param[in]  rec_care   the value of av channel's full care.

\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  fails.
\suggest call MT_UNF_DMX_GetTSBufferFullCare before this function, 
\and only modify the related bits
*/
mt_s32 MT_UNF_DMX_SetTSBufferFullCare(mt_handle hTsBuffer, MT_UNF_DMX_AV_CHN_FULL_CARE_S av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S rec_care);

/**
\brief Obtains the information about TS packets of a port.CNcomment:获取端口的TS包信息。CNend
\attention \n
If you query the information about an unattached port, an error code is returned.
CNcomment:对于没有绑定的端口,查询会返回错误。CNend
\param[in] enPortId    Port ID. CNcomment:端口号。CNend
\param[out] sPortStat   Number of TS packets .CNcomment:TS包计数信息。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetTSPortPacketNum(MT_UNF_DMX_PORT_E enPortId, MT_UNF_DMX_PORT_PACKETNUM_S *sPortStat);

/**
\brief Obtains the default attributes of a channel.CNcomment:获取通道默认属性。CNend
\attention \n
It is recommended to query the default channel attributes before creating a channel, and then modify the attributes.\n
By default, the attributes of the Section channel are returned. If other channels are used, you need to modify the attributes.
CNcomment:最好在申请通道之前使用此接口获取默认属性，然后改变需要修改的属性即可\n
默认属性按一般的section通道返回的，如果要使用其他类型的通道注意修改属性值。CNend
\param[out] pstChAttr Pointer to the structure of the channel attributes.CNcomment:通道属性结构体指针。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetChannelDefaultAttr(MT_UNF_DMX_CHAN_ATTR_S *pstChAttr);


/**
\brief Creates a PID channel based on channel attributes.CNcomment:根据通道属性创建一个PID通道。CNend
\attention \n
It is recommended to call MT_UNF_DMX_GetChannelDefaultAttr to query default channel attributes before calling MT_UNF_DMX_CreateChannel.
For the Section channel and ECM/EMM channel, you can set the CRC mode, and dynamically change the CRC mode by calling MT_UNF_DMX_SetChannelAttr.\n
For other channels, the CRC check is always disabled. If you select other CRC modes, the CRC disable mode takes effect by default.\n
You need to set the buffer size when creating a channel. The buffer size is in the unit of byte and the default size is 16 KB.\n
It is recommended to set the buffer size to a value greater than 4 KB for the Section channel, ECM/EMM channel, or post channel. If the buffer size is smaller than 4 KB, the size of 4 KB is used by default.\n
It is recommended to set the buffer size to a value greater than 64 KB for the PES channel. If the buffer size is smaller than 64 KB, the size of 64 KB is used by default.\n
The playing channel and playing/recording channel cannot be created at the same time./n
The audio/video playing channels are allocated and managed by the player.
CNcomment:建议调用此接口之前，建议先调用MT_UNF_DMX_GetChannelDefaultAttr获取通道默认属性\n
只有section类型的通道和ecm emm类型的通道支持设置CRC模式，并且可以通过属性设置接口动态修改CRC模式\n
其他类型通道只支持设置为CRC禁止，如果选择了其他类型的CRC会被默认修改为CRC禁止\n
在申请通道时注意Buffer大小的配置，默认大小为16K，buffer大小的单位为字节\n
Section、ecm emm和POST通道的buffer大小至少为4K，如果小于4K会被默认修改为4K\n
PES通道建议使用64K以上的buffer大小，如果小于64K会被默认修改为64K\n
播放类型和同时播放与录制类型的音视频通道是不能申请的\n
音视频的播放通道由Player统一分配和管理。CNend
\param[in] u32DmxId  DEMUX ID. CNcomment: DEMUX号。CNend
\param[in] pstChAttr  Pointer to the attributes of an input channel. For details, see the description of MT_UNF_DMX_CHAN_ATTR_S.CNcomment:指针类型，输入通道属性。请参见::MT_UNF_DMX_CHAN_ATTR_S。CNend
\param[out] phChannel  Pointer to the handle of an allocated channel.CNcomment:指针类型，输出分配的通道Handle。。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOFREE_CHAN  There is no available channel.CNcomment:没有空闲通道。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_CreateChannel(mt_u32 u32DmxId, const MT_UNF_DMX_CHAN_ATTR_S *pstChAttr,
            mt_handle *phChannel);


/**
\brief Destroys an existing PID channel.CNcomment:销毁创建的PID通道。CNend
\attention \n
If you release an enabled channel, the channel is disabled automatically. If you release a channel that is attached to a filter or key area, the channel is detached from the filter or key area automatically.\n
However, the filter or key area is not released.\n
It is recommended to explicitly detach channels by calling corresponding detach APIs. In this way, resources are managed in a unified manner.
CNcomment:释放通道时如果通道处于打开状态会自动关闭通道，如果通道上绑定了过滤器或密钥区，会自动解绑定\n
但是并不会释放过滤器和密钥区
建议使用解绑定接口进行显式的解绑定操作，对资源的使用进行清晰的管理。CNend
\param[in] hChannel  Channel handle.CNcomment:通道句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_DestroyChannel(mt_handle hChannel);


/**
\brief Obtains the attributes of a channel.CNcomment:获取通道属性。CNend
\attention \n
The API can be called only after a channel is allocated.CNcomment:本接口必须在通道分配以后才可以调用。CNend
\param[in] hChannel   Channel handle.CNcomment:通道Handle。CNend
\param[out] pstChAttr  Pointer to the attributes of a specified channel. For details, see the description of MT_UNF_DMX_CHAN_ATTR_S.CNcomment:指针类型，输出指定通道属性。请参见::MT_UNF_DMX_CHAN_ATTR_S。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid.CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetChannelAttr(mt_handle hChannel, MT_UNF_DMX_CHAN_ATTR_S *pstChAttr);


/**
\brief Sets the attributes of a channel.CNcomment:设置通道属性。CNend
\attention \n
The API can be called only after a channel is allocated.\n
You cannot dynamically modify all attributes except the CRC mode by calling MT_UNF_DMX_SetChannelAttr.\n
To be specific, the CRC mode can be changed dynamically after a channel is created.\n
If you modify other attributes, the error code MT_ERR_DMX_INVALID_PARA is returned.
CNcomment:本接口必须在通道分配以后才可以调用\n
除了CRC模式其他属性都不允许使用MT_UNF_DMX_SetChannelAttr接口进行动态修改\n
CRC模式可以在通道申请后的任意时刻修改，动态生效\n
要修改其他属性会返回参数非法错误码。CNend
\param[in] hChannel   Channel handle.CNcomment:通道句柄。CNend
\param[in] pstChAttr    Pointer to the attributes of an input channel. For details, see the description of MT_UNF_DMX_CHAN_ATTR_S.  CNcomment:指针类型，输入通道属性。请参见 ::MT_UNF_DMX_CHAN_ATTR_S。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetChannelAttr(mt_handle hChannel, const MT_UNF_DMX_CHAN_ATTR_S *pstChAttr);


/**
\brief Sets the PID of a channel.CNcomment:设置通道PID。CNend
\attention \n
If you set the same PID for two channels of a DEMUX, the PID of the second channel is valid, and the PID of the first channel is changed to 0x1FFF.\n
You must set the PID of a channel after the channel is disabled.\n
If you set the PID of a channel to 0x1FFF, the channel cannot receive data. That is, the channel is disabled.
CNcomment:同一路DEMUX的两个通道设置相同的PID。后设置的通道有效，先设置的通道的PID被修改为0x1fff\n
必须在通道关闭的状态下配置通道的PID\n
允许将通道的PID配置为0x1fff,配置为0x1fff后不接收任何数据，相当于关闭通道。CNend
\param[in] hChannel   Channel handle.CNcomment:通道句柄。CNend
\param[in] u32Pid     Channel PID.CNcomment:通道PID。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_OPENING_CHAN  The channel is enabled.CNcomment:通道处于打开状态。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetChannelPID(mt_handle hChannel, mt_u32 u32Pid);


/**
\brief Obtains the PID of a channel.CNcomment:获取通道配置的PID。CNend
\attention \n
If no PID is set for a channel, the obtained value is 0x1FFF.\n
You can obtain the channel PID after a channel is created.
CNcomment:没有配置PID前，获取到的PID为0x1fff\n
可以在通道申请后的任意时刻获取通道PID。CNend
\param[in] hChannel  Channel handle.CNcomment:通道句柄。CNend
\param[out] pu32Pid   Pointer to the PID of a channel (output).CNcomment:指针类型，输出通道的PID。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetChannelPID(mt_handle hChannel, mt_u32 *pu32Pid);


/**
\brief Enables a channel.CNcomment:打开通道。CNend
\attention \n
If you call this API to enable a PID channel that has been enabled, no error occurs.\n
If you call this API to enable a Section channel that is not attached to a filter, the channel cannot receive data.\n
If you want to receive data without filter, you can create a filter and then set its filter depth to 0.
CNcomment:在PID通道打开状态下，调用本接口打开通道不会发生错误\n
对于section通道，在通道没有绑定过滤器时调用本接口，通道不能接收数据\n
如果不想使用filter接收数据，可以申请一个filter将过滤深度配置为0，进行数据接收。CNend
\param[in] hChannel  Channel handle.CNcomment:通道句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_OpenChannel(mt_handle hChannel);


/**
\brief Disables a channel.CNcomment:关闭通道。CNend
\attention \n
If you this API to disable a PID channel that has been disabled, no error occurs.CNcomment:在PID通道关闭状态下，调用本接口关闭通道不会发生错误。CNend
\param[in] hChannel   Channel handle.CNcomment:通道句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_CloseChannel(mt_handle hChannel);


/**
\brief Obtains the status of a channel.CNcomment:获取通道状态。CNend
\attention \n
You can call this API after a channel is created.CNcomment:可以在通道申请后的任意时刻调用。CNend
\param[in] hChannel   Channel handle.CNcomment:通道句柄。CNend
\param[out] pstStatus  Pointer to the status of a channel (output). For details, see the description of ::MT_UNF_DMX_CHAN_STATUS_S.CNcomment:指针类型，输出通道状态。请参见::MT_UNF_DMX_CHAN_STATUS_S。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetChannelStatus(mt_handle hChannel, MT_UNF_DMX_CHAN_STATUS_S *pstStatus);

/**
\brief Obtains the ts count of a channel.CNcomment:获取通道ts包计数。CNend
\attention \n
You can call this API after a channel is created.CNcomment:可以在通道申请后的任意时刻调用。CNend
\param[in] hChannel   Channel handle.CNcomment:通道句柄。CNend
\param[out] pu32TsCount  Pointer to the count of a channel (output).CNcomment:指针类型，输出通道ts包计数。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
 */
mt_s32 MT_UNF_DMX_GetChannelTsCount(mt_handle hChannel, mt_u32 *pu32TsCount);



/**  only for av channelHandle

\brief Obtains the handle of the channel with a specific PID.CNcomment:获取配置某PID的通道句柄。CNend
\attention \n
You must specify the DEMUX ID when calling this API, because the channel PIDs of different DEMUXs can be the same.\n
You cannot query the PID that is equal to or greater than 0x1FFF; otherwise, the error code MT_ERR_DMX_INVALID_PARA is returned.\n
If no channel with a specific PID is found, an error code is returned.
CNcomment:因不同DEMUX设备的通道允许设置相同的PID，调用本接口需要指定DEMUX ID\n
不允许查询0x1fff及以上的非法PID值，否则返回参数非法错误码\n
如果没有查询到任何通道设置了要查询的PID，将返回错误码。CNend
\param[in] u32DmxId    DEMUX ID. CNcomment: DEMUX号。CNend
\param[in] u32Pid      Channel PID.CNcomment:通道PID。CNend
\param[out] phChannel Pointer to channel handle.CNcomment: 指针类型，输出通道Handle。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_UNMATCH_CHAN  No matched channel is found.CNcomment:没有查询到匹配通道。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetChannelHandle(mt_u32 u32DmxId , mt_u32 u32Pid, mt_handle *phChannel);


/**   for all 
\brief Obtains the handle of the channel with a specific PID.CNcomment:获取配置某PID的通道句柄。CNend
\attention \n
You must specify the DEMUX ID when calling this API, because the channel PIDs of different DEMUXs can be the same.\n
You cannot query the PID that is equal to or greater than 0x1FFF; otherwise, the error code MT_ERR_DMX_INVALID_PARA is returned.\n
If no channel with a specific PID is found, an error code is returned.
CNcomment:因不同DEMUX设备的通道允许设置相同的PID，调用本接口需要指定DEMUX ID\n
不允许查询0x1fff及以上的非法PID值，否则返回参数非法错误码\n
如果没有查询到任何通道设置了要查询的PID，将返回错误码。CNend
\param[in] u32DmxId    DEMUX ID. CNcomment: DEMUX号。CNend
\param[in] u32Pid      Channel PID.CNcomment:通道PID。CNend
\param[out] phChannel Pointer to channel handle.CNcomment: 指针类型，输出通道Handle。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_UNMATCH_CHAN  No matched channel is found.CNcomment:没有查询到匹配通道。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetChannelHandleByPidType(mt_u32 u32DmxId, mt_u32 u32Pid, MT_UNF_DMX_CHAN_TYPE_E enChannelType, mt_handle *phChannel);

/*
**  for Advca
*/
mt_s32 MT_UNF_DMX_GetAVChannelHandle(mt_u32 u32DmxId, mt_handle *videoChannel, mt_handle *audioChannel);

/**
\brief Obtains the number of available channels of a DEMUX.CNcomment:获取DEMUX设备可用的空闲通道的数量。CNend
\attention \n
The returned number is the number of all remaining channels including the used audio/video channels.\n
Because channels are allocated in a unified manner, the parameter u32DmxId is invalid.
CNcomment:返回的数量为所有通道剩余的数量，包括已被使用的音视频通道\n
由于通道资源统一分配所以参数u32DmxId已经没有意义。CNend
\param[in] u32DmxId         DEMUX ID. CNcomment: DEMUX号。CNend
\param[out] pu32FreeCount   Pointer to the number of available channels of a DEMUX.CNcomment:指针类型，输出DEMUX空闲通道数目。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetFreeChannelCount (mt_u32 u32DmxId, mt_u32 *pu32FreeCount);


/**
\brief Obtains the scrambled flag of the channel data.CNcomment:获取通道数据加扰标志。CNend
\attention \n
You can query the scrambled PES data and ES data.CNcomment:支持PES级加扰和TS级加扰的查询。CNend
\param[in] hChannel   Channel handle.CNcomment:通道句柄。CNend
\param[out] penScrambleFlag   Pointer to the scrambled flag of the channel data (output). For details, see the description of ::MT_UNF_DMX_SCRAMBLED_FLAG_E.CNcomment:指针类型，输出通道数据加扰标志。请参见::MT_UNF_DMX_SCRAMBLED_FLAG_E。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetScrambledFlag(mt_handle hChannel, MT_UNF_DMX_SCRAMBLED_FLAG_E *penScrambleFlag);


/**
\brief Creates a data filter.CNcomment:创建一个数据过滤器。CNend
\attention \n
The maximum depth of a filter is 16 bytes.\n
For details on how to set the attributes of a filter, see the description of ::MT_UNF_DMX_FILTER_ATTR_S.\n
You can set a filter when creating it. If you do not want to filter data, you can set the filter depth to 0.\n
To modify the attributes of a filter, call MT_UNF_DMX_SetFilterAttr.\n
The attributes can be modified dynamically.\n
If a filter is used to filter the Section data, the valid byte is byte 1 and byte 4 to byte 18.\n
If a filter is used to filter the PES data, the valid byte is byte 4 and byte 7 to byte 21.\n
The filter can be used to filter the data of general-purpose channels only, including the Section channel, ECM/EMM channel, and PES channel.\n
If a filter is allocated successfully, phFilter returns the handle of the created filter.
CNcomment:过滤器深度最大为16\n
过滤器属性配置请参考::MT_UNF_DMX_FILTER_ATTR_S\n
创建过滤器时，同时实现对过滤器的设置；若不需要数据过滤，设置过滤器深度为0即可\n
若需要修改过滤器属性，可调用MT_UNF_DMX_SetFilterAttr进行属性修改\n
支持属性的动态修改\n
过滤器用于过滤Section数据时，有效过滤字节为第1字节及第4字节～第18字节\n
过滤器用于过滤PES数据时，有效过滤字节为第4字节及第7字节～第21字节\n
过滤器只适用于通用通道的数据过滤，包括section通道、ECM/EMM通道和PES通道\n
若分配成功，phFilter返回获取的过滤器Handle。CNend
\param[in] u32DmxId    DEMUX ID. CNcomment: DEMUX号。CNend
\param[in] pstFilterAttr   Pointer to the attributes of a filter (input).CNcomment:指针类型，输入过滤器属性。CNend
\param[out] phFilter   Pointer to the handle of the created filter (output).CNcomment:指针类型，输出申请的过滤器Handle。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOFREE_FILTER There is no available filter.CNcomment:没有空闲filter。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_CreateFilter(mt_u32 u32DmxId, const MT_UNF_DMX_FILTER_ATTR_S  *pstFilterAttr,
            mt_handle *phFilter);


/**
\brief Destroys an existing data filter.CNcomment:销毁创建的数据过滤器。CNend
\attention \n
If a filter is attached to a general-purpose channel, you can call this API to release the filter when the channel is enabled or disabled.\n
If no filter is attached to a Section channel, the channel cannot receive data. This restriction is not applicable to other channels.
CNcomment:若过滤器和通用通道绑定，允许在通道打开或者关闭状态下，调用本接口释放过滤器\n
对于section通道当通道上没有任何过滤器绑定时，则不再接收数据，其他通道没有此限制。CNend
\param[in] hFilter  Filter handle.CNcomment:过滤器句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_DestroyFilter(mt_handle hFilter);


/**
\brief Releases all the filters attached to a channel.CNcomment:释放绑定在某个通道上的所有过滤器。CNend
\attention \n
You can call this API to release all filters attached to a channel when the channel is enabled or disabled.CNcomment:允许在通道打开或者关闭状态下，调用本接口释放通道上绑定的所有过滤器。CNend
\param[in] hChannel  Channel handle.CNcomment:通道句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is valid.CNcomment:输入参数非法。CNend
\see \n
N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_DeleteAllFilter(mt_handle hChannel);


/**
\brief Sets the filter criteria of a filter.CNcomment:设置过滤器的过滤条件。CNend
\attention \n
The maximum filter depth u32FilterDepth is 16.\n
For details on how to set the attributes of a filter, see the description of ::MT_UNF_DMX_FILTER_ATTR_S.\n
If a filter is used to filter the Section data, the valid byte is byte 1 and byte 4 to byte 18.\n
If a filter is used to filter the PES data, the valid byte is byte 4 and byte 7 to byte 21.\n
The attributes can be modified dynamically.
CNcomment:过滤器的深度属性u32FilterDepth最大为16\n
过滤器属性配置请参考::MT_UNF_DMX_FILTER_ATTR_S\n
过滤器用于过滤Section数据时，有效过滤字节为第1字节及第4字节～第18字节\n
过滤器用于过滤PES数据时，有效过滤字节为第4字节及第7字节～第21字节\n
支持属性的动态修改。CNend
\param[in] hFilter   Filter handle.CNcomment:过滤器句柄。CNend
\param[in] pstFilterAttr   Pointer to the attributes of a filter (input).CNcomment:指针类型，输入过滤器属性。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_ATTACHED_FILTER  The attributes cannot be set dynamically, because the filter is attached to a channel.CNcomment:过滤器绑定在通道上，不能动态设置。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetFilterAttr(mt_handle hFilter, const MT_UNF_DMX_FILTER_ATTR_S *pstFilterAttr);


/**
\brief Obtains the filter criteria of a filter.CNcomment:获取过滤器的过滤条件。CNend
\attention \n
If you call the API before a filter is allocated, an error code is returned.
CNcomment:在过滤器未分配时调用该接口，返回错误。CNend
\param[in] hFilter   Filter handle.CNcomment:过滤器句柄。CNend
\param[out] pstFilterAttr   Pointer to the attributes of a filter (output). For details, see the description of ::MT_UNF_DMX_FILTER_ATTR_S.CNcomment:指针类型，输出过滤器属性。请参见::MT_UNF_DMX_FILTER_ATTR_S。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetFilterAttr(mt_handle hFilter, MT_UNF_DMX_FILTER_ATTR_S *pstFilterAttr);


/**
\brief  Attaches filters to a specific channel.CNcomment:绑定过滤器到指定的通道。CNend
\attention \n
One or more filters can be attached to a general-purpose channel. All filters are ORed.\n
That is, if the data meets the filter criteria of a filter, the data is transmitted to the DMA buffer.\n
Filters cannot be attached to the audio/video channel and post channel.\n
If a filter is used, it cannot be attached to one or more channels repeatedly.\n
If a filter and a channel do not belong to the same DEMUX, the filter cannot be detached from the channel.\n
A filter can be attached to a general-purpose channel when the channel is enabled or disabled.
CNcomment:一个通用通道可以绑定一个或者多个数据过滤器，所有过滤器之间的过滤关系是或。\n
即只要满足其中任何一个过滤器的过滤条件，当前过滤的数据都将通过过滤而被送入DMA缓冲区\n
不支持音视频通道和POST通道的过滤器绑定\n
不能将一个过滤器重复绑定到一个或多个通道上\n
过滤器与通道不属于同一路DEMUX时，不能进行绑定操作\n
支持通用通道打开或者关闭状态下的过滤器绑定。CNend
\param[in] hFilter  Filter handle.CNcomment:过滤器句柄。CNend
\param[in] hChannel   Channel handle.CNcomment:通道句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_ATTACHED_FILTER  The filter is attached.CNcomment:过滤器已经处于绑定状态。CNend
\retval ::MT_ERR_DMX_NOT_SUPPORT   No filter can be attached to the channel.CNcomment:此类型通道不支持绑定过滤器。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_AttachFilter(mt_handle hFilter, mt_handle hChannel);



/**
\brief Detaches a filter from a specific channel.CNcomment:将过滤器从指定通道上解绑定。CNend
\attention \n
If no filter is attached to a channel, the error code MT_FAILURE is returned when you call this API.\n
If a filter and a channel do not belong to the same DEMUX, an error code is returned.\n
A filter can be detached from a general-purpose channel when the channel is enabled or disabled.
CNcomment:通道上没有绑定过滤器进行解绑定返回失败\n
过滤器与通道不属于同一路DEMUX维护时，返回错误\n
支持通用通道打开或者关闭状态下的过滤器解绑定。CNend
\param[in] hFilter   Filter handle.CNcomment:过滤器句柄。CNend
\param[in] hChannel  Channel handle.CNcomment:通道句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NOATTACH_FILTER  No filter is attached to any channel.CNcomment:filter没有绑定在任何通道上。CNend
\retval ::MT_ERR_DMX_UNMATCH_FILTER  No filter is attached to the channel.CNcomment:filter没有绑定在此通道上。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_DetachFilter(mt_handle hFilter, mt_handle hChannel);


/**
\brief Obtains the handle of the channel to which a filter is attached.CNcomment:获取过滤器所绑定的通道句柄。CNend
\attention \n
Each filter corresponds to a channel. If a filter is not attached to a channel, the error code MT_ERR_DMX_NOATTACH_FILTER is returned.CNcomment:每个过滤器唯一对应一个通道。若过滤器没有绑定任何到通道上，则返回MT_ERR_DMX_NOATTACH_FILTER错误码。CNend
\param[in] hFilter    Filter handle.CNcomment:过滤器句柄。CNend
\param[out] phChannel   Pointer to the handle of the channel to which a filter is attached (output).CNcomment:指针类型，输出过滤器绑定的通道句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOATTACH_FILTER  No filter is attached to any channel.CNcomment:filter没有绑定在任何通道上。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetFilterChannelHandle(mt_handle hFilter, mt_handle *phChannel);


/**
\brief Obtains the number of available filters of a DEMUX.CNcomment:获取DEMUX设备可用的空闲过滤器数量。CNend
\attention \n
The number of unallocated filters is returned. The filters that are allocated but not attached to channels are excluded.
CNcomment:返回当前未分配的过滤器数目，已分配但未绑定到通道上的过滤器不统计。CNend
\param[in] u32DmxId   DEMUX ID. CNcomment: DEMUX号。CNend
\param[out]  pu32FreeCount  Pointer to the number of allocated filters (output).CNcomment:指针类型，输出当前未分配的过滤器数目。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetFreeFilterCount(mt_u32 u32DmxId ,  mt_u32 * pu32FreeCount);


/**
\brief Check whether the channel received data.CNcomment:检测通道是否收到了数据。CNend
\attention \n
When you call this API, the parameter phChannel serves as the input parameter.\n
After you call this API successfully, you can obtain data directly.\n
if call this API not successfully, the error code MT_ERR_DMX_NOAVAILABLE_DATA is returned.\n
if you want to check the pes channel, section channel, post channel received data or not,\n
this API is recommended rather than MT_UNF_DMX_GetDataHandle or MT_UNF_DMX_SelectDataHandle.\n
CNcomment:调用该接口时，phChannel作为输入参数\n
当该接口调用成功时，可以直接从通道上获取数据\n
当该接口调用不成功时，返回::MT_ERR_DMX_NOAVAILABLE_DATA\n。
如果检测pes通道、seciton通道、post通道是否收到了数据，推荐使用这个API，而不是MT_UNF_DMX_GetDataHandle
或 MT_UNF_DMX_SelectDataHandle。CNend
\param[in] phChannel      Channel handle.CNcomment:通道Handle。CNend
\param[in] u32TimeOutMs   Wait timeout, in ms.CNcomment:等待超时时间，单位ms。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOAVAILABLE_DATA  No data is received in any channel.CNcomment:没有任何通道有数据到达。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_CheckDataHandle(mt_handle hChannel, mt_u32 u32TimeOutMs);

/**
\brief Obtains the handles of all channels that receive new data.CNcomment:获取有新数据到达的所有通道的句柄。CNend
\attention \n
When you call this API, the parameter pu32ChNum serves as the input parameter and is used to specify the data length pointed by phChannel. pu32ChNum indicates the number of channels to be queried.\n
The number of channels must be initialized, and the initial value cannot be 0.\n
It is recommended to set the number of channels to the total number. For details about the total number of channels.\n
After you call this API successfully, the parameter pu32ChNum serves as the output parameter and is used to specify the valid data length pointed by phChannel.\n
The block time is configurable (in ms). If u32TimeOutMs is set to 0, it indicates that the block time is 0; if u32TimeOutMs is set to 0xFFFFFFFF, it indicates infinite wait.\n
If no data is received when the block time is exceeded, the error code MT_ERR_DMX_TIMEOUT is returned.\n
If the wait time is 0, and no data is received in any channel, ::MT_ERR_DMX_NOAVAILABLE_DATA is returned.
CNcomment:调用该接口时，pu32ChNum作为输入参数，用于指定phChannel指向数组的长度，表示要查询的通道个数；\n
通道个数必须初始化，且初始值必须为非0值\n
建议将通道个数设置为通道的总个数\n
当该接口调用成功时，pu32ChNum作为输出参数，指定phChannel指向数组的有效长度\n
阻塞时间可以设置，时间单位为毫秒，设置为0不等待，设置为0xffffffff一直等待。\n
若超过阻塞时间，所有通道都没有数据到达，则返回等待超时错误\n
如果等待时间为0，而且没有任何通道有数据则返回::MT_ERR_DMX_NOAVAILABLE_DATA。CNend
\param[out] phChannel      Pointer to the number of channels. The number of channels depends on the value of pu32ChNum.CNcomment:指针类型，指向通道句柄数组。数组长度有pu32ChNum决定。CNend
\param[in] pu32ChNum    Pointer type
                           Input: indicates the number of channel handles to which pChannelHandle points.
                           Output: indicates the number of lower-layer channels that receive new data. The maximum value cannot be greater than the input value.
                           CNcomment: 指针类型。注意:该参数同时作为输入和输出。CNend
                           CNcomment: 输入：pChannelHandle指向数组的长度。CNend
                           CNcomment: 输出：底层有新数据通道的数目；最多不超过输入的数值。CNend
\param[in] u32TimeOutMs      Wait timeout, in ms.CNcomment:等待超时时间，单位ms。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOAVAILABLE_DATA  No data is received in any channel.CNcomment:没有任何通道有数据到达。CNend
\retval ::MT_ERR_DMX_TIMEOUT  The wait times out.CNcomment:等待超时。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_GetDataHandle(mt_handle *phChannel, mt_u32 *pu32ChNum,
            mt_u32 u32TimeOutMs);

/**
\brief Obtains whether data is received in some channels.CNcomment:查询部分通道是否有数据到达。CNend
\attention \n
This API is a supplementary interface of ::MT_UNF_DMX_GetDataHandle. You can call this API when you only want to check whether data is received in some channels.
The handles of the channels to be queried are specified by phWatchChannel, and the number of channels to be queried is specified by u32WatchNum.\n
If phWatchChannel is NULL, all channels are queried. The value of u32WatchNum cannot be 0.\n
The block time is configurable (in ms). If u32TimeOutMs is set to 0, it indicates that the block time is 0; if u32TimeOutMs is set to 0xFFFFFFFF, it indicates infinite wait.\n
If no data is received when the block time is exceeded, the error code MT_ERR_DMX_TIMEOUT is returned.\n
If the wait time is 0, and no data is received in any channel, ::MT_ERR_DMX_NOAVAILABLE_DATA is returned.
CNcomment:用来作为::MT_UNF_DMX_GetDataHandle的补充，当应用中只关注部分通道的数据到达情况时使用此接口\n
被监视通道的句柄由phWatchChannel指定，个数由u32WatchNum指定\n
phWatchChannel为NULL,查询所有通道的数据到达情况，u32WatchNum必须为非0值\n
阻塞时间可以设置，时间单位为毫秒，设置为0不等待，设置为0xffffffff一直等待。\n
若超过阻塞时间，所有通道都没有数据到达，则返回等待超时错误\n
如果等待时间为0，而且没有任何通道有数据则返回::MT_ERR_DMX_NOAVAILABLE_DATA。CNend
\param[in] phWatchChannel   Pointer to the number of handles of the channels to be queried. The number depends on the value of u32WatchNum.CNcomment:指针类型，指向被监视通道的句柄数组。数组长度有u32WatchNum决定。CNend
\param[in] u32WatchNum      Number of channels to be queried.CNcomment:监视通道的个数。CNend
\param[in] phDataChannel    Pointer to the number of handles of the channels that contain data The number depends on the value of pu32ChNum.CNcomment:指针类型，指向存在数据的通道的句柄数组。数组长度有pu32ChNum决定。CNend
\param[out] pu32ChNum       Pointer to the number of lower-layer channels that receive new data. The maximum value cannot be greater than the input value.CNcomment:指针类型。底层有新数据通道的数目；最多不超过被监视的通道的数值。CNend
\param[in] u32TimeOutMs     Wait timeout, in ms.CNcomment:等待超时时间，单位ms。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR   The pointer is null.CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOAVAILABLE_DATA   No data is received in any channel.CNcomment:没有任何通道有数据到达。CNend
\retval ::MT_ERR_DMX_TIMEOUT   The wait times out.CNcomment:等待超时。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_SelectDataHandle(mt_handle *phWatchChannel, mt_u32 u32WatchNum,
            mt_handle *phDataChannel, mt_u32 *pu32ChNum, mt_u32 u32TimeOutMs);


/**
\brief Obtains the received data packets from a specific channel.CNcomment:从指定通道中获取输出的数据包。CNend
\attention \n
Before calling this API, you need to set the value of u32AcquireNum to specify the maximum number of buffers that can be queried. The number of buffers depends on the value of u32AcquireNum.\n
The value of pu32AcquiredNum is not greater than that of u32AcquireNum.\n
For the Section channel, ECM channel, or EMM channel, each data packet contains a complete Section.\n
For the PES channel, each data packet may contain a complete PES. If the PES is too large, it is divided into multiple PES packets to transfer.\n
The enDataType field determines whether a transferred data packet is complete.\n
For the post channel, each data packet may contain one or more complete TS packets. Each TS packet consists of 188 bytes.\n
For the audio/video channel, data cannot be obtained by calling this API. The audio data and video data are transferred to the decoder through internal interfaces.
You cannot call this API repeatedly, but you can call this API once and release buffers for several times. Note that you must release buffers in sequence, and the address and size of the buffer to be released must be consistent with those obtained by calling MT_UNF_DMX_AcquireBuf.
In addition, you can call MT_UNF_DMX_AcquireBuf again only after all data packets are released. Otherwise, the error code MT_ERR_DMX_REACQUIRE_BUF is returned.
CNcomment:在调用本接口之前，给u32AcquireNum赋值，指定允许查询的缓冲区信息最大数目。消息缓冲区数组的长度由u32AcquireNum决定\n
实际获取的缓冲区信息数目pu32AcquiredNum数值不会超过u32AcquireNum\n
对于section通道和ECM/EMM通道，每个数据包包含一个完整的section\n
对于PES通道，每个数据包尽量包含完整的pes，但是如果pes太大，可能要分多个pes包输出，\n
输出的数据是否完整通过数据包结构的enDataType字段指定\n
对于POST类型的通道，每个数据包包含一个或多个完整的TS包，TS包长188字节\n
对于音视频通道是不能通过此接口获取数据的，音视频数据会通过内部接口直接送给解码器进行解码\n
不允许重复调用此接口，可以一次请求多次释放，但是释放必须按顺序，而且释放的地址和长度必须和请求的一致\n
并且只有释放完所有的数据包才能再次请求，否则返回重复请求错误码。CNend
\param[in] hChannel            Channel handle.CNcomment:通道句柄。CNend
\param[in] u32AcquireNum       Maximum number of obtained data packets.CNcomment:请求的数据包的最大个数。CNend
\param[out] pu32AcquiredNum    Pointer to the actual number of obtained data packets.CNcomment:指针类型，输出实际请求到的数据包个数。CNend
\param[out] pstBuf             Pointer to the number of data packets. The number depends on the value of pu32AcquiredNum.CNcomment:指针类型，指向数据包信息数组，数组个数由pu32AcquiredNum决定。CNend
\param[in] u32TimeOutMs        Wait timeout, in ms.CNcomment:等待超时时间，单位为ms。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOAVAILABLE_DATA  There is no valid data in the channel.CNcomment:通道中没有有效数据。CNend
\retval ::MT_ERR_DMX_TIMEOUT  The wait times out.CNcomment:等待超时。CNend
\retval ::MT_ERR_DMX_REACQUIRE_BUF  The data packet has been obtained.CNcomment:重复请求数据包。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_AcquireBuf(mt_handle hChannel, mt_u32 u32AcquireNum,
            mt_u32 * pu32AcquiredNum, MT_UNF_DMX_DATA_S *pstBuf,
            mt_u32 u32TimeOutMs);


/**
\brief Releases the buffers for storing data packets after data packets are processed.CNcomment:处理完数据包的数据后，释放数据包占用的缓冲空间。CNend
\attention \n
If you release buffers by calling this API, the buffers must be those obtained by calling ::MT_UNF_DMX_AcquireBuf, and the addresses and sizes of these buffers cannot be changed.\n
The number of released buffers can be less than the number of obtained buffers.
CNcomment:本接口释放的缓冲区一定是通过::MT_UNF_DMX_AcquireBuf获取的缓冲区，缓冲区的地址和长度不能更改\n
释放的缓冲区个数可以小于获取的缓冲区个数。CNend
\param[in] hChannel        Channel handl.CNcomment:通道句柄。CNend
\param[in] u32ReleaseNum   Number of released buffers.CNcomment:释放数据包个数。CNend
\param[in] pstBuf          Pointer to the number of released buffers. The number depends on the value of u32ReleaseNum.CNcomment:指针类型，指向释放缓冲区信息数组，长度由u32ReleaseNum决定。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_ReleaseBuf(mt_handle hChannel, mt_u32 u32ReleaseNum,
            MT_UNF_DMX_DATA_S *pstBuf);

/**
\brief Creates a record channel. CNcomment: 创建一个录制通道。CNend
\attention \n
A demux can be set only one recording channe.
CNcomment: 一个DEMUX上只能创建一个录制通道。CNend
\param[in]  pstRecAttr  record attribute.   CNcomment: 录制属性。CNend
\param[out] phRecChn    record handle.      CNcomment: 录制句柄。CNend
\retval ::MT_SUCCESS  Success. CNcomment: 成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment: API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment: 模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment: 输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment: 指针参数为空。CNend
\retval ::MT_ERR_DMX_NOFREE_CHAN  There is no available channel. CNcomment: 没有空闲的通道。CNend
\retval ::MT_ERR_DMX_ALLOC_MEM_FAILED  memory allocate failure. CNcomment: 内存分配失败。CNend
\retval ::MT_ERR_DMX_MMAP_FAILED  memory map failed. CNcomment: 内存映射失败。CNend
\see \n
 N/A. CNcomment:无。CNend
*/


mt_s32 MT_UNF_DMX_AcquireEs(mt_handle hChannel, MT_UNF_ES_BUF_S *pEsBuf);
mt_s32 MT_UNF_DMX_ReleaseEs(mt_handle hChannel, MT_UNF_ES_BUF_S *pEsBuf);

mt_s32  MT_UNF_DMX_CreateRecChn(MT_UNF_DMX_REC_ATTR_S *pstRecAttr, mt_handle *phRecChn);

/**
\brief Destroys an existing record channel. CNcomment: 销毁创建的录制通道。CNend
\attention \n
 N/A. CNcomment: 无。CNend
\param[in] hRecChn  record handle. CNcomment: 录制句柄。CNend
\retval ::MT_SUCCESS  Success. CNcomment: 成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment: API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment: 模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment: 输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment: 指针参数为空。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_DestroyRecChn(mt_handle hRecChn);

/**
\brief if the type of record were the MT_UNF_DMX_REC_TYPE_SELECT_PID, it would increase the recorded PID that we need. CNcomment: 录制类型为MT_UNF_DMX_REC_TYPE_SELECT_PID时增加需要录制的PID。CNend
\attention \n
 N/A. CNcomment: 无。CNend
\param[in]  hRecChn     record handle.  CNcomment: 录制句柄。CNend
\param[in]  u32Pid      pid value.      CNcomment: PID值。CNend
\param[out] phChannel   channel handle. CNcomment: 通道句柄。CNend
\retval ::MT_SUCCESS  Success. CNcomment: 成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment: API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment: 模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment: 输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment: 指针参数为空。CNend
\retval ::MT_ERR_DMX_NOT_SUPPORT  This function is not supported. CNcomment: 不支持此功能。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_AddRecPid(mt_handle hRecChn, mt_u32 u32Pid, mt_handle *phChannel);

/**
\brief if the type of record were the MT_UNF_DMX_REC_TYPE_SELECT_PID, it would delete increase PID. CNcomment: 录制类型为MT_UNF_DMX_REC_TYPE_SELECT_PID时删除已增加的PID。CNend
\attention \n
 N/A. CNcomment: 无。CNend
\param[in]  hRecChn     record handle.  CNcomment: 录制句柄。CNend
\param[in]  hChannel    channel handle. CNcomment: 通道句柄。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_DelRecPid(mt_handle hRecChn, mt_handle hChannel);

/**
\brief if the type of record were the MT_UNF_DMX_REC_TYPE_SELECT_PID, it would delete all increase PID. CNcomment: 录制类型为MT_UNF_DMX_REC_TYPE_SELECT_PID时删除所有已增加的PID。CNend
\attention \n
 N/A. CNcomment: 无。CNend
\param[in] hRecChn  record handle. CNcomment: 录制句柄。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_DelAllRecPid(mt_handle hRecChn);

/**
\brief if the type of record were the MT_UNF_DMX_REC_TYPE_ALL_PID, it would remove the specified PID. CNcomment: 录制类型为MT_UNF_DMX_REC_TYPE_ALL_PID时排除指定的PID。CNend
\attention \n
chips do not support the function.
CNcomment: 芯片不支持此功能时返回不支持。CNend
\param[in]  hRecChn record handle.  CNcomment: 录制句柄。CNend
\param[in]  u32Pid  pid value.      CNcomment: PID值。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOT_SUPPORT  This function is not supported. CNcomment: 不支持此功能。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_AddExcludeRecPid(mt_handle hRecChn, mt_u32 u32Pid);

/**
\brief if the type of record were the MT_UNF_DMX_REC_TYPE_ALL_PID, it would cancel the PID that have been removed. CNcomment: 录制类型为MT_UNF_DMX_REC_TYPE_ALL_PID时取消已排除的PID。CNend
\attention \n
chips do not support the function.
CNcomment: 芯片不支持此功能时返回不支持。CNend
\param[in]  hRecChn record handle.  CNcomment: 录制句柄。CNend
\param[in]  u32Pid  pid value.      CNcomment: PID值。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOT_SUPPORT  This function is not supported. CNcomment: 不支持此功能。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_DelExcludeRecPid(mt_handle hRecChn, mt_u32 u32Pid);

/**
\brief if the type of record were the MT_UNF_DMX_REC_TYPE_ALL_PID, it would cancel all the PID that have been removed. CNcomment: 录制类型为MT_UNF_DMX_REC_TYPE_ALL_PID时取消所有已排除的PID。CNend
\attention \n
chips do not support the function.
CNcomment: 芯片不支持此功能时返回不支持。CNend
\param[in] hRecChn  record handle. CNcomment: 录制句柄。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOT_SUPPORT  This function is not supported. CNcomment: 不支持此功能。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_DelAllExcludeRecPid(mt_handle hRecChn);

/**
\brief Starts a recording channel. CNcomment: 启动录制通道。CNend
\attention \n
when the recording channel has been activated, it should use the interface to come back success.
CNcomment: 在录制通道已启动的状态下调用本接口返回成功。CNend
\param[in] hRecChn  record handle. CNcomment: 录制句柄。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_StartRecChn(mt_handle hRecChn);

/**
\brief Stops a recording channel. CNcomment: 停止录制通道。CNend
\attention \n
when the recording channel has been stopped, it should use the interface to come back success.
CNcomment: 在录制通道已停止的状态下调用本接口返回成功。CNend
\param[in] hRecChn  record handle. CNcomment: 录制句柄。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_StopRecChn(mt_handle hRecChn);

/**
\brief Obtains record data. CNcomment: 获取录制数据。CNend
\attention \n
The obtained data must use the ::MT_UNF_DMX_ReleaseRecData function to release.
CNcomment: 调用本接口获取到的数据必须使用::MT_UNF_DMX_ReleaseRecData接口进行释放。CNend
\param[in]  hRecChn         record handle.          CNcomment: 录制句柄。CNend
\param[out] pstRecData      record data.            CNcomment: 录制数据。CNend
\param[in]  u32TimeOutMs    Wait timeout, in ms.    CNcomment: 等待超时时间，单位为毫秒。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_AcquireRecData(mt_handle hRecChn, MT_UNF_DMX_REC_DATA_S *pstRecData, mt_u32 u32TimeoutMs);

/**
\brief Releases record data. CNcomment: 释放获取到的录制数据。CNend
\attention \n
 N/A. CNcomment:无。CNend
\param[in] hRecChn      record handle.  CNcomment: 录制句柄。CNend
\param[in] pstRecData   record data.    CNcomment: 录制数据。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_ReleaseRecData(mt_handle hRecChn, const MT_UNF_DMX_REC_DATA_S *pstRecData);

/**
\brief Obtains index data. CNcomment: 获取索引数据。CNend
\attention \n
when indexing video or audio, it would use the interface to gain indexing data, or it will not support if you return.
CNcomment: 音频或视频索引时才能获取索引数据，否则返回不支持。CNend
\param[in]  hRecChn         record handle.          CNcomment: 录制句柄。CNend
\param[out] pstRecIndex     index data.             CNcomment: 索引数据。CNend
\param[in]  u32TimeOutMs    Wait timeout, in ms.    CNcomment: 等待超时时间，单位为毫秒。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOT_START_REC_CHAN  CNcomment: 录制通道未启动。CNend
\retval ::MT_ERR_DMX_NOT_SUPPORT  This function is not supported. CNcomment: 不支持此功能。CNend
\retval ::MT_ERR_DMX_NOAVAILABLE_DATA  no data. CNcomment: 无数据。CNend
\retval ::MT_ERR_DMX_TIMEOUT  The wait times out. CNcomment:等待超时。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_AcquireRecIndex(mt_handle hRecChn, MT_UNF_DMX_REC_INDEX_S *pstRecIndex, mt_u32 u32TimeoutMs);

/**
\brief Obtains the status of a record buffer. CNcomment: 获取录制缓存的状态。CNend
\attention \n
 N/A. CNcomment:无。CNend
\param[in]  hRecChn         record handle.          CNcomment: 录制句柄。CNend
\param[out] pstBufStatus    record buffer status.   CNcomment: 录制缓存的状态。CNend
\retval ::MT_SUCCESS  Success. CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails. CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized. CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A. CNcomment:无。CNend
*/
mt_s32  MT_UNF_DMX_GetRecBufferStatus(mt_handle hRecChn, MT_UNF_DMX_RECBUF_STATUS_S *pstBufStatus);

/**
\brief Set or get the extra attributes of a demux through the commands.CNcomment:通过命令设置或获取DMX附加属性CNend
\attention \n
Diffrent CMD use diffrent param, for details see the description of MT_UNF_DMX_INVOKE_TYPE_E.\n
CNcomment:不同的命令使用的参数不同，具体信息请参考命令枚举的定义MT_UNF_DMX_INVOKE_TYPE_E\n CNend
\param[in] enCmd   Command type.CNcomment:命令类型。CNend
\param[in] pCmdPara    Pointer to command param.  CNcomment:命令参数CNend
\retval ::MT_SUCCESS Success.CNcomment:成功CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空CNend
\see \n
 N/A. CNcomment:无。CNend
 */
mt_s32 MT_UNF_DMX_Invoke(MT_UNF_DMX_INVOKE_TYPE_E enCmd, const mt_void *pCmdPara);

/**
\brief config t2mi
\attention \n
\param[in]  mpi_t2mi_para   config parameter 
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  Calling this API fails.
*/
mt_void MT_UNF_DMX_T2MIConfig(MT_UNF_DMX_T2MI_CONFIG_S mpi_t2mi_para);

/**
\brief enable t2mi
\attention \n
\must call this api first, then config pid, plpid and so on.
\param[in]  enable 
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  Calling this API fails.
*/
mt_void MT_UNF_DMX_T2MIEnable(mt_u32 enable);

/**
\brief config t2mi input source channel
\attention \n
\param[in]  enPortId
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  Calling this API fails.
*/
mt_void MT_UNF_DMX_T2MISetInCh(MT_UNF_DMX_PORT_E enPortId);

/**
\brief config output destination channel
\attention \n
\param[in]  enPortId 
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  Calling this API fails.
*/
mt_void MT_UNF_DMX_T2MISetOutCh(MT_UNF_DMX_PORT_E enPortId);

/**
\brief config t2mi PID
\attention \n
\param[in]  pid
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  Calling this API fails.
*/
mt_void MT_UNF_DMX_T2MISetPid(mt_u32 pid);

/**
\brief config t2mi pld id
\attention \n
\param[in]  plpid
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  Calling this API fails.
*/
mt_void MT_UNF_DMX_T2MISetPlpid(mt_u32 plpid);

/**
\brief softreset t2mi
\attention \n
\param  na
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  Calling this API fails.
*/
mt_void MT_UNF_DMX_T2MISoftReset(mt_void);

/**
\brief config a channel portid
\attention \n
\param[in]  hChannel    channel handle.
\param[in]  enPortId    port id.   CNcomment: 录制缓存的状态。CNend
\retval ::no.
*/
mt_void MT_UNF_DMX_SetPortId(mt_handle hChannel, MT_UNF_DMX_PORT_E enPortId);

mt_void MT_UNF_DMX_Soft_Reset(mt_void);
mt_void MT_UNF_DMX_HardwareInit(mt_void);

mt_s32 MT_UNF_DMX_CreateLinkRecChn(MT_UNF_DMX_REC_ATTR_S *pstRecAttr, mt_handle *phRecChn);
mt_s32 MT_UNF_DMX_DestroyLinkRecChn(mt_handle hRecChn);
mt_s32 MT_UNF_DMX_AcquireLinkRecData(mt_handle hRecChn, MT_UNF_DMX_REC_DATA_S *pstRecData, mt_u32 u32TimeoutMs);
mt_s32 MT_UNF_DMX_ReleaseLinkRecData(mt_handle hRecChn, const MT_UNF_DMX_REC_DATA_S *pstRecData);
mt_s32 MT_UNF_DMX_AcquireLinkRecIndex(mt_handle hRecChn, MT_UNF_DMX_REC_INDEX_S *pstRecIndex, mt_u32 u32TimeoutMs);

mt_s32 MT_UNF_DMX_FastPlayStart(mt_u32 u32DmxId, MT_UNF_DMX_PORT_E enPortId, mt_handle *phRecChn);
mt_s32 MT_UNF_DMX_FastPlayStop(mt_handle phRecChn);
mt_s32 MT_UNF_DMX_GetEsBuffPhyAddr(mt_handle hChannel, ulong *esBuffPhyAddr, mt_u32 *esBuffSize);

/**
\brief get tsi's clk
\attention \n
\param[out] sel_clk   the value
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  fails.
*/
mt_s32 MT_UNF_DMX_Get_TSI_Clk(MT_UNF_DMX_TSI_CLK_E *sel_clk);

/**
\brief set tsi's clk
\attention \n
\param[in] sel_clk   the value
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  fails.
*/
mt_s32 MT_UNF_DMX_Set_TSI_Clk(MT_UNF_DMX_TSI_CLK_E sel_clk);

/**
\brief get a port's sample ctrl
\attention \n
\param[in]  enPortId    port id.
\param[out] port_ctrl   the value
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  fails.
*/
mt_s32 MT_UNF_DMX_Get_TSPort_Ctrl(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CTRL_S *port_ctrl);

/**
\brief config a port's sample ctrl
\attention \n
\param[in]  enPortId    port id.
\param[in]  port_ctrl   the value
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  fails.
\suggest call MT_UNF_DMX_Get_TSPort_Ctrl before this function, 
\and only modify the related bits
*/
mt_s32 MT_UNF_DMX_Set_TSPort_Ctrl(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CTRL_S port_ctrl);

/**
\brief get a port's sample ctrl
\attention \n
\param[in]  enPortId    port id.
\param[out] port_clk   the value
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  fails.
*/
mt_s32 MT_UNF_DMX_Get_TSPort_Clk(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CLK_SEL_E *port_clk);

/**
\brief config a port's clk
\attention \n
\param[in]  enPortId    port id.
\param[in]  port_clk   the value
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  fails.
*/
mt_s32 MT_UNF_DMX_Set_TSPort_Clk(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CLK_SEL_E port_clk);

/**
\brief get a port's src
\attention \n
\param[in]  enPortId    port id.
\param[out] port_src   the value
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  fails.
*/
mt_s32 MT_UNF_DMX_Get_TSPort_Src(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_SRC_SEL_E *port_src);

/**
\brief set a port's src
\attention \n
\param[in]  enPortId    port id.
\param[in]  port_src   the value
\retval ::MT_SUCCESS  Success.
\retval ::MT_FAILURE  fails.
*/
mt_s32 MT_UNF_DMX_Set_TSPort_Src(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_SRC_SEL_E port_src);


#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 MT_UNF_DMX_Chan_Ci_Enable(mt_handle hChannel, mt_u8 isenable);
mt_s32 MT_UNF_TSI_CI_RecChanSet(mt_handle recchan);
mt_s32 MT_UNF_TSI_CI_SwtsiChanSet(mt_handle swtsi_chan);
mt_s32 MT_UNF_TSI_CI_BufCfg(mt_u8 usecache);
mt_s32 MT_UNF_TSI_CI_ClkCfg(mt_u32 clkdiv);
mt_s32 MT_UNF_TSI_CI_TsintervalCfg(mt_u32 interval);
mt_s32 MT_UNF_TSI_Tsi2_SourceCfg(mt_u8 from_cam,mt_u8 serial);
mt_s32 MT_UNF_TSI_CI_LlnNumstartCfg(mt_u32 lln_num_start);
mt_s32 MT_UNF_TSI_CI_SwtsiByteorderCfg(mt_u32 islittle);
mt_s32 MT_UNF_TSI_CI_SwtsiBufFullCareCfg(mt_handle hChannel,mt_u32 full_care);
mt_s32 MT_UNF_TSI_CI_AhbRdDelayCfg(mt_handle hChannel, mt_u32 delay);
mt_s32 MT_UNF_TSI_CI_GetCiStaus(mt_u8 *pbufstatus,mt_u8 *ci_status);
mt_s32 MT_UNF_TSI_CI_EnableCfg(mt_u8 enable);
#endif

/** @} */  /** <!-- ==== API Declaration End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __MT_UNF_DEMUX_H__ */

