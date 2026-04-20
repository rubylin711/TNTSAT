//------------------------------------------------------------------------------
// File: config.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
// This file should be modified by some developers of C&M according to product version.
//------------------------------------------------------------------------------

#ifndef __CONFIG_H__
#define __CONFIG_H__

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
#define PLATFORM_WIN32
#elif defined(linux) || defined(__linux) || defined(ANDROID)
#define PLATFORM_LINUX
#else
#define PLATFORM_NON_OS
#endif

#ifdef PLATFORM_WIN32
#include <windows.h>
#endif

#if defined(_MSC_VER)
#define inline _inline
#elif defined(__GNUC__)
#elif defined(__ARMCC__)
#else
#error "Unknown compiler."
#endif

#define API_VERSION_MAJOR 5
#define API_VERSION_MINOR 4
#define API_VERSION_PATCH 7
#define API_VERSION ((API_VERSION_MAJOR << 16) | (API_VERSION_MINOR << 8) | API_VERSION_PATCH)

#if defined(PLATFORM_NON_OS) || defined(ANDROID) || defined(MFHMFT_EXPORTS) || defined(OMXIL_COMPONENT) || defined(DXVA_UMOD_DRIVER) || defined(__MINGW32__)
//#define SUPPORT_FFMPEG_DEMUX
#else
#define SUPPORT_FFMPEG_DEMUX
#endif

//#define REPORT_PERFORMANCE

//#define SUPPORT_MEM_PROTECT

#define PROJECT_ROOT "../../../../"
// do not define BIT_CODE_FILE_PATH in case of multiple product support. because wave410 and coda980 has different firmware binary format.
#define CORE_0_BIT_CODE_FILE_PATH "/mnt/michelangel.bin"                     // for wave410
#define CORE_1_BIT_CODE_FILE_PATH "../../../../design/asm_s/out/coda980.out" // for coda9xx

#define SUPPORT_REPEAT_EMPTY_INTERRUPT
#define SUPPORT_FIO_ACCESS
#define SUPPORT_DPB_FLUSH_ON_RESET

#endif /* __CONFIG_H__ */
