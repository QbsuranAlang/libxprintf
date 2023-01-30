/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/include/stdio.h
 *
 */


#ifndef LIBXPRINTF_X_STDIO_H
#define LIBXPRINTF_X_STDIO_H

#include "x__stdio.h"

#define X___SLBF    0x0001 /* line buffered */
#define X___SNBF    0x0002 /* unbuffered */
#define X___SRD     0x0004 /* OK to read */
#define X___SWR     0x0008 /* OK to write */

/* RD and WR are never simultaneously asserted */
#define X___SRW     0x0010 /* open for reading & writing */
#define X___SEOF    0x0020 /* found X_EOF */
#define X___SERR    0x0040 /* found error */
#define X___SMBF    0x0080 /* _buf is from malloc */
#define X___SAPP    0x0100 /* fdopen()ed in append mode */
#define X___SSTR    0x0200 /* this is an sprintf/snprintf string */
#define X___SOPT    0x0400 /* do fseek() optimisation */
#define X___SNPT    0x0800 /* do not do fseek() optimisation */
#define X___SOFF    0x1000 /* set iff _offset is in fact correct */
#define X___SMOD    0x2000 /* true => fgetln modified _p text */
#define X___SALC    0x4000 /* allocate string space dynamically */
#define X___SIGN    0x8000 /* ignore this file in _fwalk */

#define X_BUFSIZ    1024 /* size of buffer used by setbuf */
#define X_EOF       (-1)

#define X_SEEK_SET  0 /* set file offset to offset */
#define X_SEEK_CUR  1 /* set file offset to current plus offset */
#define X_SEEK_END  2 /* set file offset to X_EOF plus offset */

#endif /* !LIBXPRINTF_X_STDIO_H */
