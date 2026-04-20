/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/ 
#ifndef __MTFB_DRV_COMMON_H__
#define __MTFB_DRV_COMMON_H__
 

/*********************************add include here******************************/
#include "mtfb.h"
#include "mtfb_comm.h"
#include "mt_module_debug.h"


/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C"
{
#endif
#endif /* __cplusplus */



/***************************** Macro Definition ******************************/

#define MTFB_WORK_QUEUE      	 	"MTFB_WorkQueque"
#define OPTM_GFX_WBC2_BUFFER 	 	"MTFB_GfxWbc2"
#define DISPLAY_BUFFER_HD    		"MTFB_DispBuf_HD"
#define DISPLAY_BUFFER_SD    		"MTFB_DispBuf_SD"
#define MTFB_ZME_COEF_BUFFER 		"MTFB_ZmeCoef"
#define OPTM_GFX_CMP_BUFFER  		"MTFB_GfxCmp"
#define OPTM_GFX_CMP_QUEUE   		"MTFB_CompressQueque"

//#define CFG_MTFB_LOGO_SUPPORT
#define CFG_MTFB_VIRTUAL_COORDINATE_SUPPORT

#ifndef MT_ADVCA_FUNCTION_RELEASE
#define CFG_MTFB_PROC_SUPPORT
#endif

#define GFX_TEST_LOG	
#ifdef GFX_TEST_LOG
#define PRINT_IN  MT_INFO_MTFB("-----in-----\n")
#define PRINT_OUT MT_INFO_MTFB("-----out----\n")
#else
#define PRINT_IN 
#define PRINT_OUT
#endif 

/*the point of callback function*/
/*CNcomment:回调函数指针*/
typedef mt_void (* IntCallBack)(mt_void *pParaml, mt_void *pParamr);


#define IS_HD_LAYER(enLayerId) (enLayerId < MTFB_LAYER_HD_1)
#define IS_SD_LAYER(enLayerId) ((enLayerId >= MTFB_LAYER_SD_0) && (enLayerId < MTFB_LAYER_SD_1))
#define IS_AD_LAYER(enLayerId) ((enLayerId >= MTFB_LAYER_AD_0) && (enLayerId <= MTFB_LAYER_AD_3))
#define IS_MINOR_HD_LAYER(enLayerId) ((enLayerId <= MTFB_LAYER_HD_3) && (enLayerId >= MTFB_LAYER_HD_1))
#define IS_MINOR_SD_LAYER(enLayerId) ((enLayerId <= MTFB_LAYER_SD_3) && (enLayerId >= MTFB_LAYER_SD_1))


#define MTFB_CHECK_PONITER(pStr) do{\
if (pStr == MT_NULL){\
      MTFB_ERROR("unable to process null pointer!\n");\
      return MT_FAILURE;}\
}wmtle(0)


/*************************** Structure Definition ****************************/

typedef struct
{
    MT_BOOL bKeyEnable;         /** colorkey enable flag **//*CNcomment:colorkey 是否使能*/
    MT_BOOL bMaskEnable;        /** key mask enable flag **//*CNcomment:key mask 是否使能*/
    mt_u32 u32Key;              /** key value **/
    mt_u8 u8RedMask;           /** red mask   **/
    mt_u8 u8GreenMask;         /** green mask **/
    mt_u8 u8BlueMask;          /** blue mask  **/
    mt_u8 u8Reserved;           
    mt_u32 u32KeyMode;	 /*0:In region; 1:Out region*/

    /*Max colorkey value of red component*/
    /*CNcomment:colorkey红色分量最大值*/
    mt_u8 u8RedMax;

    /*Max colorkey value of Green component*/
    /*CNcomment:colorkey绿色分量最大值*/
    mt_u8 u8GreenMax; 

    /*Max colorkey value of blue component*/
    /*CNcomment:colorkey蓝色分量最大值*/
    mt_u8 u8BlueMax;           
    mt_u8 u8Reserved1;

    /*Min colorkey value of red component*/
    /*CNcomment:colorkey红色分量最小值*/
    mt_u8 u8RedMin;            

    /*Min colorkey value of Green component*/
    /*CNcomment:colorkey绿色分量最小值*/
    mt_u8 u8GreenMin;         

    /*Min colorkey value of blue component*/
    /*CNcomment:colorkey蓝色分量最小值*/
    mt_u8 u8BlueMin;            
    mt_u8 u8Reserved2;
}MTFB_COLORKEYEX_S;

typedef enum
{
    /*VO vertical timing interrupt */
    /*CNcomment:垂直时序中断*/
    MTFB_CALLBACK_TYPE_VO = 0x1, 

	/*3D Mode changed interrupt*/
    /*CNcomment:3D模式改变中断*/
    MTFB_CALLBACK_TYPE_3DMode_CHG = 0x2,

	/*VO Register update completed interrupt */
    /*CNcomment:寄存器更新完成中断*/
    MTFB_CALLBACK_TYPE_REGUP = 0x4, 
    
	/*Frame start interrupt */
    /*CNcomment:帧开始中断*/    
    MTFB_CALLBACK_TYPE_FRAME_START = 0x8, 
    
	/*Frame end interrupt */
    /*CNcomment:帧结束中断*/     
    MTFB_CALLBACK_TYPE_FRAME_END = 0x10,  
    
    MTFB_CALLBACK_TYPE_BUTT,
}MTFB_CALLBACK_TPYE_E;

/*scan mode*/
typedef enum
{
    MTFB_SCANMODE_P,
    MTFB_SCANMODE_I,
    MTFB_SCANMODE_BUTT,
}MTFB_SCAN_MODE_E;

/*layer state*/
typedef enum 
{
    MTFB_LAYER_STATE_ENABLE = 0x0,  /*Layer enable*/ /*CNcomment:层使能*/
    MTFB_LAYER_STATE_DISABLE,          /*Layer disable*/ /*CNcomment:层未使能*/
    MTFB_LAYER_STATE_INVALID,          /*Layer invalid*/ /*CNcomment:层无效,不存在*/
    MTFB_LAYER_STATE_BUTT 
} MTFB_LAYER_STATE_E;


/* GFX mode  */
typedef enum tagMTFB_GFX_MODE_EN
{
    MTFB_GFX_MODE_NORMAL = 0,
    MTFB_GFX_MODE_HD_WBC,
    MTFB_GFX_MODE_BUTT
}MTFB_GFX_MODE_EN;



/*osd info*/
typedef struct
{
    MTFB_LAYER_STATE_E eState;
	/*Layer work mode, same source mode or different source mode*/
    /*CNcomment:图层工作模式，同源或非同源*/
    MTFB_GFX_MODE_EN  eGfxWorkMode;
	MT_BOOL bPreMul;
    mt_u32  u32BufferPhyAddr; 
    mt_u32  u32RegPhyAddr; 
    mt_u32  u32Stride;
	/*Screen width in current format*/
    /*CNcomment:当前制式下屏幕宽*/
    mt_u32 u32ScreenWidth;  
    /*Screen height in current format*/
    /*CNcomment:当前制式下屏幕高度*/
    mt_u32 u32ScreenHeight; 
    /**outRect size*/
	MTFB_RECT stOutRect;
	MTFB_RECT stInRect;
    MTFB_COLOR_FMT_E eFmt;
    MTFB_ALPHA_S stAlpha;
    MTFB_COLORKEYEX_S stColorKey; 
	MTFB_SCAN_MODE_E eScanMode;
	MTFB_LAYER_ID_E  enSlaveryLayerID;
}MTFB_OSD_DATA_S;

/*osd info*/
typedef struct
{
    MT_BOOL bShow;
	MT_BOOL bOpen;
	/*Layer work mode, same source mode or different source mode*/
    /*CNcomment:图层工作模式，同源或非同源*/
    MTFB_GFX_MODE_EN  eGfxWorkMode;
    mt_u32 u32Stride;
	mt_u32 u32WbcBufNum;
    mt_u32 u32WbcBufSize;
	mt_u32 u32ReadBufAddr;

	/*source buffer of write back*/
	MTFB_RECT stSrcBufRect;
	/*current buffer of write back*/
	MTFB_RECT stCurWBCBufRect;
	/*max buffer of write back*/
	MTFB_RECT stMaxWbcBufRect;
	/*display region of write back*/
    MTFB_RECT stScreenRect;
	
    MTFB_COLOR_FMT_E eFmt;
	MTFB_SCAN_MODE_E eScanMode;
	MTFB_LAYER_ID_E  enLayerID;
}MTFB_SLVLAYER_DATA_S;


/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/

#ifdef __cplusplus

#if __cplusplus

}
#endif
#endif /* __cplusplus */

#endif /* __MTFB_DRV_COMMON_H__ */


