/*******************************************************************************
 *
 * File: strtok.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strtok function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include "../string.h"

char *strtok(char *s, const char *delim)
{
    static char *holder;

    if (s)
        holder = s;

    return strsep(&holder, delim);
}
