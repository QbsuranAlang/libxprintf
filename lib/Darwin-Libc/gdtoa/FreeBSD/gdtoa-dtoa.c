/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gdtoa/FreeBSD/gdtoa-dtoa.c
 *
 */


#include "x_gdtoaimp.h"

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

#if defined(Honor_FLT_ROUNDS)
#undef Check_FLT_ROUNDS
#define Check_FLT_ROUNDS
#else /* defined(Honor_FLT_ROUNDS) */
#define Rounding Flt_Rounds
#endif /* !defined(Honor_FLT_ROUNDS) */

char *x_dtoa(double d0, int mode, int ndigits, int *decpt, int *sign, char **rve) {
    /* Arguments ndigits, decpt, sign are similar to those
    of ecvt and fcvt; trailing zeros are suppressed from
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
        4,5 ==> similar to 2 and 3, respectively, but (in
            round-nearest mode) with the tests of mode 0 to
            possibly return a shorter string that rounds to d.
            With IEEE arithmetic and compilation with
            -DHonor_FLT_ROUNDS, modes 4 and 5 behave the same
            as modes 2 and 3 when FLT_ROUNDS != 1.
        6-9 ==> Debugging modes similar to mode - 4:  don't try
            fast floating-point estimate (if applicable).

        Values of mode other than 0-9 are treated as mode 0.

        Sufficient space is allocated to the return value
        to hold the suppressed trailing zeros.
    */

    int         bbits, b2, b5, be, dig, i, ieps, ilim, ilim0, ilim1,
        j, j1, k, k0, k_check, leftright, m2, m5, s2, s5,
        spec_case, try_quick;
    Long        L;
#if !defined(Sudden_Underflow)
    int         denorm;
    ULong       x;
#endif /* !!defined(Sudden_Underflow) */
    U           d, d2, eps;
    char        *s, *s0;
    double      ds;
    x_Bigint    *b, *b1, *delta, *mlo, *mhi, *S;
#if defined(SET_INEXACT)
    int         inexact, oldinexact;
#endif /* !defined(SET_INEXACT) */
#if defined(Honor_FLT_ROUNDS)
    int         Rounding;
#if defined(Trust_FLT_ROUNDS) /* only define this if FLT_ROUNDS really works! */
    Rounding = Flt_Rounds;
#else /* defined(Trust_FLT_ROUNDS) */
    Rounding = 1;
    switch(fegetround()) {
    case FE_TOWARDZERO:
    Rounding = 0;
    break;
    case FE_UPWARD:
    Rounding = 2;
    break;
    case FE_DOWNWARD:
    Rounding = 3;
    break
    }//end switch
#endif /* !defined(Trust_FLT_ROUNDS) */
#endif /* !defined(Honor_FLT_ROUNDS) */

#if !defined(MULTIPLE_THREADS)
    if(x_dtoa_result) {
        x_freedtoa(x_dtoa_result);
        x_dtoa_result = NULL;
    }//end if
#endif /* !!defined(MULTIPLE_THREADS) */
    d.d = d0;
    if(x_word0(&d) & Sign_bit) {
        /* set sign for everything, including 0's and NaNs */
        *sign = 1;
        x_word0(&d) &= ~Sign_bit;    /* clear sign bit */
    }//end if
    else {
        *sign = 0;
    }//end else

#if defined(IEEE_Arith) + defined(VAX)
#if defined(IEEE_Arith)
    if((x_word0(&d) & Exp_mask) == Exp_mask)
#else /* defined(IEEE_Arith) */
    if(x_word0(&d)  == 0x8000)
#endif /* !defined(IEEE_Arith) */
        {
        /* Infinity or NaN */
        *decpt = 9999;
#if defined(IEEE_Arith)
        if(!x_word1(&d) && !(x_word0(&d) & 0xfffff)) {
            return x_nrv_alloc("Infinity", rve, 8);
        }//end if
#endif /* !defined(IEEE_Arith) */
        return x_nrv_alloc("NaN", rve, 3);
    }//end if
#endif /* !(defined(IEEE_Arith) + defined(VAX)) */
#if defined(IBM)
    x_dval(&d) += 0; /* normalize */
#endif /* !defined(IBM) */
    if(!x_dval(&d)) {
        *decpt = 1;
        return x_nrv_alloc("0", rve, 1);
    }//end if

#if defined(SET_INEXACT)
    try_quick = oldinexact = get_inexact();
    inexact = 1;
#endif /* !defined(SET_INEXACT) */
#if defined(Honor_FLT_ROUNDS)
    if(Rounding >= 2) {
        if(*sign) {
            Rounding = Rounding == 2 ? 0 : 2;
        }//end if
        else {
            if(Rounding != 2) {
                Rounding = 0;
            }//end if
        }//end else
    }//end if
#endif /* !defined(Honor_FLT_ROUNDS) */

    b = x_d2b(x_dval(&d), &be, &bbits);
#if defined(Sudden_Underflow)
    i = (int)(x_word0(&d) >> Exp_shift1 & (Exp_mask >> Exp_shift1));
#else /* defined(Sudden_Underflow) */
    if((i = (int)(x_word0(&d) >> Exp_shift1 & (Exp_mask >> Exp_shift1))) != 0) {
#endif /* !defined(Sudden_Underflow) */
        x_dval(&d2) = x_dval(&d);
        x_word0(&d2) &= Frac_mask1;
        x_word0(&d2) |= Exp_11;
#if defined(IBM)
        if((j = 11 - x_hi0bits(x_word0(&d2) & Frac_mask)) != 0) {
            x_dval(&d2) /= 1 << j;
        }//end if
#endif /* !defined(IBM) */

        /* log(x)    ~=~ log(1.5) + (x-1.5)/1.5
         * log10(x)     =  log(x) / log(10)
         *        ~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
         * log10(&d) = (i-Bias)*log(2)/log(10) + log10(&d2)
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

        i -= Bias;
#if defined(IBM)
        i <<= 2;
        i += j;
#endif /* !defined(IBM) */
#if !defined(Sudden_Underflow)
        denorm = 0;
    }//end if
    else {
        /* d is denormalized */

        i = bbits + be + (Bias + (P - 1) - 1);
        x = i > 32 ? x_word0(&d) << (64 - i) | x_word1(&d) >> (i - 32) : x_word1(&d) << (32 - i);
        x_dval(&d2) = x;
        x_word0(&d2) -= 31 * Exp_msk1; /* adjust exponent */
        i -= (Bias + (P - 1) - 1) + 1;
        denorm = 1;
    }//end else
#endif /* !!defined(Sudden_Underflow) */
    ds = (x_dval(&d2) - 1.5) * 0.289529654602168 + 0.1760912590558 + i * 0.301029995663981;
    k = (int)ds;
    if(ds < 0. && ds != k) {
        k--;    /* want k = floor(ds) */
    }//end if
    k_check = 1;
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

#if !defined(SET_INEXACT)
#if defined(Check_FLT_ROUNDS)
    try_quick = Rounding == 1;
#else /* defined(Check_FLT_ROUNDS) */
    try_quick = 1;
#endif /* !defined(Check_FLT_ROUNDS) */
#endif /* !!defined(SET_INEXACT) */

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
        i = 18;
        ndigits = 0;
        break;
    case 2:
        leftright = 0;
        /* no break */
    case 4:
        if(ndigits <= 0) {
            ndigits = 1;
        }//end if
        ilim = ilim1 = i = ndigits;
        break;
    case 3:
        leftright = 0;
        /* no break */
    case 5:
        i = ndigits + k + 1;
        ilim = i;
        ilim1 = i - 1;
        if(i <= 0) {
            i = 1;
        }//end if
    }//end switch
    s = s0 = x_rv_alloc(i);

#if defined(Honor_FLT_ROUNDS)
    if(mode > 1 && Rounding != 1) {
        leftright = 0;
    }//end if
#endif /* !defined(Honor_FLT_ROUNDS) */

    if(ilim >= 0 && ilim <= Quick_max && try_quick) {

        /* Try to get by with floating-point arithmetic. */

        i = 0;
        x_dval(&d2) = x_dval(&d);
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
            x_dval(&d) /= ds;
        }//end if
        else if((j1 = -k) != 0) {
            x_dval(&d) *= x_tens[j1 & 0xf];
            for(j = j1 >> 4; j; j >>= 1, i++) {
                if(j & 1) {
                    ieps++;
                    x_dval(&d) *= x_bigtens[i];
                }//end if
            }//end for
        }//end if
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
            x_dval(&eps) = 0.5 / x_tens[ilim - 1] - x_dval(&eps);
            for(i = 0;;) {
                L = x_dval(&d);
                x_dval(&d) -= L;
                *s++ = '0' + (int)L;
                if(x_dval(&d) < x_dval(&eps)) {
                    goto ret1;
                }//end if
                if(1. - x_dval(&d) < x_dval(&eps)) {
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
                L = (Long)(x_dval(&d));
                if(!(x_dval(&d) -= L)) {
                    ilim = i;
                }//end if
                *s++ = '0' + (int)L;
                if(i == ilim) {
                    if(x_dval(&d) > 0.5 + x_dval(&eps)) {
                        goto bump_up;
                    }//end if
                    else if(x_dval(&d) < 0.5 - x_dval(&eps)) {
                        while(*--s == '0') {
                        }//end while
                        s++;
                        goto ret1;
                    }//end if
                    break;
                }//end if
            }//end for
#if !defined(No_leftright)
        }//end else
#endif /* !!defined(No_leftright) */
fast_failed:
        s = s0;
        x_dval(&d) = x_dval(&d2);
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
            L = (Long)(x_dval(&d) / ds);
            x_dval(&d) -= L*ds;
#if defined(Check_FLT_ROUNDS)
            /* If FLT_ROUNDS == 2, L will usually be high by 1 */
            if(x_dval(&d) < 0) {
                L--;
                x_dval(&d) += ds;
            }//end if
#endif /* !defined(Check_FLT_ROUNDS) */
            *s++ = '0' + (int)L;
            if(!x_dval(&d)) {
#if defined(SET_INEXACT)
                inexact = 0;
#endif /* !defined(SET_INEXACT) */
                break;
            }//end if
            if(i == ilim) {
#if defined(Honor_FLT_ROUNDS)
                if(mode > 1) {
                    switch(Rounding) {
                    case 0: goto ret1;
                    case 2: goto bump_up;
                    }//end switch
                }//end if
#endif /* !defined(Honor_FLT_ROUNDS) */
                x_dval(&d) += x_dval(&d);
                if(x_dval(&d) > ds || (x_dval(&d) == ds && L & 1)) {
bump_up:
                    while(*--s == '9') {
                        if(s == s0) {
                            k++;
                            *s = '0';
                            break;
                        }//end if
                    }//end while
                    ++*s++;
                }//end if
                break;
            }//end if
        }//end for
        goto ret1;
    }//end if

    m2 = b2;
    m5 = b5;
    mhi = mlo = 0;
    if(leftright) {
        i =
#if !defined(Sudden_Underflow)
            denorm ? be + (Bias + (P - 1) - 1 + 1) :
#endif /* !!defined(Sudden_Underflow) */
#if defined(IBM)
            1 + 4 * P - 3 - bbits + ((bbits + be - 1) & 3);
#else /* defined(IBM) */
            1 + P - bbits;
#endif /* !defined(IBM) */
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
    if((mode < 2 || leftright)
#if defined(Honor_FLT_ROUNDS)
            && Rounding == 1
#endif /* !defined(Honor_FLT_ROUNDS) */
        ) {
        if(!x_word1(&d) && !(x_word0(&d) & Bndry_mask)
#if !defined(Sudden_Underflow)
         && x_word0(&d) & (Exp_mask & ~Exp_msk1)
#endif /* !!defined(Sudden_Underflow) */
            ) {
            /* The special case */
            b2 += Log2P;
            s2 += Log2P;
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
#if  defined(Pack_32)
    if((i = ((s5 ? 32 - x_hi0bits(S->x[S->wds - 1]) : 1) + s2) & 0x1f) != 0) {
        i = 32 - i;
    }//end if
#else /* defined(Pack_32) */
    if((i = ((s5 ? 32 - x_hi0bits(S->x[S->wds - 1]) : 1) + s2) & 0xf) != 0) {
        i = 16 - i;
    }//end if
#endif /* !defined(Pack_32) */
    if(i > 4) {
        i -= 4;
        b2 += i;
        m2 += i;
        s2 += i;
    }//end if
    else if(i < 4) {
        i += 28;
        b2 += i;
        m2 += i;
        s2 += i;
    }//end if
    if(b2 > 0) {
        b = x_lshift(b, b2);
    }//end if
    if(s2 > 0) {
        S = x_lshift(S, s2);
    }//end if
    if(k_check) {
        if(x_cmp(b,S) < 0) {
            k--;
            b = x_multadd(b, 10, 0);    /* we botched the k estimate */
            if(leftright) {
                mhi = x_multadd(mhi, 10, 0);
            }//end if
            ilim = ilim1;
        }//end if
    }//end if
    if(ilim <= 0 && (mode == 3 || mode == 5)) {
        if(ilim < 0 || x_cmp(b, S = x_multadd(S, 5, 0)) <= 0) {
            /* no digits, fcvt style */
no_digits:
            k = -1 - ndigits;
            goto ret;
        }//end if
one_digit:
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
            mhi = x_lshift(mhi, Log2P);
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
            if(j1 == 0 && mode != 1 && !(x_word1(&d) & 1)
#if defined(Honor_FLT_ROUNDS)
                && Rounding >= 1
#endif /* !defined(Honor_FLT_ROUNDS) */
                ) {
                if(dig == '9') {
                    goto round_9_up;
                }//end if
                if(j > 0) {
                    dig++;
                }//end if
#if defined(SET_INEXACT)
                else if(!b->x[0] && b->wds <= 1) {
                    inexact = 0;
                }//end if
#endif /* !defined(SET_INEXACT) */
                *s++ = dig;
                goto ret;
            }//end if
#endif /* !!defined(ROUND_BIASED) */
            if(j < 0 || (j == 0 && mode != 1
#if !defined(ROUND_BIASED)
                && !(x_word1(&d) & 1)
#endif /* !!defined(ROUND_BIASED) */
                )) {
                if(!b->x[0] && b->wds <= 1) {
#if defined(SET_INEXACT)
                    inexact = 0;
#endif /* !defined(SET_INEXACT) */
                    goto accept_dig;
                }//end if
#if defined(Honor_FLT_ROUNDS)
                if(mode > 1) {
                    switch(Rounding) {
                    case 0: goto accept_dig;
                    case 2: goto keep_dig;
                    }//end switch
                }//end if
#endif /* !defined(Honor_FLT_ROUNDS) */
                if(j1 > 0) {
                    b = x_lshift(b, 1);
                    j1 = x_cmp(b, S);
                    if((j1 > 0 || (j1 == 0 && dig & 1)) && dig++ == '9') {
                        goto round_9_up;
                    }//end if
                }//end if
accept_dig:
                *s++ = dig;
                goto ret;
            }//end if
            if(j1 > 0) {
#if defined(Honor_FLT_ROUNDS)
                if(!Rounding) {
                    goto accept_dig;
                }//end if
#endif /* !defined(Honor_FLT_ROUNDS) */
                if(dig == '9') { /* possible if i == 1 */
round_9_up:
                    *s++ = '9';
                    goto roundoff;
                }//end if
                *s++ = dig + 1;
                goto ret;
            }//end if
#if defined(Honor_FLT_ROUNDS)
keep_dig:
#endif /* !defined(Honor_FLT_ROUNDS) */
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
            if(!b->x[0] && b->wds <= 1) {
#if defined(SET_INEXACT)
                inexact = 0;
#endif /* !defined(SET_INEXACT) */
                goto ret;
            }//end if
            if(i >= ilim) {
                break;
            }//end if
            b = x_multadd(b, 10, 0);
        }//end for
    }//end else

    /* Round off last digit */

#if defined(Honor_FLT_ROUNDS)
    switch(Rounding) {
    case 0: goto trimzeros;
    case 2: goto roundoff;
    }//end switch
#endif /* !defined(Honor_FLT_ROUNDS) */
    b = x_lshift(b, 1);
    j = x_cmp(b, S);
    if(j > 0 || (j == 0 && dig & 1)) {
roundoff:
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
#if defined(Honor_FLT_ROUNDS)
trimzeros:
#endif /* !defined(Honor_FLT_ROUNDS) */
        while(*--s == '0') {
        }//end while
        s++;
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
#if defined(SET_INEXACT)
    if(inexact) {
        if(!oldinexact) {
            x_word0(&d) = Exp_1 + (70 << Exp_shift);
            x_word1(&d) = 0;
            x_dval(&d) += 1.;
        }//end if
    }//end if
    else if(!oldinexact) {
        clear_inexact();
    }//end if
#endif /* !defined(SET_INEXACT) */
    x_Bfree(b);
    *s = 0;
    *decpt = k + 1;
    if(rve) {
        *rve = s;
    }//end if
    return s0;
}//end x_dtoa
