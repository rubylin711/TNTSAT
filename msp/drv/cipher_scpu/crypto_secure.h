/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*! 
 *******************************************************************************
 * @defgroup  IP_AdvancedCA Advanced Conditional Access
 *  @{
 *  \note project:    symphoney
 *  \note chip:     
 *  \file crypto_secure.h
 *  \author     Grant Li
 *  \date       2017/02/20
 *  \version    1.0
 *  \brief      Advanced Conditional Access Module Driver
 *******************************************************************************
 */ 


#ifndef __CRYPTO_SECURE_H__
#define __CRYPTO_SECURE_H__

/**
 * @brief   advanced ca module key size used by cipher engine.
 *
 *  A data structure used to describe key size for all cipher engines.
 */
enum  _EN_ADV_KEYSIZE
{
    ADV_KEYSIZE_64     = 8,    ///< key size bits 64
    ADV_KEYSIZE_128    = 16,    ///< key size bits 128 
    ADV_KEYSIZE_192    = 24,    ///< key size bits 192 
    ADV_KEYSIZE_256    = 32,    ///< key size bits 256 
};


/**
 * @brief   advanced ca module constant

 *
 *  A data structure used to describe all constants.
 */
enum    _EN_ADV_CONSTANT
{
    ADV_MAXKEYSIZE       = ADV_KEYSIZE_256,     ///< max key size
    ADV_MAXIVSIZE        = 16,     ///< max IV size
    ADV_HDMIKEYSIZE      = 304,      ///< HDMI key size
    ADV_MAXKELADDERLIMIT      = 6,      ///< max key ladder counter
};
/**
 * @brief   
 *
 *   used to describe success or error
 */
enum  _EN_ADV_ADDCHECKSTATUE
{
    EN_ADV_ADDCHECKSUCCESS = 0x5a5a5a5a,
    EN_ADV_ADDCHECKERROR = 0xa5a5a5a5,
};
/**
 * @brief   
 *
 *   used to describe crypto channel index
 */
enum  _EN_ADV_CRYPTO_CHANNEL
{
    ADV_CRYPTOCHANNEL_GEN  	= 0, 
    ADV_CRYPTOCHANNEL_SS  	= 1,  
    ADV_CRYPTOCHANNEL_MM	= 2,
    ADV_CRYPTOCHANNEL_RC		= 3, //runtime check
    ADV_MAXCYPROTCHANNEL  = 4,
};

/**
 * @brief   
 *
 *  A data structure used to describe decription or encryption
 */
enum  _EN_ADV_CIPHERENGINE_OP
{
    EN_ADV_CIPHERENGINE_DECRYPT = 0x5a5a5a5a,
    EN_ADV_CIPHERENGINE_ENCRYPT = 0xa5a5a5a5,
};

/**
 * @brief   
 *
 *  describle the root key from otp or input by cpu
 */
enum  _EN_ADV_KEYPATH
{
    ADV_KEYPATH_INPUT      = 0x5a5a5a5a,    ///< key path from input u8key
    ADV_KEYPATH_OTP        = 0xa5a5a5a5,    ///< key path from OTP
};


/**
 * @brief   
 *
 *  select rootkey from which SCK0-7
 */
enum  _EN_ADV_KEYOPTION
{
    ADV_KEYOPTION_FLASH                    ,        // flash key :sck0
    ADV_KEYOPTION_UNIQUE                  ,        // unique key :sck1
    ADV_KEYOPTION_COMMON                 ,       /// common key :sck2
    ADV_KEYOPTION_CW                        ,        ///cw0 :sck3 cw0:sck4
    ADV_KEYOPTION_PVR                         ,        // PVR key :sck5
    ADV_KEYOPTION_DATA                      ,        // data key :sck6
    ADV_KEYOPTION_HDCP                      ,         //mapk :sck7
    ADV_KEYOPTION_ESWCK               ,         //Generate ESWCK for software usage
    ADV_KEYOPTION_RESERVED_0          ,         //reserved, not support for symphoney
    ADV_KEYOPTION_RESERVED_1          ,         //reserved, not support for symphoney
    ADV_KEYOPTION_RESERVED_2          ,         //reserved, not support for symphoney
    ADV_KEYOPTION_RESERVED_3          ,         //reserved, not support for symphoney

};

/**
 * @brief   advanced ca module cipher engine
 *
 *  A data structure used to describe all cipher engines.
 */
enum  _EN_ADV_ALGORITHM
{
    ADV_ALGORITHM_NONE  = 0,    ///< algorithm DES
    ADV_ALGORITHM_DES   = 1,    ///< algorithm DES
    ADV_ALGORITHM_TDES  = 2,    ///< algorithm TDEA (Triple-DES)
    ADV_ALGORITHM_AES   = 3,    ///< algorithm AES
    ADV_ALGORITHM_SHA   = 4,    ///< algorithm SHA
    ADV_ALGORITHM_RSA   = 5,    ///< algorithm RSA
    //ADV_ALGORITHM_XOR   = 6,    ///< algorithm XOR
    ADV_ALGORITHM_CSA         = 7,        ///< algorithm CSA
    ADV_ALGORITHM_SHA224    ,
    ADV_ALGORITHM_SHA256    ,
    ADV_ALGORITHM_SHA384    ,
    ADV_ALGORITHM_SHA512    ,
};

/**
 * @brief   advanced ca module block mode used by cipher engine.
 *
 *  A data structure used to describe block mode for all cipher engines.
 */
enum  _EN_ADV_BLOCKMODE
{
    ADV_BLOCKMODE_ECB    = 0,    ///< block cipher mode ECB
    ADV_BLOCKMODE_CBC    = 1,    ///< block cipher mode CBC
    ADV_BLOCKMODE_CTR   = 2,    ///< block cipher mode RCBC
    ADV_BLOCKMODE_CBCDVS042    = 3,    ///< block cipher mode CFB
    ADV_BLOCKMODE_CBCCTS    = 4,    ///< block cipher mode OFB
    ADV_BLOCKMODE_RCBC    = 5,    ///< block cipher mode CTR
    ADV_BLOCKMODE_ECBCTS    = 6,    ///< block cipher mode CBCCTS
    ADV_BLOCKMODE_CFB    = 8,    ///< block cipher mode CFB8
    ADV_BLOCKMODE_CFB8    = 9,    ///< block cipher mode CFB64
    ADV_BLOCKMODE_CFB64_128    = 10,    ///< block cipher mode OFB8
    ADV_BLOCKMODE_OFB    = 12,    ///< block cipher mode OFB64
    ADV_BLOCKMODE_OFB8    = 13,    ///< block cipher mode OFB64
    ADV_BLOCKMODE_OFB64_128    = 14,    ///< block cipher mode OFB64
};


/**
 * @brief  
 *
 *  cipher engines run in dma mode or cpu mode
 */
enum  _EN_ADV_OPERATOR_MODE
{
    ADV_OPERATOR_DMA   = 0x5a, 
    ADV_OPERATOR_CPU  = 0xa5,    
};

/**
 * @brief   advanced ca module waiting mode used by cipher engine.
 *
 *  A data structure used to describe waiting mode for all cipher engines.
 */
enum  _EN_ADV_CWLOADING
{
    ADV_CWLOADING_OFF    = 0x5a5a5a5a,    ///< waiting mode polling
    ADV_CWLOADING_ON     = 0xa5a5a5a5,        ///< The clr CW loading for pti descramble 
};
/**
 * @brief   advanced ca module waiting mode used by cipher engine.
 *
 *  A data structure used to describe waiting mode for all cipher engines.
 */
enum  _EN_ADV_CWTYPE
{
    ADV_CWTYPE_ODD     = 0x5a5a5a5a,    ///< waiting mode polling
    ADV_CWTYPE_EVEN    = 0xa5a5a5a5,    ///< waiting mode interrupt
};

/**
 * @brief   advanced ca module key size used by cipher engine.
 *
 *  A data structure used to describe key size for all cipher engines.
 */
enum  _EN_ADV_MODUE_OPERATION
{
    EN_ADV_MODUE_OPERATION_M2M   	= 0, 
    EN_ADV_MODUE_OPERATION_PVR   	= 1, 
};

/**
 * @brief   advanced ca module cipher engine information structure
 *
 *  A data structure used to describe rsa key parameters
 */
struct  _EN_RSA_KEYCONFIGINFO
{
    mt_u32                             enKeySize;
    mt_u32                             *p_addr_m;
    mt_u32                             *p_addr_d;
    mt_u32                             *p_addr_e;
};
/**
 * @brief   advanced ca module cipher engine information structure
 *
 *  A data structure used to describe channel, key path, operator mode
 */
struct  _EN_ADV_CIPHERENGINEPRIVATE
{

    enum     _EN_ADV_KEYPATH     enKeyPath;
    enum     _EN_ADV_OPERATOR_MODE   enOperatorMode;
    enum 	_EN_ADV_CRYPTO_CHANNEL           channel_index; //select crypto engine cahnnel 
    mt_u8		key_sel0;
    mt_u8		key_sel1;
};


/**
 * @brief   advanced ca module cipher engine information structure
 *
 *  A data structure used to describe key information
 */
struct  _EN_ADV_KEYCONFIGINFO
{
    enum _EN_ADV_KEYSIZE     enKeySize;	//key size
    mt_u8                              u8Key[ADV_MAXKEYSIZE];	//key data
    enum  _EN_ADV_KEYOPTION         enKeyOption0;  //for 128bit keys
    mt_u32 key_index;
    enum  _EN_ADV_CIPHERENGINE_OP     enOpration;
    enum  _EN_ADV_ALGORITHM           enAlgorithm;
    enum  _EN_ADV_BLOCKMODE           enBlockMode;
    mt_u8                                    u8IV[ADV_MAXIVSIZE];        //only for some algorithm
    struct _EN_RSA_KEYCONFIGINFO          info_RSAkey;            //only rsa used
    mt_u32                                   mt_u32Length;
    mt_u8                                    *pu8Input;
    mt_u8                                    *pu8Output;
};
/**
 * @brief   advanced ca module cipher engine information structure
 *
 *  A data structure used to describe key ladder information
 */
struct  _EN_ADV_CIPHERENGINEINFO
{

    struct  _EN_ADV_KEYCONFIGINFO     *key_infor[ADV_MAXKELADDERLIMIT];
    mt_u8                            u8_process_count; //0 means do not need process
};
/**
 * @brief   advanced ca module cipher engine information structure
 *
 *  A data structure used to describe m2m key information
 */
struct  _EN_ADV_M2MCONFIGINFO
{
    enum _EN_ADV_CWTYPE      enCWType;
    mt_u8                              u8CWIndex;
	mt_u8 				     						u8CWLen;
	mt_u8				     						u8CWAddr;
    struct  _EN_ADV_CIPHERENGINEINFO      enKeyInfor;
    struct  _EN_ADV_KEYCONFIGINFO		data2data_infro;
};

enum  _EN_ADV_RUNTIMESETTING
{
    ADV_RUNTIME_EN     = 0x5a5a5a5a,   
    ADV_RUNTIME_DIS    = 0xa5a5a5a5,  
};
/**
 * @brief   advanced ca module cipher engine information structure
 *
 *  A data structure used to describe runtime check information
 */
struct  _EN_ADV_RUNTIMECHECKINFO
{
    mt_u32 RTSigAddress;
    mt_u32 RTKeyAddress;
    mt_u32 RTEvalueAddress;
    mt_u32 RTTEXStartAddress;
    mt_u32 RTTEXTLen;	
    mt_u32 RTTempAddess;
    mt_u32 RTTempLen;
    enum _EN_ADV_RUNTIMESETTING RTCPCUsed0;
    enum _EN_ADV_RUNTIMESETTING RTCPCUsed1;
    mt_u32 RTPCStart0;
    mt_u32 RTPCStart1;
    mt_u32 RTPCEnd0;
    mt_u32 RTPCEnd1;
};

struct  _EN_ADV_RUNTIMECHECKINFO_SEC
{
    struct  _EN_ADV_RUNTIMECHECKINFO RTCheckInfor;
    enum  _EN_ADV_RUNTIMESETTING RTSetting;
	enum _EN_ADV_RUNTIMESETTING	RTCheckEn;	
	//mt_u32 RTCurrentPos;
	//mt_u32 RTCheckMod;
	//mt_u32 RTCheckLength;
};

enum  _ECPU_RUNTIME_STATUES
{
    ECPU_RUNTIME_IDLE 	 	=0x55555555,
    ECPU_RUNTIME_APCPU     = 0x5a5a5a5a,   
    ECPU_RUNTIME_AVCPU    = 0xa5a5a5a5, 
    ECPU_RUNTIME_SECCPU    = 0xaaaaaaaa,  
};
enum _EN_ADV_RUNTIME_CPU
{
    CPU_AP = 0,
    CPU_AV = 1,
    CPU_SEC = 2,
    CPU_MAX,
};
enum _EN_ADV_RT_CPU_STATUS
{
    EN_RT_CPU_ON = 0x55,
    EN_RT_CPU_END = 0xaa,
};

enum _EN_ADV_CWPVR_ENABLE
{
	ECPU_CWPVR_DISABLE = 0x80000000,
    ECPU_AP_ENABLE = 0x005a,
    ECPU_AV_ENABLE = 0x5a00,
    ECPU_SEC_ENABLE = 0x5a0000,
	ECPU_CWPVRABLE = ECPU_CWPVR_DISABLE | ECPU_AP_ENABLE | ECPU_AV_ENABLE | ECPU_SEC_ENABLE,
};

enum _EN_CIPHERENGINE_STARTSTATUS
{
    EN_CIPHERENGINE_ONGOING	= 0x55,
    EN_CIPHERENGINE_START 		= 0x5a,
    EN_CIPHERENGINE_STOP		 = 0xa5,
};

enum _EN_SET_CFG_PARA_TYPE
{
    EN_SET_OTP_CFG,
    EN_SET_CAID_CFG,
    EN_SET_INST_CFG,
};
struct _EN_ADV_TRANSFER_PARAS
{
    enum  _EN_SET_CFG_PARA_TYPE para_type;
    mt_u8             *pu8Input;
    mt_u32		   mt_u32Length;
};
struct _EN_REG_RW_INFOR
{
	mt_u32 reg_add;
	mt_u32	reg_data;
	
};
#define EN_REG_RW_INFOR_MAX    (32)
struct _EN_REG_RW_TYPE
{
	struct _EN_REG_RW_INFOR pstRegInfor[EN_REG_RW_INFOR_MAX];
	mt_u32			reg_count;
};


typedef enum    _EN_ADV_ADDCHECKSTATUE    EN_ADV_ADDCHECKSTATUE;
typedef enum    _EN_ADV_RUNTIMEPC    		EN_ADV_RUNTIMEPC;
typedef enum    _EN_ADV_KEYSIZE                    EN_ADV_KEYSIZE;
typedef enum    _EN_ADV_CIPHERENGINE_OP     EN_ADV_CIPHERENGINE_OP;
typedef enum    _EN_ADV_CIPHERENGINE_IV     EN_ADV_CIPHERENGINE_IV;
typedef enum    _EN_ADV_KEYPATH                    EN_ADV_KEYPATH;
typedef enum    _EN_ADV_KEYOPTION               EN_ADV_KEYOPTION;
typedef enum    _EN_ADV_ALGORITHM               EN_ADV_ALGORITHM;
typedef enum    _EN_ADV_BLOCKMODE              EN_ADV_BLOCKMODE;
typedef enum    _EN_ADV_OPERATOR_MODE       EN_ADV_OPERATOR_MODE;
typedef enum    _EN_ADV_CWLOADING               EN_ADV_CWLOADING;
typedef enum    _EN_ADV_CWTYPE                      EN_ADV_CWTYPE;
typedef enum	_EN_ADV_MODUE_OPERATION	EN_ADV_MODUE_OPERATION;
typedef enum  _EN_ADV_RUNTIME_CPU			EN_ADV_RUNTIME_CPU;
typedef enum  _ECPU_RUNTIME_STATUES		ECPU_RUNTIME_STATUES;
typedef enum  _EN_ADV_RT_CPU_STATUS		EN_ADV_RT_CPU_STATUS;
typedef enum  _EN_ADV_CWPVR_ENABLE		EN_ADV_CWPVR_ENABLE;
typedef enum _EN_CIPHERENGINE_STARTSTATUS	EN_CIPHERENGINE_STARTSTATUS;
typedef enum _EN_SET_CFG_PARA_TYPE		EN_SET_CFG_PARA_TYPE;

typedef struct  _EN_RSA_KEYCONFIGINFO          EN_RSA_KEYCONFIGINFO;
typedef struct  _EN_ADV_KEYCONFIGINFO          EN_ADV_KEYCONFIGINFO;
typedef struct  _EN_ADV_CIPHERENGINEINFO     EN_ADV_CIPHERENGINEINFO;
//typedef struct  _EN_ADV_CWCONFIGINFO EN_ADV_CWCONFIGINFO;
typedef struct  _EN_ADV_CIPHERENGINEPRIVATE     EN_ADV_CIPHERENGINEPRIVATE;
typedef struct  _EN_ADV_M2MCONFIGINFO		EN_ADV_M2MCONFIGINFO;
typedef struct  _EN_ADV_RUNTIMECHECKINFO	EN_ADV_RUNTIMECHECKINFO;
typedef struct  _EN_ADV_RUNTIMECHECKINFO_SEC	EN_ADV_RUNTIMECHECKINFO_SEC;
typedef struct _EN_ADV_TRANSFER_PARAS		EN_ADV_TRANSFER_PARAS;
typedef struct _EN_REG_RW_INFOR			EN_REG_RW_INFOR;
typedef struct _EN_REG_RW_TYPE			EN_REG_RW_TYPE;

#define	SYMPHONY_LOCK_REG		(0xbf30f020) 
#define	LOCK_CLKMNG_BIT	(0)
#define	LOCK_DDRCTRL_BIT	(2)
#define	LOCK_SWANALOG_BIT	(13)
#define	LOCK_SECCPUREG_BIT	(14)
#define	LOCK_AVCPUBOOT_BIT	(15)	//??? need use now?
#define	LOCK_CRYPTO_BIT	(23)
#define 	LOCK_OTP_BIT	(22)
#define	LOCK_AOANALOG_BIT	(29)

#define	MBOOT_VER_OTPBIT_ADDR	(3776)
#define	MBOOT_VER_BIT_LENGTH		(8)
#define	APP_VER_OTPBIT_ADDR	(MBOOT_VER_OTPBIT_ADDR + MBOOT_VER_BIT_LENGTH)
#define	APP_VER_BIT_LENGTH			(16)
#define	AV_VER_OTPBIT_ADDR	(APP_VER_OTPBIT_ADDR + APP_VER_BIT_LENGTH)
#define	AV_VER_BIT_LENGTH			(8)

#define	OFB64_128_MODE_DIS	(3052)
#define	OFB8_MODE_DIS				(3051)
#define	OFB1_MODE_DIS				(3050)
#define	CFB64_128_MODE_DIS	(3049)
#define	CFB8_MODE_DIS				(3048)
#define	CFB1_MODE_DIS				(3047)
#define	ECBCTS_MODE_DIS			(3046)
#define	RCBCTS_MODE_DIS			(3045)
#define	CBCCTS_MODE_DIS			(3044)
#define	CBCCDVS042_MODE_DIS			(3043)
#define	CTR_MODE_DIS			(3042)
#define	CBC_MODE_DIS			(3041)
#define	DES_DIS			(3040)

#define ECPU_DATA_MAX	(0x1400000)
#define ECPU_ONCE_SHALEN	(100 * 1024)
#define ECPU_RT_DDR_ADRESS_TOP		(0xa4000000 + 0x10000)
#define ECPU_RT_DDR_ADRESS_BOTTORM	(ECPU_RT_DDR_ADRESS_TOP - 0x1000)
#define ECPU_DDR_UNPROTECT_BOTTOM		(0xa0000000)
#define ECPU_DDR_UNPROTECT_TOP		(0xa3D00000)
#define	PVR_BUFFER_TOP_ADDR		(0xa4400000)
#define	PVR_BUFFER_BOTTOM_ADDR	(0xa4040000)

#define   DDR_DISTRUB_INPUT_ADDR		(0xa0100000)
#define   DDR_DISTRUB_OUTPUT_ADDR		(ECPU_RT_DDR_ADRESS_TOP)  
#define   DDR_DISTRUB_LENGTH			(0x30000)  

#define CRYPYO_PROCESS_DATA_MAX	(0x2000000)
#define	SEC_CPU_LENGTH_MAX		(15 * 1024)
#define	SEC_CPU_LENGTH_MIN		(10 * 1024)
#define	OTHER_CPU_LENGTH_MIN		(300 * 1024)

typedef enum{
	MB_T_INVALID = 0,
	//special commands
	MB_T_DDR_READY,
	MB_T_R_REG,
	MB_T_W_REG,
	MB_T_R_OTP,
	MB_T_W_OTP,
	MB_T_RT_APCPU_SETTING,
	MB_T_RT_AVCPU_SETTING,
	MB_T_RT_SECCPU_SETTING,
	MB_T_RT_APPC_SETTING,
	MB_T_RUN_SOS,
	MB_T_MBOOT_VER_CHECK,
	MB_T_APP_VER_CHECK,
	MB_T_AV_VER_CHECK,
	MB_T_DDR_PROTECT,
	MB_T_SET_KEY_DERIV_PARAS,
	MB_T_SET_FLASHHW_KEY,
	MB_T_SET_LOCK_MODULES,

	//normal commnds
	MB_T_FLASH_OP = 100,
	MB_T_CRYPTO_ENGINE,
	MB_T_DESC_CW_SETTING,
	MB_T_M2M_SETTING,
	MB_T_DESC_ADV_CW_SETTING,
	MB_T_RT_GET_RANDOM_DATA,
	MB_T_PVR_SETTING,
	MB_T_PVRKEY_SETTING,
	
/*JUST FOR TEST CMD*/
	MB_T_TEST_START = 150,
	MB_T_TEST_DMA,
	MB_T_MAX,
}en_mbox_type;

s32  symphony_engine_start(EN_ADV_KEYCONFIGINFO *pstKeyConifgInfo,
        EN_ADV_CIPHERENGINEPRIVATE *pstEngPrivate);

/**
  \brief    This function stops a cipher engine for the given application, 
  which started before.

  \param[in]  EN_ADV_CIPHERENGINEPRIVATE *pstEngPrivate

  \return     0       Success
  \return     Others      Exception
 ******************************************************************************/
s32    symphony_engine_stop(EN_ADV_CIPHERENGINEPRIVATE *pstEngPrivate);

/**
  \brief    This function stops a cipher engine for the given application for symphony
  which started before.

  \param[in]  EN_ADV_CIPHERENGINEPRIVATE *pstEngPrivate

  \return     0       Success
  \return     Others      Exception
 ******************************************************************************/
s32   symphony_engine_close(EN_ADV_CIPHERENGINEPRIVATE *pstEngPrivate);




#endif /* __HAL_SECURE_H__ END */


/**
 * @}
 */

