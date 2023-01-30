/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/vdprintf.c
 *
 */


#include <errno.h>
#include <stdarg.h>
#include <limits.h>

#include "x_local.h"

int x__vdprintf(x_printf_comp_t __restrict pc, x_printf_domain_t __restrict domain, int fd, const char * __restrict fmt, va_list ap) {
    int                 ret;
    x_FILE              f;
    unsigned char       buf[X_BUFSIZ];
    struct x___sFILEX   ext;

    f._extra = &ext;
    X_INITEXTRA(&f);

    if(fd > SHRT_MAX) {
        errno = EMFILE;
        return X_EOF;
    }//end if

    f._p = buf;
    f._w = sizeof(buf);
    f._flags = X___SWR;
    f._file = fd;
    f._cookie = &f;
    f._write = x___swrite;
    f._bf._base = buf;
    f._bf._size = sizeof(buf);
    f._orientation = 0;
    memset(&f._mbstate, 0, sizeof(f._mbstate));

    if((ret = x___v2printf(pc, domain, &f, fmt, ap)) < 0) {
        return ret;
    }//end if

    return x___fflush(&f) ? X_EOF : ret;
}//end x__vdprintf

int x__ldprintf(x_printf_comp_t __restrict pc, x_printf_domain_t __restrict domain, int fd, const char * __restrict fmt, void ** __restrict args) {
    int                 ret;
    x_FILE              f;
    unsigned char       buf[X_BUFSIZ];
    struct x___sFILEX   ext;

    f._extra = &ext;
    X_INITEXTRA(&f);

    if(fd > SHRT_MAX) {
        errno = EMFILE;
        return X_EOF;
    }//end if

    f._p = buf;
    f._w = sizeof(buf);
    f._flags = X___SWR;
    f._file = fd;
    f._cookie = &f;
    f._write = x___swrite;
    f._bf._base = buf;
    f._bf._size = sizeof(buf);
    f._orientation = 0;
    memset(&f._mbstate, 0, sizeof(f._mbstate));

    if((ret = x___l2printf(pc, domain, &f, fmt, args)) < 0) {
        return ret;
    }//end if

    return x___fflush(&f) ? X_EOF : ret;
}//end x__vdprintf
