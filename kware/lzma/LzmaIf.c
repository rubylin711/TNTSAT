/*-----------------------------------------------------------------------------
 文件名  : LzmaIf.c
 作者    : james
 版本    :
 完成日期: 2004-07-15
 文件描述: LZMA压缩算法的解压缩接口的源文件.
 备注    :
 函数列表: 主要函数列表，每条记录应包括函数名及功能简要说明
           (1):
           (2):
           (3):
           ......

 提供给外部的接口: 本文件提供给外部的接口
           (1):
           (2):

 需要外部提供的接口: 本文件需要外部提供的接口
           (1):
           (2):

 修改历史:
        1. 修改者   :
           时间     :
           版本     :
           修改原因 :
        2. ...
-----------------------------------------------------------------------------*/
#define MAKE_INCLUDE_COMPRESS_TYPE_LZMA

//#ifdef MAKE_INCLUDE_COMPRESS_TYPE_LZMA

/*================================头文件引用=================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mt_type.h"
#include "LzmaDecode.h"
#include "mtos_printk.h"
#include "mtos_mem.h"



/*===============================外部变量引用================================*/

/*===============================全局变量定义================================*/

/*===============================外部函数引用================================*/


/* 打印输出函数 */
extern unsigned long Drv_Print(const char *pStr, ...);
#if 1
#define LZMA_Print OS_PRINTF
#else
#ifdef WIN32
#define LZMA_Print OS_PRINTF
#else
#define LZMA_Print(...) do{} while(0)
#endif
#endif // End for UNZIP_DEBUG

#ifndef MAKE_BOOTROM
/* 清狗的回调函数 */
typedef void (* Drv_Func)(void);
extern Drv_Func funcgDrvFeedSoftWatchdogHook;
#endif

static u8 * g_lzma_sys_mem = NULL;
static int g_lzma_mem_size = 0;
static int g_lzma_mem_used = 0;

#ifdef __WANDA__
#undef MT_ASSERT
#define MT_ASSERT(__x)	\
	do{	\
		if(!(__x)) {	\
			printf("%s(%s, %d ): ASSERT(%s) failed\n", __FUNCTION__, __FILE__, __LINE__, #__x);	\
			while(1);	\
		}	\
	}while(0)
#endif

void init_fake_mem_lzma(void *p_sys, int mem_size);
void* lzma_malloc(int size);
void* lzma_free(unsigned char * free_ptr);
int LzmaReadCompressed(void *object, unsigned char **buffer, unsigned int *size);
unsigned long lzma_decompress( void *p_dst, unsigned long *p_uldstlen, void *p_src, unsigned long ulsrclen );

void init_fake_mem_lzma(void *p_sys, int mem_size)
{
  g_lzma_sys_mem = p_sys;
  g_lzma_mem_size = mem_size;
  g_lzma_mem_used = 0;
  LZMA_Print("\nlzma:mem[0x%x] size[%d]\n",p_sys,mem_size);
}

void* lzma_malloc(int size)
{
  void *p_res = NULL;
  if(NULL == g_lzma_sys_mem)
  {
    return NULL;
  }
  p_res = g_lzma_sys_mem;
  g_lzma_sys_mem += size;
  g_lzma_mem_used += size;
  LZMA_Print("\nlzma:malloc [%d]\n",size);
  if(g_lzma_mem_used > g_lzma_mem_size)
  {
    LZMA_Print("\nERROR: no memory available\n");
    //MT_ASSERT(0);
  }
  return p_res;
}

void* lzma_free(unsigned char * free_ptr)
{
  return 0;
}

#ifdef _LZMA_IN_CB
/* 向上对齐 */
#define ALIGN_UP(addr,align)       ( ((unsigned int)(addr) + (unsigned int)((align) - 1)) & ~(unsigned int)((align) - 1) )

#define MAX_DECOMPRESS_READ_COUNT  10

typedef struct _CBuffer
{
  ILzmaInCallback InCallback;
  unsigned char *Buffer;
  unsigned int Size;
  unsigned int PerSize;
  unsigned int PerCount;
} CBuffer;

int LzmaReadCompressed(void *object, unsigned char **buffer, unsigned int *size)
{

    CBuffer *bo = (CBuffer *)object;

    #ifdef MAKE_BOOTROM
    /* 输出解压缩的百分比 */
    if( bo->PerCount != 0 )
    {
        LZMA_Print("%ld%%...", bo->PerCount * 100 / MAX_DECOMPRESS_READ_COUNT );
    }
    #else
    /* 清狗 */
 //   if( funcgDrvFeedSoftWatchdogHook != NULL )
    {
   //     funcgDrvFeedSoftWatchdogHook();
    }
    //Drv_FeedHardWatchDog();
    #endif

    if( bo->PerCount == ( MAX_DECOMPRESS_READ_COUNT - 1 ) )
    {
        *size   = bo->Size - bo->PerSize * bo->PerCount;
    }
    else if ( bo->PerCount < ( MAX_DECOMPRESS_READ_COUNT - 1 ) )
    {
        *size   = bo->PerSize;
    }
    else
    {
        LZMA_Print("\r\n  LzmaReadCompressed() : Read compressd mem fail !");
        return LZMA_RESULT_NOT_ENOUGH_MEM;
    }

    *buffer     = bo->Buffer + bo->PerSize * bo->PerCount;


    bo->PerCount ++;

    return LZMA_RESULT_OK;

}
#endif


/*-----------------------------------------------------------------------------
 函数名称    : LZMA_Decompress();
 功能        : LZMA压缩算法的解压缩接口;
 输入参数    : p_dst,        解压缩的目的内存的地址;
               p_uldstlen,   解压缩的目的内存的长度;
               p_src,        解压缩的源内存的地址;
               ulsrclen,    解压缩的源内存的长度;
 输出参数    : *p_uldstlen,  如果解压缩成功, 实际解压后的长度;
 返回值      : 0,           成功;
               其他,        失败.
 函数调用说明:
 典型使用示例:
-----------------------------------------------------------------------------*/
unsigned long lzma_decompress( void *p_dst, unsigned long *p_uldstlen, void *p_src, unsigned long ulsrclen )
{
  unsigned int compressedSize, outSize, outSizeProcessed, lzmaInternalSize;
  void *inStream, *outStream, *lzmaInternalData;
  unsigned char properties[5];
  unsigned char prop0;
  int ii;
  int lc, lp, pb;
  int res;
  #ifdef _LZMA_IN_CB
  CBuffer bo;
  #endif
  unsigned char *pCharSrc = (unsigned char*)p_src;

//  if( ( NULL == p_dst ) || ( NULL == p_uldstlen ) || ( NULL == p_src ) )
  if( NULL == p_uldstlen )
  {
    LZMA_Print("\r\n  LZMA_Decompress() : Input prt is null");
    return 1;
  }

  if( ( 0 == *p_uldstlen ) || ( 0 == ulsrclen ) )
  {
    LZMA_Print("\r\n  LZMA_Decompress() : Input mem len is zero");
    return 1;
  }

  memcpy(properties, p_src, sizeof(properties) );
  pCharSrc += sizeof(properties);

  outSize = 0;
  for (ii = 0; ii < 4; ii++)
  {
    unsigned char b;
    b = *pCharSrc;
    pCharSrc ++;
    outSize += (unsigned int)(b) << (ii * 8);
  }

  if (outSize == 0xFFFFFFFF)
  {
    LZMA_Print("\r\n  LZMA_Decompress() : stream version is not supported");
    return 1;
  }

  for (ii = 0; ii < 4; ii++)
  {
    unsigned char b;
    b = *pCharSrc;
    pCharSrc ++;
    if (b != 0)
    {
      LZMA_Print("\r\n  LZMA_Decompress() :  too long file");
      return 1;
    }
  }

  compressedSize = ulsrclen - 13;
  inStream = pCharSrc;

  prop0 = properties[0];
  if (prop0 >= (9*5*5))
  {
    LZMA_Print("\r\n  LZMA_Decompress() : Properties error");
    return 1;
  }
  for (pb = 0; prop0 >= (9 * 5);
    pb++, prop0 -= (9 * 5));
  for (lp = 0; prop0 >= 9;
    lp++, prop0 -= 9);
  lc = (int)prop0;

  lzmaInternalSize = (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (lc + lp)))* sizeof(CProb);

  #ifdef _LZMA_OUT_READ
  lzmaInternalSize += 100;
  #endif

  outStream = p_dst;
  if (outSize >= *p_uldstlen)
  {
    LZMA_Print("\r\n  LZMA_Decompress() : outSize is not enough. outSize(0x%lx) >= *p_uldstlen(0x%lx) ", outSize, *p_uldstlen);
    return 1;
  }

  lzmaInternalData = lzma_malloc(lzmaInternalSize);

  if (lzmaInternalData == 0)
  {
    LZMA_Print("\r\n  LZMA_Decompress() : can't allocate lzmaInternalData ! lzmaInternalSize = 0x%lx", lzmaInternalSize );
    return 1;
  }

  #ifdef _LZMA_IN_CB
  bo.InCallback.Read = LzmaReadCompressed;
  bo.Buffer = (unsigned char *)inStream;
  bo.Size = compressedSize;
  bo.PerCount = 0;
  bo.PerSize = ALIGN_UP( compressedSize / MAX_DECOMPRESS_READ_COUNT, 0x10 );
  #endif

  #ifdef _LZMA_OUT_READ
  {
    UInt32 nowPos;
    unsigned char *dictionary;
    UInt32 dictionarySize = 0;
    int i;
    for (i = 0; i < 4; i++)
      dictionarySize += (UInt32)(properties[1 + i]) << (i * 8);
    dictionary = (unsigned char *)lzma_malloc(dictionarySize);
    if (dictionary == NULL)
    {
      LZMA_Print("\r\n  LZMA_Decompress() : can't allocate dictionary. dictionarySize = 0x%lx", dictionarySize);
      return 1;
    }
    LzmaDecoderInit((unsigned char *)lzmaInternalData, lzmaInternalSize,
        lc, lp, pb,
        dictionary, dictionarySize,
        #ifdef _LZMA_IN_CB
        &bo.InCallback
        #else
        (unsigned char *)inStream, compressedSize
        #endif
        );
    for (nowPos = 0; nowPos < outSize;)
    {
      UInt32 blockSize = outSize - nowPos;
      UInt32 kBlockSize = 0x10000;
      if (blockSize > kBlockSize)
        blockSize = kBlockSize;
      res = LzmaDecode((unsigned char *)lzmaInternalData,
      ((unsigned char *)outStream) + nowPos, blockSize, &outSizeProcessed);
      if (res != 0)
      {
        LZMA_Print("\r\n  LZMA_Decompress() : error = %d\n", res);
        return 1;
      }
      if (outSizeProcessed == 0)
      {
        outSize = nowPos;
        break;
      }
      nowPos += outSizeProcessed;
    }
    lzma_free(dictionary);
  }

  #else
  res = LzmaDecode((unsigned char *)lzmaInternalData, lzmaInternalSize,
      lc, lp, pb,
      #ifdef _LZMA_IN_CB
      &bo.InCallback,
      #else
      (unsigned char *)inStream, compressedSize,
      #endif
      (unsigned char *)outStream, outSize, &outSizeProcessed);
  outSize = outSizeProcessed;
  #endif

  if (res != 0)
  {
    LZMA_Print("\r\n  LZMA_Decompress() : error = %d\n", res);
    return 1;
  }

  lzma_free(lzmaInternalData);

  *p_uldstlen = outSize;

  return 0;

}


//#endif /* MAKE_INCLUDE_COMPRESS_TYPE_LZMA */

