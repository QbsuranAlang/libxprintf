/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/xprintf.c
 *
 */


#include <err.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>

#include "x_stdio.h"
#include "x_local.h"
#include "x_xprintf_private.h"
#include "x_xprintf_domain.h"
#include "x_fvwrite.h"

/* private stuff -----------------------------------------------------*/

union x_arg {
    int                 intarg;
    long                longarg;
    intmax_t            intmaxarg;
#if !defined(NO_FLOATING_POINT)
    double              doublearg;
    long double         longdoublearg;
#endif /* !defined(NO_FLOATING_POINT) */
    wint_t              wintarg;
    char                *pchararg;
    wchar_t             *pwchararg;
    void                *pvoidarg;
#if defined(X_VECTORS)
    X_VECTORTYPE        vectorarg;
    unsigned char       vuchararg[16];
    signed char         vchararg[16];
    unsigned short      vushortarg[8];
    signed short        vshortarg[8];
    unsigned int        vuintarg[4];
    signed int          vintarg[4];
    float               vfloatarg[4];
#if defined(X_V64TYPE)
    double              vdoublearg[2];
    unsigned long long  vulonglongarg[2];
    long long           vlonglongarg[2];
#endif /* !defined(X_V64TYPE) */
#endif /* !defined(X_VECTORS)S */
};

/*
 * Macros for converting digits to letters and vice versa
 */
#define x_to_digit(c) ((c) - '0')
#define x_is_digit(c) (((unsigned)x_to_digit(c)) <= 9)

/* various globals ---------------------------------------------------*/

X_LOCAL const char x___lowercase_hex[17] = "0123456789abcdef?"; /*lint !e784 */
X_LOCAL const char x___uppercase_hex[17] = "0123456789ABCDEF?"; /*lint !e784 */

#define X_PADSIZE 16
static char x_blanks[X_PADSIZE] =
    {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
static char x_zeroes[X_PADSIZE] =
    {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

/* printing and padding functions ------------------------------------*/

#define E_NIOV 8

struct x___printf_io {
    x_FILE          *fp;
    struct x___suio uio;
    struct x___siov iov[E_NIOV];
    struct x___siov *iovp;
};

static void x___printf_init(struct x___printf_io *io) {
    io->uio.uio_iov = io->iovp = &io->iov[0];
    io->uio.uio_resid = 0;
    io->uio.uio_iovcnt = 0;
}//end x___printf_init

void x___printf_flush(struct x___printf_io *io) {
    x___sfvwrite(io->fp, &io->uio);
    x___printf_init(io);
}//end x___printf_flush

int x___printf_puts(struct x___printf_io *io, const void *ptr, int len) {
#if 0
    if(io->fp->_flags & X___SERR) {
        return 0;
    }//end if
#endif /* 0 */

    if(len == 0) {
        return 0;
    }//end if

#define X___DECONST(type, var) ((type)(uintptr_t)(const void *)(var))

    io->iovp->iov_base = X___DECONST(void *, ptr);
    io->iovp->iov_len = len;
    io->uio.uio_resid += len;
    io->iovp++;
    io->uio.uio_iovcnt++;
    if(io->uio.uio_iovcnt >= E_NIOV) {
        x___printf_flush(io);
    }//end if

    return len;
}//end x___printf_puts

int x___printf_pad(struct x___printf_io *io, int howmany, int zero) {
    int         n, ret;
    const char  *with;

    ret = 0;
    if(zero) {
        with = x_zeroes;
    }//end if
    else {
        with = x_blanks;
    }//end else

    if((n = (howmany)) > 0) {
        while(n > X_PADSIZE) {
            ret += x___printf_puts(io, with, X_PADSIZE);
            n -= X_PADSIZE;
        }//end while
        ret += x___printf_puts(io, with, n);
    }//end if

    return ret;
}//end x___printf_pad

int x___printf_out(struct x___printf_io *io, const struct x_printf_info *pi, const void *ptr, int len) {
    int ret;

    ret = 0;
    if((!pi->left) && pi->width > len) {
        ret += x___printf_pad(io, pi->width - len, pi->pad == '0');
    }//end if
    ret += x___printf_puts(io, ptr, len);
    if(pi->left && pi->width > len) {
        ret += x___printf_pad(io, pi->width - len, pi->pad == '0');
    }//end if

    return ret;
}//end x___printf_out


/* percent handling  -------------------------------------------------*/

int x___printf_arginfo_pct(const struct x_printf_info *pi, size_t n, int *argt) {
    return 0;
}//end x___printf_arginfo_pct

int x___printf_render_pct(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    return x___printf_puts(io, "%", 1);
}//end x___printf_render_pct

/* 'n' ---------------------------------------------------------------*/

int x___printf_arginfo_n(const struct x_printf_info *pi, size_t n, int *argt) {
    argt[0] = X_PA_POINTER;
    return 1;
}//end x___printf_arginfo_n

/*
 * This is a printf_render so that all output has been flushed before it
 * gets called.
 */

int x___printf_render_n(x_FILE *io, const struct x_printf_info *pi, const void *const *arg) {

    if(pi->is_char) {
        **((signed char **)arg[0]) = (signed char)pi->sofar;
    }//end if
    else if(pi->is_short) {
        **((short **)arg[0]) = (short)pi->sofar;
    }//end if
    else if(pi->is_long) {
        **((long **)arg[0]) = pi->sofar;
    }//end if
    else if(pi->is_long_double) {
        **((long long **)arg[0]) = pi->sofar;
    }//end if
    else if(pi->is_intmax) {
        **((intmax_t **)arg[0]) = pi->sofar;
    }//end if
    else if(pi->is_ptrdiff) {
        **((ptrdiff_t **)arg[0]) = pi->sofar;
    }//end if
    else if(pi->is_quad) {
        **((quad_t **)arg[0]) = pi->sofar;
    }//end if
    else if(pi->is_size) {
        **((size_t **)arg[0]) = pi->sofar;
    }//end if
    else {
        **((int **)arg[0]) = pi->sofar;
    }//end else

    return 0;
}//end x___printf_render_n

/* dynamic array handling  -------------------------------------------------*/
#define X_ARRAYDELTA 8

struct x_array {
    void    *data;
    int     itemsize;
    int     max;
};

static void x_arrayfree(struct x_array *a) {
    if(a) {
        free(a->data);
    }//end if
}//end x_arrayfree

static void *x_arrayget(struct x_array *a, int i) {
    int     oldsize, newmax, newsize;
    void    *newdata;

    if(i >= a->max) {
        oldsize = a->max * a->itemsize;
        newmax = i + X_ARRAYDELTA;
        newsize = newmax * a->itemsize;

        newdata = realloc(a->data, newsize);
        if(!newdata) {
            return NULL;
        }//end if

        memset(newdata + oldsize, 0, newsize - oldsize);
        a->data = newdata;
        a->max = newmax;
    }//end if
    return a->data + i * a->itemsize;
}//end x_arrayget

static struct x_array *x_arrayinit(struct x_array *a, int itemsize) {
    a->data = X_CALLOC(X_ARRAYDELTA, itemsize);
    if(!a->data) {
        return NULL;
    }//end if
    a->itemsize = itemsize;
    a->max = X_ARRAYDELTA;
    return a;
}//end x_arrayinit

/* -------------------------------------------------------------------------*/

int x___printf_comp(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain) {
    int                     ch, pii, *argt, nextarg, maxarg, ret, n;
    const char              *fmt;
    struct x_array          piarr, argtarr, *pa, *aa;
    struct x_printf_info    *pi, *pil;

    ret = 0;
    fmt = pc->fmt;
    maxarg = 0;
    nextarg = 1;

    pa = x_arrayinit(&piarr, sizeof(*pi));
    if(!pa) {
        return X_EOF;
    }//end if

    aa = x_arrayinit(&argtarr, sizeof(*argt));
    if(!aa) {
        x_arrayfree(pa);
        return X_EOF;
    }//end if

    for(pii = 0; ; pii++) {
        pi = x_arrayget(pa, pii);
        if(!pi) {
            ret = X_EOF;
            goto error;
        }//end if
        pil = pi;
        if(*fmt == '\0') {
            break;
        }//end if
        pil = pi + 1;
        pi->prec = -1;
        pi->pad = ' ';
#if defined(X_VECTORS)
        pi->vsep = 'X'; /* Illegal value, changed to defaults later. */
#endif /* !defined(X_VECTORS) */
        pi->begin = pi->end = fmt;
        while(*fmt != '\0' && *fmt != '%') {
            pi->end = ++fmt;
        }//end while
        if(*fmt == '\0') {
            break;
        }//end if
        fmt++;
        for(;;) {
            pi->spec = *fmt;
            switch(pi->spec) {
            case ' ':
                /*-
                 * ``If the space and + flags both appear, the space
                 * flag will be ignored.''
                 *      -- ANSI X3J11
                 */
                if(pi->showsign == 0) {
                    pi->space = 1;
                    pi->signchar = ' ';
                }//end if
                fmt++;
                continue;
            case '#':
                pi->alt = 1;
                fmt++;
                continue;
#if defined(X_VECTORS)
            case ',': case ';': case ':': case '_':
                pi->vsep = pi->spec;
                fmt++;
                continue;
#endif /* !defined(X_VECTORS) */
            case '.':
                pi->prec = 0;
                fmt++;
                if(*fmt == '*') {
                    fmt++;
                    /* Look for *nn$ and deal with it */
                    n = 0;
                    while(*fmt != '\0' && x_is_digit(*fmt)) {
                        n *= 10;
                        n += x_to_digit(*fmt);
                        fmt++;
                    }//end while
                    if(*fmt == '$') {
                        if((n + 1) > maxarg) {
                            maxarg = (n + 1);
                        }//end if
                        fmt++;
                    }//end if
                    else {
                        n = nextarg++;
                    }//end else
                    pi->get_prec = n;
                    argt = (int *)x_arrayget(aa, n);
                    if(!argt) {
                        ret = X_EOF;
                        goto error;
                    }//end if
                    *argt = X_PA_INT;
                    continue;
                }//end if
                while(*fmt != '\0' && x_is_digit(*fmt)) {
                    pi->prec *= 10;
                    pi->prec += x_to_digit(*fmt);
                    fmt++;
                }//end while
                continue;
            case '-':
                pi->left = 1;
                fmt++;
                continue;
            case '+':
                pi->showsign = 1;
                pi->signchar = '+';
                fmt++;
                continue;
            case '*':
                fmt++;
                /* Look for *nn$ and deal with it */
                n = 0;
                while(*fmt != '\0' && x_is_digit(*fmt)) {
                    n *= 10;
                    n += x_to_digit(*fmt);
                    fmt++;
                }//end while
                if(*fmt == '$') {
                    if((n + 1) > maxarg) {
                        maxarg = (n + 1);
                    }//end if
                    fmt++;
                }//end if
                else {
                    n = nextarg++;
                }//end else
                pi->get_width = n;
                argt = (int *)x_arrayget(aa, n);
                if(!argt) {
                    ret = X_EOF;
                    goto error;
                }//end if
                *argt = X_PA_INT;
                continue;
            case '%':
                fmt++;
                break;
            case '\'':
                pi->group = 1;
                fmt++;
                continue;
            case '0':
                /*-
                 * ``Note that 0 is taken as a flag, not as the
                 * beginning of a field width.''
                 *      -- ANSI X3J11
                 */
                pi->pad = '0';
                fmt++;
                continue;
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
                n = 0;
                while(*fmt != '\0' && x_is_digit(*fmt)) {
                    n *= 10;
                    n += x_to_digit(*fmt);
                    fmt++;
                }//end while
                if(*fmt == '$') {
                    if(nextarg > maxarg) {
                        maxarg = nextarg;
                    }//end if
                    nextarg = n;
                    fmt++;
                }//end if
                else {
                    pi->width = n;
                }//end else
                continue;
#if 0
            case 'D':
            case 'O':
            case 'U':
                pi->spec += ('a' - 'A');
                pi->is_intmax = 0;
                if(pi->is_long_double || pi->is_quad) {
                    pi->is_long = 0;
                    pi->is_long_double = 1;
                }//end if
                else {
                    pi->is_long = 1;
                    pi->is_long_double = 0;
                }//end else
                fmt++;
                break;
#endif /* 0 */
            case 'j':
                pi->is_intmax = 1;
                fmt++;
                continue;
            case 'q':
                pi->is_long = 0;
                pi->is_quad = 1;
                fmt++;
                continue;
            case 'L':
                pi->is_long_double = 1;
                fmt++;
                continue;
            case 'h':
                fmt++;
                if(*fmt == 'h') {
                    fmt++;
                    pi->is_char = 1;
                }//end if
                else {
                    pi->is_short = 1;
                }//end else
                continue;
            case 'l':
                fmt++;
                if(*fmt == 'l') {
                    fmt++;
                    pi->is_long_double = 1;
                    pi->is_quad = 0;
                }//end if
                else {
                    pi->is_quad = 0;
                    pi->is_long = 1;
                }//end else
                continue;
            case 't':
                pi->is_ptrdiff = 1;
                fmt++;
                continue;
            case 'v':
#if defined(X_VECTORS)
                pi->is_vec = 1;
#endif /* !defined(X_VECTORS) */
                fmt++;
                continue;
            case 'z':
                pi->is_size = 1;
                fmt++;
                continue;
            default:
                fmt++;
                break;
            }//end switch
            if(x_printf_tbl_in_range(pi->spec)) {
                switch(domain->type[x_printf_tbl_index(pi->spec)]) {
                /* ignore X_PRINTF_DOMAIN_UNUSED until later */
                case X_PRINTF_DOMAIN_FLAG:
                    errx(1, "Unexpected flag: %c", pi->spec);
                case X_PRINTF_DOMAIN_GLIBC_API:
                case X_PRINTF_DOMAIN_FBSD_API:
                    /*
                     * Insure that there are always
                     * X___PRINTFMAXARG available.
                     */
                    if(!x_arrayget(aa, nextarg + X___PRINTFMAXARG - 1)) {
                        ret = X_EOF;
                        goto error;
                    }//end if
                    pi->context = domain->tbl[x_printf_tbl_index(pi->spec)].context;
                    ch = domain->tbl[x_printf_tbl_index(pi->spec)].arginfo(
                        pi, X___PRINTFMAXARG, x_arrayget(aa, nextarg));
                    if(ch > 0) {
                        pi->arg[0] = (void *)(long)nextarg;
                    }//end if
                    if(ch > 1) {
                        pi->arg[1] = (void *)(long)(nextarg + 1);
                    }//end if
                    nextarg += ch;
                    break;
                }//end switch
            }//end if
            break;
        }//end for
    }//end for

    if(nextarg > maxarg) {
        maxarg = nextarg;
    }//end if

    pc->argt = aa->data;
    pc->pi = pa->data;
    pc->pil = pil;
    pc->maxarg = ch = maxarg;
    if(ch < 1) {
        ch = 1;
    }//end if

    pc->args = (union x_arg *)malloc(ch * sizeof(*pc->args));
    if(!pc->args) {
        ret = X_EOF;
        goto error;
    }//end if

    for(pi = pc->pi; pi < pil; pi++) {
        if(pi->arg[0]) {
            pi->arg[0] = &pc->args[(long)pi->arg[0]];
        }//end if
        if(pi->arg[1]) {
            pi->arg[1] = &pc->args[(long)pi->arg[1]];
        }//end if
    }//end for

#if 0
    fprintf(stderr, "fmt0 <%s>\n", fmt0);
    fprintf(stderr, "pil %p\n", pil);
#endif /* 0 */

    pc->domain = domain;

    return ret;
error:
    x_arrayfree(pa);
    x_arrayfree(aa);
    return ret;
}//end x___printf_comp

int x___printf_exec(x_printf_comp_t restrict pc, x_FILE * restrict fp, va_list ap) {
    int                     ch, ret, n;
    char                    unknown;
    struct x_printf_info    *pi;
    struct x___printf_io    io;

    ret = 0;
    x___printf_init(&io);
    io.fp = fp;

    for(ch = 1; ch < pc->maxarg; ch++) {
#if 0
        fprintf(stderr, "arg %d %x\n", ch, pc->argt[ch]);
#endif /* 0 */
        switch(pc->argt[ch]) {
        case X_PA_CHAR:
            pc->args[ch].intarg = (char)va_arg(ap, int);
            break;
        case X_PA_INT:
            pc->args[ch].intarg = va_arg(ap, int);
            break;
        case X_PA_INT | X_PA_FLAG_SHORT:
            pc->args[ch].intarg = (short)va_arg(ap, int);
            break;
        case X_PA_INT | X_PA_FLAG_LONG:
            pc->args[ch].longarg = va_arg(ap, long);
            break;
        case X_PA_INT | X_PA_FLAG_INTMAX:
            pc->args[ch].intmaxarg = va_arg(ap, intmax_t);
            break;
        case X_PA_INT | X_PA_FLAG_QUAD:
            pc->args[ch].intmaxarg = va_arg(ap, quad_t);
            break;
        case X_PA_INT | X_PA_FLAG_LONG_LONG:
            pc->args[ch].intmaxarg = va_arg(ap, long long);
            break;
        case X_PA_INT | X_PA_FLAG_SIZE:
            pc->args[ch].intmaxarg = va_arg(ap, size_t);
            break;
        case X_PA_INT | X_PA_FLAG_PTRDIFF:
            pc->args[ch].intmaxarg = (unsigned long)va_arg(ap, ptrdiff_t);
            break;
        case X_PA_WCHAR:
            pc->args[ch].wintarg = va_arg(ap, wint_t);
            break;
        case X_PA_POINTER:
            pc->args[ch].pvoidarg = va_arg(ap, void *);
            break;
        case X_PA_STRING:
            pc->args[ch].pchararg = va_arg(ap, char *);
            break;
        case X_PA_WSTRING:
            pc->args[ch].pwchararg = va_arg(ap, wchar_t *);
            break;
        case X_PA_DOUBLE:
#if !defined(NO_FLOATING_POINT)
            pc->args[ch].doublearg = va_arg(ap, double);
#endif /* !!defined(NO_FLOATING_POINT) */
            break;
        case X_PA_DOUBLE | X_PA_FLAG_LONG_DOUBLE:
#if !defined(NO_FLOATING_POINT)
            pc->args[ch].longdoublearg = va_arg(ap, long double);
#endif /* !!defined(NO_FLOATING_POINT) */
            break;
#if defined(X_VECTORS)
        case X_PA_VECTOR:
            pc->args[ch].vectorarg = va_arg(ap, X_VECTORTYPE);
            break;
#endif /* !defined(X_VECTORS) */
        default:
            errx(1, "argtype = %x (fmt = \"%s\")\n", pc->argt[ch], pc->fmt);
        }//end switch
    }//end for

    for(pi = pc->pi; pi < pc->pil; pi++) {
#if 0
        fprintf(stderr, "pi %p", pi);
        fprintf(stderr, " spec '%c'", pi->spec);
        fprintf(stderr, " args %d", ((uintptr_t)pi->arg[0] - (uintptr_t)pc->args) / sizeof pc->args[0]);
        if(pi->width) {
            fprintf(stderr, " width %d", pi->width);
        }//end if
        if(pi->pad) {
            fprintf(stderr, " pad 0x%x", pi->pad);
        }//end if
        if(pi->left) {
            fprintf(stderr, " left");
        }//end if
        if(pi->showsign) {
            fprintf(stderr, " showsign");
        }//end if
        if(pi->signchar) {
            fprintf(stderr, " signchar 0x%x", pi->signchar);
        }//end if
        if(pi->prec != -1) {
            fprintf(stderr, " prec %d", pi->prec);
        }//end if
        if(pi->is_char) {
            fprintf(stderr, " char");
        }//end if
        if(pi->is_short) {
            fprintf(stderr, " short");
        }//end if
        if(pi->is_long) {
            fprintf(stderr, " long");
        }//end if
        if(pi->is_long_double) {
            fprintf(stderr, " long_double");
        }//end if
        fprintf(stderr, "\n");
        fprintf(stderr, "\t\"%.*s\"\n", pi->end - pi->begin, pi->begin);
#endif /* 0 */

        if(pi->get_width) {
            pi->width = pc->args[pi->get_width].intarg;
            /*-
             * ``A negative field width argument is taken as a
             * - flag followed by a positive field width.''
             *      -- ANSI X3J11
             * They don't exclude field widths read from args.
             */
            if(pi->width < 0) {
                pi->left = 1;
                pi->width = -pi->width;
            }//end if
        }//end if
        if(pi->get_prec) {
            pi->prec = pc->args[pi->get_prec].intarg;
        }//end if
        ret += x___printf_puts(&io, pi->begin, pi->end - pi->begin);
        if(pi->spec) {
            if(!x_printf_tbl_in_range(pi->spec)) {
                goto unused;
            }//end if
            switch(pc->domain->type[x_printf_tbl_index(pi->spec)]) {
            case X_PRINTF_DOMAIN_UNUSED:
        unused:
                unknown = pi->spec;
                ret += x___printf_out(&io, pi, &unknown, 1);
                break;
            case X_PRINTF_DOMAIN_GLIBC_API:
                x___printf_flush(&io);
                pi->sofar = ret;
                ret += ((x_printf_function *)pc->domain->tbl[x_printf_tbl_index(pi->spec)].render)(
                    fp, pi, (const void *)pi->arg);
                break;
            case X_PRINTF_DOMAIN_FBSD_API:
                pi->sofar = ret;
                n = ((x_printf_render *)pc->domain->tbl[x_printf_tbl_index(pi->spec)].render)(
                    &io, pi, (const void *)pi->arg);
                if(n < 0) {
                    io.fp->_flags |= X___SERR;
                }//end if
                else {
                    ret += n;
                }//end else
                break;
            }//end switch
        }//end if
    }//end for
    x___printf_flush(&io);
    return ret;
}//end x___printf_exec

int x___lprintf_exec(x_printf_comp_t restrict pc, x_FILE * restrict fp, void ** restrict args) {
    int                     ch, ret, n, i;
    char                    unknown;
    struct x_printf_info    *pi;
    struct x___printf_io    io;

    ret = 0;
    x___printf_init(&io);
    io.fp = fp;
    i = 0;

    for(ch = 1; ch < pc->maxarg; ch++) {
#if 0
        fprintf(stderr, "arg %d %x\n", ch, pc->argt[ch]);
#endif /* 0 */
        switch(pc->argt[ch]) {
        case X_PA_CHAR:
            pc->args[ch].intarg = (char)(uintptr_t)args[i++];
            break;
        case X_PA_INT:
            pc->args[ch].intarg = (int)(uintptr_t)args[i++];
            break;
        case X_PA_INT | X_PA_FLAG_SHORT:
            pc->args[ch].intarg = (short)(uintptr_t)args[i++];
            break;
        case X_PA_INT | X_PA_FLAG_LONG:
            pc->args[ch].longarg = (long)(uintptr_t)args[i++];
            break;
        case X_PA_INT | X_PA_FLAG_INTMAX:
            pc->args[ch].intmaxarg = (intmax_t)(uintptr_t)args[i++];
            break;
        case X_PA_INT | X_PA_FLAG_QUAD:
            pc->args[ch].intmaxarg = (quad_t)(uintptr_t)args[i++];
            break;
        case X_PA_INT | X_PA_FLAG_LONG_LONG:
            pc->args[ch].intmaxarg = (long long)(uintptr_t)args[i++];
            break;
        case X_PA_INT | X_PA_FLAG_SIZE:
            pc->args[ch].intmaxarg = (size_t)(uintptr_t)args[i++];
            break;
        case X_PA_INT | X_PA_FLAG_PTRDIFF:
            pc->args[ch].intmaxarg = (unsigned long)(ptrdiff_t)(uintptr_t)args[i++];
            break;
        case X_PA_WCHAR:
            pc->args[ch].wintarg = (wint_t)(uintptr_t)args[i++];
            break;
        case X_PA_POINTER:
            pc->args[ch].pvoidarg = (void *)args[i++];
            break;
        case X_PA_STRING:
            pc->args[ch].pchararg = (char *)args[i++];
            break;
        case X_PA_WSTRING:
            pc->args[ch].pwchararg = (wchar_t *)(void *)args[i++];
            break;
        case X_PA_DOUBLE:
#if !defined(NO_FLOATING_POINT)
            pc->args[ch].doublearg = (double)(uintptr_t)args[i++];
#endif /* !!defined(NO_FLOATING_POINT) */
            break;
        case X_PA_DOUBLE | X_PA_FLAG_LONG_DOUBLE:
#if !defined(NO_FLOATING_POINT)
            pc->args[ch].longdoublearg = (long double)(uintptr_t)args[i++];
#endif /* !defined(NO_FLOATING_POINT) */
            break;
#if defined(X_VECTORS)
        case X_PA_VECTOR:
            pc->args[ch].vectorarg = *((X_VECTORTYPE *)args[i++]);
            break;
#endif /* !defined(X_VECTORS) */
        default:
            errx(1, "argtype = %x (fmt = \"%s\")\n", pc->argt[ch], pc->fmt);
        }//end switch
    }//end for

    for(pi = pc->pi; pi < pc->pil; pi++) {
#if 0
        fprintf(stderr, "pi %p", pi);
        fprintf(stderr, " spec '%c'", pi->spec);
        fprintf(stderr, " args %d", ((uintptr_t)pi->arg[0] - (uintptr_t)pc->args) / sizeof pc->args[0]);
        if(pi->width) {
            fprintf(stderr, " width %d", pi->width);
        }//end if
        if(pi->pad) {
            fprintf(stderr, " pad 0x%x", pi->pad);
        }//end if
        if(pi->left) {
            fprintf(stderr, " left");
        }//end if
        if(pi->showsign) {
            fprintf(stderr, " showsign");
        }//end if
        if(pi->signchar) {
            fprintf(stderr, " signchar 0x%x", pi->signchar);
        }//end if
        if(pi->prec != -1) {
            fprintf(stderr, " prec %d", pi->prec);
        }//end if
        if(pi->is_char) {
            fprintf(stderr, " char");
        }//end if
        if(pi->is_short) {
            fprintf(stderr, " short");
        }//end if
        if(pi->is_long) {
            fprintf(stderr, " long");
        }//end if
        if(pi->is_long_double) {
            fprintf(stderr, " long_double");
        }//end if
        fprintf(stderr, "\n");
        fprintf(stderr, "\t\"%.*s\"\n", pi->end - pi->begin, pi->begin);
#endif /* 0 */

        if(pi->get_width) {
            pi->width = pc->args[pi->get_width].intarg;
            /*-
             * ``A negative field width argument is taken as a
             * - flag followed by a positive field width.''
             *      -- ANSI X3J11
             * They don't exclude field widths read from args.
             */
            if(pi->width < 0) {
                pi->left = 1;
                pi->width = -pi->width;
            }//end if
        }//end if
        if(pi->get_prec) {
            pi->prec = pc->args[pi->get_prec].intarg;
        }//end if
        ret += x___printf_puts(&io, pi->begin, pi->end - pi->begin);
        if(pi->spec) {
            if(!x_printf_tbl_in_range(pi->spec)) {
                goto unused;
            }//end if
            switch(pc->domain->type[x_printf_tbl_index(pi->spec)]) {
            case X_PRINTF_DOMAIN_UNUSED:
        unused:
                unknown = pi->spec;
                ret += x___printf_out(&io, pi, &unknown, 1);
                break;
            case X_PRINTF_DOMAIN_GLIBC_API:
                x___printf_flush(&io);
                pi->sofar = ret;
                ret += ((x_printf_function *)pc->domain->tbl[x_printf_tbl_index(pi->spec)].render)(
                    fp, pi, (const void *)pi->arg);
                break;
            case X_PRINTF_DOMAIN_FBSD_API:
                pi->sofar = ret;
                n = ((x_printf_render *)pc->domain->tbl[x_printf_tbl_index(pi->spec)].render)(
                    &io, pi, (const void *)pi->arg);
                if(n < 0) {
                    io.fp->_flags |= X___SERR;
                }//end if
                else {
                    ret += n;
                }//end else
                break;
            }//end switch
        }//end if
    }//end for
    x___printf_flush(&io);
    return ret;
}//end x___lprintf_exec

int x___v2printf(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain, x_FILE * restrict fp, const char * restrict fmt, va_list ap) {
    int                         ret, saverrno;
    struct x__printf_compiled   spc;

    /*
     * All the printf family (including extensible printf variants) funnel
     * down to this point.  So we can do common work here, and then fork
     * out to the appropriate handler.
     */
    /* sorry, fprintf(read_only_file, "") returns X_EOF, not 0 */
    if(x_prepwrite(fp) != 0) {
        errno = EBADF;
        return X_EOF;
    }//end if
    X_ORIENT(fp, -1);

    if(pc) {
        pthread_mutex_lock(&pc->mutex);
        pthread_rwlock_rdlock(&pc->domain->rwlock);
        ret = x___printf_exec(pc, fp, ap);
        saverrno = errno;
        pthread_rwlock_unlock(&pc->domain->rwlock);
        pthread_mutex_unlock(&pc->mutex);
        errno = saverrno;
        return ret;
    }//end if
    if(!domain) {
        errno = EINVAL;
        return X_EOF;
    }//end if
    x_xprintf_domain_init();
    memset(&spc, 0, sizeof(spc));
    spc.fmt = fmt;
    /*
     * We don't need to lock the x_printf_comp_t mutex, since the
     * x_printf_comp_t was just created on the stack, and is private.
     */
    pthread_rwlock_rdlock(&domain->rwlock);
    if(x___printf_comp(&spc, domain) < 0) {
        saverrno = errno;
        pthread_rwlock_unlock(&domain->rwlock);
        errno = saverrno;
        return X_EOF;
    }//end if
    ret = x___printf_exec(&spc, fp, ap);
    saverrno = errno;
    pthread_rwlock_unlock(&domain->rwlock);

    free(spc.pi);
    free(spc.argt);
    free(spc.args);
    errno = saverrno;
    return ret;
}//end x___v2printf

int x___l2printf(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain, x_FILE * restrict fp, const char * restrict fmt, void ** restrict args) {
    int                         ret, saverrno;
    struct x__printf_compiled   spc;

    /*
     * All the printf family (including extensible printf variants) funnel
     * down to this point.  So we can do common work here, and then fork
     * out to the appropriate handler.
     */
    /* sorry, fprintf(read_only_file, "") returns X_EOF, not 0 */
    if(x_prepwrite(fp) != 0) {
        errno = EBADF;
        return X_EOF;
    }//end if
    X_ORIENT(fp, -1);

    if(pc) {
        pthread_mutex_lock(&pc->mutex);
        pthread_rwlock_rdlock(&pc->domain->rwlock);
        ret = x___lprintf_exec(pc, fp, args);
        saverrno = errno;
        pthread_rwlock_unlock(&pc->domain->rwlock);
        pthread_mutex_unlock(&pc->mutex);
        errno = saverrno;
        return ret;
    }//end if
    if(!domain) {
        errno = EINVAL;
        return X_EOF;
    }//end if
    x_xprintf_domain_init();
    memset(&spc, 0, sizeof(spc));
    spc.fmt = fmt;
    /*
     * We don't need to lock the x_printf_comp_t mutex, since the
     * x_printf_comp_t was just created on the stack, and is private.
     */
    pthread_rwlock_rdlock(&domain->rwlock);
    if(x___printf_comp(&spc, domain) < 0) {
        saverrno = errno;
        pthread_rwlock_unlock(&domain->rwlock);
        errno = saverrno;
        return X_EOF;
    }//end if
    ret = x___lprintf_exec(&spc, fp, args);
    saverrno = errno;
    pthread_rwlock_unlock(&domain->rwlock);

    free(spc.pi);
    free(spc.argt);
    free(spc.args);
    errno = saverrno;
    return ret;
}//end x___l2printf

/*
 * Helper function for `fprintf to unbuffered unix file': creates a
 * temporary buffer.  We only work on write-only files; this avoids
 * worries about ungetc buffers and so forth.
 */
static int x___v3printf(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain, x_FILE * restrict fp, const char * restrict fmt, va_list ap) {
    int                 ret;
    x_FILE              fake;
    struct x___sFILEX   extra;
    unsigned char       buf[X_BUFSIZ];

    fake._extra = &extra;
    X_INITEXTRA(&fake);

    /* copy the important variables */
    fake._flags = fp->_flags & ~X___SNBF;
    fake._file = fp->_file;
    fake._cookie = fp->_cookie;
    fake._write = fp->_write;
    fake._orientation = fp->_orientation;
    fake._mbstate = fp->_mbstate;

    /* set up the buffer */
    fake._bf._base = fake._p = buf;
    fake._bf._size = fake._w = sizeof(buf);
    fake._lbfsize = 0;    /* not actually used, but Just In Case */

    /* do the work, then copy any error status */
    ret = x___v2printf(pc, domain, &fake, fmt, ap);
    if(ret >= 0 && x___fflush(&fake)) {
        ret = X_EOF;
    }//end if
    if(fake._flags & X___SERR) {
        fp->_flags |= X___SERR;
    }//end if

    return ret;
}//end x___v3printf

static int x___l3printf(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain, x_FILE * restrict fp, const char * restrict fmt, void ** restrict args) {
    int                 ret;
    x_FILE              fake;
    struct x___sFILEX   extra;
    unsigned char       buf[X_BUFSIZ];

    fake._extra = &extra;
    X_INITEXTRA(&fake);

    /* copy the important variables */
    fake._flags = fp->_flags & ~X___SNBF;
    fake._file = fp->_file;
    fake._cookie = fp->_cookie;
    fake._write = fp->_write;
    fake._orientation = fp->_orientation;
    fake._mbstate = fp->_mbstate;

    /* set up the buffer */
    fake._bf._base = fake._p = buf;
    fake._bf._size = fake._w = sizeof(buf);
    fake._lbfsize = 0;    /* not actually used, but Just In Case */

    /* do the work, then copy any error status */
    ret = x___l2printf(pc, domain, &fake, fmt, args);
    if(ret >= 0 && x___fflush(&fake)) {
        ret = X_EOF;
    }//end if
    if(fake._flags & X___SERR) {
        fp->_flags |= X___SERR;
    }//end if

    return ret;
}//end x___l3printf

int x___xvprintf(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain, x_FILE * restrict fp, const char * restrict fmt0, va_list ap) {
    int ret;

    /* optimise fprintf(stderr) (and other unbuffered Unix files) */
    if((fp->_flags & (X___SNBF | X___SWR | X___SRW)) == (X___SNBF | X___SWR) &&
        fp->_file >= 0) {
        ret = x___v3printf(pc, domain, fp, fmt0, ap);
    }//end if
    else {
        ret = x___v2printf(pc, domain, fp, fmt0, ap);
    }//end else

    return ret;
}//end x___xvprintf

int x___xlprintf(x_printf_comp_t restrict pc, x_printf_domain_t restrict domain, x_FILE * restrict fp, const char * restrict fmt0, void ** restrict args) {
    int ret;

    /* optimise fprintf(stderr) (and other unbuffered Unix files) */
    if((fp->_flags & (X___SNBF | X___SWR | X___SRW)) == (X___SNBF | X___SWR) &&
        fp->_file >= 0) {
        ret = x___l3printf(pc, domain, fp, fmt0, args);
    }//end if
    else {
        ret = x___l2printf(pc, domain, fp, fmt0, args);
    }//end else

    return ret;
}//end x___xlprintf

/* extending ---------------------------------------------------------*/

/* No global domain support */
#if 0
int x_register_printf_function(int spec, x_printf_function *render, x_printf_arginfo_function *arginfo) {
    return x_register_printf_domain_function(NULL, spec, render, arginfo);
}//end x_register_printf_function

int x_register_printf_render(int spec, x_printf_render *render, x_printf_arginfo_function *arginfo) {
    return x_register_printf_domain_render(NULL, spec, render, arginfo);
}//end x_register_printf_render

int x_register_printf_render_std(const char *specs) {
    return x_register_printf_domain_render_std(NULL, specs);
}//end x_register_printf_render_std
#endif /* 0 */

#if defined(X_VECTORS)
/* vector support ----------------------------------------------------*/

#define X_PRINTVECTOR(_io, _pi, _arg, _cnt, _type, _elem, _render, _ret) \
    do {                                                        \
        int i;                                                  \
        _type a, *ap;                                           \
        a = (_type)(_arg)->_elem[0];                            \
        ap = &a;                                                \
        (_ret) += _render((_io), (_pi), (const void *)&ap);     \
        for(i = 1; i < (_cnt); i++) {                           \
            (_ret) += x___printf_puts((_io), (_pi)->begin, (_pi)->end - (_pi)->begin); \
            a = (_type)(_arg)->_elem[i];                        \
            (_ret) += _render((_io), (_pi), (const void *)&ap); \
        }                                                       \
    } while(0)

#define X_PRINTVECTOR_P(_io, _pi, _arg, _cnt, _elem, _render, _ret) \
    do {                                                        \
        int     i;                                              \
        void    *a, *ap;                                        \
        a = (void *)(uintptr_t)(_arg)->_elem[0];                \
        ap = &a;                                                \
        (_ret) += _render((_io), (_pi), (const void *)&ap);     \
        for(i = 1; i < (_cnt); i++) {                           \
            (_ret) += x___printf_puts((_io), (_pi)->begin, (_pi)->end - (_pi)->begin); \
            a = (void *)(uintptr_t)(_arg)->_elem[i];            \
            (_ret) += _render((_io), (_pi), (const void *)&ap); \
        }                                                       \
    } while(0)

int x___xprintf_vector(struct x___printf_io *io, const struct x_printf_info *pi, const void *const *arg) {
    int                     ret;
    char                    vsep;    /* Vector separator character. */
    const union x_arg       *argp;
    struct x_printf_info    info;

    ret = 0;
    info = *pi;
    argp = arg[0];
    vsep = pi->vsep;
    if(vsep == 'X') {
        if(pi->spec == 'c') {
            vsep = '\0';
        }//end if
        else {
            vsep = ' ';
        }//end else
    }//end if
    info.begin = info.end = &vsep;
    if(vsep) {
        info.end++;
    }//end if
    info.is_vec = 0;

    if(pi->is_short) {
        if(pi->spec == 'p') {
            X_PRINTVECTOR_P(io, &info, argp, 8, vushortarg, x___printf_render_ptr, ret);
        }//end if
        else {
            X_PRINTVECTOR(io, &info, argp, 8, unsigned int, vushortarg, x___printf_render_int, ret);
        }//end else
    }//end if
    else if(pi->is_long) {
        info.is_long = 0;
        if(pi->spec == 'p') {
            X_PRINTVECTOR_P(io, &info, argp, 4, vuintarg, x___printf_render_ptr, ret);
        }//end if
        else {
            X_PRINTVECTOR(io, &info, argp, 4, unsigned int, vuintarg, x___printf_render_int, ret);
        }//end else
#if defined(X_V64TYPE)
    }//end if
    else if(pi->is_long_double) {
        switch(pi->spec) {
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G':
            info.is_long_double = 0;
            X_PRINTVECTOR(io, &info, argp, 2, double, vdoublearg, x___printf_render_float, ret);
            break;
        case 'p':
            info.is_long_double = 0;
            X_PRINTVECTOR_P(io, &info, argp, 2, vulonglongarg, x___printf_render_ptr, ret);
            break;
        case 'd':
        case 'i':
        case 'u':
        case 'o':
        case 'x':
        case 'X':
            X_PRINTVECTOR(io, &info, argp, 2, unsigned long long, vulonglongarg, x___printf_render_int, ret);
            break;
        default:
            /*
             * The default case should never
             * happen.
             */
        case 'c':
            info.is_long_double = 0;
            X_PRINTVECTOR(io, &info, argp, 16, unsigned int, vuchararg, x___printf_render_chr, ret);
        }//end switch
#endif /* !defined(X_V64TYPE) */
    }//end if
    else {
        switch(pi->spec) {
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G':
            X_PRINTVECTOR(io, &info, argp, 4, double, vfloatarg, x___printf_render_float, ret);
            break;
        default:
            /*
             * The default case should never
             * happen.
             */
        case 'p':
            X_PRINTVECTOR_P(io, &info, argp, 16, vuchararg, x___printf_render_ptr, ret);
            break;
        case 'd':
        case 'i':
        case 'u':
        case 'o':
        case 'x':
        case 'X':
            info.is_char = 1;
            X_PRINTVECTOR(io, &info, argp, 16, unsigned int, vuchararg, x___printf_render_int, ret);
            break;
        case 'c':
            X_PRINTVECTOR(io, &info, argp, 16, unsigned int, vuchararg, x___printf_render_chr, ret);
        }//end switch
    }//end else

    return ret;
}//end x___xprintf_vector

#endif /* !defined(X_VECTORS) */
