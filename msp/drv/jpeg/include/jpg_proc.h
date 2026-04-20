/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __JPG_PROC_H__
#define __JPG_PROC_H__


/*********************************add include here******************************/
#include "mt_jpeg_config.h"


#ifdef CONFIG_JPEG_PROC_ENABLE

#include <linux/seq_file.h>
#include "mt_jpeg_hal_api.h"


/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus  
     extern "C" 
{
#endif
#endif /* __cplusplus */


   /***************************** Macro Definition ******************************/

   #define PROC_JPEG_ENTRY_NAME            "jpeg"

    /*************************** Structure Definition ****************************/


    /********************** Global Variable declaration **************************/

	
    /******************************* API declaration *****************************/

    /*****************************************************************************
    * Function     : JPEG_Proc_GetStruct
    * Description  : get the proc struct information
    * param[in]    : ppstProcInfo
    * retval       : NA
    *****************************************************************************/
    MT_VOID JPEG_Proc_GetStruct(MT_JPEG_PROC_INFO_S **ppstProcInfo);


    /*****************************************************************************
    * Function     : JPEG_Proc_init
    * Description  : 
    * param[in]    : NA
    * retval       : NA
    *****************************************************************************/
    MT_VOID JPEG_Proc_init(MT_VOID);

	
    /*****************************************************************************
    * Function     : JPEG_Proc_Cleanup
    * Description  : 
    * param[in]    : NA
    * retval       : NA
    *****************************************************************************/
    MT_VOID JPEG_Proc_Cleanup(MT_VOID);
    

    /*****************************************************************************
    * Function     : JPEG_Proc_IsOpen
    * Description  : 
    * param[in]    : NA
    * retval       : NA
    *****************************************************************************/
    MT_BOOL JPEG_Proc_IsOpen(MT_VOID);


    /*****************************************************************************
    * Function     : JPEG_Get_Proc_Status
    * Description  : 
    * param[in]    : pbProcStatus
    * retval       : NA
    *****************************************************************************/
    MT_VOID JPEG_Get_Proc_Status(MT_BOOL* pbProcStatus);


    /****************************************************************************/



#ifdef __cplusplus
#if __cplusplus 
}
#endif
#endif /* __cplusplus */

#endif /* __JPG_PROC_H__ */


#endif /** use the proc information -DCONFIG_JPEG_PROC_ENABLE **/
