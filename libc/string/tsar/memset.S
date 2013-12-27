#include <sysdep.h>

/* void *memset(void *s, int c, size_t n) */

LEAF(memset)
	.set push
	.set noreorder

	slti	t1, a2, 8		# Less than 8?
	bne	t1, zero, last8
	move	v0, a0			# Setup exit value before too late

	beq	a1, zero, ueven		# If zero pattern, no need to extend
	andi	a1, 0xff		# Avoid problems with bogus arguments
	sll	t0, a1, 8
	or	a1, t0
	sll	t0, a1, 16
	or	a1, t0			# a1 is now pattern in full word

ueven:
	subu	t0, zero, a0		# Unaligned address?
	andi	t0, 0x3
	beq	t0, zero, chkw
	subu	a2, t0
	swr	a1, 0(a0)		# Yes, handle first unaligned part
	addu	a0, t0			# Now both a0 and a2 are updated

chkw:
	andi	t0, a2, 0x7		# Enough left for one loop iteration?
	beq	t0, a2, chkl
	subu	a3, a2, t0
	addu	a3, a0			# a3 is last loop address +1
	move	a2, t0			# a2 is now # of bytes left after loop
loopw:
	addiu	a0, 8			# Handle 2 words pr. iteration
	sw	a1, -8(a0)
	bne	a0, a3, loopw
	sw	a1, -4(a0)

chkl:
	andi	t0, a2, 0x4		# Check if there is at least a full
	beq	t0, zero, last8		#  word remaining after the loop
	subu	a2, t0
	sw	a1, 0(a0)		# Yes...
	addiu	a0, 4

last8:
	blez	a2, exit		# Handle last 8 bytes (if cnt>0)
	addu	a3, a2, a0		# a3 is last address +1
lst8l:
	addiu	a0, 1
	bne	a0, a3, lst8l
	sb	a1, -1(a0)
exit:
	j	ra			# Bye, bye
	nop

	.set reorder
	.set pop
END(memset)
libc_hidden_def(memset)
