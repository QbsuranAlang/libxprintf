/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdio/FreeBSD/fvwrite.h
 *
 */


#ifndef LIBXPRINTF_FVWRITE_H
#define LIBXPRINTF_FVWRITE_H

#include <libxprintf/libxprintf_visibility.h>

/*
 * I/O descriptors for __sfvwrite().
 */
struct x___siov {
    void    *iov_base;
    size_t  iov_len;
};
struct x___suio {
    struct x___siov *uio_iov;
    int             uio_iovcnt;
    int             uio_resid;
};

X_LOCAL int x___sfvwrite(x_FILE *, struct x___suio *);

#endif /* !LIBXPRINTF_FVWRITE_H */
