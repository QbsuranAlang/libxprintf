/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/include/_stdio.h
 *
 */


#ifndef LIBXPRINTF_X__STDIO_H
#define LIBXPRINTF_X__STDIO_H

#include <sys/types.h> /* for off_t */

typedef off_t x_fpos_t;

/*
 * NB: to fit things in six character monocase externals, the stdio
 * code uses the prefix `__s' for stdio objects, typically followed
 * by a three-character attempt at a mnemonic.
 */

/* stdio buffers */
struct x___sbuf {
    unsigned char   *_base;
    int             _size;
};

/* hold a buncha junk that would grow the ABI */
struct x___sFILEX;

/*
 * stdio state variables.
 *
 * The following always hold:
 *
 *    if (_flags&(__SLBF | __SWR)) == (__SLBF | __SWR),
 *        _lbfsize is -_bf._size, else _lbfsize is 0
 *    if _flags&__SRD, _w is 0
 *    if _flags&__SWR, _r is 0
 *
 * This ensures that the getc and putc macros (or inline functions) never
 * try to write or read from a file that is in `read' or `write' mode.
 * (Moreover, they can, and do, automatically switch from read mode to
 * write mode, and back, on "r+" and "w+" files.)
 *
 * _lbfsize is used only to make the inline line-buffered output stream
 * code as compact as possible.
 *
 * _ub, _up, and _ur are used when ungetc() pushes back more characters
 * than fit in the current _bf, or when ungetc() pushes back a character
 * that does not match the previous one in _bf.  When this happens,
 * _ub._base becomes non-nil (i.e., a stream has ungetc() data iff
 * _ub._base!=NULL) and _up and _ur save the current values of _p and _r.
 *
 * NB: see WARNING above before changing the layout of this structure!
 */
struct x___sFILE {
    unsigned char   *_p;            /* current position in (some) buffer */
    int             _r;             /* read space left for getc() */
    int             _w;             /* write space left for putc() */
    short           _flags;         /* flags, below; this FILE is free if 0 */
    short           _file;          /* fileno, if Unix descriptor, else -1 */
    struct          x___sbuf _bf;   /* the buffer (at least 1 byte, if !NULL) */
    int             _lbfsize;       /* 0 or -_bf._size, for inline putc */

    /* operations */
    void        *_cookie;           /* cookie passed to io functions */
    int         (*_close)(void *);
    int         (*_read)(void *, char *, int);
    x_fpos_t    (*_seek)(void *, x_fpos_t, int);
    int         (*_write)(void *, const char *, int);

    /* separate buffer for long sequences of ungetc() */
    struct x___sbuf     _ub;        /* ungetc buffer */
    struct x___sFILEX   *_extra;    /* additions to FILE to not break ABI */
    int                 _ur;        /* saved _r when _r is counting ungetc data */

    /* tricks to meet minimum requirements even when malloc() fails */
    unsigned char _ubuf[3];         /* guarantee an ungetc() buffer */
    unsigned char _nbuf[1];         /* guarantee a getc() buffer */

    /* separate buffer for fgetln() when line crosses buffer boundary */
    struct x___sbuf _lb;            /* buffer for fgetln() */

    /* Unix stdio files get aligned to block boundaries on fseek() */
    int         _blksize;           /* stat.st_blksize (may be != _bf._size) */
    x_fpos_t    _offset;            /* current lseek offset (see WARNING) */
};

#endif /* !LIBXPRINTF_X__STDIO_H */
