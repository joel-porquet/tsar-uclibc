/*
 * Track misc arch-specific features that aren't config options
 */

#ifndef _BITS_UCLIBC_ARCH_FEATURES_H
#define _BITS_UCLIBC_ARCH_FEATURES_H

/* instruction used when calling abort() to kill yourself */
#define __UCLIBC_ABORT_INSTRUCTION__ "iitlbp %r0,(%sr0,%r0)"

/* can your target use syscall6() for mmap ? */
#define __UCLIBC_MMAP_HAS_6_ARGS__

/* does your target use syscall4() for truncate64 ? (32bit arches only) */
#undef __UCLIBC_TRUNCATE64_HAS_4_ARGS__

/* does your target have a broken create_module() ? */
#undef __UCLIBC_BROKEN_CREATE_MODULE__

/* does your target have to worry about older [gs]etrlimit() ? */
#undef __UCLIBC_HANDLE_OLDER_RLIMIT__

/* does your target have an asm .set ? */
#define __UCLIBC_HAVE_ASM_SET_DIRECTIVE__

/* define if target doesn't like .global */
#undef __UCLIBC_ASM_GLOBAL_DIRECTIVE__

/* define if target supports .weak */
#define __UCLIBC_HAVE_ASM_WEAK_DIRECTIVE__

/* define if target supports .weakext */
#undef __UCLIBC_HAVE_ASM_WEAKEXT_DIRECTIVE__

/* needed probably only for ppc64 */
#undef __UCLIBC_HAVE_ASM_GLOBAL_DOT_NAME__

/* define if target supports CFI pseudo ops */
#undef __UCLIBC_HAVE_ASM_CFI_DIRECTIVES__

/* define if target supports IEEE signed zero floats */
#define __UCLIBC_HAVE_SIGNED_ZERO__

/* the default ; is a comment on hppa */
#define __UCLIBC_ASM_LINE_SEP__ !

#endif /* _BITS_UCLIBC_ARCH_FEATURES_H */
