/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/gdtoa.tgz
 *
 */


#ifndef LIBXPRINTF_GDTOA_H
#define LIBXPRINTF_GDTOA_H

#include <sys/cdefs.h>

#include <libxprintf/libxprintf_visibility.h>

#include "x_arith.h"

#if !defined(Long)
#define Long long
#endif /* !!defined(Long) */

#if !defined(ULong)
typedef unsigned Long ULong;
#endif /* !!defined(ULong) */

#if !defined(UShort)
typedef unsigned short UShort;
#endif /* !!defined(UShort) */

enum {    /* return values from strtodg */
    x_STRTOG_Zero       = 0,
    x_STRTOG_Normal     = 1,
    x_STRTOG_Denormal   = 2,
    x_STRTOG_Infinite   = 3,
    x_STRTOG_NaN        = 4,
    x_STRTOG_NaNbits    = 5,
    x_STRTOG_NoNumber   = 6,
    x_STRTOG_Retmask    = 7,

    /* The following may be or-ed into one of the above values. */

    x_STRTOG_Neg        = 0x08, /* does not affect x_STRTOG_Inexlo or x_STRTOG_Inexhi */
    x_STRTOG_Inexlo     = 0x10,    /* returned result rounded toward zero */
    x_STRTOG_Inexhi     = 0x20, /* returned result rounded away from zero */
    x_STRTOG_Inexact    = 0x30,
    x_STRTOG_Underflow  = 0x40,
    x_STRTOG_Overflow   = 0x80
};

typedef struct FPI {
    int nbits;
    int emin;
    int emax;
    int rounding;
    int sudden_underflow;
} FPI;

__BEGIN_DECLS

X_LOCAL char *x_dtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve);
X_LOCAL char *x_gdtoa(FPI *fpi, int be, ULong *bits, int *kindp, int mode, int ndigits, int *decpt, char **rve);
X_LOCAL void x_freedtoa(char *);

__END_DECLS

#endif /* !LIBXPRINTF_GDTOA_H */
