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

    /* get the binary IP address */
    if(inet_pton(PF_INET, X_IPV4_STR, &in_addr) != 1) {
        perror("inet_pton()");
        exit(1);
    }//end if

    if(inet_pton(PF_INET6, X_IPV6_STR, &in6_addr) != 1) {
        perror("inet_pton()");
        exit(1);
    }//end if

    /* try x_asxprintf() */
    ret = x_asxprintf(&tmp, domain, "x_asxprintf(): fmt \'%%A\' got \'%s\'\n", &in_addr);
    if(ret < 0) {
        perror("x_asxprintf()");
        exit(1);
    }//end if
    fflush(stdout);
    free(tmp);

    /* try x_dxprintf() */
    ret = x_dxprintf(STDOUT_FILENO, domain, "x_dxprintf(): fmt \'%%B\' got \'%B\'\n", &in6_addr);
    if(ret < 0) {
        perror("x_dxprintf()");
        exit(1);
    }//end if

    /* try x_fxprintf_std() */
    ret = x_fxprintf_std(stdout, domain, "x_fxprintf_std(): fmt \'%%A %%B\' got \'%A %B\'\n", &in_addr, &in6_addr);
    if(ret < 0) {
        perror("x_fxprintf_std()");
        exit(1);
    }//end if
    fflush(stdout);

    /* try x_sxprintf() */
    memset(buf, 0, sizeof(buf));
    ret = x_sxprintf(buf, sizeof(buf), domain, "x_sxprintf(): fmt \'%%C\' got \'%C\'", 4, 17);
    if(ret < 0) {
        perror("x_sxprintf()");
        exit(1);
    }//end if
    fprintf(stdout, "%s\n", buf);
    fflush(stdout);

    /* try x_xprintf() */
    ret = x_xprintf(domain, "x_xprintf(): fmt \'%%C\' got \'%C\'\n", 4, 17);
    if(ret < 0) {
        perror("x_xprintf()");
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

static void pass_arguments_by_arguments(void) {
    int                 ret;
    char                *tmp, buf[128];
    void                *args[2];
    in_addr_t           in_addr;
    struct in6_addr     in6_addr;
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

    /* get the binary IP address */
    if(inet_pton(PF_INET, X_IPV4_STR, &in_addr) != 1) {
        perror("inet_pton()");
        exit(1);
    }//end if

    if(inet_pton(PF_INET6, X_IPV6_STR, &in6_addr) != 1) {
        perror("inet_pton()");
        exit(1);
    }//end if

    /* try x_lasxprintf() */
    args[0] = &in_addr;
    ret = x_lasxprintf(&tmp, domain, "x_lasxprintf(): fmt \'%%A\' got \'%A\'", args);
    if(ret < 0) {
        perror("x_lasxprintf()");
        exit(1);
    }//end if
    printf("%s\n", tmp);
    fflush(stdout);
    free(tmp);

    /* try x_ldxprintf() */
    ret = x_ldxprintf(STDOUT_FILENO, domain, "x_ldxprintf(): fmt \'%%B\' got \'%B\'\n", args);
    if(ret < 0) {
        perror("x_ldxprintf()");
        exit(1);
    }//end if

    /* try x_lfxprintf_std() */
    args[0] = &in_addr;
    args[1] = &in6_addr;
    ret = x_lfxprintf_std(stdout, domain, "x_lfxprintf_std(): fmt \'%%A %%B\' got \'%A %B\'\n", args);
    if(ret < 0) {
        perror("x_lfxprintf_std()");
        exit(1);
    }//end if
    fflush(stdout);

    /* try x_lsxprintf() */
    args[0] = (void *)(uintptr_t)4;
    args[1] = (void *)(uintptr_t)17;
    memset(buf, 0, sizeof(buf));
    ret = x_lsxprintf(buf, sizeof(buf), domain, "x_lsxprintf(): fmt \'%%C\' got \'%C\'", args);
    if(ret < 0) {
        perror("x_lsxprintf()");
        exit(1);
    }//end if
    fprintf(stdout, "%s\n", buf);
    fflush(stdout);

    /* try x_lxprintf() */
    args[0] = (void *)(uintptr_t)4;
    args[1] = (void *)(uintptr_t)17;
    ret = x_lxprintf(domain, "x_lxprintf(): fmt \'%%C\' got \'%C\'\n", args);
    if(ret < 0) {
        perror("x_lxprintf()");
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
