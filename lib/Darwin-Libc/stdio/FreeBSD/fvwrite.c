/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/fvwrite.c
 *
 */


#include <string.h>

#include "x_stdio.h"
#include "x_local.h"
#include "x_fvwrite.h"
#include "x_stdlib.h"

/*
 * Write some memory regions.  Return zero on success, X_EOF on error.
 *
 * This routine is large and unsightly, but most of the ugliness due
 * to the three different kinds of output buffering is handled here.
 */
int x___sfvwrite(x_FILE *fp, struct x___suio *uio) {
    int             w, s, nlknown, nldist;
    char            *p, *nl;
    size_t          len, blen;
    struct x___siov *iov;

    if(uio->uio_resid == 0) {
        return 0;
    }//end if

    /* make sure we can write */
    if(x_prepwrite(fp) != 0) {
        return X_EOF;
    }//end if

#define X_MIN(a, b) ((a) < (b) ? (a) : (b))
#define X_COPY(n)   (void)memcpy((void *)fp->_p, (void *)p, (size_t)(n))

    iov = uio->uio_iov;
    p = iov->iov_base;
    len = iov->iov_len;
    iov++;

#define X_GETIOV(extra_work)    \
    while(len == 0) {           \
        extra_work;             \
        p = iov->iov_base;      \
        len = iov->iov_len;     \
        iov++;                  \
    }

    if(fp->_flags & X___SNBF) {
        /*
         * Unbuffered: write up to BUFSIZ bytes at a time.
         */
        do {
            X_GETIOV(;);
            w = x__swrite(fp, p, X_MIN(len, X_BUFSIZ));
            if(w <= 0) {
                goto err;
            }//end if
            p += w;
            len -= w;
        } while((uio->uio_resid -= w) != 0);
    }//end if
    else if((fp->_flags & X___SLBF) == 0) {
        /*
         * Fully buffered: fill partially full buffer, if any,
         * and then flush.  If there is no partial buffer, write
         * one _bf._size byte chunk directly (without copying).
         *
         * String output is a special case: write as many bytes
         * as fit, but pretend we wrote everything.  This makes
         * snprintf() return the number of bytes needed, rather
         * than the number used, and avoids its write function
         * (so that the write function can be invalid).
         */
        do {
            X_GETIOV(;);
            if((fp->_flags & (X___SALC | X___SSTR)) == (X___SALC | X___SSTR) && fp->_w < len) {

                blen = fp->_p - fp->_bf._base;

                /*
                 * Alloc an extra 128 bytes (+ 1 for NULL)
                 * so we don't call realloc(3) so often.
                 */
                fp->_w = len + 128;
                fp->_bf._size = blen + len + 128;
                fp->_bf._base = x_reallocf(fp->_bf._base, fp->_bf._size + 1);
                if(fp->_bf._base == NULL) {
                    goto err;
                }//end if
                fp->_p = fp->_bf._base + blen;
            }//end if
            w = fp->_w;
            if(fp->_flags & X___SSTR) {
                if(len < w) {
                    w = len;
                }//end if
                if(w > 0) {
                    X_COPY(w);        /* copy X_MIN(fp->_w,len), */
                    fp->_w -= w;
                    fp->_p += w;
                }//end if
                w = len;    /* but pretend copied all */
            }//end if
            else if(fp->_p > fp->_bf._base && len > w) {
                /* fill and flush */
                X_COPY(w);
                /* fp->_w -= w; */ /* unneeded */
                fp->_p += w;
                if(x___fflush(fp)) {
                    goto err;
                }//end if
            }//end if
            else if(len >= (w = fp->_bf._size)) {
                /* write directly */
                w = x__swrite(fp, p, w);
                if(w <= 0) {
                    goto err;
                }//end if
            }//end if
            else {
                /* fill and done */
                w = len;
                X_COPY(w);
                fp->_w -= w;
                fp->_p += w;
            }//end else
            p += w;
            len -= w;
        } while((uio->uio_resid -= w) != 0);
    }//end if
    else {
        /*
         * Line buffered: like fully buffered, but we
         * must check for newlines.  Compute the distance
         * to the first newline (including the newline),
         * or `infinity' if there is none, then pretend
         * that the amount to write is X_MIN(len,nldist).
         */
        nlknown = 0;
        nldist = 0;    /* XXX just to keep gcc happy */
        do {
            X_GETIOV(nlknown = 0);
            if(!nlknown) {
                nl = memchr((void *)p, '\n', len);
                nldist = nl ? nl + 1 - p : len + 1;
                nlknown = 1;
            }//end if
            s = X_MIN(len, nldist);
            w = fp->_w + fp->_bf._size;
            if(fp->_p > fp->_bf._base && s > w) {
                X_COPY(w);
                /* fp->_w -= w; */
                fp->_p += w;
                if(x___fflush(fp)) {
                    goto err;
                }//end if
            }//end if
            else if(s >= (w = fp->_bf._size)) {
                w = x__swrite(fp, p, w);
                if(w <= 0) {
                    goto err;
                }//end if
            }//end if
            else {
                w = s;
                X_COPY(w);
                fp->_w -= w;
                fp->_p += w;
            }//end else
            if((nldist -= w) == 0) {
                /* copied the newline: flush and forget */
                if(x___fflush(fp))
                    goto err;
                nlknown = 0;
            }//end if
            p += w;
            len -= w;
        } while((uio->uio_resid -= w) != 0);
    }//end else

    return 0;
err:
    fp->_flags |= X___SERR;
    return X_EOF;
}//end x___sfvwrite
