/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/gen/FreeBSD/unvis.c
 *
 */


#include <stdint.h>
#include <ctype.h>
#include <errno.h>

#include "x_vis.h"

/*
 * Return the number of elements in a statically-allocated array,
 * __x.
 */
#define x___arraycount(__x) (sizeof(__x) / sizeof((__x)[0]))

/*
 * decode driven by state machine
 */
#define X_S_GROUND  0   /* haven't seen escape char */
#define X_S_START   1   /* start decoding special sequence */
#define X_S_META    2   /* metachar started (M) */
#define X_S_META1   3   /* metachar more, regular char (-) */
#define X_S_CTRL    4   /* control char started (^) */
#define X_S_OCTAL2  5   /* octal digit 2 */
#define X_S_OCTAL3  6   /* octal digit 3 */
#define X_S_HEX     7   /* mandatory hex digit */
#define X_S_HEX1    8   /* http hex digit */
#define X_S_HEX2    9   /* http hex digit 2 */
#define X_S_MIME1   10  /* mime hex digit 1 */
#define X_S_MIME2   11  /* mime hex digit 2 */
#define X_S_EATCRNL 12  /* mime eating CRNL */
#define X_S_AMP     13  /* seen & */
#define X_S_NUMBER  14  /* collecting number */
#define X_S_STRING  15  /* collecting string */

#define x_isoctal(c)    (((u_char)(c)) >= '0' && ((u_char)(c)) <= '7')
#define x_xtod(c)       (isdigit(c) ? (c - '0') : ((tolower(c) - 'a') + 10))
#define X_XTOD(c)       (isdigit(c) ? (c - '0') : ((c - 'A') + 10))

/*
 * RFC 1866
 */
static const struct x_nv {
    char    name[7];
    uint8_t value;
} x_nv[] = {
    { "AElig",  198 }, /* capital AE diphthong (ligature)  */
    { "Aacute", 193 }, /* capital A, acute accent  */
    { "Acirc",  194 }, /* capital A, circumflex accent  */
    { "Agrave", 192 }, /* capital A, grave accent  */
    { "Aring",  197 }, /* capital A, ring  */
    { "Atilde", 195 }, /* capital A, tilde  */
    { "Auml",   196 }, /* capital A, dieresis or umlaut mark  */
    { "Ccedil", 199 }, /* capital C, cedilla  */
    { "ETH",    208 }, /* capital Eth, Icelandic  */
    { "Eacute", 201 }, /* capital E, acute accent  */
    { "Ecirc",  202 }, /* capital E, circumflex accent  */
    { "Egrave", 200 }, /* capital E, grave accent  */
    { "Euml",   203 }, /* capital E, dieresis or umlaut mark  */
    { "Iacute", 205 }, /* capital I, acute accent  */
    { "Icirc",  206 }, /* capital I, circumflex accent  */
    { "Igrave", 204 }, /* capital I, grave accent  */
    { "Iuml",   207 }, /* capital I, dieresis or umlaut mark  */
    { "Ntilde", 209 }, /* capital N, tilde  */
    { "Oacute", 211 }, /* capital O, acute accent  */
    { "Ocirc",  212 }, /* capital O, circumflex accent  */
    { "Ograve", 210 }, /* capital O, grave accent  */
    { "Oslash", 216 }, /* capital O, slash  */
    { "Otilde", 213 }, /* capital O, tilde  */
    { "Ouml",   214 }, /* capital O, dieresis or umlaut mark  */
    { "THORN",  222 }, /* capital THORN, Icelandic  */
    { "Uacute", 218 }, /* capital U, acute accent  */
    { "Ucirc",  219 }, /* capital U, circumflex accent  */
    { "Ugrave", 217 }, /* capital U, grave accent  */
    { "Uuml",   220 }, /* capital U, dieresis or umlaut mark  */
    { "Yacute", 221 }, /* capital Y, acute accent  */
    { "aacute", 225 }, /* small a, acute accent  */
    { "acirc",  226 }, /* small a, circumflex accent  */
    { "acute",  180 }, /* acute accent  */
    { "aelig",  230 }, /* small ae diphthong (ligature)  */
    { "agrave", 224 }, /* small a, grave accent  */
    { "amp",     38 }, /* ampersand  */
    { "aring",  229 }, /* small a, ring  */
    { "atilde", 227 }, /* small a, tilde  */
    { "auml",   228 }, /* small a, dieresis or umlaut mark  */
    { "brvbar", 166 }, /* broken (vertical) bar  */
    { "ccedil", 231 }, /* small c, cedilla  */
    { "cedil",  184 }, /* cedilla  */
    { "cent",   162 }, /* cent sign  */
    { "copy",   169 }, /* copyright sign  */
    { "curren", 164 }, /* general currency sign  */
    { "deg",    176 }, /* degree sign  */
    { "divide", 247 }, /* divide sign  */
    { "eacute", 233 }, /* small e, acute accent  */
    { "ecirc",  234 }, /* small e, circumflex accent  */
    { "egrave", 232 }, /* small e, grave accent  */
    { "eth",    240 }, /* small eth, Icelandic  */
    { "euml",   235 }, /* small e, dieresis or umlaut mark  */
    { "frac12", 189 }, /* fraction one-half  */
    { "frac14", 188 }, /* fraction one-quarter  */
    { "frac34", 190 }, /* fraction three-quarters  */
    { "gt",      62 }, /* greater than  */
    { "iacute", 237 }, /* small i, acute accent  */
    { "icirc",  238 }, /* small i, circumflex accent  */
    { "iexcl",  161 }, /* inverted exclamation mark  */
    { "igrave", 236 }, /* small i, grave accent  */
    { "iquest", 191 }, /* inverted question mark  */
    { "iuml",   239 }, /* small i, dieresis or umlaut mark  */
    { "laquo",  171 }, /* angle quotation mark, left  */
    { "lt",      60 }, /* less than  */
    { "macr",   175 }, /* macron  */
    { "micro",  181 }, /* micro sign  */
    { "middot", 183 }, /* middle dot  */
    { "nbsp",   160 }, /* no-break space  */
    { "not",    172 }, /* not sign  */
    { "ntilde", 241 }, /* small n, tilde  */
    { "oacute", 243 }, /* small o, acute accent  */
    { "ocirc",  244 }, /* small o, circumflex accent  */
    { "ograve", 242 }, /* small o, grave accent  */
    { "ordf",   170 }, /* ordinal indicator, feminine  */
    { "ordm",   186 }, /* ordinal indicator, masculine  */
    { "oslash", 248 }, /* small o, slash  */
    { "otilde", 245 }, /* small o, tilde  */
    { "ouml",   246 }, /* small o, dieresis or umlaut mark  */
    { "para",   182 }, /* pilcrow (paragraph sign)  */
    { "plusmn", 177 }, /* plus-or-minus sign  */
    { "pound",  163 }, /* pound sterling sign  */
    { "quot",    34 }, /* double quote  */
    { "raquo",  187 }, /* angle quotation mark, right  */
    { "reg",    174 }, /* registered sign  */
    { "sect",   167 }, /* section sign  */
    { "shy",    173 }, /* soft hyphen  */
    { "sup1",   185 }, /* superscript one  */
    { "sup2",   178 }, /* superscript two  */
    { "sup3",   179 }, /* superscript three  */
    { "szlig",  223 }, /* small sharp s, German (sz ligature)  */
    { "thorn",  254 }, /* small thorn, Icelandic  */
    { "times",  215 }, /* multiply sign  */
    { "uacute", 250 }, /* small u, acute accent  */
    { "ucirc",  251 }, /* small u, circumflex accent  */
    { "ugrave", 249 }, /* small u, grave accent  */
    { "uml",    168 }, /* umlaut (dieresis)  */
    { "uuml",   252 }, /* small u, dieresis or umlaut mark  */
    { "yacute", 253 }, /* small y, acute accent  */
    { "yen",    165 }, /* yen sign  */
    { "yuml",   255 }, /* small y, dieresis or umlaut mark  */
};

/*
 * x_unvis - decode characters previously encoded by vis
 */
int x_unvis(char *cp, int c, int *astate, int flag) {
    unsigned char uc, st, ia, is, lc;

    uc = (unsigned char)c;

/*
 * Bottom 8 bits of astate hold the state machine state.
 * Top 8 bits hold the current character in the http 1866 nv string decoding
 */
#define X_GS(a)     ((a) & 0xff)
#define X_SS(a, b)  (((uint32_t)(a) << 24) | (b))
#define X_GI(a)     ((uint32_t)(a) >> 24)

    st = X_GS(*astate);

    if(flag & X_UNVIS_END) {
        switch(st) {
        case X_S_OCTAL2:
        case X_S_OCTAL3:
        case X_S_HEX2:
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case X_S_GROUND:
            return X_UNVIS_NOCHAR;
        default:
            return X_UNVIS_SYNBAD;
        }//end switch
    }//end if

    switch(st) {

    case X_S_GROUND:
        *cp = 0;
        if((flag & X_VIS_NOESCAPE) == 0 && c == '\\') {
            *astate = X_SS(0, X_S_START);
            return X_UNVIS_NOCHAR;
        }//end if
        if((flag & X_VIS_HTTP1808) && c == '%') {
            *astate = X_SS(0, X_S_HEX1);
            return X_UNVIS_NOCHAR;
        }//end if
        if((flag & X_VIS_HTTP1866) && c == '&') {
            *astate = X_SS(0, X_S_AMP);
            return X_UNVIS_NOCHAR;
        }//end if
        if((flag & X_VIS_MIMESTYLE) && c == '=') {
            *astate = X_SS(0, X_S_MIME1);
            return X_UNVIS_NOCHAR;
        }//end if
        *cp = c;
        return X_UNVIS_VALID;

    case X_S_START:
        switch(c) {
        case '\\':
            *cp = c;
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
            *cp = (c - '0');
            *astate = X_SS(0, X_S_OCTAL2);
            return X_UNVIS_NOCHAR;
        case 'M':
            *cp = (char)0200;
            *astate = X_SS(0, X_S_META);
            return X_UNVIS_NOCHAR;
        case '^':
            *astate = X_SS(0, X_S_CTRL);
            return X_UNVIS_NOCHAR;
        case 'n':
            *cp = '\n';
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case 'r':
            *cp = '\r';
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case 'b':
            *cp = '\b';
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case 'a':
            *cp = '\007';
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case 'v':
            *cp = '\v';
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case 't':
            *cp = '\t';
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case 'f':
            *cp = '\f';
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case 's':
            *cp = ' ';
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case 'E':
            *cp = '\033';
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        case 'x':
            *astate = X_SS(0, X_S_HEX);
            return X_UNVIS_NOCHAR;
        case '\n':
            /*
             * hidden newline
             */
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_NOCHAR;
        case '$':
            /*
             * hidden marker
             */
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_NOCHAR;
        default:
            if(isgraph(c)) {
                *cp = c;
                *astate = X_SS(0, X_S_GROUND);
                return X_UNVIS_VALID;
            }//end if
        }//end switch
        goto bad;

    case X_S_META:
        if(c == '-') {
            *astate = X_SS(0, X_S_META1);
        }//end if
        else if(c == '^') {
            *astate = X_SS(0, X_S_CTRL);
        }//end if
        else {
            goto bad;
        }//end else
        return X_UNVIS_NOCHAR;

    case X_S_META1:
        *astate = X_SS(0, X_S_GROUND);
        *cp |= c;
        return X_UNVIS_VALID;

    case X_S_CTRL:
        if(c == '?') {
            *cp |= 0177;
        }//end if
        else {
            *cp |= c & 037;
        }//end else
        *astate = X_SS(0, X_S_GROUND);
        return X_UNVIS_VALID;

    case X_S_OCTAL2:    /* second possible octal digit */
        if(x_isoctal(uc)) {
            /*
             * yes - and maybe a third
             */
            *cp = (*cp << 3) + (c - '0');
            *astate = X_SS(0, X_S_OCTAL3);
            return X_UNVIS_NOCHAR;
        }//end if
        /*
         * no - done with current sequence, push back passed char
         */
        *astate = X_SS(0, X_S_GROUND);
        return X_UNVIS_VALIDPUSH;

    case X_S_OCTAL3:    /* third possible octal digit */
        *astate = X_SS(0, X_S_GROUND);
        if(x_isoctal(uc)) {
            *cp = (*cp << 3) + (c - '0');
            return X_UNVIS_VALID;
        }//end if
        /*
         * we were done, push back passed char
         */
        return X_UNVIS_VALIDPUSH;

    case X_S_HEX:
        if(!isxdigit(uc)) {
            goto bad;
        }//end if
        /*FALLTHROUGH*/
    case X_S_HEX1:
        if(isxdigit(uc)) {
            *cp = x_xtod(uc);
            *astate = X_SS(0, X_S_HEX2);
            return X_UNVIS_NOCHAR;
        }//end if
        /*
         * no - done with current sequence, push back passed char
         */
        *astate = X_SS(0, X_S_GROUND);
        return X_UNVIS_VALIDPUSH;

    case X_S_HEX2:
        *astate = X_S_GROUND;
        if(isxdigit(uc)) {
            *cp = x_xtod(uc) | (*cp << 4);
            return X_UNVIS_VALID;
        }//end if
        return X_UNVIS_VALIDPUSH;

    case X_S_MIME1:
        if(uc == '\n' || uc == '\r') {
            *astate = X_SS(0, X_S_EATCRNL);
            return X_UNVIS_NOCHAR;
        }//end if
        if(isxdigit(uc) && (isdigit(uc) || isupper(uc))) {
            *cp = X_XTOD(uc);
            *astate = X_SS(0, X_S_MIME2);
            return X_UNVIS_NOCHAR;
        }//end if
        goto bad;

    case X_S_MIME2:
        if(isxdigit(uc) && (isdigit(uc) || isupper(uc))) {
            *astate = X_SS(0, X_S_GROUND);
            *cp = X_XTOD(uc) | (*cp << 4);
            return X_UNVIS_VALID;
        }//end if
        goto bad;

    case X_S_EATCRNL:
        switch(uc) {
        case '\r':
        case '\n':
            return X_UNVIS_NOCHAR;
        case '=':
            *astate = X_SS(0, X_S_MIME1);
            return X_UNVIS_NOCHAR;
        default:
            *cp = uc;
            *astate = X_SS(0, X_S_GROUND);
            return X_UNVIS_VALID;
        }//end switch

    case X_S_AMP:
        *cp = 0;
        if(uc == '#') {
            *astate = X_SS(0, X_S_NUMBER);
            return X_UNVIS_NOCHAR;
        }//end if
        *astate = X_SS(0, X_S_STRING);
        /* fallthrough */

    case X_S_STRING:
        ia = *cp;           /* index in the array */
        is = X_GI(*astate); /* index in the string */
        lc = is == 0 ? 0 : x_nv[ia].name[is - 1]; /* last character */

        if(uc == ';') {
            uc = '\0';
        }//end if

        for(; ia < x___arraycount(x_nv); ia++) {
            if(is != 0 && x_nv[ia].name[is - 1] != lc) {
                goto bad;
            }//end if
            if(x_nv[ia].name[is] == uc) {
                break;
            }//end if
        }//end for

        if(ia == x___arraycount(x_nv)) {
            goto bad;
        }//end if

        if(uc != 0) {
            *cp = ia;
            *astate = X_SS(is + 1, X_S_STRING);
            return X_UNVIS_NOCHAR;
        }//end if

        *cp = x_nv[ia].value;
        *astate = X_SS(0, X_S_GROUND);
        return X_UNVIS_VALID;

    case X_S_NUMBER:
        if(uc == ';') {
            return X_UNVIS_VALID;
        }//end if
        if(!isdigit(uc)) {
            goto bad;
        }//end if
        *cp += (*cp * 10) + uc - '0';
        return X_UNVIS_NOCHAR;

    default:
bad:
        /*
         * decoder in unknown state - (probably uninitialized)
         */
        *astate = X_SS(0, X_S_GROUND);
        return X_UNVIS_SYNBAD;
    }//end switch
}//end x_unvis

/*
 * x_strnunvisx - decode src into dst
 *
 *    Number of chars decoded into dst is returned, -1 on error.
 *    Dst is null terminated.
 */

int x_strnunvisx(char *dst, size_t dlen, const char *src, int flag) {
    int     state;
    char    c, t, *start;

    t = '\0';
    start = dst;
    state = 0;

#define X_CHECKSPACE()      \
    do {                    \
        if(dlen-- == 0) {   \
            errno = ENOSPC; \
            return -1;      \
        }                   \
    } while(/*CONSTCOND*/0)

    while((c = *src++) != '\0') {
again:
        switch(x_unvis(&t, c, &state, flag)) {
        case X_UNVIS_VALID:
            X_CHECKSPACE();
            *dst++ = t;
            break;
        case X_UNVIS_VALIDPUSH:
            X_CHECKSPACE();
            *dst++ = t;
            goto again;
        case 0:
        case X_UNVIS_NOCHAR:
            break;
        case X_UNVIS_SYNBAD:
            errno = EINVAL;
            return -1;
        default:
            errno = EINVAL;
            return -1;
        }//end switch
    }//end while

    if(x_unvis(&t, c, &state, X_UNVIS_END) == X_UNVIS_VALID) {
        X_CHECKSPACE();
        *dst++ = t;
    }//end if

    X_CHECKSPACE();
    *dst = '\0';
    return (int)(dst - start);
}//end x_strnunvisx

int x_strunvisx(char *dst, const char *src, int flag) {
    return x_strnunvisx(dst, (size_t)~0, src, flag);
}//end x_strunvisx

int x_strunvis(char *dst, const char *src) {
    return x_strnunvisx(dst, (size_t)~0, src, 0);
}//end x_strunvis

int x_strnunvis(char *dst, size_t dlen, const char *src) {
    return x_strnunvisx(dst, dlen, src, 0);
}//end x_strnunvis
