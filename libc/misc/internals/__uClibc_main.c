/*
 * Manuel Novoa III           Feb 2001
 *
 * __uClibc_main is the routine to be called by all the arch-specific
 * versions of crt0.S in uClibc.
 *
 * It is meant to handle any special initialization needed by the library
 * such as setting the global variable(s) __environ (environ) and
 * initializing the stdio package.  Using weak symbols, the latter is
 * avoided in the static library case.
 */

#define	_ERRNO_H
#include <stdlib.h>
#include <unistd.h>


/*
 * Prototypes.
 */

extern int main(int argc, char **argv, char **envp);

void __uClibc_main(int argc, char **argv, char **envp)
	 __attribute__ ((__noreturn__));



#ifdef HAVE_ELF
weak_alias(__environ, environ);
extern void weak_function _stdio_init(void);
extern void weak_function _stdio_term(void);
extern int *weak_const_function __errno_location (void);
extern int *weak_const_function __h_errno_location (void);
#else
extern void _stdio_init(void);
extern void _stdio_term(void);
extern int *__errno_location (void);
extern int *__h_errno_location (void);
#endif	

/*
 * Declare the __environ global variable and create a weak alias environ.
 * Note: Apparently we must initialize __environ for the weak environ
 * symbol to be included.
 */

char **__environ = 0;


/*
 * Now for our main routine.
 */

void __uClibc_main(int argc, char **argv, char **envp) 
{
	/* 
	 * Initialize the global variable __environ.
	 */
	__environ = envp;

#if 0
	/* Some security at this point.  Prevent starting a SUID binary
	 * where the standard file descriptors are not opened.  We have
	 * to do this only for statically linked applications since
	 * otherwise the dynamic loader did the work already.  */
	if (__builtin_expect (__libc_enable_secure, 0))
	    __libc_check_standard_fds ();
#endif
	/*
	 * Initialize stdio here.  In the static library case, this will
	 * be bypassed if not needed because of the weak alias above.
	 */
	if (_stdio_init)
	  _stdio_init();

	/*
	 * Note: It is possible that any initialization done above could
	 * have resulted in errno being set nonzero, so set it to 0 before
	 * we call main.
	 */
	if (__errno_location)
	    *(__errno_location()) = 0;
	/* Set h_errno to 0 as well */
	if (__h_errno_location)
	    *(__h_errno_location()) = 0;

	/*
	 * Finally, invoke application's main and then exit.
	 */
	exit(main(argc, argv, envp));
}

#ifndef HAVE_ELF
/*
 * Define an empty function and use it as a weak alias for the stdio
 * initialization routine.  That way we don't pull in all the stdio
 * code unless we need to.  Similarly, do the same for _stdio_term
 * so as not to include atexit unnecessarily.
 *
 * NOTE!!! This is only true for the _static_ case!!!
 */

weak_alias(__environ, environ);
#if 0
void __uClibc_empty_func(void)
{
}
weak_alias(__uClibc_empty_func, _stdio_init);
weak_alias(__uClibc_empty_func, _stdio_term);
#endif
#endif	
