/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

char attribute_hidden *__basename(const char *path)
{
	register const char *s;
	register const char *p;

	p = s = path;

	while (*s) {
		if (*s++ == '/') {
			p = s;
		}
	}

	return (char *) p;
}

strong_alias(__basename,basename)
