#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include "mt_kernel_adapt.h"
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_osal.h"
#include "mt_drv_log.h"
#include "mt_drv_proc.h"
#include "drv_log.h"
#include "mt_drv_mmz.h"
#include "drv_log_ioctl.h"
#include "mt_drv_file.h"
#include "mt_common.h"


mmz_buffer_s g_log_configbuf;
static mt_u32 g_log_modinit = 0;

/*kernel-state pointer of log control info*/
static log_config_info_s *g_log_configinfo;


#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
static log_buffer_info_s g_msg_bufinfo;
#endif

MT_DECLARE_MUTEX(g_logmutex);

#define LOG_FILE_LOCK()   down_interruptible(&g_logmutex)
#define LOG_FILE_UNLOCK() up(&g_logmutex)

#define MAX_FILENAME_LENTH 256

/* this variable will be used by /kmod/load script */
static int log_bufsize = 256 * DEBUG_MSG_BUF_RESERVE;
static char g_pathbuf[MAX_FILENAME_LENTH] = {0};
static char *udisk_logfile = g_pathbuf;
static MT_BOOL g_logfileflag = MT_FALSE;

static char g_storepath_buf[MAX_FILENAME_LENTH] = "/mnt";
char *store_path = g_storepath_buf;


char *debuglevel_name[MT_LOG_LEVEL_BUTT+1] = {
	"FATAL",
	"ERROR",
	"WARN",
	"INFO",
	"DEBUG",
	"BUTT"
};


#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
static mt_void log_buffer_reset(mt_void)
{
	unsigned long flags;
	local_irq_save(flags);
	g_msg_bufinfo.rp = g_msg_bufinfo.wp;
	g_msg_bufinfo.reset_flag++;
	local_irq_restore((unsigned long)flags);
}


mt_s32 mt_drv_log_buffer_read(mt_u8 *buf, mt_u32 buflen, mt_u32 *copylen, MT_BOOL is_kernel)
{
	log_buffer_info_s msg;
	mt_u32 buf_used;
	mt_u32 datalen1;
	mt_u32 datalen2;
	mt_u32 cplen;
	mt_u32 new_rp;
	unsigned long flags;

	if (g_msg_bufinfo.size == 0)
	{
		MT_ERR_LOG("Log buffer size is 0, please config buffer size, for example:");
		MT_ERR_LOG("Config buffer size 500K: modprobe mt_cmpi log_bufsize = 0x80000");
		return MT_FAILURE;
	}

	if (g_msg_bufinfo.wp == g_msg_bufinfo.rp)
	{
		if (g_logfileflag)
        {
			return MT_FAILURE;
        }
		else
		{
            /* the following code segment will pending when reboot or reload ko */
			wait_event_interruptible(g_msg_bufinfo.wq_nodata, (g_msg_bufinfo.wp != g_msg_bufinfo.rp));
		}
	}

	local_irq_save(flags);
	memcpy(&msg, &g_msg_bufinfo, sizeof(g_msg_bufinfo));
	local_irq_restore((unsigned long)flags);

	if (msg.wp < msg.rp)
	{
		buf_used = msg.size - msg.rp + msg.wp;
		datalen1 = msg.size - msg.rp;
		datalen2 = msg.wp;
	}
	else
	{
		buf_used = msg.wp - msg.rp;
		datalen1 = buf_used;
		datalen2 = 	0;
	}

	if (buflen <= (datalen1 + datalen2))
	{
		cplen = buflen;
	}
	else
	{
		cplen = datalen1 + datalen2;
	}

	if (datalen1 >= cplen)
	{
		if (is_kernel == MT_FALSE)
		{
			if (copy_to_user(buf, msg.start_viraddr+msg.rp, cplen))
			{
				MT_ERR_LOG("copy_to_user error\n");
				return MT_FAILURE;
			}
		}
		else
		{
			memcpy(buf, msg.start_viraddr+msg.rp, cplen);
		}

		new_rp = msg.rp + cplen;
	}
	else
	{
		if (is_kernel == MT_FALSE)
		{
			if (copy_to_user(buf, msg.start_viraddr+msg.rp, datalen1))
			{
				MT_ERR_LOG("copy_to_user error\n");
				return MT_FAILURE;
			}
		}
		else
		{
			memcpy(buf, (msg.start_viraddr+msg.rp), datalen1);
		}

		if (is_kernel == MT_FALSE)
		{
			if (copy_to_user(buf+datalen1, msg.start_viraddr, (cplen-datalen1)))
			{
				MT_ERR_LOG("copy_to_user error\n");
				return MT_FAILURE;
			}
		}
		else
		{
			memcpy((buf+datalen1), msg.start_viraddr, (cplen-datalen1));
		}

		new_rp = cplen - datalen1;
	}

	*copylen = cplen;
	if (new_rp >= msg.size)
		new_rp = 0;

	local_irq_save(flags);
	if (msg.reset_flag == g_msg_bufinfo.reset_flag)
	{
		g_msg_bufinfo.rp = new_rp;
	}
	local_irq_restore((unsigned long)flags);

	return MT_SUCCESS;
}


mt_s32 mt_drv_log_buffer_write(mt_u8 *buf, mt_u32 msglen, mt_u32 is_kernel)
{
	mt_u32 copylen1;
	mt_u32 copylen2;
	mt_u32 new_wp;
	unsigned long flags;

	if (g_msg_bufinfo.size == 0)
	{
		return MT_SUCCESS;
	}

	/*protect with semaphore while two module write at the same time*/
	local_irq_save(flags);

	//down(&g_msg_bufinfo.semWrite);
	if (g_msg_bufinfo.wp < g_msg_bufinfo.rp)
	{
		if ((g_msg_bufinfo.rp - g_msg_bufinfo.wp) < DEBUG_MSG_BUF_RESERVE)
		{
			log_buffer_reset();
		}
	}
	else
	{
		if ((g_msg_bufinfo.wp - g_msg_bufinfo.rp)
			> (g_msg_bufinfo.size - DEBUG_MSG_BUF_RESERVE))
		{
			log_buffer_reset();
		}
	}

	if ((msglen + g_msg_bufinfo.wp) >= g_msg_bufinfo.size)
	{
		copylen1 = g_msg_bufinfo.size - g_msg_bufinfo.wp;
		copylen2 = msglen - copylen1;
		new_wp = copylen2;
	}
	else
	{
		copylen1 = msglen;
		copylen2 = 0;
		new_wp = g_msg_bufinfo.wp + msglen;
	}

	if (copylen1 > 0)
	{
		if (MSG_FROM_KERNEL == is_kernel)
		{
			memcpy((g_msg_bufinfo.wp+g_msg_bufinfo.start_viraddr), buf, copylen1);
		}
		else
		{
			if(copy_from_user((g_msg_bufinfo.wp+g_msg_bufinfo.start_viraddr), buf, copylen1))
			{
				MT_ERR_LOG("copy_from_user error\n");
			}
		}
	}

	if (copylen2 > 0)
	{
		if (MSG_FROM_KERNEL == is_kernel)
		{
			memcpy(g_msg_bufinfo.start_viraddr, (buf+copylen1), copylen2);
		}
		else
		{
			if(copy_from_user(g_msg_bufinfo.start_viraddr, (buf+copylen1), copylen2))
			{
				MT_ERR_LOG("copy_from_user error\n");
			}
		}
	}

	//local_irq_save(flags);
	g_msg_bufinfo.wp = new_wp;

	if (!g_logfileflag)
	{
        wake_up_interruptible(&g_msg_bufinfo.wq_nodata);
	}

	local_irq_restore((unsigned long)flags);
	//up(&g_msg_bufinfo.semWrite);

	return MT_SUCCESS;
}



static mt_s32 log_print_to_buf(const char *format, ...)
{
	char log_str[LOG_MAX_TRACE_LEN] = {0};
	mt_u32 msglen = 0;

	va_list args;

	if (!g_log_modinit)
	{
		MT_ERR_LOG("log device not init(ker)!\n");
		return MT_FAILURE;
	}

	va_start(args, format);
	msglen = mt_osal_vsnprintf(log_str, LOG_MAX_TRACE_LEN-1, format, args);
	va_end(args);

	if (msglen >= (LOG_MAX_TRACE_LEN-1))
	{
        log_str[LOG_MAX_TRACE_LEN-1] = '\0';  /* even the 'vsnprintf' commond will do it */
        log_str[LOG_MAX_TRACE_LEN-2] = '\n';
        log_str[LOG_MAX_TRACE_LEN-3] = '.';
        log_str[LOG_MAX_TRACE_LEN-4] = '.';
        log_str[LOG_MAX_TRACE_LEN-5] = '.';
	}

	return mt_drv_log_buffer_write((mt_u8 *)log_str, msglen, MSG_FROM_KERNEL);
}
#endif

inline mt_u32 log_gettimeMs(mt_void)
{
	struct timespec64 tv;
	ktime_get_real_ts64(&tv);
	return (((mt_u32)tv.tv_sec)*1000+((mt_u32)tv.tv_nsec)/1000000);
}

mt_s32 log_add_module(mt_char* proc_name, mt_mod_id_e item_id)
{
	if (g_log_configinfo == NULL || item_id >= LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s))
	{
		return MT_FAILURE;
	}

	g_log_configinfo[item_id].level = MT_LOG_LEVEL_DEFAULT;
	mt_osal_snprintf((char *)g_log_configinfo[item_id].mod_name, sizeof(g_log_configinfo[item_id].mod_name), proc_name);
	return MT_SUCCESS;
}

mt_s32 log_remove_module(mt_char* proc_name, mt_mod_id_e item_id)
{
	if (g_log_configinfo == NULL || item_id >= LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s))
		return MT_FAILURE;

	g_log_configinfo[item_id].level = MT_LOG_LEVEL_DEFAULT;
	mt_osal_snprintf(g_log_configinfo[item_id].mod_name, sizeof(g_log_configinfo[item_id].mod_name), "Invalid");
	return MT_SUCCESS;
}

/* MT_TRUE: need print ;  MT_FALSE: not print */
static MT_BOOL log_level_check(mt_s32 level, mt_u32 mod_id)
{
	if (mod_id < LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s))
	{
        /* log module has Initialized yet */
		if(g_log_modinit && level <= g_log_configinfo[mod_id].level)
		{
			return MT_TRUE;
		}

        /* log module has not Initialized yet */
		if(!g_log_modinit && level <= MT_LOG_LEVEL_DEFAULT)
		{
			return MT_TRUE;
		}
	}
	return MT_FALSE;
}

/***********************log ops **************************/
/* 0 seria port, 1 network*/
static mt_u32 log_print_position_get(mt_mod_id_e mod_id)
{
	mt_u32 pos = 0;

	if(mod_id >= LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s))
	{
		return LOG_OUTPUT_SERIAL;
	}

	if(!g_log_modinit)
		return LOG_OUTPUT_SERIAL;

	//s32 ret = LOG_FILE_LOCK();

	if(g_logfileflag)
	{
		pos = LOG_OUTPUT_UDISK;
	}
	else
	{
		pos = g_log_configinfo[mod_id].outpos;
	}

	//LOG_FILE_UNLOCK();

	return pos;
}


#ifdef LOG_UDISK_SUPPORT
mt_s32 log_udisk_save(const mt_s8 *filename, mt_s8 *data, mt_u32 datalen)
{
	mt_s32 wlen = 0;
	struct file *fd = NULL;

	fd = mt_drv_file_open(filename, 1);
	if(!fd)
	{
		MT_ERR_LOG("MT_DRV_FILE_Open %s failure.\n", filename);
		return MT_FAILURE;
	}

	wlen = mt_drv_file_write(fd, data, datalen);
	mt_drv_file_close(fd);

	return MT_SUCCESS;
}

#define BUF_SIZE (700)
static struct task_struct *log_udisk_task = NULL;
int log_udisk_write_thread(mt_void *arg)
{
	mt_u8 buf[BUF_SIZE]={0};
	mt_u32 rlen = 0;
	mt_s32 ret = 0;
	mt_u8 file_name[MAX_FILENAME_LENTH]={0};
	MT_BOOL bSetFileFlag = MT_FALSE;

	while (1)
	{
		ret = LOG_FILE_LOCK();
		bSetFileFlag = g_logfileflag;
		mt_osal_snprintf(file_name, sizeof(file_name)-1, "%s/stb.log", (const mt_s8 *)udisk_logfile);
		LOG_FILE_UNLOCK();

		set_current_state(TASK_INTERRUPTIBLE);

		if (kthread_should_stop())
		{
			break;
		}

		if (!bSetFileFlag)
		{
			msleep_interruptible(10);
			continue;
		}

		memset(buf, 0, sizeof(buf));
		ret = mt_drv_log_buffer_read(buf, sizeof(buf)-1, &rlen, MT_TRUE);
		if (ret == MT_SUCCESS)
		{
			log_udisk_save((const mt_s8*)file_name, buf, rlen);
		}

		msleep_interruptible(100);
	}

	return 0;
}



mt_s32 log_udisk_init(const mt_u8 *diskfolder)
{
	int err;

	if (diskfolder == NULL)
	{
		return MT_FAILURE;
	}

	if (log_udisk_task == NULL)
	{
		log_udisk_task = kthread_create(log_udisk_write_thread, (void *)diskfolder, "log_udisk_task");
		if (IS_ERR(log_udisk_task))
		{
			MT_ERR_LOG("create new kernel thread failed\n");
			err = PTR_ERR(log_udisk_task);
			log_udisk_task = NULL;
			return err;
		}
		wake_up_process(log_udisk_task);
	}

	return MT_SUCCESS;
}

mt_s32 log_udisk_exit(mt_void)
{
	if (log_udisk_task)
	{
		kthread_stop(log_udisk_task);
		log_udisk_task = NULL;
	}

	return MT_SUCCESS;
}

#endif

mt_void mt_log_out(mt_u32 level, mt_mod_id_e mod_id, const char *func_name, mt_u32 line_num, const char *format, ...)
{
    va_list args;
    mt_u32  time_ms = 0;
	mt_u32  msglen = 0;
    MT_BOOL blevel = MT_FALSE;
    mt_char log_str[LOG_MAX_TRACE_LEN]={'a'};

    blevel = log_level_check(level, mod_id);
    if (blevel)
    {
        log_str[LOG_MAX_TRACE_LEN-1] = 'b';
        log_str[LOG_MAX_TRACE_LEN-2] = 'c';

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
            int pos = log_print_position_get(mod_id);

            //get time
            time_ms = log_gettimeMs();

            switch(pos)
            {
                case LOG_OUTPUT_SERIAL:
                if ( (mod_id != MT_ID_VSYNC) && (mod_id != MT_ID_ASYNC) )
                {
                    MT_PRINT(PRINT_FMT);
                }
                break;
                case LOG_OUTPUT_NETWORK:
                case LOG_OUTPUT_UDISK:
#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
                    log_print_to_buf(PRINT_FMT);
#endif
                break;
            }

        }
        else /* log module has not Initialized. */
        {
            MT_PRINT("[%s-Unknow]: %s[%d]:%s", debuglevel_name[level], func_name, line_num, log_str);
        }
    }

}

/***********************proc ops **************************/

#if 0
static int search_in_array(char *array[], int array_size, char *s)
{
    int i = 0;
    char **p = array;
    if (!array || !s )
        return -1;
    while(i++ < array_size){
        if (!*p)
            continue;
        if (!strcmp(*p++,s))
            return i - 1;
    }
    return -1;
}
#endif

static int search_mod(char *s)
{
	int i = 0;
	int cnt = LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s);

	for (i = 0; i < cnt; i++){
		if (!mt_osal_strncasecmp(g_log_configinfo[i].mod_name, s, sizeof(g_log_configinfo[i].mod_name)))
			return i;
	}
	return -1;
}

static char *strip_string(char *s, char *d)
{
	char *p = d;
	do{
		if (*s == '\n')
			*s = '\0';
		if (*s != ' ')
			*p++ = *s;
	}while(*s++ != '\0');

	return d;
}


static int seperate_string(char *s, char **left, char **right)
{
	char *p = s;
    /* find '=' */
	while(*p != '\0' && *p++ != '=');

	if(*--p != '=')
		return -1;

    /* seperate left from right vaule by '=' */
	*p = '\0';
 	*left = s;
 	*right = p+1;

	return 0;
}

mt_s32 mt_drv_log_proc_read(struct seq_file *s, mt_void *arg)
{
	mt_u32 i = 0;
	mt_u8 level = 0;
	mt_u32 total = LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s);

	if(!g_log_modinit)
	{
		PROC_PRINT(s, "Log module not init \n");
		return 0;
	}

	PROC_PRINT(s, "--------------Log path --------------------\n");
	PROC_PRINT(s, "Log path:     %s \n", udisk_logfile);

	PROC_PRINT(s, "--------------Store path ------------------\n");
	PROC_PRINT(s, "Store path:     %s \n", store_path);

	PROC_PRINT(s, "--------------Module Log Level--------------\n");

	PROC_PRINT(s, "Log module\t  level\n");
	PROC_PRINT(s, "--------------------------------------------\n");
	//PROC_PRINT(s, "Log module not init \n");

	for(i = 0; i < total; i++)
	{
		if (mt_osal_strncmp(g_log_configinfo[i].mod_name, "Invalid", 8))
		{
			level = g_log_configinfo[i].level;
			PROC_PRINT(s, "%-16s %d(%s)\n",
							g_log_configinfo[i].mod_name, level, debuglevel_name[level]);
		}
	}

	PROC_PRINT(s, "\necho mt_avplay=2 > /proc/msp/log\n");
	PROC_PRINT(s, "echo log=/mnt > /proc/msp/log\n");
	PROC_PRINT(s, "echo storepath=/mnt > /proc/msp/log\n");

	return 0;
}


mt_s32 mt_drv_log_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char m[MAX_FILENAME_LENTH] = {0};
	char d[MAX_FILENAME_LENTH] = {0};
	size_t len = MAX_FILENAME_LENTH;
	char *left, *right;
	int idx, level;
	int ret = 0;

	if (*ppos >= MAX_FILENAME_LENTH)
		return -EFBIG;

	len = min(len, count);
	if (copy_from_user(m, buf, len))
		return -EFAULT;


	if (!g_log_modinit)
	{
		MT_ERR_LOG("Log module not init\n");
		goto out;
	}

	strip_string(m, d);
    /* echo help info to current terinmal */
	if (!mt_osal_strncasecmp("help", m, 4))
	{
#if 1
		mt_drv_proc_echohelp("To modify the level, use command line in shell: \n");
        mt_drv_proc_echohelp("    echo module_name = level_number > /proc/msp/log\n");
        mt_drv_proc_echohelp("    level_number: 0-fatal, 1-error, 2-warning, 3-info\n");
        mt_drv_proc_echohelp("    example: 'echo MT_DEMUX=3 > /proc/msp/log'\n");
        mt_drv_proc_echohelp("    will change log levle of module \"MT_DEMUX\" to 3, then, \n");
        mt_drv_proc_echohelp("all message with level higher than \"info\" will be printed.\n");
        mt_drv_proc_echohelp("Use 'echo \"all = x\" > /proc/msp/log' to change all modules.\n");

        mt_drv_proc_echohelp("\n\nTo modify the log path, use command line in shell: \n");
        mt_drv_proc_echohelp("Use 'echo \"log = x\" > /proc/msp/log' to set log path.\n");
        mt_drv_proc_echohelp("Use 'echo \"log = /dev/null\" > /proc/msp/log' to close log udisk output.\n");
        mt_drv_proc_echohelp("    example: 'echo log=/home > /proc/msp/log'\n");

        mt_drv_proc_echohelp("\n\nTo modify the debug file store path, use command line in shell: \n");
        mt_drv_proc_echohelp("Use 'echo \"storepath = x\" > /proc/msp/log' to set debug file path.\n");
        mt_drv_proc_echohelp("    example: 'echo storepath=/tmp > /proc/msp/log'\n");
#endif
	}


	if (seperate_string(d, &left, &right)){
        MT_WARN_LOG("string is unkown!\n");
		goto out;
	}

	if (!mt_osal_strncasecmp("log", left, 4))
	{
		if (strlen(right) >= sizeof(g_pathbuf))
		{
			MT_ERR_LOG("log path length is over than %d!\n",sizeof(g_pathbuf));
			goto out;
		}

		ret = LOG_FILE_LOCK();
		memset(g_pathbuf, 0, sizeof(g_pathbuf));
		memcpy(g_pathbuf, right, strlen(right));
		if (memcmp(g_pathbuf, "/dev/null", strlen("/dev/null")) != 0)
		{
			g_logfileflag = MT_TRUE;
		}
		else
		{
			g_logfileflag = MT_FALSE;
		}

		g_log_configinfo->udisk_flag = (mt_u8)g_logfileflag;

		udisk_logfile = g_pathbuf;
		LOG_FILE_UNLOCK();
		MT_INFO_LOG(" set log path is g_pathbuf = %s\n", udisk_logfile);
		goto out;
	}
	else if (!mt_osal_strncasecmp("storepath", left, strlen("storepath")+1))
	{
		if (strlen(right) >= sizeof(g_storepath_buf))
		{
			MT_ERR_LOG("store path length is over than %d\n", sizeof(g_storepath_buf));
			goto out;
		}

		ret = LOG_FILE_LOCK();
		memset(g_storepath_buf, 0, sizeof(g_storepath_buf));
		memcpy(g_storepath_buf, right, strlen(right));

		store_path = g_storepath_buf;

		LOG_FILE_UNLOCK();
		MT_INFO_LOG(" set store path is g_szStorePathBuf = %s\n", g_storepath_buf);
		goto out;
	}
	else
	{
		level = simple_strtol(right, NULL, 10);
		if (!level && *right != '0')
		{
			MT_WARN_LOG("invalid value\n");
			goto out;
		}

		if (!mt_osal_strncasecmp("all", left, 4))
		{
			mt_u32 i = 0;
			mt_u32 total = LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s);
			for (i = 0; i < total; i++)
				g_log_configinfo[i].level = (mt_u8)level;
			goto out;
		}

		idx = search_mod(left);
		if (idx == -1)
		{
			MT_WARN_LOG("%s not found in array\n", left);
			return count;
		}
        g_log_configinfo[idx].level = (mt_u8)level;
	}

	MT_PRINT("\n\t module %s level change to %d(%s)\n",
		g_log_configinfo[idx].mod_name, level, debuglevel_name[level]);

out:
	*ppos = len;
	return len;
#undef TMP_BUF_LEN
}


mt_s32 mt_drv_log_set_path(log_path_s *logpath)
{
	int ret = 0;

	if (logpath == NULL || logpath->pathlen > sizeof(g_pathbuf))
	{
		return MT_FAILURE;
	}

	ret = LOG_FILE_LOCK();
	memset(g_pathbuf, 0, sizeof(g_pathbuf));
	ret = copy_from_user(g_pathbuf, logpath->path, logpath->pathlen);
	if (ret != 0)
	{
		LOG_FILE_UNLOCK();
		return MT_FAILURE;
	}

	if (memcmp(logpath->path, "/dev/null", strlen("/dev/null")) == 0)
	{
		g_logfileflag = MT_FALSE;
	}
	else
	{
		g_logfileflag = MT_TRUE;
	}

	g_log_configinfo->udisk_flag = (mt_u8)g_logfileflag;

	udisk_logfile = g_pathbuf;
	LOG_FILE_UNLOCK();

	return MT_SUCCESS;
}

mt_s32 drv_log_get_path(mt_s8 *buf, mt_u32 len)
{
	mt_s32 pathlen = 0;
	if (udisk_logfile == NULL)
	{
		return MT_FAILURE;
	}

	pathlen = strlen(udisk_logfile) + 1;

	if (pathlen > len || len <= 1)
	{
		return MT_FAILURE;
	}

	memcpy(buf, udisk_logfile, pathlen);
	return MT_SUCCESS;
}

mt_s32 mt_drv_log_set_storepath(store_path_s *storepath)
{
	int ret = 0;

	if (storepath == NULL || storepath->pathlen > sizeof(g_storepath_buf))
	{
		return MT_FAILURE;
	}

	ret = LOG_FILE_LOCK();
	memset(g_storepath_buf, 0, sizeof(g_storepath_buf));
	ret = copy_from_user(g_storepath_buf, storepath->path, storepath->pathlen);
	if (ret != 0)
	{
		LOG_FILE_UNLOCK();
		return MT_FAILURE;
	}

	store_path = g_storepath_buf;
	LOG_FILE_UNLOCK();

	return MT_SUCCESS;
}

mt_s32 mt_drv_log_get_storepath(mt_s8 *buf, mt_u32 len)
{
	mt_s32 pathlen = 0;
	if (store_path == NULL)
	{
		return MT_FAILURE;
	}

	pathlen = strlen(store_path)+1;

	if (pathlen > len || len <= 1)
	{
		return MT_FAILURE;
	}

	memcpy(buf, store_path, pathlen);
	return MT_SUCCESS;
}



/***********************module ops **************************/

static mt_s32 log_config_info_init(mt_void)
{
    mt_u32 i;

    if (MT_SUCCESS == mt_drv_mmz_alloc_and_map("CMN_LogInfo", MMZ_OTHERS,
            LOG_CONFIG_BUF_SIZE, 0, &g_log_configbuf))
    {
        memset((void *)g_log_configbuf.startVirAddr, 0, LOG_CONFIG_BUF_SIZE);
        g_log_configinfo = (log_config_info_s *)g_log_configbuf.startVirAddr;

        g_log_configinfo->udisk_flag = 0;

        /* max debug module number: 8192/28 = 341/292 */
        for (i = 0; i < LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s); i++)
        {
            g_log_configinfo[i].level = MT_LOG_LEVEL_DEFAULT;
            g_log_configinfo[i].outpos = LOG_OUTPUT_SERIAL;
            mt_osal_snprintf(g_log_configinfo[i].mod_name, sizeof(g_log_configinfo[i].mod_name), "Invalid");
        }
    }
    else
    {
        MT_ERR_LOG("calling mt_drv_mmz_alloc_and_map failed.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 mt_log_config_info_exit(mt_void)
{
    if (0 != g_log_configbuf.startVirAddr)
    {
        g_log_configinfo = NULL;
        mt_drv_mmz_unmap_and_release(&g_log_configbuf);
    }

    return MT_SUCCESS;
}

#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
static mt_s32 log_buffer_alloc(mt_u32 buf_size)
{
    mmz_buffer_s mmzbuf;
    memset(&g_msg_bufinfo, 0, sizeof(g_msg_bufinfo));
    g_msg_bufinfo.size = buf_size;
    init_waitqueue_head(&(g_msg_bufinfo.wq_nodata));
    //init_MUTEX(&g_MsgBufInfo.semWrite);

    /*alloc buffer*/
    if (buf_size > 0)
    {
        if (buf_size < DEBUG_MSG_BUF_RESERVE)
        {
            MT_ERR_LOG("Error: msp_debug device init failure, buffer too small, at least:%d!\n",
                        DEBUG_MSG_BUF_RESERVE);
            return MT_FAILURE;
        }

        /*init memory*/
        if (MT_SUCCESS == mt_drv_mmz_alloc_and_map("CMN_LogTrace", MMZ_OTHERS, buf_size, 0, &mmzbuf))
        {
            g_msg_bufinfo.start_phyaddr = mmzbuf.startPhyAddr;
            g_msg_bufinfo.start_viraddr = (mt_u8 *)mmzbuf.startVirAddr;
        }
        else
        {
            MT_ERR_LOG("Init failed, there is not enough mem for debug message buffer.\n");
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

static mt_s32 log_buffer_free(mt_void)
{
    mmz_buffer_s mmzBuf;
    if (NULL != g_msg_bufinfo.start_viraddr)
    {
        mmzBuf.startPhyAddr = g_msg_bufinfo.start_phyaddr;
        mmzBuf.startVirAddr = (ulong)g_msg_bufinfo.start_viraddr;
        mt_drv_mmz_unmap_and_release(&mmzBuf);
        g_msg_bufinfo.start_viraddr = NULL;
        g_msg_bufinfo.start_phyaddr = 0;
    }

    return MT_SUCCESS;
}
#endif

mt_void mt_drv_log_config_bufaddr(ulong *addr)
{
	if (g_log_modinit)
		*addr = g_log_configbuf.startPhyAddr;
	else
		*addr = 0;
	return;
}

mt_s32 log_getlevel(mt_u32 mod_id, mt_u8* buf, mt_u32 len)
{
    mt_u32 level = 0;

    if (mod_id >= LOG_CONFIG_BUF_SIZE/sizeof(log_config_info_s))
    {
        return 0;
    }

    if (g_log_modinit == 0)
    {
        return 0;
    }

    level = g_log_configinfo[mod_id].level;

    memset(buf, 0, len);
    if (len <= strlen(debuglevel_name[level]) )
    {
        len = strlen(debuglevel_name[level]) - 1;
    }

    memcpy(buf, debuglevel_name[level], len);

    return level;
}

mt_s32 mt_drv_log_kinit(mt_void)
{
MT_INFO_LOG("====mt_drv_log_kinit=====000=======\n");
	if (log_config_info_init() != MT_SUCCESS)
	{
		return -1;
	}
MT_INFO_LOG("====mt_drv_log_kinit=====111=======\n");
#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
	if (log_bufsize < DEBUG_MSG_BUF_RESERVE)
		log_bufsize = DEBUG_MSG_BUF_RESERVE;
MT_INFO_LOG("====mt_drv_log_kinit=====222=======\n");
	if (log_buffer_alloc(log_bufsize) != MT_SUCCESS)
	{
		mt_log_config_info_exit();
		return -1;
	}
#endif
MT_INFO_LOG("====mt_drv_log_kinit=====333=======\n");
#ifdef LOG_UDISK_SUPPORT
	log_udisk_init(udisk_logfile);
#endif

	log_add_module("ASYNC", MT_ID_ASYNC);
	log_add_module("VSYNC", MT_ID_VSYNC);
	log_add_module("MT_IR", MT_ID_IR);

	g_log_modinit = 1;

	return 0;
}

mt_void mt_drv_log_kexit(mt_void)
{
    g_log_modinit = 0;

#ifdef LOG_UDISK_SUPPORT
	log_udisk_exit();
#endif

#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
	log_buffer_free();
#endif

	mt_log_config_info_exit();
}

#ifndef MODULE
/* Legacy boot options - nonmodular */
static int __init get_logbufsize(char *str)
{
	log_bufsize = simple_strtol(str, NULL, 0);
	MT_INFO_LOG("log_bufsize = 0x%x\n", log_bufsize);
	return 1;
}

__setup("log_bufsize=", get_logbufsize);
#endif


EXPORT_SYMBOL(mt_drv_log_proc_read);
EXPORT_SYMBOL(mt_drv_log_proc_write);

#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
EXPORT_SYMBOL(mt_drv_log_buffer_read);
EXPORT_SYMBOL(mt_drv_log_buffer_write);
#endif

module_param(log_bufsize, int, S_IRUGO);
EXPORT_SYMBOL(mt_drv_log_config_bufaddr);
module_param(udisk_logfile, charp, S_IRUGO);
module_param(store_path, charp, S_IRUGO);

EXPORT_SYMBOL(store_path);
EXPORT_SYMBOL(mt_drv_log_get_storepath);


EXPORT_SYMBOL(mt_log_out);






