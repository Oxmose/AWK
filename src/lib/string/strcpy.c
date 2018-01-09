/*******************************************************************************
 *
 * File: strcpy.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strcpy function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include "../string.h"

char *strcpy(char *dst, const char *src)
{
    char *q = dst;
    const char *p = src;
    char ch;

    do {
        *q++ = ch = *p++;
    } while (ch);

    return dst;
}
