/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef  _JPG_HAL_H_
#define  _JPG_HAL_H_


/*********************************add include here******************************/
#include "mt_type.h"

/*****************************************************************************/


/*****************************************************************************/


#ifdef __cplusplus
      #if __cplusplus
   
extern "C" 
{

      #endif
#endif /* __cplusplus */

    /***************************** Macro Definition ******************************/


    /*************************** Structure Definition ****************************/

    /***************************  The enum of Jpeg image format  ******************/

    /********************** Global Variable declaration **************************/


    /******************************* API declaration *****************************/

	
	
	/*****************************************************************************
	* func			  : JpgHalInit
	* description	  : initial the jpeg device
	* param[in] 	  : none
	* retval		  : none
	* output		  : none
	* others:		  : notmtng
	*****************************************************************************/
	MT_VOID JpgHalInit(ulong u32JpegRegBase);
	
	
	 /*****************************************************************************
	* func			  : JpgHalExit
	* description	  : exit initial the jpeg device
	* param[in] 	  : none
	* retval		  : none
	* output		  : none
	* others:		  : notmtng
	*****************************************************************************/
	
	MT_VOID JpgHalExit(MT_VOID);
	
	/*****************************************************************************
	* func			  : JpgHalGetIntStatus
	* description	  : get halt status
	* param[in] 	  : none
	* retval		  : none
	* output		  : pIntStatus	the value of halt state
	* others:		  : notmtng
	*****************************************************************************/
	
	MT_VOID JpgHalGetIntStatus(MT_U32 *pIntStatus);
	
	
	
	/*****************************************************************************
	* func			  : JpgHalSetIntMask
	* description	  : set halt mask
	* param[in] 	  : IntMask 	halt mask
	* retval		  : none
	* output		  : none
	* others:		  : notmtng
	*****************************************************************************/
	
	MT_VOID JpgHalSetIntMask(MT_U32 IntMask);
	
	
	/*****************************************************************************
	* func			  : JpgHalGetIntMask
	* description	  : get halt mask
	* param[in] 	  : none
	* retval		  : none
	* output		  : pIntMask   halt mask
	* others:		  : notmtng
	*****************************************************************************/
	
	MT_VOID JpgHalGetIntMask(MT_U32 *pIntMask);
	
	 
	/*****************************************************************************
	* func			  : JpgHalSetIntStatus
	* description	  : set halt status
	* param[in] 	  : IntStatus	 the halt value
	* retval		  : none
	* output		  : none
	* others:		  : notmtng
	*****************************************************************************/
	
	MT_VOID JpgHalSetIntStatus(MT_U32 IntStatus);

    #ifdef __cplusplus

        #if __cplusplus



}
      
        #endif
        
   #endif /* __cplusplus */

#endif /*_JPG_HAL_H_ */
