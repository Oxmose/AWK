/*******************************************************************************
 *
 * File: stdio.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/10/2017
 *
 * Version: 1.0
 *
 * Standard input output file header
 *
 ******************************************************************************/

#ifndef __STDIO_H_
#define __STDIO_H_


#include "stddef.h" /* size_t */

int perror(const int error);

int printf(const char *__format, ...) __attribute__((format (printf, 1, 2)));

int vprintf(const char *__format, __builtin_va_list __vl) __attribute__((format (printf, 1, 0)));

int sprintf(char *__dest, const char *__format, ...) __attribute__((format (printf, 2, 3)));

int snprintf(char *__dest, unsigned int __size, const char *__format, ...) __attribute__((format (printf, 3, 4)));

int vsprintf(char *__dest, const char *__format, __builtin_va_list __vl) __attribute__((format (printf, 2, 0)));

int vsnprintf(char *__dest, unsigned int __size, const char *__format, __builtin_va_list __vl) __attribute__((format (printf, 3, 0)));

#endif /* __STDIO_H_ */