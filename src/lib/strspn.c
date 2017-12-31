/*******************************************************************************
 *
 * File: strspn.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strspn function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include "string.h"

size_t strspn(const char *s, const char *accept)
{
    return __strxspn(s, accept, 0);
}
