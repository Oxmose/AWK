/*******************************************************************************
 *
 * File: strsep.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strsep function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include "string.h"

char *strsep(char **stringp, const char *delim)
{
    char *s = *stringp;
    char *e;

    if (!s)
        return NULL;

    e = strpbrk(s, delim);
    if (e)
        *e++ = '\0';

    *stringp = e;
    return s;
}
