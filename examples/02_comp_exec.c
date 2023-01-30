/*
 * APPLE LICENSE
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>

#include <libxprintf/libxprintf.h>

#include "xprintf_callback.h"

#define X_IPV4_STR "169.254.2.1"
#define X_IPV6_STR "2002:b013:35a:32b3:cf9:819b:b35:5de7"

static void pass_arguments_by_variable_arguments(void) {
    int                 ret;
    char                *tmp, buf[128];
    in_addr_t           in_addr;
    struct in6_addr     in6_addr;
    x_printf_comp_t     comp;
    x_printf_domain_t   domain, copyed_domain;

    /* copy a domain from default(x_xprintf_domain_default) */
    domain = x_new_printf_domain();
    if(!domain) {
        perror("x_new_printf_domain()");
        exit(1);
    }//end if

    /* register printer */
    ret = x_register_printf_domain_function(domain, 'A', xprintf_function_ipv4_address, xprintf_arginfo_function_ipv4_address, NULL);
    if(ret != 0) {
        perror("x_register_printf_domain_function()");
        exit(1);
    }//end if

    ret = x_register_printf_domain_function(domain, 'B', xprintf_function_ipv6_address, xprintf_arginfo_function_ipv6_address, domain);
    if(ret != 0) {
        perror("x_register_printf_domain_function()");
        exit(1);
    }//end if

    ret = x_register_printf_domain_function(domain, 'C', xprintf_function_add, xprintf_arginfo_function_add, domain);
    if(ret != 0) {
        perror("x_register_printf_domain_function()");
        exit(1);
    }//end if

    /* compile the format */
    comp = x_new_printf_comp(domain, "%s: fmt \'%%A %%B %%C\' got \'%A %B %C\'\n");
    if(!comp) {
        perror("x_new_printf_comp()");
        exit(1);
    }//end if

    /* dry run compile the format, %! should not be taken */
    if(x_new_printf_comp_dry_run(domain, "%A %B %C %!") != 0) {
        perror("x_new_printf_comp_dry_run()");
        fflush(stderr);
    }//end if

    /* get the binary IP address */
    if(inet_pton(PF_INET, X_IPV4_STR, &in_addr) != 1) {
        perror("inet_pton()");
        exit(1);
    }//end if

    if(inet_pton(PF_INET6, X_IPV6_STR, &in6_addr) != 1) {
        perror("inet_pton()");
        exit(1);
    }//end if

    /* try x_asxprintf_exec() */
    ret = x_asxprintf_exec(&tmp, comp, "x_asxprintf_exec()", &in_addr, &in6_addr, 4, 17);
    if(ret < 0) {
        perror("x_asxprintf_exec()");
        exit(1);
    }//end if
    fprintf(stdout, "%s", tmp);
    fflush(stdout);
    free(tmp);

    /* try x_dxprintf_exec() */
    ret = x_dxprintf_exec(STDOUT_FILENO, comp, "x_dxprintf_exec()", &in_addr, &in6_addr, 4, 17);
    if(ret < 0) {
        perror("x_dxprintf_exec()");
        exit(1);
    }//end if

    /* try x_fxprintf_std_exec() */
    ret = x_fxprintf_std_exec(stdout, comp, "x_fxprintf_std_exec()", &in_addr, &in6_addr, 4, 17);
    if(ret < 0) {
        perror("x_fxprintf_std_exec()");
        exit(1);
    }//end if
    fflush(stdout);

    /* try x_sxprintf_exec() */
    memset(buf, 0, sizeof(buf));
    ret = x_sxprintf_exec(buf, sizeof(buf), comp, "x_sxprintf_exec()", &in_addr, &in6_addr, 4, 17);
    if(ret < 0) {
        perror("x_sxprintf_exec()");
        exit(1);
    }//end if
    fprintf(stdout, "%s", buf);
    fflush(stdout);

    /* try x_xprintf_exec() */
    ret = x_xprintf_exec(comp, "x_xprintf_exec()", &in_addr, &in6_addr, 4, 17);
    if(ret < 0) {
        perror("x_xprintf_exec()");
        exit(1);
    }//end if
    fflush(stdout);

    /* copy domain */
    copyed_domain = x_copy_printf_domain(domain);
    if(!copyed_domain) {
        perror("x_copy_printf_domain()");
        exit(1);
    }//end if
    x_free_printf_domain(copyed_domain);

    /* free */
    x_free_printf_domain(domain);
}//end pass_arguments_by_variable_arguments

static void pass_arguments_by_arguments() {
    int                 ret;
    char                *tmp, buf[128];
    void                *args[5];
    in_addr_t           in_addr;
    struct in6_addr     in6_addr;
    x_printf_comp_t     comp;
    x_printf_domain_t   domain, copyed_domain;

    /* copy a domain from default(x_xprintf_domain_default) */
    domain = x_new_printf_domain();
    if(!domain) {
        perror("x_new_printf_domain()");
        exit(1);
    }//end if

    /* register printer */
    ret = x_register_printf_domain_function(domain, 'A', xprintf_function_ipv4_address, xprintf_arginfo_function_ipv4_address, NULL);
    if(ret != 0) {
        perror("x_register_printf_domain_function()");
        exit(1);
    }//end if

    ret = x_register_printf_domain_function(domain, 'B', xprintf_function_ipv6_address, xprintf_arginfo_function_ipv6_address, domain);
    if(ret != 0) {
        perror("x_register_printf_domain_function()");
        exit(1);
    }//end if

    ret = x_register_printf_domain_function(domain, 'C', xprintf_function_add, xprintf_arginfo_function_add, domain);
    if(ret != 0) {
        perror("x_register_printf_domain_function()");
        exit(1);
    }//end if

    /* compile the format */
    comp = x_new_printf_comp(domain, "%s: fmt \'%%A %%B %%C\' got \'%A %B %C\'\n");
    if(!comp) {
        perror("x_new_printf_comp()");
        exit(1);
    }//end if

    /* dry run compile the format, %! should not be taken */
    if(x_new_printf_comp_dry_run(domain, "%A %B %C %!") != 0) {
        perror("x_new_printf_comp_dry_run()");
        fflush(stderr);
    }//end if

    /* get the binary IP address */
    if(inet_pton(PF_INET, X_IPV4_STR, &in_addr) != 1) {
        perror("inet_pton()");
        exit(1);
    }//end if

    if(inet_pton(PF_INET6, X_IPV6_STR, &in6_addr) != 1) {
        perror("inet_pton()");
        exit(1);
    }//end if

    /* try x_lasxprintf_exec() */
    args[0] = "x_lasxprintf_exec()";
    args[1] = &in_addr;
    args[2] = &in6_addr;
    args[3] = (void *)(uintptr_t)4;
    args[4] = (void *)(uintptr_t)17;
    ret = x_lasxprintf_exec(&tmp, comp, args);
    if(ret < 0) {
        perror("x_lasxprintf_exec()");
        exit(1);
    }//end if
    fprintf(stdout, "%s", tmp);
    fflush(stdout);
    free(tmp);

    /* try x_ldxprintf_exec() */
    args[0] = "x_ldxprintf_exec()";
    args[1] = &in_addr;
    args[2] = &in6_addr;
    args[3] = (void *)(uintptr_t)4;
    args[4] = (void *)(uintptr_t)17;
    ret = x_ldxprintf_exec(STDOUT_FILENO, comp, args);
    if(ret < 0) {
        perror("x_ldxprintf_exec()");
        exit(1);
    }//end if

    /* try x_lfxprintf_std_exec() */
    args[0] = "x_lfxprintf_std_exec()";
    args[1] = &in_addr;
    args[2] = &in6_addr;
    args[3] = (void *)(uintptr_t)4;
    args[4] = (void *)(uintptr_t)17;
    ret = x_lfxprintf_std_exec(stdout, comp, args);
    if(ret < 0) {
        perror("x_lfxprintf_std_exec()");
        exit(1);
    }//end if
    fflush(stdout);

    /* try x_lsxprintf_exec() */
    args[0] = "x_lsxprintf_exec()";
    args[1] = &in_addr;
    args[2] = &in6_addr;
    args[3] = (void *)(uintptr_t)4;
    args[4] = (void *)(uintptr_t)17;
    memset(buf, 0, sizeof(buf));
    ret = x_lsxprintf_exec(buf, sizeof(buf), comp, args);
    if(ret < 0) {
        perror("x_lsxprintf_exec()");
        exit(1);
    }//end if
    fprintf(stdout, "%s", buf);
    fflush(stdout);

    /* try x_lxprintf_exec() */
    args[0] = "x_lxprintf_exec()";
    args[1] = &in_addr;
    args[2] = &in6_addr;
    args[3] = (void *)(uintptr_t)4;
    args[4] = (void *)(uintptr_t)17;
    ret = x_lxprintf_exec(comp, args);
    if(ret < 0) {
        perror("x_lxprintf_exec()");
        exit(1);
    }//end if
    fflush(stdout);

    /* copy domain */
    copyed_domain = x_copy_printf_domain(domain);
    if(!copyed_domain) {
        perror("x_copy_printf_domain()");
        exit(1);
    }//end if
    x_free_printf_domain(copyed_domain);

    /* free */
    x_free_printf_domain(domain);
}//end pass_arguments_by_arguments

int main(int argc, char *argv[]) {
    pass_arguments_by_variable_arguments();
    pass_arguments_by_arguments();
    return 0;
}//end main
