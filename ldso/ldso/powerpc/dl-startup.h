/* Any assmbly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.
 * Copyright (C) 2005 by Joakim Tjernlund
 */

asm(
    "	.text\n"
    "	.globl	_start\n"
    "	.type	_start,@function\n"
    "_start:\n"
    "	mr	3,1\n" /* Pass SP to _dl_start in r3 */
    "	li	0,0\n"
    "	stwu	1,-16(1)\n" /* Make room on stack for _dl_start to store LR */
    "	stw	0,0(1)\n" /* Clear Stack frame */
    "	bl	_dl_start@local\n" /* Perform relocation */
    /*  Save the address of the apps entry point in CTR register */
    "	mtctr	3\n" /* application entry point */
    "	bl	_GLOBAL_OFFSET_TABLE_-4@local\n" /*  Put our GOT pointer in r31, */
    "	mflr	31\n"
    "	addi	1,1,16\n" /* Restore SP */
#if 0
    /* Try beeing SVR4 ABI compliant?, even though it is not needed for uClibc on Linux */
    /* argc */
    "	lwz	3,0(1)\n"
    /* find argv one word offset from the stack pointer */
    "	addi	4,1,4\n"
    /* find environment pointer (argv+argc+1) */
    "	lwz	5,0(1)\n"
    "	addi	5,5,1\n"
    "	rlwinm	5,5,2,0,29\n"
    "	add	5,5,4\n"
    /* pass the auxilary vector in r6. This is passed to us just after _envp.  */
    "2:	lwzu	0,4(6)\n"
    "	cmpwi	0,0\n"
    "	bne	2b\n"
    "	addi	6,6,4\n"
#endif
    /* Pass a termination function pointer (in this case _dl_fini) in r7.  */
    "	lwz	7,_dl_fini@got(31)\n"
    "	bctr\n" /* Jump to entry point */
    "	.size	_start,.-_start\n"
    "	.previous\n"
);

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*) ARGS)+1)

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB) \
	{int type=ELF32_R_TYPE((RELP)->r_info);		\
	 Elf32_Addr finaladdr=(SYMBOL)+(RELP)->r_addend;\
	if (type==R_PPC_RELATIVE) {			\
		*REL=(Elf32_Word)(LOAD)+(RELP)->r_addend;\
	} else if (type==R_PPC_ADDR32 || type==R_PPC_GLOB_DAT) {\
		*REL=finaladdr;				\
	} else if (type==R_PPC_JMP_SLOT) {		\
		Elf32_Sword delta=finaladdr-(Elf32_Word)(REL);\
		*REL=OPCODE_B(delta);			\
		PPC_DCBST(REL); PPC_SYNC; PPC_ICBI(REL);\
	} else {					\
		_dl_exit(100+ELF32_R_TYPE((RELP)->r_info));\
	}						\
	}
/*
 * Transfer control to the user's application, once the dynamic loader
 * is done.  This routine has to exit the current function, then
 * call the _dl_elf_main function.
 */
#define START()	    return _dl_elf_main
