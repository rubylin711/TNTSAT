/******************************************************************************
 * Montage Technology (Shanghai) Co., Ltd.                                                  
 * Montage Proprietary and Confidential                                                     
 * Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         
******************************************************************************/



#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#include "mt_drv_disp.h"

enum 
{
    MT_ERR_PNG_DEV_NOT_OPEN = 1,  /**<  tde device not open yet */ 
    MT_ERR_PNG_DEV_OPEN_FAILED,                 /**<  open tde device failed */
    MT_ERR_PNG_NULL_PTR,                        /**<  input parameters contain null ptr */
    MT_ERR_PNG_NO_MEM,                          /**<  malloc failed  */
    MT_ERR_PNG_MEM_MAP_FAILED,                  /**<  memory map failed */
    MT_ERR_PNG_INVALID_HANDLE,                  /**<  invalid job handle */
    MT_ERR_PNG_INVALID_PARA,                    /**<  invalid parameter */
    MT_ERR_PNG_NOT_ALIGNED,                     /**<  aligned error for position, stride, width */
    MT_ERR_PNG_JOB_TIMEOUT,                     /**<  blocked job wait timeout */
    MT_ERR_PNG_ERR_OCCURED,                     /**<  error occured when decoding */
};

typedef struct
{
    void (*Reset) (void* userdata);
    /** Retrieving data **/
    /*
    * Fetch data from buffer.
    *
    * The maximum number of bytes to fetch is specified by "length",
    * the actual number of bytes fetched is returned via "ret_read".
    */
    mt_s32 (*GetData) (void* userdata, mt_u32 length, void* ret_data, mt_u32* ret_read, MT_BOOL* eof);
    /*
    * Get the length of buffer in bytes.
    */
    mt_u32 (*GetLength) (void* userdata);
}MT_PNG_STREAM_CB;

#ifdef CONFIG_MT_FPGA_GPE  
typedef struct
{
//	pix_fmt_t out_fmt;
	MT_BOOL comp_dis;
  MT_BOOL sw_dec;
	MT_BOOL conv2argb_dis;
	MT_BOOL clut_gray_to_8bit;
	MT_BOOL reset_test;
} MT_PNG_DBGCFG_S;
#endif


typedef struct PNG_DECINFO
{
    // @deprecated
    mt_char *pStreamAddr; 
    // @deprecated
    mt_u32 streamLen;
    // @deprecated
    mt_char *streamReadPtr;

    phys_addr_t pImgPhyAddr;
    mt_char *pImgVirAddr;
    png_structp pPng;
    png_infop   pInfo;
    MT_PNG_STREAM_CB* streamFuncs;
    void* userdata;
    pix_fmt_t outImageFmt;
#ifdef CONFIG_MT_FPGA_GPE  
    MT_PNG_DBGCFG_S dbg_cfg;
#endif
} MT_PNG_DECINFO_S;

typedef struct
{
    phys_addr_t phyAddr;
    void *virAddr;
    pix_fmt_t pixFormat;
}MT_PNG_IMAGE_INFO_S;


//#define PNG_BS_BUF_SIZE (20*1024)
#define PNG_BS_BUF_SIZE (1024*1024)
#define PNG_ZOUT_BUF_SIZE (32 * 1024)


mt_s32 MT_PNG_Open(mt_handle *hPngHandle);
mt_void MT_PNG_Close(mt_handle hPngHandle);
mt_s32 MT_PNG_SetDecInfo(mt_handle hPngHandle, png_structp pPng, png_infop pInfo);
mt_s32 MT_PNG_SetOutImgInfo(mt_handle hPngHandle, const MT_PNG_IMAGE_INFO_S *imageInfo);
mt_s32 MT_PNG_Decode(mt_handle hPngHandle);
mt_s32 MT_PNG_GetRowBytes(mt_handle hPngHandle, pix_fmt_t pixForamt, mt_u32 *pstStride);
#ifdef CONFIG_MT_FPGA_GPE
MT_BOOL MT_PNG_IfHwSupport(MT_BOOL dbg_HWSupport);
mt_s32 MT_PNG_SetDbgInfo(mt_handle hPngHandle, MT_PNG_DBGCFG_S *dbg_cfg);
void MT_PNG_reg_test(void);
#else
MT_BOOL MT_PNG_IfHwSupport(png_structp pPng, png_infop pInfo);
#endif
// MT_PNG_SetStream is deprecated, suggest to move to MT_PNG_SetStreamCallBack
// @deprecated.
mt_s32 MT_PNG_SetStream(mt_handle hPngHandle, ulong addr, mt_u32 len);
mt_s32 MT_PNG_SetStreamCallBack(mt_handle hPngHandle, MT_PNG_STREAM_CB* funcs, void* userdata);
mt_s32 MT_PNG_GetPalette(mt_handle hPngHandle, mt_u32 *Palette, mt_u32 size);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */


