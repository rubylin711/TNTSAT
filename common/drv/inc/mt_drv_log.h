#ifndef __MT_DRV_LOG_H__
#define __MT_DRV_LOG_H__

#include "mt_type.h"
#include "mt_module.h"


#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

extern char *store_path;

#define STORE_PATH store_path

/*Define Debug Level For LOG                 */
#define MT_FATAL_LOG(fmt...)	MT_FATAL_PRINT(MT_ID_LOG, fmt)
#define MT_ERR_LOG(fmt...) 	    MT_ERR_PRINT(MT_ID_LOG, fmt)
#define MT_WARN_LOG(fmt...) 	MT_WARN_PRINT(MT_ID_LOG, fmt)
#define MT_INFO_LOG(fmt...) 	MT_INFO_PRINT(MT_ID_LOG, fmt)



typedef struct mt_log_path_s
{
	const mt_char *path;
	mt_size_t pathlen;
}log_path_s;


typedef struct mt_store_path_s
{
	const mt_char *path;
	mt_size_t pathlen;
}store_path_s;

#ifdef __KERNEL__

#include <linux/seq_file.h>

mt_s32 mt_drv_log_init(mt_void);
mt_void mt_drv_log_exit(mt_void);

mt_s32 mt_drv_log_kinit(mt_void);
mt_void mt_drv_log_kexit(mt_void);


mt_void mt_drv_log_config_bufaddr(ulong *addr);

mt_s32 mt_drv_log_proc_read(struct seq_file *s, mt_void *arg);
mt_s32 mt_drv_log_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);

mt_s32 mt_drv_log_buffer_read(mt_u8 *buf, mt_u32 buflen, mt_u32 *copylen, MT_BOOL is_kernel);
mt_s32 mt_drv_log_buffer_write(mt_u8 *buf, mt_u32 msglen, mt_u32 is_kernel);

mt_s32 mt_drv_log_set_path(log_path_s *path);
mt_s32 mt_drv_log_get_path(mt_s8 *buf, mt_u32 len);

mt_s32 mt_drv_log_set_storepath(store_path_s *path);
mt_s32 mt_drv_log_get_storepath(mt_s8 *buf, mt_u32 len);
#endif

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __MT_DRV_LOG_H__ */

