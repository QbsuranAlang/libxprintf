#
# APPLE LICENSE
#


AM_CFLAGS = -g
AM_CPPFLAGS = \
    -include $(top_srcdir)/config.h \
    -I$(top_srcdir)/lib/includes \
    -I$(top_srcdir)/examples
LDADD = \
    @LIB_LIBS_SET@ \
    $(top_srcdir)/lib/Darwin-Libc/libxprintf.la

noinst_PROGRAMS = \
    01_domain \
    02_comp_exec

01_domain_SOURCES = \
    01_domain.c \
    xprintf_callback.c \
    xprintf_callback.h

02_comp_exec_SOURCES = \
    02_comp_exec.c \
    xprintf_callback.c \
    xprintf_callback.h
