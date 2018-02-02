/*******************************************************************************
 *
 * File: keyboard.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 08/10/2017
 *
 * Version: 1.5
 *
 * Keyboard driver (PS2/USB) for the kernel.
 ******************************************************************************/

#include "graphic.h"               /* put_char */
#include "../cpu/cpu.h"            /* outb inb */
#include "../core/interrupts.h"    /* register_interrupt, cpu_state,
                                    * stack_state, set_IRQ_mask, set_IRQ_EOI */

#include "../lib/stdint.h"         /* Generic int types */
#include "../lib/stddef.h"         /* OS_RETURN_E, OS_EVENT_ID */
#include "../lib/string.h"         /* memcpy */
#include "../lib/stdio.h"          /* printf */
#include "../sync/semaphore.h"     /* semaphore_t */
#include "../sync/mutex.h"          /* mutex_t */

/* Header file */
#include "keyboard.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Keyboard security */
static volatile uint8_t secure_input;
static volatile uint8_t display_keyboard;

/* Shift key used */
static volatile uint32_t keyboard_flags;

/* Keyboard buffer */
static kbd_buffer_t kbd_buf;
static mutex_t      kbd_mutex;

/* Keyboard map */
static const key_mapper_t qwerty_map =
{
    .regular =
    {
        0,
        0,   // ESCAPE
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',                      // 10
        '0',
        '-',
        '=',
        KEY_BACKSPACE,   // BACKSPACE
        KEY_TAB,   // TAB
        'q',
        'w',
        'e',
        'r',
        't',                    // 20
        'y',
        'u',
        'i',
        'o',
        'p',
        0,   // MOD ^
        0,   // MOD ¸
        KEY_RETURN,   // ENTER
        0,   // VER MAJ
        'a',                    // 30
        's',
        'd',
        'f',
        'g',
        'h',
        'j',
        'k',
        'l',
        ';',
        0,   // MOD `           // 40
        0,   // TODO
        KEY_LSHIFT,   // LEFT SHIFT
        '<',
        'z',
        'x',
        'c',
        'v',
        'b',
        'n',
        'm',                    // 50
        ',',
        '.',
        ' ', // é
        KEY_RSHIFT,    // RIGHT SHIFT
        0,   // TODO
        0,     // ALT left / right
        ' ',
        ' ',
        0,     // F1
        0,     // F2               // 60
        0,     // F3
        0,     // F4
        0,     // F5
        0,     // F6
        0,     // F7
        0,     // F8
        0,     // F9
        0,     // SCROLL LOCK
        0,     // PAUSE
        0,                     // 70
        0,
        0,
        KEY_PGUP,
        0,
        0,
        0,
        0,
        0,
        0,
        0,                  // 80
        KEY_PGDOWN,
        0
    },

    .shifted =
    {
        0,
        0,   // ESCAPE
        '!',
        '"',
        '/',
        '$',
        '%',
        '?',
        '&',
        '*',
        '(',                      // 10
        ')',
        '_',
        '+',
        KEY_BACKSPACE,   // BACKSPACE
        KEY_TAB,         // TAB
        'Q',
        'W',
        'E',
        'R',
        'T',                    // 20
        'Y',
        'U',
        'I',
        'O',
        'P',
        0,   // MOD ^
        0,   // MOD ¨
        KEY_RETURN,   // ENTER
        0,   // VER MAJ
        'A',                    // 30
        'S',
        'D',
        'F',
        'G',
        'H',
        'J',
        'K',
        'L',
        ':',
        0,   // MOD `           // 40
        0,   // TODO
        KEY_LSHIFT,   // LEFT SHIFT
        '>',
        'Z',
        'X',
        'C',
        'V',
        'B',
        'N',
        'M',                    // 50
        '\'',
        '.',
        ' ', // É
        KEY_RSHIFT,    // RIGHT SHIFT
        0,   // TODO
        0,     // ALT left / right
        ' ',
        ' ',
        0,     // F1
        0,     // F2               // 60
        0,     // F3
        0,     // F4
        0,     // F5
        0,     // F6
        0,     // F7
        0,     // F8
        0,     // F9
        0,     // SCROLL LOCK
        0,     // PAUSE
        0,                     // 70
        0,
        0,
        KEY_PGUP,
        0,
        0,
        0,
        0,
        0,
        0,
        0,                  // 80
        KEY_PGDOWN,
        0
    }
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Parse the keycode given as parameter and execute the corresponding action.
 *
 * @param keycode The keycode to parse.
 */
static void manage_keycode(const int8_t keycode)
{
    int8_t  shifted;
    char    character;
    int32_t new_keycode;
    OS_RETURN_E err;
    int8_t  mod = 0;

    /* Manage push of release */
    if(keycode > 0)
    {
        /* Manage modifiers */
        switch(qwerty_map.regular[keycode])
        {
            case KEY_LSHIFT:
                keyboard_flags |= KBD_LSHIFT;
                mod = 1;
                break;
            case KEY_RSHIFT:
                keyboard_flags |= KBD_RSHIFT;
                mod = 1;
                break;
            default:
                break;
        }
        /* Manage only set characters */
        if(!mod &&
           (qwerty_map.regular[keycode] || qwerty_map.shifted[keycode]))
        {

            shifted = (keyboard_flags & KBD_LSHIFT) |
                      (keyboard_flags & KBD_RSHIFT);
            character = (shifted > 0) ?
                         qwerty_map.shifted[keycode] :
                         qwerty_map.regular[keycode];

            /* Manage user buffer */
            if(kbd_buf.type != 0)
            {
                if(kbd_buf.type == 1)
                {
                    /* read */
                    if(character == KEY_RETURN)
                    {
                        if(kbd_buf.read < kbd_buf.read_size)
                        {
                            kbd_buf.char_buf[kbd_buf.read++] = character;
                        }

                        kbd_buf.type = 0;

                        err = sem_post(&kbd_buf.sem);
                        if(err != OS_NO_ERR)
                        {
                            kernel_error("Keyboard driver failure");
                            kernel_panic();
                        }
                    }
                    else if(character == KEY_BACKSPACE)
                    {
                        if(kbd_buf.read > 0)
                        {
                            --kbd_buf.read;
                        }

                        kbd_buf.char_buf[kbd_buf.read] = 0;
                    }
                    else if(kbd_buf.read < kbd_buf.read_size)
                    {
                        kbd_buf.char_buf[kbd_buf.read++] = character;
                    }
                }
                else if(kbd_buf.type  == 2)
                {
                    /* getch */
                    if(kbd_buf.read < kbd_buf.read_size)
                    {
                        kbd_buf.char_buf[kbd_buf.read++] = character;
                    }

                    kbd_buf.type = 0;

                    err = sem_post(&kbd_buf.sem);
                    if(err != OS_NO_ERR)
                    {
                        kernel_error("Keyboard driver failure");
                        kernel_panic();
                    }
                }
            }

            /* Display character */
            if(display_keyboard)
            {
                if(secure_input &&
                    character != KEY_RETURN &&
                    character != KEY_BACKSPACE)
                {
                    console_write_keyboard("*", 1);
                }
                else
                {
                    console_write_keyboard(&character, 1);
                }
            }
        }
    }
    else
    {
        new_keycode = 128 + keycode;
        /* Manage modifiers */
        switch(qwerty_map.regular[new_keycode])
        {
            case KEY_LSHIFT:
                keyboard_flags &= ~KBD_LSHIFT;
                break;
            case KEY_RSHIFT:
                keyboard_flags &= ~KBD_RSHIFT;
                break;
            default:
                break;
        }
    }
}

/* Keyboard IRQ handler, read the key value and manage thread blocked on IO.
 *
 * @param cpu_state The cpu registers before the interrupt.
 * @param int_id The interrupt line that called the handler.
 * @param stack_state The stack state before the interrupt.
 */
static void keyboard_interrupt_handler(cpu_state_t* cpu_state, uint32_t int_id,
                                       stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    int8_t keycode;

    /* Read if not empty and not from auxiliary port */
    if((inb(KEYBOARD_COMM_PORT) & 0x100001) == 1)
    {
        /* Retrieve key code and test it */
        keycode = inb(KEYBOARD_DATA_PORT);

        /* Manage keycode */
        manage_keycode(keycode);
    }

    set_IRQ_EOI(KBD_IRQ_LINE);
}

OS_RETURN_E init_keyboard(void)
{
    OS_RETURN_E err;

    /* Init keyboard setings */
    keyboard_flags   = 0;
    display_keyboard = 1;

    err = mutex_init(&kbd_mutex, MUTEX_FLAG_NONE);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    memset(&kbd_buf, 0, sizeof(kbd_buffer_t));

    /* Init interuption settings */
    err = register_interrupt_handler(KBD_INTERRUPT_LINE,
                                     keyboard_interrupt_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = set_IRQ_mask(KBD_IRQ_LINE, 1);

    return err;
}

uint32_t read_keyboard(char* buffer, const uint32_t size)
{
    OS_RETURN_E err;
    uint32_t    read = 0;

    if(buffer == NULL || size == 0)
    {
        return 0;
    }

    err = mutex_pend(&kbd_mutex);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot release its mutex[%d]\n", err);
        kernel_panic();
    }

    /* Set current buffer */
    err = sem_init(&kbd_buf.sem, 0);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot create buffer semaphore[%d]\n", err);
        kernel_panic();
    }
    kbd_buf.char_buf  = buffer;
    kbd_buf.read_size = size;
    kbd_buf.read      = 0;
    kbd_buf.type      = 1;

    /* Wait for completion */
    err = sem_pend(&kbd_buf.sem);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot pend buffer semaphore[%d]\n", err);
        kernel_panic();
    }

    read = kbd_buf.read;

    /* Release resources */
    err = sem_destroy(&kbd_buf.sem);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot delete buffer semaphore[%d]\n", err);
        kernel_panic();
    }

    kbd_buf.char_buf  = NULL;
    kbd_buf.read_size = 0;
    kbd_buf.read      = 0;
    kbd_buf.type      = 0;

    err = mutex_post(&kbd_mutex);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot release its mutex[%d]\n", err);
        kernel_panic();
    }

    return read;
}

uint32_t secure_read_keyboard(char* buffer, const uint32_t size)
{
    /* Read string */
    uint32_t new_size = read_keyboard(buffer, size);

    /* Secure output */
    if(new_size < size - 1)
    {
        buffer[new_size] = '\0';
    }
    else
    {
        buffer[size - 1] = '\0';
    }

    return new_size;
}

void getch(char* character)
{
    OS_RETURN_E err;

    if(character == NULL)
    {
        return;
    }

    err = mutex_pend(&kbd_mutex);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot release its mutex[%d]\n", err);
        kernel_panic();
    }

    /* Set current buffer */
    err = sem_init(&kbd_buf.sem, 0);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot create buffer semaphore[%d]\n", err);
        kernel_panic();
    }
    
    kbd_buf.char_buf  = character;
    kbd_buf.read_size = 1;
    kbd_buf.read      = 0;
    kbd_buf.type      = 2;

    /* Wait for completion */
    err = sem_pend(&kbd_buf.sem);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot pend buffer semaphore[%d]\n", err);
        kernel_panic();
    }

    /* Release resources */
    err = sem_destroy(&kbd_buf.sem);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot delete buffer semaphore[%d]\n", err);
        kernel_panic();
    }

    kbd_buf.char_buf  = NULL;
    kbd_buf.read_size = 0;
    kbd_buf.read      = 0;
    kbd_buf.type      = 0;

    err = mutex_post(&kbd_mutex);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot release its mutex[%d]\n", err);
        kernel_panic();
    }
}

void keyboard_enable_secure(void)
{
    secure_input = 1;
}

void keyboard_disable_secure(void)
{
    secure_input = 0;
}

void keyboard_enable_display(void)
{
    display_keyboard = 1;
}

void keyboard_disable_display(void)
{
    display_keyboard = 0;
}
