/*******************************************************************************
 *
 * File: sprintf.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * sprintf function and derivatives . To be used with stdio.h header.
 *
 ******************************************************************************/

 #include "../stddef.h" /* size_t */

 /* Header file */
 #include "../stdio.h"

#if 0
#define SPRINTF_UNLIMITED -1

struct sprintf_state
{
      char *buf;
      unsigned int len;
      unsigned int max;
};

static void savechar(char *arg, int c)
{
    struct sprintf_state *state = (struct sprintf_state *)arg;

    if (state->max != (unsigned int)SPRINTF_UNLIMITED)
    {
        if (state->len == state->max)
        {
            return;
        }
    }

    state->len++;
    *state->buf = c;
    state->buf++;
}

int vsprintf(char *s, const char *fmt, __builtin_va_list args)
{
    struct sprintf_state state;
    state.max = SPRINTF_UNLIMITED;
    state.len = 0;
    state.buf = s;

    _doprnt(fmt, args, 0, (void (*)()) savechar, (char *) &state);
    *(state.buf) = '\0';

    return state.len;
}

int vsnprintf(char *s, unsigned int size, const char *fmt, __builtin_va_list args)
{
    struct sprintf_state state;
    state.max = size;
    state.len = 0;
    state.buf = s;

    _doprnt(fmt, args, 0, (void (*)()) savechar, (char *) &state);
    *(state.buf) = '\0';

    return state.len;
}

int sprintf(char *s, const char *fmt, ...)
{
    __builtin_va_list    args;

    int err;

    __builtin_va_start(args, fmt);

    err = vsprintf(s, fmt, args);

    __builtin_va_end(args);

    return err;
}

int snprintf(char *s, unsigned int size, const char *fmt, ...)
{
    __builtin_va_list    args;

    int err;

    __builtin_va_start(args, fmt);

    err = vsnprintf(s, size, fmt, args);

    __builtin_va_end(args);

    return err;
}
#endif
