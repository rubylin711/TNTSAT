#ifndef __DRV_LOG_IOCTL_H__
#define __DRV_LOG_IOCTL_H__

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_drv_log.h"

/*#define LOG_UDISK_SUPPORT*/
#define LOG_MAX_TRACE_LEN 256

/*define the buffer size for log*/
#define DEBUG_MSG_BUF_RESERVE (1024*4)
#define LOG_CONFIG_BUF_SIZE (1024*16)

#define PRINT_FMT "[%d %s-%s]:%s[%d]:%s", time_ms, debuglevel_name[level],\
					g_log_configinfo[mod_id].mod_name,\
					func_name, line_num, log_str

typedef enum mt_log_output_pos_e
{
	LOG_OUTPUT_SERIAL,
	LOG_OUTPUT_NETWORK,
	LOG_OUTPUT_UDISK,
	LOG_OUTPUT_BUTT
}log_output_pos_e;


#define LOG_OUTPUT_POS_DEFAULT (LOG_OUTPUT_SERIAL)

/*structure of mode log level */
typedef struct mt_log_config_info_s
{
    mt_u8 mod_name[16+12];     /*mode name 16 + '_' 1 + pid 10 */
    mt_u8 level;         /*log print level control*/
    mt_u8 outpos;      /*log output location, 0:serial port; 1:network;2:u-disk*/
    mt_u8 udisk_flag;        /* u-disk log flag */
}log_config_info_s;


typedef struct mt_log_buf_read_s
{
	mt_u8 *buf;
	mt_u32 size; /*buf size*/
	mt_u32 len;	 /*copyed data length*/
}log_buf_read_s;


typedef struct mt_log_buf_write_s
{
	mt_u8 *buf;
	mt_u32 len;	/*message length*/
}log_buf_write_s;


#define UMAP_CMPI_LOG_INIT 				_IOR  (MT_ID_LOG, 0, ulong)
#define UMAP_CMPI_LOG_EXIT 				_IO   (MT_ID_LOG, 1)
#define UMAP_CMPI_LOG_READ_LOG 			_IOWR (MT_ID_LOG, 2, log_buf_read_s)
#define UMAP_CMPI_LOG_WRITE_LOG 		_IOW  (MT_ID_LOG, 3, log_buf_write_s)
#define UMAP_CMPI_LOG_SET_PATH 			_IOW  (MT_ID_LOG, 4, log_path_s)
#define UMAP_CMPI_LOG_SET_STORE_PATH 	_IOW  (MT_ID_LOG, 5, store_path_s)


#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __DRV_LOG_IOCTL_H__ */

