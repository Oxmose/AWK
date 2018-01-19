/*******************************************************************************
 *
 * File: memcmp.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * memcmp function. To be used with string.h header.
 *
 ******************************************************************************/

#include "../stddef.h" /* size_t */

/* Header file */
#include "../string.h"

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *c1 = s1, *c2 = s2;
    int d = 0;

    while (n--) {
        d = (int)*c1++ - (int)*c2++;
        if (d)
            break;
    }

    return d;
}
