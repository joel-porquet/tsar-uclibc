/*
 * fgetspent.c - Based on fgetpwent.c
 * 
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <errno.h>
#include <stdio.h>
#include <shadow.h>

#define PWD_BUFFER_SIZE 256

int fgetspent_r (FILE *file, struct spwd *spwd,
	char *buff, size_t buflen, struct spwd **crap)
{
	if (file == NULL) {
		__set_errno(EINTR);
		return -1;
	}
	return(__getspent_r(spwd, buff, buflen, fileno(file)));
}

struct spwd *fgetspent(FILE * file)
{
	static char line_buff[PWD_BUFFER_SIZE];
	static struct spwd spwd;

	if (fgetspent_r(file, &spwd, line_buff, PWD_BUFFER_SIZE, NULL) != -1) {
		return &spwd;
	}
	return NULL;
}
