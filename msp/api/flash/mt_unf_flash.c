/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <linux/ioctl.h>
#include <mtd/mtd-abi.h>
#include <linux/sched.h>
#include <errno.h>
#include <malloc.h>
#include <linux/fs.h>

#include "mt_type.h"

#include "mt_unf_ecs.h"
#include "mt_unf_flash.h"
#include "mt_module_debug.h"

#include "snf_protab_1.h"
#include "snf_protab_2.h"

#include <semaphore.h>

#ifndef MT_ERR_FLASH
#define MT_ERR_FLASH  printf
#endif
#ifndef MT_INFO_FLASH
#define MT_INFO_FLASH printf
#endif

#define ERROR_CODE_NO_ERROR_ENUM
#define MAX_MTD_PATITIONS   128
typedef struct MTD_Part_Info {
    u32 magic1;
    struct MTD_Part_Info *next;
    u32 magic2;
    unsigned char   type;
    unsigned int    size;
    unsigned int    eraseSize;
    unsigned int    salvage_end_addr;
    unsigned int    blockNum;
    unsigned int    availableSize;  //可使用的大小(不可使用的是指被标识了坏块的部分)
    unsigned int    pages_per_block;
    unsigned int    page_write_size;
    unsigned int    page_oob_size;
    int    handle;
    int    flag_lastblkid;
    int    partid;
    char            path[32];
    flash_msg_callback pcallback;
    unsigned int handle_using_cnt;
} MTD_Part_Info;

typedef struct MTD_Part_Info_Header {
    u32 magic1;
    MTD_Part_Info *next;
    u32 magic2;
    pthread_mutex_t mt_info_lock;	//meta add\del
    pthread_mutex_t WRMutex;			//differ from otp'(read write)
    MT_BOOL  intialized;
    ulong part_offset[MAX_MTD_PATITIONS];   //partition'start
    ulong chipsize;
    spi_prot_block_type_t type;             //protect type
} MTD_Part_Info_Header;

struct spi_flash_id {
	uint8_t  reserved1;
    uint8_t  mid;
    uint16_t did;
    uint16_t ext_did;
    uint16_t reserved2;
};
struct spi_flash_id_uni {
    uint32_t reserved;
    uint32_t len;
    uint64_t code_uni;      //uint32_t *code_uni
};

struct status_reg_user {
    uint16_t status;
    uint16_t status_len;
};

enum flash_msg_type {
    MSG_WRITE = 0,
    MSG_READ = 1,
    MSG_ERASE = 2,
};
/* Sector Erase segment of MTD */
//#define MEMSEERASE		_IOW('M', 26, struct erase_info_user)
/* Half Block Erase segment of MTD */
//#define MEMHBERASE		_IOW('M', 27, struct erase_info_user)
#define OTPERASE		_IOW('M', 25, struct otp_info)

#define MEMGETFLASHID			_IOR('M', 28, struct spi_flash_id)
#define MEMGETSTATUSREG		_IOWR('M', 29, struct status_reg_user)
#define MEMSETSTATUSREG		_IOWR('M', 30, struct status_reg_user)
#define MEMOTPFLASHUNIID	_IOWR('M', 31, struct spi_flash_id_uni)

static MTD_Part_Info_Header mtd_part_info_head={0};

static MT_S32 flash_all_id=0;

static MTD_Part_Info * ptr_find_and_plus(MT_HANDLE DevHandle)
{
    MTD_Part_Info * mtd_part_info_ptr = NULL;
    int deep=0;

    pthread_mutex_lock(&(mtd_part_info_head.mt_info_lock));
    if(0==mtd_part_info_head.intialized) {
        pthread_mutex_unlock(&(mtd_part_info_head.mt_info_lock));
        return NULL;
    }
    if((0xaa55aa55 != mtd_part_info_head.magic1) || (0xaa55aa56 != mtd_part_info_head.magic2)) {
        MT_ERR_FLASH("++err.%s,%d\n",__FUNCTION__,__LINE__);
    }
    mtd_part_info_ptr = mtd_part_info_head.next;
    while(mtd_part_info_ptr != NULL) {
        deep++;
        if ((MT_HANDLE)mtd_part_info_ptr == DevHandle) {
            mtd_part_info_ptr->handle_using_cnt++;
            pthread_mutex_unlock(&(mtd_part_info_head.mt_info_lock));
            //printf("deep.0=%x\n",deep);
            return mtd_part_info_ptr;
        }
        if((0xaa55aa55 != mtd_part_info_ptr->magic1) || (0xaa55aa56 != mtd_part_info_ptr->magic2)) {
            MT_ERR_FLASH("++err.%s,%d\n",__FUNCTION__,__LINE__);
        }
        mtd_part_info_ptr = mtd_part_info_ptr->next;
    }
    pthread_mutex_unlock(&(mtd_part_info_head.mt_info_lock));
    //printf("deep.1=%x\n",deep);
    return NULL;
}

static void ptr_unused_minus(MTD_Part_Info *x)
{
    pthread_mutex_lock(&(mtd_part_info_head.mt_info_lock));
    if(x->handle_using_cnt) {
        x->handle_using_cnt--;
    }
    pthread_mutex_unlock(&(mtd_part_info_head.mt_info_lock));
}
static char * simple_seektonextchar(char *s)
{
    if(*s != 0) {
        while((0 != *s)&&(' ' != *s)) {
            s++;
        }
    }
    if(*s != 0) {
        while(' ' == *s) {
            s++;
        }
    }
    return s;
}

static MT_S32 mt_flash_get_lastblockidbyname(MT_CHAR *devname)
{
    MT_CHAR *seekp=(MT_CHAR *)devname;
    MT_CHAR tmp,times=0;
    MT_S32 id=0,flag_getid=0;
    if(NULL==devname) {
        return -1;
    }
    seekp=devname+strlen(devname)-1;
    while(seekp!=devname) {
        tmp=*seekp;
        //printf("c=%c\n",tmp);
        if(('0'<=tmp)&&('9'>=tmp)) {
            if(0==times) {
                id=(tmp-'0');
            } else {
                id+=10*(tmp-'0');
            }
            times++;
            flag_getid=1;
        } else {
            break;
        }
        seekp--;
    }

    if(0==flag_getid) {
        return -1;
    }
    return id;
}
static MT_S32 mt_flash_get_lastblockid(void)
{
#define PARTITION_PATH "/proc/partitions"
#define PARTITION_HEAD "mtdblock"
#define BUFFER_SIZE 4096*4
    MT_CHAR *part_head=NULL,*part_info=NULL,*seekp=NULL,tmp=0;
    FILE *fp=NULL;
    MT_U32 pos=0;
    MT_S32 id=0,flag_getid=0;

    part_head=(MT_CHAR *)malloc(BUFFER_SIZE);
    if(NULL==part_head) {
        return -1;
    }

    fp=fopen(PARTITION_PATH,"rb");
    if(NULL==fp) {
        free(part_head);
        return -1;
    }
    while(!feof(fp)) {
        part_head[pos] = (MT_CHAR)fgetc(fp);
        pos++;
    }
    fclose(fp);
    part_head[BUFFER_SIZE-1]=0;

    seekp=part_head;
    do {
        part_info=seekp;
        seekp=strstr((part_info+1),PARTITION_HEAD);
        /*if(seekp){
        	printf("c=%c,%x\n",seekp[0],(seekp-part_head));
        }*/
    } while(seekp);

    if(part_info) {
        seekp=part_info+strlen(PARTITION_HEAD);
        while(seekp) {
            tmp=*seekp;
            if(('0'<=tmp)&&('9'>=tmp)) {
                id=id*10+(tmp-'0');
                flag_getid=1;
            } else {
                break;
            }
            seekp++;
        }
    }
    free(part_head);
    if(0==flag_getid) {
        return -1;
    }
    return id;
}

static MT_S32 mt_flash_get_mtdpartoffset(ulong *part_offset,ulong *chipsize)
{
#define PARTITION_PATH_MTD "/proc/mtd"
#define ALL_PARTITION "all_flash"
#define PARTITION_HEAD_MTD "mtd"
#define BUFFER_SIZE 4096*4
    MT_CHAR *part_head=NULL,*part_info=NULL,*seekp=NULL,tmp=0;
    FILE *fp=NULL;
    MT_U32 pos=0,id=0;
    ulong tolsize=0,nowsize=0,lstsize=0;

    part_head=(MT_CHAR *)malloc(BUFFER_SIZE);
    if(NULL==part_head) {
        return -1;
    }

    fp=fopen(PARTITION_PATH_MTD,"rb");
    if(NULL==fp) {
        free(part_head);
        return -1;
    }
    while(!feof(fp) && (pos<BUFFER_SIZE)) {
        part_head[pos] = (MT_CHAR)fgetc(fp);
        pos++;
    }
    fclose(fp);

	if(BUFFER_SIZE <= pos){
		pos = (BUFFER_SIZE-1);
	}
    part_head[pos]=0;

    seekp=part_head;
    do {
        seekp=strstr((seekp+1),PARTITION_HEAD_MTD);
        if(seekp) {
            part_info=simple_seektonextchar(seekp);
            nowsize=0;
            while(' ' != *part_info) {
                tmp=*part_info;
                if('0'<=tmp && '9'>=tmp) {
                    nowsize=(nowsize<<4)|(tmp-'0');
                } else {
                    nowsize=(nowsize<<4)|(tmp-'a'+10);
                }
                part_info++;
            }
            if(0==id) {
                part_offset[id]=0;
            } else {
                part_offset[id]=part_offset[id-1]+lstsize;  //nowoff=lstoff+lstsize
            }
            tolsize+=nowsize;
            lstsize=nowsize;
            //printf("++part[%d]=%lx,%lx\n",id,part_offset[id],nowsize);
            id++;
        }
    } while(seekp);

    seekp=strstr(part_head,ALL_PARTITION);
    if(NULL==seekp) {           //if can't find "all_flash"
        flash_all_id=-1;
        *chipsize=tolsize;
    } else {                     //if exist,set 0
        if(id) {
            part_offset[id-1]=0;
            //printf("++part[%d]=%lx\n",id-1,part_offset[id-1]);
        }
        *chipsize=tolsize-lstsize;
    }
    free(part_head);
    //printf("chipsize=%x\n",*chipsize);
    return 0;
}

static MT_S32 mt_snf_getlockrang(MTD_Part_Info *mtd_part_info_ptr,MT_U32 status,spi_prot_block_type_t *type)
{
    MT_S32 ret = 0;
    struct spi_flash_id id_info;
    unsigned int count = 0;
    snf_protect_info_t  *snf_info= NULL;
    snf_protect_info_t  *snf_info_end= NULL;
    unsigned char mid = 0;
    unsigned short did = 0;

    *type = 0xffffffff;
    if(NULL == mtd_part_info_ptr) {
        return ERROR_CODE_ERROR_PARM;
    }
    ret = ioctl(mtd_part_info_ptr->handle, MEMGETFLASHID, &id_info);
    if(0 == ret) {
        mid = (id_info.mid & 0xFF);
        did = id_info.did & 0xFFFF;
    }

    if(mtd_part_info_ptr->type==MTD_NORFLASH) {
        return MT_FAILURE;
    } else if(mtd_part_info_ptr->type==MTD_NANDFLASH) {
        if((0xef==mid)||(0xea==mid)) {	//3-valid-addr
            ;
        } else {												//2-valid-addr
            did &= 0xFF00;
        }
        snf_info = snand_flash_prot_t;
        count = sizeof(snand_flash_prot_t) / sizeof(snf_protect_info_t);
        snf_info_end = snand_flash_prot_t + count;
    }

    for(snf_info=snand_flash_prot_t; snf_info < snf_info_end; snf_info++) {
        if((snf_info->m_id == mid) &&
           ((0x00==snf_info->d_id) || (snf_info->d_id == did))) {
            if((snf_info->prot_st.st&snf_info->prot_st.st_mask) == (status&snf_info->prot_st.st_mask)) {
                *type = snf_info->prot_st.prt_t;
                return MT_SUCCESS;
            }
        }
    }
    return MT_FAILURE;
}

static MT_S32 mt_snf_check_ew(MTD_Part_Info * mtd_part_info_ptr, MT_U64 erase_offset, MT_U32 erase_size)
{
    MT_U64 vld_s=0,vld_e=0,ew_s=0,ew_e=0;

    switch(mtd_part_info_head.type) {
    case PRT_UNPROT_ALL:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_PROT_ALL:
        vld_s=0;
        vld_e=0;
        break;
    case PRT_UPPER_1_64:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize/64;
        break;
    case PRT_UPPER_1_32:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize/32;
        break;
    case PRT_UPPER_1_16:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize/16;
        break;
    case PRT_UPPER_1_8:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize/8;
        break;
    case PRT_UPPER_1_4:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize/4;
        break;
    case PRT_UPPER_1_2:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize/2;
        break;
    case PRT_UPPER_3_4:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize*3/4;
        break;
    case PRT_UPPER_7_8:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize*7/8;
        break;
    case PRT_UPPER_15_16:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize*15/16;
        break;
    case PRT_UPPER_31_32:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize*31/32;
        break;
    case PRT_UPPER_63_64:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize-mtd_part_info_head.chipsize*63/64;
        break;
    case PRT_LOWER_1_256:
        vld_s=mtd_part_info_head.chipsize/256;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_1_128:
        vld_s=mtd_part_info_head.chipsize/128;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_1_64:
        vld_s=mtd_part_info_head.chipsize/64;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_1_32:
        vld_s=mtd_part_info_head.chipsize/32;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_1_16:
        vld_s=mtd_part_info_head.chipsize/16;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_1_8:
        vld_s=mtd_part_info_head.chipsize/8;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_1_4:
        vld_s=mtd_part_info_head.chipsize/4;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_1_2:
        vld_s=mtd_part_info_head.chipsize/2;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_3_4:
        vld_s=mtd_part_info_head.chipsize*3/4;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_7_8:
        vld_s=mtd_part_info_head.chipsize*7/8;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_15_16:
        vld_s=mtd_part_info_head.chipsize*15/16;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_31_32:
        vld_s=mtd_part_info_head.chipsize*31/32;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_63_64:
        vld_s=mtd_part_info_head.chipsize*63/64;
        vld_e=mtd_part_info_head.chipsize;
        break;
    case PRT_LOWER_127_128:
        vld_s=mtd_part_info_head.chipsize*127/128;
        vld_e=mtd_part_info_head.chipsize;
        break;
    default:
        vld_s=0;
        vld_e=mtd_part_info_head.chipsize;
        break;
    }
    ew_s=mtd_part_info_head.part_offset[mtd_part_info_ptr->partid]+erase_offset;
    ew_e=ew_s+erase_size;
    if((ew_s>=vld_s) && (ew_e<=vld_e)) {
        //printf("chk.ew:[%llx,%llx],[%llx,%llx] tp=%x\n",vld_s,vld_e,ew_s,ew_e,mtd_part_info_head.type);
        return MT_SUCCESS;
    }
    MT_ERR_FLASH("chk.ew:[%llx,%llx],[%llx,%llx] tp=%x\n",vld_s,vld_e,ew_s,ew_e,mtd_part_info_head.type);
    return MT_FAILURE;
}

static MT_BOOL flash_isgoodblock(MTD_Part_Info *partinfo,u32 blockid)
{
    if(MTD_NORFLASH==partinfo->type) {
        return MT_TRUE;
    } else {
        off_t ioret=0;
        MT_U64 tmp_offset = blockid*partinfo->eraseSize;
        ioret=ioctl(partinfo->handle,MEMGETBADBLOCK,&tmp_offset);
        if(0==ioret) {
            return MT_TRUE;
        }
        return MT_FALSE;
    }
}
static MT_S32 seek_to_good_block(MT_S32 handler, MT_U32 dev_size, MT_U64 block_size, MT_U64 *start_offset)
{
    MT_U64 tmp_offset = 0;
    MT_S32 ret = -1;

    if(handler < 0 || dev_size == 0 || block_size == 0 || start_offset == NULL) {
        return ERROR_CODE_ERROR_PARM;
    }
    tmp_offset = 0;
    tmp_offset = *start_offset;
next_block:
    if(tmp_offset >= dev_size) {
        ret = ERROR_CODE_NO_ERROR;
        goto end;
    }

    ret = ioctl(handler,MEMGETBADBLOCK,&tmp_offset);
    if(ret>0) {       //发现坏块
        tmp_offset +=block_size;
        goto next_block;
    } else if(ret <0) {
        ret = ERROR_CODE_ERROR_GET_BAD_BLOCK_FUNCTION_NOTSUPP;
        goto end;
    } else {	//当前块为好块
        ret = ERROR_CODE_NO_ERROR;
    }

end:
    *start_offset = tmp_offset;
    return ret;
}

static MT_S32 flash_init(MTD_Part_Info* part_info)
{
    MT_S32 ret = 0;
    MT_U64      offset = 0;
    int         bad_block = 0;
    struct mtd_info_user mtd_usr_info = {0};

    if(part_info == NULL) {
        return ERROR_CODE_ERROR_PARM;
    }
    if(part_info->handle < 0) {
        return ERROR_CODE_ERROR_PARM;
    }

    ret = ioctl(part_info->handle, MEMGETINFO, &mtd_usr_info);

    if(ret < 0) {
        return ERROR_CODE_ERROR_RESULT;
    }

    //严防除0错误
    if(mtd_usr_info.erasesize == 0) {
        return ERROR_CODE_ERROR_RESULT;
    }

    part_info->size = mtd_usr_info.size;
    part_info->eraseSize = mtd_usr_info.erasesize;
    part_info->blockNum = mtd_usr_info.size/mtd_usr_info.erasesize;
    part_info->page_write_size = mtd_usr_info.writesize;
    part_info->page_oob_size = mtd_usr_info.oobsize;
    if (0 == part_info->page_write_size) {
        return ERROR_CODE_ERROR_RESULT;
    }


    part_info->pages_per_block = part_info->eraseSize/part_info->page_write_size;
    if (0 == part_info->pages_per_block) {
        return ERROR_CODE_ERROR_RESULT;
    }

    //计算该MTD中可用空间大小
    part_info->type = mtd_usr_info.type;

    if(part_info->type == MTD_NANDFLASH) {
        part_info->availableSize = 0;
        for (offset = 0; offset<part_info->size; offset+=part_info->eraseSize) {
            if ((bad_block = ioctl(part_info->handle,MEMGETBADBLOCK,&offset))<0) {
                return bad_block;
            }

            if (bad_block == 0) {
                part_info->availableSize += part_info->eraseSize;
            }
        }
    } else {
        part_info->availableSize = mtd_usr_info.size;
    }

    return ret;
}

static MT_S32 nand_read_an_page(MT_S32 handler,void *buffer, MT_U32 read_size, MT_U64 read_offset)
{
    MT_S32 ret = 0;
    off_t f_ops_rt=0;
    ssize_t f_ops_rw=0;
    void *read_buffer = NULL;

    if(handler<0 || buffer==NULL || read_size==0) {
        return ERROR_CODE_ERROR_PARM;
    }

    read_buffer = (void*)buffer;

    pthread_mutex_lock(&mtd_part_info_head.WRMutex);
    f_ops_rt = lseek(handler, (off_t)read_offset, SEEK_SET);
    if(f_ops_rt < 0) {
        ret = ERROR_CODE_ERROR_RESULT;
    } else {
        f_ops_rw = read(handler,read_buffer,read_size);
        if(f_ops_rw == (ssize_t)read_size) {
            ret = ERROR_CODE_NO_ERROR;
        } else {
            ret = ERROR_CODE_ERROR_RESULT;
        }
    }
    pthread_mutex_unlock(&mtd_part_info_head.WRMutex);
    return ret;
}

static MT_S32 mtd_nand_flash_read(MTD_Part_Info* partInfo, MT_U32 flash_addr, void *buff, MT_U32 len)
{
    MT_S32 ret = ERROR_CODE_NO_ERROR;
    MT_U32 start_block = 0, start_page = 0, num_pages = 0, i = 0, j = 0;
    MT_U32 flash_size = 0, from_page_offset = 0, temp_length = 0, already_read_data = 0;
    MT_U64 offset = 0;
    MT_U8 *read_mem = NULL, *read_store_mem = NULL;

    if(partInfo == NULL || buff == NULL || len == 0) {
        return ERROR_CODE_ERROR_PARM;
    }

    flash_size = partInfo->size;

    if((flash_addr + len) > flash_size || len > partInfo->availableSize) {
        return ERROR_CODE_ERROR_PARM;
    }

    num_pages = 0;
    offset = 0;
    already_read_data = 0;

    //计算起始位置的block和page号
    from_page_offset = flash_addr%partInfo->page_write_size;
    start_block = flash_addr/partInfo->eraseSize;
    start_page = (flash_addr -start_block*partInfo->eraseSize)/partInfo->page_write_size;

    if(0==partInfo->flag_lastblkid) { //jump bad
        u32 blkid=0,rawid=0;
        u32 tailid=(partInfo->size/partInfo->eraseSize);
        //printf("jump read\n");
        if(0==tailid) {
            return ERROR_CODE_ERROR_PARM;
        }
        tailid--;

        for(blkid=0; blkid<start_block;) {
            if(flash_isgoodblock(partInfo,rawid)) {
                blkid++;
            }
            if(rawid==tailid) {
                break;
            }
            rawid++;
        }
        start_block=rawid;
    }

    num_pages = 1;
    if(len > (partInfo->page_write_size-from_page_offset)) {
        temp_length = len -(partInfo->page_write_size-from_page_offset);
        if(temp_length > partInfo->page_write_size) {
            num_pages += temp_length/partInfo->page_write_size;
            if(temp_length%partInfo->page_write_size != 0) { //理论说这一判断不会进入
                num_pages++;
            }
        } else {
            num_pages++;
        }
    }
#ifdef JUMP_ALL_BAD_BLOCK
    if (start_block >= 0) {
        for (i = start_block; i >= 0; i--) {
            ret = seek_to_good_block(partInfo->handle,partInfo->size,partInfo->eraseSize,&offset);
            if (ret != 0) {
                return ERROR_CODE_ERROR_READ;
            }
            if(i != 0) {
                offset +=partInfo->eraseSize;
            }
        }
    } else {
        return ERROR_CODE_ERROR_READ;
    }
#else
    offset = start_block*partInfo->eraseSize;
    ret = seek_to_good_block(partInfo->handle,flash_size,partInfo->eraseSize,&offset);
    if (ret != 0) {
        ret = ERROR_CODE_ERROR_READ;
        goto end;
    }
#endif
    offset += start_page*partInfo->page_write_size;
    read_store_mem = malloc(partInfo->page_write_size);
    memset(read_store_mem,0,partInfo->page_write_size);
    if (read_store_mem == 0) {
        return ERROR_CODE_ERROR_NOTENOUGHMEM;
    }
    read_mem = read_store_mem;

    j=num_pages;
    for (i =(partInfo->pages_per_block -start_page) ; (i > 0)&&(j > 0) ; i--,j--) {
        if (offset >= flash_size) {
            ret = ERROR_CODE_ERROR_READ;
            goto end;
        }

        if (j == num_pages) {
            ret = nand_read_an_page(partInfo->handle,read_mem,partInfo->page_write_size,offset);
            if (ret != 0) {
                ret = ERROR_CODE_ERROR_RESULT;
                goto end;
            }

            if ((partInfo->page_write_size - from_page_offset) >= len) {
                memcpy(buff,(read_store_mem+from_page_offset),len);
                read_mem = (MT_U8*)buff+len;
                already_read_data = len;
            } else {
                memcpy(buff,(read_store_mem+from_page_offset),(partInfo->page_write_size - from_page_offset));
                read_mem = (MT_U8*)buff+(partInfo->page_write_size - from_page_offset);
                already_read_data = partInfo->page_write_size - from_page_offset;
            }

            offset += partInfo->page_write_size;
            continue;
        }

        if (j == 1) {
            temp_length = partInfo->page_write_size;
            ret = nand_read_an_page(partInfo->handle,read_store_mem,partInfo->page_write_size,offset);
            if (ret != 0 ) {
                ret = ERROR_CODE_ERROR_RESULT;
                goto end;
            }
            memcpy(read_mem,read_store_mem,(len - already_read_data));
            read_mem += (len - already_read_data);
            already_read_data =len;
            offset +=partInfo->page_write_size;
            continue;
        }

        ret = nand_read_an_page(partInfo->handle,read_mem,partInfo->page_write_size,offset);
        if (ret != 0) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }

        read_mem +=partInfo->page_write_size;
        offset +=partInfo->page_write_size;
        already_read_data +=partInfo->page_write_size;
    }
    i = 0;
    for (; j>0; j--) {
        if (i==0||i >= partInfo->pages_per_block) {
            ret = seek_to_good_block(partInfo->handle,partInfo->size, partInfo->eraseSize, &offset);
            if (ret != 0) {
                ret = ERROR_CODE_ERROR_READ;
                goto end;
            }

            if (i >= partInfo->pages_per_block) {
                i = 0;
            }
        }

        if (j == 1) {
            temp_length = partInfo->page_write_size;
            ret = nand_read_an_page(partInfo->handle,read_store_mem,partInfo->page_write_size,offset);
            if (ret != 0 ) {
                ret = ERROR_CODE_ERROR_RESULT;
                goto end;
            }
            memcpy(read_mem,read_store_mem,(len - already_read_data));
            break;
        }

        ret = nand_read_an_page(partInfo->handle,read_mem,partInfo->page_write_size,offset);
        if (ret != 0) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }

        read_mem +=partInfo->page_write_size;
        offset +=partInfo->page_write_size;
        already_read_data += partInfo->page_write_size;
        i++;
    }

    ret = ERROR_CODE_NO_ERROR;
end:
    free(read_store_mem);
    return ret;
}

static MT_S32 nand_erase_an_block(MT_S32 handler, MT_U64 erase_offset, MT_U32 erase_size)
{
    MT_S32 ret = 0;
    struct erase_info_user erase_param= {0};

    if(handler<0 || erase_size==0) {
        return ERROR_CODE_ERROR_PARM;
    }

    erase_param.length = erase_size;
    erase_param.start = (MT_U32)erase_offset;
    ret = ioctl(handler,MEMERASE,&erase_param);
    if(ret<0) {
        if(errno == EIO) {
            ret = MT_FLASH_ERR_IO;
        } else {
            ret = ERROR_CODE_ERROR_RESULT;
        }
    } else {
        ret = ERROR_CODE_NO_ERROR;
    }

    return ret;
}

static MT_S32 nand_write_an_block(MT_S32 handler,const void *buffer, MT_U32 block_size, MT_U64 write_offset)
{
    MT_S32 ret = ERROR_CODE_NO_ERROR;
    off_t f_ops_rt=0;
    ssize_t f_ops_rw=0;

    if(handler<0 || buffer==NULL || block_size==0 || (write_offset%block_size)!=0 ) {
        return ERROR_CODE_ERROR_PARM;
    }
    pthread_mutex_lock(&mtd_part_info_head.WRMutex);
    f_ops_rt = lseek(handler, (off_t)write_offset, SEEK_SET);
    if(f_ops_rt < 0) {
        ret = ERROR_CODE_ERROR_RESULT;
    } else {
        f_ops_rw = write(handler,buffer,block_size);
        if(f_ops_rw < (ssize_t)block_size) {
            if(errno == EIO) {
                ret = MT_FLASH_ERR_IO;
            } else {
                ret = ERROR_CODE_ERROR_RESULT;
            }
        } else {
            ret = ERROR_CODE_NO_ERROR;
        }
    }
    pthread_mutex_unlock(&mtd_part_info_head.WRMutex);

    return ret;
}
static MT_S32 nand_write_an_block_raw(MTD_Part_Info* partInfo,void *buffer, MT_S32 block_size, MT_U64 write_offset)
{
    MT_S32 ret = ERROR_CODE_NO_ERROR;
    MT_U8 *buf = (MT_U8*) buffer;
    struct mtd_write_req page_req= {0};

    if(partInfo->handle<0 || buffer==NULL || block_size==0 ) {
        //printf("err2\n");
        return (MT_S32)ERROR_CODE_ERROR_PARM;
    }

    while (block_size > 0) {
        page_req.start=(u64)write_offset;
        page_req.len=(u64)partInfo->page_write_size;
        page_req.ooblen=(u64)partInfo->page_oob_size;
        page_req.usr_data=(ulong)buf;
        page_req.usr_oob=(ulong)(buf+partInfo->page_write_size);
        page_req.mode=MTD_OPS_PLACE_OOB;
        ret = ioctl(partInfo->handle,MEMWRITE,&page_req);
        if(ret) {
            break;
        }
        /*if(0){//test code!
        		u8 *rbuff=malloc(4*1024);
        		lseek(partInfo->handle, (long)write_offset , SEEK_SET);
        		read(partInfo->handle,rbuff,partInfo->page_write_size);
        		if(0!=memcmp(rbuff,buf,partInfo->page_write_size)){
        			printf("rw not same 1,%x,%x\n",buf[0],rbuff[0]);
        		}
        		free(rbuff);
        }*/
        write_offset += partInfo->page_write_size;
        buf += (partInfo->page_write_size + partInfo->page_oob_size);
        block_size -= (MT_S32)(partInfo->page_write_size + partInfo->page_oob_size);
    }

    return ret;
}

static MT_S32 nand_read_an_block(MT_S32 handler,void *buffer, MT_S32 block_size, MT_U64 read_offset)
{
    MT_S32 ret = ERROR_CODE_NO_ERROR;
    off_t f_ops_rt=0;
    ssize_t f_ops_rw=0;

    if(handler<0 || buffer==NULL || block_size==0 || (read_offset%(MT_U64)block_size)!=0 ) {
        return ERROR_CODE_ERROR_PARM;
    }
    pthread_mutex_lock(&mtd_part_info_head.WRMutex);
    f_ops_rt = lseek(handler, (off_t)read_offset, SEEK_SET);
    if(f_ops_rt < 0) {
        ret = ERROR_CODE_ERROR_RESULT;
    } else {
        f_ops_rw = read(handler,buffer,(unsigned int)block_size);
        if(f_ops_rw < (ssize_t)block_size) {
            ret = ERROR_CODE_ERROR_RESULT;
        } else {
            ret = ERROR_CODE_NO_ERROR;
        }
    }
    pthread_mutex_unlock(&mtd_part_info_head.WRMutex);
    return ret;
}

static MT_S32 nand_mark_bad_block(MT_S32 handler,MT_U64 mark_offset)
{
    MT_U64 tmp_offset = 0;
    MT_S32 ret = -1;

    tmp_offset = mark_offset;

    if( handler<0) {
        return ERROR_CODE_ERROR_PARM;
    }
    MT_INFO_FLASH("mtd flash address 0x%llx mark bad block\n",mark_offset);
    ret = ioctl(handler,MEMSETBADBLOCK,&tmp_offset);
    if(ret==0) {
        return ERROR_CODE_NO_ERROR;
    } else {
        return ERROR_CODE_ERROR_RESULT;
    }
}

/***************************************************************************************************
 * |  Descript | 挽救数据. 将有效数据顺延写到后面的好的block中(这里会用到递归调用)
 * |           |
 * |  Input | handler:		MTD句柄(打开MTD时获得)
 * |           | block_size:	NAND的块大小
 * |           | page_size:		NAND的页大小
 * |           | salvage_offset:	要挽救的数据相对MTD的偏移量
 * |           | dev_size:		该MTD区的大小
 * |           | buffer:		指的是要挽救的一个block长度数据
 * |  Output    | NONE
 * |  Return     | int
 * |  Warning  | 1.调用时read_offset必须为page_size(即writesize)的整数倍
 * |                 | 2.关于递归调用的相关描述:
 * |                 |   (1)递归调用发生在挽救数据时, 在其它block上也发生了数据写错误
 * |                 |   (2)递归调用发生时, 将会增加内存malloc的数量, 每增加一次递归, 内存消耗增加:
 * |                 |      (block_size+BUFFER_MARGIN)*2, 其中常见的block_size=128KB, 256KB, 512KB
 * |                 |   (3)挽救过程中发生读错误, 将会导致挽救数据失败
***************************************************************************************************/
static int nand_data_salvage(MTD_Part_Info* partInfo,MT_U64 salvage_offset, MT_U64 dev_size, const void *buffer)
{
    MT_S32 ret = 0;
    MT_U64 tmp_offset = 0;
    MT_S32 handler = 0;
    MT_U32 block_size = 0;
    MT_U8 *write_buffer = NULL,*read_buffer = NULL,*read_buffer_t = NULL,*transfer_buffer = NULL;	//由于要对指针进行加减操作, 指明指针类型比较可靠
    handler = partInfo->handle;
    block_size = partInfo->eraseSize;
    if( handler<0 || block_size==0 || salvage_offset>=dev_size || dev_size==0 || dev_size%block_size!=0 || buffer==NULL) {
        return ERROR_CODE_ERROR_PARM;
    }

    tmp_offset = salvage_offset;

    read_buffer = malloc(block_size);
    if(read_buffer == NULL) {
        return -1;
    }

    write_buffer = malloc(block_size);
    if(write_buffer == NULL) {
        free(read_buffer);
        return -1;
    }
    memcpy(write_buffer,buffer,block_size);

salvage_next_data:
    read_buffer_t = read_buffer;
    //找到下一个好的block
    if(tmp_offset>=dev_size || ((partInfo->salvage_end_addr>0)&&(tmp_offset>=partInfo->salvage_end_addr))) {
        goto end;
    }
    ret = seek_to_good_block(handler, (MT_U32)dev_size, block_size, &tmp_offset);
    if(ret != ERROR_CODE_NO_ERROR) {
        goto end;
    }
    //先将这个好的block的原有数据读出
    ret = nand_read_an_block(handler,read_buffer_t,(MT_S32)block_size,tmp_offset);//nand_flash_read(partInfo,tmp_offset,read_buffer_t,block_size);
    if (ret != ERROR_CODE_NO_ERROR) {
        ret = ERROR_CODE_ERROR_RESULT;
        goto end;
    }

    //擦除这个好block
    ret = nand_erase_an_block(handler, tmp_offset, block_size);
    if(ret == MT_FLASH_ERR_IO) {
        nand_mark_bad_block(handler, tmp_offset);
        nand_data_salvage(partInfo,tmp_offset+block_size,dev_size,read_buffer);
        goto salvage_next_data;	//如果擦除失败, 则顺延写入一个block操作
    } else if(ret != 0) {
        ret = ERROR_CODE_ERROR_RESULT;
        goto end;
    }

    //将要挽救的数据写入这个好block
    ret = nand_write_an_block(handler,write_buffer,block_size,tmp_offset);
    if(ret == MT_FLASH_ERR_IO) {
        nand_mark_bad_block(handler,tmp_offset);
        nand_data_salvage(partInfo,tmp_offset+block_size, dev_size, read_buffer);
        goto salvage_next_data;	//如果写入失败, 则顺延写入一个block操作
    } else if(ret != 0) {
        ret = ERROR_CODE_ERROR_RESULT;
        goto end;
    }
    //保存这个好块的原来数据指针
    transfer_buffer = read_buffer;
    read_buffer = write_buffer;
    write_buffer = transfer_buffer;
    tmp_offset += block_size;
    goto salvage_next_data;	//顺延写入下一个block

end:
    free(read_buffer);
    free(write_buffer);
    return 0;
}

static MT_S32 mtd_nand_flash_write(MTD_Part_Info* partInfo,MT_U32 flash_addr,void *buff, MT_U32 len)
{
    MT_S32 ret = ERROR_CODE_NO_ERROR;
    MT_U32 start_block = 0;		//起始block
    MT_U32 start_page = 0;			//在起始block中所处的page号.(不是从device的头开始计算)
    MT_U32 num_blocks = 0;			//
#ifdef JUMP_ALL_BAD_BLOCK
    MT_U32 i = 0;
#endif
    MT_U32 j = 0;
    MT_U32 front_part_block=0;	//该值在整个函数过程中, 始终为1  ?? lcz
    MT_U32 back_part_block=0;
    MT_U32 flash_size = 0;			//该MTD区的大小(包括坏块的物理大小)
    MT_U32 from_page_offset = 0;	//在第一个要写入的page中, 从该page的左边界计算的offset. 取值范围是[0, pagesize-1]
    MT_U32 from_block_offset = 0;	//在第一个要写入的block中, 从该block的左边界计算的offset. 取值范围是[0, blocksize-1]
    MT_U32 left_length = 0;
    MT_U32 temp_length = 0;
    MT_U32 read_addr = 0;
    MT_U64 offset = 0;
    MT_U8 *write_mem = NULL, *flash_write_buffer = NULL;
    MT_U8 salvage_happened = 0;
    MT_S32 handler = 0;

    MT_U64 write_cnt = 0;

    if(partInfo==NULL || buff==NULL || len==0) {
        return ERROR_CODE_ERROR_PARM;
    }

    left_length = len;
    flash_size = partInfo->size;
    if( (flash_addr+len) > flash_size || (len>partInfo->availableSize )) {
        return ERROR_CODE_ERROR_PARM;
    }

    if ((partInfo->salvage_end_addr>0) && ((flash_addr+len) > partInfo->salvage_end_addr)) {
        return ERROR_CODE_ERROR_PARM;
    }
    handler = partInfo->handle;
    num_blocks = 0;

    write_mem = (MT_U8*)buff;
    offset = 0;

    start_block = flash_addr/partInfo->eraseSize;
    start_page = (flash_addr -start_block*partInfo->eraseSize)/partInfo->page_write_size;
    from_page_offset =( flash_addr -(flash_addr/partInfo->page_write_size)*partInfo->page_write_size);
    from_block_offset = start_page*partInfo->page_write_size + from_page_offset;

    if(0==partInfo->flag_lastblkid) {
        u32 blkid=0,rawid=0;
        u32 tailid=partInfo->size/partInfo->eraseSize;
        //printf("jump write\n");
        if(0==tailid) {
            return ERROR_CODE_ERROR_PARM;
        }
        tailid--;

        for(blkid=0; blkid<start_block;) { //jump bad
            if(flash_isgoodblock(partInfo,rawid)) {
                blkid++;
            }
            if(rawid==tailid) {
                break;
            }
            rawid++;
        }
        start_block=rawid;
    }

    if (from_block_offset) {
        front_part_block = 1;
    } else {
        front_part_block = 0;
    }

    if (len >= (partInfo->eraseSize - from_block_offset)) {
        if (from_block_offset) {
            temp_length =  len - (partInfo->eraseSize - from_block_offset);
        } else {
            temp_length =  len;
        }
        if (temp_length >= partInfo->eraseSize) {
            num_blocks = temp_length/partInfo->eraseSize;
            if (temp_length%partInfo->eraseSize != 0) {
                back_part_block = 1;
            }
        } else if (temp_length > 0) {
            back_part_block = 1;
        }
    } else {
        if (front_part_block) {
            back_part_block = 0;
        } else {
            back_part_block = 1;
        }
    }
#ifdef JUMP_ALL_BAD_BLOCK
    if (start_block >= 0) {
        for (i = start_block; i >= 0; i--) {
            ret = seek_to_good_block(handler,flash_size,partInfo->eraseSize,&offset);
            if (ret != 0) {
                return ERROR_CODE_ERROR_READ;
            }
            if(i != 0 ) {
                offset += partInfo->eraseSize;
            }
        }
    } else {
        return ERROR_CODE_ERROR_PARM;
    }
#else
    offset = start_block*partInfo->eraseSize;
    ret = seek_to_good_block(handler,flash_size,partInfo->eraseSize,&offset);
    if (ret != 0) {
        ret = ERROR_CODE_ERROR_READ;
        goto end;
    }
#endif
    //MT_ERR_FLASH("%d:offset:%llx  flash_addr:%x len:%x   from_block_offset:%x  back_part_block:%x\n",__LINE__,offset,flash_addr,len,from_block_offset,back_part_block);
    flash_write_buffer = malloc(partInfo->eraseSize);
    if (flash_write_buffer == 0) {
        return ERROR_CODE_ERROR_NOTENOUGHMEM;
    }

    if (front_part_block == 1) {
        read_addr = (MT_U32)offset;
        ret = mtd_nand_flash_read(partInfo,read_addr,flash_write_buffer,partInfo->eraseSize);
        if (ret != ERROR_CODE_NO_ERROR) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }

        if ((partInfo->eraseSize - from_block_offset) >= left_length) {
            memcpy((flash_write_buffer+from_block_offset),write_mem,left_length);
            write_cnt += left_length;
            left_length = 0;
        } else {
            memcpy((flash_write_buffer+from_block_offset),write_mem,(partInfo->eraseSize - from_block_offset));
            write_mem +=(partInfo->eraseSize - from_block_offset);
            left_length = left_length -(partInfo->eraseSize - from_block_offset);

            write_cnt += partInfo->eraseSize - from_block_offset;
        }

        ret = nand_erase_an_block(handler, offset, partInfo->eraseSize);
        if(ret == MT_FLASH_ERR_IO) {
            nand_mark_bad_block(handler, offset);
            nand_data_salvage(partInfo,(offset+partInfo->eraseSize), flash_size,flash_write_buffer);
            salvage_happened = 1;
            goto next_writeable_block;
        } else if (ret != 0) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }

        ret = nand_write_an_block(handler, flash_write_buffer, partInfo->eraseSize, offset);
        if(ret == MT_FLASH_ERR_IO) {
            nand_mark_bad_block(handler, offset);
            nand_data_salvage(partInfo,(offset+partInfo->eraseSize), flash_size,flash_write_buffer);
            salvage_happened = 1;
        } else if (ret != 0) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }

next_writeable_block:
        if(partInfo->pcallback) {
            partInfo->pcallback(MSG_WRITE,write_cnt);
        }
        offset += partInfo->eraseSize;
    }

    for (j = num_blocks; j > 0; j--) {
skip_this_block:
        ret = seek_to_good_block(handler, flash_size, partInfo->eraseSize, &offset);
        if (ret != 0) {
            ret = ERROR_CODE_ERROR_READ;
            goto end;
        }

        if (salvage_happened == 1) {
            offset += partInfo->eraseSize;
            salvage_happened = 0;
            goto skip_this_block;
        }

        ret = nand_erase_an_block(handler, offset, partInfo->eraseSize);
        if(ret == MT_FLASH_ERR_IO) {
            nand_mark_bad_block(handler, offset);
            nand_data_salvage(partInfo,(offset+partInfo->eraseSize), flash_size,write_mem);
            salvage_happened = 1;
            goto next_writeable_block2;
        } else if (ret != 0) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }

        ret = nand_write_an_block(handler, write_mem, partInfo->eraseSize, offset);
        if(ret == MT_FLASH_ERR_IO) {
            nand_mark_bad_block(handler, offset);
            nand_data_salvage(partInfo,(offset+partInfo->eraseSize), flash_size,write_mem);
            salvage_happened = 1;
        } else if (ret != 0) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }

next_writeable_block2:
        write_mem +=partInfo->eraseSize;
        offset +=partInfo->eraseSize;
        left_length = left_length -partInfo->eraseSize;

        write_cnt += partInfo->eraseSize;
        if(partInfo->pcallback) {
            partInfo->pcallback(MSG_WRITE,write_cnt);
        }
    }

    if (back_part_block == 1) {
skip_this_block2:
        ret = seek_to_good_block(handler, flash_size, partInfo->eraseSize, &offset);
        if (ret != 0) {
            ret = ERROR_CODE_ERROR_READ;
            goto end;
        }

        if (salvage_happened == 1) {
            offset += partInfo->eraseSize;
            salvage_happened = 0;
            goto skip_this_block2;
        }

        read_addr = (MT_U32)offset;
        ret = mtd_nand_flash_read(partInfo,read_addr,flash_write_buffer,partInfo->eraseSize);
        if (ret != ERROR_CODE_NO_ERROR) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }
        write_cnt += left_length;
        memcpy(flash_write_buffer,write_mem,left_length);
        ret = nand_erase_an_block(handler, offset, partInfo->eraseSize);
        if(ret == MT_FLASH_ERR_IO) {
            nand_mark_bad_block(handler, offset);
            nand_data_salvage(partInfo,(offset+partInfo->eraseSize), flash_size,flash_write_buffer);
            goto next_writeable_block3;
        } else if (ret != 0) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }

        ret = nand_write_an_block(handler, flash_write_buffer, partInfo->eraseSize, offset);
        if(ret == MT_FLASH_ERR_IO) {
            nand_mark_bad_block(handler, offset);
            nand_data_salvage(partInfo,(offset+partInfo->eraseSize),flash_size,flash_write_buffer);
        } else if(ret != 0) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }

next_writeable_block3:
        if(partInfo->pcallback) {
            partInfo->pcallback(MSG_WRITE,write_cnt);
        }
        offset += partInfo->eraseSize;
    }

    ret = ERROR_CODE_NO_ERROR;
end:
    free(flash_write_buffer);
    return ret;
}

static MT_S32 mtd_nand_flash_write_raw(MTD_Part_Info* partInfo,MT_U32 flash_addr,void *buff, MT_U32 len)
{
    MT_S32 ret = ERROR_CODE_NO_ERROR;
    MT_U32 num_blocks = 0;
    MT_U32 j = 0;
    MT_U32 flash_size = 0;			//该MTD区的大小(包括坏块的物理大小)
    MT_U64 offset = 0;
    MT_U8 *write_mem = NULL;
    MT_S32 handler = 0;
    MT_U32 oobsize_perblock = 0;
    MT_U64 write_cnt = 0;
    MT_U32 pure_data_len=0;

    if(partInfo==NULL || buff==NULL || len==0) {
        return ERROR_CODE_ERROR_PARM;
    }

    pure_data_len=len/(partInfo->page_write_size + partInfo->page_oob_size)*partInfo->page_write_size;

    flash_size = partInfo->size;
    oobsize_perblock = partInfo->eraseSize / partInfo->page_write_size * partInfo->page_oob_size;
    if( (flash_addr+pure_data_len) > flash_size || ((flash_addr+pure_data_len)>partInfo->availableSize )) {
        return ERROR_CODE_ERROR_PARM;
    }
    if ((partInfo->salvage_end_addr>0) && ((flash_addr+pure_data_len) > partInfo->salvage_end_addr)) {
        return ERROR_CODE_ERROR_PARM;
    }
    if(flash_addr % partInfo->eraseSize) {
        return ERROR_CODE_ERROR_PARM;
    }
    handler = partInfo->handle;
    num_blocks = len/(partInfo->eraseSize + oobsize_perblock);
    if((num_blocks*(partInfo->eraseSize + oobsize_perblock)) != len) {
        MT_ERR_FLASH("error:%s size need to align to block with oob\n",__FUNCTION__);
        return ERROR_CODE_ERROR_PARM;
    }
    write_mem = (MT_U8*)buff;
    if ((partInfo->salvage_end_addr>0) && (flash_size > partInfo->salvage_end_addr)) {
        flash_size = partInfo->salvage_end_addr;
    }
    offset = flash_addr;
    ret = seek_to_good_block(handler,flash_size,partInfo->eraseSize,&offset);
    if (ret != 0) {
        ret = ERROR_CODE_ERROR_READ;
        goto end;
    }
    for (j = num_blocks; j > 0; j--) {
skip_this_block:
        ret = seek_to_good_block(handler, flash_size, partInfo->eraseSize, &offset);
        if (ret != 0) {
            ret = ERROR_CODE_ERROR_READ;
            goto end;
        }
        ret = nand_erase_an_block(handler, offset, partInfo->eraseSize);
        if(ret == MT_FLASH_ERR_IO) {
            nand_mark_bad_block(handler, offset);
            offset +=partInfo->eraseSize;
            goto skip_this_block;
        } else if(ret != 0) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }
        ret = nand_write_an_block_raw(partInfo, write_mem, (MT_S32)(partInfo->eraseSize+oobsize_perblock), offset);
        if (ret == MT_FLASH_ERR_IO) {
            nand_mark_bad_block(handler, offset);
            offset +=partInfo->eraseSize;
            goto skip_this_block;
        } else if(ret != 0) {
            ret = ERROR_CODE_ERROR_RESULT;
            goto end;
        }
        write_mem +=partInfo->eraseSize+oobsize_perblock;
        offset +=partInfo->eraseSize;

        write_cnt += partInfo->eraseSize+oobsize_perblock;
        if(partInfo->pcallback) {
            partInfo->pcallback(MSG_WRITE,write_cnt);
        }
    }
    ret = ERROR_CODE_NO_ERROR;
end:
    return ret;
}

static MT_S32 mtd_flash_getstatus(MT_S32 handler, MT_U8 *pu8Buf, MT_U8 u8Length)
{
    MT_S32 ret = 0;
    struct status_reg_user status_info = {0};

    status_info.status_len = u8Length;
    ret = ioctl(handler, MEMGETSTATUSREG, &status_info);
    pu8Buf[0] = (MT_U8)(status_info.status & 0xFF);
    if(u8Length>1) {
        pu8Buf[1] = (MT_U8)((status_info.status >>8) & 0xFF);
    }

    return ret;
}

static MT_S32 mtd_flash_setstatus(MT_S32 handler, MT_U8* pu8Buf, MT_U8 u8Length)
{
    MT_S32 ret = 0;
    struct status_reg_user status_info = {0};

    status_info.status_len = u8Length;
    status_info.status = pu8Buf[0];
    if(u8Length>1) {
        status_info.status = (unsigned short)(status_info.status | (unsigned short)((pu8Buf[1] <<8) & 0xFF00));
    }
    ret = ioctl(handler, MEMSETSTATUSREG, &status_info);

    return ret;
}
static MT_S32 nor_flash_erase(MT_S32 handler, MT_U64 erase_offset, MT_U32 erase_size)
{
    MT_S32 ret = 0;
    struct erase_info_user erase_param = {0};

    if(handler < 0 || erase_size == 0) {
        return ERROR_CODE_ERROR_PARM;
    }

    erase_param.length = erase_size;
    erase_param.start = (MT_U32)erase_offset;

    ret = ioctl(handler, MEMERASE, &erase_param);
    if(ret!= 0) {
        ret = ERROR_CODE_ERROR_RESULT;
    } else {
        ret = ERROR_CODE_NO_ERROR;
    }

    return ret;
}

static MT_S32 nor_flash_read(MTD_Part_Info* partInfo,MT_U32 flash_addr, void *buff,MT_U32 len)
{
    MT_S32 ret = ERROR_CODE_NO_ERROR;
    MT_U32 flash_size = 0;
    off_t f_ops_rt=0;
    ssize_t f_ops_rw=0;

    if(partInfo == NULL || buff == NULL || len == 0) {
        return ERROR_CODE_ERROR_PARM;
    }

    flash_size = partInfo->size;

    if((flash_addr + len) > flash_size) {
        return ERROR_CODE_ERROR_PARM;
    }

    if(partInfo->handle < 0) {
        return ERROR_CODE_ERROR_PARM;
    }
    pthread_mutex_lock(&mtd_part_info_head.WRMutex);
    f_ops_rt = lseek(partInfo->handle, (off_t)flash_addr, SEEK_SET);
    if(f_ops_rt < 0) {
        ret = ERROR_CODE_ERROR_RESULT;
    } else {
        f_ops_rw = read(partInfo->handle, buff, len);
        if(f_ops_rw == (ssize_t)len) {
            ret = ERROR_CODE_NO_ERROR;
        } else {
            ret = ERROR_CODE_ERROR_RESULT;
        }
    }
    pthread_mutex_unlock(&mtd_part_info_head.WRMutex);

    return ret;
}

static MT_S32 nor_flash_write(MTD_Part_Info* partInfo, MT_U32 flash_addr, void *buff, MT_U32 len)
{
    MT_S32 ret = ERROR_CODE_NO_ERROR;
    MT_U32 flash_size = 0;
    off_t f_ops_rt=0;
    ssize_t f_ops_rw=0;

    if(partInfo == NULL || buff == NULL || len == 0) {
        return ERROR_CODE_ERROR_PARM;
    }

    flash_size = partInfo->size;

    if((flash_addr + len) > flash_size) {
        return ERROR_CODE_ERROR_PARM;
    }

    if(partInfo->handle < 0) {
        return ERROR_CODE_ERROR_PARM;
    }
    pthread_mutex_lock(&mtd_part_info_head.WRMutex);
    f_ops_rt = lseek(partInfo->handle, (off_t)flash_addr, SEEK_SET);
    if(f_ops_rt < 0) {
        ret = ERROR_CODE_ERROR_RESULT;
    } else {
        f_ops_rw = write(partInfo->handle, buff, len);
        if(f_ops_rw == (ssize_t)len) {
            ret = ERROR_CODE_NO_ERROR;
        } else {
            ret = ERROR_CODE_ERROR_RESULT;
        }
    }
    pthread_mutex_unlock(&mtd_part_info_head.WRMutex);

    return ret;
}
static MT_S32 mtd_nor_flash_write(MTD_Part_Info* partInfo, MT_U32 flash_addr, void *buff, MT_U32 len)
{
    MT_S32 ret = ERROR_CODE_NO_ERROR;
    MT_U32 flash_size = 0;
    MT_U32 length = 0,left_length = 0;
    MT_U32 offset = 0;
    MT_U32 block_off = 0,block_start = 0;
    MT_U8 * max_block_buf = NULL;
    MT_U8 * tmpbuf = NULL;

    if(partInfo == NULL || buff == NULL || len == 0) {
        return ERROR_CODE_ERROR_PARM;
    }
    tmpbuf = (MT_U8*)buff;
    flash_size = partInfo->size;
    if((flash_addr + len) > flash_size) {
        return ERROR_CODE_ERROR_PARM;
    }

    if(partInfo->handle < 0) {
        return ERROR_CODE_ERROR_PARM;
    }

    length = len;
    offset = flash_addr;
    max_block_buf = malloc(partInfo->eraseSize);
    if (!max_block_buf) {
        return ERROR_CODE_ERROR_NOBUFF;
    }
    /*1.get the start block to write*/
    block_start = offset & ~(partInfo->eraseSize - 1);
    block_off = offset&(partInfo->eraseSize - 1);
    /*block_off!=0 indicate offset is in the middle of block*/
    if (block_off>0) { //head section
        ret = nor_flash_read(partInfo,block_start,max_block_buf,partInfo->eraseSize);
        if (ret) {
            free(max_block_buf);
            return ret;
        }
        if (block_off + length <= partInfo->eraseSize) { //head section only
            memcpy(max_block_buf+block_off,tmpbuf,length);//data merge
        } else {
            memcpy(max_block_buf+block_off,tmpbuf,partInfo->eraseSize-block_off);//data merge
        }
        ret = nor_flash_erase(partInfo->handle, block_start, partInfo->eraseSize);
        if(ret) {
            free(max_block_buf);
            return ret;
        }
        ret = nor_flash_write(partInfo,block_start,max_block_buf,partInfo->eraseSize);
        if ((block_off + length <= partInfo->eraseSize) || ret) { //head section only
            free(max_block_buf);
            return ret;
        }
    }
    //there are tear and/or body section
    if (block_off>0) {
        tmpbuf += (partInfo->eraseSize-block_off);
        length -= (partInfo->eraseSize-block_off);
        offset += (partInfo->eraseSize-block_off);//offset should be block alligned.
    }
    if (length < partInfo->eraseSize) { //tear section
        ret = nor_flash_read(partInfo,offset,max_block_buf,partInfo->eraseSize);
        if (ret) {
            free(max_block_buf);
            return ret;
        }
        memcpy(max_block_buf,tmpbuf,length);//data merge
        ret = nor_flash_erase(partInfo->handle, offset, partInfo->eraseSize);
        if(ret) {
            free(max_block_buf);
            return ret;
        }
        ret = nor_flash_write(partInfo,offset,max_block_buf,partInfo->eraseSize);
        free(max_block_buf);
        return ret;
    } else { //body+tear section
        left_length = length&(partInfo->eraseSize - 1);
        length -= left_length;
        ret = nor_flash_erase(partInfo->handle, offset, length);
        if(ret) {
            free(max_block_buf);
            return ret;
        }
        ret = nor_flash_write(partInfo,offset,tmpbuf,length);
        if (ret) {
            free(max_block_buf);
            return ret;
        }

        if (left_length) {
            offset += length;
            ret = nor_flash_read(partInfo,offset,max_block_buf,partInfo->eraseSize);
            if (ret) {
                free(max_block_buf);
                return ret;
            }
            tmpbuf += length;
            memcpy(max_block_buf,tmpbuf,left_length);//data merge
            ret = nor_flash_erase(partInfo->handle, offset, partInfo->eraseSize);
            if(ret) {
                free(max_block_buf);
                return ret;
            }
            ret = nor_flash_write(partInfo,offset,max_block_buf,partInfo->eraseSize);
            free(max_block_buf);
            return ret;
        }
    }

    free(max_block_buf);
    return ret;
}
int mt_unf_flash_get_fhandle(MT_HANDLE DevHandle,int *fhandle)
{
    MTD_Part_Info * mtd_part_info_ptr = NULL;

    pthread_mutex_lock(&(mtd_part_info_head.mt_info_lock));
    mtd_part_info_ptr = mtd_part_info_head.next;
    while(mtd_part_info_ptr != NULL) {
        if ((MT_HANDLE)mtd_part_info_ptr == DevHandle) {
            pthread_mutex_unlock(&(mtd_part_info_head.mt_info_lock));
            return MT_SUCCESS;
        }
        mtd_part_info_ptr = mtd_part_info_ptr->next;
    }
    *fhandle=mtd_part_info_ptr->handle;
    pthread_mutex_unlock(&(mtd_part_info_head.mt_info_lock));

    return MT_FAILURE;
}
MT_S32 mt_unf_flash_init(mt_void)
{
    MT_S32 ret = 0;
    if (mtd_part_info_head.intialized) {
        return ERROR_CODE_ERROR_ALREADY_INITIALIZED;
    }
    memset(&mtd_part_info_head,0,sizeof(MTD_Part_Info_Header));
    mtd_part_info_head.next = NULL;
    mtd_part_info_head.magic1 = 0xaa55aa55;
    mtd_part_info_head.magic2 = 0xaa55aa56;
    ret = pthread_mutex_init(&(mtd_part_info_head.mt_info_lock), NULL);
    ret|= pthread_mutex_init(&(mtd_part_info_head.WRMutex), NULL);
    mtd_part_info_head.intialized = 1;

    mt_flash_get_mtdpartoffset(mtd_part_info_head.part_offset,&mtd_part_info_head.chipsize);
    if(-1 != flash_all_id) {
        flash_all_id=mt_flash_get_lastblockid();
    }
    mtd_part_info_head.type=0xffffffff;
    //printf("++allid=%d\n",flash_all_id);
    return ret;
}

MT_S32 mt_unf_flash_deinit(mt_void)
{
    MTD_Part_Info * mtd_part_info_ptr_deleted = NULL;
    MTD_Part_Info * mtd_part_info_ptr = NULL;

    pthread_mutex_lock(&mtd_part_info_head.mt_info_lock);
    if((0xaa55aa55 != mtd_part_info_head.magic1) || (0xaa55aa56 != mtd_part_info_head.magic2)) {
        MT_ERR_FLASH("++err.%s,%d\n",__FUNCTION__,__LINE__);
    }
    mtd_part_info_ptr = mtd_part_info_head.next;
    while(mtd_part_info_ptr != NULL) {
        mtd_part_info_ptr_deleted = mtd_part_info_ptr;
        if((0xaa55aa55 != mtd_part_info_ptr->magic1) || (0xaa55aa56 != mtd_part_info_ptr->magic2)) {
            MT_ERR_FLASH("++err.%s,%d\n",__FUNCTION__,__LINE__);
        }
        mtd_part_info_ptr = mtd_part_info_ptr->next;
        close(mtd_part_info_ptr_deleted->handle);
        free(mtd_part_info_ptr_deleted);
    }
    pthread_mutex_unlock(&mtd_part_info_head.mt_info_lock);
    pthread_mutex_destroy(&mtd_part_info_head.mt_info_lock);
    pthread_mutex_destroy(&mtd_part_info_head.WRMutex);
    mtd_part_info_head.intialized = 0;
    mtd_part_info_head.type=0xffffffff;
    return MT_SUCCESS;
}

MT_S32 mt_unf_flash_open(const MT_CHAR* pMtdDevFile, MT_HANDLE* pMtdDevHandleOut)
{
    MT_S32 id=0,ret = ERROR_CODE_NO_ERROR;
    MTD_Part_Info * new_part_info_ptr = NULL;

    pthread_mutex_lock(&mtd_part_info_head.mt_info_lock);
    new_part_info_ptr = (MTD_Part_Info *)malloc(sizeof(MTD_Part_Info));
    if (!new_part_info_ptr) {
        pthread_mutex_unlock(&mtd_part_info_head.mt_info_lock);
        return ERROR_CODE_ERROR_NOBUFF;
    }
    memset(new_part_info_ptr,0,sizeof(MTD_Part_Info));
    new_part_info_ptr->salvage_end_addr = 0;
    new_part_info_ptr->handle = open(pMtdDevFile, O_SYNC | O_RDWR|O_CLOEXEC);

    if (new_part_info_ptr->handle < 0) {
        free(new_part_info_ptr);
        pthread_mutex_unlock(&mtd_part_info_head.mt_info_lock);
        return ERROR_CODE_ERROR_NODEV;
    }

    ret = flash_init(new_part_info_ptr);

    if (ret != 0) {
        close(new_part_info_ptr->handle);
        free(new_part_info_ptr);
        pthread_mutex_unlock(&mtd_part_info_head.mt_info_lock);
        return ERROR_CODE_ERROR_DEV_OPEN_FAILURE;
    }

    id=mt_flash_get_lastblockidbyname((MT_CHAR*)pMtdDevFile);
    //printf("fid=%d\n",id);
    if(id==flash_all_id) {
        new_part_info_ptr->flag_lastblkid=1;
        //printf("lstid\n");
    } else {
        new_part_info_ptr->flag_lastblkid=0;
        //printf("nrmid\n");
    }
    new_part_info_ptr->partid=id;
    new_part_info_ptr->magic1 = 0xaa55aa55;
    new_part_info_ptr->magic2 = 0xaa55aa56;
    if((0xaa55aa55 != mtd_part_info_head.magic1) || (0xaa55aa56 != mtd_part_info_head.magic2)) {
        MT_ERR_FLASH("++err.%s,%d\n",__FUNCTION__,__LINE__);
    }
    new_part_info_ptr->next = mtd_part_info_head.next;
    mtd_part_info_head.next = new_part_info_ptr;

    *pMtdDevHandleOut = (MT_HANDLE)new_part_info_ptr;
    //printf("type0=%x\n",mtd_part_info_head.type);
    if((new_part_info_ptr->type == MTD_NANDFLASH) && (0xffffffff==mtd_part_info_head.type)) {
        MT_U8 status=0;
        spi_prot_block_type_t type;
        mtd_flash_getstatus(new_part_info_ptr->handle, &status, 1);
        //printf("status=%x\n",status);
        ret=mt_snf_getlockrang(new_part_info_ptr, status, &type);
        if(MT_SUCCESS == ret) {
            mtd_part_info_head.type=type;
            //printf("type1=%x\n",mtd_part_info_head.type);
        }
    }
    //printf("type2=%x\n",mtd_part_info_head.type);
    pthread_mutex_unlock(&mtd_part_info_head.mt_info_lock);

    return MT_SUCCESS;
}

MT_S32 mt_unf_flash_close(MT_HANDLE MtdDevHandleOut)
{
    MT_S32 ret = MT_SUCCESS;
    MTD_Part_Info * mtd_part_info_ptr = NULL;
    MTD_Part_Info * mtd_part_info_ptr_lst = NULL;

    pthread_mutex_lock(&mtd_part_info_head.mt_info_lock);
    if((0xaa55aa55 != mtd_part_info_head.magic1) || (0xaa55aa56 != mtd_part_info_head.magic2)) {
        MT_ERR_FLASH("++err.%s,%d\n",__FUNCTION__,__LINE__);
    }
    mtd_part_info_ptr = mtd_part_info_head.next;

    while(mtd_part_info_ptr != NULL) {
        if ((MT_HANDLE)mtd_part_info_ptr == MtdDevHandleOut) {
            if(mtd_part_info_ptr->handle_using_cnt) {
                pthread_mutex_unlock(&(mtd_part_info_head.mt_info_lock));
                MT_ERR_FLASH("++can't close cnt=%x\n",mtd_part_info_ptr->handle_using_cnt);
                return ERROR_CODE_ERROR_TIMEOUT;
            }
            if(mtd_part_info_head.next == mtd_part_info_ptr) {
                mtd_part_info_head.next = mtd_part_info_ptr->next;
            } else {
                mtd_part_info_ptr_lst->next = mtd_part_info_ptr->next;
            }
            close(mtd_part_info_ptr->handle);
            free(mtd_part_info_ptr);
            break;
        }
        mtd_part_info_ptr_lst = mtd_part_info_ptr;
        if((0xaa55aa55 != mtd_part_info_ptr->magic1) || (0xaa55aa56 != mtd_part_info_ptr->magic2)) {
            MT_ERR_FLASH("++err.%s,%d\n",__FUNCTION__,__LINE__);
        }
        mtd_part_info_ptr = mtd_part_info_ptr->next;
    }
    pthread_mutex_unlock(&mtd_part_info_head.mt_info_lock);
    return ret;
}

MT_S32 mt_unf_flash_read (MT_HANDLE DevHandle,MT_U32 u32Addr, MT_U8 *pu8Buf, MT_U32 u32Length)
{
    MT_S32 ret=MT_FAILURE;
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(DevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    if (mtd_part_info_ptr->type == MTD_NANDFLASH) {
        ret=mtd_nand_flash_read(mtd_part_info_ptr, u32Addr, pu8Buf, u32Length);
    } else if (mtd_part_info_ptr->type == MTD_NORFLASH) {
        ret=nor_flash_read(mtd_part_info_ptr, u32Addr, pu8Buf, u32Length);
    } else {
        ret=MT_FAILURE;
    }
    ptr_unused_minus(mtd_part_info_ptr);
    return ret;
}

MT_S32 mt_unf_flash_write(MT_HANDLE DevHandle,MT_U32 u32Addr, MT_U8 *pu8Buf, MT_U32 u32Length)
{
    MT_S32 ret = MT_FAILURE;
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(DevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    if (mtd_part_info_ptr->type == MTD_NANDFLASH) {
        mtd_part_info_ptr->salvage_end_addr = 0;
        ret = mt_snf_check_ew(mtd_part_info_ptr,u32Addr,u32Length);
        if(MT_FAILURE==ret) {
            ptr_unused_minus(mtd_part_info_ptr);
            return MT_FAILURE;
        }
        ret = mtd_nand_flash_write(mtd_part_info_ptr, u32Addr, pu8Buf, u32Length);
    } else if (mtd_part_info_ptr->type == MTD_NORFLASH) {
        ret = mtd_nor_flash_write(mtd_part_info_ptr, u32Addr, pu8Buf, u32Length);
    } else {
        ret = MT_FAILURE;
    }
    ptr_unused_minus(mtd_part_info_ptr);
    return ret;
}

MT_S32 mt_unf_flash_write_only(MT_HANDLE DevHandle,MT_U32 u32Addr, MT_U8 *pu8Buf, MT_U32 u32Length)
{
    MT_S32 ret = MT_FAILURE;
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(DevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    if (mtd_part_info_ptr->type == MTD_NANDFLASH) {
        ;//return MT_FAILURE;
    } else if (mtd_part_info_ptr->type == MTD_NORFLASH) {
        ret = nor_flash_write(mtd_part_info_ptr, u32Addr, pu8Buf, u32Length);
    } else {
        ;//return MT_FAILURE;
    }
    ptr_unused_minus(mtd_part_info_ptr);
    return ret;
}

MT_S32 mt_unf_flash_write_ext(MT_HANDLE DevHandle,MT_U32 startAddr, MT_U8 *pu8Buf, MT_U32 u32Length,MT_U32 endAddr)
{
    MT_S32 ret = MT_FAILURE;
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(DevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    if (mtd_part_info_ptr->type == MTD_NANDFLASH) {
        if (endAddr && (endAddr < mtd_part_info_ptr->size)) {
            mtd_part_info_ptr->salvage_end_addr = endAddr;
        } else {
            mtd_part_info_ptr->salvage_end_addr = 0;
        }
        ret = mt_snf_check_ew(mtd_part_info_ptr,startAddr,u32Length);
        if(MT_FAILURE==ret) {
            ptr_unused_minus(mtd_part_info_ptr);
            return MT_FAILURE;
        }
        ret = mtd_nand_flash_write(mtd_part_info_ptr, startAddr, pu8Buf, u32Length);
    } else if (mtd_part_info_ptr->type == MTD_NORFLASH) {
        ret = nor_flash_write(mtd_part_info_ptr, startAddr, pu8Buf, u32Length);
    } else {
        ret = MT_FAILURE;
    }
    ptr_unused_minus(mtd_part_info_ptr);
    return ret;
}

MT_S32 mt_unf_flash_write_raw_ext(MT_HANDLE DevHandle,MT_U32 startAddr, MT_U8 *pu8Buf, MT_U32 u32Length,MT_U32 endAddr)
{
    MT_S32 ret = MT_FAILURE;
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(DevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    if (mtd_part_info_ptr->type == MTD_NANDFLASH) {
        if (endAddr && (endAddr < mtd_part_info_ptr->size)) {
            mtd_part_info_ptr->salvage_end_addr = endAddr;
        } else {
            mtd_part_info_ptr->salvage_end_addr = 0;
        }
        ret = mt_snf_check_ew(mtd_part_info_ptr,startAddr,u32Length);
        if(MT_FAILURE==ret) {
            ptr_unused_minus(mtd_part_info_ptr);
            return MT_FAILURE;
        }
        ret = mtd_nand_flash_write_raw(mtd_part_info_ptr, startAddr, pu8Buf, u32Length);
    } else if (mtd_part_info_ptr->type == MTD_NORFLASH) {
        ret = nor_flash_write(mtd_part_info_ptr, startAddr, pu8Buf, u32Length);
    } else {
        ret = MT_FAILURE;
    }
    ptr_unused_minus(mtd_part_info_ptr);
    return ret;
}

MT_S32 mt_unf_flash_get_avalen(MT_HANDLE MtdDevHandle,MT_U32 *FlashAvaLength)
{
    MT_U64      offset = 0;
    int         bad_block = 0;
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    if(mtd_part_info_ptr->type == MTD_NANDFLASH) {
        mtd_part_info_ptr->availableSize = 0;
        for (offset = 0; offset<mtd_part_info_ptr->size; offset+=mtd_part_info_ptr->eraseSize) {
            if ((bad_block = ioctl(mtd_part_info_ptr->handle,MEMGETBADBLOCK,&offset)) < 0) {
                ptr_unused_minus(mtd_part_info_ptr);
                return MT_FAILURE;
            }

            if (bad_block == 0) {
                mtd_part_info_ptr->availableSize += mtd_part_info_ptr->eraseSize;
            } else {
                MT_INFO_FLASH("flash addr offset:%llx is bad block\n",offset);
            }
        }
    }

    *FlashAvaLength = mtd_part_info_ptr->availableSize;
    ptr_unused_minus(mtd_part_info_ptr);

    return MT_SUCCESS;
}

MT_S32 mt_unf_flash_erase(MT_HANDLE MtdDevHandle,MT_U32 u32Addr,MT_U32 u32Length)
{
    MT_U64      offset = 0,prev_offset = 0;
    MT_S32 ret = 0,len = 0;
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    if( (u32Addr&(mtd_part_info_ptr->eraseSize-1)) || (u32Length&(mtd_part_info_ptr->eraseSize-1))) {
        ptr_unused_minus(mtd_part_info_ptr);
        MT_ERR_FLASH("LLD_FLASH_Erase: erase start address and erase len should be block-aligned!\n");
        return MT_FLASH_ERR_PARAM;
    }
    if((u32Addr + u32Length)> mtd_part_info_ptr->size) {
        ptr_unused_minus(mtd_part_info_ptr);
        MT_ERR_FLASH("erase area beyond partion size\n");
        return MT_FLASH_ERR_PARAM;
    }
    if(0==mtd_part_info_ptr->flag_lastblkid) {
        if(mtd_part_info_ptr->type == MTD_NANDFLASH) { //jump bad
            u32 blkid=0,rawid=0;
            u32 tailid=mtd_part_info_ptr->size/mtd_part_info_ptr->eraseSize;
            u32 start_block = (u32Addr/mtd_part_info_ptr->eraseSize);
            //printf("jump erase\n");
            if(0==tailid) {
                ptr_unused_minus(mtd_part_info_ptr);
                return MT_FAILURE;
            }
            tailid--;
            for(blkid=0; blkid<start_block;) {
                if(flash_isgoodblock(mtd_part_info_ptr,rawid)) {
                    blkid++;
                }
                if(rawid==tailid) {
                    break;
                }
                rawid++;
            }
            u32Addr=(rawid*mtd_part_info_ptr->eraseSize);
        }
    }

    if(mtd_part_info_ptr->type == MTD_NANDFLASH) {
        offset = u32Addr;
        prev_offset = u32Addr;
        len = (int)u32Length;
        ret = mt_snf_check_ew(mtd_part_info_ptr,u32Addr,u32Length);
        if(MT_FAILURE==ret) {
            ptr_unused_minus(mtd_part_info_ptr);
            return MT_FAILURE;
        }
        while(len>0) {
            ret = seek_to_good_block(mtd_part_info_ptr->handle,mtd_part_info_ptr->size,mtd_part_info_ptr->eraseSize,&offset);
            if (ret != MT_SUCCESS) {
                MT_ERR_FLASH("seek_to_good_block error :ret = %d u32Addr:%llx\n",ret,offset);
                ptr_unused_minus(mtd_part_info_ptr);
                return MT_FAILURE;
            }
            if(1==mtd_part_info_ptr->flag_lastblkid) {
#ifndef JUMP_ALL_BAD_BLOCK
                if ((offset - prev_offset) > 0) {
                    len -= (MT_S32)((offset - prev_offset) + mtd_part_info_ptr->eraseSize);
                } else {
                    len -= (MT_S32)(mtd_part_info_ptr->eraseSize);
                }
                if (len <0) {
                    ptr_unused_minus(mtd_part_info_ptr);
                    return MT_SUCCESS;
                }
#endif
            }
            ret = nand_erase_an_block(mtd_part_info_ptr->handle,offset,mtd_part_info_ptr->eraseSize);
            if(ret == MT_FLASH_ERR_IO) {
                nand_mark_bad_block(mtd_part_info_ptr->handle, offset);
                continue;
            } else if(ret != 0) {
                ptr_unused_minus(mtd_part_info_ptr);
                return MT_FAILURE;
            }
            if(1==mtd_part_info_ptr->flag_lastblkid) {
#ifndef JUMP_ALL_BAD_BLOCK
                offset += mtd_part_info_ptr->eraseSize;
                prev_offset = offset;
#else
                len -= (MT_S32)mtd_part_info_ptr->eraseSize;
                offset += mtd_part_info_ptr->eraseSize;
#endif
            } else {
                len -= (MT_S32)mtd_part_info_ptr->eraseSize;
                offset += mtd_part_info_ptr->eraseSize;
            }
        }
    } else {
        while(u32Length) {
            ret = nor_flash_erase(mtd_part_info_ptr->handle,u32Addr,mtd_part_info_ptr->eraseSize);
            if (ret != MT_SUCCESS) {
                ptr_unused_minus(mtd_part_info_ptr);
                return MT_FAILURE;
            }
            u32Length -= mtd_part_info_ptr->eraseSize;
            u32Addr += mtd_part_info_ptr->eraseSize;
        }
    }
    ptr_unused_minus(mtd_part_info_ptr);

    return MT_SUCCESS;
}

MT_S32 mt_unf_flash_getlockcfg(MT_HANDLE MtdDevHandle ,MT_U32 *puBuf,MT_U8 *u8Length,spi_prot_block_type_t type,MT_U32 *pro_mask)
{
    MT_S32 ret = 0;
    struct spi_flash_id id_info;
    unsigned int count = 0;
    snf_protect_info_t  *snf_info= NULL;
    snf_protect_info_t  *snf_info_end= NULL;
    unsigned char mid = 0;
    unsigned short did = 0;
    MTD_Part_Info * mtd_part_info_ptr = NULL;

    if((NULL == puBuf) || (NULL == u8Length)|| (NULL == pro_mask)) {
        return MT_FLASH_ERR_PARAM;
    }

    mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    ret = ioctl(mtd_part_info_ptr->handle, MEMGETFLASHID, &id_info);
    if(0 == ret) {
        mid = (id_info.mid & 0xFF);
        did = id_info.did & 0xFFFF;
    }

    if(mtd_part_info_ptr->type==MTD_NORFLASH) {
        if(FLASH_MF_GD==mid) {
            did=0;
        } else if(mid == FLASH_MF_WINB) {
            did = 0xFFFF;
        }
        snf_info = s_flash_prot_t2;	//for clear warnning
        snf_info = s_flash_prot_t;
        count = sizeof(s_flash_prot_t) / sizeof(snf_protect_info_t);
        snf_info_end = s_flash_prot_t + count;
    } else if(mtd_part_info_ptr->type==MTD_NANDFLASH) {
        if((0xef==mid)||(0xea==mid)) {	//3-valid-addr
            ;
        } else {												//2-valid-addr
            did &= 0xFF00;
        }
        snf_info = snand_flash_prot_t;
        count = sizeof(snand_flash_prot_t) / sizeof(snf_protect_info_t);
        snf_info_end = snand_flash_prot_t + count;
    }

    for((snf_info=(mtd_part_info_ptr->type==MTD_NORFLASH)?s_flash_prot_t: snand_flash_prot_t); snf_info < snf_info_end; snf_info++) {
        if((snf_info->m_id == mid) &&
           ((0x00==snf_info->d_id) || (snf_info->d_id == did)) &&
           (snf_info->prot_st.prt_t == type)) {
            *puBuf = snf_info->prot_st.st;
            *u8Length = snf_info->prot_st.len;
            *pro_mask = snf_info->prot_st.st_mask;
            ptr_unused_minus(mtd_part_info_ptr);
            return MT_SUCCESS;
        }
    }
    ptr_unused_minus(mtd_part_info_ptr);

    return MT_FAILURE;
}

MT_S32 mt_unf_flash_getstatus(MT_HANDLE MtdDevHandle,MT_U8 *pu8Buf,MT_U8 u8Length)
{
    MT_S32 ret = MT_SUCCESS;
    MTD_Part_Info * mtd_part_info_ptr = NULL;

    if(NULL == pu8Buf) {
        return MT_FLASH_ERR_PARAM;
    }

    mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    ret = mtd_flash_getstatus(mtd_part_info_ptr->handle, pu8Buf, u8Length);
    ptr_unused_minus(mtd_part_info_ptr);

    return ret;
}

MT_S32 mt_unf_flash_setstatus(MT_HANDLE MtdDevHandle,MT_U8 *pu8Buf,MT_U8 u8Length)
{
    MT_S32 ret = MT_SUCCESS;
    MTD_Part_Info *mtd_part_info_ptr = NULL;

    if(NULL == pu8Buf) {
        return MT_FLASH_ERR_PARAM;
    }

    mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    ret = mtd_flash_setstatus(mtd_part_info_ptr->handle, pu8Buf, u8Length);
    ptr_unused_minus(mtd_part_info_ptr);

    return ret;
}

MT_S32 mt_unf_flash_setprotect(MT_HANDLE MtdDevHandle,spi_prot_block_type_t type)
{
    MT_U32  value = 0;
    MT_U32 mask = 0;
    MT_S32 ret = 0;
    MT_U8 len = 0;
    MT_U8 status[4] = {0};

    ret = mt_unf_flash_getlockcfg(MtdDevHandle, &value, &len, type, &mask);
    if(ret < 0) {
        return MT_FAILURE;
    }

    value &=mask;
    status[0] = (MT_U8)(value & 0xff);
    status[1] = (MT_U8)((value >> 8) & 0xff);
    ret = mt_unf_flash_setstatus(MtdDevHandle,status,len);
    if(ret < 0) {
        return MT_FAILURE;
    }
	mtd_part_info_head.type=type;
    return MT_SUCCESS;
}

MT_S32 mt_unf_flash_info(MT_HANDLE MtdDevHandle,struct mtd_info_user *p_info)
{
    MT_S32 ret = 0;

    MTD_Part_Info * mtd_part_info_ptr = NULL;

    if(NULL == p_info) {
        return MT_FLASH_ERR_PARAM;
    }
    mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }
    ret = ioctl(mtd_part_info_ptr->handle, MEMGETINFO, p_info);
    ptr_unused_minus(mtd_part_info_ptr);

    return ret;
}

MT_S32 mt_unf_flash_id(MT_HANDLE MtdDevHandle,MT_U32 *id)
{
    MT_S32 ret = 0;
    struct spi_flash_id id_info;
    MTD_Part_Info * mtd_part_info_ptr = NULL;

    if(NULL == id) {
        return MT_FLASH_ERR_PARAM;
    }

    mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    ret = ioctl(mtd_part_info_ptr->handle, MEMGETFLASHID, &id_info);
    if(0 == ret) {
        *id = (MT_U32)((id_info.mid << 16) | (id_info.did & 0xFFFF)) ;
    }
    ptr_unused_minus(mtd_part_info_ptr);

    return ret;
}

MT_S32 mt_unf_flash_id_uni(MT_HANDLE MtdDevHandle,MT_U8 *id_uni,MT_U32 len)
{
    MT_S32 ret = 0;
    struct spi_flash_id_uni id_info;
    MTD_Part_Info * mtd_part_info_ptr = NULL;

    if(NULL == id_uni) {
        return MT_FLASH_ERR_PARAM;
    }

    mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    id_info.len=len;
    id_info.code_uni=(ulong)id_uni;

    ret = ioctl(mtd_part_info_ptr->handle, MEMOTPFLASHUNIID, &id_info);
    ptr_unused_minus(mtd_part_info_ptr);

    return ret;
}

MT_S32 mt_unf_flash_otp_write(MT_HANDLE MtdDevHandle,MT_U32 offset,MT_U32 len, u_char *buf)
{
    //if nor: ignore base. offset=n*valid_size; if spinand: offset=2+n*pagesize
    //if spinand: read a page onetime. len<=pagesize.	offset according to datasheet.
    MT_S32 ret = 0;
    MTD_Part_Info * mtd_part_info_ptr = NULL;
    MT_U32 otp_mode=MTD_OTP_USER;
    off_t f_ops_rt=0;
    ssize_t f_ops_rw=0;

    mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    pthread_mutex_lock(&mtd_part_info_head.WRMutex);
    otp_mode=MTD_OTP_USER;
    ioctl(mtd_part_info_ptr->handle,OTPSELECT, &otp_mode);

    f_ops_rt=lseek(mtd_part_info_ptr->handle, (off_t)offset, SEEK_SET);
    if(f_ops_rt < 0) {
        ret = ERROR_CODE_ERROR_RESULT;
    } else {
        f_ops_rw = write(mtd_part_info_ptr->handle,buf,len);
        if((MT_U32)f_ops_rw == (MT_U32)len) {
            ret = ERROR_CODE_NO_ERROR;
        } else {
            ret = ERROR_CODE_ERROR_RESULT;
        }
    }

    otp_mode=MTD_OTP_OFF;
    ioctl(mtd_part_info_ptr->handle, OTPSELECT, &otp_mode);
    pthread_mutex_unlock(&mtd_part_info_head.WRMutex);
    ptr_unused_minus(mtd_part_info_ptr);

    return ret;
}

MT_S32 mt_unf_flash_otp_read(MT_HANDLE MtdDevHandle,MT_U32 offset,MT_U32 len, u_char *buf)
{
    //if nor: ignore base. offset=n*valid_size; if spinand: offset=2+n*pagesize
    //if spinand: read a page onetime. len<=pagesize.	offset according to datasheet.
    MT_S32 ret = 0;
    MTD_Part_Info * mtd_part_info_ptr = NULL;
    MT_U32 otp_mode=MTD_OTP_USER;
    off_t f_ops_rt=0;
    ssize_t f_ops_rw=0;

    mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    pthread_mutex_lock(&mtd_part_info_head.WRMutex);
    otp_mode=MTD_OTP_USER;
    ioctl(mtd_part_info_ptr->handle,OTPSELECT, &otp_mode);

    f_ops_rt = lseek(mtd_part_info_ptr->handle, (off_t)offset, SEEK_SET);
    if(f_ops_rt < 0) {
        ret = ERROR_CODE_ERROR_RESULT;
    } else {
        f_ops_rw = read(mtd_part_info_ptr->handle,buf,len);
        if((MT_U32)f_ops_rw == (MT_U32)len) {
            ret = ERROR_CODE_NO_ERROR;
        } else {
            ret = ERROR_CODE_ERROR_RESULT;
        }
    }

    otp_mode=MTD_OTP_OFF;
    ioctl(mtd_part_info_ptr->handle,OTPSELECT, &otp_mode);
    pthread_mutex_unlock(&mtd_part_info_head.WRMutex);
    ptr_unused_minus(mtd_part_info_ptr);

    return ret;
}

MT_S32 mt_unf_flash_otp_erase(MT_HANDLE MtdDevHandle,MT_U32 offset)
{
    //if nor: ignore base. offset=n*valid_size; if spinand: nofunc
    //if spinand: only write 1 to 0,can't erase to 1.
    MT_S32 ret = MT_SUCCESS;
    struct otp_info otp_info[16]= {0};
    struct otp_info otp_erase = {0};
    MTD_Part_Info * mtd_part_info_ptr = NULL;
    MT_U32 otp_mode=MTD_OTP_USER;

    mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    pthread_mutex_lock(&mtd_part_info_head.WRMutex);
    if(mtd_part_info_ptr->type==MTD_NORFLASH) {
        otp_mode=MTD_OTP_USER;
        ioctl(mtd_part_info_ptr->handle,OTPSELECT, &otp_mode);

        ret = ioctl(mtd_part_info_ptr->handle, OTPGETREGIONINFO, otp_info);
        if(0==ret) {
            otp_erase.start=(offset/otp_info[0].length)*otp_info[0].length;
            otp_erase.length=otp_info[0].length;

            ret = ioctl(mtd_part_info_ptr->handle, OTPERASE, &otp_erase);
        }

        otp_mode=MTD_OTP_OFF;
        ioctl(mtd_part_info_ptr->handle,OTPSELECT, &otp_mode);
    }
    pthread_mutex_unlock(&mtd_part_info_head.WRMutex);
    ptr_unused_minus(mtd_part_info_ptr);

    return ret;
}

MT_S32 mt_unf_flash_otp_lock(MT_HANDLE MtdDevHandle,MT_U32 offset)
{
    //if nor: ignore base. offset=n*valid_size; if spinand: offset=0
    //if spinand: lock all pages.
    MT_S32 ret = 0;
    MT_U32 otp_mode=MTD_OTP_USER;
    struct otp_info otp_info[16]= {{0}};
    struct otp_info otplock= {0};
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    pthread_mutex_lock(&mtd_part_info_head.WRMutex);
    otp_mode=MTD_OTP_USER;
    ioctl(mtd_part_info_ptr->handle,OTPSELECT, &otp_mode);

    if(mtd_part_info_ptr->type == MTD_NORFLASH) {				//lock region
        ret = ioctl(mtd_part_info_ptr->handle, OTPGETREGIONINFO, otp_info);
        if(0==ret) {
            otplock.start=(offset/otp_info[0].length)*otp_info[0].length;
            otplock.length=otp_info[0].length;
            otplock.locked=1;
        }
    } else if(mtd_part_info_ptr->type == MTD_NANDFLASH) {	//lock all
        otplock.start=(MT_U32)offset;	//ignore me
        otplock.length=0x800; 				//can't set 0
        otplock.locked=1;
    }
    //printf("otp start,lenght:%x,%x\n",otplock.start,otplock.length);

    ret = ioctl(mtd_part_info_ptr->handle, OTPLOCK, &otplock);

    otp_mode=MTD_OTP_OFF;
    ioctl(mtd_part_info_ptr->handle,OTPSELECT, &otp_mode);
    pthread_mutex_unlock(&mtd_part_info_head.WRMutex);
    ptr_unused_minus(mtd_part_info_ptr);

    return ret;
}

MT_S32 mt_unf_flash_otp_lock_getinfo(MT_HANDLE MtdDevHandle,struct otp_info *oinfo,MT_U32 max_cnt,MT_U32 *valid_cnt)
{
    //if nor: get regions and lock info
    //if spinand:get lock info,pages according to datasheet
    MT_S32 ret = 0;
    MT_U32 i=0,tmp_cnt=0,otp_mode=MTD_OTP_USER;
    struct otp_info tmp_info[16]= {{0}};
    MTD_Part_Info * mtd_part_info_ptr = NULL;

    if(NULL==valid_cnt) {
        return -1;
    }
    *valid_cnt=0;
    mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    pthread_mutex_lock(&mtd_part_info_head.WRMutex);
    otp_mode=MTD_OTP_USER;
    ioctl(mtd_part_info_ptr->handle,OTPSELECT, &otp_mode);

    ret = ioctl(mtd_part_info_ptr->handle, OTPGETREGIONINFO, tmp_info);
    if(0==ret) {
        for(i=0; i<16; i++) {
            if(0==tmp_info[i].length) {
                break;
            }
            tmp_cnt++;
        }
        if(tmp_cnt>max_cnt) {
            tmp_cnt=max_cnt;
        }
        memcpy(oinfo,tmp_info,tmp_cnt*sizeof(struct otp_info));
        *valid_cnt=tmp_cnt;
    }

    otp_mode=MTD_OTP_OFF;
    ioctl(mtd_part_info_ptr->handle,OTPSELECT, &otp_mode);
    pthread_mutex_unlock(&mtd_part_info_head.WRMutex);
    ptr_unused_minus(mtd_part_info_ptr);
    return ret;
}

MT_BOOL mt_unf_flash_isbadblock(MT_HANDLE MtdDevHandle,MT_U64 offset)
{
    int ret = -1;
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FALSE;
    }

    if( (offset&(mtd_part_info_ptr->eraseSize-1))) {
        MT_ERR_FLASH("mt_unf_flash_isbadblock:address  should be block-aligned!\n");
        ptr_unused_minus(mtd_part_info_ptr);
        return MT_FALSE;
    }

    if(mtd_part_info_ptr->type == MTD_NANDFLASH) {
        ret = ioctl(mtd_part_info_ptr->handle, MEMGETBADBLOCK, &offset);
        if(ret) {
            ptr_unused_minus(mtd_part_info_ptr);
            return MT_TRUE;
        }
    }
    ptr_unused_minus(mtd_part_info_ptr);
    return MT_FALSE;
}

MT_S32 mt_unf_flash_mark_badblock(MT_HANDLE MtdDevHandle,MT_U64 offset)
{
    __kernel_loff_t ioctrl_offset=(__kernel_loff_t)offset;
    int ret = -1;
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return MT_FAILURE;
    }

    if ( (ioctrl_offset&(mtd_part_info_ptr->eraseSize-1)) ) {
        MT_ERR_FLASH("mt_unf_flash_isbadblock:address	should be block-aligned!\n");
        ptr_unused_minus(mtd_part_info_ptr);
        return MT_FAILURE;
    }

    if (mtd_part_info_ptr->type == MTD_NANDFLASH) {
        ret = ioctl(mtd_part_info_ptr->handle, MEMSETBADBLOCK, &ioctrl_offset);
        ptr_unused_minus(mtd_part_info_ptr);
        if (ret == 0) {
            return MT_SUCCESS;
        } else {
            return MT_FAILURE;
        }
    }
    ptr_unused_minus(mtd_part_info_ptr);
    return MT_FAILURE;
}

MT_VOID mt_unf_flash_setcallback(MT_HANDLE MtdDevHandle,flash_msg_callback pcallback)
{
    MTD_Part_Info * mtd_part_info_ptr = ptr_find_and_plus(MtdDevHandle);
    if (NULL == mtd_part_info_ptr) {
        return ;
    }

    mtd_part_info_ptr->pcallback = pcallback;
    ptr_unused_minus(mtd_part_info_ptr);
}

