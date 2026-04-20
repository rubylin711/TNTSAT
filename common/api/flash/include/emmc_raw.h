/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
#ifndef __EMMC_RAW_H__
#define __EMMC_RAW_H__

#include "mt_type.h"
#include <linux/types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#define EMMC_EXT_PART_ID       5
#define EMMC_SECTOR_TAIL      (0xAA55)
#define EMMC_SECTOR_SIZE      (512)
#define MMC_BLOCK_MAJOR			179
#define MMC_ERASE_CMD _IOW(MMC_BLOCK_MAJOR, 1, struct mmc_erase_cmd)


typedef enum _emmc_part_type_t
{
    EMMC_PART_TYPE_RAW,
    EMMC_PART_TYPE_LOGIC,
    EMMC_PART_TYPE_BUTT
}emmc_part_type_t;

typedef struct _emmc_flash_s
{
    /* None ext area start address.
     * Absolutely offset from emmc flash start.
     */
    mt_u64 raw_areastart;

    /* None ext area size. In Byte */
    mt_u64 raw_areasize;

    /* Block size. Default is 512B */
    mt_u32 erasesize;
}emmc_flash_s;

typedef struct _emmc_cb_s
{
    mt_s32 fd;
    mt_u64 addr;
    mt_u64 partsize;
    mt_u32 erasesize;
    emmc_part_type_t part_type;
} emmc_cb_s;

struct mmc_erase_cmd {
	unsigned int from; /* first sector to erase */
	unsigned int nr;   /* number of sectors to erase */

	/* erase command argument (SD supports only %MMC_ERASE_ARG) */
	unsigned int arg;
};


///////////////////////////////////////////////////
mt_s32 emmc_raw_init(char *bootargs);
emmc_cb_s *emmc_raw_open(mt_u64 addr,
                         mt_u64 len);
emmc_cb_s *emmc_node_open(const mt_u8 *p_node);

mt_s32 emmc_block_read(mt_s32 fd,
                       mt_u64 start,
                       mt_u32 len,
                       void *buff);

mt_s32 emmc_block_write(mt_s32 fd,
                        mt_u64 start,
                        mt_u32 len,
                        const void *buff);

mt_s32 emmc_raw_read(const emmc_cb_s *p_emmc,
                     mt_u64    offset,
                     mt_u32    len,
                     mt_u8     *buf);

mt_s32 emmc_raw_write(const emmc_cb_s *p_emmc,
                      mt_u64    offset,
                      mt_u32    len,
                      const mt_u8     *buf);

mt_s32 emmc_raw_close(emmc_cb_s *p_emmc);
#if defined (MT_EMMC_ERASE_SUPPORT)
mt_s32 emc_raw_erase(emmc_cb_s *p_emmc,
                      mt_u64    offset,
                      mt_u64    len);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif

