/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

//lint -wlib(0)
#include "string.h"

#include "mt_go_comm.h"
#include "mt_go_surface.h"
#include "mtgo_common.h"
#include "mtgo_surface.h"
#include "mtgo_adp_sys.h"
#include "mt_common.h"

#define MTGO_LOG222  printf
/***************************** Macro Definition ******************************/


#define CHECK_SURFACE_INIT() \
do \
{  \
    if (UN_INIT_STATE == s_MtGoInitSurCount) \
    { \
        MTGO_ERROR(MTGO_ERR_NOTINIT);\
        return MTGO_ERR_NOTINIT; \
    } \
}  \
while(0) 

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

//static MT_BOOL g_bSurfaceInited = MT_FALSE;
static mt_s32 s_MtGoInitSurCount = 0;

/******************************* API declaration *****************************/

mt_s32 MTGO_InitSurface()
{
    mt_s32 s32Ret;
    
    /** re initial and remember initial times */
    if (UN_INIT_STATE != s_MtGoInitSurCount)
    {
        /** */
        s_MtGoInitSurCount++;
        return MT_SUCCESS;
    }  

//    MTGO_LOG222("[%s]: [%d]\n", __FUNCTION__, __LINE__);
    s32Ret = MTGO_InitMemory();
    if (MT_SUCCESS != s32Ret)
    {
//        MTGO_LOG222("[%s]: [%d]\n", __FUNCTION__, __LINE__);
        MTGO_ERROR(s32Ret);
        goto err0;
    }

    s_MtGoInitSurCount++;
    return MT_SUCCESS;
err0:
    return s32Ret;
}

mt_s32 MTGO_DeinitSurface()
{
    /** avoid non initial case  */
    if (UN_INIT_STATE == s_MtGoInitSurCount)
    {
        MTGO_ERROR(MTGO_ERR_NOTINIT);
        return MTGO_ERR_NOTINIT;
    }

    /** if has initialed exit*/
    if (s_MtGoInitSurCount != CLEAR_INIT_STATE)
    {   
        s_MtGoInitSurCount--;
        return MT_SUCCESS;
    }

    MTGO_DeInitMemory();

    /** */
    s_MtGoInitSurCount--;
    return MT_SUCCESS;
}

/** 
\brief create surface YUV and  CLUT format
\param[in] Surface
\param[in] pType
\retval none
\return none
*/
mt_s32 MTGO_CreateSurfaceFromMem(const MTGO_SURINFO_S *pSurInfo, MTGO_MOD_E Mode, MTGO_HANDLE *pSurface)
{
    MT_PIXELDATA pData;
    MTGO_HANDLE Surface;
    mt_s32 ret;

    CHECK_SURFACE_INIT();
    
	MTGO_MemSet (pData, 0, sizeof (MT_PIXELDATA));

    pData[0].Pitch = pSurInfo->Pitch[0];
    pData[1].Pitch = pSurInfo->Pitch[0];

    Surface_CalculateBpp0(pSurInfo->PixelFormat, &(pData[0].Bpp));
    Surface_CalculateBpp1(pSurInfo->PixelFormat, &(pData[1].Bpp));
    
    if ((IS_RGB_FORMAT(pSurInfo->PixelFormat)) || IS_CLUT_FORMAT(pSurInfo->PixelFormat))
    {
        /** RGB and CLUT format */
        pData[0].Format = MTGO_PDFORMAT_RGB;
        pData[1].Format = MTGO_PDFORMAT_BUTT;
        pData[2].Format = MTGO_PDFORMAT_BUTT;
        pData[0].pData = pSurInfo->pVirAddr[0];
        pData[0].pPhyData = pSurInfo->pPhyAddr[0];
    }
    else if(pSurInfo->PixelFormat == MTGO_PF_SP_CMYK)
    {
        /**  CMYK format */
        pData[0].Format = MTGO_PDFORMAT_CM;
        pData[1].Format = MTGO_PDFORMAT_YK;
        pData[2].Format = MTGO_PDFORMAT_BUTT;
        pData[0].pData = pSurInfo->pVirAddr[0];
        pData[1].pData = pSurInfo->pVirAddr[1];
        pData[0].pPhyData = pSurInfo->pPhyAddr[0];
        pData[1].pPhyData = pSurInfo->pPhyAddr[1];
    }		
	else
    {
        /**  YUV format */
        pData[0].Format = MTGO_PDFORMAT_Y;
        pData[1].Format = MTGO_PDFORMAT_UV;
        pData[2].Format = MTGO_PDFORMAT_BUTT;
        pData[0].pData = pSurInfo->pVirAddr[0];
        pData[1].pData = pSurInfo->pVirAddr[1];
        pData[0].pPhyData = pSurInfo->pPhyAddr[0];
        pData[1].pPhyData = pSurInfo->pPhyAddr[1];
    }
    
    /** create Surface */
    ret = Surface_CreateSurface(&Surface, pSurInfo->Width, pSurInfo->Height, pSurInfo->PixelFormat);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }

    /** set surface as memory type*/
    ret = Surface_SetSurfaceType(Surface, MTGO_SUR_EMPTY_E);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        goto err1;
    }
    
    /** set privad  data */
    ret = Surface_SetSurfacePrivateData (Surface, MTGO_MOD_MEMSURFACE, pData);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        goto err1;
    }

    *pSurface = Surface;

     //printf ("w = %d, h = %d, Pitch = %d, Size = %d.\n", Width, Height, pData[0].Pitch, Size);
//     printf("CreateSurfaceFromMem0: w[%d] h[%d] f[%x][%x] adr[%x][%x] p[%x]\n", pSurInfo->Width, pSurInfo->Height, pSurInfo->PixelFormat, pData[0].Format, (mt_u32)pData[0].pData, (mt_u32)pData[0].pPhyData, pData[0].Pitch);
//	 printf("CreateSurfaceFromMem1: w[%d] h[%d] f[%x][%x] adr[%x][%x] p[%x]\n", pSurInfo->Width, pSurInfo->Height, pSurInfo->PixelFormat, pData[1].Format, (mt_u32)pData[1].pData, (mt_u32)pData[1].pPhyData, pData[1].Pitch);

    return MT_SUCCESS;
err1:
    Surface_FreeSurface(Surface);  
    return ret;
}


/** 
\brief createsurface YUV and  CLUT format
\param[in] Surface
\param[in] pType
\retval none
\return none
*/
mt_s32 MTGO_CreateSurface(const MTGO_SURINFO_S *pSurInfo, MTGO_MOD_E Mode, MTGO_HANDLE *pSurface, mt_u32 MemModID)
{
    MT_PIXELDATA pData;
    mt_u32 Size;
    MTGO_HANDLE Surface;
    mt_s32 ret;
    mt_void * ptr;
    phys_addr_t pPhyAddr = 0;
    mt_u32 WStride0 = 0, WStride1 = 0, HStride0 = 0, HStride1 = 0;
    CHECK_SURFACE_INIT();
    
    MTGO_MemSet (pData, 0, sizeof (MT_PIXELDATA));

    /** calculate basic info , if the pitch has been set, then use  default pitch.*/
    Surface_CalculateBpp0(pSurInfo->PixelFormat, &(pData[0].Bpp));
    Surface_CalculateBpp1(pSurInfo->PixelFormat, &(pData[1].Bpp));
    if (0 == pSurInfo->Pitch[0])
    {
        Surface_CalculateStride0(pSurInfo->PixelFormat, (mt_u32)pSurInfo->Width, (mt_u32)pSurInfo->Height, &WStride0, &HStride0);
    }
    else
    {   
        WStride0 = (mt_u32)pSurInfo->Pitch[0];
        HStride0 = (mt_u32)pSurInfo->Height;
    }
    
    if (0 == pSurInfo->Pitch[1])
    {
        Surface_CalculateStride1(pSurInfo->PixelFormat, (mt_u32)pSurInfo->Width, (mt_u32)pSurInfo->Height, &WStride1, &HStride1);
    }
    else
    {   
        WStride1 = pSurInfo->Pitch[1];
        HStride1 = (mt_u32)pSurInfo->Height;
    }
    
    pData[0].Pitch = WStride0;
    pData[1].Pitch = WStride1;
    Size = (mt_u32)(HStride0 * WStride0 + HStride1 * WStride1);
    /** allocate memory  */
    ptr = MTGO_MMZ_Malloc(Size, &pPhyAddr);
    
    //BM_TRACE("ptr 0x%x\n", ptr);
    if (MT_NULL == ptr)
    {
        MTGO_ERROR(MTGO_ERR_NOMEM);
        return MTGO_ERR_NOMEM;
    }
    memset(ptr, 0x00, Size);  // becuase the buffer maybe carry-over
    
    if ((IS_RGB_FORMAT(pSurInfo->PixelFormat)) || IS_CLUT_FORMAT(pSurInfo->PixelFormat))
    {
        /** RGB and CLUT format */
        pData[0].Format = MTGO_PDFORMAT_RGB;
        pData[1].Format = MTGO_PDFORMAT_BUTT;
        pData[2].Format = MTGO_PDFORMAT_BUTT;
        pData[0].pData = ptr;
        //FIXME
        pData[0].pPhyData = pPhyAddr;
    }
    else if(pSurInfo->PixelFormat == MTGO_PF_SP_CMYK)
    {
        /**  CMYK format */
        pData[0].Format = MTGO_PDFORMAT_CM;
        pData[1].Format = MTGO_PDFORMAT_YK;
        pData[2].Format = MTGO_PDFORMAT_BUTT;
        pData[0].pData = ptr;
        pData[1].pData = (mt_void *)((mt_char *)ptr + HStride0 * WStride0);
        pData[0].pPhyData = pPhyAddr;
        pData[1].pPhyData =pData[0].pPhyData + HStride0 * WStride0;
    }		
    else 
    {
        /**  YUV format */
        pData[0].Format = MTGO_PDFORMAT_Y;
        pData[1].Format = MTGO_PDFORMAT_UV;
        pData[2].Format = MTGO_PDFORMAT_BUTT;
        pData[0].pData = ptr;
        pData[1].pData = (mt_void *)((mt_char *)ptr + HStride0 * WStride0);
        pData[0].pPhyData = pPhyAddr;
        pData[1].pPhyData = pData[0].pPhyData + HStride0 * WStride0;
    }
    
    /** create Surface */
    ret = Surface_CreateSurface(&Surface, pSurInfo->Width, pSurInfo->Height, pSurInfo->PixelFormat);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        goto err0;
    }
    /** set surface memory type*/
    ret = Surface_SetSurfaceType(Surface, MTGO_SUR_MEM_E);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        goto err1;
    }

    /** set privad data  */
    ret = Surface_SetSurfacePrivateData (Surface, MTGO_MOD_MEMSURFACE, pData);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        goto err1;
    }

    *pSurface = Surface;

      //printf ("w = %d, h = %d, Pitch = %d, Size = %d.\n", Width, Height, pData[0].Pitch, Size);
//    printf ("MTGO_CreateSurface0: [%x][%x] [%d][%d] \n", pData[0].pData, pData[0].pPhyData, pData[0].Pitch, Size);
//	printf ("MTGO_CreateSurface1: [%x][%x] [%d][%d] \n", pData[1].pData, pData[1].pPhyData, pData[1].Pitch, Size);
    return MT_SUCCESS;
err1:
    Surface_FreeSurface(Surface);
err0:
    MTGO_MMZ_Free(ptr);

    return ret;
}


mt_void MTGO_FreeSurface(MTGO_HANDLE Surface)
{
    if (UN_INIT_STATE == s_MtGoInitSurCount) 
    { 
        return; 
    } 
    
    Surface_FreeSurfacePrivateData(Surface);
    Surface_FreeSurface(Surface);
    
    return ;
}

