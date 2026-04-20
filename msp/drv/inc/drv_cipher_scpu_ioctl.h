/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_CIPHER_SCPU_IOCTL_H___
#define __DRV_CIPHER_SCPU_IOCTL_H___

#define VIR2PHY_SEC(Addr) ((u32)(Addr)&0x1fffffff)
#define PHY2VIR_SEC(Addr) ((u32)(Addr)|0xa0000000)

#define VIR2NOCHACHE_SEC(Addr) (Addr|0xa0000000)
#define NOCHACHE2VIR_SEC(Addr) (Addr&0x9fffffff)


#define CACHE_LINE_SIZE_SEC 32
#define DATA_ALIGNED_SEC(x)   (x&(~(CACHE_LINE_SIZE_SEC - 1)))    

/*
 * define '6' as the magic character of security
 */
#define HAL_CIPHER_IOC_MAGIC     'h'   
/*
 * enum for cmd character 
 */
typedef enum
{
    E_SEC_CIPHER_INIT			       =0 ,
    E_SEC_CIPHER_HANDLE_CREATE            ,
    E_SEC_CIPHER_HANDLE_DESTROY           ,
    //keyladder 
    E_SEC_CIPHER_KEYLADDER_CREATE         ,
    E_SEC_CIPHER_KEYLADDER_LINK	 	  ,
    E_SEC_CIPHER_KEYLADDER_DESTROY        ,
    //hash 
    E_SEC_CIPHER_HASH_CREATE		  ,
    E_SEC_CIPHER_HASH_UPDATE	          ,
    E_SEC_CIPHER_HASH_FINAL	     	  ,
    //config and process
    E_SEC_CIPHER_INFO_CONFIG              ,
    E_SEC_CIPHER_DATA_PROCESS             ,
    //cw
    E_SEC_CIPHER_SET_CW	                  ,
    //speciall command
    E_SEC_CIPHER_SPECIAL_CMD		  ,
    //get random number
    E_SEC_CIPHER_GET_RND_NUM		  ,
    //pvr key
    E_SEC_CIPHER_GET_PVR_KEY		  ,

    E_SEC_CIPHER_SEND_MSG		     ,
    E_SEC_CIPHER_RECV_MSG		     ,
    E_SEC_END                        	     ,
}security_ioctrl_cmd_t;


/*
 * creat crypto , if it is success . return crypto handle  
 */
#define CMD_SEC_CIPHER_INIT    _IO (HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_INIT)

#define CMD_SEC_CIPHER_HANDLE_CREATE            _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_HANDLE_CREATE,struct cipher_handle_priv)

#define CMD_SEC_CIPHER_HANDLE_DESTROY    _IO (HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_HANDLE_DESTROY)


#define CMD_SEC_CIPHER_KEYLADDER_CREATE         _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_KEYLADDER_CREATE, struct cipher_keyladder_priv)

#define CMD_SEC_CIPHER_KEYLADDER_LINK           _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_KEYLADDER_LINK, struct cipher_keyladder_priv)

#define CMD_SEC_CIPHER_KEYLADDER_DESTROY    _IO  (HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_KEYLADDER_DESTROY)

#define CMD_SEC_CIPHER_HASH_CREATE              _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_HASH_CREATE, struct cipher_hash_data_priv)

#define CMD_SEC_CIPHER_HASH_UPDATE              _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_HASH_UPDATE, struct cipher_hash_data_priv)

#define CMD_SEC_CIPHER_HASH_FINAL               _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_HASH_FINAL, struct cipher_hash_data_priv)

#define CMD_SEC_CIPHER_INFO_CONFIG              _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_INFO_CONFIG,struct cipher_config_priv)

#define CMD_SEC_CIPHER_DATA_PROCESS      _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_DATA_PROCESS, struct cipher_data_priv)

#define CMD_SEC_CIPHER_SET_CW                   _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_SET_CW, struct cipher_cw_priv)

#define CMD_SEC_CIPHER_GET_RND_NUM      _IO (HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_GET_RND_NUM)

#define CMD_SEC_CIPHER_GET_PVR_KEY              _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_GET_PVR_KEY, struct cipher_pvr_priv)

#define CMD_SEC_CIPHER_SPECIAL_CMD              _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_SPECIAL_CMD, struct cipher_cmd_priv)
#define CMD_SEC_CIPHER_SEND_MSG      _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_SEND_MSG, struct cipher_send_msg_priv)
#define CMD_SEC_CIPHER_RECV_MSG      _IOWR(HAL_CIPHER_IOC_MAGIC,E_SEC_CIPHER_RECV_MSG, struct cipher_recv_msg_priv)
typedef struct _MMZ_BUF_S 
{
    mt_u32 vir_addr;
    mt_u32 phy_addr;
    mt_u32 size;
} MMZ_BUF_S;

typedef struct _CIPHER_HASH_DATA_S
{
    mt_u32 input_data_len;
    MMZ_BUF_S mmz_buf;
    mt_u32 current_offset;
} CIPHER_HASH_DATA_S;

typedef struct _CIPHER_CIPHER_DATA_S 
{
    MMZ_BUF_S mmz_inbuf;
    MMZ_BUF_S mmz_outbuf;
    mt_u32 input_data_len;
    mt_u8 algo_op;  //0:enc,1:dec
    //work mode
    mt_u8 algo_work_mode;   //TODO:0:ECB;1:CBC, other modes are not support now!
    mt_u8 is_init;
} CIPHER_CIPHER_DATA_S;

#endif
