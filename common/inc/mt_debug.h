/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
#ifndef __MT_DEBUG_H__
#define __MT_DEBUG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_type.h"
#include "mt_module.h"


#if defined(MT_LOG_SUPPORT) && (0 == MT_LOG_SUPPORT)
#undef MT_DEBUG

#define MT_PANIC(fmt...)	do{\
}while(0)

#define MT_PRINT(fmt...)	do{\
}while(0)
#else
#define MT_DEBUG
#ifdef __KERNEL__

#define MT_PRINT printk
#define MT_PANIC printk
#else
#include <stdio.h>
#define MT_PRINT printf
#define MT_PANIC printf
#endif

#endif

/*************************** Structure Definition ****************************/
/** \addtogroup     MT_DEBUG */
/** @{ */  /** <!-- [MT_DEBUG] */


/**Default level of the output debugging information*/
/**CNcomment: 默认的调试信息输出级别*/
#define MT_LOG_LEVEL_DEFAULT MT_LOG_LEVEL_ERROR

/**Level of the output debugging information*/
/**CNcomment: 调试信息输出级别*/
typedef enum mt_log_level
{
    MT_LOG_LEVEL_FATAL   = 0,     /**<Fatal error. It indicates that a critical problem occurs in the system. Therefore, you must pay attention to it.*/

    MT_LOG_LEVEL_ERROR   = 1,     /**<Major error. It indicates that a major problem occurs in the system and the system cannot run.*/

    MT_LOG_LEVEL_WARNING = 2,     /**<Warning. It indicates that a minor problem occurs in the system, but the system still can run properly.*/

    MT_LOG_LEVEL_INFO    = 3,     /**<Message. It is used to prompt users. Users can open the message when locating problems. It is recommended to disable this message in general.*/

    MT_LOG_LEVEL_DBG     = 4,     /**<Debug. It is used to prompt developers. Developers can open the message when locating problems. It is recommended to disable this message in general.*/

    MT_LOG_LEVEL_BUTT
} mt_log_level_e;

#ifndef MT_LOG_LEVEL_E
#define MT_LOG_LEVEL_E mt_log_level_e
#endif
/** @} */

/**Just only for fatal level print.   */   /**CNcomment: 为了打印致命信息而制定的宏打印级别 */
#define MT_TRACE_LEVEL_FATAL    (0)
/**Just only for error level print.   */   /**CNcomment: 为了打印错误信息而制定的宏打印级别 */
#define MT_TRACE_LEVEL_ERROR    (1)
/**Just only for warning level print. */   /**CNcomment: 为了打印警告信息而制定的宏打印级别 */
#define MT_TRACE_LEVEL_WARN     (2)
/**Just only for info level print.    */   /**CNcomment: 为了打印信息级别而制定的宏打印级别 */
#define MT_TRACE_LEVEL_INFO     (3)
/**Just only for debug level print.   */   /**CNcomment: 为了打印调试信息而制定的宏打印级别 */
#define MT_TRACE_LEVEL_DBG      (4)

#ifndef MT_LOG_LEVEL
#define MT_LOG_LEVEL         (MT_TRACE_LEVEL_INFO)
#endif


/**Just only debug output,MUST BE NOT calling it. */
/**CNcomment: 调试输出信息接口，不推荐直接调用此接口 */
extern mt_void mt_log_out(mt_u32 level, mt_mod_id_e mod_id,
			const char *func_name, mt_u32 line_num, const char *format, ...);

#ifdef MT_DEBUG

#define MT_TRACE(level, module_id, fmt...)                  	 \
	do{                                                      	 \
		mt_log_out(level, module_id,__FUNCTION__,__LINE__,fmt);  \
	}while(0)

#ifndef MT_ASSERT
#define MT_ASSERT(expr)\
	do{\
		if(!(expr)){\
			MT_PANIC("\nASSERT failed at:\n  >File name: %s\n  >Function: %s\n  >Line No. : %d\n  >Condition: %s\n",\
				__FILE__,__FUNCTION__,__LINE__,#expr);\
		}\
	}while(0)
#endif
#define MT_ASSERT_RET(expr)\
	do{\
		if(!(expr)){\
			MT_PRINT("\n<%s %d>:ASSERT Failure {" #expr "}\n",\
							__FUNCTION__,__LINE__);\
			return MT_FAILURE;\
		}\
	}while(0)

#define MT_ERROR_LOG(fmt...) \
		MT_TRACE(MT_LOG_LEVEL_ERROR, MT_ID_SYS, fmt)

#define MT_DEBUG_LOG(fmt...) \
		MT_TRACE(MT_LOG_LEVEL_INFO, MT_ID_SYS, fmt)


/**Supported for debug output to serial/network/u-disk. */
/**CNcomment: 各个模块需要调用以下宏进行输出调试信息、可输出到串口、网口、U盘存储等 */
/**Just only reserve the fatal level output. */
/**CNcomment: 仅仅保留致命的调试信息 */
#if (MT_LOG_LEVEL == MT_TRACE_LEVEL_FATAL)
#define MT_FATAL_PRINT(module_id, fmt...) 		MT_TRACE(MT_TRACE_LEVEL_FATAL, 	module_id, fmt)
#define MT_ERR_PRINT(module_id, fmt...)
#define MT_WARN_PRINT(module_id, fmt...)
#define MT_INFO_PRINT(module_id, fmt...)
#define MT_DBG_PRINT(module_id, fmt...)
/**Just only reserve the fatal/error level output. */
/**CNcomment: 仅仅保留致命的和错误级别的调试信息 */
#elif (MT_LOG_LEVEL == MT_TRACE_LEVEL_ERROR)
#define MT_FATAL_PRINT(module_id, fmt...) 		MT_TRACE(MT_TRACE_LEVEL_FATAL, 	module_id, fmt)
#define MT_ERR_PRINT(module_id, fmt...)			MT_TRACE(MT_TRACE_LEVEL_ERROR, 	module_id, fmt)
#define MT_WARN_PRINT(module_id, fmt...)
#define MT_INFO_PRINT(module_id, fmt...)
#define MT_DBG_PRINT(module_id, fmt...)
/**Just only reserve the fatal/error/warning level output. */
/**CNcomment: 仅仅保留致命的、错误的、警告级别的调试信息 */
#elif (MT_LOG_LEVEL == MT_TRACE_LEVEL_WARN)
#define MT_FATAL_PRINT(module_id, fmt...) 		MT_TRACE(MT_TRACE_LEVEL_FATAL, 		module_id, fmt)
#define MT_ERR_PRINT(module_id, fmt...)			MT_TRACE(MT_TRACE_LEVEL_ERROR, 		module_id, fmt)
#define MT_WARN_PRINT(module_id, fmt...)		MT_TRACE(MT_TRACE_LEVEL_WARN, 		module_id, fmt)
#define MT_INFO_PRINT(module_id, fmt...)
#define MT_DBG_PRINT(module_id, fmt...)
/**Just only reserve the fatal/error/warning/info level output. */
/**CNcomment: 仅仅保留致命的、错误的、警告和信息级别的调试信息 */
#elif (MT_LOG_LEVEL == MT_TRACE_LEVEL_INFO)
#define MT_FATAL_PRINT(module_id, fmt...) 		MT_TRACE(MT_TRACE_LEVEL_FATAL, 		module_id, fmt)
#define MT_ERR_PRINT(module_id, fmt...)		    MT_TRACE(MT_TRACE_LEVEL_ERROR, 		module_id, fmt)
#define MT_WARN_PRINT(module_id, fmt...)		MT_TRACE(MT_TRACE_LEVEL_WARN, 		module_id, fmt)
#define MT_INFO_PRINT(module_id, fmt...)		MT_TRACE(MT_TRACE_LEVEL_INFO, 		module_id, fmt)
#define MT_DBG_PRINT(module_id, fmt...)
#else
/**Reserve all the levels output. */
/**CNcomment: 保留所有级别调试信息 */
#define MT_FATAL_PRINT(module_id, fmt...) 		MT_TRACE(MT_TRACE_LEVEL_FATAL, 		module_id, fmt)
#define MT_ERR_PRINT(module_id, fmt...)		    MT_TRACE(MT_TRACE_LEVEL_ERROR, 		module_id, fmt)
#define MT_WARN_PRINT(module_id, fmt...)		MT_TRACE(MT_TRACE_LEVEL_WARN, 		module_id, fmt)
#define MT_INFO_PRINT(module_id, fmt...)		MT_TRACE(MT_TRACE_LEVEL_INFO, 		module_id, fmt)
#define MT_DBG_PRINT(module_id, fmt...)			MT_TRACE(MT_TRACE_LEVEL_DBG, 		module_id, fmt)
#endif

#define MT_ALWAYS_PRINT(fmt, ...)				MT_PRINT("[ALWAYS]: %s[%d]:" fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__)

#else
#define MT_FATAL_PRINT(module_id, fmt...)
#define MT_ERR_PRINT(module_id, fmt...)
#define MT_WARN_PRINT(module_id, fmt...)
#define MT_INFO_PRINT(module_id, fmt...)
#define MT_DBG_PRINT(module_id, fmt...)
#define MT_TRACE(level, module_id, fmt...)
#define MT_ASSERT(expr)
#define MT_ASSERT_RET(expr)
#define MT_DEBUG_LOG(fmt...)
#define MT_ERROR_LOG(fmt...)
#define MT_ALWAYS_PRINT(fmt...)
#endif /* endif MT_DEBUG */

/** @} */  /** <!-- ==== Structure Definition End ==== */


#define MKSTR(exp) # exp
#define MKMARCOTOSTR(exp) MKSTR(exp)
#define VERSION_STRING ("SDK_VERSION:[" MKMARCOTOSTR(SDK_VERSION) "] Build Time:[" __DATE__ ", " __TIME__ "]")


/**Initialize a user module.*/
/**CNcomment: 用户模块初始化 */
#define MT_MODULE_DECLARE(MODULE_NAME)	\
	static mt_u32 module_id = MT_INVALID_MODULE_ID;	\
	static mt_s32 __attribute__((constructor(200))) init_module_id() \
	{	 \
		return mt_module_register_by_name(MODULE_NAME, &module_id); \
	} \
	static mt_s32 __attribute__((destructor(200)))	deinit_module_id() \
	{ \
		return mt_module_unregister(module_id); \
	}

/**Defines a user module ID.*/
/**CNcomment: 用户模块ID宏定义 */
#define MODULE_ID (module_id)

/**Defines the command of the user module different level log print.*/
/**CNcomment: 用户模块日志输出宏定义 */
#define MT_MODULE_FATAL(format...)		MT_FATAL_PRINT(MODULE_ID,format)
#define MT_MODULE_ERROR(format...)		MT_ERR_PRINT(MODULE_ID,format)
#define MT_MODULE_WARN(format...)	    MT_WARN_PRINT(MODULE_ID,format)
#define MT_MODULE_DEBUG(format...)		MT_DBG_PRINT(MODULE_ID,format)
#define MT_MODULE_INFO(format...)		MT_INFO_PRINT(MODULE_ID,format)

/**Defines the memory allocate and free command used by user module.*/
/**CNcomment: 用户模块内存分配宏定义 */
#define MT_MODULE_MALLOC(size)					MT_MEM_Malloc(MODULE_ID,size)
#define MT_MODULE_FREE(mem_addr)					MT_MEM_Free(MODULE_ID,mem_addr)
#define MT_MODULE_CALLOC(mem_block, size)		MT_MEM_Calloc(MODULE_ID,mem_block,size)
#define MT_MODULE_REALLOC(mem_addr,size)		    MT_MEM_Realloc(MODULE_ID,mem_addr,size)

/*Defines Video Image Trace Log Function*/
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
#ifdef __KERNEL__
#define VTRACE							printk
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
static __inline__ unsigned long gettid(void)
{
	return current->pid;
}
#else
#define VTRACE							printf
#endif
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_DEBUG_H__ */

