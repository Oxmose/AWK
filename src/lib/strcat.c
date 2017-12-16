/*******************************************************************************
 *
 * File: strcat.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strcat function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include "string.h"

char *strcat(char *dst, const char *src)
{
	strcpy(strchr(dst, '\0'), src);
	return dst;
}
