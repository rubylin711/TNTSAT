#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mt_type.h"
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdarg.h>
#include <sys/time.h>
#include "mtos_mem.h"

void *mtos_malloc(u32 size)
{
  return malloc(size);
}

void *mtos_realloc(void *p_readdr, u32 size)
{
  return realloc(p_readdr,size);
}

void mtos_free(void *p_addr)
{
  free(p_addr);
}

void *mtos_calloc_alias(u32 n_elements, u32 elem_size)
{
  return calloc(n_elements,elem_size);
}

void *mtos_realloc_alias(void *p_readdr, u32 size)
{
  return realloc(p_readdr,size);
}

void * mtos_align_malloc(u32 size, u32 alignment)
{
    u32 *p_mem_ptr = NULL;
    u32 *p_tmp = NULL;
    u32 temp_len = 0;

    temp_len = size + alignment;

    if ((p_tmp = (u32 *) malloc(temp_len)) != NULL)
    {

      p_mem_ptr = (u32 *) (((u32)p_tmp + alignment - 1) & (~(u32) (alignment - 1)));


        if (p_mem_ptr == p_tmp)
          p_mem_ptr = (void *)((u32)p_mem_ptr + alignment);


      *((p_mem_ptr - 1)) = (u32) (p_mem_ptr - p_tmp);


      return ((void *)p_mem_ptr);
    }

    return(NULL);
}

void mtos_align_free(void *p_mem_ptr)
{
  u32 *p_ptr = NULL;

    if (p_mem_ptr == NULL)
        return;

    p_ptr = (u32 *)p_mem_ptr;
    p_ptr -= *(p_ptr - 1);
    free(p_ptr);

}

void *mtos_align_malloc_alias(u32 size, u32 alignment)
{
  u32 *p_mem_ptr = NULL;
  u32 *p_tmp = NULL;
  u32 temp_len = 0;

  temp_len = size + alignment;
  if ((p_tmp = (u32 *)malloc(temp_len)) != NULL)
  {
    p_mem_ptr = (u32 *) (((u32)p_tmp + alignment - 1) & (~(u32) (alignment - 1)));

    if (p_mem_ptr == p_tmp)
      p_mem_ptr = (void *)((u32)p_mem_ptr + alignment);

    *((p_mem_ptr - 1)) = (u32) (p_mem_ptr - p_tmp);

    return ((void *)p_mem_ptr);
  }
  return 0;
}

void mtos_align_free_alias(void *p_mem_ptr)
{
  u32 *p_ptr = NULL;

  if (p_mem_ptr == NULL)
      return;

  p_ptr = (u32 *)p_mem_ptr;
  p_ptr -= *(p_ptr - 1);
  free(p_ptr);
}


void *mtos_malloc_alias(u32 size)
{
  return malloc(size);
}

void mtos_free_alias(void *p_addr)
{
  free(p_addr);
}
