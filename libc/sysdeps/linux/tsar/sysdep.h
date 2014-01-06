#ifndef _TSAR_LINUX_SYSDEP_H
#define _TSAR_LINUX_SYSDEP_H

#ifdef __ASSEMBLER__

#include <sys/regdef.h>

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

#else /* ! __ASSEMBLER__ */

/* Pointer mangling is not yet supported for MIPS.  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#endif /* __ASSEMBLER__ */

#include <common/sysdep.h>

#endif /* _TSAR_LINUX_SYSDEP_H */
