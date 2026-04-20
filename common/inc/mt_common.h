/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __MT_COMMON_H__
#define __MT_COMMON_H__

#include "mt_type.h"
#include "mt_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
/*
 * logic and 0x1fffffff to compatible 0x1XXXXXXX and 0xBXXXXXXX
 * x support physical address or virtual address
 */
#define SYMPHONY_IO_PA(x) ((x) & 0x1fffffffUL)
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
/*
 * logic or 0xA0000000 to compatible 0x1XXXXXXX and 0xBXXXXXXX
 * x support physical address or bus address
 */
#define SYMPHONY_IO_PA(x) ((x) | 0xa0000000UL)
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
/*
 * logic or 0xA0000000 to compatible 0x1XXXXXXX and 0xBXXXXXXX
 * x support physical address or bus address
 */
	#ifdef __KERNEL__
#define SYMPHONY_IO_PA(x) ((x) | 0xa0000000UL)
	#else
		#ifdef CONFIG_MT_LONG_32
#define SYMPHONY_IO_PA(x) ((x) | 0xa0000000ULL)
		#else
#define SYMPHONY_IO_PA(x) ((x) | 0xa0000000UL)
		#endif
	#endif
#else
#error "Please config select one correct chipset"
#endif

#ifndef __KERNEL__
#include <sys/prctl.h>
int mt_set_pthread_name(const char *name);
int daemon_init(void);
#endif


#ifdef __KERNEL__
#include <linux/sizes.h>
//#ifndef SZ_1K
//#define SZ_1K     (0x00000400)
//#endif
#endif

/*******************************Structure declaration *****************************/
/** @addtogroup     COMMON */
/** @{ */ /** <!--  [COMMON] */

/** Global config structure */
typedef struct mtsys_conf_s
{
    mt_u32 u32Reverse;  /**<Not used, reserved for extension*/ /**<CNcomment: 暂时没有使用，留待扩展*/

}mt_sys_conf_s;

/** Define the chip type. */
typedef enum mtchip_type_e
{
    MT_CHIP_TYPE_MT_ARIA,
    MT_CHIP_TYPE_MT_SYMPHONY,
    MT_CHIP_TYPE_MT_SYMPHONY4,
    MT_CHIP_TYPE_MT_SYMPHONY6,

    MT_CHIP_TYPE_BUTT
}MT_CHIP_TYPE_E;

/** Define the chip version. */
typedef enum mtCHIP_VERSION_E
{
	MT_CHIP_ARIA_V100	= 0xaa100,
	MT_CHIP_ARIA_V101	= 0xaa101,
	MT_CHIP_ARIA_V200	= 0xaa200,
	MT_CHIP_ARIA_V300	= 0xaa300,
	MT_CHIP_ARIA_V400	= 0xaa400,

    MT_CHIP_CONCERTO_A0 = 0xc01a0,
    MT_CHIP_CONCERTO_A1,
    MT_CHIP_CONCERTO_A3 = 0xc01a3,
    MT_CHIP_CONCERTO_B0 = 0xc01b0,

    MT_CHIP_SYMPHONY_A0 = 0xcf1a0,
    MT_CHIP_SYMPHONY_A1,
    MT_CHIP_SYMPHONY_A2,
	MT_CHIP_SYMPHONY1_MAX,

    MT_CHIP_SYMPHONY3_A0 = 0xcf3a0,
    MT_CHIP_SYMPHONY2_A0 = 0xcf2a0,

    MT_CHIP_SYMPHONY2_A1,
    MT_CHIP_SYMPHONY2_A2,
    MT_CHIP_SYMPHONY2_A3,
	MT_CHIP_SYMPHONY2_MAX,

    MT_CHIP_SYMPHONY4_A0 = 0xcf4a0,
    MT_CHIP_SYMPHONY4_A1,
	MT_CHIP_SYMPHONY4_MAX,

	MT_CHIP_SYMPHONY6_A0 = 0xcf6a0,
	MT_CHIP_SYMPHONY6_A1,
	MT_CHIP_SYMPHONY6_MAX,

	/* urgly historical Aria Chip Version definition */
    MT_CHIP_VERSION_V100 = MT_CHIP_ARIA_V100,
    MT_CHIP_VERSION_V101 = MT_CHIP_ARIA_V101,
    MT_CHIP_VERSION_V200 = MT_CHIP_ARIA_V200,
    MT_CHIP_VERSION_V300 = MT_CHIP_ARIA_V300,
    MT_CHIP_VERSION_V400 = MT_CHIP_ARIA_V400,

    MT_CHIP_VERSION_BUTT = 0xffff,
}MT_CHIP_VERSION_E;

/** Define the chip support attrs */
typedef enum mtCHIP_CAP_E
{
    MT_CHIP_CAP_DOLBY,
    MT_CHIP_CAP_DTS,
    MT_CHIP_CAP_ADVCA,
    MT_CHIP_CAP_MACROVISION
} MT_CHIP_CAP_E;

/**System version, that is, the version of the software developer's kit (SDK)*/
typedef struct mtsys_version_s
{
    MT_CHIP_TYPE_E  enChipTypeSoft;      /**<Chip type corresponding to the SDK*/ /**<CNcomment:  SDK软件对应的芯片类型 */
    MT_CHIP_TYPE_E  enChipTypeHardWare;  /**<Chip type that is detected when the SDK is running*/ /**<CNcomment:  SDK运行时检测到的芯片类型 */
    MT_CHIP_VERSION_E enChipVersion;     /**<Chip version that is detected when the SDK is running*/ /**<CNcomment: SDK运行时检测到芯片版本号 */
    mt_char         aversion[80];        /**<version string of the sdk*/ /**<cncomment:  sdk桧件版本哄做符串 */
    mt_char         BootVersion[80];     /**<Version string of the Boot*/ /**<CNcomment:  Boot版本号字符串 */
}mt_sys_version_s;

#ifndef MT_SYS_VERSION_S
#define MT_SYS_VERSION_S mt_sys_version_s
#endif

/** Define the chip attributes */
typedef struct mtsys_chip_attr_s
{
    MT_BOOL bDolbySupport;              /**<Whether this chip support dolby or not*//**<CNcomment:芯片是否支持杜比*/

}mt_sys_chip_attr_s;

/** Maximum bytes of a buffer name */
#define MAX_BUFFER_NAME_SIZE 16

/**Structure of an MMZ buffer*/
typedef struct mtmmz_buf_s
{
    mt_char bufname[MAX_BUFFER_NAME_SIZE];  /**<Strings of an MMZ buffer name*/ /**<CNcomment:  MMZ buffer名字字符串 */
    phys_addr_t  phyaddr;                /**<Physical address of an MMZ buffer*/ /**<CNcomment:  MMZ buffer物理地址 */
    void  *kernel_viraddr;         /**<Kernel-state virtual address of an MMZ buffer*/ /**<CNcomment:  MMZ buffer内核态虚拟地址 */
    void  *user_viraddr;           /**<User-state virtual address of an MMZ buffer*/ /**<CNcomment:  MMZ buffer用户态虚拟地址 */
    ulong  bufsize;                /**<Size of an MMZ buffer*/ /**<CNcomment:  MMZ buffer大小 */
    mt_u32  overflow_threshold;     /**<Overflow threshold of an MMZ buffer, in percentage. For example, the value 100 indicates 100%.*/ /**<CNcomment:  MMZ buffer上溢水线，按百分比设置，例如: 100 indicates 100%.*/
    mt_u32  underflow_threshold;    /**<Underflow threshold of an MMZ buffer, in percentage. For example, the value 0 indicates 0%.*/ /**<CNcomment:  MMZ buffer下溢水线，按百分比设置，例如: 0 indicates 0%.*/
}mt_mmz_buf_s;

/** extended MMZ attribute for security. */
typedef struct mtmmz_security_attr_s {
	mt_u32 flags;		/* security flags */
	mt_s32 type;		/* main type, such as MT_MMZ_TYPE_VIDEO/MT_MMZ_TYPE_AUDIO/... */
	mt_s32 subtype;		/* sub type, such as MT_MMZ_SUBTYPE_VIDEO_ES/MT_MMZ_SUBTYPE_AUDIO_ES/... */

	/* reserved for future usage */
	mt_u32 reserved[5];
} mt_mmz_security_attr_s;

typedef struct mtrect_s
{
    mt_s32 s32X;
    mt_s32 s32Y;
    mt_s32 s32Width;
    mt_s32 s32Height;
} mt_rect_s;

typedef enum mtLAYER_ZORDER_E
{
    MT_LAYER_ZORDER_MOVETOP = 0,  /**<Move to the top*/ /**<CNcomment:  移到最顶部 */
    MT_LAYER_ZORDER_MOVEUP,       /**<Move up*/ /**<CNcomment:  向上移到 */
    MT_LAYER_ZORDER_MOVEBOTTOM,   /**<Move to the bottom*/ /**<CNcomment:  移到最底部 */
    MT_LAYER_ZORDER_MOVEDOWN,     /**<Move down*/ /**<CNcomment:  向下移到 */
    MT_LAYER_ZORDER_BUTT
} MT_LAYER_ZORDER_E;

/** Defines user mode proc show buffer */
/**CNcomment: 用户态PROC buffer定义 */
typedef struct mtproc_show_buffer_s
{
    mt_u8* pu8Buf;                  /**<Buffer address*/  /**<CNcomment: Buffer地址 */
    ulong size;                 /**<Buffer size*/     /**<CNcomment: Buffer大小 */
    ulong offset;               /**<Offset*/          /**<CNcomment: 打印偏移地址 */
}mt_proc_show_buffer_s;


#ifndef MT_PROC_SHOW_BUFFER_S
#define  MT_PROC_SHOW_BUFFER_S mt_proc_show_buffer_s
#endif
/** Proc show function */
/**CNcomment: Proc信息显示回调函数 */
typedef mt_s32 (* mt_proc_show_fn)(mt_proc_show_buffer_s * pstbuf, mt_void *pprivdata);

/** Proc command function */
/**CNcomment: Proc控制回调函数 */
typedef mt_s32 (* mt_proc_cmd_fn)(mt_proc_show_buffer_s * pstbuf, mt_u32 u32argc, mt_u8 *pu8argv[], mt_void *pprivdata);

/** Defines user mode proc entry */
/**CNcomment: 用户态PROC入口定义 */
typedef struct mtproc_entry_s
{
    mt_char *pszEntryName;          /**<Entry name*/            /**<CNcomment: 入口文件名 */
    mt_char *pszDirectory;          /**<Directory name. If null, the entry will be added to /proc/mcomm directory*/
                                    /**<CNcomment: 目录名，如果为空，将创建到/proc/mcomm 目录下 */
    mt_proc_show_fn pfnShowProc;    /**<Proc show function*/    /**<CNcomment: Proc信息显示回调函数 */
    mt_proc_cmd_fn pfnCmdProc;      /**<Proc command function*/ /**<CNcomment: Proc控制回调函数 */
    mt_void *pPrivData;             /**<Private data*/          /**<CNcomment: Buffer地址 */
}mt_u_proc_entry_s;

#ifndef MT_PROC_ENTRY_S
#define MT_PROC_ENTRY_S mt_u_proc_entry_s
#endif

/** Defines DDR configuration type struct */
/**CNcomment: DDR 配置类型结构定义 */
typedef struct mtsys_mem_config_s
{
    mt_u32 u32TotalSize;    /** Total memory size(MB) */      /**<CNcomment: 总内存大小(MB)*/
    mt_u32 u32MMZSize;      /**MMZ memory size(MB) */       /** <CNcomment: MMZ内存大小(MB)*/
}mt_sys_mem_config_s;

typedef struct
{
   mt_u32 tm_sec;           /* Seconds. [0-60] (1 leap second) */
   mt_u32 tm_min;           /* Minutes. [0-59] */
   mt_u32 tm_hour;          /* Hours.   [0-23] */
   mt_u32 tm_mday;          /* Day.     [1-31] */
   mt_u32 tm_mon;           /* Month.   [0-11] */
   mt_u32 tm_year;          /* Year   */
}mt_sysdate_t;

#define MMZ_OTHERS                     NULL
#define MMZ_ZONE_DDR                   "ddr"
#define MMZ_ZONE_AV                    "av"
/* Audio Output */
#define MMZ_ZONE_PCM                   "pcm"
#define MMZ_ZONE_PIP                   "pip"

#ifdef CONFIG_MT_HAVE_AUDIO_MMZ
/* Audio ES 0 */
#define MMZ_ZONE_AUDIO         "audio"
/* Audio ES 1 */
#define MMZ_ZONE_AUDIO1        "audio1"
/* PVR Descrambled Record 0 */
#define MMZ_ZONE_PVR0          "pvr0"
/* PVR Descrambled Record 1 */
#define MMZ_ZONE_PVR1          "pvr1"
#define MMZ_ZONE_PVR2          "pvr2"
#define MMZ_ZONE_PVR3          "pvr3"
#else
#define MMZ_ZONE_AUDIO         MMZ_ZONE_DDR
#define MMZ_ZONE_AUDIO1        MMZ_ZONE_DDR
#define MMZ_ZONE_PVR0          MMZ_OTHERS
#define MMZ_ZONE_PVR1          MMZ_OTHERS
#define MMZ_ZONE_PVR2          MMZ_OTHERS
#define MMZ_ZONE_PVR3          MMZ_OTHERS
#endif

#define MTL_MMZ_NAME_LEN 32
#define MTL_MMB_NAME_LEN 32

#ifndef __KERNEL__
#include <time.h>
#include <sys/types.h>

#if defined(CONFIG_MT_PHYS_RANGE_BEYOND_4G) && defined(CONFIG_MT_LONG_32)
#define PAGE_SIZE 4096ULL
#else
#define PAGE_SIZE 4096UL
#endif
#define MEM_PAGE_SIZE  PAGE_SIZE
#define PAGE_SIZE_MASK (~(PAGE_SIZE - 1))
#define MEMDEV_PAGE_ALIGN_MASK    (PAGE_SIZE - 1)

/** MMZ attribute type/subtype definition */
#define MT_MMZ_TYPE_UND			0
	#define MT_MMZ_SUBTYPE_UND			0
#define MT_MMZ_TYPE_VIDEO		1
	#define MT_MMZ_SUBTYPE_VIDEO_ES		1
	#define MT_MMZ_SUBTYPE_VIDEO_FRAME 	2
#define MT_MMZ_TYPE_AUDIO		2
	#define MT_MMZ_SUBTYPE_AUDIO_ES		1
	#define MT_MMZ_SUBTYPE_AUDIO_PCM	2
#define MT_MMZ_TYPE_DISP		3
#define MT_MMZ_TYPE_GRAPHIC		4
#define MT_MMZ_TYPE_PVR			5
	#define MT_MMZ_SUBTYPE_PVR_REC		1
/* add your mmz type definition here: */
#define MT_MMZ_TYPE_MAX			6


#if (defined(__GNUC__) && (__GNUC__ < 10))
extern pid_t gettid(void);
#endif
extern int mt_msleep(mt_u32 msecs);
extern void mt_backtrace(void);

/** @} */  /** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup      COMMON */
/** @{ */  /** <!-- [COMMON] */

/**
@brief Initializes the system. CNcomment: 系统初始化 CNend
@attention \n
You must call this API to initialize the system before using the APIs of all modules.
Though you can call other APIs successfully before calling this API, the subsequent operations may fail.\n
CNcomment: 在使用所有模块的接口之前都需要先调用此接口对系统进行初始化\n
在调用这个接口之前调用其他接口，不会返回失败，但是不保证执行的正确性 CNend
@param N/A CNcomment: 无 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::mt_failure calling this api fails. cncomment: api锏沓调筱戋包 cnend
@see \n
n/a cncomment: 铪 cnend
*/
mt_s32 mt_sys_init(mt_void);
/**
@brief Deinitializes the system. CNcomment: 系统去初始化 CNend
@attention \n
If all modules are not used, you need to call this API to deinitialize the system.\n
CNcomment: 所有模块都不再使用后调用此接口去初始化 CNend
@param n/a cncomment: 铪 cnend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::mt_failure calling this api fails. cncomment: api锏沓调筱戋包 cnend
@see \n
n/a cncomment: 铪 cnend
*/
mt_s32 mt_sys_deinit(mt_void);


/**
@brief Obtains the compiled time of a version. CNcomment: 获取版本的编译时间 CNend
@attention \n
The compiled time is the time during which the common module is made again.
CNcomment: 时间为进行common模块重新make的时间 CNend
@param[out] pstTime Pointer to the compiled time of a version (output). CNcomment: 指针类型，输出版本编译的时间。 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_get_build_time(struct tm * psttime);


/**
@brief Obtains the version number. CNcomment: 获取版本号 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[out] pstVersion Pointer to the version number (output). CNcomment: 指针类型，输出版本号。 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_get_version(mt_sys_version_s *pstversion);


/**
@brief Obtains the chip support attributes. CNcomment: 获取芯片支持的能力 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] enChipCap Indicate which chip capability. CNcomment: 输入参数，指明获取何种芯片能力 CNend
@param[out] pbSupport Pointer to the chip whether support the attributes(output). CNcomment: 输出参数，保存获取结果 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_get_chip_capability(MT_CHIP_CAP_E enchipcap, MT_BOOL *pbsupport);

/**
@brief Obtains the chip attributes. CNcomment: 获取芯片属性 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[out] pstChipAttr Pointer to the chip attributes(output). CNcomment: 指针类型，输出芯片属性 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_get_chip_attr(mt_sys_chip_attr_s *pstchipattr);

mt_s32 mt_sys_crc32(mt_u8 *pu8Src, mt_u32 u32SrcLen, ulong *pDst);

/**
@brief Obtains the chip attributes. CNcomment: 获取内存配置信息 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[out] pstConfig Pointer to address for memory configuration(output). CNcomment: 指针类型，输出内存配置信息指针 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_get_mem_config(mt_sys_mem_config_s *pstconfig);

/**
@brief Performs global system configuration. CNcomment: 设置系统的全局配置 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] pstSysConf Pointer to the address for system configuration CNcomment: 指针类型，系统配置指针地址。 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_set_conf(const mt_sys_conf_s *pstsysconf);

/**
@brief Obtains global system configuration. CNcomment: 获取系统的全局配置 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[out] pstSysConf Pointer to the system configuration (output). CNcomment: 指针类型，输出系统配置。 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_get_conf(mt_sys_conf_s *pstsysconf);

/**
@brief Sets the debugging information level of a module. CNcomment: 设置模块的调试信息级别 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] enModId Debugging ID of a module CNcomment: 模块的调试ID。 CNend
@param[in] enLogLevel Debugging information level of a module CNcomment: 模块的调试信息级别。 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
*/
mt_s32 mt_sys_set_log_level(mt_mod_id_e enmodid,  mt_log_level_e enloglevel);

/**
@brief Sets the debugging information file path for U-disk. CNcomment: 设置日志存储路径 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in]  pszLogPath Debugging information file path. CNcomment: 日志的存储路径 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
*/
mt_s32 mt_sys_set_log_path(const mt_char* pszlogpath);

/**
@brief Sets the debugging files(may be stream, YUV data, image...) save path. CNcomment: 设置调试文件存储路径 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in]  pszPath Debugging files path. CNcomment: 调试文件的存储路径 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
*/
mt_s32 mt_sys_set_store_path(const mt_char* pszpath);

/**
@brief Writes to a register or a memory. CNcomment:  写寄存器或内存 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] RegAddr Physical address of a register or a memory CNcomment: 寄存器或内存的物理地址。 CNend
@param[in] u32Value Value of a register CNcomment:  寄存器的值。 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_write_register(phys_addr_t RegAddr, mt_u32 u32Value);

/**
@brief Reads a register or a memory. CNcomment: 读寄存器或内存 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] RegAddr Physical address of a register or a memory CNcomment: 寄存器或内存的物理地址。 CNend
@param[out] pu32Value Pointer to the register value (output) CNcomment:  指针类型，输出寄存器的值。 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_read_register(phys_addr_t RegAddr, mt_u32 *pu32Value);

/**
@brief Map registers address. CNcomment:  映射寄存器地址 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] RegAddr The start physical address of registers. CNcomment: 寄存器的起始物理地址。 CNend
@param[in] u32Length  Length of the registers want to map CNcomment:   寄存器长度。 CNend
@param[out] pVirAddr  Virtual address CNcomment:   映射后的虚拟地址。 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_map_register(phys_addr_t RegAddr, mt_u32 u32Length, mt_void **pVirAddr);

/**
@brief Unmap registers address. CNcomment:  解除寄存器地址映射 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] pVirAddr The virtual address to be unmapped CNcomment: 要解除映射的虚拟地址。 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_unmap_register(mt_void *pVirAddr);

/**
@brief Get timestamp. CNcomment: 获取时间戳。 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[out] pu32TimeMs Pointer to the timestamp value (output) CNcomment: 输出时间戳。 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_sys_get_time_stamp_ms(mt_u32 *pu32TimeMs);

/**
@brief Applies for a media memory zone (MMZ) and maps the user-state address.
CNcomment:  申请mmz内存，并映射用户态地址 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in/out] pstBuf Structure of the buffer information. Bufname and bufsize are inputs, and the physical address and user-state virtual address are outputs.
                    CNcomment: buffer信息结构，bufname和bufsize作为输入,物理地址和用户态虚拟地址作为输出 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mmz_malloc(mt_mmz_buf_s *pstBuf);

/**
@brief Unmaps the user-state address and releases the MMZ. CNcomment: 解除用户态地址的映射，并释放mmz内存 CNend
@attention \n
Ensure that the lengths of the transferred physical address and user-state virtual address are correct.
CNcomment: 保证传入的物理地址、用户态虚拟地址和长度正确 CNend
@param[in] pstBuf Structure of the buffer information CNcomment: buffer信息结构 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mmz_free(mt_mmz_buf_s *pstBuf);

/**
@brief pplies for an MMZ with a specified name and obtains its physical address. CNcomment: 指定mmz的名字申请mmz内存，返回物理地址 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] size Buffer size CNcomment: buffer大小 CNend
@param[in] u32Align Alignment mode CNcomment: 对齐方式 CNend
@param[in] ps8MMZName Name of an MMZ in the buffer. If the MMZ name is set to NULL, an MMZ is anonymously applied for. CNcomment: buffer分区的名字，传入NULL匿名申请 CNend
@param[in] ps8MMBName Buffer name CNcomment: buffer块的名字 CNend
@retval ::NULL The application fails. CNcomment: 申请失败 CNend
@retval Physical address CNcomment: 物理地址 CNend
@see \n
N/A CNcomment: 无 CNend
*/
phys_addr_t mt_mmz_new(ulong size , mt_u32 u32Align, const mt_char *ps8MMZName, const mt_char *ps8MMBName);

/**
@brief pplies for an MMZ with a specified name and obtains its physical address. CNcomment: 指定mmz的名字申请mmz内存，返回物理地址 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] size Buffer size CNcomment: buffer大小 CNend
@param[in] u32Align Alignment mode CNcomment: 对齐方式 CNend
@param[in] ps8MMZName Name of an MMZ in the buffer. If the MMZ name is set to NULL, an MMZ is anonymously applied for. CNcomment: buffer分区的名字，传入NULL匿名申请 CNend
@param[in] ps8MMBName Buffer name CNcomment: buffer块的名字 CNend
@param[in] attr Buffer attribute CNcomment: buffer块的属性 CNend
@retval ::NULL The application fails. CNcomment: 申请失败 CNend
@retval Physical address CNcomment: 物理地址 CNend
@see \n
N/A CNcomment: 无 CNend
*/
phys_addr_t mt_mmz_new_secure(ulong size , mt_u32 u32Align, const mt_char *ps8MMZName, const mt_char *ps8MMBName, mt_mmz_security_attr_s *attr);

/**
@brief Releases an MMZ based on its physical address. CNcomment: 通过物理地址释放mmz内存 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] PhysAddr Physical address of a buffer CNcomment: buffer物理地址 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mmz_delete(phys_addr_t PhysAddr);

/**
@brief Maps the physical address of an MMZ applied for to a user-state virtual address. You can determine whether to cache the address.
CNcomment: 将mmz申请的物理地址映射成用户态虚拟地址，可以指定是否cached CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] PhysAddr Physical address of a buffer CNcomment: buffer物理地址 CNend
@param[in] u32Cached Whether to cache the address. 0: no; 1: yes CNcomment: 是否使用cache，0不使用，1使用 CNend
@retval ::NULL The application fails. CNcomment: 申请失败 CNend
@retval User-state virtual address CNcomment: 用户态虚地址 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_void *mt_mmz_map(phys_addr_t PhysAddr, mt_u32 u32Cached);

/**
@brief Unmaps the user-state address of an MMZ. CNcomment: 解除mmz内存用户态地址的映射 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] vAddr Virtual address of a buffer CNcomment: buffer虚拟地址 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mmz_unmap(void *vAddr);


/**
@brief Flushes D-cache to the cached MMZ. CNcomment: 对于cached类型MMZ，刷Dcache到内存 CNend
@attention \n
If the value 0 is transferred, all D-caches are refreshed; otherwise, only the transferred memory is refreshed.
CNcomment: 如果传入0，则刷新所有的Dcache；否则只刷传入的那块内存 CNend
@param[in] vAddr Virtual base address of a buffer CNcomment: buffer虚拟基地址 CNend
@param[in] offset offset of a buffer CNcomment: buffer内部偏移 CNend
@param[in] size size of flush CNcomment: size, if size is 0, meaning flush whole buffer CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mmz_flush(void *vAddr, ulong offset, ulong size);


/**
@brief Invalidate D-cache to the cached MMZ. CNcomment: 对于cached类型MMZ，丢弃Dcache中的内容 CNend
@attention \n
If the value 0 is transferred, all D-caches are refreshed; otherwise, only the transferred memory is refreshed.
CNcomment: 不能传入0，只能丢弃传入的那块内存 CNend
@param[in] vAddr Virtual base address of a buffer CNcomment: buffer虚拟基地址 CNend
@param[in] offset offset of a buffer CNcomment: buffer内部偏移 CNend
@param[in] size size of flush CNcomment: size, if size is 0, meaning invalid whole buffer CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mmz_invalidate(void *vAddr, ulong offset, ulong size);

/**
@brief Obtains the physical address and size based on the virtual address. CNcomment: 根据虚拟地址获取物理地址，以及大小 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] vaddr User-state virtual address CNcomment: 用户态虚地址 CNend
@param[out] Phyaddr Physical address  CNcomment: 物理地址 CNend
@param[out] Size Size CNcomment: 大小 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mmz_get_phyaddr(void *vaddr, phys_addr_t *Phyaddr, ulong *Size);

/**
@brief Obtains the physical address and size based on the mmz name. CNcomment: 根据MMZ名字获取物理起始地址，以及大小 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] mmz_name User-state virtual address CNcomment: MMZ名字 CNend
@param[out] phys_start Physical address  CNcomment: 物理起始地址 CNend
@param[out] size Size CNcomment: 大小 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mmz_get_start_size(const char *mmz_name, phys_addr_t *phys_start, ulong *size);

/**
@brief Maps a physical address to a user-state virtual address. CNcomment: 将物理地址映射成用户态虚拟地址 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] PhyAddr Physical address of a buffer CNcomment: buffer物理地址 CNend
@param[in] size Buffer size CNcomment: buffer的大小 CNend
@retval ::NULL The application fails. CNcomment: 申请失败 CNend
@retval User-state virtual address CNcomment: 用户态虚地址 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_void *mt_mem_map(phys_addr_t PhyAddr, ulong size);
mt_void *mt_mem_map_cache(phys_addr_t PhyAddr, ulong size);


/**
@brief Unmaps a user-state address. CNcomment: 解除用户态地址的映射 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] AddrMapped User-state virtual address of a buffer. CNcomment: buffer的用户态虚地址 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mem_unmap(void *AddrMapped);

/**
@brief Flush/Invalid a user-state address. CNcomment: 刷cache CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] virtaddr Virtual address of a buffer CNcomment: buffer虚拟地址 CNend
@param[in] size Buffer size CNcomment: buffer的大小 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mem_flush(void *virtaddr, ulong size);
mt_s32 mt_mem_invalidate(void *virtaddr, ulong size);

/**
@brief Get page info. CNcomment: 获取page block size
@attention \n
N/A CNcomment: 无 CNend
@param[in] fetch info into this buffer CNcomment: page block order CNend
@param[in] fetch info into this buffer CNcomment: pages perf block CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mem_get_pageinfo(mt_u32 *page_block_order, mt_u32 *pages_per_block);

/**
@brief Obtains the physical address based on the virtual address. CNcomment: 根据虚拟地址获取物理地址，以及大小 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] vaddr User-state virtual address CNcomment: 用户态虚地址 CNend
@param[out] Phyaddr Physical address  CNcomment: 物理地址 CNend
@retval ::MT_SUCCESS Success CNcomment: 成功 CNend
@retval ::MT_FAILURE Calling this API fails. CNcomment: API系统调用失败 CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_s32 mt_mem_get_phyaddr(void *vaddr, phys_addr_t *phy_addr);

/**
@brief Register one module to manager. CNcomment:模块注册，用于管理 CNend
@attention Before manager someone module, calling this interface. CNcomment:如需管理模块，用此接口先注册 CNend
@param[in] pszModuleName The module name CNcomment:模块名称 CNend
@param[in] u32ModuleID   The module ID. CNcomment:模块ID CNend
@retval ::MT_SUCCESS Success CNcomment:成功 CNend
@retval ::MT_FAILURE Failure CNcomment:失败 CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_s32 mt_module_register(mt_u32 u32ModuleID, const mt_char * pszModuleName);

/**
@brief Register one moudle by name. CNcomment:模块注册，ID由系统分配 CNend
@attention Before manager someone module, calling this interface. CNcomment:如需管理模块，用此接口先注册 CNend
@param[in] pszModuleName The module name CNcomment:模块名称 CNend
@param[out] pu32ModuleID The module id allocated by system. CNcomment:系统分配的模块ID CNend
@retval ::MT_SUCCESS Success CNcomment:成功 CNend
@retval ::MT_FAILURE Failure CNcomment:失败 CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_s32 mt_module_register_by_name(const mt_char * pszModuleName, mt_u32* pu32ModuleID);

/**
@brief UnRegister one module to trace. CNcomment:模块移除 CNend
@attention Before stopping to manage someone module, calling this interface. CNcomment:不需要管理此模块时，使用此接口移除模块 CNend
@param[in] u32ModuleID The module ID. CNcomment:模块ID CNend
@param[out] None CNcomment:无 CNend
@retval ::MT_SUCCESS Success CNcomment:成功 CNend
@retval ::MT_FAILURE Failure CNcomment:失败 CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_s32 mt_module_unregister(mt_u32 u32ModuleID);

/**
@brief User mode proc cretea directory. CNcomment:用户态proc创建目录 CNend
@attention You need register module before calling this API. Only support create one level directory. CNcomment:需要先注册模块，只支持创建一级目录 CNend
@param[in] pszName The directory name. CNcomment:目录名 CNend
@param[out] None CNcomment:无 CNend
@retval ::MT_SUCCESS Success CNcomment:成功 CNend
@retval ::MT_FAILURE Failure CNcomment:失败 CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_s32 mt_proc_add_dir(const mt_char *pszName);

/**
@brief User mode proc remove directory. CNcomment:用户态proc删除目录 CNend
@attention It will return fail if there are entries in the directory. CNcomment:如果目录下还有入口文件,将会删除失败 CNend
@param[in] pszName The directory name. CNcomment:目录名 CNend
@retval ::MT_SUCCESS Success CNcomment:成功 CNend
@retval ::MT_FAILURE Failure CNcomment:失败 CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_s32 mt_proc_remove_dir(const mt_char *pszName);

/**
@brief User mode proc add entry. CNcomment:用户态proc创建入口 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] u32ModuleID Module ID. CNcomment:模块ID CNend
@param[in] pstEntry Parameter of entry. CNcomment:创建入口参数 CNend
@retval ::MT_SUCCESS Success CNcomment:成功 CNend
@retval ::MT_FAILURE Failure CNcomment:失败 CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_s32 mt_proc_add_entry(mt_u32 u32ModuleID, const mt_u_proc_entry_s* pstEntry);

/**
@brief User mode proc remove entry. CNcomment:用户态proc删除入口 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] u32ModuleID Module ID. CNcomment:模块ID CNend
@param[in] pstEntry Parameter of entry. CNcomment:删除入口参数 CNend
@retval ::MT_SUCCESS Success CNcomment:成功 CNend
@retval ::MT_FAILURE Failure CNcomment:失败 CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_s32 mt_proc_remove_entry(mt_u32 u32ModuleID, const mt_u_proc_entry_s* pstEntry);

/**
@brief User mode proc print function. CNcomment:用户态proc打印内容的函数 CNend
@attention \n
N/A CNcomment: 无 CNend
@param[in] pstBuf Output buffer parameter. CNcomment:输出buffer参数 CNend
@param[in] pFmt   Format parameter. CNcomment:打印格式化参数 CNend
@retval ::MT_SUCCESS Success CNcomment:成功 CNend
@retval ::MT_FAILURE Failure CNcomment:失败 CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_s32 mt_proc_printf(mt_proc_show_buffer_s *pstBuf, const mt_char *pFmt, ...);

/**
@brief malloc the pointed size from system heap. CNcomment:从系统中分配指定大小的内存 CNend
@attention None CNcomment:无 CNend
@param[in] u32ModuleID The module ID, who need to request memory. CNcomment:模块ID CNend
@param[in] u32Size The size of requesting. CNcomment:请求分配的大小，单位是字节 CNend
@param[out] None CNcomment:无 CNend
@retval ::Valid memory address Success CNcomment:成功返回分配到的空间首地址 CNend
@retval ::NULL Failure CNcomment:失败返回NULL CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_void* mt_mem_malloc(mt_u32 u32ModuleID, mt_u32 u32Size);


/**
@brief Free the requsted memory by MT_malloc. CNcomment:释放分配的内存 CNend
@attention when stopping to use the memory, calling this interface. CNcomment:不再需要这块内存时，使用此接口进行释放 CNend
@param[in] u32ModuleID The module ID, who need to free memory. CNcomment:模块ID CNend
@param[in] pMemAddr The memory address to free CNcomment:释放空间的首地址 CNend
@param[out] None CNcomment:无 CNend
@retval ::None CNcomment:无 CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_void mt_mem_free(mt_u32 u32ModuleID, mt_void* pMemAddr);

/**
@brief Calloc memory, with u32MemBlock blocks and u32Size size per. CNcomment:分配连续大小的内存块 CNend
@attention None CNcomment:无 CNend
@param[in] u32ModuleID The module id, who need to calloc memory. CNcomment:模块ID CNend
@param[in] u32MemBlock The requesting block number. CNcomment:分配的块数 CNend
@param[in] u32Size The requesting size per block. CNcomment:每块的大小，单位是字节 CNend
@param[out] None CNcomment:无 CNend
@retval ::Valid memory address Success CNcomment:成功则返回分配到的内存首地址 CNend
@retval ::NULL Failure CNcomment:失败返回NULL CNend
@see \n
N/A CNcomment: 无 CNend
*/
mt_void* mt_mem_calloc(mt_u32 u32ModuleID, mt_u32 u32MemBlock, mt_u32 u32Size);

/**
@brief Realloc memory CNcomment:重新分配空间 CNend
@attention None CNcomment:无 CNend
@param[in] pMemAddr The memory address, which has been requested from system heap. CNcomment:要改变的内存大小的地址 CNend
@param[in] u32Size The newer memory size to request. CNcomment:新的内存空间大小，单位字节 CNend
@param[out] None CNcomment:无 CNend
@retval ::Valid memory address Success CNcomment:成功则返回新的内存空间首地址 CNend
@retval ::NULL Failure CNcomment:失败则返回NULL. CNend
@see \n
N/A CNcomment:无 CNend
*/
mt_void* mt_mem_realloc(mt_u32 u32ModuleID, mt_void *pMemAddr, mt_u32 u32Size);


/*
  * copy from include/linux/sched.h
  * if kernel reversion upgrade, you must check whether this struct changed.
  */
struct sched_attr {
               mt_u32 size;              /* Size of this structure */
               mt_u32 sched_policy;      /* Policy (SCHED_*) */
               mt_u64 sched_flags;       /* Flags */
               mt_s32 sched_nice;        /* Nice value (SCHED_OTHER,
                                         SCHED_BATCH) */
               mt_u32 sched_priority;    /* Static priority (SCHED_FIFO,
                                         SCHED_RR) */
               /* Remaining fields are for SCHED_DEADLINE */
               mt_u64 sched_runtime;
               mt_u64 sched_deadline;
               mt_u64 sched_period;
           };

long sched_setattr(pid_t pid, struct sched_attr *attr,
				  unsigned int flags);

long sched_getattr(pid_t pid, struct sched_attr *attr,
				  unsigned int size, unsigned int flags);

/* high-resolution sleep */
int mt_nanosleep_debug(const struct timespec *req, struct timespec *rem, const char *func, int line);

#define MT_NANOSLEEP(req, rem)	mt_nanosleep_debug(req, rem, __FUNCTION__, __LINE__)

/**
 * Notice: 'usec' shall >= 1000, or assert internally!
 *    < 1000us, please call MT_NANOSLEEP.
 */
int mt_usleep_debug(unsigned int usec, const char *func, int line);

/**
 * Notice: 'usec' shall >= 1000, or assert internally!
 *    < 1000us, please call MT_NANOSLEEP.
 */
#define MT_USLEEP(usec)	mt_usleep_debug(usec, __FUNCTION__, __LINE__)

static inline MT_BOOL mt_chip_is_symphony1(MT_CHIP_VERSION_E chip_id)
{
	return ((chip_id >= MT_CHIP_SYMPHONY_A0)
			&& (chip_id < MT_CHIP_SYMPHONY1_MAX))?MT_TRUE:MT_FALSE;
}

static inline MT_BOOL mt_chip_is_symphony2(MT_CHIP_VERSION_E chip_id)
{
	return ((chip_id >= MT_CHIP_SYMPHONY2_A0)
			&& (chip_id < MT_CHIP_SYMPHONY2_MAX))?MT_TRUE:MT_FALSE;
}

static inline MT_BOOL mt_chip_is_symphony4(MT_CHIP_VERSION_E chip_id)
{
	return ((chip_id >= MT_CHIP_SYMPHONY4_A0)
			&& (chip_id < MT_CHIP_SYMPHONY4_MAX))?MT_TRUE:MT_FALSE;
}

static inline MT_BOOL mt_chip_is_symphony6(MT_CHIP_VERSION_E chip_id)
{
	return ((chip_id >= MT_CHIP_SYMPHONY6_A0)
			&& (chip_id < MT_CHIP_SYMPHONY6_MAX))?MT_TRUE:MT_FALSE;
}


/**
\brief Set avcpu sysdate.
\param[in] date :date need set
*/
mt_s32 mt_sys_set_avdate(mt_sysdate_t *date);

/**
\brief Get avcpu sysdate.
\param[out] date :date need get
*/
mt_s32 mt_sys_get_avdate(mt_sysdate_t *date);

/**
\brief Get temperature.
\param[out] temp : temperature, int=temp/1000, dec=temp%1000
*/
mt_s32 mt_sys_get_temp(int *temp);

/**
 * @brief System Event
 */
typedef enum
{
	MT_SYS_EVENT_TEMP,						/* Event of Temperature */
	MT_SYS_EVENT_TEMP_HIGH_YELLOW,			/* Event of Temperature is very high */
	MT_SYS_EVENT_TEMP_HIGH_RED,				/* Event of Temperature is ultra high to auto standby */

	MT_SYS_EVENT_MAX,

} MT_SYS_EVENT_T;

/**
 * @brief System Event callback function type
 */
typedef int (*mt_sys_event_cb_func)(MT_SYS_EVENT_T event, void *data);

/**
 * @brief Register system event and callback function
 *
 * @param[in] event Event to be registered
 * @param[in] callback Event callback
 *
 * @retval
 *   MT_SUCCESS: success
 *   Others: failure
 */
int mt_unf_sys_monitor_register_event(MT_SYS_EVENT_T event, mt_sys_event_cb_func callback);

/**
 * @brief Register system event callback function
 *
  * @param[in] callback Event callback
 *
 * @retval
 *   MT_SUCCESS: success
 *   Others: failure
 */
int mt_unf_sys_monitor_register(mt_sys_event_cb_func callback);

/** @} */ /** <!-- ==== API declaration end ==== */

#endif /* endif __KERNEL__ */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_COMMON_H__ */

