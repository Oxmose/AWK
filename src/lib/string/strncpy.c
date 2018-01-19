/*******************************************************************************
 *
 * File: strncpy.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strncpy function. To be used with string.h header.
 *
 ******************************************************************************/

#include "../stddef.h" /* size_t */

/* Header file */
#include "../string.h"

char *strncpy(char *dst, const char *src, size_t n)
{
    char *q = dst;
    const char *p = src;
    char ch;

    while (n) {
        n--;
        *q++ = ch = *p++;
        if (!ch)
            break;
    }

    /* The specs say strncpy() fills the entire buffer with NUL.  Sigh. */
    memset(q, 0, n);

    return dst;
}
