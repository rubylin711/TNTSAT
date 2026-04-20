/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_JPEG_HAL_API_H__
#define __MT_JPEG_HAL_API_H__


/*********************************add include here******************************/
#include "mt_type.h"


/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C" 
{
#endif
#endif /* __cplusplus */


    /***************************** Macro Definition ******************************/
	#define VID_CMD_MAGIC     'j' 
	
	#define CMD_JPG_GETDEVICE        _IO(VID_CMD_MAGIC,   0x0)
	#define CMD_JPG_RELEASEDEVICE    _IO(VID_CMD_MAGIC,   0x1)

	#define CMD_JPG_GETINTSTATUS     _IOWR(VID_CMD_MAGIC, 0x2, JPG_GETINTTYPE_S *)

	#define CMD_JPG_SUSPEND          _IO(VID_CMD_MAGIC,   0x3)
	#define CMD_JPG_RESUME           _IO(VID_CMD_MAGIC,   0x4)

	#define CMD_JPG_GETRESUMEVALUE  _IOWR(VID_CMD_MAGIC, 0x5, unsigned int *)
	#define CMD_JPG_GETSUSPEND       _IO(VID_CMD_MAGIC,   0x6)
	#define CMD_JPG_GETRESUME        _IO(VID_CMD_MAGIC,   0x8)
	

	#define CMD_JPG_RESET            _IO(VID_CMD_MAGIC,   0x9)
	#define CMD_JPG_CANCEL_RESET    _IO(VID_CMD_MAGIC,   0x10)

	#define CMD_JPG_READPROC         _IOWR(VID_CMD_MAGIC, 0x11, MT_JPEG_PROC_INFO_S *)

    #define CMD_JPG_WRITE_REGVALUE   _IOWR(VID_CMD_MAGIC, 0x12, unsigned int *)
	#define CMD_JPG_READ_REGVALUE    _IO(VID_CMD_MAGIC,   0x13)

    /*************************** Structure Definition ****************************/
	/** halt state types **/
	typedef enum mtJPG_INTTYPE_E
	{
	
		JPG_INTTYPE_NONE       = 0,	   /** none halt happen	  **/
		JPG_INTTYPE_CONTINUE = 0x1,	           /** continue stream halt **/		
		JPG_INTTYPE_FINISH = 0x2, 	           /** finish halt		  **/	          
		JPG_INTTYPE_ERROR = 0x4,		           /** error halt 		  **/
		JPG_INTTYPE_MARKER = 0x8,                 /*marker halt*/		
		JPG_INTTYPE_BUTT
	}JPG_INTTYPE_E;
	
	/** get halt state struct **/
	typedef struct mtJPG_GETINTTYPE_S
	{
	
		JPG_INTTYPE_E IntType;	  /** halt type **/
		unsigned int TimeOut; 	  /** overtime	**/
		
	}JPG_GETINTTYPE_S;

	
 	/** structure of jpeg decode state */
    /** CNcomment:jpeg解码状态 */
	typedef enum mtJPEG_DEC_STATE_E
    {
        JPEG_DEC_FINISH_CREATE_DECOMPRESS  = 0, /**< create decompress finish   *//**<CNcomment:创建解码器结束  */
        JPEG_DEC_FINISH_STDIC               = 1, /**< stdio the stream  finish   *//**<CNcomment:关联码流结束    */
        JPEG_DEC_FINISH_READ_HEADER        = 2,  /**< read header file finish   *//**<CNcomment:解析文件结束    */
        JPEG_DEC_FINISH_START_DECOMPRESS   = 3, /**< start decompress finish    *//**<CNcomment:解码结束        */
        JPEG_DEC_FINISH_READ_SCANLINES     = 4, /**< read scanlines   finish    *//**<CNcomment:输出结束        */
        JPEG_DEC_FINISH_FINISH_DECOMPRESS  = 5, /**< finish decompress finish   *//**<CNcomment:完成结束        */
        JPEG_DEC_FINISH_DESTORY_DECOMPRESS = 6, /**< destory decompress finish  *//**<CNcomment:销毁解码器结束  */
        JPEG_DEC_SUCCESS                     = 7, /**< decode success             *//**<CNcomment:解码成功        */
        JPEG_DEC_STATE_BUTT
    }MT_JPEG_DEC_STATE_E;

 	/** structure of jpeg decode type */
    /** CNcomment:jpeg解码类型，是硬件解码还是软件解码 */
	typedef enum mtJPEG_DEC_TYPE_E
    {
    
        JPEG_DEC_HW = 0,    /**< hard decode   *//**<CNcomment:硬件解码  */
        JPEG_DEC_SW = 1,    /**< soft decode   *//**<CNcomment:软件解码  */
        JPEG_DEC_BUTT
    }MT_JPEG_DEC_TYPE_E;

	
	typedef struct mtJPG_SAVEINFO_S
	{
		mt_u32	 u32ResumeData0;
		mt_u32	 u32ResumeData1;
		mt_u32	 u32ResBitRemain;
		mt_u32	 u32ResByteConsu;
		mt_u32	 u32ResMcuy;
		mt_u32	 u32Pdy;
		mt_u32	 u32Pdcbcr;
	}MT_JPG_SAVEINFO_S;

  typedef struct mtJPG_PROC_REG_S
  {
    mt_u32 u32RegAddr;
    mt_u32 u32RegVal;
  }MT_JPG_PROC_S;

	/** structure of the proc information，the user need proc, 
	 ** echo proc on and we need echo proc on and then echo trace on */
    /** CNcomment:proc信息结构体，只给用户提供的只需要echo proc on，要是
     ** 要是自己调试只要先echo proc on 然后 echo trace on */
    typedef struct mtJPEG_PROC_INFO_S
    {
             /** our need proc information */
			 /** CNcomment:自己的调试信息  */
			 mt_u32  u32YWidth;         /**< the lu width       *//**<CNcomment:亮度宽宽度 */
			 mt_u32  u32YHeight;        /**< the lu height      *//**<CNcomment:亮度高度   */
			 mt_u32  u32YSize;          /**< the lu size        *//**<CNcomment:亮度大小   */
			 mt_u32  u32CWidth;         /**< the ch width       *//**<CNcomment:色度宽度   */
			 mt_u32  u32CHeight;        /**< the ch height      *//**<CNcomment:色度高度   */
			 mt_u32  u32CSize;          /**< the ch size        *//**<CNcomment:色度大小   */
			 mt_u32  u32YStride;        /**< the lu stride      *//**<CNcomment:亮度行间距 */
			 mt_u32  u32CbCrStride;     /**< the ch stride      *//**<CNcomment:色度行间距 */
			 mt_u32  u32DisplayW;       /**< the display width  *//**<CNcomment:显示宽度   */
			 mt_u32  u32DisplayH;       /**< the display height *//**<CNcomment:显示高度   */
             mt_u32  u32DisplayStride; /**< the display stride *//**<CNcomment:显示行间距  */
			 mt_u32  u32DecW;           /**< the dec width      *//**<CNcomment:解码宽度    */
			 mt_u32  u32DecH;           /**< the dec height     *//**<CNcomment:解码高度    */
			 mt_u32  u32DecStride;     /**< the dec stride     *//**<CNcomment:解码行间距  */
#if 0       
			 mt_u32  u32DataStartAddr; /**< the stream start address     *//**<CNcomment:码流buffer起始地址，要64字节对齐      */
			 mt_u32  u32DataEndAddr;   /**< the stream end address       *//**<CNcomment:码流buffer结束地址，要64字节对齐      */
			 mt_u32  u32SaveStartAddr; /**< the save data start address  *//**<CNcomment:存储码流的起始地址(在码流地址区域内)  */
			 mt_u32  u32SaveEndAddr;   /**< the save data end address    *//**<CNcomment:存储码流的结束地址(在码流地址区域内)  */
#endif       
             /** user need proc information */
			 /** CNcomment:用户想看到的信息 */
			 mt_u32 u32InWidth;             /**< the input widht             *//**<CNcomment:输入宽度    */
             mt_u32 u32InHeight;            /**< the input height            *//**<CNcomment:输入高度    */
             MT_BOOL bIsProgressive;        /**<progressive mode *//**<CNcomment:是否为progressive模式*/
             mt_u32 u32NumComponents;       /**<components number *//**<CNcomment:分量数目 */
             mt_u32 u32CompsInScan;       /**<components in scan *//**<CNcomment:scan个数 */
             mt_u32 u32OutWidth;            /**< the output width            *//**<CNcomment:输出宽度    */
             mt_u32 u32OutHeight;           /**< the output height           *//**<CNcomment:输出高度    */
			 mt_u32 u32OutStride;           /**< the output stride           *//**<CNcomment:输出行间距  */
             mt_u32 u32InFmt;               /**< the input format            *//**<CNcomment:输入像素格式 */
			 mt_u32 u32OutFmt;              /**< the output format           *//**<CNcomment:输出像素格式 */
             phys_addr_t u32OutPhyBuf;           /**< the output physics address  *//**<CNcomment:输出物理地址 */
			 mt_u32 u32Scale;               /**< the decode scale            *//**<CNcomment:解码缩放比例  */
			 MT_JPEG_DEC_STATE_E eDecState; /**< the decode state            *//**<CNcomment:解码状态      */
             MT_JPEG_DEC_TYPE_E eDecodeType;/**< the decode type             *//**<CNcomment:解码类型      */
			 MT_JPG_PROC_S stJpgReg[4][JCODEC_REG_NUM];
    }MT_JPEG_PROC_INFO_S;

    /********************** Global Variable declaration **************************/
 
    /******************************* API declaration *****************************/

	
    /****************************************************************************/



#ifdef __cplusplus
    
#if __cplusplus
   
}
#endif
#endif /* __cplusplus */

#endif /* __MT_JPEG_HAL_API_H__*/
