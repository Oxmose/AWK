/*******************************************************************************
 *
 * File: memset.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * memset function. To be used with string.h header.
 *
 ******************************************************************************/

#include "stddef.h" /* size_t */

/* Header file */
#include "string.h"

void *memset(void *dst, int c, size_t n)
{
    char *q = dst;

#if defined(__i386__)
    size_t nl = n >> 2;
    __asm__ __volatile__ ("cld ; rep ; stosl ; movl %3,%0 ; rep ; stosb"
              : "+c" (nl), "+D" (q)
              : "a" ((unsigned char)c * 0x01010101U), "r" (n & 3));
#elif defined(__x86_64__)
    size_t nq = n >> 3;
    __asm__ __volatile__ ("cld ; rep ; stosq ; movl %3,%%ecx ; rep ; stosb"
              :"+c" (nq), "+D" (q)
              : "a" ((unsigned char)c * 0x0101010101010101U),
            "r" ((unsigned int) n & 7));
#else
    while (n--) {
        *q++ = c;
    }
#endif

    return dst;
}