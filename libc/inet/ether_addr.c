/*
*  Copyright (C) 1996, 1997, 1999 Free Software Foundation, Inc.
*   This file was assembled from parts of the GNU C Library.
*   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*/

/*
*  2002-12-24  Nick Fedchik <nick@fedchik.org.ua>
* 	- initial uClibc port
*/


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>

#define __FORCE_GLIBC
struct ether_addr *ether_aton(const char *asc)
{
	static struct ether_addr result;

	return ether_aton_r(asc, &result);
}

struct ether_addr *ether_aton_r(const char *asc, struct ether_addr *addr)
{
	size_t cnt;

	for (cnt = 0; cnt < 6; ++cnt) {
		unsigned int number;
		char ch;

		ch = _tolower(*asc++);
		if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
			return NULL;
		number = isdigit(ch) ? (ch - '0') : (ch - 'a' + 10);

		ch = _tolower(*asc);
		if ((cnt < 5 && ch != ':')
			|| (cnt == 5 && ch != '\0' && !isspace(ch))) {
			++asc;
			if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
				return NULL;
			number <<= 4;
			number += isdigit(ch) ? (ch - '0') : (ch - 'a' + 10);

			ch = *asc;
			if (cnt < 5 && ch != ':')
				return NULL;
		}

		/* Store result.  */
		addr->ether_addr_octet[cnt] = (unsigned char) number;

		/* Skip ':'.  */
		++asc;
	}

	return addr;
}

char *ether_ntoa(const struct ether_addr *addr)
{
	static char asc[18];

	return ether_ntoa_r(addr, asc);
}

char *ether_ntoa_r(const struct ether_addr *addr, char *buf)
{
	sprintf(buf, "%x:%x:%x:%x:%x:%x",
			addr->ether_addr_octet[0], addr->ether_addr_octet[1],
			addr->ether_addr_octet[2], addr->ether_addr_octet[3],
			addr->ether_addr_octet[4], addr->ether_addr_octet[5]);
	return buf;
}
