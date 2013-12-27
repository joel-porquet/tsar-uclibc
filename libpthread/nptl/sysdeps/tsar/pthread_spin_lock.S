#include <sys/asm.h>
#include <sysdep.h>

#define _ERRNO_H 1
#include <bits/errno.h>

/*
 * int pthread_spin_lock (pthread_spinlock_t *lock);
 */

LEAF(pthread_spin_lock)
1:
	/* get current lock value */
	ll	a2, 0(a0)
	/* if the lock is taken then spin */
	bnez	a2, 1b
	/* else try to take the lock or spin again */
	li	a1, 1
	sc	a1, 0(a0)
	beqz	a1, 1b
	sync
	/* return 0 */
	li	v0, 0
	j	ra
	nop
END(pthread_spin_lock)

/*
 * int pthread_spin_trylock (pthread_spinlock_t *lock);
 */

LEAF(pthread_spin_trylock)
	/* get current lock value */
	ll	a2, 0(a0)
	/* if the lock is taken then return error */
	bnez	a2, 1f
	/* try to take the lock or return error */
	li	a1, 1
	sc	a1, 0(a0)
	beqz	a1, 1f
	sync
	/* return 0 */
	li	v0, 0
	j	ra
1:
	/* return EBUSY */
	li	v0, EBUSY
	j	ra
END(pthread_spin_trylock)
