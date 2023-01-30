/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/fbsdcompat/fpmath.h
 *
 */


#ifndef LIBXPRINTF_FPMATH_H
#define LIBXPRINTF_FPMATH_H

#include <sys/x_endian.h>

#include "x__fpmath.h"

#define X_DBL_MANL_SIZE 32

union x_IEEEd2bits {
    double              d;
    struct {
#if X__BYTE_ORDER == X__LITTLE_ENDIAN
        unsigned int    manl:32;
        unsigned int    manh:20;
        unsigned int    exp:11;
        unsigned int    sign:1;
#else /* X__BYTE_ORDER == X__LITTLE_ENDIAN */
        unsigned int    sign:1;
        unsigned int    exp:11;
        unsigned int    manh:20;
        unsigned int    manl:32;
#endif /* !(X__BYTE_ORDER == X__LITTLE_ENDIAN) */
    } bits;
};

#endif /* !LIBXPRINTF_FPMATH_H */
