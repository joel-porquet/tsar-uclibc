#ifndef _SYS_TSAR_ASM_H
#define _SYS_TSAR_ASM_H

/*
 * PIC stuff
 */

#define SETUP_GP        \
	.set noreorder ;\
	.cpload	$25    ;\
	.set reorder   ;

#define SETUP_GPX(r)    \
	.set noreorder ;\
	move r, $31    ;\
	bal 10f        ;\
	nop            ;\
10:                     \
	.cpload $31    ;\
	move $31, r    ;\
	.set reorder   ;

#define SAVE_GP(x) \
	.cprestore x

#define _ENTRY(name)             \
	.globl	name;            \
	.align	2;               \
	.type	name, @function; \
	.ent	name, 0

/* no frame */
#define ENTRY(name)   \
	_ENTRY(name); \
	name:

/* leaf routine */
#define LEAF(name)    \
	_ENTRY(name); \
	name:	.frame sp, 0, ra

/* nested routine */
#define NESTED(name, framesz, rpc) \
	_ENTRY(name);              \
	name:	.frame sp, framesz, rpc

#define	END(name)     \
	.end	name; \
	.size	name,.-name

#define ret         \
	jr	ra; \
	nop

#endif /* _SYS_TSAR_ASM_H */
