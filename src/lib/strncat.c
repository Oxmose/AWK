/*******************************************************************************
 *
 * File: strncat.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strncat function. To be used with string.h header.
 *
 ******************************************************************************/

#include "stddef.h" /* size_t */

/* Header file */
#include "string.h"

char *strncat(char *dst, const char *src, size_t n)
{
    char *q = strchr(dst, '\0');
    const char *p = src;
    char ch;

    while (n--) {
        *q++ = ch = *p++;
        if (!ch)
            return dst;
    }
    *q = '\0';

    return dst;
}
