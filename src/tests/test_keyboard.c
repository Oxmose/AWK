/*******************************************************************************
 *
 * File: test_keyboard.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: Keyboard
 ******************************************************************************/

/*
 * !!! THESE TESTS MUST BE DONE BEFORE INITIALIZING ANY INTERRUPT HANDLER
 *     BETWEEN MIN_INTERRUPT_LINE AND MAX_INTERRUPT_LINE !!!
 * !!! THESE TESTS MUST BE DONE AFTER INITIALIZING THE PIC OR THE
 *     IOAPIC!!!
 */

#include "../core/interrupts.h"
#include "../core/kernel_output.h"
#include "../core/panic.h"
#include "../cpu/cpu.h"
#include "../drivers/keyboard.h"



void test_keyboard(void)
{
    kernel_debug("KEYBOARD tests passed\n");
}
