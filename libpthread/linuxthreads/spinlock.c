/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1998 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Internal locks */

#include <errno.h>
#include <sched.h>
#include <time.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"

/* The status field of a fastlock has the following meaning:
     0: fastlock is free
     1: fastlock is taken, no thread is waiting on it
  ADDR: fastlock is taken, ADDR is address of thread descriptor for
        first waiting thread, other waiting threads are linked via
        their p_nextlock field.
   The waiting list is not sorted by priority order.
   Actually, we always insert at top of list (sole insertion mode
   that can be performed without locking).
   For __pthread_unlock, we perform a linear search in the list
   to find the highest-priority, oldest waiting thread.
   This is safe because there are no concurrent __pthread_unlock
   operations -- only the thread that locked the mutex can unlock it. */

void internal_function __pthread_lock(struct _pthread_fastlock * lock,
				      pthread_descr self)
{
  long oldstatus, newstatus;
  int spurious_wakeup_count = 0;

  do {
    oldstatus = lock->__status;
    if (oldstatus == 0) {
      newstatus = 1;
    } else {
      if (self == NULL)
	self = thread_self();
      newstatus = (long) self;
    }
    if (self != NULL) {
      ASSERT(self->p_nextlock == NULL);
      THREAD_SETMEM(self, p_nextlock, (pthread_descr) oldstatus);
    }
  } while(! compare_and_swap(&lock->__status, oldstatus, newstatus,
                             &lock->__spinlock));

  /* Suspend with guard against spurious wakeup. 
     This can happen in pthread_cond_timedwait_relative, when the thread
     wakes up due to timeout and is still on the condvar queue, and then
     locks the queue to remove itself. At that point it may still be on the
     queue, and may be resumed by a condition signal. */

  if (oldstatus != 0) {
    for (;;) {
      suspend(self);
      if (self->p_nextlock != NULL) {
	/* Count resumes that don't belong to us. */
	spurious_wakeup_count++;
	continue;
      }
      break;
    }
  }

  /* Put back any resumes we caught that don't belong to us. */
  while (spurious_wakeup_count--)
    restart(self);
}

void internal_function __pthread_unlock(struct _pthread_fastlock * lock)
{
  long oldstatus;
  pthread_descr thr, * ptr, * maxptr;
  int maxprio;

again:
  oldstatus = lock->__status;
  if (oldstatus == 0 || oldstatus == 1) {
    /* No threads are waiting for this lock.  Please note that we also
       enter this case if the lock is not taken at all.  If this wouldn't
       be done here we would crash further down.  */
    if (! compare_and_swap(&lock->__status, oldstatus, 0, &lock->__spinlock))
      goto again;
    return;
  }
  /* Find thread in waiting queue with maximal priority */
  ptr = (pthread_descr *) &lock->__status;
  thr = (pthread_descr) oldstatus;
  maxprio = 0;
  maxptr = ptr;
  while (thr != (pthread_descr) 1) {
    if (thr->p_priority >= maxprio) {
      maxptr = ptr;
      maxprio = thr->p_priority;
    }
    ptr = &(thr->p_nextlock);
    thr = *ptr;
  }
  /* Remove max prio thread from waiting list. */
  if (maxptr == (pthread_descr *) &lock->__status) {
    /* If max prio thread is at head, remove it with compare-and-swap
       to guard against concurrent lock operation */
    thr = (pthread_descr) oldstatus;
    if (! compare_and_swap(&lock->__status,
                           oldstatus, (long)(thr->p_nextlock),
                           &lock->__spinlock))
      goto again;
  } else {
    /* No risk of concurrent access, remove max prio thread normally */
    thr = *maxptr;
    *maxptr = thr->p_nextlock;
  }
  /* Wake up the selected waiting thread */
  thr->p_nextlock = NULL;
  restart(thr);
}

/* Compare-and-swap emulation with a spinlock */

#ifdef TEST_FOR_COMPARE_AND_SWAP
int __pthread_has_cas = 0;
#endif

#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP

static void __pthread_acquire(int * spinlock);

int __pthread_compare_and_swap(long * ptr, long oldval, long newval,
                               int * spinlock)
{
  int res;
  if (testandset(spinlock)) __pthread_acquire(spinlock);
  if (*ptr == oldval) {
    *ptr = newval; res = 1;
  } else {
    res = 0;
  }
  *spinlock = 0;
  return res;
}

/* This function is called if the inlined test-and-set
   in __pthread_compare_and_swap() failed */

/* The retry strategy is as follows:
   - We test and set the spinlock MAX_SPIN_COUNT times, calling
     sched_yield() each time.  This gives ample opportunity for other
     threads with priority >= our priority to make progress and
     release the spinlock.
   - If a thread with priority < our priority owns the spinlock,
     calling sched_yield() repeatedly is useless, since we're preventing
     the owning thread from making progress and releasing the spinlock.
     So, after MAX_SPIN_LOCK attemps, we suspend the calling thread
     using nanosleep().  This again should give time to the owning thread
     for releasing the spinlock.
     Notice that the nanosleep() interval must not be too small,
     since the kernel does busy-waiting for short intervals in a realtime
     process (!).  The smallest duration that guarantees thread
     suspension is currently 2ms.
   - When nanosleep() returns, we try again, doing MAX_SPIN_COUNT
     sched_yield(), then sleeping again if needed. */

static void __pthread_acquire(int * spinlock)
{
  int cnt = 0;
  struct timespec tm;

  while (testandset(spinlock)) {
    if (cnt < MAX_SPIN_COUNT) {
      sched_yield();
      cnt++;
    } else {
      tm.tv_sec = 0;
      tm.tv_nsec = SPIN_SLEEP_DURATION;
      nanosleep(&tm, NULL);
      cnt = 0;
    }
  }
}

#endif
