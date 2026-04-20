/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_PVR_CIPHER_CFG_H__
#define __MT_PVR_CIPHER_CFG_H__

#define PVR_CACHESIZE     (256*1024)    //must <= DIO_PAGES*PAGE_SIZE, DIO_PAGES define in kernel/linux-4.4.215/fs/direct-io.c, PAGE_SIZE is 4096
#define PVR_IOBLOCK       (64*1024)     //must >= stat.st_blksize, and align with stat.st_blksize, stat.st_blksize is cluster size
#define PVR_IOBLOCKBITS   16            //shift bit of PVR_IOBLOCK
#define MAX_CRYPTOSZ_W    0x80000
#define MAX_CRYPTOSZ_R    PVR_CACHESIZE

/* must be 2^n */
#define PVR_CIPHER_PKG_LEN              512        /* cipher group length */


#define PVR_CIPHER_AES_BIT_WIDTH        MT_UNF_CIPHER_BIT_WIDTH_128BIT
#define PVR_CIPHER_DES_BIT_WIDTH        MT_UNF_CIPHER_BIT_WIDTH_64BIT
#define PVR_CIPHER_3DES_BIT_WIDTH       MT_UNF_CIPHER_BIT_WIDTH_64BIT

#define PVR_CIPHER_AES_KEY_LENGTH       MT_UNF_CIPHER_KEY_AES_128BIT
#define PVR_CIPHER_DES_KEY_LENGTH       MT_UNF_CIPHER_KEY_DES_2KEY
#define PVR_CIPHER_3DES_KEY_LENGTH      MT_UNF_CIPHER_KEY_DES_3KEY

#define PVR_CIPHER_AES_WORK_MODD        MT_UNF_CIPHER_WORK_MODE_ECB
#define PVR_CIPHER_DES_WORK_MODD        MT_UNF_CIPHER_WORK_MODE_ECB
#define PVR_CIPHER_3DES_WORK_MODD       MT_UNF_CIPHER_WORK_MODE_ECB

#define PVR_CIPHER_AES_KEY_LENGTH_BIT   128


#endif  // __MT_PVR_CIPHER_CFG_H__

