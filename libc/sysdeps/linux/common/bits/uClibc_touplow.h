/*  Copyright (C) 2003     Manuel Novoa III
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
 */

/*  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!
 *
 *  Besides uClibc, I'm using this code in my libc for elks, which is
 *  a 16-bit environment with a fairly limited compiler.  It would make
 *  things much easier for me if this file isn't modified unnecessarily.
 *  In particular, please put any new or replacement functions somewhere
 *  else, and modify the makefile to use your version instead.
 *  Thanks.  Manuel
 *
 *  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION! */

#ifndef _UCLIBC_TOUPLOW_H
#define _UCLIBC_TOUPLOW_H

#include <features.h>
#include <bits/types.h>

/* glibc uses the equivalent of - typedef __int32_t __ctype_touplow_t; */

#ifdef __UCLIBC_HAS_CTYPE_SIGNED__
typedef __int16_t __ctype_touplow_t;
#else  /* __UCLIBC_HAS_CTYPE_SIGNED__ */
typedef unsigned char __ctype_touplow_t;
#endif /* __UCLIBC_HAS_CTYPE_SIGNED__ */

#endif /* _UCLIBC_TOUPLOW_H */

