/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gen/FreeBSD/vis.c
 *
 */


#include <stdint.h>
#include <errno.h>
#include <wchar.h>
#include <wctype.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <sys/param.h>

#include "x_vis.h"
#include "x_libc_private.h"

/*
 * The reason for going through the trouble to deal with character encodings
 * in vis(3), is that we use this to safe encode output of commands. This
 * safe encoding varies depending on the character set. For example if we
 * display ps output in French, we don't want to display French characters
 * as M-foo.
 */

static wchar_t *x_do_svis(wchar_t *, wint_t, int, wint_t, const wchar_t *);

#undef BELL
#define BELL L'\a'

#if defined(LC_C_LOCALE)
#define x_iscgraph(c) isgraph_l(c, LC_C_LOCALE)
#else /* defined(LC_C_LOCALE) */
/* Keep it simple for now, no locale stuff */
#define x_iscgraph(c) isgraph(c)
#endif /* !defined(LC_C_LOCALE) */

#define X_ISGRAPH(flags, c) \
    (((flags) & X_VIS_NOLOCALE) ? x_iscgraph(c) : iswgraph(c))

#define x_iswoctal(c)   (((u_char)(c)) >= L'0' && ((u_char)(c)) <= L'7')
#define x_iswwhite(c)   (c == L' ' || c == L'\t' || c == L'\n')
#define x_iswsafe(c)    (c == L'\b' || c == BELL || c == L'\r')
#define x_xtoa(c)       L"0123456789abcdef"[c]
#define X_XTOA(c)       L"0123456789ABCDEF"[c]

#define X_MAXEXTRAS     30

static const wchar_t    x_char_shell[] = L"'`\";&<>()|{}]\\$!^~";
static const wchar_t    x_char_glob[] = L"*?[#";

/*
 * This is x_do_hvis, for HTTP style (RFC 1808)
 */
static wchar_t *x_do_hvis(wchar_t *dst, wint_t c, int flags, wint_t nextc, const wchar_t *extra) {
    if(iswalnum(c) ||
        /* safe */
        c == L'$' || c == L'-' || c == L'_' || c == L'.' || c == L'+' ||
        /* extra */
        c == L'!' || c == L'*' || c == L'\'' || c == L'(' || c == L')' ||
        c == L',') {
        dst = x_do_svis(dst, c, flags, nextc, extra);
    }//end if
    else {
        *dst++ = L'%';
        *dst++ = x_xtoa(((unsigned int)c >> 4) & 0xf);
        *dst++ = x_xtoa((unsigned int)c & 0xf);
    }//end elsee

    return dst;
}//end x_do_hvis

/*
 * This is x_do_mvis, for Quoted-Printable MIME (RFC 2045)
 * NB: No handling of long lines or CRLF.
 */
static wchar_t *x_do_mvis(wchar_t *dst, wint_t c, int flags, wint_t nextc, const wchar_t *extra) {
    if((c != L'\n') &&
        /* Space at the end of the line */
        ((iswspace(c) && (nextc == L'\r' || nextc == L'\n')) ||
        /* Out of range */
        (!iswspace(c) && (c < 33 || (c > 60 && c < 62) || c > 126)) ||
        /* Specific char to be escaped */
        wcschr(L"#$@[\\]^`{|}~", c) != NULL)) {
        *dst++ = L'=';
        *dst++ = X_XTOA(((unsigned int)c >> 4) & 0xf);
        *dst++ = X_XTOA((unsigned int)c & 0xf);
    }//end if
    else {
        dst = x_do_svis(dst, c, flags, nextc, extra);
    }//end else
    return dst;
}//end x_do_mvis

/*
 * Output single byte of multibyte character.
 */
static wchar_t *x_do_mbyte(wchar_t *dst, wint_t c, int flags, wint_t nextc, int iswextra) {
    if(flags & X_VIS_CSTYLE) {
        switch(c) {
        case L'\n':
            *dst++ = L'\\'; *dst++ = L'n';
            return dst;
        case L'\r':
            *dst++ = L'\\'; *dst++ = L'r';
            return dst;
        case L'\b':
            *dst++ = L'\\'; *dst++ = L'b';
            return dst;
        case BELL:
            *dst++ = L'\\'; *dst++ = L'a';
            return dst;
        case L'\v':
            *dst++ = L'\\'; *dst++ = L'v';
            return dst;
        case L'\t':
            *dst++ = L'\\'; *dst++ = L't';
            return dst;
        case L'\f':
            *dst++ = L'\\'; *dst++ = L'f';
            return dst;
        case L' ':
            *dst++ = L'\\'; *dst++ = L's';
            return dst;
        case L'\0':
            *dst++ = L'\\'; *dst++ = L'0';
            if(x_iswoctal(nextc)) {
                *dst++ = L'0';
                *dst++ = L'0';
            }//end if
            return dst;
        /* We cannot encode these characters in X_VIS_CSTYLE
         * because they special meaning */
        case L'n':
        case L'r':
        case L'b':
        case L'a':
        case L'v':
        case L't':
        case L'f':
        case L's':
        case L'0':
        case L'M':
        case L'^':
        case L'$': /* vis(1) -l */
            break;
        default:
            if(X_ISGRAPH(flags, c) && !x_iswoctal(c)) {
                *dst++ = L'\\';
                *dst++ = c;
                return dst;
            }//end if
        }//end switch
    }//end if
    if(iswextra || ((c & 0177) == L' ') || (flags & X_VIS_OCTAL)) {
        *dst++ = L'\\';
        *dst++ = (u_char)(((u_int32_t)(u_char)c >> 6) & 03) + L'0';
        *dst++ = (u_char)(((u_int32_t)(u_char)c >> 3) & 07) + L'0';
        *dst++ = (c & 07) + L'0';
    }//end if
    else {
        if((flags & X_VIS_NOSLASH) == 0) {
            *dst++ = L'\\';
        }//end if

        if(c & 0200) {
            c &= 0177;
            *dst++ = L'M';
        }//end if

        if(iswcntrl(c)) {
            *dst++ = L'^';
            if(c == 0177) {
                *dst++ = L'?';
            }//end if
            else {
                *dst++ = c + L'@';
            }//end else
        }//end if
        else {
            *dst++ = L'-';
            *dst++ = c;
        }//end else
    }//end else

    return dst;
}//end x_do_mbyte

/*
 * This is x_do_vis, the central code of vis.
 * dst:          Pointer to the destination buffer
 * c:          Character to encode
 * flags:     Flags word
 * nextc:     The character following 'c'
 * extra:     Pointer to the list of extra characters to be
 *          backslash-protected.
 */
static wchar_t *x_do_svis(wchar_t *dst, wint_t c, int flags, wint_t nextc, const wchar_t *extra) {
    int         iswextra, i, shft;
    uint64_t    bmsk, wmsk;

    iswextra = wcschr(extra, c) != NULL;
    if(!iswextra && (X_ISGRAPH(flags, c) || x_iswwhite(c) ||
        ((flags & X_VIS_SAFE) && x_iswsafe(c)))) {
        *dst++ = c;
        return dst;
    }//end if

    /* See comment in x_istrsenvisx() output loop, below. */
    wmsk = 0;
    for(i = sizeof(wmsk) - 1; i >= 0; i--) {
        shft = i * NBBY;
        bmsk = (uint64_t)0xffLL << shft;
        wmsk |= bmsk;
        if((c & wmsk) || i == 0) {
            dst = x_do_mbyte(dst, (wint_t)((uint64_t)(c & bmsk) >> shft), flags, nextc, iswextra);
        }//end if
    }//end for

    return dst;
}//end x_do_svis

typedef wchar_t *(*visfun_t)(wchar_t *, wint_t, int, wint_t, const wchar_t *);

/*
 * Return the appropriate encoding function depending on the flags given.
 */
static visfun_t x_getvisfun(int flags) {
    if(flags & X_VIS_HTTPSTYLE) {
        return x_do_hvis;
    }//end if
    if(flags & X_VIS_MIMESTYLE) {
        return x_do_mvis;
    }//end if
    return x_do_svis;
}//end x_getvisfun

/*
 * Expand list of extra characters to not visually encode.
 */
static wchar_t *x_makeextralist(int flags, const char *src) {
    size_t          len, i;
    wchar_t         *dst, *d;
    mbstate_t       mbstate;
    const wchar_t   *s;

    memset(&mbstate, 0, sizeof(mbstate));
    len = strlen(src);
    if((dst = calloc(len + X_MAXEXTRAS, sizeof(*dst))) == NULL) {
        return NULL;
    }//end if

    if((flags & X_VIS_NOLOCALE) || mbsrtowcs(dst, &src, len, &mbstate) == (size_t)-1) {
        for(i = 0; i < len; i++) {
            dst[i] = (wchar_t)(u_char)src[i];
        }//end for
        d = dst + len;
    }//end if
    else {
        d = dst + wcslen(dst);
    }//end else

    if(flags & X_VIS_GLOB) {
        for(s = x_char_glob; *s; *d++ = *s++) {
            continue;
        }//end for
    }//end if

    if(flags & X_VIS_SHELL) {
        for(s = x_char_shell; *s; *d++ = *s++) {
            continue;
        }//end for
    }//end if

    if(flags & X_VIS_SP) {
        *d++ = L' ';
    }//end if
    if(flags & X_VIS_TAB) {
        *d++ = L'\t';
    }//end if
    if(flags & X_VIS_NL) {
        *d++ = L'\n';
    }//end if
    if(flags & X_VIS_DQ) {
        *d++ = L'"';
    }//end if
    if((flags & X_VIS_NOSLASH) == 0) {
        *d++ = L'\\';
    }//end if
    *d = L'\0';

    return dst;
}//end x_makeextralist

/*
 * x_istrsenvisx()
 *     The main internal function.
 *    All user-visible functions call this one.
 */
static int x_istrsenvisx(char **mbdstp, size_t *dlen, const char *mbsrc, size_t mblength, int flags, const char *mbextra, int *cerr_ptr) {
    int         clen, cerr, error, i, shft;
    char        *mbdst, *mdst;
    wint_t      c;
    size_t      len, olen, mbslength, maxolen;
    wchar_t     *dst, *src, *pdst, *psrc, *start, *extra;
    uint64_t    bmsk, wmsk;
    visfun_t    f;
    mbstate_t   mbstate;

    clen = 0;
    error = -1;

    mbslength = (ssize_t)mblength;
    /*
     * When inputing a single character, must also read in the
     * next character for nextc, the look-ahead character.
     */
    if(mbslength == 1) {
        mbslength++;
    }//end if

    /*
     * Input (mbsrc) is a char string considered to be multibyte
     * characters.  The input loop will read this string pulling
     * one character, possibly multiple bytes, from mbsrc and
     * converting each to wchar_t in src.
     *
     * The vis conversion will be done using the wide char
     * wchar_t string.
     *
     * This will then be converted back to a multibyte string to
     * return to the caller.
     */

    /* Allocate space for the wide char strings */
    psrc = pdst = extra = NULL;
    mdst = NULL;
    if((psrc = calloc(mbslength + 1, sizeof(*psrc))) == NULL) {
        return -1;
    }//end if
    if((pdst = calloc((16 * mbslength) + 1, sizeof(*pdst))) == NULL) {
        goto out;
    }//end if
    if(*mbdstp == NULL) {
        if((mdst = calloc((16 * mbslength) + 1, sizeof(*mdst))) == NULL) {
            goto out;
        }//end if
        *mbdstp = mdst;
    }//end if

    mbdst = *mbdstp;
    dst = pdst;
    src = psrc;

    if(flags & X_VIS_NOLOCALE) {
        /* Do one byte at a time conversion */
        cerr = 1;
    }//end if
    else {
        /* Use caller's multibyte conversion error flag. */
        cerr = cerr_ptr ? *cerr_ptr : 0;
    }//end else

#define X_MIN(a, b) ((a) < (b) ? (a) : (b))
    /*
     * Input loop.
     * Handle up to mblength characters (not bytes).  We do not
     * stop at NULs because we may be processing a block of data
     * that includes NULs.
     */
    memset(&mbstate, 0,sizeof(mbstate));
    while(mbslength > 0) {
        /* Convert one multibyte character to wchar_t. */
        if(!cerr) {
            clen = mbrtowc(src, mbsrc, X_MIN(mbslength, MB_LEN_MAX), &mbstate);
        }//end if
        if(cerr || clen < 0) {
            /* Conversion error, process as a byte instead. */
            *src = (wint_t)(u_char)*mbsrc;
            clen = 1;
            cerr = 1;
        }//end if
        if(clen == 0) {
            /*
             * NUL in input gives 0 return value. process
             * as single NUL byte and keep going.
             */
            clen = 1;
        }//end if
        /* Advance buffer character pointer. */
        src++;
        /* Advance input pointer by number of bytes read. */
        mbsrc += clen;
        /* Decrement input byte count. */
        mbslength -= clen;
    }//end while
    len = src - psrc;
    src = psrc;

    /*
     * In the single character input case, we will have actually
     * processed two characters, c and nextc.  Reset len back to
     * just a single character.
     */
    if(mblength < len) {
        len = mblength;
    }//end if

    /* Convert extra argument to list of characters for this mode. */
    extra = x_makeextralist(flags, mbextra);
    if(!extra) {
        if(dlen && *dlen == 0) {
            errno = ENOSPC;
            goto out;
        }//end if
        *mbdst = '\0';    /* can't create extra, return "" */
        error = 0;
        goto out;
    }//end if

    /* Look up which processing function to call. */
    f = x_getvisfun(flags);

    /*
     * Main processing loop.
     * Call do_Xvis processing function one character at a time
     * with next character available for look-ahead.
     */
    for(start = dst; len > 0; len--) {
        c = *src++;
        dst = (*f)(dst, c, flags, len >= 1 ? *src : L'\0', extra);
        if(dst == NULL) {
            errno = ENOSPC;
            goto out;
        }//end if
    }//end for

    /* Terminate the string in the buffer. */
    *dst = L'\0';

    /*
     * Output loop.
     * Convert wchar_t string back to multibyte output string.
     * If we have hit a multi-byte conversion error on input,
     * output byte-by-byte here.  Else use wctomb().
     */
    len = wcslen(start);
    maxolen = dlen ? *dlen : (wcslen(start) * MB_LEN_MAX + 1);
    olen = 0;
    memset(&mbstate, 0, sizeof(mbstate));
    for(dst = start; len > 0; len--) {
        if(!cerr) {
            clen = wcrtomb(mbdst, *dst, &mbstate);
        }//end if
        if(cerr || clen < 0) {
            /*
             * Conversion error, process as a byte(s) instead.
             * Examine each byte and higher-order bytes for
             * data.  E.g.,
             *    0x000000000000a264 -> a2 64
             *    0x000000001f00a264 -> 1f 00 a2 64
             */
            clen = 0;
            wmsk = 0;
            for(i = sizeof(wmsk) - 1; i >= 0; i--) {
                shft = i * NBBY;
                bmsk = (uint64_t)0xffLL << shft;
                wmsk |= bmsk;
                if((*dst & wmsk) || i == 0)
                    mbdst[clen++] = (char)((uint64_t)(*dst & bmsk) >> shft);
            }//end for
            cerr = 1;
        }//end if
        /* If this character would exceed our output limit, stop. */
        if(olen + clen > (size_t)maxolen) {
            break;
        }//end if
        /* Advance output pointer by number of bytes written. */
        mbdst += clen;
        /* Advance buffer character pointer. */
        dst++;
        /* Incrment output character count. */
        olen += clen;
    }//end for

    /* Terminate the output string. */
    *mbdst = '\0';

    if(flags & X_VIS_NOLOCALE) {
        /* Pass conversion error flag out. */
        if(cerr_ptr) {
            *cerr_ptr = cerr;
        }//end if
    }//end if

    free(extra);
    free(pdst);
    free(psrc);

    return (int)olen;
out:
    free(extra);
    free(pdst);
    free(psrc);
    free(mdst);
    return error;
}//end x_istrsenvisx

static int x_istrsenvisxl(char **mbdstp, size_t *dlen, const char *mbsrc, int flags, const char *mbextra, int *cerr_ptr) {
    return x_istrsenvisx(mbdstp, dlen, mbsrc, mbsrc != NULL ? strlen(mbsrc) : 0, flags, mbextra, cerr_ptr);
}//end x_istrsenvisxl

/*
 *    The "x_svis" variants all take an "extra" arg that is a pointer
 *    to a NUL-terminated list of characters to be encoded, too.
 *    These functions are useful e. g. to encode strings in such a
 *    way so that they are not interpreted by a shell.
 */

char *x_svis(char *mbdst, int c, int flags, int nextc, const char *mbextra) {
    int     ret;
    char    cc[2];

    cc[0] = c;
    cc[1] = nextc;

    ret = x_istrsenvisx(&mbdst, NULL, cc, 1, flags, mbextra, NULL);
    if(ret < 0) {
        return NULL;
    }//end if

    return mbdst + ret;
}//end x_svis

char *x_snvis(char *mbdst, size_t dlen, int c, int flags, int nextc, const char *mbextra) {
    int     ret;
    char    cc[2];

    cc[0] = c;
    cc[1] = nextc;

    ret = x_istrsenvisx(&mbdst, &dlen, cc, 1, flags, mbextra, NULL);
    if(ret < 0) {
        return NULL;
    }//end if

    return mbdst + ret;
}//end x_snvis

int x_strsvis(char *mbdst, const char *mbsrc, int flags, const char *mbextra) {
    return x_istrsenvisxl(&mbdst, NULL, mbsrc, flags, mbextra, NULL);
}//end x_strsvis

int x_strsnvis(char *mbdst, size_t dlen, const char *mbsrc, int flags, const char *mbextra) {
    return x_istrsenvisxl(&mbdst, &dlen, mbsrc, flags, mbextra, NULL);
}//end x_strsnvis

int x_strsvisx(char *mbdst, const char *mbsrc, size_t len, int flags, const char *mbextra) {
    return x_istrsenvisx(&mbdst, NULL, mbsrc, len, flags, mbextra, NULL);
}//end x_strsvisx

int x_strsnvisx(char *mbdst, size_t dlen, const char *mbsrc, size_t len, int flags, const char *mbextra) {
    return x_istrsenvisx(&mbdst, &dlen, mbsrc, len, flags, mbextra, NULL);
}//end x_strsnvisx

int x_strsenvisx(char *mbdst, size_t dlen, const char *mbsrc, size_t len, int flags, const char *mbextra, int *cerr_ptr) {
    return x_istrsenvisx(&mbdst, &dlen, mbsrc, len, flags, mbextra, cerr_ptr);
}//end x_strsenvisx

/*
 * vis - visually encode characters
 */
char *x_vis(char *mbdst, int c, int flags, int nextc) {
    int     ret;
    char    cc[2];

    cc[0] = c;
    cc[1] = nextc;

    ret = x_istrsenvisx(&mbdst, NULL, cc, 1, flags, "", NULL);
    if(ret < 0) {
        return NULL;
    }//end if

    return mbdst + ret;
}//end x_vis

char *x_nvis(char *mbdst, size_t dlen, int c, int flags, int nextc) {
    int     ret;
    char    cc[2];

    cc[0] = c;
    cc[1] = nextc;

    ret = x_istrsenvisx(&mbdst, &dlen, cc, 1, flags, "", NULL);
    if(ret < 0) {
        return NULL;
    }//end if

    return mbdst + ret;
}//end x_nvis

/*
 * strvis - visually encode characters from src into dst
 *
 *    Dst must be 4 times the size of src to account for possible
 *    expansion.  The length of dst, not including the trailing NULL,
 *    is returned.
 */

int x_strvis(char *mbdst, const char *mbsrc, int flags) {
    return x_istrsenvisxl(&mbdst, NULL, mbsrc, flags, "", NULL);
}//end x_strvis

int x_strnvis(char *mbdst, size_t dlen, const char *mbsrc, int flags) {
    return x_istrsenvisxl(&mbdst, &dlen, mbsrc, flags, "", NULL);
}//end x_strnvis

int x_stravis(char **mbdstp, const char *mbsrc, int flags) {
    *mbdstp = NULL;
    return x_istrsenvisxl(mbdstp, NULL, mbsrc, flags, "", NULL);
}//end x_stravis

/*
 * x_strvisx - visually encode characters from src into dst
 *
 *    Dst must be 4 times the size of src to account for possible
 *    expansion.  The length of dst, not including the trailing NULL,
 *    is returned.
 *
 *    Strvisx encodes exactly len characters from src into dst.
 *    This is useful for encoding a block of data.
 */

int x_strvisx(char *mbdst, const char *mbsrc, size_t len, int flags) {
    return x_istrsenvisx(&mbdst, NULL, mbsrc, len, flags, "", NULL);
}//end x_strvisx

int x_strnvisx(char *mbdst, size_t dlen, const char *mbsrc, size_t len, int flags) {
    return x_istrsenvisx(&mbdst, &dlen, mbsrc, len, flags, "", NULL);
}//end x_strnvisx

int x_strenvisx(char *mbdst, size_t dlen, const char *mbsrc, size_t len, int flags, int *cerr_ptr) {
    return x_istrsenvisx(&mbdst, &dlen, mbsrc, len, flags, "", cerr_ptr);
}//end x_strenvisx
