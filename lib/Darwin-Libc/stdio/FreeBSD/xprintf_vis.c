/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/xprintf_vis.c
 *
 */


#include <string.h>

#include <libxprintf/libxprintf.h>

#include "x_xprintf_private.h"
#include "x_vis.h"

int x___printf_arginfo_vis(const struct x_printf_info *pi, size_t n, int *argt) {
    argt[0] = X_PA_POINTER;
    return 1;
}//end x___printf_arginfo_vis

int x___printf_render_vis(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    int         ret;
    char        *p, *buf;
    unsigned    l;

    ret = 0;
    p = *((char **)arg[0]);
    if(p == NULL) {
        ret = x___printf_out(io, pi, "(null)", 6);
        x___printf_flush(io);
        return ret;
    }//end if

    if(pi->prec >= 0) {
        l = pi->prec;
    }//end if
    else {
        l = strlen(p);
    }//end else

    buf = X_MALLOC(l * 4 + 1);
    if(buf == NULL) {
        return -1;
    }//end if

    if(pi->showsign) {
        ret = x_strvisx(buf, p, l, X_VIS_WHITE | X_VIS_HTTPSTYLE);
    }//end if
    else if(pi->pad == '0') {
        ret = x_strvisx(buf, p, l, X_VIS_WHITE | X_VIS_OCTAL);
    }//end if
    else if(pi->alt) {
        ret = x_strvisx(buf, p, l, X_VIS_WHITE);
    }//end if
    else {
        ret = x_strvisx(buf, p, l, X_VIS_WHITE | X_VIS_CSTYLE | X_VIS_OCTAL);
    }//end else

    /* TUTU: "+=" is something wrong at here */
    ret = x___printf_out(io, pi, buf, ret);
    x___printf_flush(io);
    free(buf);

    return ret;
}//end x___printf_render_vis
