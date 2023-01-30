/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/gd_qnan.h
 *
 */


#ifndef LIBXPRINTF_GD_QNAN_H
#define LIBXPRINTF_GD_QNAN_H

#if defined(__ppc__) || defined(__ppc64__)

#define f_QNAN      0x7fc00000
#define d_QNAN0     0x7ff80000
#define d_QNAN1     0x0
#define ld_QNAN0    0x7ff80000
#define ld_QNAN1    0x0
#define ld_QNAN2    0x0
#define ld_QNAN3    0x0
#define ldus_QNAN0  0x7ff8
#define ldus_QNAN1  0x0
#define ldus_QNAN2  0x0
#define ldus_QNAN3  0x0
#define ldus_QNAN4  0x0

#elif defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__arm64__)

#define f_QNAN      0x7fc00000
#define d_QNAN0     0x0
#define d_QNAN1     0x7ff80000
#define ld_QNAN0    0x0
#define ld_QNAN1    0xc0000000
#define ld_QNAN2    0x7fff
#define ld_QNAN3    0x0
#define ldus_QNAN0  0x0
#define ldus_QNAN1  0x0
#define ldus_QNAN2  0x0
#define ldus_QNAN3  0xc000
#define ldus_QNAN4  0x7fff

#else
#error Unknown architecture
#endif

#endif /* !LIBXPRINTF_GD_QNAN_H */
