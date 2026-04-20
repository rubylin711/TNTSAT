/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_GO_TEXT22_H__
#define __MT_GO_TEXT22_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /*__cplusplus*/
#endif /*__cplusplus*/

typedef struct
{
    void *p_rgn;            //!<@~english Pointer to region handle, physical address @~chinese region句柄指针，物理地址
    void *p_rgn_mmap;       //!<@~english Pointer to region handle, virtual address @~chinese region句柄指针，虚拟地址
    mt_u32 width;           //!<@~english Width of image data buffer @~chinese 图片数据缓存的宽度
    mt_u32 height;          //!<@~english Height of image data buffer @~chinese 图片数据缓冲的高度
    mt_u32 *p_buf;          //!<@~english Pointer to the physical address of image data buffer @~chinese 图片数据缓冲的物理地址指针
    mt_u32 *p_head;         //!<@~english Pointer to the physical address of head parameter buffer @~chinese OSD头参数缓冲的物理地址指针
    mt_u32 fmt;             //!<@~english Fomat of image data in image data buffer @~chinese 图片数据的格式
    MT_BOOL b_palette;      //!<@~english A palette exist when this parameter is true @~chinese 取值为1时表示带palette
    mt_u32 *p_palette;      //!<@~english Pointer to the physical address of palette buffer @~chinese palette缓冲的物理地址指针
    MT_BOOL alpha_en;       //!<@~english alpha is valid when this parameter is true @~chinese 取值为1时表示alpha有效
    mt_u32 alpha;           //!<@~english region global alpha @~chinese region的全局alpha
    MT_BOOL keycolor_en;    //!<@~english key_color is valid when this parameter is true @~chinese 取值为1表示key_color有效
    mt_u32 key_color;       //!<@~english region key color @~chinese region的关键色
    mt_u32 bpp;             //!<@~english bit per pixel of the data in image data buffer @~chinese 每个图像数据占据的位数
    mt_u32 pitch;           //!@~english image data buffer pitch in unit of byte @~chinese 图像数据行宽，以字节为单位
    mt_u32 *p_buf_mmap;     //!<@~english Pointer to the virtual address of image data buffer @~chinese 图片数据缓冲的虚拟地址指针
    mt_u32 *p_palette_mmap; //!<@~english Pointer to the virtual address of palette buffer @~chinese palette缓冲的虚拟地址指针

    MTGO_SURINFO_S mtgoSurface;
} MTGO_SURFACE_HDL_S;

//typedef MTGO_SURINFO_S MTGO_SURFACE_HDL_S;

typedef struct
{
    mt_s32 x;
    mt_s32 y;
} MT_POS;

#define MAX_BMPSURFACE_WIDTH 2000

#define MTGO_PF_GRAY_8 (MTGO_PF_A8)

#ifdef __cplusplus
#if __cplusplus
}
#endif /*__cplusplus*/
#endif /*__cplusplus*/

#endif //  __MT_GO_TEXT22_H__
