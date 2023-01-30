/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/gdtoa.tgz
 *
 */


#include "x_gdtoaimp.h"

static x_Bigint *x_bitstob(ULong *bits, int nbits, int *bbits) {
    int         i, k;
    ULong       *be, *x, *x0;
    x_Bigint    *b;

    i = ULbits;
    k = 0;
    while(i < nbits) {
        i <<= 1;
        k++;
    }//end while
#if !defined(Pack_32)
    if(!k) {
        k = 1;
    }//end if
#endif /* !!defined(Pack_32) */
    b = x_Balloc(k);
    be = bits + ((nbits - 1) >> kshift);
    x = x0 = b->x;
    do {
        *x++ = *bits & ALL_ON;
#if defined(Pack_16)
        *x++ = (*bits >> 16) & ALL_ON;
#endif /* !defined(Pack_16) */
    } while(++bits <= be);
    i = x - x0;
    while(!x0[--i]) {
        if(!i) {
            b->wds = 0;
            *bbits = 0;
            goto ret;
        }//end if
    }//end while
    b->wds = i + 1;
    *bbits = i * ULbits + 32 - x_hi0bits(b->x[i]);
ret:
    return b;
}//end x_bitstob

/* x_dtoa for IEEE arithmetic (dmg): convert double to ASCII string.
 *
 * Inspired by "How to Print Floating-Point Numbers Accurately" by
 * Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 112-126].
 *
 * Modifications:
 *    1. Rather than iterating, we use a simple numeric overestimate
 *       to determine k = floor(log10(d)).  We scale relevant
 *       quantities using O(log2(k)) rather than O(k) x_multiplications.
 *    2. For some modes > 2 (corresponding to ecvt and fcvt), we don't
 *       try to generate digits strictly left to right.  Instead, we
 *       compute with fewer bits and propagate the carry if necessary
 *       when rounding the final digit up.  This is often faster.
 *    3. Under the assumption that input will be rounded nearest,
 *       mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22.
 *       That is, we allow equality in stopping tests when the
 *       round-nearest rule will give the same floating-point value
 *       as would satisfaction of the stopping test with strict
 *       inequality.
 *    4. We remove common factors of powers of 2 from relevant
 *       quantities.
 *    5. When converting floating-point integers less than 1e16,
 *       we use floating-point arithmetic rather than resorting
 *       to x_multiple-precision integers.
 *    6. When asked to produce fewer than 15 digits, we first try
 *       to get by with floating-point arithmetic; we resort to
 *       x_multiple-precision integer arithmetic only if we cannot
 *       guarantee that the floating-point calculation has given
 *       the correctly rounded result.  For k requested digits and
 *       "uniformly" distributed input, the probability is
 *       something like 10^(k-15) that we must resort to the Long
 *       calculation.
 */

char *x_gdtoa(FPI *fpi, int be, ULong *bits, int *kindp, int mode, int ndigits, int *decpt, char **rve) {
    /* Arguments ndigits and decpt are similar to the second and third
    arguments of ecvt and fcvt; trailing zeros are suppressed from
    the returned string.  If not null, *rve is set to point
    to the end of the return value.  If d is +-Infinity or NaN,
    then *decpt is set to 9999.

    mode:
        0 ==> shortest string that yields d when read in
            and rounded to nearest.
        1 ==> like 0, but with Steele & White stopping rule;
            e.g. with IEEE P754 arithmetic , mode 0 gives
            1e23 whereas mode 1 gives 9.999999999999999e22.
        2 ==> max(1,ndigits) significant digits.  This gives a
            return value similar to that of ecvt, except
            that trailing zeros are suppressed.
        3 ==> through ndigits past the decimal point.  This
            gives a return value similar to that from fcvt,
            except that trailing zeros are suppressed, and
            ndigits can be negative.
        4-9 should give the same return values as 2-3, i.e.,
            4 <= mode <= 9 ==> same return as mode
            2 + (mode & 1).  These modes are mainly for
            debugging; often they run slower but sometimes
            faster than modes 2-3.
        4,5,8,9 ==> left-to-right digit generation.
        6-9 ==> don't try fast floating-point estimate
            (if applicable).

        Values of mode other than 0-9 are treated as mode 0.

        Sufficient space is allocated to the return value
        to hold the suppressed trailing zeros.
    */

    U           d, eps;
    int         bbits, b2, b5, be0, dig, i, ieps, ilim, ilim0, ilim1, inex;
    int         j, j1, k, k0, k_check, kind, leftright, m2, m5, nbits;
    int         rdir, s2, s5, spec_case, try_quick;
    char        *s, *s0;
    Long        L;
    x_Bigint    *b, *b1, *delta, *mlo, *mhi, *mhi1, *S;
    double      d2, ds;

#if !defined(MULTIPLE_THREADS)
    if(x_dtoa_result) {
        x_freedtoa(x_dtoa_result);
        x_dtoa_result = NULL;
    }//end if
#endif /* !!defined(MULTIPLE_THREADS) */
    inex = 0;
    kind = *kindp &= ~x_STRTOG_Inexact;
    switch(kind & x_STRTOG_Retmask) {
    case x_STRTOG_Zero:
        goto ret_zero;
    case x_STRTOG_Normal:
    case x_STRTOG_Denormal:
        break;
    case x_STRTOG_Infinite:
        *decpt = -32768;
        return x_nrv_alloc("Infinity", rve, 8);
    case x_STRTOG_NaN:
        *decpt = -32768;
        return x_nrv_alloc("NaN", rve, 3);
    default:
        return 0;
    }//end switch
    b = x_bitstob(bits, nbits = fpi->nbits, &bbits);
    be0 = be;
    if((i = x_trailz(b)) !=0 ) {
        x_rshift(b, i);
        be += i;
        bbits -= i;
    }//end if
    if(!b->wds) {
        x_Bfree(b);
ret_zero:
        *decpt = 1;
        return x_nrv_alloc("0", rve, 1);
    }//end if

    x_dval(&d) = x_b2d(b, &i);
    i = be + bbits - 1;
    x_word0(&d) &= Frac_mask1;
    x_word0(&d) |= Exp_11;
#if defined(IBM)
    if((j = 11 - x_hi0bits(x_word0(&d) & Frac_mask)) != 0) {
        x_dval(&d) /= 1 << j;
    }//end if
#endif /* !defined(IBM) */

    /* log(x)    ~=~ log(1.5) + (x-1.5)/1.5
     * log10(x)     =  log(x) / log(10)
     *        ~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
     * log10(&d) = (i-Bias)*log(2)/log(10) + log10(d2)
     *
     * This suggests computing an approximation k to log10(&d) by
     *
     * k = (i - Bias)*0.301029995663981
     *    + ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );
     *
     * We want k to be too large rather than too small.
     * The error in the first-order Taylor series approximation
     * is in our favor, so we just round up the constant enough
     * to compensate for any error in the x_multiplication of
     * (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
     * and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
     * adding 1e-13 to the constant term more than suffices.
     * Hence we adjust the constant term to 0.1760912590558.
     * (We could get a more accurate k by invoking log10,
     *  but this is probably not worthwhile.)
     */
#if defined(IBM)
    i <<= 2;
    i += j;
#endif /* !defined(IBM) */
    ds = (x_dval(&d) - 1.5) * 0.289529654602168 + 0.1760912590558 + i * 0.301029995663981;

    /* correct assumption about exponent range */
    if((j = i) < 0) {
        j = -j;
    }//end if
    if((j -= 1077) > 0) {
        ds += j * 7e-17;
    }//end if

    k = (int)ds;
    if(ds < 0. && ds != k) {
        k--;    /* want k = floor(ds) */
    }//end if
    k_check = 1;
#if defined(IBM)
    j = be + bbits - 1;
    if((j1 = j & 3) != 0) {
        x_dval(&d) *= 1 << j1;
    }//end if
    x_word0(&d) += j << Exp_shift - 2 & Exp_mask;
#else /* defined(IBM) */
    x_word0(&d) += (be + bbits - 1) << Exp_shift;
#endif /* !defined(IBM) */
    if(k >= 0 && k <= Ten_pmax) {
        if(x_dval(&d) < x_tens[k]) {
            k--;
        }//end if
        k_check = 0;
    }//end if
    j = bbits - i - 1;
    if(j >= 0) {
        b2 = 0;
        s2 = j;
    }//end if
    else {
        b2 = -j;
        s2 = 0;
    }//end else
    if(k >= 0) {
        b5 = 0;
        s5 = k;
        s2 += k;
    }//end if
    else {
        b2 -= k;
        b5 = -k;
        s5 = 0;
    }//end else
    if(mode < 0 || mode > 9) {
        mode = 0;
    }//end if
    try_quick = 1;
    if(mode > 5) {
        mode -= 4;
        try_quick = 0;
    }//end if
    leftright = 1;
    ilim = ilim1 = -1;    /* Values for cases 0 and 1; done here to */
                /* silence erroneous "gcc -Wall" warning. */
    switch(mode) {
    case 0:
    case 1:
        i = (int)(nbits * .30103) + 3;
        ndigits = 0;
        break;
    case 2:
        leftright = 0;
        /* fallthrough */
    case 4:
        if(ndigits <= 0) {
            ndigits = 1;
        }//end if
        ilim = ilim1 = i = ndigits;
        break;
    case 3:
        leftright = 0;
        /* fallthrough */
    case 5:
        i = ndigits + k + 1;
        ilim = i;
        ilim1 = i - 1;
        if(i <= 0) {
            i = 1;
        }//end if
    }//end switch
    s = s0 = x_rv_alloc(i);

    if((rdir = fpi->rounding - 1) != 0) {
        if(rdir < 0) {
            rdir = 2;
        }//end if
        if(kind & x_STRTOG_Neg) {
            rdir = 3 - rdir;
        }//end if
    }//end if

    /* Now rdir = 0 ==> round near, 1 ==> round up, 2 ==> round down. */

    if(ilim >= 0 && ilim <= Quick_max && try_quick && !rdir
#if !defined(IMPRECISE_INEXACT)
        && k == 0
#endif /* !!defined(IMPRECISE_INEXACT) */
        ) {

        /* Try to get by with floating-point arithmetic. */

        i = 0;
        d2 = x_dval(&d);
#if defined(IBM)
        if((j = 11 - x_hi0bits(x_word0(&d) & Frac_mask)) != 0) {
            x_dval(&d) /= 1 << j;
        }//end if
#endif /* !defined(IBM) */
        k0 = k;
        ilim0 = ilim;
        ieps = 2; /* conservative */
        if(k > 0) {
            ds = x_tens[k & 0xf];
            j = k >> 4;
            if(j & Bletch) {
                /* prevent overflows */
                j &= Bletch - 1;
                x_dval(&d) /= x_bigtens[x_n_bigtens - 1];
                ieps++;
            }//end if
            for(; j; j >>= 1, i++) {
                if(j & 1) {
                    ieps++;
                    ds *= x_bigtens[i];
                }//end if
            }//end for
        }//end if
        else {
            ds = 1.;
            if((j1 = -k) != 0) {
                x_dval(&d) *= x_tens[j1 & 0xf];
                for(j = j1 >> 4; j; j >>= 1, i++) {
                    if(j & 1) {
                        ieps++;
                        x_dval(&d) *= x_bigtens[i];
                    }//end if
                }//end for
            }//end if
        }//end else
        if(k_check && x_dval(&d) < 1. && ilim > 0) {
            if(ilim1 <= 0) {
                goto fast_failed;
            }//end if
            ilim = ilim1;
            k--;
            x_dval(&d) *= 10.;
            ieps++;
        }//end if
        x_dval(&eps) = ieps * x_dval(&d) + 7.;
        x_word0(&eps) -= (P - 1) * Exp_msk1;
        if(ilim == 0) {
            S = mhi = 0;
            x_dval(&d) -= 5.;
            if(x_dval(&d) > x_dval(&eps)) {
                goto one_digit;
            }//end if
            if(x_dval(&d) < -x_dval(&eps)) {
                goto no_digits;
            }//end if
            goto fast_failed;
        }//end if
#if !defined(No_leftright)
        if(leftright) {
            /* Use Steele & White method of only
             * generating digits needed.
             */
            x_dval(&eps) = ds * 0.5 / x_tens[ilim - 1] - x_dval(&eps);
            for(i = 0;;) {
                L = (Long)(x_dval(&d) / ds);
                x_dval(&d) -= L*ds;
                *s++ = '0' + (int)L;
                if(x_dval(&d) < x_dval(&eps)) {
                    if(x_dval(&d)) {
                        inex = x_STRTOG_Inexlo;
                    }//end if
                    goto ret1;
                }//end if
                if(ds - x_dval(&d) < x_dval(&eps)) {
                    goto bump_up;
                }//end if
                if(++i >= ilim) {
                    break;
                }//end if
                x_dval(&eps) *= 10.;
                x_dval(&d) *= 10.;
            }//end for
        }//end if
        else {
#endif /* !!defined(No_leftright) */
            /* Generate ilim digits, then fix them up. */
            x_dval(&eps) *= x_tens[ilim - 1];
            for(i = 1;; i++, x_dval(&d) *= 10.) {
                if((L = (Long)(x_dval(&d) / ds)) != 0) {
                    x_dval(&d) -= L*ds;
                }//end if
                *s++ = '0' + (int)L;
                if(i == ilim) {
                    ds *= 0.5;
                    if(x_dval(&d) > ds + x_dval(&eps)) {
                        goto bump_up;
                    }//end if
                    else if(x_dval(&d) < ds - x_dval(&eps)) {
                        if(x_dval(&d)) {
                            inex = x_STRTOG_Inexlo;
                        }//end if
                        goto clear_trailing0;
                    }//end if
                    break;
                }//end if
            }//end for
#if !defined(No_leftright)
        }//end else
#endif /* !!defined(No_leftright) */
fast_failed:
        s = s0;
        x_dval(&d) = d2;
        k = k0;
        ilim = ilim0;
    }//end if

    /* Do we have a "small" integer? */

    if(be >= 0 && k <= Int_max) {
        /* Yes. */
        ds = x_tens[k];
        if(ndigits < 0 && ilim <= 0) {
            S = mhi = 0;
            if(ilim < 0 || x_dval(&d) <= 5 * ds) {
                goto no_digits;
            }//end if
            goto one_digit;
        }//end if
        for(i = 1;; i++, x_dval(&d) *= 10.) {
            L = x_dval(&d) / ds;
            x_dval(&d) -= L*ds;
#if defined(Check_FLT_ROUNDS)
            /* If FLT_ROUNDS == 2, L will usually be high by 1 */
            if(x_dval(&d) < 0) {
                L--;
                x_dval(&d) += ds;
            }//end if
#endif /* !defined(Check_FLT_ROUNDS) */
            *s++ = '0' + (int)L;
            if(x_dval(&d) == 0.) {
                break;
            }//end if
            if(i == ilim) {
                if(rdir) {
                    if(rdir == 1) {
                        goto bump_up;
                    }//end if
                    inex = x_STRTOG_Inexlo;
                    goto ret1;
                }//end if
                x_dval(&d) += x_dval(&d);
                if(x_dval(&d) > ds || (x_dval(&d) == ds && L & 1)) {
bump_up:
                    inex = x_STRTOG_Inexhi;
                    while(*--s == '9') {
                        if(s == s0) {
                            k++;
                            *s = '0';
                            break;
                        }//end if
                    }//end while
                    ++*s++;
                }//end if
                else {
                    inex = x_STRTOG_Inexlo;
clear_trailing0:
                    while(*--s == '0') {
                    }//end while
                    ++s;
                }//end else
                break;
            }//end if
        }//end for
        goto ret1;
    }//end if

    m2 = b2;
    m5 = b5;
    mhi = mlo = 0;
    if(leftright) {
        if(mode < 2) {
            i = nbits - bbits;
            if(be - i++ < fpi->emin) {
                /* denormal */
                i = be - fpi->emin + 1;
            }//end if
        }//end if
        else {
            j = ilim - 1;
            if(m5 >= j) {
                m5 -= j;
            }//end if
            else {
                s5 += j -= m5;
                b5 += j;
                m5 = 0;
            }//end else
            if((i = ilim) < 0) {
                m2 -= i;
                i = 0;
            }//end if
        }//end else
        b2 += i;
        s2 += i;
        mhi = x_i2b(1);
    }//end if
    if(m2 > 0 && s2 > 0) {
        i = m2 < s2 ? m2 : s2;
        b2 -= i;
        m2 -= i;
        s2 -= i;
    }//end if
    if(b5 > 0) {
        if(leftright) {
            if(m5 > 0) {
                mhi = x_pow5mult(mhi, m5);
                b1 = x_mult(mhi, b);
                x_Bfree(b);
                b = b1;
            }//end if
            if((j = b5 - m5) != 0) {
                b = x_pow5mult(b, j);
            }//end if
        }//end if
        else {
            b = x_pow5mult(b, b5);
        }//end else
    }//end if
    S = x_i2b(1);
    if(s5 > 0) {
        S = x_pow5mult(S, s5);
    }//end if

    /* Check for special case that d is a normalized power of 2. */

    spec_case = 0;
    if(mode < 2) {
        if(bbits == 1 && be0 > fpi->emin + 1) {
            /* The special case */
            b2++;
            s2++;
            spec_case = 1;
        }//end if
    }//end if

    /* Arrange for convenient computation of quotients:
     * shift left if necessary so divisor has 4 leading 0 bits.
     *
     * Perhaps we should just compute leading 28 bits of S once
     * and for all and pass them and a shift to x_quorem, so it
     * can do shifts and ors to compute the numerator for q.
     */
    i = ((s5 ? x_hi0bits(S->x[S->wds - 1]) : ULbits - 1) - s2 - 4) & kmask;
    m2 += i;
    if((b2 += i) > 0) {
        b = x_lshift(b, b2);
    }//end if
    if((s2 += i) > 0) {
        S = x_lshift(S, s2);
    }//end if
    if(k_check) {
        if(x_cmp(b, S) < 0) {
            k--;
            b = x_multadd(b, 10, 0);    /* we botched the k estimate */
            if(leftright) {
                mhi = x_multadd(mhi, 10, 0);
            }//end if
            ilim = ilim1;
        }//end if
    }//end if
    if(ilim <= 0 && mode > 2) {
        if(ilim < 0 || x_cmp(b, S = x_multadd(S, 5, 0)) <= 0) {
            /* no digits, fcvt style */
no_digits:
            k = -1 - ndigits;
            inex = x_STRTOG_Inexlo;
            goto ret;
        }//end if
one_digit:
        inex = x_STRTOG_Inexhi;
        *s++ = '1';
        k++;
        goto ret;
    }//end if
    if(leftright) {
        if(m2 > 0) {
            mhi = x_lshift(mhi, m2);
        }//end if

        /* Compute mlo -- check for special case
         * that d is a normalized power of 2.
         */

        mlo = mhi;
        if(spec_case) {
            mhi = x_Balloc(mhi->k);
            x_Bcopy(mhi, mlo);
            mhi = x_lshift(mhi, 1);
        }//end if

        for(i = 1;;i++) {
            dig = x_quorem(b,S) + '0';
            /* Do we yet have the shortest decimal string
             * that will round to d?
             */
            j = x_cmp(b, mlo);
            delta = x_diff(S, mhi);
            j1 = delta->sign ? 1 : x_cmp(b, delta);
            x_Bfree(delta);
#if !defined(ROUND_BIASED)
            if(j1 == 0 && !mode && !(bits[0] & 1) && !rdir) {
                if(dig == '9') {
                    goto round_9_up;
                }//end if
                if(j <= 0) {
                    if(b->wds > 1 || b->x[0]) {
                        inex = x_STRTOG_Inexlo;
                    }//end if
                }//end if
                else {
                    dig++;
                    inex = x_STRTOG_Inexhi;
                }//end else
                *s++ = dig;
                goto ret;
            }//end if
#endif /* !!defined(ROUND_BIASED) */
            if(j < 0 || (j == 0 && !mode
#if !defined(ROUND_BIASED)
                && !(bits[0] & 1)
#endif /* !!defined(ROUND_BIASED) */
                )) {
                if(rdir && (b->wds > 1 || b->x[0])) {
                    if(rdir == 2) {
                        inex = x_STRTOG_Inexlo;
                        goto accept;
                    }//end if
                    while(x_cmp(S, mhi) > 0) {
                        *s++ = dig;
                        mhi1 = x_multadd(mhi, 10, 0);
                        if(mlo == mhi) {
                            mlo = mhi1;
                        }//end if
                        mhi = mhi1;
                        b = x_multadd(b, 10, 0);
                        dig = x_quorem(b,S) + '0';
                    }//end while
                    if(dig++ == '9') {
                        goto round_9_up;
                    }//end if
                    inex = x_STRTOG_Inexhi;
                    goto accept;
                }//end if
                if(j1 > 0) {
                    b = x_lshift(b, 1);
                    j1 = x_cmp(b, S);
                    if((j1 > 0 || (j1 == 0 && dig & 1)) && dig++ == '9') {
                        goto round_9_up;
                    }//end if
                    inex = x_STRTOG_Inexhi;
                }//end if
                if(b->wds > 1 || b->x[0]) {
                    inex = x_STRTOG_Inexlo;
                }//end if
accept:
                *s++ = dig;
                goto ret;
            }//end if
            if(j1 > 0 && rdir != 2) {
                if(dig == '9') { /* possible if i == 1 */
round_9_up:
                    *s++ = '9';
                    inex = x_STRTOG_Inexhi;
                    goto roundoff;
                }//end if
                inex = x_STRTOG_Inexhi;
                *s++ = dig + 1;
                goto ret;
            }//end if
            *s++ = dig;
            if(i == ilim) {
                break;
            }//end if
            b = x_multadd(b, 10, 0);
            if(mlo == mhi) {
                mlo = mhi = x_multadd(mhi, 10, 0);
            }//end if
            else {
                mlo = x_multadd(mlo, 10, 0);
                mhi = x_multadd(mhi, 10, 0);
            }//end else
        }//end for
    }//end if
    else {
        for(i = 1;; i++) {
            *s++ = dig = x_quorem(b,S) + '0';
            if(i >= ilim) {
                break;
            }//end if
            b = x_multadd(b, 10, 0);
        }//end for
    }//end else

    /* Round off last digit */

    if(rdir) {
        if(rdir == 2 || (b->wds <= 1 && !b->x[0])) {
            goto chopzeros;
        }//end if
        goto roundoff;
    }//end if
    b = x_lshift(b, 1);
    j = x_cmp(b, S);
    if(j > 0 || (j == 0 && dig & 1)) {
roundoff:
        inex = x_STRTOG_Inexhi;
        while(*--s == '9') {
            if(s == s0) {
                k++;
                *s++ = '1';
                goto ret;
            }//end if
        }//end while
        ++*s++;
    }//end if
    else {
chopzeros:
        if(b->wds > 1 || b->x[0]) {
            inex = x_STRTOG_Inexlo;
        }//end if
        while(*--s == '0') {
        }//end while
        ++s;
    }//end else
ret:
    x_Bfree(S);
    if(mhi) {
        if(mlo && mlo != mhi) {
            x_Bfree(mlo);
        }//end if
        x_Bfree(mhi);
    }//end if
ret1:
    x_Bfree(b);
    *s = 0;
    *decpt = k + 1;
    if(rve) {
        *rve = s;
    }//end if
    *kindp |= inex;
    return s0;
}//end x_gdtoa
