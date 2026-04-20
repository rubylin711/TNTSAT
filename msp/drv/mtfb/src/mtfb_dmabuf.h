/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __MTFB_DMABUF_H__
#define __MTFB_DMABUF_H__


/*********************************add include here******************************/





/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C"
{
#endif
#endif /* __cplusplus */



/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

#include <linux/dma-buf.h>


/******************************* API declaration *****************************/
struct dma_buf *mtfb_memblock_export(phys_addr_t base, size_t size, int flags);

#ifdef __cplusplus

#if __cplusplus

}
#endif
#endif /* __cplusplus */

#endif /* __MTFB_DMABUF_H__ */

