
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 */

/* __ieee754_gamma(x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call __ieee754_gamma_r
 */

#include <math.h>
#include "math_private.h"

libm_hidden_proto(signgam)
/* __private_extern__ */
double attribute_hidden __ieee754_gamma(double x)
{
	return __ieee754_gamma_r(x,&signgam);
}
