/* Copyright (C) 1992,1993,1995,1996,1997,1998,1999,2000,2002,2003,2004
	Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>, August 1995.
   Changed by Kaz Kojima, <kkojima@rr.iij4u.or.jp>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LINUX_SH_SYSDEP_H
#define _LINUX_SH_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/sh/sysdep.h>

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	(__NR_##syscall_name)


#ifdef __ASSEMBLER__

/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in R0
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can savely
   test with -4095.  */

#define _IMM1 #-1
#define _IMM12 #-12
#undef	PSEUDO
#define	PSEUDO(name, syscall_name, args) \
 .text; \
 ENTRY (name); \
    DO_CALL (syscall_name, args); \
    mov r0,r1; \
    mov _IMM12,r2; \
    shad r2,r1; \
    not r1,r1; \
    tst r1,r1; \
    bf .Lpseudo_end; \
    SYSCALL_ERROR_HANDLER; \
 .Lpseudo_end:

#undef	PSEUDO_END
#define	PSEUDO_END(name) \
  END (name)

#undef	PSEUDO_NOERRNO
#define	PSEUDO_NOERRNO(name, syscall_name, args) \
 .text; \
 ENTRY (name); \
    DO_CALL (syscall_name, args)

#undef	PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name) \
  END (name)

#define ret_NOERRNO ret

#define	PSEUDO_ERRVAL(name, syscall_name, args) \
 .text; \
 ENTRY (name); \
    DO_CALL (syscall_name, args);

#undef	PSEUDO_END_ERRVAL
#define	PSEUDO_END_ERRVAL(name) \
  END (name)

#define ret_ERRVAL ret

#ifndef __PIC__
# define SYSCALL_ERROR_HANDLER	\
	mov.l 0f,r1; \
	jmp @r1; \
	 mov r0,r4; \
	.align 2; \
     0: .long __syscall_error

#include <libc/sysdeps/linux/sh/syscall_error.S>
#else
# ifdef RTLD_PRIVATE_ERRNO

#  define SYSCALL_ERROR_HANDLER	\
	neg r0,r1; \
	mov.l 0f,r12; \
	mova 0f,r0; \
	add r0,r12; \
	mov.l 1f,r0; \
	mov.l r1,@(r0,r12)
	bra .Lpseudo_end; \
	 mov _IMM1,r0; \
	.align 2; \
     0: .long _GLOBAL_OFFSET_TABLE_; \
     1: .long rtld_errno@GOTOFF

# elif defined _LIBC_REENTRANT

#  if USE___THREAD

#   ifndef NOT_IN_libc
#    define SYSCALL_ERROR_ERRNO __libc_errno
#   else
#    define SYSCALL_ERROR_ERRNO errno
#   endif
#   define SYSCALL_ERROR_HANDLER \
	neg r0,r1; \
	mov r12,r2; \
	mov.l 0f,r12; \
	mova 0f,r0; \
	add r0,r12; \
	mov.l 1f,r0; \
	stc gbr, r4; \
	mov.l @(r0,r12),r0; \
	bra .Lskip; \
	add r4,r0; \
	.align 2; \
	1: .long SYSCALL_ERROR_ERRNO@GOTTPOFF; \
	.Lskip: \
	mov r2,r12; \
	mov.l r1,@r0; \
	bra .Lpseudo_end; \
	mov _IMM1,r0; \
	.align 2; \
	0: .long _GLOBAL_OFFSET_TABLE_
#  else

#   define SYSCALL_ERROR_HANDLER \
	neg r0,r1; \
	mov.l r14,@-r15; \
	mov.l r12,@-r15; \
	mov.l r1,@-r15; \
	mov.l 0f,r12; \
	mova 0f,r0; \
	add r0,r12; \
	sts.l pr,@-r15; \
	mov r15,r14; \
	mov.l 1f,r1; \
	bsrf r1; \
         nop; \
     2: mov r14,r15; \
	lds.l @r15+,pr; \
	mov.l @r15+,r1; \
	mov.l r1,@r0; \
	mov.l @r15+,r12; \
	mov.l @r15+,r14; \
	bra .Lpseudo_end; \
	 mov _IMM1,r0; \
	.align 2; \
     0: .long _GLOBAL_OFFSET_TABLE_; \
     1: .long PLTJMP(C_SYMBOL_NAME(__errno_location))-(2b-.)
/* A quick note: it is assumed that the call to `__errno_location' does
   not modify the stack!  */
#  endif
# else

/* Store (-r0) into errno through the GOT.  */
#  define SYSCALL_ERROR_HANDLER						      \
	neg r0,r1; \
	mov r12,r2; \
	mov.l 0f,r12; \
	mova 0f,r0; \
	add r0,r12; \
	mov.l 1f,r0; \
	mov.l @(r0,r12),r0; \
	mov r2,r12; \
	mov.l r1,@r0; \
	bra .Lpseudo_end; \
	 mov _IMM1,r0; \
	.align 2; \
     0: .long _GLOBAL_OFFSET_TABLE_; \
     1: .long errno@GOT
# endif	/* _LIBC_REENTRANT */
#endif	/* __PIC__ */

# ifdef NEED_SYSCALL_INST_PAD
#  define SYSCALL_INST_PAD \
	or r0,r0; or r0,r0; or r0,r0; or r0,r0; or r0,r0
# else
#  define SYSCALL_INST_PAD
# endif

#define SYSCALL_INST0	trapa #0x10
#define SYSCALL_INST1	trapa #0x11
#define SYSCALL_INST2	trapa #0x12
#define SYSCALL_INST3	trapa #0x13
#define SYSCALL_INST4	trapa #0x14
#define SYSCALL_INST5	mov.l @(0,r15),r0; trapa #0x15
#define SYSCALL_INST6	mov.l @(0,r15),r0; mov.l @(4,r15),r1; trapa #0x16

#undef	DO_CALL
#define DO_CALL(syscall_name, args)	\
    mov.l 1f,r3;			\
    SYSCALL_INST##args;			\
    SYSCALL_INST_PAD;			\
    bra 2f;				\
     nop;				\
    .align 2;				\
 1: .long SYS_ify (syscall_name);	\
 2:

#endif	/* __ASSEMBLER__ */

#endif /* linux/sh/sysdep.h */
