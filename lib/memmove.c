/*******************************************************************************
 *
 * File: memmove.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * memmove function. To be used with string.h header.
 *
 ******************************************************************************/

/* size_t */
#include "stddef.h"

/* int8_t */
#include "stdint.h"

/* Header file */
#include "string.h"

void *memmove(void *dst, const void *src, size_t n)
{
    const int8_t *p = src;
    int8_t *q = dst;
#if defined(__i386__) || defined(__x86_64__)
    if (q < p) {
        __asm__ __volatile__("cld ; rep ; movsb"
                 : "+c" (n), "+S"(p), "+D"(q));
    } else {
        p += (n - 1);
        q += (n - 1);
        __asm__ __volatile__("std ; rep ; movsb"
                 : "+c" (n), "+S"(p), "+D"(q));
    }
#else
    if (q < p) {
        while (n--) {
            *q++ = *p++;
        }
    } else {
        p += n;
        q += n;
        while (n--) {
            *--q = *--p;
        }
    }
#endif
    return dst;
}