/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/xprintf_private.h
 *
 */


#ifndef LIBXPRINTF_XPRINTF_PRIVATE_H
#define LIBXPRINTF_XPRINTF_PRIVATE_H

#include <pthread.h>

#include <libxprintf/libxprintf.h>

#include "x_libc_private.h"

#if !defined(X_VECTORS)
#define X_VECTORS
typedef __attribute__((vector_size(16))) unsigned char X_VECTORTYPE;
#if defined(__SSE2__)
#define X_V64TYPE
#endif /* !defined(__SSE2__) */
#endif /* !!defined(X_VECTORS) */

/* FreeBSD extension */
struct x___printf_io;
typedef int x_printf_render(struct x___printf_io *, const struct x_printf_info *, const void *const *);

#if 0
int x_register_printf_render(int spec, x_printf_render *render, x_printf_arginfo_function *arginfo);
#endif

/*
 * Unlike register_printf_domain_function(), x_register_printf_domain_render()
 * doesn't have a context pointer, because none of the internal rendering
 * functions use it.
 */
X_LOCAL int x_register_printf_domain_render(x_printf_domain_t, int, x_printf_render *, x_printf_arginfo_function *);

/* xprintf.c */
extern const char x___lowercase_hex[17];
extern const char x___uppercase_hex[17];

X_LOCAL void x___printf_flush(struct x___printf_io *io);
X_LOCAL int x___printf_puts(struct x___printf_io *io, const void *ptr, int len);
X_LOCAL int x___printf_pad(struct x___printf_io *io, int n, int zero);
X_LOCAL int x___printf_out(struct x___printf_io *io, const struct x_printf_info *pi, const void *ptr, int len);

X_LOCAL int x___v2printf(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain, x_FILE * restrict fp, const char * restrict fmt0, va_list ap);
X_LOCAL int x___xvprintf(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain, x_FILE * restrict fp, const char * restrict fmt0, va_list ap);

X_LOCAL int x___l2printf(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain, x_FILE * restrict fp, const char * restrict fmt0, void ** restrict args);
X_LOCAL int x___xlprintf(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain, x_FILE * restrict fp, const char * restrict fmt0, void ** restrict args);

X_LOCAL x_printf_arginfo_function   x___printf_arginfo_pct;
X_LOCAL x_printf_render             x___printf_render_pct;

X_LOCAL x_printf_arginfo_function   x___printf_arginfo_n;
X_LOCAL x_printf_function           x___printf_render_n;

#if defined(X_VECTORS)
X_LOCAL x_printf_render             x___xprintf_vector;
#endif /* !defined(X_VECTORS) */

#define X_CALLOC(x,y) calloc((x), (y))
#define X_MALLOC(x)   malloc((x))

/* xprintf_domain.c */
X_LOCAL void x___xprintf_domain_init(void);
extern pthread_once_t x___xprintf_domain_once;

#define x_xprintf_domain_init() pthread_once(&x___xprintf_domain_once, x___xprintf_domain_init)

/* xprintf_errno.c */
X_LOCAL x_printf_arginfo_function   x___printf_arginfo_errno;
X_LOCAL x_printf_render             x___printf_render_errno;

/* xprintf_float.c */
X_LOCAL x_printf_arginfo_function   x___printf_arginfo_float;
X_LOCAL x_printf_render             x___printf_render_float;

/* xprintf_hexdump.c */
X_LOCAL x_printf_arginfo_function   x___printf_arginfo_hexdump;
X_LOCAL x_printf_render             x___printf_render_hexdump;

/* xprintf_int.c */
X_LOCAL x_printf_arginfo_function   x___printf_arginfo_ptr;
X_LOCAL x_printf_arginfo_function   x___printf_arginfo_int;
X_LOCAL x_printf_render             x___printf_render_ptr;
X_LOCAL x_printf_render             x___printf_render_int;

/* xprintf_quoute.c */
X_LOCAL x_printf_arginfo_function   x___printf_arginfo_quote;
X_LOCAL x_printf_render             x___printf_render_quote;

/* xprintf_str.c */
X_LOCAL x_printf_arginfo_function   x___printf_arginfo_chr;
X_LOCAL x_printf_render             x___printf_render_chr;
X_LOCAL x_printf_arginfo_function   x___printf_arginfo_str;
X_LOCAL x_printf_render             x___printf_render_str;

/* xprintf_time.c */
X_LOCAL x_printf_arginfo_function   x___printf_arginfo_time;
X_LOCAL x_printf_render             x___printf_render_time;

/* xprintf_vis.c */
X_LOCAL x_printf_arginfo_function   x___printf_arginfo_vis;
X_LOCAL x_printf_render             x___printf_render_vis;

struct x__printf_compiled {
    pthread_mutex_t         mutex;
    const char              *fmt;
    x_printf_domain_t       domain;
    struct x_printf_info    *pi;
    struct x_printf_info    *pil;
    int                     *argt;
    union x_arg             *args;
    int                     maxarg;
};

X_LOCAL int x___printf_comp(x_printf_comp_t restrict, x_printf_domain_t restrict);
X_LOCAL int x___printf_exec(x_printf_comp_t restrict, x_FILE * restrict, va_list);

X_LOCAL int x___lprintf_exec(x_printf_comp_t restrict, x_FILE * restrict, void ** restrict);

#endif /* !LIBXPRINTF_XPRINTF_PRIVATE_H */
