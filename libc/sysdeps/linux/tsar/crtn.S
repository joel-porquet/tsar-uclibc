#include <sys/regdef.h>

/*
 * _init epilog
 */
.section .init
.globl	_init
.type	_init, @function
	//lw	gp, 24(sp)
	lw	ra, 28(sp)
	.set	noreorder
	.set	nomacro
	j	ra
	addu	sp, sp, 32
	.set	macro
	.set	reorder

/*
 * _fini epilog
 */
.section .fini
.globl	_fini
.type	_fini, @function
	//lw	gp, 24(sp)
	lw	ra, 28(sp)
	.set	noreorder
	.set	nomacro
	j	ra
	addu	sp, sp, 32
	.set	macro
	.set	reorder
