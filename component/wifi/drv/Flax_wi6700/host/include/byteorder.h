
#ifndef __LYNX_BYTEORDER_H__
#define __LYNX_BYTEORDER_H__

#define MT_LITTLE_ENDIAN	1
//#define MT_BIG_ENDIAN		1

#if defined(CONFIG_LYNX_OS_LINUX)
#include <linux/types.h>
#include <asm/byteorder.h>
#else
#define bswap_16(a) ((((u16) (a) << 8) & 0xff00) | (((u16) (a) >> 8) & 0xff))
#define swab16(x) ((u16)(							\
	(((u16)(x) & (u16)0x00ffU) << 8) |			\
	(((u16)(x) & (u16)0xff00U) >> 8)))
#define swab32(x) ((u32)(								\
	(((u32)(x) & (u32)0x000000ffUL) << 24) |		\
	(((u32)(x) & (u32)0x0000ff00UL) <<  8) |		\
	(((u32)(x) & (u32)0x00ff0000UL) >>  8) |		\
	(((u32)(x) & (u32)0xff000000UL) >> 24)))

static inline void swab16s(u16 *p)
{
	*p = swab16(*p);
}

static inline void swab32s(u32 *p)
{
	*p = swab32(*p);
}

#ifdef MT_LITTLE_ENDIAN

#define le16_to_cpu(x)	(x)
#define cpu_to_le16(x)	(x)
#define be16_to_cpu(x)	swab16(x)
#define cpu_to_be16(x)	swab16(x)
#define be32_to_cpu(x)	swab32(x)
#define cpu_to_be32(x)	swab32(x)
#define cpu_to_be16s(x)	swab16s(x)
#define cpu_to_be32s(x) swab32s(x)

#else	// MT_BIG_ENDIAN

#define le16_to_cpu(x)	bswap_16(x)
#define cpu_to_le16(x)	swab16(x)
#define be16_to_cpu(x)	((u16)(x))
#define cpu_to_be16(x)	((u16)(x))
#define be32_to_cpu(x)	((u32)(x))
#define cpu_to_be32(x)	((u32)(x))
#define cpu_to_be16s(x) do { (void)(x); } while (0)
#define cpu_to_be32s(x) do { (void)(x); } while (0)
#endif	// MT_LITTLE_ENDIAN

#define htons(x) cpu_to_be16(x)
#define htonl(x) cpu_to_be32(x)
#define ntohl(x) be32_to_cpu(x)

#ifdef __CHECKER__
#define __force __attribute__((force))
#define __bitwise __attribute__((bitwise))
#else
#define __force
#define __bitwise
#endif

typedef unsigned short __bitwise __le16;
typedef unsigned short __bitwise __be16;
typedef unsigned int __bitwise __le32;
typedef unsigned int __bitwise __be32;
typedef unsigned short __bitwise __sum16;

#endif	// !defined(CONFIG_LYNX_OS_LINUX)

#define READ_BE16(a) ((u16) (((a)[0] << 8) | ((a)[1]&0xff)))
#define WRITE_BE16(a, v)			\
	do {					\
		(a)[0] = ((u16) (v)) >> 8;	\
		(a)[1] = ((u16) (v)) & 0xff;	\
	} while (0)

#define READ_LE16(a) ((u16) (((a)[1] << 8) | ((a)[0]&0xff)))
#define WRITE_LE16(a, v)			\
	do {					\
		(a)[1] = ((u16) (v)) >> 8;	\
		(a)[0] = ((u16) (v)) & 0xff;	\
	} while (0)

#define READ_BE24(a) ((u32)(((a)[0] << 16) | ((a)[1] << 8) | \
			 ((a)[2]&0xff)))
#define WRITE_BE24(a, v)					\
	do {							\
		(a)[0] = (u8) ((((u32) (v)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (v)) >> 8) & 0xff);	\
		(a)[2] = (u8) (((u32) (v)) & 0xff);		\
	} while (0)

#define READ_BE32(a) ((u32) (((a)[0] << 24) | (((a)[1]&0xff) << 16) | \
			 (((a)[2]&0xff) << 8) | ((a)[3] & 0xff)))
#define WRITE_BE32(a, v)					\
	do {							\
		(a)[0] = (u8) ((((u32) (v)) >> 24) & 0xff);	\
		(a)[1] = (u8) ((((u32) (v)) >> 16) & 0xff);	\
		(a)[2] = (u8) ((((u32) (v)) >> 8) & 0xff);	\
		(a)[3] = (u8) (((u32) (v)) & 0xff);		\
	} while (0)

#define READ_LE32(a) ((u32) (((a)[3] << 24) | ((a)[2] << 16) | \
			 ((a)[1] << 8) | ((a)[0]&0xff)))
#define WRITE_LE32(a, v)					\
	do {							\
		(a)[3] = (u8) ((((u32) (v)) >> 24) & 0xff);	\
		(a)[2] = (u8) ((((u32) (v)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (v)) >> 8) & 0xff);	\
		(a)[0] = (u8) (((u32) (v)) & 0xff);		\
	} while (0)

#define READ_BE64(a) ((((u64) (a)[0]) << 56) | (((u64) (a)[1]) << 48) | \
			 (((u64) (a)[2]) << 40) | (((u64) (a)[3]) << 32) | \
			 (((u64) (a)[4]) << 24) | (((u64) (a)[5]) << 16) | \
			 (((u64) (a)[6]) << 8) | ((u64) (a)[7]&0xff))
#define WRITE_BE64(a, v)				\
	do {						\
		(a)[0] = (u8) (((u64) (v)) >> 56);	\
		(a)[1] = (u8) (((u64) (v)) >> 48);	\
		(a)[2] = (u8) (((u64) (v)) >> 40);	\
		(a)[3] = (u8) (((u64) (v)) >> 32);	\
		(a)[4] = (u8) (((u64) (v)) >> 24);	\
		(a)[5] = (u8) (((u64) (v)) >> 16);	\
		(a)[6] = (u8) (((u64) (v)) >> 8);	\
		(a)[7] = (u8) (((u64) (v)) & 0xff);	\
	} while (0)

#define READ_LE64(a) ((((u64) (a)[7]) << 56) | (((u64) (a)[6]) << 48) | \
			 (((u64) (a)[5]) << 40) | (((u64) (a)[4]) << 32) | \
			 (((u64) (a)[3]) << 24) | (((u64) (a)[2]) << 16) | \
			 (((u64) (a)[1]) << 8) | ((u64) (a)[0]&0xff))
#define WRITE_LE64(a, v)				\
	do {						\
		(a)[7] = (u8) (((u64) (v)) >> 56);	\
		(a)[6] = (u8) (((u64) (v)) >> 48);	\
		(a)[5] = (u8) (((u64) (v)) >> 40);	\
		(a)[4] = (u8) (((u64) (v)) >> 32);	\
		(a)[3] = (u8) (((u64) (v)) >> 24);	\
		(a)[2] = (u8) (((u64) (v)) >> 16);	\
		(a)[1] = (u8) (((u64) (v)) >> 8);	\
		(a)[0] = (u8) (((u64) (v)) & 0xff);	\
	} while (0)

#endif //__LYNX_BYTEORDER_H__
