/*******************************************************************************
 *
 * File: memswap.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * memswap function. To be used with string.h header.
 *
 ******************************************************************************/

#include "stddef.h" /* size_t */

/* Header file */
#include "string.h"

void memswap(void *m1, void *m2, size_t n)
{
    char *p = m1;
    char *q = m2;
    char tmp;

    while (n--) {
        tmp = *p;
        *p = *q;
        *q = tmp;

        p++;
        q++;
    }
}
