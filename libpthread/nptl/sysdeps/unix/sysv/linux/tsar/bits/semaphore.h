#ifndef _SEMAPHORE_H
# error "Never use <bits/semaphore.h> directly; include <semaphore.h> instead."
#endif

# define __SIZEOF_SEM_T	16

/* Value returned if `sem_open' failed.  */
#define SEM_FAILED ((sem_t *) 0)

typedef union
{
	char __size[__SIZEOF_SEM_T];
	long int __align;
} sem_t;
