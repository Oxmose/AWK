/*******************************************************************************
 *
 * File: strlen.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strlen function. To be used with string.h header.
 *
 ******************************************************************************/

#include "stddef.h" /* size_t */

/* Header file */
#include "string.h"

size_t strlen(const char *s)
{
    const char *ss = s;
    while (*ss)
        ss++;
    return ss - s;
}
