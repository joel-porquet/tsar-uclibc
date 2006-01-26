/* vi: set sw=4 ts=4: */
/*
 * prctl() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "syscalls.h"
#include <stdarg.h>
/* psm: including sys/prctl.h would depend on kernel headers */
extern int prctl (int, int, int, int, int);
_syscall5(int, prctl, int, a, int, b, int, c, int, d, int, e);
