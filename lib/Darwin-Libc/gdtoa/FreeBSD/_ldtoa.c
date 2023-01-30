/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/FreeBSD/_ldtoa.c
 *
 */


#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>

#include "x_fpmath.h"
#include "x_gdtoaimp.h"
#include "x_libc_private.h"
#include "x_math.h"

/*
 * x_ldtoa() is a wrapper for x_gdtoa() that makes it smell like x_dtoa(),
 * except that the floating point argument is passed by reference.
 * When x_dtoa() is passed a NaN or infinity, it sets expt to 9999.
 * However, a long double could have a valid exponent of 9999, so we
 * use INT_MAX in x_ldtoa() instead.
 */
char *x___ldtoa(long double *ld, int mode, int ndigits, int *decpt, int *sign, char **rve) {
#if defined(__arm__) || defined(__arm64__)
    /* On arm, double == long double, so short circuit this */
    char *ret;

    ret = x___dtoa((double)*ld, mode, ndigits, decpt, sign, rve);
    if(*decpt == 9999) {
        *decpt = INT_MAX;
    }//end if
    return ret;
#else /* defined(__arm__) || defined(__arm64__) */
    FPI fpi = {
        LDBL_MANT_DIG,                  /* nbits */
        LDBL_MIN_EXP - LDBL_MANT_DIG,   /* emin */
        LDBL_MAX_EXP - LDBL_MANT_DIG,   /* emax */
        FLT_ROUNDS,                     /* rounding */
#if defined(Sudden_Underflow)    /* unused, but correct anyway */
        1
#else /* defined(Sudden_Underflow) */
        0
#endif /* !defined(Sudden_Underflow) */
    };
    int                 be, kind, type;
    char                *ret;
    void                *vbits;
    uint32_t            bits[(LDBL_MANT_DIG + 31) / 32];
    union x_IEEEl2bits  u;

    vbits = bits;
    u.e = *ld;
    type = fpclassify(u.e);

    /*
     * x_gdtoa doesn't know anything about the sign of the number, so
     * if the number is negative, we need to swap rounding modes of
     * 2 (upwards) and 3 (downwards).
     */
    *sign = u.bits.sign;
    fpi.rounding ^= (fpi.rounding >> 1) & u.bits.sign;

    be = u.bits.exp - (LDBL_MAX_EXP - 1) - (LDBL_MANT_DIG - 1);
    X_LDBL_TO_ARRAY32(u, bits);

    switch (type) {
    case FP_NORMAL:
    case FP_SUPERNORMAL:
        kind = x_STRTOG_Normal;
#if defined(LDBL_IMPLICIT_NBIT)
        bits[LDBL_MANT_DIG / 32] |= 1 << ((LDBL_MANT_DIG - 1) % 32);
#endif /* !defined(LDBL_IMPLICIT_NBIT) */
        break;
    case FP_ZERO:
        kind = x_STRTOG_Zero;
        break;
    case FP_SUBNORMAL:
        kind = x_STRTOG_Denormal;
        be++;
        break;
    case FP_INFINITE:
        kind = x_STRTOG_Infinite;
        break;
    case FP_NAN:
        kind = x_STRTOG_NaN;
        break;
    default:
        X_LIBC_ABORT("fpclassify returned %d", type);
    }//end switch

    ret = x_gdtoa(&fpi, be, vbits, &kind, mode, ndigits, decpt, rve);
    if(*decpt == -32768) {
        *decpt = INT_MAX;
    }//end if
    return ret;
#endif /* !(defined(__arm__) || defined(__arm64__)) */
}//end x___ldtoa
