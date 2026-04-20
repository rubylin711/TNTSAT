#ifndef _MT_ADP_DEBUG_H_
#define _MT_ADP_DEBUG_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mt_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MTADP_DEBUG

#ifdef MTADP_DEBUG 

#define MTADP_PRINT   printf
#else

#define MTADP_PRINT 

#endif


#ifdef __cplusplus
}
#endif /*__cplusplus*/


#endif
