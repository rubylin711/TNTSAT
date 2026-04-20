/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_dev.h"
#include "drv_dev_priv.h"
#include "vbi_inserter.h"
#include "vbi_inserter_priv.h"
#include "mtos_printk.h"
#include "mt_debug.h"

/*!
   start VBI inserter

   \param[out] param

   \return Return 0 for success and others for failure.
 */
mt_s32 vbi_inserter_dev_start(vbi_inserter_device_t *p_dev, mt_u32 param)
{
    mt_s32 ret = MT_SUCCESS;
    vbi_inserter_t      *p_inserter = (vbi_inserter_t *)(p_dev->p_priv);

    if(drv_check_cond(p_dev) == MT_SUCCESS)
    {
        MT_ASSERT(p_inserter->p_start != NULL);
        ret = p_inserter->p_start(p_inserter->p_priv, param);
    }

    return ret;
}


/*!
   stop VBI inserter

   \param[out] param

   \return Return 0 for success and others for failure.
 */
mt_s32 vbi_inserter_dev_stop(vbi_inserter_device_t *p_dev, mt_u32 param)
{
    mt_s32 ret = MT_SUCCESS;
    vbi_inserter_t      *p_inserter = (vbi_inserter_t *)(p_dev->p_priv);

    if(drv_check_cond(p_dev) == MT_SUCCESS)
    {
        MT_ASSERT(p_inserter->p_stop != NULL);
        ret = p_inserter->p_stop(p_inserter->p_priv, param);
    }

    return ret;
}


/*!
   set video standard

   \param[out] param

   \return Return 0 for success and others for failure.
 */
mt_s32 vbi_inserter_dev_set_vid_std(vbi_inserter_device_t *p_dev
    , video_std_t std)
{
    mt_s32 ret = MT_SUCCESS;
    vbi_inserter_t      *p_inserter = (vbi_inserter_t *)(p_dev->p_priv);

    if(drv_check_cond(p_dev) == MT_SUCCESS)
    {
        MT_ASSERT(p_inserter->p_set_vid_std!= NULL);
        ret = p_inserter->p_set_vid_std(p_inserter->p_priv
            , std);
    }

    return ret;
}


/*!
   inserte data

   \param[out] param

   \return Return 0 for success and others for failure.
 */
mt_s32 vbi_inserter_dev_inserte_data(vbi_inserter_device_t *p_dev
    , mt_u32 pts, mt_u8 *p_pes_data_filed, mt_u8 *p_pes_last_byte)
{
    mt_s32 ret = MT_SUCCESS;
    vbi_inserter_t      *p_inserter = (vbi_inserter_t *)(p_dev->p_priv);

    if(drv_check_cond(p_dev) == MT_SUCCESS)
    {
        MT_ASSERT(p_inserter->p_inserte_data != NULL);
        ret = p_inserter->p_inserte_data(p_inserter->p_priv
            , pts, p_pes_data_filed, p_pes_last_byte);
    }

    return ret;
}

