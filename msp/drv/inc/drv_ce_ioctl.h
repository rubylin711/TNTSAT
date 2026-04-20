/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CE_IOCTL_H__
#define __CE_IOCTL_H__
#include <linux/types.h>
#include "mt_module.h"
#include "mt_mpi_ce.h"

typedef struct _CMD_ADES_SESSION_S
{
  mt_session session;
  mt_u32 channel;
  mt_s32 ret;
} CMD_ADES_CREATE_S, CMD_ADES_DESTROY_S, CMD_ADES_PROCESS_STOP_S, CMD_ADES_ASYNC_START_S, CMD_ADES_ASYNC_WAIT_S;

typedef struct _CMD_ADES_CTRL_S
{
  mt_session session;
  MT_CE_ADES_CTRL_S ctrl;
  mt_s32 ret;
} CMD_ADES_CTRL_S, CMD_ADES_INFO_S, CMD_ADES_UPDATE_IV_S;

typedef struct _CMD_ADES_PROCESS_S
{
  mt_session session;
  /*mt_u8 *p_iv_addr;*/
  mt_u8 is_phy_addr;    /* physical address or not */
  phys_addr_t p_src_addr;
  phys_addr_t p_dst_addr;
  mt_u32 length;
  mt_u32 clear_length;
  mt_s32 ret;
} CMD_ADES_PROCESS_S, CMD_ADES_PROCESS_START_S;

typedef struct _CMD_ADES_ASYNC_REQUEST_S
{
  mt_session session;
  mt_u8 is_phy_addr;    /* physical address or not */
  phys_addr_t src;
  phys_addr_t dst;
  mt_u32 count;
  mt_u32 clear_length;
  mt_u32 protected_length;
  mt_s32 ret;
} CMD_ADES_ASYNC_REQUEST_S;

typedef struct _CMD_ADES_PROCESS_POLLING_S
{
  mt_session session;
  mt_u32 timeout;
  mt_s32 ret;
} CMD_ADES_PROCESS_POLLING_S;




typedef struct _CMD_TS_SESSION_S
{
  mt_session session;
  mt_u32 channel;
  mt_s32 ret;
} CMD_TS_CREATE_S, CMD_TS_DESTROY_S;

typedef struct _CMD_TS_CTRL_S
{
  mt_session session;
  MT_CE_TS_CTRL_S ctrl;
  mt_s32 ret;
} CMD_TS_CTRL_S, CMD_TS_INFO_S;

typedef struct _CMD_TS_PROCESS_S
{
  mt_session session;
  /*mt_u8 *p_iv_addr;*/
  mt_u8 is_phy_addr;    /* physical address or not */
  phys_addr_t p_src_addr;
  phys_addr_t p_dst_addr;
  mt_u32 length;
  mt_s32 ret;
} CMD_TS_PROCESS_S;




typedef struct _CMD_SHA_SESSION_S
{
  mt_session session;
  mt_s32 ret;
} CMD_SHA_CREATE_S, CMD_SHA_DESTROY_S, CMD_SHA_UPDATE_STOP_S;

typedef struct _CMD_SHA_INIT_S
{
  mt_session session;
  MT_CE_SHA_CTRL_S ctrl;
  mt_s32 ret;
} CMD_SHA_INIT_S, CMD_SHA_ATTR_S;

typedef struct _CMD_SHA_UPDATE_S
{
  mt_session session;
  phys_addr_t p_msg;
  mt_u8 is_phy_addr;
  mt_u32 length;
  mt_s32 ret;
} CMD_SHA_UPDATE_S, CMD_SHA_UPDATE_START_S;

typedef struct _CMD_SHA_FINAL_S
{
  mt_session session;
  mt_u8 p_dgst[64];
  mt_s32 ret;
} CMD_SHA_FINAL_S;

typedef struct _CMD_SHA_UPDATE_POLLING_S
{
  mt_session session;
  mt_u32 timeout;
  mt_s32 ret;
} CMD_SHA_UPDATE_POLLING_S;




typedef struct _CMD_RSA_SESSION_S
{
  mt_session session;
  mt_s32 ret;
} CMD_RSA_CREATE_S, CMD_RSA_DESTROY_S;

typedef struct _CMD_RSA_CTRL_S
{
    mt_session session;
    MT_CE_RSA_CTRL_S ctrl;
    mt_s32 ret;
} CMD_RSA_CTRL_S;

typedef struct _CMD_RSA_PROCESS_S
{
  mt_session session;

  mt_u8 *p_src_addr;
  mt_u8 *p_dst_addr;

  /*MT_CE_RSA_KEY_LENGTH_E key_length;*/
  /*MT_CE_RSA_EXP_LENGTH_E exp_length;*/
  mt_u32 src_length;
  
  /*mt_u8 m[256];*/
  /*mt_u8 e[256];*/
  
  /*mt_u8 src_addr[256];*/
  /*mt_u8 dst_addr[256];*/

  mt_s32 ret;
} CMD_RSA_PROCESS_S;




typedef struct _CMD_BN_SESSION_S
{
  mt_session session;
  mt_s32 ret;
} CMD_BN_CREATE_S, CMD_BN_DESTROY_S;

typedef struct _CMD_BN_MOD_MOD_S
{
  mt_session session;
  //mt_u8 *r;
  //mt_u8 *d;
  //mt_u8 *m;
  mt_u8 r[256];//mt_u8 r[64];
  mt_u8 d[256];//mt_u8 d[64];
  mt_u8 m[256];//mt_u8 m[64];
  mt_u32 d_length;
  mt_u32 m_length;
  mt_s32 ret;
} CMD_BN_MOD_MOD_S;

typedef struct _CMD_BN_MOD_MUL_S
{
  mt_session session;
  //mt_u8 *r;
  //mt_u8 *a;
  //mt_u8 *b;
  //mt_u8 *m;
  mt_u8 r[256];//mt_u8 r[64];
  mt_u8 a[256];//mt_u8 a[64];
  mt_u8 b[256];//mt_u8 b[64];
  mt_u8 m[256];//mt_u8 m[64];
  mt_u32 a_length;
  mt_u32 b_length;
  mt_u32 m_length;
  mt_s32 ret;
} CMD_BN_MOD_MUL_S;

typedef struct _CMD_BN_MOD_ADD_S
{
  mt_session session;
  //mt_u8 *r;
  //mt_u8 *a;
  //mt_u8 *b;
  //mt_u8 *m;
  mt_u8 r[256];//mt_u8 r[64];
  mt_u8 a[256];//mt_u8 a[64];
  mt_u8 b[256];//mt_u8 b[64];
  mt_u8 m[256];//mt_u8 m[64];
  mt_u32 a_length;
  mt_u32 b_length;
  mt_u32 m_length;
  mt_s32 ret;
} CMD_BN_MOD_ADD_S;

typedef struct _CMD_BN_MOD_SUB_S
{
  mt_session session;
  //mt_u8 *r;
  //mt_u8 *a;
  //mt_u8 *b;
  //mt_u8 *m;
  mt_u8 r[256];//mt_u8 r[64];
  mt_u8 a[256];//mt_u8 a[64];
  mt_u8 b[256];//mt_u8 b[64];
  mt_u8 m[256];//mt_u8 m[64];
  mt_u32 a_length;
  mt_u32 b_length;
  mt_u32 m_length;
  mt_s32 ret;
} CMD_BN_MOD_SUB_S;

typedef struct _CMD_BN_MOD_INV_S
{
  mt_session session;
  //mt_u8 *r;
  //mt_u8 *d;
  //mt_u8 *m;
  mt_u8 r[256];//mt_u8 r[64];
  mt_u8 d[256];//mt_u8 d[64];
  mt_u8 m[256];//mt_u8 m[64];
  mt_u32 d_length;
  mt_u32 m_length;
  mt_s32 ret;
} CMD_BN_MOD_INV_S;

typedef struct _CMD_BN_MOD_EXP_S
{
  mt_session session;
  //mt_u8 *r;
  //mt_u8 *a;
  //mt_u8 *p;
  //mt_u8 *m;
  mt_u8 r[256];
  mt_u8 a[256];
  mt_u8 p[256];
  mt_u8 m[256];
  mt_u32 a_length;
  mt_u32 p_length;
  mt_u32 m_length;
  mt_s32 ret;
} CMD_BN_MOD_EXP_S;





typedef struct _CMD_EC_POINT_SESSION_S
{
  mt_session session;
  mt_s32 ret;
} CMD_EC_POINT_CREATE_S, CMD_EC_POINT_DESTROY_S;

typedef struct _CMD_EC_POINT_MUL_S
{
  mt_session session;
  //MT_CE_EC_PARAMS_S xParams;
  //MT_CE_EC_POINT_S *r;
  //const mt_u8 *g_scalar;
  //MT_CE_EC_POINT_S *p_point;
  //const mt_u8 *p_scalar;


  mt_u8 xParams_q[64];
  mt_u8 xParams_a[64];
  mt_u8 xParams_GX[64];
  mt_u8 xParams_GY[64];
  mt_u32 xParams_keySize;

  
  mt_u8 r_X[64];
  mt_u8 r_Y[64];
  mt_u8 g_scalar[64];
  mt_u8 p_point_X[64];
  mt_u8 p_point_Y[64];
  mt_u8 p_scalar[64];

  mt_u32 p_point_flag;
  
  mt_s32 ret;
} CMD_EC_POINT_MUL_S;

typedef struct _CMD_EC_POINT_ADD_S
{
  mt_session session;
  //MT_CE_EC_PARAMS_S xParams;
  //MT_CE_EC_POINT_S *r;
  //MT_CE_EC_POINT_S *a;
  //MT_CE_EC_POINT_S *b;

  mt_u8 xParams_q[64];
  mt_u8 xParams_a[64];
  mt_u32 xParams_keySize;
  
  mt_u8 r_X[64];
  mt_u8 r_Y[64];
  mt_u8 a_X[64];
  mt_u8 a_Y[64];
  mt_u8 b_X[64];
  mt_u8 b_Y[64];

  mt_s32 ret;
} CMD_EC_POINT_ADD_S;





typedef struct _CMD_BGC_SESSION_S
{
    mt_session session;
    MT_CE_BGC_SEM_REQ_S req_time_out;
} CMD_BGC_REQ_S, CMD_BGC_RLS_S;

typedef struct _CMD_BGC_SETUP_S
{
    mt_session session;
    mt_u8 *start_addr;
    mt_u32 size;
    MT_CE_BGC_DELAY delay;
    mt_u8 *golden_hash;
    MT_CE_BGC_LOCK_S lock;
} CMD_BGC_SETUP_S;
    

typedef struct _CMD_CE_RST_S
{
  mt_u32 mode;
  mt_s32 ret;
} CMD_CE_RST_S;


#define CE_IOC_ADES_CREATE                  _IOWR(MT_ID_CE, 0x0, CMD_ADES_CREATE_S)
#define CE_IOC_ADES_DESTROY                 _IOWR(MT_ID_CE, 0x1, CMD_ADES_DESTROY_S)
#define CE_IOC_ADES_CONFIG                  _IOWR(MT_ID_CE, 0x2, CMD_ADES_CTRL_S)
#define CE_IOC_ADES_PROCESS                 _IOWR(MT_ID_CE, 0x3, CMD_ADES_PROCESS_S)
#define CE_IOC_ADES_PROCESS_START           _IOWR(MT_ID_CE, 0x4, CMD_ADES_PROCESS_S)
#define CE_IOC_ADES_PROCESS_POLLING         _IOWR(MT_ID_CE, 0x5, CMD_ADES_PROCESS_POLLING_S)
#define CE_IOC_ADES_PROCESS_STOP            _IOWR(MT_ID_CE, 0x6, CMD_ADES_PROCESS_STOP_S)
#define CE_IOC_ADES_GET_INFO                _IOWR(MT_ID_CE, 0x7, CMD_ADES_INFO_S)
#define CE_IOC_ADES_UPDATE_IV               _IOWR(MT_ID_CE, 0x8, CMD_ADES_UPDATE_IV_S)

#define CE_IOC_ADES_ASYNC_REQUEST           _IOWR(MT_ID_CE, 0x9, CMD_ADES_ASYNC_REQUEST_S)
#define CE_IOC_ADES_ASYNC_START             _IOWR(MT_ID_CE, 0xa, CMD_ADES_ASYNC_START_S)
#define CE_IOC_ADES_ASYNC_WAIT              _IOWR(MT_ID_CE, 0xb, CMD_ADES_ASYNC_WAIT_S)

#define CE_IOC_TS_CREATE                    _IOWR(MT_ID_CE, 0x10, CMD_TS_CREATE_S)
#define CE_IOC_TS_DESTROY                   _IOWR(MT_ID_CE, 0x11, CMD_TS_DESTROY_S)
#define CE_IOC_TS_CONFIG                    _IOWR(MT_ID_CE, 0x12, CMD_TS_CTRL_S)
#define CE_IOC_TS_PROCESS                   _IOWR(MT_ID_CE, 0x13, CMD_TS_PROCESS_S)
#define CE_IOC_TS_GET_INFO                  _IOWR(MT_ID_CE, 0x14, CMD_TS_INFO_S)

#define CE_IOC_SHA_CREATE                   _IOWR(MT_ID_CE, 0x20, CMD_SHA_CREATE_S)
#define CE_IOC_SHA_DESTROY                  _IOWR(MT_ID_CE, 0x21, CMD_SHA_DESTROY_S)
#define CE_IOC_SHA_INIT                     _IOWR(MT_ID_CE, 0x22, CMD_SHA_INIT_S)
#define CE_IOC_SHA_UPDATE                   _IOWR(MT_ID_CE, 0x23, CMD_SHA_UPDATE_S)
#define CE_IOC_SHA_FINAL                    _IOWR(MT_ID_CE, 0x24, CMD_SHA_FINAL_S)
#define CE_IOC_SHA_UPDATE_START             _IOWR(MT_ID_CE, 0x25, CMD_SHA_UPDATE_START_S)
#define CE_IOC_SHA_UPDATE_POLLING           _IOWR(MT_ID_CE, 0x26, CMD_SHA_UPDATE_POLLING_S)
#define CE_IOC_SHA_UPDATE_STOP              _IOWR(MT_ID_CE, 0x27, CMD_SHA_UPDATE_STOP_S)
#define CE_IOC_SHA_ATTR                     _IOWR(MT_ID_CE, 0x28, CMD_SHA_ATTR_S)

#define CE_IOC_RSA_CREATE                   _IOWR(MT_ID_CE, 0x30, CMD_RSA_CREATE_S)
#define CE_IOC_RSA_DESTROY                  _IOWR(MT_ID_CE, 0x31, CMD_RSA_DESTROY_S)
#define CE_IOC_RSA_CONFIG                   _IOWR(MT_ID_CE, 0x32, CMD_RSA_CTRL_S)
#define CE_IOC_RSA_PROCESS                  _IOWR(MT_ID_CE, 0x33, CMD_RSA_PROCESS_S)

#define CE_IOC_BN_CREATE                    _IOWR(MT_ID_CE, 0x40, CMD_BN_CREATE_S)
#define CE_IOC_BN_DESTROY                   _IOWR(MT_ID_CE, 0x41, CMD_BN_DESTROY_S)
#define CE_IOC_BN_MOD_MOD                   _IOWR(MT_ID_CE, 0x42, CMD_BN_MOD_MOD_S)
#define CE_IOC_BN_MOD_MUL                   _IOWR(MT_ID_CE, 0x43, CMD_BN_MOD_MUL_S)
#define CE_IOC_BN_MOD_ADD                   _IOWR(MT_ID_CE, 0x44, CMD_BN_MOD_ADD_S)
#define CE_IOC_BN_MOD_SUB                   _IOWR(MT_ID_CE, 0x45, CMD_BN_MOD_SUB_S)
#define CE_IOC_BN_MOD_INV                   _IOWR(MT_ID_CE, 0x46, CMD_BN_MOD_INV_S)
#define CE_IOC_BN_MOD_EXP                   _IOWR(MT_ID_CE, 0x47, CMD_BN_MOD_EXP_S)

#define CE_IOC_EC_POINT_CREATE              _IOWR(MT_ID_CE, 0x50, CMD_EC_POINT_CREATE_S)
#define CE_IOC_EC_POINT_DESTROY             _IOWR(MT_ID_CE, 0x51, CMD_EC_POINT_DESTROY_S)
#define CE_IOC_EC_POINT_MUL                 _IOWR(MT_ID_CE, 0x52, CMD_EC_POINT_MUL_S)
#define CE_IOC_EC_POINT_ADD                 _IOWR(MT_ID_CE, 0x53, CMD_EC_POINT_ADD_S)

#define CE_IOC_BGC_REQUEST                  _IOWR(MT_ID_CE, 0x70, CMD_BGC_REQ_S)
#define CE_IOC_BGC_RELEASE                  _IOR(MT_ID_CE, 0x71, CMD_BGC_RLS_S)
#define CE_IOC_BGC_SETUP                    _IOR(MT_ID_CE, 0x72, CMD_BGC_SETUP_S)

#define CE_IOC_MAC_CREATE                   _IOWR(MT_ID_CE, 0x80, CMD_SHA_CREATE_S)
#define CE_IOC_MAC_DESTROY                  _IOWR(MT_ID_CE, 0x81, CMD_SHA_DESTROY_S)
#define CE_IOC_MAC_INIT                     _IOWR(MT_ID_CE, 0x82, CMD_SHA_INIT_S)
#define CE_IOC_MAC_UPDATE                   _IOWR(MT_ID_CE, 0x83, CMD_SHA_UPDATE_S)
#define CE_IOC_MAC_FINAL                    _IOWR(MT_ID_CE, 0x84, CMD_SHA_FINAL_S)

#define CE_IOC_RST                          _IOWR(MT_ID_CE, 0x60, CMD_CE_RST_S)
#endif

