///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __ERRTRACING_H_
#define __ERRTRACING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pkTestFramework.h>

#ifndef ERRTRACE_PRINTMSG

   #define ERRTRACE_PRINTMSG(printf_exp)   TF_Printf printf_exp

#endif  // ERRTRACE_PRINTMSG


//===========================================================================
// Tracing Macros
//===========================================================================
#define TraceInfo0(fmt)            ERRTRACE_PRINTMSG(("%s(%d) : *** TRACEInfo *** " fmt "\n", __FILE__, __LINE__))
#define TraceInfo1(fmt,one)        ERRTRACE_PRINTMSG(("%s(%d) : *** TRACEInfo *** " fmt "\n", __FILE__, __LINE__, one))
#define TraceInfo2(fmt,one,two)    ERRTRACE_PRINTMSG(("%s(%d) : *** TRACEInfo *** " fmt "\n", __FILE__, __LINE__, one, two))
#define TraceInfo3(fmt,one,two,three)                                      \
    ERRTRACE_PRINTMSG(("%s(%d) : *** TRACEInfo *** " fmt "\n", __FILE__, __LINE__, one, two, three))
#define TraceInfo4(fmt,one,two,three,four)                                 \
    ERRTRACE_PRINTMSG(("%s(%d) : *** TRACEInfo *** " fmt "\n", __FILE__, __LINE__, one, two, three, four))
#define TraceInfo5(fmt,one,two,three,four,five)                            \
    ERRTRACE_PRINTMSG(("%s(%d) : *** TRACEInfo *** " fmt "\n", __FILE__, __LINE__, one, two, three, four, five))
#define TraceInfo6(fmt,one,two,three,four,five,six)                        \
    ERRTRACE_PRINTMSG(("%s(%d) : *** TRACEInfo *** " fmt "\n", __FILE__, __LINE__, one, two, three, four, five, six))

#define TraceResult(code)   (code); ERRTRACE_PRINTMSG(("%s(%d) : *** TRACE *** "  \
                                "code = 0x%x!\n", __FILE__, __LINE__, (code)))


#define TRACEPTR_EXIT(pkRes, ptr)                       \
                        {                               \
                            if (NULL == (ptr))          \
                            {                           \
                                pkRes = TraceResult(pkE_OUTOFMEMORY); \
                                goto exit;              \
                            }                           \
                        }

#define TRACEPK_EXIT(pkRes, action)                     \
                        {                               \
                            (pkRes) = (action);         \
                            if (pkFAILED(pkRes))        \
                            {                           \
                                TraceResult(pkRes);     \
                                goto exit;              \
                            }                           \
                        }

#define TraceError(code)    {                                   \
                                pkRESULT pkFOOFOO = (code);     \
                                if (pkFAILED(pkFOOFOO))         \
                                {                               \
                                    TraceResult(pkFOOFOO);      \
                                }                               \
                            }

#ifdef __cplusplus
}
#endif

#endif // __ERRTRACING_H_
