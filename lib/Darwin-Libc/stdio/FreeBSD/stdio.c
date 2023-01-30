/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/stdio.c
 *
 */


#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include "x_local.h"

#define X_OFF_MAX ((off_t)LLONG_MAX)

int x___swrite(void *cookie, char const *buf, int n) {
    x_FILE *fp;

    fp = cookie;
    return write(fp->_file, buf, (size_t)n);
}//end x___swrite

x_fpos_t x___sseek(void *cookie, x_fpos_t offset, int whence) {
    x_FILE *fp;

    fp = cookie;
    return lseek(fp->_file, (off_t)offset, whence);
}//end x___sseek

int x__swrite(x_FILE *fp, char const *buf, int n) {
    int ret, serrno;

    if(fp->_flags & X___SAPP) {
        serrno = errno;
        if(x__sseek(fp, (x_fpos_t)0, X_SEEK_END) == -1 && (fp->_flags & X___SOPT)) {
            return -1;
        }//end if
        errno = serrno;
    }//end if

    ret = (*fp->_write)(fp->_cookie, buf, n);
    /* __SOFF removed even on success in case O_APPEND mode is set. */
    if(ret >= 0) {
        if((fp->_flags & (X___SAPP | X___SOFF)) == (X___SAPP | X___SOFF) &&  fp->_offset <= X_OFF_MAX - ret) {
            fp->_offset += ret;
        }//end if
        else {
            fp->_flags &= ~X___SOFF;
        }//end else
    }//end if
    else if(ret < 0) {
        fp->_flags &= ~X___SOFF;
    }//end if

    return ret;
}//end x__swrite

x_fpos_t x__sseek(x_FILE *fp, x_fpos_t offset, int whence) {
    int         serrno, errret;
    x_fpos_t    ret;

    serrno = errno;
    errno = 0;
    ret = (*fp->_seek)(fp->_cookie, offset, whence);
    errret = errno;
    if(errno == 0) {
        errno = serrno;
    }//end if

    /*
     * Disallow negative seeks per POSIX.
     * It is needed here to help upper level caller
     * in the cases it can't detect.
     */
    if(ret < 0) {
        if(errret == 0) {
            if(offset != 0 || whence != X_SEEK_CUR) {
                if(X_HASUB(fp)) {
                    X_FREEUB(fp);
                }//end if
                fp->_p = fp->_bf._base;
                fp->_r = 0;
                fp->_flags &= ~X___SEOF;
            }//end if
            fp->_flags |= X___SERR;
            errno = EINVAL;
        }//end if
        else if(errret == ESPIPE) {
            fp->_flags &= ~X___SAPP;
        }//end if
        fp->_flags &= ~X___SOFF;
        ret = -1;
    }//end if
    else if(fp->_flags & X___SOPT) {
        fp->_flags |= X___SOFF;
        fp->_offset = ret;
    }//end if

    return ret;
}//end x__sseek
