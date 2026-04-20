/**
 \file
 \brief sutitle queue
 \author
 \version 1.0
 \author
 \date
 */

#ifndef __SO_QUEUE_H__
#define __SO_QUEUE_H__

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mt_type.h"
#include "mt_unf_subtouput.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define SO_RETURN(val, ret, printstr) \
do \
{ \
    if ((val)) \
    { \
        return (ret); \
    } \
} while (0)

#define SO_CALL_RETURN(val, fun, ret) \
do \
{ \
    if ((val)) \
    { \
        fun; \
        return (ret); \
    } \
} while (0)

#define SO_MEMSET  memset
#define SO_MEMMOVE memmove
#define SO_MEMCPY  memcpy
#define SO_MALLOC(size) malloc(size)

#define SO_FREE(ptr) \
do \
{ \
    if ((ptr)) \
    { \
        free((ptr)); \
        (ptr) = NULL; \
    } \
} while (0)

#define SO_SLEEP(ms) \
do \
{ \
    MT_USLEEP((mt_u32)(ms) * 1000); \
} while (0)

#define SO_NORMAL_BUFF_SIZE   (1024)

typedef mt_void* SO_QUEUE_HANDLE;
typedef MT_UNF_SO_SUBTITLE_INFO_S SO_INFO_S;


/**
\brief create queue
\attention \n
if the size of used buffer is larger than bufsize and the number of node is larger than maxNodeNum, then
can not put node in queue.
\param[in] bufsize bytes of the queue
\param[in] maxNodeNum max number of node in the queue
\param[out] handle handle of the queue

\retval ::MT_SUCCESS if success
\retval ::MT_FAILURE no memory

\see \n
non
*/
mt_s32 SO_QueueInit( mt_u32 bufsize, mt_u32 maxNodeNum, SO_QUEUE_HANDLE *handle );

/**
\brief destroy the queue
\attention \n
non
\param[in] handle handle of the queue

\retval ::MT_SUCCESS success
\retval ::MT_FAILURE queue no exist

\see \n
non
*/
mt_s32 SO_QueueDeinit(SO_QUEUE_HANDLE handle);

/**
\brief free the node in the queue
\attention \n
non
\param[in] handle handle of the queue
\param[in] pstInfo node info

\retval ::MT_SUCCESS success
\retval ::MT_FAILURE node no exist

\see \n
non
*/
mt_s32 SO_QueueFree( SO_QUEUE_HANDLE handle, SO_INFO_S *pstInfo );

/**
\brief get a node
\attention \n
non
\param[in] handle handle of the queue
\param[out] pstInfo node info of getting

\retval ::MT_SUCCESS success
\retval ::MT_FAILURE queue is empty

\see \n
non
*/
mt_s32 SO_QueueGet(SO_QUEUE_HANDLE handle, SO_INFO_S *pstInfo);

/**
\brief get a node, but no delete the node in the queue
\attention \n
non
\param[in] handle handle of the queue
\param[out] pstInfo node info

\retval ::MT_SUCCESS success
\retval ::MT_FAILURE queue is empty

\see \n
non
*/
mt_s32 SO_QueueGetNodeInfoNotDel(SO_QUEUE_HANDLE handle, SO_INFO_S *pstInfo);

/**
\brief insert a node
\attention \n
non
\param[in] handle handle of the queue
\param[out] pstInfo node info

\retval ::MT_SUCCESS success
\retval ::MT_FAILURE no memory or param invalid

\see \n
non
*/
mt_s32 SO_QueuePut(SO_QUEUE_HANDLE handle, const SO_INFO_S *pstInfo);

/**
\brief reset the queue, delete all nodes
\attention \n
non
\param[in] handle handle of the queue

\retval ::MT_SUCCESS success

\see \n
non
*/
mt_s32 SO_QueueReset( SO_QUEUE_HANDLE handle );

/**
\brief reset the queue, delete nodes by pts
\attention \n
non
\param[in] handle handle of the queue

\retval ::MT_SUCCESS successs

\see \n
non
*/
mt_s32 SO_QueueReset_ByPts( SO_QUEUE_HANDLE handle, mt_s64 s64Pts );

/**
\brief remove all nodes
\attention \n
non
\param[in] handle handle of the queue

\retval ::MT_SUCCESS success

\see \n
non
*/
mt_s32 SO_QueueRemove(SO_QUEUE_HANDLE handle);

/**
\brief get total number of the nodes
\attention \n
non
\param[in] handle handle of the queue

\retval ::MT_SUCCESS

\see \n
non
*/
mt_s32  SO_QueueNum(SO_QUEUE_HANDLE handle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SO_QUEUE_H__ */

