/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*!
@~chinese
@file mt_cmdline.h
@brief 命令行模块头文件

@~english
@file mt_cmdline.h
@brief the header file of Command

*/

#ifndef MT_CMDLINE_H_
#define  MT_CMDLINE_H_
#include <mt_type.h>

typedef enum
{
    MT_RESOURCE_NO    = 0,
    MT_RESOURCE_OSD   = 1,
    MT_RESOURCE_DISP  = 2,
    MT_RESOURCE_SND   = 4,
    MT_RESOURCE_DMX   = 8,
    MT_RESOURCE_WIFI  = 16,
    MT_RESOURCE_ETH   = 32,
    MT_RESOURCE_FRONTPANEL = 64,
    MT_RESOURCE_IR    = 128,
    MT_RESOURCE_BUT,
}MT_RESOURCE_TYPE;


extern MT_S32    MTCommandeInit(void);
extern MT_S32    MTCommandRun(MT_CHAR* cmdStr);
extern MT_S32    MTCommandRegister(const char *name,void ( * func)(mt_u8 argc,const char *argv[]),const char * help, u32 useres) ;

extern MT_BOOL   MTCommandGetArgs(const char* pInput, char* pOutput[], int *argc);
 #endif
