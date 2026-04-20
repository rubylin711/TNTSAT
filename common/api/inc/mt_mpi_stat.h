/** @defgroup USR_MODE In user mode
    @brief The following interfaces should be only called in user mode. */

/** @{ */

/** @defgroup H_MPI_MEM Memory operation
    @brief Memory operation interfaces for other CBB in user mode */

/** @defgroup H_MPI_STAT Stat opertation
    @brief Simple stat operation interfaces in user mode */

/** @} */



#ifndef __MT_MPI_STAT_H__
#define __MT_MPI_STAT_H__

#include "mt_drv_stat.h"

#ifdef __cplusplus
extern "C"{
#endif

/** @addtogroup H_MPI_STAT */
/** @{ */
typedef mt_s8 THREAD_NAME[64];

/* for userspace stat */
typedef struct
{
    MT_BOOL bUsed;
    mt_u32 min_time;    // us
    mt_u32 max_time;    // us
    mt_u32 avg_time;    // us

    THREAD_NAME name;
}stat_userspace_s;

typedef struct
{
    mt_u32 stat_thread_phyaddr;
    stat_userspace_s *stat_thread_uvirtaddr;

    mt_u32 counter;
#ifdef __KERNEL__
    struct __kernel_old_timeval tv_last;
#else
    struct timeval tv_last;
#endif
    mt_u64 time_total;    // us
}*mt_stat_handle, mt_stat_handle_s;

//typedef mt_u32 mt_stat_handle;
#define MT_STAT_INVALID_HANDLE (-1)

extern mt_s32 mt_mpi_stat_init(mt_void);
extern mt_s32 mt_mpi_stat_deinit(mt_void);

extern mt_s32 mt_mpi_stat_thread_reset(mt_stat_handle handle);
extern mt_s32 mt_mpi_stat_thread_reset_all(mt_void);

extern mt_s32 mt_mpi_stat_thread_register(mt_s8 *name, mt_stat_handle *pHandle);
extern mt_s32 mt_mpi_stat_thread_unregister(mt_stat_handle * pHandle);

extern mt_s32 mt_mpi_stat_thread_probe(mt_stat_handle handle);

extern mt_s32 mt_mpi_stat_event(STAT_EVENT_E enEvent, mt_u32 Value);
extern mt_u32 mt_mpi_stat_get_tick(mt_void);
extern mt_s32 mt_mpi_stat_notify_low_delay_event (mt_ld_event_s * evt);

/** @} */

#ifdef __cplusplus
}
#endif


#endif /* __MT_MPI_STAT_H__ */

