/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mount.h>

#include "lxc_ipc.h"
#include "mt_unf_ipcfs_type.h"

static void help(void);

static void help(void)
{
	printf("caller_mpfs [opt] [filename] [mode]\n");
	printf("type 0 fopen/creat and fclose file eg:./caller_mpfs 0 /mnt/123.txt wb+\n");
	printf("type 1 creat dir eg:./caller_mpfs 1 /mnt/dirname 511\n");
	printf("type 2 remove dir eg:./caller_mpfs 2 /mnt/dirname\n");
	printf("type 3 dir operate eg:./caller_mpfs 3 /mnt/dirname\n");
	printf("type 4 file operate eg:./caller_mpfs 4 /mnt/123.txt rb+\n");
	printf("type 5 fseek operate get file size eg:./caller_mpfs 5 /mnt/123.txt rb+\n");
	printf("type 6 stat operate eg:./caller_mpfs 6 /mnt/123.txt rb+\n");
	printf("type 7 only fread operate eg:./caller_mpfs 7 /mnt/123.txt rb+\n");
	printf("type 8 usb event eg:./caller_mpfs 8 0\n");
	printf("type 9 open,close,write,read,lseek operate eg:./caller_mpfs 9 /mnt/filetest.txt\n");
	printf("type 10 mount operate eg:./caller_mpfs 10 /dev/sda1 /mnt vfat\n");
	printf("type 11 umount operate eg:./caller_mpfs 11 /mnt\n");
	printf("type 12 file operate eg:./caller_mpfs 12 /mnt/bigbuff123.txt wb+\n");
	printf("type 13 open,close,write,read,lseek operate eg:./caller_mpfs 13 /mnt/bigbuffest.txt\n");
	printf("type 14 pread,pwrite,operate eg:./caller_mpfs 14 /mnt/bigbuffest.txt\n");
	printf("type 16 fseeko operate get file size eg:./caller_mpfs 16 /mnt/123.txt rb+\n");
}

static void usbplug_callback(int flag,char *dev)
{
	printf("%s %d flag = %d dev:%s\n",__FUNCTION__,__LINE__,flag,dev);
}

static void usbplug_callback1(int flag,char *dev)
{
	printf("%s %d flag = %d dev:%s\n",__FUNCTION__,__LINE__,flag,dev);
}

int main(int argc, char **argv)
{
	ipc_fs_handle_t ipc_usb;
	ipc_fs_handle_t ipc_usb1;
	ipc_fs_handle_t ipc_fs;
	char usb_name[LXC_IPC_NAME_SIZE] = "ipc_usb_server";
	char fs_name[LXC_IPC_NAME_SIZE] = "ipc_fs_server";
	int ret = 0;
	long fd = 0;
	DIR *fdir = NULL;
	struct dirent dent;
	unsigned char rbuff[1024] = {0};
	unsigned char wbuff[1024] = {0};
	unsigned int i = 0;
	int otp = 0;
	unsigned int param = 0;
	struct stat data;
	unsigned char *ptmp = NULL;
	off_t size = 0;
	off_t offset = 0;

	if (argc < 3) {
		help();
		return -1;
	}

	printf("caller_mpfs running\n");
	otp = atoi(argv[1]);
	ret = ipc_fs_init(&ipc_fs,fs_name);
	if(ret < 0)
	{
		printf("%s %d ipc_fs_init error!\n",__FUNCTION__,__LINE__);
		return 0;
	}	

	switch(otp){
		case 0:		//open /creat file
			if(argc < 4)
			{
				printf("%s %d param error!\n",__FUNCTION__,__LINE__);
				help();
				return 0;
			}
			printf("%s %d path : %s\n",__FUNCTION__,__LINE__,argv[2]);
			printf("%s %d mode: %s\n",__FUNCTION__,__LINE__,argv[3]);
			fd = ipc_fs_fopen(&ipc_fs,argv[2],argv[3]);
			printf("%s %d fd = %x\n",__FUNCTION__,__LINE__,fd);
			ret = ipc_fs_fclose(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
		case 1:		//creat dir
			if(argc < 4)
			{
				printf("%s %d param error!\n",__FUNCTION__,__LINE__);
				help();
				return 0;
			}
			param = (unsigned int)atoi(argv[3]);
			printf("%s %d path : %s\n",__FUNCTION__,__LINE__,argv[2]);
			printf("%s %d mode = %d\n",__FUNCTION__,__LINE__,param);
			ret = ipc_fs_mkdir(&ipc_fs,argv[2],param);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
		case 2:		//del dir
			printf("%s %d path : %s\n",__FUNCTION__,__LINE__,argv[2]);			
			ret = ipc_fs_rmdir(&ipc_fs,argv[2]);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
		case 3:		//dir operate
			printf("%s %d path : %s\n",__FUNCTION__,__LINE__,argv[2]);
			fdir = ipc_fs_opendir(&ipc_fs,argv[2]);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,fdir);
			while (ipc_fs_readdir(&ipc_fs, fdir, &dent) == 0) {
				printf("dname ==> %s\n", dent.d_name);
			}
			ret = ipc_fs_closedir(&ipc_fs,fdir);
			break;
		case 4:		//fread and fwrite
			if(argc < 4)
			{
				printf("%s %d param error!\n",__FUNCTION__,__LINE__);
				help();
				return 0;
			}
			printf("%s %d path : %s\n",__FUNCTION__,__LINE__,argv[2]);
			printf("%s %d mode: %s\n",__FUNCTION__,__LINE__,argv[3]);
			fd = ipc_fs_fopen(&ipc_fs,argv[2],argv[3]);
			printf("%s %d fd = %x\n",__FUNCTION__,__LINE__,fd);
			for(i = 0;i<900;i++)
			{
				wbuff[i] = (unsigned char)(i%255);
			}
			for(i = 0;i<1024;i++)
			{
				rbuff[i] = 0;
			}
			ret = ipc_fs_fread(&ipc_fs,rbuff,900,1,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			printf("%s %d data : \n%s\n",__FUNCTION__,__LINE__,rbuff+1);

			ret = ipc_fs_fseek(&ipc_fs,fd,0,SEEK_SET);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);

			ret = ipc_fs_fwrite(&ipc_fs,wbuff,900,1,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);

			for(i = 0;i<1024;i++)
			{
				rbuff[i] = 0;
			}

			ret = ipc_fs_fseek(&ipc_fs,fd,0,SEEK_SET);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			
			ret = ipc_fs_fread(&ipc_fs,rbuff,900,1,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			printf("%s %d data : \n%s\n",__FUNCTION__,__LINE__,rbuff+1);
			ret = ipc_fs_flush(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			ret = ipc_fs_fsync(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			ret = ipc_fs_fclose(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
		case 5:		//fseek
			if(argc < 4)
			{
				printf("%s %d param error!\n",__FUNCTION__,__LINE__);
				help();
				return 0;
			}
			printf("%s %d path : %s\n",__FUNCTION__,__LINE__,argv[2]);
			printf("%s %d mode: %s\n",__FUNCTION__,__LINE__,argv[3]);
			fd = ipc_fs_fopen(&ipc_fs,argv[2],argv[3]);
			printf("%s %d fd = %x\n",__FUNCTION__,__LINE__,fd);
			
			ret = ipc_fs_fseek(&ipc_fs,fd,0,SEEK_END);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);

			ret = ipc_fs_ftell(&ipc_fs,fd);
			printf("%s %d file size = %d\n",__FUNCTION__,__LINE__,ret);

			ret = ipc_fs_fclose(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
		case 6:		//fstat
			if(argc < 4)
			{
				printf("%s %d param error!\n",__FUNCTION__,__LINE__);
				help();
				return 0;
			}
			printf("%s %d path : %s\n",__FUNCTION__,__LINE__,argv[2]);
			printf("%s %d mode: %s\n",__FUNCTION__,__LINE__,argv[3]);
			fd = ipc_fs_fopen(&ipc_fs,argv[2],argv[3]);
			printf("%s %d fd = %x\n",__FUNCTION__,__LINE__,fd);
			
			ret = ipc_fs_stat(&ipc_fs,argv[2],&data);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			printf("%s %d st_size = %lld\n",__FUNCTION__,__LINE__,data.st_size);

			ret = ipc_fs_fclose(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
		case 7:		//read file
			if(argc < 4)
			{
				printf("%s %d param error!\n",__FUNCTION__,__LINE__);
				help();
				return 0;
			}
			printf("%s %d path : %s\n",__FUNCTION__,__LINE__,argv[2]);
			printf("%s %d mode: %s\n",__FUNCTION__,__LINE__,argv[3]);
			fd = ipc_fs_fopen(&ipc_fs,argv[2],argv[3]);
			printf("%s %d fd = %x\n",__FUNCTION__,__LINE__,fd);
			for(i = 0;i<1024;i++)
			{
				rbuff[i] = 0;
			}
			ret = ipc_fs_fread(&ipc_fs,rbuff,900,1,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			printf("%s %d data : \n%s\n",__FUNCTION__,__LINE__,rbuff+1);

			ret = ipc_fs_fclose(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
		case 8:		//usb event
			ret = ipc_fs_init(&ipc_usb,usb_name);
			if(ret < 0)
			{
				printf("%s %d ipc_fs_init error!\n",__FUNCTION__,__LINE__);
				return 0;
			}
			while (1)
			{
				fd = getchar();
				printf("input:%c\n", fd);
				switch(fd){
					case 'i':
						ret = ipc_fs_register(&ipc_usb,usbplug_callback);
						printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
						break;
					case 'j':
						ret = ipc_fs_register(&ipc_usb,usbplug_callback1);
						printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
						break;	
					case 'a':
						ret = ipc_fs_register(&ipc_usb1,usbplug_callback1);
						printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
						break;
					case 'u':
						ret = ipc_fs_unregister(&ipc_usb);
						printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
						break;	
					case 'x':
						ret = ipc_fs_unregister(&ipc_usb1);
						printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
						break;
					case 'o':
						ret = ipc_fs_init(&ipc_usb1,usb_name);
						if(ret < 0)
						{
							printf("%s %d ipc_fs_init error!\n",__FUNCTION__,__LINE__);
						}
						break;
					case 'e':
						printf("%s %d exit !\n",__FUNCTION__,__LINE__);
						return 0;
				}
				sleep(1);
			}
			
			break;
		case 9:			//file test
			printf("%s %d filepath : %s\n",__FUNCTION__,__LINE__,argv[2]);
			fd = ipc_fs_open(&ipc_fs,argv[2],O_RDWR|O_CREAT,S_IRWXU);
			printf("%s %d fd = %x\n",__FUNCTION__,__LINE__,fd);
			for(i = 0;i<900;i++)
			{
				wbuff[i] = (unsigned char)(i%255);
			}
			for(i = 0;i<1024;i++)
			{
				rbuff[i] = 0;
			}
			ret = ipc_fs_read(&ipc_fs,fd,rbuff,900);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			printf("%s %d data : \n%s\n",__FUNCTION__,__LINE__,rbuff+1);

			offset = ipc_fs_lseek(&ipc_fs,fd,0,SEEK_SET);
			printf("%s %d offset = %lld\n",__FUNCTION__,__LINE__,offset);

			ret = ipc_fs_write(&ipc_fs,fd,wbuff,900);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);

			for(i = 0;i<1024;i++)
			{
				rbuff[i] = 0;
			}

			offset = ipc_fs_lseek(&ipc_fs,fd,0,SEEK_SET);
			printf("%s %d offset = %lld\n",__FUNCTION__,__LINE__,offset);
			
			ret = ipc_fs_read(&ipc_fs,fd,rbuff,900);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			printf("%s %d data : \n%s\n",__FUNCTION__,__LINE__,rbuff+1);
			ret = ipc_fs_sync(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);

			ret = ipc_fs_fstat(&ipc_fs,fd,&data);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			printf("%s %d st_size = %lld\n",__FUNCTION__,__LINE__,data.st_size);
			
			ret = ipc_fs_close(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
		case 10:				//mount
			if(argc < 5)
			{
				printf("%s %d param error!\n",__FUNCTION__,__LINE__);
				help();
				return 0;
			}
			printf("%s %d mount dev : %s\n",__FUNCTION__,__LINE__,argv[2]);
			printf("%s %d mount path : %s\n",__FUNCTION__,__LINE__,argv[3]);
			printf("%s %d mount filesystem : %s\n",__FUNCTION__,__LINE__,argv[4]);
			ret = ipc_fs_mount(&ipc_fs,argv[2],argv[3],argv[4],MS_MGC_VAL,"umask=0000");
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
		case 11:				//umount
			printf("%s %d umount path : %s\n",__FUNCTION__,__LINE__,argv[2]);
			ret = ipc_fs_umount(&ipc_fs,argv[2]);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
		case 12:
			if(argc < 4)
			{
				printf("%s %d param error!\n",__FUNCTION__,__LINE__);
				help();
				return 0;
			}
			ptmp = (unsigned char*)malloc(0x200000);
			if(NULL == ptmp)
			{
				printf("%s %d malloc error!\n",__FUNCTION__,__LINE__);
				return 0;
			}
			for(i = 0;i<1024;i++)
			{
				ptmp[i] = 0;
			}
			printf("%s %d path : %s\n",__FUNCTION__,__LINE__,argv[2]);
			printf("%s %d mode: %s\n",__FUNCTION__,__LINE__,argv[3]);
			fd = ipc_fs_fopen(&ipc_fs,argv[2],argv[3]);
			printf("%s %d fd = %x\n",__FUNCTION__,__LINE__,fd);

			ret = ipc_fs_fread(&ipc_fs,ptmp,0x100000,2,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			if(ret > 0)
			{
				printf("%s %d data : \n%s\n",__FUNCTION__,__LINE__,ptmp+1);
			}
			for(i = 0;i<900;i++)
			{
				ptmp[i] = (unsigned char)(i%255);
			}
			ret = ipc_fs_fseek(&ipc_fs,fd,0,SEEK_SET);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);

			ret = ipc_fs_fwrite(&ipc_fs,ptmp,0x100000,2,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);

			for(i = 0;i<1024;i++)
			{
				ptmp[i] = 0;
			}

			ret = ipc_fs_fseek(&ipc_fs,fd,0,SEEK_SET);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			
			ret = ipc_fs_fread(&ipc_fs,ptmp,0x100000,2,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			if(ret > 0)
			{
				printf("%s %d data : \n%s\n",__FUNCTION__,__LINE__,ptmp+1);
			}
			ret = ipc_fs_flush(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			ret = ipc_fs_fsync(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			ret = ipc_fs_fclose(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			free(ptmp);
			break;
		case 13:
			ptmp = (unsigned char*)malloc(0x200000);
			if(NULL == ptmp)
			{
				printf("%s %d malloc error!\n",__FUNCTION__,__LINE__);
				return 0;
			}
			printf("%s %d filepath : %s\n",__FUNCTION__,__LINE__,argv[2]);
			fd = ipc_fs_open(&ipc_fs,argv[2],O_RDWR|O_CREAT,S_IRWXU);
			printf("%s %d fd = %x\n",__FUNCTION__,__LINE__,fd);

			for(i = 0;i<1024;i++)
			{
				ptmp[i] = 0;
			}
			ret = ipc_fs_read(&ipc_fs,fd,ptmp,0x200000);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			if(ret > 0)
			{
				printf("%s %d data : \n%s\n",__FUNCTION__,__LINE__,ptmp+1);
			}

			offset = ipc_fs_lseek(&ipc_fs,fd,0,SEEK_SET);
			printf("%s %d offset = %lld\n",__FUNCTION__,__LINE__,offset);
			for(i = 0;i<900;i++)
			{
				ptmp[i] = (unsigned char)(i%255);
			}
			ret = ipc_fs_write(&ipc_fs,fd,ptmp,0x200000);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);

			for(i = 0;i<1024;i++)
			{
				ptmp[i] = 0;
			}

			offset = ipc_fs_lseek(&ipc_fs,fd,0,SEEK_SET);
			printf("%s %d offset = %lld\n",__FUNCTION__,__LINE__,offset);
			
			ret = ipc_fs_read(&ipc_fs,fd,ptmp,0x200000);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			if(ret>0)
			{
				printf("%s %d data : \n%s\n",__FUNCTION__,__LINE__,ptmp+1);
			}
			ret = ipc_fs_sync(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);

			ret = ipc_fs_fstat(&ipc_fs,fd,&data);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			printf("%s %d st_size = %lld\n",__FUNCTION__,__LINE__,data.st_size);
			
			ret = ipc_fs_close(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
        case 14:
            {   //this test for pread & pwrite.
                int fd1=0;
                unsigned char *ptmp1 = NULL;
                char filename1[48]={0};

                ptmp = (unsigned char*)malloc(0x300000);
                if(NULL == ptmp)
                {
                    printf("%s %d malloc0 error!\n",__FUNCTION__,__LINE__);
                    return 0;
                }
                ptmp1 = (unsigned char*)malloc(0x300000);
                if(NULL == ptmp1)
                {
                    printf("%s %d malloc1 error!\n",__FUNCTION__,__LINE__);
                    free(ptmp);
                    return 0;
                }
                strcpy(filename1,argv[2]);
                strcat(filename1,".bak");
                fd = ipc_fs_open(&ipc_fs,argv[2],O_RDWR|O_CREAT,S_IRWXU);
                fd1 = open(filename1,O_RDWR|O_CREAT,S_IRWXU);
                //format mem
                for(i=0;i<0x300000;i++){
                    ptmp[i]=(i&0xff);
                }
                ipc_fs_pwrite(&ipc_fs,fd,ptmp,1,0);                 //write one byte.
                ipc_fs_pwrite(&ipc_fs,fd,ptmp+1,0x300000-1,1);      //write remain.
                pwrite(fd1,ptmp,1,0);                               //write one byte.
                pwrite(fd1,ptmp+1,0x300000-1,1);                    //write remain.
                
                memset(ptmp,0x00,0x300000);
                memset(ptmp1,0xff,0x300000);

                ipc_fs_pread(&ipc_fs,fd,ptmp,1,0);                  //read one byte.
                ipc_fs_pread(&ipc_fs,fd,ptmp+1,0x300000-1,1);       //read remain
                pread(fd1,ptmp1,1,0);                               //read one byte.
                pread(fd1,ptmp1+1,0x300000-1,1);                    //read remain.
                for(i=0;i<0x300000;i++){
                    if(ptmp[i]!=ptmp1[i]){
                        printf("mem is not same!\n");
                        break;
                    }
                }
                free(ptmp);
                free(ptmp1);
                ipc_fs_close(&ipc_fs,fd);
                close(fd1);
                printf("test pread pwrite ok!\n");
            }
			break;
        case 15:
            {
                ptmp = (unsigned char*)malloc(0x100000);
                if(NULL == ptmp){
                    printf("%s %d malloc0 error!\n",__FUNCTION__,__LINE__);
                    return 0;
                }
                //fd = ipc_fs_open(&ipc_fs,argv[2],(O_RDONLY|O_LARGEFILE),0777);
                fd = ipc_fs_open(&ipc_fs,argv[2],0x2000,0x1ff);
                ipc_fs_pread(&ipc_fs,fd,ptmp,0x13a0,0);       //read remain
                ipc_fs_close(&ipc_fs,fd);
                free(ptmp);
                printf("test pread file ok!\n");
            }
            break;
        case 16:
            	{
			if(argc < 4)
			{
				printf("%s %d param error!\n",__FUNCTION__,__LINE__);
				help();
				return 0;
			}
			printf("%s %d path : %s\n",__FUNCTION__,__LINE__,argv[2]);
			printf("%s %d mode: %s\n",__FUNCTION__,__LINE__,argv[3]);
			fd = ipc_fs_fopen(&ipc_fs,argv[2],argv[3]);
			printf("%s %d fd = %x\n",__FUNCTION__,__LINE__,fd);

			ret = ipc_fs_fseeko(&ipc_fs,fd,0,SEEK_END);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);

			size = ipc_fs_ftello(&ipc_fs,fd);
			printf("%s %d file size = %lld\n",__FUNCTION__,__LINE__,size);

			ret = ipc_fs_fclose(&ipc_fs,fd);
			printf("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
			break;
            	}
      case 17:			//dio pread pwrite and compare
        {
            #define K64 (64*1024)
            #define M2M (0x200000+K64)
            int i=0;
            unsigned char *rwbuff = malloc(K64);
            if(rwbuff){//64k
                fd = ipc_fs_open(&ipc_fs,argv[2],O_CREAT | O_RDWR | O_LARGEFILE | O_DIRECT|O_SYNC,0777);
                if(fd){
                    for(i=0;i<64;i++){
                        memset(rwbuff,i+'0',K64);
                        ret = ipc_fs_pwrite(&ipc_fs,fd,rwbuff,K64,i*K64);
                    }
                    unsigned char *cmpbuff = malloc(K64);
                    if(cmpbuff){
                        for(i=0;i<64;i++){
                            memset(cmpbuff,i+'0',K64);
                            ret = ipc_fs_pread(&ipc_fs,fd,rwbuff,K64,i*K64);
                            if(0==memcmp(cmpbuff,rwbuff,K64)){
                                ;//printf("+++.cmp[%d]*64k ok\n",i);
                            }
                        }
                        free(cmpbuff);
                    }
                    ret = ipc_fs_close(&ipc_fs,fd);
                }
                free(rwbuff);
            }
            rwbuff = malloc(M2M);
            if(rwbuff){//2M
                fd = ipc_fs_open(&ipc_fs,argv[2],O_CREAT | O_RDWR | O_LARGEFILE | O_DIRECT|O_SYNC,0777);
                if(fd){
                    memset(rwbuff,'6',M2M);
                    ret = ipc_fs_pwrite(&ipc_fs,fd,rwbuff,M2M,0);
                    unsigned char *cmpbuff = malloc(M2M);
                    if(cmpbuff){
                        memset(cmpbuff,'6',M2M);
                        ret = ipc_fs_pread(&ipc_fs,fd,rwbuff,M2M,0);
                        if(0==memcmp(cmpbuff,rwbuff,M2M)){
                            ;//printf("+++.cmp*2M ok\n",i);
                        }
                        free(cmpbuff);
                    }
                    ret = ipc_fs_close(&ipc_fs,fd);
                }
                free(rwbuff);
            }
        }
            break;
      case 18:      //dio read write
      {
          unsigned char *rwbuff = malloc(64*1024);
          if(rwbuff){
              for(i=0;i<(64*1024);i++){
                  rwbuff[i]='1';
              }
              fd = ipc_fs_open(&ipc_fs,argv[2],O_CREAT | O_RDWR | O_LARGEFILE | O_DIRECT|O_SYNC,0777);
              //printf("%s %d fd = %x\n",__FUNCTION__,__LINE__,fd);
              ret = ipc_fs_write(&ipc_fs,fd,rwbuff,(64*1024));
              //printf("%s %d w.ret = %x\n",__FUNCTION__,__LINE__,ret);
              offset = ipc_fs_lseek(&ipc_fs,fd,0,SEEK_SET);
              memset(rwbuff,0x00,(64*1024));
			        //printf("%s %d offset = %lld\n",__FUNCTION__,__LINE__,offset);
			        ret = ipc_fs_read(&ipc_fs,fd,rwbuff,(64*1024));
              rwbuff[16]=0;
              //printf("%s %d r.ret = %x,%s\n",__FUNCTION__,__LINE__,ret,rwbuff);
              ret = ipc_fs_close(&ipc_fs,fd);
              free(rwbuff);
          }
      }
            break;
		default:
			break;
	}
		
	return 0;
}
