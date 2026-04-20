/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_MPI_IPCS_H__
#define __MT_MPI_IPCS_H__

#include "mt_type.h"
#include "drv_ipcs_ioctl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum
{
    MT_IPCS_CHAN0 = 0,
    MT_IPCS_CHAN1,
    MT_IPCS_CHAN2,
    MT_IPCS_CHAN3,
    MT_IPCS_CHAN4,
    MT_IPCS_CHAN5,
    MT_IPCS_CHAN6,
    MT_IPCS_CHAN7
}MT_IPCS_CHAN_E;

typedef struct
{
    MT_IPCS_CHAN_E channel_id;
    unsigned int ack;    
    unsigned int msg_id;
    unsigned int param1;
    unsigned int param2;
}mpi_ipcs_msg_t;

typedef struct
{
    phys_addr_t  phyaddr;                /**<Physical address of an MMZ buffer*/
    mt_u8  *user_viraddr;           /**<User-state virtual address of an MMZ buffer*/
    mt_u32  bufsize;                /**<Size of an MMZ buffer*/
    mt_u32 cached;
}mt_ipcs_mmz_buf_s;

mt_s32 mt_mpi_ipcs_buf_malloc(mt_ipcs_mmz_buf_s *p_buf, mt_u32 bufsize);
mt_s32 mt_mpi_ipcs_buf_malloc_cached(mt_ipcs_mmz_buf_s *p_buf, mt_u32 bufsize);
mt_s32 mt_mpi_ipcs_buf_free(mt_ipcs_mmz_buf_s *p_buf);

int mt_mpi_ipcs_open(mt_handle *p_handle);
int mt_mpi_ipcs_close(mt_handle handle);

int mt_mpi_ipcs_get_scpu_response(mpi_ipcs_msg_t *p_msg);
int mt_mpi_ipcs_send_scpu_msg(mpi_ipcs_msg_t *p_msg);

unsigned int mt_mpi_ipcs_check_peer(void);
int mt_mpi_ipcs_set_local(unsigned int status);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_MPI_IPCS_H__ */

