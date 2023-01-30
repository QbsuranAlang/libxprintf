/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/makebuf.c
 *
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "x_libc_private.h"
#include "x_local.h"

#define X_MAXBUFSIZE (1 << 16)
#define X_TTYBUFSIZE 4096

/*
 * Internal routine to determine environment override buffering for a file.
 *
 * Sections of the below taken from NetBSD's version of this file under the same license.
 */
static int x___senvbuf(x_FILE *fp, size_t *bufsize, int *couldbetty) {
    return 0; /* Default to fully buffered */
}//end x___senvbuf

/*
 * Allocate a file buffer, or switch to unbuffered I/O.
 * Per the ANSI C standard, ALL tty devices default to line buffered.
 *
 * As a side effect, we set X___SOPT or X___SNPT (en/dis-able fseek
 * optimisation) right after the _fstat() that finds the buffer size.
 */
void x___smakebuf(x_FILE *fp) {
    int     flags, couldbetty;
    void    *p;
    size_t  size;

    if(fp->_flags & X___SNBF) {
        fp->_bf._base = fp->_p = fp->_nbuf;
        fp->_bf._size = 1;
        return;
    }//end if

    flags = x___swhatbuf(fp, &size, &couldbetty);
    if(fp->_file >= 0) {
        flags |= x___senvbuf(fp, &size, &couldbetty);

        if(flags & X___SNBF) {
            fp->_flags |= X___SNBF;
            fp->_bf._base = fp->_p = fp->_nbuf;
            fp->_bf._size = 1;
            return;
        }//end if
    }//end if

    if(couldbetty && isatty(fp->_file)) {
        flags |= X___SLBF;
        /* st_blksize for ttys is 128K, so make it more reasonable */
        if(size > X_TTYBUFSIZE) {
            fp->_blksize = size = X_TTYBUFSIZE;
        }//end if
    }//end if

    if((p = malloc(size)) == NULL) {
        fp->_flags |= X___SNBF;
        fp->_bf._base = fp->_p = fp->_nbuf;
        fp->_bf._size = 1;
        return;
    }//end if

    flags |= X___SMBF;
    fp->_bf._base = fp->_p = p;
    fp->_bf._size = size;
    fp->_flags |= flags;
}//end x___smakebuf

/*
 * Internal routine to determine `proper' buffering for a file.
 */
int x___swhatbuf(x_FILE *fp, size_t *bufsize, int *couldbetty) {
    struct stat st;

    if(fp->_file < 0 || fstat(fp->_file, &st) < 0) {
        *couldbetty = 0;
        *bufsize = X_BUFSIZ;
        return X___SNPT;
    }//end if

    /* could be a tty iff it is a character device */
    *couldbetty = (st.st_mode & S_IFMT) == S_IFCHR;
    if(st.st_blksize <= 0) {
        *bufsize = X_BUFSIZ;
        return X___SNPT;
    }//end if

    /*
     * Optimise fseek() only if it is a regular file.  (The test for
     * x___sseek is mainly paranoia.)  It is safe to set _blksize
     * unconditionally; it will only be used if X___SOPT is also set.
     */
    fp->_blksize = *bufsize = st.st_blksize > X_MAXBUFSIZE ? X_MAXBUFSIZE : st.st_blksize;
    return ((st.st_mode & S_IFMT) == S_IFREG && fp->_seek == x___sseek ? X___SOPT : X___SNPT);
}//end x___swhatbuf
