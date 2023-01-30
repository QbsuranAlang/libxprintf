/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/arith.h
 *
 */


#ifndef LIBXPRINTF_ARITH_H
#define LIBXPRINTF_ARITH_H

#if defined(__i386__)

#define IEEE_8087
#define Arith_Kind_ASL  1

#elif defined(__x86_64__)

#define IEEE_8087
#define Arith_Kind_ASL  1
#define Long            int
#define Intcast         (int)(long)
#define Double_Align
#define X64_bit_pointers

#elif defined(__arm__)

#if __VFP_FP__
#define IEEE_8087
#else /* __VFP_FP__ */
#define IEEE_MC68k
#endif /* !__VFP_FP__ */
#define Arith_Kind_ASL  1

#elif defined(__arm64__)

#define IEEE_8087
#define Arith_Kind_ASL  1
#define Long            int
#define Intcast         (int)(long)
#define Double_Align
#define X64_bit_pointers

#else
#error Unsupported architecture
#endif

#define Honor_FLT_ROUNDS
#define Trust_FLT_ROUNDS

#endif /* !LIBXPRINTF_ARITH_H */
