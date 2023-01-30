#
# ORIGINAL LICENSE
#


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libxprintf.pc
DISTCLEANFILES = $(pkgconfig_DATA)

AM_CFLAGS = @LIB_CFLAGS_SET@
AM_LDFLAGS = @LIB_LDFLAGS_SET@
LDADD = @LIB_LIBS_SET@

AM_CPPFLAGS = \
    -include $(top_srcdir)/config.h \
    -I$(top_srcdir)/lib/Darwin-Libc/darwin \
    -I$(top_srcdir)/lib/Darwin-Libc/fbsdcompat \
    -I$(top_srcdir)/lib/Darwin-Libc/gdtoa \
    -I$(top_srcdir)/lib/Darwin-Libc/gdtoa/gdtoa \
    -I$(top_srcdir)/lib/Darwin-Libc/include \
    -I$(top_srcdir)/lib/Darwin-Libc/stdio \
    -I$(top_srcdir)/lib/Darwin-Libc/stdio/FreeBSD \
    -I$(top_srcdir)/lib/includes \
    -DBUILDING_LIBXPRINTF

OBJECTS = \
    darwin/x_libc_private.h \
    fbsdcompat/sys/x_endian.h \
    fbsdcompat/x__fpmath.h \
    fbsdcompat/x_fpmath.h \
    gdtoa/FreeBSD/_hdtoa.c \
    gdtoa/FreeBSD/_ldtoa.c \
    gdtoa/FreeBSD/gdtoa-dtoa.c \
    gdtoa/gdtoa/dmisc.c \
    gdtoa/gdtoa/gdtoa.c \
    gdtoa/gdtoa/gmisc.c \
    gdtoa/gdtoa/misc.c \
    gdtoa/gdtoa/x_gdtoa.h \
    gdtoa/gdtoa/x_gdtoaimp.h \
    gdtoa/x_arith.h \
    gdtoa/x_gd_qnan.h \
    gen/FreeBSD/unvis.c \
    gen/FreeBSD/vis.c \
    include/x__stdio.h \
    include/x_math.h \
    include/x_stdio.h \
    include/x_stdlib.h \
    include/x_vis.h \
    locale/FreeBSD/fix_grouping.c \
    stdio/FreeBSD/fflush.c \
    stdio/FreeBSD/fvwrite.c \
    stdio/FreeBSD/makebuf.c \
    stdio/FreeBSD/stdio.c \
    stdio/FreeBSD/vasprintf.c \
    stdio/FreeBSD/vdprintf.c \
    stdio/FreeBSD/vsnprintf.c \
    stdio/FreeBSD/wsetup.c \
    stdio/FreeBSD/x_floatio.h \
    stdio/FreeBSD/x_fvwrite.h \
    stdio/FreeBSD/x_local.h \
    stdio/FreeBSD/x_printflocal.h \
    stdio/FreeBSD/x_xprintf_private.h \
    stdio/FreeBSD/xprintf.c \
    stdio/FreeBSD/xprintf_errno.c \
    stdio/FreeBSD/xprintf_float.c \
    stdio/FreeBSD/xprintf_hexdump.c \
    stdio/FreeBSD/xprintf_int.c \
    stdio/FreeBSD/xprintf_quote.c \
    stdio/FreeBSD/xprintf_str.c \
    stdio/FreeBSD/xprintf_time.c \
    stdio/FreeBSD/xprintf_vis.c \
    stdio/x_xprintf_domain.h \
    stdio/xprintf_all_in_one.c \
    stdio/xprintf_comp.c \
    stdio/xprintf_domain.c \
    stdio/xprintf_exec.c \
    stdlib/FreeBSD/reallocf.c

lib_LTLIBRARIES = libxprintf.la
libxprintf_la_SOURCES = $(OBJECTS)
libxprintf_la_LDFLAGS = -version-info @LIBRARY_VERSION_NUM@
