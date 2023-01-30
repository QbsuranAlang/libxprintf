/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/vsnprintf.c
 *
 */


#include <stdarg.h>
#include <limits.h>

#include "x_local.h"

int x__vsnprintf(x_printf_comp_t __restrict pc, x_printf_domain_t __restrict domain, char * __restrict str, size_t n, const char * __restrict fmt, va_list ap) {
    int                 ret;
    char                dummy[2];
    size_t              on;
    x_FILE              f;
    struct x___sFILEX   ext;

    f._extra = &ext;
    X_INITEXTRA(&f);

    on = n;
    if(n != 0) {
        n--;
    }//end if

#if defined(__i386__)
    /* <rdar://problem/16329527> don't corrupt the output buffer at all if the size underflowed */
    if(n > INT_MAX) {
        on = n = 0;
    }//end if
#else /* defined(__i386__) */
    if(n > INT_MAX) {
        n = INT_MAX;
    }//end if
#endif /* !defined(__i386__) */

    /* Stdio internals do not deal correctly with zero length buffer */
    if(n == 0) {
        if(on > 0) {
            *str = '\0';
        }//end if
        str = dummy;
        n = 1;
    }//end if

    f._file = -1;
    f._flags = X___SWR | X___SSTR;
    f._bf._base = f._p = (unsigned char *)str;
    f._bf._size = f._w = n;
    f._orientation = 0;
    memset(&f._mbstate, 0, sizeof(mbstate_t));
    ret = x___v2printf(pc, domain, &f, fmt, ap);
    if(on > 0) {
        *f._p = '\0';
    }//end if

    return ret;
}//end x__vsnprintf

int x__lsnprintf(x_printf_comp_t __restrict pc, x_printf_domain_t __restrict domain, char * __restrict str, size_t n, const char * __restrict fmt, void ** __restrict args) {
    int                 ret;
    char                dummy[2];
    size_t              on;
    x_FILE              f;
    struct x___sFILEX   ext;

    f._extra = &ext;
    X_INITEXTRA(&f);

    on = n;
    if(n != 0) {
        n--;
    }//end if

#if defined(__i386__)
    /* <rdar://problem/16329527> don't corrupt the output buffer at all if the size underflowed */
    if(n > INT_MAX) {
        on = n = 0;
    }//end if
#else /* defined(__i386__) */
    if(n > INT_MAX) {
        n = INT_MAX;
    }//end if
#endif /* !defined(__i386__) */

    /* Stdio internals do not deal correctly with zero length buffer */
    if(n == 0) {
        if(on > 0) {
            *str = '\0';
        }//end if
        str = dummy;
        n = 1;
    }//end if

    f._file = -1;
    f._flags = X___SWR | X___SSTR;
    f._bf._base = f._p = (unsigned char *)str;
    f._bf._size = f._w = n;
    f._orientation = 0;
    memset(&f._mbstate, 0, sizeof(mbstate_t));
    ret = x___l2printf(pc, domain, &f, fmt, args);
    if(on > 0) {
        *f._p = '\0';
    }//end if

    return ret;
}//end x__lsnprintf
