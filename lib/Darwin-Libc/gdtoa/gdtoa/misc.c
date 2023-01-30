/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/gdtoa.tgz
 *
 */


#include <libxprintf/libxprintf_visibility.h>

#include "x_gdtoaimp.h"

static x_Bigint *x_freelist[X_Kmax + 1];

#if !defined(Omit_Private_Memory)
#if !defined(PRIVATE_MEM)
#define PRIVATE_MEM 2304
#endif /* !!defined(PRIVATE_MEM) */
#define PRIVATE_mem ((PRIVATE_MEM + sizeof(double) - 1) / sizeof(double))
static double x_private_mem[PRIVATE_mem], *x_pmem_next = x_private_mem;
#endif /* !!defined(Omit_Private_Memory) */

x_Bigint *x_Balloc(int k) {
    int             x;
    x_Bigint        *rv;
#if !defined(Omit_Private_Memory)
    unsigned int    len;
#endif /* !!defined(Omit_Private_Memory) */

    X_ACQUIRE_DTOA_LOCK(0);
    /* The k > X_Kmax case does not need X_ACQUIRE_DTOA_LOCK(0), */
    /* but this case seems very unlikely. */
    if(k <= X_Kmax && (rv = x_freelist[k]) != 0) {
        x_freelist[k] = rv->next;
    }//end if
    else {
        x = 1 << k;
#if defined(Omit_Private_Memory)
        rv = (x_Bigint *)malloc(sizeof(x_Bigint) + (x - 1) * sizeof(ULong));
#else /* defined(Omit_Private_Memory) */
        len = (sizeof(x_Bigint) + (x - 1) * sizeof(ULong) + sizeof(double) - 1) / sizeof(double);
        if(k <= X_Kmax && x_pmem_next - x_private_mem + len <= PRIVATE_mem) {
            rv = (x_Bigint *)x_pmem_next;
            x_pmem_next += len;
        }//end if
        else {
            rv = (x_Bigint *)malloc(len * sizeof(double));
        }//end else
#endif /* !defined(Omit_Private_Memory) */
        rv->k = k;
        rv->maxwds = x;
    }//end else
    X_FREE_DTOA_LOCK(0);
    rv->sign = rv->wds = 0;
    return rv;
}//end x_Balloc

void x_Bfree(x_Bigint *v) {
    if(v) {
        if(v->k > X_Kmax) {
#if defined(FREE)
            FREE((void*)v);
#else /* defined(FREE) */
            free((void*)v);
#endif /* !defined(FREE) */
        }//end if
        else {
            X_ACQUIRE_DTOA_LOCK(0);
            v->next = x_freelist[v->k];
            x_freelist[v->k] = v;
            X_FREE_DTOA_LOCK(0);
        }//end else
    }//end if
}//end x_Bfree

int x_lo0bits(ULong *y) {
    int     k;
    ULong   x;

    x = *y;
    if(x & 7) {
        if(x & 1) {
            return 0;
        }//end if
        if(x & 2) {
            *y = x >> 1;
            return 1;
        }//end if
        *y = x >> 2;
        return 2;
    }//end if
    k = 0;
    if(!(x & 0xffff)) {
        k = 16;
        x >>= 16;
    }//end if
    if(!(x & 0xff)) {
        k += 8;
        x >>= 8;
    }//end if
    if(!(x & 0xf)) {
        k += 4;
        x >>= 4;
    }//end if
    if(!(x & 0x3)) {
        k += 2;
        x >>= 2;
    }//end if
    if(!(x & 1)) {
        k++;
        x >>= 1;
        if(!x) {
            return 32;
        }//end if
    }//end if
    *y = x;
    return k;
}//end x_lo0bits

x_Bigint *x_multadd(x_Bigint *b, int m, int a) {  /* multiply by m and add a */
    int         i, wds;
#if defined(ULLong)
    ULong       *x;
    ULLong      carry, y;
#else /* defined(ULLong) */
    ULong       carry, *x, y;
#if defined(Pack_32)
    ULong       xi, z;
#endif /* !defined(Pack_32) */
#endif /* !defined(ULLong) */
    x_Bigint    *b1;

    wds = b->wds;
    x = b->x;
    i = 0;
    carry = a;
    do {
#if defined(ULLong)
        y = *x * (ULLong)m + carry;
        carry = y >> 32;
        *x++ = y & 0xffffffffUL;
#else /* !defined(ULLong) */
#if defined(Pack_32)
        xi = *x;
        y = (xi & 0xffff) * m + carry;
        z = (xi >> 16) * m + (y >> 16);
        carry = z >> 16;
        *x++ = (z << 16) + (y & 0xffff);
#else /* defined(Pack_32) */
        y = *x * m + carry;
        carry = y >> 16;
        *x++ = y & 0xffff;
#endif /* !defined(Pack_32) */
#endif /* !defined(ULLong) */
    } while(++i < wds);
    if(carry) {
        if(wds >= b->maxwds) {
            b1 = x_Balloc(b->k + 1);
            x_Bcopy(b1, b);
            x_Bfree(b);
            b = b1;
        }//end if
        b->x[wds++] = carry;
        b->wds = wds;
    }//end if
    return b;
}//end x_multadd

int x_hi0bits_D2A(ULong x) {
    int k;

    k = 0;
    if(!(x & 0xffff0000)) {
        k = 16;
        x <<= 16;
    }//end if
    if(!(x & 0xff000000)) {
        k += 8;
        x <<= 8;
    }//end if
    if(!(x & 0xf0000000)) {
        k += 4;
        x <<= 4;
    }//end if
    if(!(x & 0xc0000000)) {
        k += 2;
        x <<= 2;
    }//end if
    if(!(x & 0x80000000)) {
        k++;
        if(!(x & 0x40000000)) {
            return 32;
        }//end if
    }//end if
    return k;
}//end x_hi0bits_D2A

x_Bigint *x_i2b(int i) {
    x_Bigint *b;

    b = x_Balloc(1);
    b->x[0] = i;
    b->wds = 1;
    return b;
}//end x_i2b

x_Bigint *x_mult(x_Bigint *a, x_Bigint *b) {
    int         k, wa, wb, wc;
    ULong       *x, *xa, *xae, *xb, *xbe, *xc, *xc0, y;
    x_Bigint    *c;
#if defined(ULLong)
    ULLong      carry, z;
#else /* defined(ULLong) */
    ULong       carry, z;
#if defined(Pack_32)
    ULong       z2;
#endif /* !defined(Pack_32) */
#endif /* !defined(ULLong) */

    if(a->wds < b->wds) {
        c = a;
        a = b;
        b = c;
    }//end if
    k = a->k;
    wa = a->wds;
    wb = b->wds;
    wc = wa + wb;
    if(wc > a->maxwds) {
        k++;
    }//end if
    c = x_Balloc(k);
    for(x = c->x, xa = x + wc; x < xa; x++) {
        *x = 0;
    }//end for
    xa = a->x;
    xae = xa + wa;
    xb = b->x;
    xbe = xb + wb;
    xc0 = c->x;
#if defined(ULLong)
    for(; xb < xbe; xc0++) {
        if((y = *xb++) != 0) {
            x = xa;
            xc = xc0;
            carry = 0;
            do {
                z = *x++ * (ULLong)y + *xc + carry;
                carry = z >> 32;
                *xc++ = z & 0xffffffffUL;
            } while(x < xae);
            *xc = carry;
        }//end if
    }//end for
#else /* defined(ULLong) */
#if defined(Pack_32)
    for(; xb < xbe; xb++, xc0++) {
        if((y = *xb & 0xffff) != 0) {
            x = xa;
            xc = xc0;
            carry = 0;
            do {
                z = (*x & 0xffff) * y + (*xc & 0xffff) + carry;
                carry = z >> 16;
                z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
                carry = z2 >> 16;
                x_Storeinc(xc, z2, z);
            } while(x < xae);
            *xc = carry;
        }//end if
        if((y = *xb >> 16) != 0) {
            x = xa;
            xc = xc0;
            carry = 0;
            z2 = *xc;
            do {
                z = (*x & 0xffff) * y + (*xc >> 16) + carry;
                carry = z >> 16;
                x_Storeinc(xc, z, z2);
                z2 = (*x++ >> 16) * y + (*xc & 0xffff) + carry;
                carry = z2 >> 16;
            } while(x < xae);
            *xc = z2;
        }//end if
    }//end for
#else /* defined(Pack_32) */
    for(; xb < xbe; xc0++) {
        if((y = *xb++) != 0) {
            x = xa;
            xc = xc0;
            carry = 0;
            do {
                z = *x++ * y + *xc + carry;
                carry = z >> 16;
                *xc++ = z & 0xffff;
            } while(x < xae);
            *xc = carry;
        }//end if
    }//end for
#endif /* !defined(Pack_32) */
#endif /* !defined(ULLong) */
    for(xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc) {
    }//end for
    c->wds = wc;
    return c;
}//end x_mult

static x_Bigint *x_p5s;

x_Bigint *x_pow5mult(x_Bigint *b, int k) {
    int         i;
    x_Bigint    *b1, *p5, *p51;
    static int  p05[3] = { 5, 25, 125 };

    if((i = k & 3) != 0) {
        b = x_multadd(b, p05[i - 1], 0);
    }//end if

    if(!(k >>= 2)) {
        return b;
    }//end if
    if((p5 = x_p5s) == 0) {
        /* first time */
#if defined(MULTIPLE_THREADS)
        X_ACQUIRE_DTOA_LOCK(1);
        if(!(p5 = x_p5s)) {
            p5 = x_p5s = x_i2b(625);
            p5->next = 0;
        }//end if
        X_FREE_DTOA_LOCK(1);
#else /* defined(MULTIPLE_THREADS) */
        p5 = x_p5s = x_i2b(625);
        p5->next = 0;
#endif /* !defined(MULTIPLE_THREADS) */
    }//end if
    for(;;) {
        if(k & 1) {
            b1 = x_mult(b, p5);
            x_Bfree(b);
            b = b1;
        }//end if
        if(!(k >>= 1)) {
            break;
        }//end if
        if((p51 = p5->next) == 0) {
#if defined(MULTIPLE_THREADS)
            X_ACQUIRE_DTOA_LOCK(1);
            if(!(p51 = p5->next)) {
                p51 = p5->next = x_mult(p5,p5);
                p51->next = 0;
            }//end if
            X_FREE_DTOA_LOCK(1);
#else /* defined(MULTIPLE_THREADS) */
            p51 = p5->next = x_mult(p5,p5);
            p51->next = 0;
#endif /* !defined(MULTIPLE_THREADS) */
        }//end if
        p5 = p51;
    }//end for
    return b;
}//end x_pow5mult

x_Bigint *x_lshift(x_Bigint *b, int k) {
    int         i, k1, n, n1;
    ULong       *x, *x1, *xe, z;
    x_Bigint    *b1;

    n = k >> kshift;
    k1 = b->k;
    n1 = n + b->wds + 1;
    for(i = b->maxwds; n1 > i; i <<= 1) {
        k1++;
    }//end for
    b1 = x_Balloc(k1);
    x1 = b1->x;
    for(i = 0; i < n; i++) {
        *x1++ = 0;
    }//end for
    x = b->x;
    xe = x + b->wds;
    if(k &= kmask) {
#if defined(Pack_32)
        k1 = 32 - k;
        z = 0;
        do {
            *x1++ = *x << k | z;
            z = *x++ >> k1;
        } while(x < xe);
        if((*x1 = z) != 0) {
            ++n1;
        }//end if
#else /* defined(Pack_32) */
        k1 = 16 - k;
        z = 0;
        do {
            *x1++ = *x << k  & 0xffff | z;
            z = *x++ >> k1;
        } while(x < xe);
        if(*x1 = z) {
            ++n1;
        }//end if
#endif /* !defined(Pack_32) */
    }//end if
    else {
        do {
            *x1++ = *x++;
        } while(x < xe);
    }//end else
    b1->wds = n1 - 1;
    x_Bfree(b);
    return b1;
}//end x_lshift

int x_cmp(x_Bigint *a, x_Bigint *b) {
    int     i, j;
    ULong   *xa, *xa0, *xb, *xb0;

    i = a->wds;
    j = b->wds;

    if(i -= j) {
        return i;
    }//end if
    xa0 = a->x;
    xa = xa0 + j;
    xb0 = b->x;
    xb = xb0 + j;
    for(;;) {
        if(*--xa != *--xb) {
            return *xa < *xb ? -1 : 1;
        }//end if
        if(xa <= xa0) {
            break;
        }//end if
    }//end for
    return 0;
}//end x_cmp

x_Bigint *x_diff(x_Bigint *a, x_Bigint *b) {
    int         i, wa, wb;
    ULong       *xa, *xae, *xb, *xbe, *xc;
    x_Bigint    *c;
#if defined(ULLong)
    ULLong      borrow, y;
#else /* ULLong */
    ULong       borrow, y;
#if defined(Pack_32)
    ULong       z;
#endif /* !defined(Pack_32) */
#endif /* !defined(ULLong) */

    i = x_cmp(a,b);
    if(!i) {
        c = x_Balloc(0);
        c->wds = 1;
        c->x[0] = 0;
        return c;
    }//end if
    if(i < 0) {
        c = a;
        a = b;
        b = c;
        i = 1;
    }//end if
    else {
        i = 0;
    }//end else
    c = x_Balloc(a->k);
    c->sign = i;
    wa = a->wds;
    xa = a->x;
    xae = xa + wa;
    wb = b->wds;
    xb = b->x;
    xbe = xb + wb;
    xc = c->x;
    borrow = 0;
#if defined(ULLong)
    do {
        y = (ULLong)*xa++ - *xb++ - borrow;
        borrow = y >> 32 & 1UL;
        *xc++ = y & 0xffffffffUL;
    } while(xb < xbe);
    while(xa < xae) {
        y = *xa++ - borrow;
        borrow = y >> 32 & 1UL;
        *xc++ = y & 0xffffffffUL;
    }//end while
#else /* defined(ULLong) */
#if defined(Pack_32)
    do {
        y = (*xa & 0xffff) - (*xb & 0xffff) - borrow;
        borrow = (y & 0x10000) >> 16;
        z = (*xa++ >> 16) - (*xb++ >> 16) - borrow;
        borrow = (z & 0x10000) >> 16;
        x_Storeinc(xc, z, y);
    } while(xb < xbe);
    while(xa < xae) {
        y = (*xa & 0xffff) - borrow;
        borrow = (y & 0x10000) >> 16;
        z = (*xa++ >> 16) - borrow;
        borrow = (z & 0x10000) >> 16;
        x_Storeinc(xc, z, y);
    }//end while
#else /* defined(Pack_32) */
    do {
        y = *xa++ - *xb++ - borrow;
        borrow = (y & 0x10000) >> 16;
        *xc++ = y & 0xffff;
    } while(xb < xbe);
    while(xa < xae) {
        y = *xa++ - borrow;
        borrow = (y & 0x10000) >> 16;
        *xc++ = y & 0xffff;
    }//end while
#endif /* !defined(Pack_32) */
#endif /* !defined(ULLong) */
    while(!*--xc) {
        wa--;
    }//end while
    c->wds = wa;
    return c;
}//end x_diff

double x_b2d(x_Bigint *a, int *e) {
    U       d;
    int     k;
    ULong   *xa, *xa0, w, y, z;
#if defined(VAX)
    ULong   d0, d1;
#else /* defined(VAX) */
#define d0 x_word0(&d)
#define d1 x_word1(&d)
#endif /* !defined(VAX) */

    xa0 = a->x;
    xa = xa0 + a->wds;
    y = *--xa;

    k = x_hi0bits(y);
    *e = 32 - k;
#if defined(Pack_32)
    if(k < Ebits) {
        d0 = Exp_1 | y >> (Ebits - k);
        w = xa > xa0 ? *--xa : 0;
        d1 = y << ((32-Ebits) + k) | w >> (Ebits - k);
        goto ret_d;
    }//end if
    z = xa > xa0 ? *--xa : 0;
    if(k -= Ebits) {
        d0 = Exp_1 | y << k | z >> (32 - k);
        y = xa > xa0 ? *--xa : 0;
        d1 = z << k | y >> (32 - k);
    }//end if
    else {
        d0 = Exp_1 | y;
        d1 = z;
    }//end else
#else /* defined(Pack_32) */
    if(k < Ebits + 16) {
        z = xa > xa0 ? *--xa : 0;
        d0 = Exp_1 | y << k - Ebits | z >> Ebits + 16 - k;
        w = xa > xa0 ? *--xa : 0;
        y = xa > xa0 ? *--xa : 0;
        d1 = z << k + 16 - Ebits | w << k - Ebits | y >> 16 + Ebits - k;
        goto ret_d;
    }//end if
    z = xa > xa0 ? *--xa : 0;
    w = xa > xa0 ? *--xa : 0;
    k -= Ebits + 16;
    d0 = Exp_1 | y << k + 16 | z << k | w >> 16 - k;
    y = xa > xa0 ? *--xa : 0;
    d1 = w << k + 16 | y << k;
#endif /* !defined(Pack_32) */
ret_d:
#if defined(VAX)
    x_word0(&d) = d0 >> 16 | d0 << 16;
    x_word1(&d) = d1 >> 16 | d1 << 16;
#endif /* !defined(VAX) */
    return x_dval(&d);
}//end x_b2d
#undef d0
#undef d1

x_Bigint *x_d2b(double dd, int *e, int *bits) {
    U           d;
    x_Bigint    *b;
#if !defined(Sudden_Underflow)
    int         i;
#endif /* !!defined(Sudden_Underflow) */
    int         de, k;
    ULong       *x, y, z;
#if defined(VAX)
    ULong       d0, d1;
#else /* defined(VAX) */
#define d0 x_word0(&d)
#define d1 x_word1(&d)
#endif /* !defined(VAX) */
    d.d = dd;
#if defined(VAX)
    d0 = x_word0(&d) >> 16 | x_word0(&d) << 16;
    d1 = x_word1(&d) >> 16 | x_word1(&d) << 16;
#endif /* !defined(VAX) */

#if defined(Pack_32)
    b = x_Balloc(1);
#else /* defined(Pack_32) */
    b = x_Balloc(2);
#endif /* !defined(Pack_32) */
    x = b->x;

    z = d0 & Frac_mask;
    d0 &= 0x7fffffff;    /* clear sign bit, which we ignore */
#if defined(Sudden_Underflow)
    de = (int)(d0 >> Exp_shift);
#if !defined(IBM)
    z |= Exp_msk11;
#endif /* !!defined(IBM) */
#else /* defined(Sudden_Underflow) */
    if((de = (int)(d0 >> Exp_shift)) != 0) {
        z |= Exp_msk1;
    }//end if
#endif /* !defined(Sudden_Underflow) */
#if defined(Pack_32)
    if((y = d1) != 0) {
        if((k = x_lo0bits(&y)) != 0) {
            x[0] = y | z << (32 - k);
            z >>= k;
        }//end if
        else {
            x[0] = y;
        }//end else
#if !defined(Sudden_Underflow)
        i =
#endif /* !!defined(Sudden_Underflow) */
            b->wds = (x[1] = z) !=0 ? 2 : 1;
    }//end if
    else {
        k = x_lo0bits(&z);
        x[0] = z;
#if !defined(Sudden_Underflow)
        i =
#endif /* !!defined(Sudden_Underflow) */
            b->wds = 1;
        k += 32;
    }//end else
#else /*  defined(Pack_32) */
    if((y = d1) != 0) {
        if((k = x_lo0bits(&y)) != 0) {
            if(k >= 16) {
                x[0] = y | z << 32 - k & 0xffff;
                x[1] = z >> k - 16 & 0xffff;
                x[2] = z >> k;
                i = 2;
            }//end if
            else {
                x[0] = y & 0xffff;
                x[1] = y >> 16 | z << 16 - k & 0xffff;
                x[2] = z >> k & 0xffff;
                x[3] = z >> k+16;
                i = 3;
            }//end else
        }//end if
        else {
            x[0] = y & 0xffff;
            x[1] = y >> 16;
            x[2] = z & 0xffff;
            x[3] = z >> 16;
            i = 3;
        }//end else
    }//end if
    else {
        k = x_lo0bits(&z);
        if(k >= 16) {
            x[0] = z;
            i = 0;
        }//end if
        else {
            x[0] = z & 0xffff;
            x[1] = z >> 16;
            i = 1;
        }//end else
        k += 32;
    }//end else
    while(!x[i]) {
        --i;
    }//end while
    b->wds = i + 1;
#endif /* ! defined(Pack_32) */
#if !defined(Sudden_Underflow)
    if(de) {
#endif /* !!!defined(Sudden_Underflow) */
#if defined(IBM)
        *e = (de - Bias - (P - 1) << 2) + k;
        *bits = 4 * P + 8 - k - x_hi0bits(x_word0(&d) & Frac_mask);
#else /* defined(IBM) */
        *e = de - Bias - (P - 1) + k;
        *bits = P - k;
#endif /* !defined(IBM) */
#if !defined(Sudden_Underflow)
        }
    else {
        *e = de - Bias - (P - 1) + 1 + k;
#if defined(Pack_32)
        *bits = 32 * i - x_hi0bits(x[i-1]);
#else /* defined(Pack_32) */
        *bits = (i + 2) * 16 - x_hi0bits(x[i]);
#endif /* !defined(Pack_32) */
        }
#endif /* !!defined(Sudden_Underflow) */
    return b;
}//end x_d2b
#undef d0
#undef d1

X_LOCAL const double
#if defined(IEEE_Arith)
x_bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
#else /* defined(IEEE_Arith) */
#if defined(IBM)
x_bigtens[] = { 1e16, 1e32, 1e64 };
#else /* defined(IBM) */
x_bigtens[] = { 1e16, 1e32 };
#endif /* !defined(IBM) */
#endif/* !defined(IEEE_Arith) */

X_LOCAL const double x_tens[] = {
    1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
    1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
    1e20, 1e21, 1e22
#if defined(VAX)
    , 1e23, 1e24
#endif /* !defined(VAX) */
};

#if defined(NO_STRING_H)

void *x_memcpy_D2A(void *a1, void *b1, size_t len) {
    char *a, *ae, *b, *a0;

    a = (char*)a1;
    ae = a + len;
    b = (char*)b1;
    a0 = a;
    while(a < ae) {
        *a++ = *b++;
    }//end while
    return (void *)a0;
}//end x_memcpy_D2A

#endif /* defined(NO_STRING_H) */
