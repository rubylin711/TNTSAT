/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_UNF_IR_H__
#define __MT_UNF_IR_H__

#include <linux/input.h>
#include "mt_common.h"
#include "mt_error_mpi.h"
//#include "hi_unf_keyled.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define IR_MAX_PLUSE_NUM	(128)

/*************************** Structure Definition ****************************/
/** \addtogroup      IR */
/** @{ */  /** <!-- [IR] */
typedef enum rc_proto_uapi_linux {  //from kernel 'uapi/linux/lirc.h'
	RC_PROTO_UNKNOWN_	= 0,
	RC_PROTO_OTHER_		= 1,
	RC_PROTO_RC5_		= 2,
	RC_PROTO_RC5X_20_	= 3,
	RC_PROTO_RC5_SZ_    = 4,
	RC_PROTO_JVC_		= 5,
	RC_PROTO_SONY12_    = 6,
	RC_PROTO_SONY15_    = 7,
	RC_PROTO_SONY20_    = 8,
	RC_PROTO_NEC_		= 9,
	RC_PROTO_NECX_		= 10,
	RC_PROTO_NEC32_		= 11,
	RC_PROTO_SANYO_		= 12,
	RC_PROTO_MCIR2_KBD_	= 13,
	RC_PROTO_MCIR2_MSE_	= 14,
	RC_PROTO_RC6_0_		= 15,
	RC_PROTO_RC6_6A_20_	= 16,
	RC_PROTO_RC6_6A_24_	= 17,
	RC_PROTO_RC6_6A_32_	= 18,
	RC_PROTO_RC6_MCE_	= 19,
	RC_PROTO_SHARP_		= 20,
	RC_PROTO_XMP_		= 21,
	RC_PROTO_CEC_		= 22,
	RC_PROTO_IMON_		= 23,
	RC_PROTO_RCMM12_    = 24,
	RC_PROTO_RCMM24_    = 25,
	RC_PROTO_RCMM32_    = 26,
	RC_PROTO_XBOX_DVD_	= 27,
	RC_PROTO_PANSNC_7051_   = 28,
	RC_PROTO_HD5_        = 29,
	RC_PROTO_OCN_        = 30,
	RC_PROTO_MAX_        = RC_PROTO_OCN_,
}irda_protocol_t;

#define IRDA_MAX_CHANNEL             4

  /*!
    IrDA wavefilt configuratin
    */
  typedef struct irda_wfilt_cfg
  {
    /*!
      IrDA transport protocol
      */
    mt_u8 protocol;
    /*!
      IrDA idle
      */
    mt_u8 idle;
    /*!
      IRDA XTAL
      */
    mt_u8 irda_xtal;
    /*!
      IrDA wave_len
     */
    mt_u8 addr_len;
    /*!
      IrDA wfilt code
     */
    mt_u64 wfilt_code;
    /*!
      IrDA wfilt mask
     */
    mt_u64 wfilt_mask;
  }irda_wfilt_cfg_t;

typedef struct ir_wavefilter_config{
  MT_U8 irda_protocol;
  MT_U8 irda_wfilt_channel;
  irda_wfilt_cfg_t irda_wfilt_channel_cfg[IRDA_MAX_CHANNEL];
}ir_wavefilter_config_s;

/*
  ---------        -----
  |       |        |
  |       |        |
---       ----------
->|ontime |<--     |
->|      period    |<-
*/
typedef struct ir_pluse_data
{
	MT_U16 ontime[IR_MAX_PLUSE_NUM];
	MT_U16 period[IR_MAX_PLUSE_NUM];
	MT_U16 cur;
	MT_U16 vaild;
}ir_pluse_data_s;


/**status of key*/
/**CNcomment:按键状态*/
#ifndef DEFINED_MT_UNF_KEY_STATUS_E // Redefine in mt_unf_keyled.h
#define DEFINED_MT_UNF_KEY_STATUS_E
typedef enum  mtUNF_KEY_STATUS_E
{
    MT_UNF_KEY_STATUS_DOWN = 0 ,   /**<Pressed*/   /**<CNcomment:按下按键 */
    MT_UNF_KEY_STATUS_HOLD ,       /**<Hold*/      /**<CNcomment:按住不动 */
    MT_UNF_KEY_STATUS_UP ,         /**<Released*/  /**<CNcomment:抬起按键 */

    MT_UNF_KEY_STATUS_BUTT
}MT_UNF_KEY_STATUS_E ;
#endif


/** @} */  /** <!-- ==== Structure Definition End ==== */

/******************************* API Declaration *****************************/
/** \addtogroup      IR */
/** @{ */  /** <!-- [IR] */
/*---IR---*/

/**
\brief Starts the IR device.
CNcomment:\brief 打开IR设备。CNend

\attention \n
This API can be called repeatedly. Key IDs can be received only after you can start the IR device, and then call MT_UNF_IR_Enable. \n
CNcomment:重复调用会返回成功，打开设备后必须再调用MT_UNF_IR_Enable才能正常接收键值。CNend

\param N/A                                                                   CNcomment:无 CNend
\retval MT_SUCCESS Success                                                   CNcomment:成功 CNend
\retval ::MT_ERR_IR_OPEN_ERR   The IR device fails to open                   CNcomment:IR设备打开失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_Init(mt_void);

/*do nothing*/
#define MT_UNF_IR_Open(...) do { } while (0)

/**
\brief Stops the IR device.
CNcomment:\brief 关闭IR设备。CNend

\attention \n
This API can be called repeatedly. \n
CNcomment:重复调用关闭会返回成功。CNend

\param  N/A                                                                  CNcomment:无 CNend
\retval MT_SUCCESS Success                                                   CNcomment:成功 CNend
\retval ::MT_ERR_IR_CLOSE_ERR  The IR device fails to close.                 CNcomment:IR设备关闭失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_DeInit(mt_void);

/*do nothing*/
#define MT_UNF_IR_Close(...) do { } while (0)

/**
\brief Enables One protolcol.
CNcomment:\brief 使能一种协议。CNend
\attention \n
N/A
\param[in] bEnable  IR enable. MT_TRUE: enabled; MT_FALSE: disabled           CNcomment:协议使能开关, MT_TRUE 使能, MT_FALSE 禁用。CNend
\retval MT_SUCCESS Success                                                    CNcomment:成功 CNend
\retval ::MT_ERR_IR_NOT_INIT  The IR device is not initialized.               CNcomment:IR设备未初始化 CNend
\retval ::MT_ERR_IR_INVALID_PARA  The parameter is invalid.                   CNcomment:参数非法 CNend
\retval ::MT_ERR_IR_ENABLE_FAILED It fails to enable IR device.               CNcomment:使能设备失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_Enable(MT_BOOL bEnable, irda_protocol_t u8Protocol);

/**
\brief Get usercode and keycode.
CNcomment:\brief 获取用户码和键码。CNend
\attention \n
N/A
\param[in]  *pszProtocolName    buffer used to save the protocol name       CNcomment:用于存储协议名称的缓冲区首地址 CNend
\param[in]  *pu64KeyId          Key value from kernel                       CNcomment:kernel 返出的标准按键值 CNend
\param[in]  *usrcode     CNcomment:\brief 用户码 CNend
\param[in]  *keycode     CNcomment:\brief 键码 CNend
\retval MT_SUCCESS Success                                                  CNcomment:成功 CNend
\retval ::MT_ERR_IR_INVALID_PARA  The parameter is invalid.                 CNcomment:参数非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_GetUserAndKey(mt_char *pszProtocolName, mt_u64 *pu64KeyId, mt_u32 *Usercode, mt_u32 *Keycode);

/**
\brief Obtains the key values and key status of the remote control.
CNcomment:\brief 获取遥控器的按键值和按键状态 。CNend

\attention \n

\param[out]  penPressStatus  Key status. For details about the definition, see the description of ::MT_UNF_KEY_STATUS_E.  CNcomment:按键状态。具体含义请参考::MT_UNF_KEY_STATUS_E CNend
\param[out]  pu64KeyId  Key value                                                     CNcomment:按键值 CNend
\param[out]  pszProtocolName  used to save first address of the protocol name buffer    CNcomment:用于存储协议名称的缓冲区首地址 CNend
\param[in]   s32NameSize      used to save length of the protocol name buffer           CNcomment:用于存储协议名称的缓冲区长度 CNend
\param[in] u32TimeoutMs  Timeout (in ms). 0: not blocked; 0xFFFFFFFF: infinite block  CNcomment:超时值, 单位是毫秒, 0 - 不阻塞, 0xFFFFFFFF-永久阻塞 CNend
\retval MT_SUCCESS Success                                                            CNcomment:成功 CNend
\retval ::MT_ERR_IR_NOT_INIT  The IR device is not initialized.                       CNcomment:IR设备未初始化 CNend
\retval ::MT_ERR_IR_NULL_PTR  The pointer is invalid.                                 CNcomment: 指针为空 CNend
\retval ::MT_ERR_IR_INVALID_PARA  The parameter is invalid.                           CNcomment:参数非法 CNend
\retval ::MT_ERR_IR_SET_BLOCKTIME_FAILED  The IR device fails to set block time.      CNcomment:设置阻塞时间失败 CNend
\retval ::MT_ERR_IR_READ_FAILED  The IR device fails to read key.                     CNcomment:读取键值和状态失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_GetValueWithProtocol(MT_UNF_KEY_STATUS_E *penPressStatus, mt_u64 *pu64KeyId,
                                      mt_char *pszProtocolName, mt_s32 s32NameSize, mt_u32 u32TimeoutMs);

#define MT_UNF_IR_GetValue(penPressStatus, pu64KeyId, u32TimeoutMs) MT_UNF_IR_GetValueWithProtocol(penPressStatus, \
                                                                                                   pu64KeyId, NULL, 0, \
                                                                                                   u32TimeoutMs)

/**
\brief clear irda key
\retval MT_SUCCESS Success                                                            CNcomment:成功 CNend
\retval ::MT_ERR_IR_NOT_INIT  The IR device is not initialized.                       CNcomment:IR设备未初始化 CNend
\retval ::MT_ERR_IR_INVALID_PARA  The parameter is invalid.                           CNcomment:参数非法 CNend
\retval ::MT_ERR_IR_SET_FETCHMETHOD_FAILED  The IR device fails to set fetch method.  CNcomment:设置获取方式失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_ClearkeyWithProtocol(void);

/**
\brief Set key fetch or symbol fetch from ir driver.
CNcomment:\brief 设定从红外驱动获取的是键值还是裸电平 。CNend

\attention \n
when IR_TYPE=IR_S2 is effective.
CNcomment:当IR_TYPE=IR_S2时有效。CNend

\param[in] mode 0 means key mode. 1 means symbol mode                                 CNcomment:0获取键值，1获取裸电平 CNend
\retval MT_SUCCESS Success                                                            CNcomment:成功 CNend
\retval ::MT_ERR_IR_NOT_INIT  The IR device is not initialized.                       CNcomment:IR设备未初始化 CNend
\retval ::MT_ERR_IR_INVALID_PARA  The parameter is invalid.                           CNcomment:参数非法 CNend
\retval ::MT_ERR_IR_SET_FETCHMETHOD_FAILED  The IR device fails to set fetch method.  CNcomment:设置获取方式失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_SetFetchMode(mt_s32 s32Mode);

/**
\brief Obtains the raw symbols from ir driver.
CNcomment:\brief 获取遥控器的裸电平 。CNend

\attention \n
when IR_TYPE=IR_S2 is effective.
CNcomment:当IR_TYPE=IR_S2时有效。CNend

\param[out]  pu64lower  lower pluse value                                             CNcomment:裸电平对的低位 CNend
\param[out]  pu64upper  upper space value                                             CNcomment:裸电平对的高位 CNend
\param[in] s32TimeoutMs read timeout .                                                CNcomment:读超时时间。CNend

\retval MT_SUCCESS Success                                                            CNcomment:成功 CNend
\retval ::MT_ERR_IR_NOT_INIT  The IR device is not initialized. 					  CNcomment:IR设备未初始化 CNend
\retval ::MT_ERR_IR_NULL_PTR  The pointer is invalid.								  CNcomment: 指针为空 CNend
\retval ::MT_ERR_IR_SET_BLOCKTIME_FAILED  The IR device fails to set block time.      CNcomment:设置阻塞时间失败 CNend
\retval ::MT_ERR_IR_READ_FAILED  The IR device fails to read key.					  CNcomment:读取键值和状态失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_GetSymbol(ir_pluse_data_s *plusedata);

/**
\brief Enables or disables the function of reporting the released status of a key.
CNcomment:\brief 设置是否上报按键弹起状态。CNend

\attention \n
The function is enabled by default.
CNcomment:如不设置，默认为打开。CNend

\param[in] bEnable      Key released enable.  CNcomment:按键弹起有效。CNend
                        0: disabled           CNcomment:0：关闭；CNend
                        1: enabled            CNcomment: 1：使能。CNend
\retval MT_SUCCESS  Success                                             CNcomment:成功 CNend
\retval ::MT_ERR_IR_NOT_INIT  The IR device is not initialized.         CNcomment:IR设备未初始化 CNend
\retval ::MT_ERR_IR_INVALID_PARA  The parameter is invalid.             CNcomment:参数非法 CNend
\retval ::MT_ERR_IR_SET_KEYUP_FAILED  It fails to enable released key.  CNcomment:设置上报按键弹起状态失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_EnableKeyUp(MT_BOOL bEnable);

/**
\brief Enables or disables the function of reporting the same key value. If keys are pressed and held down, data is continuously transmitted to the receive buffer. Therefore, you can enable or disable this function for applications as required.
CNcomment:\brief 设置是否上报重复按键。一直按键时，数据会源源不断的送入到接收缓冲区，因此应用程序可以根据需要来设置是否上报重复按键。CNend

\attention \n
The function is enabled by default.\n
This API must work with MT_UNF_IR_RepKeyTimeoutVal. The API MT_UNF_IR_RepKeyTimeoutVal is used to set the interval of reporting the same key value.\n
If the function of reporting the same key value is enabled, the keys are pressed and held down, and the interval is set to 300 ms, data is reported once every 300 ms.
If the function is disabled, data is reported only once regardless of how long the keys are held down.
CNcomment:如不设置，默认为打开\n
此接口需要和MT_UNF_IR_RepKeyTimeoutVal函数结合使用，由MT_UNF_IR_RepKeyTimeoutVal设置上报重复按键的间隔\n
如果使能了重复按键上报，当按键一直处于按下状态，间隔设为300毫秒，则每300毫秒会上报一次数据\n
如果禁止了重复按键上报，则不论按下多长时间，只上报一次数据。CNend
\param[in] bEnable     Repeat key report enable.    CNcomment:按键产生重复按键功能。CNend
                       0: disabled                  CNcomment:0：关闭；CNend
                       1: enabled                   CNcomment:1：使能。CNend
\retval MT_SUCCESS Success                                                     CNcomment:成功 CNend
\retval ::MT_ERR_IR_NOT_INIT   The IR device is not initialized.               CNcomment:IR设备未初始化 CNend
\retval ::MT_ERR_IR_INVALID_PARA   The parameter is invalid.                   CNcomment:参数非法 CNend
\retval ::MT_ERR_IR_SET_REPEAT_FAILED   It fails to enable repeat key.         CNcomment:设置上报重复键失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_EnableRepKey(MT_BOOL bEnable);

/**
\brief Sets the interval (in ms) of reporting the same key value.
CNcomment:\brief 设置上报重复按键的时间间隔，单位为ms 。CNend

\attention \n
This API is unavailable if the function of reporting the same key value is disabled by calling MT_UNF_IR_IsRepKey.
CNcomment:当MT_UNF_IR_IsRepKey配置为不上报重复按键时，此接口设置无效。CNend

\param[in] u32TimeoutMs   Interval of reporting the same key value. The interval ranges from 0 ms to 65,536 ms.\n
						  The value 0 will be set to 108, and the value bigger than 65536 will be set to 65536 \n
                          CNcomment:上报重复按键的时间间隔，设置范围：0ms～65536ms 。\n
						  等于0的参数会被强制设置成108，大于65536的参数会被强制设置成65536。CNend
\retval MT_SUCCESS Success CNcomment:                                          CNcomment:成功 CNend
\retval ::MT_ERR_IR_NOT_INIT  The IR device is not initialized.                CNcomment:IR设备未初始化 CNend
\retval ::MT_ERR_IR_SET_REPKEYTIMEOUT_FAILED  It fails to set repeat key timeout.  CNcomment:设置上报重复按键间隔失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_SetRepKeyTimeoutAttr(mt_u32 u32TimeoutMs);

/**
\brief Sets the interval (in ms) of reporting the first key and second key.
CNcomment:\brief 设置第一个键和第二个键之间的时间间隔，单位为ms 。CNend

\attention \n
The default value is 200ms.
CNcomment:\brief 初始默认值是200ms。CNend

\param[in] u32TimeoutMs   The interval ranges from 150 ms to 10000 ms.\n
						  and the value bigger than 10000 will be set to 10000 \n
CNcomment:\brief 设置范围:  0 ms - 10000 ms   CNend \n
\retval MT_SUCCESS Success CNcomment: \brief 成功 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_SetKey1and2TimeoutAttr(mt_u32 u32TimeoutMs);

/**
\brief IR Set Wave Filter.
CNcomment:\brief  待机唤醒键参数设置，最多支持四套遥控器。CNend
*/
/**
\brief set keys to wake up STB.
CNcomment:\brief 待机唤醒键参数设置，最多支持四套遥控器。CNend

\param[in]  irconfig .protocol and keys             CNcomment:用于唤醒STB的协议和按键

\param  N/A                                         CNcomment:无 CNend
\retval MT_SUCCESS Success                          CNcomment:成功 CNend
\retval ::MT_FAILURE  fails to get protocols.       CNcomment:失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_SetWaveFilter(ir_wavefilter_config_s *irconfig);
/**
\brief get protocols.
CNcomment:\brief 获取支持的协议。CNend

\param[in]  disprot .protocols disabled             CNcomment:目前不工作的协议
\param[out]  count_dis.number of protocols disabled CNcomment:目前不工作的协议个数
\param[in]  enprot .protocols enabled               CNcomment:目前在工作的协议
\param[out]  count_en.number of protocols enabled   CNcomment:目前在工作的协议个数

\param  N/A                                         CNcomment:无 CNend
\retval MT_SUCCESS Success                          CNcomment:成功 CNend
\retval ::MT_FAILURE  fails to get protocols.       CNcomment:获取协议失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_Get_Protocols(char disprot[][16], mt_u32 *count_dis, 
                                char enprot[][16], mt_u32 *count_en);

/**
\brief enable or disable protocols.
CNcomment:\brief 配置(使能或去使能)协议。CNend

\param[in]  prot .string protocols                          CNcomment:协议
\param[in]  count.number of protocols                       CNcomment:协议个数
\param[in]  flag_enable .enable or not(1:enable 0:disable)  CNcomment:1使能 0去使能

\param  N/A                                         CNcomment:无 CNend
\retval MT_SUCCESS Success                          CNcomment:成功 CNend
\retval ::MT_FAILURE  fails to config protocols.    CNcomment:配置协议失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_Config_Protocols_ByName(char prot[][16], mt_u32 count, mt_s32 flag_enable);

/**
\brief enable or disable protocols.
CNcomment:\brief 配置(使能或去使能)协议。CNend

\param[in]  prot .enum protocols                            CNcomment:协议
\param[in]  count.number of protocols                       CNcomment:协议个数
\param[in]  flag_enable .enable or not(1:enable 0:disable)  CNcomment:1使能 0去使能

\param  N/A                                         CNcomment:无 CNend
\retval MT_SUCCESS Success                          CNcomment:成功 CNend
\retval ::MT_FAILURE  fails to config protocols.    CNcomment:配置协议失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_Config_Protocols_ByType(irda_protocol_t *prot, mt_u32 count, mt_s32 flag_enable);

/**
\brief Starts the IR device.
CNcomment:\brief 打开Linux 标准 IR设备。CNend

\attention \n
This API can be called repeatedly. \n
CNcomment:重复调用会返回成功. CNend

\param N/A                                                                   CNcomment:无 CNend
\retval MT_SUCCESS Success                                                   CNcomment:成功 CNend
\retval ::MT_ERR_IR_OPEN_ERR   The IR device fails to open                   CNcomment:IR设备打开失败 CNend
\see \n
N/A
*/
mt_s32 MT_INPUT_EVENT_Init(mt_void);

/**
\brief Stops the IR device.
CNcomment:\brief 关闭Linux 标准IR设备。CNend

\attention \n
This API can be called repeatedly. \n
CNcomment:重复调用关闭会返回成功。CNend

\param  N/A                                                     CNcomment:无 CNend
\retval MT_SUCCESS Success                                      CNcomment:成功 CNend
\retval ::MT_ERR_IR_CLOSE_ERR  The IR device fails to close.    CNcomment:IR设备关闭失败 CNend
\use me as: 
\see \n
N/A

flow as below:
    MT_INPUT_EVENT_Init
    MT_UNF_IR_Get_Protocols
    MT_UNF_IR_Config_Protocols:enable
    MT_INPUT_EVENT_Add_KeyMap
    do{MT_INPUT_EVENT_Read_Vkey ;}while(1)
    MT_UNF_IR_Config_Protocols:disable
    MT_INPUT_EVENT_DeInit
*/
mt_s32 MT_INPUT_EVENT_DeInit(mt_void);

/**
\brief Add key map.
CNcomment:\brief 添加扫描码和虚拟键映射。CNend

\param  N/A                                                CNcomment:无 CNend
\retval MT_SUCCESS Success                                 CNcomment:成功 CNend
\retval ::MT_FAILURE  fails to add keymap.                 CNcomment:添加单个keymap失败 CNend
\see \n
N/A
*/
mt_s32 MT_INPUT_EVENT_Add_KeyMap(mt_u64 scancode,mt_u32 vkey);

/**
\brief Delete key map.
CNcomment:\brief 删除扫描码和虚拟键映射。CNend

\param  N/A                                                CNcomment:无 CNend
\retval MT_SUCCESS Success                                 CNcomment:成功 CNend
\retval ::MT_FAILURE  fails to delete keymap.              CNcomment:删除单个keymap失败 CNend
\see \n
N/A
*/
mt_s32 MT_INPUT_EVENT_Del_KeyMap(mt_u64 scancode);

/**
\brief Read Vkey.
CNcomment:\brief 读取虚拟按键。CNend

\param[in]  *ev, input event.                               CNcomment:Linux 标准 输入事件
\param[in]  timeout_ms,timeout.                             CNcomment:超时(ms)

\param  N/A                                                 CNcomment:无 CNend
\retval MT_SUCCESS Success                                  CNcomment:成功 CNend
\retval ::MT_FAILURE  fails to get vkey.                    CNcomment:获取虚拟键失败 CNend
\see \n
N/A
*/
mt_s32 MT_INPUT_EVENT_Read_Vkey(struct input_event *ev,mt_s32 timeout_ms);

/**
\brief change type to name.
CNcomment:\brief 把遥控器类型转换成字符串类型。CNend

\param[in]  prot, Protocol type.                      CNcomment:遥控类型
\param[out] pszProtocolName,Protocol name.            CNcomment:遥控名内存
\param[in]  s32NameSize, the size of name.            CNcomment:遥控名内存大小

\param  N/A                                           CNcomment:无 CNend
\retval MT_SUCCESS Success                            CNcomment:成功 CNend
\retval ::MT_FAILURE fails to get name.               CNcomment:获取协议名失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_IR_Protocal_Type2Name(irda_protocol_t prot, mt_char *pszProtocolName, mt_s32 s32NameSize);

/** @} */  /** <!-- ==== API Declaration End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef __MT_UNF_IR_H__ */
