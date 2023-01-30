/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/xprintf_int.c
 *
 */


#include <stdint.h>
#include <sys/types.h>
#include <err.h>
#include <limits.h>
#include <locale.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <libxprintf/libxprintf.h>

#include "x_xprintf_private.h"

/* private stuff -----------------------------------------------------*/

union x_arg {
    int         intarg;
    u_int       uintarg;
    long        longarg;
    u_long      ulongarg;
    intmax_t    intmaxarg;
    uintmax_t   uintmaxarg;
};

/*
 * Macros for converting digits to letters and vice versa
 */
#define x_to_char(n) ((n) + '0')

/* various globals ---------------------------------------------------*/

/*
 * The size of the buffer we use for integer conversions.
 * Technically, we would need the most space for base 10
 * conversions with thousands' grouping characters between
 * each pair of digits: 39 digits for 128 bit intmax_t plus
 * 20 grouping characters (which may be multibyte).
 * Use a bit more for better alignment of stuff.
 */
#define X_BUF 128

/* misc --------------------------------------------------------------*/

extern const char *x___fix_nogrouping(const char *str);

/*
 * Convert an unsigned long to ASCII for printf purposes, returning
 * a pointer to the first character of the string representation.
 * Octal numbers can be forced to have a leading zero; hex numbers
 * use the given digits.
 */
static char *x___ultoa(u_long val, char *endp, int base, const char *xdigs, int needgrp, const char *thousep, int thousep_len, const char *grp) {
    int     ndig;
    char    *cp;
    long    sval;

    cp = endp;

    /*
     * Handle the three cases separately, in the hope of getting
     * better/faster code.
     */
    switch(base) {
    case 10:
        if(val < 10) {    /* many numbers are 1 digit */
            *--cp = x_to_char(val);
            return cp;
        }//end if
        ndig = 0;
        /*
         * On many machines, unsigned arithmetic is harder than
         * signed arithmetic, so we do at most one unsigned mod and
         * divide; this is sufficient to reduce the range of
         * the incoming value to where signed arithmetic works.
         */
        if(val > LONG_MAX) {
            *--cp = x_to_char(val % 10);
            ndig++;
            sval = val / 10;
        }//end if
        else {
            sval = val;
        }//en delse
        do {
            *--cp = x_to_char(sval % 10);
            ndig++;
            /*
             * If (*grp == CHAR_MAX) then no more grouping
             * should be performed.
             */
            if(needgrp && ndig == *grp && *grp != CHAR_MAX && sval > 9) {
                cp -= thousep_len;
                memcpy(cp, thousep, thousep_len);
                ndig = 0;
                /*
                 * If (*(grp+1) == '\0') then we have to
                 * use *grp character (last grouping rule)
                 * for all next cases
                 */
                if(*(grp+1) != '\0') {
                    grp++;
                }//end if
            }//end if
            sval /= 10;
        } while(sval != 0);
        break;

    case 8:
        do {
            *--cp = x_to_char(val & 7);
            val >>= 3;
        } while(val);
        break;

    case 16:
        do {
            *--cp = xdigs[val & 15];
            val >>= 4;
        } while(val);
        break;

    default:            /* oops */
        assert(base == 16);
    }//end switch

    return cp;
}//end x___ultoa


/* Identical to x___ultoa, but for intmax_t. */
static char *x___ujtoa(uintmax_t val, char *endp, int base, const char *xdigs,  int needgrp, const char *thousep, int thousep_len, const char *grp) {
    int         ndig;
    char        *cp;
    intmax_t    sval;

    cp = endp;

    switch(base) {
    case 10:
        if(val < 10) {
            *--cp = x_to_char(val % 10);
            return cp;
        }//end if
        ndig = 0;
        if(val > INTMAX_MAX) {
            *--cp = x_to_char(val % 10);
            ndig++;
            sval = val / 10;
        }//end if
        else {
            sval = val;
        }//end else
        do {
            *--cp = x_to_char(sval % 10);
            ndig++;
            /*
             * If (*grp == CHAR_MAX) then no more grouping
             * should be performed.
             */
            if(needgrp && *grp != CHAR_MAX && ndig == *grp && sval > 9) {
                cp -= thousep_len;
                memcpy(cp, thousep, thousep_len);
                ndig = 0;
                /*
                 * If (*(grp+1) == '\0') then we have to
                 * use *grp character (last grouping rule)
                 * for all next cases
                 */
                if(*(grp+1) != '\0') {
                    grp++;
                }//end if
            }//end if
            sval /= 10;
        } while(sval != 0);
        break;

    case 8:
        do {
            *--cp = x_to_char(val & 7);
            val >>= 3;
        } while(val);
        break;

    case 16:
        do {
            *--cp = xdigs[val & 15];
            val >>= 4;
        } while(val);
        break;

    default:
        abort();
    }//end switch

    return cp;
}//end x___ujtoa


/* 'd' ---------------------------------------------------------------*/

int x___printf_arginfo_int(const struct x_printf_info *pi, size_t n, int *argt) {

    argt[0] = X_PA_INT;
#if defined(X_VECTORS)
    if(pi->is_vec) {
        argt[0] = X_PA_VECTOR;
    }//end if
    else
#endif /* !defined(X_VECTORS) */
    if(pi->is_ptrdiff) {
        argt[0] |= X_PA_FLAG_PTRDIFF;
    }//end if
    else if(pi->is_size) {
        argt[0] |= X_PA_FLAG_SIZE;
    }//end if
    else if(pi->is_long) {
        argt[0] |= X_PA_FLAG_LONG;
    }//end if
    else if(pi->is_intmax) {
        argt[0] |= X_PA_FLAG_INTMAX;
    }//end if
    else if(pi->is_quad) {
        argt[0] |= X_PA_FLAG_QUAD;
    }//end if
    else if(pi->is_long_double) {
        argt[0] |= X_PA_FLAG_LONG_LONG;
    }//end if
    else if(pi->is_short) {
        argt[0] |= X_PA_FLAG_SHORT;
    }//end if
    else if(pi->is_char) {
        argt[0] = X_PA_CHAR;
    }//end if

    return 1;
}//end x___printf_arginfo_int

int x___printf_render_int(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    int                 rdx, sign, zext, ngrp, l;
    int                 ret;
    int                 thousands_sep_len;        /* locale specific thousands separator length */
    char                *p, *pe;
    char                buf[X_BUF];
    char                ns;
    uintmax_t           uu;
    const char          *grouping;    /* locale specific numeric grouping rules */
    const char          *nalt, *digit;
    const char          *thousands_sep;    /* locale specific thousands separator */
    const union x_arg   *argp;

#if defined(X_VECTORS)
    if(pi->is_vec) {
        ret = x___xprintf_vector(io, pi, arg);
        x___printf_flush(io);
        return ret;
    }//end if
#endif /* !defined(X_VECTORS) */

    ret = 0;
    nalt = NULL;
    digit = x___lowercase_hex;
    ns = '\0';
    pe = buf + sizeof buf - 1;

    if(pi->group) {
        thousands_sep = localeconv()->thousands_sep;
        thousands_sep_len = strlen(thousands_sep);
        grouping = x___fix_nogrouping(localeconv()->grouping);
        ngrp = 1;
    }//end if
    else {
        thousands_sep = NULL;
        thousands_sep_len = 0;
        grouping = NULL;
        ngrp = 0;
    }//end else

    switch(pi->spec) {
    case 'd':
    case 'i':
        rdx = 10;
        sign = 1;
        break;
    case 'X':
        digit = x___uppercase_hex;
        /* fallthrough */
    case 'x':
        rdx = 16;
        sign = 0;
        break;
    case 'u':
    case 'U':
        rdx = 10;
        sign = 0;
        break;
    case 'o':
    case 'O':
        rdx = 8;
        sign = 0;
        break;
    default:
        fprintf(stderr, "pi->spec = '%c'\n", pi->spec);
        assert(1 == 0);
    }//end switch

    argp = arg[0];

    if(sign) {
        ns = pi->signchar;
    }//end if

    if(pi->is_long_double || pi->is_quad || pi->is_intmax || pi->is_size || pi->is_ptrdiff) {
        if(sign && argp->intmaxarg < 0) {
            uu = -argp->intmaxarg;
            ns = '-';
        }//end if
        else {
            uu = argp->uintmaxarg;
        }//end else
    }//end if
    else if(pi->is_long) {
        if(sign && argp->longarg < 0) {
            uu = (u_long)-argp->longarg;
            ns = '-';
        }//end if
        else {
            uu = argp->ulongarg;
        }//end else
    }//end if
    else if(pi->is_short) {
        if(sign && (short)argp->intarg < 0) {
            uu = -(short)argp->intarg;
            ns = '-';
        }//end if
        else {
            uu = (unsigned short)argp->uintarg;
        }//end else
    }//end if
    else if(pi->is_char) {
        if(sign && (signed char)argp->intarg < 0) {
            uu = -(signed char)argp->intarg;
            ns = '-';
        }//end if
        else {
            uu = (unsigned char)argp->uintarg;
        }//end else
    }//end if
    else {
        if(sign && argp->intarg < 0) {
            uu = (unsigned)-argp->intarg;
            ns = '-';
        }//end if
        else {
            uu = argp->uintarg;
        }//end else
    }//end else

    if(uu <= ULONG_MAX) {
        p = x___ultoa(uu, pe, rdx, digit, ngrp, thousands_sep, thousands_sep_len, grouping);
    }//end if
    else {
        p = x___ujtoa(uu, pe, rdx, digit, ngrp, thousands_sep, thousands_sep_len, grouping);
    }//end else

    l = 0;
    if(uu == 0) {
        /*-
         * ``The result of converting a zero value with an
         * explicit precision of zero is no characters.''
         *      -- ANSI X3J11
         *
         * ``The C Standard is clear enough as is.  The call
         * printf("%#.0o", 0) should print 0.''
         *      -- Defect Report #151
         */
        if(pi->prec == 0 && !(pi->alt && rdx == 8)) {
            p = pe;
        }//end if
    }//end if
    else if(pi->alt) {
        if(rdx == 8) {
            *--p = '0';
        }//end if
        if(rdx == 16) {
            if(pi->spec == 'x') {
                nalt = "0x";
            }//end if
            else {
                nalt = "0X";
            }//end else
            l += 2;
        }//end if
    }//end if

    l += pe - p;
    if(ns) {
        l++;
    }//end if

    /*-
     * ``... diouXx conversions ... if a precision is
     * specified, the 0 flag will be ignored.''
     *      -- ANSI X3J11
     */
    if(pi->prec > (pe - p)) {
        zext = pi->prec - (pe - p);
    }//end if
    else if(pi->prec != -1) {
        zext = 0;
    }//end if
    else if(pi->pad == '0' && pi->width > l && !pi->left) {
        zext = pi->width - l;
    }//end if
    else {
        zext = 0;
    }//end else

    l += zext;

    while(zext > 0 && p > buf) {
        *--p = '0';
        zext--;
    }//end while

    if(l < X_BUF) {
        if(ns) {
            *--p = ns;
        }//end if
        else if(nalt != NULL) {
            *--p = nalt[1];
            *--p = nalt[0];
        }//end if
        if(pi->width > (pe - p) && !pi->left) {
            l = pi->width - (pe - p);
            while(l > 0 && p > buf) {
                *--p = ' ';
                l--;
            }//end while
            if(l) {
                ret += x___printf_pad(io, l, 0);
            }//end if
        }//end if
    }//end if
    else {
        if(!pi->left && pi->width > l) {
            ret += x___printf_pad(io, pi->width - l, 0);
        }//end if
        if(ns != '\0') {
            ret += x___printf_puts(io, &ns, 1);
        }//end if
        else if(nalt != NULL) {
            ret += x___printf_puts(io, nalt, 2);
        }//end if
        if(zext > 0) {
            ret += x___printf_pad(io, zext, 1);
        }//end if
    }//end else

    ret += x___printf_puts(io, p, pe - p);
    if(pi->width > ret && pi->left) {
        ret += x___printf_pad(io, pi->width - ret, 0);
    }//end if
    x___printf_flush(io);

    return ret;
}//end x___printf_render_int

/* 'p' ---------------------------------------------------------------*/

int x___printf_arginfo_ptr(const struct x_printf_info *pi, size_t n, int *argt) {
#if defined(X_VECTORS)
    if(pi->is_vec) {
        argt[0] = X_PA_VECTOR;
    }//end if
    else
#endif /* !defined(X_VECTORS) */
    {
        argt[0] = X_PA_POINTER;
    }//end else

    return 1;
}//end x___printf_arginfo_ptr

int x___printf_render_ptr(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    int                     ret;
    uintmax_t               u;
    const void              *p;
    struct x_printf_info    p2;

#if defined(X_VECTORS)
    if(pi->is_vec) {
        ret = x___xprintf_vector(io, pi, arg);
        x___printf_flush(io);
        return ret;
    }//end if
#endif /* !defined(X_VECTORS) */

    /*-
     * ``The argument shall be a pointer to void.  The
     * value of the pointer is converted to a sequence
     * of printable characters, in an implementation-
     * defined manner.''
     *      -- ANSI X3J11
     */
    u = (uintmax_t)(uintptr_t)*((void **)arg[0]);

    p2 = *pi;

    p2.spec = 'x';
    p2.alt = 1;
    p2.is_long_double = 1;
    p = &u;
    ret = x___printf_render_int(io, &p2, &p);
    x___printf_flush(io);

    return ret;
}//end x___printf_render_ptr
