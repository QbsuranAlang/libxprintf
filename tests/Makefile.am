#
# APPLE LICENSE
#


AM_CFLAGS = -Wall -Wno-unused-function -g -DDEBUG
AM_CPPFLAGS = \
    -include $(top_srcdir)/config.h \
    -I$(top_srcdir)/lib/includes
LDADD = \
    @LIB_LIBS_SET@ \
    $(top_srcdir)/lib/Darwin-Libc/libxprintf.la

check_PROGRAMS = \
    xprintf

xprintf_SOURCES = \
    test_xprintf.c \
    xprintf_callback.c \
    xprintf_callback.h

TESTS = $(check_PROGRAMS)

.PHONY: valgrind

valgrind: $(check_PROGRAMS)
	@list='$(check_PROGRAMS)'; test -n "$$list" || exit 0; \
	arr='' || true; \
	for p in $$list; do \
		valgrind --error-exitcode=3 --leak-check=full --track-origins=yes --track-fds=yes ./"$$p"; \
		if test $$? -eq 3 ;then arr="`echo $$arr $$p`"; fi; \
	done; \
	if test -n "$$arr"; then \
		printf "\n>>>>> valgrind reported errors on \"$$arr\" <<<<<\n\n"; \
	fi
