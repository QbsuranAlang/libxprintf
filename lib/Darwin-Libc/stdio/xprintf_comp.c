/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/xprintf_comp.c
 *
 */


#include <pthread.h>
#include <string.h>
#include <errno.h>

#include <libxprintf/libxprintf.h>

#include "x_xprintf_domain.h"
#include "x_xprintf_private.h"

void x_free_printf_comp(x_printf_comp_t pc) {

    if(!pc) {
        return;
    }//end if

    free(pc->pi);
    free(pc->argt);
    free(pc->args);
    pthread_mutex_destroy(&pc->mutex);
    free(pc);
}//end x_free_printf_comp

x_printf_comp_t x_new_printf_comp(x_printf_domain_t restrict domain, const char * restrict fmt) {
    int                         ret, saverrno;
    x_printf_comp_t restrict    pc;

    if(!domain) {
        errno = EINVAL;
        return NULL;
    }//end if

    pc = X_MALLOC(sizeof(*pc) + strlen(fmt) + 1);
    if(!pc) {
        return NULL;
    }//end if

    memset(pc, 0, sizeof(*pc));
    pc->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pc->fmt = (const char *)(pc + 1);
    strcpy((char *)pc->fmt, fmt);

    x_xprintf_domain_init();
    pthread_rwlock_rdlock(&domain->rwlock);
    ret = x___printf_comp(pc, domain);
    saverrno = errno;
    pthread_rwlock_unlock(&domain->rwlock);

    if(ret < 0) {
        pthread_mutex_destroy(&pc->mutex);
        free(pc);
        errno = saverrno;
        return NULL;
    }//end if

    return pc;
}//end x_new_printf_comp

int x_new_printf_comp_dry_run(x_printf_domain_t restrict domain, const char * restrict fmt) {
    int                         ret;
    struct x_printf_info        *pi;
    x_printf_comp_t restrict    pc;

    pc = x_new_printf_comp(domain, fmt);
    if(!pc) {
        return -1;
    }//end if

    ret = 0;
    for(pi = pc->pi; pi < pc->pil; pi++) {
        if(pi->spec) {
            if((!x_printf_tbl_in_range(pi->spec)) || !pc->domain->tbl[x_printf_tbl_index(pi->spec)].render) {
                errno = EINVAL;
                ret = -1;
                break;
            }//end if
        }//end if
    }//end for

    x_free_printf_comp(pc);
    return ret;
}//end x_new_printf_comp_dry_run
