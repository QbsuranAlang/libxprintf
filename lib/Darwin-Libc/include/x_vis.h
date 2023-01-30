/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/include/vis.h
 *
 */


#ifndef LIBXPRINTF_X_VIS_H
#define LIBXPRINTF_X_VIS_H

#include <sys/types.h>

#include <libxprintf/libxprintf_visibility.h>

/*
 * to select alternate encoding format
 */
#define X_VIS_OCTAL     0x0001 /* use octal \ddd format */
#define X_VIS_CSTYLE    0x0002 /* use \[nrft0..] where appropiate */

/*
 * to alter set of characters encoded (default is to encode all
 * non-graphic except space, tab, and newline).
 */
#define X_VIS_SP    0x0004 /* also encode space */
#define X_VIS_TAB   0x0008 /* also encode tab */
#define X_VIS_NL    0x0010 /* also encode newline */
#define X_VIS_WHITE (X_VIS_SP | X_VIS_TAB | X_VIS_NL)
#define X_VIS_SAFE  0x0020 /* only encode "unsafe" characters */
#define X_VIS_DQ    0x8000 /* also encode double quotes */

/*
 * other
 */
#define X_VIS_NOSLASH   0x0040 /* inhibit printing '\' */
#define X_VIS_HTTP1808  0x0080 /* http-style escape % hex hex */
#define X_VIS_HTTPSTYLE 0x0080 /* http-style escape % hex hex */
#define X_VIS_GLOB      0x0100 /* encode glob(3) magic characters */
#define X_VIS_MIMESTYLE 0x0200 /* mime-style escape = HEX HEX */
#define X_VIS_HTTP1866  0x0400 /* http-style &#num; or &string; */
#define X_VIS_NOESCAPE  0x0800 /* don't decode `\' */
#define X__VIS_END      0x1000 /* for unvis */
#define X_VIS_SHELL     0x2000 /* encode shell special characters [not glob] */
#define X_VIS_META      (X_VIS_WHITE | X_VIS_GLOB | X_VIS_SHELL)
#define X_VIS_NOLOCALE  0x4000 /* encode using the C locale */

/*
 * unvis return codes
 */
#define X_UNVIS_VALID       1   /* character valid */
#define X_UNVIS_VALIDPUSH   2   /* character valid, push back passed char */
#define X_UNVIS_NOCHAR      3   /* valid sequence, no character produced */
#define X_UNVIS_SYNBAD      -1  /* unrecognized escape sequence */
#define X_UNVIS_ERROR       -2  /* decoder in unknown state (unrecoverable) */

/*
 * x_unvis flags
 */
#define X_UNVIS_END X__VIS_END /* no more characters */

__BEGIN_DECLS

X_LOCAL char *x_vis(char *, int, int, int);
X_LOCAL char *x_nvis(char *, size_t, int, int, int);

X_LOCAL char *x_svis(char *, int, int, int, const char *);
X_LOCAL char *x_snvis(char *, size_t, int, int, int, const char *);

X_LOCAL int x_strvis(char *, const char *, int);
X_LOCAL int x_stravis(char **, const char *, int);
X_LOCAL int x_strnvis(char *, size_t, const char *, int);

X_LOCAL int x_strsvis(char *, const char *, int, const char *);
X_LOCAL int x_strsnvis(char *, size_t, const char *, int, const char *);

X_LOCAL int x_strvisx(char *, const char *, size_t, int);
X_LOCAL int x_strnvisx(char *, size_t, const char *, size_t, int);
X_LOCAL int x_strenvisx(char *, size_t, const char *, size_t, int, int *);

X_LOCAL int x_strsvisx(char *, const char *, size_t, int, const char *);
X_LOCAL int x_strsnvisx(char *, size_t, const char *, size_t, int, const char *);
X_LOCAL int x_strsenvisx(char *, size_t, const char *, size_t , int, const char *, int *);

X_LOCAL int x_strunvis(char *, const char *);
X_LOCAL int x_strnunvis(char *, size_t, const char *);

X_LOCAL int x_strunvisx(char *, const char *, int);
X_LOCAL int x_strnunvisx(char *, size_t, const char *, int);

X_LOCAL int x_unvis(char *, int, int *, int);

__END_DECLS

#endif /* !LIBXPRINTF_X_VIS_H */
