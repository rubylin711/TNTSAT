/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "mt_type.h"
#include "mt_unf_i2c.h"



static char *device;
static unsigned long long offset;
static int offset_set;
static unsigned long len;
static int len_set;



/*
本命令主要用于mt_i2c研发自测。。
sample命令:

*/
static void usage(void)
{
	printf("sample for i2c operation.\n");
	printf("./sample_i2c R/W_operate(0/1) i2c_channel device_addr register_addr register_addr_len bytes_number  [byte0 [... byten]]\n");
	printf("./sample_i2c CFG_operate(2) i2c_channel rate(100~400)\n");
	printf("eg:\n");
	printf("Read :\n");
	printf("./sample_i2c 0 0 194 134 1 1 \n");
	printf("Write :\n");
	printf("./sample_i2c 1 0 194 134 1 1 9\n");
	printf("SetRate :\n");
	printf("./sample_i2c 2 0 400\n");
	//exit(-1);
}



#ifdef MT_SAMPLE_APP
MT_S32 MT_I2cMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    MT_S32 s32Ret = MT_FAILURE;

    MT_U8 *pData = MT_NULL;
    MT_U32 u32I2cOprType  = 0;
    MT_U32 u32DeviceAddress = 0;
    MT_U32 u32I2cNum  = 0;
    MT_U32 u32RegAddr = 0;
    MT_U32 u32RegAddrCount = 0;
    MT_U32 u32Number = 0;

    MT_U32 u32Loop;

    if (argc < 4)
    {
        usage();

        return -1;
    }
	u32I2cOprType = strtol(argv[1], NULL, 0);
	switch(u32I2cOprType){
		case 0:
			u32I2cNum = strtol(argv[2], NULL, 0);
			u32DeviceAddress = strtol(argv[3], NULL, 0);
			u32RegAddr = strtol(argv[4], NULL, 0);
			u32RegAddrCount = strtol(argv[5], NULL, 0);
			if (u32RegAddrCount > 4)
			{
				printf("register address length is error!\n");
				return -1;
			}

			u32Number = strtol(argv[6], NULL, 0);

			s32Ret = mt_unf_i2c_init();
			if (MT_SUCCESS != s32Ret)
			{
				printf("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
				return s32Ret;
			}

			pData = (MT_U8 *)malloc(u32Number);
			if (MT_NULL == pData)
			{
				printf("\n pReadData malloc() error!\n");
				mt_unf_i2c_deinit();

				return -1;
			}

			/* Read data from Device */
			s32Ret = mt_unf_i2c_read(u32I2cNum, u32DeviceAddress,
			 u32RegAddr, u32RegAddrCount, pData, u32Number);
			if (s32Ret != MT_SUCCESS)
			{
				printf("call MT_I2C_Read failed.\n");
				free(pData);
				mt_unf_i2c_deinit();
				return s32Ret;
			}

			printf("\ndata read:\n");

			for (u32Loop = 0; u32Loop < u32Number; u32Loop++)
			{
				printf("0x%02x ", pData[u32Loop]);
			}

			break;
		case 1:
			u32I2cNum = strtol(argv[2], NULL, 0);
			u32DeviceAddress = strtol(argv[3], NULL, 0);
			u32RegAddr = strtol(argv[4], NULL, 0);
			u32RegAddrCount = strtol(argv[5], NULL, 0);
			if (u32RegAddrCount > 4)
			{
				printf("register address length is error!\n");
				return -1;
			}

			u32Number = strtol(argv[6], NULL, 0);

			if (u32Number + 6 > argc)
			{
				printf("input error!\n");
				printf("Usage: i2c_write  i2c_channel  device_addr  register_addr  ");
				printf("register_addr_len  write_bytes_number  byte0 [... byten]\n");

				return -1;
			}

			s32Ret = mt_unf_i2c_init();
			if (MT_SUCCESS != s32Ret)
			{
				printf("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
				return s32Ret;
			}
			if(u32Number)
			{
				pData = (MT_U8 *)malloc(u32Number);
				if (MT_NULL == pData)
				{
					printf("\n malloc() error!\n");
					mt_unf_i2c_deinit();

					return -1;
				}
			}

			for (u32Loop = 0; u32Loop < u32Number; u32Loop++)
			{
				pData[u32Loop] = strtol(argv[u32Loop + 7], NULL, 0);
			}


			/* Read data from Device */
			s32Ret = mt_unf_i2c_write(u32I2cNum, u32DeviceAddress,
			         u32RegAddr, u32RegAddrCount, pData, u32Number);
			if (s32Ret != MT_SUCCESS)
			{
				printf("i2c write failed!\n");
			}
			else
			{
				printf("i2c write success!\n");
			}

			break;
		case 2:
			u32I2cNum = strtol(argv[2], NULL, 0);
			u32Number = strtol(argv[3], NULL,10);
			printf("i2c channel = %d rate = %d\n",u32I2cNum,u32Number);
			s32Ret = mt_unf_i2c_init();
			if (MT_SUCCESS != s32Ret)
			{
				printf("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
				return s32Ret;
			}
			s32Ret = mt_unf_i2c_set_rate_ex(u32I2cNum,u32Number*1000);
			break;
		default:
			usage();
			return -1;

	}

    free(pData);

    mt_unf_i2c_deinit();

    return s32Ret;
}

