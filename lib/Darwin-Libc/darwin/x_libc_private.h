/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/darwin/libc_private.h
 *
 */


#ifndef LIBXPRINTF_LIBC_PRIVATE_H
#define LIBXPRINTF_LIBC_PRIVATE_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* https://stackoverflow.com/questions/63372558/can-i-disable-or-ignore-apple-additions-to-c-standard-headers */
#if defined(__MACH__)
#define _ANSI_SOURCE
#endif /* !defined(__MACH__) */

#include <stdlib.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

__BEGIN_DECLS

static inline void x_abort_report_np(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    abort();
}//end x_abort_report_np

/* f must be a literal string */
#define X_LIBC_ABORT(f, ...)    \
    do {                        \
        x_abort_report_np("%s:%s:%u: " f, __FILENAME__, __func__, __LINE__, ## __VA_ARGS__); \
    }                           \
    while(0)

__END_DECLS

#endif /* !LIBXPRINTF_LIBC_PRIVATE_H */
