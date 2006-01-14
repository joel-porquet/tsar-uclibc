/* vi: set sw=4 ts=4: */
/*
 * lstat() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

/* need to hide the 64bit prototype or the weak_alias()
 * will fail when __NR_lstat64 doesnt exist */
#define lstat64 __hidelstat64
#define __lstat64 __hide__lstat64

#include "syscalls.h"
#include <unistd.h>
#include <sys/stat.h>
#include "xstatconv.h"

#undef lstat64
#undef __lstat64

#define __NR___syscall_lstat __NR_lstat
#undef __lstat
#undef lstat
static inline _syscall2(int, __syscall_lstat,
		const char *, file_name, struct kernel_stat *, buf);

int attribute_hidden __lstat(const char *file_name, struct stat *buf)
{
	int result;
	struct kernel_stat kbuf;

	result = __syscall_lstat(file_name, &kbuf);
	if (result == 0) {
		__xstat_conv(&kbuf, buf);
	}
	return result;
}
strong_alias(__lstat,lstat)

#if ! defined __NR_lstat64 && defined __UCLIBC_HAS_LFS__
hidden_strong_alias(__lstat,__lstat64)
weak_alias(lstat,lstat64)
#endif
