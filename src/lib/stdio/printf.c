/*******************************************************************************
 *
 * File: printf.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * printf function and derivatives . To be used with stdio.h header.
 *
 ******************************************************************************/

#include "../stddef.h" /* size_t */

/* Header file */
#include "../stdio.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

extern void kernel_doprint(const char* str, __builtin_va_list args);
extern void screen_put_char(const char c);

int vprintf(const char *fmt, __builtin_va_list args)
{
    kernel_doprint(fmt, args);

    return 0;
}

int printf(const char *fmt, ...)
{
    __builtin_va_list    args;
    int                  err;

    __builtin_va_start(args, fmt);

    err = vprintf(fmt, args);

    __builtin_va_end(args);

    return err;
}

int putchar(int c)
{
    screen_put_char((char)c);
    return (unsigned char)c;
}

int puts(const char *s)
{
    while (*s)
    {
        putchar(*s++);
    }
    putchar('\n');
    return 0;
}
