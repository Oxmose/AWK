/*******************************************************************************
 *
 * File: keyboard.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 1.5
 *
 * Keyboard driver (PS2) for the kernel.
 ******************************************************************************/

#ifndef __KEYBOARD_H_
#define __KEYBOARD_H_

/* Key codes */
#include "keyboard_codes.h"

#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E */
#include "../core/interrupts.h" /* cpu_state_t stack_state_t */

/* Keyboard settings */
#define KEYBOARD_COMM_PORT      0x64
#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_INTERRUPT_LINE 33
#define KEYBOARD_IRQ            1

#define KEYBOARD_BUFFER_SIZE 512

/* Flags */
#define KBD_LSHIFT 0x00000001
#define KBD_RSHIFT 0x00000002

/*********************************
 * STRUCTURES
 ********************************/
typedef struct key_mapper
{
	uint16_t regular[128];
	uint16_t shifted[128];
} key_mapper_t;

/********************************
 * FUNCTIONS
 *******************************/

/* Init keyboard settings
 *
 * @return The error or success state.
 */
OS_RETURN_E init_keyboard(void);

/* Fill buffer with keyboard buffer. Is blocking 
 *
 * @param buffer The bufer to fill with the user input.
 * @param siwe The maximum size of the buffer.
 * @return The actual number of characters read.
 */
uint32_t read_keyboard(char *buffer, const uint32_t size);

/* Fill buffer with keyboard buffer and add \0, is blocking 
 *
 * @param buffer The bufer to fill with the user input.
 * @param siwe The maximum size of the buffer.
 * @return The actual number of characters read.
 */
uint32_t secure_read_keyboard(char *buffer, const uint32_t size);

/* Read one character from the keyboard, is blocking 
 *
 * @param character The buffer to write the character to.
 */
void getch(char *character);

/* Enable secure input : replace input with * */
void keyboard_enable_secure(void);

/* Disable secure intput: show input */
void keyboard_disable_secure(void);

/* Enable keyboard character print on screen */
void keyboard_enable_display(void);

/* Disable keyboard character print on screen */
void keyboard_disable_display(void);

#endif /* __KEYBOARD_H_ */
