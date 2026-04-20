///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// <summary>
/// Trace.h
/// </summary>

#include <string>

//Tracing with linenumbers and filenames
class Tracer
{
public:
        enum
        {
            Flag_Error =            0x0001,
            Flag_FileAndLine =      0x0002,
            Flag_ErrorFileAndLine = 0x0003
        };
                Tracer(const char* file, int line, int flags = 0);
        void    Write(const char* str);
        void    operator()(const char* format, ...);
        void    operator()(const std::string& str);
        void    AssertFailed(const char* how);

const   char*      _file;
        int        _line;
        int        _flags;
};


//Debug tracing with line numbers and file names
#ifdef TV2INTERNAL
#define PRINT(x) (Tracer(__FILE__,__LINE__) x )
#define TRACE(x) (Tracer(__FILE__,__LINE__, Tracer::Flag_FileAndLine) x )
#define TRACE_ERROR(x) (Tracer(__FILE__,__LINE__, Tracer::Flag_ErrorFileAndLine) x)
#define TRACE_ASSERT(_x) if (!(_x)) { (Tracer(__FILE__,__LINE__, Tracer::Flag_ErrorFileAndLine)).AssertFailed(#_x); }
#else
#define PRINT(x) 
#define TRACE(x) 
#define TRACE_ERROR(x) 
#define TRACE_ASSERT(_x) 
#endif


#define CE_AV_LOG(a,b,c)

#define CE_AV_TUNERSESSION_LOG(x)
#define CE_AV_CLOCK_LOG(x)
#define CE_AV_TUNERTHREAD_LOG(x)
#define CE_AV_HALFACTORY_LOG(x)
#define CE_AV_DECODER_LOG(x)
#define CE_AV_RECEIVERDVR_LOG(t, x)
#define CE_AV_DVRMANAGER_LOG(t, x)
#define CE_AV_DVFS_LOG(t, x)
#define CE_AV_SYNCQUEUE_LOG(t, x)
#define CE_AV_REAPER_LOG(t, x)

