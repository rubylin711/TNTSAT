/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
@~chinese
@file mt_unf_flash.h
@brief flash模块头文件
@details flash模块相关的接口与参数定义

@~english
@file mt_unf_flash.h
@brief the header file of Flash


*/
#ifndef __MT_UNF_FLASH_H__
#define __MT_UNF_FLASH_H__

#include "mt_common.h"
#include <mtd/mtd-abi.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif 

#define MT_FLASH_NOT_OPEN		(-2)
#define MT_FLASH_ERR_PARAM	(-3)
#define MT_FLASH_ERR_IO	(-5)


typedef enum  spi_prot_block_type
{
  /*!
      block unprotection
   */
  PRT_UNPROT_ALL,
   /*!
     block protection all
  */
  PRT_PROT_ALL,
  /*!
      block protection, up address 1/64
   */
  PRT_UPPER_1_64,
  /*!
      block protection, up address 1/32
   */
  PRT_UPPER_1_32,
   /*!
      block protection, up address 1/16
   */
  PRT_UPPER_1_16,
   /*!
       block protection, up address 1/8
    */
   PRT_UPPER_1_8,
   /*!
       block protection, up address 1/4
    */
   PRT_UPPER_1_4,
    /*!
       block protection, up address 1/2
    */
   PRT_UPPER_1_2,
    /*!
      block protection, up LOWER 1/256
   */
  PRT_LOWER_1_256,
   /*!
      block protection, up LOWER 1/128
   */
  PRT_LOWER_1_128,
  /*!
      block protection, up LOWER 1/64
   */
  PRT_LOWER_1_64,
  /*!
      block protection, up LOWER 1/32
   */
  PRT_LOWER_1_32,
   /*!
      block protection, up LOWER 1/16
   */
  PRT_LOWER_1_16,
  /*!
     block protection, up LOWER 1/8
  */
  PRT_LOWER_1_8,
  /*!
     block protection, up LOWER 1/4
  */
  PRT_LOWER_1_4,
  /*!
     block protection, up LOWER 1/2
  */
  PRT_LOWER_1_2,
  /*!
     block protection, up LOWER 3/4
  */
  PRT_LOWER_3_4,
  /*!
     block protection, up LOWER 7/8
  */
  PRT_LOWER_7_8,
  /*!
     block protection, up LOWER 15/16
  */
  PRT_LOWER_15_16,
   /*!
     block protection, up LOWER 31/32
  */
  PRT_LOWER_31_32,
   /*!
     block protection, up LOWER 63/64
  */
  PRT_LOWER_63_64,
   /*!
     block protection, up LOWER 127/128
  */
  PRT_LOWER_127_128,
  /*!
     block protection, up address 3/4
  */
  PRT_UPPER_3_4,
  /*!
     block protection, up address 7/8
  */
  PRT_UPPER_7_8,
  /*!
     block protection, up address 15/16
  */
  PRT_UPPER_15_16,
   /*!
     block protection, up address 31/32
  */
  PRT_UPPER_31_32,
   /*!
     block protection, up address 63/64
  */
  PRT_UPPER_63_64,

   /*!
     block protection, BLOCK0
  */
  PRT_BLOCK_0,
}spi_prot_block_type_t;


//spi flash mamufactory id
#define FLASH_MF_MX         0xC2        //MACRONIX
#define FLASH_MF_ATMEL      0x1F        //ATMEL
#define FLASH_MF_ESMT       0x8C        //ESMT
#define FLASH_MF_SST        0xBF        //SST
#define FLASH_MF_WINB       0xEF        //WINBOND
#define FLASH_MF_SPAN       0x01        //SPANSION
#define FLASH_MF_EON        0x1C        //EON
#define FLASH_MF_AMIC       0x37        //AMIC
#define FLASH_MF_PFLASH     0x7F        //pFlash
#define FLASH_MF_GD         0xC8        //GigaDevice
#define FLASH_MF_PG         0xE0        //Paragon
#define FLASH_MF_FD         0xA1        //FuDan / FORESEE
#define FLASH_MF_BG         0xE0        //Berg
#define FLASH_MF_DQ         0x54        //DouQI
#define FLASH_MF_DS         0xF8        //DOSILICON
#define FLASH_MF_XTX        0x0B        //XTX
#define FLASH_MF_XMC        0x20        //XMC
#define FLASH_MF_ZB         0x5e        //ZB
#define FLASH_MF_HH         0x68        //HUAHONG,Boya
#define FLASH_MF_PUYA      0x85        //PUYA
#define FLASH_MF_ISSI      0x9d        //ISSI
#define FLASH_MF_ZETTA       0xba    //ZETTA

//snf_protect_info

typedef struct charsto_prot_status
{
  /*!
      status reg len
   */
  spi_prot_block_type_t prt_t;
  /*!
      status protect status
   */
  unsigned int st;
  /*!
      status protect status mask
   */
  unsigned int st_mask;
  /*!
      status reg len
   */
  unsigned char len;
}charsto_prot_status_t;

typedef struct snf_protect_info
{
    unsigned short m_id;       //manufactor id
    unsigned short d_id;       //device id

    charsto_prot_status_t prot_st;

}snf_protect_info_t;

/*
msg: flash callback message type
param: message param
*/
typedef void (*flash_msg_callback)(MT_U32 msg,MT_U64 param);


/*!
@~chinese
@addtogroup flash_api_declare  Flash
@{
@brief Flash模块读写擦操作接口说明
@details \n
Flash模块读写擦操作接口说明

@~english
@addtogroup flash_api_declare  Flash
@{
@brief Flash API description


*/
/*!
@~chinese
@brief 初始化flash设备
@param [in] 
@return ::MT_SUCCESS
@~english
@brief initialize flash device object,and return the flash device handle
@param [in] 
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_init(mt_void);
/*!
@~chinese
@brief 终止flash设备，如果有分区设备句柄未关闭，强行关闭分区设备句柄。
@param [in] 
@return ::MT_SUCCESS
@~english
@brief deinitialize the flash device object,and release its resources.
@param [in] 
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_deinit(mt_void);
/*!
@~chinese
@brief 打开flash mtd分区设备，根据mtd分区原始设备名如/dev/mtdxxx,打开该分区原始设备文件，得到该分区信息(flash类型等)，并将设备句柄及分区信息保存到
对应的分区信息结构中，并以该结构的地址为分区句柄传递给用户。
@param [in] const MT_CHAR* pMtdDevFile ::mtd分区原始设备名如/dev/mtdxxx
@param [in] MT_HANDLE* pMtdDevHandleOut ::分区句柄
@return ::MT_SUCCESS
@~english
@brief open the flash mtd char device according to the mtd  raw name,ie /dev/mtdxxx,
  and obtain the mtd char patition info(the type and real size of flash,block info,page info,badblock info etc),
  alloc a struct for these information,at last return the addressof structure as the flash partition access handle.
@param [in] const MT_CHAR* pMtdDevFile ::mtd partition char device name eg./dev/mtdxxx
@param [in] MT_HANDLE* pMtdDevHandleOut ::the mtd partition access handle.
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_open(const MT_CHAR* pMtdDevFile, MT_HANDLE* pMtdDevHandleOut);
/*!
@~chinese
@brief 关闭flash mtd分区设备，关闭分区设备句柄
@param [in] MT_HANDLE* pMtdDevHandleOut ::分区句柄
@return ::MT_SUCCESS
@~english
@brief close the flash mtd partition device,free the mtd partion access structure.
@param [in] MT_HANDLE* pMtdDevHandleOut ::the mtd partition access handle.
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_close(MT_HANDLE MtdDevHandleOut);
/*!
@~chinese
@brief 分区设备文件数据读操作，即从该mtd分区flash物理地址u32Addr开始读长度为u32Length的数据到内存pu8Buf中去。
如果u32Addr处有一个或多个坏块，将跳过这些坏块，直到好块并开始读取，如果其后出现坏块也将跳过坏块，读取好块数据直到有效数据数到达u32Length为止。
u32Addr+u32Length不可超过该分区有效数据长度
(推荐将整个分区有效数据全部读出)
@param [in] MT_HANDLE pMtdDevHandleOut ::该分区句柄
@param [in] MT_U32 u32Addr ::在该分区内开始读取数据的flash地址
@param [in] MT_U8 *pu8Buf ::数据将被放置的内存地址
@param [in] MT_U32 u32Length::将读取到的有效数据长度
@return ::MT_SUCCESS
@return ::MT_FAILURE
@~english
@brief flash data read from the partition.that is, read flash data from flash addrss u32Addr length=u32Length to memory that address pu8Buf.
if there are one or more bad block at u32Addr,function will jump over these bad block util there are good block appear.if there are bad blocks
after that, the function will also jump over thes bad blocks read good data blocks util effective data size arrive u32Length.
@param [in] MT_HANDLE pMtdDevHandleOut ::the mtd partition access handle.
@param [in] MT_U32 u32Addr ::flash address to be read.
@param [in] MT_U8 *pu8Buf ::memory address to be written.
@param [in] MT_U32 u32Length::data length to be read.
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
MT_S32 mt_unf_flash_read(MT_HANDLE DevHandle,MT_U32 u32Addr, MT_U8 *pu8Buf, MT_U32 u32Length);
/*!
@~chinese
@brief 分区设备文件数据写操作，即从内存pu8Buf开始长度为u32Length的数据写到该mtd分区flash开始地址u32Addr中去
如果u32Addr处有一个或多个坏块，将跳过这些坏块，直到好块并开始写，如果其后出现坏块也将跳过坏块，
写好块数据直到有效数据数到达u32Length为止?如果在擦或写nand flash时出现坏块，将标记该块为坏块，
并将找寻到下一个好块将数据读出，并写入到更下一好块中做数据拯救。也就是移动所有数据到下一块
为新数据空出可写位置。
u32Addr+u32Length不可超过该分区有效数据长度
(由于使用者可能操作的是整个flash 比如访问 all_flash分区 ，分区很大，且包含不该移动的分区数据。
数据拯救将花费较长时间，且移动不该移动数据，建议不使用该函数)
@param [in] MT_HANDLE pMtdDevHandleOut ::该分区句柄
@param [in] MT_U32 u32Addr ::在该分区内开始写数据的flash地址
@param [in] MT_U8 *pu8Buf ::数据所在的内存起始地址
@param [in] MT_U32 u32Length::将写的有效数据长度
@return ::MT_SUCCESS
@return ::MT_FAILURE
@~english
@brief flash data write operation, that is, write data from memory address pu8Buf data length=u32Length to flash address u32Addr in the flash
partition. if there are one or more bad blocks at flash address u32Addr ,the function will jump over these bad blocks util there are good blocks to write.
if later there are bad blocks ,the function will jump over these bad blocks to search enough good blocks to write data until the written effective data length 
arrive u32Length.

@param [in] MT_HANDLE pMtdDevHandleOut ::the mtd partition access handle.
@param [in] MT_U32 u32Addr ::flash address to be written.
@param [in] MT_U8 *pu8Buf ::memory address to be read.
@param [in] MT_U32 u32Length::data length to be written.
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
MT_S32 mt_unf_flash_write(MT_HANDLE DevHandle,MT_U32 u32Addr, MT_U8 *pu8Buf, MT_U32 u32Length);

/*!
@~chinese
@brief 分区设备文件数据写操作，不包含擦除，仅仅是写操作。即从内存pu8Buf开始长度为u32Length的数据写到该mtd分区flash开始地址u32Addr中去
如果u32Addr处有一个或多个坏块，将跳过这些坏块，直到好块并开始写，如果其后出现坏块也将跳过坏块，
写好块数据直到有效数据数到达u32Length为止?如果在擦或写nand flash时出现坏块，将标记该块为坏块，
并将找寻到下一个好块将数据读出，并写入到更下一好块中做数据拯救。也就是移动所有数据到下一块
为新数据空出可写位置。
u32Addr+u32Length不可超过该分区有效数据长度
(由于使用者可能操作的是整个flash 比如访问 all_flash分区 ，分区很大，且包含不该移动的分区数据。
数据拯救将花费较长时间，且移动不该移动数据，建议不使用该函数)
@param [in] MT_HANDLE pMtdDevHandleOut ::该分区句柄
@param [in] MT_U32 u32Addr ::在该分区内开始写数据的flash地址
@param [in] MT_U8 *pu8Buf ::数据所在的内存起始地址
@param [in] MT_U32 u32Length::将写的有效数据长度
@return ::MT_SUCCESS
@return ::MT_FAILURE
@~english
@brief flash data write operation, only write operation.that is, write data from memory address pu8Buf data length=u32Length to flash address u32Addr in the flash
partition. if there are one or more bad blocks at flash address u32Addr ,the function will jump over these bad blocks util there are good blocks to write.
if later there are bad blocks ,the function will jump over these bad blocks to search enough good blocks to write data until the written effective data length 
arrive u32Length.

@param [in] MT_HANDLE pMtdDevHandleOut ::the mtd partition access handle.
@param [in] MT_U32 u32Addr ::flash address to be written.
@param [in] MT_U8 *pu8Buf ::memory address to be read.
@param [in] MT_U32 u32Length::data length to be written.
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
MT_S32 mt_unf_flash_write_only(MT_HANDLE DevHandle,MT_U32 u32Addr, MT_U8 *pu8Buf, MT_U32 u32Length);


/*!
@~chinese
@brief 分区设备文件数据写操作，即从内存pu8Buf开始长度为u32Length的数据写到该mtd分区flash开始地址u32Addr结束地址必须是endAddr的flash中去
如果u32Addr处有一个或多个坏块，将跳过这些坏块，直到好块并开始写，如果其后出现坏块也将跳过坏块，写好块数据直到有效数据数到达u32Length为止?如果在擦或写nand flash时出现坏块，将标记该块为坏块，并将找寻到下一个好块将数据读出，并写入到更下一好块中做数据拯救。也就是移动所有数据到下一块
为新数据空出可写位置。但数据拯救也将直到endAddr截止。
u32Addr+u32Length不可超过该分区有效数据长度，也不能超过endAddr。
(该函数适合分区升级，u32Addr为新分区起始地址，endAddr为新分区结束地址， u32Length要小于 endAddr-u32Addr,以便预留出出现坏块时尚可使用的空间)
@param [in] MT_HANDLE pMtdDevHandleOut ::该分区句柄
@param [in] MT_U32 u32Addr ::在该分区内开始写数据的flash地址
@param [in] MT_U8 *pu8Buf ::数据所在的内存起始地址
@param [in] MT_U32 u32Length::将写的有效数据长度
@param [in] MT_U32 endAddr:: 写结束和做数据拯救的界定地址
@return ::MT_SUCCESS
@return ::MT_FAILURE
@~english
@brief flash data write operation, that is, write data from memory address pu8Buf data length=u32Length to flash address u32Addr in the flash
partition. if there are one or more bad blocks at flash address u32Addr ,the function will jump over these bad blocks util there are good blocks to write.
if later there are bad blocks ,the function will jump over these bad blocks to search enough good blocks to write data until the written effective data length 
arrive u32Length.

@param [in] MT_HANDLE pMtdDevHandleOut ::the mtd partition access handle.
@param [in] MT_U32 u32Addr ::flash address to be written.
@param [in] MT_U8 *pu8Buf ::memory address to be read.
@param [in] MT_U32 u32Length::data length to be written.
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
MT_S32 mt_unf_flash_write_ext(MT_HANDLE DevHandle,MT_U32 startAddr, MT_U8 *pu8Buf, MT_U32 u32Length,MT_U32 endAddr);
/*!
@~chinese
@brief 该函数是为如yaffs等写oob区的img烧写功能准备的，yaffs2 img文件是由一条条记录数据组成的，每条记录数据包括一页(如2048bytes/page)和64bytes oob数据组成。
该yaffs2 img 文件的每条记录一页数据烧写到对应spi nand flash 的一页中，oob data烧写到对应的oob区中。所以该文件长度是(2048+64)的整数倍。
分区设备文件数据写操作，即从内存pu8Buf开始长度为u32Length的数据及oob数据写到该mtd分区flash开始地址u32Addr结束地址必须是endAddr的flash的数据区及oob区中去
如果u32Addr处有一个或多个坏块，将跳过这些坏块，直到好块并开始写，如果其后出现坏块也将跳过坏块，
写好块数据直到有效数据数到达u32Length为止?如果在擦或写nand flash时出现坏块，将标记该块为坏块，并将找寻到下一个好块将数据读出，
并写入到更下一好块中做数据拯救。也就是移动所有数据到下一块
为新数据空出可写位置。但数据拯救也将直到endAddr截止。
u32Addr+u32Length不可超过该分区有效数据长度+整个分区oob区总和，也不能超过endAddr。
(该函数适合分区升级，u32Addr为新分区起始地址，endAddr为新分区结束地址， u32Length要小于 endAddr-u32Addr,以便预留出出现坏块时尚可使用的空间)
@param [in] MT_HANDLE pMtdDevHandleOut ::该分区句柄
@param [in] MT_U32 u32Addr ::在该分区内开始写数据的flash地址
@param [in] MT_U8 *pu8Buf ::数据所在的内存起始地址
@param [in] MT_U32 u32Length::将写的有效数据长度
@param [in] MT_U32 endAddr:: 写结束和做数据拯救的界定地址
@return ::MT_SUCCESS
@return ::MT_FAILURE

@~english
@brief flash data write operation, that is, write data from memory address pu8Buf data length=u32Length to flash address u32Addr in the flash
partition. if there are one or more bad blocks at flash address u32Addr ,the function will jump over these bad blocks util there are good blocks to write.
if later there are bad blocks ,the function will jump over these bad blocks to search enough good blocks to write data until the written effective data length 
arrive u32Length.
the function is dedicated to that img burn function including oob write. ie,yaffs filesystem will write file information to oob area. yaffs img file consist of 
many record data.every record data include a page data(2048bytes per page) and 64 bytes oob data.so this img file length is multiply of 2048+64 bytes.

@param [in] MT_HANDLE pMtdDevHandleOut ::the mtd partition access handle.
@param [in] MT_U32 u32Addr ::flash address to be written.
@param [in] MT_U8 *pu8Buf ::memory address to be read.
@param [in] MT_U32 u32Length::data length to be written.
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
MT_S32 mt_unf_flash_write_raw_ext(MT_HANDLE DevHandle,MT_U32 startAddr, MT_U8 *pu8Buf, MT_U32 u32Length,MT_U32 endAddr);
/*!
@~chinese
@brief 得到该mtd分区有效长度，即该分区去掉坏块后的可写数据长度
@param [in] MT_HANDLE* pMtdDevHandleOut ::分区句柄
@param [out] MT_U32 *FlashAvaLength ::待得到的分区有效长度
@return ::MT_SUCCESS
@~english
@brief obtain the effective length of the flash partition not including bad block.
@param [in] MT_HANDLE* pMtdDevHandleOut ::the mtd partition access handle.
@param [out] MT_U32 *FlashAvaLength ::effective mtd partition length to be obtained.
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_get_avalen(MT_HANDLE MtdDevHandle,MT_U32 *FlashAvaLength);
/*!
@~chinese
@brief 分区设备文件擦操作，即从mtd分区地址u32Addr开始擦除长度为u32Length(包括坏块)的flash内存。
@param [in] MT_HANDLE* pMtdDevHandleOut :: 分区句柄
@param [in] MT_U32 u32Length :: 擦除长度为u32Length(包括坏块)
@return ::MT_SUCCESS
(该函数用于擦除用户定义的新分区内的存储空间，请记住，我们的写接口也实现了擦除功能)
@~english
@brief flash block erase operation.that is, erase data block from flash address u32Addr length=u32 Length(including bad blocks).
@param [in] MT_HANDLE* pMtdDevHandleOut :: the mtd partition access handle.
@param [in] MT_U32 u32Addr:: flash address to be erase.
@param [in] MT_U32 u32Length :: flash length (including bad block) to be erase.
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_erase(MT_HANDLE MtdDevHandle,MT_U32 u32Addr,MT_U32 u32Length);

/*!
@~chinese
@brief 分区设备文件擦操作，即从mtd分区地址u32Addr开始擦除长度为u32Length的flash内存，仅用于spi nor flash。
@param [in] MT_HANDLE* pMtdDevHandleOut :: 分区句柄
@param [in] MT_U32 u32Length :: 擦除长度为u32Length
@return ::MT_SUCCESS
(该函数用于擦除用户定义的新分区内的存储空间，请注意，我们的一般写接口也实现了擦除功能)
@~english
@brief flash block erase operation.that is, erase data block from flash address u32Addr length=u32 Length.Only use for spi nor flash.
@param [in] MT_HANDLE* pMtdDevHandleOut :: the mtd partition access handle.
@param [in] MT_U32 u32Addr:: flash address to be erase.
@param [in] MT_U32 u32Length :: flash length (including bad block) to be erase.
@param [in] MT_U8 mode :: flash erase mode 0 --sector erase 1--half block erase.
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_nor_specialerase(MT_HANDLE MtdDevHandle,MT_U32 u32Addr,MT_U32 u32Length,MT_U8 mode);


/*!
@~chinese
@brief 获取flash保护配置信息
@param [in] MT_HANDLE* pMtdDevHandleOut :: 分区句柄
@param [in] MT_U32 *puBuf :: 获取信息缓存
@param [in] MT_U8 *u8Length :: 信息缓存有效长度
@param [in] spi_prot_block_type_t type :: flash保护信息类型
@return ::MT_SUCCESS
(该函数用于获取对应的flash的保护信息配置)
@~english
@brief get flash protect information
@param [in] MT_HANDLE* pMtdDevHandleOut :: the mtd partition access handle.
@param [in] MT_U32 *puBuf :: get flash information buffer
@param [in] MT_U8 *u8Length :: length for information
@param [in] spi_prot_block_type_t type  :: flash protect type
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_getlockcfg(MT_HANDLE MtdDevHandle,MT_U32 *puBuf,MT_U8 *u8Length,spi_prot_block_type_t type,MT_U32 *pro_mask);
/*!
@~chinese
@brief 获取flash的状态配置信息
@param [in] MT_HANDLE* pMtdDevHandleOut :: 分区句柄
@param [in] MT_U8 *pu8Buf ::获取的flash状态信息存放的缓存
@param [in] MT_U8 u8Length ::获取的flash状态信息存放的缓存的长度(最大的有效长度为2) 
@param [in] MT_U32 pro_mask ::获取的flash写保护mask
@return ::MT_SUCCESS
(该函数用于获取当前的flash的状态寄存器的配置信息)
@~english
@brief get flash status configure information.
@param [in] MT_HANDLE* pMtdDevHandleOut :: the mtd partition access handle.
@param [in] MT_U8 *pu8Buf:: flash information buffer.
@param [in] MT_U8 u8Length :: flash information buffer length (max valid length is 2).
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_flash_setpro_type_raw(MT_HANDLE MtdDevHandle);

/*!
@~chinese
@brief 测试flash的写保护设置
@param [in] MT_HANDLE* pMtdDevHandleOut :: 分区句柄
@return ::MT_SUCCESS
(该函数用于测试flash的写保护读写status reg)
@~english
@brief test flash status protect.
@param [in] MT_HANDLE* pMtdDevHandleOut :: the mtd partition access handle.
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_getstatus(MT_HANDLE MtdDevHandle,MT_U8 *pu8Buf,MT_U8 u8Length);
/*!
@~chinese
@brief 设置flash的状态配置信息
@param [in] MT_HANDLE* pMtdDevHandleOut :: 分区句柄
@param [in] MT_U8 *pu8Buf ::设置的flash状态信息存放的缓存
@param [in] MT_U8 u8Length ::设置的flash状态信息存放的缓存的长度(最大的有效长度为2) 
@return ::MT_SUCCESS
(该函数用于设置当前的flash的状态寄存器的配置信息)
@~english
@brief set flash status configure information.
@param [in] MT_HANDLE* pMtdDevHandleOut :: the mtd partition access handle.
@param [in] MT_U8 *pu8Buf:: flash information buffer.
@param [in] MT_U8 u8Length :: flash information buffer length (max valid length is 2).
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_setstatus(MT_HANDLE MtdDevHandle,MT_U8 *pu8Buf,MT_U8 u8Length);
/*!
@}
*/

/*!
@~chinese
@brief 获取flash 信息
@param [in] MT_HANDLE* pMtdDevHandleOut :: 分区句柄
@param [in] struct mtd_info_user *p_info ::获取的flash  信息存放的缓存
@return ::MT_SUCCESS
(该函数用于获取flash的信息)
@~english
@brief get flash id information.
@param [in] MT_HANDLE* pMtdDevHandleOut :: the mtd partition access handle.
@param [in] MT_U32 *p_info:: flash info buffer.
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_info(MT_HANDLE MtdDevHandle,struct mtd_info_user *p_info);

/*!
@~chinese
@brief 获取flash的id信息
@param [in] MT_HANDLE* pMtdDevHandleOut :: 分区句柄
@param [in] MT_U32 *id ::获取的flash id 信息存放的缓存
@return ::MT_SUCCESS
(该函数用于获取flash的id信息)
@~english
@brief get flash id information.
@param [in] MT_HANDLE* pMtdDevHandleOut :: the mtd partition access handle.
@param [in] MT_U32 *id:: flash id buffer.
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_flash_id(MT_HANDLE MtdDevHandle,MT_U32 *id);
/*!
@}
*/

MT_S32 mt_unf_flash_setprotect(MT_HANDLE MtdDevHandle,spi_prot_block_type_t type);
/*!
@~chinese
@brief 获取flash的id信息
@param [in] MT_HANDLE* pMtdDevHandleOut :: 分区句柄
@param [in] MT_U32 *id ::获取的flash id 信息存放的缓存
@return ::MT_SUCCESS
(该函数用于获取flash的id信息)
@~english
@brief get flash id information.
@param [in] MT_HANDLE* pMtdDevHandleOut :: the mtd partition access handle.
@param [in] MT_U32 *id:: flash id buffer.
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_flash_otp_read(MT_HANDLE MtdDevHandle,MT_U32 offset,MT_U32 len, u_char *buf);
MT_S32 mt_unf_flash_otp_write(MT_HANDLE MtdDevHandle,MT_U32 offset,MT_U32 len, u_char *buf);
MT_S32 mt_unf_flash_otp_lock(MT_HANDLE MtdDevHandle,MT_U32 offset);
MT_S32 mt_unf_flash_otp_erase(MT_HANDLE MtdDevHandle,MT_U32 offset);

MT_BOOL mt_unf_flash_isbadblock(MT_HANDLE MtdDevHandle,MT_U64 offset);
MT_S32 mt_unf_flash_mark_badblock(MT_HANDLE MtdDevHandle,MT_U64 offset);

MT_VOID mt_unf_flash_setcallback(MT_HANDLE MtdDevHandle,flash_msg_callback pcallback);

MT_S32 mt_unf_flash_otp_lock_getinfo(MT_HANDLE MtdDevHandle,struct otp_info *oinfo,MT_U32 max_cnt,MT_U32 *valid_cnt);
MT_S32 mt_unf_flash_id_uni(MT_HANDLE MtdDevHandle,MT_U8 *id_uni,MT_U32 len);

int mt_unf_flash_get_fhandle(MT_HANDLE DevHandle,int *fhandle);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif 

