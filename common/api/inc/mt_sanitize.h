/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_SANITIZE__
#define __MT_SANITIZE__

#ifndef __KERNEL__

#include <stdlib.h>

#ifdef CONFIG_MT_SANITIZE_TAG
#include <sanitizer/hwasan_interface.h>
#endif

#define __no_sanitize_address __attribute__((no_sanitize("address", "hwaddress")))

#ifdef CONFIG_MT_SANITIZE_TAG
#define __untagged_addr(addr)	\
	((typeof(addr))sign_extend64((u64)(addr), 55))

#define untagged_addr(addr)	({					\
	u64 __addr = (u64)(addr);					\
	__addr &= __untagged_addr(__addr);				\
	(typeof(addr))__addr;				\
})
#define __tag_shifted(tag)	((u64)(tag) << 56)
#define __tag_reset(addr)	__untagged_addr(addr)
#define __tag_get(addr)		(u8)((u64)(addr) >> 56)

static inline s64 sign_extend64(u64 value, int index)
{
	u8 shift = 63 - index;
	return (s64)(value << shift) >> shift;
}

static inline void *__tag_set(const void *addr, u8 tag)
{
	u64 __addr = (u64)addr & ~__tag_shifted(0xff);
	return (const void *)(__addr | __tag_shifted(tag));
}

static inline u8 mt_hwasan_tag_memory(void *addr, size_t size)
{
	u8 tag = random();

	if (tag >= 251) {
		tag -= 251;
	}
	if (tag <= 18) {
		tag += 18;
	}
	/* skip special tag which defined in libsanitizer/hwasan/hwasan_flags.inc */
	if ((tag == 0xbe) || (tag == 0x55)) {
		tag++;
	}
	__hwasan_tag_memory(addr, tag, size);
	return tag;
}

static inline void mt_hwasan_tag_clean(void *addr, size_t size)
{
	__hwasan_tag_memory(addr, 0, size);
}
#else
#define untagged_addr(addr) (addr)
#endif

#endif

#endif /* __MT_SANITIZE__ */
