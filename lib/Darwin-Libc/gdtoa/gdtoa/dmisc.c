/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/gdtoa.tgz
 *
 */


#include <libxprintf/libxprintf_visibility.h>

#include "x_gdtoaimp.h"

#if !defined(MULTIPLE_THREADS)
X_LOCAL char *x_dtoa_result;
#endif /* !!defined(MULTIPLE_THREADS) */

char *x_rv_alloc(int i) {
    int j, k, *r;

    j = sizeof(ULong);
    for(k = 0; sizeof(x_Bigint) - sizeof(ULong) - sizeof(int) + j <= i; j <<= 1) {
        k++;
    }//end for
    r = (int *)x_Balloc(k);
    *r = k;
    return
#if !defined(MULTIPLE_THREADS)
    x_dtoa_result =
#endif /* !!defined(MULTIPLE_THREADS) */
        (char *)(r + 1);
}//end x_rv_alloc

char *x_nrv_alloc(char *s, char **rve, int n) {
    char *rv, *t;

    t = rv = x_rv_alloc(n);
    while((*t = *s++) != 0) {
        t++;
    }//end while
    if(rve) {
        *rve = t;
    }//end if
    return rv;
}//end x_nrv_alloc

/* x_freedtoa(s) must be used to free values s returned by x_dtoa
 * when MULTIPLE_THREADS is #defined.  It should be used in all cases,
 * but for consistency with earlier versions of x_dtoa, it is optional
 * when MULTIPLE_THREADS is not defined.
 */

void x_freedtoa(char *s) {
    x_Bigint *b;

    b = (x_Bigint *)((int *)s - 1);
    b->maxwds = 1 << (b->k = *(int*)b);
    x_Bfree(b);
#if !defined(MULTIPLE_THREADS)
    if(s == x_dtoa_result) {
        x_dtoa_result = 0;
    }//end if
#endif /* !!defined(MULTIPLE_THREADS) */
}//end x_freedtoa

int x_quorem(x_Bigint *b, x_Bigint *S) {
    int     n;
    ULong   *bx, *bxe, q, *sx, *sxe;
#if defined(ULLong)
    ULLong  borrow, carry, y, ys;
#else /* defined(ULLong) */
    ULong   borrow, carry, y, ys;
#if defined(Pack_32)
    ULong   si, z, zs;
#endif /* !defined(Pack_32) */
#endif /* !defined(ULLong) */

    n = S->wds;
    if(b->wds < n) {
        return 0;
    }//end if
    sx = S->x;
    sxe = sx + --n;
    bx = b->x;
    bxe = bx + n;
    q = *bxe / (*sxe + 1);    /* ensure q <= true quotient */

    if(q) {
        borrow = 0;
        carry = 0;
        do {
#if defined(ULLong)
            ys = *sx++ * (ULLong)q + carry;
            carry = ys >> 32;
            y = *bx - (ys & 0xffffffffUL) - borrow;
            borrow = y >> 32 & 1UL;
            *bx++ = y & 0xffffffffUL;
#else /* defined(ULLong) */
#if defined(Pack_32)
            si = *sx++;
            ys = (si & 0xffff) * q + carry;
            zs = (si >> 16) * q + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) - borrow;
            borrow = (y & 0x10000) >> 16;
            z = (*bx >> 16) - (zs & 0xffff) - borrow;
            borrow = (z & 0x10000) >> 16;
            x_Storeinc(bx, z, y);
#else /* defined(Pack_32) */
            ys = *sx++ * q + carry;
            carry = ys >> 16;
            y = *bx - (ys & 0xffff) - borrow;
            borrow = (y & 0x10000) >> 16;
            *bx++ = y & 0xffff;
#endif /* !defined(Pack_32) */
#endif /* !defined(ULLong) */
        } while(sx <= sxe);
        if(!*bxe) {
            bx = b->x;
            while(--bxe > bx && !*bxe) {
                --n;
            }//end while
            b->wds = n;
        }//end if
    }//end if
    if(x_cmp(b, S) >= 0) {
        q++;
        borrow = 0;
        carry = 0;
        bx = b->x;
        sx = S->x;
        do {
#if defined(ULLong)
            ys = *sx++ + carry;
            carry = ys >> 32;
            y = *bx - (ys & 0xffffffffUL) - borrow;
            borrow = y >> 32 & 1UL;
            *bx++ = y & 0xffffffffUL;
#else /* defined(ULLong) */
#if defined(Pack_32)
            si = *sx++;
            ys = (si & 0xffff) + carry;
            zs = (si >> 16) + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) - borrow;
            borrow = (y & 0x10000) >> 16;
            z = (*bx >> 16) - (zs & 0xffff) - borrow;
            borrow = (z & 0x10000) >> 16;
            x_Storeinc(bx, z, y);
#else /* defined(Pack_32) */
            ys = *sx++ + carry;
            carry = ys >> 16;
            y = *bx - (ys & 0xffff) - borrow;
            borrow = (y & 0x10000) >> 16;
            *bx++ = y & 0xffff;
#endif /* !defined(Pack_32) */
#endif /* !defined(ULLong) */
        } while(sx <= sxe);
        bx = b->x;
        bxe = bx + n;
        if(!*bxe) {
            while(--bxe > bx && !*bxe) {
                --n;
            }//end while
            b->wds = n;
        }//end if
    }//end if
    return q;
}//end x_quorem
