
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <sys/time.h>

#include "mt_osal.h"
#include "mt_debug.h"
#include "mt_drv_struct.h"
#include "mpi_log.h"
#include "mt_mpi_mem.h"

#if defined(ANDROID)
#include <utils/Log.h>
#endif


static mt_u32 g_log_modinit = 0;
static mt_s32 g_dbgdevfd = -1;
static log_config_info_s *g_log_configinfo;/*the pointer of print control information in user state*/

const char *debuglevel_name[MT_LOG_LEVEL_BUTT+1] = {
	"FATAL",
	"ERROR",
	"WARN",
	"INFO",
	"DEBUG",
	"BUTT"
};


static pthread_mutex_t g_logmutex = PTHREAD_MUTEX_INITIALIZER;

#define MT_LOG_LOCK()  (void)pthread_mutex_lock(&g_logmutex)
#define MT_LOG_UNLOCK() (void)pthread_mutex_unlock(&g_logmutex)

MT_BOOL log_level_check(mt_s32 level, mt_u32 mod_id, MT_BOOL *is_user);
mt_u32 log_print_position_get(mt_u32 mod_id);
mt_u32 log_gettimeMs(mt_void);

static mt_s32 mpi_log_device_open(mt_char *pathname, const mt_s32 flags)
{
	mt_s32 log_fd;
	struct stat st;

	log_fd = open(pathname, flags, 0);
	if (log_fd == -1)
	{
		return MT_FAILURE;
	}

	if (fstat(log_fd, &st) == -1)
	{
		fprintf(stderr, "Cannot identify '%s'.\n", pathname);
		close(log_fd);
		return MT_FAILURE;
	}

	if (!S_ISCHR(st.st_mode))
	{
		fprintf(stderr, "%s is no device.\n", pathname);
		close(log_fd);
		return MT_FAILURE;
	}

	return log_fd;
}

static mt_s32 mpi_log_device_close(mt_s32 fd)
{
	mt_s32 ret;
	ret = close(fd);
	if (ret != MT_SUCCESS)
	{
		return MT_FAILURE;
	}
	return MT_SUCCESS;
}



#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
mt_s32 mpi_log_buffer_read(mt_u8 *buf, mt_u32 buflen, mt_u32 *copylen);
mt_s32 mpi_log_buffer_write(mt_u8 *buf, mt_u32 msglen);
mt_s32 log_print_to_buf(const char *format, ...);

mt_s32 mpi_log_buffer_read(mt_u8 *buf, mt_u32 buflen, mt_u32 *copylen)
{
	mt_s32 ret;
	log_buf_read_s rlog;

	if (g_dbgdevfd == -1)
	{
		MT_ERR_LOG("%s ERROR: device not opened.\n", __FUNCTION__);
		return MT_FAILURE;
	}

	rlog.buf = buf;
	rlog.size = buflen;
	rlog.len = 0;
	ret = ioctl(g_dbgdevfd, UMAP_CMPI_LOG_READ_LOG, &rlog);
	if (ret != MT_SUCCESS)
	{
		MT_ERR_LOG("%s ERROR: ioctl UMAP_CMPI_LOG_READ_LOG error\n", __FUNCTION__);
		return MT_FAILURE;
	}

	*copylen = rlog.len;

	return MT_SUCCESS;
}

mt_s32 mpi_log_buffer_write(mt_u8 *buf, mt_u32 msglen)
{
	mt_s32 ret;
	log_buf_write_s wlog;

	if (g_dbgdevfd == -1)
	{
		MT_ERR_LOG("%s ERROR: device not opened!\n", __FUNCTION__);
		return MT_FAILURE;
	}

	wlog.buf = buf;
	wlog.len = msglen;
	ret = ioctl(g_dbgdevfd, UMAP_CMPI_LOG_WRITE_LOG, &wlog);

	if (ret != MT_SUCCESS)
	{
		MT_ERR_LOG("%s ERROR: ioctl UMAP_CMPI_LOG_WRITE_LOG error\n", __FUNCTION__);
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}


mt_s32 log_print_to_buf(const char *format, ...)
{
	char log_str[LOG_MAX_TRACE_LEN] = {0};
	mt_s32 msglen = 0;

	va_list args;

	if (g_log_modinit == 0)
	{
		MT_ERR_LOG("log device not init(usr)!\n");
		return MT_FAILURE;
	}

	va_start(args, format);
	msglen = mt_osal_vsnprintf(log_str, LOG_MAX_TRACE_LEN-1, format, args);
	va_end(args);

	if (msglen >= (LOG_MAX_TRACE_LEN-1))
	{
		log_str[LOG_MAX_TRACE_LEN-1] = '\0';
		log_str[LOG_MAX_TRACE_LEN-2] = '\n';
		log_str[LOG_MAX_TRACE_LEN-3] = '.';
		log_str[LOG_MAX_TRACE_LEN-4] = '.';
		log_str[LOG_MAX_TRACE_LEN-5] = '.';
	}

	if ((msglen > 0) && (msglen < LOG_MAX_TRACE_LEN))
	{
		return mpi_log_buffer_write((mt_u8 *)log_str,(mt_u32)(msglen));
	}
	else
	{
		return MT_FAILURE;
	}
}
#endif

/* MT_TRUE: need print
*  MT_FALSE: not print
*/
MT_BOOL log_level_check(mt_s32 level, mt_u32 mod_id, MT_BOOL *is_user)
{
	if(mod_id < LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s))
	{
		if (g_log_modinit && level <= g_log_configinfo[mod_id].level)
		{
			return MT_TRUE;
		}

		if (!g_log_modinit && level <= MT_LOG_LEVEL_DEFAULT)
		{
			return MT_TRUE;
		}
	}
	return MT_FALSE;
}

/*
  0 serial port
  1 network
*/
mt_u32 log_print_position_get(mt_u32 mod_id)
{
	mt_u32 pos = 0;

	if(mod_id >= LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s))
	{
		return LOG_OUTPUT_SERIAL;
	}

	if(g_log_modinit == 0)
	{
		return LOG_OUTPUT_SERIAL;
	}

    if (g_log_configinfo->udisk_flag == 1)
    {
        return LOG_OUTPUT_UDISK;
    }

	pos = g_log_configinfo[mod_id].outpos;

	return pos;
}


mt_u32 log_gettimeMs(mt_void)
{
	struct timeval tv;
	(mt_void)gettimeofday(&tv, NULL);
	return (((mt_u32)tv.tv_sec)*1000+((mt_u32)tv.tv_usec)/1000);
}

mt_void mt_log_out(mt_u32 level, mt_mod_id_e mod_id, const char *func_name, mt_u32 line_num, const char *format, ...)
{
	mt_u32 time_ms = 0;
	char log_str[LOG_MAX_TRACE_LEN];
	mt_s32 msglen = 0;
    MT_BOOL blevel = MT_FALSE;
	va_list args = {0};
    MT_BOOL is_user = MT_FALSE;

	MT_LOG_LOCK();

	blevel = log_level_check((mt_s32)level, (mt_u32)mod_id, &is_user);

	if (blevel)
	{
		va_start(args, format);
		msglen = mt_osal_vsnprintf(log_str, LOG_MAX_TRACE_LEN, format, args);
		va_end(args);

		if (msglen >= LOG_MAX_TRACE_LEN)
		{
            log_str[LOG_MAX_TRACE_LEN-1] = '\0';  /* even the 'vsnprintf' commond will do it */
            log_str[LOG_MAX_TRACE_LEN-2] = '\n';
            log_str[LOG_MAX_TRACE_LEN-3] = '.';
            log_str[LOG_MAX_TRACE_LEN-4] = '.';
            log_str[LOG_MAX_TRACE_LEN-5] = '.';
		}

        /* log module has Initialized. */
		if (g_log_modinit)
		{
			/*get the time, unit ms*/
			time_ms = log_gettimeMs();
			if (!is_user)
			{
				mt_s32 nPos = (mt_s32)log_print_position_get((mt_u32)mod_id);
				switch(nPos)
				{
					case LOG_OUTPUT_SERIAL:/*output to serial port*/
					default:
#if defined(ANDROID)
						switch(level)
						{
							case MT_TRACE_LEVEL_FATAL:
								android_printLog(ANDROID_LOG_FATAL, g_log_configinfo[mod_id].mod_name, PRINT_FMT);
								break;

							case MT_TRACE_LEVEL_ERROR:
								android_printLog(ANDROID_LOG_ERROR, g_log_configinfo[mod_id].mod_name, PRINT_FMT);
								break;

							case MT_TRACE_LEVEL_WARN:
								android_printLog(ANDROID_LOG_WARN, g_log_configinfo[mod_id].mod_name, PRINT_FMT);
								break;

							case MT_TRACE_LEVEL_INFO:
								android_printLog(ANDROID_LOG_INFO, g_log_configinfo[mod_id].mod_name, PRINT_FMT);
								break;

							case MT_TRACE_LEVEL_DBG:
								android_printLog(ANDROID_LOG_DEBUG, g_log_configinfo[mod_id].mod_name, PRINT_FMT);
								break;

							default:
								android_printLog(ANDROID_LOG_DEBUG, g_log_configinfo[mod_id].mod_name, PRINT_FMT);
								break;
						}
#else
						MT_PRINT(PRINT_FMT);
#endif
						break;
					case LOG_OUTPUT_NETWORK:
					case LOG_OUTPUT_UDISK:
					#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
						log_print_to_buf(PRINT_FMT);
					#endif
						break;
				}


			}
			else
			{
				const char *module_name = (const char *)"ABC";//MT_ModuleMGR_GetModuleName(enModId);
				MT_PRINT("<\33[32m%s\33[0m>:[%s - %d]:%s\n", module_name, func_name, line_num, log_str);
			}

			MT_LOG_UNLOCK();
			return;
		}
		else
		{
			if (!is_user)
			{
				MT_PRINT("[%s-unknown]: %s[%d]:%s", debuglevel_name[level], func_name, line_num, log_str);
			}

			MT_LOG_UNLOCK();

			return;
		}
	}

	MT_LOG_UNLOCK();
	return;
}

mt_s32 mt_mpi_log_init(mt_void)
{
	mt_s32 ret;
	mt_s8 pathname[60];
	mt_u32 phyAddr;
	mt_u32 *virAddr;

	MT_LOG_LOCK();

	if (g_dbgdevfd == -1)
	{
		mt_osal_snprintf((char *)pathname,sizeof(pathname),"/dev/%s", UMAP_DEVNAME_LOG);
		g_dbgdevfd = mpi_log_device_open((char *)pathname, O_RDWR | O_NONBLOCK | O_CLOEXEC);
		if (g_dbgdevfd == MT_FAILURE)//-1
		{
			MT_LOG_UNLOCK();
			MT_ERR_LOG("mpi log device open failed\n");
			return MT_FAILURE;
		}

		ret = ioctl(g_dbgdevfd, UMAP_CMPI_LOG_INIT, &phyAddr);
		if(ret != 0)
		{
			mpi_log_device_close(g_dbgdevfd);
			MT_LOG_UNLOCK();
			MT_ERR_LOG("log init ioctl failed\n");
			return MT_FAILURE;
		}
        virAddr = (mt_u32 *)mt_mmap(phyAddr, LOG_CONFIG_BUF_SIZE);
        if(NULL ==  virAddr)
        {
            (mt_void)mpi_log_device_close(g_dbgdevfd);
            MT_LOG_UNLOCK();
            MT_ERR_LOG("MT_MPI_MMZ_Map failed\n");
            return MT_FAILURE;
        }

        g_log_configinfo = (log_config_info_s *)virAddr;
        g_log_modinit = 1;
    }

    MT_LOG_UNLOCK();

    MT_INFO_LOG("mpi log init OK\n");

    return MT_SUCCESS;
}

mt_void mt_mpi_log_deinit(mt_void)
{
    MT_LOG_LOCK();

    if (g_dbgdevfd != -1)
    {
        g_log_modinit = 0;
        if (MT_SUCCESS != mt_munmap((void*)g_log_configinfo))
        {
            MT_LOG_UNLOCK();
            MT_WARN_LOG("log info umap failed\n");
            MT_LOG_LOCK();
        }

        ioctl(g_dbgdevfd, UMAP_CMPI_LOG_EXIT);
        mpi_log_device_close(g_dbgdevfd);
        g_dbgdevfd = -1;
    }

    MT_LOG_UNLOCK();
}

mt_s32 mt_mpi_log_read(mt_u8 *pBuf, mt_u32 buflen, mt_u32 *copylen)
{
    mt_s32 ret = MT_SUCCESS;

    MT_LOG_LOCK();

    if (0 == g_log_modinit)
    {
        MT_LOG_UNLOCK();
        MT_ERR_LOG("%s ERROR: device not opened!\n", __FUNCTION__);
        return MT_FAILURE;
    }

    MT_LOG_UNLOCK();

#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
    ret = mpi_log_buffer_read(pBuf, buflen, copylen);
#endif

    return ret;
}

mt_s32 mt_mpi_log_level_set(mt_u32 mod_id, mt_log_level_e level)
{
    if ((mod_id >= LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s)) || (level >= MT_LOG_LEVEL_BUTT))
    {
        return MT_FAILURE;
    }

    MT_LOG_LOCK();

    if (0 == g_log_modinit)
    {
        MT_LOG_UNLOCK();
        MT_ERR_LOG("%s ERROR: device not opened!\n", __FUNCTION__);
        return MT_FAILURE;
    }

    g_log_configinfo[mod_id].level = (mt_u8)level;
    MT_LOG_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 mt_mpi_log_print_pos_set(mt_u32 mod_id, log_output_pos_e enprint_pos)
{
    if (mod_id >= LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s))
    {
        return MT_FAILURE;
    }

    MT_LOG_LOCK();

    if (0 == g_log_modinit)
    {
        MT_LOG_UNLOCK();
        MT_ERR_LOG("%s ERROR: device not opened!\n", __FUNCTION__);
        return MT_FAILURE;
    }

    if (enprint_pos != LOG_OUTPUT_BUTT)
    {
        g_log_configinfo[mod_id].outpos = (mt_u8)enprint_pos;
    }
    else
    {
        g_log_configinfo[mod_id].outpos = LOG_OUTPUT_POS_DEFAULT;
    }

    MT_LOG_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 mt_mpi_log_path_set(const mt_char* logpath)
{
    mt_s32 ret = MT_FAILURE;
    log_path_s s_logpath;

    MT_LOG_LOCK();

    if (logpath == NULL || g_dbgdevfd == -1)
    {
        MT_LOG_UNLOCK();

        MT_ERR_LOG(" params invalid or log non-initialized!\n");

        return ret;
    }

    s_logpath.path = logpath;
    s_logpath.pathlen = strlen((const char*)logpath) + 1;

    //1.first, check the path
    ret = access(logpath, W_OK | F_OK);
    if (ret != 0)
    {
        MT_LOG_UNLOCK();

        MT_ERR_LOG("logpath %s non-exist or non-writeable\n", logpath);

        return MT_FAILURE;
    }

    ret = ioctl(g_dbgdevfd, UMAP_CMPI_LOG_SET_PATH, &s_logpath);
    MT_LOG_UNLOCK();

    return ret;
}

mt_s32 mt_mpi_log_storepath_set(const mt_char* storepath)
{
    mt_s32 ret = MT_FAILURE;
    store_path_s s_storepath;

    MT_LOG_LOCK();

    if (storepath == NULL || g_dbgdevfd == -1)
    {
        MT_LOG_UNLOCK();
        MT_ERR_LOG(" params invalid or log non-initialized!\n");
        return ret;
    }

    s_storepath.path = storepath;
    s_storepath.pathlen = strlen((const char *)storepath) + 1;

    /* check the path */
    ret = access(storepath, W_OK);
    if (ret != 0)
    {
        MT_LOG_UNLOCK();
        MT_ERR_LOG("storepath %s non-writeable\n", storepath);
        return MT_FAILURE;
    }

    ret = ioctl(g_dbgdevfd, UMAP_CMPI_LOG_SET_STORE_PATH, &s_storepath);
    MT_LOG_UNLOCK();

    return ret;
}



