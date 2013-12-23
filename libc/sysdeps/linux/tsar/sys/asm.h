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

#endif /* _SYS_TSAR_ASM_H */
