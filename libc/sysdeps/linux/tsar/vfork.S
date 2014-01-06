/*
 * according to mips, vfork encapsulate a call to clone
 */

#include <sysdep.h>
#include <sys/asm.h>
#include <sys/regdef.h>

#define _ERRNO_H        1
#include <bits/errno.h>
#ifdef RESET_PID
#include <tls.h>
#endif

#ifndef SAVE_PID
#define SAVE_PID
#endif

#ifndef RESTORE_PID
#define RESTORE_PID
#endif

.text
.hidden __vfork

NESTED(__vfork, 32, sp)
#ifdef __PIC__
	SETUP_GP
#endif
	addiu	sp, -32
#ifdef __PIC__
	SAVE_GP((32 - 4))
#endif

	SAVE_PID

	/* load the arguments for syscall */
	li	a0, 0x4112	/* CLONE_VM | CLONE_VFORK | SIGCHLD */
	move	a1, sp

	/* do the system call */
	li	v0, __NR_clone
	syscall

	RESTORE_PID

	/* check for errors */
	sltiu	t0, v0, -4095
	beqz	t0, error

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
END(__vfork)

weak_alias(__vfork,vfork)
libc_hidden_def(vfork)
