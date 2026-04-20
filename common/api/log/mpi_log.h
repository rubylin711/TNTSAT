


#ifndef __MPI_LOG_H__
#define __MPI_LOG_H__

#include "mt_type.h"
#include "mt_debug.h"
#include "drv_log_ioctl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

mt_s32 mt_mpi_log_init(mt_void);
mt_void mt_mpi_log_deinit(mt_void);
mt_void mt_mpi_log_exit(mt_void);
mt_s32 mt_mpi_log_read(mt_u8 *buf, mt_u32 buflen, mt_u32 *copylen);
mt_s32 mt_mpi_log_level_set(mt_u32 mod_id, mt_log_level_e level);
mt_s32 mt_mpi_log_print_pos_set(mt_u32 mod_id, log_output_pos_e outpos);
mt_s32 mt_mpi_log_path_set(const mt_char* path);
mt_s32 mt_mpi_log_storepath_set(const mt_char* path);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __MPI_LOG_H__ */

