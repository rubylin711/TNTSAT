/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdarg.h>

#include "mt_type.h"
#include "mtos_printk.h"

MT_BOOL PRINT_ONOFF = 1;

void mtos_close_printk()
{
    return;
}

void mtos_open_printk()
{
    return;
}

void mtos_register_putchar(s32 (*p_printchar)(u8, u8))
{
    return;
}

void mtos_register_getchar(s32 (*p_getchar)(u8, u8 *, u32))
{
    return;
}

int mt_console_printk(const char *p_fmt, char *p_args)
{
    return 0;
}

int mtos_printk(const char *p_format, ...)
{
    va_list argptr;
    //int cnt = 0;

    va_start(argptr, p_format);
    (void)vprintf(p_format, argptr);
    va_end(argptr);

    return 0;
}

int mtos_printk_1(const char *p_format, ...)
{
    return 0;
}

int mtos_printk_0(const char *p_fmt, ...)
{
    return 0;
}

int mtos_printk_f(const char *format, double f_num)
{
    return 0;
}

#if 0
int ck_vsnprintf(char *buf, unsigned int size, const char *fmt, char *args)
{
  return 0;
}
#endif


#if 1
void OS_PRINTF( const char *p_fmt, ...)
{
    return;
}
void OS_PRINTK( const char *p_fmt, ...)
{
    return;
}

#else
void OS_PRINTF( const char *p_fmt, ...)
{

    va_list     ptr;
    char        printf_buffer[PRINTF_BUFFER_SIZE];

	if(PRINT_ONOFF)
	{
		va_start(ptr,p_fmt);
		vsnprintf(&printf_buffer[0], (size_t)PRINTF_BUFFER_SIZE, p_fmt, ptr);
		va_end(ptr);

		    printf_buffer[PRINTF_BUFFER_SIZE -1] = '\0';
		printf("%s", &printf_buffer[0]);
	}
}

void OS_PRINTK( const char *p_fmt, ...)
{

    va_list     ptr;
    char        printf_buffer[PRINTF_BUFFER_SIZE];

	if(PRINT_ONOFF)
	{
		va_start(ptr,p_fmt);
		vsnprintf(&printf_buffer[0], (size_t)PRINTF_BUFFER_SIZE, p_fmt, ptr);
		va_end(ptr);

		printf_buffer[PRINTF_BUFFER_SIZE -1] = '\0';
		printf("%s", &printf_buffer[0]);
	}
}
#endif