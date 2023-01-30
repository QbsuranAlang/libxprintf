/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/xprintf_domain.h
 *
 */


#ifndef LIBXPRINTF_XPRINTF_DOMAIN_H
#define LIBXPRINTF_XPRINTF_DOMAIN_H

#include <pthread.h>

#include <libxprintf/libxprintf.h>

#define X_PRINTF_TBL_FIRST  '!'
#define X_PRINTF_TBL_LAST   '~'
#define X_PRINTF_TBL_SIZE   (X_PRINTF_TBL_LAST - X_PRINTF_TBL_FIRST + 1)

#define x_printf_tbl_index(x)       ((x) - X_PRINTF_TBL_FIRST)
#define x_printf_tbl_in_range(x)    ((x) >= X_PRINTF_TBL_FIRST && (x) <= X_PRINTF_TBL_LAST)

enum {
    X_PRINTF_DOMAIN_UNUSED = 0,
    X_PRINTF_DOMAIN_GLIBC_API,
    X_PRINTF_DOMAIN_FBSD_API,
    X_PRINTF_DOMAIN_FLAG,
};

#define x_printf_domain_fbsd_api(d,x)   ((d)->type[x] == X_PRINTF_DOMAIN_FBSD_API)
#define x_printf_domain_flag(d,x)       ((d)->type[x] == X_PRINTF_DOMAIN_FLAG)
#define x_printf_domain_glibc_api(d,x)  ((d)->type[x] == X_PRINTF_DOMAIN_GLIBC_API)
#define x_printf_domain_unused(d,x)     ((d)->type[x] == X_PRINTF_DOMAIN_UNUSED)

struct x__printf_tbl {
    x_printf_arginfo_function   *arginfo;
    void                        *render; /* either typedef x_printf_function or x_printf_render */
    void                        *context;
};

struct x__printf_domain {
    pthread_rwlock_t        rwlock;
    char                    type[X_PRINTF_TBL_SIZE];
    struct x__printf_tbl    tbl[X_PRINTF_TBL_SIZE];
};

#endif /* !LIBXPRINTF_XPRINTF_DOMAIN_H */
