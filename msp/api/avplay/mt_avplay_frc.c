/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_avplay_frc.c
  Version       : Initial Draft
  Author        : Montage software group
  Created       : 2015/11/25
  Description   : Common definitions of MT_CODEC(video).
                  The codec wants to register to MT_CODEC need to adapt to MT_CODEC_S.
  History       :
  1.Date        : 2015/11/25
    Author      :
    Modification: Created file

*******************************************************************************/

#include <string.h>
#include <sys/time.h>
#include <sys/times.h>
#include <pthread.h>

#include "mt_avplay_frc.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

mt_s32 AVPLAY_FrcCreate(AVPLAY_S *pAvplay)
{
    /*init frc*/
    memset(&pAvplay->FrcCalAlg, 0, sizeof(AVPLAY_ALG_FRC_S));

    pAvplay->FrcCtrlInfo.s32FrmState = 0;

    pAvplay->FrcParamCfg.u32InRate = 25;
    pAvplay->FrcParamCfg.u32OutRate = 25;
    pAvplay->FrcParamCfg.u32PlayRate = AVPLAY_ALG_FRC_BASE_PLAY_RATIO;

    return MT_SUCCESS;
}

mt_s32 AVPLAY_FrcDestroy(AVPLAY_S *pAvplay)
{
    memset(&pAvplay->FrcCalAlg, 0, sizeof(AVPLAY_ALG_FRC_S));

    return MT_SUCCESS;
}

mt_s32 AVPLAY_FrcReset(AVPLAY_ALG_FRC_S *hFrc)
{
    if (hFrc == MT_NULL)
    {
       return MT_FAILURE;
    }
    memset(hFrc, 0, sizeof(AVPLAY_ALG_FRC_S));

    return MT_SUCCESS;
}

mt_void AVPLAY_FrcCalculate(AVPLAY_ALG_FRC_S *hFrc, AVPLAY_FRC_CFG_S *pstFrcCfg, AVPLAY_FRC_CTRL_S *pstFrcCtrl)
{
	mt_u32 quot, remder, u32InRate, u32OutRate, u32tmpFrameRate;
	mt_s32 flag = 0;
	mt_u64 tmp = 0;
	mt_u64 remder_cyc;
	mt_u32 u32PlayRate;
	mt_u32  u32CurID;     /* current insert or drop position in a FRC cycle, add by b00182026*/

    u32tmpFrameRate = pstFrcCfg->u32InRate;

    //if (u32tmpFrameRate <= 60) 	/* to support stream which rate is less tha 6 frame per second/ */
    if (u32tmpFrameRate < 1)
    {
    	pstFrcCtrl->s32FrmState = 0;
    	return;
    }

    if ( (hFrc->u32OutRate != pstFrcCfg->u32OutRate)
	    ||(hFrc->u32PlayRate != pstFrcCfg->u32PlayRate)			/*add by b00182026*/
	    ||(hFrc->u32InRate != u32tmpFrameRate)
	    )
    {
	    //RESET
	    AVPLAY_FrcReset(hFrc);
    	hFrc->u32InRate = u32tmpFrameRate;
    	hFrc->u32OutRate = pstFrcCfg->u32OutRate;
    	hFrc->u32PlayRate = pstFrcCfg->u32PlayRate;
    	hFrc->u32CurID = 1;		/*this should be 0 or 1 ?*/
    }

    u32PlayRate = pstFrcCfg->u32PlayRate; //8bit fraction

    u32OutRate = (hFrc->u32OutRate)*AVPLAY_ALG_FRC_PRECISION*AVPLAY_ALG_FRC_BASE_PLAY_RATIO/u32PlayRate; 	//support  256X playrate
	u32InRate = (hFrc->u32InRate)*AVPLAY_ALG_FRC_PRECISION;

    u32CurID = hFrc->u32CurID;

  // printf("inr=%d, outr=%d, playr=%d, ID=%d  ",u32InRate, u32OutRate, hFrc->u32PlayRate, hFrc->u32CurID);

	if((u32InRate) < (u32OutRate))  ////Lower framerate to Higher framerate ---need repeat
	{
	    quot = (u32OutRate)/(u32InRate);
	    remder = (u32OutRate)%(u32InRate);
	    remder_cyc = (mt_u64)(hFrc->u32InputCount%u32InRate + 1);

	    flag = 0; //if remder==0
	    if(remder==0)
	    {
		    flag = 0;
	    }
	    else
	    {
		tmp = (mt_u64)(((mt_u64)u32InRate*(mt_u64)u32CurID + remder/2) / remder);
		if (tmp == remder_cyc)
		{
		    flag = 1;
		    hFrc->u32CurID++;
		    hFrc->u32CurID = (hFrc->u32CurID%remder == 0)? remder: (hFrc->u32CurID%remder);
		}
		else
		{
		    flag = 0;
		}
	    }

	    if(flag==1)
	    {
    		//repeat time: (quot-1)+1;
    		pstFrcCtrl->s32FrmState = (mt_s32)quot;
	    }
	    else
	    {
    		//repeat time: (quot-1);
    		pstFrcCtrl->s32FrmState = (mt_s32)(quot - 1);
	    }

	}
	else if(u32InRate > u32OutRate) //Higher framerate to Lower framerate ---need drop
	{
	    quot = (u32InRate)/(u32OutRate);
	    remder = (u32InRate)-(u32OutRate);
	    remder_cyc = (mt_u64)(hFrc->u32InputCount%u32InRate + 1);

	    flag = 0; //if remder==0
	    if(remder==0)
	    {
    		flag =0;
	    }
	    else
	    {
    		tmp = (mt_u64)(((mt_u64)u32InRate*(mt_u64)u32CurID + remder/2) / remder);
    		if(tmp == remder_cyc)
    		{
    		    flag = 1;
    		    hFrc->u32CurID++;
    		    hFrc->u32CurID = (hFrc->u32CurID%remder == 0)? remder: (hFrc->u32CurID%remder);
    		}
    		else
    		{
    		    flag = 0;
    		}
	    }

	    //if flag==1, need drop this frame.
	    if(flag==1)
	    {
    		pstFrcCtrl->s32FrmState = -1;
	    }
	    else
	    {
    		pstFrcCtrl->s32FrmState = 0;
	    }
	}
	else  //don't need frame rate conversion
	{
	    pstFrcCtrl->s32FrmState = 0;
	}

    hFrc->u32InputCount++;

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


