/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/ioctl.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <linux/netlink.h>


//#include "sys/time.h"
#include "ui_manager.h"

#define MT_MAX_SUFFIXES_NUM 100
#define MT_MAX_PATH_LEN          256
#define MT_MAX_PARTTION_NUM 10
#define USB_DEVICE_ID            0x42
#define UEVENT_BUFFER_SIZE 512
#define MT_MAX_UDISK_NUM      2

#define NTFS_3G "ntfs-3g -o uid=0,gid=0,dmask=000,fmask=000,umask=000,recover %s %s"
#define VFAT_32 "mount -o usefree -t vfat %s %s"
#define UMOUNT "umount %s"


typedef enum {
    MT_DISK_EVENT_MOUNTED,  /**< �������� */
    MT_DISK_EVENT_UMOUNTED, /**< ����ж�� */
    MT_DISK_EVENT_UPDATE,   /**< ������Ϣ���£�Ϊ�˾����֪�������أ���������Ϣ��ȡ�͹��طֿ�? */
    MT_DISK_EVENT_FULL,     /**< ������ */
    MT_DISK_EVENT_BUTT
} MT_DISK_EVENT_E;

typedef enum
{
	USB_DEVICE_IDE,
	USB_DEVICE_INSERTING,
	USB_DEVICE_READY,
}USB_DEVICE_STATUS;

typedef enum
{
    FS_NTFS,
    FS_FAT32,
    FS_EXT2,
    FS_ERROR_TYPE,
}FILE_SYS_TYPE;


typedef struct MT_PATITION_INFO
{
    mt_s32		deviceid;
    mt_char		PartitionId;
    mt_char		Filesystem;
    mt_char		name[MT_MAX_PATH_LEN];
    mt_char		path[MT_MAX_PATH_LEN];
    mt_u64		totalSize;
    mt_u64		usedSize;
    mt_u64		freeSize;
    MT_BOOL		bFullReport;
    mt_u64		u64FullWaterLine;
} MT_PATITION_INFO_S;

typedef struct tagMT_UDISK_S
{
	mt_u32				deviceid;
	mt_char				Status;
	mt_u32				insert_time;
	mt_char				aszDiskName[MT_MAX_PATH_LEN];
	MT_BOOL				bAttach;
	mt_char				chDev; 
	mt_s32				PartNum;
	MT_PATITION_INFO_S	ParttionList[MT_MAX_PARTTION_NUM];
}MT_UDISK_INFO_S;

static pthread_mutex_t disk_mutex = PTHREAD_MUTEX_INITIALIZER;
static MT_UDISK_INFO_S	gUdiskInfo[MT_MAX_UDISK_NUM];
static mt_s32 socket_id = -1;
static int   g_deviceid = 1;

static mt_u32 g_usb0_status = 0;
static mt_u32 g_usb1_status = 0;

static MT_BOOL g_bScanTaskRunning = MT_FALSE;

static pthread_t g_scanThread;


static int usbdev_socket_init()
{
	struct sockaddr_nl client;
	// s32 buffersize = 1024*10;

	if(socket_id  >= 0)
		return MT_SUCCESS;

	socket_id = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT); 
	if (socket_id < 0) 
	{
		printf("Socket faild\t");
		return MT_FAILURE;
	}

	memset(&client, 0, sizeof(client));
	client.nl_family = AF_NETLINK;  
	client.nl_pid = syscall(SYS_gettid); 
	client.nl_groups = 1; /* receive broadcast message*/  

	if(bind(socket_id, (struct sockaddr*)&client, sizeof(client)) != 0)
	{
		printf("Bind\t");
		close(socket_id);
		socket_id = -1;
		return 0;
	}
	return MT_SUCCESS;
}

static mt_bool usbdev_device_check(s8 chDev, u32 u32Part)
{
	int i;

	for(i=0;i<MT_MAX_UDISK_NUM;i++)
	{
		if((gUdiskInfo[i].chDev==chDev) && (gUdiskInfo[i].bAttach==TRUE))
		{
			return TRUE;
		}	
	}
	return FALSE;
}

static int usbdev_device_add(mt_s8 chDev, mt_u32 u32Part)
{
	int i;

	pthread_mutex_lock(&disk_mutex);
	for(i=0;i<MT_MAX_UDISK_NUM;i++)
	{
		if(gUdiskInfo[i].bAttach==FALSE)
		{
			
			gUdiskInfo[i].deviceid = USB_DEVICE_ID + g_deviceid;
			gUdiskInfo[i].chDev = chDev;
			gUdiskInfo[i].PartNum = 0;
			gUdiskInfo[i].bAttach = TRUE;
			gUdiskInfo[i].Status = USB_DEVICE_INSERTING;
			gUdiskInfo[i].insert_time = time(0);
			sprintf(gUdiskInfo[i].aszDiskName,"/dev/sd%c",chDev);
			g_deviceid++;
			pthread_mutex_unlock(&disk_mutex);
			return MT_SUCCESS;
		}	
	}
	pthread_mutex_unlock(&disk_mutex);
	return MT_FAILURE;
}


static FILE_SYS_TYPE __get_filesystem_type(char * dev_path)
{
  int fd = 0;
  int iRet = 0;
  unsigned char * tmpBuffer = NULL;
  unsigned char * pOffset = NULL;
  FILE_SYS_TYPE  eRet = FS_ERROR_TYPE;
  int retry = 0;
RETRY:
  fd = open(dev_path,O_RDONLY);
  if(fd < 0)
  {
  printf("__get_filesystem_type open %s failed,%s fd=%d\n",dev_path,strerror(errno),fd);
    if((ENOENT == errno) && (retry ++ < 3))
    {
        mtos_task_sleep(200);
        printf("retry:%d\n",retry);
        goto RETRY;
    }
  return eRet;
  }

  tmpBuffer = (unsigned char*)malloc(0x400);
  if(tmpBuffer == NULL)
  {
  printf("__get_filesystem_type malloc error\n");
  close(fd);
  return eRet;
  }

  memset(tmpBuffer,0x0,0x400);
  do
  {
    iRet = read(fd,tmpBuffer,0x100);
    if(iRet <= 0)
      break;

    u8 tmp_asc[16] = {0};

    if(!memcmp((const void*)(tmpBuffer+0x52), (const void*)"FAT32", 5))//th offset with 0x52 is fat32 tag;
    {
      printf("filesystem:fat32!\n");
      eRet = FS_FAT32;
      break;
    }
    else if(!memcmp((const void*)(tmpBuffer+0x3), (const void*)"NTFS", 4))//the offset with 0x3 is ntfs tag;
    {
      printf("filesystem:ntfs!\n");
      eRet =  FS_NTFS;
      break;
    }
    else if(!memcmp((const void*)(tmpBuffer+0x36), (const void*)"FAT16", 5))
    {
      printf("filesystem:fat16!\n");
      eRet =  FS_FAT32;//mount as FAT32.
      break;
    }
    else
    {
      memset(tmpBuffer, 0x00, 0x400);
      lseek(fd,0x400, SEEK_SET); //seek to superblock1;
      iRet = read(fd, tmpBuffer, 0x400);//read the superblock1 to buffer;
      if(!iRet)
      {
        printf("read device file failed!\n");
        break;
      }

      pOffset = tmpBuffer+0x38; //the offset 0x438 is the tag of ext2:0x53 0xef;
      if((pOffset[0] == 0x53) && (pOffset[1] == 0xef))
      {
        printf("filesystem:ext2 or ext3!\n");
        eRet =  FS_EXT2;
        break;
      }
      else
      {
        printf("the offset 0x438 vaule is:0x%x 0x%x!\n", pOffset[0], pOffset[1]);
      }
    }
  }while(0);
  free(tmpBuffer);
  close(fd);
  return eRet;
}

static int usbdev_unmount_dev(char * dev)
{
	int ret = 0;
	char cmd[512];

	memset(cmd,0x0,512);
	printf("usbdev_unmount_dev %s\n",dev);
	sprintf(cmd,UMOUNT,dev);
	ret = system(cmd);
	return ret;
}

static int usbdev_mount_vfat_32(char * dev,char * path)
{
	int ret = 0;
	char cmd[512];

	memset(cmd,0x0,512);
	printf("usbdev_mount_vfat_32 %s\n",dev);
	
	sprintf(cmd,VFAT_32,dev,path);
	ret = system(cmd);
	return ret;
}

static int usbdev_mount_ntfs_3g(char * dev,char * path)
{
	int ret = 0;
	char cmd[512];

	memset(cmd,0x0,512);
	printf("usbdev_mount_ntfs_3g %s\n",dev);
	
	sprintf(cmd,NTFS_3G,dev,path);
	ret = system(cmd);
	return ret;
}

static int usbdev_mount(MT_PATITION_INFO_S * pstPattion )
{
	int iRet = 0;
	FILE_SYS_TYPE eType = FS_ERROR_TYPE;

	if(pstPattion == NULL)
		return MT_FAILURE;

	eType = __get_filesystem_type(pstPattion->name);
	if(eType == FS_ERROR_TYPE)
	{
		printf("usbdev_mount get file system type failed\n");
		return MT_FAILURE;
	}

	if(access(pstPattion->path,F_OK) != 0)
	{
		iRet = mkdir(pstPattion->path, 0777);
		if ((iRet < 0)&&(EEXIST != errno) )
		{
			printf("usbdev_mount : make dir (%s) failed\n",pstPattion->path);
			return MT_FAILURE;
		}
	}
	
	usbdev_unmount_dev(pstPattion->name);

	switch(eType)
	{
		case FS_FAT32:
			//iRet =  mount((const char*)pstPattion->name, (const char*)pstPattion->path, (const char*)"vfat", 0,"umask=000,iocharset=utf8");
			iRet =  usbdev_mount_vfat_32(pstPattion->name, pstPattion->path);
			break;
		case FS_NTFS:
			// iRet =  mount((const char*)pstPattion->name, (const char*)pstPattion->path, (const char*)"ntfs", 0,NULL);
			iRet = usbdev_mount_ntfs_3g(pstPattion->name,pstPattion->path);
			break;
		case FS_EXT2:
			iRet =  mount((const char*)pstPattion->name, (const char*)pstPattion->path, (const char*)"ext2", 0,NULL);
			break;
		default:
			return MT_FAILURE;
	}

	pstPattion->Filesystem= eType;

	if(iRet != 0)
		printf("usbdev_mount:%s %s type=%d ret=%d %s\n",pstPattion->name,pstPattion->path,eType,iRet,strerror(errno));
	if(iRet == 0)
		return MT_SUCCESS;
	else
		return MT_FAILURE;
}


static int usbdev_parttion_add(s8 chDev, u32 u32Part)
{
	int ret = 0;
	int i = 0,index = 0;

	if(u32Part == 0)
		return MT_SUCCESS;

	for(i=0;i<MT_MAX_UDISK_NUM;i++)
	{
		if(gUdiskInfo[i].chDev == chDev && gUdiskInfo[i].bAttach==TRUE)
		{
			index = gUdiskInfo[i].PartNum;
			break;
		}	
	}

	if(i >= MT_MAX_UDISK_NUM)
		return MT_FAILURE;

	if(index >= MT_MAX_PARTTION_NUM)
		return MT_FAILURE;

	sprintf(gUdiskInfo[i].ParttionList[index].name,"/dev/sd%c%d",chDev,u32Part);
	sprintf(gUdiskInfo[i].ParttionList[index].path,"/tmp/medias/sd%c%d",chDev,u32Part);
	ret = usbdev_mount(&gUdiskInfo[i].ParttionList[index]);
	if(ret != MT_SUCCESS)
	{
		memset(&gUdiskInfo[i].ParttionList[index],0x0,sizeof(MT_PATITION_INFO_S));
		return MT_FAILURE;
	}

	pthread_mutex_lock(&disk_mutex);
	gUdiskInfo[i].ParttionList[index].deviceid = gUdiskInfo[i].deviceid;
	gUdiskInfo[i].ParttionList[index].PartitionId = u32Part;
	gUdiskInfo[i].PartNum += 1;
	gUdiskInfo[i].insert_time = time(0);
	pthread_mutex_unlock(&disk_mutex);
	return MT_SUCCESS;
}

static mt_bool usbdev_parttion_check(s8 chDev, u32 u32Part)
{
	int i = 0,j = 0;

	if(u32Part == 0)
		return TRUE;
	
	for(i=0;i<MT_MAX_UDISK_NUM;i++)
	{
		if(gUdiskInfo[i].chDev == chDev && gUdiskInfo[i].bAttach==TRUE)
		{
			for(j = 0 ; j < gUdiskInfo[i].PartNum;j++)
			{
				if(gUdiskInfo[i].ParttionList[j].PartitionId == u32Part)
					return TRUE;
			}
		}	
	}
	return FALSE;
}

static mt_u32 usbdev_attach(mt_s8 chDev, mt_u32 u32Part)
{
	mt_s32 ret = 0;

	if(u32Part == 0)
		return MT_FAILURE;
	
	if( FALSE  == usbdev_device_check(chDev, u32Part))
	{
		ret = usbdev_device_add(chDev,u32Part);
		if(ret != MT_SUCCESS)
		{
			printf("usbdev_attach: sd%c Parttion=%d add dev failed\n",chDev,u32Part);
			return MT_FAILURE;
		}
	}

	if(u32Part != 0 && FALSE == usbdev_parttion_check(chDev,u32Part))
	{
		ret = usbdev_parttion_add(chDev,u32Part);
		if(ret != MT_SUCCESS)
		{
			printf("usbdev_attach: sd%c Parttion=%d add Parttion  failed\n",chDev,u32Part);
			return MT_FAILURE;
		}
	}
	else
	{
		printf("usbdev_attach: sd%c Parttion=%d is mounted\n",chDev,u32Part);
		return MT_FAILURE;
	}
	
	return MT_SUCCESS;
}

static void usbdev_scandev(void)
{
	mt_s8 chDev = 0,ParttionID = 0;
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	mt_u32 device_id = 0;

	if ((dir=opendir("/dev")) == NULL)
		return;

	while ((ptr=readdir(dir)) != NULL)
	{
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
			continue;
		if(memcmp(ptr->d_name,"sd",2) == 0)
		{
			printf("usb scan: %s\n", ptr->d_name);
			if(strlen(ptr->d_name) == 3)
			{
				chDev = ptr->d_name[2];
				ParttionID = 0;
				device_id = chDev - 'a' + 1;
				usbdev_attach(chDev,ParttionID);
			}
			else
			{
				chDev = ptr->d_name[2];
				ParttionID = ptr->d_name[3] - '0';
				device_id = chDev - 'a' + 1;
				usbdev_attach(chDev,ParttionID);
			}
		}
	}
	if(dir != NULL)
		closedir(dir);
	return;
}

static int usbdev_umount(char * mount_path)
{
	if(mount_path == NULL)
		return MT_FAILURE;

	if (umount2(mount_path, 2) < 0)
		umount2(mount_path, 2);
	rmdir(mount_path);

	return MT_SUCCESS;
}

static void mt_disk_event(MT_DISK_EVENT_E enEvent, mt_u8 port)
{
	mt_u32 status = 0;
//	printf("%s(line; %d), evt %d, port %d\n", __FUNCTION__, __LINE__, enEvent, port);
	switch (enEvent)
	{
		case MT_DISK_EVENT_MOUNTED:
			status = 1;
			break;
		case MT_DISK_EVENT_UMOUNTED:
			status = 0;
			break;
		case MT_DISK_EVENT_UPDATE:
			status = 1;
			break;
		case MT_DISK_EVENT_FULL:
		case MT_DISK_EVENT_BUTT:
		default:
			return;
	}

	manage_update_sub_event(ROOT_ID_BASIC, port + 4, &status);
	manage_update_ui(UPDATE_MODULE, ROOT_ID_BASIC);
}


static int usbdev_device_remove(s8 chDev, u32 u32Part)
{
	int i =0,j=0;
	int Partnum = 0;

	for(i=0;i<MT_MAX_UDISK_NUM;i++)
	{
		if(gUdiskInfo[i].chDev == chDev && gUdiskInfo[i].bAttach==TRUE)
		{
			if(gUdiskInfo[i].PartNum  != 0 )
			{
				pthread_mutex_lock(&disk_mutex);
				gUdiskInfo[i].bAttach = FALSE;
				Partnum = gUdiskInfo[i].PartNum;
				gUdiskInfo[i].PartNum = 0;
				pthread_mutex_unlock(&disk_mutex);
				
				for(j= 0;j < Partnum;j++)
				{
					usbdev_umount(gUdiskInfo[i].ParttionList[j].path);
				}
			}

			memset(&gUdiskInfo[i],0x0,sizeof(MT_UDISK_INFO_S));
			return MT_SUCCESS;
		}	
	}
	return MT_FAILURE;
}

static mt_u32 usbdev_detach(s8 chDev, u32 u32Part)
{
	int ret = 0;

	if(FALSE == usbdev_device_check(chDev, u32Part))
	{
		return MT_FAILURE;
	}

	ret = usbdev_device_remove(chDev,u32Part);
	if(ret != MT_SUCCESS)
	{
		printf("%s(line: %d), remove usb dev failed.\n", __FUNCTION__, __LINE__);
		return MT_FAILURE;
	}
	return MT_SUCCESS;
}


void usbdev_monitor(void)
{
	mt_u32 ret = 0;
	mt_u32 offset = 0;
	fd_set fds;
	struct timeval tv;  
	mt_u32 rcvlen = 0;
	mt_char readbuf[UEVENT_BUFFER_SIZE];
	mt_u8 * tmp  = NULL;
	mt_char chDev = 'a';
	mt_u32 u32Part = '1';
	mt_u8 port = 0;

	if(socket_id < 0)
	return;

	FD_ZERO(&fds);  
	FD_SET(socket_id, &fds);  
	tv.tv_sec = 0;  
	tv.tv_usec = 500 * 1000;  
	ret = select(socket_id + 1, &fds, NULL, NULL, &tv);  

	if(ret > 0 && FD_ISSET(socket_id, &fds))
	{
		/* receive data */  
		memset(readbuf,0,sizeof(readbuf));
		rcvlen = recv(socket_id, readbuf, sizeof(readbuf) - 1, 0);  
		if (rcvlen > 0) 
		{
			if ((strstr(readbuf, "add@") != NULL) && (strstr(readbuf, "/block") != NULL))
			{
				printf("recv: %s\n", readbuf);
				tmp = strstr(readbuf, "/block");
				if (NULL != tmp)
				{
					if(strlen(tmp) > 10)
					{
						offset = strlen(tmp) - 2;
						chDev = tmp[offset];
						u32Part = tmp[offset + 1] - '0' ;
					}
					else
					{
						chDev = tmp[9];
						u32Part = 0;
					}
					ret = usbdev_attach(chDev, u32Part);
					if(ret == MT_SUCCESS)
					{
						tmp = strstr(readbuf, "/usb");
						port = tmp[4] - '0' - 1;
						mt_disk_event(MT_DISK_EVENT_MOUNTED, port);
					}
				}
			}
			else if ((strstr(readbuf, "remove@") != NULL) && (strstr(readbuf, "/block") != NULL))
			{
				printf("recv: %s\n", readbuf);
				tmp = strstr(readbuf, "/block");
				if (NULL != tmp)
				{
					if(strlen(tmp) > 10)
					{
						offset = strlen(tmp) - 2;
						chDev = tmp[offset];
						u32Part = tmp[offset + 1] - '0' ;
					}
					else
					{
						chDev = tmp[9];
						u32Part = 0;
					}
					ret = usbdev_detach(chDev, u32Part);
					if(ret == MT_SUCCESS)
					{
						tmp = strstr(readbuf, "/usb");
						port = tmp[4] - '0' - 1;
						mt_disk_event(MT_DISK_EVENT_UMOUNTED, port);
					}
				}
			}
		}
	}
}

static void usbdev_process()
{
	mt_s32 i;
	mt_u32 current_tick = time(0);
	
	for(i=0;i<MT_MAX_UDISK_NUM;i++)
	{
		if(gUdiskInfo[i].bAttach==TRUE && gUdiskInfo[i].PartNum > 0 && gUdiskInfo[i].Status == USB_DEVICE_INSERTING && (time(0) - gUdiskInfo[i].insert_time) >= 1)
		{
			gUdiskInfo[i].Status = USB_DEVICE_READY;
		}	
	}
}

static void* usbdev_daemon_thread(void* arg)
{
	prctl(PR_SET_NAME, "Mt_disk");
	usbdev_scandev();
	while(g_bScanTaskRunning == MT_TRUE)
	{
		usbdev_monitor();
		usbdev_process();
	}
	printf("usbdev_daemon_thread return.\n");
}

mt_s32 demo_usb_init(void)
{
	
	
	memset(gUdiskInfo,0,sizeof(gUdiskInfo));
	mkdir("/tmp/medias", 0777);

	usbdev_socket_init();
	g_bScanTaskRunning = MT_TRUE;
	pthread_create(&g_scanThread, NULL, usbdev_daemon_thread, NULL);
//	pthread_detach(g_usbdev_daemon_thrd);

}

mt_s32 demo_usb_stop(void)
{
	mt_s32 ret;
	g_bScanTaskRunning = MT_FALSE;
	
	ret = pthread_join(g_scanThread, NULL);
    if(MT_SUCCESS != ret)
    {
        printf("pthread_join failed.\n");
    }
	printf("%s(line: %d), return\n", __FUNCTION__, __LINE__);
}


void demo_read_usb_file(void)
{
	FILE *pfilep = NULL;
	mt_char temp_char[256] = {0};
	mt_char databuf[256] = {0};
	mt_u16 readlen = 0;

	sprintf(temp_char, "/tmp/medias/sda1/usbtest.txt");
	pfilep = fopen(temp_char, "rb");
	if(!pfilep)
	{
		printf("file1 open err \n");
		pfilep = fopen("/dev/sda1/111", "wb+");
		if(!pfilep)
		{
			printf("file2 open err \n");
			return 0;
		}
		else
			fclose(pfilep);
	}
	readlen = fread(databuf, 1, 10, pfilep);
	if(readlen == 0)
	{
		fclose(pfilep);
		printf("fread err \n");
		return 0;
	}

	fclose(pfilep);
}



