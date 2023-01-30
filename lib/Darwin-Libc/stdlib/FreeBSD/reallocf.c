/*
 * ORIGINAL LICENSE
 *
 * Original content: https://github.com/apple-oss-distributions/Libc/blob/Libc-1534.40.2/stdlib/FreeBSD/reallocf.c
 *
 */


#include <stdlib.h>

#include "x_stdlib.h"

void *x_reallocf(void *ptr, size_t size) {
    void *nptr;

    nptr = realloc(ptr, size);

    /*
     * When the System V compatibility option (malloc "V" flag) is
     * in effect, realloc(ptr, 0) frees the memory and returns NULL.
     * So, to avoid double free, call free() only when size != 0.
     * realloc(ptr, 0) can't fail when ptr != NULL.
     */
    if(!nptr && ptr && size != 0) {
        free(ptr);
    }//end if

    return nptr;
}//end x_realloc
