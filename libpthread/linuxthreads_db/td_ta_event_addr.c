/* Get event address.
   Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

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

//#include <gnu/lib-names.h>

#include "thread_dbP.h"


td_err_e
td_ta_event_addr (const td_thragent_t *ta, td_event_e event, td_notify_t *addr)
{
  td_err_e res = TD_NOEVENT;
  const char *symbol = NULL;

  LOG (__FUNCTION__);

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  switch (event)
    {
    case TD_CREATE:
      symbol = "__linuxthreads_create_event";
      break;

    case TD_DEATH:
      symbol = "__linuxthreads_death_event";
      break;

    case TD_REAP:
      symbol = "__linuxthreads_reap_event";
      break;

    default:
      /* Event cannot be handled.  */
      break;
    }

  /* Now get the address.  */
  if (symbol != NULL)
    {
      psaddr_t taddr;

      if (ps_pglobal_lookup (ta->ph, LIBPTHREAD_SO, symbol, &taddr) == PS_OK)
	{
	  /* Success, we got the address.  */
	  addr->type = NOTIFY_BPT;
	  addr->u.bptaddr = taddr;

	  res = TD_OK;
	}
      else
	res = TD_ERR;
    }

  return res;
}
