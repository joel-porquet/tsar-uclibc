/* vi: set sw=4 ts=4: */
/*
 * tee() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <fcntl.h>

libc_hidden_proto(tee)

#ifdef __NR_tee
_syscall4(ssize_t, tee, int, __fdin, int, __fdout, size_t, __len,
	unsigned int, __flags);
#else
ssize_t tee(int __fdin, int __fdout, size_t __len, unsigned int __flags)
{
	__set_errno(ENOSYS);
	return -1;
}
#endif

libc_hidden_def(tee)


