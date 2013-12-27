#include <sysdep.h>
#include <sys/asm.h>
#include <sys/regdef.h>

#define _ERRNO_H        1
#include <bits/errno.h>
#ifdef RESET_PID
#include <tls.h>
#endif

#define CLONE_VM      0x00000100
#define CLONE_THREAD  0x00010000

NESTED(clone, 32, sp)
#ifdef __PIC__
	SETUP_GP
#endif
	addiu	sp, -32
#ifdef __PIC__
	SAVE_GP((32 - 4))
#endif
	/* check arguments */
	li	v0, -EINVAL
	beqz	a0, error
	beqz	a1, error

	/* create room on child's stack */
	subu	a1, 32
	/* save function pointer */
	sw	a0, 0(a1)
	/* save argument */
	sw	a3, 4(a1)
#ifdef RESET_PID
	/* save clone flags */
	sw	a2, 8(a2)
#endif

	/* start rearraging arguments: arg 3 becomes arg 1 */
	move	a0, a2
	/* arg 7 becomes arg 5 */
	lw	a2, (32 + 16 + 8)(sp)
	sw	a2, 16(sp)
	/* arg 6 becomes arg 4 */
	lw	a3, (32 + 16 + 4)(sp)
	/* arg 5 becomes arg 3 */
	lw	a2, (32 + 16)(sp)

	/* do the system call */
	li	v0, __NR_clone
	syscall

	/* check for errors */
	sltiu	t0, v0, -4095
	beqz	t0, error

	/* go to child initialization */
	beqz	v0, thread_start

	/* otherwise succesful return from parent */
	addiu	sp, 32
	jr	ra

error:
	addiu	sp, 32
	move	a0, v0
#ifdef __PIC__
	la	t9, __syscall_error
	jr	t9
#else
	j	__syscall_error
#endif

thread_start:
	SAVE_GP((32 - 4))

#ifdef RESET_PID
	/* check the flags */
	lw	a0, 8(sp)
	and	a1, a0, CLONE_THREAD
	bnez	a1, donepid
	and	a1, a0, CLONE_VM
	li	v0, -1
	bnez	a1, gotpid
	li	v0, __NR_getpid
	syscall
gotpid:
	READ_THREAD_POINTER(v1)
	sw	v0, PID_OFFSET(v1)
	sw	v0, TID_OFFSET(v1)
donepid:
#endif

	/* get function pointer */
	lw	t9, 0(sp)
	/* and argument */
	lw	a0, 4(sp)

	/* call the user's function */
	jal	t9

	/* call exit when the user returns */
	move	a0, v0
#ifdef __PIC__
	la	t9, _exit
	jalr	t9
#else
	jal	_exit
#endif
END(clone)
weak_alias(clone, __clone)
