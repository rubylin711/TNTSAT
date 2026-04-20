/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "mt_type.h"
#include "mt_unf_cc.h"
#include "mt_cc_parse.h"
#include "mt_cc_fifo.h"
#include "mt_cc_arib.h"
#include "com_cc708.h"

static MT_BOOL  b_init = MT_FALSE;


/******************************* API Declaration *****************************/
/**
\brief Initialize cc module. CNcomment: 初始化CC模块。CNend
\attention \n
none. CNcomment: 无。CNend
\retval ::MT_SUCCESS initialize success. CNcomment: 初始化成功。CNend
\retval ::MT_FAILURE initialize failure. CNcomment: 初始化失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Init(mt_void)
{
    Com_CC708_Init();
    return MT_SUCCESS;
}




/**
\brief DeInitialize cc module. CNcomment: 去初始化CC模块。CNend
\attention \n
none. CNcomment: 无。CNend
\retval ::MT_MT_SUCCESS deinitialize success. CNcomment: 去初始化成功。CNend
\retval ::MT_FAILURE deinitialize failure. CNcomment: 去初始化失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_DeInit(mt_void)
{
    Com_CC708_Deinit();
    return MT_SUCCESS;
}

/**
\brief Get default attribution in cc module. CNcomment: 获取CC模块的默认属性值。CNend
\attention \n
none. CNcomment: 无。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_GetDefaultAttr(MT_UNF_CC_ATTR_S *pstDefaultAttr)
{
    if(pstDefaultAttr->enCCDataType == MT_UNF_CC_DATA_TYPE_608)
    {
        pstDefaultAttr->unCCConfig.stCC608ConfigParam.enCC608DataType = MT_UNF_CC_608_DATATYPE_CC1;
        pstDefaultAttr->unCCConfig.stCC608ConfigParam.u32CC608TextColor = MT_UNF_CC_COLOR_WHITE;
        pstDefaultAttr->unCCConfig.stCC608ConfigParam.enCC608TextOpac = MT_UNF_CC_OPACITY_DEFAULT;
        pstDefaultAttr->unCCConfig.stCC608ConfigParam.u32CC608BgColor = MT_UNF_CC_COLOR_BLACK;
        pstDefaultAttr->unCCConfig.stCC608ConfigParam.enCC608BgOpac = MT_UNF_CC_OPACITY_DEFAULT;
        pstDefaultAttr->unCCConfig.stCC608ConfigParam.enCC608FontStyle = MT_UNF_CC_FONTSTYLE_DEFAULT;
        pstDefaultAttr->unCCConfig.stCC608ConfigParam.enCC608DispFormat = MT_UNF_CC_DF_720X480;
        pstDefaultAttr->unCCConfig.stCC608ConfigParam.bLeadingTailingSpace = MT_FALSE;
    }
    else if(pstDefaultAttr->enCCDataType == MT_UNF_CC_DATA_TYPE_708)
    {
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.enCC708ServiceNum = MT_UNF_CC_708_SERVICE1;
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.enCC708FontName = MT_UNF_CC_FN_DEFAULT;	
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.enCC708FontStyle = MT_UNF_CC_FONTSTYLE_DEFAULT;	
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.enCC708FontSize = MT_UNF_CC_FONTSIZE_DEFAULT;	
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.u32CC708TextColor = MT_UNF_CC_COLOR_WHITE;
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.enCC708TextOpac = MT_UNF_CC_OPACITY_DEFAULT;	
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.u32CC708BgColor = 	MT_UNF_CC_COLOR_BLACK;
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.enCC708BgOpac = MT_UNF_CC_OPACITY_DEFAULT;	
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.u32CC708WinColor = MT_UNF_CC_COLOR_BLACK;	
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.enCC708WinOpac = MT_UNF_CC_OPACITY_DEFAULT;	
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.enCC708TextEdgeType = MT_UNF_CC_EDGETYPE_DEFAULT;	
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.u32CC708TextEdgeColor = MT_UNF_CC_COLOR_BLACK;	
        pstDefaultAttr->unCCConfig.stCC708ConfigParam.enCC708DispFormat = MT_UNF_CC_DF_720X480;	
    }
    else if(pstDefaultAttr->enCCDataType == MT_UNF_CC_DATA_TYPE_ARIB)
    {
        pstDefaultAttr->unCCConfig.stCCARIBConfigParam.mt_u32BufferSize = 64*1024;
    }

    return MT_SUCCESS;
}
/**
\brief open cc module. CNcomment: 创建cc实例。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  pstAttr  cc attribution. CNcomment: 创建时传入的解码器属性。CNend
\param[out]  phCC  cc handle. CNcomment: cc模块句柄。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 创建成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 创建失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Create(MT_UNF_CC_PARAM_S *pstCCParam, MT_HANDLE *phCC)
{
    ulong ret = MT_SUCCESS;
    sCcContext* pCcContext = MT_NULL;
    if (MT_NULL == phCC || MT_NULL == pstCCParam)
    {
        return MT_FAILURE;
    }

    if(MT_FALSE == b_init)
    {
        b_init = MT_TRUE;
        if(pstCCParam->stCCAttr.enCCDataType == MT_UNF_CC_DATA_TYPE_608)
        {
            ret = cc_init_vsb(pstCCParam, 1024, EIA_608_708, NULL); 	
        }
        else if(pstCCParam->stCCAttr.enCCDataType == MT_UNF_CC_DATA_TYPE_708)
        {
            ret = cc_init_vsb(pstCCParam, 1024, EIA_DTVCC_708, NULL); 
            pCcContext = (sCcContext *)ret;
            if(MT_NULL !=  pCcContext)
            {
                MT_S32  Ret_CC708 = 0;
                Ret_CC708 = Com_CC708_Create(pstCCParam,&pCcContext->cc708_handle);
                if(Ret_CC708 != MT_SUCCESS)
                {
                    MT_ERR_CC("Com_CC708_Create failure! \n");
                }
            }
        }
        else if(pstCCParam->stCCAttr.enCCDataType == MT_UNF_CC_DATA_TYPE_ARIB)
        {
            ret = cc_init_vsb(pstCCParam, 1024, ARIB_STD24, NULL); 
        }
    }

    *phCC = ret;
	
    return MT_SUCCESS;
}

void MT_NUF_CC_Set_showmode(int show_num)
{
	cc_set_show_mode(show_num);
}

/**
\brief close cc module. CNcomment: 销毁cc实例。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 销毁成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 销毁失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Destroy(MT_HANDLE hCC)
{
    sCcContext* pCcContext = (sCcContext*)hCC;
    if(b_init == MT_TRUE)
    {
        MT_S32  Ret_CC708 = 0;
        Ret_CC708 = Com_CC708_Destroy(pCcContext->cc708_handle);
        if(Ret_CC708 != MT_SUCCESS)
        {
            MT_ERR_CC("Com_CC708_Destroy failure! \n");
        }
        pCcContext->cc708_handle = 0;
        cc_deinit_vsb();        
        b_init = MT_FALSE;
    }
    return MT_SUCCESS;
}

/**
\brief start cc module. CNcomment: 开始cc模块。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Start(MT_HANDLE hCC)
{
    
    sCcContext* pCcContext = (sCcContext*)hCC;    
    setCcDisplayOnOff(pCcContext, MT_TRUE);
    if(MT_NULL != pCcContext)
    {
        MT_S32  Ret_CC708 = 0;
        Ret_CC708 = Com_CC708_Start(pCcContext->cc708_handle); 
        if(Ret_CC708 != MT_SUCCESS)
        {
            MT_ERR_CC("Com_CC708_Start failure! \n");
        }        
    }
    return MT_SUCCESS;
}

/**
\brief stop cc module. CNcomment: 结束cc模块。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Stop(MT_HANDLE hCC)
{
    sCcContext* pCcContext = (sCcContext*)hCC;

    setCcDisplayOnOff(pCcContext, MT_FALSE);
    if(MT_NULL != pCcContext)
    {
        MT_S32  Ret_CC708 = 0;
        Ret_CC708 = Com_CC708_Stop(pCcContext->cc708_handle); 
        if(Ret_CC708 != MT_SUCCESS)
        {
            MT_ERR_CC("Com_CC708_Stop failure! \n");
        }         
    }
    return MT_SUCCESS;
}

/**
\brief reset cc module. CNcomment: 复位cc模块。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Reset(MT_HANDLE hCC)
{
    sCcContext* pCcContext = (sCcContext*)hCC;

    setCcReset(pCcContext);
    if(MT_NULL != pCcContext)
    {
        MT_S32  Ret_CC708 = 0;
        Ret_CC708 = Com_CC708_Reset(pCcContext->cc708_handle); 
        if(Ret_CC708 != MT_SUCCESS)
        {
            MT_ERR_CC("Com_CC708_Reset failure! \n");
        }            
    }
    return MT_SUCCESS;
}

/**
\brief inject mpeg userdata to  cc module. CNcomment: 注入mpeg用户数据到cc模块。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\param[in]  pstUserData  cc userdata structure used in inject cc data. CNcomment: 用户数据结构体。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_InjectUserData(MT_HANDLE hCC, MT_UNF_CC_USERDATA_S *pstUserData)
{
    sCcContext* pCcContext = (sCcContext*)hCC;

    if(pCcContext == MT_NULL_PTR)
    {
        return MT_FAILURE;
    }
    if(getCcDisplayOnOff() == MT_TRUE)
    {
      cc_fifo_put(pstUserData->pu8userdata, pstUserData->u32dataLen);
    }
    return MT_SUCCESS;
}

/**
\brief inject cc pes data to cc module. CNcomment: 注入pes数据到cc模块。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment:模块句柄。CNend
\param[in]  pmt_u8PesData  pes data address. CNcomment: pes数据首地址。CNend
\param[in]  mt_u32DataLen  pes data length. CNcomment: pes数据长度。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_InjectPESData(MT_HANDLE hCC, mt_u8 *pmt_u8PesData, mt_u32 mt_u32DataLen)
{
    sCcContext* pCcContext = (sCcContext*)hCC;

    if(getCcDisplayOnOff() == MT_TRUE)
    {
      if(pmt_u8PesData[0] == 0x00 && pmt_u8PesData[1] == 0x00 && pmt_u8PesData[2] == 0x01 && pmt_u8PesData[3] == 0xbd)
      {
              int pes_len = (pmt_u8PesData[4] << 8) + pmt_u8PesData[5];
              ccParsePesData(pCcContext, &pmt_u8PesData[6],  pes_len);
      }
    }

    return MT_SUCCESS;
}

/**
\brief get cc attribution. CNcomment: 获取cc属性信息。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\param[out]  pstCCAttr  cc attribution structure. CNcomment: 属性信息结构体。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_GetAttr(MT_HANDLE hCC, MT_UNF_CC_ATTR_S *pstCCAttr)
{
    sCcContext* pCcContext = (sCcContext*)hCC;

    if(pstCCAttr->enCCDataType == MT_UNF_CC_DATA_TYPE_608)
    {
        pstCCAttr->unCCConfig.stCC608ConfigParam.enCC608DataType = MT_UNF_CC_608_DATATYPE_CC1;
        pstCCAttr->unCCConfig.stCC608ConfigParam.u32CC608TextColor = pCcContext->ccParam.color;
        pstCCAttr->unCCConfig.stCC608ConfigParam.enCC608TextOpac = MT_UNF_CC_OPACITY_DEFAULT;
        pstCCAttr->unCCConfig.stCC608ConfigParam.u32CC608BgColor = pCcContext->ccParam.bgColor;
        pstCCAttr->unCCConfig.stCC608ConfigParam.enCC608BgOpac = MT_UNF_CC_OPACITY_DEFAULT;
        pstCCAttr->unCCConfig.stCC608ConfigParam.enCC608FontStyle = MT_UNF_CC_FONTSTYLE_DEFAULT;
        pstCCAttr->unCCConfig.stCC608ConfigParam.enCC608DispFormat = MT_UNF_CC_DF_720X480;
        pstCCAttr->unCCConfig.stCC608ConfigParam.bLeadingTailingSpace = MT_FALSE;
    }
    else if(pstCCAttr->enCCDataType == MT_UNF_CC_DATA_TYPE_708)
    {
        pstCCAttr->unCCConfig.stCC708ConfigParam.enCC708ServiceNum = MT_UNF_CC_708_SERVICE1;
        pstCCAttr->unCCConfig.stCC708ConfigParam.enCC708FontName = MT_UNF_CC_FN_DEFAULT;	
        pstCCAttr->unCCConfig.stCC708ConfigParam.enCC708FontStyle = MT_UNF_CC_FONTSTYLE_DEFAULT;	
        pstCCAttr->unCCConfig.stCC708ConfigParam.enCC708FontSize = MT_UNF_CC_FONTSIZE_DEFAULT;	
        pstCCAttr->unCCConfig.stCC708ConfigParam.u32CC708TextColor = pCcContext->ccParam.color;
        pstCCAttr->unCCConfig.stCC708ConfigParam.enCC708TextOpac = MT_UNF_CC_OPACITY_DEFAULT;	
        pstCCAttr->unCCConfig.stCC708ConfigParam.u32CC708BgColor = pCcContext->ccParam.bgColor;
        pstCCAttr->unCCConfig.stCC708ConfigParam.enCC708BgOpac = MT_UNF_CC_OPACITY_DEFAULT;	
        pstCCAttr->unCCConfig.stCC708ConfigParam.u32CC708WinColor = MT_UNF_CC_COLOR_BLACK;	
        pstCCAttr->unCCConfig.stCC708ConfigParam.enCC708WinOpac = MT_UNF_CC_OPACITY_DEFAULT;	
        pstCCAttr->unCCConfig.stCC708ConfigParam.enCC708TextEdgeType = MT_UNF_CC_EDGETYPE_DEFAULT;	
        pstCCAttr->unCCConfig.stCC708ConfigParam.u32CC708TextEdgeColor = MT_UNF_CC_COLOR_BLACK;	
        pstCCAttr->unCCConfig.stCC708ConfigParam.enCC708DispFormat = MT_UNF_CC_DF_720X480;	
    }

    return MT_SUCCESS;
}

/**
\brief set cc attribution. CNcomment:设置cc属性信息。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment. CNcomment: 模块句柄。CNend
\param[in]  pstCCAttr  cc attribution structure. CNcomment:属性信息结构体。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_SetAttr(MT_HANDLE hCC, MT_UNF_CC_ATTR_S *pstCCAttr)
{
    sCcContext* pCcContext = (sCcContext*)hCC;

    if(pstCCAttr->enCCDataType == MT_UNF_CC_DATA_TYPE_608)
    {
        pCcContext->ccParam.color = pstCCAttr->unCCConfig.stCC608ConfigParam.u32CC608TextColor;
        pCcContext->ccParam.bgColor = pstCCAttr->unCCConfig.stCC608ConfigParam.u32CC608BgColor;
    }
    else if(pstCCAttr->enCCDataType == MT_UNF_CC_DATA_TYPE_708)
    {
        pCcContext->ccParam.color = pstCCAttr->unCCConfig.stCC708ConfigParam.u32CC708TextColor;
        pCcContext->ccParam.bgColor = pstCCAttr->unCCConfig.stCC708ConfigParam.u32CC708BgColor;

    }

    return MT_SUCCESS;

}


/**
\brief get cc arib info. CNcomment: 获取arib cc字幕信息。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  arib cc handle. CNcomment: 模块句柄。CNend
\param[out]  pstCCAttr  arib cc info structure. CNcomment:arib cc字幕信息结构体。CNend
\retval ::MT_MT_SUCCESS MT_SUCCESS. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/

/*
**  ATSC CC 数据是通过mpeg-2 video中的user data部分进行传输
**  ARIB CC 数据是在ts流中以独立的es进行传输，有特定的pid
*/
mt_s32 MT_UNF_CC_GetARIBCCInfo(MT_HANDLE hCC,MT_UNF_CC_ARIB_INFO_S *pstCCAribInfo)
{


    return MT_SUCCESS;

}

/** @} */  /** <!-- ==== API declaration end ==== */

