/*******************************************************************************
 *
 * File: strpbrk.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strpbrk function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include "string.h"

char *strpbrk(const char *s, const char *accept)
{
    const char *ss = s + __strxspn(s, accept, 1);

    return *ss ? (char *)ss : NULL;
}
