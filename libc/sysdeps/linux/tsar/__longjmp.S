#include <sysdep.h>
#include <sys/asm.h>
#include <sys/regdef.h>

ENTRY(__longjmp)
#ifdef __PIC__
	SETUP_GP
#endif
	/* if val == 0, return 1, otherwise return val */
	move	v0, a1
	bnez	v0, 1f
	li	v0, 1
1:
	/* restore pc */
	lw	t9, 0(a0)
	/* restore stack pointer */
	lw	sp, 4(a0)
	/* restore callee saved registers */
	lw	s0, 8(a0)
	lw	s1, 12(a0)
	lw	s2, 16(a0)
	lw	s3, 20(a0)
	lw	s4, 24(a0)
	lw	s5, 28(a0)
	lw	s6, 32(a0)
	lw	s7, 36(a0)
	/* restore frame pointer */
	lw	fp, 40(a0)
	/* restore global pointer */
	lw	gp, 44(a0)
	jr	t9
END(__longjmp)
libc_hidden_def(__longjmp)
