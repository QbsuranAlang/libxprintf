/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/printflocal.h
 *
 */


#ifndef LIBXPRINTF_PRINTFLOCAL_H
#define LIBXPRINTF_PRINTFLOCAL_H

#include <stdint.h>
#include <sys/types.h>

#if !defined(X_VECTORS)
#define X_VECTORS
typedef __attribute__((vector_size(16))) unsigned char X_VECTORTYPE;
#if defined(__SSE2__)
#define X_V64TYPE
#endif /* defined(__SSE2__) */
#endif /* !!defined(X_VECTORS) */

/*
 * Flags used during conversion.
 */
#define X_ALT       0x001   /* alternate form */
#define X_LADJUST   0x004   /* left adjustment */
#define X_LONGDBL   0x008   /* long double */
#define X_LONGINT   0x010   /* long integer */
#define X_LLONGINT  0x020   /* long long integer */
#define X_SHORTINT  0x040   /* short integer */
#define X_ZEROPAD   0x080   /* zero (as opposed to blank) pad */
#define X_FPT       0x100   /* Floating point number */
#define X_GROUPING  0x200   /* use grouping ("'" flag) */
    /* C99 additional size modifiers: */
#define X_SIZET     0x400   /* size_t */
#define X_PTRDIFFT  0x800   /* ptrdiff_t */
#define X_INTMAXT   0x1000  /* intmax_t */
#define X_CHARINT   0x2000  /* print char using int format */
#if defined(X_VECTORS)
#define X_VECTOR    0x4000  /* Altivec or SSE vector */
#endif /* defined(X_VECTORS) */

/*
 * Macros for converting digits to letters and vice versa
 */
#define x_to_digit(c)   ((c) - '0')
#define x_is_digit(c)   ((unsigned)x_to_digit(c) <= 9)
#define x_to_char(n)    ((n) + '0')

/* Size of the static argument table. */
#define X_STATIC_ARG_TBL_SIZE 8

union x_arg {
    int                 intarg;
    u_int               uintarg;
    long                longarg;
    u_long              ulongarg;
    long long           longlongarg;
    unsigned long long  ulonglongarg;
    ptrdiff_t           ptrdiffarg;
    size_t              sizearg;
    intmax_t            intmaxarg;
    uintmax_t           uintmaxarg;
    void                *pvoidarg;
    char                *pchararg;
    signed char         *pschararg;
    short               *pshortarg;
    int                 *pintarg;
    long                *plongarg;
    long long           *plonglongarg;
    ptrdiff_t           *pptrdiffarg;
    ssize_t             *pssizearg;
    intmax_t            *pintmaxarg;
#if !defined(NO_FLOATING_POINT)
    double              doublearg;
    long double         longdoublearg;
#endif /* !!defined(NO_FLOATING_POINT) */
    wint_t              wintarg;
    wchar_t             *pwchararg;
#if defined(X_VECTORS)
    X_VECTORTYPE        vectorarg;
    unsigned char       vuchararg[16];
    signed char         vchararg[16];
    unsigned short      vushortarg[8];
    signed short        vshortarg[8];
    unsigned int        vuintarg[4];
    signed int          vintarg[4];
    float               vfloatarg[4];
#if defined(X_V64TYPE)
    double              vdoublearg[2];
    unsigned long long  vulonglongarg[2];
    long long           vlonglongarg[2];
#endif /* !defined(X_V64TYPE) */
#endif /* !defined(X_VECTORS) */
};

__BEGIN_DECLS

/* Handle positional parameters. */
X_LOCAL int x___find_arguments(const char *, va_list, union x_arg **);
X_LOCAL int x___find_warguments(const wchar_t *, va_list, union x_arg **);

__END_DECLS

#endif /* !LIBXPRINTF_PRINTFLOCAL_H */
