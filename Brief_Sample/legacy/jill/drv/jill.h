#define JILL_DEBUG		1	/* 0: no print		1: print */
#if (JILL_DEBUG == 1)
#define JILL_PAINT_DEBUG(X, ...)	\
	do {							\
		printk(KERN_DEBUG X, ##__VA_ARGS__);				\
	}							\
	while (0);
#else
#define JILL_PAINT_DEBUG(X)
#endif

#define JILL_INFO		1	/* 0: no print		1: print */
#if (JILL_INFO == 1)
#define JILL_PAINT_INFO(X, ...)	\
	do {							\
		printk(KERN_INFO X, ##__VA_ARGS__);				\
	}							\
	while (0);
#else
#define JILL_PAINT_INFO(X)
#endif

#define JILL_ERROR		1	/* 0: no print		1: print */
#if (JILL_ERROR == 1)
#define JILL_PAINT_ERROR(X, ...)	\
	do {							\
		printk(KERN_ERR X, ##__VA_ARGS__);				\
	}							\
	while (0);
#else
#define JILL_PAINT_ERROR(X)
#endif

#define 	JILL_DEVICE_NAME	"jill"

#define	JILL_GPFCON				(0x56000050)
#define	JILL_GPFCON_OFFSET		(0)
#define	JILL_GPFDATA_OFFSET	(4)
#define	JILL_GPFUP_OFFSET		(8)

#undef JILL_IRQ

struct jill_platform_data {
	long seconds;
	unsigned long nano_seconds;
	u16 gpf_con_and;
	u16 gpf_con_or;
	u16 gpf_up_or;
	u8 default_value		:4;
};
