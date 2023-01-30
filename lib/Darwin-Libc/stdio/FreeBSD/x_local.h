/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/local.h
 *
 */


#ifndef LIBXPRINTF_LOCAL_H
#define LIBXPRINTF_LOCAL_H

#if !defined(__USE_GNU)
#define __USE_GNU
#endif /* !!defined(__USE_GNU) */

#include <pthread.h>
#include <string.h>
#include <stdarg.h>

#include "x_xprintf_private.h"
#include "x_stdio.h"

/*
 * Information local to this implementation of stdio,
 * in particular, macros and private variables.
 */

X_LOCAL int x__swrite(x_FILE *, char const *, int);
X_LOCAL x_fpos_t x__sseek(x_FILE *, x_fpos_t, int);

X_LOCAL int x__vasprintf(x_printf_comp_t __restrict, x_printf_domain_t __restrict, char ** __restrict, const char * __restrict, va_list);
X_LOCAL int x__vdprintf(x_printf_comp_t __restrict, x_printf_domain_t __restrict, int, const char * __restrict, va_list);
X_LOCAL int x__vsnprintf(x_printf_comp_t __restrict, x_printf_domain_t __restrict, char * __restrict, size_t n, const char * __restrict, va_list);

X_LOCAL int x__lasprintf(x_printf_comp_t __restrict, x_printf_domain_t __restrict, char ** __restrict, const char * __restrict, void ** __restrict);
X_LOCAL int x__ldprintf(x_printf_comp_t __restrict, x_printf_domain_t __restrict, int, const char * __restrict, void ** __restrict);
X_LOCAL int x__lsnprintf(x_printf_comp_t __restrict, x_printf_domain_t __restrict, char * __restrict, size_t n, const char * __restrict, void ** __restrict);

X_LOCAL int x___fflush(x_FILE *fp);
X_LOCAL int x___sflush(x_FILE *);
X_LOCAL int x___swrite(void *, char const *, int);
X_LOCAL x_fpos_t x___sseek(void *, x_fpos_t, int);

X_LOCAL void x___smakebuf(x_FILE *);
X_LOCAL int x___swhatbuf(x_FILE *, size_t *, int *);
X_LOCAL int x___swsetup(x_FILE *);


/* hold a buncha junk that would grow the ABI */
struct x___sFILEX {
    unsigned char   *up;            /* saved _p when _p is doing ungetc data */
    pthread_mutex_t fl_mutex;       /* used for MT-safety */
    int             orientation:2;  /* orientation for fwide() */
    int             counted:1;      /* stream counted against STREAM_MAX */
    mbstate_t       mbstate;        /* multibyte conversion state */
};

#define _up             _extra->up
#define _fl_mutex       _extra->fl_mutex
#define _orientation    _extra->orientation
#define _mbstate        _extra->mbstate
#define _counted        _extra->counted

#if defined(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)
#define X_PTHREAD_MUTEX_INITIALIZER PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#else /* PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP */
#define X_PTHREAD_MUTEX_INITIALIZER PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#endif /* !PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP */

#define X_INITEXTRA(fp)                                         \
    do {                                                        \
        (fp)->_extra->up = NULL;                                \
        (fp)->_extra->fl_mutex = (pthread_mutex_t)X_PTHREAD_MUTEX_INITIALIZER; \
        (fp)->_extra->orientation = 0;                          \
        memset(&(fp)->_extra->mbstate, 0, sizeof(mbstate_t));   \
        (fp)->_extra->counted = 0;                              \
    } while(0);

/*
 * Prepare the given x_FILE for writing, and return 0 iff it
 * can be written now.  Otherwise, return X_EOF and set errno.
 */
#define x_prepwrite(fp)                 \
    ((((fp)->_flags & X___SWR) == 0 ||  \
        ((fp)->_bf._base == NULL && ((fp)->_flags & X___SSTR) == 0)) && \
    x___swsetup(fp))

/*
 * Test whether the given stdio file has an active ungetc buffer;
 * release such a buffer, without restoring ordinary unread data.
 */
#define X_HASUB(fp) ((fp)->_ub._base != NULL)
#define X_FREEUB(fp) {                      \
    if((fp)->_ub._base != (fp)->_ubuf) {    \
        free((char *)(fp)->_ub._base);      \
    }                                       \
    (fp)->_ub._base = NULL;                 \
}

/*
 * Set the orientation for a stream. If o > 0, the stream has wide-
 * orientation. If o < 0, the stream has byte-orientation.
 */
#define X_ORIENT(fp, o)                 \
    do {                                \
        if((fp)->_orientation == 0) {   \
            (fp)->_orientation = (o);   \
        }                               \
    } while (0)

#endif /* !LIBXPRINTF_LOCAL_H */
