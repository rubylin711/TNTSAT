/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "mt_flash.h"
#include "nand.h"

/*****************************************************************************/
/* change length to string */
char *int_to_size(unsigned long long size)
{
    int ix;
    static char buffer[20];

    for (ix = 0; (ix < 5) && !(size & 0x3FF) && size; ix++)
    {
        size = (size >> 10);
    }
    if (ix == 0) snprintf(buffer, 20, "%u", (unsigned int)size);
    else if (ix == 1) snprintf(buffer, 20, "%uK", (unsigned int)size);
    else if (ix == 2) snprintf(buffer, 20, "%uM", (unsigned int)size);
    else if (ix == 3) snprintf(buffer, 20, "%uG", (unsigned int)size);
    else if (ix == 4) snprintf(buffer, 20, "%uT", (unsigned int)size);
    return buffer;
}
/*****************************************************************************/
/*
 * Modified for CONNAX CA.
 * When /dev path has no excute permission, stat call will fail.
 * Try parse /proc/mtd to get max partition number.
 */
int get_max_partition(void)
{
    FILE *fd;
#define MAX_PROC_LINE_SZ    1024
    char bf[MAX_PROC_LINE_SZ];
    int nr = 0;

    fd = fopen(PROC_MTD_FILE, "r");
    if (fd == NULL) {
        DBG_OUT("Fail to open %s!\n",PROC_MTD_FILE);
        return 0;
    }
    /* skip first prompt line */
    if (NULL == fgets(bf, MAX_PROC_LINE_SZ, fd)) {
        fclose(fd);
        return -1;
    }

    while (fgets(bf, MAX_PROC_LINE_SZ, fd))
            nr ++;

    fclose(fd);
    DBG_OUT("max partition nr %d\n", nr);
    //printf("max partition nr %d\n", nr);
    /* keep the way before do */
    return nr - 1;
}
/*****************************************************************************/

int offshift(unsigned long n)
{
    int shift = -1;
    while (n)
    {
        n = n >> 1;
        shift++;
    }
    return shift;
}
/*****************************************************************************/

static char* skip_space(char* line)
{
    char* p = line;
    while (*p == ' ' || *p == '\t')
    {
        p++;
    }
    return p;
}

static char* get_word(char* line, char* value)
{
    char* p = line;
    p = skip_space(p);
    while (*p != '\t' && *p != ' '  && *p != '\n' && *p != 0)
    {
        *value++ = *p++;
    }
    *value = 0;
    return p;
}

static int get_bootargs(char *p_bootargs, unsigned short len)
{
    FILE *pf;

    if( NULL == p_bootargs)
    {
        DBG_OUT("Pointer is null.\n");
        return -1;
    }

    if(NULL == (pf = fopen("/proc/cmdline", "r")))
    {
        DBG_OUT("Failed to open '/proc/cmdline'.\n");
        return -1;
    }

    if(NULL == fgets((char*)p_bootargs, len, pf))
    {
        DBG_OUT("Failed to fgets string.\n");
        fclose(pf);
        return -1;
    }

    fclose(pf);
    return 0;
}

static mt_flash_type_t get_flash_type_by_bootargs(const char *p_bootargs, char *p_partitionname)
{
    char *partition_pos = NULL;
    char *tmppos = NULL;
    char tmpstr[64];
    int i;
    mt_flash_type_t flash_type = MT_FLASH_TYPE_BUTT;
    char typestr[MT_FLASH_TYPE_BUTT][16]={
        "mt_sfc:",
        "mtnand:",
        "mmcblk0:"};
    char *pszType[MT_FLASH_TYPE_BUTT];

    if(NULL == p_partitionname)
    {
        return (MT_FLASH_TYPE_BUTT);
    }

    for(i=0; i<MT_FLASH_TYPE_BUTT; i++)
    {
        pszType[i] = strstr((const char *)p_bootargs, typestr[i]);
    }

    memset(tmpstr, 0, sizeof(tmpstr));
    (mt_void)snprintf(tmpstr, sizeof(tmpstr)-1, "(%s)", p_partitionname);
    partition_pos = strstr((const mt_char *)p_bootargs, tmpstr);
    if(NULL == partition_pos)
    {
        return (MT_FLASH_TYPE_BUTT);
    }

    for(i=0; i<MT_FLASH_TYPE_BUTT; i++)
    {
        if(NULL == pszType[i])
        {
            continue;
        }
        /* tmppos is used to be a cursor */
        /*lint -save -e613 */
        if((partition_pos >= pszType[i])&&(pszType[i] >= tmppos))
        {
            flash_type = (mt_flash_type_t) i;
            tmppos = pszType[i];
        }
    }
    return flash_type;
}

static  mt_flash_partinfo_s g_partinfo[MAX_MTD_PARTITION];

int flash_partition_info_init(void)
{
    int ret = -1;
    int i = 0;
    char bootargs[512];
    FILE* fp = MT_NULL;
    static int init_flag = -1;
    mt_flash_partinfo_s *p_partinfo = NULL;

    if(init_flag >=0)
        return 0;

    for(i=0; i< MAX_MTD_PARTITION; i++)
    {
        p_partinfo = &g_partinfo[i];
        p_partinfo->start_addr = 0;
        p_partinfo->partsize = 0;
        p_partinfo->blocksize = 0;
        p_partinfo->flash_type = MT_FLASH_TYPE_BUTT;
        p_partinfo->perm = ACCESS_BUTT;
        memset(p_partinfo->devname, '\0', FLASH_NAME_LEN);
        memset(p_partinfo->partname, '\0', FLASH_NAME_LEN);
    }

    ret = get_bootargs(bootargs, sizeof(bootargs)-1);
    if(ret != 0)
        return ret;

    fp = fopen("/proc/mtd", "r");
    if (fp)
    {
        unsigned long long start_addr[MT_FLASH_TYPE_BUTT];
        char line[512];
        if (NULL == fgets(line, sizeof(line), fp)) {//skip first line
            fclose(fp);
            return -1;
        }

//        DBG_OUT(" devname\t  partsize\tblocksize   partname    start_addr\n");
        for(i = 0; i < MT_FLASH_TYPE_BUTT; i++)
        {
            start_addr[i] = 0;
        }

        i = 0;
        while(i < MAX_MTD_PARTITION)
        {
            char argv[4][32];
            char *p;

            if(fgets(line, sizeof(line), fp) == NULL)
            {
                break;
            }

            p = &line[0];
            p = get_word(p, argv[0]);
            p = get_word(p, argv[1]);
            p = get_word(p, argv[2]);
            p = get_word(p, argv[3]);
            p = p; /* for no TQE warning */

            p_partinfo = &g_partinfo[i];
            p_partinfo->partsize = (mt_u64)strtoull(argv[1],NULL, 16);  //partion size
            p_partinfo->blocksize = (mt_u64)(mt_u32)strtol(argv[2], NULL,16); //erase size
            //FIXME: bad usage
            strncpy(p_partinfo->devname, argv[0], (strlen(argv[0])-1));
            //FIXME: bad usage
            strncpy(p_partinfo->partname, (argv[3]+1), (strlen(argv[3])-2));
            p_partinfo->flash_type = get_flash_type_by_bootargs(bootargs , p_partinfo->partname);
            p_partinfo->start_addr = start_addr[p_partinfo->flash_type];
            start_addr[p_partinfo->flash_type] += p_partinfo->partsize;

            i++;
        }

        fclose(fp);
    }
    else
    {
        return -1;
    }

    init_flag = 0;
    return 0;
}

mt_flash_partinfo_s * get_flash_partition_info(mt_flash_type_t flash_type, const char * devname)
{
    int i;
    mt_flash_partinfo_s *p_partinfo = NULL;

    if(NULL == devname)
        return NULL;

    for(i=0; i< MAX_MTD_PARTITION; i++)
    {
        p_partinfo = &g_partinfo[i];
        if(p_partinfo->flash_type != flash_type)
            continue;
        if(strncmp(p_partinfo->devname, devname,
        strlen(p_partinfo->devname) > strlen(devname) ?
            strlen(p_partinfo->devname) : strlen(devname)) == 0)
            return (p_partinfo);
    }
    return NULL;
}

unsigned long long get_flash_total_size(mt_flash_type_t flash_type)
{
    int i;
    unsigned long long totalsize=0;
    mt_flash_partinfo_s *p_partinfo = NULL;

    for(i=0; i< MAX_MTD_PARTITION; i++)
    {
        p_partinfo = &g_partinfo[i];
        if(p_partinfo->flash_type != flash_type)
            continue;
        totalsize += p_partinfo->partsize;
    }
    return totalsize;
}

