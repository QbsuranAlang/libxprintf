/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/gdtoa.tgz
 *
 */


#include "x_gdtoaimp.h"

void x_rshift(x_Bigint *b, int k) {
    int     n;
    ULong   *x, *x1, *xe, y;

    x = x1 = b->x;
    n = k >> kshift;
    if(n < b->wds) {
        xe = x + b->wds;
        x += n;
        if(k &= kmask) {
            n = ULbits - k;
            y = *x++ >> k;
            while(x < xe) {
                *x1++ = (y | (*x << n)) & ALL_ON;
                y = *x++ >> k;
            }//end while
            if((*x1 = y) != 0) {
                x1++;
            }//end if
        }//end if
        else {
            while(x < xe) {
                *x1++ = *x++;
            }//end while
        }//end else
    }//end if
    if((b->wds = x1 - b->x) == 0) {
        b->x[0] = 0;
    }//end if
}//end x_rshift

int x_trailz(x_Bigint *b) {
    int     n;
    ULong   L, *x, *xe;

    n = 0;
    x = b->x;
    xe = x + b->wds;
    for(n = 0; x < xe && !*x; x++) {
        n += ULbits;
    }//end for
    if(x < xe) {
        L = *x;
        n += x_lo0bits(&L);
    }//end if
    return n;
}//end x_trailz
