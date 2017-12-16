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

#include "stddef.h" /* size_t */
#include "doprnt.h" /* doprnt */

/* Header file */
#include "stdio.h"

/* Screen driver must implement this function to output text to
 * the screen.
 */
extern void console_putbytes(const char *s, const uint32_t len);

#define PRINTF_BUFMAX 128

struct printf_state 
{
    char buf[PRINTF_BUFMAX];
    unsigned int index;
};

static void flush(struct printf_state *state)
{
    console_putbytes((const char *)state->buf, state->index);
    state->index = 0;
}

static void printf_char(char *arg, int c)
{
    struct printf_state *state = (struct printf_state *) arg;

    if ((c == 0) || (c == '\n') || (state->index >= PRINTF_BUFMAX))
    {
        flush(state);
        state->buf[0] = c;
        console_putbytes((const char *)state->buf, 1);
    }
    else
    {
        state->buf[state->index] = c;
        state->index++;
    }
}

int vprintf(const char *fmt, __builtin_va_list args)
{
    struct printf_state state;

    state.index = 0;
    _doprnt(fmt, args, 0, (void (*)())printf_char, (char *) &state);

    if (state.index != 0)
    {
        flush(&state);
    }
    return 0;
}    

int printf(const char *fmt, ...)
{
    __builtin_va_list    args;

    int err;

    __builtin_va_start(args, fmt);

    err = vprintf(fmt, args);

    __builtin_va_end(args);

    return err;
}

int putchar(int c)
{
    char ch = c;
    console_putbytes(&ch, 1);
    return (unsigned char)ch;
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

#undef PRINTF_BUFMAX