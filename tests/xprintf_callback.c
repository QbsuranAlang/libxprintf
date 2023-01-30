/*
 * APPLE LICENSE
 */


#include "xprintf_callback.h"
#include <arpa/inet.h>

int xprintf_function_ipv4_address(x_FILE *stream, const struct x_printf_info *info, const void *const *args) {
    char        buf[sizeof("255.255.255.255") - 1 + 1];
    in_addr_t   *addr;
    const char  *ret;

    addr = *((in_addr_t **)(args[0]));

    ret = inet_ntop(PF_INET, addr, buf, sizeof(buf));
    if(ret != buf) {
        return x_fxprintf(stream, x_xprintf_domain_default, "<invalid IPv4 address>");
    }//end if
    else {
        return x_fxprintf(stream, x_xprintf_domain_default, "%s", buf);
    }//end else
}//end xprintf_function_ipv4_address

int xprintf_arginfo_function_ipv4_address(const struct x_printf_info *info, size_t n, int *argtypes) {
    argtypes[0] = X_PA_POINTER;
    return 1;
}//end xprintf_arginfo_function_ipv4_address

int xprintf_function_ipv6_address(x_FILE *stream, const struct x_printf_info *info, const void *const *args) {
    char                buf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255") - 1 + 1];
    const char          *ret;
    struct in6_addr     *addr;
    x_printf_domain_t   domain;

    domain = info->context;

    addr = *((struct in6_addr **)(args[0]));

    ret = inet_ntop(PF_INET6, addr, buf, sizeof(buf));
    if(ret != buf) {
        return x_fxprintf(stream, domain, "<invalid IPv6 address>");
    }//end if
    else {
        return x_fxprintf(stream, domain, "%s", buf);
    }//end else
}//end xprintf_function_ipv6_address

int xprintf_arginfo_function_ipv6_address(const struct x_printf_info *info, size_t n, int *argtypes) {
    argtypes[0] = X_PA_POINTER;
    return 1;
}//end xprintf_arginfo_function_ipv6_address

int xprintf_function_add(x_FILE *stream, const struct x_printf_info *info, const void *const *args) {
    int                 a, b;
    x_printf_domain_t   domain;

    domain = info->context;

    a = *((int *)args[0]);
    b = *((int *)args[1]);

    return x_fxprintf(stream, domain, "%d", a + b);
}//end xprintf_function_add

int xprintf_arginfo_function_add(const struct x_printf_info *info, size_t n, int *argtypes) {
    argtypes[0] = X_PA_INT;
    argtypes[1] = X_PA_INT;
    return 2;
}//end xprintf_arginfo_function_add
