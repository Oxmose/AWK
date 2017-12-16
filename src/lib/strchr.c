/*******************************************************************************
 *
 * File: strchr.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strchr function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include "string.h"

char *strchr(const char *s, int c)
{
	while (*s != (char)c) {
		if (!*s)
			return NULL;
		s++;
	}

	return (char *)s;
}
