/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/xprintf_time.c
 *
 */


#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

#include <libxprintf/libxprintf.h>

#include "x_xprintf_private.h"

int x___printf_arginfo_time(const struct x_printf_info *pi, size_t n, int *argt) {
    argt[0] = X_PA_POINTER;
    return 1;
}//end x___printf_arginfo_time

#define X_MINUTE    60
#define X_HOUR      (60 * X_MINUTE)
#define X_DAY       (24 * X_HOUR)
#define X_YEAR      (365 * X_DAY)

int x___printf_render_time(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    int             i, prec, nsec, ret;
    char            buf[100], *p;
    time_t          *tp;
    intmax_t        t, tx;
    struct timeval  *tv;
    struct timespec *ts;

    if(pi->is_long) {
        tv = *((struct timeval **)arg[0]);
        t = tv->tv_sec;
        nsec = tv->tv_usec * 1000;
        prec = 6;
    }//end if
    else if(pi->is_long_double) {
        ts = *((struct timespec **)arg[0]);
        t = ts->tv_sec;
        nsec = ts->tv_nsec;
        prec = 9;
    }//end if
    else {
        tp = *((time_t **)arg[0]);
        t = *tp;
        nsec = 0;
        prec = 0;
    }//end else

    if(pi->is_long || pi->is_long_double) {
        if(pi->prec >= 0) {
            prec = pi->prec;
        }//end if
        if(prec == 0) {
            nsec = 0;
        }//end if
    }//end if

    p = buf;
    if(pi->alt) {
        tx = t;
        if(t >= X_YEAR) {
            p += sprintf(p, "%jdy", t / X_YEAR);
            t %= X_YEAR;
        }//end if
        if(tx >= X_DAY && (t != 0 || prec != 0)) {
            p += sprintf(p, "%jdd", t / X_DAY);
            t %= X_DAY;
        }//end if
        if(tx >= X_HOUR && (t != 0 || prec != 0)) {
            p += sprintf(p, "%jdh", t / X_HOUR);
            t %= X_HOUR;
        }//end if
        if(tx >= X_MINUTE && (t != 0 || prec != 0)) {
            p += sprintf(p, "%jdm", t / X_MINUTE);
            t %= X_MINUTE;
        }//end if
        if(t != 0 || tx == 0 || prec != 0)
            p += sprintf(p, "%jds", t);
    }//end if
    else {
        p += sprintf(p, "%jd", (intmax_t)t);
    }//end else

    if(prec != 0) {
        for(i = prec; i < 9; i++) {
            nsec /= 10;
        }//end for
        p += sprintf(p, ".%.*d", prec, nsec);
    }//end if

    ret = x___printf_out(io, pi, buf, p - buf);
    x___printf_flush(io);

    return ret;
}//end x___printf_render_time
