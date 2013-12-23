#ifndef _TSAR_BITS_SETJMP_H
#define _TSAR_BITS_SETJMP_H

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

typedef struct {
	void * __pc;
	void * __sp;

	int __regs[8];

	void *__fp;
	void *__gp;
} __jmp_buf[1];

#endif /* _TSAR_BITS_SETJMP_H */
