/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "mt_debug.h"
#include "mt_flash.h"
#include "nand.h"
#include "spi_raw.h"
#include "nand_raw.h"
#include "emmc_raw.h"
#include "mt_debug.h"
#include "flash_api_internal.h"

#define MAX_HANDLE        MAX_PARTS    /* Flash max handle number */
#define MAX_BOOTARGS_LEN  1024  /* The max length of bootargs */

/* Expand hiflash handle fd, only mt_flash_openby_type_and_name() use it */
#define  SPAN_PART_HANDLE 1000

#define MT_ERR_FLASH(fmt...) \
             MT_ERR_PRINT(MT_ID_FLASH, fmt)

#define MT_INFO_FLASH(fmt...) \
             MT_INFO_PRINT(MT_ID_FLASH, fmt)

typedef enum _mt_flash_dev_stat
{
    MT_FLASH_STAT_INSTALL,
    MT_FLASH_STAT_UNINSTALL,
    /*lint -save -e749 */
    MT_FLASH_STAT_BUTT
}mt_flash_dev_stat;

static mt_flash_interinfo_s g_flashinfo[MAX_HANDLE];
static mt_flash_partinfo_s g_partinfo[MAX_PARTS];
static MT_BOOL g_initflag = MT_FALSE;
static mt_u8 g_bootargs[MAX_BOOTARGS_LEN];
static mt_char g_flashstr[MT_FLASH_TYPE_BUTT][16]={
    "mt_sfc:",
    "mtnand:",
    "mmcblk0:"};
static mt_char *g_wrpos[MT_FLASH_TYPE_BUTT];
static mt_flash_dev_stat g_devstat[MT_FLASH_TYPE_BUTT];
static pthread_mutex_t g_flashmutex;

static flash_opt_s g_flashopt[MT_FLASH_TYPE_BUTT];

extern emmc_flash_s g_emmcflash;

#define CHECK_flash_init(p_flash) \
    do \
    { \
        if (!g_initflag) \
        { \
            MT_ERR_FLASH("Not init yet!\n"); \
            return MT_FAILURE; \
        } \
        if (MAX_HANDLE <= p_flash) \
        { \
            return MT_FAILURE; \
        } \
        if (INVALID_FD == g_flashinfo[p_flash].fd) \
        { \
            return MT_FAILURE; \
        } \
    } while (0)

#define CHECK_ADDR_LEN_VALID(addr, len, limit_len) \
    do \
    { \
        if ((addr >= limit_len) || ((addr + len) > limit_len)) \
        { \
            MT_ERR_FLASH("start_addr(0x%llX) + length(0x%x) or start_addr should be smaller than partsize(0x%llX)\n", addr, len, limit_len); \
            pthread_mutex_unlock(&g_flashmutex); \
            return MT_FAILURE; \
        } \
    } while (0)

static mt_s8* skip_space(mt_s8* line)
{
    mt_s8* p = line;
    while (*p == ' ' || *p == '\t')
    {
        p++;
    }
    return p;
}

static mt_s8* skip_word(mt_s8* line)
{
    mt_s8* p = line;
    while (*p != '\t' && *p != ' ' && *p != '\n' && *p != 0)
    {
        p++;
    }
    return p;
}

static mt_s8* get_word(mt_s8* line, mt_s8* value)
{
    mt_s8* p = line;
    p = skip_space(p);
    while (*p != '\t' && *p != ' ' && *p != '\n' && *p != 0)
    {
        *value++ = *p++;
    }
    *value = 0;
    return p;
}

static mt_s32 get_bootargs(mt_u8 *p_bootargs, mt_u16 len)
{
    FILE *pf;

    if( NULL == p_bootargs)
    {
        MT_ERR_FLASH("Pointer is null.\n");
        return MT_FAILURE;
    }

    if(NULL == (pf = fopen("/proc/cmdline", "r")))
    {
        MT_ERR_FLASH("Failed to open '/proc/cmdline'.\n");
        return MT_FAILURE;
    }

    if(NULL == fgets((char*)p_bootargs, len, pf))
    {
        MT_ERR_FLASH("Failed to fgets string.\n");
        fclose(pf);
        return MT_FAILURE;
    }

    fclose(pf);
    return MT_SUCCESS;
}

static mt_flash_type_t get_flash_type_by_bootargs(mt_char *partition_name)
{
    mt_char *p_partitionpos = MT_NULL;
    mt_char *p_tmppos = MT_NULL;
    mt_char p_tmpstr[64];
    mt_u32 i;
    mt_flash_type_t flash_type = MT_FLASH_TYPE_BUTT;

    if(MT_NULL == partition_name)
    {
        return (MT_FLASH_TYPE_BUTT);
    }
    memset(p_tmpstr, 0, sizeof(p_tmpstr));
    (mt_void)snprintf(p_tmpstr, sizeof(p_tmpstr)-1, "(%s)", partition_name);
    p_partitionpos = strstr((mt_char *)g_bootargs, p_tmpstr);
    if(MT_NULL == p_partitionpos)
    {
        return (MT_FLASH_TYPE_BUTT);
    }

    for(i=0; i<MT_FLASH_TYPE_BUTT; i++)
    {
        if(MT_NULL == g_wrpos[i])
        {
            continue;
        }
        /* p_tmppos is used to be a cursor */
        /*lint -save -e613 */
        if((p_partitionpos >= g_wrpos[i])&&(g_wrpos[i] >= p_tmppos))
        {
            flash_type = (mt_flash_type_t) i;
            p_tmppos = g_wrpos[i];
        }
    }
    return flash_type;
}

static mt_s32 permission_check(mt_flash_type_t flash_type,
                                     mt_u64 start_addr,
                                     mt_u64 len)
{
    mt_u64 end_addr = start_addr + len -1;
    mt_u32 i;

    for(i=0; i< MAX_PARTS; i++)
    {
        if(g_partinfo[i].flash_type != flash_type)
            continue;
        if(g_partinfo[i].perm != ACCESS_NONE)
            continue;
        if((g_partinfo[i].start_addr >= start_addr)&&(g_partinfo[i].start_addr <= end_addr))
        {
            MT_INFO_FLASH("%s(%s) is not permitted to be opened.\n", g_partinfo[i].devname, g_partinfo[i].partname);
            return MT_FAILURE;
        }
    }
    return MT_SUCCESS;
}

static mt_s32 flash_init(mt_void)
{
    mt_char line[512];
    FILE* fp = MT_NULL;
    mt_u32 i = 0;
    mt_u64 start_addr[MT_FLASH_TYPE_BUTT];
    mt_s32 ret = MT_SUCCESS;

    for(i=0; i< MAX_PARTS; i++)
    {
        g_partinfo[i].start_addr= 0;
        g_partinfo[i].partsize= 0;
        g_partinfo[i].blocksize = 0;
        g_partinfo[i].flash_type = MT_FLASH_TYPE_BUTT;
        g_partinfo[i].perm = ACCESS_BUTT;
        memset(g_partinfo[i].devname, '\0', FLASH_NAME_LEN);
        memset(g_partinfo[i].partname, '\0', FLASH_NAME_LEN);
    }

    for(i=0; i< MAX_HANDLE; i++)
    {
        g_flashinfo[i].fd = (ulong)INVALID_FD;
        g_flashinfo[i].open_addr = 0;
        g_flashinfo[i].open_len= 0;
        g_flashinfo[i].p_partinfo = NULL;
        g_flashinfo[i].flash_type = MT_FLASH_TYPE_BUTT;
    }

    if( (MT_FLASH_STAT_INSTALL == g_devstat[MT_FLASH_TYPE_SPI_0])
            || (MT_FLASH_STAT_INSTALL == g_devstat[MT_FLASH_TYPE_NAND_0]))
    {

        fp = fopen("/proc/mtd", "r");
        if (fp)
        {
            if (NULL == fgets(line, sizeof(line), fp)) { //skip first line
                fclose(fp);
                return MT_FAILURE;
            }

            MT_INFO_FLASH(" devname\t  partsize\tblocksize   partname    start_addr\n");
            for(i=0; i<MT_FLASH_TYPE_BUTT; i++)
            {
                start_addr[i] = 0;
            }
            for(i=0; fgets(line, sizeof(line), fp) != 0; i++)
            {
                mt_s8   argv[4][32];
                mt_s8   *p;
                int fd = -1;

                p = (mt_s8*)&line[0];

                p = skip_space(p);
                p = skip_word(p);
                p = get_word(p, argv[1]);
                p = get_word(p, argv[2]);
                p = get_word(p, argv[3]);
                p = p; /* for no TQE warning */

                if (i >= MAX_PARTS)
                {
                    MT_INFO_FLASH("Detected there has more than %d partitions.\n" \
                              "You should encrease MAX_PARTS in order to use left partitions!\n", MAX_PARTS);
                    break;
                }

                g_partinfo[i].partsize = (mt_u64)(mt_s64)strtoull((const char*)argv[1], (char**)NULL, 16);
                g_partinfo[i].blocksize = (mt_u32)(mt_s64)strtol((const char*)argv[2], (char**)NULL,16); //erase size
                memset(g_partinfo[i].partname, 0, sizeof(g_partinfo[i].partname));
                //FIXME: bad usage
                strncpy(g_partinfo[i].partname, (char*)(argv[3]+1), (strlen((char*)argv[3])-2));
                g_partinfo[i].flash_type = get_flash_type_by_bootargs(g_partinfo[i].partname);
                g_partinfo[i].start_addr = start_addr[g_partinfo[i].flash_type];
                start_addr[g_partinfo[i].flash_type] += g_partinfo[i].partsize;
#if defined (ANDROID)
                snprintf(g_partinfo[i].devname, sizeof(g_partinfo[i].devname), DEV_MTDBASE"%d", i);
#else
                snprintf(g_partinfo[i].devname, sizeof(g_partinfo[i].devname), "/dev/mtd%d", i);
#endif
                if ((fd = open(g_partinfo[i].devname, O_RDWR | O_CLOEXEC)) == -1)
                {
                    if ((fd = open(g_partinfo[i].devname, O_RDONLY | O_CLOEXEC)) == -1)
                    {
                        MT_ERR_FLASH("Can't open \"%s\"\n", g_partinfo[i].devname);
                        //fclose(fp);
                        g_partinfo[i].perm = ACCESS_NONE;
                        continue;
                    }
                    MT_INFO_FLASH("%s->%d access %s readonly!\n",__func__,__LINE__,g_partinfo[i].devname);
                    g_partinfo[i].perm = ACCESS_RD;
                } else {
                    MT_INFO_FLASH("%s->%d access %s read and write, i:%d, fd:%x!\n",__func__,__LINE__, g_partinfo[i].devname, i, fd);
                    g_partinfo[i].perm = ACCESS_RDWR;
                }
                close(fd);
            }
        }
        else
        {
            MT_ERR_FLASH("open /proc/mtd file failure!\n");
            return MT_FAILURE;
        }

        g_flashopt[MT_FLASH_TYPE_SPI_0].raw_erase = spi_raw_erase;
        g_flashopt[MT_FLASH_TYPE_SPI_0].raw_read  = spi_raw_read;
        g_flashopt[MT_FLASH_TYPE_SPI_0].raw_write = spi_raw_write;
        g_flashopt[MT_FLASH_TYPE_NAND_0].raw_erase = nand_raw_erase;
        g_flashopt[MT_FLASH_TYPE_NAND_0].raw_read  = nand_raw_read;
        g_flashopt[MT_FLASH_TYPE_NAND_0].raw_write = nand_raw_write;

        if (fclose(fp))
        {
            return MT_FAILURE;
        }
    }

    return ret;
}

static mt_s32 all_flash_init(void)
{
    mt_u8 loop;

    if (!g_initflag)
    {
        (mt_void)pthread_mutex_init(&g_flashmutex, NULL);

        if(MT_SUCCESS != get_bootargs(g_bootargs, (sizeof(g_bootargs) -1)))
        {
            MT_ERR_FLASH("Failed to get bootargs. \n");
            return MT_FAILURE;
        }

        for( loop = 0; loop < MT_FLASH_TYPE_BUTT; loop++)
        {
            g_devstat[loop] = MT_FLASH_STAT_UNINSTALL;
        }

        for(loop=0; loop<MT_FLASH_TYPE_BUTT; loop++)
        {
            g_wrpos[loop] = strstr((mt_char *)g_bootargs, g_flashstr[loop]);
        }

        if( MT_SUCCESS == spi_raw_init())
        {
            g_devstat[MT_FLASH_TYPE_SPI_0] = MT_FLASH_STAT_INSTALL;
        }
        /*else if(strstr((char*)g_bootargs, "mt_sfc:"))*/
        else if(MT_NULL != g_wrpos[MT_FLASH_TYPE_SPI_0])
        {
            MT_ERR_FLASH("spi init fail! \n");
            return MT_FAILURE;
        }

        if( MT_SUCCESS == nand_raw_init())
        {
            g_devstat[MT_FLASH_TYPE_NAND_0] = MT_FLASH_STAT_INSTALL;
        }
        /*else if(strstr((char*)g_bootargs, "mtnand:"))*/
        else if(MT_NULL != g_wrpos[MT_FLASH_TYPE_NAND_0])
        {
            (mt_void)spi_raw_destroy();
            MT_ERR_FLASH("nand init fail! \n");
            return MT_FAILURE;
        }

        if( MT_SUCCESS == emmc_raw_init((char *)g_bootargs))
        {
            g_devstat[MT_FLASH_TYPE_EMMC_0] = MT_FLASH_STAT_INSTALL;
        }
        else if(MT_NULL != g_wrpos[MT_FLASH_TYPE_EMMC_0])
        {
            (mt_void)spi_raw_destroy();
            (mt_void)nand_raw_destroy();
            MT_ERR_FLASH("emmc init fail! \n");
            return MT_FAILURE;
        }

        if (flash_init())
        {
            (mt_void)spi_raw_destroy();
            (mt_void)nand_raw_destroy();
            MT_ERR_FLASH("Flash init fail! \n");
            return MT_FAILURE;
        }
    }
    g_initflag = MT_TRUE;

    return MT_SUCCESS;
}

static mt_s32 mt_flash_unused_handle(void)
{
    mt_s8 loop;

    for(loop = 0; loop < MAX_HANDLE; loop++)
    {
        if(INVALID_FD == g_flashinfo[loop].fd)
        {
            break;
        }
    }

    return loop;
}

mt_s32 mt_flash_openby_type_and_addr(mt_flash_type_t flash_type,
                                     mt_u64 addr,
                                     mt_u64 len)
{
    unsigned long long  total_size = 0;
    unsigned long  pagesize = 0;
    unsigned long  blocksize = 0;
    unsigned long  oobsize = 0;
    unsigned long  blockshift = 0;
    emmc_cb_s *p_emmc_cb;
    mt_s32  p_flash = 0;

    if (all_flash_init())
    {
        return INVALID_FD;
    }

    if ((MT_FLASH_TYPE_BUTT <= flash_type))
    {
        MT_ERR_FLASH("flash_type error! \n");
        return INVALID_FD;
    }

    if( (MT_FLASH_STAT_INSTALL != g_devstat[flash_type]))
    {
        MT_ERR_FLASH("No config flash[type:%d].", flash_type);
        return INVALID_FD;
    }

    pthread_mutex_lock(&g_flashmutex);

    if( MT_FLASH_TYPE_EMMC_0 == flash_type )
    {
        p_emmc_cb = emmc_raw_open(addr, len);
        if( NULL == p_emmc_cb )
        {
            pthread_mutex_unlock(&g_flashmutex);
            return INVALID_FD;
        }

        if(MAX_HANDLE == (p_flash = mt_flash_unused_handle()))
        {
            MT_ERR_FLASH("flash array full! \n");
            (mt_void)emmc_raw_close(p_emmc_cb);
            pthread_mutex_unlock(&g_flashmutex);
            return INVALID_FD;
        }

        g_flashinfo[p_flash].fd = (ulong)p_emmc_cb;
        g_flashinfo[p_flash].flash_type = MT_FLASH_TYPE_EMMC_0;
        pthread_mutex_unlock(&g_flashmutex);
        return p_flash;
    }
    else if (MT_FLASH_TYPE_SPI_0 == flash_type)
    {
        spi_raw_get_info(&total_size, &pagesize, &blocksize, &oobsize, &blockshift);
    }
    else
    {
        nand_raw_get_info((unsigned long long *)&total_size, (unsigned long *)&pagesize, (unsigned long *)&blocksize,
            (unsigned long *)&oobsize, (unsigned long *)&blockshift);
    }

    if (0 == blocksize)
    {
        MT_ERR_FLASH("blocksize shouldn't equal 0!\n");
        pthread_mutex_unlock(&g_flashmutex);
        return INVALID_FD;
    }

    if ((int)(addr % blocksize) || (int)(len % blocksize))
    {
        MT_ERR_FLASH("Open Address(%#llx) and Len(%#llx) should be align with blocksize(0x%lX)!\n",
                     addr, len, blocksize);
        pthread_mutex_unlock(&g_flashmutex);
        return INVALID_FD;
    }

    if ((addr >= total_size) || (addr + len) > total_size)
    {
        MT_ERR_FLASH("Open Address(%#llx) and Len(%#llx) should be smaller than total_size(0x%lX)!\n",
                     addr, len, total_size);
        pthread_mutex_unlock(&g_flashmutex);
        return INVALID_FD;
    }

    if(MT_SUCCESS != permission_check(flash_type, addr, len))
    {
        MT_INFO_FLASH("%s(), %d, not permission to be opened.\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&g_flashmutex);
        return INVALID_FD;
    }

    if(MAX_HANDLE == (p_flash = mt_flash_unused_handle()))
    {
        MT_ERR_FLASH("flash array full! \n");
        pthread_mutex_unlock(&g_flashmutex);
        return INVALID_FD;
    }

    g_flashinfo[p_flash].fd = (ulong)(SPAN_PART_HANDLE + p_flash);
    g_flashinfo[p_flash].open_addr = addr;
    g_flashinfo[p_flash].open_len= len;
    g_flashinfo[p_flash].p_partinfo = NULL;
    g_flashinfo[p_flash].flash_type = flash_type;
    g_flashinfo[p_flash].pagesize = pagesize;
    g_flashinfo[p_flash].oobsize = oobsize;
    g_flashinfo[p_flash].blocksize = blocksize;

    if (MT_FLASH_TYPE_SPI_0 == flash_type)
    {
        g_flashinfo[p_flash].p_flashopt = (flash_opt_s *)&g_flashopt[MT_FLASH_TYPE_SPI_0];
    }
    if (MT_FLASH_TYPE_NAND_0 == flash_type)
    {
        g_flashinfo[p_flash].p_flashopt = (flash_opt_s *)&g_flashopt[MT_FLASH_TYPE_NAND_0];
    }
    MT_INFO_FLASH("fd= %d, open_address= %#llx, OpenLen= %#llx \n", g_flashinfo[p_flash].fd, addr, len);
    MT_INFO_FLASH("end.\n");
    pthread_mutex_unlock(&g_flashmutex);

    return p_flash;
}

mt_s32 mt_flash_openby_type_and_name(mt_flash_type_t flash_type,
                                     mt_char *partition_name)
{
    int fd = INVALID_FD;
    mt_s32  i, j;
    mt_char devname[FLASH_NAME_LEN] = {0};
    mt_u32  hPart = 0;
    mt_s32  p_flash = 0;
    mt_u64 addr = 0;
    mt_u64 len = 0;
    char *ptr;
    char media_name[20];
    emmc_cb_s *p_emmc_cb;
    unsigned long long  total_size = 0;
    unsigned long  pagesize = 0;
    unsigned long  blocksize = 0;
    unsigned long  oobsize = 0;
    unsigned long  blockshift = 0;

    if (MT_FLASH_TYPE_BUTT == flash_type)
    {
        MT_ERR_FLASH("flash_type error(flash_type=%d)! \n", flash_type);
        return INVALID_FD;
    }

    if (!partition_name)
    {
        MT_ERR_FLASH("partition_name is null! \n");
        return INVALID_FD;
    }


    if (all_flash_init())
    {
        return INVALID_FD;
    }

    if( (MT_FLASH_STAT_INSTALL != g_devstat[flash_type]))
    {
        MT_ERR_FLASH("No config flash[type:%d].", flash_type);
        return INVALID_FD;
    }

    pthread_mutex_lock(&g_flashmutex);

    if( MT_FLASH_TYPE_EMMC_0 == flash_type)
    {
#if defined (ANDROID)
        if( 0 == strncmp(partition_name, "/dev/block/mmcblk0p", strlen("/dev/block/mmcblk0p")))
#else
        if( 0 == strncmp(partition_name, "/dev/mmcblk0p", strlen("/dev/mmcblk0p")))
#endif
        {
	        char partname[FLASH_NAME_LEN];
            p_emmc_cb = emmc_node_open((mt_u8*)partition_name);
            if( NULL == p_emmc_cb )
            {
                pthread_mutex_unlock(&g_flashmutex);
                return INVALID_FD;
            }

            if(MAX_HANDLE == (p_flash =  mt_flash_unused_handle()))
            {
                MT_ERR_FLASH("flash array full! \n");
                (mt_void)emmc_raw_close(p_emmc_cb);
                pthread_mutex_unlock(&g_flashmutex);
                return INVALID_FD;
            }

            memset(partname, 0, sizeof(partname));
            memset(media_name, 0 , sizeof(media_name));
            strncpy(media_name, "mmcblk0", sizeof(media_name) - 1);
            media_name[sizeof(media_name) - 1] = '\0';
            if (find_part_from_devname(media_name, (char *)g_bootargs,
                                       partition_name, &addr, &len))
            {
                MT_ERR_FLASH("Cannot find partiton from %s\n", partition_name);
		(mt_void)emmc_raw_close(p_emmc_cb);
		pthread_mutex_unlock(&g_flashmutex);
                return INVALID_FD;
            }

            p_emmc_cb->partsize = len;
            g_flashinfo[p_flash].fd = (ulong)p_emmc_cb;
            g_flashinfo[p_flash].flash_type = MT_FLASH_TYPE_EMMC_0;
            pthread_mutex_unlock(&g_flashmutex);
            return  p_flash;
        }

        ptr = strstr((char*)g_bootargs, "mmcblk0:");
        if(NULL != ptr)
        {
            memset(media_name, 0 , sizeof(media_name));
            strncpy(media_name, "mmcblk0", sizeof(media_name) - 1);
            media_name[sizeof(media_name) - 1] = '\0';
            if (find_flash_part(ptr, media_name, partition_name, &addr, &len) == 0)
            {
                MT_ERR_FLASH("Cannot find partition: %s\n", partition_name);
            }
            else if(len == (mt_u64)(-1))
            {
                MT_ERR_FLASH("Can not contain char '-'\n");
            }
            else
            {
                p_emmc_cb = emmc_raw_open(addr, len);
                if( NULL == p_emmc_cb )
                {
                    pthread_mutex_unlock(&g_flashmutex);
                    return INVALID_FD;
                }

                if(MAX_HANDLE == (p_flash = mt_flash_unused_handle()))
                {
                    MT_ERR_FLASH("flash array full! \n");
                    (mt_void)emmc_raw_close(p_emmc_cb);
                    pthread_mutex_unlock(&g_flashmutex);
                    return INVALID_FD;
                }

                g_flashinfo[p_flash].fd = (ulong)p_emmc_cb;
                g_flashinfo[p_flash].flash_type = MT_FLASH_TYPE_EMMC_0;
                pthread_mutex_unlock(&g_flashmutex);
                return p_flash;

            }
        }

        pthread_mutex_unlock(&g_flashmutex);
        return INVALID_FD;
    }

    if(strstr((char*)g_bootargs, "mtnand:")
       || strstr((char*)g_bootargs, "mt_sfc:"))
    {
        for(i=0; i<MAX_PARTS; i++)
        {
            if(!strncmp(g_partinfo[i].devname, partition_name, strlen(partition_name) + 1) && (g_partinfo[i].flash_type == flash_type)) //eg: "/dev/mtd* "
            {
                break;
            }
            if(!strncmp(g_partinfo[i].partname, partition_name, strlen(partition_name) + 1) && (g_partinfo[i].flash_type == flash_type))
            {
                break;
            }
        }

        for(j=MAX_PARTS - 1; j>= 0; j--)
        {
            if(!strncmp(g_partinfo[j].devname, partition_name, strlen(partition_name) + 1) && (g_partinfo[j].flash_type == flash_type)) //eg: "/dev/mtd* "
            {
                break;
            }
            if(!strncmp(g_partinfo[j].partname, partition_name, strlen(partition_name) + 1) && (g_partinfo[j].flash_type == flash_type))
            {
                break;
            }
        }
        /* add i < 0 test in if branch to avoid pclint warning:
         * Warning 676: Possibly negative subscript (-1) in operator '['
         */
        if ((MAX_PARTS == i) || (i != j) || i < 0)
        {
            MT_ERR_FLASH("can not find a right flash part(i=%d, j=%d)!\n", i, j);
            pthread_mutex_unlock(&g_flashmutex);
            return INVALID_FD;
        }

        memset(devname, 0, FLASH_NAME_LEN);
        strncpy(devname, g_partinfo[i].devname, sizeof(devname));
        devname[sizeof(devname) - 1] = '\0';

        hPart = (mt_u32)i;

        if(MAX_HANDLE == (p_flash = mt_flash_unused_handle()))
        {
            MT_ERR_FLASH("flash array full! \n");
            pthread_mutex_unlock(&g_flashmutex);
            return INVALID_FD;
        }

        for(i=0; i<MAX_HANDLE; i++) // if the partition open, return index of array(g_flashinfo)
        {
            if((MT_NULL != g_flashinfo[i].p_partinfo) && (!strncmp(g_flashinfo[i].p_partinfo->devname, devname, strlen(devname) + 1)))
            {
                if (INVALID_FD != g_flashinfo[i].fd)
                {
                    MT_INFO_FLASH("fd = %d, devname =\"%s\"(%s)\n", g_flashinfo[i].fd, g_flashinfo[i].p_partinfo->devname, g_flashinfo[i].p_partinfo->partname);
                    pthread_mutex_unlock(&g_flashmutex);
                    return i;
                }
            }
        }

        if(g_partinfo[hPart].perm == ACCESS_RDWR)
        {
            fd = open(devname, O_RDWR | O_CLOEXEC);
        }
        else if(g_partinfo[hPart].perm == ACCESS_RD)
        {
            fd = open(devname, O_RDONLY | O_CLOEXEC);
        }
        else  if(g_partinfo[hPart].perm == ACCESS_WR)
        {
            fd = open(devname, O_WRONLY | O_CLOEXEC);
        }
        else
        {
            MT_ERR_FLASH("Device \"%s\"(%s) can not be opened \n", g_partinfo[hPart].devname, g_partinfo[hPart].partname);
            pthread_mutex_unlock(&g_flashmutex);
            return INVALID_FD;
        }

        if((fd < 0) || (fd >= SPAN_PART_HANDLE))
        {
            MT_ERR_FLASH("Open %s flash partition failure(fd = %d)!\n", devname, fd);
            pthread_mutex_unlock(&g_flashmutex);
            if (fd >= 0)
            {
                close (fd);
            }
            return INVALID_FD;
        }

		if (MT_FLASH_TYPE_SPI_0 == flash_type)
		{
			spi_raw_get_info(&total_size, &pagesize, &blocksize, &oobsize, &blockshift);
		}
		else
		{
			nand_raw_get_info((unsigned long long *)&total_size, (unsigned long *)&pagesize, (unsigned long *)&blocksize,
				(unsigned long *)&oobsize, (unsigned long *)&blockshift);
		}

        g_flashinfo[p_flash].fd = (ulong)fd;
        //g_flashinfo[p_flash].open_addr = 0;
        g_flashinfo[p_flash].open_len= 0;
        g_flashinfo[p_flash].p_partinfo = &g_partinfo[hPart];
        g_flashinfo[p_flash].open_addr = g_flashinfo[p_flash].p_partinfo->start_addr;
        g_flashinfo[p_flash].flash_type = flash_type;
        g_flashinfo[p_flash].pagesize = pagesize;
        g_flashinfo[p_flash].oobsize = oobsize;
        g_flashinfo[p_flash].blocksize = blocksize;

        if (MT_FLASH_TYPE_SPI_0 == flash_type)
        {
            g_flashinfo[p_flash].p_flashopt = (flash_opt_s *)&g_flashopt[MT_FLASH_TYPE_SPI_0];
        }
        if (MT_FLASH_TYPE_NAND_0 == flash_type)
        {
            g_flashinfo[p_flash].p_flashopt = (flash_opt_s *)&g_flashopt[MT_FLASH_TYPE_NAND_0];
        }
        MT_INFO_FLASH("fd = %d, devname =\"%s\"(%s)\n", g_flashinfo[p_flash].fd, g_flashinfo[p_flash].p_partinfo->devname, g_flashinfo[p_flash].p_partinfo->partname);
        MT_INFO_FLASH("end.\n");

        pthread_mutex_unlock(&g_flashmutex);
        return p_flash;
    }

    pthread_mutex_unlock(&g_flashmutex);
    return INVALID_FD;
}

static mt_s32 mt_flash_openby_name(mt_char *partition_name)
{
    mt_flash_type_t flash_type = MT_FLASH_TYPE_BUTT;
    mt_s32 i, j;
    mt_s32 p_flash;
    mt_u64 addr = 0;
    mt_u64 len = 0;
    char *ptr;
    char media_name[20];
    emmc_cb_s *p_emmc_cb;

    if (!partition_name)
    {
        MT_ERR_FLASH("partition_name is null! \n");
        return INVALID_FD;
    }

    if (all_flash_init())
    {
        return INVALID_FD;
    }

    flash_type = get_flash_type_by_bootargs(partition_name);
    if (MT_FLASH_TYPE_BUTT == flash_type)
    {
        MT_ERR_FLASH("Invalid partition name: %s\n", partition_name);
        return INVALID_FD;
    }

    if((MT_FLASH_TYPE_EMMC_0 == flash_type)
        && (NULL != (ptr = strstr((char*)g_bootargs, "mmcblk0:"))))
    {
        memset(media_name, 0 , sizeof(media_name));
        strncpy(media_name, "mmcblk0", sizeof(media_name) - 1);
        media_name[sizeof(media_name) - 1] = '\0';
        if (find_flash_part(ptr, media_name, partition_name, &addr, &len) == 0)
        {
            MT_ERR_FLASH("Cannot find partition: %s\n", partition_name);
            return INVALID_FD;
        }
        else if(len == (mt_u64)(-1))
        {
            MT_ERR_FLASH("Can not contain char '-'\n");
            return INVALID_FD;
        }
        else
        {
            pthread_mutex_lock(&g_flashmutex);
            p_emmc_cb = emmc_raw_open(addr, len);
            if( NULL == p_emmc_cb )
            {
                pthread_mutex_unlock(&g_flashmutex);
                return INVALID_FD;
            }

            if(MAX_HANDLE == (p_flash = mt_flash_unused_handle()))
            {
                MT_ERR_FLASH("flash array full! \n");
                (mt_void)emmc_raw_close(p_emmc_cb);
                pthread_mutex_unlock(&g_flashmutex);
                return INVALID_FD;
            }

            g_flashinfo[p_flash].fd = (ulong)p_emmc_cb;
            g_flashinfo[p_flash].flash_type = MT_FLASH_TYPE_EMMC_0;
            pthread_mutex_unlock(&g_flashmutex);
            return p_flash;
        }
    }

    if(strstr((char*)g_bootargs, "mtnand:")
       || strstr((char*)g_bootargs, "mt_sfc:"))
    {
        for(i=0; i<MAX_PARTS; i++)
        {
            if(!strncmp(g_partinfo[i].devname, partition_name, strlen(partition_name) + 1)) //eg: "/dev/mtd* "
            {
                break;
            }
            if(!strncmp(g_partinfo[i].partname, partition_name, strlen(partition_name) + 1))
            {
                break;
            }
        }

        for(j=MAX_PARTS - 1; j>= 0; j--)
        {
            if(!strncmp(g_partinfo[j].devname, partition_name, strlen(partition_name) + 1)) //eg: "/dev/mtd* "
            {
                break;
            }
            if(!strncmp(g_partinfo[j].partname, partition_name, strlen(partition_name) + 1))
            {
                break;
            }
        }
        /* add i < 0 test in if branch to avoid pclint warning:
         * Warning 676: Possibly negative subscript (-1) in operator '['
         */
        if ((MAX_PARTS == i) || (i != j || i < 0))
        {
            MT_ERR_FLASH("can not find a right flash part(i=%d, j=%d)!\n", i, j);
            return INVALID_FD;
        }

        flash_type = g_partinfo[i].flash_type;
        p_flash = mt_flash_openby_type_and_name(flash_type, partition_name);
        return p_flash;
    }

    return INVALID_FD;
}

mt_s32 mt_flash_open(mt_flash_type_t flash_type, mt_char *partition_name, mt_u64 addr, mt_u64 len)
{
    mt_s32  p_flash = 0;

    //MT_INFO_FLASH("flash_type=%d, PartitionName=%s, Address=0x%llx, Len=0x%llx\n", flash_type, partition_name, addr, len);

    if( NULL == partition_name )
    {
        p_flash = mt_flash_openby_type_and_addr(flash_type, addr, len);
    }
    else
    {
        if ( MT_FLASH_TYPE_BUTT == flash_type )
        {
            p_flash = mt_flash_openby_name(partition_name);
        }
        else
        {
            p_flash = mt_flash_openby_type_and_name(flash_type, partition_name);
        }
    }

    return p_flash;
}


mt_s32 mt_flash_close(mt_s32 p_flash)
{
    mt_flash_partinfo_s *p_partinfo = NULL;
    mt_u32 i;
    mt_s32 ret;
    emmc_cb_s *p_emmc_cb;

    /* avoid pclint warning */
    p_partinfo = p_partinfo;

    CHECK_flash_init(p_flash);

    if( MT_FLASH_TYPE_EMMC_0 == g_flashinfo[p_flash].flash_type )
    {
        p_emmc_cb = ((emmc_cb_s *)(g_flashinfo[p_flash].fd));
        emmc_raw_close(p_emmc_cb);
        g_flashinfo[p_flash].fd = (ulong)INVALID_FD;
        g_flashinfo[p_flash].open_addr = 0;
        g_flashinfo[p_flash].open_len= 0;
        g_flashinfo[p_flash].p_partinfo = NULL;
        g_flashinfo[p_flash].flash_type = MT_FLASH_TYPE_BUTT;

        return MT_SUCCESS;
    }

    p_partinfo = g_flashinfo[p_flash].p_partinfo;

    pthread_mutex_lock(&g_flashmutex);
    MT_INFO_FLASH("fd = %d\n", g_flashinfo[p_flash].fd);

    if (SPAN_PART_HANDLE <= g_flashinfo[p_flash].fd)
    {
        g_flashinfo[p_flash].fd = (ulong)INVALID_FD;
        g_flashinfo[p_flash].open_addr = 0;
        g_flashinfo[p_flash].open_len= 0;
        g_flashinfo[p_flash].p_partinfo = NULL;
    }
    else
    {
        ret = close((int)(g_flashinfo[p_flash].fd));
        if(0 != ret)
        {
            MT_ERR_FLASH("Close %s flash partition failure %d!\n", g_flashinfo[p_flash].p_partinfo->devname, ret);
            pthread_mutex_unlock(&g_flashmutex);
            return MT_FAILURE;
        }
        g_flashinfo[p_flash].fd = (ulong)INVALID_FD;
        g_flashinfo[p_flash].open_addr = 0;
        g_flashinfo[p_flash].open_len= 0;
        g_flashinfo[p_flash].p_partinfo = NULL;
    }

    for(i=0; i< MAX_HANDLE; i++)
    {
        if (INVALID_FD != g_flashinfo[i].fd)
        {
            break;
        }
    }

    if (MAX_HANDLE == i)
    {
        (mt_void)spi_raw_destroy();
        (mt_void)nand_raw_destroy();
        g_initflag = MT_FALSE;
    }

    MT_INFO_FLASH("end.\n");
    pthread_mutex_unlock(&g_flashmutex);

    return MT_SUCCESS;
}

static mt_s32 compensate_nand_address(mt_u64 addr, mt_u64 *start_addr)
{
    unsigned long long total_size = 0;
    unsigned long pagesize = 0;
    unsigned long blocksize = 0;
    unsigned long oobsize = 0;
    unsigned long blockshift = 0;
    int idx = 0;
    mt_s32 ret;

    nand_raw_get_info(&total_size, &pagesize, &blocksize, &oobsize, &blockshift);
    if (0 == blocksize)
    {
        MT_ERR_FLASH("blocksize shouldn't equal 0!\n");
        return MT_FAILURE;
    }
    idx = (int)(addr >> blockshift);
    ret = nand_raw_get_physical_index(*start_addr, &idx, (mt_s32)blocksize);
    if (0 != ret)
    {
        MT_ERR_FLASH("logical addr change to physical addr error!\n");
        return ret;
    }
    *start_addr += (unsigned long)((mt_u32)idx << blockshift);
    *start_addr += (addr % blocksize);

    return MT_SUCCESS;
}

mt_s32 mt_flash_erase(mt_handle p_flash, mt_u64 addr, mt_u64 len)
{
    mt_u64 start_addr = 0;
    mt_u64 limit_len = 0;
    mt_s32 ret;

    /* len bigger than 2GB should use mt_flash_erase64 instead. */
    if (len >= 0x80000000) {
	MT_ERR_FLASH("len is too big(0x%llx), should use mt_flash_erase64() instead.\n");
	return MT_FAILURE;
    }

    CHECK_flash_init(p_flash);

    if( MT_FLASH_TYPE_EMMC_0 == g_flashinfo[p_flash].flash_type)
    {
#if defined (MT_EMC_ERASE_SUPPORT)
        //printf("mt_flash_erase -> emc_raw_erase\naddr=0x%08llx, len=0x%08llx\n", addr, len);
        return emc_raw_erase((emmc_cb_s *)g_flashinfo[p_flash].fd, addr, len);
#else
        return MT_SUCCESS;
#endif
    }

    pthread_mutex_lock(&g_flashmutex);

    if (SPAN_PART_HANDLE <= g_flashinfo[p_flash].fd)
    {
        start_addr = g_flashinfo[p_flash].open_addr;
        limit_len = g_flashinfo[p_flash].open_len;
    }
    else
    {
        start_addr = g_flashinfo[p_flash].p_partinfo->start_addr;
        limit_len = g_flashinfo[p_flash].p_partinfo->partsize;
    }
    CHECK_ADDR_LEN_VALID(addr, len, limit_len);

    if (MT_FLASH_TYPE_NAND_0 == g_flashinfo[p_flash].flash_type)
    {
        ret = compensate_nand_address(addr, &start_addr);
        if (MT_SUCCESS != ret)
        {
            pthread_mutex_unlock(&g_flashmutex);
            return MT_FAILURE;
        }
    }
    else
    {
        start_addr += addr;
    }
    MT_INFO_FLASH("HANDLE=%d, Address=0x%llx, Len=0x%llx\n", p_flash, start_addr, len);

    if (!g_flashinfo[p_flash].p_flashopt->raw_erase)
    {
        MT_ERR_FLASH("flash service function ptr(raw_erase) is NULL! \n");
        pthread_mutex_unlock(&g_flashmutex);
        return MT_FAILURE;
    }

    ret = (mt_s32)g_flashinfo[p_flash].p_flashopt->raw_erase((int)(g_flashinfo[p_flash].fd), (unsigned long long)start_addr, len, g_flashinfo[p_flash].open_addr, limit_len);
    MT_INFO_FLASH("end.\n");
    pthread_mutex_unlock(&g_flashmutex);

    return ret;
}

#if 0
static mt_s64 mt_flash_erase64(mt_handle p_flash, mt_u64 addr, mt_u64 len)
{
    mt_u64 start_addr = 0;
    mt_u64 limit_len = 0;
    mt_s64 ret;

    CHECK_flash_init(p_flash);

    if( MT_FLASH_TYPE_EMMC_0 == g_flashinfo[p_flash].flash_type)
    {
        return MT_SUCCESS;
    }

    pthread_mutex_lock(&g_flashmutex);

    if (SPAN_PART_HANDLE <= g_flashinfo[p_flash].fd)
    {
        start_addr = g_flashinfo[p_flash].open_addr;
        limit_len = g_flashinfo[p_flash].open_len;
    }
    else
    {
        start_addr = g_flashinfo[p_flash].p_partinfo->start_addr;
        limit_len = g_flashinfo[p_flash].p_partinfo->partsize;
    }
    CHECK_ADDR_LEN_VALID(addr, len, limit_len);

    if (MT_FLASH_TYPE_NAND_0 == g_flashinfo[p_flash].flash_type)
    {
        ret = (mt_s64)compensate_nand_address(addr, &start_addr);
        if (MT_SUCCESS != ret)
        {
            pthread_mutex_unlock(&g_flashmutex);
            return MT_FAILURE;
        }
    }
    else
    {
        start_addr += addr;
    }
    MT_INFO_FLASH("HANDLE=%d, Address=0x%llx, Len=0x%llx\n", p_flash, start_addr, len);

    if (!g_flashinfo[p_flash].p_flashopt->raw_erase)
    {
        MT_ERR_FLASH("flash service function ptr(raw_erase) is NULL! \n");
        pthread_mutex_unlock(&g_flashmutex);
        return MT_FAILURE;
    }

    ret = (mt_s64)g_flashinfo[p_flash].p_flashopt->raw_erase(g_flashinfo[p_flash].fd, (unsigned long long)start_addr, len, g_flashinfo[p_flash].open_addr, limit_len);
    MT_INFO_FLASH("end.\n");
    pthread_mutex_unlock(&g_flashmutex);

    return ret;
}
#endif

mt_s32 mt_flash_read(mt_handle p_flash, mt_u64 addr, mt_u8 *buf, mt_u32 len, mt_u32 flags)
{
    mt_u64 start_addr = 0;
    mt_u64 limit_len = 0;
    mt_s32 ret, write_oob;
    emmc_cb_s *p_emmc_cb;

    if (NULL == buf)
    {
        return MT_FAILURE;
    }

    CHECK_flash_init(p_flash);

    pthread_mutex_lock(&g_flashmutex);

    if( MT_FLASH_TYPE_EMMC_0 == g_flashinfo[p_flash].flash_type)
    {
        p_emmc_cb = (emmc_cb_s *)(g_flashinfo[p_flash].fd);
        ret = emmc_raw_read(p_emmc_cb, addr, len, buf);
        pthread_mutex_unlock(&g_flashmutex);
	/* union return value to MT_FAILURE.*/
	if (ret <0)
		return MT_FAILURE;
	return ret;
    }

    if (SPAN_PART_HANDLE <= g_flashinfo[p_flash].fd)
    {
        start_addr = g_flashinfo[p_flash].open_addr;
        limit_len = g_flashinfo[p_flash].open_len;
    }
    else
    {
        start_addr = g_flashinfo[p_flash].p_partinfo->start_addr;
        limit_len = g_flashinfo[p_flash].p_partinfo->partsize;
    }

	if (MT_FLASH_RW_FLAG_WITH_OOB == (flags & MT_FLASH_RW_FLAG_WITH_OOB))
	{
		if (g_flashinfo[p_flash].pagesize <= 0)
		{
			MT_ERR_FLASH("Page Size of HANDLE %u is not initialized!\n", p_flash);
			pthread_mutex_unlock(&g_flashmutex);
			return MT_FAILURE;
		}
		ulong len_without_oob = (len
					   / (g_flashinfo[p_flash].oobsize
					      + g_flashinfo[p_flash].pagesize))
					  * g_flashinfo[p_flash].pagesize;
		if (len % (g_flashinfo[p_flash].oobsize
			      + g_flashinfo[p_flash].pagesize))
			len_without_oob += g_flashinfo[p_flash].pagesize;
		CHECK_ADDR_LEN_VALID(addr, len_without_oob, limit_len);
	}
	else
	{
		CHECK_ADDR_LEN_VALID(addr, len, limit_len);
	}

    if (MT_FLASH_TYPE_NAND_0 == g_flashinfo[p_flash].flash_type)
    {
        ret = compensate_nand_address(addr, &start_addr);
        if (MT_SUCCESS != ret)
        {
            pthread_mutex_unlock(&g_flashmutex);
            return MT_FAILURE;
        }
    }
    else
    {
        start_addr += addr;
    }
    MT_INFO_FLASH("HANDLE=%d, Address=0x%llx, Len=0x%x, Flag=%d\n", p_flash, start_addr, len, flags);

    if (MT_FLASH_RW_FLAG_WITH_OOB == (flags & MT_FLASH_RW_FLAG_WITH_OOB))
    {
        write_oob = 1;
    }
    else
    {
        write_oob = 0;
    }

    if (!g_flashinfo[p_flash].p_flashopt->raw_read)
    {
        MT_ERR_FLASH("flash service function ptr(raw_read) is NULL! \n");
        pthread_mutex_unlock(&g_flashmutex);
        return MT_FAILURE;
    }

    ret = g_flashinfo[p_flash].p_flashopt->raw_read((int)(g_flashinfo[p_flash].fd), (unsigned long long *)&start_addr, buf, len, g_flashinfo[p_flash].open_addr, limit_len, write_oob, 1);
    MT_INFO_FLASH("totalread =0x%x, end.\n", ret);
    pthread_mutex_unlock(&g_flashmutex);

    return ret;
}

mt_s64 mt_flash_write(mt_handle p_flash, mt_u64 addr, mt_u8 *buf, mt_u32 len, mt_u32 flags)
{
    unsigned long long start_addr = 0;
    mt_u64 limit_len = 0;
    mt_s64 ret;
    mt_s32 write_oob, erase;

    unsigned long long total_size = 0;
    unsigned long pagesize = 0;
    unsigned long blocksize = 0;
    unsigned long oobsize = 0;
    unsigned long blockshift = 0;

    unsigned long long eraselen = 0;
    ulong blocksize_new = 0;
    emmc_cb_s *p_emmc_cb;

    if (NULL == buf)
    {
        return MT_FAILURE;
    }

    CHECK_flash_init(p_flash);
    pthread_mutex_lock(&g_flashmutex);

    if( MT_FLASH_TYPE_EMMC_0 == g_flashinfo[p_flash].flash_type)
    {
        p_emmc_cb = (emmc_cb_s *)(g_flashinfo[p_flash].fd);
        ret = emmc_raw_write(p_emmc_cb, addr, len, buf);
        pthread_mutex_unlock(&g_flashmutex);
	if (ret <0)
		return MT_FAILURE;
	return ret;
    }

    if (SPAN_PART_HANDLE <= g_flashinfo[p_flash].fd)
    {
        start_addr = g_flashinfo[p_flash].open_addr;
        limit_len = g_flashinfo[p_flash].open_len;
    }
    else
    {
        start_addr = g_flashinfo[p_flash].p_partinfo->start_addr;
        limit_len = g_flashinfo[p_flash].p_partinfo->partsize;
    }
    if (MT_FLASH_RW_FLAG_WITH_OOB == (flags & MT_FLASH_RW_FLAG_WITH_OOB))
    {
		if (g_flashinfo[p_flash].pagesize <= 0)
		{
			MT_ERR_FLASH("Page Size of HANDLE %u is not initialized!\n", p_flash);
			pthread_mutex_unlock(&g_flashmutex);
			return MT_FAILURE;
		}

		ulong len_without_oob = (len
					   / (g_flashinfo[p_flash].oobsize
					      + g_flashinfo[p_flash].pagesize))
					  * g_flashinfo[p_flash].pagesize;
		if (len % (g_flashinfo[p_flash].oobsize
			      + g_flashinfo[p_flash].pagesize))
			len_without_oob += g_flashinfo[p_flash].pagesize;
		CHECK_ADDR_LEN_VALID(addr, len_without_oob, limit_len);
    }
    else
    {
        CHECK_ADDR_LEN_VALID(addr, len, limit_len);
    }

    if (MT_FLASH_TYPE_NAND_0 == g_flashinfo[p_flash].flash_type)
    {
        ret = compensate_nand_address(addr, &start_addr);
        if (MT_SUCCESS != ret)
        {
            pthread_mutex_unlock(&g_flashmutex);
            return MT_FAILURE;
        }
    }
    else
    {
        start_addr += addr;
    }
    MT_INFO_FLASH("HANDLE=%d, Address=0x%llx, Len=0x%x, Flag=%d\n", p_flash, start_addr, len, flags);

    if (MT_FLASH_TYPE_SPI_0 == g_flashinfo[p_flash].flash_type)
    {
        spi_raw_get_info(&total_size, &pagesize, &blocksize, &oobsize, &blockshift);
    }
    else if (MT_FLASH_TYPE_NAND_0 == g_flashinfo[p_flash].flash_type)
    {
        nand_raw_get_info(&total_size, &pagesize, &blocksize, &oobsize, &blockshift);
    }

    if (MT_FLASH_RW_FLAG_ERASE_FIRST == (flags & MT_FLASH_RW_FLAG_ERASE_FIRST))
    {
        erase = 1;
    }
    else
    {
        erase = 0;
    }
    /* avoid pclint div 0 warning */
    if (!pagesize) {
        pthread_mutex_unlock(&g_flashmutex);
        return -1;
    }

    if (MT_FLASH_RW_FLAG_WITH_OOB == (flags & MT_FLASH_RW_FLAG_WITH_OOB))
    {
        blocksize_new = blocksize + oobsize * (blocksize / pagesize);
        write_oob = 1;
    }
    else
    {
        blocksize_new = blocksize;
        write_oob = 0;
    }

    if (erase)
    {
        /* avoid pclint div 0 warning */
        if (!blocksize_new) {
            pthread_mutex_unlock(&g_flashmutex);
            return -1;
        }

        eraselen = len / blocksize_new;
        if (len % blocksize_new)
        {
            eraselen += 1;
        }
        eraselen = eraselen * blocksize;
        //MT_PRINT("> %s: [%d], eraselen=%#x\n", __FUNCTION__, __LINE__, eraselen);

        if (!g_flashinfo[p_flash].p_flashopt->raw_erase)
        {
            MT_ERR_FLASH("flash service function ptr(raw_erase) is NULL! \n");
            pthread_mutex_unlock(&g_flashmutex);
            return MT_FAILURE;
        }
        ret = g_flashinfo[p_flash].p_flashopt->raw_erase((int)(g_flashinfo[p_flash].fd), start_addr, eraselen, g_flashinfo[p_flash].open_addr, limit_len);
        if (0 >= ret)
        {
            if (MT_FLASH_END_DUETO_BADBLOCK != ret)
            {
                MT_ERR_FLASH("earse fail!\n");
                pthread_mutex_unlock(&g_flashmutex);
                return ret;
            }
        }
    }

    if (!g_flashinfo[p_flash].p_flashopt->raw_write)
    {
        MT_ERR_FLASH("flash service function ptr(raw_write) is NULL! \n");
        pthread_mutex_unlock(&g_flashmutex);
        return MT_FAILURE;
    }

    ret = g_flashinfo[p_flash].p_flashopt->raw_write((int)(g_flashinfo[p_flash].fd), (unsigned long long *)&start_addr, buf, len, g_flashinfo[p_flash].open_addr, limit_len, write_oob);
    MT_INFO_FLASH("totalwrite =0x%x, end.\n", ret);
    pthread_mutex_unlock(&g_flashmutex);

    return ret;
}

mt_s32 mt_flash_get_info(mt_handle p_flash, mt_flash_interinfo_s *p_flashinfo)
{
    unsigned long long total_size = 0;
    unsigned long pagesize = 0;
    unsigned long blocksize = 0;
    unsigned long oobsize = 0;
    unsigned long blockshift = 0;
    emmc_cb_s *p_emmc_cb;

    if(NULL == p_flashinfo)
    {
        return MT_FAILURE;
    }

    CHECK_flash_init(p_flash);

    if( MT_FLASH_TYPE_EMMC_0 == g_flashinfo[p_flash].flash_type)
    {
        p_emmc_cb = (emmc_cb_s *)(g_flashinfo[p_flash].fd);

        memset(p_flashinfo, 0x00, sizeof(*p_flashinfo));
        p_flashinfo->total_size = g_emmcflash.raw_areasize;
        p_flashinfo->oobsize = 0;
        p_flashinfo->partsize = p_emmc_cb->partsize;
        p_flashinfo->blocksize = (g_emmcflash.erasesize *16);
        p_flashinfo->pagesize = 0;
        p_flashinfo->fd = (ulong)p_emmc_cb;
        p_flashinfo->flash_type = MT_FLASH_TYPE_EMMC_0;
        p_flashinfo->open_addr  = p_emmc_cb->addr;
        p_flashinfo->p_flashopt = NULL;

        return MT_SUCCESS;
    }

    switch (g_flashinfo[p_flash].flash_type)
    {
        case MT_FLASH_TYPE_SPI_0:
            {
                spi_raw_get_info(&total_size, &pagesize, &blocksize, &oobsize, &blockshift);
                break;
            }
        case MT_FLASH_TYPE_NAND_0:
            {
                nand_raw_get_info(&total_size, &pagesize, &blocksize, &oobsize, &blockshift);
                break;
            }
        default :
            {
                break;
            }
    }

    p_flashinfo->total_size = total_size;

    if (SPAN_PART_HANDLE <= g_flashinfo[p_flash].fd)
    {
        p_flashinfo->partsize = g_flashinfo[p_flash].open_len;
        p_flashinfo->p_partinfo = NULL;
    }
    else
    {
        p_flashinfo->partsize = g_flashinfo[p_flash].p_partinfo->partsize;
        p_flashinfo->p_partinfo = g_flashinfo[p_flash].p_partinfo;
    }
    p_flashinfo->blocksize = blocksize;
    p_flashinfo->pagesize  = pagesize;
    p_flashinfo->oobsize   = oobsize;
    p_flashinfo->fd = g_flashinfo[p_flash].fd;
    p_flashinfo->flash_type = g_flashinfo[p_flash].flash_type;
    p_flashinfo->p_flashopt = NULL;
    p_flashinfo->open_addr  = g_flashinfo[p_flash].open_addr;
    p_flashinfo->open_len= g_flashinfo[p_flash].open_len;

    return MT_SUCCESS;
}

/*****************************************************************/
//#define __SUPPORT__FUNC__
/*****************************************************************/


