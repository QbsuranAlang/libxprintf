/*
 * APPLE LICENSE
 */


#include <libxprintf/libxprintf.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#if !defined(DEBUG)
#define DEBUG
#endif /* !!defined(DEBUG) */
#include <assert.h>

#include "xprintf_callback.h"

#define X_IPV4_STR "169.254.2.1"
#define X_IPV6_STR "2002:b013:35a:32b3:cf9:819b:b35:5de7"

static void test_domain_ap(void) {
    int                 p[2], nread;
    char                *tmp, buf1[1024], buf2[1024];
    FILE                *read_fp, *write_fp;
    time_t              t;
    in_addr_t           in_addr;
    struct timeval      tv;
    struct timespec     ts;
    struct in6_addr     in6_addr;
    x_printf_domain_t   domain, copyed_domain;

    /* new domain */
    assert((domain = x_new_printf_domain()) != NULL);

    /* register printer */
    assert(x_register_printf_domain_function(domain, 'A', xprintf_function_ipv4_address, xprintf_arginfo_function_ipv4_address, NULL) == 0);
    assert(x_register_printf_domain_function(domain, 'B', xprintf_function_ipv6_address, xprintf_arginfo_function_ipv6_address, domain) == 0);
    assert(x_register_printf_domain_function(domain, 'C', xprintf_function_add, xprintf_arginfo_function_add, domain) == 0);

    assert(inet_pton(PF_INET, X_IPV4_STR, &in_addr) == 1);
    assert(inet_pton(PF_INET6, X_IPV6_STR, &in6_addr) == 1);

    /* test x_asxprintf() */
    assert(x_asxprintf(&tmp, domain, "x_asxprintf(): %A", &in_addr) == strlen("x_asxprintf(): " X_IPV4_STR));
    assert(strcmp(tmp, "x_asxprintf(): " X_IPV4_STR) == 0);
    free(tmp);

    /* test x_dxprintf() */
    assert(pipe(p) == 0);
    assert(x_dxprintf(p[1], domain, "x_dxprintf(): %B", &in6_addr) == strlen("x_dxprintf(): " X_IPV6_STR));
    close(p[1]);
    memset(buf1, 0, sizeof(buf1));
    nread = read(p[0], buf1, sizeof(buf1));
    close(p[0]);
    assert(nread == strlen("x_dxprintf(): " X_IPV6_STR));
    assert(strcmp(buf1, "x_dxprintf(): " X_IPV6_STR) == 0);

    /* test x_fxprintf_std() */
    assert(pipe(p) == 0);
    assert((read_fp = fdopen(p[0], "r")) != NULL);
    assert((write_fp = fdopen(p[1], "w")) != NULL);
    assert(x_fxprintf_std(write_fp, domain, "x_fxprintf_std(): %A %B", &in_addr, &in6_addr) == strlen("x_fxprintf_std(): " X_IPV4_STR " " X_IPV6_STR));
    fclose(write_fp);
    memset(buf1, 0, sizeof(buf1));
    nread = fread(buf1, sizeof(char), sizeof(buf1)/sizeof(buf1[0]), read_fp);
    fclose(read_fp);
    assert(nread == strlen("x_fxprintf_std(): " X_IPV4_STR " " X_IPV6_STR));
    assert(strcmp(buf1, "x_fxprintf_std(): " X_IPV4_STR " " X_IPV6_STR) == 0);

    /* test x_sxprintf() */
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "x_sxprintf(): %C", 4, 17) == strlen("x_sxprintf(): 21"));
    assert(strcmp(buf1, "x_sxprintf(): 21") == 0);

    /* test x_xprintf() */
    assert(x_xprintf(domain, "x_xprintf(): is good to go") == strlen("x_xprintf(): is good to go"));

    /* test invalid specifier */
    assert(x_register_printf_domain_function(domain, '#', xprintf_function_add, xprintf_arginfo_function_add, NULL) != 0);

    /* test render extended, hexdump %H */
    assert(x_register_printf_domain_render_std(domain, "H") == 0);
    tmp = "\x30\x31\x32\x33\x41\x42\x43\x44"; /* 0123ABCE */
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%+#H", tmp, 8) == strlen("0000 30 31 32 33 41 42 43 44                          |0123ABCD        |"));
    assert(strcmp(buf1, "0000 30 31 32 33 41 42 43 44                          |0123ABCD        |") == 0);

    /* test render extended, errno %M */
    assert(x_register_printf_domain_render_std(domain, "M") == 0);
    strerror_r(EEXIST, buf2, sizeof(buf2));
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%M", EEXIST) == strlen(buf2));
    assert(strcmp(buf1, buf2) == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%M", 0x0806) == strlen("errno=2054/0x0806"));
    assert(strcmp(buf1, "errno=2054/0x0806") == 0);

    /* test render extended, quote %Q */
    assert(x_register_printf_domain_render_std(domain, "Q") == 0);
    tmp = "This is a string\nThis is second line";
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%Q", tmp) == strlen("\"This is a string\\nThis is second line\""));
    assert(strcmp(buf1, "\"This is a string\\nThis is second line\"") == 0);

    /* test render extended, time_t %T */
    assert(x_register_printf_domain_render_std(domain, "T") == 0);
    t = 1547801608;
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%T", &t) == strlen("1547801608"));
    assert(strcmp(buf1, "1547801608") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%#T", &t) == strlen("49y29d8h53m28s"));
    assert(strcmp(buf1, "49y29d8h53m28s") == 0);

    /* test timeval %T */
    tv.tv_sec = 10000;
    tv.tv_usec = 123456;
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%lT", &tv) == strlen("10000.123456"));
    assert(strcmp(buf1, "10000.123456") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%#lT", &tv) == strlen("2h46m40s.123456"));
    assert(strcmp(buf1, "2h46m40s.123456") == 0);

    /* test timespec %T */
    ts.tv_sec = 123456;
    ts.tv_nsec = 99999;
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%llT", &ts) == strlen("123456.000099999"));
    assert(strcmp(buf1, "123456.000099999") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%#llT", &ts) == strlen("1d10h17m36s.000099999"));
    assert(strcmp(buf1, "1d10h17m36s.000099999") == 0);

    /* test render extended, string vis %V */
    assert(x_register_printf_domain_render_std(domain, "V") == 0);
    tmp = "this is test string";
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%V", tmp) == strlen("this\\sis\\stest\\sstring"));
    assert(strcmp(buf1, "this\\sis\\stest\\sstring") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%+V", tmp) == strlen("this%20is%20test%20string"));
    assert(strcmp(buf1, "this%20is%20test%20string") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%0V", tmp) == strlen("this\\040is\\040test\\040string"));
    assert(strcmp(buf1, "this\\040is\\040test\\040string") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_sxprintf(buf1, sizeof(buf1), domain, "%#V", tmp) == strlen("this\\040is\\040test\\040string"));
    assert(strcmp(buf1, "this\\040is\\040test\\040string") == 0);

    /* test copy domain */
    assert((copyed_domain = x_copy_printf_domain(domain)) != NULL);
    x_free_printf_domain(copyed_domain);

    /* free */
    x_free_printf_domain(domain);
}//end test_domain_ap

static void test_domain_arg(void) {
    int                 p[2], nread;
    char                *tmp, buf1[1024], buf2[1024];
    FILE                *read_fp, *write_fp;
    void                *args[2];
    time_t              t;
    in_addr_t           in_addr;
    struct timeval      tv;
    struct timespec     ts;
    struct in6_addr     in6_addr;
    x_printf_domain_t   domain, copyed_domain;

    /* new domain */
    assert((domain = x_new_printf_domain()) != NULL);

    /* register printer */
    assert(x_register_printf_domain_function(domain, 'A', xprintf_function_ipv4_address, xprintf_arginfo_function_ipv4_address, NULL) == 0);
    assert(x_register_printf_domain_function(domain, 'B', xprintf_function_ipv6_address, xprintf_arginfo_function_ipv6_address, domain) == 0);
    assert(x_register_printf_domain_function(domain, 'C', xprintf_function_add, xprintf_arginfo_function_add, domain) == 0);

    assert(inet_pton(PF_INET, X_IPV4_STR, &in_addr) == 1);
    assert(inet_pton(PF_INET6, X_IPV6_STR, &in6_addr) == 1);

    /* test x_lasxprintf() */
    args[0] = &in_addr;
    assert(x_lasxprintf(&tmp, domain, "x_lasxprintf(): %A", args) == strlen("x_lasxprintf(): " X_IPV4_STR));
    assert(strcmp(tmp, "x_lasxprintf(): " X_IPV4_STR) == 0);
    free(tmp);

    /* test x_ldxprintf() */
    assert(pipe(p) == 0);
    args[0] = &in6_addr;
    assert(x_ldxprintf(p[1], domain, "x_ldxprintf(): %B", args) == strlen("x_ldxprintf(): " X_IPV6_STR));
    close(p[1]);
    memset(buf1, 0, sizeof(buf1));
    nread = read(p[0], buf1, sizeof(buf1));
    close(p[0]);
    assert(nread == strlen("x_ldxprintf(): " X_IPV6_STR));
    assert(strcmp(buf1, "x_ldxprintf(): " X_IPV6_STR) == 0);

    /* test x_lfxprintf_std() */
    assert(pipe(p) == 0);
    assert((read_fp = fdopen(p[0], "r")) != NULL);
    assert((write_fp = fdopen(p[1], "w")) != NULL);
    args[0] = &in_addr;
    args[1] = &in6_addr;
    assert(x_lfxprintf_std(write_fp, domain, "x_lfxprintf_std(): %A %B", args) == strlen("x_lfxprintf_std(): " X_IPV4_STR " " X_IPV6_STR));
    fclose(write_fp);
    memset(buf1, 0, sizeof(buf1));
    nread = fread(buf1, sizeof(char), sizeof(buf1)/sizeof(buf1[0]), read_fp);
    fclose(read_fp);
    assert(nread == strlen("x_lfxprintf_std(): " X_IPV4_STR " " X_IPV6_STR));
    assert(strcmp(buf1, "x_lfxprintf_std(): " X_IPV4_STR " " X_IPV6_STR) == 0);

    /* test x_lsxprintf() */
    args[0] = (void *)(uintptr_t)4;
    args[1] = (void *)(uintptr_t)17;
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "x_lsxprintf(): %C", args) == strlen("x_lsxprintf(): 21"));
    assert(strcmp(buf1, "x_lsxprintf(): 21") == 0);

    /* test x_lxprintf() */
    assert(x_lxprintf(domain, "x_lxprintf(): is good to go", NULL) == strlen("x_lxprintf(): is good to go"));

    /* test render extended, hexdump %H */
    assert(x_register_printf_domain_render_std(domain, "H") == 0);
    tmp = "\x30\x31\x32\x33\x41\x42\x43\x44"; /* 0123ABCE */
    args[0] = tmp;
    args[1] = (void *)(uintptr_t)8;
    memset(buf1, 0, sizeof(buf1));
    x_lsxprintf(buf1, sizeof(buf1), domain, "%+#H", args);
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%+#H", args) == strlen("0000 30 31 32 33 41 42 43 44                          |0123ABCD        |"));
    assert(strcmp(buf1, "0000 30 31 32 33 41 42 43 44                          |0123ABCD        |") == 0);

    /* test render extended, errno %M */
    assert(x_register_printf_domain_render_std(domain, "M") == 0);
    memset(buf2, 0, sizeof(buf2));
    strerror_r(EEXIST, buf2, sizeof(buf2));
    args[0] = (void *)(uintptr_t)EEXIST;
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%M", args) == strlen(buf2));
    assert(strcmp(buf1, buf2) == 0);
    args[0] = (void *)(uintptr_t)0x0806;
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%M", args) == strlen("errno=2054/0x0806"));
    assert(strcmp(buf1, "errno=2054/0x0806") == 0);

    /* test render extended, quote %Q */
    assert(x_register_printf_domain_render_std(domain, "Q") == 0);
    tmp = "This is a string\nThis is second line";
    args[0] = tmp;
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%Q", args) == strlen("\"This is a string\\nThis is second line\""));
    assert(strcmp(buf1, "\"This is a string\\nThis is second line\"") == 0);

    /* test render extended, time_t %T */
    assert(x_register_printf_domain_render_std(domain, "T") == 0);
    t = 1547801608;
    args[0] = &t;
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%T", args) == strlen("1547801608"));
    assert(strcmp(buf1, "1547801608") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%#T", args) == strlen("49y29d8h53m28s"));
    assert(strcmp(buf1, "49y29d8h53m28s") == 0);

    /* test timeval %T */
    tv.tv_sec = 10000;
    tv.tv_usec = 123456;
    args[0] = &tv;
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%lT", args) == strlen("10000.123456"));
    assert(strcmp(buf1, "10000.123456") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%#lT", args) == strlen("2h46m40s.123456"));
    assert(strcmp(buf1, "2h46m40s.123456") == 0);

    /* test timespec %T */
    ts.tv_sec = 123456;
    ts.tv_nsec = 99999;
    args[0] = &ts;
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%llT", args) == strlen("123456.000099999"));
    assert(strcmp(buf1, "123456.000099999") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%#llT", args) == strlen("1d10h17m36s.000099999"));
    assert(strcmp(buf1, "1d10h17m36s.000099999") == 0);

    /* test render extended, string vis %V */
    assert(x_register_printf_domain_render_std(domain, "V") == 0);
    tmp = "this is test string";
    args[0] = tmp;
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%V", args) == strlen("this\\sis\\stest\\sstring"));
    assert(strcmp(buf1, "this\\sis\\stest\\sstring") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%+V", args) == strlen("this%20is%20test%20string"));
    assert(strcmp(buf1, "this%20is%20test%20string") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%0V", args) == strlen("this\\040is\\040test\\040string"));
    assert(strcmp(buf1, "this\\040is\\040test\\040string") == 0);
    memset(buf1, 0, sizeof(buf1));
    assert(x_lsxprintf(buf1, sizeof(buf1), domain, "%#V", args) == strlen("this\\040is\\040test\\040string"));
    assert(strcmp(buf1, "this\\040is\\040test\\040string") == 0);

    /* test copy domain */
    assert((copyed_domain = x_copy_printf_domain(domain)) != NULL);
    x_free_printf_domain(copyed_domain);

    /* free */
    x_free_printf_domain(domain);
}//end test_domain_arg

static void test_compiled_ap(void) {
    int                 p[2], nread;
    char                *tmp, buf[1024];
    FILE                *read_fp, *write_fp;
    in_addr_t           in_addr;
    struct in6_addr     in6_addr;
    x_printf_comp_t     comp;
    x_printf_domain_t   domain, copyed_domain;

    /* new domain */
    assert((domain = x_new_printf_domain()) != NULL);

    /* register printer */
    assert(x_register_printf_domain_function(domain, 'A', xprintf_function_ipv4_address, xprintf_arginfo_function_ipv4_address, NULL) == 0);
    assert(x_register_printf_domain_function(domain, 'B', xprintf_function_ipv6_address, xprintf_arginfo_function_ipv6_address, domain) == 0);
    assert(x_register_printf_domain_function(domain, 'C', xprintf_function_add, xprintf_arginfo_function_add, domain) == 0);

    assert(inet_pton(PF_INET, X_IPV4_STR, &in_addr) == 1);
    assert(inet_pton(PF_INET6, X_IPV6_STR, &in6_addr) == 1);

    /* compile format */
    assert((comp = x_new_printf_comp(domain, "%A %B %C")) != NULL);

    /* %! should not be taken */
    assert(x_new_printf_comp_dry_run(domain, "%A %B %C %!") != 0);

    /* test x_asxprintf_exec() */
    assert(x_asxprintf_exec(&tmp, comp, &in_addr, &in6_addr, 4, 17) == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    assert(strcmp(tmp, X_IPV4_STR " " X_IPV6_STR " 21") == 0);

    /* test x_dxprintf_exec() */
    assert(pipe(p) == 0);
    assert(x_dxprintf_exec(p[1], comp, &in_addr, &in6_addr, 4, 17) == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    close(p[1]);
    memset(buf, 0, sizeof(buf));
    nread = read(p[0], buf, sizeof(buf));
    close(p[0]);
    assert(nread == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    assert(strcmp(buf, X_IPV4_STR " " X_IPV6_STR " 21") == 0);

    /* test x_fxprintf_std_exec() */
    assert(pipe(p) == 0);
    assert((read_fp = fdopen(p[0], "r")) != NULL);
    assert((write_fp = fdopen(p[1], "w")) != NULL);
    assert(x_fxprintf_std_exec(write_fp, comp, &in_addr, &in6_addr, 4, 17) == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    fclose(write_fp);
    memset(buf, 0, sizeof(buf));
    nread = fread(buf, sizeof(char), sizeof(buf)/sizeof(buf[0]), read_fp);
    fclose(read_fp);
    assert(nread == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    assert(strcmp(buf, X_IPV4_STR " " X_IPV6_STR " 21") == 0);

    /* test x_sxprintf_exec() */
    memset(buf, 0, sizeof(buf));
    assert(x_sxprintf_exec(buf, sizeof(buf), comp, &in_addr, &in6_addr, 4, 17) == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    assert(strcmp(buf, X_IPV4_STR " " X_IPV6_STR " 21") == 0);

    /* test x_xprintf_exec() */
    assert(x_xprintf_exec(comp, &in_addr, &in6_addr, 4, 17) == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));

    /* test copy domain */
    assert((copyed_domain = x_copy_printf_domain(domain)) != NULL);
    x_free_printf_domain(copyed_domain);

    /* free */
    x_free_printf_comp(comp);
    x_free_printf_domain(domain);
}//end test_compiled_ap

static void test_compiled_arg(void) {
    int                 p[2], nread;
    char                *tmp, buf[1024];
    FILE                *read_fp, *write_fp;
    void                *args[4];
    in_addr_t           in_addr;
    struct in6_addr     in6_addr;
    x_printf_comp_t     comp;
    x_printf_domain_t   domain;

    /* new domain */
    assert((domain = x_new_printf_domain()) != NULL);

    /* register printer */
    assert(x_register_printf_domain_function(domain, 'A', xprintf_function_ipv4_address, xprintf_arginfo_function_ipv4_address, NULL) == 0);
    assert(x_register_printf_domain_function(domain, 'B', xprintf_function_ipv6_address, xprintf_arginfo_function_ipv6_address, domain) == 0);
    assert(x_register_printf_domain_function(domain, 'C', xprintf_function_add, xprintf_arginfo_function_add, domain) == 0);

    assert(inet_pton(PF_INET, X_IPV4_STR, &in_addr) == 1);
    assert(inet_pton(PF_INET6, X_IPV6_STR, &in6_addr) == 1);

    /* compile format */
    assert((comp = x_new_printf_comp(domain, "%A %B %C")) != NULL);

    /* %! should not be taken */
    assert(x_new_printf_comp_dry_run(domain, "%A %B %C %!") != 0);

    /* test x_lasxprintf_exec() */
    args[0] = &in_addr;
    args[1] = &in6_addr;
    args[2] = (void *)(uintptr_t)4;
    args[3] = (void *)(uintptr_t)17;
    assert(x_lasxprintf_exec(&tmp, comp, args) == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    assert(strcmp(tmp, X_IPV4_STR " " X_IPV6_STR " 21") == 0);

    /* test x_ldxprintf_exec() */
    assert(pipe(p) == 0);
    args[0] = &in_addr;
    args[1] = &in6_addr;
    args[2] = (void *)(uintptr_t)4;
    args[3] = (void *)(uintptr_t)17;
    assert(x_ldxprintf_exec(p[1], comp, args) == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    close(p[1]);
    memset(buf, 0, sizeof(buf));
    nread = read(p[0], buf, sizeof(buf));
    close(p[0]);
    assert(nread == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    assert(strcmp(buf, X_IPV4_STR " " X_IPV6_STR " 21") == 0);

    /* test x_lfxprintf_std_exec() */
    assert(pipe(p) == 0);
    assert((read_fp = fdopen(p[0], "r")) != NULL);
    assert((write_fp = fdopen(p[1], "w")) != NULL);
    args[0] = &in_addr;
    args[1] = &in6_addr;
    args[2] = (void *)(uintptr_t)4;
    args[3] = (void *)(uintptr_t)17;
    assert(x_lfxprintf_std_exec(write_fp, comp, args) == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    fclose(write_fp);
    memset(buf, 0, sizeof(buf));
    nread = fread(buf, sizeof(char), sizeof(buf)/sizeof(buf[0]), read_fp);
    fclose(read_fp);
    assert(nread == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    assert(strcmp(buf, X_IPV4_STR " " X_IPV6_STR " 21") == 0);

    /* test x_lsxprintf_exec() */
    args[0] = &in_addr;
    args[1] = &in6_addr;
    args[2] = (void *)(uintptr_t)4;
    args[3] = (void *)(uintptr_t)17;
    memset(buf, 0, sizeof(buf));
    assert(x_lsxprintf_exec(buf, sizeof(buf), comp, args) == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));
    assert(strcmp(buf, X_IPV4_STR " " X_IPV6_STR " 21") == 0);

    /* test x_lxprintf_exec() */
    args[0] = &in_addr;
    args[1] = &in6_addr;
    args[2] = (void *)(uintptr_t)4;
    args[3] = (void *)(uintptr_t)17;
    assert(x_lxprintf_exec(comp, args) == strlen(X_IPV4_STR " " X_IPV6_STR " 21"));

    /* free */
    x_free_printf_comp(comp);
    x_free_printf_domain(domain);
}//end test_compiled_arg

static void fmtchk(const char *fmt, const char *expected) {
    int     len;
    char    buf[1024];

    memset(buf, 0, sizeof(buf));
    len = x_sxprintf(buf, sizeof(buf), x_xprintf_domain_default, fmt, 0x12);
    printf("%s(): expected: `%s'(%zd), got: `%s'(%d)\n", __func__, expected, strlen(expected), buf, len);

    assert(len == strlen(expected));
    assert(strcmp(buf, expected) == 0);
}//end fmtchk

static void fmtst1chk(const char *fmt, const char *expected) {
    int     len;
    char    buf[1024];

    memset(buf, 0, sizeof(buf));
    len = x_sxprintf(buf, sizeof(buf), x_xprintf_domain_default, fmt, 4, 0x12);
    printf("%s(): expected: `%s'(%zd), got: `%s'(%d)\n", __func__, expected, strlen(expected), buf, len);

    assert(len == strlen(expected));
    assert(strcmp(buf, expected) == 0);
}//end fmtst1chk

static void fmtst2chk(const char *fmt, const char *expected) {
    int     len;
    char    buf[1024];

    memset(buf, 0, sizeof(buf));
    len = x_sxprintf(buf, sizeof(buf), x_xprintf_domain_default, fmt, 4, 4, 0x12);
    printf("%s(): expected: `%s'(%zd), got: `%s'(%d)\n", __func__, expected, strlen(expected), buf, len);

    assert(len == strlen(expected));
    assert(strcmp(buf, expected) == 0);
}//end fmtst2chk

static void fmtst3chk(const char *expected, const char *fmt, ...) {
    int     len;
    char    buf[65536];
    va_list ap;

    memset(buf, 0, sizeof(buf));

    va_start(ap, fmt);
    len = x_vsxprintf(buf, sizeof(buf), x_xprintf_domain_default, fmt, ap);
    va_end(ap);

    printf("%s(): expected: `%s'(%zd), got: `%s'(%d)\n", __func__, expected, strlen(expected), buf, len);

    assert(len == strlen(expected));
    assert(strcmp(buf, expected) == 0);
}//end fmtst3chk

static void test_extra(void) {
    int         n;
    char        buf[1024];
    char        shortstr[] = "Hi, Z.";
    char        longstr[] = "Good morning, Doctor Chandra.  This is Hal.  \
        I am ready for my first lesson today.";
    double      d;

    /* test of %n */
    assert(x_sxprintf(buf, sizeof(buf), x_xprintf_domain_default, "%010d%n", 0, &n) == strlen("0000000000"));
    assert(n == strlen("0000000000"));
    assert(strcmp(buf, "0000000000") == 0);

    /* https://github.com/lattera/glibc/blob/master/stdio-common/tst-printf.c */
    fmtchk("%.4x", "0012");
    fmtchk("%04x", "0012");
    fmtchk("%4.4x", "0012");
    fmtchk("%04.4x", "0012");
    fmtchk("%4.3x", " 012");
    fmtchk("%04.3x", " 012");

    fmtst1chk("%.*x", "0012");
    fmtst1chk("%0*x", "0012");
    fmtst2chk("%*.*x", "0012");
    fmtst2chk("%0*.*x", "0012");

    fmtst3chk("bad format: \"%b\"", "bad format: \"%%b\"");
    fmtst3chk("nil pointer (padded): \"         0\"", "nil pointer (padded): \"%10p\"", (void *)NULL);

    fmtst3chk("decimal negative: \"-2345\"", "decimal negative: \"%d\"", -2345);
    fmtst3chk("octal negative: \"37777773327\"", "octal negative: \"%o\"", -2345);
    fmtst3chk("hex negative: \"fffff6d7\"", "hex negative: \"%x\"", -2345);
    fmtst3chk("long decimal number: \"-123456\"", "long decimal number: \"%ld\"", -123456L);
    fmtst3chk("long octal negative: \"1777777777777777773327\"", "long octal negative: \"%lo\"", -2345L);
    fmtst3chk("long unsigned decimal number: \"18446744073709428160\"", "long unsigned decimal number: \"%lu\"", -123456L);
    fmtst3chk("zero-padded LDN: \"-000123456\"", "zero-padded LDN: \"%010ld\"", -123456L);
    fmtst3chk("left-adjusted ZLDN: \"-123456   \"", "left-adjusted ZLDN: \"%-010ld\"", -123456L);
    fmtst3chk("space-padded LDN: \"   -123456\"", "space-padded LDN: \"%10ld\"", -123456L);
    fmtst3chk("left-adjusted SLDN: \"-123456   \"", "left-adjusted SLDN: \"%-10ld\"", -123456L);

    fmtst3chk("zero-padded string: \"0000Hi, Z.\"", "zero-padded string: \"%010s\"", shortstr);
    fmtst3chk("left-adjusted Z string: \"Hi, Z.0000\"", "left-adjusted Z string: \"%-010s\"", shortstr);
    fmtst3chk("space-padded string: \"    Hi, Z.\"", "space-padded string: \"%10s\"", shortstr);
    fmtst3chk("left-adjusted S string: \"Hi, Z.    \"", "left-adjusted S string: \"%-10s\"", shortstr);
    fmtst3chk("null string: \"(null)\"", "null string: \"%s\"", (char *)NULL);
    fmtst3chk("limited string: \"Good morning, Doctor C\"", "limited string: \"%.22s\"", longstr);

    fmtst3chk("a-style max: \"0x1.fffffffffffffp+1023\"", "a-style max: \"%a\"", DBL_MAX);
    fmtst3chk("a-style -max: \"-0x1.fffffffffffffp+1023\"", "a-style -max: \"%a\"", -DBL_MAX);
    fmtst3chk("e-style >= 1: \"1.234000e+01\"", "e-style >= 1: \"%e\"", 12.34);
    fmtst3chk("e-style >= .1: \"1.234000e-01\"", "e-style >= .1: \"%e\"", 0.1234);
    fmtst3chk("e-style < .1: \"1.234000e-03\"", "e-style < .1: \"%e\"", 0.001234);
    fmtst3chk("e-style big: \"1.000000000000000000000000000000000000000000000000000000000000e+20\"", "e-style big: \"%.60e\"", 1e20);
    fmtst3chk("e-style == .1: \"1.000000e-01\"", "e-style == .1: \"%e\"", 0.1);
    fmtst3chk("f-style >= 1: \"12.340000\"", "f-style >= 1: \"%f\"", 12.34);
    fmtst3chk("f-style >= .1: \"0.123400\"", "f-style >= .1: \"%f\"", 0.1234);
    fmtst3chk("f-style < .1: \"0.001234\"", "f-style < .1: \"%f\"", 0.001234);
    fmtst3chk("g-style >= 1: \"12.34\"", "g-style >= 1: \"%g\"", 12.34);
    fmtst3chk("g-style >= .1: \"0.1234\"", "g-style >= .1: \"%g\"", 0.1234);
    fmtst3chk("g-style < .1: \"0.001234\"", "g-style < .1: \"%g\"", 0.001234);
    fmtst3chk("g-style big: \"100000000000000000000\"", "g-style big: \"%.60g\"", 1e20);

    fmtst3chk(" 0.10000", " %6.5f", .099999999860301614);
    fmtst3chk(" 0.10000", " %6.5f", .1);
    fmtst3chk("x0.5000x", "x%5.4fx", .5);

    fmtst3chk("0x1", "%#03x", 1);
    fmtst3chk("something really insane: 1.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
        "something really insane: %.10000f", 1.0);

    d = FLT_MIN;
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);
    fmtst3chk("5.87747175411143754e-39", "%.17e", d / 2);

    fmtst3chk("   4.94066e-324", "%15.5e", 4.9406564584124654e-324);

    fmtst3chk("|      0.0000|  0.0000e+00|           0|", "|%12.4f|%12.4e|%12.4g|", 0.0, 0.0, 0.0);
    fmtst3chk("|      1.0000|  1.0000e+00|           1|", "|%12.4f|%12.4e|%12.4g|", 1.0, 1.0, 1.0);
    fmtst3chk("|     -1.0000| -1.0000e+00|          -1|", "|%12.4f|%12.4e|%12.4g|", -1.0, -1.0, -1.0);
    fmtst3chk("|    100.0000|  1.0000e+02|         100|", "|%12.4f|%12.4e|%12.4g|", 100.0, 100.0, 100.0);
    fmtst3chk("|   1000.0000|  1.0000e+03|        1000|", "|%12.4f|%12.4e|%12.4g|", 1000.0, 1000.0, 1000.0);
    fmtst3chk("|  10000.0000|  1.0000e+04|       1e+04|", "|%12.4f|%12.4e|%12.4g|", 10000.0, 10000.0, 10000.0);
    fmtst3chk("|  12345.0000|  1.2345e+04|   1.234e+04|", "|%12.4f|%12.4e|%12.4g|", 12345.0, 12345.0, 12345.0);
    fmtst3chk("| 100000.0000|  1.0000e+05|       1e+05|", "|%12.4f|%12.4e|%12.4g|", 100000.0, 100000.0, 100000.0);
    fmtst3chk("| 123456.0000|  1.2346e+05|   1.235e+05|", "|%12.4f|%12.4e|%12.4g|", 123456.0, 123456.0, 123456.0);

    fmtst3chk("1.234568e+06", "%e", 1234567.8);
    fmtst3chk("1234567.800000", "%f", 1234567.8);
    fmtst3chk("1.23457e+06", "%g", 1234567.8);
    fmtst3chk("123.456", "%g", 123.456);
    fmtst3chk("1e+06", "%g", 1000000.0);
    fmtst3chk("10", "%g", 10.0);
    fmtst3chk("0.02", "%g", 0.02);

    fmtst3chk("onetwo                 three                         ", "%*s%*s%*s", -1, "one", -20, "two", -30, "three");
    fmtst3chk("40000000000", "%07Lo", 040000000000ll);

    fmtst3chk("printf(\"%hhu\", 257) = 1", "printf(\"%%hhu\", %u) = %hhu", UCHAR_MAX + 2, UCHAR_MAX + 2);
    fmtst3chk("printf(\"%hu\", 65537) = 1", "printf(\"%%hu\", %u) = %hu", USHRT_MAX + 2, USHRT_MAX + 2);
    fmtst3chk("printf(\"%hhi\", 257) = 1", "printf(\"%%hhi\", %i) = %hhi", UCHAR_MAX + 2, UCHAR_MAX + 2);
    fmtst3chk("printf(\"%hi\", 65537) = 1", "printf(\"%%hi\", %i) = %hi", USHRT_MAX + 2, USHRT_MAX + 2);

    fmtst3chk("printf(\"%1$hhu\", 257) = 1", "printf(\"%%1$hhu\", %2$u) = %1$hhu", UCHAR_MAX + 2, UCHAR_MAX + 2);
    fmtst3chk("printf(\"%1$hu\", 65537) = 1", "printf(\"%%1$hu\", %2$u) = %1$hu", USHRT_MAX + 2, USHRT_MAX + 2);
    fmtst3chk("printf(\"%1$hhi\", 257) = 1", "printf(\"%%1$hhi\", %2$i) = %1$hhi", UCHAR_MAX + 2, UCHAR_MAX + 2);
    fmtst3chk("printf(\"%1$hi\", 65537) = 1", "printf(\"%%1$hi\", %2$i) = %1$hi", USHRT_MAX + 2, USHRT_MAX + 2);
}//end test_extra

static void rfg1(void) {
    fmtst3chk("     ", "%5.s", "xyz");
    fmtst3chk("   33", "%5.f", 33.3);
    fmtst3chk("   3e+08", "%8.e", 33.3e7);
    fmtst3chk("   3E+08", "%8.E", 33.3e7);
    fmtst3chk("3e+01", "%.g", 33.3);
    fmtst3chk("3E+01", "%.G", 33.3);
}//end rfg1

static void rfg2(void) {
    int prec;

    prec = 0;
    fmtst3chk("3", "%.*g", prec, 3.3);
    prec = 0;
    fmtst3chk("3", "%.*G", prec, 3.3);
    prec = 0;
    fmtst3chk("      3", "%7.*G", prec, 3.33);
    prec = 3;
    fmtst3chk(" 041", "%04.*o", prec, 33);
    prec = 7;
    fmtst3chk("  0000033", "%09.*u", prec, 33);
    prec = 3;
    fmtst3chk(" 021", "%04.*x", prec, 33);
    prec = 3;
    fmtst3chk(" 021", "%04.*X", prec, 33);
}//end rfg2

static void rfg3(void) {
    int             i = 12345;
    int             h = 1234;
    double          g = 5.0000001;
    double          d = 321.7654321;
    const char      s[] = "test-string";
    unsigned long   l = 1234567890;

    fmtst3chk("   12345  1234    11145401322     321.765432   3.217654e+02   5    test-string",
        "%1$*5$d %2$*6$hi %3$*7$lo %4$*8$f %9$*12$e %10$*13$g %11$*14$s",
       i, h, l, d, 8, 5, 14, 14, d, g, s, 14, 3, 14);
}//end rfg3

static void test_dotzero(void) {
    fmtst3chk("0", "%.0f", 0.1);
}//end test_dotzero

static void test_memory_leak(void) {
    int             i;
    char            buf[1000];
    time_t          now;
    uint32_t        ul, uh;
    union {
        double      d;
        uint64_t    bits;
    } u;

    time(&now);
    srand((unsigned int)now);
    for(i = 0; i < 10000; i++) {
        ul = (uint32_t)rand();
        uh = (uint32_t)rand();
        u.bits = (uint64_t)uh << 32 | ul;
        assert(x_sxprintf(buf, sizeof(buf), x_xprintf_domain_default, " %.2f", u.d) > 0);
    }//end for
}//end test_memory_leak

static void test_zeropad(void) {
    fmtst3chk("000.000000", "%010f", 0.0);
    fmtst3chk("0000000nan", "%010f", NAN);
    fmtst3chk("0000000inf", "%010f", INFINITY);
}//end test_zeropad

int main(int argc, char *argv[]) {
    test_domain_ap();
    test_domain_arg();
    test_compiled_ap();
    test_compiled_arg();
    test_extra();
    rfg1();
    rfg2();
    rfg3();
    test_dotzero();
    test_memory_leak();
    test_zeropad();
    return 0;
}//end main
