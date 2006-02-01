/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

libc_hidden_proto(brk)

/* This must be initialized data because commons can't have aliases.  */
extern void * __curbrk;
libc_hidden_proto(__curbrk)
void * __curbrk = 0;
libc_hidden_data_def(__curbrk)

int brk (void *addr)
{
    void *newbrk;

	__asm__ __volatile__(
		"P0 = %2;\n\t"
		"R0 = %1;\n\t"
		"excpt 0;\n\t"
		"%0 = R0;\n\t"
		: "=r"(newbrk)
		: "r"(addr), "i" (__NR_brk): "P0" );

    __curbrk = newbrk;

    if (newbrk < addr)
    {
	__set_errno (ENOMEM);
	return -1;
    }

    return 0;
}
libc_hidden_def(brk)
