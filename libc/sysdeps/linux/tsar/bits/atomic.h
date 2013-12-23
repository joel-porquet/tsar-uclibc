#ifndef _TSAR_BITS_ATOMIC_H
#define _TSAR_BITS_ATOMIC_H

#include <stdint.h>

/*
 * Atomic compare and exchange
 * MIPS32 only supports ll/sc on 32-bit words
 */

#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __tsar_link_error (); oldval; })

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __tsar_link_error (); oldval; })

/* acquire semantics (ie barrier after) */
#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval)          \
	({ __typeof (*mem) prev; int cmp;                                    \
	 __asm__ __volatile__ (                                              \
		 ".set push\n"                                               \
		 "1:"                                                        \
		 "ll	%[prev],%[m]\n"                                      \
		 "move	%[cmp], $0\n"                                        \
		 "bne	%[prev],%[oldv],2f\n"                                \
		 "move	%[cmp],%[newv]\n"                                    \
		 "sc	%[cmp],%[m]\n"                                       \
		 "beqz	%[cmp],1b\n"                                         \
		 "sync	\n"                                                  \
		 "2:\n"                                                      \
		 ".set pop\n"                                                \
		 : [prev] "=&r" (prev), [cmp] "=&r" (cmp), [m] "+m" (*mem)   \
		 : [oldv] "r" (oldval), [newv] "r" (newval)                  \
		 : "memory");                                                \
	 (__typeof (*mem))prev; })


#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __tsar_link_error (); oldval; })

/* make the compilation fail if used */
void __tsar_link_error(void);

#endif /* _TSAR_BITS_ATOMIC_H */
