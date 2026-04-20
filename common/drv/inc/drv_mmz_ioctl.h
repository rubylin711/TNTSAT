#ifndef __DRV_MMZ_IOCTL_H__
#define __DRV_MMZ_IOCTL_H__

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

/* remove pclint waring anonymous struct or union */
/*lint -save -e1504 */
/*lint -save -e657  */
/*lint -save -e658  */

/**
 * extended MMZ attribute for security etc.
 *  see: mt_mmz_security_attr_s and mmz_security_attr_s
 */
struct mmz_io_attr {
	mt_u32 flags;		/* security flags */
	mt_s32 type;		/* main type, such as MT_MMZ_TYPE_VIDEO/MT_MMZ_TYPE_AUDIO/... */
	mt_s32 subtype;		/* sub type, such as MT_MMZ_SUBTYPE_VIDEO_ES/MT_MMZ_SUBTYPE_AUDIO_ES/... */

	/* reserved for future usage */
	mt_u32 reserved[5];
};

struct mmb_info {
    phys_addr_t phys_addr;	/* phys-memory address */
    unsigned long align;		/* if you need your phys-memory have special align size */
    unsigned long size;		/* length of memory you need, in bytes */
    unsigned int order;

    void *mapped;			/* userspace mapped ptr */

    struct {
        int prot;		/* PROT_READ or PROT_WRITE */
        int flags;	/* MAP_SHARED or MAP_PRIVATE */
    };

    char mmb_name[MTL_MMB_NAME_LEN];
    char mmz_name[MTL_MMZ_NAME_LEN];
    unsigned long gfp;		/* reserved, do set to 0 */
	pid_t pid;

	struct mmz_io_attr attr;
};
/*lint -restore */

struct cache_op_mmz_area {
	void *vbase;
	ulong offset;
	ulong size;
};

struct dirty_area {
	unsigned long dirty_phys_start;    /* dirty physical address */
	unsigned long dirty_virt_start; /* dirty virtual  address, must be coherent with dirty_phys_addr */
	unsigned long dirty_size;
};

struct mmz_start_size {
	char mmz_name[MTL_MMZ_NAME_LEN];
	phys_addr_t phys_start;
	ulong nbytes;
};

#define IOC_MMB_ALLOC               _IOWR('m', 10,  struct mmb_info)
#define IOC_MMB_ATTR                _IOR ('m', 11,  struct mmb_info)
#define IOC_MMB_FREE                _IOW ('m', 12,  struct mmb_info)
#define IOC_MMB_USER_REMAP			_IOWR('m', 20,  struct mmb_info)
#define IOC_MMB_USER_REMAP_CACHED 	_IOWR('m', 21,  struct mmb_info)
#define IOC_MMB_USER_UNMAP			_IOWR('m', 22,  struct mmb_info)
#define IOC_MMB_USER_GETPHYADDR		_IOWR('m', 23,  struct mmb_info)
#define IOC_MMB_USER_TEST_S			_IOWR('m', 25,  struct mmb_info)
#define IOC_MMB_ADD_REF				_IO('r', 30)	/* ioctl(file, cmd, arg), arg is mmb_addr */
#define IOC_MMB_DEC_REF				_IO('r', 31)	/* ioctl(file, cmd, arg), arg is mmb_addr */
#define IOC_MMB_FLUSH_DCACHE		_IO('c', 40)
#define IOC_MMB_INV_DCACHE			_IO('c', 41)
#define IOC_MMB_FLUSH_DCACHE_DIRTY  _IOW('d', 50, struct dirty_area)
#define IOC_MMB_TEST_CACHE			_IOW('t',  60,  struct mmb_info)
#define IOC_MMZ_GET_START_SIZE		_IOWR('z', 70,  struct mmz_start_size)

#ifdef __KERNEL__

struct kmmb_info {
    phys_addr_t phys_addr;	/* phys-memory address */
    unsigned long align;		/* if you need your phys-memory have special align size */
    unsigned long size;		/* length of memory you need, in bytes */
    unsigned int order;

    void *mapped;			/* userspace mapped ptr */

    struct {
        int prot;	/* PROT_READ or PROT_WRITE */
        int flags;/* MAP_SHARED or MAP_PRIVATE */
        unsigned long reserved :28; /* reserved, do not use */
        unsigned long delayed_free :1;
        unsigned long map_cached :1;
#ifdef MMB_SHARE_SUPPORT
        unsigned long mmb_type :2; /*0: not share, 1 :share and in current program list , 2: share and in share list*/
#endif
    };

    char mmb_name[MTL_MMB_NAME_LEN];
    char mmz_name[MTL_MMZ_NAME_LEN];
    unsigned long gfp;		/* reserved, do set to 0 */
	pid_t pid;

	struct mmz_io_attr attr;

	struct {
		int map_ref;
		int mmb_ref;
		struct list_head list;
		mtl_mmb_t *mmb;
#ifdef MMB_SHARE_SUPPORT
		//pid_t pid;
		struct list_head share_list;
#endif
	};
};

struct kmmz_start_size {
	char mmz_name[MTL_MMZ_NAME_LEN];
	phys_addr_t phys_start;
	ulong nbytes;
};

#ifdef CONFIG_COMPAT
struct compat_dirty_area {
	compat_ulong_t dirty_phys_start;    /* dirty physical address */
	compat_ulong_t dirty_virt_start; /* dirty virtual  address, must be coherent with dirty_phys_addr */
	compat_ulong_t dirty_size;
};

struct compat_mmb_info {
    phys_addr_t phys_addr;	/* phys-memory address */
    compat_ulong_t align;		/* if you need your phys-memory have special align size */
    compat_ulong_t size;		/* length of memory you need, in bytes */
    compat_ulong_t order;

    compat_uptr_t mapped;			/* userspace mapped ptr */

    struct {
        compat_int_t prot;	/* PROT_READ or PROT_WRITE */
        compat_int_t flags;	/* MAP_SHARED or MAP_PRIVATE */
    };

    char mmb_name[MTL_MMB_NAME_LEN];
    char mmz_name[MTL_MMZ_NAME_LEN];
    compat_ulong_t gfp;		/* reserved, do set to 0 */
	compat_pid_t pid;

	struct mmz_io_attr attr;
};

struct compat_cache_op_mmz_area {
	compat_uptr_t vbase;
	compat_ulong_t offset;
	compat_ulong_t size;
};

struct compat_mmz_start_size {
	char mmz_name[MTL_MMZ_NAME_LEN];
	phys_addr_t phys_start;
	compat_ulong_t nbytes;
};

#define COMPAT_IOC_MMB_ALLOC				_IOWR('m', 10,  struct compat_mmb_info)
#define COMPAT_IOC_MMB_ATTR					_IOR ('m', 11,  struct compat_mmb_info)
#define COMPAT_IOC_MMB_FREE					_IOW ('m', 12,  struct compat_mmb_info)
#define COMPAT_IOC_MMB_USER_REMAP			_IOWR('m', 20,  struct compat_mmb_info)
#define COMPAT_IOC_MMB_USER_REMAP_CACHED	_IOWR('m', 21,  struct compat_mmb_info)
#define COMPAT_IOC_MMB_USER_UNMAP			_IOWR('m', 22,  struct compat_mmb_info)
#define COMPAT_IOC_MMB_USER_GETPHYADDR		_IOWR('m', 23,  struct compat_mmb_info)
#define COMPAT_IOC_MMB_USER_TEST_S			_IOWR('m', 25,  struct compat_mmb_info)
#define COMPAT_IOC_MMB_ADD_REF				_IO('r', 30)	/* ioctl(file, cmd, arg), arg is mmb_addr */
#define COMPAT_IOC_MMB_DEC_REF				_IO('r', 31)	/* ioctl(file, cmd, arg), arg is mmb_addr */
#define COMPAT_IOC_MMB_FLUSH_DCACHE			_IO('c', 40)
#define COMPAT_IOC_MMB_INV_DCACHE			_IO('c', 41)
#define COMPAT_IOC_MMB_FLUSH_DCACHE_DIRTY	_IOW('d', 50, struct compat_dirty_area)
#define COMPAT_IOC_MMB_TEST_CACHE			_IOW('t',  60,  struct compat_mmb_info)
#define COMPAT_IOC_MMZ_GET_START_SIZE		_IOWR('z', 70,  struct compat_mmz_start_size)
#endif

#endif

#endif /* __DRV_MMZ_IOCTL_H__ */

