#ifndef _TSAR_LINUX_SYSDEP_H
#define _TSAR_LINUX_SYSDEP_H

#ifdef __ASSEMBLER__

#include <sys/asm.h>
#include <sys/regdef.h>

#else /* ! __ASSEMBLER__ */

/* Pointer mangling is not yet supported for MIPS.  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#endif /* __ASSEMBLER__ */

#include <common/sysdep.h>

#endif /* _TSAR_LINUX_SYSDEP_H */
