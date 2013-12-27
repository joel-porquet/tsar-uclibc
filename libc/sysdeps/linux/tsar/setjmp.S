#include <sysdep.h>
#include <sys/asm.h>
#include <sys/regdef.h>

/*
 * setjmp and variants:
 *
 * setjmp/longjmp: save/restore signal mask
 * _setjmp/_longjmp: save/restore only register set and stack
 * sigsetjmp/siglongjmp: save/restore signal mask if arg1 is != 0
 */

#ifdef __PIC__
	.option	pic2
#endif

.text

/* int setjmp(jmp_buf env);
 *
 * call sigsetjmp(env, 1)
 */
ENTRY(setjmp)
#ifdef __PIC__
	SETUP_GP
	la	t9, __sigsetjmp
#endif
	li	a1, 1
#ifdef __PIC__
	jr	t9
#else
	j	__sigsetjmp
#endif
END(setjmp)

/* int _setjmp(jmp_buf env);
 *
 * call sigsetjmp(env, 0)
 */
ENTRY(_setjmp)
#ifdef __PIC__
	SETUP_GP
	la	t9, __sigsetjmp
#endif
	move	a1, zero
#ifdef __PIC__
	jr	t9
#else
	j	__sigsetjmp
#endif
END(_setjmp)

/* int __sigsetjmp(jmp_buf env, int savemask);
 */
ENTRY(__sigsetjmp)
#ifdef __PIC__
	SETUP_GP
#endif
	/* save pc */
	sw	ra, 0(a0)
	/* save stack pointer */
	sw	sp, 4(a0)
	/* save callee saved registers */
	sw	s0, 8(a0)
	sw	s1, 12(a0)
	sw	s2, 16(a0)
	sw	s3, 20(a0)
	sw	s4, 24(a0)
	sw	s5, 28(a0)
	sw	s6, 32(a0)
	sw	s7, 36(a0)
	/* save frame pointer */
	sw	fp, 40(a0)
	/* save global pointer */
	sw	gp, 44(a0)
#ifdef __PIC__
	la	t9, __sigsetjmp_save
	jr	t9
#else
	j	__sigsetjmp_save
#endif
END(__sigsetjmp)
