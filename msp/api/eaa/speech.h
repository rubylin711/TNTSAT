/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#ifndef SPEECH_H
#define SPEECH_H

#include <sys/types.h>

// conditional compilation options
#define INCLUDE_KLATT
//#define INCLUDE_MBROLA
#define INCLUDE_SONIC

#if defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN
#define ARCH_BIG
#endif

#ifdef __QNX__
#define NEED_GETOPT
#define NO_VARIADIC_MACROS
#endif


#define PLATFORM_POSIX
#define PATHSEP  '/'
// USE_PORTAUDIO or USE_PULSEAUDIO are now defined in the makefile
//#define USE_PORTAUDIO
//#define USE_PULSEAUDIO
#define USE_NANOSLEEP
#define __cdecl
//#define ESPEAK_API  extern "C"

// will look for espeak_data directory here, and also in user's home directory
#ifndef PATH_ESPEAK_DATA
   #define PATH_ESPEAK_DATA  "./espeak-data"
#endif

typedef unsigned short USHORT;
typedef unsigned char  UCHAR;
typedef double DOUBLEX;
typedef unsigned long long64;   // use this for conversion between pointers and integers



typedef struct {
   const char *mnem;
   int  value;
} MNEM_TAB;
int LookupMnem(MNEM_TAB *table, const char *string);


#ifdef PLATFORM_WINDOWS
#define N_PATH_HOME  230
#else
#ifdef _LZ_LINUX_
#define N_PATH_HOME  160
#else
#define N_PATH_HOME 32
#endif
#endif

extern char path_home[N_PATH_HOME];    // this is the espeak-data directory

extern void strncpy0(char *to,const char *from, int size);
int  GetFileLength(const char *filename);
char *Alloc(int size);
void Free(void *ptr);

#endif // SPEECH_H
