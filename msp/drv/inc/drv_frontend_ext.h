/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_FRONTEND_EXT_H__
#define __DRV_FRONTEND_EXT_H__

#include "mt_type.h"

#include "mt_drv_dev.h"

typedef mt_s32 (*fe_suspend_fn)(basedev_s *pdev, pm_message_t state);
typedef mt_s32 (*fe_resume_fn)(basedev_s *pdev);

typedef struct
{
    fe_suspend_fn fe_suspend;
    fe_resume_fn fe_resume;
} fe_export_func_s;

typedef struct
{
    struct clk *mclk;
    struct clk *xtalclk;
    struct clk *drvclk;
     atomic_t atmOpenCnt;                 /* Open times */
} fe_priv_data_s;

mt_s32 fe_drv_module_init(mt_void);
mt_void fe_drv_module_exit(mt_void);

#endif
