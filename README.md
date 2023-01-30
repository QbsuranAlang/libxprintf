libxprintf
==============

A customizable `printf()` family functions base on `OSX(Darwin)` `xprintf()` implementations. See [man 3 xprintf](https://www.unix.com/man-page/osx/3/xprintf).

The library have reserved the minimum requirement and remove redundant code.

The source code is copied from [Libc-1534.40.2 tarball](https://github.com/apple-oss-distributions/Libc/archive/Libc-1534.40.2.tar.gz) and [Libc-1534.40.2 github](https://github.com/apple-oss-distributions/Libc/tree/Libc-1534.40.2).

The `GNU C` library has provided similar implementation, but they affect `printf()` globally. Defaults will be affected and it is not safe. See [Customizing printf](https://www.gnu.org/software/libc/manual/html_node/Customizing-Printf.html).

To prevent symbol duplication, most variable and function prepend the prefix `x_` or `X_`.

All `xprintf()` function implemented three ways for accepted arguments.

1. `Variable Arguments`
2. `va_list`, see [man 3 stdarg](https://www.unix.com/man-page/osx/3/stdarg).
3. `void **args`

Supported a function(`x_new_printf_comp_dry_run()`) for dry running compiling the formatted string.

The license is copied from the source and most is APPLE LICENSE, I am not sure I am doing right or not. Please guide me the correct way to handle the license issue.

APIs
---------

All APIs are declared in `libxprintf.h`.

<h3>xprintf_domain</h3>

See [man 3 xprintf_domain](https://www.unix.com/man-page/osx/3/xprintf_domain).

```c
x_printf_domain_t x_copy_printf_domain(x_printf_domain_t __domain);
void x_free_printf_domain(x_printf_domain_t __domain);
x_printf_domain_t x_new_printf_domain(void);
int x_register_printf_domain_function(x_printf_domain_t __domain,
        int __spec, x_printf_function *__render,
        x_printf_arginfo_function *__arginfo, void *__context);
int x_register_printf_domain_render_std(x_printf_domain_t __domain,
        const char *__specs);
```

<h3>xprintf() all in one</h3>

See [man 3 xprintf](https://www.unix.com/man-page/osx/3/xprintf).

Three types for accepted arguments:

1. `x_asxprintf()`, `x_dxprintf()`, `x_fxprintf()`, `x_fxprintf_std()`, `x_sxprintf()` and `x_xprintf()` are accepted the `variable arguments`. 
2. `x_vasxprintf()`, `x_vdxprintf()`, `x_vfxprintf()`, `x_vfxprintf_std()`, `x_vsxprintf()` and `x_vxprintf()` are accepted `va_list`. See [man 3 stdarg](https://www.unix.com/man-page/osx/3/stdarg).
3. `x_lasxprintf()`, `x_ldxprintf()`, `x_lfxprintf()`, `x_lfxprintf_std()`, `x_lsxprintf()` and `x_lxprintf()` are accepted `void **args`.

```c
int x_asxprintf(char ** __restrict __ret,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...);
int x_dxprintf(int __fd, x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...);
int x_fxprintf(x_FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...);
int x_fxprintf_std(FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...);
int x_sxprintf(char * __restrict __str, size_t __size,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...);
int x_xprintf(x_printf_domain_t __restrict __domain,
        const char * __restrict __format, ...);

int x_vasxprintf(char ** __restrict __ret,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, va_list __ap);
int x_vdxprintf(int __fd, x_printf_domain_t __restrict __domain,
        const char * __restrict __format,
        va_list __ap);
int x_vfxprintf(x_FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, va_list __ap);
int x_vfxprintf_std(FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, va_list __ap);
int x_vsxprintf(char * __restrict __str, size_t __size,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, va_list __ap);
int x_vxprintf(x_printf_domain_t __restrict __domain,
        const char * __restrict __format,
        va_list __ap);

int x_lasxprintf(char ** __restrict __ret,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, void ** __restrict __args);
int x_ldxprintf(int __fd, x_printf_domain_t __restrict __domain,
        const char * __restrict __format,
        void ** __restrict __args);
int x_lfxprintf(x_FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, void ** __restrict __args);
int x_lfxprintf_std(FILE * __restrict __stream,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, void ** __restrict __args);
int x_lsxprintf(char * __restrict __str, size_t __size,
        x_printf_domain_t __restrict __domain,
        const char * __restrict __format, void ** __restrict __args);
int x_lxprintf(x_printf_domain_t __restrict __domain,
        const char * __restrict __format,
        void ** __restrict __args);
```

<h3>xprintf_comp</h3>

See [man 3 xprintf_comp](https://www.unix.com/man-page/mojave/3/xprintf_comp).

```c
void x_free_printf_comp(x_printf_comp_t __pc);
x_printf_comp_t x_new_printf_comp(x_printf_domain_t __restrict __domain,
        const char * __restrict __fmt);
int x_new_printf_comp_dry_run(x_printf_domain_t __restrict __domain,
        const char * __restrict __fmt);
```

<h3>xprintf_exec</h3>

See [man 3 xprintf_exec](https://www.unix.com/man-page/mojave/3/xprintf_exec).

Three types for accepted arguments:

1. `x_asxprintf_exec()`, `x_dxprintf_exec()`, `x_fxprintf_exec()`, `x_fxprintf_std_exec()`, `x_sxprintf_exec()` and `x_xprintf_exec()` are accepted the `variable arguments`. 
2. `x_vasxprintf_exec()`, `x_vdxprintf_exec()`, `x_vfxprintf_exec()`, `x_vfxprintf_std_exec()`, `x_vsxprintf_exec()` and `x_vxprintf_exec()` are accepted `va_list`. See [man 3 stdarg](https://www.unix.com/man-page/osx/3/stdarg).
3. `x_lasxprintf_exec()`, `x_ldxprintf_exec()`, `x_lfxprintf_exec()`, `x_lfxprintf_std_exec()`, `x_lsxprintf_exec()` and `x_lxprintf_exec()` are accepted `void **args`.

```c
int x_asxprintf_exec(char ** __restrict __ret,
        x_printf_comp_t __restrict __pc, ...);
int x_dxprintf_exec(int __fd, x_printf_comp_t __restrict __pc, ...);
int x_fxprintf_exec(x_FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, ...);
int x_fxprintf_std_exec(FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, ...);
int x_sxprintf_exec(char * __restrict __str, size_t __size,
        x_printf_comp_t __restrict __pc, ...);
int x_xprintf_exec(x_printf_comp_t __restrict __pc, ...);

int x_vasxprintf_exec(char ** __restrict __ret,
        x_printf_comp_t __restrict __pc, va_list __ap);
int x_vdxprintf_exec(int __fd, x_printf_comp_t __restrict __pc,
        va_list __ap);
int x_vfxprintf_exec(x_FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, va_list __ap);
int x_vfxprintf_std_exec(FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, va_list __ap);
int x_vsxprintf_exec(char * __restrict __str, size_t __size,
        x_printf_comp_t __restrict __pc, va_list __ap);
int x_vxprintf_exec(x_printf_comp_t __restrict __pc, va_list __ap);

int x_lasxprintf_exec(char ** __restrict __ret,
        x_printf_comp_t __restrict __pc, void ** __restrict __args);
int x_ldxprintf_exec(int __fd, x_printf_comp_t __restrict __pc,
        void ** __restrict __args);
int x_lfxprintf_exec(x_FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, void ** __restrict __args);
int x_lfxprintf_std_exec(FILE * __restrict __stream,
        x_printf_comp_t __restrict __pc, void ** __restrict __args);
int x_lsxprintf_exec(char * __restrict __str, size_t __size,
        x_printf_comp_t __restrict __pc, void ** __restrict __args);
int x_lxprintf_exec(x_printf_comp_t __restrict __pc, void ** __restrict __args);
```
