#include <sys/regdef.h>
#include <sys/asm.h>
#include <sysdep.h>
#include <features.h>


/* This is the canonical entry point, usually the first thing in the text
   segment.  The SVR4/Mips ABI (pages 3-31, 3-32) says that when the entry
   point runs, most registers' values are unspecified, except for:

   v0 ($2)	Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   sp ($29)	The stack contains the arguments and environment:
		0(%esp)			argc
		4(%esp)			argv[0]
		...
		(4*argc)(%esp)		NULL
		(4*(argc+1))(%esp)	envp[0]
		...
					NULL
   ra ($31)	The return address register is set to zero so that programs
		that search backword through stack frames recognize the last
		stack frame.
*/


/* We need to call:
   __uClibc_main (int (*main) (int, char **, char **), int argc,
		      char **argv, void (*init) (void), void (*fini) (void),
		      void (*rtld_fini) (void), void *stack_end)
*/

.text
	.type	_init, @function
	.type	_fini, @function
#ifndef __UCLIBC_CTOR_DTOR__
	.weak	_init
	.weak	_fini
#endif
	.type	main, @function
	.type	__uClibc_main, @function

NESTED(__start, 32, sp)
#ifdef __PIC__
	SETUP_GPX(zero)
#else
	la	gp, _gp	/* Setup GP correctly if we're non-PIC.  */
	move	ra, zero
#endif

	la	a0, main	/* main */
	lw	a1, 0(sp)	/* argc */
	addiu	a2, sp, 4	/* argv  */

	/* Allocate space on the stack for height arguments and
	 * make sure the stack is aligned to double words (8 bytes) */

	and	sp, -2 * 4

	subu	sp, 32
	la	a3, _init	/* init */
	la	t0, _fini
	sw	t0, 16(sp)	/* fini */
	sw	v0, 20(sp)	/* rtld_fini */
	sw	sp, 24(sp)	/* stack_end */

	jal	__uClibc_main
hlt:
	/* Crash if somehow `__uClibc_main' returns anyway.  */
	b	hlt
END(__start)

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start


