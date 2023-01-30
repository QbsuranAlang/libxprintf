/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/vasprintf.c
 *
 */


#include <errno.h>
#include <stdarg.h>

#include "x_local.h"

int x__vasprintf(x_printf_comp_t __restrict pc, x_printf_domain_t __restrict domain, char ** __restrict str, const char * __restrict fmt, va_list ap) {
    int                 ret;
    x_FILE              f;
    struct x___sFILEX   ext;

    f._extra = &ext;
    X_INITEXTRA(&f);

    f._file = -1;
    f._flags = X___SWR | X___SSTR | X___SALC;
    f._bf._base = f._p = (unsigned char *)malloc(128);
    if(f._bf._base == NULL) {
        *str = NULL;
        errno = ENOMEM;
        return -1;
    }//end if

    f._bf._size = f._w = 127;        /* Leave room for the NUL */
    f._orientation = 0;
    memset(&f._mbstate, 0, sizeof(mbstate_t));
    ret = x___v2printf(pc, domain, &f, fmt, ap);
    if(ret < 0) {
        free(f._bf._base);
        *str = NULL;
        errno = ENOMEM;
        return -1;
    }//end if

    *f._p = '\0';
    *str = (char *)f._bf._base;
    return ret;
}//end x__vasprintf

int x__lasprintf(x_printf_comp_t __restrict pc, x_printf_domain_t __restrict domain, char ** __restrict str, const char * __restrict fmt, void ** __restrict args) {
    int                 ret;
    x_FILE              f;
    struct x___sFILEX   ext;

    f._extra = &ext;
    X_INITEXTRA(&f);

    f._file = -1;
    f._flags = X___SWR | X___SSTR | X___SALC;
    f._bf._base = f._p = (unsigned char *)malloc(128);
    if(f._bf._base == NULL) {
        *str = NULL;
        errno = ENOMEM;
        return -1;
    }//end if

    f._bf._size = f._w = 127;        /* Leave room for the NUL */
    f._orientation = 0;
    memset(&f._mbstate, 0, sizeof(mbstate_t));
    ret = x___l2printf(pc, domain, &f, fmt, args);
    if(ret < 0) {
        free(f._bf._base);
        *str = NULL;
        errno = ENOMEM;
        return -1;
    }//end if

    *f._p = '\0';
    *str = (char *)f._bf._base;
    return ret;
}//end x__lasprintf
