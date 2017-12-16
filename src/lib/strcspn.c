/*******************************************************************************
 *
 * File: strcspn.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strcspn function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include "string.h"

size_t strcspn(const char *s, const char *reject)
{
	return __strxspn(s, reject, 1);
}
