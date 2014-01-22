#include <sys/asm.h>
#include <sys/regdef.h>

/*
 * _init prolog
 */
.section .init
NESTED(_init, 32, ra)
	.set	noreorder
	.cpload	t9
	.set	reorder
	subu	sp, sp, 32
	.cprestore 16
	sw	gp, 24(sp)
	sw	ra, 28(sp)
	.end	_init

/*
 * _fini prolog
 */
.section .fini
NESTED(_fini, 32, ra)
	.set	noreorder
	.cpload	t9
	.set	reorder
	subu	sp, sp, 32
	.cprestore 16
	sw	gp, 24(sp)
	sw	ra, 28(sp)
	.end	_fini
