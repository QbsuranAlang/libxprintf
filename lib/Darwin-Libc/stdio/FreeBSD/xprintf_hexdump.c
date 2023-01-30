/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/xprintf_hexdump.c
 *
 */


#include <stdio.h>

#include <libxprintf/libxprintf.h>

#include "x_xprintf_private.h"

int x___printf_arginfo_hexdump(const struct x_printf_info *pi, size_t n, int *argt) {
    argt[0] = X_PA_POINTER;
    argt[1] = X_PA_INT;
    return 2;
}//end x___printf_arginfo_hexdump

int x___printf_render_hexdump(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    int             ret;
    char            buf[100], *q;
    unsigned        u, l, j, a;
    unsigned char   *p;

    if(pi->width > 0 && pi->width < 16) {
        l = pi->width;
    }//end if
    else {
        l = 16;
    }//end else

    p = *((unsigned char **)arg[0]);
    u = *((unsigned *)arg[1]);

    ret = 0;
    a = 0;
    while(u > 0) {
        q = buf;
        if(pi->showsign) {
            q += sprintf(q, " %04x", a);
        }//end if
        for(j = 0; j < l && j < u; j++) {
            q += sprintf(q, " %02x", p[j]);
        }//end for
        if(pi->alt) {
            for(; j < l; j++) {
                q += sprintf(q, "   ");
            }//end for
            q += sprintf(q, "  |");
            for(j = 0; j < l && j < u; j++) {
                if(p[j] < ' ' || p[j] > '~') {
                    *q++ = '.';
                }//end if
                else {
                    *q++ = p[j];
                }//end else
            }//end for
            for(; j < l; j++) {
                *q++ = ' ';
            }//end for
            *q++ = '|';
        }//end if
        if(l < u) {
            j = l;
        }//end if
        else {
            j = u;
        }//end else
        p += j;
        u -= j;
        a += j;
        if(u > 0) {
            *q++ = '\n';
        }//end if
        ret += x___printf_puts(io, buf + 1, q - (buf + 1));
        x___printf_flush(io);
    }//end while

    return ret;
}//end x___printf_render_hexdump
