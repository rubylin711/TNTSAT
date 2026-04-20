/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_display.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      :
    Modification:

*********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "mt_mpi_disp.h"

#include "mt_mpi_hdmi.h"

#include "mpi_disp_tran.h"
#include "mt_mpi_pq.h"
#include "mt_module_debug.h"
#include "mt_sdvenc_macv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static const mt_u8 s_szDISPVersion[] __attribute__((used)) = "SDK_VERSION:[" MKMARCOTOSTR(SDK_VERSION) "] Build Time:[" __DATE__ ", " __TIME__ "]";
ulong p_sdenc_macv_addr;
extern mt_s32 mpi_memdev_unmap_register(mt_void *pVirAddr);
extern mt_s32 mpi_memdev_map_register(phys_addr_t RegAddr, mt_u32 u32Length, mt_void **pVirAddr);


mt_s32 DISP_CheckIntf(MT_UNF_DISP_INTF_S *pstIntf, mt_u32 u32IntfNum);

mt_s32 MT_UNF_DISP_Init(mt_void)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_DISP_Init();

    s32Ret |= MT_MPI_PQ_Init(MT_NULL);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_DeInit(mt_void)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_DISP_DeInit();

    s32Ret |= MT_MPI_PQ_DeInit();

    return s32Ret;
}

mt_s32 MT_UNF_DISP_Attach(MT_UNF_DISP_E enDstDisp, MT_UNF_DISP_E enSrcDisp)
{
    MT_DRV_DISPLAY_E enMaster, enSlave;
    mt_s32 s32Ret;

    Transfer_DispID(&enDstDisp, &enSlave, MT_TRUE);
    Transfer_DispID(&enSrcDisp, &enMaster, MT_TRUE);
    s32Ret = MT_MPI_DISP_Attach(enMaster, enSlave);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_Detach(MT_UNF_DISP_E enDstDisp, MT_UNF_DISP_E enSrcDisp)
{
    MT_DRV_DISPLAY_E enMaster, enSlave;
    mt_s32 s32Ret;

    Transfer_DispID(&enDstDisp, &enSlave, MT_TRUE);
    Transfer_DispID(&enSrcDisp, &enMaster, MT_TRUE);

    s32Ret = MT_MPI_DISP_Detach(enMaster, enSlave);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_Open(MT_UNF_DISP_E enDisp)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    if (enDisp >= MT_UNF_DISPLAY2)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    s32Ret = MT_MPI_DISP_Open(enD);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_Close(MT_UNF_DISP_E enDisp)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_Close(enD);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetSdEncPqParams(MT_UNF_DISP_E enDisp, MT_UNF_SD_ENC_PQ_PARA_S *pstPara, mt_u32 cnt)
{
    MT_DRV_DISPLAY_E enD;
    DISP_SD_ENC_PQ_PARA_S *pSdEncPqParas = NULL;
    mt_s32 s32Ret;
    mt_u32 i;
   
    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    Transfer_SdEncPqPara(&pstPara, &pSdEncPqParas, MT_TRUE);
    for(i = 0; i < cnt; i++)
    {
        s32Ret = MT_MPI_DISP_SetSdEncPqParam(enD, &pSdEncPqParas[i]);
        if(s32Ret)
        {
            MT_ERR_DISP("SetSdEncPqParams error! Item:%x Val:%x\n", __FUNCTION__, __LINE__, pSdEncPqParas[i].item, pSdEncPqParas[i].val);
        }
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_DISP_SetFormat(MT_UNF_DISP_E enDisp, MT_UNF_ENC_FMT_E enEncodingFormat)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_FMT_E enF = MT_DRV_DISP_FMT_BUTT;
    mt_s32 s32Ret;

    if ((enEncodingFormat >= MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING)
         && (enEncodingFormat <= MT_UNF_ENC_FMT_720P_50_FRAME_PACKING))
    {
        MT_ERR_DISP("para enEncodingFormat is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    s32Ret = Transfer_EncFmt(&enEncodingFormat, &enF, MT_TRUE);

    if(s32Ret)
        return s32Ret;

    s32Ret = MT_MPI_DISP_SetFormat(enD, MT_DRV_DISP_STEREO_NONE, enF);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetFormat(MT_UNF_DISP_E enDisp, MT_UNF_ENC_FMT_E *penEncodingFormat)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_FMT_E enF;
    mt_s32 s32Ret;
    MT_DRV_DISP_STEREO_MODE_E enDrv3D;

    if (!penEncodingFormat)
    {
        MT_ERR_DISP("para penEncodingFormat is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_GetFormat(enD, &enDrv3D, &enF);

    if (!s32Ret)
    {
        Transfer_EncFmt(penEncodingFormat, &enF, MT_FALSE);
    }

    return s32Ret;
}

#define MT_DISP_ASPECT_RATIO_MAX_WIDTH 0x0FFFFFFFUL
#define MT_DISP_ASPECT_RATIO_MAX_HEIGHT 0x0FFFFFFFUL

mt_s32 MT_UNF_DISP_SetAspectRatio(MT_UNF_DISP_E enDisp, MT_UNF_DISP_ASPECT_RATIO_S *pstDispAspectRatio)
{
    MT_DRV_DISPLAY_E enD;
    mt_u32 h, v;
    mt_s32 s32Ret;

    if (!pstDispAspectRatio)
    {
        MT_ERR_DISP("para aspect ratio is invalid.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    if (MT_UNF_DISP_ASPECT_RATIO_BUTT <= pstDispAspectRatio->enDispAspectRatio)
    {
        MT_ERR_DISP("para aspect ratio is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (pstDispAspectRatio->enDispAspectRatio == MT_UNF_DISP_ASPECT_RATIO_USER)
    {
        if ((pstDispAspectRatio->u32UserAspectWidth > MT_DISP_ASPECT_RATIO_MAX_WIDTH)
             || (pstDispAspectRatio->u32UserAspectHeight > MT_DISP_ASPECT_RATIO_MAX_HEIGHT)
             || (pstDispAspectRatio->u32UserAspectWidth >= (pstDispAspectRatio->u32UserAspectHeight * 16))
             || (pstDispAspectRatio->u32UserAspectHeight >= (pstDispAspectRatio->u32UserAspectWidth * 16))
           )
        {
            MT_ERR_DISP("para aspect ratio is invalid.\n");
            return MT_ERR_DISP_INVALID_PARA;
        }
    }
    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    Transfer_AspectRatio(pstDispAspectRatio, &h, &v, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetAspectRatio(enD, h, v);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetAspectRatio(MT_UNF_DISP_E enDisp, MT_UNF_DISP_ASPECT_RATIO_S *pstDispAspectRatio)
{
    MT_DRV_DISPLAY_E enD;
    mt_u32 h, v;
    mt_s32 s32Ret;

    if (!pstDispAspectRatio)
    {
        MT_ERR_DISP("para aspect ratio is invalid.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_GetAspectRatio(enD, &h, &v);
    if (!s32Ret)
    {
        Transfer_AspectRatio(pstDispAspectRatio, &h, &v, MT_FALSE);
    }

    return s32Ret;
}

mt_s32 DISP_CheckIntf(MT_UNF_DISP_INTF_S *pstIntf, mt_u32 u32IntfNum)
{
    mt_u32 u;

    if (!pstIntf)
    {
        MT_ERR_DISP("para pstIntf is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    /* check parameters */
    for (u = 0; u < u32IntfNum; u++)
    {
        if (pstIntf[u].enIntfType >= MT_UNF_DISP_INTF_TYPE_BUTT)
        {
            MT_ERR_DISP("Invalid interface type!\n");
            return MT_ERR_VO_INVALID_PARA;
        }

        if (MT_UNF_DISP_INTF_TYPE_YPBPR == pstIntf[u].enIntfType)
        {
            if ((pstIntf[u].unIntf.stYPbPr.u8DacY >= MAX_DAC_NUM)
                || (pstIntf[u].unIntf.stYPbPr.u8DacPb >= MAX_DAC_NUM)
                || (pstIntf[u].unIntf.stYPbPr.u8DacPr >= MAX_DAC_NUM)
                )
            {
                MT_ERR_DISP("Invalid YPBPR vdac number!\n");
                return MT_ERR_VO_INVALID_PARA;
            }
        }

        if (MT_UNF_DISP_INTF_TYPE_RGB == pstIntf[u].enIntfType)
        {
            if ((pstIntf[u].unIntf.stRGB.u8DacR >= MAX_DAC_NUM)
                || (pstIntf[u].unIntf.stRGB.u8DacG >= MAX_DAC_NUM)
                || (pstIntf[u].unIntf.stRGB.u8DacB >= MAX_DAC_NUM)
                )
            {
                MT_ERR_DISP("Invalid RGB vdac number!\n");
                return MT_ERR_VO_INVALID_PARA;
            }
        }
        if (MT_UNF_DISP_INTF_TYPE_VGA == pstIntf[u].enIntfType)
        {
            if ((pstIntf[u].unIntf.stVGA.u8DacR >= MAX_DAC_NUM)
                || (pstIntf[u].unIntf.stVGA.u8DacG >= MAX_DAC_NUM)
                || (pstIntf[u].unIntf.stVGA.u8DacB >= MAX_DAC_NUM)
               )
            {
                MT_ERR_DISP("Invalid VGA vdac number!\n");
                return MT_ERR_VO_INVALID_PARA;
            }
        }

        if (MT_UNF_DISP_INTF_TYPE_CVBS == pstIntf[u].enIntfType)
        {
            if (pstIntf[u].unIntf.stCVBS.u8Dac >= MAX_DAC_NUM)
            {
                MT_ERR_DISP("Invalid CVBS vdac number!\n");
                return MT_ERR_VO_INVALID_PARA;
            }
        }

        if (MT_UNF_DISP_INTF_TYPE_SVIDEO == pstIntf[u].enIntfType)
        {
            if ((pstIntf[u].unIntf.stSVideo.u8DacY >= MAX_DAC_NUM)
                 || (pstIntf[u].unIntf.stSVideo.u8DacC >= MAX_DAC_NUM)
                )
            {
                MT_ERR_DISP("Invalid SVIDEO vdac number!\n");
                return MT_ERR_VO_INVALID_PARA;
            }
        }
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_DISP_AttachIntf(MT_UNF_DISP_E enDisp, MT_UNF_DISP_INTF_S *pstIntf, mt_u32 u32IntfNum)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_INTF_S stDrvIntf;
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u32 u;

    if (pstIntf == MT_NULL)
    {
        MT_ERR_DISP("para pstIntf is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    if (enDisp >= MT_UNF_DISPLAY_BUTT)
    {
        MT_ERR_DISP("Invalid interface parameters!\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    /* check parameters */
    s32Ret = DISP_CheckIntf(pstIntf, u32IntfNum);
    if (s32Ret)
    {
        return s32Ret;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    for (u = 0; u < u32IntfNum; u++)
    {
        //stDrvIntf
        s32Ret = Transfer_Intf(&pstIntf[u], &stDrvIntf, MT_TRUE);
        if (s32Ret)
        {
            MT_ERR_DISP("Transfer_Intf interface %d failed!\n", (mt_s32)pstIntf[u].enIntfType);
            break;
        }

        //printf("unf add intf Y=%d, pb=%d, pr=%d\n",  stDrvIntf.u8VDAC_Y_G, stDrvIntf.u8VDAC_Pb_B, stDrvIntf.u8VDAC_Pr_R);
        s32Ret = MT_MPI_DISP_AddIntf(enD, &stDrvIntf);
        if (s32Ret)
        {
            MT_ERR_DISP("Attach interface %d failed!\n", (mt_s32)pstIntf[u].enIntfType);
            break;
        }
    }

    if (s32Ret)
    {
        for (; u > 0; u--)
        {
            // delete inft from [u-1]
            s32Ret = Transfer_Intf(&pstIntf[u - 1], &stDrvIntf, MT_TRUE);
            s32Ret |= MT_MPI_DISP_DelIntf(enD, &stDrvIntf);
            if (s32Ret != MT_SUCCESS)
            {
                continue;
            }
        }
    }

    return s32Ret;
}

mt_s32 MT_UNF_DISP_DetachIntf(MT_UNF_DISP_E enDisp, MT_UNF_DISP_INTF_S *pstIntf, mt_u32 u32IntfNum)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_INTF_S stDrvIntf;
    mt_u32 u;
    mt_s32 s32Ret = MT_SUCCESS;

    if (!pstIntf)
    {
        MT_ERR_DISP("para pstIntf is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    /* check parameters */
    s32Ret = DISP_CheckIntf(pstIntf, u32IntfNum);
    if (s32Ret)
    {
        return s32Ret;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    for (u = 0; u < u32IntfNum; u++)
    {
        //stDrvIntf
        s32Ret = Transfer_Intf(&pstIntf[u], &stDrvIntf, MT_TRUE);
        if (s32Ret)
        {
            MT_ERR_DISP("Transfer_Intf interface %d failed!\n", (mt_s32)pstIntf[u].enIntfType);
            break;
        }

        //printf("unf add intf Y=%d, pb=%d, pr=%d\n",  stDrvIntf.u8VDAC_Y_G, stDrvIntf.u8VDAC_Pb_B, stDrvIntf.u8VDAC_Pr_R);
        s32Ret = MT_MPI_DISP_DelIntf(enD, &stDrvIntf);
        if (s32Ret)
        {
            MT_ERR_DISP("Attach interface %d failed!\n", (mt_s32)pstIntf[u].enIntfType);
            break;
        }
    }

    return s32Ret;
}
#if 0
mt_s32 MT_UNF_DISP_GetIntf(MT_UNF_DISP_E enDisp, mt_u32* pu32IntfNum,  MT_UNF_DISP_INTF_S* pstIntf)
{
    mt_s32 s32Ret;
    s32Ret = MT_MPI_DISP_GetIntf(enDisp, pu32IntfNum,  pstIntf);
    return s32Ret;
}
#endif

mt_s32 MT_UNF_DISP_SetCustomTiming(MT_UNF_DISP_E enDisp, MT_UNF_DISP_TIMING_S *pstTiming)
{
    MT_DRV_DISP_TIMING_S stTiming;
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    if (!pstTiming)
    {
        MT_ERR_DISP("para pstTiming is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    memset(&stTiming, 0, sizeof(MT_DRV_DISP_TIMING_S));
    Transfer_Timing(pstTiming, &stTiming, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetTiming(enD, &stTiming);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetCustomTiming(MT_UNF_DISP_E enDisp, MT_UNF_DISP_TIMING_S *pstTiming)
{
    MT_DRV_DISP_TIMING_S stTiming;
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    if (!pstTiming)
    {
        MT_ERR_DISP("para pstTiming is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_GetTiming(enD, &stTiming);
    if (MT_SUCCESS == s32Ret)
    {
        Transfer_Timing(pstTiming, &stTiming, MT_FALSE);
    }

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetLayerZorder(MT_UNF_DISP_E enDisp, MT_LAYER_ZORDER_ABS_E enZFlag)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_ZORDER_ABS_E enZ;
    mt_s32 s32Ret;


    if (enZFlag  >= MT_LAYER_ZORDER_ABS_BUTT)
    {
        MT_ERR_DISP("para  is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    Transfe_ZOrder(&enZFlag, &enZ, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetLayerZorder(enD, enZ);
    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetLayerZorder(MT_UNF_DISP_E enDisp,  mt_u32 *pu32Zorder)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    if (!pu32Zorder)
    {
        MT_ERR_DISP("para pu32Zorder is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    s32Ret = MT_MPI_DISP_GetLayerZorder(enD, pu32Zorder);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetBgColor(MT_UNF_DISP_E enDisp, const MT_UNF_DISP_BG_COLOR_S *pstBgColor)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_COLOR_S stColor;
    mt_s32 s32Ret;

    if (!pstBgColor)
    {
        MT_ERR_DISP("para pstBgColor is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    Transfer_BGColor((MT_UNF_DISP_BG_COLOR_S *)pstBgColor, &stColor, MT_TRUE);
    s32Ret = MT_MPI_DISP_SetBGColor(enD, &stColor);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetBgColor(MT_UNF_DISP_E enDisp, MT_UNF_DISP_BG_COLOR_S *pstBgColor)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_COLOR_S stColor;
    mt_s32 s32Ret;

    if (!pstBgColor)
    {
        MT_ERR_DISP("para pstBgColor is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_GetBGColor(enD, &stColor);
    if (!s32Ret)
    {
    Transfer_BGColor(pstBgColor, &stColor, MT_FALSE);
    }

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetColorbar(MT_UNF_DISP_E enDisp, MT_BOOL bEnable)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetColorbar(enD, bEnable);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetOutputEnable(MT_UNF_DISP_E enDisp, MT_BOOL bEnable)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetOutputEnable(enD, bEnable);

    return s32Ret;

}

mt_s32 MT_UNF_DISP_SetBrightness(MT_UNF_DISP_E enDisp, mt_u32 u32Brightness)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    if (u32Brightness > 100)
    {
        MT_ERR_DISP("para u32Brightness is %d invalid.\n", u32Brightness);
        return MT_ERR_DISP_INVALID_PARA;
    }

    s32Ret = MT_MPI_DISP_SetBrightness(enD, u32Brightness);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetBrightness(MT_UNF_DISP_E enDisp, mt_u32 *pu32Brightness)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    if (!pu32Brightness)
    {
        MT_ERR_DISP("para pu32Brightness is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_GetBrightness(enD, pu32Brightness);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetContrast(MT_UNF_DISP_E enDisp, mt_u32 u32Contrast)
{
    MT_DRV_DISPLAY_E enD;

    mt_s32 s32Ret;

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    if (u32Contrast > 100)
    {
        MT_ERR_DISP("para u32Contrast is %d invalid.\n", u32Contrast);
        return MT_ERR_DISP_INVALID_PARA;
    }

    s32Ret = MT_MPI_DISP_SetContrast(enD, u32Contrast);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetContrast(MT_UNF_DISP_E enDisp, mt_u32 *pu32Contrast)
{
    MT_DRV_DISPLAY_E enD;

    mt_s32 s32Ret;

    if (!pu32Contrast)
    {
        MT_ERR_DISP("para pu32Contrast is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_GetContrast(enD, pu32Contrast);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetSaturation(MT_UNF_DISP_E enDisp, mt_u32 u32Saturation)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    if (u32Saturation > 100)
    {
        MT_ERR_DISP("para u32Saturation is %d invalid.\n", u32Saturation);
        return MT_ERR_DISP_INVALID_PARA;
    }

    s32Ret = MT_MPI_DISP_SetSaturation(enD, u32Saturation);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetSaturation(MT_UNF_DISP_E enDisp, mt_u32 *pu32Saturation)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    if (!pu32Saturation)
    {
        MT_ERR_DISP("para pu32Saturation is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_GetSaturation(enD, pu32Saturation);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetHuePlus(MT_UNF_DISP_E enDisp, mt_u32 u32HuePlus)
{
#if 0
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    if (u32HuePlus > 100)
    {
        MT_ERR_DISP("para u32HuePlus is %d invalid.\n", u32HuePlus);
        return MT_ERR_DISP_INVALID_PARA;
    }

    s32Ret = MT_MPI_PQ_SetHue(enD, u32HuePlus);

    return s32Ret;
#endif 
    /* need float point support */
    return MT_FAILURE;  
}

mt_s32 MT_UNF_DISP_GetHuePlus(MT_UNF_DISP_E enDisp, mt_u32 *pu32HuePlus)
{
# if 0
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    if (!pu32HuePlus)
    {
        MT_ERR_DISP("para pu32HuePlus is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_PQ_GetHue(enD, pu32HuePlus);

    return s32Ret;
#endif 
    /* need float point support */
    return MT_FAILURE;
}

mt_s32 MT_UNF_DISP_SetAlgCfg(MT_UNF_DISP_E enDisp, MT_UNF_DISP_ALG_CFG_S *pstAlg)
{
    return MT_ERR_DISP_NOT_SUPPORT;
}

mt_s32 MT_UNF_DISP_GetAlgCfg(MT_UNF_DISP_E enDisp, MT_UNF_DISP_ALG_CFG_S *pstAlg)
{
    return MT_ERR_DISP_NOT_SUPPORT;
}

mt_s32 MT_UNF_DISP_CreateVBI(MT_UNF_DISP_E enDisp, MT_UNF_DISP_VBI_CFG_S *pstCfg, mt_handle *phVbi)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_VBI_CFG_S stVbiCfg;

    mt_s32 s32Ret = MT_SUCCESS;
    if (!pstCfg)
    {
        MT_ERR_DISP("para pstCfg is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }
    if (!phVbi)
    {
        MT_ERR_DISP("para phVbi is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    Transfer_VbiCfg(pstCfg, &stVbiCfg, MT_TRUE);

    s32Ret = MT_MPI_DISP_CreateVBI(enD, &stVbiCfg, phVbi);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_DestroyVBI(mt_handle hVbi)
{
    mt_s32 s32Ret = MT_SUCCESS;

    s32Ret = MT_MPI_DISP_DestroyVBI(hVbi);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SendVBIData(mt_handle hVbi, MT_UNF_DISP_VBI_DATA_S *pstVbiData)
{
    mt_s32 s32Ret = MT_SUCCESS;
    MT_DRV_DISP_VBI_DATA_S stVbiData;

    if (!pstVbiData)
    {
        MT_ERR_DISP("para pstVbiData is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_VbiData(pstVbiData, &stVbiData, MT_TRUE);

    s32Ret = MT_MPI_DISP_SendVBIData(hVbi, &stVbiData);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetWss(MT_UNF_DISP_E enDisp, const MT_UNF_DISP_WSS_DATA_S *pstWssData)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_WSS_DATA_S stWssData = {0};
    mt_s32 s32Ret = MT_SUCCESS;

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    if (!pstWssData)
    {
        MT_ERR_DISP("para pstWssData is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

	(void)Transfer_Wss(pstWssData, &stWssData, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetWss(enDisp, &stWssData);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetMacrovision(MT_UNF_DISP_E enDisp, MT_UNF_DISP_MACROVISION_MODE_E enMode, const mt_void *pData)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret = MT_SUCCESS;

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetMacrovision(enD, (MT_DRV_DISP_MACROVISION_E)enMode);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetMacrovision(MT_UNF_DISP_E enDisp, MT_UNF_DISP_MACROVISION_MODE_E *penMode, const mt_void *pData)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret = MT_SUCCESS;
	MT_DRV_DISP_MACROVISION_E enMode;

    if (!penMode)
    {
        MT_ERR_DISP("para penMode is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    enMode = (MT_DRV_DISP_MACROVISION_E)*penMode;
    s32Ret = MT_MPI_DISP_GetMacrovision(enDisp, &enMode);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetMacrovisionRegN(MT_UNF_DISP_E enDisp, mt_u32 index, mt_u32 value)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u32 dtmp = 0;
    mt_u32 sd_fmt = 0;

    s32Ret = mpi_memdev_map_register(SYMPHONY_IO_PA(0xbf450000), 0xB0, (mt_void*)&p_sdenc_macv_addr);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_DISP("sdenc_macv_add map failed!\n");
        return s32Ret;
    }
    mt_sys_read_register(SYMPHONY_IO_PA(0xbf460034), &dtmp);
    sd_fmt = dtmp & 0x7;
	//0/1/7: PAL/; 3: SECAM; 5: PAL_N
    //2: NTSC_J; 4: PAL_M; 6:NTSC_443
	if((sd_fmt == 0) || (sd_fmt == 1) || (sd_fmt == 7) || (sd_fmt == 3) || (sd_fmt == 5))  //PAL
	{
		switch (index)
		{
		case 0:
			reg_symphony_sd_encoder_set_macv_cfg_ps_en_else((value & 0x20) >> 5);
			reg_symphony_sd_encoder_set_macv_cfg_agc_en_else((value & 0x20) >> 5);
			reg_symphony_sd_encoder_set_macv_cfg_bp_en_else((value & 0x10) >> 4);
			reg_symphony_sd_encoder_set_macv_cfg_cs_en_p((value & 0x8) >> 3);
			reg_symphony_sd_encoder_set_macv_cfg_agc_amp_chs((value & 0x4) >> 2);
			reg_symphony_sd_encoder_set_macv_cfg_sync_reduce((value & 0x3));
			MT_SDVENC_MACV_PRINT("N0 = 0x%x\n",     ((( reg_symphony_sd_encoder_get_macv_cfg_ps_en_else() && reg_symphony_sd_encoder_get_macv_cfg_agc_en_else() )? 1:0) << 5) |
					(reg_symphony_sd_encoder_get_macv_cfg_bp_en_else() << 4) |
					(reg_symphony_sd_encoder_get_macv_cfg_cs_en_p()  << 3 ) |
					(reg_symphony_sd_encoder_get_macv_cfg_agc_amp_chs() << 2) |
					reg_symphony_sd_encoder_get_macv_cfg_sync_reduce());
			break;
		case 1:
			reg_symphony_sd_encoder_set_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_p(value);
			MT_SDVENC_MACV_PRINT("N1 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_p());
			break;
		case 2:
			reg_symphony_sd_encoder_set_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_p(value);
			MT_SDVENC_MACV_PRINT("N2 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_p());
			break;
		case 3:
			reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_p(value);
			MT_SDVENC_MACV_PRINT("N3 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_p());
			break;
		case 4:
			reg_symphony_sd_encoder_set_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_p(value);
			MT_SDVENC_MACV_PRINT("N4 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_p());
			break;
		case 5:
			reg_symphony_sd_encoder_set_macv_n5_cs_spc_else_n5_cs_spc_else_p(value);
			MT_SDVENC_MACV_PRINT("N5 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_n5_cs_spc_else_p());
			break;
		case 6:
			reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field_n6_cs_num_in_field_p(value);
			MT_SDVENC_MACV_PRINT("N6 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_n6_cs_num_in_field_p());
			break;
		case 7:
			reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_p(value);
			MT_SDVENC_MACV_PRINT("N7 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_p());
			break;
		case 8:
			reg_symphony_sd_encoder_set_macv_n8_ps_dura_n8_ps_dura_p(value);
			MT_SDVENC_MACV_PRINT("N8 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n8_ps_dura_n8_ps_dura_p());
			break;
		case 9:
			reg_symphony_sd_encoder_set_macv_n9_fst_ps_start_n9_fst_ps_start_p(value);
			MT_SDVENC_MACV_PRINT("N9 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n9_fst_ps_start_n9_fst_ps_start_p());
			break;
		case 10:
			reg_symphony_sd_encoder_set_macv_n10_ps_spc_n10_ps_spc_p(value);
			MT_SDVENC_MACV_PRINT("N10 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n10_ps_spc_n10_ps_spc_p());
			break;
		case 11:
			reg_symphony_sd_encoder_set_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_p(value);
			MT_SDVENC_MACV_PRINT("N11 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_p());
			break;
		case 12:
			reg_symphony_sd_encoder_set_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_p(value);
			MT_SDVENC_MACV_PRINT("N12 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_p());
			break;
		case 13:
			reg_symphony_sd_encoder_set_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_p(value);
			MT_SDVENC_MACV_PRINT("N13 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_p());
			break;
		case 14:
			reg_symphony_sd_encoder_set_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_p(value);
			MT_SDVENC_MACV_PRINT("N14 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_p());
			break;
		case 15:
			reg_symphony_sd_encoder_set_macv_n15_bp_ln_chs_n15_bp_ln_chs_p(value);
			MT_SDVENC_MACV_PRINT("N15 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n15_bp_ln_chs_n15_bp_ln_chs_p());
			break;
		case 16:
			reg_symphony_sd_encoder_set_macv_cfg_n16_advanced_start_p(value);
			MT_SDVENC_MACV_PRINT("N16 = 0x%x\n", reg_symphony_sd_encoder_get_macv_cfg_n16_advanced_start_p());
			break;
		case 17:
			reg_symphony_sd_encoder_set_macv_n17_cs_zone1_n17_cs_zone1_p(value);
			MT_SDVENC_MACV_PRINT("N17 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n17_cs_zone1_n17_cs_zone1_p());
			break;
		case 18:
			reg_symphony_sd_encoder_set_macv_n18_cs_zone2_n18_cs_zone2_p(value);
			MT_SDVENC_MACV_PRINT("N18 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n18_cs_zone2_n18_cs_zone2_p());
			break;
		case 19:
			reg_symphony_sd_encoder_set_macv_n19_cs_zone3_n19_cs_zone3_p(value);
			MT_SDVENC_MACV_PRINT("N19 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n19_cs_zone3_n19_cs_zone3_p());
			break;
		case 20:
			reg_symphony_sd_encoder_set_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_p(value);
			MT_SDVENC_MACV_PRINT("N20 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_p());
			break;
		case 21:            
			reg_symphony_sd_encoder_set_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_p(value);
			MT_SDVENC_MACV_PRINT("N21 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_p());
			break;
		default:
			MT_SDVENC_MACV_PRINT("the register is not exist\n");
			break;

		}

	}
    else  //NTSC
    {    
		switch(index)
		{
		case 0:
			reg_symphony_sd_encoder_set_macv_cfg_ps_en_else((value & 0x20) >> 5);
			reg_symphony_sd_encoder_set_macv_cfg_agc_en_else((value & 0x20) >> 5);
			reg_symphony_sd_encoder_set_macv_cfg_bp_en_else((value & 0x10) >> 4);
			reg_symphony_sd_encoder_set_macv_cfg_cs_en_n((value & 0x8) >> 3);
			reg_symphony_sd_encoder_set_macv_cfg_agc_amp_chs((value & 0x4) >> 2);
			reg_symphony_sd_encoder_set_macv_cfg_sync_reduce((value & 0x3));
			MT_SDVENC_MACV_PRINT("N0 = 0x%x\n",     ((( reg_symphony_sd_encoder_get_macv_cfg_ps_en_else() && reg_symphony_sd_encoder_get_macv_cfg_agc_en_else() )? 1:0) << 5) |
					(reg_symphony_sd_encoder_get_macv_cfg_bp_en_else() << 4) |
					(reg_symphony_sd_encoder_get_macv_cfg_cs_en_n()  << 3 ) |
					(reg_symphony_sd_encoder_get_macv_cfg_agc_amp_chs() << 2) |
					reg_symphony_sd_encoder_get_macv_cfg_sync_reduce());
			break;
		case 1:
			reg_symphony_sd_encoder_set_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_n(value);
			MT_SDVENC_MACV_PRINT("N1 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_n());
			break;
		case 2:
			reg_symphony_sd_encoder_set_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_n(value);
			MT_SDVENC_MACV_PRINT("N2 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_n());
			break;
		case 3:
			reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_n(value);
			MT_SDVENC_MACV_PRINT("N3 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_n());
			break;
		case 4:
			reg_symphony_sd_encoder_set_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_n(value);
			MT_SDVENC_MACV_PRINT("N4 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_n());
			break;
		case 5:
			reg_symphony_sd_encoder_set_macv_n5_cs_spc_else_n5_cs_spc_else_n(value);
			MT_SDVENC_MACV_PRINT("N5 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_n5_cs_spc_else_n());
			break;
		case 6:
			reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field_n6_cs_num_in_field_n(value);
			MT_SDVENC_MACV_PRINT("N6 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_n6_cs_num_in_field_n());
			break;
		case 7:
			reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_n(value);
			MT_SDVENC_MACV_PRINT("N7 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_n());
			break;
		case 8:
			reg_symphony_sd_encoder_set_macv_n8_ps_dura_n8_ps_dura_n(value);
			MT_SDVENC_MACV_PRINT("N8 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n8_ps_dura_n8_ps_dura_n());
			break;
		case 9:
			reg_symphony_sd_encoder_set_macv_n9_fst_ps_start_n9_fst_ps_start_n(value);
			MT_SDVENC_MACV_PRINT("N9 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n9_fst_ps_start_n9_fst_ps_start_n());
			break;
		case 10:
			reg_symphony_sd_encoder_set_macv_n10_ps_spc_n10_ps_spc_n(value);
			MT_SDVENC_MACV_PRINT("N10 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n10_ps_spc_n10_ps_spc_n());
			break;
		case 11:
			reg_symphony_sd_encoder_set_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_n(value);
			MT_SDVENC_MACV_PRINT("N11 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_n());
			break;
		case 12:
			reg_symphony_sd_encoder_set_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_n(value);
			MT_SDVENC_MACV_PRINT("N12 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_n());
			break;
		case 13:
			reg_symphony_sd_encoder_set_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_n(value);
			MT_SDVENC_MACV_PRINT("N13 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_n());
			break;
		case 14:
			reg_symphony_sd_encoder_set_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_n(value);
			MT_SDVENC_MACV_PRINT("N14 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_n());
			break;
		case 15:
			reg_symphony_sd_encoder_set_macv_n15_bp_ln_chs_n15_bp_ln_chs_n(value);
			MT_SDVENC_MACV_PRINT("N15 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n15_bp_ln_chs_n15_bp_ln_chs_n());
			break;
		case 16:
			reg_symphony_sd_encoder_set_macv_cfg_n16_advanced_start_n(value);
			MT_SDVENC_MACV_PRINT("N16 = 0x%x\n", reg_symphony_sd_encoder_get_macv_cfg_n16_advanced_start_n());
			break;
		case 17:
			reg_symphony_sd_encoder_set_macv_n17_cs_zone1_n17_cs_zone1_n(value);
			MT_SDVENC_MACV_PRINT("N17 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n17_cs_zone1_n17_cs_zone1_n());
			break;
		case 18:
			reg_symphony_sd_encoder_set_macv_n18_cs_zone2_n18_cs_zone2_n(value);
			MT_SDVENC_MACV_PRINT("N18 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n18_cs_zone2_n18_cs_zone2_n());
			break;
		case 19:
			reg_symphony_sd_encoder_set_macv_n19_cs_zone3_n19_cs_zone3_n(value);
			MT_SDVENC_MACV_PRINT("N19 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n19_cs_zone3_n19_cs_zone3_n());
			break;
		case 20:
			reg_symphony_sd_encoder_set_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_n(value);
			MT_SDVENC_MACV_PRINT("N20 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_n());
			break;
		case 21:            
			reg_symphony_sd_encoder_set_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_n(value);
			MT_SDVENC_MACV_PRINT("N21 = 0x%x\n", reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_n());
			break;
		default:
			MT_SDVENC_MACV_PRINT("the register is not exist\n");
			break;
		} 
    }
    mpi_memdev_unmap_register((mt_void*)p_sdenc_macv_addr);
    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetMacrovisionRegN(MT_UNF_DISP_E enDisp, mt_u32 index, mt_u32 *pData)
{
	mt_s32 s32Ret = MT_SUCCESS;
	mt_u32 dtmp = 0;
	mt_u32 sd_fmt = 0;

	s32Ret = mpi_memdev_map_register(SYMPHONY_IO_PA(0xbf450000), 0xB0, (mt_void*)&p_sdenc_macv_addr);
	if (MT_SUCCESS != s32Ret)
	{
		MT_ERR_DISP("sdenc_macv_add map failed!\n");
		return s32Ret;
	}
	mt_sys_read_register(SYMPHONY_IO_PA(0xbf460034), &dtmp);
	sd_fmt = dtmp & 0x7;
	//0/1/7: PAL/; 3: SECAM; 5: PAL_N
	//2: NTSC_J; 4: PAL_M; 6:NTSC_443
	if((sd_fmt == 0) || (sd_fmt == 1) || (sd_fmt == 7) || (sd_fmt == 3) || (sd_fmt == 5))  //PAL
	{
		switch(index)
		{
		case 0:

			*pData = ((( reg_symphony_sd_encoder_get_macv_cfg_ps_en_else() && reg_symphony_sd_encoder_get_macv_cfg_agc_en_else() )? 1:0) << 5) |
				(reg_symphony_sd_encoder_get_macv_cfg_bp_en_else() << 4) |
				(reg_symphony_sd_encoder_get_macv_cfg_cs_en_p()  << 3 ) |
				(reg_symphony_sd_encoder_get_macv_cfg_agc_amp_chs() << 2) |
				reg_symphony_sd_encoder_get_macv_cfg_sync_reduce();
			break;
		case 1:
			*pData =  reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_p();
			break;
		case 2:
			*pData = reg_symphony_sd_encoder_get_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_p();
			break;
		case 3:
			*pData = reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_p();
			break;
		case 4:
			*pData = reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_p();
			break;
		case 5:
			*pData = reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_n5_cs_spc_else_p();
			break;
		case 6:
			*pData = reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_n6_cs_num_in_field_p();
			break;
		case 7:
			*pData = reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_p();
			break;
		case 8:
			*pData = reg_symphony_sd_encoder_get_macv_n8_ps_dura_n8_ps_dura_p();
			break;
		case 9:
			*pData = reg_symphony_sd_encoder_get_macv_n9_fst_ps_start_n9_fst_ps_start_p();
			break;
		case 10:
			*pData = reg_symphony_sd_encoder_get_macv_n10_ps_spc_n10_ps_spc_p();
			break;
		case 11:
			*pData = reg_symphony_sd_encoder_get_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_p();
			break;
		case 12:
			*pData = reg_symphony_sd_encoder_get_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_p();
			break;
		case 13:
			*pData = reg_symphony_sd_encoder_get_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_p();
			break;
		case 14:
			*pData = reg_symphony_sd_encoder_get_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_p();
			break;
		case 15:
			*pData = reg_symphony_sd_encoder_get_macv_n15_bp_ln_chs_n15_bp_ln_chs_p();
			break;
		case 16:
			*pData = reg_symphony_sd_encoder_get_macv_cfg_n16_advanced_start_p();
			break;
		case 17:
			*pData = reg_symphony_sd_encoder_get_macv_n17_cs_zone1_n17_cs_zone1_p();
			break;
		case 18:
			*pData = reg_symphony_sd_encoder_get_macv_n18_cs_zone2_n18_cs_zone2_p();
			break;
		case 19:
			*pData = reg_symphony_sd_encoder_get_macv_n19_cs_zone3_n19_cs_zone3_p();
			break;
		case 20:
			*pData = reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_p();
			break;
		case 21:
			*pData = reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_p();
			break;
		default:
			MT_SDVENC_MACV_PRINT("the register is not exist\n");
			break;
		} 
	}
	else
	{
		switch(index)
		{
		case 0:
			*pData =  ((( reg_symphony_sd_encoder_get_macv_cfg_ps_en_else() && reg_symphony_sd_encoder_get_macv_cfg_agc_en_else() )? 1:0) << 5) |
				(reg_symphony_sd_encoder_get_macv_cfg_bp_en_else() << 4) |
				(reg_symphony_sd_encoder_get_macv_cfg_cs_en_n()  << 3 ) |
				(reg_symphony_sd_encoder_get_macv_cfg_agc_amp_chs() << 2) |
				reg_symphony_sd_encoder_get_macv_cfg_sync_reduce();
			break;
		case 1:
			*pData =  reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_n();
			break;
		case 2:
			*pData =  reg_symphony_sd_encoder_get_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_n();
			break;
		case 3:
			*pData =  reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_n();
			break;
		case 4:
			*pData =  reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_n();
			break;
		case 5:
			*pData =  reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_n5_cs_spc_else_n();
			break;
		case 6:
			*pData =  reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_n6_cs_num_in_field_n();
			break;
		case 7:
			*pData =  reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_n();
			break;
		case 8:
			*pData =  reg_symphony_sd_encoder_get_macv_n8_ps_dura_n8_ps_dura_n();
			break;
		case 9:
			*pData =  reg_symphony_sd_encoder_get_macv_n9_fst_ps_start_n9_fst_ps_start_n();
			break;
		case 10:
			*pData =  reg_symphony_sd_encoder_get_macv_n10_ps_spc_n10_ps_spc_n();
			break;
		case 11:
			*pData =  reg_symphony_sd_encoder_get_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_n();
			break;
		case 12:
			*pData =  reg_symphony_sd_encoder_get_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_n();
			break;
		case 13:
			*pData =  reg_symphony_sd_encoder_get_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_n();
			break;
		case 14:
			*pData =  reg_symphony_sd_encoder_get_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_n();
			break;
		case 15:
			*pData =  reg_symphony_sd_encoder_get_macv_n15_bp_ln_chs_n15_bp_ln_chs_n();
			break;
		case 16:
			*pData =  reg_symphony_sd_encoder_get_macv_cfg_n16_advanced_start_n();
			break;
		case 17:
			*pData =  reg_symphony_sd_encoder_get_macv_n17_cs_zone1_n17_cs_zone1_n();
			break;
		case 18:
			*pData =  reg_symphony_sd_encoder_get_macv_n18_cs_zone2_n18_cs_zone2_n();
			break;
		case 19:
			*pData =  reg_symphony_sd_encoder_get_macv_n19_cs_zone3_n19_cs_zone3_n();
			break;
		case 20:
			*pData =  reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_n();
			break;
		case 21:
			*pData =  reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_n();
			break;
		default:
			MT_SDVENC_MACV_PRINT("the register is not exist\n");
			break;
		}
	}

    mpi_memdev_unmap_register((mt_void*)p_sdenc_macv_addr);
    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetCgms(MT_UNF_DISP_E enDisp, const MT_UNF_DISP_CGMS_CFG_S *pstCgmsCfg)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret = MT_SUCCESS;
    MT_DRV_DISP_CGMSA_CFG_S stCgmsCgf;

    if (!pstCgmsCfg)
    {
        MT_ERR_DISP("para pstCgmsCfg is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    (MT_VOID)Transfer_CgmsCfg(pstCgmsCfg, &stCgmsCgf, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetCgms(enDisp, (const MT_DRV_DISP_CGMSA_CFG_S *)&stCgmsCgf);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_AcquireSnapshot(MT_UNF_DISP_E enDisp, MT_UNF_VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_VIDEO_FRAME_S stFrame;
    mt_s32 s32Ret = MT_SUCCESS;

    memset(&stFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));
    if (!pstFrameInfo)
    {
        MT_ERR_DISP("para pstSnapShotFrame is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    s32Ret = MT_MPI_DISP_Snapshot_Acquire(enD, &stFrame);
    if (!s32Ret)
    {
        (mt_void) Transfer_Frame(pstFrameInfo, &stFrame, MT_FALSE);
    }

    return s32Ret;
}

mt_s32 MT_UNF_DISP_ReleaseSnapshot(MT_UNF_DISP_E enDisp, MT_UNF_VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret = MT_SUCCESS;
    MT_DRV_VIDEO_FRAME_S stFrame;

    if (!pstFrameInfo)
    {
        MT_ERR_DISP("para pstSnapShotFrame is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }
    memset(&stFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    memcpy(&stFrame.u32Priv[0], pstFrameInfo->u32Private, 64 * sizeof(mt_u32));
    s32Ret = MT_MPI_DISP_Snapshot_Release(enD, &stFrame);

    return s32Ret;
}

/**Defines the default buffer number.*/
#define MT_DISP_CAST_BUFFER_DEF_NUMBER (5)

mt_s32 MT_UNF_DISP_GetDefaultCastAttr(MT_UNF_DISP_E enDisp, MT_UNF_DISP_CAST_ATTR_S *pstAttr)
{
    if (!pstAttr)
    {
        MT_ERR_DISP("para pstAttr is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    if (enDisp >= MT_UNF_DISPLAY_BUTT)
    {
        MT_ERR_DISP("para enDisp is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    memset((void *)pstAttr, 0, sizeof(MT_UNF_DISP_CAST_ATTR_S));
    pstAttr->enFormat = MT_UNF_FORMAT_YUV_SEMIPLANAR_420;
    pstAttr->u32Width = 1280;
    pstAttr->u32Height = 720;
    pstAttr->u32BufNum = MT_DISP_CAST_BUFFER_DEF_NUMBER;
    pstAttr->bUserAlloc = MT_FALSE;
    pstAttr->bCrop = MT_FALSE;
    pstAttr->bLowDelay = MT_TRUE;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_DISP_CreateCast(MT_UNF_DISP_E enDisp, MT_UNF_DISP_CAST_ATTR_S *pstAttr, mt_handle *phCast)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_CAST_CFG_S stCfg;
    mt_s32 s32Ret = MT_SUCCESS;

    if (!pstAttr)
    {
        MT_ERR_DISP("para pstAttr is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }
    if (!phCast)
    {
        MT_ERR_DISP("para phCast is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    Transfer_CastCfg(pstAttr, &stCfg, MT_TRUE);

    s32Ret = MT_MPI_DISP_CreateCast(enD, &stCfg, phCast);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_DestroyCast(mt_handle hCast)
{
    mt_s32 s32Ret = MT_SUCCESS;

    s32Ret = MT_MPI_DISP_DestroyCast(hCast);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetLowDelayEnable(mt_handle hCast, MT_BOOL bEnable)
{
    mt_s32 s32Ret;
    s32Ret = MT_MPI_DISP_SetLowDelayEnable(hCast, bEnable);
    return s32Ret;
}
mt_s32 MT_UNF_DISP_SetCastEnable(mt_handle hCast, MT_BOOL bEnable)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_DISP_SetCastEnable(hCast, bEnable);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable)
{
    mt_s32 s32Ret;

    if (!pbEnable)
    {
        MT_ERR_DISP("para pbEnable is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    s32Ret = MT_MPI_DISP_GetCastEnable(hCast, pbEnable);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_AcquireCastFrame(mt_handle hCast, MT_UNF_VIDEO_FRAME_INFO_S *pstFrameInfo, mt_u32 u32TimeoutMs)
{
    MT_DRV_VIDEO_FRAME_S stFrame;
    mt_s32 s32Ret;
    mt_s32 s32TimeRet;
    mt_u32 u32OriTime = 0;
    mt_u32 u32Time = 0;
    mt_u32 u32Delta;

    if (!pstFrameInfo)
    {
        MT_ERR_DISP("para pstCastFrame is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    s32TimeRet = mt_sys_get_time_stamp_ms(&u32OriTime);
    if (s32TimeRet != MT_SUCCESS)
    {
        MT_ERR_DISP("GetTimeStampMs Failed\n");
        return MT_ERR_DISP_INVALID_OPT;
    }

    do
    {
        s32Ret = MT_MPI_DISP_AcquireCastFrame(hCast, &stFrame);
        if (!s32Ret)
        {
            Transfer_Frame(pstFrameInfo, &stFrame, MT_FALSE);
        }

        s32TimeRet = mt_sys_get_time_stamp_ms(&u32Time);
        if (s32TimeRet != MT_SUCCESS)
        {
            MT_ERR_DISP("GetTimeStampMs Failed\n");
            if (s32Ret != MT_SUCCESS)
            {
                return MT_ERR_DISP_INVALID_OPT;
            }
            else
            {
                return s32Ret;
            }
        }

        u32Delta = u32Time - u32OriTime;
        (mt_void) MT_USLEEP(1 * 1000);

    } while (s32Ret == MT_FAILURE && u32Delta <= u32TimeoutMs);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_ReleaseCastFrame(mt_handle hCast, MT_UNF_VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    MT_DRV_VIDEO_FRAME_S stFrame;
    mt_s32 s32Ret;

    if (!pstFrameInfo)
    {
        MT_ERR_DISP("para pstCastFrame is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    memset(&stFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));

    s32Ret = Transfer_Frame(pstFrameInfo, &stFrame, MT_TRUE);

    if (s32Ret == MT_SUCCESS)
    {
        s32Ret = MT_MPI_DISP_ReleaseCastFrame(hCast, &stFrame);
    }

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetColor(MT_UNF_DISP_E enDisp, MT_UNF_DISP_COLOR_SETTING_S *pstCS)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret = MT_SUCCESS;

    if (!pstCS)
    {
        MT_ERR_DISP("para pstCS is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    //s32Ret = MT_MPI_DISP_SetColor( enDisp, pstCS);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetColor(MT_UNF_DISP_E enDisp, MT_UNF_DISP_COLOR_SETTING_S *pstCS)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret = MT_SUCCESS;

    if (!pstCS)
    {
        MT_ERR_DISP("para pstCS is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    //s32Ret = MT_MPI_DISP_GetColor( enDisp, pstCS);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetDefaultPara(MT_UNF_DISP_E enDisp)
{
    MT_DRV_DISPLAY_E enD;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    MT_ERR_DISP("Set DefaultPara is not support now.\n");

    return MT_ERR_DISP_INVALID_OPT;
}

mt_s32 MT_UNF_DISP_Set3DMode(MT_UNF_DISP_E enDisp, MT_UNF_DISP_3D_E en3D, MT_UNF_ENC_FMT_E enEncFormat)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_FMT_E enF = MT_DRV_DISP_FMT_BUTT;
    MT_DRV_DISP_STEREO_MODE_E enDrv3D;
    mt_s32 s32Ret;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (en3D > MT_UNF_DISP_3D_TOP_AND_BOTTOM)
    {
        MT_ERR_DISP("para en3D is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (en3D == MT_UNF_DISP_3D_NONE)
    {
        if (enEncFormat >= MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING)
        {
            MT_ERR_DISP("para enEncodingFormat is invalid.\n");
            return MT_ERR_DISP_INVALID_PARA;
        }
    }
#if 0
    if (en3D == MT_UNF_DISP_3D_FRAME_PACKING)
    {
        if ((enEncFormat < MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING)
              || (enEncFormat > MT_UNF_ENC_FMT_720P_50_FRAME_PACKING))
        {
            MT_ERR_DISP("para enEncodingFormat is invalid.\n");
            return MT_ERR_DISP_INVALID_PARA;
        }
    }
    if (en3D == MT_UNF_DISP_3D_SIDE_BY_SIDE_HALF)
    {
        if ((enEncFormat != MT_UNF_ENC_FMT_1080i_60) && (enEncFormat != MT_UNF_ENC_FMT_1080i_50))
        {
            MT_ERR_DISP("para enEncodingFormat is invalid.\n");
            return MT_ERR_DISP_INVALID_PARA;
        }
    }
    if (en3D == MT_UNF_DISP_3D_TOP_AND_BOTTOM)
    {
        if ((enEncFormat != MT_UNF_ENC_FMT_1080P_24) && (enEncFormat != MT_UNF_ENC_FMT_720P_60) && (enEncFormat != MT_UNF_ENC_FMT_720P_50))
        {
            MT_ERR_DISP("para enEncodingFormat is invalid.\n");
            return MT_ERR_DISP_INVALID_PARA;
        }
    }
#endif
    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    Transfer_EncFmt(&enEncFormat, &enF, MT_TRUE);
    Transfer_Disp3DMode(&en3D, &enDrv3D, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetFormat(enD, enDrv3D, enF);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_Get3DMode(MT_UNF_DISP_E enDisp, MT_UNF_DISP_3D_E *pen3D, MT_UNF_ENC_FMT_E *penEncFormat)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_FMT_E enF;
    MT_DRV_DISP_STEREO_MODE_E enDrv3D;
    mt_s32 s32Ret;

    //CHECK_DISP_PTR(pen3D);
    //CHECK_DISP_PTR(penEncFormat);
    if (!pen3D || !penEncFormat)
    {
        MT_ERR_DISP("para is null ptr.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_GetFormat(enD, &enDrv3D, &enF);

    if (MT_SUCCESS == s32Ret)
    {
        Transfer_EncFmt(penEncFormat, &enF, MT_FALSE);
        Transfer_Disp3DMode(pen3D, &enDrv3D, MT_FALSE);
    }
    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetRightEyeFirst(MT_UNF_DISP_E enDisp, MT_BOOL bEnable)
{
    MT_DRV_DISPLAY_E enD;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }
    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable))
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    return MT_MPI_DISP_SetRightEyeFirst(enD, bEnable);
}

mt_s32 MT_UNF_DISP_SetVirtualScreen(MT_UNF_DISP_E enDisp, mt_u32 u32Width, mt_u32 u32Height)
{
    MT_DRV_DISPLAY_E enD;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    return MT_MPI_DISP_SetVirtualScreen(enD, u32Width, u32Height);
}

mt_s32 MT_UNF_DISP_GetVirtualScreen(MT_UNF_DISP_E enDisp, mt_u32 *u32Width, mt_u32 *u32Height)
{
    MT_DRV_DISPLAY_E enD;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    return MT_MPI_DISP_GetVirtualScreen(enD, u32Width, u32Height);
}
mt_s32 MT_UNF_DISP_SetSmallWindow(MT_UNF_DISP_E enDisp, mt_s32 xstart, mt_s32 ystart,mt_u32 u32Width, mt_u32 u32Height)
{
    MT_DRV_DISPLAY_E enD;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    return MT_MPI_DISP_SetSmallWindow(enD, xstart,ystart,u32Width, u32Height);
}

mt_s32 MT_UNF_DISP_SetScreenOffset(MT_UNF_DISP_E enDisp, MT_UNF_DISP_OFFSET_S *pstOffset)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_OFFSET_S drv_offset;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }
    if (!pstOffset)
    {
        MT_ERR_DISP("para pstOffset is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }
    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    Transfer_DispOffset(pstOffset, &drv_offset, MT_TRUE);

    return MT_MPI_DISP_SetScreenOffset(enD, &drv_offset);
}

mt_s32 MT_UNF_DISP_GetScreenOffset(MT_UNF_DISP_E enDisp, MT_UNF_DISP_OFFSET_S *pstOffset)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_OFFSET_S drv_offset;
    mt_s32 Ret = 0;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (!pstOffset)
    {
        MT_ERR_DISP("para pstOffset is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    Ret = MT_MPI_DISP_GetScreenOffset(enD, &drv_offset);

    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }

    Transfer_DispOffset(pstOffset, &drv_offset, MT_FALSE);

    return MT_SUCCESS;
}

mt_s32 MT_UNF_DISP_SetLayerShow(MT_UNF_DISP_E enDisp, MT_UNF_DISP_LAYER_ID_E enLayer, MT_BOOL bEnable)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_LAYER_ID_E enDrvLayer = (MT_DRV_DISP_LAYER_ID_E)enLayer;
    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable))
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    return MT_MPI_DISP_SetLayerShow(enD, enDrvLayer, bEnable);
}

mt_s32 MT_UNF_DISP_SetPPMode(MT_UNF_DISP_E enDisp, MT_UNF_DISP_PP_E enMode)
{
    MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_PPMODE_E enPPMode = MT_DRV_DISP_PPMODE_BUTT;
    mt_s32 s32Ret;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (MT_UNF_DISP_PP_BUTT <= enMode)
    {
        MT_ERR_DISP("para ppmode is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    Transfer_DispPPMode(&enMode, &enPPMode, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetPPMode(enD, enPPMode);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_VidLayerShow(MT_BOOL bEnable)
{
    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable))
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    return MT_MPI_DISP_VidLayerShow(bEnable);
}

mt_s32 MT_UNF_DISP_GetVidLayerEnable(MT_BOOL *pbEnable)
{
    mt_s32 s32Ret;

    if (!pbEnable)
    {
        MT_ERR_DISP("para pbEnable is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    s32Ret = MT_MPI_DISP_GetVidLayerEnable(pbEnable);

    return s32Ret;

}

mt_s32 MT_UNF_DISP_SetAlpha(MT_UNF_DISP_E enDisp, mt_u32 u32Alpha)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetAlpha(enD, u32Alpha);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetAlpha(MT_UNF_DISP_E enDisp, mt_u32 *u32Alpha)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);

    s32Ret = MT_MPI_DISP_GetAlpha(enD, u32Alpha);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_SetCscEnable(MT_BOOL bEnable)
{
    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable))
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    return MT_MPI_DISP_SetCscEnable(bEnable);
}

mt_s32 MT_UNF_DISP_SetDenoiseEnable(MT_BOOL bEnable)
{
    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable))
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    return MT_MPI_DISP_SetDenoiseEnable(bEnable);
}

mt_s32 MT_UNF_DISP_SetAfdEnable(MT_BOOL bEnable)
{
    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable))
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    return MT_MPI_DISP_SetAfdEnable(bEnable);
}

mt_s32 MT_UNF_DISP_SetHdVideoEnable(MT_BOOL bEnable)
{
    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable))
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    return MT_MPI_DISP_SetHdVideoEnable(bEnable);
}

mt_s32 MT_UNF_DISP_SetSdVideoEnable(MT_BOOL bEnable)
{
    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable))
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    return MT_MPI_DISP_SetSdVideoEnable(bEnable);
}

mt_s32 MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_INDEX_E eDacId, MT_BOOL bEnable)
{
    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable))
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    return MT_MPI_DISP_SetVdacOnOff((dac_index_t)eDacId, bEnable);
}

mt_s32 MT_UNF_DISP_SetDiOnOff(MT_BOOL bEnable)
{
    if ((MT_TRUE != bEnable) && (MT_FALSE != bEnable))
    {
        MT_ERR_WIN("para bEnable is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    return MT_MPI_DISP_SetDiOnOff(bEnable);
}

mt_s32 MT_UNF_DISP_SetTvCapability(MT_UNF_DISP_HDMI_MODE_E enTvCapability)
{
    //MT_DRV_DISPLAY_E enD;
    MT_DRV_DISP_HDMI_MODE_E enTvCap = MT_DRV_DISP_HDMI_MODE_BUTT;
    mt_s32 s32Ret;

    if (MT_UNF_DISP_HDMI_MODE_BUTT <= enTvCapability)
    {
        MT_ERR_DISP("para TvCapability is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispTvCap(&enTvCapability, &enTvCap, MT_TRUE);

    s32Ret = MT_MPI_DISP_SetTvCapability(enTvCap);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_ResetHardware(MT_BOOL bHighSpeed)
{
    if ((MT_TRUE != bHighSpeed) && (MT_FALSE != bHighSpeed))
    {
        MT_ERR_WIN("para bHighSpeed is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    return MT_MPI_DISP_ResetHardware(bHighSpeed);
}

mt_s32 MT_UNF_DISP_SetSdScalerEnable(MT_UNF_DISP_SCALER_MODE_E state)
{    
    if (MT_UNF_DISP_SCALER_MODE_BUTT <= state)
    {
        MT_ERR_WIN("para state is invalid.\n");
        return MT_ERR_VO_INVALID_PARA;
    }
    return MT_MPI_DISP_SetSdScalerEnable((mt_u32)state);
}

mt_s32 MT_UNF_DISP_DumpScaler2OSD(MT_UNF_DISP_DUMP_SCALER_PARA_S *pstParam)
{
    MT_DRV_DISP_DUMP_SCALER_PARA_S stDrvParam;
    Transfer_DumpScaler(pstParam, &stDrvParam, MT_TRUE);
    return MT_MPI_DISP_DumpScaler2OSD(&stDrvParam);
}

mt_s32 MT_UNF_DISP_Set_Sl_Hdr(MT_UNF_DISP_E enDisp, mt_u32 transparent_mode, mt_u32 display_brightness_hdr, mt_u32 display_brightness_sdr, mt_u32 tuning_level, mt_u32 display_OETF)
{
    MT_DRV_DISPLAY_E enD;
    mt_s32 s32Ret;

    if (enDisp > MT_UNF_DISPLAY1)
    {
        MT_ERR_DISP("para enDisp is invalid or not support now.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    Transfer_DispID(&enDisp, &enD, MT_TRUE);
    s32Ret = MT_MPI_DISP_Set_Sl_Hdr(enD, transparent_mode, (display_brightness_sdr << 16) | display_brightness_hdr, tuning_level, display_OETF);

    return s32Ret;
}

mt_s32 MT_UNF_DISP_GetSlHdrVersion(mt_u32 *pVer)
{
    mt_s32 s32Ret;

    if (!pVer)
    {
        MT_ERR_DISP("para pVer is null.\n");
        return MT_ERR_DISP_NULL_PTR;
    }

    s32Ret = MT_MPI_DISP_GetSlHdrVersion(pVer);

    return s32Ret;

}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
