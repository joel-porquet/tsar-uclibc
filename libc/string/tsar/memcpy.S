#include <sysdep.h>

/* void *memcpy(void *s1, const void *s2, size_t n) */

LEAF(memcpy)
	.set push
	.set noreorder

	slti	t0, a2, 8		# Less than 8?
	bne	t0, zero, last8
	move	v0, a0			# Setup exit value before too late

	xor	t0, a1, a0		# Find a0/a1 displacement
	andi	t0, 0x3
	bne	t0, zero, shift		# Go handle the unaligned case
	subu	t1, zero, a1
	andi	t1, 0x3			# a0/a1 are aligned, but are we
	beq	t1, zero, chk8w		#  starting in the middle of a word?
	subu	a2, t1
	lwr	t0, 0(a1)		# Yes we are... take care of that
	addu	a1, t1
	swr	t0, 0(a0)
	addu	a0, t1

chk8w:
	andi	t0, a2, 0x1f		# 32 or more bytes left?
	beq	t0, a2, chk1w
	subu	a3, a2, t0		# Yes
	addu	a3, a1			# a3 = end address of loop
	move	a2, t0			# a2 = what will be left after loop
lop8w:
	lw	t0,  0(a1)		# Loop taking 8 words at a time
	lw	t1,  4(a1)
	lw	t2,  8(a1)
	lw	t3, 12(a1)
	lw	t4, 16(a1)
	lw	t5, 20(a1)
	lw	t6, 24(a1)
	lw	t7, 28(a1)
	addiu	a0, 32
	addiu	a1, 32
	sw	t0, -32(a0)
	sw	t1, -28(a0)
	sw	t2, -24(a0)
	sw	t3, -20(a0)
	sw	t4, -16(a0)
	sw	t5, -12(a0)
	sw	t6,  -8(a0)
	bne	a1, a3, lop8w
	sw	t7,  -4(a0)

chk1w:
	andi	t0, a2, 0x3		# 4 or more bytes left?
	beq	t0, a2, last8
	subu	a3, a2, t0		# Yes, handle them one word at a time
	addu	a3, a1			# a3 again end address
	move	a2, t0
lop1w:
	lw	t0, 0(a1)
	addiu	a0, 4
	addiu	a1, 4
	bne	a1, a3, lop1w
	sw	t0, -4(a0)

last8:
	blez	a2, lst8e		# Handle last 8 bytes, one at a time
	addu	a3, a2, a1
lst8l:
	lb	t0, 0(a1)
	addiu	a0, 1
	addiu	a1, 1
	bne	a1, a3, lst8l
	sb	t0, -1(a0)
lst8e:
	jr	ra			# Bye, bye
	nop

shift:
	subu	a3, zero, a0		# Src and Dest unaligned
	andi	a3, 0x3			#  (unoptimized case...)
	beq	a3, zero, shft1
	subu	a2, a3			# a2 = bytes left
	lwr	t0, 0(a1)		# Take care of first odd part
	lwl	t0, 3(a1)
	addu	a1, a3
	swr	t0, 0(a0)
	addu	a0, a3
shft1:
	andi	t0, a2, 0x3
	subu	a3, a2, t0
	addu	a3, a1
shfth:
	lwr	t1, 0(a1)		# Limp through, word by word
	lwl	t1, 3(a1)
	addiu	a0, 4
	addiu	a1, 4
	bne	a1, a3, shfth
	sw	t1, -4(a0)
	b	last8			# Handle anything which may be left
	move	a2, t0

	.set reorder
	.set pop
END(memcpy)
libc_hidden_def(memcpy)
