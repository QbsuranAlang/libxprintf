/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/wsetup.c
 *
 */


#include <errno.h>

#include "x_stdio.h"
#include "x_local.h"

/*
 * Various output routines call wsetup to be sure it is safe to write,
 * because either _flags does not include X___SWR, or _buf is NULL.
 * _wsetup returns 0 if OK to write; otherwise, it returns X_EOF and sets errno.
 */
int x___swsetup(x_FILE *fp) {

    /*
     * If we are not writing, we had better be reading and writing.
     */
    if((fp->_flags & X___SWR) == 0) {
        if((fp->_flags & X___SRW) == 0) {
            errno = EBADF;
            fp->_flags |= X___SERR;
            return X_EOF;
        }//end if
        if(fp->_flags & X___SRD) {
            /* clobber any ungetc data */
            if(X_HASUB(fp))
                X_FREEUB(fp);
            fp->_flags &= ~(X___SRD | X___SEOF);
            fp->_r = 0;
            fp->_p = fp->_bf._base;
        }//end if
        fp->_flags |= X___SWR;
    }//end if

    /*
     * Make a buffer if necessary, then set _w.
     */
    if(fp->_bf._base == NULL) {
        x___smakebuf(fp);
    }//end if
    if(fp->_flags & X___SLBF) {
        /*
         * It is line buffered, so make _lbfsize be -_bufsize
         * for the putc() macro.  We will change _lbfsize back
         * to 0 whenever we turn off X___SWR.
         */
        fp->_w = 0;
        fp->_lbfsize = -fp->_bf._size;
    }//end if
    else {
        fp->_w = fp->_flags & X___SNBF ? 0 : fp->_bf._size;
    }//end else

    return 0;
}//end x___swsetup
