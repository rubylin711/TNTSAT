/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : mt_drv_ampshm.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/02
 * Description    : MT share memory functions for asymmetrical(AMP) AV/AP CPUs.
 * History        :
 * 1.Date         : 2017/12/02
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef _MT_AMP_SHM_H_
#define _MT_AMP_SHM_H_
#ifdef __KERNEL__
#include <linux/types.h>
#endif
//#include <sys/mman.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * sample
 * https://www.ibm.com/developerworks/aix/library/au-spunix_sharedmemory/index.html
 */
/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/wait.h>

void error_and_die(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int r;

  const char *memname = "sample";
  const size_t region_size = sysconf(_SC_PAGE_SIZE);

  int fd = shm_open(memname, O_CREAT | O_TRUNC | O_RDWR, 0666);
  if (fd == -1)
    error_and_die("shm_open");

  r = ftruncate(fd, region_size);
  if (r != 0)
    error_and_die("ftruncate");

  void *ptr = mmap(0, region_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED)
    error_and_die("mmap");
  close(fd);

  pid_t pid = fork();

  if (pid == 0) {
    u_long *d = (u_long *) ptr;
    *d = 0xdbeebee;
    exit(0);
  }
  else {
    int status;
    waitpid(pid, &status, 0);
    printf("child wrote %#lx\n", *(u_long *) ptr);
  }

  r = munmap(ptr, region_size);
  if (r != 0)
    error_and_die("munmap");

  r = shm_unlink(memname);
  if (r != 0)
    error_and_die("shm_unlink");

  return 0;
}
*/

/*

#ifndef __KERNEL__
#ifndef __mode_t_defined
typedef unsigned int mode_t;
# define __mode_t_defined
#endif

#ifndef __off_t_defined
typedef unsigned int off_t;
# define __off_t_defined
#endif

#ifndef _SIZE_T
#ifndef __size_t_defined
typedef unsigned long size_t;
# define __size_t_defined
#endif
#endif
#endif

*/

/* exclude '\0' */
#define MAX_SHM_NAME_LEN			11
#define MAX_SHM_BUFF_COUNT			32

#define MT_MAP_SECURE      			0x8000  /* AV/AP CPU Secure Share Memory */

#define AMPSHM_MAGIC_IPC_MSG_ID		0x7F01FF01

/* remote AMP SHM parameter index */
enum
{
	AMPSHM_PARA_INDEX_MANAGER_ADDR = 1,		/* parameter index: manager address */
};

int mt_ampshm_init(void);

void mt_ampshm_exit(void);

/*
 * @param[in] name length should <= MAX_SHM_NAME_LEN
 *
 * @retval fd AMP SHM file descriptor
 *         >=0: valid
 *          <0: error, invalid fd
 */
int mt_ampshm_open(const char *name, int oflag, mode_t mode);

int mt_ampshm_ftruncate(int fd, off_t length);

/*
 * @param[in] flags if MT_MAP_SECURE flag set, means allocate and map
 *                  AV/AP CPU Secure Share Memory.
 */
void *mt_ampshm_mmap(void *addr, size_t length, int prot, int flags,
		           int fd, off_t offset);

int mt_ampshm_munmap(void *addr, size_t length);

int mt_ampshm_unlink(const char *name);

ulong mt_ampshm_get_addr_by_name(const char *name);
//int mt_ampshm_close(int fd);

//after write, need flush cache, and the other side need invalid cache
//if addr is cacheable.
//int mt_ampshm_msync(void *addr, size_t length, int flags);

#ifdef __cplusplus
}
#endif

#endif	//_MT_AMP_SHM_H_

