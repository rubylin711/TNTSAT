/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_GO_SURFACE_H__
#define __MT_GO_SURFACE_H__

#include "mt_go_comm.h"

#ifdef __cplusplus
extern "C"{
#endif  /*__cplusplus*/

/***************************** Macro Definition ******************************/
/** \addtogroup      MTGO_SURFACE */
/** @{ */  /** <!-- [MTGO_SURFACE] */

typedef enum
{
    MTGO_PF_CLUT8 = 0,  /**<Palette*//**<CNcomment:调色板 */
    MTGO_PF_CLUT1,
    MTGO_PF_CLUT4,
    MTGO_PF_4444,       /**<Each pixel occupies 16 bits, and the A/R/G/B components each occupies 4 bits. They are sorted by address in descending order.*//**<CNcomment:一个像素占16bit，ARGB每分量占4bit，按地址由高至低排列 */
    MTGO_PF_0444,       /**<Each pixel occupies 16 bits, and the A/R/G/B components each occupies 4 bits. They are sorted by address in descending order. The A component does not take effect.*//**<CNcomment:一个像素占16bit，ARGB每分量占4bit，按地址由高至低排列, A分量不起作用 */

    MTGO_PF_1555,       /**<Each pixel occupies 16 bits, the R/G/B components each occupies 5 bits, and the A component occupies 1 bit. They are sorted by address in descending order.*//**<CNcomment:一个像素占16bit，RGB每分量占5bit，A分量1bit，按地址由高至低排列 */
    MTGO_PF_0555,       /**<Each pixel occupies 16 bits, the R/G/B components each occupies 5 bits, and the A component occupies 1 bit. They are sorted by address in descending order. The A component does not take effect.*//**<CNcomment:一个像素占16bit，RGB每分量占5bit，A分量1bit，按地址由高至低排列, A分量不起作用 */
    MTGO_PF_565,        /**<Each pixel occupies 16 bits, and the R/G/B components each occupies 5 bits, 6 bits, and 5 bits respectively. They are sorted by address in descending order.*//**<CNcomment:一个像素占16bit，RGB每分量分别占5bit、6bit和5bit，按地址由高至低排列 */
    MTGO_PF_8565,       /**<Each pixel occupies 24 bits, and the A/R/G/B components each occupies 8 bits, 5 bits, 6 bits, and 5 bits respectively. They are sorted by address in descending order.*//**<CNcomment:一个像素占24bit，ARGB每分量分别占8bit, 5bit、6bit和5bit，按地址由高至低排列 */
    MTGO_PF_8888,       /**<Each pixel occupies 32 bits, and the A/R/G/B components each occupies 8 bits. They are sorted by address in descending order.*//**<CNcomment:一个像素占32bit，ARGB每分量占8bit，按地址由高至低排列 */
    MTGO_PF_0888,       /**<Each pixel occupies 24 bits, and the A/R/G/B components each occupies 8 bits. They are sorted by address in descending order. The A component does not take effect.*//**<CNcomment:一个像素占24bit，ARGB每分量占8bit，按地址由高至低排列，A分量不起作用 */

    MTGO_PF_YUV400,     /**<Semi-planar YUV 400 format*//**<CNcomment semi-planar YUV 400格式 */    
    MTGO_PF_YUV420,     /**<Semi-planar YUV 420 format*//**<CNcomment: semi-planar YUV 420格式 */
    MTGO_PF_YUV422,     /**<Semi-planar YUV 422 format and horizontal sampling format*//**<CNcomment: semi-planar YUV 422格式  水平采样格式*/
    MTGO_PF_YUV422_V,   /**<Semi-planar YUV 422 format and vertical sampling format*//**<CNcomment: semi-planar YUV 422格式  垂直采样格式*/    
    MTGO_PF_YUV444,     /**<Semi-planar YUV 444 format*//**<CNcomment: semi-planar YUV 444格式 */
    
	MTGO_PF_A1, 
	MTGO_PF_A8,
        
    MTGO_PF_YUV888,
    MTGO_PF_UYVY,
    MTGO_PF_YUV8888,
	MTGO_PF_RLE8,
	MTGO_PF_SP_CMYK,	/**<Semi-planar cmyk format *//**<CNcomment semi-planar cmyk格式 */ 

    
    MTGO_PF_ACLUT88,  //for fpga
    MTGO_PF_233,      //for fpga
    MTGO_PF_XYLC,     //for fpga
    MTGO_PF_YUV420TILE,   //for fpga
    MTGO_PF_BUTT
} MTGO_PF_E;
/** @} */  /*! <!-- Macro Definition end */

/*************************** Structure Definition ****************************/
/** \addtogroup      MTGO_SURFACE */
/** @{ */  /** <!-- [MTGO_SURFACE] */

/**Component type*/
/** CNcomment:分量类型 */
typedef enum
{
    MTGO_PDFORMAT_RGB = 0,
    MTGO_PDFORMAT_RGB_PM,   /**< pre-multiplied */
    MTGO_PDFORMAT_Y,
    MTGO_PDFORMAT_UV,
    MTGO_PDFORMAT_UV_LE,    
	MTGO_PDFORMAT_CM,
	MTGO_PDFORMAT_YK,
	
    MTGO_PDFORMAT_BUTT
} MTGO_PDFORMAT_E;

typedef struct
{
    MTGO_PDFORMAT_E Format;     /**<Component type*//**<CNcomment:分量类型 */
    mt_void*        pData;      /**<Pointer to the virtual address of a component*//**<CNcomment:分量虚拟地址指针 */
    phys_addr_t       pPhyData;   /**<Pointer to the physical address of a component*//**<CNcomment:分量物理地址指针 */    
    mt_u32          Pitch;      /**<Component pitch*//**<CNcomment:分量行宽 */
    mt_u32          Bpp;        /**<Bytes per pixel*//**<CNcomment:bytes per pixel */
    mt_u32          Offset;
} MTGO_PIXELDATA_S;

/**Maximum number of data components*/
/** CNcomment:最大数据分量数 */
#define MAX_PARTPIXELDATA 3

/**The following sync modes can be used together. The non-automatic sync mode is also available.*/
/** CNcomment:下面同步模式可以组合使用，不自动同步方式有用 */
typedef enum 
{
    MTGO_SYNC_MODE_CPU = 0x01, /*Sync mode. In this mode, CPU operations are required.*//**<CNcomment:同步，下一步需要进行CPU操作*/
    MTGO_SYNC_MODE_TDE = 0x02, /*Sync mode. In this mode, the 2D acceleration operation is required.*//**<CNcomment:同步，下一步需要进行2D加速操作*/
    MTGO_SYNC_MODE_BUTT,
} MTGO_SYNC_MODE_E;


typedef enum
{
    MTGO_MEMTYPE_MMZ = 0,       /**<The memory is an media memory zone (MMZ) memory.*//**<CNcomment:使用MMZ内存进行创建 */
    MTGO_MEMTYPE_OS,            /**<The memory is an operating system (OS) memory.*//**<CNcomment:使用系统内存进行创建 */
    MTGO_MEMTYPE_BUTT
}MTGO_MEMTYPE_E;

typedef enum 
{
	MTGO_OWNER_USER = 0,        /**<The memory is managed by users. That is, the memory is allocated and destroyed by users.*//**<CNcomment:由用户进行管理，需要用户自行分配内存，销毁内存 */
	MTGO_OWNER_MTGO,            /**<The memory is managed by the MtGo module rather than users.*//**<CNcomment:由MTGO进行管理，用户不需要管理内存 */
	MTGO_OWNER_BUTT
}MTGO_OWNER_E;

typedef struct
{
    mt_s32 Width;                               /**<Surface width*//**<CNcomment:surface宽度 */
    mt_s32 Height;                              /**<Surface height*//**<CNcomment:surface高度 */
    MTGO_PF_E PixelFormat;                      /**<Pixel format of a surface*//**<CNcomment:surface像素格式*/
    mt_u32   Pitch[MAX_PARTPIXELDATA];          /**<Pitch of a surface*//**<CNcomment:surface行间距离*/
    phys_addr_t  pPhyAddr[MAX_PARTPIXELDATA];       /**<Physical address of a surface*//**<CNcomment:surface物理地址*/
    mt_char* pVirAddr[MAX_PARTPIXELDATA];       /**<Virtual address of a surface*//**<CNcomment:surface虚拟地址*/
    MTGO_MEMTYPE_E MemType;                     /**<Type of the surface memory*//**<CNcomment:surface内存类型*/
    MT_BOOL bPubPalette;                        /**<Use common Palette or not*//**<CNcomment:是否使用公共调色板;只对Clut格式有效.
                                                    若不使用公共的调色板,则使用自带的调色板*/
}MTGO_SURINFO_S;

typedef struct
{
    mt_s32 Width;                               /**<Surface width*//**<CNcomment:surface宽度 */
    mt_s32 Height;                              /**<Surface height*//**<CNcomment:surface高度 */
    MTGO_PF_E PixelFormat;                      /**<Pixel format of a surface*//**<CNcomment:surface像素格式*/

    /**<Pitch of a surface
    Pitch[0] indicates the pitch in RGB format or the pitch of the Y component in semi-planar format.
     Pitch[1] indicates the pitch of the C component in semi-planar format.
     Pitch[2] is reserved.*/
    /**<CNcomment:surface行间距离，
     Pitch[0]表示RGB格式行间距， 或者Semi-planner的Y分量的行间距
     Pitch[1]表示Semi-planner的C分量的行间距
     Pitch[2]暂时不使用。*/
    mt_u32   Pitch[MAX_PARTPIXELDATA];      

    /**<Physical address of a surface
    pPhyAddr[0] indicates the physical address in RGB format or the physical address of the Y component in semi-planar format.
    pPhyAddr[1] indicates the physical address of the C component in semi-planar format.*/
    /**<CNcomment:surface物理地址
    pPhyAddr[0]表示RGB格式物理地址， 或者Semi-planner的Y分量的物理地址
    pPhyAddr[1]表示Semi-planner的C分量的物理地址*/
    phys_addr_t pPhyAddr[MAX_PARTPIXELDATA];

    /**<Virtual address of a surface
    pVirAddr[0] indicates the virtual address in RGB format or the virtual address of the Y component in semi-planar format.
    pVirAddr[1] indicates the virtual address of the C component in semi-planar format.*/
    /**<CNcomment:surface虚拟地址
    pVirAddr[0]表示RGB格式虚拟地址， 或者Semi-planner的Y分量的虚拟地址
    pVirAddr[1]表示Semi-planner的C分量的虚拟地址*/
    mt_char* pVirAddr[MAX_PARTPIXELDATA];       /**<CNcomment: surface虚拟地址
                                                     pVirAddr[0]表示RGB格式虚拟地址， 或者Semi-planner的Y分量的虚拟地址
                                                     pVirAddr[1]表示Semi-planner的C分量的虚拟地址
                                                */
    MTGO_MEMTYPE_E MemType;                     /**<Type of the surface memory*//**<CNcomment:surface内存类型*/
    MT_BOOL bPubPalette;        /**<Use common Palette or not*//**<CNcomment:是否使用公共调色板;只对Clut格式有效*/
    MTGO_OWNER_E   MemOwner;                    /**<Memory source. For example, the memory is allocated by users or the MtGo module.*//**<CNcomment:内存的来源,例如是用户分配,还是MTGO分配*/
}MTGO_SURINFOEX_S;

/**Data component structure*/
/** CNcomment:数据分量结构 */
typedef MTGO_PIXELDATA_S MT_PIXELDATA[MAX_PARTPIXELDATA];

/** @} */  /*! <!-- Structure Definition end */

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
/** \addtogroup      MTGO_SURFACE */
/** @{ */  /** <!-- [MTGO_SURFACE] */

/** 
\brief Initializes the surface module. CNcomment:初始化Surface CNend
\attention \n
When ::MT_GO_Init is called, this API is also called.
CNcomment: ::MT_GO_Init已经包含对该接口的调用 CNend
\param N/A

\retval ::MT_SUCCESS
\retval ::MT_FAILURE

\see \n
::MT_GO_DeinitSurface
*/
mt_s32 MT_GO_InitSurface(mt_void);

/** 
\brief Deinitializes the surface module. CNcomment:去初始化Surface CNend
\attention \n
When ::MT_GO_Deinit is called, this API is also called.
CNcomment: ::MT_GO_Deinit已经包含对该接口的调用 CNend
\param N/A

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT

\see \n
::MT_GO_InitSurface
*/
mt_s32 MT_GO_DeinitSurface(mt_void);

/** 
\brief Sets the alpha value of a surface. CNcomment:设置surface的alpha值 CNend
\attention \n
N/A
\param[in]  Surface Surface handle. CNcomment:Surface句柄 CNend
\param[in]  Alpha Alpha value, ranging from 0 to 255. The value 0 indicates transparent, and the value 255 indicates opaque. CNcomment:Alpha值，范围是0-255。0表示透明，255表示不透明 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_LOCKED
\retval ::MTGO_ERR_INVHANDLE

\see \n
::MT_GO_GetSurfaceAlpha
*/
mt_s32 MT_GO_SetSurfaceAlpha(mt_handle Surface, mt_u8 Alpha);

/** 
\brief Obtains the alpha value of a surface. CNcomment:获取surface的alpha值 CNend
\attention \n
N/A
\param[in]  Surface Surface handle. CNcomment:Surface句柄 CNend
\param[out] pAlpha Pointer to the received alpha value. The pointer cannot be null. CNcomment:接收alhpa值的空间指针，不能为空 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR

\see \n
::MT_GO_SetSurfaceAlpha
*/
mt_s32 MT_GO_GetSurfaceAlpha(mt_handle Surface, mt_u8* pAlpha);

/** 
\brief Enables or disables the colorkey of a surface. CNcomment:设置是否使能surface的colorkey CNend
\attention \n
N/A
\param[in] Surface Surface handle. CNcomment:Surface句柄 CNend
\param[in] Enable Colorkey enable. MT_TRUE: enabled; MT_FALSE: disabled. CNcomment:是否使能colorKey。MT_TRUE：使能；MT_FALSE：不使能 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE

\see \n
N/A
*/
mt_s32 MT_GO_EnableSurfaceColorKey(mt_handle Surface, MT_BOOL Enable);

/** 
\brief Sets the colorkey value of a surface. CNcomment:设置surface的colorKey值 CNend
\attention \n
N/A
\param[in] Surface Surface handle. CNcomment:Surface句柄 CNend
\param[in] ColorKey Colorkey value. For the RGB format, the colorkey is padded with 32-bit colors. For the CLUT format, the colorkey is padded with color index. CNcomment:Colorkey值, 如果是RGB格式就使用全部按照32bit来进行填充，如果是CLUT格式就使用颜色索引来填充。CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_LOCKED
\retval ::MTGO_ERR_INVHANDLE

\see \n
::MT_GO_GetSurfaceColorKey
*/
mt_s32 MT_GO_SetSurfaceColorKey(mt_handle Surface, MT_COLOR ColorKey);

/** 
\brief Obtains the colorkey value of a surface. CNcomment:获取surface的colorkey值 CNend
\attention \n
N/A
\param[in]  Surface Surface handle. CNcomment:Surface句柄 CNend
\param[out] pColorKey Pointer to the received colorkey value. The value cannot be empty. CNcomment:接收colorkey值的空间指针，不可为空 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_NOCOLORKEY

\see \n
::MT_GO_SetSurfaceColorKey
*/
mt_s32 MT_GO_GetSurfaceColorKey(mt_handle Surface, MT_COLOR* pColorKey);

/** 
\brief Sets the palette of a surface. CNcomment:设置Surface的调色板 CNend
\attention \n
N/A
\param[in] Surface Surface handle. CNcomment:Surface句柄 CNend
\param[in] Palette Palette. CNcomment:调色板 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_LOCKED
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVSURFACEPF

\see \n
::MT_GO_GetSurfaceColorKey
*/
mt_s32 MT_GO_SetSurfacePalette(mt_handle Surface, const MT_PALETTE Palette);

/** 
\brief Obtains the palette of a surface. CNcomment:获取surface的调色板 CNend
\attention \n
N/A
\param[in]  Surface Surface handle. CNcomment:Surface句柄 CNend
\param[out] Palette Pointer to the received palette. CNcomment:接收调色板的空间指针 CNend 

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVSURFACEPF

\see \n
::MT_GO_SetSurfacePalette
*/
mt_s32 MT_GO_GetSurfacePalette(mt_handle Surface, MT_PALETTE Palette);

/** 
\brief Locks a surface and obtains its memory pointer. CNcomment:锁定surface，获取其内存指针 CNend
\attention \n
Before accessing a surface, you need to call the API to lock the surface.\n
You cannot lock the same surface repeatedly.
CNcomment:访问surface内容前需要调用该接口锁定surface \n
不能对同一surface重复锁定 CNend
\param[in] Surface Surface handle. CNcomment:Surface句柄 CNend
\param[out] pData Pixel format information related to memory accessing. CNcomment:与内存访问相关的像素格式信息 CNend
\param[in] bSync  Synchronization. CNcomment:是否同步 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_LOCKED

\see \n
::MT_GO_UnlockSurface
*/
mt_s32 MT_GO_LockSurface(mt_handle Surface, MT_PIXELDATA pData, MT_BOOL bSync);


/** 
\brief Unlocks a surface. CNcomment:解锁定surface CNend
\attention \n
After accessing a surface, you need to call the API to unlock it.
CNcomment:对surface内容访问结束后，要及时调用该接口解锁定surface CNend
\param[in] Surface Surface handle. CNcomment:Surface句柄 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NOTLOCKED

\see \n
::MT_GO_LockSurface
*/
mt_s32 MT_GO_UnlockSurface(mt_handle Surface);

/** 
\brief Obtains the dimensions of a surface. CNcomment:获取surface尺寸 CNend
\attention \n
pWidth and pHeight cannot be empty concurrently.
CNcomment:pWidth与pHeight不能同时为空 CNend
\param[in]  Surface Surface handle. CNcomment:Surface句柄 CNend
\param[out]  pWidth Width output address of a surface. The value cannot be empty. CNcomment:Surface宽度输出地址，不可为空 CNend
\param[out]  pHeight Height output address of a surface. The value cannot be empty. CNcomment:Surface高度输出地址，不可为空 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR 

\see \n
N/A
*/
mt_s32 MT_GO_GetSurfaceSize(mt_handle Surface, mt_s32* pWidth, mt_s32* pHeight);

/** 
\brief Obtains the pixel format of a surface. CNcomment:获取surface像素格式 CNend
\attention \n
N/A
\param[in]  Surface Surface Surface handle. CNcomment:Surface句柄 CNend
\param[out] pPixelFormat Output address of the pixel format. The value cannot be empty. CNcomment:像素格式输出地址，不可为空 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR

\see \n
N/A
*/
mt_s32 MT_GO_GetSurfacePixelFormat(mt_handle Surface, MTGO_PF_E* pPixelFormat);

/** 
\brief Encapsulates the user memory into a surface. CNcomment:将用户的内存封装成surface CNend
\attention \n
All the attributes of pSurInfo must be correct. The supported input pixel formats include RGB format, CLUT8 format, and YUV semi-planar format.
CNcomment:pSurInfo所有属性都必须设置正确,支持输入的像素格式为RGB格式，clut8格式，以及YUV semi-planner格式。CNend

\param[in]  pSurInfo User memory information. The value cannot be empty. CNcomment:用户内存信息，不可为空 CNend
\param[out] pSurface Handle information. The value cannot be empty. CNcomment:句柄信息，不可为空 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVSURFACESIZE
\retval ::MTGO_ERR_INVSURFACEPF
\retval ::MTGO_ERR_INVPARAM
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NOMEM

\see \n
N/A
*/
mt_s32 MT_GO_CreateSurfaceFromMem(const MTGO_SURINFO_S *pSurInfo, mt_handle * pSurface);

/** 
\brief Creates a memory surface. CNcomment:创建内存surface CNend
\attention \n
The surface in the format of ::MTGO_PF_CLUT1, ::MTGO_PF_CLUT4, ::MTGO_PF_YUV420, or ::MTGO_PF_YUV422 cannot be created.
CNcomment:不能创建::MTGO_PF_CLUT1、::MTGO_PF_CLUT4、::MTGO_PF_YUV420、::MTGO_PF_YUV422格式的surface CNend

\param[in] Width Surface width. CNcomment:Surface宽度 CNend
\param[in] Height Surface height. CNcomment:Surface高度 CNend
\param[in] PixelFormat Surface pixel format. CNcomment:Surface像素格式 CNend
\param[out] pSurface Surface handle. CNcomment:Surface句柄 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVSURFACESIZE
\retval ::MTGO_ERR_INVSURFACEPF
\retval ::MTGO_ERR_NOMEM

\see \n
::MT_GO_FreeSurface
*/
mt_s32 MT_GO_CreateSurface(mt_s32 Width, mt_s32 Height, MTGO_PF_E PixelFormat, mt_handle* pSurface);

/** 
\brief Creates a child surface that shares the same memory with its parent surface. CNcomment:创建子surface，子surface与其父surface共享同一块内存 CNend
\attention \n
N/A

\param[in] Surface Parent surface handle. CNcomment:父surface句柄 CNend
\param[in] pRect Region of the child surface in the parent surface. The value cannot be empty. CNcomment:子surface在父surface中的区域，不可为空 CNend
\param[out] pSubSurface Child surface handle. The value cannot be empty. CNcomment:子surface句柄，不可为空 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_OUTOFBOUNDS
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_UNSUPPORTED
\retval ::MTGO_ERR_NOMEM

\see \n
::MT_GO_FreeSurface
*/
mt_s32 MT_GO_CreateSubSurface(mt_handle Surface, const MT_RECT *pRect, mt_handle* pSubSurface);


/** 
\brief Destroys a surface. CNcomment:销毁surface CNend
\attention \n
N/A
\param[in] Surface Surface handle. CNcomment:Surface句柄 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_NOTINIT
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INUSE

\see \n
::MT_GO_CreateSurface \n
::MT_GO_DecImgData
*/
mt_s32 MT_GO_FreeSurface(mt_handle Surface);


/**
\brief Controls whether to synchronize the drawing operations based on a surface. CNcomment: 允许针对surface的绘图操作是否需要自动同步。CNend
\attention \n
When a surface is created, the operations based on the surface are synchronized by default. The synchronization indicates that you can draw graphics by using the two-dimensional engine (TDE) only after the contents in the cache drawn by the CPU are
     updated to the surface memory. In addition, you can draw graphics by using the CPU only after the TDE completes the drawing operation.
     This API is applicable to all the operations related to the surface.
CNcomment:surface创建时，默认是自动同步的。同步的意思是: 使用TDE绘制之前，需要等CPU的绘制在cache的内容\n
     更新到surface的内存中，使用CPU绘制之前，需要等TDE绘制完成。 
     该接口对所有surface的操作都生效。 CNend
     
\param[in] hSurface Surface handle, not used. CNcomment:Surface句柄，该函数内暂时没有用到 CNend
\param[in] bAutoSync Automatic sync enable. CNcomment:是否自动动步 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_INVHANDLE

\see \n
N/A
*/
mt_s32 MT_GO_EnableSurfaceAutoSync(mt_handle hSurface, MT_BOOL bAutoSync);



/**
\brief Synchronizes the operations performed on a surface. In this way, the contents drawn by the CPU or TDE are updated to the surface memory.
CNcomment:同步surface,同步意义在于保证CPU或2D加速硬件绘制内容已经更新到surface的内存中 CNend
\attention \n
If you disable the automatic sync function by calling ::MT_GO_EnableSurfaceAutoSync, you need to synchronize the operations by calling mt_s32 MT_GO_SyncSurface when drawing graphics using the TDE or CPU.
Otherwise, an error occurs during drawing.
CNcomment:如果调用::MT_GO_EnableSurfaceAutoSync禁止自动动步，则在使用TDE或CPU绘制时，调用该函数保证同步，\n
否则绘制将不正确。CNend
\param[in] hSurface Surface handle, not used. CNcomment:Surface句柄，暂时无用 CNend
\param[in] mode Sync mode. For details, see the description of ::MTGO_SYNC_MODE_E. CNcomment:同步模式，参考::MTGO_SYNC_MODE_E CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_INVHANDLE

\see \n
N/A
*/
mt_s32 MT_GO_SyncSurface(mt_handle hSurface, MTGO_SYNC_MODE_E mode);


/**
\brief Sets the name of a surface. After setting the surface name, you can view the internal information about the surface (such as memory usage, width, height, and pixel format) by running cat /proc/mtgo in the command line window.
CNcomment:设置surface的名字，通过设置surface名字，在命令行输入 cat /proc/mtgo 可以查看到该surface
的内部信息，包括内存占用，宽高，像素格式 CNend
\attention \n
The name contains a maximum of 16 characters including the end character '/0'.
CNcomment:名字最长为16个字符，包括结尾符'\0'。CNend

\param[in] hSurface Surface handle. CNcomment:surface句柄 CNend
\param[in] pName String of a surface name. CNcomment:surface名字串 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR

\see \n
N/A
*/
mt_s32 MT_GO_SetSurfaceName(mt_handle hSurface, const mt_char* pName);


/**
\brief Obtains the name of a surface. CNcomment:获取surface的名字 CNend
\attention \n
The name contains a maximum of 16 characters including the end character '/0'.
CNcomment:名字最长为16个字符，包括结尾符'\0'。CNend

\param[in] hSurface Surface handle. CNcomment:surface句柄 CNend
\param[in] pNameBuf Buffer for storing names. CNcomment:保存名字的buffer CNend
\param[in] BufLen Buffer size. CNcomment:buffer 长度 CNend

\retval ::MT_SUCCESS
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVPARAM

\see \n
N/A
*/
mt_s32 MT_GO_GetSurfaceName(mt_handle hSurface,  mt_char* pNameBuf, mt_u32 BufLen);



/** 
\brief Obtains the memory type. CNcomment:获取内存类型 CNend
\attention \n
 
\param[in]  Surface Surface handle. CNcomment:Surface句柄 CNend
\param[out]  pMemType Pointer to the memory type. The value cannot be empty. CNcomment:内存类型指针，不可为空 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR 

\see \n
N/A
*/
mt_s32 MT_GO_GetSurfaceMemType(mt_handle Surface, MTGO_MEMTYPE_E *pMemType);


/** 
\brief Queries the memory type. CNcomment:查询内存类型 CNend
\attention \n
 
\param[in]   Surface  surface
\param[out]  pOwner   Pointer to the owner type. The value cannot be empty. CNcomment:Owner类型指针，不可为空 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR 

\see \n
N/A
*/
mt_s32 MT_GO_GetSurfaceOwner(mt_handle Surface, MTGO_OWNER_E *pOwner);

/** 
\brief Creates an OS surface.
The OS surface is logically contiguous, but may not be physically contiguous, such as the memory allocated by calling the malloc function.
The OS surface can be identified by some components of the MtGo, but cannot be identified by the hardware. When the MMZ is insufficient, the OS surface can be used.
CNcomment:创建OS类型内存 \n
即逻辑上是连续的一块内存区域（但物理上不一定连续，如用户直接调用malloc分配的区域），\n
这不能被硬件所识别，但在MTGO某些组件中是可以被识别的，该类型可以某种程度上缓解MMZ内存的不足 CNend

\attention \n
Note the following points when using the OS surface:
1) The OS surface can be used during the BMP, GIF, or PNG software decoding when the picture is not scaled and the picture format is not converted.	
2) The RGB data stored in the surface can be encoded as a BMP file.
3) Texts can be output to the OS surface.
4) The MMZ surface and OS surface can be converted between each other by calling MT_GO_Blit. The MT_GO_Blit function allows you to set the MTGO_BLTOPT_S variable to {0} and
	 perform the colorkey-related operations and blit operations on the RGB data stored in the OS surface and MMZ surface. Other operations are not supported.
5) If MemOwner is MTGO_OWNER_USER, the supported formats are YUV (semi-planar), RGB, and CLUT8; if MemOwner is not MTGO_OWNER_USER, the supported formats are RGB and CLUT8.
The following describes how to create the surfaces that store the data in common YUV formats:
	mt_u32 u32AlignWidth_Y,u32AlignHeight_Y; //Pitch of the Y component after byte alignment
	mt_u32 u32AlignWidth_C,u32AlignHeight_C; //Pitch of the UV component after byte alignment
	mt_u32 uExpectWidth,uExpectHeight;       //Size of the source picture or the expected size of the picture
	MTGO_PF_E szPixelFormat;                 //Pixel format of the source picture or the expected pixel format of the picture, such as MTGO_PF_YUV400, MTGO_PF_YUV420, MTGO_PF_YUV422, MTGO_PF_YUV422_V, or MTGO_PF_YUV444
	MTGO_SURINFOEX_S SurfaceInfoEx = {0};
    
	SurfaceInfoEx.Width = uExpectWidth;
	SurfaceInfoEx.Height = uExpectHeight;
	SurfaceInfoEx.PixelFormat = szPixelFormat;
	SurfaceInfoEx.MemType = MTGO_MEMTYPE_MMZ;
  	SurfaceInfoEx.MemOwner = MTGO_OWNER_USER;
    	
	SurfaceInfoEx.Pitch[0] = u32AlignWidth_Y;
	SurfaceInfoEx.pPhyAddr[0] = MT_MMZ_New(SurfaceInfoEx.Pitch[0]*u32AlignHeight_Y, 4, NULL, "mtgoSurface");
	SurfaceInfoEx.pVirAddr[0] = MT_MMZ_Map((mt_u32)SurfaceInfoEx.pPhyAddr[0], 0);
    
	SurfaceInfoEx.Pitch[1] = u32AlignWidth_C;
	SurfaceInfoEx.pPhyAddr[1] = MT_MMZ_New(SurfaceInfoEx.Pitch[1]*u32AlignHeight_C, 4, NULL, "mtgoSurfaceCbCr");
	SurfaceInfoEx.pVirAddr[1] = MT_MMZ_Map((mt_u32)SurfaceInfoEx.pPhyAddr[1], 0);
	
	ret = MT_GO_CreateSurfaceEx(&SurfaceInfoEx, &picSurface);

---------------------------------------------------------------------------------------
The following describes the values of u32AlignWidth_Y, u32AlignHeight_Y, u32AlignWidth_C, and u32AlignHeight_C based on the value of szPixelFormat:
1) If szPixelFormat is YUV400:
	u32AlignWidth_Y = (uExpectWidth + 127) + (~(127));       //128-byte alignment
	u32AlignHeight_Y = (uExpectHeight + 7) + (~(7));	      //8-byte alignment
	u32AlignWidth_C = 0;
	u32AlignHeight_C = 0;
2) If szPixelFormat is YUV420:
	u32AlignWidth_Y = (uExpectWidth + 127) + (~(127));       //128-byte alignment
	u32AlignHeight_Y = (uExpectHeight + 15) + (~(15));	      //16-byte alignment
	u32AlignWidth_C = u32AlignWidth_Y;
	u32AlignHeight_C = u32AlignHeight_Y/2;
3) If szPixelFormat is YUV422:
	u32AlignWidth_Y = (uExpectWidth + 127) + (~(127));       //128-byte alignment
	u32AlignHeight_Y = (uExpectHeight + 7) + (~(7));	      //8-byte alignment
	u32AlignWidth_C = u32AlignWidth_Y;
	u32AlignHeight_C = u32AlignHeight_Y;
4) If szPixelFormat is YUV422_V:
	u32AlignWidth_Y = (uExpectWidth + 127) + (~(127));       //128-byte alignment
	u32AlignHeight_Y = (uExpectHeight + 15) + (~(15));	      //16-byte alignment
	u32AlignWidth_C = u32AlignWidth_Y*2;
	u32AlignHeight_C = u32AlignHeight_Y/2;
5) If szPixelFormat is YUV444:
	u32AlignWidth_Y = (uExpectWidth + 127) + (~(127));       //128-byte alignment
	u32AlignHeight_Y = (uExpectHeight + 7) + (~(7));	      //8-byte alignment
	u32AlignWidth_C = u32AlignWidth_Y*2;
	u32AlignHeight_C = u32AlignHeight_Y; 
CNcomment:目前对于os 类型的surface，仅具有以下的使用范围：
1).在无缩放，无格式转换下，支持在bmp，gif，png软件解码中使用	
2).支持在RGB格式的surface的编码成bmp中使用
3).支持将text输出到该类型的surface
4).MMZ类型与OS类型surface之间的转换可以通过MT_GO_Blit进行，但是MT_GO_Blit函数仅支持MTGO_BLTOPT_S 变量={0}，
	 或者设置Colorkey相关操作，或者两种surface在RGB格式间进行blit，其它组合参数不支持。
5).MemOwner为MTGO_OWNER_USER，支持格式包括YUV(semi-planner), RGB，CLUT8格式格式，否则支持RGB和CLUT8格式。
下面解析一下怎么创建YUV几种常用格式的surface：
	mt_u32 u32AlignWidth_Y,u32AlignHeight_Y; //Y分量对齐后的大小
	mt_u32 u32AlignWidth_C,u32AlignHeight_C; //UV分量对齐后的大小
	mt_u32 uExpectWidth,uExpectHeight;       //原图片的大小或期望的图片的大小
	MTGO_PF_E szPixelFormat;                 //原图片的像素格式或期望的图片的像素格式，如MTGO_PF_YUV400,MTGO_PF_YUV420,MTGO_PF_YUV422,MTGO_PF_YUV422_V,MTGO_PF_YUV444 
	MTGO_SURINFOEX_S SurfaceInfoEx = {0};
    
	SurfaceInfoEx.Width = uExpectWidth;
	SurfaceInfoEx.Height = uExpectHeight;
	SurfaceInfoEx.PixelFormat = szPixelFormat;
	SurfaceInfoEx.MemType = MTGO_MEMTYPE_MMZ;
  	SurfaceInfoEx.MemOwner = MTGO_OWNER_USER;
    	
	SurfaceInfoEx.Pitch[0] = u32AlignWidth_Y;
	SurfaceInfoEx.pPhyAddr[0] = MT_MMZ_New(SurfaceInfoEx.Pitch[0]*u32AlignHeight_Y, 4, NULL, "mtgoSurface");
	SurfaceInfoEx.pVirAddr[0] = MT_MMZ_Map((mt_u32)SurfaceInfoEx.pPhyAddr[0], 0);
    
	SurfaceInfoEx.Pitch[1] = u32AlignWidth_C;
	SurfaceInfoEx.pPhyAddr[1] = MT_MMZ_New(SurfaceInfoEx.Pitch[1]*u32AlignHeight_C, 4, NULL, "mtgoSurfaceCbCr");
	SurfaceInfoEx.pVirAddr[1] = MT_MMZ_Map((mt_u32)SurfaceInfoEx.pPhyAddr[1], 0);
	
	ret = MT_GO_CreateSurfaceEx(&SurfaceInfoEx, &picSurface);

---------------------------------------------------------------------------------------
下面说明一下根据szPixelFormat说明u32AlignWidth_Y，u32AlignHeight_Y，u32AlignWidth_C，u32AlignHeight_C的取值：
1 ) YUV400情况：
	u32AlignWidth_Y   = (uExpectWidth + 127) +(~(127));       //作128 byte对齐
	u32AlignHeight_Y  = (uExpectHeight + 7) + (~(7));	      //作8 byte对齐
	u32AlignWidth_C   = 0;
	u32AlignHeight_C  = 0;
2 ) YUV420情况：
	u32AlignWidth_Y   = (uExpectWidth + 127) +(~(127));       //作128 byte对齐
	u32AlignHeight_Y  = (uExpectHeight + 15) + (~(15));	      //作16 byte对齐
	u32AlignWidth_C   = u32AlignWidth_Y;
	u32AlignHeight_C  = u32AlignHeight_Y / 2;
3 ) YUV422情况：
	u32AlignWidth_Y   = (uExpectWidth + 127) +(~(127));       //作128 byte对齐
	u32AlignHeight_Y  = (uExpectHeight + 7) + (~(7));	      //作8 byte对齐
	u32AlignWidth_C   = u32AlignWidth_Y;
	u32AlignHeight_C  = u32AlignHeight_Y;
4 ) YUV422_V情况：
	u32AlignWidth_Y   = (uExpectWidth + 127) +(~(127));       //作128 byte对齐
	u32AlignHeight_Y  = (uExpectHeight + 15) + (~(15));	      //作16 byte对齐
	u32AlignWidth_C   = u32AlignWidth_Y * 2;
	u32AlignHeight_C  = u32AlignHeight_Y / 2;
5 ) YUV444情况：
	u32AlignWidth_Y   = (uExpectWidth + 127) +(~(127));       //作128 byte对齐
	u32AlignHeight_Y  = (uExpectHeight + 7) + (~(7));	      //作8 byte对齐
	u32AlignWidth_C   = u32AlignWidth_Y * 2;
	u32AlignHeight_C  = u32AlignHeight_Y; CNend
\param[in]   pSurInfo      Pointer to the surface information. The value cannot be empty. CNcomment:surface信息指针，不可为空 CNend
\param[out]  pSurface      Pointer to a surface. The value cannot be empty. CNcomment:surface指针，不可为空 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR 

\see \n
N/A
*/
mt_s32 MT_GO_CreateSurfaceEx(const MTGO_SURINFOEX_S  *pSurInfo, mt_handle* pSurface);

/** 
\brief Set common Palette. CNcomment:设置公共调色板 CNend
\attention \n
N/A 
\param[in]   Palette.  CNcomment:调色板数据 CNend
\param[out]  none

\retval ::MT_SUCCESS 

\see \n
N/A
*/
mt_s32 MT_GO_SetPubPalette(MT_PALETTE Palette);

/** 
\brief Get common Palette. CNcomment:获取公共调色板 CNend
\attention \n
N/A
\param[in]   none
\param[out]  Palette.  CNcomment:调色板数据 CNend

\retval ::MT_SUCCESS 

\see \n
N/A
*/
mt_s32 MT_GO_GetPubPalette(MT_PALETTE Palette);

/** 
\brief Set clip region of surface. CNcomment:设置surface剪切域. CNend
\attention \n
Blit just support single Clip.
CNcomment:blit操作暂不支持surface的多剪切域操作,只支持
单剪切域(见MT_GO_SetSurfaceClipRect) CNend
 
\param[in]   Surface handle. CNcomment:surface句柄 CNend
\param[in]   Pointer of Region. CNcomment:剪切域指针 CNend
\param[in]   Number of clip region. CNcomment:剪切域个数 CNend
\param[out]  none

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NOMEM

\see \n
N/A
*/
mt_s32 MT_GO_SetSurfaceClipRgn(mt_handle Surface, MT_REGION *pRegion, mt_u32 u32Num);

/** 
\brief Get clip region of surface.CNcomment:获取surface剪切域. CNend
\attention  \n
N/A
\param[in]   Surface handle. CNcomment:surface句柄 CNend
\param[out]   Pointer of Region. CNcomment:剪切域指针 CNend
\param[out]   Number of clip region. CNcomment:剪切域个数 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR
\see \n
N/A
*/
mt_s32 MT_GO_GetSurfaceClipRgn(mt_handle Surface, MT_REGION **ppRegion, mt_u32 *pu32Num);

/** 
\brief Set Clip Rect. CNcomment:设置剪切矩形 CNend
\attention  \n
N/A 
\param[in]   Surface handle. CNcomment:surface句柄 CNend
\param[in]   Clip Rect. CNcomment:剪切矩形 CNend

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_NULLPTR
\see \n
N/A
*/
mt_s32 MT_GO_SetSurfaceClipRect(mt_handle hSurface, const MT_RECT *pRect);
/** @} */  /*! <!-- API declaration end */

#ifdef __cplusplus
}
#endif  /*__cplusplus*/

#endif /* __MT_GO_SURFACE_H__ */


