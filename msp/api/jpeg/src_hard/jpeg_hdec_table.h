/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __JPEG_HDEC_TABLE_H__
#define __JPEG_HDEC_TABLE_H__


/*********************************add include here******************************/

#include  "mt_jpeglib.h"
#include  "mt_type.h"


/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C" 
{
#endif
#endif /* __cplusplus */


    /***************************** Macro Definition ******************************/
    /** \addtogroup 	 JPEG TABLE MACRO */
    /** @{ */  /** <!-- ¡¾JPEG TABLE MACRO¡¿ */


	 /** @} */	/*! <!-- Macro Definition end */


	 /*************************** Enum Definition ****************************/

	/** \addtogroup      JPEG TABLE ENUM */
    /** @{ */  /** <!-- ¡¾JPEG TABLE ENUM¡¿ */


	
    /** @} */  /*! <!-- enum Definition end */

	/*************************** Structure Definition ****************************/

	/** \addtogroup      JPEG TABLE STRUCTURE */
    /** @{ */  /** <!-- ¡¾JPEG TABLE STRUCTURE¡¿ */

	/** @} */  /*! <!-- Structure Definition end */

	
    /********************** Global Variable declaration **************************/
 
    /******************************* API declaration *****************************/

	/** \addtogroup      JPEG TABLE API */
    /** @{ */  /** <!-- ¡¾JPEG TABLE API¡¿ */
	


void jcodec_set_tab(j_decompress_ptr p_cinfo, MT_BOOL auto_flag);
void jcodec_set_iq_tab(j_decompress_ptr p_cinfo, MT_BOOL auto_flag);


#ifdef __cplusplus
    
#if __cplusplus
   
}
#endif
#endif /* __cplusplus */

#endif /* __JPEG_HDEC_TABLE_H__*/
