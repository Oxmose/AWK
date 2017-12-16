/*******************************************************************************
 *
 * File: string.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/10/2017
 *
 * Version: 1.0
 *
 * Strings and memory manipulation functions.
 *
 ******************************************************************************/

#ifndef __STRING_H_
#define __STRING_H_

#define __extern extern

#define __alias(x) __attribute__((weak, alias(x)))

#include "stddef.h" /* size_t */
#include "stdint.h" /* int8_t, uint8_t, int32_t, uint32_t */

__extern void *memccpy(void *, const void *, int32_t, size_t);
__extern void *memchr(const void *, int32_t, size_t);
__extern void *memrchr(const void *, int32_t, size_t);
__extern int32_t memcmp(const void *, const void *, size_t);
__extern void *memcpy(void *, const void *, size_t);
__extern void *memmove(void *, const void *, size_t);
__extern void *memset(void *, int32_t, size_t);
__extern void *memmem(const void *, size_t, const void *, size_t);
__extern void memswap(void *, void *, size_t);
__extern int32_t strcasecmp(const int8_t *, const int8_t *);
__extern int32_t strncasecmp(const int8_t *, const int8_t *, size_t);
__extern int8_t *strcat(int8_t *, const int8_t *);
__extern int8_t *strchr(const int8_t *, int);
__extern int8_t *strrchr(const int8_t *, int);
__extern int32_t strcmp(const int8_t *, const int8_t *);
__extern int8_t *strcpy(int8_t *, const int8_t *);
__extern size_t strcspn(const int8_t *, const int8_t *);
__extern int8_t *strdup(const int8_t *);
__extern int8_t *strndup(const int8_t *, size_t);
__extern int8_t *strerror(int32_t);
__extern int8_t *strsignal(int32_t);
__extern size_t strlen(const int8_t *);
__extern size_t strnlen(const int8_t *, size_t);
__extern int8_t *strncat(int8_t *, const int8_t *, size_t);
__extern size_t strlcat(int8_t *, const int8_t *, size_t);
__extern int32_t strncmp(const int8_t *, const int8_t *, size_t);
__extern int8_t *strncpy(int8_t *, const int8_t *, size_t);
__extern size_t strlcpy(int8_t *, const int8_t *, size_t);
__extern int8_t *strpbrk(const int8_t *, const int8_t *);
__extern int8_t *strsep(int8_t **, const int8_t *);
__extern size_t strspn(const int8_t *, const int8_t *);
__extern int8_t *strstr(const int8_t *, const int8_t *);
__extern int8_t *strtok(int8_t *, const int8_t *);

#endif                /* __STRING_H_ */