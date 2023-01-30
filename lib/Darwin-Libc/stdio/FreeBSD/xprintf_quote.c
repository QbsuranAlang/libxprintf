/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/xprintf_quote.c
 *
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <libxprintf/libxprintf.h>

#include "x_xprintf_private.h"

int x___printf_arginfo_quote(const struct x_printf_info *pi, size_t n, int *argt) {
    argt[0] = X_PA_POINTER;
    return 1;
}//end x___printf_arginfo_quote

int x___printf_render_quote(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    char        r[5];
    int         i, ret;
    const char  *str, *p, *t, *o;

    str = *((const char *const *)arg[0]);
    if(str == NULL) {
        ret = x___printf_out(io, pi, "\"(null)\"", 8);
        x___printf_flush(io);
        return ret;
    }//end if
    if(*str == '\0') {
        ret = x___printf_out(io, pi, "\"\"", 2);
        x___printf_flush(io);
        return ret;
    }//end if

    for(i = 0, p = str; *p; p++) {
        if(isspace(*p) || *p == '\\' || *p == '"') {
            i++;
        }//end if
    }//end for

    if(!i) {
        ret = x___printf_out(io, pi, str, strlen(str));
        x___printf_flush(io);
        return ret;
    }//end if

    ret = x___printf_out(io, pi, "\"", 1);
    for(t = p = str; *p; p++) {
        o = NULL;
        if(*p == '\\') {
            o = "\\\\";
        }//end if
        else if(*p == '\n') {
            o = "\\n";
        }//end if
        else if(*p == '\r') {
            o = "\\r";
        }//end if
        else if(*p == '\t') {
            o = "\\t";
        }//end if
        else if(*p == ' ') {
            o = " ";
        }//end if
        else if(*p == '"') {
            o = "\\\"";
        }//end if
        else if(isspace(*p)) {
            sprintf(r, "\\%03o", *p);
            o = r;
        }//end if
        else {
            continue;
        }//end else

        if(p != t) {
            ret += x___printf_out(io, pi, t, p - t);
        }//end if
        ret += x___printf_out(io, pi, o, strlen(o));
        t = p + 1;
    }//end for

    if(p != t) {
        ret += x___printf_out(io, pi, t, p - t);
    }//end if

    ret += x___printf_out(io, pi, "\"", 1);
    x___printf_flush(io);

    return ret;
}//end x___printf_render_quote
