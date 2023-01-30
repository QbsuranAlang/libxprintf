/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/xprintf_exec.c
 *
 */


#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

#include <libxprintf/libxprintf.h>

#include "x_local.h"
#include "x_xprintf_private.h"

int x_asxprintf_exec(char ** __restrict ret, x_printf_comp_t __restrict pc, ...) {
    int     iret;
    va_list ap;

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    va_start(ap, pc);
    iret = x__vasprintf(pc, NULL, ret, NULL, ap);
    va_end(ap);

    return iret;
}//end x_asxprintf_exec

int x_dxprintf_exec(int fd, x_printf_comp_t __restrict pc, ...) {
    int     ret;
    va_list ap;

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    va_start(ap, pc);
    ret = x__vdprintf(pc, NULL, fd, NULL, ap);
    va_end(ap);

    return ret;
}//end x_dxprintf_exec

int x_fxprintf_exec(x_FILE * __restrict stream, x_printf_comp_t __restrict pc, ...) {
    int     ret;
    va_list ap;

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    va_start(ap, pc);
    ret = x___xvprintf(pc, NULL, stream, NULL, ap);
    va_end(ap);

    return ret;
}//end x_fxprintf_exec

int x_fxprintf_std_exec(FILE * __restrict stream, x_printf_comp_t __restrict pc, ...) {
    int     ret;
    char    *str;
    va_list ap;

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    va_start(ap, pc);
    ret = x_vasxprintf_exec(&str, pc, ap);
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
}//end x_fxprintf_std_exec

int x_sxprintf_exec(char * __restrict str, size_t size, x_printf_comp_t __restrict pc, ...) {
    int     ret;
    va_list ap;

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    va_start(ap, pc);
    ret = x__vsnprintf(pc, NULL, str, size, NULL, ap);
    va_end(ap);

    return ret;
}//end x_sxprintf_exec

int x_xprintf_exec(x_printf_comp_t __restrict pc, ...) {
    int     ret;
    va_list ap;

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    va_start(ap, pc);
    ret = x_vfxprintf_std_exec(stdout, pc, ap);
    va_end(ap);

    return ret;
}//end x_xprintf_exec

int x_vasxprintf_exec(char ** __restrict ret, x_printf_comp_t __restrict pc, va_list ap) {

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    return x__vasprintf(pc, NULL, ret, NULL, ap);
}//end x_vasxprintf_exec

int x_vdxprintf_exec(int fd, x_printf_comp_t __restrict pc, va_list ap) {

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    return x__vdprintf(pc, NULL, fd, NULL, ap);
}//end x_vdxprintf_exec

int x_vfxprintf_exec(x_FILE * __restrict stream, x_printf_comp_t __restrict pc, va_list ap) {

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    return x___xvprintf(pc, NULL, stream, NULL, ap);
}//end x_vfxprintf_exec

int x_vfxprintf_std_exec(FILE * __restrict stream, x_printf_comp_t __restrict pc, va_list ap) {
    int     ret;
    char    *str;

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    ret = x_vasxprintf_exec(&str, pc, ap);
    if(ret == -1) {
        return -1;
    }//end if

    if(fwrite(str, sizeof(char), ret, stream) != ret) {
        free(str);
        return -1;
    }//end if
    free(str);

    return ret;
}//end x_vfxprintf_std_exec

int x_vsxprintf_exec(char * __restrict str, size_t size, x_printf_comp_t __restrict pc, va_list ap) {

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    return x__vsnprintf(pc, NULL, str, size, NULL, ap);
}//end x_vsxprintf_exec

int x_vxprintf_exec(x_printf_comp_t __restrict pc, va_list ap) {

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    return x_vfxprintf_std_exec(stdout, pc, ap);
}//end x_vxprintf_exec

int x_lasxprintf_exec(char ** __restrict ret, x_printf_comp_t __restrict pc, void ** __restrict args) {

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    return x__lasprintf(pc, NULL, ret, NULL, args);
}//end x_lasxprintf_exec

int x_ldxprintf_exec(int fd, x_printf_comp_t __restrict pc, void ** __restrict args) {

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    return x__ldprintf(pc, NULL, fd, NULL, args);
}//end x_ldxprintf_exec

int x_vlxprintf_exec(x_FILE * __restrict stream, x_printf_comp_t __restrict pc, void ** __restrict args) {

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    return x___xlprintf(pc, NULL, stream, NULL, args);
}//end x_vlxprintf_exec

int x_lfxprintf_std_exec(FILE * __restrict stream, x_printf_comp_t __restrict pc, void ** __restrict args) {
    int     ret;
    char    *str;

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    ret = x_lasxprintf_exec(&str, pc, args);
    if(ret == -1) {
        return -1;
    }//end if

    if(fwrite(str, sizeof(char), ret, stream) != ret) {
        free(str);
        return -1;
    }//end if
    free(str);

    return ret;
}//end x_lfxprintf_std_exec

int x_lsxprintf_exec(char * __restrict str, size_t size, x_printf_comp_t __restrict pc, void ** __restrict args) {

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    return x__lsnprintf(pc, NULL, str, size, NULL, args);
}//end x_lsxprintf_exec

int x_lxprintf_exec(x_printf_comp_t __restrict pc, void ** __restrict args) {

    if(!pc) {
        errno = EINVAL;
        return -1;
    }//end if

    return x_lfxprintf_std_exec(stdout, pc, args);
}//end x_lxprintf_exec
