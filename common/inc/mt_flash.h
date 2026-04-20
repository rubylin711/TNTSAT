/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

/**
 * \file
 * \brief describle the information about mtflash component. CNcomment:提供mtflash组件相关接口、数据结构信息。CNend
 * \attention \n
 * DO NOT write/erase flash too heavily using this mtflash interface because mtflash do not support "load balancing", use filesystems or implement "load balancing" instead if you have to. read is limitless.\n
 *       for example: if max write/erase times of MLC Nand chip is 3000, write/erase any fixed area must not exceed 3000 times, or read data may be wrong. \n\n
 * CNcomment:不要使用mtflash接口频繁write/erase flash，mtflash不支持读写均衡，如果必须要频繁write/erase操作，请使用文件系统，或者根据实际应用场景设计读写均衡算法。read操作无此限制。 \n
 *       比如: 如果MLC Nand的使用寿命为3000次，在产品整个生命周期中，对某个固定区域的write/erase操作不应该超过3000次，否则由于Nand失效读出数据可能错误。CNend
 */

#ifndef __MT_FLASH__H__
#define __MT_FLASH__H__
#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
/** \addtogroup      FLASH*/
/** @{ */  /** <!-- [FLASH] */

#define FLASH_NAME_LEN 32       /** Flash Name max length */

/** flash type */
typedef enum _mt_flash_type_t
{
    MT_FLASH_TYPE_SPI_0,    /**< SPI flash type */
    MT_FLASH_TYPE_NAND_0,   /**< NAND flash type */
    MT_FLASH_TYPE_EMMC_0,   /**< eMMC flash type */
    MT_FLASH_TYPE_BUTT      /**< Invalid flash type */
} mt_flash_type_t;

#define  INVALID_FD -1

#define  MT_FLASH_RW_FLAG_RAW           0x0   /** read and write without OOB,for example: kernel/uboot/ubi/cramfs.. */
#define  MT_FLASH_RW_FLAG_WITH_OOB      0x1   /** read and write with OOB, example: yaffs2 filesystem image */
#define  MT_FLASH_RW_FLAG_ERASE_FIRST   0x2   /** erase before write */

/** this macro for return value when nand flash have bad block or valid length less partition length */
/** CNcomment:当nand有坏块时,读/写/擦除时,有效长度可能小于分区大小/打开长度,此时为了不越界,读/写/擦除完有效长度后,返回该值 */
#define  MT_FLASH_END_DUETO_BADBLOCK    -10

/** Flash partition access permission type */
typedef enum access_perm
{
    ACCESS_NONE = 0,
    ACCESS_RD   = (1 << 1),
    ACCESS_WR   = (1 << 2),
    ACCESS_RDWR = (ACCESS_RD | ACCESS_WR),
    ACCESS_BUTT
} mt_flash_access_perm_t;

/** Flash partition descriptions */
typedef struct _mt_flash_partinfo_s
{
    mt_u64  start_addr;                  /**< Partiton start address */
    mt_u64  partsize;                   /**< Partition size */
    mt_u32  blocksize;                  /**< The Block size of the flash where this partition at */
    mt_flash_type_t flash_type;          /**< The flash type where this partition at */
    mt_char devname[FLASH_NAME_LEN];    /**< The device node name where this partition relate to */
    mt_char partname[FLASH_NAME_LEN];   /**< The partition name of this partition */
    mt_flash_access_perm_t perm;        /**< The partition access permission type */
} mt_flash_partinfo_s;

/** Flash operation descriptions */
typedef struct tag_flash_opt_s
{
    int (*raw_read)(int fd, unsigned long long *startaddr, unsigned char *buffer,
        unsigned long length, unsigned long long openaddr, unsigned long long limit_leng, int read_oob, int skip_badblock);
    int (*raw_write)(int fd, unsigned long long *startaddr, unsigned char *buffer,
        unsigned long length, unsigned long long openaddr, unsigned long long limit_leng, int write_oob);
    long long (*raw_erase)(int fd, unsigned long long startaddr,
        unsigned long long length, unsigned long long openaddr, unsigned long long limit_leng);
} flash_opt_s;

/** Flash Infomation */
typedef struct _mt_flash_interinfo_s
{
    mt_u64  total_size;                  /**< flash total size */
    mt_u64  partsize;                   /**< flash partition size */
    ulong  blocksize;                  /**< flash block size */
    ulong  pagesize;                   /**< flash page size */
    ulong  oobsize;                    /**< flash OOB size */
    ulong  fd;                         /**< file handle */ /**<CNcomment:文件句柄(按地址打开不能得到真实句柄)*/
    mt_u64  open_addr;                   /**< flash open address */
    mt_u64  open_len;                   /**< flash open length */
    mt_flash_type_t flash_type;          /**< flash type */
    flash_opt_s *p_flashopt;             /**< operation callbacks on this flash */
    mt_flash_partinfo_s *p_partinfo;     /**< parition descriptions on this flash */
} mt_flash_interinfo_s;

/** @} */  /** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup      FLASH */
/** @{ */  /** <!-- [FLASH] */


/**
\brief: open flash partiton
\attention \n
\param[in] flash_type      Flash type
\param[in] pPartitionName   CNcomment: 非EMMC器件(如SPI/NAND),只能用/dev/mtdx作为分区名。EMMC器件只能用/dev/mmcblk0px作为分区名。CNend
\param[in] addr       open address CNcomment:打开地址(当分区名无效即pPartitionName为空时使用)CNend
\param[in] len           open length CNcomment:打开长度(当分区名无效即pPartitionName为空时使用)CNend
\retval    fd               Flash handle
\retval    INVALID_FD       invaild fd
\see \n
*/
mt_s32 mt_flash_open(mt_flash_type_t flash_type, mt_char *partition_name, mt_u64 addr, mt_u64 len);

/**
\brief:  open flash patition by name
\attention \n
\param[in] pPartitionName   patition name CNcomment:非EMMC器件(如SPI/NAND),只能用/dev/mtdx作为分区名。EMMC器件只能用bootargs里blkdevparts=mmcblk0:中设定的分区名字，不能用/dev/mmcblk0px作为分区名CNend
\retval    fd               Flash hande
\retval    INVALID_FD       invaild fd
\see \n
*/
mt_handle mt_flash_openbyname(mt_char *partition_name);

/**
\brief: open flash patition by  type and name
\attention \n
\param[in] flash_type      flash type
\param[in] pPartitionName   patition name CNcomment: 非EMMC器件(如SPI/NAND),只能用/dev/mtdx作为分区名。EMMC器件只能用/dev/mmcblk0px作为分区名。CNend
\retval    fd               Flash hande
\retval    INVALID_FD       invaild fd
\see \n
*/
mt_s32 mt_flash_openby_type_and_name(mt_flash_type_t flash_type, mt_char *partition_name);

/**
\brief: open flash patition by  address
\attention \n
\param[in] flash_type       flash type
\param[in] addr        open address
\param[in] len            open length
\retval    fd                Flash hande
\retval    INVALID_FD        invaild fd
\see \n
*/
mt_s32 mt_flash_openby_type_and_addr(mt_flash_type_t flash_type, mt_u64 addr, mt_u64 len);


/**
\brief: close flash partition
\attention \n
\param[in] p_flash    flash handle
\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\see \n
*/
mt_s32 mt_flash_close(mt_s32 p_flash);

/**
\brief: erase Flash partiton
\attention \n
\param[in] p_flash      flash handle
\param[in] addr  erase start address, must align with blocksize  CNcomment:擦写地址，字节为单位，块对齐CNend
\param[in] len      data length, must align with blocksize CNcomment:数据长度，字节为单位，块对齐CNend
\retval ::TotalErase   erase total length
\retval ::MT_FLASH_END_DUETO_BADBLOCK     return value when nand flash have bad block or valid length less partition length CNcomment:擦除Flash遇到坏块结束(擦除完有效长度数据后,返回该值,一般不认为该操作失败)CNend
\retval ::MT_FAILURE   failure
\see \n
*/
mt_s32 mt_flash_erase(mt_handle p_flash, mt_u64 addr, mt_u64 len);

/**
\brief:  read data from flash
\attention \n
\param[in] p_flash       flash handle
\param[in] addr   read start address, for nand, must align with pagesize.
\param[in] buf         destination buffer pointer
\param[in] len       destination data length
\param[in] flags     OOB flag  CNcomment:可取值 MT_FLASH_RW_FLAG_WITH_OOB，表示数据内容是否带 OOB 区CNend
\retval ::TotalRead     read flash vaild length
\retval ::MT_FLASH_END_DUETO_BADBLOCK    CNcomment:读Flash遇到坏块结束(读完有效长度数据后,返回该值,由用户判断此种情况成功与否)CNend
\retval ::MT_FAILURE    failure
\see \n
*/
mt_s32 mt_flash_read(mt_handle p_flash, mt_u64 addr, mt_u8 *buf,
                     mt_u32 len, mt_u32 flags);

/**
\brief: write data to flash
\attention \n
1) forbidden used the function when yaffs filesystem is using
2) can use MT_FLASH_RW_FLAG_ERASE_FIRST manner write flah , can write over all partition one time or write with block
CNcomment:1) 不能调用该接口更新当前正使用的yaffs文件系统CNend
CNcomment:2) 调用该接口写Flash时，可以使用MT_FLASH_RW_FLAG_ERASE_FIRST，可以一次写完也可以一块一块的写，但是当写yaffs2时，
    必须先调用MT_Flash_Erase把要写的分区完全擦除CNend
\param[in] p_flash       flash handle
\param[in] addr   data start address, for nand, must align with pagesize
\param[in] buf         destination buffer pointer
\param[in] len       destination data length, for nand, if write with oob, must align with (pagesize + oobsize)
\param[in] flags     OOB flag CNcomment:可取值 MT_FLASH_RW_FLAG_WITH_OOB，表示数据内容是否带 OOB 区CNend
\retval ::TotalWrite    write flash vaild length
\retval ::MT_FLASH_END_DUETO_BADBLOCK   have bad block CNcomment:写Flash遇到坏块结束(写完有效长度数据后,返回该值,由用户判断此种情况成功与否)CNend
\retval ::MT_FAILURE
\see \n
*/
mt_s64 mt_flash_write(mt_handle p_flash, mt_u64 addr,
                      mt_u8 *buf, mt_u32 len, mt_u32 flags);

/**
\brief: get flash partition info
\attention \n
info content: TotalSize,PartSize,BlockSize,PageSize,OobSize,fd
\param[in] p_flash        flash handle
\param[in] pInterInfo    info struct pointer
\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\see \n
*/
mt_s32 mt_flash_get_info(mt_handle p_flash, mt_flash_interinfo_s *p_flashinfo);

/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // __MT_FLASH__H__
