/* vi: set sw=4 ts=4: */
/*
 * getppid() for uClibc
 *
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#ifdef	__NR_getppid
_syscall0(pid_t, getppid);
#else
libc_hidden_proto(getpid)
pid_t getppid(void)
{
	return getpid();
}
#endif
