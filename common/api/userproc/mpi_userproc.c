/******************************************************************************/


/******************************* ************** *******************************/

/* Sys headers */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include "list.h"

/* Unf headers */

/* Drv headers */
#include "mt_drv_memdev.h"

/* Local headers */
#include "mapi_proc.h"
#include "mt_mpi_mem.h"
#include "mt_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#if !(0 == MT_PROC_SUPPORT)

/***************************** Macro Definition ******************************/

#define UPROC_LOCK(Mutex)        pthread_mutex_lock(&Mutex)
#define UPROC_UNLOCK(Mutex)      pthread_mutex_unlock(&Mutex)

#define MT_PROC_MAX_PARAM_NUM (8)
#define MT_PROC_MAX_PARAM_LEN (128)

#define UPROC_CHECK_INIT \
    UPROC_LOCK(g_stUprocParam.stMutex); \
    if (-1 == g_stUprocParam.s32Fd) \
    { \
        UPROC_UNLOCK(g_stUprocParam.stMutex); \
        PROC_ERR("USERPROC not init!\n"); \
        return MT_FAILURE; \
    } \
    UPROC_UNLOCK(g_stUprocParam.stMutex);


/*************************** Structure Definition ****************************/

typedef struct taguser_proc_param_t
{
    mt_s32      s32Fd;
    MT_BOOL     bThreadStop;
    pthread_t   stThread;
    pthread_mutex_t stMutex;
    mt_u8       u8CmdBuf[MT_PROC_MAX_PARAM_NUM][MT_PROC_MAX_PARAM_LEN];
    mt_u8*      apu8Cmd[MT_PROC_MAX_PARAM_NUM];
}user_proc_param_t;


/***************************** Global Definition *****************************/


/***************************** Static Definition *****************************/

static user_proc_param_t g_stUprocParam =
{
    .s32Fd = -1,
    .stMutex = PTHREAD_MUTEX_INITIALIZER,
    .apu8Cmd = {
        g_stUprocParam.u8CmdBuf[0], g_stUprocParam.u8CmdBuf[1],
        g_stUprocParam.u8CmdBuf[2], g_stUprocParam.u8CmdBuf[3],
        g_stUprocParam.u8CmdBuf[4], g_stUprocParam.u8CmdBuf[5],
        g_stUprocParam.u8CmdBuf[6], g_stUprocParam.u8CmdBuf[7]
    }
};

static mt_void* mt_api_proc_thread(mt_void*);

/*********************************** Code ************************************/
mt_s32 mt_api_proc_init(mt_void)
{
    mt_s32 s32Ret;

    UPROC_LOCK(g_stUprocParam.stMutex);

    if (-1 == g_stUprocParam.s32Fd)
    {
        g_stUprocParam.s32Fd = open("/dev/"MT_USERPROC_DEVNAME, O_RDWR | O_CLOEXEC);
        if (0 >= g_stUprocParam.s32Fd)
        {
            UPROC_UNLOCK(g_stUprocParam.stMutex);
            PROC_FATAL("Open %s err!\n", MT_USERPROC_DEVNAME);
            return MT_FAILURE;
        }

        g_stUprocParam.bThreadStop = MT_FALSE;
        s32Ret = pthread_create(&g_stUprocParam.stThread, MT_NULL, mt_api_proc_thread, MT_NULL);
        if (MT_SUCCESS != s32Ret)
        {
            (mt_void)close(g_stUprocParam.s32Fd);
            g_stUprocParam.s32Fd = -1;
            UPROC_UNLOCK(g_stUprocParam.stMutex);
            PROC_FATAL("Create userproc thread err!\n");
            return MT_FAILURE;
        }
    }

    UPROC_UNLOCK(g_stUprocParam.stMutex);

    return MT_SUCCESS;
}

mt_s32 mt_api_proc_deinit(mt_void)
{
    UPROC_LOCK(g_stUprocParam.stMutex);

    if (-1 != g_stUprocParam.s32Fd)
    {
        g_stUprocParam.bThreadStop = MT_TRUE;
        (mt_void)pthread_join(g_stUprocParam.stThread, MT_NULL);

        (mt_void)close(g_stUprocParam.s32Fd);
        g_stUprocParam.s32Fd = -1;
    }

    UPROC_UNLOCK(g_stUprocParam.stMutex);

    return MT_SUCCESS;
}

mt_s32 mt_api_proc_add_dir(const mt_char *pszName)
{
    mt_s32 s32Ret = MT_FAILURE;

    mt_proc_dir_name_t stName = {0};

    if ((MT_NULL == pszName) || (strlen(pszName) == 0) || (strlen(pszName) > MAX_PROC_NAME_LEN))
    {
        PROC_ERR("Invalid dir name!\n");
        return MT_FAILURE;
    }

    UPROC_CHECK_INIT;

    //memcpy(stName, pszName, sizeof(stName)-1);
    strncpy((char*)stName, (const char *)pszName, sizeof(stName)-1);
    s32Ret = ioctl(g_stUprocParam.s32Fd, UMPIOC_ADD_DIR, stName);
    if (MT_SUCCESS != s32Ret)
    {
        PROC_ERR("proc created dir %s fail.\n", pszName);
    }
    else
    {
        PROC_INFO("proc created dir %s success.\n", pszName);
    }

    return s32Ret;
}

mt_s32 mt_api_proc_rm_dir(const mt_char *pszName)
{
    mt_s32 s32Ret = MT_FAILURE;

    mt_proc_dir_name_t stName = {0};

    if ((MT_NULL == pszName) || (strlen(pszName) == 0) || (strlen(pszName) > MAX_PROC_NAME_LEN))
    {
        PROC_ERR("Invalid dir name!\n");
        return MT_FAILURE;
    }

    UPROC_CHECK_INIT;

    memcpy(stName, pszName, sizeof(stName)-1);

    s32Ret = ioctl(g_stUprocParam.s32Fd, UMPIOC_REMOVE_DIR, stName);
    if (MT_SUCCESS != s32Ret)
    {
        PROC_ERR("proc remove dir %s fail.\n", pszName);
    }
    else
    {
        PROC_INFO("proc remove dir %s success.\n", pszName);
    }

    return s32Ret;
}

mt_s32 mt_api_proc_add_entry(mt_u32 u_id, const mt_u_proc_entry_s* pst_entry)
{
    mt_s32 s32Ret = MT_FAILURE;

    mt_drv_u_proc_entry_t stEntry;

    if ((MT_INVALID_MODULE_ID == u_id) || (MT_NULL == pst_entry))
    {
        PROC_ERR("Invalid param!\n");
        return MT_FAILURE;
    }

    if ((MT_NULL == pst_entry->pszEntryName) ||
        (0 == strlen(pst_entry->pszEntryName)) || (MAX_PROC_NAME_LEN < strlen(pst_entry->pszEntryName)))
    {
        PROC_ERR("Invalid entry name!\n");
        return MT_FAILURE;
    }

    if ((MT_NULL != pst_entry->pszDirectory) &&
        ((0 == strlen(pst_entry->pszDirectory)) || (MAX_PROC_NAME_LEN < strlen(pst_entry->pszDirectory))))
    {
        PROC_ERR("Invalid dir name!\n");
        return MT_FAILURE;
    }

    if ((MT_NULL == pst_entry->pfnShowProc) && (MT_NULL == pst_entry->pfnCmdProc))
    {
        PROC_ERR("Need one valid func at least!\n");
        return MT_FAILURE;
    }

    UPROC_CHECK_INIT;

    memset(&stEntry, 0, sizeof(stEntry));
    mt_osal_strncpy(stEntry.aszName, pst_entry->pszEntryName, sizeof(stEntry.aszName)-1);
    if (MT_NULL != pst_entry->pszDirectory)
    {
        mt_osal_strncpy(stEntry.aszParent, pst_entry->pszDirectory, sizeof(stEntry.aszParent)-1);
    }
    stEntry.pfnShowFunc = pst_entry->pfnShowProc;
    stEntry.pfnCmdFunc = pst_entry->pfnCmdProc;
    stEntry.pPrivData   = pst_entry->pPrivData;

    s32Ret = ioctl(g_stUprocParam.s32Fd, UMPIOC_ADD_ENTRY, &stEntry);
    if (MT_SUCCESS != s32Ret)
    {
        PROC_ERR("proc created entry %s fail:%d.\n", pst_entry->pszEntryName, s32Ret);
    }
    else
    {
        PROC_INFO("proc created entry %s success.\n", pst_entry->pszEntryName);
    }

    return s32Ret;
}

mt_s32 mt_api_proc_rm_entry(mt_u32 u_id, const mt_u_proc_entry_s* pst_entry)
{
    mt_s32 s32Ret = MT_FAILURE;

    mt_drv_u_proc_entry_t stEntry;

    if ((MT_INVALID_MODULE_ID == u_id) || (MT_NULL == pst_entry))
    {
        PROC_ERR("Invalid param!\n");
        return MT_FAILURE;
    }

    if ((MT_NULL == pst_entry->pszEntryName) ||
        (0 == strlen(pst_entry->pszEntryName)) || (MAX_PROC_NAME_LEN < strlen(pst_entry->pszEntryName)))
    {
        PROC_ERR("Invalid entry name!\n");
        return MT_FAILURE;
    }

    if ((MT_NULL != pst_entry->pszDirectory) &&
        ((0 == strlen(pst_entry->pszDirectory)) || (MAX_PROC_NAME_LEN < strlen(pst_entry->pszDirectory))))
    {
        PROC_ERR("Invalid dir name!\n");
        return MT_FAILURE;
    }

    UPROC_CHECK_INIT;

    memset(&stEntry, 0, sizeof(stEntry));
    mt_osal_strncpy(stEntry.aszName, pst_entry->pszEntryName, sizeof(stEntry.aszName)-1);
    if (MT_NULL != pst_entry->pszDirectory)
    {
        mt_osal_strncpy(stEntry.aszParent, pst_entry->pszDirectory, sizeof(stEntry.aszParent)-1);
    }

    s32Ret = ioctl(g_stUprocParam.s32Fd, UMPIOC_REMOVE_ENTRY, &stEntry);
    if (MT_SUCCESS != s32Ret)
    {
        PROC_ERR("proc remove entry %s fail:%d.\n", pst_entry->pszEntryName, s32Ret);
    }
    else
    {
        PROC_INFO("proc remove entry %s success.\n", pst_entry->pszEntryName);
    }

    return s32Ret;
}

static mt_u32 mt_api_proc_parse_cmd(const mt_char* pu8Cmd)
{
    const mt_char* p = pu8Cmd;
    mt_u32 i = 0, j = 0;

    memset(g_stUprocParam.u8CmdBuf, 0, sizeof(g_stUprocParam.u8CmdBuf));
    while (*p)
    {
        if (*p == ' ')
        {
            if (j !=0 )
            {
                i++;
                j = 0;
            }
            if (i >= MT_PROC_MAX_PARAM_NUM)
            {
                i--;
                break;
            }
        }
        else
        {
            if (j<MT_PROC_MAX_PARAM_LEN-1)
            {
                g_stUprocParam.u8CmdBuf[i][j++] = (mt_u8)*p;
            }
        }
        p++;
    }

    return i+1;
}

static mt_void* mt_api_proc_thread(mt_void* pParam)
{
    mt_s32 s32Ret;
    mt_drv_u_proc_cmdinfo_t stCMD;
    char threadname[64]={0};

    mt_proc_show_buffer_s stBuffer;

    strcpy(threadname, __FUNCTION__);
    mt_set_pthread_name(threadname);
    while (!g_stUprocParam.bThreadStop)
    {
        memset(&stCMD, 0, sizeof(mt_drv_u_proc_cmd_t));
        s32Ret = ioctl(g_stUprocParam.s32Fd, UMPIOC_GETCMD, &stCMD);

        if (MT_SUCCESS != s32Ret)
        {
            MT_USLEEP(10*1000);
            PROC_WARN("proc GetCMD fail.\n");
            continue;
        }

        if (strlen(stCMD.stCmd.aszCmd) > 0)
        {
            PROC_INFO("proc GetCMD success: %s.\n", stCMD.stCmd.aszCmd);

            stBuffer.pu8Buf = (mt_u8*)mt_mem_map(stCMD.stEntry.stBuf.startPhyAddr,
                                                                    stCMD.stEntry.stBuf.size);

            if (MT_NULL != stBuffer.pu8Buf)
            {
                stBuffer.offset = 0;
                stBuffer.size = stCMD.stEntry.stBuf.size;

                memset(stBuffer.pu8Buf, 0, stBuffer.size);

                /* read */
                if (0 == mt_osal_strncmp(MT_UPROC_READ_CMD, stCMD.stCmd.aszCmd, strlen(MT_UPROC_READ_CMD)+1))
                {
                    stCMD.stEntry.pfnShowFunc(&stBuffer, stCMD.stEntry.pPrivData);

                     if (MT_SUCCESS != ioctl(g_stUprocParam.s32Fd, UMPIOC_WAKE_READ_TASK, 0))
                    {
                        PROC_ERR("proc print fail.\n");
                     }
                }
                else /* write */
                {
                    stCMD.stEntry.pfnCmdFunc(&stBuffer,
                                mt_api_proc_parse_cmd(stCMD.stCmd.aszCmd),
                                g_stUprocParam.apu8Cmd,
                                stCMD.stEntry.pPrivData);

                     if (MT_SUCCESS != ioctl(g_stUprocParam.s32Fd, UMPIOC_WAKE_WRITE_TASK, 0))
                     {
                         PROC_ERR("proc print fail.\n");
                     }
                }

                mt_mem_unmap(stBuffer.pu8Buf);
            }
            else
            {
                PROC_ERR("map usrproc log buffer failed.\n");
            }
        }

        MT_USLEEP(10*1000);
    }

    return MT_NULL;
}

#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */

