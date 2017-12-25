/***************************************
 *
 * File: keyboard_driver.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 08/10/2017
 *
 * Version: 1.0
 *
 * See: keyboard_driver.h
 *
 * Keyboard management for the kernel and the threads.
 */
#include "pit.h"                   /* get_current_time */
#include "vga_text.h"              /* console_write_keyboard */
#include "../cpu/cpu.h"            /* outb inb */
#include "../core/interrupts.h"    /* register_interrupt, cpu_state, 
                                    * stack_state, set_IRQ_mask, set_IRQ_EOI */
#include "../core/scheduler.h"     /* lock_io, unlock_io */
#include "../lib/stdint.h"         /* Generic int types */
#include "../lib/stddef.h"         /* OS_RETURN_E, OS_EVENT_ID */
#include "../lib/string.h"         /* memcpy */
#include "../lib/stdio.h"          /* printf */
#include "../sync/lock.h"          /* enable_interrupt, disable_interrupt */

/* Header file */
#include "keyboard.h"

/* Keyboard security */
static uint8_t secure_input;
static uint8_t display_keyboard;

/* Keybard buffer */
static uint8_t buffer_enabled;

/* Shift key used */
static uint32_t keyboard_flags;

/* Keyboard buffer */
static char keyboard_buffer;
static lock_t buffer_lock;

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

static void manage_keycode(const int8_t keycode)
{

    int8_t mod = 0;
    int8_t shifted;
    char character;
    int32_t new_keycode;

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
            /* Save key */ 
            if(buffer_enabled != 0)
            {
                keyboard_buffer = character;                             
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

static void keyboard_interrupt_handler(cpu_state_t *cpu_state, uint32_t int_id, 
                                       stack_state_t *stack_state)
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

        if(buffer_enabled != 0)
        {
            /* Unlock threads waiting for keyboards */
            unlock_io(IO_KEYBOARD);
        }
;    }

    set_IRQ_EOI(KEYBOARD_IRQ);
}

OS_RETURN_E init_keyboard(void)
{
    OS_RETURN_E err;

    /* Init keyboard setings */
    buffer_enabled   = 0;
    keyboard_buffer  = 0;
    keyboard_flags   = 0;
    display_keyboard = 1;

    spinlock_init(&buffer_lock);

    /* Init interuption settings */
    err = register_interrupt_handler(KEYBOARD_INTERRUPT_LINE, 
                                     keyboard_interrupt_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = set_IRQ_mask(KEYBOARD_IRQ, 1);

    return OS_NO_ERR;
}
 
uint32_t read_keyboard(char *buffer, const uint32_t size)
{
    char read_char;
    uint32_t read = 0;

    do
    {
        getch(&read_char);
        if(read_char == KEY_RETURN)
        {
            break;
        }
        else if(read_char == KEY_BACKSPACE)
        {
            if(read > 0)
            {
                --read;
            }

            buffer[read] = 0;
        }
        else
        {
            buffer[read++] = read_char;
        }
        if(read >= size)
        {
            --read;
        }
    }
    while(1);

    get_active_thread()->io_req_time = 0;

    return read;
}

uint32_t secure_read_keyboard(char *buffer, const uint32_t size)
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

void getch(char *character)
{
    spinlock_lock(&buffer_lock);

    /* Enable buffer */
    ++buffer_enabled;

    /* Get the request time for the thread */
    get_active_thread()->io_req_time = get_current_uptime();

    /* If no character is in the buffer but the thread in IO waiting state */
    while(keyboard_buffer == 0)
    {
        spinlock_unlock(&buffer_lock);
        lock_io(IO_KEYBOARD);
        spinlock_lock(&buffer_lock);
    }
    
    *character = keyboard_buffer;
    keyboard_buffer = 0;

    /* Disable buffer */
    --buffer_enabled;

    spinlock_unlock(&buffer_lock);    
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
