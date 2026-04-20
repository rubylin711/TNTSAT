/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_type.h"
#include "jpg_hal.h"
#include "mt_drv_jpeg_reg.h"
#if (defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)|| defined(CONFIG_MT_CHIP_SYMPHONY6)) //sym6
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif
#include "mt_reg_io.h"
/***************************** Macro Definition ******************************/


/*************************** Structure Definition ****************************/



/********************** Global Variable declaration **************************/

static ulong s_u32JpgRegAddr = 0;

/******************************* API forward declarations *******************/

/******************************* API realization *****************************/
MT_U32 JPGDRV_READ_REG(ulong base,MT_U32 offset)
{
  return HAL_GET_U32((volatile MT_U32 *)((ulong)(base) + (offset)));
}

MT_VOID  JPGDRV_WRITE_REG(ulong base, MT_U32 offset, MT_U32 value)
{
  HAL_PUT_U32((volatile MT_U32 *)((ulong)(base) + (offset)), value);
}


/*****************************************************************************
* func            : JpgHalInit
* description     : initial the jpeg device
* param[in]       : none
* retval          : none
* output          : none
* others:	      : notmtng
*****************************************************************************/
MT_VOID JpgHalInit(ulong u32JpegRegBase)
{
	  s_u32JpgRegAddr = u32JpegRegBase;
}

 /*****************************************************************************
* func            : JpgHalExit
* description     : exit initial the jpeg device
* param[in]       : none
* retval          : none
* output          : none
* others:	      : notmtng
*****************************************************************************/
MT_VOID JpgHalExit(MT_VOID)
{
    s_u32JpgRegAddr = 0;
}

/*****************************************************************************
* func            : JpgHalGetIntStatus
* description     : get halt status
* param[in]       : none
* retval          : none
* output          : pIntStatus  the value of halt state
* others:	      : notmtng
*****************************************************************************/
MT_VOID JpgHalGetIntStatus(MT_U32 *pIntStatus)
{
    /**
     ** read the halt register and write it to *pIntStatus
     **/
    *pIntStatus = JPGDRV_READ_REG(s_u32JpgRegAddr, JCODEC_INT_STATE);
}

/*****************************************************************************
* func            : JpgHalSetIntStatus
* description     : set halt status
* param[in]       : IntStatus    the halt value
* retval          : none
* output          : none
* others:	      : notmtng
*****************************************************************************/
MT_VOID JpgHalSetIntStatus(MT_U32 IntStatus)
{
    /**
     ** read halt register and write it to *pIntStatus
     **/
    JPGDRV_WRITE_REG(s_u32JpgRegAddr, JCODEC_INT_STATE, IntStatus);
}

/*****************************************************************************
* func            : JpgHalSetIntMask
* description     : set halt mask
* param[in]       : IntMask     halt mask
* retval          : none
* output          : none
* others:	      : notmtng
*****************************************************************************/
MT_VOID JpgHalSetIntMask(MT_U32 IntMask)
{
    /** set halt mask with IntMask **/
    JPGDRV_WRITE_REG(s_u32JpgRegAddr, JCODEC_INT_EN, IntMask);
}


/*****************************************************************************
* func            : JpgHalGetIntMask
* description     : get halt mask
* param[in]       : none
* retval          : none
* output          : pIntMask   halt mask
* others:	      : notmtng
*****************************************************************************/
MT_VOID JpgHalGetIntMask(MT_U32 *pIntMask)
{
    /** get halt mask and write it to *pIntMask **/
    *pIntMask = JPGDRV_READ_REG(s_u32JpgRegAddr, JCODEC_INT_EN);
}
