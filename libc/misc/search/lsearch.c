/*
 * This file lifted in toto from 'Dlibs' on the atari ST  (RdeBath)
 *
 * 
 *    Dale Schumacher                         399 Beacon Ave.
 *    (alias: Dalnefre')                      St. Paul, MN  55104
 *    dal@syntel.UUCP                         United States of America
 *  "It's not reality that's important, but how you perceive things."
 */

#include <string.h>
#include <stdio.h>
#include <search.h>

#ifdef L_lfind

void attribute_hidden *__lfind(const void *key, const void *base, size_t *nmemb,
	size_t size, int (*compar)(const void *, const void *))
{
	register int n = *nmemb;

	while (n--) {
		if ((*compar) (base, key) == 0)
			return ((void*)base);
		base += size;
	}
	return (NULL);
}
strong_alias(__lfind,lfind)

#endif

#ifdef L_lsearch

extern void *__lfind (__const void *__key, __const void *__base,
		    size_t *__nmemb, size_t __size, __compar_fn_t __compar) attribute_hidden;

void *lsearch(const void *key, void *base, size_t *nmemb, 
	size_t size, int (*compar)(const void *, const void *))
{
	register char *p;

	if ((p = __lfind(key, base, nmemb, size, compar)) == NULL) {
		p = __memcpy((base + (size * (*nmemb))), key, size);
		++(*nmemb);
	}
	return (p);
}

#endif
