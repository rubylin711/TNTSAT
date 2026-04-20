/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_VI_IOCTL_H__
#define __DRV_VI_IOCTL_H__

#include "mt_debug.h"
#include "mt_unf_vi.h"
#include "mt_drv_vpss.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif

/**********************************************************
vi handle defination:
1) real vi
handle = VI_ID(16bits) + PORT(8bits) + CHAN(8bits, fix to 0)
2) virtual vi
handle = VI_ID(16bits) + PORT(8bits) + CHAN(8bits, max is 2)
 ***********************************************************/

#define MAX_VI_CHN 2
#define MAX_VI_PORT 2
#define VI_MAX_VPSS_PORT 3

typedef struct mtUNF_VI_CREATE_S
{
    MT_UNF_VI_E      enPort;
    mt_handle        hVi;
    VPSS_HANDLE      hVpss;
    MT_UNF_VI_ATTR_S stViAttr;
} VI_CREATE_S;

typedef struct mtVI_ATTR_S
{
    mt_handle        hVi;
    MT_UNF_VI_ATTR_S stAttr;
} VI_ATTR_S;

typedef struct mtVI_BUF_ATTR_S
{
    mt_handle               hVi;
    MT_UNF_VI_BUFFER_ATTR_S stBufAttr;
} VI_BUF_ATTR_S;

typedef struct mtVI_FRAME_S
{
    mt_handle                 hVi;
    MT_UNF_VIDEO_FRAME_INFO_S stViFrame;
    mt_u32                    u32TimeoutMs;
} VI_FRAME_S;

typedef int (*PFN_VI_DRV_RlsImage)(mt_handle, MT_DRV_VIDEO_FRAME_S*);
typedef int (*PFN_VI_DRV_ChangeVencInfo)(mt_handle, mt_u32, mt_u32);
typedef int (*PFN_VI_DRV_ChangeWinInfo)(mt_handle, MT_DRV_WIN_PRIV_INFO_S*);

typedef struct mtVI_VPSS_PORT_PARAM_S
{
    mt_handle                 hDst;
    mt_handle                 hPort;
    MT_BOOL                   bEnable;
    PFN_VI_DRV_RlsImage       pfRlsImage;
    PFN_VI_DRV_ChangeVencInfo pfChangeVencInfo;
    PFN_VI_DRV_ChangeWinInfo  pfChangeWinInfo;
} VI_VPSS_PORT_PARAM_S;

typedef struct mtVI_VPSS_PORT_S
{
    mt_handle            hVi;
    VPSS_HANDLE          hVpss;
    VI_VPSS_PORT_PARAM_S stPortParam;
} VI_VPSS_PORT_S;

typedef struct mtVI_VPSS_PORT_AND_WIN_S
{
    mt_handle hPort;
    mt_handle hWindow;
} VI_VPSS_PORT_AND_WIN_S;

typedef enum mtVI_ENGINE_STATE_E
{
    VI_ENGINE_ATTACHED = 0x1,
    VI_ENGINE_STARTED = 0x2,
    VI_ENGINE_BUFSET = 0x4,
    VI_ENGINE_BUTT = 0x0
} VI_ENGINE_STATE_E;

typedef struct mtVI_S
{
    mt_handle   hVi;
    VPSS_HANDLE hVpss;

    mt_u32           u32State;
    MT_UNF_VI_ATTR_S stAttr;

    VI_VPSS_PORT_PARAM_S stPortParam[VI_MAX_VPSS_PORT];
} VI_S;

typedef enum mtIOC_VI_E
{
    IOC_VI_CREATE = 0,
    IOC_VI_DESTROY,
    IOC_VI_SET_ATTR,
    IOC_VI_SET_BUF,
    IOC_VI_CREATE_VPSS_PORT,
    IOC_VI_DESTROY_VPSS_PORT,
    IOC_VI_START,
    IOC_VI_STOP,
    IOC_VI_Q_FRAME,
    IOC_VI_DQ_FRAME,
    IOC_VI_ACQUIRE_FRAME,
    IOC_VI_RELEASE_FRAME,
    IOC_VI_BUTT,
} IOC_VI_E;

#define CMD_VI_CREATE _IOWR(MT_ID_VI, IOC_VI_CREATE, VI_CREATE_S)
#define CMD_VI_DESTROY _IOWR(MT_ID_VI, IOC_VI_DESTROY, mt_handle)

#define CMD_VI_SET_ATTR _IOWR(MT_ID_VI, IOC_VI_SET_ATTR, VI_ATTR_S)
#define CMD_VI_SET_BUF _IOWR(MT_ID_VI, IOC_VI_SET_BUF, VI_BUF_ATTR_S)

#define CMD_VI_CREATE_VPSS_PORT _IOWR(MT_ID_VI, IOC_VI_CREATE_VPSS_PORT, VI_VPSS_PORT_S)
#define CMD_VI_DESTROY_VPSS_PORT _IOWR(MT_ID_VI, IOC_VI_DESTROY_VPSS_PORT, VI_VPSS_PORT_S)

#define CMD_VI_START _IOWR(MT_ID_VI, IOC_VI_START, mt_handle)
#define CMD_VI_STOP _IOWR(MT_ID_VI, IOC_VI_STOP, mt_handle)

#define CMD_VI_Q_FRAME _IOWR(MT_ID_VI, IOC_VI_Q_FRAME, VI_FRAME_S)
#define CMD_VI_DQ_FRAME _IOWR(MT_ID_VI, IOC_VI_DQ_FRAME, VI_FRAME_S)

#define CMD_VI_ACQUIRE_FRAME _IOWR(MT_ID_VI, IOC_VI_ACQUIRE_FRAME, VI_FRAME_S)
#define CMD_VI_RELEASE_FRAME _IOWR(MT_ID_VI, IOC_VI_RELEASE_FRAME, VI_FRAME_S)

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif

#endif //__DRV_VI_IOCTL_H__
