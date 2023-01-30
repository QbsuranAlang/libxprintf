/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/include/printf.h
 *
 */


#ifndef LIBXPRINTF_H
#define LIBXPRINTF_H

#include <stdarg.h>

#include <libxprintf/libxprintf_visibility.h>

/****************************************************************************
 * This is the header file for extensible printf, a set of APIs that allow
 * adding/modifying conversion specifier(s) for stdio formatted printing.
 * It is based on the GLIBC API documented in:
 *
 *   http://www.gnu.org/software/libc/manual/html_node/Customizing-Printf.html
 *
 * Because that API affects printf behavior process-wide and so is unsafe,
 * we adapt a modified form, based on the concept of printf domains in which
 * changes to conversion specifiers can be made independent of one another
 * and which don't affect the normal printf behavior.  In addition, there
 * is now a set of printf variants that take a printf domain as an argument.
 *
 * See xprintf(5) for more details.
 ****************************************************************************/

#include <wchar.h>

/* forward reference */
struct x___sFILE;
typedef struct x___sFILE x_FILE;

#if defined(__GNUC__)
#define X___XPRINTF_ATTR(x) __attribute__(x)
#else /* defined(__GNUC__) */
#define X___XPRINTF_ATTR(x) /* nothing */
#endif /* !defined(__GNUC__) */

/*
 * The API defined by GLIBC allows a renderer to take multiple arguments
 * This is obviously usable for things like (ptr+len) pairs etc.
 * The current limit is to deal with up to X__PRINTFMAXARG arguments (any
 * above this limit are ignored).
 */
#define X___PRINTFMAXARG 2

struct x_printf_info {
    /* Mac OS X extensions */
    void        *context;           /* User context pointer */
    wchar_t     vsep;               /* Vector separator char */
                        /* one of ,:;_ flag or X by default */

    /* GLIBC compatible */
    int         prec;               /* precision */
    int         width;              /* Width */
    wchar_t     spec;               /* Format letter */
    wchar_t     pad;                /* Padding char */
                        /* 0 if 0 flag set, otherwise space */

    /* FreeBSD extensions */
    wchar_t     signchar;           /* Sign char */

    /* GLIBC compatible flags */
    unsigned    is_long_double:1;   /* L or ll flag */
    unsigned    is_char:1;          /* hh flag */
    unsigned    is_short:1;         /* h flag */
    unsigned    is_long:1;          /* l flag */
    unsigned    alt:1;              /* # flag */
    unsigned    space:1;            /* Space flag */
    unsigned    left:1;             /* - flag */
    unsigned    showsign:1;         /* + flag */
    unsigned    group:1;            /* ' flag */
    unsigned    extra:1;            /* For special use (currently unused) */
    unsigned    wide:1;             /* Nonzero for wide character streams (currently unused) */

    /* FreeBSD flags */
    unsigned    is_quad:1;          /* q flag */
    unsigned    is_intmax:1;        /* j flag */
    unsigned    is_ptrdiff:1;       /* t flag */
    unsigned    is_size:1;          /* z flag */

    /* Mac OS X flags */
    unsigned    is_vec:1;           /* v flag */

    /* private */
    int         sofar;
    unsigned    get_width;
    unsigned    get_prec;
    const char  *begin;
    const char  *end;
    void        *arg[X___PRINTFMAXARG];
};

enum {
    X_PA_INT        = (1 << 0),     /* int */
    X_PA_CHAR       = (1 << 1),     /* int, cast to char */
    X_PA_WCHAR      = (1 << 2),     /* wide char */
    X_PA_STRING     = (1 << 3),     /* const char * (with '\0') */
    X_PA_WSTRING    = (1 << 4),     /* const wchar_t * */
    X_PA_POINTER    = (1 << 5),     /* void * */
    X_PA_FLOAT      = (1 << 6),     /* float (Defined but unused; best to avoid.) */
    X_PA_DOUBLE     = (1 << 7),     /* double */
    X_PA_VECTOR     = (1 << 8),     /* vector */
};

#define X_PA_FLAG_MASK          0xff0000
#define X_PA_FLAG_LONG_LONG     (1 << 16)
#define X_PA_FLAG_LONG          (1 << 17)
#define X_PA_FLAG_SHORT         (1 << 18)
#define X_PA_FLAG_PTR           (1 << 19)
#define X_PA_FLAG_QUAD          (1 << 20)
#define X_PA_FLAG_INTMAX        (1 << 21)
#define X_PA_FLAG_SIZE          (1 << 22)
#define X_PA_FLAG_PTRDIFF       (1 << 23)
#define X_PA_FLAG_LONG_DOUBLE   X_PA_FLAG_LONG_LONG

/************************ Basic Extensible Printf APIs ************************/

typedef int x_printf_arginfo_function(const struct x_printf_info *__info,
        size_t __n, int *__argtypes);
typedef int x_printf_function(x_FILE *__stream,
        const struct x_printf_info *__info, const void *const *__args);

/*
 * We don't support the GLIBC register_printf_function() or FreeBSD
 * register_printf_render_std(), because they affect printf globally
 * and are unsafe.
 */

/*************** Extensible Printf Domains APIs ****************/

struct x__printf_domain; /* forward reference */
typedef struct x__printf_domain *x_printf_domain_t;

/*
 * It's not recommended use it directly, even it's thread-safe.
 */
extern x_printf_domain_t x_xprintf_domain_default;

__BEGIN_DECLS

X_EXPORT x_printf_domain_t x_copy_printf_domain(x_printf_domain_t __domain)
        X___XPRINTF_ATTR((__nonnull__(1)));
X_EXPORT void x_free_printf_domain(x_printf_domain_t __domain);
X_EXPORT x_printf_domain_t x_new_printf_domain(void)
        X___XPRINTF_ATTR((__malloc__));
X_EXPORT int x_register_printf_domain_function(x_printf_domain_t __domain,
        int __spec, x_printf_function *__render,
        x_printf_arginfo_function *__arginfo, void *__context)
        X___XPRINTF_ATTR((__nonnull__(1)));
X_EXPORT int x_register_printf_domain_render_std(x_printf_domain_t __domain,
        const char *__specs)
        X___XPRINTF_ATTR((__nonnull__(1)));

/**** All-in-one extensible printf variants ****/
X_EXPORT int x_asxprintf(char ** __restrict __ret,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...)
        X___XPRINTF_ATTR((__nonnull__(1, 2, 3)));
X_EXPORT int x_dxprintf(int __fd, x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...)
        X___XPRINTF_ATTR((__nonnull__(2, 3)));
X_EXPORT int x_fxprintf(x_FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...)
        X___XPRINTF_ATTR((__nonnull__(1, 2, 3)));
X_EXPORT int x_fxprintf_std(FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...)
        X___XPRINTF_ATTR((__nonnull__(1, 2, 3)));
X_EXPORT int x_sxprintf(char * __restrict __str, size_t __size,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...)
        X___XPRINTF_ATTR((__nonnull__(1, 3, 4)));
X_EXPORT int x_xprintf(x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));

X_EXPORT int x_vasxprintf(char ** __restrict __ret,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(1, 2, 3)));
X_EXPORT int x_vdxprintf(int __fd, x_printf_domain_t __restrict __domain,
        const char * __restrict __format,
        va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(2, 3)));
X_EXPORT int x_vfxprintf(x_FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(1, 2, 3)));
X_EXPORT int x_vfxprintf_std(FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(1, 2, 3)));
X_EXPORT int x_vsxprintf(char * __restrict __str, size_t __size,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(1, 3, 4)));
X_EXPORT int x_vxprintf(x_printf_domain_t __restrict __domain,
        const char * __restrict __format,
        va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));

X_EXPORT int x_lasxprintf(char ** __restrict __ret,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(1, 2, 3)));
X_EXPORT int x_ldxprintf(int __fd, x_printf_domain_t __restrict __domain,
        const char * __restrict __format,
        void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(2, 3)));
X_EXPORT int x_lfxprintf(x_FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(1, 2, 3)));
X_EXPORT int x_lfxprintf_std(FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(1, 2, 3)));
X_EXPORT int x_lsxprintf(char * __restrict __str, size_t __size,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(1, 3, 4)));
X_EXPORT int x_lxprintf(x_printf_domain_t __restrict __domain,
        const char * __restrict __format,
        void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));

__END_DECLS

/******** Extensible Printf Compilation/Execution APIs *********/
struct x__printf_compiled; /* forward reference */
typedef struct x__printf_compiled *x_printf_comp_t;

__BEGIN_DECLS

X_EXPORT void x_free_printf_comp(x_printf_comp_t __pc);
X_EXPORT x_printf_comp_t x_new_printf_comp(x_printf_domain_t __restrict __domain,
        const char * __restrict __fmt)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));
X_EXPORT int x_new_printf_comp_dry_run(x_printf_domain_t __restrict __domain,
        const char * __restrict __fmt)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));

/**** Extensible printf execution ****/
X_EXPORT int x_asxprintf_exec(char ** __restrict __ret,
        x_printf_comp_t __restrict __pc, ...)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));
X_EXPORT int x_dxprintf_exec(int __fd, x_printf_comp_t __restrict __pc, ...)
        X___XPRINTF_ATTR((__nonnull__(2)));
X_EXPORT int x_fxprintf_exec(x_FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, ...)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));
X_EXPORT int x_fxprintf_std_exec(FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, ...)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));
X_EXPORT int x_sxprintf_exec(char * __restrict __str, size_t __size,
        x_printf_comp_t __restrict __pc, ...)
        X___XPRINTF_ATTR((__nonnull__(1, 3)));
X_EXPORT int x_xprintf_exec(x_printf_comp_t __restrict __pc, ...)
        X___XPRINTF_ATTR((__nonnull__(1)));

X_EXPORT int x_vasxprintf_exec(char ** __restrict __ret,
        x_printf_comp_t __restrict __pc, va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));
X_EXPORT int x_vdxprintf_exec(int __fd, x_printf_comp_t __restrict __pc,
        va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(2)));
X_EXPORT int x_vfxprintf_exec(x_FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));
X_EXPORT int x_vfxprintf_std_exec(FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));
X_EXPORT int x_vsxprintf_exec(char * __restrict __str, size_t __size,
        x_printf_comp_t __restrict __pc, va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(1, 3)));
X_EXPORT int x_vxprintf_exec(x_printf_comp_t __restrict __pc, va_list __ap)
        X___XPRINTF_ATTR((__nonnull__(1)));

X_EXPORT int x_lasxprintf_exec(char ** __restrict __ret,
        x_printf_comp_t __restrict __pc, void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));
X_EXPORT int x_ldxprintf_exec(int __fd, x_printf_comp_t __restrict __pc,
        void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(2)));
X_EXPORT int x_lfxprintf_exec(x_FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));
X_EXPORT int x_lfxprintf_std_exec(FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(1, 2)));
X_EXPORT int x_lsxprintf_exec(char * __restrict __str, size_t __size,
        x_printf_comp_t __restrict __pc, void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(1, 3)));
X_EXPORT int x_lxprintf_exec(x_printf_comp_t __restrict __pc, void ** __restrict __args)
        X___XPRINTF_ATTR((__nonnull__(1)));

__END_DECLS

#endif /* !LIBXPRINTF_H */
