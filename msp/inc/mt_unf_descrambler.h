/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */

#ifndef __MT_UNF_DESCRAMBLER_H__
#define __MT_UNF_DESCRAMBLER_H__

#include "mt_error_mpi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*************************** Structure Definition ****************************/
/** \addtogroup      Descrambler */
/** @{ */  /** <!-- [Descrambler] */

/**Conditional access (CA) type, indicating whether advanced CA is used.*/
/**CNcomment:CA类型，是否使用高安全CA*/
typedef enum mtUNF_DMX_CA_TYPE_E
{
    MT_UNF_DMX_CA_NORMAL = 0,    /**<Common CA*/ /**< CNcomment:普通CA*/
    MT_UNF_DMX_CA_ADVANCE,       /**<Advanced CA*/ /**< CNcomment:高安全CA*/

    MT_UNF_DMX_CA_BUTT
} MT_UNF_DMX_CA_TYPE_E;

/**CA Entropy reduction mode*/
/**CNcomment:熵减少模式*/
typedef enum mtUNF_DMX_CA_ENTROPY_REDUCTION_E
{
    MT_UNF_DMX_CA_ENTROPY_REDUCTION_CLOSE = 0,  /**<64bit*/
    MT_UNF_DMX_CA_ENTROPY_REDUCTION_OPEN,       /**<48bit*/

    MT_UNF_DMX_CA_ENTROPY_REDUCTION_BUTT
} MT_UNF_DMX_CA_ENTROPY_E;

/**CA dec or enc mode*/
/**CNcomment:  配置加解密模式*/
typedef enum mtUNF_DMX_CA_DEC_ENC_MODE_E
{
    MT_UNF_DMX_CA_KEY_ATTR_DEC_OPEN = 0,  /*decrypt cas2 cas3 aes des tdes asa*/
    MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN,        /*encrypt aes dest*/
    MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN_ODD,
    MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN_EVEN,    
    MT_UNF_DMX_CA_KEY_ATTR_BUTT
} MT_UNF_DMX_CA_DEC_ENC_MODE_E;

/**Type of the descrambler protocol.*/
/**CNcomment:解扰器协议类型*/
typedef enum mtUNF_DMX_DESCRAMBLER_TYPE_E
{
    MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2      = 0,   /**<CSA2.0  AUTO MODE */
    MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_TS,
    MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_PES,
    MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_HIGH8BYTE,   /*for Irdeto*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_CONFORMANCE,   /*for Irdeto*/
    
    MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3,      /**<CSA3.0  AUTO MODE */
    MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3_TS,
    MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3_PES,

    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_CTS,
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_CTS_XOR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_DVS042,
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_DVS042_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_RCBC_CTS,
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_CTS1,
    
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_TAIL_CLEAR,  
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_CTS_CLR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_CTS_XOR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_SCTE52,
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_CTS1,
    
    MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_CTS,
    MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_DVS042,
    MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_CTS1,
    
    MT_UNF_DMX_DESCRAMBLER_TYPE_ASA,
    
    MT_UNF_DMX_DESCRAMBLE_DESC_AES_CTR,                     /*add disc_mod_ctr*/
    MT_UNF_DMX_DESCRAMBLE_DESC_AES_CTR_H,
    MT_UNF_DMX_DESCRAMBLE_DESC_AES_CTR_L,
    MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR,
    MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR_H,
    MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR_L,
    MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR,
    MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR_H,
    MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR_L,

    MT_UNF_DMX_DESCRAMBLE_SM4_ECB_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_SM4_ECB_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_SM4_CBC_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_SM4_CBC_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_SM4_CBC_OFB,
    MT_UNF_DMX_DESCRAMBLE_SM4_CBC_CTS,
    MT_UNF_DMX_DESCRAMBLE_SM4_CBC_CTS_XOR,
    MT_UNF_DMX_DESCRAMBLE_SM4_CBC_CTS1,
    
    MT_UNF_DMX_DESCRAMBLE_GOST3412K_ECB_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST3412K_ECB_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_OFB,
    MT_UNF_DMX_DESCRAMBLE_GOST3412K_CTR,
    MT_UNF_DMX_DESCRAMBLE_GOST3412K_CFB,
    MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_CTS1,
    
    MT_UNF_DMX_DESCRAMBLE_GOST28147_ECB_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST28147_ECB_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_OFB,
    MT_UNF_DMX_DESCRAMBLE_GOST28147_CTR,
    MT_UNF_DMX_DESCRAMBLE_GOST28147_CFB,	
    MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_CTS1,
    
    MT_UNF_DMX_DESCRAMBLE_GOST3412M_ECB_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST3412M_ECB_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_HEAD_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_TAIL_CLEAR,
    MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_OFB,
    MT_UNF_DMX_DESCRAMBLE_GOST3412M_CTR,
    MT_UNF_DMX_DESCRAMBLE_GOST3412M_CFB,	
    MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_CTS1,
    
    MT_UNF_DMX_DESCRAMBLE_MULTI2_CBC_OFB,
	
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_IPTV  ,      /**<AES IPTV of SPE*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB   ,      /**<SPE AES ECB*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CI    ,      /**<SPE AES CIPLUS*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CI    ,      /**<DES CIPLUS*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC   ,      /**<DES CBC*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_NS    ,      /**<AES NS-Mode*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_NS   ,      /**<SMS4 NS-Mode*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_IPTV ,      /**<SMS4 IPTV*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_ECB  ,      /**<SMS4 ECB*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_CBC  ,      /**<SMS4 CBC*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC   ,      /**<AES CBC*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_DES_IPTV,
    MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_IPTV,
    MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB,
    MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC,
    MT_UNF_DMX_DESCRAMBLER_TYPE_BUTT
} MT_UNF_DMX_DESCRAMBLER_TYPE_E;


/*!
  Type of ive calc mode
  CNcomment:  IVE的格式；
  */
typedef enum
{
     /*!
      mode: cbc mdi des
      */
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MDI_DES = 1,
     /*!
      mode: cbc mdi tdes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MDI_TDES,
     /*!
      mode: cbc mdi aes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MDI_AES,
     /*!
      mode: cbc mdi dis
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MDI_DISABLE,    
     /*!
      mode: cbc mdd des
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MDD_DES,
     /*!
      mode: cbc mdd tdes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MDD_TDES,
     /*!
      mode: cbc mdd aes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MDD_AES,
     /*!
      mode: cbc mdd dis
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MDD_DISABLE,    
     /*!
      mode: cbc msc des
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MSC_DES,
     /*!
      mode: cbc msc tdes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MSC_TDES,
     /*!
      mode: cbc msc aes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MSC_AES,
     /*!
      mode: cbc msc dis
      */    
    MT_UNF_DMX_IVE_CALC_MODE_CBC_MSC_DISABLE,
     /*!
      mode: rcbc mdi des
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDI_DES,
     /*!
      mode: rcbc mdi tdes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDI_TDES,
     /*!
      mode: rcbc mdi aes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDI_AES,
     /*!
      mode: rcbc mdi dis
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDI_DISABLE,    
     /*!
      mode: rcbc mdd des
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDD_DES,
     /*!
      mode: rcbc mdd tdes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDD_TDES,
     /*!
      mode: rcbc mdd aes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDD_AES,
     /*!
      mode: rcbc mdd dis
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDD_DISABLE,    
     /*!
      mode: rcbc msc des
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MSC_DES,
     /*!
      mode: rcbc msc tdes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MSC_TDES,
     /*!
      mode: rcbc msc aes
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MSC_AES,
     /*!
      mode: rcbc msc dis
      */    
    MT_UNF_DMX_IVE_CALC_MODE_RCBC_MSC_DISABLE,            
     /*!
      mode: disable
      */    
    MT_UNF_DMX_IVE_CALC_MODE_DISABLE
}MT_UNF_DMX_IVE_calc_mode_t;

typedef struct mtUNF_DMX_DESCRAMBLER_ENC_ATTR_S
{
    MT_BOOL encOddOrEven;
    MT_BOOL encForce;            /*0:  enc playload_tsptk, 1: force enc all ts,*/	
    MT_BOOL encScrTagClr;		/*0: don't modify scr_tag ,  1: modify scr_tag*/
    MT_BOOL encTsScrClrRange;      /**0: modify scr_tage for all tsptk, 1: modify src_tag for only playload_tsptk**/
}MT_UNF_DMX_DESCRAMBLER_ENC_ATTR_S;

typedef enum mtUNF_DMX_DESCRAMBLER_TSCFG_DSMODE_E
{
	MT_UNF_DMX_CA_AUTO_DS_MODE,
	MT_UNF_DMX_CA_FORCE_TS_LAYER,
	MT_UNF_DMX_CA_FORCE_PES_LAYER,
	MT_UNF_DMX_CA_BUTT_MODE
}MT_UNF_DMX_DESCRAMBLER_TSCFG_DSMODE_E;

typedef enum mtUNF_DMX_DESCRAMBLER_TSCFG_PES_TS_CWOPT1_E
{
       MT_UNF_DMX_ACTIVE_PES_LOWBIT_DSC=1,         /*pes_cwopt1: 0, use pes scrtag;  ts_cwopt1: 1, when scb invaild, use low bit to descrambling*/
	MT_UNF_DMX_ACTIVE_PES_NO_DSC,                 /*pes_cwopt1: 0, use pes scrtag;  ts_cwopt1: 0, when scb invaild, no descrambling*/
	MT_UNF_DMX_INACTIVE_PES_SCB_NO_DSC,     /*pes_cwopt1: 1, no scramble and desceamble ; lts_cwopt1: 0, 0, when scb invaild, no descrambling*/
	MT_UNF_DMX_INACTIVE_PES_LOWBIT_DSC      /*pes_cwopt1: 1,  no scramble and desceamble ;ts_cwopt1: 1, when scb invaild, use low bit to descrambling*/
}MT_UNF_DMX_DESCRAMBLER_TSCFG_PES_TS_CWOPT1_E;

typedef enum mt_UNF_DMX_DESCRAMBLER_TSCFG_GOST_SBOX_E
{
	MT_UNF_DMX_DESCRAMBLER_TSCFG_GOST_SBOX_TEMP_PARAM,
	MT_UNF_DMX_DESCRAMBLER_TSCFG_GOST_SBOX_CRYPTO_A_PARAM,
	MT_UNF_DMX_DESCRAMBLER_TSCFG_GOST_SBOX_CRYPTO_B_PARAM,
	MT_UNF_DMX_DESCRAMBLER_TSCFG_GOST_SBOX_CRYPTO_C_PARAM,
	MT_UNF_DMX_DESCRAMBLER_TSCFG_GOST_SBOX_CRYPTO_D_PARAM,
	MT_UNF_DMX_DESCRAMBLER_TSCFG_GOST_SBOX_Z_PARAM,
}MT_UNF_DMX_DESCRAMBLER_TSCFG_GOST_SBOX_E;

/**<Descrambler ts config, for select MSB64, GOST box , GOST28147 bit inv*/ 
/**< CNcomment:选择msb64, gost 算法下的box选 择,GOST28147 存储序列选择*/
typedef struct mt_UNF_DMX_DESCRAMBLER_TSCFG_S
{
    MT_BOOL  validate_tag;
    MT_UNF_DMX_DESCRAMBLER_TSCFG_DSMODE_E tscfgDsMode; 
    MT_UNF_DMX_DESCRAMBLER_TSCFG_PES_TS_CWOPT1_E tscfgCwopt1Mode;
    MT_BOOL  des_key_msb64;           //:1;   //bit18  des key use high_64bit key
    MT_BOOL csa2_key_msb64;             //:1;   //bit19 csa2/csa2conformance caiy
    MT_BOOL multi2_key_msb64 ;           // :1;   //bit20
    MT_UNF_DMX_DESCRAMBLER_TSCFG_GOST_SBOX_E gost_sbox_sel;             //:3;   bit23:21 0-5  
    MT_BOOL gost_bit_inv;             //:1;  //bit24  ordering type for storage access in gost28147 , 1:byte ordering  0: bit ordering
}MT_UNF_DMX_DESCRAMBLER_TSCFG_S;

typedef struct mt_UNF_DMX_DESCRAMBLER_CORE_S
{
	 MT_BOOL  validate_tag;
        mt_u32 ds_core_sel;                // : 3;
        mt_u32 csa3_opti;                  // : 2;
}MT_UNF_DMX_DESCRAMBLER_CORE_S;

typedef struct mt_UNF_DMX_DESCRAMBLER_ADES_DISC_MODE_S
{
	 MT_BOOL  validate_tag;
        mt_u32 disc_mode;                   //: 8;
        mt_u32 disc_mode_ctr;            // : 2;
	 mt_u32  dis_mode_b8;             // :1;
}MT_UNF_DMX_DESCRAMBLER_ADES_DISC_MODE_S;

typedef struct mt_UNF_DMX_DESCRAMBLER_ADES_PDK_MODE_S
{
	 MT_BOOL  validate_tag;
        mt_u32 short_pkt_mode;              //: 2;
        mt_u32 small_pkt_mode;              //: 2;
}MT_UNF_DMX_DESCRAMBLER_ADES_PDK_MODE_S;

/**config clear ts desc tag*/
/**CNcomment:  配置是否修改加扰标识*/
typedef enum mT_UNF_DMX_CA_DESCRAMBLER_FLAG_CFG_E
{
    MT_UNF_DMX_DESCRAMBLER_DELETE_DESC_TAG,
    MT_UNF_DMX_DESCRAMBLER_DONOT_DELETE_DESC_TAG,		
    MT_UNF_DMX_DESCRAMBLER_DELETE_DESC_TAG_BUTT
} MT_UNF_DMX_CA_DESCRAMBLER_FLAG_CFG_E;



/**Attribute of the key area.*/
/**CNcomment:密钥区属性*/
typedef struct mtUNF_DMX_DESCRAMBLER_ATTR_S
{
    MT_UNF_DMX_CA_TYPE_E enCaType;                    /**<Whether the descrambler adopts advanced CA.*/ /**< CNcomment:解扰器是否使用高安全CA*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_E enDescramblerType;  /**<Descrambling protocol type of the descrambler*/ /**< CNcomment:解扰器解扰协议类型*/
    MT_UNF_DMX_CA_ENTROPY_E enEntropyReduction;       /**<CA Entropy reduction mode,for CSA2.0*/ /**< CNcomment:熵减少模式，CSA2.0有效*/
    MT_UNF_DMX_IVE_calc_mode_t  ivMode;
    MT_UNF_DMX_CA_DEC_ENC_MODE_E  enDecOrEncMode;
    MT_UNF_DMX_DESCRAMBLER_TSCFG_S  tsCfg;        /**<Descrambler ts config, for select MSB64, GOST box , GOST28147 bit inv*/ /**< CNcomment:是否选择msb64, gost 算法下的box选 择,GOST28147 存储序列选择*/
    MT_UNF_DMX_CA_DESCRAMBLER_FLAG_CFG_E   enTsDescFlag;
} MT_UNF_DMX_DESCRAMBLER_ATTR_S;


/*
**  for MT_UNF_DMX_CreateDescrambler Professional
*/
typedef struct mtUNF_DMX_DESCRAMBLER_PRO_ATTR_S
{
    MT_UNF_DMX_CA_TYPE_E enCaType;                    /**<Whether the descrambler adopts advanced CA.*/ /**< CNcomment:解扰器是否使用高安全CA*/
    MT_UNF_DMX_DESCRAMBLER_TYPE_E enDescramblerType;  /**<Descrambling protocol type of the descrambler*/ /**< CNcomment:解扰器解扰协议类型*/
    MT_UNF_DMX_CA_ENTROPY_E enEntropyReduction;       /**<CA Entropy reduction mode,for CSA2.0*/ /**< CNcomment:熵减少模式，CSA2.0有效*/
    MT_UNF_DMX_IVE_calc_mode_t  ivMode;
    MT_UNF_DMX_CA_DEC_ENC_MODE_E  enDecOrEncMode;
    MT_UNF_DMX_DESCRAMBLER_ENC_ATTR_S  encMode;
    MT_UNF_DMX_DESCRAMBLER_TSCFG_S  tsCfg;
    MT_UNF_DMX_DESCRAMBLER_CORE_S  dsCore;
    MT_UNF_DMX_DESCRAMBLER_ADES_DISC_MODE_S descMode;
    MT_UNF_DMX_DESCRAMBLER_ADES_PDK_MODE_S pdkMode;
    MT_UNF_DMX_CA_DESCRAMBLER_FLAG_CFG_E   enTsDescFlag;
} MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S;


/** @} */  /** <!-- ==== Structure Definition end ==== */


/******************************* API Declaration *****************************/
/** \addtogroup      Descrambler */
/** @{ */  /** <!--[Descrambler]*/

/**
\brief Creates a key area.CNcomment:创建一个密钥区。CNend
\attention \n
When creating a key area, you can ignore the DUMUX to which the key area belongs, because all DEMUXs share all key areas.
CNcomment:申请密钥区，不用关心属于哪路DEMUX，所有DEMUX共用所有密钥区。
如果是symphony2芯片，需要配置otp， descrambler才能正常工作。
otp的配置如下:
      1、nm.l 0xbf313e50 40000000   
             reset
      2、nm.l 0xbf313e50 8000000  
              nm.l 0xbf313e88 100  
              nm.l 0xbf313e5c 4000
              nm.l 0xbf313e8c 20100000
              reset    
      3、        
              nm.l 0xbf313e3c 400000
              nm.l 0xbf313be8 400000
              nm.l 0xbf313bfc 00000c00
              nm.l 0xbf313c00 00000001     
              reset                                                                       
注意:  symphony2-A0 需要配1,2,3 步，symphony2-A1  只需要配置 2, 3 步               
              symphony1、symphony3 不需要配置           CNend
\param[in] u32DmxId   DEMUX ID. CNcomment: DEMUX号。CNend
\param[out] phKey     Pointer to the handle of a created key area.CNcomment:指针类型，输出申请到的密钥区Handle。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOFREE_KEY There is no available key area. CNcomment:没有空闲的密钥区。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_CreateDescrambler(mt_u32 u32DmxId, mt_handle *phKey);


/**
\brief Creates a key area. The key area type and descrambling protocol type can be selected.CNcomment:创建一个密钥区,支持选择高安全CA和解扰协议类型。CNend
\attention \n
When an advanced CA key area is created, the descrambling protocol depends on the hardware and interface settings are ignored.\n
CNcomment:如果是高安全CA，解扰协议已经由硬件决定，接口的设置被忽略。
如果是symphony2芯片，需要配置otp， descrambler才能正常工作。
otp的配置如下:
      1、nm.l 0xbf313e50 40000000   
             reset
      2、nm.l 0xbf313e50 8000000  
              nm.l 0xbf313e88 100  
              nm.l 0xbf313e5c 4000
              nm.l 0xbf313e8c 20100000
              reset    
      3、        
              nm.l 0xbf313e3c 400000
              nm.l 0xbf313be8 400000
              nm.l 0xbf313bfc 00000c00
              nm.l 0xbf313c00 00000001     
              reset                                                                       
注意:  symphony2-A0 需要配1,2,3 步，symphony2-A1  只需要配置 2, 3 步               
              symphony1、symphony3 不需要配置           CNend
\param[in] u32DmxId   DEMUX ID. CNcomment: DEMUX号。CNend
\param[in] pstDesramblerAttr  Pointer to the attributes of a key area.CNcomment:密钥区属性指针。CNend
\param[out] phKey      Pointer to the handle of a created key area.CNcomment:指针类型，输出申请到的密钥区Handle。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOFREE_KEY  There is no available key area.CNcomment:没有空闲的密钥区。CNend
\retval ::MT_ERR_DMX_NOT_SUPPORT  Not support MT_UNF_DMX_DESCRAMBLER_ATTR_S type.CNcomment:不支持的MT_UNF_DMX_DESCRAMBLER_ATTR_S类型。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_CreateDescramblerExt(mt_u32 u32DmxId, const MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDesramblerAttr, mt_handle *phKey);

/**
\  create descrambler Professionally only for chip verf
\
\param[in] u32DmxId   DEMUX ID. CNcomment: DEMUX号。CNend
\param[in] pstDesramblerAttr  Pointer to the attributes of a key area.CNcomment:密钥区属性指针。CNend
\param[out] phKey      Pointer to the handle of a created key area.CNcomment:指针类型，输出申请到的密钥区Handle。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOFREE_KEY  There is no available key area.CNcomment:没有空闲的密钥区。CNend
\retval ::MT_ERR_DMX_NOT_SUPPORT  Not support MT_UNF_DMX_DESCRAMBLER_ATTR_S type.CNcomment:不支持的MT_UNF_DMX_DESCRAMBLER_ATTR_S类型。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_CreateDescramblerPro(mt_u32 u32DmxId, const MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S *pstDesramblerAttr, mt_handle *phKey);


/**
\brief Destroys an existing key area.CNcomment:销毁创建的密钥区。CNend
\attention \n
If a key area is attached to a channel, the key area needs to be detached from the channel first, but the channel is not disabled.\n
If a key area is detached or destroyed before the attached channel is disabled, an error may occur during data receiving.
CNcomment:如果密钥区绑定在通道上，会先从通道上解绑定密钥区，但是注意不会关闭通道\n
如果没有关闭通道则进行密钥区的解绑定或销毁操作，可能导致数据接收的错误。CNend
\param[in] hKey  Handle of the key area to be destroyed.CNcomment:待删除的密钥区Handle。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_DestroyDescrambler(mt_handle hKey);


/**
\brief Set Descrambler Attribute. CNcomment: 配置解扰的属性参数。CNend
\attention \n

\param[in] hKey  Handle of the key area to be destroyed.CNcomment:待删除的密钥区Handle。CNend
\param[in] pstAttr Pointer to the attributes of descrambler
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetDescramblerAttr(mt_handle hKey, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstAttr);

/**
\brief Get Descrambler Attribute. CNcomment: 获取解扰的属性参数。CNend
\attention \n

\param[in]   hKey  Handle of the key area to be destroyed.CNcomment:待删除的密钥区Handle。CNend
\param[out] pstAttr Pointer to the attributes of descrambler
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetDescramblerAttr(mt_handle hKey, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstAttr);

/**
\brief Sets the even keys of a key area. This API is used to configure the DEMUX descrambler based on even keys after the CA system obtains control words.CNcomment:设置keySlotId
\attention \n
CNcomment:keySlotId为高安分配的even slot Id，该接口会将MT_UNF_DMX_CreateDescrambler 时系统默认分配的keySlotId 替换，并实现重新和
descrambler 进行绑定。CNend
\param[in] hKey  Handle of the key area to be set.CNcomment:待设置的密钥区句柄。CNend
\param[in] KeySlotId  高安模块分配的key slot id 。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetDescramblerEvenKeySlot(mt_handle hKey, mt_u8 keySlotId);


/**
\brief Sets the even keys of a key area. This API is used to configure the DEMUX descrambler based on even keys after the CA system obtains control words.CNcomment:设置keySlotId
\attention \n
CNcomment:keySlotId为高安分配的odd slot Id，该接口会将MT_UNF_DMX_CreateDescrambler 时系统默认分配的keySlotId 替换，并实现重新和
descrambler 进行绑定。CNend
\param[in] hKey  Handle of the key area to be set.CNcomment:待设置的密钥区句柄。CNend
\param[in] KeySlotId  高安模块分配的key slot id 。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetDescramblerOddKeySlot(mt_handle hKey,  mt_u8 keySlotId);


/**
\brief Sets the even keys of a key area. This API is used to configure the DEMUX descrambler based on even keys after the CA system obtains control words.CNcomment:设置密钥区的偶密钥。CA系统得到控制字后，可调用本接口将偶密钥配置到DEMUX解扰模块。CNend
\attention \n
pEvenKey points to the even key data to be set. The data consists of 16 bytes: CW1, CW2, ..., and CW16.\n
The key value can be set dynamically, that is, the key value can be set at any time after a key area is created.\n
The initial value of each key is 0, which indicates that data is not descrambled.
CNcomment:pEvenKey指向要设置的偶密钥数据。数据共16byte，byte依次是CW1、CW2、……、CW16\n
支持密钥区的动态设置，可以在密钥区申请后的任意时刻设置密钥值\n
当设置密钥之前，密钥的初时值都是0，表示不做解扰。CNend
\param[in] hKey  Handle of the key area to be set.CNcomment:待设置的密钥区句柄。CNend
\param[in] pu8EvenKey  Pointer to the 16-byte even key data to be set.CNcomment:指针类型，指向要设置的偶密钥数据，必须是16个字节的数组。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetDescramblerEvenKey(mt_handle hKey, const mt_u8 *pu8EvenKey);


/**
\brief Sets the odd keys of a key area. This API is used to configure the DEMUX descrambler based on odd keys after the CA system obtains control words.CNcomment:设置密钥区的奇密钥。CA系统得到控制字后，可调用本接口将奇密钥配置到DEMUX解扰模块。CNend
\attention \n
pOddKey points to the odd key data to be set. The data consists of 16 bytes: CW1, CW2, ..., and CW16.\n
The key value can be set dynamically, that is, the key value can be set at any time after a key area is created.\n
The initial value of each key is 0, which indicates that data is not descrambled.
CNcomment:pOddKey指向要设置的奇密钥数据。奇密钥数据共16byte，byte依次是CW1、CW2、……、CW16\n
支持密钥区的动态设置，可以在密钥区申请后的任意时刻设置密钥值\n
当设置密钥之前，密钥的初时值都是0，表示不做解扰。CNend
\param[in] hKey  Handle of the key area to be set.CNcomment:待设置的密钥区句柄。CNend
\param[in] pu8OddKey   Pointer to the 16-byte odd key data to be set.CNcomment:指针类型，指向要设置的奇密钥数据，必须是16个字节的数组。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetDescramblerOddKey(mt_handle hKey, const mt_u8 *pu8OddKey);


/**
\brief Set Even IV.For algs do not use IV,do not care.CNcomment:设置偶密钥区的初始化向量。对于不涉及初始化向量的解扰算法可以不关注。CNend
\attention \n
pu8IVKey points to the iv key data to be set.The data consists of 16 bytes: CW1, CW2, ..., and CW16.\n
The key value can be set dynamically, that is, the key value can be set at any time after a key area is created.
CNcomment:pu8IVKey指向要设置的初始化向量数据。奇密钥数据共16byte，byte依次是CW1、CW2、……、CW16\n
支持密钥区的动态设置，可以在密钥区申请后的任意时刻设置。CNend
\param[in] hKey  Handle of the key area to be set.CNcomment:待设置的密钥区句柄。CNend
\param[in] pu8IVKey   Pointer to the 16-byte IV key data to be set.CNcomment:指针类型，指向要设置的奇密钥数据，必须是16个字节的数组。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetDescramblerEvenIVKey(mt_handle hKey, const mt_u8 *pu8IVKey);

/**
\brief Set Odd IV.For algs do not use IV,do not care.CNcomment:设置奇密钥区的初始化向量。对于不涉及初始化向量的解扰算法可以不关注。CNend
\attention \n
pu8IVKey points to the iv key data to be set.The data consists of 16 bytes: CW1, CW2, ..., and CW16.\n
The key value can be set dynamically, that is, the key value can be set at any time after a key area is created.
CNcomment:pu8IVKey指向要设置的初始化向量数据。奇密钥数据共16byte，byte依次是CW1、CW2、……、CW16\n
支持密钥区的动态设置，可以在密钥区申请后的任意时刻设置。CNend
\param[in] hKey  Handle of the key area to be set.CNcomment:待设置的密钥区句柄。CNend
\param[in] pu8IVKey    Pointer to the 16-byte IV key data to be set.CNcomment:指针类型，指向要设置的奇密钥数据，必须是16个字节的数组。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_SetDescramblerOddIVKey(mt_handle hKey, const mt_u8 *pu8IVKey);

/**
\brief Attaches a key area to a specific channel.CNcomment:绑定密钥区到指定通道。CNend
\attention \n
A key area can be attached to multiple channels that belong to different DEMUXs.\n
The static loading data in the key areas that are attached to all types of channels can be descrambled.\n
The same key area or different key areas cannot be attached to the same channel.
CNcomment:一个密钥区可以绑定到多个通道上，通道可以属于不同的DEMUX\n
可以对所有类型的通道绑定密钥区进行数据的解扰\n
不允许重复绑定相同或不同的密钥区到同一个通道上。CNend
\param[in] hKey    Handle of the key area to be attached.CNcomment:待绑定的密钥区句柄。CNend
\param[in] hChannel   Channel handle.CNcomment:通道句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_ATTACHED_KEY  A key area is attached to the channel.CNcomment:通道上已经有一个密钥区绑定在上面。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_AttachDescrambler(mt_handle hKey, mt_handle hChannel);


/**
\brief Detaches a key area from a channel.CNcomment:将密钥区从通道上解绑定。CNend
\attention \n
The key area used by a channel can be detached dynamically. That is, you can call this API to detach a key area at any time after it is attached.\n
The scrambled data, however, may not be descrambled after the key area is detached, which causes data error.\n
The value of a key area retains even after it is detached. If the key area is attached again, its value is still the previously configured value.\n
If you do not want to descramble data, you can detach the corresponding key area or set all key values to 0.
CNcomment:可以动态的解绑定通道使用的密钥区，可以在绑定后的任意时刻使用此接口解绑定密钥区\n
但是解绑定后可能导致加扰数据没有被解扰，导致数据错误\n
解绑定密钥区并不能改变密钥区的值，如果重新绑定密钥区，密钥值仍然是上次设置的值\n
如果不想进行解扰，除了解绑定密钥区之外，也可以直接将密钥值全部设置为0来实现。CNend
\param[in] hKey    Handle of the key area to be detached.CNcomment:待解绑定的密钥区句柄。CNend
\param[in] hChannel  Channel handle.CNcomment:通道句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NOATTACH_KEY  No key areas are attached to the channel.CNcomment:通道上没有绑定任何密钥区。CNend
\retval ::MT_ERR_DMX_UNMATCH_KEY  The specified key area is not attached to the specified channel.CNcomment:指定的密钥区没有绑定在指定的通道上。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_DetachDescrambler(mt_handle hKey, mt_handle hChannel);


/**
\brief Obtains the handle of the key area that is attached to a channel.CNcomment:获取通道绑定的密钥区句柄。CNend
\attention \n
If no key area is attached to the channel, the error code MT_ERR_DMX_NOATTACH_KEY is returned when you call this API.
CNcomment:当通道没有绑定密钥区时，调用本接口返回MT_ERR_DMX_NOATTACH_KEY错误码。CNend
\param[in] hChannel  Handle of the channel to be queried.CNcomment:要查询的通道句柄。CNend
\param[out] phKey     Pointer to the handle of the key area that is attached to a channel (output).CNcomment:指针类型，输出通道绑定的密钥区句柄。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\retval ::MT_ERR_DMX_NOATTACH_KEY  No key areas are attached to the channel.CNcomment:通道上没有绑定任何密钥区。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetDescramblerKeyHandle(mt_handle hChannel, mt_handle *phKey);


/**
\brief Obtains the number of available key areas.CNcomment:获取空闲密钥区数量。CNend
\attention \n
Because key areas are shared by multiple DEMUXs, the first parameter is meaningless. Therefore, you only need to enter a valid value.CNcomment:密钥区在几路DEMUX之间共用，所以第一个参数没有意义，只要输入一个合法值就可以了。CNend
\param[in] u32DmxId   DEMUX ID. CNcomment: DEMUX号。CNend
\param[out]  pu32FreeCount   Pointer to the number of available key areas (output).CNcomment:指针类型，输出空闲密钥区数目。CNend
\retval ::MT_SUCCESS Success.CNcomment:成功。CNend
\retval ::MT_FAILURE  Calling this API fails.CNcomment:API系统调用失败。CNend
\retval ::MT_ERR_DMX_NOT_INIT  The DEMUX module is not initialized.CNcomment:模块没有初始化。CNend
\retval ::MT_ERR_DMX_INVALID_PARA  The input parameter is invalid. CNcomment:输入参数非法。CNend
\retval ::MT_ERR_DMX_NULL_PTR  The pointer is null. CNcomment:指针参数为空。CNend
\see \n
 N/A.CNcomment:无。CNend
*/
mt_s32 MT_UNF_DMX_GetFreeDescramblerKeyCount(mt_u32 u32DmxId , mt_u32 * pu32FreeCount);

/** @} */  /** <!-- ==== API Declaration End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __MT_UNF_DESCRAMBLER_H__ */

