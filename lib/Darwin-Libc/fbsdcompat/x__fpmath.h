/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/fbsdcompat/_fpmath.h
 *
 */


#ifndef LIBXPRINTF__FPMATH_H
#define LIBXPRINTF__FPMATH_H

#if defined(__i386__) || defined(__x86_64__)

union x_IEEEl2bits {
    long double         e;
    struct {
        unsigned int    manl:32;
        unsigned int    manh:32;
        unsigned int    exp:15;
        unsigned int    sign:1;
        unsigned int    junk:16;
    } bits;
};

#define X_LDBL_MANL_SIZE 32

#define X_LDBL_TO_ARRAY32(u, a)             \
    do {                                    \
        (a)[0] = (uint32_t)(u).bits.manl;   \
        (a)[1] = (uint32_t)(u).bits.manh;   \
    } while(0)

#elif defined(__arm__) || defined(__arm64__)

union x_IEEEl2bits {
    long double     e;
    struct {
#if !defined(__ARMEB__)
    unsigned int    manl:32;
    unsigned int    manh:20;
    unsigned int    exp :11;
    unsigned int    sign:1;
#else /* !defined(__ARMEB__) */
    unsigned int    sign:1;
    unsigned int    exp :11;
    unsigned int    manh:20;
    unsigned int    manl:32;
#endif /* !!defined(__ARMEB__) */
    } bits;
};

#define X_LDBL_MANL_SIZE 32

#define X_LDBL_TO_ARRAY32(u, a)             \
    do {                                    \
        (a)[0] = (uint32_t)(u).bits.manl;   \
        (a)[1] = (uint32_t)(u).bits.manh;   \
    } while(0)

#else
#error Unsupported architecture
#endif

#endif /* !LIBXPRINTF__FPMATH_H */
