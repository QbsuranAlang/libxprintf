#
# APPLE LICENSE
#


SUBDIRS = lib examples tests

ACLOCAL_AMFLAGS = -I m4

.PHONY: examples valgrind

clean-local:
	find . -name 'core.*' -type f -exec rm -f {} \;
	find . -name 'vgcore.*' -type f -exec rm -f {} \;
	find . -name '._*' -exec rm -rf {} \;

examples:
	$(MAKE) -C lib
	$(MAKE) -C examples

valgrind:
	$(MAKE) -C tests valgrind
