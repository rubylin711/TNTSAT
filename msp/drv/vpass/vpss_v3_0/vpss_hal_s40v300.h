/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_HAL_S40V300_H__
#define __VPSS_HAL_S40V300_H__
#include"mt_type.h"

#include "mt_drv_mmz.h"

#include"vpss_common.h"

#include "mt_drv_vpss.h"
#include "vpss_reg_s40v300.h"
#include "vpss_sttinf.h"
#include "vpss_wbc.h"
#include "vpss_rwzb.h"
#include "vpss_his.h"
#include "drv_pq_ext.h"

#define  VPSS_ZME_HPREC        (1<<20)
#define  VPSS_ZME_HPREC_F        (1<<19)
#define  VPSS_ZME_HPREC_B        (1<<1)

#define  VPSS_ZME_VPREC        (1<<12)

#define HAL_VERSION_3798M 0x300

#define VPSS0_BASE_ADDR  0xf8cb0000
#define VPSS1_BASE_ADDR  0xffffffff
#define VPSS_REG_SIZE    0x5000

#define VPSS_ZME_COEF_SIZE (512)

#define VPSS_ZME_COEF_NUM 16

#define VPSS_REG_SIZE_CALC(start, end)\
    (offsetof(VPSS_REG_S, end) + sizeof(mt_u32) -\
     offsetof(VPSS_REG_S, start))

#define VPSS_HAL_CHECK_IP_VAILD(enIP) \
do{\
    if((enIP != VPSS_IP_0)&&(enIP != VPSS_IP_1))\
    {\
        VPSS_ERROR("VPSS IP%d, is Not Vaild\n", enIP);\
        return MT_FAILURE;\
    }\
}while(0)

#define VPSS_HAL_CHECK_NODE_ID_VAILD(enTaskNodeId) \
do{\
    if(enTaskNodeId >= VPSS_HAL_TASK_NODE_BUTT)\
    {\
        VPSS_ERROR("VPSS NODE ID%d, is Not Vaild\n", enTaskNodeId);\
        return MT_FAILURE;\
    }\
}while(0)

#define VPSS_HAL_CHECK_INIT(bInit) \
do{\
    if (MT_FALSE == bInit){\
        VPSS_ERROR("VPSS HAL Is Not Init\n");\
        return MT_FAILURE;\
    }\
}while(0)


#define VPSS_HAL_CHECK_NULL_PTR(ptr) \
do{\
    if (MT_NULL == ptr){\
        VPSS_ERROR("pointer is NULL!\n");\
        return MT_FAILURE;\
    }\
}while(0)


typedef enum hiVPSS_IP_E
{
    VPSS_IP_0 = 0,
    VPSS_IP_1,
    VPSS_IP_BUTT
}VPSS_IP_E;

typedef enum hiVPSS_HAL_TASK_NODE_E
{
    VPSS_HAL_TASK_NODE_2D_FIELD = 0,
    VPSS_HAL_TASK_NODE_2D,
    VPSS_HAL_TASK_NODE_3D_R,
    VPSS_HAL_TASK_NODE_3DDET,
    VPSS_HAL_TASK_NODE_P0_ZME_L2,
    VPSS_HAL_TASK_NODE_P1_ZME_L2,
    VPSS_HAL_TASK_NODE_P2_ZME_L2,
    VPSS_HAL_TASK_NODE_P0_RO_Y,
    VPSS_HAL_TASK_NODE_P0_RO_C,
    VPSS_HAL_TASK_NODE_P1_RO_Y,
    VPSS_HAL_TASK_NODE_P1_RO_C,
    VPSS_HAL_TASK_NODE_P2_RO_Y,
    VPSS_HAL_TASK_NODE_P2_RO_C,
    VPSS_HAL_TASK_NODE_P0_RO,
    VPSS_HAL_TASK_NODE_P1_RO,
    VPSS_HAL_TASK_NODE_P2_RO,
    VPSS_HAL_TASK_NODE_BUTT
}VPSS_HAL_TASK_NODE_E;


typedef struct hiVPSS_HAL_CTX_S
{
    MT_BOOL  bInit;
    MT_BOOL  bClockEn;
    mt_u32   u32LogicVersion;

    mt_u32   u32BaseRegPhy;
    mt_u32   u32BaseRegVir;

    mt_u32   au32AppPhy[VPSS_HAL_TASK_NODE_BUTT];
    mt_u32   au32AppVir[VPSS_HAL_TASK_NODE_BUTT];
    mmz_buffer_s stRegBuf;

    MT_BOOL  abUsed[VPSS_ZME_COEF_NUM];
    mt_u32   au32ZmeCoefPhy[VPSS_ZME_COEF_NUM][4];
    mt_u32   au32ZmeCoefVir[VPSS_ZME_COEF_NUM][4];
    mmz_buffer_s stZmeCoefBuf;
} VPSS_HAL_CTX_S;


typedef struct hiVPSS_HAL_ZME_PARAM_S
{
    MT_BOOL bYUV;
    mt_u32 u32YHRatio;
    mt_u32 u32CHRatio;
    mt_u32 u32YVRatio;
    mt_u32 u32CVRatio;
    ZME_FORMAT_E enInFmt;
    ZME_FORMAT_E enOutFmt;
}VPSS_HAL_ZME_PARAM_S;

typedef struct hiVPSS_HAL_FRAME_S{
    MT_DRV_FRAME_TYPE_E      eFrmType;
    mt_u32 u32Width;
    mt_u32 u32Height;
    MT_DRV_PIX_FORMAT_E enFormat;
    MT_DRV_FIELD_MODE_E enFieldMode;
    MT_BOOL bProgressive;
    MT_DRV_VID_FRAME_ADDR_S stAddr;
    MT_BOOL                  bCompressd;
    MT_DRV_PIXEL_BITWIDTH_E  enBitWidth;
    mt_u32 u32TunnelAddr;
    MT_BOOL  bTopFirst;
}VPSS_HAL_FRAME_S;


typedef struct hiVPSS_HAL_PORT_INFO_S
{
    MT_BOOL    bEnable;
    MT_RECT_S  stInCropRect; /* PORT CROP信息 */
    MT_RECT_S  stVideoRect; /* 真实显示区域 */
    MT_DRV_VPSS_ROTATION_E enRotation; /* 旋转信息 */
    MT_BOOL bNeedFlip;
    MT_BOOL bNeedMirror;

    VPSS_HAL_FRAME_S stOutInfo; /* PORT输出信息 */
} VPSS_HAL_PORT_INFO_S;


typedef struct hiVPSS_DIE_INFO_S
{
	VPSS_DIESTCFG_S stDieStCfg;
    MT_BOOL bBottom_first;

	//:TODO: 插值选择信息

}VPSS_DIE_INFO_S;

typedef struct hiVPSS_NR_INFO_S
{
    MT_BOOL bNrEn;
	VPSS_NRMADCFG_S stNrMadCfg;
}VPSS_NR_INFO_S;

typedef struct hiVPSS_CCCL_INFO_S
{
    MT_BOOL bCCCLEn;
	VPSS_HAL_FRAME_S stInRefInfo[2];
	VPSS_CCCLCNTCFG_S stCCCLCntCfg;
}VPSS_CCCL_INFO_S;


typedef enum hiVPSS_HAL_NODE_TYPE_E
{
    VPSS_HAL_NODE_2D_FRAME = 0,
    VPSS_HAL_NODE_2D_5Field,
    VPSS_HAL_NODE_2D_3Field,
    VPSS_HAL_NODE_2D_Field,
    VPSS_HAL_NODE_3D_FRAME_R, //用于配置读取偏移，在解码源为SBS/TAB，暂时不考虑拆分之后还有隔行的情况
    VPSS_HAL_NODE_PZME, //对应隔行，单场的源的类型
    VPSS_HAL_NODE_UHD, // 4K*2K场景，后面看是否有UHD非标的特殊场景，再增加类型
    VPSS_HAL_NODE_UHD_SPLIT_L,
    VPSS_HAL_NODE_UHD_SPLIT_R,
    VPSS_HAL_NODE_UHD_HALF,
    VPSS_HAL_NODE_3DDET,// 3D检测通路，只需要Y分量
    VPSS_HAL_NODE_ZME_2L,// 2级缩放节点
    VPSS_HAL_NODE_ROTATION_Y,
    VPSS_HAL_NODE_ROTATION_C,
    VPSS_HAL_NODE_ROTATION,
    VPSS_HAL_NODE_BUTT
} VPSS_HAL_NODE_TYPE_E;

typedef struct hiVPSS_HAL_INFO_S
{
    VPSS_REG_S *pstPqCfg;
    VPSS_HAL_NODE_TYPE_E enNodeType;
	VPSS_HAL_FRAME_S stInInfo;             //输入源信息
    VPSS_RWZB_INFO_S stRwzbInfo;

	/*VPSS V2_0*/
    VPSS_HAL_FRAME_S stInRefInfo[4];       //参考帧信息
    VPSS_HAL_FRAME_S stInWbcInfo;          //回写信息
    mt_u32 u32stt_w_phy_addr;
    mt_u32 u32stt_w_vir_addr;
	VPSS_NR_INFO_S stNrInfo;
	VPSS_CCCL_INFO_S stCCCLInfo;

	/*VPSS V1_0*/
    MT_DRV_VID_FRAME_ADDR_S stFieldAddr[6];
    VPSS_MT_ADDR_S stHisAddr;

    VPSS_DIE_INFO_S stDieInfo;

    mt_u32 u32ScdValue;
    VPSS_HAL_PORT_INFO_S astPortInfo[DEF_MT_DRV_VPSS_PORT_MAX_NUMBER];

} VPSS_HAL_INFO_S;


mt_s32 VPSS_HAL_Init(VPSS_IP_E enIP);
mt_s32 VPSS_HAL_DelInit(VPSS_IP_E enIP);

mt_s32 VPSS_HAL_SetClockEn(VPSS_IP_E enIP, MT_BOOL bClockEn);
mt_s32 VPSS_HAL_GetClockEn(VPSS_IP_E enIP, MT_BOOL *pbClockEn);

mt_s32 VPSS_HAL_GetIntState(VPSS_IP_E enIP, mt_u32* pu32IntState);
mt_s32 VPSS_HAL_ClearIntState(VPSS_IP_E enIP, mt_u32 u32IntState);

mt_s32 VPSS_HAL_SetNodeInfo(VPSS_IP_E enIP,
     VPSS_HAL_INFO_S *pstHalInfo,  VPSS_HAL_TASK_NODE_E enTaskNodeId);

mt_s32 VPSS_HAL_StartLogic(VPSS_IP_E enIP,
    MT_BOOL abNodeVaild[VPSS_HAL_TASK_NODE_BUTT]);

mt_s32 VPSS_HAL_GetSCDInfo(mt_u32 u32AppAddr,mt_s32 s32SCDInfo[32]);

mt_void VPSS_HAL_GetDetPixel(VPSS_IP_E enIP,mt_u32 BlkNum, mt_u8* pstData);

mt_s32 VPSS_HAL_GetBaseRegAddr(VPSS_IP_E enIP,
                                 mt_u32 *pu32PhyAddr,
                                 mt_u32 *pu32VirAddr);
#endif

