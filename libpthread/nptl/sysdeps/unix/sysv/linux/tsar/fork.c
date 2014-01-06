#include <sched.h>
#include <signal.h>
#include <sysdep.h>
#include <tls.h>

/* syscall to clone, respecting the CLONE_BACKWARD convention */
#define ARCH_FORK()                                                          \
	INLINE_SYSCALL (clone, 5,                                            \
			CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD, \
			NULL, NULL, NULL, &THREAD_SELF->tid)

#include "../fork.c"
