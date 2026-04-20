/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2020, Montage Technology Co., Ltd.
 *
 * File Name      : fw_api_bridge.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2020/11/04
 * Description    : VFMW API bridge, for FW source code not open.
 * History        :
 * 1.Date         : 2020/11/04
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include "mt_module.h"
#include "mt_drv_module.h"

#include "vfmw.h"

#undef LOG_TAG
#define LOG_TAG							"VFMW_BR"
#include "Log.h"

static VFMW_EXPORT_FUNC_S *g_vfmw_func = NULL;

#define GET_VFMW_FUNC()			do { \
									if (g_vfmw_func == NULL) { \
										(void)mt_drv_module_getfunction(MT_ID_VFMW, (mt_void **)&g_vfmw_func); \
									} \
								} while(0)

#define CHECK_VFMW_FUNC()		do { \
									if (g_vfmw_func == NULL) { \
										MLOGE("%s: VFMW function is null!!!\n",__FUNCTION__); \
										return MT_FAILURE; \
									} \
								} while(0)

SINT32 KERN_VDEC_InitWithOperation(VDEC_OPERATION_S *pArgs)
{
	GET_VFMW_FUNC();
	CHECK_VFMW_FUNC();

	if (pArgs == NULL)
	{
		MLOGE("%s: invalide parameter!\n",__FUNCTION__);
		return MT_FAILURE;
	}

	if (g_vfmw_func->pfnVDEC_InitWithOperation == NULL)
	{
		MLOGE("%s: pfnVDEC_InitWithOperation is null!\n",__FUNCTION__);
		return MT_FAILURE;
	}

	return g_vfmw_func->pfnVDEC_InitWithOperation(pArgs);
}

SINT32 KERN_VDEC_Control(SINT32 ChanID, VDEC_CID_E eCmdID, VOID *pArgs)
{
	GET_VFMW_FUNC();
	CHECK_VFMW_FUNC();
	
	if (g_vfmw_func->pfnVDEC_Control == NULL)
	{
		MLOGE("%s: pfnVDEC_Control is null!\n",__FUNCTION__);
		return MT_FAILURE;
	}

	//TODO: check parameters

	return g_vfmw_func->pfnVDEC_Control(ChanID, eCmdID, pArgs);
}

SINT32 KERN_VDEC_Exit(VOID)
{
	GET_VFMW_FUNC();
	CHECK_VFMW_FUNC();

	if (g_vfmw_func->pfnVDEC_Exit == NULL)
	{
		MLOGE("%s: pfnVDEC_Exit is null!\n",__FUNCTION__);
		return MT_FAILURE;
	}

	return g_vfmw_func->pfnVDEC_Exit();
}

