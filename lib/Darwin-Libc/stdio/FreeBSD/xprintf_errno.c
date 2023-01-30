/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/xprintf_errno.c
 *
 */


#include <stdio.h>
#include <string.h>

#include <libxprintf/libxprintf.h>

#include "x_xprintf_private.h"

int x___printf_arginfo_errno(const struct x_printf_info *pi, size_t n, int *argt) {
    argt[0] = X_PA_INT;
    return 1;
}//end x___printf_arginfo_errno

int x___printf_render_errno(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    int         ret, error;
    char        buf[64];
    const char  *p;

    ret = 0;
    error = *((const int *)arg[0]);

    memset(buf, 0, sizeof(buf));
    ret = strerror_r(error, buf, sizeof(buf));
    if(ret == 0 && strncmp(buf, "Unknown error", 13) != 0) {
        p = buf;
        ret = x___printf_out(io, pi, p, strlen(p));
        x___printf_flush(io);
        return ret;
    }//end if

    sprintf(buf, "errno=%d/%#06x", error, error);
    ret = x___printf_out(io, pi, buf, strlen(buf));
    x___printf_flush(io);

    return ret;
}//end x___printf_render_errno
