/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_HDMI_DEBUG_H__
#define __DRV_HDMI_DEBUG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "mt_type.h"
#include "drv_abs_debug.h"

#define HDMI_DEBUG_LOGTYPE                  1
#define HDMI_DEBUG_LOGMASK                  2
#define HDMI_DEBUG_LOGCATCH                 3

#define HDMI_LOGMASK_ADD_FUNC               1   // echo logmask +cec > /proc/msp/hdmi0
#define HDMI_LOGMASK_DEC_FUNC               2   // echo logmask -cec > /proc/msp/hdmi0
#define HDMI_LOGMASK_ONLY_FUNC              3   // echo logmask *cec > /proc/msp/hdmi0
#define HDMI_LOGMASK_ALL_FUNC               4   // echo logmask * > /proc/msp/hdmi0

#define HDMI_DEBUG_PRINT_ALL                (0x1 << ABS_FUNC_0)
#define HDMI_DEBUG_PRINT_EDID               (0x1 << ABS_FUNC_1)
#define HDMI_DEBUG_PRINT_HDCP               (0x1 << ABS_FUNC_2)
#define HDMI_DEBUG_PRINT_CEC                (0x1 << ABS_FUNC_3)
#define HDMI_DEBUG_PRINT_HPD                (0x1 << ABS_FUNC_4)
#define HDMI_DEBUG_PRINT_COM                (0x1 << ABS_FUNC_5)
#define HDMI_DEBUG_PRINT_END                (0x1 << ABS_FUNC_6)

#define HDMI_DBG_KMALLOC(a)                 ABS_KMALLOC(MT_ID_HDMI, a)
#define HDMI_DBG_KFREE(a)                   ABS_KFREE(MT_ID_HDMI, a)
#define HDMI_DBG_VMALLOC(a)                 ABS_VMALLOC(MT_ID_HDMI, a)
#define HDMI_DBG_VFREE(a)                   ABS_VFREE(MT_ID_HDMI, a)
#define HDMI_DBG_MEMSET(a, b, c)            ABS_MEMSET(a, b, c)

#define HDMI_PTK_PRINTK(args...)            ABS_PTK(## args)
#define HDMI_PTK_FATAL(fmt...)              ABS_FATAL(MT_ID_HDMI, fmt)
#define HDMI_PTK_ERR(fmt...)                ABS_ERR(MT_ID_HDMI, fmt)
#define HDMI_PTK_WARN(fmt...)               ABS_WARN(MT_ID_HDMI, fmt)
#define HDMI_PTK_INFO(fmt...)               ABS_INFO(MT_ID_HDMI, fmt)

//#define HDMI_DBG_FILE_DEF                   "/root/HDMI_debug.log"  //default file path
#define HDMI_DBG_FILE_DEF                   "/HDMI_debug.log"  //default file path

//#ifdef HDMI_DEBUG
mt_s32  HDMI_DbgInit(mt_void);
mt_void HDMI_DbgDeInit(mt_void);

mt_s32  ABS_ClearMem(ABS_DBG_S_PTR pABSdbg);
mt_s32  HDMI_SetDebLevel(ABS_DBG_LEVEL_T level);

/* print the strings to memory */
mt_void HDMI_Dbg(MT_U8 u8DbgLevel, mt_u32 u32HDMI_FuncNum, const MT_CHAR *fmt, ...);

/* print the strings in memory to file or serial */
//echo logcatch > /proc/msp/hdmi0
mt_void HDMI_DbgMemPrint(mt_void);

/* set the logtype, file or serial */
//echo logtype [serial/file/all] > /proc/msp/hdmi0
mt_void  HDMI_DbgSetType(PRINT_TO_DEV_E logtype, const MT_CHAR *pFilePath);

/* set the function mask */
// echo logMask [ cec/hdcp/hpd/edid/all ] > /proc/msp/hdmi0
mt_void  HDMI_DbgSetMask(mt_u32 CmdType, mt_u32 LogMask);

/* process the cmd come from hdmi_ProcWrite */
mt_s32  HDMI_ProcessCmd(mt_u32 u32CmdType, const MT_CHAR *pCmdParam, const MT_CHAR *pFilePath);

#define HDMI_DBG_INFO(no, fmt...)           HDMI_Dbg(ABS_DBG_INFO, no, fmt)
#define HDMI_DBG_WARN(no, fmt...)           HDMI_Dbg(ABS_DBG_WARN, no, fmt)
#define HDMI_DBG_ERR(no, fmt...)            HDMI_Dbg(ABS_DBG_ERROR, no, fmt)
#define HDMI_DBG_FATAL(no, fmt...)          HDMI_Dbg(ABS_DBG_FATAL, no, fmt)

//#else
//
//#define HDMI_dbg_Init()                     0
//#define HDMI_dbg_DeInit()
//#define HDMI_set_deb_level(a)               0
//#define HDMI_dbg(a, b, fmt...)
//#define HDMI_dbg_mem_print()
//#define HDMI_dbg_set_type(a, b)
//#define HDMI_dbg_set_mask(a, b)
//#define HDMI_process_cmmd(a, b, c)          0
//
//#define HDMI_DBG_INFO(no, fmt...)           0
//#define HDMI_DBG_WARN(no, fmt...)           0
//#define HDMI_DBG_ERRO(no, fmt...)           0
//#define HDMI_DBG_FATAL(no, fmt...)          0
//
//#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

