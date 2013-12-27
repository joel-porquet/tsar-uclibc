#include <sysdep.h>
#include <sys/asm.h>

.text
NESTED(_dl_runtime_resolve, 40, ra)
	.set noreorder

	/* Save GP. */
	move	v1, gp

	/* Save arguments and sp value on stack. */
	subu	sp, 40

	/* Modify t9 ($25) so as to point .cpload instruction. */
	addiu	t9, 12

	/* Compute GP. */
	SETUP_GP

	/* Save slot call pc. */
	move	v0, ra
	.cprestore 32

	/* Store function arguments from registers to stack */
	sw	t7, 36(sp)
	sw	a0, 16(sp)
	sw	a1, 20(sp)
	sw	a2, 24(sp)
	sw	a3, 28(sp)

	/* Setup functions args and call __dl_runtime_resolve */
	move	a0, t8
	move	a1, v1
	jal	__dl_runtime_resolve

	/* Restore function arguments from stack to registers */
	lw	ra, 36(sp)
	lw	a0, 16(sp)
	lw	a1, 20(sp)
	lw	a2, 24(sp)
	lw	a3, 28(sp)

	/* Do a tail call to the original function */
	addiu	sp, 40
	move	t9, v0
	jr	t9
END(_dl_runtime_resolve)
.previous

/* Assembler veneer called from the PLT header code when using the
 * non-PIC ABI.
 *
 * Code in each PLT entry puts the caller's return address into t7 ($15),
 * the PLT entry index into t8 ($24), the address of
 * _dl_runtime_pltresolve into t9 ($25) and the address of .got.plt into
 * gp ($28).  __dl_runtime_pltresolve needs a0 ($4) to hold the link map
 * and a1 ($5) to hold the index into .rel.plt (== PLT entry index * 4).
 */

.text
NESTED(_dl_runtime_pltresolve, 40, ra)
	.set noreorder

	# Save arguments and sp value in stack.
	subu    sp, 40
	lw      t2, 4(gp)
	# Modify t9 ($25) so as to point .cpload instruction.
	addiu   t9, 12
	# Compute GP.
	SETUP_GP

	/* Store function arguments from registers to stack */
	sw	t7, 36(sp)
	sw	a0, 16(sp)
	sw	a1, 20(sp)
	sw	a2, 24(sp)
	sw	a3, 28(sp)

	/* Setup functions args and call __dl_runtime_pltresolve.  */
	move	a0, t2
	sll     a1, t8, 3
	jal	__dl_runtime_pltresolve

	/* Restore function arguments from stack to registers */
	lw	ra, 36(sp)
	lw	a0, 16(sp)
	lw	a1, 20(sp)
	lw	a2, 24(sp)
	lw	a3, 28(sp)

	/* Do a tail call to the original function */
	addiu	sp, 40
	move	t9, v0
	jr	t9
END(_dl_runtime_pltresolve)
.previous
