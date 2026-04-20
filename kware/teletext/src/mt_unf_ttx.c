#include "mt_type.h"
#include "mt_unf_ttx.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "teletext_debug.h"
#include "teletext_data.h"
//#include "teletext_parse.h"
#include "mt_type.h"
#include "mt_unf_ttx.h"
#include "mt_type.h"
#include "sys_define.h"
#include "vbi_api.h"
#include "mt_unf_disp.h"

static MT_HANDLE s_hVBI = 0xffffff;//extern MT_HANDLE s_hVBI;
typedef struct tagTTX_LOCAL_PARAM_S
{
    MT_HANDLE hDataRecv[TTX_ITEM_MAX_NUM];

    pthread_mutex_t stTtxMutex;

    MT_UNF_TTX_ITEM_S astItems[TTX_ITEM_MAX_NUM]; /* all items of teletext */
    mt_u8  u8TtxItemNum;            /* the total item number */
    ulong u32UserData;
		MT_BOOL osdTtxEn;
		MT_BOOL vbiTtxEn;
		mt_u32 pageNum;
		MT_UNF_TTX_TYPE_E      enType;
} TTX_LOCAL_PARAM_S;

extern RET_CODE vbi_set_callback(MT_UNF_TTX_CB_FN pfnCB);


mt_s32 MT_UNF_TTX_DataRecv_Create(MT_UNF_TTX_PARAM_S *stTtxParam, MT_HANDLE *g_hTTX)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u8  i = 0;
    TTX_LOCAL_PARAM_S* pstParam = MT_NULL;

    if (MT_NULL == g_hTTX || MT_NULL == stTtxParam)
    {
        MT_ERR_TTX("params invalid\n");

        return MT_FAILURE;
    }

    *g_hTTX = 0;

    if (stTtxParam->u8TtxItemNum > TTX_ITEM_MAX_NUM)
    {
        MT_ERR_TTX("item num > %d\n", TTX_ITEM_MAX_NUM);

        return MT_FAILURE;
    }

    pstParam = (TTX_LOCAL_PARAM_S*)malloc(sizeof(TTX_LOCAL_PARAM_S));
    if (MT_NULL == pstParam)
    {
        MT_ERR_TTX("malloc failure...\n");

        return MT_FAILURE;
    }

    memset(pstParam, 0, sizeof(TTX_LOCAL_PARAM_S));
    pstParam->u32UserData = stTtxParam->u32UserData;

    if (pthread_mutex_init(&pstParam->stTtxMutex, MT_NULL))
    {
        free((void*)pstParam);
        return MT_FAILURE;
    }

    memcpy(pstParam->astItems, stTtxParam->astItems, sizeof(MT_UNF_TTX_ITEM_S)*TTX_ITEM_MAX_NUM);
    pstParam->u8TtxItemNum = stTtxParam->u8TtxItemNum;

    for (i = 0; i < pstParam->u8TtxItemNum; i++)
    {
        s32Ret |= TTX_DataRecv_Create(&pstParam->hDataRecv[i]);
    }
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_TTX("failed to TTX_DataRecv_Create\n");

        goto ERROR;
    }

    MT_INFO_TTX("success with handle:0x%08x!\n", *g_hTTX);

		pstParam->enType = MT_UNF_TTX_TTXSUBT_BUTT;		
    *g_hTTX = (MT_HANDLE)pstParam;
    return MT_SUCCESS;

ERROR:

    (mt_void)pthread_mutex_destroy(&pstParam->stTtxMutex);
    for (i = 0; i < pstParam->u8TtxItemNum; i++)
    {
        if (pstParam->hDataRecv[i])
        {
            s32Ret = TTX_DataRecv_Destroy(pstParam->hDataRecv[i]);
        }
    }
    free((void*)pstParam);

    *g_hTTX = 0;

    return MT_FAILURE;
}

mt_s32 MT_UNF_TTX_DataRecv_Destroy(MT_HANDLE g_hTTX)
{
    mt_s32 s32Ret = MT_SUCCESS;
    TTX_LOCAL_PARAM_S* pstParam = (TTX_LOCAL_PARAM_S*)g_hTTX;
    mt_u8 i = 0;

    if (TTX_INVALID_HANDLE == g_hTTX)
    {
        MT_ERR_TTX("param invalid!\n");
        return MT_FAILURE;
    }

    for (i = 0; i < pstParam->u8TtxItemNum; i++)
    {
        if (pstParam->hDataRecv[i])
        {
            s32Ret |= TTX_DataRecv_Destroy(pstParam->hDataRecv[i]);
            pstParam->hDataRecv[i] = 0;
        }
    }

    if (s32Ret != MT_SUCCESS)
    {
        MT_WARN_TTX("failed to TTX_DataRecv_Destroy...\n");
    }

    (mt_void)pthread_mutex_destroy(&pstParam->stTtxMutex);

    free((void*)pstParam);

    return MT_SUCCESS;
}

MT_S32 MT_UNF_TTX_Init(MT_VOID)
{	
	vbi_init_vsb();
    return 0;
}

MT_S32 MT_UNF_TTX_DeInit(MT_VOID)
{
    vbi_ttx_stop_vsb(); 
    vbi_deinit_vsb();
    return 0;
}

MT_S32 MT_UNF_TTX_Create(MT_UNF_TTX_INIT_PARA_S* pstInitParam, MT_HANDLE* phTTX)
{
	MT_UNF_DISP_VBI_CFG_S stVBICfg = {0};
	ttx_font_src_t stFontSrc = {0};

	vbi_set_callback(pstInitParam->pfnCB);

	stFontSrc.p_pal_font   = (u8*)pstInitParam->p_font->p_pal_font;
	stFontSrc.p_ntsl_font  = (u8*)pstInitParam->p_font->p_ntsl_font;
	stFontSrc.p_small_font = (u8*)pstInitParam->p_font->p_small_font;
	stFontSrc.p_hd_font    = (u8*)pstInitParam->p_font->p_hd_font;
	//vbi_set_font_src(pstInitParam->p_font);
	vbi_set_font_src(&stFontSrc);

	switch(pstInitParam->font_size)
	{
		case MT_UNF_TTX_FONT_SD:
			vbi_set_font_size_vsb(TTX_FONT_NORMAL);
			break;
		case MT_UNF_TTX_FONT_HD:
			vbi_set_font_size_vsb(TTX_FONT_HD);
			break;
		default:
			vbi_set_font_size_vsb(TTX_FONT_NORMAL);
			break;
	}
	stVBICfg.enType = MT_UNF_DISP_VBI_TYPE_TTX;
	MT_UNF_DISP_CreateVBI(MT_UNF_DISPLAY0,&stVBICfg,&s_hVBI);

  return 0;
}

MT_S32 MT_UNF_TTX_Destroy(MT_HANDLE hTTX)
{
	MT_UNF_DISP_DestroyVBI(s_hVBI);
    return 0;
}

MT_S32 MT_UNF_TTX_InjectData(MT_HANDLE g_hTTX, mt_u32 u32TtxPID, mt_u8* pu8Data, mt_u32 u32DataSize)
{
    mt_u8 i = 0;
    mt_s32 s32Ret = MT_FAILURE;
		
		MT_UNF_DISP_VBI_DATA_S stVBIData = {0};

    TTX_LOCAL_PARAM_S* pstParam = (TTX_LOCAL_PARAM_S*)g_hTTX;
    if (MT_NULL == pu8Data || 0 == u32DataSize)
    {
        printf("param invalid!\n");

        return MT_FAILURE;
    }
//    printf("linda debug %s %d\n", __func__, __LINE__);
 

//    pthread_mutex_lock(&pstParam->stTtxMutex);


    for (i = 0; i < pstParam->u8TtxItemNum; i++)
    {
        //printf("u32TtxPID = %d pstParam->astItems[%d].u32TtxPID = %d\n",u32TtxPID,i,pstParam->astItems[i].u32TtxPID);
        if (pstParam->astItems[i].u32TtxPID == u32TtxPID)
        {
            if (pstParam->hDataRecv[i])
            {
                //printf("linda debug %s %d\n", __func__, __LINE__);
                s32Ret = TTX_DataRecv_Inject(pstParam->hDataRecv[i], pu8Data, u32DataSize);
                if (s32Ret != MT_SUCCESS)
                {
                    MT_ERR_TTX("failed to TTX_DataRecv_Inject\n");
                }
            }
        }
//        printf("linda debug %s %d\n", __func__, __LINE__);
    }
		
	 stVBIData.enType = MT_UNF_DISP_VBI_TYPE_TTX;
	 stVBIData.pu8DataAddr = pu8Data;
	 stVBIData.u32DataLen = u32DataSize;
	 if(0xffffff != s_hVBI)
	 {
			 s32Ret = MT_UNF_DISP_SendVBIData(s_hVBI,&stVBIData);
	 }

   // pthread_mutex_unlock(&pstParam->stTtxMutex);
//    printf("linda debug %s %d\n", __func__, __LINE__);
    return s32Ret;
}

MT_S32 MT_UNF_TTX_ResetData(MT_HANDLE hTTX)
{
    return 0;
}

MT_S32 MT_UNF_TTX_SwitchContent (MT_HANDLE hTTX, MT_UNF_TTX_CONTENT_PARA_S* pstContentParam)
{
	mt_u32 page_num = 0;
    mt_u8 language0 = 0, language1 = 0, language2 = 0;    
    
	TTX_LOCAL_PARAM_S* pstParam = (TTX_LOCAL_PARAM_S*)hTTX;
#if 0	
	printf("\r\n ~~~~~~~~~teletext type:%d,u8MagazineNum:%d,u8PageNum:%d,u16PageSubCode:%d", 
		pstContentParam->enType,pstContentParam->stInitPgAddr.u8MagazineNum,pstContentParam->stInitPgAddr.u8PageNum,
		pstContentParam->stInitPgAddr.u16PageSubCode);
#endif
    if((pstContentParam == NULL) || (pstParam == NULL))
    {
        return MT_ERR_PARAM;
    }
    
	if(pstContentParam->enType != MT_UNF_TTX_INITTTX && pstContentParam->enType != MT_UNF_TTX_TTXSUBT 
		&& pstContentParam->enType != MT_UNF_TTX_TTXSUBT_HIP)
	{
	     return 1;
	}
    if(pstContentParam->u8ForceStop == 0)
    {
	    vbi_ttx_stop_vsb();		
    }
    if(pstContentParam && ((pstContentParam->u16MaxPageNum != 0) ||  
       (pstContentParam->u16MaxSubPageNum != 0)))
    {
        vbi_ttx_start_vsb(pstContentParam->u16MaxPageNum, pstContentParam->u16MaxSubPageNum, MT_NULL);
    }
    else
    {
	    vbi_ttx_start_vsb(100, 100, MT_NULL);
    }
    language0= (mt_u8)((pstContentParam->u32ISO639LanCode>>16)&0xFF);
    language1= (mt_u8)((pstContentParam->u32ISO639LanCode>>8)&0xFF);
    language2= (mt_u8)(pstContentParam->u32ISO639LanCode & 0xFF);
    vbi_set_language_code_vsb(language0, language1, language2);	
    vbi_set_ttx_page_draw_mode(pstContentParam->u8PageDrawMode);
	page_num = (pstContentParam->stInitPgAddr.u8MagazineNum * 256) + pstContentParam->stInitPgAddr.u8PageNum;
	pstParam->enType = pstContentParam->enType;
	pstParam->pageNum = page_num;

  return 0;
}

MT_S32 MT_UNF_TTX_Display(MT_HANDLE hTTX, MT_HANDLE hDispalyHandle)
{
    return 0;
}

MT_S32 MT_UNF_TTX_Output (MT_HANDLE hTTX, MT_UNF_TTX_OUTPUT_E enOutput, MT_BOOL bEnable)
{
    TTX_LOCAL_PARAM_S* pstParam = (TTX_LOCAL_PARAM_S*)hTTX;
    MT_UNF_DISP_VBI_CFG_S stVBICfg = {0};
//    static MT_HANDLE s_hVBI = 0xffffff;

    if(enOutput == MT_UNF_TTX_OSD_OUTPUT || enOutput == MT_UNF_TTX_DUAL_OUTPUT)
    {
        pstParam->osdTtxEn = bEnable;				
        if(bEnable == MT_FALSE)
        {
             vbi_ttx_hide_vsb(MT_TRUE);
        }
        else
        {
            if(pstParam->enType == MT_UNF_TTX_TTXSUBT ||  pstParam->enType == MT_UNF_TTX_TTXSUBT_HIP)
            {
                vbi_ttx_show_vsb(MT_TRUE, pstParam->pageNum);
            }
            else
            {
                vbi_ttx_show_vsb(MT_FALSE, pstParam->pageNum);
            }				
        }
    }
    if(enOutput == MT_UNF_TTX_VBI_OUTPUT || enOutput == MT_UNF_TTX_DUAL_OUTPUT)
    {
        if(bEnable == MT_FALSE)
        {
            MT_UNF_DISP_DestroyVBI(s_hVBI);
        }
        else
        {
            stVBICfg.enType =  MT_UNF_DISP_VBI_TYPE_TTX;
            MT_UNF_DISP_CreateVBI(MT_UNF_DISPLAY0,&stVBICfg,&s_hVBI);
        }
    }
    return 0;
}

MT_S32 MT_UNF_TTX_ExecCmd(MT_HANDLE hTTX,
                          MT_UNF_TTX_CMD_E enCMD, MT_VOID *pvCMDParam)
{
	ttx_key_t key = TTX_KEY_0;
	MT_UNF_TTX_KEY_E mt_key = *((MT_UNF_TTX_KEY_E *)pvCMDParam);
	if(enCMD == MT_UNF_TTX_CMD_KEY)
	{
		switch(mt_key)
		{
		   case MT_UNF_TTX_KEY_0:
		   	key = TTX_KEY_0;
			break;
		   case MT_UNF_TTX_KEY_1:
		   	key = TTX_KEY_1;
			break;		   	
		   case MT_UNF_TTX_KEY_2:
		   	key = TTX_KEY_2;
			break;		   	
		   case MT_UNF_TTX_KEY_3:
		   	key = TTX_KEY_3;
			break;			
		   case MT_UNF_TTX_KEY_4:
		   	key = TTX_KEY_4;
			break;		   	
		   case MT_UNF_TTX_KEY_5:
		   	key = TTX_KEY_5;
			break;		   	
		   case MT_UNF_TTX_KEY_6:
		   	key = TTX_KEY_6;
			break;		   	
		   case MT_UNF_TTX_KEY_7:
		   	key = TTX_KEY_7;
			break;		   	
		   case MT_UNF_TTX_KEY_8:
		   	key = TTX_KEY_8;
			break;		   	
		   case MT_UNF_TTX_KEY_9:
		   	key = TTX_KEY_9;
			break;		   	
		   case MT_UNF_TTX_KEY_PREVIOUS_PAGE:
		   	key = TTX_KEY_DOWN;
			break;		   	
		   case MT_UNF_TTX_KEY_NEXT_PAGE:
		   	key = TTX_KEY_UP;
			break;			   	
		   case MT_UNF_TTX_KEY_RED:
		    key = TTX_KEY_RED;
			break;
		   case MT_UNF_TTX_KEY_GREEN:
		    key = TTX_KEY_GREEN;
			break;		   	
		   case MT_UNF_TTX_KEY_YELLOW:
		    key = TTX_KEY_YELLOW;
			break;			   	
		   case MT_UNF_TTX_KEY_CYAN:
		    key = TTX_KEY_CYAN;
			break;		   	
		   case MT_UNF_TTX_KEY_INDEX:
		    key = TTX_KEY_INDEX;
			break;		   	
		   case MT_UNF_TTX_KEY_REVEAL:
		    key = TTX_KEY_CONCEAL;
			break;			   	
		   case MT_UNF_TTX_KEY_HOLD:
		   	key = TTX_KEY_HOLD;
			break;
		   case MT_UNF_TTX_KEY_MIX:
		   	key = TTX_KEY_MIX;
			break;
		   case MT_UNF_TTX_KEY_UPDATE:
		    key = TTX_KEY_INDEX;
			break;			   	
		   case MT_UNF_TTX_KEY_PREVIOUS_SUBPAGE:
		   	key = TTX_KEY_LEFT;
			break;		   	
		   case MT_UNF_TTX_KEY_NEXT_SUBPAGE:
		   	key = TTX_KEY_RIGHT;
			break;		   	
		   case MT_UNF_TTX_KEY_PREVIOUS_MAGAZINE:
		   	key = TTX_KEY_PAGE_DOWN;
			break;			   	
		   case MT_UNF_TTX_KEY_NEXT_MAGAZINE:	
 		    key = TTX_KEY_PAGE_UP;
 		    break;  
           case MT_UNF_TTX_KEY_TRANSPARENT:
				key = TTX_KEY_TRANSPARENT;
			break;
           default:
                key = 0xFFFFFFFF; //invalid key
            break;
        }
		vbi_post_ttx_key_vsb(key);
	}
    return 0;
}


