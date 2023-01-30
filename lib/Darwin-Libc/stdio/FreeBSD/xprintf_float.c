/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/xprintf_float.c
 *
 */


#include <string.h>
#include <assert.h>
#include <locale.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include <libxprintf/libxprintf.h>

#include "x_gdtoa.h"
#include "x_floatio.h"
#include "x_xprintf_private.h"

/*
 * The size of the buffer we use as scratch space for integer
 * conversions, among other things.  Technically, we would need the
 * most space for base 10 conversions with thousands' grouping
 * characters between each pair of digits.  100 bytes is a
 * conservative overestimate even for a 128-bit uintmax_t.
 */
#define X_BUF       100

#define X_DEFPREC   6    /* Default FP precision */


/* various globals ---------------------------------------------------*/


/* padding function---------------------------------------------------*/

#define X_PRINTANDPAD(p, ep, len, with)             \
    do {                                            \
        n2 = (ep) - (p);                            \
        if(n2 > (len)) {                            \
            n2 = (len);                             \
        }                                           \
        if(n2 > 0) {                                \
            ret += x___printf_puts(io, (p), n2);    \
        }                                           \
        ret += x___printf_pad(io, (len) - (n2 > 0 ? n2 : 0), (with)); \
    } while(0)

/* misc --------------------------------------------------------------*/

extern const char *x___fix_nogrouping(const char *str);

#define x_to_char(n) ((n) + '0')

static int x_exponent(char *p0, int expo, int fmtch) {
    char *p, *t;
    char expbuf[X_MAXEXPDIG];

    p = p0;
    *p++ = fmtch;
    if(expo < 0) {
        expo = -expo;
        *p++ = '-';
    }//end if
    else {
        *p++ = '+';
    }//end else

    t = expbuf + X_MAXEXPDIG;
    if(expo > 9) {
        do {
            *--t = x_to_char(expo % 10);
        } while((expo /= 10) > 9);
        *--t = x_to_char(expo);
        for(; t < expbuf + X_MAXEXPDIG; *p++ = *t++) {
        }//end for
    }//end if
    else {
        /*
         * Exponents for decimal floating point conversions
         * (%[eEgG]) must be at least two characters long,
         * whereas x_exponents for hexadecimal conversions can
         * be only one character long.
         */
        if(fmtch == 'e' || fmtch == 'E') {
            *p++ = '0';
        }//end if
        *p++ = x_to_char(expo);
    }//end else

    return p - p0;
}//end x_exponent

/* 'f' ---------------------------------------------------------------*/

int x___printf_arginfo_float(const struct x_printf_info *pi, size_t n, int *argt) {
#if defined(X_VECTORS)
    if(pi->is_vec) {
        argt[0] = X_PA_VECTOR;
    }//end if
    else {
#endif /* !defined(X_VECTORS) */
        argt[0] = X_PA_DOUBLE;
        if(pi->is_long_double) {
            argt[0] |= X_PA_FLAG_LONG_DOUBLE;
        }//end if
#if defined(X_VECTORS)
    }//end else
#endif /* !defined(X_VECTORS) */

    return 1;
}//end x___printf_arginfo_float

/*
 * We can decompose the printed representation of floating
 * point numbers into several parts, some of which may be empty:
 *
 * [+|-| ] [0x|0X] MMM . NNN [e|E|p|P] [+|-] ZZ
 *    A       B     ---C---      D       E   F
 *
 * A:    'sign' holds this value if present; '\0' otherwise
 * B:    ox[1] holds the 'x' or 'X'; '\0' if not hexadecimal
 * C:    cp points to the string MMMNNN.  Leading and trailing
 *    zeros are not in the string and must be added.
 * D:    expchar holds this character; '\0' if no x_exponent, e.g. %f
 * F:    at least two digits for decimal, at least one digit for hex
 */

int x___printf_render_float(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    int         decimal_point_len;      /* length of locale specific decimal point */
    int         dprec;                  /* a copy of prec if [diouxX], 0 otherwise */
    int         expsize;                /* character count for expstr */
    int         expt;                   /* integer value of x_exponent */
    int         flag;
    int         lead;                   /* sig figs before decimal or group sep */
    int         n2;                     /* XXX: for X_PRINTANDPAD */
    int         ndig;                   /* actual number of digits returned by dtoa */
    int         nrepeats;               /* number of repeats of the last group */
    int         nseps;                  /* number of group separators with ' */
    int         prec;                   /* precision from format; <0 for N/A */
#if 0
    int         prsize;                 /* max size of printed field */
#endif /* 0 */
    int         realsz;                 /* field size expanded by dprec, sign, etc */
    int         ret;                    /* return value accumulator */
    int         signflag;               /* true if float is negative */
    int         size;                   /* size of converted field or string */
    int         thousands_sep_len;      /* length of locale specific thousands separator */
    char        *cp;
    char        *dtoaend;               /* pointer to end of converted digits */
    char        *dtoaresult;            /* buffer allocated by dtoa */
    char        buf[X_BUF];             /* buffer with space for digits of uintmax_t */
    char        expchar;                /* x_exponent character: [eEpP\0] */
    char        expstr[X_MAXEXPDIG+2];  /* buffer for x_exponent string: e+ZZZ */
    char        ox[2];                  /* space for 0x; ox[1] is either x, X, or \0 */
    char        sign;                   /* sign prefix (' ', '+', '-', or \0) */
    double      d;
    const char  *decimal_point;         /* locale specific decimal point */
    const char  *grouping;              /* locale specific numeric grouping rules */
    const char  *thousands_sep;         /* locale specific thousands separator */
    const char  *xdigs;
    long double ld;

#if defined(X_VECTORS)
    if(pi->is_vec) {
        ret = x___xprintf_vector(io, pi, arg);
        x___printf_flush(io);
        return ret;
    }//end if
#endif /* !defined(X_VECTORS) */

    prec = pi->prec;
    ox[1] = '\0';
    sign = pi->signchar;
    flag = 0;
    ret = 0;

    thousands_sep = localeconv()->thousands_sep;
    thousands_sep_len = strlen(thousands_sep);
    grouping = NULL;
    if(pi->group) {
        grouping = x___fix_nogrouping(localeconv()->grouping);
    }//end if
    decimal_point = localeconv()->decimal_point;
    decimal_point_len = strlen(decimal_point);
    dprec = -1;

    switch(pi->spec) {
    case 'a':
    case 'A':
        if(pi->spec == 'a') {
            ox[1] = 'x';
            xdigs = x___lowercase_hex;
            expchar = 'p';
        }//end if
        else {
            ox[1] = 'X';
            xdigs = x___uppercase_hex;
            expchar = 'P';
        }//end else
        if(prec >= 0) {
            prec++;
        }//end if
        if(pi->is_long_double) {
            ld = *((long double *)arg[0]);
            dtoaresult = cp = x___hldtoa(ld, xdigs, prec, &expt, &signflag, &dtoaend);
        }//end if
        else {
            d = *((double *)arg[0]);
            dtoaresult = cp = x___hdtoa(d, xdigs, prec, &expt, &signflag, &dtoaend);
        }//end else
        if(prec < 0) {
            prec = dtoaend - cp;
        }//end if
        if(expt == INT_MAX) {
            ox[1] = '\0';
        }//end if
        goto fp_common;
    case 'e':
    case 'E':
        expchar = pi->spec;
        if(prec < 0) {   /* account for digit before decpt */
            prec = X_DEFPREC + 1;
        }//end if
        else {
            prec++;
        }//end else
        break;
    case 'f':
    case 'F':
        expchar = '\0';
        break;
    case 'g':
    case 'G':
        expchar = pi->spec - ('g' - 'e');
        if(prec == 0) {
            prec = 1;
        }//end if
        break;
    default:
        assert(pi->spec == 'f');
    }//end switch

    if(prec < 0) {
        prec = X_DEFPREC;
    }//end if
    if(pi->is_long_double) {
        ld = *((long double *)arg[0]);
        dtoaresult = cp = x___ldtoa(&ld, expchar ? 2 : 3, prec, &expt, &signflag, &dtoaend);
    }//end if
    else {
        d = *((double *)arg[0]);
        dtoaresult = cp = x_dtoa(d, expchar ? 2 : 3, prec, &expt, &signflag, &dtoaend);
        if(expt == 9999) {
            expt = INT_MAX;
        }//end if
    }//end else
fp_common:
    if(signflag) {
        sign = '-';
    }//end if
    if(expt == INT_MAX) {    /* inf or nan */
        if(*cp == 'N') {
            cp = (pi->spec >= 'a') ? "nan" : "NAN";
            sign = '\0';
        }//end if
        else {
            cp = (pi->spec >= 'a') ? "inf" : "INF";
        }//end else
        size = 3;
        flag = 1;
        goto here;
    }//end if
    ndig = dtoaend - cp;
    if(pi->spec == 'g' || pi->spec == 'G') {
        if(expt > -4 && expt <= prec) {
            /* Make %[gG] smell like %[fF] */
            expchar = '\0';
            if(pi->alt) {
                prec -= expt;
            }//end if
            else {
                prec = ndig - expt;
            }//end else
            if(prec < 0) {
                prec = 0;
            }//end if
        }//end if
        else {
            /*
             * Make %[gG] smell like %[eE], but
             * trim trailing zeroes if no # flag.
             */
            if(!pi->alt) {
                prec = ndig;
            }//end if
        }//end else
    }//end if

    if(expchar) {
        expsize = x_exponent(expstr, expt - 1, expchar);
        size = expsize + prec;
        if(prec > 1 || pi->alt) {
            ++size;
        }//end if
    }//end if
    else {
        /* space for digits before decimal point */
        if(expt > 0) {
            size = expt;
        }//end if
        else {   /* "0" */
            size = 1;
        }//end else
        /* space for decimal pt and following digits */
        if(prec || pi->alt) {
            size += prec + 1;
        }//end if
        if(grouping && expt > 0) {
            /* space for thousands' grouping */
            nseps = nrepeats = 0;
            lead = expt;
            while(*grouping != CHAR_MAX) {
                if(lead <= *grouping)
                    break;
                lead -= *grouping;
                if(*(grouping+1)) {
                    nseps++;
                    grouping++;
                }//end if
                else {
                    nrepeats++;
                }//end else
            }//end if
            size += nseps + nrepeats;
        }//end if
        else {
            lead = expt;
        }//end else
    }//end else

here:
    /*
     * All reasonable formats wind up here.  At this point, `cp'
     * points to a string which (if not flags&LADJUST) should be
     * padded out to `width' places.  If flags&ZEROPAD, it should
     * first be prefixed by any sign or other prefix; otherwise,
     * it should be blank padded before the prefix is emitted.
     * After any left-hand padding and prefixing, emit zeroes
     * required by a decimal [diouxX] precision, then print the
     * string proper, then emit zeroes required by any leftover
     * floating precision; finally, if LADJUST, pad with blanks.
     *
     * Compute actual size, so we know how much to pad.
     * size excludes decimal prec; realsz includes it.
     */
    realsz = dprec > size ? dprec : size;
    if(sign) {
        realsz++;
    }//end if
    if(ox[1]) {
        realsz += 2;
    }//end if

#if 0
    prsize = pi->width > realsz ? pi->width : realsz;
#endif /* 0 */

    /* right-adjusting blank padding */
    if(pi->pad != '0' && pi->left == 0) {
        ret += x___printf_pad(io, pi->width - realsz, 0);
    }//end if

    /* prefix */
    if(sign) {
        ret += x___printf_puts(io, &sign, 1);
    }//end if

    if(ox[1]) {    /* ox[1] is either x, X, or \0 */
        ox[0] = '0';
        ret += x___printf_puts(io, ox, 2);
    }//end if

    /* right-adjusting zero padding */
    if(pi->pad == '0' && pi->left == 0) {
        ret += x___printf_pad(io, pi->width - realsz, 1);
    }//end if

    /* leading zeroes from decimal precision */
    ret += x___printf_pad(io, dprec - size, 1);

    if(flag) {
        ret += x___printf_puts(io, cp, size);
    }//end if
    else {
        /* glue together f_p fragments */
        if(!expchar) {    /* %[fF] or sufficiently short %[gG] */
            if(expt <= 0) {
                ret += x___printf_puts(io, "0", 1);
                if(prec || pi->alt) {
                    ret += x___printf_puts(io, decimal_point, decimal_point_len);
                }//end if
                ret += x___printf_pad(io, -expt, 1);
                /* already handled initial 0's */
                prec += expt;
            }//end if
            else {
                X_PRINTANDPAD(cp, dtoaend, lead, 1);
                cp += lead;
                if(grouping) {
                    while(nseps > 0 || nrepeats > 0) {
                        if(nrepeats > 0) {
                            nrepeats--;
                        }//end if
                        else {
                            grouping--;
                            nseps--;
                        }//end else
                        ret += x___printf_puts(io, thousands_sep, thousands_sep_len);
                        X_PRINTANDPAD(cp,dtoaend, *grouping, 1);
                        cp += *grouping;
                    }//end while
                    if(cp > dtoaend) {
                        cp = dtoaend;
                    }//end if
                }//end if
                if(prec || pi->alt) {
                    ret += x___printf_puts(io, decimal_point, decimal_point_len);
                }//end if
            }//end else
            X_PRINTANDPAD(cp, dtoaend, prec, 1);
        }//end if
        else {    /* %[eE] or sufficiently long %[gG] */
            if(prec > 1 || pi->alt) {
                buf[0] = *cp++;
                memcpy(buf + 1, decimal_point, decimal_point_len);
                ret += x___printf_puts(io, buf, decimal_point_len + 1);
                ret += x___printf_puts(io, cp, ndig-1);
                ret += x___printf_pad(io, prec - ndig, 1);
            }//end if
            else {   /* XeYYY */
                ret += x___printf_puts(io, cp, 1);
            }//end if
            ret += x___printf_puts(io, expstr, expsize);
        }//end else
    }//end else

    /* left-adjusting padding (always blank) */
    if(pi->left) {
        ret += x___printf_pad(io, pi->width - realsz, 0);
    }//end if

    x___printf_flush(io);
    if(dtoaresult != NULL) {
        x_freedtoa(dtoaresult);
    }//end if

    return ret;
}//end x___printf_render_float
