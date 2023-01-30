/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/FreeBSD/_hdtoa.c
 *
 */


#include <float.h>
#include <limits.h>
#include <math.h>

#include "x_fpmath.h"
#include "x_gdtoaimp.h"
#include "x_libc_private.h"
#include "x_math.h"

/* Strings values used by dtoa() */
#define X_INFSTR "Infinity"
#define X_NANSTR "NaN"

#define X_DBL_ADJ   (DBL_MAX_EXP - 2 + ((DBL_MANT_DIG - 1) % 4))
#define X_LDBL_ADJ  (LDBL_MAX_EXP - 2 + ((LDBL_MANT_DIG - 1) % 4))

/*
 * Round up the given digit string.  If the digit string is fff...f,
 * this procedure sets it to 100...0 and returns 1 to indicate that
 * the exponent needs to be bumped.  Otherwise, 0 is returned.
 */
static int x_roundup(char *s0, int ndigits) {
    char *s;

    for(s = s0 + ndigits - 1; *s == 0xf; s--) {
        if(s == s0) {
            *s = 1;
            return 1;
        }//end if
        *s = 0;
    }//end for
    ++*s;
    return 0;
}//end x_roundup

/*
 * Round the given digit string to ndigits digits according to the
 * current rounding mode.  Note that this could produce a string whose
 * value is not representable in the corresponding floating-point
 * type.  The exponent pointed to by decpt is adjusted if necessary.
 */
static void x_dorounding(char *s0, int ndigits, int sign, int *decpt) {
    int adjust;    /* do we need to adjust the exponent? */

    adjust = 0;
    switch(FLT_ROUNDS) {
    case 0:        /* toward zero */
    default:    /* implementation-defined */
        break;
    case 1:        /* to nearest, halfway rounds to even */
        if((s0[ndigits] > 8) || (s0[ndigits] == 8 && s0[ndigits + 1] & 1)) {
            adjust = x_roundup(s0, ndigits);
        }//end if
        break;
    case 2:        /* toward +inf */
        if(sign == 0) {
            adjust = x_roundup(s0, ndigits);
        }//end if
        break;
    case 3:        /* toward -inf */
        if(sign != 0) {
            adjust = x_roundup(s0, ndigits);
        }//end if
        break;
    }//end switch

    if(adjust) {
        *decpt += 4;
    }//end if
}//end x_dorounding

/*
 * This procedure converts a double-precision number in IEEE format
 * into a string of hexadecimal digits and an exponent of 2.  Its
 * behavior is bug-for-bug compatible with dtoa() in mode 2, with the
 * following exceptions:
 *
 * - An ndigits < 0 causes it to use as many digits as necessary to
 *   represent the number exactly.
 * - The additional xdigs argument should point to either the string
 *   "0123456789ABCDEF" or the string "0123456789abcdef", depending on
 *   which case is desired.
 * - This routine does not repeat dtoa's mistake of setting decpt
 *   to 9999 in the case of an infinity or NaN.  INT_MAX is used
 *   for this purpose instead.
 *
 * Note that the C99 standard does not specify what the leading digit
 * should be for non-zero numbers.  For instance, 0x1.3p3 is the same
 * as 0x2.6p2 is the same as 0x4.cp3.  This implementation chooses the
 * first digit so that subsequent digits are aligned on nibble
 * boundaries (before rounding).
 *
 * Inputs:    d, xdigs, ndigits
 * Outputs:    decpt, sign, rve
 */
char *x___hdtoa(double d, const char *xdigs, int ndigits, int *decpt, int *sign, char **rve) {
    int                 bufsize, f;
    char                *s, *s0;
    static const int    sigfigs = (DBL_MANT_DIG + 3) / 4;
    union x_IEEEd2bits  u;

    u.d = d;
    *sign = u.bits.sign;

    switch(f = fpclassify(d)) {
    case FP_NORMAL:
        *decpt = u.bits.exp - X_DBL_ADJ;
        break;
    case FP_ZERO:
return_zero:
        *decpt = 1;
        return x_nrv_alloc("0", rve, 1);
    case FP_SUBNORMAL:
        /*
         * For processors that treat subnormals as zero, comparison
         * with zero will be equal, so we jump to the FP_ZERO case.
         */
        if(u.d == 0.0) {
            goto return_zero;
        }//end if
        u.d *= 0x1p514;
        *decpt = u.bits.exp - (514 + X_DBL_ADJ);
        break;
    case FP_INFINITE:
        *decpt = INT_MAX;
        return x_nrv_alloc(X_INFSTR, rve, sizeof(X_INFSTR) - 1);
    case FP_NAN:
        *decpt = INT_MAX;
        return x_nrv_alloc(X_NANSTR, rve, sizeof(X_NANSTR) - 1);
    default:
        X_LIBC_ABORT("fpclassify returned %d", f);
    }//end switch

    /* FP_NORMAL or FP_SUBNORMAL */

    if(ndigits == 0) {       /* x_dtoa() compatibility */
        ndigits = 1;
    }//end if

    /*
     * For simplicity, we generate all the digits even if the
     * caller has requested fewer.
     */
    bufsize = (sigfigs > ndigits) ? sigfigs : ndigits;
    s0 = x_rv_alloc(bufsize);

    /*
     * We work from right to left, first adding any requested zero
     * padding, then the least significant portion of the
     * mantissa, followed by the most significant.  The buffer is
     * filled with the byte values 0x0 through 0xf, which are
     * converted to xdigs[0x0] through xdigs[0xf] after the
     * rounding phase.
     */
    for(s = s0 + bufsize - 1; s > s0 + sigfigs - 1; s--) {
        *s = 0;
    }//end for
    for(; s > s0 + sigfigs - (X_DBL_MANL_SIZE / 4) - 1 && s > s0; s--) {
        *s = u.bits.manl & 0xf;
        u.bits.manl >>= 4;
    }//end for
    for(; s > s0; s--) {
        *s = u.bits.manh & 0xf;
        u.bits.manh >>= 4;
    }//end for

    /*
     * At this point, we have snarfed all the bits in the
     * mantissa, with the possible exception of the highest-order
     * (partial) nibble, which is dealt with by the next
     * statement.  We also tack on the implicit normalization bit.
     */
    *s = u.bits.manh | (1U << ((DBL_MANT_DIG - 1) % 4));

    /* If ndigits < 0, we are expected to auto-size the precision. */
    if(ndigits < 0) {
        for(ndigits = sigfigs; s0[ndigits - 1] == 0; ndigits--) {
        }//end for
    }//end if

    if(sigfigs > ndigits && s0[ndigits] != 0) {
        x_dorounding(s0, ndigits, u.bits.sign, decpt);
    }//end if

    s = s0 + ndigits;
    if(rve != NULL) {
        *rve = s;
    }//end if
    *s-- = '\0';
    for(; s >= s0; s--) {
        *s = xdigs[(unsigned int)*s];
    }//end for

    return s0;
}//end x___hdtoa

#if (LDBL_MANT_DIG > DBL_MANT_DIG)

/*
 * This is the long double version of x___hdtoa().
 */
char *x___hldtoa(long double e, const char *xdigs, int ndigits, int *decpt, int *sign, char **rve) {
    int                 bufsize, f;
    char                *s, *s0;
    static const int    sigfigs = (LDBL_MANT_DIG + 3) / 4;
    union x_IEEEl2bits  u;

    u.e = e;
    *sign = u.bits.sign;

    switch(f = fpclassify(e)) {
    case FP_NORMAL:
    case FP_SUPERNORMAL:
        *decpt = u.bits.exp - X_LDBL_ADJ;
        break;
    case FP_ZERO:
        *decpt = 1;
        return x_nrv_alloc("0", rve, 1);
    case FP_SUBNORMAL:
        u.e *= 0x1p514L;
        *decpt = u.bits.exp - (514 + X_LDBL_ADJ);
        break;
    case FP_INFINITE:
        *decpt = INT_MAX;
        return x_nrv_alloc(X_INFSTR, rve, sizeof(X_INFSTR) - 1);
    case FP_NAN:
        *decpt = INT_MAX;
        return x_nrv_alloc(X_NANSTR, rve, sizeof(X_NANSTR) - 1);
    default:
        X_LIBC_ABORT("fpclassify returned %d", f);
    }//end switch

    /* FP_NORMAL or FP_SUBNORMAL */

    if(ndigits == 0) {       /* dtoa() compatibility */
        ndigits = 1;
    }//end if

    /*
     * For simplicity, we generate all the digits even if the
     * caller has requested fewer.
     */
    bufsize = (sigfigs > ndigits) ? sigfigs : ndigits;
    s0 = x_rv_alloc(bufsize);

    /*
     * We work from right to left, first adding any requested zero
     * padding, then the least significant portion of the
     * mantissa, followed by the most significant.  The buffer is
     * filled with the byte values 0x0 through 0xf, which are
     * converted to xdigs[0x0] through xdigs[0xf] after the
     * rounding phase.
     */
    for(s = s0 + bufsize - 1; s > s0 + sigfigs - 1; s--) {
        *s = 0;
    }//end for
    for(; s > s0 + sigfigs - (X_LDBL_MANL_SIZE / 4) - 1 && s > s0; s--) {
        *s = u.bits.manl & 0xf;
        u.bits.manl >>= 4;
    }//end for
    for(; s > s0; s--) {
        *s = u.bits.manh & 0xf;
        u.bits.manh >>= 4;
    }//end for

    /*
     * At this point, we have snarfed all the bits in the
     * mantissa, with the possible exception of the highest-order
     * (partial) nibble, which is dealt with by the next
     * statement.  We also tack on the implicit normalization bit.
     */
    *s = u.bits.manh | (1U << ((LDBL_MANT_DIG - 1) % 4));

    /* If ndigits < 0, we are expected to auto-size the precision. */
    if(ndigits < 0) {
        for(ndigits = sigfigs; s0[ndigits - 1] == 0; ndigits--) {
        }//end for
    }//end if

    if(sigfigs > ndigits && s0[ndigits] != 0) {
        x_dorounding(s0, ndigits, u.bits.sign, decpt);
    }//end if

    s = s0 + ndigits;
    if(rve != NULL) {
        *rve = s;
    }//end if
    *s-- = '\0';
    for(; s >= s0; s--) {
        *s = xdigs[(unsigned int)*s];
    }//end for

    return s0;
}//end x___hldtoa

#else /* (LDBL_MANT_DIG > DBL_MANT_DIG) */

char *x___hldtoa(long double e, const char *xdigs, int ndigits, int *decpt, int *sign, char **rve) {
    return x___hdtoa((double)e, xdigs, ndigits, decpt, sign, rve);
}//end x___hldtoa

#endif /* !(LDBL_MANT_DIG > DBL_MANT_DIG) */
