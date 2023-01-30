/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/xprintf_all_in_one.c
 *
 */


#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include <libxprintf/libxprintf.h>

#include "x_local.h"
#include "x_xprintf_private.h"

int x_asxprintf(char ** __restrict ret, x_printf_domain_t __restrict domain, const char * __restrict format, ...) {
    int     iret;
    va_list ap;

    va_start(ap, format);
    iret = x__vasprintf(NULL, domain, ret, format, ap);
    va_end(ap);

    return iret;
}//end x_asxprintf

int x_dxprintf(int fd, x_printf_domain_t __restrict domain, const char * __restrict format, ...) {
    int     ret;
    va_list ap;

    va_start(ap, format);
    ret = x__vdprintf(NULL, domain, fd, format, ap);
    va_end(ap);

    return ret;
}//end x_dxprintf

int x_fxprintf(x_FILE * __restrict stream, x_printf_domain_t __restrict domain, const char * __restrict format, ...) {
    int     ret;
    va_list ap;

    va_start(ap, format);
    ret = x___xvprintf(NULL, domain, stream, format, ap);
    va_end(ap);

    return ret;
}//end x_fxprintf

int x_fxprintf_std(FILE * __restrict stream, x_printf_domain_t __restrict domain, const char * __restrict format, ...) {
    int     ret;
    char    *str;
    va_list ap;

    va_start(ap, format);
    ret = x_vasxprintf(&str, domain, format, ap);
    va_end(ap);

    if(ret == -1) {
        return -1;
    }//end if

    if(fwrite(str, sizeof(char), ret, stream) != ret) {
        free(str);
        return -1;
    }//end if
    free(str);

    return ret;
}//end x_fxprintf_std

int x_sxprintf(char * __restrict str, size_t size, x_printf_domain_t __restrict domain, const char * __restrict format, ...) {
    int     ret;
    va_list ap;

    va_start(ap, format);
    ret = x__vsnprintf(NULL, domain, str, size, format, ap);
    va_end(ap);

    return ret;
}//end x_sxprintf

int x_xprintf(x_printf_domain_t __restrict domain, const char * __restrict format, ...) {
    int     ret;
    va_list ap;

    va_start(ap, format);
    ret = x_vfxprintf_std(stdout, domain, format, ap);
    va_end(ap);

    return ret;
}//end x_xprintf

int x_vasxprintf(char ** __restrict ret, x_printf_domain_t __restrict domain, const char * __restrict format, va_list ap) {
    return x__vasprintf(NULL, domain, ret, format, ap);
}//end x_vasxprintf

int x_vdxprintf(int fd, x_printf_domain_t __restrict domain, const char * __restrict format, va_list ap) {
    return x__vdprintf(NULL, domain, fd, format, ap);
}//end x_vdxprintf

int x_vfxprintf(x_FILE * __restrict stream, x_printf_domain_t __restrict domain, const char * __restrict format, va_list ap) {
    return x___xvprintf(NULL, domain, stream, format, ap);
}//end x_vfxprintf

int x_vfxprintf_std(FILE * __restrict stream, x_printf_domain_t __restrict domain, const char * __restrict format, va_list ap) {
    int     ret;
    char    *str;

    ret = x_vasxprintf(&str, domain, format, ap);
    if(ret == -1) {
        return -1;
    }//end if

    if(fwrite(str, sizeof(char), ret, stream) != ret) {
        free(str);
        return -1;
    }//end if
    free(str);

    return ret;
}//end x_vfxprintf_std

int x_vsxprintf(char * __restrict str, size_t size, x_printf_domain_t __restrict domain, const char * __restrict format, va_list ap) {
    return x__vsnprintf(NULL, domain, str, size, format, ap);
}//end x_vsxprintf

int x_vxprintf(x_printf_domain_t __restrict domain, const char * __restrict format, va_list ap) {
    return x_vfxprintf_std(stdout, domain, format, ap);
}//end x_vxprintf

int x_lasxprintf(char ** __restrict ret, x_printf_domain_t __restrict domain, const char * __restrict format, void ** __restrict args) {
    return x__lasprintf(NULL, domain, ret, format, args);
}//end x_lasxprintf

int x_ldxprintf(int fd, x_printf_domain_t __restrict domain, const char * __restrict format, void ** __restrict args) {
    return x__ldprintf(NULL, domain, fd, format, args);
}//end x_ldxprintf

int x_lfxprintf(x_FILE * __restrict stream, x_printf_domain_t __restrict domain, const char * __restrict format, void ** __restrict args) {
    return x___xlprintf(NULL, domain, stream, format, args);
}//end x_lfxprintf

int x_lfxprintf_std(FILE * __restrict stream, x_printf_domain_t __restrict domain, const char * __restrict format, void ** __restrict args) {
    int     ret;
    char    *str;

    ret = x_lasxprintf(&str, domain, format, args);
    if(ret == -1) {
        return -1;
    }//end if

    if(fwrite(str, sizeof(char), ret, stream) != ret) {
        free(str);
        return -1;
    }//end if
    free(str);

    return ret;
}//end x_lfxprintf_std

int x_lsxprintf(char * __restrict str, size_t size, x_printf_domain_t __restrict domain, const char * __restrict format, void ** __restrict args) {
    return x__lsnprintf(NULL, domain, str, size, format, args);
}//end x_lsxprintf

int x_lxprintf(x_printf_domain_t __restrict domain, const char * __restrict format, void ** __restrict args) {
    return x_lfxprintf_std(stdout, domain, format, args);
}//end x_lxprintf
