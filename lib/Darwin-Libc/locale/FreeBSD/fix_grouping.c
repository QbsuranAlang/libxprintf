/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/locale/FreeBSD/fix_grouping.c
 *
 */


#include <ctype.h>
#include <limits.h>
#include <stddef.h>

static const char x___nogrouping[] = { CHAR_MAX, '\0' };

/*
 * internal helpers for SUSv3 compatibility.  Since "nogrouping" needs to
 * be just an empty string, we provide a routine to substitute x___nogrouping
 * so we don't have to modify code that expects CHAR_MAX.
 */
const char *x___fix_nogrouping(const char *str) {
    return ((str == NULL || *str == '\0') ? x___nogrouping : str);
}//end x___fix_nogrouping
