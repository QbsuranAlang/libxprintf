/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/gdtoa.tgz
 *
 */


/* This is a variation on dtoa.c that converts arbitary binary
    floating-point formats to and from decimal notation.  It uses
    double-precision arithmetic internally, so there are still
    various #ifdefs that adapt the calculations to the native
    double-precision arithmetic (any of IEEE, VAX D_floating,
    or IBM mainframe arithmetic).

    Please send bug reports to David M. Gay (dmg at acm dot org,
    with " at " changed at "@" and " dot " changed to ".").
 */

/* On a machine with IEEE extended-precision registers, it is
 * necessary to specify double-precision (53-bit) rounding precision
 * before invoking strtod or dtoa.  If the machine uses (the equivalent
 * of) Intel 80x87 arithmetic, the call
 *    _control87(PC_53, MCW_PC);
 * does this with many compilers.  Whether this or another call is
 * appropriate depends on the compiler; for this to work, it may be
 * necessary to #include "float.h" or another system-dependent header
 * file.
 */

/* strtod for IEEE-, VAX-, and IBM-arithmetic machines.
 *
 * This strtod returns a nearest machine number to the input decimal
 * string (or sets errno to ERANGE).  With IEEE arithmetic, ties are
 * broken by the IEEE round-even rule.  Otherwise ties are broken by
 * biased rounding (add half and chop).
 *
 * Inspired loosely by William D. Clinger's paper "How to Read Floating
 * Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 112-126].
 *
 * Modifications:
 *
 *    1. We only require IEEE, IBM, or VAX double-precision
 *        arithmetic (not IEEE double-extended).
 *    2. We get by with floating-point arithmetic in a case that
 *        Clinger missed -- when we're computing d * 10^n
 *        for a small integer d and the integer n is not too
 *        much larger than 22 (the maximum integer k for which
 *        we can represent 10^k exactly), we may be able to
 *        compute (d*10^k) * 10^(e-k) with just one roundoff.
 *    3. Rather than a bit-at-a-time adjustment of the binary
 *        result in the hard case, we use floating-point
 *        arithmetic to determine the adjustment to within
 *        one bit; only in really hard cases do we need to
 *        compute a second residual.
 *    4. Because of 3., we don't need a large table of powers of 10
 *        for ten-to-e (just some small tables, e.g. of 10^k
 *        for 0 <= k <= 22).
 */

/*
 * #define IEEE_8087 for IEEE-arithmetic machines where the least
 *    significant byte has the lowest address.
 * #define IEEE_MC68k for IEEE-arithmetic machines where the most
 *    significant byte has the lowest address.
 * #define Long int on machines with 32-bit ints and 64-bit longs.
 * #define Sudden_Underflow for IEEE-format machines without gradual
 *    underflow (i.e., that flush to zero on underflow).
 * #define IBM for IBM mainframe-style floating-point arithmetic.
 * #define VAX for VAX-style floating-point arithmetic (D_floating).
 * #define No_leftright to omit left-right logic in fast floating-point
 *    computation of dtoa.
 * #define Check_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3.
 * #define RND_PRODQUOT to use rnd_prod and rnd_quot (assembly routines
 *    that use extended-precision instructions to compute rounded
 *    products and quotients) with IBM.
 * #define ROUND_BIASED for IEEE-format with biased rounding.
 * #define Inaccurate_Divide for IEEE-format with correctly rounded
 *    products but inaccurate quotients, e.g., for Intel i860.
 * #define NO_LONG_LONG on machines that do not have a "long long"
 *    integer type (of >= 64 bits).  On such machines, you can
 *    #define Just_16 to store 16 bits per 32-bit Long when doing
 *    high-precision integer arithmetic.  Whether this speeds things
 *    up or slows things down depends on the machine and the number
 *    being converted.  If long long is available and the name is
 *    something other than "long long", #define Llong to be the name,
 *    and if "unsigned Llong" does not work as an unsigned version of
 *    Llong, #define #ULLong to be the corresponding unsigned type.
 * #define KR_headers for old-style C function headers.
 * #define Bad_float_h if your system lacks a float.h or if it does not
 *    define some or all of DBL_DIG, DBL_MAX_10_EXP, DBL_MAX_EXP,
 *    FLT_RADIX, FLT_ROUNDS, and DBL_MAX.
 * #define MALLOC your_malloc, where your_malloc(n) acts like malloc(n)
 *    if memory is available and otherwise does something you deem
 *    appropriate.  If MALLOC is undefined, malloc will be invoked
 *    directly -- and assumed always to succeed.  Similarly, if you
 *    want something other than the system's free() to be called to
 *    recycle memory acquired from MALLOC, #define FREE to be the
 *    name of the alternate routine.  (FREE or free is only called in
 *    pathological cases, e.g., in a gdtoa call after a gdtoa return in
 *    mode 3 with thousands of digits requested.)
 * #define Omit_Private_Memory to omit logic (added Jan. 1998) for making
 *    memory allocations from a private pool of memory when possible.
 *    When used, the private pool is PRIVATE_MEM bytes long:  2304 bytes,
 *    unless #defined to be a different length.  This default length
 *    suffices to get rid of MALLOC calls except for unusual cases,
 *    such as decimal-to-binary conversion of a very long string of
 *    digits.  When converting IEEE double precision values, the
 *    longest string gdtoa can return is about 751 bytes long.  For
 *    conversions by strtod of strings of 800 digits and all gdtoa
 *    conversions of IEEE doubles in single-threaded executions with
 *    8-byte pointers, PRIVATE_MEM >= 7400 appears to suffice; with
 *    4-byte pointers, PRIVATE_MEM >= 7112 appears adequate.
 * #define NO_INFNAN_CHECK if you do not wish to have INFNAN_CHECK
 *    #defined automatically on IEEE systems.  On such systems,
 *    when INFNAN_CHECK is #defined, strtod checks
 *    for Infinity and NaN (case insensitively).
 *    When INFNAN_CHECK is #defined and No_Hex_NaN is not #defined,
 *    strtodg also accepts (case insensitively) strings of the form
 *    NaN(x), where x is a string of hexadecimal digits (optionally
 *    preceded by 0x or 0X) and spaces; if there is only one string
 *    of hexadecimal digits, it is taken for the fraction bits of the
 *    resulting NaN; if there are two or more strings of hexadecimal
 *    digits, each string is assigned to the next available sequence
 *    of 32-bit words of fractions bits (starting with the most
 *    significant), right-aligned in each sequence.
 *    Unless GDTOA_NON_PEDANTIC_NANCHECK is #defined, input "NaN(...)"
 *    is consumed even when ... has the wrong form (in which case the
 *    "(...)" is consumed but ignored).
 * #define MULTIPLE_THREADS if the system offers preemptively scheduled
 *    multiple threads.  In this case, you must provide (or suitably
 *    #define) two locks, acquired by X_ACQUIRE_DTOA_LOCK(n) and freed
 *    by X_FREE_DTOA_LOCK(n) for n = 0 or 1.  (The second lock, accessed
 *    in x_pow5mult, ensures lazy evaluation of only one copy of high
 *    powers of 5; omitting this lock would introduce a small
 *    probability of wasting memory, but would otherwise be harmless.)
 *    You must also invoke x_freedtoa(s) to free the value s returned by
 *    x_dtoa.  You may do so whether or not MULTIPLE_THREADS is #defined.
 * #define IMPRECISE_INEXACT if you do not care about the setting of
 *    the x_STRTOG_Inexact bits in the special case of doing IEEE double
 *    precision conversions (which could also be done by the strtod in
 *    dtoa.c).
 * #define NO_HEX_FP to disable recognition of C9x's hexadecimal
 *    floating-point constants.
 * #define -DNO_ERRNO to suppress setting errno (in strtod.c and
 *    strtodg.c).
 * #define NO_STRING_H to use private versions of memcpy.
 *    On some K&R systems, it may also be necessary to
 *    #define DECLARE_SIZE_T in this case.
 * #define USE_LOCALE to use the current locale's decimal_point value.
 */

#ifndef LIBXPRINTF_DGTOAIMP_H
#define LIBXPRINTF_DGTOAIMP_H

#include "x_gdtoa.h"
#include "x_gd_qnan.h"

#if defined(Honor_FLT_ROUNDS)
#include <fenv.h>
#endif /* !defined(Honor_FLT_ROUNDS) */

#include <sys/cdefs.h>
#include <stdlib.h>
#include <string.h>

#undef IEEE_Arith
#undef Avoid_Underflow
#if defined(IEEE_MC68k)
#define IEEE_Arith
#endif /* !defined(IEEE_MC68k) */
#if defined(IEEE_8087)
#define IEEE_Arith
#endif /* !defined(IEEE_8087) */

#include <errno.h>

#if defined(Bad_float_h)

#if defined(IEEE_Arith)
#define DBL_DIG         15
#define DBL_MAX_10_EXP  308
#define DBL_MAX_EXP     1024
#define FLT_RADIX       2
#define DBL_MAX         1.7976931348623157e+308
#endif /* !defined(IEEE_Arith) */

#if defined(IBM)
#define DBL_DIG         16
#define DBL_MAX_10_EXP  75
#define DBL_MAX_EXP     63
#define FLT_RADIX       16
#define DBL_MAX         7.2370055773322621e+75
#endif /* !defined(IBM) */

#if defined(VAX)
#define DBL_DIG         16
#define DBL_MAX_10_EXP  38
#define DBL_MAX_EXP     127
#define FLT_RADIX       2
#define DBL_MAX         1.7014118346046923e+38
#define x_n_bigtens     2
#endif /* !defined(VAX) */

#if !defined(LONG_MAX)
#define LONG_MAX 2147483647
#endif /* !!defined(LONG_MAX) */

#else /* defined(Bad_float_h) */
#include <float.h>
#endif /* !defined(Bad_float_h) */

#if defined(IEEE_Arith)
#define x_n_bigtens 5
#endif /* !defined(IEEE_Arith) */

#if defined(IBM)
#define x_n_bigtens 3
#endif /* !defined(IBM) */

#if defined(VAX)
#define x_n_bigtens 2
#endif /* !defined(VAX) */

#if !defined(__MATH_H__)
#include <math.h>
#endif /* !!defined(__MATH_H__) */

__BEGIN_DECLS

#if defined(IEEE_8087) + defined(IEEE_MC68k) + defined(VAX) + defined(IBM) != 1
#error "Exactly one of IEEE_8087, IEEE_MC68k, VAX, or IBM should be defined."
#endif /* !(defined(IEEE_8087) + defined(IEEE_MC68k) + defined(VAX) + defined(IBM) != 1) */

typedef union {
    double  d;
    ULong   L[2];
} U;

#if defined(IEEE_8087)
#define x_word0(x) (x)->L[1]
#define x_word1(x) (x)->L[0]
#else /* defined(IEEE_8087) */
#define x_word0(x) (x)->L[0]
#define x_word1(x) (x)->L[1]
#endif /* !defined(IEEE_8087) */
#define x_dval(x) (x)->d

/* The following definition of x_Storeinc is appropriate for MIPS processors.
 * An alternative that might be better on some machines is
 * #define x_Storeinc(a,b,c) (*a++ = b << 16 | c & 0xffff)
 */
#if defined(IEEE_8087) + defined(VAX)
#define x_Storeinc(a, b, c) (((unsigned short *)a)[1] = (unsigned short)b, \
    ((unsigned short *)a)[0] = (unsigned short)c, a++)
#else /* defined(IEEE_8087) + defined(VAX) */
#define x_Storeinc(a, b, c) (((unsigned short *)a)[0] = (unsigned short)b, \
    ((unsigned short *)a)[1] = (unsigned short)c, a++)
#endif /* !(defined(IEEE_8087) + defined(VAX)) */

/* #define P DBL_MANT_DIG */
/* Ten_pmax = floor(P*log(2)/log(5)) */
/* Bletch = (highest power of 2 < DBL_MAX_10_EXP) / 16 */
/* Quick_max = floor((P-1)*log(FLT_RADIX)/log(10) - 1) */
/* Int_max = floor(P*log(FLT_RADIX)/log(10) - 1) */

#if defined(IEEE_Arith)
#define Exp_shift   20
#define Exp_shift1  20
#define Exp_msk1    0x100000
#define Exp_msk11   0x100000
#define Exp_mask    0x7ff00000
#define P           53
#define Bias        1023
#define Emin        (-1022)
#define Exp_1       0x3ff00000
#define Exp_11      0x3ff00000
#define Ebits       11
#define Frac_mask   0xfffff
#define Frac_mask1  0xfffff
#define Ten_pmax    22
#define Bletch      0x10
#define Bndry_mask  0xfffff
#define Bndry_mask1 0xfffff
#define LSB         1
#define Sign_bit    0x80000000
#define Log2P       1
#define Tiny0       0
#define Tiny1       1
#define Quick_max   14
#define Int_max     14

#if !defined(Flt_Rounds)
#if defined(FLT_ROUNDS)
#define Flt_Rounds FLT_ROUNDS
#else /* defined(FLT_ROUNDS) */
#define Flt_Rounds 1
#endif /* !defined(FLT_ROUNDS) */
#endif /* !!defined(Flt_Rounds) */

#else /* defined(IEEE_Arith) */
#undef  Sudden_Underflow
#define Sudden_Underflow
#if defined(IBM)
#undef Flt_Rounds
#define Flt_Rounds  0
#define Exp_shift   24
#define Exp_shift1  24
#define Exp_msk1    0x1000000
#define Exp_msk11   0x1000000
#define Exp_mask    0x7f000000
#define P           14
#define Bias        65
#define Exp_1       0x41000000
#define Exp_11      0x41000000
#define Ebits       8    /* exponent has 7 bits, but 8 is the right value in b2d */
#define Frac_mask   0xffffff
#define Frac_mask1  0xffffff
#define Bletch      4
#define Ten_pmax    22
#define Bndry_mask  0xefffff
#define Bndry_mask1 0xffffff
#define LSB         1
#define Sign_bit    0x80000000
#define Log2P       4
#define Tiny0       0x100000
#define Tiny1       0
#define Quick_max   14
#define Int_max     15
#else /* defined(IBM) */
#undef Flt_Rounds
#define Flt_Rounds  1
#define Exp_shift   23
#define Exp_shift1  7
#define Exp_msk1    0x80
#define Exp_msk11   0x800000
#define Exp_mask    0x7f80
#define P           56
#define Bias        129
#define Exp_1       0x40800000
#define Exp_11      0x4080
#define Ebits       8
#define Frac_mask   0x7fffff
#define Frac_mask1  0xffff007f
#define Ten_pmax    24
#define Bletch      2
#define Bndry_mask  0xffff007f
#define Bndry_mask1 0xffff007f
#define LSB         0x10000
#define Sign_bit    0x8000
#define Log2P       1
#define Tiny0       0x80
#define Tiny1       0
#define Quick_max   15
#define Int_max     15
#endif /* !defined(IBM) */
#endif /* !defined(IEEE_Arith) */

#if !defined(IEEE_Arith)
#define ROUND_BIASED
#endif /* !!defined(IEEE_Arith) */

#undef  Pack_16
#if !defined(Pack_32)
#define Pack_32
#endif /* !!defined(Pack_32) */

#if defined(NO_LONG_LONG)
#undef ULLong
#if defined(Just_16)
#undef Pack_32
#define Pack_16
/* When Pack_32 is not defined, we store 16 bits per 32-bit Long.
 * This makes some inner loops simpler and sometimes saves work
 * during multiplications, but it often seems to make things slightly
 * slower.  Hence the default is now to store 32 bits per Long.
 */
#endif /* !defined(Just_16) */
#else /* defined(NO_LONG_LONG) */
#if !defined(Llong)
#define Llong long long
#endif /* !!defined(Llong) */
#if !defined(ULLong)
#define ULLong unsigned Llong
#endif /* !defined(ULLong) */
#endif /* !defined(NO_LONG_LONG) */

#if defined(Pack_32)
#define ULbits  32
#define kshift  5
#define kmask   31
#define ALL_ON  0xffffffff
#else /* defined(Pack_32) */
#define ULbits  16
#define kshift  4
#define kmask   15
#define ALL_ON  0xffff
#endif /* !defined(Pack_32) */

#if !defined(MULTIPLE_THREADS)
#define X_ACQUIRE_DTOA_LOCK(n)  /* nothing */
#define X_FREE_DTOA_LOCK(n)     /* nothing */
#endif /* !!defined(MULTIPLE_THREADS) */

#define X_Kmax 9

struct x_Bigint {
    struct x_Bigint     *next;
    int                 k, maxwds, sign, wds;
    ULong               x[1];
};

typedef struct x_Bigint x_Bigint;

#if defined(NO_STRING_H)
#if defined(DECLARE_SIZE_T)
typedef unsigned int size_t;
#endif /* !defined(DECLARE_SIZE_T) */
X_LOCAL void *x_memcpy_D2A(void *, const void *, size_t);
#define x_Bcopy(x, y)   x_memcpy_D2A(&(x)->sign, &(y)->sign, (y)->wds * sizeof(ULong) + 2 * sizeof(int))
#else /* defined(NO_STRING_H) */
#define x_Bcopy(x, y)   memcpy(&(x)->sign, &(y)->sign, (y)->wds * sizeof(ULong) + 2 * sizeof(int))
#endif /* !defined(NO_STRING_H) */

#define x_hi0bits(x)    x_hi0bits_D2A((ULong)(x))

extern char *x_dtoa_result;
extern const double x_bigtens[], x_tens[];

X_LOCAL x_Bigint *x_Balloc(int);
X_LOCAL void x_Bfree(x_Bigint *);
X_LOCAL double x_b2d(x_Bigint *, int *);
X_LOCAL int x_cmp(x_Bigint *, x_Bigint *);
X_LOCAL x_Bigint *x_d2b(double, int *, int *);
X_LOCAL x_Bigint *x_diff(x_Bigint *, x_Bigint *);
X_LOCAL char *x_dtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve);
X_LOCAL int x_hi0bits_D2A(ULong);
X_LOCAL x_Bigint *x_i2b(int);
X_LOCAL int x_lo0bits(ULong *);
X_LOCAL x_Bigint *x_lshift(x_Bigint *, int);
X_LOCAL x_Bigint *x_mult(x_Bigint *, x_Bigint *);
X_LOCAL x_Bigint *x_multadd(x_Bigint *, int, int);
X_LOCAL char *x_nrv_alloc(char *, char **, int);
X_LOCAL x_Bigint *x_pow5mult(x_Bigint *, int);
X_LOCAL int x_quorem(x_Bigint *, x_Bigint *);
X_LOCAL void x_rshift(x_Bigint *, int);
X_LOCAL char *x_rv_alloc(int);
X_LOCAL int x_trailz(x_Bigint *);

__END_DECLS

#endif /* !LIBXPRINTF_DGTOAIMP_H */
