/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/xprintf_str.c
 *
 */


#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <wchar.h>

#include <libxprintf/libxprintf.h>

#include "x_xprintf_private.h"

/*
 * Convert a wide character string argument for the %ls format to a multibyte
 * string representation. If not -1, prec specifies the maximum number of
 * bytes to output, and also means that we can't assume that the wide char.
 * string ends is null-terminated.
 */
static char *x___wcsconv(wchar_t *wcsarg, int prec) {
    char                    buf[MB_LEN_MAX], *convbuf;
    size_t                  clen, nbytes;
    wchar_t                 *p;
    mbstate_t               mbs;
    static const mbstate_t  initial;

    /* Allocate space for the maximum number of bytes we could output. */
    if(prec < 0) {
        p = wcsarg;
        mbs = initial;
        nbytes = wcsrtombs(NULL, (const wchar_t **)&p, 0, &mbs);
        if(nbytes == (size_t)-1) {
            return NULL;
        }//end if
    }//end if
    else {
        /*
         * Optimisation: if the output precision is small enough,
         * just allocate enough memory for the maximum instead of
         * scanning the string.
         */
        if(prec < 128) {
            nbytes = prec;
        }//end if
        else {
            nbytes = 0;
            p = wcsarg;
            mbs = initial;
            for(;;) {
                clen = wcrtomb(buf, *p++, &mbs);
                if(clen == 0 || clen == (size_t)-1 || (int)(nbytes + clen) > prec) {
                    break;
                }//end if
                nbytes += clen;
            }//end for
        }//end else
    }//end else

    if((convbuf = X_MALLOC(nbytes + 1)) == NULL) {
        return NULL;
    }//end if

    /* Fill the output buffer. */
    p = wcsarg;
    mbs = initial;
    if((nbytes = wcsrtombs(convbuf, (const wchar_t **)&p, nbytes, &mbs)) == (size_t)-1) {
        free(convbuf);
        return NULL;
    }//end if
    convbuf[nbytes] = '\0';

    return convbuf;
}//end x___wcsconv

/* 's' ---------------------------------------------------------------*/

int x___printf_arginfo_str(const struct x_printf_info *pi, size_t n, int *argt) {

    if(pi->is_long || pi->spec == 'C') {
        argt[0] = X_PA_WSTRING;
    }//end if
    else {
        argt[0] = X_PA_STRING;
    }//end else

    return 1;
}//end x___printf_arginfo_str

int x___printf_render_str(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    int         l, ret;
    char        *convbuf;
    wchar_t     *wcp;
    const char  *p;

    if(pi->is_long || pi->spec == 'S') {
        wcp = (wchar_t *)*((wint_t **)arg[0]);
        if(wcp == NULL) {
            ret = x___printf_out(io, pi, "(null)", 6);
            x___printf_flush(io);
            return ret;
        }//end if
        convbuf = x___wcsconv(wcp, pi->prec);
        if(convbuf == NULL) {
            return -1;
        }//end if
        l = x___printf_out(io, pi, convbuf, strlen(convbuf));
        x___printf_flush(io);
        free(convbuf);
        return l;
    }//end if

    p = *((char **)arg[0]);
    if(p == NULL) {
        ret = x___printf_out(io, pi, "(null)", 6);
        x___printf_flush(io);
        return ret;
    }//end if

    l = strlen(p);
    if(pi->prec >= 0 && pi->prec < l) {
        l = pi->prec;
    }//end if

    ret = x___printf_out(io, pi, p, l);
    x___printf_flush(io);
    return ret;
}//end x___printf_render_str

/* 'c' ---------------------------------------------------------------*/

int x___printf_arginfo_chr(const struct x_printf_info *pi, size_t n, int *argt) {
#if defined(X_VECTORS)
    if(pi->is_vec) {
        argt[0] = X_PA_VECTOR;
    }//end if
    else
#endif /* !defined(X_VECTORS) */
    if(pi->is_long || pi->spec == 'C') {
        argt[0] = X_PA_WCHAR;
    }//end if
    else {
        argt[0] =X_PA_INT;
    }//end else
    return 1;
}//end x___printf_arginfo_chr

int x___printf_render_chr(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    int                     i, ret;
    char                    buf[MB_CUR_MAX];
    wint_t                  ii;
    size_t                  mbseqlen;
    mbstate_t               mbs;
    unsigned char           c;
    static const mbstate_t  initial;        /* XXX: this is bogus! */

#if defined(X_VECTORS)
    if(pi->is_vec) {
        ret = x___xprintf_vector(io, pi, arg);
        x___printf_flush(io);
        return ret;
    }//end if
#endif /* !defined(X_VECTORS) */

    if(pi->is_long || pi->spec == 'C') {

        ii = *((wint_t *)arg[0]);

        mbs = initial;
        mbseqlen = wcrtomb(buf, (wchar_t)ii, &mbs);
        if(mbseqlen == (size_t)-1) {
            return -1;
        }//end if

        ret = x___printf_out(io, pi, buf, mbseqlen);
        x___printf_flush(io);
        return ret;
    }//end if

    i = *((int *)arg[0]);
    c = i;
    i = x___printf_out(io, pi, &c, 1);
    x___printf_flush(io);

    return i;
}//end x___printf_render_chr
