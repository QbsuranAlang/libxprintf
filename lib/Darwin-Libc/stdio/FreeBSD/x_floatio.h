/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/floatio.h
 *
 */


#ifndef LIBXPRINTF_FLOATIO_H
#define LIBXPRINTF_FLOATIO_H

#include <sys/cdefs.h>

#include <libxprintf/libxprintf_visibility.h>

#define X_MAXEXPDIG 6
#if LDBL_MAX_EXP > 999999
#error "floating point buffers too small"
#endif

__BEGIN_DECLS

X_LOCAL char *x___hdtoa(double, const char *, int, int *, int *, char **);
X_LOCAL char *x___hldtoa(long double, const char *, int, int *, int *, char **);
X_LOCAL char *x___ldtoa(long double *, int, int, int *, int *, char **);

__END_DECLS

#endif /* !LIBXPRINTF_FLOATIO_H */
