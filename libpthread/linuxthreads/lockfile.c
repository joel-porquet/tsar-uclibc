/* lockfile - Handle locking and unlocking of stream.
   Copyright (C) 1996, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <bits/libc-lock.h>
#include <stdio.h>
#include <pthread.h>

#ifdef USE_IN_LIBIO
#include "../libio/libioP.h"
#endif

void
__flockfile (FILE *stream)
{
#ifdef USE_IN_LIBIO
  __pthread_mutex_lock (stream->_lock);
#else
#endif
}
#ifdef USE_IN_LIBIO
#undef _IO_flockfile
strong_alias (__flockfile, _IO_flockfile)
#endif
weak_alias (__flockfile, flockfile);


void
__funlockfile (FILE *stream)
{
#ifdef USE_IN_LIBIO
  __pthread_mutex_unlock (stream->_lock);
#else
#endif
}
#ifdef USE_IN_LIBIO
#undef _IO_funlockfile
strong_alias (__funlockfile, _IO_funlockfile)
#endif
weak_alias (__funlockfile, funlockfile);


int
__ftrylockfile (FILE *stream)
{
#ifdef USE_IN_LIBIO
  return __pthread_mutex_trylock (stream->_lock);
#else
  return 0;
#endif
}
#ifdef USE_IN_LIBIO
strong_alias (__ftrylockfile, _IO_ftrylockfile)
#endif
weak_alias (__ftrylockfile, ftrylockfile);


void
__fresetlockfiles (void)
{
#ifdef USE_IN_LIBIO
  _IO_FILE *fp;
  pthread_mutexattr_t attr;

  __pthread_mutexattr_init (&attr);
  __pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE_NP);

  for (fp = _IO_list_all; fp != NULL; fp = fp->_chain)
    __pthread_mutex_init (fp->_lock, &attr);

  __pthread_mutexattr_destroy (&attr);
#endif
}
