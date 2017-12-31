/*******************************************************************************
 *
 * File: strxspn.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * __strxspn function. To be used with string.h header.
 *
 ******************************************************************************/

#include "stddef.h" /* size_t */

/* Header file */
#include "string.h"

#define UCHAR_MAX 255U

size_t __strxspn(const char *s, const char *map, int parity)
{
    char matchmap[UCHAR_MAX + 1];
    size_t n = 0;

    /* Create bitmap */
    memset(matchmap, 0, sizeof matchmap);
    while (*map)
        matchmap[(unsigned char)*map++] = 1;

    /* Make sure the null character never matches */
    matchmap[0] = parity;

    /* Calculate span length */
    while (matchmap[(unsigned char)*s++] ^ parity)
        n++;

    return n;
}
