/*******************************************************************************
 *
 * File: memcpy.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * memcpy function. To be used with string.h header.
 *
 ******************************************************************************/

#include "../stddef.h" /* size_t */

/* Header file */
#include "../string.h"

void *memcpy(void *dst, const void *src, size_t n)
{
    const char *p = src;
    char *q = dst;
#if defined(__i386__)
    size_t nl = n >> 2;
    __asm__ __volatile__ ("cld ; rep ; movsl ; movl %3,%0 ; rep ; movsb":"+c" (nl),
              "+S"(p), "+D"(q)
              :"r"(n & 3));
#elif defined(__x86_64__)
    size_t nq = n >> 3;
    __asm__ __volatile__ ("cld ; rep ; movsq ; movl %3,%%ecx ; rep ; movsb":"+c"
              (nq), "+S"(p), "+D"(q)
              :"r"((uint32_t) (n & 7)));
#else
    while (n--) {
        *q++ = *p++;
    }
#endif

    return dst;
}
