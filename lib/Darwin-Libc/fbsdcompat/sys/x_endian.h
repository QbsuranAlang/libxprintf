/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/fbsdcompat/sys/endian.h
 *
 */


#ifndef LIBXPRINTF_ENDIAN_H
#define LIBXPRINTF_ENDIAN_H

#define X__LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define X__BIG_ENDIAN    __ORDER_BIG_ENDIAN__

#if defined(__BYTE_ORDER__)

#if __BYTE_ORDER__ == X__LITTLE_ENDIAN
#  define X__BYTE_ORDER X__LITTLE_ENDIAN
#elif __BYTE_ORDER__ == X__BIG_ENDIAN
#  define X__BYTE_ORDER X__BIG_ENDIAN
#endif

#else /* defined(__BYTE_ORDER__) */

#if defined(__LITTLE_ENDIAN__)
#  define X__BYTE_ORDER X__LITTLE_ENDIAN
#elif defined(__BIG_ENDIAN__)
#  define X__BYTE_ORDER X__BIG_ENDIAN
#endif

#endif /* !defined(__BYTE_ORDER__) */

#if !defined(X__BYTE_ORDER)
#error "Byte endianness is undetected"
#endif /* !!defined(X__BYTE_ORDER) */

#endif /* !LIBXPRINTF_ENDIAN_H */
