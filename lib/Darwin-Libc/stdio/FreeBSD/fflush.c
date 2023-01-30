/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/fflush.c
 *
 */


#include <string.h>
#include <errno.h>

#include "x_stdio.h"
#include "x_libc_private.h"
#include "x_local.h"

int x___fflush(x_FILE *fp) {
    int retval;

    if((fp->_flags & (X___SWR | X___SRW)) == 0) {
        retval = 0;
    }//end if
    else {
        retval = x___sflush(fp);
    }//end else

    return retval;
}//end x___fflush

int x___sflush(x_FILE *fp) {
    int             n, t;
    unsigned char   *p;

    t = fp->_flags;

    if((p = fp->_bf._base) == NULL) {
        return 0;
    }//end if

    /*
     * SUSv3 requires that x_fflush() on a seekable input stream updates the file
     * position indicator with the underlying seek function.  Use a dumb fseek
     * for this (don't attempt to preserve the buffers).
     */
    if((t & X___SRD) != 0) {
        if(fp->_seek == NULL) {
            /*
             * No way to seek this file -- just return "success."
             */
            return 0;
        }//end if

        n = fp->_r;

        if(n > 0) {
            /*
             * See _fseeko's dumb path.
             */
            if(x__sseek(fp, (x_fpos_t)-n, X_SEEK_CUR) == -1) {
                if(errno == ESPIPE) {
                    /*
                     * Ignore ESPIPE errors, since there's no way to put the bytes
                     * back into the pipe.
                     */
                    return 0;
                }//end if
                return X_EOF;
            }//end if

            if(X_HASUB(fp)) {
                X_FREEUB(fp);
            }//end if

            fp->_p = fp->_bf._base;
            fp->_r = 0;
            fp->_flags &= ~X___SEOF;
            memset(&fp->_mbstate, 0, sizeof(mbstate_t));
        }//end if
        return 0;
    }//end if

    if((t & X___SWR) != 0) {
        n = fp->_p - p;        /* write this much */

        /*
         * Set these immediately to avoid problems with longjmp and to allow
         * exchange buffering (via setvbuf) in user write function.
         */
        fp->_p = p;
        fp->_w = t & (X___SLBF | X___SNBF) ? 0 : fp->_bf._size;

        for(; n > 0; n -= t, p += t) {
            t = x__swrite(fp, (char *)p, n);
            if(t <= 0) {
                /* 5340694: reset _p and _w on EAGAIN */
                if(t < 0 && errno == EAGAIN) {
                    if(p > fp->_p) {
                        /* some was written */
                        memmove(fp->_p, p, n);
                        fp->_p += n;
                        if(!(fp->_flags & (X___SLBF | X___SNBF))) {
                            fp->_w -= n;
                        }//end if
                    }//end if
                }//end if
                fp->_flags |= X___SERR;
                return X_EOF;
            }//en dif
        }//end for
    }//end if

    return 0;
}//end x___sflush
