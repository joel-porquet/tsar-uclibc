/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifdef __DO_UNLOCKED

wint_t __putwchar_unlocked(wchar_t wc)
{
	return __fputwc_unlocked(wc, stdout);
}

weak_alias(__putwchar_unlocked,putwchar_unlocked)
#ifndef __UCLIBC_HAS_THREADS__
weak_alias(__putwchar_unlocked,putwchar)
#endif

#elif defined __UCLIBC_HAS_THREADS__

extern int __fputc (int __c, FILE *__stream) attribute_hidden;

wint_t putwchar(wchar_t wc)
{
	return __fputc(wc, stdout);
}

#endif
