#ifndef __MT_DRV_FILE_H__
#define __MT_DRV_FILE_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <linux/fs.h>
#include "mt_type.h"

/** @addtogroup H_FILE */
/** @{ */

struct file* mt_drv_file_open(const mt_s8* ps8FileName, mt_s32 s32Flags);
mt_void mt_drv_file_close(struct file * pFile);

mt_s32 mt_drv_file_read(struct file * pFile,  mt_u8* ps8Buf, mt_u32 u32Len);
mt_s32 mt_drv_file_write(struct file* pFile, mt_s8* ps8Buf, mt_u32 u32Len);

mt_s32 mt_drv_file_lseek(struct file *pFile, mt_s32 s32Offset, mt_s32 s32Whence);

mt_s32 mt_drv_file_get_storepath(mt_s8* ps8Buf, mt_u32 u32Len);
  

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __MT_DRV_FILE_H__ */
