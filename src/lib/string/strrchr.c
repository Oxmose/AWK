/*******************************************************************************
 *
 * File: strrchr.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strrchr function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include "../string.h"

char *strrchr(const char *s, int c)
{
    const char *found = NULL;

    while (*s) {
        if (*s == (char)c)
            found = s;
        s++;
    }

    return (char *)found;
}
