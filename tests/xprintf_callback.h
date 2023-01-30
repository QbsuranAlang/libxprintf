/*
 * APPLE LICENSE
 */


#ifndef XPRINTF_CALLBACK_H
#define XPRINTF_CALLBACK_H

#include <libxprintf/libxprintf.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>

int xprintf_function_ipv4_address(x_FILE *stream, const struct x_printf_info *info, const void *const *args);
int xprintf_arginfo_function_ipv4_address(const struct x_printf_info *info, size_t n, int *argtypes);

int xprintf_function_ipv6_address(x_FILE *stream, const struct x_printf_info *info, const void *const *args);
int xprintf_arginfo_function_ipv6_address(const struct x_printf_info *info, size_t n, int *argtypes);

int xprintf_function_add(x_FILE *stream, const struct x_printf_info *info, const void *const *args);
int xprintf_arginfo_function_add(const struct x_printf_info *info, size_t n, int *argtypes);

#endif /* !XPRINTF_CALLBACK_H */
