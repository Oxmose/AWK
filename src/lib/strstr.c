/*******************************************************************************
 *
 * File: strstr.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strstr function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include "string.h"

char *strstr(const char *haystack, const char *needle)
{
    return (char *)memmem(haystack, strlen(haystack), needle,
                  strlen(needle));
}
