#ifndef _TSAR_LINUX_SYSDEP_H
#define _TSAR_LINUX_SYSDEP_H

#include <common/sysdep.h>

#ifdef __ASSEMBLER__

#include <sys/regdef.h>

#define ENTRY(name)  \
	.globl name; \
	.align 2;    \
	.ent name,0; \
	name##:

#define	ENDPROC(name) \
	.end	name; \
	.size	name,.-name

#endif /* __ASSEMBLER__ */

#endif /* _TSAR_LINUX_SYSDEP_H */
