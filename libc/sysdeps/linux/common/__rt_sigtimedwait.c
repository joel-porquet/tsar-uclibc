/* vi: set sw=4 ts=4: */
/*
 * __rt_sigtimedwait() for uClibc
 *
 * Copyright (C) 2006 by Steven Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include <sys/syscall.h>

#ifdef __NR_rt_sigtimedwait
# include <signal.h>
# include <cancel.h>
# ifdef SIGCANCEL /* defined only in NPTL's pthreadP.h */
#  define __need_NULL
#  include <stddef.h>
#  include <string.h>
# endif

int __NC(sigtimedwait)(const sigset_t *set, siginfo_t *info,
		       const struct timespec *timeout)
{
# ifdef SIGCANCEL
	sigset_t tmpset;

	if (set != NULL && (__builtin_expect (__sigismember (set, SIGCANCEL), 0)
#  ifdef SIGSETXID
		|| __builtin_expect (__sigismember (set, SIGSETXID), 0)
#  endif
		))
	{
		/* Create a temporary mask without the bit for SIGCANCEL set.  */
		// We are not copying more than we have to.
		memcpy (&tmpset, set, _NSIG / 8);
		__sigdelset (&tmpset, SIGCANCEL);
#  ifdef SIGSETXID
		__sigdelset (&tmpset, SIGSETXID);
#  endif
		set = &tmpset;
	}
# endif

/* if this is enabled, enable the disabled section in sigwait.c */
# if defined SI_TKILL && defined SI_USER
	/* XXX The size argument hopefully will have to be changed to the
	   real size of the user-level sigset_t.  */
	/* on uClibc we use the kernel sigset_t size */
	int result = INLINE_SYSCALL(rt_sigtimedwait, 4, set, info,
				    timeout, __SYSCALL_SIGSET_T_SIZE);

	/* The kernel generates a SI_TKILL code in si_code in case tkill is
	   used.  tkill is transparently used in raise().  Since having
	   SI_TKILL as a code is useful in general we fold the results
	   here.  */
	if (result != -1 && info != NULL && info->si_code == SI_TKILL)
		info->si_code = SI_USER;

	return result;
# else
	/* on uClibc we use the kernel sigset_t size */
	return INLINE_SYSCALL(rt_sigtimedwait, 4, set, info,
			      timeout, __SYSCALL_SIGSET_T_SIZE);
# endif
}
CANCELLABLE_SYSCALL(int, sigtimedwait,
		    (const sigset_t *set, siginfo_t *info, const struct timespec *timeout),
		    (set, info, timeout))
lt_libc_hidden(sigtimedwait)
#endif
