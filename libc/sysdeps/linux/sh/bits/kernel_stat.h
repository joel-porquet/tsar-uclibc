#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks 
 * struct stat should look like...  It turns out each arch has a 
 * different opinion on the subject... */
#include <endian.h>

struct stat {
	unsigned short st_dev;
	unsigned short __pad1;
	unsigned long st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned short st_rdev;
	unsigned short __pad2;
	unsigned long  st_size;
	unsigned long  st_blksize;
	unsigned long  st_blocks;
	unsigned long  st_atime;
	unsigned long  __unused1;
	unsigned long  st_mtime;
	unsigned long  __unused2;
	unsigned long  st_ctime;
	unsigned long  __unused3;
	unsigned long  __unused4;
	unsigned long  __unused5;
};
#ifdef __USE_LARGEFILE64
struct stat64 {
#if defined(__BIG_ENDIAN__)
	unsigned char   __pad0b[6];
	unsigned short	st_dev;
#elif defined(__LITTLE_ENDIAN__)
	unsigned short	st_dev;
	unsigned char	__pad0b[6];
#else
#error Must know endian to build stat64 structure!
#endif
	unsigned char	__pad0[4];

	unsigned long	st_ino;
	unsigned int	st_mode;
	unsigned int	st_nlink;

	unsigned long	st_uid;
	unsigned long	st_gid;

#if defined(__BIG_ENDIAN__)
	unsigned char	__pad3b[6];
	unsigned short	st_rdev;
#else /* Must be little */
	unsigned short	st_rdev;
	unsigned char	__pad3b[6];
#endif
	unsigned char	__pad3[4];

	long long	st_size;
	unsigned long	st_blksize;

#if defined(__BIG_ENDIAN__)
	unsigned long	__pad4;		/* Future possible st_blocks hi bits */
	unsigned long	st_blocks;	/* Number 512-byte blocks allocated. */
#else /* Must be little */
	unsigned long	st_blocks;	/* Number 512-byte blocks allocated. */
	unsigned long	__pad4;		/* Future possible st_blocks hi bits */
#endif

	unsigned long	st_atime;
	unsigned long	__pad5;

	unsigned long	st_mtime;
	unsigned long	__pad6;

	unsigned long	st_ctime;
	unsigned long	__pad7;		/* will be high 32 bits of ctime someday */

	unsigned long	__unused1;
	unsigned long	__unused2;
};
#endif


#endif	/*  _BITS_STAT_STRUCT_H */

