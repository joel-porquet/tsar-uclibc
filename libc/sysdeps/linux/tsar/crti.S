/*
 * _init prolog
 */
.section .init
.globl	_init
.ent	_init
.type	_init, @function
_init:
	.frame	$sp, 32, $31
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp, $sp, 32
	.cprestore 16
	sw	$28, 24($sp)
	sw	$31, 28($sp)
	.end	_init

/*
 * _fini prolog
 */
.section .fini
.globl	_fini
.ent	_fini
.type	_fini, @function
_fini:
	.frame	$sp, 32, $31
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp, $sp, 32
	.cprestore 16
	sw	$28, 24($sp)
	sw	$31, 28($sp)
	.end _fini
