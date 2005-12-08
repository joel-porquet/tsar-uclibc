/* vi: set sw=4 ts=4: */
/*
 * time() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#define gettimeofday __gettimeofday

#include "syscalls.h"
#include <time.h>
#include <sys/time.h>
#ifdef __NR_time
#define __NR___time __NR_time
attribute_hidden _syscall1(time_t, __time, time_t *, t);
#else
time_t attribute_hidden __time(time_t * t)
{
	time_t result;
	struct timeval tv;

	if (gettimeofday(&tv, (struct timezone *) NULL)) {
		result = (time_t) - 1;
	} else {
		result = (time_t) tv.tv_sec;
	}
	if (t != NULL) {
		*t = result;
	}
	return result;
}
#endif
strong_alias(__time,time)
