/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */


#ifndef __MPI_MEM_H__
#define __MPI_MEM_H__

#include "../../inc/mt_type.h"
#include "../../inc/mt_common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/******************************* MPI declaration *****************************/

/*malloc memory for mmz and map to user-state address, bufname and bufsize used to input,
	physic address and user-state used to output*/
/*cncomment: éêçëmmzäú´æ£¬²¢ó³éäóã»§ì¬µøö·,bufnameºíbufsize×÷îªêäèë, îïàíµøö·ºíóã»§ì¬ğéäâµøö·×÷îªêä³ö */
mt_s32 mt_mpi_mmz_malloc(mt_mmz_buf_s *pstbuf);

/*free user-state map, release mmz memory, make sure that physic address, user-state address and lengh is right*/
/*cncomment: ½â³ıóã»§ì¬µøö·µäó³éä£¬²¢êí·åmmzäú´æ,±£ö¤´«èëµäîïàíµøö·¡¢óã»§ì¬ğéäâµøö·ºí³¤¶èõıè·*/
mt_s32 mt_mpi_mmz_free(mt_mmz_buf_s *pstbuf);

/*malloc mmz memory for appointed mmz name, return physic address*/
/*cncomment: ö¸¶¨mmzµäãû×öéêçëmmzäú´æ£¬·µ»øîïàíµøö·*/
phys_addr_t mt_mpi_mmz_new(ulong size , mt_u32 u32Align, const mt_char *ps8mmzname, const mt_char *ps8mmbname,
						mt_mmz_security_attr_s *attr);

/*cncomment: êí·åmmzäú´æ */
mt_s32 mt_mpi_mmz_delete(phys_addr_t physaddr);

/*get physic address accordint to virtual address*/
/**cncomment:  ¸ù¾ığéäâµøö·£¬»ñè¡¶ôó¦µäîïàíµøö· */
mt_s32 mt_mpi_mmz_getphyaddr(void *pRefAddr, phys_addr_t *phyaddr, ulong *size);

/*map physic address of mmz memory to user-state virtual address, can appoint whether cached*/
/**cncomment:  ½«mmzéêçëµäîïàíµøö·ó³éä³éóã»§ì¬ğéäâµøö·£¬¿éòôö¸¶¨êç·ñcached*/
mt_void *mt_mpi_mmz_map(phys_addr_t physaddr, mt_u32 u32cached);

/*unmap user-state address of mmz memory*/
/**cncomment:  ½â³ımmzäú´æóã»§ì¬µøö·µäó³éä */
mt_s32 mt_mpi_mmz_unmap(void *vaddr);

/*for cached memory, flush cache to memory*/
/**cncomment:  ¶ôóúcachedäú´æ£¬ë¢dcacheµ½äú´æ */
mt_s32 mt_mpi_mmz_flush(void *vaddr, ulong offset, ulong size);

mt_s32 mt_mpi_mmz_invalidate(void *vaddr, ulong offset, ulong size);

mt_s32 mt_mpi_mmz_get_start_size(const char *mmz_name, phys_addr_t *phys_start, ulong *size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mpi_mem_h__ */
