/* resolv.c: DNS Resolver
 *
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *                     The Silver Hammer Group, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

/*
 * Portions Copyright (c) 1985, 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *
 *  5-Oct-2000 W. Greathouse  wgreathouse@smva.com
 *                              Fix memory leak and memory corruption.
 *                              -- Every name resolution resulted in
 *                                 a new parse of resolv.conf and new
 *                                 copy of nameservers allocated by
 *                                 strdup.
 *                              -- Every name resolution resulted in
 *                                 a new read of resolv.conf without
 *                                 resetting index from prior read...
 *                                 resulting in exceeding array bounds.
 *
 *                              Limit nameservers read from resolv.conf
 *
 *                              Add "search" domains from resolv.conf
 *
 *                              Some systems will return a security
 *                              signature along with query answer for
 *                              dynamic DNS entries.
 *                              -- skip/ignore this answer
 *
 *                              Include arpa/nameser.h for defines.
 *
 *                              General cleanup
 *
 * 20-Jun-2001 Michal Moskal <malekith@pld.org.pl>
 *   partial IPv6 support (i.e. gethostbyname2() and resolve_address2()
 *   functions added), IPv6 nameservers are also supported.
 *
 * 6-Oct-2001 Jari Korva <jari.korva@iki.fi>
 *   more IPv6 support (IPv6 support for gethostbyaddr();
 *   address family parameter and improved IPv6 support for get_hosts_byname
 *   and read_etc_hosts; getnameinfo() port from glibc; defined
 *   defined ip6addr_any and in6addr_loopback)
 *
 * 2-Feb-2002 Erik Andersen <andersen@codepoet.org>
 *   Added gethostent(), sethostent(), and endhostent()
 *
 * 17-Aug-2002 Manuel Novoa III <mjn3@codepoet.org>
 *   Fixed __read_etc_hosts_r to return alias list, and modified buffer
 *   allocation accordingly.  See MAX_ALIASES and ALIAS_DIM below.
 *   This fixes the segfault in the Python 2.2.1 socket test.
 *
 * 04-Jan-2003 Jay Kulpinski <jskulpin@berkshire.rr.com>
 *   Fixed __decode_dotted to count the terminating null character
 *   in a host name.
 *
 * 02-Oct-2003 Tony J. White <tjw@tjw.org>
 *   Lifted dn_expand() and dependent ns_name_uncompress(), ns_name_unpack(),
 *   and ns_name_ntop() from glibc 2.3.2 for compatibility with ipsec-tools
 *   and openldap.
 *
 * 7-Sep-2004 Erik Andersen <andersen@codepoet.org>
 *   Added gethostent_r()
 *
 */

#define __FORCE_GLIBC
#include <features.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <resolv.h>
#include <netdb.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <arpa/nameser.h>
#include <sys/utsname.h>
#include <sys/un.h>
#include <bits/uClibc_mutex.h>

/* poll() is not supported in kernel <= 2.0, therefore if __NR_poll is
 * not available, we assume an old Linux kernel is in use and we will
 * use select() instead. */
#include <sys/syscall.h>
#ifndef __NR_poll
# define USE_SELECT
#endif

/* Experimentally off - libc_hidden_proto(memcpy) */
/* Experimentally off - libc_hidden_proto(memset) */
/* Experimentally off - libc_hidden_proto(memmove) */
/* Experimentally off - libc_hidden_proto(strchr) */
/* Experimentally off - libc_hidden_proto(strcmp) */
/* Experimentally off - libc_hidden_proto(strcpy) */
/* Experimentally off - libc_hidden_proto(strdup) */
/* Experimentally off - libc_hidden_proto(strlen) */
/* Experimentally off - libc_hidden_proto(strncat) */
/* Experimentally off - libc_hidden_proto(strncpy) */
/* libc_hidden_proto(strnlen) */
/* Experimentally off - libc_hidden_proto(strstr) */
/* Experimentally off - libc_hidden_proto(strcasecmp) */
/* libc_hidden_proto(socket) */
/* libc_hidden_proto(close) */
/* libc_hidden_proto(fopen) */
/* libc_hidden_proto(fclose) */
/* libc_hidden_proto(random) */
/* libc_hidden_proto(getservbyport) */
/* libc_hidden_proto(gethostname) */
/* libc_hidden_proto(uname) */
/* libc_hidden_proto(inet_addr) */
/* libc_hidden_proto(inet_aton) */
/* libc_hidden_proto(inet_pton) */
/* libc_hidden_proto(inet_ntop) */
/* libc_hidden_proto(connect) */
/* libc_hidden_proto(poll) */
/* libc_hidden_proto(select) */
/* libc_hidden_proto(recv) */
/* libc_hidden_proto(send) */
/* libc_hidden_proto(printf) */
/* libc_hidden_proto(sprintf) */
/* libc_hidden_proto(snprintf) */
/* libc_hidden_proto(fgets) */
/* libc_hidden_proto(getnameinfo) */
/* libc_hidden_proto(gethostbyname) */
/* libc_hidden_proto(gethostbyname_r) */
/* libc_hidden_proto(gethostbyname2_r) */
/* libc_hidden_proto(gethostbyaddr) */
/* libc_hidden_proto(gethostbyaddr_r) */
/* libc_hidden_proto(ns_name_uncompress) */
/* libc_hidden_proto(ns_name_unpack) */
/* libc_hidden_proto(ns_name_ntop) */
/* libc_hidden_proto(res_init) */
/* libc_hidden_proto(res_query) */
/* libc_hidden_proto(res_querydomain) */
/* libc_hidden_proto(gethostent_r) */
/* libc_hidden_proto(fprintf) */
/* libc_hidden_proto(__h_errno_location) */
#ifdef __UCLIBC_HAS_XLOCALE__
/* libc_hidden_proto(__ctype_b_loc) */
#elif defined __UCLIBC_HAS_CTYPE_TABLES__
/* libc_hidden_proto(__ctype_b) */
#endif

#if defined __UCLIBC_HAS_IPV4__ && defined __UCLIBC_HAS_IPV6__
#define IF_HAS_BOTH(...) __VA_ARGS__
#else
#define IF_HAS_BOTH(...)
#endif

#define MAX_RECURSE    5
#define MAX_ALIASES    5

/* 1:ip + 1:full + MAX_ALIASES:aliases + 1:NULL */
#define ALIAS_DIM      (2 + MAX_ALIASES + 1)

#undef DEBUG
/* #define DEBUG */

#ifdef DEBUG
#define DPRINTF(X,args...) fprintf(stderr, X, ##args)
#else
#define DPRINTF(X,args...)
#endif

#undef ARRAY_SIZE
#define ARRAY_SIZE(v) (sizeof(v) / sizeof((v)[0]))

/* Make sure the incoming char * buffer is aligned enough to handle our random
 * structures.  This define is the same as we use for malloc alignment (which
 * has same requirements).  The offset is the number of bytes we need to adjust
 * in order to attain desired alignment.
 */
#define ALIGN_ATTR __alignof__(double __attribute_aligned__ (sizeof(size_t)))
#define ALIGN_BUFFER_OFFSET(buf) ((ALIGN_ATTR - ((size_t)buf % ALIGN_ATTR)) % ALIGN_ATTR)


/* Structs */
struct resolv_header {
	int id;
	int qr,opcode,aa,tc,rd,ra,rcode;
	int qdcount;
	int ancount;
	int nscount;
	int arcount;
};

struct resolv_question {
	char * dotted;
	int qtype;
	int qclass;
};

struct resolv_answer {
	char * dotted;
	int atype;
	int aclass;
	int ttl;
	int rdlength;
	const unsigned char * rdata;
	int rdoffset;
	char* buf;
	size_t buflen;
	size_t add_count;
};

enum etc_hosts_action {
	GET_HOSTS_BYNAME = 0,
	GETHOSTENT,
	GET_HOSTS_BYADDR,
};

typedef union sockaddr46_t {
	struct sockaddr sa;
#ifdef __UCLIBC_HAS_IPV4__
	struct sockaddr_in sa4;
#endif
#ifdef __UCLIBC_HAS_IPV6__
	struct sockaddr_in6 sa6;
#endif
} sockaddr46_t;


__UCLIBC_MUTEX_EXTERN(__resolv_lock);

/* Protected by __resolv_lock */
extern void (*__res_sync)(void) attribute_hidden;
//extern uint32_t __resolv_opts attribute_hidden;
extern unsigned __nameservers attribute_hidden;
extern unsigned __searchdomains attribute_hidden;
extern sockaddr46_t *__nameserver attribute_hidden;
extern char **__searchdomain attribute_hidden;
#ifdef __UCLIBC_HAS_IPV4__
extern const struct sockaddr_in __local_nameserver attribute_hidden;
#else
extern const struct sockaddr_in6 __local_nameserver attribute_hidden;
#endif
/* Arbitrary */
#define MAXLEN_searchdomain 128


/* function prototypes */
extern int __get_hosts_byname_r(const char * name, int type,
			struct hostent * result_buf,
			char * buf, size_t buflen,
			struct hostent ** result,
			int * h_errnop) attribute_hidden;
extern int __get_hosts_byaddr_r(const char * addr, int len, int type,
			struct hostent * result_buf,
			char * buf, size_t buflen,
			struct hostent ** result,
			int * h_errnop) attribute_hidden;
extern FILE * __open_etc_hosts(void) attribute_hidden;
extern int __read_etc_hosts_r(FILE *fp, const char * name, int type,
			enum etc_hosts_action action,
			struct hostent * result_buf,
			char * buf, size_t buflen,
			struct hostent ** result,
			int * h_errnop) attribute_hidden;
extern int __dns_lookup(const char * name, int type,
			unsigned char ** outpacket,
			struct resolv_answer * a) attribute_hidden;

extern int __encode_dotted(const char * dotted, unsigned char * dest, int maxlen) attribute_hidden;
extern int __decode_dotted(const unsigned char * const message, int offset,
			char * dest, int maxlen) attribute_hidden;
extern int __length_dotted(const unsigned char * const message, int offset) attribute_hidden;
extern int __encode_header(struct resolv_header * h, unsigned char * dest, int maxlen) attribute_hidden;
extern void __decode_header(unsigned char * data, struct resolv_header * h) attribute_hidden;
extern int __encode_question(const struct resolv_question * const q,
			unsigned char * dest, int maxlen) attribute_hidden;
extern int __decode_question(const unsigned char * const message, int offset,
			struct resolv_question * q) attribute_hidden;
extern int __encode_answer(struct resolv_answer * a,
			unsigned char * dest, int maxlen) attribute_hidden;
extern int __decode_answer(const unsigned char * message, int offset,
			int len, struct resolv_answer * a) attribute_hidden;
extern int __length_question(const unsigned char * const message, int offset) attribute_hidden;
extern void __open_nameservers(void) attribute_hidden;
extern void __close_nameservers(void) attribute_hidden;
extern int __dn_expand(const u_char *, const u_char *, const u_char *,
			char *, int);

/*
 * Theory of operation.
 *
 * gethostbyname, getaddrinfo and friends end up here, and they sometimes
 * need to talk to DNS servers. In order to do this, we need to read /etc/resolv.conf
 * and determine servers' addresses and the like. resolv.conf format:
 *
 * nameserver <IP[v6]>
 *		Address of DNS server. Cumulative.
 *		If not specified, assumed to be on localhost.
 * search <domain1>[ <domain2>]...
 *		Append these domains to unqualified names.
 *		See ndots:n option.
 *		$LOCALDOMAIN (space-separated list) overrides this.
 * domain <domain>
 *		Effectively same as "search" with one domain.
 *		If no "domain" line is present, the domain is determined
 *		from the local host name returned by gethostname();
 *		the domain part is taken to be everything after the first dot.
 *		If there are no dots, there will be no "domain".
 *		The domain and search keywords are mutually exclusive.
 *		If more than one instance of these keywords is present,
 *		the last instance wins.
 * sortlist 130.155.160.0[/255.255.240.0] 130.155.0.0
 *		Allows addresses returned by gethostbyname to be sorted.
 *		Not supported.
 * options option[ option]...
 *		(so far we support none)
 *		$RES_OPTIONS (space-separated list) is to be added to "options"
 *  debug	sets RES_DEBUG in _res.options
 *  ndots:n	how many dots there should be so that name will be tried
 *		first as an absolute name before any search list elements
 *		are appended to it. Default 1
 *  timeout:n   how long to wait for response. Default 5
 *		(sun seems to have retrans:n synonym)
 *  attempts:n	number of rounds to do before giving up and returning
 *		an error. Default 2
 *		(sun seems to have retry:n synonym)
 *  rotate	sets RES_ROTATE in _res.options, round robin
 *		selection of nameservers. Otherwise try
 *		the first listed server first every time
 *  no-check-names
 *		sets RES_NOCHECKNAME in _res.options, which disables
 *		checking of incoming host names for invalid characters
 *		such as underscore (_), non-ASCII, or control characters
 *  inet6	sets RES_USE_INET6 in _res.options. Try a AAAA query
 *		before an A query inside the gethostbyname(), and map
 *		IPv4 responses in IPv6 "tunnelled form" if no AAAA records
 *		are found but an A record set exists
 *  no_tld_query (FreeBSDism?)
 *		do not attempt to resolve names without dots
 *
 * We will read and analyze /etc/resolv.conf as needed before
 * we do a DNS request. This happens in __dns_lookup.
 * (TODO: also re-parse it after a timeout, to catch updates).
 *
 * BSD has res_init routine which is used to initialize resolver state
 * which is held in global structure _res.
 * Generally, programs call res_init, then fiddle with _res.XXX
 * (_res.options and _res.nscount, _res.nsaddr_list[N]
 * are popular targets of fiddling) and expect subsequent calls
 * to gethostbyname, getaddrinfo, etc to use modified information.
 *
 * However, historical _res structure is quite awkward.
 * Using it for storing /etc/resolv.conf info is not desirable,
 * and __dns_lookup does not use it.
 *
 * We would like to avoid using it unless absolutely necessary.
 * If user doesn't use res_init, we should arrange it so that
 * _res structure doesn't even *get linked in* into user's application
 * (imagine static uclibc build here).
 *
 * The solution is a __res_sync function pointer, which is normally NULL.
 * But if res_init is called, it gets set and any subsequent gethostbyname
 * et al "syncronizes" our internal structures with potentially
 * modified _res.XXX stuff by calling __res_sync.
 * The trick here is that if res_init is not used and not linked in,
 * gethostbyname itself won't reference _res and _res won't be linked in
 * either. Other possible methods like
 * if (__res_sync_just_an_int_flag)
 *	__sync_me_with_res()
 * would pull in __sync_me_with_res, which pulls in _res. Bad.
 */


#ifdef L_encodeh

int attribute_hidden __encode_header(struct resolv_header *h, unsigned char *dest, int maxlen)
{
	if (maxlen < HFIXEDSZ)
		return -1;

	dest[0] = (h->id & 0xff00) >> 8;
	dest[1] = (h->id & 0x00ff) >> 0;
	dest[2] = (h->qr ? 0x80 : 0) |
		((h->opcode & 0x0f) << 3) |
		(h->aa ? 0x04 : 0) |
		(h->tc ? 0x02 : 0) |
		(h->rd ? 0x01 : 0);
	dest[3] = (h->ra ? 0x80 : 0) | (h->rcode & 0x0f);
	dest[4] = (h->qdcount & 0xff00) >> 8;
	dest[5] = (h->qdcount & 0x00ff) >> 0;
	dest[6] = (h->ancount & 0xff00) >> 8;
	dest[7] = (h->ancount & 0x00ff) >> 0;
	dest[8] = (h->nscount & 0xff00) >> 8;
	dest[9] = (h->nscount & 0x00ff) >> 0;
	dest[10] = (h->arcount & 0xff00) >> 8;
	dest[11] = (h->arcount & 0x00ff) >> 0;

	return HFIXEDSZ;
}
#endif


#ifdef L_decodeh

void attribute_hidden __decode_header(unsigned char *data, struct resolv_header *h)
{
	h->id = (data[0] << 8) | data[1];
	h->qr = (data[2] & 0x80) ? 1 : 0;
	h->opcode = (data[2] >> 3) & 0x0f;
	h->aa = (data[2] & 0x04) ? 1 : 0;
	h->tc = (data[2] & 0x02) ? 1 : 0;
	h->rd = (data[2] & 0x01) ? 1 : 0;
	h->ra = (data[3] & 0x80) ? 1 : 0;
	h->rcode = data[3] & 0x0f;
	h->qdcount = (data[4] << 8) | data[5];
	h->ancount = (data[6] << 8) | data[7];
	h->nscount = (data[8] << 8) | data[9];
	h->arcount = (data[10] << 8) | data[11];
}
#endif


#ifdef L_encoded

/* Encode a dotted string into nameserver transport-level encoding.
   This routine is fairly dumb, and doesn't attempt to compress
   the data */
int attribute_hidden __encode_dotted(const char *dotted, unsigned char *dest, int maxlen)
{
	unsigned used = 0;

	while (dotted && *dotted) {
		char *c = strchr(dotted, '.');
		int l = c ? c - dotted : strlen(dotted);

		/* two consecutive dots are not valid */
		if (l == 0)
			return -1;

		if (l >= (maxlen - used - 1))
			return -1;

		dest[used++] = l;
		memcpy(dest + used, dotted, l);
		used += l;

		if (!c)
			break;
		dotted = c + 1;
	}

	if (maxlen < 1)
		return -1;

	dest[used++] = 0;

	return used;
}
#endif


#ifdef L_decoded

/* Decode a dotted string from nameserver transport-level encoding.
   This routine understands compressed data. */
int attribute_hidden __decode_dotted(const unsigned char * const data, int offset,
				  char *dest, int maxlen)
{
	int l;
	bool measure = 1;
	unsigned total = 0;
	unsigned used = 0;

	if (!data)
		return -1;

	while ((l = data[offset++])) {
		if (measure)
			total++;
		if ((l & 0xc0) == (0xc0)) {
			if (measure)
				total++;
			/* compressed item, redirect */
			offset = ((l & 0x3f) << 8) | data[offset];
			measure = 0;
			continue;
		}

		if ((used + l + 1) >= maxlen)
			return -1;

		memcpy(dest + used, data + offset, l);
		offset += l;
		used += l;
		if (measure)
			total += l;

		if (data[offset] != 0)
			dest[used++] = '.';
		else
			dest[used++] = '\0';
	}

	/* The null byte must be counted too */
	if (measure)
		total++;

	DPRINTF("Total decode len = %d\n", total);

	return total;
}
#endif


#ifdef L_lengthd

/* Returns -1 only if data == NULL */
int attribute_hidden __length_dotted(const unsigned char * const data, int offset)
{
	int orig_offset = offset;
	int l;

	if (!data)
		return -1;

	while ((l = data[offset++])) {
		if ((l & 0xc0) == (0xc0)) {
			offset++;
			break;
		}

		offset += l;
	}

	return offset - orig_offset;
}
#endif


#ifdef L_encodeq

int attribute_hidden __encode_question(const struct resolv_question * const q,
					unsigned char *dest, int maxlen)
{
	int i;

	i = __encode_dotted(q->dotted, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;

	if (maxlen < 4)
		return -1;

	dest[0] = (q->qtype & 0xff00) >> 8;
	dest[1] = (q->qtype & 0x00ff) >> 0;
	dest[2] = (q->qclass & 0xff00) >> 8;
	dest[3] = (q->qclass & 0x00ff) >> 0;

	return i + 4;
}
#endif


#ifdef L_decodeq

int attribute_hidden __decode_question(const unsigned char * const message, int offset,
					struct resolv_question *q)
{
	char temp[256];
	int i;

	i = __decode_dotted(message, offset, temp, sizeof(temp));
	if (i < 0)
		return i;

	offset += i;

//TODO: what if strdup fails?
	q->dotted = strdup(temp);
	q->qtype = (message[offset + 0] << 8) | message[offset + 1];
	q->qclass = (message[offset + 2] << 8) | message[offset + 3];

	return i + 4;
}
#endif


#ifdef L_lengthq

/* Returns -1 only if message == NULL */
int attribute_hidden __length_question(const unsigned char * const message, int offset)
{
	int i;

	/* returns -1 only if message == NULL */
	i = __length_dotted(message, offset);
	if (i < 0)
		return i;

	return i + 4;
}
#endif


#ifdef L_encodea

int attribute_hidden __encode_answer(struct resolv_answer *a, unsigned char *dest, int maxlen)
{
	int i;

	i = __encode_dotted(a->dotted, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;

	if (maxlen < (RRFIXEDSZ + a->rdlength))
		return -1;

	*dest++ = (a->atype & 0xff00) >> 8;
	*dest++ = (a->atype & 0x00ff) >> 0;
	*dest++ = (a->aclass & 0xff00) >> 8;
	*dest++ = (a->aclass & 0x00ff) >> 0;
	*dest++ = (a->ttl & 0xff000000) >> 24;
	*dest++ = (a->ttl & 0x00ff0000) >> 16;
	*dest++ = (a->ttl & 0x0000ff00) >> 8;
	*dest++ = (a->ttl & 0x000000ff) >> 0;
	*dest++ = (a->rdlength & 0xff00) >> 8;
	*dest++ = (a->rdlength & 0x00ff) >> 0;
	memcpy(dest, a->rdata, a->rdlength);

	return i + RRFIXEDSZ + a->rdlength;
}
#endif


#ifdef L_decodea

int attribute_hidden __decode_answer(const unsigned char *message, int offset,
				  int len, struct resolv_answer *a)
{
	char temp[256];
	int i;

	DPRINTF("decode_answer(start): off %d, len %d\n", offset, len);
	i = __decode_dotted(message, offset, temp, sizeof(temp));
	if (i < 0)
		return i;

	message += offset + i;
	len -= i + RRFIXEDSZ + offset;
	if (len < 0) {
		DPRINTF("decode_answer: off %d, len %d, i %d\n", offset, len, i);
		return len;
	}

// TODO: what if strdup fails?
	a->dotted = strdup(temp);
	a->atype = (message[0] << 8) | message[1];
	message += 2;
	a->aclass = (message[0] << 8) | message[1];
	message += 2;
	a->ttl = (message[0] << 24) |
		(message[1] << 16) | (message[2] << 8) | (message[3] << 0);
	message += 4;
	a->rdlength = (message[0] << 8) | message[1];
	message += 2;
	a->rdata = message;
	a->rdoffset = offset + i + RRFIXEDSZ;

	DPRINTF("i=%d,rdlength=%d\n", i, a->rdlength);

	if (len < a->rdlength)
		return -1;
	return i + RRFIXEDSZ + a->rdlength;
}
#endif


#ifdef CURRENTLY_UNUSED
#ifdef L_encodep

int __encode_packet(struct resolv_header *h,
	struct resolv_question **q,
	struct resolv_answer **an,
	struct resolv_answer **ns,
	struct resolv_answer **ar,
	unsigned char *dest, int maxlen) attribute_hidden;
int __encode_packet(struct resolv_header *h,
	struct resolv_question **q,
	struct resolv_answer **an,
	struct resolv_answer **ns,
	struct resolv_answer **ar,
	unsigned char *dest, int maxlen)
{
	int i, total = 0;
	unsigned j;

	i = __encode_header(h, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;
	total += i;

	for (j = 0; j < h->qdcount; j++) {
		i = __encode_question(q[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}

	for (j = 0; j < h->ancount; j++) {
		i = __encode_answer(an[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}
	for (j = 0; j < h->nscount; j++) {
		i = __encode_answer(ns[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}
	for (j = 0; j < h->arcount; j++) {
		i = __encode_answer(ar[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}

	return total;
}
#endif


#ifdef L_decodep

int __decode_packet(unsigned char *data, struct resolv_header *h) attribute_hidden;
int __decode_packet(unsigned char *data, struct resolv_header *h)
{
	__decode_header(data, h);
	return HFIXEDSZ;
}
#endif


#ifdef L_formquery

int __form_query(int id, const char *name, int type, unsigned char *packet, int maxlen);
int __form_query(int id, const char *name, int type, unsigned char *packet,
				 int maxlen)
{
	struct resolv_header h;
	struct resolv_question q;
	int i, j;

	memset(&h, 0, sizeof(h));
	h.id = id;
	h.qdcount = 1;

	q.dotted = (char *) name;
	q.qtype = type;
	q.qclass = C_IN; /* CLASS_IN */

	i = __encode_header(&h, packet, maxlen);
	if (i < 0)
		return i;

	j = __encode_question(&q, packet + i, maxlen - i);
	if (j < 0)
		return j;

	return i + j;
}
#endif
#endif /* CURRENTLY_UNUSED */


#ifdef L_opennameservers

# if __BYTE_ORDER == __LITTLE_ENDIAN
#define NAMESERVER_PORT_N (__bswap_constant_16(NAMESERVER_PORT))
#else
#define NAMESERVER_PORT_N NAMESERVER_PORT
#endif

__UCLIBC_MUTEX_INIT(__resolv_lock, PTHREAD_MUTEX_INITIALIZER);

/* Protected by __resolv_lock */
void (*__res_sync)(void);
//uint32_t __resolv_opts;
unsigned __nameservers;
unsigned __searchdomains;
sockaddr46_t *__nameserver;
char **__searchdomain;
#ifdef __UCLIBC_HAS_IPV4__
const struct sockaddr_in __local_nameserver = {
	.sin_family = AF_INET,
	.sin_port = NAMESERVER_PORT_N,
};
#else
const struct sockaddr_in6 __local_nameserver = {
	.sin6_family = AF_INET6,
	.sin6_port = NAMESERVER_PORT_N,
};
#endif

/* Helpers. Both stop on EOL, if it's '\n', it is converted to NUL first */
static char *skip_nospace(char *p)
{
	while (*p != '\0' && !isspace(*p)) {
		if (*p == '\n') {
			*p = '\0';
			break;
		}
		p++;
	}
	return p;
}
static char *skip_and_NUL_space(char *p)
{
	/* NB: '\n' is not isspace! */
	while (1) {
		char c = *p;
		if (c == '\0' || !isspace(c))
			break;
		*p = '\0';
		if (c == '\n' || c == '#')
			break;
		p++;
	}
	return p;
}

/* Must be called under __resolv_lock. */
void attribute_hidden __open_nameservers(void)
{
	static uint8_t last_time;

	char szBuffer[MAXLEN_searchdomain];
	FILE *fp;
	int i;
	sockaddr46_t sa;

	if (!__res_sync) {
		/* Provide for periodic reread of /etc/resolv.conf */
		/* cur_time "ticks" every 256 seconds */
		uint8_t cur_time = ((unsigned)time(NULL)) >> 8;
		if (last_time != cur_time) {
			last_time = cur_time;
			__close_nameservers(); /* force config reread */
		}
	}

	if (__nameservers)
		goto sync;

	fp = fopen("/etc/resolv.conf", "r");
	if (!fp) {
// TODO: why? google says nothing about this...
		fp = fopen("/etc/config/resolv.conf", "r");
	}

	if (fp) {
		while (fgets(szBuffer, sizeof(szBuffer), fp) != NULL) {
			void *ptr;
			char *keyword, *p;

			keyword = p = skip_and_NUL_space(szBuffer);
			/* skip keyword */
			p = skip_nospace(p);
			/* find next word */
			p = skip_and_NUL_space(p);

			if (strcmp(keyword, "nameserver") == 0) {
				/* terminate IP addr */
				*skip_nospace(p) = '\0';
				memset(&sa, 0, sizeof(sa));
				if (0) /* nothing */;
#ifdef __UCLIBC_HAS_IPV6__
				else if (inet_pton(AF_INET6, p, &sa.sa6.sin6_addr) > 0) {
					sa.sa6.sin6_family = AF_INET6;
					sa.sa6.sin6_port = htons(NAMESERVER_PORT);
				}
#endif
#ifdef __UCLIBC_HAS_IPV4__
				else if (inet_pton(AF_INET, p, &sa.sa4.sin_addr) > 0) {
					sa.sa4.sin_family = AF_INET;
					sa.sa4.sin_port = htons(NAMESERVER_PORT);
				}
#endif
				else
					continue; /* garbage on this line */
				ptr = realloc(__nameserver, (__nameservers + 1) * sizeof(__nameserver[0]));
				if (!ptr)
					continue;
				__nameserver = ptr;
				__nameserver[__nameservers++] = sa; /* struct copy */
				continue;
			}
			if (strcmp(keyword, "domain") == 0 || strcmp(keyword, "search") == 0) {
				char *p1;

				/* free old domains ("last 'domain' or 'search' wins" rule) */
				while (__searchdomains)
					free(__searchdomain[--__searchdomains]);
				/*free(__searchdomain);*/
				/*__searchdomain = NULL; - not necessary */
 next_word:
				/* terminate current word */
				p1 = skip_nospace(p);
				/* find next word (maybe) */
				p1 = skip_and_NUL_space(p1);
				/* add it */
				ptr = realloc(__searchdomain, (__searchdomains + 1) * sizeof(__searchdomain[0]));
				if (!ptr)
					continue;
				__searchdomain = ptr;
				/* NB: strlen(p) <= MAXLEN_searchdomain) because szBuffer[] is smaller */
				ptr = strdup(p);
				if (!ptr)
					continue;
				DPRINTF("adding search %s\n", (char*)ptr);
				__searchdomain[__searchdomains++] = (char*)ptr;
				p = p1;
				if (*p)
					goto next_word;
				continue;
			}
			/* if (strcmp(keyword, "sortlist") == 0)... */
			/* if (strcmp(keyword, "options") == 0)... */
		}
		fclose(fp);
	}
	if (__nameservers == 0) {
		/* Have to handle malloc failure! What a mess...
		 * And it's not only here, we need to be careful
		 * to never write into __nameserver[0] if it points
		 * to constant __local_nameserver, or free it. */
		__nameserver = malloc(sizeof(__nameserver[0]));
		if (__nameserver)
			memcpy(__nameserver, &__local_nameserver, sizeof(__local_nameserver));
		else
			__nameserver = (void*) &__local_nameserver;
		__nameservers++;
	}
	if (__searchdomains == 0) {
		char buf[256];
		char *p;
		i = gethostname(buf, sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = '\0';
		if (i == 0 && (p = strchr(buf, '.')) != NULL && p[1]) {
			p = strdup(p + 1);
			if (!p)
				goto err;
			__searchdomain = malloc(sizeof(__searchdomain[0]));
			if (!__searchdomain) {
				free(p);
				goto err;
			}
			__searchdomain[0] = p;
			__searchdomains++;
 err: ;
		}
	}
	DPRINTF("nameservers = %d\n", __nameservers);

 sync:
	if (__res_sync)
		__res_sync();
}
#endif


#ifdef L_closenameservers

/* Must be called under __resolv_lock. */
void attribute_hidden __close_nameservers(void)
{
	if (__nameserver != (void*) &__local_nameserver)
		free(__nameserver);
	__nameserver = NULL;
	__nameservers = 0;
	while (__searchdomains)
		free(__searchdomain[--__searchdomains]);
	free(__searchdomain);
	__searchdomain = NULL;
	/*__searchdomains = 0; - already is */
}
#endif


#ifdef L_dnslookup

/* On entry:
 *  a.buf(len) = auxiliary buffer for IP addresses after first one
 *  a.add_count = how many additional addresses are there already
 *  outpacket = where to save ptr to raw packet? can be NULL
 * On exit:
 *  ret < 0: error, all other data is not valid
 *  a.add_count & a.buf: updated
 *  a.rdlength: length of addresses (4 bytes for IPv4)
 *  *outpacket: updated (packet is malloced, you need to free it)
 *  a.rdata: points into *outpacket to 1st IP addr
 *      NB: don't pass outpacket == NULL if you need to use a.rdata!
 *  a.atype: type of query?
 *  a.dotted: which name we _actually_ used. May contain search domains
 *      appended. (why the filed is called "dotted" I have no idea)
 *      This is a malloced string. May be NULL because strdup failed.
 */
int attribute_hidden __dns_lookup(const char *name, int type,
			unsigned char **outpacket,
			struct resolv_answer *a)
{
	/* Protected by __resolv_lock: */
	static int last_ns_num = 0;
	static uint16_t last_id = 1;

	int i, j, len, fd, pos, rc;
	int name_len;
#ifdef USE_SELECT
	struct timeval tv;
	fd_set fds;
#else
	struct pollfd fds;
#endif
	struct resolv_header h;
	struct resolv_question q;
	struct resolv_answer ma;
	bool first_answer = 1;
	int retries_left;
	unsigned char *packet = malloc(PACKETSZ);
	char *lookup;
	int variant = -1;  /* search domain to append, -1: none */
	int local_ns_num = -1; /* Nth server to use */
	int local_id = local_id; /* for compiler */
	int sdomains;
	bool ends_with_dot;
	sockaddr46_t sa;

	fd = -1;
	lookup = NULL;
	name_len = strlen(name);
	if ((unsigned)name_len >= MAXDNAME - MAXLEN_searchdomain - 2)
		goto fail; /* paranoia */
	lookup = malloc(name_len + 1/*for '.'*/ + MAXLEN_searchdomain + 1);
	if (!packet || !lookup || !name[0])
		goto fail;
	ends_with_dot = (name[name_len - 1] == '.');
	/* no strcpy! paranoia, user might change name[] under us */
	memcpy(lookup, name, name_len);

	DPRINTF("Looking up type %d answer for '%s'\n", type, name);
	retries_left = 0; /* for compiler */
	do {
		unsigned reply_timeout;

		if (fd != -1) {
			close(fd);
			fd = -1;
		}

		/* Mess with globals while under lock */
		/* NB: even data *pointed to* by globals may vanish
		 * outside the locks. We should assume any and all
		 * globals can completely change between locked
		 * code regions. OTOH, this is rare, so we don't need
		 * to handle it "nicely" (do not skip servers,
		 * search domains, etc), we only need to ensure
		 * we do not SEGV, use freed+overwritten data
		 * or do other Really Bad Things. */
		__UCLIBC_MUTEX_LOCK(__resolv_lock);
		__open_nameservers();
		sdomains = __searchdomains;
		lookup[name_len] = '\0';
		if ((unsigned)variant < sdomains) {
			/* lookup is name_len + 1 + MAXLEN_searchdomain + 1 long */
			/* __searchdomain[] is not bigger than MAXLEN_searchdomain */
			lookup[name_len] = '.';
			strcpy(&lookup[name_len + 1], __searchdomain[variant]);
		}
		/* first time? pick starting server etc */
		if (local_ns_num < 0) {
			local_id = last_id;
//TODO: implement /etc/resolv.conf's "options rotate"
// (a.k.a. RES_ROTATE bit in _res.options)
//			local_ns_num = 0;
//			if (_res.options & RES_ROTATE)
				local_ns_num = last_ns_num;
//TODO: use _res.retry
			retries_left = __nameservers * RES_DFLRETRY;
		}
		retries_left--;
		if (local_ns_num >= __nameservers)
			local_ns_num = 0;
		local_id++;
		local_id &= 0xffff;
		/* write new values back while still under lock */
		last_id = local_id;
		last_ns_num = local_ns_num;
		/* struct copy */
		/* can't just take a pointer, __nameserver[x]
		 * is not safe to use outside of locks */
		sa = __nameserver[local_ns_num];
		__UCLIBC_MUTEX_UNLOCK(__resolv_lock);

		memset(packet, 0, PACKETSZ);
		memset(&h, 0, sizeof(h));

		/* encode header */
		h.id = local_id;
		h.qdcount = 1;
		h.rd = 1;
		DPRINTF("encoding header\n", h.rd);
		i = __encode_header(&h, packet, PACKETSZ);
		if (i < 0)
			goto fail;

		/* encode question */
		DPRINTF("lookup name: %s\n", lookup);
		q.dotted = lookup;
		q.qtype = type;
		q.qclass = C_IN; /* CLASS_IN */
		j = __encode_question(&q, packet+i, PACKETSZ-i);
		if (j < 0)
			goto fail;
		len = i + j;

		/* send packet */
		DPRINTF("On try %d, sending query to port %d\n",
				retries_left, NAMESERVER_PORT);
		fd = socket(sa.sa.sa_family, SOCK_DGRAM, IPPROTO_UDP);
		if (fd < 0) /* paranoia */
			goto try_next_server;
		rc = connect(fd, &sa.sa, sizeof(sa));
		if (rc < 0) {
			//if (errno == ENETUNREACH) {
				/* routing error, presume not transient */
				goto try_next_server;
			//}
//For example, what transient error this can be? Can't think of any
			///* retry */
			//continue;
		}
		DPRINTF("Xmit packet len:%d id:%d qr:%d\n", len, h.id, h.qr);
		/* no error check - if it fails, we time out on recv */
		send(fd, packet, len, 0);

#ifdef USE_SELECT
//TODO: use _res.retrans
		reply_timeout = RES_TIMEOUT;
 wait_again:
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec = reply_timeout;
		tv.tv_usec = 0;
		if (select(fd + 1, &fds, NULL, NULL, &tv) <= 0) {
			DPRINTF("Timeout\n");
			/* timed out, so retry send and receive
			 * to next nameserver */
			goto try_next_server;
		}
		reply_timeout--;
#else
		reply_timeout = RES_TIMEOUT * 1000;
 wait_again:
		fds.fd = fd;
		fds.events = POLLIN;
		if (poll(&fds, 1, reply_timeout) <= 0) {
			DPRINTF("Timeout\n");
			/* timed out, so retry send and receive
			 * to next nameserver */
			goto try_next_server;
		}
//TODO: better timeout accounting?
		reply_timeout -= 1000;
#endif
		len = recv(fd, packet, PACKETSZ, MSG_DONTWAIT);
		if (len < HFIXEDSZ) {
			/* too short!
			 * it's just a bogus packet from somewhere */
 bogus_packet:
			if (reply_timeout)
				goto wait_again;
			goto try_next_server;
		}
		__decode_header(packet, &h);
		DPRINTF("id = %d, qr = %d\n", h.id, h.qr);
		if (h.id != local_id || !h.qr) {
			/* unsolicited */
			goto bogus_packet;
		}

		DPRINTF("Got response (i think)!\n");
		DPRINTF("qrcount=%d,ancount=%d,nscount=%d,arcount=%d\n",
				h.qdcount, h.ancount, h.nscount, h.arcount);
		DPRINTF("opcode=%d,aa=%d,tc=%d,rd=%d,ra=%d,rcode=%d\n",
				h.opcode, h.aa, h.tc, h.rd, h.ra, h.rcode);

		/* bug 660 says we treat negative response as an error
		 * and retry, which is, eh, an error. :)
		 * We were incurring long delays because of this. */
		if (h.rcode == NXDOMAIN) {
			/* if possible, try next search domain */
			if (!ends_with_dot) {
				DPRINTF("variant:%d sdomains:%d\n", variant, sdomains);
				if (variant < sdomains - 1) {
					/* next search domain */
					variant++;
					continue;
				}
				/* no more search domains to try */
			}
			/* dont loop, this is "no such host" situation */
			h_errno = HOST_NOT_FOUND;
			goto fail1;
		}
		/* Insert other non-fatal errors here, which do not warrant
		 * switching to next nameserver */

		/* Strange error, assuming this nameserver is feeling bad */
		if (h.rcode != 0)
			goto try_next_server;

		/* Code below won't work correctly with h.ancount == 0, so... */
		if (h.ancount <= 0) {
			h_errno = NO_DATA; /* is this correct code? */
			goto fail1;
		}
		pos = HFIXEDSZ;
		for (j = 0; j < h.qdcount; j++) {
			DPRINTF("Skipping question %d at %d\n", j, pos);
			/* returns -1 only if packet == NULL (can't happen) */
			i = __length_question(packet, pos);
			DPRINTF("Length of question %d is %d\n", j, i);
			pos += i;
		}
		DPRINTF("Decoding answer at pos %d\n", pos);

		first_answer = 1;
		for (j = 0; j < h.ancount && pos < len; j++) {
			i = __decode_answer(packet, pos, len, &ma);
			if (i < 0) {
				DPRINTF("failed decode %d\n", i);
				/* If the message was truncated but we have
				 * decoded some answers, pretend it's OK */
				if (j && h.tc)
					break;
				goto try_next_server;
			}
			pos += i;

			if (first_answer) {
				ma.buf = a->buf;
				ma.buflen = a->buflen;
				ma.add_count = a->add_count;
				memcpy(a, &ma, sizeof(ma));
				if (a->atype != T_SIG && (NULL == a->buf || (type != T_A && type != T_AAAA)))
					break;
				if (a->atype != type) {
					free(a->dotted);
					continue;
				}
				a->add_count = h.ancount - j - 1;
				if ((a->rdlength + sizeof(struct in_addr*)) * a->add_count > a->buflen)
					break;
				a->add_count = 0;
				first_answer = 0;
			} else {
				free(ma.dotted);
				if (ma.atype != type)
					continue;
				if (a->rdlength != ma.rdlength) {
					free(a->dotted);
					DPRINTF("Answer address len(%u) differs from original(%u)\n",
							ma.rdlength, a->rdlength);
					goto try_next_server;
				}
				memcpy(a->buf + (a->add_count * ma.rdlength), ma.rdata, ma.rdlength);
				++a->add_count;
			}
		}

		/* Success! */
		DPRINTF("Answer name = |%s|\n", a->dotted);
		DPRINTF("Answer type = |%d|\n", a->atype);
		if (fd != -1)
			close(fd);
		if (outpacket)
			*outpacket = packet;
		else
			free(packet);
		free(lookup);
		return len;

 try_next_server:
		/* Try next nameserver */
		local_ns_num++;
		variant = -1;
	} while (retries_left > 0);

 fail:
	h_errno = NETDB_INTERNAL;
 fail1:
	if (fd != -1)
		close(fd);
	free(lookup);
	free(packet);
	return -1;
}
#endif


#ifdef L_read_etc_hosts_r

FILE * __open_etc_hosts(void)
{
	FILE * fp;
	if ((fp = fopen("/etc/hosts", "r")) == NULL) {
		fp = fopen("/etc/config/hosts", "r");
	}
	return fp;
}

int attribute_hidden __read_etc_hosts_r(
		FILE * fp,
		const char * name,
		int type,
		enum etc_hosts_action action,
		struct hostent * result_buf,
		char * buf, size_t buflen,
		struct hostent ** result,
		int * h_errnop)
{
	struct in_addr **addr_list = NULL;
	struct in_addr *in = NULL;
	char *cp, **alias;
	int aliases, i, ret = HOST_NOT_FOUND;

	*h_errnop = NETDB_INTERNAL;

	/* make sure pointer is aligned */
	i = ALIGN_BUFFER_OFFSET(buf);
	buf += i;
	buflen -= i;
	/* Layout in buf:
	 * char *alias[ALIAS_DIM];
	 * struct in[6]_addr* addr_list[2];
	 * struct in[6]_addr* in;
	 * char line_buffer[80+];
	 */
#define in6 ((struct in6_addr *)in)
	alias = (char **)buf;
	buf += sizeof(char **) * ALIAS_DIM;
	buflen -= sizeof(char **) * ALIAS_DIM;
	if ((ssize_t)buflen < 0)
		return ERANGE;
	if (action != GETHOSTENT) {
		addr_list = (struct in_addr**)buf;
		buf += sizeof(*addr_list) * 2;
		buflen -= sizeof(*addr_list) * 2;
		in = (struct in_addr*)buf;
#ifndef __UCLIBC_HAS_IPV6__
		buf += sizeof(*in);
		buflen -= sizeof(*in);
#else
		buf += sizeof(*in6);
		buflen -= sizeof(*in6);
#endif
		if ((ssize_t)buflen < 80)
			return ERANGE;

		fp = __open_etc_hosts();
		if (fp == NULL) {
			*result = NULL;
			return errno;
		}
	}
	addr_list[0] = in;
	addr_list[1] = NULL;

	*h_errnop = HOST_NOT_FOUND;
	while (fgets(buf, buflen, fp)) {
		*strchrnul(buf, '#') = '\0';
		DPRINTF("Looking at: %s\n", buf);
		aliases = 0;

		cp = buf;
		while (*cp) {
			while (*cp && isspace(*cp))
				*cp++ = '\0';
			if (!*cp)
				break;
			if (aliases < (2 + MAX_ALIASES))
				alias[aliases++] = cp;
			while (*cp && !isspace(*cp))
				cp++;
		}
		alias[aliases] = NULL;

		if (aliases < 2)
			continue; /* syntax error really */

		if (action == GETHOSTENT) {
			/* Return whatever the next entry happens to be. */
			break;
		}
		if (action == GET_HOSTS_BYADDR) {
			if (strcmp(name, alias[0]) != 0)
				continue;
		} else {
			/* GET_HOSTS_BYNAME */
			for (i = 1; i < aliases; i++)
				if (strcasecmp(name, alias[i]) == 0)
					goto found;
			continue;
 found: ;
		}

		if (0) /* nothing */;
#ifdef __UCLIBC_HAS_IPV4__
		else if (type == AF_INET && inet_pton(AF_INET, alias[0], in) > 0) {
			DPRINTF("Found INET\n");
			result_buf->h_addrtype = AF_INET;
			result_buf->h_length = sizeof(*in);
			result_buf->h_name = alias[1];
			result_buf->h_addr_list = (char**) addr_list;
			result_buf->h_aliases = alias + 2;
			*result = result_buf;
			ret = NETDB_SUCCESS;
		}
#endif
#ifdef __UCLIBC_HAS_IPV6__
		else if (type == AF_INET6 && inet_pton(AF_INET6, alias[0], in6) > 0) {
			DPRINTF("Found INET6\n");
			result_buf->h_addrtype = AF_INET6;
			result_buf->h_length = sizeof(*in6);
			result_buf->h_name = alias[1];
			result_buf->h_addr_list = (char**) addr_list;
			result_buf->h_aliases = alias + 2;
			*result = result_buf;
			ret = NETDB_SUCCESS;
		}
#endif
		else {
			/* continue parsing in the hope the user has multiple
			 * host types listed in the database like so:
			 * <ipv4 addr> host
			 * <ipv6 addr> host
			 * If looking for an IPv6 addr, don't bail when we got the IPv4
			 */
			DPRINTF("Error: Found host but diff network type\n");
			/* NB: gethostbyname2_r depends on this feature
			 * to avoid looking for IPv6 addr of "localhost" etc */
			ret = TRY_AGAIN;
			continue;
		}
		break;
	}
	if (action != GETHOSTENT)
		fclose(fp);
	return ret;
#undef in6
}
#endif


#ifdef L_get_hosts_byname_r

int attribute_hidden __get_hosts_byname_r(const char * name, int type,
			    struct hostent * result_buf,
			    char * buf, size_t buflen,
			    struct hostent ** result,
			    int * h_errnop)
{
	return __read_etc_hosts_r(NULL, name, type, GET_HOSTS_BYNAME,
	                          result_buf, buf, buflen, result, h_errnop);
}
#endif


#ifdef L_get_hosts_byaddr_r

int attribute_hidden __get_hosts_byaddr_r(const char * addr, int len, int type,
			    struct hostent * result_buf,
			    char * buf, size_t buflen,
			    struct hostent ** result,
			    int * h_errnop)
{
#ifndef __UCLIBC_HAS_IPV6__
	char	ipaddr[INET_ADDRSTRLEN];
#else
	char	ipaddr[INET6_ADDRSTRLEN];
#endif

	switch (type) {
#ifdef __UCLIBC_HAS_IPV4__
		case AF_INET:
			if (len != sizeof(struct in_addr))
				return 0;
			break;
#endif
#ifdef __UCLIBC_HAS_IPV6__
		case AF_INET6:
			if (len != sizeof(struct in6_addr))
				return 0;
			break;
#endif
		default:
			return 0;
	}

	inet_ntop(type, addr, ipaddr, sizeof(ipaddr));

	return __read_etc_hosts_r(NULL, ipaddr, type, GET_HOSTS_BYADDR,
				result_buf, buf, buflen, result, h_errnop);
}
#endif


#ifdef L_getnameinfo

int getnameinfo(const struct sockaddr *sa, socklen_t addrlen, char *host,
				 socklen_t hostlen, char *serv, socklen_t servlen,
				 unsigned int flags)
{
	int serrno = errno;
	unsigned ok;
	struct hostent *h = NULL;
	char domain[256];

	if (flags & ~(NI_NUMERICHOST|NI_NUMERICSERV|NI_NOFQDN|NI_NAMEREQD|NI_DGRAM))
		return EAI_BADFLAGS;

	if (sa == NULL || addrlen < sizeof(sa_family_t))
		return EAI_FAMILY;

	ok = sa->sa_family;
	if (ok == AF_LOCAL) /* valid */;
#ifdef __UCLIBC_HAS_IPV4__
	else if (ok == AF_INET) {
		if (addrlen < sizeof(struct sockaddr_in))
			return EAI_FAMILY;
	}
#endif
#ifdef __UCLIBC_HAS_IPV6__
	else if (ok == AF_INET6) {
		if (addrlen < sizeof(struct sockaddr_in6))
			return EAI_FAMILY;
	}
#endif
	else
		return EAI_FAMILY;

	ok = 0;
	if (host != NULL && hostlen > 0)
		switch (sa->sa_family) {
		case AF_INET:
#ifdef __UCLIBC_HAS_IPV6__
		case AF_INET6:
#endif
			if (!(flags & NI_NUMERICHOST)) {
				if (0) /* nothing */;
#ifdef __UCLIBC_HAS_IPV6__
				else if (sa->sa_family == AF_INET6)
					h = gethostbyaddr((const void *)
						&(((const struct sockaddr_in6 *) sa)->sin6_addr),
						sizeof(struct in6_addr), AF_INET6);
#endif
#ifdef __UCLIBC_HAS_IPV4__
				else
					h = gethostbyaddr((const void *)
						&(((const struct sockaddr_in *)sa)->sin_addr),
						sizeof(struct in_addr), AF_INET);
#endif

				if (h) {
					char *c;
#undef min
#define min(x,y) (((x) > (y)) ? (y) : (x))
					if ((flags & NI_NOFQDN)
					 && (getdomainname(domain, sizeof(domain)) == 0)
					 && (c = strstr(h->h_name, domain)) != NULL
					 && (c != h->h_name) && (*(--c) == '.')
					) {
						strncpy(host, h->h_name,
							min(hostlen, (size_t) (c - h->h_name)));
						host[min(hostlen - 1, (size_t) (c - h->h_name))] = '\0';
					} else {
						strncpy(host, h->h_name, hostlen);
					}
					ok = 1;
#undef min
				}
			}

			if (!ok) {
				const char *c = NULL;

				if (flags & NI_NAMEREQD) {
					errno = serrno;
					return EAI_NONAME;
				}
				if (0) /* nothing */;
#ifdef __UCLIBC_HAS_IPV6__
				else if (sa->sa_family == AF_INET6) {
					const struct sockaddr_in6 *sin6p;

					sin6p = (const struct sockaddr_in6 *) sa;
					c = inet_ntop(AF_INET6,
						(const void *) &sin6p->sin6_addr,
						host, hostlen);
#if 0
					/* Does scope id need to be supported? */
					uint32_t scopeid;
					scopeid = sin6p->sin6_scope_id;
					if (scopeid != 0) {
						/* Buffer is >= IFNAMSIZ+1.  */
						char scopebuf[IFNAMSIZ + 1];
						char *scopeptr;
						int ni_numericscope = 0;
						size_t real_hostlen = strnlen(host, hostlen);
						size_t scopelen = 0;

						scopebuf[0] = SCOPE_DELIMITER;
						scopebuf[1] = '\0';
						scopeptr = &scopebuf[1];

						if (IN6_IS_ADDR_LINKLOCAL(&sin6p->sin6_addr)
						    || IN6_IS_ADDR_MC_LINKLOCAL(&sin6p->sin6_addr)) {
							if (if_indextoname(scopeid, scopeptr) == NULL)
								++ni_numericscope;
							else
								scopelen = strlen(scopebuf);
						} else {
							++ni_numericscope;
						}

						if (ni_numericscope)
							scopelen = 1 + snprintf(scopeptr,
								(scopebuf
								+ sizeof scopebuf
								- scopeptr),
								"%u", scopeid);

						if (real_hostlen + scopelen + 1 > hostlen)
							return EAI_SYSTEM;
						memcpy(host + real_hostlen, scopebuf, scopelen + 1);
					}
#endif
				}
#endif /* __UCLIBC_HAS_IPV6__ */
#if defined __UCLIBC_HAS_IPV4__
				else {
					c = inet_ntop(AF_INET, (const void *)
						&(((const struct sockaddr_in *) sa)->sin_addr),
						host, hostlen);
				}
#endif
				if (c == NULL) {
					errno = serrno;
					return EAI_SYSTEM;
				}
				ok = 1;
			}
			break;

		case AF_LOCAL:
			if (!(flags & NI_NUMERICHOST)) {
				struct utsname utsname;

				if (!uname(&utsname)) {
					strncpy(host, utsname.nodename, hostlen);
					break;
				};
			};

			if (flags & NI_NAMEREQD) {
				errno = serrno;
				return EAI_NONAME;
			}

			strncpy(host, "localhost", hostlen);
			break;
/* Already checked above
		default:
			return EAI_FAMILY;
*/
	}

	if (serv && (servlen > 0)) {
		if (sa->sa_family == AF_LOCAL) {
			strncpy(serv, ((const struct sockaddr_un *) sa)->sun_path, servlen);
		} else { /* AF_INET || AF_INET6 */
			if (!(flags & NI_NUMERICSERV)) {
				struct servent *s;
				s = getservbyport(((const struct sockaddr_in *) sa)->sin_port,
				      ((flags & NI_DGRAM) ? "udp" : "tcp"));
				if (s) {
					strncpy(serv, s->s_name, servlen);
					goto DONE;
				}
			}
			snprintf(serv, servlen, "%d",
				ntohs(((const struct sockaddr_in *) sa)->sin_port));
		}
	}
DONE:
	if (host && (hostlen > 0))
		host[hostlen-1] = 0;
	if (serv && (servlen > 0))
		serv[servlen-1] = 0;
	errno = serrno;
	return 0;
}
libc_hidden_def(getnameinfo)
#endif


#ifdef L_gethostbyname_r
//Does this function assume IPv4? If yes,
//what happens in IPv6-only build?

/* Bug 671 says:
 * "uClibc resolver's gethostbyname does not return the requested name
 * as an alias, but instead returns the canonical name. glibc's
 * gethostbyname has a similar bug where it returns the requested name
 * with the search domain name appended (to make a FQDN) as an alias,
 * but not the original name itself. Both contradict POSIX, which says
 * that the name argument passed to gethostbyname must be in the alias list"
 * This is fixed now, and we differ from glibc:
 *
 * $ ./gethostbyname_uclibc wer.google.com
 * h_name:'c13-ss-2-lb.cnet.com'
 * h_length:4
 * h_addrtype:2 AF_INET
 * alias:'wer.google.com' <===
 * addr: 0x4174efd8 '216.239.116.65'
 *
 * $ ./gethostbyname_glibc wer.google.com
 * h_name:'c13-ss-2-lb.cnet.com'
 * h_length:4
 * h_addrtype:2 AF_INET
 * alias:'wer.google.com.com' <===
 * addr:'216.239.116.65'
 *
 * When examples were run, /etc/resolv.conf contained "search com" line.
 */
int gethostbyname_r(const char * name,
		struct hostent * result_buf,
		char * buf,
		size_t buflen,
		struct hostent ** result,
		int * h_errnop)
{
	struct in_addr **addr_list;
	char **alias;
	char *alias0;
	unsigned char *packet;
	struct resolv_answer a;
	int i;
	int wrong_af = 0;

	*result = NULL;
	if (!name)
		return EINVAL;

	/* do /etc/hosts first */
	{
		int old_errno = errno;  /* save the old errno and reset errno */
		__set_errno(0);         /* to check for missing /etc/hosts. */
		i = __get_hosts_byname_r(name, AF_INET, result_buf,
				buf, buflen, result, h_errnop);
		if (i == NETDB_SUCCESS) {
			__set_errno(old_errno);
			return i;
		}
		switch (*h_errnop) {
			case HOST_NOT_FOUND:
				wrong_af = (i == TRY_AGAIN);
			case NO_ADDRESS:
				break;
			case NETDB_INTERNAL:
				if (errno == ENOENT) {
					break;
				}
				/* else fall through */
			default:
				return i;
		}
		__set_errno(old_errno);
	}

	DPRINTF("Nothing found in /etc/hosts\n");

	*h_errnop = NETDB_INTERNAL;

	/* prepare future h_aliases[0] */
	i = strlen(name) + 1;
	if ((ssize_t)buflen <= i)
		return ERANGE;
	memcpy(buf, name, i); /* paranoia: name might change */
	alias0 = buf;
	buf += i;
	buflen -= i;
	/* make sure pointer is aligned */
	i = ALIGN_BUFFER_OFFSET(buf);
	buf += i;
	buflen -= i;
	/* Layout in buf:
	 * char *alias[2];
	 * struct in_addr* addr_list[NN+1];
	 * struct in_addr* in[NN];
	 */
	alias = (char **)buf;
	buf += sizeof(alias[0]) * 2;
	buflen -= sizeof(alias[0]) * 2;
	addr_list = (struct in_addr **)buf;
	/* buflen may be < 0, must do signed compare */
	if ((ssize_t)buflen < 256)
		return ERANGE;

	/* we store only one "alias" - the name itself */
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO -- generate the full list
#endif
	alias[0] = alias0;
	alias[1] = NULL;

	/* maybe it is already an address? */
	{
		struct in_addr *in = (struct in_addr *)(buf + sizeof(addr_list[0]) * 2);
		if (inet_aton(name, in)) {
			addr_list[0] = in;
			addr_list[1] = NULL;
			result_buf->h_name = alias0;
			result_buf->h_aliases = alias;
			result_buf->h_addrtype = AF_INET;
			result_buf->h_length = sizeof(struct in_addr);
			result_buf->h_addr_list = (char **) addr_list;
			*result = result_buf;
			*h_errnop = NETDB_SUCCESS;
			return NETDB_SUCCESS;
		}
	}

	/* what if /etc/hosts has it but it's not IPv4?
	 * F.e. "::1 localhost6". We don't do DNS query for such hosts -
	 * "ping localhost6" should be fast even if DNS server is down! */
	if (wrong_af) {
		*h_errnop = HOST_NOT_FOUND;
		return TRY_AGAIN;
	}

	/* talk to DNS servers */
	a.buf = buf;
	/* take into account that at least one address will be there,
	 * we'll need space of one in_addr + two addr_list[] elems */
	a.buflen = buflen - ((sizeof(addr_list[0]) * 2 + sizeof(struct in_addr)));
	a.add_count = 0;
	i = __dns_lookup(name, T_A, &packet, &a);
	if (i < 0) {
		*h_errnop = HOST_NOT_FOUND;
		DPRINTF("__dns_lookup returned < 0\n");
		return TRY_AGAIN;
	}

	if (a.atype == T_A) { /* ADDRESS */
		/* we need space for addr_list[] and one IPv4 address */
		/* + 1 accounting for 1st addr (it's in a.rdata),
		 * another + 1 for NULL in last addr_list[]: */
		int need_bytes = sizeof(addr_list[0]) * (a.add_count + 1 + 1)
				/* for 1st addr (it's in a.rdata): */
				+ sizeof(struct in_addr);
		/* how many bytes will 2nd and following addresses take? */
		int ips_len = a.add_count * a.rdlength;

		buflen -= (need_bytes + ips_len);
		if ((ssize_t)buflen < 0) {
			DPRINTF("buffer too small for all addresses\n");
			/* *h_errnop = NETDB_INTERNAL; - already is */
			i = ERANGE;
			goto free_and_ret;
		}

		/* if there are additional addresses in buf,
		 * move them forward so that they are not destroyed */
		DPRINTF("a.add_count:%d a.rdlength:%d a.rdata:%p\n", a.add_count, a.rdlength, a.rdata);
		memmove(buf + need_bytes, buf, ips_len);

		/* 1st address is in a.rdata, insert it  */
		buf += need_bytes - sizeof(struct in_addr);
		memcpy(buf, a.rdata, sizeof(struct in_addr));

		/* fill addr_list[] */
		for (i = 0; i <= a.add_count; i++) {
			addr_list[i] = (struct in_addr*)buf;
			buf += sizeof(struct in_addr);
		}
		addr_list[i] = NULL;

		/* if we have enough space, we can report "better" name
		 * (it may contain search domains attached by __dns_lookup,
		 * or CNAME of the host if it is different from the name
		 * we used to find it) */
		if (a.dotted && buflen > strlen(a.dotted)) {
			strcpy(buf, a.dotted);
			alias0 = buf;
		}

		result_buf->h_name = alias0;
		result_buf->h_aliases = alias;
		result_buf->h_addrtype = AF_INET;
		result_buf->h_length = sizeof(struct in_addr);
		result_buf->h_addr_list = (char **) addr_list;
		*result = result_buf;
		*h_errnop = NETDB_SUCCESS;
		i = NETDB_SUCCESS;
		goto free_and_ret;
	}

	*h_errnop = HOST_NOT_FOUND;
	i = TRY_AGAIN;

 free_and_ret:
	free(a.dotted);
	free(packet);
	return i;
}
libc_hidden_def(gethostbyname_r)
#endif


#ifdef L_gethostbyname2_r

int gethostbyname2_r(const char *name,
		int family,
		struct hostent * result_buf,
		char * buf,
		size_t buflen,
		struct hostent ** result,
		int * h_errnop)
{
#ifndef __UCLIBC_HAS_IPV6__
	return family == (AF_INET)
		? gethostbyname_r(name, result_buf, buf, buflen, result, h_errnop)
		: HOST_NOT_FOUND;
#else
	struct in6_addr *in;
	struct in6_addr **addr_list;
	unsigned char *packet;
	struct resolv_answer a;
	int i;
	int nest = 0;
	int wrong_af = 0;

	if (family == AF_INET)
		return gethostbyname_r(name, result_buf, buf, buflen, result, h_errnop);

	*result = NULL;
	if (family != AF_INET6)
		return EINVAL;

	if (!name)
		return EINVAL;

	/* do /etc/hosts first */
	{
		int old_errno = errno;	/* Save the old errno and reset errno */
		__set_errno(0);			/* to check for missing /etc/hosts. */

		i = __get_hosts_byname_r(name, AF_INET6 /*family*/, result_buf,
				buf, buflen, result, h_errnop);
		if (i == NETDB_SUCCESS) {
			__set_errno(old_errno);
			return i;
		}
		switch (*h_errnop) {
			case HOST_NOT_FOUND:
				wrong_af = (i == TRY_AGAIN);
			case NO_ADDRESS:
				break;
			case NETDB_INTERNAL:
				if (errno == ENOENT) {
					break;
				}
				/* else fall through */
			default:
				return i;
		}
		__set_errno(old_errno);
	}
	DPRINTF("Nothing found in /etc/hosts\n");

	*h_errnop = NETDB_INTERNAL;

	/* make sure pointer is aligned */
	i = ALIGN_BUFFER_OFFSET(buf);
	buf += i;
	buflen -= i;
	/* Layout in buf:
	 * struct in6_addr* in;
	 * struct in6_addr* addr_list[2];
	 * char scratch_buf[256];
	 */
	in = (struct in6_addr*)buf;
	buf += sizeof(*in);
	buflen -= sizeof(*in);
	addr_list = (struct in6_addr**)buf;
	buf += sizeof(*addr_list) * 2;
	buflen -= sizeof(*addr_list) * 2;
	if ((ssize_t)buflen < 256)
		return ERANGE;
	addr_list[0] = in;
	addr_list[1] = NULL;
	strncpy(buf, name, buflen);
	buf[buflen] = '\0';

	/* maybe it is already an address? */
	if (inet_pton(AF_INET6, name, in)) {
		result_buf->h_name = buf;
		result_buf->h_addrtype = AF_INET6;
		result_buf->h_length = sizeof(*in);
		result_buf->h_addr_list = (char **) addr_list;
		//result_buf->h_aliases = ???
		*result = result_buf;
		*h_errnop = NETDB_SUCCESS;
		return NETDB_SUCCESS;
	}

	/* what if /etc/hosts has it but it's not IPv6?
	 * F.e. "127.0.0.1 localhost". We don't do DNS query for such hosts -
	 * "ping localhost" should be fast even if DNS server is down! */
	if (wrong_af) {
		*h_errnop = HOST_NOT_FOUND;
		return TRY_AGAIN;
	}

	/* talk to DNS servers */
// TODO: why it's so different from gethostbyname_r (IPv4 case)?
	memset(&a, '\0', sizeof(a));
	for (;;) {
// Hmm why we memset(a) to zeros only once?
		i = __dns_lookup(buf, T_AAAA, &packet, &a);
		if (i < 0) {
			*h_errnop = HOST_NOT_FOUND;
			return TRY_AGAIN;
		}
		strncpy(buf, a.dotted, buflen);
		free(a.dotted);

		if (a.atype != T_CNAME)
			break;

		DPRINTF("Got a CNAME in gethostbyname()\n");
		if (++nest > MAX_RECURSE) {
			*h_errnop = NO_RECOVERY;
			return -1;
		}
		i = __decode_dotted(packet, a.rdoffset, buf, buflen);
		free(packet);
		if (i < 0) {
			*h_errnop = NO_RECOVERY;
			return -1;
		}
	}
	if (a.atype == T_AAAA) {	/* ADDRESS */
		memcpy(in, a.rdata, sizeof(*in));
		result_buf->h_name = buf;
		result_buf->h_addrtype = AF_INET6;
		result_buf->h_length = sizeof(*in);
		result_buf->h_addr_list = (char **) addr_list;
		//result_buf->h_aliases = ???
		free(packet);
		*result = result_buf;
		*h_errnop = NETDB_SUCCESS;
		return NETDB_SUCCESS;
	}
	free(packet);
	*h_errnop = HOST_NOT_FOUND;
	return TRY_AGAIN;

#endif /* __UCLIBC_HAS_IPV6__ */
}
libc_hidden_def(gethostbyname2_r)
#endif


#ifdef L_gethostbyaddr_r

int gethostbyaddr_r(const void *addr, socklen_t addrlen, int type,
					 struct hostent * result_buf,
					 char * buf, size_t buflen,
					 struct hostent ** result,
					 int * h_errnop)

{
	struct in_addr *in;
	struct in_addr **addr_list;
	char **alias;
	unsigned char *packet;
	struct resolv_answer a;
	int i;
	int nest = 0;

	*result = NULL;
	if (!addr)
		return EINVAL;

	switch (type) {
#ifdef __UCLIBC_HAS_IPV4__
		case AF_INET:
			if (addrlen != sizeof(struct in_addr))
				return EINVAL;
			break;
#endif
#ifdef __UCLIBC_HAS_IPV6__
		case AF_INET6:
			if (addrlen != sizeof(struct in6_addr))
				return EINVAL;
			break;
#endif
		default:
			return EINVAL;
	}

	/* do /etc/hosts first */
	i = __get_hosts_byaddr_r(addr, addrlen, type, result_buf,
				buf, buflen, result, h_errnop);
	if (i == 0)
		return i;
	switch (*h_errnop) {
		case HOST_NOT_FOUND:
		case NO_ADDRESS:
			break;
		default:
			return i;
	}

	*h_errnop = NETDB_INTERNAL;

	/* make sure pointer is aligned */
	i = ALIGN_BUFFER_OFFSET(buf);
	buf += i;
	buflen -= i;
	/* Layout in buf:
	 * char *alias[ALIAS_DIM];
	 * struct in[6]_addr* addr_list[2];
	 * struct in[6]_addr* in;
	 * char scratch_buffer[256+];
	 */
#define in6 ((struct in6_addr *)in)
	alias = (char **)buf;
	buf += sizeof(*alias) * ALIAS_DIM;
	buflen -= sizeof(*alias) * ALIAS_DIM;
	addr_list = (struct in_addr**)buf;
	buf += sizeof(*addr_list) * 2;
	buflen -= sizeof(*addr_list) * 2;
	in = (struct in_addr*)buf;
#ifndef __UCLIBC_HAS_IPV6__
	buf += sizeof(*in);
	buflen -= sizeof(*in);
#else
	buf += sizeof(*in6);
	buflen -= sizeof(*in6);
#endif
	if ((ssize_t)buflen < 256)
		return ERANGE;
	alias[0] = buf;
	alias[1] = NULL;
	addr_list[0] = in;
	addr_list[1] = NULL;
	memcpy(&in, addr, addrlen);

	if (0) /* nothing */;
#ifdef __UCLIBC_HAS_IPV4__
	else IF_HAS_BOTH(if (type == AF_INET)) {
		unsigned char *tp = (unsigned char *)addr;
		sprintf(buf, "%u.%u.%u.%u.in-addr.arpa",
				tp[3], tp[2], tp[1], tp[0]);
	}
#endif
#ifdef __UCLIBC_HAS_IPV6__
	else {
		char *dst = buf;
		unsigned char *tp = (unsigned char *)addr + addrlen - 1;
		do {
			dst += sprintf(dst, "%x.%x.", tp[i] & 0xf, tp[i] >> 4);
			tp--;
		} while (tp >= (unsigned char *)addr);
		strcpy(dst, "ip6.arpa");
	}
#endif

	memset(&a, '\0', sizeof(a));
	for (;;) {
// Hmm why we memset(a) to zeros only once?
		i = __dns_lookup(buf, T_PTR, &packet, &a);
		if (i < 0) {
			*h_errnop = HOST_NOT_FOUND;
			return TRY_AGAIN;
		}

		strncpy(buf, a.dotted, buflen);
		free(a.dotted);
		if (a.atype != T_CNAME)
			break;

		DPRINTF("Got a CNAME in gethostbyaddr()\n");
		if (++nest > MAX_RECURSE) {
			*h_errnop = NO_RECOVERY;
			return -1;
		}
		/* Decode CNAME into buf, feed it to __dns_lookup() again */
		i = __decode_dotted(packet, a.rdoffset, buf, buflen);
		free(packet);
		if (i < 0) {
			*h_errnop = NO_RECOVERY;
			return -1;
		}
	}

	if (a.atype == T_PTR) {	/* ADDRESS */
		i = __decode_dotted(packet, a.rdoffset, buf, buflen);
		free(packet);
		result_buf->h_name = buf;
		result_buf->h_addrtype = type;
		result_buf->h_length = addrlen;
		result_buf->h_addr_list = (char **) addr_list;
		result_buf->h_aliases = alias;
		*result = result_buf;
		*h_errnop = NETDB_SUCCESS;
		return NETDB_SUCCESS;
	}

	free(packet);
	*h_errnop = NO_ADDRESS;
	return TRY_AGAIN;
#undef in6
}
libc_hidden_def(gethostbyaddr_r)
#endif


#ifdef L_gethostent_r

__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_MUTEX_INITIALIZER);

static smallint __stay_open;
static FILE * __gethostent_fp;

void endhostent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	__stay_open = 0;
	if (__gethostent_fp) {
		fclose(__gethostent_fp);
		__gethostent_fp = NULL;
	}
	__UCLIBC_MUTEX_UNLOCK(mylock);
}

void sethostent(int stay_open)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	__stay_open = (stay_open != 0);
	__UCLIBC_MUTEX_UNLOCK(mylock);
}

int gethostent_r(struct hostent *result_buf, char *buf, size_t buflen,
	struct hostent **result, int *h_errnop)
{
	int ret;

	__UCLIBC_MUTEX_LOCK(mylock);
	if (__gethostent_fp == NULL) {
		__gethostent_fp = __open_etc_hosts();
		if (__gethostent_fp == NULL) {
			*result = NULL;
			ret = TRY_AGAIN;
			goto DONE;
		}
	}

	ret = __read_etc_hosts_r(__gethostent_fp, NULL, AF_INET, GETHOSTENT,
		   result_buf, buf, buflen, result, h_errnop);
	if (__stay_open == 0) {
		fclose(__gethostent_fp);
		__gethostent_fp = NULL;
	}
DONE:
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return ret;
}
libc_hidden_def(gethostent_r)
#endif


#ifdef L_gethostent

struct hostent *gethostent(void)
{
	static struct hostent h;
	static char buf[
#ifndef __UCLIBC_HAS_IPV6__
			sizeof(struct in_addr) + sizeof(struct in_addr *) * 2 +
#else
			sizeof(struct in6_addr) + sizeof(struct in6_addr *) * 2 +
#endif /* __UCLIBC_HAS_IPV6__ */
			sizeof(char *) * ALIAS_DIM +
			80 /*namebuffer*/ + 2 /* margin */];
	struct hostent *host;

	gethostent_r(&h, buf, sizeof(buf), &host, &h_errno);
	return host;
}
#endif


#ifdef L_gethostbyname2

struct hostent *gethostbyname2(const char *name, int family)
{
#ifndef __UCLIBC_HAS_IPV6__
	return family == AF_INET ? gethostbyname(name) : (struct hostent*)NULL;
#else
	static struct hostent h;
	static char buf[sizeof(struct in6_addr) +
			sizeof(struct in6_addr *) * 2 +
			sizeof(char *)*ALIAS_DIM + 384/*namebuffer*/ + 32/* margin */];
	struct hostent *hp;

	gethostbyname2_r(name, family, &h, buf, sizeof(buf), &hp, &h_errno);
	return hp;
#endif
}
libc_hidden_def(gethostbyname2)
#endif


#ifdef L_gethostbyname

struct hostent *gethostbyname(const char *name)
{
#ifndef __UCLIBC_HAS_IPV6__
	static struct hostent h;
	static char buf[sizeof(struct in_addr) +
			sizeof(struct in_addr *) * 2 +
			sizeof(char *)*ALIAS_DIM + 384/*namebuffer*/ + 32/* margin */];
	struct hostent *hp;

	gethostbyname_r(name, &h, buf, sizeof(buf), &hp, &h_errno);
	return hp;
#else
	return gethostbyname2(name, AF_INET);
#endif
}
libc_hidden_def(gethostbyname)
#endif


#ifdef L_gethostbyaddr

struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type)
{
	static struct hostent h;
	static char buf[
#ifndef __UCLIBC_HAS_IPV6__
			sizeof(struct in_addr) + sizeof(struct in_addr *)*2 +
#else
			sizeof(struct in6_addr) + sizeof(struct in6_addr *)*2 +
#endif /* __UCLIBC_HAS_IPV6__ */
			sizeof(char *)*ALIAS_DIM + 384 /*namebuffer*/ + 32 /* margin */];
	struct hostent *hp;

	gethostbyaddr_r(addr, len, type, &h, buf, sizeof(buf), &hp, &h_errno);
	return hp;
}
libc_hidden_def(gethostbyaddr)
#endif


#ifdef L_res_comp

/*
 * Expand compressed domain name 'comp_dn' to full domain name.
 * 'msg' is a pointer to the begining of the message,
 * 'eomorig' points to the first location after the message,
 * 'exp_dn' is a pointer to a buffer of size 'length' for the result.
 * Return size of compressed name or -1 if there was an error.
 */
int __dn_expand(const u_char *msg, const u_char *eom, const u_char *src,
				char *dst, int dstsiz)
{
	int n = ns_name_uncompress(msg, eom, src, dst, (size_t)dstsiz);

	if (n > 0 && dst[0] == '.')
		dst[0] = '\0';
	return n;
}
#endif /* L_res_comp */


#ifdef L_ns_name

/* Thinking in noninternationalized USASCII (per the DNS spec),
 * is this character visible and not a space when printed ?
 */
static int printable(int ch)
{
	return (ch > 0x20 && ch < 0x7f);
}
/* Thinking in noninternationalized USASCII (per the DNS spec),
 * is this characted special ("in need of quoting") ?
 */
static int special(int ch)
{
	switch (ch) {
		case 0x22: /* '"' */
		case 0x2E: /* '.' */
		case 0x3B: /* ';' */
		case 0x5C: /* '\\' */
			/* Special modifiers in zone files. */
		case 0x40: /* '@' */
		case 0x24: /* '$' */
			return 1;
		default:
			return 0;
	}
}

/*
 * ns_name_uncompress(msg, eom, src, dst, dstsiz)
 *      Expand compressed domain name to presentation format.
 * return:
 *      Number of bytes read out of `src', or -1 (with errno set).
 * note:
 *      Root domain returns as "." not "".
 */
int ns_name_uncompress(const u_char *msg, const u_char *eom,
		const u_char *src, char *dst, size_t dstsiz)
{
	u_char tmp[NS_MAXCDNAME];
	int n;

	n = ns_name_unpack(msg, eom, src, tmp, sizeof tmp);
	if (n == -1)
		return -1;
	if (ns_name_ntop(tmp, dst, dstsiz) == -1)
		return -1;
	return n;
}
libc_hidden_def(ns_name_uncompress)

/*
 * ns_name_ntop(src, dst, dstsiz)
 *      Convert an encoded domain name to printable ascii as per RFC1035.
 * return:
 *      Number of bytes written to buffer, or -1 (with errno set)
 * notes:
 *      The root is returned as "."
 *      All other domains are returned in non absolute form
 */
int ns_name_ntop(const u_char *src, char *dst, size_t dstsiz)
{
	static const char digits[] = "0123456789";

	const u_char *cp;
	char *dn, *eom;
	u_char c;
	u_int n;

	cp = src;
	dn = dst;
	eom = dst + dstsiz;

	while ((n = *cp++) != 0) {
		if ((n & NS_CMPRSFLGS) != 0) {
			/* Some kind of compression pointer. */
			__set_errno(EMSGSIZE);
			return -1;
		}
		if (dn != dst) {
			if (dn >= eom) {
				__set_errno(EMSGSIZE);
				return -1;
			}
			*dn++ = '.';
		}
		if (dn + n >= eom) {
			__set_errno(EMSGSIZE);
			return -1;
		}
		for (; n > 0; n--) {
			c = *cp++;
			if (special(c)) {
				if (dn + 1 >= eom) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				*dn++ = '\\';
				*dn++ = (char)c;
			} else if (!printable(c)) {
				if (dn + 3 >= eom) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				*dn++ = '\\';
				*dn++ = digits[c / 100];
				*dn++ = digits[(c % 100) / 10];
				*dn++ = digits[c % 10];
			} else {
				if (dn >= eom) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				*dn++ = (char)c;
			}
		}
	}
	if (dn == dst) {
		if (dn >= eom) {
			__set_errno(EMSGSIZE);
			return -1;
		}
		*dn++ = '.';
	}
	if (dn >= eom) {
		__set_errno(EMSGSIZE);
		return -1;
	}
	*dn++ = '\0';
	return (dn - dst);
}
libc_hidden_def(ns_name_ntop)

/*
 * ns_name_unpack(msg, eom, src, dst, dstsiz)
 *      Unpack a domain name from a message, source may be compressed.
 * return:
 *      -1 if it fails, or consumed octets if it succeeds.
 */
int ns_name_unpack(const u_char *msg, const u_char *eom, const u_char *src,
               u_char *dst, size_t dstsiz)
{
	const u_char *srcp, *dstlim;
	u_char *dstp;
	int n, len, checked;

	len = -1;
	checked = 0;
	dstp = dst;
	srcp = src;
	dstlim = dst + dstsiz;
	if (srcp < msg || srcp >= eom) {
		__set_errno(EMSGSIZE);
		return -1;
	}
	/* Fetch next label in domain name. */
	while ((n = *srcp++) != 0) {
		/* Check for indirection. */
		switch (n & NS_CMPRSFLGS) {
			case 0:
				/* Limit checks. */
				if (dstp + n + 1 >= dstlim || srcp + n >= eom) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				checked += n + 1;
				*dstp++ = n;
				memcpy(dstp, srcp, n);
				dstp += n;
				srcp += n;
				break;

			case NS_CMPRSFLGS:
				if (srcp >= eom) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				if (len < 0)
					len = srcp - src + 1;
				srcp = msg + (((n & 0x3f) << 8) | (*srcp & 0xff));
				if (srcp < msg || srcp >= eom) {  /* Out of range. */
					__set_errno(EMSGSIZE);
					return -1;
				}
				checked += 2;
				/*
				 * Check for loops in the compressed name;
				 * if we've looked at the whole message,
				 * there must be a loop.
				 */
				if (checked >= eom - msg) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				break;

			default:
				__set_errno(EMSGSIZE);
				return -1;                    /* flag error */
		}
	}
	*dstp = '\0';
	if (len < 0)
		len = srcp - src;
	return len;
}
libc_hidden_def(ns_name_unpack)
#endif /* L_ns_name */


#ifdef L_res_init

/* Protected by __resolv_lock */
struct __res_state _res;

/* Will be called under __resolv_lock. */
static void res_sync_func(void)
{
	struct __res_state *rp = &(_res);
	int n;

	/* If we didn't get malloc failure earlier... */
	if (__nameserver != (void*) &__local_nameserver) {
		/* TODO:
		 * if (__nameservers < rp->nscount) - try to grow __nameserver[]?
		 */
#ifdef __UCLIBC_HAS_IPV6__
		if (__nameservers > rp->_u._ext.nscount)
			__nameservers = rp->_u._ext.nscount;
		n = __nameservers;
		while (--n >= 0)
			__nameserver[n].sa6 = *rp->_u._ext.nsaddrs[n]; /* struct copy */
#else /* IPv4 only */
		if (__nameservers > rp->nscount)
			__nameservers = rp->nscount;
		n = __nameservers;
		while (--n >= 0)
			__nameserver[n].sa4 = rp->nsaddr_list[n]; /* struct copy */
#endif
	}
	/* Extend and comment what program is known
	 * to use which _res.XXX member(s).
	 */
	// __resolv_opts = rp->options;
	// ...
}

/* Our res_init never fails (always returns 0) */
int res_init(void)
{
	struct __res_state *rp = &(_res);
	int i;
	int n;
#ifdef __UCLIBC_HAS_IPV6__
	int m = 0;
#endif

	__UCLIBC_MUTEX_LOCK(__resolv_lock);
	__close_nameservers();
	__open_nameservers();

	__res_sync = res_sync_func;

	memset(rp, 0, sizeof(*rp));
	rp->options = RES_INIT;
#ifdef __UCLIBC_HAS_COMPAT_RES_STATE__
	rp->retrans = RES_TIMEOUT;
	rp->retry = 4;
//TODO: pulls in largish static buffers... use simpler one?
	rp->id = random();
#endif
	rp->ndots = 1;
#ifdef __UCLIBC_HAS_EXTRA_COMPAT_RES_STATE__
	rp->_vcsock = -1;
#endif

	n = __searchdomains;
	if (n > ARRAY_SIZE(rp->dnsrch))
		n = ARRAY_SIZE(rp->dnsrch);
	for (i = 0; i < n; i++)
		rp->dnsrch[i] = __searchdomain[i];

	/* copy nameservers' addresses */
	i = 0;
#ifdef __UCLIBC_HAS_IPV4__
	n = 0;
	while (n < ARRAY_SIZE(rp->nsaddr_list) && i < __nameservers) {
		if (__nameserver[i].sa.sa_family == AF_INET) {
			rp->nsaddr_list[n] = __nameserver[i].sa4; /* struct copy */
#ifdef __UCLIBC_HAS_IPV6__
			if (m < ARRAY_SIZE(rp->_u._ext.nsaddrs)) {
				rp->_u._ext.nsaddrs[m] = (void*) &rp->nsaddr_list[n];
				m++;
			}
#endif
			n++;
		}
#ifdef __UCLIBC_HAS_IPV6__
		if (__nameserver[i].sa.sa_family == AF_INET6
		 && m < ARRAY_SIZE(rp->_u._ext.nsaddrs)
		) {
			struct sockaddr_in6 *sa6 = malloc(sizeof(sa6));
			if (sa6) {
				*sa6 = __nameserver[i].sa6; /* struct copy */
				rp->_u._ext.nsaddrs[m] = sa6;
				m++;
			}
		}
#endif
		i++;
	}
	rp->nscount = n;
#ifdef __UCLIBC_HAS_IPV6__
	rp->_u._ext.nscount = m;
#endif

#else /* IPv6 only */
	while (m < ARRAY_SIZE(rp->_u._ext.nsaddrs) && i < __nameservers) {
		struct sockaddr_in6 *sa6 = malloc(sizeof(sa6));
		if (sa6) {
			*sa6 = __nameserver[i].sa6; /* struct copy */
			rp->_u._ext.nsaddrs[m] = sa6;
			m++;
		}
		i++;
	}
	rp->_u._ext.nscount = m;
#endif

	__UCLIBC_MUTEX_UNLOCK(__resolv_lock);
	return 0;
}
libc_hidden_def(res_init)

#ifdef __UCLIBC_HAS_BSD_RES_CLOSE__
void res_close(void)
{
	__UCLIBC_MUTEX_LOCK(__resolv_lock);
	__close_nameservers();
	__res_sync = NULL;
#ifdef __UCLIBC_HAS_IPV6__
	{
		char *p1 = (char*) &(_res.nsaddr_list[0]);
		int m = 0;
		/* free nsaddrs[m] if they do not point to nsaddr_list[x] */
		while (m < ARRAY_SIZE(_res._u._ext.nsaddrs)) {
			char *p2 = (char*)(_res._u._ext.nsaddrs[m]);
			if (p2 < p1 || (p2 - p1) > sizeof(_res.nsaddr_list))
				free(p2);
		}
	}
#endif
	memset(&_res, 0, sizeof(_res));
	__UCLIBC_MUTEX_UNLOCK(__resolv_lock);
}
#endif
#endif /* L_res_init */


#ifdef L_res_query

int res_query(const char *dname, int class, int type,
              unsigned char *answer, int anslen)
{
	int i;
	unsigned char * packet = NULL;
	struct resolv_answer a;

	if (!dname || class != 1 /* CLASS_IN */) {
		h_errno = NO_RECOVERY;
		return -1;
	}

	memset(&a, '\0', sizeof(a));
	i = __dns_lookup(dname, type, &packet, &a);

	if (i < 0) {
		h_errno = TRY_AGAIN;
		return -1;
	}

	free(a.dotted);

	if (a.atype == type) { /* CNAME */
		if (i > anslen)
			i = anslen;
		memcpy(answer, packet, i);
	}
	free(packet);
	return i;
}
libc_hidden_def(res_query)

/*
 * Formulate a normal query, send, and retrieve answer in supplied buffer.
 * Return the size of the response on success, -1 on error.
 * If enabled, implement search rules until answer or unrecoverable failure
 * is detected.  Error code, if any, is left in h_errno.
 */
#define __TRAILING_DOT	(1<<0)
#define __GOT_NODATA	(1<<1)
#define __GOT_SERVFAIL	(1<<2)
#define __TRIED_AS_IS	(1<<3)
int res_search(const char *name, int class, int type, u_char *answer,
		int anslen)
{
	const char *cp, * const *domain;
	HEADER *hp = (HEADER *)(void *)answer;
	unsigned dots;
	unsigned state;
	int ret, saved_herrno;
	uint32_t _res_options;
	unsigned _res_ndots;
	char **_res_dnsrch;

	if (!name || !answer) {
		h_errno = NETDB_INTERNAL;
		return -1;
	}

 again:
	__UCLIBC_MUTEX_LOCK(__resolv_lock);
	_res_options = _res.options;
	_res_ndots = _res.ndots;
	_res_dnsrch = _res.dnsrch;
	__UCLIBC_MUTEX_UNLOCK(__resolv_lock);
	if (!(_res_options & RES_INIT)) {
		res_init(); /* our res_init never fails */
		goto again;
	}

	state = 0;
	errno = 0;
	h_errno = HOST_NOT_FOUND;	/* default, if we never query */
	dots = 0;
	for (cp = name; *cp; cp++)
		dots += (*cp == '.');

	if (cp > name && *--cp == '.')
		state |= __TRAILING_DOT;

	/*
	 * If there are dots in the name already, let's just give it a try
	 * 'as is'.  The threshold can be set with the "ndots" option.
	 */
	saved_herrno = -1;
	if (dots >= _res_ndots) {
		ret = res_querydomain(name, NULL, class, type, answer, anslen);
		if (ret > 0)
			return ret;
		saved_herrno = h_errno;
		state |= __TRIED_AS_IS;
	}

	/*
	 * We do at least one level of search if
	 *	- there is no dot and RES_DEFNAME is set, or
	 *	- there is at least one dot, there is no trailing dot,
	 *	  and RES_DNSRCH is set.
	 */
	if ((!dots && (_res_options & RES_DEFNAMES))
	 || (dots && !(state & __TRAILING_DOT) && (_res_options & RES_DNSRCH))
	) {
		bool done = 0;

		for (domain = (const char * const *)_res_dnsrch;
			*domain && !done;
			domain++) {

			ret = res_querydomain(name, *domain, class, type,
								  answer, anslen);
			if (ret > 0)
				return ret;

			/*
			 * If no server present, give up.
			 * If name isn't found in this domain,
			 * keep trying higher domains in the search list
			 * (if that's enabled).
			 * On a NO_DATA error, keep trying, otherwise
			 * a wildcard entry of another type could keep us
			 * from finding this entry higher in the domain.
			 * If we get some other error (negative answer or
			 * server failure), then stop searching up,
			 * but try the input name below in case it's
			 * fully-qualified.
			 */
			if (errno == ECONNREFUSED) {
				h_errno = TRY_AGAIN;
				return -1;
			}

			switch (h_errno) {
				case NO_DATA:
					state |= __GOT_NODATA;
					/* FALLTHROUGH */
				case HOST_NOT_FOUND:
					/* keep trying */
					break;
				case TRY_AGAIN:
					if (hp->rcode == SERVFAIL) {
						/* try next search element, if any */
						state |= __GOT_SERVFAIL;
						break;
					}
					/* FALLTHROUGH */
				default:
					/* anything else implies that we're done */
					done = 1;
			}
			/*
			 * if we got here for some reason other than DNSRCH,
			 * we only wanted one iteration of the loop, so stop.
			 */
			if (!(_res_options & RES_DNSRCH))
				done = 1;
		}
	}

	/*
	 * if we have not already tried the name "as is", do that now.
	 * note that we do this regardless of how many dots were in the
	 * name or whether it ends with a dot.
	 */
	if (!(state & __TRIED_AS_IS)) {
		ret = res_querydomain(name, NULL, class, type, answer, anslen);
		if (ret > 0)
			return ret;
	}

	/*
	 * if we got here, we didn't satisfy the search.
	 * if we did an initial full query, return that query's h_errno
	 * (note that we wouldn't be here if that query had succeeded).
	 * else if we ever got a nodata, send that back as the reason.
	 * else send back meaningless h_errno, that being the one from
	 * the last DNSRCH we did.
	 */
	if (saved_herrno != -1)
		h_errno = saved_herrno;
	else if (state & __GOT_NODATA)
		h_errno = NO_DATA;
	else if (state & __GOT_SERVFAIL)
		h_errno = TRY_AGAIN;
	return -1;
}
#undef __TRAILING_DOT
#undef __GOT_NODATA
#undef __GOT_SERVFAIL
#undef __TRIED_AS_IS
/*
 * Perform a call on res_query on the concatenation of name and domain,
 * removing a trailing dot from name if domain is NULL.
 */
int res_querydomain(const char *name, const char *domain, int class, int type,
			u_char * answer, int anslen)
{
	char nbuf[MAXDNAME];
	const char *longname = nbuf;
	size_t n, d;
#ifdef DEBUG
	uint32_t _res_options;
#endif

	if (!name || !answer) {
		h_errno = NETDB_INTERNAL;
		return -1;
	}

#ifdef DEBUG
 again:
	__UCLIBC_MUTEX_LOCK(__resolv_lock);
	_res_options = _res.options;
	__UCLIBC_MUTEX_UNLOCK(__resolv_lock);
	if (!(_res_options & RES_INIT)) {
		res_init(); /* our res_init never fails */
		goto again:
	}
	if (_res_options & RES_DEBUG)
		printf(";; res_querydomain(%s, %s, %d, %d)\n",
			   name, (domain ? domain : "<Nil>"), class, type);
#endif
	if (domain == NULL) {
		/*
		 * Check for trailing '.';
		 * copy without '.' if present.
		 */
		n = strlen(name);
		if (n + 1 > sizeof(nbuf)) {
			h_errno = NO_RECOVERY;
			return -1;
		}
		if (n > 0 && name[--n] == '.') {
			strncpy(nbuf, name, n);
			nbuf[n] = '\0';
		} else
			longname = name;
	} else {
		n = strlen(name);
		d = strlen(domain);
		if (n + 1 + d + 1 > sizeof(nbuf)) {
			h_errno = NO_RECOVERY;
			return -1;
		}
		snprintf(nbuf, sizeof(nbuf), "%s.%s", name, domain);
	}
	return res_query(longname, class, type, answer, anslen);
}
libc_hidden_def(res_querydomain)
/* res_mkquery */
/* res_send */
/* dn_comp */
/* dn_expand */
#endif

/* vi: set sw=4 ts=4: */
