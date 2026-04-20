/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "mt_drv_mem.h"
#include "mt_drv_file.h"
#include "drv_hdmi_debug.h"
#include "drv_global.h"
#include "mt_osal.h"

static ABS_DBG_S Hdmi_dbg_s;
static MT_BOOL bInited = MT_FALSE;
const  MT_CHAR *Hdmi_DbgFuncName[ABS_FUNC_15 + 1] = {
	"<F-001 > ",    /* string length is ABS_DBG_INF_DATA_LEN = 10 */
	"<F-EDID> ",
	"<F-HDCP> ",
	"<F-CEC > ",
	"<F-HPD > ",
	"<F-COM > ",
	"<F-006 > ",
	"<F-007 > ",
	"<F-008 > ",
	"<F-009 > ",
	"<F-010 > ",
	"<F-011 > ",
	"<F-012 > ",
	"<F-013 > ",
	"<F-014 > ",
	"<F-015 > "
};

mt_s32 HDMI_SetDbgLevel(ABS_DBG_LEVEL_T level)
{
	return  ABS_SetDbgLevel(&Hdmi_dbg_s, level);
}

/* print the strings to memory */
mt_void HDMI_Dbg(MT_U8 u8DbgLevel, mt_u32 u32HDMI_FuncNum, const MT_CHAR *fmt, ...)
{
	mt_u32 u32FuncNum;
	MT_U8  u8Buf[ABS_DBG_MEM_IN_BUF_SIZE + 1];
	static unsigned long start = 0, i = 1;
	va_list argp = {0};

	if (i == 1) {
		start = jiffies;
		i = 0;
	}

	if (!Hdmi_dbg_s.pdbg_addr) {
		HDMI_PTK_ERR("HDMI dbg failed, mem addr : 0x%p\n", Hdmi_dbg_s.pdbg_addr);
		return ;
	}

	switch (u32HDMI_FuncNum) {
		case HDMI_DEBUG_PRINT_ALL  :
			u32FuncNum = ABS_FUNC_0;
			break;

		case HDMI_DEBUG_PRINT_EDID  :
			u32FuncNum = ABS_FUNC_1;
			break;

		case HDMI_DEBUG_PRINT_HDCP  :
			u32FuncNum = ABS_FUNC_2;
			break;

		case HDMI_DEBUG_PRINT_CEC   :
			u32FuncNum = ABS_FUNC_3;
			break;

		case HDMI_DEBUG_PRINT_HPD   :
			u32FuncNum = ABS_FUNC_4;
			break;

		case HDMI_DEBUG_PRINT_COM   :
			u32FuncNum = ABS_FUNC_5;
			break;

		default:
			u32FuncNum = ABS_FUNC_0;
			break;
	}

	mt_osal_snprintf(u8Buf, 14, "[%10lu] ", jiffies - start);
	va_start(argp, fmt);
	mt_osal_vsnprintf(u8Buf + 13, ABS_DBG_MEM_IN_BUF_SIZE - 13, fmt, argp);
	va_end(argp);
	//    printk("h(%d) :  %s \n", strlen(u8Buf), u8Buf);

	if (u8DbgLevel >= ABS_DBG_END || u8DbgLevel < ABS_DBG_INFO) {
		u8DbgLevel = ABS_DBG_INFO;
	}

	ABS_PtkToMem(&Hdmi_dbg_s, u8DbgLevel, u32FuncNum, u8Buf);

	return ;
}

/* print the strings in memory to file or serial */
mt_void HDMI_DbgMemPrint(mt_void)
{
	MT_S8 *start = "\n****************************** start ******************************\n\n";
	MT_S8 *end = "\n******************************  end  ******************************\n";

	if (!Hdmi_dbg_s.pdbg_addr) {
		HDMI_PTK_ERR("HDMI mem out failed, mem addr : 0x%p\n", Hdmi_dbg_s.pdbg_addr);
		return ;
	}

	if (Hdmi_dbg_s.pdbg_fp && (Hdmi_dbg_s.dbg_type == DBG_FILE || Hdmi_dbg_s.dbg_type == DBG_ALL)) {
		mt_drv_file_write(Hdmi_dbg_s.pdbg_fp, start, strlen(start));
	}

	ABS_MemOut(&Hdmi_dbg_s, Hdmi_DbgFuncName);

	if (Hdmi_dbg_s.pdbg_fp && (Hdmi_dbg_s.dbg_type == DBG_FILE || Hdmi_dbg_s.dbg_type == DBG_ALL)) {
		mt_drv_file_write(Hdmi_dbg_s.pdbg_fp, end, strlen(end));
	}

	//memset(Hdmi_dbg_s.pdbg_addr, 0, Hdmi_dbg_s.dbg_mem_size);
	//Hdmi_dbg_s.pdbg_pos = Hdmi_dbg_s.pdbg_addr;

	return ;
}

/* set the logtype, file or serial */
mt_void  HDMI_DbgSetType(PRINT_TO_DEV_E logtype, const MT_CHAR *pFilePath)
{
	struct file *pFile = MT_NULL;
	static MT_CHAR file_log[DEF_FILE_NAMELENGTH];

	if (pFilePath) {
		if (mt_osal_strncmp(file_log, pFilePath, strlen(file_log))) {
			if (!(strlen(pFilePath) < DEF_FILE_NAMELENGTH)) {
				HDMI_PTK_ERR("file path is too long\n");
				return ;
			}

			/* O_TRUNC & O_APPEND can't exist at the same time */
			//pFile = mt_drv_file_open(file_log, O_RDWR | O_CREAT | O_TRUNC);
			pFile = filp_open(pFilePath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			if (IS_ERR(pFile)) {
				HDMI_PTK_ERR("open file %s failed!\n", pFilePath);
				pFile = MT_NULL;
				return ;
			}

			if (Hdmi_dbg_s.pdbg_fp) {
				mt_drv_file_close(Hdmi_dbg_s.pdbg_fp);
			}

			mt_osal_strncpy(file_log, pFilePath, strlen(pFilePath) + 1); //don't forget '\0'
		}
	} else {
		if (!Hdmi_dbg_s.pdbg_fp) {
			//pFile = MT_DRV_FILE_Open(HDMI_DBG_FILE_DEF, O_RDWR | O_CREAT | O_TRUNC);
			pFile = filp_open(HDMI_DBG_FILE_DEF, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			if (IS_ERR(pFile)) {
				HDMI_PTK_ERR("open file %s failed!\n", HDMI_DBG_FILE_DEF);
				pFile = MT_NULL;
				return ;
			}

			mt_osal_strncpy(file_log, HDMI_DBG_FILE_DEF, strlen(HDMI_DBG_FILE_DEF) + 1);
		}
	}

	ABS_SetType(&Hdmi_dbg_s, logtype, pFile);

	return ;
}

/* set the function mask */
mt_void  HDMI_DbgSetMask(mt_u32 u32CmdType, mt_u32 LogMask)
{
	mt_u32  mask = Hdmi_dbg_s.dbg_mask;

	switch (u32CmdType) {
		case HDMI_LOGMASK_ADD_FUNC:
			ABS_SetMask(&Hdmi_dbg_s, mask | LogMask);
			break;

		case HDMI_LOGMASK_DEC_FUNC:
			ABS_SetMask(&Hdmi_dbg_s, mask & ~LogMask);
			break;

		case HDMI_LOGMASK_ONLY_FUNC:
			ABS_SetMask(&Hdmi_dbg_s, LogMask);
			break;

		case HDMI_LOGMASK_ALL_FUNC:
			ABS_SetMask(&Hdmi_dbg_s, 0xFFFF);
			break;

		default:
			ABS_SetMask(&Hdmi_dbg_s, 0xFFFF);
			HDMI_PTK_ERR("wrong logmask_set_mask cmd type\n");
			return ;
	}

	return ;
}

mt_s32  HDMI_ProcessCmd(mt_u32 u32CmdType, const MT_CHAR *pCmdParam, const MT_CHAR *pFilePath)
{
	switch (u32CmdType) {
		case HDMI_DEBUG_LOGTYPE: //logtype
			if (!mt_osal_strncmp(pCmdParam, "all", DEF_FILE_NAMELENGTH)) {
				HDMI_DbgSetType(DBG_ALL, pFilePath);
			} else if (!mt_osal_strncmp(pCmdParam, "serial", DEF_FILE_NAMELENGTH)) {
				HDMI_DbgSetType(DBG_SCREEN, NULL);
			} else if (!mt_osal_strncmp(pCmdParam, "file", DEF_FILE_NAMELENGTH)) {
				HDMI_DbgSetType(DBG_FILE, pFilePath);
			} else {
				HDMI_PTK_ERR("wrong logtype param!, use default value.(serial, /root/HDMI_debug.log)\n");
				HDMI_DbgSetType(DBG_ALL, pFilePath);
			}

			break;

		case HDMI_DEBUG_LOGMASK: { //logmask
			mt_u32 u32CmdMask;

			if ('+' == *pCmdParam) {
				u32CmdMask = HDMI_LOGMASK_ADD_FUNC;
			} else if ('-' == *pCmdParam) {
				u32CmdMask = HDMI_LOGMASK_DEC_FUNC;
			} else if ('*' == *pCmdParam) {
				if ( '\0' == *(pCmdParam + 1)) {
					u32CmdMask = HDMI_LOGMASK_ALL_FUNC;
				} else {
					u32CmdMask = HDMI_LOGMASK_ONLY_FUNC;
				}
			} else {
				HDMI_PTK_ERR("wrong logmask param!\n");
				return MT_FAILURE;
			}
			pCmdParam++;

			if (!mt_osal_strncmp(pCmdParam, "ALL", DEF_FILE_NAMELENGTH)) {
				HDMI_DbgSetMask(u32CmdMask, ~HDMI_DEBUG_PRINT_ALL);
			} else if (!mt_osal_strncmp(pCmdParam, "EDID", DEF_FILE_NAMELENGTH)) {
				HDMI_DbgSetMask(u32CmdMask, HDMI_DEBUG_PRINT_EDID);
			} else if (!mt_osal_strncmp(pCmdParam, "HDCP", DEF_FILE_NAMELENGTH)) {
				HDMI_DbgSetMask(u32CmdMask, HDMI_DEBUG_PRINT_HDCP);
			} else if (!mt_osal_strncmp(pCmdParam, "CEC", DEF_FILE_NAMELENGTH)) {
				HDMI_DbgSetMask(u32CmdMask, HDMI_DEBUG_PRINT_CEC);
			} else if (!mt_osal_strncmp(pCmdParam, "HPD", DEF_FILE_NAMELENGTH)) {
				HDMI_DbgSetMask(u32CmdMask, HDMI_DEBUG_PRINT_HPD);
			} else {
				HDMI_DbgSetMask(u32CmdMask, ~HDMI_DEBUG_PRINT_ALL);
				HDMI_PTK_ERR("wrong logmask param!, use default value\n");
				return MT_FAILURE;
			}

			break;
		}

		case HDMI_DEBUG_LOGCATCH: //logcatch
			if (!mt_osal_strncmp(pCmdParam, "info", DEF_FILE_NAMELENGTH)) {
				ABS_SetDbgLevel(&Hdmi_dbg_s, ABS_DBG_INFO);
			} else if (!mt_osal_strncmp(pCmdParam, "warning", DEF_FILE_NAMELENGTH)) {
				ABS_SetDbgLevel(&Hdmi_dbg_s, ABS_DBG_WARN);
			} else if (!mt_osal_strncmp(pCmdParam, "error", DEF_FILE_NAMELENGTH)) {
				ABS_SetDbgLevel(&Hdmi_dbg_s, ABS_DBG_ERROR);
			} else if (!mt_osal_strncmp(pCmdParam, "fatal", DEF_FILE_NAMELENGTH)) {
				ABS_SetDbgLevel(&Hdmi_dbg_s, ABS_DBG_ERROR);
			} else if (!mt_osal_strncmp(pCmdParam, "clear", DEF_FILE_NAMELENGTH)) {
				ABS_ClearMem(&Hdmi_dbg_s);
				HDMI_PTK_INFO("clear debug memory.\n");
				break;
			} else {
				ABS_SetDbgLevel(&Hdmi_dbg_s, ABS_DBG_INFO);
			}

			HDMI_DbgMemPrint();
			break;

		default:
			HDMI_PTK_ERR("unkonw cmd param \n");
			return MT_FAILURE;
	}

	return MT_SUCCESS;
}

mt_s32 HDMI_DbgInit(mt_void)
{
	mt_s32 ret;

	if (MT_TRUE == bInited) {
		return MT_SUCCESS;
	}

	ret = ABS_DbgInit(&Hdmi_dbg_s, MT_ID_HDMI);
	HDMI_PTK_ERR("HDMI Inited addr:%px, size:0x%x\n", Hdmi_dbg_s.pdbg_addr, Hdmi_dbg_s.dbg_mem_size);
	if (MT_FAILURE ==  ret) {
		HDMI_PTK_ERR("HDMI Init failed!\n");
		return MT_FAILURE;
	}

	HDMI_DbgSetType(HDMI_DEBUG_PRINT_ALL, MT_NULL);
	HDMI_DbgSetMask(HDMI_LOGMASK_ALL_FUNC, 0);
	bInited = MT_TRUE;

	return MT_SUCCESS;
}

mt_void HDMI_DbgDeInit(mt_void)
{
	if (MT_FALSE == bInited) {
		return ;
	}

	if (Hdmi_dbg_s.pdbg_fp) {
		mt_drv_file_close(Hdmi_dbg_s.pdbg_fp);
	}

	ABS_DbgDeInit(&Hdmi_dbg_s);
	bInited = MT_FALSE;

	return ;
}

