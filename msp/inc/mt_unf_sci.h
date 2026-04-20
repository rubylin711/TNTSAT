/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
#ifndef __MT_UNF_SCI_H__
#define __MT_UNF_SCI_H__

#include "mt_common.h"
#include "mt_error_mpi.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


/**Output configuration of the smart card interface clock (SCICLK) pin*/
/** CNcomment:SCICLK引脚输出配置*/
typedef enum  mtUNF_SCI_MODE_E
{
    MT_UNF_SCI_MODE_CMOS = 0, /**<Complementary metal-oxide semiconductor (CMOS) output*/   /**<CNcomment:CMOS输出*/
    MT_UNF_SCI_MODE_OD, /**<Open drain (OD) output*/                                  /**<CNcomment:OD输出*/
    MT_UNF_SCI_MODE_BUTT
} MT_UNF_SCI_MODE_E;

/*!
@brief 智能卡类型
*/
typedef enum  mtUNF_SCI_TYPE_E
{
    MT_UNF_SCI_DIRECT_CARD = 0, /*正向卡*/   
    MT_UNF_SCI_INVERSE_CARD, /*反向卡*/
} MT_UNF_SCI_TYPE_E;


/*!
@brief 错误返回码
*/
typedef enum mtUNF_SCI_ERROR_E
{
    MT_UNF_SCI_ERR_ID_OVERFLOW = 0x100, //!<智能卡接口id超过最大id号
    MT_UNF_SCI_ERR_INVALID_PROTOCOL,//!<无效7816协议
    MT_UNF_SCI_ERR_OPEN_FAIL,   //!<打开智能卡接口设备失败
    MT_UNF_SCI_ERR_CLOSE_FAIL,     //!<关闭智能卡接口设备失败
    MT_UNF_SCI_ERR_RESET_FAIL,    //!复位智能卡失败
    MT_UNF_SCI_ERR_INVALID_DEV_HANDLE,    //!<无效的智能卡接口设备句柄
    MT_UNF_SCI_ERR_CARD_NOT_READY,    //!<智能卡未准备好
    MT_UNF_SCI_ERR_ATR_BUF_TOO_LITTLE,    //!<ATR buffer小于实际buffer值

} MT_UNF_SCI_ERROR_E;

/*!
@brief 智能卡接口pin脚模式
*/
typedef enum  mtUNF_SCI_PIN_MODE_E
{
    MT_UNF_SCI_PIN_MODE_BY_CHIP = 0, //!<芯片内部驱动pin脚电平
    MT_UNF_SCI_PIN_MODE_BY_PULLUP, //!<外部上拉驱动pin脚电平
    MT_UNF_SCI_PIN_MODE_BUTT
} MT_UNF_SCI_PIN_MODE_E;

/*!
@brief 智能卡接口设备端口
*/
typedef enum mtUNF_SCI_PORT_E
{
    MT_UNF_SCI_PORT0, //!<端口0
    MT_UNF_SCI_PORT1, //!<端口1
    MT_UNF_SCI_PORT_BUTT
} MT_UNF_SCI_PORT_E;

/*!
@brief 智能卡接口卡状态
*/
typedef enum mtUNF_SCI_STATUS_E
{
    MT_UNF_SCI_STATUS_NOCARD = 0,  //!<没有智能卡插入
    MT_UNF_SCI_STATUS_READY,  //!<智能卡已插入
} MT_UNF_SCI_STATUS_E;

/*!
@brief 智能卡接口7816协议
*/
typedef enum mtUNF_SCI_PROTOCOL_E
{
    MT_UNF_SCI_PROTOCOL_T0, //!<T0协议
    MT_UNF_SCI_PROTOCOL_T1, //!<T1协议
    MT_UNF_SCI_PROTOCOL_T14, //!<T14协议
    MT_UNF_SCI_PROTOCOL_BUTT
} MT_UNF_SCI_PROTOCOL_E;

/*!
@brief 电平
*/
typedef enum mtUNF_SCI_LEVEL_E
{
    MT_UNF_SCI_LEVEL_LOW,  //!<低电平
    MT_UNF_SCI_LEVEL_HIGH, //!<高电平
    MT_UNF_SCI_LEVEL_BUTT
} MT_UNF_SCI_LEVEL_E;
/** @}*/  /** <!-- ==== Structure Definition End ====*/


#pragma pack(4)
typedef struct
{
  /*!
    unit is us
    */
  mt_u32 T_cnt;
  /*!
      Trigger mode of card is removed.
      0:The falling edge indicates that card is removed.
      1:The rising edge indicates that card is removed.
    */
  mt_u8 remove_mode;
  /*!
      Hardware deactive feature enable
    */
  mt_u8 enable;
  /*!
      VCC is pulled LOW after rst_to_vcc_cnt x T_cnt
    */
  mt_u8 rst_to_vcc_cnt;
  /*!
      IO is pulled LOW after rst_to_io_cnt x T_cnt
    */
  mt_u8 rst_to_io_cnt;
  /*!
      Clk is pulled LOW after rst_to_clk_cnt x T_cnt
    */
  mt_u8 rst_to_clk_cnt;

  mt_u8 reserved[3];
}MT_UNF_SCI_H_PWROFF_S;
#pragma pack()


/******************************* API Declaration *****************************/
/** \addtogroup      SCI*/
/** @{*/  /** <!-- [SCI] */

/**
 \brief Initializes the SCI.   CNcomment:SCI初始化。CNend
 \attention \n
 \param  N/A                   CNcomment:无。CNend
 \retval 0 Success             CNcomment:成功。CNend

 \see \n
N/A
 */
mt_s32 mt_unf_sci_init(mt_void);

/**
 \brief Deinitializes the SCI. CNcomment:SCI去初始化。CNend
 \attention \n
 \param  N/A                   CNcomment:无。CNend
 \retval 0 Success             CNcomment:0 成功。CNend

 \see \n
N/A
 */
mt_s32 mt_unf_sci_deinit(mt_void);

/**
 \brief Starts an SCI device to initialize it based on the input port and protocol. If there is an SCI card, the card is also initialized.\n
CNcomment:打开SCI（Smart Card Interface）设备，针对输入的端口和协议，进行SCI接口设备的初始化；如果卡存在，则同时对卡进行初始化。CNend
\attention \n
After an SCI device is started, it is initialized based on the default configuration.\n
If you modify the configuration, you need to call MT_UNF_SCI_ResetCard for the modifications to take effect.\n
If you remove and then insert the SCI card, you need to call MT_UNF_SCI_ResetCard to reset the card.\n
You need to set frequency parameters when enabling an SCI device. The actual SCI clock provided by the chip is obtained by using the clock divider.\n
The clock divider is calculated based on the externally transferred clock frequency parameters. The fractional part of the clock divider is discarded during calculation. \n
Therefore, an enhanced error occurs when the SCI clock is obtained by dividing the system clock by the clock divider. You need to set the actual output frequency of \n
the SCI card based on the close output frequency. The SCI clock is calculated as follows: Fsci_clk = Frefclk/[(Clkicc + 1) x 2]. Where, Frefclk is the 96 MHz system clock, \n
Fsci_clk is the SCI clock required by peripherals, and clkicc is the required clock divider of registers. clkicc is calculated as follows: Clkicc = Frefclk/(2 x Fsci_clk) - 1. \n
The clkicc may be a floating point value, but only the integral part is used. Therefore, the configured frequency is different from the actual frequency.\n

CNcomment: 打开后SCI设备采用默认配置对设备进行初始化。\n 
之后如果更改了配置，需要调用MT_UNF_SCI_ResetCard来使配置生效。\n 
之后如果对卡进行拔插，需要调用MT_UNF_SCI_ResetCard来进行复位。\n 
打开设备的时候需要设置频率参数，而芯片实际给SCI 的时钟是由外部传入的 \n
时钟频率参数计算的分频因子分频得来，因为计算公式会舍弃计算的分频因子的小数部分，\n
所以通过系统时钟分频之后给卡的时钟会存在增量误差，实际配置考虑与需求最接近的值来 \n
设置和权衡，具体算法 Fsci_clk = Frefclk/[(Clkicc + 1) x 2];其中Frefclk是系统96M时钟，Fsci_clk是外部需要设置的 \n
sci时钟,实际要配入寄存器分频因子Clkicc = Frefclk/(2 x Fsci_clk) - 1;clkicc 有时候计算出来是浮点数，但只取整，\n
所以设置的频率与实际频率有偏差。CNend

 \param[in] enSciPort     ID of an SCI port. The port ID can be 0 or 1.  CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in] enSciProtocol Protocol type.                                  CNcomment:协议类型。CNend
 \param[in] u32Frequency    Frequency of the SCI card to be set, in kHz. For the T0 and T1 cards, the frequency ranges from 1 MHz to 5 MHz; for the T14 card, the frequency must be set to 6 MHz.  CNcomment:要设置的SCI卡频率。对于T0，T1卡，支持频率1MHz～5MHz；对于T14卡，只支持6MHz。单位为khz。CNend
 \retval 0 Success.                                                       CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  SCI  No SCI device is started.           CNcomment:设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.            CNcomment:参数非法。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_open(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_PROTOCOL_E enSciProtocol, mt_u32 u32Frequency);
/**
 \brief config protocol and freq again .\n
CNcomment:进行SCI接口设备的协议和频率重新配置。CNend
\attention \n
After an SCI device is started, config another protocol and another frequency again.\n

CNcomment: 在sci open后可以调这个接口重新配置协议和频率。CNend

 \param[in] enSciPort     ID of an SCI port. The port ID can be 0 or 1.  CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in] enSciProtocol Protocol type.                                  CNcomment:协议类型。CNend
 \param[in] u32Frequency    Frequency of the SCI card to be set, in kHz. For the T0 and T1 cards, the frequency ranges from 1 MHz to 5 MHz; for the T14 card, the frequency must be set to 6 MHz.  CNcomment:要设置的SCI卡频率。对于T0，T1卡，支持频率1MHz～5MHz；对于T14卡，只支持6MHz。单位为khz。CNend
 \retval 0 Success.                                                       CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  SCI  No SCI device is started.           CNcomment:设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.            CNcomment:参数非法。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_reconfig(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_PROTOCOL_E enSciProtocol, mt_u32 u32Frequency);

/**
 \brief Stops an SCI device.
CNcomment:关闭SCI设备。CNend

 \attention \n
This API cannot be called repeatedly.
CNcomment:重复关闭会失败。CNend

 \param[in] enSciPort     ID of an SCI port. The port ID can be 0 or 1.  CNcomment:SCI端口号，取值范围为0和1。CNend
 \retval 0 Success.                                                       CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT   No SCI device is started.               CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.            CNcomment:参数非法。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_close(MT_UNF_SCI_PORT_E enSciPort);

/**
 \brief Performs a warm reset on an SCI card.
CNcomment:复位SCI卡。CNend

 \attention \n
If no SCI card is inserted, the reset fails.\n
If you modify the configuration of an SCI device, you need to call mt_s32 MT_UNF_SCI_ResetCard for the modifications to take effect.\n
In addition, if you remove and insert an SCI card after calling MT_UNF_SCI_Open, you also need to call mt_s32 MT_UNF_SCI_ResetCard to reset the card.
CNcomment:没有插入卡，复位会失败。\n
当更改了SCI设备的配置后，需要调用该接口使配置生效。\n
在调用打开SCI设备接口后，对卡进行拔插，也需要调用该接口进行复位。CNend

 \param[in] enSciPort   ID of an SCI port. The port ID can be 0 or 1                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in] bWarmResetValid   Reset mode.  MT_TRUE: warm reset; MT_FALSE: cold reset (recommended)    			 CNcomment:复位方式。MT_TRUE: 热复位; MT_FALSE: 冷复位（推荐用这种方式）。CNend
 \retval 0 Success.                                                                                              CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  No SCI device is started.                                                        CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                    CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                             			 CNcomment:不可用的选项。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_resetcard(MT_UNF_SCI_PORT_E enSciPort, MT_BOOL bWarmResetValid);

/**
 \brief Deactivates an SCI card.
CNcomment:去激活SCI卡。CNend

 \attention \n
After an SCI card is deactivated, the card cannot be read or written until it is reset.
CNcomment:去激活卡后，无法读写数据。只有重新复位卡后，才能继续读写。CNend

 \param[in] enSciPort ID of an SCI port. The port ID can be 0 or 1.         CNcomment:SCI端口号，取值范围为0和1。CNend
 \retval 0   Success.                                                        CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT No SCI device is started.                    CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.               CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                 CNcomment:不可用的选项。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_deactivecard(MT_UNF_SCI_PORT_E enSciPort);

/**
 \brief Obtains the ATR data of an SCI card.
CNcomment:获取SCI卡ATR数据。CNend

 \attention \n
N/A
 \param[in]  enSciPort  ID of an SCI port. The port ID can be 0 or 1.                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[out]  pu8AtrBuf   Address for storing the obtained ATR data.                                               CNcomment:获取的ATR数据的存储地址。CNend
 \param[in]  u32AtrBufSize  Length of the ATR data read from the buffer.                                          CNcomment:ATR 数据读取 buffer 长度。CNend
 \param[out]  pu8AtrRcvCount Actual number of ATR data segments.                                                   CNcomment:实际获取的ATR数据个数。CNend
 \retval 0   Success.                                                                                             CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT   No SCI device is started.                                                        CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_NULL_PTR The pointer is invalid.                                                      	  CNcomment: 非法指针。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                     CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_NO_ATR  There is no ATR data.                                                               CNcomment:无ATR数据。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                             			  				  CNcomment:不可用的选项。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_getatr(MT_UNF_SCI_PORT_E enSciPort, mt_u8 *pu8AtrBuf, mt_u32 u32AtrBufSize, mt_u8 *pu8AtrRcvCount);

/**
 \brief Obtains the status of an SCI card.
CNcomment:获取SCI卡状态。CNend

 \attention \n
This API is a non-block API. You can transfer data to an SCI card by calling MT_UNF_SCI_Send or MT_UNF_SCI_Receive only when the card status is MT_UNF_SCI_STATUS_READY.\n
CNcomment:此接口是无阻塞函数，在卡的状态为MT_UNF_SCI_STATUS_READY时，才可调用MT_UNF_SCI_Send或MT_UNF_SCI_Receive接口与卡进行数据交互。CNend

 \param[in] enSciPort  ID of an SCI port. The port ID can be 0 or 1.                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[out] penSCIStatus  Status of an SCI card.                                                                CNcomment: SCI卡状态。CNend
 \retval 0 Success.                                                                                              CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  No SCI device is started.                                                        CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_NULL_PTR The pointer is invalid.                                                      	 CNcomment:非法指针。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                    CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                             			 CNcomment:不可用的选项。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_getcardstatus(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_STATUS_E *penSciStatus);

/**
 \brief Transmits data to an SCI card.
CNcomment:向SCI卡发送数据。CNend

 \attention \n
Data must be transmitted based on application protocols.\n
Data cannot be transmitted consecutively.\n
In addition, data is transmitted and received in block mode. Therefore, when a large number of data is being transmitted, the transmission may fail if the timeout is too small.\n
CNcomment:发送数据需要按照应用协议来发送 \n
不能连续两次发送而中间不接收数据 \n
发送读取都是阻塞的，因此发送大量的数据时，受到超时时间的限制，如果超时值比较短，有可能发送失败。CNend

 \param[in] enSciPort   ID of an SCI port. The port ID can be 0 or 1.                                            CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in]  pSciSendBuf Address for storing the data to be transmitted.                                          CNcomment:发送数据的存储地址。CNend
 \param[in] u32SendLen  Number of data segments (in byte) to be transmitted.                                      CNcomment:发送数据的个数,单位为 BYTE。CNend
 \param[in]  pu32ActLen Number of transmitted data segments (in byte).                                            CNcomment:实际发送数据个数,单位为 BYTE。CNend
 \param[in] u32TimeoutMs Wait timeout (in ms). 0: not blocked; 0xFFFFFFFF: infinite block.                        CNcomment:等待超时值, 单位是毫秒, 0 - 不阻塞, 0xFFFFFFFF-永久阻塞。CNend
 \retval 0 Success.                                                                                               CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  SCI  No SCI device is started.                                                   CNcomment:设备未打开。CNend
 \retval ::MT_ERR_SCI_NULL_PTR  The pointer is invalid.                                                     	 CNcomment:非法指针。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA The parameter is invalid.                                                     CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                             			CNcomment:不可用的选项。CNend
 \retval ::MT_ERR_SCI_SEND_ERR  The transmission operation fails.                                             	CNcomment: 发送操作失败。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_send(MT_UNF_SCI_PORT_E enSciPort, mt_u8 *pSciSendBuf, mt_u32 u32SendLen, mt_u32 *pu32ActLen,
                       mt_u32 u32TimeoutUs);

/**
 \brief Receives data from an SCI card.
CNcomment:从SCI卡接收数据。CNend

 \attention \n
You must set the data length obtained by each upper-layer application based on the protocol. If the length of the obtained data is greater than that of the returned data, this API is returned after timeout occurs.\n
CNcomment:上层应用程序必须根据协议来配置所获取的长度，如果希望获取的长度超出实际能够返回的长度，则只能等到超时到期才能返回。CNend

 \param[in] enSciPort  ID of an SCI port. The port ID can be 0 or 1.                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[out]  pSciReceiveBuf   Address for storing the received data.                                              CNcomment:接收数据的存储地址。CNend
 \param[in] u32ReceiveLen  Number of data segments (in byte) to be received.                                      CNcomment:期望接收数据的个数,单位为 BYTE。CNend
 \param[out]  pu32ActLen   Number of received data segments (in byte).                                             CNcomment:实际接收数据个数,单位为 BYTE。CNend
 \param[in] u32TimeOutMs  Wait timeout (in ms). 0: not blocked; 0xFFFFFFFF: infinite block.                       CNcomment:等待超时值, 单位是毫秒, 0 - 不阻塞, 0xFFFFFFFF-永久阻塞。CNend
 \retval 0 Success.                                                                                               CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT   No SCI device is started.                                                       CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_NULL_PTR  The pointer is invalid.                                                     	 CNcomment: 非法指针。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                    CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                             			 CNcomment:不可用的选项。CNend
 \retval ::MT_ERR_SCI_RECEIVE_ERR  The reception operation fails.                                             	CNcomment:接收操作失败。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_receive(MT_UNF_SCI_PORT_E enSciPort, mt_u8 *pSciReceiveBuf, mt_u32 u32ReceiveLen, mt_u32 *pu32ActLen,
                          mt_u32 u32TimeoutUs);

/**
 \brief Transfer data from an SCI card.
CNcomment:和SCI卡进行数据交换CNend

 \attention \n
You must set the data length obtained by each upper-layer application based on the protocol. If the length of the obtained data is greater than that of the returned data, this API is returned after timeout occurs.\n
CNcomment:上层应用程序必须根据协议来配置所获取的长度，如果希望获取的长度超出实际能够返回的长度，则只能等到超时到期才能返回。CNend

 \param[in] enSciPort  ID of an SCI port. The port ID can be 0 or 1.                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in] pCmd  Address for storing the data to be transmitted.                                             CNcomment:传输数据的存储地址。CNend
 \param[in] CmdLen  Number of data segments (in byte) to be received.                                      CNcomment:期望传输数据的个数,单位为 BYTE。CNend
 \param[out] pResponse  Address for storing the received data.                                             CNcomment:接收数据的存储地址。CNend
 \param[out] rlen  Number of data segments (in byte) to be received.                       CNcomment:实际接收数据个数,单位为 BYTE。CNend
 \retval 0 Success.                                                                                               CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT   No SCI device is started.                                                       CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_NULL_PTR  The pointer is invalid.                                                     	 CNcomment: 非法指针。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                    CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                             			 CNcomment:不可用的选项。CNend
 \retval ::MT_ERR_SCI_RECEIVE_ERR  The reception operation fails.                                             	CNcomment:接收操作失败。CNend
 \see \n
N/A
 */

mt_s32 mt_unf_sci_transfer (MT_UNF_SCI_PORT_E enSciPort, mt_u8 *pCmd, mt_u32 CmdLen, mt_u8 *pResponse,mt_u32 *rlen);

/**
 \brief Sets the active level of the VCCEN signal.
CNcomment:配置VCCEN信号线的有效电平。CNend
 \attention \n
The active level needs to be set based on the circuits of hardware, and the low level is active by default.\n
After changing the active level, you need to call MT_UNF_SCI_ResetCard for the modification take effect.\n
CNcomment:需要根据硬件电路进行配置，默认为低电平有效，更改此项配置后需要调用MT_UNF_SCI_ResetCard才能使新的配置有效。CNend

 \param[in] enSciPort ID of an SCI port. The port ID can be 0 or 1.                                              CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in] enSciVcc  Active level of a signal. MT_FALSE: active low; MT_TRUE: active high  Active level of a signal. MT_FALSE: active low; MT_TRUE: active high.  CNcomment:信号线的有效电平。MT_FALSE：低电平有效，MT_TRUE：高电平有效。CNend
 \retval 0   Success.                                                                                             CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT   No SCI device is started.                                                       CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                    CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                             			 CNcomment:不可用的选项。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_configvccen(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_LEVEL_E enSciVcc);

/**
 \brief Sets the active level of the DETECT signal.
CNcomment:配置DETECT信号线的有效电平。CNend

 \attention \n
The active level needs to be set based on the circuits of hardware, and the high level is active by default.\n
After changing the active level, you need to call MT_UNF_SCI_ResetCard for the modification take effect.\n
CNcomment:需要根据硬件电路进行设置，默认为高电平有效，更改此项配置后需要调用MT_UNF_SCI_ResetCard才能使新的配置有效。CNend

 \param[in] enSciPort  ID of an SCI port. The port ID can be 0 or 1.                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in] enSciDetect Active level of a signal. MT_FALSE: active low; MT_TRUE: active high.                     CNcomment:信号线的有效电平。MT_FALSE：低电平有效，MT_TRUE：高电平有效。CNend
 \retval 0  Success.                                                                                              CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  No SCI device is started.                                                        CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                    CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                             			 CNcomment:不可用的选项。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_configdetect(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_LEVEL_E enSciDetect);

/**
 \brief Sets the mode of a clock signal.
CNcomment:设置时钟线的模式。CNend
 \attention \n
The mode needs to be set based on the circuits of hardware, and the OD mode is selected by default.\n
After changing the mode, you need to call MT_UNF_SCI_ResetCard for the modification take effect.\n
CNcomment:需要根据硬件电路进行设置，默认为OD模式，更改此项配置后需要调用MT_UNF_SCI_ResetCard才能使新的配置有效。CNend

 \param[in] enSciPort  ID of an SCI port. The port ID can be 0 or 1.                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in] enClkMode  Mode of a clock signal.                                                                    CNcomment:时钟线的模式。CNend
 \retval 0 Success.                                                                                               CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  No SCI device is started.                                                        CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                    CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                            			 CNcomment:不可用的选项。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_configclkmode(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_MODE_E enClkMode);

/**
 \brief Sets the mode of a clock signal.
CNcomment:设置RESET线的模式。CNend
 \attention \n
The mode needs to be set based on the circuits of hardware, and the OD mode is selected by default.\n
After changing the mode, you need to call MT_UNF_SCI_ResetCard for the modification take effect.\n
CNcomment:需要根据硬件电路进行设置，默认为OD模式，更改此项配置后需要调用MT_UNF_SCI_ResetCard才能使新的配置有效。CNend

 \param[in] enSciPort  ID of an SCI port. The port ID can be 0 or 1.                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in] enClkMode  Mode of a reset signal.                                                                    CNcomment:时钟线的模式。CNend
 \retval 0 Success.                                                                                               CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  No SCI device is started.                                                        CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                    CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                            			                 CNcomment:不可用的选项。CNend
 \retval ::MT_ERR_SCI_NOTSUPPORT	Current chipset not support config RESET output type.                        CNcomment:当前芯片不支持配置RESET输出类型。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_configresetmode(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_MODE_E enResetMode);

/**
 \brief Sets the mode of a clock signal.
CNcomment:设置POWEREN线的模式。CNend
 \attention \n
The mode needs to be set based on the circuits of hardware, and the OD mode is selected by default.\n
After changing the mode, you need to call MT_UNF_SCI_ResetCard for the modification take effect.\n
CNcomment:需要根据硬件电路进行设置，默认为OD模式，更改此项配置后需要调用MT_UNF_SCI_ResetCard才能使新的配置有效。CNend

 \param[in] enSciPort  ID of an SCI port. The port ID can be 0 or 1.                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in] enClkMode  Mode of a poweren signal.                                                                    CNcomment:时钟线的模式。CNend
 \retval 0 Success.                                                                                               CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  No SCI device is started.                                                        CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                    CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                            			                 CNcomment:不可用的选项。CNend
 \retval ::MT_ERR_SCI_NOTSUPPORT	Current chipset not support config POWEREN output type.                      CNcomment:当前芯片不支持配置POWEREN输出类型。CNend

 \see \n
N/A
 */
mt_s32 mt_unf_sci_configvccenmode(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_MODE_E enVccEnMode);

/**
 \brief Sets the type of the card.
CNcomment:设置卡的类型。CNend
 \attention \n
The type needs to be set based on the card type is selected. default is direct convention card type.\n
CNcomment:需要根据实际使用的卡的类型进行设置CNend

 \param[in] enSciPort  ID of an SCI port. The port ID can be 0 or 1.                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param[in] cardtype  Type of the card.                                                                    CNcomment:卡的类型。CNend
 \retval 0 Success.                                                                                               CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  No SCI device is started.                                                        CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                                    CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                            			                 CNcomment:不可用的选项。CNend
 \retval ::MT_ERR_SCI_NOTSUPPORT	Current chipset not support config POWEREN output type.                      CNcomment:当前芯片不支持配置POWEREN输出类型。CNend

 \see \n
N/A
 */
mt_s32 mt_unf_sci_configtype(MT_UNF_SCI_PORT_E enSciPort, MT_UNF_SCI_TYPE_E cardtype);


/**
 \brief Sets the clock rate factor (in ETU) and baud rate regulator factor for special cards.
CNcomment:对特殊卡需要设置指定etu 时钟率因子，波特率调节因子。CNend

 \attention \n
This API needs to be called only for special cards.
The configured values must match the clock rate conversion factor F and bit rate regulator factor D defined in the protocol.
You can also set the factors to the values defined in the card specifications. Note that the values must be set correctly.
CNcomment:只有特需要求的卡需要设置，普通卡不需要调用此接口设置，
设置的值要与协议中规定的F时钟转率转换因子、D比特率调节因子兼容，
或者设置为卡规范中规定的值，不可随意设置。CNend

 \param [in] 	enSciPort	ID of an SCI port. The port ID can be 0 or 1.                                             CNcomment:SCI端口号，取值范围为0和1。CNend
 \param [in] 	u32ClkFactor  Clock rate conversion factor, ranging from 372 to 2048. For details, see the factor F and card features in the protocol.	CNcomment:时钟转率因子372～2048 ，具体设置参考协议的F因子和卡特性。CNend
 \param [in] 	u32BaudFactor	Baud rate regulator factor 1, 2 x n (n = 1-16). For details, see the factor D and card features in the protocol. CNcomment:波特率校正因子1、2*n (n=1～16) ，具体设置参考协议D因子和卡特性。CNend
 \retval  0 	Success.                                                                                               CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT  No SCI device is started.                                                          CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA The parameter is invalid.                                                       CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                             			   CNcomment:不可用的选项。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_setetufactor(MT_UNF_SCI_PORT_E enSciPort, mt_u32 u32ClkFactor, mt_u32 u32BaudFactor);

/**
 \brief \brief Sets the extra guard time for transmitting two consecutive bytes from the terminal to an IC card. The guard time is the interval between the start edges of two consecutive bytes.
CNcomment:设置终端向IC卡发送连续的两个字节起始沿额外增加的间隔时间。CNend

 \attention \n
This setting is performed based on related features only for the cards that require extra character guard time.
In general, the default value is used or the interval guard time is automatically set based on the ART analysis result.
CNcomment:只有特需要求的卡，需要额外设置字符保护时间的才能根据卡的相关特性设置，
不可随意设置。一般使用系统默认值，或者系统根据ATR解析自动设置。CNend
 \param [in] 	enSciPort	ID of an SCI port. The port ID can be 0 or 1.      	CNcomment:SCI端口号，取值范围为0和1。CNend
 \param [in] 	u32GuardTime  Extra guard time, ranging from 0 to 254 ETU.	   	CNcomment:额外间隔保护的范围0～254  etu。CNend
 \retval 0  Success.	                                                       		CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT   No SCI device is started.                  		CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.               		CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                          CNcomment:不可用的选项。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_setguardtime(MT_UNF_SCI_PORT_E enSciPort, mt_u32 u32GuardTime);

/**
 \brief Sets the baud rate supported by the SCI card through protocol and parameters selection (PPS) negotiation or performs negotiation by using the F and D factors returned by the ATR to switch the protocol of a card.
CNcomment:通过PPS 协商可以设置卡所支持的波特率或者用ATR 传回的F、D因子进行协商，支持多协议卡的协议切换。CNend
\attention \n
PPS negotiation is available only when the SCI card supports this function.
The negotiated baud rate must be supported by the SCI card.
The command words for negotiation must comply with the specifications defined in section 9.2 "PPS request and response" in the 7816-3 protocol.
CNcomment:要进行PPS 协商首先卡必须支持该功能，
要协商的波特率内容必须是卡所规定的支持的波特率范围，
请求协商的命令字需符合7816-3 中9.2 PPS request and response。CNend

 \param [in] 	enSciPort	ID of an SCI port. The port ID is 0 or 1.                                 CNcomment:SCI端口号，取值范围为0和1。CNend
 \param [in] 	pSciSendBuf  PPS negotiation command combined based on card specifications. If the default negotiation mode is used, the command word can be left empty.   CNcomment:根据卡规范组合的PPS协商命令，若使用系统默认的协商方式，命令字的内容可为空。CNend
 \param [in]	Sendlen	   Length of the command word to be transmitted, ranging from 0 bytes to 5 bytes. The value 0 indicates that the default negotiation mode is used.   CNcomment: 发送命令字的长度(0～5),设置为0表示使用系统默认的协商命令方式。CNend
 \param [in]	RecTimeouts  PPS response timeout after commands are transmitted. The value ranges from 1000 us to 10000000 us.  CNcomment: 发送完命令之后，接收PPS 响应的超时时间(1000-10000000),单位是微秒。CNend
 \retval 0 	Success.                                                                               		CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_NOT_INIT   No SCI device is started.                                         		CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                                      		CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_NULL_PTR		The pointer is null.                                          		CNcomment:空指针。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                                          	CNcomment:不可用的选项。CNend
 \retval ::MT_ERR_SCI_RECEIVE_ERR  A reception error occurs.                                       		CNcomment:接收错误。CNend
 \retval ::MT_ERR_SCI_PPS_PTYPE_ERR 	The protocol type for PPS negotiation is incorrect.           		CNcomment:PPS协商的协议类型错误。CNend
 \retval ::MT_ERR_SCI_PPS_FACTOR_ERR 	The F factor and D factor for PPS negotiation are incorrect. 	        CNcomment:PPS协商的F、D因子错误。CNend
 \retval ::MT_ERR_SCI_PPS_NOTSUPPORT_ERR  The PPS negotiation type is not supported.               		CNcomment:不支持的PPS协商类型。CNend

 \see \n
N/A
 */
mt_s32 mt_unf_sci_negotiatepps(MT_UNF_SCI_PORT_E enSciPort, mt_u8 *pSciSendBuf, mt_u32 Sendlen, mt_u32 RecTimeouts);

/**
 \brief  Sets the maximum number of transmission retries after a check error occurs.
CNcomment:设置校验错误后重发送最大次数。CNend
\attention \n
The number can be set only after the SCI card is reset successfully.
CNcomment:要等卡复位成功之后,才能进行设置。CNend

 \param [in] 	enSciPort	ID of an SCI port. The port ID is 0 or 1.                   CNcomment:SCI端口号，取值范围为0和1。CNend
 \param [in] 	TxRetryTimes   Number of transmission retries, ranging from 0 to 7.    	CNcomment:次数范围(0～7)。CNend
 \retval 0 	Success.                                                                  	CNcomment:成功。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.                         	CNcomment:参数非法。CNend
 \retval ::MT_ERR_SCI_INVALID_OPT	The option is invalid.                             	CNcomment:不可用的选项。CNend
 \retval ::MT_ERR_SCI_NO_ATR  	The SCI card fails to be reset.	                       	CNcomment:没复位成功。CNend

 \see \n
N/A
 */
mt_s32 mt_unf_sci_settxretries(MT_UNF_SCI_PORT_E enSciPort, mt_u32 TxRetryTimes);

/**
 \brief \brief Sets the block wait time . The time is the interval between Characters Sent in the Opposite Direction.
CNcomment:设置块等待时间。CNend

 \attention \n

CNcomment:CNend
 \param [in] 	enSciPort	ID of an SCI port. The port ID can be 0 or 1.      	CNcomment:SCI端口号，取值范围为0和1。CNend
 \param [in] 	u32BlkTime  Block wait time.	   	CNcomment:块等待时间。单位ETU CNend
 \retval 0  Success.	                                                       		CNcomment:成功。CNend
 \retval ::MT_UNF_SCI_ERR_OPEN_FAIL   No SCI device is started.                  		CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.               		CNcomment:参数非法。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_setblktimeout(MT_UNF_SCI_PORT_E enSciPort, mt_u32 u32BlkTime);

/**
 \brief \brief Sets the atr character wait time . The time is the interval between Characters Receive in ATR.
CNcomment:设置ATR字符超时时间。CNend
Users are not advised to use it themselves
 \attention \n
不建议用户自己使用
CNcomment:CNend
 \param [in] 	enSciPort	ID of an SCI port. The port ID can be 0 or 1.      	CNcomment:SCI端口号，取值范围为0和1。CNend
 \param [in] 	u32RstTime  Atr Character wait time.	   	CNcomment:ATR字符等待时间。单位ETU CNend
 \retval 0  Success.	                                                       		CNcomment:成功。CNend
 \retval ::MT_UNF_SCI_ERR_OPEN_FAIL   No SCI device is started.                  		CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.               		CNcomment:参数非法。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_setrsttimeout(MT_UNF_SCI_PORT_E enSciPort, mt_u32 u32RstTime);
/**
 \brief \brief Sets the receive character wait time .
CNcomment:设置接收字符超时时间。CNend
Users are not advised to use it themselves
 \attention \n
不建议用户自己使用
CNcomment:CNend
 \param [in] 	enSciPort	ID of an SCI port. The port ID can be 0 or 1.      	CNcomment:SCI端口号，取值范围为0和1。CNend
 \param [in] 	u32ReceTime  Receive  Character wait time.	   	CNcomment:接收字符等待时间。单位ETU CNend
 \retval 0  Success.	                                                       		CNcomment:成功。CNend
 \retval ::MT_UNF_SCI_ERR_OPEN_FAIL   No SCI device is started.                  		CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.               		CNcomment:参数非法。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_setrecetimeout(MT_UNF_SCI_PORT_E enSciPort, mt_u32 u32ReceTime);

/**
 \brief \brief let hardware know recieve completed.
CNcomment:接收完成。CNend
Users are not advised to use it themselves
 \attention \n
不建议用户自己使用
CNcomment:CNend
 \param [in] 	enSciPort	ID of an SCI port. The port ID can be 0 or 1.      	CNcomment:SCI端口号，取值范围为0和1。CNend
 \retval 0  Success.	                                                       		CNcomment:成功。CNend
 \retval ::MT_UNF_SCI_ERR_OPEN_FAIL   No SCI device is started.                  		CNcomment:SCI设备未打开。CNend
 \retval ::MT_ERR_SCI_INVALID_PARA  The parameter is invalid.               		CNcomment:参数非法。CNend
 \see \n
N/A
 */
mt_s32 mt_unf_sci_recev_completed(MT_UNF_SCI_PORT_E enSciPort);

mt_s32 mt_unf_sci_config_overload(MT_UNF_SCI_PORT_E enSciPort,ulong overload);

/** @}*/  /** <!-- ==== API Declaration End ====*/

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MT_UNF_SCI_H__ */
