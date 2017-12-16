/*******************************************************************************
 *
 * File: strnlen.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strnlen function. To be used with string.h header.
 *
 ******************************************************************************/

#include "stddef.h" /* size_t */

/* Header file */
#include "string.h"

size_t strnlen(const char *s, size_t maxlen)
{
	const char *ss = s;

	/* Important: the maxlen test must precede the reference through ss;
	   since the byte beyond the maximum may segfault */
	while ((maxlen > 0) && *ss) {
		ss++;
		maxlen--;
	}
	return ss - s;
}
