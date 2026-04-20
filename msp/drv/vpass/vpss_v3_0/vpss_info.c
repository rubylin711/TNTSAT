/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "vpss_info.h"
#include "drv_vdec_ext.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#if 1
mt_s32 VPSS_DBG_DbgInit(VPSS_DBG_S *pstDbg)
{
    mt_u32 u32Count;


    pstDbg->stInstDbg.unInfo.u32 = 0;

    pstDbg->stInstDbg.u32LastH = 0;
    pstDbg->stInstDbg.u32LastM = 0;
    pstDbg->stInstDbg.u32LastS = 0;

	/*TEST*/
    //pstDbg->stInstDbg.unInfo.bits.imginfo = 1;

    for(u32Count = 0; u32Count < DEF_MT_DRV_VPSS_PORT_MAX_NUMBER; u32Count++)
    {
        pstDbg->stPortDbg[u32Count].unInfo.u32 = 0;

		pstDbg->stPortDbg[u32Count].u32LastH = 0;
		pstDbg->stPortDbg[u32Count].u32LastM = 0;
		pstDbg->stPortDbg[u32Count].u32LastS = 0;
        //pstDbg->stPortDbg[u32Count].unInfo.bits.frameinfo = 1;
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_DBG_DbgDeInit(VPSS_DBG_S *pstDbg)
{
    return MT_SUCCESS;
}
mt_s32 VPSS_DBG_SendDbgCmd(VPSS_DBG_S *pstDbg,VPSS_DBG_CMD_S *pstCmd)
{
    mt_u32  u32Count;

    VPSS_DBG_PORT_S *pstPortDbg;
    VPSS_DBG_INST_S *pstInstDbg;

	mt_u32 u32Hour = 0;
	mt_u32 u32Minute = 0;
	mt_u32 u32Second = 0;

	(mt_void)VPSS_OSAL_GetCurTime(&u32Hour,&u32Minute,&u32Second);

    switch(pstCmd->enDbgType)
    {
        case DBG_W_YUV:
            switch(pstCmd->hDbgPart)
            {
                case DEF_DBG_SRC_ID:
                    pstInstDbg = &(pstDbg->stInstDbg);
                    pstInstDbg->unInfo.bits.writeyuv = MT_TRUE;
                    break;
                case DEF_DBG_PORT0_ID:
                case DEF_DBG_PORT1_ID:
                case DEF_DBG_PORT2_ID:
                    u32Count = pstCmd->hDbgPart - DEF_DBG_PORT0_ID;
                    pstPortDbg = &(pstDbg->stPortDbg[u32Count]);
                    pstPortDbg->unInfo.bits.writeyuv = MT_TRUE;
                    break;
                default:
                    break;
            }
            break;
        case DBG_W_STREAM:
            switch(pstCmd->hDbgPart)
            {
                case DEF_DBG_SRC_ID:
                    pstInstDbg = &(pstDbg->stInstDbg);
                    pstInstDbg->unInfo.bits.writestream = MT_TRUE;
					pstInstDbg->u32LastH = u32Hour;
					pstInstDbg->u32LastM = u32Minute;
					pstInstDbg->u32LastS = u32Second;
                    break;
                case DEF_DBG_PORT0_ID:
                case DEF_DBG_PORT1_ID:
                case DEF_DBG_PORT2_ID:
                    u32Count = pstCmd->hDbgPart - DEF_DBG_PORT0_ID;
                    pstPortDbg = &(pstDbg->stPortDbg[u32Count]);
                    pstPortDbg->unInfo.bits.writestream = MT_TRUE;
					pstPortDbg->u32LastH = u32Hour;
					pstPortDbg->u32LastM = u32Minute;
					pstPortDbg->u32LastS = u32Second;
                    break;
                default:
                    break;
            }
            break;
        case DBG_INFO_FRM:
            switch(pstCmd->hDbgPart)
            {
                case DEF_DBG_SRC_ID:
                    pstInstDbg = &(pstDbg->stInstDbg);
                    pstInstDbg->unInfo.bits.imginfo = MT_TRUE;
                    break;
                case DEF_DBG_PORT0_ID:
                case DEF_DBG_PORT1_ID:
                case DEF_DBG_PORT2_ID:
                    u32Count = pstCmd->hDbgPart - DEF_DBG_PORT0_ID;
                    pstPortDbg = &(pstDbg->stPortDbg[u32Count]);
                    pstPortDbg->unInfo.bits.frameinfo = MT_TRUE;
                    break;
                default:
                    break;
            }
            break;
        case DBG_INFO_ASP:
            switch(pstCmd->hDbgPart)
            {
                case DEF_DBG_PORT0_ID:
                case DEF_DBG_PORT1_ID:
                case DEF_DBG_PORT2_ID:
                    u32Count = pstCmd->hDbgPart - DEF_DBG_PORT0_ID;
                    pstPortDbg = &(pstDbg->stPortDbg[u32Count]);
                    pstPortDbg->unInfo.bits.asp = MT_TRUE;
                    break;
                default:
                    break;
            }
            break;
        case DBG_INFO_NONE:
            switch(pstCmd->hDbgPart)
            {
                case DEF_DBG_SRC_ID:
                    pstInstDbg = &(pstDbg->stInstDbg);
                    pstInstDbg->unInfo.u32 = 0;
                    break;
                case DEF_DBG_PORT0_ID:
                case DEF_DBG_PORT1_ID:
                case DEF_DBG_PORT2_ID:
                    u32Count =  pstCmd->hDbgPart - DEF_DBG_PORT0_ID;
                    pstPortDbg = &(pstDbg->stPortDbg[u32Count]);
                    pstPortDbg->unInfo.u32 = 0;
                    break;
                default:
                    break;
            }
            break;
        default:
            VPSS_FATAL("Cmd isn't Supported.\n");
    }

    return MT_SUCCESS;
}

mt_s32 VPSS_DBG_ReplyDbgCmd(VPSS_DBG_S *pstDbg,VPSS_DEBUG_E enCmd,mt_void* para1,mt_void* para2)
{
    MT_DRV_VIDEO_FRAME_S *pstFrm;
    mt_u32 u32Count;
    VPSS_HANDLE *phDbgPart;
    mt_u32 u32DbgPart;
    VPSS_DBG_PORT_S *pstPortDbg;
    MT_DRV_VIDEO_FRAME_S stTmpFrm;
    mt_s8 chFile[DEF_FILE_NAMELENGTH];
	mt_u32 u32Hour = 0;
	mt_u32 u32Minute = 0;
	mt_u32 u32Second = 0;

    phDbgPart  = (VPSS_HANDLE *)para1;
    u32DbgPart = (mt_u32)*phDbgPart;

	(mt_void)VPSS_OSAL_GetCurTime(&u32Hour,&u32Minute,&u32Second);

    switch (enCmd)
    {
        case DBG_W_YUV:
            pstFrm = (MT_DRV_VIDEO_FRAME_S *)para2;
            switch (u32DbgPart)
            {
                case DEF_DBG_SRC_ID:
					if (pstDbg->stInstDbg.unInfo.bits.writestream)
				    {
						u32Hour = pstDbg->stInstDbg.u32LastH;
						u32Minute = pstDbg->stInstDbg.u32LastM;
						u32Second = pstDbg->stInstDbg.u32LastS;
					}
                    if (pstDbg->stInstDbg.unInfo.bits.writeyuv
                        || pstDbg->stInstDbg.unInfo.bits.writestream)
                    {
                        mt_osal_snprintf(chFile,
                                DEF_FILE_NAMELENGTH, "vpss_src_%dX%d_%d%d%d.yuv",
                                pstFrm->u32Width,pstFrm->u32Height,u32Hour,u32Minute,u32Second);
                        VPSS_OSAL_WRITEYUV(pstFrm, chFile);
                        if (pstFrm->eFrmType == MT_DRV_FT_FPK)
                        {
                            mt_osal_snprintf(chFile,
                                DEF_FILE_NAMELENGTH, "vpss_src_right_%dX%d_%d%d%d.yuv",
                                pstFrm->u32Width,pstFrm->u32Height,u32Hour,u32Minute,u32Second);
                            memcpy(&stTmpFrm,pstFrm,sizeof(MT_DRV_VIDEO_FRAME_S));
                            memcpy(&(stTmpFrm.stBufAddr[0]),
                                   &(pstFrm->stBufAddr[1]),
                                   sizeof(MT_DRV_VID_FRAME_ADDR_S));
                            VPSS_OSAL_WRITEYUV(&stTmpFrm, chFile);
                        }
                        if (pstDbg->stInstDbg.unInfo.bits.writeyuv)
							pstDbg->stInstDbg.unInfo.bits.writeyuv = MT_FALSE;
                    }

                    break;
                case DEF_DBG_PORT0_ID:
                case DEF_DBG_PORT1_ID:
                case DEF_DBG_PORT2_ID:
                    u32Count = u32DbgPart - DEF_DBG_PORT0_ID;
                    pstPortDbg = &(pstDbg->stPortDbg[u32Count]);

					if (pstPortDbg->unInfo.bits.writestream)
				    {
						u32Hour = pstPortDbg->u32LastH;
						u32Minute = pstPortDbg->u32LastM;
						u32Second = pstPortDbg->u32LastS;
					}
                    if (pstPortDbg->unInfo.bits.writeyuv
                        || pstPortDbg->unInfo.bits.writestream)
                    {
                        mt_osal_snprintf(chFile,
                                DEF_FILE_NAMELENGTH, "vpss_p%d_%dX%d_%02d%02d%02d.yuv",u32Count,
                                pstFrm->u32Width, pstFrm->u32Height,u32Hour,u32Minute,u32Second);
                        VPSS_OSAL_WRITEYUV(pstFrm, chFile);
                        if (pstFrm->eFrmType == MT_DRV_FT_FPK)
                        {
                            mt_osal_snprintf(chFile,
                                DEF_FILE_NAMELENGTH, "vpss_p%d_right_%dX%d_%02d%02d%02d.yuv",u32Count,
                                pstFrm->u32Width, pstFrm->u32Height,u32Hour,u32Minute,u32Second);
                            memcpy(&stTmpFrm,pstFrm,sizeof(MT_DRV_VIDEO_FRAME_S));
                            memcpy(&(stTmpFrm.stBufAddr[0]),
                                   &(pstFrm->stBufAddr[1]),
                                   sizeof(MT_DRV_VID_FRAME_ADDR_S));
                            VPSS_OSAL_WRITEYUV(&stTmpFrm, chFile);
                        }
                        if (pstPortDbg->unInfo.bits.writeyuv)
                            pstPortDbg->unInfo.bits.writeyuv = MT_FALSE;
                    }
                    break;
                default:
                    VPSS_FATAL("Invalid para2 %#x\n",u32DbgPart);
                    break;
            }
            break;
        case DBG_INFO_FRM:
            pstFrm = (MT_DRV_VIDEO_FRAME_S *)para2;
            switch (u32DbgPart)
            {
                case DEF_DBG_SRC_ID:
                    if (pstDbg->stInstDbg.unInfo.bits.imginfo)
                    {
                        MT_DRV_VIDEO_PRIVATE_S *pstPriv;
                        MT_VDEC_PRIV_FRAMEINFO_S *pstVdecPriv;
                        pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstFrm->u32Priv[0]);
                        pstVdecPriv = (MT_VDEC_PRIV_FRAMEINFO_S *)&(pstPriv->u32Reserve[0]);


                        MT_PRINT("Image Info:Index %d Type %d Format %d W %d H %d Prog %d FieldMode %d PTS %d Rate %d LastFlag %#x Delta %d CodeType %d,SourceType %d,BitWidth %d\n"
                                 "           L:Y %#x C %#x YH %#x CH %#x YS %d CS %d \n"
                                 "           R:Y %#x C %#x YH %#x CH %#x YS %d CS %d \n",
                                pstFrm->u32FrameIndex,
                                pstFrm->eFrmType,
                                pstFrm->ePixFormat,
                                pstFrm->u32Width,
                                pstFrm->u32Height,
                                pstFrm->bProgressive,
                                pstFrm->enFieldMode,
                                pstFrm->u32Pts,
                                pstFrm->u32FrameRate,
                                pstPriv->u32LastFlag,
                                pstVdecPriv->s32InterPtsDelta,
                                pstVdecPriv->entype,
                                pstPriv->stVideoOriginalInfo.enSource,
                                pstFrm->enBitWidth,
                                pstFrm->stBufAddr[0].u32PhyAddr_Y,
                                pstFrm->stBufAddr[0].u32PhyAddr_C,
                                pstFrm->stBufAddr[0].u32PhyAddr_YHead,
                                pstFrm->stBufAddr[0].u32PhyAddr_CHead,
                                pstFrm->stBufAddr[0].u32Stride_Y,
                                pstFrm->stBufAddr[0].u32Stride_C,
                                pstFrm->stBufAddr[1].u32PhyAddr_Y,
                                pstFrm->stBufAddr[1].u32PhyAddr_C,
                                pstFrm->stBufAddr[1].u32PhyAddr_YHead,
                                pstFrm->stBufAddr[1].u32PhyAddr_CHead,
                                pstFrm->stBufAddr[1].u32Stride_Y,
                                pstFrm->stBufAddr[1].u32Stride_C);
                    }
                    break;
                case DEF_DBG_PORT0_ID:
                case DEF_DBG_PORT1_ID:
                case DEF_DBG_PORT2_ID:

                    u32Count = u32DbgPart - DEF_DBG_PORT0_ID;
                    pstPortDbg = &(pstDbg->stPortDbg[u32Count]);
                    if (pstPortDbg->unInfo.bits.frameinfo)
                    {
                        MT_DRV_VIDEO_PRIVATE_S *pstPriv;
                        pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstFrm->u32Priv[0]);

                        MT_PRINT("Frame Info:Index %d Type %d Format %d W %d H %d LW %d LH %d PTS %d Rate %d Cnt %d Fidelity %d u32LastFlag %d,oriField %d,BitWidth %d\n",
                                pstFrm->u32FrameIndex,
                                pstFrm->eFrmType,
                                pstFrm->ePixFormat,
                                pstFrm->u32Width,
                                pstFrm->u32Height,
                                pstFrm->stLbxInfo.s32Width,
                                pstFrm->stLbxInfo.s32Height,
                                pstFrm->u32Pts,
                                pstFrm->u32FrameRate,
                                pstPriv->u32FrmCnt,
                                pstPriv->u32Fidelity,
                                pstPriv->u32LastFlag,
                                pstPriv->eOriginField,
                                pstFrm->enBitWidth);
                    }
                    break;
                default:
                    VPSS_FATAL("Invalid para2 %#x\n",u32DbgPart);
                    break;
            }
            break;
        case DBG_INFO_ASP:

            break;
        default:
            VPSS_FATAL("Invalid para1 cmd=%#x\n",enCmd);
            break;
    }

    return MT_SUCCESS;
}
#endif
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

