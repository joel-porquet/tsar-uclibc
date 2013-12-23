#ifndef _TSAR_BITS_SYSCALLS_H
#define _TSAR_BITS_SYSCALLS_H

#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/* according to "bits/syscalls-common.h", what we really need to define here is
 * the "INTERNAL_SYSCALL_NCS" macro */

#ifndef __ASSEMBLER__

/* Attempt to make a unique syscall macro
 * - syscall number is passed to kernel via $2
 * - result is received from kernel via $2
 * - the 4 first arguments are passed via registers only ($4 to $7)
 * - the next two arguments are passed via the stack
 */
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...) \
	(__extension__                               \
	 ({long __sys_result;                        \
	  {                                          \
	  PREP_ARGS_##nr (args)                      \
	  register long __v0 __asm__ ("$2");         \
	  LOAD_ARGS_##nr                             \
	  __v0 = (name);                             \
	  __asm__ volatile (                         \
		  ".set push\n"                      \
		  ".set noreorder\n"                 \
		  LOAD_ARGS_STACK_##nr               \
		  "syscall\n"                        \
		  UNLOAD_ARGS_STACK_##nr             \
		  ".set pop\n"                       \
		  : "=r" (__v0)                      \
		  : ASM_ARGS_##nr                    \
		  : "memory");                       \
	  __sys_result = __v0;                       \
	  }                                          \
	  __sys_result; })                           \
	)

#define PREP_ARGS_0()
#define PREP_ARGS_1(a0)                     \
  long _t0 = (long) (a0);
#define PREP_ARGS_2(a0, a1)                 \
  long _t1 = (long) (a1);                   \
  PREP_ARGS_1(a0)
#define PREP_ARGS_3(a0, a1, a2)             \
  long _t2 = (long) (a2);                   \
  PREP_ARGS_2(a0, a1)
#define PREP_ARGS_4(a0, a1, a2, a3)         \
  long _t3 = (long) (a3);                   \
  PREP_ARGS_3(a0, a1, a2)
#define PREP_ARGS_5(a0, a1, a2, a3, a4)     \
  long _t4 = (long) (a4);                   \
  PREP_ARGS_4(a0, a1, a2, a3)
#define PREP_ARGS_6(a0, a1, a2, a3, a4, a5) \
  long _t5 = (long) (a5);                   \
  PREP_ARGS_5(a0, a1, a2, a3, a4)

#define LOAD_ARGS_0
#define LOAD_ARGS_1                                \
  register long _a0 __asm__ ("$4") = (long) (_t0);
#define LOAD_ARGS_2                                \
  register long _a1 __asm__ ("$5") = (long) (_t1); \
  LOAD_ARGS_1
#define LOAD_ARGS_3                                \
  register long _a2 __asm__ ("$6") = (long) (_t2); \
  LOAD_ARGS_2
#define LOAD_ARGS_4                                \
  register long _a3 __asm__ ("$7") = (long) (_t3); \
  LOAD_ARGS_3
#define LOAD_ARGS_5 LOAD_ARGS_4
#define LOAD_ARGS_6 LOAD_ARGS_5

#define ASM_ARGS_0
#define ASM_ARGS_1 "r" (_a0)
#define ASM_ARGS_2 ASM_ARGS_1, "r" (_a1)
#define ASM_ARGS_3 ASM_ARGS_2, "r" (_a2)
#define ASM_ARGS_4 ASM_ARGS_3, "r" (_a3)
#define ASM_ARGS_5 ASM_ARGS_4, [_t4] "r" (_t4)
#define ASM_ARGS_6 ASM_ARGS_5, [_t5] "r" (_t5)

#define LOAD_ARGS_STACK_0
#define LOAD_ARGS_STACK_1
#define LOAD_ARGS_STACK_2
#define LOAD_ARGS_STACK_3
#define LOAD_ARGS_STACK_4
#define LOAD_ARGS_STACK_5   \
	"subu	$29, 8*4\n" \
	"sw	%[_t4], 16($29)\n"
#define LOAD_ARGS_STACK_6   \
	LOAD_ARGS_STACK_5   \
	"sw	%[_t5], 20($29)\n"

#define UNLOAD_ARGS_STACK_0
#define UNLOAD_ARGS_STACK_1
#define UNLOAD_ARGS_STACK_2
#define UNLOAD_ARGS_STACK_3
#define UNLOAD_ARGS_STACK_4
#define UNLOAD_ARGS_STACK_5 "addiu	$29, 8*4\n"
#define UNLOAD_ARGS_STACK_6 UNLOAD_ARGS_STACK_5

#endif /* __ASSEMBLER__ */
#endif /* _TSAR_BITS_SYSCALLS_H */
