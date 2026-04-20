/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
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
#define MT_CMDLINE_H_
#include <mt_type.h>

extern MT_S32 MTCommandeInit(MT_CHAR *pCmdPr);
extern MT_S32 MTCommandDeInit(void);
extern MT_S32 MTCommandRun(MT_CHAR *cmdStr);
extern MT_S32 MTCommandRegister(const char *name,void ( * func)(const char *arg),const char * help);
extern MT_S32 MTCommandDivArgc(const char *pInput, char *pOutput, int argcNum);
extern MT_BOOL MTCommandGetArgc(const char *pInput, char *pOutput, int argcNum);
extern MT_U32 MTConverStrToInt(const char *str);
extern MT_S32 MT_SystemAsh(const char *fmt, ...);

#endif
