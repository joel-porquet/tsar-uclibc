/*
 * _init epilog
 */
.section .init
.globl	_init
.type	_init, @function
	lw	$28, 24($sp)
	lw	$31, 28($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp, $sp, 32
	.set	macro
	.set	reorder

/*
 * _fini epilog
 */
.section .fini
.globl	_fini
.type	_fini, @function
	lw	$28, 24($sp)
	lw	$31, 28($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp, $sp, 32
	.set	macro
	.set	reorder
