/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

//lint -wlib(0)
#include "mt_go.h"
#include "mtgo_common.h"
#include "mtgo_adp_sys.h"
#include "mtgo_surface.h"
#include "mtgo_blit.h"
#include "mtgo_gdev.h"
#include "mt_module_debug.h"

/***************************** Macro Definition ******************************/
#define MTGO_VER_MAJOR 2
#define MTGO_VER_MINOR 0
#define MTGO_VER_Z 1
#define MTGO_VER_P 6
#define MTGO_VER_A "a34"  /* for  release version £¬please use the words"final" */

#define MAKE_VER_BIT(x) # x
#define MAKE_MACRO2STR(exp) MAKE_VER_BIT(exp)
#define MAKE_VERSION \
    MAKE_MACRO2STR(MTGO_VER_MAJOR) "." \
    MAKE_MACRO2STR(MTGO_VER_MINOR) "." \
    MAKE_MACRO2STR(MTGO_VER_Z) "." \
    MAKE_MACRO2STR(MTGO_VER_P) " " MTGO_VER_A
    
/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
const mt_char* g_pszBuildTime = "This version is building at " __DATE__ " " __TIME__;

/******************************* API declaration *****************************/
extern mt_s32 MTGO_InitDecoder(mt_void);
extern mt_s32 MTGO_DeinitDecoder(mt_void);


mt_s32 MT_GO_Init()
{
    mt_s32 ret;

    ret = MTGO_InitSurface();
    if (ret != MT_SUCCESS)
    {
	MT_ERR_MTGO("ret :(%d) \n",ret);
        return ret;
    }


    ret = MTGO_InitBliter();
    if (ret != MT_SUCCESS)
    {
	MT_ERR_MTGO("ret :(%d) \n",ret);
        goto fail0;
    }

    ret = MTGO_InitDecoder();
    if (ret != MT_SUCCESS)
    {
	MT_ERR_MTGO("ret :(%d) \n",ret);
        goto fail4;
    }

    ret = MTGO_InitDisplay();
    if (ret != MT_SUCCESS)
    {
	MT_ERR_MTGO("ret :(%d) \n",ret);
        goto fail4;
    }

    return MT_SUCCESS;
fail4:
    (mt_void)MTGO_DeinitBliter();
    
    
fail0:
    (mt_void)MTGO_DeinitSurface();

    return ret;
}

mt_s32 MT_GO_Deinit()
{
    mt_s32 ret = MT_SUCCESS;
    ret = MTGO_DinitDisplay();
    ret |= MTGO_DeinitDecoder();
    ret |= MTGO_DeinitBliter();
    ret |= MTGO_DeinitSurface();
    if (ret != MT_SUCCESS)
    {
        MTGO_ERROR(ret);
        return ret;
    }
    return ret;
}


#ifndef TEST_IN_ROOTBOX
mt_s32 MTGO_GetRealRect(const MT_RECT* pSrcRect, const MT_RECT* pRect, MT_RECT* pRealRect)
{
    MT_REGION Src1Region, Src2Region, RealRegion;

    /** check is the input rectangle is valid */
    RECT2REGION(*pSrcRect, RealRegion);

    if (MT_NULL != pRect)
    {
        if ((0 >= pRect->w) || (0 >= pRect->h))
        {
            MTGO_ERROR(MTGO_ERR_EMPTYRECT);
            return MTGO_ERR_EMPTYRECT;
        }

        RECT2REGION (*pRect, Src1Region);
        Src2Region = RealRegion;

        RealRegion.l = MTGO_MAX(Src1Region.l, Src2Region.l);
        RealRegion.t = MTGO_MAX(Src1Region.t, Src2Region.t);

        RealRegion.r = MTGO_MIN(Src1Region.r, Src2Region.r);
        RealRegion.b = MTGO_MIN(Src1Region.b, Src2Region.b);

        if ((RealRegion.r < RealRegion.l) || (RealRegion.b < RealRegion.t))
        {
            MTGO_ERROR(MTGO_ERR_OUTOFBOUNDS);
            return MTGO_ERR_OUTOFBOUNDS;
        }
    }

    REGION2RECT (*pRealRect, RealRegion);
    return MT_SUCCESS;
}
#endif


