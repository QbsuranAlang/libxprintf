/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/xprintf_domain.c
 *
 */


#include <pthread.h>
#include <errno.h>

#include <libxprintf/libxprintf.h>

#include "x_xprintf_domain.h"
#include "x_xprintf_private.h"
#include "x_libc_private.h" /* for X_LIBC_ABORT */

/* These are flag characters and can never be used as conversion specifiers */
static const char x__printf_tbl_flags[] = "#$'*+,-.0123456789:;L_hjlqtvz";

struct x__printf_tbl_defaults_fbsd {
    const char                  *spec;
    x_printf_arginfo_function   *arginfo;
    x_printf_render             *render;
};

static struct x__printf_tbl_defaults_fbsd x__printf_tbl_defaults_fbsd[] = {
    { "%",           x___printf_arginfo_pct,     x___printf_render_pct  },
    { "AEFGaefg",    x___printf_arginfo_float,   x___printf_render_float},
    { "Cc",          x___printf_arginfo_chr,     x___printf_render_chr  },
    { "DOUXdioux",   x___printf_arginfo_int,     x___printf_render_int  },
    { "Ss",          x___printf_arginfo_str,     x___printf_render_str  },
    { "p",           x___printf_arginfo_ptr,     x___printf_render_ptr  },
};

struct x__printf_tbl_defaults_glibc {
    const char                  *spec;
    x_printf_arginfo_function   *arginfo;
    x_printf_function           *render;
};

static struct x__printf_tbl_defaults_glibc x__printf_tbl_defaults_glibc[] = {
    { "n",           x___printf_arginfo_n,       x___printf_render_n    },
};

X_EXPORT x_printf_domain_t x_xprintf_domain_default;

X_LOCAL pthread_once_t x___xprintf_domain_once = PTHREAD_ONCE_INIT;

void x___xprintf_domain_init(void) {
    int                                 n;
    const char                          *cp;
    struct x__printf_tbl_defaults_fbsd  *fbsd;
    struct x__printf_tbl_defaults_glibc *glibc;

    x_xprintf_domain_default = (x_printf_domain_t)calloc(1, sizeof(*x_xprintf_domain_default));

    if(x_xprintf_domain_default == NULL) {
        X_LIBC_ABORT("No memory");
    }//end if

    x_xprintf_domain_default->rwlock = (pthread_rwlock_t)PTHREAD_RWLOCK_INITIALIZER;

    for(cp = x__printf_tbl_flags; *cp; cp++) {
        x_xprintf_domain_default->type[x_printf_tbl_index(*cp)] = X_PRINTF_DOMAIN_FLAG;
    }//end for

    fbsd = x__printf_tbl_defaults_fbsd;
    n = sizeof(x__printf_tbl_defaults_fbsd) / sizeof(*x__printf_tbl_defaults_fbsd);

    for(; n > 0; fbsd++, n--) {
        for(cp = fbsd->spec; *cp; cp++) {
            x_xprintf_domain_default->type[x_printf_tbl_index(*cp)] = X_PRINTF_DOMAIN_FBSD_API;
            x_xprintf_domain_default->tbl[x_printf_tbl_index(*cp)] = (struct x__printf_tbl){fbsd->arginfo, fbsd->render, NULL};
        }//end for
    }//end for

    glibc = x__printf_tbl_defaults_glibc;
    n = sizeof(x__printf_tbl_defaults_glibc) / sizeof(*x__printf_tbl_defaults_glibc);
    for(; n > 0; glibc++, n--) {
        for(cp = glibc->spec; *cp; cp++) {
            x_xprintf_domain_default->type[x_printf_tbl_index(*cp)] = X_PRINTF_DOMAIN_GLIBC_API;
            x_xprintf_domain_default->tbl[x_printf_tbl_index(*cp)] = (struct x__printf_tbl){glibc->arginfo, glibc->render, NULL};
        }//end for
    }//end for
}//end x___xprintf_domain_init

x_printf_domain_t x_copy_printf_domain(x_printf_domain_t src) {
    x_printf_domain_t restrict copy;

    if(!src) {
        errno = EINVAL;
        return NULL;
    }//end if

    copy = (x_printf_domain_t)X_MALLOC(sizeof(*copy));
    if(!copy) {
        return NULL;
    }//end if

    x_xprintf_domain_init();
    pthread_rwlock_rdlock(&src->rwlock);
    *copy = *src;
    pthread_rwlock_unlock(&src->rwlock);
    copy->rwlock = (pthread_rwlock_t)PTHREAD_RWLOCK_INITIALIZER;

    return copy;
}//end x_copy_printf_domain

void x_free_printf_domain(x_printf_domain_t d) {

    if(!d) {
        return;
    }//end if

    pthread_rwlock_destroy(&d->rwlock);
    free(d);
}//end x_free_printf_domain

x_printf_domain_t x_new_printf_domain(void) {
    x_printf_domain_t restrict d;

    x_xprintf_domain_init();

    d = (x_printf_domain_t)X_MALLOC(sizeof(*d));
    if(!d) {
        return NULL;
    }//end if

    *d = *x_xprintf_domain_default;
    return d;
}//end x_new_printf_domain

int x_register_printf_domain_function(x_printf_domain_t d, int spec, x_printf_function *render, x_printf_arginfo_function *arginfo, void *context) {

    x_xprintf_domain_init();

    if(!d || !x_printf_tbl_in_range(spec)) {
        errno = EINVAL;
        return -1;
    }//end if

    x_xprintf_domain_init();

    switch(d->type[x_printf_tbl_index(spec)]) {

    case X_PRINTF_DOMAIN_FLAG:
    errno = EINVAL;
    return -1;

    default:
    pthread_rwlock_wrlock(&d->rwlock);
    if(!render || !arginfo) {
        d->type[x_printf_tbl_index(spec)] = X_PRINTF_DOMAIN_UNUSED;
    }//end if
    else {
        d->type[x_printf_tbl_index(spec)] = X_PRINTF_DOMAIN_GLIBC_API;
        d->tbl[x_printf_tbl_index(spec)] = (struct x__printf_tbl){arginfo, render, context};
    }//end else
    pthread_rwlock_unlock(&d->rwlock);
    }//end switch

    return 0;
}//end x_register_printf_domain_function

int x_register_printf_domain_render(x_printf_domain_t d, int spec, x_printf_render *render, x_printf_arginfo_function *arginfo) {

    x_xprintf_domain_init();

    if(!d || !x_printf_tbl_in_range(spec)) {
        errno = EINVAL;
        return -1;
    }//end if

    x_xprintf_domain_init();

    switch(d->type[x_printf_tbl_index(spec)]) {

    case X_PRINTF_DOMAIN_FLAG:
    errno = EINVAL;
    return -1;

    default:
    pthread_rwlock_wrlock(&d->rwlock);
    if(!render || !arginfo) {
        d->type[x_printf_tbl_index(spec)] = X_PRINTF_DOMAIN_UNUSED;
    }//end if
    else {
        d->type[x_printf_tbl_index(spec)] = X_PRINTF_DOMAIN_FBSD_API;
        d->tbl[x_printf_tbl_index(spec)] = (struct x__printf_tbl){arginfo, render, NULL};
    }//end else
    pthread_rwlock_unlock(&d->rwlock);
    }//end switch

    return 0;
}//end x_register_printf_domain_render

int x_register_printf_domain_render_std(x_printf_domain_t d, const char *specs) {
    int ret;

    if(!d) {
        errno = EINVAL;
        return -1;
    }//end if

    ret = 0;

    for(; *specs != '\0'; specs++) {
        switch(*specs) {
        case 'H':
            ret = x_register_printf_domain_render(d, *specs, x___printf_render_hexdump, x___printf_arginfo_hexdump);
            break;
        case 'M':
            ret = x_register_printf_domain_render(d, *specs, x___printf_render_errno, x___printf_arginfo_errno);
            break;
        case 'Q':
            ret = x_register_printf_domain_render(d, *specs, x___printf_render_quote, x___printf_arginfo_quote);
            break;
        case 'T':
            ret = x_register_printf_domain_render(d, *specs, x___printf_render_time, x___printf_arginfo_time);
            break;
        case 'V':
            ret = x_register_printf_domain_render(d, *specs, x___printf_render_vis, x___printf_arginfo_vis);
            break;
        default:
            errno = EINVAL;
            return -1;
        }//end switch

        if(ret < 0) {
            return ret;
        }//end if
    }//end for

    return 0;
}//end x_register_printf_domain_render_std
