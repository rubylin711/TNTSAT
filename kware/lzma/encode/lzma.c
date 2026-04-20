/* 
lzma.c
Interface of LZMA Decoder

This file written and distributed to public domain by Igor Pavlov.
This file is part of LZMA SDK 4.26 (2005-08-05)
*/
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
*/

#include "lzma.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_type.h"
#include "mtos_printk.h"
#include "mtos_mem.h"


//#include "lzmadecode.h"

const char *kCantReadMessage = "Can not read input file";
const char *kCantWriteMessage = "Can not write output file";
const char *kCantAllocateMessage = "Can not allocate memory";
typedef unsigned char  Uint8;
//typedef unsigned int size_t;
typedef unsigned int                     UInt32;
typedef unsigned char  Byte;	

#ifdef _LZMA_OUT_READ
#define kOutBufferSize (1 << 15)
unsigned char g_OutBuffer[kOutBufferSize];
#endif

#undef MT_ASSERT
#define MT_ASSERT(__x)  \
        do{     \
                if(!(__x)) {    \
                        printf("%s(%s, %d ): ASSERT(%s) failed\n", __FUNCTION__, __FILE__, __LINE__, #__x);     \
                        while(1);       \
                }       \
        }while(0)

typedef unsigned long UInt64;

typedef struct
{
  int (*Read)(void *p, void *buf, size_t *size);
    /* if (input(*size) != 0 && output(*size) == 0) means end_of_stream.
       (output(*size) < input(*size)) is allowed */
} ISeqInStream;

typedef struct
{
  size_t (*Write)(void *p, const void *buf, size_t size);
    /* Returns: result - the number of actually written bytes.
      (result < size) means error */
} ISeqOutStream;

typedef struct
{
  int (*Progress)(void *p, UInt64 inSize, UInt64 outSize);
    /* Returns: result. (result != SZ_OK) means break.
       Value (UInt64)(Int64)-1 for size means unknown value. */
} ICompressProgress;

typedef struct
{
  void *(*Alloc)(void *p, size_t size);
  void (*Free)(void *p, void *address); /* address can be 0 */
} ISzAlloc;
typedef struct _CLzmaEncProps
{
  int level;       /*  0 <= level <= 9 */
  UInt32 dictSize; /* (1 << 12) <= dictSize <= (1 << 27) for 32-bit version
                      (1 << 12) <= dictSize <= (1 << 30) for 64-bit version
                       default = (1 << 24) */
  int lc;          /* 0 <= lc <= 8, default = 3 */
  int lp;          /* 0 <= lp <= 4, default = 0 */
  int pb;          /* 0 <= pb <= 4, default = 2 */
  int algo;        /* 0 - fast, 1 - normal, default = 1 */
  int fb;          /* 5 <= fb <= 273, default = 32 */
  int btMode;      /* 0 - hashChain Mode, 1 - binTree mode - normal, default = 1 */
  int numHashBytes; /* 2, 3 or 4, default = 4 */
  UInt32 mc;        /* 1 <= mc <= (1 << 30), default = 32 */
  unsigned writeEndMark;  /* 0 - do not write EOPM, 1 - write EOPM, default = 0 */
  int numThreads;  /* 1 or 2, default = 2 */
} CLzmaEncProps;

//extern void* mtos_malloc(unsigned int size);


//extern void  mtos_free(void *p_addr);

/*This is porting for montage board*/
#if 0
static void *SzAlloc(void *p, size_t size) 
{ p = p; return mtos_malloc(size);

}

static void SzFree(void *p, void *address) 
{ p = p;  
  if (address == 0)
  	return;
  mtos_free(address); 
}
#endif

/*This is porting for windows*/

static unsigned long g_mem_ptr = 0;
static unsigned int g_cache_size = 0;

int PrintError(char *buffer, const char *message);

static void *SzAlloc(void *p, size_t size) 
{
//  OS_PRINTF("lzma alloc size %d\n",size);
  //p = p;

  if((g_mem_ptr == 0) || (g_mem_ptr == g_cache_size))
  {
    //malloc by self
    return mtos_malloc(size);
  }

  /* use user buffer, make size 16 byte alignment */
  size += 16 - (size & 15);
  g_mem_ptr += size;
  MT_ASSERT(g_mem_ptr < g_cache_size);
  
  return (void *)(g_mem_ptr-size);
}

static void SzFree(void *p, void *address) 
{
  //p = p;  

  if (address == 0)
  {
    return;
  }

  if((g_mem_ptr == 0) || (g_mem_ptr == g_cache_size))
  {
    mtos_free(address);
    return;
  }  
}

static ISzAlloc g_Alloc = { SzAlloc, SzFree };

int PrintError(char *buffer, const char *message)
{
//  sprintf(buffer + strlen(buffer), "\nError: ");
//  sprintf(buffer + strlen(buffer), message);
	return 1;
}




extern int LzmaEncode(Byte *dest, unsigned int *destLen, const Byte *src, unsigned int srcLen,
    const CLzmaEncProps *props, Byte *propsEncoded, unsigned int *propsSize, int writeEndMark,
    ICompressProgress *progress, ISzAlloc *alloc, ISzAlloc *allocBig);
extern void LzmaEncProps_Init(CLzmaEncProps *p);
extern int ERomPrintFunc (

        int             cpu,    
        const char     *fmt, 
        ...                     
);

#if 0
int lzmacompress(unsigned char *dest, unsigned int  *destLen, const unsigned char *src, unsigned int  srcLen,unsigned char *header,unsigned int *headerSize )
{
	CLzmaEncProps props;
	int result;
	LzmaEncProps_Init(&props);
	result = LzmaEncode(dest + 13, destLen, src, srcLen, &props, header, headerSize, 0,
      0, &g_Alloc, &g_Alloc);

	memcpy(dest, header, *headerSize);
	return result;

}
#endif


int lzmacompress(unsigned char *dest, unsigned int  *destLen,
      const unsigned char *src, unsigned int  srcLen, unsigned char *p_cache,unsigned int cache_size)
{
	CLzmaEncProps props;
	int result;
	unsigned char header[13];
   unsigned int headerSize = 5;

    g_mem_ptr = (unsigned long)p_cache;
    g_cache_size = g_mem_ptr + cache_size;

	LzmaEncProps_Init(&props);
	result = LzmaEncode(dest + 13, destLen, src, srcLen, &props, header, &headerSize, 0,
      0, &g_Alloc, &g_Alloc);

	memcpy(dest, header, headerSize);
  *destLen += 13;

	return result;

}



#if 0
extern int lzmacompress(unsigned char *dest, unsigned int  *destLen, const unsigned char *src, unsigned int  srcLen, unsigned char *header, unsigned int *headerSize);

//the default LZMA decompress inflate buffer size, malloc from heap
#define MBOOT_DECOMPRESS_BUFF_SIZE     (100*1024)

//default max length of maincode, just for LZMA function use,
#define DEFAULT_MAX_CODE_LEN            (2 * (MBYTES))


s32 code_decompress2(u32 startup, unsigned int size)
{
  //extern u32 __RAM_BASE;
  //volatile u32 *bl_start = &__RAM_BASE;
  u32 inflate_len = 0, code_size = 0, start = 0, i = 0;
  u32 *p_code_start = NULL;
  u32 *p_code_bk = NULL;
  u8 *p_inflate = NULL;
  s32 ret = ERR_FAILURE;

  MPRINTF("UNZIP BEGIN, init inflate buffer \n");
  p_inflate = (u8 *)mtos_malloc(MBOOT_DECOMPRESS_BUFF_SIZE);
  if(NULL == p_inflate)
  {
    MPRINTF("malloc inflate buffer failed!\n");
    MT_ASSERT(0);
  }
  init_fake_mem_lzma(p_inflate, MBOOT_DECOMPRESS_BUFF_SIZE);
  inflate_len = DEFAULT_MAX_CODE_LEN;


  OS_PRINTF("Before lzma_decompress\n");


  ret = lzma_decompress((void *)startup, &inflate_len, (void *)(0x1000 - 13), size);
  OS_PRINTF("After lzma_decompress\n");

  mtos_free(p_inflate);

  if(ret != 0)
  {
    OS_PRINTF("fail\n");
    return ERR_FAILURE;
  }
  else
  {
    OS_PRINTF("Success\n");
    MPRINTF("decompress OK, real img size[%d]\n", inflate_len);
    return SUCCESS;
  }
}

int memery_compare(unsigned char *source, unsigned char *dest, unsigned int size)
{
   
    while (size--)
    {
      if (*source++ != *dest++)
      {  
        OS_PRINTF("Memory is not the same\n");
		return 1;
      }
	  
    }
  return 0;
}


void test_lzma_compress(void)
{
  unsigned char header[13];
  unsigned int headerSize = 5;
  
  unsigned int dest_size;
  unsigned int src_size = 0x7d000;
  dest_size = src_size;
  
  unsigned char *dest_mem = (unsigned char *)0x1000;
  unsigned char *dest_temp = dest_mem - 13;
  unsigned char *dest_mem2 = (unsigned char *)0x200000;
  unsigned char *dest_mem3 = (unsigned char *)0x300000;
  
  const unsigned char *src_mem = dest_mem2;
  OS_PRINTF("Start lzmacompress\n");
  memset(dest_mem , 0, 1024 *1024);
  int k;
  
  for (k = 0; k < 0x7d000; k++)
  {
    *(dest_mem2	 + k) = 0x12; 
  
  }


  lzmacompress(dest_mem, &dest_size, src_mem, src_size, header, &headerSize);

  memcpy(dest_temp, header, headerSize);
  




  memset(dest_mem3 , 0, 1024 *1024);
  code_decompress2(0x300000, dest_size + 13);


  int ret22 = memery_compare(dest_mem2, dest_mem3, 0x7d000);


  OS_PRINTF("ret22 is %d\n", ret22);

  while(1);


}

#endif

//----------------------------------------------------------------------------
// Boolean IsGzip(Uint32 srcAddr)
// Description:
//    Check the compressiom method, whether LZMA or GZIP.
//
// return: TRUE, GZIP
//         FALSE, LZMA
//----------------------------------------------------------------------------
int IsGzip(ulong srcAddr)
{
	static int gz_header[2] = {0x1f, 0x8b};
	int len;
	Uint8 *srcStream = (Uint8 *)srcAddr;
	int ret = 1;
	
    /* Check the gzip magic header */
    for (len = 0; len < 2; len++)
    {
		if (*(srcStream ++) != gz_header[len])
		{
	    	ret = 0;
		}
    }

    return ret;
}

