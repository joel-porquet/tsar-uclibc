#include <features.h>
#include <tls.h>

/* Save the PID value.  */
#define SAVE_PID                                                       \
	READ_THREAD_POINTER(v1);	/* Get the thread pointer.  */ \
	lw	a2, PID_OFFSET(v1);	/* Load the saved PID.  */     \
	subu	a2, $0, a2;		/* Negate it.  */              \
	bnez	a2, 1f;			/* If it was zero... */        \
	lui	a2, 0x8000;		/* use 0x80000000 instead.  */ \
1:	sw	a2, PID_OFFSET(v1);	/* Store the temporary PID.  */

/* Restore the old PID value in the parent.  */
#define RESTORE_PID                                                     \
	beqz	v0, 1f;			/* If we are the parent... */   \
	READ_THREAD_POINTER(v1);	/* Get the thread pointer.  */  \
	lw	a2, PID_OFFSET(v1);	/* Load the saved PID.  */      \
	subu	a2, $0, a2;		/* Re-negate it.  */            \
	lui	a0, 0x8000;		/* Load 0x80000000... */        \
	bne	a2, a0, 2f;		/* ... compare against it... */ \
	li	a2, 0;			/* ... use 0 instead.  */       \
2:	sw	a2, PID_OFFSET(v1);	/* Restore the PID.  */         \
1:

#include <../../../../../../../libc/sysdeps/linux/tsar/vfork.S>
