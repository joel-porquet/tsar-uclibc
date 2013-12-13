#ifndef _TSAR_BITS_ENDIAN_H
#define _TSAR_BITS_ENDIAN_H

#ifndef _ENDIAN_H
# error "Never use <bits/endian.h> directly: include <endian.h> instead."
#endif

#ifndef __MIPSEL__
# error "Only support little endian"
#endif

#define __BYTE_ORDER __LITTLE_ENDIAN

#endif /* _TSAR_BITS_ENDIAN_H */
